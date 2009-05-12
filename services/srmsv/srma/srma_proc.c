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

  MODULE NAME: SRMA_PROC.C 
..............................................................................

  DESCRIPTION: This file contains SRMSV API implementation supported functions

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srma.h"

static void srma_del_srmnd_usr_node(SRMA_CB *srma,
                                    SRMA_SRMND_USR_NODE *srmnd_usr);
static SRMA_SRMND_USR_NODE *srma_check_srmnd_usr_exists(SRMA_SRMND_INFO *srmnd,
                                           SRMA_USR_APPL_NODE *appl,
                                           SRMA_SRMND_USR_NODE_TYPE node_type);


/****************************************************************************
  Name          :  srma_del_srmnd_node
 
  Description   :  This function deletes the respective SRMND node from the
                   SRMND list of SRMA.
 
  Arguments     :  srma - ptr to the SRMA control block
                   srmnd - ptr to the SRMND record of type SRMA_SRMND_INFO
 
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srma_del_srmnd_node(SRMA_CB *srma, SRMA_SRMND_INFO *srmnd)
{ 
   SRMA_SRMND_INFO *prev_srmnd_node = srmnd->prev_srmnd_node;
   SRMA_SRMND_INFO *next_srmnd_node = srmnd->next_srmnd_node;

   if (!(srmnd))
      return;
     
   /* Check if it is the only node of the list */
   if ((prev_srmnd_node == NULL) && 
       (next_srmnd_node == NULL))
   {
      srma->start_srmnd_node = NULL;
   }
   else /* Check if it is the first node of the list */
   if (prev_srmnd_node == NULL) {
      srma->start_srmnd_node = next_srmnd_node;
      next_srmnd_node->prev_srmnd_node = NULL;
   }
   else /* Check if it is the last node of the list */
   if (next_srmnd_node == NULL) {
      prev_srmnd_node->next_srmnd_node = NULL;
   }
   else /* Middle node */
   {
      prev_srmnd_node->next_srmnd_node = next_srmnd_node;
      next_srmnd_node->prev_srmnd_node = prev_srmnd_node;
   }

   m_MMGR_FREE_SRMA_SRMND_INFO(srmnd);

   return;
}


/****************************************************************************
  Name          :  srma_del_srmnd_usr_node
 
  Description   :  This function deletes the respective SRMND-USR node either
                   from the SRMND node (of SRMND list) or from the user-
                   application node (of User-Application list) of SRMA.
 
  Arguments     :  srma - ptr to the SRMA control block
                   srmnd_usr - ptr to the SRMND-USR record of type
                               SRMA_SRMND_USR_NODE
 
  Return Values :  Nothing
 
  Notes         :  None.
*****************************************************************************/
void srma_del_srmnd_usr_node(SRMA_CB *srma, SRMA_SRMND_USR_NODE *srmnd_usr)
{
   SRMA_SRMND_USR_NODE *prev_usr_node = srmnd_usr->prev_usr_node;
   SRMA_SRMND_USR_NODE *next_usr_node = srmnd_usr->next_usr_node;

   /* Check if it is the only node of the list */
   if((prev_usr_node == NULL) && 
      (next_usr_node == NULL))
   {
      if (srmnd_usr->node_type == SRMA_SRMND_USR_TYPE_NODE)
         srmnd_usr->srmnd_info->start_usr_node = NULL;          
      else  /* User Application list specific node */
         srmnd_usr->usr_appl->start_srmnd_node = NULL;
   }
   else  /* Check if it is the first node of the list */
   if (prev_usr_node == NULL) {
      if (srmnd_usr->node_type == SRMA_SRMND_USR_TYPE_NODE)
         srmnd_usr->srmnd_info->start_usr_node = next_usr_node;          
      else  /* User Application list specific node */
         srmnd_usr->usr_appl->start_srmnd_node = next_usr_node;
      
      next_usr_node->prev_usr_node = NULL;
   }
   else /* Check if it is the last node of the list */
   if (next_usr_node == NULL) {
      prev_usr_node->next_usr_node = NULL;
   }
   else /* Middle node */
   {
      prev_usr_node->next_usr_node = next_usr_node;
      next_usr_node->prev_usr_node = prev_usr_node;
   }

   /* Delete the respective SRMND user node */
   m_MMGR_FREE_SRMA_SRMND_APPL_NODE(srmnd_usr);

   return;
}


/****************************************************************************
  Name          :  srma_del_rsrc_mon_from_appl_list
 
  Description   :  This function deletes the respective resource monitor node
                   from User-Application specific list. 
 
  Arguments     :  srma - ptr to the SRMA_CB structure
                   rsrc_mon - ptr to the Resource-Monitor node of type
                               SRMA_RSRC_MON
 
  Return Values :  Nothing
 
  Notes         :  None.
*****************************************************************************/
void srma_del_rsrc_mon_from_appl_list(SRMA_CB *srma, SRMA_RSRC_MON *rsrc_mon)
{
   SRMA_RSRC_MON *prev_usr_rsrc_mon = rsrc_mon->prev_usr_rsrc_mon;
   SRMA_RSRC_MON *next_usr_rsrc_mon = rsrc_mon->next_usr_rsrc_mon;

   /* Check if it is the only node of the list */
   if ((prev_usr_rsrc_mon == NULL) && 
       (next_usr_rsrc_mon == NULL)) {
       if (rsrc_mon->usr_appl->start_rsrc_mon == rsrc_mon)
          srma_del_srmnd_usr_node(srma, rsrc_mon->usr_appl);
       else
       {
          /* LOG: some thing went wrong here */
       }
   }
   else  /* Check if it is the first node of the list */
   if (prev_usr_rsrc_mon == NULL) {
      rsrc_mon->usr_appl->start_rsrc_mon = next_usr_rsrc_mon;
      next_usr_rsrc_mon->prev_usr_rsrc_mon = NULL;
   }
   else /* Check if it is the last node of the list */
   if (next_usr_rsrc_mon == NULL) {
      prev_usr_rsrc_mon->next_usr_rsrc_mon = NULL;
   }
   else /* Middle node */
   {
      prev_usr_rsrc_mon->next_usr_rsrc_mon = next_usr_rsrc_mon;
      next_usr_rsrc_mon->prev_usr_rsrc_mon = prev_usr_rsrc_mon;
   }

   /* Unlink the connection between the rsrc and srmnd */
   rsrc_mon->usr_appl = NULL;

   return;  
}


