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

  MODULE NAME: SRMND_MDS.H

$Header: 
..............................................................................

  DESCRIPTION: Prototypes definations for SRMND-MDS functions

******************************************************************************/

#ifndef SRMND_MDS_H
#define SRMND_MDS_H
EXTERN_C uns32 srmnd_mds_reg(SRMND_CB *srmnd);
EXTERN_C uns32 srmnd_mds_unreg(SRMND_CB *srmnd);
EXTERN_C uns32 srmnd_mds_cbk(NCSMDS_CALLBACK_INFO *info);
EXTERN_C uns32 srmnd_mds_send(SRMND_CB  *srmnd,
                              SRMND_MSG *srmnd_msg,
                              MDS_DEST  *srma_dest);
#endif /* SRMND_MDS_H */



