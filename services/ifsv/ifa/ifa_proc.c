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
FILE NAME: IFA_PROC.C
DESCRIPTION: Interface Agent Routines.
******************************************************************************/

#include "ifa.h"

static uns32 ifa_ifsubr_que_list_comp (uns8 *key1, uns8 *key2);
static void ifa_cpy_attr(NCS_IFSV_SUBSCR *subr, NCS_IFSV_INTF_REC *src, NCS_IFSV_INTF_REC *dest);

uns32 gl_ifa_cb_hdl;

/*****************************************************************************
  PROCEDURE NAME    :   ifsv_ifa_subscribe
  DESCRIPTION       :   Suscribe the IfSv requests received from Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.
                        NCS_IFSV_SUBSCR *subr : Pointer to Subscription Info.

  RETURNS           :   NCSCC_RC_SUCCESS/Error Code.
  NOTES             :   None.
*****************************************************************************/
uns32 ifsv_ifa_subscribe (IFA_CB *ifa_cb, NCS_IFSV_SUBSCR *i_subr)
{
   IFSV_SUBSCR_INFO        *subr = NULL;
   uns32                   rc = NCSCC_RC_SUCCESS;
   IFSV_EVT                *evt=0;

   if((i_subr->subscr_scope != NCS_IFSV_SUBSCR_EXT) && 
      (i_subr->subscr_scope != NCS_IFSV_SUBSCR_INT) &&
      (i_subr->subscr_scope != NCS_IFSV_SUBSCR_ALL))
   {
      return NCSCC_RC_FAILURE;
   } 
   /* Check for the subscription of events. */
   if((((i_subr->i_sevts & NCS_IFSV_ADD_IFACE) == NCS_IFSV_ADD_IFACE) ||
      ((i_subr->i_sevts & NCS_IFSV_UPD_IFACE) == NCS_IFSV_UPD_IFACE) ||
      ((i_subr->i_sevts & NCS_IFSV_RMV_IFACE) == NCS_IFSV_RMV_IFACE)) &&
      (((i_subr->i_smap & NCS_IFSV_IAM_MTU) == NCS_IFSV_IAM_MTU) ||
      ((i_subr->i_smap & NCS_IFSV_IAM_IFSPEED) == NCS_IFSV_IAM_IFSPEED) ||
      ((i_subr->i_smap & NCS_IFSV_IAM_PHYADDR) == NCS_IFSV_IAM_PHYADDR) ||
      ((i_subr->i_smap & NCS_IFSV_IAM_ADMSTATE) == NCS_IFSV_IAM_ADMSTATE) ||
      ((i_subr->i_smap & NCS_IFSV_IAM_OPRSTATE) == NCS_IFSV_IAM_OPRSTATE) ||
      ((i_subr->i_smap & NCS_IFSV_IAM_NAME) == NCS_IFSV_IAM_NAME) ||
      ((i_subr->i_smap & NCS_IFSV_IAM_DESCR) == NCS_IFSV_IAM_DESCR) ||
      ((i_subr->i_smap & NCS_IFSV_IAM_LAST_CHNG) == NCS_IFSV_IAM_LAST_CHNG) ||
#if(NCS_IFSV_BOND_INTF == 1)
      ((i_subr->i_smap & NCS_IFSV_IAM_CHNG_MASTER ) == NCS_IFSV_IAM_CHNG_MASTER) ||
#endif
      ((i_subr->i_smap & NCS_IFSV_IAM_SVDEST) == NCS_IFSV_IAM_SVDEST)))
   {
      /* If either of these three is set, we assume to be correct subs value.*/
      rc = NCSCC_RC_SUCCESS;
   }
   else
   {
      /* If none of these is set true, then it has incorrect subs values. */
      rc = NCSCC_RC_FAILURE;
   }
   
   if(rc != NCSCC_RC_SUCCESS)
      {
       m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifa_subscribe returns failure due to incorrect subs values"," ");
       return rc;
      }

   /*Now check for the subscription call back function. It should not be NULL*/
   if(i_subr->i_ifsv_cb == NULL)
      return NCSCC_RC_FAILURE;
   
   if(i_subr->i_usrhdl == (uns64)(long)NULL)
      return NCSCC_RC_FAILURE;
   
   /* Allocate the memory for storing subscription info */
   if((subr = m_MMGR_ALLOC_IFSV_SUBSCR_INFO) == NULL)
   {
      m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
      return NCSCC_RC_OUT_OF_MEM;
   }

   /* Mem Set the Subr */
   m_NCS_OS_MEMSET(subr, 0, sizeof(IFSV_SUBSCR_INFO));

   /* Get the Handle from Handle Manager for this subscription Record*/
   i_subr->o_subr_hdl = ncshm_create_hdl(ifa_cb->hm_pid, NCS_SERVICE_ID_IFA,
                              (NCSCONTEXT) subr);

   /* Populate the Record with received subscription info */
   subr->info = *i_subr;

   /* Add the record to end of the linked list  */
   subr->lnode.key = (uns8*) subr;
   ncs_db_link_list_add(&ifa_cb->subr_list, &subr->lnode);

   /* If IFND is not UP return from Here */
   if(!ifa_cb->is_ifnd_up)
   {
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"IfND is not up. Subs Success",\
                      IFSV_COMP_IFA);
      return NCSCC_RC_SUCCESS;
   }

   if(((i_subr->i_sevts & NCS_IFSV_ADD_IFACE) == NCS_IFSV_ADD_IFACE) ||
      ((i_subr->i_sevts & NCS_IFSV_UPD_IFACE) == NCS_IFSV_UPD_IFACE))
   {
      /* We need tell the subscriber what we already know...
         Send the event to IFND, IFND will send all the interface 
         records that it already has */

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
      evt->info.ifnd_evt.info.if_get.my_dest   = ifa_cb->my_dest;
      evt->info.ifnd_evt.info.if_get.usr_hdl   = i_subr->o_subr_hdl;
      /* Here I want all the records, I am not filling the if_get.ifkey */
      
      /* Send the Event to IFND */
      rc = ifsv_mds_normal_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA, evt,
             ifa_cb->ifnd_dest, NCSMDS_SVC_ID_IFND);
   }

   /* Free the Event */
   if(evt)
      m_MMGR_FREE_IFSV_EVT(evt);

   if(rc == NCSCC_RC_SUCCESS)
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"Subscription Successful.",\
                      IFSV_COMP_IFA);
      return NCSCC_RC_SUCCESS;
   return rc;
}


