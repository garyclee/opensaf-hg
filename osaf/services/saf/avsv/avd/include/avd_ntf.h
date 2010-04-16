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

#define ADDITION_TEXT_LENGTH 256
#define AMF_NTF_SENDER "safApp=safAmfService"

/* Alarms */
EXTERN_C uns32 avd_gen_comp_inst_failed_ntf(AVD_CL_CB *avd_cb, AVD_COMP *comp);
EXTERN_C uns32 avd_gen_comp_clean_failed_ntf(AVD_CL_CB *avd_cb, AVD_COMP *comp);
EXTERN_C uns32 avd_gen_cluster_reset_ntf(AVD_CL_CB *avd_cb, AVD_COMP *comp);
EXTERN_C uns32 avd_gen_si_unassigned_ntf(AVD_CL_CB *avd_cb, AVD_SI *si);
EXTERN_C uns32 avd_gen_comp_proxy_status_unproxied_ntf(AVD_CL_CB *avd_cb, AVD_COMP *comp);

/* Notifications */

/*Administrative State Change Notify */
EXTERN_C uns32 avd_gen_node_admin_state_changed_ntf(AVD_CL_CB *avd_cb, AVD_AVND *node);
EXTERN_C uns32 avd_gen_su_admin_state_changed_ntf(AVD_CL_CB *avd_cb, AVD_SU *su);
EXTERN_C uns32 avd_gen_sg_admin_state_changed_ntf(AVD_CL_CB *avd_cb, AVD_SG *sg);
EXTERN_C uns32 avd_gen_si_admin_state_chg_ntf(AVD_CL_CB *avd_cb, AVD_SI *si);
/* cluster is missing - AMF B04.01 */
/* application is missing - AMF B04.01 */

/* Operational State Change Notify */
EXTERN_C uns32 avd_gen_su_oper_state_chg_ntf(AVD_CL_CB *avd_cb, AVD_SU *su);
/* node is missing - AMF B04.01 */

/* Presence state change */
EXTERN_C uns32 avd_gen_su_pres_state_chg_ntf(AVD_CL_CB *avd_cb, AVD_SU *su);


EXTERN_C uns32 avd_gen_su_ha_state_changed_ntf(AVD_CL_CB *avd_cb, struct avd_su_si_rel_tag *susi);
/* "HA Readiness state change notify" missing - AMF B04.01 */

EXTERN_C uns32 avd_gen_su_si_assigned_ntf(AVD_CL_CB *avd_cb, struct avd_su_si_rel_tag *susi);

EXTERN_C uns32 avd_gen_comp_proxy_status_proxied_ntf(AVD_CL_CB *avd_cb, AVD_COMP *comp);

/* other alarms/notifications */
EXTERN_C uns32 avd_clm_alarm_service_impaired_ntf(AVD_CL_CB *avd_cb, SaAisErrorT err);

EXTERN_C uns32 avd_clm_node_join_ntf(AVD_CL_CB *avd_cb, AVD_AVND *node);

EXTERN_C uns32 avd_clm_node_exit_ntf(AVD_CL_CB *avd_cb, AVD_AVND *node);

EXTERN_C uns32 avd_clm_node_reconfiured_ntf(AVD_CL_CB *avd_cb, AVD_AVND *node);

EXTERN_C uns32 avd_node_shutdown_failure_ntf(AVD_CL_CB *avd_cb, AVD_AVND *node, uns32 errcode);

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
