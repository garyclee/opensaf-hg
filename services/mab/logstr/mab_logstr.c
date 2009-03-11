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

  This module contains the logging/tracing functions.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:

  log_init_ncsft..........General function to print log init message
  log_ncsft...............NCSFTlayer logging facility

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/
#if (MAB_LOG == 1)

#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "mab_log_const.h"
#include "dts_papi.h"

/* to make the compiler happy */
static uns32 mab_reg_strings(void); 
static uns32 mab_dereg_strings(void); 

/******************************************************************************
 Logging stuff for Headlines 
 ******************************************************************************/

const NCSFL_STR mab_hdln_set[] = 
  {
    { MAB_HDLN_MAC_CREATE_SUCCESS,      "MAA created"                                      },
    { MAB_HDLN_MAC_VALID_MIB_REQ_RCVD,  "MAA received a valid MIB REQ"                     },
    { MAB_HDLN_MAC_INVALID_MIB_REQ_RCVD,"MAA received an invalid MIB REQ"                  },
    { MAB_HDLN_MAC_FRWD_MIB_REQ_TO_MAS, "MAA forwarded MIB REQ to MAS"                     },
    { MAB_HDLN_MAC_NO_MAS_FRWD,         "MAA failed to forward MIB REQ (No MAS)"           },
    { MAB_HDLN_MAC_PUSH_BTSE_SUCCESS,   "MAA pushed BACKTO SE onto the MIB REQ"            },
    { MAB_HDLN_MAC_PUSH_BTSE_FAILED,    "MAA failed to push BACKTO SE onto the MIB REQ"    },
    { MAB_HDLN_MAC_NULL_MIB_RSP_RCVD,   "MAA rcvd NULL MIB RSP"                            },
    { MAB_HDLN_MAC_INVALID_MIB_RSP_RCVD,"MAA received an invalid MIB RSP"                  },
    { MAB_HDLN_MAC_USR_RSP_FNC_RET,     "MAA User rsp function returned in MAA"            }, 
    { MAB_HDLN_MAC_PLAYBACK_IN_PROGRESS,"MAA PSR playback session is in progress"          },
    { MAB_HDLN_MAC_RCVD_INVALID_OP,     "MAA received an invalid message on its mailbox"   },
    { MAB_HDLN_MAC_INVALID_KEYS_RSP_FUNC,  "MAA received a MIB_ARG with invalid keys/invalid rsp func"}, 
    { MAB_HDLN_MAC_PUSH_ORIG_FAILED,    "MAA failed to push th MIB_ARG req Originator address"},
    { MAB_HDLN_MAC_POP_ORIG_FAILED,     "MAA pop to get the MIB ARG originator failed"}, 
    { MAB_HDLN_MAC_POP_NOT_MIB_ORIG,    "MAA Stack is not holding the MIB ARG originator address"},
    { MAB_HDLN_MAC_MIBARG_FREE_FAILED, "MAA Failed to free the MIB_ARG"},
    { MAB_HDLN_MAC_INSTANTIATE_SUCCESS, "MAA Instantiate succecss  for the environment"},
    { MAB_HDLN_MAC_UNINSTANTIATE_SUCCESS, "MAA Uninstantiate success for the environment"},
    { MAB_HDLN_MAC_ENV_INST_NOT_FOUND, "MAA mac_inst_list_release() node not found for the environment"},
    { MAB_HDLN_MAC_ENV_INST_ADD_SUCCESS, "MAA mac_inst_list_add() success for the environment"},
    { MAB_HDLN_MAC_ENV_INST_REL_SUCCESS, "MAA mac_inst_list_release() success for the environment"},
    { MAB_HDLN_MAC_LOG_MDS_ENC_FAILURE, "MAA MDS Encoded MSG Failure"},
    { MAB_HDLN_MAC_LOG_MDS_DEC_FAILURE, "MAA MDS Decoded MSG Failure"},

    { MAB_HDLN_MAS_CREATE_SUCCESS,      "MAS created"                                      },
    { MAB_HDLN_MAS_DESTROY_SUCCESS,      "MAS Destroyed"                                      },
    { MAB_HDLN_MAS_FIND_OAC_FAILED,     "MAS could not find an OAA for this MIB REQ"       },
    { MAB_HDLN_MAS_FIND_OAC_SUCCESS,    "MAS found an OAA for this MIB REQ"                },
    { MAB_HDLN_MAS_SNT_NO_INST_RSP,     "MAS sent NO SUCH INSTANCE rsp to MAA"             },
    { MAB_HDLN_MAS_PUSH_FISE_SUCCESS,   "MAS pushed FILTER ID SE onto the MIB REQ"         },
    { MAB_HDLN_MAS_PUSH_FISE_FAILED,    "MAS failed to push FILTER ID SE onto the MIB REQ" },
    { MAB_HDLN_MAS_PUSH_BTSE_SUCCESS,   "MAS pushed BACKTO SE onto the MIB REQ"            }, 
    { MAB_HDLN_MAS_PUSH_BTSE_FAILED,    "MAS failed to push BACKTO SE onto the MIB REQ"    },
    { MAB_HDLN_MAS_FRWD_MIB_REQ_TO_OAC, "MAS forwarded MIB REQ to OAA"                     },
    { MAB_HDLN_MAS_NO_OAC_FRWD,         "MAS failed to forward MIB REQ to OAA"             },
    { MAB_HDLN_MAS_POP_FISE_SUCCESS,    "MAS popped FILTER ID SE from the MIB RSP"         },
    { MAB_HDLN_MAS_POP_BTSE_SUCCESS,    "MAS popped BACKTO SE from the MIB RSP"            },
    { MAB_HDLN_MAS_POP_SE_FAILED,       "MAS failed to pop a SE from the MIB RSP"          },
    { MAB_HDLN_MAS_POP_XSE,             "MAS popped unexpected SE from the MIB RSP"        },
    { MAB_HDLN_MAS_RCV_NI_MIB_RSP,      "MAS rcvd NO INSTANCE MIB RSP"                     },
    { MAB_HDLN_MAS_FRWD_NI_RSP_TO_MAC,  "MAS relayed NO INSTANCE MIB RSP to MAA"           },
    { MAB_HDLN_MAS_RCV_OK_MIB_RSP,      "MAS rcvd SUCCESS MIB RSP"                         },
    { MAB_HDLN_MAS_FRWD_OK_RSP_TO_MAC,  "MAS relayed SUCCESS MIB RSP to MAA"               },
    { MAB_HDLN_MAS_NO_OK_FRWD_TO_MAC,   "MAS failed to relay SUCCESS MIB RSP to MAA"       },
    { MAB_HDLN_MAS_RCV_X_MIB_RSP,       "MAS rcvd unexpected MIB RSP type"                 },
    { MAB_HDLN_MAS_RCV_XSTAT_MIB_RSP,   "MAS rcvd MIB RSP with unexpected status"          },
    { MAB_HDLN_MAS_RCV_X_FLTR_REG_REQ,  "MAS rcvd unexpected MAB FLTR register request"    },
    { MAB_NEW_TBL_ROW_REC_SUCCESS,      "MAS created a new table row record entry"         },
    { MAB_NEW_MAS_FLTR_FAILED,          "MAS failed to create a new MAS FLTR"              },
    { MAB_NEW_MAS_FLTR_SUCCESS,         "MAS created a new MAS FLTR"                       },
    { MAB_HDLN_MAS_OAA_DOWN,            "MAS This particular OAA is down"                  },
    { MAB_HDLN_MAS_RCVD_INVALID_OP,     "MAS received an invalid message on its mailbox"   },
    { MAB_HDLN_MAS_AMF_INITIALIZE_SUCCESS, "MAS AMF Componentization is done"},
    { MAB_HDLN_MAS_AMF_HLTH_DOING_AWESOME, "MAS AMF Health status - doing fine "},
    { MAB_HDLN_MAS_AMF_CSI_DETAILS,      "MAS CSI details received from AMF"},
    { MAB_HDLN_MAS_AMF_CSI_DESC_DETAILS, "MAS CSI Descriptor(Env) details received from AMF"},  
    { MAB_HDLN_MAS_AMF_CSI_STATE_CHG_FAILED, "MAS AMF driven state change failed"},
    { MAB_HDLN_MAS_AMF_CSI_STATE_CHG_SUCCESS,"MAS AMF driven state change Success"},
    { MAB_HDLN_MAS_FLTR_INVALID_RANGES,   "MAS received a range filter with invalid ranges"},
    { MAB_HDLN_MAS_FLTR_SA_NO_OAA,        "MAS sameas destination table did not have filters to match this index"},
    { MAB_HDLN_MAS_IDX_RECEVD,            "MAS received the following index in the MIB_ARG"},
    { MAB_HDLN_MAS_AMF_HLTH_DOING_AWFUL, "MAS AMF Health is not doing good"}, 
    { MAB_HDLN_MAS_DEF_ENV_INIT_SUCCESS, "MAS Default Env Init Success for (Env-id, HAPS state)"}, 
    { MAB_HDLN_MAS_CSI_REMOVE_SUCCESS, "MAS CSI Remove for the Env-id is success"}, 
    { MAB_HDLN_MAS_AMF_CSI_REMOVE_ALL_SUCCESS, "MAS CSI Remove all is success"}, 
    { MAB_HDLN_MAS_CSI_ADD_ONE_SUCCESS, "MAS CSI ADD success for the Env-Id"}, 
    { MAB_HDLN_MAS_AMF_CSI_ATTRIB_DECODE_SUCCESS, "MAS mas_amf_csi_attrib_decode() SUCCESS : env-id:"}, 
    { MAB_HDLN_MAS_FRWD_MOVEROW_MIB_REQ_TO_OAC, "MAS Successfully forwaded the MOVEROW request to OAC (for table-id)"}, 
    { MAB_HDLN_MAS_MOVEROW_POP_XSE, "MAS mas_info_response(), unexpected pop for MOVEROW response"}, 
    { MAB_HDLN_MAS_MOVEROW_RSP_FAILED, "MAS mas_info_response(), MOVEROW failed (mib_rsp->rsp.i_status)"}, 
    { MAB_HDLN_MAS_MOVEROW_RSP_SUCCESS, "MAS mas_info_response(), MOVEROW Success"}, 
    { MAB_HDLN_MAS_POP_FISE_FAILED, "MAS mas_info_response(), Could not get the Filter-Id from the NCSMIB_ARG stack"}, 
    { MAB_HDLN_MAS_FRWD_MIB_REQ_TO_THIS_OAC, "MAS Following is the OAA address"},
    { MAB_HDLN_MAS_FLTRS_FLUSH,              "MAS Flushing the filters of following OAA" }, 
    { MAB_HDLN_MAS_OAA_ADDR,                 "MAS OAA Address" }, 
    { MAB_HDLN_MAS_MBCSV_INTF_INIT_SUCCESS, "MAS-MBCSv Interface Initialization success (MBCSv Hdl, Sel Obj)"},
    {MAB_HDLN_MAS_MBCSV_ENC, "MAS-MBCSv MAS received Encode callback(enc_type)"}, 
    {MAB_HDLN_MAS_MBCSV_DEC, "MAS-MBCSv MAS received Decode(dec_type)"}, 
    {MAB_HDLN_MAS_COLD_SYNC_DEC_SUCCESS, "MAS-MBCSv-SBY Cold Sync response(bkt-id)"}, 
    {MAB_HDLN_MAS_COLD_SYNC_ENC_COMPL, "MAS-MBCSv-ACT Completed Cold-Syncing"}, 
    {MAB_HDLN_MAS_COLD_SYNC_ENC_BKT, "MAS-MBCSv-ACT Completed cold sync update for bucket-id"}, 
    {MAB_HDLN_MAS_COLD_SYNC_DEC_COMPL, "MAS-MBCSv-SBY Completed Cold-Syncing"}, 
    {MAB_HDLN_MAS_ASYNC_COUNT_ENC, "MAS-MBCSv-ACT Async Counts getting encoded"}, 
    {MAB_HDLN_MAS_ASYNC_COUNT_DEC, "MAS-MBCSv-SBY Async Counts received from Active MAS"}, 
    {MAB_HDLN_MAS_ASYNC_COUNT_SBY, "MAS-MBCSv-SBY Current Async Counts"},
    {MAB_HDLN_SBY_OUT_OF_SYNC, "MAS-MBCSv-SBY Warm-Sync: Out of sync for bucket"}, 
    { MAB_HDLN_MAS_SBY_IN_SYNC, "MAS-MBCSv-SBY Warm-Sync: In complete SYNC with Active, ASYNC counts matched."}, 
    { MAB_HDLN_MBCSV_ERR_IND_AMF_ERR_REPORT, "MAS-MBCSv-SBY: Reported an Error report to AMF (MBCSv error code, saf_status)"},
    { MAB_HDLN_MAS_MBCSV_DATARSP_CMPLT, "MAS-MBCSv-SBY: Got data complete message, and is in SYNC with Active MAS"}, 
    { MAB_HDLN_MAS_MBCSV_ASYNC_UPDT_SUCCESS, "MAS-MBCSv-ACT: mas_red_sync(): asyn update send success (type, status)"}, 
    { MAB_HDLN_MAS_CSI_INSTALL_SUCCESS, "MAS CSI Install Success(env-id)"}, 
    { MAB_HDLN_MAS_DEF_CSI_SUCCESS, "MAS mas_amf_csi_new() State Change successful for Default PWE (asked haState)"}, 
    { MAB_HDLN_AMF_RESPONSE_POSTPONED, "MAS mas_amf_csi_new() AMF RESP for SBY is postponed because COLD SYNC not completed (invocation-id, env-id)"}, 
    { MAB_HDLN_AMF_SBY_RESPONSE_SENT, "MAS-MBCSv-SBY COLD SYNC completed, sending the pending response to AMF (invocation-id, env-id)"}, 
    { MAB_HDLN_MAS_DUP_DEF_FLTR, "MAS Duplicate Default filter registration request"},
    { MAB_HDLN_MAS_MDS_DEST, "MAS mds dest"},
    { MAB_HDLN_MAS_VERSION, "MAS version"},
    { MAB_HDLN_MAS_LOG_MDS_ENC_FAILURE, "MAS MDS Encoded MSG Failure"},
    { MAB_HDLN_MAS_LOG_MDS_DEC_FAILURE, "MAS MDS Decoded MSG Failure"},
    { MAB_HDLN_MAS_MBCSV_ENC_FAILURE, "MAS MBCSV Encode peer version not supported"},
    { MAB_HDLN_MAS_MBCSV_DEC_FAILURE, "MAS MBCSV Decode peer version not supported"},

    { MAB_HDLN_OAC_CREATE_SUCCESS,      "OAA created"                                      },
    { MAB_HDLN_OAC_TBL_EXISTS,          "OAA table already exists to register"             },
    { MAB_HDLN_OAC_NEW_TBL_SUCCESS,     "OAA created a new table record"                   },
    { MAB_HDLN_OAC_REGD_TBL,            "OAA registered a new table"                       },
    { MAB_HDLN_OAC_NO_TBL_TO_UNREG,     "OAA table does not exist to unregister"           },
    { MAB_HDLN_OAC_SCHD_UNREG_FLTR_OK,  "OAA scheduled MAB FLTR  to unregister"            },
    { MAB_HDLN_OAC_SCHD_UNREG_FLTR_NOT, "OAA failed to schedule MAB FLTR to unregister"    },
    { MAB_HDLN_OAC_DEL_TBL_REC_SUCCESS, "OAA removed this table record"                    },
    { MAB_HDLN_OAC_DEL_TBL_REC_FAILED,  "OAA failed to remove this table record"           },
    { MAB_HDLN_OAC_SCHD_DEL_TBL_REC,    "OAA schedule to delete this table record"         },
    { MAB_HDLN_OAC_UNREGD_TBL,          "OAA unregistered a table"                         },
    { MAB_HDLN_OAC_FLTR_REG_SOON_NO_TBL,"OAA can not REG MAB FLTR for a to be deleted TBL" },
    { MAB_HDLN_OAC_REG_WRONG_FLTR_TYPE, "OAA can not REG MAB FLTR(filter type mismatch)"   },
    { MAB_HDLN_OAC_NEW_FLTR_SUCCESS,    "OAA created an OAA FLTR"                          },
    { MAB_HDLN_OAC_NEW_FLTR_FAILED,     "OAA failed to create an OAA FLTR"                 },
    { MAB_HDLN_OAC_FRWD_FLTR_REG_TO_MAS,"OAA forwarded MAB FLTR REG to MAS"                },
    { MAB_HDLN_OAC_NO_FRWD_REG_TO_MAS,  "OAA failed to forward MAB FLTR REG to MAS"        },
    { MAB_HDLN_OAC_REGD_MAB_FLTR,       "OAA registered MAB FLTR"                          },
    { MAB_HDLN_OAC_TBL_DEL_SCHDD,       "OAA TBL is already scheduled to be deleted"       },
    { MAB_HDLN_OAC_NO_FLTRS_TO_UNREG,   "OAA TBL has no MAB FLTRs to unregister"           },
    { MAB_HDLN_OAC_FIND_FLTR_FAILED,    "OAA failed to find the matching MAB FLTR"         },
    { MAB_HDLN_OAC_FIND_FLTR_SUCCESS,   "OAA found the matching MAB FLTR"                  }, 
    { MAB_HDLN_OAC_FRWD_FLTR_UNREG,     "OAA forwarded MAB FLTR UNREG to MAS"              },
    { MAB_HDLN_OAC_NO_FRWD_UNREG,       "OAA failed to forward MAB FLTR UNREG to MAS"      },
    { MAB_HDLN_OAC_UNREGD_MAB_FLTR,     "OAA unregistered MAB FLTR"                        },
    { MAB_HDLN_OAC_SNT_FLTR_TO_MAS,     "OAA sent MAB FLTR to MAS"                         },
    { MAB_HDLN_OAC_SCHD_FLTR_RESEND,    "OAA schedule to resend MAB FLTR to MAS"           },
    { MAB_HDLN_OAC_POP_BTSE_SUCCESS,    "OAA popped BACKTO SE from the MIB RSP"            },
    { MAB_HDLN_OAC_POP_SE_FAILED,       "OAA failed to pop a SE from the MIB RSP"          },
    { MAB_HDLN_OAC_POP_XSE,             "OAA popped unexpected SE from the MIB RSP"        },
    { MAB_HDLN_OAC_FRWD_RSP_SUCCESS,    "OAA forwarded this MIB RSP"                       },
    { MAB_HDLN_OAC_FRWD_PSR_FAILED,     "OAA failed to forward this MIB RSP"               },
    { MAB_HDLN_OAC_MIB_REQ_FAILED,      "OAA MIB REQ failed"                               },
    { MAB_HDLN_OAC_MIB_REQ_SUCCESS,     "OAA MIB REQ returned"                             },
    { MAB_HDLN_OAC_PUSH_FLAG_FAILED,    "OAA Push Request type flag failed"                },
    { MAB_HDLN_OAC_POP_FLAG_FAILED,     "OAA Pop Request type flag failed"                 },
    { MAB_HDLN_OAC_POP_FLAG_SUCCESS,    "OAA Pop Request type flag succeeded"              },
    { MAB_HDLN_OAC_SENDTO_PSR_SUCCESS,  "OAA send to PSS SUCCESS"           },
    { MAB_HDLN_OAC_SENDTO_PSR_FAILED,  "OAA send to PSS FAIL"               },
    { MAB_HDLN_OAC_NULL_MIB_RSP_RCVD,   "OAA received NULL as the response"                },
    { MAB_HDLN_OAC_INVALID_MIB_RSP_RCVD,   "OAA received an invalid response"              },
    { MAB_HDLN_OAC_USR_RSP_FNC_RET,   "OAA called user given response callback function"   },
    { MAB_HDLN_OAC_SU_CBK_FAILED,     "OAA user given table handler failed"                },
    { MAB_HDLN_OAC_MIBARG_FREE_FAILED,  "OAA failed to free the NCSMIB_ARG"                },
    { MAB_HDLN_OAC_RCVD_INVALID_OP,     "OAA received an invalid message on its mailbox"   },
    { MAB_HDLN_OAC_INSTANTIATE_SUCCESS, "OAA Instantiate succecss  for the environment"},
    { MAB_HDLN_OAC_UNINSTANTIATE_SUCCESS, "OAA Uninstantiate success for the environment"},
    { MAB_HDLN_OAC_ENV_INST_NOT_FOUND, "OAA oac_inst_list_release() node not found for the environment"},
    { MAB_HDLN_OAC_ENV_INST_ADD_SUCCESS, "OAA oac_inst_list_add() success for the environment"},
    { MAB_HDLN_OAC_ENV_INST_REL_SUCCESS, "OAA oac_inst_list_release() success for the environment"},
    { MAB_HDLN_OAC_PCN_LIST_ADD_FAIL,   "OAA add PCN to pcn-list fail"                     },
    { MAB_HDLN_OAC_ADD_WBREQ_TO_PEND_LIST_FAIL, "OAA Add Wrmboot req to Pending List Fail" },
    { MAB_HDLN_OAC_CONVRT_WBREQ_TO_MAB_MSG_FAIL, "OAA send warmboot-req, oac_convert_input_wbreq_to_mab_request() returned FAIL" },
    { MAB_HDLN_OAC_INVLD_PCN_IN_TBL_REG, "OAA Invalid PCN in TBL_REG API"         },
    { MAB_HDLN_OAC_INVLD_PCN_IN_WARMBOOT_REQ, "OAA Invalid PCN in WARMBOOT REQ API"         },
    { MAB_HDLN_OAC_PCN_OF_WARMBOOT_REQ_NOT_IN_PCN_LIST, "OAA PCN in WARMBOOT REQ API not found in pcn_list. So, it's invalid PCN."         },
    { MAB_HDLN_OAC_WARMBOOT_REQ_SEND_SUCCESS, "OAA WARMBOOT REQ MDS send SUCCESS"              },
    { MAB_HDLN_OAC_WARMBOOT_REQ_TO_PSS_MDS_SEND_FAIL, "OAA WARMBOOT REQ MDS send FAIL"              },
    { MAB_HDLN_OAC_WARMBOOT_REQ_REJECTED_DUE_TO_INVALID_TBLS_SPCFD, "OAA WARMBOOT REQ REJECTED as all specified Tables are either Invalid or non-persistent"   },
    { MAB_HDLN_NO_CB,                   "MAB No Control Block/Rcvd CSI assignment"                         },
    { MAB_HDLN_OAC_MAS_MDS_UP,          "OAA received MAS MDS_UP event"                    },
    { MAB_HDLN_OAC_MAS_MDS_DOWN,        "OAA received MAS MDS_DOWN event"                  },
    { MAB_HDLN_OAC_PSS_MDS_UP,          "OAA received PSS MDS_UP event"                    },
    { MAB_HDLN_OAC_PSS_MDS_DOWN,        "OAA received PSS MDS_DOWN event"                  },
    { MAB_HDLN_OAC_PSS_MDS_NO_ACTIVE,   "OAA received PSS MDS_NO_ACTIVE event"             },
    { MAB_HDLN_OAC_PSS_MDS_NEW_ACTIVE, "OAA received PSS MDS_NEW_ACTIVE event"            },
    { MAB_HDLN_OAC_PSS_MDS_NEW_ACTIVE_AS_UP_EVT, "OAA received PSS MDS_NEW_ACTIVE event, which is being treated as MDS_UP" },
    { MAB_HDLN_OAA_PCN_NULL_IN_OAC_FINDADD_PCN_IN_LIST_FUNC, "OAA error - Input PCN NULL in oac_findadd_pcn_in_list() function" },
    { MAB_HDLN_OAC_INVOKING_SEND_BIND_REQ_TO_PSS, "OAA : PSS UP, so invoking oac_psr_send_bind_req() function" },
    { MAB_HDLN_OAC_INVOKING_SEND_WARMBOOT_REQ_TO_PSS, "OAA : PSS UP, so invoking send_warmboot_req code" },
    { MAB_HDLN_OAC_TERMINATING_WARMBOOT_REQ_API_PROCESSING, "OAA terminating WARMBOOT REQ API procesing" },
    { MAB_HDLN_OAC_BIND_REQ_TO_PSS_SND_SUCCESS, "OAA send BIND REQ to PSS SUCCESS" },
    { MAB_HDLN_OAC_BIND_REQ_TO_PSS_SND_FAIL, "OAA send BIND REQ to PSS FAIL" },
    { MAB_HDLN_OAC_UNBIND_REQ_TO_PSS_SND_SUCCESS, "OAA send UNBIND REQ to PSS SUCCESS" },
    { MAB_HDLN_OAC_UNBIND_REQ_TO_PSS_SND_FAIL, "OAA send UNBIND REQ to PSS FAIL" },
    { MAB_HDLN_ACK_FRM_PSS_RCVD, "OAA RCVD ACK frm PSS" },
    { MAB_HDLN_EOP_STATUS_FOR_HDL_RCVD, "OAA RCVD EOP STATUS frm PSS for wbreq-handle " },
    { MAB_HDLN_INVKNG_EOP_STATUS_CB_FOR_USR, "OAA INVOKING USR-CALLBACK FOR EOP STATUS for wbreq-handle " },
    { MAB_HDLN_INVKD_EOP_STATUS_CB_FOR_USR, "OAA INVOKED USR-CALLBACK FOR EOP STATUS for wbreq-handle " },
    { MAB_HDLN_INV_BUFZONE_EVT_TYPE, "OAA INVALID EVT TYPE ENCOUNTERED FOR BUFZONE NODE" },
    { MAB_HDLN_BUFZONE_NODE_ALLOC_FAIL, "OAA BUFZONE NODE ALLOC FAIL" },
    { MAB_HDLN_BUFZONE_SNMP_DUP_FAILED, "OAA BUFZONE NODE SNMP DUP FAIL" },
    { MAB_HDLN_BUFZONE_NODE_ADDED, "OAA BUFZONE NODE ADD SUCCESS" },
    { MAB_HDLN_BUFZONE_NODE_REMOVED, "OAA BUFZONE NODE REMOVE SUCCESS" },
    { MAB_HDLN_OAC_BUFR_ZONE_EVT_SEND_SUCCESS, "OAA BUFZONE NODE SEND SUCCESS" },
    { MAB_HDLN_OAC_BUFR_ZONE_EVT_SEND_FAIL, "OAA BUFZONE NODE SEND FAIL" },

    { MAB_HDLN_RDA_GIVEN_INIT_ROLE, "MAB RDA Given Init Role"},
    { MAB_HDLN_OAC_MAS_MDS_NO_ACTIVE, "OAA recvd MAS MDS_NO_ACTIVE event"},
    { MAB_HDLN_OAC_MAS_MDS_NEW_ACTIVE, "OAA recvd MAS MDS_NEW_ACTIVE event"},
    { MAB_HDLN_OAC_MAS_MDS_NEW_ACTIVE_AS_UP_EVT, "OAA treating MAS MDS_NEW_ACTIVE event as MDS_UP"},
    { MAB_HDLN_IGNORING_NON_PRIMARY_MDS_SVC_EVT, "OAA ignoring Non-Primary MDS SVC event"},
    { MAB_HDLN_OAC_LOG_MDS_ENC_FAILURE, "OAC MDS Encoded MSG Failure"},
    { MAB_HDLN_OAC_LOG_MDS_DEC_FAILURE, "OAC MDS Decoded MSG Failure"},

    { 0,0 }
  };

