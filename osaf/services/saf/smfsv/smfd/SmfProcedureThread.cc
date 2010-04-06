/*      - OpenSAF  -
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
 * Author(s): Ericsson AB
 *
 */

#include "SmfProcedureThread.hh"
#include "SmfUpgradeProcedure.hh"
#include "SmfUpgradeStep.hh"
#include "SmfCampaignThread.hh"
#include "SmfCampaign.hh"
#include "SmfUtils.hh"

#include <poll.h>

#include <ncssysf_def.h>
#include <ncssysf_ipc.h>
#include <ncssysf_tsk.h>
#include <logtrace.h>

#include <saImmOm.h>
#include <saImmOi.h>
#include <immutil.h>

/*====================================================================*/
/*  Data Declarations                                                 */
/*====================================================================*/

extern struct ImmutilWrapperProfile immutilWrapperProfile;

/*====================================================================*/
/*  Class SmfProcedureThread                                          */
/*====================================================================*/

/*====================================================================*/
/*  Static methods                                                    */
/*====================================================================*/

/** 
 * SmfProcedureThread::main
 * static main for the thread
 */
void
SmfProcedureThread::main(NCSCONTEXT info)
{
	SmfProcedureThread *self = (SmfProcedureThread *) info;
	self->main();
	TRACE("Procedure thread exits");
	delete self; 
}

/*====================================================================*/
/*  Methods                                                           */
/*====================================================================*/

/** 
 * Constructor
 */
SmfProcedureThread::SmfProcedureThread(SmfUpgradeProcedure * procedure):
	m_task_hdl(0),
	m_mbx(0),
	m_running(true), 
	m_procedure(procedure),
	m_procOiHandle(0),
	m_procSelectionObject(0)
{
	sem_init(&m_semaphore, 0, 0);
}

/** 
 * Destructor
 */
SmfProcedureThread::~SmfProcedureThread()
{
	TRACE_ENTER();

	//Delete the IMM handler
	deleteImmHandle();
	TRACE_LEAVE();
}

/**
 * SmfProcedureThread::start
 * Start the SmfProcedureThread.
 */
int
SmfProcedureThread::start(void)
{
	uns32 rc;

	TRACE("Starting procedure thread %s", m_procedure->getDn().c_str());

	/* Create the task */
	if ((rc =
	     m_NCS_TASK_CREATE((NCS_OS_CB) SmfProcedureThread::main, (NCSCONTEXT) this, (char*) m_PROCEDURE_TASKNAME,
			       m_PROCEDURE_TASK_PRI, m_PROCEDURE_STACKSIZE, &m_task_hdl)) != NCSCC_RC_SUCCESS) {
		LOG_ER("TASK_CREATE_FAILED");
		return -1;
	}

	if ((rc =m_NCS_TASK_DETACH(m_task_hdl)) != NCSCC_RC_SUCCESS) {
		LOG_ER("TASK_START_DETACH\n");
		return -1;
	}

	if ((rc = m_NCS_TASK_START(m_task_hdl)) != NCSCC_RC_SUCCESS) {
		LOG_ER("TASK_START_FAILED\n");
		return -1;
	}

	/* Wait for the thread to start */
	sem_wait(&m_semaphore);
	return 0;
}

/** 
 * SmfProcedureThread::stop
 * Stop the SmfProcedureThread.
 */
int 
SmfProcedureThread::stop(void)
{
	TRACE_ENTER();
	TRACE("Stopping procedure thread %s", m_procedure->getDn().c_str());

	/* send a message to the thread to make it terminate */
	PROCEDURE_EVT *evt = new PROCEDURE_EVT();
	evt->type = PROCEDURE_EVT_TERMINATE;
	this->send(evt);

	/* Wait for the thread to terminate */
	sem_wait(&m_semaphore);
	TRACE_LEAVE();
	return 0;
}

/** 
 * SmfProcedureThread::init
 * init the thread.
 */
int 
SmfProcedureThread::init(void)
{
	uns32 rc;
	SaAisErrorT result = SA_AIS_OK;

	/* Create the mailbox used for communication with this thread */
	if ((rc = m_NCS_IPC_CREATE(&m_mbx)) != NCSCC_RC_SUCCESS) {
		LOG_ER("m_NCS_IPC_CREATE FAILED %d", rc);
		return -1;
	}

	/* Attach mailbox to this thread */
	if ((rc = m_NCS_IPC_ATTACH(&m_mbx) != NCSCC_RC_SUCCESS)) {
		LOG_ER("m_NCS_IPC_ATTACH FAILED %d", rc);
		return -1;
	}

	/* Create Imm handle for our runtime objects */
	if ((rc = createImmHandle(m_procedure)) != NCSCC_RC_SUCCESS) {
		LOG_ER("createImmHandle FAILED %d", rc);
		return -1;
	}

	/* Check if our Imm runtime object already exists (switchover or restart occured) */
	result = getImmProcedure(m_procedure);
	if (result == SA_AIS_ERR_NOT_EXIST) {
		/* Create our Imm runtime object */
		if ((result = createImmProcedure(m_procedure)) != SA_AIS_OK) {
			LOG_ER("createImmProcedure FAILED %d", rc);
			return -1;
		}
	} else if (result == SA_AIS_OK) {
		/* Procedure exists, get step data */
		result = m_procedure->getImmSteps();
		if (result != SA_AIS_OK) {
			LOG_ER("getImmSteps FAILED %d", result);
			return -1;
		}
	} else {
		LOG_ER("getImmProcedure FAILED %d", result);
		return -1;
	}

	return 0;
}

