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
FILE NAME: IFDIPXS.H
DESCRIPTION: Enums, Structures and Function prototypes for IPXS
******************************************************************************/

#ifndef IFDIPXS_H
#define IFDIPXS_H

EXTERN_C uns32 ipxs_ifd_lib_req (IPXS_LIB_REQ_INFO *req_info);

EXTERN_C uns32 ipxs_ifd_ipinfo_process(IPXS_CB *cb, IPXS_IP_NODE *ip_node);

EXTERN_C uns32 ipxs_mib_tbl_req (struct ncsmib_arg *args);

EXTERN_C uns32 ipxs_ifd_ipinfo_process (IPXS_CB *cb, IPXS_IP_NODE *ip_node);

EXTERN_C uns32 ipxs_ifd_ifip_info_bcast(IPXS_CB *cb, 
                        IPXS_IFIP_INFO *ifip_info, uns32 am);

EXTERN_C uns32 ipxs_ifd_proc_v4_unnmbrd(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info, uns32 unnmbrd);
EXTERN_C uns32 ifd_ipxs_evt_process(IFSV_CB *cb, IFSV_EVT *evt);
EXTERN_C uns32 ifd_ipxs_proc_data_ifip_info(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                            NCS_IFSV_IFINDEX *if_index);

#endif /* IFSVIPXS_H */
