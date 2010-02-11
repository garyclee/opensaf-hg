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
#include <saAis.h>
#include <saSmf.h>
#include <stdio.h>
#include <logtrace.h>

#include "SmfUpgradeStep.hh"
#include "SmfCampaign.hh"
#include "SmfUpgradeProcedure.hh"
#include "SmfProcedureThread.hh"
#include "SmfStepState.hh"
#include "SmfCampaignThread.hh"
#include "SmfTargetTemplate.hh"
#include "SmfUtils.hh"
#include "immutil.h"
#include "smfd_smfnd.h"
#include "smfd.h"

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
// Class SmfUpgradeStep
// Purpose:
// Comments:
//================================================================================

SmfUpgradeStep::SmfUpgradeStep():
   m_state(SmfStepStateInitial::instance()),
   m_maxRetry(0), 
   m_retryCount(0), 
   m_restartOption(0), 
   m_procedure(NULL),
   m_stepType(SMF_STEP_UNKNOWN),
   m_switchOver(false)
{
}

// ------------------------------------------------------------------------------
// ~SmfUpgradeStep()
// ------------------------------------------------------------------------------
SmfUpgradeStep::~SmfUpgradeStep()
{
	std::list < SmfImmOperation * >::iterator it;

	/* Delete Imm operations */
	for (it = m_modificationList.begin(); it != m_modificationList.end(); ++it) {
		delete(*it);
	}
}

// ------------------------------------------------------------------------------
// init()
// ------------------------------------------------------------------------------
SaAisErrorT 
SmfUpgradeStep::init(const SaImmAttrValuesT_2 ** attrValues)
{
	const SaImmAttrValuesT_2 **attribute;

	for (attribute = attrValues; *attribute != NULL; attribute++) {
		void *value;

		if ((*attribute)->attrValuesNumber != 1) {
			LOG_ER("invalid number of values %u for %s", (*attribute)->attrValuesNumber,
			       (*attribute)->attrName);
			continue;
		}

		value = (*attribute)->attrValues[0];

		if (strcmp((*attribute)->attrName, "safSmfStep") == 0) {
			char *rdn = *((char **)value);
			m_rdn = rdn;
			TRACE("init safSmfStep = %s", rdn);
		} else if (strcmp((*attribute)->attrName, "saSmfStepMaxRetry")
			   == 0) {
			unsigned int maxRetry = *((unsigned int *)value);
			m_maxRetry = maxRetry;
			TRACE("init saSmfStepMaxRetry = %u", m_maxRetry);
		} else if (strcmp((*attribute)->attrName, "saSmfStepRetryCount")
			   == 0) {
			unsigned int retryCount = *((unsigned int *)value);
			m_retryCount = retryCount;
			TRACE("init saSmfStepRetryCount = %u", m_retryCount);
		} else if (strcmp((*attribute)->attrName, "saSmfStepRestartOption")
			   == 0) {
			unsigned int restartOption = *((unsigned int *)value);
			m_restartOption = restartOption;
			TRACE("init saSmfStepRestartOption = %u", m_restartOption);
		} else if (strcmp((*attribute)->attrName, "saSmfStepState") == 0) {
			unsigned int state = *((unsigned int *)value);

			if ((state >= SA_SMF_STEP_INITIAL) && (state <= SA_SMF_STEP_ROLLBACK_FAILED)) {
				setStepState((SaSmfStepStateT) state);
			} else {
				LOG_ER("invalid step state %u", state);
				setStepState(SA_SMF_STEP_INITIAL);
			}
			TRACE("init saSmfStepState = %u", (int)state);
		} else {
			TRACE("init unhandled attribute = %s", (*attribute)->attrName);
		}
	}

	return SA_AIS_OK;
}

