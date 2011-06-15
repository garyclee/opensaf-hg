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

..............................................................................

  DESCRIPTION: The master include for all DTS and user *.C files.
 
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#ifndef DTS_H
#define DTS_H

/* Get general definitions.....*/

#include <ncsgl_defs.h>

/* Get target's suite of header files...*/


/* From /base/common/inc */

#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncs_ubaid.h"
#include "ncsencdec_pub.h"
#include "ncs_stack.h"

#include "ncs_lib.h"

#include "ncs_mda_papi.h"
#include "mds_papi.h"
#include "logtrace.h"
/* From targsvcs/common/inc */

#include "mds_papi.h"

#include "saAis.h"

typedef uint64_t MBCSV_ANCHOR;

/* From /base/products/rms/inc */
#include "mbcsv_papi.h"
#include "mbcsv_evts.h"
#include "mbcsv_tmr.h"
#include "mbcsv_evt_msg.h"
#include "mbcsv_env.h"
#include "mbcsv_mbx.h"
#include "mbcsv_pwe_anc.h"
#include "mbcsv_mem.h"
#include "mbcsv_mds.h"
#include "mbcsv_dl_api.h"

#endif
