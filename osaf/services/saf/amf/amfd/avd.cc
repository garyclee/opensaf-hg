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

  This module is the main module for Availability Director. This module
  deals with the initialisation of AvD.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_init_proc - entry function to AvD thread or task.
  avd_initialize - creation and starting of AvD task/thread.
  avd_tmr_cl_init_func - the event handler for AMF cluster timer expiry.
  avd_destroy - the destruction of the AVD module.
  avd_lib_req - DL SE API for init and destroy of the AVD module

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <poll.h>
#include <saImmOm.h>
#include <immutil.h>
#include <logtrace.h>
#include <rda_papi.h>

#include <avd.h>
#include <avd_hlt.h>
#include <avd_imm.h>
#include <avd_clm.h>
#include <avd_cluster.h>
#include <avd_sutype.h>
#include <avd_si_dep.h>

