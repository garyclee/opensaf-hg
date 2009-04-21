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

  This file contains canned strings that are used for logging SRMSv common 
  information.
..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

#if (NCS_DTS == 1)

#include "srmsv.h"

/******************************************************************************
                          Canned string for SE-API
 ******************************************************************************/
const NCSFL_STR srmsv_seapi_set[] = 
{
   { SRMSV_LOG_SEAPI_CREATE , "SE-API Create"  },
   { SRMSV_LOG_SEAPI_DESTROY, "SE-API Destroy" },
   { SRMSV_LOG_SEAPI_SUCCESS, "Success"        },
   { SRMSV_LOG_SEAPI_FAILURE, "Failure"        },
   { 0,0 }
};

/******************************************************************************
                      Canned string for Memory
******************************************************************************/
const NCSFL_STR srmsv_mem_set[] = 
{
   {SRMSV_MEM_SRMA_MSG        , "Mem alloc for struct: SRMA_MSG"                 },
   {SRMSV_MEM_SRMA_CRT_RSRC   , "Mem alloc for struct: SRMA_CREATE_RSRC_MON_NODE"},
   {SRMSV_MEM_SRMND_MSG       , "Mem alloc for struct: SRMND_MSG"                },
   {SRMSV_MEM_SRMND_CRTD_RSRC , "Mem alloc for struct: SRMND_CREATED_RSRC_MON"   },
   {SRMSV_MEM_ALLOC_SUCCESS   , "Success"                                        },
   {SRMSV_MEM_ALLOC_FAILED    , "Failed"                                         },
   {0, 0}
};

/******************************************************************************
                     Canned string for SRMSv Resource Mon cfgs
******************************************************************************/
const NCSFL_STR srmsv_rsrc_mon_set[] = 
{
   { SRMSV_LOG_RSRC_MON_CREATE   ,   "Rsrc Mon Create"        },
   { SRMSV_LOG_RSRC_MON_DELETE   ,   "Rsrc Mon Delete"        },
   { SRMSV_LOG_RSRC_MON_SUBSCR   ,   "Rsrc Mon Subscribed"    },
   { SRMSV_LOG_RSRC_MON_UNSUBSCR ,   "Rsrc Mon Un Subscribed" },
   { SRMSV_LOG_RSRC_MON_MODIFY   ,   "Rsrc Mon Create"        },
   { SRMSV_LOG_RSRC_MON_BULK_CREATE, "Rsrc Mon Bulk Create"   },
   { SRMSV_LOG_RSRC_MON_SUCCESS  ,   "Success"                },
   { SRMSV_LOG_RSRC_MON_FAILED   ,   "Failed"                 },
   { 0,0 }
};

/******************************************************************************
                          Canned string for MDS
******************************************************************************/
const NCSFL_STR srmsv_mds_set[] = 
{
   { SRMSV_LOG_MDS_REG         , "MDS Register"     },
   { SRMSV_LOG_MDS_INSTALL     , "MDS Install"      },
   { SRMSV_LOG_MDS_SUBSCRIBE   , "MDS Subscribe"    },
   { SRMSV_LOG_MDS_UNREG       , "MDS Un-Register"  },
   { SRMSV_LOG_MDS_ADEST_CRT   , "MDS create ADEST" },
   { SRMSV_LOG_MDS_SEND        , "MDS Send"         },
   { SRMSV_LOG_MDS_PRM_GET     , "MDS Param Get"    },
   { SRMSV_LOG_MDS_RCV_CBK     , "MDS Rcv Cbk"      },
   { SRMSV_LOG_MDS_CPY_CBK     , "MDS Copy Cbk"     },
   { SRMSV_LOG_MDS_SVEVT_CBK   , "MDS Svc Evt Cbk"  },
   { SRMSV_LOG_MDS_ENC_CBK     , "MDS Enc Cbk"      },
   { SRMSV_LOG_MDS_FLAT_ENC_CBK, "MDS Flat Enc Cbk" },
   { SRMSV_LOG_MDS_DEC_CBK     , "MDS Dec Cbk"      },
   { SRMSV_LOG_MDS_FLAT_DEC_CBK, "MDS Flat Dec Cbk" },
   { SRMSV_LOG_MDS_SUCCESS     , "Success"          },
   { SRMSV_LOG_MDS_FAILURE     , "Failure"          },
   { 0,0 }
};

