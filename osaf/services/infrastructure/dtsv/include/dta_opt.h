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

#ifndef DTA_OPT_H
#define DTA_OPT_H


/*
 * NCS_DTA_REV
 *************
 * The software revision level of DTA is occasionally needed by DTA. The 
 * following #define is used to identify the software revision level of 
 * NCS_DTA and is set at shipping time. Do not disturb.
 *    
 * The release is expressed as a 3-digit number;  for example, for DTA Rev 2.0, 
 * The #define should appear as:  #define NCS_DTA_REV 200
 */

#define NCS_DTA_REV 100		/* DTA Release 1.00 */

/* DTA_LOG 
 **********
 * This flag enables logging in all DTA subcomponents. To actually
 * enable Logging behavior, not only must this flag be turned on, but
 * each DTA subcomponent must be enabled along the
 * logging dimensions desired.
 */

#ifndef DTA_LOG
#define DTA_LOG                 0
#endif

/* DTA_USE_LOCK_TYPE
 *********************
 * This flag describes which locking scheme you wish RMS to use. 
 * These values should not be changed. 
 * The options are:
 */

#define DTA_NO_LOCKS   1	/* all locks are disabled (no overhead; Safe?) */
#define DTA_TASK_LOCKS 2	/* As per LEAP docs m_INIT/START/END _CRITICAL */
#define DTA_OBJ_LOCKS  3	/* As per LEAP docs m_NCS_OBJ_LOCK scheme */

/* 
 * Set DTA_USE_LOCK_TYPE to the locking paradigm you want to use. 
 */

#ifndef NCSDTA_USE_LOCK_TYPE
#define NCSDTA_USE_LOCK_TYPE  DTA_OBJ_LOCKS
#endif

/* 
 * DTA_DEBUG
 ***********
 * If DTA_DEBUG == 1, and there is a non-SUCCESS return value from a function
 * then the macro m_DTA_DBG_SINK is invoked (see file rms_tgt.h). The default
 * implementation of this function is to print the file and line where the failure
 * occurred out to the console.
 * A customer can put a breakpoint there or leave the CONSOLE print statement
 * alone according to good development strategy, such as exception logging.
 *
 */

#ifndef DTA_DEBUG
#define DTA_DEBUG                  0
#endif

#endif   /* #ifndef DTA_OPT_H */
