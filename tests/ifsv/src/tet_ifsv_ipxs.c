#include "tet_api.h"
#include "tet_startup.h"
#include "tet_ifa.h"
#include "tet_eda.h"

extern int gl_sync_pointnum;
extern int fill_syncparameters(int);

SaAisErrorT rs;
/*************************************************************************/
/**********************IPXS API TEST CASES********************************/
/*************************************************************************/
void tet_ncs_ipxs_svc_req_subscribe(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ncs_ipxs_svc_req_subscribe : %d --------\n",
         iOption);
  switch(iOption)
    {
    case 1:
  
      ipxs_subscription(IPXS_SUBSCRIBE_INVALID_REQUEST_TYPE);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NULL request type",
                  NCSCC_RC_FAILURE);
      break;

    case 2:

      ipxs_subscription(IPXS_SUBSCRIBE_NULL_CALLBACKS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NULL callbacks",
                  NCSCC_RC_FAILURE);
      break;

    case 3:

      ipxs_subscription(IPXS_SUBSCRIBE_NULL_SUBSCRIPTION_ATTRIBUTE_BITMAP);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NULL subscription\
 attribute bitmap",NCSCC_RC_FAILURE);
      break;

    case 4:

      ipxs_subscription(IPXS_SUBSCRIBE_INVALID_SUBSCRIPTION_ATTRIBUTE_BITMAP);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with invalid subscription\
 attribute bitmap",NCSCC_RC_FAILURE);
      break;

    case 5:

      ipxs_subscription(IPXS_SUBSCRIBE_ADD_IFACE);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_ADD_IFACE\
 (subscription attribute)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 6:

      ipxs_subscription(IPXS_SUBSCRIBE_RMV_IFACE);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_RMV_IFACE\
 (subscription attribute)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 7:

      ipxs_subscription(IPXS_SUBSCRIBE_UPD_IFACE);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_UPD_IFACE\
 (subscription attribute)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 8:

      ipxs_subscription(IPXS_SUBSCRIBE_SCOPE_NODE);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_SCOPE_NODE\
 (subscription attribute)",NCSCC_RC_FAILURE);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 9:

      ipxs_subscription(IPXS_SUBSCRIBE_SCOPE_CLUSTER);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with \
