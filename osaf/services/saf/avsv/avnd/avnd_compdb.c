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

   Vinay Khanna

..............................................................................

  DESCRIPTION:
   This module deals with the creation, accessing and deletion of the component
   database in the AVND.

..............................................................................

  FUNCTIONS:

  
******************************************************************************
*/

#include "avnd.h"

#include <saImmOm.h>
#include <immutil.h>

/* AMF Class SaAmfCompGlobalAttributes */
typedef struct {
	SaUint32T saAmfNumMaxInstantiateWithoutDelay;
	SaUint32T saAmfNumMaxInstantiateWithDelay;
	SaUint32T saAmfNumMaxAmStartAttempts;
	SaUint32T saAmfNumMaxAmStopAttempts;
	SaTimeT saAmfDelayBetweenInstantiateAttempts;
} amf_comp_global_attr_t;

static amf_comp_global_attr_t comp_global_attrs;

/* AMF Class SaAmfCompType */
typedef struct amf_comp_type {
	NCS_PATRICIA_NODE tree_node;	/* name is key */
	SaNameT    name;
	saAmfCompCategoryT saAmfCtCompCategory;
	SaNameT    saAmfCtSwBundle;
	SaStringT *saAmfCtDefCmdEnv;
	SaTimeT    saAmfCtDefClcCliTimeout;
	SaTimeT    saAmfCtDefCallbackTimeout;
	SaStringT  saAmfCtRelPathInstantiateCmd;
	SaStringT  saAmfCtDefInstantiateCmdArgv;
	SaUint32T  saAmfCtDefInstantiationLevel;
	SaStringT  saAmfCtRelPathTerminateCmd;
	SaStringT  saAmfCtDefTerminateCmdArgv;
	SaStringT  saAmfCtRelPathCleanupCmd;
	SaStringT  saAmfCtDefCleanupCmdArgv;
	SaStringT  saAmfCtRelPathAmStartCmd;
	SaStringT  saAmfCtDefAmStartCmdArgv;
	SaStringT  saAmfCtRelPathAmStopCmd;
	SaStringT  saAmfCtDefAmStopCmdArgv;
	SaTimeT    saAmfCompQuiescingCompleteTimeout;
	SaAmfRecommendedRecoveryT saAmfCtDefRecoveryOnError;
	SaBoolT saAmfCtDefDisableRestart;
} amf_comp_type_t;

static SaAisErrorT avnd_comp_config_get(const SaNameT *comp_name);

/* We should only get the config attributes from IMM to avoid problems
   with a pure runtime attributes */
static SaImmAttrNameT compConfigAttributes[] = {
    "saAmfCompType",
    "saAmfCompCmdEnv,"
    "saAmfCompInstantiateCmdArgv",
    "saAmfCompInstantiateTimeout",
    "saAmfCompInstantiateLevel",
    "saAmfCompNumMaxInstantiateWithoutDelay",
    "saAmfCompNumMaxInstantiateWithDelay",
    "saAmfCompDelayBetweenInstantiateAttempts",
    "saAmfCompTerminateCmdArgv",
    "saAmfCompCleanupCmdArgv"
    "saAmfCompTerminateTimeout",
    "saAmfCompCleanupTimeout",
    "saAmfCompAmStartCmdArgv",
    "saAmfCompAmStartTimeout",
    "saAmfCompNumMaxAmStartAttempts",
    "saAmfCompAmStopCmdArgv",
    "saAmfCompAmStopTimeout",
    "saAmfCompNumMaxAmStopAttempts",
    "saAmfCompCSISetCallbackTimeout",
    "saAmfCompCSIRmvCallbackTimeout",
    "saAmfCompQuiescingCompleteTimeout",
    "saAmfCompRecoveryOnError",
    "saAmfCompDisableRestart"
};

/*****************************************************************************
 ****  Component Part of AVND AMF Configuration Database Layout           **** 
 *****************************************************************************
 
                   AVND_COMP
                   ---------------- 
   AVND_CB        | Stuff...       |
   -----------    | Attrs          |
  | COMP-Tree |-->|                |
  | ....      |   |                |
  | ....      |   |                |
   -----------    |                |
                  |                |
                  |                |
                  | Proxy ---------|-----> AVND_COMP (Proxy)
   AVND_SU        |                |
   -----------    |                |
  | Child     |-->|-SU-Comp-Next---|-----> AVND_COMP (Next)
  | Comp-List |   |                |
  |           |<--|-Parent SU      |       AVND_COMP_CSI_REC
   -----------    |                |       -------------
                  | CSI-Assign-List|----->|             |
                  |                |      |             |
                   ----------------        -------------

****************************************************************************/

static SaAisErrorT avnd_compglobalattrs_config_get(void)
{
	SaAisErrorT rc = SA_AIS_ERR_FAILED_OPERATION;
	const SaImmAttrValuesT_2 **attributes;
	SaImmAccessorHandleT accessorHandle;
	SaNameT dn = {.value = "safRdn=compGlobalAttributes,safApp=safAmfService" };

	dn.length = strlen((char *)dn.value);

	immutil_saImmOmAccessorInitialize(avnd_cb->immOmHandle, &accessorHandle);
	rc = immutil_saImmOmAccessorGet_2(accessorHandle, &dn, NULL, (SaImmAttrValuesT_2 ***)&attributes);
	if (rc != SA_AIS_OK) {
		avnd_log(NCSFL_SEV_ERROR, "saImmOmAccessorGet_2 FAILED %u", rc);
		goto done;
	}

	avnd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

	if (immutil_getAttr("saAmfNumMaxInstantiateWithoutDelay", attributes, 0,
			    &comp_global_attrs.saAmfNumMaxInstantiateWithoutDelay) != SA_AIS_OK) {
		comp_global_attrs.saAmfNumMaxInstantiateWithoutDelay = 2;
	}

	if (immutil_getAttr("saAmfNumMaxInstantiateWithDelay", attributes, 0,
			    &comp_global_attrs.saAmfNumMaxInstantiateWithDelay) != SA_AIS_OK) {
		comp_global_attrs.saAmfNumMaxInstantiateWithDelay = 0;
	}

	if (immutil_getAttr("saAmfNumMaxAmStartAttempts", attributes, 0,
			    &comp_global_attrs.saAmfNumMaxAmStartAttempts) != SA_AIS_OK) {
		comp_global_attrs.saAmfNumMaxAmStartAttempts = 2;
	}

	if (immutil_getAttr("saAmfNumMaxAmStopAttempts", attributes, 0,
			    &comp_global_attrs.saAmfNumMaxAmStopAttempts) != SA_AIS_OK) {
		comp_global_attrs.saAmfNumMaxAmStopAttempts = 2;
	}

	if (immutil_getAttr("saAmfDelayBetweenInstantiateAttempts", attributes, 0,
			    &comp_global_attrs.saAmfDelayBetweenInstantiateAttempts) != SA_AIS_OK) {
		comp_global_attrs.saAmfDelayBetweenInstantiateAttempts = 0;
	}

	immutil_saImmOmAccessorFinalize(accessorHandle);

	rc = SA_AIS_OK;

done:
	return rc;
}

