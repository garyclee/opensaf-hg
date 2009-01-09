#include "tet_api.h"
#include "tet_startup.h"
#include "tet_ifa.h"
#include "tet_eda.h"
/*************************************************************************/
/****************************DRIVER API TEST CASES***********************/
/*************************************************************************/
void tet_ncs_ifsv_drv_svc_init_req(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n--------- tet_ncs_ifsv_drv_svc_init_req : %d --------\n",iOption);
  switch(iOption)
    {
    case 1:

      ifsv_driver_init(IFSV_DRIVER_INIT_NULL_PARAM);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize  with NULL request \
type",NCSCC_RC_FAILURE);
      break;

    case 2:

      ifsv_driver_init(IFSV_DRIVER_INIT_INVALID_TYPE);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize with invalid request\
 type",NCSCC_RC_FAILURE);
      break;

    case 3:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);
      break;
    }
  printf("\n---------------------- END : %d ----------------------\n",iOption);
}

void tet_ncs_ifsv_drv_svc_destroy_req(int iOption)
{
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n--------- tet_ncs_ifsv_drv_svc_destroy_req : %d ------\n",iOption);
  switch(iOption)
    {
    case 1:

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_NULL_PARAM);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy with NULL request type",
                  NCSCC_RC_FAILURE);
      break;

    case 2:

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_INVALID_TYPE);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy with invalid request\
 type",NCSCC_RC_FAILURE);
      break;

    case 3:

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;
    }
  printf("\n---------------------- END : %d ----------------------\n",iOption);
}

