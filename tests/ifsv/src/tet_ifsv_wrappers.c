#include "tet_api.h"
#include "tet_startup.h"
#include "tet_ifa.h"
#include "tet_eda.h"
/******************************************************/
/****************** EDSV WRAPPERS *********************/
/******************************************************/
SaAisErrorT rs;

void EvtDeliverCallback(SaEvtSubscriptionIdT subscriptionId,
                        SaEvtEventHandleT deliverEventHandle,
                        SaSizeT eventDataSize)
{
  gl_eventDeliverHandle=deliverEventHandle;
  printf("\nevent deliver callback");
  gl_err=1;
  gl_hide=subscriptionId;
}

SaEvtCallbacksT gl_evtCallbacks=
  {
    saEvtEventDeliverCallback:EvtDeliverCallback
  };

void initialize()
{
  gl_version.releaseCode='B';
  gl_version.majorVersion=1;
  gl_version.minorVersion=1;
  gl_evtCallbacks.saEvtEventDeliverCallback=EvtDeliverCallback;
  gl_timeout=100000000000.0;
  gl_publisherName.length=4;
  gl_channelName.length=9;
  memcpy(&gl_channelName.value,"interface",gl_channelName.length);
  memcpy(&gl_publisherName.value,"ifsv",gl_publisherName.length); 
  gl_filterArray.filtersNumber=1;
  gl_patternArray.patternsNumber=1;
  gl_retentionTime=0;
  gl_priority=1;
  gl_err=0;
  gl_error=0;
}

void evt_initialize(SaEvtHandleT *evtHandle)
{
  rs=saEvtInitialize(evtHandle,&gl_evtCallbacks,&gl_version);
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtInitialize() failed: %d",rs);
    }
}

void evt_finalize(SaEvtHandleT evtHandle)
{
  rs=saEvtFinalize(evtHandle);
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtFinalize() failed: %d",rs);
    }
}

void evt_channelOpen(SaEvtHandleT evtHandle,SaEvtChannelHandleT *channelHandle)
{
  rs=saEvtChannelOpen(evtHandle,&gl_channelName,gl_channelOpenFlags,gl_timeout,
                      channelHandle);
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtChannelOpen() failed: %d",rs);
    }
}

void evt_channelClose(SaEvtChannelHandleT channelHandle)
{
  rs=saEvtChannelClose(channelHandle);
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtChannelClose() failed: %d",rs);
    }
}

void evt_channelUnlink(SaEvtHandleT evtHandle)
{
  rs=saEvtChannelUnlink(evtHandle,&gl_channelName);
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtChannelUnlink() failed: %d",rs);
    }
}

void evt_eventSubscribe(SaEvtChannelHandleT channelHandle,
                        SaEvtSubscriptionIdT subId)
{
  rs=saEvtEventSubscribe(channelHandle,&gl_filterArray,subId);
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtEventSubscribe() failed: %d",rs);
    }
}

void evt_dispatch(SaEvtHandleT evtHandle)
{
  rs=saEvtDispatch(evtHandle,gl_dispatchFlags);
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtDispatch() failed: %d",rs);
    }
}

void evt_eventAllocate(SaEvtChannelHandleT channelHandle,
                       SaEvtEventHandleT *eventHandle)
{
  rs=saEvtEventAllocate(channelHandle,eventHandle);
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtAllocate() failed: %d",rs);
    }
}

void evt_eventFree(SaEvtEventHandleT eventHandle)
{
  rs=saEvtEventFree(eventHandle);
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtFree() failed: %d",rs);
    }
}

void evt_eventAttributesSet(SaEvtEventHandleT *eventHandle)
{
  rs=saEvtEventAttributesSet(*eventHandle,&gl_patternArray,gl_priority,
                             gl_retentionTime,&gl_publisherName);
#if 0
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtEventAttributesSet() failed: %d",rs);
    }
#endif
}

void evt_eventPublish(SaEvtEventHandleT eventHandle,SaEvtEventIdT *evtId)
{
  rs=saEvtEventPublish(eventHandle,eventData,gl_eventDataSize,evtId);
#if 0
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtEventPublish() failed: %d",rs);
    }
#endif
}