/** 
 * SmfProcedureThread::send
 * send event to the thread.
 */
int 
SmfProcedureThread::send(PROCEDURE_EVT * evt)
{
	uns32 rc;

	TRACE("Procedure thread send event type %d", evt->type);
	rc = m_NCS_IPC_SEND(&m_mbx, (NCSCONTEXT) evt, NCS_IPC_PRIORITY_HIGH);
	return rc;
}

/** 
 * SmfProcedureThread::createImmHandle
 * Creates Imm handle for our runtime objects.
 */
SaAisErrorT 
SmfProcedureThread::createImmHandle(SmfUpgradeProcedure * procedure)
{
	SaAisErrorT rc = SA_AIS_OK;
	SaVersionT immVersion = { 'A', 2, 1 };
	const char *procDn = procedure->getDn().c_str();

	TRACE_ENTER();

	int errorsAreFatal = immutilWrapperProfile.errorsAreFatal;
	immutilWrapperProfile.errorsAreFatal = 0;

	rc = immutil_saImmOiInitialize_2(&m_procOiHandle, NULL, &immVersion);
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOiInitialize_2 fails rc=%d", rc);
		goto done;
	}

	rc = immutil_saImmOiSelectionObjectGet(m_procOiHandle, &m_procSelectionObject);
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOiSelectionObjectGet fails rc=%d", rc);
		goto done;
	}

	TRACE("saImmOiImplementerSet DN=%s", procDn);

	rc = immutil_saImmOiImplementerSet(m_procOiHandle, (char *)procDn);
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOiImplementerSet for DN=%s fails rc=%d", procDn, rc);
		goto done;
	}

	done:
	immutilWrapperProfile.errorsAreFatal = errorsAreFatal;
	TRACE_LEAVE();
	return rc;
}

/** 
 * SmfProcedureThread::deleteImmHandle
 * Deletes Imm handle for our runtime objects.
 */
SaAisErrorT 
SmfProcedureThread::deleteImmHandle()
{
	SaAisErrorT rc = SA_AIS_OK;

	TRACE_ENTER();

	rc = immutil_saImmOiImplementerClear(m_procOiHandle);
	if (rc != SA_AIS_OK) {
		if (rc == SA_AIS_ERR_BAD_HANDLE) {
			TRACE("saImmOiImplementerClear returns SA_AIS_ERR_BAD_HANDLE, probably because of switchover");
		} else {
			LOG_ER("SmfProcedureThread::deleteImmHandle:saImmOiImplementerClear fails rc=%d", rc);
		}
		goto done;
	}

	rc = immutil_saImmOiFinalize(m_procOiHandle);
	if (rc != SA_AIS_OK) {
		LOG_ER("SmfProcedureThread::deleteImmHandle:saImmOiFinalize fails rc=%d", rc);
	}

	done:
	TRACE_LEAVE();
	return rc;
}

/** 
 * SmfProcedureThread::createImmHandle
 * Creates Imm handle for our runtime objects.
 */
SaImmOiHandleT 
SmfProcedureThread::getImmHandle()
{
	return m_procOiHandle;
}

/** 
 * SmfProcedureThread::getImmProcedure
 * Get our Imm procedure objects (if exists).
 */
SaAisErrorT 
SmfProcedureThread::getImmProcedure(SmfUpgradeProcedure * procedure)
{
	SaAisErrorT rc = SA_AIS_OK;
	SmfImmUtils immutil;
	SaImmAttrValuesT_2 **attributes;
	std::list < std::string > stepList;

	TRACE_ENTER();

	TRACE("Get IMM data for %s", procedure->getDn().c_str());

	if (immutil.getObject(procedure->getDn(), &attributes) == false) {
		LOG_NO("IMM data for procedure %s not found", procedure->getDn().c_str());
		rc = SA_AIS_ERR_NOT_EXIST;
		goto done;
	}

	rc = procedure->init((const SaImmAttrValuesT_2 **)attributes);
	if (rc != SA_AIS_OK) {
		LOG_ER("Initialization failed for procedure %s", procedure->getDn().c_str());
		rc = SA_AIS_ERR_FAILED_OPERATION;
		goto done;
	}

 done:
	TRACE_LEAVE();
	return rc;
}

