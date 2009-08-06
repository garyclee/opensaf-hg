/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Emerson Network Power
 *
 */

/*****************************************************************************
..............................................................................

 
..............................................................................

  DESCRIPTION:

  This module contains procedures related to UBSAR.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  Encode functions

  ncs_ubsar_init...........Bring UBSAR layer to init state.
  ncs_ubsar_register.......Allocate a new UBSAR for a system that wants to use its services.
  ncs_ubsar_send...........send a stream after splitting it into number of packets
  ncs_ubsar_recv...........receive a packet and assemble it.
  ncs_ubsar_deregister.....Deallocate a UBSAR for a system that dont want to use UBSAR services
  ncs_ubsar_destroy........Remove the UBSAR layer.

******************************************************************************/

/** Get compile time options...**/
#include "ncs_opt.h"

/** Get general definitions...**/
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncssysf_def.h"
#include "ncsencdec_pub.h"

/** Get NCS UBSAR Facility header file.**/
#include "ncs_ubsar.h"
#include "usrbuf.h"

/*****************************************************************************

  PROCEDURE NAME:    ncs_ubsar_init

  DESCRIPTION:
 Initalize all global variables in the UBSAR library

  RETURNS:
 NCSCC_RC_SUCCESS   init went well
  NCSCC_RC_FAILURE   i_ubsar is not present

*****************************************************************************/

