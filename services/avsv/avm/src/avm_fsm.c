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

  DESCRIPTION:This module deals with posting of the received event to AvM FSM
  with necessary Pre - processing done depending on the source of the event.
  ..............................................................................

  Function Included in this Module:

  avm_msg_handler -
  avm_proc_hpi    -
  avm_proc_avd    -
  avm_proc_mib    -
  avm_fsm_handler -

******************************************************************************
*/

#include"avm.h"

static uns32
avm_proc_hpi(
               AVM_EVT_T *hpi_evt,
               AVM_CB_T *avm_cb
            );

static uns32
avm_proc_avd(
               AVM_EVT_T *avd_avm,
               AVM_CB_T *avm_cb
            );

static uns32
avm_proc_bam(
              AVM_EVT_T *bam_avm,
              AVM_CB_T *avm_cb
            );

static uns32
avm_proc_mib(
               AVM_EVT_T *mib_req,
               AVM_CB_T  *avm_cb
            );

static uns32
avm_proc_tmr(
               AVM_EVT_T *tmr,
               AVM_CB_T  *avm_cb
            );

static uns32
avm_proc_rde(
               AVM_EVT_T  *rde_evt,
               AVM_CB_T   *avm_cb
            );

static uns32
avm_proc_mds(
               AVM_EVT_T *mds_evt,
               AVM_CB_T  *avm_cb
            );

static uns32
avm_proc_ssu(
               AVM_EVT_T *hpi_evt,
               AVM_CB_T  *avm_cb
            );

static uns32
avm_proc_ssu_tmr_exp(
               AVM_EVT_T *hpi_evt,
               AVM_CB_T  *avm_cb
            );

static uns32
avm_proc_upgd_succ_tmr_exp(
               AVM_EVT_T *hpi_evt,
               AVM_CB_T  *avm_cb
            );

static uns32
avm_proc_boot_succ_tmr_exp(
               AVM_EVT_T *hpi_evt,
               AVM_CB_T  *avm_cb
            );

static uns32
avm_proc_dhcp_fail_tmr_exp(
               AVM_EVT_T *hpi_evt,
               AVM_CB_T  *avm_cb
            );

static uns32
avm_proc_bios_upgrade_tmr_exp(
               AVM_EVT_T *my_evt,
               AVM_CB_T  *avm_cb
            );

static uns32 avm_proc_bios_failover_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T  *avm_cb);

/*
extern uns32
avm_proc_ipmc_upgd_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T  *avm_cb);

extern uns32
avm_proc_ipmc_mod_upgd_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T  *avm_cb);
*/

/***********************************************************************
 ******
 * Name          : avm_msg_handler
 *
 * Description   : This function is invoked when  an event is received
 *                 at AvM
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
extern uns32
avm_msg_handler(
                 AVM_CB_T  *avm_cb,
                 AVM_EVT_T *evt
               )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_msg_handler");

   switch(evt->src)
   {
      case AVM_EVT_SSU:
      {
         rc = avm_proc_ssu_tmr_exp(evt, avm_cb);
      }
      break;

      case AVM_EVT_UPGD_SUCC:
      {
         rc = avm_proc_upgd_succ_tmr_exp(evt, avm_cb);
      }
      break;

      case AVM_EVT_BOOT_SUCC:
      {
         rc = avm_proc_boot_succ_tmr_exp(evt, avm_cb);
      }
      break;

      case AVM_EVT_DHCP_FAIL:
      {
         rc = avm_proc_dhcp_fail_tmr_exp(evt, avm_cb);
      }
      break;

      case AVM_EVT_BIOS_UPGRADE:
      {
         rc = avm_proc_bios_upgrade_tmr_exp(evt, avm_cb);   
      }
      break;   

      case AVM_EVT_IPMC_UPGD:
      {
         rc = avm_proc_ipmc_upgd_tmr_exp(evt, avm_cb);
      }
      break;

      case AVM_EVT_IPMC_MOD_UPGD:
      {
         rc = avm_proc_ipmc_mod_upgd_tmr_exp(evt, avm_cb);
      }       
      break;

      case AVM_EVT_ROLE_CHG_WAIT:
      {
         rc = avm_proc_role_chg_wait_tmr_exp(evt, avm_cb);
      }
      break;
      
      case AVM_EVT_BIOS_FAILOVER:
      {
         rc = avm_proc_bios_failover_tmr_exp(evt, avm_cb);
      }
      break;

      case AVM_EVT_HPI:
      {
         m_AVM_LOG_EVT_SRC("HPI", NCSFL_SEV_INFO);
         /* if it is first time, accumulate hotswap events for SSU processing */
         if (avm_cb->ssu_tmr.status != AVM_TMR_EXPIRED)
         {
            avm_proc_ssu(evt, avm_cb);
            break;
         }
         rc = avm_proc_hpi(evt, avm_cb);
      }
      break;

      case AVM_EVT_AVD:
      {
         m_AVM_LOG_EVT_SRC("AVD", NCSFL_SEV_INFO);
         rc = avm_proc_avd(evt, avm_cb);

         if(AVD_AVM_MSG_NULL != evt->evt.avd_evt)
         {
            m_MMGR_FREE_AVD_AVM_MSG((evt->evt.avd_evt));
         }
      }
      break;

      case AVM_EVT_BAM:
      {
         m_AVM_LOG_EVT_SRC("BAM", NCSFL_SEV_INFO);
         rc = avm_proc_bam(evt, avm_cb);

         if(BAM_AVM_MSG_NULL != evt->evt.bam_evt)
         {
            m_MMGR_FREE_BAM_AVM_MSG((evt->evt.bam_evt));
         }
      }
      break;

      case AVM_EVT_MIB:
      {
         m_AVM_LOG_EVT_SRC("MIB", NCSFL_SEV_INFO);
         rc = avm_proc_mib(evt, avm_cb);
      }
      break;

      case AVM_EVT_TMR:
      {
         m_AVM_LOG_EVT_SRC("TMR", NCSFL_SEV_INFO);
         rc = avm_proc_tmr(evt, avm_cb);
      }
      break;

      case AVM_EVT_RDE:
      {
         m_AVM_LOG_EVT_SRC("RDE", NCSFL_SEV_INFO);
         rc =  avm_proc_rde(evt, avm_cb);
      }
      break;
      
      case AVM_EVT_MDS:
      {
         m_AVM_LOG_EVT_SRC("MDS", NCSFL_SEV_INFO);
         rc = avm_proc_mds(evt, avm_cb);
      }    
      break;

      case AVM_EVT_FUND:
      {
         m_AVM_LOG_EVT_SRC("FUND", NCSFL_SEV_INFO);
         rc = avm_proc_fund(evt, avm_cb);
      }
      break;

      default:
      {
         m_AVM_LOG_EVT_SRC("INVALID PROVINCE", NCSFL_SEV_WARNING);
         rc =  NCSCC_RC_SUCCESS;
      }
   }

   if((AVM_EVT_NULL != evt) && (AVM_EVT_HPI != evt->src))
   {
      m_MMGR_FREE_AVM_EVT(evt);
   }
   return rc;
}

