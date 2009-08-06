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

  This file contains routines for interfacing with AvD. It includes the event
  handlers for the messages received from AvD (except those that influence 
  the SU SAF states).
 
..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

#include "avnd.h"

static uns32 avnd_evt_avd_hlt_updt_on_fover (AVND_CB *cb, AVND_EVT *evt);

/* macro to push the AvD msg parameters (to the end of the list) */
#define m_AVND_DIQ_REC_PUSH(cb, rec) \
{ \
   AVND_DND_LIST *list = &((cb)->dnd_list); \
   if (!(list->head)) \
       list->head = (rec); \
   else \
      list->tail->next = (rec); \
   list->tail = (rec); \
}

/* macro to pop the record (from the beginning of the list) */
#define m_AVND_DIQ_REC_POP(cb, o_rec) \
{ \
   AVND_DND_LIST *list = &((cb)->dnd_list); \
   if (list->head) { \
      (o_rec) = list->head; \
      list->head = (o_rec)->next; \
      (o_rec)->next = 0; \
      if (list->tail == (o_rec)) list->tail = 0; \
   } else (o_rec) = 0; \
}


/****************************************************************************
  Name          : avnd_evt_avd_hlt_updt_on_fover
 
  Description   : This routine processes the hlt updates sent by AVD on
                  fail over.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
static uns32 avnd_evt_avd_hlt_updt_on_fover (AVND_CB *cb, AVND_EVT *evt)
{
   AVSV_D2N_REG_HLT_MSG_INFO *info = 0;
   AVSV_HLT_INFO_MSG         *hc_info = 0;
   AVND_HC                   *hc = 0;
   uns32                     rc = NCSCC_RC_SUCCESS;
   AVSV_HLT_KEY      hc_key;

   m_AVND_LOG_FOVER_EVTS(NCSFL_SEV_NOTICE, 
      AVND_LOG_FOVR_HLT_UPDT , NCSCC_RC_SUCCESS);

   info = &evt->info.avd->msg_info.d2n_reg_hlt;

   /* scan the hc list & add each hc to hc-db */
   for (hc_info = info->hlt_list; hc_info; hc = 0, hc_info = hc_info->next)
   {
      /* Check whether this health check record exists. If no then create it */
      if (NULL == (hc = m_AVND_HCDB_REC_GET(cb->hcdb, hc_info->name)))
      {
         hc = avnd_hcdb_rec_add(cb, hc_info, &rc);

         if(NULL != hc)
         {         
            m_AVND_SEND_CKPT_UPDT_ASYNC_ADD(cb, hc, AVND_CKPT_HLT_CONFIG);
         }

         m_AVND_LOG_FOVER_EVTS(NCSFL_SEV_EMERGENCY, 
          AVND_LOG_FOVR_HLT_UPDT_FAIL , rc);

         return rc;
      }
      else
      {
         /* Since entry is present just update the fields */
         hc->max_dur = hc_info->max_duration;
         hc->period  = hc_info->period;
      }

      hc->rcvd_on_fover = TRUE;
   }

   /* Now walk through all the HC entries and delete those for which update
    * has not been received */
   memset(&hc_key, 0, sizeof(AVSV_HLT_KEY));

   while (NULL != (hc = (AVND_HC *)ncs_patricia_tree_getnext(&cb->hcdb, (uns8 *)&hc_key)))
   {
      hc_key = hc->key;

      /* If it was received on f-over message then continue */
      if (hc->rcvd_on_fover)
      {
         hc->rcvd_on_fover = FALSE;
         continue;
      }

      if(NULL != (hc = m_AVND_HCDB_REC_GET(cb->hcdb, hc_key)))
      {
         m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, hc, AVND_CKPT_HLT_CONFIG);
         /* We have not received this entry on f-over, so delete it */
         avnd_hcdb_rec_del(cb, &hc_key);
      }
   }

   return rc;
}


/****************************************************************************
  Name          : avnd_evt_avd_reg_hlt_msg
 
  Description   : This routine processes the health check update message from
                  AvD. The entire healthcheck database is sent when the node 
                  comes up. Incremental additions are also sent through this
                  message.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_avd_reg_hlt_msg (AVND_CB *cb, AVND_EVT *evt)
{
   AVSV_D2N_REG_HLT_MSG_INFO *info = 0;
   AVSV_HLT_INFO_MSG         *hc_info = 0;
   AVND_HC                   *hc = 0;
   AVND_MSG                  msg;
   uns32                     rc = NCSCC_RC_SUCCESS;

   /* dont process unless AvD is up */
   if ( !m_AVND_CB_IS_AVD_UP(cb) ) goto done;


   info = &evt->info.avd->msg_info.d2n_reg_hlt;

   /* Message ID 0 check should be removed in future */
   if ((info->msg_id != 0) && 
       (info->msg_id != (cb->rcv_msg_id+1)))
   {
      /* Log Error */
      rc = NCSCC_RC_FAILURE;
      m_AVND_LOG_FOVER_EVTS(NCSFL_SEV_EMERGENCY, 
                AVND_LOG_MSG_ID_MISMATCH, info->msg_id);

      goto done;
   }

   cb->rcv_msg_id = info->msg_id;

   /* 
    * Check whether message is sent on fail-over. If yes then it is
    * a time to process it specially.
    */
   if (info->msg_on_fover)
   {
      rc = avnd_evt_avd_hlt_updt_on_fover(cb, evt);
      goto done;
   }

   /* scan the hc list & add each hc to hc-db */
   for (hc_info = info->hlt_list; hc_info; hc = 0, hc_info = hc_info->next)
   {
      hc = avnd_hcdb_rec_add(cb, hc_info, &rc);
      if (!hc) break;
      m_AVND_SEND_CKPT_UPDT_ASYNC_ADD(cb, hc, AVND_CKPT_HLT_CONFIG);
   }

   /* 
    * if all the records aren't added, delete those 
    * records that were successfully added
    */
   if (hc_info) /* => add operation stopped in the middle */
   {
      for (hc_info = info->hlt_list; hc_info; hc_info = hc_info->next)
      {
         if(NULL != (hc = m_AVND_HCDB_REC_GET(cb->hcdb, hc_info->name)))
         {
           m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, hc, AVND_CKPT_HLT_CONFIG);
           avnd_hcdb_rec_del(cb, &hc_info->name);
         }
      }
   }

   /*** send the response to AvD (if it's sent to me) ***/
   if (info->nodeid == cb->clmdb.node_info.nodeId)
   {
      memset(&msg, 0, sizeof(AVND_MSG));
      msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG;
      if (!msg.info.avd)
      {
         rc = NCSCC_RC_FAILURE;
         goto done;
      }

      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_REG_HLT_MSG;
      msg.info.avd->msg_info.n2d_reg_hlt.msg_id = ++(cb->snd_msg_id);
      msg.info.avd->msg_info.n2d_reg_hlt.node_id = cb->clmdb.node_info.nodeId;
      if (info->hlt_list)
         msg.info.avd->msg_info.n2d_reg_hlt.hltchk_name = info->hlt_list->name;
      msg.info.avd->msg_info.n2d_reg_hlt.error = 
         (NCSCC_RC_SUCCESS == rc) ? NCSCC_RC_SUCCESS : NCSCC_RC_FAILURE;
      
      rc = avnd_di_msg_send(cb, &msg);
      if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;

      /* free the contents of avnd message */
      avnd_msg_content_free(cb, &msg);
   }

