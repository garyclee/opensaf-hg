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
 */

/*
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging/tracing functions.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:

  pdrbd_reg_strings..........Function used for registering the ASCII_SPEC table
  pdrbd_dereg_strings........Function used for de-registering ASCII_SPEC table.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "pdrbd.h"

#if (PDRBD_LOG == 1)

#include "dts_papi.h"

uns32 pdrbd_reg_strings(void);
uns32 pdrbd_log_str_lib_req(NCS_LIB_REQ_INFO *);
uns32 pdrbd_dereg_strings(void);


/******************************************************************************
 Logging stuff for Headlines
 ******************************************************************************/

const NCSFL_STR pdrbd_hdln_set[] =
  {
    { PDRBD_HDLN_PDRBD_CREATE_FAILURE,           "PDRBD creation failed" },
    { PDRBD_HDLN_PDRBD_CREATE_SUCCESS,           "PDRBD creation successful. PDRBD services available now" },
    { PDRBD_HDLN_PDRBD_FLS_REGN_SUCCESS,         "PDRBD registration with FLS successful" },
    { PDRBD_HDLN_PDRBD_PIPE_CREATE_FAILURE,      "PDRBD pipe creation failed" },
    { PDRBD_HDLN_PDRBD_PIPE_CREATE_SUCCESS,      "PDRBD pipe creation successful" },
    { PDRBD_HDLN_PDRBD_PIPE_OPEN_FAILURE,        "PDRBD pipe open failed" },
    { PDRBD_HDLN_PDRBD_PIPE_OPEN_SUCCESS,        "PDRBD pipe open successful" },
    { PDRBD_HDLN_PDRBD_MBOX_CREATE_FAILURE,      "PDRBD mail box creation failed" },
    { PDRBD_HDLN_PDRBD_MBOX_CREATE_SUCCESS,      "PDRBD mail box creation successful" },
    { PDRBD_HDLN_PDRBD_MBOX_ATTACH_FAILURE,      "PDRBD mail box attach failed" },
    { PDRBD_HDLN_PDRBD_MBOX_ATTACH_SUCCESS,      "PDRBD mail box attach successful" },
    { PDRBD_HDLN_PDRBD_THREAD_CREATE_FAILURE,    "PDRBD thread creation failed" },
    { PDRBD_HDLN_PDRBD_THREAD_CREATE_SUCCESS,    "PDRBD thread creation successful" },
    { PDRBD_HDLN_PDRBD_THREAD_START_FAILURE,     "PDRBD thread start failed" },
    { PDRBD_HDLN_PDRBD_THREAD_START_SUCCESS,     "PDRBD thread start successful" },
    { PDRBD_HDLN_NULL_INST,                      "PDRBD null instance" },
    { 0, 0 }
  };

/******************************************************************************
 Logging stuff for Service Provider (AMF)
 ******************************************************************************/

