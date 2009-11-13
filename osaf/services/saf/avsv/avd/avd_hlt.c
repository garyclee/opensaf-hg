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

  DESCRIPTION:This module deals with the creation, accessing and deletion of
  the health check database on the AVD. It also deals with all the MIB 
  operations like set,get,getnext etc related to the health check table.

  The key_size of patricia node is sizeof(AVSV_HLT_KEY) - sizeof(SaUint16T)
  In all the patricia operations however we fill the entire AVSV_HLT_KEY and 
  patricia functions will take of comparing to the appropriate length.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <saImmOm.h>
#include <immutil.h>
#include <avd_dblog.h>
#include <avd_imm.h>
#include <avd_comp.h>

static SaAisErrorT avd_hc_ccb_completed_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
		SaTimeT *value = (SaTimeT *)attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "saAmfHealthcheckPeriod")) {
			if (*value < (100 * SA_TIME_ONE_MILLISECOND)) {
				avd_log(NCSFL_SEV_ERROR, "Unreasonable saAmfHealthcheckPeriod %llu for '%s' ",
					*value, opdata->objectName.value);
				return SA_AIS_ERR_BAD_OPERATION;
			}
		} else if (!strcmp(attribute->attrName, "saAmfHealthcheckMaxDuration")) {
			if (*value < (100 * SA_TIME_ONE_MILLISECOND)) {
				avd_log(NCSFL_SEV_ERROR, "Unreasonable saAmfHealthcheckMaxDuration %llu for '%s' ",
					*value, opdata->objectName.value);
				return SA_AIS_ERR_BAD_OPERATION;
			}
		} else
			assert(0);
	}

	return SA_AIS_OK;
}

static SaAisErrorT avd_hc_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		rc = avd_hc_ccb_completed_modify_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		rc = SA_AIS_ERR_BAD_OPERATION;
		break;
	default:
		assert(0);
		break;
	}

	return rc;
}

static void avd_hc_ccb_apply_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;
	const AVD_COMP *comp;
	SaNameT comp_dn;
	char *comp_name;
	uns32 rc;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	comp_name = strstr((char *)opdata->objectName.value, "safComp");
	assert(comp_name);
	comp_dn.length = sprintf((char *)comp_dn.value, "%s", comp_name);
	comp = avd_comp_find(&comp_dn);
	assert(comp);

	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		AVSV_PARAM_INFO param;
		const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
		SaTimeT *param_val = (SaTimeT *)attribute->attrValues[0];

		memset(&param, 0, sizeof(param));
		param.class_id = AVSV_SA_AMF_HEALTH_CHECK;
		param.act = AVSV_OBJ_OPR_MOD;
		param.name = opdata->objectName;
		param.value_len = sizeof(*param_val);
		memcpy(param.value, param_val, param.value_len);

		if (!strcmp(attribute->attrName, "saAmfHealthcheckPeriod")) {
			param.attr_id = saAmfHealthcheckPeriod_ID;
		} else if (!strcmp(attribute->attrName, "saAmfHealthcheckMaxDuration")) {
			param.attr_id = saAmfHealthcheckMaxDuration_ID;
		} else
			assert(0);

		rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
		if (rc != NCSCC_RC_SUCCESS)
			avd_log(NCSFL_SEV_ERROR, "avd_snd_op_req_msg FAILED, '%s'", opdata->objectName.value);
	}
}

static void avd_hc_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE:
		break;
	case CCBUTIL_MODIFY:
		avd_hc_ccb_apply_modify_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static SaAisErrorT avd_hctype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfHealthcheckType not supported");
		break;
	case CCBUTIL_DELETE:
		rc = SA_AIS_OK;
		break;
	default:
		assert(0);
		break;
	}

	return rc;
}

static void avd_hctype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE:
		break;
	default:
		assert(0);
		break;
	}
}

void avd_hc_constructor(void)
{
	avd_class_impl_set("SaAmfHealthcheck", NULL, NULL,
		avd_hc_ccb_completed_cb, avd_hc_ccb_apply_cb);
	avd_class_impl_set("SaAmfHealthcheckType", NULL, NULL,
		avd_hctype_ccb_completed_cb, avd_hctype_ccb_apply_cb);
}

