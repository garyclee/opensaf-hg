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

#include "ifa.h"

static uns32 ipxs_subscribe(IFA_CB *ifa_cb, NCS_IPXS_SUBCR *i_unsubr);
static uns32 ipxs_unsubscribe(IFA_CB *ifa_cb, NCS_IPXS_UNSUBCR *i_unsubr);
static uns32 ipxs_ipinfo_get(IFA_CB *ifa_cb, NCS_IPXS_IPINFO_GET *ip_get);
static uns32 ipxs_app_send(IFA_CB *cb, NCS_IPXS_SUBCR *subr, IPXS_EVT *evt);
static uns32 ipxs_ifsubr_que_list_comp (uns8 *key1, uns8 *key2);
static uns32 ipxs_ipinfo_upd (IFA_CB *ifa_cb, NCS_IPXS_IPINFO_SET *ip_upd);
static uns32 ipxs_proc_is_loc_req (IFA_CB *ifa_cb, NCS_IPXS_ISLOCAL *isloc);
static uns32 ipxs_proc_node_info_get (IFA_CB *ifa_cb, NCS_IPXS_GET_NODE_REC *node_rec);
static void  ifa_ipxs_evt_destroy(IPXS_EVT *evt);
static uns32 ipxs_intf_rec_mem_free(NCS_IPXS_RSP_INFO *rsp);

/*****************************************************************************
  PROCEDURE NAME    :   ncs_ipxs_svc_req
  DESCRIPTION       :   This is the IPXS SE-API, used by applications to:
                        1. Subscribe and un-subscribe with IPXS for the 
                           interface change notifications
                        2. Getting the interface records.
                        
  ARGUMENTS         :   NCS_IPXS_REQ_INFO *info : Pointer to service req info.

  RETURNS           :   NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  NOTES             :   None.
*****************************************************************************/
uns32 ncs_ipxs_svc_req(NCS_IPXS_REQ_INFO *info)
{
   NCS_IFSV_IPXS_REQ_TYPE  type;
   uns32                   rc = NCSCC_RC_FAILURE;
   IFA_CB                  *ifa_cb = NULL;
   uns32                   cb_hdl;

   /* Validate the received information */
   if(info == NULL)
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                      "NULL pointer rcved in ncs_ipxs_svc_req",0); 
      return NCSCC_RC_FAILURE;
   }

   type = info->i_type;

   cb_hdl = m_IFA_GET_CB_HDL();
   if(cb_hdl == 0)
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                      "m_IFA_GET_CB_HDL returned NULL",0); 
     return NCSCC_RC_FAILURE;
   }
   ifa_cb = (IFA_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFA, cb_hdl);

   if(ifa_cb == NULL)
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                      "ncshm_take_hdl returned NULL",0); 
      return NCSCC_RC_FAILURE;
   }

   switch(type)
   {
   case NCS_IPXS_REQ_SUBSCR:
      rc = ipxs_subscribe(ifa_cb, &info->info.i_subcr);
      break;

   case NCS_IPXS_REQ_UNSUBSCR:
      rc = ipxs_unsubscribe(ifa_cb, &info->info.i_unsubr);
      break;

   case NCS_IPXS_REQ_IPINFO_GET:
      rc = ipxs_ipinfo_get(ifa_cb, &info->info.i_ipinfo_get);
      break;

   case NCS_IPXS_REQ_IPINFO_SET:
      rc = ipxs_ipinfo_upd(ifa_cb, &info->info.i_ipinfo_set);
      break;

   case NCS_IPXS_REQ_IS_LOCAL:
      rc = ipxs_proc_is_loc_req(ifa_cb, &info->info.i_is_local);
      break;

   case NCS_IPXS_REQ_NODE_INFO_GET:
      rc = ipxs_proc_node_info_get(ifa_cb, &info->info.i_node_rec);
      break;

   default:
      break;
   }

   /* Usage of USR_HDL for this struct has finished, return the hdl */
   ncshm_give_hdl(cb_hdl);

   return rc;
}

/*****************************************************************************
  PROCEDURE NAME    :   ncs_ipxs_mem_free_req
  DESCRIPTION       :   This is the IPXS SE-API, used by applications to:
                        1. Free Memory allocated in SYNC RESP call.

  ARGUMENTS         :   NCS_IPXS_MEM_FREE_REQ req : Contains req type and 
                        pointer to free corresponding allocated memory.

  RETURNS           :   NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  NOTES             :   None.
*****************************************************************************/
uns32 ncs_ipxs_mem_free_req(NCS_IPXS_REQ_INFO *info)
{
   uns32 res = NCSCC_RC_FAILURE;

   /* Validate the received information */
   if(info == NULL)
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                      "NULL pointer rcved in ncs_ipxs_mem_free_req",0);
      return res;
   }

   switch(info->i_type)
   {
     case NCS_IPXS_REQ_IPINFO_GET:
     {
       if((NCS_IFSV_IFGET_RSP_SUCCESS == info->info.i_ipinfo_get.o_rsp.err) &&  
          (info->info.i_ipinfo_get.o_rsp.ipinfo != NULL))
       {
         ipxs_intf_rec_list_free((NCS_IPXS_INTF_REC *)info->info.i_ipinfo_get.o_rsp.ipinfo);
         res = NCSCC_RC_SUCCESS;
       }
       break;
     }

     default:
     {
       break;
     }

   }/* switch() */

   return res;

}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_subscribe
  DESCRIPTION       :   Process the IPXS Unsuscribe requests received from 
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.
                        NCS_IPXS_SUBCR *subr : Pointer to Unsubscr Info.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