NCS_IPXS_SCOPE_CLUSTER (subscription attribute)",NCSCC_RC_FAILURE);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 10:

      ipxs_subscription(IPXS_SUBSCRIBE_NULL_ATTRIBUTES_MAP);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NULL attributes map",
                  NCSCC_RC_FAILURE);
      break;

    case 11:

      ipxs_subscription(IPXS_SUBSCRIBE_INVALID_ATTRIBUTES_MAP);
      ifsv_result("ncs_ipxs_svc_req() with invalid attributes map",
                  NCSCC_RC_FAILURE);
      break;

    case 12:

      ipxs_subscription(IPXS_SUBSCRIBE_IAM_MTU);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IFSV_IAM_MTU\
 (attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 13:

      ipxs_subscription(IPXS_SUBSCRIBE_IAM_IFSPEED);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IFSV_IAM_IFSPEED\
 (attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 14:

      ipxs_subscription(IPXS_SUBSCRIBE_IAM_PHYADDR);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IFSV_IAM_PHYADDR\
 (attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 15:

      ipxs_subscription(IPXS_SUBSCRIBE_IAM_ADMSTATE);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IFSV_IAM_ADMSTATE\
 (attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 16:

      ipxs_subscription(IPXS_SUBSCRIBE_IAM_OPRSTATE);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IFSV_IAM_OPRSTATE\
 (attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 17:

      ipxs_subscription(IPXS_SUBSCRIBE_IAM_NAME);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IFSV_IAM_NAME\
 (attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 18:

      ipxs_subscription(IPXS_SUBSCRIBE_IAM_DESCR);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IFSV_IAM_DESCR\
 (attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 19:

      ipxs_subscription(IPXS_SUBSCRIBE_IAM_LAST_CHNG);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with\
 NCS_IFSV_IAM_LAST_CHNG (attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 20:

      ipxs_subscription(IPXS_SUBSCRIBE_IAM_SVDEST);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IFSV_IAM_SVDEST\
 (attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 21:

      ipxs_subscription(IPXS_SUBSCRIBE_NULL_IP_ATTRIBUTES_MAP);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NULL ip attributes\
 map",NCSCC_RC_FAILURE);
      break;

    case 22:

      ipxs_subscription(IPXS_SUBSCRIBE_INVALID_IP_ATTRIBUTES_MAP);
      ifsv_result("ncs_ipxs_svc_req() with invalid ip attributes map",
                  NCSCC_RC_FAILURE);
      break;

    case 23:

      ipxs_subscription(IPXS_SUBSCRIBE_IPAM_ADDR);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_IPAM_ADDR\
 (ip attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 24:

      ipxs_subscription(IPXS_SUBSCRIBE_IPAM_UNNMBD);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_IPAM_UNNMBD\
 (ip attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 25:

      ipxs_subscription(IPXS_SUBSCRIBE_IPAM_RRTRID);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_IPAM_RRTRID\
 (ip attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 26:

      ipxs_subscription(IPXS_SUBSCRIBE_IPAM_RMTRID);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_IPAM_RMTRID\
 (ip attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 27:

      ipxs_subscription(IPXS_SUBSCRIBE_IPAM_RMT_AS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_IPAM_RMT_AS\
 (ip attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 28:

      ipxs_subscription(IPXS_SUBSCRIBE_IPAM_RMTIFID);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_IPAM_RMTIFID\
 (ip attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 29:

      ipxs_subscription(IPXS_SUBSCRIBE_IPAM_UD_1);
      ifsv_result("ncs_ipxs_svc_req() to subscribe  with NCS_IPXS_IPAM_UD_1\
 (ip attributes map)",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 30:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;
    }
  printf("\n---------------------- END : %d ----------------------\n",iOption);
}

void tet_ncs_ipxs_svc_req_unsubscribe(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ncs_ipxs_svc_req_unsubscribe : %d --------\n",
         iOption);
  switch(iOption)
    {
    case 1:
         
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_NULL_HANDLE);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe with NULL subscription\
 handle",NCSCC_RC_FAILURE);
      break;

    case 2:
  
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_INVALID_HANDLE);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe with invalid subscription\
 handle",NCSCC_RC_FAILURE);
      break;

    case 3:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_DUPLICATE_HANDLE);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_DUPLICATE_HANDLE);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe with duplicate \
subscription handle",NCSCC_RC_FAILURE);
      break;

    case 4:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 

      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;
    }
  printf("\n---------------------- END : %d ----------------------\n",iOption);
}

void tet_ncs_ipxs_svc_req_ipinfo_get(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ncs_ipxs_svc_req_ipinfo_get : %d --------\n",
         iOption);
  switch(iOption)
    {
    case 1:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_NULL_PARAM);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with NULL\
 parameters",NCSCC_RC_SUCCESS);
      sleep(1);

      if(gl_error!=2)
        {
          printf("\nget was successful for invalid parameters"); 
          tet_printf("get was successful for invalid parameters");
          tet_result(TET_FAIL); 
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 2:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_INVALID_KEY);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with invalid key",
                  NCSCC_RC_FAILURE);
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 3:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_KEY_IFINDEX);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with key ifindex",
                  NCSCC_RC_SUCCESS);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 4:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_INVALID_TYPE);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with invalid type\
 (key spt)",NCSCC_RC_SUCCESS);
      
      if(gl_error!=2)
        {
          printf("\nget was successful for invalid parameters"); 
          tet_printf("get was successful for invalid parameters");
          tet_result(TET_FAIL); 
        }
         
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 5:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_INTF_OTHER);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with type intf\
 other (key spt)",NCSCC_RC_SUCCESS);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 6:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INTF_LOOPBACK_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_INTF_LOOPBACK);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with type intf\
 loopback (key spt)",NCSCC_RC_SUCCESS);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 7:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INTF_ETHERNET_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_INTF_ETHERNET);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with type intf\
 ethernet (key spt)",NCSCC_RC_SUCCESS);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 8:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INTF_TUNNEL_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_INTF_TUNNEL);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with type intf\
 tunnel (key spt)",NCSCC_RC_SUCCESS);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 9:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_INVALID_SUBSCR_SCOPE);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with invalid\
 subscriber scope (key spt)",NCSCC_RC_SUCCESS);

      if(gl_error!=2)
        {
          printf("\nget was successful for invalid parameters"); 
          tet_printf("get was successful for invalid parameters");
          tet_result(TET_FAIL); 
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 10:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_SUBSCR_EXT);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with external\
 subscriber scope (key spt)",NCSCC_RC_SUCCESS);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 11:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_INT_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_SUBSCR_INT);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with internal\
 subscriber scope (key spt)",NCSCC_RC_SUCCESS);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 12:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_INVALID_ADDR_TYPE);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with invalid addr\
 type",NCSCC_RC_FAILURE);
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 13:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_ADDR_TYPE_NONE);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with key ip_addr",
                  NCSCC_RC_FAILURE);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 14:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with addr type\
 IPV4",NCSCC_RC_SUCCESS);

      if(gl_error!=2)
        {
          printf("\nget interface information was successful");
          tet_printf("get interface information was successful");
          tet_result(TET_FAIL);
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 15:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_ADDR_TYPE_IPV6);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with addr type\
 IPV6",NCSCC_RC_FAILURE);
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 16:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_INVALID_RESP_TYPE);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with invalid resp\
 type",NCSCC_RC_FAILURE);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 17:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_SYNC_RESP_TYPE);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with sync resp\
 type",NCSCC_RC_SUCCESS);
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 18:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      ifsv_result("ncs_ifsv_svc_req() to add interface",NCSCC_RC_SUCCESS);
      sleep(1);
         
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_ASYNC_RESP_TYPE);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with async resp\
 type",NCSCC_RC_SUCCESS);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }
         
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);         
      ifsv_result("ncs_ifsv_svc_req() to delete interface",NCSCC_RC_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 19:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);        
      
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_IFINDEX_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with ifindex",
                  NCSCC_RC_SUCCESS);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 20:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));
        
      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);        
  
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_SPT_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to get ip information with spt",
                  NCSCC_RC_SUCCESS);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 21:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));
        
      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);        
  
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_SYNC_IFINDEX_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to get (sync) ip information with\
 ifindex",NCSCC_RC_SUCCESS);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 22:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));
        
      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);        
  
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_SYNC_SPT_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to get (sync) ip information with spt",
                  NCSCC_RC_SUCCESS);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 23:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));
        
      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);        

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      sleep(1);
  
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_IPADDR_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to get if index information with ip\
 address",NCSCC_RC_SUCCESS);
      sleep(1);

      if(gl_error!=1)
        {
          printf("\nget interface information was not successful");
          tet_printf("get interface information was not successful");
          tet_result(TET_FAIL);
        }

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 24:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));
        
      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);        

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      sleep(1);
  
      ipxs_ipinfo_gett(IPXS_IPINFO_GET_SYNC_IPADDR_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to get (sync) if index information with\
 ip address",NCSCC_RC_SUCCESS);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;


    }
  printf("\n---------------------- END : %d ----------------------\n",iOption);
}

