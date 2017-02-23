/*      -*- OpenSAF  -*-
 *
 * Copyright Ericsson AB [2017] - All Rights Reserved
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
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "imm/saf/saImm.h"
#include "imm/saf/saImmOm.h"
#include "osaf/immutil/immutil.h"
#include "base/saf_error.h"
#include "base/osaf_extended_name.h"

static SaVersionT immVersion = {'A', 2, 11};

bool get_multivalue_type_string_from_imm(SaImmHandleT *omHandle,
					SaConstStringT objectName,
					char *attribute_name,
					char ***multivalue_array) {
	SaAisErrorT om_rc = SA_AIS_OK;
	SaImmAccessorHandleT accessorHandle;
	SaImmAttrValuesT_2 *attribute;
	SaImmAttrValuesT_2 **attributes;
	bool func_rc = true;

	//printf(">> get_multivalue_string_type_from_imm()\n");

	do {
		/* Make sure this is a NULL pointer if no values are found */
		*multivalue_array = NULL;

		om_rc = immutil_saImmOmInitialize(omHandle, NULL, &immVersion);
		if (om_rc != SA_AIS_OK) {
			printf("immutil_saImmOmInitialize Fail '%s'\n",
				saf_error(om_rc));
			func_rc = false;
			break;
		}

		//printf("\timmutil_saImmOmInitialize() Done\n");

		om_rc = immutil_saImmOmAccessorInitialize(*omHandle,
			&accessorHandle);
		if (om_rc != SA_AIS_OK) {
			printf("immutil_saImmOmAccessorInitialize failed: %s\n",
				saf_error(om_rc));
			func_rc = false;
			break;
		}

		//printf("\timmutil_saImmOmAccessorInitialize() Done\n");

		//SaConstStringT objectName = "logConfig=1,safApp=safLogService";
		SaNameT tmpObjName;
		osaf_extended_name_lend(objectName, &tmpObjName);

		/* Get the attribute */
		SaImmAttrNameT attributeNames[2];
		attributeNames[0] = attribute_name;
		attributeNames[1] = NULL;
		om_rc = immutil_saImmOmAccessorGet_2(
			accessorHandle,
			&tmpObjName,
			attributeNames,
			&attributes);
		if (om_rc != SA_AIS_OK) {
			printf("immutil_saImmOmAccessorGet_2 Fail '%s'\n",
				saf_error(om_rc));
			func_rc = false;
			break;
		}

		//printf("\timmutil_saImmOmAccessorGet_2() Done\n");

		attribute = attributes[0];
		char **str_array = NULL;

		/* Get values if there are any */
		if (attribute->attrValuesNumber > 0) {
			size_t array_len = attribute->attrValuesNumber + 1;
			str_array = (char **) calloc(array_len, sizeof(char *));
			str_array[array_len - 1] = NULL; /* NULL terminated array */

			/* Save values */
			void *value = NULL;
			for (uint32_t i = 0; i < attribute->attrValuesNumber;
				i++) {
				value = attribute->attrValues[i];
				str_array[i] = *(char **) value;
			}
		}

		*multivalue_array = str_array;
	} while (0);

	//printf("<< get_multivalue_string_type_from_imm()\n");
	return func_rc;
}

void free_multivalue(SaImmHandleT omHandle, char ***multivalue_array) {
	//printf(">> free_multivalue_array() ptr = %p\n", *multivalue_array);
	if (*multivalue_array == NULL) {
		return;
	}

	free(*multivalue_array);

	SaAisErrorT rc = immutil_saImmOmFinalize(omHandle);
	if (rc != SA_AIS_OK) {
		printf("free_multivalue: immutil_saImmOmFinalize() Fail %s\n",
			saf_error(rc));
	}
	//printf("<< free_multivalue_array()\n");
}
      