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
        
This include file contains evt processing subroutines
          
******************************************************************************
*/
#include <limits.h>   /* for MAX_INT definition */
#include "eds.h"
#include "eds_ckpt.h"

static uns32  eds_proc_eda_api_msg (EDSV_EDS_EVT *);
static uns32  eds_proc_eda_misc_msg (EDSV_EDS_EVT *);
static uns32  eds_proc_invalid_msg(EDSV_EDS_EVT *);
static uns32  eds_proc_init_msg(EDS_CB *, EDSV_EDS_EVT *);
static uns32  eds_proc_unexpected_msg(EDS_CB *, EDSV_EDS_EVT *);
static uns32  eds_proc_finalize_msg(EDS_CB *, EDSV_EDS_EVT *);
static uns32  eds_proc_chan_open_sync_msg(EDS_CB *, EDSV_EDS_EVT *);
static uns32  eds_proc_chan_open_async_msg(EDS_CB *, EDSV_EDS_EVT *);
static uns32  eds_proc_chan_close_msg(EDS_CB *, EDSV_EDS_EVT *);
static uns32  eds_proc_chan_unlink_msg(EDS_CB *, EDSV_EDS_EVT *);
static uns32  eds_proc_publish_msg(EDS_CB *, EDSV_EDS_EVT *);
static uns32  eds_proc_subscribe_msg(EDS_CB *, EDSV_EDS_EVT *);
static uns32  eds_proc_unsubscribe_msg(EDS_CB *, EDSV_EDS_EVT *);
static uns32  eds_proc_retention_time_clr_msg(EDS_CB *, EDSV_EDS_EVT *);

static uns32  eds_proc_eda_updn_mds_msg (EDSV_EDS_EVT  *evt);
static uns32  eds_process_api_evt (EDSV_EDS_EVT  *evt);
static uns32  eds_proc_ret_tmr_exp_evt (EDSV_EDS_EVT  *evt);
static uns32  eds_proc_mib_request_evt(EDSV_EDS_EVT *evt);
static uns32  eds_proc_quiesced_ack_evt(EDSV_EDS_EVT *evt);


#if (NCS_EDSV_LOG == 1)
static void   eds_publish_log_event(EDS_WORKLIST *wp,
                                    EDSV_EDA_PUBLISH_PARAM  *publish_param,
                                    SaTimeT publish_time);
#endif


/* Top level EDS dispatcher */
static const 
EDSV_EDS_EVT_HANDLER 
eds_edsv_top_level_evt_dispatch_tbl[EDSV_EDS_EVT_MAX - EDSV_EDS_EVT_BASE] =
{
   eds_process_api_evt,
   eds_proc_eda_updn_mds_msg,
   eds_proc_eda_updn_mds_msg,
   eds_proc_ret_tmr_exp_evt,
   eds_proc_mib_request_evt,
   eds_proc_quiesced_ack_evt
};

/* Main EDS dispatch table for EDSV messages */
static const 
EDSV_EDS_EVT_HANDLER eds_edsv_evt_dispatch_tbl[EDSV_MSG_MAX - EDSV_BASE_MSG] =
{
   eds_proc_eda_api_msg,
   eds_proc_eda_misc_msg,
   /* THE EDS should logically not recv any
    * other messages
    */
   eds_proc_invalid_msg, 
   eds_proc_invalid_msg,
   eds_proc_invalid_msg
};

/* Dispatch table for EDA_API realted messages */
static const
EDSV_EDS_EDA_API_MSG_HANDLER eds_eda_api_msg_dispatcher[EDSV_API_MAX - EDSV_API_BASE_MSG] =
{
   eds_proc_init_msg,
   eds_proc_unexpected_msg,
   eds_proc_unexpected_msg,
   eds_proc_finalize_msg,
   eds_proc_chan_open_sync_msg,
   eds_proc_chan_open_async_msg,
   eds_proc_chan_close_msg,
   eds_proc_chan_unlink_msg,
   eds_proc_unexpected_msg,
   eds_proc_unexpected_msg,
   eds_proc_unexpected_msg,
   eds_proc_unexpected_msg,
   eds_proc_unexpected_msg,
   eds_proc_publish_msg,
   eds_proc_subscribe_msg,
   eds_proc_unsubscribe_msg,
   eds_proc_retention_time_clr_msg
};

/* Pattern for 'LOST EVENT' event */
SaEvtEventPatternT gl_lost_evt_pattern[1] =
{
{25,25,(SaUint8T *)SA_EVT_LOST_EVENT}
};

/****************************************************************************
 * Name          : eds_proc_init_msg
 *
 * Description   : This is the function which is called when eds receives a
 *                 EDSV_EDA_INITIALIZE message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
eds_proc_init_msg(EDS_CB *cb, EDSV_EDS_EVT  *evt)
{
   uns32              rc = NCSCC_RC_SUCCESS;
   uns32              async_rc = NCSCC_RC_SUCCESS;
   uns32              loop_count=0;
   SaVersionT         *version=NULL;
   EDSV_MSG           msg;
   EDS_CKPT_DATA      ckpt;
   
   m_EDSV_DEBUG_CONS_PRINTF(" INITIALIZE EVENT....\n");   
   /* Validate the version */
   version = &(evt->info.msg.info.api_info.param.init.version);
   if (!m_EDA_VER_IS_VALID(version))
   {
      /* Send response back with error code */
      rc = SA_AIS_ERR_VERSION;
      m_LOG_EDSV_SF(EDS_INIT_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
      m_EDS_EDSV_INIT_MSG_FILL(msg, rc, 0)
         rc = eds_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                               MDS_SEND_PRIORITY_HIGH);
      if(rc != NCSCC_RC_SUCCESS)
         m_LOG_EDSV_SF(EDS_INIT_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
       
      return(rc);
   }


   /*
    * I'm allocating new reg_id's this way on the wild chance we wrap
    * around the MAX_INT value and try to use a value that's still in use
    * and valid at the begining of the range. This way we'll keep trying
    * until we find the next open slot. But we'll only try a max of
    * MAX_REG_RETRIES times before giving up completly and returning an error.
    * It's highly unlikely we'll ever wrap, but at least we'll handle it.
    */
#define MAX_REG_RETRIES 100
   loop_count = 0;
   do
   {
      loop_count++;

      /* Allocate a new regID */
      if (cb->last_reg_id == INT_MAX)   /* Handle integer wrap-around */
         cb->last_reg_id = 0;

      cb->last_reg_id++;

      /* Add this regid to the registration linked list.             */
      rc = eds_add_reglist_entry(cb, evt->fr_dest, cb->last_reg_id);
   }while((rc == NCSCC_RC_FAILURE) && (loop_count < MAX_REG_RETRIES));

   /* If we still have a bad status, return failure */
   if (rc != NCSCC_RC_SUCCESS)
   {
      /* Send response back with error code */
      rc = SA_AIS_ERR_LIBRARY;
      m_LOG_EDSV_SF(EDS_INIT_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
      m_EDS_EDSV_INIT_MSG_FILL(msg, rc, 0)
      rc = eds_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                               MDS_SEND_PRIORITY_HIGH);
      if(rc != NCSCC_RC_SUCCESS) 
        m_LOG_EDSV_SF(EDS_INIT_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
      return(rc);
   }


   /* Send response back with assigned reg_id value */
   rc = SA_AIS_OK;
   m_EDS_EDSV_INIT_MSG_FILL(msg, rc, cb->last_reg_id)
   rc = eds_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                         MDS_SEND_PRIORITY_HIGH);
   if(rc != NCSCC_RC_SUCCESS) 
      m_LOG_EDSV_SF(EDS_INIT_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
   else
      m_LOG_EDSV_SF(EDS_INIT_SUCCESS,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,0,evt->fr_dest);
   if(cb->ha_state == SA_AMF_HA_ACTIVE) /*Revisit this */
   {
      m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt)); 
      m_EDSV_FILL_ASYNC_UPDATE_REG(ckpt,cb->last_reg_id,evt->fr_dest)
      async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD); 
      if (async_rc != NCSCC_RC_SUCCESS)
      {
        /* log it */
      }
      else
      {
         m_EDSV_DEBUG_CONS_PRINTF("REG_REC ASYNC UPDATE SEND SUCCESS.....\n"); 
      }
      
   }
   return rc;
}