const NCSFL_STR pdrbd_amf_log_set[] =
  {
    { PDRBD_SP_AMF_INSTALL_FAILURE,       "AMF install failed" },
    { PDRBD_SP_AMF_INSTALL_SUCCESS,       "AMF install successful" },
    { PDRBD_SP_AMF_COMP_NAME_GET_FAILURE, "AMF component name get failed" },
    { PDRBD_SP_AMF_COMP_NAME_GET_SUCCESS, "AMF component name get successful" },
    { PDRBD_SP_AMF_COMP_REGN_FAILURE,     "AMF component registration failed" },
    { PDRBD_SP_AMF_COMP_REGN_SUCCESS,     "AMF component registration successful" },
    { PDRBD_SP_AMF_HCHK_START_FAILURE,    "AMF health check start failed" },
    { PDRBD_SP_AMF_HCHK_START_SUCCESS,    "AMF health check start successful" },
    { PDRBD_SP_AMF_GET_SEL_OBJ_FAILURE,   "AMF get select object failed" },
    { PDRBD_SP_AMF_GET_SEL_OBJ_SUCCESS,   "AMF get select object successful" },
    { PDRBD_SP_AMF_DESPATCH_FAILURE,      "AMF despatch failed" },
    { PDRBD_SP_AMF_DESPATCH_SUCCESS,      "AMF despatch successful" },
    { PDRBD_SP_AMF_RESPONSE_FAILURE,      "AMF response failed" },
    { PDRBD_SP_AMF_RESPONSE_SUCCESS,      "AMF response successful" },
    { PDRBD_SP_AMF_UNINSTALL_FAILURE,     "AMF uninstall failed" },
    { PDRBD_SP_AMF_UNINSTALL_SUCCESS,     "AMF uninstall successful" },
    { PDRBD_SP_AMF_COMP_UNREGN_FAILURE,   "AMF component unregistration failed" },
    { PDRBD_SP_AMF_COMP_UNREGN_SUCCESS,   "AMF component unregistration successful" },
    { PDRBD_SP_AMF_ACTIVE_ROLE_ASSIGN,    "AMF assigned Active role" },
    { PDRBD_SP_AMF_STANDBY_ROLE_ASSIGN,   "AMF assigned Standby role" },
    { PDRBD_SP_AMF_QUIESCED_ROLE_ASSIGN,  "AMF assigned Quiesced role" },
    { PDRBD_SP_AMF_INVALID_ROLE_ASSIGN,   "AMF assigned unrecognised role" },
    { PDRBD_SP_AMF_REMOVE_CB_INVOKE,      "AMF invoked remove callback" },
    { PDRBD_SP_AMF_HLTH_CHK_CB_INVOKE,    "AMF invoked health-check callback" },
    { PDRBD_SP_AMF_TERMINATE_CB_INVOKE,   "AMF invoked terminate callback" },
    { 0, 0 }
  };


/******************************************************************************
   Logging stuff for pipe messages
*******************************************************************************/
const NCSFL_STR pdrbdPipeLogSet[] =
  {
    { PDRBD_PIPE_MSG_INCOMPLETE,          "Incomplete message from pipe" },
    { PDRBD_PIPE_SET_PRI_FAILURE,         "Active role set failed" },
    { PDRBD_PIPE_SET_PRI_SUCCESS,         "Active role set successful" },
    { PDRBD_PIPE_SET_SEC_FAILURE,         "Standby role set failed" },
    { PDRBD_PIPE_SET_SEC_SUCCESS,         "Standby role set successful" },
    { PDRBD_PIPE_SET_QUI_FAILURE,         "Quiesced role set failed" },
    { PDRBD_PIPE_SET_QUI_SUCCESS,         "Quiesced role set successful" },
    { PDRBD_PIPE_DRBDADM_NOT_FND,         "drbdadm binary not found" },
    { PDRBD_PIPE_DRBD_MOD_LOD_FAILURE,    "drbd module load error or not found" },
    { PDRBD_PIPE_PRTN_MNT_FAILURE,        "Partition mount failed" },
    { PDRBD_PIPE_PRTN_UMNT_FAILURE,       "Partition unmount failed" },
    { PDRBD_PIPE_DRBDADM_ERR,             "drbdadm binary error" },
    { PDRBD_PIPE_REM_FAILURE,             "Remove failed" },
    { PDRBD_PIPE_REM_SUCCESS,             "Remove successful" },
    { PDRBD_PIPE_TERM_FAILURE,            "Termination failed" },
    { PDRBD_PIPE_TERM_SUCCESS,            "Termination successful" },
    { PDRBD_PIPE_CLMT_FAILURE,            "Metadata cleanup failed" },
    { PDRBD_PIPE_CLMT_SUCCESS,            "Metadata cleanup successful" },
    { PDRBD_PIPE_CS_UNCONFIGURED,         "Unconfigured connection state" },
    { PDRBD_PIPE_CS_STANDALONE,           "StandAlone connection state" },
    { PDRBD_PIPE_CS_UNCONNECTED,          "Unconnected connection state" },
    { PDRBD_PIPE_CS_WFCONNECTION,         "WFConnection connection state" },
    { PDRBD_PIPE_CS_WFREPORTPARAMS,       "WFReportParams connection state" },
    { PDRBD_PIPE_CS_CONNECTED,            "Connected connection state" },
    { PDRBD_PIPE_CS_TIMEOUT,              "Timeout connection state" },
    { PDRBD_PIPE_CS_BROKENPIPE,           "BrokenPipe connection state" },
    { PDRBD_PIPE_CS_NETWORKFAILURE,       "NetworkFailure connection state" },
    { PDRBD_PIPE_CS_WFBITMAPS,            "WFBitMapS connection state" },
    { PDRBD_PIPE_CS_WFBITMAPT,            "WFBitMapT connection state" },
    { PDRBD_PIPE_CS_SYNCSOURCE,           "SyncSource connection state" },
    { PDRBD_PIPE_CS_SYNCTARGET,           "SyncTarget connection state" },
    { PDRBD_PIPE_CS_PAUSEDSYNCS,          "PausedSyncS connection state" },
    { PDRBD_PIPE_CS_PAUSEDSYNCT,          "PausedSyncT connection state" },
    { PDRBD_PIPE_CS_INVALID,              "DRBD not running" },
    { 0, 0 }
  };