void tet_ncs_ipxs_svc_req_ipinfo_set(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ncs_ipxs_svc_req_ipinfo_set : %d --------\n",
         iOption);
  switch(iOption)
    {
    case 1:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_INVALID_IFINDEX);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with invalid\
 ifIndex",NCSCC_RC_FAILURE);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 2:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_NULL_IP_ATTRIBUTE_MAP);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with NULL ip\
 attribute map",NCSCC_RC_FAILURE);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 3:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_INVALID_IP_ATTRIBUTE_MAP);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with invalid ip\
 attribute map",NCSCC_RC_FAILURE);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 4:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IPAM_ADDR);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with attribute\
 map IPAM_ADDR",NCSCC_RC_SUCCESS);
      sleep(1);
 
      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 5:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IPAM_UNNMBD);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with attribute\
 map IPAM_UNNMBD",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 6:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IPAM_RRTRID);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with attribute\
 map IPAM_RRTRID",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 7:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IPAM_RMTRID);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with attribute\
 map IPAM_RMTRID",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 8:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IPAM_RMT_AS);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with attribute\
 map IPAM_RMT_AS",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 9:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IPAM_RMTIFID);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with attribute\
 map IPAM_RMTIFID",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 10:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IPAM_UD_1);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with attribute\
 map IPAM_UD_1",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 11:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IPV4_NUMB_FLAG);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IPV4 numbered\
 flag",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 12:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_INVALID_IP_ADDR_TYPE);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 invalid IP addr type",NCSCC_RC_FAILURE);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 13:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_NONE);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type none",NCSCC_RC_FAILURE);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 14:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));


      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type IPV4",NCSCC_RC_SUCCESS);
      sleep(1);

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;
    }
  printf("\n---------------------- END : %d ----------------------\n",iOption);
}


