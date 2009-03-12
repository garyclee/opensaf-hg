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

  MODULE NAME: SRMA_CB.H

..............................................................................

  DESCRIPTION: Defined SRMA Control Block and the related function prototypes.

******************************************************************************/

#ifndef SRMA_CB_H
#define SRMA_CB_H


#define SRMA_SVC_PVT_VER  1
#define SRMA_WRT_SRMND_SUBPART_VER_MIN 1
#define SRMA_WRT_SRMND_SUBPART_VER_MAX 1
#define SRMA_WRT_SRMND_SUBPART_VER_RANGE             \
        (SRMA_WRT_SRMND_SUBPART_VER_MAX - \
         SRMA_WRT_SRMND_SUBPART_VER_MIN +1)

#define m_NCS_SRMA_ENC_MSG_FMT_GET     m_NCS_ENC_MSG_FMT_GET
#define m_NCS_SRMA_MSG_FORMAT_IS_VALID m_NCS_MSG_FORMAT_IS_VALID


/* System Resource Monitoring Agent Control Block */
typedef struct srma_cb
{
   uns32      cb_hdl;    /* CB hdl returned by hdl manager */

   /* MDS parameters */
   MDS_HDL    mds_hdl;   /* MDS handle */
   MDS_DEST   srma_dest; /* SRMA MDS address */
   NODE_ID    node_num;  /* Stores the local node ID information */

   EDU_HDL    edu_hdl;   /* EDU handle */
   uns8       pool_id;   /* pool-id used by hdl manager */
   NCS_LOCK   lock;      /* CB lock */

   /* This timer list will be used to lookup running timer and stop it
    * when an Async response is received.
    */
   NCS_RP_TMR_CB  *srma_tmr_cb;

   /* Resource mon data base */
   NCS_PATRICIA_TREE  rsrc_mon_tree;

   /* Pointer to the list of SRMNDs that SRMA configured the rsrc's */
   SRMA_SRMND_INFO    *start_srmnd_node;

   /* Pointer to the list of user applications */
   SRMA_USR_APPL_NODE *start_usr_appl_node;
   
   MDS_SVC_PVT_SUB_PART_VER  srma_mds_ver;
} SRMA_CB;

#define  SRMA_CB_NULL  ((SRMA_CB*)0)


/* Macro to validate the dispatch flags */
#define m_SRMA_DISPATCH_FLAG_IS_VALID(flag)  \
               ((SA_DISPATCH_ONE == flag) || \
                (SA_DISPATCH_ALL == flag) || \
                (SA_DISPATCH_BLOCKING == flag))

#define m_SRMSV_RETRIEVE_SRMA_CB  (SRMA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA, gl_srma_hdl)
#define m_SRMSV_GIVEUP_SRMA_CB    ncshm_give_hdl(gl_srma_hdl)

#endif /* SRMA_CB_H */
