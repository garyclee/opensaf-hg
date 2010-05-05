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

  MODULE NAME:       ncs_vda.c   

  DESCRIPTION:       LEAP MDS destination utility APIs to create and manage 
                     MDS destinations.

******************************************************************************
*/
#include <ncsgl_defs.h>
#include "ncssysf_def.h"
#include "ncs_sprr_papi.h"
#include "mds_papi.h"
#include "mds_adm.h"
#include "mds_core.h"
#include "mda_pvt_api.h"
#include "mda_mem.h"
#include "ncs_mda_papi.h"

static MDS_CLIENT_MSG_FORMAT_VER
 gl_vda_wrt_vds_msg_fmt_array[VDA_WRT_VDS_SUBPART_VER_RANGE] = {
	1			/*msg format version for VDA subpart version 1 */
};

#define m_NCSVDA_TRACE_ARG1(X)

/***************************************************************************\
                         PRIVATE DATA STRUCTURES
\***************************************************************************/

typedef struct vda_pvt_info {
	/* Used by VDA module to send MDS messages */
	MDS_HDL mds_adest_pwe1_hdl;

	/* Information on the VDS's MDS-address. Required when a VDA needs
	   to communicate with a VDS. */
	NCS_BOOL vds_primary_up;
	MDS_DEST vds_vdest;
	NCS_LOCK vds_sync_lock;
	NCS_BOOL vds_sync_awaited;
	NCS_SEL_OBJ vds_sync_sel;

} VDA_PVT_INFO;

static VDA_PVT_INFO gl_vda_info;	/* For internal use within MDA only */
static NCS_BOOL vda_vdest_create = FALSE;

/***************************************************************************\
                         PRIVATE FUNCTION PROTOTYPES
\***************************************************************************/
static uns32 vda_create(NCS_LIB_REQ_INFO *req);
static uns32 vda_destroy(NCS_LIB_REQ_INFO *req);
static uns32 vda_instantiate(NCS_LIB_REQ_INFO *req);
static uns32 vda_uninstantiate(NCS_LIB_REQ_INFO *req);

static uns32 vda_mds_callback(struct ncsmds_callback_info *info);
static uns32 vda_destroy_vdest_locally(uns32 vdest_handle);

static void vda_sync_with_vds(void);

