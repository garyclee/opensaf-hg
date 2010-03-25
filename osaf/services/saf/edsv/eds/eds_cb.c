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
        
This contains the EDS_CB functions.
          
******************************************************************************
*/
#include "eds.h"
#include "poll.h"
#include "signal.h"

#define FD_AMF 0
#define FD_MBCSV 1
#define FD_MBX 2
#define FD_CLM 3
#define FD_IMM 4		/* Must be the last in the fds array */

static struct pollfd fds[5];
static nfds_t nfds = 5;


/****************************************************************************
 * Name          : eds_cb_init
 *
 * Description   : This function initializes the EDS_CB including the 
 *                 Patricia trees.
 *                 
 *
 * Arguments     : eds_cb * - Pointer to the EDS_CB.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 eds_cb_init(EDS_CB *eds_cb)
{
	NCS_PATRICIA_PARAMS reg_param, cname_param;

	memset(&reg_param, 0, sizeof(NCS_PATRICIA_PARAMS));
	memset(&cname_param, 0, sizeof(NCS_PATRICIA_PARAMS));

	reg_param.key_size = sizeof(uns32);
	cname_param.key_size = sizeof(SaNameT);

	/* Assign Initial HA state */
	eds_cb->ha_state = EDS_HA_INIT_STATE;
	eds_cb->csi_assigned = FALSE;
	eds_cb->cluster_node_list = NULL;
	/* Assign Version. Currently, hardcoded, This will change later */
	m_GET_MY_VERSION(eds_cb->eds_version);

	/* Initialize patricia tree for reg list */
	if (NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&eds_cb->eda_reg_list, &reg_param))
		return NCSCC_RC_FAILURE;

	/* Initialize patricia tree for channel name list */
	if (NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&eds_cb->eds_cname_list, &cname_param))
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_cb_destroy
 *
 * Description   : This function destroys the EDS_CB including the 
 *                 Patricia trees.
 *                 
 *
 * Arguments     : eds_cb * - Pointer to the EDS_CB.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
void eds_cb_destroy(EDS_CB *eds_cb)
{
	ncs_patricia_tree_destroy(&eds_cb->eda_reg_list);
	/* Check if other lists are deleted as well */
	ncs_patricia_tree_destroy(&eds_cb->eds_cname_list);

	return;
}

/****************************************************************************
 * Name          : eds_process_mbx
 *
 * Description   : This is the function which process the IPC mail box of 
 *                 EDS 
 *
 * Arguments     : mbx  - This is the mail box pointer on which EDS is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void eds_process_mbx(SYSF_MBX *mbx)
{
	EDSV_EDS_EVT *evt = NULL;

	evt = (EDSV_EDS_EVT *)m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt);
	if (evt != NULL) {
		if ((evt->evt_type >= EDSV_EDS_EVT_BASE) && (evt->evt_type < EDSV_EDS_EVT_MAX)) {
			/* This event belongs to EDS main event dispatcher */
			eds_process_evt(evt);
		} else {
			/* Free the event */
			m_LOG_EDSV_S(EDS_EVT_UNKNOWN, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, evt->evt_type, __FILE__,
				     __LINE__, 0);
			eds_evt_destroy(evt);
		}
	}
	return;
}

