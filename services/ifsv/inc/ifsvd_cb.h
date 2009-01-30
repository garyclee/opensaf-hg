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

#if 1
#ifndef IFSVD_CB_H

#define m_IFSV_IDIM_GET_MBX_HDL gl_ifsv_idimlib.idim_mbx
#define m_IFSV_IDIM_GET_TASK_HDL gl_ifsv_idimlib.idim_hdl
#define m_IFSV_IDIM_STORE_HDL(hdl) gl_ifsv_idim_hdl=hdl
#define m_IFSV_IDIM_GET_HDL gl_ifsv_idim_hdl

#define NCS_IFSV_IDIM_TASKNAME "idim"
#define NCS_IFSV_IDIM_PRIORITY  (5)
#define NCS_IFSV_IDIM_STACKSIZE NCS_STACKSIZE_HUGE

#define NCS_IFSV_NETLINK_TASKNAME "IFND-NETLINK"
#define NCS_IFSV_NETLINK_PRIORITY  (5)
#define NCS_IFSV_NETLINK_STACKSIZE NCS_STACKSIZE_HUGE

/**** global variable ******/
EXTERN_C uns32 gl_ifsv_idim_hdl;

/*****************************************************************************
 * This is the structure which holds the task and mail box handle.
 *****************************************************************************/
typedef struct ifsv_idimlib_struct
{
    SYSF_MBX   idim_mbx;
    NCSCONTEXT idim_hdl;    
} IFSV_IDIMLIB_STRUCT;

/*****************************************************************************
 * This is the control block of IDIM.
 *****************************************************************************/
typedef struct ifsv_idim_cb
{ 
   uns32            shelf;
   uns32            slot;   
   /* embedding subslot changes */
   uns32                subslot;
   MDS_HDL          mds_hdl;
   uns32            ifnd_hdl;
   MDS_DEST         ifnd_addr;
   SYSF_MBX         ifnd_mbx;
   NCS_DB_LINK_LIST port_reg; /* The ports registered with IDIM, this will be used in communicating with the Interface hardware drivers. */
} IFSV_IDIM_CB;

/*****************************************************************************
 * This is port registeration table. This holds the port and type registered.
 *****************************************************************************/
typedef struct ifsv_port_type_reg_tbl
{
   NCS_DB_LINK_LIST_NODE   q_node;
   NCS_IFSV_PORT_TYPE      port_type;
   MDS_DEST                drv_dest;
} IFSV_IDIM_PORT_TYPE_REG_TBL;

/***** Function prototypes *****/

/****  IDIM Initalization *****/
uns32 ifnd_idim_init (IFSV_CB *ifsv_cb);

/****  IDIM Destroy *****/
uns32 ifnd_idim_destroy (IFSV_CB *ifsv_cb);

#endif /*#ifndef IFSVD_CB_H */

#endif

