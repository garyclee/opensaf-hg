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

  DESCRIPTION:

  This module is the main include file for Global Lock Agent (GLA).
  
******************************************************************************
*/

#ifndef GLA_H
#define GLA_H

/* Common Headers */
#include "ncsgl_defs.h"
#include "ncs_util.h"

/* From /base/common/inc */
#include "ncs_lib.h"
#include "ncs_ubaid.h"
#include "mds_papi.h"
#include "logtrace.h"
#include "ncsencdec_pub.h"

/* DTS header file */
#include "dta_papi.h"

/* GLA Porting Include Files */
#include "glsv_defs.h"
#include "glsv_lck.h"
#include "gla_mem.h"

#include "gla_dl_api.h"
#include "gla_callbk.h"
#include "gla_evt.h"
#include "glnd_evt.h"
#include "gla_cb.h"
#include "gla_mds.h"

#endif   /* !GLA_H */
