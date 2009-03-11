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

#if (NCS_VIP == 1)

#include "ifnd.h"


/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
uns32 ifsv_vip_send_ipxs_evt(IFSV_CB *cb,
                             uns32 type,
                             NCS_IFSV_IFINDEX index,
                             NCS_IPPFX *ipInfo,
                             uns32      poolHdl,
                             uns32      ipPoolType,
                             uns8       *intfName
                             );
uns32
ncs_vip_check_ip_exists(IFSV_CB *cb,
                        uns32 hdl,
                        uns32 ipaddr,
                        uns8 *intf);
uns32
ncs_ifsv_vip_del_ip(IFSV_CB *cb,
                    uns32 hdl,
                    NCS_IPPFX *ipAddr
                    );
uns32
ifnd_ifa_proc_vipd_info_add(IFSV_CB *cb,
                            IFSV_EVT *evt
                            );
uns32
ifnd_ifa_proc_vip_free(IFSV_CB *cb,
                       IFSV_EVT  *pEvt
                       );
uns32
ifnd_proc_ifnd_vip_del_vipd(IFSV_CB *cb, 
                            IFSV_EVT  *pEvt
                            );

uns32
ifnd_process_ifa_crash(IFSV_CB *cb,
                       MDS_DEST *mdsDest
                       );

                                                                                                                             
/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

/************************************************************
* This function forms an IFND_MARK_VIP_ENTRY_STALE event
* and stores the event in the IfND mail box. This event will
* be Processed at a later stage as a part of event processing
*************************************************************/

static
uns32 ifnd_create_mark_vip_entry_stale_evt(IFSV_CB *cb, uns8 * applName,uns32 hdl, uns32 type)
{
   IFSV_EVT         *ifsv_evt;
   SYSF_MBX         *mbx;

   ifsv_evt =  m_MMGR_ALLOC_IFSV_EVT;
   if (ifsv_evt == IFSV_NULL)
   {
      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                          IFSV_VIP_MEM_ALLOC_FAILED);
      m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "IFSV_EVT allocation failed in ifnd_create_mark_vip_entry_stale_evt()"," ");
      return NCSCC_RC_FAILURE;
   }
   memset(ifsv_evt,0,sizeof(IFSV_EVT));
   ifsv_evt->type = IFND_VIP_MARK_VIPD_STALE;
   m_NCS_STRCPY(ifsv_evt->info.vip_evt.info.vipCommonEvt.handle.vipApplName,applName);
   ifsv_evt->info.vip_evt.info.vipCommonEvt.handle.poolHdl = hdl;
/*   ifsv_evt->info.vip_evt.info.vipCommonEvt.handle.ipPoolType =  type; */

   /* Put it in IFND's Event Queue */
   mbx = &cb->mbx;
   ifsv_evt->cb_hdl  = cb->cb_hdl;

   if(m_IFND_EVT_SEND(mbx, ifsv_evt, NCS_IPC_PRIORITY_NORMAL)
      == NCSCC_RC_FAILURE)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_SEND_FAIL,(long)mbx);
      m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "IFND_EVT_SEND failed in ifnd_create_mark_vip_entry_stale_evt()"," ");
      m_MMGR_FREE_IFSV_EVT(ifsv_evt);
      return (NCSCC_RC_FAILURE);
   }

   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                       IFSV_VIP_CREATED_IFND_IFND_VIP_DEL_VIPD_EVT);

  return NCSCC_RC_SUCCESS;
}

/* End of ifnd_create_mark_vip_entry_stale  function */


/************************************************************
* This function forms an IfND_IFND_VIP_DEL_VIPD event
* and stores the event in the IfND mail box. This event will
* be Processed at a later stage as a part of event processing
*************************************************************/
                                                                                                                              
/* Form an IFND event and store it in mail box.. to be processed later.*/

/*******************************************************
* This function is used to process, IFND_VIP_MARK_VIPD_STALE
* Event. 
*
*******************************************************/
uns32
ifnd_proc_vip_mark_vipd_stale(IFSV_CB *cb, IFSV_EVT  *pEvt)
{
   uns32                rc;
   IFSV_EVT             *rcvEvt=IFSV_NULL;
   NCS_PATRICIA_NODE    *pPatEntry;
   IFSV_EVT sendEvt;

   if ( cb == IFSV_NULL || pEvt == IFSV_NULL)
   {
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Received NULL cb or NULL evt ptr :ifnd_proc_vip_mark_vipd_stale "," ");
       /* LOG ERROR MESSAGE */
       return NCSCC_RC_VIP_INTERNAL_ERROR;
   }
/*
   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                          IFSV_VIP_RECEIVED_IFND_IFND_VIP_DEL_VIPD_EVT);
*/                                                                                                                              
   /* form IFND_VIP_MARK_VIPD_STALE event and send to IFD */
   memset(&sendEvt,0,sizeof(IFSV_EVT));
                                                                                                                              
   sendEvt.type = IFND_VIP_MARK_VIPD_STALE;

   m_NCS_STRCPY(&sendEvt.info.vip_evt.info.vipCommonEvt.handle.vipApplName,
                &pEvt->info.vip_evt.info.vipCommonEvt.handle.vipApplName);
                                                                                                                              
   sendEvt.info.vip_evt.info.vipCommonEvt.handle.poolHdl =
                         pEvt->info.vip_evt.info.vipCommonEvt.handle.poolHdl;
                                                                                                                              
   sendEvt.info.vip_evt.info.vipCommonEvt.handle.ipPoolType =
                      pEvt->info.vip_evt.info.vipCommonEvt.handle.ipPoolType;
/*
   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                   IFSV_VIP_SENDING_IFND_VIP_FREE_REQ );
*/                                                                                                                              
   rc = ifsv_mds_msg_sync_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
            NCSMDS_SVC_ID_IFD, cb->ifd_dest, &sendEvt, &rcvEvt,
            IFSV_MDS_SYNC_TIME);
                                                                                                                              
   if (rc != NCSCC_RC_SUCCESS || rcvEvt == IFSV_NULL) /* IF VIPD is not marked stale */
   {
       /* LOG ERROR MESSAGE */
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "MDS Sync send failed from IfND to IfD :ifnd_proc_vip_mark_vipd_stale "," ");
       return NCSCC_RC_VIP_INTERNAL_ERROR;
   }
                                                                                                                              
   if (rcvEvt != NULL && rcvEvt->type == IFSV_VIP_ERROR)
   {  rc = rcvEvt->info.vip_evt.info.errEvt.err;
      m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Received Error Evt from IfD :ifnd_proc_vip_mark_vipd_stale "," ");
      m_MMGR_FREE_IFSV_EVT(rcvEvt);
      return rc;
   }
    
   pPatEntry = ifsv_vipd_vipdc_rec_get(&cb->vipDBase,
               (NCS_IFSV_VIP_INT_HDL *)&sendEvt.info.vip_evt.info.vipCommonEvt.handle); 

   if (pPatEntry == IFSV_NULL)
       return NCSCC_RC_VIP_INTERNAL_ERROR;
                                                                                                                              
   rc = ifsv_vipd_vipdc_rec_del(&cb->vipDBase,(NCS_PATRICIA_NODE *)pPatEntry,
                                                   IFSV_VIP_REC_TYPE_VIPDC);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                   IFSV_VIP_VIPDC_RECORD_DELETION_FAILED );
      m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "VIPDC record deletion failed :ifnd_proc_vip_mark_vipd_stale "," ");
   }

   m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "VIPDC record deletion SUCCESS :ifnd_proc_vip_mark_vipd_stale "," ");
   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                   IFSV_VIP_VIPDC_RECORD_DELETION_SUCCESS );
                                                                                                                              
   if (rcvEvt != IFSV_NULL)
       m_MMGR_FREE_IFSV_EVT(rcvEvt);
 
   return NCSCC_RC_SUCCESS;
                                                                                                                              
}

