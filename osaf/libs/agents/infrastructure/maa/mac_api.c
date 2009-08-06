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

  DESCRIPTION:

  This file contains all Public APIs for the MIB Access Client (MAC) portion
  of the MIB Acess Broker (MAB) subsystemt.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#if (NCS_MAB == 1)
#include "mac.h"

/*****************************************************************************

  PROCEDURE NAME:    ncsmac_lm

  DESCRIPTION:       Core API for all MIB Access Client layer management 
                     operations used by a target system to instantiate and
                     control a MAC instance. Its operations include:

                     CREATE  a MAC instance
                     DESTROY a MAC instance
                     SET     a MAC configuration object
                     GET     a MAC configuration object

  RETURNS:           SUCCESS - All went well
                     FAILURE - internal processing didn't like something.
                               enable m_MAB_DBG_SINK() for details.

*****************************************************************************/

uns32 ncsmac_lm(NCSMAC_LM_ARG *arg)
{
	switch (arg->i_op) {
	case NCSMAC_LM_OP_CREATE:
		return mac_svc_create(&arg->info.create);

	case NCSMAC_LM_OP_DESTROY:
		return mac_svc_destroy(&arg->info.destroy);

	case NCSMAC_LM_OP_SET:
		return mac_svc_set(&arg->info.set);

	case NCSMAC_LM_OP_GET:
		return mac_svc_get(&arg->info.get);

	default:
		break;

	}
	return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
}

/*****************************************************************************

  PROCEDURE NAME:    ncsmac_mib_request
  
  DESCRIPTION:       The single entry API for all MIB Access Transactions. This 
                     API conforms to the NetPlane Common MIB Access API Prototype,
                     which has a rich set of properties associated with it, as
                     described in the LEAP documentation.

                     This function leads to MAB functionality that is responsible 
                     for:
                     
                     - Finding the MIB object specified somewhere in the system.
                     - Executing the transaction with the owning subsystem
                     - navigating the results back to the callback function of
                       the invoker.

  RETURNS:           SUCCESS - This transaction has successfully been initiated
                     FAILURE - Transaction information is not correct. Turn on
                               m_MAB_DBG_SINK() for details.

*****************************************************************************/

