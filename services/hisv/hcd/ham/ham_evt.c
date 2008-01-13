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
*  MODULE NAME:  ham_evt.c                                                   *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the functionality of HPI Application Manager's       *
*  event processing. The message requests received from HPL are processed    *
*  here.                                                                     *
*                                                                            *
*****************************************************************************/

#include "SaHpi.h"

#include "hcd.h"

/* local function declarations */
static uns32 ham_resource_power_set(HISV_EVT *evt);
static uns32 ham_chassis_id_send(HISV_EVT *evt);
static uns32 ham_chassis_id_resend(HISV_EVT *evt);
static uns32 ham_resource_reset(HISV_EVT *evt);
static uns32 ham_sel_clear(HISV_EVT *evt);
static uns32 ham_tmr_sel_clear(HISV_EVT *evt);
static uns32 ham_health_chk_response (HISV_EVT *evt);
static uns32 get_resourceid (uns8 *epath_str, uns32 epath_len,
                             SaHpiResourceIdT *resourceid);
static void set_hisv_msg(HISV_MSG *hisv_msg);
static uns32 ham_adest_update(HISV_MSG *msg, HAM_CB *ham_cb);

/* boot bank GET/SET functions */
static uns32 ham_bootbank_get (HISV_EVT *evt);
static uns32 ham_bootbank_set (HISV_EVT *evt);


/****************************************************************************
 * Name          : ham_chassis_id_send
 *
 * Description   : This function makes the HISv message and sends the
 *                 chassis-id of chassis managed by HAM, to HPL registered
 *                 MDS ADEST. HPL needs the information of HAM's chassis-id
 *                 in order to map the request to send to the instance of HAM.
 *
 * Arguments     : msg - HISV message received on mail-box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_chassis_id_send(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg;
   HAM_CB  *ham_cb;
   HISV_MSG  hisv_msg;
   uns32    rc;

   m_LOG_HISV_DTS_CONS("ham_chassis_id_send: HAM Sending chassis-id to HPL\n");

   /* check data availability in message. It holds HPL ADEST */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_chassis_id_send: No data received in HISV_MSG\n");
      return NCSCC_RC_FAILURE;
   }

   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_chassis_id_send: Failed to get ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /* store the HPL ADEST */
   ham_cb->hpl_dest = *(MDS_DEST *)msg->info.api_info.data;

   /** populate the mds message with chassis_id, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_HAM_VDEST_RET;
   m_HAM_HISV_CHASSIS_ID_FILL(hisv_msg, ham_cb->args->chassis_id,
                              ham_cb->ham_vdest, HAM_MDS_UP);

   /* send the message containing chassis_id, to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &ham_cb->hpl_dest,
                          MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_SND, evt);
   m_LOG_HISV_DTS_CONS("ham_chassis_id_send: HAM Sent chassis-id to HPL\n");

   /* give control block handle */
   ncshm_give_hdl(gl_ham_hdl);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : ham_resource_power_set
 *
 * Description   : This function process the HPL message to change the power
 *                 status of a resource. It gets the resource-id of the
 *                 resource identified by given entity-path and invokes the
 *                 HPI call saHpiResourcePowerStateSet() to change the power
 *                 status to ON, OFF or to Cycle it.
 *
 * Arguments     : msg - HISV message from HPL, contains entity-path.
 *                 power_state - Power ON, Power OFF, Power-Cycle.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_resource_power_set(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   HAM_CB  *ham_cb;
   SaErrorT       err;
#ifdef HPI_A
   SaHpiHsPowerStateT  power_state;
#else
   SaHpiPowerStateT  power_state;
#endif
   uns32    rc = NCSCC_RC_FAILURE;
   SaHpiResourceIdT resourceid;

   m_LOG_HISV_DTS_CONS("ham_resource_power_set: HAM processing resource power set request\n");

   set_hisv_msg (&hisv_msg);
   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_resource_power_set: error taking ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }
   /* check data availability in message. It holds resource entity path */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_resource_power_set: No data received in HISV_MSG\n");
      goto ret;
   }
   rc = get_resourceid(msg->info.api_info.data+4, msg->info.api_info.data_len, &resourceid);
   if (rc == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_resource_power_set: error getting resource-id\n");
      goto ret;
   }
   /* get required power status */
#ifdef HPI_A
   power_state = (SaHpiHsPowerStateT)msg->info.api_info.arg;
#else
   power_state = (SaHpiPowerStateT)msg->info.api_info.arg;
#endif
   m_NCS_CONS_PRINTF("ham_resource_power_set: power state = %d\n", power_state);

   /** got the resource-id of resource with given entity-path
    ** Invoke HPI call to change the power status of the resource.
    **/
   err =  saHpiResourcePowerStateSet(ham_cb->args->session_id, resourceid, power_state);
   if (SA_OK == err)
         rc = NCSCC_RC_SUCCESS;
   else
   {
         rc = NCSCC_RC_FAILURE;
         m_LOG_HISV_DTS_CONS("ham_resource_power_set: error saHpiResourcePowerStateSet\n");
   }
   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data = NULL;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;

ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);

   m_LOG_HISV_DTS_CONS("ham_resource_power_set: resource power state set, Invoked\n");

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}


