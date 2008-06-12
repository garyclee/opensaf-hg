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

  DESCRIPTION:

  This module contains logging/tracing functions.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef MAB_LOG_H
#define MAB_LOG_H

#include "mab_log_const.h"

EXTERN_C MABCOM_API uns32 mab_log_bind(void); 
EXTERN_C MABCOM_API uns32 mab_log_unbind(void); 

#if (MAB_LOG == 1)
struct mas_fltr;
EXTERN_C MABCOM_API uns32 log_mab_svc_prvdr_evt(uns8, uns32, uns32, MDS_DEST, uns32, uns32); 
EXTERN_C MABCOM_API uns32 log_mab_svc_prvdr_msg(uns8, uns32, uns32, MDS_DEST, uns32); 
EXTERN_C MABCOM_API uns32 log_mab_fltr_data(uns8, uns32,uns32,uns32, NCSMAB_FLTR *); 
EXTERN_C MABCOM_API uns32 log_overlapping_fltrs(uns8,uns32,uns32,struct mas_fltr*,struct mas_fltr*); 

#define m_LOG_MAB_HEADLINE(s, id) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_HDLN, MAB_FC_HDLN,\
                   NCSFL_LC_HEADLINE, s, NCSFL_TYPE_TI, id) 

#define m_LOG_MAB_HDLN_I(s, id, i1) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_HDLN_I, MAB_FC_HDLN,\
                   NCSFL_LC_HEADLINE, s, NCSFL_TYPE_TIL, id, i1) 

#define m_LOG_MAB_HDLN_II(s, id, i1, i2) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_HDLN_II, MAB_FC_HDLN,\
                   NCSFL_LC_HEADLINE, s, NCSFL_TYPE_TILL, id, i1, i2) 

#define m_LOG_MAB_SVC_PRVDR_EVT(s, id, svc_id, dest, new_state, anchor) \
        log_mab_svc_prvdr_evt(s, id, svc_id, dest, new_state, anchor)
        
#define m_LOG_MAB_SVC_PRVDR_MSG(s, id, svc_id, dest, msg_type) \
        log_mab_svc_prvdr_msg(s, id, svc_id, dest, msg_type)

#define m_LOG_MAB_SVC_PRVDR(s, id, i1, i2) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_SVC_PRVDR_FLEX, MAB_FC_SVC_PRVDR_FLEX,\
                   NCSFL_LC_SVC_PRVDR, s, NCSFL_TYPE_TILL, id, i1, i2)

#define m_LOG_MAB_LOCK(id, lck)\
         ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_LOCKS, MAB_FC_LOCKS, \
                    NCSFL_LC_MISC, NCSFL_SEV_DEBUG,\
                    NCSFL_TYPE_TIL, id, lck)

#define m_LOG_MAB_API(id) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_API, MAB_FC_API,\
                   NCSFL_LC_API, NCSFL_SEV_DEBUG, NCSFL_TYPE_TI, id)

#define m_LOG_MAB_EVT(s, id)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_EVT, MAB_FC_EVT, \
                   NCSFL_LC_EVENT, s, NCSFL_TYPE_TI, id)

#define m_LOG_MAB_TBL_DETAILS(c, s, id, env_id, tbl_id) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_TBL_DET, MAB_FC_TBL_DET,\
                   c, s, NCSFL_TYPE_TILL, id, env_id, tbl_id)

#define m_LOG_MAB_FLTR_DETAILS(c, s, id, tbl_id, fltr_id, fltr_type) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_FLTR_DET, MAB_FC_FLTR_DET,\
                   c, s, NCSFL_TYPE_TILLL, id, tbl_id, fltr_id, fltr_type)


#define m_LOG_MAB_FLTR_DATA(sev, env_id, tbl_id, fltr_id, fltr_data_ptr) \
        log_mab_fltr_data(sev, env_id, tbl_id, fltr_id, fltr_data_ptr)
        
#define m_LOG_MAB_OVERLAPPING_FLTRS(sev, env_id, tbl_id, exst_fltr, new_fltr) \
        log_overlapping_fltrs(sev, env_id, tbl_id, exst_fltr, new_fltr)

#if 0
/* not used any where */ 
#define m_LOG_MAB_OAA_DETAILS(c, s, id, addr, anc)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_OAA, MAB_FC_HDLN,\
                   c, s, NCSFL_TYPE_TICLL, id, addr, anc)
#endif
        
#define m_LOG_MAB_NO_CB(func_name)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_NO_CB, MAB_FC_HDLN,\
                   NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSFL_TYPE_TIC,\
                   MAB_HDLN_NO_CB, func_name)

#define m_LOG_MAB_CSI_DETAILS(s, id, flags, compName, csiName, state)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_CSI, \
                   MAB_FC_HDLN, NCSFL_LC_EVENT, s, \
                   "TILCCL", id, flags, compName, csiName, state)

#define m_LOG_MAB_STATE_CHG(s, id, cur_state, new_state) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_ST_CHG, \
                   MAB_FC_HDLN, NCSFL_LC_STATE, s, \
                   "TILL", id, cur_state, new_state)

#define m_LOG_MAB_MEMFAIL(s, id, func_name)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_MEMFAIL, \
                   MAB_FC_MEMFAIL, NCSFL_LC_MEMORY, s, \
                   "TIC", id, func_name)

#define m_LOG_MAB_MEM(s, id, inst_len, mem)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_MEM, \
                   MAB_FC_HDLN, NCSFL_LC_MEMORY, s, \
                   "TILD", id, inst_len, mem)

#define m_LOG_MAB_ERROR_NO_DATA(s, c, id)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_ERROR, \
                   MAB_FC_ERROR, c, s, \
                   "TI", id)