/** 
 * SmfProcedureThread::createImmProcedure
 * Create our Imm runtime object.
 */
SaAisErrorT 
SmfProcedureThread::createImmProcedure(SmfUpgradeProcedure * procedure)
{
	SaAisErrorT rc = SA_AIS_OK;
	SaNameT parentName;
	SmfCampaign *campaign = SmfCampaignThread::instance()->campaign();

	const char *safSmfProcedure = procedure->getProcName().c_str();
	uns32 saSmfProcExecLevel = procedure->getExecLevel();
	SaNameT saSmfProcMustKeepSIs = { 0 };
	SaNameT saSmfProcAcceptSIOutage = { 0 };
	uns32 saSmfProcMaxNumSIsOutage = 0;
	uns32 saSmfProcUpgrMethod = 0;
	SaTimeT saSmfProcPeriod = 0;
	uns32 saSmfProcState = SA_SMF_PROC_INITIAL;
	char *saSmfProcError = (char*)"";

	TRACE_ENTER();

	void *arr1[] = { &safSmfProcedure };
	const SaImmAttrValuesT_2 attr_safSmfProc = {
		(char*)"safSmfProcedure",
		SA_IMM_ATTR_SASTRINGT,
		1,
		arr1
	};

	void *arr2[] = { &saSmfProcExecLevel };
	const SaImmAttrValuesT_2 attr_saSmfProcExecLevel = {
		(char*)"saSmfProcExecLevel",
		SA_IMM_ATTR_SAUINT32T,
		1,
		arr2
	};

	void *arr3[] = { &saSmfProcMustKeepSIs };
	const SaImmAttrValuesT_2 attr_saSmfProcMustKeepSIs = {
		(char*)"saSmfProcMustKeepSIs",
		SA_IMM_ATTR_SANAMET,
		1,
		arr3
	};

	void *arr4[] = { &saSmfProcAcceptSIOutage };
	const SaImmAttrValuesT_2 attr_saSmfProcAcceptSIOutage = {
		(char*)"saSmfProcAcceptSIOutage",
		SA_IMM_ATTR_SANAMET,
		1,
		arr4
	};

	void *arr5[] = { &saSmfProcMaxNumSIsOutage };
	const SaImmAttrValuesT_2 attr_saSmfProcMaxNumSIsOutage = {
		(char*)"saSmfProcMaxNumSIsOutage",
		SA_IMM_ATTR_SAUINT32T,
		1,
		arr5
	};

	void *arr6[] = { &saSmfProcUpgrMethod };
	const SaImmAttrValuesT_2 attr_saSmfProcUpgrMethod = {
		(char*)"saSmfProcUpgrMethod",
		SA_IMM_ATTR_SAUINT32T,
		1,
		arr6
	};

	void *arr7[] = { &saSmfProcPeriod };
	const SaImmAttrValuesT_2 attr_saSmfProcPeriod = {
		(char*)"saSmfProcPeriod",
		SA_IMM_ATTR_SATIMET,
		1,
		arr7
	};

	void *arr8[] = { &saSmfProcState };
	const SaImmAttrValuesT_2 attr_saSmfProcState = {
		(char*)"saSmfProcState",
		SA_IMM_ATTR_SAUINT32T,
		1,
		arr8
	};

	void *arr9[] = { &saSmfProcError };
	const SaImmAttrValuesT_2 attr_saSmfProcError = {
		(char*)"saSmfProcError",
		SA_IMM_ATTR_SASTRINGT,
		1,
		arr9
	};

	const SaImmAttrValuesT_2 *attrValues[] = {
		&attr_safSmfProc,
		&attr_saSmfProcExecLevel,
		&attr_saSmfProcMustKeepSIs,
		&attr_saSmfProcAcceptSIOutage,
		&attr_saSmfProcMaxNumSIsOutage,
		&attr_saSmfProcUpgrMethod,
		&attr_saSmfProcPeriod,
		&attr_saSmfProcState,
		&attr_saSmfProcError,
		NULL
	};

	parentName.length = campaign->getDn().length();
	strncpy((char *)parentName.value, campaign->getDn().c_str(), parentName.length);
	parentName.value[parentName.length] = 0;

	int errorsAreFatal = immutilWrapperProfile.errorsAreFatal;
	immutilWrapperProfile.errorsAreFatal = 0;

	rc = immutil_saImmOiRtObjectCreate_2(m_procOiHandle, (char*)"SaSmfProcedure", &parentName, attrValues);
	immutilWrapperProfile.errorsAreFatal = errorsAreFatal;

	if (rc != SA_AIS_OK) {
		TRACE("saImmOiRtObjectCreate_2 returned %u for %s, parent %s", rc, procedure->getProcName().c_str(),
		      parentName.value);
		goto done;
	}

 done:
	TRACE_LEAVE();
	return rc;
}

