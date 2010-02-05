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

  REVISION HISTORY:

..............................................................................

  DESCRIPTION:

******************************************************************************
*/

#ifndef DTS_OPT_H
#define DTS_OPT_H


/*
 * NCS_DTS_REV
 *************
 * The software revision level of DTS is occasionally needed by DTS. The 
 * following #define is used to identify the software revision level of 
 * NCS_DTS and is set at shipping time. Do not disturb.
 *    
 * The release is expressed as a 3-digit number;  for example, for DTS Rev 2.0, 
 * The #define should appear as:  #define NCS_DTS_REV 200
 */

#define NCS_DTS_REV 100		/* DTS Release 1.00 */

/* DTS_LOG 
 **********
 * This flag enables logging in all DTS subcomponents. To actually
 * enable Logging behavior, not only must this flag be turned on, but
 * each DTS subcomponent must be enabled along the
 * logging dimensions desired.
 */

#ifndef DTS_LOG
#define DTS_LOG                 0
#endif

/* DTS_USE_LOCK_TYPE
 *********************
 * This flag describes which locking scheme you wish RMS to use. 
 * These values should not be changed. 
 * The options are:
 */

#define DTS_NO_LOCKS   1	/* all locks are disabled (no overhead; Safe?) */
#define DTS_TASK_LOCKS 2	/* As per LEAP docs m_INIT/START/END _CRITICAL */
#define DTS_OBJ_LOCKS  3	/* As per LEAP docs m_NCS_OBJ_LOCK scheme */

/* 
 * Set DTS_USE_LOCK_TYPE to the locking paradigm you want to use. 
 */

#ifndef NCSDTS_USE_LOCK_TYPE
#define NCSDTS_USE_LOCK_TYPE  DTS_OBJ_LOCKS
#endif

/* 
 * DTS_DEBUG
 ***********
 * If DTS_DEBUG == 1, and there is a non-SUCCESS return value from a function
 * then the macro m_DTS_DBG_SINK is invoked (see file rms_tgt.h). The default
 * implementation of this function is to print the file and line where the failure
 * occurred out to the console.
 * A customer can put a breakpoint there or leave the CONSOLE print statement
 * alone according to good development strategy, such as exception logging.
 *
 */

#ifndef DTS_DEBUG
#define DTS_DEBUG                  0
#endif

#ifndef DTS_TRACE
#define DTS_TRACE                  0
#endif

#ifndef DTS_TRACE2
#define DTS_TRACE2                 0
#endif

#endif   /* #ifndef DTS_OPT_H */
