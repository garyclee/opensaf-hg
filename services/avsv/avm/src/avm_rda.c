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

  DESCRIPTION: This module contains interface with RDE.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avm_rda_initialize
  avm_rda_cb
  avm_notify_rde_set_role
  avm_notify_rde_hrt_bt_lost
    
******************************************************************************
*/
#include "avm.h"

#define PRINT_ROLE(role)\
{\
    (role == 0)?m_NCS_DBG_PRINTF("\n %s %s \n", "RDA giving role to avm : ", "PCS_RDA_ACTIVE"):\
    (role == 1)?m_NCS_DBG_PRINTF("\n %s %s \n", "RDA giving role to avm : ", "PCS_RDA_STANDBY"):\
                m_NCS_DBG_PRINTF("\n %s %d \n", "RDA giving role to avm : ", role);\
}

static void
avm_rda_cb(
             uns32               cb_hdl,
             PCS_RDA_ROLE        role,
             PCSRDA_RETURN_CODE  error_code,
 PCS_RDA_CMD cmd
          );
/***********************************************************************
 ******
 * Name          : avm_rda_initialize
 *
 * Description   : This function  initiailizes rda interface 
 *                 and gets init role
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : During initialization Quiesced is ignored and hence
 *                 ignored even in the switch statement.
*********************************************************************
 *****/
extern uns32
avm_rda_initialize(AVM_CB_T *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ pcs_rda_req;

   m_AVM_LOG_FUNC_ENTRY("avm_rda_initialize");

   pcs_rda_req.req_type = PCS_RDA_LIB_INIT;
   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      return NCSCC_RC_FAILURE;
   }

   pcs_rda_req.callback_handle = cb->cb_hdl;
   pcs_rda_req.req_type        = PCS_RDA_REGISTER_CALLBACK;
   pcs_rda_req.info.call_back  = avm_rda_cb;
   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      return NCSCC_RC_FAILURE;
   }


   pcs_rda_req.req_type = PCS_RDA_GET_ROLE;
   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_GET_ROLE, pcs_rda_req.info.io_role, NCSFL_SEV_INFO);

   switch(pcs_rda_req.info.io_role)
   {
      case PCS_RDA_ACTIVE:
      {
         cb->ha_state = SA_AMF_HA_ACTIVE;
      }
      break;

      case PCS_RDA_STANDBY:
      {
         cb->ha_state = SA_AMF_HA_STANDBY;
      }
      break;

      default:
      {
         m_AVM_LOG_INVALID_VAL_ERROR(pcs_rda_req.info.io_role);
         rc = NCSCC_RC_FAILURE;
         break;
      }
   }
   
   return rc;
}

/******************************************************************
 * Name          : avm_register_rda_cb
 *
 * Description   : This function registers rda callback.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************************
 *****/
extern uns32
avm_register_rda_cb(AVM_CB_T *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ pcs_rda_req;

   pcs_rda_req.callback_handle = cb->cb_hdl;
   pcs_rda_req.req_type        = PCS_RDA_REGISTER_CALLBACK;
   pcs_rda_req.info.call_back  = avm_rda_cb;

   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_ROLE(AVM_LOG_RDA_AVM_SET_ROLE, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_NOTICE);

   return rc;

}

/******************************************************************
 * Name          : avm_unregister_rda_cb
 *
 * Description   : This function unregister rda callback.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************************
 *****/
extern uns32
avm_unregister_rda_cb(AVM_CB_T *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ pcs_rda_req;

   pcs_rda_req.req_type        = PCS_RDA_UNREGISTER_CALLBACK;

   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_ROLE(AVM_LOG_RDA_AVM_SET_ROLE, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_NOTICE);

   return rc;

}

/******************************************************************
 * Name          : avm_notify_rde_hrt_bt_lost
 *
 * Description   : This function informs hrt beat lost info to RDE
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************************
 *****/
extern uns32
avm_notify_rde_hrt_bt_lost(AVM_CB_T *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ pcs_rda_req;
   
   pcs_rda_req.req_type = PCS_RDA_AVD_HB_ERR;
   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      m_AVM_LOG_ROLE(AVM_LOG_RDA_HB, AVM_LOG_RDA_FAILURE, NCSFL_SEV_ERROR); 
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_ROLE(AVM_LOG_RDA_HB, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_INFO); 

   return rc;
}

/******************************************************************
 * Name          : avm_notify_rde_nd_hrt_bt_lost
 *
 * Description   : This function informs hrt beat lost info to RDE
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************** *****/
extern uns32
avm_notify_rde_nd_hrt_bt_lost(AVM_CB_T *cb, uns32 phy_slot_id)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ pcs_rda_req;
   
   pcs_rda_req.req_type = PCS_RDA_AVND_HB_ERR;
   pcs_rda_req.info.phy_slot_id = phy_slot_id;

   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      m_AVM_LOG_ROLE(AVM_LOG_RDA_HB, AVM_LOG_RDA_FAILURE, NCSFL_SEV_ERROR); 
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_ROLE(AVM_LOG_RDA_HB, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_INFO); 

   return rc;
}

/******************************************************************
 * Name          : avm_notify_rde_set_role
 *
 * Description   : This function informs role to RDE
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************************
 *****/
extern uns32
avm_notify_rde_set_role(AVM_CB_T *cb,
                        SaAmfHAStateT role)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ pcs_rda_req;

   pcs_rda_req.req_type = PCS_RDA_SET_ROLE;

   if(role == SA_AMF_HA_ACTIVE)
      pcs_rda_req.info.io_role = PCS_RDA_ACTIVE;
   else if (role == SA_AMF_HA_STANDBY)
      pcs_rda_req.info.io_role = PCS_RDA_STANDBY;
   else if(role == SA_AMF_HA_QUIESCED)
      pcs_rda_req.info.io_role = PCS_RDA_QUIESCED;
   else
      pcs_rda_req.info.io_role = role;

   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_ROLE(AVM_LOG_RDA_AVM_SET_ROLE, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_NOTICE);

   return rc;

}

