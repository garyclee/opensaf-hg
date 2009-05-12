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

#ifndef HCD_LOG_H
#define HCD_LOG_H


/******************************************************************************
 Logging offset indexes for Headline logging
 ******************************************************************************/
typedef enum hcd_hdln_flex
{
   HCD_CREATE_HANDLE_FAILED,
   HCD_TAKE_HANDLE_FAILED,
   HCD_IPC_TASK_INIT,
   HCD_IPC_SEND_FAIL,
   HCD_UNKNOWN_EVT_RCVD,
   HCD_EVT_PROC_FAILED,
   HCD_UNKNOWN_HAM_EVT,
   HCD_HEALTH_KEY_DEFAULT_SET
} HCD_HDLN_FLEX;

/******************************************************************************
 Logging offset indexes for AMF logging
 ******************************************************************************/
typedef enum hcd_svc_prvdr_flex
{
   HCD_AMF_READINESS_CB_OUT_OF_SERVICE,
   HCD_AMF_READINESS_CB_IN_SERVICE,
   HCD_AMF_READINESS_CB_STOPPING,
   HCD_AMF_CSI_SET_HA_STATE_ACTIVE,
   HCD_AMF_CSI_SET_HA_STATE_STANDBY,
   HCD_AMF_CSI_SET_HA_STATE_QUIESCED,
   HCD_AMF_RCVD_CONFIRM_CALL_BACK,
   HCD_AMF_RCVD_HEALTHCHK,
   HCD_AMF_INIT_SUCCESS,
   HCD_AMF_INIT_ERROR,
   HCD_AMF_REG_ERROR,
   HCD_AMF_SEL_OBJ_GET_ERROR,
   HCD_AMF_DISPATCH_ERROR,
   HCD_AMF_HLTH_CHK_START_DONE,
   HCD_AMF_HLTH_CHK_START_FAIL,
   HCD_MDS_INSTALL_FAIL,
   HCD_MDS_UNINSTALL_FAIL,
   HCD_MDS_SUBSCRIBE_FAIL,
   HCD_MDS_SEND_ERROR,
   HCD_MDS_CALL_BACK_ERROR,
   HCD_MDS_UNKNOWN_SVC_EVT,
   HCD_MDS_VDEST_DESTROY_FAIL
} HCD_SVC_PRVDR_FLEX;

/******************************************************************************
 Logging offset indexes for lock operations logging
 ******************************************************************************/
typedef enum hcd_lck_oper_flex
{
   HCD_OPER_RSC_OPER_ERROR,
   HCD_OPER_RSC_ID_ALLOC_FAIL,
   HCD_OPER_MASTER_RENAME_ERR,
   HCD_OPER_SET_ORPHAN_ERR,
   HCD_OPER_INCORRECT_STATE
} HCD_LCK_OPER_FLEX;

/******************************************************************************
 Logging offset indexes for Memory Fail logging
 ******************************************************************************/
typedef enum hcd_memfail_flex
{
   HCD_CB_ALLOC_FAILED,
   HCD_NODE_DETAILS_ALLOC_FAILED,
   HCD_EVT_ALLOC_FAILED,
   HCD_RSC_INFO_ALLOC_FAILED
} HCD_MEMFAIL_FLEX;

/******************************************************************************
 Logging offset indexes for API logging
 ******************************************************************************/
typedef enum hcd_api_flex
{
   HCD_SE_API_CREATE_SUCCESS,
   HCD_SE_API_CREATE_FAILED,
   HCD_SE_API_DESTROY_SUCCESS,
   HCD_SE_API_DESTROY_FAILED,
   HCD_SE_API_UNKNOWN

} HCD_API_FLEX;

/******************************************************************************
 Logging offset indexes for firmware progress logging
 ******************************************************************************/
typedef enum hcd_fwprog_flex
{
   /* Firmware progress event logging messages */
   HCD_SE_FWPROG_BOOT_SUCCESS,
   HCD_SE_FWPROG_NODE_INIT_SUCCESS,
} HCD_FWPROG_FLEX;

/******************************************************************************
 Logging offset indexes for firmware error logging
 ******************************************************************************/
typedef enum hcd_fwerr_flex
{
   /* Firmware error event logging messages */
   HCD_SE_FWERR_HPM_INIT_FAILED,
   HCD_SE_FWERR_HLFM_INIT_FAILED,
   HCD_SE_FWERR_SWITCH_INIT_FAILED,
   HCD_SE_FWERR_LHC_DMN_INIT_FAILED,
   HCD_SE_FWERR_LHC_RSP_INIT_FAILED,
   HCD_SE_FWERR_NW_SCRIPT_INIT_FAILED,
   HCD_SE_FWERR_DRBD_INIT_FAILED,
   HCD_SE_FWERR_TIPC_INIT_FAILED,
   HCD_SE_FWERR_IFSD_INIT_FAILED,
   HCD_SE_FWERR_DTSV_INIT_FAILED,
   HCD_SE_FWERR_MASV_INIT_FAILED,
   HCD_SE_FWERR_PSSV_INIT_FAILED,
   HCD_SE_FWERR_GLND_INIT_FAILED,
   HCD_SE_FWERR_EDSV_INIT_FAILED,
   HCD_SE_FWERR_SUBAGT_INIT_FAILED,
   HCD_SE_FWERR_SNMPD_INIT_FAILED,
   HCD_SE_FWERR_NCS_INIT_FAILED,
   HCD_SE_FWERR_UNKNOWN_EVT_FAILED

} HCD_FWERR_FLEX;

/******************************************************************************
 Logging offset indexes for Event logging
 ******************************************************************************/
