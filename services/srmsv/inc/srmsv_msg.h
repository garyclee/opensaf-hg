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

  MODULE NAME: SRMSV_EVT.H

$Header: 
..............................................................................

  DESCRIPTION: This file contains SRMSv specific event data. It also contains
               event specific function prototype definitions.

******************************************************************************/

#ifndef SRMSV_EVT_H
#define SRMSV_EVT_H


struct srma_usr_appl_node; 
struct srmnd_mon_srma_usr_node;

/*************************************************************************
            Data Structs defined for SRMA specific events
**************************************************************************/
/* Events that are generated by SRMA to SRMND */
typedef enum srma_msg_type
{
   /* Events that are generated by SRMA to SRMND */
   SRMA_UNREGISTER_MSG = 1,   /* De-registers SRMA info (includes all rsrc 
                                 mon data) from SRMSv */
   SRMA_START_MON_MSG,        /* User application can START & STOP monitoring
                                 its configured resources, it applies to all 
                                 the resources configured by SRMA instance */
   SRMA_STOP_MON_MSG,

   SRMA_CREATE_RSRC_MON_MSG,  /* Registers SRMA with SRMSv & also registers
                                 the resource monitor data with SRMSv */
   SRMA_BULK_CREATE_MON_MSG,

   /* Enums that are specific to a particular resource monitoring */
   SRMA_DEL_RSRC_MON_MSG,     /* Deletes the RSRC-MON data from SRMSv */
   SRMA_MODIFY_RSRC_MON_MSG,  /* Modify the configured resource monitor data */

   /* Enum to send the msg for watermark values of a perticular rsrc */
   SRMA_GET_WATERMARK_MSG,    /* To get the watermark values of a rsrc */

   SRMA_MSG_MAX
} SRMA_MSG_TYPE;


/* Data struct used to update the data to send SRMA_CREATE_MON_MSG,
   SRMA_MODIFY_MON_MSG events to SRMND 
   For Create: rsrc_hdl is SRMA specific Resource Handle 
   For Modify: rsrc_hdl is SRMND specific Resource Handle
*/
typedef struct srma_create_rsrc_mon
{
   uns32     rsrc_hdl;
   NCS_SRMSV_MON_INFO mon_info;
} SRMA_CREATE_RSRC_MON;

#define SRMA_MODIFY_RSRC_MON  SRMA_CREATE_RSRC_MON

typedef struct srma_create_rsrc_mon_node
{
   SRMA_CREATE_RSRC_MON  rsrc;
   struct srma_create_rsrc_mon_node *next_rsrc_mon;
} SRMA_CREATE_RSRC_MON_NODE;

/* Data struct's used to update the data to send  
   SRMA_BULK_CREATE_MON_MSG event to SRMND */
typedef struct srma_bulk_create_rsrc_mon
{
   /* Carry this information to SRMND to make SRMND to decide whether to start
      monitoring process on the configured resource or not */
   NCS_BOOL  stop_monitoring;

   /* Will be considered and update this ptr when encoding the msg at SRMA.
      This will avoid building bulk_rsrc_mon list and it is not required
      because SRMND already has the data list to process */
   struct srma_srmnd_usr_node *srmnd_usr_rsrc; 

   /* This will be needed to build this event at SRMND by decoding the 
      respective message arrived from SRMA */
   SRMA_CREATE_RSRC_MON_NODE *bulk_rsrc; 
} SRMA_BULK_CREATE_RSRC_MON;

/* Data struct's used to update the data to send SRMA_DEL_RSRC_MON_MSG 
   event to SRMND */
typedef struct srma_delete_rsrc_mon
{
   /* Resource specific SRMND handle */
   uns32  srmnd_rsrc_hdl;
} SRMA_DELETE_RSRC_MON;


/* Data struct's used to update the data to send SRMA_GET_WATERMARK_MSG
   event to SRMND */
typedef struct srma_get_watermark
{
   uns32 pid;
   NCS_SRMSV_RSRC_TYPE rsrc_type;
   NCS_SRMSV_WATERMARK_TYPE wm_type;
} SRMA_GET_WATERMARK;


typedef struct srma_msg
{
   /* One of the following above define SRMA event types */
   SRMA_MSG_TYPE msg_type;

   /* Set to true if the event need to bcast, for this msg, no need to send
      CREATED ack */
   NCS_BOOL  bcast; 

   /* User specific SRMA handle */
   uns32  handle;

   union
   {
      SRMA_GET_WATERMARK    get_wm;
      SRMA_CREATE_RSRC_MON  create_mon;
      SRMA_MODIFY_RSRC_MON  modify_rsrc_mon;
      SRMA_DELETE_RSRC_MON  del_rsrc_mon;
      SRMA_BULK_CREATE_RSRC_MON  bulk_create_mon;
   }info;
    
}SRMA_MSG;

