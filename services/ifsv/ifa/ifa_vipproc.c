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



/***************************************************************************
FILE NAME:    VIPPROC.C
DESCRIPTION:  IP ADDRESS VIRTUALIZATION ROUTINES.
***************************************************************************/


#if (NCS_VIP == 1)
#include "ifa.h"
#include <net/if.h>

#define    VIP_MAX_HDL_VAL   (uns32)429496729

/****************************************************************************
 * Name          : ncs_vip_ip_lib_init
 *
 * Description   : This function is called by IFA init function. This function initializes IFA_DB tree.
 *
 * Arguments     : cb - IFA Control Block
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : This implementation would be used in phase-11 .
 *****************************************************************************/

          
uns32 ncs_vip_ip_lib_init (IFA_CB *cb)
{
   NCS_PATRICIA_PARAMS       params;
   uns32                     res = NCSCC_RC_SUCCESS;

   memset(&params,0,sizeof(NCS_PATRICIA_PARAMS));
   params.key_size = sizeof(NCS_IFSV_VIP_INT_HDL); /* check this statement */
   params.info_size = 0;

   if ((ncs_patricia_tree_init(&cb->ifaDB, &params))
         != NCSCC_RC_SUCCESS)
      {  
         m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncs_patricia_tree_init() returned failure"," ");
         /* log message for failure */
         res = NCSCC_RC_FAILURE;
      }
   return res;
}
    

/*********************************************************************
* Code Used for validating the given ip address
*********************************************************************/
uns32  ncs_ifsv_vip_verify_ipaddr(char *ipaddr)
{
   uns32 temp_ipaddr;
   temp_ipaddr = inet_addr(ipaddr);
                                                                                                                              
   if(temp_ipaddr == -1)
   {
      return NCSCC_RC_FAILURE;
   }
                                                                                                                              
   if(((temp_ipaddr & 0xff000000) == 0) ||
     ((temp_ipaddr & 0x000000ff) == 0) ||
     ((temp_ipaddr & 0xff000000) == 0xff000000) ||
     ((temp_ipaddr & 0x00ff0000) == 0xff0000) ||
     ((temp_ipaddr & 0x0000ff00) == 0xff00) ||
     ((temp_ipaddr & 0x000000ff) == 0xff))
   {
      return NCSCC_RC_FAILURE;
   }
                                                                                                                              
   return NCSCC_RC_SUCCESS;
}


/*********************************************************************
* Code Used for validating the interface 
*********************************************************************/

/* TBD : This code needs to be moved to leap area*/
#define   m_NCS_VALIDATE_INTERFACE_NAME(str) m_ncs_validate_interface_name(str) 
  
uns32 m_ncs_validate_interface_name(char *str) {
     int sock;
     struct ifreq ifr; 
     uns32 intfRes;
     sock = socket(PF_INET,SOCK_DGRAM,0);
     if (sock == -1)
     {
        m_NCS_CONS_PRINTF("sorry coulnd't able to create a socket\n");
        intfRes = NCSCC_RC_FAILURE;
        return intfRes;
     }
     memset(&ifr, 0, sizeof(ifr));
     strcpy(&ifr.ifr_ifrn.ifrn_name,str);
     if(ioctl(sock,SIOCGIFFLAGS,&ifr) < 0)
     {
        close(sock);
        return  NCSCC_RC_INVALID_INTF;
     }
     if (ifr.ifr_ifru.ifru_flags & IFF_UP)
     {
        close(sock);
        return NCSCC_RC_SUCCESS;
     } else 
     {
        intfRes = NCSCC_RC_INTF_ADMIN_DOWN;
        close(sock);
        return intfRes;
     } 
}


