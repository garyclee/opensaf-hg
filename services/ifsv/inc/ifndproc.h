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
FILE NAME: IFNDPROC.H
DESCRIPTION: Enums, Structures and Function prototypes used for IFND processing
******************************************************************************/

#ifndef IFNDPROC_H
#define IFNDPROC_H

/* Macros used by IFND for getting interface index from IFD (IAPS) */
#define m_IFND_IFAP_IFINDEX_ALLOC(slo_port_map,cb, evt) ifnd_ifap_ifindex_alloc(slo_port_map,cb,evt)
#define m_IFND_IFAP_IFINDEX_FREE(slo_port_map,cb) ifnd_ifap_ifindex_free(slo_port_map,cb)

/* This function is used to delete all the the record in IfND */
uns32
ifnd_all_intf_rec_del (IFSV_CB *ifsv_cb);

/* This function will send a message to IfD to allocate Ifindex */
uns32
ifnd_ifap_ifindex_alloc (NCS_IFSV_SPT_MAP *spt_map,
                         IFSV_CB *ifsv_cb, NCSCONTEXT *evt);
/* This function will send a message to IfD to free Ifindex */
uns32
ifnd_ifap_ifindex_free (NCS_IFSV_SPT_MAP *spt_map, 
                        IFSV_CB *ifsv_cb);

/* This function creates an interface record */
uns32
ifnd_intf_create (IFSV_CB *ifsv_cb, 
                  struct ifsv_intf_create_info *create_intf, 
                  IFSV_SEND_INFO *sinfo);
/* This function deletes an interface record */
uns32
ifnd_intf_delete (IFSV_CB *ifsv_cb, 
                  struct ifsv_intf_destroy_info *dest_info,
                  IFSV_SEND_INFO *sinfo);

/****** Response to Interface Get request from app. ******/
uns32 
ifnd_intf_ifinfo_send (IFSV_CB *cb,  IFSV_EVT_INTF_INFO_GET  *get_info, IFSV_SEND_INFO *sinfo);

uns32 
ifnd_intf_statsinfo_send (IFSV_CB *cb,  IFSV_EVT_STATS_INFO  *stats_info);

/* Function prototype to access/clean mail Box */
NCS_BOOL ifnd_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg);

/* Function initialize CB */
uns32 ifnd_init_cb (IFSV_CB *ifsv_cb);

IFSVDLL_API uns32 ifnd_lib_init (struct ifsv_create_pwe *pwe_param);
IFSVDLL_API uns32 ifnd_lib_destroy (uns32 vrid, uns32 comp_type);


uns32 ifnd_same_dst_intf_rec_del (MDS_DEST *dest, IFSV_CB *ifsv_cb);
uns32 ifnd_same_drv_intf_rec_del (IFSV_CB *ifsv_cb);

uns32
ifnd_mds_msg_send (NCSCONTEXT msg, IFSV_EVT_TYPE msg_type, IFSV_CB *ifsv_cb);
uns32
ifnd_evt_send (NCSCONTEXT msg, IFSV_EVT_TYPE evt_type, IFSV_CB *ifsv_cb);
uns32
ifnd_idim_evt_send (NCSCONTEXT msg, IFSV_IDIM_EVT_TYPE msg_type,
                         IFSV_CB *ifsv_cb);

uns32 
ifsv_ifa_app_svd_info_indicate(IFSV_CB *cb, IFSV_INTF_DATA *actual_data, 
                            NCS_IFSV_SVC_DEST_UPD *svc_dest);
extern uns32 ifnd_mds_msg_sync_send (NCSCONTEXT msg, IFSV_EVT_TYPE msg_type,
                                     IFSV_CB *ifsv_cb, NCSCONTEXT *o_evt);
extern uns32 ifnd_sync_send_to_ifd (IFSV_INTF_DATA *intf_data, 
                                    IFSV_INTF_REC_EVT rec_evt, uns32 attr, 
                                    IFSV_CB *cb, IFSV_EVT **o_evt);
extern uns32 ifnd_same_dst_intf_rec_del (MDS_DEST *dest, IFSV_CB *ifsv_cb);


#endif /* #ifndef FNDPROC_H */


