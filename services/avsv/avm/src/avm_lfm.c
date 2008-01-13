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
 
  DESCRIPTION:     This module deals with messages involved in the interface 
                   between AvM and LFM.
  ..............................................................................

  Function Included in this Module:

  avm_lfm_mds_msg_send        -
  avm_lfm_mds_msg_bcast       -
  avm_lfm_mds_sync_send       -
  avm_send_lfm_fru_info       -
  avm_send_lfm_nd_reset_ind   -
  avm_upd_hlt_stat_from_lfm   -
  avm_lfm_resp_hlt_status     -
  avm_lfm_node_reset_ind      -
  avm_lfm_msg_handler         -

******************************************************************************
*/

#if (MOT_ATCA == 1)
#include "avm.h"

/*****************************************************************************
 * Function:  avm_lfm_mds_msg_send
 *
 * Purpose:   This function is a non-blocking async MDS message sent to LFM
 *
 * Input:     avm_cb   -  AVM Control Block
 *            msg      -  Message that need to be sent
 *            dest     -  MDS Destination address of LFM
 *            prio     -  MDS send Priority
 *
 * Returns:   NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static uns32
avm_lfm_mds_msg_send(AVM_CB_T *avm_cb, LFM_AVM_MSG *msg,
                     MDS_DEST  *dest, MDS_SEND_PRIORITY_TYPE prio)
{
    NCSMDS_INFO   mds_info;
    MDS_SEND_INFO *send_info = &mds_info.info.svc_send;
    uns32         rc = NCSCC_RC_SUCCESS;
    MDS_SENDTYPE_SND_INFO *send;

    m_AVM_LOG_FUNC_ENTRY("avm_lfm_mds_msg_send");

    /* populate the mds params */
    m_NCS_MEMSET(&mds_info, '\0', sizeof(NCSMDS_INFO));

    mds_info.i_mds_hdl = avm_cb->vaddr_pwe_hdl;
    mds_info.i_svc_id = NCSMDS_SVC_ID_AVM;
    mds_info.i_op = MDS_SEND;

    send_info->i_msg    = msg;
    send_info->i_to_svc = NCSMDS_SVC_ID_LFM;
    send_info->i_priority = prio;

    send = &send_info->info.snd;
    send_info->i_sendtype = MDS_SENDTYPE_SND;
    send->i_to_dest = *dest;

    /* send the message */
    rc = ncsmds_api(&mds_info);

    return rc;
}

/*****************************************************************************
 * Function:    avm_lfm_mds_msg_bcast
 *
 * Purpose:     This routine sends a Broadcast MDS message to LFM.
 *
 * Input:       avm_cb   - AVM Control Block
 *              msg      - Message to be broadcast.
 *
 * Returns:     NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static uns32
avm_lfm_mds_msg_bcast(AVM_CB_T *avm_cb, LFM_AVM_MSG *msg)
{
    NCSMDS_INFO   mds_info;
    MDS_SEND_INFO *send_info = &mds_info.info.svc_send;
    uns32         rc = NCSCC_RC_SUCCESS;
    MDS_SENDTYPE_BCAST_INFO *bcast;

    m_AVM_LOG_FUNC_ENTRY("avm_lfm_mds_msg_bcast");
    /* populate the mds params */
    m_NCS_MEMSET(&mds_info, '\0', sizeof(NCSMDS_INFO));

    mds_info.i_mds_hdl = avm_cb->vaddr_pwe_hdl;
    mds_info.i_svc_id = NCSMDS_SVC_ID_AVM;
    mds_info.i_op = MDS_SEND;

    send_info->i_msg    = msg;
    send_info->i_to_svc = NCSMDS_SVC_ID_LFM;
    send_info->i_priority = MDS_SEND_PRIORITY_MEDIUM;
    send_info->i_sendtype = MDS_SENDTYPE_BCAST;

    bcast = &send_info->info.bcast;
    bcast->i_bcast_scope = NCSMDS_SCOPE_NONE;

    /* send the message */
    rc = ncsmds_api(&mds_info);

    return rc;
}