done:
   return rc;
}


/****************************************************************************
  Name          : avnd_evt_avd_operation_request_msg
 
  Description   : This routine processes the operation request message from 
                  AvD. These messages are generated in response to 
                  configuration requests to AvD that modify a single object.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_avd_operation_request_msg (AVND_CB *cb, AVND_EVT *evt)
{
   AVSV_D2N_OPERATION_REQUEST_MSG_INFO *info;
   AVSV_PARAM_INFO *param = 0;
   AVND_MSG        msg;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* dont process unless AvD is up */
   if ( !m_AVND_CB_IS_AVD_UP(cb) ) goto done;

   info = &evt->info.avd->msg_info.d2n_op_req;
   param = &info->param_info;

   if (info->msg_id != (cb->rcv_msg_id+1))
   {
      /* Log Error */
      rc = NCSCC_RC_FAILURE;
      m_AVND_LOG_FOVER_EVTS(NCSFL_SEV_EMERGENCY, 
                AVND_LOG_MSG_ID_MISMATCH, info->msg_id);

      goto done;
   }

   cb->rcv_msg_id = info->msg_id;

   switch (param->table_id)
   {
   case NCSMIB_TBL_AVSV_AMF_NODE:
      {
         switch (param->act)
         {
         case AVSV_OBJ_OPR_MOD:
            switch (param->obj_id)
            {
            case saAmfNodeSuFailoverProb_ID:
               m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
               cb->su_failover_prob = m_NCS_OS_NTOHLL_P(param->value);
               break;

            case saAmfNodeSuFailoverMax_ID:
               m_AVSV_ASSERT(sizeof(uns32) == param->value_len);
               cb->su_failover_max = m_NCS_OS_NTOHL(*(uns32 *)(param->value));
               break;

            default:
               break;
            }
            break;

         case AVSV_OBJ_OPR_DEL:
            /* what's to be done here.. initiate self-destruction ??? TBD */
            break;

         default:
            m_AVSV_ASSERT(0);
         }
      }
      break;

   case NCSMIB_TBL_AVSV_AMF_SG:
      {
         switch (param->act)
         {
         case AVSV_OBJ_OPR_MOD:
            {
               AVND_SU *su = 0;

               su = m_AVND_SUDB_REC_GET(cb->sudb, param->name_net);
               if (!su) goto done;

               switch (param->obj_id)
               {
               case saAmfSGCompRestartProb_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  su->comp_restart_prob = m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVND_CKPT_SU_COMP_RESTART_PROB);
                  break;
                  
               case saAmfSGCompRestartMax_ID:
                  m_AVSV_ASSERT(sizeof(uns32) == param->value_len);
                  su->comp_restart_max = m_NCS_OS_NTOHL(*(uns32 *)(param->value));
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVND_CKPT_SU_COMP_RESTART_MAX);
                  break;
                  
               case saAmfSGSuRestartProb_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  su->su_restart_prob = m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVND_CKPT_SU_RESTART_PROB);
                  break;
                  
               case saAmfSGSuRestartMax_ID:
                  m_AVSV_ASSERT(sizeof(uns32) == param->value_len);
                  su->su_restart_max = m_NCS_OS_NTOHL(*(uns32 *)(param->value));
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVND_CKPT_SU_RESTART_MAX);
                  break;
                  
               default:
                  break;
               }
               break;
            }

         case AVSV_OBJ_OPR_DEL:
            /* avnd doesnt understand what sg is.. nothing to be done */
            break;

         default:
            m_AVSV_ASSERT(0);
         }
      }
      break;

   case NCSMIB_TBL_AVSV_AMF_SU:
      {
         switch (param->act)
         {
         case AVSV_OBJ_OPR_MOD:
            switch (param->obj_id)
            {
            default:
               break;
            }
            break;

         case AVSV_OBJ_OPR_DEL:
            {
               AVND_SU *su = 0;

               /* get the record */
               su = m_AVND_SUDB_REC_GET(cb->sudb, param->name_net);
               if (su)
               {
                  /* unreg the row from mab */
                  avnd_mab_unreg_tbl_rows(cb, NCSMIB_TBL_AVSV_NCS_SU_STAT, su->mab_hdl,
                  (su->su_is_external?cb->avnd_mbcsv_mab_hdl:cb->mab_hdl));
                  
                  /* delete the record */
                  m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, su, AVND_CKPT_SU_CONFIG);
                  rc = avnd_sudb_rec_del(cb, &param->name_net);
               }
            }
            break;

         default:
            m_AVSV_ASSERT(0);
         }
      }
      break;

   case NCSMIB_TBL_AVSV_AMF_COMP:
      {
         switch (param->act)
         {
         case AVSV_OBJ_OPR_MOD:
            {
               AVND_COMP *comp = 0;

               comp = m_AVND_COMPDB_REC_GET(cb->compdb, param->name_net);
               if (!comp) goto done;

               switch (param->obj_id)
               {
               case saAmfCompInstantiateCmd_ID:
                  memset(&comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].cmd, 0, 
                              sizeof(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].cmd));
                  memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].cmd,
                              param->value, param->value_len);
                  comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].len = param->value_len;
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_INST_CMD);
                  break;
               case saAmfCompTerminateCmd_ID:
                  memset(&comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].cmd, 0, 
                              sizeof(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].cmd));
                  memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].cmd,
                              param->value, param->value_len);
                  comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].len = param->value_len;
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_TERM_CMD);
                  break;
               case saAmfCompCleanupCmd_ID:
                  memset(&comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].cmd, 0, 
                              sizeof(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].cmd));
                  memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].cmd,
                              param->value, param->value_len);
                  comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].len = param->value_len;
                  break;
               case saAmfCompAmStartCmd_ID:
                  memset(&comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].cmd, 0, 
                              sizeof(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].cmd));
                  memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].cmd,
                              param->value, param->value_len);
                  comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].len = param->value_len;
                  break;
               case saAmfCompAmStopCmd_ID:
                  memset(&comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].cmd, 0, 
                              sizeof(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].cmd));
                  memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].cmd,
                              param->value, param->value_len);
                  comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].len = param->value_len;
                  break;
               case saAmfCompInstantiateTimeout_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].timeout = 
                     m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_INST_TIMEOUT);
                  break;
                  
               case saAmfCompDelayBetweenInstantiateAttempts_ID:
                  break;

               case saAmfCompTerminateTimeout_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].timeout = 
                     m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_TERM_TIMEOUT);
                  break;
                  
               case saAmfCompCleanupTimeout_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].timeout = 
                     m_NCS_OS_NTOHLL_P(param->value);
                  break;
                  
               case saAmfCompAmStartTimeout_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].timeout = 
                     m_NCS_OS_NTOHLL_P(param->value);
                  break;
                  
               case saAmfCompAmStopTimeout_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].timeout = 
                     m_NCS_OS_NTOHLL_P(param->value);
                  break;
                  
               case saAmfCompTerminateCallbackTimeOut_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->term_cbk_timeout = m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_TERM_CBK_TIMEOUT);
                  break;

               case saAmfCompCSISetCallbackTimeout_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->csi_set_cbk_timeout = m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_CSI_SET_CBK_TIMEOUT);
                  break;

               case saAmfCompQuiescingCompleteTimeout_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->quies_complete_cbk_timeout = m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_QUIES_CMPLT_CBK_TIMEOUT);
                  break;

               case saAmfCompCSIRmvCallbackTimeout_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->csi_rmv_cbk_timeout= m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_CSI_RMV_CBK_TIMEOUT);
                  break;

               case saAmfCompProxiedCompInstantiateCallbackTimeout_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->pxied_inst_cbk_timeout= m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_PXIED_INST_CBK_TIMEOUT);
                  break;

               case saAmfCompProxiedCompCleanupCallbackTimeout_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  comp->pxied_clean_cbk_timeout= m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_PXIED_CLEAN_CBK_TIMEOUT);
                  break;
                  
               case saAmfCompNodeRebootCleanupFail_ID:
                  break;
               
               case saAmfCompRecoveryOnError_ID:
                  m_AVSV_ASSERT(sizeof(uns32) == param->value_len);
                  comp->err_info.def_rec = m_NCS_OS_NTOHL(*(uns32 *)(param->value));
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_DEFAULT_RECVR);
                  break;

               case saAmfCompNumMaxInstantiate_ID:
                  m_AVSV_ASSERT(sizeof(uns32) == param->value_len);
                  comp->clc_info.inst_retry_max = m_NCS_OS_NTOHL(*(uns32 *)(param->value));
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_INST_RETRY_MAX);
                  break;

               case saAmfCompAMEnable_ID:
                  m_AVSV_ASSERT(1 == param->value_len);
                  comp->is_am_en = (NCS_BOOL)param->value[0];
                  comp->clc_info.am_start_retry_cnt = 0;
                  rc = avnd_comp_am_oper_req_process(cb, comp);
                  break;
                  
               case saAmfCompNumMaxInstantiateWithDelay_ID:
               case saAmfCompNumMaxAmStartAttempts_ID:
               case saAmfCompNumMaxAmStopAttempts_ID:
               default:
                  break;
               }
            }
            break;

         case AVSV_OBJ_OPR_DEL:
            {
               AVND_COMP *comp = 0;

               /* get the record */
               comp = m_AVND_COMPDB_REC_GET(cb->compdb, param->name_net);
               if (comp)
               {
                  /* unreg the row from mab */
                  avnd_mab_unreg_tbl_rows(cb, NCSMIB_TBL_AVSV_NCS_COMP_STAT, comp->mab_hdl,
                  (comp->su->su_is_external?cb->avnd_mbcsv_mab_hdl:cb->mab_hdl));
                  
                  /* delete the record */
                  m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, comp, AVND_CKPT_COMP_CONFIG);         
                  rc = avnd_compdb_rec_del(cb, &param->name_net);
               }
            }
            break;

         default:
            m_AVSV_ASSERT(0);
         }
      }
      break;

   case NCSMIB_TBL_AVSV_AMF_HLT_CHK:
      {
         AVSV_HLT_KEY hc_key;
         uns32 l_num;

         memset(&hc_key, 0, sizeof(AVSV_HLT_KEY));

         hc_key.comp_name_net.length = param->name_net.length;
         hc_key.name.keyLen = m_NCS_OS_NTOHS(param->name_sec_net.length);
         l_num = hc_key.name.keyLen;
         hc_key.key_len_net = m_NCS_OS_HTONL(l_num);

         memcpy(hc_key.comp_name_net.value, param->name_net.value, 
                         m_NCS_OS_NTOHS(hc_key.comp_name_net.length));
         memcpy(hc_key.name.key, param->name_sec_net.value, hc_key.name.keyLen);

         switch (param->act)
         {
         case AVSV_OBJ_OPR_MOD:
            {
               AVND_HC *hc = 0;

               hc = m_AVND_HCDB_REC_GET(cb->hcdb, hc_key);
               if (!hc) goto done;

               switch (param->obj_id)
               {
               case saAmfHealthCheckPeriod_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  hc->period = m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, hc, AVND_CKPT_HC_PERIOD);
                  break;
                  
               case saAmfHealthCheckMaxDuration_ID:
                  m_AVSV_ASSERT(sizeof(SaTimeT) == param->value_len);
                  hc->max_dur = m_NCS_OS_NTOHLL_P(param->value);
                  m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, hc, AVND_CKPT_HC_MAX_DUR);
                  break;
                  
               default:
                  break;
               }
            }
            break;

         case AVSV_OBJ_OPR_DEL:
          {
            AVND_HC *hc = 0;

            if(NULL != (hc = m_AVND_HCDB_REC_GET(cb->hcdb, hc_key)))
            {
              m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, hc, AVND_CKPT_HLT_CONFIG);
              rc = avnd_hcdb_rec_del(cb, &hc_key);
            }
            break;
          }
         default:
            m_AVSV_ASSERT(0);
         }
      }
      break;

   default:
      m_AVSV_ASSERT(0);
   } /* switch */

   /* 
    * Send the response to avd.
    */
   if (info->node_id == cb->clmdb.node_info.nodeId)
   {
      memset(&msg, 0, sizeof(AVND_MSG));
      msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG;
      if (!msg.info.avd) 
      {
         rc = NCSCC_RC_FAILURE;
         goto done;
      }
      
      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_OPERATION_REQUEST_MSG;
      msg.info.avd->msg_info.n2d_op_req.msg_id = ++(cb->snd_msg_id);
      msg.info.avd->msg_info.n2d_op_req.node_id = cb->clmdb.node_info.nodeId;
      msg.info.avd->msg_info.n2d_op_req.param_info = *param;
      msg.info.avd->msg_info.n2d_op_req.error = rc;
      
      rc = avnd_di_msg_send(cb, &msg);
      if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;

      /* free the contents of avnd message */
      avnd_msg_content_free(cb, &msg);
   }

