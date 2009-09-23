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

  This module is the main include file for Availability Director.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_H
#define AVD_H

#include "avsv.h"
#include "avsv_d2nmsg.h"

/* Porting Include Files */
#include "avsv_d2nmem.h"
#include "avd_def.h"
#include "avd_mem.h"
#include "avsv_ipc.h"

/* AVD-AVND common EDPs */
#include "avsv_d2nedu.h"

/* AvD managment API Include Files */
#include "avd_dl_api.h"

#include "mbcsv_papi.h"
#include "avd_ckp.h"

/* AvD specific include files */
#include "avd_tmr.h"
#include "avd_msg.h"
#include "avd_cb.h"
#include "avd_hlt.h"
#include "avd_avnd.h"
#include "avd_sg.h"
#include "avd_su.h"
#include "avd_si_dep.h"
#include "avd_si.h"
#include "avd_comp.h"
#include "avd_csi.h"
#include "avd_susi.h"
#include "avd_pg.h"

#include "ncs_avsv_bam_msg.h"
#include "avm_avd.h"

#include "avd_dblog.h"
#include "avd_fail.h"
#include "avd_evt.h"
#include "avd_proc.h"
#include "avd_hb.h"
#include "avd_mibtbl.h"

#include "avd_bam.h"
#include "avd_mds.h"

/* Checkpointing specific include files */
#include "avd_ckpt_msg.h"
#include "avd_ckpt_edu.h"
#include "avd_ckpt_updt.h"

#include "avd_avm.h"
#include "ncs_trap.h"
#include "avd_ntf.h"

#endif
