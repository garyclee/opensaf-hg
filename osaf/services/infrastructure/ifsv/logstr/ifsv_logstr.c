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
  FILE NAME: IFSV_LOGSTR.C

  DESCRIPTION: This file contains all the log string related to IfSv.

  ******************************************************************************/

/*****************************************************************************
 * Canned string for the LOG mask "NCSFL_LC_EVENT".
 *****************************************************************************/
/* Get general definitions */
#include "ncsgl_defs.h"

/* Get target's suite of header files...*/
#include "t_suite.h"

/* From /base/common/inc */
#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncsft_rms.h"
#include "ncs_ubaid.h"
#include "ncsencdec.h"
#include "ncs_stack.h"
#include "ncs_mib.h"
#include "ncs_mtbl.h"
#include "ncs_log.h"
#include "dta_papi.h"
#include "dts_papi.h"
#include "ifsv_papi.h"
#include "ifsv_log.h"
#include "ifsv_logstr.h"

const NCSFL_STR ifsv_log_evt_str[] =
{
   /* IfD Receive Events */
   {IFSV_LOG_IFD_EVT_INTF_CREATE_RCV,                   "IfD Receive Interface Create Event"},
   {IFSV_LOG_IFD_EVT_INTF_DESTROY_RCV,                  "IfD Receive Interface Destroy Event"},
   {IFSV_LOG_IFD_EVT_INIT_DONE_RCV,                     "IfD Receive IfD Init Done Event"},   
   {IFSV_LOG_IFD_EVT_IFINDEX_REQ_RCV,                   "IfD Receive Ifindex Request Event"},
   {IFSV_LOG_IFD_EVT_IFINDEX_CLEANUP_RCV,               "IfD Receive Ifindex CleanUp Event"},
   {IFSV_LOG_IFD_EVT_AGING_TMR_RCV,                     "IfD Receive Aging Timer Expiry Event"},
   {IFSV_LOG_IFD_EVT_RET_TMR_RCV,                       "IfD Receive Ret Timer Expiry Event"},
   {IFSV_LOG_IFD_EVT_REC_SYNC_RCV,                      "IfD Receive Syncup Interface Record Event"},
   {IFSV_LOG_IFD_EVT_SVC_UPD_RCV,                       "IfD Receive SVCD UPD Event"},
   {IFSV_LOG_IFD_EVT_SVC_UPD_FAILURE,                   "IfD SVCD UPD Failure"},
   {IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,                  "Standby IfD Receives Async Update"},
   {IFSV_LOG_IFD_EVT_A2S_SYNC_RESP_RCV,                     "Standby IfD Receives Sync Resp"},
   /* IfD Send Events */
   {IFSV_LOG_IFD_EVT_INTF_CREATE_SND,                   "IfD Send Interface Create Event"},
   {IFSV_LOG_IFD_EVT_INTF_DESTROY_SND,                  "IfD Send Interface Destroy Event"},
   {IFSV_LOG_IFD_EVT_INIT_DONE_SND,                     "IfD Send IfD Init Done Event"},
   {IFSV_LOG_IFD_EVT_AGING_TMR_SND,                     "IfD Send Aging Timer Expiry Event"},
   {IFSV_LOG_IFD_EVT_IFINDEX_RESP_SND,                  "IfD Send Ifindex Responce Event"},
   {IFSV_LOG_IFD_EVT_ALL_REC_SND,                       "IfD Sent All Rec to IfND, whose dest id : "},
   {IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,              "Active IfD Send Async Update"},

   /* IfND Recevie Events */
   {IFSV_LOG_IFND_EVT_INTF_CREATE_RCV,                  "IfND Receive Interface Create Event"},
   {IFSV_LOG_IFND_EVT_INTF_DESTROY_RCV,                 "IfND Receive Interface Destroy Event"},
   {IFSV_LOG_IFND_EVT_INIT_DONE_RCV,                    "IfND Receive IfND Init Done Event"},   
   {IFSV_LOG_IFND_EVT_IFINDEX_RESP_RCV,                 "IfND Receive Ifindex Reponse Event"},   
   {IFSV_LOG_IFND_EVT_INTF_INFO_GET_RCV,                "IfND Receive Get Interface Info Event"},
   {IFSV_LOG_IFND_EVT_STATS_RESP_RCV,                   "IfND Receive statistics Responce Event"},
   {IFSV_LOG_IFND_EVT_AGING_TMR_RCV,                    "IfND Receive Aging Timer Expiry Event"},
   {IFSV_LOG_IFND_EVT_MDS_DEST_GET_TMR_RCV,             "IfND Receive MDS Dest Get Timer Expiry Event"},
   {IFSV_LOG_IFND_EVT_SVCD_UPD_FROM_IFA_RCV,            "IfND Receive SVCD UPD Event From IFA"},
   {IFSV_LOG_IFND_EVT_SVCD_UPD_FROM_IFD_RCV,            "IfND Receive SVCD UPD Event From IFD"},
   {IFSV_LOG_IFND_EVT_SVCD_GET_REQ_RCV,                 "IfND Receive SVCD Get Event"},
   {IFSV_LOG_IFND_EVT_IFD_UP_RCV,                       "IfND Receive IFD UP Event"},
   /* IfND Send Events */
   {IFSV_LOG_IFND_EVT_INTF_CREATE_SND,                  "IfND Send Interface Create Event"},
   {IFSV_LOG_IFND_EVT_INTF_DESTROY_SND,                 "IfND Send Interface Destroy Event"},
   {IFSV_LOG_IFND_EVT_INIT_DONE_SND,                    "IfND Send IfND Init Done Event"},
   {IFSV_LOG_IFND_EVT_IFINDEX_REQ_SND,                  "IfND Send Ifindex Request Event"},
   {IFSV_LOG_IFND_EVT_IFINDEX_CLEANUP_SND,              "IfND Send Ifindex CleanUP Event"},
   {IFSV_LOG_IFND_EVT_AGING_TMR_SND,                    "IfND Send Interface Aging Timer Expiry Event"},   
   {IFSV_LOG_IFND_EVT_REC_SYNC_SND,                     "IfND Send Interface Record Sync To IfD"},   
   {IFSV_LOG_IFND_EVT_IDIM_HEALTH_CHK_SND,              "IfND Send Health Check To IDIM"},   
   {IFSV_LOG_IFND_EVT_GET_HW_STATS_SND,                 "IfND Send Get Hardware Stats To IDIM"},   
   {IFSV_LOG_IFND_EVT_SET_HW_PARAM_SND,                 "IfND Send Set Hardware Param To IDIM"},   
   /* IDIM Receive Events */
   {IFSV_LOG_IDIM_EVT_HW_STATS_RCV,                     "IDIM Receives Hardware Stats From IfSv Drv"},   
   {IFSV_LOG_IDIM_EVT_HW_PORT_REG_RCV,                  "IDIM Receives Port Registeration From IfSv Drv"},   
   {IFSV_LOG_IDIM_EVT_HW_PORT_STATUS_RCV,               "IDIM Receives Port Status From IfSv Drv"},   
   {IFSV_LOG_IDIM_EVT_GET_HW_STATS_RCV,                 "IDIM Receives Get Hadrware stats From IfND"},
   {IFSV_LOG_IDIM_EVT_SET_HW_PARAM_RCV,                 "IDIM Receives Set Hadrware param From IfND"},
   {IFSV_LOG_IDIM_EVT_HEALTH_CHK_RCV,                   "IDIM Receives SAF Health Check From IfSv Drv"},   
   /* IDIM Send Events */
   {IFSV_LOG_IDIM_EVT_GET_HW_DRV_STATS_SND,             "IDIM Send Get Hardware Stats To IfSv Drv"},   
   {IFSV_LOG_IDIM_EVT_SET_HW_DRV_PARAM_SND,             "IDIM Send Set Hardware Param To IfSv Drv"},   
   {IFSV_LOG_IDIM_EVT_IFND_UP_SND,                      "IDIM Send IfND UP Event To IfSv Drv"},   
   {IFSV_LOG_IDIM_EVT_INTF_CREATE_SND,                  "IDIM Send Interface Create Event"},   
   {IFSV_LOG_IDIM_EVT_INTF_DESTROY_SND,                 "IDIM Send Interface Destroy Event"},   
   {IFSV_LOG_IDIM_EVT_INTF_STATS_RESP_SND,              "IDIM Send Interface Stats Responce"},   
   {IFSV_LOG_MBCSV_MSG,                            "IfSv MBCSV Error/Success:"}, 
   {IFSV_LOG_IFA_EVT_INFO,                         "IfSv IFA Info : "},   
   {IFSV_LOG_IFA_IPXS_EVT_INFO,                    "IfSv IFA IPXS Info : "},   
   {IFSV_LOG_IFND_IPXS_EVT_INFO,                   "IfSv IFND IPXS Info : "},   
   {IFSV_LOG_IFD_IPXS_EVT_INFO,                    "IfSv IFD IPXS Info : "},   
   {IFSV_LOG_UNKNOWN_EVENT,                        "IfSv Unknown Event"},   
   {IFSV_LOG_IFSV_CREATE_MOD_MSG,                  "IfSv Interface Create or Modify Msg"},
   {IFSV_LOG_IFD_MSG,                  "IfD Info : "},
   {0, 0}  
};