/****************************************************************************
  Name          : avnd_compdb_init
 
  Description   : This routine initializes the component database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_compdb_init(AVND_CB *cb)
{
	NCS_PATRICIA_PARAMS params;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(&params, 0, sizeof(NCS_PATRICIA_PARAMS));

	if (avnd_compglobalattrs_config_get() != SA_AIS_OK)
		return NCSCC_RC_FAILURE;

	params.key_size = sizeof(SaNameT);
	rc = ncs_patricia_tree_init(&cb->compdb, &params);
	if (NCSCC_RC_SUCCESS == rc)
		m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_CREATE, AVND_LOG_COMP_DB_SUCCESS, 0, 0, NCSFL_SEV_INFO);
	else
		m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_CREATE, AVND_LOG_COMP_DB_FAILURE, 0, 0, NCSFL_SEV_CRITICAL);

	return rc;
}

/****************************************************************************
  Name          : avnd_compdb_destroy
 
  Description   : This routine destroys the component database. It deletes 
                  all the component records in the database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_compdb_destroy(AVND_CB *cb)
{
	AVND_COMP *comp = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* scan & delete each comp */
	while (0 != (comp = (AVND_COMP *)ncs_patricia_tree_getnext(&cb->compdb, (uns8 *)0))) {
		/* delete the record 
		   m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, comp, AVND_CKPT_COMP_CONFIG);
		   AvND is going down, but don't send any async update even for 
		   external components, otherwise external components will be deleted
		   from ACT. */
		rc = avnd_compdb_rec_del(cb, &comp->name);
		if (NCSCC_RC_SUCCESS != rc)
			goto err;
	}

	/* finally destroy patricia tree */
	rc = ncs_patricia_tree_destroy(&cb->compdb);
	if (NCSCC_RC_SUCCESS != rc)
		goto err;

	m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_DESTROY, AVND_LOG_COMP_DB_SUCCESS, 0, 0, NCSFL_SEV_INFO);
	return rc;

 err:
	m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_DESTROY, AVND_LOG_COMP_DB_FAILURE, 0, 0, NCSFL_SEV_CRITICAL);
	return rc;
}

