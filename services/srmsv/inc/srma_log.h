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

  MODULE NAME: SRMA_LOG.H

$Header: 
..............................................................................

  DESCRIPTION: Defines structures and defn of SRMSV log file

******************************************************************************/

#ifndef SRMA_LOG_H
#define SRMA_LOG_H

#if (NCS_SRMA_LOG == 1)

/******************************************************************************
                    Logging offset indices for Memory
******************************************************************************/
typedef enum srma_log_mem
{
   SRMA_MEM_SRMND_INFO,
   SRMA_MEM_SRMA_CB,
   SRMA_MEM_SRMND_APPL,
   SRMA_MEM_CBK_INFO,
   SRMA_MEM_CBK_REC,
   SRMA_MEM_APPL_INFO,
   SRMA_MEM_RSRC_INFO,
   SRMA_MEM_ALLOC_SUCCESS,
   SRMA_MEM_ALLOC_FAILED,
   SRMA_MEM_MAX
} SRMA_LOG_MEM;

/******************************************************************************
                    Logging offset indices for selection object
******************************************************************************/
typedef enum srma_log_sel_obj {
   SRMA_LOG_SEL_OBJ_CREATE,
   SRMA_LOG_SEL_OBJ_DESTROY,
   SRMA_LOG_SEL_OBJ_IND_SEND,
   SRMA_LOG_SEL_OBJ_IND_REM,
   SRMA_LOG_SEL_OBJ_SUCCESS,
   SRMA_LOG_SEL_OBJ_FAILURE,
   SRMA_LOG_SEL_OBJ_MAX
} SRMA_LOG_SEL_OBJ;

/******************************************************************************
                       Logging offset indices for AMF APIs
 ******************************************************************************/
typedef enum srma_log_api {
   SRMA_LOG_API_FINALIZE,
   SRMA_LOG_API_START_MON,
   SRMA_LOG_API_STOP_MON,
   SRMA_LOG_API_START_RSRC_MON,
   SRMA_LOG_API_STOP_RSRC_MON,
   SRMA_LOG_API_SUBSCR_RSRC_MON,
   SRMA_LOG_API_UNSUBSCR_RSRC_MON,
   SRMA_LOG_API_INITIALIZE,
   SRMA_LOG_API_SEL_OBJ_GET,
   SRMA_LOG_API_DISPATCH,
   SRMA_LOG_API_GET_WATERMARK_VAL,
   SRMA_LOG_API_FUNC_ENTRY,
   SRMA_LOG_API_ERR_SA_OK,
   SRMA_LOG_API_ERR_SA_LIBRARY,
   SRMA_LOG_API_ERR_SA_INVALID_PARAM,
   SRMA_LOG_API_ERR_SA_NO_MEMORY,
   SRMA_LOG_API_ERR_SA_BAD_HANDLE,
   SRMA_LOG_API_ERR_SA_NOT_EXIST,
   SRMA_LOG_API_ERR_SA_BAD_OPERATION,
   SRMA_LOG_API_ERR_SA_BAD_FLAGS,
   SRMA_LOG_API_MAX
} SRMA_LOG_API;


/******************************************************************************
                Logging offset indices for Miscelleneous stuff
******************************************************************************/
typedef enum srma_log_misc {
   SRMA_LOG_MISC_SRMND_UP,
   SRMA_LOG_MISC_SRMND_DN,
   SRMA_LOG_MISC_VALIDATE_RSRC,
   SRMA_LOG_MISC_VALIDATE_MON,
   SRMA_LOG_MISC_DUP_RSRC_MON,
   SRMA_LOG_MISC_AGENT_CREATE,
   SRMA_LOG_MISC_AGENT_DESTROY,
   SRMA_LOG_MISC_INVALID_MSG_TYPE,
   SRMA_LOG_MISC_ALREADY_STOP_MON,
   SRMA_LOG_MISC_ALREADY_START_MON,
   SRMA_LOG_MISC_RSRC_SRMND_MISSING,
   SRMA_LOG_MISC_RSRC_USER_MISSING,
   SRMA_LOG_MISC_DATA_INCONSISTENT,
   SRMA_LOG_MISC_INVALID_RSRC_TYPE,
   SRMA_LOG_MISC_INVALID_PID,
   SRMA_LOG_MISC_INVALID_CHILD_LEVEL,
   SRMA_LOG_MISC_PID_NOT_EXIST,
   SRMA_LOG_MISC_PID_NOT_LOCAL,
   SRMA_LOG_MISC_INVALID_USER_TYPE,
   SRMA_LOG_MISC_INVALID_NODE_ID,
   SRMA_LOG_MISC_INVALID_VAL_TYPE,
   SRMA_LOG_MISC_INVALID_SCALE_TYPE,
   SRMA_LOG_MISC_INVALID_CONDITION,
   SRMA_LOG_MISC_INVALID_WM_TYPE,
   SRMA_LOG_MISC_INVALID_MON_TYPE,
   SRMA_LOG_MISC_INVALID_LOC_TYPE,
   SRMA_LOG_MISC_INVALID_TOLVAL_SCALE_TYPE,
   SRMA_LOG_MISC_INVALID_TOLVAL_VALUE_TYPE,
   SRMA_LOG_MISC_SUCCESS,
   SRMA_LOG_MISC_FAILED,
   SRMA_LOG_MISC_NOTHING,
   SRMA_LOG_MISC_MAX
} SRMA_LOG_MISC;

