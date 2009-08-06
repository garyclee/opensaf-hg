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
 
  DESCRIPTION:This module deals with handling of all events in all states they
  occur. 
  ..............................................................................

  Function Included in this Module: Too many to list.
 
 
******************************************************************************
*/

#include "avm.h"

/* Trap pattern array len */
#define AVM_SWITCH_TRAP_PATTERN_ARRAY_LEN             1
static SaEvtEventPatternT avm_switch_trap_pattern_array[AVM_SWITCH_TRAP_PATTERN_ARRAY_LEN] = {
	{20, 20, (SaUint8T *)"AVM_SWITCH_SNMP_TRAP"}
};

/* Trap pattern for entity locked trap */
#define AVM_ENTITY_LOCKED_TRAP_PATTERN_ARRAY_LEN             1
static SaEvtEventPatternT avm_entity_locked_trap_pattern_array[AVM_ENTITY_LOCKED_TRAP_PATTERN_ARRAY_LEN] = {
	{22, 22, (SaUint8T *)"AVM_ENTITY_LOCKED_TRAP"}
};

/* Trap pattern for shutdown failure trap */
#define AVM_ENTITY_INACTIVE_HISV_RET_ERROR_TRAP_PATTERN_ARRAY_LEN             1
static SaEvtEventPatternT
    avm_entity_inactive_hisv_ret_error_trap_pattern_array[AVM_ENTITY_INACTIVE_HISV_RET_ERROR_TRAP_PATTERN_ARRAY_LEN] = {
	{39, 39, (SaUint8T *)"AVM_ENTITY_INACTIVE_HISV_RET_ERROR_TRAP"}
};

static uns32 avm_publish_trap(AVM_CB_T *avm_cb, NCS_TRAP *trap, SaEvtEventPatternT *pattern, uns32 pattern_len);

static uns32 avm_fmat_time_display(struct tm *time_fields, AVM_TIME_T *time, uns8 **time_op)
{
	uns16 year;

	(*time_op) = m_MMGR_ALLOC_AVM_DEFAULT_VAL(AVM_DISPLAY_TIME_LENGTH);
	if (NULL == *time_op) {
		m_AVM_LOG_INVALID_VAL_FATAL(0);
		return NCSCC_RC_FAILURE;
	}
	memset((*time_op), '\0', AVM_DISPLAY_TIME_LENGTH);

	year = m_NCS_OS_HTONS(time_fields->tm_year + 1900);
	(*time_op)[0] = *(char *)&year;
	(*time_op)[1] = *((char *)&year + 1);
	(*time_op)[2] = time_fields->tm_mon;
	(*time_op)[3] = time_fields->tm_mday;
	(*time_op)[4] = time_fields->tm_hour;
	(*time_op)[5] = time_fields->tm_min;
	(*time_op)[6] = time_fields->tm_sec;
	(*time_op)[7] = (time->milliseconds) / 100;

	return NCSCC_RC_SUCCESS;
}