/******************************************************************
 * Name          : avm_rda_cb
 *
 * Description   : This function is used by RDE to give a role to 
 *                 AvM
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************************
 *****/
static void 
avm_rda_cb(
             uns32               cb_hdl,
             PCS_RDA_ROLE        role,
             PCSRDA_RETURN_CODE  error_code,
 PCS_RDA_CMD cmd
          )
{
   uns32 rc =  NCSCC_RC_SUCCESS;
   AVM_CB_T  *cb;
   AVM_EVT_T *avm_evt;
   AVM_ENT_INFO_T *ent_info = NULL;
   SaHpiEntityPathT entity_path;
   
/* m_NCS_DBG_PRINTF("\n RDA giving role to avm : %d \n", role); */
   PRINT_ROLE(role);

   if (AVM_CB_NULL == (cb = ncshm_take_hdl(NCS_SERVICE_ID_AVM, cb_hdl)))
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return ;
   }

   if(cmd.req_type == PCS_RDA_NODE_RESET_CMD)
   {
     m_NCS_CONS_PRINTF("\n PCS_RDA_NODE_RESET_CMD recvd, slot %d, shelf %d\n",cmd.info.node_reset_info.slot_id,cmd.info.node_reset_info.shelf_id);
     m_NCS_MEMSET(&entity_path,0,sizeof(entity_path));
#ifdef HPI_A
     entity_path.Entry[0].EntityType = SAHPI_ENT_SYSTEM_BOARD;
     entity_path.Entry[0].EntityInstance = cmd.info.node_reset_info.slot_id;

     entity_path.Entry[1].EntityType = SAHPI_ENT_SYSTEM_CHASSIS;
     entity_path.Entry[1].EntityInstance = cmd.info.node_reset_info.shelf_id;

     entity_path.Entry[2].EntityType = SAHPI_ENT_ROOT;
     entity_path.Entry[2].EntityInstance = 0;
#else
     entity_path.Entry[0].EntityType = SAHPI_ENT_PHYSICAL_SLOT;
     entity_path.Entry[0].EntityLocation = cmd.info.node_reset_info.slot_id;

     entity_path.Entry[1].EntityType = SAHPI_ENT_SYSTEM_CHASSIS;
     entity_path.Entry[1].EntityLocation = cmd.info.node_reset_info.shelf_id;

     entity_path.Entry[2].EntityType = SAHPI_ENT_ROOT;
     entity_path.Entry[2].EntityLocation = 0;
#endif

     if(AVM_ENT_INFO_NULL == (ent_info = avm_find_ent_info(cb, &entity_path)))
     {
       m_NCS_CONS_PRINTF("\n Error in avm_rda_cb : ent_info is NULL\n");
       ncshm_give_hdl(g_avm_hdl);
       return ;
     }
     avm_avd_node_reset_resp(cb, AVM_NODE_RESET_SUCCESS, ent_info->node_name);
     ncshm_give_hdl(g_avm_hdl);
     return ;
   }


   m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_SET_ROLE, role, NCSFL_SEV_NOTICE);
   m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_AVM_ROLE, cb->ha_state, NCSFL_SEV_NOTICE);

   avm_evt = (AVM_EVT_T*)m_MMGR_ALLOC_AVM_EVT;

   avm_evt->src                  = AVM_EVT_RDE;
   avm_evt->fsm_evt_type         = AVM_ROLE_EVT_RDE_SET;
   avm_evt->evt.rde_evt.role     = role;   
   avm_evt->evt.rde_evt.ret_code = error_code;   

   if(m_NCS_IPC_SEND(&cb->mailbox, avm_evt, NCS_IPC_PRIORITY_HIGH)
                  == NCSCC_RC_FAILURE)
   {
      m_AVM_LOG_MBX(AVM_LOG_MBX_SEND, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
      m_MMGR_FREE_AVM_EVT(avm_evt);
      rc = NCSCC_RC_FAILURE;
   }

   ncshm_give_hdl(g_avm_hdl);

   return ;
}

/******************************************************************
 * Name          : avm_notify_rde_hrt_bt_restore
 *
 * Description   : This function informs AVD hrt beat restore info to RDE
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************************
 *****/
extern uns32
avm_notify_rde_hrt_bt_restore(AVM_CB_T *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ pcs_rda_req;
   
   pcs_rda_req.req_type = PCS_RDA_AVD_HB_RESTORE;
   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      m_AVM_LOG_ROLE(AVM_LOG_RDA_HB, AVM_LOG_RDA_FAILURE, NCSFL_SEV_ERROR); 
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_ROLE(AVM_LOG_RDA_HB, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_INFO); 
   return rc;
}

/******************************************************************
 * Name          : avm_notify_rde_nd_hrt_bt_restore
 *
 * Description   : This function informs AvND hrt beat restore info to RDE
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************** *****/
extern uns32
avm_notify_rde_nd_hrt_bt_restore(AVM_CB_T *cb, uns32 phy_slot_id)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ pcs_rda_req;
   
   pcs_rda_req.req_type = PCS_RDA_AVND_HB_RESTORE;
   pcs_rda_req.info.phy_slot_id = phy_slot_id;

   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      m_AVM_LOG_ROLE(AVM_LOG_RDA_HB, AVM_LOG_RDA_FAILURE, NCSFL_SEV_ERROR); 
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_ROLE(AVM_LOG_RDA_HB, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_INFO); 
   return rc;
}