void evt_eventDataGet(SaEvtEventHandleT eventHandle)
{
  rs=saEvtEventDataGet(eventHandle,eventData,&gl_eventDataSize);    
  if(rs!=SA_AIS_OK)
    {
      printf("\nsaEvtEventDataGet() failed: %d",rs);
    }
}
/*********************************************************************/
/*********************** IFSV CALLBACK *******************************/
/*********************************************************************/
void ifsv_cb(NCS_IFSV_SVC_RSP *rsp)
{
  int i=0;
  printf("\n\n--------Within ifsv_cb : Callback Invoked-----\n");

  m_NCS_MEMCPY(&testrsp,rsp,sizeof(NCS_IFSV_SVC_RSP));

  printf("\nResponse type: %d\n",testrsp.rsp_type);

  switch(testrsp.rsp_type)
    {
    case NCS_IFSV_IFREC_GET_RSP:
      gl_cbk=1;
      printf("\n\tCallback for IFSv Get Response\n");
      printf("\n********************\n");
      printf("\nInterface\n");      
      printf("--------------------");
      printf("\n\nReturn Code: %d",rsp->info.ifget_rsp.error);
      gl_error=rsp->info.ifget_rsp.error;
      printf("\nifIndex: %d",rsp->info.ifget_rsp.if_rec.if_index);
      printf("\nShelf Id: %d",rsp->info.ifget_rsp.if_rec.spt_info.shelf);
      printf("\nSlot Id: %d",rsp->info.ifget_rsp.if_rec.spt_info.slot);
      printf("\nPort Id: %d",rsp->info.ifget_rsp.if_rec.spt_info.port);
      printf("\nType: %d",rsp->info.ifget_rsp.if_rec.spt_info.type);
      printf("\nSubscriber Scope: %d",
             rsp->info.ifget_rsp.if_rec.spt_info.subscr_scope);      
      printf("\nInfo Type: %d",rsp->info.ifget_rsp.if_rec.info_type);
      printf("\n\nInterface Information\n");
      printf("--------------------");
      printf("\nAttribute Map Flag: %d",
             rsp->info.ifget_rsp.if_rec.if_info.if_am);
      printf("\nInterface Description: %s",
             rsp->info.ifget_rsp.if_rec.if_info.if_descr);
      printf("\nInterface Name: %s",
             rsp->info.ifget_rsp.if_rec.if_info.if_name);
      printf("\nMTU: %d",rsp->info.ifget_rsp.if_rec.if_info.mtu);
      printf("\nIf Speed: %d",rsp->info.ifget_rsp.if_rec.if_info.if_speed);
      printf("\nAdmin State: %d",
             rsp->info.ifget_rsp.if_rec.if_info.admin_state);
      printf("\nOperation Status: %d",
             rsp->info.ifget_rsp.if_rec.if_info.oper_state);
      if(rsp->info.ifget_rsp.if_rec.spt_info.type==NCS_IFSV_INTF_BINDING)
        {
          printf("\nMaster ifIndex: %d",
                 rsp->info.ifget_rsp.if_rec.if_info.bind_master_ifindex);
          printf("\nSlave ifIndex: %d",
                 rsp->info.ifget_rsp.if_rec.if_info.bind_slave_ifindex);
        }
      printf("\n\nInterface Statistics\n");
      printf("--------------------");
      printf("\nLast Change: %d",rsp->info.ifget_rsp.if_rec.if_stats.last_chg);
      printf("\nIn Octets: %d",rsp->info.ifget_rsp.if_rec.if_stats.in_octs);
      printf("\nIn Upkts: %d",rsp->info.ifget_rsp.if_rec.if_stats.in_upkts);
      printf("\nIn Nupkts: %d",rsp->info.ifget_rsp.if_rec.if_stats.in_nupkts);
      printf("\nIn Dscrds: %d",rsp->info.ifget_rsp.if_rec.if_stats.in_dscrds);
      printf("\nIn Errs: %d",rsp->info.ifget_rsp.if_rec.if_stats.in_dscrds);
      printf("\nIn Unkown Prots: %d",
             rsp->info.ifget_rsp.if_rec.if_stats.in_unknown_prots);
      printf("\nOut Octs: %d",rsp->info.ifget_rsp.if_rec.if_stats.out_octs);
      printf("\nOut Upkts: %d",rsp->info.ifget_rsp.if_rec.if_stats.out_upkts);
      printf("\nOut Nupkts: %d",
             rsp->info.ifget_rsp.if_rec.if_stats.out_nupkts);
      printf("\nOut Dscrds: %d",
             rsp->info.ifget_rsp.if_rec.if_stats.out_dscrds);
      printf("\nOut Errs: %d",rsp->info.ifget_rsp.if_rec.if_stats.out_errs);
      printf("\nOut Qlen: %d",rsp->info.ifget_rsp.if_rec.if_stats.out_qlen);
      printf("\nIf Specific: %d",
             rsp->info.ifget_rsp.if_rec.if_stats.if_specific);

      break;

    case NCS_IFSV_IFREC_ADD_NTFY:
      gl_cbk=1;
      printf("\n\tCallback to notify interface Addition\n");
      printf("\n********************\n");
      printf("\nInterface\n");      
      printf("--------------------");
      printf("\n\nifIndex: %d",rsp->info.ifadd_ntfy.if_index);
      gl_ifIndex=rsp->info.ifadd_ntfy.if_index;

      printf("\nShelf Id: %d",rsp->info.ifadd_ntfy.spt_info.shelf);
      printf("\nSlot Id: %d",rsp->info.ifadd_ntfy.spt_info.slot);
      printf("\nPort Id: %d",rsp->info.ifadd_ntfy.spt_info.port);
      printf("\nType: %d",rsp->info.ifadd_ntfy.spt_info.type);
      printf("\nSubscriber Scope: %d",
             rsp->info.ifadd_ntfy.spt_info.subscr_scope);
      printf("\nInfo Type: %d",rsp->info.ifadd_ntfy.info_type);
      printf("\n\nInterface Information\n");
      printf("--------------------");
      printf("\nAttribute Map Flag: %d",rsp->info.ifadd_ntfy.if_info.if_am);
      printf("\nInterface Description: %s",
             rsp->info.ifadd_ntfy.if_info.if_descr);
      printf("\nInterface Name: %s",rsp->info.ifadd_ntfy.if_info.if_name);
      printf("\nMTU: %d",rsp->info.ifadd_ntfy.if_info.mtu);
      printf("\nIf Speed: %d",rsp->info.ifadd_ntfy.if_info.if_speed);
      printf("\nAdmin State: %d",
             rsp->info.ifget_rsp.if_rec.if_info.admin_state);
      printf("\nOperation Status: %d",rsp->info.ifadd_ntfy.if_info.oper_state);
      if(rsp->info.ifadd_ntfy.spt_info.type==26)
        {
          printf("\nPhysical Address ");
          for(i=0;i<6;i++)
            printf(":%x",rsp->info.ifadd_ntfy.if_info.phy_addr[i]);
 
        }
      if(rsp->info.ifadd_ntfy.spt_info.type==NCS_IFSV_INTF_BINDING)
        {
          printf("\nMaster ifIndex: %d",
                 rsp->info.ifadd_ntfy.if_info.bind_master_ifindex);
          printf("\nSlave  ifIndex: %d",
                 rsp->info.ifadd_ntfy.if_info.bind_slave_ifindex);
          printf("\nMaster is on Node: %d",
                 rsp->info.ifadd_ntfy.if_info.bind_master_info.node_id);
          printf("\nSlave  is on Node: %d",
                 rsp->info.ifadd_ntfy.if_info.bind_slave_info.node_id);
        }

      /*get bond interfaces to a file*/
      char inputfile[]="/opt/opensaf/tetware/ifsv/suites/bond.text";
      FILE *fp;
      if(!strncmp(rsp->info.ifadd_ntfy.if_info.if_name,"bond",4)) 
        {
          if( (fp= fopen(inputfile,"a")) == NULL)
            perror("File not opened");
          else
            {

              fprintf(fp,"\nSlot Id: %d",rsp->info.ifadd_ntfy.spt_info.slot);
              fprintf(fp,"\nInterface Name: %s",
                      rsp->info.ifadd_ntfy.if_info.if_name);
              if(rsp->info.ifadd_ntfy.spt_info.type==26)
                {
                  fprintf(fp,"\nPhysical Address ");
                  for(i=0;i<6;i++)
                    fprintf(fp,":%x",rsp->info.ifadd_ntfy.if_info.phy_addr[i]);
                }
              if(fclose(fp)==EOF)
                perror("File not closed");

            }
        }

#if 0
      printf("\n\nInterface Statistics\n");
      printf("--------------------");
      printf("\nLast Change: %d",rsp->info.ifadd_ntfy.if_stats.last_chg);
      printf("\nIn Octets: %d",rsp->info.ifadd_ntfy.if_stats.in_octs);
      printf("\nIn Upkts: %d",rsp->info.ifadd_ntfy.if_stats.in_upkts);
      printf("\nIn Nupkts: %d",rsp->info.ifadd_ntfy.if_stats.in_nupkts);
      printf("\nIn Dscrds: %d",rsp->info.ifadd_ntfy.if_stats.in_dscrds);
      printf("\nIn Errs: %d",rsp->info.ifadd_ntfy.if_stats.in_dscrds);
      printf("\nIn Unkown Prots: %d",
             rsp->info.ifadd_ntfy.if_stats.in_unknown_prots);
      printf("\nOut Octs: %d",rsp->info.ifadd_ntfy.if_stats.out_octs);
      printf("\nOut Upkts: %d",rsp->info.ifadd_ntfy.if_stats.out_upkts);
      printf("\nOut Nupkts: %d",rsp->info.ifadd_ntfy.if_stats.out_nupkts);
      printf("\nOut Dscrds: %d",rsp->info.ifadd_ntfy.if_stats.out_dscrds);
      printf("\nOut Errs: %d",rsp->info.ifadd_ntfy.if_stats.out_errs);
      printf("\nOut Qlen: %d",rsp->info.ifadd_ntfy.if_stats.out_qlen);
      printf("\nIf Specific: %d",rsp->info.ifadd_ntfy.if_stats.if_specific);

#endif

#if 0
      if(rsp->info.ifadd_ntfy.spt_info.shelf
         == req_info.info.i_ifadd.spt_info.shelf)
        {
          if(rsp->info.ifadd_ntfy.spt_info.slot
             == req_info.info.i_ifadd.spt_info.slot)
            {
              if(rsp->info.ifadd_ntfy.spt_info.port
                 == req_info.info.i_ifadd.spt_info.port)
                {
                  if(rsp->info.ifadd_ntfy.spt_info.type
                     ==req_info.info.i_ifadd.spt_info.type)
                    {
                      if(rsp->info.ifadd_ntfy.spt_info.subscr_scope
                         ==req_info.info.i_ifadd.spt_info.subscr_scope)
                        {
                          if(rsp->info.ifadd_ntfy.if_info.if_am
                             ==req_info.info.i_ifadd.if_info.if_am)
                            {   
                              if(strcmp(rsp->info.ifadd_ntfy.if_info.if_descr,
                                        req_info.info.i_ifadd.if_info.if_descr)
                                 ==0)
                                {
                                  if(strcmp(rsp->info.ifadd_ntfy.if_info.if_name,req_info.info.i_ifadd.if_info.if_name)==0)
                                    {
                                      if(rsp->info.ifadd_ntfy.if_info.mtu
                                         ==req_info.info.i_ifadd.if_info.mtu)
                                        {
                                          if(rsp->info.ifadd_ntfy.if_info.if_speed==req_info.info.i_ifadd.if_info.if_speed)
                                            {
                                              if(rsp->info.ifadd_ntfy.if_info.admin_state==req_info.info.i_ifadd.if_info.admin_state)
                                                {
                                                  if(rsp->info.ifadd_ntfy.if_info.oper_state==req_info.info.i_ifadd.if_info.oper_state)
                                                    {
                                                      printf("\n\nInterface added successfully");
                                                      tet_printf("Interface added successfully");
                                             
                                                    }
                                                  else
                                                    {
                                                      printf("\nTest Case: FAILED");
                                                      printf("\nOper state received in callback is different from the value set");
                                                      tet_printf("Oper state speed received in callback is different from the value set");
                                                      tet_result(TET_FAIL);
                                                    }
                                                }
                                              else
                                                {
                                                  printf("\nTest Case: FAILED");
                                                  printf("\nAdmin state received in callback is different from the value set");
                                                  tet_printf("Admin state in callback is different from the value set");
                                                  tet_result(TET_FAIL);
                                                }
                                            }
                                          else
                                            {
                                              printf("\nTest Case: FAILED");
                                              printf("\nInterface speed received in callback is different from the value set");
                                              tet_printf("Interface speed received in callback is different from the value set");
                                              tet_result(TET_FAIL);
                                            }
                                        }
                                      else
                                        {
                                          printf("\nTest Case: FAILED");
                                          printf("\nMTU received in callback is different from the value set");
                                          tet_printf("MTU received in callback is different from the value set");
                                          tet_result(TET_FAIL);
                                        }
                                    }
                                  else
                                    {
                                      printf("\nTest Case: FAILED");
                                      printf("\nInterface name received in callback is different from the value set");
                                      tet_printf("Interface name received in callback is different from the value set");
                                      tet_result(TET_FAIL);
                                    }  
                                }
                              else
                                {
                                  printf("\nTest Case: FAILED");
                                  printf("\nInterface description received in callback is different from the value set");
                                  tet_printf("Interface description received in callback is different from the value set");
                                  tet_result(TET_FAIL);
                                }  
                            }
                          else
                            {
                              printf("\nTest Case: FAILED");
                              printf("\nBit map attributes received in callback is different from the value set");
                              tet_printf("Bit map attributes in callback is different from the value set");
                              tet_result(TET_FAIL);
                            }
                        }
                      else
                        {
                          printf("\nTest Case: FAILED");
                          printf("\nSubscribe scope received in callback is different from the value set");
                          tet_printf("Subscriber scope received in callback is different from the value set");
                          tet_result(TET_FAIL);
                        } 
                    }
                  else
                    {
                      printf("\nTest Case: FAILED");
                      printf("\nType received in callback is different from the value set");
                      tet_printf("Type received in callback is different from the value set");
                      tet_result(TET_FAIL);
                    }
                }
              else
                {
                  printf("\nTest Case: FAILED");
                  printf("\nPort received in callback is different from the value set");
                  tet_printf("Port received in callback is different from the value set");
                  tet_result(TET_FAIL);
                }   
            }  
          else
            {
              printf("\nTest Case: FAILED");
              printf("\nSlot ID received in callback is different from the value set");
              tet_printf("Slot ID received in callback is different from the value set");
              tet_result(TET_FAIL);
            }      
        }
      else
        {
          printf("\nTest Case: FAILED");
          printf("\nShelf ID received in callback is different from the value set");
          tet_printf("Shelf ID received in callback is different from the value set");
          tet_result(TET_FAIL);
        }
#endif
      if(gl_eds)
        {
          SaEvtEventPatternT add_ntfy[1]=
            {
              {3,3,(SaUint8T *)"add"}
            };
      
          gl_patternArray.patterns=add_ntfy; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData=rsp;
          gl_eventDataSize=4096;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
#if 0
          printf("\nevtId: %llu",gl_evtId);
#endif
        }
  
      if(strcmp(rsp->info.ifadd_ntfy.if_info.if_descr,"if_descr")==0)
        {
          cmprsp=*rsp;
        }

      printf("\n\n*******************");  
      break;

    case NCS_IFSV_IFREC_DEL_NTFY:
      printf("\n\tCallback to notify interface Deletion\n");
      printf("\n\n*******************");
      printf("\nInterface\n");
      printf("\nifIndex: %d",rsp->info.ifdel_ntfy);
      printf("\n\n*******************");

      if(strcmp(rsp->info.ifadd_ntfy.if_info.if_descr,"if_descr")==0)
        {
          cmprsp=*rsp;
        }
  
      break;

    case NCS_IFSV_IFREC_UPD_NTFY:
      gl_cbk=1;
      printf("\n\tCallback to notify interface Update\n");
      printf("\n********************\n");
      printf("\nInterface\n");      
      printf("--------------------");
      printf("\n\nifIndex: %d",rsp->info.ifupd_ntfy.if_index);

      gl_ifIndex=rsp->info.ifupd_ntfy.if_index;

      printf("\nShelf Id: %d",rsp->info.ifupd_ntfy.spt_info.shelf);
      printf("\nSlot Id: %d",rsp->info.ifupd_ntfy.spt_info.slot);
      printf("\nPort Id: %d",rsp->info.ifupd_ntfy.spt_info.port);
      printf("\nType: %d",rsp->info.ifupd_ntfy.spt_info.type);
      printf("\nSubscriber Scope: %d",
             rsp->info.ifupd_ntfy.spt_info.subscr_scope);
      printf("\nInfo Type: %d",rsp->info.ifupd_ntfy.info_type);
      printf("\n\nInterface Information\n");
      printf("--------------------");
      printf("\nAttribute Map Flag: %d",rsp->info.ifupd_ntfy.if_info.if_am);
      printf("\nInterface Description: %s",
             rsp->info.ifupd_ntfy.if_info.if_descr);
      printf("\nInterface Name: %s",rsp->info.ifupd_ntfy.if_info.if_name);
      printf("\nMTU: %d",rsp->info.ifupd_ntfy.if_info.mtu);
      printf("\nIf Speed: %d",rsp->info.ifupd_ntfy.if_info.if_speed);
      printf("\nAdmin State: %d",
             rsp->info.ifget_rsp.if_rec.if_info.admin_state);
      printf("\nOperation Status: %d",rsp->info.ifupd_ntfy.if_info.oper_state);
      if(rsp->info.ifupd_ntfy.spt_info.type==NCS_IFSV_INTF_BINDING)
        {
          printf("\nMaster ifIndex: %d",
                 rsp->info.ifupd_ntfy.if_info.bind_master_ifindex);
          printf("\nSlave ifIndex: %d",
                 rsp->info.ifupd_ntfy.if_info.bind_slave_ifindex);
        }

#if 0

      printf("\n\nInterface Statistics\n");
      printf("--------------------");
      printf("\nLast Change: %d",rsp->info.ifupd_ntfy.if_stats.last_chg);
      printf("\nIn Octets: %d",rsp->info.ifupd_ntfy.if_stats.in_octs);
      printf("\nIn Upkts: %d",rsp->info.ifupd_ntfy.if_stats.in_upkts);
      printf("\nIn Nupkts: %d",rsp->info.ifupd_ntfy.if_stats.in_nupkts);
      printf("\nIn Dscrds: %d",rsp->info.ifupd_ntfy.if_stats.in_dscrds);
      printf("\nIn Errs: %d",rsp->info.ifupd_ntfy.if_stats.in_dscrds);
      printf("\nIn Unkown Prots: %d",
             rsp->info.ifupd_ntfy.if_stats.in_unknown_prots);
      printf("\nOut Octs: %d",rsp->info.ifupd_ntfy.if_stats.out_octs);
      printf("\nOut Upkts: %d",rsp->info.ifupd_ntfy.if_stats.out_upkts);
      printf("\nOut Nupkts: %d",rsp->info.ifupd_ntfy.if_stats.out_nupkts);
      printf("\nOut Dscrds: %d",rsp->info.ifupd_ntfy.if_stats.out_dscrds);
      printf("\nOut Errs: %d",rsp->info.ifupd_ntfy.if_stats.out_errs);
      printf("\nOut Qlen: %d",rsp->info.ifupd_ntfy.if_stats.out_qlen);
      printf("\nIf Specific: %d",rsp->info.ifupd_ntfy.if_stats.if_specific);

#endif
      if(gl_eds)
        {
          SaEvtEventPatternT upd_ntfy[1]=
            {
              {3,3,(SaUint8T *)"dis"}
            };
      
          gl_patternArray.patterns=upd_ntfy; 
          evt_eventAttributesSet(&gl_eventHandle);

          eventData=rsp;
          gl_eventDataSize=4096;
          evt_eventPublish(gl_eventHandle,&gl_evtId);  
#if 0
          printf("\nevtId: %llu",gl_evtId);
#endif
        }
  
      if(strcmp(rsp->info.ifadd_ntfy.if_info.if_descr,"if_descr")==0)
        {
          cmprsp=*rsp;
        }

      printf("\n\n*******************");  
      break;

    }
  printf("\n------Leaving ifsv_cb : Callback Exited-----\n");
  fflush(stdout);
}
/*********************************************************************/
/*********************** DRIVER CALLBACK *****************************/
/*********************************************************************/
void ifsv_driver_cb(NCS_IFSV_HW_DRV_REQ *drv_req)
{
  printf("\n\n--------Within ifsv_driver_cb : Callback Invoked-----\n");
  switch(drv_req->req_type)
    {
    case NCS_IFSV_HW_DRV_STATS:

      printf("\nHardware driver statistics");
      break;

    case NCS_IFSV_HW_DRV_PORT_REG:

      printf("\nHardware driver port registration");
      break;

    case NCS_IFSV_HW_DRV_PORT_STATUS:

      printf("Hardware driver port status");
      break;

    case NCS_IFSV_HW_DRV_SET_PARAM:

      printf("Hardware driver parameter set");
      break;

    case NCS_IFSV_HW_DRV_IFND_UP:

      printf("Hardware driver to IFND up");
      break;

    case NCS_IFSV_HW_DRV_PORT_SYNC_UP:

      printf("\nHardware driver port sync up");
      break;

    case NCS_IFSV_HW_DRV_MAX_MSG:

      printf("\nHardware driver max message");
      break;
    }
  printf("\n------Leaving ifsv_driver_cb : Callback Exited-----\n");
  fflush(stdout);
}
/*********************************************************************/
/*********************** DRIVER WRAPPERS *****************************/
/*********************************************************************/
struct ifsv_driver_init driver_init_api[]={
  [IFSV_DRIVER_INIT_NULL_PARAM]     = {0},
  [IFSV_DRIVER_INIT_INVALID_TYPE]   = {24},
  [IFSV_DRIVER_INIT_SUCCESS]        = {NCS_IFSV_DRV_INIT_REQ},
};

