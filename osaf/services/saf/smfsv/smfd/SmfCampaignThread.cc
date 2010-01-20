/*       OpenSAF
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

#include <poll.h>

#include <ncssysf_def.h>
#include <ncssysf_ipc.h>
#include <ncssysf_tsk.h>
#include <logtrace.h>

#include "SmfCampaignThread.hh"
#include "SmfCampaign.hh"
#include "SmfUpgradeCampaign.hh"
#include "SmfUpgradeProcedure.hh"

SmfCampaignThread *SmfCampaignThread::s_instance = NULL;

/*====================================================================*/
/*  Class SmfCampaignThread                                           */
/*====================================================================*/

/*====================================================================*/
/*  Static methods                                                    */
/*====================================================================*/

/** 
 * SmfCampaignThread::instance
 * returns the only instance of CampaignThread.
 */
SmfCampaignThread *SmfCampaignThread::instance(void)
{
	return s_instance;
}

/** 
 * SmfCampaignThread::terminate
 * terminates (if started) the only instance of CampaignThread.
 */
void SmfCampaignThread::terminate(void)
{
	if (s_instance != NULL) {
		s_instance->stop();
		/* main will delete the CampaignThread object when terminating */
	}

	return;
}

/** 
 * SmfCampaignThread::start
 * Create the object and start the thread.
 */
int SmfCampaignThread::start(SmfCampaign * campaign)
{
	if (s_instance != NULL) {
		LOG_ER("Campaign thread already exists");
		return -1;
	}

	s_instance = new SmfCampaignThread(campaign);

	if (s_instance->start() != 0) {
		delete s_instance;
		s_instance = NULL;
		return -1;
	}
	return 0;
}

/** 
 * SmfCampaignThread::main
 * static main for the thread
 */
void SmfCampaignThread::main(NCSCONTEXT info)
{
	SmfCampaignThread *self = (SmfCampaignThread *) info;
	self->main();
	TRACE("Campaign thread exits");
	delete self;
	s_instance = NULL;
}

/*====================================================================*/
/*  Methods                                                           */
/*====================================================================*/

/** 
 * Constructor
 */
 SmfCampaignThread::SmfCampaignThread(SmfCampaign * campaign):
m_running(true), m_campaign(campaign)
{
	sem_init(&m_semaphore, 0, 0);
}

/** 
 * Destructor
 */
SmfCampaignThread::~SmfCampaignThread()
{
	SaAisErrorT rc = saNtfFinalize(m_ntfHandle);
	if (rc != SA_AIS_OK) {
		LOG_ER("Failed to finalize NTF handle %u", rc);
	}

	SmfUpgradeCampaign *upgradeCampaign = m_campaign->getUpgradeCampaign();
	if (upgradeCampaign != NULL) {
		m_campaign->setUpgradeCampaign(NULL);
		delete upgradeCampaign;
	}
}

/** 
 * SmfCampaignThread::start
 * Start the CampaignThread.
 */
