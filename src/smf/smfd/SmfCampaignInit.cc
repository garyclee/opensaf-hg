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
#include "osaf/saf/saAis.h"
#include "osaf/immutil/immutil.h"
#include "base/saf_error.h"
#include "base/logtrace.h"
#include "smf/smfd/SmfCampaignInit.h"
#include "smf/smfd/SmfCampaignThread.h"
#include "smf/smfd/SmfCampaign.h"
#include "smf/smfd/SmfImmOperation.h"
#include "smf/smfd/SmfUpgradeAction.h"
#include "smf/smfd/SmfRollback.h"
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
// Class SmfCampaignInit
// Purpose:
// Comments:
//================================================================================
SmfCampaignInit::SmfCampaignInit():
   m_addToImm(0)
{
}

// ------------------------------------------------------------------------------
// ~SmfTargetNodeTemplate()
// ------------------------------------------------------------------------------
SmfCampaignInit::~SmfCampaignInit()
{
	for (auto& addIElem : m_addToImm) {
		delete((addIElem));
	}

	for (auto& campInitElem : m_campInitAction) {
		delete((campInitElem));
	}

	for (auto& cbkElem : m_callbackAtInit) {
		delete((cbkElem));
	}

	for (auto& cbkElem : m_callbackAtBackup) {
		delete((cbkElem));
	}


	for (auto& cbkElem : m_callbackAtRollback) {
		delete((cbkElem));
        }
}

//------------------------------------------------------------------------------
// addAddToImm()
//------------------------------------------------------------------------------
void
SmfCampaignInit::addAddToImm(SmfImmOperation * i_operation)
{
	m_addToImm.push_back(i_operation);
}

//------------------------------------------------------------------------------
// getAddToImm()
//------------------------------------------------------------------------------
const std::list < SmfImmOperation * >&
SmfCampaignInit::getAddToImm()
{
	return m_addToImm;
}

//------------------------------------------------------------------------------
// addCallbackAtInit()
//------------------------------------------------------------------------------
void 
SmfCampaignInit::addCallbackAtInit(SmfCallback* i_cbk)
{
	m_callbackAtInit.push_back(i_cbk);
}

//------------------------------------------------------------------------------
// addCallbackAtBackup()
//------------------------------------------------------------------------------
void 
SmfCampaignInit::addCallbackAtBackup(SmfCallback* i_cbk)
{
	m_callbackAtBackup.push_back(i_cbk);
}

//------------------------------------------------------------------------------
// addCallbackAtRollback()
//------------------------------------------------------------------------------
void 
SmfCampaignInit::addCallbackAtRollback(SmfCallback* i_cbk)
{
	m_callbackAtRollback.push_back(i_cbk);
}

