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

  This file contains all Public APIs for the Flex Log server (DTS) portion
  of the Distributed Tracing Service (DTSv) subsystem.

  ..............................................................................

  FUNCTIONS INCLUDED in this module:

  dts_lib_req                  - SE API to create DTS.
  dts_lib_init                 - Create DTS service.
  dts_lib_destroy              - Distroy DTS service.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include <configmake.h>

#include "rda_papi.h"
#include "ncs_tasks.h"
#include "ncs_main_pub.h"

#include "dts.h"

static NCS_BOOL dts_clear_mbx(NCSCONTEXT arg, NCSCONTEXT mbx_msg);

#define DTS_ASCII_SPEC_CONFIG_FILE	PKGSYSCONFDIR "/dts_ascii_spec_config"
#define m_DTS_SLOT_ID_FILE		PKGSYSCONFDIR "/slot_id"

/****************************************************************************
 * Name          : dts_lib_req
 *
 * Description   : This is the NCS SE API which is used to init/destroy or 
 *                 Create/destroy DTS. This will be called by SBOM.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32 dts_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uns32 res = NCSCC_RC_FAILURE;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = dts_lib_init(req_info);
		break;

	case NCS_LIB_REQ_DESTROY:
		res = dts_lib_destroy();
		break;

	default:
		break;
	}
	return (res);
}

/****************************************************************************
 * Name          : dts_lib_init
 *
 * Description   : This is the function which initalize the dts libarary.
 *                 This function creates an IPC mail Box and spawns DTS
 *                 thread.
 *                 This function initializes the CB, handle manager and MDS.
 *
 * Arguments     : req_info - Request info.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 dts_lib_init(NCS_LIB_REQ_INFO *req_info)
{
	NCSCONTEXT task_handle=0;
	DTS_CB *inst = &dts_cb;
	PCS_RDA_REQ pcs_rda_req;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(inst, 0, sizeof(DTS_CB));

#if (DTS_SIM_TEST_ENV == 1)
	if ('n' == ncs_util_get_char_option(req_info->info.create.argc, req_info->info.create.argv, "DTS_TEST=")) {
		inst->is_test = FALSE;
	} else
		inst->is_test = TRUE;
#endif

	/* Initialize SAF stuff */
	/* Fill in the Health check key */
	strcpy((char *)inst->health_chk_key.key, DTS_AMF_HEALTH_CHECK_KEY);
	inst->health_chk_key.keyLen = strlen((char *)inst->health_chk_key.key);

	inst->invocationType = DTS_HB_INVOCATION_TYPE;
	/* Recommended recovery is to failover */
	inst->recommendedRecovery = DTS_RECOVERY;

	/* RDA init */
	memset(&pcs_rda_req, '\0', sizeof(pcs_rda_req));
	pcs_rda_req.req_type = PCS_RDA_LIB_INIT;
	rc = pcs_rda_request(&pcs_rda_req);
	if (rc != PCSRDA_RC_SUCCESS) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_init: pcs_rda_request() failed for PCS_RDA_LIB_INIT");
	}

	/* Get initial role from RDA */
	memset(&pcs_rda_req, '\0', sizeof(pcs_rda_req));
	pcs_rda_req.req_type = PCS_RDA_GET_ROLE;
	rc = pcs_rda_request(&pcs_rda_req);
	if (rc != PCSRDA_RC_SUCCESS) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_init: Failed to get initial role from RDA");
	}

	/* Set initial role now */
	switch (pcs_rda_req.info.io_role) {
	case PCS_RDA_ACTIVE:
		inst->ha_state = SA_AMF_HA_ACTIVE;
		m_DTS_DBG_SINK(NCSCC_RC_SUCCESS, "DTS init role is ACTIVE");
		m_LOG_DTS_API(DTS_INIT_ROLE_ACTIVE);
		break;
	case PCS_RDA_STANDBY:
		inst->ha_state = SA_AMF_HA_STANDBY;
		m_DTS_DBG_SINK(NCSCC_RC_SUCCESS, "DTS init role is STANDBY");
		m_LOG_DTS_API(DTS_INIT_ROLE_STANDBY);
		break;
	default:
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_init: Failed to get initial role from RDA");
	}

	/* RDA finalize */
	memset(&pcs_rda_req, '\0', sizeof(pcs_rda_req));
	pcs_rda_req.req_type = PCS_RDA_LIB_DESTROY;
	rc = pcs_rda_request(&pcs_rda_req);
	if (rc != PCSRDA_RC_SUCCESS) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_init: Failed to perform lib destroy on RDA");
	}

	inst->in_sync = TRUE;

	/* Attempt to open console device for logging */
	inst->cons_fd = -1;
	if ((inst->cons_fd = dts_cons_open(O_RDWR | O_NOCTTY)) < 0) {
		/* Should we return frm here on failure?? I guess NOT */
		m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_init: Failed to open console");
	}

	/* Create DTS mailbox */
	if (m_NCS_IPC_CREATE(&gl_dts_mbx) != NCSCC_RC_SUCCESS) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_init: Failed to create DTS mail box");
	}

	if (NCSCC_RC_SUCCESS != m_NCS_IPC_ATTACH(&gl_dts_mbx)) {
		m_NCS_IPC_RELEASE(&gl_dts_mbx, NULL);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_init: IPC attach failed.");
	}
