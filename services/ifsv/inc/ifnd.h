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

/* IFND.H: Master include file used by IFND *.C files */

#ifndef IFND_H
#define IFND_H


#include "ifsv.h"
#include "ifnd_dl_api.h"

#include "ifnd_mib.h"
#include "ifnd_mds.h"
#include "ifnd_saf.h"
#include "ifsvd_cb.h"
#include "ifsvdevt.h"
#include "ifndproc.h"

#if(NCS_IFSV_IPXS == 1)
#include "ifnd_ipxs.h"
#endif

extern uns32 idim_send_ifnd_up_evt (MDS_DEST *to_dest, IFSV_CB *cb);


#endif


