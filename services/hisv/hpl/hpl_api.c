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
*  MODULE NAME:  hpl_api.c                                                   *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the private library of APIs which can be used be HPI *
*  clients to interface with HPI. HPI clients like AvSv and SPSv can use the *
*  APIs to perform operations like reset of a node or a card on a chassis.   *
*  Also the API 'hpl_sel_clear()' can be used by any administrative entity   *
*  in order to clear the HPI's system event log.                             *
*                                                                            *
*****************************************************************************/

#include <config.h>

#include "hpl.h"

extern uns32 gl_hpl_mds_timeout;

/****************************************************************************
 * Name          : hpl_resource_reset
 *
 * Description   : This functions sents the request to HAM to reset the
 *                 resource identified by the 'entity_path' and located on
 *                 the chassis identified by 'chassis_id'. It first locates
 *                 the instance of HAM which is managing the chassis identified
 *                 by 'chassis_id' and sends the request to its MDS VDEST.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *                 entity_path - canonical string of entity path of resource
 *                               which should be resetted.
 *                 reset_type - type of reset (cold, warm, assertive,
 *                              de-assertive.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_resource_reset(uns32 chassis_id, uns8 *entity_path, uns32 reset_type)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns16     epath_len;
   uns32     rc;
   uns8     *hpl_data;
   HPL_TLV  *hpl_tlv;

   /* validate entity path */
   if ((entity_path == NULL) || ((epath_len = (uns16)strlen(entity_path)) == 0) )
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   /* validate reset_type */
   if (reset_type > HISV_RES_GRACEFUL_REBOOT) {
      /* invalid reset type */
      m_LOG_HISV_DEBUG("bad reset type requested\n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   /* Add 1 extra byte to the epath_len so the NULL-termination char is copied over. */
   epath_len++;

   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }

   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(epath_len+4);
   hpl_tlv = (HPL_TLV *)hpl_data;

   hpl_tlv->d_type = ENTITY_PATH;
   hpl_tlv->d_len = epath_len;

   memcpy(hpl_data+4, entity_path, epath_len);

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, HISV_RESOURCE_RESET, reset_type,
                              (epath_len + 4), hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);
   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   free_hisv_ret_msg(msg);

   return rc;
}


/****************************************************************************
 * Name          : hpl_resource_power_set
 *
 * Description   : This functions sents the request to HAM to change the
 *                 power status of resource identified by the 'entity_path'
 *                 and located on the chassis identified by 'chassis_id'.
 *                 It first locates the instance of HAM which is managing
 *                 the chassis identified by 'chassis_id' and sends the
 *                 request to its MDS VDEST.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *                 entity_path - canonical string of entity path of resource
 *                               which should be resetted.
 *                 power_state - required power state of resource (ON, OFF,
 *                               cycle)
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_resource_power_set(uns32 chassis_id, uns8 *entity_path, uns32 power_state)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc;
   uns16 epath_len;
   uns8     *hpl_data;
   HPL_TLV  *hpl_tlv;

   /* validate entity path */
   if ((entity_path == NULL) || ((epath_len = (uns16)strlen(entity_path)) == 0) )
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   /* validate the requested power state */
   if ( power_state > HISV_RES_POWER_CYCLE )
   {
      /* invalid power state */
      m_LOG_HISV_DEBUG("bad power state requested\n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   /* Add 1 extra byte to the epath_len so the NULL-termination char is copied over. */
   epath_len++;

   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }

   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(epath_len+4);
   hpl_tlv = (HPL_TLV *)hpl_data;

   hpl_tlv->d_type = ENTITY_PATH;
   hpl_tlv->d_len = epath_len;

   memcpy(hpl_data+4, entity_path, epath_len);

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, HISV_RESOURCE_POWER, power_state,
                              (epath_len+4), hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);
   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   free_hisv_ret_msg(msg);

   return rc;
}


/****************************************************************************
 * Name          : hpl_sel_clear
 *
 * Description   : This function can be used by administrative entities to
 *                 clear the system even log of the HPI managing the chassis
 *                 identified by chassis_id. It sends the syslog clear request
 *                 to HAM which is managing the chassis identified by chassis_id.
 *
 * Arguments     : chassis_id - chassis identifier of chassis for which the
 *                              clearance of HPI system event log is requested.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_sel_clear(uns32 chassis_id)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc = NCSCC_RC_SUCCESS;

   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   /** populate the mds message to send across to the HAM
    **/
   m_HPL_HISV_LOG_CMD_MSG_FILL(hisv_msg, HISV_CLEAR_SEL);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   free_hisv_ret_msg(msg);

   return rc;
}

