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
 
  DESCRIPTION:This module deals with handling of all events in all states they
  occur. 
  ..............................................................................

  Function Included in this Module: Too many to list.
 
 
******************************************************************************
*/


#include "avm.h"



static uns32
active_rde_set(
                 AVM_CB_T   *cb,
                 AVM_EVT_T  *evt
               );

static uns32
active_adm_switch(
                   AVM_CB_T   *cb,
                   AVM_EVT_T  *evt
                 );

static uns32
active_mds_quiesced_ack(
                         AVM_CB_T   *cb,
                         AVM_EVT_T  *evt
                       );

static uns32
active_avd_ack(
                  AVM_CB_T    *cb,
                  AVM_EVT_T   *evt
               );

static uns32
active_avd_hb_lost(
                   AVM_CB_T    *cb,
                   AVM_EVT_T   *evt  
                  );

static uns32
active_avnd_hb_lost(
                   AVM_CB_T    *cb,
                   AVM_EVT_T   *evt  
                  );

static uns32
quiesced_avnd_hb_lost(
                  AVM_CB_T    *cb,
                  AVM_EVT_T   *evt  
               );

static uns32
standby_avm_chg(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
                );

static uns32
standby_avd_ack(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
                );

static uns32
standby_rde_set(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
                );
static uns32
standby_avd_hb_lost(
                     AVM_CB_T   *cb,
                     AVM_EVT_T  *evt
                   );
static uns32
quiesced_rde_set(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
                 );
static uns32
quiesced_avm_rsp(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
                 );
static uns32
quiesced_avd_ack(  
                    AVM_CB_T    *cb,
                    AVM_EVT_T   *evt
                 );
static uns32
quiesced_avd_hb_lost(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
               );

static uns32
role_evt_invalid(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
               );

static uns32
role_evt_ignore(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
               );
static uns32
avm_role_chg(
              AVM_CB_T       *cb,
              SaAmfHAStateT   role
            );

static uns32
avm_mbc_role_chg(
              AVM_CB_T       *cb,
              SaAmfHAStateT   role
            );


/* Functions invoked at all possible AvM HA states. This is two dimensional
   array of First Dimension standing for Current HA-State of AvM
   and Second Dimention denoting the Event received from External Province.*/

AVM_ROLE_FSM_FUNC_PTR_T avm_role_fsm_table_g[][AVM_ROLE_EVT_MAX + 1] = 
{
   {
      role_evt_invalid,
      role_evt_ignore,
      active_rde_set,
      active_avd_ack,
      active_avd_hb_lost,
      active_avnd_hb_lost,
      active_adm_switch,
      role_evt_invalid,
      active_mds_quiesced_ack
   },

   {
      standby_avm_chg,
      role_evt_invalid,
      standby_rde_set,
      standby_avd_ack,
      standby_avd_hb_lost,
      role_evt_invalid,
      role_evt_invalid,
      role_evt_invalid,
      role_evt_invalid
   },

   {
      role_evt_invalid,
      quiesced_avm_rsp,
      quiesced_rde_set,
      quiesced_avd_ack,
      quiesced_avd_hb_lost,
      quiesced_avnd_hb_lost,
      role_evt_invalid,
      role_evt_invalid,
      role_evt_invalid
   },

   {
      role_evt_invalid,
      role_evt_invalid,
      role_evt_invalid,
      role_evt_invalid,
      role_evt_invalid,
      role_evt_invalid,
      role_evt_invalid,
      role_evt_invalid,
      role_evt_invalid
   }
};

/*************************************************************************
 * Function: active_rde_set
 *
 * Purpose:  This function is called Active SCXB receives role set from 
 *           Role Distribution Entity. When an admin switch is issued,
 *           AvM fisrt informs RDE. If RDE approves, then AvM proceeds further
 *           with carrying on admin switch  
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/

static uns32
active_rde_set(
                 AVM_CB_T   *cb,
                 AVM_EVT_T  *evt
               )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("active_rde_set");

   m_NCS_DBG_PRINTF("\n AvM Info: active_rde_set Rde-Role = %d, Rde-Ret Code = %d, Adm-Switch = %d\n", evt->evt.rde_evt.role, evt->evt.rde_evt.ret_code, cb->adm_switch);

   if(PCSRDA_RC_SUCCESS != evt->evt.rde_evt.ret_code)
   {
      m_NCS_DBG_PRINTF("\n RDE gave wrong return code in Active SCXB %d\n", evt->evt.rde_evt.ret_code);
      m_AVM_LOG_INVALID_VAL_FATAL(evt->evt.rde_evt.ret_code);
      if((PCSRDA_RC_FATAL_IPC_CONNECTION_LOST == evt->evt.rde_evt.ret_code) || (PCSRDA_RC_IPC_RECV_FAILED == evt->evt.rde_evt.ret_code))
      {
         avm_unregister_rda_cb(cb);
         rc = avm_register_rda_cb(cb);
         m_NCS_DBG_PRINTF("\n avm_register_rda_cb in Active SCXB %d\n", rc);
      }
      return rc;
   }

   /*Role Set is msg received from RDE */

  /* When an operator issues admin switch operation, AvM first informs 
   * RDE. If RDE approves AvM request, AvM proceeds with Admin Switch
   */
   
   /* This is an input check for RDE. 
      If it issues Active when AvM is in active
      We can cancel our admin operation*/
   if(PCS_RDA_ACTIVE == evt->evt.rde_evt.role)
   {
       m_NCS_DBG_PRINTF("\n RDE to go Quiesced on A-SCXB failed, hence cancelling switch Rde-Role=%d Adm-Switch=%d\n", evt->evt.rde_evt.role, cb->adm_switch);

       avm_mds_set_vdest_role(cb, SA_AMF_HA_ACTIVE);
       if(TRUE == cb->adm_switch)
       {
          avm_avd_role(cb, SA_AMF_HA_ACTIVE, AVM_FAIL_OVER);

          /* Process the events in Event Queue */
          avm_proc_evt_q(cb);
       } 

       cb->adm_switch     = FALSE;
       cb->adm_async_role_rcv = FALSE;
       m_AVM_LOG_INVALID_VAL_FATAL(evt->evt.rde_evt.ret_code);
       m_AVM_LOG_INVALID_VAL_FATAL(evt->evt.rde_evt.role);
       return rc;   
     
   }
   return rc;
}

