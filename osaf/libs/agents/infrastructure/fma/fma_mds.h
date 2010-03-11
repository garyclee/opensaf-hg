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

#ifndef FMA_MDS_H
#define FMA_MDS_H

#include "fma_fm_intf.h"

struct fma_cb;

#define FMA_FM_SYNC_MSG_DEF_TIMEOUT 1000	/* 10 seconds */

#define FMA_MDS_SUB_PART_VERSION 1

#define FMA_SUBPART_VER_MIN  1
#define FMA_SUBPART_VER_MAX  1

#define FMA_FM_MSG_FMT_VER_1 1

EXTERN_C uns32 fma_mds_reg(struct fma_cb *cb);
EXTERN_C uns32 fma_mds_dereg(struct fma_cb *cb);
EXTERN_C uns32 fma_fm_mds_msg_async_send(struct fma_cb *cb, FMA_FM_MSG *msg);
EXTERN_C uns32 fma_mds_callback(NCSMDS_CALLBACK_INFO *info);
EXTERN_C uns32 fma_fm_mds_msg_bcast(struct fma_cb *cb, FMA_FM_MSG *msg);
EXTERN_C uns32 fma_fm_mds_msg_sync_send(struct fma_cb *cb, FMA_FM_MSG *i_msg, FMA_FM_MSG **o_msg, uns32 timeout);

#endif
