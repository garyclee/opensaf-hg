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


   Sugadeesh Gudipalli, Vinay Khanna
..............................................................................

  DESCRIPTION:
   AvND component related definitions.
  
******************************************************************************
*/

#ifndef AVND_COMP_H
#define AVND_COMP_H

struct avnd_cb_tag;
struct avnd_su_si_rec;
struct avnd_su_tag;
struct avnd_srm_req_tag;

/***************************************************************************
 **********  S T R U C T U R E / E N U M  D E F I N I T I O N S  ***********
 ***************************************************************************/

/*##########################################################################
                    COMPONENT LIFE CYCLE (CLC) DEFINITIONS                  
 ##########################################################################*/

/* Need a define like this is SAF AIS header */
#define  SAAMF_CLC_LEN  AVSV_MISC_STR_MAX_SIZE

/* cmd param definitions */
#define AVND_COMP_CLC_PARAM_MAX       10
#define AVND_COMP_CLC_PARAM_SIZE_MAX  100

/*Return Exit Codes by Instantiate Code  */
#define AVND_COMP_INST_EXIT_CODE_NO_RETRY   200 

/* clc event handler declaration */
typedef uns32 (*AVND_COMP_CLC_FSM_FN)(struct avnd_cb_tag *, struct avnd_comp_tag *);

/* clc fsm events */
typedef enum avnd_comp_clc_pres_fsm_ev
{
   AVND_COMP_CLC_PRES_FSM_EV_INST = 1,
   AVND_COMP_CLC_PRES_FSM_EV_INST_SUCC,
   AVND_COMP_CLC_PRES_FSM_EV_INST_FAIL,
   AVND_COMP_CLC_PRES_FSM_EV_TERM,
   AVND_COMP_CLC_PRES_FSM_EV_TERM_SUCC,
   AVND_COMP_CLC_PRES_FSM_EV_TERM_FAIL,
   AVND_COMP_CLC_PRES_FSM_EV_CLEANUP,
   AVND_COMP_CLC_PRES_FSM_EV_CLEANUP_SUCC,
   AVND_COMP_CLC_PRES_FSM_EV_CLEANUP_FAIL,
   AVND_COMP_CLC_PRES_FSM_EV_RESTART,
   AVND_COMP_CLC_PRES_FSM_EV_ORPH,
   AVND_COMP_CLC_PRES_FSM_EV_MAX
} AVND_COMP_CLC_PRES_FSM_EV;

/* clc command type enums */
typedef enum avnd_comp_clc_cmd_type
{
   AVND_COMP_CLC_CMD_TYPE_INSTANTIATE = 1,
   AVND_COMP_CLC_CMD_TYPE_TERMINATE,
   AVND_COMP_CLC_CMD_TYPE_CLEANUP,
   AVND_COMP_CLC_CMD_TYPE_AMSTART,
   AVND_COMP_CLC_CMD_TYPE_AMSTOP,
   AVND_COMP_CLC_CMD_TYPE_MAX
} AVND_COMP_CLC_CMD_TYPE;

/* clc command parameter definition */
typedef struct avnd_comp_clc_param {
   uns8    cmd[SAAMF_CLC_LEN]; /* cmd ascii string */
   SaTimeT timeout;            /* cmd timeout value */
   uns32   len;                /* cmd len */
} AVND_COMP_CLC_CMD_PARAM;

/* clc info definition (top level wrapper structure) */
typedef struct avnd_comp_clc_info {

   /* clc commands (indexed by cmd type) */
   AVND_COMP_CLC_CMD_PARAM   cmds[AVND_COMP_CLC_CMD_TYPE_MAX - 1];

   uns32  inst_retry_max; /* configured no of instantiate retry attempts */
   uns32  inst_retry_cnt; /* curr no of instantiate retry attempts */

   uns32  am_start_retry_max; /* configured no of AM_START retry attempts */
   uns32  am_start_retry_cnt; /* curr no of AM_START retry attempts */

   /* 
    * current command execution info (note that a comp has only 
    * one outstanding command in execution other than AM related)
    */
   AVND_COMP_CLC_CMD_TYPE  exec_cmd;      /* command in execution */
   uns32                   cmd_exec_ctxt; /* command execution context */

   AVND_COMP_CLC_CMD_TYPE  am_exec_cmd;      /* command in execution */
   uns32                   am_cmd_exec_ctxt; /* command execution context */

   /* comp reg tmr info */
   SaTimeT  inst_cmd_ts; /* instantiate cmd start timestamp */
   AVND_TMR clc_reg_tmr; /* comp reg tmr */
  
   uns32                   inst_code_rcvd;  /* Store the error value 
                                                      received from the instantiate script */

} AVND_COMP_CLC_INFO;


/*##########################################################################
                        COMPONENT CALLBACK DEFINITIONS                      
 ##########################################################################*/

/* callbk list node definition */
typedef struct avnd_cbk_tag {
   uns32             opq_hdl;   /* hdl returned by hdl-mngr */
   uns32             orig_opq_hdl;   /* Original hdl to be stored for proxing it at AvND */
   /* Redundant AvND hdl, generated at STDBY AvND. If red_opq_hdl is non-zero, then
      don't destroy opq_hdl, rather destroy red_opq_hdl during avnd_comp_cbq_rec_del()*/
   uns32             red_opq_hdl;
   MDS_DEST          dest;      /* mds dest of the prc where the callback is destined */
   SaTimeT           timeout;   /* resp timeout value */
   AVND_TMR          resp_tmr;  /* timer for callback response */
   AVSV_AMF_CBK_INFO *cbk_info; /* callbk info */

   /* link to other elements */
   struct avnd_comp_tag *comp; /* bk ptr to the comp */
   struct avnd_cbk_tag   *next;
   SaNameT  comp_name_net; /* For checkpointing */
} AVND_COMP_CBK;


/*##########################################################################
                 COMPONENT SERVICE INSTANCE (CSI) DEFINITIONS               
 ##########################################################################*/

/* CSI assignment state type definition */
typedef enum avnd_comp_csi_assign_state
{
   AVND_COMP_CSI_ASSIGN_STATE_UNASSIGNED = 1,
   AVND_COMP_CSI_ASSIGN_STATE_ASSIGNING,
   AVND_COMP_CSI_ASSIGN_STATE_ASSIGNED,
   AVND_COMP_CSI_ASSIGN_STATE_REMOVING,
   AVND_COMP_CSI_ASSIGN_STATE_REMOVED,
   AVND_COMP_CSI_ASSIGN_STATE_RESTARTING,
   AVND_COMP_CSI_ASSIGN_STATE_MAX,
} AVND_COMP_CSI_ASSIGN_STATE;

