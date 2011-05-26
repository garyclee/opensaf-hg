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

#ifndef IMMD_SBEDU_H
#define IMMD_SBEDU_H

uns32 immd_sb_proc_fevs(IMMD_CB *cb, IMMSV_FEVS *fevs_msg);
uns32 immd_process_sb_msg(IMMD_CB *cb, IMMD_MBCSV_MSG *msg);
uns32 immd_mbcsv_init(IMMD_CB *cb);
uns32 immd_mbcsv_open(IMMD_CB *cb);
uns32 immd_mbcsv_selobj_get(IMMD_CB *cb);
uns32 immd_mbcsv_sync_update(IMMD_CB *cb, IMMD_MBCSV_MSG *msg);
uns32 immd_mbcsv_async_update(IMMD_CB *cb, IMMD_MBCSV_MSG *msg);
uns32 immd_mbcsv_register(IMMD_CB *cb);
uns32 immd_mbcsv_callback(NCS_MBCSV_CB_ARG *arg);
uns32 immd_mbcsv_finalize(IMMD_CB *cb);
uns32 immd_mbcsv_chgrole(IMMD_CB *cb);
uns32 immd_process_sb_fevs(IMMD_CB *cb, IMMSV_FEVS *fevs_msg);
uns32 immd_process_sb_count(IMMD_CB *cb, uns32 count, uns32 evt_type);
uns32 immd_process_node_accept(IMMD_CB *cb, IMMSV_D2ND_CONTROL *ctrl);

#endif