/******************************************************************************
 Logging stuff for MEM Logs
 ******************************************************************************/

const NCSFL_STR pdrbd_mem_log_set[] =
  {
    { PDRBD_MF_ALLOC_EVT_SUCCESS,  "Memory allocation successful"      },
    { PDRBD_MF_ALLOC_EVT_FAIL,     "Memory allocation failed"       },
    { 0,0 }
  };


/******************************************************************************
 Logging stuff for Script return logs
 ******************************************************************************/

const NCSFL_STR pdrbd_script_log_set[] =
  {
     { PDRBD_SCRIPT_CB_GET_FAILURE,       "Script ctrl block retrieval failed" },
     { PDRBD_SCRIPT_EVT_CREATE_FAILURE,   "Script event creation failed" },
     { PDRBD_SCRIPT_EXEC_FAILURE,         "Script execution failed" },
     { PDRBD_SCRIPT_EXEC_SUCCESS,         "Script execution successful" },
     { PDRBD_SCRIPT_EXIT_NORMAL,          "Script exited normally" },
     { PDRBD_SCRIPT_EXIT_ERROR,           "Script execution error" },
     { PDRBD_SCRIPT_CTX_NOT_FOUND,        "Script context not found" },
     { 0,0 }
  };


/***********************************************************************************
Logging stuff for miscellaneous logs (MDS, PROXIED COMP logs, PROXIED COMP AMF LOGS)
************************************************************************************/