/*******************************************************
* This function is used to process, IFND_IFND_VIP_DEL_VIPD
* Event. 
*
*******************************************************/
uns32
ifnd_proc_ifnd_vip_del_vipd(IFSV_CB *cb, IFSV_EVT  *pEvt)
{
   uns32                rc;
   IFSV_EVT             *rcvEvt=IFSV_NULL;
   NCS_PATRICIA_NODE    *pPatEntry;

   IFSV_EVT sendEvt;
   if ( cb == IFSV_NULL || pEvt == IFSV_NULL)
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Received NULL cb or NULL evt ptr :ifnd_proc_ifnd_vip_del_vipd "," ");
   {
       /* LOG ERROR MESSAGE */
       return NCSCC_RC_VIP_INTERNAL_ERROR;
   }

   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                          IFSV_VIP_RECEIVED_IFND_IFND_VIP_DEL_VIPD_EVT);
                                                                                                                              
   /* form IFND_VIP_FREE event and send to IFD */
   memset(&sendEvt,0,sizeof(IFSV_EVT));
                                                                                                                              
   sendEvt.type = IFND_VIP_FREE_REQ;
   m_NCS_STRCPY(&sendEvt.info.vip_evt.info.ifndVipFree.handle.vipApplName,
                &pEvt->info.vip_evt.info.ifaVipFree.handle.vipApplName);
                                                                                                                              
   sendEvt.info.vip_evt.info.ifndVipFree.handle.poolHdl =
                         pEvt->info.vip_evt.info.ifaVipFree.handle.poolHdl;
                                                                                                                              
   sendEvt.info.vip_evt.info.ifndVipFree.handle.ipPoolType =
                      pEvt->info.vip_evt.info.ifaVipFree.handle.ipPoolType;

   sendEvt.info.vip_evt.info.ifndVipFree.infraFlag =
                      pEvt->info.vip_evt.info.ifaVipFree.infraFlag;
   

   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                   IFSV_VIP_SENDING_IFND_VIP_FREE_REQ );
                                                                                                                              
   rc = ifsv_mds_msg_sync_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
            NCSMDS_SVC_ID_IFD, cb->ifd_dest, &sendEvt, &rcvEvt,
            IFSV_MDS_SYNC_TIME);
                                                                                                                              
   if (rc != NCSCC_RC_SUCCESS || rcvEvt == IFSV_NULL) /* IF VIPD is not cleared */
   {
       /* LOG ERROR MESSAGE */
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "MDS Sync send failed from IfND to IfD :ifnd_proc_ifnd_vip_del_vipd "," ");
       return NCSCC_RC_VIP_INTERNAL_ERROR;
   }
                                                                                                                              
   if (rcvEvt != NULL && rcvEvt->type == IFSV_VIP_ERROR)
   { 
      rc = rcvEvt->info.vip_evt.info.errEvt.err;
      m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Received Error Evt from IfD :ifnd_proc_ifnd_vip_del_vipd "," ");
      m_MMGR_FREE_IFSV_EVT(rcvEvt);
      return rc;
   }
    
   pPatEntry = ifsv_vipd_vipdc_rec_get(&cb->vipDBase,
               (NCS_IFSV_VIP_INT_HDL *)&sendEvt.info.vip_evt.info.ifndVipFree.handle); 

   if (pPatEntry == IFSV_NULL)
       return NCSCC_RC_VIP_INTERNAL_ERROR;
                                                                                                                              
   rc = ifsv_vipd_vipdc_rec_del(&cb->vipDBase,(NCS_PATRICIA_NODE *)pPatEntry,
                                                   IFSV_VIP_REC_TYPE_VIPDC);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "VIPDC record deletion failed :ifnd_proc_ifnd_vip_del_vipd "," ");
      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                   IFSV_VIP_VIPDC_RECORD_DELETION_FAILED );
   }

   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                   IFSV_VIP_VIPDC_RECORD_DELETION_SUCCESS );
                                                                                                                              
   if (rcvEvt != IFSV_NULL)
       m_MMGR_FREE_IFSV_EVT(rcvEvt);
 
   return NCSCC_RC_SUCCESS;
                                                                                                                              
}



/********************************************************************
* Code for unInstalling an IP Address
*********************************************************************/


uns32  m_ncs_uninstall_vip(NCS_IPPFX *ipAddr,uns8 * intf)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns8 uninstall[100];

   sprintf(uninstall, "/sbin/ip addr del %d.%d.%d.%d dev %s",(((ipAddr->ipaddr.info.v4) & 0xff000000) >> 24),
                                                          (((ipAddr->ipaddr.info.v4) & 0xff0000) >> 16),
                                                          (((ipAddr->ipaddr.info.v4) & 0xff00) >> 8),
                                                          ((ipAddr->ipaddr.info.v4) & 0xff),
                                                          (intf));
   system(uninstall);
   return rc;
}


/****************************************************************************
 * Name          : ifsv_vip_send_ipxs_evt
 *
 * Description   : This function used to form and send an IPXS event
 *
 * Arguments     : cb : this is the IfND control block pointer
 *                 type : the request is formed depending on this type 
 *                 index: Index of the interface/IPXS record 
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
                                                                                                      
uns32    ifsv_vip_send_ipxs_evt(IFSV_CB *cb,uns32 type,NCS_IFSV_IFINDEX index, 
                                 NCS_IPPFX *ipInfo,
                                 uns32  poolHdl,
                                 uns32  ipPoolType,
                                 uns8   *intfName)
{
   uns32                rc = NCSCC_RC_SUCCESS;
   uns32                ipxs_hdl;
   IPXS_EVT             sendEvt;
   IPXS_CB             *ipxs_cb;
   IPXS_IFIP_IP_INFO   ifIpInfo;


                                                                                                                         
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);
                                                                                                                             
   memset(&sendEvt,0,sizeof(IPXS_EVT));
   memset(&ifIpInfo,0,sizeof(IPXS_IFIP_IP_INFO));


   sendEvt.info.nd.atond_upd.if_index = index;

   sendEvt.info.nd.atond_upd.ip_info.shelfId = cb->shelf;
   sendEvt.info.nd.atond_upd.ip_info.slotId =  cb->slot;
   sendEvt.info.nd.atond_upd.ip_info.subslotId =  cb->subslot;
   sendEvt.info.nd.atond_upd.ip_info.nodeId =  cb->my_node_id;
   /* Here I'm putting a hack: need to remove it */
                                                                                                                             
   switch(type) /* Add the IP */
   {
      case IFSV_VIP_IPXS_ADD:

         NCS_IPXS_IPAM_VIP_SET(sendEvt.info.nd.atond_upd.ip_info.ip_attr);

         ifIpInfo.poolHdl    = poolHdl;
         ifIpInfo.ipPoolType = ipPoolType; 
         ifIpInfo.vipIp      = 1;
         /* Since this is the new IP, keep ref cnt = 1*/
         ifIpInfo.refCnt     = 1;
         ifIpInfo.ipaddr.ipaddr.type = ipInfo->ipaddr.type;
         ifIpInfo.ipaddr.mask_len = ipInfo->mask_len;
         ifIpInfo.ipaddr.ipaddr.info.v4 = ipInfo->ipaddr.info.v4;
         sendEvt.info.nd.atond_upd.ip_info.addip_cnt   =  1;

         sendEvt.info.nd.atond_upd.ip_info.addip_list = &ifIpInfo;
         m_NCS_STRCPY(&sendEvt.info.nd.atond_upd.ip_info.intfName,intfName);
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                             IFSV_VIP_SENDING_IPXS_ADD_IP_REQ);

         break;
                                                                                                                             
      case IFSV_VIP_IPXS_DEL: /* DEL the IP */
         NCS_IPXS_IPAM_VIP_SET(sendEvt.info.nd.atond_upd.ip_info.ip_attr);
         sendEvt.info.nd.atond_upd.ip_info.delip_cnt =  1;
         sendEvt.info.nd.atond_upd.ip_info.delip_list = ipInfo;
         m_NCS_STRCPY(&sendEvt.info.nd.atond_upd.ip_info.intfName,intfName);
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                             IFSV_VIP_SENDING_IPXS_DEL_IP_REQ );
         break;
                                                                                                                             
      case IFSV_VIP_IPXS_DEC_REFCNT:
         NCS_IPXS_IPAM_REFCNT_SET(sendEvt.info.nd.atond_upd.ip_info.ip_attr);
         NCS_IPXS_IPAM_VIP_SET(sendEvt.info.nd.atond_upd.ip_info.ip_attr);
         ifIpInfo.ipaddr.ipaddr.type = ipInfo->ipaddr.type;
         ifIpInfo.ipaddr.mask_len = ipInfo->mask_len;
         ifIpInfo.ipaddr.ipaddr.info.v4 = ipInfo->ipaddr.info.v4;
         m_NCS_STRCPY(&sendEvt.info.nd.atond_upd.ip_info.intfName,intfName);
         ifIpInfo.refCnt = 0;
         sendEvt.info.nd.atond_upd.ip_info.addip_cnt   =  1;
         sendEvt.info.nd.atond_upd.ip_info.addip_list = &ifIpInfo;
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                             IFSV_VIP_SENDING_IPXS_DEC_REFCNT_REQ );
         break;
                                                                                                                             
      case IFSV_VIP_IPXS_INC_REFCNT:
         NCS_IPXS_IPAM_REFCNT_SET(sendEvt.info.nd.atond_upd.ip_info.ip_attr);
         NCS_IPXS_IPAM_VIP_SET(sendEvt.info.nd.atond_upd.ip_info.ip_attr);
         ifIpInfo.ipaddr.ipaddr.type = ipInfo->ipaddr.type;
         ifIpInfo.ipaddr.mask_len = ipInfo->mask_len;
         ifIpInfo.ipaddr.ipaddr.info.v4 = ipInfo->ipaddr.info.v4;
         m_NCS_STRCPY(&sendEvt.info.nd.atond_upd.ip_info.intfName,intfName);
         ifIpInfo.refCnt = 1;
         sendEvt.info.nd.atond_upd.ip_info.addip_cnt   =  1;
         sendEvt.info.nd.atond_upd.ip_info.addip_list = &ifIpInfo;
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                             IFSV_VIP_SENDING_IPXS_INC_REFCNT_REQ);
         break;
                                                                                                                             
   }  
                                                                                                                             
   rc = ifnd_ipxs_proc_ifip_upd(ipxs_cb, &sendEvt, NULL);
   ncshm_give_hdl(ipxs_hdl);
   return rc;
}