static uns32 ipxs_subscribe (IFA_CB *cb, NCS_IPXS_SUBCR *i_subr)
{

   IPXS_SUBCR_INFO        *subr = NULL;
   uns32                   rc = NCSCC_RC_SUCCESS;
   IFSV_EVT                evt;
   IPXS_EVT                ipxs_evt;

   /* Check for the subscription of events. */
   if((((i_subr->i_subcr_flag & NCS_IFSV_ADD_IFACE) == NCS_IFSV_ADD_IFACE) ||
      ((i_subr->i_subcr_flag & NCS_IFSV_UPD_IFACE) == NCS_IFSV_UPD_IFACE) ||
      ((i_subr->i_subcr_flag & NCS_IFSV_RMV_IFACE) == NCS_IFSV_RMV_IFACE)) &&
      (((i_subr->i_ifattr & NCS_IFSV_IAM_MTU) == NCS_IFSV_IAM_MTU) ||
      ((i_subr->i_ifattr & NCS_IFSV_IAM_IFSPEED) == NCS_IFSV_IAM_IFSPEED) ||
      ((i_subr->i_ifattr & NCS_IFSV_IAM_PHYADDR) == NCS_IFSV_IAM_PHYADDR) ||
      ((i_subr->i_ifattr & NCS_IFSV_IAM_ADMSTATE) == NCS_IFSV_IAM_ADMSTATE) ||
      ((i_subr->i_ifattr & NCS_IFSV_IAM_OPRSTATE) == NCS_IFSV_IAM_OPRSTATE) ||
      ((i_subr->i_ifattr & NCS_IFSV_IAM_NAME) == NCS_IFSV_IAM_NAME) ||
      ((i_subr->i_ifattr & NCS_IFSV_IAM_DESCR) == NCS_IFSV_IAM_DESCR) ||
      ((i_subr->i_ifattr & NCS_IFSV_IAM_LAST_CHNG) == NCS_IFSV_IAM_LAST_CHNG) ||
      ((i_subr->i_ifattr & NCS_IFSV_IAM_SVDEST) == NCS_IFSV_IAM_SVDEST)) &&
      (((i_subr->i_ipattr & NCS_IPXS_IPAM_ADDR) == NCS_IPXS_IPAM_ADDR) ||
      ((i_subr->i_ipattr & NCS_IPXS_IPAM_UNNMBD) == NCS_IPXS_IPAM_UNNMBD) ||
      ((i_subr->i_ipattr & NCS_IPXS_IPAM_RRTRID) == NCS_IPXS_IPAM_RRTRID) ||
      ((i_subr->i_ipattr & NCS_IPXS_IPAM_RMTRID) == NCS_IPXS_IPAM_RMTRID) ||
      ((i_subr->i_ipattr & NCS_IPXS_IPAM_RMT_AS) == NCS_IPXS_IPAM_RMT_AS) ||
      ((i_subr->i_ipattr & NCS_IPXS_IPAM_RMTIFID) == NCS_IPXS_IPAM_RMTIFID) ||
      ((i_subr->i_ipattr & NCS_IPXS_IPAM_UD_1) == NCS_IPXS_IPAM_UD_1)))
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
       return rc;

   /*Now check for the subscription call back function. It should not be NULL*/
   if(i_subr->i_ipxs_cb == NULL)
      return NCSCC_RC_FAILURE;

   /* Allocate the memory for storing subscription info */
   if((subr = m_MMGR_ALLOC_IPXS_SUBCR_INFO) == NULL)
   {
      m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
      return NCSCC_RC_OUT_OF_MEM;
   }

   /* Mem Set the Subr */
   m_NCS_OS_MEMSET(subr, 0, sizeof(IPXS_SUBCR_INFO));

   /* Get the Handle from Handle Manager for this subscription Record*/
   i_subr->o_subr_hdl = ncshm_create_hdl(cb->hm_pid, NCS_SERVICE_ID_IFA,
                              (NCSCONTEXT) subr);

   /* Populate the Record with received subscription info */
   subr->info = *i_subr;

   /* Add the record to end of the linked list  */
   subr->lnode.key = (uns8*) subr;
   ncs_db_link_list_add(&cb->ipxs_list, &subr->lnode);

   /* If IFND is not UP return from Here */
   if(!cb->is_ifnd_up)
   {
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,"IfND is not up. Subs Success",\
                       IFSV_COMP_IFA);

      return NCSCC_RC_SUCCESS;
   }
   if((i_subr->i_subcr_flag & NCS_IPXS_ADD_IFACE) 
      || (i_subr->i_subcr_flag & NCS_IPXS_UPD_IFACE))
   {
      /* We need tell the subscriber what we already know...
         Send the event to IFND, IFND will send all the interface 
         records that it already has */

      m_NCS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
      m_NCS_MEMSET(&ipxs_evt, 0, sizeof(IPXS_EVT));

      ipxs_evt.type = IPXS_EVT_ATOND_IFIP_GET;
      ipxs_evt.info.nd.get.get_type = IFSV_INTF_GET_ALL;
      ipxs_evt.info.nd.get.my_svc_id = NCSMDS_SVC_ID_IFA;
      ipxs_evt.info.nd.get.my_dest = cb->my_dest;
      ipxs_evt.info.nd.get.usr_hdl = i_subr->o_subr_hdl;

      evt.type = IFSV_IPXS_EVT;
      evt.info.ipxs_evt = &ipxs_evt;

      /* Here I want all the records, I am not filling the if_get.ifkey */
      
      /* Send the Event to IFND */
      rc = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFA, &evt,
             cb->ifnd_dest, NCSMDS_SVC_ID_IFND);
   }
   if(rc == NCSCC_RC_SUCCESS)
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,"Subscription Successful.",\
                       IFSV_COMP_IFA);

   return rc;
}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_unsubscribe
  DESCRIPTION       :   Process the IPXS Unsuscribe requests received from 
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.
                        NCS_IPXS_UNSUBCR *subr : Pointer to Unsubscr Info.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
