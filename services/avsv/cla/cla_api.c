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

/*****************************************************************************
..............................................................................



..............................................................................

  DESCRIPTION:

  This file contains the CLM API implementation.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "cla.h"
#include "ncs_main_papi.h"

/* Macro for Verifying the input Handle & global handle */
#define m_CLA_API_HDL_VERIFY(cbhdl, hdl, o_rc) \
{ \
   /* is library Initialized && handle a 32 bit value*/ \
   if(!(cbhdl) || (hdl) > AVSV_UNS32_HDL_MAX) \
      (o_rc) = SA_AIS_ERR_BAD_HANDLE;\
};


/****************************************************************************
  Name          : saClmInitialize
 
  Description   : This function initializes the CLM for the invoking process
                  and registers the various callback functions.
 
  Arguments     : o_hdl    - ptr to the CLM handle
                  reg_cbks - ptr to a SaClmCallbacksT structure
                  ver      - Version of the CLM implementation being used 
                             by the invoking process.
 
  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saClmInitialize (SaClmHandleT *o_hdl, 
                          const SaClmCallbacksT *reg_cbks,
                          SaVersionT *ver)
{
   CLA_CB       *cb = 0;
   CLA_HDL_DB   *hdl_db = 0;
   CLA_HDL_REC  *hdl_rec = 0;
   SaAisErrorT     rc = SA_AIS_OK;
   int argc = 0;
   char **argv;

   if( (!o_hdl) || (!ver) )
   {
      return SA_AIS_ERR_INVALID_PARAM;
   }

   /* validate the version */
   if (!m_CLA_VER_IS_VALID(ver))
   {
      /* fill the supported version no */
      ver->releaseCode = 'B';
      ver->majorVersion = 1;
      ver->minorVersion = 1;
      return SA_AIS_ERR_VERSION;
   }

   ver->releaseCode = 'B';
   ver->majorVersion = 1;
   ver->minorVersion = 1;

   /* Initialize the environment */
   if (ncs_agents_startup(argc, argv) != NCSCC_RC_SUCCESS)
   {
      return SA_AIS_ERR_LIBRARY;
   }

   /* Create AVA/CLA  CB */
   if (ncs_cla_startup() != NCSCC_RC_SUCCESS)
   {
      ncs_agents_shutdown(argc, argv);
      return SA_AIS_ERR_LIBRARY;
   }

   /* retrieve CLA CB */
   if ( !(cb = (CLA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, gl_cla_hdl)) )
   {
      return SA_AIS_ERR_LIBRARY; 
   }

   /* acquire cb write lock */
   m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);

   if(m_NCS_NODE_ID_FROM_MDS_DEST(cb->avnd_intf.avnd_mds_dest) == 0)
   {
      rc = SA_AIS_ERR_TRY_AGAIN;
      goto done;
   }

   /* get the ptr to the hdl db */
   hdl_db = &cb->hdl_db;

   /* create the hdl record & store the callbacks */
   if ( !(hdl_rec = cla_hdl_rec_add(cb, hdl_db, reg_cbks)) )
      rc = SA_AIS_ERR_NO_MEMORY;

   /* pass the handle value to the appl */
   if (SA_AIS_OK == rc)
      *o_hdl = hdl_rec->hdl;

done:
   /* free the hdl rec if there's some error */
   if (hdl_rec && SA_AIS_OK != rc)
      cla_hdl_rec_del (cb, hdl_db, hdl_rec);

   /* release cb write lock */
   if(cb) m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);

   /* return CLA CB */
   if(cb) ncshm_give_hdl(gl_cla_hdl);

   /* log the status of api processing */
   m_CLA_LOG_API(CLA_LOG_API_INITIALIZE, rc+CLA_LOG_API_DISPATCH, 
                 NCSFL_SEV_INFO);

   if (SA_AIS_OK != rc)
   {
      ncs_cla_shutdown();
      ncs_agents_shutdown(argc, argv);
   }

   return rc;
}


