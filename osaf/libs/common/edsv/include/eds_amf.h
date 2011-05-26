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

#include <configmake.h>

/*****************************************************************************
..............................................................................
 
..............................................................................

  DESCRIPTION:

  This file contains important header file includes to be used by rest of EDS.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

*******************************************************************************/
#ifndef _EDS_AMF_H
#define _EDS_AMF_H
#include "eds.h"

#define EDS_HA_INVALID 0	/*Invalid HA state */
#define MAX_HA_STATE 4

typedef uns32 (*eds_HAStateHandler) (EDS_CB *cb, SaInvocationT invocation);

/* AMF HA state handler routines */
uns32 eds_invalid_state_handler(EDS_CB *cb, SaInvocationT invocation);
uns32 eds_active_state_handler(EDS_CB *cb, SaInvocationT invocation);
uns32 eds_standby_state_handler(EDS_CB *cb, SaInvocationT invocation);
uns32 eds_quiescing_state_handler(EDS_CB *cb, SaInvocationT invocation);
uns32 eds_quiesced_state_handler(EDS_CB *cb, SaInvocationT invocation);
struct next_HAState {
	uint8_t nextState1;
	uint8_t nextState2;
} nextStateInfo;		/* AMF HA state can transit to a maximum of the two defined states */

#define VALIDATE_STATE(curr,next) \
((curr > MAX_HA_STATE)||(next > MAX_HA_STATE)) ? EDS_HA_INVALID : \
(((validStates[curr].nextState1 == next)||(validStates[curr].nextState2 == next))?next: EDS_HA_INVALID)

uns32 eds_amf_init(EDS_CB *);

/* Declarations for the amf callback routines */

void
eds_amf_CSI_set_callback(SaInvocationT invocation,
			 const SaNameT *compName, SaAmfHAStateT haState, SaAmfCSIDescriptorT csiDescriptor);

void eds_amf_health_chk_callback(SaInvocationT invocation, const SaNameT *compName, SaAmfHealthcheckKeyT *checkType);

void eds_amf_comp_terminate_callback(SaInvocationT invocation, const SaNameT *compName);

void
eds_amf_csi_rmv_callback(SaInvocationT invocation,
			 const SaNameT *compName, const SaNameT *csiName, const SaAmfCSIFlagsT csiFlags);
uns32 eds_amf_register(EDS_CB *);

SaAisErrorT eds_clm_init(EDS_CB *cb);
void eds_clm_cluster_track_cbk(const SaClmClusterNotificationBufferT *notificationBuffer,
			       SaUint32T numberOfMembers, SaAisErrorT error);
void send_clm_status_change(EDS_CB *cb, SaClmClusterChangesT cluster_change, NODE_ID node_id);

#endif   /* _EDS_AMF_H */