static uns32 ipxs_unsubscribe (IFA_CB *ifa_cb, NCS_IPXS_UNSUBCR *i_unsubr)
{

   IPXS_SUBCR_INFO        *subr = NULL;

   /* Provide the Handle and Get the Subr Info */
   subr = (IPXS_SUBCR_INFO *) ncshm_take_hdl(NCS_SERVICE_ID_IFA, 
                                                   i_unsubr->i_subr_hdl);
   if(subr == NULL)
      return NCSCC_RC_FAILURE;

   /* Remove the info from Doubly linked List */
   ncs_db_link_list_remove(&ifa_cb->ipxs_list, (uns8 *)subr);

   /* Usage of USR_HDL for this struct has finished, return the hdl */
   ncshm_give_hdl(i_unsubr->i_subr_hdl);

   /* Destroy the hdl for this subscription info */
   ncshm_destroy_hdl(NCS_SERVICE_ID_IFA, i_unsubr->i_subr_hdl);

   /* Free the Subscription info */
   m_MMGR_FREE_IPXS_SUBCR_INFO(subr);
   
   m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,"Un-Subscription Successful.",\
                      IFSV_COMP_IFA);

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ipinfo_get
  DESCRIPTION       :   Process the Interface Record Get request received from
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.
                        NCS_IPXS_IPINFO_GET *ipget : Pointer to get Info.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