/*****************************************************************************
 * Canned string for the LOG mask "NCSFL_LC_SYS_CALL_FAIL".
 *****************************************************************************/
const NCSFL_STR ifsv_log_sys_call_fail_str[] =
{  
   {IFSV_LOG_MSG_QUE_CREATE_FAIL,        "IfSv Queue Create Failure"},
   {IFSV_LOG_MSG_QUE_SEND_FAIL,          "IfSv Send Queue Failure"},
   {IFSV_LOG_MSG_QUE_RCV_FAIL,           "IfSv Receive Queue Failure"},
   {IFSV_LOG_TASK_CREATE_FAIL,           "IfSv Task Create Failure"},
   {IFSV_LOG_SEM_CREATE_FAIL,            "IfSv Sem Create Failure"},
   {IFSV_LOG_SEM_TAKE_FAIL,              "IfSv Sem Take Failure"},
   {IFSV_LOG_SEM_GIVE_FAIL,              "IfSv Sem Give Failure"},   
   {IFSV_LOG_LOCK_LOCKED_FAIL,           "IfSv Lock Failed to Lock"},
   {IFSV_LOG_LOCK_UNLOCKED_FAIL,         "IfSv Lock Failed to Unlock"},
   {IFSV_LOG_LOCK_CREATE_FAIL,           "IfSv Lock Create Failed"},
   {IFSV_LOG_LOCK_DESTROY_FAIL,          "IfSv Lock Destroy Failed"},   
   {IFSV_LOG_MEM_ALLOC_FAIL,             "IfSv Memory Alloc Failed"},
   {IFSV_LOG_MEM_FREE_FAIL,              "IfSv Memory Free Failed"},
   {IFSV_LOG_HDL_MGR_FAIL,               "IfSv Handle Manager Failed"},
   {0, 0}  
};

/*****************************************************************************
 * Canned string for the LOG mask "NCSFL_LC_MEMORY".
 *****************************************************************************/
const NCSFL_STR ifsv_log_mem_str[] =
{
   {IFSV_LOG_MEM_ALLOC,          "IfSv Memory Alloc"},
   {IFSV_LOG_MEM_FREE,           "IfSv Memory Free"},   
   {0, 0}  
};

/*****************************************************************************
 * Canned string for the LOG mask "NCSFL_LC_LOCKS".
 *****************************************************************************/
const NCSFL_STR ifsv_log_lock_str[] =
{
   {IFSV_LOG_LOCK_CREATE,          "IfSv Lock Created"},
   {IFSV_LOG_LOCK_DESTROY,         "IfSv Lock Destroyed"},
   {IFSV_LOG_LOCK_LOCKED,          "IfSv Lock Resource"},
   {IFSV_LOG_LOCK_UNLOCKED,        "IfSv Unlock Resource"},   
   {0, 0}  
};

/*****************************************************************************
 * Canned string for the LOG mask "NCSFL_LC_TIMER".
 *****************************************************************************/
const NCSFL_STR ifsv_log_tmr_str[] =
{
   {IFSV_LOG_TMR_CREATE,   "IfSv Timer Create"},
   {IFSV_LOG_TMR_START,    "IfSv Timer Start"},
   {IFSV_LOG_TMR_STOP,     "IfSv Timer Stop"},
   {IFSV_LOG_TMR_DELETE,   "IfSv Timer Delete"},
   {IFSV_LOG_TMR_EXPIRY,   "IfSv Timer Expiry"},
   {0, 0}  
};

/*****************************************************************************
 * Canned string for the LOG mask "NCSFL_LC_HEADLINE".
 *****************************************************************************/