/****************************************************************************
  Name          : saClmSelectionObjectGet
 
  Description   : This function creates & returns the operating system handle 
                  associated with the CLM Handle.
 
  Arguments     : hdl       - CLM handle      - CLM handle
                  o_sel_obj - ptr to the selection object
 
  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saClmSelectionObjectGet (SaClmHandleT hdl, 
                                  SaSelectionObjectT *o_sel_obj)
{
   CLA_CB      *cb = 0;
   CLA_HDL_REC *hdl_rec = 0;
   SaAisErrorT    rc = SA_AIS_OK;

   /* verify CB-hdl & input hdl  */
   m_CLA_API_HDL_VERIFY(gl_cla_hdl, hdl, rc);
   if(SA_AIS_OK != rc) return rc;

   if ( !o_sel_obj )
      return SA_AIS_ERR_INVALID_PARAM;

   /* retrieve CLA CB */
   if ( !(cb = (CLA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, gl_cla_hdl)) )
   {
      return SA_AIS_ERR_LIBRARY; /* check */
   }

   /* retrieve hdl rec */
   if ( !(hdl_rec = (CLA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, hdl)) )
   {
      rc = SA_AIS_ERR_BAD_HANDLE; /* check */
      goto done;
   }

   /* everything's fine.. pass the sel obj to the appl */
   *o_sel_obj = (SaSelectionObjectT)m_GET_FD_FROM_SEL_OBJ(hdl_rec->sel_obj);

done:
   /* return CLA CB & hdl rec */
   if (cb) ncshm_give_hdl(gl_cla_hdl);
   if (hdl_rec) ncshm_give_hdl(hdl);

   /* log the status of api processing */
   m_CLA_LOG_API(CLA_LOG_API_SEL_OBJ_GET, rc+CLA_LOG_API_DISPATCH, 
                 NCSFL_SEV_INFO);

   return rc;
}


/****************************************************************************
  Name          : saClmDispatch
 
  Description   : This function invokes, in the context of the calling thread,
                  the next pending callback for the CLM handle.
 
  Arguments     : hdl   - CLM handle
                  flags - flags that specify the callback execution behavior
                  of the saClmDispatch() function,
 
  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saClmDispatch (SaClmHandleT hdl,
                        SaDispatchFlagsT flags)
{
   CLA_CB      *cb = 0;
   CLA_HDL_REC *hdl_rec = 0;
   SaAisErrorT    rc = SA_AIS_OK;
   int argc = 0;
   char **argv;
   uns32 pend_fin = 0;
   uns32 pend_dis = 0;

   if (!m_CLA_DISPATCH_FLAG_IS_VALID(flags))
      return SA_AIS_ERR_INVALID_PARAM;

   /* verify CB-hdl & input hdl  */
   m_CLA_API_HDL_VERIFY(gl_cla_hdl, hdl, rc);
   if(SA_AIS_OK != rc) return rc;

   /* retrieve CLA CB */
   if ( !(cb = (CLA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, gl_cla_hdl)) )
   {
      return SA_AIS_ERR_LIBRARY; /* check */
   }

   /* retrieve hdl rec */
   if ( !(hdl_rec = (CLA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, hdl)) )
   {
      rc = SA_AIS_ERR_BAD_HANDLE; /* check */
      goto done;
   }

   /* acquire cb write lock */
   m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);

   /* Increment Dispatch usgae count */
   cb->pend_dis++;

   rc = cla_hdl_cbk_dispatch(&cb, &hdl_rec, flags);

   /* Decrement Dispatch usage count */
   cb->pend_dis--;

   /* copy counts into local var */
   pend_fin = cb->pend_fin;
   pend_dis = cb->pend_dis; 

   /* release cb write lock */
   m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);

done:
   /* return CLA CB & hdl rec */
   if (cb) ncshm_give_hdl(gl_cla_hdl);
   if (hdl_rec) ncshm_give_hdl(hdl);

   /* log the status of api processing */
   m_CLA_LOG_API(CLA_LOG_API_DISPATCH, rc+CLA_LOG_API_DISPATCH, 
                 NCSFL_SEV_INFO);

   if(pend_dis == 0)
      while(pend_fin != 0)
      {
         ncs_cla_shutdown();
         ncs_agents_shutdown(argc, argv);
         pend_fin--;
      }

   return rc;
}