/**********************************************************************************************
* This Function Checks if the given IP address already exists or not
* Called From IfND routines
*
*                 
* Return Values: NCSCC_RC_SUCCESS,NCSCC_RC_FAILURE,
*                NCSCC_VIP_EXISTS_ON_OTHER_INTERFACE,
*                NCSCC_VIP_EXISTS_FOR_OTHER_HANDLE; 
*
* NOTES : The return value NCSCC_RC_SUCCESS indicates that the given
*         IP address is installed on the given interface.
*         NCSCC_RC_FAILURE: indicates that the given ip is not installed. 
***********************************************************************************************/

uns32
ncs_vip_check_ip_exists(IFSV_CB *cb, uns32 hdl,uns32 ipaddr,uns8 *intf)
{
   IPXS_CB           *ipxs_cb;
   uns32              ipxs_hdl;
   IPXS_IFIP_NODE    *ifip_node=0;
   uns32              ifindex=0;
   int ii;
   IPXS_IFIP_IP_INFO  *pIpInfo; 

   /* Get the IPXS CB */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);

   /* Get the starting node from the tree */
   ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                                  (&ipxs_cb->ifip_tbl, (uns8*)0);

   if(ifip_node == NULL)
   {
      /* TBD : IPXS TBL LOOKUP FAILED */
      /* TBD LOG : NCSCC_RC_VIP_INTF_RECS_NOT_CREATED; */
      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                          IFSV_VIP_INTF_REC_NOT_CREATED);
      ncshm_give_hdl(ipxs_hdl);
      return NCSCC_RC_VIP_INTERNAL_ERROR;
   }

   while(ifip_node)
   {
      ifindex = ifip_node->ifip_info.ifindexNet;
      for (ii = 0; ii < ifip_node->ifip_info.ipaddr_cnt; ii++)
      {
         /* Get the individual ip address from list  */
         pIpInfo = &ifip_node->ifip_info.ipaddr_list[ii]; 
         /* Compare existing ip with that of new ip */
         if(ipaddr == pIpInfo->ipaddr.ipaddr.info.v4)
         {
            /* since ip is matched check if it is VIP */
            if (pIpInfo->vipIp)
            {
               if (pIpInfo->poolHdl  == hdl)
               {
                  /* Check if IP Exists on the requested interface */
                  if(ifip_node->ifip_info.subslotId == cb->subslot &&
                     ifip_node->ifip_info.slotId == cb->slot &&
                     ifip_node->ifip_info.shelfId == cb->shelf && 
                     ifip_node->ifip_info.nodeId == cb->my_node_id) 
                  {
                     if(m_NCS_STRCMP(intf,ifip_node->ifip_info.intfName) == 0)
                     {
                        ncshm_give_hdl(ipxs_hdl);
                        return NCSCC_RC_SUCCESS;
                     } /* end of strcmp */
                     else  /* else for interface name comparision */
                     {
                         /* TBD : LOG MESSAGE */
                         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                                          IFSV_VIP_IP_EXISTS_ON_OTHER_INTERFACE);
                         ncshm_give_hdl(ipxs_hdl);
                         return NCSCC_VIP_EXISTS_ON_OTHER_INTERFACE;
                     }
 
                  } else  /* else for shelf slot port comparision */
                  {
                      /* TBD : LOG MESSAGE */
                      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                                          IFSV_VIP_IP_EXISTS_ON_OTHER_INTERFACE);
                      ncshm_give_hdl(ipxs_hdl);
                      return NCSCC_VIP_EXISTS_ON_OTHER_INTERFACE;
                  }
               } else 
               { /* else part of if (pIpInfo.poolhdl == hdl) */
                   /* TBD : LOG MESSAGE */
                   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                                       IFSV_VIP_IP_EXISTS_FOR_OTHER_HANDLE);
                   ncshm_give_hdl(ipxs_hdl);
                   return NCSCC_VIP_EXISTS_FOR_OTHER_HANDLE;
               } 
            }else     /* else part for pIpInfo->vipIP */
            {
               m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                                   IFSV_VIP_IP_EXISTS_BUT_NOT_VIP);
               ncshm_give_hdl(ipxs_hdl);
               return NCSCC_RC_IP_EXISTS_BUT_NOT_VIP;
            } /* end of else for pIpInfo->vipIP */
         }    /* end of ipaddr = */
      }       /* end of for loop */
      ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                           (&ipxs_cb->ifip_tbl, (uns8*)&ifindex);
   } /* end of while loop */
   ncshm_give_hdl(ipxs_hdl);
   return NCSCC_RC_FAILURE;
}


/****************************************************************************
 * Name          : ncs_ifsv_vip_del_ip 
 *
 * Description   : This function is used to delete the given IP Address
 *
 * Arguments     : cb - This is the IfND control Block
 *                 hdl- poolHandle of VIP record
 *                 ipAddr - contains the IP Address to be deleted.          
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32
ncs_ifsv_vip_del_ip(IFSV_CB *cb,uns32 hdl,NCS_IPPFX *ipAddr)
{
   uns32              ipxs_hdl;
   uns32              ifindex=0;
   uns32              ii;
   uns32              rc = NCSCC_RC_SUCCESS;
   IPXS_IFIP_NODE    *ifip_node=0;
   IPXS_CB           *ipxs_cb;
   IPXS_IFIP_IP_INFO *pIpInfo;

   /* Get the IPXS CB */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);

   ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                                  (&ipxs_cb->ifip_tbl, (uns8*)0);

   if(ifip_node == 0)
   {
      /* No record */
      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                          IFSV_VIP_INTF_REC_NOT_CREATED);
      ncshm_give_hdl(ipxs_hdl);
      return NCSCC_RC_FAILURE;
   }

   while(ifip_node)
   {
     ifindex = ifip_node->ifip_info.ifindexNet;
     if(ifip_node->ifip_info.subslotId == cb->subslot &&
        ifip_node->ifip_info.slotId == cb->slot && 
        ifip_node->ifip_info.shelfId == cb->shelf && 
        ifip_node->ifip_info.nodeId == cb->my_node_id)
     {
        for (ii = 0; ii < ifip_node->ifip_info.ipaddr_cnt; ii++)
        {
           pIpInfo = &ifip_node->ifip_info.ipaddr_list[ii];

           if (pIpInfo->vipIp)     
           {
               if(memcmp(ipAddr,&pIpInfo->ipaddr,sizeof(NCS_IPPFX)) == 0)
               {
                  if (pIpInfo->poolHdl  == hdl)
                  {
                     if (pIpInfo->refCnt > 1)
                     {
                        /* Send IPXS event for decrementing reference count */
                        rc = ifsv_vip_send_ipxs_evt(cb,IFSV_VIP_IPXS_DEC_REFCNT,
                                               ifip_node->ifip_info.ifindexNet,ipAddr,0,0,
                                               ifip_node->ifip_info.intfName);
                        ncshm_give_hdl(ipxs_hdl);
                        return NCSCC_RC_SUCCESS;
                     }
                     else  /* end of ifip_node->refCnt > 1*/
                     {
                        /* send IPXS event to delete the ip address */
                        rc = ifsv_vip_send_ipxs_evt(cb,IFSV_VIP_IPXS_DEL,
                                             ifip_node->ifip_info.ifindexNet,ipAddr,0,0,
                                             ifip_node->ifip_info.intfName);

                        if (rc == NCSCC_RC_SUCCESS)
                        {
                           /* uninstall the ipaddress from the given interface */
                           m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                                               IFSV_VIP_UNINSTALLING_VIRTUAL_IP);
                           m_NCS_UNINSTALL_VIP(ipAddr,ifip_node->ifip_info.intfName);
                           ncshm_give_hdl(ipxs_hdl);
                           return NCSCC_RC_SUCCESS;
                        }
                     }         /* end of else ifip_node->refCnt > 1 */
                  }            /* end of  ifip_node->poolHdl == hdl */
               }               /* end of memcmp */
            }                  /* end of if (pIpInfo->vipIp) */
         }                     /* end of for loop */
      }                        /* end of comparision of shelf, slot and node id */
      ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                                  (&ipxs_cb->ifip_tbl, (uns8*)&ifindex);
   } /* end of while loop */
   ncshm_give_hdl(ipxs_hdl);
   return NCSCC_RC_FAILURE;
}

