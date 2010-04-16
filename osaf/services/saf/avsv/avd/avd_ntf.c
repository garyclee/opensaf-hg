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
  MODULE NAME: AVD_NTF.C 

..............................................................................

  DESCRIPTION:   This file contains routines to generate ntfs .

******************************************************************************/

#include <logtrace.h>
#include <saflog.h>
#include <avd_util.h>
#include <avd_dblog.h>
#include <avd_ntf.h>

/*****************************************************************************
  Name          :  avd_gen_comp_inst_failed_ntf

  Description   :  This function generates a component instantiation failed ntf.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   comp - Pointer to the AVD_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_comp_inst_failed_ntf(AVD_CL_CB *avd_cb, AVD_COMP *comp)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Instantiation Of Component %s Failed", comp->comp_info.name.value);

	status = sendAlarmNotificationAvd(avd_cb,
						comp->comp_info.name,
						add_text,
						SA_SVC_AMF,
						SA_AMF_NTFID_COMP_INSTANTIATION_FAILED, /* 0x02 */
						SA_NTF_TIMING_PROBLEM,
						SA_NTF_SEVERITY_MAJOR);
	return status;

}

/*****************************************************************************
  Name          :  avd_gen_comp_clean_failed_ntf

  Description   :  This function generates a component cleanup failed ntf.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   comp - Pointer to the AVD_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_comp_clean_failed_ntf(AVD_CL_CB *avd_cb, AVD_COMP *comp)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Cleanup Of Component %s Failed", comp->comp_info.name.value);

	status = sendAlarmNotificationAvd(avd_cb,
						comp->comp_info.name,
						add_text,
						SA_SVC_AMF,
						SA_AMF_NTFID_COMP_CLEANUP_FAILED, /* 0x03 */
						SA_NTF_RECEIVE_FAILURE,
						SA_NTF_SEVERITY_MAJOR);

	return status;
}
/*****************************************************************************
  Name          :  avd_gen_cluster_reset_ntf

  Description   :  This function generates a cluster reset ntf as designated 
                   component failed and a cluster reset recovery as recommended
                   by the component is being done.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   comp - Pointer to the AVD_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_cluster_reset_ntf(AVD_CL_CB *avd_cb, AVD_COMP *comp)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = comp->comp_info.name.length;
	(void)memcpy(comp_name.value, comp->comp_info.name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Failure of Component %s triggered cluster reset", comp_name.value);

	status = sendAlarmNotificationAvd(avd_cb,
					  comp_name,
					  add_text,
					  SA_SVC_AMF,
					  SA_AMF_NTFID_CLUSTER_RESET, /* 0x04 */
					  SA_NTF_RECEIVE_FAILURE,
					  SA_NTF_SEVERITY_MAJOR);

	return status;
}

/*****************************************************************************
  Name          :  avd_gen_si_unassigned_ntf

  Description   :  This function generates a si unassigned ntf

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   si - Pointer to the AVD_SI struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_si_unassigned_ntf(AVD_CL_CB *avd_cb, AVD_SI *si)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();
	saflog(LOG_NOTICE, amfSvcUsrName, "SI %s has no active assignments to any SU", si->name.value);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = si->name.length;
	(void)memcpy(comp_name.value, si->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "SI designated by %s has no current active assignments to any SU", comp_name.value);

	status = sendAlarmNotificationAvd(avd_cb,
					  comp_name,
					  add_text,
					  SA_SVC_AMF,
					  SA_AMF_NTFID_SI_UNASSIGNED, /* 0x05 */
					  SA_NTF_SOFTWARE_ERROR,
					  SA_NTF_SEVERITY_MAJOR);

	return status;
}

/*****************************************************************************
  Name          :  avd_gen_comp_proxy_status_unproxied_ntf

  Description   :  This function generates a proxied component orphane ntf.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   comp - Pointer to the AVD_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_comp_proxy_status_unproxied_ntf(AVD_CL_CB *avd_cb, AVD_COMP *comp)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Component %s become orphan", comp->comp_info.name.value);

	status = sendAlarmNotificationAvd(avd_cb,
						comp->comp_info.name,
						add_text,
						SA_SVC_AMF,
						SA_AMF_NTFID_COMP_UNPROXIED, /* 0x06 */
						SA_NTF_UNEXPECTED_INFORMATION,
						SA_NTF_SEVERITY_MAJOR);

	return status;
}

