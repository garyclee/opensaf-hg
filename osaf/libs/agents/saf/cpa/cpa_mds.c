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

  This file contains routines used by CPA library for MDS Interface.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "cpa.h"

static uint32_t cpa_mds_enc_flat(CPA_CB *cb, MDS_CALLBACK_ENC_FLAT_INFO *info);
static uint32_t cpa_mds_dec_flat(CPA_CB *cb, MDS_CALLBACK_DEC_FLAT_INFO *info);
static uint32_t cpa_mds_rcv(CPA_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info);
static uint32_t cpa_mds_svc_evt(CPA_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt);
static uint32_t cpa_mds_get_handle(CPA_CB *cb);
static uint32_t cpa_mds_enc(CPA_CB *cb, MDS_CALLBACK_ENC_FLAT_INFO *info);
static uint32_t cpa_mds_dec(CPA_CB *cb, MDS_CALLBACK_DEC_FLAT_INFO *info);

FUNC_DECLARATION(CPSV_EVT);

/* Message Format Verion Tables at CPND */
MDS_CLIENT_MSG_FORMAT_VER cpa_cpnd_msg_fmt_table[CPA_WRT_CPND_SUBPART_VER_RANGE] = {
	1, 2
};

MDS_CLIENT_MSG_FORMAT_VER cpa_cpd_msg_fmt_table[CPA_WRT_CPD_SUBPART_VER_RANGE] = {
	1, 2
};

/****************************************************************************
 * Name          : cpa_mds_get_handle
 *
 * Description   : This function Gets the Handles of local MDS destination
 *
 * Arguments     : cb   : CPA control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

uint32_t cpa_mds_get_handle(CPA_CB *cb)
{
	NCSADA_INFO arg;
	uint32_t rc;

	memset(&arg, 0, sizeof(NCSADA_INFO));
	arg.req = NCSADA_GET_HDLS;
	rc = ncsada_api(&arg);

	if (rc != NCSCC_RC_SUCCESS) {
		/*m_LOG_CPA_HEADLINE(CPA_MDS_GET_HANDLE_FAILED,NCSFL_SEV_ERROR); */
		m_LOG_CPA_CCLL(CPA_API_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "ADA:GET_HDLS", __FILE__, __LINE__,
			       rc);
		return rc;
	}
	cb->cpa_mds_hdl = arg.info.adest_get_hdls.o_mds_pwe1_hdl;
	return rc;
}

/****************************************************************************
  Name          : cpa_mds_register
 
  Description   : This routine registers the CPA Service with MDS.
 
  Arguments     : cpa_cb - ptr to the CPA control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/

uint32_t cpa_mds_register(CPA_CB *cb)
{
	NCSMDS_INFO svc_info;
	MDS_SVC_ID subs_id[2] = { NCSMDS_SVC_ID_CPND, NCSMDS_SVC_ID_CPD };
	uint32_t rc = NCSCC_RC_SUCCESS;

	/* STEP1: Get the MDS Handle */
	if (cpa_mds_get_handle(cb) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/* memset the svc_info */
	memset(&svc_info, 0, sizeof(NCSMDS_INFO));

	/* STEP 2 : Install on ADEST with MDS with service ID NCSMDS_SVC_ID_CPA. */
	svc_info.i_mds_hdl = cb->cpa_mds_hdl;
	svc_info.i_svc_id = NCSMDS_SVC_ID_CPA;
	svc_info.i_op = MDS_INSTALL;

	svc_info.info.svc_install.i_yr_svc_hdl = cb->agent_handle_id;
	svc_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;	/* node specific */
	svc_info.info.svc_install.i_svc_cb = cpa_mds_callback;	/* callback */
	svc_info.info.svc_install.i_mds_q_ownership = false;	/* CPA owns the mds queue */
	svc_info.info.svc_install.i_mds_svc_pvt_ver = CPA_MDS_PVT_SUBPART_VERSION;	/* Private Subpart Version of CPA for Versioning infrastructure */

	if ((rc = ncsmds_api(&svc_info)) != NCSCC_RC_SUCCESS) {
		m_LOG_CPA_CCLL(CPA_API_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "MDS:INSTALL", __FILE__, __LINE__,
			       rc);
		return rc;
	}
	cb->cpa_mds_dest = svc_info.info.svc_install.o_dest;

	/* STEP 3: Subscribe to CPND up/down events */
	svc_info.i_op = MDS_SUBSCRIBE;
	svc_info.info.svc_subscribe.i_num_svcs = 1;
	svc_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
	svc_info.info.svc_subscribe.i_svc_ids = &subs_id[0];

	if ((rc = ncsmds_api(&svc_info)) != NCSCC_RC_SUCCESS) {
		m_LOG_CPA_CCLL(CPA_API_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "MDS:SUBSCRIBE", __FILE__, __LINE__,
			       rc);
		goto error;
	}

	return NCSCC_RC_SUCCESS;

 error:
	/* Uninstall with the mds */
	cpa_mds_unregister(cb);
	return NCSCC_RC_FAILURE;
}

