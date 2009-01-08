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

  DESCRIPTION: This module contains the array of function pointers, indexed by
  the events for both active and standby AvD. It also has the processing
  function that calls into one of these function pointers based on the event.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_invalid_func - invalid events handler.
  avd_standby_invalid_func - invalid events handler on standby AvD.
  avd_main_proc - main loop for both the active and standby AvDs.


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"

static void avd_process_event(AVD_CL_CB *cb_now, AVD_EVT *evt);
static void avd_invalid_func(AVD_CL_CB *cb,AVD_EVT *evt);
static void avd_standby_invalid_func(AVD_CL_CB *cb,AVD_EVT *evt);
static void avd_qsd_invalid_func(AVD_CL_CB *cb,AVD_EVT *evt);
static void avd_qsd_ignore_func(AVD_CL_CB *cb,AVD_EVT *evt);
static void avd_restart(AVD_CL_CB *cb,AVD_EVT *evt)
{
   /* This function will be defined once we finalise on the shutdown seq */
}



/* list of all the function pointers related to handling the events
 * for active AvD.
 */

static const AVD_EVT_HDLR  g_avd_actv_list[AVD_EVT_MAX] =
{
   /* invalid event */ 
   avd_invalid_func,  /* AVD_EVT_INVALID */
   
   /* active AvD message events processing */
   avd_node_up_func,                /* AVD_EVT_NODE_UP_MSG */
   avd_reg_hlth_func,               /* AVD_EVT_REG_HLT_MSG */
   avd_reg_su_func,                 /* AVD_EVT_REG_SU_MSG */
   avd_reg_comp_func,               /* AVD_EVT_REG_COMP_MSG */
   avd_nd_heartbeat_msg_func,       /* AVD_EVT_HEARTBEAT_MSG */
   avd_su_oper_state_func,          /* AVD_EVT_OPERATION_STATE_MSG */
   avd_su_si_assign_func,           /* AVD_EVT_INFO_SU_SI_ASSIGN_MSG */
   avd_pg_trk_act_func,             /* AVD_EVT_PG_TRACK_ACT_MSG */
   avd_oper_req_func,               /* AVD_EVT_OPERATION_REQUEST_MSG */
   avd_data_update_req_func,        /* AVD_EVT_DATA_REQUEST_MSG */
   avd_shutdown_app_su_resp_func,   /* AVD_EVT_SHUTDOWN_APP_SU_MSG */
   avd_ack_nack_event,              /* AVD_EVT_VERIFY_ACK_NACK_MSG */
   avd_comp_validation_func,       /* AVD_EVT_COMP_VALIDATION_MSG */

   /* active AvD timer events processing */
   avd_tmr_snd_hb_func,       /* AVD_EVT_TMR_SND_HB */
   avd_tmr_rcv_hb_d_func,     /* AVD_EVT_TMR_RCV_HB_D */
   avd_tmr_rcv_hb_nd_func,    /* AVD_EVT_TMR_RCV_HB_ND */
   avd_tmr_rcv_hb_init_func,  /* AVD_EVT_TMR_RCV_HB_INIT */
   avd_tmr_cl_init_func,      /* AVD_EVT_TMR_CL_INIT */
   avd_tmr_cfg_exp_func,      /* AVD_EVT_TMR_CFG */
   avd_tmr_si_dep_tol_func,   /* AVD_EVT_TMR_SI_DEP_TOL */
   /* active AvD MIB events processing */
   avd_req_mib_func,    /* AVD_EVT_MIB_REQ */

   /* active AvD MDS events processing */
   avd_mds_avd_up_func,    /* AVD_EVT_MDS_AVD_UP */
   avd_mds_avd_down_func,  /* AVD_EVT_MDS_AVD_DOWN */
   avd_mds_avnd_up_func,   /* AVD_EVT_MDS_AVND_UP */
   avd_mds_avnd_down_func,  /* AVD_EVT_MDS_AVND_DOWN */
   avd_mds_qsd_role_func,  /* AVD_EVT_MDS_QSD_ACK*/ 

   /* active AvD BAM events processing */
   avd_bam_cfg_done_func,     /* AVD_EVT_INIT_CFG_DONE_MSG */
   avd_restart,               /* AVD_EVT_RESTART */

   avd_avm_nd_shutdown_func,  /* AVD_EVT_ND_SHUTDOWN */
   avd_avm_nd_failover_func,  /* AVD_EVT_ND_FAILOVER */
   avd_avm_fault_domain_rsp,  /* AVD_EVT_FAULT_DMN_RSP */
   avd_avm_nd_reset_rsp_func, /* AVD_EVT_ND_RESET_RSP */
   avd_avm_nd_oper_st_func,   /* AVD_EVT_ND_OPER_ST */

      /* Role change Event processing */
   avd_role_change,          /* AVD_EVT_ROLE_CHANGE */

   avd_role_switch_ncs_su,   /* AVD_EVT_SWITCH_NCS_SU */
   avd_rcv_hb_d_msg,         /*  AVD_EVT_D_HB */
   avd_process_si_dep_state_evt
};