/*****************************************************************************
  PROCEDURE NAME    :   ifsv_ifa_unsubscribe
  DESCRIPTION       :   Process the IfSv Unsuscribe requests received from 
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.
                        NCS_IFSV_UNSUBSCR *subr : Pointer to Unsubscr Info.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
uns32 ifsv_ifa_unsubscribe (IFA_CB *ifa_cb, NCS_IFSV_UNSUBSCR *i_unsubr)
{
   IFSV_SUBSCR_INFO        *subr = NULL;

   /* Provide the Handle and Get the Subr Info */
   subr = (IFSV_SUBSCR_INFO *) ncshm_take_hdl(NCS_SERVICE_ID_IFA, 
                                                   i_unsubr->i_subr_hdl);
   if(subr == NULL)
   {
      m_IFD_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,IFSV_COMP_IFA);
      return NCSCC_RC_FAILURE;
   }
   /* Remove the info from Doubly linked List */
   ncs_db_link_list_remove(&ifa_cb->subr_list, (uns8 *)subr);

   /* Usage of USR_HDL for this struct has finished, return the hdl */
   ncshm_give_hdl(i_unsubr->i_subr_hdl);

   /* Destroy the hdl for this subscription info */
   ncshm_destroy_hdl(NCS_SERVICE_ID_IFA, i_unsubr->i_subr_hdl);

   /* Free the Subscription info */
   m_MMGR_FREE_IFSV_SUBSCR_INFO(subr);

   m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"Un-Subscription Successful.",\
                      IFSV_COMP_IFA);

   return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
  PROCEDURE NAME    :   ifsv_ifa_ifrec_get
  DESCRIPTION       :   Process the Interface Record Get request received from
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.
                        NCS_IFSV_UNSUBSCR *subr : Pointer to Unsubscr Info.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
