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
FILE NAME: IFSV_MDS.C
DESCRIPTION: IFA-MDS Routines
******************************************************************************/

#include "ifa.h"

MDS_CLIENT_MSG_FORMAT_VER
  IFA_WRT_IFND_MSG_FMT_ARRAY[IFA_WRT_IFND_SUBPART_VER_RANGE]={
           1 /*msg format version for  subpart version 1*/};

/****************************************************************************
 * Name          : ifa_mds_callback
 *
 * Description   : Call back function provided to MDS. MDS will call this
 *                 function for enc/dec/cpy/rcv/svc_evt operations.
 *
 * Arguments     : NCSMDS_CALLBACK_INFO *info: call back info.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifa_mds_callback (NCSMDS_CALLBACK_INFO *info)
{
   uns32    cb_hdl;
   IFA_CB   *cb=NULL;
   uns32    rc = NCSCC_RC_FAILURE;

   if(info == NULL)
      return rc;

   cb_hdl = (uns32) info->i_yr_svc_hdl;

   if((cb = (IFA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_IFA, cb_hdl)) == NULL)
   {
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"ncshm_take_hdl returned NULL",0);
      return NCSCC_RC_SUCCESS;
   }

   switch(info->i_op)
   {
   case MDS_CALLBACK_ENC:
   case MDS_CALLBACK_ENC_FLAT:
      rc = ifa_mds_enc(cb, &info->info.enc);
      if(rc != NCSCC_RC_SUCCESS)
      {
       m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifa_mds_enc() returned failure"," ");
      }
      break;

   case MDS_CALLBACK_DEC:
   case MDS_CALLBACK_DEC_FLAT:
      rc = ifa_mds_dec(cb, &info->info.dec);
      if(rc != NCSCC_RC_SUCCESS)
      {
       m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifa_mds_dec() returned failure"," ");
      }
      break;

   case MDS_CALLBACK_RECEIVE:
      rc = ifa_mds_rcv(cb, &info->info.receive);
      if(rc != NCSCC_RC_SUCCESS)
      {
       m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifa_mds_rcv() returned failure"," ");
      }
      break;

   case MDS_CALLBACK_COPY:
      rc = ifa_mds_cpy(cb, &info->info.cpy);
      if(rc != NCSCC_RC_SUCCESS)
      {
       m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifa_mds_cpy() returned failure"," ");
      }
      break;

   case MDS_CALLBACK_SVC_EVENT:
      rc = ifa_mds_svc_evt(cb, &info->info.svc_evt);
      if(rc != NCSCC_RC_SUCCESS)
      {
       m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifa_mds_svc_evt() returned failure"," ");
      }
      break;

   default:
      /* IFND will not get copy and callback event */
      rc = NCSCC_RC_FAILURE;
      break;
   }

   if(rc != NCSCC_RC_SUCCESS)
   {
    /*
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                     "Error in ifa_mds_callback processing",0);
     */
       m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"Error in ifa_mds_callback processing"," ");
    
   }

   ncshm_give_hdl((uns32)cb_hdl);

   return rc;

}

/****************************************************************************
 * Name          : ifa_mds_adest_get
 *
 * Description   : This function Gets the Handles of local MDS destination
 *
 * Arguments     : cb   : IFND control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifa_mds_adest_get (IFA_CB *cb)
{
   NCSADA_INFO   arg;
   uns32         rc;

   m_NCS_OS_MEMSET(&arg,0,sizeof(NCSADA_INFO));

   arg.req   = NCSADA_GET_HDLS;

   rc = ncsada_api(&arg);

   if(rc != NCSCC_RC_SUCCESS)
   {
    /*
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                     "Couldn't get MDS handle",0);
     */
     m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsada_api() returned failure"," ");

     return rc;
   }

   cb->my_mds_hdl       = arg.info.adest_get_hdls.o_mds_pwe1_hdl;

   return rc;
}