/****************************************************************************
 * Name          : ham_resource_reset
 *
 * Description   : This function process the HPL message to reset the resource.
 *                 It gets the resource-id of the resource identified by given
 *                 entity-path and invokes the HPI call saHpiResourceResetStateSet()
 *                 to change the reset state to ON, OFF or to Cycle it.
 *
 * Arguments     : msg - HISV message from HPL, contains entity-path.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_resource_reset(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   HAM_CB  *ham_cb;
   SaErrorT       err;
   SaHpiResourceIdT   resourceid;
   uns32    reset_type;
   uns32    rc = NCSCC_RC_FAILURE;

   m_LOG_HISV_DTS_CONS("ham_resource_reset: Invoked\n");
   set_hisv_msg (&hisv_msg);

   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_resource_reset: error taking ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }

   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_resource_reset: No data received in HISV_MSG\n");
      goto ret;
   }

   rc = get_resourceid(msg->info.api_info.data+4, msg->info.api_info.data_len, &resourceid);
   if (rc == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_resource_reset: error getting resource-id\n");
      goto ret;
   }
   /* get the type of 'reset' requested */
   reset_type = (SaHpiResetActionT)msg->info.api_info.arg;
   m_NCS_CONS_PRINTF("ham_resource_reset: reset type = %d\n", reset_type);

   /** got the resource-id of resource with given entity-path
    ** Invoke HPI call to reset the resource.
    **/
   /* first check if it needs graceful reboot */
   if (reset_type == HISV_RES_GRACEFUL_REBOOT)
   {
      SaHpiCtrlNumT ctrlNum;
      SaHpiCtrlStateT state;

#ifdef HPI_A
      ctrlNum = 15;
#else
      ctrlNum = 4608;
#endif
      state.Type = SAHPI_CTRL_TYPE_ANALOG;
      state.StateUnion.Analog = 1;
      m_LOG_HISV_DTS_CONS("Invoking graceful reboot\n");
#ifdef HPI_A
      err = saHpiControlStateSet(ham_cb->args->session_id, resourceid, ctrlNum, &state);
#else
      err = saHpiControlSet(ham_cb->args->session_id, resourceid, ctrlNum, SAHPI_CTRL_MODE_MANUAL, &state);
#endif
   }
   else
   {
      err =  saHpiResourceResetStateSet(ham_cb->args->session_id, resourceid,
                                       (SaHpiResetActionT)reset_type);
   }
   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
   {
      m_LOG_HISV_DTS_CONS("ham_resource_reset: error in saHpiResourceResetStateSet; Attempting cold reset\n");
      err =  saHpiResourceResetStateSet(ham_cb->args->session_id, resourceid,
                                        (SaHpiResetActionT)SAHPI_COLD_RESET);
      if (SA_OK == err)
         rc = NCSCC_RC_SUCCESS;
      else
      {
         m_NCS_CONS_PRINTF("ham_resource_reset: cold reset attempt failed in saHpiResourceResetStateSet\n");
         rc = NCSCC_RC_FAILURE;
      }
   }
   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data = NULL;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;

ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   m_LOG_HISV_DTS_CONS("ham_resource_reset: resource reset state set, Invoked\n");

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_sel_clear
 *
 * Description   : This functions handles the request to clear the HPI's
 *                 system event log. This request would be triggered by
 *                 any administrative entity (using HPL API).
 *
 * Arguments     : HISV message which contained this requested command.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_sel_clear(HISV_EVT *evt)
{
   HISV_MSG hisv_msg;
   HAM_CB  *ham_cb;
   SaHpiEntryIdT  current, next;
   SaHpiRptEntryT  entry;
   SaErrorT       err;
   uns32    rc = NCSCC_RC_FAILURE;

   m_LOG_HISV_DTS_CONS("ham_sel_clear: Invoked\n");
   set_hisv_msg (&hisv_msg);
   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_sel_clear: error taking ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }
   next = SAHPI_FIRST_ENTRY;
   do
   {
      /* locate the resource id of resource with given entity-path */
      current = next;
      err = saHpiRptEntryGet(ham_cb->args->session_id, current, &next, &entry);
      if (SA_OK != err)
      {
         if (current != SAHPI_FIRST_ENTRY)
         {
            m_LOG_HISV_DTS_CONS("ham_sel_clear: Error first entry\n");
            break;
         }
         else
         {
            m_LOG_HISV_DTS_CONS("ham_sel_clear: Empty RPT\n");
            break;
         }
      }
      /* invoke HPI call to clear system event log of entire domain controller */
#ifdef HPI_A
      if (entry.ResourceCapabilities & SAHPI_CAPABILITY_SEL)
#else
      if (entry.ResourceCapabilities & SAHPI_CAPABILITY_EVENT_LOG)
#endif
         err =  saHpiEventLogClear(ham_cb->args->session_id, entry.ResourceId);
      else
         err = SA_OK;
      if (SA_OK == err)
         rc = NCSCC_RC_SUCCESS;
      else
      {
         rc = NCSCC_RC_FAILURE;
         m_LOG_HISV_DTS_CONS("ham_sel_clear: error in saHpiEventLogClear\n");
      }
   } while (next != SAHPI_LAST_ENTRY);
   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data = NULL;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;

ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   m_LOG_HISV_DTS_CONS("ham_sel_clear: SEL clear, Invoked\n");

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_tmr_sel_clear
 *
 * Description   : This functions handles the request to clear the HPI's
 *                 system event log. This reques is triggered periodically
 *                 done by HAM timer.
 *
 * Arguments     : HISV message which contained this requested command.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_tmr_sel_clear(HISV_EVT *evt)
{
   HAM_CB  *ham_cb;
   SaHpiEntryIdT  current, next;
   SaHpiRptEntryT  entry;
   SaErrorT       err;
   uns32    rc = NCSCC_RC_FAILURE;

   m_LOG_HISV_DTS_CONS("ham_tmr_sel_clear: Invoked\n");
   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_tmr_sel_clear: error taking ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }
   next = SAHPI_FIRST_ENTRY;
   do
   {
      /* locate the resource id of resource with given entity-path */
      current = next;
      err = saHpiRptEntryGet(ham_cb->args->session_id, current, &next, &entry);
      if (SA_OK != err)
      {
         if (current != SAHPI_FIRST_ENTRY)
         {
            m_LOG_HISV_DTS_CONS("ham_tmr_sel_clear: Error first entry\n");
            break;
         }
         else
         {
            m_LOG_HISV_DTS_CONS("ham_tmr_sel_clear: Empty RPT\n");
#ifdef HPI_A
            saHpiResourcesDiscover(ham_cb->args->session_id);
#else
            saHpiDiscover(ham_cb->args->session_id);
#endif
            break;
         }
      }
      /* invoke HPI call to clear system event log of entire domain controller */
#ifdef HPI_A
      if (entry.ResourceCapabilities & SAHPI_CAPABILITY_SEL)
#else
      if (entry.ResourceCapabilities & SAHPI_CAPABILITY_EVENT_LOG)
#endif
         err =  saHpiEventLogClear(ham_cb->args->session_id, entry.ResourceId);
      else
         err = SA_OK;

      if (SA_OK == err)
         rc = NCSCC_RC_SUCCESS;
      else
      {
         m_LOG_HISV_DTS_CONS("ham_tmr_sel_clear: error in saHpiEventLogClear\n");
         rc = NCSCC_RC_FAILURE;
      } 
   } while (next != SAHPI_LAST_ENTRY);