uns32 ifsv_ifa_ifrec_get (IFA_CB *ifa_cb, NCS_IFSV_IFREC_GET *i_ifget, 
                            NCS_IFSV_IFGET_RSP *o_ifrec)
{
   IFSV_EVT_INTF_INFO_GET  if_get;
   uns32                   rc = NCSCC_RC_SUCCESS;
   IFSV_EVT                *evt = NULL, *o_evt = NULL;
   IFSV_SUBSCR_INFO *subr_info = NULL;
   IFA_EVT          *ifa_evt;

   if_get.ifkey         = i_ifget->i_key;
   if_get.my_dest       = ifa_cb->my_dest;

   if_get.usr_hdl       = i_ifget->i_subr_hdl;
   
   /* If IFND is not UP return from Here */
   if(!ifa_cb->is_ifnd_up)
   {
    m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"IfND is not up. Req Not processed",\
                      IFSV_COMP_IFA);
     return NCS_IFSV_IFND_DOWN_ERROR;
   }
 
   if((i_ifget->i_info_type == NCS_IFSV_IF_INFO) || 
      (i_ifget->i_info_type == NCS_IFSV_IFSTATS_INFO) ||
      (i_ifget->i_info_type == NCS_IFSV_BIND_GET_LOCAL_INTF))   
   {
      rc = NCSCC_RC_SUCCESS;
   }
   else
   {
      rc = NCSCC_RC_FAILURE; 
   }
   
   if(rc != NCSCC_RC_SUCCESS)
     {
      m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifa_rec_get() returns failure due to incorrect i_info_type"," ");
      return rc;
     }

   evt = m_MMGR_ALLOC_IFSV_EVT;

   if (evt == IFSV_NULL)
   {
     m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
     return (NCSCC_RC_FAILURE);
   }   

   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));

   /* Send the Event to IFND */
   if(i_ifget->i_rsp_type == NCS_IFSV_GET_RESP_ASYNC)
   {
      evt->type = IFND_EVT_INTF_INFO_GET;

      evt->info.ifnd_evt.info.if_get.get_type  = IFSV_INTF_GET_ONE;
      evt->info.ifnd_evt.info.if_get.my_svc_id = NCSMDS_SVC_ID_IFA;
      evt->info.ifnd_evt.info.if_get.my_dest   = ifa_cb->my_dest;
      evt->info.ifnd_evt.info.if_get.usr_hdl   = i_ifget->i_subr_hdl;
      evt->info.ifnd_evt.info.if_get.ifkey     = i_ifget->i_key;
      evt->info.ifnd_evt.info.if_get.info_type = i_ifget->i_info_type;

      /* Send the Event to IFND */
      rc = ifsv_mds_normal_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA, evt,
             ifa_cb->ifnd_dest, NCSMDS_SVC_ID_IFND);
      if(rc != NCSCC_RC_SUCCESS)
      {
         m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_mds_normal_send() returns failure"," ");
         m_MMGR_FREE_IFSV_EVT(evt);
         /* TBD:RSR Log the error*/
         return NCSCC_RC_FAILURE;
      }

   }
      else if(i_ifget->i_rsp_type == NCS_IFSV_GET_RESP_SYNC)/* Sync */ 
   {
      if(i_ifget->i_info_type == NCS_IFSV_IFSTATS_INFO)
      {
        m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                        "Stats Req cann't be processed in sync.",\
                        IFSV_COMP_IFA);
        rc = NCSCC_RC_FAILURE;
        goto free_mem;
      }
      
      evt->type = IFND_EVT_INTF_INFO_GET;

      evt->info.ifnd_evt.info.if_get.get_type  = IFSV_INTF_GET_ONE;
      evt->info.ifnd_evt.info.if_get.my_svc_id = NCSMDS_SVC_ID_IFA;
      evt->info.ifnd_evt.info.if_get.my_dest   = ifa_cb->my_dest;
      evt->info.ifnd_evt.info.if_get.usr_hdl   = i_ifget->i_subr_hdl;
      evt->info.ifnd_evt.info.if_get.ifkey     = i_ifget->i_key;
      evt->info.ifnd_evt.info.if_get.info_type = i_ifget->i_info_type;

      /* Send the Event to IFND */
      rc = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA,
                      NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, evt, &o_evt,
                      IFA_MDS_SYNC_TIME);
      if(rc == NCSCC_RC_SUCCESS)
      {
       if(o_evt->error == NCS_IFSV_NO_ERROR)
       {
        ifa_evt = &o_evt->info.ifa_evt;
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
               o_ifrec->error = ifa_evt->info.ifget_rsp.error; 
               o_ifrec->if_rec.info_type =
                           ifa_evt->info.ifget_rsp.if_rec.info_type;
               o_ifrec->if_rec.if_index =
                           ifa_evt->info.ifget_rsp.if_rec.if_index;
               o_ifrec->if_rec.spt_info =
                           ifa_evt->info.ifget_rsp.if_rec.spt_info;                          }
             else
             {
              /* We have the record, so we have information about the record
                 whether it belongs to EXT or INT interface. */
              if ((subr_info->info.subscr_scope ==NCS_IFSV_SUBSCR_ALL) || (subr_info->info.subscr_scope == ifa_evt->subscr_scope))
              {
                o_ifrec->error = ifa_evt->info.ifget_rsp.error;
                o_ifrec->if_rec = ifa_evt->info.ifget_rsp.if_rec;
                rc =ifsv_intf_info_svd_list_cpy(&ifa_evt->info.ifget_rsp.if_rec.
                     if_info, &o_ifrec->if_rec.if_info, 
                     IFSV_MALLOC_FOR_INTERNAL_USE);
              } 
              else
              {
                 /* If it doesn't matches, then also we have to send the
                    application that NO RECORD was found. */
                 o_ifrec->error = ifa_evt->info.ifget_rsp.error;
                 o_ifrec->if_rec.info_type =
                           ifa_evt->info.ifget_rsp.if_rec.info_type;
                 o_ifrec->if_rec.if_index =
                           ifa_evt->info.ifget_rsp.if_rec.if_index;
                 o_ifrec->if_rec.spt_info =
                           ifa_evt->info.ifget_rsp.if_rec.spt_info;
               }
             }

         } /* subr_info  */
         else
            rc = NCSCC_RC_FAILURE;
         ncshm_give_hdl((uns32)(long)ifa_evt->usrhdl);
       } /* evt->info.ifa_evt.usrhdl */
       else
         rc = NCSCC_RC_FAILURE;   
      } /* if(o_evt->error == NCS_IFSV_NO_ERROR) */
      else
         rc = NCSCC_RC_FAILURE;

      } /* rc == NCSCC_RC_SUCCESS  */

   }
   else
     rc = NCSCC_RC_FAILURE;