/****************************************************************************
  Name          :  srma_del_rsrc_mon_from_srmnd_list
 
  Description   :  This function deletes the respective resource monitor node
                   from SRMND specific list. 
 
  Arguments     :  srma - ptr to the SRMA_CB structure
                   rsrc_mon - ptr to the Resource-Monitor node of type
                               SRMA_RSRC_MON
 
  Return Values :  Nothing
 
  Notes         :  None.
*****************************************************************************/
void srma_del_rsrc_mon_from_srmnd_list(SRMA_CB *srma, SRMA_RSRC_MON *rsrc_mon)
{
   SRMA_RSRC_MON *prev_srmnd_rsrc = rsrc_mon->prev_srmnd_rsrc;
   SRMA_RSRC_MON *next_srmnd_rsrc = rsrc_mon->next_srmnd_rsrc;

   /* Check if it is the only node of the list */
   if ((prev_srmnd_rsrc == NULL) && 
       (next_srmnd_rsrc == NULL)) {
      if (rsrc_mon->srmnd_usr->start_rsrc_mon == rsrc_mon)
          srma_del_srmnd_usr_node(srma, rsrc_mon->srmnd_usr);         
      else
      {
         /* some thing went wrong here */
         m_SRMA_LOG_MISC(SRMA_LOG_MISC_DATA_INCONSISTENT,
                         SRMA_LOG_MISC_NOTHING,
                         NCSFL_SEV_ERROR);                       
      }
   }
   else  /* Check if it is the first node of the list */
   if (prev_srmnd_rsrc == NULL) {
      rsrc_mon->srmnd_usr->start_rsrc_mon = next_srmnd_rsrc;
      next_srmnd_rsrc->prev_srmnd_rsrc = NULL;
   }
   else /* Check if it is the last node of the list */
   if (next_srmnd_rsrc == NULL) {
      prev_srmnd_rsrc->next_srmnd_rsrc = NULL;
   }
   else /* Middle node */
   {
      prev_srmnd_rsrc->next_srmnd_rsrc = next_srmnd_rsrc;
      next_srmnd_rsrc->prev_srmnd_rsrc = prev_srmnd_rsrc;
   }

   /* Unlink the connection between the rsrc and srmnd */
   rsrc_mon->srmnd_usr = NULL;

   /* Make it 0 as we delete the resource record from respective SRMND */
   rsrc_mon->srmnd_rsrc_hdl = 0;

   return;
}


/****************************************************************************
  Name          :  srma_add_usr_appl_node
 
  Description   :  This function adds the application specific record to the 
                   SRMA's user-application list.
 
  Arguments     :  srma - ptr to the SRMA control block
                   appl - ptr to the user-application record of type 
                          SRMA_USR_APPL_NODE
 
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srma_add_usr_appl_node(SRMA_CB *srma, SRMA_USR_APPL_NODE *appl)
{    
    /* If list doesn't exists still, add the appl node as a starting node 
       of the application list. */
    if (!(srma->start_usr_appl_node))
    {
       srma->start_usr_appl_node = appl;
    }
    else /* Node exists, then dd the appl node as a starting node of the
            application list by pushing the existing node to second place */
    {
       /* Add new application node to the begining of the application list */
       appl->next_usr_appl_node = srma->start_usr_appl_node;
       srma->start_usr_appl_node->prev_usr_appl_node = appl;
       srma->start_usr_appl_node = appl;
    }

    return;
}


/****************************************************************************
  Name          :  srma_del_usr_appl_node
 
  Description   :  This function deletes the application specific record to
                   the SRMA's user-application list.
 
  Arguments     :  srma - ptr to the SRMA control block
                   appl - ptr to the user-application record of type 
                          SRMA_USR_APPL_NODE
 
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srma_del_usr_appl_node(SRMA_CB *srma, SRMA_USR_APPL_NODE *appl)
{   
   SRMA_USR_APPL_NODE *prev_usr_appl_node = appl->prev_usr_appl_node;
   SRMA_USR_APPL_NODE *next_usr_appl_node = appl->next_usr_appl_node;

   /* Check if it is the only node of the list */
   if ((prev_usr_appl_node == NULL) && 
       (next_usr_appl_node == NULL)) {
      srma->start_usr_appl_node = NULL;
   }
   else  /* Check if it is the first node of the list */
   if (prev_usr_appl_node == NULL) {
      srma->start_usr_appl_node = next_usr_appl_node;
      next_usr_appl_node->prev_usr_appl_node = NULL;
   }
   else /* Check if it is the last node of the list */
   if (next_usr_appl_node == NULL) {
      prev_usr_appl_node->next_usr_appl_node = NULL;
   }
   else /* Middle node */
   {
      prev_usr_appl_node->next_usr_appl_node = next_usr_appl_node;
      next_usr_appl_node->prev_usr_appl_node = prev_usr_appl_node;
   }

   /* Free the allocated memory for application node */
   m_MMGR_FREE_SRMA_USR_APPL_NODE(appl);

   return;
}