typedef AVSV_SUSI_ASGN  AVND_COMP_CSI_PARAM;

/* CSI info */
typedef struct avnd_comp_csi_rec {
   NCS_DB_LINK_LIST_NODE  si_dll_node;   /* node in the si dll */
   NCS_DB_LINK_LIST_NODE  comp_dll_node; /* node in the comp-csi dll */
   SaNameT                name_net;      /* csi name */
   uns32                  rank;          /* csi rank */

   uns32                  mab_hdl; /* mab hdl for this csi */

   /* state info */
   SaNameT                       act_comp_name_net; /* active comp name */
   SaAmfCSITransitionDescriptorT trans_desc;        /* transition descriptor */
   uns32                         standby_rank;      /* standby rank */
   NCS_AVSV_CSI_ATTRS            attrs;             /* list of attributes */
   AVND_COMP_CSI_ASSIGN_STATE    curr_assign_state; /* csi assignment state 
                                                       wrt current ha state */
   AVND_COMP_CSI_ASSIGN_STATE    prv_assign_state;  /* csi assignment state 
                                                       wrt prv ha state */

   /* links to other entities */
   struct avnd_comp_tag     *comp;     /* bk ptr to the comp */
   struct avnd_su_si_rec    *si;       /* bk ptr to the si record */
   SaNameT                comp_name_net;    /* For Checkpointing */
   SaNameT                si_name_net;      /* For Checkpointing */
   SaNameT                su_name_net;      /* For Checkpointing */

} AVND_COMP_CSI_REC;

#define AVND_COMP_CSI_REC_NULL ((AVND_COMP_CSI_REC *)0)


/*##########################################################################
           COMPONENT MONITORING (HEALTHCHECK / PASSIVE) DEFINITIONS         
 ##########################################################################*/

/* Component healthcheck operation enums */
typedef enum avnd_comp_hc_op
{
   AVND_COMP_HC_START = 1,
   AVND_COMP_HC_STOP,
   AVND_COMP_HC_CONFIRM,
   AVND_COMP_HC_TMR_EXP,
   AVND_COMP_HC_OP_MAX,
} AVND_COMP_HC_OP;

/* Component healthcheck status enums for AMF invoked HC */
typedef enum avnd_comp_hc_status
{
   AVND_COMP_HC_STATUS_STABLE,
   AVND_COMP_HC_STATUS_WAIT_FOR_RESP,
   AVND_COMP_HC_STATUS_SND_TMR_EXPD
} AVND_COMP_HC_STATUS;

/* Health Check Info */
typedef struct avnd_hc_rec_tag {
   NCS_DB_LINK_LIST_NODE  comp_dll_node; /* node in the comp-hc dll  */

   /* health check info */                      
   SaAmfHealthcheckKeyT        key;      /* health check key (index) */
   SaAmfHealthcheckInvocationT inv;      /* invocation type */
   AVSV_ERR_RCVR               rec_rcvr; /* recommended recovery */

   /* params used to locate the correct thread & process */
   SaAmfHandleT req_hdl; /* AMF handle value */
   MDS_DEST     dest;    /* mds dest of the prc that started health check */
                                                
   /* timer interval values */                  
   SaTimeT  period;  /* healthcheck periodic interval */
   SaTimeT  max_dur; /* hold timeout interval */

   AVND_TMR tmr;     /* healthcheck timer */
   uns32    opq_hdl; /* hdl returned by hdl-mngr (used during tmr expiry) */
   AVND_COMP_HC_STATUS status; /* indicates status of hc rec */ 

   struct avnd_comp_tag    *comp; /* back ptr to the comp */
   struct avnd_hc_rec_tag  *next;
   SaNameT  comp_name_net; /* For checkpoiting */
} AVND_COMP_HC_REC;


 
/* Passive Monitoring Info */
typedef struct avnd_pm_rec {
   NCS_DB_LINK_LIST_NODE  comp_dll_node; /* node in the comp-pm dll  */
   SaUint64T  pid;     /* pid of the prc that is being monitored (index) */
   uns32      rsrc_hdl; /* hdl returned by srmsv for this request */
   SaAmfHandleT req_hdl; /* AMF handle value */

   /* pm info */
   SaInt32T       desc_tree_depth; /* descendent tree depth */
   SaAmfPmErrorsT err;             /* error that is monitored */
   AVSV_ERR_RCVR  rec_rcvr;        /* recommended recovery */

   /* links to other entities */
   struct avnd_comp_tag *comp; /* back ptr to the comp */
} AVND_COMP_PM_REC;


/*##########################################################################
                       COMPONENT PROXIED DEFINITIONS         
 ##########################################################################*/

/* proxied info */
typedef struct avnd_pxied_rec {
   NCS_DB_LINK_LIST_NODE  comp_dll_node; /* node in the comp-pxied dll  */
   struct avnd_comp_tag *pxied_comp; /* ptr to the proxied comp */
} AVND_COMP_PXIED_REC;

#define AVND_COMP_TYPE_LOCAL_NODE           0x00000001
#define AVND_COMP_TYPE_INTER_NODE           0x00000002
#define AVND_COMP_TYPE_EXT_CLUSTER          0x00000004

/*##########################################################################
                       COMPONENT DEFINITION (TOP LEVEL)                    
 ##########################################################################*/

typedef AVSV_COMP_INFO  AVND_COMP_PARAM;