/* list of all the function pointers related to handling the events
 * for standby AvD.
 */

static const AVD_EVT_HDLR  g_avd_stndby_list[AVD_EVT_MAX] =
{
   /* invalid event */ 
   avd_invalid_func,  /* AVD_EVT_INVALID */
   
   /* standby AvD message events processing */
   avd_standby_invalid_func,   /* AVD_EVT_NODE_UP_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_REG_HLT_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_REG_SU_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_REG_COMP_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_HEARTBEAT_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_OPERATION_STATE_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_INFO_SU_SI_ASSIGN_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_PG_TRACK_ACT_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_OPERATION_REQUEST_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_DATA_REQUEST_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_SHUTDOWN_APP_SU_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_VERIFY_ACK_NACK_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_COMP_VALIDATION_MSG */

   /* standby AvD timer events processing */
   avd_tmr_snd_hb_func,           /* AVD_EVT_TMR_SND_HB */
   avd_standby_tmr_rcv_hb_d_func, /* AVD_EVT_TMR_RCV_HB_D */
   avd_standby_invalid_func,      /* AVD_EVT_TMR_RCV_HB_ND */
   avd_tmr_rcv_hb_init_func,      /* AVD_EVT_TMR_RCV_HB_INIT */
   avd_standby_invalid_func,      /* AVD_EVT_TMR_CL_INIT */
   avd_standby_invalid_func,      /* AVD_EVT_TMR_CFG */
   avd_standby_invalid_func,   /* AVD_EVT_TMR_SI_DEP_TOL */
   /* standby AvD MIB events processing */
   avd_standby_invalid_func,   /* AVD_EVT_MIB_REQ */

   /* standby AvD MDS events processing */
   avd_mds_avd_up_func,        /* AVD_EVT_MDS_AVD_UP */
   avd_standby_avd_down_func,  /* AVD_EVT_MDS_AVD_DOWN */
   avd_mds_avnd_up_func,       /* AVD_EVT_MDS_AVND_UP */
   avd_mds_avnd_down_func,     /* AVD_EVT_MDS_AVND_DOWN */
   avd_standby_invalid_func,   /* AVD_EVT_MDS_QSD_ACK*/ 

   avd_standby_invalid_func,   /* AVD_EVT_INIT_CFG_DONE_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_RESTART */

   avd_standby_invalid_func,   /* AVD_EVT_ND_SHUTDOWN */
   avd_standby_invalid_func,   /* AVD_EVT_ND_FAILOVER */  
   avd_standby_invalid_func,   /* AVD_EVT_FAULT_DMN_RSP */
   avd_standby_invalid_func,   /* AVD_EVT_ND_RESET_RSP */
   avd_standby_invalid_func,   /* AVD_EVT_ND_OPER_ST */

   /* Role change Event processing */
   avd_role_change,             /* AVD_EVT_ROLE_CHANGE */
      
   avd_standby_invalid_func,    /* AVD_EVT_SWITCH_NCS_SU */
   avd_rcv_hb_d_msg,            /*  AVD_EVT_D_HB */
   avd_standby_invalid_func     /* AVD_EVT_SI_DEP_STATE */
};


