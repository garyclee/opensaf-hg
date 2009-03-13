/******************************************************************************
#
# modification:
# 2008-11   Jony <jiangnan_hw@huawei.com>  Huawei Technologies CO., LTD.
#  1.  In tet_interface_add_subscription_loop() function, use "select" function 
#      instead of "while(1)" to wait for EVT.
#
*******************************************************************************/


#include "tet_api.h"
#include "tet_startup.h"
#include "tet_ifa.h"
#include "tet_eda.h"
void tet_ifsv_startup(void);

extern int gl_sync_pointnum;
extern int fill_syncparameters(int);

struct ifsv_subscribe subscribe_api[]={
  [IFSV_NULL_REQUEST_TYPE]                 = {0,&i_subr},
  [IFSV_INVALID_REQUEST_TYPE]              = {24,&i_subr},
  [IFSV_SUBSCRIBE_NULL_CALLBACKS]          = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_INVALID_SUBSCR_SCOPE]    = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_EXTERNAL_SUBSCR_SCOPE]   = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_INTERNAL_SUBSCR_SCOPE]   = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_NULL_EVENT_SUBSCRIPTION] = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_INVALID_EVENT_SUBSCRIPTION] = {NCS_IFSV_SVC_REQ_SUBSCR,
                                                 &i_subr},
  [IFSV_SUBSCRIBE_ADD_IFACE]               = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_RMV_IFACE]               = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_UPD_IFACE]               = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_NULL_ATTRIBUTES_MAP]     = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_INVALID_ATTRIBUTES_MAP]  = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_IAM_MTU]                 = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_IAM_IFSPEED]             = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_IAM_PHYADDR]             = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_IAM_ADMSTATE]            = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_IAM_OPRSTATE]            = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_IAM_NAME]                = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_IAM_DESCR]               = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_IAM_LAST_CHNG]           = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_IAM_SVDEST]              = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_SUCCESS]                 = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_INTERNAL_SUCCESS]        = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_EXT_ADD_SUCCESS]         = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_EXT_UPD_SUCCESS]         = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_EXT_RMV_SUCCESS]         = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_INT_ADD_SUCCESS]         = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_INT_UPD_SUCCESS]         = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_INT_RMV_SUCCESS]         = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
  [IFSV_SUBSCRIBE_ALL]                     = {NCS_IFSV_SVC_REQ_SUBSCR,&i_subr},
};

struct ifsv_unsubscribe unsubscribe_api[]={
  [IFSV_UNSUBSCRIBE_NULL_HANDLE]      = {NCS_IFSV_SVC_REQ_UNSUBSCR,&i_unsubr},
  [IFSV_UNSUBSCRIBE_INVALID_HANDLE]   = {NCS_IFSV_SVC_REQ_UNSUBSCR,&i_unsubr},
  [IFSV_UNSUBSCRIBE_DUPLICATE_HANDLE] = {NCS_IFSV_SVC_REQ_UNSUBSCR,&i_unsubr},
  [IFSV_UNSUBSCRIBE_SUCCESS]          = {NCS_IFSV_SVC_REQ_UNSUBSCR,&i_unsubr},
};

struct ifsv_ifrec_add ifrec_add_api[]={
  [IFSV_IFREC_ADD_NULL_PARAM]          = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_SPT]                 = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_TYPE_NULL]           = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_TYPE_INVALID]        = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_TYPE_INTF_OTHER]     = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_TYPE_INTF_LOOPBACK]  = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_TYPE_INTF_ETHERNET]  = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_TYPE_INTF_TUNNEL]    = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_SUBSCR_SCOPE_INVALID]= {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_SUBSCR_SCOPE_EXTERNAL]={NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_SUBSCR_SCOPE_INTERNAL]={NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INFO_TYPE_NULL]      = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INFO_TYPE_INVALID]   = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INFO_TYPE_IF_INFO]   = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INFO_TYPE_IFSTATS_INFO] = {NCS_IFSV_SVC_REQ_IFREC_ADD,
                                             &i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_NULL]    = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_INVALID] = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_MTU]     = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_IFSPEED] = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_PHYADDR] = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_ADMSTATE]= {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_OPRSTATE]= {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_NAME]    = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_DESCR]   = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_LAST_CHNG]={NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IAM_SVDEST]  = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IF_DESCR]    = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IF_NAME]     = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_MTU]         = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_IF_SPEED]    = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_ADMIN_STATE] = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_IF_INFO_OPER_STATE]  ={NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd}, 
  [IFSV_IFREC_ADD_SUCCESS]             ={NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd}, 
  [IFSV_IFREC_ADD_UPD_SUCCESS]         = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INT_SUCCESS]         = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INT_UPD_SUCCESS]     = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INTF_OTHER_SUCCESS]  = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INTF_LOOPBACK_SUCCESS]={NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INTF_ETHERNET_SUCCESS]={NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INTF_TUNNEL_SUCCESS] = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_NULL_IFINDEX]   = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_INVALID_SHELF_ID]={NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_INVALID_SLOT_ID]= {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_SUCCESS]        = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INTERFACE_DISABLE]   = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INTERFACE_ENABLE]    = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INTERFACE_MODIFY]    = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_INTERFACE_SEQ_ALLOC] = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_MIN_PORT]       = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_ZERO_PORT]      = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_MAX_PORT]       = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_OUT_PORT]       = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_SUBSCR_ALL]     = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_ZERO_MASTER]    = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_ZERO_SLAVE]     = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_SAME_MASTER_SLAVE]={NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_SECOND]         = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_EXT_SCOPE]      = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
  [IFSV_IFREC_ADD_BIND_INVALID_TYPE]   = {NCS_IFSV_SVC_REQ_IFREC_ADD,&i_ifadd},
};

struct ifsv_ifrec_del ifrec_del_api[]={
  [IFSV_IFREC_DEL_NULL_PARAM]          = {NCS_IFSV_SVC_REQ_IFREC_DEL,&i_ifdel},
  [IFSV_IFREC_DEL_INVALID_SPT]         = {NCS_IFSV_SVC_REQ_IFREC_DEL,&i_ifdel},
  [IFSV_IFREC_DEL_INVALID_TYPE]        = {NCS_IFSV_SVC_REQ_IFREC_DEL,&i_ifdel},
  [IFSV_IFREC_DEL_INVALID_SUBSCR_SCOPE]= {NCS_IFSV_SVC_REQ_IFREC_DEL,&i_ifdel},
  [IFSV_IFREC_DEL_SUCCESS]             = {NCS_IFSV_SVC_REQ_IFREC_DEL,&i_ifdel},
  [IFSV_IFREC_DEL_BIND_SUCCESS]        = {NCS_IFSV_SVC_REQ_IFREC_DEL,&i_ifdel},
};

struct ifsv_ifrec_get ifrec_get_api[]={
  [IFSV_IFREC_GET_NULL_PARAM]          = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_KEY_INVALID]         = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_KEY_IFINDEX]         = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_KEY_SPT]             = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_INFO_TYPE_INVALID]   = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_INFO_TYPE_IF_INFO]   = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_INFO_TYPE_IFSTATS_INFO]={NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_RESP_TYPE_INVALID]   = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_RESP_TYPE_SYNC]      = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_RESP_TYPE_ASYNC]     = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_IFINDEX_SUCCESS]     = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_SPT_SUCCESS]         = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_SYNC_IFINDEX_SUCCESS]= {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_SYNC_SPT_SUCCESS]    = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_IFINDEX_STATS_INFO]  = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_SPT_STATS_INFO]      = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_SYNC_IFINDEX_STATS_INFO] = {NCS_IFSV_SVC_REQ_IFREC_GET,
                                              &i_ifget},
  [IFSV_IFREC_GET_SYNC_SPT_STATS_INFO] = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_INVALID_SUBSCR_SCOPE]= {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_SUBSCR_SCOPE]        = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_BIND_LOCAL_INTF]     = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_BIND_SPT]            = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
  [IFSV_IFREC_GET_BIND_SPT_LOCAL]      = {NCS_IFSV_SVC_REQ_IFREC_GET,&i_ifget},
};

struct ifsv_svc_dest_get svc_dest_get_api[]={
  [IFSV_SVC_DEST_GET_NULL_PARAM]   = {NCS_IFSV_REQ_SVC_DEST_GET,&i_svd_get},
};

#if 0
struct ifsv_bond_interface  bond_interface_api[]={
};
#endif
/*********************************************************************/
/*********************** IFSV WRAPPERS *******************************/
/*********************************************************************/

