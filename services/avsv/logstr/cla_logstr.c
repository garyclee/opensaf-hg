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

  This file contains canned strings that are used for logging CLA information.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#if (NCS_DTS == 1)

#include "ncsgl_defs.h"
#include "t_suite.h"
#include "saAis.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "avsv_log.h"
#include "cla_log.h"
#include "avsv_logstr.h"


uns32 cla_str_reg (void);
uns32 cla_str_unreg (void);

/******************************************************************************
                      Canned string for selection object
 ******************************************************************************/
const NCSFL_STR cla_sel_obj_set[] = 
{
   { CLA_LOG_SEL_OBJ_CREATE  , "Sel Obj Create" },
   { CLA_LOG_SEL_OBJ_DESTROY , "Sel Obj Destroy" },
   { CLA_LOG_SEL_OBJ_IND_SEND, "Sel Obj Ind Send" },
   { CLA_LOG_SEL_OBJ_IND_REM , "Sel Obj Ind Rem" },
   { CLA_LOG_SEL_OBJ_SUCCESS , "Success" },
   { CLA_LOG_SEL_OBJ_FAILURE , "Failure" },
   { 0,0 }
};


/******************************************************************************
                          Canned string for CLM APIs
 ******************************************************************************/
/* ensure that the string set ordering matches that of API type definition */
const NCSFL_STR cla_api_set[] = 
{
   { CLA_LOG_API_INITIALIZE                , "saClmInitialize()"                 },
   { CLA_LOG_API_FINALIZE                  , "saClmFinalize()"                   },
   { CLA_LOG_API_TRACK_START               , "saClmClusterTrackStart()"          },
   { CLA_LOG_API_TRACK_STOP                , "saClmClusterTrackStop()"           },
   { CLA_LOG_API_NODE_GET                  , "saClmClusterNodeGet()"             },
   { CLA_LOG_API_NODE_ASYNC_GET            , "saClmClusterNodeGetAsync()"        },
   { CLA_LOG_API_SEL_OBJ_GET               , "saClmSelectionObjectGet()"         },
   { CLA_LOG_API_DISPATCH                  , "saClmDispatch()"                   },
   { CLA_LOG_API_ERR_SA_OK                 , "Ok"                                },
   { CLA_LOG_API_ERR_SA_LIBRARY            , "Lib Err"                           },
   { CLA_LOG_API_ERR_SA_VERSION            , "Ver Err"                           },
   { CLA_LOG_API_ERR_SA_INIT               , "Init Err"                          },
   { CLA_LOG_API_ERR_SA_TIMEOUT            , "Timeout Err"                       },
   { CLA_LOG_API_ERR_SA_TRY_AGAIN          , "Try Again Err"                     },
   { CLA_LOG_API_ERR_SA_INVALID_PARAM      , "Inv Param Err"                     },
   { CLA_LOG_API_ERR_SA_NO_MEMORY          , "No Mem Err"                        },
   { CLA_LOG_API_ERR_SA_BAD_HANDLE         , "Bad Hdl Err"                       },
   { CLA_LOG_API_ERR_SA_BUSY               , "Busy Err"                          },
   { CLA_LOG_API_ERR_SA_ACCESS             , "Access Err Err"                    },
   { CLA_LOG_API_ERR_SA_NOT_EXIST          , "Does Not Exist Err"                },
   { CLA_LOG_API_ERR_SA_NAME_TOO_LONG      , "Name Too Long Err"                 },
   { CLA_LOG_API_ERR_SA_EXIST              , "Exist Err"                         },
   { CLA_LOG_API_ERR_SA_NO_SPACE           , "No Space Err"                      },
   { CLA_LOG_API_ERR_SA_INTERRUPT          , "Interrupt Err"                     },
   { CLA_LOG_API_ERR_SA_SYSTEM             , "Sys Err"                           },
   { CLA_LOG_API_ERR_SA_NAME_NOT_FOUND     , "Name Not Found Err"                },
   { CLA_LOG_API_ERR_SA_NO_RESOURCES       , "No Resource Err"                   },
   { CLA_LOG_API_ERR_SA_NOT_SUPPORTED      , "Not Supp Err"                      },
   { CLA_LOG_API_ERR_SA_BAD_OPERATION      , "Bad Oper Err"                      },
   { CLA_LOG_API_ERR_SA_FAILED_OPERATION   , "Failed Oper Err"                   },
   { CLA_LOG_API_ERR_SA_MESSAGE_ERROR      , "Msg Err"                           },
   { CLA_LOG_API_ERR_SA_NO_MESSAGE         , "No Msg Err"                        },
   { CLA_LOG_API_ERR_SA_QUEUE_FULL         , "Q Full Err"                        },
   { CLA_LOG_API_ERR_SA_QUEUE_NOT_CLAILABLE, "Q Not Available Err"               },
   { CLA_LOG_API_ERR_SA_BAD_CHECKPOINT     , "Bad Checkpoint Err"                },
   { CLA_LOG_API_ERR_SA_BAD_FLAGS          , "Bad Flags Err"                     },
   { 0,0 }
};


/******************************************************************************
                     Canned string for CLA handle database
 ******************************************************************************/