done:
   return rc;
}


/****************************************************************************
  Name          : avnd_evt_avd_hb_info_msg
 
  Description   : This routine processes the heartbeat info message from AvD.
                  After processing this message, AvND starts heartbeating with
                  new rate as indicated by AvD.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_avd_hb_info_msg (AVND_CB *cb, AVND_EVT *evt)
{
   AVSV_D2N_INFO_HEARTBEAT_MSG_INFO *info = &evt->info.avd->msg_info.d2n_info_hrt_bt;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* dont process unless AvD is up */
   if ( !m_AVND_CB_IS_AVD_UP(cb) ) goto done;

   /* update the heartbeat rate */
   cb->hb_intv = info->snd_hb_intvl;

   /* stop the current hb-tmr */
   m_AVND_TMR_HB_STOP(cb);

   /* send the heartbeat */
   rc = avnd_di_hb_send(cb);

done:
   return rc;
}


/****************************************************************************
  Name          : avnd_evt_avd_ack_message
 
  Description   : This routine processes Ack message 
                  response from AvD.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_avd_ack_message (AVND_CB *cb, AVND_EVT *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* dont process unless AvD is up */
   if ( !m_AVND_CB_IS_AVD_UP(cb) ) goto done;

   /* process the ack response */
   avnd_di_msg_ack_process(cb, 
      evt->info.avd->msg_info.d2n_ack_info.msg_id_ack);