/****************************************************************************
 * Name          : eds_proc_finalize_msg
 *
 * Description   : This is the function which is called when eds receives a
 *                 EDSV_EDA_FINALIZE message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
eds_proc_finalize_msg(EDS_CB *cb, EDSV_EDS_EVT  *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 async_rc = NCSCC_RC_SUCCESS;
   EDS_CKPT_DATA ckpt; 

   m_EDSV_DEBUG_CONS_PRINTF(" FINALIZE EVENT....\n"); 
   /* This call will insure all open subscriptions, channels, and any other */
   /* resources allocated by this registration are freed up.                */
    rc = eds_remove_reglist_entry(
                      cb, evt->info.msg.info.api_info.param.finalize.reg_id,
                      FALSE);
    if (rc == NCSCC_RC_SUCCESS)
    {
      m_LOG_EDSV_SF(EDS_FINALIZE_SUCCESS,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,0,evt->fr_dest);
      if(cb->ha_state == SA_AMF_HA_ACTIVE) /*Revisit this */
      {  
         m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt));
         m_EDSV_FILL_ASYNC_UPDATE_FINALIZE(ckpt,evt->info.msg.info.api_info.param.finalize.reg_id)
         async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
         if (async_rc != NCSCC_RC_SUCCESS)
         {
           /* log it */
         }
         else
         {
            m_EDSV_DEBUG_CONS_PRINTF("FINALIZE: ASYNC UPDATE SEND SUCCESS....\n");
         }
      }
    }
    else
      m_LOG_EDSV_SF(EDS_FINALIZE_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
    

   return rc;
}

/****************************************************************************
 * Name          : eds_proc_chan_open_sync_msg
 *
 * Description   : This is the function which is called when eds receives a
 *                 EDSV_EDA_CHAN_OPEN message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
eds_proc_chan_open_sync_msg(EDS_CB *cb, EDSV_EDS_EVT  *evt)
{
   uns32  rc = NCSCC_RC_SUCCESS, rs = NCSCC_RC_SUCCESS, async_rc = NCSCC_RC_SUCCESS;
   uns32  chan_id;
   uns32  chan_open_id;
   EDSV_MSG  msg;
   time_t time_of_day;
   SaTimeT chan_create_time = 0;
   EDSV_EDA_CHAN_OPEN_SYNC_PARAM  *open_sync_param;
   EDS_CKPT_DATA    ckpt;

   m_EDSV_DEBUG_CONS_PRINTF("CHANNEL OPEN EVENT....\n");
   open_sync_param = &(evt->info.msg.info.api_info.param.chan_open_sync);
 
   /* Set the open  time here */
   m_NCS_OS_GET_TIME_STAMP(time_of_day);

   /* convert time_t to SaTimeT */
   chan_create_time = (SaTimeT)time_of_day;

   rs = eds_channel_open(cb,
                         open_sync_param->reg_id,
                         open_sync_param->chan_open_flags,
                         open_sync_param->chan_name.length,
                         open_sync_param->chan_name.value,
                         evt->fr_dest,
                         &chan_id,
                         &chan_open_id,
                         chan_create_time);
   if(rs != SA_AIS_OK)
      m_LOG_EDSV_SF(EDS_CHN_OPEN_SYNC_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rs,__FILE__,__LINE__,0,evt->fr_dest);
    
   /* Send response back */
   m_EDS_EDSV_CHAN_OPEN_SYNC_MSG_FILL(msg, rs, chan_id, chan_open_id)
   rc = eds_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                         MDS_SEND_PRIORITY_HIGH);
   if(rc != SA_AIS_OK)
      m_LOG_EDSV_SF(EDS_CHN_OPEN_SYNC_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
   
   if(rs == SA_AIS_OK)
   {
      /* Send an Async update to STANDBY EDS peer */
      if(cb->ha_state == SA_AMF_HA_ACTIVE)
      {
         m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt));
         m_EDSV_FILL_ASYNC_UPDATE_CHAN(ckpt,open_sync_param,chan_id,chan_open_id,evt->fr_dest,chan_create_time)
         async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
         if(async_rc != NCSCC_RC_SUCCESS)
         {
           /* Log it */
         }
         else
         {
           m_EDSV_DEBUG_CONS_PRINTF("CHAN_OPEN:ASYNC UPDATE SEND SUCCESS...\n");
         }
       }
      m_LOG_EDSV_SF(EDS_CHN_OPEN_SYNC_SUCCESS,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_INFO,rs,__FILE__,__LINE__,rc,evt->fr_dest);
   
    }
   return rc;
}

/****************************************************************************
 * Name          : eds_proc_chan_open_async_msg
 *
 * Description   : This is the function which is called when eds receives a
 *                 EDSV_EDA_CHAN_OPEN_ASYNC message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
eds_proc_chan_open_async_msg(EDS_CB *cb, EDSV_EDS_EVT  *evt)
{
   uns32  rc = NCSCC_RC_SUCCESS, rs = NCSCC_RC_SUCCESS , async_rc =NCSCC_RC_SUCCESS;
   uns32  chan_id;
   uns32  chan_open_id;
   EDSV_MSG  msg;
   time_t time_of_day;
   SaTimeT chan_create_time = 0;
   EDSV_EDA_CHAN_OPEN_ASYNC_PARAM  *open_async_param;
   EDS_CKPT_DATA ckpt;

   m_EDSV_DEBUG_CONS_PRINTF(" CHANNEL OPEN ASYNC EVENT....\n"); 
   open_async_param = &(evt->info.msg.info.api_info.param.chan_open_async);

   /* Set the open  time here */
   m_NCS_OS_GET_TIME_STAMP(time_of_day);

   /* convert time_t to SaTimeT */
   chan_create_time = (SaTimeT)time_of_day;

   rs = eds_channel_open(cb,
                         open_async_param->reg_id,
                         open_async_param->chan_open_flags,
                         open_async_param->chan_name.length,
                         open_async_param->chan_name.value,
                         evt->fr_dest,
                         &chan_id,
                         &chan_open_id,
                         chan_create_time);

   if(rs != SA_AIS_OK)
      m_LOG_EDSV_SF(EDS_CHN_OPEN_ASYNC_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rs,__FILE__,__LINE__,0,evt->fr_dest);

   /* Send async response back */
   m_EDS_EDSV_CHAN_OPEN_CB_MSG_FILL(msg, open_async_param->reg_id,
                                    open_async_param->inv,
                                    open_async_param->chan_name,
                                    chan_id, chan_open_id,
                                    open_async_param->chan_open_flags, rs)
   rc = eds_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                         MDS_SEND_PRIORITY_MEDIUM);
   if(rc != NCSCC_RC_SUCCESS)
      m_LOG_EDSV_SF(EDS_CHN_OPEN_ASYNC_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
   
   if(rs == SA_AIS_OK)
   {
      /* Send an Async update to STANDBY EDS peer */
      if(cb->ha_state == SA_AMF_HA_ACTIVE)
      {  
         m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt));
         m_EDSV_FILL_ASYNC_UPDATE_CHAN(ckpt,open_async_param,chan_id,chan_open_id,evt->fr_dest,chan_create_time)
         async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
         if(async_rc != NCSCC_RC_SUCCESS)
         { 
           /* Log it */
         }
         else
         {
            m_EDSV_DEBUG_CONS_PRINTF("COPENASYNC: ASYNC UPDATE SEND SUCCESS...\n");
         }
      }
      m_LOG_EDSV_SF(EDS_CHN_OPEN_ASYNC_SUCCESS,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_INFO,rs,__FILE__,__LINE__,rc,evt->fr_dest);
    }

   return rc;
}