/****************************************************************************
  Name          : saClmFinalize
 
  Description   : This function closes the association, represented by the 
                  CLM handle, between the invoking process and the CLM.
 
  Arguments     : hdl - CLM handle
 
  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saClmFinalize (SaClmHandleT hdl)
{
   CLA_CB       *cb = 0;
   CLA_HDL_DB   *hdl_db = 0;
   CLA_HDL_REC  *hdl_rec = 0;
   AVSV_NDA_CLA_MSG *msg = 0;
   SaAisErrorT     rc = SA_AIS_OK;
   int argc = 0;
   char **argv;
   NCS_BOOL agent_flag = FALSE;

   /* verify CB-hdl & input hdl  */
   m_CLA_API_HDL_VERIFY(gl_cla_hdl, hdl, rc);
   if(SA_AIS_OK != rc) return rc;

   /* retrieve CLA CB */
   if ( !(cb = (CLA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, gl_cla_hdl)) )
   {
      return SA_AIS_ERR_LIBRARY; 
   }

   /* acquire cb write lock */
   m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);

   /* get the ptr to the hdl db */
   hdl_db = &cb->hdl_db;

   /* get the hdl rec */
   m_CLA_HDL_REC_FIND(hdl_db, hdl, hdl_rec);
   if (!hdl_rec)
   {
      rc = SA_AIS_ERR_BAD_HANDLE;
      goto done;
   }

   /* populate & send the finalize message */
   if ( 0 == (msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG) )
   {
      rc = SA_AIS_ERR_NO_MEMORY;
      goto done;
   }
   m_CLA_CLM_FINALIZE_MSG_FILL(*msg, cb->prc_id, hdl);

   /* Send a async message */
   rc = cla_mds_send(cb, msg, 0, 0);

   if (NCSCC_RC_FAILURE == rc)
      rc = SA_AIS_ERR_TRY_AGAIN; /* check */

   /* delete the hdl rec */
   if (SA_AIS_OK == rc)
      cla_hdl_rec_del(cb, hdl_db, hdl_rec);

   /* can we Fialize the environment */  
   if ( SA_AIS_OK == rc && cb->pend_dis == 0 )
      agent_flag = TRUE;
   else if( SA_AIS_OK == rc && cb->pend_dis > 0 )
      cb->pend_fin++;

done:
   /* release cb write lock */
   if(cb) m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);

   /* return CLA CB */
   if (cb) ncshm_give_hdl(gl_cla_hdl);

   /* free the request message */
   if (msg) avsv_nda_cla_msg_free(msg);

   /* log the status of api processing */
   m_CLA_LOG_API(CLA_LOG_API_FINALIZE, rc+CLA_LOG_API_DISPATCH, 
                 NCSFL_SEV_INFO);

   /* Fialize the environment */  
   if(agent_flag == TRUE)
   {
      ncs_cla_shutdown();
      ncs_agents_shutdown(argc, argv);
   }

   return rc;
}


