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
*  MODULE NAME:  ham_init.c                                                  *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the routines used to intialize or destroy the HPI    *
*  Application Manage. It creats and initializes HPL control block structure *
*  and registers to MDS on a virtual address. Also contains the function     *
*  to finalize HAM, un-register MDS and free the resources & control block   *
*                                                                            *
*****************************************************************************/

#include "hcd.h"

/* global 'HAM control block' handle */
uns32 gl_ham_hdl = 0;


/****************************************************************************
 * Name          : ham_initialize
 *
 * Description   : This function creates and initializes the HAM control
 *                 block structure. It registers to MDS on virtual address
 *                 to receive requests from HPL. Also creates the IPC queue
 *                 used as mail-box to deliver the messages to HAM thread.
 *
 * Arguments     : args - holds HPI session-id, domain-id, and chassis_id
 *
 * Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ham_initialize(HPI_SESSION_ARGS *args)
{
   HAM_CB *ham_cb = NULL;
   uns32  rc = NCSCC_RC_SUCCESS;

   m_LOG_HISV_DTS_CONS("ham_initialize: Initializing HAM thread...\n");
   /* allocate HAM cb */
   if ( NULL == (ham_cb = m_MMGR_ALLOC_HAM_CB))
   {
      /* log */
      m_LOG_HISV_DTS_CONS("ham_initialize: Memory error while initializing HAM thread\n");
      rc = NCSCC_RC_FAILURE;
      goto error;
   }
   m_NCS_OS_MEMSET(ham_cb, 0, sizeof(HAM_CB));

   /* assign the HAM pool-id (used by hdl-mngr) */
   ham_cb->pool_id = NCS_HM_POOL_ID_COMMON;

   /* create the association with hdl-mngr */
   if ( 0 == (ham_cb->cb_hdl = ncshm_create_hdl(ham_cb->pool_id, NCS_SERVICE_ID_HCD,
                                                   (NCSCONTEXT)ham_cb)) )
   {
      m_LOG_HISV_DTS_CONS("ham_initialize: error create ham_cb handle\n");
      ham_cb->cb_hdl = 0;
      rc = NCSCC_RC_FAILURE;
      goto error;
   }
   /* store the cb hdl in the global variable */
   gl_ham_hdl = ham_cb->cb_hdl;

   ham_cb->args = args;
   /* get the process id */
   ham_cb->prc_id = m_NCS_OS_PROCESS_GET_ID();
   ham_cb->dest = NULL;

   /* Create the mbx to communicate with the HAM thread */
   if (NCSCC_RC_SUCCESS != (rc = m_NCS_IPC_CREATE(&ham_cb->mbx)))
   {
      /* Destroy the hdl for this CB */
      ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl);
      gl_ham_hdl = 0;
      m_LOG_HISV_DTS_CONS("ham_initialize: error creating ham mbx\n");
      /* Free the control block */
      m_MMGR_FREE_HAM_CB(ham_cb);
      return NCSCC_RC_FAILURE;
   }

   /* Attach the IPC to the created thread */
   m_NCS_IPC_ATTACH(&ham_cb->mbx);

   /* initialize the HAM cb lock */
   m_NCS_LOCK_INIT(&ham_cb->cb_lock);

   ham_cb->tmr_id = TMR_T_NULL;
   /* start the system event log clear timer */
   if (NCSCC_RC_SUCCESS != ham_start_tmr(ham_cb, HPI_SYS_LOG_CLEAR_INTERVAL))
   {
      m_LOG_HISV_DTS_CONS("ham_initialize: failed to create sys event log clear time\n");
      /* release the IPC */
      m_NCS_IPC_RELEASE(&ham_cb->mbx, NULL);
      ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, ham_cb->cb_hdl);
      /* destroy the lock */
      m_NCS_LOCK_DESTROY(&ham_cb->cb_lock);
      /* free the control block */
      m_MMGR_FREE_HAM_CB(ham_cb);
      gl_ham_hdl = 0;
      return NCSCC_RC_FAILURE;
   }
   m_LOG_HISV_DTS_CONS("ham_initialize: MDS subscription\n");

   /* Initialize the MDS channel. Install at subscribe to MDS ADEST */
   /* register with MDS */
   if ( (NCSCC_RC_SUCCESS != ham_mds_initialize(ham_cb)))
   {
      m_LOG_HISV_DTS_CONS("ham_initialize: MDS initialization failed ...\n");
      /* log */
      rc = NCSCC_RC_FAILURE;
      goto error;
   }
   m_LOG_HISV_DTS_CONS("ham_initialize: Waiting for MDS subscription to happen ...\n");
   m_NCS_TASK_SLEEP(5000);
   return rc;