ret:
   m_LOG_HISV_DTS_CONS("ham_tmr_sel_clear: Done\n");
   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_config_hotswap
 *
 * Description   : This function process the HPL message to configure the
 *                 hotswap timeouts of a resource. It handles the request
 *                 type: HS_AUTO_INSERT_TIMEOUT_SET, HS_AUTO_INSERT_TIMEOUT_GET.
 *
 * Arguments     : msg - HISV message from HPL, contains entity-path.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_config_hotswap(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   HAM_CB  *ham_cb;
   SaErrorT       err;
   SaHpiTimeoutT      Timeout;
   uns16 arg_len = sizeof(SaHpiTimeoutT);
   uns32    rc = NCSCC_RC_FAILURE;

   set_hisv_msg (&hisv_msg);
   m_LOG_HISV_DTS_CONS("ham_config_hotswap: Invoked\n");

   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_config_hotswap: error taking ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }
   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_config_hotswap: No data received in HISV_MSG\n");
      goto ret;;
   }
   /* get the value of 'timeout' requested */
   Timeout = *(SaHpiResetActionT *)((uns8 *)msg->info.api_info.data+4);
   m_NCS_CONS_PRINTF("ham_config_hotswap: Timeout = %d\n, Command = %d", 
                     (uns32)Timeout, msg->info.api_info.cmd);

   /** Invoke HPI call to configure the hotswap state of the resource.
    **/
   if (msg->info.api_info.cmd == HS_AUTO_INSERT_TIMEOUT_SET)
      err =  saHpiAutoInsertTimeoutSet(ham_cb->args->session_id, Timeout);
   else
      err =  saHpiAutoInsertTimeoutGet(ham_cb->args->session_id, &Timeout);

   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
   {
      m_LOG_HISV_DTS_CONS("ham_config_hotswap: saHpiAutoInsertTimeout Error\n");
      rc = NCSCC_RC_FAILURE;
   }
   m_NCS_CONS_PRINTF("ham_config_hotswap: Timeout = %d\n",(uns32)Timeout);
   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;

   if (msg->info.api_info.cmd == HS_AUTO_INSERT_TIMEOUT_SET)
   {
     hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
     hisv_msg.info.cbk_info.hpl_ret.h_gen.data = NULL;
   }
   else
   {
      hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = arg_len;
      hisv_msg.info.cbk_info.hpl_ret.h_gen.data = m_MMGR_ALLOC_HISV_DATA(arg_len);
      m_NCS_MEMCPY(hisv_msg.info.cbk_info.hpl_ret.h_gen.data, (uns8 *)
                   &Timeout, arg_len);
   }
ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   if (msg->info.api_info.cmd != HS_AUTO_INSERT_TIMEOUT_SET)
      m_MMGR_FREE_HISV_DATA(hisv_msg.info.cbk_info.hpl_ret.h_gen.data);

   m_LOG_HISV_DTS_CONS("ham_config_hotswap: config hotswap, auto-insert timeout, Invoked\n");

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_hs_indicator_state
 *
 * Description   : This function process the HPL message to configure the
 *                 hotswap timeouts of a resource. It handles the request
 *                 type: HS_INDICATOR_STATE_GET, HS_INDICATOR_STATE_SET
 *
 * Arguments     : msg - HISV message from HPL, contains entity-path.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_hs_indicator_state(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   SaHpiResourceIdT   resourceid;
   HAM_CB  *ham_cb;
   SaErrorT       err;
   SaHpiHsIndicatorStateT      stateT;
   uns32 ret_state, rc = NCSCC_RC_FAILURE;
   uns16 arg_len = sizeof(ret_state);

   set_hisv_msg(&hisv_msg);
   m_LOG_HISV_DTS_CONS("ham_hs_indicator_state: Invoked\n");

   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_hs_indicator_state: error taking ham_cb handle\n");
      goto ret;
   }
   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_hs_indicator_state: No data received in HISV_MSG\n");
      goto ret;
   }
   rc = get_resourceid(msg->info.api_info.data+4, msg->info.api_info.data_len, &resourceid);
   if (rc == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_hs_indicator_state: error getting resource-id\n");
      goto ret;
   }
   /* get the value of 'timeout' requested */
   stateT = msg->info.api_info.arg;

   /** Invoke HPI call to change the indicator state of the resource.
    **/
   if (msg->info.api_info.cmd == HS_INDICATOR_STATE_SET)
      err =  saHpiHotSwapIndicatorStateSet(ham_cb->args->session_id, resourceid, stateT);
   else
      err =  saHpiHotSwapIndicatorStateGet(ham_cb->args->session_id, resourceid, &stateT);

   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
     rc = NCSCC_RC_FAILURE;

   m_NCS_CONS_PRINTF("ham_hs_indicator_state: rc = %d, stateT = %d\n",rc, stateT);
   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;

   if (msg->info.api_info.cmd == HS_INDICATOR_STATE_SET)
   {
     hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
     hisv_msg.info.cbk_info.hpl_ret.h_gen.data = NULL;
   }
   else
   {
      ret_state = stateT;
      hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = arg_len;
      hisv_msg.info.cbk_info.hpl_ret.h_gen.data = m_MMGR_ALLOC_HISV_DATA(arg_len);
      m_NCS_MEMCPY(hisv_msg.info.cbk_info.hpl_ret.h_gen.data,
                   (uns8 *)&ret_state, arg_len);
   }
ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   if (msg->info.api_info.cmd != HS_INDICATOR_STATE_SET)
      m_MMGR_FREE_HISV_DATA(hisv_msg.info.cbk_info.hpl_ret.h_gen.data);
   m_LOG_HISV_DTS_CONS("ham_hs_indicator_state: config hotswap, indicator state, Invoked\n");

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_manage_hotswap
 *
 * Description   : This function process the HPL message to configure the
 *                 hotswap state of a resource. It handles the request
 *                 type: HS_POLICY_CANCEL, HS_RESOURCE_ACTIVE_SET,
 *                       HS_RESOURCE_INACTIVE_SET, HS_ACTION_REQUEST.
 *
 * Arguments     : msg - HISV message from HPL, contains entity-path.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_manage_hotswap(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   SaHpiResourceIdT   resourceid;
   HAM_CB  *ham_cb;
   SaErrorT       err;
   SaHpiHsActionT      ActionT;
   uns32 rc = NCSCC_RC_FAILURE;

   set_hisv_msg(&hisv_msg);
   m_LOG_HISV_DTS_CONS("ham_manage_hotswap: Invoked\n");

   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_manage_hotswap: error taking ham_cb handle\n");
      goto ret;
   }
   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_manage_hotswap: No data received in HISV_MSG\n");
      goto ret;
   }
   rc = get_resourceid(msg->info.api_info.data+4, msg->info.api_info.data_len, &resourceid);
   if (rc == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_manage_hotswap: error getting resource-id\n");
      goto ret;
   }
   /* get the value of 'timeout' requested */
   ActionT = msg->info.api_info.arg;

   /** Invoke HPI call to change configure the hotswap of resource.
    **/
   switch (msg->info.api_info.cmd)
   {
   case HS_POLICY_CANCEL:
#ifdef HPI_A
      err = saHpiHotSwapControlRequest(ham_cb->args->session_id, resourceid);
#else
      err = saHpiHotSwapPolicyCancel(ham_cb->args->session_id, resourceid);
#endif
      m_LOG_HISV_DTS_CONS("HAM: ham_manage_hotswap : HS_POLICY_CANCEL, Invoked\n");
      break;

   case HS_RESOURCE_ACTIVE_SET:
      err = saHpiResourceActiveSet(ham_cb->args->session_id, resourceid);
      m_LOG_HISV_DTS_CONS("HAM: ham_manage_hotswap : HS_RESOURCE_ACTIVE_SET, Invoked\n");
      break;

   case HS_RESOURCE_INACTIVE_SET:
      err = saHpiResourceInactiveSet(ham_cb->args->session_id, resourceid);
      m_LOG_HISV_DTS_CONS("HAM: ham_manage_hotswap : HS_RESOURCE_INACTIVE_SET, Invoked\n");
      break;

   case HS_ACTION_REQUEST:
      err = saHpiHotSwapActionRequest(ham_cb->args->session_id, resourceid, ActionT);
      m_LOG_HISV_DTS_CONS("HAM: ham_manage_hotswap : HS_ACTION_REQUEST, Invoked\n");
      break;

   default:
      m_LOG_HISV_DTS_CONS("HAM: ham_manage_hotswap : Invalid Command, Invoked\n");
      goto ret;
      break;
   }
   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
      rc = NCSCC_RC_FAILURE;

   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data = NULL;

ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_hs_cur_state_get
 *
 * Description   : This function process the HPL message to get the current
 *                 hot swap state of a resource.
 *
 *                 type: HS_CURRENT_HS_STATE_GET
 *
 * Arguments     : msg - HISV message from HPL, contains entity-path.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_hs_cur_state_get(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   SaHpiResourceIdT   resourceid;
   HAM_CB  *ham_cb;
   SaErrorT       err;
   SaHpiHsStateT      stateT;
   uns32 ret_state, rc = NCSCC_RC_FAILURE;
   uns16 arg_len = sizeof(ret_state);

   set_hisv_msg(&hisv_msg);
   m_LOG_HISV_DTS_CONS("ham_hs_cur_state_get: Invoked\n");
   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_hs_cur_state_get: error taking ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }
   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_hs_cur_state_get: No data received in HISV_MSG\n");
      goto ret;
   }
   rc = get_resourceid(msg->info.api_info.data+4, msg->info.api_info.data_len, &resourceid);
   if (rc == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_hs_cur_state_get: error getting resource-id\n");
      goto ret;
   }
   /* get the value of 'timeout' requested */
   stateT = msg->info.api_info.arg;

   /** Invoke HPI call to get the current hotswap state of the resource.
    **/
   err = saHpiHotSwapStateGet(ham_cb->args->session_id, resourceid, &stateT);
   m_NCS_CONS_PRINTF("ham_hs_cur_state_get: stateT = %d, err = %d\n", stateT, err);

   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
      rc = NCSCC_RC_FAILURE;
   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;
   ret_state = stateT;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = arg_len;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data = m_MMGR_ALLOC_HISV_DATA(arg_len);
   m_NCS_MEMCPY(hisv_msg.info.cbk_info.hpl_ret.h_gen.data,
                (uns8 *)&ret_state, arg_len);
ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   if (msg->info.api_info.cmd != HS_INDICATOR_STATE_SET)
      m_MMGR_FREE_HISV_DATA(hisv_msg.info.cbk_info.hpl_ret.h_gen.data);

   m_LOG_HISV_DTS_CONS("HAM: ham_hs_cur_state_get, Invoked\n");
   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_config_hs_autoextract
 *
 * Description   : This function process the HPL message to configure the
 *                 hotswap auto-extract timeouts of a resource. It handles
 *                 the request type: HS_AUTO_EXTRACT_TIMEOUT_SET, and
 *                 HS_INDICATOR_STATE_GET
 *
 * Arguments     : msg - HISV message from HPL, contains entity-path.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_config_hs_autoextract(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   SaHpiResourceIdT   resourceid;
   HAM_CB  *ham_cb;
   SaErrorT       err;
   SaHpiTimeoutT  Timeout;
   uns16 arg_len = sizeof(SaHpiTimeoutT);
   uns32 rc = NCSCC_RC_FAILURE;
   HPL_TLV  *hpl_tlv;

   set_hisv_msg(&hisv_msg);
   m_LOG_HISV_DTS_CONS("ham_config_hs_autoextract: Invoked\n");

   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_config_hs_autoextract: error getting ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }
   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_config_hs_autoextract: No data received in HISV_MSG\n");
      goto ret;
   }
   hpl_tlv = (HPL_TLV *)msg->info.api_info.data;
   rc = get_resourceid(msg->info.api_info.data+4, hpl_tlv->d_len, &resourceid);

   if (rc == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_config_hs_autoextract: error getting resource-id\n");
      goto ret;
   }

   /* get the value of 'timeout' requested */
   Timeout = *(SaHpiTimeoutT *)(msg->info.api_info.data+hpl_tlv->d_len+8);
   m_NCS_CONS_PRINTF("ham_config_hs_autoextract: Timeout = %d, Command = %d\n", 
                      (uns32)Timeout, msg->info.api_info.cmd);

   /** Invoke HPI call to configure hotswap auto extract of the resource.
    **/
   if (msg->info.api_info.cmd == HS_AUTO_EXTRACT_TIMEOUT_SET)
      err =  saHpiAutoExtractTimeoutSet(ham_cb->args->session_id, resourceid, Timeout);
   else
      err =  saHpiAutoExtractTimeoutGet(ham_cb->args->session_id, resourceid, &Timeout);

   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
   {
      rc = NCSCC_RC_FAILURE;
      m_LOG_HISV_DTS_CONS("ham_config_hs_autoextract: Error in saHpiAutoExtractTimeout\n");
   }
   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;

   if (msg->info.api_info.cmd == HS_AUTO_EXTRACT_TIMEOUT_SET)
   {
     hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
     hisv_msg.info.cbk_info.hpl_ret.h_gen.data = NULL;
   }
   else
   {
      hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = arg_len;
      hisv_msg.info.cbk_info.hpl_ret.h_gen.data = m_MMGR_ALLOC_HISV_DATA(arg_len);
      m_NCS_MEMCPY(hisv_msg.info.cbk_info.hpl_ret.h_gen.data,
                   (uns8 *)&Timeout, arg_len);
   }
ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   if (msg->info.api_info.cmd != HS_INDICATOR_STATE_SET)
      m_MMGR_FREE_HISV_DATA(hisv_msg.info.cbk_info.hpl_ret.h_gen.data);
   m_LOG_HISV_DTS_CONS("HAM: ham_config_hs_autoextract, Invoked\n");

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_alarm_add
 *
 * Description   : This function process the HPL message to add the alarm in
 *                 HPI DAT.
 *                 type: HISV_ALARM_ADD.
 *
 * Arguments     : msg - HISV message from HPL, contains entity-path.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_alarm_add(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   HAM_CB  *ham_cb;
   SaErrorT       err;
   SaHpiAlarmT   *AlarmT;
   uns32 rc = NCSCC_RC_FAILURE;

   set_hisv_msg (&hisv_msg);
   m_LOG_HISV_DTS_CONS("ham_alarm_add: Invoked\n");
   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_alarm_add: error getting ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }
   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_alarm_add: No data received in HISV_MSG\n");
      goto ret;;
   }
   /* get the value of 'timeout' requested */
   AlarmT = (SaHpiAlarmT *)((uns8 *)msg->info.api_info.data+4);
   m_NCS_CONS_PRINTF("ham_alarm_add: AlarmT = %d\n", (uns32)AlarmT);

   /** Invoke HPI call to add the alarm on HPI DAT.
    **/
   err = saHpiAlarmAdd(ham_cb->args->session_id, AlarmT);

   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
   {
      m_LOG_HISV_DTS_CONS("ham_alarm_add: saHpiAlarmAdd error\n");
      rc = NCSCC_RC_FAILURE;
   }
   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data = NULL;
ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   m_LOG_HISV_DTS_CONS("HAM: ham_alarm_add, Invoked\n");

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}


