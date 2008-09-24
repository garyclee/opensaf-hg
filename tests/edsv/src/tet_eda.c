#include <string.h>
#include "tet_api.h"
#include "tet_startup.h"
#include "tet_eda.h"
void tet_edsv_startup(void);
void tet_edsv_cleanup(void);

extern int gl_sync_pointnum;
extern int fill_syncparameters(int);
extern int maa_switch(int);
/*Initializing Global Varibles*/
int gl_jCount=0;
int gl_err=1;
int gl_major_version=0x01;
int gl_minor_version=0x01;

SaEvtLimitIdT  gl_limitId;
SaLimitValueT  gl_limitValue;


SaTimeT gl_timeout=100000000000.0;
SaInvocationT gl_invocation=3;
SaEvtEventPatternT gl_pattern[2]=
  {
    {5,5,(SaUint8T *)"Hello"},
    {4,4,(SaUint8T *)"Moto"},
  };
SaEvtEventFilterT gl_filter[1]=
  {
    {3,{5,5,(SaUint8T *)"Hello"}}
  };
const char *gl_saf_error[] =
  {
    "SA_AIS_NOT_VALID",
    "SA_AIS_OK",
    "SA_AIS_ERR_LIBRARY",
    "SA_AIS_ERR_VERSION",
    "SA_AIS_ERR_INIT",
    "SA_AIS_ERR_TIMEOUT",
    "SA_AIS_ERR_TRY_AGAIN",
    "SA_AIS_ERR_INVALID_PARAM",
    "SA_AIS_ERR_NO_MEMORY",
    "SA_AIS_ERR_BAD_HANDLE",
    "SA_AIS_ERR_BUSY",
    "SA_AIS_ERR_ACCESS",
    "SA_AIS_ERR_NOT_EXIST",
    "SA_AIS_ERR_NAME_TOO_LONG",
    "SA_AIS_ERR_EXIST",
    "SA_AIS_ERR_NO_SPACE",
    "SA_AIS_ERR_INTERRUPT",
    "SA_AIS_ERR_NAME_NOT_FOUND",
    "SA_AIS_ERR_NO_RESOURCES",
    "SA_AIS_ERR_NOT_SUPPORTED",
    "SA_AIS_ERR_BAD_OPERATION",
    "SA_AIS_ERR_FAILED_OPERATION",
    "SA_AIS_ERR_MESSAGE_ERROR",
    "SA_AIS_ERR_QUEUE_FULL",
    "SA_AIS_ERR_QUEUE_NOT_AVAILABLE",
    "SA_AIS_ERR_BAD_FLAGS",
    "SA_AIS_ERR_TOO_BIG",
    "SA_AIS_ERR_NO_SECTIONS",
   /* B03 Additions */
    "SA_AIS_ERR_NO_OP",
    "SA_AIS_ERR_REPAIR_PENDING",
    "SA_AIS_ERR_NO_BINDINGS",
    "SA_AIS_ERR_UNAVAILABLE",
};

/***********************************************/
/*******************API Testing*****************/
/***********************************************/

/*******saEvtInitialize*******/

void tet_saEvtInitializeCases(int iOption)
{
  printf("\n---------- tet_saEvtInitializeCases : %d -------------",iOption);
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtInitialize(NULL,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with NULL event handle",
             SA_AIS_ERR_INVALID_PARAM);
      break;
      
    case 2:
      gl_version.releaseCode='B';
      gl_version.majorVersion=1;
      gl_version.minorVersion=1;

      gl_rc=saEvtInitialize(&gl_evtHandle,NULL,&gl_version);
      result("saEvtInitialize() with NULL pointer to callbacks",SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 3:

#if 0

      tet_eda_destroy();

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with library destroyed",SA_AIS_ERR_LIBRARY);

      tet_eda_create();

#endif

      break;
      
    case 4:
      gl_version.releaseCode='B';
      gl_version.majorVersion=1;
      gl_version.minorVersion=1;

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with uninitialized pointer to callbacks",
             SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 5:
      gl_version.releaseCode='B';
      gl_version.majorVersion=1;
      gl_version.minorVersion=1;
      gl_evtCallbacks.saEvtChannelOpenCallback=NULL;
      gl_evtCallbacks.saEvtEventDeliverCallback=NULL;

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with NULL saEvtChannelOpenCallback and NULL \
 saEvtEventDeliverCallback",SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;  
      
    case 6:
      gl_version.releaseCode='B';
      gl_version.majorVersion=1;
      gl_version.minorVersion=1;
      gl_evtCallbacks.saEvtChannelOpenCallback=NULL;
      gl_evtCallbacks.saEvtEventDeliverCallback=EvtDeliverCallback;

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with NULL saEvtChannelOpenCallback",SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 7:
      gl_version.releaseCode='B';
      gl_version.majorVersion=1;
      gl_version.minorVersion=1;
      gl_evtCallbacks.saEvtChannelOpenCallback=EvtOpenCallback;
      gl_evtCallbacks.saEvtEventDeliverCallback=NULL;

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with NULL saEvtEventDeliverCallback",
             SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 8:
      gl_evtCallbacks.saEvtChannelOpenCallback=EvtOpenCallback;
      gl_evtCallbacks.saEvtEventDeliverCallback=EvtDeliverCallback;            

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,NULL);
      result("saEvtInitialize() with NULL version",SA_AIS_ERR_INVALID_PARAM);
      break;
      
    case 9:
      gl_version.releaseCode=(SaUint8T)(long)NULL;
      gl_version.majorVersion=(SaUint8T)(long)NULL;
      gl_version.minorVersion=(SaUint8T)(long)NULL;

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with uninitialized version",
             SA_AIS_ERR_VERSION);

      printf("\nVersion Delivered : %c %d %d",gl_version.releaseCode, 
             gl_version.majorVersion, gl_version.minorVersion);
      tet_printf("Version Delivered : %c %d %d",gl_version.releaseCode, 
                 gl_version.majorVersion, gl_version.minorVersion);
      break;
      
    case 10:
      gl_version.releaseCode='A';
      gl_version.majorVersion=1;
      gl_version.minorVersion=1;

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with error version",SA_AIS_ERR_VERSION);

      printf("\nVersion Delivered : %c %d %d",gl_version.releaseCode,
             gl_version.majorVersion, gl_version.minorVersion);
      tet_printf("Version Delivered : %c %d %d",gl_version.releaseCode,
                 gl_version.majorVersion,
                 gl_version.minorVersion);
      break;
      
    case 11:
      gl_version.releaseCode='B';
      gl_version.majorVersion=1;
      gl_version.minorVersion=0;
      gl_evtCallbacks.saEvtChannelOpenCallback=EvtOpenCallback;
      gl_evtCallbacks.saEvtEventDeliverCallback=EvtDeliverCallback;

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with minor version set to 0",SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 12:
      gl_version.releaseCode='B';
      gl_version.majorVersion=3;
      gl_version.minorVersion=0;
      gl_evtCallbacks.saEvtChannelOpenCallback=EvtOpenCallback;
      gl_evtCallbacks.saEvtEventDeliverCallback=EvtDeliverCallback;

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with major version set to 3",
             SA_AIS_ERR_VERSION);

#if 0
      tet_saEvtFinalize(&gl_evtHandle);
#endif
      break;
      
    case 13:
      gl_version.releaseCode='B';
      gl_version.majorVersion=0;
      gl_version.minorVersion=0;
      gl_evtCallbacks.saEvtChannelOpenCallback=EvtOpenCallback;
      gl_evtCallbacks.saEvtEventDeliverCallback=EvtDeliverCallback;

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() with major version set to 0",SA_AIS_OK);

      printf("\nVersion Delivered : %c %d %d",gl_version.releaseCode,
             gl_version.majorVersion,gl_version.minorVersion);
      tet_printf("Version Delivered : %c %d %d",gl_version.releaseCode,
                 gl_version.majorVersion,gl_version.minorVersion);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 14:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 15:
      gl_major_version=0x03;
      gl_minor_version=0x01;
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      tet_saEvtFinalize(&gl_evtHandle);
      break;
    
    case 16:
      printf("Sleeping for 60 seconds to do a node lock\n");
      sleep(60);
      gl_major_version=0x03;
      gl_minor_version=0x01;
      gl_evtCallbacks.saEvtChannelOpenCallback=EvtOpenCallback;
      gl_evtCallbacks.saEvtEventDeliverCallback=EvtDeliverCallback;

      gl_rc=saEvtInitialize(&gl_evtHandle,&gl_evtCallbacks,&gl_version);
      result("saEvtInitialize() after locking the node",SA_AIS_ERR_UNAVAILABLE);

      printf("\nVersion Delivered : %c %d %d",gl_version.releaseCode,
             gl_version.majorVersion,gl_version.minorVersion);
      tet_printf("Version Delivered : %c %d %d",gl_version.releaseCode,
                 gl_version.majorVersion,gl_version.minorVersion);

      break;
     

    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}

void tet_Initialize(void)
{
  printf("\n------------ 1 : tet_Initialize -----------\n");
  var_initialize();

  tet_saEvtInitialize(&gl_evtHandle);        
  
  
    

  tet_saEvtFinalize(&gl_evtHandle);
  printf("\n------------ END -----------\n");
  
    
}

/*******saEvtSelectionObjectGet*******/

