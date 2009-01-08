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

  MODULE NAME: SRMND_EVT.C
 
..............................................................................

  DESCRIPTION: This file contains SRMSv event specific routine to be processed
               at/by SRMND.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srmnd.h"

extern uns32 gl_srmnd_hdl;

/****************************************************************************
  Name          :  srmnd_process_srma_msg
 
  Description   :  This is the main routine to be called to process all the 
                   arrived events from SRMA.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   srma_msg - SRMA event type
                   srma_dest - SRMA MDS dest data
                   
  Return Values :  NCSCC_RC_SUCCESS or NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_process_srma_msg(SRMND_CB *srmnd,
                             SRMA_MSG *srma_msg,
                             MDS_DEST *srma_dest)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SRMND_SRMA_USR_KEY usr_key;


   if (!(srma_msg))
      return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(&usr_key, 0, sizeof(SRMND_SRMA_USR_KEY));

   usr_key.srma_usr_hdl = srma_msg->handle;
   usr_key.srma_dest = *srma_dest;

   switch (srma_msg->msg_type)
   {
   case SRMA_CREATE_RSRC_MON_MSG:
       rc = srmnd_create_rsrc_mon(srmnd,
                                  srma_msg->bcast,
                                  &usr_key,
                                  &srma_msg->info.create_mon,
                                  FALSE, /* Always false here */ 
                                  FALSE);
       if (rc != NCSCC_RC_SUCCESS)
       {
          m_SRMND_LOG_RSRC_MON(SRMSV_LOG_RSRC_MON_CREATE,
                               SRMSV_LOG_RSRC_MON_FAILED,
                               NCSFL_SEV_ERROR);
       }
       break;

   case SRMA_MODIFY_RSRC_MON_MSG:
       rc = srmnd_modify_rsrc_mon(srmnd,
                                  srma_msg->bcast,
                                  &usr_key,
                                  &srma_msg->info.modify_rsrc_mon);
       if (rc != NCSCC_RC_SUCCESS)
       {
          m_SRMND_LOG_RSRC_MON(SRMSV_LOG_RSRC_MON_MODIFY,
                               SRMSV_LOG_RSRC_MON_FAILED,
                               NCSFL_SEV_ERROR);
       }
       break;

   case SRMA_DEL_RSRC_MON_MSG:
       srmnd_del_srma_rsrc_mon(srmnd,
                               srma_msg->bcast,
                               &usr_key,
                               srma_msg->info.del_rsrc_mon.srmnd_rsrc_hdl);

       m_SRMND_LOG_RSRC_MON(SRMSV_LOG_RSRC_MON_DELETE,
                            SRMSV_LOG_RSRC_MON_SUCCESS,
                            NCSFL_SEV_INFO);
       break;

   case SRMA_BULK_CREATE_MON_MSG:
       rc = srmnd_bulk_create_rsrc_mon(srmnd,
                                       srma_msg->bcast,
                                       &usr_key,
                                       &srma_msg->info.bulk_create_mon);
       if (rc != NCSCC_RC_SUCCESS)
       {
          m_SRMND_LOG_RSRC_MON(SRMSV_LOG_RSRC_MON_BULK_CREATE,
                               SRMSV_LOG_RSRC_MON_FAILED,
                               NCSFL_SEV_ERROR);
       }
       else
       {
          m_SRMND_LOG_RSRC_MON(SRMSV_LOG_RSRC_MON_BULK_CREATE,
                               SRMSV_LOG_RSRC_MON_SUCCESS,
                               NCSFL_SEV_INFO);
       }
       break;

   case SRMA_START_MON_MSG:
       rc = srmnd_start_stop_appl_mon(srmnd, &usr_key, TRUE);
       if (rc != NCSCC_RC_SUCCESS)
       {
          m_SRMND_LOG_MON(SRMND_MON_APPL_START,
                          SRMND_MON_FAILED,
                          NCSFL_SEV_ERROR);
       }
       else
       {
          m_SRMND_LOG_MON(SRMND_MON_APPL_START,
                          SRMND_MON_SUCCESS,
                          NCSFL_SEV_INFO);
       }
       break;

   case SRMA_STOP_MON_MSG:
       rc = srmnd_start_stop_appl_mon(srmnd, &usr_key, FALSE);
       if (rc != NCSCC_RC_SUCCESS)
       {
          m_SRMND_LOG_MON(SRMND_MON_APPL_STOP,
                          SRMND_MON_FAILED,
                          NCSFL_SEV_ERROR);
       }
       else
       {
          m_SRMND_LOG_MON(SRMND_MON_APPL_STOP,
                          SRMND_MON_SUCCESS,
                          NCSFL_SEV_INFO);
       }
       break;

   case SRMA_UNREGISTER_MSG:
       rc = srmnd_unreg_appl(srmnd, &usr_key);
       if (rc != NCSCC_RC_SUCCESS)
       {
          m_SRMND_LOG_MON(SRMND_MON_APPL_DEL,
                          SRMND_MON_FAILED,
                          NCSFL_SEV_ERROR);
       }
       else
       {
          m_SRMND_LOG_MON(SRMND_MON_APPL_DEL,
                          SRMND_MON_SUCCESS,
                          NCSFL_SEV_INFO);
       }
       break;

   case SRMA_GET_WATERMARK_MSG:
       {
           SRMND_WATERMARK_DATA get_wm;

           m_NCS_OS_MEMSET(&get_wm, 0, sizeof(SRMND_WATERMARK_DATA));
          
           get_wm.srma_dest = *srma_dest;
           get_wm.usr_hdl = srma_msg->handle;
           get_wm.rsrc_type = srma_msg->info.get_wm.rsrc_type;
           get_wm.wm_type = srma_msg->info.get_wm.wm_type;

           if ((get_wm.rsrc_type == NCS_SRMSV_RSRC_PROC_MEM) ||
               (get_wm.rsrc_type == NCS_SRMSV_RSRC_PROC_CPU))
              get_wm.pid = srma_msg->info.get_wm.pid;

           rc = srmnd_send_msg(srmnd, (NCSCONTEXT)&get_wm, SRMND_WATERMARK_VAL_MSG);      
           if (rc != NCSCC_RC_SUCCESS)
           {
              m_SRMND_LOG_MON(SRMND_MON_GET_WM,
                              SRMND_MON_FAILED,
                              NCSFL_SEV_ERROR);
           }
           else
           {
              m_SRMND_LOG_MON(SRMND_MON_GET_WM,
                              SRMND_MON_SUCCESS,
                              NCSFL_SEV_INFO);
           }
       }
       break;

   default:
       /* Unknow SRMA message type */      
       break;
   }
    
   return rc;
}