void tet_ncs_ipxs_svc_req_get_node_rec(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ncs_ipxs_svc_req_get_node_rec : %d --------\n",
         iOption);
  switch(iOption)
    {
    case 1:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type IPV4",NCSCC_RC_SUCCESS);
      sleep(1);

      ipxs_get_node_rec(IPXS_GET_NODE_REC_NULL_PARAM);
      ifsv_result("ncs_ipxs_svc_req() to get node record information with NULL\
 param",NCSCC_RC_FAILURE);         

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 2:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type IPV4",NCSCC_RC_SUCCESS);
      sleep(1);

      ipxs_get_node_rec(IPXS_GET_NODE_REC_INVALID_NDATTR);
      ifsv_result("ncs_ipxs_svc_req() to get node record information with\
 invalid attributes",NCSCC_RC_FAILURE);         

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 3:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type IPV4",NCSCC_RC_SUCCESS);
      sleep(1);

      ipxs_get_node_rec(IPXS_GET_NODE_REC_NDATTR_RTR_ID);
      ifsv_result("ncs_ipxs_svc_req() to get node record information with\
 RTR_ID",NCSCC_RC_SUCCESS);         

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 4:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type IPV4",NCSCC_RC_SUCCESS);
      sleep(1);

      ipxs_get_node_rec(IPXS_GET_NODE_REC_NDATTR_AS_ID);
      ifsv_result("ncs_ipxs_svc_req() to get node record information with\
 AS_ID",NCSCC_RC_SUCCESS);         

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 5:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type IPV4",NCSCC_RC_SUCCESS);
      sleep(1);

      ipxs_get_node_rec(IPXS_GET_NODE_REC_NDATTR_LBIA_ID);
      ifsv_result("ncs_ipxs_svc_req() to get node record information with\
 LBIA_ID",NCSCC_RC_SUCCESS);         

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 6:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type IPV4",NCSCC_RC_SUCCESS);
      sleep(1);

      ipxs_get_node_rec(IPXS_GET_NODE_REC_NDATTR_UD_1);
      ifsv_result("ncs_ipxs_svc_req() to get node record information with\
 UD_1",NCSCC_RC_SUCCESS);         

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 7:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type IPV4",NCSCC_RC_SUCCESS);
      sleep(1);

      ipxs_get_node_rec(IPXS_GET_NODE_REC_NDATTR_UD_2);
      ifsv_result("ncs_ipxs_svc_req() to get node record information with\
 UD_2",NCSCC_RC_SUCCESS);         

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 8:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type IPV4",NCSCC_RC_SUCCESS);
      sleep(1);

      ipxs_get_node_rec(IPXS_GET_NODE_REC_NDATTR_UD_3);
      ifsv_result("ncs_ipxs_svc_req() to get node record information with\
 UD_3",NCSCC_RC_SUCCESS);         

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;

    case 9:

      ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
      memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,
             sizeof(info.info.i_subcr));

      ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
      sleep(1);

      ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
      ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and\
 IP addr type IPV4",NCSCC_RC_SUCCESS);
      sleep(1);

      ipxs_get_node_rec(IPXS_GET_NODE_REC_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to get node record information",
                  NCSCC_RC_SUCCESS);         

      ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
      sleep(1);

      memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
             sizeof(usr_ipxs_info.i_subcr));
      ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
      ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
      break;
    }
  printf("\n---------------------- END : %d ----------------------\n",iOption);
}


