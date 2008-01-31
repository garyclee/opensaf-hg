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

  This module contains declarations related to target system Memory Mgmt
  services specifically used by the H&J patricia tree library

..............................................................................
*/

/*
 * Module Inclusion Control...
 */
#ifndef _SYSF_PAT_H_
#define _SYSF_PAT_H_


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 @
 @   MEMORY MANAGER PRIMITIVES (MACROS)...
 @   Populate accordingly during portation.
 @
 @
 @  The following macros provide the front-end to the target system memory
 @  manager. These macros must be changed such that the desired operation is
 @  accomplished by using the target system memory manager primitives.
 @
 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/**
 ** NOTES:
 ** The arguments used by the m_MMGR_* macros defined below are as follows:
 **     p - a pointer
 **/

/** Target defined macros for conventional release and allocation of
 ** data structures, link-lists, etc.
 **/
#define m_MMGR_ALLOC_PATRICIA_STACK(size) \
    (NCS_PATRICIA_LEXICAL_STACK *)m_NCS_MEM_ALLOC(size, NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_COMMON, 1)
#define m_MMGR_FREE_PATRICIA_STACK(p) \
                     m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_COMMON, 1)

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 @
 @    FUNCTION PROTOTYPES
 @
 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#endif