/****************************************************************************
 * Name          : ncs_ifsv_vip_install
 *
 * Description   : This function is called by IFA-Single entry API on receiving install request.
 *
 * Arguments     : cb - IFA Control Block
 *                 NCS_VIP_IP_INSTALL : Install specific request info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE/NCS_VIP_INVALID_INTF/NCS_VIP_INVALID_STATE/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 
ncs_ifsv_vip_install(IFA_CB *ifa_cb , NCS_IFSV_VIP_INSTALL *instArg) 
{
    
     uns32                   res = NCSCC_RC_SUCCESS;
     uns32                   intfRes;
     IFSV_EVT               *evt=0,ipEvt;
     IFSV_EVT               *o_evt = NULL;
     uns8                    tmpIp[20];


     if (ifa_cb == IFSV_NULL || instArg == IFSV_NULL)
     {
        instArg->o_err = NCSCC_RC_VIP_INTERNAL_ERROR;
        return NCSCC_RC_FAILURE;
     }

     m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_RECIVED_VIP_INSTALL_REQUEST);
     if(instArg->i_handle.poolHdl == 0)
     {
        /* LOG: NULL POOL HANDLE */
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_NULL_POOL_HDL);
        instArg->o_err = NCSCC_RC_INVALID_VIP_HANDLE;
        return NCSCC_RC_FAILURE;
     }
     if(instArg->i_handle.poolHdl > VIP_MAX_HDL_VAL)
     {
        instArg->o_err = NCSCC_RC_INVALID_VIP_HANDLE;
        return NCSCC_RC_FAILURE;
     }
     if(strlen(instArg->i_handle.vipApplName) == 0)
     {
        /* LOG: NULL APPLICATION NAME */
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_NULL_APPL_NAME);
        instArg->o_err = NCSCC_RC_NULL_APPLNAME;
        return NCSCC_RC_FAILURE;
     }
     if(strlen(instArg->i_handle.vipApplName) > m_NCS_IFSV_VIP_APPL_NAME)
     {
        /* LOG: NULL APPLICATION NAME */
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_NULL_APPL_NAME);
        instArg->o_err = NCSCC_RC_APPLNAME_LEN_EXCEEDS_LIMIT;
        return NCSCC_RC_FAILURE;
     }
     /* validate the ip address and the interface received from application */
     intfRes = m_NCS_VALIDATE_INTERFACE_NAME(instArg->i_intf_name);  
     if(intfRes != NCSCC_RC_SUCCESS)     
     {
       m_IFA_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"m_NCS_VALIDATE_INTERFACE_NAME() returned failure and ret val is:",intfRes);
        if (intfRes == NCSCC_RC_INTF_ADMIN_DOWN) 
        {
           /* LOG : INTERFACE ADMIN DOWN */
           m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_INTERFACE_REQUESTED_ADMIN_DOWN);
           instArg->o_err = NCSCC_RC_INTF_ADMIN_DOWN;
           return NCSCC_RC_FAILURE;
        }
        else if (intfRes == NCSCC_RC_INVALID_INTF )
        {
           /* LOG: INVALID INTERFACE */
           m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_INVALID_INTERFACE_RECEIVED);
           instArg->o_err = NCSCC_RC_INVALID_INTF;
           return NCSCC_RC_FAILURE; 
        }
        else
           instArg->o_err = NCSCC_RC_VIP_INTERNAL_ERROR; 
           return NCSCC_RC_FAILURE;
     }

     /* Validating the given ip addr */

     if (instArg->i_ip_addr.ipaddr.info.v4)
     {
         if (instArg->i_ip_addr.ipaddr.type != NCS_IP_ADDR_TYPE_IPV4)
         {
            instArg->o_err = NCSCC_RC_IP_ADDR_TYPE_NOT_SUPPORTED;
            return NCSCC_RC_FAILURE;
         }

         if (instArg->i_ip_addr.mask_len > 32)
         {
            instArg->o_err = NCSCC_RC_INVALID_MASKLEN;
            return NCSCC_RC_FAILURE;
         }

         sprintf(tmpIp, "%d.%d.%d.%d", 
                              (((instArg->i_ip_addr.ipaddr.info.v4) & 0xff000000) >> 24),
                              (((instArg->i_ip_addr.ipaddr.info.v4) & 0xff0000) >> 16),
                              (((instArg->i_ip_addr.ipaddr.info.v4) & 0xff00) >> 8),
                              ((instArg->i_ip_addr.ipaddr.info.v4) & 0x000000ff)
                              );

         res = ncs_ifsv_vip_verify_ipaddr(tmpIp);
    
         if (res != NCSCC_RC_SUCCESS)
         {
         m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncs_ifsv_vip_verify_ipaddr() returned failure"," ");
             /* TBD : LOG: INVALID INTERFACE */
             /* m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_INVALID_INTERFACE_RECEIVED);*/
             instArg->o_err = NCSCC_RC_INVALID_IP;
             return NCSCC_RC_FAILURE; 
         }
     }
     else 
     {
         /* If IP address is null, It means we need to get ip 
            from already existing stale entry (VIPDB) */      

         memset(&ipEvt, 0, sizeof(IFSV_EVT));

         ipEvt.type = IFA_GET_IP_FROM_STALE_ENTRY_REQ;
 
         strcpy(&ipEvt.info.vip_evt.info.vipCommonEvt.handle.vipApplName,&instArg->i_handle.vipApplName);
         ipEvt.info.vip_evt.info.vipCommonEvt.handle.poolHdl = instArg->i_handle.poolHdl;
         ipEvt.info.vip_evt.info.vipCommonEvt.handle.ipPoolType =  NCS_IFSV_VIP_IP_INTERNAL;
        
         res = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA,
                   NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, &ipEvt, &o_evt,
                   IFA_VIP_MDS_SYNC_TIME);
                  
 
         if ( res != NCSCC_RC_SUCCESS)
         {
              m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_mds_msg_sync_send() returned failure"," ");
              /* LOG : MDS SEND FAILURE  */
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_MDS_SEND_FAILURE);
              instArg->o_err = NCSCC_RC_VIP_RETRY;
              return NCSCC_RC_FAILURE;
         }

         if (o_evt == IFSV_NULL)
         {
             return NCSCC_RC_FAILURE;
         }

         if(o_evt->type == IFSV_VIP_ERROR)
         { 
              instArg->o_err = o_evt->info.vip_evt.info.errEvt.err; 
              m_MMGR_FREE_IFSV_EVT(o_evt);
             /* Return Suitable error code */
              return NCSCC_RC_FAILURE;
         } 

         /* Assign the IP Address received to Install arguement */
         instArg->i_ip_addr.ipaddr.info.v4 = o_evt->info.vip_evt.info.staleIp.ipAddr.ipaddr.info.v4;
         instArg->i_ip_addr.ipaddr.type =  o_evt->info.vip_evt.info.staleIp.ipAddr.ipaddr.type;
         instArg->i_ip_addr.mask_len = o_evt->info.vip_evt.info.staleIp.ipAddr.mask_len;
         m_MMGR_FREE_IFSV_EVT(o_evt);
         o_evt = IFSV_NULL;

     } /* End of Else if IP = NULL) */



        evt = m_MMGR_ALLOC_IFSV_EVT;

        if (evt == IFSV_NULL)
        {
           /* LOG: MEM ALLOC FAILED */
           m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_INVALID_INTERFACE_RECEIVED);
           instArg->o_err = NCSCC_RC_VIP_INTERNAL_ERROR;
           return (NCSCC_RC_FAILURE);
        }

        memset(evt, 0, sizeof(IFSV_EVT));

        evt->type = IFA_VIPD_INFO_ADD_REQ;

        strcpy(&evt->info.vip_evt.info.ifaVipAdd.handle.vipApplName,&instArg->i_handle.vipApplName);
        evt->info.vip_evt.info.ifaVipAdd.handle.poolHdl = instArg->i_handle.poolHdl;
        evt->info.vip_evt.info.ifaVipAdd.handle.ipPoolType =  NCS_IFSV_VIP_IP_INTERNAL;
        strcpy(&evt->info.vip_evt.info.ifaVipAdd.intfName,&instArg->i_intf_name);
        evt->info.vip_evt.info.ifaVipAdd.ipAddr.ipaddr.info.v4 = instArg->i_ip_addr.ipaddr.info.v4;
        evt->info.vip_evt.info.ifaVipAdd.ipAddr.ipaddr.type = instArg->i_ip_addr.ipaddr.type;
        evt->info.vip_evt.info.ifaVipAdd.ipAddr.mask_len = instArg->i_ip_addr.mask_len;     
        
        /* SWITCHOVER: Remove SUPPORT FLAG and remove comparision */
        if (instArg->i_infraFlag == VIP_ADDR_NORMAL ||
            instArg->i_infraFlag == VIP_ADDR_SPECIFIC)
        {
            evt->info.vip_evt.info.ifaVipAdd.infraFlag = instArg->i_infraFlag;
        }
        else 
        {
            instArg->o_err = NCSCC_RC_INVALID_FLAG;
            m_MMGR_FREE_IFSV_EVT(evt);
            return NCSCC_RC_FAILURE; 
        }
 
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_SENDING_IFA_VIPD_INFO_ADD_REQ);
        
        res = ifsv_mds_msg_sync_send(ifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA,
                  NCSMDS_SVC_ID_IFND, ifa_cb->ifnd_dest, evt, &o_evt,
                  IFA_VIP_MDS_SYNC_TIME);
                  
 
        if ( res != NCSCC_RC_SUCCESS)
        {
             m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_mds_msg_sync_send() returned failure"," ");
             /* LOG : MDS SEND FAILURE  */
             m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_MDS_SEND_FAILURE);
             instArg->o_err = NCSCC_RC_VIP_RETRY;
             m_MMGR_FREE_IFSV_EVT(evt);
             return NCSCC_RC_FAILURE;
        }

        if (o_evt != IFSV_NULL)
        {
           if (o_evt->type == IFND_VIPD_INFO_ADD_REQ_RESP)
           {
               if( o_evt->info.vip_evt.info.ifndVipAddResp.err != NCSCC_IFND_VIP_INSTALL)  
               {
                  /* TBD : PASS ERROR MSG */
                  m_MMGR_FREE_IFSV_EVT(evt);
                  m_MMGR_FREE_IFSV_EVT(o_evt);
                  return NCSCC_RC_VIP_INTERNAL_ERROR;
               }
            /* Installing VIP onto the given interface */
            /* LOG : INSTALLING VIP AND SENDING GARP */
               m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_INSTALLING_VIP_AND_SENDING_GARP);
               m_NCS_INSTALL_VIP_AND_SEND_GARP(&instArg->i_ip_addr,(uns8 *)&instArg->i_intf_name);
               m_MMGR_FREE_IFSV_EVT(evt);
               m_MMGR_FREE_IFSV_EVT(o_evt);
            /* update IFADB */
           } /* end of o_evt->type */
           else if (o_evt->type == IFSV_VIP_ERROR)
           {
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_RECEIVED_ERROR_RESP_FOR_INSTALL);
              instArg->o_err = o_evt->info.vip_evt.info.errEvt.err;
              m_MMGR_FREE_IFSV_EVT(evt);
              m_MMGR_FREE_IFSV_EVT(o_evt);
   
              return NCSCC_RC_FAILURE;
           }
      } 
      else 
      {
         m_MMGR_FREE_IFSV_EVT(evt);
         return NCSCC_RC_VIP_INTERNAL_ERROR;
      }
  
      return NCSCC_RC_SUCCESS;

}


