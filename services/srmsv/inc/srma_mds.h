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

  MODULE NAME: SRMA_MDS.H

$Header: 
..............................................................................

  DESCRIPTION: Prototypes definations for SRMA-MDS functions

******************************************************************************/

#ifndef SRMA_MDS_H
#define SRMA_MDS_H


/****************************************************************************
 *                            srma_mds.c 
 ***************************************************************************/
EXTERN_C uns32 srma_mds_reg(SRMA_CB *cb);
EXTERN_C uns32 srma_mds_unreg(SRMA_CB *cb);
EXTERN_C uns32 srma_mds_adest_get(SRMA_CB *cb);
EXTERN_C uns32 srma_mds_cbk(NCSMDS_CALLBACK_INFO *info);
EXTERN_C uns32 srma_mds_send(SRMA_CB *srma,
                             SRMA_MSG *srma_msg,
                             MDS_DEST *srmnd_dest);
#endif /* SRMA_MDS_H */