/*****************************************************************************
 * Function:    avm_lfm_mds_sync_send
 *
 * Purpose:     This routine sends a SYNC MDS message to LFM.
 *
 * Input:       avm_cb   -  AVM Control Block
 *              msg      -  Message to be sent to LFM
 *              dest     -  MDS Destination Address of LFM
 *              mds_ctxt -  The mds_ctxt if responding to a sync call.
 *         
 *
 * Returns:     NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static uns32
avm_lfm_mds_sync_send(AVM_CB_T *avm_cb, LFM_AVM_MSG *msg,
                       MDS_DEST *dest, MDS_SYNC_SND_CTXT *mds_ctxt)
{
    NCSMDS_INFO   mds_info;
    MDS_SENDTYPE_RSP_INFO *resp = NULL;
    MDS_SEND_INFO *send_info = &mds_info.info.svc_send;
    uns32         rc = NCSCC_RC_SUCCESS;

    m_AVM_LOG_FUNC_ENTRY("avm_lfm_mds_sync_send");
    /* populate the mds params */
    m_NCS_MEMSET(&mds_info, '\0', sizeof(NCSMDS_INFO));

    mds_info.i_mds_hdl = avm_cb->vaddr_pwe_hdl;
    mds_info.i_svc_id = NCSMDS_SVC_ID_AVM;
    mds_info.i_op = MDS_SEND;

    send_info->i_msg    = msg;
    send_info->i_to_svc = NCSMDS_SVC_ID_LFM;
    send_info->i_priority = MDS_SEND_PRIORITY_MEDIUM;

    if(mds_ctxt)
    {
       resp = &send_info->info.rsp;
       send_info->i_sendtype = MDS_SENDTYPE_RSP;
       resp->i_sender_dest = *dest;
       resp->i_msg_ctxt = *mds_ctxt;
    }
    else
    {
       return NCSCC_RC_FAILURE;
    }

    /* send the message */
    rc = ncsmds_api(&mds_info);

    return rc;
}


/*****************************************************************************
 * Function:    avm_send_lfm_fru_info
 *
 * Purpose:     This function packages and sends the snapshot of
 *              FRU information in the system.
 *
 * Input:       avm_cb   -   AVM Control Block 
 *
 * Returns:     NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static uns32 avm_send_lfm_fru_info(AVM_CB_T *avm_cb, MDS_DEST dest)
{
   /* allocate message structure and fill the message and send */
   LFM_AVM_MSG *msg=NULL;
   AVM_ENT_INFO_T *ent_info = AVM_ENT_INFO_NULL;
   AVM_LFM_FRU_HSSTATE_INFO *p_info=NULL, **prev_info=NULL;   
   uns32         rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_send_lfm_fru_info");

   if((msg = m_MMGR_ALLOC_LFM_AVM_MSG) == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   m_NCS_OS_MEMSET(msg, 0, sizeof(LFM_AVM_MSG));

   msg->msg_type = AVM_LFM_FRU_HSSTATUS;
   prev_info = &msg->info.fru_hsstate_info;

   ent_info = avm_find_first_ent_info(avm_cb);

   while(ent_info != AVM_ENT_INFO_NULL)
   {
      if((p_info = m_MMGR_ALLOC_LFM_AVM_FRU_HSSTATE_INFO) == NULL)
      {
         lfm_avm_msg_free(msg);
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
      }
      m_NCS_OS_MEMSET(p_info, 0, sizeof(AVM_LFM_FRU_HSSTATE_INFO));
      
      m_NCS_MEMCPY(&p_info->fru, &ent_info->entity_path, 
                  sizeof(SaHpiEntityPathT));

      p_info->hs_state = avm_map_hs_to_hpi_hs(ent_info->current_state);

      p_info->next = NULL;

      *prev_info = p_info;
      prev_info = &p_info->next;

      ent_info = avm_find_next_ent_info(avm_cb, &ent_info->entity_path);
   }

   rc = avm_lfm_mds_msg_send(avm_cb, msg, &dest, MDS_SEND_PRIORITY_MEDIUM);

   /* Free the memory */
   lfm_avm_msg_free(msg);
   return rc;
}

