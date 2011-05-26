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

#ifndef EDA_LOG_H
#define EDA_LOG_H

/******************************************************************************
 Logging offset indexes for EDA Headline logging
 ******************************************************************************/
typedef enum eda_hdln_log_flex {
	EDA_ADA_ADEST_CREATE_FAILED,
	EDA_API_MSG_DISPATCH_FAILED,
	EDA_CB_HDL_CREATE_FAILED,
	EDA_CB_HDL_TAKE_FAILED,
	EDA_CHAN_HDL_REC_LOOKUP_FAILED,
	EDA_CHANNEL_HDL_CREATE_FAILED,
	EDA_CHANNEL_HDL_TAKE_FAILED,
	EDA_CLIENT_HDL_CREATE_FAILED,
	EDA_CLIENT_HDL_TAKE_FAILED,
	EDA_EVENT_HDL_CREATE_FAILED,
	EDA_EVENT_HDL_TAKE_FAILED,
	EDA_EVENT_PROCESSING_FAILED,
	EDA_EVT_HDL_CREATE_FAILED,
	EDA_EVT_HDL_TAKE_FAILED,
	EDA_EVT_INST_HDL_CREATE_FAILED,
	EDA_EVT_INST_HDL_TAKE_FAILED,
	EDA_EVT_INST_REC_ADD_FAILED,
	EDA_HDL_REC_ADD_FAILED,
	EDA_HDL_REC_LOOKUP_FAILED,
	EDA_IPC_CREATE_FAILED,
	EDA_IPC_ATTACH_FAILED,
	EDA_MDS_CALLBACK_PROCESS_FAILED,
	EDA_MDS_GET_HANDLE_FAILED,
	EDA_MDS_INIT_FAILED,
	EDA_MDS_SUBSCRIPTION_FAILED,
	EDA_MDS_INSTALL_FAILED,
	EDA_MDS_SUBSCRIBE_FAILED,
	EDA_MDS_UNINSTALL_FAILED,
	EDA_VERSION_INCOMPATIBLE,

	/* Memory allocation failure logs */
	EDA_MEMALLOC_FAILED,
	EDA_FAILURE,
	EDA_SUCCESS,
	EDA_INITIALIZE_FAILURE,
	EDA_INITIALIZE_SUCCESS,
	EDA_SELECTION_OBJ_GET_FAILURE,
	EDA_SELECTION_OBJ_GET_SUCCESS,
	EDA_DISPATCH_FAILURE,
	EDA_DISPATCH_SUCCESS,
	EDA_FINALIZE_FAILURE,
	EDA_FINALIZE_SUCCESS,
	EDA_OPEN_FAILURE,
	EDA_OPEN_SUCCESS,
	EDA_OPEN_ASYNC_FAILURE,
	EDA_OPEN_ASYNC_SUCCESS,
	EDA_CLOSE_FAILURE,
	EDA_CLOSE_SUCCESS,
	EDA_UNLINK_FAILURE,
	EDA_UNLINK_SUCCESS,
	EDA_EVT_ALLOCATE_FAILURE,
	EDA_EVT_ALLOCATE_SUCCESS,
	EDA_EVT_FREE_FAILURE,
	EDA_ATTRIBUTE_SET_FAILURE,
	EDA_ATTRIBUTE_SET_SUCCESS,
	EDA_ATTRIBUTE_GET_FAILURE,
	EDA_ATTRIBUTE_GET_SUCCESS,
	EDA_PATTERN_FREE_FAILURE,
	EDA_PATTERN_FREE_SUCCESS,
	EDA_DATA_GET_FAILURE,
	EDA_DATA_GET_SUCCESS,
	EDA_EVT_PUBLISH_FAILURE,
	EDA_EVT_SUBSCRIBE_FAILURE,
	EDA_EVT_UNSUBSCRIBE_FAILURE,
	EDA_EVT_RETENTION_TIME_CLEAR_FAILURE,
	EDA_LIMIT_GET_FAILURE,
} EDA_HDLN_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum eda_flex_sets {
	EDA_FC_HDLN,
	EDA_FC_HDLNF,
} EDA_FLEX_SETS;

typedef enum eda_log_ids {
	EDA_LID_HDLN,
	EDA_LID_HDLNF,
} EDA_LOG_IDS;

/*****************************************************************************
                          Macros for Logging
*****************************************************************************/

#define m_LOG_EDSV_A(id,category,sev,rc,fname,fno,data) eda_log(id,category,sev,rc,fname,fno,data)
#define m_LOG_EDSV_AF(id,category,sev,rc,fname,fno,data,handle) eda_log_f(id,category,sev,rc,fname,fno,data,handle)

/*****************************************************************************
                       Extern Function Declarations
*****************************************************************************/

void eda_flx_log_reg(void);
void eda_flx_log_dereg(void);
void eda_log(uint8_t id, uns32 category, uint8_t sev, uns32 rc, char *fname, uns32 fno, uns32 dataa);
void eda_log_f(uint8_t id, uns32 category, uint8_t sev, uns32 rc, char *fname, uns32 fno, uns32 dataa, uns64 handle);

#endif
