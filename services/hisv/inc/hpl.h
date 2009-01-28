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

/*  Get compile time options */
#include "ncs_opt.h"

/* Get general definitions */
#include "ncsgl_defs.h"

/* Get target's suite of header files...*/
#include "t_suite.h"

/* From /base/common/inc */
#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncsft_rms.h"
#include "ncs_ubaid.h"
#include "ncsencdec.h"
#include "ncs_stack.h"
#include "ncs_mib.h"
#include "ncs_mtbl.h"
#include "ncs_log.h"
#include "ncs_lib.h"
#include "ncs_util.h"

/* From targsvcs/common/inc */
#if 0
#include "sysf_mds.h"

#if (NCS_MDS == 1)
#include "mds_inc.h"
#endif
#endif
#include "mds_papi.h"

/* From /base/products/rms/inc */
#if (NCS_RMS == 1)
#include "rms_env.h"
#endif

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
EXTERN_C uns32 free_hisv_ret_msg (HISV_MSG *msg);

#endif /* !HPL_H */