const NCSFL_STR ifsv_log_headline_str[] =
{
   {IFSV_LOG_IF_TBL_CREATED,              "IfSv Interface Table Created"},
   {IFSV_LOG_IF_TBL_DESTROYED,            "IfSv Interface Table Destroy"},
   {IFSV_LOG_IF_MAP_TBL_CREATED,          "IfSv Interface Map Table Created"},
   {IFSV_LOG_IFND_NODE_ID_TBL_CREATED, "IfSv IfND Node ID Table Created"},
   {IFSV_LOG_IF_MAP_TBL_DESTROYED,        "IfSv Interface Map Table Destroy"},
   {IFSV_LOG_IF_TBL_CREATE_FAILURE,       "IfSv Interface Table Create Failure"},
   {IFSV_LOG_IF_MAP_TBL_CREATE_FAILURE,   "IfSv Interface Map Table Create Failure"},
   {IFSV_LOG_IFND_NODE_ID_TBL_CREATE_FAILURE,   "IfSv IfND Node ID Table Create Failure"},
   {IFSV_LOG_IF_TBL_DESTROY_FAILURE,      "IfSv Interface Table Destroy Failure"},
   {IFSV_LOG_IF_MAP_TBL_DESTROY_FAILURE,  "IfSv Interface Map Table Destroy Failure"},
   {IFSV_LOG_IF_TBL_ADD_SUCCESS,          "IfSv Interface Record Added"},
   {IFSV_LOG_IF_TBL_UPDATE,               "IfSv Interface Record Updated"},
   {IFSV_LOG_IF_MAP_TBL_ADD_SUCCESS,      "IF MAP TBL ENTRY ADDED"},
   {IFSV_LOG_IF_TBL_ADD_FAILURE,          "IfSv Interface Record Add Failed"},
   {IFSV_LOG_IF_MAP_TBL_ADD_FAILURE,      "IfSv Shelf/slot/port/type/scope Vs Ifindex Map Add Failed"},
   {IFSV_LOG_IF_TBL_DEL_SUCCESS,          "IfSv Interface Record Deleted"},
   {IFSV_LOG_IF_MAP_TBL_DEL_SUCCESS,      "IfSv Shelf/slot/port/type/scope Vs Ifindex Map Deleted"},
   {IFSV_LOG_IF_MAP_TBL_DEL_ALL_SUCCESS,  "IfSv All Ifindex Map deleted"},
   {IFSV_LOG_IF_TBL_DEL_FAILURE,          "IfSv Interface Record Delete Failure"},
   {IFSV_LOG_IF_MAP_TBL_DEL_FAILURE,      "IfSv Shelf/slot/port/type/scope Vs Ifindex Map Delete Failure"},
   {IFSV_LOG_IF_TBL_DEL_ALL_SUCCESS,      "IfSv Interface All Record Delete Success"},
   {IFSV_LOG_IF_TBL_DEL_ALL_FAILURE,      "IfSv Interface All Record Delete Failure"},
   {IFSV_LOG_IF_TBL_LOOKUP_SUCCESS,       "IfSv Interface Record Found"},
   {IFSV_LOG_IF_TBL_ALREADY_EXIST,        "IfSv Interface Record Already Exists"},
   {IFSV_LOG_IF_MAP_TBL_LOOKUP_SUCCESS,   "IfSv Shelf/slot/port/type/scope Vs Ifindex Map Found"},
   {IFSV_LOG_IF_TBL_LOOKUP_FAILURE,       "IfSv Interface Record Not Found"},
   {IFSV_LOG_SPT_TO_INDEX_LOOKUP_FAILURE,  "IfSv SPT to Index Look up failure. Index is :"},
   {IFSV_LOG_SPT_TO_INDEX_LOOKUP_SUCCESS,  "IfSv SPT to Index Look up Success. Index is :"},
   {IFSV_LOG_IF_MAP_TBL_LOOKUP_FAILURE,   "IfSv Shelf/slot/port/type/scope Vs Ifindex Map Not Found"},
   {IFSV_LOG_IFINDEX_ALLOC_SUCCESS,       "IfSv Ifindex Allocated Successfully"},
   {IFSV_LOG_IFINDEX_ALLOC_FAILURE,       "IfSv Ifindex Allocated Failure"},
   {IFSV_LOG_IFINDEX_FREE_SUCCESS,        "IfSv Ifindex Freed Successfully"},
   {IFSV_LOG_IFINDEX_FREE_FAILURE,        "IfSv Ifindex Freed Failure"},
   {IFSV_LOG_ADD_REC_TO_IF_RES_QUE,       "IfSv Add The Rec To The Ifindex Resolve Que"},
   {IFSV_LOG_REM_REC_TO_IF_RES_QUE,       "IfSv Remove The Rec From The Ifindex Resolve Que"},
   {IFSV_LOG_ADD_MOD_INTF_REC,            "IfSv Add/Modify Interface record"},
   {IFSV_LOG_DEL_INTF_REC,                "IfSv Delete Interface record success"},
   {IFSV_LOG_IF_STATS_INFO,               "IfSv Interface statistics"},
   {IFSV_LOG_MDS_SVC_UP,                  "IfSv MDS service UP"},
   {IFSV_LOG_IFD_RED_DOWN,                "The Active IFD is down but we are still in standby state : Deleting records of ifnd addr : ifd_nodeid :"},
   {IFSV_LOG_IFD_RED_DOWN_EVENT,          "Received RED_DOWN/NO_ACTIVE Event : Service ID : Node ID :"},
   {IFSV_LOG_MDS_SVC_DOWN,                "Received MDS service DOWN : Service ID : Node ID :"},
   {IFSV_LOG_MDS_RCV_MSG,                 "IfSv MDS receive MSG"},
   {IFSV_LOG_MDS_SEND_MSG,                "IfSv MDS send MSG"},
   {IFSV_LOG_MDS_DEC_SUCCESS,             "IfSv MDS Decoded MSG Success"},
   {IFSV_LOG_MDS_DEC_FAILURE,             "IfSv MDS Decoded MSG Failure"},
   {IFSV_LOG_MDS_ENC_SUCCESS,             "IfSv MDS Encoded MSG Success"},
   {IFSV_LOG_MDS_ENC_FAILURE,             "IfSv MDS Encoded MSG Failure"},
   {IFSV_LOG_STATE_INFO,                  "IfSv State Information"},
   {IFSV_LOG_MDS_DEST_TBL_ADD_FAILURE,    "IfSv Mds Dest Add Failed"},
   {IFSV_LOG_MDS_DEST_TBL_DEL_FAILURE,    "IfSv Mds Dest Del Failed"},
   {IFSV_LOG_SVCD_UPD_SUCCESS,            "IfSv SVCD Update Success"},
   {IFSV_LOG_NODE_ID_TBL_ADD_FAILURE,     "IfSv Node Id record add failure"},
   {IFSV_LOG_NODE_ID_TBL_DEL_FAILURE,     "IfSv Node Id record del failure"},
   {IFSV_LOG_NODE_ID_TBL_ADD_SUCCESS,     "IfSv Node Id record add success"},
   {IFSV_LOG_NODE_ID_TBL_DEL_SUCCESS,     "IfSv Node Id record del success"},
   {IFSV_LOG_IFIP_NODE_ID_TBL_ADD_SUCCESS, "IfSv IFIP Node Id record add success"},
   {IFSV_LOG_IFIP_NODE_ID_TBL_DEL_SUCCESS, "IfSv IFIP Node Id record del success"},
   {IFSV_LOG_IFIP_NODE_ID_TBL_ADD_FAILURE, "IfSv IFIP Node Id record add failure"},
   {IFSV_LOG_IFIP_NODE_ID_TBL_DEL_FAILURE, "IfSv IFIP Node Id record del failure"},
   {IFSV_LOG_IP_TBL_ADD_SUCCESS, "IfSv IP Table Id record add success"},
   {IFSV_LOG_IP_TBL_ADD_FAILURE, "IfSv IP Table Id record add failure"},
   {IFSV_LOG_IP_TBL_DEL_SUCCESS, "IfSv IP Table Id record del success"},
   {IFSV_LOG_IFSV_CREATE_MSG, "IfSv Interface Create Msg"},
   {IFSV_LOG_IFSV_CB_NULL, "IfD CB is  NULL while CSI set assignment"},
   {IFSV_LOG_IFSV_HA_QUIESCED_MSG, "Got CSI set assignment: QUIESCED"},
   {IFSV_LOG_IFSV_HA_ACTIVE_IN_WARM_COLD_MSG, "Error :Got CSI set assignment: when cold sync or warm sync is on."},
   {IFSV_LOG_IFSV_HA_ACTIVE_MSG, "Got CSI set assignment: ACTIVE"},
   {IFSV_LOG_IFSV_HA_STDBY_MSG, "Got CSI set assignment: STDBY"},
   {IFSV_WARM_COLD_SYNC_START, "IFD Warm or cold sync started"},
   {IFSV_WARM_COLD_SYNC_STOP, "IFD Warm or cold sync stopped"},
   {IFSV_LOG_FUNC_RET_FAIL, "Func returned failure"},
   {IFSV_AMF_RESP_AFTER_COLD_SYNC, "IFD Sending AMF Resp after cold sync completed"},
   {0, 0}  
};

