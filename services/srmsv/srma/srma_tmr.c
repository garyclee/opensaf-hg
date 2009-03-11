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

  MODULE NAME: SRMA_TMR.C
 
..............................................................................

  DESCRIPTION: This file contains SRMSV timer specific functions

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srma.h"

extern uns32 gl_srma_hdl;

/****************************************************************************
  Name          :  srma_timer_init
  
  Description   :  This routine is used to initialize the SRMSv timer table.
 
  Arguments     :  cb - pointer to the SRMA Control Block
                   
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
uns32 srma_timer_init(SRMA_CB *srma)
{
   NCS_RP_TMR_INIT tmr_init_info;

   memset(&tmr_init_info, 0, sizeof(NCS_RP_TMR_INIT));

   /* Update the timer specific data to initiate RP timer for SRMA */
   tmr_init_info.callback_arg   = (void *)srma;
   tmr_init_info.svc_id         = NCSMDS_SVC_ID_SRMA;
   tmr_init_info.svc_sub_id     = 0;
   tmr_init_info.tmr_callback   = srma_timeout_handler;
   tmr_init_info.tmr_ganularity = 1; /* in secs */

   if ((srma->srma_tmr_cb = m_NCS_RP_TMR_INIT(&tmr_init_info)) == NULL)
   {
      m_SRMA_LOG_TMR(SRMSV_LOG_TMR_INIT,
                     SRMSV_LOG_TMR_FAILURE,
                     NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
   else
   {
      m_SRMA_LOG_TMR(SRMSV_LOG_TMR_INIT,
                     SRMSV_LOG_TMR_SUCCESS,
                     NCSFL_SEV_INFO);
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srma_timeout_handler
 
  Description   :  This routine will be called on expiry of the OS timer. This
                   inturn calls the timeout handler for all the bucket timers.
 
  Arguments     :  void *arg - opaque argument passed when starting the timer.
  
  Return Values :  None
 
  Notes         :  None
******************************************************************************/
void srma_timeout_handler (void *arg)
{
   SRMA_CB  *srma = (SRMA_CB *)arg;
   
   if (!srma)
      return;
      
   /* Asking RP timer to call rsrc-mon specific tmr callback function */
   m_NCS_RP_TMR_EXP(srma->srma_tmr_cb);

   m_SRMA_LOG_TMR(SRMSV_LOG_TMR_EXP,
                  SRMSV_LOG_TMR_SUCCESS,
                  NCSFL_SEV_INFO);
   return;
}


/****************************************************************************
  Name          :  srma_del_rsrc_from_monitoring
 
  Description   :  This routine removes the resource monitor record from
                   monitoring process. Basically it deletes the respective 
                   record from timer specific list.

  Arguments     :  srma - ptr to the SRMA Control Block
                   rsrc - resource monitor record                  
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srma_del_rsrc_from_monitoring(SRMA_CB *srma, 
                                   SRMA_RSRC_MON *rsrc,
                                   NCS_BOOL modif_flag)
{
   /* Stop the rsrc mon timer */
   m_NCS_RP_TMR_STOP(srma->srma_tmr_cb, rsrc->tmr_id);
   m_SRMA_LOG_TMR(SRMSV_LOG_TMR_STOP,
                  SRMSV_LOG_TMR_SUCCESS,
                  NCSFL_SEV_INFO);

   /* Delete the rsrc mon timer */
   m_NCS_RP_TMR_DELETE(srma->srma_tmr_cb, rsrc->tmr_id);
   m_SRMA_LOG_TMR(SRMSV_LOG_TMR_DELETE,
                  SRMSV_LOG_TMR_SUCCESS,
                  NCSFL_SEV_INFO);

   rsrc->tmr_id = 0;

   if (!(modif_flag))
      srma_inform_appl_rsrc_expiry(srma, rsrc);

   return;
}


/****************************************************************************
  Name          :  srma_add_rsrc_for_monitoring
 
  Description   :  This routine submits the resource monitor record for 
                   monitoring process. Basically it adds the respective record
                   to timer specific list.

  Arguments     :  srma - ptr to the SRMA Control Block
                   rsrc - resource monitor record
                   period - expiry period
                   create_flg - TRUE if the tmr need to create else FALSE
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_add_rsrc_for_monitoring(SRMA_CB *srma,
                                   SRMA_RSRC_MON *rsrc,
                                   uns32 period,
                                   NCS_BOOL create_flg)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_RP_TMR_HDL tmr_id;
   
   if (create_flg) 
   {
      if ((tmr_id = m_NCS_RP_TMR_CREATE(srma->srma_tmr_cb)) == 0)
      {
         m_SRMA_LOG_TMR(SRMSV_LOG_TMR_CREATE,
                        SRMSV_LOG_TMR_FAILURE,
                        NCSFL_SEV_CRITICAL);

         return NCSCC_RC_FAILURE;    
      }
      else
      {
         m_SRMA_LOG_TMR(SRMSV_LOG_TMR_CREATE,
                        SRMSV_LOG_TMR_SUCCESS,
                        NCSFL_SEV_INFO);
      }

      rsrc->tmr_id = tmr_id;
   }

   /* Start the rsrc-mon specific timer, once the timer is created for it */
   if (rsrc->tmr_id) 
   {
      if (m_NCS_RP_TMR_START(srma->srma_tmr_cb, 
                             rsrc->tmr_id, 
                             period,
                             srma_rsrc_mon_timeout_handler,
                             NCS_UNS32_TO_PTR_CAST(rsrc->rsrc_mon_hdl)) != NCSCC_RC_SUCCESS)
      {
         m_SRMA_LOG_TMR(SRMSV_LOG_TMR_START,
                        SRMSV_LOG_TMR_FAILURE,
                        NCSFL_SEV_CRITICAL);

         rsrc->tmr_id = 0;
         return NCSCC_RC_FAILURE;   
      }
      else
      {   
         m_SRMA_LOG_TMR(SRMSV_LOG_TMR_START,
                        SRMSV_LOG_TMR_SUCCESS,
                        NCSFL_SEV_INFO);
      }
   }

   return rc;
}


/****************************************************************************
  Name          :  srma_timer_destroy
 
  Description   :  This routine destroys all the acquired resources at RP
                   timer by SRMA.

  Arguments     :  srma - ptr to the SRMA CB
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 
  Notes         :  None.
******************************************************************************/
uns32 srma_timer_destroy(SRMA_CB *srma)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Nothing to destroy */
   if (!(srma->srma_tmr_cb))
      return rc;
 
   rc = m_NCS_RP_TMR_DESTROY(&srma->srma_tmr_cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMA_LOG_TMR(SRMSV_LOG_TMR_DESTROY,
                     SRMSV_LOG_TMR_FAILURE,
                     NCSFL_SEV_CRITICAL);
      return rc;
   }
   else
   {
      m_SRMA_LOG_TMR(SRMSV_LOG_TMR_DESTROY,
                     SRMSV_LOG_TMR_SUCCESS,
                     NCSFL_SEV_INFO);
   }

   srma->srma_tmr_cb = NULL;

   return rc;
}