free_mem:
   m_MMGR_FREE_IFSV_EVT(evt);

   if(o_evt)
      m_MMGR_FREE_IFSV_EVT(o_evt);

   return rc;
}

/*****************************************************************************
  PROCEDURE NAME    :   ifsv_ifa_ifrec_add
  DESCRIPTION       :   Process the Interface Record add request received from
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
uns32 ifsv_ifa_ifrec_add (IFA_CB *ifa_cb,  NCS_IFSV_INTF_REC *i_ifrec)
{
   IFSV_EVT    *evt, *o_evt=0;
   uns32       rc = NCSCC_RC_FAILURE;

   /* Check whether the interface scope field is filled or not. If not then we 
    * will fill it to NCS_IFSV_EXT_IF. */   
   if(0 == i_ifrec->spt_info.subscr_scope)
     i_ifrec->spt_info.subscr_scope = NCS_IFSV_SUBSCR_EXT;
  
   if((i_ifrec->spt_info.subscr_scope != NCS_IFSV_SUBSCR_EXT) && 
      (i_ifrec->spt_info.subscr_scope != NCS_IFSV_SUBSCR_INT))
        return NCSCC_RC_FAILURE;

   /* If IFND is not UP return from Here */
   if(!ifa_cb->is_ifnd_up)
   {
    m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"IfND is not up. Req Not processed",\
                      IFSV_COMP_IFA);
     return NCS_IFSV_IFND_DOWN_ERROR;
   }
   evt = m_MMGR_ALLOC_IFSV_EVT;

   if (evt == IFSV_NULL)
   {
      m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
      return rc;
   }   

   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));

   evt->type = IFND_EVT_INTF_CREATE;


   evt->info.ifnd_evt.info.intf_create.intf_data.spt_info = i_ifrec->spt_info;
   evt->info.ifnd_evt.info.intf_create.intf_data.if_info = i_ifrec->if_info;
   evt->info.ifnd_evt.info.intf_create.if_attr = i_ifrec->if_info.if_am;
   evt->info.ifnd_evt.info.intf_create.evt_orig = NCS_IFSV_EVT_ORIGN_IFA;


   evt->info.ifnd_evt.info.intf_create.intf_data.originator = NCS_IFSV_EVT_ORIGN_IFA;
   evt->info.ifnd_evt.info.intf_create.intf_data.originator_mds_destination = ifa_cb->my_dest;
   evt->info.ifnd_evt.info.intf_create.intf_data.current_owner = NCS_IFSV_OWNER_IFND;

   /* Send the Event to IFND */
   rc = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA,
                 NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, evt, &o_evt, 
                 IFA_MDS_SYNC_TIME);
   if(rc == NCSCC_RC_SUCCESS)
   {
     if(o_evt == NULL)
       rc = NCSCC_RC_FAILURE;
     else
     {
       if(o_evt->error != NCS_IFSV_NO_ERROR)
         rc = NCSCC_RC_FAILURE;
       if(o_evt->error == NCS_IFSV_SYNC_TIMEOUT)
         rc = NCSCC_RC_REQ_TIMOUT;
       if(o_evt->error == NCS_IFSV_INTF_MOD_ERROR)
         rc = NCSCC_RC_SUCCESS;

     }
      m_MMGR_FREE_IFSV_EVT(evt);
   }
   else
   {
     m_MMGR_FREE_IFSV_EVT(evt);
     
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                     "Sync Send Error in rec add. Error is : ",\
                     rc);
   }

   if(rc == NCSCC_RC_SUCCESS)
   {
    i_ifrec->if_index = o_evt->info.ifa_evt.info.if_add_rsp_idx;
    m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"Rec added with index : ",\
                   i_ifrec->if_index);
   }

    if(o_evt)
       m_MMGR_FREE_IFSV_EVT(o_evt);

   return rc;
}