/****************************************************************************
 * Name          : cpa_mds_unreg
 *
 * Description   : This function un-registers the CPA Service with MDS.
 *
 * Arguments     : cb   : CPA control Block pointer.
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void cpa_mds_unregister(CPA_CB *cb)
{
	NCSMDS_INFO arg;
	uint32_t rc = NCSCC_RC_SUCCESS;

	/* Un-install your service into MDS. 
	   No need to cancel the services that are subscribed */
	memset(&arg, 0, sizeof(NCSMDS_INFO));

	arg.i_mds_hdl = cb->cpa_mds_hdl;
	arg.i_svc_id = NCSMDS_SVC_ID_CPA;
	arg.i_op = MDS_UNINSTALL;

	rc = ncsmds_api(&arg);

	if (rc != NCSCC_RC_SUCCESS) {
		m_LOG_CPA_CCLL(CPA_API_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "MDS:UNINSTALL", __FILE__, __LINE__,
			       rc);
	}
	return;
}

/****************************************************************************
  Name          : cpa_mds_callback
 
  Description   : This callback routine will be called by MDS on event arrival
 
  Arguments     : info - pointer to the mds callback info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uint32_t cpa_mds_callback(struct ncsmds_callback_info *info)
{
	CPA_CB *cpa_cb = NULL;
	uint32_t rc = NCSCC_RC_FAILURE;

	if (info == NULL)
		return rc;

	cpa_cb = (CPA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CPA, gl_cpa_hdl);

	if (!cpa_cb) {
		m_LOG_CPA_CCLL(CPA_PROC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "mds_callback:HDL_TAKE", __FILE__,
			       __LINE__, rc);
		return m_LEAP_DBG_SINK(rc);
	}

	switch (info->i_op) {
	case MDS_CALLBACK_COPY:
		rc = NCSCC_RC_FAILURE;
		break;

	case MDS_CALLBACK_ENC_FLAT:
		rc = cpa_mds_enc_flat(cpa_cb, &info->info.enc_flat);
		break;

	case MDS_CALLBACK_DEC_FLAT:
		rc = cpa_mds_dec_flat(cpa_cb, &info->info.dec_flat);
		break;
	case MDS_CALLBACK_RECEIVE:
		rc = cpa_mds_rcv(cpa_cb, &info->info.receive);
		break;

	case MDS_CALLBACK_SVC_EVENT:
		rc = cpa_mds_svc_evt(cpa_cb, &info->info.svc_evt);
		break;

	case MDS_CALLBACK_ENC:
		rc = cpa_mds_enc(cpa_cb, &info->info.enc);
		break;

	case MDS_CALLBACK_DEC:
		rc = cpa_mds_dec(cpa_cb, &info->info.dec);
		break;

	default:
		m_LOG_CPA_CCLL(CPA_PROC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_INFO, "mds_callback:unknown_op", __FILE__,
			       __LINE__, rc);
		break;
	}

	if (rc != NCSCC_RC_SUCCESS) {
		m_LOG_CPA_CCLL(CPA_PROC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_INFO, "mds_callback", __FILE__, __LINE__,
			       rc);
	}

	ncshm_give_hdl(gl_cpa_hdl);

	return rc;
}

/****************************************************************************
  Name          : cpa_mds_enc_flat
 
  Description   : This function encodes an events sent from CPA.
 
  Arguments     : cb    : CPA control Block.
                  enc_info  : Info for encoding
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
static uint32_t cpa_mds_enc_flat(CPA_CB *cb, MDS_CALLBACK_ENC_FLAT_INFO *info)
{
	CPSV_EVT *evt = NULL;
	NCS_UBAID *uba = info->io_uba;
	uint32_t rc = NCSCC_RC_SUCCESS;

	/* Get the Msg Format version from the SERVICE_ID & RMT_SVC_PVT_SUBPART_VERSION */
	if (info->i_to_svc_id == NCSMDS_SVC_ID_CPND) {
		info->o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(info->i_rem_svc_pvt_ver,
							    CPA_WRT_CPND_SUBPART_VER_MIN,
							    CPA_WRT_CPND_SUBPART_VER_MAX, cpa_cpnd_msg_fmt_table);
	}
	if (info->o_msg_fmt_ver) {
		if (info->i_to_svc_id == NCSMDS_SVC_ID_CPND) {
			/* as all the event structures are flat */
			evt = (CPSV_EVT *)info->i_msg;
			rc = cpsv_evt_enc_flat(&cb->edu_hdl, evt, uba);
			if (rc != NCSCC_RC_SUCCESS)
				m_LOG_CPA_CCLL(CPA_PROC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "mds_enc_flat",
					       __FILE__, __LINE__, rc);
			return rc;
		} else {
			m_LOG_CPA_CCLL(CPA_PROC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "mds_enc_flat", __FILE__,
				       __LINE__, rc);
			return NCSCC_RC_FAILURE;
		}
	} else {
		/* Drop The Message */
		m_LOG_CPA_CCLL(CPA_PROC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "mds_enc_flat", __FILE__, __LINE__,
			       rc);
		return NCSCC_RC_FAILURE;
	}
}