/***********************************************************************
 ******
 * Name          : avm_proc_hpi
 *
 * Description   : This function is invoked when  an event is received
 *                 at AvM  from HPI
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
static uns32
avm_proc_hpi(
               AVM_EVT_T *hpi_evt,
               AVM_CB_T  *avm_cb
            )
{
   AVM_ENT_INFO_T *ent_info;
   AVM_ENT_INFO_T  temp_ent_info;
   uns32           rc;
   uns8 str[AVM_LOG_STR_MAX_LEN]; 

   ent_info = avm_find_ent_info(avm_cb, &hpi_evt->evt.hpi_evt->entity_path);

   if(AVM_ENT_INFO_NULL == ent_info)
   {
      m_AVM_LOG_GEN("No Entity in the DB with Entity Path ", hpi_evt->evt.hpi_evt->entity_path.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_WARNING);

      m_NCS_MEMCPY(temp_ent_info.entity_path.Entry, hpi_evt->evt.hpi_evt->entity_path.Entry, sizeof(SaHpiEntityPathT));

      return NCSCC_RC_FAILURE;
   }

   if(ent_info->row_status != NCS_ROW_ACTIVE)
   {
      m_AVM_LOG_GEN_EP_STR("Row Not Active", ent_info->ep_str.name, NCSFL_SEV_WARNING);
      avm_hisv_api_cmd(ent_info, HISV_RESOURCE_POWER, HISV_RES_POWER_OFF);
      ent_info->previous_state =  ent_info->current_state;
      ent_info->current_state  =  AVM_ENT_NOT_PRESENT;
      return NCSCC_RC_FAILURE;
   }

   avm_map_hpi2fsm(avm_cb, hpi_evt->evt.hpi_evt, &hpi_evt->fsm_evt_type, ent_info);

   if(AVM_EVT_INVALID == hpi_evt->fsm_evt_type)
   {
      return NCSCC_RC_FAILURE;
   }else
   {
      if(AVM_EVT_IGNORE == hpi_evt->fsm_evt_type)
      {
         return NCSCC_RC_SUCCESS;
      }
   }

   rc = avm_fsm_handler(avm_cb, ent_info, hpi_evt);

   m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_STATE);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_DEBUG("HPI fsm handler failed", NCSFL_SEV_WARNING);
      return rc;
   }

   m_AVM_LOG_DEBUG("HPI fsm handler Successful", NCSFL_SEV_NOTICE);

   /**************************************************************
      Check if the HotSwapState is SAHPI_HS_STATE_INACTIVE and 
      the blade is locked for upgrading IPMC. If so,
      log it and call the upgrade ipmc function
   **************************************************************/ 
   if ((hpi_evt->evt.hpi_evt->hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_INACTIVE) &&
       (IPMC_BLD_LOCKED == ent_info->dhcp_serv_conf.ipmc_upgd_state))
   {
      str[0] = '\0';
      sprintf(str,"AVM-SSU: Payload blade %s: IPMC UPGRADE TRIGGERED AS BLADE GOT LOCKED",
                     ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_NOTICE);
      avm_upgrade_ipmc(avm_cb,ent_info);
   }
 
   return rc;
}