#define m_LOG_MAB_ERROR_DATA(s, c, id, i1)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_ERR_I, \
                   MAB_FC_ERROR, c, s, \
                   "TIL", id, i1)

#define m_LOG_OAA_BIND_EVT(pcn, tbl_id) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_OAA_BIND_EVT, \
           MAB_FC_EVT, NCSFL_LC_EVENT, \
           NCSFL_SEV_INFO, "TCL", pcn, tbl_id)

#define m_LOG_OAA_UNBIND_EVT(tbl_id) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_OAA_UNBIND_EVT, \
           MAB_FC_EVT, NCSFL_LC_EVENT, \
           NCSFL_SEV_INFO, "TL", tbl_id)

#define m_LOG_OAA_PCN_INFO_I(s, id, tbl_id)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_OAA_PCN_INFO_I, \
                   MAB_FC_OAA_PCN_INFO_I, NCSFL_LC_EVENT, s, \
                   "TIL", id, tbl_id)

#define m_LOG_OAA_PCN_INFO_II(s, id, pcn, tbl_id)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_OAA_PCN_INFO_II, \
                   MAB_FC_OAA_PCN_INFO_II, NCSFL_LC_EVENT, s, \
                   "TICL", id, pcn, tbl_id)

#define m_LOG_OAA_WARMBOOTREQ_INFO_I(s, id, psr_here, mas_here)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_OAA_WARMBOOTREQ_INFO_I, \
                   MAB_FC_OAA_WARMBOOTREQ_INFO_I, NCSFL_LC_EVENT, s, \
                   "TILL", id, psr_here, mas_here)

#define m_LOG_OAA_WARMBOOTREQ_INFO_II(s, id, pcn, is_sys_client, tbl_id)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_OAA_WARMBOOTREQ_INFO_II, \
                   MAB_FC_OAA_WARMBOOTREQ_INFO_II, NCSFL_LC_EVENT, s, \
                   "TICLL", id, pcn, is_sys_client, tbl_id)

#define m_LOG_OAA_WARMBOOTREQ_INFO_III(s, id, pcn, is_sys_client, tbl_id, wbreq_hdl)\
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_OAA_WARMBOOTREQ_INFO_III, \
                   MAB_FC_OAA_WARMBOOTREQ_INFO_III, NCSFL_LC_EVENT, s, \
                   "TICLLL", id, pcn, is_sys_client, tbl_id, wbreq_hdl)

#define m_LOG_MAB_ERROR_I(s, id, i1) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_ERR_I, \
                   MAB_FC_ERROR, NCSFL_LC_HEADLINE, s, \
                   NCSFL_TYPE_TIL, id, i1)

#define m_LOG_MAB_ERROR_II(c, s, id, i1, i2) \
        ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_ERR_II, \
                   MAB_FC_ERROR, c, s, \
                   NCSFL_TYPE_TILL, id, i1, i2)
 
 
#else

#define m_LOG_MAB_HEADLINE(f, id)  
#define m_LOG_MAB_HDLN_I(s, id, i1)
#define m_LOG_MAB_HDLN_II(s, id, i1, i2) 
#define m_LOG_MAB_SVC_PRVDR(s, id, i1, i2)
#define m_LOG_MAB_LOCK(id, lck)        
#define m_LOG_MAB_MEM(s, id, inst_len, mem)
#define m_LOG_MAB_API(id)        
#define m_LOG_MAB_EVT(s, id)           
#define m_LOG_MAB_TBL_DETAILS(c, s, id, env_id, tbl_id) 
#define m_LOG_MAB_FLTR_DETAILS(c, s, id, tbl_id, fltr_id, fltr_type)
#define m_LOG_MAB_FLTR_DATA(sev, env_id, tbl_id, fltr_id, fltr_data)
#define m_LOG_MAB_OVERLAPPING_FLTRS(sev, env_id, tbl_id, exst_fltr, new_fltr) 
#define m_LOG_MAB_SVC_PRVDR_EVT(s, id, svc_id, dest, new_state, anchor) 
#define m_LOG_MAB_SVC_PRVDR_MSG(s, id, svc_id, dest, msg_type)
#define m_LOG_MAB_NO_CB(func_name)
#define m_LOG_MAB_ERROR_II(c, s, id, i1, i2)
#define m_LOG_MAB_ERROR_I(s, id, i1) 
#define m_LOG_MAB_CSI_DETAILS(s, id, flags, compName, csiName, state)
#define m_LOG_MAB_STATE_CHG(s, id, cur_state, new_state)
#define m_LOG_MAB_MEMFAIL(s, id, func_name)
#define m_LOG_MAB_API(id)
#define m_LOG_MAB_MEM(s, id, inst_len, mem)
#define m_LOG_MAB_ERROR_NO_DATA(s, c, id)
#define m_LOG_MAB_ERROR_DATA(s, c, id, i1)
#define m_LOG_OAA_BIND_EVT(pcn, tbl_id)
#define m_LOG_OAA_UNBIND_EVT(tbl_id)
#define m_LOG_OAA_PCN_INFO_I(s, id, pcn)
#define m_LOG_OAA_PCN_INFO_II(s, id, pcn, tbl_id)
#define m_LOG_OAA_WARMBOOTREQ_INFO_I(s, id, psr_here, mas_here)
#define m_LOG_OAA_WARMBOOTREQ_INFO_II(s, id, pcn, is_sys_client, tbl_id)
#define m_LOG_OAA_WARMBOOTREQ_INFO_III(s, id, pcn, is_sys_client, tbl_id, wbreq_hdl)
#endif /* MAB_LOG == 1 */

#endif /* MAB_LOG_H */
