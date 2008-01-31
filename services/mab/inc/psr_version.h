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

  DESCRIPTION:

  This file contains the version related macros for PSSv  

 ****************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef PSR_VERSION_H
#define PSR_VERSION_H

/* macro to describe the version of the PSSv and its persistent store format */ 
#define m_PSS_SERVICE_VERSION 1
#define PSS_MBCSV_VERSION     2 /* Incremented as Cold-sync does not include pssv_lib_conf contents */
#define PSS_MBCSV_VERSION_MIN 1
#define PSS_PS_FORMAT_VERSION 2

/* 
 * default version for each of the table that PSSv sets for each of 
 * the application's table
 */
#define m_PSS_DEF_TBL_VERSION 1

#define PSS_MDS_SUB_PART_VERSION  1 

#define PSS_WRT_MAC_SUBPART_VER_MIN    1
#define PSS_WRT_MAC_SUBPART_VER_MAX    1

#define  PSS_WRT_MAC_SUBPART_VER_RANGE     \
         (PSS_WRT_MAC_SUBPART_VER_MAX  -   \
          PSS_WRT_MAC_SUBPART_VER_MIN + 1) 

#define PSS_WRT_OAC_SUBPART_VER_MIN    1
#define PSS_WRT_OAC_SUBPART_VER_MAX    1

#define  PSS_WRT_OAC_SUBPART_VER_RANGE     \
         (PSS_WRT_OAC_SUBPART_VER_MAX  -   \
          PSS_WRT_OAC_SUBPART_VER_MIN + 1) 

#define PSS_WRT_BAM_SUBPART_VER_MIN    1
#define PSS_WRT_BAM_SUBPART_VER_MAX    1

#define  PSS_WRT_BAM_SUBPART_VER_RANGE     \
         (PSS_WRT_BAM_SUBPART_VER_MAX  -   \
          PSS_WRT_BAM_SUBPART_VER_MIN + 1) 

#endif
