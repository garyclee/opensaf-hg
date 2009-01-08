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
FILE NAME: IFSVPROC.H
DESCRIPTION: This file contains:      
      1) Function prototypes for IfD/IfND interface record process.
******************************************************************************/
#ifndef IFDPROC_H
#define IFDPROC_H

/* This is the function used to create the interface */
uns32 ifd_intf_create (IFSV_CB *ifsv_cb, 
                       struct ifsv_intf_create_info *create_intf, 
                       IFSV_SEND_INFO *sinfo);

/* This is the function used to delete the interface */
uns32 ifd_intf_delete (IFSV_CB *ifsv_cb, 
                       struct ifsv_intf_destroy_info *dest_info,
                       IFSV_SEND_INFO *sinfo);

/* This is the function used to delete all the interface record */
uns32 ifd_all_intf_rec_del (IFSV_CB *ifsv_cb);

/* Function prototype to access/clean mail Box */
NCS_BOOL ifd_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg);

/* Function initialize CB */
uns32 ifd_init_cb (IFSV_CB *ifsv_cb);

IFSVDLL_API uns32 ifd_lib_init (struct ifsv_create_pwe *pwe_param);
IFSVDLL_API uns32 ifd_lib_destroy (uns32 vrid, uns32 comp_type);

uns32 ifd_same_dst_all_intf_rec_mark_del (MDS_DEST *dest, IFSV_CB *ifsv_cb);

uns32 ifd_evt_send (NCSCONTEXT msg, IFSV_EVT_TYPE evt_type, IFSV_CB *ifsv_cb);

uns32
ifd_process_evt (IFSV_EVT* evt);

extern uns32 ifd_same_node_id_all_intf_rec_del (NODE_ID node_id, IFSV_CB *ifsv_cb);
extern uns32 ifd_bcast_to_ifnds (IFSV_INTF_DATA *intf_data, IFSV_INTF_REC_EVT rec_evt,
                          uns32 attr, IFSV_CB *cb);

#if (NCS_VIP == 1)
uns32 ifd_init_vip_db(IFSV_CB *cb);
#endif 
#endif /* #ifndef IFDPROC_H */
