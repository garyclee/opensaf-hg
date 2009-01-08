/*****************************************************************************
 *                                                                           *
 * NOTICE TO PROGRAMMERS:  RESTRICTED USE.                                   *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED TO YOU AND YOUR COMPANY UNDER A LICENSE         *
 * AGREEMENT FROM EMERSON, INC.                                              *
 *                                                                           *
 * THE TERMS OF THE LICENSE AGREEMENT RESTRICT THE USE OF THIS SOFTWARE TO   *
 * SPECIFIC PROJECTS ("LICENSEE APPLICATIONS") IDENTIFIED IN THE AGREEMENT.  *
 *                                                                           *
 * IF YOU ARE UNSURE WHETHER YOU ARE AUTHORIZED TO USE THIS SOFTWARE ON YOUR *
 * PROJECT,  PLEASE CHECK WITH YOUR MANAGEMENT.                              *
 *                                                                           *
 *****************************************************************************


..............................................................................

DESCRIPTION:

******************************************************************************
*/


#include "fma.h"


/****************************************************************************
  Name          : fmInitialize

  Description   : 

  Arguments     :

  Return Values : 

  Notes         : None.
******************************************************************************/

SaAisErrorT fmInitialize(fmHandleT *fmHandle, const fmCallbacksT *fmCallbacks, SaVersionT *version)
{
   FMA_CB *cb = 0;
   SaAisErrorT  rc = SA_AIS_OK;
   FMA_HDL_DB *hdl_db = 0;
   FMA_HDL_REC *hdl_rec = 0;
   
   m_FMA_LOG_FUNC_ENTRY("fmInitialize");
   if ( !fmHandle || !version )
   {
      m_FMA_LOG_API(FMA_LOG_API_INITIALIZE, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
      return SA_AIS_ERR_BAD_HANDLE;
   }
 
   /** Get the FMA control block **/
   cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
   if ( cb == NULL )
   {
      m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL); 
      m_FMA_LOG_API(FMA_LOG_API_INITIALIZE, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
      rc = SA_AIS_ERR_BAD_HANDLE;
      goto done;
   }

   m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);

   /* Validate the version */
   if ( !m_FMA_VER_IS_VALID(version) )
   {  
      version->releaseCode = 'B';
      version->majorVersion = FMA_MAJOR_VERSION;
      version->minorVersion = FMA_MINOR_VERSION;
      m_FMA_LOG_API(FMA_LOG_API_INITIALIZE, FMA_LOG_API_ERR_SA_VERSION, 0, NCSFL_SEV_CRITICAL);
      rc = SA_AIS_ERR_VERSION; 
      goto done;
   }
   hdl_db = &cb->hdl_db;

   /** Add the rec to the hdl database **/
   hdl_rec = fma_hdl_rec_add(cb, hdl_db, fmCallbacks);
   if ( !hdl_rec )
   {
       m_FMA_LOG_API(FMA_LOG_API_INITIALIZE, FMA_LOG_API_ERR_SA_NO_MEMORY, 0, NCSFL_SEV_CRITICAL);
       rc = SA_AIS_ERR_NO_MEMORY;
       goto done;
   }
   
   /** Pass the handle to the caller **/
   if ( rc == SA_AIS_OK )
   {
      *fmHandle = hdl_rec->hdl;
      m_FMA_LOG_API(FMA_LOG_API_INITIALIZE, FMA_LOG_API_ERR_SA_OK, 0, NCSFL_SEV_NOTICE);
   }

done:
    /** free the hdl rec if there's some error **/
   if (hdl_rec && SA_AIS_OK != rc)
       fma_hdl_rec_del (cb, hdl_db, hdl_rec);
   
      /** Release the write lock and give the fma cb handle **/       
   if ( cb )
   {
      m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);    
      ncshm_give_hdl(gl_fma_hdl);  
   }
   
   return rc;
      
}
                                            
