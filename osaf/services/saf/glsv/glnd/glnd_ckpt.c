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

..............................................................................

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/
#include "glnd.h"
uns32 glnd_restart_resource_info_ckpt_write(GLND_CB *glnd_cb, GLND_RESOURCE_INFO *res_info);
uns32 glnd_restart_resource_info_ckpt_overwrite(GLND_CB *glnd_cb, GLND_RESOURCE_INFO *res_info);

uns32 glnd_restart_lock_event_info_ckpt_write(GLND_CB *glnd_cb, GLSV_RESTART_BACKUP_EVT_INFO restart_backup_evt);
uns32 glnd_restart_res_lock_list_ckpt_write(GLND_CB *glnd_cb, GLND_RES_LOCK_LIST_INFO *res_lock_list,
					    SaLckResourceIdT res_id, SaLckHandleT app_handle_id, uint8_t to_which_list);

uns32 glnd_restart_res_lock_list_ckpt_overwrite(GLND_CB *glnd_cb, GLND_RES_LOCK_LIST_INFO *res_lock_list,
						SaLckResourceIdT res_id, SaLckHandleT app_handle_id,
						uint8_t to_which_list);

uns32 glnd_restart_res_lock_ckpt_read(GLND_CB *glnd_cb, GLND_RESTART_RES_LOCK_LIST_INFO *restart_res_lock_info,
				      uns32 offset);

uns32 glnd_restart_resource_ckpt_read(GLND_CB *glnd_cb, GLND_RESTART_RES_INFO *restart_resource_info, uns32 offset);

uns32 glnd_restart_backup_event_read(GLND_CB *glnd_cb, GLSV_RESTART_BACKUP_EVT_INFO *restart_backup_evt, uns32 offset);

/*****************************************************************************
  PROCEDURE NAME : glnd_restart_resource_info_ckpt_write()

  DESCRIPTION    : Checkpoints resource_info.

  ARGUMENTS      : glnd_cb        - ptr to the GLND control block
                   resource_info  - resource info to be checkpointed

  RETURNS        :

  NOTES          : None
*****************************************************************************/
uns32 glnd_restart_resource_info_ckpt_write(GLND_CB *glnd_cb, GLND_RESOURCE_INFO *res_info)
{
	GLND_RESTART_RES_INFO restart_resource_info;
	NCS_OS_POSIX_SHM_REQ_INFO res_info_write;
	uns32 rc = NCSCC_RC_SUCCESS;
	uns32 shm_index = 0;

	/* Fill restart_resource_info */
	memset(&restart_resource_info, '\0', sizeof(GLND_RESTART_RES_INFO));
	restart_resource_info.resource_id = res_info->resource_id;
	restart_resource_info.resource_name = res_info->resource_name;
	restart_resource_info.lcl_ref_cnt = res_info->lcl_ref_cnt;
	restart_resource_info.master_mds_dest = res_info->master_mds_dest;
	restart_resource_info.status = res_info->status;
	restart_resource_info.master_status = res_info->master_status;
	restart_resource_info.pr_orphan_req_count = res_info->lck_master_info.pr_orphan_req_count;
	restart_resource_info.ex_orphan_req_count = res_info->lck_master_info.ex_orphan_req_count;
	restart_resource_info.pr_orphaned = res_info->lck_master_info.pr_orphaned;
	restart_resource_info.ex_orphaned = res_info->lck_master_info.ex_orphaned;

	/* Find valid sections to write res info in the shared memory  */
	glnd_find_res_shm_ckpt_empty_section(glnd_cb, &shm_index);
	restart_resource_info.shm_index = res_info->shm_index = shm_index;
	restart_resource_info.valid = GLND_SHM_INFO_VALID;

	/* Fill the POSIX shared memory req info */
	memset(&res_info_write, '\0', sizeof(NCS_OS_POSIX_SHM_REQ_INFO));

	res_info_write.type = NCS_OS_POSIX_SHM_REQ_WRITE;
	res_info_write.info.write.i_addr = glnd_cb->glnd_res_shm_base_addr;
	res_info_write.info.write.i_from_buff = &restart_resource_info;
	res_info_write.info.write.i_offset = shm_index * sizeof(GLND_RESTART_RES_INFO);
	res_info_write.info.write.i_write_size = sizeof(GLND_RESTART_RES_INFO);

	rc = ncs_os_posix_shm(&res_info_write);
	if (rc != NCSCC_RC_SUCCESS) {
		m_LOG_GLND(GLND_RESOURCE_SHM_WRITE_FAILURE, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSCC_RC_FAILURE,
			   __FILE__, __LINE__, 0, res_info->resource_id, 0);
		return rc;
	}
	return rc;

}

