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

  DESCRIPTION: This file contains SRMA logging utility routines.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "srma.h"

/******************************************************************************
                      Canned string for Memory
******************************************************************************/
const NCSFL_STR srma_mem_set[] = 
{
   {SRMA_MEM_SRMND_INFO    ,  "Mem alloc for struct: SRMA_SRMND_INFO"         },
   {SRMA_MEM_SRMA_CB       ,  "Mem alloc for struct: SRMA_CB"                 },
   {SRMA_MEM_SRMND_APPL    ,  "Mem alloc for struct: SRMA_SRMND_USR_NODE"     },
   {SRMA_MEM_CBK_INFO      ,  "Mem alloc for struct: NCS_SRMSV_RSRC_CBK_INFO" },
   {SRMA_MEM_CBK_REC       ,  "Mem alloc for struct: SRMA_PEND_CBK_REC"       },
   {SRMA_MEM_APPL_INFO     ,  "Mem alloc for struct: SRMA_USR_APPL_NODE"      },
   {SRMA_MEM_RSRC_INFO     ,  "Mem alloc for struct: SRMA_RSRC_MON"           },
   {SRMA_MEM_ALLOC_SUCCESS ,  "Success"                                       },
   {SRMA_MEM_ALLOC_FAILED  ,  "Failed"                                        },
   {0, 0}
};

/******************************************************************************
                      Canned string for selection object
******************************************************************************/
const NCSFL_STR srma_sel_obj_set[] = 
{
   { SRMA_LOG_SEL_OBJ_CREATE  ,  "Sel Obj Create"   },
   { SRMA_LOG_SEL_OBJ_DESTROY ,  "Sel Obj Destroy"  },
   { SRMA_LOG_SEL_OBJ_IND_SEND,  "Sel Obj Ind Send" },
   { SRMA_LOG_SEL_OBJ_IND_REM ,  "Sel Obj Ind Rem"  },
   { SRMA_LOG_SEL_OBJ_SUCCESS ,  "Success"          },
   { SRMA_LOG_SEL_OBJ_FAILURE ,  "Failure"          },
   { 0,0 }
};

/******************************************************************************
                          Canned string for AMF APIs
******************************************************************************/
/* ensure that the string set ordering matches that of API type definition */
const NCSFL_STR srma_api_set[] = 
{
   { SRMA_LOG_API_FINALIZE              , "ncs_srmsv_finalize()"              },
   { SRMA_LOG_API_START_MON             , "ncs_srmsv_start_monitor()"         },
   { SRMA_LOG_API_STOP_MON              , "ncs_srmsv_stop_monitor()"          },
   { SRMA_LOG_API_START_RSRC_MON        , "ncs_srmsv_start_rsrc_mon()"        },
   { SRMA_LOG_API_STOP_RSRC_MON         , "ncs_srmsv_stop_rsrc_mon()"         },
   { SRMA_LOG_API_SUBSCR_RSRC_MON       , "ncs_srmsv_subscr_rsrc_mon()"       },
   { SRMA_LOG_API_UNSUBSCR_RSRC_MON     , "ncs_srmsv_unsubscr_rsrc_mon()"     },
   { SRMA_LOG_API_INITIALIZE            , "ncs_srmsv_initialize()"            },
   { SRMA_LOG_API_SEL_OBJ_GET           , "ncs_srmsv_selection_object_get()"  },
   { SRMA_LOG_API_DISPATCH              , "ncs_srmsv_dispatch()"              },
   { SRMA_LOG_API_GET_WATERMARK_VAL     , "ncs_srmsv_get_watermark_val()"     },
   { SRMA_LOG_API_FUNC_ENTRY            , "API function entry"                },
   { SRMA_LOG_API_ERR_SA_OK             , "Ok"                                },
   { SRMA_LOG_API_ERR_SA_LIBRARY        , "Lib Err"                           },
   { SRMA_LOG_API_ERR_SA_INVALID_PARAM  , "Inv Param Err"                     },
   { SRMA_LOG_API_ERR_SA_NO_MEMORY      , "No Mem Err"                        },
   { SRMA_LOG_API_ERR_SA_BAD_HANDLE     , "Bad Hdl Err"                       },
   { SRMA_LOG_API_ERR_SA_NOT_EXIST      , "SRMND does Not Exist Err"          },
   { SRMA_LOG_API_ERR_SA_BAD_OPERATION  , "Bad Oper Err"                      },
   { SRMA_LOG_API_ERR_SA_BAD_FLAGS      , "Bad Flags Err"                     },
   { 0,0 }
};

