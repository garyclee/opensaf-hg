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

#include "ncsgl_defs.h"
#include "t_suite.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "avm_dblog.h"
#include "avm_logstr.h"

/******************************************************************************
  Logging stuff for Headlines
 ******************************************************************************/
const NCSFL_STR avm_hdln_set[] =
{
   { AVM_INVALID_VAL,                     "AN INVALID DATA VALUE"         },
   { AVM_UNKNOWN_MSG_RCVD,                "UNKNOWN EVENT RCVD"            },
   { AVM_MSG_PROC_FAILED,                 "EVENT PROCESSING FAILED"       },
   { AVM_ENTERED_FUNC,                    "ENTRED THE FUNCTION"           },
   { AVM_RCVD_VAL,                        "RECEIVED THE VALUE"            },
   { 0,0 }
};


/******************************************************************************
                          Canned string for SE-API
 ******************************************************************************/
const NCSFL_STR avm_seapi_set[] = 
{
   { AVM_LOG_SEAPI_CREATE , "SE-API Create"  },
   { AVM_LOG_SEAPI_DESTROY, "SE-API Destroy" },
   { AVM_LOG_SEAPI_SUCCESS, "Success"        },
   { AVM_LOG_SEAPI_FAILURE, "Failure"        },
   { 0,0 }
};

/******************************************************************************
                       Canned string for control block
 ******************************************************************************/
const NCSFL_STR avm_cb_set[] = 
{
   { AVM_LOG_CB_CREATE  ,       "CB Create"                      },
   { AVM_LOG_CB_DESTROY ,       "CB Destroy"                     },
   { AVM_LOG_CB_HDL_ASS_CREATE, "CB Hdl Mngr Association Create" },
   { AVM_LOG_CB_HDL_ASS_REMOVE, "CB Hdl Mngr Association Rem"    },
   { AVM_LOG_CB_RETRIEVE,       "CB Retrieve"                    },
   { AVM_LOG_CB_RETURN  ,       "CB Return"                      },
   { AVM_LOG_CB_SUCCESS ,       "Success"                        },
   { AVM_LOG_CB_FAILURE ,       "Failure"                        },
   { 0,0 }
};

/******************************************************************************
                          Canned string for RDE 
 ******************************************************************************/
const NCSFL_STR avm_rde_set[] = 
{
   { AVM_LOG_RDE_REG         , "RDE Register"     },
   { AVM_LOG_RDE_SUCCESS     , "Success"          },
   { AVM_LOG_RDE_FAILURE     , "Failure"          },
   { 0,0 }
};

/******************************************************************************
                          Canned string for MDS
 ******************************************************************************/
const NCSFL_STR avm_mds_set[] = 
{
   { AVM_LOG_MDS_REG         , "MDS Register"     },
   { AVM_LOG_MDS_INSTALL     , "MDS Install"      },
   { AVM_LOG_MDS_UNINSTALL   , "MDS Uninstall"    },
   { AVM_LOG_MDS_SUBSCRIBE   , "MDS Subscribe"    },
   { AVM_LOG_MDS_UNREG       , "MDS Un-Register"  },
   { AVM_LOG_MDS_ADEST_HDL   , "MDS Get ADEST hdl" },
   { AVM_LOG_MDS_ADEST_REG   , "MDS ADEST Register" },
   { AVM_LOG_MDS_VDEST_CRT   , "MDS create VDEST address" },
   { AVM_LOG_MDS_VDEST_ROL   , "MDS set VDEST role"},
   { AVM_LOG_MDS_VDEST_REG   , "MDS VDEST Register" },
   { AVM_LOG_MDS_VDEST_DESTROY, "MDS VDEST destroy"},
   { AVM_LOG_MDS_SEND        , "MDS Send"         },
   { AVM_LOG_MDS_RCV_CBK     , "MDS Rcv Cbk"      },
   { AVM_LOG_MDS_CPY_CBK     , "MDS Copy Cbk"     },
   { AVM_LOG_MDS_SVEVT_CBK   , "MDS Svc Evt Cbk"  },
   { AVM_LOG_MDS_SUCCESS     , "Success"          },
   { AVM_LOG_MDS_FAILURE     , "Failure"          },
   { 0,0 }
};