const NCSFL_STR mab_table_details_set[]=
{
    { MAB_MAS_FIND_REQ_TBL_FAILED,  "MAS could not find the requested table" },
    { MAB_MAS_FIND_REQ_TBL_SUCCESS, "MAS found the requested table"         },
    { MAB_MAS_RCV_SA_FLTR_REG_REQ, "MAS rcvd SAME_AS MAB FLTR register request"       },
    { MAB_MAS_RCV_ANY_FLTR_REG_REQ,"MAS rcvd ANY MAB FLTR register request"           },
    { MAB_MAS_RCV_RNG_FLTR_REG_REQ,"MAS rcvd RANGE MAB FLTR register request"         },
    { MAB_MAS_RCV_RNG_FLTR_MIN_IDX,"MAS RANGE MAB FLTR Min Index"         },
    { MAB_MAS_RCV_RNG_FLTR_MAX_IDX,"MAS RANGE MAB FLTR Max Index"         },
    { MAB_MAS_RCV_DEF_FLTR_REG_REQ,"MAS rcvd DEFAULT MAB FLTR register request"       },
    { MAB_MAS_FLTR_SA_NO_DST_TBL,    "MAS failed to find DST TBL for SAME_AS MAB FLTR"  },
    { MAB_MAS_FLTR_SA_EMPTY_DST_TBL, "MAS DST TBL for SAME_AS MAB FLTR is empty"        },
    { MAB_MAS_FLTR_SA_BAD_DST_TBL,   "MAS Incomatble DST TBL for SAME_AS MAB FLTR"      },
    { MAB_MAS_RMV_ROW_REC_FAILED,  "MAS failed to remove table row record"            },
    { MAB_MAS_RMV_ROW_REC_SUCCESS, "MAS removed table row record"                     },
    { MAB_OAC_FLTR_REG_NO_TBL,     "OAA table is not registered for this MAB FLTR"    },
    { MAB_OAC_RMV_ROW_REC_SUCCESS, "OAA removed table row record"                     },
    {MAB_MAS_RCV_EXACT_FLTR_REG_REQ, "MAS rcvd EXACT MAB FLTR register request"}, 
    {MAB_MAS_RCV_EXACT_FLTR_IDX, "MAS EXACT MAB FLTR exact index"}, 
    {0, 0}
}; 