/******************************************************************************
                    Canned string for miscellaneous SRMA events
******************************************************************************/
const NCSFL_STR srma_misc_set[] = 
{
   { SRMA_LOG_MISC_SRMND_UP       ,  "SRMND Up"                },
   { SRMA_LOG_MISC_SRMND_DN       ,  "SRMND Down"              },
   { SRMA_LOG_MISC_VALIDATE_RSRC  ,  "Validate RSRC data"      },
   { SRMA_LOG_MISC_VALIDATE_MON   ,  "Validate Monitor data"   },
   { SRMA_LOG_MISC_DUP_RSRC_MON   ,  "Duplicate Rsrc-Mon data" },
   { SRMA_LOG_MISC_AGENT_CREATE   ,  "SRMSv Agent Create: "    },
   { SRMA_LOG_MISC_AGENT_DESTROY  ,  "SRMSv Agent Destroy: "   },
   { SRMA_LOG_MISC_INVALID_MSG_TYPE, "Invalid msg type"        },
   { SRMA_LOG_MISC_ALREADY_STOP_MON, "Already stopped Monitor" },
   { SRMA_LOG_MISC_ALREADY_START_MON, "Already started Monitor"},
   { SRMA_LOG_MISC_RSRC_SRMND_MISSING, "SRMND info is missing in RSRC data"},
   { SRMA_LOG_MISC_RSRC_USER_MISSING, "User info is missing in RSRC data"},
   { SRMA_LOG_MISC_DATA_INCONSISTENT, "Inconsistent in Data"   },
   { SRMA_LOG_MISC_INVALID_RSRC_TYPE, "Invalid RSRC Type"      },
   { SRMA_LOG_MISC_INVALID_PID      ,  "Invalid PID"           },
   { SRMA_LOG_MISC_INVALID_CHILD_LEVEL, "Invalid PID Child Level"},
   { SRMA_LOG_MISC_PID_NOT_EXIST    ,  "PID doesn't exists"    },
   { SRMA_LOG_MISC_PID_NOT_LOCAL    ,  "PID is not on LOCAL node"},
   { SRMA_LOG_MISC_INVALID_USER_TYPE, "Invalid User Type"      },
   { SRMA_LOG_MISC_INVALID_NODE_ID,  "Invalid Node ID"         },
   { SRMA_LOG_MISC_INVALID_VAL_TYPE, "Invalid Value Type"      },
   { SRMA_LOG_MISC_INVALID_SCALE_TYPE, "Invalid Scale Type"    },
   { SRMA_LOG_MISC_INVALID_CONDITION, "Invalid Test Condition" },
   { SRMA_LOG_MISC_INVALID_WM_TYPE,  "Invalid Watermark Type"  },
   { SRMA_LOG_MISC_INVALID_MON_TYPE, "Invalid Monitor Type"    },
   { SRMA_LOG_MISC_INVALID_LOC_TYPE, "Invalid Location Type"   },
   { SRMA_LOG_MISC_INVALID_TOLVAL_SCALE_TYPE, "Tolerable value SCALE type mismatch with Threshold value SCALE type"},
   { SRMA_LOG_MISC_INVALID_TOLVAL_VALUE_TYPE, "Tolerable value type mismatch with Threshold value type"},
   { SRMA_LOG_MISC_SUCCESS        ,  "Success"                 },
   { SRMA_LOG_MISC_FAILED         ,  "Failed"                  },
   { SRMA_LOG_MISC_NOTHING        ,  " "                       },
   { 0,0 }
};

/******************************************************************************
       Build up the canned constant strings for the ASCII SPEC
******************************************************************************/
NCSFL_SET srma_str_set[] = 
{
   { SRMA_FC_SEAPI      ,    0,  (NCSFL_STR *) srmsv_seapi_set    },
   { SRMA_FC_MDS        ,    0,  (NCSFL_STR *) srmsv_mds_set      },
   { SRMA_FC_EDU        ,    0,  (NCSFL_STR *) srmsv_edu_set      },
   { SRMA_FC_LOCK       ,    0,  (NCSFL_STR *) srmsv_lock_set     },
   { SRMA_FC_CB         ,    0,  (NCSFL_STR *) srmsv_cb_set       },
   { SRMA_FC_RSRC_MON   ,    0,  (NCSFL_STR *) srmsv_rsrc_mon_set },
   { SRMA_FC_HDL        ,    0,  (NCSFL_STR *) srmsv_hdl_set      },
   { SRMA_FC_SRMSV_MEM  ,    0,  (NCSFL_STR *) srmsv_mem_set      },
   { SRMA_FC_TMR        ,    0,  (NCSFL_STR *) srmsv_tmr_set      },
   { SRMA_FC_MEM        ,    0,  (NCSFL_STR *) srma_mem_set       },
   { SRMA_FC_SEL_OBJ    ,    0,  (NCSFL_STR *) srma_sel_obj_set   },
   { SRMA_FC_API        ,    0,  (NCSFL_STR *) srma_api_set       },  
   { SRMA_FC_MISC       ,    0,  (NCSFL_STR *) srma_misc_set      },
   { 0, 0, 0 }
};