/****************************************************************************
 * Name          : ifnd_ifa_proc_get_ip_from_stale_entry
 *
 * Description   : This function is used to get ip from stale VIPDB
 *
 * Arguments     : cb - This is the IfND control Block
 *                 evt- ptr to evt from Ifa 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifnd_ifa_proc_get_ip_from_stale_entry(IFSV_CB *cb, IFSV_EVT *evt)
{

    uns32          rc = NCSCC_RC_SUCCESS;
/*    VIP_COMMON_EVENT    getIpEvt; */
    IFSV_EVT            ipEvt,*o_evt,respEvt;
    uns32               res;
    
    if (evt == IFSV_NULL || cb == IFSV_NULL)
       return NCSCC_RC_FAILURE;

    memset(&ipEvt, 0, sizeof(IFSV_EVT));

    ipEvt.type = IFND_GET_IP_FROM_STALE_ENTRY_REQ;

    m_NCS_STRCPY(&ipEvt.info.vip_evt.info.vipCommonEvt.handle.vipApplName,
                 &evt->info.vip_evt.info.vipCommonEvt.handle.vipApplName);

    ipEvt.info.vip_evt.info.vipCommonEvt.handle.poolHdl = 
                                 evt->info.vip_evt.info.vipCommonEvt.handle.poolHdl;
    ipEvt.info.vip_evt.info.vipCommonEvt.handle.ipPoolType =  
                            evt->info.vip_evt.info.vipCommonEvt.handle.ipPoolType;

    res = ifsv_mds_msg_sync_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
             NCSMDS_SVC_ID_IFD, cb->ifd_dest, &ipEvt, &o_evt,
             IFSV_MDS_SYNC_TIME);


    if ( res != NCSCC_RC_SUCCESS)
    {
             
        m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "MDS Sync send failed from IfND to IfD :ifnd_ifa_proc_get_ip_from_stale_entry "," ");
        /* LOG : MDS SEND FAILURE  */
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_MDS_SEND_FAILURE);
        return NCSCC_RC_VIP_RETRY;
    }
  
    if (o_evt == IFSV_NULL)
    {
        /* Return a Suitable Error */
        return NCSCC_RC_VIP_RETRY;
    }

    if (o_evt->type == IFSV_VIP_ERROR)
    {
        /* TBD : Check for the error */
        m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Recived Error Evt from IfD :ifnd_ifa_proc_get_ip_from_stale_entry "," ");
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                            IFSV_VIP_RECEIVED_ERROR_RESP_FOR_VIPD_INFO_ADD);
        rc = o_evt->info.vip_evt.info.errEvt.err;

        m_MMGR_FREE_IFSV_EVT(o_evt);

        return rc;
    }

    if ( o_evt->type == IFD_IP_FROM_STALE_ENTRY_RESP) 
    {
        /* Form the response event and send IP to IfND */
        memset(&respEvt,0,sizeof(IFSV_EVT));

        respEvt.type = IFND_IP_FROM_STALE_ENTRY_RESP;

        respEvt.info.vip_evt.info.staleIp.ipAddr.ipaddr.type =
                  o_evt->info.vip_evt.info.staleIp.ipAddr.ipaddr.type;
        respEvt.info.vip_evt.info.staleIp.ipAddr.ipaddr.info.v4 =
                  o_evt->info.vip_evt.info.staleIp.ipAddr.ipaddr.info.v4;
        respEvt.info.vip_evt.info.staleIp.ipAddr.mask_len =
                  o_evt->info.vip_evt.info.staleIp.ipAddr.mask_len;

        /* send mds response to ifa */
        rc =  ifsv_mds_send_rsp(cb->my_mds_hdl,
                           NCSMDS_SVC_ID_IFND,&evt->sinfo,
                           &respEvt);
        m_MMGR_FREE_IFSV_EVT(o_evt);
        if (rc != NCSCC_RC_SUCCESS)
        {
           m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "MDS Sync send failed from IfND to IfA :ifnd_ifa_proc_get_ip_from_stale_entry "," ");
        }
        return rc;
    }
   
    /* Return SUITABLE error code */
    return NCSCC_RC_SUCCESS;

}


