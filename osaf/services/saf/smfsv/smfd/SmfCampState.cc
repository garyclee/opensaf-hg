/*
 *
 * (C) Copyright 2009 The OpenSAF Foundation
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

/* ========================================================================
 *   INCLUDE FILES
 * ========================================================================
 */

#include "logtrace.h"
#include "smfd.h"
#include "SmfUpgradeCampaign.hh"
#include "SmfCampState.hh"
#include "SmfProcState.hh"
#include "SmfUpgradeProcedure.hh"
#include "SmfCampaignThread.hh"
#include "SmfCampaign.hh"
#include "SmfProcedureThread.hh"
#include <immutil.h>
#include <sstream>

/* ========================================================================
 *   DEFINITIONS
 * ========================================================================
 */

/* ========================================================================
 *   TYPE DEFINITIONS
 * ========================================================================
 */

/* ========================================================================
 *   DATA DECLARATIONS
 * ========================================================================
 */

/* ========================================================================
 *   FUNCTION PROTOTYPES
 * ========================================================================
 */

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// ------Base class SmfCampState------------------------------------------------
//
// SmfCampState default implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
void 
SmfCampState::getClassName(std::string & io_str) const
{
	io_str = "SmfCampState";
}

//------------------------------------------------------------------------------
// toString()
//------------------------------------------------------------------------------
void 
SmfCampState::toString(std::string & io_str) const 
{
	getClassName(io_str);
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
void 
SmfCampState::execute(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	LOG_ER("SmfCampState::execute default implementation, should NEVER be executed.");
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeInit()
//------------------------------------------------------------------------------
void 
SmfCampState::executeInit(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	LOG_ER("SmfCampState::executeInit default implementation, should NEVER be executed.");
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeProc()
//------------------------------------------------------------------------------
void 
SmfCampState::executeProc(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	LOG_ER("SmfCampState::executeProc default implementation, should NEVER be executed.");
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeWrapup()
//------------------------------------------------------------------------------
void 
SmfCampState::executeWrapup(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	LOG_ER("SmfCampState::executeWrapup default implementation, should NEVER be executed.");
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// rollback()
//------------------------------------------------------------------------------
void 
SmfCampState::rollback(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	LOG_ER("SmfCampState::rollback default implementation, should NEVER be executed.");
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// suspend()
//------------------------------------------------------------------------------
void 
SmfCampState::suspend(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	LOG_ER("SmfCampState::suspend default implementation, should NEVER be executed.");
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// commit()
//------------------------------------------------------------------------------
void 
SmfCampState::commit(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	LOG_ER("SmfCampState::commit default implementation, should NEVER be executed.");
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// changeState()
//------------------------------------------------------------------------------
void 
SmfCampState::changeState(SmfUpgradeCampaign * i_camp, SmfCampState * i_state)
{
	TRACE_ENTER();
	TRACE("SmfCampState::changeState");

	std::string newState;
	i_state->getClassName(newState);

	std::string oldState;
	i_camp->m_state->getClassName(oldState);

	TRACE("SmfCampState::changeState old state=%s , new state=%s", oldState.c_str(), newState.c_str());
	i_camp->changeState(i_state);

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfCampStateInitial implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

SmfCampState *SmfCampStateInitial::s_instance = NULL;

SmfCampState *SmfCampStateInitial::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfCampStateInitial;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
void 
SmfCampStateInitial::getClassName(std::string & io_str) const {
	io_str = "SmfCampStateInitial";
}

//------------------------------------------------------------------------------
// toString()
//------------------------------------------------------------------------------
void 
SmfCampStateInitial::toString(std::string & io_str) const {
	getClassName(io_str);
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
void 
SmfCampStateInitial::execute(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	std::string error;
	std::string s;
	std::stringstream out;

	TRACE("SmfCampStateInitial::execute implementation");

	LOG_NO("CAMP: Start upgrade campaign %s", i_camp->getCampaignName().c_str());

	//Reset error string in case the previous prerequsites check failed
        //In such case an error string is present in initial state
	SmfCampaignThread::instance()->campaign()->setError("");

	std::vector < SmfUpgradeProcedure * >::iterator iter;

	//Set the DN of the procedures
	iter = i_camp->m_procedure.begin();
	while (iter != i_camp->m_procedure.end()) {
		//Set the DN of the procedure
		std::string dn = (*iter)->getProcName() + "," + SmfCampaignThread::instance()->campaign()->getDn();
		(*iter)->setDn(dn);

		iter++;
	}

	LOG_NO("CAMP: Check repository %s", i_camp->getCampaignName().c_str());
	if (smfd_cb->repositoryCheckCmd != NULL) {
		TRACE("Executing repository check cmd %s", smfd_cb->repositoryCheckCmd);
		int rc = system(smfd_cb->repositoryCheckCmd);
		if (rc != 0) {
			error = "CAMP: Repository check command ";
			error += smfd_cb->repositoryCheckCmd;
			error += " failed ";
			out << rc;
			s = out.str();
			error += s;
			goto exit_error;
		}
	} else {
		error = "CAMP: No Repository check command found";
		goto exit_error;
	}

	LOG_NO("CAMP: Create system backup %s", i_camp->getCampaignName().c_str());
	if (smfd_cb->backupCreateCmd != NULL) {
		std::string backupCmd = smfd_cb->backupCreateCmd;
		backupCmd += " ";
		backupCmd += i_camp->getCampaignName();

		TRACE("Executing backup create cmd %s", backupCmd.c_str());
		int rc = system(backupCmd.c_str());
		if (rc != 0) {
			error = "CAMP: Backup create command ";
			error += smfd_cb->backupCreateCmd;
			error += " failed ";
			out << rc;
			s = out.str();
			error += s;
			goto exit_error;
		}
	} else {
		error = "CAMP: No backup create command  found";
		goto exit_error;
	}

	LOG_NO("CAMP: Start executing campaign %s", i_camp->getCampaignName().c_str());

	//Preparation is ready, change state and continue the campaign execution
	changeState(i_camp, SmfCampStateExecuting::instance());
	i_camp->execute();

	TRACE_LEAVE();
	return;

exit_error:
	LOG_ER(error.c_str());
	SmfCampaignThread::instance()->campaign()->setError(error);
	//Remain in state initial if prerequsites check or SMF backup fails
//	changeState(i_camp, SmfCampStateExecFailed::instance());
	TRACE_LEAVE();
	return;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfCampStateExecuting implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SmfCampState *SmfCampStateExecuting::s_instance = NULL;

SmfCampState *SmfCampStateExecuting::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfCampStateExecuting;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
void 
SmfCampStateExecuting::getClassName(std::string & io_str) const 
{
	io_str = "SmfCampStateExecuting";
}

//------------------------------------------------------------------------------
// toString()
//------------------------------------------------------------------------------
void 
SmfCampStateExecuting::toString(std::string & io_str) const 
{
	getClassName(io_str);
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
void 
SmfCampStateExecuting::execute(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();

	TRACE("SmfCampStateExecuting::execute, Do some checking");

	//Start procedure threads
	TRACE("SmfCampStateExecuting::execute, start procedure threads and set initial state");
	std::vector < SmfUpgradeProcedure * >::iterator iter;
	iter = i_camp->m_procedure.begin();
	while (iter != i_camp->m_procedure.end()) {
		SmfProcedureThread *procThread = new SmfProcedureThread(*iter);
		(*iter)->setProcThread(procThread);

		TRACE("SmfCampStateExecuting::executeProc, Starting procedure %s", (*iter)->getProcName().c_str());
		procThread->start();

		iter++;
	}

	//If a running campaign was restarted on an other node, the procedures in executing state
	//must be restarted.
	bool execProcFound = false;
	iter = i_camp->m_procedure.begin();
	while (iter != i_camp->m_procedure.end()) {
		if ((*iter)->getState() == SA_SMF_PROC_EXECUTING) {
			TRACE
			    ("SmfCampStateExecuting::execute, restarted procedure found, send CAMPAIGN_EVT_EXECUTE event to thread");
			SmfProcedureThread *procThread = (*iter)->getProcThread();
			PROCEDURE_EVT *evt = new PROCEDURE_EVT();
			evt->type = PROCEDURE_EVT_EXECUTE;
			procThread->send(evt);
			execProcFound = true;
		}

		iter++;
	}

	if (execProcFound == true) {
		//Execution will continue at SmfCampStateExecuting::executeProc() when first proc result is received.
		TRACE("SmfCampStateExecuting::execute, Wait for restarted procedures to respond.");
		TRACE_LEAVE();
		return;
	}

	/* No executing procedures, this must be a campaign start so execute campaign inititialization */
        this->executeInit(i_camp);

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeInit()
//------------------------------------------------------------------------------
void 
SmfCampStateExecuting::executeInit(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	TRACE("SmfCampStateExecuting::executeInit, Running campaign init actions");

	if (i_camp->m_campInit.execute() == false) {
		std::string error = "Campaign init failed";
		LOG_ER(error.c_str());
		SmfCampaignThread::instance()->campaign()->setError(error);
		changeState(i_camp, SmfCampStateExecFailed::instance());
		return;
	}

	TRACE("SmfCampStateExecuting::executeInit, campaign init actions completed");
        this->executeProc(i_camp);

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeProc()
//------------------------------------------------------------------------------
void 
SmfCampStateExecuting::executeProc(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	TRACE("SmfCampStateExecuting::executeProc, Starting procedure threads");

	//Find out if some procedure is still executing
	std::vector < SmfUpgradeProcedure * >::iterator iter;

	iter = i_camp->m_procedure.begin();
	while (iter != i_camp->m_procedure.end()) {
		switch ((*iter)->getState()) {
			/* TODO This does not work in case we have parallel execution of procedures */
		case SA_SMF_PROC_EXECUTING:
			{
				TRACE("SmfCampStateExecuting::Procedure %s still executing",
				      (*iter)->getProcName().c_str());
				TRACE_LEAVE();
				return;
			}
		case SA_SMF_PROC_FAILED:
			{
				TRACE("SmfCampStateExecuting::Procedure %s failed", (*iter)->getProcName().c_str());

				std::string error = "Procedure " + (*iter)->getProcName() + " failed";
				LOG_ER(error.c_str());
				SmfCampaignThread::instance()->campaign()->setError(error);
				changeState(i_camp, SmfCampStateExecFailed::instance());
				TRACE_LEAVE();
				return;
			}
		default:
			break;
		}
		iter++;
	}

	TRACE
	    ("SmfCampStateExecuting::No procedures in executing state was found, proceed with next execution level procedures");

	//The procedure vector is sorted in execution level order (low -> high)
	//Lowest number shall be executed first.

	int execLevel = -1;
	iter = i_camp->m_procedure.begin();
	while (iter != i_camp->m_procedure.end()) {
		//If the state is initial and the execution level is new or the same as 
		//the procedure just started.
		TRACE("SmfCampStateExecuting::executeProc, procedure %s, state=%u, execLevel=%d",
		      (*iter)->getProcName().c_str(), (*iter)->getState(), (*iter)->getExecLevel());

		if (execLevel != -1) {
			TRACE("SmfCampStateExecuting::executeProc, a procedure already run at execLevel=%d, start parallel procedure", 
			      execLevel);
		}

		if (((*iter)->getState() == SA_SMF_PROC_INITIAL)
		    && ((execLevel == -1)
			|| (execLevel == (*iter)->getExecLevel()))) {
			SmfProcedureThread *procThread = (*iter)->getProcThread();
			PROCEDURE_EVT *evt = new PROCEDURE_EVT();
			evt->type = PROCEDURE_EVT_EXECUTE;
			procThread->send(evt);
			execLevel = (*iter)->getExecLevel();
		}

		iter++;
	}

	if (execLevel != -1) {
		TRACE("SmfCampStateExecuting::executeProc, Wait for procedure results");
		TRACE_LEAVE();
		return;
	}

	TRACE("SmfCampStateExecuting::executeProc, All procedures executed, start wrapup");
        this->executeWrapup(i_camp);

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeWrapup()
//------------------------------------------------------------------------------
void 
SmfCampStateExecuting::executeWrapup(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	LOG_NO("CAMP: Running campaign wrapup complete actions");

	if (i_camp->m_campWrapup.executeCampComplete() == false) {
		std::string error = "Campaign wrapup.executeCampComplete failed";
		LOG_ER(error.c_str());
		SmfCampaignThread::instance()->campaign()->setError(error);
		changeState(i_camp, SmfCampStateExecFailed::instance());
		return;
	}
	// TODO Start wait to complete timer
	LOG_NO("CAMP: Start wait to complete timer (not implemented yet)");

	/* TODO State completed should be set when waitToComplete times out */
	changeState(i_camp, SmfCampStateExecCompleted::instance());

	LOG_NO("CAMP: Upgrade campaign completed %s", i_camp->getCampaignName().c_str());
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// suspend()
//------------------------------------------------------------------------------
void 
SmfCampStateExecuting::suspend(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	TRACE("SmfCampStateExecuting::suspend implementation");

	/* Send suspend message to all procedures */
	std::vector < SmfUpgradeProcedure * >::iterator iter;

	for (iter = i_camp->m_procedure.begin(); iter != i_camp->m_procedure.end(); ++iter) {
                TRACE("SmfCampStateExecuting::Procedure %s executing, send suspend",
                      (*iter)->getProcName().c_str());
                SmfProcedureThread *procThread = (*iter)->getProcThread();
                PROCEDURE_EVT *evt = new PROCEDURE_EVT();
                evt->type = PROCEDURE_EVT_SUSPEND;
                procThread->send(evt);
	}

        changeState(i_camp, SmfCampStateSuspendingExec::instance());
        /* Wait for responses from all procedures (SmfCampStateSuspendingExec::execProc) */

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfCampStateExecutionCompleted implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SmfCampState *SmfCampStateExecCompleted::s_instance = NULL;

SmfCampState *SmfCampStateExecCompleted::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfCampStateExecCompleted;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
void 
SmfCampStateExecCompleted::getClassName(std::string & io_str) const 
{
	io_str = "SmfCampStateExecutionCompleted";
}

//------------------------------------------------------------------------------
// toString()
//------------------------------------------------------------------------------
void SmfCampStateExecCompleted::toString(std::string & io_str) const 
{
	getClassName(io_str);
}

//------------------------------------------------------------------------------
// commit()
//------------------------------------------------------------------------------
void 
SmfCampStateExecCompleted::commit(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();

	i_camp->m_campWrapup.executeCampWrapup(); // No action if wrapup is faulty

	changeState(i_camp, SmfCampStateCommitted::instance());

	LOG_NO("CAMP: Upgrade campaign committed %s", i_camp->getCampaignName().c_str());

	// TODO Start wait to allow new campaign timer
	LOG_NO("CAMP: Start wait to allow new campaign timer (not implemented yet)");

	/* Commit all procedures to remove all runtime objects */
	std::vector < SmfUpgradeProcedure * >::iterator iter;

	for (iter = i_camp->m_procedure.begin(); iter != i_camp->m_procedure.end(); ++iter) {
		SmfProcedureThread *procThread = (*iter)->getProcThread();
		PROCEDURE_EVT *evt = new PROCEDURE_EVT();
		evt->type = PROCEDURE_EVT_COMMIT;
		procThread->send(evt);
	}

	/* Terminate campaign thread */
	CAMPAIGN_EVT *evt = new CAMPAIGN_EVT();
	evt->type = CAMPAIGN_EVT_TERMINATE;
	SmfCampaignThread::instance()->send(evt);

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfCampStateSuspendingExec implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SmfCampState *SmfCampStateSuspendingExec::s_instance = NULL;

SmfCampState *SmfCampStateSuspendingExec::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfCampStateSuspendingExec;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// SmfCampStateSuspendingExec()
//------------------------------------------------------------------------------
SmfCampStateSuspendingExec::SmfCampStateSuspendingExec(void):
        m_numOfProcResponses(0)
{
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
void 
SmfCampStateSuspendingExec::getClassName(std::string & io_str) const 
{
	io_str = "SmfCampStateSuspendingExec";
}

//------------------------------------------------------------------------------
// toString()
//------------------------------------------------------------------------------
void 
SmfCampStateSuspendingExec::toString(std::string & io_str) const 
{
	getClassName(io_str);
}

//------------------------------------------------------------------------------
// suspend()
//------------------------------------------------------------------------------
void 
SmfCampStateSuspendingExec::executeProc(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	TRACE("SmfCampStateSuspendingExec::executeProc implementation");

        /* If first response, set number of expected responses */
        if (m_numOfProcResponses == 0) {
                m_numOfProcResponses = i_camp->m_procedure.size();
        }

        /* Decrease the response counter */
        m_numOfProcResponses--;

        /* If last response, change state to suspended */
        if (m_numOfProcResponses == 0) {
		changeState(i_camp, SmfCampStateExecSuspended::instance());
	}

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfCampStateExecSuspended implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SmfCampState *SmfCampStateExecSuspended::s_instance = NULL;

SmfCampState *SmfCampStateExecSuspended::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfCampStateExecSuspended;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
void 
SmfCampStateExecSuspended::getClassName(std::string & io_str) const 
{
	io_str = "SmfCampStateExecSuspended";
}

//------------------------------------------------------------------------------
// toString()
//------------------------------------------------------------------------------
void 
SmfCampStateExecSuspended::toString(std::string & io_str) const 
{
	getClassName(io_str);
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
void 
SmfCampStateExecSuspended::execute(SmfUpgradeCampaign * i_camp)
{
        int numOfSuspendedProc = 0;
	TRACE_ENTER();
	TRACE("SmfCampStateExecSuspended::execute implementation");

	/* Send execute to all suspended procedures */
	std::vector < SmfUpgradeProcedure * >::iterator iter;

	for (iter = i_camp->m_procedure.begin(); iter != i_camp->m_procedure.end(); ++iter) {
		switch ((*iter)->getState()) {
		case SA_SMF_PROC_SUSPENDED:
			{
				TRACE("SmfCampStateExecuting::Procedure %s suspended, send execute",
				      (*iter)->getProcName().c_str());
				SmfProcedureThread *procThread = (*iter)->getProcThread();
				PROCEDURE_EVT *evt = new PROCEDURE_EVT();
				evt->type = PROCEDURE_EVT_EXECUTE;
				procThread->send(evt);
                                numOfSuspendedProc++;
                                break;
			}
		default:
			break;
		}
	}

	changeState(i_camp, SmfCampStateExecuting::instance());

        /* If no suspended procedures existed, execute next procedure in state initial.
           I.e. we were suspended in between procedures */
        if (numOfSuspendedProc == 0) {
                /* Execute next procedure */
                CAMPAIGN_EVT *evt = new CAMPAIGN_EVT();
                evt->type = CAMPAIGN_EVT_EXECUTE_PROC;
                SmfCampaignThread::instance()->send(evt);
        }

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfCampStateCommitted implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SmfCampState *SmfCampStateCommitted::s_instance = NULL;

SmfCampState *SmfCampStateCommitted::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfCampStateCommitted;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
void 
SmfCampStateCommitted::getClassName(std::string & io_str) const 
{
	io_str = "SmfCampStateCommitted";
}

//------------------------------------------------------------------------------
// toString()
//------------------------------------------------------------------------------
void 
SmfCampStateCommitted::toString(std::string & io_str) const 
{
	getClassName(io_str);
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
void 
SmfCampStateCommitted::execute(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	TRACE("SmfCampStateCommitted::execute implementation");
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfCampStateErrorDetected implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SmfCampState *SmfCampStateErrorDetected::s_instance = NULL;

SmfCampState *SmfCampStateErrorDetected::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfCampStateErrorDetected;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
void 
SmfCampStateErrorDetected::getClassName(std::string & io_str) const 
{
	io_str = "SmfCampStateErrorDetected";
}

//------------------------------------------------------------------------------
// toString()
//------------------------------------------------------------------------------
void 
SmfCampStateErrorDetected::toString(std::string & io_str) const 
{
	getClassName(io_str);
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
void 
SmfCampStateErrorDetected::execute(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	TRACE("SmfCampStateErrorDetected::execute implementation");
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfCampStateExecFailed implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SmfCampState *SmfCampStateExecFailed::s_instance = NULL;

SmfCampState *SmfCampStateExecFailed::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfCampStateExecFailed;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
void 
SmfCampStateExecFailed::getClassName(std::string & io_str) const 
{
	io_str = "SmfCampStateExecFailed";
}

//------------------------------------------------------------------------------
// toString()
//------------------------------------------------------------------------------
void 
SmfCampStateExecFailed::toString(std::string & io_str) const 
{
	getClassName(io_str);
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
void 
SmfCampStateExecFailed::execute(SmfUpgradeCampaign * i_camp)
{
	TRACE_ENTER();
	TRACE("SmfCampStateExecFailed::execute implementation");
	TRACE_LEAVE();
}
