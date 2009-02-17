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
*  MODULE NAME:  sim.c                                                       *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the functionality of HPI Shelf Init Manager.         *
*  ShIM receives the firmware progress messages from HSM and it printes them *
*  through DTS.                                                              *
*                                                                            *
*****************************************************************************/

#include "hcd.h"

/* local function declarations */
static uns32 sim_process_mbx(SYSF_MBX *mbx);

/****************************************************************************
 * Name          : his_hcd_sim
 *
 * Description   : This is actual SIM thread. It receives the system firmware
 *                 progress messages from HSM and it printes them through DTS.
 *
 * Arguments     : HPI_SESSION_ARGS pointer holds the session and domain
 *                 identifier value and chassis_id of the chassis.
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hcd_sim()
{
   SIM_CB  *sim_cb = 0;
   SYSF_MBX *mbx;
   uns32    rc = NCSCC_RC_SUCCESS;
   NCS_SEL_OBJ_SET     all_sel_obj, temp_all_sel_obj;
   NCS_SEL_OBJ         mbx_fd;

   /* retrieve SIM CB */
   sim_cb = (SIM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_sim_hdl);
   if(!sim_cb)
      return NCSCC_RC_FAILURE;

   /* Get the selection object from the MBX */
   mbx_fd = m_NCS_IPC_GET_SEL_OBJ(&sim_cb->mbx);
   mbx = &sim_cb->mbx;

   /* Give back the handle */
   ncshm_give_hdl(gl_sim_hdl);

   /* Set the fd for internal events */
   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);

   /** SIM thread main processing loop, receives request from HPL.
    **/
   for(;;)
   {
      temp_all_sel_obj = all_sel_obj;

      /* pend on select */
      if(m_NCS_SEL_OBJ_SELECT(mbx_fd, &temp_all_sel_obj,NULL,NULL,NULL) < 0)
      {
         rc = NCSCC_RC_FAILURE;
         m_NCS_CONS_PRINTF("hcd_sim: error in select\n");
         m_NCS_TASK_SLEEP(2000);
         continue;
      }
      /* process the SIM Mail box */
      if (m_NCS_SEL_OBJ_ISSET(mbx_fd, &temp_all_sel_obj))
         /* now got the IPC mail box event */
         sim_process_mbx(mbx);
      else
         m_LOG_HISV_DEBUG("BUG:  SELECT returns with empty obj-set\n");

   }
   return rc;
}


/****************************************************************************
 * Name          : sim_process_mbx
 *
 * Description   : This function process the messages received on SIM mail
 *                 box. Sytem firmware progress events from HSM and the
 *                 health check events from AMF are expected on this mailbox.
 *
 * Arguments     : SYSF_MBX, the pointer to SIM mailbox queue.
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
sim_process_mbx(SYSF_MBX *mbx)
{
   SIM_EVT *evt = NULL;

   /* receive all the messages delivered on mailbox and process them */
   while(NULL != (evt = (SIM_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx,evt)))
   {
      if (evt != NULL)
      {
         /* process the received message */
         /* m_LOG_HISV_DEBUG("Received Message\n"); */
         sim_process_evt(evt);
      }
      else
      {
         /* message null */
         m_LOG_HISV_DEBUG("received NULL message\n");
      }
   }
   return NCSCC_RC_SUCCESS;
}

