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

/*****************************************************************************
..............................................................................
  MODULE NAME: SNMPTM.H

..............................................................................

  DESCRIPTION: Contains Include files for SNMPTM

******************************************************************************
*/
#ifndef SNMPTM_H
#define SNMPTM_H

#ifndef NCS_SNMPTM
#define NCS_SNMPTM 1
#endif

/*****************************************************************************
                      Common Include Files.
*****************************************************************************/
#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_mib_pub.h>
#include <opensaf/ncs_ubaid.h>
#include <opensaf/ncs_svd.h>
#include <opensaf/ncspatricia.h>
#include <opensaf/ncs_iplib.h>
#include <opensaf/ncsmiblib.h>
#include <opensaf/ncs_edu_pub.h>
#include <opensaf/oac_papi.h>
#include <opensaf/ncssysf_tsk.h>
#include <opensaf/ncssysf_lck.h>
#include <opensaf/ncs_main_papi.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncs_hdl_pub.h>
#include <opensaf/ncssysfpool.h>
#include <opensaf/ncssysf_mem.h>
#include <opensaf/ncs_cli.h>
#include <opensaf/mac_papi.h>
#include <opensaf/ncs_trap.h>
#include <opensaf/mds_papi.h>

/* SAF specific includes */
#include <saAis.h>
#include <saAmf.h>
#include <saEvt.h>

/*****************************************************************************
                      SNMPTM Base Include Files.
*****************************************************************************/
#include "snmptm_tblid.h"
#include "snmptm_defs.h"
#include "ncs_snmptm.h"
#include "snmptm_tbl.h"
#include "snmptm_cb.h"
#include "snmptm_mapi.h"
#include "snmptm_cli.h"

#endif /* SNMPTM_H */

