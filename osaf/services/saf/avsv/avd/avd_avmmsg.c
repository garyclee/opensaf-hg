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

  This module deals with the AvD interaction with the AvM. The messages 
  received from the AvM and the messages to be sent to the AvM are 
  listed here. 

..............................................................................

  FUNCTIONS INCLUDED in this module:
  
  Send Routines:
  avd_avm_send_shutdown_resp
  avd_avm_send_failover_resp
  avd_avm_send_fault_domain_req
  avd_avm_send_reset_req

  Receive/Process routines:
  avd_avm_rcv_msg

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#include <logtrace.h>
#include "avd.h"
#include "avd_avm.h"

#define PRINT_ROLE(role, status)\
{\
    (role == 1)?m_NCS_DBG_PRINTF("\n AVD: ROLE = %s", "ACTIVE"):\
    (role == 2)?m_NCS_DBG_PRINTF("\n AVD: ROLE = %s", "STANDBY"):\
    (role == 3)?m_NCS_DBG_PRINTF("\n AVD: ROLE = %s", "QUIESCED"):\
                m_NCS_DBG_PRINTF("\n AVD: ROLE = %d",  role);\
    (status == 1)?m_NCS_DBG_PRINTF(" -- RESP = %s\n", "SUCCESS"):\
    (status == 2)?m_NCS_DBG_PRINTF(" -- RESP = %s\n", "FAILURE"):\
                  m_NCS_DBG_PRINTF(" -- RESP = %d\n", status);\
}

/****************************************************************************
 *  Name          : avd_avm_rcv_msg
 * 
 *  Description   : This routine is invoked by AvD when a message arrives
 *  from AVM. It converts the message to the corresponding event and posts
 *  the message to the mailbox for processing by the main loop.
 * 
 *  Arguments     : cb_hdl     -  AvD cb Handle.
 *                  rcv_msg    -  ptr to the received message
 * 
 *  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 * 
 *  Notes         : None.
 ***************************************************************************/

uns32 avd_avm_rcv_msg(uns32 cb_hdl, AVM_AVD_MSG_T *rcv_msg)
{
	AVD_EVT *evt = AVD_EVT_NULL;
	AVD_CL_CB *cb = NULL;

	/* check that the message ptr is not NULL */
	if (rcv_msg == NULL) {
		m_AVD_LOG_INVALID_VAL_ERROR(0);
		return NCSCC_RC_FAILURE;
	}

	/* create the message event */
	evt = malloc(sizeof(AVD_EVT));
	if (evt == AVD_EVT_NULL) {
		/* log error */
		m_AVD_LOG_MEM_FAIL_LOC(AVD_EVT_ALLOC_FAILED);
		/* free the message and return */
		avm_avd_free_msg(&rcv_msg);
		return NCSCC_RC_FAILURE;
	}

	/* get the CB from the handle manager */
	if ((cb = (AVD_CL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVD, cb_hdl)) == NULL) {
		/* log error */
		free(evt);
		/* free the message and return */
		avm_avd_free_msg(&rcv_msg);
		return NCSCC_RC_FAILURE;
	}

	evt->cb_hdl = cb_hdl;
	evt->rcv_evt = (rcv_msg->msg_type + AVD_EVT_INIT_MAX);
	evt->info.avm_msg = rcv_msg;

	if (m_NCS_IPC_SEND(&cb->avd_mbx, evt, NCS_IPC_PRIORITY_HIGH)
	    != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_SEND);
		/* return AvD CB handle */
		ncshm_give_hdl(cb_hdl);
		/* log error */
		/* free the message */
		avm_avd_free_msg(&rcv_msg);
		evt->info.avm_msg = NULL;
		/* free the event and return */
		free(evt);

		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_SEND);

	/* return AvD CB handle */
	ncshm_give_hdl(cb_hdl);

	m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_RCV_CBK);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avd_avm_send_msg
 
  Description   : This is a routine that is invoked to send messages
                  to AVM.
 
  Arguments     : cb_hdl    - AvD control block Handle.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : 
******************************************************************************/
static uns32 avd_avm_send_msg(AVD_CL_CB *cb, AVD_AVM_MSG_T *snd_msg)
{
	NCSMDS_INFO snd_mds;
	uns32 rc;

	memset(&snd_mds, '\0', sizeof(NCSMDS_INFO));

	snd_mds.i_mds_hdl = cb->adest_hdl;
	snd_mds.i_svc_id = NCSMDS_SVC_ID_AVD;
	snd_mds.i_op = MDS_SEND;
	snd_mds.info.svc_send.i_msg = (NCSCONTEXT)snd_msg;
	snd_mds.info.svc_send.i_to_svc = NCSMDS_SVC_ID_AVM;
	snd_mds.info.svc_send.i_priority = MDS_SEND_PRIORITY_HIGH;
	snd_mds.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
	snd_mds.info.svc_send.info.snd.i_to_dest = cb->avm_mds_dest;

	if ((rc = ncsmds_api(&snd_mds)) != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_SEND);
		return rc;
	}

	m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_SEND);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