//------------------------------------------------------------------------------
// addCampInitAction()
//------------------------------------------------------------------------------
void 
SmfCampaignInit::addCampInitAction(SmfUpgradeAction * i_action)
{
	m_campInitAction.push_back(i_action);
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
bool 
SmfCampaignInit::execute()
{
	TRACE_ENTER();

        SaAisErrorT result = SA_AIS_OK;
        SmfImmUtils immUtil;

        LOG_NO("CAMP: Campaign init, start add to IMM (%zu)", m_addToImm.size());

        std::string addToImmRollbackCcbDn;
        addToImmRollbackCcbDn = "smfRollbackElement=AddToImmCcb,";
        addToImmRollbackCcbDn += SmfCampaignThread::instance()->campaign()->getDn();

        if ((result = smfCreateRollbackElement(addToImmRollbackCcbDn,
                                               SmfCampaignThread::instance()->getImmHandle())) != SA_AIS_OK) {
                LOG_ER("SmfCampaignInit failed to create addToImm rollback element %s, rc=%s", 
                       addToImmRollbackCcbDn.c_str(), saf_error(result));
                return false;
        }

	if (m_addToImm.size() > 0) {
                SmfRollbackCcb rollbackCcb(addToImmRollbackCcbDn,
                                           SmfCampaignThread::instance()->getImmHandle());

		if (immUtil.doImmOperations(m_addToImm, &rollbackCcb) != SA_AIS_OK) {
			LOG_ER("SmfCampaignInit add to IMM failed");
			return false;
		}

                if ((result = rollbackCcb.execute()) != SA_AIS_OK) {
			LOG_ER("SmfCampaignInit failed to store rollback CCB rc=%s", saf_error(result));
			return false;
                }
	}
	///////////////////////
	//Campaign init actions
	///////////////////////

	LOG_NO("CAMP: Campaign init, start init actions (%zu)", m_campInitAction.size());

        std::string initRollbackDn;
        initRollbackDn = "smfRollbackElement=CampInit,";
        initRollbackDn += SmfCampaignThread::instance()->campaign()->getDn();

        if ((result = smfCreateRollbackElement(initRollbackDn,
                                               SmfCampaignThread::instance()->getImmHandle())) != SA_AIS_OK) {
                LOG_ER("SmfCampaignInit failed to create campaign init rollback element %s, rc=%s", 
                       initRollbackDn.c_str(), saf_error(result));
                return false;
        }

	for (auto& upActElem : m_campInitAction) {
		SaAisErrorT rc = (*upActElem).execute(SmfCampaignThread::instance()->getImmHandle(),
				&initRollbackDn);
		if (rc != SA_AIS_OK) {
			LOG_ER("SmfCampaignInit init action %d failed, rc=%s", (*upActElem).getId(), saf_error(rc));
			TRACE_LEAVE();
			return false;
		}
	}

	LOG_NO("CAMP: Campaign init completed");

	TRACE_LEAVE();

	return true;
}

//------------------------------------------------------------------------------
// executeCallbackAtInit()
//------------------------------------------------------------------------------
SaAisErrorT
SmfCampaignInit::executeCallbackAtInit()
{
	std::string dn; //Dummy DN, needs only to be set for callback in a step.
	for (auto& cbkElem : m_callbackAtInit) {
		SaAisErrorT rc = (*cbkElem).execute(dn);
		if (rc == SA_AIS_ERR_FAILED_OPERATION) {
			LOG_NO("SmfCampaignInit callback at init %s failed, rc=%s", (*cbkElem).getCallbackLabel().c_str(), saf_error(rc));
			return rc;
		}
	}

	return SA_AIS_OK;
}

//------------------------------------------------------------------------------
// executeCallbackAtBackup()
//------------------------------------------------------------------------------
SaAisErrorT
SmfCampaignInit::executeCallbackAtBackup()
{
	std::string dn;

	for (auto& cbkElem : m_callbackAtBackup) {
		SaAisErrorT rc = (*cbkElem).execute(dn);
		if (rc == SA_AIS_ERR_FAILED_OPERATION) {
			LOG_ER("SmfCampaignInit callbackAtBackup %s failed, rc=%s", (*cbkElem).getCallbackLabel().c_str(), saf_error(rc));
			return rc;
		}
	}
	return SA_AIS_OK;
}

//------------------------------------------------------------------------------
// rollback()
//------------------------------------------------------------------------------
bool 
SmfCampaignInit::rollback()
{
	LOG_NO("CAMP: Start rollback of campaign init actions (%zu)", m_campInitAction.size());

        SaAisErrorT rc;
        std::string initRollbackDn;
        initRollbackDn = "smfRollbackElement=CampInit,";
        initRollbackDn += SmfCampaignThread::instance()->campaign()->getDn();

	std::list < SmfUpgradeAction * >::reverse_iterator upActiter;

        /* For each action (in reverse order) call rollback */
	for (upActiter = m_campInitAction.rbegin(); upActiter != m_campInitAction.rend(); ++upActiter) {
		rc = (*upActiter)->rollback(initRollbackDn);
		if (rc != SA_AIS_OK) {
			LOG_ER("SmfCampaignInit rollback of init action %d failed, rc=%s", (*upActiter)->getId(), saf_error(rc));
			return false;
		}
		
	}

        LOG_NO("CAMP: Campaign init, rollback add to IMM (%zu)", m_addToImm.size());

        SmfImmUtils immUtil;
        std::string addToImmRollbackCcbDn;
        addToImmRollbackCcbDn = "smfRollbackElement=AddToImmCcb,";
        addToImmRollbackCcbDn += SmfCampaignThread::instance()->campaign()->getDn();

        SmfRollbackCcb rollbackCcb(addToImmRollbackCcbDn,
                                   SmfCampaignThread::instance()->getImmHandle());

        if ((rc = rollbackCcb.rollback()) != SA_AIS_OK) {
                LOG_ER("SmfCampaignInit failed to rollback add to IMM CCB, rc=%s", saf_error(rc));
                return false;
	}

#if 0
	std::list < SmfCallbackOptions * >m_callbackAtRollback;
#endif

	LOG_NO("CAMP: Rollback of campaign init actions completed");
	return true;
}
//------------------------------------------------------------------------------
// executeCallbackAtRollback()
//------------------------------------------------------------------------------
bool
SmfCampaignInit::executeCallbackAtRollback()
{
	std::string dn;
	////////////////////////
	//Callback at rollback
	////////////////////////

	TRACE_ENTER();
	for (auto& cbkElem : m_callbackAtRollback) {
		SaAisErrorT rc = (*cbkElem).rollback(dn);
		LOG_NO("SmfCampaignInit callbackAtRollback, rc=%s", saf_error(rc));
		if (rc == SA_AIS_ERR_FAILED_OPERATION) {
			LOG_ER("SmfCampaignInit callbackAtRollback %s failed, rc=%s", (*cbkElem).getCallbackLabel().c_str(), saf_error(rc));
			TRACE_LEAVE();
			return false;
		}
	}
	TRACE_LEAVE();
	return true;
}
