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

  This file captures the initialization of BOM Access Manager.
  
******************************************************************************
*/

/* NCS specific include files */

#include <gl_defs.h>
#include <t_suite.h>
#include <ncs_mib_pub.h>
#include <ncs_lib.h>
#include <ncsgl_defs.h>
#include <mds_papi.h>
#include "ncsmiblib.h"

#include "bam.h"
#include "bam_log.h"
#include "ncs_avsv_bam_msg.h"

EXTERN_C uns32 gl_bam_avd_cfg_msg_num;
EXTERN_C uns32 gl_ncs_bam_hdl;

static uns32 bam_process_si_dep_list(void);

/****************************************************************************
*  Name          : avd_bam_build_si_dep_list
* 
*  Description   : This routine builds the SI-SI dependency list to update
*                  the SI-SI dependency cfg-data to AVD at the final stages
*                  just before sending CFG-DONE message.
* 
*  Arguments     : si_name - Sponsor SI name
*                  si_dep_name - Dependent SI name
*                  tol_time - Tolerance Time value
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
uns32 avd_bam_build_si_dep_list(char *si_name, char *si_dep_name, char *tol_time)
{
	NCS_BAM_CB *bam_cb = NULL;
	BAM_SI_DEP_NODE *si_dep_node = NULL;

	if ((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL) {
		m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
		return NCSCC_RC_FAILURE;
	}

	si_dep_node = (BAM_SI_DEP_NODE *)m_MMGR_ALLOC_BAM_SI_DEP(sizeof(BAM_SI_DEP_NODE));
	if (si_dep_node == NULL) {
		ncshm_give_hdl(gl_ncs_bam_hdl);
		/* LOG the error message */
		return NCSCC_RC_FAILURE;
	}

	memset(si_dep_node, 0, sizeof(BAM_SI_DEP_NODE));

	strncpy(si_dep_node->si_name, si_name, BAM_MAX_LEN - 1);
	strncpy(si_dep_node->si_dep_name, si_dep_name, BAM_MAX_LEN - 1);
	strncpy(si_dep_node->tol_time, tol_time, BAM_MAX_LEN - 1);

	si_dep_node->next = bam_cb->si_dep_list;
	bam_cb->si_dep_list = si_dep_node;

	ncshm_give_hdl(gl_ncs_bam_hdl);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
*  Name          : bam_process_si_dep_list
* 
*  Description   : This function updates the SI-SI dependency data to AVD
* 
*  Arguments     : NONE.
* 
*  Return Values : 
* 
*  Notes         : 
******************************************************************************/
static uns32 bam_process_si_dep_list()
{
	NCS_BAM_CB *cb = NULL;
	SaAisErrorT rc = SA_AIS_OK;
	NCSMIB_TBL_ID table_id;
	uns32 param_id;
	NCSMIB_FMAT_ID format;
	NCSMIB_IDX mib_idx;
	char rowStatus[4];
	BAM_SI_DEP_NODE *si_dep_node = NULL;

	if ((cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL) {
		m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
		return NCSCC_RC_FAILURE;
	}

	while (cb->si_dep_list != NULL) {
		si_dep_node = cb->si_dep_list;

		/* Update the SI dependency data */
		ncs_bam_build_mib_idx(&mib_idx, si_dep_node->si_name, NCSMIB_FMAT_OCT);
		ncs_bam_add_sec_mib_idx(&mib_idx, si_dep_node->si_dep_name, NCSMIB_FMAT_OCT);

		rc = ncs_bam_search_table_for_oid(gl_amfConfig_table,
						  gl_amfConfig_table_size,
						  "SIInstance", "SIdepToltime", &table_id, &param_id, &format);

		if (rc == SA_AIS_OK)
			ncs_bam_generate_counter64_mibset(table_id, param_id, &mib_idx, si_dep_node->tol_time);

		rc = ncs_bam_search_table_for_oid(gl_amfConfig_table,
						  gl_amfConfig_table_size,
						  "SIInstance", "SIdepRowStatus", &table_id, &param_id, &format);

		if (rc == SA_AIS_OK) {
			sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
			ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, rowStatus, format);
		}

		ncs_bam_free(mib_idx.i_inst_ids);

		cb->si_dep_list = cb->si_dep_list->next;

		m_MMGR_FREE_NCS_BAM_SI_DEP(si_dep_node);
	}

	ncshm_give_hdl(gl_ncs_bam_hdl);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
*  Name          : avd_bam_snd_message
* 
*  Description   : This is a routine that is invoked to send a message
*                  to AVD.
* 
*  Arguments     : AVD_BAM_MSG - the message to be sent.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
uns32 avd_bam_snd_message(AVD_BAM_MSG *snd_msg)
{
	NCSMDS_INFO snd_mds;
	uns32 rc;
	NCS_BAM_CB *bam_cb = NULL;

	if ((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL) {
		m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
		return NCSCC_RC_FAILURE;
	}

	memset(&snd_mds, '\0', sizeof(NCSMDS_INFO));

	snd_mds.i_mds_hdl = bam_cb->mds_hdl;
	snd_mds.i_svc_id = NCSMDS_SVC_ID_BAM;
	snd_mds.i_op = MDS_SEND;
	snd_mds.info.svc_send.i_msg = (NCSCONTEXT)snd_msg;
	snd_mds.info.svc_send.i_to_svc = NCSMDS_SVC_ID_AVD;
	snd_mds.info.svc_send.i_priority = MDS_SEND_PRIORITY_HIGH;
	snd_mds.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
	snd_mds.info.svc_send.info.snd.i_to_dest = bam_cb->avd_dest;
	if ((rc = ncsmds_api(&snd_mds)) != NCSCC_RC_SUCCESS) {
		ncshm_give_hdl(gl_ncs_bam_hdl);
		return rc;
	}

	ncshm_give_hdl(gl_ncs_bam_hdl);

	/* m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_SEND); */
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
*  Name          : ncs_bam_send_cfg_done_msg
* 
*  Description   : This is a routine that is invoked to send a config DONE
*                  message to AVD.
* 
*  Arguments     : NONE.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
void ncs_bam_send_cfg_done_msg()
{
	AVD_BAM_MSG *snd_msg = m_MMGR_ALLOC_AVD_BAM_MSG;

	/* Send SI-SI dependency cfg's to AVD just before sending the cfg-DONE message */
	if (bam_process_si_dep_list() != NCSCC_RC_SUCCESS) {
		m_LOG_BAM_MDS_ERR(BAM_MDS_SEND_ERROR, NCSFL_SEV_ERROR);
		m_MMGR_FREE_AVD_BAM_MSG(snd_msg);
		return;
	}

	/* Fill in the message */
	memset(snd_msg, '\0', sizeof(AVD_BAM_MSG));
	snd_msg->msg_type = BAM_AVD_CFG_DONE_MSG;
	snd_msg->msg_info.msg.check_sum = gl_bam_avd_cfg_msg_num;

	if (avd_bam_snd_message(snd_msg) != NCSCC_RC_SUCCESS) {
		m_LOG_BAM_MDS_ERR(BAM_MDS_SEND_ERROR, NCSFL_SEV_ERROR);
		m_MMGR_FREE_AVD_BAM_MSG(snd_msg);
	} else {
		m_LOG_BAM_MDS_ERR(BAM_MDS_SEND_SUCCESS, NCSFL_SEV_INFO);
	}

	return;
}