void ifsv_subscribe_fill(int i)
{
  if(i>11)
    {
      i_subr.i_ifsv_cb=(NCS_IFSV_SUBSCR_CB)ifsv_cb;
      if(i<24)
        {
          i_subr.subscr_scope=NCS_IFSV_SUBSCR_EXT;
        }
      i_subr.i_sevts=NCS_IFSV_ADD_IFACE|NCS_IFSV_RMV_IFACE|NCS_IFSV_UPD_IFACE; 
    }
  switch(i)
    {
    case 1:
      break;

    case 2:
      break;

    case 3:
  
      i_subr.i_ifsv_cb=(NCS_IFSV_SUBSCR_CB)"";
      break;

    case 4:

      i_subr.i_ifsv_cb=(NCS_IFSV_SUBSCR_CB)ifsv_cb;       
      i_subr.subscr_scope=24;
      break;

    case 5:
 
      i_subr.i_ifsv_cb=(NCS_IFSV_SUBSCR_CB)ifsv_cb;
      i_subr.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      i_subr.i_sevts=NCS_IFSV_ADD_IFACE; 
      i_subr.i_smap=NCS_IFSV_IAM_MTU;
      break;
     
    case 6:

      i_subr.i_ifsv_cb=(NCS_IFSV_SUBSCR_CB)ifsv_cb;
      i_subr.subscr_scope=NCS_IFSV_SUBSCR_INT;
      i_subr.i_sevts=NCS_IFSV_ADD_IFACE; 
      i_subr.i_smap=NCS_IFSV_IAM_MTU;
      break;   

    case 7:

      i_subr.i_ifsv_cb=(NCS_IFSV_SUBSCR_CB)ifsv_cb;
      i_subr.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      i_subr.i_sevts=0; 
      break;

    case 8:

      i_subr.i_ifsv_cb=(NCS_IFSV_SUBSCR_CB)ifsv_cb;
      i_subr.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      i_subr.i_sevts=24;
      break;
  
    case 9:

      i_subr.i_ifsv_cb=(NCS_IFSV_SUBSCR_CB)ifsv_cb;
      i_subr.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      i_subr.i_sevts=NCS_IFSV_ADD_IFACE; 
      i_subr.i_smap=NCS_IFSV_IAM_MTU;
      break;
 
    case 10:

      i_subr.i_ifsv_cb=(NCS_IFSV_SUBSCR_CB)ifsv_cb;
      i_subr.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      i_subr.i_sevts=NCS_IFSV_RMV_IFACE; 
      i_subr.i_smap=NCS_IFSV_IAM_MTU;
      break;

    case 11:

      i_subr.i_ifsv_cb=(NCS_IFSV_SUBSCR_CB)ifsv_cb;
      i_subr.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      i_subr.i_sevts=NCS_IFSV_UPD_IFACE; 
      i_subr.i_smap=NCS_IFSV_IAM_MTU;
      break;

    case 12:

      i_subr.i_smap=0;
      break;
         
    case 13:

      i_subr.i_smap=1024;
      break;

    case 14:

      i_subr.i_smap=NCS_IFSV_IAM_MTU;
      break;

    case 15:

      i_subr.i_smap=NCS_IFSV_IAM_IFSPEED;
      break;

    case 16:

      i_subr.i_smap=NCS_IFSV_IAM_PHYADDR;
      break;

    case 17:

      i_subr.i_smap=NCS_IFSV_IAM_ADMSTATE;
      break;

    case 18:

      i_subr.i_smap=NCS_IFSV_IAM_OPRSTATE;
      break;

    case 19:

      i_subr.i_smap=NCS_IFSV_IAM_NAME;
      break;

    case 20:

      i_subr.i_smap=NCS_IFSV_IAM_DESCR;
      break;

    case 21:

      i_subr.i_smap=NCS_IFSV_IAM_LAST_CHNG;
      break;

    case 22:

      i_subr.i_smap=NCS_IFSV_IAM_SVDEST;
      break;

    case 23:

      i_subr.i_smap=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|NCS_IFSV_IAM_PHYADDR|
        NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|
        NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST;
      break;

    case 24:

      i_subr.subscr_scope=NCS_IFSV_SUBSCR_INT;
      i_subr.i_smap=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|NCS_IFSV_IAM_PHYADDR|
        NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|
        NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST;
      break;

    case 25:

      i_subr.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      i_subr.i_smap=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|NCS_IFSV_IAM_PHYADDR|
        NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|
        NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST;
      i_subr.i_sevts=NCS_IFSV_ADD_IFACE;
      break;

    case 26:

      i_subr.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      i_subr.i_smap=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|NCS_IFSV_IAM_PHYADDR|
        NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|
        NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST;
      i_subr.i_sevts=NCS_IFSV_UPD_IFACE;
      break;

    case 27:

      i_subr.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      i_subr.i_smap=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|NCS_IFSV_IAM_PHYADDR|
        NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|
        NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST;
      i_subr.i_sevts=NCS_IFSV_RMV_IFACE;
      break;

    case 28:

      i_subr.subscr_scope=NCS_IFSV_SUBSCR_INT;
      i_subr.i_smap=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|NCS_IFSV_IAM_PHYADDR|
        NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|
        NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST;
      i_subr.i_sevts=NCS_IFSV_ADD_IFACE;
      break;

    case 29:

      i_subr.subscr_scope=NCS_IFSV_SUBSCR_INT;
      i_subr.i_smap=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|NCS_IFSV_IAM_PHYADDR|
        NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|
        NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST;
      i_subr.i_sevts=NCS_IFSV_UPD_IFACE;
      break;

    case 30:

      i_subr.subscr_scope=NCS_IFSV_SUBSCR_INT;
      i_subr.i_smap=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|NCS_IFSV_IAM_PHYADDR|
        NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|
        NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST;
      i_subr.i_sevts=NCS_IFSV_RMV_IFACE;
      break;

    case 31:

      i_subr.subscr_scope=NCS_IFSV_SUBSCR_ALL;
      i_subr.i_smap=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|NCS_IFSV_IAM_PHYADDR|
        NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|
        NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST|
        NCS_IFSV_IAM_CHNG_MASTER;
      i_subr.i_sevts=NCS_IFSV_RMV_IFACE|NCS_IFSV_ADD_IFACE|NCS_IFSV_UPD_IFACE;

    default:
      break;   
    }
}

void ifsv_subscription(int i)
{
  ifsv_subscribe_fill(i);

  req_info.i_req_type=subscribe_api[i].i_req_type;                   
  req_info.info.i_subr.i_ifsv_cb=*subscribe_api[i].i_subr->i_ifsv_cb;
  req_info.info.i_subr.subscr_scope=subscribe_api[i].i_subr->subscr_scope;   
  req_info.info.i_subr.i_sevts=subscribe_api[i].i_subr->i_sevts;
  req_info.info.i_subr.i_smap=subscribe_api[i].i_subr->i_smap;
  req_info.info.i_subr.i_usrhdl=(uns64)1;
   
  gl_rc=ncs_ifsv_svc_req(&req_info);
  if(gl_rc==NCSCC_RC_SUCCESS)
    {
      printf("\n\nSubscription Handle : %u",req_info.info.i_subr.o_subr_hdl); 
      gl_subscription_hdl=req_info.info.i_subr.o_subr_hdl;
    }
}

void ifsv_unsubscribe_fill(int i)
{
  switch(i)
    {
    case 1:
        
      i_unsubr.i_subr_hdl=0; 
      break;
 
    case 2:
        
      i_unsubr.i_subr_hdl=24;
      break;

    case 3:
 
      i_unsubr.i_subr_hdl=req_info.info.i_subr.o_subr_hdl;
      break;

    case 4:
 
      i_unsubr.i_subr_hdl=req_info.info.i_subr.o_subr_hdl;
      break;

    default:
        
      break;
    }               
}

void ifsv_unsubscription(int i)
{
  ifsv_unsubscribe_fill(i);

  req_info.i_req_type=unsubscribe_api[i].i_req_type;
  req_info.info.i_unsubr.i_subr_hdl=unsubscribe_api[i].i_unsubr->i_subr_hdl;   

  printf("\n\nSubscription Handle: %u",req_info.info.i_unsubr.i_subr_hdl);
  gl_rc=ncs_ifsv_svc_req(&req_info);
}

