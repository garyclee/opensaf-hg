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

  MODULE NAME: SRMND_DB.C
 
..............................................................................

  DESCRIPTION: This file contains the general routines that interfaces (reads
               /writes from/to the SRMND database.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srmnd.h"

/* Static Function Prototypes */
static uns32 srmnd_add_rsrc_mon_to_rsrc_type_list(SRMND_CB *srmnd, 
                                             SRMA_CREATE_RSRC_MON *create_mon,
                                             SRMND_RSRC_MON_NODE *rsrc,
                                             SRMND_RSRC_MON_NODE **wm_rsrc);
static uns32 srmnd_add_rsrc_mon_to_user_list(SRMND_CB *srmnd,
                                             SRMND_SRMA_USR_KEY *usr_key,
                                             SRMND_RSRC_MON_NODE *rsrc,
                                             NCS_BOOL stop_monitoring,
                                             NCS_BOOL bulk_create);
static SRMND_RSRC_MON_NODE *srmnd_check_rsrc_duplicate(SRMND_MON_SRMA_USR_NODE *user,
                                           uns32 rsrc_hdl);

static void srmnd_set_subscr_sent_flg(SRMND_RSRC_TYPE_NODE *rsrc_type_node);

/****************************************************************************
  Name          :  srmnd_del_appl_node
 
  Description   :  This routine deletes the respective application node from
                   the application specific list of SRMND.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   user_node - Ptr to the user application node
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_del_appl_node(SRMND_CB *srmnd, SRMND_MON_SRMA_USR_NODE *user_node)
{
   SRMND_MON_SRMA_USR_NODE *prev_srma_node = user_node->prev_srma_node;
   SRMND_MON_SRMA_USR_NODE *next_srma_node = user_node->next_srma_node;

   /* Check if the rsrc's are still hanging with user-appl node.. if they are
      so don't delete the appl node.. first delete all of its resources */
   if (user_node->start_rsrc_mon_node != NULL)
      return; 

   /* Check if it is the only node of the list */
   if ((prev_srma_node == NULL) && 
       (next_srma_node == NULL))
   {
      srmnd->start_srma_node = NULL;
   }
   else /* Check if it is the first node of the list */
   if (prev_srma_node == NULL) {
      srmnd->start_srma_node = next_srma_node;
      next_srma_node->prev_srma_node = NULL;
   }
   else /* Check if it is the last node of the list */
   if (next_srma_node == NULL) {
      prev_srma_node->next_srma_node = NULL;
   }
   else /* Middle node */
   {
      prev_srma_node->next_srma_node = next_srma_node;
      next_srma_node->prev_srma_node = prev_srma_node;
   }

   /* Free the user-application node */
   m_MMGR_FREE_SRMND_USER_NODE(user_node);

   return;
}


/****************************************************************************
  Name          :  srmnd_start_stop_appl_mon
 
  Description   :  This routine adds all the resources of an application.to 
                   monitoring process.                   

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   usr_key - Ptr to the user key of the application node
                   start_flg - TRUE for start monitoring
                               FALSE for stop monitoring

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_start_stop_appl_mon(SRMND_CB *srmnd,
                                SRMND_SRMA_USR_KEY *usr_key,
                                NCS_BOOL start_flg)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SRMND_MON_SRMA_USR_NODE *user_node = NULL;
   SRMND_RSRC_MON_NODE     *rsrc = NULL;

   /* Given the user-appl key, get the respective application node 
      if not exists, create the new appl node and update the appropriate 
      info */
   if ((user_node = srmnd_get_appl_node(srmnd, usr_key)) == NULL)
   {
      if ((user_node = m_MMGR_ALLOC_SRMND_USER_NODE) == NULL)
      {
         m_SRMND_LOG_MEM(SRMND_MEM_USER_NODE,
                         SRMND_MEM_ALLOC_FAILED,
                         NCSFL_SEV_CRITICAL);

         return NCSCC_RC_FAILURE;
      }

      m_NCS_OS_MEMSET((char *)user_node, 0, sizeof(SRMND_MON_SRMA_USR_NODE));

      /* Update the user specific key */
      user_node->usr_key = *usr_key; 

      /* Add the user-appl node to the user-appl list of SRMND */
      m_SRMND_ADD_APPL_NODE(srmnd, user_node);      
   }

   /* Set monitoring control parameter */
   if (start_flg)
      user_node->stop_monitoring = FALSE;
   else
      user_node->stop_monitoring = TRUE;
 
   rsrc = user_node->start_rsrc_mon_node;
   while (rsrc)
   {
      /* Reset the sent flag, such that it re-starts sending the threshold 
         events */
      rsrc->sent_flag &= ~(RSRC_APPL_NOTIF_SENT);

      /* Don't do either adding to or deleting from Monitoring, if the
         user type is SUBSCR type */
      if (rsrc->usr_type == NCS_SRMSV_USER_SUBSCRIBER)
      {
         /* Ptr to the next application resource */
         rsrc = rsrc->next_srma_usr_rsrc;
         continue;
      }

      if (start_flg)
      {
         /* Add the resource to monitoring process */
         if ((rc = srmnd_add_rsrc_for_monitoring(srmnd, rsrc, TRUE))
             != NCSCC_RC_SUCCESS)
         {
            m_SRMND_LOG_MON(SRMND_MON_RSRC_ADD,
                            SRMND_MON_FAILED,
                            NCSFL_SEV_ERROR);
            return rc;
         }
      }
      else
      {
         /* Delete the resource from monitoring process */
         srmnd_del_rsrc_from_monitoring(srmnd->srmnd_tmr_cb, &rsrc->tmr_id);
      }

      /* Ptr to the next application resource */
      rsrc = rsrc->next_srma_usr_rsrc;
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_del_srma_rsrc_mon
 
  Description   :  This routine deletes the resource from SRMND database. 
                   No monitoring is done on this resource.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   rsrc_hdl - resource specific handle
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_del_srma_rsrc_mon(SRMND_CB *srmnd, 
                             NCS_BOOL bcast,
                             SRMND_SRMA_USR_KEY *usr_key,
                             uns32 rsrc_hdl)
{
   uns32 rsrc_mon_hdl = 0;

   if (bcast)
   {
      SRMND_MON_SRMA_USR_NODE *usr = NULL;
      SRMND_RSRC_MON_NODE *rsrc = SRMND_RSRC_MON_NODE_NULL;

      /* Check whether the respective user node already exists */
      usr = srmnd_get_appl_node(srmnd, usr_key);
      if (usr)
      {
         /* Check whether rsrc exists or not */
         if ((rsrc = srmnd_check_rsrc_duplicate(usr, rsrc_hdl)) == NULL)
         {
            /* No association record?? :-( ok log it & return error */
            m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                            SRMSV_LOG_HDL_FAILURE,
                            NCSFL_SEV_ERROR);
            return;
         }
         
         rsrc_mon_hdl = rsrc->rsrc_mon_hdl;
      }
      else
         return;
   }
   else
      rsrc_mon_hdl = rsrc_hdl;

   srmnd_del_rsrc_mon(srmnd, rsrc_mon_hdl);

   return;
}


