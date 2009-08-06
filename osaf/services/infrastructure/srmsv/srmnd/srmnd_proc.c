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

  MODULE NAME: SRMND_PROC.C
 
..............................................................................

  DESCRIPTION: This file contains the general routines that interfaces (reads
               /writes from/to the SRMND database.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srmnd.h"

/* Static function prototypes */
static void srmnd_mbx_process (SYSF_MBX *mbx);
static void srmnd_del_rsrc_type_node(SRMND_CB *srmnd,
                                     SRMND_RSRC_TYPE_NODE *rsrc_type_node);

/* SRMND control-dump routines */
static void srmnd_cb_dump(SRMND_CB *srmnd, FILE *fp);
static void srmnd_sigusr2_handler(int i_sig_num);
static void srmnd_srma_dump(SRMND_MON_SRMA_USR_NODE *usr, FILE *fp);
static void srmnd_rsrc_dump(SRMND_RSRC_MON_NODE *rsrc, FILE *fp);
static void srmnd_samples_dump(SRMND_SAMPLE_DATA *sample, FILE *fp);
static void srmnd_monitor_info_dump(SRMND_MONITOR_INFO *mon_info, uns16 rsrc_type, FILE *fp);
static void srmnd_monitor_data_dump(NCS_SRMSV_MON_DATA *mon_data, FILE *fp);
static void srmnd_value_dump(NCS_SRMSV_VALUE *value, FILE *fp);
static void srmnd_rsrc_type_dump(SRMND_RSRC_TYPE_NODE *rsrc_type, FILE *fp);

extern uns32 gl_srmnd_hdl;

/****************************************************************************
  Name          :  srmnd_del_rsrc_type_node
 
  Description   :  This routine deletes the respective resource type node
                   from the SRMND data base

  Arguments     :  rsrc_type_node - ptr to the SRMND_RSRC_TYPE_NODE struct                   
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_del_rsrc_type_node(SRMND_CB *srmnd,
                              SRMND_RSRC_TYPE_NODE *rsrc_type_node)
{
   SRMND_RSRC_TYPE_NODE *prev_rsrc_type = rsrc_type_node->prev_rsrc_type;
   SRMND_RSRC_TYPE_NODE *next_rsrc_type = rsrc_type_node->next_rsrc_type;

   /* Resource monitor records are still hanging on.. so don't delete it */
   if ((rsrc_type_node->start_rsrc_mon_list != NULL) ||
       (rsrc_type_node->start_rsrc_subs_list != NULL) ||
       (rsrc_type_node->watermark_rsrc != NULL))
      return;
   
   /* Check if it is the only node of the list */
   if ((prev_rsrc_type == NULL) && 
       (next_rsrc_type == NULL))
   {
      srmnd->start_rsrc_type = NULL;
   }
   else /* Check if it is the first node of the list */
   if (prev_rsrc_type == NULL) {
      srmnd->start_rsrc_type = next_rsrc_type;
      next_rsrc_type->prev_rsrc_type = NULL;
   }
   else /* Check if it is the last node of the list */
   if (next_rsrc_type == NULL) {
      prev_rsrc_type->next_rsrc_type = NULL;
   }
   else /* Middle node */
   {
      prev_rsrc_type->next_rsrc_type = next_rsrc_type;
      next_rsrc_type->prev_rsrc_type = prev_rsrc_type;
   }

   m_MMGR_FREE_SRMND_RSRC_TYPE(rsrc_type_node);

   return;
}


/****************************************************************************
  Name          :  srmnd_del_rsrc_mon_from_rsrc_type_list
 
  Description   :  This routine deletes the respective resource monitor record
                   from resource type list.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   rsrc - resource monitor record                  
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
void srmnd_del_rsrc_mon_from_rsrc_type_list(SRMND_CB *srmnd,
                                            SRMND_RSRC_MON_NODE *rsrc_mon)
{   
   SRMND_RSRC_MON_NODE **start_rsrc_mon;
   SRMND_RSRC_MON_NODE *prev_rsrc_type_ptr = rsrc_mon->prev_rsrc_type_ptr;
   SRMND_RSRC_MON_NODE *next_rsrc_type_ptr = rsrc_mon->next_rsrc_type_ptr;

   if (!(rsrc_mon->rsrc_type_node))
      return;

   if (rsrc_mon->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_WATERMARK)
   {
      rsrc_mon->rsrc_type_node->watermark_rsrc = NULL;
   }
   else
   {
      if (rsrc_mon->usr_type == NCS_SRMSV_USER_SUBSCRIBER)
      {
         start_rsrc_mon = (SRMND_RSRC_MON_NODE **)(&(rsrc_mon->rsrc_type_node->
                                                     start_rsrc_subs_list));
      }
      else
      {
         start_rsrc_mon = (SRMND_RSRC_MON_NODE **)(&(rsrc_mon->rsrc_type_node->
                                                     start_rsrc_mon_list));
      }

      /* Check if it is the only node of the list */
      if ((prev_rsrc_type_ptr == NULL) && 
          (next_rsrc_type_ptr == NULL)) 
      {
          if (*start_rsrc_mon == rsrc_mon)
          {
             *start_rsrc_mon = NULL; 
          }
          else
          {
             m_SRMND_LOG_MISC(SRMND_LOG_MISC_DATA_INCONSISTENT,
                              SRMND_LOG_MISC_NOTHING,
                              NCSFL_SEV_ERROR);          
          }
      }
      else  /* Check if it is the first node of the list */
      if (prev_rsrc_type_ptr == NULL) {
         *start_rsrc_mon = next_rsrc_type_ptr;
         next_rsrc_type_ptr->prev_rsrc_type_ptr = NULL;
      }
      else /* Check if it is the last node of the list */
      if (next_rsrc_type_ptr == NULL) {
         prev_rsrc_type_ptr->next_rsrc_type_ptr = NULL;
      }
      else /* Middle node */
      {
         prev_rsrc_type_ptr->next_rsrc_type_ptr = next_rsrc_type_ptr;
         next_rsrc_type_ptr->prev_rsrc_type_ptr = prev_rsrc_type_ptr;
      }
   }
   
   /* Check do we need to delete the resource type record also. Need to delete
      the rsrc type record if no rsrc-mon data is hanging under this node */
   if ((rsrc_mon->rsrc_type_node->start_rsrc_mon_list == NULL) &&
       (rsrc_mon->rsrc_type_node->start_rsrc_subs_list == NULL) &&
       (rsrc_mon->rsrc_type_node->watermark_rsrc == NULL))
   {
      srmnd_del_rsrc_type_node(srmnd, rsrc_mon->rsrc_type_node); 
   }

   /* Unlink the connection between the rsrc and srmnd */
   rsrc_mon->prev_rsrc_type_ptr = NULL;
   rsrc_mon->next_rsrc_type_ptr = NULL;
   rsrc_mon->rsrc_type_node = NULL;

   return;
}


