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

  DESCRIPTION: This file contains SRMND logging utility routines.
..............................................................................

  FUNCTIONS INCLUDED in this module:  

******************************************************************************
*/

#include "srmnd.h"

/******************************************************************************
                      Canned string for Monitoring
******************************************************************************/
const NCSFL_STR srmnd_mon_set[] = 
{
    { SRMND_MON_RSRC_ADD   ,  "Add Rsrc for Monitoring"            },
    { SRMND_MON_RSRC_DEL   ,  "Del Rsrc from Monitoring"           },
    { SRMND_MON_RSRC_START ,  "Start Monitoring Rsrc"              },
    { SRMND_MON_RSRC_STOP  ,  "Stop Monitoring Rsrc"               },
    { SRMND_MON_APPL_START ,  "Start Monitoring Appl Rsrc's"       },
    { SRMND_MON_APPL_STOP  ,  "Stop Monitoring Appl Rsrc's"        },
    { SRMND_MON_APPL_DEL   ,  "Delete Appl Rsrc's from Monitoring" },
    { SRMND_MON_GET_WM     ,  "Get Watermark Values"               },
    { SRMND_MON_SUCCESS    ,  "Success"                            },
    { SRMND_MON_FAILED     ,  "Failed"                             },
    {0, 0}
};

/******************************************************************************
                      Canned string for Memory
******************************************************************************/
const NCSFL_STR srmnd_mem_set[] = 
{
   {SRMND_MEM_USER_NODE     ,  "Mem alloc for struct: SRMND_MON_SRMA_USR_NODE" },
   {SRMND_MEM_SRMND_CB      ,  "Mem alloc for struct: SRMND_CB"                },
   {SRMND_MEM_RSRC_TYPE     ,  "Mem alloc for struct: SRMND_RSRC_TYPE_NODE"    },
   {SRMND_MEM_SAMPLE_DATA   ,  "Mem alloc for struct: SRMND_SAMPLE_DATA"       },
   {SRMND_MEM_PID_DATA      ,  "Mem alloc for struct: SRMND_PID_DATA"          },
   {SRMND_MEM_EVENT         ,  "Mem alloc for struct: SRMND_EVT"               },
   {SRMND_MEM_RSRC_INFO     ,  "Mem alloc for struct: SRMA_RSRC_MON"           },
   {SRMND_MEM_ALLOC_SUCCESS ,  "Success"                                       },
   {SRMND_MEM_ALLOC_FAILED  ,  "Failed"                                        },
   {0, 0}
};

/******************************************************************************
                    Canned string for miscellaneous SRMND events
******************************************************************************/
const NCSFL_STR srmnd_misc_set[] = 
{
   { SRMND_LOG_MISC_SRMA_UP     ,  "SRMA Up"            },
   { SRMND_LOG_MISC_SRMA_DN     ,  "SRMA Down"          },
   { SRMND_LOG_MISC_ND_CREATE   ,  "SRMSv ND Create"    },
   { SRMND_LOG_MISC_ND_DESTROY  ,  "SRMSv ND Destroy"   },
   { SRMND_LOG_MISC_GET_APPL    ,  "Get the Appl Node"  },
   { SRMND_LOG_MISC_DATA_INCONSISTENT, "Inconsistency in Data"   },
   { SRMND_LOG_MISC_SUCCESS     ,  "Success"            },
   { SRMND_LOG_MISC_FAILED      ,  "Failed"             },
   { SRMND_LOG_MISC_NOTHING     ,  " "                  },
   { 0,0 }
};

/******************************************************************************
                    Canned string for AMF specific SRMND events
******************************************************************************/
const NCSFL_STR srmnd_amf_set[] = 
{
   { SRMND_LOG_AMF_CBK_INIT      ,  "SRMND AMF CBK init"          },
   { SRMND_LOG_AMF_INITIALIZE    ,  "SRMND AMF Initialize"        },
   { SRMND_LOG_AMF_GET_SEL_OBJ   ,  "SRMND AMF get sel obj"       },
   { SRMND_LOG_AMF_GET_COMP_NAME ,  "SRMND AMF get comp name"     },
   { SRMND_LOG_AMF_FINALIZE      ,  "SRMND AMF Finalize"          },
   { SRMND_LOG_AMF_STOP_HLTCHECK ,  "SRMND AMF stop health check" },
   { SRMND_LOG_AMF_START_HLTCHECK,  "SRMND AMF start health check"},
   { SRMND_LOG_AMF_REG_COMP      ,  "SRMND AMF reg comp"          },
   { SRMND_LOG_AMF_UNREG_COMP    ,  "SRMND AMF unreg comp"        },
   { SRMND_LOG_AMF_DISPATCH      ,  "SRMND AMF Dispatch"          },
   { SRMND_LOG_AMF_SUCCESS       ,  "Success"                     },
   { SRMND_LOG_AMF_FAILED        ,  "Failed"                      },
   { SRMND_LOG_AMF_NOTHING       ,  " "                           },
   { 0,0 }
};