/****************************************************************************
  Name          : avnd_compdb_rec_add
 
  Description   : This routine adds a component record to the component 
                  database. If a component is already present, nothing is 
                  done.
 
  Arguments     : cb   - ptr to the AvND control block
                  info - ptr to the component params (comp-name -> nw order)
                  rc   - ptr to the operation result
 
  Return Values : ptr to the component record, if success
                  0, otherwise
 
  Notes         : None.
******************************************************************************/
AVND_COMP *avnd_compdb_rec_add(AVND_CB *cb, AVND_COMP_PARAM *info, uns32 *rc)
{
	AVND_COMP *comp = 0;
	AVND_SU *su = 0;
	SaNameT su_name;

	*rc = NCSCC_RC_SUCCESS;

	/* verify if this component is already present in the db */
	if (0 != m_AVND_COMPDB_REC_GET(cb->compdb, info->name)) {
		*rc = AVND_ERR_DUP_COMP;
		goto err;
	}

	/*
	 * Determine if the SU is present
	 */
	/* extract the su-name from comp dn */
	memset(&su_name, 0, sizeof(SaNameT));
	avsv_cpy_SU_DN_from_DN(&su_name, &info->name);

	/* get the su record */
	su = m_AVND_SUDB_REC_GET(cb->sudb, su_name);
	if (!su) {
		*rc = AVND_ERR_NO_SU;
		goto err;
	}

	/* a fresh comp... */
	comp = calloc(1, sizeof(AVND_COMP));
	if (!comp) {
		*rc = AVND_ERR_NO_MEMORY;
		goto err;
	}

	/*
	 * Update the config parameters.
	 */
	/* update the comp-name (patricia key) */
	memcpy(&comp->name, &info->name, sizeof(SaNameT));

	/* update the component attributes */
	comp->inst_level = info->inst_level;

	comp->is_am_en = info->am_enable;

	switch (info->category) {
	case NCS_COMP_TYPE_SA_AWARE:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_SAAWARE);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PREINSTANTIABLE);
		break;

	case NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_SAAWARE);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PREINSTANTIABLE);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
		break;

	case NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_SAAWARE);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
		break;

	case NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PREINSTANTIABLE);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
		break;

	case NCS_COMP_TYPE_EXTERNAL_NON_PRE_INSTANTIABLE:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
		break;

	case NCS_COMP_TYPE_NON_SAF:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
		break;

	default:
		break;
	}			/* switch */

	m_AVND_COMP_RESTART_EN_SET(comp, (info->comp_restart == TRUE) ? FALSE : TRUE);

	comp->cap = info->cap;
	comp->node_id = cb->clmdb.node_info.nodeId;

	/* update CLC params */
	comp->clc_info.inst_retry_max = info->max_num_inst;
	comp->clc_info.am_start_retry_max = info->max_num_amstart;

	/* instantiate cmd params */
	memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].cmd, info->init_info, info->init_len);
	comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].len = info->init_len;
	comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].timeout = info->init_time;

	/* terminate cmd params */
	memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].cmd, info->term_info, info->term_len);
	comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].len = info->term_len;
	comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].timeout = info->term_time;

	/* cleanup cmd params */
	memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].cmd, info->clean_info, info->clean_len);
	comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].len = info->clean_len;
	comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].timeout = info->clean_time;

	/* am-start cmd params */
	memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].cmd, info->amstart_info, info->amstart_len);
	comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].len = info->amstart_len;
	comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].timeout = info->amstart_time;

	/* am-stop cmd params */
	memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].cmd, info->amstop_info, info->amstop_len);
	comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].len = info->amstop_len;
	comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].timeout = info->amstop_time;

	/* update the callback response time out values */
	if (info->terminate_callback_timeout)
		comp->term_cbk_timeout = info->terminate_callback_timeout;
	else
		comp->term_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

	if (info->csi_set_callback_timeout)
		comp->csi_set_cbk_timeout = info->csi_set_callback_timeout;
	else
		comp->csi_set_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

	if (info->quiescing_complete_timeout)
		comp->quies_complete_cbk_timeout = info->quiescing_complete_timeout;
	else
		comp->quies_complete_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

	if (info->csi_rmv_callback_timeout)
		comp->csi_rmv_cbk_timeout = info->csi_rmv_callback_timeout;
	else
		comp->csi_rmv_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

	if (info->proxied_inst_callback_timeout)
		comp->pxied_inst_cbk_timeout = info->proxied_inst_callback_timeout;
	else
		comp->pxied_inst_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

	if (info->proxied_clean_callback_timeout)
		comp->pxied_clean_cbk_timeout = info->proxied_clean_callback_timeout;
	else
		comp->pxied_clean_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

	/* update the default error recovery param */
	comp->err_info.def_rec = info->def_recvr;

	/*
	 * Update the rest of the parameters with default values.
	 */
	if (m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(comp))
		m_AVND_COMP_OPER_STATE_SET(comp, SA_AMF_OPERATIONAL_DISABLED);
	else
		m_AVND_COMP_OPER_STATE_SET(comp, SA_AMF_OPERATIONAL_ENABLED);

	comp->avd_updt_flag = FALSE;

	/* synchronize comp oper state */
	m_AVND_COMP_OPER_STATE_AVD_SYNC(cb, comp, *rc);
	if (NCSCC_RC_SUCCESS != *rc)
		goto err;

	comp->pres = SA_AMF_PRESENCE_UNINSTANTIATED;

	/* create the association with hdl-mngr */
	if ((0 == (comp->comp_hdl = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_AVND, (NCSCONTEXT)comp)))) {
		*rc = AVND_ERR_HDL;
		goto err;
	}

	/* 
	 * Initialize the comp-hc list.
	 */
	comp->hc_list.order = NCS_DBLIST_ANY_ORDER;
	comp->hc_list.cmp_cookie = avnd_dblist_hc_rec_cmp;
	comp->hc_list.free_cookie = 0;

	/* 
	 * Initialize the comp-csi list.
	 */
	comp->csi_list.order = NCS_DBLIST_ASSCEND_ORDER;
	comp->csi_list.cmp_cookie = avsv_dblist_saname_cmp;
	comp->csi_list.free_cookie = 0;

	/* 
	 * Initialize the pm list.
	 */
	avnd_pm_list_init(comp);

	/*
	 * initialize proxied list
	 */
	avnd_pxied_list_init(comp);

	/*
	 * Add to the patricia tree.
	 */
	comp->tree_node.bit = 0;
	comp->tree_node.key_info = (uns8 *)&comp->name;
	*rc = ncs_patricia_tree_add(&cb->compdb, &comp->tree_node);
	if (NCSCC_RC_SUCCESS != *rc) {
		*rc = AVND_ERR_TREE;
		goto err;
	}

	/*
	 * Add to the comp-list (maintained by su)
	 */
	m_AVND_SUDB_REC_COMP_ADD(*su, *comp, *rc);
	if (NCSCC_RC_SUCCESS != *rc) {
		*rc = AVND_ERR_DLL;
		goto err;
	}

	/*
	 * Update su bk ptr.
	 */
	comp->su = su;

	if (TRUE == su->su_is_external) {
		m_AVND_COMP_TYPE_SET_EXT_CLUSTER(comp);
	} else
		m_AVND_COMP_TYPE_SET_LOCAL_NODE(comp);

	m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_REC_ADD, AVND_LOG_COMP_DB_SUCCESS, &info->name, 0, NCSFL_SEV_NOTICE);
	avnd_hc_config_get(comp);
	return comp;

 err:
	if (AVND_ERR_DLL == *rc)
		ncs_patricia_tree_del(&cb->compdb, &comp->tree_node);

	if (comp) {
		if (comp->comp_hdl)
			ncshm_destroy_hdl(NCS_SERVICE_ID_AVND, comp->comp_hdl);

		free(comp);
	}

	m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_REC_ADD, AVND_LOG_COMP_DB_FAILURE, &info->name, 0, NCSFL_SEV_CRITICAL);
	return 0;
}

/****************************************************************************
  Name          : avnd_compdb_rec_del
 
  Description   : This routine deletes a component record from the component 
                  database. 
 
  Arguments     : cb       - ptr to the AvND control block
                  name - ptr to the comp-name (in n/w order)
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : This routine expects a NULL comp-csi list.
******************************************************************************/
uns32 avnd_compdb_rec_del(AVND_CB *cb, SaNameT *name)
{
	AVND_COMP *comp = 0;
	AVND_SU *su = 0;
	SaNameT su_name;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* get the comp */
	comp = m_AVND_COMPDB_REC_GET(cb->compdb, *name);
	if (!comp) {
		rc = AVND_ERR_NO_COMP;
		goto err;
	}

	/* comp should not be attached to any csi when it is being deleted */
	assert(!comp->csi_list.n_nodes);

	/*
	 * Determine if the SU is present
	 */
	/* extract the su-name from comp dn */
	memset(&su_name, 0, sizeof(SaNameT));
	avsv_cpy_SU_DN_from_DN(&su_name, name);
	su_name.length = su_name.length;

	/* get the su record */
	su = m_AVND_SUDB_REC_GET(cb->sudb, su_name);
	if (!su) {
		rc = AVND_ERR_NO_SU;
		goto err;
	}

	/* 
	 * Remove from the comp-list (maintained by su).
	 */
	rc = m_AVND_SUDB_REC_COMP_REM(*su, *comp);
	if (NCSCC_RC_SUCCESS != rc) {
		rc = AVND_ERR_DLL;
		goto err;
	}

	/* 
	 * Remove from the patricia tree.
	 */
	rc = ncs_patricia_tree_del(&cb->compdb, &comp->tree_node);
	if (NCSCC_RC_SUCCESS != rc) {
		rc = AVND_ERR_TREE;
		goto err;
	}

	/* 
	 * Delete the various lists (hc, pm, pg, cbk etc) maintained by this comp.
	 */
	avnd_comp_hc_rec_del_all(cb, comp);
	avnd_comp_cbq_del(cb, comp, FALSE);
	avnd_comp_pm_rec_del_all(cb, comp);

	/* remove the association with hdl mngr */
	ncshm_destroy_hdl(NCS_SERVICE_ID_AVND, comp->comp_hdl);

	m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_REC_DEL, AVND_LOG_COMP_DB_SUCCESS, name, 0, NCSFL_SEV_INFO);

	/* free the memory */
	free(comp);

	return rc;

 err:
	m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_REC_DEL, AVND_LOG_COMP_DB_FAILURE, name, 0, NCSFL_SEV_CRITICAL);
	return rc;
}