/****************************************************************************
 * Name          : ifa_mds_init
 *
 * Description   : This function intializes the MDS procedures for IFA 
 *
 * Arguments     : cb   : IFA control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifa_mds_init (IFA_CB *cb)
{
   NCSMDS_INFO          arg;
   uns32                rc;
   MDS_SVC_ID           svc_id;

   /* Create the vertual Destination for IFA */
   rc = ifa_mds_adest_get(cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
     m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifa_mds_adest_get() returned failure"," ");
     return rc;
   }

   /* Install your service into MDS */
   m_NCS_OS_MEMSET(&arg,0,sizeof(NCSMDS_INFO));

   arg.i_mds_hdl        = cb->my_mds_hdl;
   arg.i_svc_id         = NCSMDS_SVC_ID_IFA;
   arg.i_op             = MDS_INSTALL;

   arg.info.svc_install.i_yr_svc_hdl      = cb->cb_hdl;
   arg.info.svc_install.i_install_scope   = NCSMDS_SCOPE_INTRANODE;
   arg.info.svc_install.i_svc_cb          = ifa_mds_callback;
   arg.info.svc_install.i_mds_q_ownership = FALSE;
   arg.info.svc_install.i_mds_svc_pvt_ver = IFA_SVC_PVT_SUBPART_VERSION;
   
   rc = ncsmds_api(&arg);

   if (rc != NCSCC_RC_SUCCESS)
   {
    /*
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                     "Couldn't install mds",0);
     */
     m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsmds_api() returned failure,Couldn't install mds"," ");
      goto install_err;
   }
   

   /* Store the self destination */
   cb->my_dest = arg.info.svc_install.o_dest;
   cb->my_anc  = arg.info.svc_install.o_anc;

   /* IFA is subscribing for IFND MDS service */
   arg.i_op                            = MDS_SUBSCRIBE;
   arg.info.svc_subscribe.i_scope      = NCSMDS_SCOPE_INTRANODE;
   arg.info.svc_subscribe.i_num_svcs   = 1;
   svc_id = NCSMDS_SVC_ID_IFND;
   arg.info.svc_subscribe.i_svc_ids    = &svc_id;

   rc = ncsmds_api(&arg);

   if (rc != NCSCC_RC_SUCCESS)
   {
    /*
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                     "Couldn't subscribe mds",0);
     */
     m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsmds_api() returned failure,Couldn't subscribe mds"," ");
      goto subr_err;
   }
   return (rc);

subr_err:
   /* Uninstall the service */
   arg.i_op                         = MDS_UNINSTALL;
   rc = ncsmds_api(&arg);   /* Don't care the rc */

install_err:
   return rc;
}

/****************************************************************************
 * Name          : ifa_mds_shut
 *
 * Description   : This function un-registers the IFA Service with MDS.
 *
 * Arguments     : cb   : IFA control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
void ifa_mds_shut (IFA_CB *cb)
{
   NCSMDS_INFO          arg;
   uns32                rc;

   /* Un-install your service into MDS. 
   No need to cancel the services that are subscribed*/
   m_NCS_OS_MEMSET(&arg,0,sizeof(NCSMDS_INFO));

   arg.i_mds_hdl        = cb->my_mds_hdl;
   arg.i_svc_id         = NCSMDS_SVC_ID_IFA;
   arg.i_op             = MDS_UNINSTALL;

   rc = ncsmds_api(&arg);

   if (rc != NCSCC_RC_SUCCESS)
   {
    /*
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                     "Couldn't uninstall mds",0);
     */
     m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsmds_api() returned failure,Couldn't uninstall mds"," ");
      return;
   }

   return;
}


