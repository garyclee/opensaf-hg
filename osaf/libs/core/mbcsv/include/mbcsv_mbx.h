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
          
   DESCRIPTION:
            
   This module contains the MBCSv MBX list structure and the corresponding functions.
              
*******************************************************************************/

/*
* Module Inclusion Control...
*/
#ifndef MBCSV_MBX_H
#define MBCSV_MBX_H

#include "mbcsv.h"

/***********************************************************************************@
*
*                        MBCSv MDS registration list
*
*    MBCSv maintains the registration information with MDS in a separate data 
*    structure. This data structure will get updated with each open and close call. 
*    MBCSv will need this data structure to find out number of user registered on 
*    the particular PWE.  
***********************************************************************************/

/*
 * Related function prototypes.
 */

uns32 mbcsv_add_new_mbx(uns32 pwe_hdl, SS_SVC_ID svc_id, SYSF_MBX mbx);
uns32 mbcsv_rmv_entry(uns32 pwe_hdl, SS_SVC_ID svc_id);
SYSF_MBX mbcsv_get_mbx(uns32 pwe_hdl, SS_SVC_ID svc_id);
SYSF_MBX mbcsv_get_next_entry_for_pwe(uns32 pwe_hdl, SS_SVC_ID *svc_id);
uns32 mbcsv_initialize_mbx_list(void);
uns32 mbcsv_destroy_mbx_list(void);

#endif
