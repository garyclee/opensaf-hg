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

/*****************************************************************************
..............................................................................

  MODULE NAME: rde_amf.h

..............................................................................

  DESCRIPTION: Contains definitions of the RDE AMF structures

..............................................................................

******************************************************************************
*/

#ifndef RDE_AMF_H
#define RDE_AMF_H

/*
 * Includes
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "ncs_main_papi.h"
#include "saAis.h"
#include "saAmf.h"

/*
 * Macro used to get the AMF version used 
 */
#define m_RDE_GET_AMF_VER(amf_ver) amf_ver.releaseCode='B'; amf_ver.majorVersion=0x01; amf_ver.minorVersion=0x00;

/*
 * RDE AMF control information
 */
typedef struct {
	char comp_name[256];
	SaAmfHandleT amf_hdl;	/* AMF handle */
	SaSelectionObjectT amf_fd;	/* AMF selection fd */
	bool is_amf_up;	/* For amf_fd and pipe_fd */

} RDE_AMF_CB;

extern uint32_t rde_amf_init(RDE_AMF_CB *rde_amf_cb);

#endif