/****************************************************************************
  Name          : saClmClusterTrack
 
  Description   : This fuction requests CLM to start tracking the changes
                  in the cluster membership.
 
  Arguments     : hdl   - CLM handle
                  flags - flags that determines when the CLM track callback is 
                          invoked
                  buf   - ptr to the linear buffer provided by the appl
                  num   - buffer size

  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saClmClusterTrack(SaClmHandleT              hdl,
                              SaUint8T                  flags,
                              SaClmClusterNotificationBufferT *buf)
{
   CLA_CB       *cb = 0;
   CLA_HDL_DB   *hdl_db = 0;
   CLA_HDL_REC  *hdl_rec = 0;
   AVSV_NDA_CLA_MSG *msg = 0;
   AVSV_NDA_CLA_MSG *msg_rsp = 0;
   SaAisErrorT     rc = SA_AIS_OK;

   /* verify CB-hdl & input hdl  */
   m_CLA_API_HDL_VERIFY(gl_cla_hdl, hdl, rc);
   if(SA_AIS_OK != rc) return rc;

   /* validate the falgs */
   if(! ((flags & SA_TRACK_CURRENT ) ||
         (flags & SA_TRACK_CHANGES ) ||
         (flags & SA_TRACK_CHANGES_ONLY) ) )
   {
      return SA_AIS_ERR_BAD_FLAGS;
   }

   if((flags & SA_TRACK_CHANGES) && (flags & SA_TRACK_CHANGES_ONLY))
   {
      return SA_AIS_ERR_BAD_FLAGS;
   }

   /* validate the notify buffer */
   if ( (flags & SA_TRACK_CURRENT) && buf && buf->notification )
   {
      if (!buf->numberOfItems)
         return SA_AIS_ERR_INVALID_PARAM;
   }

   /* retrieve CLA CB */
   if ( !(cb = (CLA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, gl_cla_hdl)) )
   {
      return SA_AIS_ERR_LIBRARY; 
   }

   /* acquire cb write lock */
   m_NCS_LOCK(&cb->lock, NCS_LOCK_READ);

   /* get the ptr to the hdl db */
   hdl_db = &cb->hdl_db;

   /* get the hdl rec */
   m_CLA_HDL_REC_FIND(hdl_db, hdl, hdl_rec);
   if (!hdl_rec)
   {
      rc = SA_AIS_ERR_BAD_HANDLE;
      goto done;
   }

   /* Validate if flag is TRACK_CURRENT and no callback and no buffer provided */
   if ( (flags & SA_TRACK_CURRENT) && ((!buf) || !(buf->notification)) &&
        (!(hdl_rec->reg_cbk.saClmClusterTrackCallback)))
   {
      rc = SA_AIS_ERR_INIT;
      goto done; 
   }

   /* populate & send the TRACK message */
   if ( 0 == (msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG) )
   {
      rc = SA_AIS_ERR_NO_MEMORY;
      goto done;
   }

   m_CLA_TRACK_START_MSG_FILL(*msg, cb->prc_id, hdl, flags);

   if(buf)
   {
      if(flags & SA_TRACK_CURRENT)
      {
         /* Do a sync mds send and get information about all
          * nodes that are currently members in the cluster
          * Ref: sec 3.5.1
          */

         msg->info.api_info.is_sync_api = TRUE;
         rc = cla_mds_send(cb, msg, &msg_rsp, 0);

         if(rc == NCSCC_RC_SUCCESS)
         {
            m_AVSV_ASSERT(AVSV_AVND_CLM_API_RESP_MSG == msg_rsp->type);
            m_AVSV_ASSERT(AVSV_CLM_TRACK_START == msg_rsp->info.api_resp_info.type);
            rc = msg_rsp->info.api_resp_info.rc;

            /* Copy the cluster node info into buf 
             * 
             * Perform the sanity check whether sufficient memory is supplied 
             * by buf pointer before invoking the callback.
             *
             * Check the number of items and num fields.
             */
            if((rc == SA_AIS_OK) && (buf->notification != NULL) &&
               (buf->numberOfItems >= msg_rsp->info.api_resp_info.param.track.num))
            {
                /* Overwrite the numberOfItems and copy it to buffer */
                buf->numberOfItems = msg_rsp->info.api_resp_info.param.track.num;

                memset(buf->notification, 0, 
                          sizeof(SaClmClusterNotificationT) * buf->numberOfItems);

                memcpy(buf->notification, msg_rsp->info.api_resp_info.param.track.notify, 
                          sizeof(SaClmClusterNotificationT) * buf->numberOfItems);


                /* since this is SYNC call the appl can read the buffer.
                 * There is no need to invoke the Callback 
                 *
                 * We are done.
                 */
            }
            else if((rc == SA_AIS_OK) && (buf->notification == NULL) )
            {
               /* we need to ignore the numberOfItems and allocate the space
               ** This memory will be freed by the application
               */
                buf->numberOfItems = msg_rsp->info.api_resp_info.param.track.num;

                buf->notification = (SaClmClusterNotificationT *)malloc(sizeof(SaClmClusterNotificationT) * buf->numberOfItems);

                if(buf->notification == NULL )
                   rc = SA_AIS_ERR_NO_RESOURCES;

                memset(buf->notification, 0, 
                          sizeof(SaClmClusterNotificationT) * buf->numberOfItems);

                memcpy(buf->notification, msg_rsp->info.api_resp_info.param.track.notify, 
                          sizeof(SaClmClusterNotificationT) * buf->numberOfItems);


            }
            else if(rc == SA_AIS_OK)
            {
               buf->numberOfItems = msg_rsp->info.api_resp_info.param.track.num;
               rc = SA_AIS_ERR_NO_SPACE;
            }
         }
         else if (NCSCC_RC_FAILURE == rc)
            rc = SA_AIS_ERR_TRY_AGAIN; /* check */
      }
      else
      {
         /* verify if the callback is set during Initialize () 
          * Check only for Track callbecause someone might not be
          * interested in Async callback and might not have that 
          */
         if(!hdl_rec->reg_cbk.saClmClusterTrackCallback)
         {
            rc = SA_AIS_ERR_INIT;
            goto done;
         }

         /* Ignore the buffer and send a normal mds message */
         msg->info.api_info.is_sync_api = FALSE;
         rc = cla_mds_send(cb, msg, 0, 0);
         if(rc == NCSCC_RC_FAILURE) /* MDS send failure */
            rc = SA_AIS_ERR_TRY_AGAIN;
      }
   }
   else /* buf is NULL */
   {
      /* verify if the callback is set during Initialize () 
       * Check only for Track callbecause someone might not be
       * interested in Async callback and might not have that 
       */
      if(!hdl_rec->reg_cbk.saClmClusterTrackCallback)
      {
         rc = SA_AIS_ERR_INIT;
         goto done;
      }

      /* information about nodes that are currently members in the cluster
       * will be notified in CALLBACK send a normal MDS message
       */
      msg->info.api_info.is_sync_api = FALSE;
      rc = cla_mds_send(cb, msg, 0, 0);
      if(rc == NCSCC_RC_FAILURE) /* MDS send failure */
         rc = SA_AIS_ERR_TRY_AGAIN;
   }

