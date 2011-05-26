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
  Active to Standby DTS.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef DTS_CKPT_MSG_H
#define DTS_CKPT_MSG_H

typedef enum dtsv_ckpt_msg_reo_type {
	/* 
	 * Messages that update entire data structure and all the 
	 * config fields of that structure. All these message ID's
	 * are also used for sending cold sync updates.
	 */
	DTSV_CKPT_DTS_SVC_REG_TBL_CONFIG = 1,
	DTSV_CKPT_DTA_DEST_LIST_CONFIG,
	DTSV_CKPT_GLOBAL_POLICY_CONFIG,
	DTSV_CKPT_DTS_LOG_FILE_CONFIG,
	DTSV_CKPT_MSG_MAX
} DTSV_CKPT_MSG_REO_TYPE;

/* 
 * Async update count. It will be used for warm sync verification.
 */
typedef struct dtsv_async_updt_cnt {
	uint32_t dts_svc_reg_tbl_updt;
	uint32_t dta_dest_list_updt;
	uint32_t global_policy_updt;
	uint32_t dts_log_updt;
} DTSV_ASYNC_UPDT_CNT;

/* Macros for sending Async updates to standby */
#define DTSV_COLD_SYNC_RSP_ASYNC_UPDT_CNT (DTSV_CKPT_GLOBAL_POLICY_CONFIG + 1)
#define m_DTSV_SEND_CKPT_UPDT_ASYNC(cb, act, r_hdl, r_type) \
              dtsv_send_ckpt_data(cb, act, r_hdl, r_type, NCS_MBCSV_SND_USR_ASYNC)

#endif
