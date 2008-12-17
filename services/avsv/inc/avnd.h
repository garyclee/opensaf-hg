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

  This module is the main include file for Availability Node Director.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVND_H
#define AVND_H

#include "avsv.h"
#include "avnd_dl_api.h"

/* Porting Include Files */
#include "avsv_d2nmem.h"
#include "avsv_n2avamem.h"
#include "avsv_n2clamem.h"
#include "avnd_defs.h"
#include "avnd_mem.h"
#include "avsv_ipc.h"

/* AvSv Common Files */
#include "avsv_amfparam.h"
#include "avsv_clmparam.h"
#include "avsv_d2nmsg.h"
#include "avsv_n2avamsg.h"
#include "avsv_n2clamsg.h"

/* AvND Files */
#include "avnd_tmr.h"
#include "avnd_mds.h"
#include "avnd_proc.h"
#include "avnd_hc.h"
#include "avnd_err.h"
#include "avnd_comp.h"
#include "avnd_evt.h"
#include "avnd_su.h"
#include "avnd_pg.h"
#include "avnd_di.h"
#include "avnd_util.h"
#include "avnd_clm.h"
#include "avnd_cb.h"
#include "avnd_log.h"
#include "avnd_mib.h"
#include "avnd_srm.h"
#include "ncs_trap.h"
#include "avnd_trap.h"
#endif
