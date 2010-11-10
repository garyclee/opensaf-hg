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
#include <immutil.h>
#include "SmfUpgradeCampaign.hh"
#include "SmfCampaignThread.hh"
#include "SmfCampaign.hh"
#include "SmfUpgradeProcedure.hh"
#include "SmfProcedureThread.hh"
#include "SmfCampState.hh"
#include "SmfUtils.hh"
#include "smfd.h"

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
	TRACE_ENTER();

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
	case SA_SMF_CMPG_SUSPENDED_BY_ERROR_DETECTED:
		{
			m_state = SmfCampStateSuspendedByErrorDetected::instance();
			break;
		}
	case SA_SMF_CMPG_ERROR_DETECTED_IN_SUSPENDING:
		{
			m_state = SmfCampStateErrorDetectedInSuspending::instance();
			break;
		}
	case SA_SMF_CMPG_ROLLING_BACK:
		{
			m_state = SmfCampRollingBack::instance();
			break;
		}
	case SA_SMF_CMPG_SUSPENDING_ROLLBACK:
		{
			m_state = SmfCampSuspendingRollback::instance();
			break;
		}
	case SA_SMF_CMPG_ROLLBACK_SUSPENDED:
		{
			m_state = SmfCampRollbackSuspended::instance();
			break;
		}
	case SA_SMF_CMPG_ROLLBACK_COMPLETED:
		{
			m_state = SmfCampRollbackCompleted::instance();
			break;
		}
	case SA_SMF_CMPG_ROLLBACK_COMMITTED:
		{
			m_state = SmfCampRollbackCommitted::instance();
			break;
		}
	case SA_SMF_CMPG_ROLLBACK_FAILED:
		{
			m_state = SmfCampRollbackFailed::instance();
			break;
		}
	default:
		{
			LOG_ER("SmfUpgradeCampaign: Trying to set unknown state %d", i_state);
		}
	}

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// addUpgradeProcedure()
//------------------------------------------------------------------------------
bool 
SmfUpgradeCampaign::addUpgradeProcedure(SmfUpgradeProcedure * i_procedure)
{
	std::vector < SmfUpgradeProcedure * >::iterator iter;
 
        iter = this->m_procedure.begin();
        while (iter != this->m_procedure.end()) {
                if (!strcmp((*iter)->getProcName().c_str(), i_procedure->getProcName().c_str())){
                        LOG_ER("CAMP: Procedure %s is already present, invalid upgrade campaign", i_procedure->getProcName().c_str());
                        return false;
                }
                iter++;
        }
	m_procedure.push_back(i_procedure);
	return true;
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
// createCampRestartInfo()
//------------------------------------------------------------------------------
SaAisErrorT
SmfUpgradeCampaign::createCampRestartInfo()
{
	TRACE_ENTER();
	SaAisErrorT rc = SA_AIS_OK;

	SmfImmRTCreateOperation icoCampRestartInfo;

	icoCampRestartInfo.setClassName("OpenSafSmfCampRestartInfo");
	icoCampRestartInfo.setParentDn(SmfCampaignThread::instance()->campaign()->getDn());
	icoCampRestartInfo.setImmHandle(SmfCampaignThread::instance()->getImmHandle());

	SmfImmAttribute attrsmfRestartInfo;
        attrsmfRestartInfo.setName("smfRestartInfo");
        attrsmfRestartInfo.setType("SA_IMM_ATTR_SASTRINGT");
        attrsmfRestartInfo.addValue("smfRestartInfo=info");
        icoCampRestartInfo.addValue(attrsmfRestartInfo);

	SmfImmAttribute attrsmfCampRestartCnt;
        attrsmfCampRestartCnt.setName("smfCampRestartCnt");
        attrsmfCampRestartCnt.setType("SA_IMM_ATTR_SAUINT32T");
        attrsmfCampRestartCnt.addValue("0");
        icoCampRestartInfo.addValue(attrsmfCampRestartCnt);

        rc = icoCampRestartInfo.execute(); //Create the object
	if (rc != SA_AIS_OK){
                LOG_ER("SmfUpgradeCampaign::createCampRestartInfo: icoCampRestartInfo.execute() returned %d", rc);
        }

	TRACE_LEAVE();
	return rc;
}

//------------------------------------------------------------------------------
// tooManyRestarts()
//------------------------------------------------------------------------------
SaAisErrorT 
SmfUpgradeCampaign::tooManyRestarts(bool *o_result)
{
	TRACE_ENTER();
	SaAisErrorT rc = SA_AIS_OK;
	SaImmAttrValuesT_2 **attributes;
	int curCnt = 0;

	/* Read the SmfCampRestartInfo object smfCampRestartCnt attr */
	std::string obj = "smfRestartInfo=info," + SmfCampaignThread::instance()->campaign()->getDn();
	SmfImmUtils immUtil;
	if (immUtil.getObject(obj, &attributes) == true) {
		const SaUint32T* cnt = immutil_getUint32Attr((const SaImmAttrValuesT_2 **)attributes, "smfCampRestartCnt", 0);

		curCnt = *cnt;

		/* Increment and store counter */
		curCnt++;
		SmfImmRTUpdateOperation imoCampRestartInfo;
		std::string dn = "smfRestartInfo=info," + SmfCampaignThread::instance()->campaign()->getDn();
		imoCampRestartInfo.setDn(dn);
		imoCampRestartInfo.setImmHandle(SmfCampaignThread::instance()->getImmHandle());
		imoCampRestartInfo.setOp("SA_IMM_ATTR_VALUES_REPLACE");

		SmfImmAttribute attrsmfCampRestartCnt;
		attrsmfCampRestartCnt.setName("smfCampRestartCnt");
		attrsmfCampRestartCnt.setType("SA_IMM_ATTR_SAUINT32T");
		char buf[5];
		snprintf(buf, 4, "%d", curCnt);
		attrsmfCampRestartCnt.addValue(buf);
		imoCampRestartInfo.addValue(attrsmfCampRestartCnt);

		rc = imoCampRestartInfo.execute(); //Modify the object
		if (rc != SA_AIS_OK){
			LOG_ER("SmfUpgradeCampaign::createCampRestartInfo: imoCampRestartInfo.execute() returned %d", rc);
		}
	}

	int maxCnt = smfd_cb->smfCampMaxRestart;
	TRACE("maxCnt=%d, curCnt=%d", 	maxCnt, curCnt);	
	if (curCnt > maxCnt){
		TRACE("TRUE");
		*o_result = true;
	} else {
		TRACE("FALSE");
		*o_result = false;
	}

	TRACE_LEAVE();
	return rc;
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::execute()
{
	TRACE_ENTER();
        SmfCampResultT campResult;

        while (1) {
                campResult = m_state->execute(this);

                if (campResult != SMF_CAMP_CONTINUE) {
                        break;
                }
        }

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeInit()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::executeInit()
{
	TRACE_ENTER();
	//m_state->executeInit(this);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeProc()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::executeProc()
{
	TRACE_ENTER();
        SmfCampResultT campResult;

        while (1) {
                campResult = m_state->executeProc(this);

                if (campResult != SMF_CAMP_CONTINUE) {
                        break;
                }
        }

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeWrapup()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::executeWrapup()
{
	TRACE_ENTER();
	//m_state->executeWrapup(this);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// rollback()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::rollback()
{
	TRACE_ENTER();
        SmfCampResultT campResult;

        while (1) {
                campResult = m_state->rollback(this);

                if (campResult != SMF_CAMP_CONTINUE) {
                        break;
                }
        }

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// rollbackProc()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::rollbackProc()
{
	TRACE_ENTER();
        SmfCampResultT campResult;

        while (1) {
                campResult = m_state->rollbackProc(this);

                if (campResult != SMF_CAMP_CONTINUE) {
                        break;
                }
        }

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// suspend()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::suspend()
{
	TRACE_ENTER();
        SmfCampResultT campResult;

        while (1) {
                campResult = m_state->suspend(this);

                if (campResult != SMF_CAMP_CONTINUE) {
                        break;
                }
        }
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// commit()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::commit()
{
	TRACE_ENTER();
        SmfCampResultT campResult;

        while (1) {
                campResult = m_state->commit(this);

                if (campResult != SMF_CAMP_CONTINUE) {
                        break;
                }
        }
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// procResult()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::procResult(SmfUpgradeProcedure* i_procedure, SmfProcResultT i_result)
{
	TRACE_ENTER();
        SmfCampResultT campResult;

        while (1) {
                campResult = m_state->procResult(this, i_procedure, i_result);

                if (campResult != SMF_CAMP_CONTINUE) {
                        break;
                }
        }

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// continueExec()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::continueExec()
{
	TRACE_ENTER();
        SaSmfCmpgStateT currentState = m_state->getState();

        /* Check if we have restarted too many times */
        bool o_result;
	if (this->tooManyRestarts(&o_result) == SA_AIS_OK){
		if (o_result == true) {
			LOG_ER("The campaign have been restarted to many times");
			int cnt = smfd_cb->smfCampMaxRestart;
			std::string error = "To many campaign restarts, max " + cnt;
			SmfCampaignThread::instance()->campaign()->setError(error);
			changeState(SmfCampStateExecFailed::instance());
			TRACE_LEAVE();
			return;
		}
	} else {
		LOG_ER("continueExec() restart number check failed");
		std::string error = "Restart number check failed";
		SmfCampaignThread::instance()->campaign()->setError(error);
		changeState(SmfCampStateExecFailed::instance());
		TRACE_LEAVE();
		return;
	}

        switch (currentState) {
        case SA_SMF_CMPG_EXECUTING:
                execute();
                break;
        case SA_SMF_CMPG_ROLLING_BACK:
                rollback();
                break;
        default:
                TRACE("current continue state is %u, do nothing", currentState);
                break;
        }
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// resetMaintenanceState()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::resetMaintenanceState()
{
	TRACE_ENTER();

        LOG_NO("CAMP: Campaign wrapup, reset saAmfSUMaintenanceCampaign flags");
        //Find all SUs in the system
        std::list < std::string > objectList;
	SmfImmUtils immUtil;
        (void)immUtil.getChildren("", objectList, SA_IMM_SUBTREE, "SaAmfSU");

        //Reset saAmfSUMaintenanceCampaign for all found SUs
        const std::string campDn = SmfCampaignThread::instance()->campaign()->getDn();
        std::list < SmfImmOperation * > operations;
        std::list < std::string >::const_iterator suit;
	SaImmAttrValuesT_2 **attributes;

        for (suit = objectList.begin(); suit != objectList.end(); ++suit) {

		if (immUtil.getObject((*suit), &attributes) == true) {
			const SaNameT *maintCamp =
				immutil_getNameAttr((const SaImmAttrValuesT_2 **)attributes,
						    "saAmfSUMaintenanceCampaign",
						    0);

			if ((maintCamp != NULL) && (maintCamp->length > 0)) {
				SmfImmModifyOperation *modop = new (std::nothrow) SmfImmModifyOperation;
				assert(modop != 0);
				modop->setDn(*suit);
				modop->setOp("SA_IMM_ATTR_VALUES_DELETE");
				SmfImmAttribute saAmfSUMaintenanceCampaign;
				saAmfSUMaintenanceCampaign.setName("saAmfSUMaintenanceCampaign");
				saAmfSUMaintenanceCampaign.setType("SA_IMM_ATTR_SANAMET");
				saAmfSUMaintenanceCampaign.addValue(campDn);
				modop->addValue(saAmfSUMaintenanceCampaign);
				operations.push_back(modop);
			}
		}
        }

        if (immUtil.doImmOperations(operations) != SA_AIS_OK) {
                LOG_ER("SmfUpgradeStep::setMaintenanceState(), fails to reset all saAmfSUMaintenanceCampaign attr");
        }

        //Delete the created SmfImmModifyOperation instances
        std::list < SmfImmOperation * > ::iterator operIter;
        for (operIter = operations.begin(); operIter != operations.end(); ++operIter) {
                delete (*operIter);
        }

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// removeRunTimeObjects()
//------------------------------------------------------------------------------
void 
SmfUpgradeCampaign::removeRunTimeObjects()
{
	TRACE_ENTER();

	/* Remove smfRestartInfo runtime object */
        LOG_NO("CAMP: Campaign wrapup, Remove runtime objects");

        SaNameT objectName;
	std::string dn = "smfRestartInfo=info," + SmfCampaignThread::instance()->campaign()->getDn(); 
	objectName.length = dn.length();
	strncpy((char *)objectName.value, dn.c_str(), objectName.length);
	objectName.value[objectName.length] = 0;

	SaAisErrorT rc = immutil_saImmOiRtObjectDelete(SmfCampaignThread::instance()->getImmHandle(),	//The OI handle
					   &objectName);

	if (rc != SA_AIS_OK) {
		LOG_ER("immutil_saImmOiRtObjectDelete returned %u for %s, continuing", rc, dn.c_str());
	}

	/* Remove campaign rollback element runtime objects */
        dn = "smfRollbackElement=AddToImmCcb," + SmfCampaignThread::instance()->campaign()->getDn(); 
	objectName.length = dn.length();
	strncpy((char *)objectName.value, dn.c_str(), objectName.length);
	objectName.value[objectName.length] = 0;

	rc = immutil_saImmOiRtObjectDelete(SmfCampaignThread::instance()->getImmHandle(),	//The OI handle
                                           &objectName);

	if (rc != SA_AIS_OK) {
		LOG_ER("immutil_saImmOiRtObjectDelete returned %u for %s, continuing", rc, dn.c_str());
	}

        dn = "smfRollbackElement=CampInit," + SmfCampaignThread::instance()->campaign()->getDn(); 
	objectName.length = dn.length();
	strncpy((char *)objectName.value, dn.c_str(), objectName.length);
	objectName.value[objectName.length] = 0;

	rc = immutil_saImmOiRtObjectDelete(SmfCampaignThread::instance()->getImmHandle(),	//The OI handle
					   &objectName);

	if (rc != SA_AIS_OK) {
		LOG_ER("immutil_saImmOiRtObjectDelete returned %u for %s, continuing", rc, dn.c_str());
	}

        dn = "smfRollbackElement=CampComplete," + SmfCampaignThread::instance()->campaign()->getDn(); 
	objectName.length = dn.length();
	strncpy((char *)objectName.value, dn.c_str(), objectName.length);
	objectName.value[objectName.length] = 0;

	rc = immutil_saImmOiRtObjectDelete(SmfCampaignThread::instance()->getImmHandle(),	//The OI handle
					   &objectName);

	if (rc != SA_AIS_OK) {
		LOG_ER("immutil_saImmOiRtObjectDelete returned %u for %s, continuing", rc, dn.c_str());
	}

	TRACE_LEAVE();
}

