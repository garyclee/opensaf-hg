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

  MODULE NAME: SRMST_STAT.C
 
..............................................................................

  DESCRIPTION: This file contains the routines that are used to access/collect
               the resource statistics data.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srmsv.h"
#include "sys/sysinfo.h"


/* Static Function Proto-Types */
static uns32 srmst_proc_get_cpu_ldavg(double *cpu_ldavg,
                                      NCS_SRMSV_RSRC_TYPE rsrc_type);
static NCS_BOOL srmst_proc_check_thread(uns32 pid, NCS_BOOL *is_thread);

/****************************************************************************
  Name          :  srmst_proc_get_mem_stats
 
  Description   :  This routine gets the Memory specific statistics from proc
                   file system.

  Arguments     :  mem_val - return memory value
                   line_num - line number to read
                   item_loc - memory item location in the read line 
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmst_proc_get_mem_stats(uns32 *mem_val, uns8 line_num, uns8 item_loc)
{
   char line_buffer [SRMST_MAX_BUF_SIZE];
   char item_buffer [SRMST_ITEM_BUF_SIZE];

   *mem_val = 0;

   /* Get a line from the file */
   memset(line_buffer, 0, SRMST_MAX_BUF_SIZE);
   if (srmst_file_parser("/proc/meminfo", line_buffer, line_num) == 1)
   {
      memset(item_buffer , 0, SRMST_ITEM_BUF_SIZE);
      /* Get a mem item from the file */
      if (srmst_str_parser(line_buffer,
                           MEM_STAT_SPLIT_PATTERN,
                           item_buffer,
                           item_loc) == 1)
      {
         *mem_val = (atol(item_buffer) * 1024);

         return NCSCC_RC_SUCCESS;
      }
   }

   return NCSCC_RC_FAILURE;
}


/****************************************************************************
  Name          :  srmst_proc_get_pid_mem_stats
 
  Description   :  This routine gets the Memory specific statistics from proc
                   file system.

  Arguments     :  pid - process ID
                   mem_val - return memory value
                   line_num - line number to read
                   item_loc - memory item location in the read line 
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmst_proc_get_pid_mem_stats(uns32 pid,
                                   uns32 *mem_val,
                                   uns8 line_num,
                                   uns8 item_loc)
{
   char  line_buffer [SRMST_MAX_BUF_SIZE];
   char  item_buffer [SRMST_ITEM_BUF_SIZE];
   char  pid_path[SRMST_DIR_PATH_LEN];

   memset(&pid_path, 0, SRMST_DIR_PATH_LEN);
   sprintf(pid_path, "/proc/%d/status", pid);  

   /* Get a line from the file */
   memset(line_buffer, 0, SRMST_MAX_BUF_SIZE);
   if (srmst_file_parser(pid_path, line_buffer, line_num) == 1)
   {
      memset(item_buffer, 0, SRMST_ITEM_BUF_SIZE);
      /* Get the virtual memsize of a process */
      if (srmst_str_parser(line_buffer,
                           " ",
                           item_buffer,
                           item_loc) == 1)
      {
         *mem_val = (atol(item_buffer) * 1024);
         return NCSCC_RC_SUCCESS;
      }
   }

   return NCSCC_RC_FAILURE;
}