/****************************************************************************
 * Name          : ham_alarm_delete
 *
 * Description   : This function process the HPL message to delete the alarm
 *                 in HPI DAT.
 *                 type: HISV_ALARM_DELETE.
 *
 * Arguments     : msg - HISV message from HPL.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_alarm_delete(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   HAM_CB  *ham_cb;
   SaErrorT       err;
   SaHpiAlarmIdT  AlarmId;
   SaHpiSeverityT SeverityT;
   uns32    rc = NCSCC_RC_FAILURE;

   set_hisv_msg (&hisv_msg);
   m_LOG_HISV_DTS_CONS("ham_alarm_delete: Invoked\n");

   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_alarm_delete: error getting ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }
   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_alarm_delete: No data received in HISV_MSG\n");
      goto ret;;
   }
   /* get the value of 'AlarmId' and SeveritT */
   AlarmId = msg->info.api_info.arg;
   SeverityT = *(SaHpiSeverityT *)((uns8 *)msg->info.api_info.data+4);
   m_NCS_CONS_PRINTF("ham_alarm_delete: AlarmId = %d\n", AlarmId);

   /** Invoke HPI call to delete the alarm from HPI DAT.
    **/
   err =  saHpiAlarmDelete(ham_cb->args->session_id, AlarmId, SeverityT);
   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
   {
      m_LOG_HISV_DTS_CONS("ham_alarm_delete: saHpiAlarmDelete returned error\n");
      rc = NCSCC_RC_FAILURE;
   }

   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data = NULL;

ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   m_LOG_HISV_DTS_CONS("HAM: ham_alarm_delete, Invoked\n");

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_alarm_get
 *
 * Description   : This function process the HPL message to get the Alarm
 *                 from the HPI DAT.
 *
 *                 type: HISV_ALARM_GET
 *
 * Arguments     : msg - HISV message from HPL.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_alarm_get(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   HAM_CB  *ham_cb;
   SaErrorT       err;
   SaHpiAlarmIdT      AlarmId;
   SaHpiAlarmT        Alarm;
   uns32 rc = NCSCC_RC_FAILURE;
   uns16 arg_len = sizeof(SaHpiAlarmT);

   set_hisv_msg(&hisv_msg);
   m_LOG_HISV_DTS_CONS("ham_alarm_get: Invoked\n");
   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_alarm_get: error getting ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }
   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_alarm_get: No data received in HISV_MSG\n");
      goto ret;
   }
   /* get the value of 'AlarmId'*/
   AlarmId = msg->info.api_info.arg;
   m_NCS_CONS_PRINTF("ham_alarm_get: given AlarmId = %d\n", AlarmId);

   /** Invoke HPI call to get the alarm from the HPI DAT.
    **/
   err = saHpiAlarmGet(ham_cb->args->session_id, AlarmId, &Alarm);

   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
   {
      m_LOG_HISV_DTS_CONS("ham_alarm_get: saHpiAlarmGet\n");
      rc = NCSCC_RC_FAILURE;
   }
   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = arg_len;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data = m_MMGR_ALLOC_HISV_DATA(arg_len);
   m_NCS_MEMCPY(hisv_msg.info.cbk_info.hpl_ret.h_gen.data,
                (uns8 *)&Alarm, arg_len);
ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   if (msg->info.api_info.cmd != HS_INDICATOR_STATE_SET)
      m_MMGR_FREE_HISV_DATA(hisv_msg.info.cbk_info.hpl_ret.h_gen.data);
   m_LOG_HISV_DTS_CONS("HAM: ham_alarm_get, Invoked\n");

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_evlog_time
 *
 * Description   : This function process the HPL message to get/set the
 *                 system event log time.
 *                 EVENTLOG_TIMEOUT_GET, and
 *                 EVENTLOG_TIMEOUT_SET
 *
 * Arguments     : msg - HISV message from HPL, contains entity-path.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_evlog_time(HISV_EVT *evt)
{
   HISV_MSG *msg = &evt->msg, hisv_msg;
   SaHpiResourceIdT   resourceid;
   HAM_CB  *ham_cb;
   SaErrorT       err;
   SaHpiTimeoutT  Timeout;
   uns16 arg_len = sizeof(SaHpiTimeoutT);
   uns32 rc = NCSCC_RC_FAILURE;
   HPL_TLV  *hpl_tlv;

   m_LOG_HISV_DTS_CONS("ham_evlog_time: get or set system event log time\n");

   set_hisv_msg(&hisv_msg);
   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_evlog_time: error getting ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      goto ret;
   }
   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_evlog_time: No data received in HISV_MSG\n");
      goto ret;
   }
   hpl_tlv = (HPL_TLV *)msg->info.api_info.data;
   rc = get_resourceid(msg->info.api_info.data+4, hpl_tlv->d_len, &resourceid);

   if (rc == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_evlog_time: error getting resource-id\n");
      goto ret;
   }
   /* get the value of 'timeout' requested */
   Timeout = *(SaHpiTimeoutT *)(msg->info.api_info.data+hpl_tlv->d_len+8);

   m_NCS_CONS_PRINTF("ham_evlog_time: saHpiEventLogTimeCmd = %d, Timeout = %d\n",
                      (uns32)Timeout, msg->info.api_info.cmd);
   /** Invoke HPI call to get or set the system event log time.
    **/
   if (msg->info.api_info.cmd == EVENTLOG_TIMEOUT_SET)
      err =  saHpiEventLogTimeSet(ham_cb->args->session_id, resourceid, Timeout);
   else
      err =  saHpiEventLogTimeGet(ham_cb->args->session_id, resourceid, &Timeout);

   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
   {
      rc = NCSCC_RC_FAILURE;
      m_LOG_HISV_DTS_CONS("ham_evlog_time: error get - set saHpiEventLogTime\n");
   }
   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val = rc;

   if (msg->info.api_info.cmd == EVENTLOG_TIMEOUT_SET)
   {
     hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
     hisv_msg.info.cbk_info.hpl_ret.h_gen.data = NULL;
   }
   else
   {
      hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = arg_len;
      hisv_msg.info.cbk_info.hpl_ret.h_gen.data = m_MMGR_ALLOC_HISV_DATA(arg_len);
      m_NCS_MEMCPY(hisv_msg.info.cbk_info.hpl_ret.h_gen.data,
                   (uns8 *)&Timeout, arg_len);
   }
ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   if (msg->info.api_info.cmd != HS_INDICATOR_STATE_SET)
      m_MMGR_FREE_HISV_DATA(hisv_msg.info.cbk_info.hpl_ret.h_gen.data);
   m_LOG_HISV_DEBUG("ham_evlog_time: ham_evlog_time, Invoked\n");
   m_NCS_CONS_PRINTF("ham_evlog_time: ham_evlog_time, Invoked, Timeout = %d\n", (uns32)Timeout);

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_health_chk_response
 *
 * Description   : This functions sends the health check response of HAM
 *                 thread to HCD process.
 *
 * Arguments     : HISV message which contained this requested command.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_health_chk_response (HISV_EVT *evt)
{
   uns32    rc = NCSCC_RC_FAILURE;
   HISV_EVT * hisv_evt;
   HISV_MSG * resp;

   m_LOG_HISV_DTS_CONS("ham_health_chk_response: HAM Health check response\n");
   /* retrieve HCD CB */
   HCD_CB * hcd_cb = (HCD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hcd_hdl);
   if (hcd_cb == NULL)
   {
      m_LOG_HISV_DTS_CONS("ham_health_chk_response: Error getting HCD CB handle\n");
      return rc;
   }

   /* response for health-check of HAM */
   /* allocate the event */
   if (NULL == (hisv_evt = m_MMGR_ALLOC_HISV_EVT))
   {
      m_LOG_HISV_DTS_CONS("ham_health_chk_response: error m_MMGR_ALLOC_HISV_EVTs\n");
      ncshm_give_hdl(gl_hcd_hdl);
      return rc;
   }
   resp = &hisv_evt->msg;
   resp->info.api_info.cmd = HISV_HAM_HEALTH_CHECK;

   /* send the request to HCD mailbox */
   if(m_NCS_IPC_SEND(&hcd_cb->mbx, hisv_evt, NCS_IPC_PRIORITY_VERY_HIGH)
                      == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_health_chk_response: failed to deliver msg on mail-box\n");
      m_MMGR_FREE_HISV_EVT(hisv_evt);
      ncshm_give_hdl(gl_hcd_hdl);
      return rc;
   }
   m_LOG_HISV_DTS_CONS("ham_health_chk_response: HAM ham_health_chk_response, Invoked\n");
  
   ncshm_give_hdl(gl_hcd_hdl);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : get_resourceid
 *
 * Description   : This functions gets the resource-id of the resource given
 *                 its entity path.
 *
 * Arguments     : entity path and its length.
 *
 * Return Values : ResourceId, NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
get_resourceid (uns8 *epath_str, uns32 epath_len, SaHpiResourceIdT *resourceid)
{
   HAM_CB  *ham_cb;
   SaHpiEntityPathT  epath;
   SaHpiEntryIdT  current, next;
   SaHpiRptEntryT  entry;
   SaErrorT       err;
   uns32    rc = NCSCC_RC_FAILURE;
#ifndef HPI_A
   uns32 i;
#endif

   m_NCS_CONS_PRINTF("get_resourceid: Entity-Path = %s\n",epath_str);
   /* m_LOG_HISV_GEN_STR("get_resourceid: given entity-path", epath_str, NCSFL_SEV_INFO); */

   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("get_resourceid: error in getting ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   /* convert the canonical entity path string to entity path structure */
   if (NCSCC_RC_FAILURE == string_to_epath(epath_str, epath_len, &epath))
   {
      /* m_LOG_HISV_GEN_STR("get_resourceid: resource does not exist for give entity-path", 
                          epath_str, NCSFL_SEV_INFO); */
      m_NCS_CONS_PRINTF("get_resourceid: resource does not exist for give entity-path %s\n",
                         epath_str);
      ncshm_give_hdl(gl_ham_hdl);
      return NCSCC_RC_FAILURE;
   }
   next = SAHPI_FIRST_ENTRY;
   do
   {
#ifdef HPI_A
      int32 len = (SAHPI_MAX_ENTITY_PATH - 2) * sizeof(SaHpiEntityT);
#else
      int32 len = (SAHPI_MAX_ENTITY_PATH - 1) * sizeof(SaHpiEntityT);
#endif

      /* locate the resource id of resource with given entity-path */
      current = next;
      err = saHpiRptEntryGet(ham_cb->args->session_id, current, &next, &entry);
      if (SA_OK != err)
      {
         if (current != SAHPI_FIRST_ENTRY)
         {
            m_LOG_HISV_DTS_CONS("get_resourceid: Error first entry\n");
            break;
         }
         else
         {
            m_LOG_HISV_DTS_CONS("get_resourceid: Empty RPT\n");
            break;
         }
      }

#ifdef HPI_A
      /* got the entry, check for matching entity path, ignore Group tuples */
      if (m_NCS_MEMCMP(&epath, (int8 *)&entry.ResourceEntity.Entry[2], len))
         continue;

      /* IR00060725: fix till we move to HPI B spec entity path mechanism */
      if ((entry.ResourceEntity.Entry[2].EntityType == SAHPI_ENT_SYSTEM_BOARD) && 
          (entry.ResourceEntity.Entry[0].EntityType != 160))
         continue;
#else
      if (!((entry.ResourceEntity.Entry[1].EntityLocation > 0) &&
            (entry.ResourceEntity.Entry[1].EntityLocation <= MAX_NUM_SLOTS) &&
            (entry.ResourceEntity.Entry[1].EntityType == SAHPI_ENT_PHYSICAL_SLOT) &&
            (
#ifdef HPI_B_02
             (entry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_PICMG_FRONT_BLADE) ||
#endif
             (entry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SWITCH_BLADE))))
      {
         /* Not a controller or a payload blade */
         continue;
      }

      /* clean entity path */
      for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++) {
         if ( entry.ResourceEntity.Entry[i].EntityType == SAHPI_ENT_ROOT ) {
            if (i == (SAHPI_MAX_ENTITY_PATH - 1))
               break;

            /* found root entry, zero out rest of Entry array */
            m_NCS_MEMSET(&entry.ResourceEntity.Entry[i+1], 0, (SAHPI_MAX_ENTITY_PATH - i - 1) * sizeof(SaHpiEntityT));
            break;
         }
      }

      if (m_NCS_MEMCMP(&epath, (int8 *)&entry.ResourceEntity.Entry[1], len) != 0)
         continue;
#endif

      /** got the resource-id of resource with given entity-path
       **/
      *resourceid = entry.ResourceId;
      m_NCS_CONS_PRINTF("get_resourceid: resource id of the entity is %d\n", entry.ResourceId);
      rc = NCSCC_RC_SUCCESS;
      break;
   } while (next != SAHPI_LAST_ENTRY);

   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : set_hisv_msg
 *
 * Description   : Default return value to HPL, in case of failure.
 *
 * Arguments     : msg - HISV message to set to default.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/

static
void set_hisv_msg(HISV_MSG *hisv_msg)
{
   hisv_msg->info.cbk_info.ret_type = HPL_GENERIC_DATA;
   hisv_msg->info.cbk_info.hpl_ret.h_gen.ret_val = NCSCC_RC_FAILURE;

   hisv_msg->info.cbk_info.hpl_ret.h_gen.data_len = 0;
   hisv_msg->info.cbk_info.hpl_ret.h_gen.data = NULL;
   return;
}

/* HPL Hot swap configuration commands */
static const HAM_EVT_REQ_HDLR ham_func_tbl[HISV_MAX_API_CMD+1] =
{
   ham_resource_power_set,  /* change the power status of resource */
   ham_resource_reset,      /* reset the resource */
   ham_sel_clear,           /* clear the HPI system event log */
   ham_chassis_id_send,     /* get the chassis id of chassis managed by HAM */

   /* HPL Hot swap configuration commands */
   ham_config_hotswap,   /* hot swap auto insert timeout get  */
   ham_config_hotswap,   /* hot swap auto insert timeout set  */
   ham_config_hs_autoextract,   /* hot swap auto extract timeout get */
   ham_config_hs_autoextract,   /* hot swap auto extract timeout set */
   ham_hs_cur_state_get,        /* hot swap current state get */
   ham_hs_indicator_state,      /* hot swap indicator state get */
   ham_hs_indicator_state,   /* hot swap indicator state set */

   /* HPL Hot swap manage commands */
   ham_manage_hotswap,        /* hot swap policy cancel */
   ham_manage_hotswap,        /* hot swap activate the resource */
   ham_manage_hotswap,        /* hot swap inactivate the resource */
   ham_manage_hotswap,        /* perform hot swap action request */

   /* HPI Alarm commands */
   ham_alarm_add,               /* Add Alarm in HPI DAT */
   ham_alarm_get,               /* Get Alarm from HPI DAT */
   ham_alarm_delete,            /* Delete Alarm from HPI DAT */

   ham_evlog_time,              /* get the event log time of resource */
   ham_evlog_time,              /* set the event log time of resource */

   ham_health_chk_response,   /* command to health check the HAM */
   ham_tmr_sel_clear,         /* clear the SEL timely */
   ham_chassis_id_resend,     /* resends chassis-id to all HPL Adests afer re-discovery */

#ifdef HPI_A
   ham_bootbank_get,          /* get the boot bank value of payload blade */
   ham_bootbank_set,          /* set the boot bank value of payload blade */
#endif
   NULL                       /* last in HISV API commands */
};

/****************************************************************************
 * Name          : ham_process_evt
 *
 * Description   : Process the received request from HPL. Identfies the mes-
 *                 command and invokes appropriate function to handle the
 *                 request. Frees the message after processing is done.
 *
 * Arguments     : msg - HISV message received on mail-box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32
ham_process_evt(HISV_EVT *evt)
{
   uns32 rc = NCSCC_RC_FAILURE;
   HISV_MSG *msg = &evt->msg;

   /* retrieve HCD CB */
   HCD_CB * hcd_cb = (HCD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hcd_hdl);
   HAM_CB * ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl);
   if ((hcd_cb == NULL) || (ham_cb == NULL))
   {
      m_LOG_HISV_DTS_CONS("ham_process_evt: error taking ham_cb handle\n");
      hisv_evt_destroy(evt);
       if (hcd_cb != NULL)
          ncshm_give_hdl(gl_hcd_hdl);
       if (ham_cb != NULL)
          ncshm_give_hdl(gl_ham_hdl);
      return NCSCC_RC_FAILURE;
   }

   /* if this is a stand-by component and not for getting chassis-id, return */
   if (((hcd_cb->ha_state != SA_AMF_HA_ACTIVE) || (hcd_cb->args->rediscover) ||
       (!hcd_cb->args->session_valid)) && (msg->info.api_info.cmd != HISV_CHASSIS_ID_GET))
   {
      HISV_MSG hisv_msg;
      set_hisv_msg (&hisv_msg);
      /* send the message to HPL ADEST */
      rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                             MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
      m_LOG_HISV_DTS_CONS("ham_process_evt: HPI session re-initialization not yet complete\n");
      hisv_evt_destroy(evt);
      ncshm_give_hdl(gl_hcd_hdl);
      ncshm_give_hdl(gl_ham_hdl);
      return rc;
   }
   /* update ADEST list of HPL users in cases of chassis id get */
   if (msg->info.api_info.cmd == HISV_CHASSIS_ID_GET)
      ham_adest_update(msg, ham_cb);

   /* handles not required now */
   ncshm_give_hdl(gl_hcd_hdl);
   ncshm_give_hdl(gl_ham_hdl);

   m_LOG_HISV_DTS_CONS("ham_process_evt: Invoked\n");
   m_NCS_CONS_PRINTF("ham_process_evt: command = %d\n",msg->info.api_info.cmd);
   if ((msg->info.api_info.cmd < 0) ||
       (msg->info.api_info.cmd >= HISV_MAX_API_CMD))
   {
      m_LOG_HISV_DEBUG("ham_process_evt: Unknow request from HPL client\n");
      m_NCS_CONS_PRINTF("ham_process_evt: Unknow request %d from HPL client\n", msg->info.api_info.cmd);
      hisv_evt_destroy(evt);
      return NCSCC_RC_FAILURE;
   }
   if (ham_func_tbl[msg->info.api_info.cmd] == NULL)
   {
      m_LOG_HISV_DEBUG("ham_process_evt: No HAM handler installed for this HPL cmd\n");
      m_NCS_CONS_PRINTF("ham_process_evt: No HAM handler installed for the HPL cmd %d\n", msg->info.api_info.cmd);
      hisv_evt_destroy(evt);
      return NCSCC_RC_FAILURE;
   }
   /** Dispatch the request **/
   rc = (*ham_func_tbl[msg->info.api_info.cmd]) (evt);
   hisv_evt_destroy(evt);
   return rc;
}