/****************************************************************************
  Name          :  srma_validate_mon_info
 
  Description   :  This routine validates the resource monitor data, esp,,
                   resource specific data that was requested by an application.

  Arguments     :  rsrc - Ptr to the resource specific data struct                  
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_validate_mon_info(NCS_SRMSV_MON_INFO *mon_info, 
                             NODE_ID  node_num)
{
   NCS_SRMSV_RSRC_TYPE rsrc_type = mon_info->rsrc_info.rsrc_type;
  
   /* By default for watermark user-type should be always NCS_SRMSV_USER_REQUESTOR */
   if (mon_info->monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_WATERMARK)
      mon_info->rsrc_info.usr_type = NCS_SRMSV_USER_REQUESTOR;

   /* Validate the RSRC info content */
   if (srma_validate_rsrc_info(&mon_info->rsrc_info,
                               node_num) != NCSCC_RC_SUCCESS)
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_VALIDATE_RSRC,
                      SRMA_LOG_MISC_FAILED,
                      NCSFL_SEV_ERROR);

      return NCSCC_RC_FAILURE;
   }

   /* Validate the MON data content */
   if (srma_validate_mon_data(&mon_info->monitor_data,
                              rsrc_type) != NCSCC_RC_SUCCESS)
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_VALIDATE_MON,
                      SRMA_LOG_MISC_FAILED,
                      NCSFL_SEV_ERROR);

      return NCSCC_RC_FAILURE;
   }


   return NCSCC_RC_SUCCESS;             
}


/****************************************************************************
  Name          :  srma_validate_rsrc_info
 
  Description   :  This routine validates the resource monitor data, esp,,
                   resource specific data that was requested by an application.

  Arguments     :  rsrc - Ptr to the resource specific data struct                  
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_validate_rsrc_info(NCS_SRMSV_RSRC_INFO *rsrc, 
                              NODE_ID  node_num)
{
   /* Valid Resource Type */
   if ((rsrc->rsrc_type <= 0) || (rsrc->rsrc_type >= NCS_SRMSV_RSRC_END))
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_RSRC_TYPE,
                      SRMA_LOG_MISC_NOTHING,
                      NCSFL_SEV_ERROR);

      return NCSCC_RC_FAILURE;
   }

   /* Valid User Type */
   if ((rsrc->usr_type <= 0) || (rsrc->usr_type >= NCS_SRMSV_USER_END))
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_USER_TYPE,
                      SRMA_LOG_MISC_NOTHING,
                      NCSFL_SEV_ERROR);

      return NCSCC_RC_FAILURE;
   }

   /* Validate SRMND location */
   switch (rsrc->srmsv_loc)
   {
   case NCS_SRMSV_NODE_LOCAL: 
   case NCS_SRMSV_NODE_ALL:
       rsrc->node_num = node_num;
       break;

   case NCS_SRMSV_NODE_REMOTE:
       /* Node number is must and it should not be local node id */
       if ((rsrc->node_num < 1) || (rsrc->node_num == node_num))
       {
          m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_NODE_ID,
                          SRMA_LOG_MISC_NOTHING,
                          NCSFL_SEV_ERROR);

          return NCSCC_RC_FAILURE;
       }
       break;
  
   default: /* Default is Local */
       rsrc->srmsv_loc = NCS_SRMSV_NODE_LOCAL;
       rsrc->node_num = node_num;
       break;
   }
   
   /* For Process related resource types, PID is must */
   switch (rsrc->rsrc_type)
   {
   case NCS_SRMSV_RSRC_PROC_EXIT:
       /* Child level should be >= -1 */
       if (rsrc->child_level < SRMA_PROC_DESCENDANTS)
       {
          m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_CHILD_LEVEL,
                          SRMA_LOG_MISC_NOTHING,
                          NCSFL_SEV_ERROR);

          return NCSCC_RC_FAILURE;
       }

       if (rsrc->srmsv_loc != NCS_SRMSV_NODE_LOCAL)
       {
          m_SRMA_LOG_MISC(SRMA_LOG_MISC_PID_NOT_LOCAL,
                          SRMA_LOG_MISC_NOTHING,
                          NCSFL_SEV_ERROR);

          return NCSCC_RC_FAILURE;             
       }
   
       /* Don't put the BREAK here, as the following checks also required */

   case NCS_SRMSV_RSRC_PROC_MEM:
   case NCS_SRMSV_RSRC_PROC_CPU:
       /* PID should not be < 0 */
       if (rsrc->pid <= 0)
       {
          m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_PID,
                          SRMA_LOG_MISC_NOTHING,
                          NCSFL_SEV_ERROR);

          return NCSCC_RC_FAILURE;
       }

       /* If it is a local node, check about the process exists here itself */
       if (rsrc->srmsv_loc == NCS_SRMSV_NODE_LOCAL)
       {
          NCS_BOOL is_thread = FALSE;

          if (m_SRMSV_CHECK_PROCESS_EXIST(rsrc->pid, &is_thread) != TRUE)
          {
             m_SRMA_LOG_MISC(SRMA_LOG_MISC_PID_NOT_EXIST,
                             SRMA_LOG_MISC_NOTHING,
                             NCSFL_SEV_ERROR);

             return NCSCC_RC_FAILURE;             
          }

          if (is_thread)
             rsrc->child_level = 0; /* no descendants */
       }

       if ((rsrc->rsrc_type == NCS_SRMSV_RSRC_PROC_MEM) ||
           (rsrc->rsrc_type == NCS_SRMSV_RSRC_PROC_CPU))
       {
          /* Bcast option doesn't work for this rsrc_type */
          if (rsrc->srmsv_loc == NCS_SRMSV_NODE_ALL)
          {
             m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_LOC_TYPE,
                             SRMA_LOG_MISC_NOTHING,
                             NCSFL_SEV_ERROR);

             return NCSCC_RC_FAILURE;             
          }

          /* Purely of this rsrc type, no child info required */
          rsrc->child_level = 0; /* no descendants */
       }
       break;

   default:
       rsrc->pid = 0;         /* Made it to zero for other resources */
       rsrc->child_level = 0; /* Made it to zero for other resources */
       break;
   }

   return NCSCC_RC_SUCCESS;             
}