MABMAC_API uns32 ncsmac_mib_request(NCSMIB_ARG *req)
{
	MAC_INST *inst;
	MAB_MSG msg;
	uns32 code;
	NCSMIB_OP req_op = 0;
	uns32 hm_hdl;
	uns16 k = 0;
	MDS_HDL tx_mds_hdl;

	m_MAC_LK_INIT;

	m_MAB_DBG_TRACE("\nncsmac_mib_request():entered.");

	/* Do some high level validation checking */

	if (req == NULL) {	/* is there a request? */
		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAC_INVALID_MIB_REQ_RCVD);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (!m_NCSMIB_ISIT_A_REQ(req->i_op)) {	/* is it type 'request' ? */
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAC_ERR_INVALID_MIB_REQ_OP, req->i_op);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* save the request type now... it will be changes after we call MDS */
	req_op = req->i_op;

	if ((req->i_usr_key == 0) || (req->i_mib_key == 0) || (req->i_rsp_fnc == NULL)) {
		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAC_INVALID_KEYS_RSP_FUNC);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Find the right Virtual Router Instance */
	hm_hdl = (uns32)req->i_mib_key;
	if ((inst = (MAC_INST *)m_MAC_VALIDATE_HDL(hm_hdl)) == NULL) {
		m_LOG_MAB_NO_CB("ncsmac_mib_request()");
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_MAC_LK(&inst->lock, NCS_LOCK_READ);
	/*m_NCS_LOCK_V2(&inst->lock, NCS_LOCK_READ,NCS_SERVICE_ID_MAB, MAC_LOCK_ID); */
	m_LOG_MAB_LOCK(MAB_LK_MAC_LOCKED, &inst->lock);

	tx_mds_hdl = inst->mds_hdl;

	m_LOG_MAB_API(MAB_API_MAC_MIB_REQ);

	/* Make sure that a playback session is not in progress */
	if ((inst->playback_session == TRUE) &&
	    ((req->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)) {
		NCS_BOOL is_ok = TRUE;

		if (inst->plbck_tbl_list != NULL) {
			/* This is for warmboot playback */
			for (k = 1; k < inst->plbck_tbl_list[0]; k++) {
				if ((uns16)req->i_tbl_id == inst->plbck_tbl_list[k]) {
					is_ok = FALSE;
					break;
				}
			}
		} else {
			/* This is for on-demand playback */
			is_ok = FALSE;
		}
		if (is_ok == FALSE) {
			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAC_PLAYBACK_IN_PROGRESS);
			m_MAC_UNLK(&inst->lock, NCS_LOCK_READ);
			ncshm_give_hdl(hm_hdl);
			m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_PLBK_IN_PROGRESS);
		}
	}

	/* Make sure MAC sees MAS */
	if (inst->mas_here == FALSE) {
		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAC_NO_MAS_FRWD);
		m_MAC_UNLK(&inst->lock, NCS_LOCK_READ);
		ncshm_give_hdl(hm_hdl);
		m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_NO_MAS);
	}
	/* Looks like everything is lined up.. Lets do it!! 
	 * Put private stack in start state and push 
	 */
	{
		NCS_SE *se;	/* prepare for NCSMIB_ARG stack stuff */
		uns8 *stream;

		/* Save information about the original MIB request sender */
		if ((se = ncsstack_push(&req->stack, NCS_SE_TYPE_MIB_ORIG, (uns8)sizeof(NCS_SE_MIB_ORIG))) == NULL) {
			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAC_PUSH_ORIG_FAILED);
			ncshm_give_hdl(hm_hdl);
			m_MAC_UNLK(&inst->lock, NCS_LOCK_READ);
			m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		stream = m_NCSSTACK_SPACE(se);

		ncs_encode_64bit(&stream, req->i_usr_key);

		if (4 == sizeof(void *))
			ncs_encode_32bit(&stream, (uns32)(long)req->i_rsp_fnc);
		else if (8 == sizeof(void *))
			ncs_encode_64bit(&stream, (long)req->i_rsp_fnc);

		/* Push the directions to get back...  */
		if ((se = ncsstack_push(&req->stack, NCS_SE_TYPE_BACKTO, (uns8)sizeof(NCS_SE_BACKTO))) == NULL)
		{
			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAC_PUSH_BTSE_FAILED);
			ncshm_give_hdl(hm_hdl);
			m_MAC_UNLK(&inst->lock, NCS_LOCK_READ);
			m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAC_PUSH_BTSE_SUCCESS);
		stream = m_NCSSTACK_SPACE(se);

		mds_st_encode_mds_dest(&stream, &inst->my_vcard);
		ncs_encode_16bit(&stream, NCSMDS_SVC_ID_MAC);
		ncs_encode_16bit(&stream, inst->vrid);

		/* assign all msg fields rather then memset */
		msg.vrid = inst->vrid;
		memset(&msg.fr_card, 0, sizeof(msg.fr_card));
		msg.fr_svc = 0;
		msg.op = MAB_MAS_REQ_HDLR;
		msg.data.data.snmp = req;

	}

	/* Send the message */
	m_MAC_UNLK(&inst->lock, NCS_LOCK_READ);
	m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED, &inst->lock);
	code = mab_mds_snd(tx_mds_hdl, &msg, NCSMDS_SVC_ID_MAC, NCSMDS_SVC_ID_MAS, inst->mas_vcard);
	if (code != NCSCC_RC_SUCCESS) {
		m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_MAC_MDS_SND_MSG_FAILED, NCSMDS_SVC_ID_MAS, code);
		ncshm_give_hdl(hm_hdl);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	switch (req_op) {
	case NCSMIB_OP_REQ_SETROW:
	case NCSMIB_OP_REQ_TESTROW:
	case NCSMIB_OP_REQ_SETALLROWS:
	case NCSMIB_OP_REQ_REMOVEROWS:
		m_MMGR_FREE_BUFR_LIST(req->req.info.setrow_req.i_usrbuf);
		req->req.info.setrow_req.i_usrbuf = NULL;
		break;

	case NCSMIB_OP_REQ_CLI:
		m_MMGR_FREE_BUFR_LIST(req->req.info.cli_req.i_usrbuf);
		req->req.info.cli_req.i_usrbuf = NULL;
		break;

	default:
		break;
	}

	m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAC_FRWD_MIB_REQ_TO_MAS);

	m_MAB_DBG_TRACE("\nncsmac_mib_request():left.");

	ncshm_give_hdl(hm_hdl);
	return code;
}

