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
 */

#ifndef PDRBD_UTIL_H
#define PDRBD_UTIL_H


#include "pdrbd.h"

extern uns32 pdrbd_script_execute(PSEUDO_CB *,char *,uns32,uns32,uns32);
extern uns32 pdrbd_handle_script_resp(PSEUDO_CB *,PDRBD_EVT *);
extern uns32 pdrbd_process_peer_msg(PSEUDO_CB *cb, PDRBD_EVT *evt);


#endif /* PDRBD_UTIL_H */