/*****************************************************************************
 * Function:   avm_send_lfm_nd_reset_ind
 *
 * Purpose:   This is function interface to send a node reset indication to LFM 
 *
 * Input:     avm_cb   -   AVM Control Block
 *            ent_path -   SAF specified HPI entity Path
 *
 * Returns:   NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
 
EXTERN_C uns32 
avm_send_lfm_nd_reset_ind(AVM_CB_T *avm_cb, SaHpiEntityPathT *ent_path)
{
   /* allocate message structure and fill the message and send */
   LFM_AVM_MSG *msg=NULL;
   uns32         rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_send_lfm_nd_reset_ind");

   if((msg = m_MMGR_ALLOC_LFM_AVM_MSG ) == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   m_NCS_OS_MEMSET(msg, 0, sizeof(LFM_AVM_MSG));

   msg->msg_type = AVM_LFM_NODE_RESET_IND;

   m_NCS_MEMCPY(&msg->info.node_reset_ind, ent_path, 
               sizeof(SaHpiEntityPathT));

   rc = avm_lfm_mds_msg_bcast(avm_cb, msg);

   /* Free the memory */
   lfm_avm_msg_free(msg);

   return rc;
}

/*****************************************************************************
 * Function:   avm_upd_hlt_stat_from_lfm
 *
 * Purpose:    This routine will update the health status of the plane/sam
 *             as informed by the LFM.
 *
 * Input:      avm_cb   -  AVM Control Block 
 *             info     -  Health status update info from the MDS message
 *
 * Returns:    NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static uns32 
avm_upd_hlt_stat_from_lfm(AVM_CB_T *avm_cb,
                          HEALTH_STATUS_UPDATE_INFO *info)
{
   m_AVM_LOG_FUNC_ENTRY("avm_upd_hlt_stat_from_lfm");

   if(!info)
   {
      /* LOG error */
      return NCSCC_RC_FAILURE;
   }

   if(info->plane_status != STATUS_NONE)
      avm_cb->hlt_status[info->plane] = info->plane_status;

   if(info->sam_status != STATUS_NONE)
      avm_cb->hlt_status[info->sam] = info->sam_status;

   m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, avm_cb->hlt_status, 
                                   AVM_CKPT_HLT_STATUS);
#if 0
   /*Introduce this at later point when we have time to test */
   /* Undo the changes in avm_dblog.h and avm_logstr.c */
   /* Now log the status for debugging purposes */
   if(avm_cb->hlt_status[HEALTH_STATUS_PLANE_A] == STATUS_HEALTHY)
      m_AVM_LOG_RDE(AVM_LOG_RDE_PLANE_X, AVM_LOG_RDE_HEALTH_GOOD, NCSFL_SEV_NOTICE);
   else
      m_AVM_LOG_RDE(AVM_LOG_RDE_PLANE_X, AVM_LOG_RDE_HEALTH_BAD, NCSFL_SEV_NOTICE);

   if(avm_cb->hlt_status[HEALTH_STATUS_SAM_A] == STATUS_HEALTHY)
      m_AVM_LOG_RDE(AVM_LOG_RDE_SAM_X, AVM_LOG_RDE_HEALTH_GOOD, NCSFL_SEV_NOTICE);
   else
      m_AVM_LOG_RDE(AVM_LOG_RDE_SAM_X, AVM_LOG_RDE_HEALTH_BAD, NCSFL_SEV_NOTICE);

   if(avm_cb->hlt_status[HEALTH_STATUS_PLANE_B] == STATUS_HEALTHY)
      m_AVM_LOG_RDE(AVM_LOG_RDE_PLANE_Y, AVM_LOG_RDE_HEALTH_GOOD, NCSFL_SEV_NOTICE);
   else
      m_AVM_LOG_RDE(AVM_LOG_RDE_PLANE_Y, AVM_LOG_RDE_HEALTH_BAD, NCSFL_SEV_NOTICE);

   if(avm_cb->hlt_status[HEALTH_STATUS_SAM_B] == STATUS_HEALTHY)
      m_AVM_LOG_RDE(AVM_LOG_RDE_SAM_B, AVM_LOG_RDE_HEALTH_GOOD, NCSFL_SEV_NOTICE);
   else
      m_AVM_LOG_RDE(AVM_LOG_RDE_SAM_B, AVM_LOG_RDE_HEALTH_BAD, NCSFL_SEV_NOTICE);
