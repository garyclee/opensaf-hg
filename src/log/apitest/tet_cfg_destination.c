/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2017 The OpenSAF Foundation
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

#include "log/apitest/logtest.h"

// These test cases are for testing #2258 - alternative destinations

//==============================================================================
// Validate `logRecordDestinationConfiguration`
//==============================================================================
const char validDest[] = "desta;UNIX_SOCKET;/var/lib/opensaf/mds_log.sock";
// Multi destinations
const char multiDest1[] = "desta;UNIX_SOCKET;/var/lib/opensaf/mds_log.sock";
const char multiDest2[] = "destb;UNIX_SOCKET;/var/lib/opensaf/local.sock";
// Delete destination
const char delDest[] = "";
// NIL destination configution.
const char nildest[] = "destc;UNIX_SOCKET;";

//>
// Abnormal cases
//<
// Only destination type = "unix" is supported.
const char invalidTypeDest[] = "test;invalid;/var/lib/opensaf/mds_log.sock";
// "name" & "type" must have at least.
const char invalidFmtDest[] = "test";
// "name" must not contain special characters
const char invalidNameDest[] = "destA?abc;UNIX_SOCKET;";
// Same "name" must go with same "dest" and vice versa.
const char invalidDuplicatedDest[] =
	"destb;UNIX_SOCKET;/var/lib/opensaf/mds_log.sock";
const char nilname[] = "destc";

// Configure destination command
const char cfgObjDn[] = "logConfig=1,safApp=safLogService";
const char kSetDestConf[] = "immcfg -a logRecordDestinationConfiguration";

// Verify it is OK to set an valid destination.
void cfgOneValidDest(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s=\"%s\" %s", kSetDestConf, validDest, cfgObjDn);
	rc = systemCall(command);

	// Cleanup by removing all values
	sprintf(command,"%s='' %s", kSetDestConf, cfgObjDn);
	systemCall(command);

	rc_validate(rc, 0);
}

// Verify it is OK to set multi valid destinations.
void cfgMultiValidDest(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s=\"%s\" %s", kSetDestConf, multiDest1, cfgObjDn);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		return;
	}

	// Multiple configuration
	sprintf(command,"%s+=\"%s\" %s", kSetDestConf, multiDest2, cfgObjDn);
	rc = systemCall(command);

	// Cleanup by removing all values
	sprintf(command,"%s='' %s", kSetDestConf, cfgObjDn);
	systemCall(command);

	rc_validate(rc, 0);
}

// Verify it is OK to delete one destination.
void delOneDest(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s=\"%s\" %s", kSetDestConf, multiDest1, cfgObjDn);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		return;
	}

	// Add multiple configuration
	sprintf(command,"%s+=\"%s\" %s", kSetDestConf, multiDest2, cfgObjDn);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		return;
	}

	// Delete one configuration
	sprintf(command,"%s-=\"%s\" %s", kSetDestConf, multiDest2, cfgObjDn);
	rc = systemCall(command);

	// Cleanup by removing all values
	sprintf(command,"%s='' %s", kSetDestConf, cfgObjDn);
	systemCall(command);

	rc_validate(rc, 0);
}

// Verify it is OK to delete destinations
void delCfgDest(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s=  %s", kSetDestConf, cfgObjDn);
	rc = systemCall(command);

	rc_validate(rc, 0);
}

// Verify it is NOK to set type different from "unix"
void invalidTypeDestFn(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s=\"%s\" %s 2> /dev/null",
		kSetDestConf, invalidTypeDest, cfgObjDn);
	rc = system(command);
	rc_validate(WEXITSTATUS(rc), 1);
}

// Verify it is NOK to set wrong destination format
// Note: right format is one that have at least "name" and "type"
void invalidFmtDestFn(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s=\"%s\" %s 2> /dev/null",
		kSetDestConf, invalidFmtDest, cfgObjDn);
	rc = system(command);
	rc_validate(WEXITSTATUS(rc), 1);
}

// Verify it is NOK to set configuration with name contain special character
void invalidNameDestFn(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s=\"%s\" %s 2> /dev/null",
		kSetDestConf, invalidNameDest, cfgObjDn);
	rc = system(command);
	rc_validate(WEXITSTATUS(rc), 1);
}

// Verify it is NOK to have destination configuration duplicated.
// The rule is same "name" must go with same "value", and vice versa.
void duplicatedDestFn(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s=\"%s\" %s", kSetDestConf, multiDest1, cfgObjDn);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		return;
	}

	sprintf(command,"%s+=\"%s\" %s 2> /dev/null",
		kSetDestConf, invalidDuplicatedDest, cfgObjDn);
	rc = system(command);

	// Cleanup by removing all values
	sprintf(command,"%s='' %s", kSetDestConf, cfgObjDn);
	systemCall(command);

	rc_validate(WEXITSTATUS(rc), 1);
}

//==============================================================================
// These test cases are used to test `saLogRecordDestination`
//==============================================================================
const char validName[] = "desta";
const char multiName[] = "test2";