/*************************************************************************
 * Function: active_adm_switch
 *
 * Purpose:  This function is called when operator isses admin switch over
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/

static uns32
active_adm_switch(
                   AVM_CB_T   *cb,
                   AVM_EVT_T  *evt
                 )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("active_adm_switch");

   /* Adm Switch is msg received at AvM from SNMP manager */

   /* If AvM is running over the platform check the alternate plane health
    * before proceeding with the switchover 
    */
   if(cb->is_platform == TRUE)
   {
      /* 
      ** Determine if you are residing on PLANE_A or PLANE_B
      ** Check the Health of other plane and other sam health status
      ** Allow Admin Switchover only if the other plane and sam are 
      ** healthy.
      */ 
#if 1
#define F101_SLOT_NW_PLANE_A 1

      FILE *fp = NULL;
      char line[80] = {0};
      uns32 slot_id = 0;
      
      fp = popen ("hpmcmd -c slotnumber", "r");
      if(fp == NULL)
      {
         m_NCS_DBG_PRINTF("Could not retrieve hpm slot info\n");
         m_AVM_LOG_ROLE(AVM_LOG_SWOVR_FAILURE, AVM_LOG_RDA_FAILURE, 
                        NCSFL_SEV_ERROR);
         pclose(fp);
         return NCSCC_RC_FAILURE;
      }
      else
      {
         do
         {
            if(fgets(line, sizeof(line), fp) != NULL )
            {
               slot_id = atoi(line);
            }
         }while(0);
      }
      pclose(fp);
#else

#define F101_SLOT_NW_PLANE_A 7 /* This is for 14 slot chassis  else 1*/

      NCS_NODE_ID node_id = m_NCS_GET_NODE_ID;
      NCS_CHASSIS_ID chassis;
      NCS_PHY_SLOT_ID slot_id;
      NCS_SUB_SLOT_ID sub_slot;
      m_NCS_GET_PHYINFO_FROM_NODE_ID(node_id, &chassis, &slot_id, &sub_slot);
#endif

      if(slot_id == F101_SLOT_NW_PLANE_A)
      {
         if((cb->hlt_status[HEALTH_STATUS_PLANE_B] == STATUS_UNHEALTHY) ||
            (cb->hlt_status[HEALTH_STATUS_SAM_B] == STATUS_UNHEALTHY) )
         {
            m_NCS_DBG_PRINTF("Health Status of PLANE_B : \n SAM: %d,  PLANE: %d\n", cb->hlt_status[HEALTH_STATUS_SAM_B], cb->hlt_status[HEALTH_STATUS_PLANE_B]);
            m_AVM_LOG_ROLE(AVM_LOG_SWOVR_FAILURE, AVM_LOG_RDA_FAILURE, 
                        NCSFL_SEV_ERROR);
            /* Generate a Trap syaing that switchover failed */
            avm_adm_switch_trap(cb, ncsAvmSwitch_ID, AVM_SWITCH_FAILURE);
            return NCSCC_RC_FAILURE;
         }
      }
      else if(slot_id != 0)
      {
         if((cb->hlt_status[HEALTH_STATUS_PLANE_A] == STATUS_UNHEALTHY) ||
            (cb->hlt_status[HEALTH_STATUS_SAM_A] == STATUS_UNHEALTHY) )
         {
            m_NCS_DBG_PRINTF("Health Status of PLANE_A : \n SAM: %d,  PLANE: %d\n", cb->hlt_status[HEALTH_STATUS_SAM_A], cb->hlt_status[HEALTH_STATUS_PLANE_A]);
            m_AVM_LOG_ROLE(AVM_LOG_SWOVR_FAILURE, AVM_LOG_RDA_FAILURE, 
                        NCSFL_SEV_ERROR);
            
            /* Generate a Trap saying that switch failed */
            avm_adm_switch_trap(cb, ncsAvmSwitch_ID, AVM_SWITCH_FAILURE);
            return NCSCC_RC_FAILURE;
         }
      }
      else
      {
         m_NCS_DBG_PRINTF("Could not retrieve proper slot info\n");
         m_AVM_LOG_ROLE(AVM_LOG_SWOVR_FAILURE, AVM_LOG_RDA_FAILURE, 
                        NCSFL_SEV_ERROR);

         /* Generate a Trap saying that switch failed */
         avm_adm_switch_trap(cb, ncsAvmSwitch_ID, AVM_SWITCH_FAILURE);
         return NCSCC_RC_FAILURE;
      }
   } /* End of Platform specific stuff */

   /* Set admin switch flag to true */
   if(TRUE == cb->adm_switch)
   {
      return rc;
   }

   cb->adm_switch =  TRUE;
   cb->adm_async_role_rcv = FALSE;

   /* First AvM informs RDE about role switchover 
    * AvM should inform RDE to go quiesced. But the current implementation
    * of RDE takes the role and will not send any asyn ack back and hence
    * proceed with the role switchover of the AVD also
    */

   if(NCSCC_RC_SUCCESS != (rc = avm_notify_rde_set_role(cb, SA_AMF_HA_QUIESCED)))
   {
      m_NCS_DBG_PRINTF("\n rde_set->Quiesced Failed on Active SCXB, hence cancelling switch ");
      m_AVM_LOG_INVALID_VAL_FATAL(rc);
      cb->adm_switch =  FALSE;
      avm_adm_switch_trap(cb, ncsAvmSwitch_ID, AVM_SWITCH_FAILURE);
      return rc;
   }

   rc = avm_avd_role(cb, SA_AMF_HA_QUIESCED, AVM_ADMIN_SWITCH_OVER);

   return rc;
   
}

