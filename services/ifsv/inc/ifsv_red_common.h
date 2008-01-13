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

#ifndef IFSV_RED_COMMON_H
#define IFSV_RED_COMMON_H

/*****************************************************************************
 * This file contains common data structures, enums, #defines, function
 * prototypes, needed for IfSV, used for both IfND and IfD.
*****************************************************************************/
/*****************************************************************************
 * This is the states of IfND during restart.
 ****************************************************************************/
typedef enum ifnd_restart_state
{
   NCS_IFSV_IFND_INIT_DONE_STATE_BASE = 1,
   NCS_IFSV_IFND_INIT_DONE_STATE = NCS_IFSV_IFND_INIT_DONE_STATE_BASE,
   NCS_IFSV_IFND_MDS_DEST_GET_STATE,
   NCS_IFSV_IFND_DATA_RETRIVAL_STATE,
   NCS_IFSV_IFND_OPERATIONAL_STATE,
   NCS_IFSV_IFND_INIT_DONE_STATE_MAX
}IFND_RESTART_STATE;

#endif
