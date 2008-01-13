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

  This file contains forward declarations of HISv - AMF functions.

*****************************************************************************/

#ifndef HCD_AMF_H
#define HCD_AMF_H

#if 0

#define HCD_HA_INVALID 0 /*Invalid HA state */
#define MAX_HA_STATE 4

typedef uns32 (*hcd_HAStateHandler)(HCD_CB *cb,
                                      SaInvocationT invocation);

/* AMF HA state handler routines */
uns32 hcd_invalid_state_handler(HCD_CB *cb,
                              SaInvocationT invocation);
uns32 hcd_active_state_handler(HCD_CB *cb,
                              SaInvocationT invocation);
uns32 hcd_standby_state_handler(HCD_CB *cb,
                              SaInvocationT invocation);
uns32 hcd_quiescing_state_handler(HCD_CB *cb,
                              SaInvocationT invocation);
uns32 hcd_quiesced_state_handler(HCD_CB *cb,
                              SaInvocationT invocation);
struct next_HAState{
      uns8 nextState1;
      uns8 nextState2;
}nextStateInfo; /* AMF HA state can transit to a maximum of the two defined states */

#define VALIDATE_STATE(curr,next) \
((curr > MAX_HA_STATE)||(next > MAX_HA_STATE)) ? HCD_HA_INVALID : \
(((validStates[curr].nextState1 == next)||(validStates[curr].nextState2 == next))?next: HCD_HA_INVALID)

#endif /* 0 */

EXTERN_C void hcd_amf_CSI_set_callback (SaInvocationT invocation,
                          const SaNameT  *compName, SaAmfHAStateT  haState,
                          SaAmfCSIDescriptorT csiDescriptor);
EXTERN_C void hcd_amf_health_chk_callback (SaInvocationT invocation,
                          const SaNameT *compName,
                          SaAmfHealthcheckKeyT *checkType);
EXTERN_C void hcd_amf_comp_terminate_callback(SaInvocationT invocation,
                          const SaNameT *compName);
EXTERN_C void hcd_amf_csi_rmv_callback(SaInvocationT invocation,
                          const SaNameT *compName,
                          const SaNameT *csiName,
                          SaAmfCSIFlagsT *csiFlags);
EXTERN_C uns32 hcd_amf_init (HCD_CB *hcd_cb);
EXTERN_C uns32 hisv_hcd_health_check(SYSF_MBX *mbx);

#endif   /* HCD_AMF_H */