/*****************************************************************************
 *****************************************************************************

     P R I V A T E  Support Functions for MAC APIs are below

*****************************************************************************
*****************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:    mac_svc_create

  DESCRIPTION:       Create an instance of MAC, set configuration profile to
                     default, install this MAC with MDS and subscribe to MAS
                     and OAC events.

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 mac_svc_create(NCSMAC_CREATE *create)
{
	MAC_INST *inst;
	NCSMDS_INFO mds_info;
	MDS_SVC_ID subsvc[2];
	uns32 status;
	NCS_SPIR_REQ_INFO spir_info;

	/* what is this all about? */
	m_MAC_LK_INIT;

	if (create->i_lm_cbfnc == NULL) {
		/* log the failure */
		m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_SVC_PRVDR, MAB_MAC_ERR_NO_LM_CALLBACK);
		return NCSCC_RC_FAILURE;
	}

	if ((inst = m_MMGR_ALLOC_MAC_INST) == NULL) {
		/* log the memory failure */
		m_LOG_MAB_MEMFAIL(NCSFL_SEV_ERROR, MAB_MF_MAC_INST_ALLOC_FAILED, "mac_svc_create()");

		/* log the env-id as well */
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAC_ERR_CB_ALLOC_FAILED_ENVID, create->i_vrid);

		return NCSCC_RC_FAILURE;
	}

	/* confirm all fields are initialized  memset(inst,0,sizeof(MAC_INST)); */
	memset(inst, 0, sizeof(MAC_INST));
	m_MAC_LK_CREATE(&inst->lock);
	m_MAC_LK(&inst->lock, NCS_LOCK_WRITE);

	/* MAC cannot police for illegal duplicates, since MAC has no concept or */
	/* access to entire set. Customer must be diligent not to do wrong thing */
	inst->vrid = create->i_vrid;

	/* get the PWE handle for this PWE */
	memset(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO));
	spir_info.type = NCS_SPIR_REQ_LOOKUP_CREATE_INST;
	spir_info.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
	spir_info.i_instance_name = m_MDS_SPIR_ADEST_NAME;
	spir_info.i_environment_id = create->i_vrid;
	status = ncs_spir_api(&spir_info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_MAC_ERR_SPIR_LOOKUP_CREATE_FAILED, status, create->i_vrid);
		/* free the allocated MAA CB */
		m_MAC_UNLK(&inst->lock, NCS_LOCK_WRITE);
		m_MMGR_FREE_MAC_INST(inst);
		return status;
	}

	/* store the PWE handle of OAA */
	inst->mds_hdl = (MDS_HDL)spir_info.info.lookup_create_inst.o_handle;

	/* initialize the mbx pointer */
	inst->mbx = create->i_mbx;
	inst->hm_poolid = create->i_hm_poolid;
	inst->hm_hdl = ncshm_create_hdl(inst->hm_poolid, NCS_SERVICE_ID_MAB, (NCSCONTEXT)inst);
	/* set the default values to the clients */
	inst->mas_here = FALSE;
	inst->playback_session = FALSE;
	inst->psr_here = FALSE;

	/* LM Callback stuff */
	inst->lm_cbfnc = create->i_lm_cbfnc;

	m_NCS_SEL_OBJ_CREATE(&inst->mas_sync_sel);

	/* MAC joins the MDS crowd... advertises its presence */
	/* install MAA */
	memset(&mds_info, 0, sizeof(mds_info));
	mds_info.i_mds_hdl = inst->mds_hdl;
	mds_info.i_op = MDS_INSTALL;
	mds_info.i_svc_id = NCSMDS_SVC_ID_MAC;
	mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
	mds_info.info.svc_install.i_mds_q_ownership = FALSE;
	mds_info.info.svc_install.i_svc_cb = mac_mds_cb;
	mds_info.info.svc_install.i_yr_svc_hdl = inst->hm_hdl;
	mds_info.info.svc_install.i_mds_svc_pvt_ver = (uns8)MAA_MDS_SUB_PART_VERSION;
	status = ncsmds_api(&mds_info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_MAC_ERR_MDS_INSTALL_FAILED, status, create->i_vrid);
		m_MAC_UNLK(&inst->lock, NCS_LOCK_WRITE);
		/* free the allocated MAA CB */
		m_MMGR_FREE_MAC_INST(inst);
		return status;
	}
	inst->my_vcard = mds_info.info.svc_install.o_dest;

	/* subscribe to MAS and PSS process events */
	memset(&mds_info, 0, sizeof(mds_info));
	mds_info.i_mds_hdl = inst->mds_hdl;
	mds_info.i_op = MDS_SUBSCRIBE;
	mds_info.i_svc_id = NCSMDS_SVC_ID_MAC;
	subsvc[0] = NCSMDS_SVC_ID_MAS;
	subsvc[1] = NCSMDS_SVC_ID_PSS;
	mds_info.info.svc_subscribe.i_num_svcs = 2;
	mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
	mds_info.info.svc_subscribe.i_svc_ids = subsvc;
	status = ncsmds_api(&mds_info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_MAC_ERR_MDS_SUBSCRB_FAILED, status, create->i_vrid);
		m_MAC_UNLK(&inst->lock, NCS_LOCK_WRITE);
		/* free the allocated MAA CB */
		m_MMGR_FREE_MAC_INST(inst);
		return NCSCC_RC_FAILURE;
	}

	/* return the handle */
	create->o_mac_hdl = inst->hm_hdl;

	/* commenting for testing. */
	m_MAC_UNLK(&inst->lock, NCS_LOCK_WRITE);

	return status;
}