/****************************************************************************
  Name          : avnd_compdb_csi_rec_get
 
  Description   : This routine gets the comp-csi relationship record from 
                  the csi-list (maintained on comp).
 
  Arguments     : cb            - ptr to AvND control block
                  comp_name - ptr to the comp-name (n/w order)
                  csi_name      - ptr to the CSI name
 
  Return Values : ptr to the comp-csi record (if any)
 
  Notes         : None
******************************************************************************/
AVND_COMP_CSI_REC *avnd_compdb_csi_rec_get(AVND_CB *cb, SaNameT *comp_name, SaNameT *csi_name)
{
	AVND_COMP_CSI_REC *csi_rec = 0;
	AVND_COMP *comp = 0;

	/* get the comp & csi records */
	comp = m_AVND_COMPDB_REC_GET(cb->compdb, *comp_name);
	if (comp)
		csi_rec = m_AVND_COMPDB_REC_CSI_GET(*comp, *csi_name);

	return csi_rec;
}

/****************************************************************************
  Name          : avnd_compdb_csi_rec_get_next
 
  Description   : This routine gets the next comp-csi relationship record from 
                  the csi-list (maintained on comp).
 
  Arguments     : cb            - ptr to AvND control block
                  comp_name - ptr to the comp-name (n/w order)
                  csi_name      - ptr to the CSI name
 
  Return Values : ptr to the comp-csi record (if any)
 
  Notes         : None
******************************************************************************/
AVND_COMP_CSI_REC *avnd_compdb_csi_rec_get_next(AVND_CB *cb, SaNameT *comp_name, SaNameT *csi_name)
{
	AVND_COMP_CSI_REC *csi = 0;
	AVND_COMP *comp = 0;

	/* get the comp  & the next csi */
	comp = m_AVND_COMPDB_REC_GET(cb->compdb, *comp_name);
	if (comp) {
		if (csi_name->length)
			for (csi = m_AVND_COMPDB_REC_CSI_GET_FIRST(*comp);
			     csi && !(m_CMP_HORDER_SANAMET(*csi_name, csi->name) < 0);
			     csi = m_AVND_COMPDB_REC_CSI_NEXT(*comp, *csi)) ;
		else
			csi = m_AVND_COMPDB_REC_CSI_GET_FIRST(*comp);
	}

	/* found the csi */
	if (csi)
		goto done;

	/* find the csi in the remaining comp recs */
	for (comp = m_AVND_COMPDB_REC_GET_NEXT(cb->compdb, *comp_name); comp;
	     comp = m_AVND_COMPDB_REC_GET_NEXT(cb->compdb, comp->name)) {
		csi = m_AVND_COMPDB_REC_CSI_GET_FIRST(*comp);
		if (csi)
			break;
	}

 done:
	return csi;
}

uns32 avnd_comp_oper_req(AVND_CB *cb, AVSV_PARAM_INFO *param)
{
	uns32 rc = NCSCC_RC_FAILURE;

	avnd_log(NCSFL_SEV_NOTICE, "Op %u, %s", param->act, param->name.value);

	switch (param->act) {
	case AVSV_OBJ_OPR_MOD: {
			AVND_COMP *comp = 0;

			comp = m_AVND_COMPDB_REC_GET(cb->compdb, param->name);
			if (!comp) {
				avnd_log(LOG_ERR, "failed to get %s", param->name.value);
				goto done;
			}

			switch (param->attr_id) {
			case saAmfCompInstantiateCmd_ID:
				memset(&comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].cmd,
				       0,
				       sizeof(comp->
					      clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE -
							    1].cmd));
				memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].cmd,
				       param->value, param->value_len);
				comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].len =
				    param->value_len;
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_INST_CMD);
				break;
			case saAmfCompTerminateCmd_ID:
				memset(&comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].cmd,
				       0,
				       sizeof(comp->clc_info.
					      cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].cmd));
				memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].cmd,
				       param->value, param->value_len);
				comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].len =
				    param->value_len;
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_TERM_CMD);
				break;
			case saAmfCompCleanupCmd_ID:
				memset(&comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].cmd, 0,
				       sizeof(comp->clc_info.
					      cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].cmd));
				memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].cmd,
				       param->value, param->value_len);
				comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].len =
				    param->value_len;
				break;
			case saAmfCompAmStartCmd_ID:
				memset(&comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].cmd, 0,
				       sizeof(comp->clc_info.
					      cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].cmd));
				memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].cmd,
				       param->value, param->value_len);
				comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].len =
				    param->value_len;
				break;
			case saAmfCompAmStopCmd_ID:
				memset(&comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].cmd, 0,
				       sizeof(comp->clc_info.
					      cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].cmd));
				memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].cmd,
				       param->value, param->value_len);
				comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].len =
				    param->value_len;
				break;
			case saAmfCompInstantiateTimeout_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].timeout =
				    m_NCS_OS_NTOHLL_P(param->value);
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_INST_TIMEOUT);
				break;

			case saAmfCompDelayBetweenInstantiateAttempts_ID:
				break;

			case saAmfCompTerminateTimeout_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].timeout =
				    m_NCS_OS_NTOHLL_P(param->value);
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVND_CKPT_COMP_TERM_TIMEOUT);
				break;

			case saAmfCompCleanupTimeout_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].timeout =
				    m_NCS_OS_NTOHLL_P(param->value);
				break;

			case saAmfCompAmStartTimeout_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].timeout =
				    m_NCS_OS_NTOHLL_P(param->value);
				break;

			case saAmfCompAmStopTimeout_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].timeout =
				    m_NCS_OS_NTOHLL_P(param->value);
				break;

			case saAmfCompTerminateCallbackTimeOut_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->term_cbk_timeout = m_NCS_OS_NTOHLL_P(param->value);
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp,
								 AVND_CKPT_COMP_TERM_CBK_TIMEOUT);
				break;

			case saAmfCompCSISetCallbackTimeout_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->csi_set_cbk_timeout = m_NCS_OS_NTOHLL_P(param->value);
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp,
								 AVND_CKPT_COMP_CSI_SET_CBK_TIMEOUT);
				break;

			case saAmfCompQuiescingCompleteTimeout_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->quies_complete_cbk_timeout = m_NCS_OS_NTOHLL_P(param->value);
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp,
								 AVND_CKPT_COMP_QUIES_CMPLT_CBK_TIMEOUT);
				break;

			case saAmfCompCSIRmvCallbackTimeout_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->csi_rmv_cbk_timeout = m_NCS_OS_NTOHLL_P(param->value);
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp,
								 AVND_CKPT_COMP_CSI_RMV_CBK_TIMEOUT);
				break;

			case saAmfCompProxiedCompInstantiateCallbackTimeout_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->pxied_inst_cbk_timeout = m_NCS_OS_NTOHLL_P(param->value);
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp,
								 AVND_CKPT_COMP_PXIED_INST_CBK_TIMEOUT);
				break;

			case saAmfCompProxiedCompCleanupCallbackTimeout_ID:
				assert(sizeof(SaTimeT) == param->value_len);
				comp->pxied_clean_cbk_timeout = m_NCS_OS_NTOHLL_P(param->value);
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp,
								 AVND_CKPT_COMP_PXIED_CLEAN_CBK_TIMEOUT);
				break;

			case saAmfCompNodeRebootCleanupFail_ID:
				break;

			case saAmfCompRecoveryOnError_ID:
				assert(sizeof(uns32) == param->value_len);
				comp->err_info.def_rec = m_NCS_OS_NTOHL(*(uns32 *)(param->value));
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp,
								 AVND_CKPT_COMP_DEFAULT_RECVR);
				break;

			case saAmfCompNumMaxInstantiate_ID:
				assert(sizeof(uns32) == param->value_len);
				comp->clc_info.inst_retry_max =
				    m_NCS_OS_NTOHL(*(uns32 *)(param->value));
				m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp,
								 AVND_CKPT_COMP_INST_RETRY_MAX);
				break;

			case saAmfCompAMEnable_ID:
				assert(1 == param->value_len);
				comp->is_am_en = (NCS_BOOL)param->value[0];
				comp->clc_info.am_start_retry_cnt = 0;
				rc = avnd_comp_am_oper_req_process(cb, comp);
				break;

			case saAmfCompNumMaxInstantiateWithDelay_ID:
				break;
			case saAmfCompNumMaxAmStartAttempts_ID:
				break;
			case saAmfCompNumMaxAmStopAttempts_ID:
				break;
			case saAmfCompType_ID: {
				SaNameT *value = (SaNameT *)param->value;

				assert(sizeof(SaNameT) == param->value_len);
				assert(comp->pres == SA_AMF_PRESENCE_UNINSTANTIATED);

				avnd_log(NCSFL_SEV_NOTICE, "saAmfCompType %s %u", value->value, value->length);
				rc = avnd_compdb_rec_del(cb, &param->name);
				if (NCSCC_RC_SUCCESS != rc) {
					avnd_log(NCSFL_SEV_ERROR, "avnd_compdb_rec_del FAILED %u", rc);
					goto done;
				}

				rc = avnd_comp_config_get(&param->name);
				if (SA_AIS_OK != rc) {
					rc = NCSCC_RC_FAILURE;
					avnd_log(NCSFL_SEV_ERROR, "avnd_comp_config_get FAILED %u", rc);
					goto done;
				}
				break;
			}
			default:
				break;
			}
		}
		break;

	case AVSV_OBJ_OPR_DEL:
		{
			AVND_COMP *comp = 0;

			/* get the record */
			comp = m_AVND_COMPDB_REC_GET(cb->compdb, param->name);
			if (comp) {
				/* delete the record */
				m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, comp, AVND_CKPT_COMP_CONFIG);
				rc = avnd_compdb_rec_del(cb, &param->name);
			}
		}
		break;

	default:
		assert(0);
	}

	rc = NCSCC_RC_SUCCESS;