/*****************************************************************************
  PROCEDURE NAME    :   ifsv_ifa_ifrec_del
  DESCRIPTION       :   Process the Interface Record Del request received from
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
uns32 ifsv_ifa_ifrec_del (IFA_CB *ifa_cb,  NCS_IFSV_SPT *i_ifdel)
{
   IFSV_EVT    *evt, *o_evt = NULL;
   uns32       rc = NCSCC_RC_FAILURE;

   /* Check whether the interface scope field is filled or not. If not then we 
    * will fill it to NCS_IFSV_EXT_IF. */   
   if (0 == i_ifdel->subscr_scope)
     i_ifdel->subscr_scope = NCS_IFSV_SUBSCR_EXT;
    
   if((i_ifdel->subscr_scope != NCS_IFSV_SUBSCR_EXT) && 
      (i_ifdel->subscr_scope != NCS_IFSV_SUBSCR_INT))
        return NCSCC_RC_FAILURE;

   /* If IFND is not UP return from Here */
   if(!ifa_cb->is_ifnd_up)
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"IfND is not up. Req Not processed",\
                      IFSV_COMP_IFA);
     return NCS_IFSV_IFND_DOWN_ERROR;
   }
   evt = m_MMGR_ALLOC_IFSV_EVT;

   if (evt == IFSV_NULL)
   {
      m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
      return rc;
   }   

   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));

   evt->type = IFND_EVT_INTF_DESTROY;

   evt->info.ifnd_evt.info.intf_destroy.spt_type = *i_ifdel;
   evt->info.ifnd_evt.info.intf_destroy.orign    = NCS_IFSV_EVT_ORIGN_IFA;
   evt->info.ifnd_evt.info.intf_destroy.own_dest = ifa_cb->my_dest;

   /* Send the Event to IFND */
   rc = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA,
                  NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, evt, &o_evt,
                  IFA_MDS_SYNC_TIME);

   if(rc != NCSCC_RC_SUCCESS)
   {
     m_IFA_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"unable to delete record. MDS Sync send failed with value :",rc);
   
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                     "Sync Send Error in rec del. Error is : ",\
                      rc);
      m_MMGR_FREE_IFSV_EVT(evt);
      /* TBD:RSR Log the error*/
      return rc;
   }
   if(rc == NCSCC_RC_SUCCESS)
   {
     m_MMGR_FREE_IFSV_EVT(evt);
     if(o_evt == NULL)
       rc = NCSCC_RC_FAILURE;
     else
     {
       if(o_evt->error != NCS_IFSV_NO_ERROR)
       {
         rc = NCSCC_RC_FAILURE;
        if(o_evt)
         m_MMGR_FREE_IFSV_EVT(o_evt);
        return rc;
       }
     }
   }
   m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"Rec Del Success", 0);

   if(o_evt)
       m_MMGR_FREE_IFSV_EVT(o_evt);

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  PROCEDURE NAME    :   ifsv_ifa_svcd_upd
  DESCRIPTION       :   Process the SVC update request received from
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
uns32 ifsv_ifa_svcd_upd (IFA_CB *ifa_cb,  NCS_IFSV_SVC_DEST_UPD *i_svcd)
{
   IFSV_EVT    evt, *o_evt = NULL;
   uns32       rc = NCSCC_RC_FAILURE;

   /* If IFND is not UP return from Here */
   if(!ifa_cb->is_ifnd_up)
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"IfND is not up. Req Not processed",\
                      IFSV_COMP_IFA);
     return NCS_IFSV_IFND_DOWN_ERROR;
   }
   m_NCS_MEMSET(&evt, 0, sizeof(IFSV_EVT));

   evt.type = IFND_EVT_SVCD_UPD_FROM_IFA;

   evt.info.ifnd_evt.info.svcd = *i_svcd;

   /* Send the sync req to IFND */
   rc = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA,
                 NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, &evt, &o_evt, 
                 IFA_MDS_SYNC_TIME);

   if(rc == NCSCC_RC_SUCCESS)
   {
/*
      if(evt != IFSV_NULL)
         m_MMGR_FREE_IFSV_EVT(&evt);
*/
      
     if(o_evt == NULL)
       rc = NCSCC_RC_FAILURE;
     else
     {
       if(o_evt->error != NCS_IFSV_NO_ERROR)
         rc = NCSCC_RC_FAILURE;
     }
   }
   else
   {
/*
      if(&evt)
         m_MMGR_FREE_IFSV_EVT(&evt);
*/
      
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                     "Sync Send Error in SVCD Upd. Error is : ",\
                      rc);
     return rc;
   }

   if(rc == NCSCC_RC_SUCCESS)
   {
    m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                   "SVCD Upd Success. The index is ",\
                   i_svcd->i_ifindex);
   }
   if(o_evt)
       m_MMGR_FREE_IFSV_EVT(o_evt);
   return rc;
} /* The function ifsv_ifa_svcd_upd() ends here. */

