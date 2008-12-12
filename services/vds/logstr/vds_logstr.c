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

  DESCRIPTION:

  This file contains canned strings that are used for logging vds 
  information.
..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

#if (NCS_DTS == 1)

#include "vds.h"



/******************************************************************************
                      Canned string for Memory
******************************************************************************/

const NCSFL_STR vds_mem_set[] =
{ 
  { VDS_LOG_MEM_VDS_CB, "Mem alloc for struct: VDS_CB"                    },
  { VDS_LOG_MEM_VDS_DB_INFO, "Mem alloc for struct: VDS_DBINFO_NODE"      },
  { VDS_LOG_MEM_VDS_ADEST_INFO, "Mem alloc for struct: VDS_ADEST_NODE"    },
  { VDS_LOG_MEM_VDA_INFO,  "MEM alloc for VDA Info"                       },
  { VDS_LOG_MEM_VDA_INFO_FREE,  "MEM free for VDA Info"                   },
  { VDS_LOG_MEM_VDS_EVT, "Mem alloc for struct: VDS_EVT"                  },
  { VDS_LOG_MEM_VDS_EVT_FREE, "Mem free for struct: VDS_EVT"                  },
  { VDS_LOG_MEM_VDS_CKPT_BUFFER, "MEM Alloc for CKPT BUFFER"        },
  { VDS_LOG_MEM_ALLOC_SUCCESS, "Success"                            },
  { VDS_LOG_MEM_ALLOC_FAILURE, "Failure "                                   },
  { VDS_LOG_MEM_NOTHING, " "                                                },
  { 0 ,0 }
};

const NCSFL_STR vds_evt_set[] =
{
  { VDS_LOG_EVT_CREATE,  "VDS Event Created  "          },
  { VDS_LOG_EVT_RECEIVE, "VDS Event Received "          },
  { VDS_LOG_EVT_DESTROY, "VDS Event Destroyed "         },
  { VDS_LOG_EVT_VDEST_CREATE, "VDest Create Event "     },  
  { VDS_LOG_EVT_VDEST_LOOKUP, "VDest Lookup Event "     }, 
  { VDS_LOG_EVT_VDEST_DESTROY, "VDest Destroy Event "   },
  { VDS_LOG_EVT_SUCCESS,  "Success "                    }, 
  { VDS_LOG_EVT_FAILURE,  "Failure "                     },
  { VDS_LOG_EVT_NOTHING      ,  " "               },
  { 0 ,0 }
};


/******************************************************************************
                          Canned string for MDS
******************************************************************************/
const NCSFL_STR vds_mds_set[] = 
{
   { VDS_LOG_MDS_REG         , "MDS Register"     },
   { VDS_LOG_MDS_INSTALL     , "MDS Install"      },
   { VDS_LOG_MDS_SUBSCRIBE   , "MDS Subscribe"    },
   { VDS_LOG_MDS_UNREG       , "MDS Un-Register"  },
   { VDS_LOG_MDS_SEND        , "MDS Send"         },
   { VDS_LOG_MDS_CALL_BACK     , "MDS CAll Back"    },
   { VDS_LOG_MDS_RCV_CBK     , "MDS Rcv Cbk"      },
   { VDS_LOG_MDS_CPY_CBK     , "MDS Copy Cbk"     },
   { VDS_LOG_MDS_SVEVT_CBK   , "MDS Svc Evt Cbk"  },
   { VDS_LOG_MDS_ENC_CBK     , "MDS Enc Cbk"      },
   { VDS_LOG_MDS_FLAT_ENC_CBK , "MDS Flat Enc Cbk" },
   { VDS_LOG_MDS_DEC_CBK     , "MDS Dec Cbk"      },
   { VDS_LOG_MDS_FLAT_DEC_CBK , "MDS Flat Dec Cbk" },
   { VDS_LOG_MDS_ROLE_ACK ,     "MDS Role Ack" },
   { VDS_LOG_VDA_ROLECHANGE ,   "VDA Role Change" },
   { VDS_LOG_MDS_SUCCESS     , "Success"          },
   { VDS_LOG_MDS_FAILURE     , "Failure"          },
   { VDS_LOG_MDS_NOTHING      ,  " "               },
   { 0,0 }
};


