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
	struct avd_su_tag *list_of_su;
};

/**
 * Get SaAmfSUType from IMM and create internal objects
 * 
 * @return SaAisErrorT
 */
extern SaAisErrorT avd_sutype_config_get(void);

/**
 * Get SaAmfSUType object using given key
 * @param dn
 * 
 * @return struct avd_sutype*
 */
extern struct avd_sutype *avd_sutype_get(const SaNameT *dn);

/**
 * Class constructor, must be called before any other function
 */
extern void avd_sutype_constructor(void);

/**
 * Add SU to SU Type internal list
 * @param su
 */
extern void avd_sutype_add_su(struct avd_su_tag* su);

/**
 * Remove SU from SU Type internal list
 * @param su
 */
extern void avd_sutype_remove_su(struct avd_su_tag* su);

#endif
