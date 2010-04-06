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

#include <algorithm>
#include "logtrace.h"
#include "SmfUpgradeCampaign.hh"
#include "SmfCampaignThread.hh"
#include "SmfCampaign.hh"
#include "SmfUpgradeProcedure.hh"
#include "SmfProcedureThread.hh"
#include "SmfCampState.hh"

//#include "rollingupgrade.h"
//#include "campaigntime.h"

class SmfUpgradeProcedure;

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

// ------------------------------------------------------------------------------
// SmfUpgradeCampaign()
// ------------------------------------------------------------------------------
 SmfUpgradeCampaign::SmfUpgradeCampaign():
    m_state(SmfCampStateInitial::instance()), 
    m_campPackageName(""), m_campaignName(""), 
    m_noNameSpaceSchemaLocation(""),
    m_xsi(""), 
    m_campaignPeriod(""), 
    m_configurationBase(""), 
    m_campInit(),
    m_waitToCommit(0),
    m_waitToAllowNewCampaign(0),
    m_noOfExecutingProc(0)
{

}

// ------------------------------------------------------------------------------
// ~SmfUpgradeCampaign()
// ------------------------------------------------------------------------------
SmfUpgradeCampaign::~SmfUpgradeCampaign()
{
	TRACE_ENTER();
	//Delete procedures
	std::vector < SmfUpgradeProcedure * >::iterator iter;

	for (iter = m_procedure.begin(); iter != m_procedure.end(); ++iter) {
		delete(*iter);
	}

	TRACE_LEAVE();
}

// ------------------------------------------------------------------------------
// changeState()
// ------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::changeState(const SmfCampState * i_state)
{
	m_state = const_cast < SmfCampState * >(i_state);
	SmfCampaignThread::instance()->campaign()->setState(m_state->getState());
}

// ------------------------------------------------------------------------------
// getClassName(std::string& io_name)
// ------------------------------------------------------------------------------
void  
SmfUpgradeCampaign::getClassName(std::string & io_str) const
{
	io_str = "SmfUpgradeCampaign";
}

// ------------------------------------------------------------------------------
// toString()
// ------------------------------------------------------------------------------
void  
SmfUpgradeCampaign::toString(std::string & io_str) const
{
	getClassName(io_str);
}

//------------------------------------------------------------------------------
// setCampPackageName()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::setCampPackageName(const std::string & i_name)
{
	m_campPackageName = i_name;
}

//------------------------------------------------------------------------------
// getCampPackageName()
//------------------------------------------------------------------------------
const std::string & 
SmfUpgradeCampaign::getCampPackageName(void)
{
	return m_campPackageName;
}

//------------------------------------------------------------------------------
// setCampaignName()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::setCampaignName(const std::string & i_name)
{
	m_campaignName = i_name;
}

//------------------------------------------------------------------------------
// getCampaignName()
//------------------------------------------------------------------------------
const std::string & 
SmfUpgradeCampaign::getCampaignName(void)
{
	return m_campaignName;
}

//------------------------------------------------------------------------------
// setXsi()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::setXsi(const std::string & i_name)
{
	m_xsi = i_name;
}

//------------------------------------------------------------------------------
// setNameSpaceSchemaLocation()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::setNameSpaceSchemaLocation(const std::string & i_name)
{
	m_noNameSpaceSchemaLocation = i_name;
}

//------------------------------------------------------------------------------
// setCampaignPeriod()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::setCampaignPeriod(const std::string & i_period)
{
	m_campaignPeriod = i_period;
}

//------------------------------------------------------------------------------
// setDependencyDescription()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::setConfigurationBase(const std::string & i_confbase)
{
	m_configurationBase = i_confbase;
}