/*****************************************************************************
 * Canned string for the LOG mask "NCSFL_LC_API".
 *****************************************************************************/
const NCSFL_STR ifsv_log_func_entry_str[] =
{
   {IFSV_LOG_FUNC_ENTERED,              "IfSv Enter Function"},
   {0, 0}  
};

/*****************************************************************************
 * Canned string for the LOG mask "NCSFL_LC_API".
 *****************************************************************************/
const NCSFL_STR ifsv_log_api_str[] =
{
   {IFSV_LOG_SE_INIT_DONE,         "IfSv SE API Initialization Done"},
   {IFSV_LOG_SE_INIT_FAILURE,      "IfSv SE Initialization Failure"},
   {IFSV_LOG_SE_DESTROY_DONE,      "IfSv SE Destroy Done"},
   {IFSV_LOG_SE_DESTROY_FAILURE,   "IfSv SE Destroy Failure"},
   {IFSV_LOG_CB_INIT_DONE,         "IfSv CB Initialization Done"},
   {IFSV_LOG_CB_INIT_FAILURE,      "IfSv CB Initialization Failure"},
   {IFSV_LOG_CB_DESTROY_DONE,      "IfSv CB Destroy Done"},
   {IFSV_LOG_CB_DESTROY_FAILURE,   "IfSv CB Destroy Failure"},
   {IFSV_LOG_MDS_INIT_DONE,        "IfSv MDS Initialization Done"},
   {IFSV_LOG_MDS_INIT_FAILURE,     "IfSv MDS Initialization Failure"},
   {IFSV_LOG_MDS_DESTROY_DONE,     "IfSv MDS Destroy Done"},
   {IFSV_LOG_MDS_DESTROY_FAILURE,  "IfSv MDS Destroy Failure"},
   {IFSV_LOG_RMS_INIT_DONE,        "IfSv RMS Initialization Done"},
   {IFSV_LOG_RMS_INIT_FAILURE,     "IfSv RMS Initialization Failure"},
   {IFSV_LOG_RMS_DESTROY_DONE,     "IfSv RMS Destroy Done"},
   {IFSV_LOG_RMS_DESTROY_FAILURE,  "IfSv RMS Destroy Failure"},
   {IFSV_LOG_AMF_INIT_DONE,        "IfSv AMF Initialization Done"},
   {IFSV_LOG_AMF_INIT_FAILURE,     "IfSv AMF Initialization Failure"},
   {IFSV_LOG_AMF_REG_DONE,         "IfSv AMF Registeration Done"},
   {IFSV_LOG_AMF_REG_FAILURE,      "IfSv AMF Registeration Failure"},
   {IFSV_LOG_AMF_HEALTH_CHK_START_DONE,  "IfSv AMF Health check started Done"},
   {IFSV_LOG_AMF_HEALTH_CHK_START_FAILURE, "IfSv AMF Health check start Failure"},
   {IFSV_LOG_AMF_HA_STATE_CHNG,    "IfSv AMF HA state Change"},
   {IFSV_LOG_AMF_READY_STATE_CHNG, "IfSv AMF Readiness state Change"},
   {IFSV_LOG_AMF_HEALTH_CHK,       "IfSv AMF Health check is called"},
   {IFSV_LOG_AMF_CONF_OPER,        "IfSv AMF Operation Confirm"},
   {IFSV_LOG_AMF_GET_OBJ_FAILURE,  "IfSv AMF Get Obj Failure"},
   {IFSV_LOG_AMF_DISP_FAILURE,     "IfSv AMF Dispatch Failure"},
   {IFSV_LOG_IDIM_INIT_DONE,       "IfSv IDIM Initialization Done"},
   {IFSV_LOG_IDIM_INIT_FAILURE,    "IfSv IDIM Initialization Failure"},
   {IFSV_LOG_IDIM_DESTROY_DONE,    "IfSv IDIM Destroy Done"},
   {IFSV_LOG_IDIM_DESTROY_FAILURE, "IfSv IDIM Destroy Failure"},
   {IFSV_LOG_DRV_INIT_DONE,       "IfSv DRV Initialization Done"},
   {IFSV_LOG_DRV_INIT_FAILURE,    "IfSv DRV Initialization Failure"},
   {IFSV_LOG_DRV_DESTROY_DONE,    "IfSv DRV Destroy Done"},
   {IFSV_LOG_DRV_DESTROY_FAILURE, "IfSv DRV Destroy Failure"},
   {IFSV_LOG_DRV_PORT_REG_DONE,   "IfSv DRV Port Reg Done"},
   {IFSV_LOG_DRV_PORT_REG_FAILURE,"IfSv DRV Port Reg Failure"},
   {0, 0}  
};

