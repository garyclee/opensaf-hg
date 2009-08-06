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
 
  DESCRIPTION:This is the include file for AvM->AVD interface.
  ..............................................................................

  Function Included in this Module:
 
******************************************************************************
*/

#ifndef __AVM_AVD_API_H__
#define __AVM_AVD_API_H__

/*AVM APIS*/
extern uns32 avm_avd_node_failover_req(AVM_CB_T *, AVM_LIST_HEAD_T *, AVM_NODE_OPERSTATE_T
    );
extern uns32 avm_avd_node_shutdown_req(AVM_CB_T *, AVM_LIST_HEAD_T *);
extern uns32 avm_avd_node_operstate(AVM_CB_T *, AVM_LIST_HEAD_T *, AVM_NODE_OPERSTATE_T
    );
extern uns32 avm_avd_fault_domain_resp(AVM_CB_T *, AVM_LIST_HEAD_T *, SaNameT
    );
extern uns32 avm_avd_node_reset_resp(AVM_CB_T *, AVM_NODE_RESET_RESP_T, SaNameT
    );

extern uns32 avm_avd_role(AVM_CB_T *avm_cb, SaAmfHAStateT role, AVM_ROLE_CHG_CAUSE_T cause);

#endif   /* __AVM_AVD_API_H__ */
