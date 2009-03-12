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

  MODULE NAME: SRMND_LOG.H

..............................................................................

  DESCRIPTION: Defines structures and defn of SRMSV log file

******************************************************************************/

#ifndef SRMND_LOG_H
#define SRMND_LOG_H

#if (NCS_SRMND_LOG == 1)
/******************************************************************************
                      Canned string for Monitoring
******************************************************************************/
typedef enum srmnd_log_mon
{
   SRMND_MON_RSRC_ADD,
   SRMND_MON_RSRC_DEL,
   SRMND_MON_RSRC_START,
   SRMND_MON_RSRC_STOP,
   SRMND_MON_APPL_START,
   SRMND_MON_APPL_STOP, 
   SRMND_MON_APPL_DEL,
   SRMND_MON_GET_WM,
   SRMND_MON_SUCCESS,
   SRMND_MON_FAILED,
   SRMND_MON_MAX
} SRMND_LOG_MON;

/******************************************************************************
                      Canned string for Memory
******************************************************************************/
typedef enum srmnd_log_mem 
{
   SRMND_MEM_USER_NODE,
   SRMND_MEM_SRMND_CB,
   SRMND_MEM_RSRC_TYPE,
   SRMND_MEM_SAMPLE_DATA,
   SRMND_MEM_PID_DATA,
   SRMND_MEM_EVENT, 
   SRMND_MEM_RSRC_INFO,
   SRMND_MEM_ALLOC_SUCCESS,
   SRMND_MEM_ALLOC_FAILED,
   SRMND_MEM_MAX
} SRMND_LOG_MEM;

typedef enum srmnd_log_amf 
{
   SRMND_LOG_AMF_CBK_INIT,
   SRMND_LOG_AMF_INITIALIZE,
   SRMND_LOG_AMF_GET_SEL_OBJ,
   SRMND_LOG_AMF_GET_COMP_NAME,
   SRMND_LOG_AMF_FINALIZE,
   SRMND_LOG_AMF_STOP_HLTCHECK,
   SRMND_LOG_AMF_START_HLTCHECK,
   SRMND_LOG_AMF_REG_COMP,
   SRMND_LOG_AMF_UNREG_COMP,
   SRMND_LOG_AMF_DISPATCH,
   SRMND_LOG_AMF_SUCCESS,
   SRMND_LOG_AMF_FAILED,
   SRMND_LOG_AMF_NOTHING,
   SRMND_LOG_AMF_MAX
} SRMND_LOG_AMF;

/******************************************************************************
                    Canned string for miscellaneous SRMND events
******************************************************************************/
typedef enum srmnd_log_misc
{
   SRMND_LOG_MISC_SRMA_UP,
   SRMND_LOG_MISC_SRMA_DN,
   SRMND_LOG_MISC_ND_CREATE,
   SRMND_LOG_MISC_ND_DESTROY,
   SRMND_LOG_MISC_GET_APPL,
   SRMND_LOG_MISC_DATA_INCONSISTENT,
   SRMND_LOG_MISC_SUCCESS,
   SRMND_LOG_MISC_FAILED,
   SRMND_LOG_MISC_NOTHING,
   SRMND_LOG_MISC_MAX
} SRMA_LOG_MISC;

#endif  /* (NCS_SRMND_LOG == 1) */

/******************************************************************************
    Logging offset indices for canned constant strings for the ASCII SPEC
******************************************************************************/
typedef enum srmnd_flex_sets {
   SRMND_FC_SEAPI,
   SRMND_FC_MDS,
   SRMND_FC_EDU,
   SRMND_FC_LOCK,
   SRMND_FC_CB,
   SRMND_FC_RSRC_MON,
   SRMND_FC_HDL,
   SRMND_FC_TIM,
   SRMND_FC_TMR,
   SRMND_FC_SRMSV_MEM,
   SRMND_FC_TREE,
   SRMND_FC_MEM,
   SRMND_FC_MON,
   SRMND_FC_AMF,
   SRMND_FC_MISC,
} SRMA_FLEX_SETS;