/****************************************************************************
 * Name          : hpl_config_hotswap
 *
 * Description   : This functions can be used by AvM to configure the hot
 *                 swap timer values or get the currently configured values.
 *                 Hot swap commands listed with the enum hotswap_config_cmd
 *                 can be used with this API.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *                 entity_path - canonical string of entity path of resource
 *                               which should be resetted.
 *                 hs_config_cmd - hot swap config command:
 *                                 HS_AUTO_INSERT_TIMEOUT_SET,
 *                                 HS_AUTO_INSERT_TIMEOUT_GET.
 *                 arg - argument to the hs_config_cmd.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_config_hotswap(uns32 chassis_id, HISV_API_CMD hs_config_cmd, uns64 *arg)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc, len=0;
   uns16     arg_len = 8, ret_len=0;
   uns8     *hpl_data;
   HPL_TLV  *hpl_tlv;

   if ((hs_config_cmd != HS_AUTO_INSERT_TIMEOUT_SET)
        || (hs_config_cmd != HS_INDICATOR_STATE_SET))
   {
      printf("Invalid hotswap config command %d\n", hs_config_cmd);
      m_LOG_HISV_DEBUG("Invalid hotswap config command \n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(arg_len+4);

   hpl_tlv = (HPL_TLV *)hpl_data;
   hpl_tlv->d_type = HOTSWAP_CONFIG;
   hpl_tlv->d_len = arg_len;
   len = arg_len + 4;
   memcpy(((uns8 *)hpl_tlv)+4, (uns8 *)arg, arg_len);

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, hs_config_cmd, hs_config_cmd,
                                        len, hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);
   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   ret_len = msg->info.cbk_info.hpl_ret.h_gen.data_len;

   if ((hs_config_cmd == HS_AUTO_INSERT_TIMEOUT_GET) && (ret_len > 0))
      memcpy((uns8 *)arg, (uns8 *)(msg->info.cbk_info.hpl_ret.h_gen.data), ret_len);
   free_hisv_ret_msg(msg);

   return rc;
}

/****************************************************************************
 * Name          : hpl_config_hs_indicator
 *
 * Description   : This functions can be used by AvM to configure the hot
 *                 swap auto extract timer values or get the currently configured
 *                 value. Hot swap commands listed with the enum HISV_API_CMD
 *                 can be used with this API.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *
 *                 hs_ind_cmd - hot swap config command:
 *                                 HS_INDICATOR_STATE_GET
 *                                 HS_INDICATOR_STATE_SET
 *                 arg - pointer argument to the hpl_config_hs_indicator.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_config_hs_indicator(uns32 chassis_id, uns8 *entity_path,
                         HISV_API_CMD hs_ind_cmd, uns32 *arg)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc, len=0, ret_len=0;
   uns16 epath_len;
   uns8     *hpl_data;
   HPL_TLV  *hpl_tlv;

   /* validate entity path */
   if ((entity_path == NULL) || ((epath_len = (uns16)strlen(entity_path)) == 0) )
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /* Add 1 extra byte to the epath_len so the NULL-termination char is copied over. */
   epath_len++;
   if ((hs_ind_cmd != HS_INDICATOR_STATE_GET)
        && (hs_ind_cmd != HS_INDICATOR_STATE_SET))
   {
      printf("Invalid hotswap indicator config command %d\n", hs_ind_cmd);
      m_LOG_HISV_DEBUG("Invalid hotswap indicator config command \n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(epath_len+4);

   hpl_tlv = (HPL_TLV *)hpl_data;
   hpl_tlv->d_type = ENTITY_PATH;
   hpl_tlv->d_len = epath_len;
   memcpy(hpl_data+4, entity_path, epath_len);
   len = epath_len+4;

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, hs_ind_cmd, (*arg),
                                        len, hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   ret_len = msg->info.cbk_info.hpl_ret.h_gen.data_len;

   if ((hs_ind_cmd == HS_INDICATOR_STATE_GET) && (ret_len > 0))
      *arg = *(uns32 *)(msg->info.cbk_info.hpl_ret.h_gen.data);

   free_hisv_ret_msg(msg);

   return rc;
}

/****************************************************************************
 * Name          : hpl_config_hs_state_get
 *
 * Description   : This functions can be used by AvM to get the current hot
 *                 swap state of a resource.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *
 *                 hs_ind_cmd - hot swap config command:
 *                                 HS_CURRENT_HS_STATE_GET
 *                 arg - pointer argument to the hpl_config_hs_state_get.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_config_hs_state_get(uns32 chassis_id, uns8 *entity_path, uns32 *arg)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc, len=0, ret_len = 0;
   uns16 epath_len;
   uns8     *hpl_data;
   HPL_TLV  *hpl_tlv;

   /* validate entity path */
   if ((entity_path == NULL) || ((epath_len = (uns16)strlen(entity_path)) == 0) )
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /* Add 1 extra byte to the epath_len so the NULL-termination char is copied over. */
   epath_len++;
   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(epath_len+4);

   hpl_tlv = (HPL_TLV *)hpl_data;
   hpl_tlv->d_type = ENTITY_PATH;
   hpl_tlv->d_len = epath_len;
   memcpy(hpl_data+4, entity_path, epath_len);
   len = epath_len+4;

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, HS_CURRENT_HS_STATE_GET, (*arg),
                                        len, hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   ret_len = msg->info.cbk_info.hpl_ret.h_gen.data_len;

   if (ret_len > 0)
      *arg = *(uns32 *)(msg->info.cbk_info.hpl_ret.h_gen.data);

   free_hisv_ret_msg(msg);

   return rc;
}