/****************************************************************************
 * Name          : eds_proc_chan_close_msg
 *
 * Description   : This is the function which is called when eds receives a
 *                 EDSV_EDA_CHAN_CLOSE message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
eds_proc_chan_close_msg(EDS_CB *cb, EDSV_EDS_EVT  *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS , async_rc = NCSCC_RC_SUCCESS;
   EDSV_EDA_CHAN_CLOSE_PARAM  *close_param;
   EDS_CKPT_DATA ckpt;
   
   close_param = &(evt->info.msg.info.api_info.param.chan_close);
   m_EDSV_DEBUG_CONS_PRINTF(" CHANNEL CLOSE EVENT....\n");
   rc = eds_channel_close(cb,
                          close_param->reg_id,
                          close_param->chan_id,
                          close_param->chan_open_id,
                          FALSE);
   if(rc == NCSCC_RC_SUCCESS)
   {
        /* Send an Async update to STANDBY EDS peer */
     if(cb->ha_state == SA_AMF_HA_ACTIVE)
     {  
        m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt)); 
        m_EDSV_FILL_ASYNC_UPDATE_CCLOSE(ckpt,close_param->reg_id,close_param->chan_id,close_param->chan_open_id)
        async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
        if(async_rc != NCSCC_RC_SUCCESS)
        {
          /* Log it */
        }
        else
        { 
          m_EDSV_DEBUG_CONS_PRINTF("CHAN_CLOSE:ASYNC UPDATE SEND SUCCESS...\n");
        }
     }
     m_LOG_EDSV_SF(EDS_CHN_CLOSE_SUCCESS,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,0,evt->fr_dest);

   }
   else
     m_LOG_EDSV_SF(EDS_CHN_CLOSE_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
   return rc;
}

/****************************************************************************
 * Name          : eds_proc_chan_unlink_msg
 *
 * Description   : This is the function which is called when eds receives a
 *                 EDSV_EDA_CHAN_UNLINK message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
eds_proc_chan_unlink_msg(EDS_CB *cb, EDSV_EDS_EVT  *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS, rs = NCSCC_RC_SUCCESS , async_rc = NCSCC_RC_SUCCESS;
   EDSV_EDA_CHAN_UNLINK_PARAM  *unlink_param;
   EDSV_MSG msg;
   EDS_CKPT_DATA ckpt;

   unlink_param = &(evt->info.msg.info.api_info.param.chan_unlink);
   m_EDSV_DEBUG_CONS_PRINTF(" CHANNEL UNLINK EVENT....\n");
   rs = eds_channel_unlink(cb,
                           unlink_param->chan_name.length,
                           unlink_param->chan_name.value);
   if(rs != SA_AIS_OK)
     m_LOG_EDSV_SF(EDS_CHN_UNLINK_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rs,__FILE__,__LINE__,0,evt->fr_dest);
     
     /* Send response back */
   m_EDS_EDSV_CHAN_UNLINK_SYNC_MSG_FILL(msg, rs, unlink_param->chan_name)
   rc = eds_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                         MDS_SEND_PRIORITY_HIGH);
   if(rc != NCSCC_RC_SUCCESS)
     m_LOG_EDSV_SF(EDS_CHN_UNLINK_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);

  if(rs == SA_AIS_OK)
  {      /* Send an Async update to STANDBY EDS peer */
    if(cb->ha_state == SA_AMF_HA_ACTIVE)
    {
       m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt));
       m_EDSV_FILL_ASYNC_UPDATE_CUNLINK(ckpt,unlink_param->chan_name.value,unlink_param->chan_name.length)
       async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
       if(async_rc != NCSCC_RC_SUCCESS)
       {
         /* Log it */
       }
       else
       {
          m_EDSV_DEBUG_CONS_PRINTF("CHAN_UNLINK: ASYNC UPDATE SEND SUCCESS...\n");
       }
    }
    m_LOG_EDSV_SF(EDS_CHN_UNLINK_SUCCESS,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,0,evt->fr_dest);
  }
   return rc;
}

/****************************************************************************
 * Name          : eds_proc_publish_msg
 *
 * Description   : This is the function which is called when eds receives a
 *                 EDSV_EDA_PUBLISH message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
eds_proc_publish_msg(EDS_CB *cb, EDSV_EDS_EVT *evt)
{
   uns32                   rc = NCSCC_RC_SUCCESS;
   uns32                   async_rc = NCSCC_RC_SUCCESS;
   uns32                   copen_id_Net;
   SaTimeT                 publish_time = 0;         /* placeholder for now */
   MDS_SEND_PRIORITY_TYPE  prio;
   EDS_WORKLIST            *wp;
   CHAN_OPEN_REC           *co;
   SUBSC_REC               *subrec;
   EDSV_MSG                msg;
   time_t                  time_of_day;
   EDSV_EDA_PUBLISH_PARAM  *publish_param;
   uns32                   retd_evt_chan_open_id = 0;
   EDS_CKPT_DATA           ckpt;
   publish_param = &(evt->info.msg.info.api_info.param).publish;
   m_EDSV_DEBUG_CONS_PRINTF(" PUBLISH EVENT....\n"); 
   
   /* Get worklist ptr for this chan */
   wp = eds_get_worklist_entry(cb->eds_work_list, publish_param->chan_id);
   if (!wp)
   {
      m_LOG_EDSV_SF(EDS_PUBLISH_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,NCSCC_RC_FAILURE,__FILE__,__LINE__,publish_param->chan_id,evt->fr_dest);
      return(NCSCC_RC_FAILURE);
   }
   /* Set the publish time here */
   m_NCS_OS_GET_TIME_STAMP(time_of_day);

   /* convert time_t to SaTimeT */
   publish_time = (SaTimeT)time_of_day;
   
   /* Log the event header info */
#if (NCS_EDSV_LOG == 1)
   eds_publish_log_event(wp, publish_param, publish_time);
