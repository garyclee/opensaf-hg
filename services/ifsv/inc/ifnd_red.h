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

#ifndef IFND_RED_H
#define IFND_RED_H

/*****************************************************************************
 * This file contains the data structures, enums, #defines, function 
 * prototypes, needed for IfND no redundancy design.
 *****************************************************************************/

/*****************************************************************************
 * This is the data structure for storing MDS_DESTs, received from IfAs or
 * Drivers. 
 *****************************************************************************/
typedef struct ifnd_mds_type_info
{
   MDS_DEST              mds_dest;
   NCSMDS_SVC_ID         type; /* IFA or Driver. */
}IFND_MDS_TYPE_INFO;

typedef struct ifnd_mds_dest_info_rec
{
   NCS_PATRICIA_NODE     pat_node;
   IFND_MDS_TYPE_INFO    info;
}IFND_MDS_DEST_INFO_REC;


/*****************************************************************************
 *     Function prototype 
 *****************************************************************************/
uns32 ifnd_rcv_ifa_up_evt (MDS_DEST *ifd_dest, IFSV_CB *cb);
uns32 ifnd_mds_dest_get (MDS_DEST *mds_dest, IFSV_CB *cb);
uns32 ifnd_mds_dest_destroy_all (IFSV_CB *cb);
uns32 ifnd_mds_dest_info_tmr (IFSV_EVT* evt, IFSV_CB *cb);
uns32 ifnd_data_retrival_from_ifd (IFSV_CB *ifsv_cb, 
                             IFSV_INTF_CREATE_INFO *create_intf, 
                             IFSV_SEND_INFO *sinfo);

uns32 ifd_reten_timer_for_ifnd (IFSV_EVT* evt, IFSV_CB *cb);
extern uns32 ifnd_ifa_same_dst_intf_rec_and_mds_dest_del (MDS_DEST *mds_dest, IFSV_CB *cb);
extern uns32 ifnd_mds_dest_del (MDS_DEST mds_dest, IFSV_CB *cb);
extern uns32 ifnd_mds_svc_evt_ifa (MDS_DEST *ifa_dest, IFSV_CB *cb);
extern uns32 ifnd_mds_svc_evt_ifdrv (MDS_DEST *drv_dest, IFSV_CB *cb);
extern uns32 ifnd_drv_same_dst_intf_rec_and_mds_dest_del (MDS_DEST *mds_dest, IFSV_CB *cb);
extern IFND_MDS_TYPE_INFO * ifnd_mds_dest_type_get (MDS_DEST mds_dest, IFSV_CB *cb);
extern uns32 ifnd_send_idim_ifndup_event (IFSV_CB *cb);


/************************ file ends here *******************************/
#endif 
