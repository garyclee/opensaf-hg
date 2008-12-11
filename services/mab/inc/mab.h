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

  $Header: /ncs/software/release/UltraLinq/MAB/MAB1.0/base/products/mab/inc/mab.h 9     3/28/01 1:08p Questk $


..............................................................................

  DESCRIPTION: The master include for all MAB and user *.C files.
 
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#ifndef MAB_H
#define MAB_H

/*  Get compile time options...*/

#include "ncs_opt.h"

/* Get general definitions.....*/

#include "gl_defs.h"

#if (NCS_MAB == 1)

/* Get target's suite of header files...*/
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
#if (NCS_MAS == 1)
#include "saAmf.h"
#include "ncs_saf.h"
#endif /* #if (NCS_MAS == 1) */ 

/* From targsvcs/common/inc */

#include "mds_papi.h"  
#include "sysf_file.h"


/* From /base/products/mab/inc and pubinc */
#include "mab_env.h"
#include "mab_penv.h"
#include "mab_msg.h"
#include "mab_log.h"

#include "mac_papi.h"
#include "mac_api.h"
#include "mac_pvt.h"

#if (NCS_MAS == 1)
#include "app_amf.h"
#if (NCS_MAS_RED == 1)
#include "mas_mbcsv.h"
#endif /* #if (NCS_MAS_RED == 1) */
#endif /* (NCS_MAS == 1) */

#include "mas_api.h"
#include "mas_pvt.h"
#if (NCS_MAS ==1)
#include "mas_amf.h"
#endif /* (NCS_MAS ==1) */

#include "oac_papi.h"
#include "oac_dl_api.h"
#include "oac_api.h"
#include "oac_pvt.h"

#include "dta_papi.h"

#endif /* (NCS_MAB == 1) */

#endif /* #ifndef MAB_H */ 

