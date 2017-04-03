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
#include "osaf/immutil/immutil.h"
#include "base/saf_error.h"
#include "base/logtrace.h"
#include "smf/smfd/SmfCampaign.h"
#include "smf/smfd/SmfCampaignThread.h"
#include "smf/smfd/SmfCampaignWrapup.h"
#include "smf/smfd/SmfImmOperation.h"
#include "smf/smfd/SmfRollback.h"
#include "smf/smfd/SmfUpgradeAction.h"
#include "smf/smfd/SmfUtils.h"

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

//================================================================================
// Class SmfCampaignWrapup
// Purpose:
// Comments:
//================================================================================
SmfCampaignWrapup::SmfCampaignWrapup():
   m_removeFromImm(0)
{
}

// ------------------------------------------------------------------------------
// ~SmfTargetNodeTemplate()
// ------------------------------------------------------------------------------
SmfCampaignWrapup::~SmfCampaignWrapup()
{
  for (auto& elemE : m_removeFromImm) {
    delete((elemE));
  }
  for (auto& actionElem : m_campCompleteAction) {
    delete((actionElem));
  }
  for (auto& actionElem : m_campWrapupAction) {
    delete((actionElem));
  }
  for (auto& cbkElem : m_callbackAtCommit) {
    delete((cbkElem));
  }
}

//------------------------------------------------------------------------------
// addRemoveFromImm()
//------------------------------------------------------------------------------
void
SmfCampaignWrapup::addRemoveFromImm(SmfImmOperation * i_operation)
{
	m_removeFromImm.push_back(i_operation);
}

//------------------------------------------------------------------------------
// addCallbackAtCommit()
//------------------------------------------------------------------------------
void 
SmfCampaignWrapup::addCallbackAtCommit(SmfCallback* i_cbk)
{
	m_callbackAtCommit.push_back(i_cbk);
}

//------------------------------------------------------------------------------
// addCampCompleteAction()
//------------------------------------------------------------------------------
void 
SmfCampaignWrapup::addCampCompleteAction(SmfUpgradeAction * i_action)
{
	m_campCompleteAction.push_back(i_action);
}

