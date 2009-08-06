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

  This file captures the initialization of BOM Access Manager.
  
******************************************************************************
*/

/* NCS specific include files */

#include <gl_defs.h>
#include <t_suite.h>
#include <ncs_mib_pub.h>
#include <ncs_lib.h>
#include <ncsgl_defs.h>
#include <mds_papi.h>
#include <mds_adm.h>

#include "bam.h"
#include "bam_log.h"
#include "ncs_avsv_bam_msg.h"

EXTERN_C uns32 gl_ncs_bam_hdl;

const MDS_CLIENT_MSG_FORMAT_VER bam_avd_msg_fmt_map_table[BAM_AVD_SUBPART_VER_MAX] = { AVSV_AVD_BAM_MSG_FMT_VER_1 };
const MDS_CLIENT_MSG_FORMAT_VER bam_avm_msg_fmt_map_table[BAM_AVM_SUBPART_VER_MAX] = { AVSV_AVM_BAM_MSG_FMT_VER_1 };
const MDS_CLIENT_MSG_FORMAT_VER bam_pss_msg_fmt_map_table[BAM_PSS_SUBPART_VER_MAX] = { NCS_BAM_PSS_MSG_FMT_VER_1 };

static uns32 bam_mds_svc_evt(NCS_BAM_CB *bam_cb, MDS_CALLBACK_SVC_EVENT_INFO *evt_info);

static uns32 bam_avd_mds_cpy(MDS_CALLBACK_COPY_INFO *cpy_info)
{
	AVD_BAM_MSG *msg = cpy_info->i_msg;
	cpy_info->o_msg_fmt_ver = bam_avd_msg_fmt_map_table[cpy_info->i_rem_svc_pvt_ver - 1];
	cpy_info->o_cpy = (NCSCONTEXT)msg;
	cpy_info->i_msg = 0;	/* memory is not allocated and hence this */

	return NCSCC_RC_SUCCESS;
}