typedef struct avnd_comp_tag {
   NCS_PATRICIA_NODE      tree_node;   /* comp tree node (key is comp name) */
   NCS_DB_LINK_LIST_NODE  su_dll_node; /* su dll node (key is inst-level) */

   SaNameT  name_net;   /* comp name */
   uns32    inst_level; /* comp instantiation level */
   
   uns32       mab_hdl;  /* mab handle for this comp */
   uns32       comp_hdl; /* hdl returned by hdl-mngr */

   /* component attributes */
   uns32     flag;           /* comp attributes */
   NCS_BOOL  is_restart_en;  /* flag to indicate if comp-restart is allowed */
   NCS_COMP_CAPABILITY_MODEL cap; /* comp capability model */
   NCS_BOOL  is_am_en;

   /* clc info */
   AVND_COMP_CLC_INFO  clc_info;

   /* Update received flag, which will normally be FALSE and will be
    * TRUE if updates are received from the AVD on fail-over.*/
   NCS_BOOL            avd_updt_flag;

   /* component registration info */
   SaAmfHandleT reg_hdl;  /* registered handle value */
   MDS_DEST     reg_dest; /* mds dest of the registering prc */

   /* component states */                                
   NCS_OPER_STATE   oper;  /* operational state */
   NCS_PRES_STATE   pres;  /* presence state */

   /* 
    * component request info (healthcheck, passive 
    * monitoring, protection group tracking etc)
    */
   NCS_DB_LINK_LIST  hc_list;  /* health check info list */
   NCS_DB_LINK_LIST  pm_list;  /* passive monitoring req list */

   AVND_COMP_CBK  *cbk_list;    /* pending callback list */

   /* call back responce timeout values */
   SaTimeT  term_cbk_timeout;          /* terminate          */
   SaTimeT  csi_set_cbk_timeout;       /* csi set            */
   SaTimeT  quies_complete_cbk_timeout;/* quiescing_complete */
   SaTimeT  csi_rmv_cbk_timeout;       /* csi remove         */
   SaTimeT  pxied_inst_cbk_timeout;    /* proxied instantiate*/
   SaTimeT  pxied_clean_cbk_timeout;   /* proxied cleanup    */

   AVND_CERR_INFO  err_info; /* comp error information */

   uns32 curr_proxied_cnt; /* proxied comp count (if any) 
                              ## not used, to be deleted */

   NCS_DB_LINK_LIST  csi_list; /* csi list */

   struct avnd_su_tag   *su;         /* back ptr to parent SU */
   
   struct avnd_comp_tag *pxy_comp; /* ptr to the proxy comp (if any) */
   
   AVND_COMP_CLC_PRES_FSM_EV pend_evt; /* stores last fsm event got in orph state*/
   
   AVND_TMR orph_tmr; /* proxied component registration timer alias orphaned timer*/
   
   NCS_DB_LINK_LIST  pxied_list;  /* list of proxied comp in this proxy */
   NODE_ID           node_id; /* It will used for internode proxy-proxied components. */
   uns32             comp_type; /* Whether the component is LOCAL, INTERNODE or EXT */
   MDS_SYNC_SND_CTXT mds_ctxt;
   NCS_BOOL          reg_resp_pending; /* If the reg resp is pending from 
                                          proxied comp AvND, it TRUE. */
   SaNameT           proxy_comp_name_net; /* Used for Checkpointing. */

} AVND_COMP;

#define AVND_COMP_NULL ((AVND_COMP *)0)


/***************************************************************************
 ******************  M A C R O   D E F I N I T I O N S  ********************
 ***************************************************************************/

/* macros for comp oper state */
#define m_AVND_COMP_OPER_STATE_IS_ENABLED(x) \
                      ((NCS_OPER_STATE_ENABLE == (x)->oper))
#define m_AVND_COMP_OPER_STATE_IS_DISABLED(x) \
                      ((NCS_OPER_STATE_DISABLE == (x)->oper))
#define m_AVND_COMP_OPER_STATE_SET(x, val)  (((x)->oper = val))

/* macros to manage the presence state */
#define m_AVND_COMP_PRES_STATE_SET(x, val)  ((x)->pres = val)
#define m_AVND_COMP_PRES_STATE_IS_UNINSTANTIATED(x) \
           ( NCS_PRES_UNINSTANTIATED == (x)->pres )
#define m_AVND_COMP_PRES_STATE_IS_INSTANTIATED(x) \
           ( NCS_PRES_INSTANTIATED == (x)->pres )
#define m_AVND_COMP_PRES_STATE_IS_INSTANTIATING(x) \
           ( NCS_PRES_INSTANTIATING == (x)->pres )
#define m_AVND_COMP_PRES_STATE_IS_RESTARTING(x) \
           ( NCS_PRES_RESTARTING == (x)->pres )
#define m_AVND_COMP_PRES_STATE_IS_ORPHANED(x) \
           ( NCS_PRES_ORPHANED == (x)->pres )
#define m_AVND_COMP_PRES_STATE_IS_TERMINATING(x) \
           ( NCS_PRES_TERMINATING == (x)->pres )
#define m_AVND_COMP_PRES_STATE_IS_INSTANTIATIONFAILED(x) \
           ( NCS_PRES_INSTANTIATIONFAILED == (x)->pres )
#define m_AVND_COMP_PRES_STATE_IS_TERMINATIONFAILED(x) \
           ( NCS_PRES_TERMINATIONFAILED == (x)->pres )

/* pre-configured comp type values */
#define AVND_COMP_TYPE_LOCAL           0x00000001
#define AVND_COMP_TYPE_PROXY           0x00000002
#define AVND_COMP_TYPE_PROXIED         0x00000004
#define AVND_COMP_TYPE_PREINSTANTIABLE 0x00000008
#define AVND_COMP_TYPE_SAAWARE         0x00000010

/* component state (comp-reg, failed etc.) values */
#define AVND_COMP_FLAG_REG            0x00000100
#define AVND_COMP_FLAG_FAILED         0x00000200
#define AVND_COMP_FLAG_INST_CMD_SUCC  0x00000400
#define AVND_COMP_FLAG_ALL_CSI        0x00000800
#define AVND_COMP_FLAG_TERM_FAIL      0x00001000 /* This flag will be set for a small duration
                                                    from a term cbk or cmd fail to the time we
                                                    invoke cleanup cbk or cmd */
/* The following flag is set for a proxy who is proxying an external comp.*/
#define AVND_PROXY_FOR_EXT_COMP       0x00002000



#define m_AVND_COMP_TYPE_IS_LOCAL_NODE(x)   (((x)->comp_type) & AVND_COMP_TYPE_LOCAL_NODE)
#define m_AVND_COMP_TYPE_IS_INTER_NODE(x)   (((x)->comp_type) & AVND_COMP_TYPE_INTER_NODE)
#define m_AVND_COMP_TYPE_IS_EXT_CLUSTER(x)  (((x)->comp_type) & AVND_COMP_TYPE_EXT_CLUSTER)