void ifsv_ifrec_add_fill(int i)
{
  char if_descr[15]="if_descr",if_name[10]="if_name";
   
  if(i!=1)
    {
      if(i<42||i>56)
        {
          i_ifadd.spt_info.shelf=shelf_id;
          i_ifadd.spt_info.slot=slot_id;
        }
      else
        {
          i_ifadd.spt_info.shelf=IFSV_BINDING_SHELF_ID;
          i_ifadd.spt_info.slot=IFSV_BINDING_SLOT_ID;
        }
      if(i!=35) 
        {
          if(i<42||i>56)
            {
              i_ifadd.spt_info.port=i;
            }
          else
            {
              i_ifadd.spt_info.port=(i/3)-3;
            }
        } 
      else
        {
          i_ifadd.spt_info.port=i-1;
        }
      if(i>8)
        {
          if(i<42||i>56) 
            {
              i_ifadd.spt_info.type=NCS_IFSV_INTF_OTHER;
            }
          else
            {
              i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
            }
        }
      
      if(i>11)
        {
          if(i<42||i>56)
            {
              i_ifadd.spt_info.subscr_scope=NCS_IFSV_SUBSCR_EXT;
            } 
          else
            {
              i_ifadd.spt_info.subscr_scope=NCS_IFSV_SUBSCR_INT;
            }
        }
      if(i>15)
        {
          i_ifadd.info_type=NCS_IFSV_IF_INFO;
        }
      if(i>26)
        {
          if(i<42||i>56)
            {
              i_ifadd.if_info.if_am=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|
                NCS_IFSV_IAM_PHYADDR|NCS_IFSV_IAM_ADMSTATE|
                NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|NCS_IFSV_IAM_DESCR|
                NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST;
            }
          else
            {
              i_ifadd.if_info.if_am=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|
                NCS_IFSV_IAM_PHYADDR|NCS_IFSV_IAM_ADMSTATE|
                NCS_IFSV_IAM_OPRSTATE|NCS_IFSV_IAM_NAME|NCS_IFSV_IAM_DESCR|
                NCS_IFSV_IAM_LAST_CHNG|NCS_IFSV_IAM_SVDEST|
                NCS_IFSV_IAM_CHNG_MASTER;
            }
        }
      if(i>27)
        {
          if(i<42||i>56) 
            {
              memcpy(i_ifadd.if_info.if_descr,if_descr,sizeof(if_descr));
            }
          else
            {
              memcpy(i_ifadd.if_info.if_descr,"binterface",
                     sizeof("binterface"));
            }
        }
      if(i>28)
        {
          if(i<42||i>56)
            {
              memcpy(i_ifadd.if_info.if_name,if_name,sizeof(if_name)); 
            }
          else
            {
              memcpy(i_ifadd.if_info.if_name,"bname",sizeof("bname")); 
            }
        }
      if(i>29&&i!=35)
        {
          i_ifadd.if_info.mtu=i; 
        }
      else
        {
          i_ifadd.if_info.mtu=i-1; 
        }
      if(i>30&&i!=35)
        {
          i_ifadd.if_info.if_speed=i*10; 
        } 
      else
        {
          i_ifadd.if_info.if_speed=(i-1)*10; 
        }
      if(i>32)
        {
          i_ifadd.if_info.admin_state=1; 
        }
      if(i>33&&i!=35)
        {
          i_ifadd.if_info.oper_state=1; 
        }
    }

  switch(i)
    {
    case 1:
      break;

    case 2:
         
      break;
 
    case 3:
   
      i_ifadd.spt_info.type=0;
      break;
 
    case 4:
     
      i_ifadd.spt_info.type=256;
      break;
 
    case 5:
    
      i_ifadd.spt_info.type=NCS_IFSV_INTF_OTHER;
      break;
 
    case 6:
     
      i_ifadd.spt_info.type=NCS_IFSV_INTF_LOOPBACK;
      break;
 
    case 7:
     
      i_ifadd.spt_info.type=NCS_IFSV_INTF_ETHERNET;
      break;
 
    case 8:
     
      i_ifadd.spt_info.type=NCS_IFSV_INTF_TUNNEL;
      break;
 
    case 9:
     
      i_ifadd.spt_info.subscr_scope=24;
      break;

    case 10:
    
      i_ifadd.spt_info.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      break;

    case 11:
    
      i_ifadd.spt_info.subscr_scope=NCS_IFSV_SUBSCR_INT;
      break;

    case 12:
  
      i_ifadd.info_type=0;
      break;

    case 13:
  
      i_ifadd.info_type=24;
      break;

    case 14:
  
      i_ifadd.info_type=NCS_IFSV_IF_INFO;
      break;

    case 15:
  
      i_ifadd.info_type=NCS_IFSV_IFSTATS_INFO;
      break;

    case 16:
  
      i_ifadd.if_info.if_am=0;
      break;

    case 17:
  
      i_ifadd.if_info.if_am=1024;
      break;

    case 18:
  
      i_ifadd.if_info.if_am=NCS_IFSV_IAM_MTU;
      break;

    case 19:
  
      i_ifadd.if_info.if_am=NCS_IFSV_IAM_IFSPEED;
      break;

    case 20:
  
      i_ifadd.if_info.if_am=NCS_IFSV_IAM_PHYADDR;
      break;

    case 21:
  
      i_ifadd.if_info.if_am=NCS_IFSV_IAM_ADMSTATE;
      break;

    case 22:
  
      i_ifadd.if_info.if_am=NCS_IFSV_IAM_OPRSTATE;
      break;

    case 23:
  
      i_ifadd.if_info.if_am=NCS_IFSV_IAM_NAME;
      break;

    case 24:
  
      i_ifadd.if_info.if_am=NCS_IFSV_IAM_DESCR;
      break;

    case 25:
  
      i_ifadd.if_info.if_am=NCS_IFSV_IAM_LAST_CHNG;
      break;

    case 26:
  
      i_ifadd.if_info.if_am=NCS_IFSV_IAM_SVDEST;
      break;

    case 27:
         
      memcpy(i_ifadd.if_info.if_descr,if_descr,sizeof(if_descr));
      break;

    case 28:

      memcpy(i_ifadd.if_info.if_name,if_name,sizeof(if_name)); 
      break;

    case 29:

      i_ifadd.if_info.mtu=i; 
      break;

    case 30:

      i_ifadd.if_info.if_speed=i*10; 
      break;
 
    case 31:
 
      break;

    case 32:

      i_ifadd.if_info.admin_state=1;
      break;

    case 33:

      i_ifadd.if_info.oper_state=1;
      break;
  
    case 34:
 
      break;

    case 35:

      i_ifadd.if_info.admin_state=1;
      i_ifadd.if_info.oper_state=1;
      i_ifadd.if_info.if_speed=i*2; 
      break; 
  
    case 36:
 
      i_ifadd.spt_info.subscr_scope=NCS_IFSV_SUBSCR_INT;
      break;

    case 37:

      i_ifadd.spt_info.subscr_scope=NCS_IFSV_SUBSCR_INT;
      i_ifadd.if_info.if_speed=i*1; 
      i_ifadd.spt_info.port=i-1;
      break;

    case 38:
    
      i_ifadd.spt_info.type=NCS_IFSV_INTF_OTHER;
      break;
 
    case 39:

      i_ifadd.spt_info.type=NCS_IFSV_INTF_LOOPBACK;
      break;
 
    case 40:

      i_ifadd.spt_info.type=NCS_IFSV_INTF_ETHERNET;
      break;
 
    case 41:

      i_ifadd.spt_info.type=NCS_IFSV_INTF_TUNNEL;
      break;

    case 42:
  
      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.spt_info.subscr_scope=NCS_IFSV_SUBSCR_INT;

      i_ifadd.if_info.bind_master_ifindex=0;
      i_ifadd.if_info.bind_slave_ifindex=0;
      break;

    case 43:
  
      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.spt_info.subscr_scope=NCS_IFSV_SUBSCR_INT;
      i_ifadd.spt_info.shelf=shelf_id;

      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;
      break;

    case 44:
  
      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.spt_info.slot=slot_id;    
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;
      break;

    case 45:

      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;  
      break;

    case 46:

      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;  
      i_ifadd.spt_info.port=1;
      break; 

    case 47:

      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;  
      i_ifadd.spt_info.port=0;
      break; 

    case 48:

      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;  
      i_ifadd.spt_info.port=30;
      break; 

    case 49:
      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;  
      i_ifadd.spt_info.port=31;
      break;

    case 50:
      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;  
      i_ifadd.spt_info.subscr_scope=NCS_IFSV_SUBSCR_ALL;
      break;

    case 51:
      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=0;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;  
      break;

    case 52:
      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=0;  
      break;

    case 53:
      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;  
      break;

    case 54:
      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex+1;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex+1;  
      break;

    case 55:
      i_ifadd.spt_info.type=NCS_IFSV_INTF_BINDING;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;  
      i_ifadd.spt_info.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      break;

    case 56:
      i_ifadd.spt_info.type=NCS_IFSV_INTF_OTHER;
      i_ifadd.if_info.bind_master_ifindex=b_iface.mifindex;
      i_ifadd.if_info.bind_slave_ifindex=b_iface.sifindex;  
      break;
    default:
   
      break;
    }
}

void ifsv_ifrec_add(int i)
{
  static int try_again_count;
  ifsv_ifrec_add_fill(i);

  req_info.i_req_type=ifrec_add_api[i].i_req_type;
  req_info.info.i_ifadd.spt_info.shelf
    =ifrec_add_api[i].i_ifadd->spt_info.shelf;
  req_info.info.i_ifadd.spt_info.slot=ifrec_add_api[i].i_ifadd->spt_info.slot;
  req_info.info.i_ifadd.spt_info.port=ifrec_add_api[i].i_ifadd->spt_info.port;
  req_info.info.i_ifadd.spt_info.type=ifrec_add_api[i].i_ifadd->spt_info.type;
  req_info.info.i_ifadd.spt_info.subscr_scope
    =ifrec_add_api[i].i_ifadd->spt_info.subscr_scope;

  req_info.info.i_ifadd.info_type=ifrec_add_api[i].i_ifadd->info_type;

  req_info.info.i_ifadd.if_info.if_am=ifrec_add_api[i].i_ifadd->if_info.if_am;
  memcpy(req_info.info.i_ifadd.if_info.if_descr,
         ifrec_add_api[i].i_ifadd->if_info.if_descr,
         sizeof(ifrec_add_api[i].i_ifadd->if_info.if_descr));
  memcpy(req_info.info.i_ifadd.if_info.if_name,
         ifrec_add_api[i].i_ifadd->if_info.if_name,
         sizeof(ifrec_add_api[i].i_ifadd->if_info.if_name));
  req_info.info.i_ifadd.if_info.mtu=ifrec_add_api[i].i_ifadd->if_info.mtu;
  req_info.info.i_ifadd.if_info.if_speed
    =ifrec_add_api[i].i_ifadd->if_info.if_speed;
  req_info.info.i_ifadd.if_info.admin_state
    =ifrec_add_api[i].i_ifadd->if_info.admin_state;
  req_info.info.i_ifadd.if_info.oper_state
    =ifrec_add_api[i].i_ifadd->if_info.oper_state;

  if(req_info.info.i_ifadd.spt_info.type==NCS_IFSV_INTF_BINDING)
    {
      req_info.info.i_ifadd.if_info.bind_master_ifindex
        =ifrec_add_api[i].i_ifadd->if_info.bind_master_ifindex;
      req_info.info.i_ifadd.if_info.bind_slave_ifindex
        =ifrec_add_api[i].i_ifadd->if_info.bind_slave_ifindex;
    }
   
  gl_ifIndex=0;
  gl_cbk=0;
  gl_rc=ncs_ifsv_svc_req(&req_info);
  WHILE_TRY_AGAIN(gl_rc)
    {
      ++try_again_count;
      RETRY_SLEEP;
      gl_rc=ncs_ifsv_svc_req(&req_info);
      if(gl_rc==SA_AIS_OK)
        printf("\n Interface Add Request Try Again Count = %d\n",try_again_count);
    }
  sleep(3); /*Wait for Dispatch*/
  if( (gl_ifIndex==0) && (gl_cbk==0) && (gl_rc==1) 
      && (usr_req_info.i_subr.i_sevts==NCS_IFSV_ADD_IFACE) )
    {
      printf("\nInterface is not added");
      tet_printf("Interface is not added");
      tet_result(TET_FAIL); 
    }
}

void ifsv_ifrec_del_fill(int i)
{
  switch(i)
    {
    case 1:

      break;

    case 2:

      i_ifdel.shelf=shelf_id;
      i_ifdel.slot=slot_id;
      i_ifdel.port=30;
      i_ifdel.type=NCS_IFSV_INTF_OTHER;
      i_ifdel.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      break;       

    case 3:

      i_ifdel.shelf=shelf_id;
      i_ifdel.slot=slot_id;
      i_ifdel.port=34;
      i_ifdel.type=256;
      i_ifdel.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      break;

    case 4:

      i_ifdel.shelf=shelf_id;
      i_ifdel.slot=slot_id;
      i_ifdel.port=34;
      i_ifdel.type=NCS_IFSV_INTF_OTHER;
      i_ifdel.subscr_scope=24;
      break;

    case 5:

      i_ifdel.shelf=shelf_id;
      i_ifdel.slot=slot_id;
      i_ifdel.port=req_info.info.i_ifadd.spt_info.port;
      i_ifdel.type=req_info.info.i_ifadd.spt_info.type;
      i_ifdel.subscr_scope=req_info.info.i_ifadd.spt_info.subscr_scope;
      break;

    case 6:
  
      i_ifdel.shelf=req_info.info.i_ifadd.spt_info.shelf;
      i_ifdel.slot=req_info.info.i_ifadd.spt_info.slot;
      i_ifdel.port=req_info.info.i_ifadd.spt_info.port;
      i_ifdel.type=req_info.info.i_ifadd.spt_info.type;
      i_ifdel.subscr_scope=req_info.info.i_ifadd.spt_info.subscr_scope;
 
    default:
 
      break;
    }
}

void ifsv_ifrec_del(int i)
{
  ifsv_ifrec_del_fill(i);

  req_info.i_req_type=ifrec_del_api[i].i_req_type;
  req_info.info.i_ifdel.shelf=ifrec_del_api[i].i_ifdel->shelf;
  req_info.info.i_ifdel.slot=ifrec_del_api[i].i_ifdel->slot;
  req_info.info.i_ifdel.port=ifrec_del_api[i].i_ifdel->port;
  req_info.info.i_ifdel.type=ifrec_del_api[i].i_ifdel->type;
  req_info.info.i_ifdel.subscr_scope=ifrec_del_api[i].i_ifdel->subscr_scope;

  gl_rc=ncs_ifsv_svc_req(&req_info);
  if(gl_rc==NCSCC_RC_SUCCESS)
    sleep(15);
}

