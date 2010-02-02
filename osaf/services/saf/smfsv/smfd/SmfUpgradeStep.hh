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

#ifndef SMFUPGRADESTEP_HH
#define SMFUPGRADESTEP_HH

/* ========================================================================
 *   INCLUDE FILES
 * ========================================================================
 */

#include <string>
#include <vector>
#include <list>

#include <saSmf.h>
#include <saImmOi.h>
#include "SmfStepState.hh"
#include "SmfTargetTemplate.hh"

/* ========================================================================
 *   DEFINITIONS
 * ========================================================================
 */

class SmfUpgradeProcedure;

/* ========================================================================
 *   TYPE DEFINITIONS
 * ========================================================================
 */

typedef enum {
	SMF_STEP_COMPLETED = 1,
	SMF_STEP_SWITCHOVER = 2,
	SMF_STEP_FAILED = 3
} SmfStepResultT;

typedef enum {
	SMF_STEP_UNKNOWN = 1,
	SMF_STEP_SW_INSTALL = 2,
	SMF_STEP_AU_LOCK = 3,
	SMF_STEP_AU_RESTART = 4,
	SMF_STEP_NODE_REBOOT = 5
} SmfStepT;


/* ========================================================================
 *   DATA DECLARATIONS
 * ========================================================================
 */

///------------------------------------------------------------------------------
/// Purpose: Class for activation unit
///------------------------------------------------------------------------------
class SmfActivationUnit {
 public:

///
/// Purpose:  Constructor
/// @param    -
/// @return   None
///
      SmfActivationUnit() 
      {
      }
///
/// Purpose:  Destructor
/// @param    None
/// @return   None
/// 
      ~SmfActivationUnit() 
      {
      }

      std::list < std::string > m_actedOn;
};