/****************************************************************************
  Name          :  srma_rsrc_mon_timeout_handler
 
  Description   :  This routine is called when ever the monitoring rate is
                   expired and needs to check out the rsrc specific statistics
                   against the test condition.

  Arguments     :  arg = rsrc mon handle submitted at the time of timer start
                   
  Return Values :  Nothing.
 
  Notes         :  None.
******************************************************************************/
void srma_rsrc_mon_timeout_handler(NCSCONTEXT arg)
{
   uns32 rsrc_mon_hdl = NCS_PTR_TO_UNS32_CAST(arg);
   SRMA_CB *srma = NULL;
   SRMA_RSRC_MON *rsrc = NULL;
   
   /* retrieve srma cb */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);
      return;      
   }

   /* retrieve srma cb */
   if ((rsrc = (SRMA_RSRC_MON *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                        (uns32)rsrc_mon_hdl)) == NULL)
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);

      /* release SRMA specific handle */
      m_SRMSV_GIVEUP_SRMA_CB;

      return;      
   }

   if (rsrc->rsrc_mon_expiry)
   {
      uns32 period = 0;
      time_t  now;
      m_GET_TIME_STAMP(now);

      if (now < rsrc->rsrc_mon_expiry)
      {
         /* LOG something wrong here.. should not expired before the 
            expiry period, so LOG the respective data */

         /* After LOG the message, ok.. what to do?? add the rsrc to timer
            list then */
         period = (now - rsrc->rsrc_mon_expiry);
         srma_add_rsrc_for_monitoring(srma, rsrc, period, FALSE);
      }
      else
      {
         if (srma->srma_tmr_cb != NULL)
            srma_del_rsrc_from_monitoring(srma, rsrc, FALSE);
      }
   }
      
   /* release SRMA specific handle */
   m_SRMSV_GIVEUP_SRMA_CB;

   /* release SRMA specific handle */
   ncshm_give_hdl(rsrc_mon_hdl);

   return;
}