/****************************************************************************
  Name          : fmSelectionObjectGet

  Description   :

  Arguments     :

  Return Values :

  Notes         : None.
******************************************************************************/
SaAisErrorT fmSelectionObjectGet( fmHandleT fmHandle, SaSelectionObjectT *selectionObject)
{
   FMA_CB      *cb = 0;
   FMA_HDL_REC *hdl_rec = 0;
   SaAisErrorT rc = SA_AIS_OK;

   m_FMA_LOG_FUNC_ENTRY("fmSelectionObjectGet");

   /** Verify fma cb handle and input handle **/
   if ( !gl_fma_hdl || (fmHandle > FMA_UNS32_HDL_MAX))
   {
      m_FMA_LOG_API(FMA_LOG_API_SEL_OBJ_GET, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL); 
      return SA_AIS_ERR_BAD_HANDLE;
   }

   if ( !selectionObject )
   {
      m_FMA_LOG_API(FMA_LOG_API_SEL_OBJ_GET, FMA_LOG_API_ERR_SA_INVALID_PARAM, 0, NCSFL_SEV_CRITICAL);
      return SA_AIS_ERR_INVALID_PARAM;
   }
   
   /** Get the FMA control block **/
   cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
   if ( cb == NULL )
   {
       m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
       m_FMA_LOG_API(FMA_LOG_API_SEL_OBJ_GET, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
       rc =  SA_AIS_ERR_BAD_HANDLE;
       goto done;
   }

   m_NCS_LOCK(&cb->lock, NCS_LOCK_READ);
   
   hdl_rec = (FMA_HDL_REC*)ncshm_take_hdl( NCS_SERVICE_ID_FMA, fmHandle );
   if ( !hdl_rec )
   {
       m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_GET,FMA_LOG_HDL_DB_FAILURE,fmHandle,NCSFL_SEV_CRITICAL);
       m_FMA_LOG_API(FMA_LOG_API_SEL_OBJ_GET, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL); 
       rc = SA_AIS_ERR_BAD_HANDLE;
       goto done;
   }
   /*pass the sel obj to the appl */
   *selectionObject = (SaSelectionObjectT)m_GET_FD_FROM_SEL_OBJ(hdl_rec->sel_obj);
   m_FMA_LOG_API(FMA_LOG_API_SEL_OBJ_GET, FMA_LOG_API_ERR_SA_OK, 0, NCSFL_SEV_NOTICE);

done:
   /** Release READ lock and fma cb handle **/
   if ( cb )
   {
      m_NCS_UNLOCK(&cb->lock, NCS_LOCK_READ);
      ncshm_give_hdl(gl_fma_hdl);
   }
   /** Release hdl rec handle **/
   if ( hdl_rec )
      ncshm_give_hdl( fmHandle );
   
   return rc;

}

