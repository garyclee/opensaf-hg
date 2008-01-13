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
 
#ifndef NCS_POSIX_TMR_H
#define NCS_POSIX_TMR_H

#if (NCS_POSIX_TMR == 1)

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <time.h>
#include <sys/signal.h>
#include <semaphore.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define NCS_MAX_TMRS 64

typedef void (*ncs_ptmr_notify_t)();

int ncs_ptmr_create(ncs_ptmr_notify_t cb);
int ncs_ptmr_settime(timer_t timerid, const struct itimerspec *timeout);
int ncs_ptmr_stop(timer_t timerid);
int ncs_ptmr_delete(timer_t timerid);

#ifdef  __cplusplus
}
#endif

#endif

#endif /*NCS_POSIX_TMR*/


