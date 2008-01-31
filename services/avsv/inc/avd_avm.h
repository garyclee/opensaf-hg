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

  This module is the include file for Availability Directors AVM message
  processing.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_AVM_H
#define AVD_AVM_H

EXTERN_C uns32 avd_avm_send_shutdown_resp(AVD_CL_CB *, SaNameT *, uns32);
EXTERN_C uns32 avd_avm_send_failover_resp(AVD_CL_CB *, SaNameT *, uns32);
EXTERN_C uns32 avd_avm_rcv_msg(uns32 , AVM_AVD_MSG_T *);
EXTERN_C uns32 avd_avm_send_fault_domain_req(AVD_CL_CB *, SaNameT *);
EXTERN_C uns32 avd_avm_send_reset_req(AVD_CL_CB *, SaNameT *);


EXTERN_C void avd_avm_mark_nd_absent(AVD_CL_CB *, AVD_AVND *);
EXTERN_C void avd_avm_nd_shutdown_func(AVD_CL_CB *, AVD_EVT *);
EXTERN_C void avd_handle_nd_failover_shutdown(AVD_CL_CB *, AVD_AVND *, SaBoolT );
EXTERN_C void avd_chk_failover_shutdown_cxt(AVD_CL_CB *, AVD_AVND *, SaBoolT);
EXTERN_C void avd_avm_nd_failover_func(AVD_CL_CB *, AVD_EVT *);
EXTERN_C void avd_avm_fault_domain_rsp(AVD_CL_CB *, AVD_EVT *);
EXTERN_C void avd_avm_nd_reset_rsp_func(AVD_CL_CB *, AVD_EVT *);
EXTERN_C void avd_avm_nd_oper_st_func(AVD_CL_CB *, AVD_EVT *);
EXTERN_C uns32 avd_avm_role_rsp(AVD_CL_CB *cb, NCS_BOOL status, SaAmfHAStateT role);
EXTERN_C uns32 avd_avm_d_hb_lost_msg(AVD_CL_CB *cb, uns32 node);
EXTERN_C uns32 avd_avm_nd_hb_lost_msg(AVD_CL_CB *cb, uns32 node);
EXTERN_C uns32 avd_avm_node_reset_rsp(AVD_CL_CB *cb, uns32 node);

#endif