/****************************************************************************
  Name          :  srmnd_del_rsrc_mon_from_user_list
 
  Description   :  This routine deletes the respective resource monitor record
                   from user-application list.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   rsrc - resource monitor record                  
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
void srmnd_del_rsrc_mon_from_user_list(SRMND_CB *srmnd,
                                       SRMND_RSRC_MON_NODE *rsrc_mon)
{
   SRMND_RSRC_MON_NODE *prev_srma_usr_rsrc = rsrc_mon->prev_srma_usr_rsrc;
   SRMND_RSRC_MON_NODE *next_srma_usr_rsrc = rsrc_mon->next_srma_usr_rsrc;
   SRMND_MON_SRMA_USR_NODE *srma_usr_node = rsrc_mon->srma_usr_node;

   if (!(srma_usr_node))
      return;
  
   /* Check if it is the only node of the list */
   if ((prev_srma_usr_rsrc == NULL) && 
       (next_srma_usr_rsrc == NULL)) 
   {
       if (srma_usr_node->start_rsrc_mon_node == rsrc_mon)
       {
          srma_usr_node->start_rsrc_mon_node = NULL; 
          
          /* No more rsrc's, Now delete the application node itself */
          if (srma_usr_node->stop_monitoring != TRUE)
             srmnd_del_appl_node(srmnd, srma_usr_node);
       }
       else
       {
          m_SRMND_LOG_MISC(SRMND_LOG_MISC_DATA_INCONSISTENT,
                           SRMND_LOG_MISC_NOTHING,
                           NCSFL_SEV_ERROR);
       }
   }
   else  /* Check if it is the first node of the list */
   if (prev_srma_usr_rsrc == NULL) {
      srma_usr_node->start_rsrc_mon_node = next_srma_usr_rsrc;
      next_srma_usr_rsrc->prev_srma_usr_rsrc = NULL;
   }
   else /* Check if it is the last node of the list */
   if (next_srma_usr_rsrc == NULL) {
      prev_srma_usr_rsrc->next_srma_usr_rsrc = NULL;
   }
   else /* Middle node */
   {
      prev_srma_usr_rsrc->next_srma_usr_rsrc = next_srma_usr_rsrc;
      next_srma_usr_rsrc->prev_srma_usr_rsrc = prev_srma_usr_rsrc;
   }
   
   /* Unlink the connection between the rsrc and srmnd */
   rsrc_mon->prev_srma_usr_rsrc = NULL;
   rsrc_mon->next_srma_usr_rsrc = NULL;
   rsrc_mon->srma_usr_node = NULL;

   return;
}


/****************************************************************************
  Name          :  srmnd_remove_rsrc_pid_data
 
  Description   :  This routine removes the descendants information of a PID
                   from the resource monitor record.

  Arguments     :  rsrc - ptr to the resource monitor record                   
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_remove_rsrc_pid_data(SRMND_RSRC_MON_NODE *rsrc)
{
   SRMND_PID_DATA *pid_data = rsrc->descendant_pids;
   SRMND_PID_DATA *next_pid_node;

   /* Traverse to all the list of pids and delete them */
   while (pid_data)
   {
      next_pid_node = pid_data->next_pid;
      m_MMGR_FREE_SRMND_PID(pid_data);
      pid_data = next_pid_node;
      rsrc->pid_num--;
   }
   
   rsrc->descendant_pids = NULL;

   return;
}


/****************************************************************************
  Name          :  srmnd_get_pid_node
 
  Description   :  This routine retrieves the respective PID record from the
                   resource monitor record.

  Arguments     :  rsrc - ptr to the resource monitor record
                   pid - PID to be searched
                   
  Return Values :  NULL / respective PID record
 
  Notes         :  None.
******************************************************************************/
SRMND_PID_DATA *srmnd_get_pid_node(SRMND_RSRC_MON_NODE *rsrc, uns32 pid)
{
   SRMND_PID_DATA *pid_node = rsrc->descendant_pids;

   if (pid == 0)
      return NULL;

   /* Traverse through all PID records and match for the PID num */
   while (pid_node != NULL)
   {
      /* Check the PID matches */
      if (pid_node->pid == pid)
         return pid_node;
      
      pid_node = pid_node->next_pid;
   }

   return NULL;
}