#endif

   m_NCS_DBG_PRINTF("Plane status as reported by lfm\n");
   m_NCS_DBG_PRINTF("Health Status of PLANE_A : \n SAM: %d,  PLANE: %d\n", avm_cb->hlt_status[HEALTH_STATUS_SAM_A], avm_cb->hlt_status[HEALTH_STATUS_PLANE_A]);
   m_NCS_DBG_PRINTF("Health Status of PLANE_B : \n SAM: %d,  PLANE: %d\n", avm_cb->hlt_status[HEALTH_STATUS_SAM_B], avm_cb->hlt_status[HEALTH_STATUS_PLANE_B]);
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function:  avm_lfm_resp_hlt_status
 *
 * Purpose:   This message is the response to the health status query 
 *            from LFM. LFM sends a sync MDS message and hence send sync resp
 *
 * Input:     avm_cb   -  AVM Control Block
 *            ent      -  The entity for which the health status is responded
 *            dest     -  MDS Destination Address of sender
 *            mds_ctxt -  MDS context needed for the sync rsp
 *
 * Returns:   NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static uns32 
avm_lfm_resp_hlt_status(AVM_CB_T *avm_cb, 
                        HEALTH_STATUS_ENTITY ent,
                        MDS_DEST *dest, 
                        MDS_SYNC_SND_CTXT *mds_ctxt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   LFM_AVM_MSG *msg=NULL;

   m_AVM_LOG_FUNC_ENTRY("avm_lfm_resp_hlt_status");

   if((msg = m_MMGR_ALLOC_LFM_AVM_MSG) == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   m_NCS_OS_MEMSET(msg, 0, sizeof(LFM_AVM_MSG));

   msg->msg_type = AVM_LFM_HEALTH_STATUS_RSP;

   switch(ent)
   {
      case HEALTH_STATUS_PLANE_A_SAM_A:
         msg->info.hlt_status_rsp.sam = HEALTH_STATUS_SAM_A;
         msg->info.hlt_status_rsp.sam_status = avm_cb->hlt_status[HEALTH_STATUS_SAM_A];

      case HEALTH_STATUS_PLANE_A:
         msg->info.hlt_status_rsp.plane = HEALTH_STATUS_PLANE_A;
         msg->info.hlt_status_rsp.plane_status = avm_cb->hlt_status[HEALTH_STATUS_PLANE_A];
         break;

      case HEALTH_STATUS_PLANE_B_SAM_B:
         msg->info.hlt_status_rsp.sam = HEALTH_STATUS_SAM_B;
         msg->info.hlt_status_rsp.sam_status = avm_cb->hlt_status[HEALTH_STATUS_SAM_B];
      case HEALTH_STATUS_PLANE_B:
         msg->info.hlt_status_rsp.plane = HEALTH_STATUS_PLANE_B;
         msg->info.hlt_status_rsp.plane_status = avm_cb->hlt_status[HEALTH_STATUS_PLANE_B];
         break;

      case HEALTH_STATUS_SAM_A:
         msg->info.hlt_status_rsp.sam = HEALTH_STATUS_SAM_A;
         msg->info.hlt_status_rsp.sam_status = avm_cb->hlt_status[HEALTH_STATUS_SAM_A];
         break;
      case HEALTH_STATUS_SAM_B:
         msg->info.hlt_status_rsp.sam = HEALTH_STATUS_SAM_B;
         msg->info.hlt_status_rsp.sam_status = avm_cb->hlt_status[HEALTH_STATUS_SAM_B];
         break;
      default:
         /* TBD log an error */
         return NCSCC_RC_FAILURE;
   }

   m_NCS_DBG_PRINTF("Plane status as reported to lfm\n");
   m_NCS_DBG_PRINTF("Health Status of PLANE_A : \n SAM: %d,  PLANE: %d\n", avm_cb->hlt_status[HEALTH_STATUS_SAM_A], avm_cb->hlt_status[HEALTH_STATUS_PLANE_A]);
   m_NCS_DBG_PRINTF("Health Status of PLANE_B : \n SAM: %d,  PLANE: %d\n", avm_cb->hlt_status[HEALTH_STATUS_SAM_B], avm_cb->hlt_status[HEALTH_STATUS_PLANE_B]);
   

   rc = avm_lfm_mds_sync_send(avm_cb, msg, dest, mds_ctxt);

   /* Free the memory */
   lfm_avm_msg_free(msg);

   return rc;

                            
}

/*****************************************************************************
 * Function:  avm_lfm_node_reset_ind
 *
 * Purpose:  
 *
 * Input:   
 *          
 *
 * Returns: 
 *
 * NOTES: This is the function invoked in the context of receving a node 
 *        reset message from LFM.
 * 
 **************************************************************************/
static uns32
avm_lfm_node_reset_ind(AVM_CB_T *avm_cb, SaHpiEntityPathT *ep)
{
   AVM_ENT_INFO_T *ent_info;

   m_AVM_LOG_FUNC_ENTRY("avm_lfm_node_reset_ind");

   if(AVM_ENT_INFO_NULL == (ent_info = avm_find_ent_info(avm_cb, ep)))
   {
      /* LOG error */
      return NCSCC_RC_FAILURE;
   }

   /* All we have to do here is just relay it to AvD */
   avm_avd_node_reset_resp(avm_cb, AVM_NODE_RESET_SUCCESS, ent_info->node_name);

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function:    avm_lfm_msg_handler
 *
 * Purpose:     This is the main routine which looks into the message type
 *              and calls the corresponding handler function.
 *
 * Input:       avm_cb  -  AVM control Block
 *              evt     -  Event filled in the MDS message rcv ctxt.
 *
 * Returns:   NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
EXTERN_C uns32 
avm_lfm_msg_handler(AVM_CB_T *avm_cb, AVM_EVT_T *evt)
{
   uns32  rc=NCSCC_RC_SUCCESS;
   LFM_AVM_MSG *msg=NULL;

   m_AVM_LOG_FUNC_ENTRY("avm_lfm_msg_handler");

   if(evt == NULL) 
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   if(evt->src != AVM_EVT_LFM) 
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   if((msg = evt->evt.lfm_evt.msg ) == NULL)
      return NCSCC_RC_FAILURE;

   switch(msg->msg_type)
   {
   case LFM_AVM_HEALTH_STATUS_REQ:
      rc = avm_lfm_resp_hlt_status(avm_cb, msg->info.hlt_status_req, 
                                 &evt->evt.lfm_evt.fr_dest, &evt->evt.lfm_evt.mds_ctxt);
      break;

   case LFM_AVM_HEALTH_STATUS_UPD:
      rc = avm_upd_hlt_stat_from_lfm(avm_cb, &msg->info.hlt_status_upd);
      break;

   case LFM_AVM_HEALTH_ALL_FRU_HS_REQ:
   {
      /* if we got this request we are on platform else PC */
      avm_cb->is_platform = TRUE;
      rc = avm_send_lfm_fru_info(avm_cb, evt->evt.lfm_evt.fr_dest);
      break;
   }

   case LFM_AVM_NODE_RESET_IND:
      rc = avm_lfm_node_reset_ind(avm_cb, &msg->info.node_reset_ind);
      break;

   case LFM_AVM_SWOVER_REQ:
   {
      if(avm_cb->adm_switch == TRUE)
      {
         m_AVM_LOG_INVALID_VAL_FATAL(avm_cb->adm_switch);
         rc = NCSCC_RC_FAILURE;
      }
      else
      {
         /* Treat this as Admin switch and process */
         avm_cb->adm_switch = TRUE;

         /* First AvM informs RDE about role switchover 
          * AvM should inform RDE to go quiesced. But the current implementation
          * of RDE takes the role and will not send any asyn ack back and hence
          * proceed with the role switchover of the AVD also
          */

         if(NCSCC_RC_SUCCESS != (rc = avm_notify_rde_set_role(avm_cb, 
                                                           SA_AMF_HA_QUIESCED)))
         {
            m_NCS_DBG_PRINTF("\n rde_set->Quiesced Failed on Active SCXB, hence cancelling switch Triggered by LFM");
            m_AVM_LOG_INVALID_VAL_FATAL(rc);
            avm_cb->adm_switch =  FALSE;
            avm_adm_switch_trap(avm_cb, ncsAvmSwitch_ID, AVM_SWITCH_FAILURE);
            return rc;
         }

         rc = avm_avd_role(avm_cb, SA_AMF_HA_QUIESCED, AVM_ADMIN_SWITCH_OVER);
      }
      break;
   }

   default:
      break;
   }

   return rc;
}

#endif /* MOT_ATCA */