#endif


   /** If this event needs to be retained then 
    ** we need to validate the publisher's copen
    ** id and retain the event
    **/
   if (publish_param->retention_time > 0)
   {
      /* Grab the chan_open_rec for this id */
      copen_id_Net = m_NCS_OS_HTONL(publish_param->chan_open_id);
      if (NULL == (co = (CHAN_OPEN_REC *)ncs_patricia_tree_get(&wp->chan_open_rec,
                                                               (uns8*)&copen_id_Net)))
      {
         m_LOG_EDSV_SF(EDS_PUBLISH_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,NCSCC_RC_FAILURE,__FILE__,__LINE__,copen_id_Net,evt->fr_dest);
         return(NCSCC_RC_FAILURE);
      }

      rc = eds_store_retained_event(cb, wp, co, publish_param, publish_time);
      if (rc != NCSCC_RC_SUCCESS)
      {
         m_LOG_EDSV_SF(EDS_PUBLISH_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,NCSCC_RC_FAILURE,__FILE__,__LINE__,0,evt->fr_dest);
         return(NCSCC_RC_FAILURE);
      }
      else
         /** store the parent channel's event id 
          **/
         retd_evt_chan_open_id = co->chan_open_id;
      
   }

   /** Let's try to publish
    ** this event now
    **/

   /* Go through all chan_open_rec's under this channel */
   co = (CHAN_OPEN_REC *)ncs_patricia_tree_getnext(&wp->chan_open_rec, (uns8*)0);
   while(co)
   {
      subrec = co->subsc_rec_head;
      while(subrec)                /* while subscriptions... */
      {
         /* Does the patterns/filters match? */
         if (eds_pattern_match(publish_param->pattern_array, subrec->filters))
         {
            
            /* Fill in the event record to send */
            m_EDS_EDSV_DELIVER_EVENT_CB_MSG_FILL(msg,
                                                 co->reg_id,
                                                 subrec->subscript_id,
                                                 subrec->chan_id,
                                                 subrec->chan_open_id,
                                                 publish_param->pattern_array,
                                                 publish_param->priority,
                                                 publish_param->publisher_name,
                                                 publish_time,
                                                 publish_param->retention_time,
                                                 publish_param->event_id,  
                                                 retd_evt_chan_open_id,
                                                 publish_param->data_len,
                                                 publish_param->data)

               /* Determine evt to MDS priority mapping */
               prio = edsv_map_ais_prio_to_mds_snd_prio(publish_param->priority);

               /* Send the event. Only once per match/per open_id */
               if (NCSCC_RC_SUCCESS != (rc = eds_mds_msg_send(cb,&msg,&co->chan_opener_dest,
                                        NULL,prio)))
               {
                    m_LOG_EDSV_SF(EDS_PUBLISH_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,msg.type,__FILE__,__LINE__,m_NCS_NODE_ID_FROM_MDS_DEST(co->chan_opener_dest),evt->fr_dest);
                    
               }

#if 0             
            rc = eds_mds_ack_send(cb,&msg,co->chan_opener_dest,EDA_MDS_DEF_TIMEOUT,prio);
            
            if ( rc != NCSCC_RC_SUCCESS)
            {
                /* The event was lost due to underlying transport mechanism *
                 * Send a "LOST EVENT" event. 'B' spec compliance 
                 */

               m_LOG_EDSV_SF(EDS_PUBLISH_LOST_EVENT,NCSFL_LC_EDSV_DATA,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,m_NCS_NODE_ID_FROM_MDS_DEST(co->chan_opener_dest),evt->fr_dest);
               
               /* Construct "lost event" header */

               m_NCS_MEMSET(&lost_evt_msg, 0, sizeof(EDSV_MSG));
               lost_evt_msg.type = EDSV_EDS_CBK_MSG;
               lost_evt_msg.info.cbk_info.type = EDSV_EDS_DELIVER_EVENT;

               m_EDSV_LOST_EVENT_FILL(&(lost_evt_msg.info.cbk_info.param.evt_deliver_cbk));
               if (NCSCC_RC_SUCCESS != (rc = eds_mds_msg_send(cb,&lost_evt_msg,&co->chan_opener_dest,
                                    &evt->mds_ctxt,prio)))
               {
                    m_LOG_EDSV_SF(EDS_PUBLISH_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,msg.type,__FILE__,__LINE__,m_NCS_NODE_ID_FROM_MDS_DEST(co->chan_opener_dest),evt->fr_dest);
                    
               }
               if (cb->ha_state == SA_AMF_HA_STANDBY)
                   wp->chan_row.num_lost_evts +=1;

              /* Free the lost event pattern array */
              m_MMGR_FREE_EVENT_PATTERN_ARRAY(lost_evt_msg.info.cbk_info.param.evt_deliver_cbk.pattern_array);

            }/* End LOST events handling */
#endif  /* Not the right way of fixing the Lost Event Machanism */
            break;
         }
         subrec = subrec->next;
      }
      co = (CHAN_OPEN_REC *)ncs_patricia_tree_getnext(&wp->chan_open_rec,
                                                      (uns8*)&co->copen_id_Net);
   }

  /** If this event has been retained, send an async update &
   ** transfer memory ownership here.
   **/
   if (0 != retd_evt_chan_open_id)
   {
        /* Send an Async update to STANDBY EDS peer */
     if(cb->ha_state == SA_AMF_HA_ACTIVE)
     {  
        m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt));
        m_EDSV_FILL_ASYNC_UPDATE_RETAIN_EVT(ckpt,publish_param,publish_time) 
        async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
       if(async_rc != NCSCC_RC_SUCCESS)
       {
          /* Log it */
       }
       else
       {
          m_EDSV_DEBUG_CONS_PRINTF("RETEN_EVT: ASYNC UPDATE SEND SUCCESS....\n");
       }
     }
      /* Now give the patternarray & data ownership to the reten record */
      publish_param->pattern_array = NULL;
      publish_param->data_len = 0;
      publish_param->data = NULL;
   }/* End retd event chan open id */

   return rc;
}


/****************************************************************************
 * Name          : eds_proc_subscribe_msg
 *
 * Description   : This is the function which is called when eds receives a
 *                 EDSV_EDA_SUBSCRIBE message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
eds_proc_subscribe_msg(EDS_CB *cb, EDSV_EDS_EVT  *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 async_rc = NCSCC_RC_SUCCESS;
   EDA_REG_REC   *reglst;
   SUBSC_REC     *subrec;
   EDSV_EDA_SUBSCRIBE_PARAM  *subscribe_param;
   EDS_WORKLIST  *channel_entry;
   EDS_RETAINED_EVT_REC *retd_evt_rec;
   MDS_SEND_PRIORITY_TYPE  prio;
   EDSV_MSG       msg;
   EDS_CKPT_DATA ckpt;
   SaUint8T         list_iter;

   subscribe_param = &(evt->info.msg.info.api_info.param.subscribe);
   m_EDSV_DEBUG_CONS_PRINTF(" SUBSCRIBE EVENT....\n"); 
   
   /* Make sure this is a valid regID */
   reglst = eds_get_reglist_entry(cb, subscribe_param->reg_id);
   if (reglst == NULL)
   {
      m_LOG_EDSV_SF(EDS_SUBSCRIBE_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,NCSCC_RC_FAILURE,__FILE__,__LINE__,0,evt->fr_dest);
      return(NCSCC_RC_FAILURE);
   }
   /* Allocate a new Subscription Record */
   subrec = m_MMGR_ALLOC_EDS_SUBREC(sizeof(SUBSC_REC));
   if (subrec == NULL)
   {
      m_LOG_EDSV_SF(EDS_MEM_ALLOC_FAILED,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,NCSCC_RC_FAILURE,__FILE__,__LINE__,0,evt->fr_dest);
      return(NCSCC_RC_FAILURE);
   }
   m_NCS_MEMSET(subrec, 0, sizeof(SUBSC_REC));

   /* Fill in some data */
   subrec->reg_list      = reglst;
   subrec->subscript_id  = subscribe_param->sub_id;
   subrec->chan_id       = subscribe_param->chan_id;
   subrec->chan_open_id  = subscribe_param->chan_open_id;

   /* Take ownership of the filters memory.           */
   /* We'll free it when the subscription is removed. */
   subrec->filters = subscribe_param->filter_array;
  
   /* Add the subscription to our internal lists */
   rc = eds_add_subscription(cb, subscribe_param->reg_id, subrec);
   if (rc != NCSCC_RC_SUCCESS)
   {
      subscribe_param->filter_array = NULL;
      m_LOG_EDSV_SF(EDS_SUBSCRIBE_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
      return(NCSCC_RC_FAILURE);        /* and return failure */
   }

   /** Find the channel entry to look for 
    ** retained events.
    **/
   channel_entry = 
      eds_get_worklist_entry(cb->eds_work_list, 
                              subrec->chan_id);

   /* Check for any retained events in this channel 
    * that need to be published as 
    * they might match the subscription.
    */
   if (NULL != channel_entry)
   {

       for(list_iter = SA_EVT_HIGHEST_PRIORITY; list_iter <= SA_EVT_LOWEST_PRIORITY ; list_iter++)
       {
          retd_evt_rec = channel_entry->ret_evt_list_head[list_iter];
          while( retd_evt_rec)
          {
                if (eds_pattern_match(retd_evt_rec->patternArray, 
                               subscribe_param->filter_array)) 
                {
                   /* Fill in the event record to send */
                   m_EDS_EDSV_DELIVER_EVENT_CB_MSG_FILL(msg,
                        subscribe_param->reg_id,
                        subrec->subscript_id,
                        subrec->chan_id,
                        subrec->chan_open_id,
                        retd_evt_rec->patternArray,
                        retd_evt_rec->priority,
                        retd_evt_rec->publisherName,
                        retd_evt_rec->publishTime,
                        retd_evt_rec->retentionTime,
                        retd_evt_rec->event_id,  
                        retd_evt_rec->retd_evt_chan_open_id,
                        retd_evt_rec->data_len,
                        retd_evt_rec->data)
               
                   /* Determine evt to MDS priority mapping */
                   prio = 
                     edsv_map_ais_prio_to_mds_snd_prio(retd_evt_rec->priority);
                   if (NCSCC_RC_SUCCESS != (rc = eds_mds_msg_send(cb,&msg,&evt->fr_dest,
                                                     NULL,prio)))
                   {
                         m_LOG_EDSV_SF(EDS_SUBSCRIBE_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,msg.type,evt->fr_dest);
                   }

#if 0               
                   rc = eds_mds_ack_send(cb,&msg,evt->fr_dest,EDA_MDS_DEF_TIMEOUT,prio);
                   if(rc != NCSCC_RC_SUCCESS)
                   {  /* The event was lost */
                       m_LOG_EDSV_SF(EDS_SUBSCRIBE_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,m_NCS_NODE_ID_FROM_MDS_DEST(evt->fr_dest),evt->fr_dest);
                       m_EDSV_LOST_EVENT_FILL(&(msg.info.cbk_info.param.evt_deliver_cbk));
                       if (NCSCC_RC_SUCCESS != (rc = eds_mds_msg_send(cb,&msg,&evt->fr_dest,
                                                      &evt->mds_ctxt,prio)))
                       {
                           m_LOG_EDSV_SF(EDS_SUBSCRIBE_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,msg.type,evt->fr_dest);
                       }
                   }/* End lost event */
                if(rc != NCSCC_RC_SUCCESS)
                  channel_entry->chan_row.num_lost_evts +=1;
#endif
                }/*end pattern match */
                retd_evt_rec = retd_evt_rec->next;
          }
      }
   }
               
      
      
   
   /* Send an async update to the STANDBY EDS peer */
     if(cb->ha_state == SA_AMF_HA_ACTIVE)
     {
        m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt));
        m_EDSV_FILL_ASYNC_UPDATE_SUBSCRIBE(ckpt,subscribe_param)
        async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
       if(async_rc != NCSCC_RC_SUCCESS)
       {
          /* Log it */
       }
       else
       {
          m_EDSV_DEBUG_CONS_PRINTF(" SUBSCRIBE: ASYNC UPDATE SEND SUCCESS....\n");
       }
     }
 
   /* Transfer memory ownership to the 
    * subscription record now 
    */
   subscribe_param->filter_array = NULL;

   return rc;
}