void ifsv_ifrec_get_fill(int i)
{
  i_ifget.i_rsp_type=NCS_IFSV_GET_RESP_ASYNC;
  i_ifget.i_subr_hdl=gl_subscription_hdl;
  switch(i)
    {
    case 1:
  
      break;

    case 2:
         
      i_ifget.i_key.type=24;
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      break;

    case 3:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      break;

    case 4:

      i_ifget.i_key.type=NCS_IFSV_KEY_SPT;
      i_ifget.i_key.info.spt.shelf=shelf_id;
      i_ifget.i_key.info.spt.slot=slot_id;
      i_ifget.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ifget.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type;
      i_ifget.i_key.info.spt.subscr_scope=
        req_info.info.i_ifadd.spt_info.subscr_scope;
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      break;

    case 5:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;
       
      i_ifget.i_info_type=24;
      break;

    case 6:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;
       
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      break;

    case 7:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;
       
      i_ifget.i_info_type=NCS_IFSV_IFSTATS_INFO;
      break;

    case 8:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;
       
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;

      i_ifget.i_rsp_type=24;
      break;

    case 9:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;
       
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      i_ifget.i_rsp_type=NCS_IFSV_GET_RESP_SYNC;
      break;

    case 10:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;
       
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      break;

    case 11:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;
       
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;

      break;

    case 12:

      i_ifget.i_key.type=NCS_IFSV_KEY_SPT;
      i_ifget.i_key.info.spt.shelf=shelf_id;
      i_ifget.i_key.info.spt.slot=slot_id;
      i_ifget.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ifget.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type;
      i_ifget.i_key.info.spt.subscr_scope=
        req_info.info.i_ifadd.spt_info.subscr_scope;
       
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      break;

    case 13:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;
       
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      i_ifget.i_rsp_type=NCS_IFSV_GET_RESP_SYNC;
      break;

    case 14:

      i_ifget.i_key.type=NCS_IFSV_KEY_SPT;
      i_ifget.i_key.info.spt.shelf=shelf_id;
      i_ifget.i_key.info.spt.slot=slot_id;
      i_ifget.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ifget.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type;
      i_ifget.i_key.info.spt.subscr_scope=
        req_info.info.i_ifadd.spt_info.subscr_scope;
       
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      i_ifget.i_rsp_type=NCS_IFSV_GET_RESP_SYNC;
      break;

    case 15:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;

      i_ifget.i_info_type=NCS_IFSV_IFSTATS_INFO;
      break;

    case 16:

      i_ifget.i_key.type=NCS_IFSV_KEY_SPT;
      i_ifget.i_key.info.spt.shelf=shelf_id;
      i_ifget.i_key.info.spt.slot=slot_id;
      i_ifget.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ifget.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type;
      i_ifget.i_key.info.spt.subscr_scope=
        req_info.info.i_ifadd.spt_info.subscr_scope;
       
      i_ifget.i_info_type=NCS_IFSV_IFSTATS_INFO;
      break;

    case 17:

      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;

      i_ifget.i_info_type=NCS_IFSV_IFSTATS_INFO;
      i_ifget.i_rsp_type=NCS_IFSV_GET_RESP_SYNC;
      break;

    case 18:

      i_ifget.i_key.type=NCS_IFSV_KEY_SPT;
      i_ifget.i_key.info.spt.shelf=shelf_id;
      i_ifget.i_key.info.spt.slot=slot_id;
      i_ifget.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ifget.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type;
      i_ifget.i_key.info.spt.subscr_scope=
        req_info.info.i_ifadd.spt_info.subscr_scope;
       
      i_ifget.i_info_type=NCS_IFSV_IFSTATS_INFO;
      i_ifget.i_rsp_type=NCS_IFSV_GET_RESP_SYNC;
      break;

    case 19:

      i_ifget.i_key.type=NCS_IFSV_KEY_SPT;
      i_ifget.i_key.info.spt.shelf=shelf_id;
      i_ifget.i_key.info.spt.slot=slot_id;
      i_ifget.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ifget.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type;
      i_ifget.i_key.info.spt.subscr_scope=12;

      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      break;

    case 20:

      i_ifget.i_key.type=NCS_IFSV_KEY_SPT;
      i_ifget.i_key.info.spt.shelf=shelf_id;
      i_ifget.i_key.info.spt.slot=slot_id;
      i_ifget.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ifget.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type;
      i_ifget.i_key.info.spt.subscr_scope=NCS_IFSV_SUBSCR_EXT;

      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      break;

    case 21:
      i_ifget.i_key.type=NCS_IFSV_KEY_IFINDEX;
      i_ifget.i_key.info.iface=gl_ifIndex;
      i_ifget.i_info_type=NCS_IFSV_BIND_GET_LOCAL_INTF;
      break;
    
    case 22:
      i_ifget.i_key.type=NCS_IFSV_KEY_SPT;
      i_ifget.i_key.info.spt.shelf=IFSV_BINDING_SHELF_ID;
      i_ifget.i_key.info.spt.slot=IFSV_BINDING_SLOT_ID;
      i_ifget.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ifget.i_key.info.spt.type=NCS_IFSV_INTF_BINDING;
      i_ifget.i_key.info.spt.subscr_scope=NCS_IFSV_SUBSCR_INT;
      i_ifget.i_info_type=NCS_IFSV_IF_INFO;
      break; 
    
    case 23:
      i_ifget.i_key.type=NCS_IFSV_KEY_SPT;
      i_ifget.i_key.info.spt.shelf=IFSV_BINDING_SHELF_ID;
      i_ifget.i_key.info.spt.slot=IFSV_BINDING_SLOT_ID;
      i_ifget.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ifget.i_key.info.spt.type=NCS_IFSV_INTF_BINDING;
      i_ifget.i_key.info.spt.subscr_scope=NCS_IFSV_SUBSCR_INT;
      i_ifget.i_info_type=NCS_IFSV_BIND_GET_LOCAL_INTF;
      break; 

    default:

      break;         
    }
}

void ifsv_ifrec_get(int i)
{
  ifsv_ifrec_get_fill(i);
  
  req_info.i_req_type=ifrec_get_api[i].i_req_type;
  req_info.info.i_ifget.i_key.type=ifrec_get_api[i].i_ifget->i_key.type;
   
  if(req_info.info.i_ifget.i_key.type==NCS_IFSV_KEY_IFINDEX)
    {
      req_info.info.i_ifget.i_key.info.iface=
        ifrec_get_api[i].i_ifget->i_key.info.iface;
    }
  else if(req_info.info.i_ifget.i_key.type==NCS_IFSV_KEY_SPT)
    {
      req_info.info.i_ifget.i_key.info.spt.shelf=
        ifrec_get_api[i].i_ifget->i_key.info.spt.shelf; 
      req_info.info.i_ifget.i_key.info.spt.slot=
        ifrec_get_api[i].i_ifget->i_key.info.spt.slot; 
      req_info.info.i_ifget.i_key.info.spt.port=
        ifrec_get_api[i].i_ifget->i_key.info.spt.port; 
      req_info.info.i_ifget.i_key.info.spt.type=
        ifrec_get_api[i].i_ifget->i_key.info.spt.type; 
      req_info.info.i_ifget.i_key.info.spt.subscr_scope=
        ifrec_get_api[i].i_ifget->i_key.info.spt.subscr_scope; 
    }
  req_info.info.i_ifget.i_info_type=ifrec_get_api[i].i_ifget->i_info_type;
  req_info.info.i_ifget.i_rsp_type=ifrec_get_api[i].i_ifget->i_rsp_type;
  req_info.info.i_ifget.i_subr_hdl=gl_subscription_hdl;

  gl_cbk=0;
  gl_error=0;
  gl_rc=ncs_ifsv_svc_req(&req_info);
  sleep(2);
  if(req_info.info.i_ifget.i_rsp_type==NCS_IFSV_GET_RESP_SYNC)
    {
      printf("\n********************\n");
      printf("\nInterface\n");
      printf("--------------------");
      printf("\n\nReturn Code: %d",req_info.o_ifget_rsp.error);
      gl_error=req_info.o_ifget_rsp.error;
      printf("\nifIndex: %d",req_info.o_ifget_rsp.if_rec.if_index);
      printf("\nShelf Id: %d",req_info.o_ifget_rsp.if_rec.spt_info.shelf);
      printf("\nSlot Id: %d",req_info.o_ifget_rsp.if_rec.spt_info.slot);
      printf("\nPort Id: %d",req_info.o_ifget_rsp.if_rec.spt_info.port);
      printf("\nType: %d",req_info.o_ifget_rsp.if_rec.spt_info.type);
      printf("\nSubscriber Scope: %d",
             req_info.o_ifget_rsp.if_rec.spt_info.subscr_scope);
      printf("\nInfo Type: %d",req_info.o_ifget_rsp.if_rec.info_type);
      printf("\n\nInterface Information\n");
      printf("--------------------");
      printf("\nAttribute Map Flag: %d",
             req_info.o_ifget_rsp.if_rec.if_info.if_am);
      printf("\nInterface Description: %s",
             req_info.o_ifget_rsp.if_rec.if_info.if_descr);
      printf("\nInterface Name: %s",
             req_info.o_ifget_rsp.if_rec.if_info.if_name);
      printf("\nMTU: %d",req_info.o_ifget_rsp.if_rec.if_info.mtu);
      printf("\nIf Speed: %d",req_info.o_ifget_rsp.if_rec.if_info.if_speed);
      printf("\nAdmin State: %d",
             req_info.o_ifget_rsp.if_rec.if_info.admin_state);
      printf("\nOperation Status: %d",
             req_info.o_ifget_rsp.if_rec.if_info.oper_state);
      printf("\n\nInterface Statistics\n");
      printf("--------------------");
      printf("\nLast Change: %d",
             req_info.o_ifget_rsp.if_rec.if_stats.last_chg);
      printf("\nIn Octets: %d",req_info.o_ifget_rsp.if_rec.if_stats.in_octs);
      printf("\nIn Upkts: %d",req_info.o_ifget_rsp.if_rec.if_stats.in_upkts);
      printf("\nIn Nupkts: %d",req_info.o_ifget_rsp.if_rec.if_stats.in_nupkts);
      printf("\nIn Dscrds: %d",req_info.o_ifget_rsp.if_rec.if_stats.in_dscrds);
      printf("\nIn Errs: %d",req_info.o_ifget_rsp.if_rec.if_stats.in_dscrds);
      printf("\nIn Unkown Prots: %d",
             req_info.o_ifget_rsp.if_rec.if_stats.in_unknown_prots);
      printf("\nOut Octs: %d",req_info.o_ifget_rsp.if_rec.if_stats.out_octs);
      printf("\nOut Upkts: %d",req_info.o_ifget_rsp.if_rec.if_stats.out_upkts);
      printf("\nOut Nupkts: %d",
             req_info.o_ifget_rsp.if_rec.if_stats.out_nupkts);
      printf("\nOut Dscrds: %d",
             req_info.o_ifget_rsp.if_rec.if_stats.out_dscrds);
      printf("\nOut Errs: %d",req_info.o_ifget_rsp.if_rec.if_stats.out_errs);
      printf("\nOut Qlen: %d",req_info.o_ifget_rsp.if_rec.if_stats.out_qlen);
      printf("\nIf Specific: %d",
             req_info.o_ifget_rsp.if_rec.if_stats.if_specific);
    }
  else
    {
      sleep(5);/*Lets wait for dispatch */
      if(gl_cbk==0&&gl_rc==1)
        {
          printf("\nCallback not invoked to get interface information");
          tet_printf("Callback not invoked to get interface information");
          tet_result(TET_FAIL); 
        }
    }
}