done:
	return rc;
}

static void avnd_comptype_delete(amf_comp_type_t *compt)
{
	free(compt->saAmfCtDefCmdEnv);
	free(compt->saAmfCtRelPathInstantiateCmd);
	free(compt->saAmfCtDefInstantiateCmdArgv);
	free(compt->saAmfCtRelPathTerminateCmd);
	free(compt->saAmfCtDefTerminateCmdArgv);
	free(compt->saAmfCtRelPathCleanupCmd);
	free(compt->saAmfCtDefCleanupCmdArgv);
	free(compt->saAmfCtRelPathAmStartCmd);
	free(compt->saAmfCtDefAmStartCmdArgv);
	free(compt->saAmfCtRelPathAmStopCmd);
	free(compt->saAmfCtDefAmStopCmdArgv);
	free(compt);
}

static amf_comp_type_t *avnd_comptype_create(const SaNameT *dn)
{
	SaImmAccessorHandleT accessorHandle;
	amf_comp_type_t *compt;
	int rc = -1;
	const char *str;
	const SaImmAttrValuesT_2 **attributes;

	if ((compt = calloc(1, sizeof(amf_comp_type_t))) == NULL) {
		avnd_log(NCSFL_SEV_ERROR, "calloc FAILED");
		goto done;
	}

	(void)immutil_saImmOmAccessorInitialize(avnd_cb->immOmHandle, &accessorHandle);

	if (immutil_saImmOmAccessorGet_2(accessorHandle, dn, NULL, (SaImmAttrValuesT_2 ***)&attributes) != SA_AIS_OK) {
		avnd_log(NCSFL_SEV_ERROR, "saImmOmAccessorGet_2 FAILED for '%s'", dn->value);
		goto done;
	}

	memcpy(compt->name.value, dn->value, dn->length);
	compt->name.length = dn->length;
	compt->tree_node.key_info = (uns8 *)&(compt->name);

	if (immutil_getAttr("saAmfCtCompCategory", attributes, 0, &compt->saAmfCtCompCategory) != SA_AIS_OK)
		assert(0);

	(void)immutil_getAttr("saAmfCtSwBundle", attributes, 0, &compt->saAmfCtSwBundle);
#if 0
	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefCmdEnv", 0)) != NULL)
		compt->saAmfCtDefCmdEnv = strdup(str);
#endif
	(void)immutil_getAttr("saAmfCtDefClcCliTimeout", attributes, 0, &compt->saAmfCtDefClcCliTimeout);

	(void)immutil_getAttr("saAmfCtDefCallbackTimeout", attributes, 0, &compt->saAmfCtDefCallbackTimeout);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtRelPathInstantiateCmd", 0)) != NULL)
		compt->saAmfCtRelPathInstantiateCmd = strdup(str);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefInstantiateCmdArgv", 0)) != NULL)
		compt->saAmfCtDefInstantiateCmdArgv = strdup(str);

	if (immutil_getAttr("saAmfCtDefInstantiationLevel", attributes, 0, &compt->saAmfCtDefInstantiationLevel) != SA_AIS_OK)
		compt->saAmfCtDefInstantiationLevel = 0;

	if ((str = immutil_getStringAttr(attributes, "saAmfCtRelPathTerminateCmd", 0)) != NULL)
		compt->saAmfCtRelPathTerminateCmd = strdup(str);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefTerminateCmdArgv", 0)) != NULL)
		compt->saAmfCtDefTerminateCmdArgv = strdup(str);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtRelPathCleanupCmd", 0)) != NULL)
		compt->saAmfCtRelPathCleanupCmd = strdup(str);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefCleanupCmdArgv", 0)) != NULL)
		compt->saAmfCtDefCleanupCmdArgv = strdup(str);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtRelPathAmStartCmd", 0)) != NULL)
		compt->saAmfCtRelPathAmStartCmd = strdup(str);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefAmStartCmdArgv", 0)) != NULL)
		compt->saAmfCtDefAmStartCmdArgv = strdup(str);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtRelPathAmStopCmd", 0)) != NULL)
		compt->saAmfCtRelPathAmStopCmd = strdup(str);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefAmStopCmdArgv", 0)) != NULL)
		compt->saAmfCtDefAmStopCmdArgv = strdup(str);

	(void)immutil_getAttr("saAmfCtDefQuiesingCompleteTimeout", attributes, 0,
			      &compt->saAmfCompQuiescingCompleteTimeout);

	if (immutil_getAttr("saAmfCtDefRecoveryOnError", attributes, 0, &compt->saAmfCtDefRecoveryOnError) != SA_AIS_OK)
		assert(0);

	if (immutil_getAttr("saAmfCtDefDisableRestart", attributes, 0, &compt->saAmfCtDefDisableRestart) != SA_AIS_OK)
		compt->saAmfCtDefDisableRestart = SA_FALSE;

	rc = 0;

 done:
	if (rc != 0) {
		avnd_comptype_delete(compt);
		compt = NULL;
	}

	(void)immutil_saImmOmAccessorFinalize(accessorHandle);

	return compt;
}