/************************************************************
* Insert Functions here.
************************************************************/
/**************************************************************
* code to process IFA_VIPD_INFO_ADD request from IfA
* Called From: 
* Return values : ERROR CODES
* NOTES :  This function is called when ifnd receives an
*          event from IfA for addition of Virtual IPAddress 
*          information to the Virtual IP Address database.
**************************************************************/
uns32 ifnd_ifa_proc_vipd_info_add(IFSV_CB *cb, IFSV_EVT *evt)
{
    uns32                     rc = NCSCC_RC_SUCCESS;
    uns32                     ipChkRes;
    uns32                     index;
    NCS_IFSV_VIP_INT_HDL      vipHandle;
    IFSV_EVT                  sendIfdEvt;
    IFSV_EVT                  sendEvt;
    IFSV_IFND_VIPDC          *pVipdcEntry = IFSV_NULL;
    IFSV_EVT                 *pRcvIfdEvt  = IFSV_NULL;
    NCS_PATRICIA_NODE        *pPatEntry   = IFSV_NULL;
    NCS_DB_LINK_LIST_NODE    *ifaOwner    = IFSV_NULL;
    NCS_DB_LINK_LIST_NODE    *ipNode      = IFSV_NULL;
    NCS_DB_LINK_LIST_NODE    *intfNode    = IFSV_NULL;

    uns8                      intfStr[15];

    if ( cb == NULL || evt == NULL)
         return NCSCC_RC_VIP_INTERNAL_ERROR;

    if ( cb->ifd_card_up != TRUE)
    {
      return  NCSCC_RC_VIP_RETRY; 
    }
 
    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_RECEIVED_IFA_VIPD_INFO_ADD_REQ);

   /* form a Key to check into the VIPDC Table */

    memset(&vipHandle,0,sizeof(NCS_IFSV_VIP_INT_HDL));

    m_NCS_STRCPY(&vipHandle.vipApplName,evt->info.vip_evt.info.ifaVipAdd.handle.vipApplName);
    vipHandle.poolHdl     = evt->info.vip_evt.info.ifaVipAdd.handle.poolHdl;
    vipHandle.ipPoolType     = evt->info.vip_evt.info.ifaVipAdd.handle.ipPoolType;

    pPatEntry = ifsv_vipd_vipdc_rec_get(&cb->vipDBase, 
                        (NCS_IFSV_VIP_INT_HDL *)&evt->info.vip_evt.info.ifaVipAdd.handle);

    if (pPatEntry == IFSV_NULL)
    {
         /*  LOG: VIPDC LOOKUP FAILED */
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_VIPDC_LOOKUP_FAILED);

         /* Checking if the IP already exists for another application */
         ipChkRes = ncs_vip_check_ip_exists(
                          cb, evt->info.vip_evt.info.ifaVipAdd.handle.poolHdl,
                          evt->info.vip_evt.info.ifaVipAdd.ipAddr.ipaddr.info.v4,
                          (uns8 *)&evt->info.vip_evt.info.ifaVipAdd.intfName);

         if (ipChkRes == NCSCC_VIP_EXISTS_ON_OTHER_INTERFACE || 
                 ipChkRes == NCSCC_VIP_EXISTS_FOR_OTHER_HANDLE ||
                 ipChkRes == NCSCC_RC_IP_EXISTS_BUT_NOT_VIP  ||
                 ipChkRes == NCSCC_RC_VIP_INTERNAL_ERROR ) 
         {
             m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                                 IFSV_VIP_REQUESTED_VIP_ALREADY_EXISTS);
             return ipChkRes;
         }
         /* Check whether record for requested interface exists */
         index =  ifsv_vip_get_global_ifindex(cb,
                          evt->info.vip_evt.info.ifaVipAdd.intfName);

         if (index == 0)
             return NCSCC_RC_VIP_INTERNAL_ERROR;
    

         /* Since there is no vipdc record found, send a vipd add request to IfD */
         memset(&sendIfdEvt, 0, sizeof(IFSV_EVT));

         sendIfdEvt.type = IFND_VIPD_INFO_ADD_REQ;

         /* Forming a IfD event and sending sync mds message to IfD */
         m_NCS_STRCPY(&sendIfdEvt.info.vip_evt.info.ifndVipAdd.handle.vipApplName,
                          &evt->info.vip_evt.info.ifaVipAdd.handle.vipApplName);

         sendIfdEvt.info.vip_evt.info.ifndVipAdd.handle.poolHdl = 
                           evt->info.vip_evt.info.ifaVipAdd.handle.poolHdl;

         sendIfdEvt.info.vip_evt.info.ifndVipAdd.handle.ipPoolType =
                           evt->info.vip_evt.info.ifaVipAdd.handle.ipPoolType;
    
         sprintf(intfStr,"NODE%d:%s",cb->my_node_id,
                            (uns8 *)&evt->info.vip_evt.info.ifaVipAdd.intfName);
