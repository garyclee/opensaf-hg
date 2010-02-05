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

  This is the main include file for HCD.

******************************************************************************
*/

#ifndef HCD_H
#define HCD_H

/* Get general definitions */
#include "ncsgl_defs.h"

/* Get target's suite of header files...*/

/* From /base/common/inc */
#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncs_ubaid.h"
#include "ncsencdec_pub.h"
#include "ncs_stack.h"
#include "ncs_log.h"
#include "ncs_lib.h"
#include "dta_papi.h"
#include "ncs_util.h"

/* From targsvcs/common/inc */
#include "mds_papi.h"

#include "ncs_mda_pvt.h"
#include "mds_papi.h"

/* HCD specific inc Files */
#include <SaHpi.h>

#include "hcd_util.h"
#include "hisv_msg.h"
#include "ham_cb.h"
#include "hsm_cb.h"
#include "sim_cb.h"
#include "ham_mds.h"
#include "hcd_mem.h"
#include "hcd_log.h"
#include "hpl_msg.h"

/* priority and stack size of HCD threads */
#define HSM_TASK_PRIORITY     5
#define HSM_STACKSIZE         NCS_STACKSIZE_HUGE
#define SIM_TASK_PRIORITY     5
#define SIM_STACKSIZE         NCS_STACKSIZE_HUGE
#define HAM_TASK_PRIORITY     5
#define HAM_STACKSIZE         NCS_STACKSIZE_HUGE
#define HPI_INIT_MAX_RETRY    3

/* time required to stop auto HPI hotswap */
#define HPI_AUTO_HS_STOP_TMR 0xFFFFFFFF

/*** Macro used to get the AMF version used ****/
#define m_HISV_GET_AMF_VER(amf_ver) amf_ver.releaseCode='B'; amf_ver.majorVersion=0x01; amf_ver.minorVersion=0x01;

/*****************************************************************************
 * Data Structure Used to hold HCD control block
 *****************************************************************************/
typedef struct hcd_cb_tag {
	SYSF_MBX mbx;		/* HCDs mailbox                              */
	SaNameT comp_name;	/* Component name - "HCD"                    */
	uns8 hm_poolid;		/* For use with handle manager               */
	NCSCONTEXT task_hdl;
	uns32 my_hdl;		/* Handle manager handle                     */
	SaAmfHandleT amf_hdl;	/* AMF handle, obtained thru AMF init        */
	SaAmfHAStateT ha_state;	/* present AMF HA state of the component     */
	V_DEST_QA my_anc;	/* required for MDS VDEST */
	V_DEST_RL mds_role;	/* mds role   */
	void *p_s_handle;	/* semaphore to health check the HSM         */
	HPI_SESSION_ARGS *args;	/* HPI session arguments */
} HCD_CB;

#define m_HISV_HCD_RETRIEVE_HCD_CB  ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hcd_hdl)
#define m_HISV_HCD_GIVEUP_HCD_CB    ncshm_give_hdl(gl_hcd_hdl)

/* HAM CB global handle declaration */
EXTERN_C uns32 gl_hcd_hdl;
EXTERN_C uns32 gl_ham_hdl;
EXTERN_C uns32 gl_hsm_hdl;
EXTERN_C uns32 gl_sim_hdl;

/* External function declarations */
EXTERN_C uns32 hisv_main(int argc, char **argv);
EXTERN_C uns32 ncs_hisv_hcd_lib_req(NCS_LIB_REQ_INFO *req_info);
EXTERN_C uns32 ham_initialize(HPI_SESSION_ARGS *args);
EXTERN_C uns32 ham_finalize(void);
EXTERN_C uns32 hsm_initialize(HPI_SESSION_ARGS *args);
EXTERN_C uns32 hsm_finalize(void);
EXTERN_C uns32 sim_initialize(void);
EXTERN_C uns32 sim_finalize(void);
EXTERN_C uns32 hsm_eda_chan_initialize(HSM_CB *hsm_cb);
EXTERN_C uns32 hsm_eda_chan_finalize(HSM_CB *hsm_cb);
EXTERN_C uns32 hcd_hsm(void);
EXTERN_C uns32 hcd_sim(void);
EXTERN_C uns32 hcd_ham(void);
EXTERN_C uns32 discover_domain(HPI_SESSION_ARGS *ptr);
EXTERN_C uns32 hcd_amf_init(HCD_CB *hcd_cb);
EXTERN_C NCS_BOOL hcd_clear_mbx(NCSCONTEXT arg, NCSCONTEXT msg);
EXTERN_C uns32 hcd_cb_init(HCD_CB *hcd_cb);
EXTERN_C uns32 hcd_cb_destroy(HCD_CB *hcd_cb);

#endif   /* !HCD_H */
