/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2016 The OpenSAF Foundation
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

/*****************************************************************************

  DESCRIPTION:


******************************************************************************
*/

#ifndef AMF_SI_ASSIGN_H
#define AMF_SI_ASSIGN_H

#include <string>
#include <saAmf.h>

struct SaAmfSIAssignment {
	public:
		SaNameT su;
		SaNameT si;
		SaAmfHAStateT saAmfSISUHAState;
		uint32_t saAmfSISUHAReadinessState;
};

struct SaAmfCSIAssignment {
	public:
		SaNameT csi;
		SaNameT comp;
		SaAmfHAStateT saAmfCSICompHAState;
		uint32_t saAmfCSICompHAReadinessState;
};

#endif	/* AMF_SI_ASSIGN_H */