uns32 ubsar_init(NCS_UBSAR_INIT *init)
{
	NCS_UBSAR *ubs;

	if (init->i_ubsar == NULL)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	ubs = init->i_ubsar;

	ubs->max_size = init->i_size;	/* max_size of sent packet         */
	ubs->time_interval = init->i_time_interval;	/* wait timer interval             */
	ubs->lm_notify_func = init->i_lm_cb_func;	/* notify function pointer         */
	ubs->lm_notify_arg = init->i_lm_cb_arg;	/* callback arg1 for layerm        */
	ubs->last_seq_no = 0;	/* last seq number start val       */
	ubs->arrival_time = 0;	/* arrival time of the last packet */
	ubs->start = NULL;	/* set the start value             */

	m_MMGR_UBQ_CREATE(ubs->ubq_ubf);	/* put UBQ in start state          */
	m_MMGR_UBQ_CREATE(ubs->ubq_seg);	/* put UBQ in start state          */

	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************

  PROCEDURE NAME:    ncs_ubsar_segment

  DESCRIPTION:
 Segment a stream passed by application and adding trailer.

  RETURNS:  SUCCESS/FAILURE

*****************************************************************************/

uns32 ubsar_segment(NCS_UBSAR_SEGMENT *seg)
{
	NCS_UBSAR *ubs;
	uns16 flag_and_seq_no = 0x0000;
	uns16 last_seq_no = 0x0000;
	USRBUF *packet = NULL;

	if (seg->i_ubsar == NULL)
		return m_LEAP_DBG_SINK(0);

	seg->o_cnt = 0;
	seg->o_segments = NULL;

	ubs = seg->i_ubsar;	/* get the ubsar_handle */

	/* segment the USRBUF stream into smaller packets */

	seg->o_cnt = m_MMGR_FRAG_BUFR(seg->i_bigbuf, (uns32)(ubs->max_size - NCS_UBSAR_CPLT_TRLR_SIZE), ubs->ubq_ubf);

	if (seg->o_cnt == 0) {	/* SMM unclear that if 0, we shouldn't free i_bigbuf */
		while ((packet = m_MMGR_UBQ_DQ_HEAD(ubs->ubq_ubf)) != NULL)	/* free usrbuf's in ubq */
			m_MMGR_FREE_BUFR_LIST(packet);

		seg->o_cnt = 0;
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Append trailer to each packet
	 *
	 * Check that the seq no should not be less than no of segments
	 * sequence number should not overflow onto the flag
	 */
	for (last_seq_no = 0; last_seq_no < seg->o_cnt; last_seq_no++) {
		packet = m_MMGR_UBQ_DQ_HEAD(ubs->ubq_ubf);

		if (last_seq_no == (seg->o_cnt - 1))
			flag_and_seq_no = (uns16)(UBSAR_TRLR_FLAG | last_seq_no);
		else
			flag_and_seq_no = last_seq_no;

		/* append trailers:
		 * 8 bytes of application specific trailer AND
		 * 2 bytes flag and sequence no
		 */
		ncs_encode_uns32(packet, seg->i_app_trlr_1);
		ncs_encode_uns32(packet, seg->i_app_trlr_2);
		ncs_encode_uns16(packet, flag_and_seq_no);

		m_MMGR_UBQ_NQ_TAIL(ubs->ubq_seg, packet);
	}

	seg->o_segments = &ubs->ubq_seg;
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ubsar_get_app_trlr

  DESCRIPTION:  Get the application specific trailer

  RETURNS:
    NCSCC_RC_SUCCESS 
    NCSCC_RC_FAILURE 

*****************************************************************************/

uns32 ubsar_get_app_trlr(NCS_UBSAR_TRLR *trlr)
{
	uns32 ttl_len;
	char t_buffer[NCS_UBSAR_APP_TRLR_SIZE + 1];
	uns8 *p_buffer;

	ttl_len = m_MMGR_LINK_DATA_LEN(trlr->i_inbuf);	/* get the total length */

	/* Get the first app trailer in a buffer */

	if ((p_buffer = (uns8 *)m_MMGR_PTR_MID_DATA(trlr->i_inbuf,
						    ttl_len - NCS_UBSAR_CPLT_TRLR_SIZE,
						    NCS_UBSAR_APP_TRLR_SIZE - 4, t_buffer)) == NULL)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	trlr->o_app_trlr_1 = (uns32)ncs_decode_32bit(&p_buffer);	/* decode the trailer */

	/* Get the second app trailer in a buffer */
	if ((p_buffer = (uns8 *)m_MMGR_PTR_MID_DATA(trlr->i_inbuf,
						    ttl_len - NCS_UBSAR_CPLT_TRLR_SIZE + 4,
						    NCS_UBSAR_APP_TRLR_SIZE - 4, t_buffer)) == NULL)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	trlr->o_app_trlr_2 = (uns32)ncs_decode_32bit(&p_buffer);	/* decode the trailer */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_ubsar_assemble

  DESCRIPTION:
 Assemble a stream

  RETURNS:
    NCSCC_RC_SUCCESS 
    NCSCC_RC_FAILURE 

*****************************************************************************/

uns32 ubsar_assemble(NCS_UBSAR_ASSEMBLE *ass)
{
	uns32 ttl_len;
	char t_buffer[NCS_UBSAR_TRLR_SIZE + 1];
	uns8 *p_buffer;
	uns16 flag_seq_no, flag, seq_no;
	NCS_UBSAR *ubs;

	ass->o_assembled = NULL;	/* set response to default state */

	ubs = ass->i_ubsar;
	if (ubs == NULL)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	m_GET_TIME_STAMP(ubs->arrival_time);	/* Update the packet arrival time */
	ttl_len = m_MMGR_LINK_DATA_LEN(ass->i_inbuf);	/* get the total length */

	/* Get the trailer in a buffer */

	if ((p_buffer = (uns8 *)m_MMGR_PTR_MID_DATA(ass->i_inbuf,
						    ttl_len - NCS_UBSAR_TRLR_SIZE,
						    NCS_UBSAR_TRLR_SIZE, t_buffer)) == NULL)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	/* decode the trailer */

	flag_seq_no = (uns16)ncs_decode_16bit(&p_buffer);
	flag = (uns16)(flag_seq_no & UBSAR_TRLR_FLAG);
	seq_no = (uns16)(flag_seq_no & UBSAR_SEQ_MASK);

	/* compare the seq no. */

	if (ubs->last_seq_no != seq_no) {	/* if out of sequence, bail out!! */
		m_MMGR_FREE_BUFR_LIST(ubs->start);	/* clean up the ubsar */
		ubs->start = NULL;
		ubs->last_seq_no = 0;
		ubs->arrival_time = 0;

		/* notify LM */
		(ubs->lm_notify_func) (ubs->lm_notify_arg, NCS_UBSAR_LM_WRONG_SEQ_NO);

		/* if this new seq no is first packet of the message store it */
		if (seq_no != 0)
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* remove trailer from end of USRBUF */

	m_MMGR_REMOVE_FROM_END(ass->i_inbuf, NCS_UBSAR_CPLT_TRLR_SIZE);

	/* append pkt to UBSAR start */

	if (ubs->start != NULL)
		m_MMGR_APPEND_DATA(ubs->start, ass->i_inbuf);
	else
		ubs->start = ass->i_inbuf;

	ass->i_inbuf = NULL;	/* don't give pointer back !! */

	/* if pkt is last packet, clear seq no, timestamp etc. */

	if (flag) {
		ubs->last_seq_no = 0;
		ubs->arrival_time = 0;
		ass->o_assembled = ubs->start;
		ubs->start = NULL;

		return NCSCC_RC_SUCCESS;
	}

	ubs->last_seq_no++;	/* update seq no */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_ubsar_destroy

  DESCRIPTION:
 Release all resources of UBSAR layer.

  RETURNS:
    NCSCC_RC_SUCCESS
    NCSCC_RC_FAILURE

  NOTES:

*****************************************************************************/

uns32 ubsar_destroy(NCS_UBSAR_DESTROY *dest)
{
	NCS_UBSAR *ubs;

	if (dest->i_ubsar == NULL)
		return NCSCC_RC_FAILURE;

	ubs = dest->i_ubsar;

	ubs->max_size = 0;	/* max_size of sent packet */
	ubs->time_interval = 0;	/* wait timer interval */
	ubs->lm_notify_func = 0;	/* notify function pointer */
	ubs->last_seq_no = 0;	/* last seq number start val */
	ubs->arrival_time = 0;	/* arrival time of the last packet */

	if (ubs->start != NULL) {
		m_MMGR_FREE_BUFR_LIST(ubs->start);
		ubs->start = NULL;
	}

	m_MMGR_UBQ_RELEASE(ubs->ubq_seg);
	m_MMGR_UBQ_RELEASE(ubs->ubq_ubf);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_ubsar_request

  DESCRIPTION:  Main controler routine 

  ARGUMENTS:  args specifying the request

  RETURNS:   SUCCESS/FAILURE

  NOTES:   

*****************************************************************************/
uns32 ncs_ubsar_request(NCS_UBSAR_ARGS *args)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	if (args == NULL)
		return NCSCC_RC_FAILURE;

	switch (args->op) {
	case NCS_UBSAR_OP_INIT:
		rc = ubsar_init(&args->info.init);
		break;

	case NCS_UBSAR_OP_SGMNT:
		rc = ubsar_segment(&args->info.segment);
		break;

	case NCS_UBSAR_OP_ASMBL:
		rc = ubsar_assemble(&args->info.assemble);
		break;

	case NCS_UBSAR_OP_GETAPT:
		rc = ubsar_get_app_trlr(&args->info.app_trlr);
		break;

	case NCS_UBSAR_OP_DESTROY:
		rc = ubsar_destroy(&args->info.destroy);
		break;

	case NCS_UBSAR_OP_SET_OBJ:
		{
			NCS_UBSAR_SET_OBJ *set = &args->info.set_obj;

			if (set->i_ubsar == NULL) {
				rc = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
				break;
			}

			/* OK, lets see what oject value is to be set */

			switch (set->i_obj) {
			case NCS_UBSAR_OBJID_SEG_SIZE:
				set->i_ubsar->max_size = set->i_val;
				break;

			case NCS_UBSAR_OBJID_TIMEOUT:
				set->i_ubsar->arrival_time = set->i_val;
				break;

			default:
				m_LEAP_DBG_SINK_VOID(NCSCC_RC_FAILURE);
				break;
			}
			break;	/* for case NCS_UBSAR_OP_SET_OBJ */
		}

	default:
		rc = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
	return rc;
}
