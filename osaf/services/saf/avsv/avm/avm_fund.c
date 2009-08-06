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

/*****************************************************************************/

#include "avm.h"
#include "mac_papi.h"
uns32 avm_gen_fund_mib_set(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, uns32 cmd_id, uns8 *filename)
{
	NCSMIB_ARG arg;
	AVM_FUND_USR_BUF out_usr_buf;
	struct slot_info sss;
	USRBUF *buff;
	uns8 *buff_ptr;
	uns32 xch_id = 1;
	uns8 str[AVM_LOG_STR_MAX_LEN];
	uns32 inst_ids[2] = { 0 };
	/* vivek_avm */
	uns8 local_helper[AVM_NAME_STR_LENGTH];

	str[0] = '\0';

	memset(&sss, 0, sizeof(struct slot_info));
	memset(&arg, 0, sizeof(NCSMIB_ARG));
	memset(&out_usr_buf, 0, sizeof(AVM_FUND_USR_BUF));
	memset(local_helper, 0, sizeof(local_helper));
	ncsmib_init(&arg);

	/* avm_fund_get_scxb_ipaddr(&out_usr_buf); - remove later - JPL */

	/* vivek_avm */
	memcpy(local_helper, ent_info->dhcp_serv_conf.ipmc_helper_node.name,
	       ent_info->dhcp_serv_conf.ipmc_helper_node.length);
	extract_slot_shelf_subslot(local_helper, &sss);

	inst_ids[0] = htons(sss.slot);
	inst_ids[1] = htons(sss.shelf);

	out_usr_buf.cmd_parm.cmd_id = htons(cmd_id);
	/* send the ipmb address in char format */
	snprintf(out_usr_buf.cmd_parm.payload_ipaddr, sizeof(out_usr_buf.cmd_parm.payload_ipaddr) - 1, "%x",
		 ent_info->dhcp_serv_conf.ipmb_addr);
	strncpy(out_usr_buf.cmd_parm.filename, filename, sizeof(out_usr_buf.cmd_parm.filename));

	buff = m_MMGR_ALLOC_BUFR(sizeof(USRBUF));
	if (buff == NULL) {
		snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload blade %s: USRBUF memory allocation failed",
			 ent_info->ep_str.name);
		m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);
		return NCSCC_RC_FAILURE;
	}

	m_BUFR_STUFF_OWNER(buff);

	buff_ptr = m_MMGR_RESERVE_AT_END(&buff, sizeof(AVM_FUND_USR_BUF), uns8 *);
	if (buff_ptr == NULL) {
		snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload blade %s: USRBUF reserve at end failed",
			 ent_info->ep_str.name);
		m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);
		return NCSCC_RC_FAILURE;
	}

	memcpy(buff_ptr, &out_usr_buf, sizeof(AVM_FUND_USR_BUF));

	arg.req.info.cli_req.i_usrbuf = buff;
	/* Fill the MIB_ARG structure */
	arg.i_op = NCSMIB_OP_REQ_CLI;
	arg.i_tbl_id = NCSMIB_FUND_TBL;
	arg.i_rsp_fnc = avm_fund_resp_func;
	arg.i_xch_id = xch_id++;
	arg.i_idx.i_inst_ids = inst_ids;
	arg.i_idx.i_inst_len = 2;

	/* Update CLI_REQ structure of MIB_ARG */
	arg.req.info.cli_req.i_wild_card = FALSE;
	if ((avm_fund_send_mib_req(ent_info, &arg)) == NCSCC_RC_FAILURE) {
		snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload blade %s: avm_fund_send_mib_req failed",
			 ent_info->ep_str.name);
		m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