done:
   return rc;
}


/****************************************************************************
  Name          : avnd_evt_tmr_snd_hb
 
  Description   : This routine sends the periodic heartbeat to AVD.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_tmr_snd_hb (AVND_CB *cb, AVND_EVT *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* send the heartbeat to AvD */
   rc = avnd_di_hb_send(cb);

   return rc;
}


/****************************************************************************
  Name          : avnd_evt_tmr_rcv_msg_rsp
 
  Description   : This routine handles the expiry of the msg response timer.
                  It resends the message if the retry count is not already 
                  exceeded.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_tmr_rcv_msg_rsp (AVND_CB *cb, AVND_EVT *evt)
{
   AVND_TMR_EVT      *tmr = &evt->info.tmr;
   AVND_DND_MSG_LIST *rec = 0;
   uns32             rc = NCSCC_RC_SUCCESS;

   /* retrieve the message record */
   if ( (0 == (rec = (AVND_DND_MSG_LIST *)ncshm_take_hdl(NCS_SERVICE_ID_AVND, tmr->opq_hdl))) )
      goto done;

   rc = avnd_diq_rec_send(cb, rec);

   ncshm_give_hdl(tmr->opq_hdl);

done:
   return rc;
}


/****************************************************************************
  Name          : avnd_evt_mds_avd_up
 
  Description   : This routine processes the AvD up event from MDS. It sends 
                  the node-up message to AvD.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_mds_avd_up (AVND_CB *cb, AVND_EVT *evt)
{
   AVND_MSG msg;
   uns32 rc = NCSCC_RC_SUCCESS;

/* Avd is already UP, reboot the node */
   if ( m_AVND_CB_IS_AVD_UP(cb) )
       ncs_reboot("AVD is already up");
  
   /* send node up message to AvD */
   memset(&msg, 0, sizeof(AVND_MSG));

   if ( 0 != (msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG) )
   {
      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_CLM_NODE_UP_MSG;
      msg.info.avd->msg_info.n2d_clm_node_up.msg_id = ++(cb->snd_msg_id);
      msg.info.avd->msg_info.n2d_clm_node_up.node_id = cb->clmdb.node_info.nodeId;
      msg.info.avd->msg_info.n2d_clm_node_up.boot_timestamp = 
         cb->clmdb.node_info.bootTimestamp;
      msg.info.avd->msg_info.n2d_clm_node_up.adest_address = cb->avnd_dest;
      
      rc = avnd_di_msg_send(cb, &msg);
      if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;
   }
   else rc = NCSCC_RC_FAILURE;

   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);

   return rc;
}


/****************************************************************************
  Name          : avnd_evt_mds_avd_dn
 
  Description   : This routine processes the AvD down event from MDS.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : What should be done? TBD.
******************************************************************************/
uns32 avnd_evt_mds_avd_dn (AVND_CB *cb, AVND_EVT *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   ncs_reboot("MDS down received for AVD");

   return rc;
}


