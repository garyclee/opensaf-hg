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
*                                                                            *
*  MODULE NAME:  hpl_init.c                                                  *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the routines used to intialize or destroy the HPI    *
*  Private Library.                                                          *
*                                                                            *
*****************************************************************************/

#include "hpl.h"

/* global cb handle */
uns32 gl_hpl_hdl = 0;


/****************************************************************************
 * Name          : hpl_initialize
 *
 * Description   : This function initializes the HPL library. It creates
 *                 HPL control block and initializes it. It registers to
 *                 MDS with an absolute address.
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_initialize(NCS_LIB_CREATE *create_info)
{
   HPL_CB *hpl_cb = NULL;
   uns32  rc = NCSCC_RC_SUCCESS;

   /* allocate HPL cb */
   if ( NULL == (hpl_cb = m_MMGR_ALLOC_HPL_CB))
   {
      /* log */
      rc = NCSCC_RC_FAILURE;
      goto error;
   }
   memset(hpl_cb, 0, sizeof(HPL_CB));

   /* assign the HPL pool-id (used by hdl-mngr) */
   hpl_cb->pool_id = NCS_HM_POOL_ID_COMMON;

   /* create the association with hdl-mngr */
   if (0 == (hpl_cb->cb_hdl = ncshm_create_hdl(hpl_cb->pool_id, NCS_SERVICE_ID_HPL,
                                        (NCSCONTEXT)hpl_cb)) )
   {
      /* log */
      rc = NCSCC_RC_FAILURE;
      goto error;
   }
   /* store the cb hdl in the global variable */
   gl_hpl_hdl = hpl_cb->cb_hdl;

   /* get the process id */
   hpl_cb->prc_id = m_NCS_OS_PROCESS_GET_ID();

   /* initialize the HPL cb lock */
   m_NCS_LOCK_INIT(&hpl_cb->cb_lock);

   /* Initialize the MDS channel. Install at subscribe to MDS ADEST */

   /* register with MDS */
   if ( (NCSCC_RC_SUCCESS != hpl_mds_initialize(hpl_cb)))
   {
      /* log */
      rc = NCSCC_RC_FAILURE;
      goto error;
   }

   return rc;

error:
   if (hpl_cb)
   {
      /* remove the association with hdl-mngr */
      if (hpl_cb->cb_hdl)
         ncshm_destroy_hdl(NCS_SERVICE_ID_HPL, hpl_cb->cb_hdl);

      /* destroy the lock */
      m_NCS_LOCK_DESTROY(&hpl_cb->cb_lock);

      /* free the control block */
      if (NULL != hpl_cb)
         m_MMGR_FREE_HPL_CB(hpl_cb);

      gl_hpl_hdl = 0;
   }
   return rc;
}


/****************************************************************************
 * Name          : hpl_finalize
 *
 * Description   : This routine destroys finalizes the HPL library usage.
 *                 It un-registers the HPL from MDS and frees the associated
 *                 control block resources before destroying the HPL control
 *                 block itself.
 *
 * Arguments     : destroy_info - ptr to the destroy information
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
 *****************************************************************************/

uns32 hpl_finalize (NCS_LIB_DESTROY *destroy_info)
{
   HPL_CB *hpl_cb = 0;
   uns32 rc = NCSCC_RC_SUCCESS;
   HAM_INFO *ham_inst, *ham_prev;

   /* retrieve HPL CB */
   hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl);
   if(!hpl_cb)
      return NCSCC_RC_FAILURE;

   /* unregister with MDS */
   rc = hpl_mds_finalize(hpl_cb);

   /* destroy the lock */
   m_NCS_LOCK_DESTROY(&hpl_cb->cb_lock);

   /* free the allocated HAM_INFO list */
   ham_inst = hpl_cb->ham_inst;
   while (ham_inst != NULL)
   {
      ham_prev = ham_inst;
      ham_inst = ham_inst->ham_next;
      m_MMGR_FREE_HAM_INFO(ham_prev);
   }

   /* return HPL CB */
   ncshm_give_hdl(gl_hpl_hdl);

   /* remove the association with hdl-mngr */
   ncshm_destroy_hdl(NCS_SERVICE_ID_HPL, hpl_cb->cb_hdl);

   /* free the control block */
   m_MMGR_FREE_HPL_CB(hpl_cb);

   /* reset the global cb handle */
   gl_hpl_hdl = 0;

   return rc;
}


/****************************************************************************
  Name          : ncs_hpl_lib_req

  Description   : This routine is exported to the external entities & is used
                  to initialize or destroy the HPI Private library of HISv

  Arguments     : req_info - ptr to the request info

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         : None
******************************************************************************/

uns32 ncs_hpl_lib_req (NCS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   switch (req_info->i_op)
   {
   case NCS_LIB_REQ_CREATE:
      rc =  hpl_initialize(&req_info->info.create);
      break;

   case NCS_LIB_REQ_DESTROY:
      rc = hpl_finalize(&req_info->info.destroy);
      break;

   default:
      break;
   }

   return rc;
}

/****************************************************************************
 * Name          : free_hisv_ret_msg
 *
 * Description   : Free the allocated return message from HAM.
 *
 * Arguments     : HISV_MSG *msg
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32
free_hisv_ret_msg (HISV_MSG *msg)
{
   if (msg == NULL) return NCSCC_RC_SUCCESS;

   if ((msg->info.cbk_info.hpl_ret.h_gen.data_len > 0 ) &&
       (msg->info.cbk_info.hpl_ret.h_gen.data != NULL ))
       m_MMGR_FREE_HISV_DATA((msg->info.cbk_info.hpl_ret.h_gen.data));

   m_MMGR_FREE_HISV_MSG(msg);
   return NCSCC_RC_SUCCESS;
}