#if (NCS_VIP == 1)
const NCSFL_STR ifsv_log_vip_str[] =
{
  {IFSV_VIP_NULL_POOL_HDL,  "Pool Handle Null in VIPDatabase"},
  {IFSV_VIP_NULL_APPL_NAME,  "ApplName NULL in Handle of VIPDatabase"},
  {IFSV_INTERFACE_REQUESTED_ADMIN_DOWN, "Requested Interface State is Admin Down "},
  {IFSV_VIP_INVALID_INTERFACE_RECEIVED,  "Invalid Interface Name Received"},
  {IFSV_VIP_MDS_SEND_FAILURE,  "MDS Send Failure"},
  {IFSV_VIP_VIPDC_DATABASE_CREATED,  "VIPDC Database Created Successfully"},
  {IFSV_VIP_VIPD_DATABASE_CREATED,  "VIPD Database Created Successfully"},
  {IFSV_VIP_VIPDC_DATABASE_CREATE_FAILED,  "VIPDC Database Create Failed"},
  {IFSV_VIP_VIPD_DATABASE_CREATE_FAILED,  "VIPD Database Create Failed"},
  {IFSV_VIP_VIPD_LOOKUP_SUCCESS,  "VIPD Lookup Success"},
  {IFSV_VIP_VIPD_LOOKUP_FAILED,  "VIPD Lookup Failed"},
  {IFSV_VIP_VIPDC_LOOKUP_SUCCESS,  "VIPDC Lookup Success"},
  {IFSV_VIP_VIPDC_LOOKUP_FAILED,  "VIPDC Lookup Failed"},
  {IFSV_VIP_ADDED_IP_NODE_TO_VIPDC,  "IP Node Added Successfully to VIPDC Cache"},
  {IFSV_VIP_ADDED_IP_NODE_TO_VIPD,  "Successfully Added IP Node to VIPDatabase "},
  {IFSV_VIP_ADDED_INTERFACE_NODE_TO_VIPDC,  "Interface Node Added Successfully to VIPD Cache"},
  {IFSV_VIP_ADDED_INTERFACE_NODE_TO_VIPD,  "Successfully Added Interface Node to VIPDatabase "},
  {IFSV_VIP_ADDED_OWNER_NODE_TO_VIPDC,  "Successfully Added Owner Node to VIPDC"},
  {IFSV_VIP_ADDED_OWNER_NODE_TO_VIPD,  "Successfully Added Owner Node to VIPD  Record "},

  {IFSV_VIP_RECORD_ADDITION_TO_VIPDC_SUCCESS,  "VIPD Record Addition to VIPDC Success"},
  {IFSV_VIP_RECORD_ADDITION_TO_VIPDC_FAILURE,  "VIPDC Record Addition to VIPDC Failed"},
  {IFSV_VIP_RECORD_ADDITION_TO_VIPD_SUCCESS,  "Successfully Added Record to VIPD "},
  {IFSV_VIP_RECORD_ADDITION_TO_VIPD_FAILURE,  "failed to Add Record VIPD "},

  {IFSV_VIP_RECEIVED_REQUEST_FROM_SAME_IFA,  "Received Request from same IFA"},
  {IFSV_VIP_RECEIVED_VIP_EVENT,  "Received VIP Event"},
  {IFSV_VIP_INTERNAL_ERROR,  "Internal Error Occured "},
  {IFSV_VIP_INSTALLING_VIP_AND_SENDING_GARP,  "Installing VIP and Send a GARP"},

  {IFSV_VIP_VIPDC_CLEAR_FAILED,  "Unable to Clear VIPD Cache"},
  {IFSV_VIP_VIPD_CLEAR_FAILED,  "Unable to Clear VIPD"},

  {IFSV_VIP_LINK_LIST_NODE_DELETION_FAILED,  "LINK LIST Node Deletion Failed"},
  {IFSV_VIP_RECORD_DELETION_FAILED,  "Record Deletion Failed"},
  {IFSV_VIP_REQUESTED_VIP_ALREADY_EXISTS,  "Requested VIP Already Exists "},
  {IFSV_VIP_MEM_ALLOC_FAILED,  "Memory Allocation Failed "},

  {IFSV_VIP_EXISTS_ON_OTHER_INTERFACE,  "VIP Exists on Other Interface"},
  {IFSV_VIP_EXISTS_FOR_OTHER_HANDLE,  "VIP Exists for Other Handle"},
  {IFSV_VIP_IPXS_TABLE_LOOKUP_FAILED,  "IPXS Table Lookup Failed"},

  {IFSV_VIP_INVALID_PARAM,  "Invalid Parameters"},
  
/*
 * VIP MIB Related Log Strings
*/
  {IFSV_VIP_GET_NEXT_INITIALLY_NO_RECORD_FOUND, "GetNext::for InstId== NULL Initially No Records Found"},
  {IFSV_VIP_GET_NEXT_INITIALLY_RECORD_FOUND, "GetNext::for InstId== NULL Initial Record Found"},
  {IFSV_VIP_GET_NEXT_NO_RECORD_FOUND, "GetNext::for an InstId No Records Found"},
  {IFSV_VIP_GET_NEXT_RECORD_FOUND, "GetNext::for an InstId Record Found"},
  {IFSV_VIP_GET_NEXT_INVALID_INDEX_RECEIVED, "GetNext::Application Handle is Invalid"},
  {IFSV_VIP_GET_NO_RECORD_FOUND, "Get::for an InstId No Record Found"},
  {IFSV_VIP_GET_RECORD_FOUND, "Get::for an InstId Record Found"},
  {IFSV_VIP_GET_INVALID_INDEX_RECEIVED, "Get::Invalid Index Received is Invalid"},
  {IFSV_VIP_GET_NULL_INDEX_RECEIVED, "Get::NULL Index Received "},

/*
 * VIP CLI Related Log Strings
*/
  {IFSV_VIP_CLI_MIB_SYNC_REQ_CALL_FAILED, "appip_process_get_row_request::mib Sync request Failed "},
  {IFSV_VIP_CLI_SINGLE_ENTRY_DISPLAY, "appip_process_get_row_request::Entry Found "}, 
  {IFSV_VIP_CLI_DISPLAY_ALL_ENTRIES, "appip_process_get_row_request::Display All Entries "}, 
  {IFSV_VIP_CLI_INVALID_INDEX_RECEIVED, "appip_populate_display_data::Invalid Index to Populate Display data "}, 
  {IFSV_VIP_CLI_SHOW_ALL_FAILED, "appip_show_all_CEF::Failed "}, 
  {IFSV_VIP_CLI_DONE_FAILURE, "appip_ncs_cli_done::Failed "}, 
  {IFSV_VIP_CLI_INVALID_APPL_NAME_INPUT_TO_CLI, "appip_change_mode_appip_CEF::Invalid Application Name Specified By User"},
  {IFSV_VIP_CLI_INVALID_HANDLE_INPUT_TO_CLI, "appip_change_mode_appip_CEF::Invalid Handle Specified By User"},
  {IFSV_VIP_SINGLE_ENTRY_SHOW_CEF_FAILED, "appip_show_single_entry_CEF::Failed"},
  {IFSV_VIP_CLI_MODE_CHANGE_CEF_SUCCESS, "appip_change_mode_appip_CEF::Success"},

/*
 * VIP MIBLIB && OAC Registration RELATED
*/
  {IFSV_VIP_MIBLIB_REGISTRATION_SUCCESS, "vip_miblib_reg::Success"},
  {IFSV_VIP_MIBLIB_REGISTRATION_FAILURE, "vip_miblib_reg::Failure"},
  {IFSV_VIP_OAC_REGISTRATION_SUCCESS, "vip_reg_with_oac::Success"},
  {IFSV_VIP_OAC_REGISTRATION_FAILURE, "vip_reg_with_oac::Failure"},
  {IFSV_VIP_MIB_TABLE_REGISTRATION_SUCCESS, "ncsvipentry_tbl_reg::Success"},
  {IFSV_VIP_MIB_TABLE_REGISTRATION_FAILURE, "ncsvipentry_tbl_reg::Failure"},

  {IFSV_VIP_VIRTUAL_IP_ADDRESS_INSTALL_FAILED, "IFA VIP Installation Failed"},
  {IFSV_VIP_VIRTUAL_IP_ADDRESS_INSTALL_SUCCESS, "IFND VIP Installation Success"},
  {IFSV_VIP_VIRTUAL_IP_ADDRESS_FREE_FAILED, "IFND Vip free failed"},
  {IFSV_VIP_VIRTUAL_IP_ADDRESS_FREE_SUCCESS, "IFND Vip free success"},
  {IFSV_VIP_RECIVED_VIP_INSTALL_REQUEST, "IFA: received virtual ip install request"},
  {IFSV_VIP_SENDING_IFA_VIPD_INFO_ADD_REQ, "IFA: Sending IFA_VIPD_INFO_ADD_REQ"},
  {IFSV_VIP_RECEIVED_ERROR_RESP_FOR_INSTALL, "IFA: received error resp for install req"},
  {IFSV_VIP_RECEIVED_ERROR_RESP_FOR_FREE, "IFA:received errro resp for free req"},
  {IFSV_VIP_SENDING_IFA_VIP_FREE_REQ, "IFA: Sending IFA_VIP_FREE_REQ"},
  {IFSV_VIP_RECEIVED_IFA_VIP_FREE_REQ, "IFND: Received IFA_VIP_FREE_REQ"},
  {IFSV_VIP_RECEIVED_IFA_VIPD_INFO_ADD_REQ, "IFND: Received IFA_VIPD_INFO_ADD_REQ"},
  {IFSV_VIP_SENDING_IFND_VIPD_INFO_ADD_REQ, "IFND: Sending IFND_VIPD_INFO_ADD_REQ"},
  {IFSV_VIP_RECEIVED_IFND_VIPD_INFO_ADD_REQ, "IFD: Received IFND_VIPD_INFO_ADD_REQ"},
  {IFSV_VIP_RECEIVED_ERROR_RESP_FOR_VIPD_INFO_ADD, "Received error resp for VIPD_INFO_ADD"},
  {IFSV_VIP_SENDING_IPXS_ADD_IP_REQ, "IFND: Sending IPXS_ADD_IP_REQ"},
  {IFSV_VIP_SENDING_IPXS_INC_REFCNT_REQ, "IFND: Sending IPXS_INC_REFCNT_REQ"},
  {IFSV_VIP_RECEIVED_REQUEST_FROM_DIFFERENT_IFA, "IFND: Received request from different IFA"},
  {IFSV_VIP_SENDING_IFND_VIPD_INFO_ADD_REQ_RESP, "IFND: Sending IFND_VIPD_INFO_ADD_REQ_RESP"},
  {IFSV_VIP_SENDING_IFND_VIP_FREE_REQ, "IFND: Sending request to free Virtual IP"},
  {IFSV_VIP_RECEIVED_IFND_VIP_FREE_REQ, "IFD: Received request to free Virtual IP"},
  {IFSV_VIP_SENDING_IFND_VIP_FREE_REQ_RESP, "IFND: Sending response to free request"},
  {IFSV_VIP_RECEIVED_IFA_CRASH_MSG, "IFND: Received IFA Crashed Message"},
  {IFSV_VIP_RECORD_DELETION_SUCCESS, "IFSV: VIP/VIPDC record deletion success"},
  {IFSV_VIP_SENDING_IFD_VIP_FREE_REQ_RESP, "IFD: Sending response to free request"},
  {IFSV_VIP_SENDING_IFD_VIPD_INFO_ADD_REQ_RESP, "IFD: Sending response to vip add request"},

  {IFSV_VIP_CREATED_IFND_IFND_VIP_DEL_VIPD_EVT, "IFND : Created ifnd_ifnd_vip_del_vipd_evt"},
  {IFSV_VIP_RECEIVED_IFND_IFND_VIP_DEL_VIPD_EVT,"IFND : Received ifnd_ifnd_vip_del_vipd_evt"},
  {IFSV_VIP_VIPDC_RECORD_DELETION_FAILED,"IFND : Vipd record deletion from vipdc failed"}, 
  {IFSV_VIP_VIPD_RECORD_DELETION_FAILED, "IFD  : Vipd record deletion from vipdB failed"},
  {IFSV_VIP_VIPDC_RECORD_DELETION_SUCCESS,"IFND : Vipd record deletion from vipdc success"}, 
  {IFSV_VIP_VIPD_RECORD_DELETION_SUCCESS, "IFD  : Vipd record deletion from vipdB success"},
  {IFSV_VIP_SENDING_IPXS_DEL_IP_REQ,"IFND: Sending IPXS_DEL_IP_REQ"},
  {IFSV_VIP_SENDING_IPXS_DEC_REFCNT_REQ,"IFND: Sending IPXS_DEC_REFCNT_REQ"},
  {IFSV_VIP_INTF_REC_NOT_CREATED,"IFND: Interface record does not exist/created "},
  {IFSV_VIP_IP_EXISTS_ON_OTHER_INTERFACE,"IFND: Requested Virtual IP Address exists on other interface"},
  {IFSV_VIP_IP_EXISTS_FOR_OTHER_HANDLE,"IFND: Requested Virtual IP Address exists for other handle"},
  {IFSV_VIP_IP_EXISTS_BUT_NOT_VIP,"IFND: Requested Virtual IP Address already  exists but it is not VIP"},
  {IFSV_VIP_UNINSTALLING_VIRTUAL_IP,"IFND : Uninstalling Virtual IP Address"},

  {0, 0}
};
#endif

