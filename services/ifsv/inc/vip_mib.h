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

#include "ncsgl_defs.h"
#include "ncs_mib_pub.h"
#include "ncs_ubaid.h"
#include "ncs_svd.h"
#include "ncspatricia.h"
#include "ncs_iplib.h"
#include "ncsmiblib.h"
#include "ncs_edu_pub.h"
#include "oac_papi.h"
#include "ncssysf_tsk.h"
#include "ncssysf_lck.h"
#include "ncs_main_papi.h"
#include "ncssysf_def.h"
#include "ncs_hdl_pub.h"
#include "ncssysfpool.h"
#include "ncssysf_mem.h"
#include "ncs_cli.h"
#include "mac_papi.h"
#include "ncs_trap.h"


#include "vip_tbl.h"
#include "vip_mapi.h"

/*
** Return Codes shared across the VIP CLI && VIP MIB ...
 **/

#define IFSV_VIP_NO_RECORDS_IN_VIP_DATABASE  1030 /* All went well -success           */
#define IFSV_VIP_INVALID_INDEX               1031 /* Invalid Index Submitteb By User*/






typedef struct vip_ip_intf_data_tag
{
    uns32 ipaddress;
    uns8 mask_len;
    uns8 interface_name[128];
} VIP_IP_INTF_DATA;
                                                                                                                             

