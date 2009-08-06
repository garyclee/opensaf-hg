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

#include <bam.h>

extern uns32
ncs_bam_rda_reg(NCS_BAM_CB *bam_cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ pcs_rda_req;

   pcs_rda_req.req_type = PCS_RDA_LIB_INIT;
   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      return NCSCC_RC_FAILURE;
   }
   
   pcs_rda_req.req_type = PCS_RDA_GET_ROLE;
   if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req))
   {
      return NCSCC_RC_FAILURE;
   }

   switch(pcs_rda_req.info.io_role)
   {
      case PCS_RDA_ACTIVE:
      {
         bam_cb->ha_state = SA_AMF_HA_ACTIVE;
      }
      break;
   
      case PCS_RDA_STANDBY:
      {
         bam_cb->ha_state = SA_AMF_HA_STANDBY;
      }
      break;
    
      default:
      {
         rc = NCSCC_RC_FAILURE;
         break;
      }
  }
  
  return rc;
}
