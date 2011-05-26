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

  DESCRIPTION:
   The DTSv Circular Buffer Operation table.

  ..............................................................................

  FUNCTIONS INCLUDED in this module:
  dts_dump_log_to_op_device          - CB - dump log to output device.
  dts_circular_buffer_alloc          - Allocate circular buffer.
  dts_circular_buffer_free           - Free circular buffer.
  dts_circular_buffer_clear          - Clear circular buffer.
  dts_cir_buff_set_default           - Set circular buffer parameters to default.
  dts_dump_to_cir_buffer             - Dump circular buffer contents.
  dts_buff_size_increased            - Increase the circular buffer size.
  dts_buff_size_decreased            - Decrease circular buffer size.
  dts_dump_buffer_to_buffer          - Dump from src to dst buffer.
******************************************************************************/

#include "dts.h"

/**************************************************************************
 Function: dts_dump_log_to_op_device

 Purpose:  Give me pointer to your circular buffer and the device name where 
           you wanna dump, I will dump it for you if everything goes fine.

 Input:    cir_buff : Circular buffer.
           device   : specify the output device where log to be dumped.
           file     : File where to dump.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
uns32 dts_dump_log_to_op_device(CIR_BUFFER *cir_buff, uint8_t device, char *file)
{
	uint8_t i, num;
	uns32 j;
	FILE *fh;
	char *str = dts_cb.cb_log_str;
	char *ptr;
	uint8_t inuse_buff = 0;
	NCS_BOOL found = FALSE;

	if (cir_buff->buff_allocated == FALSE)
		return NCSCC_RC_FAILURE;

	/* First Step : Find the INUSE buffer and then start reading from the next */
	for (i = 0; i < NUM_BUFFS; i++) {
		if (cir_buff->buff_part[i].status == INUSE) {
			found = TRUE;
			inuse_buff = i;
			break;
		}
	}

	/* No buffer found INUSE. So something looks wrong */
	if (found == FALSE)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_dump_log_to_op_device: You should not come to this place. SERIOUS issue. No buffer found to be in use");

	/* Take care of the buffer wrap around case. Start from the next buff. */
	if (++inuse_buff >= NUM_BUFFS)
		inuse_buff = 0;

	num = inuse_buff;
	for (i = 0; i < NUM_BUFFS; i++) {
		/* "num" should be between 0 to --NUM_BUFFS */
		if (num >= NUM_BUFFS)
			num = 0;

		if (cir_buff->buff_part[num].status == CLEAR) {
			num++;
			continue;
		}

		ptr = cir_buff->buff_part[num].cir_buff_ptr;

		if (device == LOG_FILE) {
			if ((fh = sysf_fopen(file, "a+")) != NULL) {
				fprintf(fh, "\n NEW PART \n");

				for (j = 0; j < cir_buff->buff_part[num].num_of_elements; j++) {
					strcpy(str, (char *)ptr);
					fprintf(fh, "%s", (const char *)str);
					ptr += (strlen(str) + 1);
				}

				fclose(fh);
			} else
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "URGENT: dts_dump_log_to_op_device: Unable to Open FILE. Something is going wrong.");
		} else if (device == OUTPUT_CONSOLE) {
			for (j = 0; j < cir_buff->buff_part[num].num_of_elements; j++) {
				strcpy(str, (char *)ptr);
				TRACE("%s", str);
				ptr += (strlen(str) + 1);
			}
		} else
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dts_dump_log_to_op_device: Device type received is not correct");

		if (cir_buff->buff_part[num].status == INUSE)
			return NCSCC_RC_SUCCESS;

		num++;
	}

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_circular_buffer_alloc

 Purpose:  This function is used for allocating the circular buffer depending
           on its size, if the circular buffer logging is enabled.

 Input:    cir_buff     : Pointer to the circular buffer array.
           buffer_size  : Total buffer size to be used.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_circular_buffer_alloc(CIR_BUFFER *cir_buff, uns32 buffer_size)
{
	uns32 size = ((buffer_size * 1024) / NUM_BUFFS);
	uns32 i;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	if (cir_buff->buff_allocated == TRUE)
		return NCSCC_RC_FAILURE;

	/* Allocate a single chunk of memory of size == buffer_size */
	cir_buff->buff_part[0].cir_buff_ptr = m_MMGR_ALLOC_CIR_BUFF(buffer_size * 1024);

	if (cir_buff->buff_part[0].cir_buff_ptr == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_circular_buffer_alloc: Failed to allocate circular buffer");
	memset(cir_buff->buff_part[0].cir_buff_ptr, '\0', (buffer_size * 1024));

	/* 
	 * Now assign all the buffer partitions to default value.
	 * Also, assign addressed to each partition poniter.
	 */
	for (i = 0; i < NUM_BUFFS; i++) {
		cir_buff->buff_part[i].cir_buff_ptr = (cir_buff->buff_part[0].cir_buff_ptr + (size * i));

		cir_buff->buff_part[i].num_of_elements = 0;
		cir_buff->buff_part[i].status = CLEAR;
	}

	cir_buff->cur_buff_num = 0;
	cir_buff->cur_buff_offset = 0;
	cir_buff->inuse = FALSE;
	cir_buff->part_size = size;
	cir_buff->cur_location = cir_buff->buff_part[0].cir_buff_ptr;
	cir_buff->buff_part[0].status = INUSE;
	cir_buff->buff_allocated = TRUE;

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_circular_buffer_free

 Purpose:  This function is used for freeing the circular buffer depending
           on its size.

 Input:    cir_buff     : Pointer to the circular buffer array.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_circular_buffer_free(CIR_BUFFER *cir_buff)
{
	uns32 i;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	if (cir_buff->buff_allocated == FALSE)
		return NCSCC_RC_FAILURE;

	if (0 != cir_buff->buff_part[0].cir_buff_ptr)
		m_MMGR_FREE_CIR_BUFF(cir_buff->buff_part[0].cir_buff_ptr);

	for (i = 0; i < NUM_BUFFS; i++) {
		cir_buff->buff_part[i].cir_buff_ptr = NULL;
		cir_buff->buff_part[i].num_of_elements = 0;
		cir_buff->buff_part[i].status = CLEAR;

	}

	cir_buff->cur_buff_num = 0;
	cir_buff->cur_buff_offset = 0;
	cir_buff->part_size = 0;
	cir_buff->inuse = FALSE;
	cir_buff->cur_location = NULL;
	cir_buff->buff_allocated = FALSE;

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_circular_buffer_clear

 Purpose:  This function is used for clearing the circular buffer so that
           the next dumped message will start logging a fresh.

 Input:    cir_buff     : Pointer to the circular buffer array.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_circular_buffer_clear(CIR_BUFFER *cir_buff)
{
	uns32 i;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	if (cir_buff->buff_allocated == FALSE)
		return NCSCC_RC_FAILURE;

	for (i = 0; i < NUM_BUFFS; i++) {
		cir_buff->buff_part[i].num_of_elements = 0;
		cir_buff->buff_part[i].status = CLEAR;
	}

	cir_buff->cur_buff_num = 0;
	cir_buff->cur_buff_offset = 0;
	cir_buff->inuse = FALSE;
	cir_buff->cur_location = cir_buff->buff_part[0].cir_buff_ptr;
	cir_buff->buff_part[0].status = INUSE;
	cir_buff->buff_allocated = TRUE;

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_cir_buff_set_default

 Purpose:  This function is used for setting to default all the circular buffer
           parameters.

 Input:    cir_buff     : Pointer to the circular buffer array.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_cir_buff_set_default(CIR_BUFFER *cir_buff)
{
	uns32 i;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	for (i = 0; i < NUM_BUFFS; i++) {
		cir_buff->buff_part[i].cir_buff_ptr = NULL;
		cir_buff->buff_part[i].num_of_elements = 0;
		cir_buff->buff_part[i].status = CLEAR;
	}

	cir_buff->cur_buff_num = 0;
	cir_buff->cur_buff_offset = 0;
	cir_buff->inuse = FALSE;
	cir_buff->cur_location = NULL;
	cir_buff->buff_allocated = FALSE;

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_dump_to_cir_buffer

 Purpose:  This function is used for allocating the circular buffer depending
           on its size, if the circular buffer logging is enabled.

 Input:    cir_buff : Pointer to the circular buffer array.
           str      : String to be copied into the buffer.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_dump_to_cir_buffer(CIR_BUFFER *cir_buff, char *str)
{
	uns32 str_len = strlen(str) + 1;

	if (TRUE != cir_buff->buff_allocated)
		return NCSCC_RC_FAILURE;

	cir_buff->inuse = TRUE;

	/*
	 * Check whether we are crossing the partition size limit. If yes then 
	 * start dumping on the new partiotion.
	 */
	if ((cir_buff->cur_buff_offset + str_len) > cir_buff->part_size) {
		cir_buff->buff_part[cir_buff->cur_buff_num].status = FULL;

		if (++cir_buff->cur_buff_num >= NUM_BUFFS) {
			cir_buff->cur_buff_num = 0;
		}

		/* We want to dump on new partition so do the basic initialization */
		cir_buff->cur_location = cir_buff->buff_part[cir_buff->cur_buff_num].cir_buff_ptr;
		cir_buff->cur_buff_offset = 0;

		cir_buff->buff_part[cir_buff->cur_buff_num].status = INUSE;
		cir_buff->buff_part[cir_buff->cur_buff_num].num_of_elements = 0;
	}

	/*
	 * So either we are within the range of buffer partion or we are 
	 * dumping on the new partition. So dump the message.
	 * Here if we are still exceeding the partition size, then there is some
	 * problem and we are not able to log this message into circular buffer.
	 * Need to look at this problem so return failure.
	 */
	if ((cir_buff->cur_buff_offset + str_len) > cir_buff->part_size)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_dump_to_cir_buffer: Hmmm!! Looks like your message does not fit into the buffer part. Increase buffer size.");

	/* So everything is set to dump your message in buffer. Good Luck!! */
	strcpy((char *)cir_buff->cur_location, str);
	cir_buff->cur_location += str_len;

	cir_buff->buff_part[cir_buff->cur_buff_num].num_of_elements++;
	cir_buff->cur_buff_offset += str_len;

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_buff_size_increased

 Purpose:  This function is called when circular buffer size is increased.

 Input:    cir_buff : Pointer to the circular buffer array.
           new_size : New size of the buffer.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_buff_size_increased(CIR_BUFFER *cir_buff, uns32 new_size)
{
	CIR_BUFFER tmp_buff = *cir_buff;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	/* Changes to allow SET on buffer size before having to 
	 *            set log device to buffer first.
	 */
	if (dts_cir_buff_set_default(cir_buff) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	if (dts_circular_buffer_alloc(cir_buff, new_size) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*
	 * Check whether the buffer is in use. If yes then only copy from 
	 * old to new buffer. Otherwise there is no point is copying since we won't 
	 * have any messages logged into the buffer.
	 */
	if (tmp_buff.inuse == TRUE) {
		if (dts_dump_buffer_to_buffer(&tmp_buff, cir_buff, NUM_BUFFS) != NCSCC_RC_SUCCESS)
			return NCSCC_RC_FAILURE;
	}

	cir_buff->inuse = tmp_buff.inuse;

	/* Free memory allocated for the old buffer. */
	dts_circular_buffer_free(&tmp_buff);

	return NCSCC_RC_SUCCESS;

}

/**************************************************************************\
 Function: dts_buff_size_decreased

 Purpose:  This function is called when circular buffer size is increased.

 Input:    cir_buff : Pointer to the circular buffer array.
           new_size : New size of the buffer.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_buff_size_decreased(CIR_BUFFER *cir_buff, uns32 new_size)
{
	CIR_BUFFER tmp_buff = *cir_buff;
	uns32 i = 0;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	/* Changes to allow SET on buffer size before having to 
	 *            set log device to buffer first.
	 */
	if (dts_cir_buff_set_default(cir_buff) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	if (dts_circular_buffer_alloc(cir_buff, new_size) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*
	 * Check whether the buffer is in use. If yes then only copy from 
	 * old to new buffer. Otherwise there is no point is copying since we won't 
	 * have any messages logged into the buffer.
	 */
	if (tmp_buff.inuse == TRUE) {
		/* 
		 * Since buffer size is decreased we are now going to find how many
		 * partitions of old buffer we can copy into the new buffer.
		 * If user decreases the size by less that 1/NUM_BUFFS size then 
		 * we will not able to dump a single message.
		 */
		for (i = 0; i < NUM_BUFFS; i++) {
			if ((tmp_buff.part_size * (i + 1)) > new_size)
				break;
		}

		if (dts_dump_buffer_to_buffer(&tmp_buff, cir_buff, i) != NCSCC_RC_SUCCESS)
			return NCSCC_RC_FAILURE;
	}

	cir_buff->inuse = tmp_buff.inuse;

	/* Free memory allocated for the old buffer. */
	dts_circular_buffer_free(&tmp_buff);

	return NCSCC_RC_SUCCESS;

}

/**************************************************************************\
 Function: dts_dump_buffer_to_buffer

 Purpose:  Give me pointer to your src and dst circular buffer  
           you wanna dump, I will dump it for you if everything goes fine.

 Input:    src_cir_buff : Source Circular buffer.
           dst_cir_buff : Destination circular buffer.
           number       : Number of buffer partitions to be dumped.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
\**************************************************************************/
uns32 dts_dump_buffer_to_buffer(CIR_BUFFER *src_cir_buff, CIR_BUFFER *dst_cir_buff, uns32 number)
{
	uint8_t i = 0, num = 0;
	uns32 j = 0;
	char *ptr = NULL;
	uint8_t inuse_buff = 0;
	NCS_BOOL found = FALSE;

	if ((src_cir_buff->buff_allocated == FALSE) || (dst_cir_buff->buff_allocated == FALSE))
		return NCSCC_RC_FAILURE;

	if (number == 0)
		return NCSCC_RC_SUCCESS;

	/* First Step : Find the INUSE buffer and then start reading from the next */
	for (i = 0; i < NUM_BUFFS; i++) {
		if (src_cir_buff->buff_part[i].status == INUSE) {
			found = TRUE;
			inuse_buff = i;
			break;
		}
	}

	/* No buffer found INUSE. So something looks wrong? -- You should never hit this case. */
	if (found == FALSE)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "URGENT: dts_dump_to_cir_buffer: No buffer found INUSE. So something looks wrong?");

	/* Take care of the buffer wrap around case. Start from the next buff. */
	if (++inuse_buff >= NUM_BUFFS)
		inuse_buff = 0;

	num = inuse_buff;

	/* 
	 * Since we have to dump only "number" of buffers, we should
	 * skip "(NUM_BUFFS - number)" buffers and start copying 
	 * from the next 
	 */
	for (i = 0; i < (NUM_BUFFS - number); i++) {
		if (++num >= NUM_BUFFS)
			num = 0;
	}

	/* Loop for "NUM_BUFFS" times */
	for (i = 0; i < NUM_BUFFS; i++) {
		/* "num" should be between 0 to --NUM_BUFFS */
		if (num >= NUM_BUFFS)
			num = 0;

		/* If buffer status is clear then no need to dump */
		if (src_cir_buff->buff_part[num].status == CLEAR) {
			num++;
			continue;
		}

		ptr = src_cir_buff->buff_part[num].cir_buff_ptr;

		/* Copy all the elemetns from source to destination buffer. */
		for (j = 0; j < src_cir_buff->buff_part[num].num_of_elements; j++) {
			if (dts_dump_to_cir_buffer(dst_cir_buff, (char *)ptr) != NCSCC_RC_SUCCESS)
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "URGENT: dts_dump_to_cir_buffer: Failed to copy to new buffer");

			ptr += (strlen((char *)ptr) + 1);
		}

		if (src_cir_buff->buff_part[num].status == INUSE)
			return NCSCC_RC_SUCCESS;

		num++;
	}

	return NCSCC_RC_SUCCESS;
}