/*****************************************************************************

  PROCEDURE NAME:    mac_svc_destroy

  DESCRIPTION:       Destroy an instance of MAC. Withdraw from MDS and free
                     this MAC_INST of MAC.

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 mac_svc_destroy(NCSMAC_DESTROY *destroy)
{
	MAC_INST *inst;
	NCSMDS_INFO info;
	uns32 status;
	NCS_SPIR_REQ_INFO spir_info;

	m_MAC_LK_INIT;

	if ((inst = (MAC_INST *)m_MAC_VALIDATE_HDL(destroy->i_mac_hdl)) == NULL) {
		/* log the failure */
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAC_ERR_CB_NULL, destroy->i_mac_hdl);
		return NCSCC_RC_FAILURE;
	}

	m_MAC_LK(&inst->lock, NCS_LOCK_READ);

	m_LOG_MAB_API(MAB_API_MAC_SVC_DESTROY);

	/* Uninstall from MDS services                                */
	/* Unsubscribe from service events is implicit with Uninstall */
	memset(&info, 0, sizeof(info));
	info.i_mds_hdl = inst->mds_hdl;
	info.i_svc_id = NCSMDS_SVC_ID_MAC;
	info.i_op = MDS_UNINSTALL;
	status = ncsmds_api(&info);
	if (status != NCSCC_RC_SUCCESS) {
		m_MAC_UNLK(&inst->lock, NCS_LOCK_READ);
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_MAC_ERR_MDS_UNINSTALL_FAILED, status, destroy->i_env_id);
		m_MAB_DBG_VOID;
	}

	/* release the PWE instance from SPRR */
	memset(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO));
	spir_info.type = NCS_SPIR_REQ_REL_INST;
	spir_info.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
	spir_info.i_instance_name = m_MDS_SPIR_ADEST_NAME;
	spir_info.i_environment_id = destroy->i_env_id;
	status = ncs_spir_api(&spir_info);
	if (status != NCSCC_RC_SUCCESS) {
		m_MAC_UNLK(&inst->lock, NCS_LOCK_READ);
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_MAC_ERR_SPIR_REL_INST_FAILED, status, destroy->i_env_id);
	}

	/* done with the handles */
	ncshm_give_hdl(destroy->i_mac_hdl);
	ncshm_destroy_hdl(NCS_SERVICE_ID_MAB, inst->hm_hdl);

	/* mac_vrtr_remove() will unlock the lock for this instance */
	m_MAC_UNLK(&inst->lock, NCS_LOCK_READ);
	m_MAC_LK_DLT(&inst->lock);
	m_MMGR_FREE_MAC_INST(inst);

	return status;
}

