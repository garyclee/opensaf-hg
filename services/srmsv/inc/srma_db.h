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

  MODULE NAME: SRMA_DB.H

$Header: 
..............................................................................

  DESCRIPTION: This file has the data structure definitions defined for SRMA 
               database. It also contains the related function prototype 
               definitions.

******************************************************************************/

#ifndef SRMA_DB_H
#define SRMA_DB_H

struct srma_srmnd_usr_node;
struct srma_rsrc_mon;

#define SRMA_PROC_DESCENDANTS  -1

typedef enum
{
  SRMA_SRMND_USR_TYPE_NODE = 1,
  SRMA_USR_SRMND_TYPE_NODE
} SRMA_SRMND_USR_NODE_TYPE;

/****************************************************************************
 * This data structure holds the SRMND information to which the application
 * requested to monitor the resource. It maintains SRMND specific resource
 * monitor records with respect to the application..
 ***************************************************************************/
typedef struct srma_srmnd_info
{
    MDS_DEST  srmnd_dest;
    NCS_BOOL  is_srmnd_up;

    NODE_ID   node_id;

    /* Resource list that are configured for this SRMND */
    struct srma_srmnd_usr_node  *start_usr_node;

    /* Prev and Next ptrs of SRMND nodes */
    struct srma_srmnd_info  *prev_srmnd_node;
    struct srma_srmnd_info  *next_srmnd_node;
} SRMA_SRMND_INFO;

#define  SRMA_SRMND_INFO_NULL  ((SRMA_SRMND_INFO*)0)


/****************************************************************************
 * An application specific data structure, was created when the application
 * registers with SRMSv service. Application specific requested resources are
 * enlisted under this structure with respect to the corresponding SRMND 
 * structure.
 ***************************************************************************/
typedef struct srma_usr_appl_node
{
    /* Handle of this node */
    uns32               user_hdl;
    NCS_SEL_OBJ         sel_obj;

    /* List of resources that need to broadcast */
    struct srma_rsrc_mon *bcast_rsrcs;

    /* Set to true if any one its resource monitor data set to bcast */
    long  bcast; 

    /* Set to TRUE to STOP monitoring the requested resources of the
       application */
    NCS_BOOL            stop_monitoring;

    /* Application Call back function */
    NCS_SRMSV_CALLBACK  srmsv_call_back;    

    /* Set to TRUE - stops adding cbk data for applications */
    NCS_BOOL            block_updating_cbk;

    /* Pending data, to be sent to application upon dispatch call */
    struct srma_pend_callback  pend_cbk;

    /* SRMND list configured by this application */
    struct srma_srmnd_usr_node  *start_srmnd_node;  

    /* Prev & Next User application node ptrs */
    struct srma_usr_appl_node  *prev_usr_appl_node;
    struct srma_usr_appl_node  *next_usr_appl_node;
} SRMA_USR_APPL_NODE;

#define  SRMA_USR_APPL_NODE_NULL  ((SRMA_USR_APPL_NODE*)0)


/****************************************************************************
 * This is the main data structure that holds the information of the resource
 * monitor data requested by an application
 ***************************************************************************/
typedef struct srma_rsrc_mon
{
    NCS_PATRICIA_NODE       rsrc_pat_node; /* Tree element in the RSRC mon.structure */
    uns32                   rsrc_mon_hdl;  /* Handle for this resource mon cfg,
                                              also a KEY for this node */
    NCS_SRMSV_RSRC_INFO     rsrc_info;     /* Monitored resource key */
    NCS_BOOL                stop_monitoring;  /* In the monitoring mode or not */
    NCS_SRMSV_MON_DATA      monitor_data;  /* Monitor config & control data */  
    time_t                  rsrc_mon_expiry; /* 0 means remains forever, 
                                      current timestamp + monitoring interval */

    NCS_RP_TMR_HDL          tmr_id;
    /* Handle asigned for the resource by SRMND */
    uns32                   srmnd_rsrc_hdl;

    /* Can get to the respect SRMND node through this ptr,
       if rsrc_info->srmsv_loc == ALL (bcast) it will be NULL */
    struct srma_srmnd_usr_node  *srmnd_usr;

    /* Can get to the user-application node through this Ptr */
    struct srma_srmnd_usr_node  *usr_appl;    

    struct srma_usr_appl_node  *bcast_appl;

    /* Prev & Next ptrs of resource that are specific to the application */
    struct srma_rsrc_mon    *prev_usr_rsrc_mon;
    struct srma_rsrc_mon    *next_usr_rsrc_mon;
    
    /* Prev & Next ptrs of resource that are specific to particular SRMND
       and also used to maintain rsrc's in BCAST list of SRMA if 
       rsrc_info->srmsv_loc == ALL (bcast) */
    struct srma_rsrc_mon    *prev_srmnd_rsrc;  
    struct srma_rsrc_mon    *next_srmnd_rsrc;

    /* Prev & Next ptrs of resource that hangs under a specific aging bucket */
    struct srma_rsrc_mon    *prev_aging_rsrc_mon;
    struct srma_rsrc_mon    *next_aging_rsrc_mon;
} SRMA_RSRC_MON;

#define  SRMA_RSRC_MON_NULL  ((SRMA_RSRC_MON*)0)


/****************************************************************************
 * This data structure holds the information about the target SMRND to which
 * the application is requested to monitor the resource. Application requested
 * resource monitor records are enlisted under this structure with respect to
 * the requested application structure
 ***************************************************************************/
typedef struct srma_srmnd_usr_node
{
    /* As this struct servies as a node under USER list Node and
       under SRMND list Node, to identify under which list this comes under
       below given flag type will be used. */
    SRMA_SRMND_USR_NODE_TYPE node_type;

    /* Pointer to which SRMND it belongs to */
    SRMA_SRMND_INFO  *srmnd_info;
    
    /* For User Application specific data */
    SRMA_USR_APPL_NODE *usr_appl;

    /* Resource list that are configured by the user */
    SRMA_RSRC_MON    *start_rsrc_mon;

    struct srma_srmnd_usr_node  *prev_usr_node;
    struct srma_srmnd_usr_node  *next_usr_node;
} SRMA_SRMND_USR_NODE;

#define  SRMA_SRMND_USR_NODE_NULL  ((SRMA_SRMND_USR_NODE*)0)

#endif /* SRMA_DB_H */