/****************************************************************************
  Name          :  srmnd_remove_pid_data
 
  Description   :  This routine removes PID and its descendants information 
                   from the resource monitor record.

  Arguments     :  rsrc - ptr to the resource monitor record                   
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_remove_pid_data(SRMND_RSRC_MON_NODE *rsrc,
                           uns32 pid,
                           SRMND_PID_DATA *pid_data)
{
   SRMND_PID_DATA *pid_node = NULL, *c_pid = NULL, *next_c_pid = NULL;
   SRMND_PID_DATA *prev_pid = NULL, *next_pid = NULL;

   if (pid_data != NULL)
      pid_node = pid_data;
   else
      pid_node = srmnd_get_pid_node(rsrc, pid);

   if (pid_node == NULL)
      return;

   c_pid = pid_node->child_list;

   /* Traverse to all the list of CHILD pids and delete them */
   while (c_pid)
   {
      next_c_pid = c_pid->next_child_pid;
      prev_pid = c_pid->prev_pid;
      next_pid = c_pid->next_pid;

      /* De-list from the descendants list of resource monitor record */
      /* Check if it is the only node of the list */
      if ((prev_pid == NULL) && (next_pid == NULL))
      { 
         rsrc->descendant_pids = NULL;
      }
      else /* Check if it is the first node of the list */
      if (prev_pid == NULL) {
         rsrc->descendant_pids = next_pid;
         next_pid->prev_pid = NULL;
      }
      else /* Check if it is the last node of the list */
      if (next_pid == NULL) {
         prev_pid->next_pid = NULL;
      }
      else /* Middle node */
      {
         prev_pid->next_pid = next_pid;
         next_pid->prev_pid = prev_pid;
      }
      
      m_MMGR_FREE_SRMND_PID(c_pid);
      c_pid = next_c_pid;
   }

   /* Make child list ptr to NULL */
   pid_node->child_list = NULL;

   /* If the pid_node itself is a child and placed in the child list of its
      parent so delete it from the parent's child list */
   if (pid_node->child_level > 1)
   {
      prev_pid = pid_node->prev_child_pid;
      next_pid = pid_node->next_child_pid;

      /* Now delete the PID node itself */
      if ((prev_pid == NULL) && (next_pid == NULL))
      { 
         pid_node->parent->child_list = NULL;
      }
      else /* Check if it is the first node of the list */
      if (prev_pid == NULL) {
         pid_node->parent->child_list = next_pid;
         next_pid->prev_child_pid = NULL;
      }
      else /* Check if it is the last node of the list */
      if (next_pid == NULL) {
         prev_pid->next_child_pid = NULL;
      }
      else /* Middle node */
      {
         prev_pid->next_child_pid = next_pid;
         next_pid->prev_child_pid = prev_pid;
      }
   }
  
   /* Now delete pid_node from the descendants list of rsrc */ 
   prev_pid = pid_node->prev_pid;
   next_pid = pid_node->next_pid;

   /* Now delete the PID node itself */
   if ((prev_pid == NULL) && 
       (next_pid == NULL))
   { 
      rsrc->descendant_pids = NULL;
   }
   else /* Check if it is the first node of the list */
   if (prev_pid == NULL) {
      rsrc->descendant_pids = next_pid;
      next_pid->prev_pid = NULL;
   }
   else /* Check if it is the last node of the list */
   if (next_pid == NULL) {
      prev_pid->next_pid = NULL;
   }
   else /* Middle node */
   {
      prev_pid->next_pid = next_pid;
      next_pid->prev_pid = prev_pid;
   }
      
   m_MMGR_FREE_SRMND_PID(pid_node);

   return;
}


/****************************************************************************
  Name          :  srmnd_get_sample_record
 
  Description   :  This routine gets the next sample data record to be 
                   updated. Once all the sample records are updated, the 
                   next update will be done to the first sample record and so
                   on (cycle).

  Arguments     :  rsrc - ptr to the resource monitor record                   
                   
  Return Values :  Sample data record
 
  Notes         :  None.
******************************************************************************/
SRMND_SAMPLE_DATA *srmnd_get_sample_record(SRMND_RSRC_MON_NODE *rsrc)
{
   uns16 sample_num = 0;
   SRMND_SAMPLE_DATA *sample_data = rsrc->mon_data.sample_data;
   uns16 samples = rsrc->mon_data.monitor_data.mon_cfg.threshold.samples;
   uns16 *next_sample_to_update = &rsrc->mon_data.next_sample_to_update;
  
   /* Still need to build the sample data records, so return null */
   if (rsrc->mon_data.samples_updated < samples)
   {
      /* Now push last updated data to sample data list */
      if ((sample_data = m_MMGR_ALLOC_SRMND_SAMPLE) == NULL)
      {
         m_SRMND_LOG_MEM(SRMND_MEM_SAMPLE_DATA,
                         SRMND_MEM_ALLOC_FAILED,
                         NCSFL_SEV_CRITICAL);

         return NULL;
      }
      memset((char *)sample_data, 0, sizeof(SRMND_SAMPLE_DATA));

      /* Add the sample record to the rsrc sample list */
      sample_data->next_sample = rsrc->mon_data.sample_data;
      rsrc->mon_data.sample_data = sample_data;

      rsrc->mon_data.samples_updated++;
   }
   else
   {
      /* update next sample record to be updated */
      *next_sample_to_update = (*next_sample_to_update % samples) + 1;

      /* Update the sample record number, the first allocated sample data
         record is at the end of the list */
      sample_num = (samples - *next_sample_to_update);

      /* Get the corresponding sample data record */
      while (sample_num)
      {
         sample_num--;
         sample_data = sample_data->next_sample;
      }
   }

   /* ooooo.. you got it */
   return sample_data;
}


/****************************************************************************
  Name          :  srmnd_delete_rsrc_sample_data
 
  Description   :  This routine removes the collected sample data from the
                   resource monitor record.

  Arguments     :  rsrc - ptr to the resource monitor record                   
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_delete_rsrc_sample_data(SRMND_RSRC_MON_NODE *rsrc)
{
   SRMND_SAMPLE_DATA *sample_data = rsrc->mon_data.sample_data;
   SRMND_SAMPLE_DATA *next_sample_data = NULL;

   /* Clean up all allocated sample records */
   while (sample_data)
   {
      next_sample_data = sample_data->next_sample;
      m_MMGR_FREE_SRMND_SAMPLE(sample_data);
      sample_data = next_sample_data;
   }
  
   rsrc->mon_data.sample_data = NULL;
   rsrc->mon_data.recent_updated = 0;
   rsrc->mon_data.next_sample_to_update = 0;
   rsrc->mon_data.samples_updated = 0;

   return;
}