/**
 * SmfProcedureThread::updateImmAttr
 * Updates a runtime attribute in the IMM
 */
int 
SmfProcedureThread::updateImmAttr(const char *dn, SaImmAttrNameT attributeName, SaImmValueTypeT attrValueType,
				      void *value)
{
	SaAisErrorT rc = SA_AIS_OK;

	TRACE_ENTER();
	rc = immutil_update_one_rattr(m_procOiHandle, dn, attributeName, attrValueType, value);

	if (rc != SA_AIS_OK) {
		LOG_ER("update attribute failed %d, dn %s, attr %s", rc, dn, attributeName);
		TRACE_LEAVE();
		return NCSCC_RC_FAILURE;
	}

	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

/** 
 * SmfProcedureThread::processEvt
 * process events in the mailbox.
 */
void 
SmfProcedureThread::processEvt(void)
{
	PROCEDURE_EVT *evt;

	evt = (PROCEDURE_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(&m_mbx, evt);
	if (evt != NULL) {
		TRACE("Procedure thread received event type %d", evt->type);

		switch (evt->type) {
		case PROCEDURE_EVT_TERMINATE:
			{
				/* */
				m_running = false;
				break;
			}

		case PROCEDURE_EVT_EXECUTE:
			{
				TRACE("Executing procedure %s", m_procedure->getProcName().c_str());

				m_procedure->execute();

				break;
			}

		case PROCEDURE_EVT_EXECUTE_INIT:
			{
				TRACE("Executing init actions %s", m_procedure->getProcName().c_str());
				m_procedure->executeInit();
				break;
			}

		case PROCEDURE_EVT_EXECUTE_STEP:
			{
				TRACE("Executing steps %s", m_procedure->getProcName().c_str());
				m_procedure->executeStep();
				break;
			}

		case PROCEDURE_EVT_EXECUTE_WRAPUP:
			{
				TRACE("Executing wrapup actions %s", m_procedure->getProcName().c_str());
				m_procedure->executeWrapup();
				break;
			}

		case PROCEDURE_EVT_SUSPEND:
			{
				m_procedure->suspend();
				break;
			}

		case PROCEDURE_EVT_COMMIT:
			{
				m_procedure->commit();
				break;
			}

		case PROCEDURE_EVT_ROLLBACK:
			{
				m_procedure->rollback();
				break;
			}
		default:
			{
				LOG_ER("unknown event received %d", evt->type);
			}
		}

		delete(evt);
	}
}

typedef enum {
	PROC_MBX_FD,
	PROC_OI_FD,
	PROC_MAX_FD
} proc_pollfd_t;

/** 
 * SmfProcedureThread::handleEvents
 * handle incoming events to the thread.
 */
int 
SmfProcedureThread::handleEvents(void)
{
	NCS_SEL_OBJ mbx_fd;
	struct pollfd fds[PROC_MAX_FD];

	mbx_fd = ncs_ipc_get_sel_obj(&m_mbx);

	/* Set up all file descriptors to listen to */
	fds[PROC_MBX_FD].fd = mbx_fd.rmv_obj;
	fds[PROC_MBX_FD].events = POLLIN;
	fds[PROC_OI_FD].fd = m_procSelectionObject;
	fds[PROC_OI_FD].events = POLLIN;

	TRACE("Procedure thread %s waiting for events", m_procedure->getDn().c_str());

	while (m_running) {
		int ret = poll(fds, PROC_MAX_FD, -1);

		if (ret == -1) {
			if (errno == EINTR)
				continue;

			LOG_ER("poll failed - %s", strerror(errno));
			break;
		}

		/* Process the Mail box events */
		if (fds[PROC_MBX_FD].revents & POLLIN) {
			/* dispatch MBX events */
			processEvt();
		}

		/* Process the Imm callback events */
		if (fds[PROC_OI_FD].revents & POLLIN) {
			SaAisErrorT rc = SA_AIS_OK;

			if ((rc = saImmOiDispatch(m_procOiHandle, SA_DISPATCH_ALL)) != SA_AIS_OK) {
				LOG_ER("saImmOiDispatch FAILED: %u", rc);
			}
		}
	}

	return 0;
}

/** 
 * SmfProcedureThread::main
 * main for the thread.
 */
void 
SmfProcedureThread::main(void)
{
	if (this->init() == 0) {
		/* Mark the thread started */
		sem_post(&m_semaphore);

		this->handleEvents();	/* runs forever until stopped */

		/* Mark the thread terminated */
		sem_post(&m_semaphore);
	} else {
		LOG_ER("init failed");
	}
}
