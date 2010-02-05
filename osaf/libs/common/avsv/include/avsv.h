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

  This module is the main include file for the entire Availability Service.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVSV_H
#define AVSV_H

/* temporary compilation flags setting that will go away */
#if (NCS_AVSV_LOG == 1)
#define NCS_AVA_LOG 1
#define NCS_CLA_LOG 1
#else				/* NCS_AVSV_LOG == 1 */
#define NCS_AVA_LOG 0
#define NCS_CLA_LOG 0
#endif   /* NCS_AVSV_LOG == 1 */

#include "ncsgl_defs.h"
#include "ncsdlib.h"
#include "ncs_saf.h"
#include "ncs_lib.h"
#include "mds_papi.h"
#include "ncs_mda_pvt.h"
#include "ncs_edu_pub.h"
#include "ncs_main_pvt.h"
#include "avsv_log.h"
#include "avsv_defs.h"
#include "avsv_util.h"
#include "ncs_util.h"
#include "avsv_eduutil.h"

#endif