done:
   /* return CLA CB */
   if (cb) ncshm_give_hdl(gl_cla_hdl);

   /* release cb write lock */
   if(cb) m_NCS_UNLOCK(&cb->lock, NCS_LOCK_READ);

   /* free the request message */
   if (msg) avsv_nda_cla_msg_free(msg);

   /* free the response message */
   if (msg_rsp) avsv_nda_cla_msg_free(msg_rsp);

   /* log the status of api processing */
   m_CLA_LOG_API(CLA_LOG_API_TRACK_START, rc+CLA_LOG_API_DISPATCH, 
                 NCSFL_SEV_INFO);

   return rc;
}


/****************************************************************************
  Name          : saClmClusterTrackStop
 
  Description   : This fuction requests CLM to stop tracking the changes
                  in the cluster membership.
 
  Arguments     : hdl   - CLM handle

  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saClmClusterTrackStop(SaClmHandleT hdl)
{
   CLA_CB       *cb = 0;
   CLA_HDL_DB   *hdl_db = 0;
   CLA_HDL_REC  *hdl_rec = 0;
   AVSV_NDA_CLA_MSG *msg = 0;
   SaAisErrorT     rc = SA_AIS_OK;
   AVSV_NDA_CLA_MSG *msg_rsp = 0;

   /* verify CB-hdl & input hdl  */
   m_CLA_API_HDL_VERIFY(gl_cla_hdl, hdl, rc);
   if(SA_AIS_OK != rc) return rc;

   /* retrieve CLA CB */
   if ( !(cb = (CLA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, gl_cla_hdl)) )
   {
      return SA_AIS_ERR_LIBRARY; /* check */
   }

   /* acquire cb write lock */
   m_NCS_LOCK(&cb->lock, NCS_LOCK_READ);

   /* get the ptr to the hdl db */
   hdl_db = &cb->hdl_db;

   /* get the hdl rec */
   m_CLA_HDL_REC_FIND(hdl_db, hdl, hdl_rec);
   if (!hdl_rec)
   {
      rc = SA_AIS_ERR_BAD_HANDLE;
      goto done;
   }

   /* populate & send the TRACK_STOP message */
   if ( 0 == (msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG) )
   {
      rc = SA_AIS_ERR_NO_MEMORY;
      goto done;
   }

   m_CLA_TRACK_STOP_MSG_FILL(*msg, cb->prc_id, hdl);

   rc = cla_mds_send(cb, msg, &msg_rsp, 0);

   if(rc == NCSCC_RC_SUCCESS)
   {
      m_AVSV_ASSERT(AVSV_AVND_CLM_API_RESP_MSG == msg_rsp->type);
      m_AVSV_ASSERT(AVSV_CLM_TRACK_STOP == msg_rsp->info.api_resp_info.type);
      rc = msg_rsp->info.api_resp_info.rc;
   }
   else if(rc == NCSCC_RC_FAILURE) /* MDS send failure */
       rc = SA_AIS_ERR_TRY_AGAIN;
 
   /* clear the pending callbacks in the hdl rec */
   if (SA_AIS_OK == rc)
      cla_hdl_cbk_list_del(cb, hdl_rec);