/*****************************************************************************
  PROCEDURE NAME : glnd_restart_lock_event_info_ckpt_write()

  DESCRIPTION    : Checkpoints resource_info.

  ARGUMENTS      : glnd_cb        - ptr to the GLND control block

  RETURNS        :

  NOTES          : None
*****************************************************************************/
uns32 glnd_restart_lock_event_info_ckpt_write(GLND_CB *glnd_cb, GLSV_RESTART_BACKUP_EVT_INFO restart_backup_evt)
{
	NCS_OS_POSIX_SHM_REQ_INFO evt_info_write;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* Fill the POSIX shared memory req info */
	memset(&evt_info_write, '\0', sizeof(NCS_OS_POSIX_SHM_REQ_INFO));

	evt_info_write.type = NCS_OS_POSIX_SHM_REQ_WRITE;
	evt_info_write.info.write.i_addr = glnd_cb->glnd_evt_shm_base_addr;
	evt_info_write.info.write.i_from_buff = &restart_backup_evt;
	evt_info_write.info.write.i_offset = restart_backup_evt.shm_index * sizeof(GLSV_RESTART_BACKUP_EVT_INFO);
	evt_info_write.info.write.i_write_size = sizeof(GLSV_RESTART_BACKUP_EVT_INFO);

	rc = ncs_os_posix_shm(&evt_info_write);
	if (rc != NCSCC_RC_SUCCESS) {
		m_LOG_GLND(GLND_EVT_LIST_SHM_WRITE_FAILURE, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSCC_RC_FAILURE,
			   __FILE__, __LINE__, 0, 0, 0);
		return rc;
	}
	return rc;

}

/*****************************************************************************
  PROCEDURE NAME : glnd_restart_resource_info_ckpt_overwrite()

  DESCRIPTION    : Checkpoints resource_info.

  ARGUMENTS      : glnd_cb        - ptr to the GLND control block
                   resource_info  - resource info to be checkpointed

  RETURNS        :

  NOTES          : None
*****************************************************************************/
uns32 glnd_restart_resource_info_ckpt_overwrite(GLND_CB *glnd_cb, GLND_RESOURCE_INFO *res_info)
{
	GLND_RESTART_RES_INFO restart_resource_info;
	GLND_RESTART_RES_INFO *glnd_res_shm_base_addr = NULL;
	NCS_OS_POSIX_SHM_REQ_INFO res_info_write;
	uns32 rc = NCSCC_RC_SUCCESS;

	glnd_res_shm_base_addr = glnd_cb->glnd_res_shm_base_addr;

	/* Fill restart_resource_info */
	memset(&restart_resource_info, '\0', sizeof(GLND_RESTART_RES_INFO));
	restart_resource_info.resource_id = res_info->resource_id;
	restart_resource_info.resource_name = res_info->resource_name;
	restart_resource_info.lcl_ref_cnt = res_info->lcl_ref_cnt;
	restart_resource_info.master_mds_dest = res_info->master_mds_dest;
	restart_resource_info.status = res_info->status;
	restart_resource_info.master_status = res_info->master_status;
	restart_resource_info.pr_orphan_req_count = res_info->lck_master_info.pr_orphan_req_count;
	restart_resource_info.ex_orphan_req_count = res_info->lck_master_info.ex_orphan_req_count;
	restart_resource_info.pr_orphaned = res_info->lck_master_info.pr_orphaned;
	restart_resource_info.ex_orphaned = res_info->lck_master_info.ex_orphaned;
	restart_resource_info.shm_index = res_info->shm_index;
	restart_resource_info.valid = GLND_SHM_INFO_VALID;

	memset((glnd_res_shm_base_addr + res_info->shm_index), '\0', sizeof(GLND_RESTART_RES_INFO));

	/* Fill the POSIX shared memory req info */
	memset(&res_info_write, '\0', sizeof(NCS_OS_POSIX_SHM_REQ_INFO));

	res_info_write.type = NCS_OS_POSIX_SHM_REQ_WRITE;
	res_info_write.info.write.i_addr = glnd_cb->glnd_res_shm_base_addr;
	res_info_write.info.write.i_from_buff = &restart_resource_info;
	res_info_write.info.write.i_offset = res_info->shm_index * sizeof(GLND_RESTART_RES_INFO);
	res_info_write.info.write.i_write_size = sizeof(GLND_RESTART_RES_INFO);

	rc = ncs_os_posix_shm(&res_info_write);
	if (rc != NCSCC_RC_SUCCESS) {
		m_LOG_GLND(GLND_RESOURCE_SHM_WRITE_FAILURE, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSCC_RC_FAILURE,
			   __FILE__, __LINE__, 0, res_info->resource_id, 0);
		return rc;
	}
	return rc;

}