/*****************************************************************************
 * String set function maping, which maps with the Log type with the 
 * corresponding functions.
 *****************************************************************************/
NCSFL_SET ifsv_log_str_set[] = 
{
   {IFSV_LC_EVENT_L,           0, (NCSFL_STR*)ifsv_log_evt_str},   
   {IFSV_LC_EVENT_LL,          0, (NCSFL_STR*)ifsv_log_evt_str},   
   {IFSV_LC_EVENT_LLL,         0, (NCSFL_STR*)ifsv_log_evt_str},   
   {IFSV_LC_EVENT_INIT,        0, (NCSFL_STR*)ifsv_log_evt_str},   
   {IFSV_LC_EVENT_IFINDEX,     0, (NCSFL_STR*)ifsv_log_evt_str},

   {IFSV_LC_MEMORY,          0, (NCSFL_STR*)ifsv_log_mem_str},
   {IFSV_LC_LOCKS,           0, (NCSFL_STR*)ifsv_log_lock_str},
   {IFSV_LC_TIMER,           0, (NCSFL_STR*)ifsv_log_tmr_str},
   {IFSV_LC_SYS_CALL_FAIL,   0, (NCSFL_STR*)ifsv_log_sys_call_fail_str},      
   {IFSV_LC_HEADLINE,        0, (NCSFL_STR*)ifsv_log_headline_str},
   {IFSV_LC_HEADLINE_CL,     0, (NCSFL_STR*)ifsv_log_headline_str},
   {IFSV_LC_HEADLINE_CLLLLL, 0, (NCSFL_STR*)ifsv_log_headline_str},

   {IFSV_LC_FUNC_ENTRY_L,        0, (NCSFL_STR*)ifsv_log_func_entry_str},
   {IFSV_LC_FUNC_ENTRY_LL,       0, (NCSFL_STR*)ifsv_log_func_entry_str},
   {IFSV_LC_FUNC_ENTRY_LLL,      0, (NCSFL_STR*)ifsv_log_func_entry_str},
   {IFSV_LC_FUNC_ENTRY_LLLL,     0, (NCSFL_STR*)ifsv_log_func_entry_str},
   {IFSV_LC_FUNC_ENTRY_LLLLL,    0, (NCSFL_STR*)ifsv_log_func_entry_str},
   {IFSV_LC_FUNC_ENTRY_LLLLLL,   0, (NCSFL_STR*)ifsv_log_func_entry_str},

   {IFSV_LC_API_L,             0, (NCSFL_STR*)ifsv_log_api_str},   
   {IFSV_LC_API_LL,            0, (NCSFL_STR*)ifsv_log_api_str},
   {IFSV_LC_API_LLL,           0, (NCSFL_STR*)ifsv_log_api_str},
   {IFSV_LC_API_LLLL,          0, (NCSFL_STR*)ifsv_log_api_str},   
   {IFSV_LC_API_LLLLL,         0, (NCSFL_STR*)ifsv_log_api_str},
  
#if (NCS_VIP == 1)
   {IFSV_VIP_LC_API,         0, (NCSFL_STR*)ifsv_log_vip_str},
#endif
   {IFSV_LC_HEADLINE_CLLLLLLL,  0, (NCSFL_STR*)ifsv_log_evt_str},
   { 0,0,0 }
};

