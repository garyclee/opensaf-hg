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
  FILE NAME: IFSV_TMR.C

  DESCRIPTION: IfSv Timer related routines.

  FUNCTIONS INCLUDED in this module:
  ifsv_timer_expiry ... Interface timer expiry.
  ifsv_tmr_start ...... Start Interface timer.
  ifsv_tmr_stop ....... Stop Interface timer.

******************************************************************************/
#include "ifsv.h"

static void ifsv_timer_expiry (NCSCONTEXT uarg);

/****************************************************************************
 * Name          : ifsv_timer_expiry
 *
 * Description   : This function which is registered with the OS tmr function,
 *                 which will post a message to the corresponding mailbox 
 *                 depending on the component type.
 *
 * Arguments     : uarg  - Argument passed to the timer expiry callback 
 *                         function. 
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void
ifsv_timer_expiry (NCSCONTEXT uarg)
{
   IFSV_TMR *tmr = (IFSV_TMR*)uarg;
   NCS_IPC_PRIORITY priority = NCS_IPC_PRIORITY_HIGH;
   IFSV_CB  *cb;
   IFSV_EVT *evt;
   uns32    ifsv_hdl = tmr->uarg;

   if (tmr != NULL)
   {
      /* post a message to the corresponding component */
      if ((cb = (NCSCONTEXT) ncshm_take_hdl(tmr->svc_id, ifsv_hdl))
            != IFSV_CB_NULL)
      {         
         evt = m_MMGR_ALLOC_IFSV_EVT;
         if (evt == IFSV_NULL)
         {
            m_IFSV_LOG_SYS_CALL_FAIL(\
               ifsv_log_svc_id_from_comp_type(cb->comp_type),\
               IFSV_LOG_MEM_ALLOC_FAIL,0);
            return;
         }
         m_NCS_MEMSET(evt,0,sizeof(IFSV_EVT));
         evt->vrid   = cb->vrid;
         evt->cb_hdl = ifsv_hdl;

         switch(tmr->tmr_type)
         {
           case NCS_IFSV_IFND_MDS_DEST_GET_TMR :
           {
             evt->type = IFND_EVT_TMR_EXP;
             evt->info.ifd_evt.info.tmr_exp.tmr_type = 
                                             NCS_IFSV_IFND_MDS_DEST_GET_TMR;
             m_IFD_LOG_TMR(IFSV_LOG_TMR_EXPIRY, tmr->info.reserved);
             break;
           }

           case NCS_IFSV_IFND_EVT_INTF_AGING_TMR :
           {
             evt->type = IFND_EVT_TMR_EXP;
             evt->info.ifnd_evt.info.tmr_exp.tmr_type = 
                                             NCS_IFSV_IFND_EVT_INTF_AGING_TMR;
             evt->info.ifnd_evt.info.tmr_exp.info.ifindex = tmr->info.ifindex;
             m_IFD_LOG_TMR(IFSV_LOG_TMR_EXPIRY, tmr->info.ifindex);
             break;
           }

           case NCS_IFSV_IFD_EVT_INTF_AGING_TMR :
           {
             evt->type = IFD_EVT_TMR_EXP;
             evt->info.ifd_evt.info.tmr_exp.tmr_type = 
                                             NCS_IFSV_IFD_EVT_INTF_AGING_TMR;
             evt->info.ifd_evt.info.tmr_exp.info.ifindex = tmr->info.ifindex;
             m_IFD_LOG_TMR(IFSV_LOG_TMR_EXPIRY, tmr->info.ifindex);
             break;
           }

           case NCS_IFSV_IFD_EVT_RET_TMR :
           {
             evt->type = IFD_EVT_TMR_EXP;
             evt->info.ifd_evt.info.tmr_exp.tmr_type = 
                                             NCS_IFSV_IFD_EVT_RET_TMR;
             evt->info.ifd_evt.info.tmr_exp.info.node_id = tmr->info.node_id;
             m_IFD_LOG_TMR(IFSV_LOG_TMR_EXPIRY, tmr->info.ifindex);
             break;
           }
      
           case NCS_IFSV_IFD_IFND_REC_FLUSH_TMR :
           {
             evt->type = IFD_EVT_TMR_EXP;
             evt->info.ifd_evt.info.tmr_exp.tmr_type = 
                                             NCS_IFSV_IFD_IFND_REC_FLUSH_TMR;
            if( cb->ifnd_rec_flush_tmr != IFSV_NULL)
            {
                m_MMGR_FREE_IFSV_TMR(cb->ifnd_rec_flush_tmr);
                cb->ifnd_rec_flush_tmr = IFSV_NULL;
            }

             break;
           }


           default :
           {
             return;
             break;
           }
          } /* switch (tmr->tmr_type) */

         m_NCS_IPC_SEND(&cb->mbx, evt, priority);
         ncshm_give_hdl(ifsv_hdl);;
      }
   }
   return;
}


