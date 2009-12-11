/*      -*- OpenSAF  -*-
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

/* Simple AMF OI test program based on code from immcfg.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ctype.h>
#include <libgen.h>
#include <assert.h>

#include <saAis.h>
#include <saImmOm.h>
#include <immutil.h>

static SaVersionT immVersion = { 'A', 2, 1 };

typedef enum {
	INVALID = 0,
	CREATE = 1,
	DELETE = 2,
	MODIFY
} op_t;

static void test(int x)
{
	if (!x)
		exit(EXIT_FAILURE);
}

/**
 * Alloc SaImmAttrModificationT_2 object and initialize its attributes from nameval (x=y)
 * @param objectName
 * @param nameval
 * 
 * @return SaImmAttrModificationT_2*
 */
static SaImmAttrModificationT_2 *new_attr_mod(const SaNameT *objectName, char *nameval)
{
	int res = 0;
	char *tmp = strdup(nameval);
	char *name, *value;
	SaImmAttrModificationT_2 *attrMod;
	SaImmClassNameT className = immutil_get_className(objectName);
	SaAisErrorT error;

	if (className == NULL) {
		res = -1;
		goto done;
	}

	attrMod = malloc(sizeof(SaImmAttrModificationT_2));

	if ((value = strstr(tmp, "=")) == NULL) {
		res = -1;
		goto done;
	}

	name = tmp;
	*value = '\0';
	value++;

	error = immutil_get_attrValueType(className, name, &attrMod->modAttr.attrValueType);
	if (error == SA_AIS_ERR_NOT_EXIST) {
		fprintf(stderr, "Class '%s' does not exist\n", className);
		res = -1;
		goto done;
	}

	if (error != SA_AIS_OK) {
		fprintf(stderr, "Attribute '%s' does not exist in class '%s'\n", name, className);
		res = -1;
		goto done;
	}

	attrMod->modType = SA_IMM_ATTR_VALUES_REPLACE;
	attrMod->modAttr.attrName = name;
	attrMod->modAttr.attrValuesNumber = 1;
	attrMod->modAttr.attrValues = malloc(sizeof(SaImmAttrValueT *));
	attrMod->modAttr.attrValues[0] = immutil_new_attrValue(attrMod->modAttr.attrValueType, value);

 done:
	free(className);
	if (res != 0) {
		free(attrMod);
		attrMod = NULL;
	}
	return attrMod;
}

/**
 * Alloc SaImmAttrValuesT_2 object and initialize its attributes from nameval (x=y)
 * @param className
 * @param nameval
 * 
 * @return SaImmAttrValuesT_2*
 */
static SaImmAttrValuesT_2 *new_attr_value(const SaImmClassNameT className, char *nameval, int is_rdn)
{
	int res = 0;
	char *name = strdup(nameval), *p;
	char *value;
	SaImmAttrValuesT_2 *attrValue;
	SaAisErrorT error;

	attrValue = malloc(sizeof(SaImmAttrValuesT_2));

	p = strchr(name, '=');
	*p = '\0';
	value = p + 1;

	attrValue->attrName = strdup(name);

	error = immutil_get_attrValueType(className, attrValue->attrName, &attrValue->attrValueType);

	if (error == SA_AIS_ERR_NOT_EXIST) {
		fprintf(stderr, "Class '%s' does not exist\n", className);
		res = -1;
		goto done;
	}

	if (error != SA_AIS_OK) {
		fprintf(stderr, "Attribute '%s' does not exist in class '%s'\n", name, className);
		res = -1;
		goto done;
	}

	attrValue->attrValuesNumber = 1;
	attrValue->attrValues = malloc(sizeof(SaImmAttrValueT *));
	if (is_rdn)
		attrValue->attrValues[0] = immutil_new_attrValue(attrValue->attrValueType, nameval);
	else
		attrValue->attrValues[0] = immutil_new_attrValue(attrValue->attrValueType, value);

	printf("\t%s: %s=%s, type=%u\n",
		__FUNCTION__, attrValue->attrName, value, attrValue->attrValueType);

 done:
	free(name);
	if (res != 0) {
		free(attrValue);
		attrValue = NULL;
	}

	return attrValue;
}

