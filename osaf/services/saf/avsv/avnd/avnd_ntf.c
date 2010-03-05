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
  MODULE NAME: AVND_NTF.C 

..............................................................................

  DESCRIPTION:   This file contains routines to generate ntfs .

******************************************************************************/

#include "avnd.h"

/*****************************************************************************
  Name          :  avnd_gen_comp_inst_failed_ntf

  Description   :  This function generates a component instantiation failed ntf.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_comp_inst_failed_ntf(AVND_CB *avnd_cb, AVND_COMP *comp)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	/* Log the componet insantiation failed info */
	avnd_log_clc_ntfs(AVND_LOG_NTFS_INSTANTIATION, AVND_LOG_NTFS_FAILED, &(comp->name), NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = comp->name.length;
	(void)memcpy(comp_name.value, comp->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Instentiation Of Component %s Failed", comp_name.value);

	status = sendAlarmNotificationAvnd(avnd_cb,
					   comp_name,
					   add_text,
					   0x02,
					   SA_NTF_TIMING_PROBLEM,
					   SA_NTF_SEVERITY_MAJOR);
	return status;

}

/*****************************************************************************
  Name          :  avnd_gen_comp_term_failed_ntf

  Description   :  This function generates a component termination failed ntf.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_comp_term_failed_ntf(AVND_CB *avnd_cb, AVND_COMP *comp)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	/* Log the componet termination failed info */
	avnd_log_clc_ntfs(AVND_LOG_NTFS_TERMINATION, AVND_LOG_NTFS_FAILED, &(comp->name), NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = comp->name.length;
	(void)memcpy(comp_name.value, comp->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Termination Of Component %s Failed", comp_name.value);

	status = sendAlarmNotificationAvnd(avnd_cb,
					   comp_name,
					   add_text,
					   0x03,
					   SA_NTF_RECEIVE_FAILURE,
					   SA_NTF_SEVERITY_MAJOR);

	return status;
}

/*****************************************************************************
  Name          :  avnd_gen_comp_clean_failed_ntf

  Description   :  This function generates a component cleanup failed ntf.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_comp_clean_failed_ntf(AVND_CB *avnd_cb, AVND_COMP *comp)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	/* Log the cleanup insantiation failed info */
	avnd_log_clc_ntfs(AVND_LOG_NTFS_CLEANUP, AVND_LOG_NTFS_FAILED, &(comp->name), NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = comp->name.length;
	(void)memcpy(comp_name.value, comp->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Cleanup Of Component %s Failed", comp_name.value);

	status = sendAlarmNotificationAvnd(avnd_cb,
					   comp_name,
					   add_text,
					   0x03,
					   SA_NTF_RECEIVE_FAILURE,
					   SA_NTF_SEVERITY_MAJOR);

	return status;
}

/*****************************************************************************
  Name          :  avnd_gen_comp_proxied_orphaned_ntf

  Description   :  This function generates a proxied component orphane ntf.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_comp_proxied_orphaned_ntf(AVND_CB *avnd_cb, AVND_COMP *comp)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	/* Log the component orphaned info  */
	if (comp->pxy_comp)
		avnd_log_proxied_orphaned_ntfs(AVND_LOG_NTFS_ORPHANED,
					       AVND_LOG_NTFS_FAILED,
					       &(comp->name),
					       &(comp->pxy_comp->name),
					       NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = comp->name.length;
	(void)memcpy(comp_name.value, comp->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Component %s become orphan", comp_name.value);

	status = sendAlarmNotificationAvnd(avnd_cb,
					   comp_name,
					   add_text,
				           0x06,
					   SA_NTF_UNEXPECTED_INFORMATION,
					   SA_NTF_SEVERITY_MAJOR);

	return status;
}

/*****************************************************************************
  Name          :  avnd_gen_su_oper_chg_ntf

  Description   :  This function generates a su oper state change ntf.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   su - Pointer to the AVND_SU struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_su_oper_state_chg_ntf(AVND_CB *avnd_cb, AVND_SU *su)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	/* Log the SU oper state  */
	avnd_log_su_oper_state_ntfs(su->oper, &(su->name), NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = su->name.length;
	(void)memcpy(comp_name.value, su->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Oper state of SU %s changed", comp_name.value);

	status = sendStateChangeNotificationAvnd(avnd_cb,
						 comp_name,
						 add_text,
						 0x6C,
						 SA_NTF_OBJECT_OPERATION,
						 SA_AMF_OP_STATE,
						 su->oper);
	return status;
}

/*****************************************************************************
  Name          :  avnd_gen_su_pres_state_chg_ntf

  Description   :  This function generates a su presense state change ntf.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   su - Pointer to the AVND_SU struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_su_pres_state_chg_ntf(AVND_CB *avnd_cb, AVND_SU *su)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	/* Log the SU oper state  */
	avnd_log_su_pres_state_ntfs(su->pres, &(su->name), NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = su->name.length;
	(void)memcpy(comp_name.value, su->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Present state of SU %s changed", comp_name.value);

	status = sendStateChangeNotificationAvnd(avnd_cb,
						 comp_name,
						 add_text,
						 0x6D,
						 SA_NTF_OBJECT_OPERATION,
						 SA_AMF_PRESENCE_STATE,
						 su->pres);

	return status;
}

/*****************************************************************************
  Name          :  avnd_gen_comp_fail_on_node_ntf

  Description   :  This function generates a component failed ntf.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp    - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_comp_fail_on_node_ntf(AVND_CB *avnd_cb, AVND_ERR_SRC errSrc, AVND_COMP *comp)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNameT comp_name;
	SaUint8T add_text[SA_MAX_NAME_LENGTH];

	avnd_log_comp_failed_ntfs(avnd_cb->node_info.nodeId, &(comp->name), errSrc-1, NCSFL_SEV_NOTICE);

	memset(comp_name.value, '\0', SA_MAX_NAME_LENGTH);
	comp_name.length = comp->name.length;
	(void)memcpy(comp_name.value, comp->name.value, comp_name.length);

	memset(&add_text, '\0', sizeof(add_text));
	sprintf((SaInt8T*)add_text, "Component %s Failed on node", comp_name.value);

	status = sendAlarmNotificationAvnd(avnd_cb,
					   comp_name,
					   add_text,
					   0x63,
					   SA_NTF_RECEIVE_FAILURE,
					   SA_NTF_SEVERITY_MAJOR);

	return status;
}

void fill_ntf_header_part_avnd(SaNtfNotificationHeaderT *notificationHeader,
			       SaNtfEventTypeT eventType,
			       SaNameT comp_name,
			       SaUint8T *add_text,
			       SaUint16T minorid,
			       SaInt8T *avnd_name)
{
	*notificationHeader->eventType = eventType;
	*notificationHeader->eventTime = (SaTimeT)SA_TIME_UNKNOWN;

	notificationHeader->notificationObject->length = comp_name.length;
	(void)memcpy(notificationHeader->notificationObject->value, comp_name.value, comp_name.length);

	notificationHeader->notifyingObject->length = strlen(avnd_name);
	(void)memcpy(notificationHeader->notifyingObject->value, avnd_name, strlen(avnd_name));

	notificationHeader->notificationClassId->vendorId = SA_NTF_VENDOR_ID_SAF;
	notificationHeader->notificationClassId->majorId = SA_SVC_AMF;
	notificationHeader->notificationClassId->minorId = minorid;

	(void)strcpy(notificationHeader->additionalText, (SaInt8T*)add_text);

}

uns32 sendAlarmNotificationAvnd(AVND_CB *avnd_cb,
				SaNameT comp_name,
				SaUint8T *add_text,
				SaUint16T minorid,
				uns32 probableCause,
				uns32 perceivedSeverity)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNtfAlarmNotificationT myAlarmNotification;

	status = saNtfAlarmNotificationAllocate(avnd_cb->ntfHandle, &myAlarmNotification,
						/* numCorrelatedNotifications */
						0,
						/* lengthAdditionalText */
						ADDITION_TEXT_LENGTH,
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
		/* log the error code here */
		avnd_log(NCSFL_SEV_ERROR,"saNtfAlarmNotificationAllocate Failed (%u)",status);
		return NCSCC_RC_FAILURE;
	}

	fill_ntf_header_part_avnd(&myAlarmNotification.notificationHeader,
				  SA_NTF_ALARM_PROCESSING,
				  comp_name,
				  add_text,
				  minorid,
				  AVND_NTF_SENDER);

	*(myAlarmNotification.probableCause) = probableCause;
	*(myAlarmNotification.perceivedSeverity) = perceivedSeverity;

	status = saNtfNotificationSend(myAlarmNotification.notificationHandle);

	if (status != SA_AIS_OK) {
		saNtfNotificationFree(myAlarmNotification.notificationHandle);
		/* log the error code here */
		avnd_log(NCSFL_SEV_ERROR,"saNtfNotificationSend Failed (%u)",status);
		return NCSCC_RC_FAILURE;
	}

	status = saNtfNotificationFree(myAlarmNotification.notificationHandle);

	if (status != SA_AIS_OK) {
		/* log the error code here */
		avnd_log(NCSFL_SEV_ERROR,"saNtfNotificationFree Failed (%u)",status);
		return NCSCC_RC_FAILURE;
	}

	return status;

}

uns32 sendStateChangeNotificationAvnd(AVND_CB *avnd_cb,
				      SaNameT comp_name,
				      SaUint8T *add_text,
				      SaUint16T minorId,
				      uns32 sourceIndicator,
				      SaUint16T stateId,
				      SaUint16T newState)
{
	uns32 status = NCSCC_RC_FAILURE;
	SaNtfStateChangeNotificationT myStateNotification;

	status = saNtfStateChangeNotificationAllocate(avnd_cb->ntfHandle,/* handle to Notification Service instance */
						      &myStateNotification,
						      /* number of correlated notifications */
						      0,
						      /* length of additional text */
						      ADDITION_TEXT_LENGTH,
						      /* number of additional info items */
						      0,
						      /* number of state changes */
						      1,
						      /* use default allocation size */
						      0);

	if (status != SA_AIS_OK) {
		/* log the error code here */
		avnd_log(NCSFL_SEV_ERROR,"saNtfStateChangeNotificationAllocate Failed (%u)",status);
		return NCSCC_RC_FAILURE;
	}

	fill_ntf_header_part_avnd(&myStateNotification.notificationHeader,
				  SA_NTF_OBJECT_STATE_CHANGE,
				  comp_name,
				  add_text,
				  minorId,
				  AVND_NTF_SENDER);

	*(myStateNotification.sourceIndicator) = sourceIndicator;
	myStateNotification.changedStates->stateId = stateId;
	myStateNotification.changedStates->oldStatePresent = SA_FALSE;
	myStateNotification.changedStates->newState = newState;

	status = saNtfNotificationSend(myStateNotification.notificationHandle);

	if (status != SA_AIS_OK) {
		saNtfNotificationFree(myStateNotification.notificationHandle);
		/* log the error code here */
		avnd_log(NCSFL_SEV_ERROR,"saNtfNotificationSend Failed (%u)",status);
		return NCSCC_RC_FAILURE;
	}

	status = saNtfNotificationFree(myStateNotification.notificationHandle);

	if (status != SA_AIS_OK) {
		/* log the error code here */
		avnd_log(NCSFL_SEV_ERROR,"saNtfNotificationFree Failed (%u)",status);
		return NCSCC_RC_FAILURE;
	}

	return status;

}