/****************************************************************************
  Name          : fmDispatch

  Description   :

  Arguments     :

  Return Values :

  Notes         : None.
******************************************************************************/
SaAisErrorT fmDispatch( fmHandleT fmHandle, SaDispatchFlagsT dispatchFlags )
{
   FMA_CB      *cb = 0;
   FMA_HDL_REC *hdl_rec = 0;
   SaAisErrorT rc = SA_AIS_OK;
   
   m_FMA_LOG_FUNC_ENTRY("fmDispatch");
   /** Verify fma cb handle and input handle **/
   if ( !gl_fma_hdl || (fmHandle > FMA_UNS32_HDL_MAX))
   {
      m_FMA_LOG_API(FMA_LOG_API_DISPATCH, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
      return SA_AIS_ERR_BAD_HANDLE;
   }
   if ((dispatchFlags != SA_DISPATCH_ONE) && (dispatchFlags != SA_DISPATCH_ALL) && (dispatchFlags != SA_DISPATCH_BLOCKING))
   {
       m_FMA_LOG_API(FMA_LOG_API_DISPATCH, FMA_LOG_API_ERR_SA_INVALID_PARAM, 0, NCSFL_SEV_CRITICAL);
      return SA_AIS_ERR_INVALID_PARAM;
   }
   
   /** Extract the FMA CB **/
   cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
   if ( cb == NULL )
   {
       m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
       m_FMA_LOG_API(FMA_LOG_API_DISPATCH, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
       rc = SA_AIS_ERR_BAD_HANDLE;
       goto done;
   }
   
   m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);
   
   /** Extract the REC Hdl **/
   hdl_rec = (FMA_HDL_REC*)ncshm_take_hdl( NCS_SERVICE_ID_FMA, fmHandle );
   if ( !hdl_rec )
   {
       m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_GET,FMA_LOG_HDL_DB_FAILURE,fmHandle,NCSFL_SEV_CRITICAL);
       m_FMA_LOG_API(FMA_LOG_API_DISPATCH, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
       rc = SA_AIS_ERR_BAD_HANDLE;
       goto done;
   }
   cb->pend_dispatch++;    
   rc = fma_hdl_callbk_dispatch(&cb, &hdl_rec, dispatchFlags);
   cb->pend_dispatch--;

done:
   if (cb)
   {
      m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);
      ncshm_give_hdl(gl_fma_hdl);
   }
   if ( hdl_rec )  
      ncshm_give_hdl(fmHandle);
   if ( rc == SA_AIS_OK )
       m_FMA_LOG_API(FMA_LOG_API_DISPATCH, FMA_LOG_API_ERR_SA_OK, 0, NCSFL_SEV_NOTICE);
   return rc;
}
/****************************************************************************
  Name          : fmResponse

  Description   :

  Arguments     :

  Return Values :

  Notes         : None.
******************************************************************************/
SaAisErrorT fmResponse(fmHandleT fmHandle, SaInvocationT invocation, SaAisErrorT error)
{
   FMA_CB      *cb = 0;
   FMA_HDL_REC *hdl_rec = 0;
   SaAisErrorT rc = SA_AIS_OK;
   FMA_PEND_RESP *resp_list = 0;
   FMA_PEND_RESP_REC *resp_rec = 0;

   m_FMA_LOG_FUNC_ENTRY("fmResponse");
   if ( !gl_fma_hdl || (fmHandle > FMA_UNS32_HDL_MAX))
   {
      m_FMA_LOG_API(FMA_LOG_API_RESP, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
      return SA_AIS_ERR_BAD_HANDLE;
   }
   
   /** Extract the FMA CB **/
   cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
   if ( cb == NULL )
   {
       m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
       m_FMA_LOG_API(FMA_LOG_API_RESP, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
       rc = SA_AIS_ERR_BAD_HANDLE;
       goto done;
   }
   
  /** Extract the REC Hdl **/
   hdl_rec = (FMA_HDL_REC*)ncshm_take_hdl( NCS_SERVICE_ID_FMA, fmHandle );
   if ( !hdl_rec )
   {   
       m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_GET,FMA_LOG_HDL_DB_FAILURE,fmHandle,NCSFL_SEV_CRITICAL);
       m_FMA_LOG_API(FMA_LOG_API_RESP, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
       rc = SA_AIS_ERR_BAD_HANDLE;
       goto done;
   }
   
   resp_list = &hdl_rec->pend_resp;
   if(!resp_list)
   {   
       m_FMA_LOG_API(FMA_LOG_API_RESP, FMA_LOG_API_ERR_SA_EMPTY_LIST, 0, NCSFL_SEV_CRITICAL);
      rc = SA_AIS_ERR_LIBRARY;
      goto done;
   }

   resp_rec = fma_hdl_pend_resp_get(resp_list,invocation);      
   
   if ( !resp_rec )
   {
      m_FMA_LOG_API(FMA_LOG_API_RESP, FMA_LOG_API_ERR_SA_INVALID_PARAM, 0, NCSFL_SEV_INFO);
      rc = SA_AIS_ERR_INVALID_PARAM;
      goto done;
   }
  
   /* Free the invocation obj as it is the handle taken from hdl-mnger . Hence destory the handle */
   if ( resp_rec->cbk_info->inv )
   {
      ncshm_destroy_hdl(NCS_SERVICE_ID_FMA,resp_rec->cbk_info->inv);
   }
  
   /* Remove the resp rec from the resp list */
   resp_rec = fma_hdl_pend_resp_pop (resp_list,invocation); 

   /* Free the resp rec */
   m_MMGR_FREE_FMA_CBK_INFO(resp_rec->cbk_info); 
   m_MMGR_FREE_FMA_PEND_CBK_REC(resp_rec);
   resp_rec = NULL;

done:
   if ( cb )
      ncshm_give_hdl(gl_fma_hdl);   
   if ( hdl_rec )
      ncshm_give_hdl(fmHandle);    

   if ( rc == SA_AIS_OK )
       m_FMA_LOG_API(FMA_LOG_API_RESP, FMA_LOG_API_ERR_SA_OK, 0, NCSFL_SEV_NOTICE);
   return rc;
}


/****************************************************************************
  Name          : fmFinalize

  Description   : 

  Arguments     : 

  Return Values : 

  Notes         : None.
******************************************************************************/
SaAisErrorT fmFinalize( fmHandleT fmHandle )
{
   FMA_HDL_DB *hdl_db = 0;
   FMA_HDL_REC *hdl_rec = 0;
   FMA_CB *cb = 0;   
   uns32 hdl = (uns32)fmHandle;
   SaAisErrorT  rc = SA_AIS_OK;
   NCS_BOOL ncs_agent = FALSE;   

   m_FMA_LOG_FUNC_ENTRY("fmFinalize");
   /** Verify fma cb handle and input handle **/
   if ( !gl_fma_hdl || (fmHandle > FMA_UNS32_HDL_MAX))
   {
      m_FMA_LOG_API(FMA_LOG_API_FINALIZE, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_INFO);
      return SA_AIS_ERR_BAD_HANDLE;
   }
  
    /** Extract the FMA CB **/
   cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
   if ( cb == NULL )
   {
       m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
       m_FMA_LOG_API(FMA_LOG_API_FINALIZE, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_INFO);
       rc = SA_AIS_ERR_BAD_HANDLE;
       goto done;
   }
   
   m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);

   hdl_db = &cb->hdl_db;

   hdl_rec = (FMA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_FMA, hdl);
  
   if ( !hdl_rec )
   {
       m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_GET,FMA_LOG_HDL_DB_FAILURE,fmHandle,NCSFL_SEV_CRITICAL);     
       m_FMA_LOG_API(FMA_LOG_API_FINALIZE, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_INFO); 
       rc = SA_AIS_ERR_BAD_HANDLE;
       goto done;
   }
   
   /** delete the hdl rec **/
   if (SA_AIS_OK == rc) 
      fma_hdl_rec_del(cb, hdl_db, hdl_rec);

   if ( SA_AIS_OK == rc && cb->pend_dispatch == 0)
       ncs_agent = TRUE;

done:
   
   if (cb)
   {
     m_NCS_UNLOCK(&(cb)->lock, NCS_LOCK_WRITE);
     ncshm_give_hdl(gl_fma_hdl);
   }

   if ( rc == SA_AIS_OK )
       m_FMA_LOG_API(FMA_LOG_API_FINALIZE, FMA_LOG_API_ERR_SA_OK, 0, NCSFL_SEV_NOTICE);

   return rc;
}