/******************************************************************************
                          Canned string for LOCK
******************************************************************************/
const NCSFL_STR vds_lock_set[] = 
{
   { VDS_LOG_LOCK_INIT    , "Lck Init"     },
   { VDS_LOG_LOCK_DESTROY , "Lck Destroy" },
   { VDS_LOG_LOCK_TAKE    , "Lck Take"     },
   { VDS_LOG_LOCK_GIVE    , "Lck Give"     },
   { VDS_LOG_LOCK_SUCCESS , "Success"      },
   { VDS_LOG_LOCK_FAILURE , "Failure"      },
   { 0,0 }
};



/******************************************************************************
                       Canned string for control block
 ******************************************************************************/
const NCSFL_STR vds_cb_set[] = 
{
   { VDS_LOG_CB_CREATE  ,  "CB Create"   },
   { VDS_LOG_CB_DESTROY ,  "CB Destroy"  },
   { VDS_LOG_CB_RETRIEVE ,  "CB Retrieve" },
   { VDS_LOG_CB_RETURN  ,  "CB Return"   },
   { VDS_LOG_CB_SUCCESS ,  "Success"     },
   { VDS_LOG_CB_FAILURE ,  "Failure"     },
   { 0,0 }
};

/******************************************************************************
                       Canned string for Hdl Manager
******************************************************************************/
const NCSFL_STR vds_hdl_set[] = 
{
   { VDS_LOG_HDL_CREATE_CB         ,  "CB HDL Create"         },
   { VDS_LOG_HDL_DESTROY_CB        ,  "CB HDL Destroy"        },
   { VDS_LOG_HDL_RETRIEVE_CB       ,  "CB HDL Retrieve"       },
   { VDS_LOG_HDL_RETURN_CB         ,  "CB HDL Return"         },
   { VDS_LOG_HDL_SUCCESS           ,  "Success"               },
   { VDS_LOG_HDL_FAILURE           ,  "Failure"               },
   { 0,0 }
};

/******************************************************************************
                       Canned string for Task, IPC, Mailbox
 ******************************************************************************/
const NCSFL_STR vds_tim_set[] = 
{
   { VDS_LOG_TASK_CREATE , "Task Create"  },
   { VDS_LOG_TASK_START  , "Task Start"   },
   { VDS_LOG_TASK_RELEASE , "Task Release" },
   { VDS_LOG_IPC_CREATE  , "IPC Create"   },
   { VDS_LOG_IPC_ATTACH  , "IPC Attach"   },
   { VDS_LOG_IPC_SEND    , "IPC Send"     },
   { VDS_LOG_IPC_DETACH  , "IPC Detach"   },
   { VDS_LOG_IPC_RELEASE , "IPC Release"  },
   { VDS_LOG_MBX_CREATE ,  "Mbx Create"  },
   { VDS_LOG_MBX_DESTROY , "Mbx Destroy" },
   { VDS_LOG_TIM_SUCCESS , "Success"      },
   { VDS_LOG_TIM_FAILURE , "Failure"      },
   { 0,0 }
};

/******************************************************************************
                       Canned string for Patricia Tree
 ******************************************************************************/
const NCSFL_STR vds_pat_set[] = 
{
   { VDS_LOG_PAT_INIT    , "Patricia Init"    },
   { VDS_LOG_PAT_DESTROY , "Patricia Destroy" },
   { VDS_LOG_PAT_ADD_ID     , "Patricia Add with ID"     },
   { VDS_LOG_PAT_DEL_ID     , "Patricia Del with ID"     },
   { VDS_LOG_PAT_ADD_NAME , "Patricia  Add with Name"    },
   { VDS_LOG_PAT_DEL_NAME ,"Patricia Del with Name" },
   { VDS_LOG_PAT_SUCCESS , "Success"          },
   { VDS_LOG_PAT_FAILURE , "Failure"          },
   { 0,0 }
};