done:
   /* return CLA CB */
   if (cb) ncshm_give_hdl(gl_cla_hdl);

   /* release cb write lock */
   if(cb) m_NCS_UNLOCK(&cb->lock, NCS_LOCK_READ);

   /* free the request message */
   if (msg) avsv_nda_cla_msg_free(msg);

   /* free the response message */
   if (msg_rsp) avsv_nda_cla_msg_free(msg_rsp);

   /* log the status of api processing */
   m_CLA_LOG_API(CLA_LOG_API_TRACK_STOP, rc+CLA_LOG_API_DISPATCH, 
                 NCSFL_SEV_INFO);

   return rc;
}


/****************************************************************************
  Name          : saClmClusterNodeGet
 
  Description   : This fuction synchronously gets the information about a 
                  cluster member (identified by node-id).
 
  Arguments     : hdl          - CLM handle
                  node_id      - node-id for which information is to be 
                                 retrieved
                  timeout      - time-interval within which the information 
                                 should be returned
                  cluster_node - ptr to the node info

  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saClmClusterNodeGet(SaClmHandleT      hdl,
                             SaClmNodeIdT      node_id, 
                             SaTimeT           timeout,
                             SaClmClusterNodeT *cluster_node)
{
   CLA_CB       *cb = 0;
   CLA_HDL_DB   *hdl_db = 0;
   CLA_HDL_REC  *hdl_rec = 0;
   AVSV_NDA_CLA_MSG *msg = 0;
   AVSV_NDA_CLA_MSG *msg_rsp = 0;
   SaAisErrorT     rc = SA_AIS_OK;

   /* Validate the params */
   if(timeout == 0)
      return SA_AIS_ERR_TIMEOUT;

   /* verify CB-hdl & input hdl  */
   m_CLA_API_HDL_VERIFY(gl_cla_hdl, hdl, rc);
   if(SA_AIS_OK != rc) return rc;

   /* retrieve CLA CB */
   if ( !(cb = (CLA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, gl_cla_hdl)) )
   {
      return SA_AIS_ERR_LIBRARY; /* check */
   }

   /* acquire cb write lock */
   m_NCS_LOCK(&cb->lock, NCS_LOCK_READ);

   /* get the ptr to the hdl db */
   hdl_db = &cb->hdl_db;

   /* get the hdl rec */
   m_CLA_HDL_REC_FIND(hdl_db, hdl, hdl_rec);
   if (!hdl_rec)
   {
      rc = SA_AIS_ERR_BAD_HANDLE;
      goto done;
   }

   /* populate & send the GET message */
   if ( 0 == (msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG) )
   {
      rc = SA_AIS_ERR_NO_MEMORY;
      goto done;
   }

   m_CLA_NODE_GET_MSG_FILL(*msg, cb->prc_id, hdl, node_id);

   if(cluster_node)
   {
      /* Do a sync mds send and get information about node indicated 
       * by the node_id
       * Ref: sec 3.5.4
       */

      rc = cla_mds_send(cb, msg, &msg_rsp, timeout);

      if(rc == NCSCC_RC_SUCCESS)
      {
         m_AVSV_ASSERT(AVSV_AVND_CLM_API_RESP_MSG == msg_rsp->type);
         m_AVSV_ASSERT(AVSV_CLM_NODE_GET == msg_rsp->info.api_resp_info.type);
         rc = msg_rsp->info.api_resp_info.rc;

         /*
          * Copy the cluster node info into buffer cluster_node and 
          * since this is SYNC call the appl can read the buffer.
          */
          memset(cluster_node, 0, sizeof(SaClmClusterNodeT));

          if(rc == SA_AIS_OK)
          {
             memcpy(cluster_node, 
                          &msg_rsp->info.api_resp_info.param.node_get, 
                          sizeof(SaClmClusterNodeT));
          }
      }     
      else if(rc == NCSCC_RC_FAILURE) /* MDS send failure */
         rc = SA_AIS_ERR_TRY_AGAIN;
   }
   else /* cluster_node buf is NULL */
   {
      /*
       * The spec doesnt say abt this error code for this API. But no other 
       * error code makes any sense and this code is valid and defined by SA 
       * and is used by TRACK api call.
       */
      rc = SA_AIS_ERR_INVALID_PARAM;
   }