#if (DTS_FLOW == 1)
	/* Keeping count of messages in DTS mailbox */
	if (NCSCC_RC_SUCCESS != m_NCS_IPC_CONFIG_USR_COUNTERS(&gl_dts_mbx, NCS_IPC_PRIORITY_LOW, &inst->msg_count)) {
		m_NCS_IPC_DETACH(&gl_dts_mbx, dts_clear_mbx, inst);
		m_NCS_IPC_RELEASE(&gl_dts_mbx, NULL);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_lib_init: Failed to initialize DTS msg counters with LEAP");
	}
#endif

	/* Smik - initialize the signal handler */
	if ((dts_app_signal_install(SIGUSR1, dts_amf_sigusr1_handler)) == -1) {
		m_NCS_IPC_DETACH(&gl_dts_mbx, dts_clear_mbx, inst);
		m_NCS_IPC_RELEASE(&gl_dts_mbx, NULL);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_init: Failed to install signal handler");
	}

	{
		DTS_LM_ARG arg;
		memset(&arg, 0, sizeof(DTS_LM_ARG));

		arg.i_op = DTS_LM_OP_CREATE;
		arg.info.create.i_hmpool_id = NCS_HM_POOL_ID_COMMON;
		arg.info.create.i_vrid = 1;
		arg.info.create.task_handle = task_handle;

		/* Now create and initialize DTS control block */
		if (dts_lm(&arg) != NCSCC_RC_SUCCESS) {
			m_NCS_IPC_DETACH(&gl_dts_mbx, dts_clear_mbx, inst);
			m_NCS_IPC_RELEASE(&gl_dts_mbx, NULL);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_init: Failed to create and init DTS CB");
		}
	}



	if (dts_log_bind() != NCSCC_RC_SUCCESS) {
		m_DTS_LK(&inst->lock);
		m_LOG_DTS_LOCK(DTS_LK_LOCKED, &inst->lock);
		/* Do cleanup activities */
		m_NCS_IPC_DETACH(&gl_dts_mbx, dts_clear_mbx, inst);
		m_NCS_IPC_RELEASE(&gl_dts_mbx, NULL);
		inst->created = FALSE;
		dtsv_mbcsv_deregister(inst);
		dts_mds_unreg(inst, TRUE);
		dtsv_clear_asciispec_tree(inst);
		dtsv_clear_libname_tree(inst);
		ncs_patricia_tree_destroy(&inst->svcid_asciispec_tree);
		ncs_patricia_tree_destroy(&inst->libname_asciispec_tree);
		ncs_patricia_tree_destroy(&inst->svc_tbl);
		ncs_patricia_tree_destroy(&inst->dta_list);
		m_DTS_UNLK(&inst->lock);
		m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &inst->lock);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_init: Unable to bind DTS with DTSv");
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : dts_lib_destroy
 *
 * Description   : This is the function which destroy's the DTS lib.
 *                 This function releases the Task and the IPX mail Box.
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 dts_lib_destroy(void)
{

/*#if (DTS_SIM_TEST_ENV == 1)
   if (dts_cb.amf_init == FALSE)
   {*/
	DTS_LM_ARG arg;

	/* Destroy the DTS CB */
	arg.i_op = DTS_LM_OP_DESTROY;
	arg.info.destroy.i_meaningless = (void *)0x1234;

	if (dts_lm(&arg) != NCSCC_RC_SUCCESS) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lib_destroy: Failed to destroy DTS CB");
	}
	/*}
	   #endif */


	m_NCS_IPC_DETACH(&gl_dts_mbx, dts_clear_mbx, &dts_cb);

	m_NCS_IPC_RELEASE(&gl_dts_mbx, NULL);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : dts_clear_mbx
 *
 * Description   : This is the function which deletes all the messages from 
 *                 the mail box.
 *
 * Arguments     : arg     - argument to be passed.
 *                 mbx_msg - Message start pointer.
 *
 * Return Values : TRUE/FALSE
 *
 * Notes         : None.
 *****************************************************************************/