/****************************************************************************
 * Name          : ham_adest_update
 *
 * Description   : Update list of HPL Adests
 *
 * Arguments     : msg - HISV and HAM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ham_adest_update(HISV_MSG *msg, HAM_CB *ham_cb)
{
   HAM_ADEST_LIST  * dest;
   m_LOG_HISV_DTS_CONS("ham_adest_update: Updating Adest List\n");

   /* check data availability in message. It holds HPL ADEST */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_adest_update: No data received in HISV_MSG\n");
      return NCSCC_RC_FAILURE;
   }
   dest = ham_cb->dest;
   while (dest != NULL)
   {
      if (!m_NCS_MEMCMP(&dest->addr, msg->info.api_info.data, sizeof(MDS_DEST)))
      {
         return NCSCC_RC_SUCCESS;
      }
      dest = dest->next;
   }
   dest = m_MMGR_ALLOC_HAM_ADEST;
   if (dest == NULL)
      return NCSCC_RC_FAILURE;

   m_NCS_MEMCPY(&dest->addr, msg->info.api_info.data, sizeof(MDS_DEST));
   dest->next = ham_cb->dest;
   ham_cb->dest = dest;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : ham_chassis_id_resend
 *
 * Description   : This function makes the HISv message and resends the
 *                 chassis-id of chassis managed by HAM, to HPL registered
 *                 MDS ADEST. This is required after re-discovery
 *
 * Arguments     : msg - HISV message received on mail-box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_chassis_id_resend(HISV_EVT *evt)
{
   HAM_CB  *ham_cb;
   HISV_MSG  hisv_msg;
   uns32    rc;
   HAM_ADEST_LIST  *dest, *prev;

   m_LOG_HISV_DTS_CONS("ham_chassis_id_resend: HAM Re-Sending chassis-id to HPL\n");

   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_chassis_id_send: Failed to get ham_cb handle\n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   dest = ham_cb->dest;
   while (dest != NULL)
   {
      /* store the HPL ADEST */
      ham_cb->hpl_dest = dest->addr;
      /** populate the mds message with chassis_id, to send across to the HPL
       **/
      hisv_msg.info.cbk_info.ret_type = HPL_HAM_VDEST_RET;
      m_HAM_HISV_CHASSIS_ID_FILL(hisv_msg, ham_cb->args->chassis_id,
                                 ham_cb->ham_vdest, HAM_MDS_UP);

      /* send the message containing chassis_id, to HPL ADEST */
      rc = ham_mds_msg_send (ham_cb, &hisv_msg, &ham_cb->hpl_dest,
                             MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_SND, evt);
      prev = dest;
      dest = dest->next;
      m_MMGR_FREE_HAM_ADEST(prev);
   }
   ham_cb->dest = NULL;
   /* give control block handle */
   ncshm_give_hdl(gl_ham_hdl);

   return NCSCC_RC_SUCCESS;
}