/****************************************************************************
  Name          : avnd_di_hb_send
 
  Description   : This routine sends the heartbeat to AvD.
 
  Arguments     : cb - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_di_hb_send (AVND_CB *cb)
{
   AVND_MSG msg;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* determine if avd heartbeating is on */
   if ( !m_AVND_IS_AVD_HB_ON(cb) )
      return rc;

   memset(&msg, 0, sizeof(AVND_MSG));

   /* populate the heartbeat msg */
   if ( 0 != (msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG) )
   {
      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_HEARTBEAT_MSG;
      msg.info.avd->msg_info.n2d_hrt_bt.node_id = cb->clmdb.node_info.nodeId;
      
      /* send the heartbeat to AvD */
      rc = avnd_di_msg_send(cb, &msg);
      
      /* restart the heartbeat timer */
      if ( NCSCC_RC_SUCCESS == rc )
      {
         m_AVND_TMR_HB_START(cb, rc);
         msg.info.avd = 0;
      }
   }
   else rc = NCSCC_RC_FAILURE;

   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);

   return rc;
}


/****************************************************************************
  Name          : avnd_di_oper_send
 
  Description   : This routine sends the operational state of the node & the
                  service unit to AvD.
 
  Arguments     : cb   - ptr to the AvND control block
                  su   - ptr to the su
                  rcvr - recommended recovery
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_di_oper_send (AVND_CB *cb, AVND_SU *su, uns32 rcvr)
{
   AVND_MSG msg;
   uns32    rc = NCSCC_RC_SUCCESS;

   memset(&msg, 0, sizeof(AVND_MSG));

   /* populate the oper msg */
   if ( 0 != (msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG) )
   {
      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_OPERATION_STATE_MSG;
      msg.info.avd->msg_info.n2d_opr_state.msg_id = ++(cb->snd_msg_id);
      msg.info.avd->msg_info.n2d_opr_state.node_id = cb->clmdb.node_info.nodeId;
      msg.info.avd->msg_info.n2d_opr_state.node_oper_state = cb->oper_state;

      if (su) 
      {
         msg.info.avd->msg_info.n2d_opr_state.su_name_net = su->name_net;
         msg.info.avd->msg_info.n2d_opr_state.su_oper_state = su->oper;
         
         /* Console Print to help debugging */
        if((su->is_ncs == TRUE) && (su->oper == NCS_OPER_STATE_DISABLE)) 
          m_NCS_DBG_PRINTF("\nAvSv: -%s SU Oper state got disabled\n", su->name_net.value);
      }

      msg.info.avd->msg_info.n2d_opr_state.rec_rcvr = rcvr;
      
      /* send the msg to AvD */
      rc = avnd_di_msg_send(cb, &msg);
      if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;
   }
   else rc = NCSCC_RC_FAILURE;

   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);

   return rc;
}


/****************************************************************************
  Name          : avnd_di_susi_resp_send
 
  Description   : This routine sends the SU-SI assignment/removal response to 
                  AvD.
 
  Arguments     : cb - ptr to the AvND control block
                  su - ptr to the su
                  si - ptr to SI record (if 0, response is meant for all the 
                       SIs that belong to the SU)
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_di_susi_resp_send (AVND_CB *cb, AVND_SU *su, AVND_SU_SI_REC *si)
{
   AVND_SU_SI_REC *curr_si = 0;
   AVND_MSG       msg;
   uns32          rc = NCSCC_RC_SUCCESS;

   memset(&msg, 0, sizeof(AVND_MSG));

   /* get the curr-si */
   curr_si = (si) ? si : (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_FIRST(&su->si_list);
   m_AVSV_ASSERT(curr_si);

   /* populate the susi resp msg */
   if ( 0 != (msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG) )
   {
      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_INFO_SU_SI_ASSIGN_MSG;
      msg.info.avd->msg_info.n2d_su_si_assign.msg_id = ++(cb->snd_msg_id);
      msg.info.avd->msg_info.n2d_su_si_assign.node_id = cb->clmdb.node_info.nodeId;
      msg.info.avd->msg_info.n2d_su_si_assign.msg_act = 
                 ( m_AVND_SU_SI_CURR_ASSIGN_STATE_IS_ASSIGNED(curr_si) || 
                   m_AVND_SU_SI_CURR_ASSIGN_STATE_IS_ASSIGNING(curr_si) ) ? 
                    ((!curr_si->prv_state) ? AVSV_SUSI_ACT_ASGN : 
                                             AVSV_SUSI_ACT_MOD) : AVSV_SUSI_ACT_DEL;
      msg.info.avd->msg_info.n2d_su_si_assign.su_name_net = su->name_net;
      if (si) msg.info.avd->msg_info.n2d_su_si_assign.si_name_net = si->name_net;
      msg.info.avd->msg_info.n2d_su_si_assign.ha_state = 
         (SA_AMF_HA_QUIESCING == curr_si->curr_state) ? SA_AMF_HA_QUIESCED : 
                                                        curr_si->curr_state;
      msg.info.avd->msg_info.n2d_su_si_assign.error = 
                 ( m_AVND_SU_SI_CURR_ASSIGN_STATE_IS_ASSIGNED(curr_si) ||
                   m_AVND_SU_SI_CURR_ASSIGN_STATE_IS_REMOVED(curr_si) ) ? 
                                  NCSCC_RC_SUCCESS : NCSCC_RC_FAILURE;

      /* send the msg to AvD */
      rc = avnd_di_msg_send(cb, &msg);
      if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;

      /* we have completed the SU SI msg processing */
      m_AVND_SU_ASSIGN_PEND_RESET(su);
      m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVND_CKPT_SU_FLAG_CHANGE);
   }
   else rc = NCSCC_RC_FAILURE;

   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);

   return rc;
}