void tet_ncs_ifsv_drv_svc_port_reg(int iOption)
{
  NCS_IFSV_SVC_RSP *rsp=0;
  SaEvtEventFilterT port_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"add"}}
    };
  SaEvtEventPatternT end[1]=
    {
      {3,3,(SaUint8T *)"end"}
    };
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ncs_ifsv_drv_svc_port_reg : %d --------\n",iOption);
  if(gl_eds)
    {
      initialize();
      evt_initialize(&gl_evtHandle);
      printf("\nevtHandle: %llu",gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_SUBSCRIBER|SA_EVT_CHANNEL_PUBLISHER;
      evt_channelOpen(gl_evtHandle,&gl_channelHandle);
      printf("\nchannelHandle: %llu",gl_channelHandle);

      gl_filterArray.filters=port_reg;
      gl_subscriptionId=3;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
    }

  switch(iOption)
    {
    case 1:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_NULL_PARAM);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with NULL param",
                  NCSCC_RC_FAILURE);

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 2:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_NULL);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with NULL if_am",
                  NCSCC_RC_FAILURE);

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 3:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_INVALID);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with invalid if_am",
                  NCSCC_RC_FAILURE);

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 4:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_MTU);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with IAM MTU",
                  NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;

              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      printf("\n\nInterface added using port registration");
                      gl_error=1;
                      tet_result(TET_PASS);
                    }
                  free(eventData);
                }
            }
        }
  
      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);

      break;

    case 5:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_IFSPEED);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with IAM IFSPEED",
                  NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      printf("\n\nInterface added using port registration");
                      gl_error=1;
                      tet_result(TET_PASS);
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 6:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_PHYADDR);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with IAM PHYADDR",
                  NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      printf("\n\nInterface added using port registration");
                      gl_error=1;
                      tet_result(TET_PASS);
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 7:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_ADMSTATE);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with IAM ADMSTATE",
                  NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      printf("\n\nInterface added using port registration");
                      gl_error=1;
                      tet_result(TET_PASS);
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 8:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_OPRSTATE);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with IAM OPRSTATE",
                  NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      printf("\n\nInterface added using port registration");
                      gl_error=1;
                      tet_result(TET_PASS);
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 9:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_NAME);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with IAM NAME",
                  NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      printf("\n\nInterface added using port registration");
                      gl_error=1;
                      tet_result(TET_PASS);
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 10:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_DESCR);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with IAM DESCR",
                  NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      printf("\n\nInterface added using port registration");
                      gl_error=1;
                      tet_result(TET_PASS);
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 11:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_LAST_CHNG);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with IAM LAST_CHNG",
                  NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      printf("\n\nInterface added using port registration");
                      gl_error=1;
                      tet_result(TET_PASS);
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 12:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_IAM_SVDEST);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with IAM SVDEST",
                  NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      printf("\n\nInterface added using port registration");
                      gl_error=1;
                      tet_result(TET_PASS);
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 13:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_PORT_TYPE_NULL);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with port type\
 NULL",NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      if(drv_info.info.port_reg->port_info.port_type.type==
                         rsp->info.ifadd_ntfy.spt_info.type)
                        {
                          printf("\n\nInterface added using portregistration");
                          gl_error=1;
                          tet_result(TET_PASS);
                        }
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 14:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_PORT_TYPE_INVALID);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with port type\
 invalid",NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      if(drv_info.info.port_reg->port_info.port_type.type==
                         rsp->info.ifadd_ntfy.spt_info.type)
                        {
                          printf("\n\nInterface added using portregistration");
                          gl_error=1;
                          tet_result(TET_PASS);
                        }
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);

      break;

    case 15:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_PORT_TYPE_INTF_OTHER);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with port type\
 INTF_OTHER",NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      if(drv_info.info.port_reg->port_info.port_type.type==
                         rsp->info.ifadd_ntfy.spt_info.type)
                        {
                          printf("\n\nInterface added using portregistration");
                          gl_error=1;
                          tet_result(TET_PASS);
                        }
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 16:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_PORT_TYPE_LOOPBACK);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with port type\
 LOOPBACK",NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      if(drv_info.info.port_reg->port_info.port_type.type==
                         rsp->info.ifadd_ntfy.spt_info.type)
                        {
                          printf("\n\nInterface added using portregistration");
                          gl_error=1;
                          tet_result(TET_PASS);
                        }
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 17:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_PORT_TYPE_ETHERNET);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with port type\
 ETHERNET",NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      if(drv_info.info.port_reg->port_info.port_type.type==
                         rsp->info.ifadd_ntfy.spt_info.type)
                        {
                          printf("\n\nInterface added using portregistration");
                          gl_error=1;
                          tet_result(TET_PASS);
                        }
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 18:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_PORT_TYPE_TUNNEL);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port with port type\
 TUNNEL",NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;
               
              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      if(drv_info.info.port_reg->port_info.port_type.type==
                         rsp->info.ifadd_ntfy.spt_info.type)
                        {
                          printf("\n\nInterface added using portregistration");
                          gl_error=1;
                          tet_result(TET_PASS);
                        }
                    }
                  free(eventData);
                }
            }
        }

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;

    case 19:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);

      if(gl_eds)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;

              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      if(drv_info.info.port_reg->port_info.port_type.type==
                         rsp->info.ifadd_ntfy.spt_info.type)
                        {
                          if(drv_info.info.port_reg->port_info.oper_state==
                             rsp->info.ifadd_ntfy.if_info.oper_state)
                            {
                              if(drv_info.info.port_reg->port_info.admin_state==rsp->info.ifadd_ntfy.if_info.admin_state)
                                {
                                  if(drv_info.info.port_reg->port_info.mtu==
                                     rsp->info.ifadd_ntfy.if_info.mtu)
                                    {
                                      if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                                sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                        {
                                          if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                            {
                                              printf("\n\nInterface added using port registration");
                                              gl_error=1;
                                              tet_result(TET_PASS);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
              free(eventData);
            }
        }


      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
      break;
    }
  if(gl_eds)
    {
      if(gl_listNumber!=-1&&iOption==19)
        {
          evt_eventAllocate(gl_channelHandle,&gl_eventHandle);
          printf("\neventHandle: %llu",gl_eventHandle);
   
          gl_patternArray.patterns=end; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData="end";
          gl_eventDataSize=3;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
          printf("\nevtId: %llu",gl_evtId);

          evt_eventFree(gl_eventHandle);
        }
   
      evt_channelClose(gl_channelHandle);
 
      evt_finalize(gl_evtHandle);
      if(gl_error!=1&&iOption>3)
        {
          printf("\n\nPort registration failed");
          tet_result(TET_FAIL);
        }
    }
  printf("\n---------------------- END : %d ----------------------\n",iOption);
}

void tet_ncs_ifsv_drv_svc_send_req(int iOption)
{
  NCS_IFSV_SVC_RSP *rsp=0;
  SaEvtEventFilterT port_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"add"}}
    };
  SaEvtEventPatternT end[1]=
    {
      {3,3,(SaUint8T *)"end"}
    };
  if(iOption==1)
    printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ncs_ifsv_drv_svc_send_req : %d --------\n",iOption);
  if(gl_eds && gl_eds_on)
    {
      initialize();
      evt_initialize(&gl_evtHandle);
      printf("\nevtHandle: %llu",gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_SUBSCRIBER|SA_EVT_CHANNEL_PUBLISHER;
      evt_channelOpen(gl_evtHandle,&gl_channelHandle);
      printf("\nchannelHandle: %llu",gl_channelHandle);

      gl_filterArray.filters=port_reg;
      gl_subscriptionId=3;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
    }
  switch(iOption)
    {
    case 1:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);

      if(gl_eds && gl_eds_on)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;

              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      if(drv_info.info.port_reg->port_info.port_type.type==
                         rsp->info.ifadd_ntfy.spt_info.type)
                        {
                          if(drv_info.info.port_reg->port_info.oper_state==
                             rsp->info.ifadd_ntfy.if_info.oper_state)
                            {
                              if(drv_info.info.port_reg->port_info.admin_state==rsp->info.ifadd_ntfy.if_info.admin_state)
                                {
                                  if(drv_info.info.port_reg->port_info.mtu==
                                     rsp->info.ifadd_ntfy.if_info.mtu)
                                    {
                                      if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                                sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                        {
                                          if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                            {
                                              printf("\n\nInterface added using port registration");
                                              gl_error=1;
                                              tet_result(TET_PASS);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
              free(eventData);
            }
        }

      ifsv_driver_send(IFSV_DRIVER_SEND_NULL_PARAM);
      ifsv_result("ncs_ifsv_drv_svc_req() to send interface info with NULL\
 param",NCSCC_RC_SUCCESS);

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
 
      break;

    case 2:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);

      if(gl_eds && gl_eds_on)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;

              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      if(drv_info.info.port_reg->port_info.port_type.type==
                         rsp->info.ifadd_ntfy.spt_info.type)
                        {
                          if(drv_info.info.port_reg->port_info.oper_state==
                             rsp->info.ifadd_ntfy.if_info.oper_state)
                            {
                              if(drv_info.info.port_reg->port_info.admin_state==rsp->info.ifadd_ntfy.if_info.admin_state)
                                {
                                  if(drv_info.info.port_reg->port_info.mtu==
                                     rsp->info.ifadd_ntfy.if_info.mtu)
                                    {
                                      if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                                sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                        {
                                          if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                            {
                                              printf("\n\nInterface added using port registration");
                                              gl_error=1;
                                              tet_result(TET_PASS);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
              free(eventData);
            }
        }
     
      ifsv_driver_send(IFSV_DRIVER_SEND_MSG_TYPE_INVALID);
      ifsv_result("ncs_ifsv_drv_svc_req() to send interface info with invalid\
 msg type",NCSCC_RC_FAILURE);

      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
 
      break;

    case 3:

      ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

      ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);

      if(gl_eds && gl_eds_on)
        {
          sleep(5);

          gl_dispatchFlags=1;
          evt_dispatch(gl_evtHandle);

          if(gl_err==1&&gl_hide==gl_subscriptionId)
            {
              gl_eventDataSize=4096;
              eventData=(void *)malloc(gl_eventDataSize);
              evt_eventDataGet(gl_eventDeliverHandle);
              rsp=(NCS_IFSV_SVC_RSP *)eventData;

              if(drv_info.info.port_reg->port_info.if_am==
                 rsp->info.ifadd_ntfy.if_info.if_am)
                {
                  if(drv_info.info.port_reg->port_info.port_type.port_id==
                     rsp->info.ifadd_ntfy.spt_info.port)
                    {
                      if(drv_info.info.port_reg->port_info.port_type.type==
                         rsp->info.ifadd_ntfy.spt_info.type)
                        {
                          if(drv_info.info.port_reg->port_info.oper_state==
                             rsp->info.ifadd_ntfy.if_info.oper_state)
                            {
                              if(drv_info.info.port_reg->port_info.admin_state==rsp->info.ifadd_ntfy.if_info.admin_state)
                                {
                                  if(drv_info.info.port_reg->port_info.mtu==
                                     rsp->info.ifadd_ntfy.if_info.mtu)
                                    {
                                      if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                                sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                        {
                                          if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                            {
                                              printf("\n\nInterface added using port registration");
                                              gl_error=1;
                                              tet_result(TET_PASS);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
              free(eventData);
            }
        }
#if 1     
      ifsv_driver_send(IFSV_DRIVER_SEND_MSG_TYPE_PORT_REG);
      ifsv_result("ncs_ifsv_drv_svc_req() to send interface info",
                  NCSCC_RC_SUCCESS);
      sleep(2);
#endif
      ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
      ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);
 
      break;
    }
  if(gl_eds && gl_eds_on)
    {
      if(gl_listNumber!=-1&&iOption==3)
        {
          evt_eventAllocate(gl_channelHandle,&gl_eventHandle);
          printf("\neventHandle: %llu",gl_eventHandle);
   
          gl_patternArray.patterns=end; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData="end";
          gl_eventDataSize=3;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
          printf("\nevtId: %llu",gl_evtId);

          evt_eventFree(gl_eventHandle);
        }
 
      evt_channelClose(gl_channelHandle);
 
      evt_finalize(gl_evtHandle);
      if(gl_error!=1)
        {
          printf("\n\nPort registration failed");
          tet_result(TET_FAIL);
        }
    }
  printf("\n---------------------- END : %d ----------------------\n",iOption);
}

/*************************************************************************/
/**********************DRIVER FUNCTIONALITY  TEST CASES*******************/
/*************************************************************************/
void tet_ifsv_driver_add_interface()
{
  NCS_IFSV_SVC_RSP *rsp=0;
  SaEvtEventFilterT port_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"add"}}
    };
  SaEvtEventPatternT end[1]=
    {
      {3,3,(SaUint8T *)"end"}
    };
  printf("\n-----------------------------------------------------------\n");
  printf("\n--------------tet_ifsv_driver_add_interface----------------\n");
  if(gl_eds && gl_eds_on)
    {
      initialize();
      evt_initialize(&gl_evtHandle);
      printf("\nevtHandle: %llu",gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_SUBSCRIBER|SA_EVT_CHANNEL_PUBLISHER;
      evt_channelOpen(gl_evtHandle,&gl_channelHandle);
      printf("\nchannelHandle: %llu",gl_channelHandle);

      gl_filterArray.filters=port_reg;
      gl_subscriptionId=3;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
    }

  ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);
  
  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);

  
    
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 
#if 0
          if(drv_info.info.port_reg->port_info.if_am==
             rsp->info.ifadd_ntfy.if_info.if_am)
            {
#endif
              if(drv_info.info.port_reg->port_info.port_type.port_id==
                 rsp->info.ifadd_ntfy.spt_info.port)
                {
                  if(drv_info.info.port_reg->port_info.port_type.type==
                     rsp->info.ifadd_ntfy.spt_info.type)
                    {
                      if(drv_info.info.port_reg->port_info.oper_state==
                         rsp->info.ifadd_ntfy.if_info.oper_state)
                        {
                          if(drv_info.info.port_reg->port_info.admin_state==
                             rsp->info.ifadd_ntfy.if_info.admin_state)
                            {
                              if(drv_info.info.port_reg->port_info.mtu==
                                 rsp->info.ifadd_ntfy.if_info.mtu)
                                {
                                  if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                            sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                    {
                                      if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                        {
                                          printf("\n\nInterface added using port registration");
                                          gl_error=1;
                                          tet_result(TET_PASS);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
#if 0
            }
#endif
          free(eventData);
        }
    }

  ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);

  if(gl_eds && gl_eds_on)
    {
      if(gl_listNumber!=-1)
        {
          evt_eventAllocate(gl_channelHandle,&gl_eventHandle);
          printf("\neventHandle: %llu",gl_eventHandle);
   
          gl_patternArray.patterns=end; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData="end";
          gl_eventDataSize=3;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
          printf("\nevtId: %llu",gl_evtId);

          evt_eventFree(gl_eventHandle);
        }

      evt_channelClose(gl_channelHandle);
 
      evt_finalize(gl_evtHandle);
      if(gl_error!=1)
        {
          printf("\n\nPort registration failed");
          tet_result(TET_FAIL);
        }
    }
  printf("\n---------------------END-----------------------------------\n");
}

void tet_ifsv_driver_add_multiple_interface()
{
  NCS_IFSV_SVC_RSP *rsp=0;
  SaEvtEventFilterT port_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"add"}}
    };
  SaEvtEventPatternT end[1]=
    {
      {3,3,(SaUint8T *)"end"}
    };
  printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ifsv_driver_add_multiple_interface----------\n");
  if(gl_eds && gl_eds_on)
    {
      initialize();
      evt_initialize(&gl_evtHandle);
      printf("\nevtHandle: %llu",gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_SUBSCRIBER|SA_EVT_CHANNEL_PUBLISHER;
      evt_channelOpen(gl_evtHandle,&gl_channelHandle);
      printf("\nchannelHandle: %llu",gl_channelHandle);

      gl_filterArray.filters=port_reg;
      gl_subscriptionId=3;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
    }

  ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.if_am==
             rsp->info.ifadd_ntfy.if_info.if_am)
            {
              if(drv_info.info.port_reg->port_info.port_type.port_id==
                 rsp->info.ifadd_ntfy.spt_info.port)
                {
                  if(drv_info.info.port_reg->port_info.port_type.type==
                     rsp->info.ifadd_ntfy.spt_info.type)
                    {
                      if(drv_info.info.port_reg->port_info.oper_state==
                         rsp->info.ifadd_ntfy.if_info.oper_state)
                        {
                          if(drv_info.info.port_reg->port_info.admin_state==
                             rsp->info.ifadd_ntfy.if_info.admin_state)
                            {
                              if(drv_info.info.port_reg->port_info.mtu==
                                 rsp->info.ifadd_ntfy.if_info.mtu)
                                {
                                  if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                            sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                    {
                                      if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                        {
                                          printf("\n\nInterface added using port registration");
                                          gl_error=1;
                                          tet_result(TET_PASS);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
          free(eventData);
        }
    }

  
    

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_PORT_TYPE_TUNNEL);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.if_am==
             rsp->info.ifadd_ntfy.if_info.if_am)
            {
              if(drv_info.info.port_reg->port_info.port_type.port_id==
                 rsp->info.ifadd_ntfy.spt_info.port)
                {
                  if(drv_info.info.port_reg->port_info.port_type.type==
                     rsp->info.ifadd_ntfy.spt_info.type)
                    {
                      if(drv_info.info.port_reg->port_info.oper_state==
                         rsp->info.ifadd_ntfy.if_info.oper_state)
                        {
                          if(drv_info.info.port_reg->port_info.admin_state==
                             rsp->info.ifadd_ntfy.if_info.admin_state)
                            {
                              if(drv_info.info.port_reg->port_info.mtu==
                                 rsp->info.ifadd_ntfy.if_info.mtu)
                                {
                                  if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                            sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                    {
                                      if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                        {
                                          printf("\n\nInterface added using port registration");
                                          gl_error+=1;
                                          tet_result(TET_PASS);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
          free(eventData);
        }
    }

  ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);

  if(gl_eds && gl_eds_on)
    {
      if(gl_listNumber!=-1)
        {
          evt_eventAllocate(gl_channelHandle,&gl_eventHandle);
          printf("\neventHandle: %llu",gl_eventHandle);
   
          gl_patternArray.patterns=end; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData="end";
          gl_eventDataSize=3;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
          printf("\nevtId: %llu",gl_evtId);

          evt_eventFree(gl_eventHandle);
        }

      evt_channelClose(gl_channelHandle);
 
      evt_finalize(gl_evtHandle);
      if(gl_error!=2)
        {
          printf("\n\nPort registration failed");
          tet_result(TET_FAIL);
        }
    }
  printf("\n---------------------END-----------------------------------\n");
}

void tet_ifsv_driver_disable_interface()
{
  NCS_IFSV_SVC_RSP *rsp=0;
  SaEvtEventFilterT port_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"add"}}
    };
  SaEvtEventPatternT end[1]=
    {
      {3,3,(SaUint8T *)"end"}
    };
  SaEvtEventFilterT dis_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"dis"}}
    };
  printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ifsv_driver_disable_interface----------\n");
  if(gl_eds && gl_eds_on)
    {
      initialize();
      evt_initialize(&gl_evtHandle);
      printf("\nevtHandle: %llu",gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_SUBSCRIBER|SA_EVT_CHANNEL_PUBLISHER;
      evt_channelOpen(gl_evtHandle,&gl_channelHandle);
      printf("\nchannelHandle: %llu",gl_channelHandle);

      gl_filterArray.filters=port_reg;
      gl_subscriptionId=3;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
    }

  ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.if_am==
             rsp->info.ifadd_ntfy.if_info.if_am)
            {
              if(drv_info.info.port_reg->port_info.port_type.port_id==
                 rsp->info.ifadd_ntfy.spt_info.port)
                {
                  if(drv_info.info.port_reg->port_info.port_type.type==
                     rsp->info.ifadd_ntfy.spt_info.type)
                    {
                      if(drv_info.info.port_reg->port_info.oper_state==
                         rsp->info.ifadd_ntfy.if_info.oper_state)
                        {
                          if(drv_info.info.port_reg->port_info.admin_state==
                             rsp->info.ifadd_ntfy.if_info.admin_state)
                            {
                              if(drv_info.info.port_reg->port_info.mtu==
                                 rsp->info.ifadd_ntfy.if_info.mtu)
                                {
                                  if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                            sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                    {
                                      if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                        {
                                          printf("\n\nInterface added using port registration");
                                          gl_error=1;
                                          tet_result(TET_PASS);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
          free(eventData);
        }
      gl_filterArray.filters=dis_reg;
      gl_subscriptionId=4;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
    }

  
    

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_DISABLE_INTERFACE);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.oper_state==2)
            {
              printf("\n\nInterface disabled using port registration");
              gl_error+=1;
              tet_result(TET_PASS);
            }
          free(eventData);
        }
    }


  ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);

  if(gl_eds && gl_eds_on)
    {
      if(gl_listNumber!=-1)
        {
          evt_eventAllocate(gl_channelHandle,&gl_eventHandle);
          printf("\neventHandle: %llu",gl_eventHandle);
   
          gl_patternArray.patterns=end; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData="end";
          gl_eventDataSize=3;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
          printf("\nevtId: %llu",gl_evtId);

          evt_eventFree(gl_eventHandle);
        }

      evt_channelClose(gl_channelHandle);
 
      evt_finalize(gl_evtHandle);
      if(gl_error!=2)
        {
          printf("\n\nPort registration with disable failed");
          tet_result(TET_FAIL);
        }
    }
  printf("\n---------------------END-----------------------------------\n");
}

void tet_ifsv_driver_enable_interface()
{
  SaEvtSubscriptionIdT tempId;
  NCS_IFSV_SVC_RSP *rsp=0;
  SaEvtEventFilterT port_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"add"}}
    };
  SaEvtEventPatternT end[1]=
    {
      {3,3,(SaUint8T *)"end"}
    };
  SaEvtEventFilterT dis_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"dis"}}
    };
  printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ifsv_driver_enable_interface----------\n");
  if(gl_eds && gl_eds_on)
    {
      initialize();
      evt_initialize(&gl_evtHandle);
      printf("\nevtHandle: %llu",gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_SUBSCRIBER|SA_EVT_CHANNEL_PUBLISHER;
      evt_channelOpen(gl_evtHandle,&gl_channelHandle);
      printf("\nchannelHandle: %llu",gl_channelHandle);

      gl_filterArray.filters=port_reg;
      gl_subscriptionId=3;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
      tempId=gl_subscriptionId;
    }

  ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.if_am==
             rsp->info.ifadd_ntfy.if_info.if_am)
            {
              if(drv_info.info.port_reg->port_info.port_type.port_id==
                 rsp->info.ifadd_ntfy.spt_info.port)
                {
                  if(drv_info.info.port_reg->port_info.port_type.type==
                     rsp->info.ifadd_ntfy.spt_info.type)
                    {
                      if(drv_info.info.port_reg->port_info.oper_state==
                         rsp->info.ifadd_ntfy.if_info.oper_state)
                        {
                          if(drv_info.info.port_reg->port_info.admin_state==
                             rsp->info.ifadd_ntfy.if_info.admin_state)
                            {
                              if(drv_info.info.port_reg->port_info.mtu==
                                 rsp->info.ifadd_ntfy.if_info.mtu)
                                {
                                  if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                            sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                    {
                                      if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                        {
                                          printf("\n\nInterface added using port registration");
                                          gl_error=1;
                                          tet_result(TET_PASS);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
          free(eventData);
        }
      gl_filterArray.filters=dis_reg;
      gl_subscriptionId=4;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
    }

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_DISABLE_INTERFACE);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.oper_state==2)
            {
              printf("\n\nInterface disabled using port registration");
              gl_error+=1;
              tet_result(TET_PASS);
            }
          free(eventData);
        }
    }

  
    

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_ENABLE_INTERFACE);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.oper_state==1)
            {
              printf("\n\nInterface added using port registration");
              gl_error+=1;
              tet_result(TET_PASS);
            }
          free(eventData);
        }
    }


  ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);

  if(gl_eds && gl_eds_on)
    {
      if(gl_listNumber!=-1)
        {
          evt_eventAllocate(gl_channelHandle,&gl_eventHandle);
          printf("\neventHandle: %llu",gl_eventHandle);
   
          gl_patternArray.patterns=end; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData="end";
          gl_eventDataSize=3;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
          printf("\nevtId: %llu",gl_evtId);

          evt_eventFree(gl_eventHandle);
        }

      evt_channelClose(gl_channelHandle);
 
      evt_finalize(gl_evtHandle);
      if(gl_error!=3)
        {
          printf("\n\nPort registration with disable and enable failed");
          tet_result(TET_FAIL);
        }
    }
  printf("\n---------------------END-----------------------------------\n");
}

void tet_ifsv_driver_modify_interface()
{
  SaEvtSubscriptionIdT tempId;
  NCS_IFSV_SVC_RSP *rsp=0;
  SaEvtEventFilterT port_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"add"}}
    };
  SaEvtEventPatternT end[1]=
    {
      {3,3,(SaUint8T *)"end"}
    };
  SaEvtEventFilterT dis_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"dis"}}
    };
  printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ifsv_driver_modify_interface----------\n");
  if(gl_eds && gl_eds_on)
    {
      initialize();
      evt_initialize(&gl_evtHandle);
      printf("\nevtHandle: %llu",gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_SUBSCRIBER|SA_EVT_CHANNEL_PUBLISHER;
      evt_channelOpen(gl_evtHandle,&gl_channelHandle);
      printf("\nchannelHandle: %llu",gl_channelHandle);

      gl_filterArray.filters=port_reg;
      gl_subscriptionId=3;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
      tempId=gl_subscriptionId;
    }

  ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.if_am==
             rsp->info.ifadd_ntfy.if_info.if_am)
            {
              if(drv_info.info.port_reg->port_info.port_type.port_id==
                 rsp->info.ifadd_ntfy.spt_info.port)
                {
                  if(drv_info.info.port_reg->port_info.port_type.type==
                     rsp->info.ifadd_ntfy.spt_info.type)
                    {
                      if(drv_info.info.port_reg->port_info.oper_state==
                         rsp->info.ifadd_ntfy.if_info.oper_state)
                        {
                          if(drv_info.info.port_reg->port_info.admin_state==
                             rsp->info.ifadd_ntfy.if_info.admin_state)
                            {
                              if(drv_info.info.port_reg->port_info.mtu==
                                 rsp->info.ifadd_ntfy.if_info.mtu)
                                {
                                  if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                            sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                    {
                                      if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                        {
                                          printf("\n\nInterface added using port registration");
                                          gl_error=1;
                                          tet_result(TET_PASS);
                                          gl_eventDeliverHandle=0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
          free(eventData);
        }
      gl_filterArray.filters=dis_reg;
      gl_subscriptionId=4;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
    }

  
    

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_MODIFY_INTERFACE);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.port_type.port_id==
             rsp->info.ifadd_ntfy.spt_info.port)
            {
              if(drv_info.info.port_reg->port_info.mtu==
                 rsp->info.ifadd_ntfy.if_info.mtu)
                {
                  if(drv_info.info.port_reg->port_info.speed==
                     rsp->info.ifadd_ntfy.if_info.if_speed)
                    {
                      printf("\n\nInterface updated using port registration");
                      gl_error+=1;
                      tet_result(TET_PASS);
                    }
                }
            }

          free(eventData);
        }
    }


  ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);

  if(gl_eds && gl_eds_on)
    {
      if(gl_listNumber!=-1)
        {
          evt_eventAllocate(gl_channelHandle,&gl_eventHandle);
          printf("\neventHandle: %llu",gl_eventHandle);
   
          gl_patternArray.patterns=end; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData="end";
          gl_eventDataSize=3;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
          printf("\nevtId: %llu",gl_evtId);

          evt_eventFree(gl_eventHandle);
        }

      evt_channelClose(gl_channelHandle);
 
      evt_finalize(gl_evtHandle);
      if(gl_error!=2)
        {
          printf("\n\nInterface update failed through port registration");
          tet_result(TET_FAIL);
        }
    }
  printf("\n---------------------END-----------------------------------\n");
}