/****************************************************************************
  Name          : cpa_mds_dec_flat
 
  Description   : This function decodes an events sent to CPA.
 
  Arguments     : cb    : CPA control Block.
                  dec_info  : Info for decoding
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
static uint32_t cpa_mds_dec_flat(CPA_CB *cb, MDS_CALLBACK_DEC_FLAT_INFO *info)
{
	CPSV_EVT *evt = NULL;
	NCS_UBAID *uba = info->io_uba;
	uint32_t rc = NCSCC_RC_SUCCESS;
	bool is_valid_msg_fmt = false;

	if (info->i_fr_svc_id == NCSMDS_SVC_ID_CPND) {
		is_valid_msg_fmt = m_NCS_MSG_FORMAT_IS_VALID(info->i_msg_fmt_ver,
							     CPA_WRT_CPND_SUBPART_VER_MIN,
							     CPA_WRT_CPND_SUBPART_VER_MAX, cpa_cpnd_msg_fmt_table);
	} else if (info->i_fr_svc_id == NCSMDS_SVC_ID_CPD) {
		is_valid_msg_fmt = m_NCS_MSG_FORMAT_IS_VALID(info->i_msg_fmt_ver,
							     CPA_WRT_CPD_SUBPART_VER_MIN,
							     CPA_WRT_CPD_SUBPART_VER_MAX, cpa_cpd_msg_fmt_table);
	}

	if (is_valid_msg_fmt) {

		if (info->i_fr_svc_id == NCSMDS_SVC_ID_CPND || info->i_fr_svc_id == NCSMDS_SVC_ID_CPD) {
			evt = (CPSV_EVT *)m_MMGR_ALLOC_CPSV_EVT(NCS_SERVICE_ID_CPA);
			if (evt == NULL) {
				m_LOG_CPA_CCLL(CPA_MEM_ALLOC_FAILED, NCSFL_LC_CKPT_MGMT,
					       NCSFL_SEV_ERROR, "mds_dec_flat", __FILE__, __LINE__,
					       NCSCC_RC_OUT_OF_MEM);
				return NCSCC_RC_OUT_OF_MEM;
			}
			info->o_msg = evt;
			rc = cpsv_evt_dec_flat(&cb->edu_hdl, uba, evt);
			return rc;
		} else {
			m_LOG_CPA_CCLL(CPA_PROC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_INFO, "mds_dec_flat", __FILE__,
				       __LINE__, rc);
			return NCSCC_RC_FAILURE;
		}
	} else {
		m_LOG_CPA_CCLL(CPA_PROC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_INFO, "mds_dec_flat", __FILE__, __LINE__,
			       rc);
		return NCSCC_RC_FAILURE;
	}
}

/****************************************************************************
 * Name          : cpa_mds_rcv
 *
 * Description   : MDS will call this function on receiving CPA.
 *
 * Arguments     : cb - CPA Control Block
 *                 rcv_info - MDS Receive information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uint32_t cpa_mds_rcv(CPA_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info)
{
	uint32_t rc = NCSCC_RC_SUCCESS;

	CPSV_EVT *evt = (CPSV_EVT *)rcv_info->i_msg;

	evt->sinfo.ctxt = rcv_info->i_msg_ctxt;
	evt->sinfo.dest = rcv_info->i_fr_dest;
	evt->sinfo.to_svc = rcv_info->i_fr_svc_id;

	/* Process the received event at CPA */
	if (cpa_process_evt(cb, evt) != NCSCC_RC_SUCCESS)
		rc = NCSCC_RC_FAILURE;

	/* Free the Event */
	m_MMGR_FREE_CPSV_EVT(evt, NCS_SERVICE_ID_CPA);

	return rc;
}