/****************************************************************************
 * Name          : ifsv_tmr_start
 *
 * Description   : This function which is used to start the timer for IFND and 
 *                 IFD.
 *
 * Arguments     : tmr      - timer argument which needs to be register with 
 *                            the timer callback argument (which would be 
 *                            called with the timer callback as an argument).
 *                 cb       - IfSv control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : It is the responsibility of the caller to fill
 *                 NCS_IFSV_TMR_TYPE and its associated field, e.g. for 
 *                 NCS_IFSV_IFND_EVT_INTF_AGING_TMR and 
 *                 NCS_IFSV_IFD_EVT_INTF_AGING_TMR, index has to be filled.
 *****************************************************************************/
uns32
ifsv_tmr_start (IFSV_TMR *tmr, IFSV_CB *cb)
{
   uns32  info;
   uns32  timer_period;

   tmr->uarg    = cb->cb_hdl;
   
   switch(tmr->tmr_type)
   {
     case NCS_IFSV_IFND_MDS_DEST_GET_TMR : 
     {
       timer_period = NCS_IFSV_IFND_MDS_DEST_GET_TMR_PERIOD; 
       break;
     }

     case NCS_IFSV_IFD_EVT_RET_TMR : 
     {
       timer_period = NCS_IFSV_IFD_RET_TMR_PERIOD; 
       break;
     }
   
     case NCS_IFSV_IFD_IFND_REC_FLUSH_TMR:
     {
        timer_period = NCS_IFSV_IFD_IFND_FLUSH_TMR_PERIOD;
        break;
     }

     case NCS_IFSV_IFND_EVT_INTF_AGING_TMR : 
     case NCS_IFSV_IFD_EVT_INTF_AGING_TMR : 
     {
       timer_period = IFSV_CLEANUP_TMR_PERIOD; 
       if(tmr->info.ifindex == 0)
          return NCSCC_RC_FAILURE;
       break;
     }
     
     default :
     {
       return NCSCC_RC_FAILURE;
       break;
     }
   } /* switch (tmr->tmr_type) */

   info = tmr->info.ifindex;

   if (tmr->id == TMR_T_NULL)
   {      
      m_NCS_TMR_CREATE (tmr->id, timer_period, 
         ifsv_timer_expiry, (void*)tmr);
      m_IFSV_LOG_TMR(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_TMR_CREATE, info);
   }
   
   if (tmr->is_active == FALSE)
   {      
      m_NCS_TMR_START(tmr->id, timer_period, 
         ifsv_timer_expiry,(void*)tmr);      
      tmr->is_active = TRUE;
      m_IFSV_LOG_TMR(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_TMR_START, info);
   }

   return (NCSCC_RC_SUCCESS);
}


/****************************************************************************
 * Name          : ifsv_tmr_stop
 *
 * Description   : This function which is used to stop the IfSv timer.
 *
 * Arguments     : tmr      - timer needs to be stoped. 
 *                 cb       - IfSv CB pointer.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_tmr_stop (IFSV_TMR *tmr,IFSV_CB *cb)
{
   if (tmr->is_active == TRUE)
   {      
      m_NCS_TMR_STOP(tmr->id);
      tmr->is_active = FALSE;
      m_IFSV_LOG_TMR(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_TMR_STOP,tmr->info.ifindex);
   }
   if (tmr->id != TMR_T_NULL)
   {
      m_IFSV_LOG_TMR(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_TMR_DELETE,tmr->info.ifindex);
      m_NCS_TMR_DESTROY(tmr->id);
      tmr->id = TMR_T_NULL;      
   }
   return;
}