/****************************************************************************
 * Name          : eds_proc_unsubscribe_msg
 *
 * Description   : This is the function which is called when eds receives a
 *                 EDSV_EDA_UNSUBSCRIBE message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
eds_proc_unsubscribe_msg(EDS_CB *cb, EDSV_EDS_EVT  *evt)
{
   uns32   rc = NCSCC_RC_SUCCESS;
   uns32   async_rc = NCSCC_RC_SUCCESS;
   EDSV_EDA_UNSUBSCRIBE_PARAM  *unsubscribe_param;
   EDS_CKPT_DATA ckpt;
   
   unsubscribe_param = &(evt->info.msg.info.api_info.param.unsubscribe);
   m_EDSV_DEBUG_CONS_PRINTF(" UNSUBSCRIBE EVENT....\n"); 
   /* Remove subscription from our lists */
   rc = eds_remove_subscription(cb,
                                unsubscribe_param->reg_id,
                                unsubscribe_param->chan_id,
                                unsubscribe_param->chan_open_id,
                                unsubscribe_param->sub_id);
   if (rc == NCSCC_RC_SUCCESS)
   {
      /* Send an async checkpoint update to STANDBY EDS */
      if(cb->ha_state == SA_AMF_HA_ACTIVE)
      {
         m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt));
         m_EDSV_FILL_ASYNC_UPDATE_UNSUBSCRIBE(ckpt,unsubscribe_param)
         async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
        if(async_rc != NCSCC_RC_SUCCESS)
        {
           /* Log it */
        }
        else
        {
           m_EDSV_DEBUG_CONS_PRINTF(" UNSUBSCRIBE: ASYNC UPDATE SEND SUCCESS....\n");
        }
      }

      m_LOG_EDSV_SF(EDS_UNSUBSCRIBE_SUCCESS,NCSFL_LC_EDSV_DATA,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,0,evt->fr_dest);
   }
   else
   {
      rc = NCSCC_RC_FAILURE;
      m_LOG_EDSV_SF(EDS_UNSUBSCRIBE_FAILURE,NCSFL_LC_EDSV_DATA,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
   }
   return rc;
}

/****************************************************************************
 * Name          : eds_proc_retention_time_clr_msg
 *
 * Description   : This is the function which is called when eds receives a
 *                 EDSV_EDA_RETENTION_TIME_CLR message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
eds_proc_retention_time_clr_msg(EDS_CB *cb, EDSV_EDS_EVT  *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS,rs = NCSCC_RC_SUCCESS;
   uns32 async_rc = NCSCC_RC_SUCCESS;
   EDSV_MSG msg;

   m_EDSV_DEBUG_CONS_PRINTF(" RETENTION TIMER CLEAR EVENT....\n"); 
   EDSV_EDA_RETENTION_TIME_CLR_PARAM *param =
      &(evt->info.msg.info.api_info.param.rettimeclr);
   EDS_CKPT_DATA ckpt;

   
   /* Lock the EDS_CB */
   m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE);

   rc = eds_clear_retained_event(cb,
                                 param->chan_id,
                                 param->chan_open_id,
                                 param->event_id,
                                 FALSE);
   if(rc != NCSCC_RC_SUCCESS)
      m_LOG_EDSV_SF(EDS_RETENTION_TMR_CLR_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
#if 0   
   if (NULL != (wp = 
            eds_get_worklist_entry(cb->eds_work_list, param->chan_id)))
   {
      
     /** If no one else interested in this channel, 
      ** remove it completely 
      **/
      if (wp->use_cnt == 0 
          && 
          wp->ret_evt_list_head == NULL)
         eds_remove_worklist_entry(cb, wp->chan_id);
   }
#endif

   /* Unlock the EDS_CB */
   m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
   
   /* Send response back */
   m_EDS_EDSV_CHAN_RETENTION_TMR_CLEAR_SYNC_MSG_FILL(msg, rc)
   rs = eds_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                         MDS_SEND_PRIORITY_HIGH);

   if(rs != NCSCC_RC_SUCCESS) 
      m_LOG_EDSV_SF(EDS_RETENTION_TMR_CLR_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rs,__FILE__,__LINE__,0,evt->fr_dest);
   if(rc == NCSCC_RC_SUCCESS)
   {
      /*Send an async checkpoint update to STANDBY EDS peer */
      if(cb->ha_state == SA_AMF_HA_ACTIVE)
      {
          m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt)); 
          m_EDSV_FILL_ASYNC_UPDATE_RETEN_CLEAR(ckpt,param)
          async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
          if(async_rc != NCSCC_RC_SUCCESS)
          {   
             /* Log it */
          }
          else
          {
            m_EDSV_DEBUG_CONS_PRINTF(" RETEN_TIME_CLR: ASYNC UPDATE SEND SUCCESS....\n");
          }
      }
      m_LOG_EDSV_SF(EDS_RETENTION_TMR_CLR_SUCCESS,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_INFO,rs,__FILE__,__LINE__,rc,evt->fr_dest);
   }
   return rs;
}

/****************************************************************************
 * Name          : eds_proc_unexpected_msg
 *
 * Description   : This is the function which is called when EDS receives a
 *                 an unexpected message which not expected to be sent to the
 *                 EDS.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
eds_proc_unexpected_msg(EDS_CB *cb, EDSV_EDS_EVT  *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   m_LOG_EDSV_SF(EDS_EVT_UNKNOWN,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__,0,evt->fr_dest);

   return rc;
}

/****************************************************************************
 * Name          : eds_proc_invalid_msg
 *
 * Description   : This is the function which is called when EDS receives a
 *                 an unexpected message which not expected to be sent to the
 *                 EDS.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
eds_proc_invalid_msg(EDSV_EDS_EVT  *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   m_LOG_EDSV_SF(EDS_EVT_UNKNOWN,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__,0,evt->fr_dest);

   return rc;
}

/****************************************************************************
 * Name          : eds_proc_eda_misc_msg
 *
 * Description   : This is the function which is called when eds receives any
 *                 internal messages from EDA which are not directly API invocation
 *                 related.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
eds_proc_eda_misc_msg (EDSV_EDS_EVT  *evt)
{
   /** 
    ** Currently there are no messages of this 
    ** category, so just destroy the message
    ** and return. This must have been dispatched in error.
    **/
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_proc_ret_tmr_exp_evt
 *
 * Description   : This is the function which is called when eds receives any
 *                 a retention tmr expiry evt
 *
 * Arguments     : evt  - Evt that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