static NCS_COMP_TYPE_VAL amf_comp_category_to_ncs(SaUint32T saf_comp_category)
{
	NCS_COMP_TYPE_VAL ncs_comp_category = 0;

	/* Check for mandatory attr for SA_AWARE. */
	if (saf_comp_category & SA_AMF_COMP_SA_AWARE) {
		/* It shouldn't match with any of others, we don't care about SA_AMF_COMP_LOCAL as it is optional */
		if ((saf_comp_category & ~SA_AMF_COMP_LOCAL) == SA_AMF_COMP_SA_AWARE) {
			ncs_comp_category = NCS_COMP_TYPE_SA_AWARE;
		}
	} else {
		/* Not SA_AMF_COMP_PROXY, SA_AMF_COMP_CONTAINER, SA_AMF_COMP_CONTAINED and SA_AMF_COMP_SA_AWARE */
		if (saf_comp_category & SA_AMF_COMP_LOCAL) {
			/* It shouldn't match with any of others */
			if ((saf_comp_category & SA_AMF_COMP_PROXIED) ||
			    (!(saf_comp_category & SA_AMF_COMP_PROXIED_NPI))) {
				ncs_comp_category = NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE;
			} else if ((saf_comp_category & SA_AMF_COMP_PROXIED_NPI) ||
				   (!(saf_comp_category & SA_AMF_COMP_PROXIED))) {
				ncs_comp_category = NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE;
			} else if (!((saf_comp_category & SA_AMF_COMP_PROXIED)) ||
				   (!(saf_comp_category & SA_AMF_COMP_PROXIED_NPI))) {
				ncs_comp_category = NCS_COMP_TYPE_NON_SAF;
			}
		} else {
			/* Not SA_AMF_COMP_PROXY, SA_AMF_COMP_CONTAINER, SA_AMF_COMP_CONTAINED, SA_AMF_COMP_SA_AWARE and 
			   SA_AMF_COMP_LOCAL */
			if (saf_comp_category & SA_AMF_COMP_PROXIED_NPI) {
				ncs_comp_category = NCS_COMP_TYPE_EXTERNAL_NON_PRE_INSTANTIABLE;
			} else {
				/* Not SA_AMF_COMP_PROXY, SA_AMF_COMP_CONTAINER, SA_AMF_COMP_CONTAINED, SA_AMF_COMP_SA_AWARE, 
				   SA_AMF_COMP_LOCAL and SA_AMF_COMP_PROXIED_NPI. So only thing left is SA_AMF_COMP_PROXIED */
				/* Whether SA_AMF_COMP_PROXIED is set or not, we don't care, as it is optional */
				ncs_comp_category = NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE;
			}

		}		/* else of if((comp_type->ct_comp_category & SA_AMF_COMP_PROXIED)  */
	}			/* else of if(comp_type->ct_comp_category & SA_AMF_COMP_SA_AWARE)  */

	return ncs_comp_category;
}

static void init_comp_category(AVND_COMP *comp, saAmfCompCategoryT category)
{
	switch (amf_comp_category_to_ncs(category)) {
	case NCS_COMP_TYPE_SA_AWARE:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_SAAWARE);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PREINSTANTIABLE);
		break;

	case NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_SAAWARE);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PREINSTANTIABLE);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
		break;

	case NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_SAAWARE);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
		break;

	case NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PREINSTANTIABLE);
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
		break;

	case NCS_COMP_TYPE_EXTERNAL_NON_PRE_INSTANTIABLE:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
		break;

	case NCS_COMP_TYPE_NON_SAF:
		m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
		break;

	default:
		assert(0);
		break;
	}
}

