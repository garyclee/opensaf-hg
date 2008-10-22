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

  DESCRIPTION:This module contains the array of function pointers, indexed by
  the events for the AvND. It also has the processing
  function that calls into one of these function pointers based on the event.

 
..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <sched.h>
#include "avnd.h"


static NCS_BOOL avnd_mbx_process(SYSF_MBX *);
static void avnd_evt_process (AVND_EVT *);

static uns32 avnd_evt_invalid_func(AVND_CB *cb,AVND_EVT *evt);


/* list of all the function pointers related to handling the events
*/
static const AVND_EVT_HDLR  g_avnd_func_list[AVND_EVT_MAX] =
{
   /* invalid event */
   avnd_evt_invalid_func, /* AVND_EVT_INVALID */

   /* AvD message event types */
   avnd_evt_avd_node_update_msg,        /* AVND_EVT_AVD_NODE_UPDATE_MSG */
   avnd_evt_avd_node_up_msg,            /* AVND_EVT_AVD_NODE_UP_MSG */
   avnd_evt_avd_reg_hlt_msg,            /* AVND_EVT_AVD_REG_HLT_MSG */
   avnd_evt_avd_reg_su_msg,             /* AVND_EVT_AVD_REG_SU_MSG */
   avnd_evt_avd_reg_comp_msg,           /* AVND_EVT_AVD_REG_COMP_MSG */
   avnd_evt_avd_info_su_si_assign_msg,  /* AVND_EVT_AVD_INFO_SU_SI_ASSIGN_MSG */
   avnd_evt_avd_clm_node_on_fover,      /* AVND_EVT_AVD_NODE_ON_FOVER */
   avnd_evt_avd_pg_track_act_rsp_msg,   /* AVND_EVT_AVD_PG_TRACK_ACT_RSP_MSG */
   avnd_evt_avd_pg_upd_msg,             /* AVND_EVT_AVD_PG_UPD_MSG */
   avnd_evt_avd_operation_request_msg,  /* AVND_EVT_AVD_OPERATION_REQUEST_MSG */
   avnd_evt_avd_hb_info_msg,            /* AVND_EVT_AVD_HB_INFO_MSG */
   avnd_evt_avd_su_pres_msg,            /* AVND_EVT_AVD_SU_PRES_MSG */
   avnd_evt_avd_verify_message,         /* AVND_EVT_AVD_VERIFY_MSG */
   avnd_evt_avd_ack_message,            /* AVND_EVT_AVD_ACK_MSG */
   avnd_evt_avd_shutdown_app_su_msg,    /* AVND_EVT_AVD_SHUTDOWN_APP_SU_MSG */
   avnd_evt_avd_set_leds_msg,           /* AVND_EVT_AVD_SET_LEDS_MSG */

   /* AvA event types */ 
   avnd_evt_ava_finalize,            /* AVND_EVT_AVA_AMF_FINALIZE */
   avnd_evt_ava_comp_reg,            /* AVND_EVT_AVA_COMP_REG */
   avnd_evt_ava_comp_unreg,          /* AVND_EVT_AVA_COMP_UNREG */
   avnd_evt_ava_pm_start,            /* AVND_EVT_AVA_PM_START */
   avnd_evt_ava_pm_stop,             /* AVND_EVT_AVA_PM_STOP */
   avnd_evt_ava_hc_start,            /* AVND_EVT_AVA_HC_START */
   avnd_evt_ava_hc_stop,             /* AVND_EVT_AVA_HC_STOP */
   avnd_evt_ava_hc_confirm,          /* AVND_EVT_AVA_HC_CONFIRM */
   avnd_evt_ava_csi_quiescing_compl, /* AVND_EVT_AVA_CSI_QUIESCING_COMPL */
   avnd_evt_ava_ha_get,              /* AVND_EVT_AVA_HA_GET */
   avnd_evt_ava_pg_start,            /* AVND_EVT_AVA_PG_START */
   avnd_evt_ava_pg_stop,             /* AVND_EVT_AVA_PG_STOP */
   avnd_evt_ava_err_rep,             /* AVND_EVT_AVA_ERR_REP */
   avnd_evt_ava_err_clear,           /* AVND_EVT_AVA_ERR_CLEAR */
   avnd_evt_ava_resp,                /* AVND_EVT_AVA_RESP */

   /* CLA event types */ 
   avnd_evt_cla_finalize,       /* AVND_EVT_CLA_FINALIZE */
   avnd_evt_cla_track_start,    /* AVND_EVT_CLA_TRACK_START */
   avnd_evt_cla_track_stop,     /* AVND_EVT_CLA_TRACK_STOP */
   avnd_evt_cla_node_get,       /* AVND_EVT_CLA_NODE_GET */
   avnd_evt_cla_node_async_get, /* AVND_EVT_CLA_NODE_ASYNC_GET */

   /* timer event types */ 
   avnd_evt_tmr_hc,           /* AVND_EVT_TMR_HC */
   avnd_evt_tmr_cbk_resp,     /* AVND_EVT_TMR_CBK_RESP */
   avnd_evt_tmr_snd_hb,       /* AVND_EVT_TMR_SND_HB */
   avnd_evt_tmr_rcv_msg_rsp,  /* AVND_EVT_TMR_RCV_MSG_RSP */
   avnd_evt_tmr_clc_comp_reg, /* AVND_EVT_TMR_CLC_COMP_REG */
   avnd_evt_tmr_su_err_esc,   /* AVND_EVT_TMR_SU_ERR_ESC */
   avnd_evt_tmr_node_err_esc, /* AVND_EVT_TMR_NODE_ERR_ESC */
   avnd_evt_tmr_clc_pxied_comp_inst, /* AVND_EVT_TMR_CLC_PXIED_COMP_INST */
   avnd_evt_tmr_clc_pxied_comp_reg, /* AVND_EVT_TMR_CLC_PXIED_COMP_REG */

   /* mds event types */ 
   avnd_evt_mds_avd_up,  /* AVND_EVT_MDS_AVD_UP */
   avnd_evt_mds_avd_dn,  /* AVND_EVT_MDS_AVD_DN */
   avnd_evt_mds_ava_dn,  /* AVND_EVT_MDS_AVA_DN */
   avnd_evt_mds_cla_dn,  /* AVND_EVT_MDS_CLA_DN */

   /* clc event types */ 
   avnd_evt_clc_resp,    /* AVND_EVT_CLC_RESP */

   /* internal event types */ 
   avnd_evt_comp_pres_fsm_ev,  /* AVND_EVT_COMP_PRES_FSM_EV */
   avnd_evt_last_step_term     /* AVND_EVT_LAST_STEP_TERM */
};