/****************************************************************************
  Name          : fmNodeResetInd

  Description   :

  Arguments     :

  Return Values :

  Notes         : None.
******************************************************************************/

SaAisErrorT fmNodeResetInd(fmHandleT fmHandle, SaHpiEntityPathT entityReset)
{
    SaAisErrorT rc = SA_AIS_OK;
    FMA_CB *fma_cb = 0;
    FMA_HDL_REC *hdl_rec = 0;  
    FMA_FM_MSG *fma_fm_msg = NULL;
    
    m_FMA_LOG_FUNC_ENTRY("fmNodeResetInd");
    
    if ( !gl_fma_hdl || (fmHandle > FMA_UNS32_HDL_MAX))
    {
        m_FMA_LOG_API(FMA_LOG_API_NODE_RESET_IND, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
        return SA_AIS_ERR_BAD_HANDLE;
    }
    
    /** Extract the FMA CB **/
    fma_cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
    if ( fma_cb == NULL )
    {
        m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
        m_FMA_LOG_API(FMA_LOG_API_NODE_RESET_IND, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
        return  SA_AIS_ERR_BAD_HANDLE;
    }
        
    /** Extract the REC Hdl : This verifies whether the caller has registered with FMA or not **/
    hdl_rec = (FMA_HDL_REC*)ncshm_take_hdl( NCS_SERVICE_ID_FMA, fmHandle );
    if ( !hdl_rec )
    {
        ncshm_give_hdl(gl_fma_hdl);
        m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_GET,FMA_LOG_HDL_DB_FAILURE,fmHandle,NCSFL_SEV_CRITICAL);
        m_FMA_LOG_API(FMA_LOG_API_NODE_RESET_IND, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
        return  SA_AIS_ERR_BAD_HANDLE;
    }
   
    fma_fm_msg = m_MMGR_ALLOC_FMA_FM_MSG;
    if (fma_fm_msg == FMA_FM_MSG_NULL)
    {
        m_FMA_LOG_MEM(FMA_LOG_FMA_FM_MSG_ALLOC, FMA_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
        m_FMA_LOG_API(FMA_LOG_API_NODE_RESET_IND, FMA_LOG_API_ERR_SA_NO_MEMORY, 0, NCSFL_SEV_INFO);
        ncshm_give_hdl(fmHandle);
        ncshm_give_hdl(gl_fma_hdl);
        return SA_AIS_ERR_NO_MEMORY;
    }
   
    /* formulate the fma_fm_msg for the received message */
    fma_fm_msg->msg_type = FMA_FM_EVT_NODE_RESET_IND;
    fma_get_slot_site_from_ent_path(entityReset, &(fma_fm_msg->info.phy_addr.slot), &(fma_fm_msg->info.phy_addr.site));
    
    if(fma_fm_mds_msg_bcast(fma_cb, fma_fm_msg) != NCSCC_RC_SUCCESS)
    {
        m_FMA_LOG_API(FMA_LOG_API_RESP, FMA_LOG_API_ERR_FAILED_OPERATION, 0, NCSFL_SEV_CRITICAL);
        m_MMGR_FREE_FMA_FM_MSG(fma_fm_msg);
        fma_fm_msg = NULL;
        ncshm_give_hdl(fmHandle);
        ncshm_give_hdl(gl_fma_hdl);
        return SA_AIS_ERR_FAILED_OPERATION;
    }
    
    if (hdl_rec)
        ncshm_give_hdl(fmHandle);  
    if ( fma_cb )
        ncshm_give_hdl(gl_fma_hdl); 
    if(fma_fm_msg)
        m_MMGR_FREE_FMA_FM_MSG(fma_fm_msg);

    if(rc == SA_AIS_OK)
        m_FMA_LOG_API(FMA_LOG_API_NODE_RESET_IND, FMA_LOG_API_ERR_SA_OK, 0, NCSFL_SEV_NOTICE);
    
    return rc;
}

/****************************************************************************
  Name          : fmCanSwitchoverProceed

  Description   :

  Arguments     :

  Return Values :

  Notes         : None.
******************************************************************************/

SaAisErrorT fmCanSwitchoverProceed(fmHandleT fmHandle, SaBoolT *CanSwitchoverProceed)
{
    SaAisErrorT rc = SA_AIS_OK;
    FMA_CB *fma_cb = 0;
    FMA_HDL_REC *hdl_rec = 0;
    uns32 timeout = 1000; /* 10 secs */
    FMA_FM_MSG *i_msg, *o_msg;
  
    m_FMA_LOG_FUNC_ENTRY("fmCanSwitchoverProceed");
    
    *CanSwitchoverProceed = SA_FALSE ; 

    if ( !gl_fma_hdl || (fmHandle > FMA_UNS32_HDL_MAX))
    {
        m_FMA_LOG_API(FMA_LOG_API_CAN_SWITCHOVER, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
        return SA_AIS_ERR_BAD_HANDLE;
    }
    
    /** Extract the FMA CB **/
    fma_cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
    if ( fma_cb == NULL )
    {
        m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
        m_FMA_LOG_API(FMA_LOG_API_CAN_SWITCHOVER, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
        return  SA_AIS_ERR_BAD_HANDLE;
    }
    
    /** Extract the REC Hdl : This verifies whether the caller has registered with FMA or not **/
    hdl_rec = (FMA_HDL_REC*)ncshm_take_hdl( NCS_SERVICE_ID_FMA, fmHandle );
    if ( !hdl_rec )
    {
        ncshm_give_hdl(gl_fma_hdl);
        m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_GET,FMA_LOG_HDL_DB_FAILURE,fmHandle,NCSFL_SEV_CRITICAL);
        m_FMA_LOG_API(FMA_LOG_API_CAN_SWITCHOVER, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
        return  SA_AIS_ERR_BAD_HANDLE;
        
    }
    
    i_msg = m_MMGR_ALLOC_FMA_FM_MSG;
    if (i_msg == FMA_FM_MSG_NULL)
    {
        m_FMA_LOG_MEM(FMA_LOG_FMA_FM_MSG_ALLOC, FMA_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
        m_FMA_LOG_API(FMA_LOG_API_CAN_SWITCHOVER, FMA_LOG_API_ERR_SA_NO_MEMORY, 0, NCSFL_SEV_INFO);
        ncshm_give_hdl(fmHandle);
        ncshm_give_hdl(gl_fma_hdl);
        return SA_AIS_ERR_NO_MEMORY;
    }
    
    o_msg = m_MMGR_ALLOC_FMA_FM_MSG;
    if (o_msg == FMA_FM_MSG_NULL)
    {
        m_FMA_LOG_MEM(FMA_LOG_FMA_FM_MSG_ALLOC, FMA_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
        m_FMA_LOG_API(FMA_LOG_API_CAN_SWITCHOVER, FMA_LOG_API_ERR_SA_NO_MEMORY, 0, NCSFL_SEV_INFO);
        ncshm_give_hdl(fmHandle);
        ncshm_give_hdl(gl_fma_hdl);
        return SA_AIS_ERR_NO_MEMORY;
    }
   
    /* formulate the fma_fm_msg for the received message */
    i_msg->msg_type = FMA_FM_EVT_CAN_SMH_SW;
    
    if(fma_fm_mds_msg_sync_send(fma_cb, i_msg, &o_msg, timeout) != NCSCC_RC_SUCCESS)
    {
        m_FMA_LOG_MDS(FMA_LOG_MDS_SENDRSP, FMA_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
        m_FMA_LOG_API(FMA_LOG_API_CAN_SWITCHOVER, FMA_LOG_API_ERR_FAILED_OPERATION, 0, NCSFL_SEV_CRITICAL);
        m_MMGR_FREE_FMA_FM_MSG(i_msg);
        i_msg = NULL;
        m_MMGR_FREE_FMA_FM_MSG(o_msg);
        o_msg = NULL;
        ncshm_give_hdl(fmHandle);
        ncshm_give_hdl(gl_fma_hdl);
        return SA_AIS_ERR_FAILED_OPERATION;
    }
    
    if(o_msg->msg_type != FMA_FM_EVT_SMH_SW_RESP)
    {
        m_FMA_LOG_API(FMA_LOG_API_CAN_SWITCHOVER, FMA_LOG_API_ERR_FAILED_OPERATION, 0, NCSFL_SEV_CRITICAL);
        return SA_AIS_ERR_FAILED_OPERATION;
    }
    
    *CanSwitchoverProceed = (o_msg->info.response == FMA_FM_SMH_CAN_SW)?SA_TRUE:SA_FALSE;
    
    if (hdl_rec)
        ncshm_give_hdl(fmHandle);  
    if (fma_cb)
        ncshm_give_hdl(gl_fma_hdl); 
    if(i_msg)
        m_MMGR_FREE_FMA_FM_MSG(i_msg);
    if(o_msg)
        m_MMGR_FREE_FMA_FM_MSG(o_msg);

    if(rc == SA_AIS_OK)
        m_FMA_LOG_API(FMA_LOG_API_CAN_SWITCHOVER, FMA_LOG_API_ERR_SA_OK, 0, NCSFL_SEV_NOTICE);
    
    return rc;
}

/****************************************************************************
  Name          : fmNodeHeartbeatInd

  Description   :

  Arguments     :

  Return Values :

  Notes         : None.
******************************************************************************/

SaAisErrorT fmNodeHeartbeatInd(fmHandleT fmHandle, fmHeartbeatIndType fmHbType, SaHpiEntityPathT ent_path)
{
    SaAisErrorT rc = SA_AIS_OK;
    FMA_CB *fma_cb = 0;
    FMA_HDL_REC *hdl_rec = 0;  
    FMA_FM_MSG *fma_fm_msg = NULL;

    m_FMA_LOG_FUNC_ENTRY("fmNodeHeartbeatInd");

    if ( !gl_fma_hdl || (fmHandle > FMA_UNS32_HDL_MAX))
    {
       m_FMA_LOG_API(FMA_LOG_API_NODE_HB_IND, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
       return SA_AIS_ERR_BAD_HANDLE;
    }

    /** Extract the FMA CB **/
    fma_cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
    if ( fma_cb == NULL )
    {
        m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
        m_FMA_LOG_API(FMA_LOG_API_NODE_HB_IND, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
        return  SA_AIS_ERR_BAD_HANDLE;
    }
      
    /** Extract the REC Hdl : This verifies whether the caller has registered with FMA or not **/
    hdl_rec = (FMA_HDL_REC*)ncshm_take_hdl( NCS_SERVICE_ID_FMA, fmHandle );
    if ( !hdl_rec )
    {
        ncshm_give_hdl(gl_fma_hdl);
        m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_GET,FMA_LOG_HDL_DB_FAILURE,fmHandle,NCSFL_SEV_CRITICAL);
        m_FMA_LOG_API(FMA_LOG_API_NODE_HB_IND, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_CRITICAL);
        return  SA_AIS_ERR_BAD_HANDLE;
    }

    fma_fm_msg = m_MMGR_ALLOC_FMA_FM_MSG;
    if (fma_fm_msg == FMA_FM_MSG_NULL)
    {
        m_FMA_LOG_MEM(FMA_LOG_FMA_FM_MSG_ALLOC, FMA_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
        m_FMA_LOG_API(FMA_LOG_API_NODE_HB_IND, FMA_LOG_API_ERR_SA_NO_MEMORY, 0, NCSFL_SEV_INFO);
        ncshm_give_hdl(fmHandle);
        ncshm_give_hdl(gl_fma_hdl);
        return SA_AIS_ERR_NO_MEMORY;
    }
    
    /* formulate the fma_fm_msg for the received message */
    if(fmHbType == fmHeartbeatLost)
        fma_fm_msg->msg_type = FMA_FM_EVT_HB_LOSS;
    else
        fma_fm_msg->msg_type = FMA_FM_EVT_HB_RESTORE;
    
    fma_get_slot_site_from_ent_path(ent_path, &(fma_fm_msg->info.phy_addr.slot), &(fma_fm_msg->info.phy_addr.site));

    if(fma_fm_mds_msg_async_send(fma_cb, fma_fm_msg) != NCSCC_RC_SUCCESS)
    {
        m_FMA_LOG_MDS(FMA_LOG_MDS_SEND, FMA_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
        m_FMA_LOG_API(FMA_LOG_API_NODE_HB_IND, FMA_LOG_API_ERR_FAILED_OPERATION, 0, NCSFL_SEV_CRITICAL);
        m_MMGR_FREE_FMA_FM_MSG(fma_fm_msg);
        fma_fm_msg = NULL;
        ncshm_give_hdl(fmHandle);
        ncshm_give_hdl(gl_fma_hdl);
        return SA_AIS_ERR_FAILED_OPERATION;
    }
    
    if (hdl_rec)
        ncshm_give_hdl(fmHandle);  
    if ( fma_cb )
        ncshm_give_hdl(gl_fma_hdl); 
    if(fma_fm_msg)
        m_MMGR_FREE_FMA_FM_MSG(fma_fm_msg);
    if(rc == SA_AIS_OK)
        m_FMA_LOG_API(FMA_LOG_API_NODE_HB_IND, FMA_LOG_API_ERR_SA_OK, 0, NCSFL_SEV_NOTICE);
    
    return rc;
}

