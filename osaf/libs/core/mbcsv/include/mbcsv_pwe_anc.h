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
#ifndef MBCSV_PWE_ANC_H
#define MBCSV_PWE_ANC_H

#include "mbcsv.h"

/*
 * Prototypes of PWE anchor.
 */
uns32 mbcsv_destroy_peer_list(void);
uns32 mbcsv_add_new_pwe_anc(uns32 pwe_hdl, MBCSV_ANCHOR anchor);
uns32 mbcsv_rmv_pwe_anc_entry(uns32 pwe_hdl, MBCSV_ANCHOR anchor);
uns32 mbcsv_get_next_anchor_for_pwe(uns32 pwe_hdl, MBCSV_ANCHOR *anchor);
uns32 mbcsv_send_brodcast_msg(uns32 pwe_hdl, MBCSV_EVT *msg, CKPT_INST *ckpt);
uns32 mbcsv_rmv_ancs_for_pwe(uns32 pwe_hdl);
uns32 mbcsv_initialize_peer_list(void);

#endif
