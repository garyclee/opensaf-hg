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

#ifndef EDA_H
#define EDA_H

#include "ncsgl_defs.h"

#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncs_ubaid.h"
#include "ncsencdec_pub.h"
#include "ncs_stack.h"
#include "ncs_log.h"
#include "ncs_lib.h"

#include "mds_papi.h"

#if (NCS_DTS == 1)
#include "dts_papi.h"
#endif

#if (NCS_DTA == 1)
#include "dta_papi.h"
#endif

#include "edsv_msg.h"
#include "eda_cb.h"
#include "eda_hdl.h"
#include "eda_mds.h"
#include "eda_mem.h"
#include "eda_log.h"
#include "edsv_logstr.h"
#include "edsv_mem.h"
#include "edsv_defs.h"
#include "edsv_util.h"

/* EDA CB global handle declaration */
uns32 gl_eda_hdl;

/* EDA Default MDS timeout value */
#define EDA_MDS_DEF_TIMEOUT 100

/* EDSv maximum length of a pattern size */
#define EDSV_MAX_PATTERN_SIZE 255

/*Length of a default publisher name */
#define EDSV_DEF_NAME_LEN 4

/*Max limit for reserved events (from 0 - 1000)*/
#define MAX_RESERVED_EVENTID 1000

#endif   /* !EDA_H */
