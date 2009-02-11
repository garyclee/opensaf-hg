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