/******************************************************************************
                    Canned string for miscellaneous VDS events
******************************************************************************/
const NCSFL_STR vds_misc_set[] = 
{
   { VDS_LOG_MISC_VDS_UP       ,  "VDA Up "         },
   { VDS_LOG_MISC_VDS_DN       ,  "VDA Down "       },
   { VDS_LOG_MISC_VDS_CREATE   ,  "VDS Create "     },
   { VDS_LOG_MISC_VDS_DESTROY  ,  "VDS Destroy "    },
   { VDS_LOG_MISC_SUCCESS      ,  "Success "        },
   { VDS_LOG_MISC_FAILURE      ,  "Failure "         },
   { VDS_LOG_MISC_NOTHING      ,  " "               },
   { 0,0 }
};

/******************************************************************************
                    Canned string for AMF specific VDS events
******************************************************************************/
const NCSFL_STR vds_amf_set[] = 
{
   { VDS_LOG_AMF_CBK_INIT      ,  "VDS AMF CBK init"            },
   { VDS_LOG_AMF_INITIALIZE    ,  "VDS AMF Initialize"          },
   { VDS_LOG_AMF_FINALIZE      ,  "VDS AMF Finalize"            },
   { VDS_LOG_AMF_GET_SEL_OBJ   ,  "VDS AMF get sel obj"         },
   { VDS_LOG_AMF_GET_COMP_NAME ,  "VDS AMF get comp name"       },
   { VDS_LOG_AMF_INIT      ,      "VDS AMF Init"                 },
   { VDS_LOG_AMF_FIN      ,       "VDS AMF Fin"                 },
   { VDS_LOG_AMF_STOP_HLTCHECK ,  "VDS AMF stop health check"   },
   { VDS_LOG_AMF_START_HLTCHECK ,  "VDS AMF start health check"  },
   { VDS_LOG_AMF_REG_COMP      ,  "VDS AMF reg comp"            },
   { VDS_LOG_AMF_UNREG_COMP    ,  "VDS AMF Error Report"          },
   { VDS_LOG_AMF_DISPATCH      ,  "VDS AMF Dispatch"            },
   { VDS_LOG_AMF_RESPONSE      ,  "VDS AMF Send Response"       },
   { VDS_LOG_AMF_ACTIVE        ,  "VDS AMF Got ACTIVE Assignment"  },
   { VDS_LOG_AMF_QUIESCING     ,  "VDS AMF Got QUIESCING Assignment" },
   { VDS_LOG_AMF_QUIESCED      ,  "VDS AMF Got QUIESCED Assignment "},
   { VDS_LOG_AMF_STANDBY       ,  "VDS AMF Got STANDBY Assignment "},

   { VDS_LOG_AMF_QUIESCING_QUIESCED ,  "VDS AMF QUIESCING_QUIESEED"},
   { VDS_LOG_AMF_TERM_CALLBACK ,  "VDS AMF Terminate Call-back "},
   { VDS_LOG_AMF_REMOVE_CALLBACK, "VDS AMF Remove Call-back" },
   { VDS_LOG_AMF_SET_CALLBACK   , "VDS AMF CSI SET CAllback Called" },
   { VDS_LOG_AMF_INVALID ,        "VDS AMF got INVALID STATE "},  
   { VDS_LOG_AMF_SUCCESS       ,  "Success"                     },
   { VDS_LOG_AMF_FAILURE        ,  "Failure"                    },
   { VDS_LOG_AMF_NOTHING       ,  " "                           },
   { 0,0 }
};



