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

  $Header: /ncs/software/release/UltraLinq/MAB/MAB1.0/targsvcs/products/mab/mab_opt.h 4     8/23/01 11:53a Questk $

..............................................................................

  DESCRIPTION:


******************************************************************************
*/

#ifndef MAB_OPT_H
#define MAB_OPT_H


#include "ncs_opt.h"

/* By default only OAC code gets enabled if NCS_MAB is ON. NCS_MAS
   and NCS_MAC are OFF by default. Please see below.
*/
#ifndef NCS_MAS
/* MAS is disabled by default. To enable it, either NCS_MAS should be
   set on the command line or be setting NCS_MAS to 1 below
   (Base code does not make use of these flags yet : PHANI : 9/Mar/04)

*/
#define NCS_MAS 0 
#endif

#ifndef NCS_PSR
/* PSR is disabled by default. To enable it, either NCS_PSR should be
   set on the command line or be setting NCS_MAS to 1 below
   (Base code does not make use of these flags yet : PHANI : 9/Mar/04)

*/
#define NCS_PSR 0 
#endif

#ifndef NCS_MAC
/* MAC is disabled by default. To enable it, either NCS_MAS should be
   set on the command line or be setting NCS_MAS to 1 below
   (Base code does not make use of these flags yet : PHANI : 9/Mar/04)
*/
#define NCS_MAC 0 
#endif


/*
 * NCS_MAB_REV
 *************
 * The software revision level of MAB is occasionally needed by MAB. The 
 * following #define is used to identify the software revision level of 
 * NCS_MAB and is set at shipping time. Do not disturb.
 *    
 * The release is expressed as a 3-digit number;  for example, for MAB Rev 2.0, 
 * The #define should appear as:  #define NCS_MAB_REV 200
 */

#define NCS_MAB_REV 100  /* MAB Release 1.00 */


/* MAB_LOG 
 **********
 * This flag enables logging in all MAB subcomponents. To actually
 * enable Logging behavior, not only must this flag be turned on, but
 * each MAB subcomponent (MAC, MAS or OAC) must be enabled along the
 * logging dimensions desired.
 */

#ifndef MAB_LOG
#define MAB_LOG                 0
#endif


/* MAB_USE_LOCK_TYPE
 *********************
 * This flag describes which locking scheme you wish RMS to use. 
 * These values should not be changed. 
 * The options are:
 */

#define MAB_NO_LOCKS   1   /* all locks are disabled (no overhead; Safe?) */
#define MAB_TASK_LOCKS 2   /* As per LEAP docs m_INIT/START/END _CRITICAL */
#define MAB_OBJ_LOCKS  3   /* As per LEAP docs m_NCS_OBJ_LOCK scheme */

/* 
 * Set MAB_USE_LOCK_TYPE to the locking paradigm you want to use. 
 */

#ifndef NCSMAB_USE_LOCK_TYPE
#define NCSMAB_USE_LOCK_TYPE  MAB_NO_LOCKS
#endif


/* MAB_MIB_ID_HASH_TBL_SIZE
 **************************
 * This flag sets the size of the OAC and MAS hash tables used to store
 * internal information. There is no real operational segnificance other
 * than lookup speed. This is the Management plane, so this is not 
 * percieved to be a critical path in most systems.
 */

#define MAB_MIB_ID_HASH_TBL_SIZE  30

/* 
 * MAB_DEBUG
 ***********
 * If MAB_DEBUG == 1, and there is a non-SUCCESS return value from a function
 * then the macro m_MAB_DBG_SINK is invoked (see file rms_tgt.h). The default
 * implementation of this function is to print the file and line where the failure
 * occurred out to the console.
 * A customer can put a breakpoint there or leave the CONSOLE print statement
 * alone according to good development strategy, such as exception logging.
 *
 */

#ifndef MAB_DEBUG
#define MAB_DEBUG                  0
#endif

#ifndef MAB_TRACE
#define MAB_TRACE                  0
#endif

#ifndef MAB_TRACE2
#define MAB_TRACE2                 0
#endif


#endif /* #ifndef MAB_OPT_H */