/*
         m_NCS_STRCPY(&sendIfdEvt.info.vip_evt.info.ifndVipAdd.intfName,
                           &evt->info.vip_evt.info.ifaVipAdd.intfName);
*/
         m_NCS_STRCPY(&sendIfdEvt.info.vip_evt.info.ifndVipAdd.intfName,
                       intfStr);

         sendIfdEvt.info.vip_evt.info.ifndVipAdd.ipAddr.ipaddr.info.v4 =
                           evt->info.vip_evt.info.ifaVipAdd.ipAddr.ipaddr.info.v4;

         sendIfdEvt.info.vip_evt.info.ifndVipAdd.ipAddr.ipaddr.type =
                           evt->info.vip_evt.info.ifaVipAdd.ipAddr.ipaddr.type;

         sendIfdEvt.info.vip_evt.info.ifndVipAdd.ipAddr.mask_len = 
                           evt->info.vip_evt.info.ifaVipAdd.ipAddr.mask_len;

         sendIfdEvt.info.vip_evt.info.ifndVipAdd.infraFlag =
                           evt->info.vip_evt.info.ifaVipAdd.infraFlag;
    

         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_SENDING_IFND_VIPD_INFO_ADD_REQ);

         /* Sending event to IfD */
         rc = ifsv_mds_msg_sync_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                 NCSMDS_SVC_ID_IFD, cb->ifd_dest, &sendIfdEvt, &pRcvIfdEvt,
                 IFSV_MDS_SYNC_TIME);

         if (rc != NCSCC_RC_SUCCESS || pRcvIfdEvt == IFSV_NULL)
         {
            /* LOG ERROR MESSAGE */
            m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "MDS Sync send failed from IfND to IfD :ifnd_ifa_proc_vipd_info_add "," ");
            m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_INTERNAL_ERROR);
            return NCSCC_RC_VIP_RETRY;
         }

         if (pRcvIfdEvt->type == IFSV_VIP_ERROR)
         {
            /* TBD : Check for the error */
            m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Received Error Evt from IfD :ifnd_ifa_proc_vipd_info_add "," ");
            m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                                IFSV_VIP_RECEIVED_ERROR_RESP_FOR_VIPD_INFO_ADD);
            rc = pRcvIfdEvt->info.vip_evt.info.errEvt.err;
            m_MMGR_FREE_IFSV_EVT(pRcvIfdEvt);   
            
            return rc;
         }
         /* this is event rcd from IfD and no use here after */
         m_MMGR_FREE_IFSV_EVT(pRcvIfdEvt);   

         /* VIPD entry has been added into VIPD. Now add an entry into VIPDC */

         pVipdcEntry = m_MMGR_ALLOC_IFSV_VIPDC_REC ;

         if(pVipdcEntry == IFSV_NULL)
         {
            /* LOG: MEMORY ALLOC FAILED */
            m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                                IFSV_VIP_MEM_ALLOC_FAILED);
            return NCSCC_RC_VIP_INTERNAL_ERROR;
         }

         /* Forming a VIPDC record entry */
         memset(pVipdcEntry,0,sizeof(IFSV_IFND_VIPDC));

         m_NCS_STRCPY(&pVipdcEntry->handle.vipApplName,
                      &evt->info.vip_evt.info.ifaVipAdd.handle.vipApplName);

         pVipdcEntry->handle.poolHdl =
                       evt->info.vip_evt.info.ifaVipAdd.handle.poolHdl;

         pVipdcEntry->handle.ipPoolType =
                       evt->info.vip_evt.info.ifaVipAdd.handle.ipPoolType;

         /* Since this is the initial entry, ref cnt will be 1 */
         pVipdcEntry->ref_cnt = 1;   

         /* Set the allocated and complete flags in the entry flasgs attribute */
         m_IFSV_VIP_ALLOCATED_SET(pVipdcEntry->vip_entry_attr);
         m_IFSV_VIP_ENTRY_COMPLETE_SET(pVipdcEntry->vip_entry_attr);

         /* Since this is the first time,
            Initialize all the lists associated with that record  */

         rc = ifsv_vip_init_ip_list(&pVipdcEntry->ip_list);
         if (rc != NCSCC_RC_SUCCESS)
         {
            /* Free the record in all the failed cases  */
            m_MMGR_FREE_IFSV_VIPDC_REC(pVipdcEntry);
            return NCSCC_RC_VIP_INTERNAL_ERROR;
         }
         rc = ifsv_vip_init_intf_list(&pVipdcEntry->intf_list);
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_MMGR_FREE_IFSV_VIPDC_REC(pVipdcEntry);
            return NCSCC_RC_VIP_INTERNAL_ERROR;
         }
         rc = ifsv_vip_init_owner_list(&pVipdcEntry->owner_list);
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_MMGR_FREE_IFSV_VIPDC_REC(pVipdcEntry);
            return NCSCC_RC_VIP_INTERNAL_ERROR;
         }

         /* Add IP NODE, INTF NODE and OWNER NODE to the respective lists */
         rc = ifsv_vip_add_ip_node(&pVipdcEntry->ip_list,
                                   &evt->info.vip_evt.info.ifaVipAdd.ipAddr,
                                   (uns8 *)&evt->info.vip_evt.info.ifaVipAdd.intfName);
         {
            if ( rc != NCSCC_RC_SUCCESS) 
            {
               m_MMGR_FREE_IFSV_VIPDC_REC(pVipdcEntry);
               return NCSCC_RC_VIP_INTERNAL_ERROR;
            }

               /* LOG: ADDED IP NODE TO VIPDC CACHE */
            m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_ADDED_IP_NODE_TO_VIPDC);
         }

         /* Adding Intf node to the intf list of the vipd cache*/
         rc = ifsv_vip_add_intf_node (&pVipdcEntry->intf_list,
                                      intfStr);
         {
            if ( rc != NCSCC_RC_SUCCESS) 
            {
               /* LOG ERROR MESSAGE */
               ifsv_vip_free_ip_list(&pVipdcEntry->ip_list);
               m_MMGR_FREE_IFSV_VIPDC_REC(pVipdcEntry);
               return NCSCC_RC_VIP_INTERNAL_ERROR;
            } 

            m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_ADDED_INTERFACE_NODE_TO_VIPDC);
               /* LOG: ADDED INTERFACE NODE TO VIPDC CACHE */
         }
         
         /* SWITCHOVER : REMOVE SUPPORT FLAG */
         /* Adding owner node to the owner list */  
         rc = ifsv_vip_add_owner_node(&pVipdcEntry->owner_list,
                                      &evt->sinfo.dest
                                      ,evt->info.vip_evt.info.ifaVipAdd.infraFlag
                                     );
         {
            if ( rc != NCSCC_RC_SUCCESS)
            {
               ifsv_vip_free_ip_list(&pVipdcEntry->ip_list);
               ifsv_vip_free_intf_list(&pVipdcEntry->intf_list);
               m_MMGR_FREE_IFSV_VIPDC_REC(pVipdcEntry);
               return NCSCC_RC_VIP_INTERNAL_ERROR;
            }

               /* LOG: ADDED OWNER NODE TO VIPDC CACHE */
            m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_ADDED_OWNER_NODE_TO_VIPDC);
         }

         pPatEntry = (NCS_PATRICIA_NODE *)pVipdcEntry;

         pPatEntry->key_info = (uns8 *)&pVipdcEntry->handle;    
         /* Here we have formed the vipdc cache record. 
            Now add it to the main database */
         rc = ifsv_vipd_vipdc_rec_add(&cb->vipDBase,pPatEntry);
                      

         if(rc != NCSCC_RC_SUCCESS)
         {
            /* LOG AN ERROR MESSAGE */
            /* Free all the Lists added above */
            /* LOG : RECORD ADDITION TO VIPDC FAILED */
            m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                                IFSV_VIP_RECORD_ADDITION_TO_VIPDC_FAILURE);
            ifsv_vip_free_ip_list(&pVipdcEntry->ip_list);
            ifsv_vip_free_intf_list(&pVipdcEntry->intf_list);
            ifsv_vip_free_owner_list(&pVipdcEntry->owner_list);
            m_MMGR_FREE_IFSV_VIPDC_REC(pVipdcEntry);
            return NCSCC_RC_VIP_INTERNAL_ERROR;
         }
 
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                             IFSV_VIP_RECORD_ADDITION_TO_VIPDC_SUCCESS);
 
         if(ipChkRes == NCSCC_RC_FAILURE)
         {
            /* Get the Global index for the given interface */
            index =  ifsv_vip_get_global_ifindex(cb,
                     evt->info.vip_evt.info.ifaVipAdd.intfName);

            m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                             IFSV_VIP_SENDING_IPXS_ADD_IP_REQ);
            
            /* FORM AN IPXS EVENT FOR ADDING AN IPXS ENTRY */
            rc = ifsv_vip_send_ipxs_evt(cb,
                       IFSV_VIP_IPXS_ADD,
                       index,
                       &evt->info.vip_evt.info.ifaVipAdd.ipAddr, 
                       evt->info.vip_evt.info.ifaVipAdd.handle.poolHdl,
                       evt->info.vip_evt.info.ifaVipAdd.handle.ipPoolType,
                       (uns8 *)&evt->info.vip_evt.info.ifaVipAdd.intfName);
         }
         else 
         {
            /* Form IPXS event for updating the existing IPXS table */
            index =  ifsv_vip_get_global_ifindex(cb,
                                  (uns8 *)&evt->info.vip_evt.info.ifaVipAdd.intfName);
            m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                             IFSV_VIP_SENDING_IPXS_INC_REFCNT_REQ);
            rc = ifsv_vip_send_ipxs_evt(cb,IFSV_VIP_IPXS_INC_REFCNT,
                                        index,
                                        &evt->info.vip_evt.info.ifaVipAdd.ipAddr,
                                        0,0,
                                        (uns8 *)&evt->info.vip_evt.info.ifaVipAdd.intfName); 
         }

    } /* end of if pPatEntry == NULL */
    else  /* IF entry already exists in VIPD cache */
    {
       /* VIPDC LOOKUP SUCCESS */
       pVipdcEntry = (IFSV_IFND_VIPDC *)pPatEntry;

       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                      IFSV_VIP_VIPDC_LOOKUP_SUCCESS );
       
       /* Check if the request is from the same IfA */
       ifaOwner = ncs_db_link_list_find(&pVipdcEntry->owner_list,(uns8 *)&evt->sinfo.dest);
       if (ifaOwner != IFSV_NULL) /* Implies we received the request from same ifa */
       {
           /* RECIVED REQUEST FROM SAME IFA */
           m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                      IFSV_VIP_RECEIVED_REQUEST_FROM_SAME_IFA);

           ipNode = ncs_db_link_list_find(&(pVipdcEntry->ip_list),
                           (uns8 *)&evt->info.vip_evt.info.ifaVipAdd.ipAddr);
           if (ipNode != IFSV_NULL )
              return NCSCC_RC_IP_ALREADY_EXISTS;
           else
              return NCSCC_VIP_HDL_IN_USE_FOR_DIFF_IP;
       }
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                  IFSV_VIP_RECEIVED_REQUEST_FROM_DIFFERENT_IFA);
      /* IF control comes here it means that,
         we received the add request from a different IfA */

       ipNode = ncs_db_link_list_find(&(pVipdcEntry->ip_list),
                     (uns8 *)&evt->info.vip_evt.info.ifaVipAdd.ipAddr);


         /* SWITCHOVER : REMOVE SUPPORT FLAG */
       if (ipNode != IFSV_NULL)
       {
          /* Check whether the ip is requested for the existing interface or not */
         
          sprintf(intfStr,"NODE%d:%s",cb->my_node_id,
                            (uns8 *)&evt->info.vip_evt.info.ifaVipAdd.intfName);

          intfNode = ncs_db_link_list_find(&pVipdcEntry->intf_list,
                     (uns8 *)&intfStr);

          if (intfNode == IFSV_NULL)
          {
             return  NCSCC_VIP_EXISTS_ON_OTHER_INTERFACE; 
          }
          else  /* Remove else part when we support multiple applications
                   using same application name for installing
                   already existing ip address */  
          {
             return NCSCC_VIP_APPL_NAME_ALREADY_IN_USE_FOR_SAME_IP;
          }

          /* Add IfA to Owner List */
          rc = ifsv_vip_add_owner_node(&pVipdcEntry->owner_list,
                                       &evt->sinfo.dest
                                       ,evt->info.vip_evt.info.ifaVipAdd.infraFlag
                                      );
          if ( rc != NCSCC_RC_SUCCESS)
             return NCSCC_RC_VIP_INTERNAL_ERROR;

          /* Increment the refrence count */
          pVipdcEntry->ref_cnt = pVipdcEntry->ref_cnt + 1;

          /* LOG: ADDED OWNER NODE TO VIPDC CACHE */
          m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND, IFSV_VIP_ADDED_OWNER_NODE_TO_VIPDC);
       }

    }  /* end of else */

      /* send a IFND_IP_INSTALL event to ifa */
    memset(&sendEvt, 0, sizeof(IFSV_EVT));
    sendEvt.type = IFND_VIPD_INFO_ADD_REQ_RESP;
    sendEvt.info.vip_evt.info.ifndVipAddResp.err = NCSCC_IFND_VIP_INSTALL;

    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                IFSV_VIP_SENDING_IFND_VIPD_INFO_ADD_REQ_RESP);

    /* Sync resp */
    rc = ifsv_mds_send_rsp(cb->my_mds_hdl,
                           NCSMDS_SVC_ID_IFND, &evt->sinfo,
                           &sendEvt);
    if (rc != NCSCC_RC_SUCCESS)
    {
        m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
         "MDS Sync send failed to IfA from IfND :ifnd_ifa_proc_vipd_info_add "," ");
    }

    return rc;

}

/**************************************************************
* End of code to process IFA_VIPD_INFO_ADD request from IfA
**************************************************************/