static uns32 ipxs_ipinfo_get (IFA_CB *ifa_cb, NCS_IPXS_IPINFO_GET *ip_get)
{
   uns32                   rc = NCSCC_RC_SUCCESS;
   IFSV_EVT                evt, *o_evt = NULL;
   IPXS_EVT                ipxs_evt, *o_ipxs_evt;
   
   /* If IFND is not UP return from Here */
   if(!ifa_cb->is_ifnd_up)
   {
    m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                     "IfND is not up. Req Not processed",IFSV_COMP_IFA);

    return NCS_IFSV_IFND_DOWN_ERROR;
   }
   m_NCS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_MEMSET(&ipxs_evt, 0, sizeof(IPXS_EVT));

   evt.type = IFSV_IPXS_EVT;
   evt.info.ipxs_evt = (NCSCONTEXT) &ipxs_evt;

   ipxs_evt.type = IPXS_EVT_ATOND_IFIP_GET;
   ipxs_evt.info.nd.get.get_type = IFSV_INTF_GET_ONE;
   ipxs_evt.info.nd.get.my_svc_id = NCSMDS_SVC_ID_IFA;
   ipxs_evt.info.nd.get.my_dest = ifa_cb->my_dest;
   ipxs_evt.info.nd.get.usr_hdl = ip_get->i_subr_hdl;
   ipxs_evt.info.nd.get.key = ip_get->i_key;

   if(ip_get->i_key.type == NCS_IPXS_KEY_IP_ADDR)
   {
     if(ip_get->i_key.info.ip_addr.type != NCS_IP_ADDR_TYPE_IPV4)
      return NCSCC_RC_FAILURE;
   }

   /* Send the Event to IFND */
   if(ip_get->i_rsp_type == NCS_IFSV_GET_RESP_ASYNC)
   {
      /* In case of async requests, application has to provide "i_subr_hdl" */
      if(ip_get->i_subr_hdl == 0)
      {
         m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                          "ip_get->i_subr_hdl is NULL",IFSV_COMP_IFA);
         return NCSCC_RC_FAILURE;
      }

      /* Send the Event to IFND */
      rc = ifsv_mds_normal_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA, &evt,
             ifa_cb->ifnd_dest, NCSMDS_SVC_ID_IFND);

   }
   else if(ip_get->i_rsp_type == NCS_IFSV_GET_RESP_SYNC) /* Sync */
   {
      rc = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA, 
                 NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, &evt, &o_evt, 
                 IFA_MDS_SYNC_TIME);

      if((rc == NCSCC_RC_SUCCESS) && (NULL != o_evt))
      {
         /* Copy the received information */
         o_ipxs_evt = (IPXS_EVT *)o_evt->info.ipxs_evt;

         ip_get->o_rsp.err = o_ipxs_evt->info.agent.get_rsp.err;

         if(ip_get->o_rsp.err == NCS_IFSV_IFGET_RSP_SUCCESS)
         {
            ip_get->o_rsp.ipinfo = m_MMGR_ALLOC_NCS_IPXS_INTF_REC;

            if(ip_get->o_rsp.ipinfo == 0)
            {
              m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
              ifa_ipxs_evt_destroy(o_ipxs_evt); 
              m_MMGR_FREE_IFSV_EVT(o_evt);
              return NCSCC_RC_FAILURE;
            }
            m_NCS_OS_MEMSET(ip_get->o_rsp.ipinfo, 0, sizeof(NCS_IPXS_INTF_REC));

            ipxs_intf_rec_cpy(o_ipxs_evt->info.agent.get_rsp.ipinfo, 
                              ip_get->o_rsp.ipinfo, IFSV_MALLOC_FOR_INTERNAL_USE);
         }
        ifa_ipxs_evt_destroy(o_ipxs_evt); 
        m_MMGR_FREE_IFSV_EVT(o_evt);
      }
      else
      {
       rc = NCSCC_RC_FAILURE;
      }
   }
   else
   {
     rc = NCSCC_RC_FAILURE;
   }

   if(rc != NCSCC_RC_SUCCESS)
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                      "IP info get request unsuccessful.",IFSV_COMP_IFA);
   }
   else
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                      "IP info get request successful.",IFSV_COMP_IFA);

   return rc;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_proc_is_loc_req
  DESCRIPTION       :   Process the Interface Record Get request received from
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.
                        NCS_IPXS_IPINFO_GET *ipget : Pointer to get Info.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
static uns32 ipxs_proc_is_loc_req (IFA_CB *ifa_cb, NCS_IPXS_ISLOCAL *isloc)
{
   uns32                   rc = NCSCC_RC_SUCCESS;
   IFSV_EVT                evt, *o_evt;
   IPXS_EVT                ipxs_evt, *o_ipxs_evt;

   /* If IFND is not UP return from Here */
   if(!ifa_cb->is_ifnd_up)
   {
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                     "IfND is not up. Req Not processed",IFSV_COMP_IFA);
      return NCS_IFSV_IFND_DOWN_ERROR;
   }
   m_NCS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_MEMSET(&ipxs_evt, 0, sizeof(IPXS_EVT));

   evt.type = IFSV_IPXS_EVT;
   evt.info.ipxs_evt = (NCSCONTEXT) &ipxs_evt;

   ipxs_evt.type = IPXS_EVT_ATOND_IS_LOCAL;
   ipxs_evt.info.nd.is_loc.ip_addr = isloc->i_ip_addr;
   ipxs_evt.info.nd.is_loc.maskbits = isloc->i_maskbits;
   ipxs_evt.info.nd.is_loc.obs = isloc->i_obs;

   /* Send MDS sync send */
   rc = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA, 
                 NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, &evt, &o_evt, 
                 IFA_MDS_SYNC_TIME);

   if(rc != NCSCC_RC_SUCCESS)
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                      "Is loc request unsuccessful.",IFSV_COMP_IFA);
     return rc;
   }

   if(o_evt)
   {
      o_ipxs_evt = (IPXS_EVT*) o_evt->info.ipxs_evt;
      isloc->o_answer = o_ipxs_evt->info.agent.isloc_rsp.o_answer;
      isloc->o_iface = o_ipxs_evt->info.agent.isloc_rsp.o_iface;
      ifa_ipxs_evt_destroy(o_ipxs_evt); 
      m_MMGR_FREE_IFSV_EVT(o_evt);
   }
   else
      return NCSCC_RC_FAILURE;

   m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                      "Is loc request successful.",IFSV_COMP_IFA);
   return rc;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ipinfo_upd
  DESCRIPTION       :   Process the IP info update request received from
                        Applications.

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.
                        NCS_IPXS_IPINFO_GET *ipget : Pointer to get Info.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
