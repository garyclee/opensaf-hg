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
 * Author(s): Ericsson AB
 *
 */

/*****************************************************************************
  DESCRIPTION:

  This module is the main include file for the entire Imm Service.
*****************************************************************************/

#ifndef IMMSV_H
#define IMMSV_H

#include "ncsgl_defs.h"

#include "ncs_saf.h"
#include "ncs_lib.h"
#include "mds_papi.h"
#include "ncs_edu_pub.h"
#include "ncssysf_lck.h"
#include "ncssysf_mem.h"
#include "ncssysf_def.h"
#include "ncs_main_papi.h"

#include <logtrace.h>

#include "immsv_evt.h"
/* IMMSV Common Macros */

/*** Macro used to get the AMF version used ****/
#define m_IMMSV_GET_AMF_VER(amf_ver) amf_ver.releaseCode='B'; amf_ver.majorVersion=0x01; amf_ver.minorVersion=0x01;


#define m_IMMSV_CONVERT_SATIME_TEN_MILLI_SEC(t)      (t)/(10000000)	/* 10^7 */

#endif   /* IMMSV_H */
