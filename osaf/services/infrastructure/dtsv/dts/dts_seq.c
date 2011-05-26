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

    This file has the functions of the Flex Log Server (DTS) for Message 
    Sequencing.

  ..............................................................................

  FUNCTIONS INCLUDED in this module:
  dts_sort_msgs           - Sort messages.
  dts_sift_down           - Sort support function.
  dts_queue_seq_msg       - Queue messages for sequencing.
  dts_dump_seq_msg        - Dump sorted messages.
  dts_enable_sequencing   - Enable sequencing.
  dts_disable_sequencing  - Disable Sequencing.
  dts_tmrcb               - Timer call back function.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "dts.h"
#include "ncssysf_mem.h"

static void dts_tmrcb(void *arg);
/****************************************************************************
  PROCEDURE NAME  : dts_sort_msgs
  DESCRIPTION     : This finction is used for sorting the messages in the 
                    sequencing Array. We are using heap sort for sorting.

  ARGUMENTS       : msg_seq - Array of messages to be sequenced.
      
  RETURNS         : None.
  NOTES:
*****************************************************************************/
void dts_sort_msgs(SEQ_ARRAY *msg_seq, uint32_t array_size)
{
	int i;

	SEQ_ARRAY temp;

	for (i = ((array_size / 2) - 1); i >= 0; i--) {
		dts_sift_down(msg_seq, i, array_size - 1);
	}

	for (i = array_size - 1; i >= 1; i--) {
		temp = msg_seq[0];
		msg_seq[0] = msg_seq[i];
		msg_seq[i] = temp;
		dts_sift_down(msg_seq, 0, i - 1);
	}
}

/****************************************************************************
  PROCEDURE NAME  : dts_sift_down
  DESCRIPTION     : Heap Sort supporting function.

  ARGUMENTS       : msg_seq - Array of messages to be sequenced.
                    root
                    bottom
      
  RETURNS         : None.
  NOTES:
*****************************************************************************/
void dts_sift_down(SEQ_ARRAY *msg_seq, uint32_t root, uint32_t bottom)
{
	uint32_t done, maxchild;
	SEQ_ARRAY temp;

	done = 0;
	while ((root * 2 <= bottom) && (!done)) {
		if (root * 2 == bottom)
			maxchild = root * 2;
		else if ((msg_seq[root * 2].ms_time.seconds > msg_seq[root * 2 + 1].ms_time.seconds) ||
			 ((msg_seq[root * 2].ms_time.seconds == msg_seq[root * 2 + 1].ms_time.seconds) &&
			  (msg_seq[root * 2].ms_time.millisecs > msg_seq[root * 2 + 1].ms_time.millisecs)))
			maxchild = root * 2;
		else
			maxchild = root * 2 + 1;

		if ((msg_seq[root].ms_time.seconds < msg_seq[maxchild].ms_time.seconds) ||
		    ((msg_seq[root].ms_time.seconds == msg_seq[maxchild].ms_time.seconds) &&
		     (msg_seq[root].ms_time.millisecs < msg_seq[maxchild].ms_time.millisecs))) {
			temp = msg_seq[root];
			msg_seq[root] = msg_seq[maxchild];
			msg_seq[maxchild] = temp;
			root = maxchild;
		} else
			done = 1;
	}
}