static uns32 ipxs_ipinfo_upd (IFA_CB *ifa_cb, NCS_IPXS_IPINFO_SET *ip_upd)
{
   uns32                   rc = NCSCC_RC_SUCCESS;
   uns32  addr_count = 0;
   IFSV_EVT                evt, *o_evt = NULL;
   IPXS_EVT                ipxs_evt, *o_ipxs_evt = NULL;

   if(((ip_upd->ip_info.ip_attr & NCS_IPXS_IPAM_ADDR) == NCS_IPXS_IPAM_ADDR) ||
      ((ip_upd->ip_info.ip_attr & NCS_IPXS_IPAM_UNNMBD) == 
                                                       NCS_IPXS_IPAM_UNNMBD) ||
      ((ip_upd->ip_info.ip_attr & NCS_IPXS_IPAM_RRTRID) == 
                                                       NCS_IPXS_IPAM_RRTRID) ||
      ((ip_upd->ip_info.ip_attr & NCS_IPXS_IPAM_RMTRID) == 
                                                       NCS_IPXS_IPAM_RMTRID) ||
      ((ip_upd->ip_info.ip_attr & NCS_IPXS_IPAM_RMT_AS) == 
                                                      NCS_IPXS_IPAM_RMT_AS) ||
      ((ip_upd->ip_info.ip_attr & NCS_IPXS_IPAM_RMTIFID) == 
                                                      NCS_IPXS_IPAM_RMTIFID) ||
      ((ip_upd->ip_info.ip_attr & NCS_IPXS_IPAM_UD_1) == NCS_IPXS_IPAM_UD_1))
    {
      rc = NCSCC_RC_SUCCESS; 
    }
    else
    {
      rc = NCSCC_RC_FAILURE;
    }

   if(rc != NCSCC_RC_SUCCESS)
       return rc;

   if(m_NCS_IPXS_IS_IPAM_ADDR_SET(ip_upd->ip_info.ip_attr))
   {
     if(ip_upd->ip_info.addip_cnt)
     {
        if(ip_upd->ip_info.addip_list == NULL)
           return NCSCC_RC_FAILURE;

        for(addr_count = 0; addr_count<ip_upd->ip_info.addip_cnt; addr_count++)
        {
          if(ip_upd->ip_info.addip_list[addr_count].ipaddr.ipaddr.type !=
                                                         NCS_IP_ADDR_TYPE_IPV4)
             return NCSCC_RC_FAILURE;
        }
     }

     if(ip_upd->ip_info.delip_cnt)
     {
        if(ip_upd->ip_info.delip_list == NULL)
           return NCSCC_RC_FAILURE;

        for(addr_count = 0; addr_count<ip_upd->ip_info.delip_cnt; addr_count++)
        {
          if(ip_upd->ip_info.delip_list[addr_count].ipaddr.type !=
                                                         NCS_IP_ADDR_TYPE_IPV4)
             return NCSCC_RC_FAILURE;
        }
     }

     if(ip_upd->ip_info.rrtr_ipaddr.type == NCS_IP_ADDR_TYPE_IPV4)
        return NCSCC_RC_FAILURE;

   }



   /* If IFND is not UP return from Here */
   if(!ifa_cb->is_ifnd_up)
   {
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                     "IfND is not up. Req Not processed",IFSV_COMP_IFA);
      return NCS_IFSV_IFND_DOWN_ERROR;   
   }
   m_NCS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_MEMSET(&ipxs_evt, 0, sizeof(IPXS_EVT));

   evt.type = IFSV_IPXS_EVT;
   evt.info.ipxs_evt = (NCSCONTEXT) &ipxs_evt;

   ipxs_evt.type = IPXS_EVT_ATOND_IFIP_UPD;

   ipxs_evt.info.nd.atond_upd.if_index = ip_upd->if_index;

   rc = ipxs_ipinfo_cpy(&ip_upd->ip_info, 
                 &ipxs_evt.info.nd.atond_upd.ip_info, IFSV_MALLOC_FOR_MDS_SEND);
   
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,"ipxs_ipinfo_cpy error",rc);
      goto free_mem;
   }

   /* Send MDS sync send */
   rc = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA,
                 NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, &evt, &o_evt,
                 IFA_MDS_SYNC_TIME);

   if(rc != NCSCC_RC_SUCCESS)
   {
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                       "Sync Send Error in IP info upd. Error is : ",\
                        rc);
      goto free_mem;
   }
   else
   {
     if(o_evt == NULL)
       rc = NCSCC_RC_FAILURE;
     else
     {
       o_ipxs_evt = o_evt->info.ipxs_evt;
       if(o_ipxs_evt->error != NCS_IFSV_NO_ERROR)
         rc = NCSCC_RC_FAILURE;
       ifa_ipxs_evt_destroy(o_ipxs_evt); 
       m_MMGR_FREE_IFSV_EVT(o_evt);
     }
   }

free_mem:
   
   /* Free only arrtibute pointers */
   ipxs_intf_rec_attr_free(&ipxs_evt.info.nd.atond_upd, FALSE);

   if(rc == NCSCC_RC_SUCCESS)
   {
     m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                      "IP Info upd request successful.",IFSV_COMP_IFA);
   }
   return rc;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_proc_node_info_get
  DESCRIPTION       :   Process the node info get request from application

  ARGUMENTS         :   IFA_CB *ifa_cb   : Pointer to IFA Control Block.
                        NCS_IPXS_IPINFO_GET *ipget : Pointer to get Info.

  RETURNS           :   NCSCC_RC_SUCCESS.
  NOTES             :   None.