const NCSFL_STR mab_fltr_details_set[]=
{
    { MAB_MAS_FLTR_FIND_FAILED,    "MAS failed to find the matching MAB FLTR"     },
    { MAB_MAS_FLTR_FIND_SUCCESS,   "MAS found the matching MAB FLTR"              }, 
    { MAB_MAS_FLTR_RMV_FAILED, "MAS failed to remove MAS FLTR"                    },
    { MAB_MAS_FLTR_RMV_SUCCESS,"MAS removed MAS FLTR"                             },
    { MAB_OAC_FLTR_RMV_SUCCESS,"OAA removed OAC_FLTR"                             },
    { MAB_OAC_FLTR_RMV_FAILED,"OAA OAC_FLTR remove failed"                             },
    {0, 0}
};

/******************************************************************************
 Logging stuff for Service Provider (MDS) 
 ******************************************************************************/

const NCSFL_STR mab_svc_prvdr_set[] = 
  {
    { MAB_SP_MAC_MDS_INSTALL_FAILED,   "MAA Failed to install MDS"             },
    { MAB_SP_MAC_MDS_INSTALL_SUCCESS,  "MAA installed into MDS"                },
    { MAB_SP_MAC_MDS_UNINSTALL_FAILED, "MAA Failed to uninstall MDS"           },
    { MAB_SP_MAC_MDS_UNINSTALL_SUCCESS,"MAA uninstalled from MDS"              },
    { MAB_SP_MAC_MDS_SUBSCR_FAILED,    "MAA failed to subscribe to MDS evts"   },
    { MAB_SP_MAC_MDS_SUBSCR_SUCCESS,   "MAA subscribed to MDS evts"            },
    { MAB_SP_MAC_MDS_SND_MSG_FAILED,   "MAA failed to send MDS msg"            },
    { MAB_SP_MAC_MDS_RCV_MSG,          "MAA recvd MDS msg"                     },
    { MAB_SP_MAC_MDS_RCV_EVT,          "MAA recvd MDS evt"                     },
    { MAB_SP_MAC_MDS_CBINFO_NULL,      "MAA MDS thread received NULL callback info"},

    { MAB_SP_MAS_MDS_INSTALL_FAILED,   "MAS Failed to install into MDS"        },
    { MAB_SP_MAS_MDS_INSTALL_SUCCESS,  "MAS installed into MDS"                },
    { MAB_SP_MAS_MDS_UNINSTALL_FAILED, "MAS Failed to uninstall from MDS"      },
    { MAB_SP_MAS_MDS_UNINSTALL_SUCCESS,"MAS uninstalled from MDS"              },
    { MAB_SP_MAS_MDS_SUBSCR_FAILED,    "MAS failed to subscribe to MDS evts"   },
    { MAB_SP_MAS_MDS_SUBSCR_SUCCESS,   "MAS subscribed to MDS evts"            },
    { MAB_SP_MAS_MDS_SND_MSG_SUCCESS,  "MAS sent MDS msg successfully"         },
    { MAB_SP_MAS_MDS_SND_MSG_FAILED,   "MAS failed to send MDS msg"            },
    { MAB_SP_MAS_MDS_RCV_MSG,          "MAS recvd MDS msg"                     },
    { MAB_SP_MAS_MDS_RCV_EVT,          "MAS recvd MDS evt"                     },
    { MAB_SP_MAS_MDS_CBINFO_NULL,      "MAS MDS thread received NULL callback info"},
    { MAB_SP_OAC_MDS_INSTALL_FAILED,   "OAA Failed to install into MDS"        },
    { MAB_SP_OAC_MDS_INSTALL_SUCCESS,  "OAA installed into MDS"                },
    { MAB_SP_OAC_MDS_UNINSTALL_FAILED, "OAA Failed to uninstall from MDS"      },
    { MAB_SP_OAC_MDS_UNINSTALL_SUCCESS,"OAA uninstalled from MDS"              },
    { MAB_SP_OAC_MDS_SUBSCR_FAILED,    "OAA failed to subscribe to MDS evts"   },
    { MAB_SP_OAC_MDS_SUBSCR_SUCCESS,   "OAA subscribed to MDS evts"            },
    { MAB_SP_OAC_MDS_SND_MSG_SUCCESS,  "OAA sent MDS msg successfully"         },
    { MAB_SP_OAC_MDS_SND_MSG_FAILED,   "OAA failed to send MDS msg"            },
    { MAB_SP_OAC_MDS_RCV_MSG,          "OAA recvd MDS msg"                     },
    { MAB_SP_OAC_MDS_RCV_EVT,          "OAA recvd MDS evt"                     },
    { MAB_SP_OAC_MDS_CBINFO_NULL,      "OAA MDS thread received NULL callback info"},

    { 0,0 }
  };


