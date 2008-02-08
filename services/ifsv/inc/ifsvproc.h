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
#ifndef IFSVPROC_H
#define IFSVPROC_H

#if (NCS_IFD == 1)
/* This is the function used to create the interface */
uns32 ifd_intf_create (IFSV_CB *ifsv_cb, 
                       struct ifsv_intf_create_info *create_intf,
                       IFSV_SEND_INFO *sinfo);

/* This is the function used to delete the interface */
uns32 ifd_intf_delete (IFSV_CB *ifsv_cb, 
                       struct ifsv_intf_destroy_info *dest_info);

/* This is the function used to delete all the interface record */
uns32 ifd_all_intf_rec_del (IFSV_CB *ifsv_cb);
#endif

#if (NCS_IFD == 1)
/* This function is used to delete all the the record in IfND */
uns32
ifnd_all_intf_rec_del (IFSV_CB *ifsv_cb);

/* This function will send a message to IfD to allocate Ifindex */
uns32
ifnd_ifap_ifindex_alloc (NCS_IFSV_SPT_MAP *spt_map,
                         IFSV_CB *ifsv_cb);
/* This function will send a message to IfD to free Ifindex */
uns32
ifnd_ifap_ifindex_free (NCS_IFSV_SPT_MAP *spt_map, 
                        IFSV_CB *ifsv_cb);

/* This function creates an interface record */
uns32
ifnd_intf_create (IFSV_CB *ifsv_cb, 
                  struct ifsv_intf_create_info *create_intf);
/* This function deletes an interface record */
uns32
ifnd_intf_delete (IFSV_CB *ifsv_cb, 
                  struct ifsv_intf_destroy_info *dest_info);
#endif


#endif /* #ifndef IFSVPROC_H */
