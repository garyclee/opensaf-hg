#include "ncs_lib.h"
#include "ncs_iplib.h"
#include "ifa_papi.h"
#include "ipxs_papi.h"
#include "saAis.h"
#include "saEvt.h"


SaAisErrorT gl_rc;

void tware_mem_ign(void);
void tware_mem_dump(void);

#define subscription_loop 1
#define gl_eds 1
#define ND_RESTART 3
#define IFA_SNMP 4
typedef struct biface
{
  NCS_IFSV_IFINDEX mifindex;
  NCS_IFSV_IFINDEX sifindex;      
}BIFACE;

BIFACE b_iface;

typedef struct ifsv_mailbox
{
  NCSCONTEXT task_hdl;
  SYSF_MBX   mbx;
}IFSV_MAILBOX;

IFSV_MAILBOX *mbx_cb;
#if (NCS_VIP == 1)
const char *gl_ifsv_error_code[]=
  {
    "NO CODE",
    "NCSCC_RC_SUCCESS",
    "NCSCC_RC_FAILURE",
  };
#endif

typedef enum
  {
    IFSV_API_TEST=1,
    IFSV_FUNC_TEST,     
    VIP_TWO_NODE_TEST,
  }IFSV_TEST_LIST;

int gl_ifIndex,shelf_id,slot_id,gl_subscription_hdl,gl_tCase,gl_listNumber,
  gl_iteration,gl_cbk,gl_error,sid;
int gl_eds_on;
void *eventData;

NCS_IFSV_SVC_REQ req_info;
NCS_IFSV_DRV_SVC_REQ drv_info;
NCS_IFSV_PORT_REG port_reg;
NCS_IPXS_REQ_INFO info;
NCS_IFSV_SVC_RSP testrsp,cmprsp;
NCS_IPXS_RSP_INFO cmpip;
#if (NCS_VIP == 1)
void vip_result(char *saf_msg, int resflag);
void result(char *saf_msg,int ref);
void tet_ncs_ifsv_svc_req_vip_free(int iOption);
void tet_ncs_ifsv_svc_req_vip_install(int i);
void tet_ncs_ifsv_free_spec_ip(int iOption);
void tet_ncs_ifsv_free_spec_ip(int iOption);
void tet_ncs_ifsv_svc_req_node_1(int iOption);
void tet_ncs_ifsv_svc_req_node_2(int iOption);
void dummy(void);
#endif
void tet_ncs_ifsv_svc_req_failover_test(int iOption);
void ifsv_result(char *saf_msg,SaAisErrorT ref);
void tet_run_ifsv_app(void);
void tet_ifsv_startup(void);
void tet_driver_init_req(void);
void tet_driver_destroy_req(void);

/* START NEW */
#if (NCS_VIP == 1)

#define IFSV_VIP_FREE_APPL_SUCCESS 99

uns8 applName[100];
uns32 hdl,uninstallip=0;
int expectedResult, gl_pid=-1;

char *VIP_ERRORS[] = {
  "INVALID",
  "SUCCESS",
  "FAILURE",
  "INVALID",
  "NCSCC_RC_INTF_ADMIN_DOWN",
  "NCSCC_RC_INVALID_INTF",
  "NCSCC_RC_INVALID_IP",
  "NCSCC_RC_IP_ALREADY_EXISTS",
  "NCSCC_RC_INVALID_PARAM",
  "NCSCC_RC_INVALID_VIP_HANDLE",
  "NCSCC_RC_NULL_APPLNAME",
  "NCSCC_RC_VIP_RETRY",
  "NCSCC_RC_VIP_INTERNAL_ERROR",
  "NCSCC_VIP_EXISTS_ON_OTHER_INTERFACE",
  "NCSCC_RC_IP_EXISTS_BUT_NOT_VIP",
  "NCSCC_VIP_HDL_IN_USE_FOR_DIFF_IP",
  "NCSCC_RC_APPLNAME_LEN_EXCEEDS_LIMIT",
  "NCSCC_VIP_EXISTS_FOR_OTHER_HANDLE",
  "NCSCC_RC_APPLICAION_NOT_OWNER_OF_IP",
  "NCSCC_RC_INVALID_FLAG",
};

typedef enum
  {
    IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_DIFF_INTF_DIFF_NODE=1, /*FAILURE*/
    IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE, /*FAILURE*/
    IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE, /*FAIL*/
    IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_DIFF_INTF_DIFF_NODE,/*SUCCESS*/
    IFSV_VIP_UNINSTALL_FROM_NON_OWNER, /*FAILURE*/
    IFSV_VIP_TWO_INSTALL_ONE_UNINSTALL, /*SUCCESS*/
  }DISTRIBUTED_TESTCASE;