#ifdef HPI_A
/****************************************************************************
 * Name          : ham_bootbank_get
 *
 * Description   : This function process the HPL message to get the current
 *                 boot bank value of a payload blade.
 *
 *                 type: HISV_BOOTBANK_GET
 *
 * Arguments     : evt - HISV message from HPL, contains details of request.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
ham_bootbank_get (HISV_EVT *evt)
{
   HISV_MSG          *msg = &evt->msg, hisv_msg = {0};
   SaHpiResourceIdT   resourceid;
   HAM_CB            *ham_cb = NULL;
   SaErrorT           err;
   SaHpiCtrlStateT    state;
   uns32              rc = NCSCC_RC_FAILURE;
   uns16              arg_len = sizeof(SaHpiCtrlStateDiscreteT);
   SaHpiCtrlStateDiscreteT discrete_val = 0;

   set_hisv_msg(&hisv_msg);
   m_LOG_HISV_DTS_CONS("ham_bootbank_get : Invoked\n");
   
   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_bootbank_get : error taking ham_cb handle\n");
      return NCSCC_RC_FAILURE;
   }

   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_bootbank_get : No data received in HISV_MSG\n");
      goto ret;
   }
   rc = get_resourceid(msg->info.api_info.data+4, msg->info.api_info.data_len, &resourceid);
   if (rc == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_bootbank_get : error getting resource-id\n");
      goto ret;
   }

   /** Invoke HPI call to get the current boot bank of the payload blade.
    **/
   err = saHpiControlStateGet (ham_cb->args->session_id, resourceid,
                               CTRL_NUM_BOOTBANK, &state);
   m_NCS_CONS_PRINTF("ham_bootbank_get: state = %d, err = %d\n", state.StateUnion.Discrete, err);

   if (SA_OK == err)
      rc = NCSCC_RC_SUCCESS;
   else
      rc = NCSCC_RC_FAILURE;

   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type               = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val  = rc;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = arg_len;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data     = m_MMGR_ALLOC_HISV_DATA(arg_len);
   discrete_val                                  = state.StateUnion.Discrete;
  
   /* Extract the payload blade's Boot bank value from discrete_val.
      The value we get from HPI contains both Set Selector and Boot bank values.
      As we need only boot bank info, we are extracting that here. */  
   discrete_val = discrete_val & 0x1;  
                                     
   m_NCS_MEMCPY(hisv_msg.info.cbk_info.hpl_ret.h_gen.data,
                (uns8 *)&discrete_val, arg_len);
ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);

   if (msg->info.api_info.cmd != HISV_BOOTBANK_SET)
      m_MMGR_FREE_HISV_DATA(hisv_msg.info.cbk_info.hpl_ret.h_gen.data);

   m_LOG_HISV_DTS_CONS("HAM: ham_bootbank_get, Invoked and done\n");
   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