/******************************************************************************
                          Canned string for MAS
 ******************************************************************************/
const NCSFL_STR avm_mas_set[] = 
{
   { AVM_LOG_MIBLIB_REGISTER,         "MIBLIB Register"},
   { AVM_LOG_MIB_CPY,                 "MIB copy       "},
   { AVM_LOG_OAC_REGISTER,            "TBL and ROW Register with OAC"},  
   { AVM_LOG_OAC_REGISTER_TBL,        "TBL Register with OAC "   },
   { AVM_LOG_OAC_UNREGISTER_TBL,      "TBL Unregister with OAC" },
   { AVM_LOG_OAC_REGISTER_ROW,        "ROW Register along with Filter with OAC"   },
   { AVM_LOG_OAC_UNREGISTER_ROW,      "ROW Unregister with OAC" },
   { AVM_LOG_PSS_REGISTER,            "Registration with PSS  " },   
   { AVM_LOG_OAC_WARMBOOT,            " Warmboot Req -> PSS   " },
   { AVM_LOG_MAS_SKIP,                "Skip"           },
   { AVM_LOG_MAS_SUCCESS,             "Success"        },
   { AVM_LOG_MAS_FAILURE,             "Failure"        },
   { 0,0 }
};

/******************************************************************************
                          Canned string for EDA
 ******************************************************************************/
const NCSFL_STR avm_eda_set[] = 
{
   { AVM_LOG_EDA_CREATE,            "EDA Library Create"   },
   { AVM_LOG_EDA_EVT_INIT     ,     "EDA EVT Initialize"   },
   { AVM_LOG_EDA_EVT_FINALIZE ,     "EDA EVT Finalize  "   },
   { AVM_LOG_EDA_CHANNEL_OPEN ,     "EDA Channel Open"     },
   { AVM_LOG_EDA_CHANNEL_CLOSE,     "EDA Channel Close"    },
   { AVM_LOG_EDA_EVT_SUBSCRIBE,     "EDA Subscription"     },
   { AVM_LOG_EDA_EVT_UNSUBSCRIBE,   "EDA Un-Subscription"  },
   { AVM_LOG_EDA_EVT_SELOBJ,        "EDA Selection Object" }, 
   { AVM_LOG_EDA_EVT_CBK      ,     "EDA Cbk"              },
   { AVM_LOG_EDA_SUCCESS      ,     "Success"              },
   { AVM_LOG_EDA_FAILURE      ,     "Failure"              },
   { 0,0 }
};

/******************************************************************************
                          Canned string for MBCSv 
 ******************************************************************************/
const NCSFL_STR avm_mbcsv_set[] = 
{
   { AVM_MBCSV_MSG_WARM_SYNC_REQ, "Warm Sync Request"  },
   { AVM_MBCSV_MSG_WARM_SYNC_RESP, "Warm Sync Resp  "  },
   { AVM_MBCSV_MSG_DATA_RESP,     "MBCSv Data Resp "  },
   { AVM_MBCSV_MSG_DATA_RESP_COMPLETE, " MBCSv Data resp Complete" },
   { AVM_MBCSV_MSG_DATA_REQ,      "Data Req" },
   { AVM_MBCSV_MSG_ENC,           "Encode" },
   { AVM_MBCSV_MSG_DEC,           "Decode" },
   { AVM_MBCSV_MSG_COLD_ENC,      "Cold Sync Encode" },
   { AVM_MBCSV_MSG_COLD_DEC,      "Cold Sync Decode" },
   { AVM_MBCSV_MSG_ASYNC_ENC,      "Async Sync Encode" },
   { AVM_MBCSV_MSG_ASYNC_DEC,      "Async Sync Decode" },
   { AVM_MBCSV_SUCCESS,           "Success " },
   { AVM_MBCSV_FAILURE,           "Failure " },
   { AVM_MBCSV_IGNORE,            "Ignore " },
   { 0,0 }
};

