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

#ifndef FM_MDS_H
#define FM_MDS_H

#define FM_MDS_SUB_PART_VERSION 1

#define FM_SUBPART_VER_MIN      1
#define FM_SUBPART_VER_MAX      1

#define FM_FMA_MSG_FMT_VER_1    1
#define FM_FM_MSG_FMT_VER_1     1

EXTERN_C uns32 fm_mds_init(FM_CB *cb);

EXTERN_C uns32 fm_mds_finalize(FM_CB *cb);

EXTERN_C uns32 fm_mds_sync_send(FM_CB *fm_cb, NCSCONTEXT msg,
                                NCSMDS_SVC_ID svc_id,
                                MDS_SEND_PRIORITY_TYPE priority,
                                MDS_SENDTYPES  send_type,
                                MDS_DEST   *i_to_dest,
                                MDS_SYNC_SND_CTXT *mds_ctxt); 


EXTERN_C uns32 fm_mds_async_send(FM_CB *fm_cb,NCSCONTEXT msg,
                                 NCSMDS_SVC_ID svc_id,
                                 MDS_SEND_PRIORITY_TYPE priority,
                                 MDS_SENDTYPES  send_type,
                                 MDS_DEST   i_to_dest,
                                 NCSMDS_SCOPE_TYPE bcast_scope); 

#endif

