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

#ifndef SMFSTEPSTATE_HH
#define SMFSTEPSTATE_HH

/* ========================================================================
 *   INCLUDE FILES
 * ========================================================================
 */

#include <string>

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

class SmfUpgradeStep;

//================================================================================
// Class SmfStepState
//================================================================================
///
/// Purpose: Base class for all procedure steps.
///
class SmfStepState {
 public:

	virtual ~ SmfStepState() {
	};

	virtual std::string getClassName() const;

	virtual bool execute(SmfUpgradeStep * i_step);

	virtual SaSmfStepStateT getState() const = 0;

 protected:

	void changeState(SmfUpgradeStep * i_step, SmfStepState * i_state);
};

//================================================================================
// Class SmfStepStateInitial
//================================================================================
///
/// Purpose: Initial state for the procedure step..
///
class SmfStepStateInitial:public SmfStepState {
 public:

	static SmfStepState *instance(void);

	virtual std::string getClassName() const;

	virtual bool execute(SmfUpgradeStep * i_step);

	virtual SaSmfStepStateT getState() const {
		return SA_SMF_STEP_INITIAL;
 } private:

	static SmfStepState *s_instance;
};

//================================================================================
// Class SmfStepStateExecuting
//================================================================================
///
/// Purpose: Executing state for the procedure step..
///
class SmfStepStateExecuting:public SmfStepState {
 public:

	static SmfStepState *instance();

	virtual std::string getClassName() const;

	virtual bool execute(SmfUpgradeStep * i_step);

	virtual SaSmfStepStateT getState() const {
		return SA_SMF_STEP_EXECUTING;
 } private:

	bool executeSwInstall(SmfUpgradeStep * i_step);
	bool executeSwInstallAct(SmfUpgradeStep * i_step);
	bool executeAuLock(SmfUpgradeStep * i_step);
	bool executeAuLockAct(SmfUpgradeStep * i_step);
	bool executeAuRestart(SmfUpgradeStep * i_step);
	bool executeAuRestartAct(SmfUpgradeStep * i_step);
	bool executeNodeReboot(SmfUpgradeStep * i_step);
	bool executeNodeRebootAct(SmfUpgradeStep * i_step);

	static SmfStepState *s_instance;
};

//================================================================================
// Class SmfStepStateCompleted
//================================================================================
///
/// Purpose: Execution completed state for the procedure step..
///
class SmfStepStateCompleted:public SmfStepState {
 public:

	static SmfStepState *instance();

	virtual std::string getClassName() const;

	virtual SaSmfStepStateT getState() const {
		return SA_SMF_STEP_COMPLETED;
 } private:

	static SmfStepState *s_instance;
};

//================================================================================
// Class SmfStepStateFailed
//================================================================================
///
/// Purpose: Execution failed state for the procedure step..
///
class SmfStepStateFailed:public SmfStepState {
 public:

	static SmfStepState *instance();

	virtual std::string getClassName() const;

	virtual SaSmfStepStateT getState() const {
		return SA_SMF_STEP_FAILED;
 } private:

	static SmfStepState *s_instance;
};

#endif				// SMFSTEPSTATE_HH