/****************************************************************************
 * Name          : ifa_mds_svc_evt
 *
 * Description   : IFA is informed when MDS events occurr that he has 
 *                 subscribed to
 *
 * Arguments     : 
 *   cb          : IFA control Block.
 *   enc_info    : Svc evt info.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifa_mds_svc_evt(IFA_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt)
{
   IFSV_EVT                *evt = NULL;
   uns32 rc = NCSCC_RC_SUCCESS;

   switch (svc_evt->i_change)
   {
   case NCSMDS_DOWN:
         m_IFA_LOG_EVT_LL(IFSV_LOG_IFA_EVT_INFO,\
         "IFSV_LOG_MDS_SVC_DOWN event rcv. svc_evt->i_svc_id,\
          svc_evt->i_node_id",svc_evt->i_svc_id,svc_evt->i_node_id);
         if(svc_evt->i_svc_id == NCSMDS_SVC_ID_IFND)
         {
           cb->is_ifnd_up = FALSE;
         }
         break;
   case NCSMDS_UP:
         m_IFA_LOG_EVT_LL(IFSV_LOG_IFA_EVT_INFO,\
         "IFSV_LOG_MDS_SVC_UP event rcv. svc_evt->i_svc_id,\
          svc_evt->i_node_id",svc_evt->i_svc_id,svc_evt->i_node_id);
      if(svc_evt->i_svc_id == NCSMDS_SVC_ID_IFND)
      {
         cb->is_ifnd_up = TRUE;
         cb->ifnd_dest = svc_evt->i_dest;
         
         /* Need to notify all the records subscribed applications */
         evt = m_MMGR_ALLOC_IFSV_EVT;
         
         if (evt == IFSV_NULL)
         {
            m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
            return (NCSCC_RC_FAILURE);
         }   
         
         m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));
         
         evt->type = IFND_EVT_INTF_INFO_GET;
         evt->info.ifnd_evt.info.if_get.get_type  = IFSV_INTF_GET_ALL;
         evt->info.ifnd_evt.info.if_get.info_type = NCS_IFSV_IF_INFO;
         
         evt->info.ifnd_evt.info.if_get.my_svc_id = NCSMDS_SVC_ID_IFA;
         evt->info.ifnd_evt.info.if_get.my_dest   = cb->my_dest;
         evt->info.ifnd_evt.info.if_get.usr_hdl   = 0;
         /* Here I want all the records, I am not filling the if_get.ifkey */
         
         /* Send the Event to IFND */
         rc = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFA, evt,
            cb->ifnd_dest, NCSMDS_SVC_ID_IFND);
         /* Free the Event */
         if(evt)
          m_MMGR_FREE_IFSV_EVT(evt);

      }
      break;
   default:
      break;
   }
   
   return rc;
}


/****************************************************************************
 * Name          : ifa_mds_rcv
 *
 * Description   : MDS will call this function on receiving IFND messages.
 *
 * Arguments     : cb - IFA Control Block
 *                 rcv_info - MDS Receive information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifa_mds_rcv(IFA_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info)
{
   IFSV_EVT         *evt = (IFSV_EVT *) rcv_info->i_msg;
   IFA_EVT          *ifa_evt;
   IFSV_SUBSCR_INFO *subr_info = NULL;
   uns32 rc = NCSCC_RC_SUCCESS;

   if(evt == NULL)
      return NCSCC_RC_FAILURE;

   m_IFA_LOG_EVT_LL(IFSV_LOG_IFA_EVT_INFO,\
         "IFSV_LOG_MDS_RCV_MSG event rcv. rcv_info->i_fr_svc_id,\
         cb->cb_hdl",rcv_info->i_fr_svc_id,cb->cb_hdl);

   /* Update the sender info, usefull in sync requests */
   evt->sinfo.ctxt = rcv_info->i_msg_ctxt;
   evt->sinfo.dest = rcv_info->i_fr_dest;
   evt->sinfo.to_svc = rcv_info->i_fr_svc_id;
   if(rcv_info->i_rsp_reqd) 
   {
      evt->sinfo.stype = MDS_SENDTYPE_RSP;
   }
   
   if(evt->type <= IFSV_EVT_MAX)
   {
      ifa_evt = &evt->info.ifa_evt;
      
      /* Right now Application callback is called under the MDS context...
      Need to change this if it hits MDS performance.
      The alternative approch is to have a thread for IFA or to call this under 
      application context (using select & Dispatch) */
      
      if(ifa_evt->usrhdl)
      {
         /* Need to inform only to one user */
         subr_info = (IFSV_SUBSCR_INFO *) ncshm_take_hdl(NCS_SERVICE_ID_IFA, 
            (uns32)(long)ifa_evt->usrhdl);
         /* Update the application */
         if(subr_info)
         {
            /* Now check whether the record is for internal or external 
             * interfaces. 
             * If the record is for internal interfaces then send it to all 
             * those applications who has subscribed for the internal 
             * interfaces.
             * If the record is for external interfaces then send it to all 
             * those applications who has subscribed for the external 
             * interfaces. */
             if(ifa_evt->info.ifget_rsp.error == NCS_IFSV_IFGET_RSP_NO_REC)
             {
               /* Since no record is there, so just send the application
                  this info, no need to check for EXT/INT interface. */
               ifa_app_send (cb, &subr_info->info, ifa_evt, evt->type, FALSE);
             }
             else
             {
              /* We have the record, so we have information about the record
                 whether it belongs to EXT or INT or BOTH interface.  */
              if (subr_info->info.subscr_scope == NCS_IFSV_SUBSCR_ALL)
              {
                  ifa_app_send (cb, &subr_info->info, ifa_evt, evt->type, TRUE);
              }
              else
              {
                if (subr_info->info.subscr_scope == ifa_evt->subscr_scope)
                    ifa_app_send (cb, &subr_info->info, ifa_evt, evt->type, TRUE);
                else
                {
                 /* If it doesn't matches, then also we have to send the 
                    application that NO RECORD was found. */
                 ifa_app_send (cb, &subr_info->info, ifa_evt, evt->type, FALSE);
                }
              }
             }
            ncshm_give_hdl((uns32)(long)ifa_evt->usrhdl);
         }
      }
      else
      {
         NCS_DB_LINK_LIST_NODE *list_node;
         
         list_node = m_NCS_DBLIST_FIND_FIRST(&cb->subr_list);
         
         /* Traverse the subscription info, send to all applications */
         while(list_node)
         {
            subr_info = (IFSV_SUBSCR_INFO*)list_node;
             /* Now check whether the record is for internal or external 
             * interfaces. 
             * If the record is for internal interfaces then send it to all 
             * those applications who has subscribed for the internal 
             * interfaces.
             * If the record is for external interfaces then send it to all 
             * those applications who has subscribed for the external 
             * interfaces. */
             if (subr_info->info.subscr_scope == NCS_IFSV_SUBSCR_ALL)
                 ifa_app_send(cb, &subr_info->info, ifa_evt, evt->type, TRUE);
             else
             {
               if (subr_info->info.subscr_scope == ifa_evt->subscr_scope)
                   ifa_app_send(cb, &subr_info->info, ifa_evt, evt->type, TRUE);
             }
            list_node = m_NCS_DBLIST_FIND_NEXT(list_node);
         }
      }
   }
