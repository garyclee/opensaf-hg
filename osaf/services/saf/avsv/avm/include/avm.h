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
 
  DESCRIPTION:This is the common include file for all AvM source files.
  ..............................................................................

 
******************************************************************************
*/

#ifndef __AVM_H__
#define __AVM_H__

/* Common Include Files */
#include "ncs_opt.h"
#include "ncsgl_defs.h"
#include "t_suite.h"

#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncsft_rms.h"
#include "ncs_ubaid.h"
#include "ncsencdec.h"
#include "ncs_stack.h"
#include "ncssysf_def.h"
#include "ncsdlib.h"
#include "ncs_mib.h"
#include "ncs_lib.h"
#include "ncs_dummy.h"

#include "ncs_mda_pvt.h"
#include "mds_papi.h"
/*#include "mds_papi.h"*/

/* #include "ncs_edu_pub.h" */

#include "ncs_mib_pub.h"
#include "ncsmiblib.h"
#include "ncs_main_pvt.h"
#include "oac_papi.h"
#include "oac_api.h"
#include "ncspatricia.h"
#include "ncs_edu_pub.h"
#include "mbcsv_papi.h"
#include "ncs_saf_edu.h"
#include "ncs_trap.h"

#include "rda_papi.h"

#include "saAis.h"
#include "saEvt.h"
#include "saClm.h"

#if (NCS_AVM_LOG == 1)
#include "ncs_log.h"
#include "dta_papi.h"
#endif

#include <SaHpi.h>

#include "hpl.h"
#include "hpl_api.h"
#include "hpl_msg.h"
#include "avm_defs.h"

#include "avm_dhcp_conf.h"
#include "ncs_bam_avm.h"
#include "avm_dl_api.h"
#include "avm_dblog.h"
#include "avm_avd.h"
#include "avm_avd_api.h"
#include "avm_fsm.h"
#include "avm_tmr.h"
#include "avm_db.h"
#include "avm_evt.h"
#include "avm_fund.h"
#include "fm_papi.h"
#include "avm_cb.h"
#include "avm_mem.h"
#include "avm_hpi_util.h"
#include "avm_util.h"
#include "avm_ckpt_msg.h"
#include "avm_ckp.h"
#include "avm_ckpt_edu.h"
#include "avm_ckpt_updt.h"
#include "avm_mapi.h"
#include "avm_mds.h"
#include "ncs_util.h"

extern uns32 g_avm_hdl;
extern void *g_avm_task_hdl;

#endif   /*__AVM_H__ */