const NCSFL_STR pdrbd_misc_log_set[] =
  {
    { PDRBD_MISC_MDS_GET_HDL_FAIL,              "MDS Get Handle Failed" }, 
    { PDRBD_MISC_MDS_REG_FAIL,                  "MDS Registration Failed" },
    { PDRBD_MISC_PROXIED_COMP_FILE_ABSENT,      "Proxied Component conf file not found" },
    { PDRBD_MISC_MAX_PROXIED_COMP_EXCEED,       "Components Exceeded Max Limit" },
    { PDRBD_MISC_NO_VALID_ENTRIES,              "No Valid Entries in conf file" },
    { PDRBD_MISC_MSNG_COMP_ID_IN_CONF_FILE,     "No Comp Id in proxied conf file" },
    { PDRBD_MISC_MSNG_SU_ID_IN_CONF_FILE,       "No Su ID in proxied conf file" },
    { PDRBD_MISC_MSNG_RSRC_NAME_IN_CONF_FILE,   "No Resource name in proxied conf file" },
    { PDRBD_MISC_MSNG_DEVICE_NAME_IN_CONF_FILE, "No Device name in proxied conf file" },
    { PDRBD_MISC_MSNG_MNT_PNT_IN_CONF_FILE,     "No Mount point in proxied conf file" },
    { PDRBD_MISC_MSNG_DATA_DSC_NAME_IN_CONF_FILE,"No Data disk name in proxied conf file" },
    { PDRBD_MISC_MSNG_METADATA_DSC_NAME_IN_CONF_FILE, "No Metadata disk name in proxied conf file" },
    { PDRBD_MISC_AMF_INITIALIZE_FAILURE,        "Initialiation with AMF failed" },
    { PDRBD_MISC_AMF_INITIALIZE_SUCCESS,        "Initialization with AMF Success" },
    { PDRBD_MISC_AMF_REG_FAILED,                "Registration with AMF Failed" },
    { PDRBD_MISC_AMF_REG_SUCCESS,               "Registration with AMF Success" },
    { PDRBD_MISC_AMF_ERROR_RPT_FAILED,          "Health check error report failed" },  
    { PDRBD_MISC_AMF_ERROR_RTP_SUCCESS,         "Health Check error report Success" },
    { PDRBD_MISC_AMF_CBK_INIT_FAILED,           "Proxied Comp call back initialzation failed" },
    { PDRBD_MISC_AMF_CBK_INIT_SUCCESS,          "Proxied Comp call back initialization success" },
    { PDRBD_MISC_AMF_CBK_UNINIT_FAILED,         "Proxied Comp call back uninitialization failed" },  
    { PDRBD_MISC_AMF_CBK_UNINIT_SUCCESS,        "Proxied Comp call back uninitialization success" }, 
    { PDRBD_MISC_AMF_COMP_NAME_REG_FAILED,      "Proxied Comp Name reg with AMF Failed" },  
    { PDRBD_MISC_AMF_COMP_NAME_REG_SUCCESS,     "Proxied Comp Name reg with AMF Success" },
    { PDRBD_MISC_AMF_COMP_NAME_UNREG_FAILED,    "Proxied Comp Name unreg with AMF Failed" },  
    { PDRBD_MISC_AMF_COMP_NAME_UNREG_SUCCESS,   "Proxied Comp Name unreg with AMF Success" },
    { PDRBD_MISC_AMF_COMP_RCVD_ACT_ROLE,        "Proxied Comp Got Active AMF Role" },
    { PDRBD_MISC_AMF_ACT_CSI_SCRIPT_EXEC_FAILED, "Proxied comp ACTIVE CSI_Set script exec failed" },
    { PDRBD_MISC_AMF_ACT_CSI_SCRIPT_EXEC_SUCCESS,"Proxied comp ACTIVE CSI_Set script exec Success" },
    { PDRBD_MISC_AMF_COMP_RCVD_STDBY_ROLE,       "Proxied Comp Got STDBY AMF Role" },
    { PDRBD_MISC_AMF_STDBY_CSI_SCRIPT_EXEC_FAILED, "Proxied comp STANDBY CSI_Set script exec failed" },
    { PDRBD_MISC_AMF_STDBY_CSI_SCRIPT_EXEC_SUCCESS, "Proxied comp STANDBY CSI_Set script exec Success" },
    { PDRBD_MISC_AMF_COMP_RCVD_QUISCED_ROLE,        "Proxied Comp Got QUISCED AMF Role" },
    { PDRBD_MISC_AMF_QSED_CSI_SCRIPT_EXEC_FAILED,   "proxied comp QUIESCED CSI_Set script exec failed" }, 
    { PDRBD_MISC_AMF_QSED_CSI_SCRIPT_EXEC_SUCCESS,  "proxied comp QUIESCED CSI_Set script exec failed" },
    { PDRBD_MISC_AMF_COMP_RCVD_INVALID_ROLE,        "Proxied Comp Got QUISCED AMF Role" }, 
    { PDRBD_MISC_AMF_COMP_RCVD_RMV_CBK,             "Proxied Comp Got Invalid Role" },
    { PDRBD_MISC_AMF_COMP_RCVD_TERM_CBK,            "Proxied Comp Got Terminate Callback" },
    { PDRBD_MISC_AMF_COMP_TERM_SCRIPT_EXEC_FAILED,  "Proxied comp Terminate script exec failed" },
    { PDRBD_MISC_AMF_HLT_CHK_SCRIPT_TMD_OUT,        "Health Check script Exec timed out" },
    { PDRBD_MISC_INVOKING_SCRIPT_TO_CLEAN_RSRC_METADATA, "Invoking script to clean resource metadata" },
    { PDRBD_MISC_CLEAN_METADATA_SCRIPT_EXEC_FAILED,  "Script to clean resource metadata failed" },  
    { PDRBD_MISC_RECONNECTING_RESOURCE,              "Reconnecting resource" },
    { PDRBD_MISC_PROXIED_CONF_FILE_PARSE_FAILED,     "Proxied conf file parsing failed" },
    { PDRBD_MISC_UNABLE_TO_PARSE_PROXIED_INFO,       "unable to parse proxied info" },
    { PDRBD_MISC_PROXIED_INFO_FROM_CONF_FILE,        "Proxied Information regarding comp in conf file" },
    { PDRBD_MISC_UNABLE_TO_RETRIEVE_NODE_ID,         "Unable to retrieve node id of the component" },
    { PDRBD_MISC_PSEUDO_DRBD_HCK_SRC_EXEC_FAILED,    "Pseudo DRBD health check script exec failed" },
    { PDRBD_MISC_AMF_COMP_RMV_SCRIPT_EXEC_FAILED,    "Proxied Component remove script exec failed" },
    { PDRBD_MISC_GET_ADEST_HDL_FAILED,               "unable to get mds Adest Hdl" },
    { PDRBD_MISC_MDS_HDL_IS_NULL,                    "MDS Handle is null in CB" },
    { PDRBD_MISC_MDS_INSTALL_SUBSCRIBE_FAILED,       "Failed to install and subscribe with MDS" },
    { PDRBD_MISC_MDS_SUBSCRIPTION_FAILED,            "Failed to subscribe with MDS" },
    { PDRBD_MISC_MDS_CBK_WITH_WRONG_OPERATION,       "MDS Call back called with wrong operation" },
    { PDRBD_MISC_MDS_EVT_RECD,                       "MDS Event Received" },
    { PDRBD_MISC_MDS_DOWN_EVT_RCVD,                  "MDS Down Event received" },
    { PDRBD_MISC_MDS_UP_EVT_RCVD,                    "MDS Up Event Received" },
    { PDRBD_MISC_RCVD_NULL_USR_BUFF,                 "Received NULL User buffer" },
    { PDRBD_MISC_RCVD_NULL_MSG_DATA,                 "Received Message with no data" },
    { PDRBD_MISC_NCS_ENC_RESERVE_SPC_NULL,           "Ncs encode reserve space returned NULL value" },
    { PDRBD_MISC_WRONG_MSG_TYPE_RCVD,                "Wrong message type received for encoding" },
    { PDRBD_MISC_RCVD_NULL_USR_BUF_FOR_DEC,          "Received Null User buffer for decoding" },
    { PDRBD_MISC_RCVD_NULL_MSG_BUF_FOR_DEC,          "Received Null Message data for decoding" },
    { PDRBD_MISC_MEM_ALLOC_FAILED_FOR_DEC,           "Memory Allocation Failed in MDS Decoding" },
    { PDRBD_MISC_DEC_FLAT_SPACE_RET_NULL,            "Ncs decode flatten space return NULL value" },
    { PDRBD_MISC_WRNG_MSG_TYPE_RCVD_DEC,             "Wrong Message type received in MDS decode" },  
    { PDRBD_MISC_MDS_ASYNC_SEND_FAILED,              "MDS Async Send Failed" },
    { PDRBD_MISC_PROXIED_COMP_LEN_EXCEED_MAX_LEN,    "Proxied comp name length exceed max val" },
    { PDRBD_MISC_PROXIED_SUID_LEN_EXCEED_MAX_LEN,    "Proxied comp SU-ID Length exceed Max val" },
    { PDRBD_MISC_PROXIED_RSC_LEN_EXCEED_MAX_LEN,     "Pxoxied comp resource name len exceed max val" },
    { PDRBD_MISC_PROXIED_DEV_LEN_EXCEED_MAX_LEN,     "Proxied comp Dev name length exceed max val" },
    { PDRBD_MISC_PROXIED_MNT_PNT_LEN_EXCEED_MAX_LEN, "Proxied comp mnt pnt length exceed max val" },
    { PDRBD_MISC_PROXIED_DATA_DSC_LEN_EXCEED_MAX_LEN, "Proxied comp data disc length exceed max val" },
    { PDRBD_MISC_PROXIED_META_DATA_DSC_LEN_EXCEED_MAX_LEN, "Proxied comp meta data disc len exceed max val" }, 

    { 0,0 }

  };

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET pdrbd_str_set[] =
  {
    { PDRBD_FC_HDLN,           0, (NCSFL_STR*) pdrbd_hdln_set      },
    { PDRBD_FC_AMF,            0, (NCSFL_STR*) pdrbd_amf_log_set },
    { PDRBD_FC_PIPE,           0, (NCSFL_STR*) pdrbdPipeLogSet },
    { PDRBD_FC_MEM,            0, (NCSFL_STR*) pdrbd_mem_log_set      },
    { PDRBD_FC_SCRIPT,         0, (NCSFL_STR*) pdrbd_script_log_set   },
    { PDRBD_FC_MISC,           0, (NCSFL_STR*) pdrbd_misc_log_set     },
    { 0,0,0 }
  };

