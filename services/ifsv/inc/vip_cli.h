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

#include "ifa_papi.h"
#include "vip.h"


typedef NCS_IFSV_VIP_INT_HDL NCS_CLI_MODE_DATA;

typedef struct vip_data_display_tag
{
    uns8 *p_service_name;
    uns32 handle;
    uns32 ip_type;
    uns32 ipaddress;
    uns32 mask_len;
    uns8 *p_interface_name;
    uns8 installed_intf[128];
    uns32 configured;
    uns32 ip_range;
    uns32 opr_status;
}VIP_DATA_DISPLAY;