/****************************************************************************
  Name          : avnd_main_process
 
  Description   : This routine is an entry point for the AvND task.
 
  Arguments     : arg - ptr to the cb handle
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void avnd_main_process (void *arg)
{
   AVND_CB            *cb = 0;
   SYSF_MBX           *mbx;
   uns32              cb_hdl;
   NCS_SEL_OBJ_SET    wait_sel_objs;
   NCS_SEL_OBJ        mbx_sel_obj;
   NCS_SEL_OBJ        highest_sel_obj;
   NCS_SEL_OBJ        ncs_srm_sel_obj;
   SaSelectionObjectT srm_sel_obj;
   NCS_BOOL           avnd_exit = FALSE;

   /* Only change scheduling class to real time on payload nodes. */
   if (getenv("AVD") == NULL)
   {
       struct sched_param param = {.sched_priority = 79};

       if (sched_setscheduler(0, SCHED_RR, &param) == -1)
           syslog(LOG_ERR, "Could not set scheduling class for avnd: %s", strerror(errno));
   }

   /* get cb-hdl */
   cb_hdl = *((uns32 *)arg);

   /* retrieve avnd cb */
   if ( 0 == (cb = (AVND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVND, cb_hdl)) )
   {
      m_AVND_LOG_CB(AVSV_LOG_CB_RETRIEVE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return;
   }

   /* get the mbx */
   mbx = &cb->mbx;

   /* reset the wait select objects */
   m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);

   /* get the mbx select object */
   mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(mbx);

   /* get the EDSv select object - TBD in future */

   /* get srm sel obj */
   srm_sel_obj = cb->srm_sel_obj;
   m_SET_FD_IN_SEL_OBJ((uns32)srm_sel_obj,ncs_srm_sel_obj);



   /* set all the select objects on which avnd waits */
   m_NCS_SEL_OBJ_SET(mbx_sel_obj, &wait_sel_objs);
   /*m_NCS_SEL_OBJ_SET(edsv_sel_obj, &wait_sel_objs);*/
   m_NCS_SEL_OBJ_SET(ncs_srm_sel_obj, &wait_sel_objs);

   /* get the highest select object in the set */
   highest_sel_obj = m_GET_HIGHER_SEL_OBJ(mbx_sel_obj, ncs_srm_sel_obj);

   /* before waiting, return avnd cb */
   ncshm_give_hdl(cb_hdl);

   /* now wait forever */
   while (m_NCS_SEL_OBJ_SELECT(highest_sel_obj, &wait_sel_objs, NULL, NULL, NULL) != -1)
   {
      /* avnd mbx processing */
      if ( m_NCS_SEL_OBJ_ISSET(mbx_sel_obj, &wait_sel_objs) )
      {
          avnd_exit = avnd_mbx_process(mbx);
          if(TRUE == avnd_exit)
             break;
      }

      /* edsv event processing - TBD in future */


      /* srmsv event processing */
      if( m_NCS_SEL_OBJ_ISSET(ncs_srm_sel_obj, &wait_sel_objs) )
         avnd_srm_process(cb_hdl);


      /* again set all the wait select objs */
      m_NCS_SEL_OBJ_SET(mbx_sel_obj, &wait_sel_objs);
      m_NCS_SEL_OBJ_SET(ncs_srm_sel_obj, &wait_sel_objs);
   }

  if(FALSE == avnd_exit)
  {
    m_AVND_LOG_INVALID_VAL_ERROR(0);
    m_NCS_SYSLOG(NCS_LOG_ERR,"NCS_AvSv: Avnd Func. Thread Exited"); 
  }

   /* Give some time for cleanup and reboot scipts */
   sleep(5);
   exit(0);

   return;
}