static NCS_BOOL dts_clear_mbx(NCSCONTEXT arg, NCSCONTEXT mbx_msg)
{
	DTSV_MSG *msg = (DTSV_MSG *)mbx_msg;
	DTSV_MSG *mnext;

	mnext = msg;
	while (mnext) {
		mnext = (DTSV_MSG *)msg->next;

		m_MMGR_FREE_DTSV_MSG(msg);

		msg = mnext;
	}
	return TRUE;
}

/****************************************************************************\
*  Name:          dts_apps_ascii_spec_load                                         * 
*                                                                            *
*  Description:   To load/unload the Application specific ASCII Spec table in DTS *  
*                                                                            *
*  Arguments:     uns8* - Name of the Configuration File                     *
*                 uns32 - what_to_do                                         *
*                          1 - REGISTER the ASCII Spec table                 *
*                          0 - UNREGISTER the ASCII spec table               *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
*                 NCSCC_RC_FAILURE   -  failure                              *
\****************************************************************************/
uns32 dts_apps_ascii_spec_load(uns8 *file_name, uns32 what_to_do)
{
	/* get the instruments ready */
	FILE *fp = NULL;
	char lib_name[DTS_MAX_LIBNAME] = { 0 };
	char func_name[DTS_MAX_FUNCNAME] = { 0 };
	int32 nargs = 0;
	uns32 status = NCSCC_RC_SUCCESS;
	uns32 (*reg_unreg_routine) () = NULL;
	char *dl_error = NULL;
	NCS_LIB_REQ_INFO req_info;
	ASCII_SPEC_LIB *lib_entry;
	NCS_OS_DLIB_HDL *lib_hdl = NULL;
	char dbg_str[DTS_MAX_LIB_DBG];

	/* open the file */
	fp = fopen((char *)file_name, "r");
	if (fp == NULL) {
		/* inform that there is no such file, and return */
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_apps_ascii_spec_load: Unable to load the file");
	}

	/* continue till the you reach the end of the file */
	while (((nargs = fscanf(fp, "%s %s", lib_name, func_name)) == 2) && (nargs != EOF)) {
		/* Check if lib is already loaded or not */
		if ((lib_entry =
		     (ASCII_SPEC_LIB *)ncs_patricia_tree_get(&dts_cb.libname_asciispec_tree,
							     (const uns8 *)lib_name)) != NULL) {
			memset(func_name, 0, DTS_MAX_FUNCNAME);
			memset(lib_name, 0, DTS_MAX_LIBNAME);
			continue;
		}

		/* Load the library if REGISTRATION is to be peformed */
		if (what_to_do == 1) {
			lib_hdl = m_NCS_OS_DLIB_LOAD(lib_name, m_NCS_OS_DLIB_ATTR);
			if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) {
				/* log the error returned from dlopen() */
				m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_apps_ascii_spec_load: Unable to load library.");

				dts_log(NCSFL_SEV_ERROR, "\ndts_apps_ascii_spec_load(): m_NCS_OS_DLIB_LOAD() failed: %s\n", lib_name);
				reg_unreg_routine = NULL;
				lib_hdl = NULL;
				memset(func_name, 0, DTS_MAX_FUNCNAME);
				memset(lib_name, 0, DTS_MAX_LIBNAME);
				continue;
			}
		}

		/* load the symbol into DTS Engine process space */
		reg_unreg_routine = m_NCS_OS_DLIB_SYMBOL(lib_hdl, func_name);
		if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) {
			/* log the error returned from dlopen() */
			m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_apps_ascii_spec_load: Unable to load symbol");

			dts_log
			    (NCSFL_SEV_ERROR, "\ndts_apps_ascii_spec_load(): m_NCS_OS_DLIB_SYMBOL()  failed(lib name, func name, error): %s, %s, %s\n",
			     lib_name, func_name, dl_error);

			reg_unreg_routine = NULL;
			lib_hdl = NULL;
			memset(func_name, 0, DTS_MAX_FUNCNAME);
			memset(lib_name, 0, DTS_MAX_LIBNAME);
			continue;
		}

		/* do the INIT/DEINIT now... */
		if (reg_unreg_routine != NULL) {
			memset(&req_info, 0, sizeof(NCS_LIB_REQ_INFO));
			req_info.i_op = NCS_LIB_REQ_CREATE;

			/* do the registration */
			status = (*reg_unreg_routine) (&req_info);
			if (status != NCSCC_RC_SUCCESS) {
				/* log the error */
				sprintf(dbg_str, "ASCII spec registration failed for - %s", lib_name);
				m_LOG_DTS_DBGSTR_NAME(DTS_GLOBAL, dbg_str, 0, 0);
			} else {
				/* log the success, and the function name */
				/* Create patricia tree entry for ascii_spec lib name */
				lib_entry = m_MMGR_ALLOC_DTS_LIBNAME;
				if (lib_entry == NULL) {
					fclose(fp);
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dts_apps_ascii_spec_load: Memory allocation for patricia node failed");
				}
				memset(lib_entry, '\0', sizeof(ASCII_SPEC_LIB));
				strcpy((char *)lib_entry->lib_name, lib_name);
				lib_entry->libname_node.key_info = (uns8 *)lib_entry->lib_name;
				lib_entry->lib_hdl = lib_hdl;
				lib_entry->use_count++;
				/* Add node to patricia tree table */
				if (ncs_patricia_tree_add
				    (&dts_cb.libname_asciispec_tree,
				     (NCS_PATRICIA_NODE *)lib_entry) != NCSCC_RC_SUCCESS) {
					fclose(fp);
					m_MMGR_FREE_DTS_LIBNAME(lib_entry);
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dts_apps_ascii_spec_load: Failed to add node to paticia tree");
				}
			}	/*end of else */
		}
		/*end of if reg_unreg_routine != NULL */
		reg_unreg_routine = NULL;
		lib_hdl = NULL;
		memset(func_name, 0, DTS_MAX_FUNCNAME);
		memset(lib_name, 0, DTS_MAX_LIBNAME);

	}			/* for all the libraries  - end of while */

	if (nargs != EOF) {
		/* log the error */
		m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_apps_ascii_spec_load: Config file is corrupted.");

		/* set the return code */
		status = NCSCC_RC_FAILURE;
	}

	/* close the file */
	fclose(fp);
	return status;
}