///
/// Purpose: Class for upgrade step
///
class SmfUpgradeStep {
 public:

///
/// Purpose:  Constructor
/// @param    -
/// @return   None
///
	SmfUpgradeStep();

///
/// Purpose:  Destructor
/// @param    None
/// @return   None
///
	~SmfUpgradeStep();

///
/// Purpose:  Init attributes from IMM object
/// @param    Imm attributes
/// @return   SA_AIS_OK on OK or error code
///
	SaAisErrorT init(const SaImmAttrValuesT_2 ** attrValues);

///
/// Purpose:  Get current step state
/// @param    None
/// @return   A pointer to the state
///
	SaSmfStepStateT getState() const;

///
/// Purpose:  Set the name of the step
/// @param    A string specifying the name of the step
/// @return   None
///
	void setRdn(const std::string & i_rdn);

///
/// Purpose:  Get the name of the step
/// @param    -
/// @return   A string specifying the name of the step
///
	const std::string & getRdn() const;

///
/// Purpose:  Set the DN of the step
/// @param    -
/// @return   -
///
	void setDn(const std::string & i_dn);

///
/// Purpose:  Get the DN of the step
/// @param    -
/// @return   -
///
	const std::string & getDn() const;

///
/// Purpose:  Set the max retry
/// @param    The max retry
/// @return   None
///
	void setMaxRetry(unsigned int i_maxRetry);

///
/// Purpose:  Get the max retry
/// @param    -
/// @return   The max retry
///
	unsigned int getMaxRetry() const;

///
/// Purpose:  Set the retry count
/// @param    The retry count
/// @return   None
///
	void setRetryCount(unsigned int i_retryCount);

///
/// Purpose:  Get the retry count
/// @param    -
/// @return   The retry count
///
	unsigned int getRetryCount() const;

///
/// Purpose:  Set the restart option
/// @param    The restart option
/// @return   None
///
	void setRestartOption(unsigned int i_restartOption);

///
/// Purpose:  Get the restart option
/// @param    -
/// @return   The restart option
///
	unsigned int getRestartOption() const;

///
/// Purpose:  Execute the upgrade step
/// @param    None
/// @return   None
///
	SmfStepResultT execute();

///
/// Purpose:  Set the procedure
/// @param    A ptr to our procedure 
/// @return   None
///
	void setProcedure(SmfUpgradeProcedure * i_procedure);

///
/// Purpose:  Add an activation unit DN
/// @param    A DN to an activation unit 
/// @return   None
///
	void addActivationUnit(const std::string & i_activationUnit);

///
/// Purpose:  Get the activation unit DN
/// @param     
/// @return   A list of DN to activation units
///
	const std::list < std::string > &getActivationUnitList();

///
/// Purpose:  Add an deactivation unit DN
/// @param    A DN to an deactivation unit 
/// @return   None
///
	void addDeactivationUnit(const std::string & i_deactivationUnit);

///
/// Purpose:  Get the deactivation unit DN
/// @param     
/// @return   A list of DN to deactivation units
///
	const std::list < std::string > &getDeactivationUnitList();

///
/// Purpose:  Add a sw bundle to remove
/// @param    A DN to a bundle 
/// @return   None
///
	void addSwRemove(const std::list < SmfBundleRef * >&i_swRemove);
	void addSwRemove(std::list<SmfBundleRef> const& i_swRemove);

///
/// Purpose:  Get the sw remove list
/// @param     
/// @return   A list of bundles to remove
///
	const std::list < SmfBundleRef > &getSwRemoveList();

///
/// Purpose:  Add a sw bundle to add
/// @param    A DN to a bundle 
/// @return   None
///
	void addSwAdd(const std::list < SmfBundleRef* >&i_swAdd);
	void addSwAdd(std::list<SmfBundleRef> const& i_swAdd);

///
/// Purpose:  Get the sw add list
/// @param     
/// @return   A list of bundles to add
///
	const std::list < SmfBundleRef > &getSwAddList();

///
/// Purpose:  Add a modification
/// @param    A Imm modification 
/// @return   None
///
	void addModification(SmfImmModifyOperation * i_modification);

///
/// Purpose:  Get modifications
/// @param    None
/// @return   A list of pointers to SmfImmOperation objects
///
	std::list < SmfImmOperation * >& getModifications();

/// Purpose:  Add an Imm operation (add/remove/modify)
/// @param    i_immoperation A Imm operation (free'ed by this object!)
/// @return   None
///
	void addImmOperation(SmfImmOperation* i_immoperation);

///
/// Purpose:  Set the node where sw should be added/removed
/// @param    A DN to an AMF node 
/// @return   None
///
	void setSwNode(const std::string & i_swNode);

///
/// Purpose:  Get the node where sw should be added/removed
/// @param     
/// @return   A DN to an AMF node
///
	const std::string & getSwNode();

///
/// Purpose:  Set type of step
/// @param    type 
/// @return   None
///
	void setStepType(SmfStepT i_type);

///
/// Purpose:  Get type of step
/// @param     
/// @return   type
///
	SmfStepT getStepType();

///
/// Purpose:  Offline install bundles
/// @param    on what node to offline install bundles
/// @return   true on success else false
///
	bool offlineInstallBundles(const std::string & i_node);

///
/// Purpose:  Online install bundles
/// @param    on what node to online install bundles
/// @return   true on success else false
///
	bool onlineInstallBundles(const std::string & i_node);

///
/// Purpose:  Offline remove bundles
/// @param    on what node to offline remove bundles 
/// @return   true on success else false
///
	bool offlineRemoveBundles(const std::string & i_node);

///
/// Purpose:  Online remove bundles
/// @param    on what node to offline remove bundles 
/// @return   true on success else false
///
	bool onlineRemoveBundles(const std::string & i_node);

///
/// Purpose:  Lock deactivation units 
/// @param    - 
/// @return   true on success else false
///
	bool lockDeactivationUnits();

///
/// Purpose:  Lock deactivation units 
/// @param    - 
/// @return   true on success else false
///
	bool terminateDeactivationUnits();

///
/// Purpose:  Unlock activation units 
/// @param    - 
/// @return   true on success else false
///
	bool unlockActivationUnits();

///
/// Purpose:  instantiate activation units 
/// @param    - 
/// @return   true on success else false
///
	bool instantiateActivationUnits();

///
/// Purpose:  restart activation units 
/// @param    - 
/// @return   true on success else false
///
	bool restartActivationUnits();

///
/// Purpose:  modifyInformationModel  
/// @param    - 
/// @return   true on success else false
///
	bool modifyInformationModel();

///
/// Purpose:  setMaintenanceState  
/// @param    - 
/// @return   true on success else false
///
	bool setMaintenanceState();

///
/// Purpose:  createSaAmfNodeSwBundles 
/// @param    - 
/// @return   true on success else false
///
	bool createSaAmfNodeSwBundles(const std::string & i_node);

///
/// Purpose:  deleteSaAmfNodeSwBundles 
/// @param    - 
/// @return   true on success else false
///
	bool deleteSaAmfNodeSwBundles(const std::string & i_node);

///
/// Purpose:  setSwitchOver  
/// @param    i_switchover true if switch over ordered 
/// @return   true on success else false
///
	void setSwitchOver(bool i_switchover);

///
/// Purpose:  getSwitchOver  
/// @param    - 
/// @return   true if switch over ordered
///
	bool getSwitchOver();

