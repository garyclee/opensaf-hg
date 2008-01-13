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
FILE NAME: IFNDIPXS.H
DESCRIPTION: Enums, Structures and Function prototypes for IPXS
******************************************************************************/

#ifndef IFNDIPXS_H
#define IFNDIPXS_H

uns32 ipxs_ifnd_lib_req (IPXS_LIB_REQ_INFO *req_info);

uns32 ifnd_ipxs_evt_process(IFSV_CB *cb, IFSV_EVT *evt);

uns32 ipxs_ifnd_if_info_attr_cpy(IPXS_CB *cb, uns32 ifindex, 
                                              NCS_IPXS_INTF_REC *intf_rec);

uns32 ipxs_ifa_app_svd_info_indicate(IFSV_CB *cb, 
                               IFSV_INTF_DATA *actual_data, 
                               NCS_IFSV_SVC_DEST_UPD *svc_dest);
extern uns32 ifnd_ipxs_data_proc_ifip_info (IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                            NCS_IPXS_IPINFO **rcv_ifip_info);
extern uns32 ifnd_ipxs_proc_ifa_app_add(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                        IFSV_SEND_INFO *sinfo);

#endif /* IFSVIPXS_H */
