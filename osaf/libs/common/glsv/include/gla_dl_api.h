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

#ifndef GLA_DL_API_H
#define GLA_DL_API_H

GLADLL_API uns32 gla_lib_req(NCS_LIB_REQ_INFO *req_info);
EXTERN_C unsigned int ncs_gla_startup(void);
EXTERN_C unsigned int ncs_gla_shutdown(void);

#endif
