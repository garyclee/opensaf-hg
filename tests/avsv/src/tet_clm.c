
#include "tet_api.h"
/* ncs_files .... */
#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"
#include "ncssysf_ipc.h"


#include <saClm.h>
void tware_clm_cluster_trk(void);
void tware_clm_cluster_node_get(void);
void tware_clm_cluster_node_get_async(void);
void tware_clm_cluster_track_stop(void);
void tware_clm_succ_init(void);
void tware_clm_cluster_trk_cbk(const SaClmClusterNotificationBufferT *notif_buffer,
                               SaUint32T num_members,
                               SaAisErrorT error);
void tware_clm_node_get_cbk(SaInvocationT invocation,
                            const SaClmClusterNodeT *cluster_node,
                            SaAisErrorT error);
                            
void tware_clm_intialize(int sub_test_arg);
void tware_clm_selobj(int sub_test_arg);

#ifdef TET_A
SaVersionT gClmVersion;


#define m_TET_GET_CLM_VER(clm_ver) clm_ver.releaseCode='B'; clm_ver.majorVersion=0x01; clm_ver.minorVersion=0x01;
SaClmHandleT gClmHandle;

#define tet_print  printf

SYSF_MBX       g_tware_clm_mbx;

void tware_clm_cluster_trk_cbk(const SaClmClusterNotificationBufferT *notif_buffer,
                               SaUint32T num_members,
                               SaAisErrorT error)
{


  tet_print(" The tware_clm_cluster_trk_cbk is called \n");
  tet_infoline(" The tware_clm_cluster_trk_cbk is called \n");


  tet_print("The Number of Members in the cluster :%d \n\n",num_members); 
 
  tet_print("The Notif Buffer contents num_of_items %d \n:",notif_buffer->numberOfItems);

/*  free(notif_buffer); */
  return;

}


void tware_clm_node_get_cbk(SaInvocationT invocation,
                            const SaClmClusterNodeT *cluster_node,
                            SaAisErrorT error)
{

  tet_print(" The invocation num recevied is %llu \n",invocation);
  tet_print(" The node_id  recevied is %d \n",cluster_node->nodeId);

  return;
}       


void tware_clm_intialize(int sub_test_arg)
{
  SaAisErrorT  clm_err;
  SaClmCallbacksT  ClmCallbacks;  

  ClmCallbacks.saClmClusterNodeGetCallback = tware_clm_node_get_cbk;
  ClmCallbacks.saClmClusterTrackCallback   = tware_clm_cluster_trk_cbk;
 
  switch (sub_test_arg)
  {
   default:

      m_TET_GET_CLM_VER(gClmVersion);
      clm_err = saClmInitialize(&gClmHandle,&ClmCallbacks,
                          &gClmVersion);

      if (clm_err != SA_AIS_OK)
      {
        tet_infoline("CLM INIT FAILED ");
        tet_printf("CLM INIT FAILED with error %d ",clm_err);
      }
  }
}


void tware_clm_selobj(int sub_test_arg)
{
    NCS_SEL_OBJ_SET     all_sel_obj;
        NCS_SEL_OBJ         mbx_fd = m_NCS_IPC_GET_SEL_OBJ(& g_tware_clm_mbx);    
    SaSelectionObjectT  amf_sel_obj;
    SaAisErrorT            amf_error;   
    NCS_SEL_OBJ         ams_ncs_sel_obj;
    NCS_SEL_OBJ         high_sel_obj;

    m_NCS_SEL_OBJ_ZERO(&all_sel_obj);    
    m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);

    amf_error = saClmSelectionObjectGet(gClmHandle, &amf_sel_obj);

    if (amf_error != SA_AIS_OK)
    {     
        tet_infoline(" DAMNIT: saClmSelectionObjectGet failed \n");       
        return;
    }
    m_SET_FD_IN_SEL_OBJ(amf_sel_obj,ams_ncs_sel_obj);
    m_NCS_SEL_OBJ_SET(ams_ncs_sel_obj, &all_sel_obj);

        high_sel_obj = m_GET_HIGHER_SEL_OBJ(ams_ncs_sel_obj,mbx_fd);

    
    while (m_NCS_SEL_OBJ_SELECT(high_sel_obj,&all_sel_obj,0,0,NULL) != -1)
    {      
        /* process all the AMF messages */
        if (m_NCS_SEL_OBJ_ISSET(ams_ncs_sel_obj,&all_sel_obj))
        {
                  saClmDispatch(gClmHandle,SA_DISPATCH_ALL);
                }

        }
}