error:
   if (ham_cb)
   {
      /* release the IPC */
      m_LOG_HISV_DTS_CONS("ham_initialize: Failed\n");
      m_NCS_IPC_RELEASE(&ham_cb->mbx, NULL);

      /* stop and destroy the timer */
      ham_stop_tmr(ham_cb);

      /* remove the association with hdl-mngr */
      if (ham_cb->cb_hdl)
         ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, ham_cb->cb_hdl);

      /* destroy the lock */
      m_NCS_LOCK_DESTROY(&ham_cb->cb_lock);

      /* free the control block */
      m_MMGR_FREE_HAM_CB(ham_cb);

      gl_ham_hdl = 0;
   }
   m_LOG_HISV_DTS_CONS("ham_initialize: Done\n");
   return rc;
}


/****************************************************************************
 * Name          : ham_clear_mbx
 *
 * Description   : This is the function which deletes all the messages from
 *                 the HAM mail-box. This is called as a callback from IPC
 *                 detach function
 *
 * Arguments     : arg     - argument to be passed.
 *                 msg     - Event start pointer.
 *
 * Return Values : TRUE/FALSE
 *
 * Notes         : None.
 *****************************************************************************/

static NCS_BOOL
ham_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg)
{
   HISV_EVT  *pEvt = (HISV_EVT *)msg;
   HISV_EVT  *pnext;
   pnext = pEvt;
   m_LOG_HISV_DTS_CONS("ham_clear_mbx: Clearing mbx...\n");
   while (pnext)
   {
      pnext = (/* (HISV_EVT *)&*/(pEvt->next));
      hisv_evt_destroy(pEvt);
      pEvt = pnext;
   }
   m_LOG_HISV_DTS_CONS("ham_clear_mbx: Clearing Done\n");
   return TRUE;
}


/****************************************************************************
 * Name          : ham_finalize
 *
 * Description   : This routine un-registers the MDS, detaches HAM mailbox
 *                 from IPC, destroys the HAM control block and frees the
 *                 resources.
 *
 * Arguments     : destroy_info - ptr to the destroy info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
 *****************************************************************************/

uns32 ham_finalize ()
{
   HAM_CB *ham_cb = 0;
   uns32 rc;

   m_LOG_HISV_DTS_CONS("ham_finalize: cleaning up ham_cb...\n");
   /* retrieve HAM CB */
   ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl);
   if(!ham_cb)
   {
      m_LOG_HISV_DTS_CONS("ham_finalize: error getting ham_cb handle\n");
      return NCSCC_RC_FAILURE;
   }
   /* return HAM CB */
   ncshm_give_hdl(gl_ham_hdl);

   /* remove the association with hdl-mngr */
   ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, ham_cb->cb_hdl);

   /* stop and destroy the timer */
   ham_stop_tmr(ham_cb);

   /* unregister with MDS */
   if (NCSCC_RC_SUCCESS != (rc = ham_mds_finalize(ham_cb)))
      m_LOG_HISV_DTS_CONS("ham_finalize: failed to unregister with MDS\n");

   /* stop & kill the created task */
   m_NCS_TASK_STOP(ham_cb->task_hdl);
   m_NCS_TASK_RELEASE(ham_cb->task_hdl);

   /* Detach from IPC */
   m_NCS_IPC_DETACH(&ham_cb->mbx, ham_clear_mbx, ham_cb);

   /* release the IPC */
   m_NCS_IPC_RELEASE(&ham_cb->mbx, NULL);

   /* destroy the lock */
   m_NCS_LOCK_DESTROY(&ham_cb->cb_lock);

   /* free the control block */
   m_MMGR_FREE_HAM_CB(ham_cb);

   /* reset the global cb handle */
   gl_ham_hdl = 0;
   m_LOG_HISV_DTS_CONS("ham_finalize: Done\n");
   return rc;
}
