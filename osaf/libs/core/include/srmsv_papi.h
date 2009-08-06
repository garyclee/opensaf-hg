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

  MODULE NAME: SRMSV_PAPI.H

..............................................................................

  DESCRIPTION: This file contains the enum definitions, data structs used 
               across SRMSv service by SRMA & SRMND. These definitions are
               published to the external world in order to interface with
               SRMSv service.

******************************************************************************/

#ifndef SRMSV_PAPI_H
#define SRMSV_PAPI_H

#include "ncsgl_defs.h"

#define SRMSV_MAX_UNS32_VAL (4294967295 - 5)

/* Different types of RESOURCES */
typedef enum
{
    /* CPU specific resources */
   NCS_SRMSV_RSRC_CPU_UTIL = 1,   /* CPU total utilization */
   NCS_SRMSV_RSRC_CPU_KERNEL,     /* CPU Kernel utilization */
   NCS_SRMSV_RSRC_CPU_USER,       /* CPU User utilization */ 
   NCS_SRMSV_RSRC_CPU_UTIL_ONE,   /* CPU utilization of the last minute */
   NCS_SRMSV_RSRC_CPU_UTIL_FIVE,  /* CPU utilization of the last 5 minute */
   NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN, /* CPU utilization of the last 15 minute */

   /* Memory Specific Resources */
   NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,   
   NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE,

   NCS_SRMSV_RSRC_SWAP_SPACE_USED,
   NCS_SRMSV_RSRC_SWAP_SPACE_FREE,

   NCS_SRMSV_RSRC_USED_BUFFER_MEM,
   NCS_SRMSV_RSRC_USED_CACHED_MEM,
        
   /* Process specific resource */
   NCS_SRMSV_RSRC_PROC_EXIT,  /* Monitor the process, for exit */
   NCS_SRMSV_RSRC_PROC_MEM,   /* Monitor the memory consumed by the process */
   NCS_SRMSV_RSRC_PROC_CPU,   /* Monitor the CPU utilization of the process */
   NCS_SRMSV_RSRC_END
} NCS_SRMSV_RSRC_TYPE;


/* Enum defintions for SRMND locations */
typedef enum  
{
   NCS_SRMSV_NODE_LOCAL = 1,   
   NCS_SRMSV_NODE_REMOTE,
   NCS_SRMSV_NODE_ALL,         
   NCS_SRMSV_NODE_END
} NCS_SRMSV_NODE_LOCATION;


/* Different types threshold CONDITIONS */
typedef enum
{
   NCS_SRMSV_THRESHOLD_VAL_IS_AT = 1,
   NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT,
   NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE,
   NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,
   NCS_SRMSV_THRESHOLD_VAL_IS_BELOW,
   NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,
   NCS_SRMSV_THRESHOLD_VAL_IS_END
} NCS_SRMSV_THRESHOLD_TEST_CONDITION;

/* VALUE types */
typedef enum
{
   NCS_SRMSV_VAL_TYPE_FLOAT = 1,
   NCS_SRMSV_VAL_TYPE_INT32,
   NCS_SRMSV_VAL_TYPE_UNS32,
   NCS_SRMSV_VAL_TYPE_INT64,
   NCS_SRMSV_VAL_TYPE_UNS64,
   NCS_SRMSV_VAL_TYPE_END
} NCS_SRMSV_VAL_TYPE;

/* Different SCALE types */
typedef enum
{
   NCS_SRMSV_STAT_SCALE_BYTE = 1, /* Bytes */
   NCS_SRMSV_STAT_SCALE_KBYTE,    /* Kilobytes (1024)   */
   NCS_SRMSV_STAT_SCALE_MBYTE,    /* Megabytes (1024^2) */
   NCS_SRMSV_STAT_SCALE_GBYTE,    /* Gigabytes (1024^3) */
   NCS_SRMSV_STAT_SCALE_TBYTE,    /* Terabytes (1024^4) */
   NCS_SRMSV_STAT_SCALE_PERCENT,  /* Value in % */
   NCS_SRMSV_STAT_SCALE_END       /* enum end */
} NCS_SRMSV_STAT_SCALE_TYPE;


/* VALUE structure */
typedef struct ncs_srmsv_value
{
   NCS_SRMSV_VAL_TYPE         val_type; /* Default (0) it will be considered as int32 */
   /* States the scale type of this value */
   NCS_SRMSV_STAT_SCALE_TYPE  scale_type;
   union
   {
       double f_val;
       int32  i_val32;
       uns32  u_val32;
       int64  i_val64;
       uns64  u_val64;
   } val;
} NCS_SRMSV_VALUE;


/* different monitor types supported */
typedef enum
{
   NCS_SRMSV_MON_TYPE_THRESHOLD = 1,   /* generates an event when a level is exceeded. */
   NCS_SRMSV_MON_TYPE_WATERMARK,       /* records the highest and/or lowest value observed. */
   NCS_SRMSV_MON_TYPE_LEAKY_BUCKET,    /* generates an event when a counter exceeds a rate of increase. */
   NCS_SRMSV_MON_TYPE_END
} NCS_SRMSV_MONITOR_TYPE;