/*****************************************************************************
  Name          :  avd_gen_node_admin_state_changed_ntf

  Description   :  This function generates a node admin state changed ntf

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   node - Pointer to the AVD_AVND struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_node_admin_state_changed_ntf(AVD_CL_CB *avd_cb, AVD_AVND *node)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = node->name.length;
	(void)memcpy(comp_name.value, node->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Admin state of node %s changed", comp_name.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						comp_name,
						add_text,
						SA_SVC_AMF,
						0x65,
						SA_NTF_MANAGEMENT_OPERATION,
					        SA_AMF_ADMIN_STATE,
					        node->saAmfNodeAdminState);

	return status;
}

/*****************************************************************************
  Name          :  avd_gen_sg_admin_state_changed_ntf

  Description   :  This function generates a sg admin state changed ntf

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   sg - Pointer to the AVD_SG struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_sg_admin_state_changed_ntf(AVD_CL_CB *avd_cb, AVD_SG *sg)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();
	avd_log_admin_state_ntfs(sg->saAmfSGAdminState, &(sg->name), NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = sg->name.length;
	(void)memcpy(comp_name.value, sg->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Admin state of SG %s changed", comp_name.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						comp_name,
						add_text,
						SA_SVC_AMF,
						0x67,
						SA_NTF_MANAGEMENT_OPERATION,
						SA_AMF_ADMIN_STATE,
						sg->saAmfSGAdminState);

	return status;
}

/*****************************************************************************
  Name          :  avd_gen_su_admin_state_changed_ntf

  Description   :  This function generates a su admin state changed ntf

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   su - Pointer to the AVD_SU struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_su_admin_state_changed_ntf(AVD_CL_CB *avd_cb, AVD_SU *su)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = su->name.length;
	(void)memcpy(comp_name.value, su->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Admin state of SU %s changed", comp_name.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						comp_name,
						add_text,
						SA_SVC_AMF,
						0x66,
						SA_NTF_MANAGEMENT_OPERATION,
						SA_AMF_ADMIN_STATE,
						su->saAmfSUAdminState);

	return status;
}

/*****************************************************************************
  Name          :  avd_gen_si_admin_state_chg_ntf

  Description   :  This function generates a si admin state change ntf.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   si - Pointer to the AVD_SI struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_si_admin_state_chg_ntf(AVD_CL_CB *avd_cb, AVD_SI *si)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();
	avd_log_admin_state_ntfs(si->saAmfSIAdminState, &(si->name), NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = si->name.length;
	(void)memcpy(comp_name.value, si->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Admin state of SI %s changed", comp_name.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						comp_name,
						add_text,
						SA_SVC_AMF,
						0x68,
						SA_NTF_MANAGEMENT_OPERATION,
						SA_AMF_ADMIN_STATE,
						si->saAmfSIAdminState);

	return status;
}

/*****************************************************************************
  Name          :  avd_gen_su_oper_chg_ntf

  Description   :  This function generates a su oper state change ntf.

  Arguments     :  avd_cb - Pointer to the AVD_CB structure
                   su - Pointer to the AVD_SU struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_su_oper_state_chg_ntf(AVD_CL_CB *avd_cb, AVD_SU *su)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	/* Log the SU oper state  */
	TRACE_ENTER();
	avd_log_oper_state_ntfs(su->saAmfSUOperState, &(su->name), NCSFL_SEV_NOTICE);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Oper state of SU %s changed", su->name.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						 su->name,
						 add_text,
						 SA_SVC_AMF,
						 SA_AMF_NTFID_SU_OP_STATE,
						 SA_NTF_OBJECT_OPERATION,
						 SA_AMF_OP_STATE,
						 su->saAmfSUOperState);
	return status;
}

