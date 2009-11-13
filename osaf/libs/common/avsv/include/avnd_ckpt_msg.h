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

  DESCRIPTION:

  This include file contains the message definitions for chepointing data from 
  Active to Standby AVND.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVND_CKPT_MSG_H
#define AVND_CKPT_MSG_H

typedef enum avnd_ckpt_msg_reo_type {
	/* 
	 * Messages that update entire data structure and all the 
	 * config fields of that structure. All these message ID's
	 * are also used for sending cold sync updates.
	 */
	AVND_CKPT_HLT_CONFIG,
	AVND_CKPT_SU_CONFIG,
	AVND_CKPT_COMP_CONFIG,
	AVND_CKPT_SU_SI_REC,
	AVND_CKPT_SIQ_REC,
	AVND_CKPT_CSI_REC = 5,
	AVND_CKPT_COMP_HLT_REC,
	AVND_CKPT_COMP_CBK_REC,
	AVND_COLD_SYNC_RSP_ASYNC_UPDT_CNT = 8,
	/* Cold sync update reo types are till here */
	AVND_COLD_SYNC_UPDT_MAX = AVND_COLD_SYNC_RSP_ASYNC_UPDT_CNT,

	/* 
	 * Messages to update independent fields.
	 */

	/* Health Check Config Async Update messages */
	AVND_CKPT_HC_PERIOD = AVND_COLD_SYNC_UPDT_MAX,
	AVND_CKPT_HC_MAX_DUR,

	/* SU Async Update messages */
	AVND_CKPT_SU_FLAG_CHANGE = 10,
	AVND_CKPT_SU_ERR_ESC_LEVEL,
	AVND_CKPT_SU_COMP_RESTART_PROB,
	AVND_CKPT_SU_COMP_RESTART_MAX,
	AVND_CKPT_SU_RESTART_PROB,
	AVND_CKPT_SU_RESTART_MAX,
	AVND_CKPT_SU_COMP_RESTART_CNT,
	AVND_CKPT_SU_RESTART_CNT,
	AVND_CKPT_SU_ERR_ESC_TMR,
	AVND_CKPT_SU_OPER_STATE,
	AVND_CKPT_SU_PRES_STATE = 20,

	/* Component Async Update messages */
	AVND_CKPT_COMP_FLAG_CHANGE = 21,
	AVND_CKPT_COMP_REG_HDL,
	AVND_CKPT_COMP_REG_DEST,
	AVND_CKPT_COMP_OPER_STATE,
	AVND_CKPT_COMP_PRES_STATE,
	AVND_CKPT_COMP_TERM_CBK_TIMEOUT,
	AVND_CKPT_COMP_CSI_SET_CBK_TIMEOUT,
	AVND_CKPT_COMP_QUIES_CMPLT_CBK_TIMEOUT,
	AVND_CKPT_COMP_CSI_RMV_CBK_TIMEOUT,
	AVND_CKPT_COMP_PXIED_INST_CBK_TIMEOUT = 30,
	AVND_CKPT_COMP_PXIED_CLEAN_CBK_TIMEOUT,
	AVND_CKPT_COMP_ERR_INFO,
	AVND_CKPT_COMP_DEFAULT_RECVR,
	AVND_CKPT_COMP_PEND_EVT,
	AVND_CKPT_COMP_ORPH_TMR,
	AVND_CKPT_COMP_NODE_ID,
	AVND_CKPT_COMP_TYPE,
	AVND_CKPT_COMP_MDS_CTXT,
	AVND_CKPT_COMP_REG_RESP_PENDING,
	AVND_CKPT_COMP_INST_CMD = 40,
	AVND_CKPT_COMP_TERM_CMD,
	AVND_CKPT_COMP_INST_TIMEOUT,
	AVND_CKPT_COMP_TERM_TIMEOUT,
	AVND_CKPT_COMP_INST_RETRY_MAX,
	AVND_CKPT_COMP_INST_RETRY_CNT,
	AVND_CKPT_COMP_EXEC_CMD,
	AVND_CKPT_COMP_CMD_EXEC_CTXT,
	AVND_CKPT_COMP_INST_CMD_TS,
	AVND_CKPT_COMP_CLC_REG_TMR,
	AVND_CKPT_COMP_INST_CODE_RCVD = 50,
	AVND_CKPT_COMP_PROXY_PROXIED_ADD,
	AVND_CKPT_COMP_PROXY_PROXIED_DEL,

	/*  SU SI RECORD Async Update messages */
	AVND_CKPT_SU_SI_REC_CURR_STATE = 53,
	AVND_CKPT_SU_SI_REC_PRV_STATE,
	AVND_CKPT_SU_SI_REC_CURR_ASSIGN_STATE,
	AVND_CKPT_SU_SI_REC_PRV_ASSIGN_STATE,

	/* CSI REC Async Update messages */
	AVND_CKPT_COMP_CSI_ACT_COMP_NAME = 57,
	AVND_CKPT_COMP_CSI_TRANS_DESC,
	AVND_CKPT_COMP_CSI_STANDBY_RANK,
	AVND_CKPT_COMP_CSI_CURR_ASSIGN_STATE = 60,
	AVND_CKPT_COMP_CSI_PRV_ASSIGN_STATE,

	/* Comp Health Check Async Update messages */
	AVND_CKPT_COMP_HC_REC_STATUS,
	AVND_CKPT_COMP_HC_REC_TMR,

	/* Comp Callback Record Async Update messages */
	AVND_CKPT_COMP_CBK_REC_AMF_HDL = 64,
	AVND_CKPT_COMP_CBK_REC_MDS_DEST,
	AVND_CKPT_COMP_CBK_REC_TMR,
	AVND_CKPT_COMP_CBK_REC_TIMEOUT,

	AVND_CKPT_MSG_MAX = 68,

	/* Sync commit message should be last one */
	AVND_SYNC_COMMIT = AVND_CKPT_MSG_MAX
} AVND_CKPT_MSG_REO_TYPE;