/****************************************************************************
  Name          : avnd_di_mib_upd_send
 
  Description   : This routine sends mib update of those objects that reside 
                  on AvD but are maintained on AvND.
 
  Arguments     : cb    - ptr to the AvND control block
                  param - ptr to the params that are to be updated to AvD
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_di_mib_upd_send (AVND_CB *cb, AVSV_PARAM_INFO *param)
{
   AVND_MSG msg;
   uns32    rc = NCSCC_RC_SUCCESS;

   memset(&msg, 0, sizeof(AVND_MSG));

   /* populate the msg */
   if ( 0 != (msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG) )
   {
      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_DATA_REQUEST_MSG;
      msg.info.avd->msg_info.n2d_data_req.msg_id = ++(cb->snd_msg_id);
      msg.info.avd->msg_info.n2d_data_req.node_id = cb->clmdb.node_info.nodeId;
      msg.info.avd->msg_info.n2d_data_req.param_info = *param;

      /* send the msg to AvD */
      rc = avnd_di_msg_send(cb, &msg);
      if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;
   }
   else rc = NCSCC_RC_FAILURE;

   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);

   return rc;
}


/****************************************************************************
  Name          : avnd_di_pg_act_send
 
  Description   : This routine sends pg track start and stop requests to AvD.
 
  Arguments     : cb           - ptr to the AvND control block
                  csi_name_net - ptr to the csi-name
                  act          - req action (start/stop)
                  fover        - TRUE if the message being sent on fail-over.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_di_pg_act_send (AVND_CB           *cb, 
                           SaNameT           *csi_name_net, 
                           AVSV_PG_TRACK_ACT actn, 
                           NCS_BOOL          fover)
{
   AVND_MSG msg;
   uns32    rc = NCSCC_RC_SUCCESS;

   memset(&msg, 0, sizeof(AVND_MSG));

   /* populate the msg */
   if ( 0 != (msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG) )
   {
      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_PG_TRACK_ACT_MSG;
      msg.info.avd->msg_info.n2d_pg_trk_act.node_id = cb->clmdb.node_info.nodeId;
      msg.info.avd->msg_info.n2d_pg_trk_act.csi_name_net = *csi_name_net;
      msg.info.avd->msg_info.n2d_pg_trk_act.actn = actn;
      msg.info.avd->msg_info.n2d_pg_trk_act.msg_on_fover = fover;

      /* send the msg to AvD */
      rc = avnd_di_msg_send(cb, &msg);
      if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;
   }
   else rc = NCSCC_RC_FAILURE;

   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);

   return rc;
}


/****************************************************************************
  Name          : avnd_di_msg_send
 
  Description   : This routine sends the message to AvD.
 
  Arguments     : cb  - ptr to the AvND control block
                  msg - ptr to the message
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  Notes         : None
******************************************************************************/
uns32 avnd_di_msg_send (AVND_CB *cb, AVND_MSG *msg)
{
   AVND_DND_MSG_LIST *rec = 0;
   uns32             rc = NCSCC_RC_SUCCESS;

   /* nothing to send */
   if (!msg) goto done;

   /* Heartbeat and Verify Ack nack and PG track action msgs are not buffered */
   if ( m_AVSV_N2D_MSG_IS_HB(msg->info.avd) || 
        m_AVSV_N2D_MSG_IS_PG_TRACK_ACT(msg->info.avd)) 
   {
       /*send the response to AvD */
      rc = avnd_mds_send(cb, msg, &cb->avd_dest, 0);
      goto done;
   }
   else if( m_AVSV_N2D_MSG_IS_VER_ACK_NACK(msg->info.avd))
   {
       /*send the response to active AvD (In case MDS has not updated its
                                                      tables by this time) */
      rc = avnd_mds_red_send(cb, msg, &cb->avd_dest, &cb->active_avd_adest);
      goto done;
   }


   /* add the record to the AvD msg list */
   if ((0 != (rec = avnd_diq_rec_add(cb, msg))) )
   {
      /* send the message */
      avnd_diq_rec_send(cb, rec);
   }
   else rc = NCSCC_RC_FAILURE;

done:
   if (NCSCC_RC_SUCCESS != rc && rec)
   {
      /* pop & delete */
      m_AVND_DIQ_REC_FIND_POP(cb, rec);
      avnd_diq_rec_del(cb, rec);
   }
   return rc;
}


/****************************************************************************
  Name          : avnd_di_ack_nack_msg_send
 
  Description   : This routine processes the data verify message sent by newly
                  Active AVD.
 
  Arguments     : cb  - ptr to the AvND control block
                  rcv_id - Receive message ID for AVND
                  view_num - Cluster view number
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_di_ack_nack_msg_send(AVND_CB *cb, uns32 rcv_id, uns32 view_num)
{
   AVND_MSG                   msg;
   uns32        rc = NCSCC_RC_SUCCESS;

   /*** send the response to AvD ***/
   memset(&msg, 0, sizeof(AVND_MSG));

   if ( 0 != (msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG) )
   {
      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_VERIFY_ACK_NACK_MSG;
      msg.info.avd->msg_info.n2d_ack_nack_info.msg_id = (cb->snd_msg_id +1);
      msg.info.avd->msg_info.n2d_ack_nack_info.node_id = cb->clmdb.node_info.nodeId;

      if (rcv_id != cb->rcv_msg_id)
         msg.info.avd->msg_info.n2d_ack_nack_info.ack = FALSE;
      else
         msg.info.avd->msg_info.n2d_ack_nack_info.ack = TRUE;

      m_AVND_LOG_FOVER_EVTS(NCSFL_SEV_NOTICE, 
         AVND_LOG_FOVR_MSG_ID_ACK_NACK, 
         msg.info.avd->msg_info.n2d_ack_nack_info.ack);

      if (view_num != cb->clmdb.curr_view_num)
         msg.info.avd->msg_info.n2d_ack_nack_info.v_num_ack = FALSE;
      else
         msg.info.avd->msg_info.n2d_ack_nack_info.v_num_ack = TRUE;

      m_AVND_LOG_FOVER_EVTS(NCSFL_SEV_NOTICE, 
         AVND_LOG_FOVR_VNUM_ACK_NACK, 
         msg.info.avd->msg_info.n2d_ack_nack_info.v_num_ack);

      rc = avnd_di_msg_send(cb, &msg);
      if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;
   }
   else rc = NCSCC_RC_FAILURE;

   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);

   return rc;
}