#define m_AVND_COMPT_TYPE_SET(x, flag)      (((x)->comp_type) |= (flag))
#define m_AVND_COMP_TYPE_SET_LOCAL_NODE(x)  m_AVND_COMPT_TYPE_SET(x, AVND_COMP_TYPE_LOCAL_NODE) 
#define m_AVND_COMP_TYPE_SET_INTER_NODE(x)  m_AVND_COMPT_TYPE_SET(x, AVND_COMP_TYPE_INTER_NODE) 
#define m_AVND_COMP_TYPE_SET_EXT_CLUSTER(x) m_AVND_COMPT_TYPE_SET(x, AVND_COMP_TYPE_EXT_CLUSTER)

/* macros for checking the comp types */
#define m_AVND_COMP_TYPE_IS_LOCAL(x)              (((x)->flag) & AVND_COMP_TYPE_LOCAL)
#define m_AVND_COMP_TYPE_IS_PROXY(x)              (((x)->flag) & AVND_COMP_TYPE_PROXY)
#define m_AVND_COMP_TYPE_IS_PROXIED(x)            (((x)->flag) & AVND_COMP_TYPE_PROXIED)
#define m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(x)    (((x)->flag) & AVND_COMP_TYPE_PREINSTANTIABLE)
#define m_AVND_COMP_TYPE_IS_SAAWARE(x)            (((x)->flag) & AVND_COMP_TYPE_SAAWARE)

/* macros for setting the comp types */
#define m_AVND_COMP_TYPE_SET(x, bitmap) (((x)->flag) |= (bitmap))
#define m_AVND_COMP_TYPE_LOCAL_SET(x)            m_AVND_COMP_TYPE_SET(x, AVND_COMP_TYPE_LOCAL)
#define m_AVND_COMP_TYPE_PROXY_SET(x)            m_AVND_COMP_TYPE_SET(x, AVND_COMP_TYPE_PROXY)
#define m_AVND_COMP_TYPE_PROXIED_SET(x)          m_AVND_COMP_TYPE_SET(x, AVND_COMP_TYPE_PROXIED)
#define m_AVND_COMP_TYPE_PREINSTANTIABLE_SET(x)  m_AVND_COMP_TYPE_SET(x, AVND_COMP_TYPE_PREINSTANTIABLE)
#define m_AVND_COMP_TYPE_SAAWARE_SET(x)          m_AVND_COMP_TYPE_SET(x, AVND_COMP_TYPE_SAAWARE)

/* macros for resetting the comp types */
#define m_AVND_COMP_TYPE_RESET(x, bitmap) (((x)->flag) &= ~(bitmap))
#define m_AVND_COMP_TYPE_LOCAL_RESET(x)           m_AVND_COMP_TYPE_RESET(x, AVND_COMP_TYPE_LOCAL)
#define m_AVND_COMP_TYPE_PROXY_RESET(x)           m_AVND_COMP_TYPE_RESET(x, AVND_COMP_TYPE_PROXY)
#define m_AVND_COMP_TYPE_PROXIED_RESET(x)         m_AVND_COMP_TYPE_RESET(x, AVND_COMP_TYPE_PROXIED)
#define m_AVND_COMP_TYPE_PREINSTANTIABLE_RESET(x) m_AVND_COMP_TYPE_RESET(x, AVND_COMP_TYPE_PREINSTANTIABLE)
#define m_AVND_COMP_TYPE_SAAWARE_RESET(x)         m_AVND_COMP_TYPE_RESET(x, AVND_COMP_TYPE_SAAWARE)

/* macros for checking the comp states */
#define m_AVND_COMP_IS_REG(x)            (((x)->flag) & AVND_COMP_FLAG_REG)
#define m_AVND_COMP_IS_FAILED(x)         (((x)->flag) & AVND_COMP_FLAG_FAILED)
#define m_AVND_COMP_IS_INST_CMD_SUCC(x)  (((x)->flag) & AVND_COMP_FLAG_INST_CMD_SUCC)
#define m_AVND_COMP_IS_ALL_CSI(x)        (((x)->flag) & AVND_COMP_FLAG_ALL_CSI)
#define m_AVND_COMP_IS_TERM_FAIL(x)      (((x)->flag) & AVND_COMP_FLAG_TERM_FAIL)
#define m_AVND_PROXY_IS_FOR_EXT_COMP(x)  (((x)->flag) & AVND_PROXY_FOR_EXT_COMP)

/* macros for setting the comp states */
#define m_AVND_COMP_REG_SET(x)            (((x)->flag) |= AVND_COMP_FLAG_REG)
#define m_AVND_COMP_FAILED_SET(x)         (((x)->flag) |= AVND_COMP_FLAG_FAILED)
#define m_AVND_COMP_INST_CMD_SUCC_SET(x)  (((x)->flag) |= AVND_COMP_FLAG_INST_CMD_SUCC)
#define m_AVND_COMP_ALL_CSI_SET(x)        (((x)->flag) |= AVND_COMP_FLAG_ALL_CSI)
#define m_AVND_COMP_TERM_FAIL_SET(x)      (((x)->flag) |= AVND_COMP_FLAG_TERM_FAIL)
#define m_AVND_PROXY_FOR_EXT_COMP_SET(x)  (((x)->flag) |= AVND_PROXY_FOR_EXT_COMP)

/* macros for resetting the comp states */
#define m_AVND_COMP_REG_RESET(x)            (((x)->flag) &= ~AVND_COMP_FLAG_REG)
#define m_AVND_COMP_FAILED_RESET(x)         (((x)->flag) &= ~AVND_COMP_FLAG_FAILED)
#define m_AVND_COMP_INST_CMD_SUCC_RESET(x)  (((x)->flag) &= ~AVND_COMP_FLAG_INST_CMD_SUCC)
#define m_AVND_COMP_ALL_CSI_RESET(x)        (((x)->flag) &= ~AVND_COMP_FLAG_ALL_CSI)
#define m_AVND_COMP_TERM_FAIL_RESET(x)      (((x)->flag) &= ~AVND_COMP_FLAG_TERM_FAIL)