uns32 avm_adm_switch_trap(AVM_CB_T *avm_cb, uns32 trap_id, AVM_SWITCH_STATUS_T param_val)
{

	NCS_TRAP snmptm_avm_trap;
	NCS_TRAP_VARBIND *i_trap_varbind = NULL;
	NCS_TRAP_VARBIND *temp_trap_varbind = NULL;
	uns32 status = NCSCC_RC_FAILURE;
	AVM_TIME_T time;

	time_t tod;
	struct tm *time_fields;
	uns8 *time_op;

	/* Fill in the trap details */
	snmptm_avm_trap.i_trap_tbl_id = NCSMIB_TBL_AVM_TRAPS;
	snmptm_avm_trap.i_trap_id = trap_id;
	snmptm_avm_trap.i_inform_mgr = TRUE;
	snmptm_avm_trap.i_trap_vb = i_trap_varbind;

	if (trap_id == ncsAvmSwitch_ID) {
		i_trap_varbind = m_MMGR_ALLOC_AVM_TRAP_VB;
		if (i_trap_varbind == NULL) {
			m_AVM_LOG_INVALID_VAL_ERROR(0);
			return NCSCC_RC_FAILURE;
		}

		memset(i_trap_varbind, '\0', sizeof(NCS_TRAP_VARBIND));

		/* Fill in the object nfmResourceLink details */
		i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_SCALAR;

		i_trap_varbind->i_param_val.i_param_id = ncsAvmSwitchStatus_ID;
		i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;

		i_trap_varbind->i_param_val.i_length = sizeof(uns32);
		i_trap_varbind->i_param_val.info.i_int = param_val;

		i_trap_varbind->i_idx.i_inst_len = 0;

		temp_trap_varbind = m_MMGR_ALLOC_AVM_TRAP_VB;
		if (NULL == temp_trap_varbind) {
			m_AVM_LOG_INVALID_VAL_ERROR(0);
			m_MMGR_FREE_AVM_TRAP_VB(i_trap_varbind);
			return NCSCC_RC_FAILURE;
		}

		memset(temp_trap_varbind, '\0', sizeof(NCS_TRAP_VARBIND));
		temp_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_SCALAR;
		temp_trap_varbind->i_param_val.i_param_id = ncsAvmSwitchTime_ID;
		temp_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;

		m_GET_MSEC_TIME_STAMP(&time.seconds, &time.milliseconds);
		tod = (time_t)time.seconds;
		time_fields = localtime(&tod);
		status = avm_fmat_time_display(time_fields, &time, &time_op);
		if (NCSCC_RC_SUCCESS != status) {
			m_AVM_LOG_INVALID_VAL_ERROR(status);
			m_MMGR_FREE_AVM_TRAP_VB(i_trap_varbind);
			m_MMGR_FREE_AVM_TRAP_VB(temp_trap_varbind);
			return NCSCC_RC_FAILURE;
		}

		/* strncpy(temp_trap_varbind->i_param_val.info.i_oct, time_op, AVM_DISPLAY_TIME_LENGTH); */
		temp_trap_varbind->i_param_val.info.i_oct = time_op;
		temp_trap_varbind->i_param_val.i_length = AVM_DISPLAY_TIME_LENGTH;

		temp_trap_varbind->i_idx.i_inst_len = 0;

		i_trap_varbind->next_trap_varbind = temp_trap_varbind;
	}

	snmptm_avm_trap.i_trap_vb = i_trap_varbind;

	if (NCSCC_RC_SUCCESS !=
	    (status =
	     avm_publish_trap(avm_cb, &snmptm_avm_trap, avm_switch_trap_pattern_array,
			      AVM_SWITCH_TRAP_PATTERN_ARRAY_LEN))) {
		m_AVM_LOG_INVALID_VAL_ERROR(status);
	}

	m_MMGR_FREE_AVM_TRAP_VB(i_trap_varbind);
	m_MMGR_FREE_AVM_TRAP_VB(temp_trap_varbind);
	m_MMGR_FREE_AVM_DEFAULT_VAL(time_op);
	return status;
}

/*****************************************************************************
  Name          :  avm_entity_locked_trap

  Description   :  This function is used to send a trap when an entity is
                   locked.

  Arguments     :  avm_cb       - Pointer to AVM control block
                   ent_info     - Pointer to  struct representing Entities
                                  in System

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  None

*****************************************************************************/
uns32 avm_entity_locked_trap(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info)
{
	NCS_TRAP trap;
	NCS_TRAP_VARBIND *entity_trap_varbind = NULL;
	uns32 inst_id_len = 0;
	uns32 inst_id[AVM_MAX_INDEX_LEN];
	uns32 i = 0;
	uns32 status = NCSCC_RC_SUCCESS;

	/* Table ID of trap */
	trap.i_trap_tbl_id = NCSMIB_TBL_AVM_TRAPS;
	/* Trap ID */
	trap.i_trap_id = ncsAvmEntityLocked_ID;

	/* Inform manager */
	trap.i_inform_mgr = TRUE;

	/* Parameter in trap */

	/* Entity path */
	/* Allocate memory and initialize */
	entity_trap_varbind = m_MMGR_ALLOC_AVM_TRAP_VB;
	if (entity_trap_varbind == NULL) {
		m_AVM_LOG_INVALID_VAL_ERROR(0);
		return NCSCC_RC_FAILURE;
	}
	memset(entity_trap_varbind, '\0', sizeof(NCS_TRAP_VARBIND));

	/* Prepare index for ncsAvmEntDeployTable */
	inst_id_len = m_NCS_OS_NTOHS(ent_info->ep_str.length);
	memset(inst_id, 0, AVM_MAX_INDEX_LEN);

	inst_id[0] = inst_id_len;

	for (i = 0; i < inst_id_len; i++) {
		inst_id[i + 1] = (uns32)(ent_info->ep_str.name[i]);
	}

	/* Fill */
	entity_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_ENT_DEPLOYMENT;
	entity_trap_varbind->i_param_val.i_param_id = ncsAvmEntPath_ID;
	entity_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
	entity_trap_varbind->i_param_val.i_length = ent_info->ep_str.length;
	entity_trap_varbind->i_param_val.info.i_oct = ent_info->ep_str.name;
	entity_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
	entity_trap_varbind->i_idx.i_inst_ids = inst_id;

	/* Attach to trap */
	trap.i_trap_vb = entity_trap_varbind;

	/* create and send trap */
	status =
	    avm_publish_trap(avm_cb, &trap, &avm_entity_locked_trap_pattern_array[0],
			     AVM_ENTITY_LOCKED_TRAP_PATTERN_ARRAY_LEN);

	/* Deallocate memory */
	m_MMGR_FREE_AVM_TRAP_VB(entity_trap_varbind);

	return status;
}

