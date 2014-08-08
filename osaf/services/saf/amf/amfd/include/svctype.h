/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2014 The OpenSAF Foundation
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

#ifndef SVCTYPE_H
#define SVCTYPE_H


typedef struct avd_amf_svc_type_tag {

	SaNameT name;
	char **saAmfSvcDefActiveWeight;
	char **saAmfSvcDefStandbyWeight;
	AVD_SI *list_of_si;

} AVD_SVC_TYPE;

SaAisErrorT avd_svctype_config_get(void);
void avd_svctype_add_si(AVD_SI *si);
void avd_svctype_remove_si(AVD_SI *si);
void avd_svctype_constructor(void);

extern AmfDb<std::string, AVD_SVC_TYPE> *svctype_db;

#endif
