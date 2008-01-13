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

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */


#if (NCS_DTS == 1)
#include "dts.h"
#include "bam_log.h"

/******************************************************************************
 Logging stuff for Headlines 
 ******************************************************************************/
const NCSFL_STR bam_hdln_set[] = 
{
   { BAM_CREATE_HANDLE_FAILED,            "HANDLE CREATE FAILED"          },
   { BAM_TAKE_HANDLE_FAILED,              "HANDLE TAKE FAILED"            },
   { BAM_TAKE_LOCK_FAILED,                "TAKE LOCK FAILED"              }, 
   { BAM_IPC_TASK_INIT,                   "FAILURE IN TASK INITIATION"    },
   { BAM_UNKNOWN_EVT_RCVD,                "UNKNOWN EVENT RCVD"            },
   { BAM_EVT_PROC_FAILED,                 "EVENT PROCESSING FAILED"       },
   { BAM_FILE_ARG_IGNORED,                "FILE ARGUMENT IGNORED"         }, 
   { BAM_UNKNOWN_MSG,                     "UNKNOWN MSG"                   },
   { 0,0 }
};

/******************************************************************************
 Logging stuff for Mem Fail 
 ******************************************************************************/
const NCSFL_STR bam_memfail_set[] = 
{
   { BAM_CB_ALLOC_FAILED,           "CONTROL BLOCK ALLOC FAILED"},
   { BAM_MBX_ALLOC_FAILED,          "MAILBOX CREATION ALLOC FAILED"},
   { BAM_ALLOC_FAILED,              "MEMORY ALLOCATION FAILED"},
   { BAM_MESSAGE_ALLOC_FAILED,      "NODE MESSAGE ALLOC FAILED"},
   { BAM_EVT_ALLOC_FAILED,          "BAM EVENT ALLOC FAILED"},
   { 0,0 }
};

/******************************************************************************
 Logging stuff for API 
 ******************************************************************************/
const NCSFL_STR bam_api_set[] = 
{
   { BAM_SE_API_CREATE_SUCCESS,     "BAM API CREATE SUCCESS"      },
   { BAM_SE_API_CREATE_FAILED,      "BAM API CREATE FAILED"       },
   { BAM_SE_API_DESTROY_SUCCESS,    "BAM API DESTROY SUCCESS"     },
   { BAM_SE_API_DESTROY_FAILED,     "BAM API DESTROY FAILED"      },
   { BAM_SE_API_UNKNOWN,            "BAM API UNKNOWN CALL"        },
   { 0,0 }
};

/******************************************************************************
 Logging stuff for BAM INFO
 ******************************************************************************/
const NCSFL_STR bam_info_set[] = 
{
   { BAM_PARSE_SUCCESS,              "PARSE COMPLETE"             },
   { BAM_PARSE_ERROR,                "PARSE FAILED"               },
   { BAM_NO_OID_MATCH,               "No matching OID for TAG"    },
   { BAM_NO_PROTO_MATCH,             "No matching prototype"      },
   { BAM_INVALID_DOMNODE,            "Invalid DOM Node"           },
   { BAM_BOMFILE_ERROR,              "XML FILE error"             },
   { BAM_ENT_PAT_ADD_FAIL,           "PATRICIA Tree op failed"    },
   { BAM_ENT_PAT_INIT_FAIL,          "PATRICIA Tree init failed" },
   { BAM_NCS_CONFIG,                 "NCSConfig" },
   { BAM_HW_DEP_CONFIG,              "HardWare-Deployment-Config" },
   { BAM_APP_CONFIG,                 "AppConfig" },
   { BAM_VALID_CONFIG,               "ValidationConfig" },
   { BAM_PSS_NAME,                   "PSSv" },
   { BAM_START_PARSE,                "Starting Parsing Of" },
   { BAM_APP_FILE,                   "AppConfig.xml" },
   { BAM_APP_NOT_PRES,               "is not present or could not be open" },
   { 0,0 }
};

/******************************************************************************
 Logging stuff for Messages 
 ******************************************************************************/
const NCSFL_STR bam_msg_set[] = 
{
   { BAM_DEF_MSG,                        "Rcvd Default Message"                 },
   { 0,0 }
};


/******************************************************************************
 Logging stuff for Service providers (MDS, AMF)
 ******************************************************************************/
