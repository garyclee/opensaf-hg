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

  This module contains the extern declaration for registering
  Availability Agent's canned strings with Dtsv server.
  
******************************************************************************
*/

#ifndef GLSV_LOGSTR_H
#define GLSV_LOGSTR_H

#include "glsv_defs.h"

#if (NCS_DTS == 1)
uns32 glnd_reg_strings(void);
uns32 gla_reg_strings(void);
uns32 gld_reg_strings(void);
uns32 glnd_unreg_strings(void);
uns32 gld_unreg_strings(void);
uns32 gla_unreg_strings(void);
uns32 glsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
#endif   /* (NCS_DTS == 1) */

#endif   /* !AVA_LOGSTR_H */