/******************************************************************************
                          Canned string for HPL
 ******************************************************************************/
const NCSFL_STR avm_hpl_set[] = 
{
   { AVM_LOG_HPL_INIT,           "HPL Initialize" },
   { AVM_LOG_HPL_DESTROY,        "HPL DESTROY"    },
   { AVM_LOG_HPL_API,            "HPL API:   "    },
   { AVM_LOG_HPL_SUCCESS,        "Success"        },
   { AVM_LOG_HPL_FAILURE,        "Failure"        },
   { 0,0 }
};

/******************************************************************************
                       Canned string for Mailbox
 ******************************************************************************/
const NCSFL_STR avm_mbx_set[] = 
{
   { AVM_LOG_MBX_CREATE , "Mbx Create"  },
   { AVM_LOG_MBX_ATTACH , "Mbx Attach"  },
   { AVM_LOG_MBX_DESTROY, "Mbx Destroy" },
   { AVM_LOG_MBX_DETACH , "Mbx Detach"  },
   { AVM_LOG_MBX_SEND   , "Mbx Send"    },
   { AVM_LOG_MBX_SUCCESS, "Success"     },
   { AVM_LOG_MBX_FAILURE, "Failure"     },
   { 0,0 }
};


/******************************************************************************
                       Canned string for Task
 ******************************************************************************/
const NCSFL_STR avm_task_set[] = 
{
   { AVM_LOG_TASK_CREATE , "Task Create"  },
   { AVM_LOG_TASK_START  , "Task Start"   },
   { AVM_LOG_TASK_RELEASE, "Task Release" },
   { AVM_LOG_TASK_SUCCESS, "Success"      },
   { AVM_LOG_TASK_FAILURE, "Failure"      },
   { 0,0 }
};

/******************************************************************************
                       Canned string for Patricia Tree
 ******************************************************************************/
const NCSFL_STR avm_pat_set[] = 
{
   { AVM_LOG_PAT_INIT   , "Patricia Init" },
   { AVM_LOG_PAT_ADD    , "Patricia Add"  },
   { AVM_LOG_PAT_DEL    , "Patricia Del"  },
   { AVM_LOG_PAT_SUCCESS,  "Success"       },
   { AVM_LOG_PAT_FAILURE,  "Failure"       },
   { 0,0 }
};

/******************************************************************************
                          Canned string for TMR

 ******************************************************************************/
const NCSFL_STR avm_tmr_set[] = 
{
   { AVM_LOG_TMR_CREATED ,     "Timer Created" },
   { AVM_LOG_TMR_STARTED ,     "Timer Started" },
   { AVM_LOG_TMR_EXPIRED ,     "Timer Expired" },
   { AVM_LOG_TMR_STOPPED ,     "Timer Stopped" },
   { AVM_LOG_TMR_DESTROYED,    "Timer Destroyed"},
   { AVM_LOG_TMR_SUCCESS ,     "Success"       },
   { AVM_LOG_TMR_FAILURE ,     "Failure"       },
   { 0,0 }
};

/******************************************************************************
   Logging stuff for Mem Fail
 ******************************************************************************/