const NCSFL_STR cla_hdl_db_set[] = 
{
   { CLA_LOG_HDL_DB_CREATE     , "Hdl DB Create"      },
   { CLA_LOG_HDL_DB_DESTROY    , "Hdl DB Destroy"     },
   { CLA_LOG_HDL_DB_REC_ADD    , "Hdl DB Rec Add"     },
   { CLA_LOG_HDL_DB_REC_CBK_ADD, "Hdl DB Rec Cbk Add" },
   { CLA_LOG_HDL_DB_REC_DEL    , "Hdl DB Rec Del"     },
   { CLA_LOG_HDL_DB_SUCCESS    , "Success"            },
   { CLA_LOG_HDL_DB_FAILURE    , "Failure"            },
   { 0,0 }
};


/******************************************************************************
                    Canned string for miscellaneous CLA events
 ******************************************************************************/
const NCSFL_STR cla_misc_set[] = 
{
   { CLA_LOG_MISC_AVND_UP, "AvND Up" },
   { CLA_LOG_MISC_AVND_DN, "AvND Down" },
   { 0,0 }
};



/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/
NCSFL_SET cla_str_set[] = 
{
   { CLA_FC_SEAPI  , 0, (NCSFL_STR *) avsv_seapi_set  },
   { CLA_FC_MDS    , 0, (NCSFL_STR *) avsv_mds_set    },
   { CLA_FC_EDU    , 0, (NCSFL_STR *) avsv_edu_set    },
   { CLA_FC_LOCK   , 0, (NCSFL_STR *) avsv_lock_set   },
   { CLA_FC_CB     , 0, (NCSFL_STR *) avsv_cb_set     },
   { CLA_FC_CBK    , 0, (NCSFL_STR *) avsv_cbk_set    },
   { CLA_FC_SEL_OBJ, 0, (NCSFL_STR *) cla_sel_obj_set },
   { CLA_FC_API    , 0, (NCSFL_STR *) cla_api_set     },
   { CLA_FC_HDL_DB , 0, (NCSFL_STR *) cla_hdl_db_set  },
   { CLA_FC_MISC   , 0, (NCSFL_STR *) cla_misc_set    },
   { 0, 0, 0 }
};

NCSFL_FMAT cla_fmat_set[] = 
{
   /* <SE-API Create/Destroy> <Success/Failure> */
   { CLA_LID_SEAPI  , NCSFL_TYPE_TII,  "[%s] %s %s\n"             },

   /* <MDS Register/Install/...> <Success/Failure> */
   { CLA_LID_MDS    , NCSFL_TYPE_TII,  "[%s] %s %s\n"             },

   /* <EDU Init/Finalize> <Success/Failure> */
   { CLA_LID_EDU    , NCSFL_TYPE_TII,  "[%s] %s %s\n"             },

   /* <Lock Init/Finalize/Take/Give> <Success/Failure> */
   { CLA_LID_LOCK   , NCSFL_TYPE_TII,  "[%s] %s %s\n"             },

   /* <CB Create/Destroy/...> <Success/Failure> */
   { CLA_LID_CB     , NCSFL_TYPE_TII,  "[%s] %s %s\n"             },

   /* Invoked <cbk-name> */
   { CLA_LID_CBK    , NCSFL_TYPE_TI,   "[%s] Invoked %s \n"       },

   /* <Sel Obj Create/Destroy/...> <Success/Failure> */
   { CLA_LID_SEL_OBJ, NCSFL_TYPE_TII,  "[%s] %s %s\n"             },

   /* Processed <API Name>: <Ok/...(clm-err-code)> */
   { CLA_LID_API    , NCSFL_TYPE_TII,  "[%s] Processed %s: %s\n"  },

   /* <Hdl DB Create/Destroy/...> <Success/Failure>: Hdl: <hdl> */
   { CLA_LID_HDL_DB , NCSFL_TYPE_TIIL, "[%s] %s %s: Hdl: %u\n"    },

   /* <AvND Up/Down...> */
   { CLA_LID_MISC   , NCSFL_TYPE_TI,   "[%s] %s\n"                },

   { 0, 0, 0 }
};

NCSFL_ASCII_SPEC cla_ascii_spec = 
{
   NCS_SERVICE_ID_CLA,          /* CLA service id */
   AVSV_LOG_VERSION,            /* CLA log version id */
   "CLA",                       /* used for generating log filename */
   (NCSFL_SET *)  cla_str_set,  /* CLA const strings ref by index */
   (NCSFL_FMAT *) cla_fmat_set, /* CLA string format info */
   0,                           /* placeholder for str_set cnt */
   0                            /* placeholder for fmat_set cnt */
};


/****************************************************************************
  Name          : cla_str_reg
 
  Description   : This routine registers the CLA canned strings with DTS.
                  
  Arguments     : None
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
 *****************************************************************************/
uns32 cla_str_reg (void)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_DTSV_REG_CANNED_STR arg;

   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
   arg.info.reg_ascii_spec.spec = &cla_ascii_spec;
   
   rc = ncs_dtsv_ascii_spec_api(&arg);

   return rc;
}


/****************************************************************************
  Name          : cla_str_unreg
 
  Description   : This routine unregisters the CLA canned strings from DTS.
                  
  Arguments     : None
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
 *****************************************************************************/
uns32 cla_str_unreg (void)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_DTSV_REG_CANNED_STR arg;

   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
   arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_CLA;
   arg.info.dereg_ascii_spec.version = AVSV_LOG_VERSION;

   rc = ncs_dtsv_ascii_spec_api(&arg);

   return rc;
}
#endif /* (NCS_DTS == 1) */