//------------------------------------------------------------------------------
// addCampWrapupAction()
//------------------------------------------------------------------------------
void 
SmfCampaignWrapup::addCampWrapupAction(SmfUpgradeAction * i_action)
{
	m_campWrapupAction.push_back(i_action);
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
bool 
SmfCampaignWrapup::executeCampWrapup()
{
	TRACE_ENTER();
        bool rc = true;

	///////////////////////
	//Callback at commit
	///////////////////////
	std::string dn;
	for (auto& cbkElem : m_callbackAtCommit) {
		SaAisErrorT rc = (*cbkElem).execute(dn);
		if (rc == SA_AIS_ERR_FAILED_OPERATION) {
			LOG_NO("SmfCampaignCommit callback %s failed, rc=%s", (*cbkElem).getCallbackLabel().c_str(), saf_error(rc));
		}
	}

	// The actions below are trigged by a campaign commit operation.
	// The campaign will enter state "commited" even if some actions fails.
	// Just log errors and try to execute as many operations as possible.

	LOG_NO("CAMP: Campaign wrapup, start wrapup actions (%zu)", m_campWrapupAction.size());
	for (auto& elem : m_campWrapupAction) {
		if ((*elem).execute(0) != SA_AIS_OK) {
			LOG_NO("SmfCampaignWrapup campWrapupActions %d failed", (*elem).getId());
		}
	}

	LOG_NO("CAMP: Campaign wrapup, start remove from IMM (%zu)", m_removeFromImm.size());
	if (m_removeFromImm.size() > 0) {
		SmfImmUtils immUtil;
		if (immUtil.doImmOperations(m_removeFromImm) != SA_AIS_OK) {
			LOG_NO("SmfCampaignWrapup remove from IMM failed");
		}
	}

	LOG_NO("CAMP: Campaign wrapup actions completed");

	TRACE_LEAVE();

	return rc;
}

//------------------------------------------------------------------------------
// rollback()
//------------------------------------------------------------------------------
bool 
SmfCampaignWrapup::rollbackCampWrapup()
{
	TRACE_ENTER();
        bool rc = true;

	///////////////////////
	//Callback at commit
	///////////////////////
        
	LOG_NO("CAMP: Campaign wrapup, rollback campaign commit callbacks");
	std::string dn;
	for (auto& cbkElem : m_callbackAtCommit) {
		SaAisErrorT rc = (*cbkElem).rollback(dn);
		if (rc == SA_AIS_ERR_FAILED_OPERATION) {
			LOG_NO("SmfCampaignCommit rollback of callback %s failed (rc=%s), ignoring", 
                               (*cbkElem).getCallbackLabel().c_str(), saf_error(rc));
		}
	}

	// The actions below are trigged by a campaign commit operation after rollback.
	// The campaign will enter state "rollback_commited" even if some actions fails.
	// Just log errors and try to execute as many operations as possible.
        // 

	LOG_NO("CAMP: Campaign wrapup, rollback wrapup actions (%zu)", m_campWrapupAction.size());
	for (auto& elem : m_campWrapupAction) {
                SmfImmCcbAction* immCcb = NULL;
                if ((immCcb = dynamic_cast<SmfImmCcbAction*>(elem)) != NULL) {
                        /* Since noone of these IMM CCB has been executed it's no point
                           in trying to roll them back */
			TRACE("SmfCampaignWrapup skipping immCcb rollback %d", 
                               (*elem).getId()); 
                        continue;
                }
		if ((*elem).rollback(dn) != SA_AIS_OK) {
			LOG_NO("SmfCampaignWrapup rollback campWrapupAction %d failed, ignoring", 
                               (*elem).getId());
		}
	}

        /* Since the removeFromImm is made for the upgrade case there is 
           no point in trying them at rollback */

	LOG_NO("CAMP: Campaign wrapup rollback actions completed");

	TRACE_LEAVE();

	return rc;
}

//------------------------------------------------------------------------------
// executeComplete()
//------------------------------------------------------------------------------
bool 
SmfCampaignWrapup::executeCampComplete()
{
	TRACE_ENTER();

	//Campaign wrapup complete actions
	LOG_NO("CAMP: Start campaign complete actions (%zu)", m_campCompleteAction.size());
        SaAisErrorT result;
        std::string completeRollbackDn;
        completeRollbackDn = "smfRollbackElement=CampComplete,";
        completeRollbackDn += SmfCampaignThread::instance()->campaign()->getDn();

        if ((result = smfCreateRollbackElement(completeRollbackDn,
                                               SmfCampaignThread::instance()->getImmHandle())) != SA_AIS_OK) {
                LOG_ER("SmfCampaignWrapup failed to create campaign complete rollback element %s, rc=%s", 
                       completeRollbackDn.c_str(), saf_error(result));
                return false;
        }

	for (auto& elem : m_campCompleteAction) {
		if ((result = (*elem).execute(SmfCampaignThread::instance()->getImmHandle(),
                                               &completeRollbackDn)) != SA_AIS_OK) {
			LOG_ER("SmfCampaignWrapup campCompleteAction %d failed, rc=%s", (*elem).getId(), saf_error(result));
			return false;
		}
	}

	LOG_NO("CAMP: Campaign complete actions completed");

	TRACE_LEAVE();

	return true;
}

//------------------------------------------------------------------------------
// rollbackCampComplete()
//------------------------------------------------------------------------------
bool 
SmfCampaignWrapup::rollbackCampComplete()
{
	LOG_NO("CAMP: Start rollback campaign complete actions (%zu)", m_campCompleteAction.size());

        SaAisErrorT rc;
        std::string completeRollbackDn;

        TRACE("Start rollback of all complete actions ");

        completeRollbackDn = "smfRollbackElement=CampComplete,";
        completeRollbackDn += SmfCampaignThread::instance()->campaign()->getDn();

	std::list < SmfUpgradeAction * >::reverse_iterator upActiter;

        TRACE("Start rollback of all complete actions (in reverse order)");
        /* For each action (in reverse order) call rollback */
	for (upActiter = m_campCompleteAction.rbegin(); upActiter != m_campCompleteAction.rend(); ++upActiter) {
		rc = (*upActiter)->rollback(completeRollbackDn);
		if (rc != SA_AIS_OK) {
			LOG_ER("SmfCampaignWrapup rollback of complete action %d failed, rc=%s", (*upActiter)->getId(), saf_error(rc));
			return false;
		}
		
	}

	LOG_NO("CAMP: Rollback of campaign complete actions completed");
	return true;
}