const NCSFL_STR  vds_ckpt_set[] = 
{
 { VDS_LOG_CKPT_CBK_INIT ,          "VDS CKPT CBK init"                         }, 
 { VDS_LOG_CKPT_INITIALIZE ,        "VDS CKPT Initialize"                       },
 { VDS_LOG_CKPT_FINALIZE ,          "VDS CKPT Finalize"                         },
 { VDS_LOG_CKPT_REG_COMP ,          "VDS CKPT reg comp api "                    },
 { VDS_LOG_CKPT_UNREG_COMP ,        "VDS CKPT unreg comp api "                  },
 { VDS_LOG_CKPT_INIT ,              "VDS CKPT Init api"                         },
 { VDS_LOG_CKPT_FIN ,               "VDS CKPT Finalize api"                     },
 { VDS_LOG_CKPT_OPEN ,              "VDS CKPT Open api"                          },
 { VDS_LOG_CKPT_CLOSE ,             "VDS CKPT Close api"                         },
 { VDS_LOG_CKPT_READ ,              "VDS CkPT read"                              },
 { VDS_LOG_CKPT_WRITE ,             "VDS CkPT write"                             },
 { VDS_LOG_CKPT_OVERWRITE ,         "VDS CkPT overwrite "                        },
 { VDS_LOG_CKPT_DELETE ,            "VDS CkPT delete "                           },
 { VDS_LOG_CKPT_SEC_DB_CWRITE ,     "VDS CKPT DB Section create,write api"    },
 { VDS_LOG_CKPT_SEC_DB_READ ,       "VDS CKPT DB Section Read api"            },
 { VDS_LOG_CKPT_SEC_DB_OVERWRITE ,  "VDS CKPT DB Section Over Write api"      }, 
 { VDS_LOG_CKPT_SEC_DELETE ,        "VDS CKPT  Section Delete api"               }, 
 { VDS_LOG_CKPT_SEC_CB_CWRITE ,     "VDS CKPT CB Section create,write api"    },
 { VDS_LOG_CKPT_SEC_CB_READ ,       "VDS CKPT CB Section Read api"            },
 { VDS_LOG_CKPT_SEC_CB_OVERWRITE ,  "VDS CKPT CB Section Over Write api"      },
 { VDS_LOG_CKPT_SEC_ITER_INIT ,     "VDS CKPT Section Iterator Init api"          },
 { VDS_LOG_CKPT_SEC_ITER_NEXT ,     "VDS CKPT Section Next Iteration api"          },
 { VDS_LOG_CKPT_SEC_ITER_FIN ,      "VDS CKPT Section Iterator FIn api"          },
 { VDS_LOG_CKPT_SEC_VDS_VERSION_CREATE, "VDS CKPT Section Create for VDS Version" },
 { VDS_LOG_CKPT_SEC_VDS_VERSION_READ, "VDS CKPT Section Read for VDS Version" },
 { VDS_LOG_CKPT_SUCCESS ,           "Success"                                    },
 { VDS_LOG_CKPT_FAILURE ,           "Failure"                                   },
 { VDS_LOG_CKPT_NOTHING ,           " "                                              },
 { 0,0}
};



/******************************************************************************
       Build up the canned constant strings for the ASCII SPEC
******************************************************************************/
NCSFL_SET vds_str_set[] = 
{
   { VDS_FC_MDS    ,    0,  (NCSFL_STR *) vds_mds_set      },
   { VDS_FC_LOCK   ,    0,  (NCSFL_STR *) vds_lock_set     },
   { VDS_FC_CB     ,    0,  (NCSFL_STR *) vds_cb_set       },
   { VDS_FC_HDL    ,    0,  (NCSFL_STR *) vds_hdl_set      },
   { VDS_FC_TIM    ,    0,  (NCSFL_STR *) vds_tim_set      },
   { VDS_FC_TREE   ,    0,  (NCSFL_STR *) vds_pat_set      },
   { VDS_FC_MEM    ,    0,  (NCSFL_STR *) vds_mem_set      },
   { VDS_FC_EVT    ,    0,  (NCSFL_STR *) vds_evt_set      },
   { VDS_FC_AMF    ,    0,  (NCSFL_STR *) vds_amf_set      },
   { VDS_FC_CKPT    ,   0,  (NCSFL_STR *) vds_ckpt_set     },
   { VDS_FC_MISC   ,    0,  (NCSFL_STR *) vds_misc_set     },
   { 0, 0, 0 }
};