void ifsv_svc_dest_get_fill(int i)
{
  switch(i)
    {
    case 1:

      i_svd_get.i_ifindex=gl_ifIndex;
      i_svd_get.i_svcid=NCSMDS_SVC_ID_IFA;
        
      break;
   
    default:
      break;
    }
}

void ifsv_svc_dest_get(int i)
{
  ifsv_svc_dest_get_fill(i);

  req_info.i_req_type=NCS_IFSV_REQ_SVC_DEST_GET;
   
  req_info.info.i_svd_get.i_ifindex=svc_dest_get_api[i].i_svd_get->i_ifindex;
  req_info.info.i_svd_get.i_svcid=svc_dest_get_api[i].i_svd_get->i_svcid;

  gl_rc=ncs_ifsv_svc_req(&req_info);
  if(gl_rc==NCSCC_RC_SUCCESS)
    {
      printf("\nMDS destination: %llu",req_info.info.i_svd_get.o_dest);
      printf("\nAnswer (boolean): %d",req_info.info.i_svd_get.o_answer);
    }   
   
}

/*********************************************************************/
/*********************** IFSV API TEST CASES *************************/
/*********************************************************************/

void tet_ncs_ifsv_svc_req_subscribe(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n--------tet_ncs_ifsv_svc_req_subscribe : %d -------\n",iOption);
  switch(iOption)
    {
    case 1:  

      ifsv_subscription(IFSV_NULL_REQUEST_TYPE);            
      ifsv_result("ncs_ifsv_svc_req() with NULL request type",
                  NCSCC_RC_FAILURE);
      break;

    case 2:

      ifsv_subscription(IFSV_INVALID_REQUEST_TYPE);
      ifsv_result("ncs_ifsv_svc_req() with invalid request type",
                  NCSCC_RC_FAILURE);
      break;

    case 3:

      ifsv_subscription(IFSV_SUBSCRIBE_NULL_CALLBACKS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NULL callbacks",
                  NCSCC_RC_FAILURE);
      break;

    case 4:
      
      ifsv_subscription(IFSV_SUBSCRIBE_INVALID_SUBSCR_SCOPE);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with invalid subscriber\
 scope",NCSCC_RC_FAILURE);
      break;

    case 5:

      ifsv_subscription(IFSV_SUBSCRIBE_EXTERNAL_SUBSCR_SCOPE);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with external subscriber \
scope",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 6:

      ifsv_subscription(IFSV_SUBSCRIBE_INTERNAL_SUBSCR_SCOPE);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with internal subscriber\
 scope",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 7:

      ifsv_subscription(IFSV_SUBSCRIBE_NULL_EVENT_SUBSCRIPTION);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NULL event\
 subscription",NCSCC_RC_FAILURE);
      break;

    case 8:

      ifsv_subscription(IFSV_SUBSCRIBE_INVALID_EVENT_SUBSCRIPTION);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with invalid event\
 subscription",NCSCC_RC_FAILURE);
      break;
   
    case 9:

      ifsv_subscription(IFSV_SUBSCRIBE_ADD_IFACE);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_ADD_IFACE \
(event subscription)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 10:

      ifsv_subscription(IFSV_SUBSCRIBE_RMV_IFACE);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_RMV_IFACE \
(event subscription)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 11:

      ifsv_subscription(IFSV_SUBSCRIBE_UPD_IFACE);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_UPD_IFACE \
(event subscription)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 12:

      ifsv_subscription(IFSV_SUBSCRIBE_NULL_ATTRIBUTES_MAP);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NULL attributes map",
                  NCSCC_RC_FAILURE);
      break;

    case 13:

      ifsv_subscription(IFSV_SUBSCRIBE_INVALID_ATTRIBUTES_MAP);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with invalid attributes map"
                  ,NCSCC_RC_FAILURE);
      break;

    case 14:

      ifsv_subscription(IFSV_SUBSCRIBE_IAM_MTU);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_IAM_MTU \
(attributes map)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 15:

      ifsv_subscription(IFSV_SUBSCRIBE_IAM_IFSPEED);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_IAM_IFSPEED\
 (attributes map)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 16:

      ifsv_subscription(IFSV_SUBSCRIBE_IAM_PHYADDR);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_IAM_PHYADDR\
 (attributes map)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 17:

      ifsv_subscription(IFSV_SUBSCRIBE_IAM_ADMSTATE);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_IAM_ADMSTATE\
 (attributes map)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 18:

      ifsv_subscription(IFSV_SUBSCRIBE_IAM_OPRSTATE);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_IAM_OPRSTATE\
 (attributes map)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 19:

      ifsv_subscription(IFSV_SUBSCRIBE_IAM_NAME);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_IAM_NAME\
 (attributes map)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 20:

      ifsv_subscription(IFSV_SUBSCRIBE_IAM_DESCR);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_IAM_DESCR\
 (attributes map)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 21:

      ifsv_subscription(IFSV_SUBSCRIBE_IAM_LAST_CHNG);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_IAM_LAST_CHNG\
 (attributes map)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 22:

      ifsv_subscription(IFSV_SUBSCRIBE_IAM_SVDEST);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with NCS_IFSV_IAM_SVDEST\
 (attributes map)",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 23:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with all event \
subscriptions and all attribute maps",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 24:

      ifsv_subscription(IFSV_SUBSCRIBE_INTERNAL_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe with all event \
subscriptions and all attribute maps",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;
    }   
  printf("\n-----------------------END : %d -------------------\n",iOption);
}

void tet_ncs_ifsv_svc_req_unsubscribe(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n--------tet_ncs_ifsv_svc_req_unsubscribe : %d -------\n",iOption);
  switch(iOption)
    {
    case 1:
         
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_NULL_HANDLE);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe with NULL subscription \
handle",NCSCC_RC_FAILURE);
      break;
    case 2:
  
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_INVALID_HANDLE);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe with invalid subscription\
 handle",NCSCC_RC_FAILURE);
      break;
    case 3:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe",NCSCC_RC_SUCCESS); 

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_DUPLICATE_HANDLE);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_DUPLICATE_HANDLE);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe with duplicate \
subscription handle",NCSCC_RC_FAILURE);
      break;

    case 4:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe",NCSCC_RC_SUCCESS); 

      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;
    }
  printf("\n----------------------- END  : %d -------------------\n",iOption);
}

void tet_ncs_ifsv_svc_req_ifrec_add(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n--------tet_ncs_ifsv_svc_req_ifrec_add : %d --------\n",iOption);
  switch(iOption)
    {
    case 1:
  
      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));
         
      ifsv_ifrec_add(IFSV_IFREC_ADD_NULL_PARAM);
      printf("\n\nncs_ifsv_svc_req() to add interface with NULL param");
      printf("\nFailure : %d",gl_rc);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 2:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SPT);
      ifsv_result("ncs_ifsv_svc_req() to add interface with SPT info",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 3:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_TYPE_NULL);
      ifsv_result("ncs_ifsv_svc_req() to add interface with NULL type",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 4:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_TYPE_INVALID);
      ifsv_result("ncs_ifsv_svc_req() to add interface with invalid type",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 5:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_TYPE_INTF_OTHER);
      ifsv_result("ncs_ifsv_svc_req() to add interface with interface other\
 type",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 6:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_TYPE_INTF_LOOPBACK);
      ifsv_result("ncs_ifsv_svc_req() to add interface with loopback type",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 7:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_TYPE_INTF_ETHERNET);
      ifsv_result("ncs_ifsv_svc_req() to add interface with ethernet type",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 8:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_TYPE_INTF_TUNNEL);
      ifsv_result("ncs_ifsv_svc_req() to add interface with tunnel type",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 9:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUBSCR_SCOPE_INVALID);
      ifsv_result("ncs_ifsv_svc_req() to add interface with invalid subscriber\
 scope",NCSCC_RC_FAILURE);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 10:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUBSCR_SCOPE_EXTERNAL);
      ifsv_result("ncs_ifsv_svc_req() to add interface with external \
subscriber scope",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 11:

      ifsv_subscription(IFSV_SUBSCRIBE_INTERNAL_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUBSCR_SCOPE_INTERNAL);
      ifsv_result("ncs_ifsv_svc_req() to add interface with internal \
subscribe scope",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 12:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INFO_TYPE_NULL);
      ifsv_result("ncs_ifsv_svc_req() to add interface with NULL type",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 13:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INFO_TYPE_INVALID);
      ifsv_result("ncs_ifsv_svc_req() to add interface with invalid type",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 14:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INFO_TYPE_IF_INFO);
      ifsv_result("ncs_ifsv_svc_req() to add interface with if_info type",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 15:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INFO_TYPE_IFSTATS_INFO);
      ifsv_result("ncs_ifsv_svc_req() to add interface with ifstats type",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 16:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_NULL);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM NULL",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 17:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_INVALID);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM invalid",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 18:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_MTU);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM mtu",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 19:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_IFSPEED);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM ifspeed",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 20:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_PHYADDR);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM phyaddr",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 21:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_ADMSTATE);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM admin_state",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 22:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_OPRSTATE);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM oper_state",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 23:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_NAME);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM name",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 24:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_DESCR);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM descr",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 25:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_LAST_CHNG);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM last_chng",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 26:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IAM_SVDEST);
      ifsv_result("ncs_ifsv_svc_req() to add interface with IAM svdest",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 27:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IF_DESCR);
      ifsv_result("ncs_ifsv_svc_req() to add interface with if_descr",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 28:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IF_NAME);
      ifsv_result("ncs_ifsv_svc_req() to add interface with if_name",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 29:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_MTU);
      ifsv_result("ncs_ifsv_svc_req() to add interface with mtu",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 30:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_IF_SPEED);
      ifsv_result("ncs_ifsv_svc_req() to add interface with if_speed",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 31: 
       
      break;

    case 32:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_ADMIN_STATE);
      ifsv_result("ncs_ifsv_svc_req() to add interface with admin_state",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 33:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_IF_INFO_OPER_STATE);
      ifsv_result("ncs_ifsv_svc_req() to add interface with oper_state",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 34:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 35:
      break;
    case 36:
      break;
    case 37:
      break;

    case 38:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INTF_OTHER_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface (type: intf_other)",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 39:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INTF_LOOPBACK_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface (type: intf_loopback)",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 40:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INTF_ETHERNET_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface (type: intf_ethernet)",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
        
      break;

    case 41:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
                  NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INTF_TUNNEL_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface (type: intf_tunnel)",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

      break;

    }
  printf("\n----------------------- END  : %d -------------------\n",iOption);
}