/****************************************************************************
  Name          :  srmnd_del_rsrc_mon
 
  Description   :  This routine deletes the resource from SRMND database. 
                   No monitoring is done on this resource.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   rsrc_hdl - resource specific handle
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_del_rsrc_mon(SRMND_CB *srmnd, uns32 rsrc_hdl)
{
   SRMND_RSRC_MON_NODE *rsrc = SRMND_RSRC_MON_NODE_NULL;

   /* retrieve application specific SRMSv record */
   if (!(rsrc = (SRMND_RSRC_MON_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                                      rsrc_hdl)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_ERROR);
      return;
   }

   /* Delete the resource from Resource Type List */
   if (rsrc->rsrc_type_node)
      srmnd_del_rsrc_mon_from_rsrc_type_list(srmnd, rsrc);

   /* Delete the resource from user-application List */
   if (rsrc->srma_usr_node)
      srmnd_del_rsrc_mon_from_user_list(srmnd, rsrc);

   /* Delete any PID data from Resource */
   if (rsrc->descendant_pids)
      srmnd_remove_rsrc_pid_data(rsrc);

   /* Delete the respective resource mon node from the resource-mon tree */
   ncs_patricia_tree_del(&srmnd->rsrc_mon_tree, &rsrc->rsrc_mon_pat_node);

   /* Delete the resource mon from monitoring process */
   srmnd_del_rsrc_from_monitoring(srmnd->srmnd_tmr_cb, &rsrc->tmr_id);
   
   /* Delete any sample data */
   if (rsrc->mon_data.sample_data != NULL)
      srmnd_delete_rsrc_sample_data(rsrc);

   /* release Application specific handle */
   ncshm_give_hdl(rsrc_hdl);
  
   ncshm_destroy_hdl(NCS_SERVICE_ID_SRMND, rsrc->rsrc_mon_hdl);

   /* Delete SRMA Control Block */
   m_MMGR_FREE_SRMND_RSRC_MON(rsrc);

   return;
}