NCSFL_FMAT vds_fmat_set[] = 
{
   /* <MDS Register/Install/...> <Success/Failure> */
   { VDS_LID_MDS    , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <Lock Init/Finalize/Take/Give> <Success/Failure> */
   { VDS_LID_LOCK   , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <CB Create/Destroy/...> <Success/Failure> */
   { VDS_LID_CB     , NCSFL_TYPE_TII,  "[%s] %s %s\n"                        },

   /* <Hdl DB Create/Destroy/...> <Success/Failure>: Hdl: <hdl> */
   { VDS_LID_HDL , NCSFL_TYPE_TII, "[%s] %s %s\n"                            },

   /* <Task/Mbx create/destroy...> <Success/Failure> */
   { VDS_LID_TIM , NCSFL_TYPE_TII, "[%s] %s %s\n"                            },

    /* <PAT tree> <Success/Failure> */
   { VDS_LID_TREE  , NCSFL_TYPE_TIIL,  "[%s] %s %s %ld \n"                         },

   /* <Memory Alloc> <Success/Failure> */
   { VDS_LID_MEM  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                          },
   
   /* <Event > <Type of Event> */
   { VDS_LID_EVT  , NCSFL_TYPE_TII,  "[%s] %s %s\n"                          },
   
   /* <AMF> <Success/Failure> */
   { VDS_LID_AMF  , NCSFL_TYPE_TIIL,  "[%s] %s %s %lu\n"                          },
   
   /* <CKPT> <Success/Failure> */
   { VDS_LID_CKPT  , NCSFL_TYPE_TIIL,  "[%s] %s %s %lu\n"                          },

   /* <VDS Miscelleneous...> */
   { VDS_LID_MISC   , NCSFL_TYPE_TII,   "[%s] %s  %s\n"                          },

   { VDS_LID_VDEST, "TCLC", "[%s] VDEST Name %s VDESTID %lu Persistance %s\n"     },
   
   { VDS_LID_AMF_STATE ,"TCC", "[%s] CSI Assignment done from %s  to %s\n" }, 
   { VDS_LID_GENERIC, "TCLC","[%s]  %s   %lu  %s  \n"},
   { VDS_LID_MSG_FMT, "TCCLCL", "[%s] %s %s  Service ID %lu with  %s  %lu \n" },
   {0,0,0}
};


NCSFL_ASCII_SPEC vds_ascii_spec = 
{
   NCS_SERVICE_ID_VDS,          /* VDS service id */
   VDS_LOG_VERSION,             /* VDS log version id */
   "VDS",                       /* used for generating log filename */
   (NCSFL_SET *)  vds_str_set,  /* VDS const strings ref by index */
   (NCSFL_FMAT *) vds_fmat_set, /* VDS string format info */
   0,                             /* placeholder for str_set cnt */
   0                              /* placeholder for fmat_set cnt */
};


/****************************************************************************
  Name          : vds_log_str_lib_req
 
  Description   : Library entry point for VDS log string library.
 
  Arguments     : req_info  - This is the pointer to the input information 
                              which SBOM gives.  
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 
  Notes         : None.
 *****************************************************************************/
uns32 vds_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = vds_logstr_reg();
         break;

      case NCS_LIB_REQ_DESTROY:
         res = vds_logstr_unreg();
         break;

      default:
         break;
   }
   return (res);
}


/****************************************************************************
  Name          :  vds_logstr_reg
 
  Description   :  This routine registers VDS with flex log service.
                  
  Arguments     :  None.
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
 *****************************************************************************/
uns32 vds_logstr_reg()
{
   NCS_DTSV_REG_CANNED_STR arg;

   m_NCS_OS_MEMSET(&arg, 0, sizeof(NCS_DTSV_REG_CANNED_STR)); 
   
   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
   arg.info.reg_ascii_spec.spec = &vds_ascii_spec;

   if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;
    
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  vds_logstr_unreg
 
  Description   :  This routine unregisters VDS with flex log service.
                  
  Arguments     :  None.
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
 *****************************************************************************/
uns32 vds_logstr_unreg()
{
   NCS_DTSV_REG_CANNED_STR arg;

   m_NCS_OS_MEMSET(&arg, 0, sizeof(NCS_DTSV_REG_CANNED_STR)); 

   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
   arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_VDS;
   arg.info.dereg_ascii_spec.version = VDS_LOG_VERSION;

   if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;
    
   return NCSCC_RC_SUCCESS;
}

#endif  /* (NCS_DTS == 1) */

