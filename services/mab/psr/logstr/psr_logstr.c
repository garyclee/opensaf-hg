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


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging functions of PSSv.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:    log_pss_headline()
                                        log_pss_svc_prvdr()
                                        log_pss_lock()
                                        log_pss_memfail()
                                        log_pss_evt()
                                        log_pss_api()
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/


#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncs_stack.h"
#include "ncs_mib.h"
#include "ncs_mtbl.h"
#include "ncs_log.h"
#include "psr_log.h"
#include "dts_papi.h"


/******************************************************************************
 Logging stuff for Headlines 
 ******************************************************************************/

const NCSFL_STR pss_hdln_set[] = 
{
    { PSS_HDLN_TBL_ALREADY_REG,     "MIB table already registered with PSS"            },
    { PSS_HDLN_TBL_ID_TOO_LARGE,     "MIB table ID is greater than allowable value"            },
    { PSS_HDLN_TBL_DESC_NULL,     "MIB table description is NULL, implying table is not loaded"            },
    { PSS_HDLN_ENTER_WBREQ_HNDLR_FUNC,  "PSS entering Warmboot Playback function"      },
    { PSS_HDLN_EXIT_WBREQ_HNDLR_FUNC,    "PSS exiting Warmboot Playback function"      },
    { PSS_HDLN_START_OD_PLAYBACK,  "PSS entering On-demand Playback function"      },/* 5 */
    { PSS_HDLN_END_OD_PLAYBACK,    "PSS exiting On-demand Playback function"     },
    { PSS_HDLN_WARMBOOT_TBL_ERR,    "PSS Warmboot Playback error in table"},
    { PSS_HDLN_REQUEST_PROCESS_ERR, "PSS FAIL to process SNMP request"},
    { PSS_HDLN_RECEIVED_CLI_REQ,    "PSS CLI command MIB request received"},
    { PSS_HDLN_SAVE_CURR_CONF,      "PSS Saving Current Configuration"},
    { PSS_HDLN_TBL_BIND_PROCESS_ERR, "PSS FAIL to process Table-bind request"               },
    { PSS_HDLN_TBL_UNBIND_PROCESS_ERR, "PSS FAIL to process Table-unbind request"               },
    { PSS_HDLN_MIB_REQ_FAILED,      "PSS MIB Request FAIL"   },
    { PSS_HDLN_MIB_REQ_SUCCEEDED,   "PSS MIB Request succeeded"}, /*10 */
    { PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS, "PSS MIB Request response is not Success"                           },
    { PSS_HDLN_MIB_REQ_RETURN_NO_SUCH_TBL, "PSS MIB Request response is NO_SUCH_TBL"                           },
    { PSS_HDLN_MIB_REQ_RSP_NOT_SET,      "PSS MIB Request response type is not NCSMIB_OP_RSP_SET"                           },
    { PSS_HDLN_MIB_REQ_RSP_NOT_SETROW,      "PSS MIB Request response type is not NCSMIB_OP_RSP_SETROW"                           },
    { PSS_HDLN_MIB_REQ_RSP_NOT_SETALLROWS,      "PSS MIB Request response type is not NCSMIB_OP_RSP_SETALLROWS"                           },
    { PSS_HDLN_MIB_REQ_RSP_NOT_REMOVEROWS,      "PSS MIB Request response type is not NCSMIB_OP_RSP_REMOVEROWS"                           },
    { PSS_HDLN_PLAYBACK_TBL_ERR,    "PSS On Demand Playback Error Processing Table"    },
    { PSS_HDLN_PLAYBACK_CHG_ERR,    "PSS On Demand Pback Error processing Chg List"    },
    { PSS_HDLN_PLAYBACK_ADD_ERR,    "PSS On Demand Pback Error processing Add List"    },
    { PSS_HDLN_SAVE_TBL_ERROR,      "PSS Error saving table to current configuration"  },
    { PSS_HDLN_LMCBFNC_NULL,        "PSS LM Callback function is NULL"                 },
    { PSS_HDLN_PSSTS_API_NULL,      "PSS Target Service API is NULL"                   },
    { PSS_HDLN_NCSMIBTM_INIT_ERR,    "PSS Error initializing NCSMIB Transaction Manager" },
    { PSS_HDLN_PSSTS_CONFIG_ERR,    "PSS Unable to get Config from PSSTS"              },
    { PSS_HDLN_FILE_CREATE_FAIL,    "PSS File creation FAIL"}, /* 23 */
    { PSS_HDLN_INVALID_MIB_DATA,    "PSS Invalid mibdata structure. Corrupted data"    }, /* 24 */
    { PSS_HDLN_ENTRY_FOUND,         "PSS MIB table entry found on disk"                },
    { PSS_HDLN_ENTRY_NOT_FOUND,     "PSS MIB table entry not found on disk"            },
    { PSS_HDLN_OAC_DATA_DELETING,   "PSS Deleting all data for OAA"                    },
    { PSS_HDLN_OAC_DATA_DELETED,    "PSS Deleted all data for OAA"                     },
    { PSS_HDLN_CLI_INV_TBL,         "PSS CLI request received for invalid table"       },
    { PSS_HDLN_SAVE_CURR_CONF_DONE, "PSS Finished saving current configuration"        },
    { PSS_HDLN_PROCESS_DIFFQ_START, "PSS Processing diff queue - start"                },
    { PSS_HDLN_PROCESS_DIFFQ_END,   "PSS Processing diff queue - end"                  },
    { PSS_HDLN_SAVE_TO_STORE_START, "PSS Save MIB table to store - start"              },
    { PSS_HDLN_SAVE_TO_STORE_END,   "PSS Save MIB table to store - end"                },
    { PSS_HDLN_SET_EXST_PRO,        "PSS Set existing profile name"                    },
    { PSS_HDLN_SET_NEW_PRO,         "PSS Set new profile name"                         },
    { PSS_HDLN_SET_CUR_PRO,         "PSS Set current profile name"                     },
    { PSS_HDLN_CLI_REQ_NOOP,        "PSS CLI Request - noop"                           },
    { PSS_HDLN_CLI_REQ_LOAD,        "PSS CLI Request - load"                           },
    { PSS_HDLN_CLI_REQ_SAVE,        "PSS CLI Request - save"                           },
    { PSS_HDLN_CLI_REQ_COPY,        "PSS CLI Request - copy"                           },
    { PSS_HDLN_CLI_REQ_RENAME,      "PSS CLI Request - rename"                         },
    { PSS_HDLN_CLI_REQ_INVALID,     "PSS CLI Request - invalid"                        },
    { PSS_HDLN_OAC_REG_SUCCESS,     "PSS Registered tables and filters with OAA"       },
    { PSS_HDLN_OAC_REG_FAILURE,     "PSS Failed to register tables and filters with OAA"},
    { PSS_HDLN_NULL_INST,           "PSS Invalid handle received"                      },
    { PSS_HDLN_NULL_SNMP,           "PSS Invalid ncsmib_arg handle received"           },
    { PSS_HDLN_NULL_USRBUF,         "PSS NULL USRBUF in ncsmib_arg received"           },
    { PSS_HDLN_INVALID_PWE_HDL,     "PSS Received invalid PWE handle"                  },
    { PSS_HDLN_MAC_PBKS_SUCCESS,    "PSS Sent playback start to MAA - success"         },
    { PSS_HDLN_MAC_PBKS_FAILURE,    "PSS Sent playback start to MAA - failure"         },
    { PSS_HDLN_MAC_PBKE_SUCCESS,    "PSS Sent playback end to MAA - success"           },
    { PSS_HDLN_MAC_PBKE_FAILURE,    "PSS Sent playback end to MAA - failure"           },
    { PSS_HDLN_OAC_PBKS_SUCCESS,    "PSS Sent playback start to OAA - success"         },
    { PSS_HDLN_OAC_PBKS_FAILURE,    "PSS Sent playback start to OAA - failure"         },
    { PSS_HDLN_OAC_PBKE_SUCCESS,    "PSS Sent playback end to OAA - success"           },
    { PSS_HDLN_OAC_PBKE_FAILURE,    "PSS Sent playback end to OAA - failure"           },
    { PSS_HDLN_PAT_TREE_FAILURE,    "PSS Failed to initialize patricia tree"           },
    { PSS_HDLN_SPCN_LIST_FILE_OPEN_FAIL, "PSS Failed to open pssv_spcn_list file"      },
    { PSS_HDLN_SPCN_LIST_FILE_OPEN_IN_APPEND_MODE_FAIL, "PSS open pssv_spcn_list file in append-mode FAIL"             },
    { PSS_HDLN_SPCN_ADD_OP_FAIL,    "PSS Add SPCN entry FAIL"                        },
    { PSS_HDLN_LIB_CONF_FILE_OPEN_FAIL, "PSS Failed to open pssv_lib_conf file"        },
    { PSS_HDLN_CLIENT_ENTRY_NULL, "PSS Client entry not found"                         },
    { PSS_HDLN_CLIENT_ENTRY_ADD_FAIL, "PSS Client entry add FAIL"                    },
    { PSS_HDLN_WBSORTDB_INIT_FAIL, "PSS Warmboot Sort database init FAIL"            },
    { PSS_HDLN_WBSORTDB_ADD_ENTRY_FAIL, "PSS Warmboot Sort database entry add FAIL"  },
    { PSS_HDLN_WBPLBCK_SORT_OP_FAIL, "PSS executing of Warmboot Sort database FAIL"  },
    { PSS_HDLN_ODSORTDB_INIT_FAIL, "PSS On-demand Sort database init FAIL"           },
    { PSS_HDLN_ODPLBCK_OP_FAIL, "PSS On-demand playback FAIL"                        },
    { PSS_HDLN_ODSORTDB_ADD_ENTRY_FAIL, "PSS On-demand Sort database entry add FAIL" },
    { PSS_HDLN_OAA_ENTRY_ADD_FAIL, "PSS OAA_ENTRY add FAIL"                          },
    { PSS_HDLN_SNMP_REQ_FRM_INV_MDS_DEST_RCVD, "PSS SNMP request from Invalid MDS_DEST" },
    { PSS_HDLN_TBL_BIND_RCVD_FROM_MDSDEST, "TBL BIND request from MDS_DEST" },
    { PSS_HDLN_TBL_BIND_FND_TBL_REC_ALREADY_FOR_MDSDEST, "TBL rec already exists for MDS_DEST" },
    { PSS_HDLN_TBL_REC_NOT_FND, "PSS Table record not found"                           },
    { PSS_HDLN_NULL_TBL_BIND, "PSS Invalid Table Bind Info from OAA-SU"             },
    { PSS_HDLN_TBL_BIND_FAIL, "PSS Table Bind FAIL"             },
    { PSS_HDLN_TBL_BIND_INVLD_PWE_HDL, "PSS Table Bind op: Invalid PWE-handle"             },
    { PSS_HDLN_TBL_BIND_SUCCESS, "PSS Table Bind op SUCCESS"             },
    { PSS_HDLN_TBL_REC_PATTREE_INIT_FAIL, "PSS Table Record Patricia Tree Init FAIL"             },
    { PSS_HDLN_TBL_UNBIND_FAIL, "PSS Table Unbind FAIL"             },
    { PSS_HDLN_TBL_UNBIND_OAA_CLT_NODE_NOT_FOUND, "PSS Table Unbind op: oaa_clt_node not found"             },
    { PSS_HDLN_TBL_UNBIND_PCN_NODE_NOT_FOUND, "PSS Table Unbind op: pcn_node not found"             },
    { PSS_HDLN_TBL_UNBIND_INVLD_PWE_HDL, "PSS Table Unbind op: Invalid PWE-handle"             },
    { PSS_HDLN_TBL_REC_ADD_FAIL, "PSS Table record add FAIL"             },
    { PSS_HDLN_STR, "PSS Data string:"}, 
    { PSS_HDLN_AMF_INITIALIZE_SUCCESS, "PSS AMF Initialization successful"},
    { PSS_HDLN_AMF_HLTH_DOING_AWESOME, "PSS AMF Health status: Awesome"}, 
    { PSS_HDLN_AMF_HLTH_DOING_AWFUL, "PSS AMF Health status: Awful"}, 
    { PSS_HDLN_AMF_CSI_DETAILS, "PSS CSI Details reeived from AMF"},
    { PSS_HDLN_AMF_CSI_ADD_ONE_SUCCESS, "PSS CSI Add success"},
    { PSS_HDLN_AMF_CSI_STATE_CHG_SUCCESS, "PSS CSI State Change success:"},
    { PSS_HDLN_AMF_CSI_STATE_CHG_FAILED, "PSS CSI State Change FAIL(current state, asked state)"}, 
    { PSS_HDLN_AMF_CSI_REMOVE_SUCCESS, "PSS one CSI Remove success (envId)"},
    { PSS_HDLN_AMF_CSI_REMOVE_ALL_SUCCESS,"PSS CSI Remove all Success"},
    { PSS_HDLN_CREATE_SUCCESS, "PSS Create Success"}, 
    { PSS_HDLN_TGT_SVCS_CREATE_SUCCESS, "PSS Target Services Create Success"},
    { PSS_HDLN_TGT_SVCS_DESTROY_SUCCESS, "PSS Target Services Destroy Success"}, 
    { PSS_HDLN_DEF_ENV_INIT_SUCCESS, "PSS Initialization in Default Env Success"},
    { PSS_HDLN_MAA_REL_INST_SUCCESS, "PSS MAA instance release for (EnvId) Success"},
    { PSS_HDLN_MDS_REL_INST_SUCCESS, "PSS MDS instance release for (EnvId) Success"}, 
    { PSS_HDLN_PWE_CB_OAA_TREE_DESTROY_SUCCESS, "PSS OAA Tree destroy Success in this Environement"}, 
    { PSS_HDLN_PWE_CB_CLNT_TREE_DESTROY_SUCCESS, "PSS Client tree destroy Success in this Environement"}, 
    { PSS_HDLN_PWE_CB_DESTROY_SUCCESS, "PSS PSS_PWE_CB destroy success for this Environemnt"},
    { PSS_HDLN_AMF_CSI_ATTRIB_DECODE_SUCCESS, "PSS pss_amf_csi_attrib_decode(): Successfully decode the Env-Id from CSI Attributes (decode_env)"},
    { PSS_HDLN_MAS_MDS_UP, "PSS received MAS MDS_UP event"},
    { PSS_HDLN_MAS_MDS_DOWN, "PSS received MAS MDS_DOWN event"},
    { PSS_HDLN_BAM_MDS_UP, "PSS received BAM MDS_UP event"},
    { PSS_HDLN_BAM_MDS_DOWN, "PSS received BAM MDS_DOWN event"},
    { PSS_HDLN_INVALID_PROFILE_SPCFD, "Invalid profile specified for CLI command"},
    { PSS_HDLN_GET_PROF_CLIENTS_OP_FAILED, "Get profile-clients operation FAIL"},
    { PSS_HDLN_AMF_SIGNAL_RCVD, "PSS-AMF pss_amf_sigusr1_handler() entered"},
    { PSS_HDLN_AMF_COMP_MSG_SENT, "PSS-AMF pss_amf_sigusr1_handler() done"}, 
    { PSS_HDLN_INVLD_CONF_DONE_MSG_RCVD, "PSS-BAM conf_done with invalid PCN rcvd"}, 
    { PSS_HDLN_INVLD_PCN_IN_WBREQ_TERMINATING_SESSION, "Invalid PCN in WBREQ, so terminating the playback session"}, 
    { PSS_HDLN_SPCN_ENTRY_FOUND, "SPCN entry found"}, 
    { PSS_HDLN_PCN_NODE_FOUND, "PCN node found"}, 
    { PSS_HDLN_PCN_NODE_NOT_FOUND, "PCN node NOT found"}, 
    { PSS_HDLN_FWDING_REQ_TO_BAM, "Forwarding WBREQ to BAM"}, 
    { PSS_PWE_CB_TAKE_HANDLE_IN_MBCSVCB_FAILED, "pwe_cb take handle FAIL in MBCSv callback"}, 
    { PSS_HDLN_RE_PCN_ADD_FAIL, "RE PCN ADD TO CLIENT-TREE FAIL"}, 
    { PSS_HDLN_RE_OAA_TREE_ADD_FAIL, "RE OAA ADD TO OAA-TREE FAIL"}, 
    { PSS_HDLN_RE_TBL_REC_ADD_FAIL, "RE TBL ADD FAIL"}, 
    { PSS_HDLN_RE_TBL_REC_ADD_SUCCESS, "RE TBL ADD SUCCESS"}, 
    { PSS_HDLN_MBCSV_INITIALIZE_FAIL, "MBCSV INITIALIZE FAIL"}, 
    { PSS_HDLN_MBCSV_FINALIZE_FAIL, "MBCSV FINALIZE FAIL"}, 
    { PSS_HDLN_MBCSV_FINALIZE_SUCCESS, "MBCSV FINALIZE SUCCESS"}, 
    { PSS_HDLN_MBCSV_SET_OP_FAIL, "MBCSV SET OP FAIL"}, 
    { PSS_HDLN_MBCSV_SET_OP_SUCCESS, "MBCSV SET OP SUCCESS"}, 
    { PSS_HDLN_MBCSV_OPEN_FAIL, "MBCSV OPEN FAIL"}, 
    { PSS_HDLN_MBCSV_OPEN_SUCCESS, "MBCSV OPEN SUCCESS"}, 
    { PSS_HDLN_MBCSV_CLOSE_FAIL, "MBCSV CLOSE FAIL"}, 
    { PSS_HDLN_MBCSV_CLOSE_SUCCESS, "MBCSV CLOSE SUCCESS"}, 
    { PSS_HDLN_MBCSV_DISPATCH_FAIL, "MBCSV DISPATCH FAIL"}, 
    { PSS_HDLN_MBCSV_DISPATCH_SUCCESS, "MBCSV DISPATCH SUCCESS"}, 
    { PSS_HDLN_MBCSV_CB_NULL_ARG_PTR, "MBCSV CB NULL PTR TO NCS_MBCSV_CB_ARG passed by MBCSv"}, 
    { PSS_HDLN_MBCSV_CB_INVOKED, "MBCSV CB invoked"}, 
    { PSS_HDLN_MBCSV_ENC_CB_INVOKED, "MBCSV ENC CB invoked"}, 
    { PSS_HDLN_MBCSV_DEC_CB_INVOKED, "MBCSV DEC CB invoked"}, 
    { PSS_HDLN_MBCSV_PERR_INFO_CB_INVOKED, "MBCSV PEER INFO CB invoked"}, 
    { PSS_HDLN_MBCSV_NOTIFY_CB_INVOKED, "MBCSV NOTIFY CB invoked"}, 
    { PSS_HDLN_MBCSV_ERR_IND_CB_INVOKED, "MBCSV ERR IND CB invoked"}, 
    { PSS_HDLN_MBCSV_CB_INVLD_OP_PASSED, "MBCSV CB Invalid OP type passed"}, 
    { PSS_HDLN_MBCSV_GET_SELECT_OBJ_FAIL, "MBCSV GET SELECTION OBJECT FAIL"}, 
    { PSS_HDLN_MBCSV_GET_SELECT_OBJ_SUCCESS, "MBCSV GET SELECTION OBJECT SUCCESS"}, 
    { PSS_HDLN_CKPT_STATE, "CHECKPOINT STATUS"}, 
    { PSS_HDLN_INVLD_ENC_MSG_TYPE, "MBCSV INVALID ENC MSG TYPE in encode callback"}, 
    { PSS_HDLN_INVLD_DEC_MSG_TYPE, "MBCSV INVALID DEC MSG TYPE in decode callback"}, 
    { PSS_HDLN_ASYNC_UPDATE_ENC_STATUS, "MBCSV ASYNC UPDATE ENC STATUS"}, 
    { PSS_HDLN_ASYNC_UPDATE_ENC_STARTED, "MBCSV ASYNC UPDATE ENC STARTED"}, 
    { PSS_HDLN_COLD_SYNC_REQ_ENC_DONE, "MBCSV COLD SYNC REQ ENC DONE"}, 
    { PSS_HDLN_COLD_SYNC_RESP_ENC_FAIL, "MBCSV COLD SYNC RESP ENC FAIL"}, 
    { PSS_HDLN_COLD_SYNC_RESP_ENC_STARTED, "MBCSV COLD SYNC RESP ENC STARTED"}, 
    { PSS_HDLN_COLD_SYNC_RESP_ENC_SUCCESS, "MBCSV COLD SYNC RESP ENC SUCCESS"}, 
    { PSS_HDLN_COLD_SYNC_RESP_ENC_IN_NON_ACTIVE_ROLE, "MBCSV COLD SYNC RESP ENC IN NON-ACTIVE ROLE"}, 
    { PSS_HDLN_WARM_SYNC_REQ_ENC_DONE, "MBCSV WARM SYNC REQ ENC DONE"}, 
    { PSS_HDLN_WARM_SYNC_RESP_ENC_STARTED, "MBCSV WARM SYNC RESP ENC STARTED"}, 
    { PSS_HDLN_WARM_SYNC_RESP_ENC_FAIL, "MBCSV WARM SYNC RESP ENC FAIL"}, 
    { PSS_HDLN_WARM_SYNC_RESP_ENC_SUCCESS, "MBCSV WARM SYNC RESP ENC SUCCESS"}, 
    { PSS_HDLN_WARM_SYNC_RESP_ENC_IN_NON_ACTIVE_ROLE, "MBCSV WARM SYNC RESP ENC IN NON-ACTIVE ROLE"}, 
    { PSS_HDLN_DATA_REQ_ENC_IN_NON_STANDBY_ROLE, "MBCSV DATA REQ ENC IN NON-STANDBY ROLE"}, 
    { PSS_HDLN_DATA_RESP_ENC_STARTED, "MBCSV DATA RESP ENC STARTED"}, 
    { PSS_HDLN_DATA_RESP_ENC_FAIL, "MBCSV DATA RESP ENC FAIL"}, 
    { PSS_HDLN_DATA_RESP_ENC_SUCCESS, "MBCSV DATA RESP ENC SUCCESS"}, 
    { PSS_HDLN_DATA_RESP_ENC_IN_NON_ACTIVE_ROLE, "MBCSV DATA RESP ENC IN NON-ACTIVE ROLE"}, 
    { PSS_HDLN_ASYNC_UPDATE_DEC_STARTED, "MBCSV ASYNC UPDATE DEC STARTED"}, 
    { PSS_HDLN_ASYNC_UPDATE_DEC_STATUS, "MBCSV ASYNC UPDATE DEC STATUS"}, 
    { PSS_HDLN_COLD_SYNC_REQ_DEC_DONE, "MBCSV COLD SYNC REQ DEC DONE"}, 
    { PSS_HDLN_COLD_SYNC_RESP_DEC_STARTED, "MBCSV COLD SYNC RESP DEC STARTED"}, 
    { PSS_HDLN_COLD_SYNC_RESP_DEC_FAIL, "MBCSV COLD SYNC RESP DEC FAIL"}, 
    { PSS_HDLN_COLD_SYNC_RESP_DEC_SUCCESS, "MBCSV COLD SYNC RESP DEC SUCCESS"}, 
    { PSS_HDLN_COLD_SYNC_RESP_DEC_IN_NON_ACTIVE_ROLE, "MBCSV COLD SYNC RESP DEC IN NON-ACTIVE ROLE"}, 
    { PSS_HDLN_COLD_SYNC_RESP_DEC_IN_CKPT_STATE, "MBCSV COLD SYNC RESP DEC IN CHECKPOINT STATE"}, 
    { PSS_HDLN_INVKD_PENDING_AMF_RESP, "INVOKED PENDING AMF RESPONSE CALLBACK FOR ACTIVE"}, 
    { PSS_HDLN_SEND_DATA_REQ_FAIL, "SEND DATA REQ FAIL"}, 
    { PSS_HDLN_WARM_SYNC_REQ_DEC_DONE, "MBCSV WARM SYNC REQ DEC DONE"}, 
    { PSS_HDLN_WARM_SYNC_RESP_DEC_STARTED, "MBCSV WARM SYNC RESP DEC STARTED"}, 
    { PSS_HDLN_WARM_SYNC_RESP_DEC_FAIL, "MBCSV WARM SYNC RESP DEC FAIL"}, 
    { PSS_HDLN_WARM_SYNC_RESP_DEC_SUCCESS, "MBCSV WARM SYNC RESP DEC SUCCESS"}, 
    { PSS_HDLN_WARM_SYNC_RESP_DEC_IN_ACTIVE_ROLE, "MBCSV WARM SYNC RESP DEC IN ACTIVE ROLE"}, 
    { PSS_HDLN_DATA_REQ_DEC_IN_NON_ACTIVE_ROLE, "DATA REQ DEC IN NON-ACTIVE ROLE"}, 
    { PSS_HDLN_DATA_REQ_DEC_DONE, "DATA REQ DEC DONE"}, 
    { PSS_HDLN_DATA_RESP_DEC_STARTED, "DATA RESP DEC STARTED"}, 
    { PSS_HDLN_DATA_RESP_DEC_FAIL, "DATA RESP DEC FAIL"}, 
    { PSS_HDLN_DATA_RESP_DEC_SUCCESS, "DATA RESP DEC SUCCESS"}, 
    { PSS_HDLN_DATA_RESP_DEC_FAIL_WITH_COLD_SYNC_COMPLT_BUT_WARM_SYNC_FAIL, "DATA RESP DEC FAIL WITH COLD_SYNC_COMPLETE state, BUT WARM_SYNC FAIL"}, 
    { PSS_HDLN_DATA_RESP_DEC_IN_NON_STANDBY_ROLE, "DATA RESP DEC IN NON-STANDBY ROLE"}, 
    { PSS_HDLN_VDEST_ROLE_QUIESCED_INVLD_PWE, "Invalid PWE handle during VDEST role change to Quiesced"}, 
    { PSS_HDLN_MBCSV_SET_CKPT_ROLE_FAIL, "MBCSv SET CKPT ROLE FAIL"}, 
    { PSS_HDLN_MBCSV_SET_CKPT_ROLE_SUCCESS, "MBCSv SET CKPT ROLE SUCCESS"}, 
    { PSS_HDLN_MBCSV_SEND_CKPT_FAIL, "MBCSv SEND CKPT ROLE FAIL"}, 
    { PSS_HDLN_MBCSV_SEND_CKPT_SUCCESS, "MBCSv SEND CKPT ROLE SUCCESS"}, 
    { PSS_HDLN_MBCSV_MDEST_ENC_FAIL, "MBCSv ENC MDS_DEST FAIL"}, 
    { PSS_HDLN_MBCSV_MDEST_ENC_SUCCESS, "MBCSv ENC MDS_DEST SUCCESS"}, 
    { PSS_HDLN_MBCSV_MDEST_DEC_FAIL, "MBCSv DEC MDS_DEST FAIL"}, 
    { PSS_HDLN_MBCSV_MDEST_DEC_SUCCESS, "MBCSv DEC MDS_DEST SUCCESS"}, 
    { PSS_HDLN_MBCSV_ENC_PCN_SUCCESS, "MBCSv ENC PCN SUCCESS"}, 
    { PSS_HDLN_MBCSV_ENC_PCN_NULL, "MBCSv ENC PCN is NULL"}, 
    { PSS_HDLN_MBCSV_ENC_PCN_FAIL, "MBCSv ENC PCN FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_PCN_FAIL, "MBCSv DEC PCN FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_PCN_SUCCESS, "MBCSv DEC PCN SUCCESS"}, 
    { PSS_HDLN_MBCSV_DEC_PP_PCN_NULL, "MBCSv DEC PCN Double-pointer is NULL"}, 
    { PSS_HDLN_MBCSV_TBL_ID_ENC_SUCCESS, "MBCSv ENC TBL-ID SUCCESS"}, 
    { PSS_HDLN_MBCSV_TBL_ID_ENC_FAIL, "MBCSv ENC TBL-ID FAIL"}, 
    { PSS_HDLN_MBCSV_TBL_ID_DEC_SUCCESS, "MBCSv DEC TBL-ID SUCCESS"}, 
    { PSS_HDLN_MBCSV_TBL_ID_DEC_FAIL, "MBCSv DEC TBL-ID FAIL"}, 
    { PSS_HDLN_RE_RESUME_ACTIVE_ROLE_EVT_POSTED, "RE RESUME ACTIVE ROLE EVT POSTED TO MAILBOX"}, 
    { PSS_HDLN_MBCSV_PLBCK_SSNINFO_SESSION_DONE, "MBCSv PLBCK_SSNINFO SESSION COMPLETED"}, 
    { PSS_HDLN_MBCSV_MIB_IDX_LEN_ENC_FAIL, "MBCSv MIB_IDX LEN ENC FAIL"}, 
    { PSS_HDLN_MBCSV_MIB_IDX_LEN_ENC_SUCCESS, "MBCSv MIB_IDX LEN ENC SUCCESS"}, 
    { PSS_HDLN_MBCSV_MIB_IDX_LEN_DEC_FAIL, "MBCSv MIB_IDX LEN DEC FAIL"}, 
    { PSS_HDLN_MBCSV_MIB_IDX_LEN_DEC_SUCCESS, "MBCSv MIB_IDX LEN DEC SUCCESS"}, 
    { PSS_HDLN_MBCSV_MIB_IDX_ENC_FAIL, "MBCSv MIB_IDX ENC FAIL"}, 
    { PSS_HDLN_MBCSV_MIB_IDX_ENC_SUCCESS, "MBCSv MIB_IDX ENC SUCCESS"}, 
    { PSS_HDLN_MBCSV_MIB_IDX_DEC_FAIL, "MBCSv MIB_IDX DEC FAIL"}, 
    { PSS_HDLN_MBCSV_MIB_IDX_DEC_SUCCESS, "MBCSv MIB_IDX DEC SUCCESS"}, 
    { PSS_HDLN_MBCSV_ENC_MIB_OBJ_ID_DONE, "MBCSv MIB_OBJ_ID ENC DONE"}, 
    { PSS_HDLN_MBCSV_ENC_MIB_OBJ_ID_FAIL, "MBCSv MIB_OBJ_ID ENC FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_MIB_OBJ_ID_DONE, "MBCSv MIB_OBJ_ID DEC DONE"}, 
    { PSS_HDLN_MBCSV_DEC_MIB_OBJ_ID_FAIL, "MBCSv MIB_OBJ_ID DEC FAIL"}, 
    { PSS_HDLN_MBCSV_ENC_SEQ_NUM_DONE, "MBCSv SEQ_NUM ENC DONE"}, 
    { PSS_HDLN_MBCSV_ENC_SEQ_NUM_FAIL, "MBCSv SEQ_NUM ENC FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_SEQ_NUM_DONE, "MBCSv SEQ_NUM DEC DONE"}, 
    { PSS_HDLN_MBCSV_DEC_SEQ_NUM_FAIL, "MBCSv SEQ_NUM DEC FAIL"}, 
    { PSS_HDLN_MBCSV_ENC_IS_SYS_CLIENT_BOOL_DONE, "MBCSv IS_SYS_CLIENT BOOLEAN ENC DONE"}, 
    { PSS_HDLN_MBCSV_DEC_IS_SYS_CLIENT_BOOL_DONE, "MBCSv IS_SYS_CLIENT BOOLEAN DEC DONE"}, 
    { PSS_HDLN_MBCSV_ENC_ALT_PROFILE_DONE, "MBCSv ALT_PROFILE ENC DONE"}, 
    { PSS_HDLN_MBCSV_ENC_ALT_PROFILE_FAIL, "MBCSv ALT_PROFILE ENC FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_ALT_PROFILE_DONE, "MBCSv ALT_PROFILE DEC DONE"}, 
    { PSS_HDLN_MBCSV_DEC_ALT_PROFILE_FAIL, "MBCSv ALT_PROFILE DEC FAIL"}, 
    { PSS_HDLN_MBCSV_ENC_BMASK_FAIL, "MBCSv BMASK ENC FAIL"}, 
    { PSS_HDLN_MBCSV_ENC_BMASK_DONE, "MBCSv BMASK ENC DONE"}, 
    { PSS_HDLN_MBCSV_DEC_BMASK_FAIL, "MBCSv BMASK DEC FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_BMASK_DONE, "MBCSv BMASK DEC DONE"}, 
    { PSS_HDLN_MBCSV_TBL_ID_FLUSHED_BEFORE_POPULATING_RE_DATA, "MBCSv TBL FLUSHED BEFORE POPULATING RE DATA"}, 
    { PSS_HDLN_MBCSV_ASYNC_ENC_CALLBACK_EVT_START, "MBCSv ASYNC-UPDT ENC CALLBACK START"}, 
    { PSS_HDLN_MBCSV_ASYNC_ENC_CALLBACK_EVT_STATUS, "MBCSv ASYNC-UPDT ENC CALLBACK STATUS"}, 
    { PSS_HDLN_MBCSV_ASYNC_ENC_CALLBACK_INVALID_EVT, "MBCSv ASYNC-UPDT ENC CALLBACK EVT INVALID"}, 
    { PSS_HDLN_MBCSV_ASYNC_DEC_CALLBACK_EVT_START, "MBCSv ASYNC-UPDT DEC CALLBACK START"}, 
    { PSS_HDLN_MBCSV_ASYNC_DEC_CALLBACK_EVT_STATUS, "MBCSv ASYNC-UPDT DEC CALLBACK STATUS"}, 
    { PSS_HDLN_MBCSV_ASYNC_DEC_CALLBACK_INVALID_EVT, "MBCSv ASYNC-UPDT DEC CALLBACK EVT INVALID"}, 
    { PSS_HDLN_MBCSV_ASYNC_EVT_HANDLING_START, "MBCSv ASYNC-UPDT EVT HANDLING START"}, 
    { PSS_HDLN_MBCSV_ASYNC_EVT_HANDLING_STATUS, "MBCSv ASYNC-UPDT EVT HANDLING STATUS"}, 
    { PSS_HDLN_MBCSV_ASYNC_EVT_HANLDING_EVT_INVALID, "MBCSv ASYNC-UPDT EVT TO BE HANDLED IS INVALID"},
    { PSS_HDLN_GET_PROF_MIB_LIST_PER_PCN_OP_FAILED, "GET MIB LIST per PCN FAIL"}, 
    { PSS_HDLN_ZERO_MDS_DEST, "MDS Destination received is 0"}, 
    { PSS_HDLN_INVALID_PWE_CB, "Invalid PWE_CB pointer encountered"},
    { PSS_HDLN_OAA_NODE_NOT_FND_FOR_MDEST, "OAA pat node not found for MDEST"},
    { PSS_HDLN_TBL_NODE_NOT_FND_FOR_TBL_ID, "Table node not found in OAA hash-list"},
    { PSS_HDLN_OAA_DOWN_EVENT_FAILED, "pss_handle_oaa_down_event failed "},
    { PSS_HDLN_OAA_DOWN_EVENT_SUCCESS, "pss_handle_oaa_down_event success "},
    { PSS_HDLN_STDBY_OAA_DOWN_LIST_ADD_FAILED, "Failed to Add this OAA to STBY OAA_DOWN BUFFER "},
    { PSS_HDLN_STDBY_OAA_DOWN_LIST_ADD_SUCCESS, "OAA successfully added to STBY OAA_DOWN BUFFER "},
    { PSS_HDLN_STDBY_OAA_DOWN_LIST_DELETE_FAILED, "Failed to delete this OAA from STBY OAA_DOWN BUFFER "},
    { PSS_HDLN_STDBY_OAA_DOWN_LIST_DELETE_SUCCESS, "OAA successfully deleted from STBY OAA_DOWN BUFFER "},
    { PSS_HDLN_OAA_ENTRY_NOT_FOUND, "oaa entyr not found in oaa tree"},
    { PSS_HDLN_PSS_VERSION, "PSS Version: "},
    { PSS_HDLN_OAA_ACK_FAILED, "PSS Warmboot Playback Failure: OAA ACK sent failed " },
    { PSS_HDLN_REFORMAT_REQUEST, "PSS Reformat request type:" },
    { PSS_HDLN_PROFILE_FOUND, "PSS profile found: "},
    { PSS_HDLN_PWE_FOUND, "PSS PWE found: "},
    { PSS_HDLN_PWE_COUNT, "PSS PWE count: "},
    { PSS_HDLN_PCN_COUNT, "PSS PCN count: "},
    { PSS_HDLN_PCN_FOUND, "PSS PCN found: "},
    { PSS_HDLN_TABLE_COUNT, "PSS Table count: "},
    { PSS_HDLN_ERR_TABLE_FOUND, "PSS Error occured for Table"},
    { PSS_HDLN_STORE_REFORMATTING_FROM_1_TO_1EXT_START, "PSS Started Store Reformatting from 1 to 1Ext format"},
    { PSS_HDLN_STORE_REFORMATTING_FROM_1_TO_1EXT_SUCCESS, "PSS Store Reformatting from 1 to 1Ext format succesful"}, 
    { PSS_HDLN_STORE_REFORMATTING_FROM_1EXT_TO_HIGHER_START, "PSS Started Store Reformatting from 1Ext to Higher format"}, 
    { PSS_HDLN_STORE_REFORMATTING_FROM_1EXT_TO_HIGHER_SUCCESS, "PSS Store Reformatting from 1Ext to Higher format succesful"}, 
    { PSS_HDLN_STORE_REFORMATTING_FROM_HIGHER_TO_1EXT_START, "PSS Started Store Reformatting from higher to 1Ext format"}, 
    { PSS_HDLN_STORE_REFORMATTING_FROM_HIGHER_TO_1EXT_SUCCESS, "PSS Store Reformatting from higher to 1Ext format succesful"}, 
    { PSS_HDLN_MOVE_TABLE_DETAILS_FILE_FAIL, "PSS Move Table Details File Failed"},
    { PSS_HDLN_TABLE_DETAILS_EXISTS_ON_PERSISTENT_STORE, "PSS Table Details File found in persistent store"},
    { PSS_HDLN_RENAME_FILE_FAILED, "PSS renaming file failed"},
    { PSS_HDLN_INVALID_TABLE_DETAILS_HEADER_LEN, "PSS invalid table details header length"},
    { PSS_HDLN_INVALID_FILE_SIZE, "PSS invalid file size"},
    { PSS_HDLN_PROPERTIES_CHANGED_BUT_NOT_TABLE_VERSION, "Table properties changed with out change in table version"},
    { PSS_HDLN_PROFILE_CREATION_FAIL, "PSS creation of profile failed"},
    { PSS_HDLN_SCAN_DIR_FAILED, "Scanning the directory for Table IDs failed" },
    { PSS_HDLN_GET_NEXT_PROFILE_OP_FAILED, "Get next profile failed"},
    { PSS_HDLN_PROFILE_COPY_FAIL, "PSS copying of profile failed"},
    { PSS_HDLN_INVALID_PERSISTENT_STORE_FORMAT, "Invalid Persistent store format version"},
    { PSS_HDLN_STORE_REFORMATTING_FROM_HIGHER_TO_2_START, "PSS Started Store Reformatting from Higher to 2 format"}, 
    { PSS_HDLN_STORE_REFORMATTING_FROM_HIGHER_TO_2_SUCCESS, "PSS Store Reformatting from Higher to 2 format succesful"}, 
    { PSS_HDLN_MDS_ENC_FAILURE, "Incompatile Message format to encode"},
    { PSS_HDLN_MDS_DEC_FAILURE, "Incompatile Message format to decode"},
    { PSS_HDLN_INVALID_MBCSV_PEER_VER, "Incompatible MBCSv peer version"},
    { PSS_HDLN_READ_LIB_CONF_FAIL, "read on pssv_lib_conf failed while reading the entry: "},
    { 0,0 }
};

const NCSFL_STR pss_hdln2_set[] = 
{
    { PSS_HDLN_DEL_SPCN_PEND_WBREQ_LIST_NODE, "DELETING SPCN_WBREQ_PEND_LIST NODE"}, 
    { PSS_HDLN_SPCN_PEND_WBREQ_LIST_NODE_ALREADY_PRESENT, "SPCN_WBREQ_PEND_LIST NODE ALREADY PRESENT"}, 
    { PSS_HDLN_ADD_SPCN_PEND_WBREQ_LIST_NODE_ALLOC_FAIL, "SPCN_WBREQ_PEND_LIST ADD NODE ALLOC FAIL"}, 
    { PSS_HDLN_ADD_SPCN_PEND_WBREQ_LIST_NODE_PCN_ALLOC_FAIL, "SPCN_WBREQ_PEND_LIST ADD NODE PCN ALLOC FAIL"}, 
    { PSS_HDLN_ADD_SPCN_PEND_WBREQ_LIST_SUCCESS, "SPCN_WBREQ_PEND_LIST ADD NODE SUCCESS"}, 
    { PSS_HDLN_MBCSV_RE_NEW_PLBCK_SSN_DETAILS_COPIED, "MBCSV RE NEW PLCBK_SSNINFO DETAILS COPIED"}, 
    { PSS_HDLN_MBCSV_RE_NEW_PLBCK_SSN_DETAILS_MODIFIED, "MBCSV RE PLCBK_SSNINFO DETAILS MODIFIED"}, 
    { PSS_HDLN_MBCSV_RE_PLBCK_SSN_DETAILS_FLUSHED, "MBCSV RE PLCBK_SSNINFO DETAILS FLUSHED"}, 
    { PSS_HDLN_MBCSV_RE_PLBCK_SSN_DETAILS_FLUSHED_EVEN_WHEN_NO_PLBCK_ACTIVE, "MBCSV RE PLCBK_SSNINFO DETAILS FLUSHED EVEN WHEN NO PLBCK IN PROGRESS"}, 
    { PSS_HDLN_RE_SYNC_CKPT_EVT_TYPE, "MBCSV RE SYNC CKPT EVT TYPE"}, 
    { PSS_HDLN_RE_SYNC_INVALID_CKPT_EVT_TYPE, "MBCSV RE SYNC INVALID CKPT EVT TYPE"}, 
    { PSS_HDLN_RE_DIRECT_SYNC_CKPT_EVT_TYPE, "MBCSV RE DIRECT SYNC CKPT EVT TYPE"}, 
    { PSS_HDLN_RE_DIRECT_SYNC_INVALID_CKPT_EVT_TYPE, "MBCSV RE DIRECT SYNC INVALID CKPT EVT TYPE"}, 
    { PSS_HDLN_RE_SYNC_CKPT_EVT_POPULATE_FAIL, "MBCSV RE SYNC CKPT EVT POPULATE FAIL"}, 
    { PSS_HDLN_RE_SYNC_CKPT_EVT_SND_FAIL, "MBCSV RE SYNC CKPT EVT SND FAIL"}, 
    { PSS_HDLN_RE_SYNC_CKPT_EVT_SND_SUCCESS, "MBCSV RE SYNC CKPT EVT SND SUCCESS"}, 
    { PSS_HDLN_SELECT_RETURN_VAL, "SELECT RETURN VALUE:"}, 
    { PSS_HDLN_INVK_AMF_DISPATCH, "INVKNG AMF DISPATCH"}, 
    { PSS_HDLN_AMF_DISPATCH_RETURN_STATUS, "AMF DISPATCH RETURN STATUS:"}, 
    { PSS_HDLN_INVK_MBCSV_DISPATCH, "INVKNG MBCSV DISPATCH"}, 
    { PSS_HDLN_MBCSV_DISPATCH_RETURN_STATUS, "MBCSV DISPATCH RETURN STATUS:"}, 
    { PSS_HDLN_INVK_MBX_DISPATCH, "INVKNG MBX DISPATCH"}, 
    { PSS_HDLN_MBX_DISPATCH_RETURN_STATUS, "MBX DISPATCH RETURN STATUS:"}, 
    { PSS_HDLN_MBX_DISPATCH_RETURN_IN_HA_STATE, "MBX DISPATCH RETURNED IN HA_STATE:"}, 
    { PSS_HDLN_INVK_MBX_FLUSH_EVTS, "INVOKING FLUSH-OF-MBX EVTs"},
    { PSS_HDLN_RETURN_FROM_THREAD_HANDLER, "RETURNING FROM THREAD HANDLER ROUTINE"},
    { PSS_HDLN_FLUSH_MBX_EVT_TYPE, "FLUSHING MBX-EVT TYPE:"},
    { PSS_HDLN_ERR_AMF_SBY_TO_ACT_FAILED_COLD_SYNC, "AMF STANDBY-to-ACTIVE FAIL due to cold-sync-incomplete status"},
    { PSS_HDLN_ERR_AMF_SBY_TO_ACT_FAILED_WARM_SYNC, "AMF STANDBY-to-ACTIVE FAIL due to warm-sync-incomplete status"},
    { PSS_HDLN_MBCSV_ERR_IND_AMF_ERR_REPORT, "MBCSv ERR IND - AMF ERROR REPORT send status"},
    { PSS_HDLN_SEND_DATA_RESP_FAIL, "SEND DATA RESP FAIL"}, 
    { PSS_HDLN_MBCSV_ENC_DATA_RESP_PAYLOAD_FAIL, "ENC DATA RESP PAYLOAD FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_DATA_RESP_PAYLOAD_FAIL, "DEC DATA RESP PAYLOAD FAIL"}, 
    { PSS_HDLN_MBCSV_ENC_COLD_SYNC_RESP_PAYLOAD_FAIL, "ENC COLD SYNC RESP PAYLOAD FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_COLD_SYNC_RESP_PAYLOAD_FAIL, "DEC COLD SYNC RESP PAYLOAD FAIL"}, 
    { PSS_HDLN2_LIB_COMP_FILE_OPEN_FAIL, "COMP FILE OPEN FAIL"}, 
    { PSS_HDLN_ERR_AMF_SBY_TO_ACT_FAILED_LIBCONF_FILE_RELOAD, "AMF STANDBY-to-ACTIVE FAIL due to LIB_CONF FILE RELOAD FAIL"}, 
    { PSS_HDLN_ERR_AMF_SBY_TO_ACT_FAILED_SPCNLIST_FILE_RELOAD, "AMF STANDBY-to-ACTIVE FAIL due to SPCN_LIST FILE RELOAD FAIL"}, 
    { PSS_HDLN_MBCSV_ENC_LIBNAME_FAIL, "MBCSV ENC LIBNAME FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_LIBNAME_FAIL, "MBCSV DEC LIBNAME FAIL"}, 
    { PSS_HDLN_MBCSV_ENC_APPNAME_FAIL, "MBCSV ENC APPNAME FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_APPNAME_FAIL, "MBCSV DEC APPNAME FAIL"}, 
    { PSS_HDLN_PID_FILE_OPEN_FAIL, "PID FILE OPEN FAIL"}, 
    { PSS_HDLN_COMP_NAME_FILE_OPEN_FAIL, "COMP NAME FILE OPEN FAIL"}, 
    { PSS_HDLN_MBCSV_SEND_DATA_REQ_FAIL, "MBCSV SEND DATA REQ FAIL"}, 
    { PSS_HDLN_MBCSV_SEND_DATA_REQ_SUCCESS, "MBCSV SEND DATA REQ SUCCESS"}, 
    { PSS_HDLN_MBCSV_ENC_PARTIAL_DELETE_DONE_SUCCESS, "MBCSV ENC PARTIAL_DELETE_DONE SUCCESS"}, 
    { PSS_HDLN_MBCSV_ENC_PARTIAL_DELETE_DONE_FAIL, "MBCSV ENC PARTIAL_DELETE_DONE FAIL"}, 
    { PSS_HDLN_MBCSV_DEC_PARTIAL_DELETE_DONE_SUCCESS, "MBCSV DEC PARTIAL_DELETE_DONE SUCCESS"}, 
    { PSS_HDLN_MBCSV_DEC_PARTIAL_DELETE_DONE_FAIL, "MBCSV DEC PARTIAL_DELETE_DONE FAIL"}, 
    { PSS_HDLN2_RE_RESUME_ACTIVE_FUNCTIONALITY_INVOKED, "RESUME ACTIVE FUNCTIONALITY API INVOKED"}, 
    { PSS_HDLN2_RE_RESUME_ACTIVE_FUNCTIONALITY_COMPLETED, "RESUME ACTIVE FUNCTIONALITY API COMPLETED"}, 
    { PSS_HDLN2_CLI_REQ_REPLACE, "PSS CLI REQ - REPLACE"}, 
    { PSS_HDLN2_CLI_REQ_RELOADLIBCONF, "PSS CLI REQ - RELOADLIBCONF"}, 
    { PSS_HDLN2_CLI_REQ_RELOADSPCNLIST, "PSS CLI REQ - RELOADSPCNLIST"}, 
    { PSS_HDLN2_TEST_EXST_PRO, "PSS Test existing profile name"}, 
    { PSS_HDLN2_TEST_NEW_PRO, "PSS Test new profile name"}, 
    { PSS_HDLN2_TEST_TRIGGER_OP, "PSS Test Trigger Operation"}, 
    { PSS_HDLN2_TEST_CUR_PRO, "PSS Test current profile name"}, 
    { PSS_HDLN2_IGNORING_NON_PRIMARY_MDS_SVC_EVT, "PSS ignoring non-primary MDS SVC event"}, 
    { PSS_HDLN2_MAS_MDS_NO_ACTIVE, "PSS received MAS MDS_NO_ACTIVE event"}, 
    { PSS_HDLN2_MAS_MDS_NEW_ACTIVE, "PSS received MAS MDS_NEW_ACTIVE event"}, 
    { PSS_HDLN2_MAS_MDS_NEW_ACTIVE_AS_UP_EVT, "PSS received MAS MDS_NEW_ACTIVE event, which is being treated as MDS_UP"}, 
    { PSS_HDLN2_BAM_MDS_NO_ACTIVE, "PSS received BAM MDS_NO_ACTIVE event"}, 
    { PSS_HDLN2_BAM_MDS_NEW_ACTIVE, "PSS received BAM MDS_NEW_ACTIVE event"}, 
    { PSS_HDLN2_BAM_MDS_NEW_ACTIVE_AS_UP_EVT, "PSS received BAM MDS_NEW_ACTIVE event, which is being treated as MDS_UP"}, 
    { PSS_HDLN2_MIB_REQ_RETURN_NO_MAS, "PSS MIB Request response is NO_MAS" },
    { PSS_HDLN2_MIB_REQ_RETURN_VALUE, "PSS MIB Request response is :" },
    { PSS_HDLN2_MIB_REQ_RETRY_COUNT, "PSS MIB Request retrying...retry count is :" },
    { PSS_HDLN2_RDA_GIVEN_INIT_ROLE, "PSS RDA GIVEN INIT ROLE:" },
    { PSS_HDLN2_SCLR_SET_OCT_OBJECT_LENGTH_MISMATCH_WITH_STREAM_SPEC_MAX_LEN, "PSS SCLR SET fail, OCT object length mismatch with stream spec max length" },
    { PSS_HDLN2_SCLR_SET_OCT_OBJECT_LENGTH_MISMATCH_WITH_DISCRETE_OCT_STR_MAX_LEN, "PSS SCLR SET fail, OCT object length mismatch with discrete object max length" },
    { PSS_HDLN2_SET_FMAT_ID_NOT_SUPPORTED, "PSS SET fmat_id not supported" },
    { PSS_HDLN2_SET_VAR_INFO_LEN_NOT_SUPPORTED, "PSS SET fail, var_info length not supported" },
    { PSS_HDLN2_SET_NUM_INST_IDS_MISMATCH, "PSS SET fail, num_inst_ids mismatch" },
    { PSS_HDLN2_SET_PARAM_ID_MISMATCH_WITH_VAR_INFO_PARAM_ID, "PSS SET fail, param_id mismatch with var_info param_id" },
    { PSS_HDLN2_SET_OPEN_TEMP_FILE_FAIL, "PSS SET fail, open temp file fail" },
    { PSS_HDLN2_SET_COPY_TEMP_FILE_FAIL, "PSS SET fail, copy temp file fail" },
    { PSS_HDLN2_SET_COPY_TEMP_FILE_TO_ALT_PROFILE_FAIL, "PSS SET fail, copy temp file to alternate profile fail" },
    { PSS_HDLN2_SET_OPEN_MIB_FILE_FAIL, "PSS open MIB file fail, " },
    { PSS_HDLN2_DEL_INST_NODE_FROM_PAT_TREE_FAIL, "PSS delete inst_node from patricia tree fail, " },
    { PSS_HDLN2_SET_KEY_FROM_INST_IDS_FMAT_ID_NOT_SUPPORTED, "PSS SET-KEY-FROM-INST-IDS, fmat_id not supported" },
    { PSS_HDLN2_SET_KEY_FROM_INST_IDS_FMAT_INT_NOT_SUPPORTED, "PSS SET-KEY-FROM-INST-IDS fail, INT fmat type not supported" },
    { PSS_HDLN2_SET_KEY_FRM_INST_IDS_NUM_INST_IDS_MISMATCH, "PSS SET-KEY-FROM-INST-IDS fail, num_inst_ids mismatch" },
    { PSS_HDLN2_SETROW_DEC_PARAM_FAIL, "PSS SETROW op: ncsparm_dec_parm() fail" },
    { PSS_HDLN2_SETROW_DEC_PARAM_DONE_FAIL, "PSS SETROW op: ncsparm_dec_done() fail" },
    { PSS_HDLN2_SETALLROWS_DEC_INIT_FAIL, "PSS SETALLROWS op: ncsrow_dec_init() fail, return value is 0" },
    { PSS_HDLN2_SETALLROWS_DEC_ROW_INIT_FAIL, "PSS SETALLROWS op: ncsrow_dec_init() fail, return value is 0" },
    { PSS_HDLN2_SETALLROWS_DEC_ROW_INST_IDS_FAIL, "PSS SETALLROWS op: ncsrow_dec_inst_ids() fail" },
    { PSS_HDLN2_SETALLROWS_DEC_PARAM_FAIL, "PSS SETALLROWS op: ncsrow_dec_param() fail" },
    { PSS_HDLN2_SETALLROWS_ROW_DEC_DONE_FAIL, "PSS SETALLROWS op: ncsrow_dec_done() fail" },
    { PSS_HDLN2_SETALLROWS_DEC_DONE_FAIL, "PSS SETALLROWS op: ncssetallrows_dec_done() fail" },
    { PSS_HDLN2_REMOVEROWS_DEC_INIT_FAIL, "PSS REMOVEROWS op: ncsremrow_dec_init() fail, return value is 0" },
    { PSS_HDLN2_REMOVEROWS_DEC_INST_IDS_FAIL, "PSS REMOVEROWS op: ncsremrow_dec_inst_ids() fail" },
    { PSS_HDLN2_REMOVEROWS_DEC_DONE_FAIL, "PSS REMOVEROWS op: ncsremrow_dec_done() fail" },
    { PSS_HDLN2_WBREQ_EOP_STATUS_SEND_FAIL, "PSS warmboot playback status: send fail" },
    { PSS_HDLN2_WBREQ_EOP_STATUS_SEND_SUCCESS, "PSS warmboot playback status: send success" },
    { PSS_HDLN2_RE_TBL_REC_ADD_FAIL_MIB_DESC_NULL, "RE TBL ADD FAIL: REASON: mib description NULL" },
    { PSS_HDLN2_LIB_CONF_FILE_OPEN_IN_APPEND_MODE_FAIL, "PSS open pssv_lib_conf file in append-mode FAIL" },
    { PSS_HDLN2_SET_PARAM_VAL_FAIL_VALIDATION_TEST, "PSS SNMP REQ - PARAM_VAL FAIL VALIDATE TEST" },
    { 0,0 }
};

/******************************************************************************
 Logging stuff for Service Provider (MDS) 
 ******************************************************************************/

const NCSFL_STR pss_svc_prvdr_set[] = 
{
    { PSS_SP_MDS_INSTALL_FAILED,   "PSS Failed to install into MDS"        },
    { PSS_SP_MDS_INSTALL_SUCCESS,  "PSS installed into MDS"                },
    { PSS_SP_MDS_UNINSTALL_FAILED, "PSS Failed to uninstall from MDS"      },
    { PSS_SP_MDS_UNINSTALL_SUCCESS,"PSS uninstalled from MDS"              },
    { PSS_SP_MDS_SUBSCR_FAILED,    "PSS FAIL to subscribe to MDS evts"   },
    { PSS_SP_MDS_SUBSCR_SUCCESS,   "PSS subscribed to MDS evts"            },
    { PSS_SP_MDS_SND_MSG_SUCCESS,  "PSS sent MDS msg successfully"         },
    { PSS_SP_MDS_SND_MSG_FAILED,   "PSS FAIL to send MDS msg"            },
    { PSS_SP_MDS_RCV_MSG,          "PSS recvd MDS msg"                     },
    { PSS_SP_MDS_RCV_EVT,          "PSS recvd MDS evt"                     },
    { 0,0 }
};

/******************************************************************************
 Logging stuff for Locks 
 ******************************************************************************/

const NCSFL_STR pss_lock_set[] = 
{
    { PSS_LK_LOCKED,     "PSS locked"         },
    { PSS_LK_UNLOCKED,   "PSS unlocked"       },
    { PSS_LK_CREATED,    "PSS lock created"   },
    { PSS_LK_DELETED,    "PSS lock destroyed" },
    { 0,0 }
};

NCSFL_FMAT pss_fmat_set[] = 
  {
    { PSS_LID_HDLN,           NCSFL_TYPE_TI,    "PSS %s HEADLINE : %s\n"     },
    { PSS_LID_SVC_PRVDR_FLEX, NCSFL_TYPE_TILL,  "PSS %s SVC PRVDR: %s, %ld, %ld\n"     },
    { PSS_LID_LOCKS,          NCSFL_TYPE_TIL,   "PSS %s LOCK     : %s (%p)\n"},
    { PSS_LID_MEMFAIL,        "TIC",            "PSS %s MEM FAIL:%s in %s\n\n"}, 
    { PSS_LID_API,            NCSFL_TYPE_TI ,   "PSS %s API      : %s\n"     },
    { PSS_LID_STORE,         "TICLL",    "PSS %s STORE : %s, pcn=%s,pwe=%ld,tbl=%ld\n" },
    { PSS_LID_LIB,           "TIC",    "PSS %s : %s dl_error=%s\n" },
    { PSS_LID_LIB_INFO,      "TICC",   "PSS %s : %s %s, %s\n" },
    { PSS_LID_VAR_INFO,      "TILLLLLLLLLLLLLLLL", "PSS %s : %s TBL:%ld,PSSOFSET:%ld,PARMID:%ld,OFFSET:%ld,LEN:%ld,IS_INDX:%ld,ACCSS:%ld,STATUS:%ld,SETWHENDOWN:%ld,FMTID:%ld,OBJTYPE:%ld,INT_MIN:%ld,INT_MAX:%ld,OCT_MIN:%ld,OCT_MAX:%ld,IS_READONLY_PERSISTENT:%ld\n" },
    { PSS_LID_INFO,          "TICL",   "PSS %s : %s PCN=%s, Playback-from-BOM=%ld\n" },
    { PSS_LID_TBL_INFO,      "TICLLLLLLLL",   "PSS %s : %s TBL=%s, TBL_ID=%ld, RNK=%ld, NUM_OBJ=%ld, NUM_INSTS=%ld, MAYKEY=%ld, MAXROW=%ld, CAP=%ld, BMAP_LEN=%ld\n" },
    { PSS_LID_WBREQ_INFO,    "TICLLL",  "PSS %s : %s PCN=%s, pwe_id=%ld, is_system_client=%ld, tbl_id=%ld\n" },
    { PSS_LID_PLBCK,         "TICL",   "PSS %s : %s PCN=%s, pwe_id=%ld\n" },
    { PSS_LID_LAST_UPDT_INFO, "TICLL",   "PSS %s : %s PCN=%s, pwe_id=%ld, tbl_id=%ld\n" },
    { PSS_LID_CLIENT_ENTRY,  "TICL",   "PSS %s : %s PCN=%s, pwe_id=%ld\n" },
    { PSS_LID_SNMP_REQ,      "TILL",   "PSS %s : %s pwe_id=%ld, tbl_id=%ld\n" },
    { PSS_LID_MEMDUMP,       "TID",    "PSS %s : %s, [DATA]=%s\n" },
    { PSS_LID_HDLN_I,        NCSFL_TYPE_TIL, "PSS %s HEADLINE: %s data: %ld\n"},  
    { PSS_LID_HDLN2_I,       NCSFL_TYPE_TIL, "PSS %s HEADLINE: %s data: %ld\n"},  
    { PSS_LID_HDLN_C,        NCSFL_TYPE_TIC, "PSS %s HEADLINE: %s String: %s\n"},  
    { PSS_LID_HDLN_II,       NCSFL_TYPE_TILL,"PSS %s HEADLINE: %s data1: %ld, data2: %ld\n"},  
    { PSS_LID_HDLN_III,      NCSFL_TYPE_TILLL,"PSS %s HEADLINE: %s data1: %ld, data2: %ld data3:%ld\n"},  
    { PSS_LID_STR,           NCSFL_TYPE_TIC, "PSS %s: %s data: %s\n"}, 
    { PSS_LID_ERROR,         "TI",     "PSS %s ERROR: %s\n" }, /* 15 */ 
    { PSS_LID_ERR_I,         "TIL",    "PSS %s ERROR: %s, error: %ld\n" },
    { PSS_LID_ERR_II,        "TILL",   "PSS %s ERROR: %s, error: %ld error: %ld\n" },
    { PSS_LID_CSI,           "TILCCL", "PSS %s CSI DATA: %s\n\t\tcsiFlags: %ld,\n\t\tCompName: %s\n\t\tcsiName:%s\n\t\tHA State: %ld\n\n"}, 
    { PSS_LID_ST_CHG,        "TILL",   "PSS %s STATE CHG: %s\n\t\tCurr State:%ld, New State: %ld\n\n"},
    { PSS_LID_CONF_DONE,     "TIC",   "PSS %s CONF_DONE: %s PCN:%s\n\n"},
    { PSS_LID_BAM_REQ,     "TIC",   "PSS %s BAM REQ: %s PCN:%s\n\n"},
    { PSS_LID_PLBCK_SET_COUNT, "TICLLL",   "PSS %s %s PCN:%s TBL:%ld CPBLTY:%ld CNT:%ld\n\n"},
    { PSS_LID_TBL_BIND, "TICL",   "PSS %s %s PCN:%s TBL:%ld \n\n"},
    { PSS_LID_TBL_UNBIND, "TIL",   "PSS %s %s TBL:%ld \n\n"},
    { PSS_LID_WBREQ_I, "TICL",   "PSS %s %s PCN=%s, PWE=%ld \n\n"},
    { PSS_LID_WBREQ_II, "TIL",   "PSS %s %s tbl_id=%ld \n\n"},
    { PSS_LID_OAA_ACK_EVT, "TIL",   "PSS %s %s seq_num=%ld \n\n"},
    { PSS_LID_NO_CB,          NCSFL_TYPE_TIC,   "PSS %s HEADLINE : %s MDS DEST: %s\n"     },
    { PSS_LID_TBL_DTLS,      "TILLLLLLLL","PSS %s : %s \nDetails from Persistent Store:\n\t Table Version: %ld, Max Row Len: %ld Max Key Len: %ld Bitmap Len: %ld \nDetails from Loaded Libraries:\n\t Table Version: %ld, Max Row Len: %ld Max Key Len: %ld Bitmap Len: %ld \n"},
    { PSS_LID_HDLN_CLL,       NCSFL_TYPE_TICLL, "PSS %s HEADLINE: %s data1: %s, data2: %ld data3:%ld\n"},
    { 0, 0, 0 }
  };

/******************************************************************************
 Logging stuff for API 
 ******************************************************************************/
const NCSFL_STR pss_api_set[] = 
  {
    { PSS_API_SVC_DESTROY,    "ncspss_lm(DESTROY)"        },
    { PSS_API_SVC_GET,        "ncspss_lm(GET)"            },
    { PSS_API_SVC_SET,        "ncspss_lm(SET)"            },
    { PSS_API_TBL_REG,        "ncspss_ss(TBL_REG)"        },
    { PSS_API_TBL_UNREG,      "ncspss_ss(TBL_UNREG)"      },
    { PSS_API_ROW_REG,        "ncspss_ss(ROW_REG)"        },
    { PSS_API_ROW_UNREG,      "ncspss_ss(ROW_UNREG)"      },
    { PSS_API_AMF_COMP,       "pss_amf_componentize()"    },

    { 0,0 }
  };

/******************************************************************************
 Logging stuff for Mem Fail 
 ******************************************************************************/
const NCSFL_STR pss_memfail_set[] = 
{
    { PSS_MF_MABMSG_ALLOC_FAILED, "Failed to create MAB MSG"},
    { PSS_MF_HDLN_CB_ALLOC_FAILURE,   "PSS No memory to allocate for PSS TBL"            },
    { PSS_MF_TBL_INFO_ALLOC_FAIL, "PSS PSS_MIB_TBL_INFO alloc FAIL" },
    { PSS_MF_VAR_INFO_ALLOC_FAIL, "PSS PSS_VAR_INFO alloc FAIL" },
    { PSS_MF_PSS_TBL_RNK_ALLOC_FAIL, "PSS PSS_MIB_TBL_RANK alloc FAIL" },
    { PSS_MF_PSS_CB_ALLOC_FAIL, "PSS CB alloc FAIL" },
    { PSS_MF_PSS_SPCN_LIST, "PSS SPCN_LIST alloc FAIL" },
    { PSS_MF_PCN_STRING_ALLOC_FAIL, "PSS PCN string alloc FAIL" },
    { PSS_MF_PWE_CB_ALLOC_FAIL, "PSS PWE_CB alloc FAIL" },
    { PSS_MF_TBL_REC_ALLOC_FAIL, "PSS PSS_TBL_REC alloc FAIL" },
    { PSS_MF_MIB_TBL_DATA_ALLOC_FAIL, "PSS PSS_MIB_TBL_DATA alloc FAIL" },
    { PSS_MF_OCT_ALLOC_FAIL, "PSS Octet alloc FAIL" },
    { PSS_MF_MIB_ROW_DATA_ALLOC_FAIL, "PSS MIB Row data alloc FAIL" },
    { PSS_MF_SCLR_MIB_ROW_ALLOC_FAIL, "PSS Scalar MIB Row alloc FAIL" },
    { PSS_MF_SPCN_LIST_ALLOC_FAIL, "PSS SPCN_LIST alloc FAIL" },
    { PSS_MF_PSS_WBSORT_ENTRY_ALLOC_FAIL, "PSS PSS_WBSORT_ENTRY alloc FAIL" },
    { PSS_MF_PSS_ODSORT_ENTRY_ALLOC_FAIL, "PSS PSS_ODSORT_ENTRY alloc FAIL" },
    { PSS_MF_PSS_ODSORT_TBL_REC_ALLOC_FAIL, "PSS PSS_ODSORT_TBL_REC alloc FAIL" },
    { PSS_MF_OAA_ENTRY_ALLOC_FAIL, "PSS OAA_ENTRY alloc FAIL" },
    { PSS_MF_CLIENT_ENTRY_ALLOC_FAIL, "PSS CLIENT_ENTRY alloc FAIL" },
    { PSS_MF_MIB_INST_IDS_ALLOC_FAIL, "PSS MIB_INST_IDS alloc FAIL" },
    { PSS_MF_OAA_CLT_ID_ALLOC_FAIL, "PSS OAA_CLT_ID alloc FAIL" },
    { PSS_MF_PSS_QELEM_ALLOC_FAIL, "PSS PSS_QELEM alloc FAIL" },
    { PSS_MF_MMGR_BUFFER_ALLOC_FAIL, "PSS MMGR Buffer alloc FAIL" },
    { PSS_MF_MMGR_SPCN_WBREQ_PEND_LIST_FAIL, "PSS MMGR SPCN_WBREQ_PEND_LIST alloc FAIL" },
    { PSS_MF_CSI_NODE_ALLOC_FAILED, "PSS malloc FAIL for PSS_CSI_NODE"},
    { PSS_MF_UBA_ENC_INIT_SPACE_FAIL, "PSS ncs_enc_init_space() call FAIL"},
    { PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL, "PSS ncs_enc_reserve_space() call FAIL"},
    { PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL, "PSS ncs_dec_flatten_space() call FAIL"},
    { PSS_MF_UBA_ENC_OCTETS_FAIL, "PSS ncs_encode_n_octets_in_uba() call FAIL"},
    { PSS_MF_UBA_DEC_OCTETS_FAIL, "PSS ncs_decode_n_octets_from_uba() call FAIL"},
    { PSS_MF_MAB_PSS_TBL_LIST_ALLOC_FAIL, "PSS MAB_PSS_TBL_LIST alloc FAIL"},
    { PSS_MF_MAB_PSS_WARMBOOT_REQ_ALLOC_FAIL, "PSS MAB_PSS_WARMBOOT_REQ alloc FAIL"},
    { PSS_MF_RFRSH_TBL_LIST_ALLOC_FAIL, "PSS_RFRSH_TBL_LIST  alloc FAIL"},
    { PSS_MF_NCSMIB_TBL_INFO_ALLOC_FAIL, "PSS NCSMIB_TBL_INFO alloc FAIL"},
    { PSS_MF_TBL_DETAILS_HDR_ALLOC_FAIL, "PSS PSS_TABLE_DETAILS_HEADER alloc FAIL"},
    { 0,0 }
};

/******************************************************************************
 Logging stuff for persistent store
 ******************************************************************************/

const NCSFL_STR pss_store_set[] = 
  {
    { PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL, "MIB search in persistent store FAIL"},
    { PSS_MIB_EXISTS_IN_INMEMORY_STORE, "MIB entries found in in-memory store"},
    { PSS_MIB_EXISTS_ON_PERSISTENT_STORE, "MIB entries found on persistent store"},
    { PSS_MIB_OPEN_FAIL, "MIB open in persistent store FAIL"},
    { PSS_MIB_READ_FAIL, "MIB read in persistent store FAIL"},
    { PSS_MIB_FILE_SIZE_OP_FAIL, "Can't get MIB File size in persistent store" },
    { PSS_MIB_WRITE_FAIL, "Can't write to MIB in persistent store"},
    { PSS_MIB_CLOSE_FAIL, "MIB close in persistent store FAIL"},
    { PSS_MIB_SEEK_FAIL, "File seek on persistent store FAIL"},
    { PSS_MIB_TABLE_DETAILS_READ_FAIL, "Read on Table details file in persistent store FAIL"},
    { 0,0 }
  };

/******************************************************************************
 Logging stuff for MIB Library operations
 ******************************************************************************/

const NCSFL_STR pss_lib_set[] = 
  {
    { PSS_LIB_OP_CONF_FILE_OPEN_FAIL, "pssv_lib_conf file open FAIL"},
    { PSS_LIB_OP_SYM_LKUP_ERROR, "Symbol lookup FAIL"},
    { PSS_LIB_OP_SYM_RET_ERROR, "Symbol invocation returned error"},
    { 0,0 }
};

/******************************************************************************
 Errors and associated data
 ******************************************************************************/
const NCSFL_STR pss_err_set[] = 
{
   { PSS_ERR_AMF_DISPATCH_FAILED, "PSS pss_mbx_amf_process(): saAmfDispatch() FAIL(saf error):"},
   { PSS_ERR_DO_EVTS_FAILED,"PSS pss_mbx_amf_process(): pss_do_evts() FAIL(error):"},
   { PSS_ERR_DO_EVT_FAILED,"PSS pss_mbx_amf_process(): pss_do_evt() FAIL(error):"},
   { PSS_ERR_MBX_RCV_MSG_FAIL,"PSS pss_mbx_amf_process(): m_PSS_RCV_MSG() FAIL(error):"},
   { PSS_ERR_SELECT_FAILED, "PSS pss_mbx_amf_process(): select() returned <= 0"},
   { PSS_ERR_AMF_ATTRIBS_INIT_FAILED, "PSS pss_amf_componentize(): ncs_app_amf_attribs_init() FAIL(error): "},        
   { PSS_ERR_AMF_INITIALIZE_FAILED, "PSS pss_amf_componentize(): ncs_app_amf_initialize() FAIL(error): "}, 
   { PSS_ERR_AMF_CSI_REMOVE_ALL_FAILED,"PSS pss_amf_prepare_will(): pss_amf_csi_remove_all() FAIL(saf error): "}, 
   { PSS_ERR_VDEST_DESTROY_FAILED,"PSS pss_amf_prepare_will(): NCSVDA_VDEST_DESTROYoperation for ncsvda_api() FAIL(error): "}, 
   { PSS_ERR_APP_AMF_FINALIZE_FAILED,"PSS pss_amf_prepare_will(): ncs_app_amf_finalize() FAIL(saf_error): "}, 
   { PSS_ERR_AMF_CSI_FLAGS_ILLEGAL, "PSS pss_amf_csi_set(): CSI Assignment happened with illegal flag values (flag values): "}, 
   { PSS_ERR_AMF_ENVID_ILLEGAL, "PSS pss_amf_csi_new(): Invalid Environment id is received from the CSI Attributes (envid):"}, 
   { PSS_ERR_CSI_ADD_ONE_FAILED, "PSS pss_amf_csi_new(): Adding a new CSI FAIL(error, envid):"}, 
   { PSS_ERR_MDS_UNINSTALL_FAILED,"PSS pss_amf_csi_remove_one(): MDS_UNINSTALL operation for ncsmds_api() FAIL(error, envid)"}, 
   { PSS_ERR_PSSTBL_DESTROY_FAILED, "PSS pss_amf_csi_remove_one(): pss_pwe_cb_destroy() FAIL(error, envid)"}, 
   { PSS_ERR_CSI_DELINK_FAILED, "PSS pss_amf_csi_remove_one(): pss_amf_csi_list_delink() FAIL for envid:"}, 
   { PSS_ERR_MDS_PWE_QUERY_FAILED,"PSS pss_amf_csi_vdest_role_change(): MDS_QUERY_PWE operation for ncsmds_api() FAIL(error): "}, 
   { PSS_ERR_MDS_VDEST_ROLE_CHG_FAILED, "PSS pss_amf_csi_vdest_role_change(): NCSVDA_VDEST_CHG_ROLE operation for ncsvda_api() FAIL(error):"}, 
   { PSS_ERR_COMP_NAME_FOPEN_FAILED, "PSS pss_amf_sigusr1_handler(): fopen() FAIL for m_PSS_COMP_NAME_FILE"}, 
   { PSS_ERR_COMP_NAME_FSCANF_FAILED, "PSS pss_amf_sigusr1_handler(): fscanf() to read the PSS AMF component name FAIL"},
   { PSS_ERR_COMP_NAME_SETENV_FAILED, "PSS pss_amf_sigusr1_handler(): setting the SA_AMF_COMPONENT_NAME environment variable FAIL"}, 
   { PSS_ERR_IPC_CREATE_FAILED, "PSS pss_lib_init(): m_NCS_IPC_CREATE() FAIL"}, 
   { PSS_ERR_IPC_ATTACH_FAILED, "PSS pss_lib_init(): m_NCS_IPC_ATTACH() FAIL"}, 
   { PSS_ERR_SIG_INSTALL_FAILED, "PSS pss_lib_init(): ncs_app_signal_install() FAIL"}, 
   { PSS_ERR_TASK_CREATE_FAILED, "PSS pss_lib_init(): m_NCS_TASK_CREATE() FAIL"}, 
   { PSS_ERR_TASK_START_FAILED, "PSS pss_lib_init(): m_NCS_TASK_START() FAIL"}, 
   { PSS_ERR_TGT_SVSC_CREATE_FAILED, "PSS pss_lib_init(): pss_ts_create() FAIL(error): "}, 
   { PSS_ERR_DEF_ENV_CREATE_FAILED, "PSS pss_lib_init(): Default Env Creation pss_create() FAIL(error): "}, 
   { PSS_ERR_TGT_SVSC_LM_CREATE_FAILED, "PSS pss_ts_create(): NCS_PSSTS_LM_OP_CREATE operation for ncspssts_lm() FAIL(error): "}, 
   { PSS_ERR_VDEST_CREATE_FAILED, "PSS pss_create(): NCSVDA_VDEST_CREATE operation for ncsvda_api() FAIL(error): "}, 
   { PSS_ERR_LM_CREATE_FAILED, "PSS pss_create(): NCSPSS_LM_OP_CREATE operation for ncspss_lm() FAIL(error): "}, 
   { PSS_ERR_TGT_SVCS_DESTROY_FAILED, "PSS pss_ts_destroy(): NCS_PSSTS_LM_OP_DESTROY operation for ncspssts_lm() FAIL(error): "}, 
   { PSS_ERR_SPIR_MDS_LOOKUP_CREATE_FAILED, "PSS pss_pwe_cb_init(): PSS_ERR_SPIR_MDS_LOOKUP_CREATE_FAILED operation for m_MDS_SP_ABST_NAME FAIL(error, envid): "}, 
   { PSS_ERR_SPIR_MAA_LOOKUP_CREATE_FAILED, "PSS pss_pwe_cb_init(): PSS_ERR_SPIR_MDS_LOOKUP_CREATE_FAILED operation for m_MAA_SP_ABST_NAME FAIL(error, envid): "}, 
   { PSS_ERR_SPIR_OAA_LOOKUP_CREATE_FAILED, "PSS pss_svc_create(): PSS_ERR_SPIR_MDS_LOOKUP_CREATE_FAILED operation for m_OAA_SP_ABST_NAME FAIL(error, envid): "}, 
   { PSS_ERR_VDEST_ROLE_CHANGE_FAILED, "PSS pss_amf_csi_install(): pss_amf_csi_vdest_role_change() FAIL(error): "}, 
   { PSS_ERR_MDS_PWE_INSTALL_FAILED,"PSS pss_amf_csi_install(): MDS_INSTALL operation for ncsmds_api() FAIL(error): "},  
   { PSS_ERR_MDS_SUBSCRIBE_FAILED, "PSS pss_amf_csi_install(): MDS_SUBSCRIBE operation for ncsmds_api() FAIL(error): "}, 
   { PSS_ERR_PWE_CB_INIT_FAILED, "PSS pss_amf_csi_install(): pss_pwe_cb_init() FAIL(error, envid)"}, 
   { PSS_ERR_DEF_ENV_INIT_FAILED, "PSS pss_svc_create(): pss_amf_csi_install() FAIL for default env and HAPS state (envid, HAPS state):"}, 
   { PSS_ERR_SPIR_MAA_REL_INST_FAILED,"PSS pss_pwe_cb_destroy(): NCS_SPIR_REQ_REL_INST for m_MAA_SP_ABST_NAME (error, envid):"}, 
   { PSS_ERR_SPIR_MDS_REL_INST_FAILED, "PSS pss_pwe_cb_destroy(): NCS_SPIR_REQ_REL_INST for m_MDS_SP_ABST_NAME (error, envid):"}, 
   { PSS_ERR_AMF_INVALID_PWE_ID, "PSS pss_amf_csi_install(): received in valid Env-ID" }, 
   { PSS_ERR_AMF_INVALID_HA_STATE, "PSS pss_amf_csi_install(): received invalid HA state"}, 
   { PSS_ERR_CREATE_LM_API_NOT_YET_CALLED, "PSS pss_amf_csi_install(): PSS_CB handle is zero (CREATE LM API did not get called)"}, 
   { PSS_ERR_OAA_TREE_DESTROY_FAILED, "PSS pss_pwe_cb_destroy(): ncs_patricia_tree_destroy() FAIL for OAA tree"}, 
   { PSS_ERR_CLIENT_TABLE_TREE_DESTROY_FAILED, "PSS  pss_pwe_cb_destroy(): ncs_patricia_tree_destroy() FAIL for Client table tree"}, 
   { PSS_ERR_AMF_CSI_ATTRIBS_COUNT_INVALID, "PSS pss_amf_csi_attrib_decode(): Ivalid CSI Attribs count received (csiDescriptor.csiAttr.number)"}, 
   { PSS_ERR_AMF_CSI_ATTRIBS_NULL, "PSS pss_amf_csi_attrib_decode(): csiDescriptor.csiAttr.attr is NULL"},
   { PSS_ERR_AMF_CSI_ATTRIBS_NAME_OR_ATTRIB_VAL_NULL, "PSS pss_amf_csi_attrib_decode(): csiDescriptor.csiAttr.attr->attrName or csiDescriptor.csiAttr.attr->attrValue is NULL"}, 
   { PSS_ERR_AMF_CSI_SSCANF_FAILED, "PSS pss_amf_csi_attrib_decode(): strtol() FAIL(errno, decoded_env)"},
   { PSS_ERR_AMF_CSI_DECODED_ENV_ID_OUTOF_REACH, "PSS pss_amf_csi_attrib_decode(): Decoded env is out of reach (decoded_env)"}, 
   { PSS_ERR_AMF_CSI_ATTRIBS_INCONCISTENT, "PSS pss_amf_csi_set_one(): pss_amf_csi_list_find_envid() FAIL(env_id)"},
   { PSS_ERR_AMF_CSI_ATTRIBS_NAME_INVALID, "PSS pss_amf_csi_set_one(): strcmp() FAIL beacause of invalid attribute name"},
   { PSS_ERR_MBCSV_DISPATCH_FAILED, "MBCSV DISPATCH FAIL"},
   { PSS_ERR_AMF_TIMER_FAILED, "PSS amf initialisation timer failed"},
   { PSS_ERR_RDA_INIT_ROLE_GET_FAILED, "PSS RDA INIT ROLE GET FAILED"},
   { PSS_ERR_TS_CREATE_FAILED, "Target Service Instance create failed"},
   { 0,0 }
};


const NCSFL_STR pss_info_set[] = 
  {
    { PSS_INFO_SPCN, "PSS SPCN INFO"},
    { PSS_INFO_SPCN_ENC_DONE, "PSS SPCN INFO ENC DONE"},
    { PSS_INFO_SPCN_DEC_DONE, "PSS SPCN INFO DEC DONE"},
    { PSS_INFO_NO_CLIENTS_FOUND_IN_PROFILE, "PSS NO CLIENTS FOUND IN PROFILE"},
    { 0,0 }
  };

const NCSFL_STR pss_lib_info_set[] = 
  {
    { PSS_LIB_INFO_CONF_FILE_ENTRY, "PSS pssv_lib_conf entry :"},
    { PSS_LIB_INFO_CONF_FILE_ENTRY_LOADED, "PSS MIB symbol loaded:"},
    { PSS_LIB_INFO_CONF_FILE_ENTRY_MBCSV_ENC_DONE, "PSS MBCSV LIB_CONF FILE ENTRY ENC DONE:"},
    { PSS_LIB_INFO_CONF_FILE_ENTRY_MBCSV_DEC_DONE, "PSS MBCSV LIB_CONF FILE ENTRY DEC DONE:"},
    { 0,0 }
  };

const NCSFL_STR pss_var_info_set[] = 
  {
    { PSS_VAR_INFO_DETAILS, "TBL_VAR_INFO:"},
    { 0,0 }
  };

const NCSFL_STR pss_tbl_info_set[] = 
  {
    { PSS_TBL_DESC_INFO, "PSS TBL-REG INFO"},
    { 0,0 }
  };

const NCSFL_STR pss_wbreq_set[] = 
  {
    { PSS_WBREQ_RCVD_INFO, "PSS WBREQ RCVD INFO"},
    { PSS_WBREQ_RCVD_TBL_IS_NOT_PERSISTENT, "PSS WBREQ RCVD TBL is not persistent"},
    { PSS_WBREQ_RCVD_PROCESS_START, "PSS WBREQ processing STARTS"},
    { PSS_WBREQ_RCVD_PROCESS_END, "PSS WBREQ processing ENDS"},
    { PSS_WBREQ_RCVD_PROCESS_SKIP, "PSS WBREQ processing SKIPPED"},
    { PSS_WBREQ_RCVD_PROCESS_ABORT, "PSS WBREQ processing ABORTED"},
    { PSS_WBREQ_RCVD_PLBCK_OPTION_SET_TO_BAM, "PSS WBREQ : SPCN source set to BAM"},
    { PSS_WBREQ_RCVD_PLBCK_OPTION_SET_TO_PSS, "PSS WBREQ : SPCN source set to PSS"},
    { 0,0 }
  };


const NCSFL_STR pss_bam_req_set[] = 
  {
    { PSS_BAM_REQ_SENT, "PSS BAM REQ SENT"},
    { PSS_BAM_REQ_FAIL_BAM_NOT_ALIVE, "PSS BAM REQ SEND FAIL - REASON BAM NOT ALIVE"},
    { PSS_BAM_REQ_FAIL_MDS_ERROR, "PSS BAM REQ SEND FAIL - REASON MDS ERROR"},
    { 0,0 }
  };

const NCSFL_STR pss_plbck_set_count_set[] = 
  {
    { PSS_PLBCK_SET_COUNT, "ALL MIB EVTS"},
    { 0,0 }
  };

const NCSFL_STR pss_tbl_bind_set[] = 
  {
    { PSS_TBL_BIND_RCVD, "TBL-BIND RCVD"},
    { PSS_TBL_BIND_MIB_DESC_NULL, "TBL-BIND : MIB-DESC is NULL, so it's a non-persistent MIB"},
    { PSS_TBL_BIND_DONE, "TBL-BIND DONE"},
    { 0,0 }
  };

const NCSFL_STR pss_tbl_unbind_set[] = 
  {
    { PSS_TBL_UNBIND_RCVD, "TBL-UNBIND RCVD"},
    { PSS_TBL_UNBIND_FND_OAA_CLT_NODE, "TBL-UNBIND OP: oaa_clt_node found"},
    { PSS_TBL_UNBIND_SUCCESS, "TBL-UNBIND OP SUCCESS"},
    { PSS_TBL_UNBIND_IS_NOT_PERSISTENT_TABLE, "TBL-UNBIND OP: TBL is not persistent"},
    { 0,0 }
  };

const NCSFL_STR pss_wbreq_1_set[] = 
  {
    { PSS_WBREQ_I_PROCESS_SORTED_LIST_START, "process sorted-tbl-list START"},
    { PSS_WBREQ_I_PROCESS_SORTED_LIST_END, "process sorted-tbl-list END"},
    { PSS_WBREQ_I_PROCESS_SORTED_LIST_ABORT, "process sorted-tbl-list ABORT"},
    { 0,0 }
  };

const NCSFL_STR pss_wbreq_2_set[] = 
  {
    { PSS_WBREQ_II_SORT_FNC_TBL_DATA_AVAILABLE, "Sort Fnc: Table data available in persistent store"},
    { PSS_WBREQ_II_TBL_ADDED_TO_SORT_DB, "Table added to Sort-database"},
    { PSS_WBREQ_II_SORT_FNC_SPCN_TBL_DATA_NOT_AVAILABLE, "Sort Fnc: SPCN Table data not found in persistent store"},
    { PSS_WBREQ_II_SORT_FNC_TBL_DATA_NOT_AVAILABLE, "Table data not found in persistent store"},
    { PSS_WBREQ_II_INPUT_TBL_NOT_PERSISTENT, "Input Table is NOT persistent"},
    { PSS_WBREQ_II_SORTING_INPUT_TBL, "Sorting Input Table"},
    { PSS_WBREQ_II_SORT_FNC_INPUT_TBL_FND_IN_PCN_NODE, "Input Table rec fnd in pcn_node"},
    { PSS_WBREQ_II_SORT_FNC_INPUT_TBL_NOT_FND_IN_PCN_NODE, "Input Table rec not fnd in pcn_node"},
    { PSS_WBREQ_II_PROCESS_TBL_OF_SORT_DB, "Processing TBL of Sort-DB"},
    { 0,0 }
  };

const NCSFL_STR pss_oaa_ack_evt_set[] = 
  {
    { PSS_OAA_ACK_SEND_SUCCESS, "OAA-ACK SEND SUCCESS"},
    { PSS_OAA_ACK_SEND_FAIL, "OAA-ACK SEND FAIL"},
    { 0,0 }
  };

const NCSFL_STR pss_conf_done_set[] = 
  {
    { PSS_CONF_DONE_RCVD, "BAM CONF_DONE RCVD"},
    { PSS_CONF_DONE_FINISHED, "BAM CONF_DONE FINISH"},
    { PSS_CONF_DONE_FAIL, "BAM CONF_DONE MSG PROCESSING FAIL"},
    { 0,0 }
  };

const NCSFL_STR pss_plbck_set[] = 
  {
    { PSS_PLBCK_REQ, "PSS PLAYBACK REQ"},
    { PSS_PLBCK_DONE, "PSS PLAYBACK DONE"},
    { PSS_PLBCK_REQ_TO_BAM, "PSS PLAYBACK REQ to BAM"},
    { PSS_PLBCK_LAST_UPDATE_BIT_SET, "PSS PLAYBACK LAST UPDATE BIT SET"},
    { 0,0 }
  };

const NCSFL_STR pss_last_updt_info_set[] = 
  {
    { PSS_LAST_UPDT_IS_SET, "PSS LAST UPDT bit set for SET event"},
    { PSS_LAST_UPDT_IS_SETROW, "PSS LAST UPDT bit set for SETROW event"},
    { PSS_LAST_UPDT_IS_SETALLROWS, "PSS LAST UPDT bit set for SETALLROWS event"},
    { PSS_LAST_UPDT_IS_REMOVEROWS, "PSS LAST UPDT bit set for REMOVEROWS event"},
    { 0,0 }
  };

const NCSFL_STR pss_client_entry_set[] = 
  {
    { PSS_CLIENT_ENTRY_ADD, "PSS CLIENT ENTRY ADD"},
    { PSS_CLIENT_ENTRY_DEL, "PSS CLIENT ENTRY DEL"},
    { 0,0 }
  };

const NCSFL_STR pss_snmp_req_set[] = 
  {
    { PSS_SNMP_REQ_SET, "PSS SNMP REQ SET"},
    { PSS_SNMP_REQ_SETROW, "PSS SNMP REQ SETROW"},
    { PSS_SNMP_REQ_MOVEROW, "PSS SNMP REQ MOVEROW"},
    { PSS_SNMP_REQ_SETALLROWS, "PSS SNMP REQ SETALLROWS"},
    { PSS_SNMP_REQ_REMOVEROWS, "PSS SNMP REQ REMOVEROWS"},
    { PSS_SNMP_REQ_TBL_ID_TOO_LARGE, "PSS MIB table id greater than allowable table-ID"    },
    { PSS_SNMP_REQ_NO_TBL_REG, "SNMP REQ RCVD for non-bound MIB table"    },
    { PSS_SNMP_REQ_ZERO_INST_LEN_OR_NULL_INST_IDS, "SNMP REQ ZERO INST-IDS-LEN or NULL INST-IDS"    },
    { PSS_SNMP_REQ_ADD_ROW_FAIL, "SNMP REQ ADD ROW FAIL"    },
    { PSS_SNMP_REQ_TBL_REC_NOT_INITED, "SNMP REQ : TBL_REC NOT INITED"    },
    { PSS_SNMP_REQ_INDEX_VALIDATE_FAIL, "SNMP REQ : INDEX validate FAIL"    },
    { PSS_SNMP_REQ_PARAMVAL_VALIDATE_FAIL, "SNMP REQ : PARAM_VAL validate FAIL"    },
    { 0,0 }
  };

const NCSFL_STR pss_memdump_set[] = 
  {
    { PSS_NCSMIB_ARG_TBL_DUMP, "PSS NCSMIB_ARG TBL DATA"},
    { PSS_NCSMIB_ARG_INSTID_DUMP, "Instance_ids"},
    { PSS_NCSMIB_ARG_PARAM_INFO_DUMP, "Param_info"},
    { PSS_NCSMIB_ARG_INT_INFO_DUMP, "Int_param_val"},
    { PSS_NCSMIB_ARG_OCT_INFO_DUMP, "Oct_param_val"},
    { 0,0 }
  };

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET pss_str_set[] = 
{
    { PSS_FC_HDLN,           0, (NCSFL_STR*) pss_hdln_set      },
    { PSS_FC_HDLN2,          0, (NCSFL_STR*) pss_hdln2_set     },
    { PSS_FC_SVC_PRVDR_FLEX, 0, (NCSFL_STR*) pss_svc_prvdr_set },
    { PSS_FC_LOCKS,          0, (NCSFL_STR*) pss_lock_set      },
    { PSS_FC_MEMFAIL,        0, (NCSFL_STR*) pss_memfail_set   },
    { PSS_FC_API,            0, (NCSFL_STR*) pss_api_set       },
    { PSS_FC_STORE,          0, (NCSFL_STR*) pss_store_set     },
    { PSS_FC_LIB,            0, (NCSFL_STR*) pss_lib_set       },
    { PSS_FC_INFO,           0, (NCSFL_STR*) pss_info_set      },
    { PSS_FC_LIB_INFO,       0, (NCSFL_STR*) pss_lib_info_set  },
    { PSS_FC_VAR_INFO,       0, (NCSFL_STR*) pss_var_info_set  },
    { PSS_FC_TBL_INFO,       0, (NCSFL_STR*) pss_tbl_info_set  },
    { PSS_FC_WBREQ_INFO,     0, (NCSFL_STR*) pss_wbreq_set     },
    { PSS_FC_PLBCK,          0, (NCSFL_STR*) pss_plbck_set     },
    { PSS_FC_LAST_UPDT_INFO, 0, (NCSFL_STR*) pss_last_updt_info_set },
    { PSS_FC_CLIENT_ENTRY,   0, (NCSFL_STR*) pss_client_entry_set   },
    { PSS_FC_SNMP_REQ,       0, (NCSFL_STR*) pss_snmp_req_set  },
    { PSS_FC_MEMDUMP,        0, (NCSFL_STR*) pss_memdump_set   },
    { PSS_FC_ERROR,          0, (NCSFL_STR*) pss_err_set       },
    { PSS_FC_CONF_DONE,      0, (NCSFL_STR*) pss_conf_done_set },
    { PSS_FC_BAM_REQ,        0, (NCSFL_STR*) pss_bam_req_set   },
    { PSS_FC_PLBCK_SET_COUNT, 0, (NCSFL_STR*) pss_plbck_set_count_set},
    { PSS_FC_TBL_BIND,       0, (NCSFL_STR*) pss_tbl_bind_set  },
    { PSS_FC_TBL_UNBIND,     0, (NCSFL_STR*) pss_tbl_unbind_set},
    { PSS_FC_WBREQ_I,        0, (NCSFL_STR*) pss_wbreq_1_set   },
    { PSS_FC_WBREQ_II,       0, (NCSFL_STR*) pss_wbreq_2_set   },
    { PSS_FC_OAA_ACK_EVT,    0, (NCSFL_STR*) pss_oaa_ack_evt_set},
    { 0,0,0 }
};

NCSFL_ASCII_SPEC pss_ascii_spec = 
  {
    NCS_SERVICE_ID_PSS,              /* PSS subsystem   */
    PSSV_LOG_VERSION,                /* PSS log version ID */
    "PSS",                           /* Subsystem name  */
    (NCSFL_SET*)  pss_str_set,       /* PSS const strings referenced by index */
    (NCSFL_FMAT*) pss_fmat_set,      /* PSS string format info */
    0,                               /* Place holder for str_set count */
    0                                /* Place holder for fmat_set count */
  };

/*****************************************************************************

  pss_reg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/
uns32 pss_reg_strings()
{
    NCS_DTSV_REG_CANNED_STR arg;

    memset(&arg, 0, sizeof(NCS_DTSV_REG_CANNED_STR)); 

    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
    arg.info.reg_ascii_spec.spec = &pss_ascii_spec;
    if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
        return NCSCC_RC_FAILURE;
    
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  pss_dereg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/
uns32 pss_dereg_strings()
{
    NCS_DTSV_REG_CANNED_STR arg;

    memset(&arg, 0, sizeof(NCS_DTSV_REG_CANNED_STR)); 
    
    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
    arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_PSS;
    arg.info.dereg_ascii_spec.version = PSSV_LOG_VERSION;

    if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
        return NCSCC_RC_FAILURE;
    
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pss_log_str_lib_req
 *
 * Description   : Library entry point for pss log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which SBOM gives.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pssv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = pss_reg_strings();
         break;

      case NCS_LIB_REQ_DESTROY:
         res = pss_dereg_strings();
         break;

      default:
         break;
   }
   return (res);
}

