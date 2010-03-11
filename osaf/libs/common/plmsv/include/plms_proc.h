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


/* This file contains the definitions and prototypes for amf related stuff of PLMS */


#define PLMS_COMP_FILE_NAME_LEN 256

EXTERN_C uns32 plms_amf_init();
EXTERN_C uns32 plms_amf_register();
EXTERN_C uns32 plms_quiescing_state_handler(SaInvocationT invocation);
EXTERN_C uns32 plms_quiesced_state_handler(SaInvocationT invocation);
EXTERN_C uns32 plms_invalid_state_handler(SaInvocationT invocation);
EXTERN_C void plms_amf_health_chk_callback(SaInvocationT invocation, const SaNam					eT *compName, SaAmfHealthcheckKeyT 
					*checkType);
EXTERN_C void plms_amf_CSI_set_callback(SaInvocationT invocation, const SaNameT                                         *compName, SaAmfHAStateT new_haState, 
                                         SaAmfCSIDescriptorT csiDescriptor);
EXTERN_C void plms_amf_comp_terminate_callback(SaInvocationT invocation, const SaNameT *ccompName);
void
EXTERN_C plms_amf_csi_rmv_callback(SaInvocationT invocation, const SaNameT *compName, const SaNameT *csiName, const SaAmfCSIFlagsT csiFlags);
EXTERN_C uns32 plms_healthcheck_start();
EXTERN_C uns32 plms_edp_plms_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                        NCSCONTEXT ptr, uns32 *ptr_data_len, 
					EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
					EDU_ERR *o_err);
