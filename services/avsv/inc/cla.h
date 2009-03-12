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

  This module is the main include file for Cluster Membership Agent (CLA).
  
******************************************************************************
*/

#ifndef CLA_H
#define CLA_H


#include "avsv.h"
#include "avnd.h"
#include "cla_dl_api.h"

/* Porting Include Files */
#include "avsv_n2clamem.h"
#include "cla_def.h"
#include "cla_mem.h"

#include "avsv_clmparam.h"
#include "avsv_n2clamsg.h"
#include "avsv_n2claedu.h"

#include "cla_hdl.h"
#include "cla_mds.h"
#include "cla_cb.h"
#include "cla_log.h"

/* CLA CB global handle declaration */
EXTERN_C uns32 gl_cla_hdl;

#endif /* !CLA_H */