char *INSTALL_TESTCASE[] = {
  "IFSV_VIP_INSTALL_NULL_APPL", /*1*/
  "IFSV_VIP_INSTALL_NULL_HDL", /*2*/
  "IFSV_VIP_INSTALL_INVALID_INTF", /*3*/
  "IFSV_VIP_INSTALL_NULL_INTF", /*4*/
  "IFSV_VIP_INSTALL_NULL_IP", /*5*/
  "IFSV_VIP_INSTALL_SUCC", /*6*/
  "IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_DIFF_IP_DIFF_INTF_SAME_NODE", /*SUCCESS*/ /*7*/
  "IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_DIFF_IP_SAME_INTF_SAME_NODE", /*SUCCESS*/ /*8*/
  "IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_DIFF_INTF_SAME_NODE", /*FAILURE*/ /*9*/
  /*"IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_DIFF_INTF_DIFF_NODE",*/ /*FAILURE*/
  /*"IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE",*/ /*FAILURE*/
  "IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_SAME_INTF", /*FAILURE*/ /*10*/
  "IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_SAME_INTF", /*SUCCESS*/ /*11*/
  "IFSV_VIP_INSTALL_SAME_EXISTING_PARAMS", /*FAIL*/ /*12*/
  "IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_DIFF_INTF", /*FAIL*/ /*13*/
  "IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_DIFF_IP_SAME_INTF", /*FAIL*/ /*14*/
  "IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_DIFF_IP_DIFF_INTF", /*FAIL*/ /*15*/
  "IFSV_VIP_INSTALL_DEL_INSTALL", /*SUCCESS*/ /*16*/
  "IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_SAME_INTF", /*SUCCESS*/ /*17*/
  "IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_DIFF_INTF_SAME_NODE", /*SUCCESS*/ /*18*/
  /*IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE,*/ /*FAIL*/
  "IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_SAME_INTF", /*SUCCESS*/ /*19*/
  "IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_DIFF_INTF_SAME_NODE", /*SUCCESS*/ /*20*/
  /*IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_DIFF_INTF_DIFF_NODE,*/
  /*     IFSV_VIP_INSTALL_IP_POOL,
         IFSV_VIP_INSTALL_IFA_CRASH,
         IFSV_VIP_INSTALL_IFND_CRASH,
         IFSV_VIP_INSTALL_IFD_CRASH,
         IFSV_VIP_INSTALL_FAILOVER,
         IFSV_VIP_INSTALL_STDBY,*/
  "IFSV_VIP_INSTALL_ON_INTF_DOWN", /*21*/
  "IFSV_VIP_INSTALL_INTF_DOWN_AND_UP", /*22*/
  /*     "IFSV_VIP_INSTALL_INVALID_IP",
         IFSV_VIP_INSTALL_ERR_RETRY,*/
  "IFSV_VIP_INSTALL_SAME_AS_EXISTING_NORMAL_IP", /*23*/
  "IFSV_VIP_INSTALL_TOO_LONG_APPL", /*24*/
  "IFSV_VIP_INSTALL_INVALID_INFRAFLAG", /*25*/
  "IFSV_VIP_INSTALL_INFRAFLAG_ADDR_SPECIFIC", /*26*/
  /*IFSV_VIP_UNINSTALL_FROM_NON_OWNER,*/
};

struct ifsv_vip_install{
  NCS_IFSV_SVC_REQ_TYPE i_req_type;
  NCS_IFSV_VIP_INSTALL *i_vip_install;
};

NCS_IFSV_VIP_INSTALL i_vip_install;

typedef enum
  {
    IFSV_VIP_INSTALL_NULL_APPL=1,
    IFSV_VIP_INSTALL_NULL_HDL,
    IFSV_VIP_INSTALL_INVALID_INTF,
    IFSV_VIP_INSTALL_NULL_INTF,
    IFSV_VIP_INSTALL_NULL_IP,
    IFSV_VIP_INSTALL_SUCC,
    IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_DIFF_IP_DIFF_INTF_SAME_NODE, /*SUCCESS*/
    IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_DIFF_IP_SAME_INTF_SAME_NODE, /*SUCCESS*/
    IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_DIFF_INTF_SAME_NODE, /*FAILURE*/
    /*IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_DIFF_INTF_DIFF_NODE,*/ /*FAILURE*/
    IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_SAME_INTF, /*FAILURE*/
    IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_SAME_INTF, /*SUCCESS*/
    IFSV_VIP_INSTALL_SAME_EXISTING_PARAMS, /*FAIL*/
    IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_DIFF_INTF, /*FAIL*/
    IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_DIFF_IP_SAME_INTF, /*FAIL*/
    IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_DIFF_IP_DIFF_INTF, /*FAIL*/
    IFSV_VIP_INSTALL_DEL_INSTALL, /*SUCCESS*/
    IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_SAME_INTF, /*SUCCESS*/
    IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_DIFF_INTF_SAME_NODE, /*FAIL*/
    /*IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE,*/ /*FAIL*/
    IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_SAME_INTF, /*SUCCESS*/
    IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_DIFF_INTF_SAME_NODE, /*SUCCESS*/
    /*IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_DIFF_INTF_DIFF_NODE,*/
    /*     IFSV_VIP_INSTALL_IP_POOL,
           IFSV_VIP_INSTALL_IFA_CRASH,
           IFSV_VIP_INSTALL_IFND_CRASH,
           IFSV_VIP_INSTALL_IFD_CRASH,
           IFSV_VIP_INSTALL_FAILOVER,
           IFSV_VIP_INSTALL_STDBY,*/
    IFSV_VIP_INSTALL_ON_INTF_DOWN,
    IFSV_VIP_INSTALL_INTF_DOWN_AND_UP,
    /*     IFSV_VIP_INSTALL_INVALID_IP,
           IFSV_VIP_INSTALL_ERR_RETRY,*/
    IFSV_VIP_INSTALL_SAME_AS_EXISTING_NORMAL_IP,
    IFSV_VIP_INSTALL_TOO_LONG_APPL,
    IFSV_VIP_INSTALL_INVALID_INFRAFLAG,
    IFSV_VIP_INSTALL_INFRAFLAG_ADDR_SPECIFIC, /*26*/
    /*IFSV_VIP_UNINSTALL_FROM_NON_OWNER,*/
  }IFSV_VIP_INSTALL;

struct ifsv_vip_free{
  NCS_IFSV_SVC_REQ_TYPE i_req_type; 
  NCS_IFSV_VIP_FREE *i_vip_free;
};

NCS_IFSV_VIP_FREE i_vip_free;

char *FREE_TESTCASE[] = {
  "IFSV_VIP_FREE_NULL_APPL",
  "IFSV_VIP_FREE_NULL_HDL",
  "IFSV_VIP_FREE_INVALID_APPL",
  "IFSV_VIP_FREE_INVALID_HDL",
  "IFSV_VIP_FREE_SUCC",
  "IFSV-VIP_FREE_TWICE",
};
typedef enum
  {
    IFSV_VIP_FREE_NULL_APPL=1,
    IFSV_VIP_FREE_NULL_HDL,
    IFSV_VIP_FREE_INVALID_APPL,
    IFSV_VIP_FREE_INVALID_HDL,
    IFSV_VIP_FREE_SUCC,
    IFSV_VIP_FREE_TWICE,
  }IFSV_VIP_FREE;
