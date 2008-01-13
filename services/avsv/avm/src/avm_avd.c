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
 
  DESCRIPTION:This module deals with AVM-AVD interface.
  ..............................................................................

  Function Included in this Module:

  avm_avd_node_failover_req  - 
  avm_avd_node_shtdown_req   - 
  avm_avd_node_operstate     -
  avm_avd_faultdomain_resp   - 
  avm_avd_node_reset_resp    -
 
******************************************************************************
*/


#include "avm.h"

/***********************************************************************
 ******
 * Name          : avm_avd_node_failover_req
 *
 *
 * Description   : This function ise used to send failover of nodes to AvD 
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_LIST_HEAD_T- Pointer to the head of a list
 *                 AVM_NODE_OPERSTATE_T operstate of the nodes
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/

extern uns32 
avm_avd_node_failover_req(
                           AVM_CB_T             *avm_cb,
                           AVM_LIST_HEAD_T      *head, 
                           AVM_NODE_OPERSTATE_T oper_state
                          )
{
   AVM_AVD_MSG_T *avm_avd; 
   uns32          rc;

   avm_avd           = m_MMGR_ALLOC_AVM_AVD_MSG;
   avm_avd->msg_type = AVM_AVD_NODE_FAILOVER_REQ_MSG;
   avm_avd->avm_avd_msg.failover_req.head       = *head;
   avm_avd->avm_avd_msg.failover_req.oper_state = oper_state;

   if(AVM_LIST_NODE_NULL == head->node)
   {
      m_AVM_LOG_DEBUG("avm_avd_node_failover_req:No nodes in the list", NCSFL_SEV_INFO);
      return NCSCC_RC_SUCCESS;
   }

   rc = avm_mds_msg_send(avm_cb, avm_avd, &avm_cb->adest, MDS_SEND_PRIORITY_HIGH);    
   if(NCSCC_RC_SUCCESS != rc)
   {
      avm_avd_free_msg(&avm_avd);
   }
     
   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_avd_node_shutdown_req
 *
 *
 * Description   : This function ise used to send shutdown of nodes to AvD 
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_LIST_HEAD_T- Pointer to the head of a list
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
extern uns32 
avm_avd_node_shutdown_req(
                           AVM_CB_T          *avm_cb,
                           AVM_LIST_HEAD_T   *head   
                         )
{

   AVM_AVD_MSG_T *avm_avd; 
   uns32          rc;

   avm_avd           = m_MMGR_ALLOC_AVM_AVD_MSG;
   avm_avd->msg_type = AVM_AVD_NODE_SHUTDOWN_REQ_MSG;
   avm_avd->avm_avd_msg.shutdown_req.head = *head;

   if(AVM_LIST_NODE_NULL == head->node)
   {
      m_AVM_LOG_DEBUG("avm_avd_node_shutdown_req:No nodes in the list", NCSFL_SEV_INFO);
      return NCSCC_RC_SUCCESS;
   }
   rc = avm_mds_msg_send(avm_cb, avm_avd, &avm_cb->adest, MDS_SEND_PRIORITY_HIGH);    
   if(NCSCC_RC_SUCCESS != rc)
   {
      avm_avd_free_msg(&avm_avd);
   }

   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_avd_node_operstate
 *
 *
 * Description   : This function ise used to send shutdown of nodes to AvD 
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_LIST_HEAD_T- Pointer to the head of a list
 *                AVM_NODE_OPERSTATE_T - operstate of the nodes   
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
extern uns32 
avm_avd_node_operstate(
                        AVM_CB_T             *avm_cb,
                        AVM_LIST_HEAD_T      *head,
                        AVM_NODE_OPERSTATE_T operstate
                      )
{
   AVM_AVD_MSG_T *avm_avd;
   uns32          rc;
 
   avm_avd           = m_MMGR_ALLOC_AVM_AVD_MSG;
   avm_avd->msg_type = AVM_AVD_NODE_OPERSTATE_MSG;
   avm_avd->avm_avd_msg.operstate.head       = *head;
   avm_avd->avm_avd_msg.operstate.oper_state = operstate;

   if(AVM_LIST_NODE_NULL == head->node)
   {
      m_AVM_LOG_DEBUG("avm_avd_node_operstate:No nodes in the list", NCSFL_SEV_INFO);
      return NCSCC_RC_SUCCESS;
   }

   rc = avm_mds_msg_send(avm_cb, avm_avd, &avm_cb->adest, MDS_SEND_PRIORITY_HIGH);    
   if(NCSCC_RC_SUCCESS != rc)
   {
      avm_avd_free_msg(&avm_avd);
   }

   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_avd_fault_domain_resp
 *
 *
 * Description   : This function ise used to send shutdown of nodes to AvD 
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_LIST_HEAD_T- Pointer to the head of a list
 *                 SaNameT        - node name   
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
extern uns32 
avm_avd_fault_domain_resp(
                           AVM_CB_T          *avm_cb,
                           AVM_LIST_HEAD_T   *head,
                           SaNameT           node_name 
                         )
{
   AVM_AVD_MSG_T  *avm_avd;   
   uns32           rc;  
  
   avm_avd           = m_MMGR_ALLOC_AVM_AVD_MSG; 
   avm_avd->msg_type = AVM_AVD_FAULT_DOMAIN_RESP_MSG;
   avm_avd->avm_avd_msg.faultdomain_resp.head = *head;
   m_NCS_MEMCPY(&avm_avd->avm_avd_msg.faultdomain_resp.node_name, &node_name, sizeof(SaNameT));

   if(AVM_LIST_NODE_NULL == head->node)
   {
      m_AVM_LOG_DEBUG("avm_avd_fault_domain_resp: No nodes in the list", NCSFL_SEV_INFO);
      return NCSCC_RC_SUCCESS;
   }

   rc = avm_mds_msg_send(avm_cb, avm_avd, &avm_cb->adest, MDS_SEND_PRIORITY_HIGH);    
   
   if(NCSCC_RC_SUCCESS != rc)
   {
      avm_avd_free_msg(&avm_avd);
   }

   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_avd_node_reset_resp
 *
 *
 * Description   : This function ise used to send shutdown of nodes to AvD 
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_NODE_RESET_RESP_T -  Reset Response
 *                 SaNameT        - node name   
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
extern uns32 
avm_avd_node_reset_resp( 
                        AVM_CB_T               *avm_cb,
                        AVM_NODE_RESET_RESP_T   reset_resp,
                        SaNameT                 node_name
                       )
{

   AVM_AVD_MSG_T  *avm_avd;   
   uns32           rc;
    
   avm_avd           = m_MMGR_ALLOC_AVM_AVD_MSG;
   avm_avd->msg_type = AVM_AVD_NODE_RESET_RESP_MSG;
   avm_avd->avm_avd_msg.reset_resp.resp = reset_resp;
   m_NCS_MEMCPY(&avm_avd->avm_avd_msg.reset_resp.node_name, &node_name, sizeof(SaNameT));

   rc = avm_mds_msg_send(avm_cb, avm_avd, &avm_cb->adest, MDS_SEND_PRIORITY_HIGH);    
     
   if(NCSCC_RC_SUCCESS != rc)
   {
      avm_avd_free_msg(&avm_avd);
   }

   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_avd_role
 *
 * Description   : This function ise used to send role to AvD
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_NODE_RESET_RESP_T -  Reset Response
 *                 SaNameT        - node name
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/
extern uns32 
avm_avd_role( 
              AVM_CB_T               *avm_cb,
              SaAmfHAStateT           role,
              AVM_ROLE_CHG_CAUSE_T    cause
            )
{

   AVM_AVD_MSG_T  *avm_avd;   
   uns32           rc;

   m_AVM_LOG_FUNC_ENTRY("avm_avd_role");     

   avm_avd           = m_MMGR_ALLOC_AVM_AVD_MSG;
   avm_avd->msg_type = AVM_AVD_SYS_CON_ROLE_MSG;
   avm_avd->avm_avd_msg.role.role  = role;
   avm_avd->avm_avd_msg.role.cause = cause;

   rc = avm_mds_msg_send(avm_cb, avm_avd, &avm_cb->adest, MDS_SEND_PRIORITY_HIGH);    
     
   if(NCSCC_RC_SUCCESS != rc)
   {
      avm_avd_free_msg(&avm_avd);
   }

   return NCSCC_RC_SUCCESS;
}