/****************************************************************************
  Name          :  srma_validate_value
 
  Description   :  This routine validates the resource specific threshold
                   value that was requested by an application.

  Arguments     :  val - Ptr to the NCS_SRMSV_VALUE struct                  
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_validate_value(NCS_SRMSV_VALUE *val, NCS_SRMSV_RSRC_TYPE rsrc_type)
{
   if ((val->val_type <= 0) || (val->val_type >= NCS_SRMSV_VAL_TYPE_END))
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_VAL_TYPE,
                      SRMA_LOG_MISC_NOTHING,
                      NCSFL_SEV_ERROR);

      /* Update with default parameter */
      val->val_type = NCS_SRMSV_VAL_TYPE_INT32;      
   }

   if ((val->scale_type <= 0) || (val->scale_type >= NCS_SRMSV_STAT_SCALE_END))
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_SCALE_TYPE,
                      SRMA_LOG_MISC_NOTHING,
                      NCSFL_SEV_ERROR);

      return NCSCC_RC_FAILURE;
   }
   else /* For CPU specific resources, scale_type should be in % 'percent' only */
   {
      switch(rsrc_type)
      {
      case NCS_SRMSV_RSRC_CPU_UTIL: 
      case NCS_SRMSV_RSRC_CPU_KERNEL:
      case NCS_SRMSV_RSRC_CPU_USER: 
      case NCS_SRMSV_RSRC_CPU_UTIL_ONE:
      case NCS_SRMSV_RSRC_CPU_UTIL_FIVE:
      case NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN:
      case NCS_SRMSV_RSRC_PROC_CPU:
          if (val->scale_type != NCS_SRMSV_STAT_SCALE_PERCENT) 
          {
             m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_SCALE_TYPE,
                             SRMA_LOG_MISC_NOTHING,
                             NCSFL_SEV_ERROR);
   
             return NCSCC_RC_FAILURE;
          }
          break;

      default:
          break;
      }
   }
  
   /* % values should always considered in float types */
   if (val->scale_type == NCS_SRMSV_STAT_SCALE_PERCENT) 
   {
      switch (val->val_type)
      {
      case NCS_SRMSV_VAL_TYPE_INT32:
          val->val.f_val = (double) val->val.i_val32;
          break;

      case NCS_SRMSV_VAL_TYPE_UNS32:
          val->val.f_val = (double) val->val.u_val32; 
          break;

      case NCS_SRMSV_VAL_TYPE_INT64:
          val->val.f_val = (double) val->val.i_val64; 
          break;

      case NCS_SRMSV_VAL_TYPE_UNS64:
          val->val.f_val = (double) val->val.u_val64;
          break;

      default:
          break;
      }

      if (val->val.f_val > 100)
         val->val.f_val = 100;  /* Max percent value */   

      /* Update with default parameter */
      val->val_type = NCS_SRMSV_VAL_TYPE_FLOAT;      
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srma_validate_mon_data
 
  Description   :  This routine validates the resource monitor data, esp.. 
                   monitor specific data that was requested by an application.

  Arguments     :  mon_data - Ptr to the NCS_SRMSV_MON_DATA struct                  
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_validate_mon_data(NCS_SRMSV_MON_DATA *mon_data, NCS_SRMSV_RSRC_TYPE rsrc_type)
{
   /* invalid monitoring rate?? so set to default monitoring rate i.e. 5secs */
   if (mon_data->monitoring_rate <= 0)
      mon_data->monitoring_rate = SRMSV_DEF_MONITORING_RATE;

   switch (mon_data->monitor_type)
   {
   case NCS_SRMSV_MON_TYPE_THRESHOLD:
       {
          NCS_SRMSV_THRESHOLD_TEST_CONDITION condition;

          /* Not required to validate monitor data for process specific resource type,
          monitor data is just ignored in this case */
          if (rsrc_type == NCS_SRMSV_RSRC_PROC_EXIT)
             return NCSCC_RC_SUCCESS;

          if (srma_validate_value(&mon_data->mon_cfg.threshold.threshold_val, rsrc_type)
               != NCSCC_RC_SUCCESS)
             return NCSCC_RC_FAILURE;

          condition = mon_data->mon_cfg.threshold.condition;
          if ((condition <= 0) || (condition >= NCS_SRMSV_THRESHOLD_VAL_IS_END))
          {
             m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_CONDITION,
                             SRMA_LOG_MISC_NOTHING,
                             NCSFL_SEV_ERROR);

             return NCSCC_RC_FAILURE;
          }

          if ((condition == NCS_SRMSV_THRESHOLD_VAL_IS_AT) || 
              (condition == NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT))
          {
             if ((mon_data->mon_cfg.threshold.tolerable_val.scale_type != 0) && 
                 (mon_data->mon_cfg.threshold.tolerable_val.scale_type != mon_data->mon_cfg.threshold.threshold_val.scale_type))
             {
                m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_TOLVAL_SCALE_TYPE,
                                SRMA_LOG_MISC_NOTHING,
                                NCSFL_SEV_ERROR);

                /* Log the message */
                return NCSCC_RC_FAILURE;
             }

             /* Tolerable value's val_type and scale_type should be same as threshold value */
             if ((mon_data->mon_cfg.threshold.tolerable_val.val_type != 0) && 
                 (mon_data->mon_cfg.threshold.tolerable_val.val_type != mon_data->mon_cfg.threshold.threshold_val.val_type))
             {
                m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_TOLVAL_VALUE_TYPE,
                                SRMA_LOG_MISC_NOTHING,
                                NCSFL_SEV_ERROR);

                /* Log the message */
                return NCSCC_RC_FAILURE;
             }
          
             mon_data->mon_cfg.threshold.tolerable_val.val_type = mon_data->mon_cfg.threshold.threshold_val.val_type;
             mon_data->mon_cfg.threshold.tolerable_val.scale_type = mon_data->mon_cfg.threshold.threshold_val.scale_type;
          }

          /* Default sample is 1 */
          if (mon_data->mon_cfg.threshold.samples < 1)
             mon_data->mon_cfg.threshold.samples = 1;
       }
       break;

   case NCS_SRMSV_MON_TYPE_WATERMARK:
       {
          /* By default watermark monitor type is always DUAL */
          mon_data->mon_cfg.watermark_type = NCS_SRMSV_WATERMARK_DUAL; 

          if (rsrc_type == NCS_SRMSV_RSRC_PROC_EXIT)
          {
             /* Inconsistency: For this monitor type its an invalid rsrc_type */ 
             return NCSCC_RC_FAILURE;
          }
       }
       break;

   case NCS_SRMSV_MON_TYPE_LEAKY_BUCKET:
       if (rsrc_type == NCS_SRMSV_RSRC_PROC_EXIT)
       {
          /* Inconsistency: For this monitor type its an invalid rsrc_type */ 
          return NCSCC_RC_FAILURE;
       }

       if (srma_validate_value(&mon_data->mon_cfg.leaky_bucket.bucket_size, rsrc_type) 
          != NCSCC_RC_SUCCESS)
       {
          /* Log the message */
          return NCSCC_RC_FAILURE;
       }

       if (srma_validate_value(&mon_data->mon_cfg.leaky_bucket.fill_size, rsrc_type) 
          != NCSCC_RC_SUCCESS)
       {
          /* Log the message */
          return NCSCC_RC_FAILURE;
       }
       break;

   default:
      /* If rsrc_typpe is just PROC_EXIT, just return error */
      if (rsrc_type == NCS_SRMSV_RSRC_PROC_EXIT)
         return NCSCC_RC_SUCCESS;

      m_SRMA_LOG_MISC(SRMA_LOG_MISC_INVALID_MON_TYPE,
                      SRMA_LOG_MISC_NOTHING,
                      NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
      break;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srma_check_duplicate_mon_info
 
  Description   :  This routine checks whether the resource monitor request 
                   is already made by the application.

  Arguments     :  appl - Ptr to the SRMA_USR_APPL_NODE struct
                          (application specific)
                   mon_info - Ptr to the NCS_SRMSV_MON_INFO struct 
                          (Resource Monitor Data).
 
  Return Values :  ptr of the matched resource record (or) NULL
 
  Notes         :  None.
******************************************************************************/
SRMA_RSRC_MON *srma_check_duplicate_mon_info(SRMA_USR_APPL_NODE *appl,
                                             NCS_SRMSV_MON_INFO *info,
                                             NCS_BOOL only_rsrc_info)
{
   SRMA_SRMND_USR_NODE *usr_srmnd = appl->start_srmnd_node;
   SRMA_RSRC_MON *rsrc = SRMA_RSRC_MON_NULL;   

   /* Traverse through all the resources of an application, check
      whether the requested resource monitor is already requested */
   while (usr_srmnd)   
   {
       rsrc = usr_srmnd->start_rsrc_mon;
       while (rsrc) 
       {           
           if (!memcmp(&info->rsrc_info,
                                &rsrc->rsrc_info, sizeof(NCS_SRMSV_RSRC_INFO))) 
           {
               /* If it is TRUE then return the respective resource */
               if (only_rsrc_info != FALSE)
                  return rsrc;
               
               if (!memcmp(&info->monitor_data, &rsrc->monitor_data, 
                           sizeof(NCS_SRMSV_MON_DATA)))               
                  return rsrc;
           }
           rsrc = rsrc->next_usr_rsrc_mon;

       } /* End of while(rsrc) loop */

       usr_srmnd = usr_srmnd->next_usr_node;
   } /* End of while (usr_srmnd) loop */
   
   return NULL;
}


/****************************************************************************
  Name          :  srma_add_rsrc_mon_to_appl_list
 
  Description   :  This routine adds the resource monitor record to the user 
                   application maintained by the SRMSv agent (SRMA).

  Arguments     :  srma - Ptr to the SRMA CB
                   appl - Ptr to the SRMA_USR_APPL_NODE struct
                          (application specific)
                   rsrc - Ptr to the SRMA_RSRC_MON struct 
                          (Resource Monitor Record).
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_add_rsrc_mon_to_appl_list(SRMA_CB *srma,
                                     SRMA_RSRC_MON *rsrc,
                                     SRMA_USR_APPL_NODE *appl)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    SRMA_SRMND_USR_NODE *srmnd_usr = SRMA_SRMND_USR_NODE_NULL;
    SRMA_SRMND_INFO *srmnd = SRMA_SRMND_INFO_NULL;
    
    if (rsrc->srmnd_usr)
       srmnd = rsrc->srmnd_usr->srmnd_info;
    
    if (!(srmnd))
    {
       m_SRMA_LOG_MISC(SRMA_LOG_MISC_RSRC_SRMND_MISSING,
                       SRMA_LOG_MISC_NOTHING,
                       NCSFL_SEV_ERROR);

       /* SRMND association information is missing in rsrc specific data, 
          first try to make an association with SRMND list */
       return NCSCC_RC_FAILURE;
    }

    /* Check whether the srmnd-usr record exists, if not create it and the 
       rsrc to it. */
    if ((srmnd_usr = srma_check_srmnd_usr_exists(srmnd,
                                          appl,
                                          SRMA_USR_SRMND_TYPE_NODE)) == NULL)
    {
       srmnd_usr = srma_create_srmnd_usr_add_rsrc(appl,
                                                  rsrc,
                                                  SRMA_USR_SRMND_TYPE_NODE);
       srmnd_usr->srmnd_info = srmnd;
    }
    else
    {
       if (!(srmnd_usr->start_rsrc_mon))
       {
          srmnd_usr->start_rsrc_mon = rsrc; 
       }
       else
       {
          rsrc->next_usr_rsrc_mon = srmnd_usr->start_rsrc_mon;
          srmnd_usr->start_rsrc_mon->prev_usr_rsrc_mon = rsrc;
          srmnd_usr->start_rsrc_mon = rsrc;
       }

       rsrc->usr_appl = srmnd_usr;
    }

    return rc;
}