/***********************************************************************
 ******
 * Name          : avm_proc_avd
 *
 *
 * Description   : This function is invoked when  an event is received
 *                 at AvM  from AvD
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
static uns32
avm_proc_avd(
               AVM_EVT_T *avd_avm,
               AVM_CB_T  *avm_cb
            )
{
   AVM_NODE_INFO_T  *node_info = AVM_NODE_INFO_NULL;
   SaNameT           node_name;
   uns32             rc = NCSCC_RC_SUCCESS;


   m_AVM_LOG_FUNC_ENTRY("avm_proc_avd");

   if(AVD_AVM_D_HRT_BEAT_LOST_MSG ==  avd_avm->evt.avd_evt->msg_type)
   {
      m_NCS_DBG_PRINTF("Heart Beat lost in AvM");
      avd_avm->fsm_evt_type = AVM_ROLE_EVT_AVD_HB;
      rc = avm_role_fsm_handler(avm_cb, avd_avm);
      return rc;
   }
   else if(AVD_AVM_ND_HRT_BEAT_LOST_MSG ==  avd_avm->evt.avd_evt->msg_type)
   {
      avd_avm->fsm_evt_type = AVM_ROLE_EVT_AVND_HB;
      rc = avm_role_fsm_handler(avm_cb, avd_avm);
      return rc;
   }
   else if(AVD_AVM_SYS_CON_ROLE_ACK_MSG == avd_avm->evt.avd_evt->msg_type)
   {
      avd_avm->fsm_evt_type = AVM_ROLE_EVT_AVD_ACK;
      rc = avm_role_fsm_handler(avm_cb, avd_avm);
      return rc;
   }
   else if(AVD_AVM_D_HRT_BEAT_RESTORE_MSG ==  avd_avm->evt.avd_evt->msg_type)
   {
      m_NCS_DBG_PRINTF("Heart Beat Restore in AvM");
      avd_avm->fsm_evt_type = AVM_ROLE_EVT_AVD_HB_RESTORE;
      rc = avm_role_fsm_handler(avm_cb, avd_avm);
      return rc;
   }
   else if(AVD_AVM_ND_HRT_BEAT_RESTORE_MSG ==  avd_avm->evt.avd_evt->msg_type)
   {
      avd_avm->fsm_evt_type = AVM_ROLE_EVT_AVND_HB_RESTORE;
      rc = avm_role_fsm_handler(avm_cb, avd_avm);
      return rc;
   }

   avm_map_avd2fsm(avd_avm, avd_avm->evt.avd_evt->msg_type, &avd_avm->fsm_evt_type, &node_name);
   if(AVM_EVT_INVALID == avd_avm->fsm_evt_type)
   {
      m_AVM_LOG_EVT(AVM_LOG_EVT_INVALID, "AVD", NCSFL_SEV_WARNING);
      return NCSCC_RC_FAILURE;
   }

   node_info  = avm_find_node_name_info(avm_cb, node_name);
   if(AVM_NODE_INFO_NULL == node_info)
   {
      m_AVM_LOG_GEN("No Entity in the DB with Node Name ", node_name.value, SA_MAX_NAME_LENGTH, NCSFL_SEV_WARNING);
      avm_handle_avd_error(avm_cb, AVM_ENT_INFO_NULL,  avd_avm);
      return NCSCC_RC_FAILURE;
   }

   if(node_info->ent_info->row_status != NCS_ROW_ACTIVE)
   {
      m_AVM_LOG_GEN_EP_STR("Row Not Active", node_info->ent_info->ep_str.name, NCSFL_SEV_WARNING);
      return NCSCC_RC_FAILURE;
   }

   rc = avm_fsm_handler(avm_cb, node_info->ent_info, avd_avm);
   m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, node_info->ent_info, AVM_CKPT_ENT_STATE);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_DEBUG("AvD fsm handler failed", NCSFL_SEV_WARNING);
      return rc;
   }
   m_AVM_LOG_DEBUG("AvD fsm handler Successful", NCSFL_SEV_INFO);

   return rc;
}

/***********************************************************************
 ******
 * Name          : avm_proc_bam
 *
 *
 * Description   : This function is invoked when  an event is received
 *                 at AvM  from BAM
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
static uns32
avm_proc_bam(
               AVM_EVT_T *bam_avm,
               AVM_CB_T  *avm_cb
            )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_proc_bam");
   switch(bam_avm->evt.bam_evt->msg_type)
   {
     case BAM_AVM_CFG_DONE_MSG:
     {
 
       if(AVM_CONFIG_DONE == avm_cb->config_state)
       {
          m_AVM_LOG_DEBUG("BAM Conf Done Twice ", NCSFL_SEV_WARNING);
          return NCSCC_RC_SUCCESS;
       } 
       if(avm_cb->config_cnt != bam_avm->evt.bam_evt->msg_info.msg.check_sum)
       {
           m_NCS_NID_NOTIFY(NID_SCAP_ERR1);
       }
        avm_cb->config_state = AVM_CONFIG_DONE;

        rc = avm_add_root(avm_cb);
        rc = avm_check_deployment(avm_cb);
        if(NCSCC_RC_FAILURE == rc)
        {
           m_AVM_LOG_DEBUG("Configuration Wrong", NCSFL_SEV_EMERGENCY);
           return NCSCC_RC_FAILURE;
        }
     }
     break;

     /* Future */
     case BAM_AVM_HW_ENT_INFO:
     {
        rc = avm_bld_validation_info(avm_cb, bam_avm->evt.bam_evt->msg_info.ent_info);
        if(NCSCC_RC_SUCCESS == rc)
        {
           if(NCSCC_RC_SUCCESS   != avm_mab_send_warmboot_req(avm_cb))
           {
              m_AVM_LOG_MAS(AVM_LOG_OAC_WARMBOOT, AVM_LOG_MAS_FAILURE, NCSFL_SEV_CRITICAL);
              return NCSCC_RC_FAILURE;
           }
           m_AVM_LOG_MAS(AVM_LOG_OAC_WARMBOOT, AVM_LOG_MAS_SUCCESS, NCSFL_SEV_INFO);
        }
     }
     break;
     default:
     {
        m_AVM_LOG_INVALID_VAL_ERROR(bam_avm->evt.bam_evt->msg_type);
        rc = NCSCC_RC_FAILURE;
     }
   }
   return rc;
}


