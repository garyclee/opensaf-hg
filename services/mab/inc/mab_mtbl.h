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

  This module is where users can add their MIB Table ID Definitions that are
  going to participate in MAB functionality.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#ifndef MAB_MTBL_H
#define MAB_MTBL_H

/*****************************************************************************

  N e t P l a n e  M I B   A c c e s s   B r o k e r
  
  U s e r  D e f i n e d   M I B   T a b l e   I D s
  
    
    TO ADD A User Defined Service:
    =============================
    Replace mnemonic MIB_UD_TBL_ID_STUB_1 to a TBL ID name that suits you.
    Replace mnemonic MIB_UD_TBL_ID_STUB_2 to another TBL ID name that suits you.
    etc.

    Software Upgrade Issue
    ======================
    This scheme 'compresses' the name space of MIB TBL IDs that exist in MAB by
    guarenteeing that all names are contiguous. In fact, MAB hashes entries 
    based on TBL IDs rather than setting up per-table 'buckets', so 'contiguous'
    and 'compressed' to date, are not particularly important design/performance
    conditions.

    However, Given this scheme, a TBL ID name can change between releases as 
    NetPlane continues to populate its name space, pushing the user's name 'up'. 
    This can be an issue in some software upgrade scenerios 
    
      NOTE: There are lots of 'compatability' issues to solve in a software
      upgrade case. Here we are only drawing attention to one of many such
      issues.
      
    Several solutions are possible, all revolving around the proposition that
    MIB TBL ID names must remain the same across releases:

    1 - comment out new NetPlane names in ncs_mtbl.h that show up between
        releases.
    2 - Add some fake PADDING service names between the last NetPlane and
        first customer name in anticipation of further NetPlane growth.
    3 - Start your MIB TBL ID assignments way outside NetPlane's set. (This is
        a variation of 2).


  NOTE: The mnemonic NCSMIB_TBL_END is the current last OpenSAF MIB
        Table ID, which is found at the end of the MIB table enums in file
        ncs_mtbl.h.
******************************************************************************/

typedef enum mib_ud_tbl_id {
  
  MIB_UD_TBL_ID_STUB_0  = NCSMIB_TBL_END , /* Last OpenSAF MIB ID + 1 */

  MIB_UD_TBL_ID_STUB_1,
  MIB_UD_TBL_ID_STUB_2,
  MIB_UD_TBL_ID_STUB_3,
  MIB_UD_TBL_ID_STUB_4,
  MIB_UD_TBL_ID_STUB_5,


  /* This field is reserved and not to be touched. All customer defined mib table ids
   * should be added between NCSMIB_TBL_END and NCSMIB_UD_TBL_ID_END
   */

  NCSMIB_UD_TBL_ID_END
  } MIB_UD_TBL_ID;
  

#endif
                   