/****************************************************************************
  Name          :  srmnd_add_rsrc_mon_to_user_list
 
  Description   :  This routine checks whether the user-application node 
                   exists in the resource type list of SRMND.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   usr_key - user key of the application node
                   rsrc - resource monitor record to add
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_add_rsrc_mon_to_user_list(SRMND_CB *srmnd,
                                      SRMND_SRMA_USR_KEY *usr_key,
                                      SRMND_RSRC_MON_NODE *rsrc,
                                      NCS_BOOL stop_monitoring,
                                      NCS_BOOL bulk_create)
{
   SRMND_MON_SRMA_USR_NODE *user = NULL;

   /* Check whether the respective user node already exists */
   user = srmnd_get_appl_node(srmnd, usr_key);
   
   /* If not exists */
   if (!(user))
   {
      if ((user = m_MMGR_ALLOC_SRMND_USER_NODE) == NULL)
      {
         m_SRMND_LOG_MEM(SRMND_MEM_USER_NODE,
                         SRMND_MEM_ALLOC_FAILED,
                         NCSFL_SEV_CRITICAL);

         return NCSCC_RC_FAILURE;
      }

      m_NCS_OS_MEMSET((char *)user, 0, sizeof(SRMND_MON_SRMA_USR_NODE));

      /* Update the user specific key */
      user->usr_key = *usr_key; 

      /* Add the user-appl node to the user-appl list of SRMND */
      m_SRMND_ADD_APPL_NODE(srmnd, user);      
   }

   if (user->start_rsrc_mon_node != NULL)      
   {
      /* Add new application node to the begining of the application list */
      rsrc->next_srma_usr_rsrc = user->start_rsrc_mon_node;
      user->start_rsrc_mon_node->prev_srma_usr_rsrc = rsrc;   
   }
   
   user->start_rsrc_mon_node = rsrc;
   rsrc->srma_usr_node = user;

   /* Should be updated only in case, if the create is of BULK type */
   if (bulk_create)
      user->stop_monitoring = stop_monitoring;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmnd_add_rsrc_mon_to_rsrc_type_list
 
  Description   :  This routine checks whether the resource type specific 
                   node exists in the resource type list of SRMND.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   usr_key - user key of the application node
                   rsrc - resource monitor record to add
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_add_rsrc_mon_to_rsrc_type_list(SRMND_CB *srmnd, 
                                           SRMA_CREATE_RSRC_MON *create_mon,
                                           SRMND_RSRC_MON_NODE *rsrc, 
                                           SRMND_RSRC_MON_NODE **wm_rsrc)
{
   SRMND_RSRC_TYPE_NODE *rsrc_type_node = SRMND_RSRC_TYPE_NODE_NULL;
   NCS_SRMSV_RSRC_TYPE rsrc_type;
   uns32 pid;

   rsrc_type = create_mon->mon_info.rsrc_info.rsrc_type;
   pid = create_mon->mon_info.rsrc_info.pid;
   *wm_rsrc = NULL;
 
   /* Check whether the respective user node already exists */
   rsrc_type_node = srmnd_check_rsrc_type_node(srmnd, rsrc_type, pid);
   
   /* If not exists */
   if (!(rsrc_type_node))
   {
      if ((rsrc_type_node = m_MMGR_ALLOC_SRMND_RSRC_TYPE) == NULL)
      {
         m_SRMND_LOG_MEM(SRMND_MEM_RSRC_TYPE,
                         SRMND_MEM_ALLOC_FAILED,
                         NCSFL_SEV_CRITICAL);

         return NCSCC_RC_FAILURE;
      }

      m_NCS_OS_MEMSET((char *)rsrc_type_node, 0, sizeof(SRMND_RSRC_TYPE_NODE));

      /* Update the user specific key */
      rsrc_type_node->rsrc_type = rsrc_type;

      /* Add the rsrc-type node to the rsrc-type list of SRMND */
      m_SRMND_ADD_RSRC_TYPE_NODE(srmnd, rsrc_type_node);      
   }

   rsrc_type_node->pid = pid;
   rsrc_type_node->child_level = create_mon->mon_info.rsrc_info.child_level;
   rsrc->rsrc_type_node = rsrc_type_node;

   /* Not required to add to resource type list if monitor type is of watermark */
   if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_WATERMARK)
   {
      if (rsrc_type_node->watermark_rsrc == NULL)          
         rsrc_type_node->watermark_rsrc = rsrc;
      else
      {
         /* LOG message, some thing wrong.. must have detected much before */
         return NCSCC_RC_FAILURE;
      }

      return NCSCC_RC_SUCCESS;
   }

   /*
    * Following lines are purely meant for threshold monitoring
    */
   switch (rsrc->usr_type)
   {
   case NCS_SRMSV_USER_REQUESTOR:
   case NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR:
       if (rsrc_type_node->start_rsrc_mon_list != NULL)          
       {
          /* Add new application node to the begining of the application list */
          rsrc->next_rsrc_type_ptr = rsrc_type_node->start_rsrc_mon_list;
          rsrc_type_node->start_rsrc_mon_list->prev_rsrc_type_ptr = rsrc;          
       }
       
       rsrc_type_node->start_rsrc_mon_list = rsrc;

       break;

   case NCS_SRMSV_USER_SUBSCRIBER:
       if (rsrc_type_node->start_rsrc_subs_list != NULL)          
       {
          /* Add new application node to the begining of the application list */
          rsrc->next_rsrc_type_ptr = rsrc_type_node->start_rsrc_subs_list;
          rsrc_type_node->start_rsrc_subs_list->prev_rsrc_type_ptr = rsrc;          
       }

       rsrc_type_node->start_rsrc_subs_list = rsrc;
       break;

   default:
       break;
   }
   
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmnd_check_rsrc_duplicate
 
  Description   :  This routine checks whether appl rsrc-mon already created,
                   if created.. it returns TRUE else it returns FALSE 

  Arguments     :  user - ptr to the SRMND_MON_SRMA_USR_NODE struct node
                   rsrc_hdl - rsrc handle to be searched
                   
  Return Values :  TRUE / FALSE
 
  Notes         :  None.
******************************************************************************/
SRMND_RSRC_MON_NODE *srmnd_check_rsrc_duplicate(SRMND_MON_SRMA_USR_NODE *user,
                                                uns32 rsrc_hdl)
{
   SRMND_RSRC_MON_NODE *rsrc = NULL;

   rsrc = user->start_rsrc_mon_node;
   /* Traverse through all the application specific rsrc's */
   while (rsrc)
   {
      /* if their handle matches.. then return TRUE */
      if (rsrc->srma_rsrc_hdl == rsrc_hdl)
         return rsrc;

      /* Not matched?? ok.. go for next rsrc */
      rsrc = rsrc->next_srma_usr_rsrc;  
   }

   return NULL;
}