/****************************************************************************
 * Name          : hpl_config_hs_autoextract
 *
 * Description   : This functions can be used by AvM to configure the hot
 *                 swap auto extract timer values or get the currently configured
 *                 value. Hot swap commands listed with the enum HISV_API_CMD
 *                 can be used with this API.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *
 *                 hs_config_cmd - hot swap config command:
 *                                 HS_AUTO_EXTRACT_TIMEOUT_GET,
 *                                 HS_AUTO_EXTRACT_TIMEOUT_SET,
 *                 arg - pointer argument to the hs_config_hs_autoextract.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_config_hs_autoextract(uns32 chassis_id, uns8 *entity_path,
                         HISV_API_CMD hs_config_cmd, uns64 *arg)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc, len=0;
   uns16 epath_len, arg_len = 8, ret_len = 0;
   uns8     *hpl_data;
   HPL_TLV  *hpl_tlv;

   /* validate entity path */
   if ((entity_path == NULL) || ((epath_len = (uns16)strlen(entity_path)) == 0) )
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /* Add 1 extra byte to the epath_len so the NULL-termination char is copied over. */
   epath_len++;
   if ((hs_config_cmd != HS_AUTO_EXTRACT_TIMEOUT_GET)
        && (hs_config_cmd != HS_AUTO_EXTRACT_TIMEOUT_SET))
   {
      printf("Invalid hotswap config command %d\n", hs_config_cmd);
      m_LOG_HISV_DEBUG("Invalid hotswap config command \n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(epath_len+arg_len+8);

   hpl_tlv = (HPL_TLV *)hpl_data;
   hpl_tlv->d_type = ENTITY_PATH;
   hpl_tlv->d_len = epath_len;
   memcpy(hpl_data+4, entity_path, epath_len);
   len = epath_len+4;

   hpl_tlv = (HPL_TLV *)(hpl_data + epath_len + 4);
   hpl_tlv->d_type = HOTSWAP_CONFIG;
   hpl_tlv->d_len = arg_len;
   len += arg_len + 4;
   memcpy(((uns8 *)hpl_tlv)+4, (uns8 *)arg, arg_len);

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, hs_config_cmd, hs_config_cmd,
                                        len, hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   ret_len = msg->info.cbk_info.hpl_ret.h_gen.data_len;

   if ((hs_config_cmd == HS_AUTO_EXTRACT_TIMEOUT_GET) && (ret_len > 0))
      memcpy((uns8 *)arg, (uns8 *)(msg->info.cbk_info.hpl_ret.h_gen.data), 8);

   free_hisv_ret_msg(msg);

   return rc;
}


/****************************************************************************
 * Name          : hpl_manage_hotswap
 *
 * Description   : This functions can be used by AvM to Manage the hot
 *                 swap states of the fru resources.
 *                 Hot swap commands listed with the enum hotswap_manage_cmd
 *                 can be used with this API.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *                 entity_path - canonical string of entity path of resource
 *                               which should be resetted.
 *                 hs_config_cmd - hot swap config command:
 *                                 HS_POLICY_CANCEL,
 *                                 HS_RESOURCE_ACTIVE_SET,
 *                                 HS_RESOURCE_INACTIVE_SET,
 *                                 HS_ACTION_REQUEST.
 *                 arg - argument to the hs_config_cmd.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_manage_hotswap(uns32 chassis_id, uns8 *entity_path,
                         HISV_API_CMD hs_manage_cmd, uns32 arg)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc, len=0;
   uns16 epath_len;
   uns8     *hpl_data;
   HPL_TLV  *hpl_tlv;

   /* validate entity path */
   if ((entity_path == NULL) || ((epath_len = (uns16)strlen(entity_path)) == 0) )
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /* Add 1 extra byte to the epath_len so the NULL-termination char is copied over. */
   epath_len++;
   if ((hs_manage_cmd != HS_POLICY_CANCEL)
        && (hs_manage_cmd != HS_RESOURCE_ACTIVE_SET)
        && (hs_manage_cmd != HS_RESOURCE_INACTIVE_SET)
        && (hs_manage_cmd != HS_ACTION_REQUEST))
   {
      printf("Invalid hotswap manage command %d\n", hs_manage_cmd);
      m_LOG_HISV_DEBUG("Invalid hotswap manage command \n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(epath_len+4);

   hpl_tlv = (HPL_TLV *)hpl_data;
   hpl_tlv->d_type = ENTITY_PATH;
   hpl_tlv->d_len = epath_len;
   memcpy(hpl_data+4, entity_path, epath_len);
   len = epath_len+4;

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, hs_manage_cmd, arg,
                                        len, hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   free_hisv_ret_msg(msg);

   return rc;
}


/****************************************************************************
 * Name          : hpl_alarm_add
 *
 * Description   : This functions can be used to add the Alarm on HPI DAT.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *                 alarm_cmd - Alarm Add command:
 *                             HISV_ALARM_ADD
 *                 arg - argument 'alarmT'.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_alarm_add(uns32 chassis_id, HISV_API_CMD alarm_cmd,
                         uns16 arg_len, uns8* arg)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc, len=0;
   uns8     *hpl_data;
   HPL_TLV  *hpl_tlv;

   if ((alarm_cmd != HISV_ALARM_ADD) || (arg == NULL))
   {
      printf("Invalid hpl_alarm_add command %d\n", alarm_cmd);
      m_LOG_HISV_DEBUG("Invalid hpl_alarm_add command \n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(arg_len+4);

   hpl_tlv = (HPL_TLV *)hpl_data;
   hpl_tlv->d_type = ALARM_TYPE;
   hpl_tlv->d_len = arg_len;
   len = arg_len + 4;
   memcpy(((uns8 *)hpl_tlv)+4, arg, arg_len);

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, alarm_cmd, alarm_cmd,
                                        len, hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   free_hisv_ret_msg(msg);

   return rc;
}

/****************************************************************************
 * Name          : hpl_alarm_get
 *
 * Description   : This functions can be used to add the Alarm on HPI DAT.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *                 alarm_cmd -  Alarm Add command:
 *                              HISV_ALARM_GET
 *                 alarm_id  -  alarm identifier to receive the alarm.
 *                 arg - argument to receive alarm.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_alarm_get(uns32 chassis_id, HISV_API_CMD alarm_cmd, uns32 alarm_id,
                         uns16 arg_len, uns8* arg)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc, ret_len = 0;

   if ((alarm_cmd != HISV_ALARM_GET) || (arg == NULL))
   {
      printf("Invalid hpl_alarm_add command %d\n", alarm_cmd);
      m_LOG_HISV_DEBUG("Invalid hpl_alarm_add command \n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, alarm_cmd, alarm_id, 0, 0);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   ret_len = msg->info.cbk_info.hpl_ret.h_gen.data_len;
   if (ret_len > 0)
      memcpy(arg, msg->info.cbk_info.hpl_ret.h_gen.data, arg_len);
   free_hisv_ret_msg(msg);

   return rc;
}

/****************************************************************************
 * Name          : hpl_alarm_delete
 *
 * Description   : This functions can be used to add the Alarm on HPI DAT.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *                 alarm_cmd -  Alarm Add command:
 *                              HISV_ALARM_DELETE
 *                 arg - argument to the hs_alarm_cmd.
 *                 alarm_severity - severity of alarms to be deleted.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_alarm_delete(uns32 chassis_id, HISV_API_CMD alarm_cmd, uns32 alarm_id,
                         uns32 alarm_severity)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc, len=0;
   uns8     *hpl_data;
   HPL_TLV  *hpl_tlv;

   if (alarm_cmd != HISV_ALARM_DELETE)
   {
      printf("Invalid hpl_alarm_add command %d\n", alarm_cmd);
      m_LOG_HISV_DEBUG("Invalid hpl_alarm_add command \n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(8);

   hpl_tlv = (HPL_TLV *)hpl_data;
   hpl_tlv->d_type = ALARM_SEVERITY;
   hpl_tlv->d_len = 4;
   len = 8;
   memcpy(((uns8 *)hpl_tlv)+4, (uns8 *)&alarm_severity, 4);

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, alarm_cmd, alarm_id,
                                        len, hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   free_hisv_ret_msg(msg);

   return rc;
}


/****************************************************************************
 * Name          : hpl_event_log_time
 *
 * Description   : This functions can be used to get or set the event log
 *                 time stamp of a resource.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *
 *                 evlog_time_cmd - evlog_time_cmd time get/set:
 *                                 EVENTLOG_TIMEOUT_GET,
 *                                 EVENTLOG_TIMEOUT_SET,
 *                 arg - pointer argument to the get/set evt_log_time.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_event_log_time(uns32 chassis_id, uns8 *entity_path,
                         HISV_API_CMD evlog_time_cmd, uns64 *arg)
{
   HPL_CB   *hpl_cb;
   MDS_DEST  ham_dest;
   HISV_MSG  hisv_msg, *msg;
   uns32     rc, len=0;
   uns16 epath_len, arg_len = 8, ret_len = 0;
   uns8     *hpl_data;
   HPL_TLV  *hpl_tlv;

   /* validate entity path */
   if ((entity_path == NULL) || ((epath_len = (uns16)strlen(entity_path)) == 0) )
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /* Add 1 extra byte to the epath_len so the NULL-termination char is copied over. */
   epath_len++;
   if ((evlog_time_cmd != EVENTLOG_TIMEOUT_GET)
        && (evlog_time_cmd != EVENTLOG_TIMEOUT_SET))
   {
      printf("Invalid hpl_event_log_time command %d\n", evlog_time_cmd);
      m_LOG_HISV_DEBUG("Invalid hpl_event_log_time command \n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(epath_len+arg_len+8);

   hpl_tlv = (HPL_TLV *)hpl_data;
   hpl_tlv->d_type = ENTITY_PATH;
   hpl_tlv->d_len = epath_len;
   memcpy(hpl_data+4, entity_path, epath_len);
   len = epath_len+4;

   hpl_tlv = (HPL_TLV *)(hpl_data + epath_len + 4);
   hpl_tlv->d_type = EVLOG_TIME;
   hpl_tlv->d_len = arg_len;
   len += arg_len + 4;
   memcpy(((uns8 *)hpl_tlv)+4, (uns8 *)arg, arg_len);

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, evlog_time_cmd, evlog_time_cmd,
                                        len, hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);
   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   ret_len = msg->info.cbk_info.hpl_ret.h_gen.data_len;

   if ((evlog_time_cmd == EVENTLOG_TIMEOUT_GET) && (ret_len > 0))
      *arg = *(uns64 *)(msg->info.cbk_info.hpl_ret.h_gen.data);

   free_hisv_ret_msg(msg);

   return rc;
}

/****************************************************************************
 * Name          : hpl_chassi_id_get
 *
 * Description   : This functions can be used to get the chassis-id;
 *                 only in case of single chassis system
 *
 *                 arg - pointer argument to the hpl_chassis_id_get.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE. chassis-id in *arg.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_chassi_id_get(uns32 *arg)
{
   HPL_CB   *hpl_cb;
   HAM_INFO *ham_inst;

   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      *arg = -1;
      printf("Could not retrieve chassis-id. Failed to get HPL control block.\n");
      return NCSCC_RC_FAILURE;
   }
   /* if MDS registration with HCD is done, we'd have got chassis-id */
   ham_inst = hpl_cb->ham_inst;
   if (ham_inst == NULL)
   {
      printf("Could not retrieve chassis-id. HPL not yet registered with HCD.\n");
      *arg = -1;
      ncshm_give_hdl(gl_hpl_hdl);
      return NCSCC_RC_FAILURE;
   }
   *arg = ham_inst->chassis_id;
   ncshm_give_hdl(gl_hpl_hdl);
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : hpl_bootbank_get
 *
 * Description   : This functions can be used by AvM to get the current 
 *                 boot bank value of a payload blade.
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *                 entity_path - canonical string of entity path of resource
 *                                 which should be resetted.
 *                 o_bootbank_number  - pointer argument to the 
 *                              hpl_bootbank_get that will contain output.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 
hpl_bootbank_get (uns32 chassis_id, uns8 *entity_path, uns8 *o_bootbank_number)
{
   HPL_CB   *hpl_cb = NULL;
   MDS_DEST  ham_dest = {0};
   HISV_MSG  hisv_msg, *msg = NULL;
   uns32     rc, len = 0, ret_len = 0;
   uns16     epath_len;
   uns8     *hpl_data = NULL;
   HPL_TLV  *hpl_tlv = NULL;
   uns32     status_len = sizeof(uns32);
   memset(&hisv_msg, 0, sizeof(hisv_msg));
   /* validate entity path */
   if ((entity_path == NULL) || ((epath_len = (uns16)strlen(entity_path)) == 0) )
   {
      printf("hpl_bootbank_get : Error - Entity path supplied is wrong\n");
      return NCSCC_RC_FAILURE;
   } 
   /* Add 1 extra byte to the epath_len so the NULL-termination char is copied over. */
   epath_len++;
   
   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   { 
      printf("hpl_bootbank_get : Error - Could not retrieve hpl_cb\n");
      return NCSCC_RC_FAILURE;
   }
   
    /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("hpl_bootbank_get : No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }
   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(epath_len + status_len);

   hpl_tlv         = (HPL_TLV *)hpl_data;
   hpl_tlv->d_type = ENTITY_PATH;
   hpl_tlv->d_len  = epath_len;
   memcpy (hpl_data + status_len, entity_path, epath_len);
   len             = epath_len + status_len;

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, HISV_BOOTBANK_GET, (*o_bootbank_number),
                                        len, hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);

   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc      = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   ret_len = msg->info.cbk_info.hpl_ret.h_gen.data_len;

   if (ret_len > 0)
   {
      *o_bootbank_number = *(uns32*)(msg->info.cbk_info.hpl_ret.h_gen.data);
      printf("hpl_bootbank_get : o_bootbank_number : %d\n",*o_bootbank_number);
   }
   else
   {
     printf("\nhpl_bootbank_get : Error - ret_len is less than 0\n");
   } 

   free_hisv_ret_msg(msg);

   return rc;
}

/****************************************************************************
 * Name          : hpl_bootbank_set
 *
 * Description   : This functions can be used by AvM to set the boot 
 *                 bank value of a payload blade (Bank-A / Bank-B)
 *
 * Arguments     : chassis_id - chassis-id of chassis on which resource is
 *                              located.
 *                 entity_path - canonical string of entity path of resource
 *                                 which should be resetted.
 *                 i_bootbank_number  - new boot bank value that needs
 *                                       to be set.      
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 
hpl_bootbank_set (uns32 chassis_id, uns8 *entity_path, uns8 i_bootbank_number)
{
   HPL_CB   *hpl_cb = NULL;
   MDS_DEST  ham_dest = {0}; 
   HISV_MSG  hisv_msg, *msg = NULL;
   uns16     epath_len;
   uns32     rc;
   uns8     *hpl_data = NULL;
   HPL_TLV  *hpl_tlv  = NULL;
   uns32     status_len = sizeof(uns32);

   memset(&hisv_msg, 0, sizeof(hisv_msg));
   /* validate entity path */
   if ((entity_path == NULL) || ((epath_len = (uns16)strlen(entity_path)) == 0) )
   {
      printf("hpl_bootbank_set : Error - Entity path supplied is wrong\n");
      return NCSCC_RC_FAILURE;
   }
   /* Add 1 extra byte to the epath_len so the NULL-termination char is copied over. */
   epath_len++;

   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      printf("hpl_bootbank_set : Error - Could not retrieve hpl_cb\n");
      return NCSCC_RC_FAILURE;
   }
   /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("hpl_bootbank_set : No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }

   /** populate the mds message to send across to the HAM
    **/
   hpl_data = m_MMGR_ALLOC_HPL_DATA(epath_len + status_len);
   hpl_tlv  = (HPL_TLV *)hpl_data;

   hpl_tlv->d_type = ENTITY_PATH;
   hpl_tlv->d_len  = epath_len;

   memcpy(hpl_data + status_len, entity_path, epath_len);

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, HISV_BOOTBANK_SET, i_bootbank_number,
                              (epath_len + status_len), hpl_data);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);

   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   free_hisv_ret_msg(msg);

   return rc;
}