eds_proc_ret_tmr_exp_evt (EDSV_EDS_EVT  *evt)
{
   EDS_RETAINED_EVT_REC *ret_evt;
   uns32                store_chan_id;
   uns32 rc = NCSCC_RC_SUCCESS;
   EDS_CB  *eds_cb;

   m_EDSV_DEBUG_CONS_PRINTF("RETENTION TIMER EXPIERY EVENT.....\n");
   /* retrieve retained evt */
   if (NULL == (eds_cb = (EDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_EDS, evt->cb_hdl)))
   {
      m_LOG_EDSV_SF(EDS_RETENTION_TMR_EXP_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0,evt->fr_dest);
      return NCSCC_RC_FAILURE;
   }

   /* retrieve retained evt */
   if (NULL == (ret_evt = (EDS_RETAINED_EVT_REC *)ncshm_take_hdl
            (NCS_SERVICE_ID_EDS, evt->info.tmr_info.opq_hdl)))
   {
      m_LOG_EDSV_SF(EDS_RETENTION_TMR_EXP_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0,evt->fr_dest);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_LOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);

   /** store the chan id as we would need it
    ** after the event is freed
    **/
   store_chan_id = ret_evt->chan_id;

/* CHECKPOINT:
   if ( EDS_CB->ha_state == standby)
        compose a EDSV_CKPT_RETENTION_TIME_CLEAR_MSG and send to standby peer. 
*/
   /** This also frees the event **/
   rc = eds_clear_retained_event(eds_cb, 
                     ret_evt->chan_id, 
                     ret_evt->retd_evt_chan_open_id, 
                     ret_evt->event_id,
                     TRUE);
#if 0
   if (rc == NCSCC_RC_SUCCESS)
   {
      if (NULL != (wp = 
         eds_get_worklist_entry(eds_cb->eds_work_list, 
         store_chan_id)))
      {
         
        /** If no one else is interested in this channel, 
         ** remove it completely 
         **/
         if (wp->use_cnt == 0 
            && 
            wp->ret_evt_list_head == NULL)
            eds_remove_worklist_entry(eds_cb, wp->chan_id);
      }
   }
#endif

   m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);

   if (NCSCC_RC_SUCCESS != rc)
   {
      m_LOG_EDSV_SF(EDS_RETENTION_TMR_EXP_FAILURE,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0,evt->fr_dest);
      ncshm_give_hdl(evt->info.tmr_info.opq_hdl);
      ncshm_give_hdl(evt->cb_hdl);
      return rc;
   }


   ncshm_give_hdl(evt->cb_hdl);
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_proc_eda_updn_mds_msg
 *
 * Description   : This is the function which is called when eds receives any
 *                 a EDA UP/DN message via MDS subscription.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
