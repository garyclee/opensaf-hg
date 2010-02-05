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

  This module is the main include file for HPL.

******************************************************************************
*/

#ifndef HPL_H
#define HPL_H

/* Get general definitions */
#include "ncsgl_defs.h"

/* Get target's suite of header files...*/

/* From /base/common/inc */
#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncs_ubaid.h"
#include "ncsencdec_pub.h"
#include "ncs_stack.h"
#include "ncs_log.h"
#include "ncs_lib.h"
#include "ncs_util.h"

/* From targsvcs/common/inc */
#include "mds_papi.h"

/* HPL specific inc Files */

#include "hpl_api.h"
#include "hpl_cb.h"
#include "hisv_msg.h"
#include "hpl_mds.h"
#include "hpl_mem.h"
#include "hcd_mem.h"
#include "hcd_log.h"

/* HPL CB global handle declaration */
EXTERN_C uns32 gl_hpl_hdl;
EXTERN_C uns32 free_hisv_ret_msg(HISV_MSG *msg);

#endif   /* !HPL_H */