int
 SmfCampaignThread::start(void)
{
	uns32 rc;

	TRACE("Starting campaign thread %s", m_campaign->getDn().c_str());

	/* Create the task */
	if ((rc =
	     m_NCS_TASK_CREATE((NCS_OS_CB) SmfCampaignThread::main, (NCSCONTEXT) this, m_CAMPAIGN_TASKNAME,
			       m_CAMPAIGN_TASK_PRI, m_CAMPAIGN_STACKSIZE, &m_task_hdl)) != NCSCC_RC_SUCCESS) {
		LOG_ER("TASK_CREATE_FAILED");
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
 * SmfCampaignThread::stop
 * Stop the SmfCampaignThread.
 */
int SmfCampaignThread::stop(void)
{
	TRACE("Stopping campaign thread %s", m_campaign->getDn().c_str());

	/* send a message to the thread to make it terminate */
	CAMPAIGN_EVT *evt = new CAMPAIGN_EVT();
	evt->type = CAMPAIGN_EVT_TERMINATE;
	this->send(evt);

	/* Wait for the thread to terminate */
	sem_wait(&m_semaphore);
	return 0;
}

/** 
 * SmfCampaignThread::campaign
 * Returns the Campaign object.
 */
SmfCampaign *SmfCampaignThread::campaign(void)
{
	return m_campaign;
}

/** 
 * SmfCampaignThread::init
 * init the thread.
 */
int SmfCampaignThread::init(void)
{
	uns32 rc;

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

	return 0;
}

/** 
 * SmfCampaignThread::initNtf
 * init the NTF state notification.
 */
int SmfCampaignThread::initNtf(void)
{
	SaAisErrorT rc = SA_AIS_ERR_TRY_AGAIN;
	SaVersionT ntfVersion = { 'A', 1, 1 };
	unsigned int numOfTries = 50;

	while (rc == SA_AIS_ERR_TRY_AGAIN && numOfTries > 0) {
		rc = saNtfInitialize(&m_ntfHandle, NULL, &ntfVersion);
		if (rc != SA_AIS_ERR_TRY_AGAIN) {
			break;
		}
		usleep(200000);
		numOfTries--;
	}

	if (rc != SA_AIS_OK) {
		LOG_ER("saNtfInitialize FAILED %d", rc);
		return -1;
	}

	return 0;
}

/** 
 * SmfCampaignThread::send
 * send event to the thread.
 */
int SmfCampaignThread::send(CAMPAIGN_EVT * evt)
{
	uns32 rc;

	TRACE("Campaign thread send event type %d", evt->type);
	rc = m_NCS_IPC_SEND(&m_mbx, (NCSCONTEXT) evt, NCS_IPC_PRIORITY_HIGH);

	return rc;
}

/** 
 * SmfCampaignThread::sendStateNotification
 * send state change notification to NTF.
 */
int SmfCampaignThread::sendStateNotification(const std::string & dn, uns32 classId, SaNtfSourceIndicatorT sourceInd,
					     uns32 stateId, uns32 newState)
{
	SaAisErrorT rc;
	int result = 0;
	SaNtfStateChangeNotificationT ntfStateNot;

	rc = saNtfStateChangeNotificationAllocate(m_ntfHandle, &ntfStateNot, 1,	/* numCorrelated Notifications */
						  0,	/* length addition text */
						  0,	/* num additional info */
						  1,	/* num of state changes */
						  0);	/* variable data size */
	if (rc != SA_AIS_OK) {
		LOG_ER("saNtfStateChangeNotificationAllocate FAILED %d", rc);
		return -1;
	}

	/* Notifying object */
	ntfStateNot.notificationHeader.notifyingObject->length = sizeof(SMF_NOTIFYING_OBJECT) - 1;
	strcpy((char *)ntfStateNot.notificationHeader.notifyingObject->value, SMF_NOTIFYING_OBJECT);

	/* Notification object */
	ntfStateNot.notificationHeader.notificationObject->length = dn.length();
	strcpy((char *)ntfStateNot.notificationHeader.notificationObject->value, dn.c_str());

	/* Event type */
	*(ntfStateNot.notificationHeader.eventType) = SA_NTF_OBJECT_STATE_CHANGE;

	/* event time to be set automatically to current
	   time by saNtfNotificationSend */
	*(ntfStateNot.notificationHeader.eventTime) = (SaTimeT) SA_TIME_UNKNOWN;

	/* set Notification Class Identifier */
	ntfStateNot.notificationHeader.notificationClassId->vendorId = VENDOR_ID;
	ntfStateNot.notificationHeader.notificationClassId->majorId = SA_SVC_SMF;
	ntfStateNot.notificationHeader.notificationClassId->minorId = classId;

	/* Set source indicator */
	*(ntfStateNot.sourceIndicator) = sourceInd;

	/* Set state changed */
	ntfStateNot.changedStates[0].stateId = stateId;
	ntfStateNot.changedStates[0].newState = newState;

	/* Send the notification */
	rc = saNtfNotificationSend(ntfStateNot.notificationHandle);
	if (rc != SA_AIS_OK) {
		LOG_ER("saNtfNotificationSend FAILED %d", rc);
		result = -1;
		goto done;
	}

 done:
	rc = saNtfNotificationFree(ntfStateNot.notificationHandle);
	if (rc != SA_AIS_OK) {
		LOG_ER("saNtfNotificationFree FAILED %d", rc);
	}
	return result;
}

/** 
 * SmfCampaignThread::processEvt
 * process events in the mailbox.
 */
void SmfCampaignThread::processEvt(void)
{
	CAMPAIGN_EVT *evt;

	evt = (CAMPAIGN_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(&m_mbx, evt);
	if (evt != NULL) {
		TRACE("Campaign thread received event type %d", evt->type);

		switch (evt->type) {
		case CAMPAIGN_EVT_TERMINATE:
			{
				/* */
				m_running = false;
				break;
			}

		case CAMPAIGN_EVT_EXECUTE:
			{
				m_campaign->adminOpExecute();
				break;
			}

		case CAMPAIGN_EVT_SUSPEND:
			{
				m_campaign->adminOpSuspend();
				break;
			}

		case CAMPAIGN_EVT_COMMIT:
			{
				m_campaign->adminOpCommit();
				break;
			}

		case CAMPAIGN_EVT_ROLLBACK:
			{
				m_campaign->adminOpRollback();
				break;
			}

		case CAMPAIGN_EVT_EXECUTE_INIT:
			{
				m_campaign->getUpgradeCampaign()->executeInit();
				break;
			}

		case CAMPAIGN_EVT_EXECUTE_PROC:
			{
				m_campaign->getUpgradeCampaign()->executeProc();
				break;
			}

		case CAMPAIGN_EVT_EXECUTE_WRAPUP:
			{
				m_campaign->getUpgradeCampaign()->executeWrapup();
				break;
			}

		case CAMPAIGN_EVT_PROCEDURE_RC:
			{
				TRACE("Procedure result from %s = %u",
				      evt->event.procResult.procedure->getProcName().c_str(), evt->event.procResult.rc);
                                /* TODO We need to send the procedure result to the state machine
                                   so we can see what response this is. This is needed for suspend 
                                   where we can receive an execute response in the middle of the suspend
                                   responses if a procedure just finished when we sent the suspend to it. */
				m_campaign->getUpgradeCampaign()->executeProc();
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

/** 
 * SmfCampaignThread::handleEvents
 * handle incoming events to the thread.
 */
int SmfCampaignThread::handleEvents(void)
{
	NCS_SEL_OBJ mbx_fd;
	struct pollfd fds[1];

	mbx_fd = ncs_ipc_get_sel_obj(&m_mbx);

	/* Set up all file descriptors to listen to */
	fds[0].fd = mbx_fd.rmv_obj;
	fds[0].events = POLLIN;

	TRACE("Campaign thread %s waiting for events", m_campaign->getDn().c_str());

	while (m_running) {
		int ret = poll(fds, 1, -1);

		if (ret == -1) {
			if (errno == EINTR)
				continue;

			LOG_ER("poll failed - %s", strerror(errno));
			break;
		}

		/* Process the Mail box events */
		if (fds[0].revents & POLLIN) {
			/* dispatch MBX events */
			processEvt();
		}
	}

	return 0;
}

/** 
 * SmfCampaignThread::main
 * main for the thread.
 */
void SmfCampaignThread::main(void)
{
	if (this->init() == 0) {
		/* Mark the thread started */
		sem_post(&m_semaphore);

		if (initNtf() != 0) {
			LOG_ER("initNtf failed");
		}

		this->handleEvents();	/* runs forever until stopped */

		/* Mark the thread terminated */
		sem_post(&m_semaphore);
	} else {
		LOG_ER("init failed");
	}
}