/* list of all the function pointers related to handling the events
 * for No role AvD.
 */
static const AVD_EVT_HDLR  g_avd_init_list[AVD_EVT_MAX] =
{
   /* invalid event */
   avd_invalid_func,  /* AVD_EVT_INVALID */

   /* standby AvD message events processing */
   avd_standby_invalid_func,   /* AVD_EVT_NODE_UP_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_REG_HLT_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_REG_SU_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_REG_COMP_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_HEARTBEAT_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_OPERATION_STATE_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_INFO_SU_SI_ASSIGN_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_PG_TRACK_ACT_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_OPERATION_REQUEST_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_DATA_REQUEST_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_SHUTDOWN_APP_SU_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_VERIFY_ACK_NACK_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_COMP_VALIDATION_MSG */

   /* standby AvD timer events processing */
   avd_standby_invalid_func,   /* AVD_EVT_TMR_SND_HB */
   avd_standby_invalid_func,   /* AVD_EVT_TMR_RCV_HB_D */
   avd_standby_invalid_func,   /* AVD_EVT_TMR_RCV_HB_ND */
   avd_standby_invalid_func,   /* AVD_EVT_TMR_RCV_HB_INIT */
   avd_standby_invalid_func,   /* AVD_EVT_TMR_CL_INIT */
   avd_standby_invalid_func,   /* AVD_EVT_TMR_CFG */
   avd_standby_invalid_func,   /* AVD_EVT_TMR_SI_DEP_TOL */

   /* standby AvD MIB events processing */
   avd_standby_invalid_func,   /* AVD_EVT_MIB_REQ */

   /* standby AvD MDS events processing */
   avd_standby_invalid_func,   /* AVD_EVT_MDS_AVD_UP */
   avd_standby_invalid_func,   /* AVD_EVT_MDS_AVD_DOWN */
   avd_standby_invalid_func,   /* AVD_EVT_MDS_AVND_UP */
   avd_standby_invalid_func,   /* AVD_EVT_MDS_AVND_DOWN */
   avd_standby_invalid_func,   /* AVD_EVT_MDS_QSD_ACK */
   avd_standby_invalid_func,   /* AVD_EVT_INIT_CFG_DONE_MSG */
   avd_standby_invalid_func,   /* AVD_EVT_RESTART */

   avd_standby_invalid_func,   /* AVD_EVT_ND_SHUTDOWN */
   avd_standby_invalid_func,   /* AVD_EVT_ND_FAILOVER */
   avd_standby_invalid_func,   /* AVD_EVT_FAULT_DMN_RSP */
   avd_standby_invalid_func,   /* AVD_EVT_ND_RESET_RSP */
   avd_standby_invalid_func,   /* AVD_EVT_ND_OPER_ST */

   /* Role change Event processing */
   avd_role_change,             /* AVD_EVT_ROLE_CHANGE */

   avd_standby_invalid_func,    /* AVD_EVT_SWITCH_NCS_SU */
   avd_standby_invalid_func,    /* AVD_EVT_D_HB */
   avd_standby_invalid_func     /* AVD_EVT_SI_DEP_STATE */
};

/* list of all the function pointers related to handling the events
 * for quiesced AvD.
 */