/****************************************************************************
  Name          :  srmnd_check_rsrc_type_node
 
  Description   :  This routine checks whether the resource type specific 
                   node exists in the resource type list of SRMND.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   rsrc_type - resource type
                   pid - PID if the resource type is of PROCESS specific
                   
  Return Values :  resource-type node / NULL
 
  Notes         :  None.
******************************************************************************/
SRMND_RSRC_TYPE_NODE *srmnd_check_rsrc_type_node(SRMND_CB *srmnd,
                                                 NCS_SRMSV_RSRC_TYPE rsrc_type,
                                                 uns32 pid)
{
   SRMND_RSRC_TYPE_NODE *rsrc_type_node = srmnd->start_rsrc_type;

   /* Check whether the respective Resource Type Node is existing */
   while (rsrc_type_node)
   {
      if ((rsrc_type_node->rsrc_type == rsrc_type) &&
          (rsrc_type_node->pid == pid))
          return rsrc_type_node;

      rsrc_type_node = rsrc_type_node->next_rsrc_type;
   }
   
   return NULL;
}


/****************************************************************************
  Name          :  srmnd_get_appl_node
 
  Description   :  This routine gets the user-application node (if it exists)
                   from application specific list of SRMND.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   usr_key - Ptr to the user key of the application node
                   
  Return Values :  user-application node / NULL
 
  Notes         :  None.
******************************************************************************/
SRMND_MON_SRMA_USR_NODE  *srmnd_get_appl_node(SRMND_CB *srmnd, 
                                              SRMND_SRMA_USR_KEY *usr_key)
{
   SRMND_MON_SRMA_USR_NODE *user_node = srmnd->start_srma_node;

   while (user_node)
   {
      if ((user_node->usr_key.srma_usr_hdl == usr_key->srma_usr_hdl) && 
          (!memcmp(&user_node->usr_key.srma_dest,
                   &usr_key->srma_dest, sizeof(MDS_DEST))))
      {
         return user_node;
      }

      user_node = user_node->next_srma_node;
   }
   
   return NULL;
}


/****************************************************************************
  Name          :  srmnd_del_srma
 
  Description   :  This routine deletes all the SRMA specific application
                   database from SRMND.

  Arguments     :  srmnd - ptr to the SRMND Control Block
                   srma_dest - ptr to MDS dest of SRMA
                   
  Return Values :  user-application node / NULL
 
  Notes         :  None.
******************************************************************************/
void srmnd_del_srma(SRMND_CB *srmnd, MDS_DEST *srma_dest)
{
   SRMND_MON_SRMA_USR_NODE *user_node = NULL;
   SRMND_MON_SRMA_USR_NODE *next_user_node = NULL;

   user_node = srmnd->start_srma_node;

   /* Traverse through the users list of SRMA in SRMND */
   while (user_node)
   {
      next_user_node = user_node->next_srma_node;

      if (srma_dest != NULL)
         if (memcmp(&user_node->usr_key.srma_dest, srma_dest, sizeof(MDS_DEST)))
         {
            user_node = next_user_node;   
            continue;
         }

      /* Delete all the configured resources of an application */
      srmnd_del_appl_rsrc(srmnd, user_node);

      /* Now delete the application node itself */
      srmnd_del_appl_node(srmnd, user_node);

      user_node = next_user_node;
   }

   return;
}


/****************************************************************************
  Name          :  srmnd_main_process
 
  Description   :  This routine is an entry point for the SRMND task.
 
  Arguments     :  arg - ptr to the cb handle
 
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void srmnd_main_process (void *arg)
{
   SRMND_CB        *cb = 0;
   SYSF_MBX        *mbx;
   uns32           cb_hdl;
   NCS_SEL_OBJ_SET wait_sel_objs;
   NCS_SEL_OBJ     mbx_sel_obj;
   NCS_SEL_OBJ     highest_sel_obj;
   SaAisErrorT     saf_status = SA_AIS_OK; 
   NCS_SEL_OBJ     amf_sel_obj;

   /* get cb-hdl */
   cb_hdl = *((uns32 *)arg);

   /* retrieve SRMND cb */
   if ((cb = (SRMND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND, cb_hdl)) == NULL)
   {
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_ERROR);
      return;
   }

   /* Install the sig handler for SIGUSR2, to dump the datastruct contents */
   if ((signal(SIGUSR2, srmnd_sigusr2_handler)) == SIG_ERR)
   {
       /* Log the message */
      ncshm_give_hdl(cb_hdl);
      return;
   }

   /* Initialize the interface with Availability Management Framework */
   if (srmnd_amf_initialize(cb) != NCSCC_RC_SUCCESS)
   {
      ncshm_give_hdl(cb_hdl);

      m_SRMND_LOG_AMF(SRMND_LOG_AMF_INITIALIZE,
                      SRMND_LOG_AMF_FAILED,
                      NCSFL_SEV_CRITICAL);
      return; 
   }
   else
   {
      m_SRMND_LOG_AMF(SRMND_LOG_AMF_INITIALIZE,
                      SRMND_LOG_AMF_SUCCESS,
                      NCSFL_SEV_INFO);
   }

   /* get the mbx */
   mbx = &cb->mbx;

   /* reset the wait select objects */
   m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);

   /* get the mbx select object */
   mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(mbx);
   /* set all the select objects on which srmnd waits */
   m_NCS_SEL_OBJ_SET(mbx_sel_obj, &wait_sel_objs);  

   m_SET_FD_IN_SEL_OBJ((uns32)cb->amf_sel_obj,
                       amf_sel_obj);

   m_NCS_SEL_OBJ_SET(amf_sel_obj, &wait_sel_objs);

   /* get the highest select object in the set */
   highest_sel_obj = m_GET_HIGHER_SEL_OBJ(amf_sel_obj, mbx_sel_obj);
   
   /* before waiting, return srmnd cb */
   ncshm_give_hdl(cb_hdl);

   /* now wait forever */
   while (m_NCS_SEL_OBJ_SELECT(highest_sel_obj, &wait_sel_objs, NULL, NULL, NULL) != -1)
   {
      /* srmnd mbx processing */
      if (m_NCS_SEL_OBJ_ISSET(amf_sel_obj, &wait_sel_objs))
      {
         /* dispatch all the AMF pending function */
         saf_status = saAmfDispatch(cb->amf_handle, SA_DISPATCH_ALL);
         if (saf_status != SA_AIS_OK)
         {
            m_SRMND_LOG_AMF(SRMND_LOG_AMF_DISPATCH,
                            SRMND_LOG_AMF_FAILED,
                            NCSFL_SEV_ERROR);  
         }
      }         
 
      /* srmnd mbx processing */
      if (m_NCS_SEL_OBJ_ISSET(mbx_sel_obj, &wait_sel_objs))
         srmnd_mbx_process(mbx);

      /* reset the wait select objects */
      m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);

      /* again set all the wait select objs */
      m_NCS_SEL_OBJ_SET(mbx_sel_obj, &wait_sel_objs);
      m_NCS_SEL_OBJ_SET(amf_sel_obj, &wait_sel_objs);
   }

   return;
}


