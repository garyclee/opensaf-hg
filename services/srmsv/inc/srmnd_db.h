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

  MODULE NAME: SRMND_DB.H

..............................................................................

  DESCRIPTION: This file has the data structure definitions defined for SRMND 
               database. It also contains the related function prototype 
               definitions.

******************************************************************************/

#ifndef SRMND_DB_H
#define SRMND_DB_H

#define RSRC_APPL_NOTIF_SENT        0x01
#define RSRC_APPL_SUBS_SENT_PENDING 0x02

struct srmnd_rsrc_mon_node;

typedef enum
{
   SRMSV_MONITOR_STATE_UP = 1,
   SRMSV_MONITOR_STATE_DOWN,
   SRMSV_MONITOR_STATE_END
} SRMND_MONITOR_STATE;

typedef struct srmnd_srma_usr_key
{
   /* Both srma_usr_hdl + srma_dest represents as the application key */
   uns32    srma_usr_hdl; /* User application handle created at SRMA */
   MDS_DEST srma_dest;    /* SRMA Adest Info */
}SRMND_SRMA_USR_KEY;

/*************************************************************************
 * SRMND maintains a list of SRMA records, each SRMA record will maintain 
 * a list of its cfg'ed monitor resource structs.
 *************************************************************************/
typedef struct srmnd_mon_srma_usr_node
{
   SRMND_SRMA_USR_KEY usr_key;

   /* Control parameter, whether to add the appl rsrc's for 
      monitoring */  
   NCS_BOOL  stop_monitoring;
 
   /* Starting ptr of a resource mon node in the list */
   struct srmnd_rsrc_mon_node *start_rsrc_mon_node;

   /* Prev & Next SRMA ptr's of the SRMA list */
   struct srmnd_mon_srma_usr_node *prev_srma_node;
   struct srmnd_mon_srma_usr_node *next_srma_node;
} SRMND_MON_SRMA_USR_NODE;

#define  SRMND_MON_SRMA_USR_NODE_NULL  ((SRMND_MON_SRMA_USR_NODE*)0)

/*************************************************************************
 * For each resource type SRMND maintains a list of configuration types. 
 * This information is useful when a subscriber is added on fly to monitor
 * a particular resource.
 *************************************************************************/
typedef struct srmnd_rsrc_type_node
{
   /* Resource info. rsrc_type + pid will be the key of rsrc type */
   NCS_SRMSV_RSRC_TYPE  rsrc_type;
   uns32 pid;
   int   child_level;

   /* Active rsrc rec to get watermark levels */
   struct srmnd_rsrc_mon_node *watermark_rsrc;

   /* Monitor information with respect to each resource */
   struct srmnd_rsrc_mon_node *start_rsrc_mon_list;

   /* Subscriber information with respect to each resource */
   struct srmnd_rsrc_mon_node *start_rsrc_subs_list;

   /* Prev & Next resource ptrs of the resource list */
   struct srmnd_rsrc_type_node *prev_rsrc_type;
   struct srmnd_rsrc_type_node *next_rsrc_type;

} SRMND_RSRC_TYPE_NODE;

#define  SRMND_RSRC_TYPE_NODE_NULL  ((SRMND_RSRC_TYPE_NODE*)0)

typedef struct srmnd_cpu_data
{
   uns32 usertime;
   uns32 nicertime;
   uns32 kerneltime;
   uns32 idletime;
} SRMND_CPU_DATA;

typedef struct srmnd_mem_data
{
   uns32 used_mem;
   uns32 free_mem;
   uns32 used_swap_space;
   uns32 free_swap_space;
   uns32 buff_mem;
} SRMND_MEM_DATA;

typedef struct srmnd_sample_data
{
   /* Time Stamp at which the sample value was updated */
   time_t when_updated;
  
   /* Resource Threshold Value */
   NCS_SRMSV_VALUE value;

   /* Ptr to the next sample, != NULL if number of samples > 1 */
   struct srmnd_sample_data  *next_sample;
} SRMND_SAMPLE_DATA;

#define  SRMND_SAMPLE_DATA_NULL  ((SRMND_SAMPLE_DATA*)0)


typedef struct srmnd_watermark_val
{
   NCS_SRMSV_VALUE l_val;
   NCS_SRMSV_VALUE h_val;
} SRMND_WATERMARK_VAL;

/*************************************************************************
 * This struct.maintains the monitor control information on a particular
 * resource.
 *************************************************************************/
