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

#include "dts_tgt.h"


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
#include "ncsmiblib.h"

#if (NCS_CLI == 1)
#include "ncs_cli.h"
#endif

#include "ncs_mda_pvt.h"
#include "ncs_mda_papi.h"
#include "mds_papi.h"
/*Smik - included ncs_edu_pub.h */
#include "ncs_edu_pub.h"
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

#include "oac_papi.h"

#include "saAis.h"

#if (NCS_CLI == 1)
#include "dtsvcli.h"
#endif

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "mbcsv_papi.h"
#include "ncs_main_pvt.h"
#include "dts_opt.h"
#include "dtsv_msg.h"
#include "dts_papi.h"
#include "dta_api.h"
#include "dta_papi.h"
#include "dts_mapi.h"
#include "dts_log.h"
#include "dts_api.h"
#include "dts_mem.h"
#include "dts_def.h"
#include "sysf_log.h"

/*#include "dts_ckp.h"*/
#include "dts_ckpt_msg.h"

#include "dts_pvt.h"
#include "dtsv_mem.h"
#include "dts_dl_api.h"
#include "dts_amf.h"

#include "dts_ckp.h"
#include "dts_ckpt_updt.h"
#include "dts_ckpt_edu.h"

#endif