void tet_saEvtSelectionObjectGetCases(int iOption)
{
  printf("\n---------- tet_saEvtSelectionObjectGetCases : %d -------------",
         iOption);
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtSelectionObjectGet((SaEvtHandleT)(long)NULL,&gl_selObject);
      result("saEvtSelectionObjectGet() with NULL event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
      
    case 2:
      gl_evtHandle=(SaEvtHandleT)(long)NULL;

      gl_rc=saEvtSelectionObjectGet(gl_evtHandle,&gl_selObject);
      result("saEvtSelectionObjectGet() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtSelectionObjectGet(-123,&gl_selObject);
      result("saEvtSelectionObjectGet() with garbage event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 4:
      var_initialize();
      gl_evtCallbacks.saEvtChannelOpenCallback=NULL;
      gl_evtCallbacks.saEvtEventDeliverCallback=NULL;
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtSelectionObjectGet(gl_evtHandle,&gl_selObject);
      result("saEvtSelectionObjectGet() with NULL event callbacks handle",
             SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 5:
      var_initialize();
      gl_evtCallbacks.saEvtChannelOpenCallback=NULL;
      gl_evtCallbacks.saEvtEventDeliverCallback=EvtDeliverCallback;
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtSelectionObjectGet(gl_evtHandle,&gl_selObject);
      result("saEvtSelectionObjectGet() with NULL saEvtChannelOpenCallback \
event handle",SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 6:
      var_initialize();
      gl_evtCallbacks.saEvtChannelOpenCallback=EvtOpenCallback;
      gl_evtCallbacks.saEvtEventDeliverCallback=NULL;
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtSelectionObjectGet(gl_evtHandle,&gl_selObject);
      result("saEvtSelectionObjectGet() with NULL saEvtEventDeliverCallback \
event handle",SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle); 

      gl_rc=saEvtSelectionObjectGet(gl_evtHandle,NULL);
      result("saEvtSelectionObjectGet() with NULL selection object",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_rc=saEvtSelectionObjectGet((SaEvtHandleT)(long)NULL,
                                    (SaSelectionObjectT)(long)NULL);
      result("saEvtSelectionObjectGet() with NULL event handle and NULL \
selection object",SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      
      tet_saEvtSelectionObjectGet(&gl_evtHandle,&gl_selObject);
      

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}

void tet_SelectionObjectGet(void)
{
  printf("\n------------ 8 : tet_SelectionObjectGet -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  
  tet_saEvtSelectionObjectGet(&gl_evtHandle,&gl_selObject);

  
    
  
  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");
  
    
}

/*******saEvtDispatch*******/
void Dispatch_Blocking_Thread(void)
{
  gl_dispatchFlags=SA_DISPATCH_BLOCKING;
  gl_rc=saEvtDispatch(gl_evtHandle,gl_dispatchFlags);
}
void tet_saEvtDispatchCases(int iOption)
{
  printf("\n---------- tet_saEvtDispatchCases : %d -------------",iOption);
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtDispatch((SaEvtHandleT)(long)NULL,SA_DISPATCH_ONE);
      result("saEvtDispatch() with NULL event handle",SA_AIS_ERR_BAD_HANDLE);
      break;

    case 2:
      gl_evtHandle=(SaEvtHandleT)(long)NULL;

      gl_rc=saEvtDispatch(gl_evtHandle,SA_DISPATCH_ONE);
      result("saEvtDispatch() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);        
      break;

    case 3:
      gl_rc=saEvtDispatch(123,SA_DISPATCH_ONE);
      result("saEvtDispatch() with garbage event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtDispatch(gl_evtHandle,0);
      result("saEvtDispatch() with uninitialized dispatch flags",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
        
    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtDispatch(gl_evtHandle,24);
      result("saEvtDispatch() with dispatch flags set to garbage value(24)",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtFinalize(&gl_evtHandle);
      break;


    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_dispatchFlags=SA_DISPATCH_ONE;
      gl_rc=saEvtDispatch(gl_evtHandle,gl_dispatchFlags);
      result("saEvtDispatch() with dispatch flags set to DISPATCH_ALL",
             SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_dispatchFlags=SA_DISPATCH_ALL;
      gl_rc=saEvtDispatch(gl_evtHandle,gl_dispatchFlags);
      result("saEvtDispatch() with dispatch flags set to DISPATCH_ALL",
             SA_AIS_OK);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
          
#if 0  

    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      /*
        gl_dispatchFlags=SA_DISPATCH_BLOCKING;
        gl_rc=saEvtDispatch(gl_evtHandle,gl_dispatchFlags);*/
      /*Dispatch  thread*/
      if(tet_create_task((NCS_OS_CB)Dispatch_Blocking_Thread)
         ==NCSCC_RC_SUCCESS )
        {
          printf("\tTask has been Created\t");fflush(stdout);

          sleep(3);
          fflush(stdout);
#if 1
          /*Now Release the Initialize Thread*/
          if(m_NCS_TASK_RELEASE(gl_t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
#endif
        }

      result("saEvtDispatch() with dispatch flags set to DISPATCH_BLOCKING",
             SA_AIS_OK);
      tet_saEvtFinalize(&gl_evtHandle);
      break;

#endif

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      
      tet_saEvtDispatch(&gl_evtHandle);
      

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_Dispatch(void)
{
  printf("\n------------ 9 : tet_Dispatch -----------\n");
  var_initialize();

  tet_saEvtInitialize(&gl_evtHandle);

  
  
    

  tet_saEvtDispatch(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");
  
    

}

/*******saEvtFinalize*******/

void tet_saEvtFinalizeCases(int iOption)
{
  printf("\n---------- tet_saEvtFinalizeCases : %d -------------",iOption);
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtFinalize((SaEvtHandleT)(long)NULL);
      result("saEvtFinalize() with NULL event handle",SA_AIS_ERR_BAD_HANDLE);
      break;
        
    case 2:
      gl_evtHandle=(SaEvtHandleT)(long)NULL;

      gl_rc=saEvtFinalize(gl_evtHandle);
      result("saEvtFinalize() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtFinalize(123);
      result("saEvtFinalize() with garbage value",SA_AIS_ERR_BAD_HANDLE);
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      
      tet_saEvtFinalize(&gl_evtHandle);
      
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}

void tet_Finalize()
{
  printf("\n------------ 17 : tet_Finalize -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  
    

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");
  
    
}

/*******saEvtChannelOpen*******/

void tet_saEvtChannelOpenCases(int iOption)
{
  printf("\n---------- tet_saEvtChannelOpenCases : %d -------------",iOption);
  SaNameT emptyName={0,""},errName={4,"Test"};
       
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtChannelOpen((SaEvtHandleT)(long)NULL,&gl_channelName,
                             SA_EVT_CHANNEL_CREATE,gl_timeout,
                             &gl_channelHandle);
      result("saEvtChannelOpen() with NULL event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
      
    case 2:
      gl_evtHandle=(SaEvtHandleT)(long)NULL;

      gl_rc=saEvtChannelOpen(gl_evtHandle,&gl_channelName,
                             SA_EVT_CHANNEL_CREATE,gl_timeout,
                             &gl_channelHandle);
      result("saEvtChannelOpen() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
      
    case 3:
      gl_rc=saEvtChannelOpen(123,&gl_channelName,SA_EVT_CHANNEL_CREATE,
                             gl_timeout,&gl_channelHandle);
      result("saEvtChannelOpen() with garbage event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
      
    case 4:
      gl_version.releaseCode='B';
      gl_version.majorVersion=1;
      gl_version.minorVersion=1;
      gl_evtCallbacks.saEvtChannelOpenCallback=NULL;
      gl_evtCallbacks.saEvtEventDeliverCallback=NULL;

      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      gl_rc=saEvtChannelOpen(gl_evtHandle,&gl_channelName,
                             SA_EVT_CHANNEL_CREATE, gl_timeout,
                             &gl_channelHandle);
      result("saEvtChannelOpen() with NULL event callbacks",SA_AIS_OK);

      tet_saEvtChannelClose(&gl_channelHandle);
  
      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
  
      break;
      
    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_rc=saEvtChannelOpen(gl_evtHandle,NULL,SA_EVT_CHANNEL_CREATE,
                             gl_timeout, &gl_channelHandle);
      result("saEvtChannelOpen() with NULL channel name",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_rc=saEvtChannelOpen(gl_evtHandle,&emptyName,SA_EVT_CHANNEL_CREATE,
                             gl_timeout,&gl_channelHandle);
      result("saEvtChannelOpen() with uninitialized channel name",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_rc=saEvtChannelOpen(gl_evtHandle,&gl_channelName,
                             (SaEvtChannelOpenFlagsT)(long)NULL,gl_timeout,
                             &gl_channelHandle);
      result("saEvtChannelOpen() with NULL channel open flags",
             SA_AIS_ERR_BAD_FLAGS);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_channelOpenFlags=(SaEvtChannelOpenFlagsT)(long)NULL;
      gl_rc=saEvtChannelOpen(gl_evtHandle,&gl_channelName,gl_channelOpenFlags,
                             gl_timeout,&gl_channelHandle);
      result("saEvtChannelOpen() with uninitialized channel open flags",
             SA_AIS_ERR_BAD_FLAGS);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_rc=saEvtChannelOpen(gl_evtHandle,&gl_channelName,24,gl_timeout,
                             &gl_channelHandle);
      result("saEvtChannelOpen() with garbage channel open flags(24)",
             SA_AIS_ERR_BAD_FLAGS);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
       
    case 10:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpen(gl_evtHandle,&errName,SA_EVT_CHANNEL_PUBLISHER,
                             gl_timeout,&gl_channelHandle);
      result("saEvtChannelOpen() with SA_EVT_CHANNEL_PUBLISHER before channel\
 name(Test) is given",SA_AIS_ERR_NOT_EXIST);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 11:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpen(gl_evtHandle,&errName,SA_EVT_CHANNEL_SUBSCRIBER,
                             gl_timeout,&gl_channelHandle);
      result("saEvtChannelOpen() with SA_EVT_CHANNEL_SUBSCRIBER before channel\
 name(Test) is given",SA_AIS_ERR_NOT_EXIST);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 12:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpen(gl_evtHandle,&gl_channelName,
                             SA_EVT_CHANNEL_CREATE,(SaTimeT)(long)NULL,
                             &gl_channelHandle);
      result("saEvtChannelOpen() with NULL timeout",SA_AIS_ERR_TIMEOUT);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 13:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpen(gl_evtHandle,&gl_channelName,
                             SA_EVT_CHANNEL_CREATE,-3,&gl_channelHandle);
      result("saEvtChannelOpen() with negative timeout",SA_AIS_ERR_TIMEOUT);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 14:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpen(gl_evtHandle,&gl_channelName,
                             SA_EVT_CHANNEL_CREATE,999999999,
                             &gl_channelHandle);
      result("saEvtChannelOpen() with large timeout",SA_AIS_OK);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 15:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_rc=saEvtChannelOpen(gl_evtHandle,&gl_channelName,
                             SA_EVT_CHANNEL_CREATE,gl_timeout,NULL);
      result("saEvtChannelOpen() with NULL channel handle",
             SA_AIS_ERR_INVALID_PARAM);        

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 16:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
      

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_ChannelOpen()
{
  printf("\n------------ 2 : tet_ChannelOpen -----------\n");
#if 0
  int iCount;
  char tempName[10];
  SaEvtChannelHandleT gHandle[1000];
  SaEvtEventHandleT eHandle[1000];
#endif

  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);
 
  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;


#if 0
  for(iCount=0;iCount<1000;iCount++)
    {
      sprintf(tempName,"channel%3d",iCount);
      gl_channelName.length=sizeof(tempName);
      memcpy(gl_channelName.value,tempName,sizeof(tempName));
#endif
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);        
#if 0
      gHandle[iCount]=gl_channelHandle;
#endif
      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);
#if 0
      eHandle[iCount]=gl_eventHandle;
#endif

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=10000000000000.0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

#if 0
    }
#endif


  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 1 \n");
  
    

#if 0 
  for(iCount=0;iCount<1000;iCount++)
    {
      gl_channelHandle=gHandle[iCount];
#endif
      tet_saEvtChannelClose(&gl_channelHandle);
#if 0
      sprintf(tempName,"channel%3d",iCount);
      gl_channelName.length=sizeof(tempName);
      memcpy(gl_channelName.value,tempName,sizeof(tempName));
#endif
      tet_saEvtChannelUnlink(&gl_evtHandle);
#if 0
    }
#endif
  

  tet_saEvtFinalize(&gl_evtHandle);


  printf("\n------------ END -----------\n");
  
    
}
void Initialize_Thread()
{
  tet_saEvtInitialize(&gl_evtHandle);
}
void SelectionObjectGet_Thread()
{
  tet_saEvtSelectionObjectGet(&gl_evtHandle,&gl_selObject);
}
void Dispatch_Thread()
{
  tet_saEvtDispatch(&gl_evtHandle);
}
void ChannelOpen_Thread()
{
  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
}
void Allocate_Thread()
{
  tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);
}
void AttributesSet_Thread()
{
  gl_patternArray.patternsNumber=2;
  gl_patternArray.patterns=gl_pattern;
  gl_retentionTime=10000000000000.0;
  gl_allocatedNumber=2;
  gl_patternLength=5;
  tet_saEvtEventAttributesSet(&gl_eventHandle);
}
void AttributesGet_Thread()
{
  tet_saEvtEventAttributesGet(&gl_eventHandle);
}
void Publish_Thread()
{
  tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);
}
void Free_Thread()
{
  tet_saEvtEventFree(&gl_eventHandle);
}
void ChannelClose_Thread()
{
  tet_saEvtChannelClose(&gl_channelHandle);
}
void ChannelUnlink_Thread()
{
  tet_saEvtChannelUnlink(&gl_evtHandle);
}
void Finalize_Thread()
{
  tet_saEvtFinalize(&gl_evtHandle);
}
void tet_Publish_Thread()
{
  printf("\n------------ 1 : tet_Publish_Thread -----------\n");
  tet_printf("\n------------ 1 : tet_Publish_Thread -----------\n");
  var_initialize();
  /*Initialize thread*/
  if(tet_create_task((NCS_OS_CB)Initialize_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the Initialize Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }
  /*SelectionObjectGet thread*/
  if(tet_create_task((NCS_OS_CB)SelectionObjectGet_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the SelectionObjectGet Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }
  
  /*Dispatch thread*/
  if(tet_create_task((NCS_OS_CB)Dispatch_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the Dispatch Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }
 
  /*ChannelOpen thread*/
  if(tet_create_task((NCS_OS_CB)ChannelOpen_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the ChannelOpen Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }

  /*Allocate thread*/
  if(tet_create_task((NCS_OS_CB)Allocate_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the Allocate Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }
  /*AttributesSet thread*/
  if(tet_create_task((NCS_OS_CB)AttributesSet_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the AttributesSet Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }
  /*Publish thread*/
  if(tet_create_task((NCS_OS_CB)Publish_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the Publish Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }


  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 1 \n");
  
    

  /*AttributesGet thread*/
  if(tet_create_task((NCS_OS_CB)AttributesGet_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(2);
      fflush(stdout);
      /*Now Release the AttributesGet Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }  

  /*Free thread*/
  if(tet_create_task((NCS_OS_CB)Free_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);

      sleep(1);
      fflush(stdout);
      /*Now Release the Free Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }
  /*ChannelClose thread*/
  if(tet_create_task((NCS_OS_CB)ChannelClose_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the ChannelClose Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }

  /*ChannelUnlink thread*/
  if(tet_create_task((NCS_OS_CB)ChannelUnlink_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the ChannelUnlink Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }
  /*Finalize thread*/
  if(tet_create_task((NCS_OS_CB)Finalize_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the Finalize Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }


  printf("\n------------ END -----------\n");
  
    
}


/*******saEvtChannelOpenAsync*******/

void tet_saEvtChannelOpenAsyncCases(int iOption)
{
  printf("\n---------- tet_saEvtChannelOpenAsyncCases : %d -------------",
         iOption);
  SaNameT asyncchannelName={4,"Temp"},emptyName={0,""};

  gl_invocation=3;
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtChannelOpenAsync((SaEvtHandleT)(long)NULL,gl_invocation,
                                  &asyncchannelName,SA_EVT_CHANNEL_CREATE);
      result("saEvtChannelOpenAsync() with NULL event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 2:
      gl_evtHandle=(SaEvtHandleT)(long)NULL;

      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  SA_EVT_CHANNEL_CREATE);
      result("saEvtChannelOpenAsync() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtChannelOpenAsync(123,gl_invocation,&asyncchannelName,
                                  SA_EVT_CHANNEL_CREATE);
      result("saEvtChannelOpenAsync() with garbage event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
      
    case 4:
      var_initialize();
      gl_evtCallbacks.saEvtChannelOpenCallback=NULL;
      gl_evtCallbacks.saEvtEventDeliverCallback=NULL;

      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  SA_EVT_CHANNEL_CREATE);
      result("saEvtChannelOpenAsync() with NULL event callbacks",
             SA_AIS_ERR_INIT);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 5: 
      var_initialize();
      gl_evtCallbacks.saEvtChannelOpenCallback=NULL;

      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  gl_channelOpenFlags);
      result("saEvtChannelOpenAsync() with NULL channel open callback",
             SA_AIS_ERR_INIT);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 6:
      var_initialize();
      gl_evtCallbacks.saEvtEventDeliverCallback=NULL;
  
      tet_saEvtInitialize(&gl_evtHandle);
      
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  gl_channelOpenFlags);
      result("saEvtChannelOpenAsync() with NULL event Deliver Callback",
             SA_AIS_OK);
      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();

      tet_saEvtChannelClose(&gl_channelHandle);

      gl_channelName=asyncchannelName;
      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
                
    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
       
      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  SA_EVT_CHANNEL_PUBLISHER);
      result("saEvtChannelOpenAsync() with SA_EVT_CHANNEL_PUBLISHER on async\
 channel name(Temp)",SA_AIS_OK);

      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
        
      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  SA_EVT_CHANNEL_SUBSCRIBER);
      result("saEvtChannelOpenAsync() with SA_EVT_CHANNEL_SUBSCRIBER on async\
 channel name(Temp)",SA_AIS_OK);
      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      
      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,(SaInvocationT)(long)NULL,
                                  &asyncchannelName,SA_EVT_CHANNEL_CREATE);
      result("saEvtChannelOpenAsync() with NULL invocation",SA_AIS_OK);
      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();

      tet_saEvtChannelClose(&gl_channelHandle);

      gl_channelName=asyncchannelName;
      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 10:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      
      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  SA_EVT_CHANNEL_CREATE);
      result("saEvtChannelOpenAsync() with uninitialized invocation",
             SA_AIS_OK);
      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();

      tet_saEvtChannelClose(&gl_channelHandle);

      gl_channelName=asyncchannelName;
      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 11:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  SA_EVT_CHANNEL_CREATE);
      result("saEvtChannelOpenAsync() with initialized invocation",SA_AIS_OK);
      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();

      tet_saEvtChannelClose(&gl_channelHandle); 
      
      gl_channelName=asyncchannelName;
      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 12:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      
      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,-123,&asyncchannelName,
                                  SA_EVT_CHANNEL_CREATE);
      result("saEvtChannelOpenAsync() with garbage invocation",SA_AIS_OK);
      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();
 
      tet_saEvtChannelClose(&gl_channelHandle);
   
      gl_channelName=asyncchannelName;
      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 13:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,NULL,
                                  SA_EVT_CHANNEL_CREATE);
      result("saEvtChannelOpenAsync() with NULL channel name",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 14:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&emptyName,
                                  SA_EVT_CHANNEL_CREATE);
      result("saEvtChannelOpenAsync() with uninitialized channel name",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 15:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  (SaEvtChannelOpenFlagsT)(long)NULL);
      result("saEvtChannelOpenAsync() with NULL channel open flags",
             SA_AIS_ERR_BAD_FLAGS);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 16:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_channelOpenFlags=(SaEvtChannelOpenFlagsT)(long)NULL;
      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  gl_channelOpenFlags);
      result("saEvtChannelOpenAsync() with uninitialized channel open flags",
             SA_AIS_ERR_BAD_FLAGS);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 17:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_rc=saEvtChannelOpenAsync(gl_evtHandle,gl_invocation,&asyncchannelName,
                                  24);
      result("saEvtChannelOpenAsync() with garbage channel open flags(24)",
             SA_AIS_ERR_BAD_FLAGS);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 18:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      
      tet_saEvtChannelOpenAsync(&gl_evtHandle);
      
      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}

void tet_ChannelOpenAsync()
{
  printf("\n------------ 3 : tet_ChannelOpenAsync -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);
        
  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
  
  tet_saEvtChannelOpenAsync(&gl_evtHandle);

  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 0 \n");
  
    
  
  thread_creation(&gl_evtHandle);
   DISPATCH_SLEEP();

  tet_saEvtChannelClose(&gl_channelHandle);
 
  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);


  printf("\n------------ END -----------\n");
  
    
}
void ChannelOpenAsync_Thread()
{
  tet_saEvtChannelOpenAsync(&gl_evtHandle);
}
void tet_ChannelOpenAsync_Thread()
{
  printf("\n------------ 2 : tet_ChannelOpenAsync_Thread -----------\n");
  tet_printf("\n------------ 2 : tet_ChannelOpenAsync_Thread -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);
        
  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
  
  /*ChannelOpenAsync thread*/
  if(tet_create_task((NCS_OS_CB)ChannelOpenAsync_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(2);
      fflush(stdout);
      /*Now Release the ChannelOpenAsync Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }  
  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 0 \n");
  
    
  
  thread_creation(&gl_evtHandle);
   DISPATCH_SLEEP();

  tet_saEvtChannelClose(&gl_channelHandle);
 
  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);


  printf("\n------------ END -----------\n");
  
    
}
/*******saEvtChannelClose*******/

void tet_saEvtChannelCloseCases(int iOption)
{
  printf("\n---------- tet_saEvtChannelCloseCases : %d -------------",iOption);
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtChannelClose((SaEvtChannelHandleT)(long)NULL);
      result("saEvtChannelClose() with NULL channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 2:
      gl_channelHandle=(SaEvtChannelHandleT)(long)NULL;
     
      gl_rc=saEvtChannelClose(gl_channelHandle);
      result("saEvtChannelClose() with uninitialized channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
        
    case 3:
      gl_rc=saEvtChannelClose(123);
      result("saEvtChannelClose() with garbage channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      
      tet_saEvtChannelClose(&gl_channelHandle);
      

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_ChannelClose()
{
  printf("\n------------ 15 : tet_ChannelClose -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=4;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);


  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 0 \n");
  
    
  
  tet_saEvtChannelClose(&gl_channelHandle);
  
  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");

  
    
}

/*******saEvtChannelUnlink*******/

void tet_saEvtChannelUnlinkCases(int iOption)
{
  printf("\n---------- tet_saEvtChannelUnlinkCases : %d -------------",
         iOption);
  SaNameT emptyName,tempName={4,"temp"};
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtChannelUnlink((SaEvtHandleT)(long)NULL,&gl_channelName);
      result("saEvtChannelUnlink() with NULL event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
                
    case 2:
      gl_evtHandle=(SaEvtHandleT)(long)NULL;

      gl_rc=saEvtChannelUnlink(gl_evtHandle,&gl_channelName);
      result("saEvtChannelUnlink() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtChannelUnlink(123,&gl_channelName);
      result("saEvtChannelUnlink() with garbage event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
        
    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_rc=saEvtChannelClose(gl_channelHandle);
 
      gl_rc=saEvtChannelUnlink(gl_evtHandle,NULL);
      result("saEvtChannelUnlink() with NULL channel name",
             SA_AIS_ERR_INVALID_PARAM);
 
      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      gl_rc=saEvtChannelUnlink(gl_evtHandle,&emptyName);
      result("saEvtChannelUnlink() with uninitialized channel name",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    
    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
  
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      gl_rc=saEvtChannelUnlink(gl_evtHandle,&tempName);
      result("saEvtChannelUnlink() with channel name no event",
             SA_AIS_ERR_NOT_EXIST);        

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      
      tet_saEvtChannelUnlink(&gl_evtHandle);
      

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      gl_rc=saEvtChannelUnlink(gl_evtHandle,&gl_channelName);
      result("saEvtChannelUnlink() invoked",SA_AIS_OK);

      gl_rc=saEvtChannelUnlink(gl_evtHandle,&gl_channelName);
      result("saEvtChannelUnlink() with channel name (News) duplicate check",
             SA_AIS_ERR_NOT_EXIST);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      memcpy(&tempName.value,"abcd",sizeof("abcd"));
      gl_rc=saEvtChannelUnlink(gl_evtHandle,&tempName);
      result("saEvtChannelUnlink() with invalid channel name",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_ChannelUnlink()
{
  printf("\n------------ 16 : tet_ChannelUnlink -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=4;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 0 \n");
    
  tet_saEvtChannelClose(&gl_channelHandle);
  printf("\n NumUsers= 0 : NumSubscribers= 0 : NumPublishers= 0 \n");
  
  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");

  
    
}

/*******saEvtEventAllocate*******/

void tet_saEvtEventAllocateCases(int iOption)
{
  printf("\n---------- tet_saEvtEventAllocateCases : %d -------------",
         iOption);
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtEventAllocate((SaEvtChannelHandleT)(long)NULL,&gl_eventHandle);
      result("saEvtEventAllocate() with NULL channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 2:
      gl_channelHandle=(SaEvtChannelHandleT)(long)NULL;

      gl_rc=saEvtEventAllocate(gl_channelHandle,&gl_eventHandle);
      result("saEvtEventAllocate() with uninitialized channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
        
    case 3:
      gl_rc=saEvtEventAllocate(123,&gl_eventHandle);
      result("saEvtEventAllocate() with garbage channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
   
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_rc=saEvtEventAllocate(gl_channelHandle,NULL);
      result("saEvtEventAllocate() with NULL event handle",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);
 
      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_rc=saEvtEventAllocate(gl_channelHandle,&gl_eventHandle);
      result("saEvtEventAllocate() with channel open sync for subscriber",
             SA_AIS_OK);

      tet_saEvtChannelClose(&gl_channelHandle);
 
      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE | SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_rc=saEvtEventAllocate(gl_channelHandle,&gl_eventHandle);
      result("saEvtEventAllocate() with channel opened with create",
             SA_AIS_OK);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 7:        
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      
      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);        
      

      tet_saEvtEventFree(&gl_eventHandle);
 
      tet_saEvtChannelClose(&gl_channelHandle);
 
      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_Allocate()
{
  printf("\n------------ 5 : tet_Allocate -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);
 
  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);


  tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);
  
  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 0 \n");
  
    

  tet_saEvtEventFree(&gl_eventHandle);

  tet_saEvtChannelClose(&gl_channelHandle);

  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);                

  printf("\n------------ END -----------\n");
  
    
}

/*******saEvtEventFree*******/

void tet_saEvtEventFreeCases(int iOption)
{
  printf("\n---------- tet_saEvtEventFreeCases : %d -------------",iOption);
  SaEvtChannelHandleT localChannelHandle;
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtEventFree((SaEvtEventHandleT)(long)NULL);
      result("saEvtEventFree() with NULL event handle",SA_AIS_ERR_BAD_HANDLE);
      break;

    case 2:
      gl_eventHandle=(SaEvtEventHandleT)(long)NULL;

      gl_rc=saEvtEventFree(gl_eventHandle);
      result("saEvtEventFree() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtEventFree(123);
      result("saEvtEventFree() with garbage event handle",
             SA_AIS_ERR_BAD_HANDLE);        
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      localChannelHandle=gl_channelHandle;
      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_rc=saEvtEventFree(gl_eventHandle);
      result("saEvtEventFree() with channel closed",SA_AIS_OK);

      tet_saEvtChannelClose(&localChannelHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);

      break;
      
    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      
      tet_saEvtEventFree(&gl_eventHandle);
      

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);
  
      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      gl_rc=saEvtEventFree(gl_eventHandle);
      result("saEvtEventFree() with channel closed",SA_AIS_ERR_BAD_HANDLE);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_Free()
{
  printf("\n------------ 14 : tet_Free -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

  tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);


  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 1 \n");
  
    
  
  tet_saEvtEventFree(&gl_eventHandle);
  

  tet_saEvtChannelClose(&gl_channelHandle);

  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);        

  printf("\n------------ END -----------\n");

  
    
}
/*******saEvtEventAttributesGet*******/

void tet_saEvtEventAttributesGetCases(int iOption)
{
  SaEvtEventPatternArrayT *temp;
  printf("\n---------- tet_saEvtEventAttributesGetCases : %d -------------",
         iOption);
  SaEvtEventPatternT maxPatternSize[1]=
    {
      {3,255,(SaUint8T *)"Hello"}
    };
  gl_patternArray.patternsNumber=0;
  gl_patternArray.patterns=NULL;
        
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtEventAttributesGet((SaEvtEventHandleT)(long)NULL,&gl_patternArray,
                                    &gl_priority,&gl_retentionTime,
                                    &gl_publisherName,&gl_publishTime,
                                    &gl_evtId);
      result("saEvtEventAttributesGet() with NULL event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 2:
      gl_eventHandle=(SaEvtEventHandleT)(long)NULL;

      gl_rc=saEvtEventAttributesGet(gl_eventHandle,&gl_patternArray,
                                    &gl_priority,&gl_retentionTime,
                                    &gl_publisherName,&gl_publishTime,
                                    &gl_evtId);
      result("saEvtEventAttributesGet() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtEventAttributesGet(123,&gl_patternArray,&gl_priority,
                                    &gl_retentionTime,&gl_publisherName,
                                    &gl_publishTime,&gl_evtId);
      result("saEvtEventAttributesGet() with garbage event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesGet(gl_eventHandle,NULL,&gl_priority,
                                    &gl_retentionTime,&gl_publisherName,
                                    &gl_publishTime,&gl_evtId);
      result("saEvtEventAttributesGet() with NULL pattern array",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
        
    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesGet(gl_eventHandle,&gl_patternArray,
                                    &gl_priority,&gl_retentionTime,
                                    &gl_publisherName,&gl_publishTime,
                                    &gl_evtId);
      result("saEvtEventAttributesGet() with empty pattern array",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesGet(gl_eventHandle,&gl_patternArray,NULL,
                                    &gl_retentionTime,&gl_publisherName,
                                    &gl_publishTime,&gl_evtId);
      result("saEvtEventAttributesGet() with NULL priority",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
            
    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
   
      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_priority=(SaEvtEventPriorityT)(long)NULL;
      gl_rc=saEvtEventAttributesGet(gl_eventHandle,&gl_patternArray,
                                    &gl_priority,&gl_retentionTime,
                                    &gl_publisherName,&gl_publishTime,
                                    &gl_evtId);
      result("saEvtEventAttributesGet() with uninitialized priority",
             SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesGet(gl_eventHandle,&gl_patternArray,
                                    &gl_priority,NULL,&gl_publisherName,
                                    &gl_publishTime,&gl_evtId);
      result("saEvtEventAttributesGet() with NULL retention time",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
 
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
 
      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);
 
      gl_retentionTime=(SaTimeT)(long)NULL;
      gl_rc=saEvtEventAttributesGet(gl_eventHandle,&gl_patternArray,
                                    &gl_priority,&gl_retentionTime,
                                    &gl_publisherName,&gl_publishTime,
                                    &gl_evtId);
      result("saEvtEventAttributesGet() with uninitialized retention time",
             SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);
 
      tet_saEvtChannelClose(&gl_channelHandle);
 
      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
           
    case 10:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesGet(gl_eventHandle,&gl_patternArray,
                                    &gl_priority,&gl_retentionTime,NULL,
                                    &gl_publishTime,&gl_evtId);
      result("saEvtEventAttributesGet() with NULL publisher name",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
            
    case 11:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesGet(gl_eventHandle,&gl_patternArray,
                                    &gl_priority,&gl_retentionTime,
                                    &gl_publisherName,NULL,&gl_evtId);
      result("saEvtEventAttributesGet() with NULL publish time",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 12:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesGet(gl_eventHandle,&gl_patternArray,
                                    &gl_priority,&gl_retentionTime,
                                    &gl_publisherName,&gl_publishTime,NULL);
      result("saEvtEventAttributesGet() with NULL event id",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 13:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=1;
      gl_patternArray.patterns=maxPatternSize;
      gl_allocatedNumber=1;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      temp=(SaEvtEventPatternArrayT *)malloc(sizeof(SaEvtEventPatternArrayT));
      temp->patternsNumber=1;
      temp->allocatedNumber=1;
      temp->patterns=(SaEvtEventPatternT *)malloc(sizeof(SaEvtEventPatternT));
      temp->patterns->allocatedSize=255;
      temp->patterns->patternSize=255;
      temp->patterns->pattern=(SaUint8T *)malloc(255);
      gl_rc=saEvtEventAttributesGet(gl_eventHandle,temp,&gl_priority,
                                    &gl_retentionTime,&gl_publisherName,
                                    &gl_publishTime,&gl_evtId);
      result("saEvtEventAttributesGet() with pattern size greater than 256",
             SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 14:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      
      gl_allocatedNumber=0;
      tet_saEvtEventAttributesGet(&gl_eventHandle);
      

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);            
      break;

     case 15:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=1;
      gl_patternArray.patterns=maxPatternSize;
      gl_allocatedNumber=1;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      temp=(SaEvtEventPatternArrayT *)malloc(sizeof(SaEvtEventPatternArrayT));
      temp->patternsNumber=1;
      temp->allocatedNumber=1;
      temp->patterns=(SaEvtEventPatternT *)malloc(sizeof(SaEvtEventPatternT));
      temp->patterns->allocatedSize=255;
      temp->patterns->patternSize=255;
      temp->patterns->pattern=(SaUint8T *)malloc(255);
      gl_rc=saEvtEventAttributesGet(gl_eventHandle,temp,&gl_priority,
                                    &gl_retentionTime,&gl_publisherName,
                                    &gl_publishTime,&gl_evtId);
      result("saEvtEventAttributesGet() with pattern size greater than 256",
             SA_AIS_OK);
      temp->patterns=NULL;
      gl_rc=saEvtEventPatternFree(gl_eventHandle,temp->patterns);
      result("saEvtEventPatternsFree() with NULL patterns",
             SA_AIS_ERR_INVALID_PARAM);
      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

     case 16:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=1;
      gl_patternArray.patterns=maxPatternSize;
      gl_allocatedNumber=1;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      temp=(SaEvtEventPatternArrayT *)malloc(sizeof(SaEvtEventPatternArrayT));
      temp->patternsNumber=1;
      temp->allocatedNumber=1;
      temp->patterns=(SaEvtEventPatternT *)malloc(sizeof(SaEvtEventPatternT));
      temp->patterns->allocatedSize=255;
      temp->patterns->patternSize=255;
      temp->patterns->pattern=(SaUint8T *)malloc(255);
      gl_rc=saEvtEventAttributesGet(gl_eventHandle,temp,&gl_priority,
                                    &gl_retentionTime,&gl_publisherName,
                                    &gl_publishTime,&gl_evtId);
      result("saEvtEventAttributesGet() with pattern size greater than 256",
             SA_AIS_OK);
      tet_saEvtEventFree(&gl_eventHandle);
      gl_rc=saEvtEventPatternFree(gl_eventHandle,temp->patterns);
      result("saEvtEventPatternsFree() for already saEvtEventFreed patterns",
             SA_AIS_ERR_BAD_HANDLE);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

     case 17:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=1;
      gl_patternArray.patterns=maxPatternSize;
      gl_allocatedNumber=1;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      temp=(SaEvtEventPatternArrayT *)malloc(sizeof(SaEvtEventPatternArrayT));
      temp->patternsNumber=1;
      temp->allocatedNumber=1;
      temp->patterns=(SaEvtEventPatternT *)malloc(sizeof(SaEvtEventPatternT));
      temp->patterns->allocatedSize=255;
      temp->patterns->patternSize=255;
      temp->patterns->pattern=(SaUint8T *)malloc(255);
      gl_rc=saEvtEventAttributesGet(gl_eventHandle,temp,&gl_priority,
                                    &gl_retentionTime,&gl_publisherName,
                                    &gl_publishTime,&gl_evtId);
      result("saEvtEventAttributesGet() with pattern size greater than 256",
             SA_AIS_OK);
      gl_rc=saEvtEventPatternFree(gl_eventHandle,temp->patterns);
      result("saEvtEventPatternsFree() Success Case",
             SA_AIS_OK);
      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}

void tet_AttributesGet()
{
  printf("\n------------ 10 : tet_AttributesGet -----------\n");
  
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

  tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

  gl_patternArray.patternsNumber=2;
  gl_patternArray.patterns=gl_pattern;
  gl_retentionTime=1000.0;
  gl_priority=3;
  gl_allocatedNumber=2;
  gl_patternLength=5;
  tet_saEvtEventAttributesSet(&gl_eventHandle);


  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 0 \n");
  
    
  
  tet_saEvtEventAttributesGet(&gl_eventHandle);
  

  tet_saEvtEventFree(&gl_eventHandle);

  tet_saEvtChannelClose(&gl_channelHandle);

  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");
  
    

}
/*******saEvtEventAttributesSet*******/

void tet_saEvtEventAttributesSetCases(int iOption)
{
  printf("\n---------- tet_saEvtEventAttributesSetCases : %d -------------",
         iOption);
  SaNameT emptyName={0,""},maxName={257,"Hello"};
  SaEvtEventPatternT maxPatternSize[1]=
    {
      {3,444,(SaUint8T *)"Hello"}
    };
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtEventAttributesSet((SaEvtEventHandleT)(long)NULL,&gl_patternArray,
                                    gl_priority,gl_retentionTime,
                                    &gl_publisherName);
      result("saEvtEventAttributesSet() with NULL event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
      
    case 2:
      gl_eventHandle=(SaEvtEventHandleT)(long)NULL;

      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,gl_retentionTime,
                                    &gl_publisherName);
      result("saEvtEventAttributesSet() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtEventAttributesSet(123,&gl_patternArray,gl_priority,
                                    gl_retentionTime,&gl_publisherName);
      result("saEvtEventAttributesSet() with garbage event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesSet(gl_eventHandle,NULL,gl_priority,
                                    gl_retentionTime,&gl_publisherName);
      result("saEvtEventAttributesSet() with NULL pattern array",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=0;
      gl_patternArray.patterns=NULL;
      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,gl_retentionTime,
                                    &gl_publisherName);
      result("saEvtEventAttributesSet() with uninitialized pattern array",
             SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);   

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    (SaEvtEventPriorityT)(long)NULL,gl_retentionTime,
                                    &gl_publisherName);
      result("saEvtEventAttributesSet() with NULL priority",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);   

      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,5,
                                    gl_retentionTime,&gl_publisherName);
      result("saEvtEventAttributesSet() with garbage priority",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,(SaTimeT)(long)NULL,
                                    &gl_publisherName);
      result("saEvtEventAttributesSet() with NULL retention time",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,256,&gl_publisherName);
      result("saEvtEventAttributesSet() with garbage retention time",
             SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 10:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,gl_retentionTime,NULL);
      result("saEvtEventAttributesSet() with NULL publisher name",SA_AIS_OK); 

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 11:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,gl_retentionTime,&emptyName);
      result("saEvtEventAttributesSet() with empty publisher name",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 12:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,gl_retentionTime,&maxName);
      result("saEvtEventAttributesSet() with publisher name length greater \
than max length",SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 13:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,gl_retentionTime,
                                    &gl_publisherName);
      result("saEvtEventAttributesSet() with channel open subscriber case",
             SA_AIS_ERR_ACCESS);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 14:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,gl_retentionTime,
                                    &gl_publisherName);
      result("saEvtEventAttributesSet() with channel open with create",
             SA_AIS_ERR_ACCESS);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 15:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags= SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=1;
      gl_patternArray.patterns=maxPatternSize;
      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,gl_retentionTime,
                                    &gl_publisherName);
      result("saEvtEventAttributesSet() with pattern size greater than 256",
             SA_AIS_ERR_TOO_BIG);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 16:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_rc=saEvtEventAttributesSet(gl_eventHandle,&gl_patternArray,
                                    gl_priority,gl_retentionTime,
                                    &gl_publisherName);
      result("saEvtEventAttributesSet() with channel closed",
             SA_AIS_ERR_BAD_HANDLE);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    
    case 17:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);
      

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_AttributesSet()
{        
  printf("\n------------ 6 : tet_AttributesSet -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

  tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);


  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 1 \n");
  
    
  
  gl_patternArray.patternsNumber=2;
  gl_patternArray.patterns=gl_pattern;
  gl_retentionTime=1000.0;
  gl_allocatedNumber=2;
  gl_patternLength=5;
  tet_saEvtEventAttributesSet(&gl_eventHandle);
  

  tet_saEvtEventFree(&gl_eventHandle);

  tet_saEvtChannelClose(&gl_channelHandle);

  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");
  
    
}
/*******saEvtEventDataGet*******/

void tet_saEvtEventDataGetCases(int iOption)
{
  char tempData[15];
  SaSizeT eventDataSize;
  printf("\n---------- tet_saEvtEventDataGetCases : %d -------------",iOption);
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtEventDataGet((SaEvtEventHandleT)(long)NULL,&gl_eventData,
                              &gl_eventDataSize);
      result("saEvtEventDataGet() with NULL event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
      
    case 2:
      gl_eventHandle=(SaEvtEventHandleT)(long)NULL;

      gl_rc=saEvtEventDataGet(gl_eventHandle,&gl_eventData,&gl_eventDataSize);
      result("saEvtEventDataGet() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtEventDataGet(123,&gl_eventData,&gl_eventDataSize);
      result("saEvtEventDataGet() with garbage event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER
        |SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=1000000000.0;
      gl_publisherName.length=4;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

       strcpy (gl_eventData,"Publish");
      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();
 
      gl_rc=saEvtEventDataGet(gl_eventDeliverHandle,NULL,&gl_eventDataSize);
      result("saEvtEventDataGet() with NULL event data",SA_AIS_OK);

      printf("\nEventDataSize allocated by the Event Service : %llu",
             gl_eventDataSize);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER
        |SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=1000000000.0;
      gl_publisherName.length=4;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);
       strcpy (gl_eventData,"Publish");
      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();
 
      gl_rc=saEvtEventDataGet(gl_eventDeliverHandle,NULL,NULL);
      result("saEvtEventDataGet() with NULL event data and event data size",
             SA_AIS_OK);
      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
        
    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER
        |SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=1000000000.0;
      gl_publisherName.length=4;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);
       strcpy (gl_eventData,"Publish");
      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();
 
      gl_rc=saEvtEventDataGet(gl_eventDeliverHandle,gl_eventData,NULL);
      result("saEvtEventDataGet() with NULL event data size",
             SA_AIS_ERR_NO_SPACE);
      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
    
      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=1000000000000.0;
      gl_publisherName.length=4;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      strcpy (gl_eventData,"Publish");
      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);
      
      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();

      gl_rc=saEvtEventDataGet(gl_eventDeliverHandle,&tempData,&eventDataSize);
      result("saEvtEventDataGet() with event data size > length than allowed",
             SA_AIS_ERR_NO_SPACE);
      gl_eventDataSize=20;

      tet_saEvtEventRetentionTimeClear(&gl_channelHandle,&gl_evtId);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
            
    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=10000000000.0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      strcpy (gl_eventData,"Publish");
      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      gl_rc=saEvtEventDataGet(gl_eventHandle,&gl_eventData,&gl_eventDataSize);
      result("saEvtEventDataGet() with channel when published",
             SA_AIS_ERR_BAD_HANDLE);

      tet_saEvtEventRetentionTimeClear(&gl_channelHandle,&gl_evtId);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
  
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      gl_subscriptionId=37;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);
      gl_subscriptionId=3;

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=1000000000000.0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      strcpy (gl_eventData,"Publish");

      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);
       DISPATCH_SLEEP();

      thread_creation(&gl_evtHandle);

      gl_eventDataSize=0;
      gl_rc=saEvtEventDataGet(gl_eventDeliverHandle,&gl_eventData,
                              &gl_eventDataSize);
      result("saEvtEventDataGet() with no memory",SA_AIS_ERR_NO_SPACE);      
      gl_eventDataSize=20;

      tet_saEvtEventRetentionTimeClear(&gl_channelHandle,&gl_evtId);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 10:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
   
      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=100000000000.0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

       strcpy (gl_eventData,"Publish");
      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      thread_creation(&gl_evtHandle);
       DISPATCH_SLEEP();
      tet_saEvtEventDataGet(&gl_eventDeliverHandle);
      

      tet_saEvtEventRetentionTimeClear(&gl_channelHandle,&gl_evtId);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_DataGet()
{
  printf("\n------------ 11 : tet_DataGet -----------\n");
  var_initialize();
  
  tet_saEvtInitialize(&gl_evtHandle);
        
  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
    |SA_EVT_CHANNEL_PUBLISHER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
  
  gl_filterArray.filtersNumber=1;
  gl_filterArray.filters=gl_filter;
  tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

  tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

  gl_retentionTime=1000000000000.0;
  gl_patternArray.patternsNumber=2;
  gl_patternArray.patterns=gl_pattern;
  gl_allocatedNumber=2;
  gl_patternLength=5;
  tet_saEvtEventAttributesSet(&gl_eventHandle);

   strcpy (gl_eventData,"Publish"); 
  tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

  printf("\n NumUsers= 1 : NumSubscribers= 1 : NumPublishers= 1 \n");
  
    
  
  thread_creation(&gl_evtHandle);
   DISPATCH_SLEEP();

  tet_saEvtEventDataGet(&gl_eventDeliverHandle);

  tet_saEvtEventRetentionTimeClear(&gl_channelHandle,&gl_evtId);
  
  printf("\n ChannelNumRetainedEvents = 0 ");

  tet_saEvtEventFree(&gl_eventHandle);
  tet_saEvtChannelClose(&gl_channelHandle);

  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");

  
    
  
}
void DataGet_Thread()
{
  tet_saEvtEventDataGet(&gl_eventDeliverHandle);
}
void tet_DataGet_Thread()
{
  printf("\n------------ 4 : tet_DataGet -----------\n");
  tet_printf("\n------------ 4 : tet_DataGet -----------\n");
  var_initialize();
  
  tet_saEvtInitialize(&gl_evtHandle);
        
  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
    |SA_EVT_CHANNEL_PUBLISHER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
  
  gl_filterArray.filtersNumber=1;
  gl_filterArray.filters=gl_filter;
  tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

  tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

  gl_retentionTime=1000000000000.0;
  gl_patternArray.patternsNumber=2;
  gl_patternArray.patterns=gl_pattern;
  gl_allocatedNumber=2;
  gl_patternLength=5;
  tet_saEvtEventAttributesSet(&gl_eventHandle);

  strcpy (gl_eventData,"Publish");
  tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

  printf("\n NumUsers= 1 : NumSubscribers= 1 : NumPublishers= 1 \n");
  
    
  
  thread_creation(&gl_evtHandle);
   DISPATCH_SLEEP();

  /*DataGet thread*/
  if(tet_create_task((NCS_OS_CB)DataGet_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(2);
      fflush(stdout);
      /*Now Release the DataGet Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }
  tet_saEvtEventRetentionTimeClear(&gl_channelHandle,&gl_evtId);
  
  printf("\n ChannelNumRetainedEvents = 0 ");

  tet_saEvtEventFree(&gl_eventHandle);
  tet_saEvtChannelClose(&gl_channelHandle);

  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");

  
    
  
}

/*******saEvtEventPublish*******/

void tet_saEvtEventPublishCases(int iOption)
{
  printf("\n---------- tet_saEvtEventPublishCases : %d -------------",iOption);
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtEventPublish((SaEvtEventHandleT)(long)NULL,&gl_eventData,
                              gl_eventDataSize,&gl_evtId);
      result("saEvtEventPublish() with NULL event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 2:
      gl_eventHandle=(SaEvtEventHandleT)(long)NULL;

      gl_rc=saEvtEventPublish(gl_eventHandle,&gl_eventData,gl_eventDataSize,
                              &gl_evtId);
      result("saEvtEventPublish() with uninitialized event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtEventPublish(123,&gl_eventData,gl_eventDataSize,&gl_evtId);
      result("saEvtEventPublish() with garbage event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      gl_rc=saEvtEventPublish(gl_eventHandle,NULL,gl_eventDataSize,&gl_evtId);
      result("saEvtEventPublish() with NULL event data",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

       strcpy (gl_eventData,""); 
      gl_rc=saEvtEventPublish(gl_eventHandle,&gl_eventData,gl_eventDataSize,
                              &gl_evtId);
      result("saEvtEventPublish() with empty event data",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
     
    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      gl_rc=saEvtEventPublish(gl_eventHandle,&gl_eventData,(SaSizeT)(long)NULL,
                              &gl_evtId);
      result("saEvtEventPublish() with NULL event data size",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      gl_eventDataSize=0;
      gl_rc=saEvtEventPublish(gl_eventHandle,&gl_eventData,gl_eventDataSize,
                              &gl_evtId);
      result("saEvtEventPublish() with 0 event data size",SA_AIS_OK);
      gl_eventDataSize=20;

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      gl_rc=saEvtEventPublish(gl_eventHandle,&gl_eventData,gl_eventDataSize,
                              NULL);
      result("saEvtEventPublish() with NULL event id",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=4;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      gl_rc=saEvtEventPublish(gl_eventHandle,&gl_eventData,gl_eventDataSize,
                              &gl_evtId);
      result("saEvtEventPublish() with channel opened with create",
             SA_AIS_ERR_ACCESS);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break; 

    case 10:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_rc=saEvtEventAttributesSet(gl_eventHandle,NULL,gl_priority,
                                    gl_retentionTime,&gl_publisherName);
      resultSuccess("saEvtEventAttributesSet() invoked with NULL pattern \
array",gl_rc);      

       strcpy (gl_eventData,"Publish");
      gl_rc=saEvtEventPublish(gl_eventHandle,&gl_eventData,gl_eventDataSize,
                              &gl_evtId);
      result("saEvtEventPublish() with NULL pattern array",SA_AIS_OK);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 11:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      gl_eventDataSize=4097;
      gl_rc=saEvtEventPublish(gl_eventHandle,&gl_eventData,gl_eventDataSize,
                              &gl_evtId);
      result("saEvtEventPublish() with max event data size",
             SA_AIS_ERR_TOO_BIG);
      gl_eventDataSize=20;

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 12:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      strcpy (gl_eventData,"Publish"); 
      
      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);
      

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 13:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=4;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      gl_rc=saEvtEventPublish(gl_eventHandle,&gl_eventData,gl_eventDataSize,
                              &gl_evtId);
      result("saEvtEventPublish() with channel opened with subscribe access",
             SA_AIS_ERR_ACCESS);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break; 
    }
  
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_Publish()
{
  printf("\n------------ 7 : tet_Publish -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

  tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

  gl_patternArray.patternsNumber=2;
  gl_patternArray.patterns=gl_pattern;
  gl_retentionTime=100000000000000.0;
  gl_allocatedNumber=2;
  gl_patternLength=5;
  tet_saEvtEventAttributesSet(&gl_eventHandle);

  tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 1 \n");
  
    
  
  tet_saEvtEventRetentionTimeClear(&gl_channelHandle,&gl_evtId);
  tet_saEvtEventFree(&gl_eventHandle);
  tet_saEvtChannelClose(&gl_channelHandle);

  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");
  
    
}

/*******saEvtEventSubscribe*******/

void tet_saEvtEventSubscribeCases(int iOption)
{
  printf("\n---------- tet_saEvtEventSubscribeCases : %d -------------",
         iOption);
  SaEvtEventFilterT duplicate[2]=
    {
      {3,{5,5,(SaUint8T *)"Hello"}},        
      {3,{4,4,(SaUint8T *)"Hell"}}
    };
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtEventSubscribe((SaEvtChannelHandleT)(long)NULL,&gl_filterArray,
                                gl_subscriptionId);
      result("saEvtEventSubscribe() with NULL channel Handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 2:
      gl_channelHandle=(SaEvtChannelHandleT)(long)NULL;

      gl_rc=saEvtEventSubscribe(gl_channelHandle,&gl_filterArray,
                                gl_subscriptionId);
      result("saEvtEventSubscribe() with empty channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtEventSubscribe(123,&gl_filterArray,gl_subscriptionId);
      result("saEvtEventSubscribe() with garbage channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
        
    case 4:
      gl_version.releaseCode='B';
      gl_version.majorVersion=1;
      gl_version.minorVersion=1;
      gl_evtCallbacks.saEvtChannelOpenCallback=NULL;
      gl_evtCallbacks.saEvtEventDeliverCallback=NULL;

      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_rc=saEvtEventSubscribe(gl_channelHandle,&gl_filterArray,
                                gl_subscriptionId);
      result("saEvtEventSubscribe() with NULL event callbacks",
             SA_AIS_ERR_INIT);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_rc=saEvtEventSubscribe(gl_channelHandle,NULL,gl_subscriptionId);
      result("saEvtEventSubscribe() with NULL filter Array",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_filterArray.filtersNumber=(SaSizeT)(long)NULL;
      gl_filterArray.filters=NULL;
      gl_rc=saEvtEventSubscribe(gl_channelHandle,&gl_filterArray,
                                gl_subscriptionId);
      result("saEvtEventSubscribe() with empty filter array",SA_AIS_OK);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;

      gl_rc=saEvtEventSubscribe(gl_channelHandle,&gl_filterArray,
                                (SaEvtSubscriptionIdT)(long)NULL);
      result("saEvtEventSubscribe() with NULL subscription id",SA_AIS_OK);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      gl_rc=saEvtEventSubscribe(gl_channelHandle,&gl_filterArray,0);
      result("saEvtEventSubscribe() with 0 subscriptionId",SA_AIS_OK);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      gl_rc=saEvtEventSubscribe(gl_channelHandle,&gl_filterArray,1);
      result("saEvtEventSubscribe() with garbage subscription id",SA_AIS_OK);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 10:     
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_rc=saEvtEventSubscribe(gl_channelHandle,&gl_filterArray,
                                gl_subscriptionId);
      result("saEvtEventSubscribe() with channel not opened for subscriber",
             SA_AIS_ERR_ACCESS);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 11:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);    

      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      gl_rc=saEvtEventSubscribe(gl_channelHandle,&gl_filterArray,
                                gl_subscriptionId);
      result("saEvtEventSubscribe() with big filters number",SA_AIS_OK);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
            
    case 12:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
      
      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      gl_rc=saEvtEventSubscribe(gl_channelHandle,&gl_filterArray,
                                gl_subscriptionId);
      result("saEvtEventSubscribe() invoked",SA_AIS_OK);

      
      gl_filterArray.filtersNumber=2;
      gl_filterArray.filters=duplicate;
      gl_rc=saEvtEventSubscribe(gl_channelHandle,&gl_filterArray,
                                gl_subscriptionId);
      result("saEvtEventSubscribe() with duplicate subscription",
             SA_AIS_ERR_EXIST);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 13:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
      
      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);
      

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_Subscribe()
{
  printf("\n------------ 4 : tet_Subscribe -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
  
  gl_filterArray.filtersNumber=1;
  gl_filterArray.filters=gl_filter;

  tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);
  
  printf("\n NumUsers= 1 : NumSubscribers= 1 : NumPublishers= 0 \n");
  
    

  tet_saEvtChannelClose(&gl_channelHandle);

  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");
  
    
}
void Subscribe_Thread()
{
  gl_filterArray.filtersNumber=1;
  gl_filterArray.filters=gl_filter;

  tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);
}
void Unsubscribe_Thread()
{
  tet_saEvtEventUnsubscribe(&gl_channelHandle);
}
void tet_Subscribe_Thread()
{
  printf("\n------------ 3 : tet_Subscribe -----------\n");
  tet_printf("\n------------ 3 : tet_Subscribe -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
  
  /*Subscribe thread*/
  if(tet_create_task((NCS_OS_CB)Subscribe_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(2);
      fflush(stdout);
      /*Now Release the Subscribe Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    } 
  printf("\n NumUsers= 1 : NumSubscribers= 1 : NumPublishers= 0 \n");
  
    

  /*Unsubscribe thread*/
  if(tet_create_task((NCS_OS_CB)Unsubscribe_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);
        
      sleep(1);
      fflush(stdout);
      /*Now Release the Unsubscribe Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }

  tet_saEvtChannelClose(&gl_channelHandle);

  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");
  
    
}
/*******saEvtEventUnsubscribe*******/

void tet_saEvtEventUnsubscribeCases(int iOption)
{
  printf("\n---------- tet_saEvtEventUnsubscribeCases : %d -------------",
         iOption);
  switch(iOption)
    {
    case 1:
      gl_rc=saEvtEventUnsubscribe((SaEvtChannelHandleT)(long)NULL,gl_subscriptionId);
      result("saEvtEventUnsubscribe() with NULL gl_channelHandle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 2:
      
      gl_channelHandle=(SaEvtChannelHandleT)(long)NULL;

      gl_rc=saEvtEventUnsubscribe(gl_channelHandle,gl_subscriptionId);
      result("saEvtEventUnsubscribe() with empty channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      gl_rc=saEvtEventUnsubscribe(123,gl_subscriptionId);
      result("saEvtEventUnsubscribe() with garbage channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
    
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
      
      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter; 
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

      gl_rc=saEvtEventUnsubscribe(gl_channelHandle,(SaEvtSubscriptionIdT)(long)NULL);
      result("saEvtEventUnsubscribe() with NULL subscription id",
             SA_AIS_ERR_NOT_EXIST);
 
      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);
 
      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
    
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
      
      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter; 
      gl_subscriptionId=0;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

      gl_rc=saEvtEventUnsubscribe(gl_channelHandle,0);
      result("saEvtEventUnsubscribe() with 0 subscription id",SA_AIS_OK);
 
      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);
 
      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

      gl_rc=saEvtEventUnsubscribe(gl_channelHandle,123);
      result("saEvtEventUnsubscribe() with garbage subscription id",
             SA_AIS_ERR_NOT_EXIST);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);
      
      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);

      gl_subscriptionId=5;
      gl_rc=saEvtEventUnsubscribe(gl_channelHandle,gl_subscriptionId);
      result("saEvtEventUnsubscribe() with differnt channel handle for \
different subscription id",SA_AIS_ERR_NOT_EXIST);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_subscriptionId=3;
      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);
      
      
      tet_saEvtEventUnsubscribe(&gl_channelHandle);
      

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_Unsubscribe()
{
  printf("\n------------ 12 : tet_Unsubscribe -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

  gl_filterArray.filtersNumber=1;
  gl_filterArray.filters=gl_filter;
  
  tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);


  printf("\n NumUsers= 1 : NumSubscribers= 1 : NumPublishers= 0 \n");
  
    
  
  tet_saEvtEventUnsubscribe(&gl_channelHandle);
  
  printf("\n NumUsers= 1 : NumSubscribers= 1 : NumPublishers= 0 \n");

  tet_saEvtChannelClose(&gl_channelHandle);
  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);        

  printf("\n------------ END -----------\n");
  
    

}

/*******saEvtEventRetentionTimeClear*******/

void tet_saEvtEventRetentionTimeClearCases(int iOption)
{
  printf("\n---------- tet_saEvtEventRetentionTimeClearCases : %d -----------",
         iOption);
  SaEvtChannelHandleT channelTempHandle;
  SaEvtEventHandleT eventTempHandle;
  SaEvtEventIdT evtTempId;
  switch(iOption)
    {
    case 1:
      gl_evtId=1001;

      gl_rc=saEvtEventRetentionTimeClear((SaEvtChannelHandleT)(long)NULL,gl_evtId);
      result("saEvtEventRetentionTimeClear() with NULL channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 2:
      gl_evtId=1001;
      gl_channelHandle=(SaEvtChannelHandleT)(long)NULL;

      gl_rc=saEvtEventRetentionTimeClear(gl_channelHandle,gl_evtId);
      result("saEvtEventRetentionTimeClear() with empty channel handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;
      
    case 3:
      gl_evtId=1001;

      gl_rc=saEvtEventRetentionTimeClear(123,gl_evtId);
      result("saEvtEventRetentionTimeClear() with garbage channel handle",
             SA_AIS_ERR_BAD_HANDLE);        
      break;

    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_rc=saEvtEventRetentionTimeClear(gl_channelHandle,gl_evtId);
      result("saEvtEventRetentionTimeClear() with different channel handle",
             SA_AIS_ERR_NOT_EXIST);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
      
    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_rc=saEvtEventRetentionTimeClear(gl_channelHandle,(SaEvtEventIdT)(long)NULL);
      result("saEvtEventRetentionTimeClear() with NULL evt id",
             SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_rc=saEvtEventRetentionTimeClear(gl_channelHandle,1234);        
      result("saEvtEventRetentionTimeClear() with garbage evt id",
             SA_AIS_ERR_NOT_EXIST);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtChannelOpen(&gl_evtHandle,&channelTempHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      tet_saEvtEventAllocate(&channelTempHandle,&eventTempHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=100000000000.0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);
      gl_allocatedNumber=2;
      gl_patternLength=5;

      tet_saEvtEventAttributesSet(&eventTempHandle);

      tet_saEvtEventPublish(&eventTempHandle,&evtTempId);

      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      gl_rc=saEvtEventRetentionTimeClear(gl_channelHandle,100000);
      result("saEvtEventRetentionTimeClear() with different event id than\
 published",SA_AIS_ERR_NOT_EXIST);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtEventFree(&eventTempHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelClose(&channelTempHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=100000000000.0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

       strcpy (gl_eventData,"Publish");
      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      gl_evtId=3;
      gl_rc=saEvtEventRetentionTimeClear(gl_channelHandle,gl_evtId);
      result("saEvtEventRetentionTimeClear() with different event id(1-1000)\
 for channel handle",SA_AIS_ERR_INVALID_PARAM);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      gl_rc=saEvtEventRetentionTimeClear(gl_channelHandle,gl_evtId);
      result("saEvtEventRetentionTimeClear() with 0 retention time when \
published",SA_AIS_ERR_NOT_EXIST);

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
        
    case 10:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=100000000000.0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      
      tet_saEvtEventRetentionTimeClear(&gl_channelHandle,&gl_evtId);        
      

      tet_saEvtEventFree(&gl_eventHandle);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
        
    case 11:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);

      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=10000000000000.0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);

      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      tet_saEvtEventFree(&gl_eventHandle);

      gl_rc=saEvtEventRetentionTimeClear(gl_channelHandle,gl_evtId);
      gl_rc=saEvtEventRetentionTimeClear(gl_channelHandle,gl_evtId);
      result("saEvtEventRetentionTimeClear() with event free already invoked",
             SA_AIS_ERR_NOT_EXIST);

      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);

      tet_saEvtFinalize(&gl_evtHandle);
      break;
    }
  printf("\n-------------------- END : %d ------------------------",iOption);
}
void tet_RetentionTimeClear()
{
  printf("\n------------ 13 : tet_RetentionTimeClear -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

  tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

  gl_patternArray.patternsNumber=2;
  gl_patternArray.patterns=gl_pattern;
  gl_retentionTime=1000000000000.0;
  gl_allocatedNumber=2;
  gl_patternLength=5;
  tet_saEvtEventAttributesSet(&gl_eventHandle);

  tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 1 \n");
  
    
  
  tet_saEvtEventRetentionTimeClear(&gl_channelHandle,&gl_evtId);

  printf("\n ChannelNumRetainedEvents = 0 ");

  tet_saEvtChannelClose(&gl_channelHandle);
  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);        

  printf("\n------------ END -----------\n");
  
    

}
void RetentionTimeClear_Thread()
{
  tet_saEvtEventRetentionTimeClear(&gl_channelHandle,&gl_evtId);
}
void tet_RetentionTimeClear_Thread()
{
  printf("\n------------ 5 : tet_RetentionTimeClear_Thread -----------\n");
  tet_printf("\n------------ 5 : tet_RetentionTimeClear_Thread -----------\n");
  var_initialize();
  tet_saEvtInitialize(&gl_evtHandle);

  gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_PUBLISHER;
  tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

  tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);

  gl_patternArray.patternsNumber=2;
  gl_patternArray.patterns=gl_pattern;
  gl_retentionTime=1000000000000.0;
  gl_allocatedNumber=2;
  gl_patternLength=5;
  tet_saEvtEventAttributesSet(&gl_eventHandle);

  tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

  printf("\n NumUsers= 1 : NumSubscribers= 0 : NumPublishers= 1 \n");
  
    

  /*Clear thread*/
  if(tet_create_task((NCS_OS_CB)RetentionTimeClear_Thread)
     ==NCSCC_RC_SUCCESS )
    {
      printf("\tTask has been Created\t");fflush(stdout);

      sleep(1);
      fflush(stdout);
      /*Now Release the Clear Thread*/
      if(m_NCS_TASK_RELEASE(gl_t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
    }
  printf("\n ChannelNumRetainedEvents = 0 ");

  tet_saEvtChannelClose(&gl_channelHandle);
  tet_saEvtChannelUnlink(&gl_evtHandle);

  tet_saEvtFinalize(&gl_evtHandle);

  printf("\n------------ END -----------\n");
  
    

}

void tet_saEvtLimitGetCases(int iOption)
{
  printf("\n---------- tet_saEvtLimitGetCases : %d -------------",
         iOption);

  switch(iOption)
    {

    case 1:

      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      gl_rc=saEvtLimitGet(gl_evtHandle,gl_limitId,NULL);
      result("saEvtLimitGet() with NULL LimitValue",
             SA_AIS_ERR_INVALID_PARAM);
      break;

    case 2:
      m_NCS_MEMSET(&gl_limitValue,0,sizeof(SaLimitValueT));
      gl_rc=saEvtLimitGet((SaEvtHandleT)(long)NULL,gl_limitId,&gl_limitValue);
      result("saEvtLimitGet() with NULL event handle",
             SA_AIS_ERR_BAD_HANDLE);
      break;

    case 3:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      m_NCS_MEMSET(&gl_limitValue,0,sizeof(SaLimitValueT));
      gl_limitId = SA_EVT_MAX_RETENTION_DURATION_ID+1;
      gl_rc=saEvtLimitGet(gl_evtHandle,gl_limitId,&gl_limitValue);
      result("saEvtLimitGet() with biggerlimitId value",
             SA_AIS_ERR_INVALID_PARAM);
      break;
      
    case 4:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      m_NCS_MEMSET(&gl_limitValue,0,sizeof(SaLimitValueT));
      gl_limitId = 0;
      gl_rc=saEvtLimitGet(gl_evtHandle,gl_limitId,&gl_limitValue);
      result("saEvtLimitGet() with zero limitId value",
             SA_AIS_ERR_INVALID_PARAM);
      break;

    case 5:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      m_NCS_MEMSET(&gl_limitValue,0,sizeof(SaLimitValueT));
      gl_limitId = SA_EVT_MAX_NUM_CHANNELS_ID;
      gl_rc=saEvtLimitGet(gl_evtHandle,gl_limitId,&gl_limitValue);
      result("saEvtLimitGet() with limitId = Max Number of Channels",
             SA_AIS_OK);
      tet_printf("MaxNumChannels Limit Receieved = %ld",gl_limitValue.uint64Value);
      break;

    case 6:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      m_NCS_MEMSET(&gl_limitValue,0,sizeof(SaLimitValueT));
      gl_limitId = SA_EVT_MAX_EVT_SIZE_ID;
      gl_rc=saEvtLimitGet(gl_evtHandle,gl_limitId,&gl_limitValue);
      result("saEvtLimitGet() with limitId = Max Event Size",
             SA_AIS_OK);
      tet_printf("MaxEventSizeLimit Receieved = %ld",gl_limitValue.uint64Value);
      break;

    case 7:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      m_NCS_MEMSET(&gl_limitValue,0,sizeof(SaLimitValueT));
      gl_limitId = SA_EVT_MAX_PATTERN_SIZE_ID;
      gl_rc=saEvtLimitGet(gl_evtHandle,gl_limitId,&gl_limitValue);
      result("saEvtLimitGet() with limitId = Max Pattern Size",
             SA_AIS_OK);
      tet_printf("MaxPatternSizeLimit Receieved = %ld",gl_limitValue.uint64Value);
      break;

    case 8:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      m_NCS_MEMSET(&gl_limitValue,0,sizeof(SaLimitValueT));
      gl_limitId = SA_EVT_MAX_NUM_PATTERNS_ID;
      gl_rc=saEvtLimitGet(gl_evtHandle,gl_limitId,&gl_limitValue);
      result("saEvtLimitGet() with limitId = Max Num Patterns",
             SA_AIS_OK);
      tet_printf("MaxNumPatterns Limit Receieved = %ld",gl_limitValue.uint64Value);
      break;

    case 9:
      var_initialize();
      tet_saEvtInitialize(&gl_evtHandle);
      m_NCS_MEMSET(&gl_limitValue,0,sizeof(SaLimitValueT));
      gl_limitId = SA_EVT_MAX_RETENTION_DURATION_ID;
      gl_rc=saEvtLimitGet(gl_evtHandle,gl_limitId,&gl_limitValue);
      result("saEvtLimitGet() with limitId = Max RetentionTime Duration",
             SA_AIS_OK);
      tet_printf("MaxRetentionTime Duration Limit Receieved = %ld",gl_limitValue.timeValue);
      break;
    } /* End Switch */
  printf("\n------------ END -----------\n");

}
/*******Stress Test*******/

void tet_stressTest()
{
  for(;;)
    {        
      tet_saEvtInitialize(&gl_evtHandle);
                
      gl_channelOpenFlags=SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER
        |SA_EVT_CHANNEL_PUBLISHER;
      tet_saEvtChannelOpen(&gl_evtHandle,&gl_channelHandle);

      gl_filterArray.filtersNumber=1;
      gl_filterArray.filters=gl_filter;
      tet_saEvtEventSubscribe(&gl_channelHandle,&gl_subscriptionId);
                
      tet_saEvtEventAllocate(&gl_channelHandle,&gl_eventHandle);
        
      gl_patternArray.patternsNumber=2;
      gl_patternArray.patterns=gl_pattern;
      gl_retentionTime=10000000000.0;
      gl_allocatedNumber=2;
      gl_patternLength=5;
      tet_saEvtEventAttributesSet(&gl_eventHandle);
       strcpy (gl_eventData,"Publish");  
      tet_saEvtEventPublish(&gl_eventHandle,&gl_evtId);

      tet_saEvtEventFree(&gl_eventHandle);
                
      tet_saEvtChannelClose(&gl_channelHandle);

      tet_saEvtChannelUnlink(&gl_evtHandle);
                
      tet_saEvtFinalize(&gl_evtHandle);
       DISPATCH_SLEEP();
    }
}

/******User Defs******/

#if 0

void resultSuccess(char *gl_saf_msg,SaAisErrorT rc)
{
  if(gl_hide==0&&iCmpCount!=8)
    {
      printf("\n----------------------------------------");
      printf("\n\n\n%s",&gl_saf_msg[0]);
      if(gl_cbk!=1)
        {
          tet_printf("%s",&gl_saf_msg[0]);
        }
      if(rc!=SA_AIS_OK)
        {
          printf("\n\nFAILURE : ");
          printf("%s\n",gl_saf_error[rc]);
          if(gl_cbk!=1)
            {
              tet_printf("Failure : %s",gl_saf_error[rc]);
              tet_result(TET_FAIL);
            }
          gl_err=rc;
        }
      else
        {
          printf("\n\nSUCCESS : ");
          printf("%s\n",gl_saf_error[rc]);
          if(gl_cbk!=1)
            {
              tet_printf("Success : %s",gl_saf_error[rc]);
              tet_result(TET_PASS);
            }
        }
      if(gl_error==1)
        {
          gl_error=rc;
        }
      printf("\n----------------------------------------");
    }
  gl_cbk=0;
}

#endif
#if 0
Rajesh
void result(char *gl_saf_msg,SaAisErrorT ref)
{
  printf("\n\n\n%s",&gl_saf_msg[0]);
  tet_printf("%s",&gl_saf_msg[0]);
  if(gl_rc==ref)
    {
      printf("\n\nSUCCESS : ");
      tet_printf("Success : %s",gl_saf_error[gl_rc]);
      tet_result(TET_PASS);
    }
  else             
    {
      printf("\n\nFAILURE : ");
      tet_printf("Failure : %s",gl_saf_error[gl_rc]);
      tet_result(TET_FAIL); 
      gl_error=gl_rc;
    }
  printf("%s\n",gl_saf_error[gl_rc]);
}

void resultSuccess(char *gl_saf_msg,SaAisErrorT rc)
{
  if(iCmpCount!=8)
    {
      printf("\n----------------------------------------");
      printf("\n\n\n%s",&gl_saf_msg[0]);
      if(gl_cbk!=1)
        {
          tet_printf("%s",&gl_saf_msg[0]);
        }
      if(rc!=SA_AIS_OK)
        {
          printf("\n\nFAILURE : ");
          printf("%s\n",gl_saf_error[rc]);
          if(gl_cbk!=1)
            {
              tet_printf("Failure : %s",gl_saf_error[rc]);
              tet_result(TET_FAIL);
            }
          gl_err=rc; 
        }       
      else
        {
          printf("\n\nSUCCESS : ");
          printf("%s\n",gl_saf_error[rc]);
          if(gl_cbk!=1)
            {
              tet_printf("Success : %s",gl_saf_error[rc]);
              tet_result(TET_PASS);
            }
        }
      if(gl_error==1)
        {
          gl_error=rc;
        }
      printf("\n----------------------------------------");
    }
  gl_cbk=0;
}
#endif
struct tet_testlist edsv_test[]=
  {
    {tet_saEvtInitializeCases,1,1},
    {tet_saEvtInitializeCases,1,2},
#if 0 
    /*    {tet_saEvtInitializeCases,1,3},*/
    {tet_saEvtInitializeCases,1,4},
    {tet_saEvtInitializeCases,1,5},
    {tet_saEvtInitializeCases,1,6},
    {tet_saEvtInitializeCases,1,7},
    {tet_saEvtInitializeCases,1,8},
    {tet_saEvtInitializeCases,1,9},
    {tet_saEvtInitializeCases,1,10},
    {tet_saEvtInitializeCases,1,11},
    {tet_saEvtInitializeCases,1,12},
    {tet_saEvtInitializeCases,1,13},
    {tet_saEvtInitializeCases,1,14},

    {tet_saEvtSelectionObjectGetCases,2,1},
    {tet_saEvtSelectionObjectGetCases,2,2},
    {tet_saEvtSelectionObjectGetCases,2,3},
    {tet_saEvtSelectionObjectGetCases,2,4},
    {tet_saEvtSelectionObjectGetCases,2,5},
    {tet_saEvtSelectionObjectGetCases,2,6},
    {tet_saEvtSelectionObjectGetCases,2,7},
    {tet_saEvtSelectionObjectGetCases,2,8},
    {tet_saEvtSelectionObjectGetCases,2,9},

    {tet_saEvtDispatchCases,3,1},
    {tet_saEvtDispatchCases,3,2},
    {tet_saEvtDispatchCases,3,3},
    {tet_saEvtDispatchCases,3,4},
    {tet_saEvtDispatchCases,3,5},
    {tet_saEvtDispatchCases,3,6},
    {tet_saEvtDispatchCases,3,7},
    /*  {tet_saEvtDispatchCases,3,8},*/
    {tet_saEvtDispatchCases,3,9},

    {tet_saEvtFinalizeCases,4,1},
    {tet_saEvtFinalizeCases,4,2},
    {tet_saEvtFinalizeCases,4,3},
    {tet_saEvtFinalizeCases,4,4},

    {tet_saEvtChannelOpenCases,5,1},
    {tet_saEvtChannelOpenCases,5,2},
    {tet_saEvtChannelOpenCases,5,3},
    {tet_saEvtChannelOpenCases,5,4},
    {tet_saEvtChannelOpenCases,5,5},
    {tet_saEvtChannelOpenCases,5,6},
    {tet_saEvtChannelOpenCases,5,7},
    {tet_saEvtChannelOpenCases,5,8},
    {tet_saEvtChannelOpenCases,5,9},
    {tet_saEvtChannelOpenCases,5,10},
    {tet_saEvtChannelOpenCases,5,11},
    {tet_saEvtChannelOpenCases,5,12},
    {tet_saEvtChannelOpenCases,5,13},
    {tet_saEvtChannelOpenCases,5,14},
    {tet_saEvtChannelOpenCases,5,15},
    {tet_saEvtChannelOpenCases,5,16},

    {tet_saEvtChannelOpenAsyncCases,6,1},
    {tet_saEvtChannelOpenAsyncCases,6,2},
    {tet_saEvtChannelOpenAsyncCases,6,3},
    {tet_saEvtChannelOpenAsyncCases,6,4},
    {tet_saEvtChannelOpenAsyncCases,6,5},
    {tet_saEvtChannelOpenAsyncCases,6,6},
    {tet_saEvtChannelOpenAsyncCases,6,7},
    {tet_saEvtChannelOpenAsyncCases,6,8},
    {tet_saEvtChannelOpenAsyncCases,6,9},
    {tet_saEvtChannelOpenAsyncCases,6,10},
    {tet_saEvtChannelOpenAsyncCases,6,11},
    /*  {tet_saEvtChannelOpenAsyncCases,6,12},*/
    {tet_saEvtChannelOpenAsyncCases,6,13},
    {tet_saEvtChannelOpenAsyncCases,6,14},
    {tet_saEvtChannelOpenAsyncCases,6,15},
    {tet_saEvtChannelOpenAsyncCases,6,16},
    {tet_saEvtChannelOpenAsyncCases,6,17},
    {tet_saEvtChannelOpenAsyncCases,6,18},

    {tet_saEvtChannelCloseCases,7,1},
    {tet_saEvtChannelCloseCases,7,2},
    {tet_saEvtChannelCloseCases,7,3},
    {tet_saEvtChannelCloseCases,7,4},

    {tet_saEvtChannelUnlinkCases,8,1},
    {tet_saEvtChannelUnlinkCases,8,2},
    {tet_saEvtChannelUnlinkCases,8,3},
    {tet_saEvtChannelUnlinkCases,8,4},
    {tet_saEvtChannelUnlinkCases,8,5},
    {tet_saEvtChannelUnlinkCases,8,6},
    {tet_saEvtChannelUnlinkCases,8,7},
    {tet_saEvtChannelUnlinkCases,8,8},

    {tet_saEvtEventAllocateCases,9,1},
    {tet_saEvtEventAllocateCases,9,2},
    {tet_saEvtEventAllocateCases,9,3},
    {tet_saEvtEventAllocateCases,9,4},
    {tet_saEvtEventAllocateCases,9,5},
    {tet_saEvtEventAllocateCases,9,6},
    {tet_saEvtEventAllocateCases,9,7},

    {tet_saEvtEventFreeCases,10,1},
    {tet_saEvtEventFreeCases,10,2},
    {tet_saEvtEventFreeCases,10,3},
    {tet_saEvtEventFreeCases,10,4},
    {tet_saEvtEventFreeCases,10,5},
    /*    {tet_saEvtEventFreeCases,10,6},*/

    {tet_saEvtEventAttributesSetCases,11,1},
    {tet_saEvtEventAttributesSetCases,11,2},
    {tet_saEvtEventAttributesSetCases,11,3},
    {tet_saEvtEventAttributesSetCases,11,4},
    {tet_saEvtEventAttributesSetCases,11,5},
    {tet_saEvtEventAttributesSetCases,11,6},
    {tet_saEvtEventAttributesSetCases,11,7},
    {tet_saEvtEventAttributesSetCases,11,8},
    {tet_saEvtEventAttributesSetCases,11,9},
    {tet_saEvtEventAttributesSetCases,11,10},
    {tet_saEvtEventAttributesSetCases,11,11},
    {tet_saEvtEventAttributesSetCases,11,12},
    {tet_saEvtEventAttributesSetCases,11,13},
    {tet_saEvtEventAttributesSetCases,11,14},
    {tet_saEvtEventAttributesSetCases,11,15},
    {tet_saEvtEventAttributesSetCases,11,16},
    {tet_saEvtEventAttributesSetCases,11,17},

    {tet_saEvtEventAttributesGetCases,12,1},
    {tet_saEvtEventAttributesGetCases,12,2},
    {tet_saEvtEventAttributesGetCases,12,3},
    {tet_saEvtEventAttributesGetCases,12,4},
    {tet_saEvtEventAttributesGetCases,12,5},
    {tet_saEvtEventAttributesGetCases,12,6},
    {tet_saEvtEventAttributesGetCases,12,7},
    {tet_saEvtEventAttributesGetCases,12,8},
    {tet_saEvtEventAttributesGetCases,12,9},
    {tet_saEvtEventAttributesGetCases,12,10},
    {tet_saEvtEventAttributesGetCases,12,11},
    {tet_saEvtEventAttributesGetCases,12,12},
    {tet_saEvtEventAttributesGetCases,12,13},
    {tet_saEvtEventAttributesGetCases,12,14},
    
    {tet_saEvtEventDataGetCases,13,1},
    {tet_saEvtEventDataGetCases,13,2},
    {tet_saEvtEventDataGetCases,13,3},
   /* {tet_saEvtEventDataGetCases,13,4},
      {tet_saEvtEventDataGetCases,13,5},
      {tet_saEvtEventDataGetCases,13,6},*/
    {tet_saEvtEventDataGetCases,13,7},
    {tet_saEvtEventDataGetCases,13,8},
    {tet_saEvtEventDataGetCases,13,9},
    {tet_saEvtEventDataGetCases,13,10},

    {tet_saEvtEventPublishCases,14,1},  
    {tet_saEvtEventPublishCases,14,2},
    {tet_saEvtEventPublishCases,14,3},
    {tet_saEvtEventPublishCases,14,4},
    {tet_saEvtEventPublishCases,14,5},
    {tet_saEvtEventPublishCases,14,6},
    {tet_saEvtEventPublishCases,14,7},
    {tet_saEvtEventPublishCases,14,8},
    {tet_saEvtEventPublishCases,14,9},
    {tet_saEvtEventPublishCases,14,10},
    {tet_saEvtEventPublishCases,14,11},
    {tet_saEvtEventPublishCases,14,12},
    {tet_saEvtEventPublishCases,14,13},

    {tet_saEvtEventSubscribeCases,15,1},
    {tet_saEvtEventSubscribeCases,15,2},
    {tet_saEvtEventSubscribeCases,15,3},
    {tet_saEvtEventSubscribeCases,15,4},
    {tet_saEvtEventSubscribeCases,15,5},
    {tet_saEvtEventSubscribeCases,15,6},
    {tet_saEvtEventSubscribeCases,15,7},
    {tet_saEvtEventSubscribeCases,15,8},
    {tet_saEvtEventSubscribeCases,15,9},
    {tet_saEvtEventSubscribeCases,15,10},
    {tet_saEvtEventSubscribeCases,15,11},
    {tet_saEvtEventSubscribeCases,15,12},
    {tet_saEvtEventSubscribeCases,15,13},

    {tet_saEvtEventUnsubscribeCases,16,1},
    {tet_saEvtEventUnsubscribeCases,16,2},
    {tet_saEvtEventUnsubscribeCases,16,3},
    {tet_saEvtEventUnsubscribeCases,16,4},
    {tet_saEvtEventUnsubscribeCases,16,5},
    {tet_saEvtEventUnsubscribeCases,16,6},
    {tet_saEvtEventUnsubscribeCases,16,7},
    {tet_saEvtEventUnsubscribeCases,16,8},

    {tet_saEvtEventRetentionTimeClearCases,17,1},
    {tet_saEvtEventRetentionTimeClearCases,17,2},
    {tet_saEvtEventRetentionTimeClearCases,17,3},
    {tet_saEvtEventRetentionTimeClearCases,17,4},
    {tet_saEvtEventRetentionTimeClearCases,17,5},
    {tet_saEvtEventRetentionTimeClearCases,17,6},
    {tet_saEvtEventRetentionTimeClearCases,17,7},
    {tet_saEvtEventRetentionTimeClearCases,17,8},
    {tet_saEvtEventRetentionTimeClearCases,17,9},
    {tet_saEvtEventRetentionTimeClearCases,17,10},
    {tet_saEvtEventRetentionTimeClearCases,17,11},
#endif
    {NULL,0}
  };
struct tet_testlist api_test[]=
  {
    {tet_Initialize,1},
#if 0 
    {tet_ChannelOpen,2},
    {tet_ChannelOpenAsync,3},
    {tet_Subscribe,4},
    {tet_Allocate,5},
    {tet_AttributesSet,6},
    {tet_Publish,7},
    {tet_SelectionObjectGet,8},
    {tet_Dispatch,9},
    {tet_AttributesGet,10},
    {tet_DataGet,11},
    {tet_Unsubscribe,12},
    {tet_RetentionTimeClear,13},
    {tet_Free,14},
    {tet_ChannelClose,15},
    {tet_ChannelUnlink,16},
    {tet_Finalize,17},
#endif
    {NULL,0}
  };
struct tet_testlist func_test[]=
  {
    {tet_ChannelOpen_SingleEvtHandle,1},
#if 0 
    {tet_ChannelOpen_SingleEvtHandle_SamePatterns,2},
    {tet_ChannelOpen_SingleEvtHandle_DifferentPatterns,3},
    {tet_ChannelOpen_MultipleEvtHandle,4},
    {tet_ChannelOpen_MultipleEvtHandle_SamePatterns,5}, /*run in a loop failing*/
    {tet_ChannelOpen_MultipleEvtHandle_DifferentPatterns,6},
    {tet_ChannelOpen_MultipleOpen,7},
    {tet_ChannelClose_Simple,8},
    {tet_ChannelClose_Subscriber,9},
    {tet_ChannelUnlink_Simple,10},
    {tet_ChannelUnlink_OpenInstance,11},
    {tet_EventFree_Simple,12},
    {tet_AttributesSet_DeliverCallback,13},
    {tet_AttributesGet_ReceivedEvent,14},
    {tet_EventPublish_NullPatternArray,15}, 
    {tet_EventSubscribe_PrefixFilter,16},
    {tet_EventSubscribe_SuffixFilter,17},
    {tet_EventSubscribe_ExactFilter,18},
    {tet_EventSubscribe_AllPassFilter,19},
    {tet_EventSubscribe_LessFilters,20},
    {tet_EventSubscribe_LessPatterns,21},
    {tet_EventSubscribe_LessPatterns_FilterSize,22},
    {tet_EventSubscribe_LessFilters_DifferentOrder,23},
    {tet_EventSubscribe_PatternMatch_DifferentSubscriptions,24}, /*snmp printfs added till here*/
    {tet_ChannelOpenAsync_Simple,25},
    {tet_EventSubscribe_multipleFilters,26},
    {tet_EventSubscribe_PrefixFilter_NoMatch,27},
    {tet_EventSubscribe_SuffixFilter_NoMatch,28},
    {tet_EventSubscribe_ExactFilter_NoMatch,29},
    {tet_EventSubscribe_DiffFilterTypes,30},
    {tet_EventSubscribe_PrefixFilter_FilterSize,31},
    {tet_EventPublish_NullPatternArray_AttributesSet,32},
    {tet_Complex_Subscriber_Publisher,33},
    {tet_Same_Subscriber_Publisher_DiffChannel,34},
    {tet_Sync_Async_Subscriber_Publisher,35},
    {tet_Event_Subscriber_Unsubscriber,36},
    {tet_EventDeliverCallback_AttributesSet_RetentionTimeClear,37},
    {tet_MultipleHandle_Subscribers_Publisher,38},
    {tet_EventPublish_Priority,39},
    {tet_EventPublish_EventOrder,40},
    {tet_EventPublish_EventFree,41},
    {tet_EventFree_DeliverCallback,42},
    {tet_EventRetentionTimeClear_EventFree,43},
    {tet_EventPublish_ChannelUnlink,44},
    {tet_EventUnsubscribe_EventPublish,45},
    {tet_EventAttributesGet_LessPatternsNumber,46},
    {tet_Simple_Test,47},
#endif
#if gl_red == 1
#endif
    {NULL,0}
  };

struct tet_testlist b03_test[]=
  {
    {tet_saEvtInitializeCases,1,15},
    {tet_saEvtInitializeCases,1,16}, /* Manual test for SA_AIS_ERR_UNAVAILABLE */
    {tet_saEvtLimitGetCases,2,1},
    {tet_saEvtLimitGetCases,2,2},
    {tet_saEvtLimitGetCases,2,3},
    {tet_saEvtLimitGetCases,2,4},
    {tet_saEvtLimitGetCases,2,5},
    {tet_saEvtLimitGetCases,2,6},
    {tet_saEvtLimitGetCases,2,7},
    {tet_saEvtLimitGetCases,2,8},
    {tet_saEvtLimitGetCases,2,9},
    {tet_saEvtEventAttributesGetCases,3,15},/*saEvtEventPatternFree cases by EventAlloc*/
    {tet_saEvtEventAttributesGetCases,3,16},
    {tet_saEvtEventAttributesGetCases,3,17},
    {tet_PatternFree_ReceivedEvent,18},
    {NULL,0}
  };

struct tet_testlist *edsv_testlist[]=
  {
    [EDSV_TEST]=edsv_test,
    [EDSV_API_TEST] = api_test,
    [EDSV_FUNC_TEST] = func_test,
    [EDSV_CLM_TEST] = b03_test,
  };

void tet_edsv_startup()
{
#ifdef TET_SMOKE_TEST
  edsv_smoke_test();
  return;
#endif
   
#ifdef TET_EDSV
  tet_run_edsv_app();
#endif
}
void tet_edsv_cleanup()
{
  tet_test_cleanup();
  return;
}
void gl_defs()
{
  int length=0;
  char *temp=NULL;
  gl_edsv_defs gl_struct;

  printf("\n\n********************\n");
  printf("\nGlobal structure variables\n");

  
  temp=(char *)getenv("TET_CASE");
  if(temp)
    {
      gl_struct.tCase=atoi(temp);
    }
  else
    {
      gl_struct.tCase=-1;
    }
  gl_tCase=gl_struct.tCase;
  printf("\nTest Case Number: %d",gl_tCase);

  temp=(char *)getenv("TET_ITERATION");
  if(temp)
    {
      gl_struct.iteration=atoi(temp);
    }
  else
    {
      gl_struct.iteration=1;
    }
  gl_iteration=gl_struct.iteration;
  printf("\nNumber of iterations: %d",gl_iteration);

  temp=(char *)getenv("TET_LIST_NUMBER");
  if(temp&&(atoi(temp)>0&&(atoi(temp)<=6)))
    {
      gl_struct.listNumber=atoi(temp);
    }
  else
    {
      gl_struct.listNumber=-1;
    }
  gl_listNumber=gl_struct.listNumber;
  printf("\nTest List to be executed: %d",gl_listNumber);

  temp=(char *)getenv("TET_CHANNEL_NAME");
  if(temp)
    {
      gl_struct.channelName.length=strlen(temp);
      strcpy(gl_struct.channelName.value,temp);
    }
  else
    {
      gl_struct.channelName.length=12;
      strcpy(gl_struct.channelName.value,"tempChannel");
    }
  gl_channelName.length=gl_struct.channelName.length;
  strcpy(gl_channelName.value,gl_struct.channelName.value);
  printf("\nChannel Name: ");
  while(length<gl_channelName.length)
    {
      printf("%c",gl_channelName.value[length]);
      length++;
    }

  temp=(char *)getenv("TET_PUBLISHER_NAME");
  if(temp)
    {
      gl_struct.publisherName.length=strlen(temp);
      strcpy(gl_struct.publisherName.value,temp);
    }
  else
    {
      gl_struct.publisherName.length=9;
      strcpy(gl_struct.publisherName.value,"Anonymous");
    }
  m_NCS_MEMSET(gl_publisherName.value,'\0',sizeof(SaNameT));
  gl_publisherName.length=gl_struct.publisherName.length;
  strcpy(gl_publisherName.value,gl_struct.publisherName.value);
  printf("\nPublisher Name: ");
  length=0;
  while(length<gl_publisherName.length)
    {
      printf("%c",gl_publisherName.value[length]);
      length++;
    }

  temp=(char *)getenv("TET_EVENTDATA_SIZE");
  if(temp)
    {
      gl_struct.eventDataSize=atoi(temp);
    }
  else
    {
      gl_struct.eventDataSize=20;
    }
  gl_eventDataSize=gl_struct.eventDataSize;
  printf("\nEventData size: %llu",gl_eventDataSize);

  temp=(char *)getenv("TET_SUBSCRIPTION_ID");
  if(temp)
    {
      gl_struct.subscriptionId=atoi(temp);
    }
  else
    {
      gl_struct.subscriptionId=3;
    }
  gl_subscriptionId=gl_struct.subscriptionId;
  printf("\nSubscription Id: %u",gl_subscriptionId);

  printf("\n\n********************");
}

void tet_run_edsv_app()
{
  int iterCount=0,listCount=0;
  gl_defs();
      tet_test_start(gl_tCase,b03_test);
#if 0 
#ifndef TET_ALL

  tware_mem_ign();
  for(iterCount=1;iterCount<=gl_iteration;iterCount++)
    {
      printf("\n---------------- ITERATION : %d --------------\n",iterCount);
      tet_printf("\n-------------- ITERATION : %d --------------\n",iterCount);
      if(gl_listNumber==-1)
        {
            {
              for(listCount=1;listCount<=4;listCount++)
                {
                  tet_test_start(gl_tCase,edsv_testlist[listCount]);
                }
            }
        }
      else
        {
          if(gl_listNumber!=4)
            {
              tet_test_start(gl_tCase,edsv_testlist[gl_listNumber]);
            }
          
        }
      printf("\nNumber of iterations: %d",iterCount);
      tet_printf("\nNumber of iterations: %d",iterCount);
      sleep(2);
    }
  printf("\nPRESS ENTER TO GET MEM DUMP");

  getchar();

  tware_mem_dump();
  sleep(2);
  tware_mem_dump();
  tet_edsv_cleanup(); 

#else

  while(1)
    {
      tet_test_start(gl_tCase,edsv_test);

      tet_test_start(gl_tCase,api_test);

      tet_test_start(gl_tCase,func_test);

      tet_test_start(gl_tCase,b03_test);

    }
#endif
#endif
}

void tet_run_edsv_dist_cases()
{
}