static uns32 bam_avm_mds_cpy(MDS_CALLBACK_COPY_INFO *cpy_info)
{
	BAM_AVM_MSG_T *msg = cpy_info->i_msg;

	cpy_info->o_msg_fmt_ver = bam_avm_msg_fmt_map_table[cpy_info->i_rem_svc_pvt_ver - 1];
	cpy_info->o_cpy = (NCSCONTEXT)msg;
	cpy_info->i_msg = 0;	/* memory is not allocated and hence this */

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : bam_mds_callback
 *
 * Description   : Call back function provided to MDS. MDS will call this
 *                 function for enc/dec/cpy/rcv/svc_evt operations.
 *
 * Arguments     : NCSMDS_CALLBACK_INFO *info: call back info.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 bam_mds_callback(NCSMDS_CALLBACK_INFO *info)
{
	MDS_CLIENT_HDL cb_hdl;
	NCS_BAM_CB *bam_cb = NULL;
	uns32 rc = NCSCC_RC_FAILURE;

	if (info == NULL)
		return rc;

	cb_hdl = info->i_yr_svc_hdl;

	if ((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, (uns32)cb_hdl)) == NULL) {
		m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
		return NCSCC_RC_FAILURE;
	}

	switch (info->i_op) {
	case MDS_CALLBACK_RECEIVE:

		if ((info->info.receive.i_fr_svc_id == NCSMDS_SVC_ID_PSS) &&
		    (info->info.receive.i_to_svc_id == NCSMDS_SVC_ID_BAM)) {
			/* store the AVD MDS_DEST info here to be used later */
			bam_cb->pss_dest = info->info.receive.i_fr_dest;

			rc = bam_psr_rcv_msg(bam_cb, (PSS_BAM_MSG *)info->info.receive.i_msg);
			if (rc != NCSCC_RC_SUCCESS) {
				ncshm_give_hdl((uns32)cb_hdl);

				/* m_LOG_BAM_SVC_PRVDR(BAM_MDS_RCV_ERROR, NCSFL_SEV_ERROR); */
				return (NCSCC_RC_FAILURE);
			}
		}
		break;

	case MDS_CALLBACK_SVC_EVENT:
		{
			rc = bam_mds_svc_evt(bam_cb, &info->info.svc_evt);
			if (rc != NCSCC_RC_SUCCESS) {
				ncshm_give_hdl((uns32)cb_hdl);

				/* m_LOG_BAM_SVC_PRVDR(BAM_MDS_RCV_ERROR, NCSFL_SEV_ERROR); */
				return rc;
			}
		}
		break;
	case MDS_CALLBACK_COPY:
		{
			if ((info->info.cpy.i_to_svc_id == NCSMDS_SVC_ID_AVD)) {
				rc = bam_avd_mds_cpy(&info->info.cpy);
				if (rc != NCSCC_RC_SUCCESS) {
					ncshm_give_hdl((uns32)cb_hdl);

					/* Log a Message */
					return rc;
				}
			} else if ((info->info.cpy.i_to_svc_id == NCSMDS_SVC_ID_AVM)) {
				rc = bam_avm_mds_cpy(&info->info.cpy);
				if (rc != NCSCC_RC_SUCCESS) {
					ncshm_give_hdl((uns32)cb_hdl);

					/* Log a Message */
					return rc;
				}
			}
		}
		break;
	case MDS_CALLBACK_ENC:
	case MDS_CALLBACK_ENC_FLAT:
		{
			if ((info->i_yr_svc_id != NCSMDS_SVC_ID_BAM) ||
			    (info->info.enc.i_to_svc_id != NCSMDS_SVC_ID_PSS)) {
				ncshm_give_hdl((uns32)cb_hdl);

				/* log error */
				return NCSCC_RC_FAILURE;
			}

			info->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
									     BAM_PSS_SUBPART_VER_MIN,
									     BAM_PSS_SUBPART_VER_MAX,
									     bam_pss_msg_fmt_map_table);

			if (info->info.enc.o_msg_fmt_ver < NCS_BAM_PSS_MSG_FMT_VER_1) {
				ncshm_give_hdl((uns32)cb_hdl);

				/* log the problem */
				return NCSCC_RC_FAILURE;
			}

			rc = pss_bam_mds_enc(cb_hdl, info->info.enc.i_msg,
					     info->info.enc.i_to_svc_id, info->info.enc.io_uba);
			if (rc != NCSCC_RC_SUCCESS) {
				ncshm_give_hdl((uns32)cb_hdl);

				/* log error */
				return rc;
			}
		}

		break;
	case MDS_CALLBACK_DEC:
	case MDS_CALLBACK_DEC_FLAT:
		{
			if ((info->i_yr_svc_id != NCSMDS_SVC_ID_BAM) ||
			    (info->info.dec.i_fr_svc_id != NCSMDS_SVC_ID_PSS)) {
				ncshm_give_hdl((uns32)cb_hdl);

				/* log error */
				return NCSCC_RC_FAILURE;
			}

			if (!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec.i_msg_fmt_ver,
						       BAM_PSS_SUBPART_VER_MIN,
						       BAM_PSS_SUBPART_VER_MAX, bam_pss_msg_fmt_map_table)) {
				/* log the problem */
				ncshm_give_hdl((uns32)cb_hdl);

				return NCSCC_RC_FAILURE;
			}

			rc = pss_bam_mds_dec(cb_hdl, &info->info.dec.o_msg,
					     info->info.dec.i_fr_svc_id, info->info.dec.io_uba);
			if (rc != NCSCC_RC_SUCCESS) {
				ncshm_give_hdl((uns32)cb_hdl);

				/* log error */
				return rc;
			}
		}

		break;
	default:
		rc = NCSCC_RC_FAILURE;
		break;
	}

	ncshm_give_hdl((uns32)cb_hdl);

	return rc;
}

/****************************************************************************
  Name          : bam_mds_svc_evt
 
  Description   : This routine is invoked to inform BAM of MDS events. BAM 
                  had subscribed to these events during MDS registration.
 
  Arguments     : bam_cb   - ptr to the BAM control block
                  evt_info - ptr to the MDS event info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
static uns32 bam_mds_svc_evt(NCS_BAM_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *evt_info)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	BAM_EVT *evt = NULL;

	switch (evt_info->i_change) {
	case NCSMDS_UP:
		if (evt_info->i_svc_id == NCSMDS_SVC_ID_AVD) {
			if (m_NCS_MDS_DEST_EQUAL(&cb->my_addr, &evt_info->i_dest)) {
				cb->avd_dest = evt_info->i_dest;
			}
		} else if (evt_info->i_svc_id == NCSMDS_SVC_ID_AVM) {
			if (m_NCS_MDS_DEST_EQUAL(&cb->my_addr, &evt_info->i_dest)) {
				cb->avm_dest = evt_info->i_dest;
				if (cb->ha_state == SA_AMF_HA_ACTIVE) {
					evt = m_MMGR_ALLOC_BAM_DEFAULT_VAL(sizeof(BAM_EVT));
					if (evt == NULL) {
						/* LOG an error */
						return NCSCC_RC_FAILURE;
					}

					evt->cb_hdl = cb->cb_handle;
					evt->evt_type = NCS_BAM_AVM_UP;

					if (m_NCS_IPC_SEND(cb->bam_mbx, evt, NCS_IPC_PRIORITY_HIGH)
					    != NCSCC_RC_SUCCESS) {
						m_MMGR_FREE_BAM_DEFAULT_VAL(evt);
						return NCSCC_RC_FAILURE;
					}
				}
			}
		}
		{
			/* m_LOG_BAM_SVC_PRVDR(BAM_MDS_RCV_ERROR, NCSFL_SEV_ERROR); */
		}
		break;

	case NCSMDS_DOWN:
		if (evt_info->i_svc_id == NCSMDS_SVC_ID_AVD) {
			memset(&cb->avd_dest, 0, sizeof(MDS_DEST));
		} else if (evt_info->i_svc_id == NCSMDS_SVC_ID_AVM) {
			memset(&cb->avm_dest, 0, sizeof(MDS_DEST));
		}
		break;

	default:
		/* m_LOG_BAM_SVC_PRVDR(BAM_MDS_RCV_ERROR, NCSFL_SEV_ERROR); */
		break;
	}

	return rc;
}

