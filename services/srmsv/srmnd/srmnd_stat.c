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

  MODULE NAME: SRMND_STAT.C
 
..............................................................................

  DESCRIPTION: This file contains the routines that are used to access/collect
               the resource statistics data.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srmnd.h"


static void srmnd_update_mem_stat(uns32 i_val, uns32 total, 
                                  NCS_SRMSV_VALUE *value);

/****************************************************************************
  Name          :  srmnd_get_cpu_utilization_stats
 
  Description   :  This routine updates the CPU utilization statistics 
                   (CPU user utilization, CPU kernel utilization) information
                   and updates it in its sample record.

  Arguments     :  rsrc - ptr to the SRMND_RSRC_MON_NODE structure                   
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  None.
******************************************************************************/
uns32 srmnd_get_cpu_utilization_stats(SRMND_RSRC_MON_NODE *rsrc)
{
   uns32 totaltime = 0;
   uns32 last_totaltime = 0;   
   SRMND_CPU_DATA  cpu_data;
   uns32 *rsrc_usertime   = &rsrc->mon_data.info.cpu_data.usertime;
   uns32 *rsrc_nicertime  = &rsrc->mon_data.info.cpu_data.nicertime;
   uns32 *rsrc_kerneltime = &rsrc->mon_data.info.cpu_data.kerneltime;
   uns32 *rsrc_idletime   = &rsrc->mon_data.info.cpu_data.idletime;
   NCS_SRMSV_VALUE   *result_value = &rsrc->mon_data.result_value;
   SRMND_SAMPLE_DATA *sample_data;
   NCS_SRMSV_RSRC_TYPE rsrc_type = rsrc->rsrc_type_node->rsrc_type;
   double f_total_time = 0, f_diff_time = 0;
   
   memset(&cpu_data, 0, sizeof(SRMND_CPU_DATA)); 

   /* Get the CPU utilization specific statistics */
   if (m_SRMSV_GET_CPU_UTIL_STATS(&cpu_data) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;
   
   /* Get the timestamp at which the stats are collected */ 
   m_GET_TIME_STAMP(rsrc->mon_data.recent_updated);
   
   totaltime = (cpu_data.usertime + cpu_data.nicertime + 
                cpu_data.kerneltime + cpu_data.idletime);
 
   last_totaltime = (*rsrc_usertime + *rsrc_nicertime +
                     *rsrc_kerneltime + *rsrc_idletime); 

   f_total_time = (double)(totaltime - last_totaltime);

   /* No change.. then just return */
   if (f_total_time == 0)
      return NCSCC_RC_SUCCESS;
   
   switch (rsrc_type)
   {
   case NCS_SRMSV_RSRC_CPU_UTIL:
       f_diff_time = (double) (cpu_data.idletime - *rsrc_idletime);     
       result_value->val.f_val =
                            (100 - (((f_diff_time)/(f_total_time)) * 100));
       break;

   case NCS_SRMSV_RSRC_CPU_USER:
       f_diff_time = (double) (cpu_data.usertime - *rsrc_usertime);       
       result_value->val.f_val = (((f_diff_time)/(f_total_time)) * 100);
       break;

   case NCS_SRMSV_RSRC_CPU_KERNEL:
       f_diff_time = (double) (cpu_data.kerneltime - *rsrc_kerneltime);       
       result_value->val.f_val = (((f_diff_time)/(f_total_time)) * 100);
       break;

   default:
       return NCSCC_RC_FAILURE;
   }

   /* Update the resource record with these CPU statistics */
   *rsrc_usertime   = cpu_data.usertime;
   *rsrc_nicertime  = cpu_data.nicertime;
   *rsrc_kerneltime = cpu_data.kerneltime;
   *rsrc_idletime   = cpu_data.idletime;

   if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_THRESHOLD)
   {
      if (rsrc->mon_data.monitor_data.mon_cfg.threshold.samples > 1) 
      {
         /* Get the sample record to update the statistics, only be returned
            if number of sample records are created equivalent to number of
            sample requested for */
         if ((sample_data = srmnd_get_sample_record(rsrc)) == NULL)      
            return NCSCC_RC_FAILURE;
         
         sample_data->when_updated = rsrc->mon_data.recent_updated;
         sample_data->value = *result_value;
      }
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmnd_update_mem_stat
 
  Description   :  This routine calculates the memory statistic data 
                   according to the scale type defined.

  Arguments     :  i_val - memory value to be calculated
                   total - total memory value
                   value - Output result value
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_update_mem_stat(uns32 i_val, uns32 total, NCS_SRMSV_VALUE *value)
{

   switch(value->scale_type)
   {
   case NCS_SRMSV_STAT_SCALE_PERCENT:
      {
         double f_val = (double)i_val;
         double f_total = (double)total;
         double res_val = 0;

         res_val =  ((f_val/f_total)*100);
         value->val.f_val = res_val;
      }
      break;

   case NCS_SRMSV_STAT_SCALE_KBYTE: /* Kilobytes (1024)   */
       value->val.u_val32 = (uns32)(i_val/1024);
       break;

   case NCS_SRMSV_STAT_SCALE_MBYTE: /* Megabytes (1024^2) */
       value->val.u_val32 = (uns32)(i_val/(1024^2));
       break;

   case NCS_SRMSV_STAT_SCALE_GBYTE: /* Gigabytes (1024^3) */
       value->val.u_val32 = (uns32)(i_val/(1024^3));
       break;

   case NCS_SRMSV_STAT_SCALE_TBYTE: /* Terabytes (1024^4) */
       value->val.u_val32 = (uns32)(i_val/(1024^4));
       break;

   case NCS_SRMSV_STAT_SCALE_BYTE:  /* Bytes */
   default:
       value->val.u_val32 = i_val;
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srmnd_get_memory_utilization_stats
 
  Description   :  This routine updates the Memory utilization statistics 
                   (used&free physical memory and swap space) information
                   and updates it in its sample record.

  Arguments     :  rsrc - ptr to the SRMND_RSRC_MON_NODE structure
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  None.
******************************************************************************/
uns32 srmnd_get_memory_utilization_stats(SRMND_RSRC_MON_NODE *rsrc)
{
   SRMND_SAMPLE_DATA  *sample_data;
   NCS_SRMSV_RSRC_TYPE rsrc_type = rsrc->rsrc_type_node->rsrc_type;
   NCS_SRMSV_VALUE *rsrc_stat_value = &rsrc->mon_data.result_value;
   uns32 total_mem = 0;
   uns32 tmp_mem = 0;
   uns32 rc = NCSCC_RC_SUCCESS; 

   /* Get the timestamp at which the stats are collected */ 
   m_GET_TIME_STAMP(rsrc->mon_data.recent_updated);

   if ((rsrc_type == NCS_SRMSV_RSRC_SWAP_SPACE_USED) || 
       (rsrc_type == NCS_SRMSV_RSRC_SWAP_SPACE_FREE))
   {
      /* Get the total swap space of the system */
      rc = m_SRMSV_GET_MEM_STATS(&total_mem,
                                 MEM_STAT_SWAP_TOTAL_LINE_NUM,
                                 MEM_STAT_ITEM_POSITION);
      if (rc != NCSCC_RC_SUCCESS) return rc;
   }
   else
   {
      /* Get the total physical memory of the system */
      rc = m_SRMSV_GET_MEM_STATS(&total_mem,
                                MEM_STAT_TOTAL_LINE_NUM,
                                MEM_STAT_ITEM_POSITION);
      if (rc != NCSCC_RC_SUCCESS) return rc;
   }

   switch(rsrc_type)
   {
   case NCS_SRMSV_RSRC_MEM_PHYSICAL_USED:
       {
          /* Get the free physical memory of the system */
          rc = m_SRMSV_GET_MEM_STATS(&tmp_mem,
                                     MEM_STAT_FREE_LINE_NUM, 
                                     MEM_STAT_ITEM_POSITION);
          if (rc != NCSCC_RC_SUCCESS) return rc;

          srmnd_update_mem_stat((total_mem - tmp_mem), 
                                total_mem, 
                                rsrc_stat_value);
       }
       break;

   case NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE:
       /* Get the free physical memory of the system */
       rc = m_SRMSV_GET_MEM_STATS(&tmp_mem,
                                  MEM_STAT_FREE_LINE_NUM, 
                                  MEM_STAT_ITEM_POSITION);

       srmnd_update_mem_stat(tmp_mem, total_mem, rsrc_stat_value);
       break;

   case NCS_SRMSV_RSRC_SWAP_SPACE_USED:
       /* Get the free swap space of the system */
       rc = m_SRMSV_GET_MEM_STATS(&tmp_mem,
                                  MEM_STAT_SWAP_FREE_LINE_NUM, 
                                  MEM_STAT_ITEM_POSITION);
       if (rc != NCSCC_RC_SUCCESS) return rc;

       srmnd_update_mem_stat((total_mem - tmp_mem),
                             total_mem,
                             rsrc_stat_value);
       break;

   case NCS_SRMSV_RSRC_SWAP_SPACE_FREE:
       /* Get the free swap space of the system */
       rc = m_SRMSV_GET_MEM_STATS(&tmp_mem,
                                  MEM_STAT_SWAP_FREE_LINE_NUM, 
                                  MEM_STAT_ITEM_POSITION);
       if (rc != NCSCC_RC_SUCCESS) return rc;

       srmnd_update_mem_stat(tmp_mem, total_mem, rsrc_stat_value);
       break;

   case NCS_SRMSV_RSRC_USED_BUFFER_MEM:
       /* Get the used buffer space of the system */
       rc = m_SRMSV_GET_MEM_STATS(&tmp_mem,
                               MEM_STAT_BUFF_LINE_NUM, 
                               MEM_STAT_ITEM_POSITION);

       srmnd_update_mem_stat(tmp_mem, total_mem, rsrc_stat_value);
       break;

   default:
       return NCSCC_RC_FAILURE;
       break;
   }
  
 
   if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_THRESHOLD)
   {
      if (rsrc->mon_data.monitor_data.mon_cfg.threshold.samples > 1)
      {
         /* Get the sample record to update the statistics, only be returned
            if number of sample records are created equivalent to number of
            sample requested for */
         if ((sample_data = srmnd_get_sample_record(rsrc)) == NULL)      
            return NCSCC_RC_FAILURE;
         
         sample_data->when_updated = rsrc->mon_data.recent_updated;
         sample_data->value = *rsrc_stat_value;
      }
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmnd_get_cpu_ldavg_stats
 
  Description   :  This routine updates the CPU load avg stats information.                   

  Arguments     :  rsrc - ptr to SRMND_RSRC_MON_NODE structure
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_get_cpu_ldavg_stats(SRMND_RSRC_MON_NODE *rsrc)
{   
   uns16 rsrc_type = rsrc->rsrc_type_node->rsrc_type;
   double *cpu_ldavg = &rsrc->mon_data.info.cpu_ldavg;
   NCS_SRMSV_VALUE *rsrc_stat_value = &rsrc->mon_data.result_value;

   switch (rsrc_type)
   {
   case NCS_SRMSV_RSRC_CPU_UTIL_ONE:
       if (m_SRMSV_GET_CPU_LDAVG_ONE(cpu_ldavg) != NCSCC_RC_SUCCESS)
          return NCSCC_RC_FAILURE;
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL_FIVE:
       if (m_SRMSV_GET_CPU_LDAVG_FIVE(cpu_ldavg) != NCSCC_RC_SUCCESS)
          return NCSCC_RC_FAILURE;
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN:
       if (m_SRMSV_GET_CPU_LDAVG_FIFTEEN(cpu_ldavg) != NCSCC_RC_SUCCESS)
          return NCSCC_RC_FAILURE;
       break;

   default:
          return NCSCC_RC_FAILURE;
       break;
   }
   
   /* Get the timestamp at which the stats are collected */ 
   m_GET_TIME_STAMP(rsrc->mon_data.recent_updated);
  
   /* Update the threshold value */ 
   rsrc_stat_value->val.f_val = *cpu_ldavg;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmnd_update_pid_data
 
  Description   :  This routine updates the descendants information of a PID
                   in its resource monitor record.

  Arguments     :  rsrc - ptr to the resource monitor record                   
                   create_mon - ptr to the SRMA_CREATE_RSRC_MON record
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_update_pid_data(SRMND_RSRC_MON_NODE *rsrc)
{
   uns32  c_pid[SRMSV_MAX_PID_DESCENDANTS];
   uns8   pid_num = 0, check_pid_num = 0;
   uns32  pid = rsrc->rsrc_type_node->pid;
   SRMND_PID_DATA *pid_data = NULL;
   SRMND_PID_DATA *pid_node = NULL, *next_pid_node = NULL;
   NCS_BOOL check_child_pids = TRUE;

   /* No PID to process, so what to do?? ok. just return */
   if (pid == 0)
      return NCSCC_RC_SUCCESS;
   
   /* Clear off the content of input fields */
   memset(&c_pid, 0, sizeof(uns32)*SRMSV_MAX_PID_DESCENDANTS);
   pid_num = 0;

   /* Get the child pid information */
   m_SRMSV_GET_CHILD_PIDS(pid, c_pid, &pid_num);
   if (pid_num == 0)
      return NCSCC_RC_SUCCESS;
   
   /* Update the children data to the rsrc monitor record */
   while (pid_num)
   {
      if ((pid_data = m_MMGR_ALLOC_SRMND_PID) == NULL)
      {
         m_SRMND_LOG_MEM(SRMND_MEM_PID_DATA,
                         SRMND_MEM_ALLOC_FAILED,
                         NCSFL_SEV_CRITICAL);

         return NCSCC_RC_FAILURE;
      }
          
      memset((char *)pid_data, 0, sizeof(SRMND_PID_DATA));

      pid_data->pid = c_pid[--pid_num];
      pid_data->child_level = 1;

      /* Add to desceandants list of rsrc */
      if (rsrc->descendant_pids == NULL)
      {
         rsrc->descendant_pids = pid_data;
         rsrc->pid_num++;
      }
      else
      {
         pid_data->next_pid = rsrc->descendant_pids;
         rsrc->descendant_pids->prev_pid = pid_data;
         rsrc->descendant_pids = pid_data;
         rsrc->pid_num++;
      }
   }
   
   /* Just monitor the children is enough, so get back */
   if (rsrc->rsrc_type_node->child_level == 1)
      return NCSCC_RC_SUCCESS;
   
   while (check_child_pids)
   {
      pid_node = rsrc->descendant_pids;
      check_pid_num = 0;

      while (pid_node)
      {
         next_pid_node = pid_node->next_pid;

         /* If the child pid data is already collected for a PID (or)
            if the level of collecting child info. is enough then CONITNUE */
         if ((pid_node->checked_childs == TRUE) ||
             ((pid_node->child_level == rsrc->rsrc_type_node->child_level)))
         {
            check_pid_num++; 
            pid_node = next_pid_node;
            continue;
         }
         else  /* Need to collect the child data */
         {
            /* Clear off the content of input fields */
            memset(&c_pid, 0, sizeof(uns32)*SRMSV_MAX_PID_DESCENDANTS);
            pid_num = 0;
            pid = pid_node->pid;

            /* Get the child PIDs info */
            m_SRMSV_GET_CHILD_PIDS(pid, c_pid, &pid_num);
       
            /* childrens exists, then update the resource monitor 
               record accordingly */
            if (pid_num != 0)
            {
               while (pid_num)
               {
                  if ((pid_data = m_MMGR_ALLOC_SRMND_PID) == NULL)
                  { 
                     m_SRMND_LOG_MEM(SRMND_MEM_PID_DATA,
                                     SRMND_MEM_ALLOC_FAILED,
                                     NCSFL_SEV_CRITICAL);

                     return NCSCC_RC_FAILURE;
                  }
          
                  memset((char *)pid_data, 0, sizeof(SRMND_PID_DATA));

                  pid_data->pid = c_pid[--pid_num];
                  pid_data->child_level = (pid_node->child_level + 1);
                  pid_data->parent = pid_node;

                  /* Add to descendants list of rsrc */
                  pid_data->next_pid = rsrc->descendant_pids;
                  rsrc->descendant_pids->prev_pid = pid_data;
                  rsrc->descendant_pids = pid_data;
                  rsrc->pid_num++;

                  if (pid_node->child_list == NULL) 
                  {
                     pid_node->child_list = pid_data;
                  }
                  else
                  {
                     pid_data->next_child_pid = pid_node->child_list;
                     pid_node->child_list->prev_child_pid = pid_data;
                     pid_node->child_list = pid_data;
                  }
               }
            }

            pid_node->checked_childs = TRUE;
         }
      }

      /* All the pids are checked */
      if (rsrc->pid_num == check_pid_num)
         check_child_pids = FALSE;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmnd_get_proc_memory_util_stats
 
  Description   :  This routine updates the process Memory utilization stats 
                   (used physical memory by the process) information and
                   updates it in its sample record.

  Arguments     :  rsrc - ptr to the SRMND_RSRC_MON_NODE structure
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  None.
******************************************************************************/
uns32 srmnd_get_proc_memory_util_stats(SRMND_RSRC_MON_NODE *rsrc)
{
   SRMND_SAMPLE_DATA  *sample_data;
   NCS_SRMSV_VALUE *rsrc_stat_value = &rsrc->mon_data.result_value;
   uns32 pid = rsrc->rsrc_type_node->pid;
   uns32 total_mem = 0;
   uns32 tmp_mem = 0;
   uns32 rc = NCSCC_RC_SUCCESS; 

   /* Get the total physical memory of the system */
   rc = m_SRMSV_GET_MEM_STATS(&total_mem,
                              MEM_STAT_TOTAL_LINE_NUM,
                              MEM_STAT_ITEM_POSITION);
   if (rc != NCSCC_RC_SUCCESS) return rc;

   /* Get the memory val consumed by the process */
   rc = m_SRMSV_GET_PROC_MEM_STATS(pid,
                                   &tmp_mem,
                                   MEM_STAT_PROC_UTIL_LINE_NUM, 
                                   MEM_STAT_PROC_ITEM_POS);
   if (rc != NCSCC_RC_SUCCESS) return rc;

   srmnd_update_mem_stat(tmp_mem, total_mem, rsrc_stat_value); 

   if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_THRESHOLD)
   {
      if (rsrc->mon_data.monitor_data.mon_cfg.threshold.samples > 1)
      {
         /* Get the sample record to update the statistics, only be returned
            if number of sample records are created equivalent to number of
            sample requested for */
         if ((sample_data = srmnd_get_sample_record(rsrc)) == NULL)      
            return NCSCC_RC_FAILURE;
         
         sample_data->when_updated = rsrc->mon_data.recent_updated;
         sample_data->value = *rsrc_stat_value;
      }
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_get_proc_cpu_util_stats
 
  Description   :  This routine updates the process CPU utilization stats 
                   information and updates it in its sample record.

  Arguments     :  rsrc - ptr to the SRMND_RSRC_MON_NODE structure
                   
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  None.
******************************************************************************/
uns32 srmnd_get_proc_cpu_util_stats(SRMND_RSRC_MON_NODE *rsrc)
{
   uns32 rc = NCSCC_RC_SUCCESS; 
   uns32 cpu_jiffies = 0;
   uns32 new_jiffies = 0;
   uns32 diff_time = 0;
   uns32 old_jiffies = rsrc->mon_data.info.proc_cpu_jiffies;
   long long old_time_ms = rsrc->mon_data.time_ms;

 
   /* Get the CPU utilized by the process */
   rc = m_SRMSV_GET_PROC_CPU_STATS(rsrc->rsrc_type_node->pid,
                                   &new_jiffies);
   if (rc != NCSCC_RC_SUCCESS) return rc;

   rsrc->mon_data.time_ms = m_NCS_OS_GET_TIME_MS;
   m_GET_TIME_STAMP(rsrc->mon_data.recent_updated);
   rsrc->mon_data.info.proc_cpu_jiffies = new_jiffies;

   diff_time = (uns32)(rsrc->mon_data.time_ms - old_time_ms);
   cpu_jiffies = (new_jiffies - old_jiffies);

   /* First iteration check?? so please return */
   if (old_jiffies == 0)
      return NCSCC_RC_CONTINUE;

   /* No changein CPU util jiffies??  so please return */ 
   if (cpu_jiffies == 0)
      return rc;

   /* Doing it on milli-sec's so multiple it by 1000 */
   rsrc->mon_data.result_value.val.f_val = (double)((double)cpu_jiffies/diff_time * 1000);

   if (rsrc->mon_data.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_THRESHOLD)
   {
      if (rsrc->mon_data.monitor_data.mon_cfg.threshold.samples > 1)
      {
         SRMND_SAMPLE_DATA  *sample_data = NULL;

         /* Get the sample record to update the statistics, only be returned
            if number of sample records are created equivalent to number of
            sample requested for */
         if ((sample_data = srmnd_get_sample_record(rsrc)) == NULL)      
            return NCSCC_RC_FAILURE;
         
         sample_data->when_updated = rsrc->mon_data.recent_updated;
         sample_data->value = rsrc->mon_data.result_value;
      }
   }

   return rc;
}