/****************************************************************************
 * Name          : dts_cons_init                                            *
 *                                                                          *
 * Description   : Set console_dev to a working console.                    *
 *                                                                          *
 * Arguments     : None.                                                    *
 *                                                                          *
 * Return Values : None                                                     *
 *                                                                          *
 * Notes         : None.                                                    *
 ****************************************************************************/
void dts_cons_init(void)
{
	DTS_CB *inst = &dts_cb;
	int32 fd;
	uns32 tried_devcons = 0;
	uns32 tried_vtmaster = 0;
	char *s;

	if ((s = getenv("CONSOLE")) != NULL)
		inst->cons_dev = s;
	else {
		inst->cons_dev = DTS_CNSL;	/* defined as "/dev/console" */
		tried_devcons++;
	}

	while ((fd = open(inst->cons_dev, O_RDONLY | O_NONBLOCK)) < 0) {
		if (!tried_devcons) {
			tried_devcons++;
			inst->cons_dev = DTS_CNSL;
			continue;
		}
		if (!tried_vtmaster) {
			tried_vtmaster++;
			inst->cons_dev = DTS_VT_MASTER;	/* defined as "/dev/tty0" */
			continue;
		}
		break;
	}

	if (fd < 0)
		inst->cons_dev = "/dev/null";
	else
		close(fd);

	return;
}

/****************************************************************************\
 * Name          : dts_cons_open                                            *
 *                                                                          *
 * Description   : Open the console device.                                 *
 *                                                                          *
 * Arguments     : mode - input parameter specifies open for readonly/read  *
 *                 write etc.                                               *
 *                                                                          *
 * Return Values : file descriptor.                                         *
 *                                                                          *
 * Notes         : None.                                                    *
\****************************************************************************/
int32 dts_cons_open(uns32 mode)
{
	DTS_CB *inst = &dts_cb;
	int32 f, fd = -1;
	uns32 m;

	dts_cons_init();

	/*Open device in non blocking mode */
	m = mode | O_NONBLOCK;

	for (f = 0; f < 5; f++)
		if ((fd = open(inst->cons_dev, m)) >= 0)
			break;

	if (fd < 0)
		return fd;

	if (m != mode)
		fcntl(fd, F_SETFL, mode);

	return fd;
}

/*****************************************************************************\
 *  Name:          dts_app_signal_install                                     *
 *                                                                            *
 *  Description:   to install a singnal handler for a given signal            *
 *                                                                            *
 *  Arguments:     int i_sig_num - for which signal?                          *
 *                 SIG_HANDLR    - Handler to hanlde the signal               *
 *                                                                            *
 *  Returns:       0  - everything is OK                                      *
 *                 -1 - failure                                               *
 *  NOTE:                                                                     *
\*****************************************************************************/
/* install the signal handler */
int32 dts_app_signal_install(int i_sig_num, SIG_HANDLR i_sig_handler)
{
	struct sigaction sig_act;

	if ((i_sig_num <= 0) || (i_sig_num > 32) ||	/* not the real time signals */
	    (i_sig_handler == NULL))
		return -1;

	/* now say that, we are interested in this signal */
	memset(&sig_act, 0, sizeof(struct sigaction));
	sig_act.sa_handler = i_sig_handler;
	return sigaction(i_sig_num, &sig_act, NULL);

}