/****************************************************************************
  Name          : avnd_mbx_process
 
  Description   : This routine dequeues & processes the events from the avnd
                  mailbox.
 
  Arguments     : mbx - ptr to the mailbox
 
  Return Values : TRUE - avnd needs to be destroyed
                  FALSE - normal mbx processing
 
  Notes         : None
******************************************************************************/
NCS_BOOL avnd_mbx_process (SYSF_MBX *mbx)
{
   AVND_EVT *evt = 0;
   uns32 cb_hdl = 0;
   AVND_CB *cb = 0;

   /* process each event */
   while ( 0 != (evt = (AVND_EVT *)m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt)) )
   {
      cb_hdl = evt->cb_hdl;

      avnd_evt_process(evt);
      
      /* retrieve avnd cb */
      if ( 0 == (cb = (AVND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVND, cb_hdl)) )
      {
         m_AVND_LOG_CB(AVSV_LOG_CB_RETRIEVE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
         return TRUE;
      }

      if(cb->destroy == TRUE)
      {
         ncshm_give_hdl(cb_hdl);
         avnd_destroy();
         return TRUE;
      }

      ncshm_give_hdl(cb_hdl);

   }

   return FALSE;
}


/****************************************************************************
  Name          : avnd_evt_process
 
  Description   : This routine processes the AvND event.
 
  Arguments     : mbx - ptr to the mailbox
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void avnd_evt_process (AVND_EVT *evt)
{
   AVND_CB *cb = 0;
   uns32   rc = NCSCC_RC_SUCCESS;

   /* nothing to process */
   if (!evt) goto done;

   /* validate the event type */
   if ( (evt->type <= AVND_EVT_INVALID) ||
        (evt->type >= AVND_EVT_MAX) )
   {
      /* log */
      goto done;
   }

   /* retrieve avnd cb */
   if ( 0 == (cb = (AVND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVND, evt->cb_hdl)) )
   {
      m_AVND_LOG_CB(AVSV_LOG_CB_RETRIEVE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      goto done;
   }

   /* Temp: AvD Down Handling */
   if ( TRUE == cb->is_avd_down ) goto done;

   /* log the event reception */
   m_AVND_LOG_EVT(evt->type, 0, 0, NCSFL_SEV_INFO);

   /* acquire cb write lock */
   m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);

   /* invoke the event handler */
   rc = g_avnd_func_list[evt->type](cb, evt);

   /* release cb write lock */
   m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);

   /* log the result of event processing */
   m_AVND_LOG_EVT(evt->type, 0, 
                  (rc == NCSCC_RC_SUCCESS) ? AVND_LOG_EVT_SUCCESS: AVND_LOG_EVT_FAILURE, 
                  NCSFL_SEV_INFO);

done:
   /* return avnd cb */
   if (cb) ncshm_give_hdl(evt->cb_hdl);

   /* free the event */
   if (evt) avnd_evt_destroy(evt);

   return;
}


/*****************************************************************************
 * Function: avnd_evt_invalid_func
 *
 * Purpose:  This function is the handler for invalid events. It just
 * dumps the event to the debug log and returns.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static uns32 avnd_evt_invalid_func(AVND_CB *cb,AVND_EVT *evt)
{

   /* This function should never be called
    * log the event to the debug log and return
    */
   m_AVSV_ASSERT(0);

   return NCSCC_RC_SUCCESS;
}