NCSFL_FMAT srma_fmat_set[] = 
{
   /* <SE-API Create/Destroy> <Success/Failure> */
   { SRMA_LID_SEAPI  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <MDS Register/Install/...> <Success/Failure> */
   { SRMA_LID_MDS    , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <EDU Init/Finalize> <Success/Failure> */
   { SRMA_LID_EDU    , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <Lock Init/Finalize/Take/Give> <Success/Failure> */
   { SRMA_LID_LOCK   , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <CB Create/Destroy/...> <Success/Failure> */
   { SRMA_LID_CB     , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <RSRC-MON DB Create/Destroy/...> <Success/Failure>: Hdl: <hdl> */
   { SRMA_LID_RSRC_MON , NCSFL_TYPE_TII, "[%s] %s %s\n"                       },

   /* <Hdl Create/Destroy/...> <Success/Failure>: Hdl: <hdl> */
   { SRMA_LID_HDL , NCSFL_TYPE_TII, "[%s] %s %s\n"                            },

   /* <Memory Alloc> <Success/Failure> */
   { SRMA_LID_SRMSV_MEM  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                    },

   /* <Memory Alloc> <Success/Failure> */
   { SRMA_LID_TMR  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                          },

   /* <Memory Alloc> <Success/Failure> */
   { SRMA_LID_MEM  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                          },

   /* <Sel Obj Create/Destroy/...> <Success/Failure> */
   { SRMA_LID_SEL_OBJ, NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* Processed <API Name>: <Ok/...(amf-err-code)> for comp <Comp Name>  */
   { SRMA_LID_API    , NCSFL_TYPE_TII, "[%s] Processed %s: %s\n"              },

   /* <SRMND Up/Down...> */
   { SRMA_LID_MISC   , NCSFL_TYPE_TII,   "[%s] %s %s\n"                       },

   { SRMA_LID_MSG_FMT, "TCCLCL", "[%s] %s %s  Service ID %lu with  %s  %lu \n" },

   { 0, 0, 0 }
};


NCSFL_ASCII_SPEC srma_ascii_spec = 
{
   NCS_SERVICE_ID_SRMA,          /* SRMA subsystem   */
   SRMSV_LOG_VERSION,            /* SRMA log version ID */
   "SRMA",                       /* Subsystem name  */
   (NCSFL_SET*)  srma_str_set,   /* SRMA const strings referenced by index */
   (NCSFL_FMAT*) srma_fmat_set,  /* SRMA string format info */
   0,                            /* Place holder for str_set count */
   0                             /* Place holder for fmat_set count */
};


/****************************************************************************
  Name          : srma_log_str_lib_req
 
  Description   : Library entry point for SRMA log string library.
 
  Arguments     : req_info  - This is the pointer to the input information 
                              which SBOM gives.  
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 
  Notes         : None.
 *****************************************************************************/
uns32 srma_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = srma_logstr_reg();
         break;

      case NCS_LIB_REQ_DESTROY:
         res = srma_logstr_unreg();
         break;

      default:
         break;
   }
   return (res);
}


/****************************************************************************
  Name          :  srma_logstr_reg
 
  Description   :  This routine registers SRMA with flex log service.
                  
  Arguments     :  None.
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
 *****************************************************************************/
uns32 srma_logstr_reg()
{
    NCS_DTSV_REG_CANNED_STR arg;

    memset(&arg, 0, sizeof(NCS_DTSV_REG_CANNED_STR)); 

    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
    arg.info.reg_ascii_spec.spec = &srma_ascii_spec;

    if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;
    
    return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srma_logstr_unreg
 
  Description   :  This routine unregisters SRMA with flex log service.
                  
  Arguments     :  None.
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
 *****************************************************************************/
uns32 srma_logstr_unreg()
{
   NCS_DTSV_REG_CANNED_STR arg;

   memset(&arg, 0, sizeof(NCS_DTSV_REG_CANNED_STR)); 
    
   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
   arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_SRMA;
   arg.info.dereg_ascii_spec.version = SRMSV_LOG_VERSION;

   if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;
    
   return NCSCC_RC_SUCCESS;
}