// ------------------------------------------------------------------------------
// changeState()
// ------------------------------------------------------------------------------
void 
SmfUpgradeStep::changeState(const SmfStepState * i_state)
{
	TRACE_ENTER();
	//Change state class pointer
	m_state = const_cast < SmfStepState * >(i_state);

	setImmStateAndSendNotification(i_state->getState());

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// getState()
//------------------------------------------------------------------------------
SaSmfStepStateT 
SmfUpgradeStep::getState() const
{
	return m_state->getState();
}

//------------------------------------------------------------------------------
// setStepState()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setStepState(SaSmfStepStateT i_state)
{
	TRACE_ENTER();

	switch (i_state) {
	case SA_SMF_STEP_INITIAL:
		{
			m_state = SmfStepStateInitial::instance();
			break;
		}

	case SA_SMF_STEP_EXECUTING:
		{
			m_state = SmfStepStateExecuting::instance();
			break;
		}

	case SA_SMF_STEP_COMPLETED:
		{
			m_state = SmfStepStateCompleted::instance();
			break;
		}

	case SA_SMF_STEP_FAILED:
		{
			m_state = SmfStepStateFailed::instance();
			break;
		}

#if 0
		/* Remains to be implemented */
		SA_SMF_STEP_UNDOING = 3, SA_SMF_STEP_UNDONE = 5, SA_SMF_STEP_ROLLING_BACK =
		    7, SA_SMF_STEP_UNDOING_ROLLBACK = 8, SA_SMF_STEP_ROLLED_BACK = 9, SA_SMF_STEP_ROLLBACK_UNDONE =
		    10, SA_SMF_STEP_ROLLBACK_FAILED = 11
#endif
	default:
		{
			LOG_ER("unknown state");
		}
	}

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// setRdn()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setRdn(const std::string & i_rdn)
{
	m_rdn = i_rdn;
}

//------------------------------------------------------------------------------
// getRdn()
//------------------------------------------------------------------------------
const std::string & 
SmfUpgradeStep::getRdn() const
{
	return m_rdn;
}

//------------------------------------------------------------------------------
// setDn()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setDn(const std::string & i_dn)
{
	m_dn = i_dn;
}

//------------------------------------------------------------------------------
// getDn()
//------------------------------------------------------------------------------
const std::string & 
SmfUpgradeStep::getDn() const
{
	return m_dn;
}

//------------------------------------------------------------------------------
// setMaxRetry()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setMaxRetry(unsigned int i_maxRetry)
{
	m_maxRetry = i_maxRetry;
}

//------------------------------------------------------------------------------
// getMaxRetry()
//------------------------------------------------------------------------------
unsigned int 
SmfUpgradeStep::getMaxRetry() const
{
	return m_maxRetry;
}

//------------------------------------------------------------------------------
// setRetryCount()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setRetryCount(unsigned int i_retryCount)
{
	m_retryCount = i_retryCount;
}

//------------------------------------------------------------------------------
// getRetryCount()
//------------------------------------------------------------------------------
unsigned int 
SmfUpgradeStep::getRetryCount() const
{
	return m_retryCount;
}

//------------------------------------------------------------------------------
// setRestartOption()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setRestartOption(unsigned int i_restartOption)
{
	m_restartOption = i_restartOption;
}

//------------------------------------------------------------------------------
// getRestartOption()
//------------------------------------------------------------------------------
unsigned int 
SmfUpgradeStep::getRestartOption() const
{
	return m_restartOption;
}

//------------------------------------------------------------------------------
// setImmStateAndSendNotification()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setImmStateAndSendNotification(SaSmfStepStateT i_state)
{
	TRACE_ENTER();

	if (m_procedure == NULL) {
		return;
	}

	m_procedure->getProcThread()->updateImmAttr(this->getDn().c_str(), "saSmfStepState", SA_IMM_ATTR_SAUINT32T, &i_state);

	SmfCampaignThread::instance()->sendStateNotification(m_dn, 0x67, SA_NTF_MANAGEMENT_OPERATION, SA_SMF_STEP_STATE, i_state);
	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// addActivationUnit()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::addActivationUnit(const std::string & i_activationUnit)
{
	m_activationUnit.m_actedOn.push_back(i_activationUnit);
}

//------------------------------------------------------------------------------
// getActivationUnits()
//------------------------------------------------------------------------------
const std::list < std::string > &
SmfUpgradeStep::getActivationUnitList()
{
	return m_activationUnit.m_actedOn;
}

//------------------------------------------------------------------------------
// addDeactivationUnit()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::addDeactivationUnit(const std::string & i_deactivationUnit)
{
	m_deactivationUnit.m_actedOn.push_back(i_deactivationUnit);
}

//------------------------------------------------------------------------------
// getDeactivationUnits()
//------------------------------------------------------------------------------
const std::list < std::string > &
SmfUpgradeStep::getDeactivationUnitList()
{
	return m_deactivationUnit.m_actedOn;
}

//------------------------------------------------------------------------------
// addSwRemove()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::addSwRemove(const std::list < SmfBundleRef * >&i_swRemove)
{
	std::list < SmfBundleRef * >::const_iterator it = i_swRemove.begin();
	while (it != i_swRemove.end()) {
		SmfBundleRef newSwRemove = *(*it);
		m_swRemoveList.push_back(newSwRemove);
		it++;
	}
}
void 
SmfUpgradeStep::addSwRemove(std::list<SmfBundleRef> const& i_swRemove)
{
	m_swRemoveList = i_swRemove;
}

//------------------------------------------------------------------------------
// getSwRemoveList()
//------------------------------------------------------------------------------
const std::list < SmfBundleRef > &
SmfUpgradeStep::getSwRemoveList()
{
	return m_swRemoveList;
}

//------------------------------------------------------------------------------
// addSwAdd()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::addSwAdd(const std::list < SmfBundleRef * >&i_swAdd)
{
	std::list < SmfBundleRef * >::const_iterator it = i_swAdd.begin();
	while (it != i_swAdd.end()) {
		SmfBundleRef newSwAdd = *(*it);
		m_swAddList.push_back(newSwAdd);
		it++;
	}
}
void 
SmfUpgradeStep::addSwAdd(std::list<SmfBundleRef> const& i_swAdd)
{
	m_swAddList = i_swAdd;
}

//------------------------------------------------------------------------------
// getSwAddList()
//------------------------------------------------------------------------------
const std::list < SmfBundleRef > &
SmfUpgradeStep::getSwAddList()
{
	return m_swAddList;
}

//------------------------------------------------------------------------------
// addModification()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::addModification(SmfImmModifyOperation * i_modification)
{
	m_modificationList.push_back(i_modification);
}

//------------------------------------------------------------------------------
// getModifications()
//------------------------------------------------------------------------------
std::list < SmfImmOperation * >&
SmfUpgradeStep::getModifications()
{
	return m_modificationList;
}

//------------------------------------------------------------------------------
// addImmOperation()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::addImmOperation(SmfImmOperation* i_immoperation)
{
	m_modificationList.push_back(i_immoperation);
}

//------------------------------------------------------------------------------
// setSwNode()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setSwNode(const std::string & i_swNode)
{
	m_swNode = i_swNode;
}

//------------------------------------------------------------------------------
// getSwNode()
//------------------------------------------------------------------------------
const std::string & 
SmfUpgradeStep::getSwNode()
{
	return m_swNode;
}

//------------------------------------------------------------------------------
// setStepType()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setStepType(SmfStepT i_type)
{
	m_stepType = i_type;
}

//------------------------------------------------------------------------------
// getStepType()
//------------------------------------------------------------------------------
SmfStepT 
SmfUpgradeStep::getStepType()
{
	return m_stepType;
}

//------------------------------------------------------------------------------
// offlineInstallBundles()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::offlineInstallBundles(const std::string & i_node)
{
	TRACE("Offline install bundles on node %s", i_node.c_str());
	return callBundleScript(SMF_STEP_OFFLINE_INSTALL, m_swAddList, i_node);
}

//------------------------------------------------------------------------------
// onlineInstallBundles()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::onlineInstallBundles(const std::string & i_node)
{
	TRACE("Online install bundles on node %s", i_node.c_str());
	return callBundleScript(SMF_STEP_ONLINE_INSTALL, m_swAddList, i_node);
}

//------------------------------------------------------------------------------
// offlineRemoveBundles()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::offlineRemoveBundles(const std::string & i_node)
{
	TRACE("Offline remove bundles on node %s", i_node.c_str());
	return callBundleScript(SMF_STEP_OFFLINE_REMOVE, m_swRemoveList, i_node);
}

//------------------------------------------------------------------------------
// onlineRemoveBundles()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::onlineRemoveBundles(const std::string & i_node)
{
	TRACE("Online remove bundles on node %s", i_node.c_str());
	return callBundleScript(SMF_STEP_ONLINE_REMOVE, m_swRemoveList, i_node);
}

//------------------------------------------------------------------------------
// lockDeactivationUnits()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::lockDeactivationUnits()
{
	TRACE("lock deactivation units");
	const SaImmAdminOperationParamsT_2 **params = (const SaImmAdminOperationParamsT_2 **) new SaImmAdminOperationParamsT_2;
	params[0] = NULL;	//Null terminate the list of parameter pointers
	return callAdminOperation(SA_AMF_ADMIN_LOCK, params, m_deactivationUnit.m_actedOn);
}

//------------------------------------------------------------------------------
// terminateDeactivationUnits()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::terminateDeactivationUnits()
{
	TRACE("terminate deactivation units");
	const SaImmAdminOperationParamsT_2 **params = (const SaImmAdminOperationParamsT_2 **) new SaImmAdminOperationParamsT_2;
	params[0] = NULL;	//Null terminate the list of parameter pointers
	return callAdminOperation(SA_AMF_ADMIN_LOCK_INSTANTIATION, params, m_deactivationUnit.m_actedOn);
}

//------------------------------------------------------------------------------
// unlockActivationUnits()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::unlockActivationUnits()
{
	TRACE("unlock activation units");
	const SaImmAdminOperationParamsT_2 **params = (const SaImmAdminOperationParamsT_2 **) new SaImmAdminOperationParamsT_2;
	params[0] = NULL;	//Null terminate the list of parameter pointers
	return callAdminOperation(SA_AMF_ADMIN_UNLOCK, params, m_activationUnit.m_actedOn);
}

//------------------------------------------------------------------------------
// instantiateActivationUnits()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::instantiateActivationUnits()
{
	TRACE("instantiate activation units");
	const SaImmAdminOperationParamsT_2 **params = (const SaImmAdminOperationParamsT_2 **) new SaImmAdminOperationParamsT_2;
	params[0] = NULL;	//Null terminate the list of parameter pointers
	return callAdminOperation(SA_AMF_ADMIN_UNLOCK_INSTANTIATION, params, m_activationUnit.m_actedOn);
}

//------------------------------------------------------------------------------
// restartActivationUnits()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::restartActivationUnits()
{
	TRACE("restart activation units");
	const SaImmAdminOperationParamsT_2 **params = (const SaImmAdminOperationParamsT_2 **) new SaImmAdminOperationParamsT_2;
	params[0] = NULL;	//Null terminate the list of parameter pointers
	return callAdminOperation(SA_AMF_ADMIN_RESTART, params, m_activationUnit.m_actedOn);
}

//------------------------------------------------------------------------------
// modifyInformationModel()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::modifyInformationModel()
{
	if (m_modificationList.size() > 0) {
		TRACE("Modifying information model");
		SmfImmUtils immUtil;
		return immUtil.doImmOperations(m_modificationList);
	}

	TRACE("Nothing to modify in information model");
	return true;
}
//------------------------------------------------------------------------------
// setMaintenanceState()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::setMaintenanceState()
{
        TRACE_ENTER();

        bool rc = true;
	SmfImmUtils immUtil;
        std::list < std::string > suList;
        std::list < std::string >::iterator it;

        for (it = m_activationUnit.m_actedOn.begin(); it != m_activationUnit.m_actedOn.end(); ++it) {
                if ((*it).find("safAmfNode") == 0) { 
                        //If DN is a node, set saAmfSUMaintenanceCampaign for all SUs on the node
			// Find all SU's on the node
                        std::list < std::string > objectList;
                        SaImmAttrValuesT_2 **attributes;
			(void)immUtil.getChildren("", objectList, SA_IMM_SUBTREE, "SaAmfSU");

                        std::list < std::string >::const_iterator suit;
			for (suit = objectList.begin(); suit != objectList.end(); ++suit) {
				if (immUtil.getObject((*suit), &attributes) == true) {

                                        // Check if the found SU is hosted by the activation unit (node)
                                        const SaNameT *hostedByNode =
                                                immutil_getNameAttr((const SaImmAttrValuesT_2 **)attributes,
                                                                    "saAmfSUHostedByNode",
                                                                    0);
                                        if ((hostedByNode != NULL)
                                            && (strcmp((*it).c_str(), (char *)hostedByNode->value) == 0)) {
                                                /* The SU is hosted by the AU node */
                                                suList.push_back(*suit);
                                        }
				}
			}
                }else if ((*it).find("safSu") == 0) { 
                        //If DN is a SU, set saAmfSUMaintenanceCampaign for this SU only
                        suList.push_back(*it);
                }else if ((*it).find("safComp") == 0) {
                        //IF DN is a component, set saAmfSUMaintenanceCampaign for the hosting SU
                        //Extract the SU name from the DN
                        std::string su = ((*it).substr((*it).find(",") + 1, std::string::npos));
                        suList.push_back(su);
                } else {
                        LOG_ER("SmfUpgradeStep::setMaintenanceState(): unknown activation unit type %s", (*it).c_str());
                        TRACE_LEAVE();
                        return false;
                }
        }

        //Get the campaign DN
        const std::string campDn = SmfCampaignThread::instance()->campaign()->getDn();
        std::list < SmfImmOperation * > operations;

        //For all SUs, set saAmfSUMaintenanceCampaign attribute
        for (it = suList.begin(); it != suList.end(); ++it) {
                SmfImmModifyOperation *modop = new (std::nothrow) SmfImmModifyOperation;
                assert(modop != 0);
                modop->setDn(*it);
                modop->setOp("SA_IMM_ATTR_VALUES_REPLACE");
                SmfImmAttribute saAmfSUMaintenanceCampaign;
                saAmfSUMaintenanceCampaign.setName("saAmfSUMaintenanceCampaign");
                saAmfSUMaintenanceCampaign.setType("SA_IMM_ATTR_SANAMET");
                saAmfSUMaintenanceCampaign.addValue(campDn);
                modop->addValue(saAmfSUMaintenanceCampaign);
                operations.push_back(modop);
        }
        
        if (!immUtil.doImmOperations(operations)) {
                LOG_ER("SmfUpgradeStep::setMaintenanceState(), fails to set saAmfSUMaintenanceCampaign");
                rc = false;
        }

        //Delete the created SmfImmModifyOperation instances
        std::list < SmfImmOperation * > ::iterator operIter;
        for (operIter = operations.begin(); operIter != operations.end(); ++operIter) {
                delete (*operIter);
        }

        //Note:All saAmfSUMaintenanceCampaign settings shall be removed in the campaign wrap-up.

        TRACE_LEAVE();
	return rc;
}

//------------------------------------------------------------------------------
// createSaAmfNodeSwBundles()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::createSaAmfNodeSwBundles(const std::string & i_node)
{
	TRACE_ENTER();
	std::list < SmfBundleRef >::const_iterator bundleit;
	for (bundleit = m_swAddList.begin(); bundleit != m_swAddList.end(); ++bundleit) {
		if (i_node.length() > 0) {
			if (createOneSaAmfNodeSwBundle(i_node, *bundleit) == false) {
				TRACE_LEAVE();
				return false;
			}
		} else {
			std::list<std::string> swNodeList;
			if (!calculateSingleStepNodes(bundleit->getPlmExecEnvList(), swNodeList)) {
				TRACE_LEAVE();
				return false;
			}
			std::list<std::string>::const_iterator n;
			for (n = swNodeList.begin(); n != swNodeList.end(); n++) {
				if (createOneSaAmfNodeSwBundle(*n, *bundleit) == false) {
					TRACE_LEAVE();
					return false;
				}
			}
		}
	}

	TRACE_LEAVE();
	return true;
}

//------------------------------------------------------------------------------
// createOneSaAmfNodeSwBundle()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::createOneSaAmfNodeSwBundle(
	const std::string& i_node,
	const SmfBundleRef& i_bundle)
{
	TRACE_ENTER();
        SmfImmUtils immUtil;
	SaImmAttrValuesT_2 **attributes;

	//Check if object alredy exist
	std::string escapedDn = replaceAllCopy(i_bundle.getBundleDn(), ",", "\\,");
	std::string object = "safInstalledSwBundle=" + escapedDn + "," + i_node;
	if (immUtil.getObject(object, &attributes)) {
		// Object already exists
		TRACE_LEAVE();
		return true;
	}

	TRACE("SaAmfNodeSwBundle %s does not exist in Imm, create it", object.c_str());
	SmfImmCreateOperation icoSaAmfNodeSwBundle;

	icoSaAmfNodeSwBundle.setClassName("SaAmfNodeSwBundle");
	icoSaAmfNodeSwBundle.setParentDn(i_node);

	SmfImmAttribute attrsafInstalledSwBundle;
	attrsafInstalledSwBundle.setName("safInstalledSwBundle");
	attrsafInstalledSwBundle.setType("SA_IMM_ATTR_SANAMET");
	attrsafInstalledSwBundle.addValue("safInstalledSwBundle=" + escapedDn);
	icoSaAmfNodeSwBundle.addValue(attrsafInstalledSwBundle);

	SmfImmAttribute attrsaAmfNodeSwBundlePathPrefix;
	attrsaAmfNodeSwBundlePathPrefix.setName("saAmfNodeSwBundlePathPrefix");
	attrsaAmfNodeSwBundlePathPrefix.setType("SA_IMM_ATTR_SASTRINGT");
	attrsaAmfNodeSwBundlePathPrefix.addValue(i_bundle.getPathNamePrefix());
	icoSaAmfNodeSwBundle.addValue(attrsaAmfNodeSwBundlePathPrefix);

	std::list < SmfImmOperation * > immOperations;
	immOperations.push_back(&icoSaAmfNodeSwBundle);
	if (immUtil.doImmOperations(immOperations) != SA_AIS_OK) {
		TRACE_LEAVE();
		return false;
	}

	TRACE_LEAVE();
	return true;
}

//------------------------------------------------------------------------------
// deleteSaAmfNodeSwBundle()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::deleteOneSaAmfNodeSwBundle(
	const std::string & i_node,
	const SmfBundleRef& i_bundle)
{
	TRACE_ENTER();

        SmfImmUtils immUtil;
	SaImmAttrValuesT_2 **attributes;
	std::string bundleDn = i_bundle.getBundleDn();

	//Check if object alredy exist
	std::string escapedDn = replaceAllCopy(bundleDn, ",", "\\,");
	std::string object = "safInstalledSwBundle=" + escapedDn + "," + i_node;
	if (immUtil.getObject(object, &attributes) == true) {
		TRACE("SaAmfNodeSwBundle %s exist in Imm, delete it", object.c_str());
		SmfImmDeleteOperation idoSaAmfNodeSwBundle;

		idoSaAmfNodeSwBundle.setDn(object);
		std::list < SmfImmOperation * > immOperations;
		immOperations.push_back(&idoSaAmfNodeSwBundle);

		TRACE("immUtil.doImmOperations");
		if (immUtil.doImmOperations(immOperations) != SA_AIS_OK) {
			TRACE_LEAVE();
			return false;
		}
	}

	TRACE_LEAVE();

	return true;
}

//------------------------------------------------------------------------------
// deleteSaAmfNodeSwBundle()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::deleteSaAmfNodeSwBundles(const std::string & i_node)
{
	TRACE_ENTER();
	std::list < SmfBundleRef >::const_iterator bundleit;
 
	for (bundleit = m_swRemoveList.begin(); bundleit != m_swRemoveList.end(); ++bundleit) {
		if (i_node.length() > 0) {
			if (deleteOneSaAmfNodeSwBundle(i_node, *bundleit) == false) {
				TRACE_LEAVE();
				return false;
			}
		} else {
			std::list<std::string> swNodeList;
			if (!calculateSingleStepNodes(bundleit->getPlmExecEnvList(), swNodeList)) {
				TRACE_LEAVE();
				return false;
			}
			std::list<std::string>::const_iterator n;
			for (n = swNodeList.begin(); n != swNodeList.end(); n++) {
				if (deleteOneSaAmfNodeSwBundle(*n, *bundleit) == false) {
					TRACE_LEAVE();
					return false;
				}
			}
                }
        }

	TRACE_LEAVE();

	return true;
}

//------------------------------------------------------------------------------
// calculateSingleStepNodes()
//------------------------------------------------------------------------------
bool SmfUpgradeStep::calculateSingleStepNodes(
	std::list<SmfPlmExecEnv> const& i_plmExecEnvList,
	std::list<std::string>& o_nodelist)
{
	o_nodelist = m_swNodeList;
	std::list<SmfPlmExecEnv>::const_iterator ee;
	for (ee = i_plmExecEnvList.begin(); ee != i_plmExecEnvList.end(); ee++) {
		std::string const& amfnode = ee->getAmfNode();
		if (amfnode.length() == 0) {
			LOG_ER("Only AmfNodes can be handled in plmExecEnv");
			return false;
		}
		o_nodelist.push_back(amfnode);
	}

	/* The m_swNodeList itself may contain duplicates and combined with the PlmExecEnv list
	   duplicates are almost certain. */
	o_nodelist.sort();
	o_nodelist.unique();
	return true;
}

//------------------------------------------------------------------------------
// setSwitchOver()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setSwitchOver(bool i_switchover)
{
        m_switchOver = i_switchover;
}

//------------------------------------------------------------------------------
// getSwitchOver()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::getSwitchOver()
{
        return m_switchOver;
}

//------------------------------------------------------------------------------
// callBundleScript()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::callBundleScript(SmfInstallRemoveT i_order,
                                 const std::list < SmfBundleRef > &i_bundleList,
				 const std::string & i_node)
{
	std::list < SmfBundleRef >::const_iterator bundleit;
	std::string command;
	std::string cmdAttr;
	std::string argsAttr;
	SaImmAttrValuesT_2 **attributes;
	const char *cmd = NULL;
	const char *args = NULL;
	bool result = true;
	SaTimeT timeout = smfd_cb->cliTimeout;	/* Default timeout */
	SmfImmUtils immUtil;

	TRACE_ENTER();

	for (bundleit = i_bundleList.begin(); bundleit != i_bundleList.end(); ++bundleit) {
		/* Get bundle object from IMM */
		if (immUtil.getObject((*bundleit).getBundleDn(), &attributes) == false) {
			return false;
		}

		TRACE("Found bundle %s in Imm", (*bundleit).getBundleDn().c_str());

		switch (i_order) {
		case SMF_STEP_OFFLINE_INSTALL:
			{
				/* Find offline install script name and args */
				cmdAttr = "saSmfBundleInstallOfflineCmdUri";
				argsAttr = "saSmfBundleInstallOfflineCmdArgs";
				break;
			}
		case SMF_STEP_ONLINE_INSTALL:
			{
				/* Find online install script name and args */
				cmdAttr = "saSmfBundleInstallOnlineCmdUri";
				argsAttr = "saSmfBundleInstallOnlineCmdArgs";
				break;
			}
		case SMF_STEP_OFFLINE_REMOVE:
			{
				/* Find offline remove script name and args */
				cmdAttr = "saSmfBundleRemoveOfflineCmdUri";
				argsAttr = "saSmfBundleRemoveOfflineCmdArgs";
				break;
			}
		case SMF_STEP_ONLINE_REMOVE:
			{
				/* Find online remove script name and args */
				cmdAttr = "saSmfBundleRemoveOnlineCmdUri";
				argsAttr = "saSmfBundleRemoveOnlineCmdArgs";
				break;
			}
		}

		/* Get cmd attribute */
		cmd = immutil_getStringAttr((const SaImmAttrValuesT_2 **)
					    attributes, cmdAttr.c_str(), 0);
		if (cmd == NULL) {
			LOG_NO("callBundleScript cmd %s not found for %s", cmdAttr.c_str(),
			       (*bundleit).getBundleDn().c_str());
			/* cmd is not a must so don't set result = false */
			goto done;
		}
		command = cmd;

		/* Get args attribute */
		args = immutil_getStringAttr((const SaImmAttrValuesT_2 **)
					     attributes, argsAttr.c_str(), 0);
		if (args != NULL) {
			command += " ";
			command += args;
		}

		/* Get timeout attribute */
		const SaTimeT *defaultTimeout = immutil_getTimeAttr((const SaImmAttrValuesT_2 **)attributes,
								    "saSmfBundleDefaultCmdTimeout",
								    0);
		if (defaultTimeout != NULL) {
			timeout = *defaultTimeout;
		}

		if (i_node.length() == 0) {

			/* In the single-step upgrade the nodes for
			   the bundle is provided in the m_swNodeList
			   and the PlmExecEnv list. The step itself is
			   not bound to a particular node, so the
			   "i_node" will be empty. */

			std::list<std::string> swNodeList;
			if (!calculateSingleStepNodes(bundleit->getPlmExecEnvList(), swNodeList)) {
				result = false;
				goto done;					
			}

			std::list<std::string>::const_iterator n;
			for (n = swNodeList.begin(); n != swNodeList.end(); n++) {
				char const* nodeName = n->c_str();
				TRACE("Executing bundle script '%s' on node '%s' (single-step)", 
				      command.c_str(), nodeName);
				MDS_DEST nodeDest = getNodeDestination(*n);
				if (nodeDest == 0) {
					LOG_ER("no node destination found for node [%s]", nodeName);
					result = false;
					goto done;
				}
				int rc = smfnd_remote_cmd(command.c_str(), nodeDest, timeout / 10000000);
				if (rc != 0) {
					LOG_ER("executing command '%s' on node '%s' failed with rc %d", 
					       command.c_str(), nodeName, rc);
					result = false;
					goto done;
				}
			}

		} else {

			TRACE("Executing bundle script '%s' on node '%s'", 
			      command.c_str(), i_node.c_str());
			MDS_DEST nodeDest = getNodeDestination(i_node);
			if (nodeDest == 0) {
				LOG_ER("no node destination found for node %s", i_node.c_str());
				result = false;
				goto done;
			}

			/* TODO : how to handle the case where the cmd is an URI */
			/* First fetch the script using e.g. wget etc */

			/* Execute the bundle script remote on node */
			int rc = smfnd_remote_cmd(command.c_str(), nodeDest, timeout / 10000000);
			/* convert ns to 10 ms timeout */
			if (rc != 0) {
				LOG_ER("executing command '%s' on node '%s' failed with rc %d", 
				       command.c_str(), i_node.c_str(), rc);
				result = false;
				goto done;
			}
		}
	}
 done:

	TRACE_LEAVE();
	return result;
}