/****************************************************************************
  Name          :  srma_create_srmnd_usr_add_rsrc
 
  Description   :  Based on the node type resource monitor record will be 
                   added to user-srmnd-user of either user application list
                   or srmnd list

  Arguments     :  srmnd_or_appl - void ptr, of either application node or 
                                   srmnd node
                   rsrc - Ptr to the SRMA_RSRC_MON struct 
                          (Resource Monitor Record).
                   node_type - Can be either SRMA_SRMND_USR_NODE or 
                                             SRMA_USR_SRMND_TYPE_NODE
 
  Return Values :  Ptr of the added SRMA_SRMND_USR_NODE.
 
  Notes         :  None.
******************************************************************************/
SRMA_SRMND_USR_NODE *srma_create_srmnd_usr_add_rsrc(NCSCONTEXT srmnd_or_appl,
                                                    SRMA_RSRC_MON *rsrc,
                                           SRMA_SRMND_USR_NODE_TYPE node_type)
{
   SRMA_SRMND_USR_NODE *srmnd_usr = SRMA_SRMND_USR_NODE_NULL;

   if (!(srmnd_usr = m_MMGR_ALLOC_SRMA_SRMND_APPL_NODE))
   {
      m_SRMA_LOG_MEM(SRMA_MEM_SRMND_APPL,
                     SRMA_MEM_ALLOC_FAILED,
                     NCSFL_SEV_CRITICAL);
      return NULL;
   }
   memset((char *)srmnd_usr, 0, sizeof(SRMA_SRMND_USR_NODE));

   /* Update the node type, whether srmnd_usr belongs to 
      SRMND specific list or User-Appl specific list */
   srmnd_usr->node_type = node_type;

   if (node_type == SRMA_SRMND_USR_TYPE_NODE)  /* Belongs to SRMND list */
   {
      SRMA_SRMND_INFO *srmnd = SRMA_SRMND_INFO_NULL;

      srmnd = (SRMA_SRMND_INFO *)srmnd_or_appl;
      srmnd_usr->srmnd_info = srmnd;
      rsrc->srmnd_usr = srmnd_usr;
      
      /* Add srmnd_usr node to srmnd node */
      if (!(srmnd->start_usr_node))
      {
         srmnd->start_usr_node = srmnd_usr;
      }
      else /* Add as a first node of the SRMND list */
      {
         srmnd_usr->next_usr_node = srmnd->start_usr_node;
         srmnd->start_usr_node->prev_usr_node = srmnd_usr;
         srmnd->start_usr_node = srmnd_usr;
      }
   }
   else /* Belongs to User-Application list */
   {
      SRMA_USR_APPL_NODE *appl = SRMA_USR_APPL_NODE_NULL;

      appl = (SRMA_USR_APPL_NODE *)srmnd_or_appl;
      srmnd_usr->usr_appl = appl;
      rsrc->usr_appl = srmnd_usr;

      /* Add srmnd_usr node to srmnd node */
      if (!(appl->start_srmnd_node))
      {
         appl->start_srmnd_node = srmnd_usr;
      }
      else /* Add as a first node of the SRMND list */
      {
         srmnd_usr->next_usr_node = appl->start_srmnd_node;
         appl->start_srmnd_node->prev_usr_node = srmnd_usr;
         appl->start_srmnd_node = srmnd_usr;
      }
   }

   srmnd_usr->start_rsrc_mon = rsrc;

   return srmnd_usr;
}