/* Types of WATERMARK */
typedef enum
{
   NCS_SRMSV_WATERMARK_HIGH = 1,  /* records the highest observed value over the monitoring interval. */
   NCS_SRMSV_WATERMARK_LOW,       /* records the lowest observed value over the monitoring interval. */
   NCS_SRMSV_WATERMARK_DUAL,      /* records both the highest and lowest observed values over the monitoring interval. */
   NCS_SRMSV_WATERMARK_END 
} NCS_SRMSV_WATERMARK_TYPE;


/* USER types */
typedef enum
{
   NCS_SRMSV_USER_REQUESTOR = 1,
   NCS_SRMSV_USER_SUBSCRIBER,
   NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,
   NCS_SRMSV_USER_END
} NCS_SRMSV_USER_TYPE;


/* Threshold configuration data structure used while configuring 
 * resource monitor data.
 */
typedef struct ncs_srmsv_threshold_cfg
{
   /* Number of samples to be consider (to calculate on an avg)
      on a particular resource statistics */
   uns32  samples;

   /* Acceptable threshold value */
   NCS_SRMSV_VALUE  threshold_val;
    
   NCS_SRMSV_THRESHOLD_TEST_CONDITION  condition;

   /* Used only when the condition is EQUAL or NOTEQUAL to */
   NCS_SRMSV_VALUE  tolerable_val; 
    
} NCS_SRMSV_THRESHOLD_CFG;


/* Structure for LEAKY BUCKET */
typedef struct ncs_srmsv_leaky_bucket_cfg
{   
   NCS_SRMSV_VALUE  bucket_size; /* Max bucket size */
   NCS_SRMSV_VALUE  fill_size;   /* Periodic fill value */
} NCS_SRMSV_LEAKY_BUCKET_CFG;


typedef struct srmsv_watermark_info
{
   NCS_BOOL wm_exist;
   uns32 pid;                      
   NCS_SRMSV_RSRC_TYPE rsrc_type;
   NCS_SRMSV_WATERMARK_TYPE wm_type;
   NCS_SRMSV_VALUE low_val;
   NCS_SRMSV_VALUE high_val;
} SRMSV_WATERMARK_VAL;

typedef struct ncs_srmsv_mon_data
{   
   time_t  monitoring_rate; /* sec's bet'n each sample taken by the monitor: 
                             60 = 1 sample/minute, etc.a monitoringRate can not
                             be set to zero */
   time_t  monitoring_interval;  /* Period of time at which the resource will
                                     be monitored */
    
   NCS_SRMSV_MONITOR_TYPE monitor_type;

   union 
   {
       NCS_SRMSV_THRESHOLD_CFG    threshold;
       NCS_SRMSV_WATERMARK_TYPE   watermark_type;
       NCS_SRMSV_LEAKY_BUCKET_CFG leaky_bucket;
   } mon_cfg;
    
   uns32 evt_severity; /* Used for logging while the requested threshold 
                           test condition is met */
} NCS_SRMSV_MON_DATA;

#define  NCS_SRMSV_MON_DATA_NULL  ((NCS_SRMSV_MON_DATA*)0)


/* Resource Statistic info to identify the statistic of a resource */
typedef struct ncs_srmsv_rsrc_info
{    
   NCS_SRMSV_RSRC_TYPE  rsrc_type;
   NCS_SRMSV_USER_TYPE  usr_type;     /* Either REQUESTOR / SUBSCR / 
                                         REQUESTOR + SUBSCR */
   uns32  pid;                        /* Process ID, updated only in case of  
                                         rsrc to be monitored is a process */
   int    child_level;                /* Depth level of the childs (of above
                                         pid) to be monitored */
   NCS_SRMSV_NODE_LOCATION srmsv_loc; /* Remote or for ALL nodes, Default is
                                         LOCAL */
   uns32  node_num;                 /* Can be set if srmsv location is
                                         REMOTE */
} NCS_SRMSV_RSRC_INFO;

#define  NCS_SRMSV_RSRC_INFO_NULL  ((NCS_SRMSV_RSRC_INFO*)0)


/* This structure consists of the complete information of Resource Monitor Data
 */
typedef struct ncs_srmsv_mon_info
{
   NCS_SRMSV_RSRC_INFO rsrc_info;    /* Monitored resource info */
   NCS_SRMSV_MON_DATA  monitor_data; /* Monitor config & control data, not
                                        required to if the user type is SUBSCR */
} NCS_SRMSV_MON_INFO;

#define  NCS_SRMSV_MON_INFO_NULL  ((NCS_SRMSV_MON_INFO*)0)

#endif  /* SRMSV_PAPI_H */