#endif
/* END NEW */

struct ifsv_subscribe{
  NCS_IFSV_SVC_REQ_TYPE i_req_type;
  NCS_IFSV_SUBSCR *i_subr;
};

NCS_IFSV_SUBSCR i_subr;

typedef enum
  {
    IFSV_NULL_REQUEST_TYPE=1,
    IFSV_INVALID_REQUEST_TYPE,
    IFSV_SUBSCRIBE_NULL_CALLBACKS,
    IFSV_SUBSCRIBE_INVALID_SUBSCR_SCOPE,
    IFSV_SUBSCRIBE_EXTERNAL_SUBSCR_SCOPE,
    IFSV_SUBSCRIBE_INTERNAL_SUBSCR_SCOPE,
    IFSV_SUBSCRIBE_NULL_EVENT_SUBSCRIPTION,
    IFSV_SUBSCRIBE_INVALID_EVENT_SUBSCRIPTION,
    IFSV_SUBSCRIBE_ADD_IFACE,
    IFSV_SUBSCRIBE_RMV_IFACE,
    IFSV_SUBSCRIBE_UPD_IFACE,
    IFSV_SUBSCRIBE_NULL_ATTRIBUTES_MAP,
    IFSV_SUBSCRIBE_INVALID_ATTRIBUTES_MAP,
    IFSV_SUBSCRIBE_IAM_MTU,
    IFSV_SUBSCRIBE_IAM_IFSPEED,
    IFSV_SUBSCRIBE_IAM_PHYADDR,
    IFSV_SUBSCRIBE_IAM_ADMSTATE,       
    IFSV_SUBSCRIBE_IAM_OPRSTATE,
    IFSV_SUBSCRIBE_IAM_NAME,
    IFSV_SUBSCRIBE_IAM_DESCR=20,
    IFSV_SUBSCRIBE_IAM_LAST_CHNG,
    IFSV_SUBSCRIBE_IAM_SVDEST,
    IFSV_SUBSCRIBE_SUCCESS,   
    IFSV_SUBSCRIBE_INTERNAL_SUCCESS,
    IFSV_SUBSCRIBE_EXT_ADD_SUCCESS=25,
    IFSV_SUBSCRIBE_EXT_UPD_SUCCESS,
    IFSV_SUBSCRIBE_EXT_RMV_SUCCESS,
    IFSV_SUBSCRIBE_INT_ADD_SUCCESS=28,
    IFSV_SUBSCRIBE_INT_UPD_SUCCESS,
    IFSV_SUBSCRIBE_INT_RMV_SUCCESS,
    IFSV_SUBSCRIBE_ALL,
  }IFSV_SUBSCRIBE;

struct ifsv_unsubscribe{
  NCS_IFSV_SVC_REQ_TYPE i_req_type;
  NCS_IFSV_UNSUBSCR *i_unsubr;
};

NCS_IFSV_UNSUBSCR i_unsubr;

typedef enum
  {
    IFSV_UNSUBSCRIBE_NULL_HANDLE=1,
    IFSV_UNSUBSCRIBE_INVALID_HANDLE,
    IFSV_UNSUBSCRIBE_DUPLICATE_HANDLE,
    IFSV_UNSUBSCRIBE_SUCCESS,
  }IFSV_UNSUBSCRIBE;
  
struct ifsv_ifrec_add{
  NCS_IFSV_SVC_REQ_TYPE i_req_type;
  NCS_IFSV_INTF_REC *i_ifadd;
};

NCS_IFSV_INTF_REC i_ifadd;