/***************************************************************************\
                         PUBLIC VDA FUNCTIONS
\***************************************************************************/
uns32 vda_lib_req(NCS_LIB_REQ_INFO *req)
{
	switch (req->i_op) {
	case NCS_LIB_REQ_CREATE:
		m_NCSVDA_TRACE_ARG1("VDA:LIB_CREATE\n");
		return vda_create(req);

	case NCS_LIB_REQ_INSTANTIATE:
		m_NCSVDA_TRACE_ARG1("VDA:LIB_INSTANTIATE\n");
		return vda_instantiate(req);

	case NCS_LIB_REQ_UNINSTANTIATE:
		m_NCSVDA_TRACE_ARG1("VDA:LIB_UNINSTANTIATE\n");
		return vda_uninstantiate(req);

	case NCS_LIB_REQ_DESTROY:
		m_NCSVDA_TRACE_ARG1("VDA:LIB_DESTROY\n");
		return vda_destroy(req);

	default:
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
}

uns32 ncsvda_api(NCSVDA_INFO *vda_info)
{
	NCS_SPIR_REQ_INFO spir_req;
	NCSMDS_INFO svc_info;
	uns32 policy;
	SaAmfCSIAttributeT attr[1];

	switch (vda_info->req) {
	case NCSVDA_VDEST_CREATE:

		/* Get the PWE-HDL */
		memset(&spir_req, 0, sizeof(spir_req));
		spir_req.type = NCS_SPIR_REQ_LOOKUP_CREATE_INST;
		spir_req.i_environment_id = 1;
		spir_req.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
		policy = vda_info->info.vdest_create.i_policy;
		spir_req.info.lookup_create_inst.i_inst_attrs.number = 1;
		spir_req.info.lookup_create_inst.i_inst_attrs.attr = attr;
		spir_req.info.lookup_create_inst.i_inst_attrs.attr[0].attrName = (SaUint8T *)m_SPRR_VDEST_POLICY_ATTR_NAME;	/* Name-value name */
		/* typecasting to avoid warning in 64bit compilation */
		spir_req.info.lookup_create_inst.i_inst_attrs.attr[0].attrValue = (SaUint8T *)&policy;	/* Must be an uns32 pointer */

		if (vda_info->info.vdest_create.i_create_type == NCSVDA_VDEST_CREATE_SPECIFIC) {
			if ((vda_info->info.vdest_create.info.specified.i_vdest == 0)
			    || (vda_info->info.vdest_create.info.specified.i_vdest > NCSMDS_MAX_VDEST)) {
				return vda_info->o_result = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
			}
			/* End Fix */

			/* Check if the VDEST is in allowed range */
			if (m_MDS_GET_VDEST_ID_FROM_MDS_DEST(vda_info->info.vdest_create.info.specified.i_vdest)
			    > NCSVDA_UNNAMED_MAX) {
				return vda_info->o_result = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
			}

			m_MDS_FIXED_VDEST_TO_INST_NAME(m_MDS_GET_VDEST_ID_FROM_MDS_DEST
						       (vda_info->info.vdest_create.info.specified.i_vdest),
						       &spir_req.i_instance_name);
		} else {
			return NCSCC_RC_FAILURE;

		}

		if (ncs_spir_api(&spir_req) != NCSCC_RC_SUCCESS) {
			return vda_info->o_result = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
		vda_info->info.vdest_create.o_mds_pwe1_hdl = (MDS_HDL)spir_req.info.lookup_create_inst.o_handle;

		/* Get the vdest and vdest-hdl. We can use the SPRR to get 
		   the vdest-hdl but not the VDEST. Hence, we instead use the
		   MDS_QUERY_PWE_INFO for "one-shot-two-birds" deal.

		   There is no other alternative here, other than a MDS_INSTALL
		   and MDS_UNINSTALL merely to get the VDEST-IDENTIFIER. */

		memset(&svc_info, 0, sizeof(svc_info));
		svc_info.i_mds_hdl = vda_info->info.vdest_create.o_mds_pwe1_hdl;
		svc_info.i_op = MDS_QUERY_PWE;
		svc_info.i_svc_id = NCSMDS_SVC_ID_VDA;	/* Doesn't matter */
		if (ncsmds_api(&svc_info) != NCSCC_RC_SUCCESS) {
			return vda_info->o_result = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}

		/* VDEST to be created is specific */
		vda_info->info.vdest_create.o_mds_vdest_hdl =
		    (MDS_HDL)vda_info->info.vdest_create.info.specified.i_vdest;

		return vda_info->o_result = NCSCC_RC_SUCCESS;

	case NCSVDA_VDEST_CHG_ROLE:
		return vda_info->o_result = vda_chg_role_vdest(&vda_info->info.vdest_chg_role.i_vdest,
							       vda_info->info.vdest_chg_role.i_new_role);

	case NCSVDA_VDEST_LOOKUP:
		/* Need to contact VDS to allocate VDEST value. */

		/* STEP : Send a message to NCSMDS_SVC_ID_VDS  to lookup VDEST */
		if (gl_vda_info.vds_primary_up == FALSE) {
			/* Primary VDS is not yet up. So quit. We should never reach
			 * here unless genesis code missequences startup events.
			 */
			vda_sync_with_vds();

			/* Check again if VDS is up */
			if (gl_vda_info.vds_primary_up == FALSE)
				/* Primary VDS is not yet up. So quit. We should never reach
				 * here unless genesis code missequences startup events.
				 */
				return vda_info->o_result = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}

		memset(&svc_info, 0, sizeof(svc_info));
		svc_info.i_mds_hdl = gl_vda_info.mds_adest_pwe1_hdl;
		svc_info.i_op = MDS_SEND;
		svc_info.i_svc_id = NCSMDS_SVC_ID_VDA;	/* My i.e. source svc-id */
		svc_info.info.svc_send.i_msg = (NCSCONTEXT)(vda_info);
		svc_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_LOW;
		svc_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SNDRSP;
		svc_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_VDS;
		svc_info.info.svc_send.info.sndrsp.i_time_to_wait = 1000;	/* 3s */
		svc_info.info.svc_send.info.sndrsp.i_to_dest = gl_vda_info.vds_vdest;
		if ((ncsmds_api(&svc_info) != NCSCC_RC_SUCCESS) || (vda_info->o_result != NCSCC_RC_SUCCESS))
			return vda_info->o_result = NCSCC_RC_FAILURE;

		return vda_info->o_result = NCSCC_RC_SUCCESS;

	case NCSVDA_VDEST_DESTROY:
		memset(&spir_req, 0, sizeof(spir_req));
		spir_req.i_environment_id = 1;
		/* Release the handle to that PWE */
		if (m_MDS_GET_VDEST_ID_FROM_MDS_DEST(vda_info->info.vdest_destroy.i_vdest)
				> NCSVDA_UNNAMED_MAX) {
			return vda_info->o_result = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}

		m_MDS_FIXED_VDEST_TO_INST_NAME(m_MDS_GET_VDEST_ID_FROM_MDS_DEST
				(vda_info->info.vdest_destroy.i_vdest),
				&spir_req.i_instance_name);
		spir_req.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
		spir_req.type = NCS_SPIR_REQ_REL_INST;
		spir_req.info.rel_inst = 0;	/* Dummy unused value */
		if (ncs_spir_api(&spir_req) != NCSCC_RC_SUCCESS) {
			return vda_info->o_result = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
		return vda_info->o_result = NCSCC_RC_SUCCESS;

	case NCSVDA_PWE_CREATE:

		/* First create pwe on give VDEST with LOOKUP_CREATE query */
		memset(&spir_req, 0, sizeof(spir_req));
		spir_req.type = NCS_SPIR_REQ_LOOKUP_CREATE_INST;
		spir_req.i_environment_id = (PW_ENV_ID)vda_info->info.pwe_create.i_pwe_id;
		spir_req.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
		m_MDS_FIXED_VDEST_TO_INST_NAME(m_MDS_GET_VDEST_ID_FROM_MDS_DEST
					       (vda_info->info.pwe_create.i_mds_vdest_hdl), &spir_req.i_instance_name);
		spir_req.info.lookup_create_inst.i_inst_attrs.number = 0;
		spir_req.info.lookup_create_inst.i_inst_attrs.attr = attr;

		if (ncs_spir_api(&spir_req) != NCSCC_RC_SUCCESS) {
			return vda_info->o_result = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
		vda_info->info.pwe_create.o_mds_pwe_hdl = spir_req.info.lookup_create_inst.o_handle;

		return vda_info->o_result = NCSCC_RC_SUCCESS;

	case NCSVDA_PWE_DESTROY:
		/* NOTE: The PWE-ID from PWE-handle is done a little
		   bit differently from that in NCSVDA_PWE_DESTROY
		 */
		memset(&spir_req, 0, sizeof(spir_req));
		spir_req.i_environment_id = m_MDS_GET_PWE_ID_FROM_PWE_HDL(vda_info->info.pwe_destroy.i_mds_pwe_hdl);
		/* Release the handle to that PWE */

		m_MDS_FIXED_VDEST_TO_INST_NAME(m_MDS_GET_VDEST_ID_FROM_PWE_HDL
				(vda_info->info.pwe_destroy.i_mds_pwe_hdl), &spir_req.i_instance_name);
		spir_req.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
		spir_req.type = NCS_SPIR_REQ_REL_INST;
		spir_req.info.rel_inst = 0;	/* Dummy unused value */
		if (ncs_spir_api(&spir_req) != NCSCC_RC_SUCCESS) {
			return vda_info->o_result = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
		return vda_info->o_result = NCSCC_RC_SUCCESS;

	default:
		return vda_info->o_result = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
}

void vda_sync_with_vds()
{

	uns32 timeout = 3000;
	NCSMDS_INFO svc_info;
	MDS_SVC_ID subscribe_list[1] = { NCSMDS_SVC_ID_VDS };

	/* Take VDS-sync-lock */
	m_NCS_LOCK(&gl_vda_info.vds_sync_lock, NCS_LOCK_WRITE);

	if (vda_vdest_create == FALSE) {
		/* STEP : Install on ADEST with MDS as service NCSMDS_SVC_ID_VDA. */
		svc_info.i_mds_hdl = gl_vda_info.mds_adest_pwe1_hdl;
		svc_info.i_svc_id = NCSMDS_SVC_ID_VDA;
		svc_info.i_op = MDS_INSTALL;
		svc_info.info.svc_install.i_mds_q_ownership = FALSE;
		svc_info.info.svc_install.i_svc_cb = vda_mds_callback;
		svc_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)(long)&gl_vda_info;
		svc_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
		svc_info.info.svc_install.i_mds_svc_pvt_ver = VDA_SVC_PVT_VER;
		m_NCS_DBG_PRINTF("VDA MDS INSTALL service-id:%d VDA_SVC_PVT_VER: %d \n", NCSMDS_SVC_ID_VDA,
				 VDA_SVC_PVT_VER);
		/* Invoke MDS api */
		if (ncsmds_api(&svc_info) != NCSCC_RC_SUCCESS) {
			m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
			m_NCS_UNLOCK(&gl_vda_info.vds_sync_lock, NCS_LOCK_WRITE);
			return;
		}

		/* STEP : Subscribe to VDS up/down events */
		svc_info.i_op = MDS_SUBSCRIBE;
		svc_info.info.svc_subscribe.i_num_svcs = 1;
		svc_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
		svc_info.info.svc_subscribe.i_svc_ids = subscribe_list;
		/* Invoke MDS api */

		if (ncsmds_api(&svc_info) != NCSCC_RC_SUCCESS) {
			/* Uninstall? But then reaching here itself is fatal */

			m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
			m_NCS_UNLOCK(&gl_vda_info.vds_sync_lock, NCS_LOCK_WRITE);
			return;
		}

		vda_vdest_create = TRUE;
	}

	/* The following condition ensures, that whenever VDS is
	   detected UP just before we make "sync_awaited" flag = TRUE.
	   Not putting this indication, may cause us to unnecessary
	   block on the sync_obj until timeout.
	 */
	if (gl_vda_info.vds_primary_up) {
		/* Release VDS-sync-lock */
		m_NCS_UNLOCK(&gl_vda_info.vds_sync_lock, NCS_LOCK_WRITE);
		return;
	}

	gl_vda_info.vds_sync_awaited = TRUE;
	m_NCS_SEL_OBJ_CREATE(&gl_vda_info.vds_sync_sel);
	/* Release VDS-sync-lock */
	m_NCS_UNLOCK(&gl_vda_info.vds_sync_lock, NCS_LOCK_WRITE);

	/* STEP : Await indication from MDS saying MDS is up */
	m_NCS_SEL_OBJ_POLL_SINGLE_OBJ(gl_vda_info.vds_sync_sel, &timeout);
	/* Ignore return value of m_NCS_SEL_OBJ_SELECT   */

	/* STEP : Destroy the sync-object */
	/* Take VDS-sync-lock */
	m_NCS_LOCK(&gl_vda_info.vds_sync_lock, NCS_LOCK_WRITE);

	gl_vda_info.vds_sync_awaited = FALSE;
	m_NCS_SEL_OBJ_DESTROY(gl_vda_info.vds_sync_sel);

	/* Release VDS-sync-lock */
	m_NCS_UNLOCK(&gl_vda_info.vds_sync_lock, NCS_LOCK_WRITE);

	return;
}

/***************************************************************************\
                        PRIVATE ADA FUNCTIONS
\***************************************************************************/
static uns32 vda_create(NCS_LIB_REQ_INFO *req)
{
	NCS_SPLR_REQ_INFO splr_req;
	NCS_SPIR_REQ_INFO spir_req;
	memset(&splr_req, 0, sizeof(splr_req));

	m_NCS_LOCK_INIT(&gl_vda_info.vds_sync_lock);
	/* STEP : Register VDA as a service provider */
	splr_req.i_sp_abstract_name = m_VDA_SP_ABST_NAME;
	splr_req.type = NCS_SPLR_REQ_REG;
	splr_req.info.reg.instantiation_api = vda_lib_req;
	splr_req.info.reg.instantiation_flags = NCS_SPLR_INSTANTIATION_PER_INST_NAME;
	splr_req.info.reg.user_se_api = (NCSCONTEXT)ncsvda_api;

	if (ncs_splr_api(&splr_req) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}

	/* STEP : Get the MDS-handle to the ADEST */
	memset(&spir_req, 0, sizeof(spir_req));
	spir_req.type = NCS_SPIR_REQ_LOOKUP_CREATE_INST;
	spir_req.i_environment_id = 1;
	spir_req.i_instance_name = m_MDS_SPIR_ADEST_NAME;
	spir_req.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
	spir_req.info.lookup_create_inst.i_inst_attrs.number = 0;

	if (ncs_spir_api(&spir_req) != NCSCC_RC_SUCCESS) {
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
	gl_vda_info.mds_adest_pwe1_hdl = (MDS_HDL)spir_req.info.lookup_create_inst.o_handle;

	return NCSCC_RC_SUCCESS;
}

static uns32 vda_destroy(NCS_LIB_REQ_INFO *req)
{
	NCS_SPLR_REQ_INFO splr_req;
	NCS_SPIR_REQ_INFO spir_req;

	NCSMDS_INFO svc_info;

	m_NCS_LOCK_DESTROY(&gl_vda_info.vds_sync_lock);

	if (vda_vdest_create == TRUE) {
		/* STEP : Uninstall from ADEST with MDS as service NCSMDS_SVC_ID_VDA. */
		memset(&svc_info, 0, sizeof(svc_info));
		svc_info.i_mds_hdl = gl_vda_info.mds_adest_pwe1_hdl;
		svc_info.i_svc_id = NCSMDS_SVC_ID_VDA;
		svc_info.i_op = MDS_UNINSTALL;
		svc_info.info.svc_uninstall.i_msg_free_cb = NULL;	/* NULL for callback-model services */
		/* Invoke MDS api */
		if (ncsmds_api(&svc_info) != NCSCC_RC_SUCCESS)
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	}
	/* STEP : Release the use of ADEST MDS handle */
	memset(&spir_req, 0, sizeof(spir_req));
	spir_req.type = NCS_SPIR_REQ_REL_INST;
	spir_req.i_environment_id = 1;
	spir_req.i_instance_name = m_MDS_SPIR_ADEST_NAME;
	spir_req.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
	spir_req.info.rel_inst = 0;	/* dummy value */
	if (ncs_spir_api(&spir_req) != NCSCC_RC_SUCCESS) {
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
	gl_vda_info.mds_adest_pwe1_hdl = 0;

	/* STEP : Deregister VDA as a service provider */
	memset(&splr_req, 0, sizeof(splr_req));
	splr_req.i_sp_abstract_name = m_VDA_SP_ABST_NAME;
	splr_req.type = NCS_SPLR_REQ_DEREG;
	splr_req.info.dereg.dummy = 0;
	if (ncs_splr_api(&splr_req) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}

	vda_vdest_create = FALSE;

	return NCSCC_RC_SUCCESS;
}

static uns32 vda_instantiate(NCS_LIB_REQ_INFO *req)
{
	uns32 vdest_id;
	NCS_BOOL is_named_vdest;
	MDS_DEST new_vdest;
	MDS_HDL new_vdest_hdl;
	uns32 policy;
	uns32 attr_num;

	memset(&new_vdest, 0, sizeof(new_vdest));

	/* STEP : Check for "reserved" instance names */
	switch (mda_get_inst_name_type(&req->info.inst.i_inst_name)) {
	case MDA_INST_NAME_TYPE_UNNAMED_VDEST:
		is_named_vdest = FALSE;
		break;

	case MDA_INST_NAME_TYPE_NAMED_VDEST:
		is_named_vdest = TRUE;
		break;

	default:
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Set VDEST Policy to default */
	policy = NCS_VDEST_TYPE_DEFAULT;
	/* Override the default, if specific attribute for policy is provided */
	for (attr_num = 0; attr_num < req->info.inst.i_inst_attrs.number; attr_num++) {
		if (0 ==
		    memcmp(req->info.inst.i_inst_attrs.attr[attr_num].attrName, m_SPRR_VDEST_POLICY_ATTR_NAME,
			   strlen(m_SPRR_VDEST_POLICY_ATTR_NAME))) {
			policy = *(uns32 *)(req->info.inst.i_inst_attrs.attr[attr_num].attrValue);
		}
	}

	if (is_named_vdest == FALSE) {
		/* A fixed VDEST has been specified, Find it */
		mds_inst_name_to_fixed_vdest(&req->info.inst.i_inst_name, &vdest_id);

		/* Check if the VDEST is in allowed range */
		if (vdest_id > NCSVDA_UNNAMED_MAX) {
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}

		/* Specified VDEST has to be created. Need not go to a
		 ** VDS for this purpose. 
		 */
		m_NCSVDA_SET_VDEST(new_vdest, vdest_id);
	} else {
		return NCSCC_RC_FAILURE;
	}

	/* STEP : Create VDEST locally. */
	if (vda_create_vdest_locally(policy, &new_vdest, &new_vdest_hdl)
	    != NCSCC_RC_SUCCESS) {
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
	req->info.inst.o_inst_hdl = (MDS_HDL)new_vdest_hdl;
	req->info.inst.o_arg = NULL;

	return NCSCC_RC_SUCCESS;
}

static uns32 vda_uninstantiate(NCS_LIB_REQ_INFO *req)
{
	MDS_DEST gone_vdest;
	NCS_BOOL is_named_vdest;

	memset(&gone_vdest, 0, sizeof(gone_vdest));

	/* STEP : Check for "reserved" instance names */
	switch (mda_get_inst_name_type(&req->info.uninst.i_inst_name)) {
	case MDA_INST_NAME_TYPE_UNNAMED_VDEST:
		is_named_vdest = FALSE;
		break;

	case MDA_INST_NAME_TYPE_NAMED_VDEST:
		is_named_vdest = TRUE;
		break;

	default:
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (is_named_vdest) {
		return NCSCC_RC_FAILURE;
	}

	/* STEP : Create VDEST locally. */
	if (vda_destroy_vdest_locally(req->info.uninst.i_inst_hdl) != NCSCC_RC_SUCCESS) {
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
	return NCSCC_RC_FAILURE;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_create_vdest_locally (public)
\*-------------------------------------------------------------------------*/
uns32 vda_create_vdest_locally(uns32 i_pol, MDS_DEST *i_vdest, MDS_HDL *o_mds_vdest_hdl)
{
	NCSMDS_ADMOP_INFO admop_info;

	memset(&admop_info, 0, sizeof(admop_info));

	/* STEP : We first check if the VDEST has already been created locally */
	admop_info.i_op = MDS_ADMOP_VDEST_QUERY;
	admop_info.info.vdest_query.i_local_vdest = *i_vdest;
	if (ncsmds_adm_api(&admop_info) == NCSCC_RC_SUCCESS) {
		/* VDEST already created */
		MDA_TRACE1_ARG1("VDEST already created locally.\n");
		*o_mds_vdest_hdl = admop_info.info.vdest_query.o_local_vdest_hdl;
		return NCSCC_RC_SUCCESS;
	}

	/* STEP : We need to created the VDEST locally, because it is not already 
	   created */
	admop_info.i_op = MDS_ADMOP_VDEST_CREATE;
	admop_info.info.vdest_create.i_vdest = *i_vdest;
	admop_info.info.vdest_create.i_policy = i_pol;
	/* Invoke MDS api */
	if (ncsmds_adm_api(&admop_info) != NCSCC_RC_SUCCESS)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	*o_mds_vdest_hdl = admop_info.info.vdest_create.o_mds_vdest_hdl;
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_chg_role_vdest  (public)
\*-------------------------------------------------------------------------*/
uns32 vda_chg_role_vdest(MDS_DEST *i_vdest, V_DEST_RL i_new_role)
{
	NCSMDS_ADMOP_INFO admop_info;

	memset(&admop_info, 0, sizeof(admop_info));

	admop_info.i_op = MDS_ADMOP_VDEST_CONFIG;
	admop_info.info.vdest_config.i_new_role = i_new_role;
	admop_info.info.vdest_config.i_vdest = *i_vdest;

	/* Invoke MDS api */
	return ncsmds_adm_api(&admop_info);
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_destroy_vdest_locally (private)
\*-------------------------------------------------------------------------*/
static uns32 vda_destroy_vdest_locally(uns32 vdest_handle)
{
	NCSMDS_ADMOP_INFO admop_info;

	memset(&admop_info, 0, sizeof(admop_info));

	admop_info.i_op = MDS_ADMOP_VDEST_DESTROY;
	admop_info.info.vdest_destroy.i_vdest_hdl = (MDS_HDL)vdest_handle;
	/* Invoke MDS api */
	return ncsmds_adm_api(&admop_info);
}

static uns32 vda_mds_cb_cpy(struct ncsmds_callback_info *info);
static uns32 vda_mds_cb_enc(struct ncsmds_callback_info *info);
static uns32 vda_mds_cb_dec(struct ncsmds_callback_info *info);
static uns32 vda_mds_cb_enc_flat(struct ncsmds_callback_info *info);
static uns32 vda_mds_cb_dec_flat(struct ncsmds_callback_info *info);
static uns32 vda_mds_cb_rcv(struct ncsmds_callback_info *info);
static uns32 vda_mds_cb_svc_event(struct ncsmds_callback_info *info);
static uns32 dummy(struct ncsmds_callback_info *info);
/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_mds_callback (private)
\*-------------------------------------------------------------------------*/
static uns32 vda_mds_callback(struct ncsmds_callback_info *info)
{
	static NCSMDS_CALLBACK_API cb_set[MDS_CALLBACK_SVC_MAX] = {
		vda_mds_cb_cpy,	/* MDS_CALLBACK_COPY      */
		vda_mds_cb_enc,	/* MDS_CALLBACK_ENC       */
		vda_mds_cb_dec,	/* MDS_CALLBACK_DEC       */
		vda_mds_cb_enc_flat,	/* MDS_CALLBACK_ENC_FLAT  */
		vda_mds_cb_dec_flat,	/* MDS_CALLBACK_DEC_FLAT  */
		vda_mds_cb_rcv,	/* MDS_CALLBACK_RECEIVE   */
		vda_mds_cb_svc_event,	/* MDS_CALLBACK_SVC_EVENT */
		dummy,		/* MDS_CALLBACK_SYS_EVENT */
		dummy,		/* MDS_CALLBACK_QUIESCED_ACK */
		dummy,		/* MDS_CALLBACK_DIRECT_RECEIVE */
	};

	return (*cb_set[info->i_op]) (info);
}

static uns32 dummy(struct ncsmds_callback_info *info)
{
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_mds_cb_cpy (private)
\*-------------------------------------------------------------------------*/
static uns32 vda_mds_cb_cpy(struct ncsmds_callback_info *info)
{
	NCSVDA_INFO *vda_info;
	MDS_CLIENT_MSG_FORMAT_VER msg_fmt_version;

	/* Need NOT allocate a new copy. VDA always uses blocking MDS calls, 
	 * The blocking call made by VDA does not return until the 
	 * VDS receive callback is called (even in the case of a timeout). 
	 * Thus, "info->info.cpy.imsg" is guaranteed to be valid till
	 * the VDS receieve-callback returns. 
	 */
	MDA_TRACE1_ARG1("vda_mds_cb_cpy:\n");

	msg_fmt_version = m_NCS_VDA_ENC_MSG_FMT_GET(info->info.cpy.i_rem_svc_pvt_ver,
						    VDA_WRT_VDS_SUBPART_VER_MIN,
						    VDA_WRT_VDS_SUBPART_VER_MAX, gl_vda_wrt_vds_msg_fmt_array);
	if (0 == msg_fmt_version) {
		MDA_TRACE1_ARG2("Msg To Service-id:%d Dropped :", info->info.cpy.i_to_svc_id);
		MDA_TRACE1_ARG2("Remote Svc Sub Sart Version:%d \n", info->info.cpy.i_rem_svc_pvt_ver);
		return NCSCC_RC_FAILURE;
	}
	info->info.cpy.o_msg_fmt_ver = msg_fmt_version;

	vda_info = m_MMGR_ALLOC_NCSVDA_INFO;
	if (vda_info == NULL)
		return NCSCC_RC_FAILURE;

	*vda_info = *(NCSVDA_INFO *)info->info.cpy.i_msg;
	info->info.cpy.o_cpy = vda_info;
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_mds_cb_enc (private)
\*-------------------------------------------------------------------------*/
static uns32 vda_mds_cb_enc(struct ncsmds_callback_info *info)
{
	NCSVDA_INFO *vda_info;
	NCS_UBAID *uba;
	MDS_CLIENT_MSG_FORMAT_VER msg_fmt_version;

	MDA_TRACE1_ARG1("vda_mds_cb_enc:\n");

	vda_info = info->info.enc.i_msg;
	uba = info->info.enc.io_uba;

	msg_fmt_version = m_NCS_VDA_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
						    VDA_WRT_VDS_SUBPART_VER_MIN,
						    VDA_WRT_VDS_SUBPART_VER_MAX, gl_vda_wrt_vds_msg_fmt_array);
	if (0 == msg_fmt_version) {
		MDA_TRACE1_ARG2("Msg To Service-id:%d Dropped :", info->info.enc.i_to_svc_id);
		MDA_TRACE1_ARG2("Remote Svc Sub Sart Version:%d \n", info->info.enc.i_rem_svc_pvt_ver);
		return NCSCC_RC_FAILURE;
	}
	info->info.enc.o_msg_fmt_ver = msg_fmt_version;

	vda_util_enc_8bit(uba, (uns8)vda_info->req);

	switch (vda_info->req) {
	case NCSVDA_VDEST_CREATE:
		/* ENCODE: An 8-bit flag */
		vda_util_enc_8bit(uba, (uns8)vda_info->info.vdest_create.i_persistent);

		/* ENCODE: An 8-bit enum value */
		vda_util_enc_8bit(uba, (uns8)vda_info->info.vdest_create.i_policy);

		break;

	case NCSVDA_VDEST_LOOKUP:
		/* ENCODE: String name */
		vda_util_enc_vdest_name(uba, &vda_info->info.vdest_lookup.i_name);
		break;

	case NCSVDA_VDEST_DESTROY:
		/* ENCODE: String name */
		vda_util_enc_vdest_name(uba, &vda_info->info.vdest_destroy.i_name);
		break;

	default:
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_mds_cb_dec (private)
\*-------------------------------------------------------------------------*/
static uns32 vda_mds_cb_dec(struct ncsmds_callback_info *info)
{
	NCSVDA_INFO *vda_info;
	NCS_UBAID *uba = info->info.dec.io_uba;

	MDA_TRACE1_ARG1("vda_mds_cb_dec:\n");
	/* The is the VDS's response to a NCSVDA_VDEST_CREATE request */
	if (info->info.dec.o_msg == NULL)
		/* Should never reach here as VDA uses synchronous callbacks */
		return NCSCC_RC_FAILURE;

	if (0 == m_NCS_VDA_MSG_FORMAT_IS_VALID(info->info.dec.i_msg_fmt_ver,
					       VDA_WRT_VDS_SUBPART_VER_MIN,
					       VDA_WRT_VDS_SUBPART_VER_MAX, gl_vda_wrt_vds_msg_fmt_array)) {
		MDA_TRACE1_ARG2("Msg From service-id: %d dropped :", info->info.dec.i_fr_svc_id);
		MDA_TRACE1_ARG2("Msg Format Version: %d  \n", info->info.dec.i_msg_fmt_ver);
		return NCSCC_RC_FAILURE;
	}

	vda_info = info->info.dec.o_msg;

	vda_info->req = vda_util_dec_8bit(uba);
	vda_info->o_result = vda_util_dec_8bit(uba);

	switch (vda_info->req) {
	case NCSVDA_VDEST_CREATE:
		/* DECODE: vdest */
		/* Not doing anything as VDS is removed */
		break;

	case NCSVDA_VDEST_LOOKUP:
		/* DECODE: vdest */
		vda_util_dec_vdest(uba, &vda_info->info.vdest_lookup.o_vdest);
		break;

	case NCSVDA_VDEST_DESTROY:
		/* DECODE: vdest */
		vda_util_dec_vdest(uba, &vda_info->info.vdest_destroy.i_vdest);
		break;

	default:
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_mds_cb_enc_flat (private)
\*-------------------------------------------------------------------------*/
static uns32 vda_mds_cb_enc_flat(struct ncsmds_callback_info *info)
{
	NCSVDA_INFO *vda_info;
	NCS_UBAID *uba = info->info.enc_flat.io_uba;
	MDS_CLIENT_MSG_FORMAT_VER msg_fmt_version;

	MDA_TRACE1_ARG1("vda_mds_cb_enc_flat:\n");

	msg_fmt_version = m_NCS_VDA_ENC_MSG_FMT_GET(info->info.enc_flat.i_rem_svc_pvt_ver,
						    VDA_WRT_VDS_SUBPART_VER_MIN,
						    VDA_WRT_VDS_SUBPART_VER_MAX, gl_vda_wrt_vds_msg_fmt_array);
	if (0 == msg_fmt_version) {
		MDA_TRACE1_ARG2("Msg To Service-id:%d Dropped : ", info->info.enc_flat.i_to_svc_id);
		MDA_TRACE1_ARG2("Remote Svc Sub Part Version:%d \n", info->info.enc_flat.i_rem_svc_pvt_ver);
		return NCSCC_RC_FAILURE;
	}
	info->info.enc_flat.o_msg_fmt_ver = msg_fmt_version;

	/* Structure is already flat. This can be done real quick */
	vda_info = info->info.enc_flat.i_msg;
	ncs_encode_n_octets_in_uba(uba, (uns8 *)vda_info, sizeof(*vda_info));
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_mds_cb_dec_flat  (private)
\*-------------------------------------------------------------------------*/
static uns32 vda_mds_cb_dec_flat(struct ncsmds_callback_info *info)
{
	NCSVDA_INFO *vda_info;
	NCS_UBAID *uba = info->info.dec_flat.io_uba;

	MDA_TRACE1_ARG1("vda_mds_cb_dec_flat:\n");

	if (0 == m_NCS_VDA_MSG_FORMAT_IS_VALID(info->info.dec_flat.i_msg_fmt_ver,
					       VDA_WRT_VDS_SUBPART_VER_MIN,
					       VDA_WRT_VDS_SUBPART_VER_MAX, gl_vda_wrt_vds_msg_fmt_array)) {
		MDA_TRACE1_ARG2("Msg From Service-id:%d Dropped :", info->info.dec_flat.i_fr_svc_id);
		MDA_TRACE1_ARG2("MSG Format Version :%d \n", info->info.dec_flat.i_msg_fmt_ver);
		return NCSCC_RC_FAILURE;
	}

	/* The is the VDS's response to a NCSVDA_VDEST_CREATE request */
	if (info->info.dec_flat.o_msg == NULL)
		/* Should never reach here */
		return NCSCC_RC_FAILURE;
	vda_info = info->info.dec_flat.o_msg;

	ncs_decode_n_octets_from_uba(uba, (uns8 *)vda_info, sizeof(*vda_info));
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_mds_cb_rcv (private)
\*-------------------------------------------------------------------------*/
static uns32 vda_mds_cb_rcv(struct ncsmds_callback_info *info)
{
	/* Never expected to be called as VDA always uses SNDRSP variant
	 * of MDS sends.
	 */

	if (0 == m_NCS_VDA_MSG_FORMAT_IS_VALID(info->info.receive.i_msg_fmt_ver,
					       VDA_WRT_VDS_SUBPART_VER_MIN,
					       VDA_WRT_VDS_SUBPART_VER_MAX, gl_vda_wrt_vds_msg_fmt_array)) {
		MDA_TRACE1_ARG2("Msg From Service-id:%d Dropped :", info->info.receive.i_fr_svc_id);
		MDA_TRACE1_ARG2("Msg Format Version: %d \n", info->info.receive.i_msg_fmt_ver);
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

	MDA_TRACE1_ARG1("vda_mds_cb_rcv:\n");
	return NCSCC_RC_FAILURE;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_mds_cb_svc_event (private)
\*-------------------------------------------------------------------------*/
static uns32 vda_mds_cb_svc_event(struct ncsmds_callback_info *info)
{
	MDA_TRACE1_ARG1("vda_mds_cb_svc_event:\n");

	if (info->info.svc_evt.i_svc_id != NCSMDS_SVC_ID_VDS)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	if (info->info.svc_evt.i_change == NCSMDS_UP) {
		/* MDA is only interested in the "primary VDS", so it only records
		 * the VDEST and not the anchor 
		 */

		/* Take VDS-sync-lock */
		m_NCS_LOCK(&gl_vda_info.vds_sync_lock, NCS_LOCK_WRITE);

		MDA_TRACE1_ARG2("vda_mds_cb_svc_event:VDS=%s\n", "UP");
		gl_vda_info.vds_primary_up = TRUE;
		gl_vda_info.vds_vdest = info->info.svc_evt.i_dest;

		if (gl_vda_info.vds_sync_awaited == TRUE) {
			m_NCS_SEL_OBJ_IND(gl_vda_info.vds_sync_sel);
		}

		/* Release VDS-sync-lock */
		m_NCS_UNLOCK(&gl_vda_info.vds_sync_lock, NCS_LOCK_WRITE);
	} else if (info->info.svc_evt.i_change == NCSMDS_DOWN) {
		MDA_TRACE1_ARG2("vda_mds_cb_svc_event:VDS=%s\n", "DOWN");
		gl_vda_info.vds_primary_up = FALSE;
	} else if (info->info.svc_evt.i_change == NCSMDS_CHG_ROLE) {
		/* Do nothing */
	}
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_util_enc_8bit (public)
\*-------------------------------------------------------------------------*/
uns32 vda_util_enc_8bit(NCS_UBAID *uba, uns8 data)
{
	uns8 *p8;
	p8 = ncs_enc_reserve_space(uba, 1);
	ncs_encode_8bit(&p8, data);
	ncs_enc_claim_space(uba, 1);
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_util_dec_8bit (public)
\*-------------------------------------------------------------------------*/
uns8 vda_util_dec_8bit(NCS_UBAID *uba)
{
	uns8 data;
	uns8 *p8;
	p8 = ncs_dec_flatten_space(uba, &data, 1);
	data = ncs_decode_8bit(&p8);
	ncs_dec_skip_space(uba, 1);
	return data;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_util_enc_vdest_name (public)
\*-------------------------------------------------------------------------*/
uns32 vda_util_enc_vdest_name(NCS_UBAID *uba, SaNameT *name)
{
	vda_util_enc_8bit(uba, (uns8)name->length);
	vda_util_enc_n_octets(uba, name->length, name->value);
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_util_dec_vdest_name (public)
\*-------------------------------------------------------------------------*/
uns32 vda_util_dec_vdest_name(NCS_UBAID *uba, SaNameT *name)
{
	name->length = vda_util_dec_8bit(uba);
	vda_util_dec_n_octets(uba, name->length, name->value);
	return NCSCC_RC_SUCCESS;
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_util_enc_vdest (public)
\*-------------------------------------------------------------------------*/
uns32 vda_util_enc_vdest(NCS_UBAID *uba, MDS_DEST *i_dest)
{
	return mds_uba_encode_mds_dest(uba, i_dest);
}

/*-------------------------------------------------------------------------*\
     FUNCTION NAME : vda_util_dec_vdest (public)
\*-------------------------------------------------------------------------*/
uns32 vda_util_dec_vdest(NCS_UBAID *uba, MDS_DEST *o_dest)
{
	return mds_uba_decode_mds_dest(uba, o_dest);
}