/****************************************************************************
  Name          :  srmnd_mbx_process
 
  Description   :  This routine dequeues & processes the events from the 
                   srmnd mailbox.
 
  Arguments     :  mbx - ptr to the mailbox
 
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void srmnd_mbx_process(SYSF_MBX *mbx)
{
   SRMND_EVT *evt = NULL;

   /* process each event */
   while ((evt = (SRMND_EVT *)m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt)) != NULL)
      srmnd_evt_process(evt);

   return;
}


/****************************************************************************
  Name          :  srmnd_samples_dump
                                                                           
  Description   :  Dumps SRMSv resource monitor's sampling data.       
                                                                            
  Arguments     :  sample - pointer to SRMND_SAMPLE_DATA structure
                   fp - file pointer                    
                                                                            
  Return Values :  None.
 
  Notes         :  None
****************************************************************************/
void  srmnd_samples_dump(SRMND_SAMPLE_DATA *sample, FILE *fp)
{
   char  update_time[60] = {0};
   uns16 sample_count = 0;


   if (sample == NULL)
   {
      fprintf(fp, "\n No Samples Available");
      return;
   }

   while (sample)
   {
      fprintf(fp, "\n Sample ptr: 0x%lx", (long)sample);
      fprintf(fp, "\n Next sample ptr: 0x%lx", (long)sample->next_sample);
      memset(update_time, 0, sizeof(update_time));
      m_GET_ASCII_DATE_TIME_STAMP(sample->when_updated, update_time);
      fprintf(fp, "\n\t Updated time: %s", update_time);
      fprintf(fp, "\n %d.Sample Value:", ++sample_count);
      srmnd_value_dump(&sample->value, fp);
      sample = sample->next_sample;
   }

   return;
}


/****************************************************************************
  Name          :  srmnd_monitor_info_dump
                                                                           
  Description   :  Dumps SRMSv monitor data record.       
                                                                            
  Arguments     :  mon_info - pointer to SRMND_MONITOR_INFO structure
                   rsrc_type - Resource Type enum value
                   fp - file pointer                    
                                                                            
  Return Values :  None.
 
  Notes         :  None
****************************************************************************/
void srmnd_monitor_info_dump(SRMND_MONITOR_INFO *mon_info, uns16 rsrc_type, FILE *fp)
{
   char last_updated[60]={0};


   /* Dump monitor data */   
   srmnd_monitor_data_dump(&mon_info->monitor_data, fp);
 
   m_GET_ASCII_DATE_TIME_STAMP(mon_info->recent_updated, last_updated);
   fprintf(fp, "\n Last Updated: %s", last_updated);
 
   if (mon_info->monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_WATERMARK)
   {
      fprintf(fp, "\n Watermark Values:");
      if ((mon_info->monitor_data.mon_cfg.watermark_type == NCS_SRMSV_WATERMARK_LOW) ||
          (mon_info->monitor_data.mon_cfg.watermark_type == NCS_SRMSV_WATERMARK_DUAL))
         srmnd_value_dump(&mon_info->wm_val.l_val, fp);

      if ((mon_info->monitor_data.mon_cfg.watermark_type == NCS_SRMSV_WATERMARK_HIGH) ||
          (mon_info->monitor_data.mon_cfg.watermark_type == NCS_SRMSV_WATERMARK_DUAL))
         srmnd_value_dump(&mon_info->wm_val.h_val, fp);

      return;
   }

   switch (rsrc_type)
   {
   case NCS_SRMSV_RSRC_CPU_UTIL:
   case NCS_SRMSV_RSRC_CPU_KERNEL:
   case NCS_SRMSV_RSRC_CPU_USER:
       fprintf(fp, "\n User Time (jiffies): %d", mon_info->info.cpu_data.usertime);
       fprintf(fp, "\n Kernel Time (jiffies): %d", mon_info->info.cpu_data.kerneltime);
       fprintf(fp, "\n Nicer Time (jiffies): %d", mon_info->info.cpu_data.nicertime);
       fprintf(fp, "\n Idle Time (jiffies): %d", mon_info->info.cpu_data.idletime);
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL_ONE:
       fprintf(fp, "\n CPU ldavg_1: %f", mon_info->info.cpu_ldavg);
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL_FIVE:
       fprintf(fp, "\n CPU ldavg_5: %f", mon_info->info.cpu_ldavg);
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN:
       fprintf(fp, "\n CPU ldavg_15: %f", mon_info->info.cpu_ldavg);
       break;

   case NCS_SRMSV_RSRC_PROC_CPU:
       fprintf(fp, "\n Process CPU jiffies: %u", mon_info->info.proc_cpu_jiffies);
       break;

   default:
       break;
   }

   fprintf(fp, "\n Result Value:");
   srmnd_value_dump(&mon_info->result_value, fp);

   fprintf(fp, "\n Next sample to update: %d", mon_info->next_sample_to_update);
   fprintf(fp, "\n Samples Updated: %d", mon_info->samples_updated);
 
   /* Dump samples data */ 
   srmnd_samples_dump(mon_info->sample_data, fp);

   return;
}