*  Name          : avd_avm_send_shutdown_resp
* 
*  Description   : This is a routine that is invoked to send a shutdown response
*                  message to AvM informing that the shutdown is done/success or*                  indicating a failure.
* 
*  Arguments     : cb    - AvD control block Handle.
*                  node  - SaNameT of the node for which the shutdown op was
*                          performed.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
uns32 avd_avm_send_shutdown_resp(AVD_CL_CB *cb, SaNameT *node, uns32 status)
{
	AVD_AVM_MSG_T *snd_msg = m_MMGR_ALLOC_AVD_AVM_MSG;
	uns32 rc = NCSCC_RC_SUCCESS;

	assert(0);

	/* Fill in the message */
	memset(snd_msg, '\0', sizeof(AVD_AVM_MSG_T));
	snd_msg->msg_type = AVD_AVM_NODE_SHUTDOWN_RESP_MSG;

	snd_msg->avd_avm_msg.shutdown_resp.node_name.length = node->length;
	memcpy(&snd_msg->avd_avm_msg.shutdown_resp.node_name.value, node->value,
	       snd_msg->avd_avm_msg.shutdown_resp.node_name.length);

	snd_msg->avd_avm_msg.shutdown_resp.recovery_status = status;

	rc = avd_avm_send_msg(cb, snd_msg);

	if (rc != NCSCC_RC_SUCCESS)
		m_MMGR_FREE_AVD_AVM_MSG(snd_msg);

	return rc;
}

/****************************************************************************
*  Name          : avd_avm_send_failover_resp
* 
*  Description   : This is a routine that is invoked to send a failover response
*                  message to AvM informing that the failover is done/success or*                  indicating a failure.
* 
*  Arguments     : cb    - AvD control block Handle.
*                  node  - SaNameT of the node for which the failover op was
*                          performed.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
uns32 avd_avm_send_failover_resp(AVD_CL_CB *cb, SaNameT *node, uns32 status)
{
	AVD_AVM_MSG_T *snd_msg = m_MMGR_ALLOC_AVD_AVM_MSG;
	uns32 rc = NCSCC_RC_SUCCESS;

	assert(0);

	/* Fill in the message */
	memset(snd_msg, '\0', sizeof(AVD_AVM_MSG_T));
	snd_msg->msg_type = AVD_AVM_NODE_FAILOVER_RESP_MSG;

	snd_msg->avd_avm_msg.failover_resp.node_name.length = node->length;
	memcpy(&snd_msg->avd_avm_msg.failover_resp.node_name.value, node->value,
	       snd_msg->avd_avm_msg.failover_resp.node_name.length);

	snd_msg->avd_avm_msg.failover_resp.recovery_status = status;
	rc = avd_avm_send_msg(cb, snd_msg);

	if (rc != NCSCC_RC_SUCCESS)
		m_MMGR_FREE_AVD_AVM_MSG(snd_msg);

	return rc;
}

/****************************************************************************
*  Name          : avd_avm_send_fault_domain_req
* 
*  Description   : This is a routine that is invoked to send a request
*                  message to AvM to get the fault domain information.
*                  Depending on the fault domain the AvD need to act on the
*                  error and apply it to more than one AMF Object.
* 
*  Arguments     : cb    - AvD control block Handle.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
uns32 avd_avm_send_fault_domain_req(AVD_CL_CB *cb, SaNameT *node)
{
	AVD_AVM_MSG_T *snd_msg = m_MMGR_ALLOC_AVD_AVM_MSG;
	uns32 rc = NCSCC_RC_SUCCESS;

	assert(0);

	/* Fill in the message */
	memset(snd_msg, '\0', sizeof(AVD_AVM_MSG_T));
	snd_msg->msg_type = AVD_AVM_FAULT_DOMAIN_REQ_MSG;

	snd_msg->avd_avm_msg.fault_domain_req.node_name.length = node->length;
	memcpy(&snd_msg->avd_avm_msg.fault_domain_req.node_name.value, node->value,
	       snd_msg->avd_avm_msg.fault_domain_req.node_name.length);

	rc = avd_avm_send_msg(cb, snd_msg);

	if (rc != NCSCC_RC_SUCCESS)
		m_MMGR_FREE_AVD_AVM_MSG(snd_msg);

	return rc;
}