typedef enum
  {
    IFSV_IFREC_ADD_NULL_PARAM=1,
    IFSV_IFREC_ADD_SPT,
    IFSV_IFREC_ADD_TYPE_NULL,
    IFSV_IFREC_ADD_TYPE_INVALID,
    IFSV_IFREC_ADD_TYPE_INTF_OTHER,
    IFSV_IFREC_ADD_TYPE_INTF_LOOPBACK,
    IFSV_IFREC_ADD_TYPE_INTF_ETHERNET,
    IFSV_IFREC_ADD_TYPE_INTF_TUNNEL,
    IFSV_IFREC_ADD_SUBSCR_SCOPE_INVALID,
    IFSV_IFREC_ADD_SUBSCR_SCOPE_EXTERNAL,
    IFSV_IFREC_ADD_SUBSCR_SCOPE_INTERNAL,
    IFSV_IFREC_ADD_INFO_TYPE_NULL,
    IFSV_IFREC_ADD_INFO_TYPE_INVALID,
    IFSV_IFREC_ADD_INFO_TYPE_IF_INFO,
    IFSV_IFREC_ADD_INFO_TYPE_IFSTATS_INFO,
    IFSV_IFREC_ADD_IF_INFO_IAM_NULL, 
    IFSV_IFREC_ADD_IF_INFO_IAM_INVALID,
    IFSV_IFREC_ADD_IF_INFO_IAM_MTU,
    IFSV_IFREC_ADD_IF_INFO_IAM_IFSPEED,
    IFSV_IFREC_ADD_IF_INFO_IAM_PHYADDR,
    IFSV_IFREC_ADD_IF_INFO_IAM_ADMSTATE,       
    IFSV_IFREC_ADD_IF_INFO_IAM_OPRSTATE,
    IFSV_IFREC_ADD_IF_INFO_IAM_NAME,
    IFSV_IFREC_ADD_IF_INFO_IAM_DESCR,
    IFSV_IFREC_ADD_IF_INFO_IAM_LAST_CHNG,
    IFSV_IFREC_ADD_IF_INFO_IAM_SVDEST,
    IFSV_IFREC_ADD_IF_INFO_IF_DESCR,
    IFSV_IFREC_ADD_IF_INFO_IF_NAME,
    IFSV_IFREC_ADD_IF_INFO_MTU,
    IFSV_IFREC_ADD_IF_INFO_IF_SPEED,
    IFSV_IFREC_ADD_IF_INFO_PHY_ADDR,
    IFSV_IFREC_ADD_IF_INFO_ADMIN_STATE,
    IFSV_IFREC_ADD_IF_INFO_OPER_STATE,
    IFSV_IFREC_ADD_SUCCESS=34,
    IFSV_IFREC_ADD_UPD_SUCCESS,
    IFSV_IFREC_ADD_INT_SUCCESS,
    IFSV_IFREC_ADD_INT_UPD_SUCCESS,
    IFSV_IFREC_ADD_INTF_OTHER_SUCCESS,
    IFSV_IFREC_ADD_INTF_LOOPBACK_SUCCESS,
    IFSV_IFREC_ADD_INTF_ETHERNET_SUCCESS,
    IFSV_IFREC_ADD_INTF_TUNNEL_SUCCESS,
    IFSV_IFREC_ADD_BIND_NULL_IFINDEX=42,
    IFSV_IFREC_ADD_BIND_INVALID_SHELF_ID,
    IFSV_IFREC_ADD_BIND_INVALID_SLOT_ID,
    IFSV_IFREC_ADD_BIND_SUCCESS=45,
    IFSV_IFREC_ADD_BIND_MIN_PORT,
    IFSV_IFREC_ADD_BIND_ZERO_PORT,
    IFSV_IFREC_ADD_BIND_MAX_PORT,
    IFSV_IFREC_ADD_BIND_OUT_PORT,
    IFSV_IFREC_ADD_BIND_SUBSCR_ALL=50,
    IFSV_IFREC_ADD_BIND_ZERO_MASTER,
    IFSV_IFREC_ADD_BIND_ZERO_SLAVE,
    IFSV_IFREC_ADD_BIND_SAME_MASTER_SLAVE,
    IFSV_IFREC_ADD_BIND_SECOND=54,
    IFSV_IFREC_ADD_BIND_EXT_SCOPE=55,
    IFSV_IFREC_ADD_BIND_INVALID_TYPE=56,
    IFSV_IFREC_ADD_INTERFACE_DISABLE,
    IFSV_IFREC_ADD_INTERFACE_ENABLE,
    IFSV_IFREC_ADD_INTERFACE_MODIFY,
    IFSV_IFREC_ADD_INTERFACE_SEQ_ALLOC,
  }IFSV_IFREC_ADD;

struct ifsv_ifrec_del{
  NCS_IFSV_SVC_REQ_TYPE i_req_type;
  NCS_IFSV_SPT *i_ifdel;
};

NCS_IFSV_SPT i_ifdel;

typedef enum
  {
    IFSV_IFREC_DEL_NULL_PARAM=1,
    IFSV_IFREC_DEL_INVALID_SPT,
    IFSV_IFREC_DEL_INVALID_TYPE,
    IFSV_IFREC_DEL_INVALID_SUBSCR_SCOPE,
    IFSV_IFREC_DEL_SUCCESS,
    IFSV_IFREC_DEL_BIND_SUCCESS,
  }IFSV_IFREC_DEL;

struct ifsv_ifrec_get{
  NCS_IFSV_SVC_REQ_TYPE i_req_type;
  NCS_IFSV_IFREC_GET *i_ifget;
};

NCS_IFSV_IFREC_GET i_ifget;

typedef enum
  {
    IFSV_IFREC_GET_NULL_PARAM=1,
    IFSV_IFREC_GET_KEY_INVALID,
    IFSV_IFREC_GET_KEY_IFINDEX,
    IFSV_IFREC_GET_KEY_SPT,
    IFSV_IFREC_GET_INFO_TYPE_INVALID,
    IFSV_IFREC_GET_INFO_TYPE_IF_INFO,
    IFSV_IFREC_GET_INFO_TYPE_IFSTATS_INFO,
    IFSV_IFREC_GET_RESP_TYPE_INVALID,
    IFSV_IFREC_GET_RESP_TYPE_SYNC,
    IFSV_IFREC_GET_RESP_TYPE_ASYNC=10,
    IFSV_IFREC_GET_IFINDEX_SUCCESS,
    IFSV_IFREC_GET_SPT_SUCCESS,
    IFSV_IFREC_GET_SYNC_IFINDEX_SUCCESS,
    IFSV_IFREC_GET_SYNC_SPT_SUCCESS,
    IFSV_IFREC_GET_IFINDEX_STATS_INFO,
    IFSV_IFREC_GET_SPT_STATS_INFO,
    IFSV_IFREC_GET_SYNC_IFINDEX_STATS_INFO,
    IFSV_IFREC_GET_SYNC_SPT_STATS_INFO,
    IFSV_IFREC_GET_INVALID_SUBSCR_SCOPE, 
    IFSV_IFREC_GET_SUBSCR_SCOPE=20,
    IFSV_IFREC_GET_BIND_LOCAL_INTF,
    IFSV_IFREC_GET_BIND_SPT,
    IFSV_IFREC_GET_BIND_SPT_LOCAL,
  }IFSV_IFREC_GET;

struct ifsv_svc_dest_get{
  NCS_IFSV_SVC_REQ_TYPE i_req_type;
  NCS_IFSV_SVC_DEST_GET *i_svd_get;
};

NCS_IFSV_SVC_DEST_GET i_svd_get;

typedef enum
  {
    IFSV_SVC_DEST_GET_NULL_PARAM=1,
  }IFSV_SVC_DEST_GET;

struct ifsv_driver_init{
  NCS_IFSV_DRV_REQ_TYPE req_type;
};

typedef enum
  {
    IFSV_DRIVER_INIT_NULL_PARAM=1,
    IFSV_DRIVER_INIT_INVALID_TYPE,    
    IFSV_DRIVER_INIT_SUCCESS,
  }IFSV_DRIVER_INIT;