/****************************************************************************
  Name          :  srmnd_monitor_data_dump
                                                                           
  Description   :  Dumps SRMSv monitor data record.       
                                                                            
  Arguments     :  mon_data - pointer to NCS_SRMSV_MON_DATA structure
                   fp - file pointer                    
                                                                            
  Return Values :  None.
 
  Notes         :  None
****************************************************************************/
void  srmnd_monitor_data_dump(NCS_SRMSV_MON_DATA *mon_data, FILE *fp)
{
   fprintf(fp, "\n Monitoring Rate: %u", (uns32) mon_data->monitoring_rate);
   fprintf(fp, "\n Monitoring Interval: %u", (uns32) mon_data->monitoring_interval);

   switch(mon_data->monitor_type)
   {
   case NCS_SRMSV_MON_TYPE_THRESHOLD:
       fprintf(fp, "\n Monitoring Type: Threshold");
       fprintf(fp, "\n Samples: %d", mon_data->mon_cfg.threshold.samples);
       fprintf(fp, "\n Condition: %d", mon_data->mon_cfg.threshold.condition);

       fprintf(fp, "\n Threshold Value:");
       srmnd_value_dump(&mon_data->mon_cfg.threshold.threshold_val, fp);

       fprintf(fp, "\n Tolerable Value:");
       srmnd_value_dump(&mon_data->mon_cfg.threshold.tolerable_val, fp);
       break;

   case NCS_SRMSV_MON_TYPE_WATERMARK:
       fprintf(fp, "\n Monitoring Type: WaterMark");
       fprintf(fp, "\n WM Monitor Type: %d", mon_data->mon_cfg.watermark_type);
       break;

   case NCS_SRMSV_MON_TYPE_LEAKY_BUCKET:
       fprintf(fp, "\n Monitoring Type: LeakyBucket");
  
       fprintf(fp, "\n Bucket Size:");
       srmnd_value_dump(&mon_data->mon_cfg.leaky_bucket.bucket_size, fp);

       fprintf(fp, "\n Fill Size:");
       srmnd_value_dump(&mon_data->mon_cfg.leaky_bucket.fill_size, fp);
       break;

   default:
       fprintf(fp, "\n ERROR: Monitoring Type: Unknown");
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srmnd_value_dump
                                                                           
  Description   :  Dumps SRMSv value record.       
                                                                            
  Arguments     :  value - pointer to SRMSv value node
                   fp - file pointer                    
                                                                            
  Return Values :  None.
 
  Notes         :  None
****************************************************************************/
void srmnd_value_dump(NCS_SRMSV_VALUE *value, FILE *fp)
{
   if (value->scale_type)
      fprintf(fp, "\n\t Scale Type: %d", value->scale_type);

   switch (value->val_type)
   {
   case NCS_SRMSV_VAL_TYPE_FLOAT:
       fprintf(fp, "\n\t Value Type: float");
       fprintf(fp, "\n\t Value: %f", value->val.f_val);
       break;

   case NCS_SRMSV_VAL_TYPE_INT32:
       fprintf(fp, "\n\t Value Type: long");
       fprintf(fp, "\n\t Value: %d", value->val.i_val32);
       break;

   case NCS_SRMSV_VAL_TYPE_UNS32:
       fprintf(fp, "\n\t Value Type: unsigned long");
       fprintf(fp, "\n\t Value: %u", value->val.u_val32);
       break;

   case NCS_SRMSV_VAL_TYPE_INT64:
       fprintf(fp, "\n\t Value Type: long long");
       fprintf(fp, "\n\t Value: %lld", value->val.i_val64);
       break;

   case NCS_SRMSV_VAL_TYPE_UNS64:
       fprintf(fp, "\n\t Value Type: unsigned long long");
       fprintf(fp, "\n\t Value: %llu", value->val.u_val64);
       break;
   
   default:
       fprintf(fp, "\n\t -NIL-");
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srmnd_rsrc_dump
                                                                           
  Description   :  Dumps resource monitor data.       
                                                                            
  Arguments     :  rsrc - pointer to resource monitor node
                   fp - file pointer                    
                                                                            
  Return Values :  None.
 
  Notes         :  None
****************************************************************************/
void srmnd_rsrc_dump(SRMND_RSRC_MON_NODE *rsrc, FILE *fp)
{
   char expiry[60] = {0};


   if (!rsrc->rsrc_type_node)
   {
      fprintf(fp, "\n\n Resource Type is Missing");
      return;
   }

   fprintf(fp, "\n\n Resource ptr: 0x%lx", (long)(SRMND_RSRC_MON_NODE *)rsrc);
   fprintf(fp, "\n Resource Type: %d", rsrc->rsrc_type_node->rsrc_type);
   fprintf(fp, "\n Monitor Type: %d", rsrc->mon_data.monitor_data.monitor_type);
   fprintf(fp, "\n SRMND Rsrc Mon Hdl: 0x%x", rsrc->rsrc_mon_hdl);
   fprintf(fp, "\n SRMA  Rsrc Mon Hdl: 0x%x", rsrc->srma_rsrc_hdl);
   fprintf(fp, "\n Threshold Event SENT flag: %d", rsrc->sent_flag);

   fprintf(fp, "\n User List Ptr's:");
   fprintf(fp, "\n\t Prev-ptr: 0x%lx", (long)(SRMND_RSRC_MON_NODE *)rsrc->prev_srma_usr_rsrc);
   fprintf(fp, "\n\t Next-ptr: 0x%lx", (long)(SRMND_RSRC_MON_NODE *)rsrc->next_srma_usr_rsrc);

   fprintf(fp, "\n Rsrc Type List Ptr's:");
   fprintf(fp, "\n\t Prev-ptr: 0x%lx", (long)(SRMND_RSRC_MON_NODE *)rsrc->prev_rsrc_type_ptr);
   fprintf(fp, "\n\t Next-ptr: 0x%lx", (long)(SRMND_RSRC_MON_NODE *)rsrc->next_rsrc_type_ptr);

   if ((rsrc->rsrc_type_node->rsrc_type == NCS_SRMSV_RSRC_PROC_EXIT) &&
       (rsrc->descendant_pids != NULL))
   {
      fprintf(fp, "\n\n PID: %d", rsrc->pid_num);
      if (rsrc->descendant_pids)
      {
         fprintf(fp, "\n Descendant PID:");
         fprintf(fp, "\n Descendant PID: %d Child Level: %d", rsrc->descendant_pids->pid, rsrc->descendant_pids->child_level);
      }
   }

   fprintf(fp, "\n User Type: %d", rsrc->usr_type);
   fprintf(fp, "\n Monitor State: %d", rsrc->monitor_state);

   m_GET_ASCII_DATE_TIME_STAMP(rsrc->rsrc_mon_expiry, expiry);
   fprintf(fp, "\n Rsrc Mon Expiry: %s", expiry);

   if (rsrc->srma_usr_node != NULL)
   {
      fprintf(fp, "\n User Data:");
      fprintf(fp, "\n\t User Handle: 0x%x", rsrc->srma_usr_node->usr_key.srma_usr_hdl);
      fprintf(fp, "\n\t MDS Dest: 0x%x \n", (uns32)rsrc->srma_usr_node->usr_key.srma_dest);
   }

   srmnd_monitor_info_dump(&rsrc->mon_data, rsrc->rsrc_type_node->rsrc_type, fp);
   srmnd_value_dump(&rsrc->notify_val, fp);

   return; 
}

 
/****************************************************************************
  Name          :  srmnd_rsrc_type_dump
                                                                           
  Description   :  Dumps resource type list data (includes rsrc monitor cfg &
                   subscriber's of a particular resource).       
                                                                            
  Arguments     :  rsrc_type - pointer to resource type node
                   fp - file pointer                    
                                                                            
  Return Values :  None.
 
  Notes         :  None
****************************************************************************/
void srmnd_rsrc_type_dump(SRMND_RSRC_TYPE_NODE *rsrc_type, FILE *fp)
{
   SRMND_RSRC_MON_NODE  *rsrc = NULL;
   uns16  rsrc_count = 0, rsrc_type_count = 0;


   fprintf(fp, "\n\n\n ******************************");
   fprintf(fp, "\n *       RSRC TYPE LIST       *");
   fprintf(fp, "\n ******************************");

   if (!rsrc_type)
   {
      fprintf(fp, "\n\n\t No Resource's available");
      return;
   }


   while (rsrc_type != NULL)
   { 
      fprintf(fp, "\n\n\n %d.RSRC TYPE: %d", ++rsrc_type_count, rsrc_type->rsrc_type);
      fprintf(fp, "\n\t WaterMark rsrc ptr: 0x%lx", (long)rsrc_type->watermark_rsrc);
      fprintf(fp, "\n\t RSRC_MON list start_ptr: 0x%lx", (long)rsrc_type->start_rsrc_mon_list);
      fprintf(fp, "\n\t SUBSCR list start_ptr: 0x%lx", (long)rsrc_type->start_rsrc_subs_list);
      fprintf(fp, "\n\t RSRC TYPE list ptr's:");
      fprintf(fp, "\n\t\t Prev ptr: 0x%lx", (long)rsrc_type->prev_rsrc_type);
      fprintf(fp, "\n\t\t Next ptr: 0x%lx", (long)rsrc_type->next_rsrc_type);

      if ((rsrc_type->rsrc_type == NCS_SRMSV_RSRC_PROC_EXIT) ||
          (rsrc_type->rsrc_type == NCS_SRMSV_RSRC_PROC_MEM)  ||
          (rsrc_type->rsrc_type == NCS_SRMSV_RSRC_PROC_CPU))
      {
         fprintf(fp, "\n PID: %d", rsrc_type->pid);
         fprintf(fp, "\n Child Level: %d", rsrc_type->child_level);
      }

      rsrc = rsrc_type->watermark_rsrc;
      if (rsrc != NULL)
      {
         fprintf(fp, "\n\n Watermark Resource Data");
          fprintf(fp, "\n =======================");
         srmnd_rsrc_dump(rsrc, fp);
      }

      rsrc = rsrc_type->start_rsrc_mon_list;
      rsrc_count = 0;
      while (rsrc)
      {
          fprintf(fp, "\n\n %d.Resource Monitor", ++rsrc_count);
          fprintf(fp, "\n ====================");
          srmnd_rsrc_dump(rsrc, fp);
          rsrc = rsrc->next_rsrc_type_ptr;
      }
      fprintf(fp, "\n\n TOTAL RSRC MON: %d", rsrc_count);
       
      rsrc = rsrc_type->start_rsrc_subs_list;
      if (rsrc != NULL)
      {
         rsrc_count = 0;
         while (rsrc)
         {
            fprintf(fp, "\n\n %d.Subscriber:", ++rsrc_count);
            fprintf(fp, "\n ==============");
            srmnd_rsrc_dump(rsrc, fp);
            rsrc = rsrc->next_rsrc_type_ptr;
         } 
         
         fprintf(fp, "\n\n\n TOTAL SUBSCRIBERS: %d", rsrc_count);
      }

      rsrc_type = rsrc_type->next_rsrc_type;
   
      fflush(fp);
   }

   fprintf(fp, "\n TOTAL RSRC TYPES: %d", rsrc_type_count);

   return;
}


/****************************************************************************
  Name          :  srmnd_srma_dump
                                                                           
  Description   :  Dumps SRMSv Agent specific data.       
                                                                            
  Arguments     :  usr - pointer to user node
                   fp - file pointer                    
                                                                            
  Return Values :  None.
 
  Notes         :  None
****************************************************************************/
void srmnd_srma_dump(SRMND_MON_SRMA_USR_NODE *usr, FILE *fp)
{
   SRMND_RSRC_MON_NODE *rsrc = NULL;
   uns16 rsrc_count = 0, usr_count = 0;


   fprintf(fp, "\n\n\n ******************************");
   fprintf(fp, "\n *       USER APPL LIST       *");
   fprintf(fp, "\n ******************************");
   if (usr == NULL)
   {
      fprintf(fp, "\n\n\t SRMSv Agents doesn't exist");
      return;
   }

   fprintf(fp, "\n\n RSRC_MON list start ptr: 0x%lx", (long)usr->start_rsrc_mon_node);
   fprintf(fp, "\n SRMA list ptr's:");
   fprintf(fp, "\n\t Prev ptr: 0x%lx", (long)usr->prev_srma_node);
   fprintf(fp, "\n\t Next ptr: 0x%lx", (long)usr->next_srma_node);

   while (usr)
   {
      fprintf(fp, "\n %d.USER APPL DATA  ", ++usr_count);

      fprintf(fp, "\n\t User Handle: 0x%x", usr->usr_key.srma_usr_hdl);
      fprintf(fp, "\n\t MDS Dest: 0x%x", (uns32)usr->usr_key.srma_dest);
      fprintf(fp, "\n\t Stop Monitoring: %d", usr->stop_monitoring);

      rsrc = usr->start_rsrc_mon_node;
    
      rsrc_count = 0;
      while (rsrc)
      {
         fprintf(fp, "\n\n %d.RSRC MON:", ++rsrc_count);
         fprintf(fp, "\n =============");
         srmnd_rsrc_dump(rsrc, fp);
         rsrc = rsrc->next_srma_usr_rsrc;
      }

      fprintf(fp, "\n\n\n TOTAL RSRC MON: %d", rsrc_count);

      usr = usr->next_srma_node;
      fflush(fp);
   }

   fprintf(fp, "\n TOTAL USERs : %d\n\n", usr_count);
   fflush(fp);

   return;
}


/******************************************************************************
  Name          :  srmnd_sigusr2_handler
                                                                           
  Description   :  SIGNAL handler function, being called to dump the SRMND 
                   database on issuing a signal of SIGUSR2
                                                                            
  Arguments     :  i_sig_num - Signal Number (SIGUSR2) 
                                                                            
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void srmnd_sigusr2_handler(int i_sig_num)
{
   SRMND_CB  *srmnd = NULL;
   SRMND_EVT *srmnd_evt = NULL;


   /* retrieve srmnd cb */
   if ((srmnd = (SRMND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                           (uns32)gl_srmnd_hdl)) == NULL)
      return;

   /* Post the event to SRMND about timer expiration */
   srmnd_evt = srmnd_evt_create(srmnd->cb_hdl,
                                NULL,
                                NULL,
                                SRMND_SIG_EVT_TYPE);
   if (srmnd_evt)
      m_NCS_IPC_SEND(&srmnd->mbx, srmnd_evt, NCS_IPC_PRIORITY_LOW);

   ncshm_give_hdl((uns32)gl_srmnd_hdl);

   return;
}


/******************************************************************************
  Name          :  srmnd_cb_dump
                                                                           
  Description   :  Dumps the SRMND CB content
                                                                            
  Arguments     :  srmnd - SRMND CB ptr
                   fp - File ptr                    
                                                                            
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void srmnd_cb_dump(SRMND_CB *srmnd, FILE *fp)
{
   /* PRINT the SRMND CB content */
   fprintf(fp, "\n\n SRMND_CB:");
   fprintf(fp, "\n\t Node Id: 0x%x", m_NCS_NODE_ID_FROM_MDS_DEST(srmnd->srmnd_dest));
   fprintf(fp, "\n\t MDS dest: 0x%x", (uns32)srmnd->srmnd_dest);
   fprintf(fp, "\n\t HA State: %d", srmnd->ha_cstate);
   fprintf(fp, "\n\t Oper Status: %d", srmnd->oper_status);
   fprintf(fp, "\n\t Health Chk Started Flg: %d", srmnd->health_check_started);
   fprintf(fp, "\n\t AMF Handle: 0x%x", (uns32)srmnd->amf_handle);
   fprintf(fp, "\n\t SRMA list start ptr: 0x%lx", (long)srmnd->start_srma_node);
   fprintf(fp, "\n\t Rsrc Type List ptr: 0x%lx \n", (long)srmnd->start_rsrc_type);

   fflush(fp);
   return;
}

 
/******************************************************************************
  Name          :  srmnd_db_dump
                                                                           
  Description   :  Dumps the SRMND database into a file in /var/crash/SRMND_DB
                   _DUMP_<timestamp> 
                                                                            
  Arguments     :  srmnd - SRMND CB ptr                    
                                                                            
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void srmnd_db_dump(SRMND_CB *srmnd)
{
   time_t   tod;
   FILE     *fp;
   int8     asc_tod[70]={0};
   int8     tmp_file[90]={0};


   /* compose the temporary file name */
   m_GET_ASCII_DATE_TIME_STAMP(tod, asc_tod);

   /* get the final name to be used */
   strcpy(tmp_file, "/var/crash/SRMND_DB_DUMP_");
   strcat(tmp_file, asc_tod);

   /* open the file in /tmp directory */
   fp = fopen(tmp_file, "w");
   if (fp == NULL)
      return;

   fprintf(fp, "\n *********************************************************");
   fprintf(fp, "\n *        S R M N D   D A T A  B A S E   D U M P         *");
   fprintf(fp, "\n *                                                       *");
   fprintf(fp, "\n * DUMP TIME: %s                         *", asc_tod);
   fprintf(fp, "\n *********************************************************");

   if (!srmnd)
   {
      fprintf(fp, "\n\n SRMND CB ptr is NULL");
      fflush(fp);

      /* close the fp */
      fclose(fp);
      return;
   }

   /* Dump SRMND CB info */
   srmnd_cb_dump(srmnd, fp);

   /* Dump client specific SRMSv Agent data */
   srmnd_srma_dump(srmnd->start_srma_node, fp);

   /* PRINT the SRMND rsrc's cfgs */
   srmnd_rsrc_type_dump(srmnd->start_rsrc_type, fp);

   fflush(fp);

   /* close the fp */
   fclose(fp);

   return;
}