#if(NCS_IFSV_IPXS == 1)  
   else if(evt->type == IFSV_IPXS_EVT)
   {
      rc = ifa_ipxs_evt_process(cb, evt);
   }
#endif
   ifsv_evt_ptrs_free(evt, evt->type);
   m_MMGR_FREE_IFSV_EVT(evt);

  return rc;
}


/*****************************************************************************
  PROCEDURE NAME: ifa_mds_enc
  DESCRIPTION   : This function encodes an events sent from IFA.
  ARGUMENTS:
      cb        : IFA control Block.
      enc_info  : Info for encoding

  RETURNS       : NCSCC_RC_SUCCESS/Error Code.
      
  NOTES:
*****************************************************************************/
uns32 ifa_mds_enc(IFA_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info)
{

   EDU_ERR   ederror = 0;   
   uns32     rc = NCSCC_RC_SUCCESS;

   enc_info->o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(enc_info->i_rem_svc_pvt_ver,
                               IFA_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT,
                               IFA_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT,
                               IFA_WRT_IFND_MSG_FMT_ARRAY);
   if(0 == enc_info->o_msg_fmt_ver)
   {
    m_IFA_LOG_EVT_LL(IFSV_LOG_IFA_EVT_INFO,\
         "IFSV_LOG_MDS_ENC_FAILURE. enc_info->i_to_svc_id,\
          enc_info->i_rem_svc_pvt_ver",\
          enc_info->i_to_svc_id,enc_info->i_rem_svc_pvt_ver);
    return NCSCC_RC_FAILURE;
   }

   rc = m_NCS_EDU_EXEC(&cb->edu_hdl, ifsv_edp_ifsv_evt, enc_info->io_uba, 
                       EDP_OP_TYPE_ENC, (IFSV_EVT*)enc_info->i_msg, &ederror);
   if(rc != NCSCC_RC_SUCCESS)
   {
     m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"m_NCS_EDU_EXEC() returned failure"," ");
      /* Free calls to be added here. */
      m_NCS_EDU_PRINT_ERROR_STRING(ederror);
      m_IFA_LOG_EVT_LL(IFSV_LOG_IFA_EVT_INFO,\
         "IFSV_LOG_MDS_ENC_FAILURE. enc_info->i_to_svc_id,\
          ((IFSV_EVT*)(enc_info->i_msg))->type",\
         enc_info->i_to_svc_id,((IFSV_EVT*)(enc_info->i_msg))->type);
      return rc;
   }
   m_IFA_LOG_EVT_LL(IFSV_LOG_IFA_EVT_INFO,\
         "IFSV_LOG_MDS_ENC_SUCCESS. enc_info->i_to_svc_id,\
         ((IFSV_EVT*)(enc_info->i_msg))->type",\
         enc_info->i_to_svc_id,((IFSV_EVT*)(enc_info->i_msg))->type);
   return rc;

}