static AVND_COMP *avnd_comp_create(const SaNameT *comp_name, const SaImmAttrValuesT_2 **attributes, AVND_SU *su)
{
	int rc = -1;
	AVND_COMP *comp;
	AVND_COMP_CLC_CMD_PARAM *cmd;
	char *cmd_argv;
	const char *str;
	amf_comp_type_t *comptype;
	SaBoolT disable_restart;

	if ((comp = calloc(1, sizeof(*comp))) == NULL) {
		avnd_log(NCSFL_SEV_ERROR, "calloc FAILED for '%s'", comp_name->value);
		goto done;
	}

	memcpy(&comp->name, comp_name, sizeof(comp->name));
	comp->name.length = comp_name->length;

	if (immutil_getAttr("saAmfCompType", attributes, 0, &comp->saAmfCompType) != SA_AIS_OK) {
		avnd_log(NCSFL_SEV_ERROR, "Get saAmfCompType FAILED for '%s'", comp_name->value);
		goto done;
	}

	if ((comptype = avnd_comptype_create(&comp->saAmfCompType)) == NULL) {
		avnd_log(NCSFL_SEV_ERROR, "avnd_comptype_create FAILED for '%s'", comp->saAmfCompType.value);
		goto done;
	}

	cmd = &comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1];
	if (comptype->saAmfCtRelPathInstantiateCmd != NULL) {
		strcpy(cmd->cmd, comptype->saAmfCtRelPathInstantiateCmd);
		cmd_argv = cmd->cmd + strlen(cmd->cmd);
		*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */

		if ((str = immutil_getStringAttr(attributes, "saAmfCompInstantiateCmdArgv", 0)) == NULL)
			str = comptype->saAmfCtDefInstantiateCmdArgv;

		if (str != NULL)
			strcpy(cmd_argv, str);

		cmd->len = strlen(cmd->cmd);
	}

	if (immutil_getAttr("saAmfCompInstantiateTimeout", attributes, 0, &cmd->timeout) != SA_AIS_OK) {
		cmd->timeout = comptype->saAmfCtDefClcCliTimeout;
	}

	if (immutil_getAttr("saAmfCompInstantiateLevel", attributes, 0, &comp->inst_level) != SA_AIS_OK)
		comp->inst_level = comptype->saAmfCtDefInstantiationLevel;

	if (immutil_getAttr("saAmfCompNumMaxInstantiateWithoutDelay", attributes,
			    0, &comp->clc_info.inst_retry_max) != SA_AIS_OK)
		comp->clc_info.inst_retry_max = comp_global_attrs.saAmfNumMaxInstantiateWithoutDelay;

#if 0
	//  TODO
	if (immutil_getAttr("saAmfCompNumMaxInstantiateWithDelay", attributes,
			    0, &comp->max_num_inst_delay) != SA_AIS_OK)
		comp->comp_info.max_num_inst = comp_global_attrs.saAmfNumMaxInstantiateWithDelay;

	if (immutil_getAttr("saAmfCompDelayBetweenInstantiateAttempts", attributes,
			    0, &comp->inst_retry_delay) != SA_AIS_OK)
		comp->inst_retry_delay = comp_global_attrs.saAmfDelayBetweenInstantiateAttempts;
#endif

	cmd = &comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1];
	if (comptype->saAmfCtRelPathTerminateCmd != NULL) {
		strcpy(cmd->cmd, comptype->saAmfCtRelPathTerminateCmd);
		cmd_argv = cmd->cmd + strlen(cmd->cmd);
		*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */

		if ((str = immutil_getStringAttr(attributes, "saAmfCompTerminateCmdArgv", 0)) == NULL)
			str = comptype->saAmfCtDefTerminateCmdArgv;

		if (str != NULL)
			strcpy(cmd_argv, str);

		cmd->len = strlen(cmd->cmd);
	}

	if (immutil_getAttr("saAmfCompTerminateTimeout", attributes, 0, &cmd->timeout) != SA_AIS_OK)
		cmd->timeout = comptype->saAmfCtDefCallbackTimeout;

	cmd = &comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1];
	if (comptype->saAmfCtRelPathCleanupCmd != NULL) {
		strcpy(cmd->cmd, comptype->saAmfCtRelPathCleanupCmd);
		cmd_argv = cmd->cmd + strlen(cmd->cmd);
		*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */

		if ((str = immutil_getStringAttr(attributes, "saAmfCompCleanupCmdArgv", 0)) == NULL)
			str = comptype->saAmfCtDefCleanupCmdArgv;

		if (str != NULL)
			strcpy(cmd_argv, str);

		cmd->len = strlen(cmd->cmd);
	}

	if (immutil_getAttr("saAmfCompCleanupTimeout", attributes, 0, &cmd->timeout) != SA_AIS_OK)
		cmd->timeout = comptype->saAmfCtDefCallbackTimeout;

	cmd = &comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1];
	if (comptype->saAmfCtRelPathAmStartCmd != NULL) {
		strcpy(cmd->cmd, comptype->saAmfCtRelPathAmStartCmd);
		cmd_argv = cmd->cmd + strlen(cmd->cmd);
		*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */

		if ((str = immutil_getStringAttr(attributes, "saAmfCompAmStartCmdArgv", 0)) == NULL)
			str = comptype->saAmfCtDefAmStartCmdArgv;

		if (str != NULL)
			strcpy(cmd_argv, str);

		cmd->len = strlen(cmd->cmd);
		comp->is_am_en = TRUE;
	}

	if (immutil_getAttr("saAmfCompAmStartTimeout", attributes, 0, &cmd->timeout) != SA_AIS_OK)
		cmd->timeout = comptype->saAmfCtDefClcCliTimeout;

	if (immutil_getAttr("saAmfCompNumMaxAmStartAttempts", attributes,
			    0, &comp->clc_info.am_start_retry_max) != SA_AIS_OK)
		comp->clc_info.am_start_retry_max = comp_global_attrs.saAmfNumMaxAmStartAttempts;

	cmd = &comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1];
	if (comptype->saAmfCtRelPathAmStopCmd != NULL) {
		strcpy(cmd->cmd, comptype->saAmfCtRelPathAmStopCmd);
		cmd_argv = cmd->cmd + strlen(cmd->cmd);
		*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */

		if ((str = immutil_getStringAttr(attributes, "saAmfCompAmStopCmdArgv", 0)) == NULL)
			str = comptype->saAmfCtDefAmStopCmdArgv;

		if (str != NULL)
			strcpy(cmd_argv, str);

		cmd->len = strlen(cmd->cmd);
	}

	if (immutil_getAttr("saAmfCompAmStopTimeout", attributes, 0, &cmd->timeout) != SA_AIS_OK)
		cmd->timeout = comptype->saAmfCtDefClcCliTimeout;

	if (immutil_getAttr("saAmfCompNumMaxAmStopAttempts", attributes,
			    0, &comp->clc_info.saAmfCompNumMaxAmStopAttempts) != SA_AIS_OK)
		comp->clc_info.saAmfCompNumMaxAmStopAttempts = comp_global_attrs.saAmfNumMaxAmStopAttempts;

	if (immutil_getAttr("saAmfCompCSISetCallbackTimeout", attributes,
			    0, &comp->csi_set_cbk_timeout) != SA_AIS_OK)
		comp->csi_set_cbk_timeout = comptype->saAmfCtDefCallbackTimeout;

	if (immutil_getAttr("saAmfCompCSIRmvCallbackTimeout", attributes,
			    0, &comp->csi_rmv_cbk_timeout) != SA_AIS_OK)
		comp->csi_rmv_cbk_timeout = comptype->saAmfCtDefCallbackTimeout;

	if (immutil_getAttr("saAmfCompQuiescingCompleteTimeout", attributes,
			    0, &comp->quies_complete_cbk_timeout) != SA_AIS_OK)
		comp->quies_complete_cbk_timeout = comptype->saAmfCompQuiescingCompleteTimeout;

	if (immutil_getAttr("saAmfCompRecoveryOnError", attributes, 0, &comp->err_info.def_rec) != SA_AIS_OK)
		comp->err_info.def_rec = comptype->saAmfCtDefRecoveryOnError;

	if (immutil_getAttr("saAmfCompDisableRestart", attributes, 0, &disable_restart) != SA_AIS_OK)
		disable_restart = comptype->saAmfCtDefDisableRestart;

	comp->is_restart_en = (disable_restart == SA_TRUE) ? FALSE : TRUE;

	if (m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(comp))
		m_AVND_COMP_OPER_STATE_SET(comp, SA_AMF_OPERATIONAL_DISABLED);
	else
		m_AVND_COMP_OPER_STATE_SET(comp, SA_AMF_OPERATIONAL_ENABLED);

	init_comp_category(comp, comptype->saAmfCtCompCategory);

	comp->avd_updt_flag = FALSE;

	/* synchronize comp oper state */
	m_AVND_COMP_OPER_STATE_AVD_SYNC(avnd_cb, comp, rc);