/****************************************************************************
 * Name          : eds_main_process
 *
 * Description   : This is the function which is given as a input to the 
 *                 EDS task.
 *                 This function will be select of both the FD's (AMF FD and
 *                 Mail Box FD), depending on which FD has been selected, it
 *                 will call the corresponding routines.
 *
 * Arguments     : mbx  - This is the mail box pointer on which EDS is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void eds_main_process(SYSF_MBX *mbx)
{

	NCS_SEL_OBJ mbx_fd;
	SaAisErrorT error = SA_AIS_OK;
	EDS_CB *eds_cb = NULL;

	if (NULL == (eds_cb = (EDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl))) {
		m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__, 0);
		return;
	}

	mbx_fd = m_NCS_IPC_GET_SEL_OBJ(&eds_cb->mbx);

	/* Give back the handle */
	ncshm_give_hdl(gl_eds_hdl);

	/* Initialize with IMM */
	if (eds_imm_init(eds_cb) == SA_AIS_OK) {
		if (eds_cb->ha_state == SA_AMF_HA_ACTIVE) {
			if (eds_imm_declare_implementer(eds_cb->immOiHandle) != SA_AIS_OK)
				m_EDSV_DEBUG_CONS_PRINTF("Implementer Set Failed\n");
		}
	} else
		m_EDSV_DEBUG_CONS_PRINTF("Imm Init Failed \n");

	/* Set up all file descriptors to listen to */
	fds[FD_AMF].fd = eds_cb->amfSelectionObject;
	fds[FD_AMF].events = POLLIN;
	fds[FD_CLM].fd = eds_cb->clm_sel_obj;
	fds[FD_CLM].events = POLLIN;
	fds[FD_MBCSV].fd = eds_cb->mbcsv_sel_obj;
	fds[FD_MBCSV].events = POLLIN;
	fds[FD_MBX].fd = mbx_fd.rmv_obj;
	fds[FD_MBX].events = POLLIN;
	fds[FD_IMM].fd = eds_cb->imm_sel_obj;
	fds[FD_IMM].events = POLLIN;

	while (1) {
		int ret = poll(fds, nfds, -1);

		if (ret == -1) {
			if (errno == EINTR)
				continue;

			m_EDSV_DEBUG_CONS_PRINTF("poll failed - %s", strerror(errno));
			break;
		}
		/* process all the AMF messages */
		if (fds[FD_AMF].revents & POLLIN) {
			m_EDSV_DEBUG_CONS_PRINTF("AMF EVENT HAS OCCURRED....\n");
			/* dispatch all the AMF pending callbacks */
			error = saAmfDispatch(eds_cb->amf_hdl, SA_DISPATCH_ALL);
			if (error != SA_AIS_OK) {
				m_LOG_EDSV_S(EDS_AMF_DISPATCH_FAILURE, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR,
						     error, __FILE__, __LINE__, 0);
			}
		}

		/* process all mbcsv messages */
		if (fds[FD_MBCSV].revents & POLLIN) {
			m_EDSV_DEBUG_CONS_PRINTF("MBCSV EVENT HAS OCCURRED....\n");
			error = eds_mbcsv_dispatch(eds_cb->mbcsv_hdl);
			if (NCSCC_RC_SUCCESS != error)
				m_EDSV_DEBUG_CONS_PRINTF("MBCSV DISPATCH FAILED...\n");
			else
				m_EDSV_DEBUG_CONS_PRINTF("MBCSV DISPATCH SUCCESS...\n");
		}

		/* Process the EDS Mail box, if eds is ACTIVE. */
		if (fds[FD_MBX].revents & POLLIN) {
			m_EDSV_DEBUG_CONS_PRINTF("MAILBOX EVENT HAS OCCURRED....\n");
			/* now got the IPC mail box event */
			eds_process_mbx(mbx);
		}

		/* process the CLM messages */
		if (fds[FD_CLM].revents & POLLIN) {
			m_EDSV_DEBUG_CONS_PRINTF("CLM EVENT HAS OCCURRED....\n");

			/* dispatch all the AMF pending callbacks */
			error = saClmDispatch(eds_cb->clm_hdl, SA_DISPATCH_ALL);
			if (error != SA_AIS_OK)
				m_EDSV_DEBUG_CONS_PRINTF("CLM Dispatch failed, error \n");
		}

		/* process the IMM messages */
		if (eds_cb->immOiHandle && fds[FD_IMM].revents & POLLIN) {
			m_EDSV_DEBUG_CONS_PRINTF("IMM EVENT HAS OCCURRED....\n");

			/* dispatch all the IMM pending function */
			error = saImmOiDispatch(eds_cb->immOiHandle, SA_DISPATCH_ONE);

			/*
			 ** BAD_HANDLE is interpreted as an IMM service restart. Try 
			 ** reinitialize the IMM OI API in a background thread and let 
			 ** this thread do business as usual especially handling write 
			 ** requests.
			 **
			 ** All other errors are treated as non-recoverable (fatal) and will
			 ** cause an exit of the process.
			 */

			if (error == SA_AIS_ERR_BAD_HANDLE) {
				m_EDSV_DEBUG_CONS_PRINTF("saImmOiDispatch returned BAD_HANDLE %u", error);

				/* Invalidate the IMM OI handle. */
				eds_cb->immOiHandle = 0;
				/* 
				 ** Skip the IMM file descriptor in next poll(), IMM fd must
				 ** be the last in the fd array.
				 */
				nfds = FD_IMM;
				/* Reinitiate IMM */
				error = eds_imm_init(eds_cb);
				if (error == SA_AIS_OK) {
					/* If this is the active server, become implementer again. */
					if (eds_cb->ha_state == SA_AMF_HA_ACTIVE)
						eds_imm_declare_implementer(eds_cb->immOiHandle);
					m_EDSV_DEBUG_CONS_PRINTF("eds_imm_init successful\n");
				}
			
				fds[FD_IMM].fd = eds_cb->imm_sel_obj;
				nfds = FD_IMM + 1;
			} else if (error != SA_AIS_OK) {
				m_EDSV_DEBUG_CONS_PRINTF("saImmOiDispatch FAILED: %u", error);
				break;
			}

		}
	}

	return;
}	/* End eds_main_process() */