// Invalid name
const char invalidName[] = "te?st";

// Configure destination name command
const char systemDN[] = "safLgStrCfg=saLogSystem,safApp=safLogService";
const char kSetDestStreamConf[] = "immcfg -a saLogRecordDestination";

// Verify it is OK to set an valid destination name
void cfgOneDestName(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s=\"%s\" %s", kSetDestStreamConf, validName, systemDN);
	rc = systemCall(command);

	// Cleanup by removing all values
	sprintf(command,"%s='' %s", kSetDestStreamConf, systemDN);
	systemCall(command);

	rc_validate(rc, 0);
}

// Verify it is OK to set multiple destination names
void cfgMultiDestName(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s=\"%s\" %s", kSetDestStreamConf, validName, systemDN);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		return;
	}

	sprintf(command,"%s+=\"%s\" %s", kSetDestStreamConf, multiName, systemDN);
	rc = systemCall(command);

	// Cleanup by removing all values
	sprintf(command,"%s='' %s", kSetDestStreamConf, systemDN);
	systemCall(command);

	rc_validate(rc, 0);
}

// Verify it is OK to clear destination name
void delDestName(void)
{
	int rc;
	char command[1000];

	sprintf(command,"%s= %s", kSetDestStreamConf, systemDN);
	rc = systemCall(command);

	rc_validate(rc, 0);
}

// Verify it is NOK to set an invalid destination name
// Rule: invalid means having special characters in its name
void invalidDestName(void)
{
	SaAisErrorT rc;
	char command[1000];

	sprintf(command,"%s=\"%s\" %s 2>/dev/null", kSetDestStreamConf, invalidName, systemDN);
	rc = system(command);
	rc_validate(WEXITSTATUS(rc), 1);
}

//==============================================================================
// Send Log record to one configured destination
//==============================================================================
//
// 1) Configure an valid destination to configurable object
// 2) Configure an valid destination name to system log stream
// 3) Send an log record to the system log stream
// 4) Wait a bit for msg come to the end (guess number)
// 5) Verify if RFC syslog comes to the receiver
// Note:
// Use MDS log server as an receiver.
//
const char sendMsg[] = "writing a record to destination";
const char sendCmd[] = "saflogger -y";
const char mdsLogPath[] = "/var/log/opensaf/mds.log";

bool is_executed_on_active_node()
{
	char node[256];
	sprintf(node,"SC-%d", get_active_sc());
	return (strcmp(node, hostname()) == 0);
}

// Verify if the record comes to destination or not
void writeToDest(void)
{
	int rc;
	char command[1000];
	bool disable_stdout = true;
	SaConstStringT s_stdout = "1> /dev/null";

	if (is_executed_on_active_node() == false) {
		fprintf(stdout, "Should perform this test case on active node. Report PASSED\n");
		rc_validate(0, 0);
		return;
	}

	if (getenv("LOGTEST_ENABLE_STDOUT")) disable_stdout = false;

	// 1) Configure an valid destination
	sprintf(command,"%s=\"%s\" %s", kSetDestConf, validDest, cfgObjDn);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		return;
	}

	// 2) Configure an valid destination name
	sprintf(command,"%s=\"%s\" %s", kSetDestStreamConf, validName, systemDN);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		goto clear_attr;
	}

	// Avoid getting old msg
	int r = rand();

	// 3) Send an log record to system log stream
	sprintf(command,"%s \"%s_%d\"", sendCmd, sendMsg, r);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		goto clear_attr;
	}

	// 5) Verify if that sent msg comes to the end
	sprintf(command,"grep \"%s_%d\" %s %s", sendMsg, r, mdsLogPath,
		disable_stdout ? s_stdout : "");

	int count = 0;
tryagain:
	rc = system(command);
	if (WEXITSTATUS(rc) != 0) {
		sleep(1); count++;
		if (count < 20) {
			goto tryagain;
		} else {
			fprintf(stderr, "Failed to perform cmd %s\n", command);
		}
	}

	rc_validate(WEXITSTATUS(rc), 0);

clear_attr:
	// Cleanup by removing all values
	sprintf(command,"%s='' %s", kSetDestConf, cfgObjDn);
	systemCall(command);

	sprintf(command,"%s='' %s", kSetDestStreamConf, systemDN);
	systemCall(command);
}

