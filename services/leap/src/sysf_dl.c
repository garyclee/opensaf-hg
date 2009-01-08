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

  MODULE NAME: SYSF_DL.C

.............................................................................

  DESCRIPTION:


  NOTES:

******************************************************************************
*/


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            H&J Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"
#include "ncs_dlprm.h"

#include "ncs_dl.h"


/*****************************************************************************

  Macro NAME:        SYSF_DL_REQUEST

  DESCRIPTION:       This function will be used by the DL
                  user to send DL control and data requests to the DL
                  stack.

  ARGUMENTS:
   ipr:           Pointer to a NCS_DL_REQUEST_INFO information block.

  RETURNS:
   NCSCC_RC_SUCCESS:  Success.
   NCSCC_RC_FAILURE:  Failure.

  NOTES:

*****************************************************************************/

uns32
sysf_dl_request (struct ncs_dl_request_info_tag *dlr)
{
   uns32 rc;     
   switch (dlr->i_request)
   {
   case NCS_DL_CTRL_REQUEST_BIND:

      rc = ncs_dl_bind_l2_layer (dlr);     
      break;

   case NCS_DL_CTRL_REQUEST_UNBIND:

      rc = ncs_dl_unbind_l2_layer (dlr);
      break;

   case NCS_DL_CTRL_REQUEST_OPEN:

      rc = ncs_dl_open_l2_layer (dlr);
      break;

   case NCS_DL_CTRL_REQUEST_CLOSE:
     
      rc = ncs_dl_close_l2_layer (dlr);
      break;

   case NCS_DL_DATA_REQUEST_SEND_DATA:
     
      rc = ncs_dl_send_data_to_l2_layer (dlr);
      break;

   case NCS_DL_CTRL_REQUEST_MULTICAST_JOIN:
     
      rc = ncs_dl_mcast_join (dlr);
      break;

   case NCS_DL_CTRL_REQUEST_MULTICAST_LEAVE:
 
      rc = ncs_dl_mcast_leave (dlr);
      break;
   /* Fix for IR10315 */
   case NCS_DL_CTRL_REQUEST_GET_ETH_ADDR:
      return (ncs_dl_getl2_eth_addr ((uns32 *)dlr->info.ctrl.get_eth_addr.io_if_index,\
                dlr->info.ctrl.get_eth_addr.io_if_name,\
                    dlr->info.ctrl.get_eth_addr.o_mac_addr));
      break;
   default:
     
      return NCSCC_RC_FAILURE;
   }

   if (rc != NCSCC_RC_SUCCESS)
       return m_LEAP_DBG_SINK(rc);

   return NCSCC_RC_SUCCESS;
}






