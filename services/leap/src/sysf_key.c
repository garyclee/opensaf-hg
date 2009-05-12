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

  MODULE NAME: SYSF_KEY.C



..............................................................................

  DESCRIPTION:


  NOTES:
  

******************************************************************************
*/

#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncs_svd.h"
#include "sysf_key.h"

/* Array for storing the Subsystem Key */
NCS_KEY  subsys_list[NCS_SERVICE_ID_MAX][MAX_VR];