struct ifsv_driver_destroy{
  NCS_IFSV_DRV_REQ_TYPE req_type;
};

typedef enum
  {
    IFSV_DRIVER_DESTROY_NULL_PARAM=1,
    IFSV_DRIVER_DESTROY_INVALID_TYPE,    
    IFSV_DRIVER_DESTROY_SUCCESS,
  }IFSV_DRIVER_DESTROY;

struct ifsv_driver_port_reg{
  NCS_IFSV_DRV_REQ_TYPE req_type;
  NCS_IFSV_PORT_REG *port_reg;
};

NCS_IFSV_PORT_REG port_reg;

typedef enum{
  IFSV_DRIVER_PORT_REG_NULL_PARAM=1,
    IFSV_DRIVER_PORT_REG_IAM_NULL,
    IFSV_DRIVER_PORT_REG_IAM_INVALID,
    IFSV_DRIVER_PORT_REG_IAM_MTU,
    IFSV_DRIVER_PORT_REG_IAM_IFSPEED,
    IFSV_DRIVER_PORT_REG_IAM_PHYADDR,
    IFSV_DRIVER_PORT_REG_IAM_ADMSTATE,
    IFSV_DRIVER_PORT_REG_IAM_OPRSTATE,
    IFSV_DRIVER_PORT_REG_IAM_NAME,
    IFSV_DRIVER_PORT_REG_IAM_DESCR,
    IFSV_DRIVER_PORT_REG_IAM_LAST_CHNG,
    IFSV_DRIVER_PORT_REG_IAM_SVDEST,
    IFSV_DRIVER_PORT_REG_PORT_TYPE_NULL,
    IFSV_DRIVER_PORT_REG_PORT_TYPE_INVALID,
    IFSV_DRIVER_PORT_REG_PORT_TYPE_INTF_OTHER,
    IFSV_DRIVER_PORT_REG_PORT_TYPE_LOOPBACK,
    IFSV_DRIVER_PORT_REG_PORT_TYPE_ETHERNET,
    IFSV_DRIVER_PORT_REG_PORT_TYPE_TUNNEL,
    IFSV_DRIVER_PORT_REG_SUCCESS,
    IFSV_DRIVER_PORT_REG_UPD_SUCCESS,
    IFSV_DRIVER_PORT_REG_DISABLE_INTERFACE,
    IFSV_DRIVER_PORT_REG_ENABLE_INTERFACE,
    IFSV_DRIVER_PORT_REG_MODIFY_INTERFACE,
    IFSV_DRIVER_PORT_REG_INTERFACE_SEQ_ALLOC,
    }IFSV_DRIVER_PORT_REG;

struct ifsv_driver_send {
  NCS_IFSV_DRV_REQ_TYPE req_type;
  NCS_IFSV_HW_INFO *hw_info;   
};

NCS_IFSV_HW_INFO hw_info;

typedef enum
  {
    IFSV_DRIVER_SEND_NULL_PARAM=1,
    IFSV_DRIVER_SEND_MSG_TYPE_INVALID,
    IFSV_DRIVER_SEND_MSG_TYPE_PORT_REG,
  }IFSV_DRIVER_SEND;

struct ipxs_subscriber{
  NCS_IFSV_IPXS_REQ_TYPE i_type;
  NCS_IPXS_SUBCR *i_subcr;
};

NCS_IPXS_SUBCR i_subcr;

typedef enum
  {
    IPXS_SUBSCRIBE_INVALID_REQUEST_TYPE=1,
    IPXS_SUBSCRIBE_NULL_CALLBACKS,
    IPXS_SUBSCRIBE_NULL_SUBSCRIPTION_ATTRIBUTE_BITMAP,
    IPXS_SUBSCRIBE_INVALID_SUBSCRIPTION_ATTRIBUTE_BITMAP,
    IPXS_SUBSCRIBE_ADD_IFACE,
    IPXS_SUBSCRIBE_RMV_IFACE,
    IPXS_SUBSCRIBE_UPD_IFACE,
    IPXS_SUBSCRIBE_SCOPE_NODE,
    IPXS_SUBSCRIBE_SCOPE_CLUSTER,
    IPXS_SUBSCRIBE_NULL_ATTRIBUTES_MAP,
    IPXS_SUBSCRIBE_INVALID_ATTRIBUTES_MAP,
    IPXS_SUBSCRIBE_IAM_MTU,
    IPXS_SUBSCRIBE_IAM_IFSPEED,
    IPXS_SUBSCRIBE_IAM_PHYADDR,
    IPXS_SUBSCRIBE_IAM_ADMSTATE,
    IPXS_SUBSCRIBE_IAM_OPRSTATE,
    IPXS_SUBSCRIBE_IAM_NAME,
    IPXS_SUBSCRIBE_IAM_DESCR,
    IPXS_SUBSCRIBE_IAM_LAST_CHNG,
    IPXS_SUBSCRIBE_IAM_SVDEST,
    IPXS_SUBSCRIBE_NULL_IP_ATTRIBUTES_MAP,
    IPXS_SUBSCRIBE_INVALID_IP_ATTRIBUTES_MAP,
    IPXS_SUBSCRIBE_IPAM_ADDR,
    IPXS_SUBSCRIBE_IPAM_UNNMBD,
    IPXS_SUBSCRIBE_IPAM_RRTRID,
    IPXS_SUBSCRIBE_IPAM_RMTRID,
    IPXS_SUBSCRIBE_IPAM_RMT_AS,
    IPXS_SUBSCRIBE_IPAM_RMTIFID,
    IPXS_SUBSCRIBE_IPAM_UD_1,
    IPXS_SUBSCRIBE_SUCCESS,
    IPXS_SUBSCRIBE_ADD_SUCCESS,
    IPXS_SUBSCRIBE_UPD_SUCCESS,
    IPXS_SUBSCRIBE_RMV_SUCCESS,
  }IPXS_SUBSCRIBE;