/* macros for comp csi assignment state */
#define m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_UNASSIGNED(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_UNASSIGNED == (x)->curr_assign_state))
#define m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_ASSIGNING(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_ASSIGNING == (x)->curr_assign_state))
#define m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_ASSIGNED(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_ASSIGNED == (x)->curr_assign_state))
#define m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_REMOVING(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_REMOVING == (x)->curr_assign_state))
#define m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_REMOVED(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_REMOVED == (x)->curr_assign_state))
#define m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_RESTARTING(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_RESTARTING == (x)->curr_assign_state))
#define m_AVND_COMP_CSI_CURR_ASSIGN_STATE_SET(x, val)  \
           (((x)->curr_assign_state = val))

#define m_AVND_COMP_CSI_PRV_ASSIGN_STATE_IS_UNASSIGNED(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_UNASSIGNED == (x)->prv_assign_state))
#define m_AVND_COMP_CSI_PRV_ASSIGN_STATE_IS_ASSIGNING(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_ASSIGNING == (x)->prv_assign_state))
#define m_AVND_COMP_CSI_PRV_ASSIGN_STATE_IS_ASSIGNED(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_ASSIGNED == (x)->prv_assign_state))
#define m_AVND_COMP_CSI_PRV_ASSIGN_STATE_IS_REMOVING(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_REMOVING == (x)->prv_assign_state))
#define m_AVND_COMP_CSI_PRV_ASSIGN_STATE_IS_REMOVED(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_REMOVED == (x)->prv_assign_state))
#define m_AVND_COMP_CSI_PRV_ASSIGN_STATE_IS_RESTARTING(x) \
           ((AVND_COMP_CSI_ASSIGN_STATE_RESTARTING == (x)->prv_assign_state))
#define m_AVND_COMP_CSI_PRV_ASSIGN_STATE_SET(x, val)  \
           (((x)->prv_assign_state = val))

/* macros for managing the comp restart-enable flag */
#define m_AVND_COMP_RESTART_EN_SET(x, val) ((x)->is_restart_en = val)
#define m_AVND_COMP_IS_RESTART_EN(x)       ((x)->is_restart_en == TRUE)
#define m_AVND_COMP_IS_RESTART_DIS(x)      ((x)->is_restart_en == FALSE)


/* macro to reset the comp-reg params */
#define m_AVND_COMP_REG_PARAM_RESET(cb, comp) \
{ \
   /* stop comp-reg tmr */ \
   if ( m_AVND_TMR_IS_ACTIVE((comp)->clc_info.clc_reg_tmr) ) \
      m_AVND_TMR_COMP_REG_STOP((cb), *(comp)); \
   (comp)->reg_hdl = 0; \
   memset(&(comp)->reg_dest, 0, sizeof(MDS_DEST)); \
   m_AVND_COMP_REG_RESET((comp)); \
};

/* macro to reset the instantiate params */
#define m_AVND_COMP_CLC_INST_PARAM_RESET(comp) \
{ \
   /* reset the inst-cmd start timestamp */ \
   (comp)->clc_info.inst_cmd_ts = 0; \
   /* reset inst-cmd succ flag */ \
   m_AVND_COMP_INST_CMD_SUCC_RESET((comp)); \
};


/* macro to determine if the pre-configured proxy comp has any proxied comps */
#define m_AVND_COMP_IS_PROXY(x)  ((x)->curr_proxied_cnt)

/* macro to determine if the pre-configured proxied comp has any proxy comp */
#define m_AVND_COMP_IS_PROXIED(x)  ((x)->proxy_comp)

/* macro to retrieve component ptr from su dll node ptr */
#define m_AVND_COMP_SU_DLL_NODE_OFFSET \
            ( (uns8 *)&(AVND_COMP_NULL->su_dll_node) - (uns8 *)AVND_COMP_NULL )
#define m_AVND_COMP_FROM_SU_DLL_NODE_GET(x)  \
            ( (x) ? ((AVND_COMP *)(((uns8 *)(x)) - m_AVND_COMP_SU_DLL_NODE_OFFSET)) : 0 )
                

/* macro to retrieve csi ptr from comp-csi dll node ptr */
#define m_AVND_CSI_COMP_DLL_NODE_OFFSET \
            ( (uns8 *)&(AVND_COMP_CSI_REC_NULL->comp_dll_node) - (uns8 *)AVND_COMP_CSI_REC_NULL )
#define m_AVND_CSI_REC_FROM_COMP_DLL_NODE_GET(x)  \
            ( (x) ? ((AVND_COMP_CSI_REC *)(((uns8 *)(x)) - m_AVND_CSI_COMP_DLL_NODE_OFFSET)) : 0 )

/* macro to get a component record from comp-db */
#define m_AVND_COMPDB_REC_GET(compdb, name_net) \
   (AVND_COMP *)ncs_patricia_tree_get(&(compdb), (uns8 *)&(name_net))

/* macro to get the next component record from comp-db */
#define m_AVND_COMPDB_REC_GET_NEXT(compdb, name_net) \
   (AVND_COMP *)ncs_patricia_tree_getnext(&(compdb), (uns8 *)&(name_net))

/* macro to add a csi record to the comp-csi list */
#define m_AVND_COMPDB_REC_CSI_ADD(comp, csi, rc) \
{ \
   (csi).comp_dll_node.key = (uns8 *)&(csi).name_net; \
   (rc) = ncs_db_link_list_add(&(comp).csi_list, &(csi).comp_dll_node); \
};

/* macro to remove a csi record from the comp-csi list */
#define m_AVND_COMPDB_REC_CSI_REM(comp, csi) \
           ncs_db_link_list_delink(&(comp).csi_list, &(csi).comp_dll_node);

/* macro to get a csi record from the comp-csi list */
#define m_AVND_COMPDB_REC_CSI_GET(comp, csi_name_net) \
           m_AVND_CSI_REC_FROM_COMP_DLL_NODE_GET(ncs_db_link_list_find(&(comp).csi_list, \
                                                 (uns8 *)&(csi_name_net)))

/* macro to get the first csi record from the comp-csi list */
#define m_AVND_COMPDB_REC_CSI_GET_FIRST(comp) \
           m_AVND_CSI_REC_FROM_COMP_DLL_NODE_GET(m_NCS_DBLIST_FIND_FIRST(&(comp).csi_list))

/* macro to get the next csi record from the comp-csi list */
#define m_AVND_COMPDB_REC_CSI_NEXT(comp, csi) \
           m_AVND_CSI_REC_FROM_COMP_DLL_NODE_GET( \
              m_NCS_DBLIST_FIND_NEXT( \
                 ncs_db_link_list_find(&(comp).csi_list, \
                                       (uns8 *)&((csi).name_net)) \
                                    ) \
                                           )