/*************************************************************************
 * Function: active_avd_ack
 *
 * Purpose:  This function is called when AvM recives ack from AvD
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/
static uns32
active_avd_ack(
                  AVM_CB_T    *cb,
                  AVM_EVT_T   *evt
               )
{
   uns32 rc = NCSCC_RC_SUCCESS;
  
   m_AVM_LOG_FUNC_ENTRY("active_avd_ack");

   /* Role Ack is a message received at AvM from AvD. */

   /* AvD giving ack for init role. AvD accepted Init Role sent by AvM
    */ 
    
   m_AVM_LOG_ROLE_SRC(AVM_LOG_AVD_ROLE_RSP, evt->evt.avd_evt->avd_avm_msg.role_ack.role, evt->evt.avd_evt->avd_avm_msg.role_ack.rc, NCSFL_SEV_NOTICE);
   m_NCS_DBG_PRINTF("\n AvM Info: active_avd_ack AvD-Role = %d, AvD-Ret Code = %d, Adm-Switch = %d\n", evt->evt.avd_evt->avd_avm_msg.role_ack.role, evt->evt.avd_evt->avd_avm_msg.role_ack.rc, cb->adm_switch);

   if((SA_AMF_HA_ACTIVE == evt->evt.avd_evt->avd_avm_msg.role_ack.role)
      && (NCSCC_RC_SUCCESS == evt->evt.avd_evt->avd_avm_msg.role_ack.rc))
   {
      return NCSCC_RC_SUCCESS;
   }

   /* When AvM is in active state, It will inform AvD to go either Quiesced 
      or Active */

   /* AvM will not receive ack for any role from AvD in active */
   if((SA_AMF_HA_QUIESCED != evt->evt.avd_evt->avd_avm_msg.role_ack.role)
      || (NCSCC_RC_SUCCESS != evt->evt.avd_evt->avd_avm_msg.role_ack.rc) 
      || (FALSE == cb->adm_switch))
   {
      m_AVM_LOG_INVALID_VAL_ERROR(evt->evt.avd_evt->avd_avm_msg.role_ack.role);
      m_AVM_LOG_INVALID_VAL_ERROR(evt->evt.avd_evt->avd_avm_msg.role_ack.rc);
      m_AVM_LOG_INVALID_VAL_ERROR(cb->adm_switch);
      cb->adm_switch = FALSE;
      cb->adm_async_role_rcv = FALSE;
      m_NCS_DBG_PRINTF("\n rde_set->Active, AvD rejected switch request, hence cancelling switch ");
      rc = avm_notify_rde_set_role(cb, SA_AMF_HA_ACTIVE);
     
      /* Process the events in Event Queue */
      avm_proc_evt_q(cb);
      
      /* Generate a Trap about switch failure */
      avm_adm_switch_trap(cb, ncsAvmSwitch_ID, AVM_SWITCH_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   rc = avm_mds_set_vdest_role(cb, SA_AMF_HA_QUIESCED);

   return rc;
}

/*************************************************************************
 * Function: active_avd_hb_lost
 *
 * Purpose:  This function is called when AvM recives heart beat lost from 
 *           AvD
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/
static uns32
active_avd_hb_lost(
                  AVM_CB_T    *cb,
                  AVM_EVT_T   *evt  
               )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* HB is a message recived from AvD when it looses heart beat 
    * with other AvD. AvM will not make any decisions when it receives
      Heart Beat loss. It just inform RDE. */ 

   m_AVM_LOG_FUNC_ENTRY("active_avd_hb_lost");
   m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_HB, cb->ha_state, NCSFL_SEV_NOTICE);

   /* Inform RDE abt hrt bt lost */
   m_NCS_DBG_PRINTF("\n rde_hrt_bt_lost on Active \n");
   rc = avm_notify_rde_hrt_bt_lost(cb);
   
   return rc;
}

static uns32
quiesced_avnd_hb_lost(
                  AVM_CB_T    *cb,
                  AVM_EVT_T   *evt  
               )
{
   return NCSCC_RC_SUCCESS;
}

/*************************************************************************
 * Function: active_avnd_hb_lost
 *
 * Purpose:  This function is called when AvM recives heart beat lost from 
 *           AvD
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/
static uns32
active_avnd_hb_lost(
                  AVM_CB_T    *cb,
                  AVM_EVT_T   *evt  
               )
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVD_AVM_MSG_T *msg = NULL;
   uns32 node_id = 0;
   uns8  chassis_id=0, sub_slot_id=0, phy_slot_id=0;

   m_NCS_DBG_PRINTF("active_avnd_hb_lost \n\n");
   /* HB is a message recived from AvD when it looses heart beat 
    * with AvND. AvM will not make any decisions when it receives
      Heart Beat loss. It just inform LFM. */ 

   m_AVM_LOG_FUNC_ENTRY("active_avnd_hb_lost");
  
   /* Extract the node_id and convert it to phy_slot_id */
   if((msg = evt->evt.avd_evt) == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   node_id = msg->avd_avm_msg.avnd_hb_lost.node_id;

   /* Convert the logical node to Physical node */

   m_NCS_GET_PHYINFO_FROM_NODE_ID(node_id, &chassis_id,
                                  &phy_slot_id, &sub_slot_id);

   /* Inform RDE abt AvND hrt bt lost */
   rc = avm_notify_rde_nd_hrt_bt_lost(cb, phy_slot_id);
   
   if(cb->is_platform == FALSE)
   {
      avm_avd_node_reset_resp(cb,  AVM_NODE_RESET_SUCCESS, msg->avd_avm_msg.avnd_hb_lost.node_name);
   }
   
   return rc;
}