/***********************************************************************
 ******
 * Name          : avm_proc_mib
 *
 *
 * Description   : This function is invoked when  an event is received
 *                 at AvM  from MIB
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
static uns32
avm_proc_mib(
               AVM_EVT_T *mib_req,
               AVM_CB_T  *avm_cb
            )
{

   NCSMIBLIB_REQ_INFO  miblib_req;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_proc_mib");

   if (mib_req->evt.mib_req == NULL)
   {
      m_AVM_LOG_EVT(AVM_LOG_EVT_INVALID, "MAB", NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

  if((AVM_CONFIG_DONE == avm_cb->config_state) &&
     (mib_req->evt.mib_req->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME ))
  {
   /* On demand PSS playback not supported */
      m_AVM_LOG_DEBUG("On demand playback not supported", NCSFL_SEV_NOTICE);
      return NCSCC_RC_FAILURE;
  }


   m_NCS_MEMSET(&miblib_req, '\0', sizeof(NCSMIBLIB_REQ_INFO));

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
   miblib_req.info.i_mib_op_info.args = mib_req->evt.mib_req;
   miblib_req.info.i_mib_op_info.cb = avm_cb;

   if(avm_cb->config_state != AVM_CONFIG_NOT_DONE)
   {
     avm_cb->config_cnt++;
   }

   /* call the mib routine handler */
   ncsmiblib_process_req(&miblib_req);

   if((m_NCSMIB_PSSV_PLBCK_IS_LAST_EVENT(mib_req->evt.mib_req->i_policy) == TRUE) &&
      (AVM_CONFIG_NOT_DONE == avm_cb->config_state))
   {
     avm_cb->config_state = AVM_CONFIG_DONE;
     rc = avm_add_root(avm_cb);
     rc = avm_check_deployment(avm_cb);
     if(NCSCC_RC_FAILURE == rc)
     {
        m_AVM_LOG_DEBUG("Configuration Wrong", NCSFL_SEV_EMERGENCY);
        return NCSCC_RC_FAILURE;
     }
   }

   /*
    * Free the MIB arg structure. This MIB arg is a
    * copy made by us, Since we have already processed it free it.
    */

   ncsmib_memfree(mib_req->evt.mib_req);
   mib_req->evt.mib_req = NULL;

   return rc;
}