/****************************************************************************
  Name          :  srmst_proc_get_cpu_ldavg
                                                                                
  Description   :  This routine gets the CPU specific load avgs.
                                                                                
  Arguments     :  cpu_ldavg - returns CPU ldavg value
                   rsrc_type - Type of the resource (CPU specific)
                                                                                
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
                                                                                
  Notes         :  None.
******************************************************************************/
uns32 srmst_proc_get_cpu_ldavg(double *cpu_ldavg,
                               NCS_SRMSV_RSRC_TYPE rsrc_type)
{
   FILE *fp;
   float avg1, avg2, avg3;
   int rc;
                                                                                
   /* Open the file in read mode */
   if ((fp = fopen("/proc/loadavg", "r")) == NULL)
   {
      /* LOG appropriate message */
      return NCSCC_RC_FAILURE;
   }

   rc = fscanf(fp, "%f %f %f", &avg1, &avg2, &avg3);
   fclose(fp);
                                                                                
   switch(rsrc_type)
   {
   case NCS_SRMSV_RSRC_CPU_UTIL_ONE:
       *cpu_ldavg = avg1;
       break;
                                                                                
   case NCS_SRMSV_RSRC_CPU_UTIL_FIVE:
       *cpu_ldavg = avg2;
       break;
                                                                                
   case NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN:
       *cpu_ldavg = avg3;
       break;
                                                                                
   default:
       /* LOG appropriate message */
       break;
   }
                                                                                
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmst_proc_get_cpu_ldavg_one
 
  Description   :  This routine gets the CPU specific load avg for 1 min.

  Arguments     :  cpu_ldavg - gets CPU 1 min ldavg
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmst_proc_get_cpu_ldavg_one(double *cpu_ldavg)
{
   srmst_proc_get_cpu_ldavg(cpu_ldavg, NCS_SRMSV_RSRC_CPU_UTIL_ONE);
   return NCSCC_RC_SUCCESS; 
}


/****************************************************************************
  Name          :  srmst_proc_get_cpu_ldavg_five
 
  Description   :  This routine gets the CPU specific load avg for 5 min.

  Arguments     :  cpu_ldavg - gets CPU 5 min ldavg
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmst_proc_get_cpu_ldavg_five(double *cpu_ldavg)
{ 
   srmst_proc_get_cpu_ldavg(cpu_ldavg, NCS_SRMSV_RSRC_CPU_UTIL_FIVE);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmst_proc_get_cpu_ldavg_fifteen
 
  Description   :  This routine gets the CPU specific load avg for 15 min.

  Arguments     :  cpu_ldavg - gets CPU 15 min ldavg 
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmst_proc_get_cpu_ldavg_fifteen(double *cpu_ldavg)
{
   srmst_proc_get_cpu_ldavg(cpu_ldavg, NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN);
   return NCSCC_RC_SUCCESS; 
}


/****************************************************************************
  Name          :  srmst_proc_get_cpu_util_stats
 
  Description   :  This routine gets the CPU specific statistics from the 
                   /proc file system.

  Arguments     :  cpu_stats - ptr to the SRMND_CPU_DATA structure
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmst_proc_get_cpu_util_stats(SRMND_CPU_DATA *cpu_stats)
{
   char line_buffer [SRMST_MAX_BUF_SIZE]; 
   char item_buffer [SRMST_ITEM_BUF_SIZE];

   /* Get a line from the file */
   if (srmst_file_parser(CPU_STAT_FILE, line_buffer, CPU_STAT_LINE_NUM)
      != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;      

   /* Get the CPU user util time */
   if (srmst_str_parser(line_buffer, 
                        CPU_STAT_SPLIT_PATTERN,
                        item_buffer,
                        CPU_STAT_USER_POSITION) != NCSCC_RC_SUCCESS)
   {
      /* LOG the appropriate message */
      return NCSCC_RC_FAILURE;
   }
   else
      cpu_stats->usertime = atol(item_buffer);

   /* Get the CPU nicer util time */
   if (srmst_str_parser(line_buffer,
                        CPU_STAT_SPLIT_PATTERN,
                        item_buffer,
                        CPU_STAT_NICER_POSITION) != NCSCC_RC_SUCCESS)
   {
      /* LOG the appropriate message */
      return NCSCC_RC_FAILURE;
   }
   else
      cpu_stats->nicertime = atol(item_buffer);

   /* Get the CPU kernel util time */
   if (srmst_str_parser(line_buffer,
                        CPU_STAT_SPLIT_PATTERN,
                        item_buffer,
                        CPU_STAT_KERNEL_POSITION) != NCSCC_RC_SUCCESS)
   {
      /* LOG the appropriate message */
      return NCSCC_RC_FAILURE;
   }
   else
      cpu_stats->kerneltime = atol(item_buffer);

   /* Get the CPU idle time */
   if (srmst_str_parser(line_buffer,
                        CPU_STAT_SPLIT_PATTERN,
                        item_buffer,
                        CPU_STAT_IDLE_POSITION) != NCSCC_RC_SUCCESS)
   {
      /* LOG the appropriate message */
      return NCSCC_RC_FAILURE;
   }
   else
      cpu_stats->idletime = atol(item_buffer);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmst_proc_get_child_pids
 
  Description   :  This routine gets all the child PIDs of a given process
                   (PID)

  Arguments     :  i_pid - PID of the process
                   c_pid - holds all the child PIDs
                   pid_num - Number of child PIDs
 
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmst_proc_get_child_pids(uns32 i_pid, uns32 *c_pid, uns8 *pid_num)
{
   DIR    *dir;
   struct dirent *dit;
   char   dir_path[SRMST_DIR_PATH_LEN];
   uns32  pid = 0, p_pid = 0;
   uns8   pid_count = 0;
   char line_buffer[SRMST_MAX_BUF_SIZE];
   char item_buffer[SRMST_ITEM_BUF_SIZE];

   if ((dir = opendir("/proc/")) != NULL)
   {
      while ((dit = readdir(dir)) != NULL) 
      {
          pid = atol(dit->d_name);
          if (pid == 0) continue;

          memset(&dir_path, 0, SRMST_DIR_PATH_LEN);
          sprintf(dir_path, "/proc/%d", pid);
          strcat(dir_path, "/stat");
 
          memset(line_buffer, 0, SRMST_MAX_BUF_SIZE);
          memset(item_buffer, 0, SRMST_ITEM_BUF_SIZE);

          /* get a line from the file */
          if (srmst_file_parser(dir_path, line_buffer, PID_STAT_LINE_NUM) != NCSCC_RC_SUCCESS)
             return;  
    
          /* get the data from the line */
          if (srmst_str_parser(line_buffer,
                               PID_STAT_SPLIT_PATTERN,
                               item_buffer,
                               PID_STAT_PARENT_POSITION) != NCSCC_RC_SUCCESS)
          {
             /* LOG appropriate message */
             return;
          }

          p_pid = atol(item_buffer);
          if (i_pid == p_pid)
             c_pid[pid_count++] = pid;
           
          /* Exceeding the max descendants range supported */
          if (pid_count > SRMSV_MAX_PID_DESCENDANTS)
          {
             /* LOG the warning message */
             break;
          }
       }
       closedir(dir);
   }
   
   *pid_num = pid_count;

   return;
}



/****************************************************************************
  Name          :  srmst_proc_check_process_active
 
  Description   :  This routine check whether the PID is a THREAD type

  Arguments     :  rsrc_type: Type of the resource to be monitor
                   value : (Return) Statistic Value of the requested resource
 
  Return Values :  TRUE (or) FALSE
 
  Notes         :  For THREADS statm (process usage memory) contains 0 values.
******************************************************************************/
static NCS_BOOL srmst_proc_check_thread(uns32 pid, NCS_BOOL *is_thread)
{
   char  line_buffer [SRMST_MAX_BUF_SIZE];
   char  item_buffer [SRMST_ITEM_BUF_SIZE];
   char  pid_path[SRMST_DIR_PATH_LEN];
   uns32 mem_val = 0;

   memset(&pid_path, 0, SRMST_DIR_PATH_LEN);
   sprintf(pid_path, "/proc/%d/statm", pid);  

   /* Get a line from the file */
   memset(line_buffer, 0, SRMST_MAX_BUF_SIZE);
   if (srmst_file_parser(pid_path, line_buffer, 1) == 1)
   {
      memset(item_buffer, 0, SRMST_ITEM_BUF_SIZE);
      /* Get the virtual memsize of a process */
      if (srmst_str_parser(line_buffer,
                           " ",
                           item_buffer,
                           0) == 1)
      {
         mem_val = atol(item_buffer);

         /* mem_val will be 0 for thread PID */
         if (mem_val == 0)
         {
            *is_thread = TRUE;
            return TRUE;
         }
      }
   }

   return FALSE;
}


/****************************************************************************
  Name          :  srmst_proc_check_process_active
 
  Description   :  This routine check whether the process is active or not

  Arguments     :  pid : PID to monitor
 
  Return Values :  TRUE (or) FALSE
 
  Notes         :  None.
******************************************************************************/
NCS_BOOL srmst_proc_check_process_active(uns32 pid, NCS_BOOL *is_thread)
{
   DIR   *dir;
   char  pid_path[SRMST_DIR_PATH_LEN];

   memset(&pid_path, 0, SRMST_DIR_PATH_LEN);

   /* Always initialise it with FALSE */
   *is_thread = FALSE;

   sprintf(pid_path, "/proc/%d", pid);
   /* Check whether the process directory exists in the proc file system */
   if ((dir = opendir(pid_path)) == NULL)
      return FALSE;
   else
   {  int fd = 0;

      closedir(dir);
      memset(&pid_path, 0, SRMST_DIR_PATH_LEN);
      sprintf(pid_path, "/proc/%d/exe", pid);     
      if ((fd = open(pid_path, O_RDONLY)) < 0)
         return srmst_proc_check_thread(pid, is_thread);
      else
         close(fd);
   }

   return TRUE;
}


/****************************************************************************
  Name          :  srmst_proc_get_pid_cpu_stats
 
  Description   :  This routine gets the CPU utilization statistics of a 
                   process from proc file system.

  Arguments     :  pid - process ID
                   cpu_jiffies - total number of CPU jiffies allotted.
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmst_proc_get_pid_cpu_stats(uns32 pid,
                                   uns32 *cpu_jiffies)
{
   char  line_buffer [SRMST_MAX_BUF_SIZE];
   char  item_buffer [SRMST_ITEM_BUF_SIZE];
   char  pid_path[SRMST_DIR_PATH_LEN];
   uns32 user_jiffies = 0;
   uns32 sys_jiffies = 0;

 
   *cpu_jiffies = 0;

   memset(&pid_path, 0, SRMST_DIR_PATH_LEN);
   sprintf(pid_path, "/proc/%d/stat", pid);  

   /* Get a line from the file */
   memset(line_buffer, 0, SRMST_MAX_BUF_SIZE);
   if (srmst_file_parser(pid_path, line_buffer, CPU_STAT_PROC_UTIL_LINE_NUM) != 1)
      return NCSCC_RC_FAILURE;
   
   memset(item_buffer, 0, SRMST_ITEM_BUF_SIZE);
   /* Get the virtual memsize of a process */
   if (srmst_str_parser(line_buffer,
                        " ",
                        item_buffer,
                        CPU_STAT_PROC_USER_ITEM_POS) == 1)
   {
      user_jiffies = atol(item_buffer);
   }
   else
      return NCSCC_RC_FAILURE;

   memset(item_buffer, 0, SRMST_ITEM_BUF_SIZE);
   /* Get the virtual memsize of a process */
   if (srmst_str_parser(line_buffer,
                        " ",
                        item_buffer,
                        CPU_STAT_PROC_SYS_ITEM_POS) == 1)
   {
      sys_jiffies = atol(item_buffer);
   }
   else
      return NCSCC_RC_FAILURE;
  
   *cpu_jiffies = (user_jiffies + sys_jiffies); 

   return NCSCC_RC_SUCCESS;
}

