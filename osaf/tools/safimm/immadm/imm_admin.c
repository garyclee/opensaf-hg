/*      -*- OpenSAF  -*-
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

/*
 * This file contains a command line utility to perform IMM admin operations.
 * Example: immadm -o 3 -p saAmfNodeSuFailoverMax:SA_INT32_T:7 "safAmfNode=Node01,safAmfCluster=1"
 */

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

#include <saAis.h>
#include <saImmOm.h>

#include <immutil.h>
#include "saf_error.h"
#include <poll.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define PARAMDELIM ":"

static SaVersionT immVersion = { 'A', 2, 11 };
static SaImmOiImplementerNameT implName = NULL;


/**
 * 
 */
static void usage(const char *progname)
{
	printf("\nNAME\n");
	printf("\t%s - perform an IMM admin operation\n", progname);

	printf("\nSYNOPSIS\n");
	printf("\t%s [options] [object DN]...\n", progname);

	printf("\nDESCRIPTION\n");
	printf("\t%s is a IMM OM client used to ....\n", progname);

	printf("\nOPTIONS\n");
	printf("\t-h, --help\n");
	printf("\t\tthis help\n");
	printf("\t-o, --operation-id <id>\n");
	printf("\t\tnumerical operation ID (mandatory)\n");
	printf("\t-p, --parameter <p>\n");
	printf("\t\tparameter(s) to admin op\n");
	printf("\t\tParameter syntax: <name>:<type>:<value>\n");
	printf("\t\tValue types according to imm.xsd.\n"
	       "\t\tValid types: SA_INT32_T, SA_UINT32_T, SA_INT64_T, SA_UINT64_T\n"
	       "\t\t\tSA_TIME_T, SA_NAME_T, SA_FLOAT_T, SA_DOUBLE_T, SA_STRING_T\n");
	printf("\t-a, --applier <oi-name> <class-name> (requires 'configure --enable-tests')\n");
	printf("\t\tRegister oi for class. Prefix OI-name with '@' for applier OI\n");

	printf("\nEXAMPLE\n");
	printf("\timmadm -o 2 safAmfNode=SC-2,safAmfCluster=myAmfCluster\n");
	printf("\timmadm -o 1 -p lockOption:SA_STRING_T:trylock  safEE=SC-1,safDomain=domain_1\n");

}

static SaImmValueTypeT str2_saImmValueTypeT(const char *str)
{
	if (!str)
		return -1;

	if (strcmp(str, "SA_INT32_T") == 0)
		return SA_IMM_ATTR_SAINT32T;
	if (strcmp(str, "SA_UINT32_T") == 0)
		return SA_IMM_ATTR_SAUINT32T;
	if (strcmp(str, "SA_INT64_T") == 0)
		return SA_IMM_ATTR_SAINT64T;
	if (strcmp(str, "SA_UINT64_T") == 0)
		return SA_IMM_ATTR_SAUINT64T;
	if (strcmp(str, "SA_TIME_T") == 0)
		return SA_IMM_ATTR_SATIMET;
	if (strcmp(str, "SA_NAME_T") == 0)
		return SA_IMM_ATTR_SANAMET;
	if (strcmp(str, "SA_FLOAT_T") == 0)
		return SA_IMM_ATTR_SAFLOATT;
	if (strcmp(str, "SA_DOUBLE_T") == 0)
		return SA_IMM_ATTR_SADOUBLET;
	if (strcmp(str, "SA_STRING_T") == 0)
		return SA_IMM_ATTR_SASTRINGT;
	if (strcmp(str, "SA_ANY_T") == 0)
		return SA_IMM_ATTR_SAANYT;

	return -1;
}

static int init_param(SaImmAdminOperationParamsT_2 *param, char *arg)
{
	int res = 0;
	char *attrValue;
	char *tmp = strdup(arg);

	if ((param->paramName = strtok(tmp, PARAMDELIM)) == NULL) {
		res = -1;
		goto done;
	}

	if ((param->paramType = str2_saImmValueTypeT(strtok(NULL, PARAMDELIM))) == -1) {
		res = -1;
		goto done;
	}

	if ((attrValue = strtok(NULL, PARAMDELIM)) == NULL) {
		res = -1;
		goto done;
	}

	param->paramBuffer = immutil_new_attrValue(param->paramType, attrValue);

	if (param->paramBuffer == NULL)
		return -1;

 done:
	return res;
}

