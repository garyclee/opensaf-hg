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

  MODULE NAME: SNMPTM_TBLID.H


..............................................................................

  DESCRIPTION:  This file list out the table-id enums used for SNMPTM demo.

******************************************************************************
*/
#ifndef SNMPTM_TBLID_H
#define SNMPTM_TBLID_H

/* Enum definitions for SNMPTM table IDs 
 * ensure no table IDs are defined beyond MIB_UD_TBL_ID_END
 * which is defined in ncs_mtbl.h.
 */
#define SNMPTM_MIB_TBL_ID_START  (MIB_UD_TBL_ID_END - 20)
 
typedef enum 
{
   NCSMIB_TBL_SNMPTM_SCALAR = SNMPTM_MIB_TBL_ID_START,
   NCSMIB_TBL_SNMPTM_TBLONE,
   NCSMIB_TBL_SNMPTM_TBLTWO,
   NCSMIB_TBL_SNMPTM_TBLTHREE,
   NCSMIB_TBL_SNMPTM_TBLFOUR,
   NCSMIB_TBL_SNMPTM_TBLFIVE,
   NCSMIB_TBL_SNMPTM_TBLSIX, /* SP-MIB */
   NCSMIB_TBL_SNMPTM_TBLSEVEN, 
   NCSMIB_TBL_SNMPTM_TBLEIGHT, 
   NCSMIB_TBL_SNMPTM_TBLNINE, 
   NCSMIB_TBL_SNMPTM_TBLTEN, 
   NCSMIB_TBL_SNMPTM_NOTIF,
   NCSMIB_TBL_SNMPTM_COMMAND
} SNMPTM_TBL_ID;

#endif

