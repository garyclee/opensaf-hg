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


#ifndef _IFSV_DEMO_H
#define _IFSV_DEMO_H
#include "ifa_papi.h"
#include "ncssysf_ipc.h"
#include "ifsv_app.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <asm/ioctls.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h> /* close */
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <sys/uio.h>

#define IFSV_DRV_SEND_END     (4)
#define IFSV_DRV_NOTSEND_END  (3)
#define IFSV_DRV_NOTSEND_CONT (2)
#define IFSV_DRV_SEND_CONT    (1)

#define IFSV_DRV_EOF          (3)
#define IFSV_DRV_EOL          (2)

#define IFSV_DRV_CHAR_FOUND      (1)
#define IFSV_DRV_CHAR_NOTFOUND   (0)

#define IFSV_DRV_WORD_FOUND      (0)
#define IFSV_DRV_WORD_NOTFOUND   (1)

#define IFSV_DRV_DEMO_PRIORITY   (4)
#define IFSV_DRV_DEMO_STACK_SIZE NCS_STACKSIZE_HUGE
#define IFSV_DEMO_MAIN_MAX_INPUT (5)

#define m_IFSV_ALL_ATTR_SET(attr) \
     (attr = (NCS_IFSV_IAM_MTU | NCS_IFSV_IAM_IFSPEED | NCS_IFSV_IAM_PHYADDR |      \
              NCS_IFSV_IAM_ADMSTATE | NCS_IFSV_IAM_OPRSTATE | NCS_IFSV_IAM_OPRSTATE \
              | NCS_IFSV_IAM_NAME | NCS_IFSV_IAM_DESCR | NCS_IFSV_IAM_LAST_CHNG ))


typedef struct stats_info
{
   uns32 i_bytes;
   uns32 i_pack;
   uns32 i_error;
   uns32 i_drop;
   uns32 o_bytes;
   uns32 o_pack;
   uns32 o_error;
   uns32 o_drop;
} STATS_INFO;
typedef struct intf_info
{
   uns32 valid; /** 1 valid and 0 not valid **/
   uns32 port;
   uns32 port_type;
   uns32 MTU;
   uns32 admin_status;
   uns32 oper_status;
   unsigned char phy[7];
   unsigned char name[64];
} INTF_INFO;
typedef struct shelf_slot
{
   uns32 shelf;
   uns32 slot;
   uns32 subslot;
} SHELF_SLOT;
uns32 
ifsv_drv_get_specific_intf_info (INTF_INFO *intf_info, uns32 port_num);
uns32
ifsv_demo_reg_port_info (uns32 port_num, uns32 port_type, uns8 *i_phy, 
             uns32 oper_state, uns32 admin_state, uns32 MTU,char *if_name, 
             uns32 if_speed);
NCSCONTEXT
ifsv_drv_poll_intf_info (NCSCONTEXT info);
uns32 
ifsv_app_demo_start(NCSCONTEXT *info);
uns32
ifsv_drv_demo_start(NCSCONTEXT *info);
#endif