/****************************************************************************
  PROCEDURE NAME  : dts_queue_seq_msg
  DESCRIPTION     : This finction is called from the dts_log_data() function if 
                    sequencing is enabled on the node. This function queues 
                    messages received from the DTA to the sequencing Array.

  ARGUMENTS       : inst - Ponter to the DTS CB.
                    msg  - Message to be logged.
                    plcy-hdl - policy handle which has alreay got decoded.
      
  RETURNS         : NCSCC_RC_SUCCESS,
                    NCSCC_RC_FAILURE.
  NOTES:
*****************************************************************************/
#define DTSV_MSG_TIME_STAMP   (2 * sizeof(uint32_t))
#define DTS_MAX_SIZE_DATA  512
uint32_t dts_queue_seq_msg(DTS_CB *inst, DTSV_MSG *msg)
{
	/*
	 * Copy the time stamp portion of the mssage received 
	 * dump it into the sequencing buffer along with the message.
	 */
	if (msg == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_queue_seq_msg: Message to be queued is NULL");

	if (inst->s_buffer.arr_ptr == NULL) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_queue_seq_msg: Sequence buffer pointer is NULL. Unable to queue message");
	}

	inst->s_buffer.arr_ptr[inst->s_buffer.num_msgs].ms_time.seconds = msg->data.data.msg.log_msg.hdr.time.seconds;
	inst->s_buffer.arr_ptr[inst->s_buffer.num_msgs].ms_time.millisecs
	    = msg->data.data.msg.log_msg.hdr.time.millisecs;

	msg->seq_msg = true;

	/* Now copy the message pointer into sequencing buffer */
	inst->s_buffer.arr_ptr[inst->s_buffer.num_msgs].msg = msg;

	/*
	 * If the number of messages dumped into the sequencing buffer exceeds the 
	 * max size, then it is time to dump the messages.
	 */
	if (++inst->s_buffer.num_msgs >= SEQ_ARR_MAX_SIZE) {
		if (dts_dump_seq_msg(inst, false) != NCSCC_RC_SUCCESS)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dts_queue_seq_msg: Failed to dump messages in the sequencing buffer");
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  PROCEDURE NAME  : dts_dump_seq_msg
  DESCRIPTION     : This function dumps the messages after sequncing.

  ARGUMENTS       : inst - Ponter to the DTS CB.
                    all  - This flag indicates whether to dump all the messages
                           in the sequencing buffer.
      
  RETURNS         : NCSCC_RC_SUCCESS,
                    NCSCC_RC_FAILURE.
  NOTES:
*****************************************************************************/
uint32_t dts_dump_seq_msg(DTS_CB *inst, bool all)
{
	uint32_t i, j = 0, num_msg_dump = 0;
	SEQ_BUFFER *buffer = &inst->s_buffer;

	m_DTS_LK(&inst->lock);
	m_LOG_DTS_LOCK(DTS_LK_LOCKED, &inst->lock);

	if (inst->s_buffer.arr_ptr == NULL) {
		m_DTS_UNLK(&inst->lock);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_dump_seq_msg: Sequenc buffer pointer is NULL. Failed to dump messages");
	}

	/*
	 * So we want to dump the messages in the sequencing buffer,
	 * first stop the timer and then start message sequencing.
	 */
	m_NCS_TMR_STOP(inst->tmr);
	dts_sort_msgs(inst->s_buffer.arr_ptr, inst->s_buffer.num_msgs);

	/* Check whether we want to dump all the messages or only half of the buffer */
	if (all) {
		num_msg_dump = buffer->num_msgs;
	} else {
		num_msg_dump = buffer->num_msgs / 2;
	}

	/*
	 * Now dump messages which are in sequence. 
	 * Here we need to free the message which we used to store in the sequencing
	 * buffer after logging.
	 */
	for (i = 0; i < num_msg_dump; i++) {
		if (dts_log_data(buffer->arr_ptr[i].msg) != NCSCC_RC_SUCCESS) {
			if ((buffer->arr_ptr[i].msg)->data.data.msg.log_msg.uba.ub != NULL)
				m_MMGR_FREE_BUFR_LIST((buffer->arr_ptr[i].msg)->data.data.msg.log_msg.uba.ub);
		}
		m_MMGR_FREE_DTSV_MSG(buffer->arr_ptr[i].msg);
		buffer->arr_ptr[i].msg = NULL;
	}

	if (all) {
		m_NCS_TMR_START(inst->tmr, SEQ_TIME, dts_tmrcb, 0);
		m_DTS_UNLK(&inst->lock);
		m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &inst->lock);
		return NCSCC_RC_SUCCESS;
	}

	/*
	 * So we have dumped top half of sequenced message successfully; so now
	 * adjust the sequenced array. Shift lowe half of the messages to the 
	 * top half.
	 */
	for (i = 0; ((buffer->num_msgs / 2) + i) < buffer->num_msgs; i++) {
		buffer->arr_ptr[i] = buffer->arr_ptr[i + (buffer->num_msgs / 2)];
		j++;
	}
	buffer->num_msgs = j;

	m_NCS_TMR_START(inst->tmr, SEQ_TIME, dts_tmrcb, 0);
	m_DTS_UNLK(&inst->lock);
	m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &inst->lock);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  PROCEDURE NAME  : dts_enable_sequencing
  DESCRIPTION     : Function which allocates the sequencing bufeer and initializes
                    all the variables. Function is called after enabling the 
                    sequencing.

  ARGUMENTS       : inst - Ponter to the DTS CB.
                    
      
  RETURNS         : NCSCC_RC_SUCCESS,
                    NCSCC_RC_FAILURE.
  NOTES:
*****************************************************************************/
uint32_t dts_enable_sequencing(DTS_CB *inst)
{
	/* 
	 * User has enabled the sequencing. So allocate the sequencing buffer.
	 */
	inst->s_buffer.arr_ptr = m_MMGR_ALLOC_SEQ_BUFF(SEQ_ARR_MAX_SIZE);

	if (inst->s_buffer.arr_ptr == NULL) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_enable_sequencing: Sequenc buffer pointer is NULL. Failed to dump messages");
	}

	/* 
	 * Create and start the sequencing pointer.
	 */
	m_NCS_TMR_CREATE(inst->tmr, SEQ_TIME, dts_tmrcb, 0);

	m_NCS_TMR_START(inst->tmr, SEQ_TIME, dts_tmrcb, 0);

	inst->s_buffer.num_msgs = 0;

	m_LOG_DTS_DBGSTR(DTS_IGNORE, "Message Sequencing is enabled.", 0, 0);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  PROCEDURE NAME  : dts_disable_sequencing
  DESCRIPTION     : Function which de-allocates the sequencing bufeer and 
                    re-initializes all the variables. Function is called after
                    disabling the sequencing.

  ARGUMENTS       : inst - Ponter to the DTS CB.
                    
      
  RETURNS         : NCSCC_RC_SUCCESS,
                    NCSCC_RC_FAILURE.
  NOTES:
*****************************************************************************/
uint32_t dts_disable_sequencing(DTS_CB *inst)
{
	/*
	 * User has disable the sequencing, so dump remaining 
	 * Messages of the sequencing buffer and destroy timer,
	 * free sequencing buffer.
	 */
	m_NCS_TMR_STOP(inst->tmr);

	dts_dump_seq_msg(inst, true);

	m_NCS_TMR_DESTROY(inst->tmr);

	if (NULL != inst->s_buffer.arr_ptr) {
		m_MMGR_FREE_SEQ_BUFF(inst->s_buffer.arr_ptr);
		inst->s_buffer.arr_ptr = NULL;
	}

	inst->s_buffer.num_msgs = 0;

	m_LOG_DTS_DBGSTR(DTS_IGNORE, "Message Sequencing is disabled.", 0, 0);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 *
 * Function Name: dts_tmrcb
 *
 * Purpose: Timer call back function for dts.
 *    
 * Arguments: Transaction id of the item
 *
 * Return Value: None
 *
 ****************************************************************************/

static void dts_tmrcb(void *arg)
{
	DTSV_MSG *mm;

	/*
	 * We got sequencing timer event, so send the message to the 
	 * DTS thhread that it is time to do sequencing and dump the messages.
	 */
	mm = m_MMGR_ALLOC_DTSV_MSG;
	if (mm == NULL) {
		return;
	}
	memset(mm, '\0', sizeof(DTSV_MSG));

	mm->msg_type = DTSV_DUMP_SEQ_MSGS;

	if (m_DTS_SND_MSG(&gl_dts_mbx, mm, NCS_IPC_PRIORITY_NORMAL) != NCSCC_RC_SUCCESS) {
		if (mm != NULL)
			m_MMGR_FREE_DTSV_MSG(mm);
		return;
	}

	return;
}