/*****************************************************************************
  PROCEDURE NAME : glnd_restart_res_lock_list_ckpt_write()

  DESCRIPTION    : Checkpoints resource_lock_list_info.

  ARGUMENTS      : glnd_cb             - ptr to the GLND control block
                   res_lock_list_info  - resource lock list info to be checkpointed

  RETURNS        :

  NOTES          : None
*****************************************************************************/
uns32 glnd_restart_res_lock_list_ckpt_write(GLND_CB *glnd_cb, GLND_RES_LOCK_LIST_INFO *res_lock_list,
					    SaLckResourceIdT res_id, SaLckHandleT app_handle_id, uint8_t to_which_list)
{
	GLND_RESTART_RES_LOCK_LIST_INFO restart_res_lock_list_info;
	NCS_OS_POSIX_SHM_REQ_INFO lck_list_info_write;
	uns32 rc = NCSCC_RC_SUCCESS;
	uns32 shm_index;

	memset(&restart_res_lock_list_info, 0, sizeof(GLND_RESTART_RES_LOCK_LIST_INFO));

	/* Fill resstart_res_lock_list_info */
	restart_res_lock_list_info.app_handle_id = app_handle_id;
	restart_res_lock_list_info.resource_id = res_id;
	restart_res_lock_list_info.lcl_resource_id = res_lock_list->lcl_resource_id;
	restart_res_lock_list_info.lck_info_hdl_id = res_lock_list->lck_info_hdl_id;
	restart_res_lock_list_info.lock_info = res_lock_list->lock_info;
	restart_res_lock_list_info.req_mdest_id = res_lock_list->req_mdest_id;
	restart_res_lock_list_info.glnd_res_lock_mds_ctxt = res_lock_list->glnd_res_lock_mds_ctxt;
	restart_res_lock_list_info.unlock_req_sent = res_lock_list->unlock_req_sent;
	restart_res_lock_list_info.unlock_call_type = res_lock_list->unlock_call_type;
	restart_res_lock_list_info.to_which_list = to_which_list;
	restart_res_lock_list_info.non_master_status = res_lock_list->non_master_status;

	/* Find valid sections to write res info in the shared memory  */
	glnd_find_lck_shm_ckpt_empty_section(glnd_cb, &shm_index);
	restart_res_lock_list_info.shm_index = res_lock_list->shm_index = shm_index;
	restart_res_lock_list_info.valid = GLND_SHM_INFO_VALID;

	/* Fill the POSIX shared memory req info */
	memset(&lck_list_info_write, '\0', sizeof(NCS_OS_POSIX_SHM_REQ_INFO));

	lck_list_info_write.type = NCS_OS_POSIX_SHM_REQ_WRITE;
	lck_list_info_write.info.write.i_addr = glnd_cb->glnd_lck_shm_base_addr;
	lck_list_info_write.info.write.i_from_buff = &restart_res_lock_list_info;
	lck_list_info_write.info.write.i_offset = shm_index * sizeof(GLND_RESTART_RES_LOCK_LIST_INFO);
	lck_list_info_write.info.write.i_write_size = sizeof(GLND_RESTART_RES_LOCK_LIST_INFO);

	rc = ncs_os_posix_shm(&lck_list_info_write);
	if (rc != NCSCC_RC_SUCCESS) {
		m_LOG_GLND(GLND_LCK_LIST_SHM_WRITE_FAILURE, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSCC_RC_FAILURE,
			   __FILE__, __LINE__, app_handle_id, res_id, res_lock_list->lock_info.lcl_lockid);
		return rc;
	}
	return rc;

}