void tet_ncs_ifsv_svc_req_ifrec_del(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n-------- tet_ncs_ifsv_svc_req_ifrec_del : %d -------\n",iOption);
  switch(iOption)
    {
    case 1:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));
         
      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_del(IFSV_IFREC_DEL_NULL_PARAM);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface with NULL param",
                  NCSCC_RC_FAILURE);
      sleep(1);
 
      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);  
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

      break; 
   
    case 2:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_del(IFSV_IFREC_DEL_INVALID_SPT);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface with invalid\
 (shelf,slot and port)",NCSCC_RC_FAILURE);
      sleep(1);
 
      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);  
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;
   
    case 3:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_INVALID_TYPE);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface with invalid type",
                  NCSCC_RC_FAILURE);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);  
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;
   
    case 4:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_INVALID_SUBSCR_SCOPE);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface with invalid\
 subscriber scope",NCSCC_RC_FAILURE);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);  
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;
   
    case 5:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;
    }
  printf("\n----------------------- END  : %d -------------------\n",iOption);
}

void tet_ncs_ifsv_svc_req_ifrec_get(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n-------- tet_ncs_ifsv_svc_req_ifrec_get : %d -------\n",iOption);
  switch(iOption)
    {
    case 1:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));
         
      ifsv_ifrec_get(IFSV_IFREC_GET_NULL_PARAM);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with NULL\
 param",NCSCC_RC_FAILURE);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 2:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_KEY_INVALID);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with key\
 invalid",NCSCC_RC_SUCCESS);
      sleep(1);
         
      if(gl_error!=2)
        {
          printf("\nInterface get information was successful for invalid\
 arguments"); 
          tet_printf("\nInterface get information was successful for invalid\
 arguments"); 
          tet_result(TET_FAIL);
        }

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 3:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));
         
      ifsv_ifrec_get(IFSV_IFREC_GET_KEY_IFINDEX);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with key\
 ifindex",NCSCC_RC_SUCCESS);
      sleep(1);
         
      if(gl_error!=1)
        {
          printf("\nInterface get information was not successful"); 
          tet_printf("\nInterface get information was not successful"); 
          tet_result(TET_FAIL);
        }

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 4:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_KEY_SPT);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with key\
 spt",NCSCC_RC_SUCCESS);
      sleep(1);
         
      if(gl_error!=1)
        {
          printf("\nInterface get information was not successful"); 
          tet_printf("\nInterface get information was not successful"); 
          tet_result(TET_FAIL);
        }

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 5:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_INFO_TYPE_INVALID);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with \
invalid info_type",NCSCC_RC_FAILURE);
      sleep(1);
         
      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 6:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_INFO_TYPE_IF_INFO);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with \
if_info info_type",NCSCC_RC_SUCCESS);
      sleep(1);
         
      if(gl_error!=1)
        {
          printf("\nInterface get information was not successful"); 
          tet_printf("\nInterface get information was not successful"); 
          tet_result(TET_FAIL);
        }

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 7:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_INFO_TYPE_IFSTATS_INFO);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with \
ifstats info_type",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 8:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_RESP_TYPE_INVALID);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with \
invalid resp_type",NCSCC_RC_FAILURE);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 9:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_RESP_TYPE_SYNC);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with sync\
 resp_type",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 10:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_RESP_TYPE_ASYNC);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with async\
 resp_type",NCSCC_RC_SUCCESS);
      sleep(1);
         
      if(gl_error!=1)
        {
          printf("\nInterface get information was not successful"); 
          tet_printf("\nInterface get information was not successful"); 
          tet_result(TET_FAIL);
        }

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 11:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_IFINDEX_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with key \
ifindex",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 12:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_SPT_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with key\
 spt",NCSCC_RC_SUCCESS);
      sleep(1);
         
      if(gl_error!=1)
        {
          printf("\nInterface get information was not successful"); 
          tet_printf("\nInterface get information was not successful"); 
          tet_result(TET_FAIL);
        }

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 13:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_SYNC_IFINDEX_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to get (sync) interface information with\
 key ifindex",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 14:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_SYNC_SPT_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to get (sync) interface information with\
 key spt",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 15:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_IFINDEX_STATS_INFO);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with key\
 ifindex (stats info)",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 16:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_SPT_STATS_INFO);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with key\
 spt (stats info)",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 17:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_SYNC_IFINDEX_STATS_INFO);
      ifsv_result("ncs_ifsv_svc_req() to get (sync) interface information with\
 key ifindex (stats info)",NCSCC_RC_FAILURE);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 18:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_SYNC_SPT_STATS_INFO);
      ifsv_result("ncs_ifsv_svc_req() to get (sync) interface information with\
 key spt (stats info)",NCSCC_RC_FAILURE);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 19:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition \
and deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_INVALID_SUBSCR_SCOPE);
      ifsv_result("ncs_ifsv_svc_req() to get interface with invalid subscriber\
 scope",NCSCC_RC_SUCCESS);
      sleep(1);
         
      if(gl_error!=2)
        {
          printf("\nInterface get information was successful for invalid \
arguments"); 
          tet_printf("\nInterface get information was successful for invalid\
 arguments"); 
          tet_result(TET_FAIL);
        }

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;

    case 20:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_SUBSCR_SCOPE);
      ifsv_result("ncs_ifsv_svc_req() to get iterface with subscriber handle",
                  NCSCC_RC_SUCCESS);
      sleep(1);
         
      if(gl_error!=1)
        {
          printf("\nInterface get information was not successful"); 
          tet_printf("\nInterface get information not successful"); 
          tet_result(TET_FAIL);
        }

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;
    case 21:
      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_ifrec_get(IFSV_IFREC_GET_KEY_IFINDEX);
      ifsv_result("ncs_ifsv_svc_req() to get interface information with key\
 ifindex",NCSCC_RC_SUCCESS);
      sleep(1);

      if(gl_error!=1)
        {
          printf("\nInterface get information was not successful");
          tet_printf("\nInterface get information was not successful");
          tet_result(TET_FAIL);
        }

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

      break;
    }
  printf("\n----------------------- END  : %d -------------------\n",iOption);
}

void tet_ncs_ifsv_svc_req_dest_get(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n-------- tet_ncs_ifsv_svc_req_dest_get : %d -------\n",iOption);
  switch(iOption)
    {
    case 1:

      ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition\
 and deletion",NCSCC_RC_SUCCESS);

      memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
             sizeof(req_info.info.i_subr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
             sizeof(req_info.info.i_ifadd));

      ifsv_svc_dest_get(IFSV_SVC_DEST_GET_NULL_PARAM);
      ifsv_result("ncs_ifsv_svc_req() to get svc dest with NULL PARAM",
                  NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
             sizeof(usr_req_info.i_ifadd));
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
             sizeof(usr_req_info.i_subr));
      ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      
      break;
                      
  
    }
  printf("\n----------------------- END  : %d -------------------\n",iOption);
}

/*************************************************************************/
/**********************FUNCTIONALITY TEST CASES***************************/
/*************************************************************************/
void kill_ifnd()
{
  static int count;
  printf("\n\t Killing ifnd : Wait for Sometime : Count = %d\n",++count);
  tet_printf("\n\t Killing ifnd : Count = %d\n",count);
  fflush(stdout);
  sleep(1);
  system("sudo kill ncs_ifnd.out");
  sleep(4);
}