/*************************************************************************
 * Function:  hpl_decode_hisv_evt
 *
 * Purpose:   program handler for HPI_HISV_EVT_T. This is invoked when HPL 
 *            has to perform Decode on HPI_HISV_EVT_T after receiving
 *
 * Input    :
 *          : input buffer and structure to be decoded in to
 * Output   :
 #
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 *
 * NOTES    :
 *
 ************************************************************************/ 

uns32
hpl_decode_hisv_evt (HPI_HISV_EVT_T *evt_struct, uns8 *evt_data, uns32 data_len, uns32 version)
{
   uns32 rc = NCSCC_RC_SUCCESS, i, mac_len = MAX_MAC_ENTRIES * MAC_DATA_LEN;
   uns8  *p8;

   if ((evt_struct == NULL) || (evt_data == NULL))
      return NCSCC_RC_FAILURE;

   if (data_len < sizeof(HPI_HISV_EVT_T))
   {
      printf("mismatch in expected (%d) and received (%d) data length\n",sizeof(HPI_HISV_EVT_T),data_len);
      return NCSCC_RC_FAILURE;
   }

   p8 = evt_data;

   /* decode the event */
   evt_struct->hpi_event.Source = ncs_decode_32bit(&p8);
   evt_struct->hpi_event.EventType = ncs_decode_32bit(&p8);
   evt_struct->hpi_event.Timestamp = ncs_decode_64bit(&p8);
   evt_struct->hpi_event.Severity = ncs_decode_32bit(&p8);

   memcpy((uns8 *)&evt_struct->hpi_event.EventDataUnion, p8, (int32)sizeof(SaHpiEventUnionT));
   p8 += (int32)sizeof(SaHpiEventUnionT);

   /* decode entity path */
   for (i=0; i<SAHPI_MAX_ENTITY_PATH; i++)
   {
      evt_struct->entity_path.Entry[i].EntityType = ncs_decode_32bit(&p8);
#ifdef HAVE_HPI_A01
      evt_struct->entity_path.Entry[i].EntityInstance = ncs_decode_32bit(&p8);
#else
      evt_struct->entity_path.Entry[i].EntityLocation = ncs_decode_32bit(&p8);
#endif
   }

   /* decode inventory data */
   /* decode product name */
   evt_struct->inv_data.product_name.DataType = ncs_decode_32bit(&p8);
   evt_struct->inv_data.product_name.Language = ncs_decode_32bit(&p8);
   evt_struct->inv_data.product_name.DataLength = ncs_decode_8bit(&p8);

   memcpy((uns8 *)&evt_struct->inv_data.product_name.Data, p8, SAHPI_MAX_TEXT_BUFFER_LENGTH);
   p8 += SAHPI_MAX_TEXT_BUFFER_LENGTH;

   /* decode product version */
   evt_struct->inv_data.product_version.DataType = ncs_decode_32bit(&p8);
   evt_struct->inv_data.product_version.Language = ncs_decode_32bit(&p8);
   evt_struct->inv_data.product_version.DataLength = ncs_decode_8bit(&p8);

   memcpy((uns8 *)&evt_struct->inv_data.product_version.Data, p8, SAHPI_MAX_TEXT_BUFFER_LENGTH);
   p8 += SAHPI_MAX_TEXT_BUFFER_LENGTH;

   /* decode OEM inventory data */
   evt_struct->inv_data.oem_inv_data.type = ncs_decode_8bit(&p8);
   evt_struct->inv_data.oem_inv_data.mId = ncs_decode_32bit(&p8);
   evt_struct->inv_data.oem_inv_data.mot_oem_rec_id = ncs_decode_32bit(&p8);
   evt_struct->inv_data.oem_inv_data.rec_format_ver = ncs_decode_32bit(&p8);
   evt_struct->inv_data.oem_inv_data.num_mac_entries = ncs_decode_32bit(&p8);

   memcpy((uns8 *)&evt_struct->inv_data.oem_inv_data.interface_mac_addr, p8, mac_len);
   p8 += mac_len;

   /* decode version */
   evt_struct->version = ncs_decode_32bit(&p8);

   return rc;
}