*****************************************************************************/
static uns32 ipxs_proc_node_info_get (IFA_CB *ifa_cb, NCS_IPXS_GET_NODE_REC *node_rec)
{
   uns32                   rc = NCSCC_RC_SUCCESS;
   IFSV_EVT                evt, *o_evt;
   IPXS_EVT                ipxs_evt, *o_ipxs_evt;

   /* If IFND is not UP return from Here */
   if(!ifa_cb->is_ifnd_up)
   {
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                     "IfND is not up. Req Not processed",IFSV_COMP_IFA);
      return NCS_IFSV_IFND_DOWN_ERROR;
   }
   m_NCS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_MEMSET(&ipxs_evt, 0, sizeof(IPXS_EVT));

   evt.type = IFSV_IPXS_EVT;
   evt.info.ipxs_evt = (NCSCONTEXT) &ipxs_evt;

   ipxs_evt.type = IPXS_EVT_ATOND_NODE_REC_GET;


   /* Send MDS sync send */
   rc = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA, 
                 NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, &evt, &o_evt, 
                 IFA_MDS_SYNC_TIME);

   if(rc != NCSCC_RC_SUCCESS)
   {
      m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                     "Sync Send Error in node info Get. Error is : ",\
                      rc);

      return rc;
   }

   if(o_evt)
   {
      o_ipxs_evt = (IPXS_EVT *)o_evt->info.ipxs_evt;
      node_rec->o_node_rec = o_ipxs_evt->info.agent.node_rec;
      ifa_ipxs_evt_destroy(o_ipxs_evt); 
      m_MMGR_FREE_IFSV_EVT(o_evt);
   }
   else
      return NCSCC_RC_FAILURE;

   m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                      "node info get request successful.",IFSV_COMP_IFA);
   return rc;
}


/****************************************************************************
 * Name          : ifa_ipxs_evt_process
 *
 * Description   : IFA-MDS will call this function on receiving IFND messages.
 *
 * Arguments     : cb - IFA Control Block
 *                 evt  -> Received Event
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifa_ipxs_evt_process(IFA_CB *cb, IFSV_EVT *evt)
{
   IPXS_EVT          *ipxs_evt;
   IPXS_SUBCR_INFO   *subr_info = NULL;
   uns32 rc = NCSCC_RC_SUCCESS;

   ipxs_evt = evt->info.ipxs_evt;
   
   if(ipxs_evt->usrhdl)
   {
      /* Need to inform only to one user */
      subr_info = (IPXS_SUBCR_INFO *) ncshm_take_hdl(NCS_SERVICE_ID_IFA, 
         (uns32)ipxs_evt->usrhdl);
      /* Update the application */
      if(subr_info)
      {
         ipxs_app_send(cb, &subr_info->info, ipxs_evt);
      }

      if(subr_info)
         ncshm_give_hdl((uns32)ipxs_evt->usrhdl);
   }
   else
   {
      NCS_DB_LINK_LIST_NODE *list_node;
      
      list_node = m_NCS_DBLIST_FIND_FIRST(&cb->ipxs_list);
      
      /* Traverse the subscription info, send to all applications */
      while(list_node)
      {
         subr_info = (IPXS_SUBCR_INFO*)list_node;
         ipxs_app_send(cb, &subr_info->info, ipxs_evt);
         
         list_node = m_NCS_DBLIST_FIND_NEXT(list_node);
      }
   }

   ifa_ipxs_evt_destroy(ipxs_evt);

  return rc;
}