/*****************************************************************************
  PROCEDURE NAME : glnd_restart_res_lock_list_ckpt_overwrite()

  DESCRIPTION    : Checkpoints resource_lock_list_info.

  ARGUMENTS      : glnd_cb             - ptr to the GLND control block
                   res_lock_list_info  - resource lock list info to be checkpointed

  RETURNS        :

  NOTES          : None
*****************************************************************************/
uns32 glnd_restart_res_lock_list_ckpt_overwrite(GLND_CB *glnd_cb, GLND_RES_LOCK_LIST_INFO *res_lock_list,
						SaLckResourceIdT res_id, SaLckHandleT app_handle_id, uint8_t to_which_list)
{
	GLND_RESTART_RES_LOCK_LIST_INFO restart_res_lock_list_info;
	NCS_OS_POSIX_SHM_REQ_INFO lck_list_info_write;
	GLND_RESTART_RES_LOCK_LIST_INFO *shm_base_addr = NULL;
	uns32 rc = NCSCC_RC_SUCCESS;

	shm_base_addr = glnd_cb->glnd_lck_shm_base_addr;

	/* Fill resstart_res_lock_list_info */
	restart_res_lock_list_info.app_handle_id = app_handle_id;
	restart_res_lock_list_info.resource_id = res_id;
	restart_res_lock_list_info.lcl_resource_id = res_lock_list->lcl_resource_id;
	restart_res_lock_list_info.lck_info_hdl_id = res_lock_list->lck_info_hdl_id;
	restart_res_lock_list_info.lock_info = res_lock_list->lock_info;
	restart_res_lock_list_info.req_mdest_id = res_lock_list->req_mdest_id;
	restart_res_lock_list_info.glnd_res_lock_mds_ctxt = res_lock_list->glnd_res_lock_mds_ctxt;
	restart_res_lock_list_info.unlock_req_sent = res_lock_list->unlock_req_sent;
	restart_res_lock_list_info.to_which_list = to_which_list;
	restart_res_lock_list_info.non_master_status = res_lock_list->non_master_status;
	restart_res_lock_list_info.shm_index = res_lock_list->shm_index;
	restart_res_lock_list_info.valid = GLND_SHM_INFO_VALID;

	memset((shm_base_addr + res_lock_list->shm_index), '\0', sizeof(GLND_RESTART_RES_LOCK_LIST_INFO));

	/* Fill the POSIX shared memory req info */
	memset(&lck_list_info_write, '\0', sizeof(NCS_OS_POSIX_SHM_REQ_INFO));

	lck_list_info_write.type = NCS_OS_POSIX_SHM_REQ_WRITE;
	lck_list_info_write.info.write.i_addr = glnd_cb->glnd_lck_shm_base_addr;
	lck_list_info_write.info.write.i_from_buff = &restart_res_lock_list_info;
	lck_list_info_write.info.write.i_offset = res_lock_list->shm_index * sizeof(GLND_RESTART_RES_LOCK_LIST_INFO);
	lck_list_info_write.info.write.i_write_size = sizeof(GLND_RESTART_RES_LOCK_LIST_INFO);

	rc = ncs_os_posix_shm(&lck_list_info_write);
	if (rc != NCSCC_RC_SUCCESS) {
		m_LOG_GLND(GLND_LCK_LIST_SHM_WRITE_FAILURE, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSCC_RC_FAILURE,
			   __FILE__, __LINE__, app_handle_id, res_id, res_lock_list->lock_info.lcl_lockid);
		return rc;
	}
	return rc;

}