/*************************************************************************/
/**********************IPXS FUNCTIONALITY TEST CASES**********************/
/*************************************************************************/
void tet_interface_subscribe_ipxs()
{
  printf("\n-------------tet_interface_subscribe_ipxs------------------\n");
  printf("\n\nipxs : Interface Addition/Deletion : External Subscription");

  ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS); 
  ifsv_result("ncs_ipxs_svc_req() to subscribe for interface addition and\
 deletion",NCSCC_RC_SUCCESS);
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1); 



  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS); 
  sleep(1);

  
    

  
  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_add_subscribe_ipxs()
{
  printf("\n-------------tet_interface_add_subscribe_ipxs--------------\n");
  printf("\n\nipxs : Interface Addition: External Subscription");

  ipxs_subscription(IPXS_SUBSCRIBE_ADD_SUCCESS); 
  ifsv_result("ncs_ipxs_svc_req() to subscribe for interface addition",
              NCSCC_RC_SUCCESS);
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1); 


  
    

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS); 
  sleep(1);


  
  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_upd_subscribe_ipxs()
{
  printf("\n--------------tet_interface_upd_subscribe_ipxs-------------\n");
  printf("\n\nipxs : Interface Update : External Subscription");

  ipxs_subscription(IPXS_SUBSCRIBE_UPD_SUCCESS); 
  ifsv_result("ncs_ipxs_svc_req() to subscribe for interface updation",
              NCSCC_RC_SUCCESS);
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1); 



  ifsv_ifrec_add(IFSV_IFREC_ADD_UPD_SUCCESS);
  sleep(1);

  
    


  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS); 
  sleep(1);


  
  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_rmv_subscribe_ipxs()
{
  printf("\n--------------tet_interface_rmv_subscribe_ipxs-------------\n");
  printf("\n\nipxs : Interface Removal : External Subscription");

  ipxs_subscription(IPXS_SUBSCRIBE_RMV_SUCCESS); 
  ifsv_result("ncs_ipxs_svc_req() to subscribe for interface deletion",
              NCSCC_RC_SUCCESS);
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1); 



  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS); 
  sleep(1);

  
    

  
  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_add_ipxs()
{
  printf("\n-------------------tet_interface_add_ipxs------------------\n");
  printf("\nInterface addition using IP extended services");

  ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS);
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));


  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1);

  
    


  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
  sleep(1);



  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_addip_ipxs()
{
  printf("\n----------------tet_interface_addip_ipxs-------------------\n");
  printf("\nAssociating IP address with the interface using IP extended\
 services");

  ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS);
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1);



  ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
  ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and IP\
 addr type IPV4",NCSCC_RC_SUCCESS);
  sleep(1);

  
    


  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
  sleep(1);



  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);

  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_ifindex_ipxs()
{
  printf("\n---------------tet_interface_get_ifindex_ipxs--------------\n");

  ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1);        



  ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
  ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and IP\
 addr type IPV4",NCSCC_RC_SUCCESS);
  sleep(1);

  
    

      
  ipxs_ipinfo_gett(IPXS_IPINFO_GET_IFINDEX_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to get ip information with ifindex",
              NCSCC_RC_SUCCESS);



  if(gl_error!=1)
    {
      printf("\nget interface information was not successful");
      tet_printf("get interface information was not successful");
      tet_result(TET_FAIL);
    }

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
  sleep(1);



  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_sync_ifindex_ipxs()
{
  printf("\n---------------tet_interface_get_sync_ifindex_ipxs---------\n");

  ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1);        



  ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
  ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and IP\
 addr type IPV4",NCSCC_RC_SUCCESS);
  sleep(1);

  
    

      
  ipxs_ipinfo_gett(IPXS_IPINFO_GET_SYNC_IFINDEX_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to get (sync) ip information with ifindex",
              NCSCC_RC_SUCCESS);

  if(gl_error!=1)
    {
      printf("\nget interface information was not successful");
      tet_printf("get interface information was not successful");
      tet_result(TET_FAIL);
    }

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
  sleep(1);



  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");


}