/**************************************************************
* code to process IFA_VIP_FREE request from IfA
**************************************************************/
uns32   ifnd_ifa_proc_vip_free(IFSV_CB *cb,IFSV_EVT  *pEvt)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFSV_EVT                sendEvt,*rcvEvt = NULL;
    IFSV_IFND_VIPDC        *pVipdc;
    NCS_PATRICIA_NODE      *pPatEntry;
    NCS_DB_LINK_LIST_NODE  *pIpNode;
    NCS_IFSV_VIP_IP_LIST   *pIpList;
    NCS_DB_LINK_LIST_NODE  *tmpPtr;
    NCS_DB_LINK_LIST_NODE  *ifaOwner = IFSV_NULL;
    NCS_DB_LINK_LIST_NODE      *pNode;
    NCS_IFSV_VIP_OWNER_LIST    *pOwnerNode;

    printf("received free req in ifnd_vipproc.c \n");    
    if ( cb == IFSV_NULL || pEvt == IFSV_NULL)
    {
       /* LOG ERROR MESSAGE */
       return NCSCC_RC_VIP_INTERNAL_ERROR;
    }

    if ( cb->ifd_card_up != TRUE)
    {
      printf("ifd_card_up flag says ifd is not active \n");
      return  NCSCC_RC_VIP_RETRY;
    }

    printf("ifd_card_up flag says ifd is active \n");

    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
               IFSV_VIP_RECEIVED_IFA_VIP_FREE_REQ);

    pPatEntry = ifsv_vipd_vipdc_rec_get(&cb->vipDBase,
                           (NCS_IFSV_VIP_INT_HDL *)&pEvt->info.vip_evt.info.ifaVipFree.handle);
    if (pPatEntry == IFSV_NULL)
    {
       /* Log: LOOKUP FAILED IN VIPDC */
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_VIPDC_LOOKUP_FAILED);
       return NCSCC_RC_INVALID_PARAM;
    }
    pVipdc = (IFSV_IFND_VIPDC *)pPatEntry;

    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                   IFSV_VIP_VIPDC_LOOKUP_SUCCESS );

    if(pVipdc->ref_cnt > 1)
    {
         /* SWITCHOVER : MODIFY LOGIC ACCORDING TO DIFF FLAG */
       /* Check if the free request is from TERMINATE script */
       /* In that case the infra flag will be set and MDS destinations */
       /* doesn't match. so check for the owner with infrastructural flag*/
       /* in the owner list and remove that owner from the list*/
       if(pEvt->info.vip_evt.info.ifaVipFree.infraFlag == VIP_ADDR_SPECIFIC)
       {
          pNode = ifsv_vip_search_infra_owner(&pVipdc->owner_list);
          if (pNode == IFSV_NULL)
          {
             return NCSCC_RC_INVALID_PARAM;
          }
          pOwnerNode = (NCS_IFSV_VIP_OWNER_LIST *)pNode;
          rc = ncs_db_link_list_del(&pVipdc->owner_list,(uns8 *)&pOwnerNode->owner);
          if (rc == NCSCC_RC_FAILURE)
          {
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,
                                  IFSV_VIP_LINK_LIST_NODE_DELETION_FAILED);
             /* Check for sending error in the event*/
             return NCSCC_RC_VIP_INTERNAL_ERROR;
          }
          pVipdc->ref_cnt--;
       }
       else
       {       
          /* If ref count is more than 1, then we need not free the Virtual IP */
          /* removeing the owner from the owners list */
          rc = ncs_db_link_list_del(&pVipdc->owner_list,(uns8 *)&pEvt->sinfo.dest);
          if (rc == NCSCC_RC_FAILURE)
          {
             /* Check for sending error in the event*/      
             return NCSCC_RC_VIP_INTERNAL_ERROR;
          }      
          pVipdc->ref_cnt--;

       }/*end of else */
       /* Send a success msg to ifa */
    } else {
       /* SWITCHOVER : MODIFY ACCORDING TO DIFF FLAG */
       /* Check if the free request is from TERMINATE script */
       /* In that case the infra flag will be set and MDS destinations */
       /* doesn't match. so check for the owner with infrastructural flag*/
       /* in the owner list and remove that owner from the list*/
       if(pEvt->info.vip_evt.info.ifaVipFree.infraFlag == VIP_ADDR_SPECIFIC)
       {
          pNode = ifsv_vip_search_infra_owner(&pVipdc->owner_list);
          if (pNode == IFSV_NULL)
          {
             return NCSCC_RC_INVALID_PARAM;
          }
          pOwnerNode = (NCS_IFSV_VIP_OWNER_LIST *)pNode;
          if(pOwnerNode->infraFlag == 0)
          {
             return NCSCC_RC_INVALID_PARAM;
          }
       }
       else 
      {
           /* Check if application is owner of IP */
          ifaOwner = ncs_db_link_list_find(&pVipdc->owner_list,(uns8 *)&pEvt->sinfo.dest);
          if (ifaOwner == IFSV_NULL) /* Implies we received the request from same ifa */
          {
             return NCSCC_RC_APPLICAION_NOT_OWNER_OF_IP;
          }
       }
       /* If the ref cnt is not > 1, delete all the Ip addresses from the list and remove the record */
       pIpNode = m_NCS_DBLIST_FIND_FIRST(&pVipdc->ip_list);
       if (pIpNode == IFSV_NULL)
       {
          return NCSCC_RC_VIP_INTERNAL_ERROR;
       }
       while(pIpNode)
       {
          tmpPtr = pIpNode;
          pIpList = (NCS_IFSV_VIP_IP_LIST *)pIpNode;
          /* Delete the IP address by traversing through IPXS table */
          rc = ncs_ifsv_vip_del_ip(cb,pVipdc->handle.poolHdl,&pIpList->ip_addr);
          pIpNode = m_NCS_DBLIST_FIND_NEXT(tmpPtr);
       }

       /* Check for the Nature of Free request. If it is for switchover
          or for normal free */

       memset(&sendEvt,0,sizeof(IFSV_EVT));
 
       if(pEvt->info.vip_evt.info.ifaVipFree.infraFlag ==  VIP_ADDR_NORMAL)
       {
           /* form IFND_VIP_FREE event and send to IFD */
           sendEvt.type = IFND_VIP_FREE_REQ;
       }
       else 
       {
           sendEvt.type = IFND_VIP_MARK_VIPD_STALE;
       }
       m_NCS_STRCPY(&sendEvt.info.vip_evt.info.ifndVipFree.handle.vipApplName,
                    &pEvt->info.vip_evt.info.ifaVipFree.handle.vipApplName);

       sendEvt.info.vip_evt.info.ifndVipFree.handle.poolHdl = 
                             pEvt->info.vip_evt.info.ifaVipFree.handle.poolHdl;

       sendEvt.info.vip_evt.info.ifndVipFree.handle.ipPoolType = 
                          pEvt->info.vip_evt.info.ifaVipFree.handle.ipPoolType;
       
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                   IFSV_VIP_SENDING_IFND_VIP_FREE_REQ );

       rc = ifsv_mds_msg_sync_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                NCSMDS_SVC_ID_IFD, cb->ifd_dest, &sendEvt, &rcvEvt,
                IFSV_MDS_SYNC_TIME);

       if (rc != NCSCC_RC_SUCCESS || rcvEvt == IFSV_NULL) /* IF VIPD is not cleared */
       {
          /* LOG ERROR MESSAGE */
           m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "MDS Sync send failed from IfND to IfD :ifnd_ifa_proc_vip_free "," ");
          return NCSCC_RC_VIP_RETRY;
       }
       if (rcvEvt->type != IFSV_VIP_ERROR)
       {
          /* TBD : LOG MESSAGE */
          m_MMGR_FREE_IFSV_EVT(rcvEvt);   
       }
       if (rcvEvt->type == IFSV_VIP_ERROR)
       {
          /* Fill the correct error value from recieved event */
           m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "Received Error Evt :ifnd_ifa_proc_vip_free "," ");
          rc = rcvEvt->info.vip_evt.info.errEvt.err; 
          m_MMGR_FREE_IFSV_EVT(rcvEvt);
          return rc;
       }

       /* VIPD Record is removed, Now remove the VIPDC record */
       rc = ifsv_vipd_vipdc_rec_del(&cb->vipDBase,(NCS_PATRICIA_NODE *)pVipdc,IFSV_VIP_REC_TYPE_VIPDC);

       if (rc == NCSCC_RC_FAILURE) /* IF VIPDC is not cleared */
       {
           m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "VIPDC record deletion failed :ifnd_ifa_proc_vip_free "," ");
          m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_VIPDC_RECORD_DELETION_FAILED);
          /* LOG ERROR MESSAGE */
          return NCSCC_RC_VIP_INTERNAL_ERROR;
       }
   } /* end of else pvipdc->ref_cnt > 1 */
      /* Sync resp */
   memset(&sendEvt,0,sizeof(IFSV_EVT));
   sendEvt.type = IFND_VIP_FREE_REQ_RESP;

   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
               IFSV_VIP_SENDING_IFND_VIP_FREE_REQ_RESP );

   rc = ifsv_mds_send_rsp(cb->my_mds_hdl,
                         NCSMDS_SVC_ID_IFND, &pEvt->sinfo,
                         &sendEvt);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
       "MDS Sync send failed to IfA from IfND :ifnd_ifa_proc_vip_free "," ");
   }
    
   return rc;
}




 /***************************************************************
* IFND CODE : EVENT PROCESSING
***************************************************************/