/*****************************************************************************
 * String format to be printed, which dictates the Flex Log to print in the 
 * format which we are specifing.
 *****************************************************************************/
NCSFL_FMAT ifsv_log_fmat_set[] = 
{
   { IFSV_LFS_EVENT_L,           NCSFL_TYPE_TICL,      "[%s] %s %s  %ld\n"     },
   { IFSV_LFS_EVENT_LL,          NCSFL_TYPE_TICLL,     "[%s] %s %s  %ld %ld\n"     },
   { IFSV_LFS_EVENT_LLL,         NCSFL_TYPE_TICLLL,    "[%s] %s %s  %ld %ld %ld\n"     },
   { IFSV_LFS_EVENT_INIT,        NCSFL_TYPE_TIL,      "[%s] %s %ld\n"     },
   { IFSV_LFS_EVENT_IFINDEX,     NCSFL_TYPE_TIL,      "[%s] %s Ifindex %ld\n"     },
   
   { IFSV_LFS_MEMORY,            NCSFL_TYPE_TIL,      "[%s] %s  %ld\n"     },

   { IFSV_LFS_LOCKS,             NCSFL_TYPE_TIL,      "[%s] %s  %ld\n"     },

   { IFSV_LFS_TIMER,             NCSFL_TYPE_TIL,      "[%s] %s  Timer ID %ld\n"     },

   { IFSV_LFS_SYS_CALL_FAIL,     NCSFL_TYPE_TIL,      "[%s] %s (ID - %ld)\n"     },
   
   { IFSV_LFS_HEADLINE,          NCSFL_TYPE_TILL,     "[%s] %s  (Info - %ld,%ld)\n"    },

   { IFSV_LFS_ADD_MOD_REC,       NCSFL_TYPE_TICLLLLL, "[%s] %s  Route Add Mod (%s) Attr - %ld MTU - %ld if speed - %ld admin status - %ld oper status %ld\n"    },

   { IFSV_LFS_DEL_REC,           NCSFL_TYPE_TICL,     "[%s] %s  Route Del (%s) ifindex - %ld\n"    },

   { IFSV_LFS_IF_STAT_INFO,      NCSFL_TYPE_TICLLLLL, "[%s] %s  Intf statistics (%s) status - %ld in_octs - %ld in_dscrds - %ld out_octs - %ld out_dscrds - %ld\n"    },

   { IFSV_LFS_FUNC_ENTRY_L,      NCSFL_TYPE_TIL,      "[%s] %s  Arg : %ld \n"    },
   { IFSV_LFS_FUNC_ENTRY_LL,     NCSFL_TYPE_TILL,     "[%s] %s  Arg : %ld, %ld\n"    },
   { IFSV_LFS_FUNC_ENTRY_LLL,    NCSFL_TYPE_TILLL,    "[%s] %s  Arg : %ld, %ld %ld\n" },
   { IFSV_LFS_FUNC_ENTRY_LLLL,   NCSFL_TYPE_TILLLL,   "[%s] %s  Arg : %ld, %ld %ld %ld\n" },
   { IFSV_LFS_FUNC_ENTRY_LLLLL,  NCSFL_TYPE_TILLLLL,  "[%s] %s  Arg : %ld, %ld %ld %ld %ld\n" },
   { IFSV_LFS_FUNC_ENTRY_LLLLLL, NCSFL_TYPE_TILLLLLL, "[%s] %s  Arg : %ld, %ld %ld %ld %ld %ld\n" },


   { IFSV_LFS_API_L,             NCSFL_TYPE_TIL,      "[%s] %s  Arg : %ld \n"   },
   { IFSV_LFS_API_LL,            NCSFL_TYPE_TILL,     "[%s] %s  Arg : %ld %ld\n" },
   { IFSV_LFS_API_LLL,           NCSFL_TYPE_TILLL,    "[%s] %s  Arg : %ld %ld %ld\n"   },
   { IFSV_LFS_API_LLLL,          NCSFL_TYPE_TILLLL,   "[%s] %s  Arg : %ld %ld %ld %ld\n" },
   { IFSV_LFS_API_LLLLL,         NCSFL_TYPE_TILLLLL,  "[%s] %s  Arg : %ld %ld %ld %ld %ld\n" },
   { IFSV_VIP_FLC_API,           NCSFL_TYPE_TI,       "[%s] %s \n"},
   { IFSV_LFS_LOG_STR_2_ERROR,   NCSFL_TYPE_TCIC,     "[%s] %s %s %s \n"},
   { IFSV_LFS_LOG_STR_ERROR,     NCSFL_TYPE_TICL,     "[%s] %s %s %ld \n"},
   { IFSV_LFS_REQ_ORIG_INFO,     NCSFL_TYPE_TICLLLLLLL,  "[%s] %s %s %ld %ld %ld %ld %ld %ld %ld  \n"},
   { 0, 0, 0 }
};