static const AVD_EVT_HDLR  g_avd_quiesc_list[AVD_EVT_MAX] =
{
   /* invalid event */ 
   avd_invalid_func,  /* AVD_EVT_INVALID */
   
   /* active AvD message events processing */
   avd_qsd_ignore_func,             /* AVD_EVT_NODE_UP_MSG */
   avd_reg_hlth_func,               /* AVD_EVT_REG_HLT_MSG */
   avd_reg_su_func,                 /* AVD_EVT_REG_SU_MSG */
   avd_reg_comp_func,               /* AVD_EVT_REG_COMP_MSG */
   avd_nd_heartbeat_msg_func,       /* AVD_EVT_HEARTBEAT_MSG */
   avd_su_oper_state_func,          /* AVD_EVT_OPERATION_STATE_MSG */
   avd_su_si_assign_func,           /* AVD_EVT_INFO_SU_SI_ASSIGN_MSG */
   avd_qsd_ignore_func,             /* AVD_EVT_PG_TRACK_ACT_MSG */
   avd_oper_req_func,               /* AVD_EVT_OPERATION_REQUEST_MSG */
   avd_data_update_req_func,        /* AVD_EVT_DATA_REQUEST_MSG */
   avd_shutdown_app_su_resp_func,   /* AVD_EVT_SHUTDOWN_APP_SU_MSG */
   avd_qsd_invalid_func,            /* AVD_EVT_VERIFY_ACK_NACK_MSG */
   avd_comp_validation_func,       /* AVD_EVT_COMP_VALIDATION_MSG */

   /* active AvD timer events processing */
   avd_tmr_snd_hb_func,  /* AVD_EVT_TMR_SND_HB */
   avd_tmr_rcv_hb_d_func,/* AVD_EVT_TMR_RCV_HB_D */
   avd_qsd_ignore_func,  /* AVD_EVT_TMR_RCV_HB_ND */
   avd_qsd_ignore_func,  /* AVD_EVT_TMR_RCV_HB_INIT */
   avd_qsd_ignore_func,  /* AVD_EVT_TMR_CL_INIT */
   avd_qsd_ignore_func,  /* AVD_EVT_TMR_CFG */
   avd_qsd_ignore_func,   /* AVD_EVT_TMR_SI_DEP_TOL */

   /* active AvD MIB events processing */
   avd_qsd_req_mib_func, /* AVD_EVT_MIB_REQ */

   /* active AvD MDS events processing */
   avd_mds_avd_up_func,    /* AVD_EVT_MDS_AVD_UP */
   avd_mds_avd_down_func,  /* AVD_EVT_MDS_AVD_DOWN */
   avd_mds_avnd_up_func,   /* AVD_EVT_MDS_AVND_UP */
   avd_mds_avnd_down_func, /* AVD_EVT_MDS_AVND_DOWN */
   avd_mds_qsd_role_func,  /* AVD_EVT_MDS_QSD_ACK*/ 

   /* active AvD BAM events processing */
   avd_qsd_invalid_func,    /* AVD_EVT_INIT_CFG_DONE_MSG */
   avd_restart,             /* AVD_EVT_RESTART */

   avd_qsd_invalid_func,  /* AVD_EVT_ND_SHUTDOWN */
   avd_qsd_invalid_func,  /* AVD_EVT_ND_FAILOVER */
   avd_qsd_invalid_func,  /* AVD_EVT_FAULT_DMN_RSP */
   avd_qsd_invalid_func,  /* AVD_EVT_ND_RESET_RSP */
   avd_qsd_invalid_func,  /* AVD_EVT_ND_OPER_ST */

      /* Role change Event processing */
   avd_role_change,          /* AVD_EVT_ROLE_CHANGE */
   avd_qsd_invalid_func,     /* AVD_EVT_SWITCH_NCS_SU */
   avd_rcv_hb_d_msg,         /*  AVD_EVT_D_HB */
   avd_qsd_invalid_func      /* AVD_EVT_TMR_SI_DEP_TOL */
};