struct ipxs_unsubscriber{
  NCS_IFSV_IPXS_REQ_TYPE i_type;
  NCS_IPXS_UNSUBCR *i_unsubcr; 
};

NCS_IPXS_UNSUBCR i_unsubcr;

typedef enum 
  {
    IPXS_UNSUBSCRIBE_NULL_HANDLE=1,
    IPXS_UNSUBSCRIBE_INVALID_HANDLE,
    IPXS_UNSUBSCRIBE_DUPLICATE_HANDLE,
    IPXS_UNSUBSCRIBE_SUCCESS,
  }IPXS_UNSUBSCRIBE;

struct ipxs_ipinfo_gett{
  NCS_IFSV_IPXS_REQ_TYPE i_type;
  NCS_IPXS_IPINFO_GET *i_ipinfo_get;
};

NCS_IPXS_IPINFO_GET i_ipinfo_get;

typedef enum
  {
    IPXS_IPINFO_GET_NULL_PARAM=1,
    IPXS_IPINFO_GET_INVALID_KEY,
    IPXS_IPINFO_GET_KEY_IFINDEX,
    IPXS_IPINFO_GET_INVALID_TYPE,
    IPXS_IPINFO_GET_INTF_OTHER,
    IPXS_IPINFO_GET_INTF_LOOPBACK,
    IPXS_IPINFO_GET_INTF_ETHERNET,
    IPXS_IPINFO_GET_INTF_TUNNEL,
    IPXS_IPINFO_GET_INVALID_SUBSCR_SCOPE,
    IPXS_IPINFO_GET_SUBSCR_EXT,
    IPXS_IPINFO_GET_SUBSCR_INT,
    IPXS_IPINFO_GET_INVALID_ADDR_TYPE,
    IPXS_IPINFO_GET_ADDR_TYPE_NONE,
    IPXS_IPINFO_GET_ADDR_TYPE_IPV4,
    IPXS_IPINFO_GET_ADDR_TYPE_IPV6,
    IPXS_IPINFO_GET_INVALID_RESP_TYPE,
    IPXS_IPINFO_GET_SYNC_RESP_TYPE=17,
    IPXS_IPINFO_GET_ASYNC_RESP_TYPE,
    IPXS_IPINFO_GET_IFINDEX_SUCCESS,
    IPXS_IPINFO_GET_SPT_SUCCESS,
    IPXS_IPINFO_GET_SYNC_IFINDEX_SUCCESS=21,
    IPXS_IPINFO_GET_SYNC_SPT_SUCCESS=22,
    IPXS_IPINFO_GET_IPADDR_SUCCESS,
    IPXS_IPINFO_GET_SYNC_IPADDR_SUCCESS=24,   
    IPXS_IPINFO_GET_NULL_CALLBACKS,
  }IPXS_IPINFO_GET;

struct ipxs_ipinfo_sett{
  NCS_IFSV_IPXS_REQ_TYPE i_type;
  NCS_IPXS_IPINFO_SET *i_ipinfo_set;
};

NCS_IPXS_IPINFO_SET i_ipinfo_set;

typedef enum
  {
    IPXS_IPINFO_SET_INVALID_IFINDEX=1,
    IPXS_IPINFO_SET_NULL_IP_ATTRIBUTE_MAP,
    IPXS_IPINFO_SET_INVALID_IP_ATTRIBUTE_MAP,
    IPXS_IPINFO_SET_IPAM_ADDR,
    IPXS_IPINFO_SET_IPAM_UNNMBD,
    IPXS_IPINFO_SET_IPAM_RRTRID,
    IPXS_IPINFO_SET_IPAM_RMTRID,
    IPXS_IPINFO_SET_IPAM_RMT_AS,
    IPXS_IPINFO_SET_IPAM_RMTIFID,
    IPXS_IPINFO_SET_IPAM_UD_1,
    IPXS_IPINFO_SET_IPV4_NUMB_FLAG,
    IPXS_IPINFO_SET_INVALID_IP_ADDR_TYPE,
    IPXS_IPINFO_SET_IP_ADDR_TYPE_NONE,
    IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4,  
  }IPXS_IPINFO_SET;

struct ipxs_i_is_local{
  NCS_IFSV_IPXS_REQ_TYPE i_type;
  NCS_IPXS_ISLOCAL *i_is_local;
}; 

NCS_IPXS_ISLOCAL i_is_local; 

typedef enum
  {
    IPXS_I_IS_LOCAL_NULL_PARAM=1,
    IPXS_I_IS_LOCAL_INVALID_IPADDRESS_TYPE,  
    IPXS_I_IS_LOCAL_IPADDRESS_IPV4,
    IPXS_I_IS_LOCAL_SUCCESS,
  }IPXS_I_IS_LOCAL;

struct ipxs_get_node_rec{
  NCS_IFSV_IPXS_REQ_TYPE i_type;
  NCS_IPXS_GET_NODE_REC *i_node_rec;
};

NCS_IPXS_GET_NODE_REC i_node_rec;

typedef enum
  {
    IPXS_GET_NODE_REC_NULL_PARAM=1,
    IPXS_GET_NODE_REC_INVALID_NDATTR,
    IPXS_GET_NODE_REC_NDATTR_RTR_ID,
    IPXS_GET_NODE_REC_NDATTR_AS_ID,
    IPXS_GET_NODE_REC_NDATTR_LBIA_ID,
    IPXS_GET_NODE_REC_NDATTR_UD_1,
    IPXS_GET_NODE_REC_NDATTR_UD_2,
    IPXS_GET_NODE_REC_NDATTR_UD_3,
    IPXS_GET_NODE_REC_SUCCESS,
  }IPXS_GET_NODE_REC;

typedef struct usr_ifsv_svc_req
{
  NCS_IFSV_SVC_REQ_TYPE i_req_type;
  NCS_IFSV_SUBSCR i_subr;
  NCS_IFSV_UNSUBSCR i_unsubr;
  NCS_IFSV_IFREC_GET i_ifget;
  NCS_IFSV_INTF_REC i_ifadd;
  NCS_IFSV_SPT i_ifdel;
}USR_IFSV_SVC_REQ;

