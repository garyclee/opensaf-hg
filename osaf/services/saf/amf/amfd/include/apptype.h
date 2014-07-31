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

  DESCRIPTION: Application type class
  
****************************************************************************/
#ifndef AVD_APPTYPE_H
#define AVD_APPTYPE_H

#include <saAmf.h>
#include <saImm.h>

#include <sg.h>
#include <si.h>
#include "db_template.h"

class AVD_APP;

typedef struct avd_app_type_tag {
	SaNameT name;
	SaNameT *sgAmfApptSGTypes;
	uint32_t no_sg_types;
	AVD_APP *list_of_app;
} AVD_APP_TYPE;

extern AVD_APP_TYPE *avd_apptype_get(const SaNameT *dn);
extern void avd_apptype_add_app(AVD_APP *app);
extern void avd_apptype_remove_app(AVD_APP *app);
extern SaAisErrorT avd_apptype_config_get(void);
extern void avd_apptype_constructor(void);

#endif