static uns32 ipxs_app_send(IFA_CB *cb, NCS_IPXS_SUBCR *subr, IPXS_EVT *evt)
{
   NCS_IPXS_RSP_INFO  rsp;
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_BOOL    call_app = FALSE;
   NCS_BOOL    node_scope_flag = FALSE;/* To check whether the intf belong 
                                          to the same node.*/


   m_NCS_MEMSET(&rsp, 0, sizeof(NCS_IPXS_RSP_INFO));

   switch(evt->type)
   {
   case IPXS_EVT_NDTOA_IFIP_RSP:
      if(subr->i_subcr_flag & NCS_IPXS_ADD_IFACE)
      {
         rsp.type = NCS_IPXS_REC_GET_RSP;
         rsp.usrhdl   = subr->i_usrhdl;
         rsp.info.get_rsp.err = evt->info.agent.get_rsp.err;

         /* Copy the interface record linked list */
         rc = ipxs_intf_rec_list_cpy(evt->info.agent.get_rsp.ipinfo, 
                          &rsp.info.get_rsp.ipinfo, NULL, &call_app, evt->type, node_scope_flag);
         m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                         "IFIP resp being sent to App. Index is ",\
                          evt->info.agent.get_rsp.ipinfo->if_index);

      }
     break;

   case IPXS_EVT_NDTOA_IFIP_ADD:
      if(subr->i_subcr_flag & NCS_IPXS_ADD_IFACE)
      {
         if(subr->i_subcr_flag & NCS_IPXS_SCOPE_NODE)
         {
          node_scope_flag = TRUE; 
         }
         
         rsp.type = NCS_IPXS_REC_ADD_NTFY;
         rsp.usrhdl   = subr->i_usrhdl;

         /* Copy the interface record linked list */
         rc = ipxs_intf_rec_list_cpy(evt->info.agent.ip_add.i_ipinfo, 
                          &rsp.info.add.i_ipinfo, subr, &call_app, evt->type,node_scope_flag);
         m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                         "IFIP add being sent to App. Index is ",\
                          evt->info.agent.ip_add.i_ipinfo->if_index);
      }
      break;

   case IPXS_EVT_NDTOA_IFIP_DEL:
      if(subr->i_subcr_flag & NCS_IPXS_RMV_IFACE)
      {
         if(subr->i_subcr_flag & NCS_IPXS_SCOPE_NODE)
         {
          node_scope_flag = TRUE; 
         }
         
         rsp.type = NCS_IPXS_REC_DEL_NTFY;
         rsp.usrhdl   = subr->i_usrhdl;

         /* Copy the interface record linked list */
         rc = ipxs_intf_rec_list_cpy(evt->info.agent.ip_del.i_ipinfo, 
                       &rsp.info.del.i_ipinfo, subr, &call_app, evt->type,node_scope_flag);
         m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                         "IFIP del being sent to App. Index is ",\
                          evt->info.agent.ip_del.i_ipinfo->if_index);
      }
      break;

   case IPXS_EVT_NDTOA_IFIP_UPD:
      if(subr->i_subcr_flag & NCS_IPXS_UPD_IFACE)
      {
         if(subr->i_subcr_flag & NCS_IPXS_SCOPE_NODE)
         {
          node_scope_flag = TRUE; 
         }
         
         rsp.type = NCS_IPXS_REC_UPD_NTFY;
         rsp.usrhdl   = subr->i_usrhdl;

         /* Copy the interface record linked list */
         rc = ipxs_intf_rec_list_cpy(evt->info.agent.ip_del.i_ipinfo, 
                       &rsp.info.del.i_ipinfo, subr, &call_app, evt->type,node_scope_flag);
         m_IFA_LOG_EVT_L(IFSV_LOG_IFA_IPXS_EVT_INFO,\
                         "IFIP upd being sent to App. Index is ",\
                          evt->info.agent.ip_del.i_ipinfo->if_index);
      }
      break;

   default:
      return NCSCC_RC_FAILURE;
      break;
   }

   if(call_app)
      subr->i_ipxs_cb(&rsp);
   /* Free the memory.*/
   ipxs_intf_rec_mem_free(&rsp); 

   return rc;
}

/****************************************************************************
 * Name          : ipxs_ifsubr_que_list_comp
 *
 * Description   : Compares the subscription info pointers 
 *
 *****************************************************************************/
static uns32
ipxs_ifsubr_que_list_comp (uns8 *key1, uns8 *key2)
{   
   if ((key1 == NULL) || (key2 == NULL))
   {
      return (0xffff);
   }   
   return (m_NCS_MEMCMP(key1, key2, sizeof(uns32)));
}