#ifdef HAVE_TESTS
#define FD_IMM_OI 0
static struct pollfd fds[1];
static nfds_t nfds = 1;
static SaImmOiHandleT immOiHandle = 0LL;
static SaSelectionObjectT immOiSelectionObject = 0;



static SaAisErrorT saImmOiCcbObjectModifyCallback(SaImmOiHandleT immOiHandle,
						  SaImmOiCcbIdT ccbId,
						  const SaNameT *objectName, const SaImmAttrModificationT_2 **attrMods)
{
	SaAisErrorT rc = SA_AIS_OK;
	printf("Modify callback on %s - object:%s ccbId:%llu\n", implName, objectName->value, ccbId);

	struct CcbUtilCcbData *ccbUtilCcbData;
	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) == NULL) {
			fprintf(stderr, "Failed to get CCB objectfor %llu\n", ccbId);
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}

	/* "memorize the modification request" */
	ccbutil_ccbAddModifyOperation(ccbUtilCcbData, objectName, attrMods);

	/*rc = SA_AIS_ERR_BAD_OPERATION;*/

 done:
	return rc;
}

static SaAisErrorT saImmOiCcbObjectCreateCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId,
	const SaImmClassNameT className, const SaNameT *parentName, const SaImmAttrValuesT_2 **attr)
{
	SaAisErrorT rc = SA_AIS_OK;
	struct CcbUtilCcbData *ccbUtilCcbData;

	printf("Create callback on %s - parent:%s ccbId:%llu\n", implName, parentName->value, ccbId);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) == NULL) {
			fprintf(stderr, "Failed to get CCB object for %llu\n", ccbId);
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}



	ccbutil_ccbAddCreateOperation(ccbUtilCcbData, className, parentName, attr);

 done:
	return SA_AIS_OK;
}

static SaAisErrorT saImmOiCcbObjectDeleteCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId,
	const SaNameT *objectName)
{
	SaAisErrorT rc = SA_AIS_OK;
	struct CcbUtilCcbData *ccbUtilCcbData;
	printf("Delete callback on %s - object:%s ccbId:%llu\n", implName, objectName->value, ccbId);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) == NULL) {
			fprintf(stderr, "Failed to get CCB object for %llu\n", ccbId);
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}
	ccbutil_ccbAddDeleteOperation(ccbUtilCcbData, objectName);

 done:
	return rc;
}


static SaAisErrorT saImmOiCcbCompletedCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	printf("Completed callback on %s - ccbId:%llu\n", implName, ccbId);
	return SA_AIS_OK;
}

static void saImmOiCcbAbortCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	struct CcbUtilCcbData *ccbUtilCcbData;

	printf("ABORT callbackon %s. Cleanup CCB %llu\n", implName, ccbId);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) != NULL)
		/* Verify nok outcome with ccbUtilCcbData->userData */
		ccbutil_deleteCcbData(ccbUtilCcbData);
	else
		fprintf(stderr, "Failed to get CCB object for %llu\n", ccbId);
}


static void saImmOiCcbApplyCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	struct CcbUtilCcbData *ccbUtilCcbData;
	struct CcbUtilOperationData *ccbUtilOperationData;

	printf("APPLY CALLBACK on %s cleanup CCB:%llu\n", implName, ccbId);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		fprintf(stderr, "Failed to get CCB object for %llu\n", ccbId);
		goto done;
	}

	ccbUtilOperationData = ccbUtilCcbData->operationListHead;

	ccbutil_deleteCcbData(ccbUtilCcbData);
 done:
	return;
}