/*****************************************************************************
  PROCEDURE NAME    :   ifsv_ifa_svcd_get
  DESCRIPTION       :   Process the SVC get request received from
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
uns32 ifsv_ifa_svcd_get (IFA_CB *ifa_cb,  NCS_IFSV_SVC_DEST_GET *i_svcd)
{
   IFSV_EVT    evt, *o_evt=0;
   uns32       rc = NCSCC_RC_FAILURE;
   
   /* If IFND is not UP return from Here */
   if(!ifa_cb->is_ifnd_up)
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"IfND is not up. Req Not processed",\
                      IFSV_COMP_IFA);
     return NCS_IFSV_IFND_DOWN_ERROR;
   }
   m_NCS_MEMSET(&evt, 0, sizeof(IFSV_EVT));

   evt.type = IFND_EVT_SVCD_GET;

   evt.info.ifnd_evt.info.svcd_get = *i_svcd;

   /* Send the Event to IFND */
   rc = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA,
                 NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, &evt, &o_evt, 
                 IFA_MDS_SYNC_TIME);

   if(rc == NCSCC_RC_SUCCESS)
   {
     if(o_evt == NULL)
       rc = NCSCC_RC_FAILURE;
     else
     {
       if(o_evt->error != NCS_IFSV_NO_ERROR)
         rc = NCSCC_RC_FAILURE;
     }
   }
   else
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                     "Sync Send Error in SVCD Get. Error is : ",\
                      rc);
     return rc;
   }


   if(rc == NCSCC_RC_SUCCESS)
   {
    m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                   "SVCD Get Success. The index is ",\
                   i_svcd->i_ifindex);
    i_svcd->o_dest = o_evt->info.ifa_evt.info.svcd_rsp.o_dest;
    i_svcd->o_answer = o_evt->info.ifa_evt.info.svcd_rsp.o_answer;
    
   }

   if(o_evt)
      m_MMGR_FREE_IFSV_EVT(o_evt);
   return rc;
}

/****************************************************************************
 * Name          : ifa_ifsubr_que_list_comp
 *
 * Description   : Compares the subscription info pointers 
 *
 *****************************************************************************/
static uns32
ifa_ifsubr_que_list_comp (uns8 *key1, uns8 *key2)
{   
   if ((key1 == NULL) || (key2 == NULL))
   {
      return (0xffff);
   }   
   return (m_NCS_MEMCMP(key1, key2, sizeof(uns32)));
}


/****************************************************************************
 * Name          : ifa_lib_init
 *
 * Description   : This is the function which initalize the IFA library.
 *                 
 *
 * Arguments     : pointer to NCS_LIB_CREATE
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifa_lib_init (NCS_LIB_CREATE *create)
{

   IFA_CB   *cb = NULL;
   uns32  rc;
   
   /* register with the Flex log service */

   if ((rc = ifsv_flx_log_reg(IFSV_COMP_IFA)) != NCSCC_RC_SUCCESS)
   {
      m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_flx_log_reg() returned failure"," ");
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,"ifsv_flx_log_reg failure",\
                      IFSV_COMP_IFA);
      return(NCSCC_RC_FAILURE);
   }

   /* Malloc the Control Block for IFA */
   cb = m_MMGR_ALLOC_IFA_CB;

   if(cb == NULL)
   {
      m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_MEMSET(cb, 0, sizeof(IFSV_CB));

   cb->hm_pid = NCS_HM_POOL_ID_COMMON;

   /* Initialize the subscription info Linked List */
   cb->subr_list.order      = NCS_DBLIST_ANY_ORDER;
   cb->subr_list.cmp_cookie = ifa_ifsubr_que_list_comp; 

   /* Handle Manager Init Routines */
   if((cb->cb_hdl = ncshm_create_hdl(cb->hm_pid, 
               NCS_SERVICE_ID_IFA, (NCSCONTEXT)cb)) == 0)
   {
      m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_HDL_MGR_FAIL,IFSV_COMP_IFA);
      rc = NCSCC_RC_FAILURE;
      goto ifa_hdl_fail;            
   }                  

   m_NCS_EDU_HDL_INIT(&cb->edu_hdl);
   
   /* Does Agent Code need to Logging TBD:RSR */
   
   /* MDSv init */
   if ((rc = ifa_mds_init(cb)) != NCSCC_RC_SUCCESS)
   {
      m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifa_mds_init() returned failure"," ");
      m_IFA_LOG_API_L(IFSV_LOG_MDS_INIT_FAILURE,IFSV_COMP_IFA);
      goto ifa_mds_init_fail; 
   }
#if(NCS_IFSV_IPXS == 1)
   ipxs_agent_lib_init(cb);
#endif

#if (NCS_VIP == 1)
   rc = ncs_vip_ip_lib_init(cb);
   if (rc == NCSCC_RC_FAILURE)
   {
      goto ifa_vip_init_fail;
   }
#endif  

   m_IFA_SET_CB_HDL(cb->cb_hdl);
   m_IFA_LOG_API_L(IFSV_LOG_SE_INIT_DONE,IFSV_COMP_IFA);
   return rc;
  
#if (NCS_VIP == 1)
ifa_vip_init_fail: 
#endif  
ifa_mds_init_fail:
   /* Destroy the CB Handle */
   ncshm_destroy_hdl(NCS_SERVICE_ID_IFA, cb->cb_hdl);

ifa_hdl_fail:
   /* Free the Control Block */
   m_MMGR_FREE_IFA_CB(cb);

   return rc;
}