/*****************************************************************************

  PROCEDURE NAME:   ifa_mds_dec

  DESCRIPTION   :
  This function decodes a buffer containing an IFA events that was received 
  from off card.

  ARGUMENTS:        
      cb        : IFA control Block.
      dec_info  : Info for decoding

  RETURNS       : NCSCC_RC_SUCCESS/Error Code.

  NOTES:

*****************************************************************************/
uns32 ifa_mds_dec (IFA_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info)
{
   EDU_ERR   ederror = 0;
   IFSV_EVT  *ifsv_evt;
   uns32     rc = NCSCC_RC_SUCCESS;

   if( 0 == m_NCS_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                                      IFA_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT,
                                      IFA_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT,
                                      IFA_WRT_IFND_MSG_FMT_ARRAY))
   {
      m_IFA_LOG_EVT_LL(IFSV_LOG_IFA_EVT_INFO,\
         "IFSV_LOG_MDS_DEC_FAILURE. dec_info->i_fr_svc_id,\
         dec_info->i_msg_fmt_ver",\
         dec_info->i_fr_svc_id,dec_info->i_msg_fmt_ver);
      return NCSCC_RC_FAILURE;
   }

   ifsv_evt        = m_MMGR_ALLOC_IFSV_EVT;
   if(NULL == ifsv_evt)
   {
     m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
     return NCSCC_RC_FAILURE;
   }
   dec_info->o_msg = (NCSCONTEXT)ifsv_evt;

   rc = m_NCS_EDU_EXEC(&cb->edu_hdl, ifsv_edp_ifsv_evt, dec_info->io_uba, 
                       EDP_OP_TYPE_DEC, (IFSV_EVT**)&dec_info->o_msg, &ederror);
   if(rc != NCSCC_RC_SUCCESS)
   {
     m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"m_NCS_EDU_EXEC() returned failure"," ");
      m_MMGR_FREE_IFSV_EVT(ifsv_evt);
      m_NCS_EDU_PRINT_ERROR_STRING(ederror);
      m_IFA_LOG_EVT_LL(IFSV_LOG_IFA_EVT_INFO,\
         "IFSV_LOG_MDS_DEC_FAILURE. dec_info->i_fr_svc_id,\
         ifsv_evt->type",\
         dec_info->i_fr_svc_id,ifsv_evt->type);
      return rc;
   }
     m_IFA_LOG_EVT_LL(IFSV_LOG_IFA_EVT_INFO,\
         "IFSV_LOG_MDS_DEC_SUCCESS. dec_info->i_fr_svc_id,\
         ifsv_evt->type",\
         dec_info->i_fr_svc_id,ifsv_evt->type);
   ifsv_evt->priority = NCS_IPC_PRIORITY_NORMAL;
   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   ifa_mds_cpy

  DESCRIPTION   :
  This Function copies the MDS message.

  ARGUMENTS:        
      cb        : IFA control Block.
      dec_info  : Info for decoding

  RETURNS       : NCSCC_RC_SUCCESS/Error Code.

  NOTES:

*****************************************************************************/
uns32 ifa_mds_cpy (IFA_CB *cb, MDS_CALLBACK_COPY_INFO *cpy_info)
{
   uns8*  stream;
   IFSV_EVT *evt;
   
   USE (cpy_info->i_last);

   /* cpy_info->o_msg_fmt_ver is not filled as we assume ifa and ifnd will not be in same process and this callback will not be called */
   evt = m_MMGR_ALLOC_IFSV_EVT;

   if(!evt)
   {
     m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
     return NCSCC_RC_FAILURE;
   }
   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));

   stream = (uns8*)evt;

   m_NCS_OS_STRCPY((char*)stream,(char*)cpy_info->i_msg);
   cpy_info->o_cpy = (void*)stream;
   return NCSCC_RC_SUCCESS;
}