/******************************************************************************
 Logging stuff for Locks 
 ******************************************************************************/

const NCSFL_STR mab_lock_set[] = 
  {
    { MAB_LK_MAC_LOCKED,     "MAA locked"         },
    { MAB_LK_MAC_UNLOCKED,   "MAA unlocked"       },
    { MAB_LK_MAC_CREATED,    "MAA lock created"   },
    { MAB_LK_MAC_DELETED,    "MAA lock destroyed" },
    { MAB_LK_MAS_LOCKED,     "MAS locked"         },
    { MAB_LK_MAS_UNLOCKED,   "MAS unlocked"       },
    { MAB_LK_MAS_CREATED,    "MAS lock created"   },
    { MAB_LK_MAS_DELETED,    "MAS lock destroyed" },
    { MAB_LK_OAC_LOCKED,     "OAA locked"         },
    { MAB_LK_OAC_UNLOCKED,   "OAA unlocked"       },
    { MAB_LK_OAC_CREATED,    "OAA lock created"   },
    { MAB_LK_OAC_DELETED,    "OAA lock destroyed" },
    { 0,0 }
  };


/******************************************************************************
 Logging stuff for Mem Fail 
 ******************************************************************************/

const NCSFL_STR mab_memfail_set[] = 
  {
    { MAB_MF_MABMSG_CREATE, "Failed to create MAB MSG"},

    { MAB_MF_MAC_INST_ALLOC_FAILED, "MAA malloc failed for MAC_INST* (MAA control Block)"},
    
    { MAB_MF_MAS_FLTR_CREATE_FAILED, "MAS malloc failed for NCSMAB_FLTR"},
    { MAB_MF_MAS_FLTR_MIN_IDX_CREATE_FAILED, "MAS malloc failed for MIN IDX for a range filter"}, 
    { MAB_MF_MAS_FLTR_MAX_IDX_CREATE_FAILED, "MAS malloc failed for MAX IDX for a range filter"}, 
    { MAB_MF_MAS_CSI_NODE_ALLOC_FAILED, "MAS malloc failed for MAS_CSI_NODE"}, 
    { MAB_MF_MAS_TBL_ALLOC_FAILED, "MAS malloc failed for MAS_TBL"}, 
    { MAB_MF_MAS_ROW_REC_ALLOC_FAILED, "MAS malloc failed for MAS_ROW_REC"},
    { MAB_MF_MAS_MAB_FLTR_ANCHOR_NODE_ALLOC_FAILED, "MAS malloc failed for MAB_FLTR_ANCHOR_NODE"},
    { MAB_MF_MAS_WARM_SYNC_CNTXT_ALLOC_FAILED, "MAS malloc failed for MAS_WARM_SYNC_CNTXT"},
    { MAB_MF_MAS_WARM_SYNC_BKT_LIST_ALLOC_FAILED, "MAS malloc failed for bkt list (uns8*)"},

    { MAB_MF_OAC_FLTR_CREATE_FAILED, "OAA malloc failed for NCSMAB_FLTR"},
    { MAB_MF_OAC_FLTR_MIN_IDX_CREATE_FAILED, "OAA malloc failed for MIN IDX for a range filter"}, 
    { MAB_MF_OAC_FLTR_MAX_IDX_CREATE_FAILED, "OAA malloc failed for MAX IDX for a range filter"}, 
    { MAB_MF_OAC_INST_ALLOC_FAILED, "OAA malloc failed for OAC_TBL* (OAA control Block)"},

    { MAB_MF_MAB_INST_NODE_ALLOC_FAILED, "MAB malloc failed for MAB_INST_NODE"}, 
    { MAB_MF_OAC_PCN_LIST_ALLOC_FAIL, "OAA alloc failed for OAC_PCN_LIST" },
    { MAB_MF_OAA_PCN_STRING_ALLOC_FAIL, "MAB alloc failed for OAA PCN string" },
    { MAB_MF_OAC_NCSOAC_PSS_TBL_LIST_ALLOC_FAIL, "OAA alloc failed for NCSOAC_PSS_TBL_LIST" },
    { MAB_MF_OAA_WBREQ_PEND_LIST_ALLOC_FAIL, "OAA alloc failed for OAA_WBREQ_PEND_LIST" },
    { MAB_MF_MAB_PSS_TBL_LIST_ALLOC_FAIL, "OAA alloc failed for MAB_PSS_TBL_LIST" },
    { MAB_MF_OAA_PSS_WARMBOOT_REQ, "OAA alloc failed for MAB_PSS_WARMBOOT_REQ" },
    { MAB_MF_OAA_BUFR_HASH_LIST, "OAA alloc failed for OAA_BUFR_HASH_LIST" },
    { MAB_MF_OAA_PSS_TBL_BIND_EVT, "OAA alloc failed for MAB_PSS_TBL_BIND_EVT" },
    { MAB_MF_OAA_WBREQ_HDL_LIST_ALLOC_FAIL, "OAA alloc failed for OAA_WBREQ_HDL_LIST" },
    { 0,0 }
  };

