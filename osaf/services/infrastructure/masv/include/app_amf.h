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

  
  .....................................................................  
  
  DESCRIPTION: This file describes AMF Interface definitions for MAS/PSR in 
               common. 
*****************************************************************************/
#ifndef APP_AMF_H
#define APP_AMF_H

#include "gl_defs.h"
#include "t_suite.h"
#include "saAmf.h"
#include "mds_papi.h"
#include "mab_penv.h"
#include "ncs_mib_pub.h"
#include "mab_msg.h"

/* Timeout to retry AMF interface initialisation. */
#define m_NCS_AMF_INIT_TIMEOUT_IN_SEC    3

/* APP HA States */
typedef enum ncs_app_ha_state {
	NCS_APP_AMF_HA_STATE_NULL = 0,
	NCS_APP_AMF_HA_STATE_ACTIVE = SA_AMF_HA_ACTIVE,
	NCS_APP_AMF_HA_STATE_STANDBY = SA_AMF_HA_STANDBY,
	NCS_APP_AMF_HA_STATE_QUISCED = SA_AMF_HA_QUIESCED,
	NCS_APP_AMF_HA_STATE_QUISCING = SA_AMF_HA_QUIESCING,
	NCS_APP_AMF_HA_STATE_MAX
} NCS_APP_AMF_HA_STATE;

/* application HA state machine handler */
typedef uns32
 (*NCS_APP_AMF_HA_STATE_MACHINCE) (void *data, PW_ENV_ID, SaAmfCSIDescriptorT);

typedef struct ncs_app_csi_attribs {
	/* Environment ID */
	PW_ENV_ID env_id;

	/* workload description */
	SaNameT work_desc;

	/* workload attributes */
	SaAmfCSIAttributeListT work_params;

	/* CSI state */
	NCS_APP_AMF_HA_STATE ha_state;

	/* PWE Handle */
	MDS_HDL env_hdl;

} NCS_APP_AMF_CSI_ATTRIBS;

/* Retry AMF interface initialisation timer information */
typedef struct ncs_app_amf_init_timer {
	/* Timer id */
	tmr_t timer_id;

	/* Time period in seconds */
	uns32 period;

	/* Message type that will be sent on timer expiry */
	MAB_SVC_OP msg_type;

	/* Mailbox to which message is sent on timer expiry */
	SYSF_MBX *mbx;

} NCS_APP_AMF_INIT_TIMER;

/* AMF attributes of an application */
typedef struct ncs_app_amf_attribs {
	/* version info of the SAF implementation */
	SaVersionT safVersion;

	/* AMF Handle */
	SaAmfHandleT amfHandle;

	/* for AMF communication */
	SaSelectionObjectT amfSelectionObject;

	/* component Name */
	SaNameT compName;

	/* AMF Callbacks */
	SaAmfCallbacksT amfCallbacks;

	/* Dispatch  Flags */
	SaDispatchFlagsT dispatchFlags;

	/* Health Check Key */
	SaAmfHealthcheckKeyT healthCheckKey;

	/* Health Check Invocation Type, app driven or AMF driven?? */
	SaAmfHealthcheckInvocationT invocationType;

	/* Health Check Failure Recommended Recovery */
	SaAmfRecommendedRecoveryT recommendedRecovery;

	/* state machine to handle the HA state */
	NCS_APP_AMF_HA_STATE_MACHINCE
	 ncs_app_amf_hastate_dispatch[NCS_APP_AMF_HA_STATE_MAX]
	    [NCS_APP_AMF_HA_STATE_MAX];

	/* Selection object of USR1 signal handler. */
	NCS_SEL_OBJ sighdlr_sel_obj;

	/* Timer information for retrying amf interface initialisation */
	NCS_APP_AMF_INIT_TIMER amf_init_timer;

} NCS_APP_AMF_ATTRIBS;

typedef struct ncs_app_amf_ha_state_handlers {
	/* Function to handle an invalid state transition request */
	NCS_APP_AMF_HA_STATE_MACHINCE invalidTrans;

	/* Function to init the CSI in ACTIVE state */
	NCS_APP_AMF_HA_STATE_MACHINCE initActive;

	/* Function to go to ACTIVE */
	NCS_APP_AMF_HA_STATE_MACHINCE activeTrans;

	/* Function to init the CSI in STANDBY state */
	NCS_APP_AMF_HA_STATE_MACHINCE initStandby;

	/* Function to go to STANDBY state */
	NCS_APP_AMF_HA_STATE_MACHINCE standbyTrans;

	/* Function to go to QUIESCED state */
	NCS_APP_AMF_HA_STATE_MACHINCE quiescedTrans;

	/* Function to go to QUIESCING state */
	NCS_APP_AMF_HA_STATE_MACHINCE quiescingTrans;

	/* Function to go from QUIESCING to QUIESCED */
	NCS_APP_AMF_HA_STATE_MACHINCE quiescingToQuiesced;
} NCS_APP_AMF_HA_STATE_HANDLERS;

/*signal handler */
typedef void
 (*SIG_HANDLR) (int);

EXTERN_C int32 ncs_app_signal_install(int i_sig_num, SIG_HANDLR i_sig_handler);

/* generic invalid state transition handler */
EXTERN_C uns32 ncs_app_amf_invalid_state_process(void *, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to initialize the AMF attributes of an application */
EXTERN_C uns32
ncs_app_amf_attribs_init(NCS_APP_AMF_ATTRIBS *attribs,
			 int8 *health_key,
			 SaDispatchFlagsT dispatch_flags,
			 SaAmfHealthcheckInvocationT hc_inv_type,
			 SaAmfRecommendedRecoveryT recovery,
			 NCS_APP_AMF_HA_STATE_HANDLERS *state_handler, SaAmfCallbacksT *amfCallbacks);

/* routine to intialize the AMF */
EXTERN_C SaAisErrorT ncs_app_amf_initialize(NCS_APP_AMF_ATTRIBS *);

/* Unregistration and Finalization with AMF Library */
EXTERN_C SaAisErrorT ncs_app_amf_finalize(NCS_APP_AMF_ATTRIBS *);

/* Function to start timer to retry the initialisation of AMF interface. */
EXTERN_C uns32 ncs_app_amf_init_start_timer(NCS_APP_AMF_INIT_TIMER *amf_init_timer);

/* RDA API Interface */
/* RDA Initialize, get the role, finalize */
EXTERN_C uns32 app_rda_init_role_get(NCS_APP_AMF_HA_STATE *o_init_role);

#endif
