/*      -- OpenSAF  --
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

#ifndef _SmfCampaign_hh_
#define _SmfCampaign_hh_

#include <string>
#include <list>
#include <saAis.h>
#include <saSmf.h>
#include <saImmOi.h>

#include <SmfCampaignThread.hh>

#define SMF_CONFIG_OBJECT_DN      "smfConfig=1,safApp=safSmfService"
#define SMF_BACKUP_CREATE_ATTR    "smfBackupCreateCmd"
#define SMF_BUNDLE_CHECK_ATTR     "smfBundleCheckCmd"
#define SMF_NODE_CHECK_ATTR       "smfNodeCheckCmd"
#define SMF_REPOSITORY_CHECK_ATTR "smfRepositoryCheckCmd"
#define SMF_CLUSTER_REBOOT_ATTR   "smfClusterRebootCmd"
#define SMF_ADMIN_OP_TIMEOUT_ATTR "smfAdminOpTimeout"
#define SMF_CLI_TIMEOUT_ATTR      "smfCliTimeout"

class SmfUpgradeCampaign;
class SmfUpgradeProcedure;

///
/// Purpose: Holds data for the campaigns stored in IMM.
///

class SmfCampaign {
 public:
	SmfCampaign(const SaNameT * dn);
	 SmfCampaign(const SaNameT * parent, const SaImmAttrValuesT_2 ** attrValues);
	~SmfCampaign();

	const std::string & getDn(void);
	bool executing(void);

	SaAisErrorT init(const SaImmAttrValuesT_2 ** attrValues);

	SaAisErrorT verify(const SaImmAttrModificationT_2 ** attrMods);
	SaAisErrorT modify(const SaImmAttrModificationT_2 ** attrMods);

	SaAisErrorT adminOperation(const SaImmAdminOperationIdT opId, const SaImmAdminOperationParamsT_2 ** params);

	SaAisErrorT adminOpExecute(void);
	SaAisErrorT adminOpSuspend(void);
	SaAisErrorT adminOpCommit(void);
	SaAisErrorT adminOpRollback(void);

//    void procResult(SmfUpgradeProcedure* procedure, PROCEDURE_RESULT rc);
	/* TODO Remove procResult ?? */
	void procResult(SmfUpgradeProcedure * procedure, int rc);

	void setState(SaSmfCmpgStateT state);
	void setConfigBase(SaTimeT configBase);
	void setExpectedTime(SaTimeT expectedTime);
	void setElapsedTime(SaTimeT elapsedTime);
	void setError(const std::string & error);
	void setUpgradeCampaign(SmfUpgradeCampaign * i_campaign);
	SmfUpgradeCampaign *getUpgradeCampaign();

 private:

	 std::string m_dn;
	 std::string m_cmpg;
	 std::string m_cmpgFileUri;
	SaTimeT m_cmpgConfigBase;
	SaTimeT m_cmpgExpectedTime;
	SaTimeT m_cmpgElapsedTime;
	SaSmfCmpgStateT m_cmpgState;
	 std::string m_cmpgError;
	SmfUpgradeCampaign *m_upgradeCampaign;
};

///
/// Purpose: The list of SMF campaigns
///

class SmfCampaignList {
 public:
	static SmfCampaignList *instance(void);

	SmfCampaign *get(const SaNameT * dn);
	SaAisErrorT add(SmfCampaign * newCampaign);
	SaAisErrorT del(const SaNameT * dn);
	void cleanup(void);

 private:
	 SmfCampaignList();
	~SmfCampaignList();

	 std::list < SmfCampaign * >m_campaignList;
	static SmfCampaignList *s_instance;
};

#endif
