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

  This file contains logging offset indices that are used for logging SRMSv 
  common information.
..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

#ifndef VDS_LOG_H
#define VDS_LOG_H

/******************************************************************************
                      Logging offset indices for MDS
******************************************************************************/
typedef enum vds_log_mds {
   VDS_LOG_MDS_REG,
   VDS_LOG_MDS_INSTALL,
   VDS_LOG_MDS_SUBSCRIBE,
   VDS_LOG_MDS_UNREG,
   VDS_LOG_MDS_SEND,
   VDS_LOG_MDS_CALL_BACK,
   VDS_LOG_MDS_RCV_CBK,
   VDS_LOG_MDS_CPY_CBK,
   VDS_LOG_MDS_SVEVT_CBK,
   VDS_LOG_MDS_ENC_CBK,
   VDS_LOG_MDS_FLAT_ENC_CBK,
   VDS_LOG_MDS_DEC_CBK,
   VDS_LOG_MDS_FLAT_DEC_CBK,
   VDS_LOG_MDS_ROLE_ACK,
   VDS_LOG_VDA_ROLECHANGE,
   VDS_LOG_MDS_SUCCESS,
   VDS_LOG_MDS_FAILURE,
   VDS_LOG_MDS_NOTHING,
   VDS_LOG_MDS_MAX
} VDS_LOG_MDS; 


/******************************************************************************
                        Logging offset indices for LOCK
******************************************************************************/
typedef enum vds_log_lock {
   VDS_LOG_LOCK_INIT,
   VDS_LOG_LOCK_DESTROY,
   VDS_LOG_LOCK_TAKE,
   VDS_LOG_LOCK_GIVE,
   VDS_LOG_LOCK_SUCCESS,
   VDS_LOG_LOCK_FAILURE,
   VDS_LOG_LOCK_MAX
} VDS_LOG_LOCK; 

/******************************************************************************
                     Logging offset indices for control block
******************************************************************************/
typedef enum vds_log_cb {
   VDS_LOG_CB_CREATE,
   VDS_LOG_CB_DESTROY,
   VDS_LOG_CB_RETRIEVE,
   VDS_LOG_CB_RETURN,
   VDS_LOG_CB_SUCCESS,
   VDS_LOG_CB_FAILURE,
   VDS_LOG_CB_MAX
} VDS_LOG_CB; 

/******************************************************************************
                     Logging offset indices for task
******************************************************************************/
typedef enum vds_log_task {
   VDS_LOG_TASK_CREATE,
   VDS_LOG_TASK_START,
   VDS_LOG_TASK_RELEASE,
   VDS_LOG_IPC_CREATE,  
   VDS_LOG_IPC_ATTACH,
   VDS_LOG_IPC_SEND,
   VDS_LOG_IPC_DETACH,
   VDS_LOG_IPC_RELEASE,  
   VDS_LOG_MBX_CREATE,
   VDS_LOG_MBX_DESTROY,
   VDS_LOG_TIM_SUCCESS,
   VDS_LOG_TIM_FAILURE,
   VDS_LOG_TIM_MAX
} VDS_LOG_TASK;
/******************************************************************************
                     Logging offset indices for patricia tree
******************************************************************************/
typedef enum vds_log_pat {
   VDS_LOG_PAT_INIT,
   VDS_LOG_PAT_DESTROY,
   VDS_LOG_PAT_ADD_ID,
   VDS_LOG_PAT_DEL_ID,
   VDS_LOG_PAT_ADD_NAME,
   VDS_LOG_PAT_DEL_NAME,
   VDS_LOG_PAT_SUCCESS,
   VDS_LOG_PAT_FAILURE,
   VDS_LOG_PAT_MAX
} VDS_LOG_PAT; 

/******************************************************************************
                Logging offset indices for Handle Manager associations
******************************************************************************/
typedef enum vds_log_hdl {
   VDS_LOG_HDL_CREATE_CB,
   VDS_LOG_HDL_DESTROY_CB,
   VDS_LOG_HDL_RETRIEVE_CB,
   VDS_LOG_HDL_RETURN_CB,
   VDS_LOG_HDL_SUCCESS,
   VDS_LOG_HDL_FAILURE,
   VDS_LOG_HDL_MAX
} VDS_LOG_HDL; 

/******************************************************************************
                    Logging offset indices for Memory
******************************************************************************/
typedef enum vds_log_mem
{
   VDS_LOG_MEM_VDS_CB,
   VDS_LOG_MEM_VDS_DB_INFO,
   VDS_LOG_MEM_VDS_ADEST_INFO,
   VDS_LOG_MEM_VDA_INFO,
   VDS_LOG_MEM_VDA_INFO_FREE,
   VDS_LOG_MEM_VDS_EVT,
   VDS_LOG_MEM_VDS_EVT_FREE,
   VDS_LOG_MEM_VDS_CKPT_BUFFER,
   VDS_LOG_MEM_ALLOC_SUCCESS,
   VDS_LOG_MEM_ALLOC_FAILURE,
   VDS_LOG_MEM_NOTHING,
   VDS_LOG_MEM_ALLOC_MAX
} VDS_LOG_MEM;

