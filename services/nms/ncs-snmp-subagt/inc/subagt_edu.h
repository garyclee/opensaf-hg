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

  
  .....................................................................  
  
  DESCRIPTION: This file describes the Encode/Decode Utility routines 
  
  ***************************************************************************/ 
#ifndef SUBAGT_EDU_H
#define SUBAGT_EDU_H

struct ncs_trap;

/* EDU EDP initialization */ 
uns32
snmpsubagt_edu_edp_initialize(EDU_HDL *); 

/* decode the trap from the event data */
uns32 
subagt_edu_ncs_trap_decode(EDU_HDL  *edu_hdl, 
                           uns8     *evt_data,
                           uns32    evt_data_size, 
                           struct ncs_trap *trap_header);
#endif