void tet_interface_get_spt_ipxs()
{
  printf("\n---------------tet_interface_get_spt_ipxs------------------\n");

  ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1);        



  ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
  ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and IP\
 addr type IPV4",NCSCC_RC_SUCCESS);
  sleep(1);

  
    

      
  ipxs_ipinfo_gett(IPXS_IPINFO_GET_SPT_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to get ip information with spt",
              NCSCC_RC_SUCCESS);

  if(gl_error!=1)
    {
      printf("\nget interface information was not successful");
      tet_printf("get interface information was not successful");
      tet_result(TET_FAIL);
    }

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
  sleep(1);



  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");


} 

void tet_interface_get_sync_spt_ipxs()
{
  printf("\n------------------tet_interface_get_sync_spt_ipxs----------\n");

  ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1);        



  ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
  ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and IP\
 addr type IPV4",NCSCC_RC_SUCCESS);
  sleep(1);

  
    

      
  ipxs_ipinfo_gett(IPXS_IPINFO_GET_SYNC_SPT_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to get (sync) ip information with spt",
              NCSCC_RC_SUCCESS);

  if(gl_error!=1)
    {
      printf("\nget interface information was not successful");
      tet_printf("get interface information was not successful");
      tet_result(TET_FAIL);
    }

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
  sleep(1);



  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");

} 

void tet_interface_get_ipaddr_ipxs()
{
  printf("\n----------------tet_interface_get_ipaddr_ipxs--------------\n");

  ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1);        



  ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
  ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and IP\
 addr type IPV4",NCSCC_RC_SUCCESS);
  sleep(1);

  
    

      
  ipxs_ipinfo_gett(IPXS_IPINFO_GET_IPADDR_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to get ip information with ipaddr",
              NCSCC_RC_SUCCESS);

  if(gl_error!=1)
    {
      printf("\nget interface information was not successful");
      tet_printf("get interface information was not successful");
      tet_result(TET_FAIL);
    }

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
  sleep(1);



  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");
} 

void tet_interface_get_sync_ipaddr_ipxs()
{
  printf("\n---------------tet_interface_get_sync_ipaddr_ipxs----------\n");

  ipxs_subscription(IPXS_SUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to subscribe",NCSCC_RC_SUCCESS); 
  memcpy(&usr_ipxs_info.i_subcr,&info.info.i_subcr,sizeof(info.info.i_subcr));

  ifsv_ifrec_add(IFSV_IFREC_ADD_SUCCESS);
  sleep(1);        



  ipxs_ipinfo_sett(IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4);
  ifsv_result("ncs_ipxs_svc_req() to set ip information with IP count and IP\
 addr type IPV4",NCSCC_RC_SUCCESS);
  sleep(1);

  
    

      
  ipxs_ipinfo_gett(IPXS_IPINFO_GET_SYNC_IPADDR_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to get (sync) ip information with ipaddr",
              NCSCC_RC_SUCCESS);

  ifsv_ifrec_del(IFSV_IFREC_DEL_SUCCESS);
  sleep(1);



  memcpy(&info.info.i_subcr,&usr_ipxs_info.i_subcr,
         sizeof(usr_ipxs_info.i_subcr));
  ipxs_unsubscription(IPXS_UNSUBSCRIBE_SUCCESS);
  ifsv_result("ncs_ipxs_svc_req() to unsubscribe",NCSCC_RC_SUCCESS);
  printf("\n-----------------------END---------------------------------\n");


} 