/******************************************************************************
 Logging stuff for API 
 ******************************************************************************/

const NCSFL_STR mab_api_set[] = 
  {
    { MAB_API_MAC_MIB_REQ,        "ncsmac_mib_request()"      },
    { MAB_API_MAC_SVC_DESTROY,    "ncsmac_lm(DESTROY)"        },
    { MAB_API_MAC_SVC_GET,        "ncsmac_lm(GET)"            },
    { MAB_API_MAC_SVC_SET,        "ncsmac_lm(SET)"            },

    { MAB_API_MAS_SVC_CREATE,     "ncsmas_lm(CREATE)"         },
    { MAB_API_MAS_SVC_DESTROY,    "ncsmas_lm(DESTROY)"        },
    { MAB_API_MAS_SVC_GET,        "ncsmas_lm(GET)"            },
    { MAB_API_MAS_SVC_SET,        "ncsmas_lm(SET)"            },

    { MAB_API_OAC_SVC_CREATE,    "ncsoac_lm(CREATE)"        },
    { MAB_API_OAC_SVC_DESTROY,    "ncsoac_lm(DESTROY)"        },
    { MAB_API_OAC_SVC_GET,        "ncsoac_lm(GET)"            },
    { MAB_API_OAC_SVC_SET,        "ncsoac_lm(SET)"            },
    { MAB_API_OAC_TBL_REG,        "ncsoac_ss(TBL_REG)"        },
    { MAB_API_OAC_TBL_UNREG,      "ncsoac_ss(TBL_UNREG)"      },
    { MAB_API_OAC_ROW_REG,        "ncsoac_ss(ROW_REG)"        },
    { MAB_API_OAC_ROW_UNREG,      "ncsoac_ss(ROW_UNREG)"      },

    { 0,0 }
  };


/******************************************************************************
 Logging stuff for API 
 ******************************************************************************/

const NCSFL_STR mab_evt_set[] = 
  {
    { MAB_EV_LM_OAC_TBL_REG_EXIST,       "OAA Can not register an OAA tbl record. An OAA tbl record already exists."},
    { MAB_EV_LM_OAC_TBL_UNREG_NONEXIST,  "OAA Can not unregister an OAA tbl record. No OAA tbl record exists."},
    { MAB_EV_LM_OAC_FLTR_REG_NO_TBL,     "OAA Can not register an OAA fltr. No OAA tbl record exists."},
    { MAB_EV_LM_OAC_FLTR_TYPE_MM,        "OAA Can not register an OAA fltr. Fltr type mismatch."},
    { MAB_EV_LM_OAC_FLTR_UNREG_NO_TBL,   "OAA Can not unregister an OAA fltr. No OAA tbl record exists."},
    { MAB_EV_LM_OAC_FLTR_UNREG_NO_FLTR,  "OAA Can not unregister an OAA fltr. No OAA fltr exists."},
    { MAB_EV_LM_OAC_TBL_BIND_TO_PSS,  "OAA bind event to PSS."},
    { MAB_EV_LM_OAC_TBL_UNBIND_TO_PSS,  "OAA unbind event to PSS."},
    { MAB_EV_LM_MAS_FTYPE_REG_UNSUPP,    "MAS Can not register a MAS fltr. Unsupported fltr type."},
    { MAB_EV_LM_MAS_FTYPE_REG_MM,        "MAS Can not register a MAS fltr. Fltr type mismatch."},
    { MAB_EV_LM_MAS_FSIZE_REG_MM,        "MAS Can not register a MAS fltr. Fltr size mismatch."},
    { MAB_EV_LM_MAS_FLTR_REG_EXIST,      "MAS Can not register a MAS fltr. Fltr already exists."},
    { MAB_EV_LM_OVRLAP_FLTR,             "MAS Can not register a MAS fltr. Overlapping fltrs."},
    { MAB_EV_LM_MAS_FLTR_REG_INV_SA,     "MAS Can not register a MAS fltr. Invalid SAME_AS fltr."},
    { MAB_EV_LM_MAS_FLTR_REG_SS_MM,      "MAS Can not register a MAS fltr. Subsystem mismatch."},
    { MAB_EV_LM_MAS_FLTR_UNREG_NO_FLTR,  "MAS Can not unregister a MAS fltr. No fltr exists."},
    { MAB_EV_LM_MAS_FLTR_UNREG_SS_MM,    "MAS Can not unregister a MAS fltr. Subsystem mismatch."},
    { MAB_EV_LM_MAS_FLTR_MRRSP_NO_FLTR,  "MAS MOVEROW RSP can not find a MAS fltr. No fltr exists."},

    { 0,0 }
  };

const NCSFL_STR mab_oaa_pcn_info_1_set[] = 
{
  {MAB_OAA_PCN_INFO_TBL_IN_WARMBOOT_REQ_NOT_PERSISTENT, "TBL is not persistent"}, 
  {MAB_OAA_PCN_INFO_WARMBOOT_REQ_TBL_NOT_FND_IN_PCN_LIST, "TBL not found in pcn_list"}, 
  {MAB_OAA_PCN_INFO_TBL_MADE_NON_PERSISTENT, "TBL is made not-persistent"}, 
  {MAB_OAA_PCN_INFO_PSS_NOT_PRESENT_DURING_TBL_REG, "PSS NOT ALIVE during TBL-REG op"}, 
  {MAB_OAA_PCN_INFO_UNBIND_REQ_TO_PSS_MDS_SND_FAIL, "UNBIND REQ MDS SEND to PSS FAIL"}, 
  {MAB_OAA_PCN_INFO_UNBIND_REQ_TO_PSS_MDS_SND_SUCCESS, "UNBIND REQ MDS SEND to PSS SUCCESS"}, 
  {MAB_OAA_PCN_INFO_TBL_FND_IN_PCN_LIST_DURING_UNBIND_OP, "TBL found in pcn_list during UNBIND op"}, 
  {MAB_OAA_PCN_INFO_TBL_NOT_FND_IN_PCN_LIST_DURING_UNBIND_OP, "TBL not found in pcn_list during UNBIND op"}, 
  { 0, 0} 
};

const NCSFL_STR mab_oaa_pcn_info_2_set[] = 
{
  {MAB_OAA_PCN_INFO_PCN_FND_IN_PCN_LIST_OAC_FINDADD_PCN_IN_LIST_FUNC, "PCN fnd in pcn_list"}, 
  {MAB_OAA_PCN_INFO_BIND_REQ_TO_PSS_MDS_SND_FAIL, "BIND REQ MDS SEND to PSS FAIL"}, 
  {MAB_OAA_PCN_INFO_BIND_REQ_TO_PSS_MDS_SND_SUCCESS, "BIND REQ MDS SEND to PSS SUCCESS"}, 
  {MAB_OAA_PCN_INFO_WARMBOOT_REQ_FOR_NON_PERSISTENT_TABLE_RCVD, "WARMBOOT REQ RCVD for Non-persistent Table"}, 
  {MAB_OAA_PCN_INFO_WARMBOOT_REQ_FOR_NON_BOUND_TABLE_RCVD, "WARMBOOT REQ RCVD for Non-bound Table"}, 
  {0, 0} 
};

const NCSFL_STR mab_oaa_warmbootreq_info_1_set[] = 
{
  {MAB_OAA_WARMREQ_INFO_I_PSS_MAS_STATUS, "PSS-MAS ALIVE STATUS"}, 
  { 0, 0} 
};

const NCSFL_STR mab_oaa_warmbootreq_info_2_set[] =
{
  {MAB_OAA_WARMREQ_INFO_II_RCVD_REQ, "RCVD REQ"}, 
  {MAB_OAA_WARMREQ_INFO_II_REQ_MDS_EVT_TO_PSS_COMPOSED, "REQ to PSS COMPOSED"}, 
  { 0, 0} 
};

const NCSFL_STR mab_oaa_warmbootreq_info_3_set[] =
{
  {MAB_OAA_WARMREQ_INFO_III_REQ_MDS_EVT_TO_PSS_COMPOSED, "REQ to PSS COMPOSED"}, 
  { 0, 0} 
};