//------------------------------------------------------------------------------
// setCampState()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::setCampState(SaSmfCmpgStateT i_state)
{
	switch (i_state) {
	case SA_SMF_CMPG_INITIAL:
		{
			m_state = SmfCampStateInitial::instance();
			break;
		}

	case SA_SMF_CMPG_EXECUTING:
		{
			m_state = SmfCampStateExecuting::instance();
			break;
		}

	case SA_SMF_CMPG_SUSPENDING_EXECUTION:
		{
			m_state = SmfCampStateSuspendingExec::instance();
			break;
		}

	case SA_SMF_CMPG_EXECUTION_SUSPENDED:
		{
			m_state = SmfCampStateExecSuspended::instance();
			break;
		}

	case SA_SMF_CMPG_EXECUTION_COMPLETED:
		{
			m_state = SmfCampStateExecCompleted::instance();
			break;
		}

	case SA_SMF_CMPG_CAMPAIGN_COMMITTED:
		{
			m_state = SmfCampStateCommitted::instance();
			break;
		}

	case SA_SMF_CMPG_ERROR_DETECTED:
		{
			m_state = SmfCampStateErrorDetected::instance();
			break;
		}

	case SA_SMF_CMPG_EXECUTION_FAILED:
		{
			m_state = SmfCampStateExecFailed::instance();
			break;
		}

#if 0
		/*Not implemented yet */
		SA_SMF_CMPG_SUSPENDED_BY_ERROR_DETECTED = 8,
		    SA_SMF_CMPG_ERROR_DETECTED_IN_SUSPENDING = 9, SA_SMF_CMPG_ROLLING_BACK =
		    11, SA_SMF_CMPG_SUSPENDING_ROLLBACK = 12, SA_SMF_CMPG_ROLLBACK_SUSPENDED =
		    13, SA_SMF_CMPG_ROLLBACK_COMPLETED = 14, SA_SMF_CMPG_ROLLBACK_COMMITTED =
		    15, SA_SMF_CMPG_ROLLBACK_FAILED = 16
#endif
	default:
		{
			LOG_ER("unknown state");
		}
	}

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// addUpgradeProcedure()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::addUpgradeProcedure(SmfUpgradeProcedure * i_procedure)
{
	m_procedure.push_back(i_procedure);
}

//------------------------------------------------------------------------------
// getUpgradeProcedures()
//------------------------------------------------------------------------------
const std::vector < SmfUpgradeProcedure * >&
SmfUpgradeCampaign::getUpgradeProcedures()
{
	return m_procedure;
}

//------------------------------------------------------------------------------
// sortProceduresInExecLevelOrder()
//------------------------------------------------------------------------------
void SmfUpgradeCampaign::sortProceduresInExecLevelOrder()
{
	TRACE_ENTER();

	std::vector < SmfUpgradeProcedure * >::iterator iter;
	std::list < int >levels;

	//Find out used procedure execution levels, just one entry for each level
	for (iter = m_procedure.begin();iter != m_procedure.end(); iter++) {
		levels.push_back((*iter)->getExecLevel());
	}

	//Sort the execution levels and remove duplicates
	levels.sort();
	levels.unique();

	//Create a sorted procedure vector
	std::vector < SmfUpgradeProcedure * >sortedProc;

	std::list < int >::iterator levelIter;
	for (levelIter = levels.begin(); levelIter != levels.end(); levelIter++) {
		iter = m_procedure.begin();
		while (iter != m_procedure.end()) {
			if ((*levelIter) == (*iter)->getExecLevel())
				sortedProc.push_back(*iter);
			iter++;
		}
	}

	//Copy the sorted procedure vector to class member
	m_procedure = sortedProc;

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// addAddToImm()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::addCampInitAddToImm(SmfImmOperation * i_operation)
{
	TRACE_ENTER();
	m_campInit.addAddToImm(i_operation);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// addCampInitAction()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::addCampInitAction(SmfUpgradeAction * i_action)
{
	TRACE_ENTER();
	m_campInit.addCampInitAction(i_action);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// addCampWrapupRemoveFromImm()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::addCampWrapupRemoveFromImm(SmfImmOperation * i_operation)
{
	TRACE_ENTER();
	m_campWrapup.addRemoveFromImm(i_operation);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// addCampCommitAction()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::addCampCompleteAction(SmfUpgradeAction * i_action)
{
	TRACE_ENTER();
	m_campWrapup.addCampCompleteAction(i_action);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// addCampWrapupAction()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::addCampWrapupAction(SmfUpgradeAction * i_action)
{
	TRACE_ENTER();
	m_campWrapup.addCampWrapupAction(i_action);
	TRACE_LEAVE();
}
//------------------------------------------------------------------------------
// setWaitToCommit()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::setWaitToCommit(SaTimeT i_time)
{
	TRACE_ENTER();
	m_waitToCommit = i_time;
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// getWaitToCommit()
//------------------------------------------------------------------------------
SaTimeT
SmfUpgradeCampaign::getWaitToCommit()
{
	return m_waitToCommit;
}

//------------------------------------------------------------------------------
// setWaitToAllowNewCampaign()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::setWaitToAllowNewCampaign(SaTimeT i_time)
{
	TRACE_ENTER();
	m_waitToAllowNewCampaign = i_time;
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// getWaitToAllowNewCampaign()
//------------------------------------------------------------------------------
SaTimeT
SmfUpgradeCampaign::getWaitToAllowNewCampaign()
{
	return m_waitToAllowNewCampaign ;
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::execute()
{
	TRACE_ENTER();
	m_state->execute(this);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeInit()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::executeInit()
{
	TRACE_ENTER();
	m_state->executeInit(this);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeProc()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::executeProc()
{
	TRACE_ENTER();
	m_state->executeProc(this);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeWrapup()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::executeWrapup()
{
	TRACE_ENTER();
	m_state->executeWrapup(this);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// rollback()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::rollback()
{
	TRACE_ENTER();
	m_state->rollback(this);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// suspend()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::suspend()
{
	TRACE_ENTER();
	m_state->suspend(this);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// commit()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::commit()
{
	TRACE_ENTER();
	m_state->commit(this);
	TRACE_LEAVE();
}