NCSFL_FMAT pdrbd_fmat_set[] =
{                                         /*  T,State,SVCID, pwe, anch, ....*/
    { PDRBD_LID_HDLN,           "TIL",       " [%s] PDRBD HEADLINE: %s  %ld \n\n" },
    { PDRBD_LID_AMF,            "TIL",       " [%s] PDRBD AMF EVENT: %s %ld \n\n" },
    { PDRBD_LID_PIPE,           "TIL",       " [%s] PDRBD PIPE EVENT: %s %ld \n\n" },
    { PDRBD_LID_MEM,            "TIL",       " [%s] PDRBD MEM EVENT: %s %ld \n\n"},
    { PDRBD_LID_SCRIPT,         "TIL",       " [%s] PDRBD SCRIPT EVENT: %s %ld \n\n"},
    { PDRBD_LID_MISC,           "TIC",       " [%s] PDRBD MISC EVENT FOR COMP:   %s %s \n\n"},
    { PDRBD_LID_MISC_INDEX,     "TI",        " [%s] PDRBD MISC EVENT %s \n\n"},  
    { PDRBD_LID_OTHER,          "TCL",       " [%s] PDRBD LOG: %s  %ld \n\n" },
    { 0, 0, 0 }
};


NCSFL_ASCII_SPEC pdrbd_ascii_spec =
  {
    NCS_SERVICE_ID_PDRBD,           /* PDRBD subsystem */
    PDRBD_LOG_VERSION,              /* PDRBD revision ID */
    "PDRBD",                        /* Name to be printed for log file */
    (NCSFL_SET*)  pdrbd_str_set,    /* PDRBD const strings referenced by index */
    (NCSFL_FMAT*) pdrbd_fmat_set,   /* PDRBD string format info */
    0,                              /* Place holder for str_set count */
    0                               /* Place holder for fmat_set count */
  };

