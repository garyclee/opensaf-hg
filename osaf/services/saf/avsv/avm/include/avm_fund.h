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

#ifndef __NCS_AVM_FUND_H__
#define __NCS_AVM_FUND_H__

enum {
/*
  FUND_F101_BIOS_UPGRADE_CMD = 1,
  FUND_F101_IPMC_UPGRADE_CMD = 2,
  FUND_UPGRADE_MARK_CMD = 3,
  FUND_MARK_CMD = 4,
  FUND_SHOW_CMD = 5,
  FUND_RESET_CMD = 6,
  FUND_PAYLOAD_IPMC_UPGRADE_CMD = 7,
  FUND_PAYLOAD_BIOS_UPGRADE_CMD = 8,
  FUND_PAYLOAD_SHOW_IPMC_CMD = 9
*/
	AVM_FUND_PLD_BLD_AND_RTM_IPMC_UPGD_CMD = 10,
	AVM_FUND_PLD_BLD_AND_RTM_IPMC_DNGD_CMD,
	AVM_FUND_PLD_BLD_IPMC_DNGD_CMD,
	AVM_FUND_PLD_RTM_IPMC_DNGD_CMD
} cmd_ids;

typedef enum {
	GEN_ERROR = 1,
	TIME_OUT,
	FCU_QUERY_NOT_SUPPORTED,
	FCU_QUERY_INVALID_VALUE,
	FCU_QUERY_DEVICE_NOT_FOUND,
	FCU_WRITE_NOT_SUPPORTED,
	FCU_WRITE_INVALID_VALUE,
	FCU_WRITE_DEVICE_NOT_FOUND,
	FCU_WRITE_FAILURE,
	FCU_VERIFY_FAILURE,
	FUND_FCU_BUSY,
	FUND_TFTP_ERROR,
	UNKNOWN_RTM_TYPE,
	UNKNOWN_ERROR
} AVM_UPGD_ERR;

typedef enum {
	IPMC_SAME_VERSION = 126,
	PAYLOAD_IPMC_UPGD_SUCCESS,
	PAYLOAD_RTM_NOT_PRESENT
} AVM_UPGD_PRG;

typedef enum {
	AVM_UPGRADE_SUCCESS = 1,
	AVM_SAME_VERSION,
	AVM_ROLLBACK_TRIGGERED,
	AVM_ROLLBACK_SUCCESS
} AVM_TRAP_UPG_PRG;

/*changed in conformance with 64 bit changes*/
typedef struct {
	uns32 cmd_id;
	uns32 node_num;
	uns32 slot_num;
	uns8 device_name[255];
	uns8 mark_flag;
	uns8 mark_bank_num;
	char filename[255];
	char payload_ipaddr[20];
} AVM_FUND_CMD_PARM;

typedef struct {
	AVM_FUND_CMD_PARM cmd_parm;
	char scxb_ipaddr[20];
/* 7107 Support */
	char payload_ipaddr[20];
} AVM_FUND_USR_BUF;

extern uns32 avm_proc_fund(AVM_EVT_T *fund_resp, AVM_CB_T *avm_cb);

extern uns32 avm_gen_fund_mib_set(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, uns32 cmd_id, uns8 *filename);

extern uns32 avm_fund_send_mib_req(AVM_ENT_INFO_T *ent_info, NCSMIB_ARG *mib_arg);
extern uns32 avm_proc_role_chg_wait_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T *avm_cb);

/*
extern uns32
avm_proc_ipmc_upgd_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T  *avm_cb);

extern uns32
avm_proc_ipmc_mod_upgd_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T  *avm_cb);
*/
extern uns32 avm_proc_role_chg_wait_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T *avm_cb);

uns32 avm_fund_resp_func(NCSMIB_ARG *resp);

#endif   /*__NCS_AVM_FUND_H__*/
