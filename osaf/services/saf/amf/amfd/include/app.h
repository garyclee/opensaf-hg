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

#include <include/apptype.h>
#include <sg.h>
#include <si.h>
#include "db_template.h"

class AVD_APP {

public:
	SaNameT name;
	SaNameT saAmfAppType;
	SaAmfAdminStateT saAmfApplicationAdminState;
	SaUint32T saAmfApplicationCurrNumSGs;
	AVD_SG *list_of_sg;
	AVD_SI *list_of_si;
	AVD_APP *app_type_list_app_next;
	struct avd_app_type_tag *app_type;
	
	AVD_APP();
	explicit AVD_APP(const SaNameT *dn);
	~AVD_APP();
	
	void add_si(AVD_SI *si);
	void remove_si(AVD_SI *si);
	void add_sg(AVD_SG *sg);
	void remove_sg(AVD_SG *sg);

private:
	AVD_APP(const AVD_APP&);
	AVD_APP& operator=(const AVD_APP&);
};

extern AmfDb<std::string, AVD_APP> *app_db;

extern SaAisErrorT avd_app_config_get(void);
extern void avd_app_constructor(void);

#endif
