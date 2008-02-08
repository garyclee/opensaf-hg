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

  MODULE NAME: SRMSV.H

$Header: 
..............................................................................

  DESCRIPTION: Master include file for all SRMSv *.C files.

******************************************************************************/

#ifndef SRMSV_H
#define SRMSV_H


#include <stdarg.h>

/* Get the compile time options */
#include "ncs_opt.h"

/* Get General Definations */
#include "gl_defs.h"

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
#include "ncsmiblib.h"
#include "ncs_mtbl.h"
#include "ncs_log.h"
#include "ncs_lib.h"
#include "ncsdlib.h"
#include "ncs_tmr.h"
#include "ncs_queue.h"
#include "ncs_main_pvt.h"
#include "ncs_main_papi.h"
#include "ncs_util.h"

/* From targsvcs/common/inc */
#include "mds_papi.h"

#include "ncs_edu_pub.h"

#if 0
#include "sysf_mds.h"  
#if (NCS_MDS == 1)
#include "mds_inc.h"
#endif
#endif

/* AMF header files */
#include "saAis.h"
#include "saAmf.h"

/* mtree header files */
#include "ncs_mtree.h"
#include "ncs_xmtre.h"

/* DTS header file */
#include "dts_papi.h"
#include "dta_papi.h"

/* SRMSv specific header files */
#include "srmsv_papi.h"
#include "srma_papi.h"
#include "srmsv_db.h"
#include "srmsv_defs.h"
#include "srmsv_msg.h"
#include "srma_cbk.h"
#include "srma_db.h"
#include "srmnd_db.h"
#include "srmsv_mem.h"
#include "srmsv_log.h"
#include "srmsv_logstr.h"

/* SRMST specific header file */
#include "srmst_defs.h"


#endif /* SRMSV_H */