USR_IFSV_SVC_REQ usr_req_info;

typedef struct usr_ipxs_svc_req
{
  NCS_IFSV_IPXS_REQ_TYPE i_type;
  NCS_IPXS_SUBCR i_subcr;
  NCS_IPXS_UNSUBCR i_unsubcr;
  NCS_IPXS_IPINFO_GET i_ipinfo_get;
  NCS_IPXS_IPINFO_SET i_ipinfo_set;
  NCS_IPXS_ISLOCAL i_is_local;
  NCS_IPXS_GET_NODE_REC i_node_rec;
}USR_IPXS_SVC_REQ;

USR_IPXS_SVC_REQ usr_ipxs_info;

/*Edsv Function Declarations*/
void initialize(void);
void evt_initialize(SaEvtHandleT *evtHandle);
void evt_finalize(SaEvtHandleT evtHandle);
void evt_channelOpen(SaEvtHandleT evtHandle,SaEvtChannelHandleT *channelHandle);
void evt_channelClose(SaEvtChannelHandleT channelHandle);
void evt_channelUnlink(SaEvtHandleT evtHandle);
void evt_eventSubscribe(SaEvtChannelHandleT channelHandle,
                        SaEvtSubscriptionIdT subId);
void evt_dispatch(SaEvtHandleT evtHandle);
void evt_eventAllocate(SaEvtChannelHandleT channelHandle,
                       SaEvtEventHandleT *eventHandle);
void evt_eventFree(SaEvtEventHandleT eventHandle);
void evt_eventAttributesSet(SaEvtEventHandleT *eventHandle);
void evt_eventPublish(SaEvtEventHandleT eventHandle,SaEvtEventIdT *evtId);
void evt_eventDataGet(SaEvtEventHandleT eventHandle);

/*ifsv related*/
void ifsv_subscription(int i);
void ifsv_unsubscription(int i);
void ifsv_ifrec_add(int i);
void ifsv_ifrec_del(int i);

void ipxs_subscription(int i);
void ipxs_unsubscription(int i);

/*ipxs testcase */
/*Dependants*/
void ipxs_ipinfo_gett(int);
void ipxs_ipinfo_sett(int);
void ipxs_i_is_local(int);
void ipxs_get_node_rec(int);
/*Function Declarations*/
void tet_ncs_ipxs_svc_req_subscribe(int);
void tet_ncs_ipxs_svc_req_unsubscribe(int);
void tet_ncs_ipxs_svc_req_ipinfo_get(int);
void tet_ncs_ipxs_svc_req_ipinfo_set(int);
void tet_ncs_ipxs_svc_req_get_node_rec(int);
void tet_ncs_ifsv_svc_req_dest_get(int);
/*Func*/
void tet_interface_subscribe_ipxs(void);
void tet_interface_add_subscribe_ipxs(void);
void tet_interface_upd_subscribe_ipxs(void);
void tet_interface_rmv_subscribe_ipxs(void);
void tet_interface_add(void);
void tet_interface_add_internal(void);
void tet_interface_update(void);
void tet_interface_add_ipxs(void);
void tet_interface_addip_ipxs(void);
void tet_interface_get_ifindex_ipxs(void);
void tet_interface_get_sync_ifindex_ipxs(void);
void tet_interface_get_spt_ipxs(void);
void tet_interface_get_sync_spt_ipxs(void);
void tet_interface_get_ipaddr_ipxs(void);
void tet_interface_get_sync_ipaddr_ipxs(void);
/*ifsv Driver*/
/*Dependant*/
void ifsv_driver_init(int);
void ifsv_driver_destroy(int);
void ifsv_driver_port_reg(int);
/*Func*/
void tet_ifsv_driver_add_interface(void);
void tet_ifsv_driver_add_multiple_interface(void);
void tet_ifsv_driver_disable_interface(void);
void tet_ifsv_driver_enable_interface(void);
void tet_ifsv_driver_modify_interface(void);
void tet_ifsv_driver_seq_alloc(void);
void tet_ifsv_driver_timer_aging(void);
/*api*/
void ifsv_driver_send(int);

void tet_ncs_ifsv_drv_svc_init_req(int);
void tet_ncs_ifsv_drv_svc_destroy_req(int);
void tet_ncs_ifsv_drv_svc_port_reg(int);
void tet_ncs_ifsv_drv_svc_send_req(int);

/*wrapper related*/
void ifsv_cb(NCS_IFSV_SVC_RSP *rsp);
void ifsv_driver_cb(NCS_IFSV_HW_DRV_REQ *drv_req);
void ipxs_cb(NCS_IPXS_RSP_INFO *rsp);


/*ND Restart*/
void kill_ifnd(void);