/*****************************************************************************
  Name          :  avm_entity_inactive_hisv_ret_error_trap

  Description   :  This function is used to send a trap when state of entity
                   is set to inactive in AVM but request to HISV for setting 
                   the state to Inactive returned error. 

  Arguments     :  avm_cb       - Pointer to AVM control block
                   ent_info     - Pointer to  struct representing Entities
                                  in System

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  None

*****************************************************************************/
uns32 avm_entity_inactive_hisv_ret_error_trap(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info)
{
	NCS_TRAP trap;
	NCS_TRAP_VARBIND *entity_trap_varbind = NULL;
	uns32 inst_id_len = 0;
	uns32 inst_id[AVM_MAX_INDEX_LEN];
	uns32 i = 0;
	uns32 status = NCSCC_RC_SUCCESS;

	/* Table ID of trap */
	trap.i_trap_tbl_id = NCSMIB_TBL_AVM_TRAPS;
	/* Trap ID */
	trap.i_trap_id = ncsAvmEntityStateIsNowInactiveButHISVreturnedError_ID;

	/* Inform manager */
	trap.i_inform_mgr = TRUE;

	/* Parameter in trap */

	/* Entity path */
	/* Allocate memory and initialize */
	entity_trap_varbind = m_MMGR_ALLOC_AVM_TRAP_VB;
	if (entity_trap_varbind == NULL) {
		m_AVM_LOG_INVALID_VAL_ERROR(0);
		return NCSCC_RC_FAILURE;
	}
	memset(entity_trap_varbind, '\0', sizeof(NCS_TRAP_VARBIND));

	/* Prepare index for ncsAvmEntDeployTable */
	inst_id_len = m_NCS_OS_NTOHS(ent_info->ep_str.length);
	memset(inst_id, 0, AVM_MAX_INDEX_LEN);
	inst_id[0] = inst_id_len;
	for (i = 0; i < inst_id_len; i++) {
		inst_id[i + 1] = (uns32)(ent_info->ep_str.name[i]);
	}

	/* Fill */
	entity_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_ENT_DEPLOYMENT;
	entity_trap_varbind->i_param_val.i_param_id = ncsAvmEntPath_ID;
	entity_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
	entity_trap_varbind->i_param_val.i_length = ent_info->ep_str.length;
	entity_trap_varbind->i_param_val.info.i_oct = ent_info->ep_str.name;
	entity_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
	entity_trap_varbind->i_idx.i_inst_ids = inst_id;

	/* Attach to trap */
	trap.i_trap_vb = entity_trap_varbind;

	/* create and send trap */
	status = avm_publish_trap(avm_cb, &trap, &avm_entity_inactive_hisv_ret_error_trap_pattern_array[0],
				  AVM_ENTITY_INACTIVE_HISV_RET_ERROR_TRAP_PATTERN_ARRAY_LEN);

	/* Deallocate memory */
	m_MMGR_FREE_AVM_TRAP_VB(entity_trap_varbind);

	return status;
}