/*****************************************************************************
 * Function: avd_invalid_func
 *
 * Purpose:  This function is the handler for invalid events. It just
 * dumps the event to the debug log and returns.
 *
 * Input: cb - the AVD control block
 *        evt - The event information.
 *
 * Returns: None.
 *
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static void avd_invalid_func(AVD_CL_CB *cb,AVD_EVT *evt)
{

   /* This function should never be called
    * log the event to the debug log and return
    */
   
   /* we need not send sync update to stanby */
   cb->sync_required = FALSE;

   m_AVD_LOG_INVALID_VAL_FATAL(0);
   return;
}


/*****************************************************************************
 * Function: avd_standby_invalid_func
 *
 * Purpose:  This function is the handler for invalid events in standby state.
 * This function might be called during system trauma. It just dumps the
 * event to the debug log at a information level and returns.
 *
 *
 * Input: cb - the AVD control block
 *        evt - The event information.
 *
 * Returns: None.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static void avd_standby_invalid_func(AVD_CL_CB *cb,AVD_EVT *evt)
{

   /* This function should generally never be called
    * log the event to the debug log at information level and return
    */
   AVD_DND_MSG *n2d_msg;

   m_AVD_LOG_FUNC_ENTRY("avd_standby_invalid_func");
   m_AVD_LOG_INVALID_VAL_ERROR(0);

   if((evt->rcv_evt >= AVD_EVT_NODE_UP_MSG) && (evt->rcv_evt <= AVD_EVT_VERIFY_ACK_NACK_MSG))
   {
      if (evt->info.avnd_msg == NULL)
      {
         /* log error that a message contents is missing */
         m_AVD_LOG_INVALID_VAL_ERROR(0);
         return;
      }

      n2d_msg = evt->info.avnd_msg;
      m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
   }
   
}


/*****************************************************************************
 * Function: avd_quiesced_invalid_func
 *
 * Purpose:  This function is the handler for invalid events in quiesced state.
 * This function might be called during system trauma. It just dumps the
 * event to the debug log at a information level and returns.
 *
 *
 * Input: cb - the AVD control block
 *        evt - The event information.
 *
 * Returns: None.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static void avd_qsd_invalid_func(AVD_CL_CB *cb,AVD_EVT *evt)
{

   /* This function should generally never be called
    * log the event to the debug log at information level and return
    */
    AVD_DND_MSG *n2d_msg;

   m_AVD_LOG_FUNC_ENTRY("avd_qsd_invalid_func");
   m_AVD_LOG_INVALID_VAL_ERROR(0);

   /* we need not send sync update to stanby */
   cb->sync_required = FALSE;

   if((evt->rcv_evt >= AVD_EVT_NODE_UP_MSG) && (evt->rcv_evt <= AVD_EVT_VERIFY_ACK_NACK_MSG))
   {
       if (evt->info.avnd_msg == NULL)
      {
         /* log error that a message contents is missing */
         m_AVD_LOG_INVALID_VAL_ERROR(0);
         return;
      }

      n2d_msg = evt->info.avnd_msg;
      m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
   }
   
}

/*****************************************************************************
 * Function: avd_qsd_ignore_func 
 *
 * Purpose:  This function is the handler for events in quiesced state which
 * which are to be ignored.
 *
 *
 * Input: cb - the AVD control block
 *        evt - The event information.
 *
 * Returns: None.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static void avd_qsd_ignore_func(AVD_CL_CB *cb,AVD_EVT *evt)
{
   /* Ignore this Event. Free this msg */ 
   AVD_DND_MSG *n2d_msg;

   m_AVD_LOG_FUNC_ENTRY("avd_qsd_ignore_func");
   m_AVD_LOG_RCVD_VAL(((long)evt));
   m_AVD_LOG_RCVD_NAME_VAL(evt,sizeof(AVD_EVT));

   /* we need not send sync update to stanby */
   cb->sync_required = FALSE;

   if((evt->rcv_evt >= AVD_EVT_NODE_UP_MSG) && (evt->rcv_evt <= AVD_EVT_VERIFY_ACK_NACK_MSG))
   {
        if (evt->info.avnd_msg == NULL)
      {
         /* log error that a message contents is missing */
         m_AVD_LOG_INVALID_VAL_ERROR(0);
         return;
      }

      n2d_msg = evt->info.avnd_msg;
      m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
   }

}