void tet_interface_subscribe()
{
  printf("\n----------------tet_interface_subscribe--------------------\n");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe without adding interface",
              NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));


  
    

  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");
}
void tet_interface_add_subscribe()
{
  printf("\n-------------tet_interface_add_subscribe-------------------\n");
  printf("\n\nInterface addition subscription");

  ifsv_subscription(IFSV_SUBSCRIBE_EXT_ADD_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition",
              NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);

  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);
  
    

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         

  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");
}
void tet_interface_upd_subscribe()
{
  printf("\n-------------tet_interface_upd_subscribe-------------------\n");
  printf("\n\nInterface Update : External Subscription");

  ifsv_subscription(IFSV_SUBSCRIBE_EXT_UPD_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface updation",
              NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  



  ifsv_ifrec_add(IFSV_IFREC_ADD_UPD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);

  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_rmv_subscribe()
{
  printf("\n----------tet_interface_rmv_subscribe----------------------\n");
  printf("\n\nInterface Removal : External Subscription");

  ifsv_subscription(IFSV_SUBSCRIBE_EXT_RMV_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface removal",
              NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  



  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);

  
    


  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);


  printf("\n-----------------------END---------------------------------\n");
}
#if 0
void tet_interface_subscribe_all()
{
  printf("\n------------tet_interface_subscribe_all--------------------\n");
  printf("\n\nInterface External Subscription For ALL");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition, \
updation and removal",NCSCC_RC_SUCCESS);
  sleep(5);
  printf("\n Observe the RTM port addition Callback \n");
  printf("\t Now Remove the RTM PORT and PRESS any KEY to EXIT\n");
  fflush(stdout);
  getchar();
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");
}
#endif
#if 1
void tet_interface_subscribe_all()
{
  printf("\n------------tet_interface_subscribe_all--------------------\n");
  printf("\n\nInterface External Subscription For ALL");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition, \
updation and removal",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));
  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  
  
  ifsv_ifrec_add(IFSV_IFREC_ADD_UPD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);


  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}
#endif
void tet_interface_int_subscribe()
{
  printf("\n--------------tet_interface_int_subscribe--------------\n");
  printf("\n\nInternal Subscription Scope");

  ifsv_subscription(IFSV_SUBSCRIBE_INT_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition \
(scope: int)",NCSCC_RC_SUCCESS);
  
  
  
    
  
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_int_add_subscribe()
{
  printf("\n--------------tet_interface_int_add_subscribe--------------\n");
  printf("\n\nInterface Addition : Internal Subscription Scope");

  ifsv_subscription(IFSV_SUBSCRIBE_INT_ADD_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition \
(scope: int)",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  
  
    


  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_int_upd_subscribe()
{
  printf("\n--------------tet_interface_int_upd_subscribe--------------\n");
  printf("\n\nInterface Update : Internal Subscription");

  ifsv_subscription(IFSV_SUBSCRIBE_INT_UPD_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface updation\
 (scope: int)",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1); 



  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_UPD_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1); 

  
    


  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_int_rmv_subscribe()
{
  printf("\n--------------tet_interface_int_rmv_subscribe--------------\n");
  printf("\n\nInterface Removal : Internal Subscription");

  ifsv_subscription(IFSV_SUBSCRIBE_INT_RMV_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface removal \
(scope: int)",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  



  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);

  
    


  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_int_subscribe_all()
{
  printf("\n--------------tet_interface_int_subscribe_all--------------\n");
  printf("\n\nInterface Internal Subscription for ALL");

  ifsv_subscription(IFSV_SUBSCRIBE_INTERNAL_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition,\
 updation and removal (scope: int)",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  
  
  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_UPD_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1); 

  
    


  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_add_after_unsubscribe()
{
  printf("\n------------tet_interface_add_after_unsubscribe------------\n");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition,\
 updation and removal",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface without subscription",
              NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);

  printf("\n-----------------------END---------------------------------\n");
}


void tet_interface_disable()
{
  printf("\n---------------------tet_interface_disable-----------------\n");
  printf("\n\nInterface addition");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and \
deletion",NCSCC_RC_SUCCESS);

  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);

  
    

  ifsv_ifrec_add(IFSV_IFREC_ADD_INTERFACE_DISABLE);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);

  if(testrsp.info.ifadd_ntfy.if_info.oper_state==2)
    {
      printf("\nAdded interface was disabled");
    }
  else
    {
      printf("\nAdded interface was not disabled");
    }
    
  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");

}

void tet_interface_enable()
{
  printf("\n----------------------tet_interface_enable-----------------\n");
  printf("\n\nInterface addition");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and \
deletion",NCSCC_RC_SUCCESS);

  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);


  ifsv_ifrec_add(IFSV_IFREC_ADD_INTERFACE_DISABLE);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);

  if(testrsp.info.ifadd_ntfy.if_info.oper_state==2)
    {
      printf("\nAdded interface was disabled");
    }
  else
    {
      printf("\nAdded interface was not disabled");
    }

  
    

  ifsv_ifrec_add(IFSV_IFREC_ADD_INTERFACE_ENABLE);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);

  if(testrsp.info.ifadd_ntfy.if_info.oper_state==1)
    {
      printf("\nDisabled interface was enabled");
    }
  else
    {
      printf("\nDisabled interface was not enabled");
    }
    
  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);


  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");
}

void tet_interface_modify()
{
  printf("\n---------------------tet_interface_modify------------------\n");
  printf("\n\nInterface addition");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);

  
    

  ifsv_ifrec_add(IFSV_IFREC_ADD_INTERFACE_MODIFY);
  ifsv_result("ncs_ifsv_svc_req() to modify interface",NCSCC_RC_SUCCESS);
  sleep(1);

  if( (testrsp.info.ifadd_ntfy.if_info.if_speed==1) &&
      (testrsp.info.ifadd_ntfy.if_info.mtu==1) )
    {
      printf("\nInterface was updated");
    }
  else
    {
      printf("\nInterface was not updated");
    }
    

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);


  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");
}

void tet_interface_seq_alloc()
{
  int check;
  printf("\n----------------------tet_interface_seq_alloc--------------\n");
  printf("\n\nInterface addition");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and \
deletion",NCSCC_RC_SUCCESS);

  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);
  check=testrsp.info.ifadd_ntfy.if_index;
  check++;

  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));
   
  
    

  ifsv_ifrec_add(IFSV_IFREC_ADD_INTERFACE_SEQ_ALLOC);
  ifsv_result("ncs_ifsv_svc_req() to add interface (seq alloc)",
              NCSCC_RC_SUCCESS);
  sleep(1);

  if(testrsp.info.ifadd_ntfy.if_index==check)
    {
      printf("\nInterface added sequentially");
    }
  else
    {
      printf("\nInterface not added sequentially");
    }

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);

  req_info.i_req_type=NCS_IFSV_SVC_REQ_IFREC_DEL;
 
  req_info.info.i_ifdel.shelf=usr_req_info.i_ifadd.spt_info.shelf;
  req_info.info.i_ifdel.slot=usr_req_info.i_ifadd.spt_info.slot;
  req_info.info.i_ifdel.port=usr_req_info.i_ifadd.spt_info.port;
  req_info.info.i_ifdel.type=usr_req_info.i_ifadd.spt_info.type;
  req_info.info.i_ifdel.subscr_scope=
    usr_req_info.i_ifadd.spt_info.subscr_scope;

  ncs_ifsv_svc_req(&req_info);
  sleep(15); 


  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");
}

void tet_interface_timer_aging()
{
  int check;
  printf("\n--------------------tet_interface_timer_aging--------------\n");
  printf("\n\nInterface addition");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);

  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);
  check=testrsp.info.ifadd_ntfy.if_index;
   

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);
  
  
    

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface (after aging of timer)",
              NCSCC_RC_SUCCESS);
  sleep(1);

  if(testrsp.info.ifadd_ntfy.if_index==check)
    {
      printf("\nInterface added with same ifIndex after the timer expires");
    }
  else
    {
      printf("\nInterface not added with same ifIndex after the timer expires");
    }

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);


  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");
}

