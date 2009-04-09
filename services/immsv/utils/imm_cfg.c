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

#define PARAMDELIM "="

static SaVersionT immVersion = {'A', 2, 1};

/**
 * 
 */
static void usage(const char *progname)
{
    printf("\nNAME\n");
    printf("\t%s - configure attribute(s) for an IMM object\n", progname);

    printf("\nSYNOPSIS\n");
    printf("\t%s [options] <object name>\n", progname);

    printf("\nDESCRIPTION\n");
    printf("\t%s is an IMM OM client used to configure attribute(s) for an IMM object\n", progname);

    printf("\nOPTIONS\n");
    printf("  -h or --help                    this help\n");
    printf("  -i  or -- <object name> )\n");
    printf("  -a name=value or --attribute name=value <object name> )\n");

    printf("\nEXAMPLE\n");
    printf("   immcfg -a saAmfNodeSuFailoverMax=7 \"safAmfNode=Node01,safAmfCluster=1\"\n");
}

static SaImmAttrModificationT_2 *new_attrMod(SaNameT *objectName, char *arg)
{
    int res = 0;
    char *tmp = strdup(arg);
    char *name, *value;
    SaImmAttrModificationT_2 *attrMod;

    attrMod = malloc(sizeof(SaImmAttrModificationT_2));

    if ((name = strtok(tmp, PARAMDELIM)) == NULL)
    {
        res = -1;
        goto done;
    }

    attrMod->modType = SA_IMM_ATTR_VALUES_REPLACE;
    attrMod->modAttr.attrName = strdup(name);
    if (immutil_get_attrValueType(objectName, attrMod->modAttr.attrName, &attrMod->modAttr.attrValueType) != SA_AIS_OK)
    {
        fprintf(stderr, "Invalid attribute name: %s\n", name);
        free(attrMod);
        attrMod = NULL;
        goto done;
    }

    attrMod->modAttr.attrValuesNumber = 1;

    if ((value = strtok(NULL, PARAMDELIM)) == NULL)
    {
        fprintf(stderr, "Attribute value missing for: %s\n", arg);
        free(attrMod);
        attrMod = NULL;
        goto done;
    }

    attrMod->modAttr.attrValues = malloc(sizeof(SaImmAttrValueT *));
    attrMod->modAttr.attrValues[0] = immutil_new_attrValue(
        attrMod->modAttr.attrValueType, value);

 done:
    free(tmp);
    return attrMod;
}

int main(int argc, char *argv[])
{
    int rc = EXIT_SUCCESS;
    int c;
    struct option long_options[] =
    {
        {"attribute", required_argument, 0, 'a'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    SaAisErrorT error;
    SaImmHandleT immHandle;
    SaImmAdminOwnerNameT adminOwnerName = basename(argv[0]);
    SaImmAdminOwnerHandleT ownerHandle;
    SaNameT objectName;
    const SaNameT *objectNames[] = {&objectName, NULL};
    SaImmCcbFlagsT ccbFlags = SA_IMM_CCB_REGISTERED_OI;
    SaImmCcbHandleT ccbHandle;
    int i;
    int optargs_len = 0; /* one off */
    char **optargs = NULL;
    SaImmAttrModificationT_2 *attrMod;
    SaImmAttrModificationT_2 **attrMods = NULL;
    int attrMods_len = 1;

    while (1)
    {
        c = getopt_long(argc, argv, "a:h", long_options, NULL);

        if (c == -1) /* have all command-line options have been parsed? */
            break;

        switch (c)
        {
            case 'a':
                optargs = realloc(optargs, ++optargs_len * sizeof(char*));
                optargs[optargs_len - 1] = strdup(optarg);
                break;
            case 'h':
                usage(basename(argv[0]));
                exit(EXIT_SUCCESS);
                break;
            default:
                fprintf(stderr, "Try '%s --help' for more information\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    /* Can only handle one object currently */
    if ((argc - optind) != 1)
    {
        fprintf(stderr, "error - wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    strncpy((char *) objectName.value, argv[optind], SA_MAX_NAME_LENGTH);
    objectName.length = strnlen((char *) objectName.value, SA_MAX_NAME_LENGTH);

    for (i = 0; i < optargs_len; i++)
    {
        attrMods = realloc(attrMods, (attrMods_len + 1) * sizeof(SaImmAttrModificationT_2*));
        if ((attrMod = new_attrMod(&objectName, optargs[i])) == NULL)
            exit(EXIT_FAILURE);

        attrMods[attrMods_len - 1] = attrMod;
        attrMods[attrMods_len] = NULL;
        attrMods_len++;
    }

    (void) immutil_saImmOmInitialize(&immHandle, NULL, &immVersion);

    error = saImmOmAdminOwnerInitialize(immHandle, adminOwnerName, SA_TRUE, &ownerHandle);
    if (error != SA_AIS_OK)
    {
        fprintf(stderr, "error - saImmOmAdminOwnerInitialize FAILED: %s\n", saf_error(error));
        rc = EXIT_FAILURE;
        goto done_om_finalize;
    }

    error = saImmOmAdminOwnerSet(ownerHandle, objectNames, SA_IMM_ONE);
    if (error != SA_AIS_OK)
    {
        if (error == SA_AIS_ERR_NOT_EXIST)
            fprintf(stderr, "error - object '%s' does not exist\n", objectName.value);
        else
            fprintf(stderr, "error - saImmOmAdminOwnerSet FAILED: %s\n", saf_error(error));

        rc = EXIT_FAILURE;
        goto done_admin_owner_finalize;
    }

    error = saImmOmCcbInitialize(ownerHandle, ccbFlags, &ccbHandle);
    if (error != SA_AIS_OK)
    {
        fprintf(stderr, "error - saImmOmCcbInitialize FAILED: %s\n", saf_error(error));
        rc = EXIT_FAILURE;
        goto done_admin_owner_release;
    }

    error = saImmOmCcbObjectModify_2(ccbHandle, &objectName, (const SaImmAttrModificationT_2 **) attrMods);
    if (error != SA_AIS_OK)
    {
        fprintf(stderr, "error - saImmOmCcbObjectModify_2 FAILED: %s\n", saf_error(error));
        rc = EXIT_FAILURE;
        goto done_ccb_finalize;
    }

    error = saImmOmCcbApply(ccbHandle);
    if (error != SA_AIS_OK)
    {
        fprintf(stderr, "error - saImmOmCcbApply FAILED: %s\n", saf_error(error));
        exit(EXIT_FAILURE);
    }

done_ccb_finalize:
    error = saImmOmCcbFinalize(ccbHandle);
    if (error != SA_AIS_OK)
    {
        fprintf(stderr, "error - saImmOmCcbFinalize FAILED: %s\n", saf_error(error));
        exit(EXIT_FAILURE);
    }

done_admin_owner_release:
    error = saImmOmAdminOwnerRelease(ownerHandle, objectNames, SA_IMM_SUBTREE);
    if (error != SA_AIS_OK)
    {
        fprintf(stderr, "error - saImmOmAdminOwnerRelease FAILED: %s\n", saf_error(error));
        exit(EXIT_FAILURE);
    }

done_admin_owner_finalize:
    error = saImmOmAdminOwnerFinalize(ownerHandle);
    if (SA_AIS_OK != error)
    {
        fprintf(stderr, "error - saImmOmAdminOwnerFinalize FAILED: %s\n",  saf_error(error));
        exit(EXIT_FAILURE);
    }

done_om_finalize:
    (void) immutil_saImmOmFinalize(immHandle);

    exit(rc);
}