/***********************************************************************
 ******
 * Name          : avm_proc_tmr
 *
 *
 * Description   : This function is invoked when  an event is received
 *                 at AvM  from TMR
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
static uns32
avm_proc_tmr(
               AVM_EVT_T *tmr,
               AVM_CB_T  *avm_cb
            )
{

   uns32 rc;
   NCS_SEL_OBJ        temp_eda_sel_obj;

   if(avm_cb->eda_init_tmr.status == AVM_TMR_STOPPED)
   {
      m_AVM_LOG_TMR_IGN(AVM_LOG_TMR_EXPIRED, "Already stopped. Ignore it",  NCSFL_SEV_INFO);
      return NCSCC_RC_SUCCESS;
   }

   if( AVM_EDA_DONE == avm_cb->eda_init)
    {
       m_AVM_LOG_DEBUG("Wrong EDA timer Exp. Event, EDA is already initialized ",NCSFL_SEV_ERROR);
       return NCSCC_RC_SUCCESS;
    }

   rc = avm_eda_initialize(avm_cb);

   switch(rc)
   {
      case AVM_EDA_FAILURE:
      {
         m_AVM_INIT_EDA_TMR_START(avm_cb);
      }
      break;

      case NCSCC_RC_FAILURE:
      {
         m_AVM_LOG_EDA(AVM_LOG_EDA_CREATE, AVM_LOG_EDA_FAILURE, NCSFL_SEV_CRITICAL);
      }
      break;

      case NCSCC_RC_SUCCESS:
      {
         m_SET_FD_IN_SEL_OBJ(avm_cb->eda_sel_obj, temp_eda_sel_obj);
         m_NCS_SEL_OBJ_SET(temp_eda_sel_obj, &avm_cb->sel_obj_set);
         avm_cb->sel_high = m_GET_HIGHER_SEL_OBJ(temp_eda_sel_obj, avm_cb->sel_high);
         avm_cb->eda_init = AVM_EDA_DONE;

         m_AVM_LOG_DEBUG("EDA sel obj set in CB", NCSFL_SEV_INFO);
         avm_stop_tmr(avm_cb, &tmr->evt.tmr);
         m_AVM_LOG_EDA(AVM_LOG_EDA_CREATE, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);
      }
      break;

      default:
      {
         m_AVM_LOG_DEBUG("Debug avm_eda.c", NCSFL_SEV_CRITICAL);
      }
   }

   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_proc_rde
 *
 *
 * Description   : This function is invoked when  an event is received
 *                 at AvM  from BAM
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
static uns32
avm_proc_rde(
               AVM_EVT_T  *rde_evt,
               AVM_CB_T   *avm_cb
            )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   rde_evt->fsm_evt_type = AVM_ROLE_EVT_RDE_SET;
   rc = avm_role_fsm_handler(avm_cb, rde_evt);

   return rc;
}

/***********************************************************************
 ******
 * Name          : avm_proc_mds
 *
 *
 * Description   : This function is invoked when  an event is received
 *                 at AvM  from MDS
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
static uns32
avm_proc_mds(
               AVM_EVT_T *mds,
               AVM_CB_T  *avm_cb
            )
{

   uns32 rc = NCSCC_RC_SUCCESS;

   if(mds->fsm_evt_type == AVM_ROLE_EVT_AVD_UP)
   {
      rc = avm_avd_role(avm_cb, avm_cb->ha_state, AVM_INIT_ROLE);
      return rc;
   }

   rc =  avm_role_fsm_handler(avm_cb, mds);
   return rc;
}

/***********************************************************************
 ******
 * Name          : avm_fsm_handler
 *
 *
 * Description   : This function maps to AvM FSM handler functions
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
extern uns32
avm_fsm_handler(
                  AVM_CB_T       *avm_cb,
                  AVM_ENT_INFO_T *ent_info,
                  AVM_EVT_T      *fsm_evt
               )
{
   uns32 rc = NCSCC_RC_FAILURE;

   if(((ent_info->current_state >= AVM_ENT_NOT_PRESENT) && (ent_info->current_state <= AVM_ENT_INVALID)) &&
       ((fsm_evt->fsm_evt_type    >= AVM_EVT_FRU_INSTALL) && (fsm_evt->fsm_evt_type   <= AVM_EVT_INVALID)))
   {
      if((avm_fsm_table_g[ent_info->current_state][fsm_evt->fsm_evt_type].current_state
                                                           == ent_info->current_state) &&
        (avm_fsm_table_g[ent_info->current_state][fsm_evt->fsm_evt_type].evt
                                                           == fsm_evt->fsm_evt_type))
      {
         rc = avm_fsm_table_g[ent_info->current_state][fsm_evt->fsm_evt_type].f_ptr(avm_cb, ent_info, fsm_evt);
      }else
      {
         m_AVM_LOG_DEBUG("Debug AvM Code", NCSFL_SEV_CRITICAL);
      }
   }else
   {
      m_AVM_LOG_DEBUG("Debug AvM Code", NCSFL_SEV_CRITICAL);
   }
   return rc;
}

/***********************************************************************
 ******
 * Name          : avm_role_fsm_handler
 *
 *
 * Description   : This function maps to AvM Role FSM handler functions
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
extern uns32
avm_role_fsm_handler(
                     AVM_CB_T       *cb,
                     AVM_EVT_T      *fsm_evt
                   )
{
   uns32 rc = NCSCC_RC_FAILURE;

   if(((cb->ha_state >= SA_AMF_HA_ACTIVE) && (cb->ha_state <= SA_AMF_HA_QUIESCING)) &&
       ((fsm_evt->fsm_evt_type  >= AVM_ROLE_EVT_AVM_CHG) && (fsm_evt->fsm_evt_type  <= AVM_ROLE_EVT_MAX)))
   {
         rc = avm_role_fsm_table_g[cb->ha_state - 1][fsm_evt->fsm_evt_type](cb, fsm_evt);
   }else
   {
      m_AVM_LOG_DEBUG("Debug AvM Code", NCSFL_SEV_CRITICAL);
   }
   return rc;
}


/***********************************************************************
 ******
 * Name          : avm_proc_ssu
 *
 * Description   : This function is invoked when  an event is received
 *                 at AvM  from HPI for first time SSU processing
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
static uns32
avm_proc_ssu(
               AVM_EVT_T *hpi_evt,
               AVM_CB_T  *avm_cb
            )
{
   uns32 rc = NCSCC_RC_FAILURE;
   AVM_EVT_T     *ssu_evt = AVM_EVT_NULL;

   /* create a duplicate packet and enqueue the message for SSU timer */
   ssu_evt = (AVM_EVT_T*) m_MMGR_ALLOC_AVM_EVT;
   if(ssu_evt == AVM_EVT_NULL)
   {
      m_AVM_LOG_MEM(AVM_LOG_EVT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      m_NCS_DBG_PRINTF("\n Malloc failed for AvM Evt");
      return rc;
   }
   ssu_evt->src = AVM_EVT_SSU;
   ssu_evt->data_len = hpi_evt->data_len;

   ssu_evt->ssu_evt_id.evt_id = hpi_evt->ssu_evt_id.evt_id;
   ssu_evt->ssu_evt_id.src = hpi_evt->ssu_evt_id.src;

   /* Alloc Data Storage */
   ssu_evt->evt.hpi_evt = m_MMGR_ALLOC_AVM_DEFAULT_VAL(hpi_evt->data_len + 1);

   if(NULL == ssu_evt->evt.hpi_evt)
   {
      m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return rc;
   }
   m_NCS_MEMSET(ssu_evt->evt.hpi_evt, 0, hpi_evt->data_len+1);
   m_NCS_MEMCPY(ssu_evt->evt.hpi_evt, hpi_evt->evt.hpi_evt, hpi_evt->data_len);

   if (m_NCS_IPC_SEND(&avm_cb->ssu_mbx, ssu_evt, NCS_IPC_PRIORITY_HIGH)
       != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_AVM_DEFAULT_VAL(ssu_evt->evt.hpi_evt);
      m_MMGR_FREE_AVM_EVT(ssu_evt);
      m_AVM_LOG_MBX(AVM_LOG_MBX_SEND, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
   }
   /* start the timer if it is not already running */
   if (avm_cb->ssu_tmr.status != AVM_TMR_RUNNING)
      m_AVM_SSU_TMR_START(avm_cb);

   return NCSCC_RC_SUCCESS;
}


/***********************************************************************
 ******
 * Name          : avm_proc_ssu_tmr_exp
 *
 * Description   : This function is invoked when  an event is received
 *                 back after timer expiry for SSU processing
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
static uns32
avm_proc_ssu_tmr_exp(
               AVM_EVT_T *hpi_evt,
               AVM_CB_T  *avm_cb
            )
{
   uns32 rc = NCSCC_RC_FAILURE;
   uns32 dhconf_chg = 0;
   m_AVM_LOG_FUNC_ENTRY("avm_tmr_exp");

   /* retrieve AvM CB */
   if(avm_cb == AVM_CB_NULL)
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   /* retrieve and process the messages from SSU Mail box */
   while ((hpi_evt = (AVM_EVT_T*)m_NCS_IPC_NON_BLK_RECEIVE(&(avm_cb->ssu_mbx), hpi_evt))
        != AVM_EVT_NULL)
   {
      /* reconfigure DHCP server for all insertion pending events */
      if (avm_ssu_dhcp_tmr_reconfig (avm_cb, hpi_evt) == AVM_DHCP_CONF_CHANGE)
         dhconf_chg = AVM_DHCP_CONF_CHANGE;
      /* process hpi events */
      rc = avm_proc_hpi(hpi_evt, avm_cb);

      /* send ack to passive */
      m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, &hpi_evt->ssu_evt_id, AVM_CKPT_EVT_ID);

      /* free data storage */
      if(NULL != hpi_evt->evt.hpi_evt)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(hpi_evt->evt.hpi_evt);
      }
      if(AVM_EVT_NULL != hpi_evt)
      {
         m_MMGR_FREE_AVM_EVT(hpi_evt);
      }
   }
   avm_cb->ssu_tmr.status = AVM_TMR_EXPIRED;
   avm_stop_tmr(avm_cb, &avm_cb->ssu_tmr);
   m_NCS_TASK_SLEEP(100);
   /* Invoke script to stop the DHCP server, if its configuration has been changed */
   if (dhconf_chg == AVM_DHCP_CONF_CHANGE)
      if (m_NCS_SYSTEM(AVM_DHCPD_SCRIPT AVM_DHCP_SCRIPT_ARG));
      {
         return NCSCC_RC_FAILURE;
      }
   return NCSCC_RC_SUCCESS;
}


