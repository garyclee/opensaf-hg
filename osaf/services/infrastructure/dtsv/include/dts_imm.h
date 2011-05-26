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

#ifndef DTS_IMM_H
#define DTS_IMM_H

#include "dts.h"
#include "saImm.h"
#include "immutil.h"
#include "saImmOm.h"
#include "saImmOi.h"

/* IMMSv Defs */
#define DTS_IMM_RELEASE_CODE 'A'
#define DTS_IMM_MAJOR_VERSION 0x02
#define DTS_IMM_MINOR_VERSION 0x01
#define IMPLEMENTER_NAME "OpenSAFDtsvService"

/* DTSv Defs */                                   
#define DTS_GLOBAL_LOG_POLICY "opensafGlobalLogPolicy=safDtsv,safApp=safDtsvService"
#define DTS_APP_OBJ "safApp=safDtsvService"
#define DTS_GLOBAL_CLASS_NAME "OpenSAFDtsvGlobalLogPolicy"
#define DTS_NODE_CLASS_NAME "OpenSAFDtsvNodeLogPolicy"
#define DTS_SERVICE_CLASS_NAME "OpenSAFDtsvServiceLogPolicy"

/* function prototypes */
void dts_imm_declare_implementer(DTS_CB *cb);
SaAisErrorT dts_imm_initialize(DTS_CB *cb);
void dts_imm_reinit_bg(DTS_CB * cb);
SaAisErrorT dts_read_log_policies(char *className);
SaAisErrorT dts_saImmOiImplementerClear(SaImmOiHandleT immOiHandle);
unsigned int dts_parse_node_policy_DN(char *objName, SVC_KEY *key);
unsigned int dts_parse_service_policy_DN(char *objName, SVC_KEY *key);
unsigned int dts_configure_global_policy();
unsigned int dtsv_global_filtering_policy_change(DTS_CB *inst, unsigned int param_id);
unsigned int dts_service_log_policy_set(DTS_CB *inst, char *objName, void *attrib_info, enum CcbUtilOperationType UtilOp);
unsigned int dts_node_log_policy_set(DTS_CB *inst, char *objName, void *attrib_info, enum CcbUtilOperationType UtilOp);
unsigned int dts_global_log_policy_set(DTS_CB *inst, struct CcbUtilOperationData *ccbUtilOperationData);

/******************************************************************************
                           Global Policy Table
******************************************************************************/
typedef enum dtsv_global_policy_obj_id {
	osafDtsvGlobalMessageLogging_ID = 1,
	osafDtsvGlobalLogDevice_ID,
	osafDtsvGlobalLogFileSize_ID,
	osafDtsvGlobalFileLogCompFormat_ID,
	osafDtsvGlobalCircularBuffSize_ID,
	osafDtsvGlobalCirBuffCompFormat_ID,
	osafDtsvGlobalLoggingState_ID,
	osafDtsvGlobalCategoryBitMap_ID,
	osafDtsvGlobalSeverityBitMap_ID,
	osafDtsvGlobalNumOfLogFiles_ID,
	osafDtsvGlobalLogMsgSequencing_ID,
	osafDtsvGlobalCloseOpenFiles_ID,
	osafDtsvScalarsMax_ID
} DTSV_GLOBAL_POLICY_OBJ_ID;

/******************************************************************************
                           Node Policy Table
******************************************************************************/
typedef enum dtsv_node_policy_obj_id {
	osafDtsvNodeIndexNode_ID = 1,
	osafDtsvNodeMessageLogging_ID,
	osafDtsvNodeLogDevice_ID,
	osafDtsvNodeLogFileSize_ID,
	osafDtsvNodeFileLogCompFormat_ID,
	osafDtsvNodeCircularBuffSize_ID,
	osafDtsvNodeCirBuffCompFormat_ID,
	osafDtsvNodeLoggingState_ID,
	osafDtsvNodeCategoryBitMap_ID,
	osafDtsvNodeSeverityBitMap_ID,
	osafDtsvNodeLogPolicyEntryMax_ID
} DTSV_NODE_POLICY_OBJ_ID;

/******************************************************************************
                           Service Policy Table
******************************************************************************/
typedef enum dtsv_svc_policy_obj_id {
	osafDtsvServiceIndexNode_ID = 1,
	osafDtsvServiceIndexService_ID,
	osafDtsvServiceLogDevice_ID,
	osafDtsvServiceLogFileSize_ID,
	osafDtsvServiceFileLogCompFormat_ID,
	osafDtsvServiceCircularBuffSize_ID,
	osafDtsvServiceCirBuffCompFormat_ID,
	osafDtsvServiceLoggingState_ID,
	osafDtsvServiceCategoryBitMap_ID,
	osafDtsvServiceSeverityBitMap_ID,
	osafDtsvServiceLogPolicyEntryMax_ID
} DTSV_SVC_POLICY_OBJ_ID;

#endif