done:
   /* return CLA CB */
   if (cb) ncshm_give_hdl(gl_cla_hdl);

   /* release cb write lock */
   if(cb) m_NCS_UNLOCK(&cb->lock, NCS_LOCK_READ);

   /* free the request message */
   if (msg) avsv_nda_cla_msg_free(msg);

   /* free the response message */
   if (msg_rsp) avsv_nda_cla_msg_free(msg_rsp);

   /* log the status of api processing */
   m_CLA_LOG_API(CLA_LOG_API_NODE_GET, rc+CLA_LOG_API_DISPATCH, 
                 NCSFL_SEV_INFO);

   return rc;
}


/****************************************************************************
  Name          : saClmClusterNodeGetAsync
 
  Description   : This fuction asynchronously gets the information about a 
                  cluster member (identified by node-id).
 
  Arguments     : hdl          - CLM handle
                  inv          - invocation value
                  node_id      - node-id for which information is to be 
                                 retrieved
                  cluster_node - ptr to the node info

  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saClmClusterNodeGetAsync(SaClmHandleT      hdl,
                                  SaInvocationT     inv,
                                  SaClmNodeIdT      node_id)
{
   CLA_CB       *cb = 0;
   CLA_HDL_DB   *hdl_db = 0;
   CLA_HDL_REC  *hdl_rec = 0;
   AVSV_NDA_CLA_MSG *msg = 0;
   SaAisErrorT     rc = SA_AIS_OK;

   /* verify CB-hdl & input hdl  */
   m_CLA_API_HDL_VERIFY(gl_cla_hdl, hdl, rc);
   if(SA_AIS_OK != rc) return rc;

   /* retrieve CLA CB */
   if ( !(cb = (CLA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, gl_cla_hdl)) )
   {
      return SA_AIS_ERR_LIBRARY; /* check */
   }

   /* acquire cb write lock */
   m_NCS_LOCK(&cb->lock, NCS_LOCK_READ);

   /* get the ptr to the hdl db */
   hdl_db = &cb->hdl_db;

   /* get the hdl rec */
   m_CLA_HDL_REC_FIND(hdl_db, hdl, hdl_rec);
   if (!hdl_rec)
   {
      rc = SA_AIS_ERR_BAD_HANDLE;
      goto done;
   }

   /* verify if the callback is set during Initialize () 
    * Check only for Track callbecause someone might not be
    * interested in Async callback and might not have that 
    */
   if(!hdl_rec->reg_cbk.saClmClusterNodeGetCallback)
   {
      rc = SA_AIS_ERR_INIT;
      goto done;
   }


   /* populate & send the GET ASYNC message */
   if ( 0 == (msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG) )
   {
      rc = SA_AIS_ERR_NO_MEMORY;
      goto done;
   }

   m_CLA_NODE_ASYNC_GET_MSG_FILL(*msg, cb->prc_id, hdl, inv, node_id);

   /* information about nodes that are currently members in the cluster
    * will be notified in CALLBACK send a normal MDS message
    */
   rc = cla_mds_send(cb, msg, 0, 0);
   if (NCSCC_RC_FAILURE == rc)
      rc = SA_AIS_ERR_TRY_AGAIN; /* check */

done:
   /* return CLA CB */
   if (cb) ncshm_give_hdl(gl_cla_hdl);

   /* release cb write lock */
   if (cb) m_NCS_UNLOCK(&cb->lock, NCS_LOCK_READ);

   /* free the request message */
   if (msg) avsv_nda_cla_msg_free(msg);

   /* log the status of api processing */
   m_CLA_LOG_API(CLA_LOG_API_NODE_ASYNC_GET, rc+CLA_LOG_API_DISPATCH, 
                 NCSFL_SEV_INFO);

   return rc;
}