/*****************************************************************************
  PROCEDURE NAME : glnd_restart_res_lock_ckpt_read()

  DESCRIPTION    : Reads resource_info from checkpoint.

  ARGUMENTS      : glnd_cb        - ptr to the GLND control block
                   section_id     -
  RETURNS        :

  NOTES          : None
*****************************************************************************/
uns32 glnd_restart_res_lock_ckpt_read(GLND_CB *glnd_cb, GLND_RESTART_RES_LOCK_LIST_INFO *restart_res_lock_info,
				      uns32 offset)
{
	SaAisErrorT rc = SA_AIS_OK;
	NCS_OS_POSIX_SHM_REQ_INFO read_req;

	/*Use read option of shared memory to fill ckpt_queue_info */
	memset(&read_req, '\0', sizeof(NCS_OS_POSIX_SHM_REQ_INFO));

	read_req.type = NCS_OS_POSIX_SHM_REQ_READ;
	read_req.info.read.i_addr = glnd_cb->glnd_lck_shm_base_addr;
	read_req.info.read.i_read_size = sizeof(GLND_RESTART_RES_LOCK_LIST_INFO);
	read_req.info.read.i_offset = offset * sizeof(GLND_RESTART_RES_LOCK_LIST_INFO);
	read_req.info.read.i_to_buff = (GLND_RESTART_RES_LOCK_LIST_INFO *)restart_res_lock_info;

	rc = ncs_os_posix_shm(&read_req);
	if (rc != NCSCC_RC_SUCCESS)
		m_LOG_GLND(GLND_LCK_LIST_SHM_READ_FAILURE, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSCC_RC_FAILURE,
			   __FILE__, __LINE__, 0, 0, 0);

	return rc;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_restart_resource_ckpt_read()

  DESCRIPTION    : Reads resource_info from checkpoint.

  ARGUMENTS      : glnd_cb        - ptr to the GLND control block
                   section_id     -
  RETURNS        :

  NOTES          : None
*****************************************************************************/
uns32 glnd_restart_resource_ckpt_read(GLND_CB *glnd_cb, GLND_RESTART_RES_INFO *restart_resource_info, uns32 offset)
{
	NCS_OS_POSIX_SHM_REQ_INFO read_req;
	uns32 rc = NCSCC_RC_FAILURE;

	/*Use read option of shared memory to fill ckpt_queue_info */
	memset(&read_req, '\0', sizeof(NCS_OS_POSIX_SHM_REQ_INFO));

	read_req.type = NCS_OS_POSIX_SHM_REQ_READ;
	read_req.info.read.i_addr = glnd_cb->glnd_res_shm_base_addr;
	read_req.info.read.i_read_size = sizeof(GLND_RESTART_RES_INFO);
	read_req.info.read.i_offset = offset * sizeof(GLND_RESTART_RES_INFO);
	read_req.info.read.i_to_buff = (GLND_RESTART_RES_INFO *)restart_resource_info;

	rc = ncs_os_posix_shm(&read_req);
	if (rc != NCSCC_RC_SUCCESS)
		m_LOG_GLND(GLND_RESOURCE_SHM_READ_FAILURE, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSCC_RC_FAILURE,
			   __FILE__, __LINE__, 0, 0, 0);

	return rc;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_restart_backup_event_read()

  DESCRIPTION    : Reads resource_info from checkpoint.

  ARGUMENTS      : glnd_cb        - ptr to the GLND control block
                   section_id     -
  RETURNS        :

  NOTES          : None
*****************************************************************************/
uns32 glnd_restart_backup_event_read(GLND_CB *glnd_cb, GLSV_RESTART_BACKUP_EVT_INFO *restart_backup_evt, uns32 offset)
{
	SaAisErrorT rc = SA_AIS_OK;
	NCS_OS_POSIX_SHM_REQ_INFO read_req;

	/*Use read option of shared memory to fill */
	memset(&read_req, '\0', sizeof(NCS_OS_POSIX_SHM_REQ_INFO));

	read_req.type = NCS_OS_POSIX_SHM_REQ_READ;
	read_req.info.read.i_addr = glnd_cb->glnd_evt_shm_base_addr;
	read_req.info.read.i_read_size = sizeof(GLSV_RESTART_BACKUP_EVT_INFO);
	read_req.info.read.i_offset = offset * sizeof(GLSV_RESTART_BACKUP_EVT_INFO);
	read_req.info.read.i_to_buff = (GLSV_RESTART_BACKUP_EVT_INFO *)restart_backup_evt;

	rc = ncs_os_posix_shm(&read_req);
	if (rc != NCSCC_RC_SUCCESS)
		m_LOG_GLND(GLND_EVT_LIST_SHM_READ_FAILURE, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSCC_RC_FAILURE,
			   __FILE__, __LINE__, 0, 0, 0);

	return rc;
}
