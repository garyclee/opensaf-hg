#ifndef _IFSV_DEMO_H
#define _IFSV_DEMO_H

#include "gl_defs.h"
#include "t_suite.h"
#include "ncs_mds_pub.h"
#include "ifsv_papi.h"
#include "ifa_papi.h"

/* This is the structure which stores all the necessary information which 
 * Interface process 
 */
#define IFSV_DT_MAX_TEST_APP       (20) /* 0-9 for applications for testing 
                                           external interfaces, 10-19 for 
                                           applications for testing internal 
                                           interfaces. This is also helpful in 
                                           convention that if the first digit 
                                           is 1, then this index is for 
                                           application testing internal 
                                           interfaces, otherwise it is for 
                                           application testing external 
                                           interfaces. Hope that 10 
                                           applications are good enough to 
                                           test either internal/external 
                                           interfaces.*/
#define IFSV_DT_TEST_APP_PRIORITY     (4)
#define IFSV_DT_TEST_APP_STACKSIZE    (8000)
typedef struct ifsv_dt_test_app_cb
{
  NCSCONTEXT task_hdl;
  SYSF_MBX   mbx;
  uns32      app_no;
  uns32      shelf_no;
  uns32      slot_no;
  uns32      IfA_hdl;
#if(NCS_IFSV_IPXS == 1)
  uns32      ipxs_hdl;
#endif
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
uns32 IfsvDtTestAppIfaSub(uns32 app_no, uns32 evt_attr, uns32 rec_attr, 
                          NCS_IFSV_SUBSCR_SCOPE scope);
uns32 IfsvDtTestAppIfaUnsub(uns32 app_no);
uns32 IfsvDtTestAppGetStats(uns32 app_no, uns32 shelf, uns32 slot, uns32 port, 
                            uns32 port_type, NCS_IFSV_SUBSCR_SCOPE scope);
uns32 IfsvDtTestAppGetIfinfo(uns32 app_no, uns32 shelf, uns32 slot, uns32 port,
                             uns32 port_typei, NCS_IFSV_SUBSCR_SCOPE scope);
uns32 IfsvDtTestAppGetAll(uns32 app_no, uns32 ifindex);
uns32 IfsvDtTestAppIfGetIntf(uns32 app_no, uns32 ifindex);
uns32 IfsvDtTestAppDestroy(uns32 app_no);
uns32 IfsvDtTestAppAddIntf(uns32 app_num, char *if_name, uns32 port_num, 
                           uns32 port_type, uns8 *iphy, uns32 oper_state, 
                           uns32 MTU,uns32 speed, NCS_IFSV_SUBSCR_SCOPE scope);
uns32 IfsvDtTestAppModIntfStatus(uns32 app_num, uns32 port_num, 
                                 uns32 port_type, uns32 oper_state,
                                 NCS_IFSV_SUBSCR_SCOPE scope);
uns32 IfsvDtTestAppModIntfMTU(uns32 app_num, uns32 port_num, uns32 port_type,
                              uns32 MTU, NCS_IFSV_SUBSCR_SCOPE scope);
uns32 IfsvDtTestAppModIntfSpeed(uns32 app_num, uns32 port_num, 
                                uns32 port_type, uns32 speed,
                                NCS_IFSV_SUBSCR_SCOPE scope);
uns32 IfsvdtTestAppModIntfPhy(uns32 app_num, uns32 port_num, uns32 port_type,
                              char *temp_phy, NCS_IFSV_SUBSCR_SCOPE scope);
uns32 IfsvDtTestAppModIntfName(uns32 app_num, uns32 port_num, uns32 port_type,
                               char *temp_name, NCS_IFSV_SUBSCR_SCOPE scope);
uns32 IfsvDtTestAppDelIntf(uns32 app_num, uns32 port_num, uns32 port_typei,
                           NCS_IFSV_SUBSCR_SCOPE scope);

/* IPX related functions */
uns32 tet_IpxsTestAppIfaSub(uns32 app_no, uns32 evt_attr, uns32 rec_attr,
                            uns32 ip_attr);
uns32 tet_IpxsTestAppIfaUnsub(uns32 app_no);
uns32 tet_IpxsTestAppGetInfoFromIfindex(uns32 app_no, uns32 ifindex );
uns32 tet_IpxsTestAppGetInfoFromSpt(uns32 app_no, uns32 shelf, uns32 slot, 
                                    uns32 port, uns32 type);
uns32 tet_IpxsTestAppGetInfoFromIpAddr(uns32 app_no, char *ip_str);
uns32 tet_NcsIfsvIpxsIsLocal(char *ip_str, uns8 mask_len, NCS_BOOL obs_flag);
uns32 tet_IfsvTestAppGetIpinfo(uns32 app_no, uns32 ifindex);
uns32 tet_IpxsTestAppSetorUpdIPaddr(uns32 app_no, uns32 ifindex, char *ip_str);
uns32 tet_IpxsTestAppSetUnnmbrd(uns32 app_no, uns32 ifindex,NCS_BOOL o_answer);
uns32 tet_IpxsTestAppUnSetUnnmbrd(uns32 app_no, uns32 ifindex,
                                  NCS_BOOL o_answer);
uns32 tet_IpxsTestAppDelIPaddr(uns32 app_no, uns32 ifindex, char *ip_str);
uns32 tet_IpxsTestAppSetRRTRID(uns32 app_no, uns32 ifindex, char *ip_str);
uns32 tet_IpxsTestAppSetRMTRID(uns32 app_no, uns32 ifindex, char *ip_str);
uns32 tet_IpxsTestAppSetRMTAS(uns32 app_no, uns32 ifindex, uns16 rmt_as_id);
uns32 tet_IpxsTestAppSetRMTIFID(uns32 app_no, uns32 ifindex, uns32 rmtifindex);

/* structure definition for array list execution  */

struct ifsv_testlist {

  void (*testfunc)();
  int arg;
};

#endif /* _IFSV_DEMO_H  */