/*****************************************************************************
  Name          :  avd_gen_su_pres_state_chg_ntf

  Description   :  This function generates a su presense state change ntf.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   su - Pointer to the AVD_SU struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_su_pres_state_chg_ntf(AVD_CL_CB *avd_cb, AVD_SU *su)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Presence state of SU %s changed", su->name.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						 su->name,
						 add_text,
						 SA_SVC_AMF,
						 SA_AMF_NTFID_SU_PRESENCE_STATE,
						 SA_NTF_OBJECT_OPERATION,
						 SA_AMF_PRESENCE_STATE,
						 su->saAmfSUPresenceState);

	return status;
}

/*****************************************************************************
  Name          :  avd_gen_su_ha_state_changed_ntf

  Description   :  This function generates a su ha state changed ntf

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   susi - Pointer to the AVD_SU_SI_REL struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  
*****************************************************************************/
uns32 avd_gen_su_ha_state_changed_ntf(AVD_CL_CB *avd_cb, AVD_SU_SI_REL *susi)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name_su, comp_name_si;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();
	avd_log_susi_ha_ntfs(susi->state, &(susi->su->name), &(susi->si->name), NCSFL_SEV_NOTICE, FALSE);

	memset(comp_name_su.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name_su.length = susi->su->name.length;
	(void)memcpy(comp_name_su.value, susi->su->name.value, comp_name_su.length);

	memset(comp_name_si.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name_si.length = susi->si->name.length;
	(void)memcpy(comp_name_si.value, susi->si->name.value, comp_name_si.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "The HA state of SI %s assigned to SU %s changed", comp_name_si.value, comp_name_su.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						comp_name_su,
						add_text,
						SA_SVC_AMF,
						0x6E,
						SA_NTF_OBJECT_OPERATION,
						SA_AMF_HA_STATE,
						susi->state);

	return status;
}

/*****************************************************************************
  Name          :  avd_gen_su_si_assigned_ntf

  Description   :  This function generates a su ha state changed ntf only when a si 
                   assignment is done to a su.  

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   susi - Pointer to the AVD_SU_SI_REL struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  
*****************************************************************************/
uns32 avd_gen_su_si_assigned_ntf(AVD_CL_CB *avd_cb, AVD_SU_SI_REL *susi)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name_su, comp_name_si;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();
	avd_log_susi_ha_ntfs(susi->state, &(susi->su->name), &(susi->si->name), NCSFL_SEV_NOTICE, TRUE);

	memset(comp_name_su.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name_su.length = susi->su->name.length;
	(void)memcpy(comp_name_su.value, susi->su->name.value, comp_name_su.length);

	memset(comp_name_si.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name_si.length = susi->si->name.length;
	(void)memcpy(comp_name_si.value, susi->si->name.value, comp_name_si.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "The HA state of SI %s assigned to SU %s changed", comp_name_si.value, comp_name_su.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						comp_name_su,
						add_text,
						SA_SVC_AMF,
						0x6E,
						SA_NTF_OBJECT_OPERATION,
						SA_AMF_HA_STATE,
						susi->state);

	return status;
}

/*****************************************************************************
  Name          :  avd_gen_comp_proxy_status_proxied_ntf

  Description   :  This function generates a notify when a component once again
				   is proxied.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_comp_proxy_status_proxied_ntf(AVD_CL_CB *avd_cb, AVD_COMP *comp)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Component %s is now proxied", comp->comp_info.name.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						comp->comp_info.name,
						add_text,
						SA_SVC_AMF,
						SA_AMF_NTFID_COMP_PROXY_STATUS, /* 0x070 */
						SA_NTF_OBJECT_OPERATION,
						SA_AMF_PROXY_STATUS,
						SA_AMF_PROXY_STATUS_PROXIED);

	return status;
}

/*****************************************************************************
  Name          :  avd_clm_alarm_service_impaired_ntf

  Description   :  This function generates a ntf when the CLM service is not
                   able to provide its service.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   err -    Error Code

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_clm_alarm_service_impaired_ntf(AVD_CL_CB *avd_cb, SaAisErrorT err)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH] = "CLM service";
	TRACE_ENTER();

	comp_name.length = strlen((SaInt8T*)add_text);
	(void)memcpy(comp_name.value, add_text, strlen((SaInt8T*)add_text));

	strcpy((SaInt8T*)add_text, "CLM service impaired");

	status = sendAlarmNotificationAvd(avd_cb,
					  comp_name,
					  add_text,
					  SA_SVC_CLM,
					  0x01,
					  SA_NTF_OUT_OF_SERVICE,
					  SA_NTF_SEVERITY_MAJOR);

	return status;
}

/*****************************************************************************
  Name          :  avd_clm_node_join_ntf

  Description   :  This function generate a node join ntf

  Arguments     :  avd_cb -    Pointer to the AVD_CL_CB structure
                   node -      Pointer to the AVD_AVND structure

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         : 
*****************************************************************************/
uns32 avd_clm_node_join_ntf(AVD_CL_CB *avd_cb, AVD_AVND *node)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();
	avd_log_clm_node_ntfs(AVD_NTFS_CLUSTER, AVD_NTFS_JOINED, &(node->name), NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = node->name.length;
	(void)memcpy(comp_name.value, node->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "CLM node %s Joined", comp_name.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						comp_name,
						add_text,
						SA_SVC_CLM,
						0x65,
						SA_NTF_MANAGEMENT_OPERATION,
						SA_CLM_CLUSTER_CHANGE_STATUS,
						SA_CLM_NODE_JOINED);

	return status;
}