/**
 * Create object(s) of the specified class, initialize attributes with values from optarg.
 * 
 * @param objectNames
 * @param className
 * @param ownerHandle
 * @param optargs
 * @param optargs_len
 * 
 * @return int
 */
int object_create(SaImmCcbHandleT ccbHandle, const char *dn, const SaImmClassNameT className,
	SaImmAdminOwnerHandleT ownerHandle, char **optargs, int optargs_len)
{
	SaAisErrorT error;
	int i;
	SaImmAttrValuesT_2 *attrValue;
	SaImmAttrValuesT_2 **attrValues = NULL;
	int attr_len = 1;
	int rc = EXIT_FAILURE;
	char *parent = NULL;
	SaNameT parentName = {0};
	char *str, *delim;
	SaNameT objectName = {0};

	printf("%s: class=%s, dn='%s'\n", __FUNCTION__, className, dn);

	strcpy((char*)objectName.value, dn);
	objectName.length = strlen(dn);

	for (i = 0; i < optargs_len; i++) {
		attrValues = realloc(attrValues, (attr_len + 1) * sizeof(SaImmAttrValuesT_2 *));
		if ((attrValue = new_attr_value(className, optargs[i], 0)) == NULL)
			goto done;
		
		attrValues[attr_len - 1] = attrValue;
		attrValues[attr_len] = NULL;
		attr_len++;
	}

	str = strdup(dn);
	if ((delim = strchr(str, ',')) != NULL) {
		/* a parent exist */
		if (*(delim - 1) == 0x5c) {
			/* comma delimiter is escaped, search again */
			delim += 2;
			delim = strchr(delim, ',');
		}

		*delim = '\0';
		parent = delim + 1;
		if (!parent) {
			fprintf(stderr, "error - malformed object DN\n");
			goto done;
		}

		parentName.length = sprintf((char*)parentName.value, "%s", parent);
	}

	attrValues = realloc(attrValues, (attr_len + 1) * sizeof(SaImmAttrValuesT_2 *));
	attrValue = new_attr_value(className, str, 1);
	attrValues[attr_len - 1] = attrValue;
	attrValues[attr_len] = NULL;

	if ((error = saImmOmCcbObjectCreate_2(ccbHandle, className, &parentName,
		(const SaImmAttrValuesT_2**)attrValues)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbObjectCreate_2 FAILED with %u\n", error);
		goto done;
	}

	rc = 0;

done:
	return rc;
}

/**
 * Modify object(s) with the attributes specifed in the optargs array
 * 
 * @param objectNames
 * @param ownerHandle
 * @param optargs
 * @param optargs_len
 * 
 * @return int
 */
int object_modify(SaImmCcbHandleT ccbHandle, const char *dn,
	SaImmAdminOwnerHandleT ownerHandle, char **optargs, int optargs_len)
{
	SaAisErrorT error;
	int i;
	int attr_len = 1;
	int rc = EXIT_FAILURE;
	SaImmAttrModificationT_2 *attrMod;
	SaImmAttrModificationT_2 **attrMods = NULL;
	SaNameT objectName = {0};
	const SaNameT *objectNames[] = {&objectName, NULL};

	strcpy((char*)objectName.value, dn);
	objectName.length = strlen(dn);

	for (i = 0; i < optargs_len; i++) {
		attrMods = realloc(attrMods, (attr_len + 1) * sizeof(SaImmAttrModificationT_2 *));
		if ((attrMod = new_attr_mod(objectNames[i], optargs[i])) == NULL)
			exit(EXIT_FAILURE);
		
		attrMods[attr_len - 1] = attrMod;
		attrMods[attr_len] = NULL;
		attr_len++;
	}

	if ((error = saImmOmAdminOwnerSet(ownerHandle, (const SaNameT **)objectNames, SA_IMM_ONE)) != SA_AIS_OK) {
		if (error == SA_AIS_ERR_NOT_EXIST)
			fprintf(stderr, "error - object '%s' does not exist\n", objectNames[0]->value);
		else
			fprintf(stderr, "error - saImmOmAdminOwnerSet FAILED: %u\n", error);
		
		goto done;
	}

	if ((error = saImmOmCcbObjectModify_2(ccbHandle, &objectName,
		(const SaImmAttrModificationT_2 **)attrMods)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbObjectModify_2 FAILED: %u\n", error);
		goto done_release;
	}

	rc = 0;

 done_release:
	if ((error = saImmOmAdminOwnerRelease(ownerHandle, (const SaNameT **)objectNames, SA_IMM_ONE)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmAdminOwnerRelease FAILED: %u\n", error);
		goto done;
	}
 done:
	return rc;
}

/**
 * Delete object(s) in the NULL terminated array using one CCB.
 * @param objectNames
 * @param ownerHandle
 * 
 * @return int
 */
int object_delete(SaImmCcbHandleT ccbHandle, const char *dn, SaImmAdminOwnerHandleT ownerHandle)
{
	SaAisErrorT error;
	int rc = EXIT_FAILURE;
	SaNameT objectName = {0};
	const SaNameT *objectNames[] = {&objectName, NULL};

	strcpy((char*)objectName.value, dn);
	objectName.length = strlen(dn);

	if ((error = saImmOmAdminOwnerSet(ownerHandle, (const SaNameT **)objectNames,
		SA_IMM_SUBTREE)) != SA_AIS_OK) {

		if (error == SA_AIS_ERR_NOT_EXIST)
			fprintf(stderr, "error - object does not exist\n");
		else
			fprintf(stderr, "error - saImmOmAdminOwnerSet FAILED: %u\n", error);
		
		goto done;
	}

	if ((error = saImmOmCcbObjectDelete(ccbHandle, &objectName)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbObjectDelete for '%s' FAILED: %u\n",
			dn, error);
		goto done;
	}

	rc = 0;
done:
	return rc;
}

static void delete_test_case(SaImmCcbHandleT ccbHandle, SaImmAdminOwnerHandleT ownerHandle)
{
	int rc;

	rc = object_delete(ccbHandle, "safApp=Hello", ownerHandle);
	test(rc == 0);

	rc = object_delete(ccbHandle, "safAmfNode=TestNode1,safAmfCluster=myAmfCluster", ownerHandle);
	test(rc == 0);

	rc = object_delete(ccbHandle, "safSvcType=Hello", ownerHandle);
	test(rc == 0);

	rc = object_delete(ccbHandle, "safSuType=Hello", ownerHandle);
	test(rc == 0);

	rc = object_delete(ccbHandle, "safSgType=Hello", ownerHandle);
	test(rc == 0);

	rc = object_delete(ccbHandle, "safAppType=Hello", ownerHandle);
	test(rc == 0);

	rc = object_delete(ccbHandle, "safCompType=Hello", ownerHandle);
	test(rc == 0);

	rc = object_delete(ccbHandle, "safCSType=Hello", ownerHandle);
	test(rc == 0);
}

static void create_test_case(SaImmCcbHandleT ccbHandle, SaImmAdminOwnerHandleT ownerHandle)
{
	int rc = EXIT_SUCCESS;
	char *optargs[32];

	optargs[0] = "saAmfNodeSuFailOverProb=777777777";
	optargs[1] = "saAmfNodeSuFailoverMax=7";
	optargs[2] = "saAmfNodeAutoRepair=1";
	optargs[3] = "saAmfNodeAdminState=3";
	optargs[4] = "saAmfNodeClmNode=safNode=PL_2_5,safCluster=myClmCluster";
	rc = object_create(ccbHandle, "safAmfNode=TestNode1,safAmfCluster=myAmfCluster", "SaAmfNode",
		ownerHandle, optargs, 5);
	test(rc == 0);

	optargs[0] = "saAmfNGNodeList=safAmfNode=TestNode1,safAmfCluster=myAmfCluster";
	rc = object_create(ccbHandle, "safAmfNodeGroup=TestNodeGroup,safAmfCluster=myAmfCluster", "SaAmfNodeGroup",
		ownerHandle, optargs, 1);
	test(rc == 0);

	rc = object_create(ccbHandle, "safSvcType=Hello", "SaAmfSvcBaseType", ownerHandle, NULL, 0);
	test(rc == 0);
	rc = object_create(ccbHandle, "safVersion=1.0,safSvcType=Hello", "SaAmfSvcType", ownerHandle, NULL, 0);
	test(rc == 0);
	rc = object_create(ccbHandle, "safSuType=Hello", "SaAmfSUBaseType", ownerHandle, NULL, 0);
	test(rc == 0);

	optargs[0] = "saAmfSutIsExternal=0";
	optargs[1] = "saAmfSutDefSUFailover=1";
	optargs[2] = "saAmfSutProvidesSvcTypes=safVersion=1.0,safSvcType=Hello";
	rc = object_create(ccbHandle, "safVersion=1.0,safSuType=Hello", "SaAmfSUType", ownerHandle, optargs, 3);
	test(rc == 0);

	rc = object_create(ccbHandle, "safSgType=Hello", "SaAmfSGBaseType", ownerHandle, NULL, 0);
	test(rc == 0);

	optargs[0] = "saAmfSgtValidSuTypes=safVersion=1.0,safSuType=Hello";
	optargs[1] = "saAmfSgtRedundancyModel=1";
	optargs[2] = "saAmfSgtDefSuRestartProb=4000000";
	optargs[3] = "saAmfSgtDefSuRestartMax=10";
	optargs[4] = "saAmfSgtDefCompRestartProb=4000000";
	optargs[5] = "saAmfSgtDefCompRestartMax=10";
	optargs[6] = "saAmfSgtDefAutoAdjustProb=4000000";
	rc = object_create(ccbHandle, "safVersion=1.0,safSgType=Hello", "SaAmfSGType", ownerHandle, optargs, 7);
	test(rc == 0);

	rc = object_create(ccbHandle, "safAppType=Hello", "SaAmfAppBaseType", ownerHandle, NULL, 0);
	test(rc == 0);

	optargs[0] = "saAmfApptSGTypes=safVersion=1.0,safSgType=Hello";
	rc = object_create(ccbHandle, "safVersion=1.0,safAppType=Hello", "SaAmfAppType", ownerHandle, optargs, 1);
	test(rc == 0);

	optargs[0] = "saAmfAppType=safVersion=1.0,safAppType=Hello";
	optargs[1] = "saAmfApplicationAdminState=3";
	rc = object_create(ccbHandle, "safApp=Hello", "SaAmfApplication", ownerHandle, optargs, 2);
	test(rc == 0);

	optargs[0] = "saAmfSGType=safVersion=1.0,safSgType=Hello";
	rc = object_create(ccbHandle, "safSg=Hello,safApp=Hello", "SaAmfSG", ownerHandle, optargs, 1);
	test(rc == 0);

	rc = object_create(ccbHandle, "safCompType=Hello", "SaAmfCompBaseType", ownerHandle, optargs, 0);
	test(rc == 0);

	optargs[0] = "saAmfCtCompCategory=1";
	optargs[1] = "saAmfCtDefRecoveryOnError=2";
	optargs[2] = "saAmfCtRelPathInstantiateCmd=amf_demo_script";
        optargs[3] = "saAmfCtDefInstantiateCmdArgv=instantiate";
	optargs[4] = "saAmfCtRelPathCleanupCmd=amf_demo_script";
	optargs[5] = "saAmfCtDefCleanupCmdArgv=cleanup";
	optargs[6] = "saAmfCtSwBundle=safBundle=AmfDemo";
	rc = object_create(ccbHandle, "safVersion=1.0,safCompType=Hello", "SaAmfCompType", ownerHandle, optargs, 7);
	test(rc == 0);

	optargs[0] = "saAmfSUType=safVersion=1.0,safSuType=Hello";
	optargs[1] = "saAmfSUHostNodeOrNodeGroup=safAmfNode=SC_2_1,safAmfCluster=myAmfCluster";
	optargs[2] = "saAmfSUAdminState=3";
	rc = object_create(ccbHandle, "safSu=HelloSC1,safSg=Hello,safApp=Hello", "SaAmfSU", ownerHandle, optargs, 3);
	test(rc == 0);
	optargs[1] = "saAmfSUHostNodeOrNodeGroup=safAmfNode=SC_2_2,safAmfCluster=myAmfCluster";
	rc = object_create(ccbHandle, "safSu=HelloSC2,safSg=Hello,safApp=Hello", "SaAmfSU", ownerHandle, optargs, 3);
	test(rc == 0);

	optargs[0] = "saAmfCompType=safVersion=1.0,safCompType=Hello";
	rc = object_create(ccbHandle, "safComp=Hello,safSu=HelloSC1,safSg=Hello,safApp=Hello", "SaAmfComp",
		ownerHandle, optargs, 1);
	test(rc == 0);
	rc = object_create(ccbHandle, "safComp=Hello,safSu=HelloSC2,safSg=Hello,safApp=Hello", "SaAmfComp",
		ownerHandle, optargs, 1);
	test(rc == 0);

	optargs[0] = "saAmfSvcType=safVersion=1.0,safSvcType=Hello";
	optargs[1] = "saAmfSIProtectedbySG=safSg=Hello,safApp=Hello";
	rc = object_create(ccbHandle, "safSi=Hello1,safApp=Hello", "SaAmfSI", ownerHandle, optargs, 2);
	test(rc == 0);

	rc = object_create(ccbHandle, "safCSType=Hello", "SaAmfCSBaseType", ownerHandle, optargs, 0);
	test(rc == 0);

	rc = object_create(ccbHandle, "safVersion=1.0,safCSType=Hello", "SaAmfCSType", ownerHandle, optargs, 0);
	test(rc == 0);

	optargs[0] = "saAmfCSType=safVersion=1.0,safCSType=Hello";
	rc = object_create(ccbHandle, "safCsi=Hello,safSi=Hello1,safApp=Hello", "SaAmfCSI", ownerHandle, optargs, 1);
	test(rc == 0);

	rc = object_create(ccbHandle, "safCsiAttr=NoVal,safCsi=Hello,safSi=Hello1,safApp=Hello", "SaAmfCSIAttribute",
		ownerHandle, optargs, 0);
	test(rc == 0);

	optargs[0] = "saAmfCSIAttriValue=dummy1";
	rc = object_create(ccbHandle, "safCsiAttr=SingleVal,safCsi=Hello,safSi=Hello1,safApp=Hello",
		"SaAmfCSIAttribute", ownerHandle, optargs, 1);
	test(rc == 0);

	optargs[0] = "saAmfCSIAttriValue=dummy21";
	optargs[1] = "saAmfCSIAttriValue=dummy22";
	rc = object_create(ccbHandle, "safCsiAttr=MultiVal,safCsi=Hello,safSi=Hello1,safApp=Hello",
		"SaAmfCSIAttribute", ownerHandle, optargs, 2);
	test(rc == 0);

	optargs[0] = "saAmfHctDefMaxDuration=1000000000";
	optargs[1] = "saAmfHctDefPeriod=10000000000";
	rc = object_create(ccbHandle, "safHealthcheckKey=AmfDemo,safVersion=1.0,safCompType=Hello",
		"SaAmfHealthcheckType", ownerHandle, optargs, 2);
	test(rc == 0);

	optargs[0] = "saAmfHealthcheckMaxDuration=1000000000";
	optargs[1] = "saAmfHealthcheckPeriod=10000000000";
	rc = object_create(ccbHandle,
		"safHealthcheckKey=AmfDemo2,safComp=Hello,safSu=HelloSC1,safSg=Hello,safApp=Hello",
		"SaAmfHealthcheck", ownerHandle, optargs, 2);
	test(rc == 0);

	optargs[0] = "saAmfCtCompCapability=1";
	rc = object_create(ccbHandle,
		"safSupportedCsType=safVersion=1.0\\,safCSType=Hello,safVersion=1.0,safCompType=Hello",
		"SaAmfCtCsType", ownerHandle, optargs, 1);
	test(rc == 0);

	rc = object_create(ccbHandle,
		"safSupportedCsType=safVersion=1.0\\,safCSType=Hello,safComp=Hello,safSu=HelloSC1,safSg=Hello,safApp=Hello",
		"SaAmfCompCsType", ownerHandle, optargs, 0);
	test(rc == 0);
	rc = object_create(ccbHandle,
		"safSupportedCsType=safVersion=1.0\\,safCSType=Hello,safComp=Hello,safSu=HelloSC2,safSg=Hello,safApp=Hello",
		"SaAmfCompCsType", ownerHandle, optargs, 0);
	test(rc == 0);

#if 1
	optargs[0] = "saAmfNodeSwBundlePathPrefix=/opt/amf_demo";
	rc = object_create(ccbHandle,
		"safInstalledSwBundle=safBundle=AmfDemo,safAmfNode=SC_2_1,safAmfCluster=myAmfCluster",
		"SaAmfNodeSwBundle", ownerHandle, optargs, 1);
	test(rc == 0);
	rc = object_create(ccbHandle,
		"safInstalledSwBundle=safBundle=AmfDemo,safAmfNode=SC_2_2,safAmfCluster=myAmfCluster",
		"SaAmfNodeSwBundle", ownerHandle, optargs, 1);
	test(rc == 0);
#endif
}

int main(int argc, char *argv[])
{
	int rc = EXIT_SUCCESS;
	int c;
	struct option long_options[] = {
		{"abort", no_argument, 0, 'a'},
		{"delete", no_argument, 0, 'd'},
		{0, 0, 0, 0}
	};
	SaAisErrorT error;
	SaImmHandleT immHandle;
	SaImmAdminOwnerNameT adminOwnerName = basename(argv[0]);
	SaImmAdminOwnerHandleT ownerHandle;
	SaImmCcbHandleT ccbHandle;
	int abort = 0;
	int delete = 0;
	SaNameT parentName;
	const SaNameT *parentNames[] = {&parentName, NULL};

	while (1) {
		c = getopt_long(argc, argv, "ad", long_options, NULL);

		if (c == -1)	/* have all command-line options have been parsed? */
			break;

		switch (c) {
		case 'a':
			abort = 1;
			break;
		case 'd':
			delete = 1;
			break;
		default:
			fprintf(stderr, "Try '%s --help' for more information\n", argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}

	(void)immutil_saImmOmInitialize(&immHandle, NULL, &immVersion);

	error = saImmOmAdminOwnerInitialize(immHandle, adminOwnerName, SA_TRUE, &ownerHandle);
	if (error != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmAdminOwnerInitialize FAILED: %u\n", error);
		rc = EXIT_FAILURE;
		goto done_om_finalize;
	}

	if ((error = saImmOmCcbInitialize(ownerHandle, 0, &ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbInitialize FAILED: %u\n", error);
		rc = EXIT_FAILURE;
		goto done_om_finalize;
	}

	strcpy((char*)parentName.value, "safAmfCluster=myAmfCluster");
	parentName.length = strlen((char*)parentName.value);
	error = saImmOmAdminOwnerSet(ownerHandle, parentNames, SA_IMM_SUBTREE);
	test(error == SA_AIS_OK);

	if (delete)
		delete_test_case(ccbHandle, ownerHandle);
	else
		create_test_case(ccbHandle, ownerHandle);

	if (!abort) {
		if ((error = saImmOmCcbApply(ccbHandle)) != SA_AIS_OK) {
			fprintf(stderr, "error - saImmOmCcbApply FAILED: %u\n", error);
			goto done_om_finalize;
		}
	}

	if ((error = saImmOmCcbFinalize(ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbFinalize FAILED: %u\n", error);
		goto done_om_finalize;
	}

	error = saImmOmAdminOwnerRelease(ownerHandle, (const SaNameT **)parentNames, SA_IMM_SUBTREE);
	test(error == SA_AIS_OK);

	error = saImmOmAdminOwnerFinalize(ownerHandle);
	if (SA_AIS_OK != error) {
		fprintf(stderr, "error - saImmOmAdminOwnerFinalize FAILED: %u\n", error);
		rc = EXIT_FAILURE;
		goto done_om_finalize;
	}

done_om_finalize:
	(void)immutil_saImmOmFinalize(immHandle);

	if (SA_AIS_OK == error)
		printf("test success\n");
	else
		printf("test failure\n");
	exit(rc);
}