//	comp->cap = info->cap;
	comp->node_id = avnd_cb->clmdb.node_info.nodeId;
	comp->pres = SA_AMF_PRESENCE_UNINSTANTIATED;

	/* Initialize the comp-hc list. */
	comp->hc_list.order = NCS_DBLIST_ANY_ORDER;
	comp->hc_list.cmp_cookie = avnd_dblist_hc_rec_cmp;
	comp->hc_list.free_cookie = 0;

	/* Initialize the comp-csi list. */
	comp->csi_list.order = NCS_DBLIST_ASSCEND_ORDER;
	comp->csi_list.cmp_cookie = avsv_dblist_saname_cmp;
	comp->csi_list.free_cookie = 0;

	avnd_pm_list_init(comp);

	/* initialize proxied list */
	avnd_pxied_list_init(comp);

	/* Add to the patricia tree. */
	comp->tree_node.key_info = (uns8 *)&comp->name;
	if(ncs_patricia_tree_add(&avnd_cb->compdb, &comp->tree_node) != NCSCC_RC_SUCCESS)
		avnd_log(NCSFL_SEV_ERROR, "ncs_patricia_tree_add FAILED for '%s'", comp_name->value);

	/* Add to the comp-list (maintained by su) */
	m_AVND_SUDB_REC_COMP_ADD(*su, *comp, rc);

	comp->su = su;

	if (TRUE == su->su_is_external) {
		m_AVND_COMP_TYPE_SET_EXT_CLUSTER(comp);
	} else
		m_AVND_COMP_TYPE_SET_LOCAL_NODE(comp);

	m_AVND_SEND_CKPT_UPDT_ASYNC_ADD(avnd_cb, comp, AVND_CKPT_COMP_CONFIG);

	/* determine if su is pre-instantiable */
	if (m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(comp)) {
		m_AVND_SU_PREINSTANTIABLE_SET(comp->su);
		m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(avnd_cb, comp->su, AVND_CKPT_SU_FLAG_CHANGE);
//			m_AVND_SU_OPER_STATE_SET_AND_SEND_NTF(avnd_cb, comp->su, SA_AMF_OPERATIONAL_DISABLED);
		m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(avnd_cb, comp->su, AVND_CKPT_SU_OPER_STATE);
	}

	/* determine if su is restart capable */
	if (m_AVND_COMP_IS_RESTART_DIS(comp)) {
		m_AVND_SU_RESTART_DIS_SET(comp->su);
		m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(avnd_cb, comp->su, AVND_CKPT_SU_FLAG_CHANGE);
	}

	rc = 0;
done:
	if (rc != 0) {
		free(comp);
		comp = NULL;
	}
	avnd_comptype_delete(comptype);
	return comp;
}

/**
 * Get configuration for all AMF Comp objects related to the 
 * specified SU from IMM and create internal objects. 
 * 
 * @param su 
 * 
 * @return SaAisErrorT 
 */
SaAisErrorT avnd_comp_config_get_su(AVND_SU *su)
{
	SaAisErrorT rc, error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT comp_name;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfComp";
	AVND_COMP *comp;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if ((rc = immutil_saImmOmSearchInitialize_2(avnd_cb->immOmHandle, &su->name,
		SA_IMM_SUBTREE, SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR,
		&searchParam, compConfigAttributes, &searchHandle)) != SA_AIS_OK) {

	    avnd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize_2 failed: %u", rc);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &comp_name,
		(SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		avnd_log(NCSFL_SEV_NOTICE, "'%s'", comp_name.value);

		if ((comp = avnd_comp_create(&comp_name, attributes, su)) == NULL)
			goto done2;

		avnd_hc_config_get(comp);
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:
	return error;
}

/**
 * Get configuration for a DN specified AMF Comp object from IMM
 * and create an internal object. 
 * 
 * @param dn 
 * 
 * @return SaAisErrorT 
 */

static SaAisErrorT avnd_comp_config_get(const SaNameT *dn)
{
	SaAisErrorT rc, error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmAccessorHandleT accessorHandle;
	const SaImmAttrValuesT_2 **attributes;
	AVND_COMP *comp;
	AVND_SU *su;
	SaNameT su_name;
	char *p;

	immutil_saImmOmAccessorInitialize(avnd_cb->immOmHandle, &accessorHandle);

	rc = immutil_saImmOmAccessorGet_2(accessorHandle, dn,
		compConfigAttributes, (SaImmAttrValuesT_2 ***)&attributes);

	if (rc != SA_AIS_OK) {
		avnd_log(NCSFL_SEV_ERROR, "saImmOmAccessorGet_2 FAILED %u", rc);
		goto done;
	}

	p = strstr((char*)dn->value, "safSu");
	memset(&su_name, 0, sizeof(su_name));
	strcpy((char*)su_name.value, p);
	su_name.length = strlen(p);
	su = m_AVND_SUDB_REC_GET(avnd_cb->sudb, su_name);
	assert(su);

	if ((comp = avnd_comp_create(dn, attributes, su)) == NULL)
		goto done;

	avnd_hc_config_get(comp);

	error = SA_AIS_OK;

done:
	immutil_saImmOmAccessorFinalize(accessorHandle);
	return error;
}