/*****************************************************************************
  Name          :  avd_clm_node_exit_ntf

  Description   :  This function generate a node exit ntf

  Arguments     :  avd_cb -    Pointer to the AVD_CL_CB structure
                   node -      Pointer to the AVD_AVND structure

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_clm_node_exit_ntf(AVD_CL_CB *avd_cb, AVD_AVND *node)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();
        avd_log_clm_node_ntfs(AVD_NTFS_CLUSTER, AVD_NTFS_EXITED, &(node->name), NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = node->name.length;
	(void)memcpy(comp_name.value, node->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Clm node %s Exited", comp_name.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						comp_name,
						add_text,
						SA_SVC_CLM,
						0x66,
						SA_NTF_OBJECT_OPERATION,
						SA_CLM_CLUSTER_CHANGE_STATUS,
						SA_CLM_NODE_LEFT);

	return status;
}

/*****************************************************************************
  Name          :  avd_clm_node_reconfiured_ntf

  Description   :  This function generate a node reconfiured ntf

  Arguments     :  avd_cb -    Pointer to the AVD_CL_CB structure
                   node -      Pointer to the AVD_AVND structure

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_clm_node_reconfiured_ntf(AVD_CL_CB *avd_cb, AVD_AVND *node)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = node->name.length;
	(void)memcpy(comp_name.value, node->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Member Node %s Reconfigured", comp_name.value);

	status = sendStateChangeNotificationAvd(avd_cb,
						comp_name,
						add_text,
						SA_SVC_CLM,
						0x67,
						SA_NTF_OBJECT_OPERATION,
						SA_CLM_CLUSTER_CHANGE_STATUS,
						SA_CLM_NODE_RECONFIGURED);

	return status;
}

/*****************************************************************************
  Name          :  avd_node_shutdown_failure_ntf

  Description   :  This function sends the ntf corresponding to a failure
                   of shutdown for a node.

  Arguments     :  avd_cb   - Pointer to the AVD control block
                   node     - Pointer to AVD-AvND data structure
                   errcode  - Value which indicates the result that occurred

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  errcode =
                  1: Node is active system controller
                  2: SUs are in same SG on node
                  3: SG is unstable
*****************************************************************************/
uns32 avd_node_shutdown_failure_ntf(AVD_CL_CB *avd_cb, AVD_AVND *node, uns32 errcode)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	TRACE_ENTER();
	avd_log_shutdown_failure(&(node->name), NCSFL_SEV_NOTICE, errcode);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = node->name.length;
	(void)memcpy(comp_name.value, node->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Node shutdown failure of %s node", comp_name.value);

	status = sendAlarmNotificationAvd(avd_cb,
					  comp_name,
					  add_text,
					  SA_SVC_AMF,
					  0x64,
					  SA_NTF_RECEIVE_FAILURE,
					  SA_NTF_SEVERITY_MAJOR);

	return status;
}

void fill_ntf_header_part_avd(SaNtfNotificationHeaderT *notificationHeader,
			      SaNtfEventTypeT eventType,
			      SaNameT comp_name,
			      SaUint8T *add_text,
			      SaUint16T majorId,
			      SaUint16T minorId,
			      SaInt8T *avd_name)
{
	*notificationHeader->eventType = eventType;
	*notificationHeader->eventTime = (SaTimeT)SA_TIME_UNKNOWN;

	notificationHeader->notificationObject->length = comp_name.length;
	(void)memcpy(notificationHeader->notificationObject->value, comp_name.value, comp_name.length);

	notificationHeader->notifyingObject->length = strlen(avd_name);
	(void)memcpy(notificationHeader->notifyingObject->value, avd_name, strlen(avd_name));

	notificationHeader->notificationClassId->vendorId = SA_NTF_VENDOR_ID_SAF;
	notificationHeader->notificationClassId->majorId = majorId;
	notificationHeader->notificationClassId->minorId = minorId;

	(void)strcpy(notificationHeader->additionalText, (SaInt8T*)add_text);

}

