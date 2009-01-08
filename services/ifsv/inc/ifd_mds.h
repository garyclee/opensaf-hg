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

  MODULE NAME: IFD_MDS.H

$Header: 
..............................................................................

  DESCRIPTION: Prototypes definations for IFD-MDS functions


******************************************************************************/

#ifndef IFD_MDS_H
#define IFD_MDS_H

/* embedding subslot changes for backward compatibility */
#define IFD_SVC_PVT_SUBPART_VERSION 2
#define IFD_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT 1
#define IFD_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT 2
#define IFD_WRT_IFND_SUBPART_VER_RANGE             \
        (IFD_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT - \
         IFD_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT +1)

uns32 ifd_mds_init (IFSV_CB *cb);
uns32 ifd_mds_shut (IFSV_CB *cb);

#endif /* IFD_MDS_H */
