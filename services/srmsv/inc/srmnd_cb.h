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

  MODULE NAME: SRMND_CB.H

$Header: 
..............................................................................

  DESCRIPTION: SRMND Control Block and the related function prototypes are 
               defined here.

******************************************************************************/

#ifndef SRMND_CB_H
#define SRMND_CB_H


#define SRMND_SVC_PVT_VER  1
#define SRMND_WRT_SRMA_SUBPART_VER_MIN 1
#define SRMND_WRT_SRMA_SUBPART_VER_MAX 1
#define SRMND_WRT_SRMA_SUBPART_VER_RANGE             \
        (SRMND_WRT_SRMA_SUBPART_VER_MAX - \
         SRMND_WRT_SRMA_SUBPART_VER_MIN +1)

#define m_NCS_SRMND_ENC_MSG_FMT_GET     m_NCS_ENC_MSG_FMT_GET
#define m_NCS_SRMND_MSG_FORMAT_IS_VALID m_NCS_MSG_FORMAT_IS_VALID


#define m_SRMND_TASK_PRIORITY (5)
#define m_SRMND_STACKSIZE     NCS_STACKSIZE_HUGE

#define m_SRMND_TASK_RELEASE(task_hdl) \
{ \
   m_NCS_TASK_RELEASE(task_hdl); \
   task_hdl = 0;                 \
}
   
                  

/* SRMND HA State */
typedef enum srmnd_ha_state
{
   SRMND_HA_STATE_NULL = 0, 
   SRMND_HA_STATE_ACTIVE = SA_AMF_HA_ACTIVE, 
   SRMND_HA_STATE_QUIESCED = SA_AMF_HA_QUIESCED,
   SRMND_HA_STATE_QUIESCING = SA_AMF_HA_QUIESCING,
   SRMND_HA_STATE_MAX 
} SRMND_HA_STATE;

/****************************************************************************
 * System Resource Monitoring Node Director Control Block
 ***************************************************************************/
typedef struct srmnd_cb
{
   SYSF_MBX      mbx;        /* mail box on which SRMND waits */
   uns32         node_id;    /* Node Id on which the SRMND is running */

   MDS_HDL       mds_hdl;    /* MDS handle */
   MDS_DEST      srmnd_dest; /* SRMND MDS address */
    
   NCS_LOCK      cb_lock;    /* CB Lock */
   uns8          pool_id;    /* pool-id used by handle-mgr */
   uns32         cb_hdl;     /* hdl returned by handle-mgr */
   EDU_HDL       edu_hdl;    /* EDU handle */

   NCS_SRMSV_ND_OPER_STATUS oper_status; /* SRMND operation status, would be
                                            set when AMF makes SRMND active */
   /* AMF specific attributes */
   NCS_BOOL        health_check_started; /* healthcheck start flag */
   SaNameT         comp_name;     /* component Name */
   SRMND_HA_STATE  ha_cstate;     /* HA Current state */    
   SaAmfHandleT    amf_handle;    /* AMF Handle */ 
   SaSelectionObjectT    amf_sel_obj;
   SaAmfHealthcheckKeyT  health_check_key; /* Health check key info */ 

   /* Resource mon data base */
   NCS_PATRICIA_TREE   rsrc_mon_tree;
    
   /* Timer list on which the timer threads operate */
   NCS_RP_TMR_CB  *srmnd_tmr_cb;

   /* List of SRMA's with their resources registered with SRMND */
   SRMND_MON_SRMA_USR_NODE  *start_srma_node;

   /* List of resource types registered with SRMND for monitoring */
   SRMND_RSRC_TYPE_NODE *start_rsrc_type;   

   MDS_SVC_PVT_SUB_PART_VER srmnd_mds_ver; 
} SRMND_CB;

#define  SRMND_CB_NULL  ((SRMND_CB*)0)

/****************************************************************************
 *                            Function Prototypes 
 ***************************************************************************/
EXTERN_C uns32 srmnd_destroy (NCS_BOOL deactivate_flg);
EXTERN_C uns32 srmnd_active(SRMND_CB *srmnd);
EXTERN_C uns32 srmnd_deactive(SRMND_CB *srmnd);

#endif /* SRMND_CB_H */