extern uns32 avm_fund_send_mib_req(AVM_ENT_INFO_T *ent_info, NCSMIB_ARG *mib_arg)
{
	uns32 rc;
	uns8 str[AVM_LOG_STR_MAX_LEN];

	str[0] = '\0';

	mib_arg->i_usr_key = ent_info->ent_hdl;
	mib_arg->i_mib_key = gl_mac_handle;	/* TBD JPL */

	rc = ncsmib_timed_request(mib_arg, ncsmac_mib_request, 72000);
	if (rc != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

uns32 avm_fund_resp_func(NCSMIB_ARG *resp)
{
	uns32 cb_hdl;
	AVM_EVT_T *evt = AVM_EVT_NULL;
	AVM_CB_T *cb = NULL;
	AVM_ENT_INFO_T *ent_info = NULL;

	if (resp->i_usr_key == 0)
		return NCSCC_RC_FAILURE;

	/* Get the ent info data for target blade */
	if (NULL == (ent_info = (AVM_ENT_INFO_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, resp->i_usr_key))) {
		return NCSCC_RC_FAILURE;
	}

	/* Get AVM CB from Ent data */
	if (AVM_CB_NULL == (cb = ncshm_take_hdl(NCS_SERVICE_ID_AVM, ent_info->cb_hdl))) {
		m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		ncshm_give_hdl(resp->i_usr_key);
		return NCSCC_RC_FAILURE;
	}
	cb_hdl = ent_info->cb_hdl;

	evt = m_MMGR_ALLOC_AVM_EVT;

	if (AVM_EVT_NULL == evt) {
		m_AVM_LOG_MEM(AVM_LOG_EVT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
		ncshm_give_hdl(cb_hdl);
		ncshm_give_hdl(resp->i_usr_key);
		return NCSCC_RC_FAILURE;
	}

	evt->src = AVM_EVT_FUND;

	/* make a copy of the MIB request */
	evt->evt.mib_req = ncsmib_memcopy(resp);

	if (NULL == evt->evt.mib_req) {
		m_AVM_LOG_MAS(AVM_LOG_MIB_CPY, AVM_LOG_MAS_FAILURE, NCSFL_SEV_EMERGENCY);
		m_MMGR_FREE_AVM_EVT(evt);
		ncshm_give_hdl(cb_hdl);
		ncshm_give_hdl(resp->i_usr_key);
		return NCSCC_RC_FAILURE;
	}

	if (m_NCS_IPC_SEND(&cb->mailbox, evt, NCS_IPC_PRIORITY_VERY_HIGH)
	    != NCSCC_RC_SUCCESS) {
		m_AVM_LOG_MBX(AVM_LOG_MBX_SEND, AVM_LOG_MBX_FAILURE, NCSFL_SEV_EMERGENCY);
		m_MMGR_FREE_AVM_EVT(evt);
		ncshm_give_hdl(cb_hdl);
		ncshm_give_hdl(resp->i_usr_key);
		return NCSCC_RC_FAILURE;
	}

	ncshm_give_hdl(cb_hdl);
	ncshm_give_hdl(resp->i_usr_key);
	return NCSCC_RC_SUCCESS;
}

uns32 avm_proc_fund(AVM_EVT_T *fund_resp, AVM_CB_T *cb)
{
	NCSMIB_ARG *resp;

	AVM_ENT_INFO_T *ent_info;
	uns8 unlock = 0, failure = 0, rollback = 0;
	uns8 str[AVM_LOG_STR_MAX_LEN];
	uns32 retStat = NCSCC_RC_SUCCESS;

	uns32 buff_len = 0;
	uns8 *buff_ptr;
	char local_buf[10];
	uns8 pld_bld_status = 0, pld_rtm_status = 0;

	resp = fund_resp->evt.mib_req;	/* should I free it?? - TBD - JPL */
	/* Get the ent info */
	if (NULL == (ent_info = (AVM_ENT_INFO_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, resp->i_usr_key))) {
		return NCSCC_RC_FAILURE;
	}
	str[0] = '\0';

	/* Check for the role_chg_wait_tmr, if it is running, stop it */
	if (ent_info->role_chg_wait_tmr.status == AVM_TMR_RUNNING)
		avm_stop_tmr(cb, &ent_info->role_chg_wait_tmr);

	switch (resp->rsp.i_status) {
	case NCSCC_RC_SUCCESS:
		{
			switch (resp->rsp.info.cli_rsp.i_cmnd_id) {
			case AVM_FUND_PLD_BLD_AND_RTM_IPMC_UPGD_CMD:
				{
					/* Get the size of the buffer */
					buff_len = m_MMGR_LINK_DATA_LEN(resp->rsp.info.cli_rsp.o_answer);
					/* Stop the timer */
					avm_stop_tmr(cb, &ent_info->ipmc_tmr);
					if (buff_len <= 0 || buff_len > 2) {
						/* IPMC UPGRADE Command failed. Log it. Send the trap */
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload %s: AVM_FUND_PLD_BLD_AND_RTM_IPMC_UPGD_CMD: Failed: Unknown resp length",
							 ent_info->ep_str.name);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);
						cb->upgrade_error_type = UNKNOWN_ERROR;
						cb->upgrade_module = IPMC_PLD;
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);
						failure = 1;
						/* AVM is not aware, what happened at the FUND. Rollback both PLD BLD and RTM to be safe side */
						rollback = IPMC_PLD;	/* Rollback both the modules */
					} else {
						if ((buff_ptr =
						     (uns8 *)m_MMGR_DATA_AT_START(resp->rsp.info.cli_rsp.o_answer,
										  buff_len, local_buf)) == (uns8 *)0) {
							/* Log Error */
							snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
								 "AVM-SSU: Payload %s: AVM_FUND_PLD_BLD_AND_RTM_IPMC_UPGD_CMD: Failed: BUFF Data Extract",
								 ent_info->ep_str.name);
							m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);

							ncshm_give_hdl(resp->i_usr_key);
							return NCSCC_RC_SUCCESS;
						}

						if (buff_len == 1) {
							/* Only if the Payload blade IPMC upgrade is failed, the buff_len will be 1 */
							pld_bld_status = buff_ptr[0];

							if (pld_bld_status >= GEN_ERROR
							    && pld_bld_status <= FUND_TFTP_ERROR) {
								cb->upgrade_error_type = pld_bld_status;
								unlock = 1;
							} else {
								/* If the error type is unknown, AVM doesn't know, what happened at FUND.
								   Rollback IPMC_PLD_BLD to be safe side */
								cb->upgrade_error_type = UNKNOWN_ERROR;
								rollback = IPMC_PLD_BLD;
							}
							snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
								 "AVM-SSU: Payload %s: Payload IPMC upgrade failed: ErrorType=%d ",
								 ent_info->ep_str.name, cb->upgrade_error_type);
							m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);
							cb->upgrade_module = IPMC_PLD_BLD;
							avm_send_boot_upgd_trap(cb, ent_info,
										ncsAvmSwFwUpgradeFailure_ID);
							failure = 1;
						} else {
							pld_bld_status = buff_ptr[0];
							pld_rtm_status = buff_ptr[1];

							if ((pld_bld_status == IPMC_SAME_VERSION)
							    || (pld_bld_status == PAYLOAD_IPMC_UPGD_SUCCESS)) {
								if (pld_bld_status == PAYLOAD_IPMC_UPGD_SUCCESS) {
									ent_info->dhcp_serv_conf.pld_bld_ipmc_status =
									    pld_bld_status;
									cb->upgrade_prg_evt = AVM_UPGRADE_SUCCESS;
								} else {
									cb->upgrade_prg_evt = AVM_SAME_VERSION;
									ent_info->dhcp_serv_conf.pld_bld_ipmc_status =
									    pld_bld_status;
								}
								cb->upgrade_module = IPMC_PLD_BLD;
								snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
									 "AVM-SSU: Payload blade %s: IPMC PLD BLD Upgrade Success: Progress Tpye=%d ",
									 ent_info->ep_str.name, cb->upgrade_prg_evt);
								m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
								avm_send_boot_upgd_trap(cb, ent_info,
											ncsAvmUpgradeProgress_ID);
								unlock = 1;
							} else {
								/* AVM is not aware, what happened at the FUND. Rollback PLD BLD to be safe side */
								snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
									 "AVM-SSU: Payload blade %s: IPMC PLD BLD Unknown Upgrade Status: ",
									 ent_info->ep_str.name);
								m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);
								cb->upgrade_error_type = UNKNOWN_ERROR;
								cb->upgrade_module = IPMC_PLD_BLD;
								avm_send_boot_upgd_trap(cb, ent_info,
											ncsAvmSwFwUpgradeFailure_ID);
								failure = 1;
								rollback = IPMC_PLD_BLD;
							}

							if ((pld_rtm_status >= GEN_ERROR)
							    && (pld_rtm_status <= UNKNOWN_RTM_TYPE)) {
								cb->upgrade_error_type = pld_rtm_status;
								cb->upgrade_module = IPMC_PLD_RTM;
								snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
									 "AVM-SSU: Payload  %s: Payload RTM IPMC upgrade failed: ErrorType=%d ",
									 ent_info->ep_str.name, cb->upgrade_error_type);
								m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);
								avm_send_boot_upgd_trap(cb, ent_info,
											ncsAvmSwFwUpgradeFailure_ID);
								failure = 1;

								/* Rollback the Payload blade IPMC, if it is upgraded */
								if (ent_info->dhcp_serv_conf.pld_bld_ipmc_status) {
									rollback = IPMC_PLD_BLD;
								} else
									unlock = 1;
							} else if ((pld_rtm_status >= IPMC_SAME_VERSION)
								   && (pld_rtm_status <= PAYLOAD_RTM_NOT_PRESENT)) {
								if (pld_rtm_status == PAYLOAD_IPMC_UPGD_SUCCESS) {
									ent_info->dhcp_serv_conf.pld_rtm_ipmc_status =
									    pld_rtm_status;
									if (rollback) {
										/* If AVM already decided to rollback the payload blade, rollback the payload rtm also */
										rollback = IPMC_PLD;
									}
									cb->upgrade_prg_evt = AVM_UPGRADE_SUCCESS;
									cb->upgrade_module = IPMC_PLD_RTM;
									avm_send_boot_upgd_trap(cb, ent_info,
												ncsAvmUpgradeProgress_ID);
								} else if (pld_rtm_status == IPMC_SAME_VERSION) {
									ent_info->dhcp_serv_conf.pld_rtm_ipmc_status =
									    pld_rtm_status;
									cb->upgrade_prg_evt = AVM_SAME_VERSION;
									cb->upgrade_module = IPMC_PLD_RTM;
									avm_send_boot_upgd_trap(cb, ent_info,
												ncsAvmUpgradeProgress_ID);
								}
								snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
									 "AVM-SSU: Payload blade %s: IPMC PLD RTM Upgrade Success: Progress Tpye=%d ",
									 ent_info->ep_str.name, cb->upgrade_prg_evt);
								m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);

								if (rollback == 0)
									unlock = 1;
							} else {
								/* AVM is not aware, what happened at the FUND. Rollback PLD BLD & RTM to be safe side */
								snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
									 "AVM-SSU: Payload blade %s: IPMC PLD RTM Unknown Upgrade Status: ",
									 ent_info->ep_str.name);
								m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);
								cb->upgrade_error_type = UNKNOWN_ERROR;
								m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
								cb->upgrade_module = IPMC_PLD_RTM;
								avm_send_boot_upgd_trap(cb, ent_info,
											ncsAvmSwFwUpgradeFailure_ID);
								failure = 1;
								rollback = IPMC_PLD;
							}
						}
					}
				}
				if (failure) {
					snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
						 "AVM-SSU: Payload %s: AVM_FUND_PLD_BLD_AND_RTM_IPMC_UPGD_CMD: Handle Failure",
						 ent_info->ep_str.name);
					m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);

					/* Check the type of upgrade */
					if (INTEG == ent_info->dhcp_serv_conf.upgrade_type) {
						/* if the type is INTEG */

						AVM_ENT_DHCP_CONF *dhcp_conf = &ent_info->dhcp_serv_conf;

						if (ent_info->dhcp_serv_conf.curr_act_label->other_label->status !=
						    SSU_UPGD_FAILED) {
							/* change the state to upgrade failure */
							ent_info->dhcp_serv_conf.curr_act_label->other_label->status =
							    SSU_UPGD_FAILED;

							/* rollback the preferred label */
							if (dhcp_conf->def_label_num == AVM_DEFAULT_LABEL_1)
								dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_2;
							else
								dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_1;

							dhcp_conf->default_label =
							    dhcp_conf->default_label->other_label;

							/* Set the preferred label also to other label */
							dhcp_conf->pref_label.length =
							    dhcp_conf->default_label->name.length;
							memcpy(dhcp_conf->pref_label.name,
							       dhcp_conf->default_label->name.name,
							       dhcp_conf->pref_label.length);

							dhcp_conf->default_chg = FALSE;	/* TBD - JPL */

							snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
								 "AVM-SSU: Payload blade %s : Preferred label set back to current active label: %s",
								 ent_info->ep_str.name, dhcp_conf->pref_label.name);
							m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);

							m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(cb, ent_info,
											AVM_CKPT_ENT_DHCP_STATE_CHG);
						}
					}
					if (rollback == IPMC_PLD_BLD) {
						/* reset the status */
						ent_info->dhcp_serv_conf.pld_bld_ipmc_status = 0;
						ent_info->dhcp_serv_conf.ipmc_upgd_state = IPMC_PLD_BLD_ROLLBACK_IN_PRG;
						m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
									       AVM_CKPT_ENT_UPGD_STATE_CHG);
						cb->upgrade_prg_evt = AVM_ROLLBACK_TRIGGERED;
						cb->upgrade_module = IPMC_PLD_BLD;
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: IPMC PLD BLD - Rollback triggered ",
							 ent_info->ep_str.name);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmUpgradeProgress_ID);
						m_AVM_UPGD_IPMC_PLD_MOD_TMR_START(cb, ent_info);
						retStat =
						    avm_gen_fund_mib_set(cb, ent_info, AVM_FUND_PLD_BLD_IPMC_DNGD_CMD,
									 ent_info->dhcp_serv_conf.curr_act_label->
									 file_name.name);
						ncshm_give_hdl(resp->i_usr_key);
						return retStat;
					}
					if (rollback == IPMC_PLD) {
						/* reset the status */
						ent_info->dhcp_serv_conf.pld_bld_ipmc_status = 0;
						ent_info->dhcp_serv_conf.pld_rtm_ipmc_status = 0;
						ent_info->dhcp_serv_conf.ipmc_upgd_state = IPMC_ROLLBACK_IN_PRG;

						m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
									       AVM_CKPT_ENT_UPGD_STATE_CHG);

						cb->upgrade_prg_evt = AVM_ROLLBACK_TRIGGERED;
						cb->upgrade_module = IPMC_PLD;
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: IPMC PLD - Rollback triggered ",
							 ent_info->ep_str.name);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmUpgradeProgress_ID);
						m_AVM_UPGD_IPMC_PLD_TMR_START(cb, ent_info);
						retStat =
						    avm_gen_fund_mib_set(cb, ent_info,
									 AVM_FUND_PLD_BLD_AND_RTM_IPMC_DNGD_CMD,
									 ent_info->dhcp_serv_conf.curr_act_label->
									 file_name.name);
						ncshm_give_hdl(resp->i_usr_key);
						return retStat;
					}

					ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
					m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);

				}
				break;

			case AVM_FUND_PLD_BLD_IPMC_DNGD_CMD:
				{
					uns32 buff_len = 0;
					uns8 pld_bld_status = 0;

					/* Stop the timer */
					avm_stop_tmr(cb, &ent_info->ipmc_mod_tmr);

					/* Get the size of the buffer */
					buff_len = m_MMGR_LINK_DATA_LEN(resp->rsp.info.cli_rsp.o_answer);

					if (buff_len <= 0 || buff_len > 1) {
						/* IPMC Payload blade downgrade Command failed. Log it. Send the trap */
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: AVM_FUND_PLD_BLD_IPMC_DNGD_CMD failed - unknown resp length",
							 ent_info->ep_str.name);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
						cb->upgrade_error_type = GEN_ERROR;
						cb->upgrade_module = IPMC_PLD_BLD;
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);

						ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
						m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
									       AVM_CKPT_ENT_UPGD_STATE_CHG);
						ncshm_give_hdl(resp->i_usr_key);
						return NCSCC_RC_FAILURE;
					}

					if ((buff_ptr = (uns8 *)m_MMGR_DATA_AT_START(resp->rsp.info.cli_rsp.o_answer,
										     buff_len,
										     local_buf)) == (uns8 *)0) {
						/* Error handling TBD - JPL */
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload %s: AVM_FUND_PLD_BLD_IPMC_DNGD_CMD: Failed: BUFF Data Extract",
							 ent_info->ep_str.name);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);

						ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
						m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
									       AVM_CKPT_ENT_UPGD_STATE_CHG);
						ncshm_give_hdl(resp->i_usr_key);
						return NCSCC_RC_FAILURE;
					}

					pld_bld_status = buff_ptr[0];

					if (pld_bld_status >= GEN_ERROR && pld_bld_status <= FUND_TFTP_ERROR) {
						cb->upgrade_error_type = pld_bld_status;

						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: Payload blade IPMC downgrade failed: ErrorType=%d ",
							 ent_info->ep_str.name, cb->upgrade_error_type);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
						cb->upgrade_module = IPMC_PLD_BLD;
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);
						ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
						m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
									       AVM_CKPT_ENT_UPGD_STATE_CHG);
						ncshm_give_hdl(resp->i_usr_key);
						return NCSCC_RC_FAILURE;
					}
					if (pld_bld_status == PAYLOAD_IPMC_UPGD_SUCCESS) {
						/* ent_info->dhcp_serv_conf.pld_bld_ipmc_status = pld_bld_status;  - TBD - JPL */
						cb->upgrade_prg_evt = AVM_UPGRADE_SUCCESS;
						cb->upgrade_module = IPMC_PLD_BLD;
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: IPMC PLD BLD Downgrade Success: Progress Tpye=%d ",
							 ent_info->ep_str.name, cb->upgrade_prg_evt);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmUpgradeProgress_ID);	/* sending trap TBD - JPL */
						unlock = 1;
					} else if (pld_bld_status == IPMC_SAME_VERSION) {
						/* shouldn't fall into this case - TBD - JPL */
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: IPMC PLD BLD - SAME VERSION: Rollback not required: Progress Type=%d ",
							 ent_info->ep_str.name, pld_bld_status);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
						unlock = 1;
					} else {
						/* AVM doesn't know, what happened at FUND */
						cb->upgrade_error_type = UNKNOWN_ERROR;
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: Payload blade IPMC Rollback failed: ErrorType=%d ",
							 ent_info->ep_str.name, cb->upgrade_error_type);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
						cb->upgrade_module = IPMC_PLD_BLD;
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);
						ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
						m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
									       AVM_CKPT_ENT_UPGD_STATE_CHG);
						ncshm_give_hdl(resp->i_usr_key);
						return NCSCC_RC_FAILURE;
					}
				}
				break;

			case AVM_FUND_PLD_RTM_IPMC_DNGD_CMD:
				{
					uns32 buff_len = 0;
					uns8 pld_rtm_status = 0;

					/* Stop the timer */
					avm_stop_tmr(cb, &ent_info->ipmc_mod_tmr);

					/* Get the size of the buffer */
					buff_len = m_MMGR_LINK_DATA_LEN(resp->rsp.info.cli_rsp.o_answer);

					if (buff_len <= 0 || buff_len > 1) {
						/* IPMC Payload rtm downgrade Command failed. Log it. Send the trap */
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: AVM_FUND_PLD_RTM_IPMC_DNGD_CMD failed - unknown resp length",
							 ent_info->ep_str.name);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
						cb->upgrade_error_type = GEN_ERROR;
						cb->upgrade_module = IPMC_PLD_RTM;
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);

						ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
						m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
									       AVM_CKPT_ENT_UPGD_STATE_CHG);
						ncshm_give_hdl(resp->i_usr_key);
						return NCSCC_RC_FAILURE;
					}

					if ((buff_ptr = (uns8 *)m_MMGR_DATA_AT_START(resp->rsp.info.cli_rsp.o_answer,
										     buff_len,
										     local_buf)) == (uns8 *)0) {
						/* Error handling TBD - JPL */
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload %s: AVM_FUND_PLD_RTM_IPMC_DNGD_CMD: Failed: BUFF Data Extract",
							 ent_info->ep_str.name);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);

						ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
						m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
									       AVM_CKPT_ENT_UPGD_STATE_CHG);
						ncshm_give_hdl(resp->i_usr_key);
						return NCSCC_RC_FAILURE;
					}
					pld_rtm_status = buff_ptr[0];

					if ((pld_rtm_status >= GEN_ERROR) && (pld_rtm_status <= UNKNOWN_RTM_TYPE)) {
						cb->upgrade_error_type = pld_rtm_status;
						cb->upgrade_module = IPMC_PLD_RTM;
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: Payload RTM IPMC downgrade failed: ErrorType=%d ",
							 ent_info->ep_str.name, cb->upgrade_error_type);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);

						ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
						m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
									       AVM_CKPT_ENT_UPGD_STATE_CHG);
						ncshm_give_hdl(resp->i_usr_key);
						return NCSCC_RC_FAILURE;
					}
					if (pld_rtm_status == PAYLOAD_IPMC_UPGD_SUCCESS) {
						/* ent_info->dhcp_serv_conf.pld_bld_ipmc_status = pld_bld_status;  - TBD - JPL */
						cb->upgrade_prg_evt = AVM_UPGRADE_SUCCESS;
						cb->upgrade_module = IPMC_PLD_RTM;
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: IPMC PLD RTM Downgrade Success: Progress Tpye=%d ",
							 ent_info->ep_str.name, cb->upgrade_prg_evt);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmUpgradeProgress_ID);	/* sending trap TBD - JPL */
						unlock = 1;
					} else if ((pld_rtm_status >= IPMC_SAME_VERSION)
						   && (pld_rtm_status <= PAYLOAD_RTM_NOT_PRESENT)) {
						/* shouldn't fall into this case - TBD - JPL */
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: IPMC PLD RTM - Rollback not required ",
							 ent_info->ep_str.name);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
						unlock = 1;
					} else {
						/* AVM doesn't know, what happened at FUND */
						cb->upgrade_error_type = UNKNOWN_ERROR;
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: Payload RTM IPMC Rollback failed: ErrorType=%d ",
							 ent_info->ep_str.name, cb->upgrade_error_type);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
						cb->upgrade_module = IPMC_PLD_RTM;
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);

						ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
						m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
									       AVM_CKPT_ENT_UPGD_STATE_CHG);
						ncshm_give_hdl(resp->i_usr_key);
						return NCSCC_RC_FAILURE;
					}
				}
				break;

			case AVM_FUND_PLD_BLD_AND_RTM_IPMC_DNGD_CMD:
				{
					uns32 buff_len = 0;
					uns8 pld_bld_status = 0, pld_rtm_status = 0;

					/* Stop the timer */
					avm_stop_tmr(cb, &ent_info->ipmc_tmr);

					/* Get the size of the buffer */
					buff_len = m_MMGR_LINK_DATA_LEN(resp->rsp.info.cli_rsp.o_answer);

					if (buff_len <= 0 || buff_len > 2) {
						/* IPMC DOWNGRADE Command failed. Log it. Send the trap */
						snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
							 "AVM-SSU: Payload blade %s: AVM_FUND_PLD_BLD_AND_RTM_IPMC_DNGD_CMD failed - unknown resp length",
							 ent_info->ep_str.name);
						m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
						cb->upgrade_error_type = GEN_ERROR;
						cb->upgrade_module = IPMC_PLD;
						avm_send_boot_upgd_trap(cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);
						failure = 1;
					} else {

						if ((buff_ptr =
						     (uns8 *)m_MMGR_DATA_AT_START(resp->rsp.info.cli_rsp.o_answer,
										  buff_len, local_buf)) == (uns8 *)0) {
							/* Error handling TBD - JPL */
							snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
								 "AVM-SSU: Payload %s: AVM_FUND_PLD_BLD_AND_RTM_IPMC_DNGD_CMD: Failed: BUFF Data Extract",
								 ent_info->ep_str.name);
							m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);

							ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
							m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info,
										       AVM_CKPT_ENT_UPGD_STATE_CHG);
							ncshm_give_hdl(resp->i_usr_key);
							return NCSCC_RC_FAILURE;
						}
						if (buff_len == 1) {
							/* Only if the Payload blade IPMC downgrade is failed, the buff_len will be 1 */
							pld_bld_status = buff_ptr[0];

							if (pld_bld_status >= GEN_ERROR
							    && pld_bld_status <= FUND_TFTP_ERROR)
								cb->upgrade_error_type = pld_bld_status;
							else
								cb->upgrade_error_type = UNKNOWN_ERROR;

							snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
								 "AVM-SSU: Payload blade %s: Payload blade IPMC Rollback failed: ErrorType=%d ",
								 ent_info->ep_str.name, cb->upgrade_error_type);
							m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
							cb->upgrade_module = IPMC_PLD_BLD;
							avm_send_boot_upgd_trap(cb, ent_info,
										ncsAvmSwFwUpgradeFailure_ID);
							failure = 1;
						} else {
							pld_bld_status = buff_ptr[0];
							pld_rtm_status = buff_ptr[1];

							if ((pld_bld_status == IPMC_SAME_VERSION)
							    || (pld_bld_status == PAYLOAD_IPMC_UPGD_SUCCESS)) {
								if (pld_bld_status == IPMC_SAME_VERSION) {
									cb->upgrade_prg_evt = AVM_SAME_VERSION;
								} else
									cb->upgrade_prg_evt = AVM_UPGRADE_SUCCESS;

								cb->upgrade_module = IPMC_PLD_BLD;
								snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
									 "AVM-SSU: Payload blade %s: IPMC PLD BLD Rollback Success: Progress Type=%d ",
									 ent_info->ep_str.name, cb->upgrade_prg_evt);
								m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
								avm_send_boot_upgd_trap(cb, ent_info,
											ncsAvmUpgradeProgress_ID);
								unlock = 1;	/* TBD - JPL */
							} else {
								if (pld_bld_status >= GEN_ERROR
								    && pld_bld_status <= FUND_TFTP_ERROR)
									cb->upgrade_error_type = pld_bld_status;
								else
									cb->upgrade_error_type = UNKNOWN_ERROR;

								cb->upgrade_module = IPMC_PLD_BLD;
								snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
									 "AVM-SSU: Payload blade %s: Payload blade IPMC Rollback failed: ErrorType=%d ",
									 ent_info->ep_str.name, cb->upgrade_error_type);
								m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
								avm_send_boot_upgd_trap(cb, ent_info,
											ncsAvmSwFwUpgradeFailure_ID);
								failure = 1;
							}

							if ((pld_rtm_status >= GEN_ERROR)
							    && (pld_rtm_status <= UNKNOWN_RTM_TYPE)) {
								cb->upgrade_error_type = pld_rtm_status;
								cb->upgrade_module = IPMC_PLD_RTM;
								snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
									 "AVM-SSU: Payload blade %s: Payload RTM IPMC Rollback failed: ErrorType=%d ",
									 ent_info->ep_str.name, cb->upgrade_error_type);
								m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
								avm_send_boot_upgd_trap(cb, ent_info,
											ncsAvmSwFwUpgradeFailure_ID);
								failure = 1;
							} else if ((pld_rtm_status >= IPMC_SAME_VERSION)
								   && (pld_rtm_status <= PAYLOAD_RTM_NOT_PRESENT)) {
								if (pld_rtm_status == PAYLOAD_IPMC_UPGD_SUCCESS) {
									cb->upgrade_prg_evt = AVM_UPGRADE_SUCCESS;
									cb->upgrade_module = IPMC_PLD_RTM;
									snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
										 "AVM-SSU: Payload blade %s: IPMC PLD RTM Rollback Success:Progress Type=%d",
										 ent_info->ep_str.name,
										 cb->upgrade_prg_evt);
									m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
									avm_send_boot_upgd_trap(cb, ent_info,
												ncsAvmUpgradeProgress_ID);
								} else {
									/* it shouldn't fall into this case */
									snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
										 "AVM-SSU: Payload blade %s: IPMC PLD RTM Rollback Success:Progress Type=%d",
										 ent_info->ep_str.name,
										 cb->upgrade_prg_evt);
									m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
								}
								unlock = 1;
							} else {
								cb->upgrade_error_type = UNKNOWN_ERROR;
								cb->upgrade_module = IPMC_PLD_RTM;
								snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
									 "AVM-SSU: Payload blade %s: Payload RTM IPMC Rollback failed: ErrorType=%d ",
									 ent_info->ep_str.name, cb->upgrade_error_type);
								m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
								avm_send_boot_upgd_trap(cb, ent_info,
											ncsAvmSwFwUpgradeFailure_ID);
								failure = 1;
							}
						}
					}
				}
				if (failure == 1) {
					ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
					m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
				}
				break;

			default:
				/* Invalid Command ID. Hence Log/Send Trap */
				snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
					 "AVM-SSU: Payload blade %s: IPMC Invalid CMD :%d", ent_info->ep_str.name,
					 resp->rsp.info.cli_rsp.i_cmnd_id);
				m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
				cb->upgrade_error_type = GEN_ERROR;
				cb->upgrade_module = IPMC_PLD;
				avm_send_boot_upgd_trap(cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);
				break;
			}
		}
		break;
	case NCSCC_RC_FAILURE:
	default:
		{
			/* FUND always sends SUCCESS. Hence Log/Send Trap. Something
			   unknown happened                                           */
			snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload blade %s: IPMC CMD :%d failed",
				 ent_info->ep_str.name, resp->rsp.info.cli_rsp.i_cmnd_id);
			m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
			cb->upgrade_error_type = GEN_ERROR;
			cb->upgrade_module = IPMC_PLD;
			avm_send_boot_upgd_trap(cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);
			ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
			m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
		}
		break;
	}
	/* unlock the payload blade, if the flag is set */
	if (unlock) {
		NCSMIB_ARG ssu_dummy_mib_res;
		AVM_EVT_T fsm_evt;
		uns32 rc;
		/* unlock the target payload blade, after upgrading the IPMC */

		snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload  %s Unlocked : IPMC CMD :%d",
			 ent_info->ep_str.name, resp->rsp.info.cli_rsp.i_cmnd_id);
		m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);

		ent_info->dhcp_serv_conf.ipmc_upgd_state = IPMC_UPGD_DONE;
		fsm_evt.evt.mib_req = &ssu_dummy_mib_res;
		m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);

		fsm_evt.fsm_evt_type = AVM_ADM_UNLOCK + AVM_EVT_ADM_OP_BASE - 1;
		rc = avm_fsm_handler(cb, ent_info, &fsm_evt);
		if (rc != NCSCC_RC_SUCCESS) {
			m_MMGR_FREE_MIB_OCT(fsm_evt.evt.mib_req->rsp.add_info);
		}
		m_AVM_SEND_CKPT_UPDT_ASYNC_ADD(cb, ent_info, AVM_CKPT_ENT_ADM_OP);
	}
	if (failure) {
		ncshm_give_hdl(resp->i_usr_key);
		return NCSCC_RC_FAILURE;
	}
	ncshm_give_hdl(resp->i_usr_key);
	return NCSCC_RC_SUCCESS;
}