/****************************************************************************
  Name          :  srmnd_send_msg
 
  Description   :  This routine sends SRMND specific event to the respective
                   SRMA.
 
  Arguments     :  srmnd - ptr to the SRMND CB
                   msg_type - One of the SRMA event type
                   appl_hdl - Application Specific Handle
                   srmnd_info - Ptr to the SRMND info, has the SRMND dest
                                info to which the event need to send. if it
                                is NULL, so it is broadcast event.
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
uns32 srmnd_send_msg(SRMND_CB *srmnd, 
                     NCSCONTEXT data,
                     SRMND_MSG_TYPE msg_type)
{
   SRMND_MSG srmnd_msg;
   SRMND_MSG tmp_srmnd_msg;
   SRMND_RSRC_MON_NODE *rsrc = NULL;
   SRMND_MON_SRMA_USR_NODE *user = NULL;
   MDS_DEST srma_dest;
   SRMND_RSRC_TYPE_NODE *rsrc_type_node = NULL;
   SRMND_WATERMARK_DATA *wm_data = NULL;

   /* If SRMND oper status is not UP, so no need to send any
      kind of message */
   if (srmnd->oper_status != SRMSV_ND_OPER_STATUS_UP)
      return NCSCC_RC_FAILURE;
   
   m_NCS_OS_MEMSET(&srma_dest, 0, sizeof(MDS_DEST));
   m_NCS_OS_MEMSET(&tmp_srmnd_msg, 0, sizeof(SRMND_MSG));
   
   /* Update the event type */
   tmp_srmnd_msg.msg_type = msg_type;

   switch (msg_type)
   {
   case SRMND_CREATED_MON_MSG:
       rsrc = (SRMND_RSRC_MON_NODE *)data;
       tmp_srmnd_msg.srma_rsrc_hdl = rsrc->srma_rsrc_hdl;
       tmp_srmnd_msg.info.srmnd_rsrc_mon_hdl = rsrc->rsrc_mon_hdl;
       srma_dest = rsrc->srma_usr_node->usr_key.srma_dest;
       break;

   case SRMND_BULK_CREATED_MON_MSG:
       user = (SRMND_MON_SRMA_USR_NODE *)data;
       tmp_srmnd_msg.info.bulk_rsrc_mon.srma_usr_node = user;
       srma_dest = user->usr_key.srma_dest;
       break;

   case SRMND_APPL_NOTIF_MSG:
       rsrc = (SRMND_RSRC_MON_NODE *)data;
       tmp_srmnd_msg.srma_rsrc_hdl = rsrc->srma_rsrc_hdl;
       tmp_srmnd_msg.info.notify_val = rsrc->notify_val;
       srma_dest = rsrc->srma_usr_node->usr_key.srma_dest;
       break;

   case SRMND_SYNCHRONIZED_MSG:
       /* Get the respective SRMA adest info */
       srma_dest = *(MDS_DEST *)(data);        
       break;

   case SRMND_WATERMARK_VAL_MSG:
       wm_data = (SRMND_WATERMARK_DATA *)data;
       srma_dest = wm_data->srma_dest;        

       /* Update the user handle info of the client who made the request */
       tmp_srmnd_msg.srma_rsrc_hdl = wm_data->usr_hdl;

       /* Check whether the respective rsrc node already exists */
       rsrc_type_node = srmnd_check_rsrc_type_node(srmnd, wm_data->rsrc_type, wm_data->pid);
       if (rsrc_type_node)
          rsrc = rsrc_type_node->watermark_rsrc;
     
       tmp_srmnd_msg.info.wm_val.pid = wm_data->pid;
       tmp_srmnd_msg.info.wm_val.rsrc_type = wm_data->rsrc_type;
       tmp_srmnd_msg.info.wm_val.wm_type = wm_data->wm_type;

       /* Update if RSRC exists for WM mon-type and it has some WM data to send */
       if ((rsrc) && (rsrc->mon_data.wm_val.l_val.val_type != 0))
       {  
          /* Update only if wm_rsrc configuration exists */
          tmp_srmnd_msg.info.wm_val.wm_exist = TRUE;

          switch(wm_data->wm_type)
          {
          case NCS_SRMSV_WATERMARK_LOW:
              tmp_srmnd_msg.info.wm_val.low_val = rsrc->mon_data.wm_val.l_val;
              break;

          case NCS_SRMSV_WATERMARK_HIGH:
              tmp_srmnd_msg.info.wm_val.high_val = rsrc->mon_data.wm_val.h_val;
              break;

          case NCS_SRMSV_WATERMARK_DUAL:
              tmp_srmnd_msg.info.wm_val.low_val = rsrc->mon_data.wm_val.l_val;
              tmp_srmnd_msg.info.wm_val.high_val = rsrc->mon_data.wm_val.h_val;
              break;

          default:
              break;
          }
       }
       break;

   case SRMND_PROC_EXIT_MSG:
       rsrc = (SRMND_RSRC_MON_NODE *)data;
       tmp_srmnd_msg.srma_rsrc_hdl = rsrc->srma_rsrc_hdl;
       srma_dest = rsrc->srma_usr_node->usr_key.srma_dest;
       break;

   case SRMND_WATERMARK_EXIST_MSG:
       wm_data = (SRMND_WATERMARK_DATA *)data;
       srma_dest = wm_data->srma_dest;        

       /* Update the user handle info of the client who made the request */
       tmp_srmnd_msg.srma_rsrc_hdl = wm_data->usr_hdl;
       break;

   default:
       /* Unknown SRMND message type */
       break;
   }
   
   /* If msg_type is of NOTIFICATION/PROC_EXIT type, then need to check respective 
      SUBSCR list to send the message */   
   if ((msg_type == SRMND_APPL_NOTIF_MSG) || (msg_type == SRMND_PROC_EXIT_MSG))
   {
      MDS_DEST subscr_dest;
      SRMND_RSRC_MON_NODE *next_subscr;
      SRMND_RSRC_MON_NODE *subscr_node = rsrc->rsrc_type_node->start_rsrc_subs_list;

      while (subscr_node)
      {
         m_NCS_OS_MEMSET(&subscr_dest, 0, sizeof(MDS_DEST));

         next_subscr = subscr_node->next_rsrc_type_ptr;
        
         /* If the rsrc sent flag is set to NOTIF-SENT and if the subscr node
            sent flag is not set to SUBS-SENT-PEND then don't send any event.. 
            continue with the next subscr. */ 
         if ((rsrc->sent_flag & RSRC_APPL_NOTIF_SENT) &&
             (!(rsrc->sent_flag & RSRC_APPL_SUBS_SENT_PENDING)) &&
             (!(subscr_node->sent_flag & RSRC_APPL_SUBS_SENT_PENDING)))
         {
            subscr_node = next_subscr;
            continue;
         }

         /* Get the MDS dest of the subscr node */
         subscr_dest = subscr_node->srma_usr_node->usr_key.srma_dest;
 
         m_NCS_OS_MEMSET(&srmnd_msg, 0, sizeof(SRMND_MSG));

         /* Now copy the content of temp store of SRMND msg */
         srmnd_msg = tmp_srmnd_msg;
         
         /* Fix for IR00058963 */
         srmnd_msg.srma_rsrc_hdl = subscr_node->srma_rsrc_hdl;

         /* Sending SRMA event to the respective SRMND */
         srmnd_mds_send(srmnd, &srmnd_msg, &subscr_dest);
        
         /* Now reset the subs-sent-pending flag */
         subscr_node->sent_flag &= ~(RSRC_APPL_SUBS_SENT_PENDING);

         subscr_node = next_subscr;
      }
      
      /* Now reset the subs-sent-pending flag */
      rsrc->sent_flag &= ~(RSRC_APPL_SUBS_SENT_PENDING);
   }
  
   if (((msg_type == SRMND_APPL_NOTIF_MSG) &&
        (!(rsrc->sent_flag & RSRC_APPL_NOTIF_SENT)) &&
        (rsrc->usr_type == NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR)) ||
       (msg_type != SRMND_APPL_NOTIF_MSG))
   {
      m_NCS_OS_MEMSET(&srmnd_msg, 0, sizeof(SRMND_MSG));

      /* Now copy the content of temp store of SRMND msg */
      srmnd_msg = tmp_srmnd_msg;
   
      /* Sending SRMND event to the respective SRMA */
      srmnd_mds_send(srmnd, &srmnd_msg, &srma_dest);
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmnd_del_srma_msg
 
  Description   :  This routine deletes the SRMA message data (that was 
                   arrived from SRMA).
 
  Arguments     :  msg - ptr to the SRMA message                   
 
  Return Values :  Nothing
 
  Notes         :  None
******************************************************************************/
void srmnd_del_srma_msg(SRMA_MSG *msg)
{
   /* Check msg for non-NULL, else return */
   if (msg == NULL)
      return;

   /* If the message type is Bulk Create then delete rsrc's list data */
   if (msg->msg_type == SRMA_BULK_CREATE_MON_MSG)
   {
      SRMA_CREATE_RSRC_MON_NODE *rsrc = NULL, *next_rsrc = NULL;

      rsrc = msg->info.bulk_create_mon.bulk_rsrc;
      /* Delete all the rsrc specific data */
      while (rsrc)
      {
         next_rsrc = rsrc->next_rsrc_mon;        
         /* Delete the rsrc node */
         m_MMGR_FREE_SRMA_CREATE_RSRC(rsrc);
         rsrc = next_rsrc;
      }
   }

   /* Ok now delete the SRMA message */
   m_MMGR_FREE_SRMA_MSG(msg);

   return;
}


/****************************************************************************
  Name          :  srmnd_process_tmr_msg
 
  Description   :  This routine is called when ever the monitoring rate is
                   expired and needs to check out the rsrc specific statistics
                   against the test condition.

  Arguments     :  arg = rsrc mon handle submitted at the time of timer start
                   
  Return Values :  Nothing.
 
  Notes         :  None.
******************************************************************************/
void srmnd_process_tmr_msg(uns32 rsrc_mon_hdl)
{
   SRMND_CB *srmnd = NULL;
   SRMND_RSRC_MON_NODE *rsrc = NULL;
   NCS_BOOL rsrc_delete = FALSE;
   
   /* retrieve srmnd cb */
   if ((srmnd = (SRMND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                       (uns32)gl_srmnd_hdl)) == SRMND_CB_NULL)
   {      
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                      SRMSV_LOG_HDL_SUCCESS,
                      NCSFL_SEV_ERROR);
      return;      
   }

   /* retrieve srmnd cb */
   if ((rsrc = (SRMND_RSRC_MON_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                               (uns32)rsrc_mon_hdl)) == NULL)
   {
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                      SRMSV_LOG_HDL_SUCCESS,
                      NCSFL_SEV_ERROR);

      /* release SRMND specific handle */
      ncshm_give_hdl(gl_srmnd_hdl);
      return;      
   }

   if (rsrc->tmr_id == 0)
   {
      /* release SRMND specific handle */
      ncshm_give_hdl(rsrc_mon_hdl);

      /* Seems the rsrc has been deleted from monitoring process */   
      /* release SRMND specific handle */
      ncshm_give_hdl(gl_srmnd_hdl);      

      return;
   }

   srmnd_check_rsrc_stats(srmnd, rsrc, &rsrc_delete);
  
   if (rsrc_delete != TRUE)
   {
      if (rsrc->rsrc_mon_expiry)
      {
         time_t  now;
         m_GET_TIME_STAMP(now);

         if (now > rsrc->rsrc_mon_expiry)
         {
            /* release SRMND specific handle */
            ncshm_give_hdl(rsrc_mon_hdl);

            /* Monitoring interval is completed, so delete the rsrc */
            srmnd_del_rsrc_mon(srmnd, rsrc_mon_hdl);

            /* release SRMND specific handle */
            ncshm_give_hdl(gl_srmnd_hdl);

            return;
         }
      }
   
      /* Ok now again add the rsrc for monitoring */
      if (srmnd_add_rsrc_for_monitoring(srmnd, rsrc, FALSE) != NCSCC_RC_SUCCESS)
      {
         m_SRMND_LOG_MON(SRMND_MON_RSRC_ADD,
                         SRMND_MON_FAILED,
                         NCSFL_SEV_ERROR);
      }
   }

   /* release rsrc specific handle */
   ncshm_give_hdl(rsrc_mon_hdl);

   if (rsrc_delete)
      srmnd_del_rsrc_mon(srmnd, rsrc_mon_hdl);

   /* release SRMND specific handle */
   ncshm_give_hdl(gl_srmnd_hdl);

   return;
}






