/* macro to add a healthcheck record to the comp-hc list */
#define m_AVND_COMPDB_REC_HC_ADD(comp, hc, rc) \
{ \
   (hc).comp_dll_node.key = (uns8 *)&(hc); \
   (rc) = ncs_db_link_list_add(&(comp).hc_list, &(hc).comp_dll_node); \
};

/* macro to remove a healthcheck record from the comp-hc list */
#define m_AVND_COMPDB_REC_HC_REM(comp, hc) \
           ncs_db_link_list_delink(&(comp).hc_list, &(hc).comp_dll_node);

/* macro to get a healthcheck record from the comp-hc list */
#define m_AVND_COMPDB_REC_HC_GET(comp, hc_key) \
           (AVND_COMP_HC_REC *)ncs_db_link_list_find(&(comp).hc_list, \
                                                     (uns8 *)&(hc_key))

/* macro to get the specified healthcheck callback from the comp-cbk list */
#define m_AVND_COMPDB_CBQ_HC_CBK_GET(comp, hck, o_rec) \
{ \
   for ((o_rec) = (comp)->cbk_list; \
        (o_rec) && !( (AVSV_AMF_HC == (o_rec)->cbk_info->type) && \
                      (0 == m_CMP_HORDER_SAHCKEY((o_rec)->cbk_info->param.hc.hc_key, hck)) ); \
        (o_rec) = (o_rec)->next); \
}

#define m_AVND_COMP_OPER_STATE_AVD_SYNC(cb, comp, o_rc) \
{ \
   AVSV_PARAM_INFO param; \
   memset(&param, 0, sizeof(AVSV_PARAM_INFO)); \
   param.table_id = NCSMIB_TBL_AVSV_AMF_COMP; \
   param.obj_id = saAmfCompOperState_ID; \
   param.name_net = (comp)->name_net; \
   param.act = AVSV_OBJ_OPR_MOD; \
   *((uns32 *)param.value) = m_NCS_OS_HTONL((comp)->oper); \
   param.value_len = sizeof(uns32); \
   (o_rc) = avnd_di_mib_upd_send((cb), &param); \
};

/* macro to parse the clc cmd string */
#define m_AVND_COMP_CLC_STR_PARSE(st, sc, ac, av, tav) \
{ \
   char str[SAAMF_CLC_LEN], *tok = NULL; \
   /* copy the str as strtok modifies the original str */ \
   strcpy(str, st); \
   ac = 0; \
   if ( NULL != (tok = m_NCS_STRTOK(str, " ")) ) { \
      strcpy(sc, tok); \
      av[ac] = sc; \
   } \
   ac++; \
   while ( (NULL != (tok = m_NCS_STRTOK(NULL, " "))) && \
           (ac < (AVND_COMP_CLC_PARAM_MAX+1)) ) { \
      if ( m_NCS_STRLEN(tok) > AVND_COMP_CLC_PARAM_SIZE_MAX ) break; \
      strcpy(tav[ac], tok); \
      av[ac] = tav[ac]; \
      ac++;\
   } \
   if ( NULL != tok ) { \
      sc[0] = (char)(long)NULL; \
      av[0] = NULL; \
      ac = 0; \
   } else \
      av[ac] = NULL; \
}

/*****************************************************************************
                 Macros to fill the callback parameters
*****************************************************************************/

/* macro to populate the 'healthcheck' callback params */
#define m_AVND_AMF_HC_CBK_FILL(cbk, cn, hck) \
{ \
   (cbk).type = AVSV_AMF_HC; \
   (cbk).param.hc.comp_name_net = (cn); \
   (cbk).param.hc.hc_key = (hck); \
}

/* macro to populate the 'terminate' callback params */
#define m_AVND_AMF_COMP_TERM_CBK_FILL(cbk, cn) \
{ \
   (cbk).type = AVSV_AMF_COMP_TERM; \
   (cbk).param.comp_term.comp_name_net = (cn); \
}

/* macro to populate the 'csi set' callback params */
#define m_AVND_AMF_CSI_SET_CBK_FILL(cbk, cn, has, csid, at) \
{ \
   (cbk).type = AVSV_AMF_CSI_SET; \
   (cbk).param.csi_set.comp_name_net = (cn); \
   (cbk).param.csi_set.ha = (has); \
   (cbk).param.csi_set.csi_desc = (csid); \
   (cbk).param.csi_set.attrs = (at); \
}

/* macro to populate the 'csi remove' callback params */
#define m_AVND_AMF_CSI_REM_CBK_FILL(cbk, cn, csn, csf) \
{ \
   (cbk).type = AVSV_AMF_CSI_REM; \
   (cbk).param.csi_rem.comp_name_net = (cn); \
   (cbk).param.csi_rem.csi_name_net = (csn); \
   (cbk).param.csi_rem.csi_flags = (csf); \
}

/* macro to populate the 'proxied instantiate' callback params */
#define m_AVND_AMF_PXIED_COMP_INST_CBK_FILL(cbk, cn) \
{ \
   (cbk).type = AVSV_AMF_PXIED_COMP_INST; \
   (cbk).param.comp_term.comp_name_net = (cn); \
}

/* macro to populate the 'proxied cleanup' callback params */
#define m_AVND_AMF_PXIED_COMP_CLEAN_CBK_FILL(cbk, cn) \
{ \
   (cbk).type = AVSV_AMF_PXIED_COMP_CLEAN; \
   (cbk).param.comp_term.comp_name_net = (cn); \
}


/*****************************************************************************
       Macros to manage the pending callback list 
*****************************************************************************/

/* macro to push the callback parameters (to the end of the list) */
#define m_AVND_COMP_CBQ_START_PUSH(comp, rec) \
{ \
   AVND_COMP_CBK *prv = (comp)->cbk_list, *curr; \
   if((comp)->cbk_list == 0)\
      (comp)->cbk_list = rec;\
   else\
   {\
   for (curr = (comp)->cbk_list; curr ; \
        prv = curr, curr = curr->next); \
   prv->next = (rec);\
   (rec)->next = 0;\
   }\
}

