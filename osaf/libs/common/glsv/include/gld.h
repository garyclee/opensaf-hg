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

  This module is the main include file for Global Lock Director (GLD).
  
******************************************************************************
*/

#ifndef GLD_H
#define GLD_H
typedef enum {

  NO_CRD_QA_VAL,
  V_CARD_QA_1,
  V_CARD_QA_2,

  } V_CARD_QA;


/* Common Headers */
#include "ncsgl_defs.h"
#include "t_suite.h"


/* From /base/common/inc */
#include "ncs_lib.h"
#include "ncs_ubaid.h"
#include "mbcsv_papi.h"
#include "mds_papi.h"
#include "ncs_log.h"
#include "ncs_edu_pub.h"
#include "ncsencdec.h"
#include "mbcsv_papi.h"
#include "ncsmiblib.h"
#include "ncs_mda_pvt.h"
#include "ncs_mda_papi.h"
#include "ncs_util.h"

/* DTS header file */
#include "dts_papi.h"
#include "dta_papi.h"
#include "saClm.h"
#include "glsv_defs.h"
#include "glsv_lck.h"
#include "gld_cb.h"
#include "gld_amf.h"
#include "gld_dl_api.h"
#include "gld_api.h"
#include "gld_evt.h"
#include "glnd_evt.h"
#include "gld_log.h"
#include "gld_mds.h"
#include "gld_mem.h"
#include "glnd_mem.h"
#include "gld_edu.h"
#include "gld_tmr.h"
#include "gld_mbcsv.h"
#include "oac_papi.h"
#include "glsv_mapi.h"
#include "gld_mib.h"

#include "cpsv_mem.h"
#include "cpa_mem.h"

#endif /* !GLD_H */
