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

#ifndef AVD_SUTYPE_H
#define AVD_SUTYPE_H

#include <saAis.h>
#include <ncspatricia.h>
#include <su.h>

struct avd_sutype {
	NCS_PATRICIA_NODE tree_node; /* key will be su type name */
	SaNameT name;
	SaUint32T saAmfSutIsExternal;
	SaUint32T saAmfSutDefSUFailover;
	SaNameT *saAmfSutProvidesSvcTypes; /* array of DNs, size in number_svc_types */
	unsigned int number_svc_types;	/* size of array saAmfSutProvidesSvcTypes */
	AVD_SU *list_of_su;
};
extern AmfDb<std::string, avd_sutype> *sutype_db;

/**
 * Get SaAmfSUType from IMM and create internal objects
 * 
 * @return SaAisErrorT
 */
extern SaAisErrorT avd_sutype_config_get(void);

/**
 * Class constructor, must be called before any other function
 */
extern void avd_sutype_constructor(void);

/**
 * Add SU to SU Type internal list
 * @param su
 */
extern void avd_sutype_add_su(AVD_SU* su);

/**
 * Remove SU from SU Type internal list
 * @param su
 */
extern void avd_sutype_remove_su(AVD_SU* su);

#endif