/* macro to pop the callback parameters (from the beginning of the list) */
#define m_AVND_COMP_CBQ_START_POP(comp, o_rec) \
{ \
   if ((comp)->cbk_list) { \
      (o_rec) = (comp)->cbk_list; \
      (comp)->cbk_list = (o_rec)->next; \
      (o_rec)->next = 0; \
   } else (o_rec) = 0; \
}

/* macro to pop a given callback record from the list */
#define m_AVND_COMP_CBQ_REC_POP(comp, rec, o_found) \
{ \
   AVND_COMP_CBK *prv = (comp)->cbk_list, *curr; \
   o_found = 0;\
   for (curr = (comp)->cbk_list; \
        curr && !(curr == (rec)); \
        prv = curr, curr = curr->next); \
   if (curr) \
   { \
      /* found the record... pop it */ \
      o_found = 1;\
      if ( curr == (comp)->cbk_list ) \
         (comp)->cbk_list = curr->next; \
      else \
         prv->next = curr->next; \
      curr->next = 0; \
   } \
}

/* macro to get the callback record with the same inv value */
/* note that inv value is derived from the hdl mngr */
#define m_AVND_COMP_CBQ_INV_GET(comp, invc, o_rec) \
{ \
   for ((o_rec) = (comp)->cbk_list; \
        (o_rec) && !(((o_rec)->cbk_info->inv) == (invc)); \
        (o_rec) = (o_rec)->next); \
}

#define m_AVND_COMP_CBQ_ORIG_INV_GET(comp, inv, o_rec) \
{ \
   for ((o_rec) = (comp)->cbk_list; \
        (o_rec) && !((o_rec)->orig_opq_hdl == (inv)); \
        (o_rec) = (o_rec)->next); \
}

/***********************************************************************
 ******  E X T E R N A L   F U N C T I O N   D E C L A R A T I O N S  ******
 ***************************************************************************/

EXTERN_C uns32 avnd_comp_clc_fsm_run(struct avnd_cb_tag *, AVND_COMP *, 
                                     AVND_COMP_CLC_PRES_FSM_EV);

EXTERN_C uns32 avnd_comp_clc_fsm_trigger(struct avnd_cb_tag *, AVND_COMP *, 
                                         AVND_COMP_CLC_PRES_FSM_EV);

EXTERN_C uns32 avnd_comp_cbk_send (struct avnd_cb_tag *, AVND_COMP *,
                                   AVSV_AMF_CBK_TYPE, AVND_COMP_HC_REC *, 
                                   AVND_COMP_CSI_REC *);
EXTERN_C uns32 avnd_comp_clc_cmd_execute(struct avnd_cb_tag *, struct avnd_comp_tag *, 
                                       enum avnd_comp_clc_cmd_type);


EXTERN_C AVND_COMP_HC_REC *avnd_comp_hc_get (AVND_COMP *, uns32, uns32);
EXTERN_C uns32 avnd_dblist_hc_rec_cmp(uns8 *key1, uns8 *key2);
EXTERN_C void avnd_comp_hc_rec_del_all (struct avnd_cb_tag *, AVND_COMP *);

EXTERN_C void avnd_comp_cbq_del (struct avnd_cb_tag *, AVND_COMP *,NCS_BOOL);
EXTERN_C void avnd_comp_cbq_rec_pop_and_del (struct avnd_cb_tag *, AVND_COMP *, 
                                             AVND_COMP_CBK *, NCS_BOOL);
EXTERN_C AVND_COMP_CBK *avnd_comp_cbq_rec_add (struct avnd_cb_tag *, AVND_COMP *,
                                        AVSV_AMF_CBK_INFO *, MDS_DEST *, SaTimeT);
EXTERN_C uns32 avnd_comp_cbq_send (struct avnd_cb_tag *, AVND_COMP *, MDS_DEST *, 
                                   SaAmfHandleT, AVSV_AMF_CBK_INFO *, SaTimeT);

EXTERN_C void avnd_comp_cbq_rec_del (struct avnd_cb_tag *, AVND_COMP *, AVND_COMP_CBK *);

EXTERN_C uns32 avnd_comp_cbq_rec_send (struct avnd_cb_tag *, AVND_COMP *, AVND_COMP_CBK *, NCS_BOOL );

EXTERN_C uns32 avnd_compdb_init(struct avnd_cb_tag *);
EXTERN_C uns32 avnd_compdb_destroy(struct avnd_cb_tag *);
EXTERN_C AVND_COMP *avnd_compdb_rec_add(struct avnd_cb_tag *, AVND_COMP_PARAM *, uns32 *);
EXTERN_C uns32 avnd_compdb_rec_del(struct avnd_cb_tag *, SaNameT *);
EXTERN_C AVND_COMP_CSI_REC *avnd_compdb_csi_rec_get(struct avnd_cb_tag *, SaNameT *, SaNameT *);
EXTERN_C AVND_COMP_CSI_REC *avnd_compdb_csi_rec_get_next(struct avnd_cb_tag *, SaNameT *, SaNameT *);

EXTERN_C uns32 avnd_amf_resp_send(struct avnd_cb_tag  *, AVSV_AMF_API_TYPE, 
                                  SaAisErrorT, uns8 *, MDS_DEST *, 
                                  MDS_SYNC_SND_CTXT *, AVND_COMP *, NCS_BOOL);

EXTERN_C void avnd_comp_hc_finalize (struct avnd_cb_tag *, AVND_COMP *, 
                                     SaAmfHandleT, MDS_DEST *);
EXTERN_C void avnd_comp_cbq_finalize (struct avnd_cb_tag *, AVND_COMP *, 
                                      SaAmfHandleT, MDS_DEST *);

EXTERN_C void avnd_comp_cbq_csi_rec_del (struct avnd_cb_tag *, AVND_COMP *,
                                         SaNameT   *);

EXTERN_C uns32 avnd_comp_csi_remove(struct avnd_cb_tag *, AVND_COMP *, 
                                    AVND_COMP_CSI_REC *);

EXTERN_C uns32 avnd_comp_csi_assign(struct avnd_cb_tag *, AVND_COMP *, 
                                    AVND_COMP_CSI_REC *);

EXTERN_C uns32 avnd_comp_csi_reassign(struct avnd_cb_tag *, AVND_COMP *);

EXTERN_C uns32 avnd_comp_csi_assign_done (struct avnd_cb_tag *, AVND_COMP *, 
                                          AVND_COMP_CSI_REC *);

