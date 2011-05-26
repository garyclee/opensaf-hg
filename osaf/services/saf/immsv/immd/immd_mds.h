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
 * Author(s): Ericsson AB
 *
 */

#ifndef IMMD_MDS_H
#define IMMD_MDS_H

uns32 immd_mds_register(IMMD_CB *cb);
void immd_mds_unregister(IMMD_CB *cb);
uns32 immd_mds_msg_sync_send(IMMD_CB *cb, uns32 to_svc, MDS_DEST to_dest,
				      IMMSV_EVT *i_evt, IMMSV_EVT **o_evt, uns32 timeout);

uns32 immd_mds_msg_send(IMMD_CB *cb, uns32 to_svc, MDS_DEST to_dest, IMMSV_EVT *evt);

uns32 immd_mds_send_rsp(IMMD_CB *cb, IMMSV_SEND_INFO *s_info, IMMSV_EVT *evt);

uns32 immd_mds_bcast_send(IMMD_CB *cb, IMMSV_EVT *evt, NCSMDS_SVC_ID to_svc);
uns32 immd_mds_vdest_create(IMMD_CB *cb);

#endif   /* IMMD_MDS_H */