/*****************************************************************************
 Logging Errors                              
******************************************************************************/
const NCSFL_STR mab_error_set[] = 
{
    { MAB_MAC_ERR_INVALID_MIB_REQ_OP,   "MAA received a MIB_ARG with invalid operation"}, 
    { MAB_MAC_ERR_MDS_CB_INVALID_OP,    "MAA MDS thread received an invalid operation in callback info"},
    { MAB_MAC_ERR_SPLR_REG_FAILED,    "MAA ncs_splr_api() failed for NCS_SPLR_REQ_REG operation"},
    { MAB_MAC_ERR_SPLR_DEREG_FAILED,    "MAA ncs_splr_api() failed for NCS_SPLR_REQ_DEREG operation"},
    { MAB_MAC_ERR_IPC_CREATE_FAILED,    "MAA m_NCS_IPC_CREATE() failed"},
    { MAB_MAC_ERR_IPC_ATTACH_FAILED,    "MAA m_NCS_IPC_ATTACH() failed"},
    { MAB_MAC_ERR_TASK_CREATE_FAILED,    "MAA m_NCS_TASK_CREATE() failed"},
    { MAB_MAC_ERR_TASK_START_FAILED,    "MAA IPC m_NCS_TASK_START() failed"},
    { MAB_MAC_ERR_CREATE_FAILED,       "MAA ncsmac_lm() for NCSMAC_LM_OP_CREATE failed for NCSMAC_LM_OP_CREATE operation"}, 
    { MAB_MAC_ERR_SPIR_LOOKUP_CREATE_FAILED,"MAA ncs_spir_api() for NCSMAC_LM_OP_CREATE failed for NCS_SPIR_REQ_LOOKUP_CREATE_INST_INFO operation"}, 
    { MAB_MAC_ERR_DESTROY_FAILED,"MAA ncsmac_lm() failed for Env-Id"}, 
    { MAB_MAC_ERR_SPIR_RMV_INST_FAILED,"MAA ncs_spir_api() for failed for NCS_SPIR_REQ_RMV_INST operation"}, 
    { MAB_MAC_ERR_INST_LIST_ADD_FAILED,"MAA mac_inst_list_add() failed(error, env-id)"}, 
    { MAB_MAC_ERR_NO_LM_CALLBACK, "MAA mac_svc_create() failed since there is no LM callback given"},
    { MAB_MAC_ERR_CB_ALLOC_FAILED_ENVID, "MAA mac_svc_create() malloc for CB failed for Env-Id"},
    { MAB_MAC_ERR_PWE_QUERY_FAILED, "MAA mac_svc_create() MDS_QUERY_PWE operation failed for Env-Id"},
    { MAB_MAC_ERR_CREATED_ON_NON_ADEST, "MAA mac_svc_create() MAA did not creat on an ADEST for Env-Id"},
    { MAB_MAC_ERR_MDS_INSTALL_FAILED, "MAA mac_svc_create() MDS_INSTALL failed for Env-Id"},
    { MAB_MAC_ERR_MDS_SUBSCRB_FAILED, "MAA mac_svc_create() MDS_SUBSCRIBE failed for Env-Id"},
    { MAB_MAC_ERR_MDS_UNINSTALL_FAILED, "MAA mac_svc_destroy() MDS_UNINSTALL failed for Env-Id"},
    { MAB_MAC_ERR_CB_NULL, "MAA mac_svc_destroy() get the CB as null from m_MAC_VALIDATE_HDL macro"},
    { MAB_MAC_ERR_SPIR_REL_INST_FAILED, "MAA mac_svc_destroy() NCS_SPIR_REQ_REL_INST failed for Env-Id"},
    { MAB_MAC_ERR_MDS_TO_MBX_SND_FAILED, "MAA failed to send the MAB_MSG from MDS thread to its mailbox(status, MSG type)"},/* 21 */

    { MAB_MAS_ERR_IPC_CREATE_FAILED,    "MAS Mailbox Createtion failed"}, 
    { MAB_MAS_ERR_IPC_ATTACH_FAILED,    "MAS Mailbox Attach failed"}, 
    { MAB_MAS_ERR_SIG_INSTALL_FAILED,   "MAS Signal Installation for AMF componentization failed"}, 
    { MAB_MAS_ERR_TASK_CREATE_FAILED,   "MAS Task Creation failed"}, 
    { MAB_MAS_ERR_TASK_START_FAILED,    "MAS Task Start failed"}, 
    { MAB_MAS_ERR_VDEST_CREATE_FAILED,  "MAS VDEST creation failed"}, 
    { MAB_MAS_ERR_LM_CREATE_FAILED,     "MAS LM Create failed"}, 
    { MAB_MAS_ERR_LM_DESTROY_FAILED,    "MAS LM Destroy Failed"}, 
    { MAB_MAS_ERR_DEF_ENV_INIT_FAILED,  "MAS Default Environment Intialization failed for Env-Id and Initial HAPS state respectively "}, 
    { MAB_MAS_ERR_MDS_PWE_CREATE_FAILED, "MAS MDS PWE Create failed"}, 
    { MAB_MAS_ERR_VDEST_ROLE_CHANGE_FAILED, "MAS VDEST role change failed"},
    { MAB_MAS_ERR_MDS_PWE_INSTALL_FAILED, "MAS MDS PWE Install failed"}, 
    { MAB_MAS_ERR_MDS_SUBSCRIBE_FAILED, "MAS MDS Subscribe failed"},
    { MAB_MAS_ERR_MDS_CB_INVALID_OP,   "MAS MDS thread received an invalid operation in callback info"},
    { MAB_MAS_ERR_FLTR_SA_INVALID_TBL,   "MAS received a sameas filter of a table which is not registered"},
    { MAB_MAS_ERR_SNT_NO_TBL_RSP_FAILED,"MAS NO SUCH TABLE rsp to MAA failed"                },
    { MAB_MAS_ERR_SNT_NO_TBL_RSP,      "MAS sent NO SUCH TABLE rsp to MAA"                },
    { MAB_MAS_ERR_DEF_FLTR_NOT_REGISTERED, "MAS moverow operation is requested for a table with no staging area/no default filter registered"}, 
    { MAB_MAS_ERR_MAC_SND_FAILED,       "MAS MIB_ARG send to MAA failed"}, 
    { MAB_MAS_ERR_OAA_SND_FAILED,       "MAS MIB_ARG send to OAA failed"}, 
    { MAB_MAS_ERR_NO_NI_FRWD_TO_MAC,   "MAS failed to relay NO INSTANCE MIB RSP to MAA"   }, 
    { MAB_MAS_ERR_AMF_ENVID_ILLEGAL,        "MAS Invalid Environment ID received from AMF"    }, 
    { MAB_MAS_ERR_AMF_DISPATCH_FAILED, "MAS AMF Dispatch failed"}, 
    { MAB_MAS_ERR_DO_EVTS_FAILED,      "MAS mas_do_evts() failed"},
    { MAB_MAS_ERR_AMF_ATTRIBS_INIT_FAILED, "MAS ncs_app_amf_attribs_init() failed"},
    { MAB_MAS_ERR_AMF_INITIALIZE_FAILED, "MAS ncs_app_amf_initialize() failed"}, 
    { MAB_MAS_ERR_MDS_PWE_QUERY_FAILED, "MAS MDS_QUERY_PWE operation failed"}, 
    { MAB_MAS_ERR_MDS_VDEST_ROLE_CHG_FAILED, "MAS NCSVDA_VDEST_CHG_ROLE operation on VDEST failed"}, 
    { MAB_MAS_ERR_MDS_UNINSTALL_FAILED, "MAS MDS_UNINSTALL operation failed"}, 
    { MAB_MAS_ERR_MDS_PWE_DESTROY_FAILED, "MAS NCSVDA_PWE_DESTROY operation failed"}, 
    { MAB_MAS_ERR_MASTBL_DESTROY_FAILED, "MAS mas_amf_csi_mas_tbl_destroy() failed"}, 
    { MAB_MAS_ERR_CSI_DELINK_FAILED, "MAS mas_amf_csi_list_delink() failed"}, 
    { MAB_MAS_ERR_AMF_CSI_REMOVE_ALL_FAILED, "MAS mas_amf_csi_remove_all() failed"}, 
    { MAB_MAS_ERR_VDEST_DESTROY_FAILED, "MAS MDS VDEST Destroy failed"}, 
    { MAB_MAS_ERR_APP_AMF_FINALIZE_FAILED, "MAS ncs_app_amf_finalize() failed"}, 
    { MAB_MAS_ERR_CSI_ADD_ONE_FAILED, "MAS CSI ADD ONE Failed with error, for env-id"}, 
    { MAB_MAS_ERR_AMF_CSI_FLAGS_ILLEGAL, "MAS  mas_amf_csi_set() received illegal CSI Descriptor flags"}, 
    { MAB_MAS_ERR_AMF_CSI_ATT_COUNT_INVD, "MAS mas_amf_csi_attrib_decode(): Invalid number of CSI attributes received "},
    {MAB_MAS_ERR_AMF_CSI_ATTRIBS_NULL, "MAS mas_amf_csi_attrib_decode(): received NULL attributes"}, 
    {MAB_MAS_ERR_AMF_CSI_ATTS_NAME_OR_ATTRIB_VAL_NULL, "MAS mas_amf_csi_attrib_decode(): Either CSI attribute name or value is NULL"}, 
    { MAB_MAS_ERR_AMF_CSI_ATTRIBS_NAME_INVALID, "MAS mas_amf_csi_attrib_decode(): Invalid attribute name received"},
    { MAB_MAS_ERR_AMF_CSI_SSCANF_FAILED, "MAS mas_amf_csi_attrib_decode(): sscanf() failed"}, 
    { MAB_MAS_ERR_AMF_CSI_DECODED_ENV_ID_OUTOF_REACH, "MAS mas_amf_csi_attrib_decode() Decoded Env is out of reach"}, 
    { MAB_MAS_ERR_AMF_FINALIZE_FAILED, "MAS saAmfFinalize() failed."}, 
    { MAB_MAS_ERR_MOVEROW_MAC_SND_FAILED, "MAS mas_forward_msg() failed to send MOVEROW request to MAC(for tbl-id)"}, 
    { MAB_MAS_ERR_MOVEROW_OAC_SND_FAILED, "MAS mas_forward_msg() failed to send MOVEROW request to OAC(for tble-id)"}, 
    { MAB_MAS_ERR_INFORM, "MAS mas_exact_fltr_almighty_inform(): EXACT Filter processing failed (reason, data follows)"},
    { MAB_MAS_ERR_AMF_TIMER_FAILED, "MAS AMF initialisation timer failed"},
    { MAB_MAS_ERR_SIGUSR2_INSTALL_FAILED, "MAS SIGUSR2 Install failes"}, 
    { MAB_MAS_ERR_MDS_TO_MBX_SND_FAILED, "MAS failed to send the MAB_MSG from MDS thread to its mailbox (status, MSG type) "},
    { MAB_MAS_ERR_MBCSV_INIT_FAILED, "MAS-MBCSv NCS_MBCSV_OP_INITIALIZE operation failed (svc-id, error)" },
    { MAB_MAS_ERR_MBCSV_SEL_OBJ_GET_FAILED, "MAS-MBCSv NCS_MBCSV_OP_SEL_OBJ_GET operation failed (mbcsv_hdl, error)" },
    { MAB_MAS_ERR_MBCSV_DISPATCH_FAILED, "MAS-MBCSv NCS_MBCSV_OP_DISPATCH operation failed (handle, status)" },
    { MAB_MAS_ERR_MBCSV_CKPT_CLOSE_FAILED, "MAS-MBCSv NCS_MBCSV_OP_CLOSE operation failed (env-id,status)"}, 
    { MAB_MAS_ERR_MBCSV_CKPT_OPEN_FAILED, "MAS-MBCSv NCS_MBCSV_OP_OPEN operation failed (env-id,status)"}, 
    { MAB_MAS_ERR_MBCSV_CKPT_CHG_ROLE_FAILED, "MAS-MBCSv NCS_MBCSV_OP_CHG_ROLE operation faile (haState, status)"},
    { MAB_MAS_ERR_MBCSV_ENC_UNSUP_MSG_TYPE, "MAS-MBCSv Interface received invalid message type during encode (msg_type)"}, 
    { MAB_MAS_ERR_MBCSV_DEC_UNSUP_MSG_TYPE, "MAS-MBCSv Interface received invalid message type during decode (msg_type)"}, 
    { MAB_MAS_ERR_MBCSV_CB_ERR_IND, "MAS-MBCSv Callback received an error indication (err_code, err_flag)"},
    { MAB_MAS_ERR_COLD_SYNC_DEC_BKT_NON_NULL, "MAS-MBCSv Cold_sync response decode success, but add failed because of a non-null pointer (bucket-id)"},
    { MAB_MAS_ERR_COLD_SYNC_ENC_BKT_FAILED, "MAS-MBCSv-ACT Cold Sync encode failed for bucket-id"}, 
    { MAB_MAS_ERR_COLD_SYNC_DEC_BKT_FAILED, "MAS-MBCSv-SBY Cold Sync decode failed for bucket-id"}, 
    { MAB_MAS_ERR_MBCSV_OP_NOT_SUPPORTED, "MAS-MBCSV mas_mbcsv_cb() received an unsupported mbcsv operation (mbcsv op code)"},
    { MAB_MAS_ERR_AMF_SBY_TO_ACT_FAILED_COLD_SYNC, "MAS-AMF-SBY: Standby ==> Active transition failed because Cold Sync is not complete"}, 
    { MAB_MAS_ERR_AMF_SBY_TO_ACT_FAILED_WARM_SYNC, "MAS-AMF-SBY: Standby ==> Active transition failed because Warm Sync is not complete"}, 
    { MAB_MAS_ERR_MBCSV_API_SND_USR_ASYNC_FAILED, "MAS-MBCSv-ACT: mas_red_sync() failed for NCS_MBCSV_ACT_UPDATE operation (type of updt, error)"}, 
    { MAB_MAS_ERR_MBCSV_ENC_FAILED, "MAS-MBCSv: mas_red_msg_enc(), Encode of a message failed (enc->io_msg_type, rc)"},
    { MAB_MAS_ERR_MBCSV_DEC_FAILED, "MAS-MBCSv: mas_red_msg_dec(), Decode of a message failed (dec->i_msg_type, rc)"},
    { MAB_MAS_ERR_MBCSV_WARM_SYNC_BKT_CNT_ZERO, "MAS-MBCSv-ACT: mas_red_data_rsp_enc_cb(), data_response encode called for zero out of sync buckets"}, 
    { MAB_MAS_ERR_MBCSV_DATA_RSP_ENCODE_FAILED_FOR_BKT, "MAS-MBCSv-ACT: mas_red_data_rsp_enc_cb() data response encode failed for bucket (to_be_sent_bkt, inst->hash[to_be_sent_bkt])"}, 
    { MAB_MAS_ERR_MBCSV_API_DATA_REQ_FAILED, "MAS-MBCSv-SBY: mas_red_warm_sync_resp_dec() failed for NCS_MBCSV_OP_SEND_DATA_REQ op(bkt_count, status)"}, 
    { MAB_MAS_ERR_MBCSV_DATA_REQ_DEC_BKTCNT_ZERO, "MAS-MBCSv-ACT: mas_red_data_req_dec() out of sync bucket count is zero"}, 
    { MAB_MAS_ERR_PREPARE_WILL_FAILED, "MAS mas_svc_destroy(): mas_amf_prepare_will() failed (status)"}, 
    { MAB_MAS_ERR_MBCSV_FINALIZE_FAILED, "MAS-MBCSv mas_svc_destroy(): ncs_mbcsv_svc() NCS_MBCSV_OP_FINALIZE failed (status)"},
    { MAB_MAS_ERR_MBCSV_CB_NULL_ARG, "MAS-MBCSv mas_mbcsv_cb() NULL Argument received"}, 
    { MAB_MAS_ERR_MBCSV_CB_INVALID_HDL, "MAS-MBCSv mas_mbcsv_cb() received invalid MAS Control Blocl Handle"}, 
    { MAB_MAS_ERR_AMF_INVALID_ROLE, "MAS mas_amf_csi_new(): Received an Invalid initial role from AMF (haState)"}, 
    { MAB_MAS_ERR_AMF_INCONSISTENT_CSI, "MAS mas_amf_csi_new(): Already a CSI is assigned for this ENV (envid)"},  
    { MAB_MAS_ERR_AMF_STATE_CHG_FAILED, "MAS mas_amf_csi_new(): mas_amf_csi_state_change_process() failed (asked state, curr state)"}, 
    { MAB_MAS_ERR_RDA_INIT_ROLE_GET_FAILED, "MAS mas_svc_create(): app_rda_init_role_get() failed (return code)"}, 
    { MAB_MAS_ERR_GET_ROLE_FAILED, "MAS mas_get_crd_role(): ncsmds_api(): MDS_QUERY_DEST failed (MDS DEST and FILTER info follows)(error): "},

    { MAB_OAC_ERR_ENV_ID_MM,            "OAA Environment ID mismatch(CB vs stack)"         },
    { MAB_OAC_ERR_RSP_UNKNWN_SVC_ID,   "OAA Can not send the response to this SVC-ID"     },
    { MAB_OAC_ERR_REQ_UNKNWN_SVC_ID,   "OAA Can not send the request to this SVC-ID"      },
    { MAB_OAC_ERR_MDS_CB_INVALID_OP,   "OAA MDS thread received an invalid operation in callback info"},
    { MAB_OAC_ERR_SPLR_REG_FAILED,    "OAA ncs_splr_api() failed for NCS_SPLR_REQ_REG operation"},
    { MAB_OAC_ERR_SPLR_DEREG_FAILED,    "OAA ncs_splr_api() failed for NCS_SPLR_REQ_DEREG operation"},
    { MAB_OAC_ERR_IPC_CREATE_FAILED,    "OAA m_NCS_IPC_CREATE() failed"},
    { MAB_OAC_ERR_IPC_ATTACH_FAILED,    "OAA m_NCS_IPC_ATTACH() failed"},
    { MAB_OAC_ERR_TASK_CREATE_FAILED,    "OAA m_NCS_TASK_CREATE() failed"},
    { MAB_OAC_ERR_TASK_START_FAILED,    "OAA IPC m_NCS_TASK_START() failed"},
    { MAB_OAC_ERR_CREATE_FAILED,       "OAA Failed to create"},
    { MAB_OAC_ERR_DESTROY_FAILED,"OAA ncsoac_lm() destroy failed for Env-Id(status, env-id)"}, 
    { MAB_OAC_ERR_SPIR_RMV_INST_FAILED,"OAA ncs_spir_api() for failed for NCS_SPIR_REQ_RMV_INST operation"}, 
    { MAB_OAC_ERR_INST_LIST_ADD_FAILED,"OAA oac_inst_list_add() failed(error, env-id)"}, 
    { MAB_OAC_ERR_NO_LM_CALLBACK, "OAA oac_svc_create() failed since there is no LM callback given"},
    { MAB_OAC_ERR_CB_ALLOC_FAILED_ENVID, "OAA oac_svc_create() malloc for CB failed for Env-Id"},
    { MAB_OAC_ERR_SPIR_LOOKUP_CREATE_FAILED,"OAA ncs_spir_api() for NCSOAC_LM_OP_CREATE failed for NCS_SPIR_REQ_LOOKUP_CREATE_INST_INFO operation"}, 
    { MAB_OAC_ERR_PWE_QUERY_FAILED, "OAA oac_svc_create() MDS_QUERY_PWE operation failed for Env-Id"},
    { MAB_OAC_ERR_MDS_INSTALL_FAILED, "OAA oac_svc_create() MDS_INSTALL failed for Env-Id"},
    { MAB_OAC_ERR_MDS_SUBSCRB_FAILED, "OAA oac_svc_create() MDS_SUBSCRIBE failed for Env-Id"},
    { MAB_OAC_ERR_MDS_OAA_SUBSCRB_FAILED, "OAA oac_svc_create() MDS_SUBSCRIBE for OAA evts failed for Env-Id"},
    { MAB_OAC_ERR_MDS_UNINSTALL_FAILED, "OAA oac_svc_destroy() MDS_UNINSTALL failed for Env-Id"},
    { MAB_OAC_ERR_CB_NULL, "OAA oac_svc_destroy() get the CB as null from m_OAC_VALIDATE_HDL macro"},
    { MAB_OAC_ERR_SPIR_REL_INST_FAILED, "OAA ncs_spir_api() failed for NCS_SPIR_REQ_REL_INST"},
    { MAB_OAC_ERR_MDS_TO_MBX_SND_FAILED, "OAA failed to send the MAB_MSG from MDS thread to its mailbox (status, msg type)"},

    /* RDA interface related errors */
    { MAB_ERR_RDA_INVALID_OP_LOC, "MAB RDA app_rda_init_role_get(): Invalid o/p location"}, 
    { MAB_ERR_RDA_LIB_INIT_FAILED, "MAB RDA app_rda_init_role_get(): PCS_RDA_LIB_INIT operation failed (error)"},
    { MAB_ERR_RDA_API_GET_ROLE_FAILED, "MAB RDA app_rda_init_role_get(): PCS_RDA_GET_ROLE operation failed (error)"}, 
    { MAB_ERR_RDA_INVALID_ROLE,"MAB RDA app_rda_init_role_get(): PCS_RDA_GET_ROLE gave invalid state (state)"},
    { MAB_ERR_RDA_LIB_FIN_FAILED, "MAB RDA app_rda_init_role_get(): PCS_RDA_LIB_DESTROY operation failed (error)"},
    
    {0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET mab_str_set[] = 
{
    { MAB_FC_HDLN,           0, (NCSFL_STR*) mab_hdln_set      },
    { MAB_FC_SVC_PRVDR_FLEX, 0, (NCSFL_STR*) mab_svc_prvdr_set },
    { MAB_FC_LOCKS,          0, (NCSFL_STR*) mab_lock_set      },
    { MAB_FC_MEMFAIL,        0, (NCSFL_STR*) mab_memfail_set   },
    { MAB_FC_API,            0, (NCSFL_STR*) mab_api_set       },
    { MAB_FC_EVT,            0, (NCSFL_STR*) mab_evt_set       },
    { MAB_FC_TBL_DET,        0, (NCSFL_STR*) mab_table_details_set },
    { MAB_FC_FLTR_DET,      0, (NCSFL_STR*) mab_fltr_details_set },
    { MAB_FC_ERROR,         0, (NCSFL_STR*) mab_error_set },
    { MAB_FC_OAA_PCN_INFO_I, 0, (NCSFL_STR*) mab_oaa_pcn_info_1_set },
    { MAB_FC_OAA_PCN_INFO_II, 0, (NCSFL_STR*) mab_oaa_pcn_info_2_set },
    { MAB_FC_OAA_WARMBOOTREQ_INFO_I, 0, (NCSFL_STR*) mab_oaa_warmbootreq_info_1_set },
    { MAB_FC_OAA_WARMBOOTREQ_INFO_II, 0, (NCSFL_STR*) mab_oaa_warmbootreq_info_2_set },
    { MAB_FC_OAA_WARMBOOTREQ_INFO_III, 0, (NCSFL_STR*) mab_oaa_warmbootreq_info_3_set },
    { 0,0,0 }
};

NCSFL_FMAT mab_fmat_set[] = 
{
    { MAB_LID_HDLN,           NCSFL_TYPE_TI,    "MAB %s HEADLINE : %s\n"     },
    { MAB_LID_SVC_PRVDR_FLEX, NCSFL_TYPE_TILL,  "MAB %s SVC PRVDR: %s, %ld, %ld\n"     },
    { MAB_LID_LOCKS,          NCSFL_TYPE_TIL,   "MAB %s LOCK     : %s (%p)\n"},
    { MAB_LID_MEMFAIL,        "TIC",             "MAB %s MEM FAIL:%s in %s\n\n"}, 
    { MAB_LID_API,            NCSFL_TYPE_TI ,   "MAB %s API      : %s\n"     },
    { MAB_LID_EVT,            NCSFL_TYPE_TI,    "MAB %s EVENT    : %s\n"     },
    { MAB_LID_TBL_DET,        NCSFL_TYPE_TILL,  "MAB %s TABLE    : %s Env-id: %ld, Tbl-id: %ld\n"     },
    { MAB_LID_FLTR_DET,       NCSFL_TYPE_TILLL, "MAB %s FLTR     : %s Tbl-id: %ld, Fltr-id: %ld, Fltr-Type: %ld\n"     },
    { MAB_LID_SVC_PRVDR_EVT,  NCSFL_TYPE_TICLLL,"MAB %s SVC PRVDR: %s\n\t\tFrom:%s, \n\t\tFrom Svc-id: %ld, NewState: %ld, Anchor:%ld\n\n"     },
    { MAB_LID_SVC_PRVDR_MSG,  NCSFL_TYPE_TICLL, "MAB %s SVC PRVDR: %s\n\t\tFrom:%s,\n\t\tFrom Svc-id: %ld, MsgType: %ld\n\n"     },
    { MAB_LID_FLTR_RANGE,     "TILLLLL",        "MAB %s FLTR     : %s\n\t\tEnv-id:%ld, Tbl-id: %ld, Fltr-id: %ld, idx-len: %ld, bgn-idx:%ld\n"},
    { MAB_LID_FLTR_RANGE_INDEX,"TID",           "MAB %s FLTR     : %s\n\t\t %s\n"},
    { MAB_LID_FLTR_ANY,       NCSFL_TYPE_TILLL, "MAB %s FLTR     : %s\n\t\tEnv-id:%ld, Tbl-id: %ld, Fltr-id: %ld\n\n"     },
    { MAB_LID_FLTR_SA,        NCSFL_TYPE_TILLLL,"MAB %s FLTR     : %s\n\t\tEnv-id:%ld, Tbl-id: %ld, Fltr-id: %ld, sameAsTbl-id:%ld\n\n"     },
    { MAB_LID_FLTR_DEF,       NCSFL_TYPE_TILLL, "MAB %s FLTR     : %s\n\t\tEnv-id:%ld, Tbl-id: %ld, Fltr-id: %ld\n\n"     },
    { MAB_LID_NO_CB,          NCSFL_TYPE_TIC,   "MAB %s HEADLINE : %s function: %s\n"     },
    { MAB_LID_ERR_II,         NCSFL_TYPE_TILL,  "MAB %s ERROR : %s   %ld    %ld\n"     },
    { MAB_LID_ERR_I,          NCSFL_TYPE_TIL,   "MAB %s ERROR : %s %ld \n"     },
    { MAB_LID_CSI,            "TILCCL",        "MAB %s CSI DATA: %s\n\t\tcsiFlags: %ld,\n\t\tCompName: %s\n\t\tcsiName:%s\n\t\tHA State: %ld\n\n"}, 
    { MAB_LID_ST_CHG,        NCSFL_TYPE_TILL,   "MAB %s STATE CHG: %s\n\t\tCurr State:%ld, New State: %ld\n\n"},
    { MAB_LID_MEM,           "TILD" ,    "MAB %s MEMORY   : %s\n idx-len: %ld,\n index: %s\n"},
    { MAB_LID_ERROR,          NCSFL_TYPE_TI,   "MAB %s ERROR : %s\n"},
    { MAB_LID_HDLN_I,         NCSFL_TYPE_TIL, "MAB %s HEADLINE : %s data1: %ld\n"},
    { MAB_LID_HDLN_II,        NCSFL_TYPE_TILL, "MAB %s HEADLINE : %s data1: %ld, data2: %ld\n"},
    { MAB_LID_OAA_BIND_EVT,   "TCL" ,    "OAA %s TBL-BIND: PCN:%s,TBL-ID:%ld\n"     },
    { MAB_LID_OAA_UNBIND_EVT, "TL" ,    "OAA %s TBL-UNBIND: TBL-ID:%ld\n"     },
    { MAB_LID_FLTR_EXACT,     "TILLLLL",        "MAB %s EXACT FLTR     : %s\n\t\tEnv-id:%ld, Tbl-id: %ld, Fltr-id: %ld, idx-len: %ld, bgn-idx:%ld\n"},
    { MAB_LID_FLTR_EXACT_INDEX,"TID",           "MAB %s EXACT FLTR     : %s\n\t\t %s\n"},
    { MAB_LID_OAA_PCN_INFO_I, "TIL",          "OAA %s INFO : %s\n\t\t PCN=<NULL>, TBL=%ld\n"},
    { MAB_LID_OAA_PCN_INFO_II, "TICL",         "OAA %s INFO : %s\n\t\t PCN=%s, TBL=%ld\n"},
    { MAB_LID_OAA_WARMBOOTREQ_INFO_I, "TILL", "OAA %s WARMBOOT REQ: %s\n\t\t PSS=%ld MAS=%ld \n"},
    { MAB_LID_OAA_WARMBOOTREQ_INFO_II, "TICLL", "OAA %s WARMBOOT REQ INFO: %s\n\t\t PCN=%s, is_system_client=%ld, TBL_ID=%ld \n"},
    { MAB_LID_OAA_WARMBOOTREQ_INFO_III, "TICLLL", "OAA %s WARMBOOT REQ INFO: %s\n\t\t PCN=%s, is_system_client=%ld, TBL_ID=%ld, wbreq_hdl=%ld\n"},
    { MAB_LID_ANC, "TF", "MAB %s ANCHOR INFO: %s\n"},
    { 0, 0, 0 }
};

NCSFL_ASCII_SPEC mab_ascii_spec = 
{
    NCS_SERVICE_ID_MAB,              /* MAB subsystem   */
    MASV_LOG_VERSION,                /* MAB log version ID */
    "MASV",                          /* Subsystem name  */
    (NCSFL_SET*)  mab_str_set,       /* MAB const strings referenced by index */
    (NCSFL_FMAT*) mab_fmat_set,      /* MAB string format info */
    0,                               /* Place holder for str_set count */
    0                                /* Place holder for fmat_set count */
};

/****************************************************************************
 * Name          : mab_log_str_lib_req
 *
 * Description   : Library entry point for mab log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
mab_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = mab_reg_strings();
         break;

      case NCS_LIB_REQ_DESTROY:
         res = mab_dereg_strings();
         break;

      default:
         break;
   }
   return (res);
}

/*****************************************************************************

  ncsdts_reg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/

uns32 mab_reg_strings()
{
    NCS_DTSV_REG_CANNED_STR arg;

    memset(&arg, 0, sizeof(NCS_DTSV_REG_CANNED_STR)); 

    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
    arg.info.reg_ascii_spec.spec = &mab_ascii_spec;
    if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
        return NCSCC_RC_FAILURE;
    
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  ncsdts_dereg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/

uns32 mab_dereg_strings()
{
    NCS_DTSV_REG_CANNED_STR arg;

    memset(&arg, 0, sizeof(NCS_DTSV_REG_CANNED_STR)); 
    
    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
    arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_MAB;
    arg.info.dereg_ascii_spec.version = MASV_LOG_VERSION;

    if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
        return NCSCC_RC_FAILURE;
    
    return NCSCC_RC_SUCCESS;
}

#endif


