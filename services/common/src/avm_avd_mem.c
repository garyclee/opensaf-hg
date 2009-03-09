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
 
  DESCRIPTION:This is the file for AvM->AVD Memory Deallocations
  ..............................................................................

  Function Included in this Module:
 
******************************************************************************
*/


#include "avm_avd.h"

static uns32
avm_avd_free_list(AVM_LIST_HEAD_T head);

static uns32
avm_avd_free_list(AVM_LIST_HEAD_T head)
{
   AVM_LIST_NODE_T *node_ptr;
   AVM_LIST_NODE_T *temp;
   
   node_ptr = head.node;

   for(node_ptr = head.node; AVM_LIST_NODE_NULL != node_ptr;)
   {
      temp = node_ptr->next;
      m_MMGR_FREE_AVM_AVD_LIST_NODE(node_ptr);
      node_ptr = temp;
   }
   return NCSCC_RC_SUCCESS;
}

extern uns32
avm_avd_free_msg(AVM_AVD_MSG_T **avm_avd)
{
   switch((*avm_avd)->msg_type)
   {
      case AVM_AVD_NODE_FAILOVER_REQ_MSG:
      {
         avm_avd_free_list((*avm_avd)->avm_avd_msg.failover_req.head);
      }
      break;

      case AVM_AVD_NODE_SHUTDOWN_REQ_MSG:
      {
         avm_avd_free_list((*avm_avd)->avm_avd_msg.shutdown_req.head);
      }
      break;
   
      case AVM_AVD_NODE_OPERSTATE_MSG:
      {
         avm_avd_free_list((*avm_avd)->avm_avd_msg.operstate.head);
      }
      break;
      
      case AVM_AVD_FAULT_DOMAIN_RESP_MSG:
      {
         avm_avd_free_list((*avm_avd)->avm_avd_msg.faultdomain_resp.head);
      }
      break;

      default:
      break; 
   }

   m_MMGR_FREE_AVM_AVD_MSG(*avm_avd);
   *avm_avd = AVM_AVD_MSG_NULL;

   return NCSCC_RC_SUCCESS;
}   
