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

  MODULE NAME: DTS_MAPI.H

  $Header: 


..............................................................................

  DESCRIPTION: Contains a definition of all the MIB tables supported, and a 
            list of the relevant table access subroutines.


******************************************************************************
*/
#ifndef DTS_MAPI_H
#define DTS_MAPI_H


/******************************************************************************
                           Global Policy Table
******************************************************************************/
typedef enum dtsv_global_policy_obj_id
{
   ncsDtsvGlobalMessageLogging_ID = 1,
   ncsDtsvGlobalLogDevice_ID,
   ncsDtsvGlobalLogFileSize_ID,
   ncsDtsvGlobalFileLogCompFormat_ID,
   ncsDtsvGlobalCircularBuffSize_ID,
   ncsDtsvGlobalCirBuffCompFormat_ID,
   ncsDtsvGlobalLoggingState_ID,
   ncsDtsvGlobalCategoryBitMap_ID,
   ncsDtsvGlobalSeverityBitMap_ID,
   ncsDtsvGlobalNumOfLogFiles_ID,
   ncsDtsvGlobalLogMsgSequencing_ID,
   ncsDtsvGlobalCloseOpenFiles_ID,

   ncsDtsvScalarsMax_ID
} DTSV_GLOBAL_POLICY_OBJ_ID;

/******************************************************************************
                           Node Policy Table
******************************************************************************/
typedef enum dtsv_node_policy_obj_id
{
   ncsDtsvNodeIndexNode_ID = 1,
   ncsDtsvNodeRowStatus_ID,
   ncsDtsvNodeMessageLogging_ID,
   ncsDtsvNodeLogDevice_ID,
   ncsDtsvNodeLogFileSize_ID,
   ncsDtsvNodeFileLogCompFormat_ID,
   ncsDtsvNodeCircularBuffSize_ID,
   ncsDtsvNodeCirBuffCompFormat_ID,
   ncsDtsvNodeLoggingState_ID,
   ncsDtsvNodeCategoryBitMap_ID,
   ncsDtsvNodeSeverityBitMap_ID,

   ncsDtsvNodeLogPolicyEntryMax_ID
} DTSV_NODE_POLICY_OBJ_ID;

/******************************************************************************
                           Service Policy Table
******************************************************************************/
typedef enum dtsv_svc_policy_obj_id
{
   ncsDtsvServiceIndexNode_ID = 1,
   ncsDtsvServiceIndexService_ID,
   ncsDtsvServiceRowStatus_ID,
   ncsDtsvServiceLogDevice_ID,
   ncsDtsvServiceLogFileSize_ID,
   ncsDtsvServiceFileLogCompFormat_ID,
   ncsDtsvServiceCircularBuffSize_ID,
   ncsDtsvServiceCirBuffCompFormat_ID,
   ncsDtsvServiceLoggingState_ID,
   ncsDtsvServiceCategoryBitMap_ID,
   ncsDtsvServiceSeverityBitMap_ID,

   ncsDtsvServiceLogPolicyEntryMax_ID
} DTSV_SVC_POLICY_OBJ_ID;

/******************************************************************************
                    CIRCULAR BUFFER OPERATION TABLE
******************************************************************************/
typedef enum dtsv_cir_buff_op_obj_id
{
   dtsvCirbuffOpIndexNode_Id = 1,
   dtsvCirbuffOpIndexService_Id,
   dtsvCirbuffOpOperation,
   dtsvCirbuffOpDevice,

   dtsvCirBuffOpMax_Id
} DTSV_CIR_BUFF_OP_OBJ_ID;

/******************************************************************************
                    MASv CLI COMMAND TABLE
******************************************************************************/
typedef enum dtsv_cmd_tbl_id
{
   dtsvAddGlobalConsole = 1,
   dtsvRmvGlobalConsole,
   dtsvDispGlobalConsole,
   dtsvAddNodeConsole,
   dtsvRmvNodeConsole,
   dtsvDispNodeConsole,
   dtsvAddSvcConsole,
   dtsvRmvSvcConsole,
   dtsvDispSvcConsole,
   dtsvAsciiSpecReload,
   dtsvPrintCurrConfig,

   dtsvCmdMax
}DTSV_CMD_TBL_ID;


EXTERN_C uns32 
ncsdtsvscalars_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSMIB_VAR_INFO *var_info, NCS_BOOL flag);
EXTERN_C uns32 
ncsdtsvscalars_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

EXTERN_C uns32 
ncsdtsvscalars_next(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data, 
                   uns32 *next_inst_id, uns32 *next_inst_len);
EXTERN_C uns32 
ncsdtsvscalars_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                     NCSMIB_SETROW_PARAM_VAL* params,
                     struct ncsmib_obj_info* objinfo,
                     NCS_BOOL flag);
EXTERN_C uns32
ncsdtsvscalars_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);
EXTERN_C uns32
ncsdtsvscalars_extract(NCSMIB_PARAM_VAL *param_val,
                       NCSMIB_VAR_INFO *var_info,
                       NCSCONTEXT data,
                       NCSCONTEXT buffer);

/* Defines for node related mib functions */
EXTERN_C uns32 
ncsdtsvnodelogpolicyentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSMIB_VAR_INFO *var_info, NCS_BOOL flag);
EXTERN_C uns32 
ncsdtsvnodelogpolicyentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

EXTERN_C uns32 
ncsdtsvnodelogpolicyentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data, 
                   uns32 *next_inst_id, uns32 *next_inst_len);
EXTERN_C uns32 
ncsdtsvnodelogpolicyentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                     NCSMIB_SETROW_PARAM_VAL* params,
                     struct ncsmib_obj_info* objinfo,
                     NCS_BOOL flag);
EXTERN_C uns32
ncsdtsvnodelogpolicyentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);
EXTERN_C uns32
ncsdtsvnodelogpolicyentry_extract(NCSMIB_PARAM_VAL *param_val,
                                  NCSMIB_VAR_INFO *var_info,
                                  NCSCONTEXT data,
                                  NCSCONTEXT buffer);

/* Service level routines for this table */
EXTERN_C uns32 
ncsdtsvservicelogpolicyentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSMIB_VAR_INFO *var_info, NCS_BOOL flag);
EXTERN_C uns32 
ncsdtsvservicelogpolicyentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

EXTERN_C uns32 
ncsdtsvservicelogpolicyentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data, 
                   uns32 *next_inst_id, uns32 *next_inst_len);
EXTERN_C uns32 
ncsdtsvservicelogpolicyentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                     NCSMIB_SETROW_PARAM_VAL* params,
                     struct ncsmib_obj_info* objinfo,
                     NCS_BOOL flag);
EXTERN_C uns32
ncsdtsvservicelogpolicyentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);
EXTERN_C uns32
ncsdtsvservicelogpolicyentry_extract(NCSMIB_PARAM_VAL *param_val,
                                  NCSMIB_VAR_INFO *var_info,
                                  NCSCONTEXT data,
                                  NCSCONTEXT buffer);


EXTERN_C uns32 
dtsv_extract_policy_obj(NCSMIB_PARAM_VAL *param_val, NCSMIB_VAR_INFO *var_info,
                      NCSCONTEXT data, NCSCONTEXT buffer);


#endif