/******************************************************************************
                    Logging offset indices for AMF
******************************************************************************/
typedef enum vds_log_amf
{
  VDS_LOG_AMF_CBK_INIT,
  VDS_LOG_AMF_INITIALIZE,
  VDS_LOG_AMF_FINALIZE,
  VDS_LOG_AMF_GET_SEL_OBJ,
  VDS_LOG_AMF_GET_COMP_NAME,
  VDS_LOG_AMF_INIT,
  VDS_LOG_AMF_FIN,
  VDS_LOG_AMF_STOP_HLTCHECK,
  VDS_LOG_AMF_START_HLTCHECK,
  VDS_LOG_AMF_REG_COMP,
  VDS_LOG_AMF_UNREG_COMP,
  VDS_LOG_AMF_DISPATCH,
  VDS_LOG_AMF_RESPONSE,
  VDS_LOG_AMF_ACTIVE,
  VDS_LOG_AMF_QUIESCING,
  VDS_LOG_AMF_QUIESCED,
  VDS_LOG_AMF_STANDBY,   
  VDS_LOG_AMF_QUIESCING_QUIESCED,
  VDS_LOG_AMF_TERM_CALLBACK ,
  VDS_LOG_AMF_REMOVE_CALLBACK,
  VDS_LOG_AMF_SET_CALLBACK,
  VDS_LOG_AMF_INVALID ,     
  VDS_LOG_AMF_SUCCESS,
  VDS_LOG_AMF_FAILURE,
  VDS_LOG_AMF_NOTHING,
  VDS_LOG_AMF_MAX
} VDS_LOG_AMF; 

typedef enum vds_log_ckpt
{
  VDS_LOG_CKPT_CBK_INIT,
  VDS_LOG_CKPT_INITIALIZE,
  VDS_LOG_CKPT_FINALIZE,
  VDS_LOG_CKPT_REG_COMP,
  VDS_LOG_CKPT_UNREG_COMP,
  VDS_LOG_CKPT_INIT,
  VDS_LOG_CKPT_FIN,
  VDS_LOG_CKPT_OPEN,
  VDS_LOG_CKPT_CLOSE,
  VDS_LOG_CKPT_READ,
  VDS_LOG_CKPT_WRITE,
  VDS_LOG_CKPT_OVERWRITE,
  VDS_LOG_CKPT_DELETE,
  VDS_LOG_CKPT_SEC_DB_CWRITE,       
  VDS_LOG_CKPT_SEC_DB_READ,       
  VDS_LOG_CKPT_SEC_DB_OVERWRITE, 
  VDS_LOG_CKPT_SEC_DELETE,
  VDS_LOG_CKPT_SEC_CB_CWRITE,       
  VDS_LOG_CKPT_SEC_CB_READ,       
  VDS_LOG_CKPT_SEC_CB_OVERWRITE, 
  VDS_LOG_CKPT_SEC_ITER_INIT,
  VDS_LOG_CKPT_SEC_ITER_NEXT,
  VDS_LOG_CKPT_SEC_ITER_FIN,
  VDS_LOG_CKPT_SEC_VDS_VERSION_CREATE,
  VDS_LOG_CKPT_SEC_VDS_VERSION_READ,
  VDS_LOG_CKPT_SUCCESS,
  VDS_LOG_CKPT_FAILURE,
  VDS_LOG_CKPT_NOTHING,
  VDS_LOG_CKPT_MAX
} VDS_LOG_CKPT;

typedef enum vds_log_evt
{
  VDS_LOG_EVT_CREATE,
  VDS_LOG_EVT_RECEIVE,
  VDS_LOG_EVT_DESTROY,
  VDS_LOG_EVT_VDEST_CREATE,
  VDS_LOG_EVT_VDEST_LOOKUP,
  VDS_LOG_EVT_VDEST_DESTROY,
  VDS_LOG_EVT_SUCCESS,
  VDS_LOG_EVT_FAILURE,
  VDS_LOG_EVT_NOTHING,
  VDS_LOG_EVT_MAX
} VDS_LOG_EVT;

typedef enum vds_log_misc
{
    VDS_LOG_MISC_VDS_UP,
    VDS_LOG_MISC_VDS_DN,
    VDS_LOG_MISC_VDS_CREATE,
    VDS_LOG_MISC_VDS_DESTROY,
    VDS_LOG_MISC_SUCCESS,
    VDS_LOG_MISC_FAILURE,
    VDS_LOG_MISC_NOTHING,
    VDS_LOG_MISC_MAX
}VDS_LOG_MISC; 