/****************************************************************************
 * Name          : ham_bootbank_set
 *
 * Description   : This function process the HPL message to set the boot bank
 *                 of any payload blade.
 *
 *                 type: HISV_BOOTBANK_SET
 *
 * Arguments     : evt - HISV message from HPL, contains details of request.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_bootbank_set (HISV_EVT *evt)
{
   HISV_MSG           *msg = &evt->msg, hisv_msg = {0};
   HAM_CB             *ham_cb = NULL;
   SaErrorT            err;
   SaHpiResourceIdT    resourceid;
   uns8                i_boot_bank_number;
   uns32               rc = NCSCC_RC_FAILURE;
     
   SaHpiCtrlNumT       ctrlNum;
   SaHpiCtrlStateT     state;

   m_LOG_HISV_DTS_CONS("ham_bootbank_set : Invoked\n");
   set_hisv_msg (&hisv_msg);

   /** retrieve HAM CB
    **/
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("ham_bootbank_set : error taking ham_cb handle\n");
      return NCSCC_RC_FAILURE;
   }

   /* check data availability in message. It contains entity-path of resource */
   if ((NULL == msg->info.api_info.data) || (msg->info.api_info.data_len == 0))
   {
      m_LOG_HISV_DTS_CONS("ham_bootbank_set : No data received in HISV_MSG\n");
      goto ret;
   }

   rc = get_resourceid(msg->info.api_info.data+4, msg->info.api_info.data_len, &resourceid);
   if (rc == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_bootbank_set : error getting resource-id\n");
      goto ret;
   }
   /* get the boot bank number to be set */
   i_boot_bank_number = (SaHpiCtrlNumT)msg->info.api_info.arg;
   m_NCS_CONS_PRINTF("ham_bootbank_set : boot bank number = %d\n", i_boot_bank_number);

   /** got the resource-id of resource with given entity-path
    ** Invoke HPI call to set the boot bank of payload blade.
    **/

    ctrlNum                   = CTRL_NUM_BOOTBANK;
    state.Type                = SAHPI_CTRL_TYPE_DISCRETE;
    state.StateUnion.Discrete = i_boot_bank_number;

    err = saHpiControlStateSet (ham_cb->args->session_id, resourceid, ctrlNum, &state);

    if (SA_OK == err)
       rc = NCSCC_RC_SUCCESS;
    else
       rc = NCSCC_RC_FAILURE;

   /** populate the mds message with return value, to send across to the HPL
    **/
   hisv_msg.info.cbk_info.ret_type               = HPL_GENERIC_DATA;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data     = NULL;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.data_len = 0;
   hisv_msg.info.cbk_info.hpl_ret.h_gen.ret_val  = rc;

ret:
   /* send the message to HPL ADEST */
   rc = ham_mds_msg_send (ham_cb, &hisv_msg, &evt->fr_dest,
                                MDS_SEND_PRIORITY_HIGH, MDS_SENDTYPE_RSP, evt);
   m_LOG_HISV_DTS_CONS("ham_bootbank_set : Invoked and done\n");

   /* give handle */
   ncshm_give_hdl(gl_ham_hdl);
   return rc;
}

#endif