/****************************************************************************
  Name          : avnd_di_reg_su_rsp_snd
 
  Description   : This routine sends response to register SU message.
 
  Arguments     : cb  - ptr to the AvND control block
                  su_name - SU name.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_di_reg_su_rsp_snd(AVND_CB *cb, SaNameT *su_name, uns32 ret_code)
{
   AVND_MSG                 msg;
   uns32       rc = NCSCC_RC_SUCCESS;

      memset(&msg, 0, sizeof(AVND_MSG));
   if ( 0 != (msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG) )
   {
      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_REG_SU_MSG;
      msg.info.avd->msg_info.n2d_reg_su.msg_id = ++(cb->snd_msg_id);
      msg.info.avd->msg_info.n2d_reg_su.node_id = cb->clmdb.node_info.nodeId;
      msg.info.avd->msg_info.n2d_reg_su.su_name_net = *su_name;
      msg.info.avd->msg_info.n2d_reg_su.error = 
         (NCSCC_RC_SUCCESS == ret_code)? NCSCC_RC_SUCCESS: NCSCC_RC_FAILURE;
      
      rc = avnd_di_msg_send(cb, &msg);
      if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;
   }
   else rc = NCSCC_RC_FAILURE;

   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avnd_di_reg_comp_rsp_snd
 
  Description   : This routine sends response to register COMP message.
 
  Arguments     : cb  - ptr to the AvND control block
                  su_name - SU name.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_di_reg_comp_rsp_snd(AVND_CB *cb, SaNameT *comp_name, uns32 ret_code)
{
   AVND_MSG                 msg;
   uns32       rc = NCSCC_RC_SUCCESS;

   memset(&msg, 0, sizeof(AVND_MSG));
   if ( 0 != (msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG) )
   {
      memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
      msg.type = AVND_MSG_AVD;
      msg.info.avd->msg_type = AVSV_N2D_REG_COMP_MSG;
      msg.info.avd->msg_info.n2d_reg_comp.msg_id = ++(cb->snd_msg_id);
      msg.info.avd->msg_info.n2d_reg_comp.node_id = cb->clmdb.node_info.nodeId;
      msg.info.avd->msg_info.n2d_reg_comp.comp_name_net = *comp_name;
      msg.info.avd->msg_info.n2d_reg_comp.error = 
            (NCSCC_RC_SUCCESS == ret_code) ? NCSCC_RC_SUCCESS : NCSCC_RC_FAILURE;

      rc = avnd_di_msg_send(cb, &msg);
      if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;
   }
   else rc = NCSCC_RC_FAILURE;

   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avnd_di_msg_ack_process
 
  Description   : This routine processes the the acks that are generated by
                  AvD in response to messages from AvND. It pops the 
                  corresponding record from the AvD msg list & frees it.
 
  Arguments     : cb  - ptr to the AvND control block
                  mid - message-id
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_di_msg_ack_process (AVND_CB *cb, uns32 mid)
{
   AVND_DND_MSG_LIST *rec = 0;

   /* find & pop the matching record */
   m_AVND_DIQ_REC_FIND(cb, mid, rec);
   if (rec)
   {
      m_AVND_DIQ_REC_FIND_POP(cb, rec);
      avnd_diq_rec_del(cb, rec);
   }

   return;
}


/****************************************************************************
  Name          : avnd_diq_del
 
  Description   : This routine clears the AvD msg list.
 
  Arguments     : cb - ptr to the AvND control block
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_diq_del (AVND_CB *cb)
{
   AVND_DND_MSG_LIST *rec = 0;

   do
   {
      /* pop the record */
      m_AVND_DIQ_REC_POP(cb, rec);
      if (!rec) break;

      /* delete the record */
      avnd_diq_rec_del(cb, rec);
   } while (1);

   return;
}


/****************************************************************************
  Name          : avnd_diq_rec_add
 
  Description   : This routine adds a record to the AvD msg list.
 
  Arguments     : cb  - ptr to the AvND control block
                  msg - ptr to the message
 
  Return Values : ptr to the newly added msg record
 
  Notes         : None.
******************************************************************************/
AVND_DND_MSG_LIST *avnd_diq_rec_add (AVND_CB *cb, AVND_MSG *msg)
{
   AVND_DND_MSG_LIST *rec = 0;

   if ( (0 == (rec = m_MMGR_ALLOC_AVND_DND_MSG_LIST)) )
      goto error;

   memset(rec, 0, sizeof(AVND_DND_MSG_LIST));
   
   /* create the association with hdl-mngr */
   if ( (0 == (rec->opq_hdl = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_AVND,
                                          (NCSCONTEXT)rec))) )
      goto error;

   /* store the msg (transfer memory ownership) */
   rec->msg.type = msg->type;
   rec->msg.info.avd = msg->info.avd;
   msg->info.avd = 0;

   /* push the record to the AvD msg list */
   m_AVND_DIQ_REC_PUSH(cb, rec);

   return rec;

error:
   if (rec) avnd_diq_rec_del(cb, rec);

   return 0;
}


/****************************************************************************
  Name          : avnd_diq_rec_del
 
  Description   : This routine deletes the record from the AvD msg list.
 
  Arguments     : cb  - ptr to the AvND control block
                  rec - ptr to the AvD msg record
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_diq_rec_del (AVND_CB *cb, AVND_DND_MSG_LIST *rec)
{
   /* remove the association with hdl-mngr */
   if (rec->opq_hdl)
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVND, rec->opq_hdl);

   /* stop the AvD msg response timer */
   if ( m_AVND_TMR_IS_ACTIVE(rec->resp_tmr) )
      m_AVND_TMR_MSG_RESP_STOP(cb, *rec);

   /* free the avnd message contents */
   avnd_msg_content_free(cb, &rec->msg);

   /* free the record */
   m_MMGR_FREE_AVND_DND_MSG_LIST(rec);

   return;
}


