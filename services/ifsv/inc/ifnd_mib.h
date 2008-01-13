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
FILE NAME: IFND_MIB.H
DESCRIPTION: Function prototypes used for IfND used For MIB access.
******************************************************************************/

/* Function Prototype */
#ifndef IFND_MIB_H
#define IFND_MIB_H

uns32 ifnd_reg_with_miblib(void);
uns32 ifnd_unreg_with_mab(IFSV_CB *cb);
uns32 ifnd_reg_with_mab(IFSV_CB *cb);
uns32 ifnd_mib_tbl_req (struct ncsmib_arg *args);

#endif