/*****************************************************************************

  PROCEDURE NAME:    mac_svc_get

  DESCRIPTION:       Fetch the value of a MAC object, which are:
                       NCSMAC_OBJID_LOG_VAL
                       NCSMAC_OBJID_LOG_ON_OFF
                       NCSMAC_OBJID_SUBSYS_ON

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/

uns32 mac_svc_get(NCSMAC_GET *get)
{
	MAC_INST *inst;
	uns32 *pval = &get->o_obj_val;

	m_MAC_LK_INIT;

	if ((inst = (MAC_INST *)m_MAC_VALIDATE_HDL(get->i_mac_hdl)) == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	m_MAC_LK(&inst->lock, NCS_LOCK_READ);

	switch (get->i_obj_id) {
	case NCSMAC_OBJID_LOG_VAL:
		break;
	case NCSMAC_OBJID_LOG_ON_OFF:
		break;
	case NCSMAC_OBJID_SUBSYS_ON:
		*pval = inst->mac_enbl;
		break;
	default:
		m_LOG_MAB_API(MAB_API_MAC_SVC_GET);
		ncshm_give_hdl(get->i_mac_hdl);
		m_MAC_UNLK(&inst->lock, NCS_LOCK_READ);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_LOG_MAB_API(MAB_API_MAC_SVC_GET);
	ncshm_give_hdl(get->i_mac_hdl);
	m_MAC_UNLK(&inst->lock, NCS_LOCK_READ);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    mac_svc_set

  DESCRIPTION:       Set the value of a MAC object, which are:
                       NCSMAC_OBJID_LOG_VAL
                       NCSMAC_OBJID_LOG_ON_OFF
                       NCSMAC_OBJID_SUBSYS_ON

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/

uns32 mac_svc_set(NCSMAC_SET *set)
{
	MAC_INST *inst;

	m_MAC_LK_INIT;

	if ((inst = (MAC_INST *)m_MAC_VALIDATE_HDL(set->i_mac_hdl)) == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	m_MAC_LK(&inst->lock, NCS_LOCK_WRITE);

	switch (set->i_obj_id) {
	case NCSMAC_OBJID_LOG_VAL:
		break;
	case NCSMAC_OBJID_LOG_ON_OFF:
		break;
	case NCSMAC_OBJID_SUBSYS_ON:
		inst->mac_enbl = set->i_obj_val;
		break;
	default:
		ncshm_give_hdl(set->i_mac_hdl);
		m_LOG_MAB_API(MAB_API_MAC_SVC_SET);
		m_MAC_UNLK(&inst->lock, NCS_LOCK_WRITE);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	ncshm_give_hdl(set->i_mac_hdl);

	m_LOG_MAB_API(MAB_API_MAC_SVC_SET);
	m_MAC_UNLK(&inst->lock, NCS_LOCK_WRITE);

	return NCSCC_RC_SUCCESS;
}

#endif   /* (NCS_MAB == 1) */