extern uns32 avm_proc_ipmc_upgd_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T *avm_cb)
{
	AVM_ENT_INFO_T *ent_info;
	uns8 str[AVM_LOG_STR_MAX_LEN];
	uns32 rc;

	str[0] = '\0';
	m_AVM_LOG_FUNC_ENTRY("avm_proc_ipmc_upgd_tmr_exp");

	/* retrieve AvM CB */
	if (avm_cb == AVM_CB_NULL) {
		m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		return NCSCC_RC_FAILURE;
	}

	/* Take the entity handle to retrive DHCP related stuff */
	if (NULL == (ent_info = (AVM_ENT_INFO_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, my_evt->evt.tmr.ent_hdl))) {
		return NCSCC_RC_FAILURE;
	}

	avm_stop_tmr(avm_cb, &ent_info->ipmc_tmr);

	/* Check.. whether the timer expired while upgrading or rolling back */

	if (ent_info->dhcp_serv_conf.ipmc_upgd_state == IPMC_ROLLBACK_IN_PRG) {
		/* Timer expired while rolling back */
		/* Log it as CRITICAL error and send the trap */
		avm_cb->upgrade_error_type = TIME_OUT;
		avm_cb->upgrade_module = IPMC_PLD;
		snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload blade %s: Payload IPMC Rollback Timedout",
			 ent_info->ep_str.name);
		m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
		avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);
	} else {
      /*****************************************************************/
		/* Timer expired while upgrading the IPMC. Do the following:     */
		/*        - Log the error.                                       */
		/*        - Send the trap.                                       */
		/*        - Rollback the preferred label if required             */
		/*        - Trigger the rollback of IPMC                         */
      /*****************************************************************/

		snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload blade %s: IPMC Upgrade Timedout",
			 ent_info->ep_str.name);
		m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);

		avm_cb->upgrade_error_type = TIME_OUT;
		avm_cb->upgrade_module = IPMC_PLD;
		avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);

		/* Check the type of upgrade */
		if (INTEG == ent_info->dhcp_serv_conf.upgrade_type) {
			/* if the type is INTEG */
			AVM_ENT_DHCP_CONF *dhcp_conf = &ent_info->dhcp_serv_conf;

			if (ent_info->dhcp_serv_conf.curr_act_label->other_label->status != SSU_UPGD_FAILED) {
				/* change the state to upgrade failure */
				ent_info->dhcp_serv_conf.curr_act_label->other_label->status = SSU_UPGD_FAILED;

				/* rollback the preferred label */
				if (dhcp_conf->def_label_num == AVM_DEFAULT_LABEL_1)
					dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_2;
				else
					dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_1;

				dhcp_conf->default_label = dhcp_conf->default_label->other_label;

				/* Set the preferred label also to other label */
				dhcp_conf->pref_label.length = dhcp_conf->default_label->name.length;
				memcpy(dhcp_conf->pref_label.name, dhcp_conf->default_label->name.name,
				       dhcp_conf->pref_label.length);

				dhcp_conf->default_chg = FALSE;
				m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);

				snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
					 "AVM-SSU: Payload blade %s : Preferred label set back to current active label: %s",
					 ent_info->ep_str.name, dhcp_conf->pref_label.name);
				m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
			}
		}

		avm_cb->upgrade_prg_evt = AVM_ROLLBACK_TRIGGERED;
		avm_cb->upgrade_module = IPMC_PLD;
		snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload blade %s: IPMC PLD - Rollback triggered ",
			 ent_info->ep_str.name);
		m_AVM_LOG_DEBUG(str, NCSFL_SEV_NOTICE);
		avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmUpgradeProgress_ID);

		ent_info->dhcp_serv_conf.ipmc_upgd_state = IPMC_ROLLBACK_IN_PRG;
		m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
		m_AVM_UPGD_IPMC_PLD_TMR_START(avm_cb, ent_info);
		/* ncshm_give_hdl ?? - TBD - JPL */
		rc = avm_gen_fund_mib_set(avm_cb, ent_info, AVM_FUND_PLD_BLD_AND_RTM_IPMC_DNGD_CMD,
					  ent_info->dhcp_serv_conf.curr_act_label->file_name.name);
		ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
		return rc;
	}
	ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
	return NCSCC_RC_SUCCESS;
}