uns32 sendAlarmNotificationAvd(AVD_CL_CB *avd_cb,
			       SaNameT comp_name,
			       SaUint8T *add_text,
			       SaUint16T majorId,
			       SaUint16T minorId,
			       uns32 probableCause,
			       uns32 perceivedSeverity)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNtfAlarmNotificationT myAlarmNotification;

	status = saNtfAlarmNotificationAllocate(avd_cb->ntfHandle, &myAlarmNotification,
						/* numCorrelatedNotifications */
						0,
						/* lengthAdditionalText */
						strlen((char*)add_text)+1,
						/* numAdditionalInfo */
						0,
						/* numSpecificProblems */
						0,
						/* numMonitoredAttributes */
						0,
						/* numProposedRepairActions */
						0,
						/*variableDataSize */
						0);

	if (status != SA_AIS_OK) {
		LOG_ER("%s: saNtfAlarmNotificationAllocate Failed (%u)", __FUNCTION__, status);
		return NCSCC_RC_FAILURE;
	}

	fill_ntf_header_part_avd(&myAlarmNotification.notificationHeader,
				 SA_NTF_ALARM_PROCESSING,
				 comp_name,
				 add_text,
				 majorId,
				 minorId,
				 AMF_NTF_SENDER);

	*(myAlarmNotification.probableCause) = probableCause;
	*(myAlarmNotification.perceivedSeverity) = perceivedSeverity;

	status = saNtfNotificationSend(myAlarmNotification.notificationHandle);

	if (status != SA_AIS_OK) {
		saNtfNotificationFree(myAlarmNotification.notificationHandle);
		LOG_ER("%s: saNtfNotificationSend Failed (%u)", __FUNCTION__, status);
		return NCSCC_RC_FAILURE;
	}

	status = saNtfNotificationFree(myAlarmNotification.notificationHandle);

	if (status != SA_AIS_OK) {
		LOG_ER("%s: saNtfNotificationFree Failed (%u)", __FUNCTION__, status);
		return NCSCC_RC_FAILURE;
	}

	return status;

}

uns32 sendStateChangeNotificationAvd(AVD_CL_CB *avd_cb,
				     SaNameT comp_name,
				     SaUint8T *add_text,
				     SaUint16T majorId,
				     SaUint16T minorId,
				     uns32 sourceIndicator,
				     SaUint16T stateId,
				     SaUint16T newState)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNtfStateChangeNotificationT myStateNotification;

	status = saNtfStateChangeNotificationAllocate(avd_cb->ntfHandle,/* handle to Notification Service instance */
						      &myStateNotification,
						      /* number of correlated notifications */
						      0,
						      /* length of additional text */
						      strlen((char*)add_text)+1,
						      /* number of additional info items */
						      0,
						      /* number of state changes */
						      1,
						      /* use default allocation size */
						      0);

	if (status != SA_AIS_OK) {
		LOG_ER("%s: saNtfStateChangeNotificationAllocate Failed (%u)", __FUNCTION__, status);
		return NCSCC_RC_FAILURE;
	}

	fill_ntf_header_part_avd(&myStateNotification.notificationHeader,
				 SA_NTF_OBJECT_STATE_CHANGE,
				 comp_name,
				 add_text,
				 majorId,
				 minorId,
				 AMF_NTF_SENDER);

	*(myStateNotification.sourceIndicator) = sourceIndicator;
	myStateNotification.changedStates->stateId = stateId;
	myStateNotification.changedStates->oldStatePresent = SA_FALSE;
	myStateNotification.changedStates->newState = newState;

	status = saNtfNotificationSend(myStateNotification.notificationHandle);

	if (status != SA_AIS_OK) {
		saNtfNotificationFree(myStateNotification.notificationHandle);
		LOG_ER("%s: saNtfNotificationSend Failed (%u)", __FUNCTION__, status);
		return NCSCC_RC_FAILURE;
	}

	status = saNtfNotificationFree(myStateNotification.notificationHandle);

	if (status != SA_AIS_OK) {
		LOG_ER("%s: saNtfNotificationFree Failed (%u)", __FUNCTION__, status);
		return NCSCC_RC_FAILURE;
	}

	return status;

}