/****************************************************************************
  Name          :  srmnd_set_subscr_sent_flg
 
  Description   :  This routine sets the subscr not-sent flag for all the 
                   resources of the same type.

  Arguments     :  rsrc_type_node - ptr to the  SRMND_RSRC_TYPE_NODE struct
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_set_subscr_sent_flg(SRMND_RSRC_TYPE_NODE *rsrc_type_node)
{
  SRMND_RSRC_MON_NODE *rsrc = rsrc_type_node->start_rsrc_mon_list;

  /* Set subscr not-sent flag to all the resource of the list */
  while (rsrc)
  {
     /* The subscriber will be notified about the generated threshold event 
        if the sent flag of rsrc is not yet set to NOTIF-SENT type. Hence it 
        is not required to set the subscr not-sent flag if the sent flag is 
        not set to NOTIF-SENT type.
     */
     if (rsrc->sent_flag & RSRC_APPL_NOTIF_SENT)
        rsrc->sent_flag |= RSRC_APPL_SUBS_SENT_PENDING; 

     rsrc = rsrc->next_rsrc_type_ptr; 
  }
  
  return; 
}


/****************************************************************************
  Name          :  srmnd_create_rsrc_mon
 
  Description   :  This routine creates a resource monitor record in the
                   SRMND database w.r.t user-application. SRMSv starts 
                   monitoring this resource.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   bcast - is it a bcast request from SRMA (TRUE / FALSE)
                   usr_key - user key of the application node
                   create-mon - ptr to the SRMA_CREATE_RSRC_MON struct
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_create_rsrc_mon(SRMND_CB *srmnd,
                            NCS_BOOL bcast,
                            SRMND_SRMA_USR_KEY *usr_key,
                            SRMA_CREATE_RSRC_MON *create_mon,
                            NCS_BOOL stop_monitoring,
                            NCS_BOOL bulk_create)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SRMND_RSRC_MON_NODE *rsrc = SRMND_RSRC_MON_NODE_NULL;
   NCS_SRMSV_USER_TYPE user_type = 0;
   NCS_SRMSV_RSRC_TYPE rsrc_type = 0;
   SRMND_MON_SRMA_USR_NODE *usr = NULL;
   SRMND_RSRC_MON_NODE *wm_rsrc = NULL;
   SRMND_RSRC_TYPE_NODE *rsrc_type_node = NULL;

   if (!(create_mon))
      return NCSCC_RC_FAILURE;

   user_type = create_mon->mon_info.rsrc_info.usr_type;
   rsrc_type = create_mon->mon_info.rsrc_info.rsrc_type;

   /* Check whether the respective user node already exists */
   usr = srmnd_get_appl_node(srmnd, usr_key);
   if (usr)
   {
      /* Check whether rsrc already created */
      if (srmnd_check_rsrc_duplicate(usr, create_mon->rsrc_hdl) != NULL)
         return NCSCC_RC_SUCCESS;
   }

   /* If it is a watermark cfg, pl. check whether the cfg exists already for the rsrc */
   if (create_mon->mon_info.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_WATERMARK)
   {
      rsrc_type_node = srmnd_check_rsrc_type_node(srmnd, rsrc_type, create_mon->mon_info.rsrc_info.pid);
      if ((rsrc_type_node != NULL) && (rsrc_type_node->watermark_rsrc))
      {
         SRMND_WATERMARK_DATA wm_data;

         m_NCS_OS_MEMSET(&wm_data, 0, sizeof(SRMND_WATERMARK_DATA));

         wm_data.srma_dest = usr_key->srma_dest;
         wm_data.usr_hdl = create_mon->rsrc_hdl;

         srmnd_send_msg(srmnd, (NCSCONTEXT)&wm_data, SRMND_WATERMARK_EXIST_MSG);      
         return NCSCC_RC_SUCCESS;
      }
   }

   /* Allocate the memory for resource monitor */
   if ((rsrc = m_MMGR_ALLOC_SRMND_RSRC_MON) == NULL)
   {
      m_SRMND_LOG_MEM(SRMND_MEM_RSRC_INFO,
                      SRMND_MEM_ALLOC_FAILED,
                      NCSFL_SEV_CRITICAL);

      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET((char *)rsrc, 0, sizeof(SRMND_RSRC_MON_NODE));

   /* create the handle for rsrc mon record (association with hdl-mngr) */
   if (!(rsrc->rsrc_mon_hdl = ncshm_create_hdl(srmnd->pool_id,
                                               NCS_SERVICE_ID_SRMND, 
                                               (NCSCONTEXT)rsrc)))
   {
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_CREATE_RSRC_MON,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_CRITICAL); 

      /* Delete the allocated SRMA Control Block */
      m_MMGR_FREE_SRMND_RSRC_MON(rsrc);

      return NCSCC_RC_FAILURE;      
   }
   m_SRMND_LOG_HDL(SRMSV_LOG_HDL_CREATE_RSRC_MON,
                   SRMSV_LOG_HDL_SUCCESS,
                   NCSFL_SEV_INFO);

   rsrc->usr_type = user_type;
   rsrc->srma_rsrc_hdl = create_mon->rsrc_hdl;

   /* Add to patricia tree */
   rsrc->rsrc_mon_pat_node.key_info = (uns8 *)&rsrc->rsrc_mon_hdl;
   if (ncs_patricia_tree_add(&(srmnd->rsrc_mon_tree), &(rsrc->rsrc_mon_pat_node))
       != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_TREE(SRMSV_LOG_PAT_ADD,
                       SRMSV_LOG_PAT_FAILURE,
                       NCSFL_SEV_ERROR);

      srmnd_del_rsrc_mon(srmnd, rsrc->rsrc_mon_hdl);

      return NCSCC_RC_FAILURE;     
   }
   
   /* Only these users will have the monitor information */
   if ((user_type == NCS_SRMSV_USER_REQUESTOR) ||
       (user_type == NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR))
      rsrc->mon_data.monitor_data = create_mon->mon_info.monitor_data;

   /* Add the resource monitor record to user list */
   if ((rc = srmnd_add_rsrc_mon_to_user_list(srmnd, usr_key, rsrc, stop_monitoring, bulk_create)) != NCSCC_RC_SUCCESS)
   {
      srmnd_del_rsrc_mon(srmnd, rsrc->rsrc_mon_hdl);
      return rc;
   }
   
   /* Add the resource monitor record to resource type list */
   if ((rc = srmnd_add_rsrc_mon_to_rsrc_type_list(srmnd,
                                                  create_mon,
                                                  rsrc,
                                                  &wm_rsrc)) != NCSCC_RC_SUCCESS)
   {
      srmnd_del_rsrc_mon(srmnd, rsrc->rsrc_mon_hdl);
      return rc;
   }
   

   /* if the resource mon is process specific then update the child PID info */
   switch (rsrc_type)
   {
   case NCS_SRMSV_RSRC_PROC_EXIT:
       if (create_mon->mon_info.rsrc_info.child_level != 0)
       {
          if ((rc = srmnd_update_pid_data(rsrc) != NCSCC_RC_SUCCESS))
          {
             srmnd_del_rsrc_mon(srmnd, rsrc->rsrc_mon_hdl);
             return rc;
          }
       }
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL: 
   case NCS_SRMSV_RSRC_CPU_KERNEL:
   case NCS_SRMSV_RSRC_CPU_USER: 
   case NCS_SRMSV_RSRC_CPU_UTIL_ONE:
   case NCS_SRMSV_RSRC_CPU_UTIL_FIVE:
   case NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN:
   case NCS_SRMSV_RSRC_PROC_CPU:
       if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_THRESHOLD)
       {  
          m_SRMND_UPDATE_VALUE(rsrc->mon_data.monitor_data.mon_cfg.threshold.threshold_val,
                               NCS_SRMSV_VAL_TYPE_FLOAT,
                               NCS_SRMSV_STAT_SCALE_PERCENT); 
       }

       m_SRMND_UPDATE_VALUE(rsrc->mon_data.result_value,
                            NCS_SRMSV_VAL_TYPE_FLOAT,
                            NCS_SRMSV_STAT_SCALE_PERCENT); 
       break;

   case NCS_SRMSV_RSRC_MEM_PHYSICAL_USED:
   case NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE:
   case NCS_SRMSV_RSRC_SWAP_SPACE_USED:
   case NCS_SRMSV_RSRC_SWAP_SPACE_FREE:
   case NCS_SRMSV_RSRC_USED_BUFFER_MEM:
   case NCS_SRMSV_RSRC_USED_CACHED_MEM:
   case NCS_SRMSV_RSRC_PROC_MEM:
       if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_THRESHOLD)
       {
          m_SRMND_UPDATE_VALUE(rsrc->mon_data.result_value,
                               rsrc->mon_data.monitor_data.mon_cfg.threshold.threshold_val.val_type,
                               rsrc->mon_data.monitor_data.mon_cfg.threshold.threshold_val.scale_type);
       }
       else
       if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_WATERMARK)
       {
          m_SRMND_UPDATE_VALUE(rsrc->mon_data.result_value,
                               NCS_SRMSV_VAL_TYPE_UNS32,
                               NCS_SRMSV_STAT_SCALE_KBYTE); 
       }  
       break;

   default:
       break;
   }

   if (wm_rsrc)
   {
      /* Delete the resource mon from monitoring process */
      srmnd_del_rsrc_from_monitoring(srmnd->srmnd_tmr_cb, &wm_rsrc->tmr_id);
   }

   /* Only these users will have the monitor information */
   if ((user_type == NCS_SRMSV_USER_REQUESTOR) ||
       (user_type == NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR))
   {
      time_t monitoring_interval = create_mon->mon_info.monitor_data.monitoring_interval;

      /* Monitoring interval is defined?? so update the resource mon expiry
         period then */
      if (monitoring_interval)
      {
         time_t now;
         m_GET_TIME_STAMP(now);
         rsrc->rsrc_mon_expiry = now + monitoring_interval;
      }

      /* Add the resource to monitoring process */
      if ((rc = srmnd_add_rsrc_for_monitoring(srmnd, rsrc, TRUE)) != NCSCC_RC_SUCCESS)
      {
         m_SRMND_LOG_MON(SRMND_MON_RSRC_ADD,
                         SRMND_MON_FAILED,
                         NCSFL_SEV_ERROR);

         srmnd_del_rsrc_mon(srmnd, rsrc->rsrc_mon_hdl);

         return rc;
      }
   }
   else /* For a purely subscriber */
   {
      rsrc->sent_flag |= RSRC_APPL_SUBS_SENT_PENDING; 
      srmnd_set_subscr_sent_flg(rsrc->rsrc_type_node);
   }
 
   /* Only if it is a unicast CREATE request, then send CREATED ack to the source */
   if ((bcast != TRUE) && (bulk_create != TRUE))
   {
      srmnd_send_msg(srmnd, (NCSCONTEXT)rsrc, SRMND_CREATED_MON_MSG);      
   }
   
   return rc;
}