extern uns32 avm_proc_ipmc_mod_upgd_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T *avm_cb)
{
	AVM_ENT_INFO_T *ent_info;
	uns8 str[AVM_LOG_STR_MAX_LEN];

	str[0] = '\0';
	m_AVM_LOG_FUNC_ENTRY("avm_proc_ipmc_mod_upgd_tmr_exp");

	/* retrieve AvM CB */
	if (avm_cb == AVM_CB_NULL) {
		m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		return NCSCC_RC_FAILURE;
	}

	/* Take the entity handle to retrive DHCP related stuff */
	if (NULL == (ent_info = (AVM_ENT_INFO_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, my_evt->evt.tmr.ent_hdl))) {
		return NCSCC_RC_FAILURE;
	}

	avm_stop_tmr(avm_cb, &ent_info->ipmc_mod_tmr);

	/* Check whether the timer expired for IPMC PLD BLD or IPMC PLD RTM */
	if (ent_info->dhcp_serv_conf.ipmc_upgd_state == IPMC_PLD_BLD_ROLLBACK_IN_PRG) {
		/* Timer expired while rolling back IPMC PLD BLD */
		/* Log it as CRITICAL error and send the trap */
		avm_cb->upgrade_error_type = TIME_OUT;
		avm_cb->upgrade_module = IPMC_PLD_BLD;
		snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload blade %s: IPMC_PLD_BLD Rollback Timedout",
			 ent_info->ep_str.name);
		m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
		avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);
	} else if (ent_info->dhcp_serv_conf.ipmc_upgd_state == IPMC_PLD_RTM_ROLLBACK_IN_PRG) {
		/* Timer expired while rolling back IPMC PLD RTM */
		/* Log it as CRITICAL error and send the trap */
		avm_cb->upgrade_error_type = TIME_OUT;
		avm_cb->upgrade_module = IPMC_PLD_RTM;
		snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload blade %s: IPMC_PLD_RTM Rollback Timedout",
			 ent_info->ep_str.name);
		m_AVM_LOG_DEBUG(str, NCSFL_SEV_CRITICAL);
		avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);
	} else {
		/* TBD - JPL */
	}
	ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
	return NCSCC_RC_SUCCESS;
}

