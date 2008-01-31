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
  MODULE NAME: IFSV_IR_SUBAGT_INIT.H 
******************************************************************************/

#ifndef _IFSV_IR_SUBAGT_INIT_H_
#define _IFSV_IR_SUBAGT_INIT_H_


#include "subagt_mab.h"
#include "subagt_oid_db.h"


/* Application specific common REGISTER routine */

extern uns32 subagt_register_ifsv_ir_subsys(void);

/* Application specific common UNREGISTER routine */

extern uns32 subagt_unregister_ifsv_ir_subsys(void);

#endif /* _IFSV_IR_SUBAGT_INIT_H_ */ 
  

