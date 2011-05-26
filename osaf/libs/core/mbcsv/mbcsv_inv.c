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

  DESCRIPTION:

  This file contains API to print Message Based 
  Checkpointing Service (MBCSV) Inventory.

  ..............................................................................

  FUNCTIONS INCLUDED in this module:
  mbcsv_prt_inv    - Print MBCSV inventory.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mbcsv.h"

#if (MBCSV_DEBUG == 1)
#define   NUMBER_OF_HA_ROLES  4
#define   MAX_FSM_STATES      6

const char mbcsv_prt_role[] = { 'I', 'A', 'S', 'Q' };

const char *mbcsv_prt_fsm_state[NUMBER_OF_HA_ROLES][MAX_FSM_STATES] = {
	{"IDLE", "----", "----", "----", "----", "----"},
	{"IDLE", "WFCS", "KSIS", "MACT", "----", "----"},
	{"IDLE", "WTCS", "SISY", "WTWS", "VWSD", "WFDR"},
	{"QUIE", "----", "----", "----", "----", "----"}
};

extern MBCSV_CB mbcsv_cb;

#define m_MBCSV_PRT_INV_HDR \
{ \
   printf("\n------------------------------------------------------------------------------"); \
   printf("\n|             M  B  C  S  V     I  n  v  e  n  t  o  r  y                     |"); \
   printf("\n------------------------------------------------------------------------------"); \
   printf("\n|       SVC     |          CSI          |               PEER's                |"); \
   printf("\n------------------------------------------------------------------------------"); \
   printf("\n|SVC_ID|MBCSVHDL| PWEHDL | CSIHDL | W |R|    My Anchor   | My hdl |peer hdl|  |"); \
   printf("\n|---------------|    My Anchor   |ipfrd |R|FSMS|C|SYN|iwpcooacdcw|Version|    |"); \
   printf("\n------------------------------------------------------------------------------"); \
}

#define m_MBCSV_PRT_INV_FOOTER \
{ \
   printf("\n------------------------------------------------------------------------------"); \
   printf("\n----------M  B  C  S  V     I  n  v  e  n  t  o  r  y    E  n  d -------------"); \
   printf("\n------------------------------------------------------------------------------"); \
}

#define m_MBCSV_PRT_PEER_INFO(peer_ptr) \
{ \
   printf("\n|               |                       |%16" PRIX64 "|%8X|%8X|  |", \
   (uint64_t)peer_ptr->peer_anchor,(uint32_t)peer_ptr->hdl, (uint32_t)peer_ptr->peer_hdl); \
   \
   printf("\n|               |                       |%c|%s|%c| %c |%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d|%2d |    |", \
   mbcsv_prt_role[peer_ptr->peer_role],  \
   mbcsv_prt_fsm_state[ckpt->my_role][peer_ptr->state - 1], ((peer_ptr->incompatible)?'F':'T'), \
   ((peer_ptr->cold_sync_done)?'T':'F'), peer_ptr->incompatible, peer_ptr->warm_sync_sent,  \
   peer_ptr->peer_disc, peer_ptr->ckpt_msg_sent, peer_ptr->okay_to_async_updt, peer_ptr->okay_to_send_ntfy, \
   peer_ptr->ack_rcvd, peer_ptr->cold_sync_done, peer_ptr->data_resp_process, peer_ptr->c_syn_resp_process, \
   peer_ptr->w_syn_resp_process, peer_ptr->version); \
}

/****************************************************************************
 * PROCEDURE     : mbcsv_prt_inv
 *
 * Description   : This is the function which prints Inventory of MBCSV.  
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t mbcsv_prt_inv(void)
{
	MBCSV_REG *mbc_reg;
	CKPT_INST *ckpt;
	PEER_INST *peer_ptr;
	SS_SVC_ID svc_id = 0;
	SaAisErrorT rc = SA_AIS_OK;
	uint32_t pwe_hdl = 0;
	uint32_t c_count = 0, p_count = 0;

	if (mbcsv_cb.created == false)
		return m_MBCSV_DBG_SINK(SA_AIS_ERR_NOT_EXIST, "MBCA instance is not created. First call mbcsv dl api.");

	m_NCS_LOCK(&mbcsv_cb.global_lock, NCS_LOCK_READ);
	m_LOG_MBCSV_GL_LOCK(MBCSV_LK_LOCKED, &mbcsv_cb.global_lock);

	m_MBCSV_PRT_INV_HDR;

	/* 
	 * Walk through MBCSv reg list.
	 */
	while (NULL != (mbc_reg = (MBCSV_REG *)ncs_patricia_tree_getnext(&mbcsv_cb.reg_list, (const uint8_t *)&svc_id))) {
		m_NCS_LOCK(&mbc_reg->svc_lock, NCS_LOCK_READ);

		m_LOG_MBCSV_SVC_LOCK(MBCSV_LK_LOCKED, mbc_reg->svc_id, &mbc_reg->svc_lock);
		svc_id = mbc_reg->svc_id;

		printf("\n|%6d|%8X|         MY  CSI       |                                  |",
		       svc_id, mbc_reg->mbcsv_hdl);

		c_count = 0;
		pwe_hdl = 0;

		while (NULL != (ckpt = (CKPT_INST *)ncs_patricia_tree_getnext(&mbc_reg->ckpt_ssn_list,
									      (const uint8_t *)&pwe_hdl))) {
			c_count++;
			pwe_hdl = ckpt->pwe_hdl;

			printf("\n|               |%8X|%8X| %c |%c|                                  |",
			       ckpt->pwe_hdl, ckpt->ckpt_hdl, (ckpt->warm_sync_on ? 'T' : 'F'),
			       mbcsv_prt_role[ckpt->my_role]);
			printf("\n|               |%16" PRIX64 "|%1d%1d%1d%1d%1d |           MY   PEERS              |",
			       (uint64_t)ckpt->my_anchor, ckpt->in_quiescing, ckpt->peer_up_sent, ckpt->ftm_role_set,
			       ckpt->role_set, ckpt->data_req_sent);

			p_count = 0;
			peer_ptr = ckpt->peer_list;

			while (NULL != peer_ptr) {
				p_count++;
				m_MBCSV_PRT_PEER_INFO(peer_ptr);
				peer_ptr = peer_ptr->next;
			}

			if (p_count == 0)
				printf
				    ("\n|               |                       |           NONE                   |");
			else {
				if (NULL != ckpt->active_peer) {
					printf
					    ("\n|               |                       |           MY ACTIVE PEER         |");
					m_MBCSV_PRT_PEER_INFO(ckpt->active_peer);
				}
			}
			printf("\n|               |-----------------------|----------------------------------|");
		}

		if (c_count == 0)
			printf("\n|               |           NONE        |           NONE                   |");

		printf("\n------------------------------------------------------------------------------");

		m_NCS_UNLOCK(&mbc_reg->svc_lock, NCS_LOCK_READ);
		m_LOG_MBCSV_SVC_LOCK(MBCSV_LK_UNLOCKED, mbc_reg->svc_id, &mbc_reg->svc_lock);
	}

	m_MBCSV_PRT_INV_FOOTER;

	m_NCS_UNLOCK(&mbcsv_cb.global_lock, NCS_LOCK_READ);
	m_LOG_MBCSV_GL_LOCK(MBCSV_LK_UNLOCKED, &mbcsv_cb.global_lock);

	return rc;
}

#endif