/****************************************************************************
  Name          :  srma_check_srmnd_usr_exists
 
  Description   :  Based on the node type, this routine checks either 
                   application specific list or srmnd specific list for the 
                   existence of user-srmnd-user node. If it doesn't exists, 
                   it returns NULL.

  Arguments     :  srmnd - ptr to the SRMA_SRMND_INFO struct (srmnd specific)
                   appl -  ptr to the SRMA_USR_APPL_NODE struct (appl specific)
                   node_type - Can be either SRMA_SRMND_USR_TYPE_NODE or 
                                             SRMA_USR_SRMND_TYPE_NODE
 
  Return Values :  Ptr of the added SRMA_SRMND_USR_NODE.
 
  Notes         :  None.
******************************************************************************/
SRMA_SRMND_USR_NODE *srma_check_srmnd_usr_exists(SRMA_SRMND_INFO *srmnd,
                                                 SRMA_USR_APPL_NODE *appl,
                                                 SRMA_SRMND_USR_NODE_TYPE node_type)
{
   SRMA_SRMND_USR_NODE *srmnd_usr = NULL;
   NCS_BOOL  got_it = FALSE;
   
   switch (node_type)
   {
   case SRMA_SRMND_USR_TYPE_NODE:
       srmnd_usr = srmnd->start_usr_node;
       while (srmnd_usr)
       {
          if (srmnd_usr->usr_appl)
          {
             if (srmnd_usr->usr_appl->user_hdl == appl->user_hdl)
             {
                got_it = TRUE;
                break;
             }
          }
          else
          {
             m_SRMA_LOG_MISC(SRMA_LOG_MISC_RSRC_USER_MISSING,
                             SRMA_LOG_MISC_NOTHING,
                             NCSFL_SEV_ERROR);
          }
          
          srmnd_usr = srmnd_usr->next_usr_node;
       }
       break;

   case SRMA_USR_SRMND_TYPE_NODE:
       srmnd_usr = appl->start_srmnd_node;
       while (srmnd_usr)
       {
          if (srmnd_usr->srmnd_info)
          {             
             if (!memcmp(&srmnd_usr->srmnd_info->srmnd_dest, &srmnd->srmnd_dest,
                         sizeof(MDS_DEST)))           
             {
                got_it = TRUE;
                break;
             }
          }
          else
          {
             m_SRMA_LOG_MISC(SRMA_LOG_MISC_RSRC_SRMND_MISSING,
                             SRMA_LOG_MISC_NOTHING,
                             NCSFL_SEV_ERROR);
          }

          srmnd_usr = srmnd_usr->next_usr_node;
       }
       break;

   default:
       break;
   }

   /* Respective srmnd_user node doesn't exists */
   if (!got_it)
      return NULL;

   return srmnd_usr;
}