eds_proc_eda_updn_mds_msg (EDSV_EDS_EVT  *evt)
{
   EDS_CB *cb=NULL;
   EDS_CKPT_DATA ckpt;
   uns32 rc=NCSCC_RC_SUCCESS;
   uns32 async_rc=NCSCC_RC_SUCCESS;
   /* Retreive the cb handle */
   if(NULL == (cb = (EDS_CB*) ncshm_take_hdl(NCS_SERVICE_ID_EDS, evt->cb_hdl)))
   {
      m_LOG_EDSV_SF(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0,evt->fr_dest);
      return NCSCC_RC_FAILURE;
   }

   switch(evt->evt_type)
   {
   case EDSV_EDS_EVT_EDA_UP:
        break;
   case EDSV_EDS_EVT_EDA_DOWN:
      if ((cb->ha_state == SA_AMF_HA_ACTIVE)|| (cb->ha_state == SA_AMF_HA_QUIESCED))
      {
         /* Remove this EDA entry from our processing lists */
         eds_remove_regid_by_mds_dest(cb, evt->fr_dest);
         /*Send an async checkpoint update to STANDBY EDS peer */
         if(cb->ha_state == SA_AMF_HA_ACTIVE)
         {
            m_NCS_MEMSET(&ckpt, 0, sizeof(ckpt));
            m_EDSV_FILL_ASYNC_UPDATE_AGENT_DOWN(ckpt,evt->fr_dest)
            async_rc=send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
            if(async_rc != NCSCC_RC_SUCCESS)
            {
              /* Log it */
            }
            else
              m_EDSV_DEBUG_CONS_PRINTF("AGENT DOWN : ASYNC UPDATE SEND SUCCESS...\n");
         }
      }
      else if(cb->ha_state == SA_AMF_HA_STANDBY)
      {
#if 0
          m_NCS_CONS_PRINTF("MDS DOWN MDS DEST:%x in Event Processing ....\n",evt->fr_dest);
#endif
          EDA_DOWN_LIST *eda_down_rec =NULL;
          if( eds_eda_entry_valid(cb , evt->fr_dest) )
          {
              if (NULL == (eda_down_rec = m_MMGR_ALLOC_EDA_DOWN_LIST(sizeof(EDA_DOWN_LIST))))
              {
                  /* Log it*/
                  rc = NCSCC_RC_OUT_OF_MEM;
                  m_LOG_EDSV_S(EDS_MEM_ALLOC_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
                  break;
              }
              m_NCS_MEMSET(eda_down_rec, 0, sizeof(EDA_DOWN_LIST));
              eda_down_rec->mds_dest = evt->fr_dest;
#if 0
              m_NCS_CONS_PRINTF("Added MDS DOWN MDS DEST:%x in Event Processing ....\n",evt->fr_dest);
#endif
              if( cb->eda_down_list_head == NULL )
              {
                  cb->eda_down_list_head = eda_down_rec;
              }
              else
              {
                 if(cb->eda_down_list_tail)
                    cb->eda_down_list_tail->next = eda_down_rec;
              }
              cb->eda_down_list_tail  = eda_down_rec;
          }
      }

/* CHECKPOINT:
    1) Add a MDS_UP_DOWN_FLAG==(TIMER_STARTED | TIMER_RESET) in EDS_CB.
    2) if ( MDS_UP_DOWN_FLAG == TIMER_RESET)
            start the mds_up_down_timer
       else
            do nothing
    3) In the mds_up_down_timer_timeout_handler(), invoke eds_remove_regid_by_mds_dest(). 
*/
      break;
   default:
         break;
   }

   /* Give the cb handle back */
   ncshm_give_hdl(evt->cb_hdl);
   
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_proc_eda_api_msg
 *
 * Description   : This is the function which is called when eds receives any
 *                 EDA_API message.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
eds_proc_eda_api_msg (EDSV_EDS_EVT  *evt)
{
   EDS_CB *cb;

   if  ((evt) &&
       ((evt->info.msg.info.api_info.type >= EDSV_API_BASE_MSG) &&
        (evt->info.msg.info.api_info.type <  EDSV_API_MAX)))
   {
      /* Retrieve the cb handle */
      if(NULL == (cb = (EDS_CB*) ncshm_take_hdl(NCS_SERVICE_ID_EDS, evt->cb_hdl)))
      {
         m_LOG_EDSV_SF(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0,evt->fr_dest);
         return NCSCC_RC_FAILURE;
      }
      if(cb->scalar_objects.svc_state != STOPPED)
      {
       
         if (eds_eda_api_msg_dispatcher[evt->info.msg.info.api_info.type](cb, evt) != NCSCC_RC_SUCCESS)
         {
#if 0
            m_LOG_EDSV_SF(EDS_API_MSG_DISPATCH_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0,evt->fr_dest);
#endif
            ncshm_give_hdl(evt->cb_hdl);
            return NCSCC_RC_FAILURE;
         }
      }
      else
      {
        ncshm_give_hdl(evt->cb_hdl);
        return NCSCC_RC_FAILURE;
      }
      /* Give the cb handle back */
      ncshm_give_hdl(evt->cb_hdl);

   }
   else
   {
      m_LOG_EDSV_SF(EDS_EVT_UNKNOWN,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,evt->evt_type,__FILE__,__LINE__,0,evt->fr_dest);
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_process_api_evt
 *
 * Description   : This is the function which is called when eds receives an
 *                 event either because of an API Invocation or other internal
 *                 messages from EDA clients
 *
 * Arguments     : evt  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
eds_process_api_evt (EDSV_EDS_EVT  *evt)
{
   if (evt->evt_type  == EDSV_EDS_EDSV_MSG)
   {
      if ((evt->info.msg.type  >= EDSV_BASE_MSG) && 
          (evt->info.msg.type  <  EDSV_MSG_MAX))
      {
         if (eds_edsv_evt_dispatch_tbl[evt->info.msg.type](evt) != NCSCC_RC_SUCCESS)
         {
#if 0
            m_LOG_EDSV_SF(EDS_EVENT_PROCESSING_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,evt->info.msg.type,__FILE__,__LINE__,0,evt->fr_dest);
#endif
         }
      }
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_proc_mib_request_evt
 *
 * Description   : This is the function which is called when eds receives an
 *                 mib request event from MAB(MIB Access Broker). 
 *
 * Arguments     : evt  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
eds_proc_mib_request_evt(EDSV_EDS_EVT *evt)
{
  NCSMIBLIB_REQ_INFO  miblib_req;
  EDS_CB *eds_cb;

  if (evt->evt_type  == EDSV_EVT_MIB_REQ)
  {
      /* Retrieve the cb handle */
      if(NULL == (eds_cb = (EDS_CB*) ncshm_take_hdl(NCS_SERVICE_ID_EDS, evt->cb_hdl)))
      {
         m_LOG_EDSV_SF(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0,evt->fr_dest);
         return NCSCC_RC_FAILURE;
      }

      if (evt->info.mib_req == NULL)
      {
         m_LOG_EDSV_SF(EDS_EVENT_PROCESSING_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0,evt->fr_dest);
         return NCSCC_RC_FAILURE;
      }

      m_NCS_MEMSET(&miblib_req, '\0', sizeof(NCSMIBLIB_REQ_INFO));
   
      miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
      miblib_req.info.i_mib_op_info.args = evt->info.mib_req;
      miblib_req.info.i_mib_op_info.cb = eds_cb;
   
      ncsmiblib_process_req(&miblib_req);

      /* Give the cb handle back */
      ncshm_give_hdl(evt->cb_hdl);
      
      ncsmib_memfree(evt->info.mib_req);
      evt->info.mib_req = NULL;
     

      return NCSCC_RC_SUCCESS;
   }
   else
   {
      m_LOG_EDSV_SF(EDS_EVENT_PROCESSING_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0,evt->fr_dest);
      return NCSCC_RC_FAILURE;
   }

} /*End eds_proc_mib_request */

/****************************************************************************
 * Name          : eds_proc_quiesced_ack_evt
 *
 * Description   : This is the function which is called when eds receives an
 *                       quiesced ack event from MDS 
 *
 * Arguments     : evt  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
eds_proc_quiesced_ack_evt(EDSV_EDS_EVT *evt)
{
     EDS_CB *cb;
     /* Retrieve the cb handle */
     if(NULL == (cb = (EDS_CB*) ncshm_take_hdl(NCS_SERVICE_ID_EDS, evt->cb_hdl)))
     {
        m_LOG_EDSV_SF(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0,evt->fr_dest);
        return NCSCC_RC_FAILURE;
     }

     if(cb->is_quisced_set == TRUE)
     {
        cb->ha_state = SA_AMF_HA_QUIESCED ;
        /* Inform MBCSV of HA state change */
        if ( eds_mbcsv_change_HA_state(cb) != NCSCC_RC_SUCCESS)
        {
           m_EDSV_DEBUG_CONS_PRINTF("EDS-MBCSV- CHANGE ROLE FAILED....\n");
        }

        /* Update control block */
        saAmfResponse(cb->amf_hdl, cb->amf_invocation_id, SA_AIS_OK);  
        cb->is_quisced_set = FALSE;
        /* Give the cb handle back */
        ncshm_give_hdl(evt->cb_hdl);
     }
     else
     {
       ncshm_give_hdl(evt->cb_hdl);
     }
  return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_process_evt
 *
 * Description   : This is the function which is called when eds receives an
 *                 event of any kind.
 *
 * Arguments     : msg  - Message that was posted to the EDS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
eds_process_evt (EDSV_EDS_EVT  *evt)
{
   EDS_CB *cb;

     /* Retrieve the cb handle */
   if(NULL == (cb = (EDS_CB*) ncshm_take_hdl(NCS_SERVICE_ID_EDS, evt->cb_hdl)))
   {
      m_LOG_EDSV_SF(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0,evt->fr_dest);
      return NCSCC_RC_FAILURE;
    }

   if(cb->ha_state == SA_AMF_HA_ACTIVE)
   { 
     if ((evt->evt_type >= EDSV_EDS_EVT_BASE) && 
         (evt->evt_type <=  EDSV_EVT_MIB_REQ))
     {
          /** Invoke the evt dispatcher **/
          eds_edsv_top_level_evt_dispatch_tbl[evt->evt_type](evt);
     }
     else if(evt->evt_type == EDSV_EVT_QUIESCED_ACK)
     {
              eds_proc_quiesced_ack_evt(evt);
     }
     else
        m_LOG_EDSV_SF(EDS_EVENT_PROCESSING_FAILED,NCSFL_LC_EDSV_CONTROL,NCSFL_SEV_ERROR,evt->evt_type,__FILE__,__LINE__,0,evt->fr_dest);
   }
   else
   {
      if( (evt->evt_type == EDSV_EDS_RET_TIMER_EXP) || (evt->evt_type == EDSV_EDS_EVT_EDA_DOWN))
            /** Invoke the evt dispatcher **/
          eds_edsv_top_level_evt_dispatch_tbl[evt->evt_type](evt);
   }
   
   ncshm_give_hdl(evt->cb_hdl);
   /* Free the event */
   if (NULL != evt)
      eds_evt_destroy(evt);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_evt_destroy
 *
 * Description   : This is the function which is called to destroy an EDS event.
 *
 * Arguments     : struct edsv_eds_evt_tag *.
 *
 * Return Values : NONE
 *
 * Notes         : None.
 *****************************************************************************/
void 
eds_evt_destroy(EDSV_EDS_EVT *evt)
{
   if (EDSV_EDS_EDSV_MSG == evt->evt_type)
   {
      if (EDSV_EDA_API_MSG == evt->info.msg.type)
      {
         if (EDSV_EDA_PUBLISH == evt->info.msg.info.api_info.type)
         {
            /* free the pattern array */
            if (NULL != evt->info.msg.info.api_info.param.publish.pattern_array)
               edsv_free_evt_pattern_array(evt->info.msg.info.api_info.param.publish.pattern_array);

            /* free event data */ 
            if (NULL != evt->info.msg.info.api_info.param.publish.data)
               m_MMGR_FREE_EDSV_EVENT_DATA(evt->info.msg.info.api_info.param.publish.data);

         }
         else if(EDSV_EDA_SUBSCRIBE == evt->info.msg.info.api_info.type)
         {
            /* free the filter array */
            if (NULL != evt->info.msg.info.api_info.param.subscribe.filter_array)
               edsv_free_evt_filter_array(evt->info.msg.info.api_info.param.subscribe.filter_array);
         }
      }
   }
         
   /** There are no other pointers 
    ** off the evt, so free the evt
    **/ 
   m_MMGR_FREE_EDSV_EDS_EVT(evt);
   evt = NULL;

   return;
}

#if (NCS_EDSV_LOG == 1)
/****************************************************************************
 * Name          : eds_publish_log_event
 *
 * Description   : Logs header info for a published event.
 *
 * Arguments     : worklist pointer to the event channel.
 *                 publish_param of the event.
 *                 publish time of the event.
 *
 * Return Values : NONE
 *
 * Notes         : None.
 *****************************************************************************/
static void 
eds_publish_log_event(EDS_WORKLIST *wp,
                      EDSV_EDA_PUBLISH_PARAM  *publish_param,
                      SaTimeT publish_time)
{
   uns32  x;
   uns32  is_ascii;
   int8   *ptr;
   int8   str[524];


   /* See if the channel name is a printable string.
    * If not, just give the length for now.
    */
   is_ascii = TRUE;
   ptr = (int8 *)wp->cname;
   for (x=0; x< wp->cname_len; x++)
   {
      if (isascii(*ptr++))
         continue;
      else
      {
         is_ascii = FALSE;
         break;
      }
   }

   if (is_ascii)
      m_NCS_OS_LOG_SPRINTF(str, "Channel:%s, ", wp->cname);
   else
      m_NCS_OS_LOG_SPRINTF(str, "Channel:<BINARY VALUE Length:%d>, ", wp->cname_len);


   /*
    *  <<< TODO >>>
    * For now we append the channel name and publisher name together.
    * This should be split when the new logging format NCSFL_TYPE_TICCLLL
    * is added. Also will need to modify the format string in eds_logstr.c
    */


   /* See if the publisher name is a printable string.
    * If not, just say it's binary of some length.
    */
   is_ascii = TRUE;
   ptr = (int8 *)publish_param->publisher_name.value;
   for (x=0; x<publish_param->publisher_name.length; x++)
   {
      if (isascii(*ptr++))
         continue;
      else
      {
         is_ascii = FALSE;
         break;
      }
   }

   ptr = &str[strlen(str)];   /* Get to end of previous string */
   if (is_ascii)
      m_NCS_OS_LOG_SPRINTF(ptr, "Publisher:%s", publish_param->publisher_name.value);
   else
      m_NCS_OS_LOG_SPRINTF(ptr, "Publisher:<BINARY VALUE Length:%d>, ",
              publish_param->publisher_name.length);


   m_LOG_EDS_EVENT(EDS_EVENT_HDR_LOG,
                   str,
                   (uns32)publish_param->event_id,
                   (uns32)publish_time,
                   (uns32)publish_param->priority,
                   (uns32)publish_param->retention_time);
}

#endif

/* 
void  eds_copy_evt_to_ckpt(EDSV_API_INFO evt_msg, EDSV_CKPT_DATA *ckpt_msg, MDS_DEST dest)
{
   switch(evt_msg.type)
   {
     case  EDSV_EDA_INITIALIZE:  
                evt_msg.param.init;
     case  EDSV_EDA_FINALIZE:
                evt_msg.param.finalize;
     case  EDSV_EDA_CHAN_OPEN_SYNC:
                 evt_msg.param.chan_open_sync;
     case  EDSV_EDA_CHAN_OPEN_ASYNC:
                 evt_msg.param.chan_open_async;
     case  EDSV_EDA_CHAN_CLOSE:
                 evt_msg.param.chan_close;
     case  EDSV_EDA_CHAN_UNLINK:
                 evt_msg.param.chan_unlink;
     case  EDSV_EDA_PUBLISH:
                 evt_msg.param.publish;
     case  EDSV_EDA_SUBSCRIBE:
                 evt_msg.param.subscribe;
     case  EDSV_EDA_UNSUBSCRIBE:
                 evt_msg.param.unsubscribe;
     case  EDSV_EDA_RETENTION_TIME_CLR:
                 evt_msg.param.rettimeclr;
     default:
}


#define m_EDSV_FILL_ASYNC_UPDATE_REG(regid,dest) \
  ckpt->ckpt_rec.reg_rec.reg_id = regid;\
  ckpt->ckpt_rec.reg_rec.eda_client_dest = dest;

#define m_EDSV_FILL_ASYNC_UPDATE_CHAN(evt,ckpt,dest) \
  ckpt.ckpt_rec.chan_rec.reg_id = evt.param.chan_open_sync.reg_id; \
  ckpt.ckpt_rec.chan_rec.chan_id = 0;\
  ckpt.ckpt_rec.chan_rec.last_copen_id = 0;\
  ckpt.ckpt_rec.chan_rec.chan_attrib = evt.param.chan_open_sync.chan_open_flags; \
  ckpt.ckpt_rec.chan_rec.use_cnt = 0;\
  ckpt.ckpt_rec.chan_rec.chan_opener_dest = dest;\
  ckpt.ckpt_rec.chan_rec.cname_len = evt.param.chan_open_sync.chan_name.value; \
  m_NCS_MEMCPY(ckpt.ckpt_rec.chan_rec.cname,evt.param.chan_open_sync.chan_name.value,ckpt.ckpt_rec.chan_rec.cname_len);


#define m_EDSV_FILL_ASYNC_UPDATE_COPEN(evt,ckpt) \
  ckpt.ckpt_rec.chan_open_rec.reg_id=  \
  ckpt.ckpt_rec.chan_open_rec.chan_id=  \
  ckpt.ckpt_rec.chan_open_rec.chan_open_id=  \
  ckpt.ckpt_rec.chan_open_rec.chan_attrib=  \
  ckpt.ckpt_rec.chan_open_rec.use_cnt=  \
  ckpt.ckpt_rec.chan_open_rec.chan_opener_dest=  \
  ckpt.ckpt_rec.chan_open_rec.cname_len=  \
  m_NCS_MEMCPY(ckpt.ckpt_rec.chan_open_rec.cname,,ckpt.ckpt_rec.chan_open_rec.cname_len); 


#define m_EDSV_FILL_ASYNC_UPDATE_CCLOSE(evt,ckpt)\
  ckpt.ckpt_rec.chan_close_rec.reg_id=evt.param.chan_close.reg_id;  \
  ckpt.ckpt_rec.chan_close_rec.chan_id=evt.param.chan_close.chan_id;  \
  ckpt.ckpt_rec.chan_close_rec.chan_open_id=evt.param.chan_open_id;

#define m_EDSV_FILL_ASYNC_UPDATE_CUNLINK(evt,ckpt)\
  ckpt.ckpt_rec.chan_unlink_rec.reg_id=evt.param;\
  ckpt.ckpt_rec.chan_unlink_rec.chan_name.length=evt.param.chan_unlink.chan_name.length;\
  ckpt.ckpt_rec.chan_unlink_rec.chan_name.value=evt.param.chan_unlink.chan_name.value;

#define m_EDSV_FILL_ASYNC_UPDATE_RETAIN_EVT(evt,ckpt,pattern_array,data)\
  ckpt.ckpt_rec.retain_evt_rec.data.event_id=evt.param.publish.event_id;\
  ckpt.ckpt_rec.retain_evt_rec.data.priority=evt.param.publish.priority;\
  ckpt.ckpt_rec.retain_evt_rec.data.retention_time=evt.param.publish.retention_time;\
  ckpt.ckpt_rec.retain_evt_rec.data.publishTime=evt.param.publish.publishTime;\
  ckpt.ckpt_rec.retain_evt_rec.data.publisher_name.length=evt.param.publish.publisher_name.length;\
  ckpt.ckpt_rec.retain_evt_rec.data.publisher_name.value=evt.param.publish.publisher_name.value;\
  ckpt.ckpt_rec.retain_evt_rec.data.pattern_array=evt.param.publish.pattern_array;\
  ckpt.ckpt_rec.retain_evt_rec.data.data=evt.param.publish.data;\
  ckpt.ckpt_rec.retain_evt_rec.data.data_len=evt.param.publish.data_len;

#define m_EDSV_FILL_ASYNC_UPDATE_SUBSCRIBE(evt,ckpt,filter_array)\
  ckpt.ckpt_rec.subscribe_rec.data.reg_id=evt.param.subscribe.reg_id;\
  ckpt.ckpt_rec.subscribe_rec.data.sub_id=evt.param.subscribe.sub_id;\
  ckpt.ckpt_rec.subscribe_rec.data.chan_id=evt.param.subscribe.chan_id;\
  ckpt.ckpt_rec.subscribe_rec.data.chan_open_id=evt.param.subscribe.chan_open_id;\
  ckpt.ckpt_rec.subscribe_rec.data.filter_array=filter_array;

#define m_EDSV_FILL_ASYNC_UPDATE_UNSUBSCRIBE(evt,ckpt)\
  ckpt.ckpt_rec.unsubscribe_rec.data.reg_id=evt.param.unsubscribe.reg_id;\
  ckpt.ckpt_rec.unsubscribe_rec.data.chan_id=evt.param.unsubscribe.chan_id;\
  ckpt.ckpt_rec.unsubscribe_rec.data.chan_open_id=evt.param.unsubscribe.chan_open_id;\
  ckpt.ckpt_rec.unsubscribe_rec.data.sub_id=evt.param.unsubscribe.sub_id;

#define m_EDSV_FILL_ASYNC_UPDATE_RETEN_TIME_CLR(evt,ckpt)\
  ckpt.ckpt_rec.reten_time_clr_rec.data.chan_id=evt.param.rettimeclr.chan_id;\
  ckpt.ckpt_rec.reten_time_clr_rec.data.chan_open_id=evt.param.rettimeclr.chan_open_id;\
  ckpt.ckpt_rec.reten_time_clr_rec.data.event_id=evt.param.rettimeclr.chan_id;\

*/