/****************************************************************************
  Name          :  srmnd_modify_rsrc_mon
 
  Description   :  This routine modifies the data of a resource monitor 
                   record.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   modify_mon - ptr to the SRMA_MODIFY_RSRC_MON struct
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_modify_rsrc_mon(SRMND_CB *srmnd,
                            NCS_BOOL bcast,
                            SRMND_SRMA_USR_KEY *usr_key,
                            SRMA_MODIFY_RSRC_MON *modify_mon)
{
   SRMND_RSRC_MON_NODE *rsrc = SRMND_RSRC_MON_NODE_NULL;
   NCS_SRMSV_USER_TYPE user_type = 0;
   NCS_SRMSV_RSRC_TYPE rsrc_type = 0;
   time_t monitoring_interval = 0;
   NCS_SRMSV_MONITOR_TYPE monitor_type = 0;
   uns32 i_pid = 0;
   int   child_level = 0;
   uns32  rc = NCSCC_RC_SUCCESS;
   NCS_BOOL rsrc_type_changed = FALSE, add_rsrc_monitoring = FALSE;
   uns32 *rsrc_samples = NULL, samples = 0;
   SRMND_RSRC_MON_NODE *wm_rsrc = NULL;


   if (!(modify_mon))
      return NCSCC_RC_FAILURE;

   /* Get the monitor parameters */
   user_type           = modify_mon->mon_info.rsrc_info.usr_type;
   rsrc_type           = modify_mon->mon_info.rsrc_info.rsrc_type;
   monitoring_interval = modify_mon->mon_info.monitor_data.monitoring_interval;
   monitor_type        = modify_mon->mon_info.monitor_data.monitor_type;
   i_pid               = modify_mon->mon_info.rsrc_info.pid;
   child_level         = modify_mon->mon_info.rsrc_info.child_level;

   if (monitor_type == NCS_SRMSV_MON_TYPE_THRESHOLD)
      samples = modify_mon->mon_info.monitor_data.mon_cfg.threshold.samples;
   
   if (bcast)
   {
      SRMND_MON_SRMA_USR_NODE *usr = NULL;
      SRMND_RSRC_MON_NODE *rsrc = SRMND_RSRC_MON_NODE_NULL;

      /* Check whether the respective user node already exists */
      usr = srmnd_get_appl_node(srmnd, usr_key);
      if (usr)
      {
         /* Check whether rsrc already created */
         if ((rsrc = srmnd_check_rsrc_duplicate(usr, modify_mon->rsrc_hdl)) == NULL)
         {
            /* No association record?? :-( ok log it & return error */
            m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                            SRMSV_LOG_HDL_FAILURE,
                            NCSFL_SEV_ERROR);
            return NCSCC_RC_FAILURE;
         }
      }

      modify_mon->rsrc_hdl = rsrc->rsrc_mon_hdl;
   }

   /* retrieve application specific SRMSv record */
   if (!(rsrc = (SRMND_RSRC_MON_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                                      modify_mon->rsrc_hdl)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }
  
   if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_THRESHOLD)
      rsrc_samples = &rsrc->mon_data.monitor_data.mon_cfg.threshold.samples;

   /* Set the flag if there is a change in the rsrc type */
   if (rsrc_type != rsrc->rsrc_type_node->rsrc_type)
      rsrc_type_changed = TRUE;

   /* If resource type is modified then delete the resource from the respective
      resource type list and add it to the modified resource type specific list */
   if ((rsrc_type_changed) || (user_type != rsrc->usr_type))
   {   
      if (user_type == NCS_SRMSV_USER_REQUESTOR)
      {
         /* Delete the resource mon from monitoring process */
         srmnd_del_rsrc_from_monitoring(srmnd->srmnd_tmr_cb, &rsrc->tmr_id);

         m_NCS_OS_MEMSET(&rsrc->mon_data.monitor_data, 0,
                         sizeof(NCS_SRMSV_MON_DATA));
      }

      /* Delete the resource from Resource Type List */
      srmnd_del_rsrc_mon_from_rsrc_type_list(srmnd, rsrc);

      rsrc->usr_type = user_type;

      if (srmnd_add_rsrc_mon_to_rsrc_type_list(srmnd,
                                               modify_mon,
                                               rsrc,
                                               &wm_rsrc) != NCSCC_RC_SUCCESS)
      {
         /* release Application specific handle */
         ncshm_give_hdl(modify_mon->rsrc_hdl);

         /* Debug sink */
         return NCSCC_RC_FAILURE;
      }
   }

   /* Check for monitor data modifications */
   if ((user_type == NCS_SRMSV_USER_SUBSCRIBER) ||
       (user_type == NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR))
   {
      time_t monitoring_rate = modify_mon->mon_info.monitor_data.monitoring_rate;
    
      /* Reset the sent flag, such that it starts sending the threshold 
         events based on the modified values */
      rsrc->sent_flag &= ~(RSRC_APPL_NOTIF_SENT);

      if (monitoring_rate != rsrc->mon_data.monitor_data.monitoring_rate) 
      {
         /* Delete the resource mon from monitoring process */
         srmnd_del_rsrc_from_monitoring(srmnd->srmnd_tmr_cb, &rsrc->tmr_id);

         rsrc->mon_data.monitor_data.monitoring_rate = monitoring_rate;
       
         /* Set this flag in order to add the rsrc for monitoring at the 
            end of this function */
         add_rsrc_monitoring = TRUE;         
      }
   }

   /* Change in sample's.. so delete the exisiting sample update structures */
   if ((rsrc_samples != NULL) && (samples != *rsrc_samples))
   {
      if (rsrc->mon_data.sample_data != NULL)
         srmnd_delete_rsrc_sample_data(rsrc);

      *rsrc_samples = samples;
   }

   /* Monitor type itself changed.. ok take appropriate actions */
   if (monitor_type != rsrc->mon_data.monitor_data.monitor_type)
   {
      rsrc->mon_data.monitor_data.monitor_type = monitor_type;

      if (monitor_type == NCS_SRMSV_MON_TYPE_WATERMARK)
      {
         if (rsrc->mon_data.sample_data != NULL)
            srmnd_delete_rsrc_sample_data(rsrc);
         
         rsrc->mon_data.next_sample_to_update = 0;
         *rsrc_samples = 0;
      }
   }

   switch (rsrc->rsrc_type_node->rsrc_type)
   {
   case NCS_SRMSV_RSRC_PROC_EXIT:
       /* PID is 0 then return failure */
       if (!(i_pid))
       {
          /* release Application specific handle */
          ncshm_give_hdl(modify_mon->rsrc_hdl);
          return NCSCC_RC_FAILURE;
       }

       /* Modified PID info, then update it */
       if ((i_pid != rsrc->rsrc_type_node->pid) ||
           (rsrc->rsrc_type_node->child_level != child_level) ||
           (rsrc_type_changed))
       {
          rsrc->rsrc_type_node->pid = i_pid;
          rsrc->rsrc_type_node->child_level = child_level;

          /* Delete any PID data from Resource */
          if (rsrc->descendant_pids)
             srmnd_remove_rsrc_pid_data(rsrc);

          rsrc->pid_num = 0;
           
          /* Need to update and monitor the child pids too */
          if (rsrc->rsrc_type_node->child_level != 0)
          {
             /* Update it with pid descendants */
             if ((rc = srmnd_update_pid_data(rsrc)) != NCSCC_RC_SUCCESS)
             {
                /* release Application specific handle */
                ncshm_give_hdl(modify_mon->rsrc_hdl);
                return rc;
             }
          }
       }
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL: 
   case NCS_SRMSV_RSRC_CPU_KERNEL:
   case NCS_SRMSV_RSRC_CPU_USER: 
   case NCS_SRMSV_RSRC_CPU_UTIL_ONE:
   case NCS_SRMSV_RSRC_CPU_UTIL_FIVE:
   case NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN:
   case NCS_SRMSV_RSRC_PROC_CPU:
       if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_THRESHOLD)
       {  
          /* Val type and scale type for CPU specific rsrc'c */
          NCS_SRMSV_VALUE *rsrc_value = &rsrc->mon_data.result_value;
          NCS_SRMSV_VALUE *rsrc_thresold_val = &rsrc->mon_data.monitor_data.mon_cfg.threshold.threshold_val;

          rsrc->mon_data.monitor_data.mon_cfg.threshold.threshold_val = modify_mon->mon_info.monitor_data.mon_cfg.threshold.threshold_val;

          rsrc_value->val_type = NCS_SRMSV_VAL_TYPE_FLOAT;
          rsrc_value->scale_type = NCS_SRMSV_STAT_SCALE_PERCENT;
          rsrc_thresold_val->val_type = NCS_SRMSV_VAL_TYPE_FLOAT;
          rsrc_thresold_val->scale_type = NCS_SRMSV_STAT_SCALE_PERCENT;
       }
       break;

   default:
       if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_THRESHOLD)
       {
          /* Val type and scale type for any other rsrc'c */
          NCS_SRMSV_VALUE *rsrc_value = &rsrc->mon_data.result_value;

          rsrc->mon_data.monitor_data.mon_cfg.threshold.threshold_val = modify_mon->mon_info.monitor_data.mon_cfg.threshold.threshold_val;

          rsrc_value->val_type = rsrc->mon_data.monitor_data.mon_cfg.threshold.threshold_val.val_type;
          rsrc_value->scale_type = rsrc->mon_data.monitor_data.mon_cfg.threshold.threshold_val.scale_type;
       }
       break;
   }

   /* Monitoring interval is modified then update it in resource monitor records */
   if (monitoring_interval != rsrc->mon_data.monitor_data.monitoring_interval)
   {
      rsrc->mon_data.monitor_data.monitoring_interval = monitoring_interval;

      if (rsrc->mon_data.monitor_data.monitoring_interval)
      {
         time_t now;
         m_GET_TIME_STAMP(now);
         rsrc->rsrc_mon_expiry = 
             (now + monitoring_interval);
      }
   }

   if (wm_rsrc)
   {
      /* Delete the resource mon from monitoring process */
      srmnd_del_rsrc_from_monitoring(srmnd->srmnd_tmr_cb, &wm_rsrc->tmr_id);
   }

   /* Add the resource to monitoring process */
   if (add_rsrc_monitoring)
   {
      if ((rc = srmnd_add_rsrc_for_monitoring(srmnd, rsrc, TRUE)) != NCSCC_RC_SUCCESS)
      {
         m_SRMND_LOG_MON(SRMND_MON_RSRC_ADD,
                         SRMND_MON_FAILED,
                         NCSFL_SEV_ERROR);

         /* release Application specific handle */
         ncshm_give_hdl(modify_mon->rsrc_hdl);

         return rc;
      }
   }

   /* release Application specific handle */
   ncshm_give_hdl(modify_mon->rsrc_hdl);

   return rc;
}


