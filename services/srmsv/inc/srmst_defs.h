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

  MODULE NAME: SRMST_PROC.H

..............................................................................

  DESCRIPTION: This file comtains the MACRO definitions and enum definitions
               defined for SRMST, used by SRMND.

******************************************************************************/

#ifndef SRMST_PROC_H
#define SRMST_PROC_H

#define SRMST_DIR_PATH_LEN  128
#define SRMST_MAX_BUF_SIZE  1024
#define SRMST_ITEM_BUF_SIZE 128

/* PID specific macros */
#define  PID_STAT_LINE_NUM          1
#define  PID_STAT_SPLIT_PATTERN     " "
#define  PID_STAT_PARENT_POSITION   3


/* CPU specific macros */
#define  CPU_STAT_FILE              "/proc/stat"
#define  CPU_STAT_LINE_NUM          1
#define  CPU_STAT_SPLIT_PATTERN     " "
#define  CPU_STAT_USER_POSITION     1
#define  CPU_STAT_NICER_POSITION    2
#define  CPU_STAT_KERNEL_POSITION   3
#define  CPU_STAT_IDLE_POSITION     4

/* Memory specific macros */
#define MEM_STAT_FILE              "/proc/meminfo"
#define MEM_STAT_SPLIT_PATTERN     " "
#define MEM_STAT_ITEM_POSITION     1
#define MEM_STAT_TOTAL_LINE_NUM    1
#define MEM_STAT_FREE_LINE_NUM     2
#define MEM_STAT_BUFF_LINE_NUM     3
#define MEM_STAT_SWAP_TOTAL_LINE_NUM  12 
#define MEM_STAT_SWAP_FREE_LINE_NUM   13


#define  MEM_STAT_PROC_ITEM_POS      1
#define  MEM_STAT_PROC_UTIL_LINE_NUM 14

#define  CPU_STAT_PROC_UTIL_LINE_NUM  1
#define  CPU_STAT_PROC_USER_ITEM_POS  13
#define  CPU_STAT_PROC_SYS_ITEM_POS   14

#define  m_SRMSV_GET_CHILD_PIDS        srmst_proc_get_child_pids
#define  m_SRMSV_GET_MEM_STATS         srmst_proc_get_mem_stats
#define  m_SRMSV_GET_CPU_UTIL_STATS    srmst_proc_get_cpu_util_stats
#define  m_SRMSV_CHECK_PROCESS_EXIST   srmst_proc_check_process_active
#define  m_SRMSV_GET_CPU_LDAVG_ONE     srmst_proc_get_cpu_ldavg_one
#define  m_SRMSV_GET_CPU_LDAVG_FIVE    srmst_proc_get_cpu_ldavg_five
#define  m_SRMSV_GET_CPU_LDAVG_FIFTEEN srmst_proc_get_cpu_ldavg_fifteen
#define  m_SRMSV_GET_PROC_MEM_STATS    srmst_proc_get_pid_mem_stats
#define  m_SRMSV_GET_PROC_CPU_STATS    srmst_proc_get_pid_cpu_stats

/****************************************************************************
            Function Prototype Definitions
****************************************************************************/
/* Below given routines are much specific to Linux /proc file system */
EXTERN_C NCS_BOOL srmst_proc_check_process_active(uns32 pid, NCS_BOOL *is_thread);
EXTERN_C uns32 srmst_proc_get_mem_stats(uns32 *mem_val, 
                                        uns8 line_num, 
                                        uns8 item_loc);
EXTERN_C uns32    srmst_proc_get_cpu_util_stats(SRMND_CPU_DATA *cpu_stats);
EXTERN_C void     srmst_proc_get_child_pids(uns32 i_pid,
                                            uns32 *c_pid,
                                            uns8 *pid_num);

/* Generic routines used to parse the input file for certain item */
EXTERN_C uns32 srmst_file_parser(char *in_file, char *out_string, uns16 line);
EXTERN_C uns32 srmst_str_parser(char *in_string,
                                char *in_split_pattern,
                                char *out_string,
                                uns16 num);
EXTERN_C uns32 srmst_get_data_from_file(char *in_file,
                                        uns16 line_num,
                                        uns16 position,
                                        char *in_split_pattern,
                                        uns32 *result);
EXTERN_C uns32 srmst_proc_get_cpu_ldavg_one(double *cpu_ldavg);
EXTERN_C uns32 srmst_proc_get_cpu_ldavg_five(double *cpu_ldavg);
EXTERN_C uns32 srmst_proc_get_cpu_ldavg_fifteen(double *cpu_ldavg);
EXTERN_C uns32 srmst_proc_get_pid_mem_stats(uns32 pid,
                                            uns32 *mem_val,
                                            uns8 line_num,
                                            uns8 item_loc);
EXTERN_C uns32 srmst_proc_get_pid_cpu_stats(uns32 pid,
                                            uns32 *cpu_jiffies);
#endif /* SRMST_PROC_H */


