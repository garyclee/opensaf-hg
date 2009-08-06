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

******************************************************************************
*/

#ifndef MDS_PVT_H
#define MDS_PVT_H
#include "ncsgl_defs.h"

/****************************************************************************
 *
 * Function Name: mds_node_link_reset
 *
 * Purpose: To reset the TIPC link tolerance <self_node-peer_node>to the 
 *           default value of 1.5 sec. peer_node is the node_id passed
 *
 * Return Value:  NCSCC_RC_SUCCESS On Success
 *                NCSCC_RC_FAILURE on failure
 *
 ****************************************************************************/
uns32 mds_node_link_reset(NCS_NODE_ID node_id);

#endif
