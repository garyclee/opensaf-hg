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

  DESCRIPTION:This module does the initialisation of HPI.


..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_hpi_init - initializes HPI


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"


/****************************************************************************
  Name          : avd_hpi_init
 
  Description   : This routine initializes HPI and retrivies the
  node address and the node id of the node on which this AVD is
  running. 
 
  Arguments     : cb - ptr to the AVD control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_hpi_init (AVD_CL_CB *cb)
{

   return NCSCC_RC_SUCCESS;
}
