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


#ifndef _IFSV_APP_H
#define _IFSV_APP_H
#include "ifa_papi.h"

/*#include "ncs_saf.h"
 This is the structure which stores all the necessary information which 
 * Interface process 
 */
#define IFSV_DT_MAX_TEST_APP       (5)
#define IFSV_DT_TEST_APP_PRIORITY     (4)
#define IFSV_DT_TEST_APP_STACKSIZE    NCS_STACKSIZE_HUGE

typedef struct ifsv_dt_test_app_cb
{
   NCSCONTEXT task_hdl;
   SYSF_MBX   mbx;
   uns32      app_no;
   uns32      shelf_no;
   uns32      slot_no;
   uns32      IfA_hdl;
   NCS_BOOL   inited;   
} IFSV_DT_TEST_APP_CB;

IFSV_DT_TEST_APP_CB gifsv_dt_test_app[IFSV_DT_MAX_TEST_APP];
extern uns32 gifsv_dt_total_test_app_created;
typedef enum sint_ifsv_test_app_evt_type
{
   IFSV_DT_TEST_APP_EVT_ADD_INTF = 1,
   IFSV_DT_TEST_APP_EVT_DEL_INTF,
   IFSV_DT_TEST_APP_EVT_MOD_INTF,
   IFSV_DT_TEST_APP_EVT_INTF_INFO,
   IFSV_DT_TEST_APP_EVT_STAT_INFO,
   IFSV_DT_TEST_APP_BIND_GET_LOCAL_INTF,
   IFSV_DT_TEST_APP_EVT_INFO_ALL
} IFSV_DT_TEST_APP_EVT_TYPE;
typedef struct ifsv_dt_test_app_evt
{
   struct sint_ifsv_test_app_evt *next;
   IFSV_DT_TEST_APP_EVT_TYPE   evt_type; 
   NCS_IFSV_SVC_RSP            rsp;
   /* uns32 shelf;
   uns32 slot;
   uns32 port;
   uns32 type;
   
   NCS_IFSV_INTF_STATS stats;      IFSV_DT_TEST_APP_EVT_STAT_INFO/IFSV_DT_TEST_APP_EVT_INFO_ALL*/
   NCS_IFSV_INTF_INFO  intf_info; /* IFSV_DT_TEST_APP_EVT_ADD_INTF/IFSV_DT_TEST_APP_EVT_MOD_INTF/IFSV_DT_TEST_APP_EVT_INTF_INFO/IFSV_DT_TEST_APP_EVT_INFO_ALL*/
   
} IFSV_DT_TEST_APP_EVT;
EXTERN_C uns32 
ifsv_dt_test_ifa_sub_cb(NCS_IFSV_SVC_RSP *rsp);
EXTERN_C uns32 IfsvDtTestAppCreate(uns32 shelf,uns32 slot);
uns32 IfsvDtTestAppIfaSub(uns32 app_no, uns32 evt_attr, uns32 rec_attr);
uns32 IfsvDtTestAppIfaUnsub(uns32 app_no);
uns32 IfsvDtTestAppGetStats(uns32 app_no, uns32 shelf, uns32 slot, uns32 port, 
                             uns32 port_type);
uns32 IfsvDtTestAppGetIfinfo(uns32 app_no, uns32 shelf, uns32 slot, uns32 port ,uns32 port_type);
uns32 IfsvDtTestAppGetAll(uns32 app_no, uns32 ifindex);
uns32 IfsvDtTestAppDestroy(uns32 app_no);
uns32 IfsvDtTestAppAddIntf(uns32 app_num, char *if_name, uns32 port_num, 
                            uns32 port_type, uns8 *iphy, uns32 oper_state, 
                            uns32 MTU, uns32 speed);
uns32 IfsvDtTestAppModIntfStatus(uns32 app_num, uns32 port_num, 
                                   uns32 port_type, uns32 oper_state);
uns32 IfsvDtTestAppModIntfMTU(uns32 app_num, uns32 port_num, uns32 port_type,
                                uns32 MTU);
uns32 IfsvDtTestAppModIntfSpeed(uns32 app_num, uns32 port_num, 
                                  uns32 port_type, uns32 speed);
uns32 IfsvdtTestAppModIntfPhy(uns32 app_num, uns32 port_num, uns32 port_type,
                                char *temp_phy );
uns32 IfsvDtTestAppModIntfName(uns32 app_num, uns32 port_num, uns32 port_type,
                                 char *temp_name);
uns32 IfsvDtTestAppDelIntf(uns32 app_num, uns32 port_num, uns32 port_type);
uns32 IfsvDtTestAppGetBondLocalIfinfo(uns32 app_no, uns32 shelf, uns32 slot, uns32 port ,uns32 port_type);
uns32 IfsvDtTestAppSwapBondIntf(uns32 app_num, uns32 port_num,
                                   uns32 port_type, uns32 master_ifindex, uns32 slave_ifindex);
#endif /* _IFSV_DEMO_H  */