/******************************************************************************
       Build up the canned constant strings for the ASCII SPEC
******************************************************************************/
NCSFL_SET srmnd_str_set[] = 
{
   { SRMND_FC_SEAPI  ,    0,  (NCSFL_STR *) srmsv_seapi_set    },
   { SRMND_FC_MDS    ,    0,  (NCSFL_STR *) srmsv_mds_set      },
   { SRMND_FC_EDU    ,    0,  (NCSFL_STR *) srmsv_edu_set      },
   { SRMND_FC_LOCK   ,    0,  (NCSFL_STR *) srmsv_lock_set     },
   { SRMND_FC_CB     ,    0,  (NCSFL_STR *) srmsv_cb_set       },
   { SRMND_FC_RSRC_MON ,  0,  (NCSFL_STR *) srmsv_rsrc_mon_set },
   { SRMND_FC_HDL    ,    0,  (NCSFL_STR *) srmsv_hdl_set      },
   { SRMND_FC_TIM    ,    0,  (NCSFL_STR *) srmsv_tim_set      },
   { SRMND_FC_TMR    ,    0,  (NCSFL_STR *) srmsv_tmr_set      },
   { SRMND_FC_SRMSV_MEM,  0,  (NCSFL_STR *) srmsv_mem_set      },
   { SRMND_FC_TREE,       0,  (NCSFL_STR *) srmsv_pat_set      },
   { SRMND_FC_MEM    ,    0,  (NCSFL_STR *) srmnd_mem_set      },
   { SRMND_FC_MON    ,    0,  (NCSFL_STR *) srmnd_mon_set      },
   { SRMND_FC_AMF    ,    0,  (NCSFL_STR *) srmnd_amf_set      },
   { SRMND_FC_MISC   ,    0,  (NCSFL_STR *) srmnd_misc_set     },
   { 0, 0, 0 }
};


NCSFL_FMAT srmnd_fmat_set[] = 
{
   /* <SE-API Create/Destroy> <Success/Failure> */
   { SRMND_LID_SEAPI  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <MDS Register/Install/...> <Success/Failure> */
   { SRMND_LID_MDS    , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <EDU Init/Finalize> <Success/Failure> */
   { SRMND_LID_EDU    , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <Lock Init/Finalize/Take/Give> <Success/Failure> */
   { SRMND_LID_LOCK   , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <CB Create/Destroy/...> <Success/Failure> */
   { SRMND_LID_CB     , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <RSRC-MON Create/Destroy/...> <Success/Failure>: Hdl: <hdl> */
   { SRMND_LID_RSRC_MON , NCSFL_TYPE_TII, "[%s] %s %s\n"                       },

   /* <Hdl DB Create/Destroy/...> <Success/Failure>: Hdl: <hdl> */
   { SRMND_LID_HDL , NCSFL_TYPE_TII, "[%s] %s %s\n"                            },

   /* <Task/Mbx create/destroy...> <Success/Failure> */
   { SRMND_LID_TIM , NCSFL_TYPE_TII, "[%s] %s %s\n"                            },

   /* <Timer Create/Destroy/...> <Success/Failure>*/
   { SRMND_LID_TMR , NCSFL_TYPE_TII, "[%s] %s %s\n"                            },

   /* <Memory Alloc> <Success/Failure> */
   { SRMND_LID_SRMSV_MEM  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                    },

   /* <PAT tree> <Success/Failure> */
   { SRMND_LID_TREE  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                         },

   /* <Memory Alloc> <Success/Failure> */
   { SRMND_LID_MEM  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                          },
   
   /* <Monitoring> <Success/Failure> */
   { SRMND_LID_MON  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                          },

   /* <AMF> <Success/Failure> */
   { SRMND_LID_AMF  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                          },
   /* <SRMND Miscelleneous...> */
   { SRMND_LID_MISC   , NCSFL_TYPE_TII,   "[%s] %s  %s\n"                           },
   { SRMND_LID_MSG_FMT, "TCCLCL", "[%s] %s %s  Service ID %lu with  %s  %lu \n" },

   { 0, 0, 0 }
};


NCSFL_ASCII_SPEC srmnd_ascii_spec = 
{
   NCS_SERVICE_ID_SRMND,          /* SRMND service id */
   SRMSV_LOG_VERSION,             /* SRMND log version id */
   "SRMND",                       /* used for generating log filename */
   (NCSFL_SET *)  srmnd_str_set,  /* SRMND const strings ref by index */
   (NCSFL_FMAT *) srmnd_fmat_set, /* SRMND string format info */
   0,                             /* placeholder for str_set cnt */
   0                              /* placeholder for fmat_set cnt */
};


/****************************************************************************
  Name          : srmnd_log_str_lib_req
 
  Description   : Library entry point for SRMND log string library.
 
  Arguments     : req_info  - This is the pointer to the input information 
                              which SBOM gives.  
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 
  Notes         : None.
 *****************************************************************************/
uns32 srmnd_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = srmnd_logstr_reg();
         break;

      case NCS_LIB_REQ_DESTROY:
         res = srmnd_logstr_unreg();
         break;

      default:
         break;
   }
   return (res);
}


/****************************************************************************
  Name          :  srmnd_logstr_reg
 
  Description   :  This routine registers SRMND with flex log service.
                  
  Arguments     :  None.
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
 *****************************************************************************/
uns32 srmnd_logstr_reg()
{
   NCS_DTSV_REG_CANNED_STR arg;

   memset(&arg, 0, sizeof(NCS_DTSV_REG_CANNED_STR)); 
   
   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
   arg.info.reg_ascii_spec.spec = &srmnd_ascii_spec;

   if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;
    
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmnd_logstr_unreg
 
  Description   :  This routine unregisters SRMND with flex log service.
                  
  Arguments     :  None.
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
 *****************************************************************************/
uns32 srmnd_logstr_unreg()
{
   NCS_DTSV_REG_CANNED_STR arg;

   memset(&arg, 0, sizeof(NCS_DTSV_REG_CANNED_STR)); 

   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
   arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_SRMND;
   arg.info.dereg_ascii_spec.version = SRMSV_LOG_VERSION;
 
   if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;
    
   return NCSCC_RC_SUCCESS;
}



