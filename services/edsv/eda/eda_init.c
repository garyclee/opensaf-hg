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

  This file contains the initialization and destroy routines for EDA library.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "eda.h"

/* global cb handle */
uns32 gl_eda_hdl = 0;


/****************************************************************************
  Name          : ncs_eda_lib_req
 
  Description   : This routine is exported to the external entities & is used
                  to create & destroy the EDA library.
 
  Arguments     : req_info - ptr to the request info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 ncs_eda_lib_req (NCS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   switch (req_info->i_op)
   {
   case NCS_LIB_REQ_CREATE:
      rc =  eda_create(&req_info->info.create);
      break;
       
   case NCS_LIB_REQ_DESTROY:
      eda_destroy(&req_info->info.destroy);
      break;
       
   default:
      break;
   }

   return rc;
}

/****************************************************************************
  Name          : eda_create
 
  Description   : This routine creates & initializes the EDA control block.
 
  Arguments     : create_info - ptr to the create info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 eda_create (NCS_LIB_CREATE *create_info)
{
   EDA_CB *cb = 0;
   uns32  rc = NCSCC_RC_SUCCESS;

   if (NULL == create_info)
      return NCSCC_RC_FAILURE;

   /* Register with the Logging subsystem */
   eda_flx_log_reg();

   /* allocate EDA cb */
   if ( NULL == (cb = m_MMGR_ALLOC_EDA_CB))
   {
      m_LOG_EDSV_A(EDA_MEMALLOC_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      rc = NCSCC_RC_FAILURE;
      goto error;
   }

   m_NCS_OS_MEMSET(cb, 0, sizeof(EDA_CB));

   /* assign the EDA pool-id (used by hdl-mngr) */
   cb->pool_id = NCS_HM_POOL_ID_COMMON;

   /* create the association with hdl-mngr */
   if ( 0 == (cb->cb_hdl = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_EDA, 
                                        (NCSCONTEXT)cb)) )
   {
      m_LOG_EDSV_A(EDA_CB_HDL_CREATE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      rc = NCSCC_RC_FAILURE;
      goto error;
   }

   /* get the process id */
   cb->prc_id = m_NCS_OS_PROCESS_GET_ID();

   /* initialize the eda cb lock */
   m_NCS_LOCK_INIT(&cb->cb_lock);
   m_NCS_LOCK_INIT(&cb->eds_sync_lock);

   /* Store the cb hdl in the global variable */
   gl_eda_hdl = cb->cb_hdl;

   /* register with MDS */
   if ( (NCSCC_RC_SUCCESS != (rc = eda_mds_init(cb))))
   {
      m_LOG_EDSV_A(EDA_MDS_INIT_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      rc = NCSCC_RC_FAILURE;
      goto error;
   }
   
   eda_sync_with_eds(cb);
   return rc;

error:
   if (cb)
   {
      /* remove the association with hdl-mngr */
      if (cb->cb_hdl)
         ncshm_destroy_hdl(NCS_SERVICE_ID_EDA, cb->cb_hdl);

      /* delete the eda init instances */
      eda_hdl_list_del(&cb->eda_init_rec_list);

      /* destroy the lock */
      m_NCS_LOCK_DESTROY(&cb->cb_lock);

      /* free the control block */
      m_MMGR_FREE_EDA_CB(cb);
   }
   return rc;
}


/****************************************************************************
  Name          : eda_destroy
 
  Description   : This routine destroys the EDA control block.
 
  Arguments     : destroy_info - ptr to the destroy info
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void eda_destroy (NCS_LIB_DESTROY *destroy_info)
{
   EDA_CB *cb = 0;
   
   /* retrieve EDA CB */
   cb = (EDA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_EDA, gl_eda_hdl);
   if(!cb)
   {
      m_LOG_EDSV_A(EDA_CB_HDL_TAKE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      return;
   }
   /* delete the hdl db */
   eda_hdl_list_del (&cb->eda_init_rec_list);

   /* unregister with MDS */
   eda_mds_finalize (cb);
   
   /* destroy the lock */
   m_NCS_LOCK_DESTROY(&cb->cb_lock);

   /* de register with the flex log */
   eda_flx_log_dereg();
   
   /* return EDA CB */
   ncshm_give_hdl(gl_eda_hdl);

   /* remove the association with hdl-mngr */
   ncshm_destroy_hdl(NCS_SERVICE_ID_EDA, cb->cb_hdl);
   
   /* free the control block */
   m_MMGR_FREE_EDA_CB(cb);

   /* reset the global cb handle */
   gl_eda_hdl = 0;

   return;
}
