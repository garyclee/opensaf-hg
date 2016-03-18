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
 * Author(s): Ericsson AB
 *
 */

/*****************************************************************************
..............................................................................
..............................................................................

  DESCRIPTION:

  This module is the main include file for IMM Node Director (IMMND).

*****************************************************************************/

#ifndef IMMND_H
#define IMMND_H

#include "immsv.h"
#include "immnd_cb.h"
#include "immnd_init.h"
#include "osaf_time.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_IMM_PBE
#define ENABLE_PBE 1
#else
#define ENABLE_PBE 0
#endif


/*
 * Get the time in seconds
 *
 * This is a wrapper function to get the clocktime in seconds.
 * Output vlaue will be in seconds.
 */

static inline void osaf_clock_gettime_sec(clockid_t i_clk_id,
        time_t* o_t)
{
       struct timespec o_ts;
       osaf_clock_gettime(i_clk_id, &o_ts) ;
       *o_t = o_ts.tv_sec;
}

#endif   /* IMMND_H */