/*****************************************************************************
 * Function: avd_main_proc
 *
 * Purpose: This is the main infinite loop in which both the active and
 * standby AvDs execute waiting for events to happen. When woken up
 * due to an event, based on the HA state it moves to either the active
 * or standby processing modules. Even in Init state the same arrays are used.
 *
 * Input: cb - AVD control block
 *
 * Returns: NONE.
 *
 * NOTES: This function will never return execept in case of init errors.
 *
 * 
 **************************************************************************/

void avd_main_proc(AVD_CL_CB *cb)
{
   uns32 cb_hdl;
   NCS_SEL_OBJ_SET sel_obj_set;
   NCS_SEL_OBJ     sel_high,mbx_sel_obj,mbcsv_sel_obj;
   AVD_CL_CB       *cb_now;
   AVD_EVT         *evt;
   uns32           msg;

   USE(msg);
   cb_hdl = cb->cb_handle;
   mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(&cb->avd_mbx);
   sel_high = cb->sel_high;
   sel_obj_set = cb->sel_obj_set;
   m_SET_FD_IN_SEL_OBJ(cb->mbcsv_sel_obj, mbcsv_sel_obj);

   m_AVD_LOG_FUNC_ENTRY("avd_main_proc");
 
   /* start of the infinite loop */

   /* Do a wait(select) for one of the different modules to send
    * a message to work
    */
   while (m_NCS_SEL_OBJ_SELECT(sel_high, &sel_obj_set, NULL, NULL, NULL) != -1 )
   {
      /* get the CB from the handle manager */
      if ((cb_now = ncshm_take_hdl(NCS_SERVICE_ID_AVD,cb_hdl)) ==
        AVD_CL_CB_NULL)
      {
         /* log the problem */
         m_AVD_LOG_INVALID_VAL_FATAL(cb_hdl);
         continue;
      }
      
      m_AVD_LOG_RCVD_VAL((long)cb_now);

      if (m_NCS_SEL_OBJ_ISSET (mbx_sel_obj, &sel_obj_set))
      {
         evt = (AVD_EVT *)m_NCS_IPC_NON_BLK_RECEIVE(&cb_now->avd_mbx,msg);

         if ((cb_now->avd_fover_state) && (evt != AVD_EVT_NULL) &&
             (evt->rcv_evt > AVD_EVT_INVALID) && (evt->rcv_evt < AVD_EVT_MAX) && (evt->cb_hdl == cb_now->cb_handle))
         {
            /* 
             * We are in fail-over, so process only 
             * verify ack/nack and timer expiry events. Enqueue
             * all other events.
             */
            if ((evt->rcv_evt == AVD_EVT_TMR_SND_HB) ||
                (evt->rcv_evt == AVD_EVT_HEARTBEAT_MSG) ||
                (evt->rcv_evt == AVD_EVT_VERIFY_ACK_NACK_MSG))
            {
               /* Process the event */
               avd_process_event(cb_now, evt);
            }
            else
            {
               AVD_EVT_QUEUE  *queue_evt;
               /* Enqueue this event */
               if (NULL != (queue_evt = m_MMGR_ALLOC_EVT_ENTRY))
               {
                  m_NCS_MEMSET(queue_evt, '\0', sizeof(AVD_EVT_QUEUE));
                  queue_evt->evt = evt;
                  m_AVD_EVT_QUEUE_ENQUEUE(cb_now, queue_evt);
               }
               else
               {
                  /* Log Error */
               }

               /* If it is Receive HB timer event then remove the node
                * from the node list */
               if ((evt->rcv_evt == AVD_EVT_TMR_RCV_HB_ND) || (evt->rcv_evt == AVD_EVT_TMR_RCV_HB_D))
               {
                  AVD_FAIL_OVER_NODE  *node_fovr;

                  if (NULL != (node_fovr = 
                     (AVD_FAIL_OVER_NODE*)ncs_patricia_tree_get(&cb->node_list,
                     (uns8*)&evt->info.tmr.node_id)))
                  {
                     ncs_patricia_tree_del(&cb->node_list,
                        &node_fovr->tree_node_id_node);
                     m_MMGR_FREE_CLM_NODE_ID(node_fovr);
                  }
               }
            }

            if (cb_now->node_list.n_nodes == 0)
            {
               AVD_EVT_QUEUE  *queue_evt;
               
               /* We have received the info from all the nodes.*/
               cb_now->avd_fover_state = FALSE;
               
               /* Dequeue, all the messages from the queue
               and process them now */
               m_AVD_EVT_QUEUE_DEQUEUE(cb_now, queue_evt);
               
               while (NULL != queue_evt)
               {
                  avd_process_event(cb_now, queue_evt->evt);
                  m_MMGR_FREE_EVT_ENTRY(queue_evt);
                  m_AVD_EVT_QUEUE_DEQUEUE(cb_now, queue_evt);
               }
            }
         }
         else if ((FALSE == cb_now->avd_fover_state) && (evt != AVD_EVT_NULL) && 
                  (evt->rcv_evt > AVD_EVT_INVALID) && (evt->rcv_evt < AVD_EVT_MAX) && (evt->cb_hdl == cb_now->cb_handle))
         {
            avd_process_event(cb_now, evt);
         }
         else if(evt == AVD_EVT_NULL)
         {
            /* log fatal error that a NULL event was returned. */
            m_AVD_LOG_INVALID_VAL_FATAL(0);
         }
         else
         {
            /* log fatal error that invalid event id was received
             */
            m_AVD_LOG_RCVD_VAL(((long)evt));
            m_AVD_LOG_RCVD_NAME_VAL(evt,sizeof(AVD_EVT));
            m_AVD_LOG_EVT_INVAL(evt->rcv_evt);
            m_MMGR_FREE_AVD_EVT(evt);
         }

      } /* if (m_NCS_SEL_OBJ_ISSET (mbx_sel_obj, &sel_obj_set)) */

      if (m_NCS_SEL_OBJ_ISSET (mbcsv_sel_obj, &sel_obj_set))
      {
         if (NCSCC_RC_SUCCESS != avsv_mbcsv_dispatch(cb, SA_DISPATCH_ALL))
         {
            /* 
             * Log error; but continue with our loop.
             */
            m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DISPATCH_FAILURE, 
            NCSFL_SEV_ERROR, NCSCC_RC_FAILURE);
         }
      }/* if (m_NCS_SEL_OBJ_ISSET (mbcsv_sel_obj, &sel_obj_set))*/

      /* reinitialize the selection object list and the highest
       * selection object value from the CB
       */
      sel_high = cb_now->sel_high;
      sel_obj_set = cb_now->sel_obj_set;

      /* give back the handle */
      ncshm_give_hdl(cb_hdl);

   } /* end of the infinite loop */

  m_AVD_LOG_INVALID_VAL_FATAL(0);
  m_NCS_SYSLOG(NCS_LOG_CRIT,"NCS_AvSv: Avd-Functional Thread Failed");

  sleep(3);/*Let the DTSV log be Printed */
  exit(0);

} /* avd_main_proc */