/****************************************************************************
 * Name          : ifa_lib_destroy
 *
 * Description   : This is the function which destroy the IFA libarary.
 *                
 *
 * Arguments     : pointer to NCS_LIB_DESTROY
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifa_lib_destroy (NCS_LIB_DESTROY *destroy)
{ 
   uns32    ifa_hdl;
   IFA_CB   *cb = NULL;

   /* Get the Handle Manager Handle */
   ifa_hdl = m_IFA_GET_CB_HDL();

   /* Take the IFA_CB */
   if ((cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFA, ifa_hdl))
      == NULL)
   {
      m_IFA_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,IFSV_COMP_IFA);
      return (NCSCC_RC_FAILURE);
   }
   
   /* Unsubscribe with MDS */
   ifa_mds_shut(cb);

   /* TBD:RSR Free the subscription linked list */

   /* Give the Handle */
   ncshm_give_hdl(ifa_hdl);

   /* Destroy the Handle */
   ncshm_destroy_hdl(NCS_SERVICE_ID_IFA, cb->cb_hdl);

   m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);

   /* Free the CB */
   m_MMGR_FREE_IFA_CB (cb);

   m_IFA_LOG_API_L(IFSV_LOG_SE_DESTROY_DONE,IFSV_COMP_IFA);

   return (NCSCC_RC_SUCCESS);
}

static void ifa_cpy_attr(NCS_IFSV_SUBSCR *subr, NCS_IFSV_INTF_REC *src, NCS_IFSV_INTF_REC *dest)
{
   dest->if_index = src->if_index;
   dest->spt_info = src->spt_info;

   if(src->info_type == NCS_IFSV_IF_INFO)
   {
      if(m_IFSV_IS_MTU_ATTR_SET(src->if_info.if_am) && 
                       m_IFSV_IS_MTU_ATTR_SET(subr->i_smap))
      {
         m_IFSV_MTU_ATTR_SET(dest->if_info.if_am);
         dest->if_info.mtu = src->if_info.mtu;
      }

      if(m_IFSV_IS_IFSPEED_ATTR_SET(src->if_info.if_am) && 
                       m_IFSV_IS_IFSPEED_ATTR_SET(subr->i_smap))
      {
         m_IFSV_IFSPEED_ATTR_SET(dest->if_info.if_am);
         dest->if_info.if_speed = src->if_info.if_speed;
      }

      if(m_IFSV_IS_PHYADDR_ATTR_SET(src->if_info.if_am) && 
                       m_IFSV_IS_PHYADDR_ATTR_SET(subr->i_smap))
      {
         m_IFSV_PHYADDR_ATTR_SET(dest->if_info.if_am);

         m_NCS_MEMCPY(dest->if_info.phy_addr, src->if_info.phy_addr, 
            sizeof(src->if_info.phy_addr));
      }

      if(m_IFSV_IS_ADMSTATE_ATTR_SET(src->if_info.if_am) && 
                       m_IFSV_IS_ADMSTATE_ATTR_SET(subr->i_smap))
      {
         m_IFSV_ADMSTATE_ATTR_SET(dest->if_info.if_am);
         dest->if_info.admin_state = src->if_info.admin_state;
      }

      if(m_IFSV_IS_OPRSTATE_ATTR_SET(src->if_info.if_am) && 
                       m_IFSV_IS_OPRSTATE_ATTR_SET(subr->i_smap))
      {
         m_IFSV_OPRSTATE_ATTR_SET(dest->if_info.if_am);
         dest->if_info.oper_state = src->if_info.oper_state;
      }

      if(m_IFSV_IS_NAME_ATTR_SET(src->if_info.if_am) && 
                      m_IFSV_IS_NAME_ATTR_SET(subr->i_smap))
      {
         m_IFSV_NAME_ATTR_SET(dest->if_info.if_am);
         m_NCS_MEMCPY(dest->if_info.if_name, src->if_info.if_name, 
            IFSV_IF_NAME_SIZE);
      }

      if(m_IFSV_IS_DESCR_ATTR_SET(src->if_info.if_am) && 
                       m_IFSV_IS_DESCR_ATTR_SET(subr->i_smap))
      {
         m_IFSV_DESCR_ATTR_SET(dest->if_info.if_am);
         m_NCS_MEMCPY(dest->if_info.if_descr, src->if_info.if_descr, 
            IFSV_IF_DESC_SIZE);
      }

      if(m_IFSV_IS_LAST_CHNG_ATTR_SET(src->if_info.if_am) && 
                       m_IFSV_IS_LAST_CHNG_ATTR_SET(subr->i_smap))
      {
         m_IFSV_LAST_CHNG_ATTR_SET(dest->if_info.if_am);
         dest->if_info.last_change = src->if_info.last_change;
      }
#if (NCS_IFSV_BOND_INTF == 1)
      if(m_IFSV_IS_MASTER_CHNG_ATTR_SET(src->if_info.if_am) && 
                       m_IFSV_IS_MASTER_CHNG_ATTR_SET(subr->i_smap))
      {
         m_IFSV_CHNG_MASTER_ATTR_SET(dest->if_info.if_am);
         dest->if_info.bind_master_ifindex = src->if_info.bind_master_ifindex;
         dest->if_info.bind_slave_ifindex = src->if_info.bind_slave_ifindex;
      }
#endif

      if(m_IFSV_IS_SVDEST_ATTR_SET(src->if_info.if_am) && 
                     m_IFSV_IS_SVDEST_ATTR_SET(subr->i_smap))
      {
         m_IFSV_SVDEST_ATTR_SET(dest->if_info.if_am);

         ifsv_intf_info_svd_list_cpy(&src->if_info, 
                  &dest->if_info, IFSV_MALLOC_FOR_INTERNAL_USE);
      }
   }
   else if(src->info_type == NCS_IFSV_IFSTATS_INFO)
   {
      dest->if_stats = src->if_stats;
   }

   return;
}