/******************************************************************************
                          Canned string for EDU
******************************************************************************/
const NCSFL_STR srmsv_edu_set[] = 
{
   { SRMSV_LOG_EDU_INIT    , "EDU Init"     },
   { SRMSV_LOG_EDU_FINALIZE, "EDU Finalize" },
   { SRMSV_LOG_EDU_SUCCESS , "Success"      },
   { SRMSV_LOG_EDU_FAILURE , "Failure"      },
   { 0,0 }
};

/******************************************************************************
                          Canned string for LOCK
******************************************************************************/
const NCSFL_STR srmsv_lock_set[] = 
{
   { SRMSV_LOG_LOCK_INIT    , "Lck Init"     },
   { SRMSV_LOG_LOCK_DESTROY , "Lck Destroy" },
   { SRMSV_LOG_LOCK_TAKE    , "Lck Take"     },
   { SRMSV_LOG_LOCK_GIVE    , "Lck Give"     },
   { SRMSV_LOG_LOCK_SUCCESS , "Success"      },
   { SRMSV_LOG_LOCK_FAILURE , "Failure"      },
   { 0,0 }
};


/******************************************************************************
                          Canned string for Timer
******************************************************************************/
const NCSFL_STR srmsv_tmr_set[] = 
{
   { SRMSV_LOG_TMR_INIT    , "Timer Init"     },
   { SRMSV_LOG_TMR_DESTROY , "Timer Destroy"  },
   { SRMSV_LOG_TMR_EXP     , "Timer Take"     },
   { SRMSV_LOG_TMR_STOP    , "Timer Give"     },
   { SRMSV_LOG_TMR_START   , "Timer Give"     },
   { SRMSV_LOG_TMR_CREATE  , "Timer Destroy"  },
   { SRMSV_LOG_TMR_DELETE  , "Timer Destroy"  },
   { SRMSV_LOG_TMR_SUCCESS , "Success"        },
   { SRMSV_LOG_TMR_FAILURE , "Failure"        },
   { 0,0 }
};

/******************************************************************************
                       Canned string for control block
 ******************************************************************************/
const NCSFL_STR srmsv_cb_set[] = 
{
   { SRMSV_LOG_CB_CREATE  ,  "CB Create"   },
   { SRMSV_LOG_CB_DESTROY ,  "CB Destroy"  },
   { SRMSV_LOG_CB_RETRIEVE,  "CB Retrieve" },
   { SRMSV_LOG_CB_RETURN  ,  "CB Return"   },
   { SRMSV_LOG_CB_SUCCESS ,  "Success"     },
   { SRMSV_LOG_CB_FAILURE ,  "Failure"     },
   { 0,0 }
};