void ifsv_subscribe_fill(int i);
void ifsv_unsubscribe_fill(int i);
void ifsv_ifrec_add_fill(int i);
void ifsv_ifrec_del_fill(int i);
void ifsv_ifrec_get_fill(int i);
void ifsv_ifrec_get(int i);
void ifsv_svc_dest_get_fill(int i);
void ifsv_svc_dest_get(int i);
void tet_ncs_ifsv_svc_req_subscribe(int iOption);
void tet_ncs_ifsv_svc_req_unsubscribe(int iOption);
void tet_ncs_ifsv_svc_req_ifrec_add(int iOption);
void tet_ncs_ifsv_svc_req_ifrec_del(int iOption);
void tet_ncs_ifsv_svc_req_ifrec_get(int iOption);
void kill_ifnd(void);
void tet_interface_subscribe(void);
void tet_interface_add_subscribe(void);
void tet_interface_upd_subscribe(void);
void tet_interface_rmv_subscribe(void);
void tet_interface_subscribe_all(void);
void tet_interface_int_subscribe(void);
void tet_interface_int_add_subscribe(void);
void tet_interface_int_upd_subscribe(void);
void tet_interface_int_rmv_subscribe(void);
void tet_interface_int_subscribe_all(void);
void tet_interface_add_after_unsubscribe(void);
void tet_interface_disable(void);
void tet_interface_enable(void);
void tet_interface_modify(void);
void tet_interface_seq_alloc(void);
void tet_interface_timer_aging(void);
void tet_interface_get_ifindex(void);
void tet_interface_get_ifindex_internal(void);
void tet_interface_get_spt(void);
void tet_interface_get_spt_internal(void);
void tet_interface_get_ifindex_internal_sync(void);
void tet_interface_get_ifindex_sync(void);
void tet_interface_get_spt_sync(void);
void tet_interface_get_spt_internal_sync(void);
void tet_interface_get_ifIndex(void);
void tet_interface_get_SPT(void);
void tet_interface_add(void);
void tet_interface_add_internal(void);
void tet_interface_update(void);
void tet_interface_add_subscription(void);
void tet_interface_add_subscription_loop(void);
void tet_interface_addip_ipxs_loop(void);
void gl_ifsv_defs(void);
void tet_run_ifsv_app(void);
void tet_ifsv_driver_add_interface(void);
void tet_ifsv_driver_add_multiple_interface(void);
void tet_ifsv_driver_disable_interface(void);
void tet_ifsv_driver_enable_interface(void);
void tet_ifsv_driver_modify_interface(void);
void tet_ifsv_driver_seq_alloc(void);
void tet_ifsv_driver_timer_aging(void);
void tet_interface_subscribe_ipxs(void);
void tet_interface_add_subscribe_ipxs(void);
void tet_interface_upd_subscribe_ipxs(void);
void tet_interface_rmv_subscribe_ipxs(void);
void tet_interface_add_ipxs(void);
void tet_interface_addip_ipxs(void);
void tet_interface_get_ifindex_ipxs(void);
void tet_interface_get_sync_ifindex_ipxs(void);
void tet_interface_get_spt_ipxs(void);
void tet_interface_get_sync_spt_ipxs(void);
void tet_interface_get_ipaddr_ipxs(void);
void tet_interface_get_sync_ipaddr_ipxs(void);
void initialize(void);
void ifsv_driver_port_reg_fill(int i);
void ifsv_driver_send_fill(int i);
void ipxs_subscribe_fill(int i);
void ipxs_unsubscribe_fill(int i);
void ipxs_ipinfo_get_fill(int i);
void ipxs_ipinfo_set_fill(int i);
void ipxs_i_is_local_fill(int i);
void ipxs_get_node_rec_fill(int i);
void ifsv_subscribe_fill(int i);
void ifsv_unsubscribe_fill(int i);

void ifsv_ifrec_add_fill(int i);
void ifsv_ifrec_del_fill(int i);
void ifsv_ifrec_get_fill(int i);
void ifsv_svc_dest_get_fill(int i);

void ifsv_svc_dest_get(int i);
void tet_ncs_ifsv_svc_req_subscribe(int iOption);
void tet_ncs_ifsv_svc_req_unsubscribe(int iOption);
void tet_ncs_ifsv_svc_req_ifrec_add(int iOption);
void tet_ncs_ifsv_svc_req_ifrec_del(int iOption);
void tet_ncs_ifsv_svc_req_ifrec_get(int iOption);
void kill_ifnd(void);
void tet_interface_subscribe(void);
void tet_interface_add_subscribe(void);
void tet_interface_upd_subscribe(void);
void tet_interface_rmv_subscribe(void);
void tet_interface_subscribe_all(void);
void tet_interface_int_subscribe(void);
void tet_interface_int_add_subscribe(void);
void tet_interface_int_upd_subscribe(void);
void tet_interface_int_rmv_subscribe(void);
void tet_interface_int_subscribe_all(void);
void tet_interface_add_after_unsubscribe(void);
void tet_interface_disable(void);
void tet_interface_enable(void);
void tet_interface_modify(void);
void tet_interface_seq_alloc(void);
void tet_interface_timer_aging(void);
void tet_interface_get_ifindex(void);
void tet_interface_get_ifindex_internal(void);
void tet_interface_get_spt(void);
void tet_interface_get_spt_internal(void);
void tet_interface_get_ifindex_sync(void);
void tet_interface_get_ifindex_internal_sync(void);
void tet_interface_get_spt_sync(void);
void tet_interface_get_spt_internal_sync(void);
void tet_interface_get_ifIndex(void);
void tet_interface_get_SPT(void);
void tet_interface_add(void);
void tet_interface_add_internal(void);
void tet_interface_update(void);
void tet_interface_add_subscription(void);
void tet_interface_add_subscription_loop(void);
void tet_interface_addip_ipxs_loop(void);
void gl_ifsv_defs(void);
void tet_run_ifsv_app(void);
void tet_ifsv_driver_add_interface(void);
void tet_ifsv_driver_add_multiple_interface(void);
void tet_ifsv_driver_disable_interface(void);
void tet_ifsv_driver_enable_interface(void);
void tet_ifsv_driver_modify_interface(void);
void tet_ifsv_driver_seq_alloc(void);
void tet_ifsv_driver_timer_aging(void);
void tet_interface_subscribe_ipxs(void);
void tet_interface_add_subscribe_ipxs(void);
void tet_interface_upd_subscribe_ipxs(void);
void tet_interface_rmv_subscribe_ipxs(void);
void tet_interface_add_ipxs(void);
void tet_interface_addip_ipxs(void);
void tet_interface_get_ifindex_ipxs(void);
void tet_interface_get_sync_ifindex_ipxs(void);
void tet_interface_get_spt_ipxs(void);
void tet_interface_get_sync_spt_ipxs(void);
void tet_interface_get_ipaddr_ipxs(void);
void tet_interface_get_sync_ipaddr_ipxs(void);
void vip_wait(void);
void networkaddr_to_ascii(uint32_t net_ip);
int ip_on_intf(char *intf);