/*************************************************************************
 * Function: active_mds_quiesced_ack
 *
 * Purpose:  This function is called when operator isses admin switch over
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/

static uns32
active_mds_quiesced_ack(
                        AVM_CB_T   *cb,
                        AVM_EVT_T  *evt
                       )
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SaHpiEntityPathT entity_path;
   AVM_ENT_INFO_T *ent_info;

   m_AVM_LOG_FUNC_ENTRY("active_mds_quiesced_ack");

   m_NCS_DBG_PRINTF("\n MDS Quiesced Ack on Active %d\n", cb->adm_switch);
 
   /* Set admin switch flag to true */
   if(TRUE == cb->adm_switch)
   {
      m_NCS_DBG_PRINTF("\n SCXB becoming Quiesced and Informing S-SCXB to go Active\n");
        
      m_AVM_LOG_ROLE_OP(AVM_LOG_ROLE_QUIESCED, cb->ha_state, NCSFL_SEV_NOTICE);

      /* AvM changing the role to Quiesced. Changes MDS Vdest role to Quiesced*/
      if(NCSCC_RC_SUCCESS != (rc = avm_role_chg(cb, SA_AMF_HA_QUIESCED)))
      {
          m_AVM_LOG_INVALID_VAL_FATAL(rc);
          m_NCS_DBG_PRINTF("\n avm_role_chg->SA_AMF_HA_QUIESCED failed \n");
          return NCSCC_RC_FAILURE;
      }

      /* AvM changing MBCSv role to Quiesced */
      if(NCSCC_RC_SUCCESS != (rc = avm_mbc_role_chg(cb, SA_AMF_HA_QUIESCED)))
      {
          m_AVM_LOG_INVALID_VAL_FATAL(rc);
          m_NCS_DBG_PRINTF("\n avm_mbc_role_chg->SA_AMF_HA_QUIESCED failed \n");
          rc = NCSCC_RC_FAILURE;
      }
       
      /* If ACK is successful, inform standby AvM to go Active 
      */
      /* AvM sending other AvM Role Change message */
      if(NCSCC_RC_SUCCESS != (rc = avm_send_role_chg_notify_msg(cb, SA_AMF_HA_ACTIVE)))
      {
         m_NCS_DBG_PRINTF("\n Informing S-SCXB->Active failed\n");
         m_AVM_LOG_INVALID_VAL_FATAL(rc);
         return rc;
      }
      cb->ha_state = SA_AMF_HA_QUIESCED;

      m_NCS_MEMSET(entity_path.Entry, 0, sizeof(SaHpiEntityPathT));
      for(ent_info = avm_find_next_ent_info(cb, &entity_path); 
          ent_info != AVM_ENT_INFO_NULL; ent_info = avm_find_next_ent_info(cb, &entity_path))
      {
         m_NCS_MEMCPY(entity_path.Entry, ent_info->entity_path.Entry, sizeof(SaHpiEntityPathT));
         ent_info->power_cycle = FALSE;
      }
      m_AVM_LOG_ROLE(AVM_LOG_ROLE_QUIESCED, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_NOTICE);
   }   
   
   return rc;
}


/*************************************************************************
 * Function: standby_avm_chg
 *
 * Purpose:  This function is called when Standby AvM recives 
 *           role change msg from Active AvM
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/
static uns32
standby_avm_chg(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
                )
{
   uns32 rc = NCSCC_RC_SUCCESS;


   /* Role Change is message exchanged between AvMs incase of admin 
      switch.
      Active AvM->Standby AvM  */

   m_AVM_LOG_FUNC_ENTRY("standby_avm_chg");
   m_NCS_DBG_PRINTF("\n S-SCXB Received Role-Change from Q-SCXB\n");
  
   if(!cb->cold_sync) 
   {
      m_AVM_LOG_INVALID_VAL_FATAL(cb->cold_sync);
   }

   /* Incase of admin switch, AvM AvM can only inform Standby AvM to go 
      Active. Any other role is invalid and Error */
   if(evt->evt.avm_role_msg.role_info.role_chg.role == SA_AMF_HA_STANDBY)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(0);
      return NCSCC_RC_FAILURE;
   }

   /* Standby AvM first takes RDEs permission to go Active and then informs
      Standby AvD to go Active*/

   m_NCS_DBG_PRINTF("\n S-SCXB setting RDE-ROLE to go Active \n");
   rc =  avm_notify_rde_set_role(cb, evt->evt.avm_role_msg.role_info.role_chg.role);

   cb->adm_switch = TRUE;
   cb->adm_async_role_rcv = FALSE;

   return rc;
}