struct ifsv_driver_destroy driver_destroy_api[]={
  [IFSV_DRIVER_DESTROY_NULL_PARAM]     = {0},
  [IFSV_DRIVER_DESTROY_INVALID_TYPE]   = {24},
  [IFSV_DRIVER_DESTROY_SUCCESS]        = {NCS_IFSV_DRV_DESTROY_REQ},
};

struct ifsv_driver_port_reg driver_port_reg_api[]={
  [IFSV_DRIVER_PORT_REG_NULL_PARAM]    = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_NULL]      = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_INVALID]   = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_MTU]       = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_IFSPEED]   = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_PHYADDR]   = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_ADMSTATE]  = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_OPRSTATE]  = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_NAME]      = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_DESCR]     = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_LAST_CHNG] = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_IAM_SVDEST]    = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_PORT_TYPE_NULL]= {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_PORT_TYPE_INVALID] = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_PORT_TYPE_INTF_OTHER] = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_PORT_TYPE_LOOPBACK] = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_PORT_TYPE_ETHERNET] = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_PORT_TYPE_TUNNEL] = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_SUCCESS]       = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_UPD_SUCCESS]   = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_DISABLE_INTERFACE] = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_ENABLE_INTERFACE] = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_MODIFY_INTERFACE] = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
  [IFSV_DRIVER_PORT_REG_INTERFACE_SEQ_ALLOC] = {NCS_IFSV_DRV_PORT_REG_REQ,&port_reg},
};

struct ifsv_driver_send driver_send_api[]={
  [IFSV_DRIVER_SEND_NULL_PARAM]          = {NCS_IFSV_DRV_SEND_REQ,&hw_info},
  [IFSV_DRIVER_SEND_MSG_TYPE_INVALID]    = {NCS_IFSV_DRV_SEND_REQ,&hw_info},
  [IFSV_DRIVER_SEND_MSG_TYPE_PORT_REG]   = {NCS_IFSV_DRV_SEND_REQ,&hw_info},
};

/******* Driver Wrappers ***********/
void ifsv_driver_init(int i)
{
  drv_info.req_type=driver_init_api[i].req_type;

  gl_rc=ncs_ifsv_drv_svc_req(&drv_info);
}

void ifsv_driver_destroy(int i)
{
  drv_info.req_type=driver_destroy_api[i].req_type;
  sleep(1);
  gl_rc=ncs_ifsv_drv_svc_req(&drv_info);
  if(gl_rc==NCSCC_RC_SUCCESS)
    sleep(15);
}

void ifsv_driver_port_reg_fill(int i)
{
  char if_name[20]="Interface_Driver";
  if(i>12)
    {
      port_reg.port_info.if_am=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED
        |NCS_IFSV_IAM_PHYADDR|NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE
        |NCS_IFSV_IAM_NAME|NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG
        |NCS_IFSV_IAM_SVDEST;
    }

  port_reg.port_info.port_type.port_id=i;
  port_reg.port_info.oper_state=1;
  port_reg.port_info.admin_state=1;
  port_reg.port_info.mtu=i;
  memcpy(port_reg.port_info.if_name,if_name,sizeof(if_name));
  port_reg.port_info.speed=i*10;
  port_reg.drv_data="Driver";
  port_reg.hw_get_set_cb=(NCS_IFSV_HW_DRV_GET_SET_CB)ifsv_driver_cb;
    
  switch(i)
    {
    case 1:

      break;

    case 2:

      port_reg.port_info.if_am=0;
      break;

    case 3:

      port_reg.port_info.if_am=1024;
      break;

    case 4:

      port_reg.port_info.if_am=NCS_IFSV_IAM_MTU;
      break;

    case 5:

      port_reg.port_info.if_am=NCS_IFSV_IAM_IFSPEED;
      break;

    case 6:

      port_reg.port_info.if_am=NCS_IFSV_IAM_PHYADDR;
      break;

    case 7:

      port_reg.port_info.if_am=NCS_IFSV_IAM_ADMSTATE;
      break;

    case 8:

      port_reg.port_info.if_am=NCS_IFSV_IAM_OPRSTATE;
      break;

    case 9:

      port_reg.port_info.if_am=NCS_IFSV_IAM_NAME;
      break;

    case 10:

      port_reg.port_info.if_am=NCS_IFSV_IAM_DESCR;
      break;

    case 11:

      port_reg.port_info.if_am=NCS_IFSV_IAM_LAST_CHNG;
      break;

    case 12:

      port_reg.port_info.if_am=NCS_IFSV_IAM_SVDEST;
      break;
 
    case 13:
 
      port_reg.port_info.port_type.type=0;
      break;
 
    case 14:
 
      port_reg.port_info.port_type.type=256;
      break;
 
    case 15:
 
      port_reg.port_info.port_type.type=NCS_IFSV_INTF_OTHER;
      break;
 
    case 16:
 
      port_reg.port_info.port_type.type=NCS_IFSV_INTF_LOOPBACK;
      break;
 
    case 17:
 
      port_reg.port_info.port_type.type=NCS_IFSV_INTF_ETHERNET;
      break;
 
    case 18:
 
      port_reg.port_info.port_type.type=NCS_IFSV_INTF_TUNNEL;
      break;

    case 19:

      port_reg.port_info.port_type.type=NCS_IFSV_INTF_OTHER;
      port_reg.port_info.oper_state=1;
      port_reg.port_info.admin_state=1;
      port_reg.port_info.mtu=i;
      memcpy(port_reg.port_info.if_name,if_name,sizeof(if_name));
      port_reg.port_info.speed=i*10;
      port_reg.drv_data="Driver";
      port_reg.hw_get_set_cb=(NCS_IFSV_HW_DRV_GET_SET_CB)ifsv_driver_cb;
      break;

    case 20:

      port_reg.port_info.port_type.type=NCS_IFSV_INTF_OTHER;
      port_reg.port_info.oper_state=0;
      port_reg.port_info.admin_state=0;
      port_reg.port_info.mtu=i+1;
      memcpy(port_reg.port_info.if_name,if_name,sizeof(if_name));
      port_reg.port_info.speed=i*20;
      port_reg.drv_data="Driver";
      port_reg.hw_get_set_cb=(NCS_IFSV_HW_DRV_GET_SET_CB)ifsv_driver_cb;
      break;

    case 21:

      i=19;
      port_reg.port_info.port_type.type=NCS_IFSV_INTF_OTHER;
      port_reg.port_info.oper_state=2;
      port_reg.port_info.admin_state=1;
      port_reg.port_info.port_type.port_id=i;
      port_reg.port_info.mtu=i;
      memcpy(port_reg.port_info.if_name,if_name,sizeof(if_name));
      port_reg.port_info.speed=i*10;
      port_reg.drv_data="Driver";
      port_reg.hw_get_set_cb=(NCS_IFSV_HW_DRV_GET_SET_CB)ifsv_driver_cb; 
      break;

    case 22:

      i=19;
      port_reg.port_info.port_type.type=NCS_IFSV_INTF_OTHER;
      port_reg.port_info.oper_state=1;
      port_reg.port_info.admin_state=1;
      port_reg.port_info.port_type.port_id=i;
      port_reg.port_info.mtu=i;
      memcpy(port_reg.port_info.if_name,if_name,sizeof(if_name));
      port_reg.port_info.speed=i*10;
      port_reg.drv_data="Driver";
      port_reg.hw_get_set_cb=(NCS_IFSV_HW_DRV_GET_SET_CB)ifsv_driver_cb; 
      break;

    case 23:

      i=19;
      port_reg.port_info.port_type.type=NCS_IFSV_INTF_OTHER;
      port_reg.port_info.oper_state=2;
      port_reg.port_info.admin_state=2;
      port_reg.port_info.port_type.port_id=i;
      port_reg.port_info.mtu=i*2;
      port_reg.port_info.speed=i*20;
      port_reg.drv_data="Driver";
      port_reg.hw_get_set_cb=(NCS_IFSV_HW_DRV_GET_SET_CB)ifsv_driver_cb; 
      break;

    case 24:

      port_reg.port_info.port_type.type=NCS_IFSV_INTF_OTHER;
      port_reg.port_info.oper_state=1;
      port_reg.port_info.admin_state=1;
      port_reg.port_info.mtu=i;
      memcpy(port_reg.port_info.if_name,if_name,sizeof(if_name));
      port_reg.port_info.speed=i*10;
      port_reg.drv_data="Driver";
      port_reg.hw_get_set_cb=(NCS_IFSV_HW_DRV_GET_SET_CB)ifsv_driver_cb;
      break;

    default:
 
      break;
    }
}

void ifsv_driver_port_reg(int i)
{
  ifsv_driver_port_reg_fill(i);

  drv_info.req_type=driver_port_reg_api[i].req_type;

  drv_info.info.port_reg=
    (NCS_IFSV_PORT_REG *)malloc(sizeof(NCS_IFSV_PORT_REG));

  drv_info.info.port_reg->hw_get_set_cb=
    (NCS_IFSV_HW_DRV_GET_SET_CB)ifsv_driver_cb;

  drv_info.info.port_reg->port_info.if_am=
    driver_port_reg_api[i].port_reg->port_info.if_am;

  drv_info.info.port_reg->port_info.port_type.port_id=
    driver_port_reg_api[i].port_reg->port_info.port_type.port_id;
  drv_info.info.port_reg->port_info.port_type.type=
    driver_port_reg_api[i].port_reg->port_info.port_type.type;

  drv_info.info.port_reg->port_info.oper_state=
    driver_port_reg_api[i].port_reg->port_info.oper_state;

  drv_info.info.port_reg->port_info.admin_state=
    driver_port_reg_api[i].port_reg->port_info.admin_state;
    
  drv_info.info.port_reg->port_info.mtu=
    driver_port_reg_api[i].port_reg->port_info.mtu;

  memcpy(drv_info.info.port_reg->port_info.if_name,
         driver_port_reg_api[i].port_reg->port_info.if_name,
         sizeof(driver_port_reg_api[i].port_reg->port_info.if_name));

  drv_info.info.port_reg->port_info.speed=
    driver_port_reg_api[i].port_reg->port_info.speed;
   

  gl_rc=ncs_ifsv_drv_svc_req(&drv_info);
}

void ifsv_driver_send_fill(int i)
{
  switch(i)
    {
    case 1:
                    
      break;

    case 2:
    
      hw_info.msg_type=24;
      break;

    case 3:
 
      hw_info.msg_type=NCS_IFSV_HW_DRV_PORT_STATUS|NCS_IFSV_HW_DRV_STATS;
      hw_info.port_type.port_id=
        drv_info.info.port_reg->port_info.port_type.port_id;
      hw_info.port_type.type=drv_info.info.port_reg->port_info.port_type.type;
      break;
     
    default:
       
      break;
    }
}