// Verify if the record comes to destination or not
// if no destination name set
const char sendMsgNoDest[] = "[No dest name set] writing a record to destination";
void writeToNoDestName(void)
{
	int rc;
	char command[1000];
	bool disable_stdout = true;
	SaConstStringT s_stdout = "1> /dev/null";

	if (is_executed_on_active_node() == false) {
		fprintf(stdout, "Should perform this test case on active node. Report PASSED\n");
		rc_validate(0, 0);
		return;
	}

	if (getenv("LOGTEST_ENABLE_STDOUT")) disable_stdout = false;

	// 1) Configure an valid destination
	sprintf(command,"%s=\"%s\" %s", kSetDestConf, validDest, cfgObjDn);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		return;
	}

	// 2) Delete Destination name
	sprintf(command,"%s= %s", kSetDestStreamConf, systemDN);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		goto clear_attr;
	}

	// Avoid getting old msg
	int r = rand();

	// 3) Send an log record to system log stream
	sprintf(command,"%s \"%s_%d\"", sendCmd, sendMsg, r);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		goto clear_attr;
	}

	// 4) Sleep for a while (5s) as mds log server could busy
	// handling log comming from other OpenSAF components
	sleep(2);

	// 5) Verify if that sent msg comes to the end
	sprintf(command,"grep \"%s_%d\" %s %s", sendMsgNoDest, r, mdsLogPath,
		disable_stdout ? s_stdout : "");
	rc = system(command);
	if (WEXITSTATUS(rc) == 0) {
		fprintf(stderr, "log record is written to  local file\n");
	}

	rc_validate(WEXITSTATUS(rc), 1);

clear_attr:
	// Cleanup by removing all values
	sprintf(command,"%s='' %s", kSetDestConf, cfgObjDn);
	systemCall(command);
}

// Verify if the record comes to destination or not
// if destination name is set but no destination configured to that name.
const char sendMsgNil[] = "[Dest name with nil configuration] writing a record to destination";
void writeToNilDestCfg(void)
{
	int rc;
	char command[1000];
	bool disable_stdout = true;
	SaConstStringT s_stdout = "1> /dev/null";

	if (is_executed_on_active_node() == false) {
		fprintf(stdout, "Should perform this test case on active node. Report PASSED\n");
		rc_validate(0, 0);
		return;
	}

	if (getenv("LOGTEST_ENABLE_STDOUT")) disable_stdout = false;

	// 1) Configure an valid destination
	sprintf(command,"%s=\"%s\" %s", kSetDestConf, validDest, cfgObjDn);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		return;
	}

	// 2) Add a nil destination
	sprintf(command,"%s+=\"%s\" %s", kSetDestConf, nildest, cfgObjDn);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		goto clear_attr;
	}

	// 3) Add destination name with nil destination configuation.
	sprintf(command,"%s=\"%s\" %s", kSetDestStreamConf, nilname, systemDN);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		goto clear_attr;
	}

	// Avoid getting old msg
	int r = rand();

	// 4) Send an log record to system log stream
	sprintf(command,"%s \"%s_%d\"", sendCmd, sendMsg, r);
	rc = systemCall(command);
	if (rc != 0) {
		rc_validate(rc, 0);
		goto clear_attr;
	}

	// 4) Sleep for a while (2s) as mds log server could busy
	// handling log comming from other OpenSAF components
	sleep(2);

	// 5) Verify if that sent msg comes to the end
	sprintf(command,"grep \"%s_%d\" %s %s", sendMsgNil, r, mdsLogPath,
		disable_stdout ? s_stdout : "");
	rc = system(command);
	if (WEXITSTATUS(rc) == 0) {
		fprintf(stderr, "log record is written to  local file\n");
	}

	rc_validate(WEXITSTATUS(rc), 1);

clear_attr:
	// Cleanup by removing all values
	sprintf(command,"%s='' %s", kSetDestConf, cfgObjDn);
	systemCall(command);

	sprintf(command,"%s='' %s", kSetDestStreamConf, systemDN);
	systemCall(command);
}

__attribute__ ((constructor)) static void cfgDest_constructor(void)
{
	/* Test suite for testing #2258 */
	// Normal test cases verify `logRecordDestinationConfiguration`
	test_suite_add(18, "LOG OI tests, configure alternative destinations");
	test_case_add(18, cfgOneValidDest, "Configure one valid destination, OK");
	test_case_add(18, cfgMultiValidDest, "Configure multi valid destinations, OK");
	test_case_add(18, delOneDest, "Delete one destinations, OK");
	test_case_add(18, delCfgDest, "Delete all configure, OK");
	// Abnormal test cases
	test_case_add(18, invalidTypeDestFn, "Set destination with invalid type, NOK");
	test_case_add(18, invalidFmtDestFn, "Set destination with invalid format, NOK");
	test_case_add(18, invalidNameDestFn, "Set destination with invalid name, NOK");
	test_case_add(18, duplicatedDestFn, "Set destination with duplicated info, NOK");
	// Normal test cases verify `saLogRecordDestination`
	test_case_add(18, cfgOneDestName, "Configure one valid destination name, OK");
	test_case_add(18, cfgMultiDestName, "Configure multi valid destination names, OK");
	test_case_add(18, delDestName, "Delete all destination names, OK");
	// Abnormal
	test_case_add(18, invalidDestName, "Configure an invalid destination name, NOK");
	// Write log record to destination
	test_case_add(18, writeToDest, "Write a log record to destination, OK");
	test_case_add(18, writeToNoDestName, "Write to stream with no destination name set, OK");
	test_case_add(18, writeToNilDestCfg, "Write to stream with destination name set but nil cfg, OK");
}