//------------------------------------------------------------------------------
// callAdminOperation()
//------------------------------------------------------------------------------
bool 
SmfUpgradeStep::callAdminOperation(unsigned int i_operation,
                                   const SaImmAdminOperationParamsT_2 ** i_params,
                                   const std::list < std::string > &i_dnList)
{
	std::list < std::string >::const_iterator dnit;
	SmfImmUtils immUtil;

	TRACE_ENTER();

	for (dnit = i_dnList.begin(); dnit != i_dnList.end(); ++dnit) {
		if (immUtil.callAdminOperation((*dnit), i_operation, i_params, smfd_cb->adminOpTimeout) == false) {
			LOG_ER("Failed to call admin operation %u on %s", i_operation, (*dnit).c_str());
			return false;
		}
	}

	TRACE_LEAVE();
	return true;
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
SmfStepResultT 
SmfUpgradeStep::execute()
{
	SmfStepResultT stepResult = SMF_STEP_COMPLETED;

	TRACE_ENTER();

	m_state->execute(this);

        if (m_switchOver == true) {
                TRACE("Step ordered switch over");
		stepResult = SMF_STEP_SWITCHOVER;
        } else {

                TRACE("Step state after executing %u", m_state->getState());

                switch (m_state->getState()) {
                case SA_SMF_STEP_COMPLETED:
                        stepResult = SMF_STEP_COMPLETED;
                        break;
                default:
                        stepResult = SMF_STEP_FAILED;
                        break;
                }

        }
	TRACE_LEAVE();
	return stepResult;
}

//------------------------------------------------------------------------------
// setProcedure()
//------------------------------------------------------------------------------
void 
SmfUpgradeStep::setProcedure(SmfUpgradeProcedure * i_procedure)
{
	m_procedure = i_procedure;
}