void tet_ifsv_driver_seq_alloc()
{
  int check;
  SaEvtSubscriptionIdT tempId;
  NCS_IFSV_SVC_RSP *rsp=0;
  SaEvtEventFilterT port_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"add"}}
    };
  SaEvtEventPatternT end[1]=
    {
      {3,3,(SaUint8T *)"end"}
    };
  printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ifsv_driver_seq_alloc----------\n");
  if(gl_eds && gl_eds_on)
    {
      initialize();
      evt_initialize(&gl_evtHandle);
      printf("\nevtHandle: %llu",gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_SUBSCRIBER|SA_EVT_CHANNEL_PUBLISHER;
      evt_channelOpen(gl_evtHandle,&gl_channelHandle);
      printf("\nchannelHandle: %llu",gl_channelHandle);

      gl_filterArray.filters=port_reg;
      gl_subscriptionId=3;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
      tempId=gl_subscriptionId;
    }

  ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.if_am==
             rsp->info.ifadd_ntfy.if_info.if_am)
            {
              if(drv_info.info.port_reg->port_info.port_type.port_id==
                 rsp->info.ifadd_ntfy.spt_info.port)
                {
                  if(drv_info.info.port_reg->port_info.port_type.type==
                     rsp->info.ifadd_ntfy.spt_info.type)
                    {
                      if(drv_info.info.port_reg->port_info.oper_state==
                         rsp->info.ifadd_ntfy.if_info.oper_state)
                        {
                          if(drv_info.info.port_reg->port_info.admin_state==
                             rsp->info.ifadd_ntfy.if_info.admin_state)
                            {
                              if(drv_info.info.port_reg->port_info.mtu==
                                 rsp->info.ifadd_ntfy.if_info.mtu)
                                {
                                  if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                            sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                    {
                                      if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                        {
                                          printf("\n\nInterface added using port registration");
                                          gl_error=1;
                                          tet_result(TET_PASS);
                                          gl_eventDeliverHandle=0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
          check=rsp->info.ifadd_ntfy.if_index;
          check++;
          free(eventData);
        }
    }

  
    

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_INTERFACE_SEQ_ALLOC);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.if_am==
             rsp->info.ifadd_ntfy.if_info.if_am)
            {
              if(drv_info.info.port_reg->port_info.port_type.port_id==
                 rsp->info.ifadd_ntfy.spt_info.port)
                {
                  if(drv_info.info.port_reg->port_info.port_type.type==
                     rsp->info.ifadd_ntfy.spt_info.type)
                    {
                      if(drv_info.info.port_reg->port_info.oper_state==
                         rsp->info.ifadd_ntfy.if_info.oper_state)
                        {
                          if(drv_info.info.port_reg->port_info.admin_state==
                             rsp->info.ifadd_ntfy.if_info.admin_state)
                            {
                              if(drv_info.info.port_reg->port_info.mtu==
                                 rsp->info.ifadd_ntfy.if_info.mtu)
                                {
                                  if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                            sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                    {
                                      if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                        {
                                          printf("\n\nInterface added using port registration");
                                          gl_error=1;
                                          tet_result(TET_PASS);
                                          gl_eventDeliverHandle=0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
#if 0
      if(rsp->info.ifadd_ntfy.if_index==check)
        {
          printf("\nInterface added seq, using port registration");
        }
      else
        {
          printf("\nInterface not added seq, using port registration");
        }
#endif
      free(eventData);
    }

  ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);

  if(gl_eds && gl_eds_on)
    {
      if(gl_listNumber!=-1)
        {
          evt_eventAllocate(gl_channelHandle,&gl_eventHandle);
          printf("\neventHandle: %llu",gl_eventHandle);
   
          gl_patternArray.patterns=end; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData="end";
          gl_eventDataSize=3;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
          printf("\nevtId: %llu",gl_evtId);

          evt_eventFree(gl_eventHandle);
        }

      evt_channelClose(gl_channelHandle);
 
      evt_finalize(gl_evtHandle);
    }
  printf("\n---------------------END-----------------------------------\n");
}

void tet_ifsv_driver_timer_aging()
{
  int check;
  SaEvtSubscriptionIdT tempId;
  NCS_IFSV_SVC_RSP *rsp=0;
  SaEvtEventFilterT port_reg[1]=
    {
      {3,{3,3,(SaUint8T *)"add"}}
    };
  SaEvtEventPatternT end[1]=
    {
      {3,3,(SaUint8T *)"end"}
    };
  printf("\n-----------------------------------------------------------\n");
  printf("\n-----------tet_ifsv_driver_timer_aging----------\n");
  if(gl_eds && gl_eds_on)
    {
      initialize();
      evt_initialize(&gl_evtHandle);
      printf("\nevtHandle: %llu",gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_SUBSCRIBER|SA_EVT_CHANNEL_PUBLISHER;
      evt_channelOpen(gl_evtHandle,&gl_channelHandle);
      printf("\nchannelHandle: %llu",gl_channelHandle);

      gl_filterArray.filters=port_reg;
      gl_subscriptionId=3;
      evt_eventSubscribe(gl_channelHandle,gl_subscriptionId);
      tempId=gl_subscriptionId;
    }

  ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.if_am==
             rsp->info.ifadd_ntfy.if_info.if_am)
            {
              if(drv_info.info.port_reg->port_info.port_type.port_id==
                 rsp->info.ifadd_ntfy.spt_info.port)
                {
                  if(drv_info.info.port_reg->port_info.port_type.type==
                     rsp->info.ifadd_ntfy.spt_info.type)
                    {
                      if(drv_info.info.port_reg->port_info.oper_state==
                         rsp->info.ifadd_ntfy.if_info.oper_state)
                        {
                          if(drv_info.info.port_reg->port_info.admin_state==
                             rsp->info.ifadd_ntfy.if_info.admin_state)
                            {
                              if(drv_info.info.port_reg->port_info.mtu==
                                 rsp->info.ifadd_ntfy.if_info.mtu)
                                {
                                  if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                            sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                    {
                                      if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                        {
                                          printf("\n\nInterface added using port registration");
                                          gl_error=1;
                                          tet_result(TET_PASS);
                                          gl_eventDeliverHandle=0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
          check=rsp->info.ifadd_ntfy.if_index;
          free(eventData);
        }
    }

  ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);

  
    

  ifsv_driver_init(IFSV_DRIVER_INIT_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to initialize",NCSCC_RC_SUCCESS);

  ifsv_driver_port_reg(IFSV_DRIVER_PORT_REG_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to register port",NCSCC_RC_SUCCESS);
  
  if(gl_eds && gl_eds_on)
    {
      sleep(5);
 
      gl_dispatchFlags=1;
      evt_dispatch(gl_evtHandle);

      if(gl_err==1&&gl_hide==gl_subscriptionId)
        {
          gl_eventDataSize=4096;
          eventData=(void *)malloc(gl_eventDataSize);
          evt_eventDataGet(gl_eventDeliverHandle);
          rsp=(NCS_IFSV_SVC_RSP *)eventData; 

          if(drv_info.info.port_reg->port_info.if_am==
             rsp->info.ifadd_ntfy.if_info.if_am)
            {
              if(drv_info.info.port_reg->port_info.port_type.port_id==
                 rsp->info.ifadd_ntfy.spt_info.port)
                {
                  if(drv_info.info.port_reg->port_info.port_type.type==
                     rsp->info.ifadd_ntfy.spt_info.type)
                    {
                      if(drv_info.info.port_reg->port_info.oper_state==
                         rsp->info.ifadd_ntfy.if_info.oper_state)
                        {
                          if(drv_info.info.port_reg->port_info.admin_state==
                             rsp->info.ifadd_ntfy.if_info.admin_state)
                            {
                              if(drv_info.info.port_reg->port_info.mtu==
                                 rsp->info.ifadd_ntfy.if_info.mtu)
                                {
                                  if(memcmp(drv_info.info.port_reg->port_info.if_name,rsp->info.ifadd_ntfy.if_info.if_name,
                                            sizeof(rsp->info.ifadd_ntfy.if_info.if_name))==0)
                                    {
                                      if(drv_info.info.port_reg->port_info.speed==rsp->info.ifadd_ntfy.if_info.if_speed)
                                        {
                                          printf("\n\nInterface added using port registration");
                                          gl_error=1;
                                          tet_result(TET_PASS);
                                          gl_eventDeliverHandle=0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
      /*   
           if(rsp->info.ifadd_ntfy.if_index==check)
           {
           printf("\nInterface added with same ifIndex, using port registration");
           }
           else
           {
           printf("\nInterface not added with same ifIndex, using port registration");
           }
      */
      free(eventData);
    }

  ifsv_driver_destroy(IFSV_DRIVER_DESTROY_SUCCESS);
  ifsv_result("ncs_ifsv_drv_svc_req() to destroy",NCSCC_RC_SUCCESS);

  if(gl_eds && gl_eds_on)
    {
#if 0
      if(gl_listNumber!=-1)
#endif
        {
          evt_eventAllocate(gl_channelHandle,&gl_eventHandle);
          printf("\neventHandle: %llu",gl_eventHandle);
   
          gl_patternArray.patterns=end; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData="end";
          gl_eventDataSize=3;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
          printf("\nevtId: %llu",gl_evtId);

          evt_eventFree(gl_eventHandle);
        }

      evt_channelClose(gl_channelHandle);
 
      evt_finalize(gl_evtHandle);
    }
  printf("\n---------------------END-----------------------------------\n");
}
