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
 * Author(s): Emerson Network Power
 *
 */
/****************************************************************************

  DESCRIPTION:

  This module is the include file for Sa Amf Application object management.
  
****************************************************************************/
#ifndef AVD_APP_H
#define AVD_APP_H

#include <saAmf.h>
#include <saImm.h>

#include <sg.h>
#include <si.h>
#include "db_template.h"

typedef struct avd_app_type_tag {
	SaNameT name;
	SaNameT *sgAmfApptSGTypes;
	uint32_t no_sg_types;
	struct avd_app_tag *list_of_app;
} AVD_APP_TYPE;

// TODO (hafe) change to class AmfApp
typedef struct avd_app_tag {
	SaNameT name;
	SaNameT saAmfAppType;
	SaAmfAdminStateT saAmfApplicationAdminState;
	SaUint32T saAmfApplicationCurrNumSGs;
	struct avd_sg_tag *list_of_sg;
	struct avd_si_tag *list_of_si;
	struct avd_app_tag *app_type_list_app_next;
	struct avd_app_type_tag *app_type;
} AVD_APP;

extern AmfDb<AVD_APP> *app_db;

extern void avd_app_add_si(AVD_APP *app, struct avd_si_tag *si);
extern void avd_app_remove_si(AVD_APP *app, struct avd_si_tag *si);
extern void avd_app_add_sg(AVD_APP *app, struct avd_sg_tag *sg);
extern void avd_app_remove_sg(AVD_APP *app, struct avd_sg_tag *sg);
extern SaAisErrorT avd_app_config_get(void);
extern void avd_app_constructor(void);

extern AVD_APP_TYPE *avd_apptype_get(const SaNameT *dn);
extern void avd_apptype_add_app(AVD_APP *app);
extern void avd_apptype_remove_app(AVD_APP *app);
extern SaAisErrorT avd_apptype_config_get(void);
extern void avd_apptype_constructor(void);

#endif