void ifsv_driver_send(int i)
{
  ifsv_driver_send_fill(i);
 
  drv_info.req_type=driver_send_api[i].req_type;

  drv_info.info.hw_info=(NCS_IFSV_HW_INFO *)malloc(sizeof(NCS_IFSV_HW_INFO));
  drv_info.info.hw_info->msg_type=driver_send_api[i].hw_info->msg_type;
   
  drv_info.info.hw_info->port_type.port_id=
    driver_send_api[i].hw_info->port_type.port_id;
  drv_info.info.hw_info->port_type.type=
    driver_send_api[i].hw_info->port_type.type;
    
   
  gl_rc=ncs_ifsv_drv_svc_req(&drv_info);
}
/*********************************************************************/
/***********************   IPXS CALLBACK *****************************/
/*********************************************************************/
void ipxs_cb(NCS_IPXS_RSP_INFO *rsp)
{
  struct in_addr in;
  NCS_IPXS_INTF_REC *ptr = NULL;
 
  printf("\n\n--------Within ipxs_cb : Callback Invoked-----\n");
  printf("\nResponse type: %d\n",rsp->type);
   
   
  switch(rsp->type)
    {
    case NCS_IPXS_REC_GET_RSP:
 
      gl_cbk=1;
      printf("\n\tCallback for IPXS get response");
      printf("\n********************\n");
      printf("\nReturn Code: %d",rsp->info.get_rsp.err);
      gl_error=rsp->info.get_rsp.err;
      printf("\n\nInterface\n");
      printf("--------------------");
      printf("\n\nifIndex: %d",rsp->info.get_rsp.ipinfo->if_index);
      printf("\nShelf Id: %d",rsp->info.get_rsp.ipinfo->spt.shelf);
      printf("\nSlot Id: %d",rsp->info.get_rsp.ipinfo->spt.slot);
      printf("\nPort : %d",rsp->info.get_rsp.ipinfo->spt.port);
      printf("\nType : %d",rsp->info.get_rsp.ipinfo->spt.type);
      printf("\nSubscriber Scope : %d",
             rsp->info.get_rsp.ipinfo->spt.subscr_scope);
      printf("\nInterface local on this node: %d",
             rsp->info.get_rsp.ipinfo->is_lcl_intf);
      printf("\n\nInterface attribute information\n");      
      printf("--------------------");
      printf("\nAttribute Map Flag: %d",
             rsp->info.get_rsp.ipinfo->if_info.if_am);
      printf("\nInterface Description: %s",
             rsp->info.get_rsp.ipinfo->if_info.if_descr);
      printf("\nInterface Name: %s",rsp->info.get_rsp.ipinfo->if_info.if_name);
      printf("\nMTU: %d",rsp->info.get_rsp.ipinfo->if_info.mtu);
      printf("\nIf Speed: %d",rsp->info.get_rsp.ipinfo->if_info.if_speed);
      printf("\nAdmin State: %d",
             rsp->info.get_rsp.ipinfo->if_info.admin_state);
      printf("\nOper State: %d",rsp->info.get_rsp.ipinfo->if_info.oper_state);
      printf("\n\nIP attribute information\n");      
      printf("--------------------");
      printf("\n\nAttribute bit map: %d",
             rsp->info.get_rsp.ipinfo->ip_info.ip_attr);
      printf("\nIPV4 numbered flag: %d",
             rsp->info.get_rsp.ipinfo->ip_info.is_v4_unnmbrd);
      if(rsp->info.get_rsp.ipinfo->ip_info.addip_list)
        {
          printf("\nIP address type(added IP): %d",
                 rsp->info.get_rsp.ipinfo->ip_info.addip_list->ipaddr.ipaddr.type);
          in.s_addr=(rsp->info.get_rsp.ipinfo->ip_info.addip_list->ipaddr.ipaddr.info.v4);
          printf("\nIP address: %s",inet_ntoa(in));
          printf("\nBit mask length(added IP): %d",
                 rsp->info.get_rsp.ipinfo->ip_info.addip_list->ipaddr.mask_len);
          printf("\nIP address(added) count list: %d",
                 rsp->info.get_rsp.ipinfo->ip_info.addip_cnt);
        }

      if(rsp->info.get_rsp.ipinfo->ip_info.delip_list)
        {
          printf("\nIP address type(deleted IP): %d",
                 rsp->info.get_rsp.ipinfo->ip_info.delip_list->ipaddr.type);
          printf("\nBit mask length(deleted IP): %d",
                 rsp->info.get_rsp.ipinfo->ip_info.delip_list->mask_len);
          printf("\nIP address(deleted) count list: %d",
                 rsp->info.get_rsp.ipinfo->ip_info.delip_cnt);
        }
      printf("\nNode address at other end: %d",
             rsp->info.get_rsp.ipinfo->ip_info.rrtr_ipaddr.type);
      printf("\nRouter ID at other end: %d",
             rsp->info.get_rsp.ipinfo->ip_info.rrtr_rtr_id);
      printf("\nAutonomous system ID at other end: %d",
             rsp->info.get_rsp.ipinfo->ip_info.rrtr_as_id);
      printf("\nifIndex at other end: %d",
             rsp->info.get_rsp.ipinfo->ip_info.rrtr_if_id);
      printf("\nUser defined: %d",rsp->info.get_rsp.ipinfo->ip_info.ud_1);
      if(rsp->info.get_rsp.err==1)
        {
          printf("\n\nInterface get information successful");
        }
      printf("\n\n*******************");  
      break;

    case NCS_IPXS_REC_ADD_NTFY:

      gl_cbk=1; 
      ptr = rsp->info.add.i_ipinfo; 
      if(strcmp(ptr->if_info.if_descr,"if_descr")==0)
        {
          cmpip=*rsp;
          cmpip.info.add.i_ipinfo=(NCS_IPXS_INTF_REC *)malloc(sizeof(NCS_IPXS_INTF_REC));
          *cmpip.info.add.i_ipinfo=*rsp->info.add.i_ipinfo;
        }


      while(ptr) 
        {
#if 0
          if(ptr->is_lcl_intf==1) 
#endif
            {
              if(1)
                {
                  printf("\n\tCallback to notify interface Addition");
                  printf("\n********************\n");
                  printf("\nInterface\n");      
                  printf("--------------------");
                  printf("\n\nifIndex: %d",ptr->if_index);
                  gl_ifIndex=ptr->if_index;
#if 0
                  if( (strncmp(iname,
                               ptr->if_info.if_name,3==0))
                      &&
                      (ptr->if_info.oper_state==1) )
                    {
                      printf("\nIn cmp:");
       
                      if(b_iface.mifindex==0)
                        {   
                          b_iface.mifindex=gl_ifIndex; 
                          sid=ptr->spt.slot;
                        }
                      if((b_iface.sifindex==0)
                         &&
                         sid!=ptr->spt.slot)
                        {
                          b_iface.sifindex=gl_ifIndex; 
                        }
                      printf("\nifindex for master: %d",b_iface.mifindex);
                      printf("\nifindex for slave: %d",b_iface.sifindex);
                    }      
#endif
                  printf("\nShelf Id: %d",ptr->spt.shelf);
                  printf("\nSlot Id: %d",ptr->spt.slot);
                  printf("\nPort : %d",ptr->spt.port);
                  printf("\nType : %d",ptr->spt.type);
                  printf("\nSubscriber Scope : %d",
                         ptr->spt.subscr_scope);
                  printf("\nInterface local on this node: %d",
                         ptr->is_lcl_intf);
                  printf("\n\nInterface attribute information\n");      
                  printf("--------------------");
                  printf("\nAttribute Map Flag: %d",
                         ptr->if_info.if_am);
                  printf("\nInterface Description: %s",
                         ptr->if_info.if_descr);
                  printf("\nInterface Name: %s",
                         ptr->if_info.if_name);
                  printf("\nMTU: %d",ptr->if_info.mtu);
                  printf("\nIf Speed: %d",
                         ptr->if_info.if_speed);
                  printf("\nAdmin State: %d",
                         ptr->if_info.admin_state);
                  printf("\nOper State: %d",
                         ptr->if_info.oper_state);
                  printf("\n\nIP attribute information\n");      
                  printf("--------------------");
                  printf("\n\nAttribute bit map: %d",
                         ptr->ip_info.ip_attr);
                  printf("\nIPV4 numbered flag: %d",
                         ptr->ip_info.is_v4_unnmbrd);
                  if(ptr->ip_info.addip_list)
                    {
                      printf("\nIP address type(added IP): %d",
                             ptr->ip_info.addip_list->ipaddr.ipaddr.type);
                      printf("\nBit mask length(added IP): %d",
                             ptr->ip_info.addip_list->ipaddr.mask_len);
                      printf("\nIP address(added) count list: %d",
                             ptr->ip_info.addip_cnt);
              
                    }
#if 0
                  if(ptr->ip_info.delip_list)
                    {
                      printf("\nIP address type(deleted IP): %d",
                             ptr->ip_info.delip_list->ipaddr.type);
                      printf("\nBit mask length(deleted IP): %d",
                             ptr->ip_info.delip_list->mask_len);
                      printf("\nIP address(deleted) count list: %d",
                             ptr->ip_info.delip_cnt);
                    }
#endif
                  printf("\nNode address at other end: %d",
                         ptr->ip_info.rrtr_ipaddr.type);
                  printf("\nRouter ID at other end: %d",
                         ptr->ip_info.rrtr_rtr_id);
                  printf("\nAutonomous system ID at other end: %d",
                         ptr->ip_info.rrtr_as_id);
                  printf("\nifIndex at other end: %d",
                         ptr->ip_info.rrtr_if_id);
                  printf("\nUser defined: %d",
                         ptr->ip_info.ud_1);
                  printf("\n\nInterface added successfully");
                  printf("\n\n*******************");  

#if 0
                  if(ptr->spt.shelf
                     ==req_info.info.i_ifadd.spt_info.shelf)
                    {
                      if(ptr->spt.slot
                         ==req_info.info.i_ifadd.spt_info.slot)
                        {
                          if(ptr->spt.port
                             ==req_info.info.i_ifadd.spt_info.port)
                            {
                              if(ptr->spt.type
                                 ==req_info.info.i_ifadd.spt_info.type)
                                {
                                  if(ptr->spt.subscr_scope
                                     ==req_info.info.i_ifadd.spt_info.subscr_scope)
                                    {
                                      if(ptr->if_info.if_am
                                         ==req_info.info.i_ifadd.if_info.if_am)
                                        {   
                                          if(strcmp(ptr->if_info.if_descr,req_info.info.i_ifadd.if_info.if_descr)==0)
                                            {
                                              if(strcmp(ptr->if_info.if_name,req_info.info.i_ifadd.if_info.if_name)==0)
                                                {
                                                  if(ptr->if_info.mtu==req_info.info.i_ifadd.if_info.mtu)
                                                    {
                                                      if(ptr->if_info.if_speed==req_info.info.i_ifadd.if_info.if_speed)
                                                        {
                                                          if(ptr->if_info.admin_state==req_info.info.i_ifadd.if_info.admin_state)
                                                            {
                                                              if(ptr->if_info.oper_state==req_info.info.i_ifadd.if_info.oper_state)
                                                                {
                                                                  printf("\n\nInterface added successfully");
                                                                  tet_printf("Interface added successfully");
                                                                }
                                                              else
                                                                {
                                                                  printf("\nTest Case: FAILED");
                                                                  printf("\nOper state received in callback is different from the value set");
                                                                  tet_printf("Oper state speed received in callback is different from the value set");
                                                                  tet_result(TET_FAIL);
                                                                }
                                                            }
                                                          else
                                                            {
                                                              printf("\nTest Case: FAILED");
                                                              printf("\nAdmin state received in callback is different from the value set");
                                                              tet_printf("Admin state in callback is different from the value set");
                                                              tet_result(TET_FAIL);
                                                            }
                                                        }
                                                      else
                                                        {
                                                          printf("\nTest Case: FAILED");
                                                          printf("\nInterface speed received in callback is different from the value set");
                                                          tet_printf("Interface speed received in callback is different from the value set");
                                                          tet_result(TET_FAIL);
                                                        }
                                                    }
                                                  else
                                                    {
                                                      printf("\nTest Case: FAILED");
                                                      printf("\nMTU received in callback is different from the value set");
                                                      tet_printf("MTU received in callback is different from the value set");
                                                      tet_result(TET_FAIL);
                                                    }
                                                }
                                              else
                                                {
                                                  printf("\nTest Case: FAILED");
                                                  printf("\nInterface name received in callback is different from the value set");
                                                  tet_printf("Interface name received in callback is different from the value set");
                                                  tet_result(TET_FAIL);
                                                }  
                                            }
                                          else
                                            {
                                              printf("\nTest Case: FAILED");
                                              printf("\nInterface description received in callback is different from the value set");
                                              tet_printf("Interface description received in callback is different from the value set");
                                              tet_result(TET_FAIL);
                                            }  
                                        }
                                      else
                                        {
                                          printf("\nTest Case: FAILED");
                                          printf("\nBit map attributes received in callback is different from the value set");
                                          tet_printf("Bit map attributes in callback is different from the value set");
                                          tet_result(TET_FAIL);
                                        }
                                    }
                                  else
                                    {
                                      printf("\nTest Case: FAILED");
                                      printf("\nSubscribe scope received in callback is different from the value set");
                                      tet_printf("Subscriber scope received in callback is different from the value set");
                                      tet_result(TET_FAIL);
                                    } 
                                }
                              else
                                {
                                  printf("\nTest Case: FAILED");
                                  printf("\nType received in callback is different from the value set");
                                  tet_printf("Type received in callback is different from the value set");
                                  tet_result(TET_FAIL);
                                }
                            }
                          else
                            {
                              printf("\nTest Case: FAILED");
                              printf("\nPort received in callback is different from the value set");
                              tet_printf("Port received in callback is different from the value set");
                              tet_result(TET_FAIL);
                            }   
                        }  
                      else
                        {
                          printf("\nTest Case: FAILED");
                          printf("\nSlot ID received in callback is different from the value set");
                          tet_printf("Slot ID received in callback is different from the value set");
                          tet_result(TET_FAIL);
                        }      
                    }
                  else
                    {
                      printf("\nTest Case: FAILED");
                      printf("\nShelf ID received in callback is different from the value set");
                      tet_printf("Shelf ID received in callback is different from the value set");
                      tet_result(TET_FAIL);
                    }
#endif
                }
            }
          ptr=ptr->next;
        }
      break;

    case NCS_IPXS_REC_DEL_NTFY:

      printf("\n\tCallback to notify interface Deletion");
      printf("\n********************\n");
      printf("\nInterface\n");      
      printf("--------------------");
      printf("\n\nifIndex: %d",rsp->info.del.i_ipinfo->if_index);
      printf("\nShelf Id: %d",rsp->info.del.i_ipinfo->spt.shelf);
      printf("\nSlot Id: %d",rsp->info.del.i_ipinfo->spt.slot);
      printf("\nPort : %d",rsp->info.del.i_ipinfo->spt.port);
      printf("\nType : %d",rsp->info.del.i_ipinfo->spt.type);
      printf("\nSubscriber Scope : %d",
             rsp->info.del.i_ipinfo->spt.subscr_scope);
      printf("\nInterface local on this node: %d",
             rsp->info.del.i_ipinfo->is_lcl_intf);
      printf("\n\nInterface attribute information\n");      
      printf("--------------------");
      printf("\nAttribute Map Flag: %d",rsp->info.del.i_ipinfo->if_info.if_am);
      printf("\nInterface Description: %s",
             rsp->info.del.i_ipinfo->if_info.if_descr);
      printf("\nInterface Name: %s",rsp->info.del.i_ipinfo->if_info.if_name);
      printf("\nMTU: %d",rsp->info.del.i_ipinfo->if_info.mtu);
      printf("\nIf Speed: %d",rsp->info.del.i_ipinfo->if_info.if_speed);
      printf("\nAdmin State: %d",rsp->info.del.i_ipinfo->if_info.admin_state);
      printf("\nOper State: %d",rsp->info.del.i_ipinfo->if_info.oper_state);
      printf("\n\nIP attribute information\n");      
      printf("--------------------");
      printf("\n\nAttribute bit map: %d",
             rsp->info.del.i_ipinfo->ip_info.ip_attr);
      printf("\nIPV4 numbered flag: %d",
             rsp->info.del.i_ipinfo->ip_info.is_v4_unnmbrd);
      if(rsp->info.del.i_ipinfo->ip_info.addip_list)
        {
          printf("\nIP address type(added IP): %d",
                 rsp->info.del.i_ipinfo->ip_info.addip_list->ipaddr.ipaddr.type);
          printf("\nBit mask length(added IP): %d",
                 rsp->info.del.i_ipinfo->ip_info.addip_list->ipaddr.mask_len);
          printf("\nIP address(added) count list: %d",
                 rsp->info.del.i_ipinfo->ip_info.addip_cnt);
        }
      if(rsp->info.del.i_ipinfo->ip_info.delip_list)
        {
          printf("\nIP address type(deleted IP): %d",
                 rsp->info.del.i_ipinfo->ip_info.delip_list->ipaddr.type);
          in.s_addr=(rsp->info.del.i_ipinfo->ip_info.delip_list->ipaddr.info.v4);
          printf("\nIP address (deleted IP): %s",(inet_ntoa(in)));
          printf("\nBit mask length(deleted IP): %d",
                 rsp->info.del.i_ipinfo->ip_info.delip_list->mask_len);
          printf("\nIP address(deleted) count list: %d",
                 rsp->info.del.i_ipinfo->ip_info.delip_cnt);
        }
      printf("\nNode address at other end: %d",
             rsp->info.del.i_ipinfo->ip_info.rrtr_ipaddr.type);
      printf("\nRouter ID at other end: %d",
             rsp->info.del.i_ipinfo->ip_info.rrtr_rtr_id);
      printf("\nAutonomous system ID at other end: %d",
             rsp->info.del.i_ipinfo->ip_info.rrtr_as_id);
      printf("\nifIndex at other end: %d",
             rsp->info.del.i_ipinfo->ip_info.rrtr_if_id);
      printf("\nUser defined: %d",
             rsp->info.del.i_ipinfo->ip_info.ud_1);
      printf("\n\nInterface deleted successfully");
      printf("\n\n*******************");  
#if 0 
      free(rsp->info.del.i_ipinfo);
#endif
      break;

    case NCS_IPXS_REC_UPD_NTFY:

      gl_cbk=1;
      printf("\n\tCallback to notify interface Updation");
      printf("\n********************\n");
      printf("\nInterface\n");      
      printf("--------------------");
      printf("\n\nifIndex: %d",rsp->info.upd.i_ipinfo->if_index);
      printf("\nShelf Id: %d",rsp->info.upd.i_ipinfo->spt.shelf);
      printf("\nSlot Id: %d",rsp->info.upd.i_ipinfo->spt.slot);
      printf("\nPort : %d",rsp->info.upd.i_ipinfo->spt.port);
      printf("\nType : %d",rsp->info.upd.i_ipinfo->spt.type);
      printf("\nSubscriber Scope : %d",
             rsp->info.upd.i_ipinfo->spt.subscr_scope);
      printf("\nInterface local on this node: %d",
             rsp->info.upd.i_ipinfo->is_lcl_intf);
      printf("\n\nInterface attribute information\n");      
      printf("--------------------");
      printf("\nAttribute Map Flag: %d",rsp->info.upd.i_ipinfo->if_info.if_am);
      printf("\nInterface Description: %s",
             rsp->info.upd.i_ipinfo->if_info.if_descr);
      printf("\nInterface Name: %s",rsp->info.upd.i_ipinfo->if_info.if_name);
      printf("\nMTU: %d",rsp->info.upd.i_ipinfo->if_info.mtu);
      printf("\nIf Speed: %d",rsp->info.upd.i_ipinfo->if_info.if_speed);
      printf("\nAdmin State: %d",rsp->info.upd.i_ipinfo->if_info.admin_state);
      printf("\nOper State: %d",rsp->info.upd.i_ipinfo->if_info.oper_state);
      printf("\n\nIP attribute information\n");      
      printf("--------------------");
      printf("\n\nAttribute bit map: %d",
             rsp->info.upd.i_ipinfo->ip_info.ip_attr);
      printf("\nIPV4 numbered flag: %d",
             rsp->info.upd.i_ipinfo->ip_info.is_v4_unnmbrd);
      if(rsp->info.upd.i_ipinfo->ip_info.addip_list)
        {
          printf("\nIP address type(added IP): %d",
                 rsp->info.upd.i_ipinfo->ip_info.addip_list->ipaddr.ipaddr.type);
          in.s_addr=(rsp->info.upd.i_ipinfo->ip_info.addip_list->ipaddr.ipaddr.info.v4);       
          printf("\nIP address: %s",inet_ntoa(in));
          printf("\nBit mask length(added IP): %d",
                 rsp->info.upd.i_ipinfo->ip_info.addip_list->ipaddr.mask_len);
          printf("\nIP address(added) count list: %d",
                 rsp->info.upd.i_ipinfo->ip_info.addip_cnt);
        }
#if 0
      if(rsp->info.upd.i_ipinfo->ip_info.delip_list)
        {
          printf("\nIP address type(deleted IP): %d",
                 rsp->info.upd.i_ipinfo->ip_info.delip_list->ipaddr.type);
          printf("\nBit mask length(deleted IP): %d",
                 rsp->info.upd.i_ipinfo->ip_info.delip_list->mask_len);
          printf("\nIP address(deleted) count list: %d",
                 rsp->info.upd.i_ipinfo->ip_info.delip_cnt);
        }
#endif
      printf("\nNode address at other end: %d",
             rsp->info.upd.i_ipinfo->ip_info.rrtr_ipaddr.type);
      printf("\nRouter ID at other end: %d",
             rsp->info.upd.i_ipinfo->ip_info.rrtr_rtr_id);
      printf("\nAutonomous system ID at other end: %d",
             rsp->info.upd.i_ipinfo->ip_info.rrtr_as_id);
      printf("\nifIndex at other end: %d",
             rsp->info.upd.i_ipinfo->ip_info.rrtr_if_id);
      printf("\nUser defined: %d",rsp->info.upd.i_ipinfo->ip_info.ud_1);
      printf("\n\nInterface updated successfully");
      printf("\n\n*******************");  

#if 0
      free(rsp->info.upd.i_ipinfo);
#endif
      break;
    }   
  printf("\n------Leaving ipxs_cb : Callback Exited-----\n");
  fflush(stdout);
} 
/*********************************************************************/
/***********************   IPXS WRAPPERS *****************************/
/*********************************************************************/
struct ipxs_subscriber ipxs_subscribe_api[]={
  [IPXS_SUBSCRIBE_INVALID_REQUEST_TYPE]  = {24,&i_subcr},
  [IPXS_SUBSCRIBE_NULL_CALLBACKS]        = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_NULL_SUBSCRIPTION_ATTRIBUTE_BITMAP] = {NCS_IPXS_REQ_SUBSCR,
                                                         &i_subcr},
  [IPXS_SUBSCRIBE_INVALID_SUBSCRIPTION_ATTRIBUTE_BITMAP] ={NCS_IPXS_REQ_SUBSCR,
                                                           &i_subcr},
  [IPXS_SUBSCRIBE_ADD_IFACE]                  = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_RMV_IFACE]                  = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_UPD_IFACE]                  = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_SCOPE_NODE]                 = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_SCOPE_CLUSTER]              = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_NULL_ATTRIBUTES_MAP]        = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_INVALID_ATTRIBUTES_MAP]     = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IAM_MTU]                    = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IAM_IFSPEED]                = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IAM_PHYADDR]                = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IAM_ADMSTATE]               = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IAM_OPRSTATE]               = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IAM_NAME]                   = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IAM_DESCR]                  = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IAM_LAST_CHNG]              = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IAM_SVDEST]                 = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_NULL_IP_ATTRIBUTES_MAP]     = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_INVALID_IP_ATTRIBUTES_MAP]  = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IPAM_ADDR]                  = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IPAM_UNNMBD]                = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IPAM_RRTRID]                = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IPAM_RMTRID]                = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IPAM_RMT_AS]                = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IPAM_RMTIFID]               = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_IPAM_UD_1]                  = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_SUCCESS]                    = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_ADD_SUCCESS]                = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_UPD_SUCCESS]                = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
  [IPXS_SUBSCRIBE_RMV_SUCCESS]                = {NCS_IPXS_REQ_SUBSCR,&i_subcr},
};