/*****************************************************************************
 * Function: avd_process_event 
 *
 * Purpose: This function executes the event handler for the current AVD
 *           state. This function will be used in the main AVD thread.
 *
 * Input: cb  - AVD control block
 *        evt - ptr to AVD_EVT got from mailbox 
 *
 * Returns: NONE.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static void avd_process_event(AVD_CL_CB *cb_now, AVD_EVT *evt)
{
   m_AVD_LOG_RCVD_VAL(((long)evt));
   m_AVD_LOG_RCVD_NAME_VAL(evt,sizeof(AVD_EVT));
   m_AVD_LOG_EVT_INFO(AVD_RCVD_EVENT,evt->rcv_evt);

   /* check the HA state */
   if (cb_now->role_set == FALSE)
      g_avd_init_list[evt->rcv_evt](cb_now,evt);
   else if (cb_now->avail_state_avd == SA_AMF_HA_ACTIVE)
   {
      /* if active call g_avd_actv_list functions*/
      g_avd_actv_list[evt->rcv_evt](cb_now,evt);
      
      /*
      * Just processed the event.
      * Time to send sync send the standby and then
      * all the AVND messages we have queued in our queue.
      */
      if(cb_now->sync_required == TRUE)
         m_AVSV_SEND_CKPT_UPDT_SYNC(cb_now, NCS_MBCSV_ACT_UPDATE, 0);
      
      avd_d2n_msg_dequeue(cb_now);
   }
   else if (cb_now->avail_state_avd == SA_AMF_HA_STANDBY)
   {
      /* if standby call g_avd_stndby_list functions */
      g_avd_stndby_list[evt->rcv_evt](cb_now,evt);
   }
   else if (cb_now->avail_state_avd == SA_AMF_HA_QUIESCED)
   {
      /* if quiesced call g_avd_quiesc_list functions */
      g_avd_quiesc_list[evt->rcv_evt](cb_now,evt);
      /*
      * Just processed the event.
      * Time to send sync send the standby and then
      * all the AVND messages we have queued in our queue.
      */
      if (cb_now->sync_required == TRUE) 
         m_AVSV_SEND_CKPT_UPDT_SYNC(cb_now, NCS_MBCSV_ACT_UPDATE, 0);
      
      avd_d2n_msg_dequeue(cb_now);

   }
   else
   {
      /* log error that the CB is in invalid HA state. */
      m_AVD_LOG_INVALID_VAL_FATAL(cb_now->avail_state_avd);
   }

   /* reset the sync falg */
   cb_now->sync_required = TRUE;
  
   m_MMGR_FREE_AVD_EVT(evt);

}