/****************************************************************************
 * Name          : bam_mds_adest_get
 *
 * Description   : This function Gets the Handles of local MDS destination
 *
 * Arguments     : cb   : BAM control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 bam_mds_adest_get(NCS_BAM_CB *cb)
{
	NCSADA_INFO arg;
	uns32 rc;

	memset(&arg, 0, sizeof(NCSADA_INFO));

	arg.req = NCSADA_GET_HDLS;
	arg.info.adest_get_hdls.i_create_oac = FALSE;

	rc = ncsada_api(&arg);

	if (rc != NCSCC_RC_SUCCESS) {
		/* Log, */
		return rc;
	}

	cb->mds_hdl = arg.info.adest_get_hdls.o_mds_pwe1_hdl;

	return rc;
}

uns32 ncs_bam_mds_reg(NCS_BAM_CB *cb)
{
	NCSMDS_INFO svc_to_mds_info;
	uns32 rc = NCSCC_RC_SUCCESS;
	MDS_SVC_ID svc_id[2] = { 0 };

	/* Create the Destination for BAM */
	rc = bam_mds_adest_get(cb);
	if (rc != NCSCC_RC_SUCCESS) {
		/* LOG */
		return rc;
	}

	/* Install your service into MDS */
	memset(&svc_to_mds_info, 0, sizeof(NCSMDS_INFO));

	svc_to_mds_info.i_mds_hdl = cb->mds_hdl;
	svc_to_mds_info.i_svc_id = NCSMDS_SVC_ID_BAM;
	svc_to_mds_info.i_op = MDS_INSTALL;

	svc_to_mds_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)cb->cb_handle;
	svc_to_mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
	svc_to_mds_info.info.svc_install.i_svc_cb = bam_mds_callback;
	svc_to_mds_info.info.svc_install.i_mds_q_ownership = FALSE;
	svc_to_mds_info.info.svc_install.i_mds_svc_pvt_ver = BAM_MDS_SUB_PART_VERSION;

	rc = ncsmds_api(&svc_to_mds_info);

	if (rc != NCSCC_RC_SUCCESS) {
		/* LOG */
		return NCSCC_RC_FAILURE;
	}

	/* Store the self destination */
	cb->my_addr = svc_to_mds_info.info.svc_install.o_dest;
	cb->card_anchor = svc_to_mds_info.info.svc_install.o_anc;

   /*** subscribe to AVD mds event ***/
	svc_to_mds_info.i_op = MDS_SUBSCRIBE;
	svc_to_mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_INTRANODE;
	svc_to_mds_info.info.svc_subscribe.i_svc_ids = svc_id;
	svc_to_mds_info.info.svc_subscribe.i_num_svcs = 2;
	svc_id[0] = NCSMDS_SVC_ID_AVD;
	svc_id[1] = NCSMDS_SVC_ID_AVM;

	if (ncsmds_api(&svc_to_mds_info) != NCSCC_RC_SUCCESS) {
		svc_to_mds_info.i_op = MDS_UNINSTALL;

		ncsmds_api(&svc_to_mds_info);	/* Don't care the rc */

		m_LOG_BAM_SVC_PRVDR(BAM_MDS_SUBSCRIBE_FAIL, NCSFL_SEV_ERROR);
		return NCSCC_RC_FAILURE;
	}

	return rc;
}

/*****************************************************************************
 * Function: bam_mds_unreg
 *
 * Purpose:  This function unregisters BAM with MDS. 
 *
 * Input: Pointer to bam Control Block.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
void bam_mds_unreg(NCS_BAM_CB *bam_cb)
{
	NCSMDS_INFO svc_to_mds_info;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(&svc_to_mds_info, 0, sizeof(NCSMDS_INFO));

	/* uninstall MDS */
	svc_to_mds_info.i_mds_hdl = bam_cb->mds_hdl;
	svc_to_mds_info.i_svc_id = NCSMDS_SVC_ID_BAM;
	svc_to_mds_info.i_op = MDS_UNINSTALL;

	rc = ncsmds_api(&svc_to_mds_info);

	if (rc != NCSCC_RC_SUCCESS) {
		/* log an error */
		return;
	}
	return;
}