const NCSFL_STR avm_mem_set[] =
{
   { AVM_LOG_DEFAULT_ALLOC,                   "Buffer Alloc"            },
   { AVM_LOG_EVT_ALLOC,                       "Event Alloc "            },
   { AVM_LOG_ENTITY_PATH_ALLOC,               "Entity Path Alloc "      },
   { AVM_LOG_INVENTORY_DATA_REC_ALLOC,        "Inventory Data Rec Alloc"},
   { AVM_LOG_FAULT_DOMAIN_ALLOC,              "Fault Domain Alloc"      },
   { AVM_LOG_ENT_INFO_ALLOC,                  "Entity Info Alloc"       },
   { AVM_LOG_VALID_INFO_ALLOC,                "Valid Info Alloc"       },
   { AVM_LOG_NODE_INFO_ALLOC,                 "Node Info Alloc"         },
   { AVM_LOG_LIST_NODE_INFO_ALLOC,            "List Node Info Alloc"    }, 
   { AVM_LOG_ENT_INFO_LIST_ALLOC,             "Entity Info List Alloc"  },
   { AVM_LOG_PATTERN_ARRAY_ALLOC,             "Pattern Array Alloc"     },
   { AVM_LOG_PATTERN_ALLOC,                   "Pattern Alloc"           },
   { AVM_LOG_MEM_ALLOC_SUCCESS,               "Success "                },
   { AVM_LOG_MEM_ALLOC_FAILURE,               "Failure "                }, 

   { 0,0 }
};

/******************************************************************************
                          Canned string for ROLE 
 ******************************************************************************/
const NCSFL_STR avm_role_set[] = 
{
   { AVM_LOG_RDA_REG             , "RDA Register"      },
   { AVM_LOG_RDA_GET_ROLE        , "RDA Get Role"      },
   { AVM_LOG_RDA_UNINSTALL       , "RDA Uninstall"     },
   { AVM_LOG_RDA_SET_ROLE        , "RDA Seting AvM  Role" },
   { AVM_LOG_RDA_AVM_SET_ROLE    , "AVM Setting RDA Role" },
   { AVM_LOG_RDA_AVM_ROLE        , "Current HA State    " },
   { AVM_LOG_RDA_HB              , "HB Lost    " },
   { AVM_LOG_RDA_CBK         , "RDA Rcv Cbk"      },
   { AVM_LOG_SND_ROLE_CHG    , "Send Role Change to Other SCXB" },
   { AVM_LOG_SND_ROLE_RSP    , "Send Role RSP to Other SCXB" },
   { AVM_LOG_RCV_ROLE_RSP    , "Rcv Role RSP from Other SCXB" },
   { AVM_LOG_SWOVR_FAILURE   , "Switchover Operation" },
   { AVM_LOG_ROLE_CHG    ,      "Role Change RSP" },
   { AVM_LOG_AVD_ROLE_RSP    ,      "AvD Role RSP" },
   { AVM_LOG_ROLE_QUIESCED   ,      "SCXB Quiesced" },
   { AVM_LOG_ROLE_ACTIVE     ,      "SCXB Active" },
   { AVM_LOG_ROLE_STANDBY    ,      "SCXB Standby" },
   { AVM_LOG_RDA_SUCCESS     , "Success"          },
   { AVM_LOG_RDA_FAILURE     , "Failure"          },
   { AVM_LOG_RDA_IGNORE      , "Ignore"           }, 
   { 0,0 }
};

/******************************************************************************
   Logging Stuff for Events
******************************************************************************/
const NCSFL_STR avm_evt_set[] =
{
   {AVM_LOG_EVT_SRC,                      "Received Event from    "},
   {AVM_LOG_EVT_HOTSWAP,                  "Received Hotswap Event :"},
   {AVM_LOG_EVT_RESOURCE,                 "Received Resource Event :"},
   {AVM_LOG_EVT_SENSOR,                   "Received Sensor  Event :"},
   {AVM_LOG_EVT_SAME_SENSOR,              "Same Sensor Asserted Again"},
   {AVM_LOG_EVT_IGNORE_SENSOR_ASSERT,     "Entity is not yet (Deasert && Failover Resp), Ignore the assert"}, 
   {AVM_LOG_EVT_SENSOR_ASSERT,            "Post Sensor Assert to FSM"},
   {AVM_LOG_EVT_IGNORE_SENSOR_DEASSERT,   "Sensor asserts still pending to be deasserted, Ignore the deassert"},
   {AVM_LOG_EVT_SHUTDOWN_RESP,            "Received Shutdown Resp "},
   {AVM_LOG_EVT_FAILOVER_RESP,            "Received Failover Resp "},
   {AVM_LOG_EVT_RESET_REQ,                "Received Reset Req from "},
   {AVM_LOG_EVT_FSM,                      "FSM evt:                "},
   {AVM_LOG_EVT_INVALID,                  "Invalid Event  "},
   {AVM_LOG_EVT_IGNORE,                   "Ignore Event        "},
   {AVM_LOG_EVT_NONE,                     " "                   },
   { 0, 0} 
}; 