uns32
ncs_ifsv_vip_free(IFA_CB *pifa_cb , NCS_IFSV_VIP_FREE *pFreeArg)
{
 
   uns32            res = NCSCC_RC_SUCCESS;
   IFSV_EVT         evt;
   IFSV_EVT         *o_evt;  
 
   if ( pifa_cb == IFSV_NULL || pFreeArg == IFSV_NULL)
   {
      /* Log Error Message */
      return NCSCC_RC_FAILURE;
   }

   if(pFreeArg->i_handle.poolHdl < 1)
   {
      /* LOG: NULL POOL HANDLE */
      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_NULL_POOL_HDL);
      pFreeArg->o_err = NCSCC_RC_INVALID_VIP_HANDLE;
      return NCSCC_RC_FAILURE;
   }
   if(strlen(pFreeArg->i_handle.vipApplName) == 0)
   {
      /* LOG : NULL APPLICATION NAME */
      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_NULL_APPL_NAME);
      pFreeArg->o_err = NCSCC_RC_NULL_APPLNAME;
      return NCSCC_RC_FAILURE;
   }

   memset(&evt,0,sizeof(IFSV_EVT));

   evt.type = IFA_VIP_FREE_REQ;
   strcpy(&evt.info.vip_evt.info.ifaVipFree.handle.vipApplName,&pFreeArg->i_handle.vipApplName);
   evt.info.vip_evt.info.ifaVipFree.handle.poolHdl = pFreeArg->i_handle.poolHdl;
   evt.info.vip_evt.info.ifaVipFree.handle.ipPoolType =  NCS_IFSV_VIP_IP_INTERNAL;
 
   /* SWITCHOVER: Remove HALS_SUPPORT flag and remove comparision */
   if (pFreeArg->i_infraFlag == VIP_ADDR_NORMAL ||
       pFreeArg->i_infraFlag ==  VIP_ADDR_SPECIFIC ||
       pFreeArg->i_infraFlag == VIP_ADDR_SWITCH)
   {
      evt.info.vip_evt.info.ifaVipFree.infraFlag = pFreeArg->i_infraFlag;
   }
   else 
   {
        pFreeArg->o_err = NCSCC_RC_INVALID_FLAG;
        return NCSCC_RC_FAILURE; 
   }

  
   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_SENDING_IFA_VIP_FREE_REQ);
  
   /* Send the event to IfND */
   res = ifsv_mds_msg_sync_send(pifa_cb->my_mds_hdl, NCSMDS_SVC_ID_IFA,
             NCSMDS_SVC_ID_IFND, pifa_cb->ifnd_dest, &evt, &o_evt,
             IFA_VIP_MDS_SYNC_TIME);
              

   if (res != NCSCC_RC_SUCCESS)
   {  
       m_IFA_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_mds_msg_sync_send() returned failure"," ");
       pFreeArg->o_err = NCSCC_RC_VIP_RETRY;
       return NCSCC_RC_FAILURE;  
   }
   
   if (o_evt != IFSV_NULL) 
   {
     if (o_evt->type == IFSV_VIP_ERROR)
     {
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFA,IFSV_VIP_RECEIVED_ERROR_RESP_FOR_FREE);
        pFreeArg->o_err = o_evt->info.vip_evt.info.errEvt.err;
        m_MMGR_FREE_IFSV_EVT(o_evt);

        return NCSCC_RC_FAILURE;
     }
     /* This means we got a successful response. Free the event memory */
     m_MMGR_FREE_IFSV_EVT(o_evt);
   } else 
   {
        pFreeArg->o_err = NCSCC_RC_VIP_INTERNAL_ERROR;
        return NCSCC_RC_FAILURE; 
   }
 
   return NCSCC_RC_SUCCESS;
}

#endif