/*****************************************************************************

  ncspdrbd_reg_strings

  DESCRIPTION: Function is used for registering the canned strings with the PDRBD.

*****************************************************************************/

uns32 pdrbd_reg_strings()
{
    NCS_DTSV_REG_CANNED_STR arg;

    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
    arg.info.reg_ascii_spec.spec = &pdrbd_ascii_spec;
    if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pdrbd_log_str_lib_req
 *
 * Description   : Library entry point for pdrbd log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which SBOM gives.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
pdrbd_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = pdrbd_reg_strings();
         break;

      case NCS_LIB_REQ_DESTROY:
         res = pdrbd_dereg_strings();
         break;

      default:
         break;
   }
   return (res);
}

/*****************************************************************************

  ncspdrbd_dereg_strings

  DESCRIPTION: Function is used for registering the canned strings with the PDRBD.

*****************************************************************************/

uns32 pdrbd_dereg_strings()
{
    NCS_DTSV_REG_CANNED_STR arg;

    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
    arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_PDRBD;
    arg.info.dereg_ascii_spec.version = PDRBD_LOG_VERSION;

    if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    return NCSCC_RC_SUCCESS;
}

#else  /* (PDRBD_LOG == 0) */

/*****************************************************************************

  pdrbd_reg_strings

  DESCRIPTION: When PDRBD_LOG is disabled, we don't register a thing,

*****************************************************************************/

uns32 pdrbd_reg_strings(char* fname)
  {
  USE(fname);
  return NCSCC_RC_SUCCESS;
  }

#endif
