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
  MODULE NAME: AVD_NTF.H

..............................................................................

  DESCRIPTION:   This file contains avd ntfs related definitions.

******************************************************************************/

#ifndef AVD_NTF_H
#define AVD_NTF_H

#include <avd_comp.h>
#include <avd_susi.h>

#define ADDITION_TEXT_LENGTH 320
#define AMF_NTF_SENDER "safApp=safAmfService"

/* Alarms */
EXTERN_C void avd_send_comp_inst_failed_alarm(const SaNameT *comp_name, const SaNameT *node_name);
EXTERN_C void avd_send_comp_clean_failed_alarm(const SaNameT *comp_name, const SaNameT *node_name);
EXTERN_C void avd_send_cluster_reset_alarm(const SaNameT *comp_name);
EXTERN_C void avd_send_si_unassigned_alarm(const SaNameT *si_name);
EXTERN_C void avd_send_comp_proxy_status_unproxied_alarm(const SaNameT *comp_name);

/* Notifications */
EXTERN_C void avd_send_admin_state_chg_ntf(const SaNameT *name, SaAmfNotificationMinorIdT minor_id,
					SaAmfAdminStateT old_state, SaAmfAdminStateT new_state);
EXTERN_C void avd_send_oper_chg_ntf(const SaNameT *name, SaAmfNotificationMinorIdT minor_id, 
					SaAmfOperationalStateT old_state, SaAmfOperationalStateT new_state);
EXTERN_C void avd_send_su_pres_state_chg_ntf(const SaNameT *su_name, SaAmfPresenceStateT old_state,
					SaAmfPresenceStateT new_state);
EXTERN_C void avd_send_su_ha_state_chg_ntf(const SaNameT *su_name, const SaNameT *si_name,
					SaAmfHAStateT old_state, SaAmfHAStateT new_state);
EXTERN_C void avd_send_su_ha_readiness_state_chg_ntf(const SaNameT *su_name, const SaNameT *si_name,
					SaAmfHAReadinessStateT old_state, SaAmfHAReadinessStateT new_state);
EXTERN_C void avd_send_si_assigned_ntf(const SaNameT *si_name, SaAmfAssignmentStateT old_state,
					SaAmfAssignmentStateT new_state);
EXTERN_C void avd_send_comp_proxy_status_proxied_ntf(const SaNameT *comp_name,
					SaAmfProxyStatusT old_state, SaAmfProxyStatusT new_state);

/* general functions */
EXTERN_C void fill_ntf_header_part(SaNtfNotificationHeaderT *notificationHeader,
				   SaNtfEventTypeT eventType,
				   SaNameT *comp_name,
				   SaUint8T *add_text,
				   SaUint16T majorId,
				   SaUint16T minorId,
				   SaInt8T *avnd_name);

EXTERN_C uns32 sendAlarmNotificationAvd(AVD_CL_CB *avd_cb,
					SaNameT comp_name,
					SaUint8T *add_text,
					SaUint16T majorId,
					SaUint16T minorId,
					uns32 probableCause,
					uns32 perceivedSeverity);

EXTERN_C uns32 sendStateChangeNotificationAvd(AVD_CL_CB *avd_cb,
					      SaNameT comp_name,
					      SaUint8T *add_text,
					      SaUint16T majorId,
					      SaUint16T minorId,
					      uns32 sourceIndicator,
					      SaUint16T stateId,
					      SaUint16T newState);

#endif
