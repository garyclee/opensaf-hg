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
 */

/* PDRBD.H: Master include file for PDRBD Service and user *.C files */

#ifndef PDRBD_H
#define PDRBD_H

#include <stdarg.h>
/* Get the compile time options */
#include "ncs_opt.h"

/* Get General Definations */
#include "gl_defs.h"

/* Get target's suite of header files...*/
#include "t_suite.h"

/* From /base/common/inc */
#include "ncs_main_pvt.h"
#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncsft_rms.h"
#include "ncs_ubaid.h"
#include "ncsencdec.h"
#include "ncs_stack.h"
#include "ncs_log.h"
#include "ncs_lib.h"
#include "ncsdlib.h"
#include "ncs_edu_pub.h"

#include "saAis.h"
#include "saEvt.h"
#include "saClm.h"

#include "dta_papi.h"
#if (NCS_DTS == 1)
#include "dts_papi.h"
#endif

#include "pdrbd_cb.h"
#include "pdrbd_mem.h"
#include "pdrbd_amf.h"
#include "pdrbd_log.h"
#include "pdrbd_util.h"
#include "pdrbd_mds.h"

#endif