/*****************************************************************************
 * Function: avd_process_hb_event 
 *
 * Purpose: This function executes the event handler for the current AVD
 *           state. This function will be used in the avd_hb thread only.
 *
 * Input: cb  - AVD control block
 *        evt - ptr to AVD_EVT got from mailbox 
 *
 * Returns: NONE.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

void avd_process_hb_event(AVD_CL_CB *cb_now, AVD_EVT *evt)
{
   m_AVD_LOG_RCVD_VAL(((long)evt));
   m_AVD_LOG_RCVD_NAME_VAL(evt,sizeof(AVD_EVT));
   m_AVD_LOG_EVT_INFO(AVD_RCVD_EVENT,evt->rcv_evt);

   /* check the HA state */
   if (cb_now->role_set == FALSE)
      g_avd_init_list[evt->rcv_evt](cb_now,evt);
   else if (cb_now->avail_state_avd == SA_AMF_HA_ACTIVE)
   {
      /* if active call g_avd_actv_list functions*/
      g_avd_actv_list[evt->rcv_evt](cb_now,evt);
      
   }
   else if (cb_now->avail_state_avd == SA_AMF_HA_STANDBY)
   {
      /* if standby call g_avd_stndby_list functions */
      g_avd_stndby_list[evt->rcv_evt](cb_now,evt);
   }
   else if (cb_now->avail_state_avd == SA_AMF_HA_QUIESCED)
   {
      /* if quiesced call g_avd_quiesc_list functions */
      g_avd_quiesc_list[evt->rcv_evt](cb_now,evt);

   }
   else
   {
      /* log error that the CB is in invalid HA state. */
      m_AVD_LOG_INVALID_VAL_FATAL(cb_now->avail_state_avd);
   }

   m_MMGR_FREE_AVD_EVT(evt);

}

