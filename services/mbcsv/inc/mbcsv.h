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

  $Header: 


..............................................................................

  DESCRIPTION: The master include for all DTS and user *.C files.
 
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#ifndef DTS_H
#define DTS_H

/*  Get compile time options...*/

#include "ncs_opt.h"

/* Get general definitions.....*/

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
#include "ncs_mtbl.h"

#if (MBCSV_LOG == 1)
#include "dta_papi.h"
#endif

#include "ncs_lib.h"
#include "ncsmiblib.h"

/*#include "ncs_mda_pvt.h"*/
#include "ncs_mda_papi.h"
#include "mds_papi.h"

/* From targsvcs/common/inc */
#if 0
#include "sysf_mds.h"  
#if (NCS_MDS == 1)
#include "mds_inc.h"
#endif
#endif

#if (NCS_MDS == 1)
#include "mds_papi.h"
#endif

#include "saAis.h"

typedef uns64 MBCSV_ANCHOR;

/* From /base/products/rms/inc */
#include "mbcsv_papi.h"
#include "mbcsv_dbg.h"
#include "mbcsv_evts.h"
#include "mbcsv_tmr.h"
#include "mbcsv_evt_msg.h"
#include "mbcsv_env.h"
#include "mbcsv_log.h"
#include "mbcsv_mbx.h"
#include "mbcsv_pwe_anc.h"
#include "mbcsv_mem.h"
#include "mbcsv_mds.h"
#include "mbcsv_dl_api.h"

#endif