#define  SRMA_MSG_NULL  ((SRMA_MSG*)0)


/*************************************************************************
            Data Structs defined for SRMND specific events
**************************************************************************/
/* Events that are generated by SRMND to SRMA */
typedef enum srmnd_msg_type
{
   SRMND_CREATED_MON_MSG = 1,  
   SRMND_BULK_CREATED_MON_MSG,
   SRMND_APPL_NOTIF_MSG,
   SRMND_SYNCHRONIZED_MSG,
   SRMND_MBX_MSG_HEALTH_CHECK_START,
   SRMND_WATERMARK_VAL_MSG,
   SRMND_WATERMARK_EXIST_MSG,
   SRMND_PROC_EXIT_MSG,
   SRMND_MSG_MAX
} SRMND_MSG_TYPE;


/* Following two structs are used for the SRMND_BULK_CREATED_MON_MSG */
typedef struct srmnd_created_rsrc_mon
{
   uns32    srma_rsrc_hdl;
   uns32    srmnd_rsrc_hdl;
   struct srmnd_created_rsrc_mon  *next_srmnd_rsrc_mon;
} SRMND_CREATED_RSRC_MON;

#define  SRMND_CREATED_RSRC_MON_NULL  ((SRMND_CREATED_RSRC_MON*)0)


/* Data struct's used to update the watermark specific data  */
typedef struct srmnd_watermark_data
{
   MDS_DEST  srma_dest;
   uns32     usr_hdl;
   uns32     pid;
   NCS_SRMSV_RSRC_TYPE  rsrc_type;
   NCS_SRMSV_WATERMARK_TYPE wm_type;
} SRMND_WATERMARK_DATA;

typedef struct srmnd_bulk_created_rsrc_mon
{
   /* Will be considered and update this ptr when encoding the msg at SRMA.
      This will avoid building bulk_rsrc_mon list and it is not required
      because SRMA already has the data list to process */
   struct srmnd_mon_srma_usr_node *srma_usr_node;
 
   /* This will be needed to build this event at SRMND by decoding the 
      respective message arrived from SRMA */
   SRMND_CREATED_RSRC_MON  *bulk_rsrc_mon;
} SRMND_BULK_CREATED_RSRC_MON;


/* Event data struct used by the SRMS to send an event to the respective SRMA */
typedef struct srmnd_msg
{
    /* One of the following above define SRMND event types */
    SRMND_MSG_TYPE  msg_type;

    /* Resource Specific SRMA handle, not required to update for
       'SRMND_SYNCHRONIZED_MSG' event */
    uns32  srma_rsrc_hdl;

    union
    {
        /* Update it for SRMND_CREATED_MON_MSG */
        uns32  srmnd_rsrc_mon_hdl;

        /* Update it for SRMND_BULK_CREATED_MON_MSG */
        SRMND_BULK_CREATED_RSRC_MON  bulk_rsrc_mon;

        /* Update it for SRMND_THRESHOLD_MSG */
        NCS_SRMSV_VALUE  notify_val;
     
        SRMSV_WATERMARK_VAL wm_val;
    } info;
    
} SRMND_MSG;

#define  SRMND_MSG_NULL  ((SRMND_MSG*)0)

/* Function prototypes */
EXTERN_C void  srmsv_agent_nd_msg_free(SRMND_MSG *msg);
EXTERN_C uns32 srmsv_edp_nd_msg(EDU_HDL *edu_hdl,
                                EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr,
                                uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env,
                                EDP_OP_TYPE op,
                                EDU_ERR *o_err);

EXTERN_C uns32 srmsv_edp_agent_msg(EDU_HDL *edu_hdl,
                                   EDU_TKN *edu_tkn,
                                   NCSCONTEXT ptr,
                                   uns32 *ptr_data_len,
                                   EDU_BUF_ENV *buf_env,
                                   EDP_OP_TYPE op,
                                   EDU_ERR *o_err);

EXTERN_C uns32 srmsv_edp_agent_enc_bulk_create(EDU_HDL *,
                                               EDU_TKN *,
                                               NCSCONTEXT,
                                               uns32 *,
                                               EDU_BUF_ENV *,
                                               EDP_OP_TYPE,
                                               EDU_ERR *);

EXTERN_C uns32 srmsv_edp_nd_enc_bulk_created(EDU_HDL *, 
                                               EDU_TKN *,
                                               NCSCONTEXT,
                                               uns32 *,
                                               EDU_BUF_ENV *,
                                               EDP_OP_TYPE,
                                               EDU_ERR *);

#endif /* SRMSV_EVT_H */