/*****************************************************************************
 * Ascii spec of IfA.
 *****************************************************************************/
NCSFL_ASCII_SPEC ifa_ascii_spec =
  {
    NCS_SERVICE_ID_IFA,             /* service ID */
    IFSV_LOG_VERSION,               /* Revision ID */
    "IfA",
    (NCSFL_SET*)  ifsv_log_str_set,  /* log string set */
    (NCSFL_FMAT*) ifsv_log_fmat_set, /* log format set */
    0,                              /* Place holder for str_set count */
    0                               /* Place holder for fmat_set count */
  };

/*****************************************************************************
 * Ascii spec of IfND.
 *****************************************************************************/
NCSFL_ASCII_SPEC ifnd_ascii_spec = 
  {
    NCS_SERVICE_ID_IFND,             /* service ID */
    IFSV_LOG_VERSION,                /* Revision ID */
    "IfND",
    (NCSFL_SET*)  ifsv_log_str_set,  /* log string set */
    (NCSFL_FMAT*) ifsv_log_fmat_set, /* log format set */
    0,                              /* Place holder for str_set count */
    0                               /* Place holder for fmat_set count */
  };

/*****************************************************************************
 * Ascii spec of IfD.
 *****************************************************************************/
NCSFL_ASCII_SPEC ifd_ascii_spec = 
  {
    NCS_SERVICE_ID_IFD,              /* service ID */
    IFSV_LOG_VERSION,                /* Revision ID */
    "IfD",
    (NCSFL_SET*)  ifsv_log_str_set,  /* log string set */
    (NCSFL_FMAT*) ifsv_log_fmat_set, /* log format set */
    0,                              /* Place holder for str_set count */
    0                               /* Place holder for fmat_set count */
  };


/****************************************************************************
 * Name          : ifsv_flx_log_ascii_set_reg
 *
 * Description   : This is the function which registers the IfSv logging ascci
 *                 set with the FLEX Log server.
 *                 
 *
 * Arguments     : None. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void ifsv_flx_log_ascii_set_reg(void)
{
   NCS_DTSV_REG_CANNED_STR arg;

   memset(&arg,0,sizeof(NCS_DTSV_REG_CANNED_STR));

   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
   arg.info.reg_ascii_spec.spec = &ifd_ascii_spec;

   ncs_dtsv_ascii_spec_api(&arg);

   arg.info.reg_ascii_spec.spec = &ifnd_ascii_spec; 

   ncs_dtsv_ascii_spec_api(&arg);

   arg.info.reg_ascii_spec.spec = &ifa_ascii_spec;

   ncs_dtsv_ascii_spec_api(&arg);
   return;
}

/****************************************************************************
 * Name          : ifsv_flx_log_ascii_set_dereg
 *
 * Description   : This is the function which deregisters the IfSv logging
 *                 ascii set with the FLEX Log server.
 *                 
 *
 * Arguments     : None. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void ifsv_flx_log_ascii_set_dereg(void)
{
   NCS_DTSV_REG_CANNED_STR arg;   
   
   memset(&arg,0,sizeof(NCS_DTSV_REG_CANNED_STR));   

   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;

   arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_IFA;
   arg.info.dereg_ascii_spec.version = IFSV_LOG_VERSION;
  
   ncs_dtsv_ascii_spec_api(&arg); 

   arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_IFND;

   ncs_dtsv_ascii_spec_api(&arg);

   arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_IFD;

   ncs_dtsv_ascii_spec_api(&arg);
   return;
}

/****************************************************************************
 * Name          : ifsv_log_str_lib_req
 *
 * Description   : Library entry point for ifsv log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which SBOM gives.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         ifsv_flx_log_ascii_set_reg();
         break;

      case NCS_LIB_REQ_DESTROY:
         ifsv_flx_log_ascii_set_dereg();
         break;

      default:
         break;
   }
   return NCSCC_RC_SUCCESS;
}
  