/***********************************************************************
 ******
 * Name          : avm_proc_upgd_succ_tmr_exp
 *
 * Description   : This function is invoked when  an event is received
 *                 back after timer expiry for software upgrade success.
 *                 It is a upgrade Error handler timer.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
static uns32
avm_proc_upgd_succ_tmr_exp(
               AVM_EVT_T *my_evt,
               AVM_CB_T  *avm_cb
            )
{
   AVM_ENT_INFO_T *ent_info;
   uns8           logbuf[500];

   m_AVM_LOG_FUNC_ENTRY("avm_proc_upgd_succ_tmr_exp");

   /* retrieve AvM CB */
   if(avm_cb == AVM_CB_NULL)
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   /* Take the entity handle to retrive DHCP related stuff */
   if(NULL == (ent_info = 
      (AVM_ENT_INFO_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, my_evt->evt.tmr.ent_hdl)))
   {
      return NCSCC_RC_FAILURE;
   }

   avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);
   /* Rollback to old committed label */
   sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Upgrade Failed with %s, BootProgressStatus=%x",
                 ent_info->ep_str.name,ent_info->dhcp_serv_conf.curr_act_label->name.name,ent_info->boot_prg_status);
   m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);

   avm_ssu_dhcp_rollback(avm_cb, ent_info);

   ent_info->dhcp_serv_conf.upgd_prgs = FALSE;
   m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);

   sysf_sprintf(logbuf, "AVM-SSU: Payloadblade %s : Rolling back to %s",
                         ent_info->ep_str.name,ent_info->dhcp_serv_conf.curr_act_label->name.name);
   m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);

   ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_proc_boot_succ_tmr_exp
 *
 * Description   : This function is invoked when  an event is received
 *                 back after timer expiry for software boot success.
 *                 It is a boot Error handler timer.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
static uns32
avm_proc_boot_succ_tmr_exp(
               AVM_EVT_T *my_evt,
               AVM_CB_T  *avm_cb
            )
{
   AVM_ENT_INFO_T *ent_info;
   uns8           logbuf[500];
   uns32 chassis;
   uns8  *entity_path     = NULL;
   uns32 res = NCSCC_RC_SUCCESS;
   uns32 rc_fm;


   m_AVM_LOG_FUNC_ENTRY("avm_proc_boot_succ_tmr_exp");

   /* retrieve AvM CB */
   if(avm_cb == AVM_CB_NULL)
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   /* Take the entity handle to retrive DHCP related stuff */
   if(NULL == (ent_info = 
      (AVM_ENT_INFO_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, my_evt->evt.tmr.ent_hdl)))
   {
      return NCSCC_RC_FAILURE;
   }
   
   /* If standby Just stop the timer and return - Fix for 91104 */


   avm_stop_tmr(avm_cb, &ent_info->boot_succ_tmr);
   if((avm_cb->ha_state == SA_AMF_HA_STANDBY))
   {
     ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
     return NCSCC_RC_SUCCESS;
   }
   /* Fix for IR00084096 */
   avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmBootFailure_ID);
   sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : BootFailed for %s",
                       ent_info->ep_str.name,ent_info->node_name.value);
   m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);

   /* Fix for IR 85686. Call hpl_resource_reset to do cold reset of the node */

   res = avm_convert_entity_path_to_string(ent_info->entity_path, &entity_path);
   if(NCSCC_RC_SUCCESS != res)
   {
      m_AVM_LOG_GEN("Cant convert the entity path to string:", ent_info->entity_path.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   res = avm_find_chassis_id(&ent_info->entity_path, &chassis);
   if(NCSCC_RC_SUCCESS != res)
   {
      m_AVM_LOG_GEN_EP_STR("Cant get the Chassis id:", ent_info->ep_str.name,  NCSFL_SEV_ERROR);
      if(NULL != entity_path)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(entity_path);
      }
      return NCSCC_RC_FAILURE;
   }
   /* Do cold reset here, and if the returned value indicates a successful reset then inform AvD about the reset. */
   res = hpl_resource_reset(chassis, entity_path, HISV_RES_COLD_RESET);

   if(res == NCSCC_RC_SUCCESS)
      avm_avd_node_reset_resp(avm_cb, AVM_NODE_RESET_SUCCESS, ent_info->node_name);

   /* Send node reset indication to FM, so that FM starts timer to check boot progress */
   rc_fm = avm_notify_fm_node_reset(avm_cb, &ent_info->entity_path);

   m_NCS_SYSLOG (NCS_LOG_NOTICE, "avm_proc_boot_succ_tmr_exp(): AVM issued cold reset for payload - %s, return value of API: %d, sent reset indication to FMwith return value of API: %d\n", ent_info->ep_str.name, res, rc_fm);

   ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);

   if(NULL != entity_path)
   {
      m_MMGR_FREE_AVM_DEFAULT_VAL(entity_path);
   }

   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_proc_dhcp_fail_tmr_exp
 *
 * Description   : This function is invoked when  an event is received
 *                 back after timer expiry for dhconf script error.
 *                 It is a dhcp script failure handler timer.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