typedef enum hcd_evt_flex
{
   HCD_EVT_NULL_STR,
   HCD_EVT_RSC_CLOSE,
   HCD_EVT_SET_ORPHAN,
   HCD_EVT_MDS_HAM_UP,
   HCD_EVT_MDS_HAM_DOWN,

} HCD_EVT_FLEX;

/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum hcd_flex_sets
{
   HCD_FC_HDLN,
   HCD_FC_MEMFAIL,
   HCD_FC_API,
   HCD_FC_FWPROG,
   HCD_FC_FWERR,
   HCD_FC_EVT,
   HCD_FC_SVC_PRVDR,
   HCD_FC_LCK_OPER,
   HCD_FC_HOTSWAP
}HCD_FLEX_SETS;

typedef enum hcd_log_ids
{
   HCD_LID_HDLN,
   HCD_LID_MEMFAIL,
   HCD_LID_API,
   HCD_LID_FWPROG,
   HCD_LID_FWERR,
   HCD_LID_EVT,
   HCD_LID_SVC_PRVDR,
   HCD_LID_LCK_OPER,
   HCD_LID_GEN_INFO,
   HCD_LID_GEN_INFO2,
   HCD_LID_GEN_INFO3,
   HCD_LID_GEN_INFO4
} HCD_LOG_IDS;

typedef enum hcd_evt_hswap
{
   HCD_HOTSWAP_NULL_STR,
   HCD_HOTSWAP_CURR_STATE,
   HCD_HOTSWAP_PREV_STATE,
   HCD_NON_HOTSWAP_TYPE
} HCD_EVT_HSWAP;

/* DTSv version support */
#define HISV_LOG_VERSION 2

#if (NCS_HISV_LOG == 1)
void hisv_flx_log_reg(void);
void hisv_flx_log_dereg(void);
EXTERN_C uns32 hisv_log_bind(void);
EXTERN_C uns32 hisv_log_unbind(void);

EXTERN_C void hisv_log_headline(uns8 hdln_id, uns8 sev);
EXTERN_C void hisv_log_memfail(uns8 mf_id);
EXTERN_C void hisv_log_api(uns8 api_id, uns8 sev);
EXTERN_C void hisv_log_fwprog(uns8 fwp_id, uns8 sev, uns32 num);
EXTERN_C void hisv_log_fwerr(uns8 fwe_id, uns8 sev, uns32 num);
EXTERN_C void hisv_log_evt(uns8 evt_id, uns32 rsc_id, uns32 node);
EXTERN_C void hisv_log_svc_prvdr(uns8 sp_id, uns8 sev);
EXTERN_C void hisv_log_lck_oper(uns8 lck_id, uns8 sev ,
                               char *rsc_name, uns32 rsc_id,
                               uns32 node);

#define m_LOG_HISV_HEADLINE(id, sev)        hisv_log_headline(id,sev)
#define m_LOG_HISV_MEMFAIL(id)              hisv_log_memfail(id)
#define m_LOG_HISV_API(id,sev)              hisv_log_api(id,sev)
#define m_LOG_HISV_FWPROG(id,sev,num)       hisv_log_fwprog(id, sev, num)
#define m_LOG_HISV_FWERR(id,sev,num)        hisv_log_fwerr(id, sev, num)

#define m_LOG_HISV_DEBUG(str)               ncs_logmsg(NCS_SERVICE_ID_HCD, \
                                            HCD_LID_GEN_INFO, HCD_FC_EVT, \
                                            NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSFL_TYPE_TC, str)

#define m_LOG_HISV_DEBUG_VAL(str, val)      ncs_logmsg(NCS_SERVICE_ID_HCD, \
                                            HCD_LID_GEN_INFO4, HCD_FC_HOTSWAP, \
                                            NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, NCSFL_TYPE_TILL, str, val)

#define m_LOG_HISV_DTS_CONS(str)   \
{ \
   m_LOG_HISV_DEBUG(str); \
   printf(str); \
}

#define m_LOG_HISV_GEN(str, addrs, length, severity) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = length;\
   nam_val.dump = (char *)addrs; \
   ncs_logmsg(NCS_SERVICE_ID_HCD, HCD_LID_GEN_INFO2, HCD_FC_EVT, NCSFL_LC_HEADLINE, severity, "TCP", str, nam_val);\
}
#define m_LOG_HISV_GEN_STR(str, addrs, severity) \
{\
   ncs_logmsg(NCS_SERVICE_ID_HCD, HCD_LID_GEN_INFO3, HCD_FC_EVT, NCSFL_LC_HEADLINE, severity, "TCC", str, addrs);\
}
#define m_LOG_HISV_EVT(id,rsc_id,node)      hisv_log_evt(id,rsc_id,node)
#define m_LOG_HISV_SVC_PRVDR(id, sev)       hisv_log_svc_prvdr(id,sev)
#define m_LOG_HISV_LCK_OPER(id, sev, name, rsc_id, node) hisv_log_lck_oper(id, sev, \
                                                        name, rsc_id,node)
#else
#define m_LOG_HISV_HEADLINE(id, sev)         ;
#define m_LOG_HISV_MEMFAIL(id)               ;
#define m_LOG_HISV_API(id,sev)               ;
#define m_LOG_HISV_FWPROG(id,sev,num)        ;
#define m_LOG_HISV_FWERR(id,sev,num)         ;
#define m_LOG_HISV_DEBUG(str)                ;
#define m_LOG_HISV_EVT(id,rsc_id,node)       ;
#define m_LOG_HISV_SVC_PRVDR(id, sev)        ;
#define m_LOG_HISV_LCK_OPER(id, sev, name, rsc_id, node)    ;

#endif  /* NCS_HISV_LOG == 1*/

#endif
