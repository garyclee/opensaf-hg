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

#include "fm.h"


/****************************************************************************
 * Name          : fm_rda_init 
 *
 * Description   : Initializes RDA interface, Get a role and Register Callback. 
 *
 * Arguments     : Pointer to Control Block 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
uns32 fm_rda_init(FM_CB *fm_cb)
{
   uns32       rc;
   uns32       status = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ rda_req;

   /* initialize the RDA Library */
   memset(&rda_req, 0, sizeof(PCS_RDA_REQ));
   rda_req.req_type = PCS_RDA_LIB_INIT;
   rc = pcs_rda_request (&rda_req);
   if (rc  != PCSRDA_RC_SUCCESS)
   {
      m_NCS_SYSLOG(NCS_LOG_INFO,"RDA lib init failed \n");
      return NCSCC_RC_FAILURE;
   }

   /* get the role */
   memset(&rda_req, 0, sizeof(PCS_RDA_REQ));
   rda_req.req_type = PCS_RDA_GET_ROLE;
   rc = pcs_rda_request(&rda_req);
   if (rc != PCSRDA_RC_SUCCESS)
   {
      /* set the error code to be returned */
      status = NCSCC_RC_FAILURE;
      /* finalize */
      m_NCS_SYSLOG(NCS_LOG_INFO,"RDA get role failed \n");
      goto rda_lib_destroy;
   }

   /* update role in fm_cb */
   if ((rda_req.info.io_role == PCS_RDA_ACTIVE) || 
       (rda_req.info.io_role == PCS_RDA_STANDBY))
   {
      fm_cb->role = rda_req.info.io_role;
   }
   else
   {
      /* set the error code to be returned */
      status = NCSCC_RC_FAILURE;
      m_NCS_SYSLOG(NCS_LOG_INFO,"RDA role is neither Active nor Standby \n");
      goto rda_lib_destroy;
   }

   /* subscribe callback */
   memset(&rda_req, 0, sizeof(PCS_RDA_REQ));
   rda_req.req_type = PCS_RDA_REGISTER_CALLBACK;
   rda_req.callback_handle = gl_fm_hdl;
   rda_req.info.call_back = fm_rda_callback;
   rc = pcs_rda_request(&rda_req);
   if (rc != PCSRDA_RC_SUCCESS)
   {
      /* set the error code to be returned */
      status = NCSCC_RC_FAILURE;
      m_NCS_SYSLOG(NCS_LOG_INFO,"RDA callback subscription failed \n");
      /* finalize */
      goto rda_lib_destroy;
   }

   return status;

   /* finalize the library */
rda_lib_destroy:
   m_NCS_SYSLOG(NCS_LOG_INFO,"RDA lib destroy called \n");
   memset(&rda_req, 0, sizeof(PCS_RDA_REQ));
   rda_req.req_type = PCS_RDA_LIB_DESTROY;
   rc = pcs_rda_request(&rda_req);
   if (rc != PCSRDA_RC_SUCCESS)
   {
      m_NCS_SYSLOG(NCS_LOG_INFO,"RDA lib destroy failed in fm_rda_init \n");
      return NCSCC_RC_FAILURE;
   }

   /* return the final status */
   return status;
}


/****************************************************************************
 * Name          : fm_rda_finalize 
 *
 * Description   : Finalizes RDA interfaces.
 *
 * Arguments     : Pointer to Control Block. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
uns32 fm_rda_finalize(FM_CB *fm_cb)
{
   uns32       rc;
   uns32       status = NCSCC_RC_SUCCESS;
   PCS_RDA_REQ rda_req;

   memset(&rda_req, 0, sizeof(PCS_RDA_REQ));
   rda_req.req_type = PCS_RDA_LIB_DESTROY;
   rc = pcs_rda_request(&rda_req);
   if (rc != PCSRDA_RC_SUCCESS)
   {
      m_NCS_SYSLOG(NCS_LOG_INFO,"RDA lib destroy failed in fm_rda_finalize \n");
      status = NCSCC_RC_FAILURE;
   }

   return status;
}


/****************************************************************************
 * Name          : fm_rda_callback
 *
 * Description   :  Callback for role change indication.
 *
 * Arguments     :  Callback Handle, Pointer to Callback information, Error Code.
 *
 * Return Values : None.
 * 
 * Notes         : None. 
 *****************************************************************************/
void fm_rda_callback(uns32 cb_hdl, PCS_RDA_CB_INFO *cb_info, PCSRDA_RETURN_CODE error_code)
{
   FM_CB *fm_cb = NULL;

   /* Take handle */
   fm_cb = ncshm_take_hdl(NCS_SERVICE_ID_GFM, cb_hdl);

   if(cb_info->cb_type == PCS_RDA_ROLE_CHG_IND)
   {
      m_NCS_SYSLOG(NCS_LOG_INFO,
            "fm_rda_callback(): CurrentState: %s, NewState: %s\n", 
            role_string[fm_cb->role], role_string[cb_info->info.io_role]);
      if ((cb_info->info.io_role == PCS_RDA_ACTIVE) || 
          (cb_info->info.io_role == PCS_RDA_STANDBY))
      {
         /* Update local role */
         fm_cb->role = cb_info->info.io_role;
      }
   }

   /* Release handle */
   ncshm_give_hdl(cb_hdl);
   fm_cb = NULL;

   return;
}


/****************************************************************************
 * Name          : fm_rda_set_role
 *
 * Description   : Sends role set request.
 *
 * Arguments     :  Pointer to Control block and role.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
uns32 fm_rda_set_role(FM_CB *fm_cb, PCS_RDA_ROLE role)
{
   PCS_RDA_REQ rda_req;
   uns32 rc;

   /* set the RDA role to active */
   memset(&rda_req, 0, sizeof(PCS_RDA_REQ));
   rda_req.req_type = PCS_RDA_SET_ROLE;
   rda_req.info.io_role = role;

   rc = pcs_rda_request(&rda_req);
   if (rc != PCSRDA_RC_SUCCESS)
   {
      m_NCS_SYSLOG(NCS_LOG_INFO,
            "fm_rda_set_role() Failed: CurrentState: %d, AskedState: %d\n", 
            role_string[fm_cb->role], role_string[role]);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_SYSLOG(NCS_LOG_INFO,
            "fm_rda_set_role() Success: CurrentState: %d, AskedState: %d\n", 
            role_string[fm_cb->role], role_string[role]);

   return NCSCC_RC_SUCCESS;
}