/****************************************************************************
  Name          :  srma_check_srmnd_exists
 
  Description   :  This routine gets the respective SRMND specific node by
                   MDS_DEST.

  Arguments     :  srma - ptr to the SRMA Control Block
                   srmnd_dest - MDS_DEST to check for                   
 
  Return Values :  ptr to SRMA_SRMND_INFO (or) NULL.
 
  Notes         :  None.
******************************************************************************/
SRMA_SRMND_INFO *srma_check_srmnd_exists(SRMA_CB *srma, NODE_ID node_id)
{
   SRMA_SRMND_INFO *srmnd = srma->start_srmnd_node;

   /* Traverse SRMND list and return respective SRMND specific node if the 
      srmnd_dest matches with it */
   while (srmnd)
   {
      /* Node-Id matched.. so return the respective srmnd */
      if (srmnd->node_id == node_id)
         return srmnd;

      srmnd = srmnd->next_srmnd_node;
   }
  
   return NULL;
}


/****************************************************************************
  Name          :  srma_add_srmnd_node
 
  Description   :  This routine creates SRMND node and adds it to the SRMND
                   list.

  Arguments     :  srma - ptr to the SRMA Control Block
                   srmnd_dest - MDS dest of the SRMND                   
 
  Return Values :  srmnd node ptr / NULL
 
  Notes         :  None.
******************************************************************************/
SRMA_SRMND_INFO *srma_add_srmnd_node(SRMA_CB *srma, NODE_ID node_id)
{
   SRMA_SRMND_INFO *srmnd = NULL;

   if (!(srmnd = m_MMGR_ALLOC_SRMA_SRMND_INFO))
   {
      m_SRMA_LOG_MEM(SRMA_MEM_SRMND_INFO,
                     SRMA_MEM_ALLOC_FAILED,
                     NCSFL_SEV_CRITICAL);
      return srmnd;
   }

   memset((char *)srmnd, 0, sizeof(SRMA_SRMND_INFO));
   
   /* Update the node id */
   srmnd->node_id = node_id;

   /* No SRMND nodes exists in the SRMND list of SRMA_CB */
   if (!(srma->start_srmnd_node)) 
   {
      srma->start_srmnd_node = srmnd;
   }
   else /* Add as a first node of the SRMND list */
   {
      srmnd->next_srmnd_node = srma->start_srmnd_node;
      srma->start_srmnd_node->prev_srmnd_node = srmnd;
      srma->start_srmnd_node = srmnd;
   }

   return srmnd;
}


/****************************************************************************
  Name          :  srma_add_rsrc_mon_to_srmnd_list
 
  Description   :  This routine adds the resource monitor record to the 
                   respective SRMND list.

  Arguments     :  srma - ptr to the SRMA Control Block
                   rsrc - ptr to the resource monitor record
                   appl - ptr to the SRMA_USR_APPL_NODE struct (appl specific)
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE.
 
  Notes         :  None.
******************************************************************************/
uns32 srma_add_rsrc_mon_to_srmnd_list(SRMA_CB *srma,
                                      SRMA_RSRC_MON *rsrc,
                                      SRMA_USR_APPL_NODE *appl,
                                      NODE_ID node_id)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    SRMA_SRMND_INFO *srmnd = SRMA_SRMND_INFO_NULL;
    SRMA_SRMND_USR_NODE *srmnd_usr = SRMA_SRMND_USR_NODE_NULL;    
    
    if ((srmnd = srma_check_srmnd_exists(srma, node_id)) == NULL)
    {
       srmnd = srma_add_srmnd_node(srma, node_id);
       if (srmnd == NULL)
          return NCSCC_RC_FAILURE;
              
       srmnd_usr = srma_create_srmnd_usr_add_rsrc(srmnd, 
                                                  rsrc,
                                                  SRMA_SRMND_USR_TYPE_NODE);
       srmnd_usr->usr_appl = appl;
    }
    else
    {
       if ((srmnd_usr = srma_check_srmnd_usr_exists(srmnd,
                                        appl,
                                        SRMA_SRMND_USR_TYPE_NODE)) == NULL)
       {
          srmnd_usr = srma_create_srmnd_usr_add_rsrc(srmnd,
                                                     rsrc,
                                                     SRMA_SRMND_USR_TYPE_NODE);
          srmnd_usr->usr_appl = appl;
       }
       else
       {
          if (!(srmnd_usr->start_rsrc_mon))
          {
             srmnd_usr->start_rsrc_mon = rsrc; 
          }
          else
          {
             rsrc->next_srmnd_rsrc = srmnd_usr->start_rsrc_mon;
             srmnd_usr->start_rsrc_mon->prev_srmnd_rsrc = rsrc;
             srmnd_usr->start_rsrc_mon = rsrc;
          }

          rsrc->srmnd_usr = srmnd_usr;
       }
    }

    return rc;
}


/****************************************************************************
  Name          :  srma_add_rsrc_mon_to_bcast_list
 
  Description   :  This routine adds resource monitor record to bcast list 
                   of SRMA.

  Arguments     :  srma - ptr to the SRMA Control Block
                   rsrc_mon_hdl - returns the handle of the created resource
                                  monitor record
 
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srma_add_rsrc_mon_to_bcast_list(SRMA_RSRC_MON *rsrc, 
                                     SRMA_USR_APPL_NODE *appl)
{

   if (!(appl))
      return;
   
   /* Empty list ?? */
   if (appl->bcast_rsrcs == NULL)
      appl->bcast_rsrcs = rsrc;
   else /* List has some rsrc(s) */
   {
      appl->bcast_rsrcs->prev_srmnd_rsrc = rsrc;
      rsrc->next_srmnd_rsrc = appl->bcast_rsrcs;
      appl->bcast_rsrcs = rsrc;
   }

   rsrc->bcast_appl = appl;
   appl->bcast++; 

   return;
}


