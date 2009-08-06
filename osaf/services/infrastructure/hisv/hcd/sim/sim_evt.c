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
*  MODULE NAME:  sim_evt.c                                                   *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the functionality of HPI Shelf Init Manager's        *
*  event processing. The system firmware progress message requests received  *
*  from HSM are processed.                                                   *
*  here.                                                                     *
*                                                                            *
*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <SaHpi.h>
#include "hcd.h"

/* local function declarations */
static uns32 sim_health_chk_response (SIM_EVT *evt);
static uns32 sim_log_fprog_evt (SIM_EVT *evt);

/****************************************************************************
 * Name          : sim_log_fprog_evt
 *
 * Description   : This functions prints the system firmware progress events
 *                 through DTS
 *
 * Arguments     : SIM message which contained this requested command.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
sim_log_fprog_evt (SIM_EVT *evt)
{
   uns32    rc = NCSCC_RC_FAILURE, data2;
   HCD_CB * hcd_cb;
   SIM_CB *sim_cb = 0;
   SaHpiEventT        * fp_evt;          /* firmware progress event */
   SaHpiEntityPathT   * epath;           /* entity path of the resource */
   SaHpiUint8T  type, offset;

   if (evt->evt_type != HCD_SIM_FIRMWARE_PROGRESS)
      return NCSCC_RC_FAILURE;

   /* retrieve HCD CB */
   hcd_cb = (HCD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hcd_hdl);
   sim_cb = (SIM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_sim_hdl);
   if ((hcd_cb == NULL) || (sim_cb == NULL))
   {
      m_LOG_HISV_DTS_CONS("sim_log_fprog_evt: Error getting HCD CB handle\n");
      return rc;
   }
   /* get the event and entity path */
   fp_evt = &evt->msg.fp_evt;
   epath = &evt->msg.epath;

   /* get the values */
   type = (uns8)fp_evt->EventDataUnion.SensorEvent.EventState;

   /* Get the data2 otherwise return if it is not an OEM code */
   if (fp_evt->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_SENSOR_SPECIFIC)
   
      data2 = fp_evt->EventDataUnion.SensorEvent.SensorSpecific;
   else
   if (fp_evt->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_OEM)
      data2 = fp_evt->EventDataUnion.SensorEvent.Oem;
   else
   {
      printf("Non OEM sensor specific firmware progress event\n");
      ncshm_give_hdl(gl_hcd_hdl);
      return NCSCC_RC_SUCCESS;
   }
   offset = type >> 1;
   /* remove the lower 8 bits which contains FD incase of OEM */
   if ((fp_evt->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_OEM) &&
       !(fp_evt->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_SENSOR_SPECIFIC))
      data2 = (data2 >> 8) & 0xff;

   /* log the event through DTS */
#ifdef HAVE_HPI_A01
   if ((offset == HPI_EVT_FWPROG_PROG) && (data2 >= HPI_SE_FWPROG_CODE_OFFSET) && 
       (epath->Entry[2].EntityInstance <= MAX_NUM_SLOTS))
   {
      data2 = (data2 - HPI_SE_FWPROG_CODE_OFFSET);
      /* this is to avoid logging of duplicate events due to set event receiver logic  */
      if (data2 == HCD_SE_FWPROG_BOOT_SUCCESS)
         sim_cb->fwprog_done[epath->Entry[2].EntityInstance] = 1;
      if (sim_cb->fwprog_done[epath->Entry[2].EntityInstance])
      {
         m_LOG_HISV_FWPROG(data2, NCSFL_SEV_ALERT, epath->Entry[2].EntityInstance);
         if (data2 == HCD_SE_FWPROG_NODE_INIT_SUCCESS)
            sim_cb->fwprog_done[epath->Entry[2].EntityInstance] = 0;
      }
      else
         printf("dropped a set event receiver logic triggered event\n");
   }
   else
   if ((offset == HPI_EVT_FWPROG_ERROR) && (data2 >= HPI_SE_FWPROG_CODE_OFFSET))
   {
      data2 = (data2 - HPI_SE_FWPROG_CODE_OFFSET);
      m_LOG_HISV_FWERR(data2, NCSFL_SEV_ALERT, epath->Entry[2].EntityInstance);
   }
   else
      printf("Firmware progress event %d %2x received from resource %d\n", offset, data2, epath->Entry[2].EntityInstance);
