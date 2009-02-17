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

/* IFD.H: Master include file used by IFD *.c Files*/

#ifndef IFD_H
#define IFD_H

#include "ifsv.h"
#include "ifd_dl_api.h"
#include "ifsvifap.h"
#include "ifdproc.h"
#include "ifd_mds.h"
#include "ifd_mib.h"
#include "ifd_saf.h"
#include "ifsv_entp_mapi.h"
#include "mbcsv_papi.h"
#include "ifd_red.h"
#include "ifsvifap.h"


#if(NCS_IFSV_IPXS == 1)
#include "ifd_ipxs.h"
#include "ipxs_cip.h"
#include "ipxs_mapi.h"
#endif

#if(NCS_VIP == 1)
extern uns32 
ifd_vip_evt_process(IFSV_CB *cb, IFSV_EVT *evt);
extern uns32 
ifd_all_vip_rec_del (IFSV_CB *ifsv_cb);
#endif

#if(NCS_IFSV_BOND_INTF == 1)
#include "ifd_ir.h"
#endif



#endif