struct ipxs_unsubscriber ipxs_unsubscribe_api[]={
  [IPXS_UNSUBSCRIBE_NULL_HANDLE]        = {NCS_IPXS_REQ_UNSUBSCR,&i_unsubcr},  
  [IPXS_UNSUBSCRIBE_INVALID_HANDLE]     = {NCS_IPXS_REQ_UNSUBSCR,&i_unsubcr}, 
  [IPXS_UNSUBSCRIBE_DUPLICATE_HANDLE]   = {NCS_IPXS_REQ_UNSUBSCR,&i_unsubcr},
  [IPXS_UNSUBSCRIBE_SUCCESS]            = {NCS_IPXS_REQ_UNSUBSCR,&i_unsubcr},
};

struct ipxs_ipinfo_gett ipxs_ipinfo_get_api[]={
  [IPXS_IPINFO_GET_NULL_PARAM]       = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_INVALID_KEY]      = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_KEY_IFINDEX]      = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_INVALID_TYPE]     = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_INTF_OTHER]       = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_INTF_LOOPBACK]    = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_INTF_ETHERNET]    = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_INTF_TUNNEL]      = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_INVALID_SUBSCR_SCOPE]    = {NCS_IPXS_REQ_IPINFO_GET,
                                               &i_ipinfo_get},
  [IPXS_IPINFO_GET_SUBSCR_EXT]       = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_SUBSCR_INT]       = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_INVALID_ADDR_TYPE]= {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_ADDR_TYPE_NONE]   = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_ADDR_TYPE_IPV4]   = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_ADDR_TYPE_IPV6]   = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_INVALID_RESP_TYPE]= {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_SYNC_RESP_TYPE]   = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_ASYNC_RESP_TYPE]  = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_IFINDEX_SUCCESS]  = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_SPT_SUCCESS]      = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_SYNC_IFINDEX_SUCCESS]    = {NCS_IPXS_REQ_IPINFO_GET,
                                               &i_ipinfo_get},
  [IPXS_IPINFO_GET_SYNC_SPT_SUCCESS] = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_IPADDR_SUCCESS]   = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
  [IPXS_IPINFO_GET_SYNC_IPADDR_SUCCESS]     = {NCS_IPXS_REQ_IPINFO_GET,
                                               &i_ipinfo_get},
  [IPXS_IPINFO_GET_NULL_CALLBACKS]   = {NCS_IPXS_REQ_IPINFO_GET,&i_ipinfo_get},
};

