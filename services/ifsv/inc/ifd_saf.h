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
FILE NAME: IFD_SAF.H
DESCRIPTION: Function prototypes used for IfD used in AMF.
******************************************************************************/
#ifndef IFD_SAF_H
#define IFD_SAF_H

#if 0
/* Removed for B Spec Compliance */
void
ifd_saf_readiness_state_callback (SaInvocationT invocation,
                                   const SaNameT *compName,
                                   SaAmfReadinessStateT readinessState);
void
ifd_saf_pend_oper_confirm_callback (SaInvocationT invocation,
                                    const SaNameT *compName,
                                    SaAmfPendingOperationFlagsT pendOperFlags);
void
ifd_saf_CSI_set_callback (SaInvocationT invocation,
                          const SaNameT  *compName,
                          const SaNameT  *csiName,
                          SaAmfCSIFlagsT csiFlags,
                          SaAmfHAStateT  *haState,
                          SaNameT        *activeCompName,
                          SaAmfCSITransitionDescriptorT transitionDesc);
#endif
void ifd_saf_CSI_set_callback(SaInvocationT invocation,
                         const SaNameT *compName,
                         SaAmfHAStateT haState,
                         SaAmfCSIDescriptorT csiDescriptor);

void
ifd_saf_health_chk_callback (SaInvocationT invocation,
                             const SaNameT *compName,
                             SaAmfHealthcheckKeyT *checkType);
void ifd_saf_CSI_rem_callback (SaInvocationT invocation,
                               const SaNameT *compName,
                               const SaNameT *csiName,
                               const SaAmfCSIFlagsT csiFlags);
void ifd_saf_comp_terminate_callback (SaInvocationT invocation,
                                      const SaNameT *compName);
uns32
ifd_amf_init (IFSV_CB *ifsv_cb);

#endif