/*************************************************************************
 * Function: standby_avd_ack
 *
 * Purpose:  This function is called when Standby AvM recives 
 *           role ack msg from Standby AvD
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/
static uns32
standby_avd_ack(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
                )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Role Ack is a message exchanged between AvD and AvM lying on 
      same card*/

   m_AVM_LOG_FUNC_ENTRY("standby_avd_ack");

   /* Standby SCXB when it comes up first, AvM gets the role from RDE and 
      informs AvD on Standby SCXB to go Standby 
      Standby AvM received ack from standby AvD at init time */ 

   m_AVM_LOG_ROLE_SRC(AVM_LOG_AVD_ROLE_RSP, evt->evt.avd_evt->avd_avm_msg.role_ack.role, evt->evt.avd_evt->avd_avm_msg.role_ack.rc, NCSFL_SEV_NOTICE);

   m_NCS_DBG_PRINTF("\n AvM Info: standby_avd_ack AvD-Role = %d, AvD-Ret Code = %d, Adm-Switch = %d\n", evt->evt.avd_evt->avd_avm_msg.role_ack.role, evt->evt.avd_evt->avd_avm_msg.role_ack.rc, cb->adm_switch);

   /* this should change... the future rde part of code will never get to execute */
   if(SA_AMF_HA_STANDBY == evt->evt.avd_evt->avd_avm_msg.role_ack.role)
   {
      return NCSCC_RC_SUCCESS;
   }

   if(SA_AMF_HA_ACTIVE != evt->evt.avd_evt->avd_avm_msg.role_ack.role)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(evt->evt.avd_evt->avd_avm_msg.role_ack.role);
      m_NCS_DBG_PRINTF("\n S-SCXB Received AvD Ack with Invalid role %d\n", evt->evt.avd_evt->avd_avm_msg.role_ack.role);
      return NCSCC_RC_SUCCESS;  
   }

   /* In case of Admin switch or Heart beat loss on Standby SCXB,
      Standby AvM informs Standby AvD to go active */

   /* Standby AvM received ack from Standby AvD for active role */

   if(NCSCC_RC_SUCCESS == evt->evt.avd_evt->avd_avm_msg.role_ack.rc)
   {
      m_NCS_DBG_PRINTF("\n SCXB becoming Active \n");
      m_AVM_LOG_ROLE_OP(AVM_LOG_ROLE_ACTIVE, cb->ha_state, NCSFL_SEV_NOTICE);

      /* If Standby AvM receives Ack from Standby AvD to go Active,
         if an admin switch over has been triggered,
         Standby AvM has to inform other SCXB to go Standby 
         If there is a heart beat loss detected, S-AvM need not 
         inform other AvM.*/

      if(TRUE == cb->adm_switch)
      {
         cb->adm_switch = FALSE;
         cb->ha_state = SA_AMF_HA_ACTIVE;

         /* Standby AvM has to change it MDS Vdest role and has to empty
            its event queue maintained if there are any unprocessed events
            in the queue . */
         /* Standby AvM has to change it MBCSv role */
         rc =  avm_mbc_role_chg(cb, cb->ha_state);

         rc =  avm_role_chg(cb, SA_AMF_HA_ACTIVE);
         
         /* Standby AvM has inform other AvM to go Standby */
         rc =  avm_send_role_rsp_notify_msg(cb, SA_AMF_HA_ACTIVE, NCSCC_RC_SUCCESS);
         avm_adm_switch_trap(cb, ncsAvmSwitch_ID, AVM_SWITCH_SUCCESS);
      }else
      {

      /* Incase of heart beat loss, Standby AvM need not take the pain of informing
         other AvM. Its assumed that other AvM will be Reset */

         cb->ha_state = SA_AMF_HA_ACTIVE;
         cb->adm_switch = FALSE;
         cb->adm_async_role_rcv = FALSE;

         /* Standby AvM has to change it MBCSv role */ 
         rc =  avm_mbc_role_chg(cb, cb->ha_state);
         
         /* Standby AvM has to change it MDS Vdest role and has to empty
            its event queue maintained if there are any unprocessed events
            in the queue . */
         rc =  avm_role_chg(cb, SA_AMF_HA_ACTIVE);

      }
      m_AVM_LOG_ROLE_OP(AVM_LOG_ROLE_ACTIVE, cb->ha_state, NCSFL_SEV_NOTICE);
   }else
   {
      /* Future RDE*/
      m_NCS_DBG_PRINTF("\n S-SCXB Received AvD Ack with Failure ret-code %d\n", evt->evt.avd_evt->avd_avm_msg.role_ack.rc);
      
      /* Standby AvM first takes RDEs permission to go Standny and then informs
         Quiesced AvD to go Active*/
      
      cb->adm_switch = FALSE;

      rc =  avm_notify_rde_set_role(cb, SA_AMF_HA_STANDBY);
   }
   return rc;
}

/*************************************************************************
 * Function: standby_rde_set
 *
 * Purpose:  This function is called when Standby AvM recives 
 *           role set msg from RDE
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/
static uns32
standby_rde_set(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
                )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("standby_rde_set");

   /* Role Set is message exchanged between AvM on RDE on same card */

   /*If S-AvM receives Role Set from RDE to go Active,
     If there is Admin Switch triggered, AvM has to inform AvD with a cause
     AVM_ADMIN_SWITCH_OVER.
     If there is any heart beat loss detected,  AvM has to inform AvD with a cause
     AVM_FAILOVER */

   m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_SET_ROLE, evt->evt.rde_evt.role, NCSFL_SEV_NOTICE);

   m_NCS_DBG_PRINTF("\n AvM Info: standby_rde_set Rde-Role = %d, Rde-Ret Code = %d, Adm-Switch = %d\n", evt->evt.rde_evt.role, evt->evt.rde_evt.ret_code, cb->adm_switch);

   if(PCSRDA_RC_SUCCESS != evt->evt.rde_evt.ret_code)
   {
      m_NCS_DBG_PRINTF("\n S-SCXB Received Rde-Set with Failure ret-code %d\n", evt->evt.rde_evt.ret_code);
      if((PCSRDA_RC_FATAL_IPC_CONNECTION_LOST == evt->evt.rde_evt.ret_code) || (PCSRDA_RC_IPC_RECV_FAILED == evt->evt.rde_evt.ret_code))
      {
         avm_unregister_rda_cb(cb);
         rc = avm_register_rda_cb(cb);
         m_NCS_DBG_PRINTF("\n avm_register_rda_cb in Standby SCXB %d\n", rc);
      }
      return rc;
   }
 
   if(PCS_RDA_ACTIVE == evt->evt.rde_evt.role)
   {
      if(TRUE == cb->adm_switch)
      {
         /* Incase of admin switch AVM_ADMIN_SWITCH_OVER is the cause given to AvD */
         m_NCS_DBG_PRINTF("\n S-SCXB Informing AvD to go Active as switchover\n");
         rc = avm_avd_role(cb, SA_AMF_HA_ACTIVE, AVM_ADMIN_SWITCH_OVER);
         return NCSCC_RC_SUCCESS;
      }else
      {
         /* Incase of fail_over AVM_FAILOVER is the cause given to AvD */
         m_NCS_DBG_PRINTF("\n S-SCXB Informing AvD to go Active as failover\n");
         rc = avm_avd_role(cb, SA_AMF_HA_ACTIVE, AVM_FAIL_OVER);
      }
   }else
   {
      m_NCS_DBG_PRINTF("\n S-SCXB Received Rde-Set with role != PCS_RDA_ACTIVE \n", evt->evt.rde_evt.role);
      if(TRUE == cb->adm_switch)
      {
         m_NCS_DBG_PRINTF("\n S-SCXB Received Rde-Set with role != PCS_RDA_ACTIVE, hence cancelling switch%d\n", evt->evt.rde_evt.role);
         rc =  avm_send_role_rsp_notify_msg(cb, SA_AMF_HA_ACTIVE, NCSCC_RC_FAILURE);
         cb->adm_switch = FALSE;
      }
   }

   return rc;
}