	friend class SmfStepState;
	friend class SmfStepStateExecuting;
	friend class SmfStepStateCompleted;
	friend class SmfStepStateFailed;

 private:
	typedef enum {
		SMF_STEP_OFFLINE_INSTALL = 1,
		SMF_STEP_ONLINE_INSTALL  = 2,
		SMF_STEP_OFFLINE_REMOVE  = 3,
		SMF_STEP_ONLINE_REMOVE   = 4
	} SmfInstallRemoveT;

///
/// Purpose:  Call bundle script on remote node
/// @param    -
/// @return   -
///
	bool callBundleScript(SmfInstallRemoveT i_order, const std::list < SmfBundleRef > &i_bundleList,
			      const std::string & i_node);

	bool createOneSaAmfNodeSwBundle(
		const std::string& i_node,
		const SmfBundleRef& bundle);
	bool deleteOneSaAmfNodeSwBundle(
		const std::string & i_node,
		const SmfBundleRef& i_bundle);

///
/// Purpose:  Call admin operation on all dn in list
/// @param    -
/// @return   -
///
	bool callAdminOperation(unsigned int i_operation, const SaImmAdminOperationParamsT_2 ** params,
				const std::list < std::string > &i_dnList);

///
/// Purpose:  Set the state in IMM step object and send state change notification
/// @param    -
/// @return   -
///
	void setImmStateAndSendNotification(SaSmfStepStateT i_state);

///
/// Purpose:  Change the step stste.  If i_onlyInternalState == false, the IMM step object is updated and 
///           a state change event is sent
/// @param    -
/// @return   A ptr to the step thread
///
	void changeState(const SmfStepState * i_state);

///
/// Purpose:  Set step state
/// @param    The state
/// @return   None
///
	void setStepState(SaSmfStepStateT i_state);

///
/// Purpose: Disables copy constructor
///
	 SmfUpgradeStep(const SmfUpgradeStep &);

///
/// Purpose: Disables assignment operator
///
	SmfUpgradeStep & operator=(const SmfUpgradeStep &);

	SmfStepState *m_state;	          // Pointer to current step state class
	unsigned int m_maxRetry;	  // Max Retry
	unsigned int m_retryCount; 	  // Retry count
	unsigned int m_restartOption;	  // Restart option
        std::string m_rdn;
	std::string m_dn;
	SmfUpgradeProcedure *m_procedure; // The procedure that we are part of
	SmfActivationUnit m_activationUnit;
	SmfActivationUnit m_deactivationUnit;
	std::list < SmfBundleRef > m_swRemoveList;
	std::list < SmfBundleRef > m_swAddList;
	std::list < SmfImmOperation * >m_modificationList;
	std::string m_swNode;	         // The node where bundles should be added/removed
	SmfStepT m_stepType;	         // Type of activation unit
        bool     m_switchOver;           // Switchover executed 
};

#endif				// SMFUPGRADESTEP_HH