static const SaImmOiCallbacksT_2 callbacks = {
	.saImmOiAdminOperationCallback = NULL,
	.saImmOiCcbAbortCallback = saImmOiCcbAbortCallback,
	.saImmOiCcbApplyCallback = saImmOiCcbApplyCallback,
	.saImmOiCcbCompletedCallback = saImmOiCcbCompletedCallback,
	.saImmOiCcbObjectCreateCallback = saImmOiCcbObjectCreateCallback,
	.saImmOiCcbObjectDeleteCallback = saImmOiCcbObjectDeleteCallback,
	.saImmOiCcbObjectModifyCallback = saImmOiCcbObjectModifyCallback,
	.saImmOiRtAttrUpdateCallback = NULL
};
#endif

int main(int argc, char *argv[])
{
	int c;
	struct option long_options[] = {
		{"parameter", required_argument, 0, 'p'},
		{"operation-id", required_argument, 0, 'o'},
		{"help", no_argument, 0, 'h'},
		{"applier", required_argument, 0, 'a'},
		{0, 0, 0, 0}
	};
	SaAisErrorT error;
	SaImmHandleT immHandle;
	SaImmAdminOwnerNameT adminOwnerName = basename(argv[0]);
	SaImmAdminOwnerHandleT ownerHandle;
	SaNameT objectName;
	const SaNameT *objectNames[] = { &objectName, NULL };
	SaAisErrorT operationReturnValue = -1;
	SaImmAdminOperationParamsT_2 *param;
	const SaImmAdminOperationParamsT_2 **params;
	SaImmAdminOperationIdT operationId = -1;

	int params_len = 0;

	params = realloc(NULL, sizeof(SaImmAdminOperationParamsT_2 *));
	params[0] = NULL;

	while (1) {
		c = getopt_long(argc, argv, "p:o:a:h", long_options, NULL);

		if (c == -1)	/* have all command-line options have been parsed? */
			break;

		switch (c) {
		case 'o':
			operationId = strtoll(optarg, (char **)NULL, 10);
			if ((operationId == 0) && ((errno == EINVAL) || (errno == ERANGE))) {
				fprintf(stderr, "Illegal operation ID\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'p':
			params_len++;
			params = realloc(params, (params_len + 1) * sizeof(SaImmAdminOperationParamsT_2 *));
			param = malloc(sizeof(SaImmAdminOperationParamsT_2));
			params[params_len - 1] = param;
			params[params_len] = NULL;
			if (init_param(param, optarg) == -1) {
				fprintf(stderr, "Illegal parameter: %s\n", optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'h':
			usage(basename(argv[0]));
			exit(EXIT_SUCCESS);
			break;
		case 'a':
#ifdef HAVE_TESTS
			implName = strdup(optarg);
			operationId = 0;
			if ((errno == EINVAL) || (errno == ERANGE)) {
				fprintf(stderr, "Illegal applier implementer name\n");
				exit(EXIT_FAILURE);
			}
			break;
#else
			fprintf(stderr, "Implementer/applier '-a' only available with 'configure --enable-tests'\n");
			exit(EXIT_FAILURE);
#endif
		default:
			fprintf(stderr, "Try '%s --help' for more information\n", argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (operationId == -1) {
		fprintf(stderr, "error - must specify numerical operation ID\n");
		exit(EXIT_FAILURE);
	}

	/* Need at least one object to operate on */
	if ((argc - optind) == 0) {
		fprintf(stderr, "error - wrong number of arguments\n");
		exit(EXIT_FAILURE);
	}

	if(implName) goto applier;

	error = saImmOmInitialize(&immHandle, NULL, &immVersion);
	if (error != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmInitialize FAILED: %s\n", saf_error(error));
		exit(EXIT_FAILURE);
	}

	error = saImmOmAdminOwnerInitialize(immHandle, adminOwnerName, SA_TRUE, &ownerHandle);
	if (error != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmAdminOwnerInitialize FAILED: %s\n", saf_error(error));
		exit(EXIT_FAILURE);
	}

	/* Remaining arguments should be object names on which the admin op should be performed. */
	while (optind < argc) {
		strncpy((char *)objectName.value, argv[optind], SA_MAX_NAME_LENGTH);
		objectName.length = strlen((char *)objectName.value);

		error = saImmOmAdminOwnerSet(ownerHandle, objectNames, SA_IMM_ONE);
		if (error != SA_AIS_OK) {
			if (error == SA_AIS_ERR_NOT_EXIST)
				fprintf(stderr, "error - saImmOmAdminOwnerSet - object '%s' does not exist\n",
					objectName.value);
			else
				fprintf(stderr, "error - saImmOmAdminOwnerSet FAILED: %s\n", saf_error(error));
			exit(EXIT_FAILURE);
		}

		error = saImmOmAdminOperationInvoke_2(ownerHandle, &objectName, 0, operationId,
			params, &operationReturnValue, SA_TIME_ONE_SECOND * 60);

		if (error != SA_AIS_OK) {
			fprintf(stderr, "error - saImmOmAdminOperationInvoke_2 FAILED: %s\n",
				saf_error(error));
			exit(EXIT_FAILURE);
		}

		if (operationReturnValue != SA_AIS_OK) {
			fprintf(stderr, "error - saImmOmAdminOperationInvoke_2 admin-op RETURNED: %s\n",
				saf_error(operationReturnValue));
			exit(EXIT_FAILURE);
		}

		error = saImmOmAdminOwnerRelease(ownerHandle, objectNames, SA_IMM_ONE);
		if (error != SA_AIS_OK) {
			fprintf(stderr, "error - saImmOmAdminOwnerRelease FAILED: %s\n", saf_error(error));
			exit(EXIT_FAILURE);
		}

		optind++;
	}

	error = saImmOmAdminOwnerFinalize(ownerHandle);
	if (SA_AIS_OK != error) {
		fprintf(stderr, "error - saImmOmAdminOwnerFinalize FAILED: %s\n", saf_error(error));
		exit(EXIT_FAILURE);
	}

	error = saImmOmFinalize(immHandle);
	if (SA_AIS_OK != error) {
		fprintf(stderr, "error - saImmOmFinalize FAILED: %s\n", saf_error(error));
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);

 applier:
#ifdef HAVE_TESTS
	printf("Implementer: %s\n", implName);
	error = saImmOiInitialize_2(&immOiHandle, &callbacks, &immVersion);
	if (error != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOiInitialize FAILED: %s\n", saf_error(error));
		exit(EXIT_FAILURE);
	}
	printf("ImmVersion: %c %u %u\n", immVersion.releaseCode, immVersion.majorVersion, immVersion.minorVersion);

	error = saImmOiImplementerSet(immOiHandle, implName);
	if (error != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOiImplementerSet FAILED: %s\n", saf_error(error));
		exit(EXIT_FAILURE);
	}

	/* Remaining arguments should be class names for which implementer is set. */
	while (optind < argc) {
		strncpy((char *)objectName.value, argv[optind], SA_MAX_NAME_LENGTH);
		objectName.length = strlen((char *)objectName.value);

		printf("Class: %s\n", objectName.value);
		error = saImmOiClassImplementerSet(immOiHandle, (SaImmClassNameT) objectName.value);
		if (error != SA_AIS_OK) {
			fprintf(stderr, "error - saImmOiClassImplementerSet FAILED: %s\n", saf_error(error));
			exit(EXIT_FAILURE);
		}

		optind++;
	}

	/* go int odispatch loop */
	error = saImmOiSelectionObjectGet(immOiHandle, &immOiSelectionObject);
	if (error != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOiSelectionObjectGet FAILED: %s\n", saf_error(error));
		exit(EXIT_FAILURE);
	}

	fds[FD_IMM_OI].fd = immOiSelectionObject;
	fds[FD_IMM_OI].events = POLLIN;

        while(1) {
		int ret = poll(fds, nfds, -1);
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			fprintf(stderr, "poll failed - %s\n", strerror(errno));
			break;
		}

		if (immOiHandle && fds[FD_IMM_OI].revents & POLLIN) {
			error = saImmOiDispatch(immOiHandle, SA_DISPATCH_ALL);

			if (error == SA_AIS_ERR_BAD_HANDLE) {
				fprintf(stderr, "saImmOiDispatch returned BAD_HANDLE\n");
				immOiHandle = 0;
				break;
			}
		}
	}

#endif	

	exit(EXIT_SUCCESS);
}