/*************************************************************************
 * Function: standby_avd_hb_lost
 *
 * Purpose:  This function is called when Standby AvM recives 
 *           HRT beat lost from AvD 
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/
static uns32
standby_avd_hb_lost(
                    AVM_CB_T   *cb,
                    AVM_EVT_T  *evt
                   )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* hb_lost is a message exchanged between AvD and AvM lying on same card */
  
   m_AVM_LOG_FUNC_ENTRY("standby_avd_hb_lost");
   
   /* If Standby AvM recived heart beat loss message from Standby AvD, AvM 
      just informs about it to RDE.  */


   m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_HB, cb->ha_state, NCSFL_SEV_NOTICE);

   if(FALSE == cb->cold_sync)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(0);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_DBG_PRINTF("\n rde_hrt_bt_lost on Standby \n");
   rc = avm_notify_rde_hrt_bt_lost(cb);

   return rc;
}

/*************************************************************************
 * Function: quiesced_rde_set
 *
 * Purpose:  This function is called when Quiesced AvM receives 
 *           role set from rDE
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/
static uns32
quiesced_rde_set(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
                )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Role Set is message exchanged between RDE and AvM on same card */
   m_AVM_LOG_FUNC_ENTRY("quiesced_rde_set");
  
   /* If RDE sets the role to Standby when AvM is in Quiesced state
      that is perfect flow because, AvM goes to Quiesced only during
      Admin switchover */ 

   m_NCS_DBG_PRINTF("\n AvM Info: quiesced_rde_set Rde-Role = %d, Rde-Ret Code = %d, Adm-Switch = %d\n", evt->evt.rde_evt.role, evt->evt.rde_evt.ret_code, cb->adm_switch);

   m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_SET_ROLE, evt->evt.rde_evt.role, NCSFL_SEV_NOTICE);
   if(PCSRDA_RC_SUCCESS != evt->evt.rde_evt.ret_code)
   {
      m_NCS_DBG_PRINTF("\n Q-SCXB received rde-set with wrong return code %d\n", evt->evt.rde_evt.ret_code); 
      if((PCSRDA_RC_FATAL_IPC_CONNECTION_LOST == evt->evt.rde_evt.ret_code) || (PCSRDA_RC_IPC_RECV_FAILED == evt->evt.rde_evt.ret_code))
      {
         avm_unregister_rda_cb(cb);
         rc = avm_register_rda_cb(cb);
         m_NCS_DBG_PRINTF("\n avm_register_rda_cb in Quiesced SCXB %d\n", rc);
      }
      return rc;
   }

   if(PCS_RDA_STANDBY == evt->evt.rde_evt.role)
   {
      m_NCS_DBG_PRINTF("\n Standby in Quiesced state \n"); 

      /* just confirm that this is indeed in switchover context and 
       * mark that the RDE role has changed to standby and wait for
       * response from other AvM to go standby
       */
       if(cb->adm_switch == TRUE)
       {
          cb->adm_async_role_rcv = TRUE;
       }
      return rc;
   }else if (PCS_RDA_ACTIVE == evt->evt.rde_evt.role)
   {
         m_NCS_DBG_PRINTF("\n Q-SCXB received rde-set with PCS_RDA_ACTIVE, hence cancelling switch\n"); 
      /* If Quiesced AvM receives Role Set from RDE to go  Active, then 
         assume that other SCXB has gone wrong. Quiesced SCXB has to 
         go Active. 
         Quiesced AvM informs Quiesced AvD to go Active */

      /* One possibility of receiving Active role from RDE when in 
         Quiesced is, AvD could have detected heart beat loss.
         Make the flag false because RDE has responded with Active */ 
   
         cb->ha_state    = SA_AMF_HA_ACTIVE;
         cb->adm_switch  = FALSE;
     
         /* Inform AvD about role change */   
         rc = avm_avd_role(cb, SA_AMF_HA_ACTIVE, AVM_FAIL_OVER);
         m_NCS_DBG_PRINTF("\n Q-SCXB informing AvD to go Active as failover %d\n", rc); 

         /* AvM Has to set MBCSv role */
         rc = avm_mbc_role_chg(cb, SA_AMF_HA_ACTIVE);
      
         /* AvM has set its V-DEST role and process events in its Event
         Queue if there are any unprocessed events */

         rc = avm_role_chg(cb, SA_AMF_HA_ACTIVE);
   }
   return rc;
} 