struct ipxs_ipinfo_sett ipxs_ipinfo_set_api[]={
  [IPXS_IPINFO_SET_INVALID_IFINDEX]  = {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
  [IPXS_IPINFO_SET_NULL_IP_ATTRIBUTE_MAP] = {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},   
  [IPXS_IPINFO_SET_INVALID_IP_ATTRIBUTE_MAP]={NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},   
  [IPXS_IPINFO_SET_IPAM_ADDR]        = {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
  [IPXS_IPINFO_SET_IPAM_UNNMBD]      = {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
  [IPXS_IPINFO_SET_IPAM_RRTRID]      = {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
  [IPXS_IPINFO_SET_IPAM_RMTRID]      = {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
  [IPXS_IPINFO_SET_IPAM_RMT_AS]      = {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
  [IPXS_IPINFO_SET_IPAM_RMTIFID]     = {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
  [IPXS_IPINFO_SET_IPAM_UD_1]        = {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
  [IPXS_IPINFO_SET_IPV4_NUMB_FLAG]   = {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
  [IPXS_IPINFO_SET_INVALID_IP_ADDR_TYPE]={NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},   
  [IPXS_IPINFO_SET_IP_ADDR_TYPE_NONE]= {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
  [IPXS_IPINFO_SET_IP_ADDR_TYPE_IPV4]= {NCS_IPXS_REQ_IPINFO_SET,&i_ipinfo_set},
};

struct ipxs_i_is_local ipxs_i_is_local_api[]={
  [IPXS_I_IS_LOCAL_NULL_PARAM]           = {NCS_IPXS_REQ_IS_LOCAL,&i_is_local},
  [IPXS_I_IS_LOCAL_INVALID_IPADDRESS_TYPE]={NCS_IPXS_REQ_IS_LOCAL,&i_is_local},
  [IPXS_I_IS_LOCAL_IPADDRESS_IPV4]       = {NCS_IPXS_REQ_IS_LOCAL,&i_is_local},
  [IPXS_I_IS_LOCAL_SUCCESS]              = {NCS_IPXS_REQ_IS_LOCAL,&i_is_local},
};

struct ipxs_get_node_rec ipxs_get_node_rec_api[]={
  [IPXS_GET_NODE_REC_NULL_PARAM]    = {NCS_IPXS_REQ_NODE_INFO_GET,&i_node_rec},
  [IPXS_GET_NODE_REC_INVALID_NDATTR]= {NCS_IPXS_REQ_NODE_INFO_GET,&i_node_rec},
  [IPXS_GET_NODE_REC_NDATTR_RTR_ID] = {NCS_IPXS_REQ_NODE_INFO_GET,&i_node_rec},
  [IPXS_GET_NODE_REC_NDATTR_AS_ID]  = {NCS_IPXS_REQ_NODE_INFO_GET,&i_node_rec},
  [IPXS_GET_NODE_REC_NDATTR_LBIA_ID]= {NCS_IPXS_REQ_NODE_INFO_GET,&i_node_rec},
  [IPXS_GET_NODE_REC_NDATTR_UD_1]   = {NCS_IPXS_REQ_NODE_INFO_GET,&i_node_rec},
  [IPXS_GET_NODE_REC_NDATTR_UD_2]   = {NCS_IPXS_REQ_NODE_INFO_GET,&i_node_rec},
  [IPXS_GET_NODE_REC_NDATTR_UD_3]   = {NCS_IPXS_REQ_NODE_INFO_GET,&i_node_rec},
  [IPXS_GET_NODE_REC_SUCCESS]       = {NCS_IPXS_REQ_NODE_INFO_GET,&i_node_rec},
};
/**** ipxs wrappers *******/
void ipxs_subscribe_fill(int i)
{
  if(i>2)
    {
      i_subcr.i_ipxs_cb=(NCS_IPXS_SUBSCR_CB)ipxs_cb;
    }
  if(i>9)
    {
      i_subcr.i_subcr_flag=NCS_IPXS_ADD_IFACE|NCS_IPXS_RMV_IFACE
        |NCS_IPXS_UPD_IFACE;
    }
  if(i>20)
    {
      i_subcr.i_ifattr=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED
        |NCS_IFSV_IAM_PHYADDR|NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE
        |NCS_IFSV_IAM_NAME|NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG
        |NCS_IFSV_IAM_SVDEST;
    }
  switch(i)
    {
    case 1:

      break;

    case 2:

      i_subcr.i_ipxs_cb=(NCS_IPXS_SUBSCR_CB)(NULL);
      i_subcr.i_subcr_flag=NCS_IPXS_ADD_IFACE|NCS_IPXS_RMV_IFACE
        |NCS_IPXS_UPD_IFACE;
      i_subcr.i_ifattr=NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED
        |NCS_IFSV_IAM_PHYADDR|NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE
        |NCS_IFSV_IAM_NAME|NCS_IFSV_IAM_DESCR|NCS_IFSV_IAM_LAST_CHNG
        |NCS_IFSV_IAM_SVDEST;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR|NCS_IPXS_IPAM_UNNMBD
        |NCS_IPXS_IPAM_RRTRID|NCS_IPXS_IPAM_RMTRID|NCS_IPXS_IPAM_RMT_AS
        |NCS_IPXS_IPAM_RMTIFID|NCS_IPXS_IPAM_UD_1;
      break;

    case 3:

      i_subcr.i_subcr_flag=0;     
      break;

    case 4:
 
      i_subcr.i_subcr_flag=256;     
      break;

    case 5:
 
      i_subcr.i_subcr_flag=NCS_IPXS_ADD_IFACE;     
      i_subcr.i_ifattr=NCS_IFSV_IAM_MTU;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break;

    case 6:
 
      i_subcr.i_subcr_flag=NCS_IPXS_RMV_IFACE;     
      i_subcr.i_ifattr=NCS_IFSV_IAM_MTU;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break;

    case 7:
 
      i_subcr.i_subcr_flag=NCS_IPXS_UPD_IFACE;     
      i_subcr.i_ifattr=NCS_IFSV_IAM_MTU;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break;

    case 8:
 
      i_subcr.i_subcr_flag=NCS_IPXS_SCOPE_NODE;     
      i_subcr.i_ifattr=NCS_IFSV_IAM_MTU;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break;

    case 9:
 
      i_subcr.i_subcr_flag=NCS_IPXS_SCOPE_CLUSTER;     
      i_subcr.i_ifattr=NCS_IFSV_IAM_MTU;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break;

    case 10:

      i_subcr.i_ifattr=0;
      break; 

    case 11:

      i_subcr.i_ifattr=1024;
      break; 

    case 12:

      i_subcr.i_ifattr=NCS_IFSV_IAM_MTU;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break; 

    case 13:

      i_subcr.i_ifattr=NCS_IFSV_IAM_IFSPEED;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break; 

    case 14:

      i_subcr.i_ifattr=NCS_IFSV_IAM_PHYADDR;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break; 

    case 15:

      i_subcr.i_ifattr=NCS_IFSV_IAM_ADMSTATE;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break; 

    case 16:

      i_subcr.i_ifattr=NCS_IFSV_IAM_OPRSTATE;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break; 

    case 17:

      i_subcr.i_ifattr=NCS_IFSV_IAM_NAME;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break; 

    case 18:

      i_subcr.i_ifattr=NCS_IFSV_IAM_DESCR;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break; 

    case 19:

      i_subcr.i_ifattr=NCS_IFSV_IAM_LAST_CHNG;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break; 

    case 20:

      i_subcr.i_ifattr=NCS_IFSV_IAM_SVDEST;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break; 

    case 21:

      i_subcr.i_ipattr=0;
      break;
 
    case 22:

      i_subcr.i_ipattr=256;
      break;
 
    case 23:

      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR;
      break;
 
    case 24:

      i_subcr.i_ipattr=NCS_IPXS_IPAM_UNNMBD;
      break;
 
    case 25:

      i_subcr.i_ipattr=NCS_IPXS_IPAM_RRTRID;
      break;
 
    case 26:

      i_subcr.i_ipattr=NCS_IPXS_IPAM_RMTRID;
      break;
 
    case 27:

      i_subcr.i_ipattr=NCS_IPXS_IPAM_RMT_AS;
      break;
 
    case 28:

      i_subcr.i_ipattr=NCS_IPXS_IPAM_RMTIFID;
      break;
 
    case 29:

      i_subcr.i_ipattr=NCS_IPXS_IPAM_UD_1;
      break;

    case 30:
  
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR|NCS_IPXS_IPAM_UNNMBD
        |NCS_IPXS_IPAM_RRTRID|NCS_IPXS_IPAM_RMTRID|NCS_IPXS_IPAM_RMT_AS
        |NCS_IPXS_IPAM_RMTIFID|NCS_IPXS_IPAM_UD_1;
      break;

    case 31:

      i_subcr.i_subcr_flag=NCS_IPXS_ADD_IFACE;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR|NCS_IPXS_IPAM_UNNMBD
        |NCS_IPXS_IPAM_RRTRID|NCS_IPXS_IPAM_RMTRID|NCS_IPXS_IPAM_RMT_AS
        |NCS_IPXS_IPAM_RMTIFID|NCS_IPXS_IPAM_UD_1;
      break;

    case 32:

      i_subcr.i_subcr_flag=NCS_IPXS_UPD_IFACE;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR|NCS_IPXS_IPAM_UNNMBD
        |NCS_IPXS_IPAM_RRTRID|NCS_IPXS_IPAM_RMTRID|NCS_IPXS_IPAM_RMT_AS
        |NCS_IPXS_IPAM_RMTIFID|NCS_IPXS_IPAM_UD_1;
      break;

    case 33:

      i_subcr.i_subcr_flag=NCS_IPXS_RMV_IFACE;
      i_subcr.i_ipattr=NCS_IPXS_IPAM_ADDR|NCS_IPXS_IPAM_UNNMBD
        |NCS_IPXS_IPAM_RRTRID|NCS_IPXS_IPAM_RMTRID|NCS_IPXS_IPAM_RMT_AS
        |NCS_IPXS_IPAM_RMTIFID|NCS_IPXS_IPAM_UD_1;
      break;
 
    default:

      break;
    }
}

void ipxs_subscription(int i)
{
  ipxs_subscribe_fill(i);

  info.i_type=ipxs_subscribe_api[i].i_type;                   
   
  info.info.i_subcr.i_ipxs_cb=*ipxs_subscribe_api[i].i_subcr->i_ipxs_cb;
  info.info.i_subcr.i_subcr_flag=ipxs_subscribe_api[i].i_subcr->i_subcr_flag;
  info.info.i_subcr.i_ifattr=ipxs_subscribe_api[i].i_subcr->i_ifattr;
  info.info.i_subcr.i_ipattr=ipxs_subscribe_api[i].i_subcr->i_ipattr;
  
  gl_rc=ncs_ipxs_svc_req(&info);
  if(gl_rc==NCSCC_RC_SUCCESS)
    {
      printf("\n\nSubscription Handle : %u",info.info.i_subcr.o_subr_hdl); 
      gl_subscription_hdl=info.info.i_subcr.o_subr_hdl;
    }
}

void ipxs_unsubscribe_fill(int i)
{
  switch(i)
    {
    case 1:
        
      i_unsubcr.i_subr_hdl=0; 
      break;
 
    case 2:
        
      i_unsubcr.i_subr_hdl=24;
      break;

    case 3:
 
      i_unsubcr.i_subr_hdl=info.info.i_subcr.o_subr_hdl;
      break;

    case 4:
 
      i_unsubcr.i_subr_hdl=info.info.i_subcr.o_subr_hdl;
      break;

    default:
        
      break;
    }               
}

void ipxs_unsubscription(int i)
{
  ipxs_unsubscribe_fill(i);

  info.i_type=ipxs_unsubscribe_api[i].i_type;

  info.info.i_unsubr.i_subr_hdl=ipxs_unsubscribe_api[i].i_unsubcr->i_subr_hdl;

  printf("\n\nSubscription Handle: %u",info.info.i_unsubr.i_subr_hdl);
  gl_rc=ncs_ipxs_svc_req(&info);
}

void ipxs_ipinfo_get_fill(int i)
{
  i_ipinfo_get.i_rsp_type=NCS_IFSV_GET_RESP_ASYNC;
  i_ipinfo_get.i_subr_hdl=info.info.i_subcr.o_subr_hdl;
  switch(i)
    {
    case 1:
      
      break;

    case 2:
 
      i_ipinfo_get.i_key.type=256;
      break;   

    case 3:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IFINDEX;
      i_ipinfo_get.i_key.info.iface=gl_ifIndex;
      break;  

    case 4:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_SPT;
      i_ipinfo_get.i_key.info.spt.shelf=shelf_id;
      i_ipinfo_get.i_key.info.spt.slot=slot_id;
      i_ipinfo_get.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ipinfo_get.i_key.info.spt.type=256; 
      i_ipinfo_get.i_key.info.spt.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      break;   

    case 5:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_SPT;
      i_ipinfo_get.i_key.info.spt.shelf=shelf_id;
      i_ipinfo_get.i_key.info.spt.slot=slot_id;
      i_ipinfo_get.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ipinfo_get.i_key.info.spt.type=NCS_IFSV_INTF_OTHER; 
      i_ipinfo_get.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type; 
      i_ipinfo_get.i_key.info.spt.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      break;   

    case 6:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_SPT;
      i_ipinfo_get.i_key.info.spt.shelf=shelf_id;
      i_ipinfo_get.i_key.info.spt.slot=slot_id;
      i_ipinfo_get.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ipinfo_get.i_key.info.spt.type=NCS_IFSV_INTF_LOOPBACK; 
      i_ipinfo_get.i_key.info.spt.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      break;   

    case 7:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_SPT;
      i_ipinfo_get.i_key.info.spt.shelf=shelf_id;
      i_ipinfo_get.i_key.info.spt.slot=slot_id;
      i_ipinfo_get.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ipinfo_get.i_key.info.spt.type=NCS_IFSV_INTF_ETHERNET; 
      i_ipinfo_get.i_key.info.spt.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      break;   

    case 8:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_SPT;
      i_ipinfo_get.i_key.info.spt.shelf=shelf_id;
      i_ipinfo_get.i_key.info.spt.slot=slot_id;
      i_ipinfo_get.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ipinfo_get.i_key.info.spt.type=NCS_IFSV_INTF_TUNNEL; 
      i_ipinfo_get.i_key.info.spt.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      break;   

    case 9:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_SPT;
      i_ipinfo_get.i_key.info.spt.shelf=shelf_id;
      i_ipinfo_get.i_key.info.spt.slot=slot_id;
      i_ipinfo_get.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ipinfo_get.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type; 
      i_ipinfo_get.i_key.info.spt.subscr_scope=256;
      break;   

    case 10:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_SPT;
      i_ipinfo_get.i_key.info.spt.shelf=shelf_id;
      i_ipinfo_get.i_key.info.spt.slot=slot_id;
      i_ipinfo_get.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ipinfo_get.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type; 
      i_ipinfo_get.i_key.info.spt.subscr_scope=NCS_IFSV_SUBSCR_EXT;
      break;   

    case 11:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_SPT;
      i_ipinfo_get.i_key.info.spt.shelf=shelf_id;
      i_ipinfo_get.i_key.info.spt.slot=slot_id;
      i_ipinfo_get.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ipinfo_get.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type; 
      i_ipinfo_get.i_key.info.spt.subscr_scope=NCS_IFSV_SUBSCR_INT;
      break;   

    case 12:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IP_ADDR;
      i_ipinfo_get.i_key.info.ip_addr.type=256;
      break;   

    case 13:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IP_ADDR;
      i_ipinfo_get.i_key.info.ip_addr.type=NCS_IP_ADDR_TYPE_NONE;
      break;   

    case 14:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IP_ADDR;
      i_ipinfo_get.i_key.info.ip_addr.type=NCS_IP_ADDR_TYPE_IPV4;
      break;   

    case 15:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IP_ADDR;
      i_ipinfo_get.i_key.info.ip_addr.type=NCS_IP_ADDR_TYPE_IPV6;
      break;  

    case 16:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IFINDEX;
      i_ipinfo_get.i_key.info.iface=gl_ifIndex;
      i_ipinfo_get.i_rsp_type=256;
      break;  

    case 17:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IFINDEX;
      i_ipinfo_get.i_key.info.iface=gl_ifIndex;
      i_ipinfo_get.i_rsp_type=NCS_IFSV_GET_RESP_SYNC;

    case 18:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IFINDEX;
      i_ipinfo_get.i_key.info.iface=gl_ifIndex;

    case 19:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IFINDEX;
      i_ipinfo_get.i_key.info.iface=gl_ifIndex;
      break;  

    case 20:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_SPT;
      i_ipinfo_get.i_key.info.spt.shelf=shelf_id;
      i_ipinfo_get.i_key.info.spt.slot=slot_id;
      i_ipinfo_get.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ipinfo_get.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type; 
      i_ipinfo_get.i_key.info.spt.subscr_scope=
        req_info.info.i_ifadd.spt_info.subscr_scope;
      break;   

    case 21:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IFINDEX;
      i_ipinfo_get.i_key.info.iface=gl_ifIndex;
      i_ipinfo_get.i_rsp_type=NCS_IFSV_GET_RESP_SYNC;
      break;  

    case 22:
 
      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_SPT;
      i_ipinfo_get.i_key.info.spt.shelf=shelf_id;
      i_ipinfo_get.i_key.info.spt.slot=slot_id;
      i_ipinfo_get.i_key.info.spt.port=req_info.info.i_ifadd.spt_info.port;
      i_ipinfo_get.i_key.info.spt.type=req_info.info.i_ifadd.spt_info.type; 
      i_ipinfo_get.i_key.info.spt.subscr_scope=
        req_info.info.i_ifadd.spt_info.subscr_scope;
      i_ipinfo_get.i_rsp_type=NCS_IFSV_GET_RESP_SYNC;
      break;   

    case 23:

      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IP_ADDR;
      i_ipinfo_get.i_key.info.ip_addr.type=NCS_IP_ADDR_TYPE_IPV4;
      i_ipinfo_get.i_key.info.ip_addr.info.v4=
        info.info.i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.info.v4;
      break;

    case 24:

      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IP_ADDR;
      i_ipinfo_get.i_key.info.ip_addr.type=NCS_IP_ADDR_TYPE_IPV4;
      i_ipinfo_get.i_key.info.ip_addr.info.v4=
        info.info.i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.info.v4;
      i_ipinfo_get.i_rsp_type=NCS_IFSV_GET_RESP_SYNC;
#if 1
      i_ipinfo_get.o_rsp.err=0;
      i_ipinfo_get.o_rsp.ipinfo = NULL;
#endif
      break;

    case 25:

      i_ipinfo_get.i_key.type=NCS_IPXS_KEY_IFINDEX;
      i_ipinfo_get.i_key.info.iface=gl_ifIndex;
      i_ipinfo_get.i_rsp_type=NCS_IFSV_GET_RESP_ASYNC;
      break;

    default:
        
      break;
    }               
}

void ipxs_ipinfo_gett(int i)
{
  struct in_addr in;
  ipxs_ipinfo_get_fill(i);

  info.i_type=ipxs_ipinfo_get_api[i].i_type;

  info.info.i_ipinfo_get.i_key.type=
    ipxs_ipinfo_get_api[i].i_ipinfo_get->i_key.type;
   
  if(info.info.i_ipinfo_get.i_key.type==NCS_IPXS_KEY_IFINDEX)
    {
      info.info.i_ipinfo_get.i_key.info.iface=
        ipxs_ipinfo_get_api[i].i_ipinfo_get->i_key.info.iface;
    }
  else if(info.info.i_ipinfo_get.i_key.type==NCS_IPXS_KEY_SPT)
    {
      info.info.i_ipinfo_get.i_key.info.spt.shelf=
        ipxs_ipinfo_get_api[i].i_ipinfo_get->i_key.info.spt.shelf;
      info.info.i_ipinfo_get.i_key.info.spt.slot=
        ipxs_ipinfo_get_api[i].i_ipinfo_get->i_key.info.spt.slot;
      info.info.i_ipinfo_get.i_key.info.spt.port=
        ipxs_ipinfo_get_api[i].i_ipinfo_get->i_key.info.spt.port;
      info.info.i_ipinfo_get.i_key.info.spt.type=
        ipxs_ipinfo_get_api[i].i_ipinfo_get->i_key.info.spt.type;
      info.info.i_ipinfo_get.i_key.info.spt.subscr_scope=
        ipxs_ipinfo_get_api[i].i_ipinfo_get->i_key.info.spt.subscr_scope;
    }
  else
    {
      info.info.i_ipinfo_get.i_key.info.ip_addr.type=
        ipxs_ipinfo_get_api[i].i_ipinfo_get->i_key.info.ip_addr.type;
      info.info.i_ipinfo_get.i_key.info.ip_addr.info.v4=
        ipxs_ipinfo_get_api[i].i_ipinfo_get->i_key.info.ip_addr.info.v4;
    }

  info.info.i_ipinfo_get.i_rsp_type=
    ipxs_ipinfo_get_api[i].i_ipinfo_get->i_rsp_type;
  info.info.i_ipinfo_get.i_subr_hdl=gl_subscription_hdl;

  gl_cbk=0;
  gl_rc=ncs_ipxs_svc_req(&info);
  sleep(2);
  if(info.info.i_ipinfo_get.i_rsp_type==NCS_IFSV_GET_RESP_SYNC)
    {
      printf("\n********************\n");
      printf("\nReturn Code: %d",info.info.i_ipinfo_get.o_rsp.err);
      gl_error=info.info.i_ipinfo_get.o_rsp.err;
      if((info.info.i_ipinfo_get.o_rsp.ipinfo)
         &&(gl_error!=NCS_IFSV_IFGET_RSP_NO_REC))
        {
          printf("\n\nInterface\n");
          printf("--------------------");
          printf("\n\nifIndex: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->if_index);
          printf("\nShelf Id: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->spt.shelf);
          printf("\nSlot Id: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->spt.slot);
          printf("\nPort : %d",info.info.i_ipinfo_get.o_rsp.ipinfo->spt.port);
          printf("\nType : %d",info.info.i_ipinfo_get.o_rsp.ipinfo->spt.type);
          printf("\nSubscriber Scope : %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->spt.subscr_scope);
          printf("\nInterface local on this node: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->is_lcl_intf);
          printf("\n\nInterface attribute information\n");
          printf("--------------------");
          printf("\nAttribute Map Flag: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->if_info.if_am);
          printf("\nInterface Description: %s",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->if_info.if_descr);
          printf("\nInterface Name: %s",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->if_info.if_name);
          printf("\nMTU: %d",info.info.i_ipinfo_get.o_rsp.ipinfo->if_info.mtu);
          printf("\nIf Speed: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->if_info.if_speed);
          printf("\nAdmin State: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->if_info.admin_state);
          printf("\nOper State: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->if_info.oper_state);
          printf("\n\nIP attribute information\n");
          printf("--------------------");
          printf("\n\nAttribute bit map: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.ip_attr);
          printf("\nIPV4 numbered flag: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.is_v4_unnmbrd);
          if(info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.addip_list)
            {
              printf("\nIP address type(added IP): %d",
                     info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.addip_list->ipaddr.ipaddr.type);
              in.s_addr=(info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.addip_list->ipaddr.ipaddr.info.v4);
              printf("\nIP address: %s",inet_ntoa(in));
              printf("\nBit mask length(added IP): %d",
                     info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.addip_list->ipaddr.mask_len);
              printf("\nIP address(added) count list: %d",
                     info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.addip_cnt);
            }
          if(info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.delip_list)
            {
              printf("\nIP address type(deleted IP): %d",
                     info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.delip_list->ipaddr.type);
              printf("\nBit mask length(deleted IP): %d",
                     info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.delip_list->mask_len);
              printf("\nIP address(deleted) count list: %d",
                     info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.delip_cnt);
            }
          printf("\nNode address at other end: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.rrtr_ipaddr.type);
          printf("\nRouter ID at other end: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.rrtr_rtr_id);
          printf("\nAutonomous system ID at other end: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.rrtr_as_id);
          printf("\nifIndex at other end: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.rrtr_if_id);
          printf("\nUser defined: %d",
                 info.info.i_ipinfo_get.o_rsp.ipinfo->ip_info.ud_1);
          printf("\n\n*******************");
#if 1
          gl_rc=ncs_ipxs_mem_free_req(&info);
#endif
        }
    }   
  else
    {
      if(gl_cbk==0&&gl_rc==1)
        {
          printf("\nCallback not invoked to get interface information");
          tet_printf("Callback not invoked to get interface information");
          tet_result(TET_FAIL); 
        }
    }
}

void ipxs_ipinfo_set_fill(int i)
{
  char ip_str[]="2.2.2.2";
  NCS_IPV4_ADDR addr;
  addr = inet_addr(ip_str);
   
  i_ipinfo_set.ip_info.addip_list=
    (IPXS_IFIP_IP_INFO *)malloc(sizeof(IPXS_IFIP_IP_INFO));
  i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.type=NCS_IP_ADDR_TYPE_IPV4;
  i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.info.v4=addr;
  i_ipinfo_set.ip_info.addip_cnt=1;
  if(i>1)
    {
      i_ipinfo_set.if_index=gl_ifIndex;
    }
  if(i>10)
    {
      i_ipinfo_set.ip_info.ip_attr=NCS_IPXS_IPAM_ADDR|NCS_IPXS_IPAM_UNNMBD
        |NCS_IPXS_IPAM_RRTRID|NCS_IPXS_IPAM_RMTRID|NCS_IPXS_IPAM_RMT_AS
        |NCS_IPXS_IPAM_RMTIFID|NCS_IPXS_IPAM_UD_1;
    }
  if(i>11)
    {
      i_ipinfo_set.ip_info.is_v4_unnmbrd=2;
    }
  switch(i)
    {
    case 1:
        
      i_ipinfo_set.if_index=0; 
      break;

    case 2:

      i_ipinfo_set.ip_info.ip_attr=0;
      break;

    case 3:

      i_ipinfo_set.ip_info.ip_attr=1024;
      break;

    case 4:

      i_ipinfo_set.ip_info.ip_attr=NCS_IPXS_IPAM_ADDR; 
      break;

    case 5:

      i_ipinfo_set.ip_info.ip_attr=NCS_IPXS_IPAM_UNNMBD; 
      break;

    case 6:

      i_ipinfo_set.ip_info.ip_attr=NCS_IPXS_IPAM_RRTRID; 
      break;

    case 7:

      i_ipinfo_set.ip_info.ip_attr=NCS_IPXS_IPAM_RMTRID; 
      break;

    case 8:

      i_ipinfo_set.ip_info.ip_attr=NCS_IPXS_IPAM_RMT_AS; 
      break;

    case 9:

      i_ipinfo_set.ip_info.ip_attr=NCS_IPXS_IPAM_RMTIFID; 
      break;

    case 10:

      i_ipinfo_set.ip_info.ip_attr=NCS_IPXS_IPAM_UD_1; 
      break;

    case 11:

      i_ipinfo_set.ip_info.is_v4_unnmbrd=1;
      break;

    case 12:

      i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.type=256;
      i_ipinfo_set.ip_info.addip_cnt=1;
      break;

    case 13:

      i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.type=
        NCS_IP_ADDR_TYPE_NONE;
      break;
 
    case 14:

      i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.type=
        NCS_IP_ADDR_TYPE_IPV4;
      i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.info.v4=addr;

      i_ipinfo_set.ip_info.addip_list->ipaddr.mask_len=24; 

      i_ipinfo_set.ip_info.addip_cnt=1;
      i_ipinfo_set.ip_info.ud_1=1;
      break;
 
    default:
 
      break;
    }
}

void ipxs_ipinfo_sett(int i)
{
  ipxs_ipinfo_set_fill(i);

  info.i_type=ipxs_ipinfo_set_api[i].i_type;

  info.info.i_ipinfo_set.if_index=ipxs_ipinfo_set_api[i].i_ipinfo_set->if_index;
  info.info.i_ipinfo_set.ip_info.ip_attr=
    ipxs_ipinfo_set_api[i].i_ipinfo_set->ip_info.ip_attr;
  info.info.i_ipinfo_set.ip_info.is_v4_unnmbrd=
    ipxs_ipinfo_set_api[i].i_ipinfo_set->ip_info.is_v4_unnmbrd;

  info.info.i_ipinfo_set.ip_info.addip_list=
    (IPXS_IFIP_IP_INFO *)malloc(sizeof(IPXS_IFIP_IP_INFO));
  info.info.i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.type=
    ipxs_ipinfo_set_api[i].i_ipinfo_set->ip_info.addip_list->ipaddr.ipaddr.type;
  info.info.i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.info.v4=
    ipxs_ipinfo_set_api[i].i_ipinfo_set->ip_info.addip_list->ipaddr.ipaddr.info.v4;
  info.info.i_ipinfo_set.ip_info.addip_list->ipaddr.mask_len=
    ipxs_ipinfo_set_api[i].i_ipinfo_set->ip_info.addip_list->ipaddr.mask_len;
  info.info.i_ipinfo_set.ip_info.addip_cnt=
    ipxs_ipinfo_set_api[i].i_ipinfo_set->ip_info.addip_cnt;
      
  info.info.i_ipinfo_set.ip_info.delip_list=
    (NCS_IPPFX *)malloc(sizeof(NCS_IPPFX));
  info.info.i_ipinfo_set.ip_info.delip_list->ipaddr.type=
    info.info.i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.type;
  info.info.i_ipinfo_set.ip_info.delip_list->ipaddr.info.v4=
    info.info.i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.info.v4;

  gl_cbk=0;
  gl_rc=ncs_ipxs_svc_req(&info);
  sleep(1);
  if(gl_cbk==0&&gl_rc==1)
    {
      printf("\nInterface not updated with IP address");
      tet_printf("Interface not updated with IP address");
      tet_result(TET_FAIL);
    }
}

void ipxs_i_is_local_fill(int i)
{
  switch(i)
    {
    case 1:
 
      break;

    case 2:

      i_is_local.i_ip_addr.type=256;
      break;

    case 3:

      i_is_local.i_ip_addr.type=NCS_IP_ADDR_TYPE_IPV4;
      i_is_local.i_ip_addr.info.v4=
        info.info.i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.info.v4;
      if(info.info.i_ipinfo_set.ip_info.addip_list)
        {
#if 0
          i_is_local.i_maskbits=
            info.info.i_ipinfo_set.ip_info.addip_list->mask_len;
#endif
        }
      break;  

    case 4:

      i_is_local.i_ip_addr.type=NCS_IP_ADDR_TYPE_IPV4;
      if(info.info.i_ipinfo_set.ip_info.addip_list)
        {
          i_is_local.i_ip_addr.info.v4=
            info.info.i_ipinfo_set.ip_info.addip_list->ipaddr.ipaddr.info.v4;
#if 0
          i_is_local.i_maskbits=
            info.info.i_ipinfo_set.ip_info.addip_list->mask_len;
#endif
        }
      break;  
 
    default:

      break;
    }
}

void ipxs_i_is_local(int i)
{
  ipxs_i_is_local_fill(i);
   
  info.i_type=ipxs_i_is_local_api[i].i_type;
   
  info.info.i_is_local.i_ip_addr.type=
    ipxs_i_is_local_api[i].i_is_local->i_ip_addr.type;  
  info.info.i_is_local.i_ip_addr.info.v4=
    ipxs_i_is_local_api[i].i_is_local->i_ip_addr.info.v4; 

  info.info.i_is_local.i_maskbits=
    ipxs_i_is_local_api[i].i_is_local->i_maskbits;
 
  info.info.i_is_local.i_obs=ipxs_i_is_local_api[i].i_is_local->i_obs;
 
  gl_error=0; 
  gl_rc=ncs_ipxs_svc_req(&info);
  if(gl_rc==NCSCC_RC_SUCCESS)
    {
      printf("\n\nInterface get information for IP address configured");
      printf("\nifIndex: %d",info.info.i_is_local.o_iface);
      gl_error=info.info.i_is_local.o_iface;
      printf("\nIndex local/remote: %d",info.info.i_is_local.o_answer);   
    }  
}

void ipxs_get_node_rec_fill(int i)
{
  switch(i)
    {
    case 1:

      break;

    case 2:
  
      i_node_rec.o_node_rec.ndattr=256;
      break;

    case 3:

      i_node_rec.o_node_rec.ndattr=NCS_IPXS_NAM_RTR_ID;
      break;

    case 4:
 
      i_node_rec.o_node_rec.ndattr=NCS_IPXS_NAM_AS_ID;
      break;

    case 5:
 
      i_node_rec.o_node_rec.ndattr=NCS_IPXS_NAM_LBIA_ID;
      break;

    case 6:
 
      i_node_rec.o_node_rec.ndattr=NCS_IPXS_NAM_UD1_ID;
      break;

    case 7:
 
      i_node_rec.o_node_rec.ndattr=NCS_IPXS_NAM_UD2_ID;
      break;

    case 8:
 
      i_node_rec.o_node_rec.ndattr=NCS_IPXS_NAM_UD3_ID;
      break;

    case 9:
 
      i_node_rec.o_node_rec.ndattr=NCS_IPXS_NAM_RTR_ID|NCS_IPXS_NAM_AS_ID
        |NCS_IPXS_NAM_LBIA_ID|NCS_IPXS_NAM_UD1_ID|NCS_IPXS_NAM_UD2_ID
        |NCS_IPXS_NAM_UD3_ID;
      break;

    default:
        
      break;
    }
}

void ipxs_get_node_rec(int i)
{
  struct in_addr in;
  ipxs_get_node_rec_fill(i);

  info.i_type=ipxs_get_node_rec_api[i].i_type;

  info.info.i_node_rec.o_node_rec.ndattr=
    ipxs_get_node_rec_api[i].i_node_rec->o_node_rec.ndattr;

  gl_error=0;
  gl_rc=ncs_ipxs_svc_req(&info);
  if(gl_rc==NCSCC_RC_SUCCESS)
    {
      printf("\n\nNode record information");
      printf("\nRouter id: %d",info.info.i_node_rec.o_node_rec.rtr_id);
      gl_error=info.info.i_node_rec.o_node_rec.rtr_id;
      in.s_addr=(info.info.i_node_rec.o_node_rec.lb_ia);
      printf("\nLoopback interface address: %s",inet_ntoa(in));
      printf("\nUser Defined: %d",info.info.i_node_rec.o_node_rec.ud_1);
      if(gl_error==0)
        {
          printf("\nNode record information (get) failed");
          tet_printf("Node record information (get) failed");
          tet_result(TET_FAIL);
        }
    }
}


