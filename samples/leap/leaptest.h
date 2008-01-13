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

  
  CONTENTS:


****************************************************************************/

#include "ncsgl_defs.h"
#include "ncs_osprm.h"

#ifndef SYSF_IP_LOG
#define SYSF_IP_LOG  0
#endif


#ifndef NCS_LT_DEBUG
#define NCS_LT_DEBUG  0
#endif

typedef int (*NCS_LT_TESTFUNCTION) (int argc, char **argv);

int lt_multicast(int argc, char **argv);
int lt_rawip(int argc, char **argv);
int lt_udp(int argc, char **argv);
int lt_tcp(int argc, char **argv);
int lt_memdiag(int argc, char **argv);
int lt_lockManager(int argc, char **argv);
int bufferManager_testSuite (int argc, char **argv);
int timerService_testSuite (int argc, char **argv);
int atomicCounting_testSuite (int argc, char **argv);
int lt_tcp_listen(int argc, char **argv);
int lt_timer_remaining(int argc, char **argv);
int lt_nexthopip(int argc, char **argv);
int lt_file_ops(int argc, char **argv);
int lt_processlib_ops(int argc, char **argv);
int lt_mailbox_ops(int argc, char **argv);
int lt_patricia_ops(int argc, char **argv);
int lt_encdec_ops(int argc, char **argv);
uns32 lt_dl_app_routine(int argument);

typedef enum
{
    NCS_LT_TEST_RC_SUCCESS = NCSCC_RC_SUCCESS,
    NCS_LT_TEST_RC_FAILURE = NCSCC_RC_FAILURE,
    NCS_LT_TEST_RC_INVALID_USAGE = NCSCC_RC_SUCCESS + NCSCC_RC_FAILURE,
    NCS_LT_TEST_RC_SENTINAL
} NCS_LT_TEST_RC;