uns32 ifa_app_send(IFA_CB *cb, NCS_IFSV_SUBSCR *subr, 
                   IFA_EVT *evt, IFSV_EVT_TYPE evt_type, uns32 rec_found)
{
   NCS_IFSV_SVC_RSP  rsp;
   uns32 rc = NCSCC_RC_SUCCESS;


   m_NCS_MEMSET(&rsp, 0, sizeof(NCS_IFSV_SVC_RSP));

   switch(evt_type)
   {
   case IFA_EVT_INTF_CREATE:
      /* When Scope doesnot match then don't deliver to the appl. */
      if(rec_found != TRUE)
        return NCSCC_RC_FAILURE;

      if((subr->i_sevts & NCS_IFSV_ADD_IFACE) == NCS_IFSV_ADD_IFACE)
      {
         rsp.rsp_type = NCS_IFSV_IFREC_ADD_NTFY;
         rsp.usrhdl   = subr->i_usrhdl;
         ifa_cpy_attr(subr, &evt->info.if_add_upd, &rsp.info.ifadd_ntfy);
         m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                         "Rec Add info being sent to App. Index is ",\
                         evt->info.if_add_upd.if_index);
      }
      else
      {
         return rc;
      }
      break;

   case IFA_EVT_INTF_UPDATE:
      if((subr->i_sevts & NCS_IFSV_UPD_IFACE) == NCS_IFSV_UPD_IFACE)
      {
         rsp.rsp_type = NCS_IFSV_IFREC_UPD_NTFY;
         rsp.usrhdl   = subr->i_usrhdl;
         ifa_cpy_attr(subr, &evt->info.if_add_upd, &rsp.info.ifupd_ntfy);
         m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                         "Rec Upd info being sent to App. Index is ",\
                         evt->info.if_add_upd.if_index);
      }
      else
      {
         return rc;
      }
      break;

   case IFA_EVT_INTF_DESTROY:
      if((subr->i_sevts & NCS_IFSV_RMV_IFACE) == NCS_IFSV_RMV_IFACE)
      {
         rsp.rsp_type = NCS_IFSV_IFREC_DEL_NTFY;
         rsp.usrhdl   = subr->i_usrhdl;
         rsp.info.ifdel_ntfy = evt->info.if_del;
         m_IFA_LOG_EVT_L(IFSV_LOG_IFA_EVT_INFO,\
                         "Rec Destroy info being sent to App. Index is ",\
                         evt->info.if_del);
      }
      else
         return rc;
      break;
      
   case IFA_EVT_IFINFO_GET_RSP:
      rsp.rsp_type = NCS_IFSV_IFREC_GET_RSP;
      rsp.usrhdl   = subr->i_usrhdl;
      if(rec_found == TRUE)
      {
       /* We found the record succesfully. */
       rsp.info.ifget_rsp.error = evt->info.ifget_rsp.error;

       rsp.info.ifget_rsp.if_rec = evt->info.ifget_rsp.if_rec;
       rc = ifsv_intf_info_svd_list_cpy(&evt->info.ifget_rsp.if_rec.if_info, 
            &rsp.info.ifget_rsp.if_rec.if_info, IFSV_MALLOC_FOR_INTERNAL_USE);
      }
      else
      {
        /* Either we could not found or the applications is not authorized
           to take this information. */
        rsp.rsp_type = NCS_IFSV_IFREC_GET_RSP;
        rsp.usrhdl = subr->i_usrhdl;
        rsp.info.ifget_rsp.error = NCS_IFSV_IFGET_RSP_NO_REC;
        rsp.info.ifget_rsp.if_rec.info_type = 
                           evt->info.ifget_rsp.if_rec.info_type;
        rsp.info.ifget_rsp.if_rec.if_index = 
                           evt->info.ifget_rsp.if_rec.if_index;
        rsp.info.ifget_rsp.if_rec.spt_info = 
                           evt->info.ifget_rsp.if_rec.spt_info;
      }
      break;

   default:
      return NCSCC_RC_FAILURE;
      break;
   }

   subr->i_ifsv_cb(&rsp);

   return rc;

}


    