#else
   if ((offset == HPI_EVT_FWPROG_PROG) && (data2 >= HPI_SE_FWPROG_CODE_OFFSET) &&
       (epath->Entry[1].EntityLocation <= MAX_NUM_SLOTS))
   {
      data2 = (data2 - HPI_SE_FWPROG_CODE_OFFSET);
      /* this is to avoid logging of duplicate events due to set event receiver logic  */
      if (data2 == HCD_SE_FWPROG_BOOT_SUCCESS)
         sim_cb->fwprog_done[epath->Entry[1].EntityLocation] = 1;
      if (sim_cb->fwprog_done[epath->Entry[1].EntityLocation])
      {
         m_LOG_HISV_FWPROG(data2, NCSFL_SEV_ALERT, epath->Entry[1].EntityLocation);
         if (data2 == HCD_SE_FWPROG_NODE_INIT_SUCCESS)
            sim_cb->fwprog_done[epath->Entry[1].EntityLocation] = 0;
      }
      else
         printf("dropped a set event receiver logic triggered event\n");
   }
   else
   if ((offset == HPI_EVT_FWPROG_ERROR) && (data2 >= HPI_SE_FWPROG_CODE_OFFSET))
   {
      data2 = (data2 - HPI_SE_FWPROG_CODE_OFFSET);
      m_LOG_HISV_FWERR(data2, NCSFL_SEV_ALERT, epath->Entry[1].EntityLocation);
   }
   else
      printf("Firmware progress event %d %2x received from resource %d\n", offset, data2, epath->Entry[1].EntityLocation);
#endif
   
   ncshm_give_hdl(gl_hcd_hdl);
   ncshm_give_hdl(gl_sim_hdl);
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : sim_health_chk_response
 *
 * Description   : This functions sends the health check response of SIM
 *                 thread to HCD process.
 *
 * Arguments     : SIM message which contained this requested command.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
sim_health_chk_response (SIM_EVT *evt)
{
   uns32    rc = NCSCC_RC_FAILURE;
   SIM_EVT * sim_evt;

   /* retrieve HCD CB */
   HCD_CB * hcd_cb = (HCD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hcd_hdl);
   if (hcd_cb == NULL)
   {
      m_LOG_HISV_DTS_CONS("sim_log_fprog_evt: Error getting HCD CB handle\n");
      return rc;
   }
   /* response for health-check of SIM */
   /* allocate the event */
   if (NULL == (sim_evt = m_MMGR_ALLOC_SIM_EVT))
   {
      m_LOG_HISV_DTS_CONS("sim_log_fprog_evt: error m_MMGR_ALLOC_SIM_EVTs\n");
      ncshm_give_hdl(gl_hcd_hdl);
      return rc;
   }
   sim_evt->evt_type = HCD_SIM_HEALTH_CHECK_RESP;

   /* send the request to HCD mailbox */
   if(m_NCS_IPC_SEND(&hcd_cb->mbx, sim_evt, NCS_IPC_PRIORITY_VERY_HIGH)
                      == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("sim_log_fprog_evt: failed to deliver msg on mail-box\n");
      m_MMGR_FREE_SIM_EVT(sim_evt);
      ncshm_give_hdl(gl_hcd_hdl);
      return rc;
   }
   ncshm_give_hdl(gl_hcd_hdl);
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : sim_evt_destroy
 *
 * Description   : Destroys SIM Event list.
 *
 * Arguments     : sim_evt - pointer to SIM event and associate message.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32
sim_evt_destroy(SIM_EVT *sim_evt)
{
   SIM_MSG *msg;

   if (NULL == sim_evt)
      return NCSCC_RC_FAILURE;
   msg = &sim_evt->msg;

   /* free the event block allocated */
   m_MMGR_FREE_SIM_EVT(sim_evt);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : sim_process_evt
 *
 * Description   : Process the events received from ShIM and AMF. Identfies
 *                 the message command and invokes appropriate function to
 *                 handle the event. Frees the message after processing is done.
 *
 * Arguments     : msg - ShIM message received on mail-box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32
sim_process_evt(SIM_EVT *evt)
{
   uns32 rc = NCSCC_RC_FAILURE;
   /* SIM_MSG *msg = &evt->msg; */

   /** check for healt-check event **/
   if (evt->evt_type == HCD_SIM_HEALTH_CHECK_REQ)
      rc = sim_health_chk_response(evt);

   /* check for system firmware progress events */
   if (evt->evt_type == HCD_SIM_FIRMWARE_PROGRESS)
      rc = sim_log_fprog_evt(evt);

   sim_evt_destroy(evt);
   return rc;
}