/*****************************************************************************
logging offset indices for canned constant strings for the ASCII SPEC
******************************************************************************/
typedef enum vds_flex_sets {
   VDS_FC_MDS,
   VDS_FC_LOCK,
   VDS_FC_CB,
   VDS_FC_HDL,
   VDS_FC_TIM,
   VDS_FC_TREE,
   VDS_FC_MEM,
   VDS_FC_EVT,
   VDS_FC_AMF,
   VDS_FC_CKPT,
   VDS_FC_MISC
} VDS_FLEX_SETS;

typedef enum vds_log_ids {
   VDS_LID_MDS,
   VDS_LID_LOCK,
   VDS_LID_CB,
   VDS_LID_HDL,
   VDS_LID_TIM,
   VDS_LID_TREE,
   VDS_LID_MEM,
   VDS_LID_EVT,
   VDS_LID_AMF,
   VDS_LID_CKPT,
   VDS_LID_MISC,
   VDS_LID_VDEST,
   VDS_LID_AMF_STATE,
   VDS_LID_GENERIC,
   VDS_LID_MSG_FMT
} VDS_LOG_IDS;
 
/*****************************************************************************
                          Macros for Logging
*****************************************************************************/
/* #if (NCS_VDS_LOG == 1) */
/* DTSv versioning support */
#define VDS_LOG_VERSION 3

#define m_VDS_LOG_MEM(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_MEM, \
                                                    VDS_FC_MEM, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)


#define m_VDS_LOG_AMF(op, st, sev, status)     ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_AMF, \
                                                    VDS_FC_AMF, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TIIL, \
                                                    op, \
                                                    st, status)

#define m_VDS_LOG_MDS(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_MDS, \
                                                    VDS_FC_MDS, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)


#define m_VDS_LOG_LOCK(op, st, sev)    ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_LOCK, \
                                                    VDS_FC_LOCK, \
                                                    NCSFL_LC_LOCKS, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_VDS_LOG_CB(op, st, sev)      ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_CB, \
                                                    VDS_FC_CB, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)


#define m_VDS_LOG_MISC(op, st, sev)    ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_MISC, \
                                                    VDS_FC_MISC, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_VDS_LOG_HDL(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_HDL, \
                                                    VDS_FC_HDL, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_VDS_LOG_TIM(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_TIM, \
                                                    VDS_FC_TIM, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_VDS_LOG_TREE(op, st, sev, rc)     ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_TREE, \
                                                    VDS_FC_TREE, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TIIL, \
                                                    op, \
                                                    st,rc)

#define m_VDS_LOG_CKPT(op, st, sev, rc)     ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_CKPT, \
                                                    VDS_FC_CKPT, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TIIL, \
                                                    op, \
                                                    st, rc)


#define m_VDS_LOG_EVT(op, st, sev)     ncs_logmsg(NCS_SERVICE_ID_VDS, \
                                                    VDS_LID_EVT, \
                                                    VDS_FC_EVT, \
                                                    NCSFL_LC_HEADLINE, \
                                                    sev, \
                                                    NCSFL_TYPE_TII, \
                                                    op, \
                                                    st)

#define m_VDS_LOG_VDEST(var1,var2,var3)     ncs_logmsg(NCS_SERVICE_ID_VDS,\
                                                    VDS_LID_VDEST,\
                                                    0,\
                                                    NCSFL_LC_HEADLINE,\
                                                    NCSFL_SEV_INFO,\
                                                    "TCLC", \
                                                    var1,\
                                                    var2,var3) 

#define m_VDS_LOG_AMF_STATE(var1,var2)     ncs_logmsg(NCS_SERVICE_ID_VDS,\
                                                    VDS_LID_AMF_STATE,\
                                                    0,\
                                                    NCSFL_LC_HEADLINE,\
                                                    NCSFL_SEV_NOTICE,\
                                                    "TCC", \
                                                    var1,\
                                                    var2)


#define m_VDS_LOG_GENERIC(var1,var2,var3)     ncs_logmsg(NCS_SERVICE_ID_VDS,\
                                                    VDS_LID_GENERIC,\
                                                    0,\
                                                    NCSFL_LC_HEADLINE,\
                                                    NCSFL_SEV_INFO,\
                                                    "TCLC", \
                                                    var1,\
                                                    var2,var3) 
#define m_VDS_LOG_MDS_VER(var1,var2,var3,var4,var5) ncs_logmsg(NCS_SERVICE_ID_VDS,\
                                                    VDS_LID_MSG_FMT,\
                                                    0,\
                                                    NCSFL_LC_HEADLINE,\
                                                    NCSFL_SEV_NOTICE,\
                                                    "TCCLCL", \
                                                    var1,\
                                                    var2,var3,var4,var5)
#endif /* !VDS_LOG_H */