/******************************************************************************
                          Canned string for EVENT QUEUE
 ******************************************************************************/
const NCSFL_STR avm_evt_q_set[] = 
{
   { AVM_LOG_EVT_Q_ENQ         , "Enqueue"      },
   { AVM_LOG_EVT_Q_DEQ         , "Dequeue"      },
   { AVM_LOG_EVT_Q_SUCCESS     , "Success"          },
   { AVM_LOG_EVT_Q_FAILURE     , "Failure"          },
   { AVM_LOG_EVT_Q_IGNORE      , "Ignore"           }, 
   { 0,0 }
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET avm_str_set[] = 
{
   { AVM_FC_HDLN,           0, (NCSFL_STR*) avm_hdln_set    },
   { AVM_FC_SEAPI,          0, (NCSFL_STR*) avm_seapi_set   },
   { AVM_FC_CB,             0, (NCSFL_STR*) avm_cb_set      },
   { AVM_FC_MBX,            0, (NCSFL_STR*) avm_mbx_set     },
   { AVM_FC_PATRICIA,       0, (NCSFL_STR*) avm_pat_set     },
   { AVM_FC_TASK,           0, (NCSFL_STR*) avm_task_set    },
   { AVM_FC_RDE,            0, (NCSFL_STR*) avm_rde_set     },
   { AVM_FC_MDS,            0, (NCSFL_STR*) avm_mds_set     },
   { AVM_FC_MAS,            0, (NCSFL_STR*) avm_mas_set     },
   { AVM_FC_EDA,            0, (NCSFL_STR*) avm_eda_set     },
   { AVM_FC_TMR,            0, (NCSFL_STR*) avm_tmr_set     }, 
   { AVM_FC_MBCSV,          0, (NCSFL_STR*) avm_mbcsv_set },
   { AVM_FC_HPL,            0, (NCSFL_STR*) avm_hpl_set     },
   { AVM_FC_MEM,            0, (NCSFL_STR*) avm_mem_set     },
   { AVM_FC_ROLE,           0, (NCSFL_STR*) avm_role_set    },
   { AVM_FC_EVT,            0, (NCSFL_STR*) avm_evt_set     },
   { AVM_FC_EVT_Q,          0, (NCSFL_STR*) avm_evt_q_set   },
   { 0,0,0 }
};

NCSFL_FMAT avm_fmat_set[] = 
{
   { AVM_LID_HDLN,         NCSFL_TYPE_TIC,     "%s AVM: %s %s\n"},
   { AVM_LID_HDLN_VAL,     NCSFL_TYPE_TICLL,   "%s AVM: %s at %s:%ld val %ld\n"},
   { AVM_LID_HDLN_STR,     "TICLP",            "%s AVM: %s at %s:%ld val %s\n"},
   { AVM_LID_PATRICIA_VAL, "TIIP",             "%s AVM: %s %s val %ld\n"},
   { AVM_LID_PATRICIA_VAL_STR,    "TIIC",       "%s AVM: %s %s val %s\n"}, 
   { AVM_LID_MAS,          NCSFL_TYPE_TIIL,   "%s AVM: %s %s Id %ld \n"}, 
   { AVM_LID_EDA,          NCSFL_TYPE_TIIL,   "%s AVM: %s %s Id %ld \n"},
   { AVM_LID_TMR,          NCSFL_TYPE_TIIL,   "%s AVM: %s %s %ld \n"}, 
   { AVM_LID_TMR_IGN,      NCSFL_TYPE_TICL,    "%s AVM %s  %s Id %ld\n"}, 
   { AVM_LID_MEM,          NCSFL_TYPE_TIICL,   "%s AVM: %s %s at %s:%ld\n"},
   { AVM_LID_ROLE,         NCSFL_TYPE_TIL,    "%s AVM: %s %ld \n"},
   { AVM_LID_ROLE_SRC,     NCSFL_TYPE_TILL,    "%s AVM: %s Role %ld Rsp %ld\n"},
   { AVM_LID_MBCSV,        NCSFL_TYPE_TIIL,    "%s AVM: %s %s %ld\n"}, 
   { AVM_LID_HPL_API,      NCSFL_TYPE_TICI,    "%s AVM: %s %s \n"}, 

   { AVM_LID_FUNC_RETVAL,  NCSFL_TYPE_TII,     "%s AVM: %s %s\n"},
   { AVM_LID_HPI_HS_EVT,     "TICLL",           "%s AVM: %s %s %ld %ld\n"},
   { AVM_LID_HPI_RES_EVT,     "TCIL",           "%s AVM: %s %s  Event: %ld\n"},
   { AVM_LID_EVT_AVD_DTL,      "TIP",              "%s AVM: %s %s \n"},
   { AVM_LID_EVT,               NCSFL_TYPE_TILL,    "%s AVM: %s %ld %ld\n"},
   { AVM_LID_ENT_INFO,          NCSFL_TYPE_TILLL,   "%s AVM: %s Current State: %ld Previous State: %ld FSM_EVT: %ld\n"},
   { AVM_LID_GEN_INFO1,         NCSFL_TYPE_TC,     "%s AVM:  %s\n"},  
   { AVM_LID_GEN_INFO2,        "TCP",              "%s AVM: %s %s \n"},
   { AVM_LID_GEN_INFO3,        "TCC",              "%s AVM: %s %s \n"},
   { AVM_LID_EVT_Q,             NCSFL_TYPE_TILL,   "%s AVM: %s Event Id : %ld Boolean : %ld\n"},
   { 0, 0, 0 }
};


NCSFL_ASCII_SPEC avm_ascii_spec = 
  {
    NCS_SERVICE_ID_AVM,            /* AVM subsystem */
    AVM_LOG_VERSION,               /* AVM log version ID */
    "AVM",
    (NCSFL_SET*)  avm_str_set,     /* AVM const strings referenced by index */
    (NCSFL_FMAT*) avm_fmat_set,    /* AVM string format info */
    0,                             /* Place holder for str_set count */
    0                              /* Place holder for fmat_set count */
  };

/*****************************************************************************

  avm_reg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/
EXTERN_C uns32 avm_reg_strings(void)
{
    NCS_DTSV_REG_CANNED_STR arg;
    
    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
    arg.info.reg_ascii_spec.spec = &avm_ascii_spec;
    if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
        return NCSCC_RC_FAILURE;
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  avm_unreg_strings

  DESCRIPTION: Function is used for unregistering the canned strings with the 
               DTS.

*****************************************************************************/
EXTERN_C uns32 avm_unreg_strings(void)
{
    NCS_DTSV_REG_CANNED_STR arg;

    arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
    arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_AVM;
    arg.info.dereg_ascii_spec.version = AVM_LOG_VERSION;

    if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
        return NCSCC_RC_FAILURE;
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : avm_log_str_lib_req
 *
 * Description   : Library entry point for mbcsv log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which SBOM gives.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32 avm_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = avm_reg_strings();
         break;
      
      case NCS_LIB_REQ_DESTROY:
         res = avm_unreg_strings();
         break;

      default:
         break;
   }
   return (res);
}

#endif /* (NCS_DTS == 1) */