/*************************************************************************
 * Function: quiesced_avm_rsp
 *
 * Purpose:  This function is called when Quisced AvM recives 
 *           role resp from then standby AvM 
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/
static uns32
quiesced_avm_rsp(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
                 )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Role Resp is a message exchanged between AvMs. Its a response to
      Role Change message triggered by once Active now Quiesced AvM. 
      Role Resp captures whether other AvM has succeeded or failed to
      become Active. */

   m_AVM_LOG_FUNC_ENTRY("quiesced_avm_rsp");

   /* Quisced AvM shall not receive this msg with any other role */
   if(SA_AMF_HA_ACTIVE != evt->evt.avm_role_msg.role_info.role_rsp.role)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(evt->evt.avm_role_msg.role_info.role_rsp.role);
      return NCSCC_RC_FAILURE;
   } 

   m_AVM_LOG_ROLE_OP(AVM_LOG_RCV_ROLE_RSP, evt->evt.avm_role_msg.role_info.role_rsp.role_rsp, NCSFL_SEV_NOTICE);

   /* When Quiesced AvM has a heart beat loss with other SCXB, role resp from other
      SCXB is ignored */

   /* If then standby AvM responds with success, Quiesced AvM informs 
    * Quisced AvD to go standby. Also, need to check if the AvM received
    * standby role from local RDE due to LHC protocol messaging. If received
    * the role is consistent and hence nothing to do else set the role
    * with STANDBY coz initially we sent Quiesced. */
   if(NCSCC_RC_SUCCESS == evt->evt.avm_role_msg.role_info.role_rsp.role_rsp)
   {
      m_NCS_DBG_PRINTF("\n SCXB becoming Standby \n");
      m_AVM_LOG_ROLE_OP(AVM_LOG_ROLE_STANDBY, cb->ha_state, NCSFL_SEV_NOTICE);

      rc = avm_avd_role(cb, SA_AMF_HA_STANDBY, AVM_ADMIN_SWITCH_OVER);
      cb->ha_state = SA_AMF_HA_STANDBY;
      if((cb->adm_switch == TRUE) && (cb->adm_async_role_rcv == FALSE))
      {
         /* ignore the return status for now */
         avm_notify_rde_set_role(cb, SA_AMF_HA_STANDBY);
      }
      else
      {
         cb->adm_async_role_rcv = FALSE;
      }
      rc = avm_mds_set_vdest_role(cb, SA_AMF_HA_STANDBY);
      rc = avm_mbc_role_chg(cb, cb->ha_state);
   }else
   {
      /* Other AvM has responded with a failure to go Active. Quiesced
         AvM has to go Active. AvM is informing AvD to go active */    
      m_NCS_DBG_PRINTF("\n SCXB becoming Active, S-SCXB rejected to go Standby\n");
      m_AVM_LOG_ROLE_OP(AVM_LOG_ROLE_ACTIVE, cb->ha_state, NCSFL_SEV_NOTICE);

      rc = avm_avd_role(cb, SA_AMF_HA_ACTIVE, AVM_ADMIN_SWITCH_OVER);
   }

   cb->adm_switch = FALSE; 
   cb->adm_async_role_rcv = FALSE;

   return rc;
} 

/*************************************************************************
 * Function: quiesced_avd_ack
 *
 * Purpose:  This function is called when Quisced AvM recives 
 *           role ack from then quisced AvD 
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
********************************************************************/
static uns32
quiesced_avd_ack(  
                    AVM_CB_T    *cb,
                    AVM_EVT_T   *evt
                 )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("quiesced_avd_ack");

   /* Role ack is message from AvD to AvM lying on same card */   
   m_NCS_DBG_PRINTF("\n Q-SCXB received AvD Ack %d %d \n", evt->evt.avd_evt->avd_avm_msg.role_ack.role, evt->evt.avd_evt->avd_avm_msg.role_ack.rc);

   /* Quiesced AvD has succeeded to go Active */
   if(SA_AMF_HA_ACTIVE == evt->evt.avd_evt->avd_avm_msg.role_ack.role)
   {
      if(NCSCC_RC_SUCCESS == evt->evt.avd_evt->avd_avm_msg.role_ack.rc)
      {
         cb->ha_state = SA_AMF_HA_ACTIVE;

         m_NCS_DBG_PRINTF("\n Q-SCXB->Active avd_ack SUCCESSFUL \n");
         rc = avm_notify_rde_set_role(cb, cb->ha_state);

         rc = avm_mbc_role_chg(cb, cb->ha_state);
         rc = avm_role_chg(cb, SA_AMF_HA_ACTIVE);
         avm_adm_switch_trap(cb, ncsAvmSwitch_ID, AVM_SWITCH_FAILURE);
      }else
      {
         /* Quiesced AvD dint succeed to go Active 
            Something fundamentally wrong */
         m_NCS_DBG_PRINTF("\n System Screwed , Q-SCXB could not go Active incase of failure\n");
         avm_adm_switch_trap(cb, ncsAvmSwitch_ID, AVM_SWITCH_FAILURE);
         rc = NCSCC_RC_FAILURE;
      }
   }else if(SA_AMF_HA_STANDBY == evt->evt.avd_evt->avd_avm_msg.role_ack.role)
   {
      /* Quiesced AvD has failed to go Standby.*/

      if(NCSCC_RC_SUCCESS != evt->evt.avd_evt->avd_avm_msg.role_ack.rc)
      {
         /* Future , Add Failure to RDE */
         m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
         return NCSCC_RC_FAILURE;
      }
      /* Quiesced AvD has succeeded to go Standby.
         Admin Switch over is success full. */
      avm_mds_set_vdest_role(cb, SA_AMF_HA_STANDBY);

      /* Set the HA-State of AvM to Standby */
      cb->ha_state = SA_AMF_HA_STANDBY;

      rc = avm_mbc_role_chg(cb, cb->ha_state);
   }
   return rc;
} 

