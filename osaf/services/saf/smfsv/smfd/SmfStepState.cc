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
#include "SmfUpgradeStep.hh"
#include "SmfStepState.hh"
#include "SmfUtils.hh"
#include "immutil.h"

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
// ------Base class SmfStepState------------------------------------------------
//
// SmfStepState default implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
std::string 
SmfStepState::getClassName()const
{
	return "SmfStepState";
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
bool 
SmfStepState::execute(SmfUpgradeStep * i_step)
{
	TRACE_ENTER();
	LOG_ER("SmfStepState::execute default implementation, should NEVER be executed.");
	TRACE_LEAVE();
	return false;
}

//------------------------------------------------------------------------------
// changeState()
//------------------------------------------------------------------------------
void 
SmfStepState::changeState(SmfUpgradeStep * i_step, SmfStepState * i_state)
{
	TRACE_ENTER();
	TRACE("SmfStepState::changeState");

	std::string newState = i_state->getClassName();
	std::string oldState = i_step->m_state->getClassName();

	TRACE("SmfStepState::changeState old state=%s , new state=%s", oldState.c_str(), newState.c_str());
	i_step->changeState(i_state);

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfStepStateInitial implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

SmfStepState *SmfStepStateInitial::s_instance = NULL;

SmfStepState *
SmfStepStateInitial::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfStepStateInitial;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
std::string 
SmfStepStateInitial::getClassName()const
{
	return "SmfStepStateInitial";
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
bool 
SmfStepStateInitial::execute(SmfUpgradeStep * i_step)
{
	TRACE_ENTER();

	TRACE("Start executing upgrade step %s", i_step->getDn().c_str());

        /* Find out what type of step this is */
        const std::list <std::string> &auList = i_step->getActivationUnitList();

        if (auList.empty()) {
                /* No AU in the step (only install/remove bundles) */
		if (i_step->getDeactivationUnitList().empty()) {
			i_step->setStepType(SMF_STEP_SW_INSTALL);
		} else {
			/* We have deactivation units. This must be a
			 * single-step removal package. */
			i_step->setStepType(SMF_STEP_AU_LOCK);
		}
        } else {
		bool rebootNeeded = false;
                SmfImmUtils immutil;
                const std::string& firstAU = auList.front();
                SaImmAttrValuesT_2 ** attributes;

		/* Check if any bundle requires node reboot, in such case only AU = node is accepted */
		TRACE("SmfStepStateInitial::execute:Check if any software bundle requires reboot");
		const std::list < SmfBundleRef > &removeList = i_step->getSwRemoveList();
		std::list< SmfBundleRef >::const_iterator bundleIter;
		bundleIter = removeList.begin();
		while (bundleIter != removeList.end()) {
			/* Read the saSmfBundleRemoveOfflineScope to detect if the bundle requires reboot */
			if (immutil.getObject((*bundleIter).getBundleDn(), &attributes) == false) {
				LOG_ER("Could not find software bundle  %s", (*bundleIter).getBundleDn().c_str());
				changeState(i_step, SmfStepStateFailed::instance());
				TRACE_LEAVE();
				return false;
			}
			const SaUint32T* scope = immutil_getUint32Attr((const SaImmAttrValuesT_2 **)attributes, 
								       "saSmfBundleRemoveOfflineScope",
								       0);

			if ((scope != NULL) && (*scope == SA_SMF_CMD_SCOPE_PLM_EE)) {
				TRACE("SmfStepStateInitial::execute:The SW bundle %s requires reboot to remove", 
				      (*bundleIter).getBundleDn().c_str());

				rebootNeeded = true;
				break;
			}
			bundleIter++;
		}

		const std::list < SmfBundleRef > &addList = i_step->getSwAddList();
		bundleIter = addList.begin();
		while (bundleIter != addList.end()) {
			/* Read the saSmfBundleInstallOfflineScope to detect if the bundle requires reboot */
			if (immutil.getObject((*bundleIter).getBundleDn(), &attributes) == false) {
				LOG_ER("Could not find software bundle  %s", (*bundleIter).getBundleDn().c_str());
				changeState(i_step, SmfStepStateFailed::instance());
				TRACE_LEAVE();
				return false;
			}
			const SaUint32T* scope = immutil_getUint32Attr((const SaImmAttrValuesT_2 **)attributes, 
								       "saSmfBundleInstallOfflineScope",
								       0);

			if ((scope != NULL) && (*scope == SA_SMF_CMD_SCOPE_PLM_EE)) {
				TRACE("SmfStepStateInitial::execute:The SW bundle %s requires reboot to install", 
				      (*bundleIter).getBundleDn().c_str());

				rebootNeeded = true;
				break;
			}

			bundleIter++;
		}

                /* Check type of AU object. We assume all AU has to be of same type so only check first */ 
                if (immutil.getObject(firstAU, &attributes) == false) {
                        LOG_ER("Could not find AU %s", firstAU.c_str());
                        changeState(i_step, SmfStepStateFailed::instance());
			TRACE_LEAVE();
			return false;
                }

                const char * className = immutil_getStringAttr((const SaImmAttrValuesT_2 **)attributes, SA_IMM_ATTR_CLASS_NAME, 0);
                if (className == NULL) {
                        LOG_ER("class name not found for AU %s", firstAU.c_str());
                        changeState(i_step, SmfStepStateFailed::instance());
			TRACE_LEAVE();
                        return false;
                }

                if (strcmp(className, "SaAmfNode") == 0) {
                        /* AU is AMF node */

			if (rebootNeeded) {

				/* TODO Check if the step will lock/reboot our own node and if so
				   move our campaign execution to the other controller using e.g. 
				   admin operation SWAP on the SI we belong to. Then the other
				   controller will continue with this step and do the lock/reboot */

				/* if (AU == our_own_node) {
				   i_step->setSwitchOver(true); 
				   return true; 
				   } */

				i_step->setStepType(SMF_STEP_NODE_REBOOT);
			} else {
				i_step->setStepType(SMF_STEP_AU_LOCK);
			}

                } else if (strcmp(className, "SaAmfSU") == 0) {
                        /* AU is SU */
			if (rebootNeeded) {
				LOG_ER("A software bundle requires reboot but the AU is a SU (%s)", firstAU.c_str());
				changeState(i_step, SmfStepStateFailed::instance());
				TRACE_LEAVE();
				return false;
			}

                        i_step->setStepType(SMF_STEP_AU_LOCK);
                } else if (strcmp(className, "SaAmfComp") == 0) {
                        /* AU is Component */
			if (rebootNeeded) {
				LOG_ER("A software bundle requires reboot but the AU is a Component (%s)", firstAU.c_str());
				changeState(i_step, SmfStepStateFailed::instance());
				TRACE_LEAVE();
				return false;
			}

                        i_step->setStepType(SMF_STEP_AU_RESTART);
                } else {
                        LOG_ER("class name %s for %s unknown as AU", className, firstAU.c_str());
                        changeState(i_step, SmfStepStateFailed::instance());
			TRACE_LEAVE();
                        return false;
                }
        }

	changeState(i_step, SmfStepStateExecuting::instance());
	if (i_step->execute() == false) {
		TRACE_LEAVE();
		return false;
	}

	TRACE_LEAVE();
	return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfStepStateExecuting implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SmfStepState *SmfStepStateExecuting::s_instance = NULL;

SmfStepState *
SmfStepStateExecuting::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfStepStateExecuting;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
std::string 
SmfStepStateExecuting::getClassName()const
{
	return "SmfStepStateExecuting";
}

//------------------------------------------------------------------------------
// execute()
//------------------------------------------------------------------------------
bool 
SmfStepStateExecuting::execute(SmfUpgradeStep * i_step)
{

	TRACE_ENTER();
	TRACE("Executing step %s", i_step->getDn().c_str());
	bool rc = true;

        switch (i_step->getStepType()) {
        case SMF_STEP_SW_INSTALL:
                {
                        if (executeSwInstall(i_step) == false){
				LOG_ER("executeSwInstall failed");
				rc = false;
			}
                        break;
                }
        case SMF_STEP_AU_LOCK:
                {
                        if (executeAuLock(i_step) == false){
				LOG_ER("executeAuLock failed");
				rc = false;
			}
                        break;
                }
        case SMF_STEP_AU_RESTART:
                {
                        if (executeAuRestart(i_step) == false){
				LOG_ER("executeAuRestart failed");
				rc = false;
			}
                        break;
                }
        case SMF_STEP_NODE_REBOOT:
                {
                        if (executeNodeReboot(i_step) == false){
				LOG_ER("executeNodeReboot failed");
				rc = false;
			}
                        break;
                }
        default:
                {
                        LOG_ER("Unknown step type %d", i_step->getStepType());
                        changeState(i_step, SmfStepStateFailed::instance());
			rc = false;
                        break;
                }
        }

	return rc;

	TRACE_LEAVE();
}

//------------------------------------------------------------------------------
// executeSwInstall()
//------------------------------------------------------------------------------
bool 
SmfStepStateExecuting::executeSwInstall(SmfUpgradeStep * i_step)
{

	TRACE_ENTER();
	LOG_NO("STEP: Executing SW install step %s", i_step->getDn().c_str());

	/* Online installation of new software */
	LOG_NO("STEP: Online installation of new software");
	if (i_step->onlineInstallBundles(i_step->getSwNode()) == false) {
		LOG_ER("Failed to online install bundles");
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}

        /* Offline uninstallation of old software */
        LOG_NO("STEP: Offline uninstallation of old software");
        if (i_step->offlineRemoveBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to offline remove bundles");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
	}

        /* Offline installation of new software */
        LOG_NO("STEP: Offline installation of new software");
        if (i_step->offlineInstallBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to offline install bundles");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Create SaAmfNodeSwBundle object */
        LOG_NO("STEP: Create SaAmfNodeSwBundle object");
        if (i_step->createSaAmfNodeSwBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to create SaAmfNodeSwBundle object");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

#if 0
//Moved to the procedure
	/* Online uninstallation of old software */
	LOG_NO("STEP: Online uninstallation of old software");
	if (i_step->onlineRemoveBundles(i_step->getSwNode()) == false) {
		LOG_ER("Failed to online remove bundles");
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}

        /* Delete SaAmfNodeSwBundle object */
        LOG_NO("STEP: Delete SaAmfNodeSwBundle object");
        if (i_step->deleteSaAmfNodeSwBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to delete SaAmfNodeSwBundle object");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }
#endif

	LOG_NO("STEP: Upgrade SW install step completed %s", i_step->getDn().c_str());
	changeState(i_step, SmfStepStateCompleted::instance());
	TRACE_LEAVE();

	return true;
}

//------------------------------------------------------------------------------
// executeAuLock()
//------------------------------------------------------------------------------
bool 
SmfStepStateExecuting::executeAuLock(SmfUpgradeStep * i_step)
{

	TRACE_ENTER();
	LOG_NO("STEP: Executing AU lock step %s", i_step->getDn().c_str());

	/* Online installation of new software */
	LOG_NO("STEP: Online installation of new software");
	if (i_step->onlineInstallBundles(i_step->getSwNode()) == false) {
		LOG_ER("Failed to online install new bundles in step=%s",i_step->getRdn().c_str());
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}

        /* Lock deactivation units */
        LOG_NO("STEP: Lock deactivation units");
        if (i_step->lockDeactivationUnits() == false) {
                LOG_ER("Failed to Lock deactivation units in step=%s",i_step->getRdn().c_str());
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Terminate deactivation units */
        LOG_NO("STEP: Terminate deactivation units");
        if (i_step->terminateDeactivationUnits() == false) {
                LOG_ER("Failed to Terminate deactivation units in step=%s",i_step->getRdn().c_str());
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Offline uninstallation of old software */
        LOG_NO("STEP: Offline uninstallation of old software");
        if (i_step->offlineRemoveBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to offline remove old bundles in step=%s",i_step->getRdn().c_str());
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
	}

	/* Modify information model and set maintenance status */
	LOG_NO("STEP: Modify information model and set maintenance status");
	if (i_step->modifyInformationModel() == false) {
		LOG_ER("Failed to Modify information model in step=%s",i_step->getRdn().c_str());
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}
//TODO: Remove comments when AMF is updated
#warning "AMF_FAULTY: Remove comments when AMF is updated, setMaintenanceState()"
#if 0
	if (i_step->setMaintenanceState() == false) {
                LOG_ER("Failed to set maintenance state in step=%s",i_step->getRdn().c_str());
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
	}
#endif
        /* Offline installation of new software */
        LOG_NO("STEP: Offline installation of new software");
        if (i_step->offlineInstallBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to offline install new software in step=%s",i_step->getRdn().c_str());
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Create SaAmfNodeSwBundle object */
        LOG_NO("STEP: Create SaAmfNodeSwBundle object");
        if (i_step->createSaAmfNodeSwBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to create SaAmfNodeSwBundle object in step=%s",i_step->getRdn().c_str());
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Instantiate activation units */
        LOG_NO("STEP: Instantiate activation units");
        if (i_step->instantiateActivationUnits() == false) {
                LOG_ER("Failed to Instantiate activation units in step=%s",i_step->getRdn().c_str());
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Unlock activation units */
        LOG_NO("STEP: Unlock activation units");
        if (i_step->unlockActivationUnits() == false) {
                LOG_ER("Failed to Unlock activation units in step=%s",i_step->getRdn().c_str());
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }
#if 0
//Moved to the procedure
	/* Online uninstallation of old software */
	LOG_NO("STEP: Online uninstallation of old software");
	if (i_step->onlineRemoveBundles(i_step->getSwNode()) == false) {
		LOG_ER("Failed to online remove bundles in step=%s",i_step->getRdn().c_str());
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}

        /* Delete SaAmfNodeSwBundle object */
        LOG_NO("STEP: Delete SaAmfNodeSwBundle object");
        if (i_step->deleteSaAmfNodeSwBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to delete SaAmfNodeSwBundle object in step=%s",i_step->getRdn().c_str());
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }
#endif

	LOG_NO("STEP: Upgrade AU lock step completed %s", i_step->getDn().c_str());
	changeState(i_step, SmfStepStateCompleted::instance());
	TRACE_LEAVE();

	return true;
}

//------------------------------------------------------------------------------
// executeAuRestart()
//------------------------------------------------------------------------------
bool 
SmfStepStateExecuting::executeAuRestart(SmfUpgradeStep * i_step)
{
        //The step actions below are executed for steps containing
        //restartable activation units i.e. restartable components.
	TRACE_ENTER();
	LOG_NO("STEP: Executing AU restart step %s", i_step->getDn().c_str());

	/* Online installation of new software */
	LOG_NO("STEP: Online installation of new software");
	if (i_step->onlineInstallBundles(i_step->getSwNode()) == false) {
		LOG_ER("Failed to online install bundles");
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}

        /* Create SaAmfNodeSwBundle object */
        LOG_NO("STEP: Create SaAmfNodeSwBundle object");
        if (i_step->createSaAmfNodeSwBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to create SaAmfNodeSwBundle object");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

	/* Modify information model and set maintenance status */
	LOG_NO("STEP: Modify information model and set maintenance status");
	if (i_step->modifyInformationModel() == false) {
		LOG_ER("Failed to Modify information model");
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}
//TODO: Shall maintenance status be set here for restartable units ??

        /* Restartable activation units, restart them */
        LOG_NO("STEP: Restart activation units");
        if (i_step->restartActivationUnits() == false) {
                LOG_ER("Failed to Restart activation units");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

#if 0
//Moved to the procedure
        /* Online uninstallation of old software */
        LOG_NO("STEP: Online uninstallation of old software");
        if (i_step->onlineRemoveBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to online remove bundles");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Delete SaAmfNodeSwBundle object */
        LOG_NO("STEP: Delete SaAmfNodeSwBundle object");
        if (i_step->deleteSaAmfNodeSwBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to delete SaAmfNodeSwBundle object");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }
#endif

	LOG_NO("STEP: Upgrade AU restart step completed %s", i_step->getDn().c_str());
	changeState(i_step, SmfStepStateCompleted::instance());
	TRACE_LEAVE();

	return true;
}

//------------------------------------------------------------------------------
// executeNodeReboot()
//------------------------------------------------------------------------------
bool 
SmfStepStateExecuting::executeNodeReboot(SmfUpgradeStep * i_step)
{
	TRACE_ENTER();
	LOG_NO("STEP: Executing node reboot step %s", i_step->getDn().c_str());

	/* All online installations are performed here regardless if the bundles needs reboot or not */
	/* Online installation of all new software */
	LOG_NO("STEP: Online installation of new software (no reboot required)");
	if (i_step->onlineInstallBundles(i_step->getSwNode()) == false) {
		LOG_ER("Failed to online install bundles");
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}

        /* Lock deactivation units */
        LOG_NO("STEP: Lock deactivation units");
        if (i_step->lockDeactivationUnits() == false) {
                LOG_ER("Failed to Lock deactivation units");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Terminate deactivation units */
        LOG_NO("STEP: Terminate deactivation units");
        if (i_step->terminateDeactivationUnits() == false) {
                LOG_ER("Failed to Terminate deactivation units");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Offline uninstallation of old software (no reboot required) */
        LOG_NO("STEP: Offline uninstallation of old software (no reboot required)");
        if (i_step->offlineRemoveBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to offline remove bundles");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
	}

	/* Modify information model and set maintenance status */
	LOG_NO("STEP: Modify information model and set maintenance status");
	if (i_step->modifyInformationModel() == false) {
		LOG_ER("Failed to Modify information model");
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}

        /* TODO: Reboot node */

        /* TODO: Node is up again */

	/* Online uninstallation of old software (reboot required) */
	LOG_NO("STEP: Online uninstallation of old software (reboot required)");
	if (i_step->onlineRemoveBundles(i_step->getSwNode()) == false) {
		LOG_ER("Failed to online remove bundles");
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}

        /* Offline installation of new software (no reboot required) */
        LOG_NO("STEP: Offline installation of new software (no reboot required)");
        if (i_step->offlineInstallBundles(i_step->getSwNode()) == false) {
                LOG_ER("Failed to offline install bundles");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Instantiate activation units */
        LOG_NO("STEP: Instantiate activation units");
        if (i_step->instantiateActivationUnits() == false) {
                LOG_ER("Failed to Instantiate activation units");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

        /* Unlock activation units */
        LOG_NO("STEP: Unlock activation units");
        if (i_step->unlockActivationUnits() == false) {
                LOG_ER("Failed to Unlock activation units");
                changeState(i_step, SmfStepStateFailed::instance());
                return false;
        }

#if 0
//Moved to the procedure

	/* Online uninstallation of old software (no reboot required) */
	LOG_NO("STEP: Online uninstallation of old software (no reboot required)");
	if (i_step->onlineRemoveBundles(i_step->getSwNode()) == false) {
		LOG_ER("Failed to online remove bundles");
		changeState(i_step, SmfStepStateFailed::instance());
		return false;
	}
#endif

	LOG_NO("STEP: Upgrade node reboot step completed %s", i_step->getDn().c_str());
	changeState(i_step, SmfStepStateCompleted::instance());
	TRACE_LEAVE();

	return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfStepStateCompleted implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SmfStepState *SmfStepStateCompleted::s_instance = NULL;

SmfStepState *
SmfStepStateCompleted::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfStepStateCompleted;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
std::string 
SmfStepStateCompleted::getClassName()const
{
	return "SmfStepStateCompleted";
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SmfStepStateFailed implementations
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SmfStepState *SmfStepStateFailed::s_instance = NULL;

SmfStepState *
SmfStepStateFailed::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfStepStateFailed;
	}

	return s_instance;
}

//------------------------------------------------------------------------------
// getClassName()
//------------------------------------------------------------------------------
std::string 
SmfStepStateFailed::getClassName()const
{
	return "SmfStepStateFailed";
}
