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

  MODULE NAME: SRMA_MSG.C
 
..............................................................................

  DESCRIPTION: This file contains SRMA event specific routines

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srma.h"

/****************************************************************************
  Name          :  srma_send_appl_msg
 
  Description   :  This routine sends application specific SRMA event to
                   the respective SRMND.
 
  Arguments     :  msg_type - One of the SRMA event type
                   appl_hdl - Application Specific Handle
                   srmnd_info - Ptr to the SRMND info, has the SRMND dest
                                info to which the event need to send. if it
                                is NULL, so it is broadcast event.
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
uns32 srma_send_appl_msg(SRMA_CB *srma, 
                         SRMA_MSG_TYPE msg_type,
                         uns32 appl_hdl,
                         SRMA_SRMND_INFO *srmnd_info)
{
   uns32    rc = NCSCC_RC_SUCCESS;
   SRMA_MSG srma_msg;
   MDS_DEST srmnd_dest;

   /* Get the MDS dest data */
   if (srmnd_info)
   {
      memset(&srmnd_dest, 0, sizeof(MDS_DEST));
      srmnd_dest = srmnd_info->srmnd_dest;
   }
 
   switch (msg_type)
   {
   case SRMA_UNREGISTER_MSG:
   case SRMA_START_MON_MSG:
   case SRMA_STOP_MON_MSG:
       memset(&srma_msg, 0, sizeof(SRMA_MSG));

       /* Update the event data */
       srma_msg.msg_type = msg_type;
       srma_msg.handle = appl_hdl;

       /* SRMND dest info will be NULL if it is a broadcast request */
       if (srmnd_info == NULL)
       {
          srma_msg.bcast = TRUE;
          rc = srma_mds_send(srma, &srma_msg, NULL);
       }
       else
       {
          /* Sending SRMA event to the respective SRMND(s) */
          rc = srma_mds_send(srma, &srma_msg, &srmnd_dest);
       }
       break;

   default:
       /* Invalid message type, so.. */
       assert(0);
       break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srma_send_rsrc_msg
 
  Description   :  This routine sends resource specific SRMA event to the
                   respective SRMND.
 
  Arguments     :  msg_type - One of the SRMA event type
                   appl_hdl - Application Specific Handle
                   srmnd_info - Ptr to the SRMND info, has the SRMND dest
                                info to which the event need to send. if it
                                is NULL, so it is broadcast event.
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
uns32 srma_send_rsrc_msg(SRMA_CB *srma,
                         SRMA_USR_APPL_NODE *appl,
                         MDS_DEST *srmnd_dest,
                         SRMA_MSG_TYPE msg_type,
                         SRMA_RSRC_MON *rsrc)
{
   uns32    rc = NCSCC_RC_SUCCESS;
   SRMA_MSG srma_msg;

   /* Check the correctness of the input */
   if ((msg_type < SRMA_CREATE_RSRC_MON_MSG) || (msg_type >= SRMA_MSG_MAX))
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_MSG_TYPE,
                      SRMA_LOG_MISC_NOTHING,
                      NCSFL_SEV_ERROR);

      return NCSCC_RC_FAILURE;
   }

   memset(&srma_msg, 0, sizeof(SRMA_MSG));

   /* Update the event data */
   srma_msg.msg_type = msg_type;
   srma_msg.handle = appl->user_hdl;

   /* TRUE for broadcast send, FALSE for unicast send  */
   if (srmnd_dest == NULL)
      srma_msg.bcast = TRUE;   

   switch (msg_type)
   {
   case SRMA_CREATE_RSRC_MON_MSG:
       /* Update the create resource monitor dat */
       srma_msg.info.create_mon.rsrc_hdl = rsrc->rsrc_mon_hdl;
       srma_msg.info.create_mon.mon_info.rsrc_info = rsrc->rsrc_info;
       srma_msg.info.create_mon.mon_info.monitor_data = rsrc->monitor_data;
       break;

   case SRMA_MODIFY_RSRC_MON_MSG:
       /* Update the modified resource monitor data */
       if (rsrc->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
          srma_msg.info.modify_rsrc_mon.rsrc_hdl = rsrc->rsrc_mon_hdl;
       else
          srma_msg.info.modify_rsrc_mon.rsrc_hdl = rsrc->srmnd_rsrc_hdl;

       srma_msg.info.modify_rsrc_mon.mon_info.rsrc_info = rsrc->rsrc_info;
       srma_msg.info.modify_rsrc_mon.mon_info.monitor_data = rsrc->monitor_data;
       break;

   case SRMA_DEL_RSRC_MON_MSG:
       /* Update the handle of the resource monitor that to be deleted */
       if (rsrc->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
          srma_msg.info.del_rsrc_mon.srmnd_rsrc_hdl = rsrc->rsrc_mon_hdl;
       else
          srma_msg.info.del_rsrc_mon.srmnd_rsrc_hdl = rsrc->srmnd_rsrc_hdl;
       break;

   case SRMA_GET_WATERMARK_MSG:
       srma_msg.info.get_wm.rsrc_type = rsrc->rsrc_info.rsrc_type;
       srma_msg.info.get_wm.pid = rsrc->rsrc_info.pid;
       srma_msg.info.get_wm.wm_type = rsrc->monitor_data.mon_cfg.watermark_type;
       break;

   default:
       break;
   }

   /* Sending SRMA event to the respective SRMND */
   rc = srma_mds_send(srma, &srma_msg, srmnd_dest);

   return rc;
}


/****************************************************************************
  Name          :  srma_process_srmnd_msg
 
  Description   :  This is the main routine to be called to process all the 
                   arrived events from SRMND.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   srma_msg - SRMA event type
                   srma_dest - SRMA MDS dest data
                   
  Return Values :  NCSCC_RC_SUCCESS or NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_process_srmnd_msg(SRMA_CB   *srma,
                             SRMND_MSG *srmnd_msg,
                             MDS_DEST  *srmnd_dest)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SRMA_RSRC_MON *rsrc_mon;

   switch (srmnd_msg->msg_type)
   {
   case SRMND_CREATED_MON_MSG:
       
       rsrc_mon = (SRMA_RSRC_MON*)ncs_patricia_tree_get(&srma->rsrc_mon_tree,
                                                        (uns8*)&srmnd_msg->srma_rsrc_hdl);
       if (!(rsrc_mon))
       {
          /* No association record?? :-( ok log it & return error */
          m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                         SRMSV_LOG_HDL_FAILURE,
                         NCSFL_SEV_ERROR);
          return rc;
       }
       rsrc_mon->srmnd_rsrc_hdl = srmnd_msg->info.srmnd_rsrc_mon_hdl;
       break;

   case SRMND_BULK_CREATED_MON_MSG:
       {
          SRMND_CREATED_RSRC_MON *bulk_rsrc = srmnd_msg->info.bulk_rsrc_mon.bulk_rsrc_mon;
          SRMND_CREATED_RSRC_MON *next_rsrc;

          while (bulk_rsrc)
          {   
              next_rsrc = bulk_rsrc->next_srmnd_rsrc_mon;
              
              rsrc_mon = (SRMA_RSRC_MON*)ncs_patricia_tree_get(&srma->rsrc_mon_tree,
                                                  (uns8*)&bulk_rsrc->srma_rsrc_hdl);
              if (!(rsrc_mon))
              {
                 /* No association record?? :-( ok log it & continue */
                 m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                                SRMSV_LOG_HDL_FAILURE,
                                NCSFL_SEV_ERROR);

                 bulk_rsrc = next_rsrc;
                 continue;
              }

              /* Update the SRMND resource mon handle */
              rsrc_mon->srmnd_rsrc_hdl = bulk_rsrc->srmnd_rsrc_hdl;

              /* Free the memory */
              m_MMGR_FREE_SRMND_CREATED_RSRC(bulk_rsrc);

              bulk_rsrc = next_rsrc;
          }

          /* Bulk rsrc mon list is deleted.. so make the respective list ptr
             to NULL */
          srmnd_msg->info.bulk_rsrc_mon.bulk_rsrc_mon = NULL;
       }
       break;

   case SRMND_PROC_EXIT_MSG:
       {
          NCS_SRMSV_RSRC_CBK_INFO cbk_info;
          SRMA_USR_APPL_NODE *appl = NULL;
                  
          rsrc_mon = (SRMA_RSRC_MON*)ncs_patricia_tree_get(&srma->rsrc_mon_tree,
                                             (uns8*)&srmnd_msg->srma_rsrc_hdl);
          if (!(rsrc_mon))              
          {
             /* No association record?? :-( ok log it & return error */
             m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                            SRMSV_LOG_HDL_FAILURE,
                            NCSFL_SEV_ERROR);
             return rc;
          }
         
          /* Get the respective appl record ptr to which the rsrc-mon
             record belongs to */         
         
          if (rsrc_mon->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
             appl = rsrc_mon->bcast_appl;
          else
             appl = rsrc_mon->usr_appl->usr_appl;

          memset(&cbk_info, 0, sizeof(NCS_SRMSV_RSRC_CBK_INFO));

          switch (rsrc_mon->rsrc_info.rsrc_type)
          {
          case NCS_SRMSV_RSRC_PROC_MEM:
          case NCS_SRMSV_RSRC_PROC_CPU:
              cbk_info.notify.pid = rsrc_mon->rsrc_info.pid;
             
               /* now no need to maintain the resource-mon record in SRMA database
                  so delete it then. */
               rc = srma_appl_delete_rsrc_mon(srma, srmnd_msg->srma_rsrc_hdl, FALSE);
               if (rc != SA_AIS_OK)
                  return NCSCC_RC_FAILURE;         
              break;

          default:
              return NCSCC_RC_FAILURE;         
          }
         
          cbk_info.notif_type = SRMSV_CBK_NOTIF_PROCESS_EXPIRED;
          cbk_info.rsrc_mon_hdl = srmnd_msg->srma_rsrc_hdl;
          cbk_info.node_num = m_NCS_NODE_ID_FROM_MDS_DEST(*srmnd_dest);

          /* Add the application notification message to cbk_list of an appl */
          rc = srma_update_appl_cbk_info(srma, &cbk_info, appl);
       }
       break;

   case SRMND_APPL_NOTIF_MSG:
       {
          NCS_SRMSV_RSRC_CBK_INFO cbk_info;
          SRMA_USR_APPL_NODE *appl = NULL;
                  
          rsrc_mon = (SRMA_RSRC_MON*)ncs_patricia_tree_get(&srma->rsrc_mon_tree,
                                             (uns8*)&srmnd_msg->srma_rsrc_hdl);
          if (!(rsrc_mon))              
          {
             /* No association record?? :-( ok log it & return error */
             m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                            SRMSV_LOG_HDL_FAILURE,
                            NCSFL_SEV_ERROR);
             return rc;
          }
         
          /* Get the respective appl record ptr to which the rsrc-mon
             record belongs to */         
         
          if (rsrc_mon->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
             appl = rsrc_mon->bcast_appl;
          else
             appl = rsrc_mon->usr_appl->usr_appl;

          memset(&cbk_info, 0, sizeof(NCS_SRMSV_RSRC_CBK_INFO));

          cbk_info.notif_type = SRMSV_CBK_NOTIF_RSRC_THRESHOLD;
          cbk_info.rsrc_mon_hdl = srmnd_msg->srma_rsrc_hdl;
          cbk_info.node_num = m_NCS_NODE_ID_FROM_MDS_DEST(*srmnd_dest);

          switch (rsrc_mon->rsrc_info.rsrc_type)
          {
          case NCS_SRMSV_RSRC_PROC_EXIT:
              cbk_info.notify.pid = srmnd_msg->info.notify_val.val.u_val32;
              cbk_info.notif_type = SRMSV_CBK_NOTIF_PROCESS_EXPIRED;

              if (cbk_info.notify.pid == rsrc_mon->rsrc_info.pid)
              {
                 /* now no need to maintain the resource-mon record in SRMA database
                    so delete it then. */
                 rc = srma_appl_delete_rsrc_mon(srma, srmnd_msg->srma_rsrc_hdl, FALSE);
                 if (rc != SA_AIS_OK)
                    return NCSCC_RC_FAILURE;         
              }
              break;

          default:
              cbk_info.notify.rsrc_value = srmnd_msg->info.notify_val;
              break;
          }
         
          /* Add the application notification message to cbk_list of an appl */
          rc = srma_update_appl_cbk_info(srma, &cbk_info, appl);
       }
       break;

   case SRMND_SYNCHRONIZED_MSG:
       /* synchronize SRMND specific SRMA resource monitor data with SRMND */
       rc = srma_update_sync_cbk_info(srma, srmnd_dest);       
       break;

   case SRMND_WATERMARK_VAL_MSG:
       {
          SRMA_USR_APPL_NODE *appl = NULL;
          NCS_SRMSV_RSRC_CBK_INFO cbk_info;

          /* retrieve application specific SRMSv record */
          if (!(appl = (SRMA_USR_APPL_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                                    srmnd_msg->srma_rsrc_hdl)))
          {
             /* No association record?? :-( ok log it & return error */
             m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                            SRMSV_LOG_HDL_FAILURE,
                            NCSFL_SEV_ERROR);
             return rc;
          }

          /* return application specific SRMSv handle */
          ncshm_give_hdl(srmnd_msg->srma_rsrc_hdl);

          memset(&cbk_info, 0, sizeof(NCS_SRMSV_RSRC_CBK_INFO));

          /* Update the CBK record */
          cbk_info.notif_type = SRMSV_CBK_NOTIF_WATERMARK_VAL;
          cbk_info.node_num = m_NCS_NODE_ID_FROM_MDS_DEST(*srmnd_dest);
          cbk_info.notify.watermarks = srmnd_msg->info.wm_val;

          /* Add the application notification message to cbk_list of 
             an application */
          rc = srma_update_appl_cbk_info(srma, &cbk_info, appl);
       }
       break;

   case SRMND_WATERMARK_EXIST_MSG:
       {
           NCS_SRMSV_RSRC_CBK_INFO cbk_info;
           SRMA_USR_APPL_NODE *appl = NULL;

           rsrc_mon = (SRMA_RSRC_MON*)ncs_patricia_tree_get(&srma->rsrc_mon_tree,
                                                 (uns8*)&srmnd_msg->srma_rsrc_hdl);
           if (!(rsrc_mon))              
           {
              /* No association record?? :-( ok log it & return error */
              m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                             SRMSV_LOG_HDL_FAILURE,
                             NCSFL_SEV_ERROR);
              return rc;
           }

           /* Get the associated appl record */
           if (rsrc_mon->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
              appl = rsrc_mon->bcast_appl;
           else
              appl = rsrc_mon->usr_appl->usr_appl;

           memset(&cbk_info, 0, sizeof(NCS_SRMSV_RSRC_CBK_INFO));

           /* Update the CBK record */
           cbk_info.notif_type = SRMSV_CBK_NOTIF_WM_CFG_ALREADY_EXIST;
           cbk_info.rsrc_mon_hdl = srmnd_msg->srma_rsrc_hdl;
           cbk_info.node_num = m_NCS_NODE_ID_FROM_MDS_DEST(*srmnd_dest);
           cbk_info.notify.rsrc_type = rsrc_mon->rsrc_info.rsrc_type;

           /* now no need to maintain the resource-mon record in SRMA database
              so delete it then. */
           rc = srma_appl_delete_rsrc_mon(srma, srmnd_msg->srma_rsrc_hdl, FALSE);
           if (rc != SA_AIS_OK)
              return NCSCC_RC_FAILURE;         

           /* Add the application notification message to cbk_list of 
              an application */
           rc = srma_update_appl_cbk_info(srma, &cbk_info, appl);
       }
       break;

   default:
       break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srma_del_srmnd_msg
 
  Description   :  This routine deletes the SRMND message data (that was 
                   arrived from SRMND).
 
  Arguments     :  msg - ptr to the SRMND message                   
 
  Return Values :  Nothing
 
  Notes         :  None
******************************************************************************/
void srma_del_srmnd_msg(SRMND_MSG *msg)
{
   /* msg ptr is NULL, so just return */
   if (!msg)
      return;

   /* If the message type is Bulk Create then delete rsrc's list data */
   if (msg->msg_type == SRMND_BULK_CREATED_MON_MSG)
   {
      SRMND_CREATED_RSRC_MON *rsrc = NULL, *next_rsrc = NULL;

      rsrc = msg->info.bulk_rsrc_mon.bulk_rsrc_mon;
      /* Delete all the rsrc specific data */
      while (rsrc)
      {
         next_rsrc = rsrc->next_srmnd_rsrc_mon;        
         /* Delete the rsrc node */
         m_MMGR_FREE_SRMND_CREATED_RSRC(rsrc);
         rsrc = next_rsrc;
      }

      msg->info.bulk_rsrc_mon.bulk_rsrc_mon = NULL;
   }

   /* Ok now delete the SRMA message */
   m_MMGR_FREE_SRMND_MSG(msg);

   return;
}