/****************************************************************************
  Name          :  srma_del_rsrc_mon_from_bcast_list
 
  Description   :  This routine deletes resource monitor record from bcast
                   list of SRMA.

  Arguments     :  srma - ptr to the SRMA Control Block
                   rsrc_mon_hdl - returns the handle of the created resource
                                  monitor record
 
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srma_del_rsrc_mon_from_bcast_list(SRMA_RSRC_MON *rsrc_mon, 
                                       SRMA_USR_APPL_NODE *appl)
{  
   SRMA_RSRC_MON *prev_srmnd_rsrc = rsrc_mon->prev_srmnd_rsrc;
   SRMA_RSRC_MON *next_srmnd_rsrc = rsrc_mon->next_srmnd_rsrc;

   if ((rsrc_mon->rsrc_info.srmsv_loc != NCS_SRMSV_NODE_ALL) || (appl == NULL)) 
      return;
  
   /* Check if it is the only node of the list */
   if ((prev_srmnd_rsrc == NULL) && 
       (next_srmnd_rsrc == NULL)) {
      if (appl->bcast_rsrcs == rsrc_mon)
      {
         appl->bcast_rsrcs = NULL;
         appl->bcast = 0;
      }
      else
      {
         /* some thing went wrong here */
         m_SRMA_LOG_MISC(SRMA_LOG_MISC_DATA_INCONSISTENT,
                         SRMA_LOG_MISC_NOTHING,
                         NCSFL_SEV_ERROR);
      }
   }
   else  /* Check if it is the first node of the list */
   if (prev_srmnd_rsrc == NULL) {
      appl->bcast_rsrcs = next_srmnd_rsrc;
      next_srmnd_rsrc->prev_srmnd_rsrc = NULL;
   }
   else /* Check if it is the last node of the list */
   if (next_srmnd_rsrc == NULL) {
      prev_srmnd_rsrc->next_srmnd_rsrc = NULL;
   }
   else /* Middle node */
   {
      prev_srmnd_rsrc->next_srmnd_rsrc = next_srmnd_rsrc;
      next_srmnd_rsrc->prev_srmnd_rsrc = prev_srmnd_rsrc;
   }
  
   if (appl->bcast > 0)
      appl->bcast--;
 
   rsrc_mon->bcast_appl = NULL;

   return;
}


/****************************************************************************
  Name          :  srma_send_srmnd_rsrc_mon_data
 
  Description   :  This routine synchronizes SRMND specific SRMA resource
                   monitor data with SRMND.

  Arguments     :  srma - ptr to the SRMA Control Block
                   srmnd_dest - MDS_DEST of SRMND to which the data need to 
                                be updated.
 
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srma_send_srmnd_rsrc_mon_data(SRMA_CB *srma,
                                   MDS_DEST *srmnd_dest,
                                   uns32 usr_hdl)
{
   SRMA_SRMND_INFO *srmnd = NULL;
   MDS_DEST        dest;
   SRMA_RSRC_MON   *rsrc = NULL;
   SRMA_SRMND_USR_NODE *srmnd_usr = NULL;


   /* Destination node does'nt exits?? so just return */
   if ((srmnd = srma_check_srmnd_exists(srma, m_NCS_NODE_ID_FROM_MDS_DEST(*srmnd_dest)))
       == NULL)
      return;

   /* SRMND is not UP.. so no need to send the data */
   if (srmnd->is_srmnd_up != TRUE)
      return;
      
   memset(&dest, 0, sizeof(MDS_DEST));
   dest = srmnd->srmnd_dest;

   srmnd_usr = srmnd->start_usr_node;
   while (srmnd_usr)
   {
       /* No application specific ptr.. ok continue for next one */
       if (!(srmnd_usr->usr_appl))
       {
          /* get next user application node */
          srmnd_usr = srmnd_usr->next_usr_node;
          continue;
       }
        
       /* user handle mismatched.. ok continue for next one */
       if (srmnd_usr->usr_appl->user_hdl != usr_hdl)
       {
          /* get next user application node */
          srmnd_usr = srmnd_usr->next_usr_node;
          continue;
       }       

       rsrc = srmnd_usr->start_rsrc_mon;
       while (rsrc)
       { 
          srma_send_rsrc_msg(srma, 
                           srmnd_usr->usr_appl,
                           &rsrc->srmnd_usr->srmnd_info->srmnd_dest, 
                           SRMA_CREATE_RSRC_MON_MSG,
                           rsrc);

           rsrc = rsrc->next_srmnd_rsrc;
        }

       /* I am done, because the list has only one application specific node
          and that one I got it now */
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srma_inform_appl_rsrc_expiry
 
  Description   :  This routine informs the application about the expiry
                   of the configured resource.

  Arguments     :  srma - ptr to the SRMA Control Block
                   rsrc - ptr to the rsrc-mon structure
 
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_inform_appl_rsrc_expiry(SRMA_CB *srma, SRMA_RSRC_MON *rsrc)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_SRMSV_RSRC_CBK_INFO cbk_info;

   memset(&cbk_info, 0, sizeof(NCS_SRMSV_RSRC_CBK_INFO));

   cbk_info.rsrc_mon_hdl = rsrc->rsrc_mon_hdl;
   cbk_info.notif_type   = SRMSV_CBK_NOTIF_RSRC_MON_EXPIRED;

   /* Add the rsrc-mon expired notification message to cbk_list of 
      an application */
   if (rsrc->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
      rc = srma_update_appl_cbk_info(srma, &cbk_info, rsrc->bcast_appl);
   else
      rc = srma_update_appl_cbk_info(srma, &cbk_info, rsrc->usr_appl->usr_appl);

   return rc;
}