/******************************************************************************
                       Canned string for Hdl Manager
******************************************************************************/
const NCSFL_STR srmsv_hdl_set[] = 
{
   { SRMSV_LOG_HDL_CREATE_CB         ,  "CB HDL Create"         },
   { SRMSV_LOG_HDL_DESTROY_CB        ,  "CB HDL Destroy"        },
   { SRMSV_LOG_HDL_RETRIEVE_CB       ,  "CB HDL Retrieve"       },
   { SRMSV_LOG_HDL_RETURN_CB         ,  "CB HDL Return"         },
   { SRMSV_LOG_HDL_CREATE_RSRC_MON   ,  "RSRC_MON HDL Create"   },
   { SRMSV_LOG_HDL_DESTROY_RSRC_MON  ,  "RSRC_MON HDL Destroy"  },
   { SRMSV_LOG_HDL_RETRIEVE_RSRC_MON ,  "RSRC_MON HDL Retrieve" },
   { SRMSV_LOG_HDL_RETURN_RSRC_MON   ,  "RSRC_MON HDL Return"   },
   { SRMSV_LOG_HDL_CREATE_USER       ,  "User Appl HDL Create"  },
   { SRMSV_LOG_HDL_DESTROY_USER      ,  "User Appl HDL Destroy" },
   { SRMSV_LOG_HDL_RETRIEVE_USER     ,  "USER HDL Retrieve"     },
   { SRMSV_LOG_HDL_RETURN_USER       ,  "USER HDL Return"       },
   { SRMSV_LOG_HDL_SUCCESS           ,  "Success"               },
   { SRMSV_LOG_HDL_FAILURE           ,  "Failure"               },
   { 0,0 }
};

/******************************************************************************
                       Canned string for Mailbox
 ******************************************************************************/
const NCSFL_STR srmsv_mbx_set[] = 
{
   { SRMSV_LOG_MBX_CREATE , "Mbx Create"  },
   { SRMSV_LOG_MBX_ATTACH , "Mbx Attach"  },
   { SRMSV_LOG_MBX_DESTROY, "Mbx Destroy" },
   { SRMSV_LOG_MBX_DETACH , "Mbx Detach"  },
   { SRMSV_LOG_MBX_SEND   , "Mbx Send"    },
   { SRMSV_LOG_MBX_SUCCESS, "Success"     },
   { SRMSV_LOG_MBX_FAILURE, "Failure"     },
   { 0,0 }
};

/******************************************************************************
                       Canned string for Task, IPC, Mailbox
 ******************************************************************************/
const NCSFL_STR srmsv_tim_set[] = 
{
   { SRMSV_LOG_TASK_CREATE , "Task Create"  },
   { SRMSV_LOG_TASK_START  , "Task Start"   },
   { SRMSV_LOG_TASK_RELEASE, "Task Release" },
   { SRMSV_LOG_IPC_CREATE  , "IPC Create"   },
   { SRMSV_LOG_IPC_ATTACH  , "IPC Attach"   },
   { SRMSV_LOG_IPC_SEND    , "IPC Send"     },
   { SRMSV_LOG_IPC_DETACH  , "IPC Detach"   },
   { SRMSV_LOG_IPC_RELEASE , "IPC Release"  },
   { SRMSV_LOG_TIM_SUCCESS , "Success"      },
   { SRMSV_LOG_TIM_FAILURE , "Failure"      },
   { 0,0 }
};

/******************************************************************************
                       Canned string for Patricia Tree
 ******************************************************************************/
const NCSFL_STR srmsv_pat_set[] = 
{
   { SRMSV_LOG_PAT_INIT   , "Patricia Init"    },
   { SRMSV_LOG_PAT_DESTROY, "Patricia Destroy" },
   { SRMSV_LOG_PAT_ADD    , "Patricia Add"     },
   { SRMSV_LOG_PAT_DEL    , "Patricia Del"     },
   { SRMSV_LOG_PAT_SUCCESS, "Success"          },
   { SRMSV_LOG_PAT_FAILURE, "Failure"          },
   { 0,0 }
};


/****************************************************************************
  Name          : srmsv_log_str_lib_req
 
  Description   : Library entry point for SRMSv log string library.
 
  Arguments     : req_info  - This is the pointer to the input information 
                              which SBOM gives.  
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 
  Notes         : None.
 *****************************************************************************/
uns32 srmsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   /* SRMA Registration */
   res = srma_log_str_lib_req(req_info);
   if (res != NCSCC_RC_SUCCESS)
      return res;

   /* SRMND Registration */
   res = srmnd_log_str_lib_req(req_info);

   return (res);
}

#endif /* (NCS_DTS == 1) */

