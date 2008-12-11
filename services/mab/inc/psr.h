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


******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#ifndef PSR_H
#define PSR_H

#include "ncsgl_defs.h"
#include "ncs_opt.h"
#include "t_suite.h"
#include "mab_tgt.h"

#include "mds_papi.h"

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
#include "ncsmiblib.h"
#include "patricia.h"
#include "ncs_mda_pvt.h"
#include "ncs_sprr_papi.h"
#include "saAmf.h"
#include "ncs_saf.h"

#include "dta_papi.h"

/* From targsvcs/common/inc */
#include "mds_papi.h"  
#include "sysf_file.h"


/* From /base/products/mab/inc and pubinc */
#include "mab_env.h"
#include "mab_penv.h"
#include "mab_msg.h"

#include "oac_papi.h"
#include "mac_papi.h"

#include "app_amf.h"
#include "psr_version.h"
#include "psr_def.h"
#include "psr_mapi.h"
#include "psr_api.h"
#include "psr_dl_api.h"
#include "psr_mib_load.h"
#include "pss_mbcsv.h"
#include "psr_pvt.h"
#include "psr_bam.h"
#include "psr_log.h"
#include "psr_amf.h"

#endif /* #ifndef PSR_H */ 