typedef struct srmnd_monitor_info
{
   NCS_SRMSV_MON_DATA  monitor_data;

   /* Time Stamp at which the sample value was updated */
   time_t recent_updated;

   long long time_ms;

   /* Based on the rsrc type, appropriate datastruct is taken */
   union
   {
      SRMND_CPU_DATA cpu_data;
      double         cpu_ldavg;
      uns32          proc_cpu_jiffies;
   } info;

   NCS_SRMSV_VALUE result_value;

   SRMND_WATERMARK_VAL  wm_val;

   /* Sample record num to update the sample data,
      next_sample_to_update = (next_sample_to_update  mod samples) + 1 */
   uns16  next_sample_to_update;

   /* till now updated samples. Should not exceed the samples requested for */
   uns16  samples_updated;

   /* Sample records will be created against the number of samples requested 
      Sampling should be done once all the records are updated with resource
      statistics data */
   SRMND_SAMPLE_DATA  *sample_data;
} SRMND_MONITOR_INFO;

#define  SRMND_MONITOR_INFO_NULL  ((SRMND_MONITOR_INFO*)0)


/* Used to store PIDs of descandants */
typedef struct srmnd_pid_data
{
   /* PID to monitor */
   uns32 pid;
   NCS_BOOL checked_childs;
   uns8     child_level;
 
   struct srmnd_pid_data *parent;
   struct srmnd_pid_data *child_list;

   /* Child Ptrs */
   struct srmnd_pid_data *prev_child_pid;
   struct srmnd_pid_data *next_child_pid;

   /* Pid list ptrs */
   struct srmnd_pid_data *prev_pid;
   struct srmnd_pid_data *next_pid;
   
} SRMND_PID_DATA;


/*************************************************************************
 * This is the main resource record holds the complete information of
 * resource data, monitor data, subscription data etc.. 
 *************************************************************************/
typedef struct srmnd_rsrc_mon_node
{
   NCS_PATRICIA_NODE   rsrc_mon_pat_node; 
    
   /* Handle assigned for this resource record, 
      this acts as a key to identify this record */
   uns32   rsrc_mon_hdl;

   /* SRMA specific Resource Mon handle */
   uns32   srma_rsrc_hdl;

   /* Should set the respective sent Macro flag defined above */
   uns8    sent_flag;

   /* Ptr to the resource type record to which it belongs to */
   SRMND_RSRC_TYPE_NODE  *rsrc_type_node;

   /* Should be updated only if the resource type belongs to PID 
      descendants specific */
   uns16           pid_num;
   /* Contains all pid descendants info */
   SRMND_PID_DATA  *descendant_pids;
   
   /* User type is just a
      subscriber/requestor/both requestor & subscriber */
   NCS_SRMSV_USER_TYPE  usr_type;

   /* Updated only if the user is requestor or requestor + subscriber */
   SRMND_MONITOR_INFO  mon_data;
    
   /* In the monitoring mode or not */
   SRMND_MONITOR_STATE monitor_state; 

   /* timer-id returned by the RP timer create */
   NCS_RP_TMR_HDL tmr_id;
   /* when the rsrc will be deleted from monitoring process, '0' forever */
   time_t  rsrc_mon_expiry;    
    
   /* Pointer of the SRMA user node by which this resource is configured */
   SRMND_MON_SRMA_USR_NODE     *srma_usr_node;
    
   NCS_SRMSV_VALUE             notify_val;

   /* Prev & Next resource ptr's of the resource list of SRMA */
   struct srmnd_rsrc_mon_node  *prev_srma_usr_rsrc;
   struct srmnd_rsrc_mon_node  *next_srma_usr_rsrc;
      
   /* Prev & Next resource ptr's of (1) the subscriber list if the rsrc mon
      is just subscriber else they are the ptrs resource-type list */
   struct srmnd_rsrc_mon_node  *prev_rsrc_type_ptr;
   struct srmnd_rsrc_mon_node  *next_rsrc_type_ptr;

} SRMND_RSRC_MON_NODE;

#define  SRMND_RSRC_MON_NODE_NULL  ((SRMND_RSRC_MON_NODE*)0)


/* Add the user-appl node as a starting node of user-appl list of SRMND */
#define m_SRMND_ADD_APPL_NODE(srmnd, user)           \
{ \
   if (srmnd->start_srma_node != NULL) {             \
      user->next_srma_node = srmnd->start_srma_node; \
      srmnd->start_srma_node->prev_srma_node = user; \
   } \
   srmnd->start_srma_node = user; \
}

/* Add the rsrc-type node as a starting node of rsrc-type list of SRMND */
#define m_SRMND_ADD_RSRC_TYPE_NODE(srmnd, rsrc_type)      \
{ \
   if (srmnd->start_rsrc_type != NULL) {                  \
      rsrc_type->next_rsrc_type = srmnd->start_rsrc_type; \
      srmnd->start_rsrc_type->prev_rsrc_type = rsrc_type; \
   } \
   srmnd->start_rsrc_type = rsrc_type; \
}

#define m_SRMND_UPDATE_VALUE(value, valtype, scaletype) \
{ \
   value.val_type   = valtype;   \
   value.scale_type = scaletype; \
}


#endif /* SRMND_DB_H */