void tware_clm_cluster_trk()
{

  SaClmClusterNotificationBufferT notif_buffer;
  SaClmClusterNotificationT   not_buf;
  SaUint8T  trk_flag=SA_TRACK_CURRENT;
  SaAisErrorT error;
  
  notif_buffer.viewNumber = 0;
  notif_buffer.numberOfItems =1;
  notif_buffer.notification = &not_buf;
 
  if(( error =  saClmClusterTrack(gClmHandle,
                    trk_flag,
                    NULL)) !=SA_AIS_OK)
   {
      tet_infoline("ERROR: saClmClusterTrack  Failed");
      printf("ERROR: saClmClusterTrack  Failed\n");
      tet_printf("ERROR: saClmClusterTrack  Failed\n");
      return;
   } 


      printf("saClmClusterTrack PASSED\n ");
      tet_printf("saClmClusterTrack PASSED\n ");
      tet_infoline("saClmClusterTrack PASSED\n ");
      return;
}


void tware_clm_cluster_node_get()
{
   SaAisErrorT  error;
   SaTimeT      time_out;
   SaClmClusterNodeT cluster_node;
   SaClmNodeIdT  node_id= SA_CLM_LOCAL_NODE_ID;
 

   if((error = saClmClusterNodeGet(gClmHandle,
                                   node_id,
                                   time_out,
                                   &cluster_node)) != SA_AIS_OK)
    {

      tet_infoline("ERROR: saClmClusterNodeGet  Failed");
      tet_print("ERROR: saClmClusterNodeGet  Failedi\n");
      return;

    }
      tet_infoline("saClmClusterNodeGet PASSED ");
      tet_print("saClmClusterNodeGet PASSED ");
      return;
}

void tware_clm_cluster_node_get_async()
{
   SaAisErrorT  error = SA_AIS_OK;
   SaInvocationT invocation= 1234;
   SaClmNodeIdT  node_id = SA_CLM_LOCAL_NODE_ID;


   if((error = saClmClusterNodeGetAsync(gClmHandle,
                                   invocation,
                                   node_id)) != SA_AIS_OK)
    {

      tet_print("ERROR: saClmClusterNodeGetAsync  Failed");
      tet_infoline("ERROR: saClmClusterNodeGetAsync  Failed");
      return;

    }
      tet_print("saClmClusterNodeGetAsync PASSED ");
      tet_infoline("saClmClusterNodeGetAsync PASSED ");
      return;
}


void tware_clm_cluster_track_stop()
{
  SaAisErrorT error;

  if((error = saClmClusterTrackStop(gClmHandle)) != SA_AIS_OK)
  {
      tet_infoline("ERROR: saClmClusterTrackStop Failed");
      return;


  }

      tet_infoline("saClmClusterTrackStop PASSED ");
      return;

}


void tware_clm_succ_init()
{

uint32_t rc = NCSCC_RC_SUCCESS ; 

        rc = m_NCS_IPC_CREATE(&g_tware_clm_mbx); 
        
        rc = m_NCS_IPC_ATTACH(&g_tware_clm_mbx);

  tware_clm_intialize(1);

  tware_clm_cluster_node_get_async();
  tware_clm_cluster_trk();

/*  tware_clm_cluster_node_get();  */
  tware_clm_selobj(1);   
 /* tware_clm_cluster_node_get(); */

}

#ifdef TET_THREAD
void tware_create_clm_thread()
{

 pthread_t new_thread;

tet_pthread_create(&new_thread, NULL, tware_clm_succ_init,NULL,20); 


}
#endif

#endif