/****************************************************************************
  Name          : avnd_diq_rec_send
 
  Description   : This routine sends message (contained in the record) to 
                  AvD. It allocates a new message, copies the contents from 
                  the message contained in the record & then sends it to AvD.
 
  Arguments     : cb  - ptr to the AvND control block
                  rec - ptr to the msg record
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  Notes         : None
******************************************************************************/
uns32 avnd_diq_rec_send (AVND_CB *cb, AVND_DND_MSG_LIST *rec)
{
   AVND_MSG msg;
   uns32 rc = NCSCC_RC_SUCCESS;

   memset(&msg, 0, sizeof(AVND_MSG));

   /* copy the contents from the record */
   rc = avnd_msg_copy(cb, &msg, &rec->msg);
   
   /* send the message to AvD */
   if (NCSCC_RC_SUCCESS == rc)
      rc = avnd_mds_send(cb, &msg, &cb->avd_dest, 0);

   /* start the msg response timer */
   if (NCSCC_RC_SUCCESS == rc)
   {
      if (rec->msg.info.avd->msg_type ==  AVSV_N2D_CLM_NODE_UP_MSG)
         m_AVND_TMR_MSG_RESP_START(cb, *rec, rc);
      msg.info.avd = 0;
   } 

   /* free the msg */
   avnd_msg_content_free(cb, &msg);

   return rc;
}


/****************************************************************************
  Name          : avnd_evt_avd_shutdown_app_su_msg
 
  Description   : This routine processes shutdown of application SUs.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_avd_shutdown_app_su_msg(AVND_CB *cb, AVND_EVT *evt)
{
   uns32         rc = NCSCC_RC_SUCCESS;
   AVND_SU       *su = 0;
   NCS_BOOL      empty_sulist = TRUE;

   AVSV_D2N_SHUTDOWN_APP_SU_MSG_INFO *info = &evt->info.avd->msg_info.d2n_shutdown_app_su;

   if (info->msg_id != (cb->rcv_msg_id+1))
   {
      /* Log Error */
      rc = NCSCC_RC_FAILURE;
      m_AVND_LOG_FOVER_EVTS(NCSFL_SEV_EMERGENCY, 
                AVND_LOG_MSG_ID_MISMATCH, info->msg_id);

      goto done;
   }

   cb->rcv_msg_id = info->msg_id;

   if(cb->term_state == AVND_TERM_STATE_SHUTTING_APP_SU)
   {
      /* we need not do anything we are already shutting down
       we will send the responce after shutting down the app su's */
      goto done;
   }

   if(cb->term_state == AVND_TERM_STATE_SHUTTING_APP_DONE ||
         cb->term_state == AVND_TERM_STATE_SHUTTING_NCS_SU)
   {
      /* we are already done with shutdown of APP SU just send an ACK */
      avnd_snd_shutdown_app_su_msg(cb);
      goto done;
   }
      
  cb->term_state = AVND_TERM_STATE_SHUTTING_APP_SU;
   
   /* dont process unless AvD is up */
   if ( !m_AVND_CB_IS_AVD_UP(cb) ) 
      return rc;

   su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uns8 *)0);

   /* scan & drive the SU term by PRES_STATE FSM on each su */
   while(su != 0)
   {
      if((su->is_ncs == SA_TRUE) || (TRUE == su->su_is_external))
      {
         su = (AVND_SU *)
              ncs_patricia_tree_getnext(&cb->sudb, (uns8*)&su->name_net);
         continue;
      }

      /* to be on safer side lets remove the si's remaining if any */
      rc = avnd_su_si_remove(cb, su, 0);

      /* delete all the curr info on su & comp */
      /*rc = avnd_su_curr_info_del(cb, su);*/

      /* now terminate the su */
      if(( m_AVND_SU_IS_PREINSTANTIABLE(su) ) && 
         (su->pres != NCS_PRES_UNINSTANTIATED) &&
         (su->pres != NCS_PRES_INSTANTIATIONFAILED) &&
         (su->pres != NCS_PRES_TERMINATIONFAILED) )
      {
         empty_sulist = FALSE;

         /* trigger su termination for pi su */
         rc = avnd_su_pres_fsm_run(cb, su, 0, AVND_SU_PRES_FSM_EV_TERM);
      }

      su = (AVND_SU *)
              ncs_patricia_tree_getnext(&cb->sudb, (uns8*)&su->name_net);
   }

   if(empty_sulist == TRUE)
   {
      /* No SUs to be processed for termination.
      ** send the response message to AVD informing DONE. 
      */
      cb->term_state = AVND_TERM_STATE_SHUTTING_APP_DONE;
      avnd_snd_shutdown_app_su_msg(cb);
   }

done:
   return rc;
}
/****************************************************************************
  Name          : avnd_dnd_list_destroy
 
  Description   : This routine clean the dnd list in the cb
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_dnd_list_destroy(AVND_CB *cb)
{
   AVND_DND_LIST *list = &((cb)->dnd_list); 
   AVND_DND_MSG_LIST *rec = 0;
   
   if(list) 
      rec = list->head;

   while(rec)
   {
      /* find & pop the matching record */
      m_AVND_DIQ_REC_FIND_POP(cb, rec);
      avnd_diq_rec_del(cb, rec);

      rec = list->head;
   }

   return;
}

/****************************************************************************
  Name          : avnd_snd_shutdown_app_su_msg
 
  Description   : This routine sends the response to AVD for 
                  shutdown application SUs msg.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : None.
 
  Notes         : MARK the node_state as shutting down before invoking this 
                  function so we have the context what to do when 
                  we get the event "last step of termination".
******************************************************************************/
void avnd_snd_shutdown_app_su_msg(AVND_CB *cb)
{
   AVND_MSG msg;
   uns32    rc = NCSCC_RC_SUCCESS;

   memset(&msg, 0, sizeof(AVND_MSG));
   msg.info.avd = m_MMGR_ALLOC_AVSV_DND_MSG;
   if (!msg.info.avd)
   {
      rc = NCSCC_RC_FAILURE;
      goto done;
   }

   memset(msg.info.avd, 0, sizeof(AVSV_DND_MSG));
   msg.type = AVND_MSG_AVD;
   msg.info.avd->msg_type = AVSV_N2D_SHUTDOWN_APP_SU_MSG;
   msg.info.avd->msg_info.n2d_shutdown_app_su.msg_id = ++(cb->snd_msg_id); 
   msg.info.avd->msg_info.n2d_shutdown_app_su.node_id = 
                            m_NCS_NODE_ID_FROM_MDS_DEST(cb->avnd_dest);
   
   rc = avnd_di_msg_send(cb, &msg);
   if ( NCSCC_RC_SUCCESS == rc ) msg.info.avd = 0;

   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);

done:
   return ;
}
