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
  MODULE NAME: SNMPTM_BGN.C

..............................................................................

  DESCRIPTION:   SNMPTM demo file, contains the procedures to initialise MDS,
                 OAC to make an SNMPTM TEST_MIB appl as a stand alone system.
******************************************************************************
*/

#include "snmptm.h"

/****************************************************************************
 Function Name:  main

 Purpose      :  call consoleMain. This allows the demo to work for pSOS as 
                 well since it has its own main (which will call this
                 consoleMain). 
                             
 Arguments    : 

 Return Value :  None
****************************************************************************/
#ifndef _NCS_IGNORE_MAIN_
int main(int argc, char **argv)
{
    snmptm_cli_main(argc, argv);
    return NCSCC_RC_SUCCESS;
}
#endif