/****************************************************************************
  Name          :  srmnd_bulk_create_rsrc_mon
 
  Description   :  This routine creates a mulitple resource monitor records
                   in the SRMND database w.r.t user-application. SRMSv starts 
                   monitoring these resources.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   bcast - is it a bcast request from SRMA (TRUE / FALSE)
                   usr_key - user key of the application node
                   bulk_create_mon - ptr to the SRMA_BULK_CREATE_RSRC_MON
                                     struct
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_bulk_create_rsrc_mon(SRMND_CB *srmnd,
                                 NCS_BOOL bcast,
                                 SRMND_SRMA_USR_KEY *usr_key,
                                 SRMA_BULK_CREATE_RSRC_MON *bulk_create_mon)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SRMA_CREATE_RSRC_MON *create_mon = NULL;
   SRMA_CREATE_RSRC_MON_NODE *srma_rsrc_mon = NULL;
   SRMND_MON_SRMA_USR_NODE *user = NULL;
  
   if (!(bulk_create_mon))
      return NCSCC_RC_FAILURE;

   srma_rsrc_mon = bulk_create_mon->bulk_rsrc;

   /* Traverse through all the config list and create the respective resource
      monitor records */
   while (srma_rsrc_mon)
   {
      create_mon = &srma_rsrc_mon->rsrc;
      if ((rc = srmnd_create_rsrc_mon(srmnd,
                                      bcast,
                                      usr_key,
                                      create_mon, 
                                      bulk_create_mon->stop_monitoring,
                                      TRUE)) != NCSCC_RC_SUCCESS)
      {
         m_SRMND_LOG_RSRC_MON(SRMSV_LOG_RSRC_MON_CREATE,
                              SRMSV_LOG_RSRC_MON_FAILED,
                              NCSFL_SEV_ERROR);
      }
    
      srma_rsrc_mon = srma_rsrc_mon->next_rsrc_mon;
   }

   /* get the respective user node */
   if (bcast != TRUE)
   {
      user = srmnd_get_appl_node(srmnd, usr_key);
      if (user != NULL)
      {
         srmnd_send_msg(srmnd, (NCSCONTEXT)user, SRMND_BULK_CREATED_MON_MSG);          
      }
      else
      {
         m_SRMND_LOG_MISC(SRMND_LOG_MISC_GET_APPL,
                          SRMND_LOG_MISC_FAILED,
                          NCSFL_SEV_NOTICE);
      }
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_del_appl_rsrc
 
  Description   :  This routine deletes the respective rsrc mon data from the
                   SRMND database.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   user_node - ptr to the SRMND_MON_SRMA_USR_NODE struct                   
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_del_appl_rsrc(SRMND_CB *srmnd, SRMND_MON_SRMA_USR_NODE *user_node)
{
   SRMND_RSRC_MON_NODE *rsrc = NULL;
   SRMND_RSRC_MON_NODE *next_rsrc = NULL;

   rsrc = user_node->start_rsrc_mon_node;
   while (rsrc)
   {
      next_rsrc = rsrc->next_srma_usr_rsrc;

      /* Delete the resource from monitoring process */
      srmnd_del_rsrc_from_monitoring(srmnd->srmnd_tmr_cb, &rsrc->tmr_id);        

      /* Delete any sample data */
      if (rsrc->mon_data.sample_data != NULL)
         srmnd_delete_rsrc_sample_data(rsrc);

      /* Delete the resource from Resource Type List */
      if (rsrc->rsrc_type_node)
         srmnd_del_rsrc_mon_from_rsrc_type_list(srmnd, rsrc);

      /* Delete any PID data from Resource */
      if (rsrc->descendant_pids)
         srmnd_remove_rsrc_pid_data(rsrc);

      /* Delete the respective resource mon node from the resource-mon tree */
      ncs_patricia_tree_del(&srmnd->rsrc_mon_tree, &rsrc->rsrc_mon_pat_node);

      ncshm_destroy_hdl(NCS_SERVICE_ID_SRMND, rsrc->rsrc_mon_hdl);

      /* Delete SRMA Control Block */
      m_MMGR_FREE_SRMND_RSRC_MON(rsrc); 

      rsrc = next_rsrc;
   }

   /* All the resources of the application has been deleted */
   user_node->start_rsrc_mon_node = NULL;

   return;
}


/****************************************************************************
  Name          :  srmnd_unreg_appl
 
  Description   :  This routine un registers the application from the SRMND
                   process. Monitoring process is stopped on all the 
                   resources of an application and deletes the respective
                   data from SRMND database.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   usr_key - user key of the application node                   
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_unreg_appl(SRMND_CB *srmnd,
                       SRMND_SRMA_USR_KEY *usr_key)
{
   SRMND_MON_SRMA_USR_NODE *user_node = SRMND_MON_SRMA_USR_NODE_NULL;   

   if ((user_node = srmnd_get_appl_node(srmnd, usr_key)) == NULL)
   {
      m_SRMND_LOG_MISC(SRMND_LOG_MISC_GET_APPL,
                       SRMND_LOG_MISC_FAILED,
                       NCSFL_SEV_NOTICE);
      return NCSCC_RC_FAILURE;
   }

   /* Delete all the configured resources of an application */
   srmnd_del_appl_rsrc(srmnd, user_node);

   /* Now delete the application node itself */
   srmnd_del_appl_node(srmnd, user_node);
   
   return NCSCC_RC_SUCCESS;
}