void ipxs_agent_lib_init(IFA_CB *cb)
{
   /* Initialize the subscription info Linked List */
   cb->subr_list.order      = NCS_DBLIST_ANY_ORDER;
   cb->ipxs_list.cmp_cookie = ipxs_ifsubr_que_list_comp; 

   return;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_intf_rec_list_cpy
  DESCRIPTION       :   Function to copy the interface record list.
*****************************************************************************/
uns32 ipxs_intf_rec_list_cpy(NCS_IPXS_INTF_REC *src, NCS_IPXS_INTF_REC **dest,
                             NCS_IPXS_SUBCR *subr, NCS_BOOL *is_app_send, 
                             IPXS_EVT_TYPE evt_type,NCS_BOOL node_scope_flag)
{
   
   NCS_IPXS_INTF_REC *cur_rec=0, *prev_rec=0, *src_rec=0;
   uns32 rc=NCSCC_RC_SUCCESS;
   NCS_IPXS_ATTR_BM  orig_ip, subr_ip_attr;
   NCS_IFSV_BM       orig_if, subr_if_attr;

   if(subr)
   {
      subr_ip_attr = subr->i_ipattr;
      subr_if_attr = subr->i_ifattr;
   }
   else
   {
      /* Set all attributes */
      subr_ip_attr = 0xffffffff;
      subr_if_attr = 0xffffffff;
   }


   src_rec = src;

   while(src_rec)
   {
      /* Send to applications only if they are intrested */
      if((evt_type != IPXS_EVT_NDTOA_IFIP_UPD) ||
         (src_rec->ip_info.ip_attr & subr_ip_attr) || 
         (src_rec->if_info.if_am & subr_if_attr))
      {
        if(node_scope_flag == TRUE)
        {
         /*This means the app has subscribed for local intf changes. */
         if(src_rec->is_lcl_intf == FALSE)
         {
           /* This record doesn't belong to this node. So don't send 
              it the this app. Try copying next record.*/
            goto next_rec;
         }
        }

         /* Malloc the IPXS interface rec */
         cur_rec = m_MMGR_ALLOC_NCS_IPXS_INTF_REC;   /* TBD Mem checks */
         if(cur_rec == NULL)
         {
           m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,IFSV_COMP_IFA);
           return NCSCC_RC_FAILURE;
         }
         m_NCS_OS_MEMSET(cur_rec, 0, sizeof(NCS_IPXS_INTF_REC));

         /* Store the original attributes info */
         orig_ip = src_rec->ip_info.ip_attr;
         orig_if = src_rec->if_info.if_am;

         /* Tamper the src_rec attributes */
         src_rec->ip_info.ip_attr &= subr_ip_attr;
         src_rec->if_info.if_am &= subr_if_attr;

         rc = ipxs_intf_rec_cpy(src_rec, cur_rec, IFSV_MALLOC_FOR_INTERNAL_USE);

         /* Restore the original attribute maps */
         src_rec->ip_info.ip_attr = orig_ip;
         src_rec->if_info.if_am = orig_if;

         if(rc != NCSCC_RC_SUCCESS)
            goto free_mem;

         if(prev_rec == 0)
         {
            *dest= cur_rec;
            cur_rec->next = NULL;
         }
         else
         {
            prev_rec->next = cur_rec;
            cur_rec->next = NULL;
         }
         prev_rec = cur_rec;

         *is_app_send = TRUE;
      }
next_rec:
      
      src_rec = src_rec->next;

   }


   return NCSCC_RC_SUCCESS;

free_mem:
   
   return rc;
}


/*
 ***************************************************************************
 *  PROCEDURE NAME    :   ifa_ipxs_evt_destroy                             *
 *  DESCRIPTION       :   Function to free the  IPXSEVT, and its           *
 *                                     allocated sub memebers list.        *
 ***************************************************************************
*/

void
ifa_ipxs_evt_destroy(IPXS_EVT *evt)
{
   switch(evt->type)
   {
      case IPXS_EVT_NDTOA_IFIP_RSP:
                         /* Need to delete the intf_rec in this case */
           ipxs_intf_rec_list_free(evt->info.agent.get_rsp.ipinfo);
            m_MMGR_FREE_IPXS_EVT(evt);
            /* function which takes care of deleting the Linked List */
            break;
      case IPXS_EVT_NDTOA_IFIP_ADD:
           /* Need to delete the intf_rec in this case */
           ipxs_intf_rec_list_free(evt->info.agent.ip_add.i_ipinfo);
            m_MMGR_FREE_IPXS_EVT(evt);
            break;
      case IPXS_EVT_NDTOA_IFIP_DEL:
            /* Need to delete the intf_rec in this case */
            ipxs_intf_rec_list_free(evt->info.agent.ip_del.i_ipinfo);
            m_MMGR_FREE_IPXS_EVT(evt);
            break;
      case IPXS_EVT_NDTOA_IFIP_UPD:
      case IPXS_EVT_NDTOA_IFIP_UPD_RESP:
           /* Need to delete the intf_rec in this case */
           ipxs_intf_rec_list_free(evt->info.agent.ip_upd.i_ipinfo);
           m_MMGR_FREE_IPXS_EVT(evt);
           break;

      default:
           m_MMGR_FREE_IPXS_EVT(evt);
           break;
   }
   return;
}
/****************************************************************************
 * Name          : ipxs_intf_rec_mem_free
 *
 * Description   : Delete the memory, which has been allocated in response. 
 *
 *****************************************************************************/
static uns32
ipxs_intf_rec_mem_free(NCS_IPXS_RSP_INFO *rsp)
{
   if(rsp == NULL)
      return NCSCC_RC_FAILURE;

   switch(rsp->type)
   {
     case NCS_IPXS_REC_GET_RSP:
     {
       ipxs_intf_rec_list_free(rsp->info.get_rsp.ipinfo);
       break;
     }

     case NCS_IPXS_REC_ADD_NTFY:
     {
       ipxs_intf_rec_list_free(rsp->info.add.i_ipinfo);
       break;
     }

     case NCS_IPXS_REC_DEL_NTFY:
     {
       ipxs_intf_rec_list_free(rsp->info.del.i_ipinfo);
       break;
     }

     case NCS_IPXS_REC_UPD_NTFY:
     {

       ipxs_intf_rec_list_free(rsp->info.del.i_ipinfo);
       break;
     }

     default:
     {
       return NCSCC_RC_FAILURE;
     }

   }/* switch() */

   return NCSCC_RC_SUCCESS;

}
