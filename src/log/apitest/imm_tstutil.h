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

#include "log/apitest/logtest.h"

#ifndef IMM_TSTUTIL_H
#define IMM_TSTUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Handle reading of multivalue of string type from log service configuration
 * object
 */

/**
 * Get one multi value of string type (SA_IMM_ATTR_SASTRINGT) from the
 * "logConfig=1,safApp=safLogService" object
 *
 * See also get_attr_value() for other non multivalue attributes
 *
 * Note: Memory is allocated for the multivalue_array that has to be freed
 *       after usage. Use free_multivalue_array()
 *
 * @param attribute_name[in]
 * @param multivalue_array[out] NULL terminated array of strings (char *)
 * @return false if Fail. Fail message is printed on stdout
 */
bool get_multivalue_type_string_from_imm(SaImmHandleT *omHandle,
                                         SaConstStringT objectName,
                                         char *attribute_name,
                                         char ***multivalue_array);

/**
 * Used to free memory allocated for the multivalue_array by function
 * get_multivalue_string_type_from_imm()
 *
 * @param multivalue_array[in]
 */
void free_multivalue(SaImmHandleT omHandle, char ***multivalue_array);

#ifdef __cplusplus
}
#endif

#endif /* IMM_TSTUTIL_H */