/****************************************************************************
 * Name          : hpl_entity_path_lookup
 *
 * Description   : This function can be used to get an entity-path based on
 *                 chassis number (chassisID)  and a blade number (bladeID).
 *                 Check return value for success or failure of the call.
 *                 Check return string length for zero and non-zero to see
 *                 if lookup found a match.
 *                 flag is set to 0 - return full string (SAHPI_ENT_SYSTEM_CHASSIS) format entity path.
 *                 flag is set to 1 - return numeric string format entity path.
 *                 flag is set to 2 - return array-based format entity path.
 *                 flag is set to 3 - return short string (SYSTEM_CHASSIS) format entity path.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 hpl_entity_path_lookup(uns32 flag, uns32 chassis_id, uns32 blade_id, uns8 *entity_path, size_t entity_path_size)
{
   HPL_CB      *hpl_cb;
   MDS_DEST    ham_dest;
   HISV_MSG    hisv_msg, *msg;
   uns32       rc;
   uns8        *hpl_data;
   HPL_PAYLOAD *hpl_pload;
   uns16       ret_len=0;

   /* validate the flag parameter */
   if (flag > HPL_EPATH_FLAG_SHORTSTR)
   {
      /* invalid entity-path format flag */
      m_LOG_HISV_DEBUG("bad entity-path format flag passed to hpl_entity_path_lookup()\n");
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   /** retrieve HPL CB
    **/
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL, gl_hpl_hdl)))
   {
      printf("Could not retrieve entity path lookup. Failed to get HPL control block.\n");
      return NCSCC_RC_FAILURE;
   }

  /** get the MDS VDEST of HAM which is managing the chassis
    ** identified by chassis_id. Function uses lock internally.
    **/
   if (NCSCC_RC_FAILURE == (rc = get_ham_dest(hpl_cb, &ham_dest, chassis_id)))
   {
      printf("No HAM managing chassis %d\n", chassis_id);
      ncshm_give_hdl(gl_hpl_hdl);
      return rc;
   }

   hpl_data = m_MMGR_ALLOC_HPL_DATA(sizeof(HPL_PAYLOAD));
   hpl_pload = (HPL_PAYLOAD *)hpl_data;
   hpl_pload->d_tlv.d_type = LOOKUP;
   hpl_pload->d_tlv.d_len = (sizeof(HPL_PAYLOAD));
   hpl_pload->d_chassisID = chassis_id;
   hpl_pload->d_bladeID = blade_id;

   m_HPL_HISV_ENTITY_MSG_FILL(hisv_msg, HISV_ENTITYPATH_LOOKUP, flag, sizeof(HPL_PAYLOAD), hpl_pload);

   /* send the synchronous MDS request message to HAM instance */
   msg = hpl_mds_msg_sync_send (hpl_cb, &hisv_msg, &ham_dest,
                                MDS_SEND_PRIORITY_HIGH, gl_hpl_mds_timeout);

   /* give control block handle */
   ncshm_give_hdl(gl_hpl_hdl);
   m_MMGR_FREE_HPL_DATA(hpl_data);

   if (msg == NULL) return NCSCC_RC_FAILURE;
   rc = msg->info.cbk_info.hpl_ret.h_gen.ret_val;
   ret_len = msg->info.cbk_info.hpl_ret.h_gen.data_len;

   if (ret_len > 0) {
      /* Check return length.  For array, it must be equal to or less than user buffer size. */
      /* For string, it must be less than user buffer size.                                  */
      if (((flag == HPL_EPATH_FLAG_ARRAY) && (ret_len <= entity_path_size)) ||
          (ret_len < entity_path_size))
      { 
         memcpy((uns8 *)entity_path, (uns8 *)(msg->info.cbk_info.hpl_ret.h_gen.data), ret_len);
      } 
      else
      { 
         free_hisv_ret_msg(msg); 
         return NCSCC_RC_FAILURE; 
      } 
   }

   /* If this is a string version of the entity-path, null-terminate the  */
   /* entity-path where we're copied the data to.                         */
   if (flag != HPL_EPATH_FLAG_ARRAY) {
      entity_path[ret_len] = 0;
   }

   free_hisv_ret_msg(msg);
   return NCSCC_RC_SUCCESS;
}