uns32 avm_proc_role_chg_wait_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T *avm_cb)
{
	AVM_ENT_INFO_T *ent_info;
	uns8 str[AVM_LOG_STR_MAX_LEN];
	uns32 rc = NCSCC_RC_SUCCESS;

	str[0] = '\0';
	m_AVM_LOG_FUNC_ENTRY("avm_proc_role_chg_wait_tmr_exp");

	/* retrieve AvM CB */
	if (avm_cb == AVM_CB_NULL) {
		m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		return NCSCC_RC_FAILURE;
	}

	/* Take the entity handle to retrive DHCP related stuff */
	if (NULL == (ent_info = (AVM_ENT_INFO_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, my_evt->evt.tmr.ent_hdl))) {
		return NCSCC_RC_FAILURE;
	}

	avm_stop_tmr(avm_cb, &ent_info->role_chg_wait_tmr);

	snprintf(str, AVM_LOG_STR_MAX_LEN - 1, "AVM-SSU: Payload blade %s: Role change timer expired: Ipmc state: %d",
		 ent_info->ep_str.name, ent_info->dhcp_serv_conf.ipmc_upgd_state);
	m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);

	/* Check the state of ipmc upgrade and take appropriate action */
	switch (ent_info->dhcp_serv_conf.ipmc_upgd_state) {
	case IPMC_UPGD_IN_PRG:
		{
			m_AVM_UPGD_IPMC_PLD_TMR_START(avm_cb, ent_info);
			rc = avm_gen_fund_mib_set(avm_cb, ent_info, AVM_FUND_PLD_BLD_AND_RTM_IPMC_UPGD_CMD,
						  ent_info->dhcp_serv_conf.curr_act_label->other_label->file_name.name);
			if (rc != NCSCC_RC_SUCCESS) {
				if (INTEG == ent_info->dhcp_serv_conf.upgrade_type) {

					snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
						 "AVM-SSU: Payload %s: Role change timer expired: INTEG Failure ",
						 ent_info->ep_str.name);
					m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);

					AVM_ENT_DHCP_CONF *dhcp_conf = &ent_info->dhcp_serv_conf;
					ent_info->dhcp_serv_conf.curr_act_label->other_label->status = SSU_UPGD_FAILED;

					m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label1.status,
								ncsAvmEntDHCPConfLabel1Status_ID, ent_info);
					m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label2.status,
								ncsAvmEntDHCPConfLabel2Status_ID, ent_info);

					if (dhcp_conf->def_label_num == AVM_DEFAULT_LABEL_1)
						dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_2;
					else
						dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_1;

					dhcp_conf->default_label = dhcp_conf->default_label->other_label;
					dhcp_conf->pref_label.length = dhcp_conf->default_label->name.length;
					memcpy(dhcp_conf->pref_label.name, dhcp_conf->default_label->name.name,
					       dhcp_conf->pref_label.length);

					m_AVM_SSU_PSSV_PUSH_STR(avm_cb, ent_info->dhcp_serv_conf.pref_label.name,
								ncsAvmEntDHCPConfPrefLabel_ID,
								ent_info, ent_info->dhcp_serv_conf.pref_label.length);

					dhcp_conf->default_chg = FALSE;
					m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);

					ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
					m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
				} else if (IPMC == ent_info->dhcp_serv_conf.upgrade_type) {
					snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
						 "AVM-SSU: Payload %s: Role change timer expired: IPMC Failure ",
						 ent_info->ep_str.name);
					m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);

					ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
					m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
				}
			}
			ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
			return rc;
		}
		break;

	case IPMC_ROLLBACK_IN_PRG:
		{
			m_AVM_UPGD_IPMC_PLD_TMR_START(avm_cb, ent_info);
			rc = avm_gen_fund_mib_set(avm_cb, ent_info, AVM_FUND_PLD_BLD_AND_RTM_IPMC_DNGD_CMD,
						  ent_info->dhcp_serv_conf.curr_act_label->file_name.name);
			ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
			return rc;
		}
		break;

	case IPMC_PLD_BLD_ROLLBACK_IN_PRG:
		{
			m_AVM_UPGD_IPMC_PLD_MOD_TMR_START(avm_cb, ent_info);
			rc = avm_gen_fund_mib_set(avm_cb, ent_info, AVM_FUND_PLD_BLD_IPMC_DNGD_CMD,
						  ent_info->dhcp_serv_conf.curr_act_label->file_name.name);
			ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
			return rc;
		}
		break;

	case IPMC_PLD_RTM_ROLLBACK_IN_PRG:
		{
			m_AVM_UPGD_IPMC_PLD_MOD_TMR_START(avm_cb, ent_info);
			rc = avm_gen_fund_mib_set(avm_cb, ent_info, AVM_FUND_PLD_RTM_IPMC_DNGD_CMD,
						  ent_info->dhcp_serv_conf.curr_act_label->file_name.name);
			ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
			return rc;
		}
		break;

	default:
		{
			snprintf(str, AVM_LOG_STR_MAX_LEN - 1,
				 "AVM-SSU: Payload blade %s: role change timer expired at wrong ipmc state - DEBUG it ",
				 ent_info->ep_str.name);
			m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);
			ncshm_give_hdl(my_evt->evt.tmr.ent_hdl);
			return NCSCC_RC_FAILURE;
		}
	}
}
