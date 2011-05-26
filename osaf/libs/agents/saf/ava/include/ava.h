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

..............................................................................

  DESCRIPTION:

  This module is the main include file for Availability Agent (AvA).
  
******************************************************************************
*/

#ifndef AVA_H
#define AVA_H
#include <ncs_main_papi.h>
#include "avsv.h"
#include "ava_dl_api.h"

/* Porting Include Files */
#include "ava_def.h"

#include "avsv_amfparam.h"
#include "avsv_n2avamsg.h"
#include "avsv_n2avaedu.h"

#include "ava_hdl.h"
#include "ava_mds.h"
#include "ava_cb.h"

#include<logtrace.h>

/* AvA CB global handle declaration */
uint32_t gl_ava_hdl;

#endif   /* !AVA_H */