/****************************************************************************
*  Name          : avd_avm_send_reset_req
* 
*  Description   : This is a routine that is invoked to send a request
*                  message to AvM to get the fault domain information.
*                  Depending on the fault domain the AvD need to act on the
*                  error and apply it to more than one AMF Object.
* 
*  Arguments     : cb    - AvD control block Handle.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
uns32 avd_avm_send_reset_req(AVD_CL_CB *cb, SaNameT *node)
{
#if 0
	AVD_AVM_MSG_T *snd_msg = m_MMGR_ALLOC_AVD_AVM_MSG;
	uns32 rc = NCSCC_RC_SUCCESS;

	m_AVD_LOG_FUNC_ENTRY("avd_avm_send_reset_req");

	/* Fill in the message */
	memset(snd_msg, '\0', sizeof(AVD_AVM_MSG_T));
	snd_msg->msg_type = AVD_AVM_NODE_RESET_REQ_MSG;

	snd_msg->avd_avm_msg.reset_req.node_name.length = node->length;
	memcpy(&snd_msg->avd_avm_msg.reset_req.node_name.value, node->value,
	       snd_msg->avd_avm_msg.reset_req.node_name.length);

	rc = avd_avm_send_msg(cb, snd_msg);

	if (rc != NCSCC_RC_SUCCESS)
		m_MMGR_FREE_AVD_AVM_MSG(snd_msg);

	return rc;
#else
	AVD_AVND *avnd = NULL;

	/* Temporary hack till the plm and clm comes into picture */	
	/* first get the avnd structure from the node name */
	avnd = avd_node_get(node);

	if (avnd == NULL) {
		return NCSCC_RC_FAILURE;
	}

	if (avnd->node_info.nodeId == cb->node_id_avd) {
		ncs_reboot("A reset has been trigerred for this node");
	} else if (avnd->node_state != AVD_AVND_STATE_ABSENT) {

		/* clean up the heartbeat timer for this node. */
		m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);
		avd_stop_tmr(cb, &(avnd->heartbeat_rcv_avnd));
		m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

		avd_avm_mark_nd_absent(cb, avnd);
	}

	return NCSCC_RC_SUCCESS;

#endif

}

/****************************************************************************
  Name          : avd_avd_mds_cpy
 
  Description   : This is a callback routine that is invoked to copy 
                  AvD-AvM messages. 
 
  Arguments     : cpy_info - ptr to the MDS copy info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_avm_mds_cpy(MDS_CALLBACK_COPY_INFO *cpy_info)
{
	AVD_AVM_MSG_T *msg = cpy_info->i_msg;

	cpy_info->o_cpy = (NCSCONTEXT)msg;
	cpy_info->i_msg = 0;	/* memory is not allocated and hence this */
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
*  Name          : avd_avm_role_rsp
* 
*  Description   : This is a routine that is invoked to send a response
*                  message to AvM for the role set.
* 
*  Arguments     : cb    - AvD control block Handle.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
uns32 avd_avm_role_rsp(AVD_CL_CB *cb, NCS_BOOL status, SaAmfHAStateT role)
{
	TRACE_ENTER2("%u", status);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avd_avm_node_reset_rsp
 *
 * Purpose:  AVSV function to create node reset responce.
 *
 * Input: cb - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avd_avm_node_reset_rsp(AVD_CL_CB *cb, uns32 node)
{
	AVM_AVD_MSG_T *msg;
	AVD_AVND *avnd;

	m_AVD_LOG_FUNC_ENTRY("avd_avm_node_reset_rsp");

	msg = m_MMGR_ALLOC_AVM_AVD_MSG;
	memset(msg, '\0', sizeof(AVM_AVD_MSG_T));

	msg->msg_type = AVM_AVD_NODE_RESET_RESP_MSG;
	avnd = avd_node_find_nodeid(node);

	if (NULL == avnd) {
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd_other);
		return NCSCC_RC_FAILURE;
	}

	memcpy(msg->avm_avd_msg.reset_resp.node_name.value,
	       avnd->node_info.nodeName.value, avnd->node_info.nodeName.length);

	msg->avm_avd_msg.reset_resp.node_name.length = avnd->node_info.nodeName.length;

	avd_avm_rcv_msg(cb->cb_handle, msg);

	return NCSCC_RC_SUCCESS;
}
