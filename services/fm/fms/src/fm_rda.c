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

  $$


  MODULE NAME: FM
  AUTHOR: vishal soni (vishal.soni@emerson.com)

..............................................................................

  DESCRIPTION:

  This file contains the RDA routine for FM.


******************************************************************************/

#include "fm.h"

uns32 fm_rda_init(FM_CB *);
uns32 fm_rda_finalize(FM_CB *);
void fm_rda_callback(uns32, PCS_RDA_CB_INFO *, PCSRDA_RETURN_CODE);
uns32 fm_rda_set_role(FM_CB *, PCS_RDA_ROLE);


uns32 fm_rda_init(FM_CB *fm_cb)
{
    uns32       rc;
    uns32       status = NCSCC_RC_SUCCESS;
    PCS_RDA_REQ rda_req;

    /* initialize the RDA Library */
    m_NCS_MEMSET(&rda_req, 0, sizeof(PCS_RDA_REQ));
    rda_req.req_type = PCS_RDA_LIB_INIT;
    rc = pcs_rda_request (&rda_req);
    if (rc  != PCSRDA_RC_SUCCESS)
    {
                m_NCS_SYSLOG(NCS_LOG_INFO,"RDA lib init failed \n");
        return NCSCC_RC_FAILURE;
    }

    /* get the role */
    m_NCS_MEMSET(&rda_req, 0, sizeof(PCS_RDA_REQ));
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
    if (rda_req.info.io_role == PCS_RDA_ACTIVE || rda_req.info.io_role == PCS_RDA_STANDBY)
                fm_cb->role = rda_req.info.io_role;
    else
    {
        /* set the error code to be returned */
        status = NCSCC_RC_FAILURE;
                m_NCS_SYSLOG(NCS_LOG_INFO,"RDA role is neither Active nor Standby \n");
        goto rda_lib_destroy;
    }

    /* subscribe callback */
    m_NCS_MEMSET(&rda_req, 0, sizeof(PCS_RDA_REQ));
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
    m_NCS_MEMSET(&rda_req, 0, sizeof(PCS_RDA_REQ));
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



uns32 fm_rda_finalize(FM_CB *fm_cb)
{
    
        uns32       rc;
    uns32       status = NCSCC_RC_SUCCESS;
    PCS_RDA_REQ rda_req;

/* TODO : unregister of call back required if we directly finalize? */

    m_NCS_MEMSET(&rda_req, 0, sizeof(PCS_RDA_REQ));
    rda_req.req_type = PCS_RDA_LIB_DESTROY;
    rc = pcs_rda_request(&rda_req);
    if (rc != PCSRDA_RC_SUCCESS)
    {
                m_NCS_SYSLOG(NCS_LOG_INFO,"RDA lib destroy failed in fm_rda_finalize \n");
        status = NCSCC_RC_FAILURE;
    }

        return status;
}


void fm_rda_callback(uns32 cb_hdl, PCS_RDA_CB_INFO *cb_info, PCSRDA_RETURN_CODE error_code)
{

        FM_CB *fm_cb = NULL;

        /* Take handle */
        fm_cb = ncshm_take_hdl(NCS_SERVICE_ID_GFM, cb_hdl);

        if(cb_info->cb_type == PCS_RDA_ROLE_CHG_IND)
        {
           m_NCS_SYSLOG(NCS_LOG_INFO,
            "fm_rda_callback(): CurrentState: %d, NewState: %d\n", 
            fm_cb->role, cb_info->info.io_role);
           if (cb_info->info.io_role == PCS_RDA_ACTIVE || cb_info->info.io_role == PCS_RDA_STANDBY)
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

uns32 fm_rda_set_role(FM_CB *fm_cb, PCS_RDA_ROLE role)
{
    PCS_RDA_REQ rda_req;
    uns32 rc;

    /* set the RDA role to active */
    m_NCS_MEMSET(&rda_req, 0, sizeof(PCS_RDA_REQ));
    rda_req.req_type = PCS_RDA_SET_ROLE;
    rda_req.info.io_role = role;

    rc = pcs_rda_request(&rda_req);
    if (rc == PCSRDA_RC_SUCCESS)
    {
        m_NCS_SYSLOG(NCS_LOG_INFO,
            "fm_rda_set_role() Success: CurrentState: %d, AskedState: %d\n", 
            fm_cb->role, role);
        return NCSCC_RC_SUCCESS;
    }
    else
    {
        m_NCS_SYSLOG(NCS_LOG_INFO,
            "fm_rda_set_role() Failed: CurrentState: %d, AskedState: %d\n", 
            fm_cb->role, role);
        return NCSCC_RC_FAILURE;
    }
}

