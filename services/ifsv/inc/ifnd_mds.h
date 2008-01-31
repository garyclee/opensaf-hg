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

  MODULE NAME: IFND_MDS.H

$Header: 
..............................................................................

  DESCRIPTION: Prototypes definations for IFND-MDS functions


******************************************************************************/

#ifndef IFND_MDS_H
#define IFND_MDS_H

#define IFND_SVC_PVT_SUBPART_VERSION 1

#define IFND_WRT_IFD_SUBPART_VER_AT_MIN_MSG_FMT 1
#define IFND_WRT_IFD_SUBPART_VER_AT_MAX_MSG_FMT 1
#define IFND_WRT_IFD_SUBPART_VER_RANGE             \
        (IFND_WRT_IFD_SUBPART_VER_AT_MAX_MSG_FMT -  \
         IFND_WRT_IFD_SUBPART_VER_AT_MIN_MSG_FMT +1)

#define IFND_WRT_IFA_SUBPART_VER_AT_MIN_MSG_FMT 1
#define IFND_WRT_IFA_SUBPART_VER_AT_MAX_MSG_FMT 1
#define IFND_WRT_IFA_SUBPART_VER_RANGE             \
        (IFND_WRT_IFA_SUBPART_VER_AT_MAX_MSG_FMT -  \
         IFND_WRT_IFA_SUBPART_VER_AT_MIN_MSG_FMT +1)

#define IFND_WRT_DRV_SUBPART_VER_AT_MIN_MSG_FMT 1
#define IFND_WRT_DRV_SUBPART_VER_AT_MAX_MSG_FMT 1
#define IFND_WRT_DRV_SUBPART_VER_RANGE             \
        (IFND_WRT_DRV_SUBPART_VER_AT_MAX_MSG_FMT -  \
         IFND_WRT_DRV_SUBPART_VER_AT_MIN_MSG_FMT +1)


uns32 ifnd_mds_init (IFSV_CB *cb);
uns32 ifnd_mds_shut (IFSV_CB *cb);
uns32 ifnd_rcv_ifd_up_evt (MDS_DEST *ifd_dest,IFSV_CB *cb);
uns32 ifnd_mds_evt (MDS_DEST *ifd_dest,IFSV_CB *cb, IFSV_EVT_TYPE evt_type);

#endif /* IFND_MDS_H */