/* this function will be called from ifnd event process code */
uns32 ifnd_vip_evt_process(IFSV_CB *cb, IFSV_EVT *evt)
 {
   uns32    rc = NCSCC_RC_SUCCESS;
   IFSV_EVT errEvt;
 
   /* LOG: RECEIVED VIP EVENT */
   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND, IFSV_VIP_RECEIVED_VIP_EVENT);

   switch(evt->type)
   {
       case IFA_VIPD_INFO_ADD_REQ:
          rc = ifnd_ifa_proc_vipd_info_add(cb,evt);
          break;
       case IFA_VIP_FREE_REQ:
          rc = ifnd_ifa_proc_vip_free(cb,evt);
          break;
       case IFND_IFND_VIP_DEL_VIPD:
          rc = ifnd_proc_ifnd_vip_del_vipd(cb,evt); 
          break;
       case IFND_VIP_MARK_VIPD_STALE:
          rc = ifnd_proc_vip_mark_vipd_stale(cb,evt);
          break;
       case IFA_GET_IP_FROM_STALE_ENTRY_REQ:
          rc = ifnd_ifa_proc_get_ip_from_stale_entry(cb,evt); 
          break;
       default:
          rc = NCSCC_RC_FAILURE;
          break;
   }

   if (rc != NCSCC_RC_SUCCESS && rc != NCSCC_RC_FAILURE)
   {
      memset(&errEvt,0,sizeof(IFSV_EVT));
      errEvt.type = IFSV_VIP_ERROR;
      printf("Sending Error Evt to ifa. Error Number %d \n",rc);
      errEvt.info.vip_evt.info.errEvt.err = rc;

      /* Send an error event to the ifa waiting for response */
      rc =  ifsv_mds_send_rsp(cb->my_mds_hdl,
                             NCSMDS_SVC_ID_IFND,&evt->sinfo,
                             &errEvt);
   }
   return rc;
}


/************************************************************************
* Code for Processing IfA Crash Flow Scenario
* NOTES : This piece of code goes into ifnd_same_dst_intf_rec_del function in ifndproc.c file.
************************************************************************/

uns32 ifnd_process_ifa_crash(IFSV_CB *cb, MDS_DEST *mdsDest)
{
   NCS_IFSV_VIP_INT_HDL      key;
   uns32                     rc;
   NCS_PATRICIA_NODE        *vipdcNode;
   IFSV_IFND_VIPDC          *vipdcRec;
   NCS_DB_LINK_LIST_NODE    *ownerNode;
   NCS_DB_LINK_LIST_NODE    *ipAddrNode,*tmpAddrNode;
   NCS_IFSV_VIP_IP_LIST     *pIpList ;
   NCS_IFSV_VIP_OWNER_LIST  *ownerInfo;
   
   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
               IFSV_VIP_RECEIVED_IFA_CRASH_MSG );

   /* Traverse the VIPD Cache tree for finding out the records belonging to crashed IfA */
   memset(&key,0,sizeof(NCS_IFSV_VIP_INT_HDL));
   vipdcNode = ncs_patricia_tree_getnext(&cb->vipDBase,(uns8 *)0);

   if (vipdcNode == NULL) {
     /* LOG ERROR MESSAGE */
      return NCSCC_RC_FAILURE;
   }
   vipdcRec = (IFSV_IFND_VIPDC *)vipdcNode;
   while (vipdcNode != IFSV_NULL)
   {
      memcpy(&key,&vipdcRec->handle,sizeof(NCS_IFSV_VIP_INT_HDL));
      if (vipdcRec->ref_cnt > 1) /* if more than 1 IfA is accessing the record */
      {
         /* Remove the owner node from the list and reduce the reference count */
         ownerNode = ncs_db_link_list_find(&vipdcRec->owner_list,(uns8 *) mdsDest);
         if(ownerNode != IFSV_NULL)
         {
         /* SWITCHOVER : REMOVE THE FLAG AND MODIFY CODE ACCORDINGLY */
            ownerInfo = (NCS_IFSV_VIP_OWNER_LIST *)ownerNode;
            if (ownerInfo->infraFlag == VIP_ADDR_SPECIFIC)
            {
               /* Don't do anything. leave the entry as it is */
               /* Since that is an infrastructural entry */
               /* continue;*/
            }
            else
            {       
                ncs_db_link_list_del(&vipdcRec->owner_list,(uns8 *)mdsDest);
                vipdcRec->ref_cnt--;
            }
         }
      }
      else /* if vipdcRec->ref_cnt == 0 */
      {
         ownerNode = ncs_db_link_list_find(&vipdcRec->owner_list,(uns8 *)mdsDest);
         if (ownerNode != IFSV_NULL)
         {
         /* SWITCHOVER : REMOVE THE FLAG AND MODIFY CODE ACCORDINGLY */
            ownerInfo = (NCS_IFSV_VIP_OWNER_LIST *)ownerNode;
            if (ownerInfo->infraFlag == VIP_ADDR_SPECIFIC)
            {
               /* Don't do anything. Leave the entry as it is */
               /* Since that is an infrastructural entry */
               /*  continue; */
            }
            else
            {         
               ipAddrNode = m_NCS_DBLIST_FIND_FIRST(&vipdcRec->ip_list);
               while(ipAddrNode)
               {
                  tmpAddrNode = ipAddrNode;
               
                  pIpList = (NCS_IFSV_VIP_IP_LIST *)ipAddrNode;
                  /* Delete the IP address by traversing through IPXS table */
                  rc = ncs_ifsv_vip_del_ip(cb,vipdcRec->handle.poolHdl,&pIpList->ip_addr);
                  /* Here we need to start a timer */
                  ipAddrNode = m_NCS_DBLIST_FIND_NEXT(tmpAddrNode);

                  if (ipAddrNode == IFSV_NULL)
                  {
                        /* Here we need to mark the vipd entry as stale. So form the ifnd
                           event and send it to IfD for marking entry as stale */
                                   
                        ifnd_create_mark_vip_entry_stale_evt(cb,vipdcRec->handle.vipApplName ,
                                                    vipdcRec->handle.poolHdl,
                                                  vipdcRec->handle.ipPoolType );
/*
                     * Form IFND_VIP_DEL_VIPD evt and post it to ifnd mail box 
                        This is to delete VIPData base records at later point of time *

                        ifnd_create_ifnd_evt(cb,vipdcRec->handle.vipApplName ,
                                                    vipdcRec->handle.poolHdl,
                                                  vipdcRec->handle.ipPoolType );
*/
                  }   /* end of if ipAddrNode == NULL */
               }      /* end of while (ipAddrNode ) */
            }
         }         /* end of If ownerNode != IFSV_NULL */
      }            /* end of else if vipdcRec->ref_cnt == 0 */
      vipdcNode = ncs_patricia_tree_getnext(&cb->vipDBase,(uns8 *)&key);
      vipdcRec = (IFSV_IFND_VIPDC *)vipdcNode;
      memset(&key,0,sizeof(NCS_IFSV_VIP_INT_HDL));
   } /* end of while vipdcNode != NULL */
   return NCSCC_RC_SUCCESS;
}


#endif