static uns32
avm_proc_dhcp_fail_tmr_exp(
               AVM_EVT_T *my_evt,
               AVM_CB_T  *avm_cb
            )
{
   uns32          rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_proc_dhcp_fail_tmr_exp");

   /* retrieve AvM CB */
   if(avm_cb == AVM_CB_NULL)
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_DEBUG("AVM-SSU: DHCP FAIL timer expired",NCSFL_SEV_NOTICE);

   avm_stop_tmr(avm_cb, &avm_cb->dhcp_fail_tmr);
   
   if ((rc = (m_NCS_SYSTEM(AVM_DHCPD_SCRIPT AVM_DHCP_SCRIPT_ARG))))
   {
      m_AVM_LOG_DEBUG("AVM-SSU: DHCP STOP failed again. Restarting DHCP FAIL timer.",NCSFL_SEV_ERROR);
      m_AVM_SSU_DHCP_FAIL_TMR_START(avm_cb);
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_DEBUG("AVM-SSU: DHCP STOP executed Succesfully",NCSFL_SEV_NOTICE);

   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_proc_bios_upgrade_tmr_exp
 *
 * Description   : This function is invoked when  an event is received
 *                 back after timer expiry for bios upgrade timer.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
static uns32
avm_proc_bios_upgrade_tmr_exp(
               AVM_EVT_T *my_evt,
               AVM_CB_T  *avm_cb
            )
{
   AVM_ENT_INFO_T *ent_info;
   uns8           logbuf[500];
   uns32 rc;
   logbuf[0] = '\0';

   m_AVM_LOG_FUNC_ENTRY("avm_proc_bios_upgrade_tmr_exp");

   /* retrieve AvM CB */
   if(avm_cb == AVM_CB_NULL)
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   /* Take the entity handle to retrive DHCP related stuff */
   if(NULL == (ent_info =
      (AVM_ENT_INFO_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, my_evt->evt.tmr.ent_hdl)))
   {
      return NCSCC_RC_FAILURE;
   }
   avm_stop_tmr(avm_cb, &ent_info->bios_upgrade_tmr);
   
   if(ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
   {
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);
      ent_info->dhcp_serv_conf.bios_upgd_state = BIOS_TMR_EXPIRED;
      m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);

      sysf_sprintf(logbuf, "AVM-SSU: Payload  %s :Bios Upgrade Tmr Exp :Switch Bios Bank", ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
      rc = avm_hisv_api_cmd(ent_info, HISV_PAYLOAD_BIOS_SWITCH, 0);

      if (rc != NCSCC_RC_SUCCESS)
      {
         logbuf[0] = '\0';
         ent_info->dhcp_serv_conf.bios_upgd_state = 0;   /* clear the state */
         m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);

         sysf_sprintf(logbuf, "AVM-SSU: Payload %s :Bios Upgrade Tmr Exp :Failed to switch Bios Bank", ent_info->ep_str.name);
         m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_CRITICAL);
         ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
         return NCSCC_RC_FAILURE;
      }
      /* Rollback the NCS and Rollback the IPMC if required */
      avm_ssu_dhcp_rollback(avm_cb, ent_info);
      ent_info->dhcp_serv_conf.upgd_prgs = FALSE;
      ent_info->dhcp_serv_conf.bios_upgd_state = 0;   /* Role back done */
      m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
      /* async update */
      m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);
   } 
   else
   {
      ent_info->dhcp_serv_conf.bios_upgd_state = 0;
      m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);

      /* If it is not in upgrade case, might be in rollback case */  
      sysf_sprintf(logbuf, "AVM-SSU: Payload %s :Bios Upgrade Tmr Exp :Rollback Failed", ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_CRITICAL);
      ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}
