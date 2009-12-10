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
 * This file contains a command line utility to configure attributes for an IMM object.
 * Example: immcfg [-a attr-name[+|-]=attr-value]+ "safAmfNode=Node01,safAmfCluster=1"
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

static SaVersionT immVersion = { 'A', 2, 1 };

typedef enum {
	INVALID = 0,
	CREATE = 1,
	DELETE = 2,
	MODIFY = 3,
	IMMFILE = 4
} op_t;


// The interface function which implements the -f opton (imm_import.cc)
int importImmXML(char* xmlfileC, char* adminOwnerName, int verbose);


/**
 *
 */
static void usage(const char *progname)
{
	printf("\nNAME\n");
	printf("\t%s - create, delete or modify IMM configuration object(s)\n", progname);

	printf("\nSYNOPSIS\n");
	printf("\t%s [options] [object DN]...\n", progname);

	printf("\nDESCRIPTION\n");
	printf("\t%s is an IMM OM client used to create, delete an IMM or modify attribute(s) for IMM object(s)\n", progname);
	printf("\tThe default operation if none specified is modify.\n");
	printf("\tWhen creating or modifying several objects, they have to be of the same class.");

	printf("\nOPTIONS\n");
	printf("\t-a, --attribute name=value [object DN]... \n");
	printf("\t-c, --create <class name> [object DN]... \n");
	printf("\t-d, --delete [object DN]... \n");
	printf("\t-h, --help                    this help\n");
	printf("\t-m, --modify [object DN]... \n");
	printf("\t-v, --verbose (only valid with -f/--file option)\n");
	printf("\t-f, --file <imm.xml file containing classes and/or objects>\n");

	printf("\nEXAMPLE\n");
	printf("\timmcfg -a saAmfNodeSuFailoverMax=7 safAmfNode=Node01,safAmfCluster=1\n");
	printf("\t\tchange one attribute for one object\n");
	printf("\timmcfg -c SaAmfApplication -a saAmfAppType=Test safApp=myTestApp1\n");
	printf("\t\tcreate one object setting one initialized attribute\n");
	printf("\timmcfg -d safAmfNode=Node01,safAmfCluster=1\n");
	printf("\t\tdelete one object\n");
	printf("\timmcfg -d safAmfNode=Node01,safAmfCluster=1 safAmfNode=Node02,safAmfCluster=1\n");
	printf("\t\tdelete two objects\n");
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
static SaImmAttrValuesT_2 *new_attr_value(const SaImmClassNameT className, char *nameval)
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
	attrValue->attrValues[0] = immutil_new_attrValue(attrValue->attrValueType, value);

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
int object_create(const SaNameT **objectNames, const SaImmClassNameT className,
	SaImmAdminOwnerHandleT ownerHandle, char **optargs, int optargs_len)
{
	SaAisErrorT error;
	int i;
	SaImmAttrValuesT_2 *attrValue;
	SaImmAttrValuesT_2 **attrValues = NULL;
	int attr_len = 1;
	int rc = EXIT_FAILURE;
	char *parent = NULL;
	SaNameT parentName;
	char *str, *delim;
	const SaNameT *parentNames[] = {&parentName, NULL};
	SaImmCcbHandleT ccbHandle;

	for (i = 0; i < optargs_len; i++) {
		attrValues = realloc(attrValues, (attr_len + 1) * sizeof(SaImmAttrValuesT_2 *));
		if ((attrValue = new_attr_value(className, optargs[i])) == NULL)
			goto done;

		attrValues[attr_len - 1] = attrValue;
		attrValues[attr_len] = NULL;
		attr_len++;
	}

	if ((error = saImmOmCcbInitialize(ownerHandle, 0, &ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbInitialize FAILED: %s\n", saf_error(error));
		goto done;
	}

	i = 0;
	while (objectNames[i] != NULL) {
		str = strdup((char*)objectNames[i]->value);
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

			if ((error = saImmOmAdminOwnerSet(ownerHandle, parentNames, SA_IMM_SUBTREE)) != SA_AIS_OK) {
				if (error == SA_AIS_ERR_NOT_EXIST)
					fprintf(stderr, "error - parent '%s' does not exist\n", parentName.value);
				else {
					fprintf(stderr, "error - saImmOmAdminOwnerSet FAILED: %s\n", saf_error(error));
					goto done;
				}
			}
		}

		attrValues = realloc(attrValues, (attr_len + 1) * sizeof(SaImmAttrValuesT_2 *));
		attrValue = new_attr_value(className, str);
		attrValues[attr_len - 1] = attrValue;
		attrValues[attr_len] = NULL;

		if ((error = saImmOmCcbObjectCreate_2(ccbHandle, className, &parentName,
			(const SaImmAttrValuesT_2**)attrValues)) != SA_AIS_OK) {
			fprintf(stderr, "error - saImmOmCcbObjectCreate_2 FAILED with %s\n",
				saf_error(error));
			goto done;
		}
		i++;
	}

	if ((error = saImmOmCcbApply(ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbApply FAILED: %s\n", saf_error(error));
		goto done_release;
	}

	if ((error = saImmOmCcbFinalize(ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbFinalize FAILED: %s\n", saf_error(error));
		goto done_release;
	}

	rc = 0;

done_release:
	if (parent && (error = saImmOmAdminOwnerRelease(
		ownerHandle, (const SaNameT **)parentNames, SA_IMM_SUBTREE)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmAdminOwnerRelease FAILED: %s\n", saf_error(error));
		goto done;
	}
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
int object_modify(const SaNameT **objectNames, SaImmAdminOwnerHandleT ownerHandle, char **optargs, int optargs_len)
{
	SaAisErrorT error;
	int i;
	int attr_len = 1;
	int rc = EXIT_FAILURE;
	SaImmAttrModificationT_2 *attrMod;
	SaImmAttrModificationT_2 **attrMods = NULL;
	SaImmCcbHandleT ccbHandle;

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
			fprintf(stderr, "error - saImmOmAdminOwnerSet FAILED: %s\n", saf_error(error));

		goto done;
	}

	if ((error = saImmOmCcbInitialize(ownerHandle, 0, &ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbInitialize FAILED: %s\n", saf_error(error));
		goto done_release;
	}

	i = 0;
	while (objectNames[i] != NULL) {
		if ((error = saImmOmCcbObjectModify_2(ccbHandle, objectNames[i],
			(const SaImmAttrModificationT_2 **)attrMods)) != SA_AIS_OK) {
			fprintf(stderr, "error - saImmOmCcbObjectModify_2 FAILED: %s\n", saf_error(error));
			goto done_release;
		}
		i++;
	}

	if ((error = saImmOmCcbApply(ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbApply FAILED: %s\n", saf_error(error));
		goto done_release;
	}

	if ((error = saImmOmCcbFinalize(ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbFinalize FAILED: %s\n", saf_error(error));
		goto done_release;
	}

	rc = 0;

 done_release:
	if ((error = saImmOmAdminOwnerRelease(ownerHandle, (const SaNameT **)objectNames, SA_IMM_ONE)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmAdminOwnerRelease FAILED: %s\n", saf_error(error));
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
int object_delete(const SaNameT **objectNames, SaImmAdminOwnerHandleT ownerHandle)
{
	SaAisErrorT error;
	int rc = EXIT_FAILURE;
	SaImmCcbHandleT ccbHandle;
	int i = 0;

	if ((error = saImmOmAdminOwnerSet(ownerHandle, (const SaNameT **)objectNames,
		SA_IMM_SUBTREE)) != SA_AIS_OK) {

		if (error == SA_AIS_ERR_NOT_EXIST)
			fprintf(stderr, "error - object does not exist\n");
		else
			fprintf(stderr, "error - saImmOmAdminOwnerSet FAILED: %s\n", saf_error(error));

		goto done;
	}

	if ((error = saImmOmCcbInitialize(ownerHandle, 0, &ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbInitialize FAILED: %s\n", saf_error(error));
		goto done;
	}

	while (objectNames[i] != NULL) {
		if ((error = saImmOmCcbObjectDelete(ccbHandle, objectNames[i])) != SA_AIS_OK) {
			fprintf(stderr, "error - saImmOmCcbObjectDelete for '%s' FAILED: %s\n",
				objectNames[i]->value, saf_error(error));
			goto done;
		}
		i++;
	}

	if ((error = saImmOmCcbApply(ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbApply FAILED: %s\n", saf_error(error));
		goto done;
	}

	if ((error = saImmOmCcbFinalize(ccbHandle)) != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmCcbFinalize FAILED: %s\n", saf_error(error));
		goto done;
	}

	rc = 0;
done:
	return rc;
}

int main(int argc, char *argv[])
{
	int rc = EXIT_SUCCESS;
	int c;
	struct option long_options[] = {
		{"attribute", required_argument, 0, 'a'},
		{"create", required_argument, 0, 'c'},
		{"file", required_argument, 0, 'f'},
		{"delete", no_argument, 0, 'd'},
		{"help", no_argument, 0, 'h'},
		{"modify", no_argument, 0, 'm'},
		{"verbose", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};
	SaAisErrorT error;
	SaImmHandleT immHandle;
	SaImmAdminOwnerNameT adminOwnerName = basename(argv[0]);
	SaImmAdminOwnerHandleT ownerHandle;
	SaNameT **objectNames = NULL;
	int objectNames_len = 1;
	SaNameT *objectName;
	int optargs_len = 0;	/* one off */
	char **optargs = NULL;
        SaImmClassNameT className = NULL;
	op_t op = INVALID;
	char* xmlFilename;
	int verbose = 0;

	while (1) {
		c = getopt_long(argc, argv, "a:c:f:dhmv", long_options, NULL);

		if (c == -1)	/* have all command-line options have been parsed? */
			break;

		switch (c) {
		case 'a':
			optargs = realloc(optargs, ++optargs_len * sizeof(char *));
			optargs[optargs_len - 1] = strdup(optarg);
			break;
		case 'c':
			className = optarg;
			if (op == INVALID)
				op = CREATE;
			else {
				fprintf(stderr, "error - only one operation at a time supported\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'd': {
			if (op == INVALID)
				op = DELETE;
			else {
				fprintf(stderr, "error - only one operation at a time supported\n");
				exit(EXIT_FAILURE);
			}
			break;
		}
		case 'h':
			usage(basename(argv[0]));
			exit(EXIT_SUCCESS);
			break;
		case 'f':
			if (op == INVALID)
				op = IMMFILE;
			else {
				fprintf(stderr, "error - only one operation at a time supported\n");
				exit(EXIT_FAILURE);
			}
			xmlFilename = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'm': {
			if (op == INVALID)
				op = MODIFY;
			else {
				fprintf(stderr, "error - only one operation at a time supported\n");
				exit(EXIT_FAILURE);
			}
			break;
		}
		default:
			fprintf(stderr, "Try '%s --help' for more information\n", argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (op == IMMFILE) {
		rc = importImmXML(xmlFilename, adminOwnerName, verbose);
		exit(rc);
	}

	/* Modify is default */
	if (op == INVALID)
		op = MODIFY;

	/* Remaining arguments should be object names. Need at least one... */
	if ((argc - optind) < 1) {
		fprintf(stderr, "error - specify at least one object\n");
		exit(EXIT_FAILURE);
	}

	while (optind < argc) {
		objectNames = realloc(objectNames, (objectNames_len + 1) * sizeof(SaNameT*));
		objectName = objectNames[objectNames_len - 1] = malloc(sizeof(SaNameT));
		objectNames[objectNames_len++] = NULL;
		objectName->length = snprintf((char*)objectName->value, SA_MAX_NAME_LENGTH, "%s", argv[optind++]);
	}

	(void)immutil_saImmOmInitialize(&immHandle, NULL, &immVersion);

	error = saImmOmAdminOwnerInitialize(immHandle, adminOwnerName, SA_TRUE, &ownerHandle);
	if (error != SA_AIS_OK) {
		fprintf(stderr, "error - saImmOmAdminOwnerInitialize FAILED: %s\n", saf_error(error));
		rc = EXIT_FAILURE;
		goto done_om_finalize;
	}

	switch (op) {
	case CREATE:
		rc = object_create((const SaNameT **)objectNames, className, ownerHandle, optargs, optargs_len);
		break;
	case DELETE:
		rc = object_delete((const SaNameT **)objectNames, ownerHandle);
		break;
	case MODIFY:
		rc = object_modify((const SaNameT **)objectNames, ownerHandle, optargs, optargs_len);
		break;
	default:
		fprintf(stderr, "error - no operation specified\n");
		break;
	}

	error = saImmOmAdminOwnerFinalize(ownerHandle);
	if (SA_AIS_OK != error) {
		fprintf(stderr, "error - saImmOmAdminOwnerFinalize FAILED: %s\n", saf_error(error));
		rc = EXIT_FAILURE;
		goto done_om_finalize;
	}

 done_om_finalize:
	(void)immutil_saImmOmFinalize(immHandle);

	exit(rc);
}