typedef enum srmnd_log_ids {
   SRMND_LID_SEAPI,
   SRMND_LID_MDS,
   SRMND_LID_EDU,
   SRMND_LID_LOCK,
   SRMND_LID_CB,
   SRMND_LID_RSRC_MON,
   SRMND_LID_HDL,
   SRMND_LID_TIM,
   SRMND_LID_TMR,
   SRMND_LID_SRMSV_MEM,
   SRMND_LID_TREE,
   SRMND_LID_MEM,
   SRMND_LID_MON,
   SRMND_LID_AMF,
   SRMND_LID_MISC,
   SRMND_LID_MSG_FMT,
} SRMND_LOG_IDS;

/*****************************************************************************
                          Macros for Logging
*****************************************************************************/
#if (NCS_SRMND_LOG == 1)
#define m_SRMND_LOG_SEAPI(op, st, sev)   ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_SEAPI, \
                                                    SRMND_FC_SEAPI, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_MEM(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_MEM, \
                                                    SRMND_FC_MEM, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_MON(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_MON, \
                                                    SRMND_FC_MON, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_SRMSV_MEM(op, st, sev)   ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_SRMSV_MEM, \
                                                    SRMND_FC_SRMSV_MEM, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_AMF(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_AMF, \
                                                    SRMND_FC_AMF, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_MDS(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_MDS, \
                                                    SRMND_FC_MDS, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_EDU(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_EDU, \
                                                    SRMND_FC_EDU, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_LOCK(op, st, sev)    ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_LOCK, \
                                                    SRMND_FC_LOCK, \
                                                    NCSFL_LC_LOCKS, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_CB(op, st, sev)      ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_CB, \
                                                    SRMND_FC_CB, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_RSRC_MON(op, st, sev) ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                     SRMND_LID_RSRC_MON, \
                                                     SRMND_FC_RSRC_MON, \
                                                     NCSFL_LC_HEADLINE, \
                                                     sev, \
                                                     NCSFL_TYPE_TII, \
                                                     op, \
                                                     st)

#define m_SRMND_LOG_MISC(op, st, sev)    ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_MISC, \
                                                    SRMND_FC_MISC, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_HDL(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_HDL, \
                                                    SRMND_FC_HDL, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_TIM(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_TIM, \
                                                    SRMND_FC_TIM, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_TMR(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_TMR, \
                                                    SRMND_FC_TMR, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_SRMND_LOG_TREE(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_SRMND, \
                                                    SRMND_LID_TREE, \
                                                    SRMND_FC_TREE, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)
#define m_SRMND_LOG_MDS_VER(var1,var2,var3,var4,var5) ncs_logmsg(NCS_SERVICE_ID_SRMND,\
                                                    SRMND_LID_MSG_FMT,\
                                                    0,\
                                                    NCSFL_LC_HEADLINE,\
                                                    NCSFL_SEV_NOTICE,\
                                                    "TCCLCL", \
                                                    var1,\
                                                    var2,var3,var4,var5)



#else /* NCS_SRMND_LOG == 1 */

#define m_SRMND_LOG_SEAPI(op, st, sev)
#define m_SRMND_LOG_MEM(op, st, sev)
#define m_SRMND_LOG_MDS(op, st, sev)
#define m_SRMND_LOG_EDU(op, st, sev)
#define m_SRMND_LOG_LOCK(op, st, sev)
#define m_SRMND_LOG_CB(op, st, sev)
#define m_SRMND_LOG_RSRC_MON(op, st, sev)
#define m_SRMND_LOG_MISC(op, sev)
#define m_SRMND_LOG_HDL(op, st, sev)
#define m_SRMND_LOG_TIM(op, st, sev)
#define m_SRMND_LOG_MDS_VER(var1,var2,var3,var4,var5)
#endif /* NCS_SRMND_LOG == 1 */

#if (NCS_SRMND_LOG == 1)
EXTERN_C uns32 srmnd_log_unreg(void);
EXTERN_C uns32 srmnd_log_reg(void);
#endif

#endif /* SRMND_LOG_H */