/****************************************************************************
 * Name          : cpa_mds_svc_evt
 *
 * Description   : CPA is informed when MDS events occurr that he has 
 *                 subscribed to
 *
 * Arguments     : 
 *   cb          : CPA control Block.
 *   svc_evt    : Svc evt info.
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/

static uint32_t cpa_mds_svc_evt(CPA_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt)
{
    SaCkptCheckpointHandleT  prev_ckpt_id=0;
    CPA_GLOBAL_CKPT_NODE *gc_node = NULL;
    uint32_t proc_rc = 0, i = 0, no_of_nodes = 0;
    uint32_t counter=cb->gbl_ckpt_tree.n_nodes;
    CPSV_EVT send_evt;
    CPSV_REF_CNT   ref_cnt_array[100];

	/* TBD: The CPND and CPD restarts are to be implemented post April release */
	switch (svc_evt->i_change) {
	case NCSMDS_DOWN:
		switch (svc_evt->i_svc_id) {
		case NCSMDS_SVC_ID_CPND:
			if (m_NCS_GET_NODE_ID == m_NCS_NODE_ID_FROM_MDS_DEST(svc_evt->i_dest)) {
				m_NCS_LOCK(&cb->cpnd_sync_lock, NCS_LOCK_WRITE);
				cb->is_cpnd_up = false;
				m_NCS_UNLOCK(&cb->cpnd_sync_lock, NCS_LOCK_WRITE);
			}
			break;
		default:
			break;
		}

		break;
	case NCSMDS_UP:
		switch (svc_evt->i_svc_id) {
		case NCSMDS_SVC_ID_CPND:

			/* get the node_id and compare with the node_id of the mdest */
			if (m_NCS_GET_NODE_ID == m_NCS_NODE_ID_FROM_MDS_DEST(svc_evt->i_dest)) {
				m_NCS_LOCK(&cb->cpnd_sync_lock, NCS_LOCK_WRITE);
				cb->is_cpnd_up = true;
				cb->cpnd_mds_dest = svc_evt->i_dest;
				if (cb->cpnd_sync_awaited == true) {
					m_NCS_SEL_OBJ_IND(cb->cpnd_sync_sel);
				}
				m_NCS_UNLOCK(&cb->cpnd_sync_lock, NCS_LOCK_WRITE);
           /* Get the First Node */
           gc_node = (CPA_GLOBAL_CKPT_NODE *)ncs_patricia_tree_getnext(&cb->gbl_ckpt_tree,
                                           (uint8_t*)&prev_ckpt_id);
           if(gc_node) 
           {
            for(i=0;i<counter;i++) 
             {
                prev_ckpt_id = gc_node->gbl_ckpt_hdl;
                if(gc_node->ref_cnt > 1)
                {
              	  ref_cnt_array[no_of_nodes].ckpt_ref_cnt= gc_node->ref_cnt - 1;
              	  ref_cnt_array[no_of_nodes].ckpt_id = gc_node->gbl_ckpt_hdl;
                  no_of_nodes++;
                }
                gc_node = (CPA_GLOBAL_CKPT_NODE *)ncs_patricia_tree_getnext(&cb->gbl_ckpt_tree,
                                                        (uint8_t*)&prev_ckpt_id);
             }  

               memset(&send_evt, 0, sizeof(CPSV_EVT));
               send_evt.type = CPSV_EVT_TYPE_CPND;
               send_evt.info.cpnd.type = CPND_EVT_A2ND_CKPT_REFCNTSET;
               memcpy(send_evt.info.cpnd.info.refCntsetReq.ref_cnt_array,ref_cnt_array,no_of_nodes*sizeof(CPSV_REF_CNT));
           
               send_evt.info.cpnd.info.refCntsetReq.no_of_nodes = no_of_nodes;
         
               proc_rc = cpa_mds_msg_send(cb->cpa_mds_hdl,&cb->cpnd_mds_dest,&send_evt,NCSMDS_SVC_ID_CPND);
    
               switch (proc_rc)
               {
                 case NCSCC_RC_SUCCESS:
                      break;
                 case NCSCC_RC_REQ_TIMOUT:
                      m_LOG_CPA_CCLLFF(CPA_API_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR,
                                               "active_ckpt_info_bcast :MDS", __FILE__ ,__LINE__, proc_rc ,0, cb->cpnd_mds_dest);
                      break;
                 default:
                      m_LOG_CPA_CCLLFF(CPA_API_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR,
                                                "active_ckpt_info_bcast:MDS", __FILE__ ,__LINE__, proc_rc ,0, cb->cpnd_mds_dest);
                      break;
               }
           } 
         }
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : cpa_mds_enc

  Description   : This function encodes an events sent from CPA to remote CPND.

  Arguments     : cb    : CPA control Block.
                  info  : Info for encoding

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         : None.
******************************************************************************/
static uint32_t cpa_mds_enc(CPA_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info)
{
	CPSV_EVT *pevt = NULL;
	EDU_ERR ederror = 0;
	NCS_UBAID *io_uba = enc_info->io_uba;
	uint32_t rc = NCSCC_RC_SUCCESS;
	uint8_t *pstream = NULL;

	/* Get the Msg Format version from the SERVICE_ID & RMT_SVC_PVT_SUBPART_VERSION */
	if (enc_info->i_to_svc_id == NCSMDS_SVC_ID_CPND) {
		enc_info->o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(enc_info->i_rem_svc_pvt_ver,
								CPA_WRT_CPND_SUBPART_VER_MIN,
								CPA_WRT_CPND_SUBPART_VER_MAX, cpa_cpnd_msg_fmt_table);
	}
	if (enc_info->o_msg_fmt_ver) {
		pevt = (CPSV_EVT *)enc_info->i_msg;
		if (pevt->type == CPSV_EVT_TYPE_CPND) {
			if (pevt->info.cpnd.type == CPND_EVT_A2ND_CKPT_WRITE) {
				pstream = ncs_enc_reserve_space(io_uba, 12);
				if (!pstream)
					return m_CPSV_DBG_SINK(NCSCC_RC_FAILURE,
							       "Memory alloc failed in cpa_mds_enc \n");
				ncs_encode_32bit(&pstream, pevt->type);	/* CPSV_EVT Type */
				ncs_encode_32bit(&pstream, pevt->info.cpnd.error);	/* cpnd_evt error This is for backword compatible purpose with EDU enc/dec with 3.0.2 */
				ncs_encode_32bit(&pstream, pevt->info.cpnd.type);	/* cpnd_evt SubType */
				ncs_enc_claim_space(io_uba, 12);

				rc = cpsv_ckpt_access_encode(&pevt->info.cpnd.info.ckpt_write, io_uba);
				return rc;
			} else if (pevt->info.cpnd.type == CPND_EVT_A2ND_CKPT_READ) {
				pstream = ncs_enc_reserve_space(io_uba, 12);
				if (!pstream)
					return m_CPSV_DBG_SINK(NCSCC_RC_FAILURE,
							       "Memory alloc failed in cpa_mds_enc \n");
				ncs_encode_32bit(&pstream, pevt->type);	/* CPSV_EVT Type */
				ncs_encode_32bit(&pstream, pevt->info.cpnd.error);	/* cpnd_evt error This is for backword compatible purpose with EDU enc/dec with 3.0.2 */
				ncs_encode_32bit(&pstream, pevt->info.cpnd.type);	/* cpnd_evt SubType */
				ncs_enc_claim_space(io_uba, 12);

				rc = cpsv_ckpt_access_encode(&pevt->info.cpnd.info.ckpt_read, io_uba);
				return rc;
			}
         else  if(pevt->info.cpnd.type == CPND_EVT_A2ND_CKPT_REFCNTSET)
         {
             if(enc_info->o_msg_fmt_ver < 2)
                return NCSCC_RC_FAILURE;
             else
                {
		  pstream = ncs_enc_reserve_space(io_uba, 12);
                  if(!pstream)
                     return m_CPSV_DBG_SINK(NCSCC_RC_FAILURE,"Memory alloc failed in cpa_mds_enc \n");
                  ncs_encode_32bit(&pstream , pevt->type);              
                  ncs_encode_32bit(&pstream , pevt->info.cpnd.error);  
                  ncs_encode_32bit(&pstream , pevt->info.cpnd.type);         
                  ncs_enc_claim_space(io_uba, 12);

                  rc = cpsv_ref_cnt_encode(io_uba,&pevt->info.cpnd.info.refCntsetReq);
                  return rc; 
                }
         }
      }  /* For all other cases call EDU othen than Write/Read API's */
		rc = m_NCS_EDU_EXEC(&cb->edu_hdl, FUNC_NAME(CPSV_EVT),
				    enc_info->io_uba, EDP_OP_TYPE_ENC, pevt, &ederror);
		return rc;
	} else {
		/* Drop The Message As Msg Fmt Version Not understandable */
		m_LOG_CPA_CCL(CPA_PROC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "mds_enc", __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
}

/****************************************************************************
  Name          : cpa_mds_dec

  Description   : This function decodes an events sent to CPA.

  Arguments     : cb    : CPA control Block.
                  info  : Info for decoding

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         : None.
******************************************************************************/
static uint32_t cpa_mds_dec(CPA_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info)
{
	CPSV_EVT *msg_ptr = NULL;
	EDU_ERR ederror = 0;
	uint32_t rc = NCSCC_RC_SUCCESS;
	uint8_t local_data[20];
	uint8_t *pstream;
	bool is_valid_msg_fmt = false;

	if (dec_info->i_fr_svc_id == NCSMDS_SVC_ID_CPND) {
		is_valid_msg_fmt = m_NCS_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
							     CPA_WRT_CPND_SUBPART_VER_MIN,
							     CPA_WRT_CPND_SUBPART_VER_MAX, cpa_cpnd_msg_fmt_table);
	} else if (dec_info->i_fr_svc_id == NCSMDS_SVC_ID_CPD) {
		is_valid_msg_fmt = m_NCS_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
							     CPA_WRT_CPD_SUBPART_VER_MIN,
							     CPA_WRT_CPD_SUBPART_VER_MAX, cpa_cpd_msg_fmt_table);
	}

	if (is_valid_msg_fmt) {

		msg_ptr = m_MMGR_ALLOC_CPSV_EVT(NCS_SERVICE_ID_CPA);
		if (!msg_ptr)
			return NCSCC_RC_FAILURE;

		memset(msg_ptr, 0, sizeof(CPSV_EVT));
		dec_info->o_msg = (NCSCONTEXT)msg_ptr;
		pstream = ncs_dec_flatten_space(dec_info->io_uba, local_data, 8);
		msg_ptr->type = ncs_decode_32bit(&pstream);
		if (msg_ptr->type == CPSV_EVT_TYPE_CPA) {
			msg_ptr->info.cpa.type = ncs_decode_32bit(&pstream);
			if (msg_ptr->info.cpa.type == CPA_EVT_ND2A_CKPT_DATA_RSP) {
				ncs_dec_skip_space(dec_info->io_uba, 8);
				rc = cpsv_data_access_rsp_decode(&msg_ptr->info.cpa.info.sec_data_rsp,
								 dec_info->io_uba);
				goto free;

			}
			/* if(msg_ptr->info.cpa.type == CPA_EVT_ND2A_CKPT_DATA_RSP) */
		}
		/* if( msg_ptr->type == CPSV_EVT_TYPE_CPA) */
		/* For all Other Cases Other Than CPA( Read / Write Rsp Follow EDU rules */
		rc = m_NCS_EDU_EXEC(&cb->edu_hdl, FUNC_NAME(CPSV_EVT),
				    dec_info->io_uba, EDP_OP_TYPE_DEC, (CPSV_EVT **)&dec_info->o_msg, &ederror);
 free:
		if (rc != NCSCC_RC_SUCCESS) {
			m_MMGR_FREE_CPSV_EVT(dec_info->o_msg, NCS_SERVICE_ID_CPA);
		}
		return rc;
	} else {
		m_LOG_CPA_CCL(CPA_PROC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_INFO, "mds_dec", __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
}

/****************************************************************************
  Name          : cpa_mds_msg_sync_send
 
  Description   : This routine sends the CPA message to CPND.
 
  Arguments     :
                  uint32_t      cpa_mds_hdl Handle of CPA
                  MDS_DEST  *destination - destintion to send to
                  CPSV_EVT   *i_evt - CPSV_EVT pointer
                  CPSV_EVT   **o_evt - CPSV_EVT pointer to result data
                  timeout - timeout value in 10 ms 
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uint32_t cpa_mds_msg_sync_send(uint32_t cpa_mds_hdl, MDS_DEST *destination, CPSV_EVT *i_evt, CPSV_EVT **o_evt, uint32_t timeout)
{

	NCSMDS_INFO mds_info;
	uint32_t rc;

	if (!i_evt)
		return NCSCC_RC_FAILURE;

	memset(&mds_info, 0, sizeof(NCSMDS_INFO));
	mds_info.i_mds_hdl = cpa_mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_CPA;
	mds_info.i_op = MDS_SEND;

	/* fill the send structure */
	mds_info.info.svc_send.i_msg = (NCSCONTEXT)i_evt;
	mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
	mds_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_CPND;
	mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SNDRSP;

	/* fill the send rsp strcuture */
	mds_info.info.svc_send.info.sndrsp.i_time_to_wait = timeout;	/* timeto wait in 10ms */
	mds_info.info.svc_send.info.sndrsp.i_to_dest = *destination;

	/* send the message */
	rc = ncsmds_api(&mds_info);

	if (rc == NCSCC_RC_SUCCESS)
		*o_evt = mds_info.info.svc_send.info.sndrsp.o_rsp;

	return rc;
}

/****************************************************************************
  Name          : cpa_mds_msg_send
 
  Description   : This routine sends the CPA message to CPND.
 
  Arguments     : uint32_t cpa_mds_hdl Handle of CPA
                  MDS_DEST  *destination - destintion to send to
                  CPSV_EVT   *i_evt - CPSV_EVT pointer
                  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uint32_t cpa_mds_msg_send(uint32_t cpa_mds_hdl, MDS_DEST *destination, CPSV_EVT *i_evt, uint32_t to_svc)
{

	NCSMDS_INFO mds_info;
	uint32_t rc;

	if (!i_evt)
		return NCSCC_RC_FAILURE;

	memset(&mds_info, 0, sizeof(NCSMDS_INFO));
	mds_info.i_mds_hdl = cpa_mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_CPA;
	mds_info.i_op = MDS_SEND;

	/* fill the send structure */
	mds_info.info.svc_send.i_msg = (NCSCONTEXT)i_evt;
	mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
	mds_info.info.svc_send.i_to_svc = to_svc;
	mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;

	/* fill the send rsp strcuture */
	mds_info.info.svc_send.info.snd.i_to_dest = *destination;

	/* send the message */
	rc = ncsmds_api(&mds_info);

	return rc;
}