static uns32 avm_publish_trap(AVM_CB_T *avm_cb, NCS_TRAP *trap, SaEvtEventPatternT *pattern, uns32 pattern_array_len)
{
	uns32 status = NCSCC_RC_SUCCESS;
	SaEvtEventPatternArrayT patternArray;
	SaEvtEventIdT eventId;
	SaSizeT eventDataSize;
	SaAisErrorT saStatus;
	SaUint8T priority = SA_EVT_HIGHEST_PRIORITY;
	SaTimeT retention_time = AVM_SWITCH_TRAP_RETENTION_TIME;
	SaEvtEventHandleT eventHandle;
	uns32 tlv_size;
	EDU_ERR o_err;
	uns8 *enc_buffer;

	if (FALSE == avm_cb->trap_channel) {
		if (NCSCC_RC_SUCCESS != avm_open_trap_channel(avm_cb)) {
			m_AVM_LOG_INVALID_VAL_ERROR(avm_cb->trap_channel);
			return NCSCC_RC_FAILURE;
		} else {
			avm_cb->trap_channel = TRUE;
		}
	}

	memset(&patternArray, '\0', sizeof(SaEvtEventPatternArrayT));
	memset(&eventHandle, '\0', sizeof(SaEvtEventHandleT));
	memset(&eventId, '\0', sizeof(SaEvtEventIdT));

	if (!trap)
		return NCSCC_RC_FAILURE;

	/* Fill trap version */
	trap->i_version = m_NCS_TRAP_VERSION;

	/* Encode the Trap */
	tlv_size = ncs_tlvsize_for_ncs_trap_get(trap);

	/* Allocate the encoded buffer */
	enc_buffer = (SaUint8T *)m_MMGR_ALLOC_AVM_DEFAULT_VAL(tlv_size);
	if (NULL == enc_buffer) {
		m_AVM_LOG_INVALID_VAL_FATAL(0);
		return NCSCC_RC_FAILURE;
	}

	memset(enc_buffer, '\0', tlv_size);

	/* Encode the Buffer */
	status =
	    m_NCS_EDU_TLV_EXEC(&avm_cb->edu_hdl, ncs_edp_ncs_trap, enc_buffer, tlv_size, EDP_OP_TYPE_ENC, trap, &o_err);
	if (NCSCC_RC_SUCCESS != status) {
		m_MMGR_FREE_AVM_DEFAULT_VAL(enc_buffer);
		m_AVM_LOG_INVALID_VAL_ERROR(o_err);
		return NCSCC_RC_FAILURE;
	}

	eventDataSize = (SaSizeT)tlv_size;

	saStatus = saEvtEventAllocate(avm_cb->trap_chan_hdl, &eventHandle);
	if (SA_AIS_OK != saStatus) {
		m_AVM_LOG_INVALID_VAL_ERROR(saStatus);
		m_MMGR_FREE_AVM_DEFAULT_VAL(enc_buffer);
		return NCSCC_RC_FAILURE;
	}

	patternArray.patternsNumber = pattern_array_len;
	patternArray.patterns = pattern;

	saStatus =
	    saEvtEventAttributesSet(eventHandle, &patternArray, priority, retention_time, &avm_cb->publisherName);
	if (saStatus != SA_AIS_OK) {
		m_AVM_LOG_INVALID_VAL_ERROR(saStatus);
		m_MMGR_FREE_AVM_DEFAULT_VAL(enc_buffer);
		saEvtEventFree(eventHandle);
		return NCSCC_RC_FAILURE;
	}

	saStatus = saEvtEventPublish(eventHandle, enc_buffer, eventDataSize, &eventId);

	if (SA_AIS_OK != saStatus) {
		m_AVM_LOG_INVALID_VAL_ERROR(saStatus);
		m_MMGR_FREE_AVM_DEFAULT_VAL(enc_buffer);
		saEvtEventFree(eventHandle);
		return NCSCC_RC_FAILURE;
	}

	saStatus = saEvtEventFree(eventHandle);
	m_MMGR_FREE_AVM_DEFAULT_VAL(enc_buffer);
	return NCSCC_RC_SUCCESS;

}