/***********************************************************************
 ******
 * Name          : avm_proc_bios__failover_tmr_exp
 *
 * Description   : 
 *                 
 *
 * Arguments     :
 *             
 *           
 *          
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
static uns32
avm_proc_bios_failover_tmr_exp(
               AVM_EVT_T *my_evt,
               AVM_CB_T  *avm_cb
            )
{
   AVM_ENT_INFO_T *ent_info = NULL;
   uns8  *entity_path_str   = NULL;
   uns32 rc;
   uns32 chassis_id;
   uns8  bootbank_number;
   uns8 logbuf[AVM_LOG_STR_MAX_LEN];

   logbuf[0] = '\0';

   /* retrieve AvM CB */
   if(avm_cb == AVM_CB_NULL)
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   /* Take the entity handle to retrive DHCP related stuff */
   if(NULL == (ent_info =
      (AVM_ENT_INFO_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, my_evt->evt.tmr.ent_hdl)))
   {
      return NCSCC_RC_FAILURE;
   }
   avm_stop_tmr(avm_cb, &ent_info->bios_failover_tmr);

   
   if(ent_info->dhcp_serv_conf.bios_upgd_state)
   {
      rc = avm_convert_entity_path_to_string(ent_info->entity_path, &entity_path_str);

      if(NCSCC_RC_SUCCESS != rc)
      {
         m_AVM_LOG_GEN("Cant convert the entity path to string:", ent_info->entity_path.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_ERROR);
         ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
         return NCSCC_RC_FAILURE;
      }

      rc = avm_find_chassis_id(&ent_info->entity_path, &chassis_id);

     sysf_sprintf(logbuf, "AVM-SSU: Payload %s : Bios Failover Tmr Exp: Bios State=%d", ent_info->ep_str.name, 
                                                                                        ent_info->dhcp_serv_conf.bios_upgd_state);
     m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_DEBUG);

      switch(ent_info->dhcp_serv_conf.bios_upgd_state)
      {
            case BIOS_TMR_EXPIRED:
            {
               sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Going to switch Bios Bank", ent_info->ep_str.name);
               m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
               rc = avm_hisv_api_cmd(ent_info, HISV_PAYLOAD_BIOS_SWITCH, 0);


               if (rc != NCSCC_RC_SUCCESS)
               {
                  logbuf[0] = '\0';
                  ent_info->dhcp_serv_conf.bios_upgd_state = 0;   /* clear the state */
                  m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);

                  sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Failed to switch Bios Bank", ent_info->ep_str.name);
                  m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_CRITICAL);
                  ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
                  return NCSCC_RC_FAILURE;
               }
               /* Rollback the NCS and Rollback the IPMC if required */
               avm_ssu_dhcp_rollback(avm_cb, ent_info);
               ent_info->dhcp_serv_conf.upgd_prgs = FALSE;
               ent_info->dhcp_serv_conf.bios_upgd_state = 0;   /* Role back done */
               m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
               m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);
            }
            break;

            case BIOS_EXP_BANK_0:
            {
               rc = hpl_bootbank_get (chassis_id, entity_path_str, &bootbank_number);
               if (rc != NCSCC_RC_SUCCESS)
               {
                  sysf_sprintf(logbuf, "AVM-SSU: Payload %s : Bios Failover Tmr Exp: Get Bios Bank", ent_info->ep_str.name);
                  m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_ERROR);
                  break;
               }
               if (bootbank_number == 0)
               {
                  rc = hpl_bootbank_set (chassis_id, entity_path_str, 1);
               }

               if (rc != NCSCC_RC_SUCCESS)
               {
                  logbuf[0] = '\0';

                  ent_info->dhcp_serv_conf.bios_upgd_state = 0;   /* clear the state */
                  m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);

                  sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Failed to switch Bios Bank", ent_info->ep_str.name);
                  m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_CRITICAL);
                  ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
                  return NCSCC_RC_FAILURE;
               }
               /* Rollback the NCS and Rollback the IPMC if required */
               avm_ssu_dhcp_rollback(avm_cb, ent_info);
               ent_info->dhcp_serv_conf.upgd_prgs = FALSE;
               ent_info->dhcp_serv_conf.bios_upgd_state = 0;  /* switching done, reset the state */
               m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
               m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);

            }
            break;

            case BIOS_EXP_BANK_1:
            {
               rc = hpl_bootbank_get (chassis_id, entity_path_str, &bootbank_number);
               if (rc != NCSCC_RC_SUCCESS)
               {
                  sysf_sprintf(logbuf, "AVM-SSU: Payload %s : Bios Failover Tmr Exp: Get Bios Bank", ent_info->ep_str.name);
                  m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_ERROR);
                  break;
               }
               if (bootbank_number == 1)
               {
                  rc = hpl_bootbank_set (chassis_id, entity_path_str, 0);
               }

               if (rc != NCSCC_RC_SUCCESS)
               {
                  logbuf[0] = '\0';

                  ent_info->dhcp_serv_conf.bios_upgd_state = 0;   /* clear the state */
                  m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);

                  sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Failed to switch Bios Bank", ent_info->ep_str.name);
                  m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_CRITICAL);
                  ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
                  return NCSCC_RC_FAILURE;
               }

               /* Rollback the NCS and Rollback the IPMC if required */
               avm_ssu_dhcp_rollback(avm_cb, ent_info);
               ent_info->dhcp_serv_conf.upgd_prgs = FALSE;
               ent_info->dhcp_serv_conf.bios_upgd_state = 0;  /* switching done, reset the state */
               m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
               m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);
            }
            break;

            default:
            {
               sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Invalid bios_upgd_state:%d",
                                        ent_info->ep_str.name,ent_info->dhcp_serv_conf.bios_upgd_state);
               m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_CRITICAL);
            }
            break;
      }
   }
   ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
   return NCSCC_RC_SUCCESS;
}
