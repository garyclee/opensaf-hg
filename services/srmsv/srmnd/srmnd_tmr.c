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

  MODULE NAME: SRMND_TMR.C
 
..............................................................................

  DESCRIPTION: This file contains SRMSV timer specific functions

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srmnd.h"

extern uns32 gl_srmnd_hdl;

/****************************************************************************
  Name          :  srmnd_timer_init
  
  Description   :  This routine is used to initialize the SRMSv timer table.
 
  Arguments     :  cb - pointer to the SRMND Control Block
                   
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
uns32 srmnd_timer_init(SRMND_CB *srmnd_cb)
{
   NCS_RP_TMR_INIT tmr_init_info;
   
   m_NCS_OS_MEMSET(&tmr_init_info, 0, sizeof(NCS_RP_TMR_INIT));

   tmr_init_info.callback_arg   = (void *)srmnd_cb;
   tmr_init_info.svc_id         = NCSMDS_SVC_ID_SRMND;
   tmr_init_info.svc_sub_id     = 0;
   tmr_init_info.tmr_callback   = srmnd_timeout_handler;
   tmr_init_info.tmr_ganularity = 1; /* in secs */

   if ((srmnd_cb->srmnd_tmr_cb = m_NCS_RP_TMR_INIT(&tmr_init_info)) == NULL)
   {
      m_SRMND_LOG_TMR(SRMSV_LOG_TMR_INIT,
                      SRMSV_LOG_TMR_FAILURE,
                      NCSFL_SEV_CRITICAL);

      return NCSCC_RC_FAILURE;
   }
   
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : srmnd_timeout_handler
 
  Description   : This routine will be called on expiry of the OS timer. This
                  inturn calls the timeout handler for all the bucket timers.
 
  Arguments     : void *arg - opaque argument passed when starting the timer.
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void srmnd_timeout_handler (void *arg)
{
   SRMND_EVT *srmnd_evt = NULL;
   SRMND_CB *srmnd = (SRMND_CB *)arg;

   /* retrieve srmnd cb */
   if (!srmnd)
   {      
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                      SRMSV_LOG_HDL_SUCCESS,
                      NCSFL_SEV_ERROR);
      return;      
   }

   /* Post the event to SRMND about timer expiration */
   srmnd_evt = srmnd_evt_create(srmnd->cb_hdl,
                                NULL,
                                NULL,
                                SRMND_TMR_EVT_TYPE);
   if (srmnd_evt)
      m_NCS_IPC_SEND(&srmnd->mbx, srmnd_evt, NCS_IPC_PRIORITY_HIGH);

   return;
}


/****************************************************************************
  Name          :  srmnd_del_rsrc_from_monitoring
 
  Description   :  This routine removes the resource monitor record from
                   monitoring process. Basically it deletes the respective 
                   record from timer specific list.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   rsrc - resource monitor record                  
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_del_rsrc_from_monitoring(NCS_RP_TMR_CB *srmnd_tmr_cb, NCS_RP_TMR_HDL *tmr_id)
{
   /* just return if tmr_id is zero (or) tmr_cb is NULL */
   if (!(srmnd_tmr_cb) || (*tmr_id == 0))
      return;

   /* Stop the rsrc mon timer */
   m_NCS_RP_TMR_STOP(srmnd_tmr_cb, *tmr_id);

   /* Delete the rsrc mon timer */
   m_NCS_RP_TMR_DELETE(srmnd_tmr_cb, *tmr_id);

   *tmr_id = 0;

   return;
}


/****************************************************************************
  Name          :  srmnd_add_rsrc_for_monitoring
 
  Description   :  This routine submits the resource monitor record for 
                   monitoring process. Basically it adds the respective record
                   to timer specific list.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   rsrc - resource monitor record                  
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_add_rsrc_for_monitoring(SRMND_CB *srmnd,
                                    SRMND_RSRC_MON_NODE *rsrc,
                                    NCS_BOOL create_flg)
{
   NCS_RP_TMR_HDL tmr_id;
   uns32 period = rsrc->mon_data.monitor_data.monitoring_rate;

   /* User application is in STOP monitor mode.. so don't add the resource
      for monitoring */    
   if (rsrc->srma_usr_node != NULL)
      if (rsrc->srma_usr_node->stop_monitoring == TRUE)
         return NCSCC_RC_SUCCESS;
 
   if (create_flg) 
   {
      if ((tmr_id = m_NCS_RP_TMR_CREATE(srmnd->srmnd_tmr_cb)) == 0)
      {
         m_SRMND_LOG_TMR(SRMSV_LOG_TMR_CREATE,
                         SRMSV_LOG_TMR_FAILURE,
                         NCSFL_SEV_CRITICAL);

         return NCSCC_RC_FAILURE;    
      }
      rsrc->tmr_id = tmr_id;
   }

   if (rsrc->tmr_id != 0)
   {
      if (m_NCS_RP_TMR_START(srmnd->srmnd_tmr_cb, 
                             rsrc->tmr_id, 
                             period,
                             srmnd_rsrc_mon_timeout_handler,
                             (void *)rsrc->rsrc_mon_hdl) != NCSCC_RC_SUCCESS)
      {
         m_SRMND_LOG_TMR(SRMSV_LOG_TMR_START,
                         SRMSV_LOG_TMR_FAILURE,
                         NCSFL_SEV_CRITICAL);

         rsrc->tmr_id = 0;
         return NCSCC_RC_FAILURE;   
      }
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmnd_timer_destroy
 
  Description   :  This routine destroys all the acquired resources at RP
                   timer by SRMND.

  Arguments     :  srmnd - ptr to the SRMND CB
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_timer_destroy(SRMND_CB *srmnd)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Nothing to destroy */
   if (!(srmnd->srmnd_tmr_cb))
      return rc;
 
   rc = m_NCS_RP_TMR_DESTROY(&srmnd->srmnd_tmr_cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_TMR(SRMSV_LOG_TMR_DESTROY,
                      SRMSV_LOG_TMR_FAILURE,
                      NCSFL_SEV_CRITICAL);
      return rc;
   }

   srmnd->srmnd_tmr_cb = NULL;

   return rc;
}

/****************************************************************************
  Name          :  srmnd_rsrc_mon_timeout_handler
 
  Description   :  This routine is called when ever the monitoring rate is
                   expired and needs to check out the rsrc specific statistics
                   against the test condition.

  Arguments     :  arg = rsrc mon handle submitted at the time of timer start
                   
  Return Values :  Nothing.
 
  Notes         :  None.
******************************************************************************/
void srmnd_rsrc_mon_timeout_handler(NCSCONTEXT arg)
{
   uns32 rsrc_mon_hdl = (uns32)arg;

   srmnd_process_tmr_msg(rsrc_mon_hdl);
   return;
}




