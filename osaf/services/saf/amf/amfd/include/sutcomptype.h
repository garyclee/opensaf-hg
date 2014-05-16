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
 * Author(s): Ericsson
 *
 */

/*****************************************************************************

  DESCRIPTION: SuTypeCompType class definition
  
******************************************************************************
*/

#ifndef AVD_SUCOMPTYPE_H
#define AVD_SUCOMPTYPE_H

#include <saAmf.h>
#include <ncspatricia.h>
#include "sutype.h"

typedef struct {
	NCS_PATRICIA_NODE tree_node;	/* key is name */
	SaNameT name;
	SaUint32T saAmfSutMaxNumComponents;
	SaUint32T saAmfSutMinNumComponents;
	SaUint32T curr_num_components;
} AVD_SUTCOMP_TYPE;

AVD_SUTCOMP_TYPE *avd_sutcomptype_get(const SaNameT *dn);
SaAisErrorT avd_sutcomptype_config_get(SaNameT *sutype_name, struct avd_sutype *sut);
void avd_sutcomptype_constructor(void);

#endif