#if (NCS_AVND_MBCSV_CKPT == 1)
/* Macros for sending Async updates to standby */
#define m_AVND_SEND_CKPT_UPDT_ASYNC(cb, act, r_hdl, r_type) \
              avnd_send_ckpt_data(cb, act, NCS_PTR_TO_UNS64_CAST(r_hdl), r_type, NCS_MBCSV_SND_USR_ASYNC)

#define m_AVND_SEND_CKPT_UPDT_ASYNC_ADD(cb, r_hdl, r_type) \
        if(NCSCC_RC_SUCCESS == avnd_ckpt_for_ext(cb, NCS_PTR_TO_UNS64_CAST(r_hdl), r_type)) \
              m_AVND_SEND_CKPT_UPDT_ASYNC(cb, NCS_MBCSV_ACT_ADD, r_hdl, r_type)

#define m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, r_hdl, r_type) \
        if(NCSCC_RC_SUCCESS == avnd_ckpt_for_ext(cb, NCS_PTR_TO_UNS64_CAST(r_hdl), r_type)) \
              m_AVND_SEND_CKPT_UPDT_ASYNC(cb, NCS_MBCSV_ACT_RMV, r_hdl, r_type)

#define m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, r_hdl, r_type) \
        if(NCSCC_RC_SUCCESS == avnd_ckpt_for_ext(cb, NCS_PTR_TO_UNS64_CAST(r_hdl), r_type)) \
              m_AVND_SEND_CKPT_UPDT_ASYNC(cb, NCS_MBCSV_ACT_UPDATE, r_hdl, r_type)

/* Macro for sending Sync update to Standby */
#define m_AVND_SEND_CKPT_UPDT_SYNC(cb, act, r_hdl) \
              avnd_send_ckpt_data(cb, act, NCS_PTR_TO_UNS64_CAST(r_hdl), AVND_SYNC_COMMIT, NCS_MBCSV_SND_SYNC)
#else
/* Macros for sending Async updates to standby */
#define m_AVND_SEND_CKPT_UPDT_ASYNC(cb, act, r_hdl, r_type)

#define m_AVND_SEND_CKPT_UPDT_ASYNC_ADD(cb, r_hdl, r_type)

#define m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, r_hdl, r_type)

#define m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, r_hdl, r_type)

/* Macro for sending Sync update to Standby */
#define m_AVND_SEND_CKPT_UPDT_SYNC(cb, act, r_hdl)
#endif

#endif