EXTERN_C uns32 avnd_comp_csi_remove_done (struct avnd_cb_tag *, AVND_COMP *, 
                                          AVND_COMP_CSI_REC *);

EXTERN_C uns32 avnd_comp_csi_qscd_assign_fail_prc (struct avnd_cb_tag *, 
                                                   AVND_COMP *, 
                                                   AVND_COMP_CSI_REC *);

EXTERN_C uns32 avnd_comp_curr_info_del(struct avnd_cb_tag *, AVND_COMP *);

EXTERN_C uns32 ncsscomptableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 ncsscomptableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 ncsscomptableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 ncsscomptableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 ncsscomptableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
EXTERN_C uns32 ncsscomptableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);

EXTERN_C uns32 saamfscompcsitableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 saamfscompcsitableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 saamfscompcsitableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfscompcsitableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 saamfscompcsitableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);

EXTERN_C uns32 avnd_comp_clc_cmd_execute(struct avnd_cb_tag *, AVND_COMP *,
                                AVND_COMP_CLC_CMD_TYPE );
EXTERN_C uns32 saamfscompcsitableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);


EXTERN_C uns32 avnd_srm_req_list_init( struct avnd_cb_tag * );
EXTERN_C uns32 avnd_srm_req_list_destroy(struct avnd_cb_tag *);
EXTERN_C struct avnd_srm_req_tag * avnd_srm_req_add (struct avnd_cb_tag *,
                                                 uns32, struct avnd_pm_rec *);
EXTERN_C uns32 avnd_srm_req_del(struct avnd_cb_tag*, uns32);
EXTERN_C struct avnd_srm_req_tag *avnd_srm_req_get(struct avnd_cb_tag*, uns32);
EXTERN_C uns32 avnd_srm_req_free(NCS_DB_LINK_LIST_NODE *);

EXTERN_C void avnd_pm_list_init(AVND_COMP *);
EXTERN_C uns32 avnd_pm_rec_free(NCS_DB_LINK_LIST_NODE *);
EXTERN_C uns32 avnd_comp_pm_rec_add(struct avnd_cb_tag *, AVND_COMP *, AVND_COMP_PM_REC *);
EXTERN_C void avnd_comp_pm_rec_del (struct avnd_cb_tag *, AVND_COMP *, AVND_COMP_PM_REC *);
EXTERN_C void avnd_comp_pm_rec_del_all (struct avnd_cb_tag *, AVND_COMP *);
EXTERN_C uns32 avnd_comp_pm_stop_process (struct avnd_cb_tag *, AVND_COMP *, AVSV_AMF_PM_STOP_PARAM  *, SaAisErrorT *);
EXTERN_C uns32 avnd_comp_pm_start_process (struct avnd_cb_tag *, AVND_COMP *, AVSV_AMF_PM_START_PARAM *, SaAisErrorT *);
EXTERN_C uns32 avnd_comp_pmstart_modify (struct avnd_cb_tag *, AVSV_AMF_PM_START_PARAM *, AVND_COMP_PM_REC *, SaAisErrorT *);
EXTERN_C AVND_COMP_PM_REC *avnd_comp_new_rsrc_mon (struct avnd_cb_tag *, AVND_COMP *, AVSV_AMF_PM_START_PARAM *, SaAisErrorT *);
EXTERN_C NCS_BOOL avnd_comp_pm_rec_cmp(AVSV_AMF_PM_START_PARAM *, AVND_COMP_PM_REC *);
EXTERN_C uns32 avnd_evt_ava_pm_start (struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_pm_stop (struct avnd_cb_tag *,  struct avnd_evt_tag *);
EXTERN_C void avnd_comp_pm_param_val(struct avnd_cb_tag *, AVSV_AMF_API_TYPE, uns8 *, AVND_COMP **, AVND_COMP_PM_REC **,
                            SaAisErrorT  *);
EXTERN_C void avnd_comp_pm_finalize(struct avnd_cb_tag *, AVND_COMP *, SaAmfHandleT);

EXTERN_C uns32 avnd_comp_am_start (struct avnd_cb_tag *, AVND_COMP *);
EXTERN_C uns32 avnd_comp_am_stop (struct avnd_cb_tag *, AVND_COMP *);
EXTERN_C uns32 avnd_comp_am_oper_req_process (struct avnd_cb_tag *, AVND_COMP *);
EXTERN_C uns32 avnd_comp_amstop_clc_res_process (struct avnd_cb_tag *, AVND_COMP *, NCS_OS_PROC_EXEC_STATUS );
EXTERN_C uns32 avnd_comp_amstart_clc_res_process (struct avnd_cb_tag *, AVND_COMP *, NCS_OS_PROC_EXEC_STATUS );
EXTERN_C void avnd_pxied_list_init(AVND_COMP *);
EXTERN_C uns32 avnd_comp_proxy_unreg(struct avnd_cb_tag *, AVND_COMP *);
EXTERN_C void avnd_comp_unreg_cbk_process(struct avnd_cb_tag *, AVND_COMP *);
EXTERN_C void avnd_comp_cmplete_all_assignment(struct avnd_cb_tag *, AVND_COMP *);
EXTERN_C void avnd_comp_cmplete_all_csi_rec(struct avnd_cb_tag *, AVND_COMP *);
EXTERN_C uns32 avnd_comp_hc_rec_start (struct avnd_cb_tag *, AVND_COMP *, AVND_COMP_HC_REC *);
EXTERN_C uns32 avnd_comp_unreg_prc(struct avnd_cb_tag *, AVND_COMP *, AVND_COMP *);
EXTERN_C uns32 avnd_mbcsv_comp_hc_rec_tmr_exp (struct avnd_cb_tag *cb,
                                               AVND_COMP   *comp,
                                               AVND_COMP_HC_REC *rec);
EXTERN_C AVND_COMP_HC_REC *avnd_mbcsv_comp_hc_rec_add (struct avnd_cb_tag  *cb,
                                        AVND_COMP               *comp,
                                        AVSV_AMF_HC_START_PARAM *hc_start,
                                        MDS_DEST                *dest);
EXTERN_C void avnd_mbcsv_comp_hc_rec_del (struct avnd_cb_tag *cb, AVND_COMP *comp,
                                          AVND_COMP_HC_REC *rec);

#endif /* !AVND_COMP_H */
