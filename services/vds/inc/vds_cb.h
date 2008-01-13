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

  MODULE NAME:       vds_cb.h


  DESCRIPTION:

******************************************************************************
*/
#ifndef VDS_CB_H
#define VDS_CB_H

#define m_VDS_TASK_PRIORITY (5)
#define m_VDS_STACKSIZE     (8000)

#define VDS_CANCEL_THREAD         TRUE
#define VDS_DONT_CANCEL_THREAD    FALSE

#define VDS_SVC_PVT_VER  1
#define VDS_WRT_VDA_SUBPART_VER_MIN 1
#define VDS_WRT_VDA_SUBPART_VER_MAX 1
#define VDS_WRT_VDA_SUBPART_VER_RANGE \
        (VDS_WRT_VDA_SUBPART_VER_MAX - \
         VDS_WRT_VDA_SUBPART_VER_MIN +1)

#define m_NCS_VDS_ENC_MSG_FMT_GET     m_NCS_ENC_MSG_FMT_GET
#define m_NCS_VDS_MSG_FORMAT_IS_VALID m_NCS_MSG_FORMAT_IS_VALID

typedef struct vds_vdest_adest_info
{
   MDS_DEST   adest;      /* Adest of requesting VDA */
   struct vds_vdest_adest_info *next;
} VDS_VDEST_ADEST_INFO;


typedef struct vds_vdest_db_info
{
   NCS_PATRICIA_NODE       name_pat_node;  /* For index by name */
   SaNameT                 vdest_name;     /* Key to this structure */

   NCS_PATRICIA_NODE       id_pat_node;    /* For index by VDEST */
   MDS_DEST                vdest_id;
   VDS_VDEST_ADEST_INFO    *adest_list;    /* Sorted list */
   NCS_BOOL                persistent;
} VDS_VDEST_DB_INFO;


typedef struct vds_cb
{
   SYSF_MBX               mbx;        /* mail box on which SRMND waits */
   MDS_HDL                mds_hdl;    /* MDS handle */
   NCS_LOCK               cb_lock;    /* CB Lock */
   uns8                   pool_id;    /* pool-id used by handle-mgr */
   uns32                  cb_hdl;     /* hdl returned by handle-mgr */
   VDS_AMF_ATTRIB         amf;
   VDS_CKPT_ATTRIB        ckpt;
   NCS_BOOL               mds_init_done;
   MDS_DEST               vds_vdest;    /* my MDS_DEST stored for convenience */

   /*  TREE of VDS_VDEST_DB_INFO nodes indexed by name */
   NCS_PATRICIA_TREE      vdest_name_db;

   /*  TREE of VDS_VDEST_DB_INFO nodes indexed by id */
   NCS_PATRICIA_TREE      vdest_id_db;

   MDS_DEST               latest_allocated_vdest;
} VDS_CB;

#define VDS_CB_NULL (VDS_CB *)0

#endif