#endif  /* (NCS_SRMA_LOG == 1) */

/******************************************************************************
    Logging offset indices for canned constant strings for the ASCII SPEC
******************************************************************************/
typedef enum srma_flex_sets {
   SRMA_FC_SEAPI,
   SRMA_FC_MDS,
   SRMA_FC_EDU,
   SRMA_FC_LOCK,
   SRMA_FC_CB,
   SRMA_FC_RSRC_MON,
   SRMA_FC_HDL,
   SRMA_FC_SRMSV_MEM,
   SRMA_FC_TMR,
   SRMA_FC_MEM,
   SRMA_FC_SEL_OBJ,
   SRMA_FC_API,
   SRMA_FC_MISC,   
} SRMA_FLEX_SETS;

typedef enum srma_log_ids {
   SRMA_LID_SEAPI,
   SRMA_LID_MDS,
   SRMA_LID_EDU,
   SRMA_LID_LOCK,
   SRMA_LID_CB,
   SRMA_LID_RSRC_MON,
   SRMA_LID_HDL,
   SRMA_LID_SRMSV_MEM,
   SRMA_LID_TMR,
   SRMA_LID_MEM,
   SRMA_LID_SEL_OBJ,
   SRMA_LID_API,
   SRMA_LID_MISC,   
   SRMA_LID_MSG_FMT,
} SRMA_LOG_IDS;

/*****************************************************************************
                          Macros for Logging
*****************************************************************************/
#if (NCS_SRMA_LOG == 1)
#define m_SRMA_LOG_SEAPI(op, st, sev)    ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_SEAPI, \
                                                    SRMA_FC_SEAPI, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_MEM(op, st, sev)      ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_MEM, \
                                                    SRMA_FC_MEM, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_SRMSV_MEM(op, st, sev)  ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_SRMSV_MEM, \
                                                    SRMA_FC_SRMSV_MEM, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_MDS(op, st, sev)      ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_MDS, \
                                                    SRMA_FC_MDS, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_EDU(op, st, sev)      ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_EDU, \
                                                    SRMA_FC_EDU, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_LOCK(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_LOCK, \
                                                    SRMA_FC_LOCK, \
                                                    NCSFL_LC_LOCKS, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_CB(op, st, sev)       ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_CB, \
                                                    SRMA_FC_CB, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_SEL_OBJ(op, st, sev)   ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_SEL_OBJ, \
                                                    SRMA_FC_SEL_OBJ, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_API(op, st, sev)       ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_API, \
                                                    SRMA_FC_API, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_RSRC_MON(op, st, sev)  ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_RSRC_MON, \
                                                    SRMA_FC_RSRC_MON, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_HDL(op, st, sev)      ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_HDL, \
                                                    SRMA_FC_HDL, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_TMR(op, st, sev)      ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_TMR, \
                                                    SRMA_FC_TMR, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMA_LOG_MISC(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMA, \
                                                    SRMA_LID_MISC, \
                                                    SRMA_FC_MISC, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)
#define m_SRMA_LOG_MDS_VER(var1,var2,var3,var4,var5) ncs_logmsg(NCS_SERVICE_ID_SRMA,\
                                                    SRMA_LID_MSG_FMT,\
                                                    0,\
                                                    NCSFL_LC_HEADLINE,\
                                                    NCSFL_SEV_NOTICE,\
                                                    "TCCLCL", \
                                                    var1,\
                                                    var2,var3,var4,var5)

#else /* NCS_SRMA_LOG == 1 */

#define m_SRMA_LOG_SEAPI(op, st, sev)
#define m_SRMA_LOG_MEM(op, st, sev)
#define m_SRMA_LOG_MDS(op, st, sev)
#define m_SRMA_LOG_EDU(op, st, sev)
#define m_SRMA_LOG_LOCK(op, st, sev)
#define m_SRMA_LOG_CB(op, st, sev)
#define m_SRMA_LOG_SEL_OBJ(op, st, sev)
#define m_SRMA_LOG_API(t, s, sev)
#define m_SRMA_LOG_RSRC_MON(op, st, sev)
#define m_SRMA_LOG_MISC(op, sev)
#define m_SRMA_LOG_MDS_VER(var1,var2,var3,var4,var5)
#endif /* NCS_SRMA_LOG == 1 */

#if (NCS_SRMA_LOG == 1)
EXTERN_C uns32 srma_log_unreg(void);
EXTERN_C uns32 srma_log_reg(void);
#endif

#endif /* SRMA_LOG_H */