/*************************************************************************
 * Function: quiesced_avd_hb_lost
 *
 * Purpose:  This function is called when Quisced AvM recives 
 *           hrt beat loss from AvD 
 *
 * Input: cb    - the AvM control block
 *        evt   - Evt received at AvM
 *
 * Returns: NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES:
 *
********************************************************************/
static uns32
quiesced_avd_hb_lost(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
               )
{
   uns32 rc = NCSCC_RC_SUCCESS;
  
   m_AVM_LOG_FUNC_ENTRY("quiesced_avd_hb_lost");
   m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_HB, cb->ha_state, NCSFL_SEV_NOTICE);

   m_NCS_DBG_PRINTF("\n rde_hrt_bt_lost on Quisced \n");
   rc = avm_notify_rde_hrt_bt_lost(cb);

   return rc;
}

/************************************************************
 * Function : role_evt_invalid
 *
 * Purpose  : This function is called when role related
 *            event is recived and  HA-State of  AvM are 
 *            not relevant
 *
 *Inputs:
 *            AVM_CB_T* - AvM CB pointer
 *            AVM_EVT_T - Event received at AvM    
 *Outputs: -
 *
 *Return Value - NCSCC_RC_SUCCESS
 ****************************************************************/
static uns32
role_evt_invalid(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
               )
{
   m_AVM_LOG_FUNC_ENTRY("role_evt_invalid");
   return NCSCC_RC_SUCCESS;
}

/************************************************************
 * Function : role_evt_ignore
*
 * Purpose  : This function is called when role related
 *            event is received at current HA-State of  AvM  
 *            can be ignored
 *
 *Inputs:
 *            AVM_CB_T* - AvM CB pointer
 *            AVM_EVT_T - Event received at AvM    
 *Outputs: -
 *
 *Return Value - NCSCC_RC_SUCCESS
 ****************************************************************/

static uns32
role_evt_ignore(
                  AVM_CB_T   *cb,
                  AVM_EVT_T  *evt
               )
{
   m_AVM_LOG_FUNC_ENTRY("role_evt_ignore");
   return NCSCC_RC_SUCCESS;
}

/************************************************************
 * Function : avm_role_chg
 *
 * Purpose  : This function is called when AvM has to do a role
 *            change.     
 *
 *Inputs:
 *            AVM_CB_T* - AvM CB pointer
 *            AVM_EVT_T - Event received at AvM    
 *Outputs: -
 *
 *Return Value - NCSCC_RC_SUCCESS
 ****************************************************************/
static uns32 
avm_role_chg(
              AVM_CB_T       *cb,
              SaAmfHAStateT   role
            )
{
   uns32 rc =  NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_role_chg");

   if(SA_AMF_HA_QUIESCED != role)
   {
      /* Setting V-DEST rol to MDS */
      if(NCSCC_RC_SUCCESS != (rc = avm_mds_set_vdest_role(cb, role)))
      {
         return NCSCC_RC_FAILURE;
      }
   }

   /* When AvM is going quiesced or standby, event queue anyway would be emoty.
      Just to play safe, empty it. Its like assiging 0 to 0ed 
      variable */
   if(SA_AMF_HA_QUIESCED == role) 
   {
      avm_empty_evt_q(cb);
   }

   /* When AvM is going Active, Process the events at Event Queue.
      If there are any unporcessed events, process them */
   if(SA_AMF_HA_ACTIVE == role)
   {

      /* Check if the upgrade was under progress */
      avm_role_change_check_pld_upgd_prg(cb);

      rc = avm_proc_evt_q(cb);

#if 0
      AVM_ENT_INFO_T      *ent_info;
      SaHpiEntityPathT   entity_path;

      /* for each node, if upgrade is in progress, start the SSU timer */
      m_NCS_MEMSET(entity_path.Entry, 0, sizeof(SaHpiEntityPathT));  
      for(ent_info = avm_find_next_ent_info(cb, &entity_path); 
          ent_info != AVM_ENT_INFO_NULL; ent_info = avm_find_next_ent_info(cb, &entity_path))
      {
         m_NCS_MEMCPY(entity_path.Entry, ent_info->entity_path.Entry, sizeof(SaHpiEntityPathT));
         if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
            m_AVM_SSU_UPGR_TMR_START(cb, ent_info);
         avm_role_change_check_pld_upgd_prg(cb, ent_info);
      }
#endif
   }

   return rc;
} 

/************************************************************
 * Function : avm_mbc_role_chg
 *
 * Purpose  : This function is called when AvM has to do a role
 *            change for MBCSv.     
 *
 *Inputs:
 *            AVM_CB_T* - AvM CB pointer
 *            AVM_EVT_T - Event received at AvM    
 *Outputs: -
 *
 *Return Value - NCSCC_RC_SUCCESS
 ****************************************************************/
static uns32
avm_mbc_role_chg(
                 AVM_CB_T       *cb,
                 SaAmfHAStateT   role
                )
{
   uns32 rc =  NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_mbc_role_chg");

   /* Set MBCSv role */
   if(NCSCC_RC_SUCCESS != avm_set_ckpt_role(cb, role))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(role);
      return NCSCC_RC_FAILURE;
   }

   /* Dipatch any requests waiting in MBCSA to be dispatched to AvM */
   if(NCSCC_RC_SUCCESS != avm_mbc_dispatch(cb, SA_DISPATCH_ALL))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(SA_DISPATCH_ALL);
   }

   return rc; 
} 