void tet_interface_get_ifindex()
{
  printf("\n-----------------tet_interface_get_ifindex-----------------\n");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition, \
updation and removal",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));

  ifsv_ifrec_get(IFSV_IFREC_GET_IFINDEX_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to get interface information with key ifindex",NCSCC_RC_SUCCESS);
  sleep(1);
         
  if(gl_error!=1)
    {
      printf("\nInterface get information was not successful"); 
      tet_printf("\nInterface get information was not successful"); 
      tet_result(TET_FAIL);
    }

  memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
         sizeof(usr_req_info.i_ifadd));

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_ifindex_internal()
{
  printf("\n--------------tet_interface_get_ifindex_internal-----------\n");

  ifsv_subscription(IFSV_SUBSCRIBE_INTERNAL_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition, \
updation and removal",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));

  ifsv_ifrec_get(IFSV_IFREC_GET_IFINDEX_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to get interface information with key \
ifindex (internal)",NCSCC_RC_SUCCESS);
  sleep(1);
         
  if(gl_error!=1)
    {
      printf("\nInterface get information was not successful"); 
      tet_printf("\nInterface get information was not successful"); 
      tet_result(TET_FAIL);
    }

  memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
         sizeof(usr_req_info.i_ifadd));

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_spt()
{
  printf("\n--------------------tet_interface_get_spt------------------\n");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition, \
updation and removal",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));

  ifsv_ifrec_get(IFSV_IFREC_GET_SPT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to get interface information with key spt",
              NCSCC_RC_SUCCESS);
  sleep(1);
         
  if(gl_error!=1)
    {
      printf("\nInterface get information was not successful"); 
      tet_printf("\nInterface get information was not successful"); 
      tet_result(TET_FAIL);
    }

  memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
         sizeof(usr_req_info.i_ifadd));

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_spt_internal()
{
  printf("\n---------------tet_interface_get_spt_internal--------------\n");

  ifsv_subscription(IFSV_SUBSCRIBE_INTERNAL_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition, \
updation and removal",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));

  ifsv_ifrec_get(IFSV_IFREC_GET_SPT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to get interface information with key spt\
 (internal)",NCSCC_RC_SUCCESS);
  sleep(1);
         
  if(gl_error!=1)
    {
      printf("\nInterface get information was not successful"); 
      tet_printf("\nInterface get information was not successful"); 
      tet_result(TET_FAIL);
    }

  memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
         sizeof(usr_req_info.i_ifadd));

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_ifindex_sync()
{
  printf("\n----------------tet_interface_get_ifindex_sync-------------\n");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition, \
updation and removal",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));

  ifsv_ifrec_get(IFSV_IFREC_GET_SYNC_IFINDEX_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to get (sync) interface information with \
key ifindex",NCSCC_RC_SUCCESS);
  sleep(1);


         
  if(gl_error!=1)
    {
      printf("\nInterface get information was not successful"); 
      tet_printf("\nInterface get information was not successful"); 
      tet_result(TET_FAIL);
    }

  memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
         sizeof(usr_req_info.i_ifadd));

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_ifindex_internal_sync()
{
  printf("\n-----------tet_interface_get_ifindex_internal_sync---------\n");

  ifsv_subscription(IFSV_SUBSCRIBE_INTERNAL_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition, \
updation and removal",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));

  ifsv_ifrec_get(IFSV_IFREC_GET_SYNC_IFINDEX_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to get (sync) interface information with \
key ifindex (internal)",NCSCC_RC_SUCCESS);
  sleep(1);


         
  if(gl_error!=1)
    {
      printf("\nInterface get information was not successful"); 
      tet_printf("\nInterface get information was not successful"); 
      tet_result(TET_FAIL);
    }

  memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
         sizeof(usr_req_info.i_ifadd));

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_spt_sync()
{
  printf("\n--------------tet_interface_get_spt_sync-------------------\n");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition,\
 updation and removal",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));

  ifsv_ifrec_get(IFSV_IFREC_GET_SYNC_SPT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to get (sync) interface information with \
key spt",NCSCC_RC_SUCCESS);
  sleep(1);


         
  if(gl_error!=1)
    {
      printf("\nInterface get information was not successful"); 
      tet_printf("\nInterface get information was not successful"); 
      tet_result(TET_FAIL);
    }

  memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
         sizeof(usr_req_info.i_ifadd));

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_spt_internal_sync()
{
  printf("\n--------------tet_interface_get_spt_internal_sync----------\n");

  ifsv_subscription(IFSV_SUBSCRIBE_INTERNAL_SUCCESS); 
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition, \
updation and removal",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);  

  
    


  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));

  ifsv_ifrec_get(IFSV_IFREC_GET_SYNC_SPT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to get (sync) interface information with \
key spt (internal)",NCSCC_RC_SUCCESS);
  sleep(1);

  
    

         
  if(gl_error!=1)
    {
      printf("\nInterface get information was not successful"); 
      tet_printf("\nInterface get information was not successful"); 
      tet_result(TET_FAIL);
    }

  memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
         sizeof(usr_req_info.i_ifadd));

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_ifIndex()
{
  printf("\n---------------------tet_interface_get_ifIndex-------------\n");
  printf("\n\nInterface get information using ifindex info");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and \
deletion",NCSCC_RC_SUCCESS);

  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));
  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));

  ifsv_ifrec_get(IFSV_IFREC_GET_SYNC_IFINDEX_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to get interface information with key \
ifindex",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
         sizeof(usr_req_info.i_ifadd));
  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_SPT()
{
  printf("\n------------------------tet_interface_get_SPT--------------\n");
  printf("\n\nInterface get information using SPT info");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and \
deletion",NCSCC_RC_SUCCESS);

  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);


         
  memcpy(&usr_req_info.i_ifadd,&req_info.info.i_ifadd,
         sizeof(req_info.info.i_ifadd));

  ifsv_ifrec_get(IFSV_IFREC_GET_SPT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to get interface information with key spt",
              NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_ifadd,&usr_req_info.i_ifadd,
         sizeof(usr_req_info.i_ifadd));
  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_add()
{
  printf("\n------------------------tet_interface_add------------------\n");
  printf("\n\nInterface addition");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and \
deletion",NCSCC_RC_SUCCESS);

  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);


    
  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);

  
    


  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_add_internal()
{
  printf("\n-------------------tet_interface_add_internal--------------\n");
  printf("\n\nInterface addition");

  ifsv_subscription(IFSV_SUBSCRIBE_INTERNAL_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and \
deletion",NCSCC_RC_SUCCESS);

  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_INT_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);

  
    

    
  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_update()
{
  printf("\n---------------------tet_interface_update------------------\n");
  printf("\n\nInterface addition");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and \
deletion",NCSCC_RC_SUCCESS);

  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
  sleep(1);



  ifsv_ifrec_add(IFSV_IFREC_ADD_UPD_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to update interface",NCSCC_RC_SUCCESS);
  sleep(1);

  
    


  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
  ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
  sleep(1);



  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_add_subscription()
{
  printf("\n--------------------tet_interface_add_subscription---------\n");
  printf("\n\nInterface addition");

  ifsv_subscription(IFSV_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and \
deletion",NCSCC_RC_SUCCESS);



  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");


}
      
/*************************************************************************/
/*************************** WITH EDSV FIRST TEST CASE  ******************/
/*************************************************************************/
void tet_interface_add_subscription_loop()
{
  char *temp;

 /***************Modify by JN********************/
   SaAisErrorT 			rc;
   NCS_SEL_OBJ              evt_ncs_sel_obj;
   NCS_SEL_OBJ_SET          wait_sel_obj;
   SaSelectionObjectT       evt_sel_obj;
/***************Modify end**********************/

  SaEvtEventFilterT end[1]=
    { 
      {3,{3,3,(SaUint8T *)"end"}}
    };
  printf("\n-----------------tet_interface_add_subscription_loop-------\n");
  printf("\n\nInterface addition");
  if(gl_eds && gl_eds_on)
    {
      initialize();
    
      evt_initialize(&gl_evtHandle);
      printf("\nevtHandle: %llu",gl_evtHandle);
      
      gl_channelOpenFlags=SA_EVT_CHANNEL_PUBLISHER|SA_EVT_CHANNEL_CREATE
        |SA_EVT_CHANNEL_SUBSCRIBER;
      evt_channelOpen(gl_evtHandle,&gl_channelHandle);
      printf("\nchannelHandle: %llu",gl_channelHandle);

      evt_eventAllocate(gl_channelHandle,&gl_eventHandle);         
      printf("\neventHandle: %llu",gl_eventHandle);

      gl_filterArray.filters=end;
      gl_subscriptionId=4;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
    }

  ifsv_subscription(IFSV_SUBSCRIBE_ALL);
  ifsv_result("ncs_ifsv_svc_req() to subscribe for interface addition and \
deletion",NCSCC_RC_SUCCESS);
  memcpy(&usr_req_info.i_subr,&req_info.info.i_subr,
         sizeof(req_info.info.i_subr));

  /*In a Loop wait for the Intrf Addition Callbacks*/ 
  if(gl_eds && gl_eds_on)
    {
      gl_dispatchFlags=1;
      
/**************Modify by JN*******************/
/*      while(1)
        {
          evt_dispatch(gl_evtHandle);
          sleep(1);
       
          if(gl_err==1&&gl_subscriptionId==gl_hide)
            {
              eventData=(void *)malloc(3); 
              gl_eventDataSize=3;
              evt_eventDataGet(gl_eventDeliverHandle);
              temp=(char *)eventData; 
            
              if(memcmp(temp,"end",gl_eventDataSize)==0)    
                {
                  free(eventData);
                  break;
                }
              free(eventData);
            }   
        }
        */

   if (SA_AIS_OK != (rc = saEvtSelectionObjectGet(gl_evtHandle, &evt_sel_obj)))
   {
      m_NCS_CONS_PRINTF("\n EDSv: EDA: SaEvtSelectionObjectGet() failed. rc=%d \n",rc);
      return;
   }

   m_NCS_CONS_PRINTF("\n EDSv: EDA: Obtained Selection Object Successfully !!! \n");
   /* Reset the wait select objects */
   m_NCS_SEL_OBJ_ZERO(&wait_sel_obj);

   /* derive the fd for EVT selection object */
   m_SET_FD_IN_SEL_OBJ((uns32)evt_sel_obj, evt_ncs_sel_obj);

   /* Set the EVT select object on which EDSv toolkit application waits */
   m_NCS_SEL_OBJ_SET(evt_ncs_sel_obj, &wait_sel_obj);

  
   while (m_NCS_SEL_OBJ_SELECT(evt_ncs_sel_obj,&wait_sel_obj,NULL,NULL,NULL) != -1)
   {
      /* Process EDSv evt messages */
      if (m_NCS_SEL_OBJ_ISSET(evt_ncs_sel_obj, &wait_sel_obj))
      {
         /* Dispatch all pending messages */
         printf ("\n EDSv: EDA: Dispatching message received on demo channel\n");
         
         /*######################################################################
                        Demonstrating the usage of saEvtDispatch()
         ######################################################################*/
         evt_dispatch(gl_evtHandle);      
         /* Rcvd the published event, now escape */
         if(gl_err==1&&gl_subscriptionId==gl_hide)
         {
              eventData=(void *)malloc(3); 
              gl_eventDataSize=3;
              evt_eventDataGet(gl_eventDeliverHandle);
              temp=(char *)eventData; 
            
              if(memcmp(temp,"end",gl_eventDataSize)==0)    
                {
                  free(eventData);
                  break;
                }
              free(eventData);
          }   
      }

      /* Again set EVT select object to wait for another callback */
      m_NCS_SEL_OBJ_SET(evt_ncs_sel_obj, &wait_sel_obj);
   }

/**************Modify end****************/      
  
      evt_eventFree(gl_eventHandle);
         
      evt_channelClose(gl_channelHandle);

      evt_channelUnlink(gl_evtHandle);

      evt_finalize(gl_evtHandle);
    }
  memcpy(&req_info.info.i_subr,&usr_req_info.i_subr,
         sizeof(usr_req_info.i_subr));
  ifsv_unsubscription(IFSV_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ifsv_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");
}

void tet_interface_addip_ipxs_loop()
{
  printf("\n----------------tet_interface_addip_ipxs_loop--------------\n");
  printf("\nAssociating IP address with the interface using IP extended \
services");

  ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS);
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1);

  ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
  ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and IP \
addr type IPV4",NCSCC_RC_SUCCESS);
  sleep(1);
#if 0
  getchar();   
#endif

  
    

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
  sleep(1);

  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");
}


/**************************************************************************/
/***************************** UTILITY FUNCTIONS **************************/
/**************************************************************************/

void ifsv_result(char *saf_msg,SaAisErrorT ref)
{
  printf("\n\n%s",saf_msg);
  tet_printf("%s",saf_msg);

  if(gl_rc==ref)
    {
      printf("\nSuccess : %d",gl_rc);
      tet_printf("Success : %d",gl_rc);
      tet_result(TET_PASS);
      if(gl_rc==NCSCC_RC_SUCCESS)
        {
        }
    }
  else
    {
      printf("\nFailure : %d",gl_rc);
      tet_printf("Failure : %d",gl_rc);
      tet_result(TET_FAIL);
    }
  if(gl_rc==9)
    {
      getchar();
    }
}


void gl_ifsv_defs()
{
  char *temp=NULL;

  printf("\n\n********************\n");
  printf("\nGlobal structure variables\n");

  temp=(char *)getenv("TET_CASE");
  if(temp)
    {
      gl_tCase=atoi(temp);
    }
  else
    {
      gl_tCase=-1;
    }
  printf("\nTest Case Number: %d",gl_tCase);

  temp=(char *)getenv("TET_ITERATION");
  if(temp)
    {
      gl_iteration=atoi(temp);
    }
  else
    {
      gl_iteration=1;
    }
  printf("\nNumber of iterations: %d",gl_iteration);

  temp=(char *)getenv("TET_LIST_NUMBER");
  if(temp&&(atoi(temp)>0&&(atoi(temp)<4)))
    {
      gl_listNumber=atoi(temp);
    }
  else
    {
      gl_listNumber=-1;
    }
  printf("\nTest List to be executed: %d",gl_listNumber);

  temp=(char *)getenv("RED_FLAG");

  temp=(char *)getenv("EDS_ON");
  if(temp)
    {
      gl_eds_on=atoi(temp);
      printf("\nEDS Enabled ? %d",gl_eds_on);
    }
  else
    gl_eds_on=0;

  temp=(char *)getenv("MASTER_IFINDEX");
  if(temp)
    {
      b_iface.mifindex =atoi(temp);
      printf("\nMaster ifIndex = %d",b_iface.mifindex);
    }
  else
    b_iface.mifindex=0;
  temp=(char *)getenv("SLAVE_IFINDEX");
  if(temp)
    {
      b_iface.sifindex =atoi(temp);
      printf("\nSlave ifIndex =  %d",b_iface.sifindex);
    }
  else
    b_iface.sifindex=0;
  printf("\n\n********************\n");
}