const NCSFL_STR bam_svc_prvdr_set[] = 
{
   { BAM_MDS_INSTALL_FAIL,                        "MDS Install failed"              },
   { BAM_MDS_INSTALL_SUCCESS,                     "MDS Install success"             },
   { BAM_MDS_UNINSTALL_FAIL,                      "MDS Uninstall failed"            },
   { BAM_MDS_SUBSCRIBE_FAIL,                      "MDS Subscription failed"         },
   { BAM_MDS_SUBSCRIBE_SUCCESS,                   "MDS Subscribe sucess"            },
   { BAM_MDS_SEND_ERROR,                          "MDS Send failed"                 },
   { BAM_MDS_CALL_BACK_ERROR,                     "MDS Call back error"             },
   { BAM_MDS_UNKNOWN_SVC_EVT,                     "MDS unsubscribed svc msg"        },
   { BAM_MDS_VDEST_CREATE_FAIL,                   "MDS vdest create failed"         },
   { BAM_MDS_VDEST_DESTROY_FAIL,                  "MDS vdest destroy failed"        },
   { BAM_MDS_SEND_SUCCESS,                        "MDS Send Success"                },
   { 0,0 }
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET bam_str_set[] = 
{
   { BAM_FC_HDLN,           0, (NCSFL_STR*) bam_hdln_set      },
   { BAM_FC_MEMFAIL,        0, (NCSFL_STR*) bam_memfail_set   },
   { BAM_FC_API,            0, (NCSFL_STR*) bam_api_set       },
   { BAM_FC_MSG,            0, (NCSFL_STR*) bam_msg_set       },
   { BAM_FC_SVC_PRVDR,      0, (NCSFL_STR*) bam_svc_prvdr_set },
   { BAM_FC_INFO,           0, (NCSFL_STR*) bam_info_set      },
   { 0,0,0 }
};

NCSFL_FMAT bam_fmat_set[] = 
{
   { BAM_LID_HDLN,           NCSFL_TYPE_TI,     "%s BAM HEADLINE : %s\n"     },
   { BAM_LID_MEMFAIL,        NCSFL_TYPE_TI,     "%s BAM MEM_ERR: %s\n"        },
   { BAM_LID_API,            NCSFL_TYPE_TI,     "%s BAM API: %s\n"           },
   { BAM_LID_MSG,            NCSFL_TYPE_TIL,    "%s BAM MSG: %s rcvd frm:%d\n"},
   { BAM_LID_MSG_TIL,       NCSFL_TYPE_TIL,    "%s BAM INFO: %s info %d \n"},
   { BAM_LID_MSG_TIC,       NCSFL_TYPE_TIC,    "%s BAM INFO: %s info %s \n"},
   { BAM_LID_SVC_PRVDR,      NCSFL_TYPE_TI,     "%s BAM SVC PRVDR: %s \n"},
   { BAM_LID_MDS_SND,        NCSFL_TYPE_TICL,   "%s BAM MDS: %s at %s:%d \n"},
   { BAM_LID_MSG_TII,        NCSFL_TYPE_TII,    "%s BAM: %s %s \n"},
   { 0, 0, 0 }
};

NCSFL_ASCII_SPEC bam_ascii_spec = 
  {
    NCS_SERVICE_ID_BAM,             /* BAM subsystem */
    BAM_LOG_VERSION,                /* BAM log version ID */
    "BAM",
    (NCSFL_SET*)  bam_str_set,       /* BAM const strings referenced by index */
    (NCSFL_FMAT*) bam_fmat_set,      /* BAM string format info */
    0,                              /* Place holder for str_set count */
    0                               /* Place holder for fmat_set count */
  };

/*****************************************************************************

  bam_reg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/
uns32 bam_reg_strings()
{
    NCS_DTSV_REG_CANNED_STR arg;
    
    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
    arg.info.reg_ascii_spec.spec = &bam_ascii_spec;
    if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
        return NCSCC_RC_FAILURE;
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  bam_unreg_strings

  DESCRIPTION: Function is used for unregistering the canned strings with the 
               DTS.

*****************************************************************************/
uns32 bam_unreg_strings()
{
    NCS_DTSV_REG_CANNED_STR arg;

    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
    arg.info.dereg_ascii_spec.svc_id =  NCS_SERVICE_ID_BAM;
    arg.info.dereg_ascii_spec.version = BAM_LOG_VERSION;

    if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
        return NCSCC_RC_FAILURE;
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : bam_log_str_lib_req

  Description   : Library entry point for BAM log string library.

  Arguments     : req_info  - This is the pointer to the input information

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..

  Notes         : None.
*****************************************************************************/
uns32 bam_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = bam_reg_strings();
         break;

      case NCS_LIB_REQ_DESTROY:
         res = bam_unreg_strings();
         break;

      default:
         break;
   }
   return (res);
}

#endif /* if NCS_DTS == 1*/
