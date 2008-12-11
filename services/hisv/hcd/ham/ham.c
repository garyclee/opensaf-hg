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
*  MODULE NAME:  ham.c                                                       *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the functionality of HPI Application Manager.        *
*  HAM receives the request from HPI clients over MDS. It invokes the        *
*  required HPI call on open HPI session whenever request is received.       *
*                                                                            *
*****************************************************************************/

#include "hcd.h"

/* local function declarations */
static uns32 ham_process_mbx(SYSF_MBX *mbx);

/****************************************************************************
 * Name          : his_hcd_ham
 *
 * Description   : This is actual HAM thread. It gets as argument, the
 *                 session and domain identifiers of HPI session it manages.It
 *                 receives requests from HPI clients through MDS and invokes
 *                 appropriate HPI call. It also periodically clears the HPI's
 *                 system event log.
 *
 * Arguments     : HPI_SESSION_ARGS pointer holds the session and domain
 *                 identifier value and chassis_id of the chassis.
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hcd_ham()
{
   HAM_CB  *ham_cb = 0;
   SYSF_MBX *mbx;
   uns32    rc = NCSCC_RC_SUCCESS;
   NCS_SEL_OBJ_SET     all_sel_obj, temp_all_sel_obj;
   NCS_SEL_OBJ         mbx_fd;

   m_LOG_HISV_DTS_CONS("hcd_ham: Initializing HAM\n");
   /* retrieve HAM CB */
   ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl);
   if(!ham_cb)
   {
      m_LOG_HISV_DTS_CONS("hcd_ham: Could not get ham_cb handle\n");
      return NCSCC_RC_FAILURE;
   }

   /* Get the selection object from the MBX */
   mbx_fd = m_NCS_IPC_GET_SEL_OBJ(&ham_cb->mbx);
   mbx = &ham_cb->mbx;
   m_LOG_HISV_DTS_CONS("hcd_ham: Got HAM mail box\n");

   /* Give back the handle */
   ncshm_give_hdl(gl_ham_hdl);

   /* Set the fd for internal events */
   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);

   /** HAM thread main processing loop, receives request from HPL.
    **/
   for(;;)
   {
      temp_all_sel_obj = all_sel_obj;

      m_LOG_HISV_DTS_CONS("hcd_ham: HAM waiting to get request from HPL\n");
      /* pend on select */
      if(m_NCS_SEL_OBJ_SELECT(mbx_fd, &temp_all_sel_obj,NULL,NULL,NULL) < 0)
      {
         m_LOG_HISV_DTS_CONS("hcd_ham: Its HAM error at selection object\n");
         rc = NCSCC_RC_FAILURE;
         /* to break or to sleep and continue?  */
         m_NCS_TASK_SLEEP(2000);;
         continue;
      }
      m_LOG_HISV_DTS_CONS("hcd_ham: HAM Processing request from HPL\n");
      /* process the HAM Mail box */
      if (m_NCS_SEL_OBJ_ISSET(mbx_fd, &temp_all_sel_obj))
         /* now got the IPC mail box event */
         ham_process_mbx(mbx);
      else
         m_LOG_HISV_DTS_CONS("hcd_ham: BUG:  SELECT returns with empty obj-set\n");
   }
   return rc;
}


/****************************************************************************
 * Name          : ham_process_mbx
 *
 * Description   : This function process the messages received on HAM mail
 *                 box. Request from HPL clients are delivered to HAM mailbox
 *                 through MDS. Also the HAM timer puts on mailbox, the job
 *                 clearing HPI's sytem event log
 *
 * Arguments     : SYSF_MBX, the pointer to HAM mailbox queue.
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_process_mbx(SYSF_MBX *mbx)
{
   HISV_EVT *evt = NULL;

   /* receive all the messages delivered on mailbox and process them */
   while(NULL != (evt = (HISV_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx,evt)))
   {
      if (evt != NULL)
      {
         /* process the received message */
         /* m_LOG_HISV_DTS_CONS("Received Message\n"); */
         ham_process_evt(evt);
      }
      else
      {
         /* message null */
         m_LOG_HISV_DTS_CONS("ham_process_mbx: received NULL message\n");
      }
   }
   return NCSCC_RC_SUCCESS;
}

