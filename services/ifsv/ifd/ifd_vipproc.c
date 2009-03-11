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

#if(NCS_VIP == 1)

#include "ifd.h"
#include "vip.h"

/********************************************************************
*                 Start of Static Function Declaration
*********************************************************************/
static uns32 ifd_ifnd_proc_vipd_info_add(IFSV_CB *cb, IFSV_EVT *evt);


/********************************************************************
*                 End of Static Function Declaration
*********************************************************************/


uns32 ifd_init_vip_db(IFSV_CB *cb);
uns32  ifd_ifnd_proc_vipd_free(IFSV_CB *cb, IFSV_EVT *pEvt);

/********************************************************************
* function for initializing VIP related databases in IfD control block
*********************************************************************/

uns32 ifd_init_vip_db(IFSV_CB *cb)
{
      NCS_PATRICIA_PARAMS  params;
      
      memset(&params,0,sizeof(NCS_PATRICIA_PARAMS));
      params.key_size = sizeof(NCS_IFSV_VIP_INT_HDL);
      params.info_size = 0;
      if ((ncs_patricia_tree_init(&cb->vipDBase, &params))
         != NCSCC_RC_SUCCESS)
      {
         m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncs_patricia_tree_init() returned failure"," ");
         m_NCS_OS_LOCK(&cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_VIPD_DATABASE_CREATE_FAILED);
         return  NCSCC_RC_FAILURE;
      }
/* TBD : LOG STRINGS NEED TO BE DEFINED FOR VIP */
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_VIPD_DATABASE_CREATED);
      return NCSCC_RC_SUCCESS;
}


/****************************************************************************
* Function Name: ifd_proc_ifnd_get_ip_from_stale_entry 
* Arguements   : IFSV control block ptr, IFSV_EVT ptr
* Called From  : ifd_vip_evt_process function
* NOTES        : This function is called when IfD receives
*                any event from IfND requesting for 
*                getting an IP from stale VIP database 
****************************************************************************/
uns32 ifd_proc_ifnd_get_ip_from_stale_entry(IFSV_CB *cb, IFSV_EVT *pEvt)
{
    uns32      rc = NCSCC_RC_SUCCESS;
    NCS_PATRICIA_NODE        *pPNode;
    IFSV_IFD_VIPD_RECORD     *pVipdRec;
    NCS_IFSV_VIP_INT_HDL      vipHandle;
    IFSV_EVT                  respEvt;
    NCS_DB_LINK_LIST_NODE    *pIpNode;
    NCS_IFSV_VIP_IP_LIST    *pIpInfo;

    if (cb == IFSV_NULL || pEvt == IFSV_NULL)
        return NCSCC_RC_FAILURE;

    memset(&vipHandle,0,sizeof(NCS_IFSV_VIP_INT_HDL));

    memcpy(&vipHandle,&pEvt->info.vip_evt.info.vipCommonEvt.handle,
                                           sizeof(NCS_IFSV_VIP_INT_HDL));

    pPNode = ifsv_vipd_vipdc_rec_get(&cb->vipDBase,&vipHandle);
    if (pPNode == IFSV_NULL)
    {
        /* Implies that there are no records */
        return NCSCC_RC_INVALID_PARAM;
    }

    pVipdRec = (IFSV_IFD_VIPD_RECORD *)pPNode;
    rc = m_IFSV_IS_VIP_ENTRY_STALE(pVipdRec->vip_entry_attr);
    if (rc != TRUE)
    {
        /* Form an Error Evt and send it to IfND */
        return NCSCC_RC_VIP_RETRY;
    }

    /* Form the response event and send IP to IfND */
    memset(&respEvt,0,sizeof(IFSV_EVT));
   
    respEvt.type = IFD_IP_FROM_STALE_ENTRY_RESP;
/*
    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, 
                        IFSV_VIP_SENDING_IFD_VIP_FREE_REQ_RESP);
*/
    /* Since we have support for only one IP for a given handle  */
    /* Copy the IP Info directly and send the ip as response */
    /* This routine needs to be changed once list is supported */

    pIpNode = m_NCS_DBLIST_FIND_FIRST(&pVipdRec->ip_list); 
    if (pIpNode == IFSV_NULL)
    {
        return NCSCC_RC_VIP_INTERNAL_ERROR;
    }
    pIpInfo = (NCS_IFSV_VIP_IP_LIST *)pIpNode;
    respEvt.info.vip_evt.info.staleIp.ipAddr.ipaddr.type =
             pIpInfo->ip_addr.ipaddr.type;
    respEvt.info.vip_evt.info.staleIp.ipAddr.ipaddr.info.v4 = 
             pIpInfo->ip_addr.ipaddr.info.v4;
    respEvt.info.vip_evt.info.staleIp.ipAddr.mask_len = 
             pIpInfo->ip_addr.mask_len;

    /* send mds response to ifnd */
    rc =  ifsv_mds_send_rsp(cb->my_mds_hdl,
                           NCSMDS_SVC_ID_IFD,&pEvt->sinfo,
                           &respEvt);
    return rc;
}

/********************************************************************
* This function is called when IfD receives event for marking
* a VIPD record as stale.
* Function to process the event for marking vip entry as stale 
*********************************************************************/
uns32 ifd_vip_mark_entry_stale(IFSV_CB *cb,IFSV_EVT *pEvt)
{
    NCS_PATRICIA_NODE        *pPNode;
    IFSV_IFD_VIPD_RECORD     *pVipdRec;
    NCS_IFSV_VIP_INT_HDL      vipHandle;
    IFSV_EVT                  respEvt;
    uns32                     rc;

    memset(&vipHandle,0,sizeof(NCS_IFSV_VIP_INT_HDL));

    memcpy(&vipHandle,&pEvt->info.vip_evt.info.vipCommonEvt.handle,
                                           sizeof(NCS_IFSV_VIP_INT_HDL));

    pPNode = ifsv_vipd_vipdc_rec_get(&cb->vipDBase,&vipHandle);
    if (pPNode == IFSV_NULL)
    {
        /* Implies that there are no records */
        return NCSCC_RC_INVALID_PARAM;
    }

    pVipdRec = (IFSV_IFD_VIPD_RECORD *)pPNode;

   printf("Marking Entry as stale with ApplName : %s and handle: %d \n",
                   vipHandle.vipApplName,vipHandle.poolHdl);

    /* making the entry as stale */
    m_IFSV_VIP_STALE_ENTRY_SET(pVipdRec->vip_entry_attr);

   /* do checkpointing for the modified record */
   rc = ifd_a2s_async_update(cb, IFD_A2S_VIPD_REC_MODIFY_MSG,
                                        (uns8 *)(pVipdRec));
   /*LOGS for IFD_A2S_VIPD_REC_MODIFY_MSG EVT */

    memset(&respEvt,0,sizeof(IFSV_EVT));

    /* form response event and send to IFND */
    respEvt.type = IFD_VIP_FREE_REQ_RESP;

    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,
                        IFSV_VIP_SENDING_IFD_VIP_FREE_REQ_RESP);

    /* send mds response to ifnd */
    rc =  ifsv_mds_send_rsp(cb->my_mds_hdl,
                           NCSMDS_SVC_ID_IFD,&pEvt->sinfo,
                           &respEvt);
    return rc;

}


/********************************************************************
* This Function is called when IfND is crashed.                     * 
* This Function Marks the VIPDB records as stale for the records    *
* Created by IfND which is crashed.                                 * 
* Calling Function : ifd_same_dst_all_intf_rec_mark_del             *
********************************************************************/

uns32 ifd_vip_process_ifnd_crash(MDS_DEST *pIfndDst,IFSV_CB *pIfsvCb)
{
    NCS_PATRICIA_NODE        *pPNode;
    IFSV_IFD_VIPD_RECORD     *pVipdRec;
    NCS_DB_LINK_LIST_NODE    *pOwnerNode;
    NCS_IFSV_VIP_INT_HDL      vipHandle;

    memset(&vipHandle,0,sizeof(NCS_IFSV_VIP_INT_HDL));

    pPNode= ncs_patricia_tree_getnext(&pIfsvCb->vipDBase,(uns8 *)0);
    if (pPNode == IFSV_NULL)
    {
        /* Implies that there are no records */
        return NCSCC_RC_SUCCESS;
    }
    while (pPNode)
    {
        pVipdRec = (IFSV_IFD_VIPD_RECORD *)pPNode;
        memcpy(&vipHandle,&pVipdRec->handle,sizeof(NCS_IFSV_VIP_INT_HDL));
        /* Write logic here for finding record related to crashed IFND */
        pOwnerNode = ncs_db_link_list_find(&pVipdRec->owner_list,(uns8 *)pIfndDst);
        if(pOwnerNode != IFSV_NULL)
        {
           printf("IFND CRASH: Marking Entry as stale with ApplName : %s and handle: %d \n",
                   vipHandle.vipApplName,vipHandle.poolHdl);
           /* making the entry as stale */
           m_IFSV_VIP_STALE_ENTRY_SET(pVipdRec->vip_entry_attr);
        }
        pPNode= ncs_patricia_tree_getnext(&pIfsvCb->vipDBase,(uns8 *)&vipHandle);
    }   /* End of while(pPNode) loop */
    return NCSCC_RC_SUCCESS;
}       /* End of processing of ifnd_crash */



/********************************************************************
* function to process IFND_VIP_FREE event
*********************************************************************/

/****************************************************************************
 * Name          : ifd_ifnd_proc_vipd_free
 *
 * Description   : This function is used to process a free event from
 *                 IfND's.
 *
 * Arguments     : cb        - This is the IfD control block pointer
 *                 pEvt      - pointer to the IFSV_EVT from IfND.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32  ifd_ifnd_proc_vipd_free(IFSV_CB *cb,IFSV_EVT *pEvt )
{
    uns32                    rc = NCSCC_RC_SUCCESS;
    NCS_PATRICIA_NODE            *pPatEntry;
    IFSV_IFD_VIPD_RECORD         *pVipd;
    IFSV_EVT                      sendEvt;
    NCS_DB_LINK_LIST_NODE        *pNode;

    if (cb == IFSV_NULL || pEvt == IFSV_NULL)
    {
       /* Log Error Message */
       return NCSCC_RC_VIP_INTERNAL_ERROR;
    }
    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, 
                        IFSV_VIP_RECEIVED_IFND_VIP_FREE_REQ);

    pPatEntry = ifsv_vipd_vipdc_rec_get(&cb->vipDBase,
                          &pEvt->info.vip_evt.info.ifndVipFree.handle);
    if (pPatEntry == IFSV_NULL)
    {
       /* Log ERROR MESSAGE */
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,IFSV_VIP_VIPD_LOOKUP_FAILED);
       return NCSCC_RC_INVALID_PARAM;
    }

    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,IFSV_VIP_VIPD_LOOKUP_SUCCESS);

    pVipd = (IFSV_IFD_VIPD_RECORD *)pPatEntry;
    if(pVipd->ref_cnt > 1)
    {
       /* If ref count is more than 1, then we need not free the Entry IP */
       /* Just remove the owner address from the owner list */
       pVipd->ref_cnt = pVipd->ref_cnt - 1;

       /* Here we need to check if the free request is from TERMINATE script *
       *  In that case the MDS destinations doesnt match. so we need to check*
       *  For the presence of infrastructural flag in the owner list and delete*
       *  The owner node with infra structural flag */
       if(pEvt->info.vip_evt.info.ifndVipFree.infraFlag == VIP_ADDR_SPECIFIC)
       {
          pNode = ifsv_vip_search_infra_owner(&pVipd->owner_list);      
          if (pNode == IFSV_NULL)
          {
             return NCSCC_RC_VIP_INTERNAL_ERROR;
          }
          rc = ncs_db_link_list_del(&pVipd->owner_list,(uns8 *)&pEvt->sinfo.dest);
          if (rc == NCSCC_RC_FAILURE)
          {
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, 
                                  IFSV_VIP_LINK_LIST_NODE_DELETION_FAILED);
             /* Check for sending error in the event*/
             return NCSCC_RC_VIP_INTERNAL_ERROR;
          }
       }
       else 
      {
          rc = ncs_db_link_list_del(&pVipd->owner_list,(uns8 *)&pEvt->sinfo.dest);
          if (rc == NCSCC_RC_FAILURE)
          {
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, 
                                  IFSV_VIP_LINK_LIST_NODE_DELETION_FAILED);
             /* Check for sending error in the event*/
             return NCSCC_RC_VIP_INTERNAL_ERROR;
          }
       }

       /* Send a success msg to ifnd */
    }
    else  /* If reference count == 1 */
    {
       /* Send the trigger point to Standby IfD. */
       rc = ifd_a2s_async_update(cb, IFD_A2S_VIPD_REC_DEL_MSG,
                                       (uns8 *)(pVipd));
       /*  Insert the checkpoint failure handling code. */
       /* If reference count is 1, delete the entry */
       rc = ifsv_vipd_vipdc_rec_del(&cb->vipDBase,(NCS_PATRICIA_NODE *)pVipd,IFSV_VIP_REC_TYPE_VIPD);
       if (rc == NCSCC_RC_FAILURE) /* IF VIPD is not cleared */
       {
          /* LOG ERROR MESSAGE */
          m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_RECORD_DELETION_FAILED);
          return NCSCC_RC_VIP_INTERNAL_ERROR;
       }
       /* TBD :: KK AND ACHYUT: add checkpt code for record del : FULL RECORD */
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_RECORD_DELETION_SUCCESS);
    }
   
    memset(&sendEvt,0,sizeof(IFSV_EVT));

    /* form response event and send to IFND */
    sendEvt.type = IFD_VIP_FREE_REQ_RESP;

    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, 
                        IFSV_VIP_SENDING_IFD_VIP_FREE_REQ_RESP);
    
    /* send mds response to ifnd */
    rc =  ifsv_mds_send_rsp(cb->my_mds_hdl,
                           NCSMDS_SVC_ID_IFD,&pEvt->sinfo,
                           &sendEvt);
    return rc;
}



/****************************************************************************
* Functions for processing individual events received by ifd
****************************************************************************/

/****************************************************************************
* Function Name: ifd_ifnd_proc_vipd_info_add
* Arguements   : IFSV control block ptr, IFSV_EVT ptr
* Called From  : ifd_vip_evt_process function
* NOTES        : This function is called when IfD receives
*                any event from IfND requesting for addition 
*                of Virtual IP Address related info into VIP database 
****************************************************************************/
uns32 ifd_ifnd_proc_vipd_info_add(IFSV_CB *cb, IFSV_EVT *evt)
{
    uns32                    rc = NCSCC_RC_SUCCESS;
    IFSV_EVT                 respEvt;
    IFSV_IFD_VIPD_RECORD     *pVipdRec;
    NCS_PATRICIA_NODE        *pVipdNode;
    NCS_DB_LINK_LIST_NODE    *ifndOwner;
    NCS_DB_LINK_LIST_NODE    *ipNode = IFSV_NULL;
    NCS_DB_LINK_LIST_NODE    *startPtr,*tmpPtr;
    NCS_IFSV_VIP_IP_LIST     *pIpList;
 
    if (cb == IFSV_NULL || evt == IFSV_NULL)
       /* Log Error message */
        return NCSCC_RC_VIP_INTERNAL_ERROR;


    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_RECEIVED_IFND_VIPD_INFO_ADD_REQ);

    pVipdNode = ifsv_vipd_vipdc_rec_get(&cb->vipDBase,
                             &evt->info.vip_evt.info.ifndVipAdd.handle);


    if(pVipdNode != IFSV_NULL ) /* Indicates an Entry already exists with given handle */
    {
       /* Log Error message */
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_VIPD_LOOKUP_FAILED);

       /* Check whether we received the add request from the existing ifnd */
       pVipdRec = (IFSV_IFD_VIPD_RECORD *)pVipdNode;
       ifndOwner = ncs_db_link_list_find(&pVipdRec->owner_list,(uns8 *)&evt->sinfo.dest);
       if (ifndOwner != IFSV_NULL) /* Implies we received the request from existing ifnd */
       {
          ipNode = ncs_db_link_list_find(&(pVipdRec->ip_list),
                          (uns8 *)&evt->info.vip_evt.info.ifndVipAdd.ipAddr);

          if (ipNode != IFSV_NULL )
          {
             m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_REQUESTED_VIP_ALREADY_EXISTS);
             /* Check if the vipd entry is stale */
             if (( m_IFSV_IS_VIP_ENTRY_STALE(pVipdRec->vip_entry_attr)) != TRUE)
             {
                 return NCSCC_RC_IP_ALREADY_EXISTS;
             }
             else 
             { 
                 /* Make the entry as not stale */
                 m_IFSV_VIP_ENTRY_STALE_FREE(pVipdRec->vip_entry_attr);

                 /* Updating the New interface in IP LIST */
                
                 pIpList = ( NCS_IFSV_VIP_IP_LIST *)ipNode;
                 m_NCS_STRCPY(&pIpList->intfName,&evt->info.vip_evt.info.ifndVipAdd.intfName);

                 /* Update the interface to new interface */
                 /* First remove old interface from list and add new intf */
                 startPtr = m_NCS_DBLIST_FIND_FIRST(&pVipdRec->intf_list);
                 while (startPtr)
                 {
                     tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);
                     if (tmpPtr == IFSV_NULL)
                     {
                         tmpPtr = startPtr;
                         startPtr = IFSV_NULL;
                     }
                     ncs_db_link_list_delink(&pVipdRec->intf_list,tmpPtr);
                     m_MMGR_FREE_IFSV_INTF_NODE(tmpPtr); 
                 }

                 /* Adding Intf node to the intf list of the vipd cache*/
                 rc = ifsv_vip_add_intf_node (&pVipdRec->intf_list,
                                    (uns8 *)&evt->info.vip_evt.info.ifndVipAdd.intfName);
                 {
                    if ( rc == NCSCC_RC_FAILURE)
                    {
                       /* LOG ERROR MESSAGE */
                       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_INTERNAL_ERROR);
                       return NCSCC_RC_VIP_INTERNAL_ERROR;
                    }
                    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_ADDED_INTERFACE_NODE_TO_VIPD);
                 }


                 /* check pointing the modified VIPD record */
                 /* Send the trigger point to Standby IfD. */

                 rc = ifd_a2s_async_update(cb, IFD_A2S_VIPD_REC_MODIFY_MSG,
                                        (uns8 *)(pVipdRec));

                 /* form a IFD_VIPD_INFO_ADD_RESP response event and send to IfND */

                 memset (&respEvt,0,sizeof(IFSV_EVT));

                 respEvt.type = IFD_VIPD_INFO_ADD_REQ_RESP;
                 respEvt.info.vip_evt.info.ifdVipAddResp.err = NCSCC_RC_SUCCESS;

                 m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,
                             IFSV_VIP_SENDING_IFD_VIPD_INFO_ADD_REQ_RESP);

                 rc =  ifsv_mds_send_rsp(cb->my_mds_hdl,
                               NCSMDS_SVC_ID_IFD,&evt->sinfo,
                               &respEvt);
                 return rc;
             }
          }
          else
          {
             return NCSCC_VIP_HDL_IN_USE_FOR_DIFF_IP;
          }
          
       }
       /* IF control comes here it means that we received the add request from a different ifnd */ 
       /* IF the request is for exisitng IP then we Modify the records.*/  
       /* IF the request is for new IP then add the ip to list: NEXT PHASE */
       ipNode = ncs_db_link_list_find(&(pVipdRec->ip_list),
                       (uns8 *)&evt->info.vip_evt.info.ifndVipAdd.ipAddr);

       if (ipNode != IFSV_NULL )
       {
          if (( m_IFSV_IS_VIP_ENTRY_STALE(pVipdRec->vip_entry_attr)) == TRUE)
          {
              /* If the control comes here, It means node fail over or
                 switch over has occured. so modify the record with new
                 ifnd address and update the interface. Also make the entry
                 as non stale  */
 
                 /* Updating the New interface in IP LIST */
                 pIpList = ( NCS_IFSV_VIP_IP_LIST *)ipNode;
                 m_NCS_STRCPY(&pIpList->intfName,&evt->info.vip_evt.info.ifndVipAdd.intfName);

                 /* Freeing Owner List. At present only one owner will be there */
                 startPtr = m_NCS_DBLIST_FIND_FIRST(&pVipdRec->owner_list);
                 while (startPtr)
                 {
                     tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);
                     if (tmpPtr == IFSV_NULL)
                     {
                         tmpPtr = startPtr;
                         startPtr = IFSV_NULL;
                     }
                     ncs_db_link_list_delink(&pVipdRec->owner_list,tmpPtr);
                     m_MMGR_FREE_IFSV_OWNER_NODE(tmpPtr); 
                 }

                 /* Now Add the New Owner to the Owner list of vipd record */
                 rc = ifsv_vip_add_owner_node(&pVipdRec->owner_list,
                                              &evt->sinfo.dest
                                   ,evt->info.vip_evt.info.ifndVipAdd.infraFlag
                                   );
                 {
                    if ( rc == NCSCC_RC_FAILURE)
                    {
                        return NCSCC_RC_VIP_INTERNAL_ERROR;
                    }
                    /* LOG: ADDED OWNER NODE TO VIPD */
                    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,
                                          IFSV_VIP_ADDED_OWNER_NODE_TO_VIPD);
                 }

                 /* Update the interface to new interface */
                 /* First remove old interface from list and add new intf */
                 startPtr = m_NCS_DBLIST_FIND_FIRST(&pVipdRec->intf_list);
                 while (startPtr)
                 {
                     tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);
                     if (tmpPtr == IFSV_NULL)
                     {
                         tmpPtr = startPtr;
                         startPtr = IFSV_NULL;
                     }
                     ncs_db_link_list_delink(&pVipdRec->intf_list,tmpPtr);
                     m_MMGR_FREE_IFSV_INTF_NODE(tmpPtr);
                 }

                 /* Adding Intf node to the intf list of the vipd cache*/
                 rc = ifsv_vip_add_intf_node (&pVipdRec->intf_list,
                                    (uns8 *)&evt->info.vip_evt.info.ifndVipAdd.intfName);
                 {
                    if ( rc == NCSCC_RC_FAILURE)
                    {
                       /* LOG ERROR MESSAGE */
                       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_INTERNAL_ERROR);
                       return NCSCC_RC_VIP_INTERNAL_ERROR;
                    }
                    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_ADDED_INTERFACE_NODE_TO_VIPD);
                 }

                 m_IFSV_VIP_ENTRY_STALE_FREE(pVipdRec->vip_entry_attr);

                 /* Do a checkpointing for the modified record */

                 rc = ifd_a2s_async_update(cb, IFD_A2S_VIPD_REC_MODIFY_MSG,
                                        (uns8 *)(pVipdRec));
          }
          else 
          {
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_REQUESTED_VIP_ALREADY_EXISTS);
              return NCSCC_VIP_EXISTS_ON_OTHER_INTERFACE;
          }
       }
       /* Add the IP to IP LIST NEXT PHASE*/

       pVipdRec = (IFSV_IFD_VIPD_RECORD *)pVipdNode;
 
       /* Do a checkpoint of VIPD */
      
    } /* end of pVipdNode != IFSV_NULL */
    else  /* if vipd Record found */
    {
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,IFSV_VIP_VIPD_LOOKUP_FAILED);

       pVipdRec = m_MMGR_ALLOC_IFSV_VIPD_REC ;
 
       if (pVipdRec == IFSV_NULL)
       {
          /* Log Error message */
          m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_MEM_ALLOC_FAILED);
          return NCSCC_RC_VIP_INTERNAL_ERROR;
       }  
       memset(pVipdRec,0,sizeof(IFSV_IFD_VIPD_RECORD));
   
       /* TBD: Fill this attributes */
       m_NCS_STRCPY(&pVipdRec->handle.vipApplName,
               &evt->info.vip_evt.info.ifndVipAdd.handle.vipApplName);

       pVipdRec->handle.poolHdl = 
               evt->info.vip_evt.info.ifndVipAdd.handle.poolHdl;

       pVipdRec->handle.ipPoolType = 
               evt->info.vip_evt.info.ifndVipAdd.handle.ipPoolType;

       /* Since this is the first entry, the reference count will be 1 */
       pVipdRec->ref_cnt = 1;
  
       m_IFSV_VIP_ALLOCATED_SET(pVipdRec->vip_entry_attr);  /* Setting the VIPD entry as allocated */
       m_IFSV_VIP_APPLIP_SET(pVipdRec->vip_entry_attr);  
       m_IFSV_VIP_ENTRY_COMPLETE_SET(pVipdRec->vip_entry_attr);
       m_IFSV_VIP_APPL_CONFIGURED_SET(pVipdRec->vip_entry_attr);
  
       /* Check for the Initialization of this Linked Lists */
       rc = ifsv_vip_init_ip_list(&pVipdRec->ip_list);
       if (rc != NCSCC_RC_SUCCESS)
       {  
          m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_vip_init_ip_list() returned failure"," ");
          /* NEED TO FREE THE RECORD */
          m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec);  
          return NCSCC_RC_VIP_INTERNAL_ERROR;
       }
       rc = ifsv_vip_init_intf_list(&pVipdRec->intf_list);
       if (rc != NCSCC_RC_SUCCESS)
       {  
          m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_vip_init_intf_list() returned failure"," ");
          /* NEED TO FREE THE RECORD AND THE INITIALIZED LISTS */
          m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec);
          return NCSCC_RC_VIP_INTERNAL_ERROR;
       }
       rc = ifsv_vip_init_owner_list(&pVipdRec->owner_list);
       if (rc != NCSCC_RC_SUCCESS)
       {
          m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_vip_init_owner_list() returned failure"," ");
          /* NEED TO FREE THE RECORD AND INITIALIZED LISTS */
          m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec);
          return NCSCC_RC_VIP_INTERNAL_ERROR;
       }
       rc = ifsv_vip_init_allocated_ip_list(&pVipdRec->alloc_ip_list);
       if (rc != NCSCC_RC_SUCCESS)
       {
          m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_vip_init_allocated_ip_list() returned failure"," ");
          /* NEED TO FREE THE RECORD AND INITIALIZED LISTS */
          m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec);
          return NCSCC_RC_VIP_INTERNAL_ERROR;
       }
       /* Add IP NODE, INTF NODE and OWNER NODE to the respective lists */
       rc = ifsv_vip_add_ip_node(&pVipdRec->ip_list,
                                 &evt->info.vip_evt.info.ifndVipAdd.ipAddr,
                                 (uns8 *)&evt->info.vip_evt.info.ifndVipAdd.intfName);
       {
          if ( rc == NCSCC_RC_FAILURE)
          {
             /* LOG ERROR MESSAGE */
             m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_INTERNAL_ERROR);
             m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec);
             return NCSCC_RC_VIP_INTERNAL_ERROR;
          }
             m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_ADDED_IP_NODE_TO_VIPD);
       }
 
       /* Adding Intf node to the intf list of the vipd cache*/
       rc = ifsv_vip_add_intf_node (&pVipdRec->intf_list,
                                    (uns8 *)&evt->info.vip_evt.info.ifndVipAdd.intfName);
       {
          if ( rc == NCSCC_RC_FAILURE)
          {
             /* LOG ERROR MESSAGE */
             m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_INTERNAL_ERROR);
             ifsv_vip_free_ip_list(&pVipdRec->ip_list);
             m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec);
             return NCSCC_RC_VIP_INTERNAL_ERROR;
          }
             m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_ADDED_INTERFACE_NODE_TO_VIPD);
       }

      /* Adding owner node to the owner list */
       rc = ifsv_vip_add_owner_node(&pVipdRec->owner_list,
                                   &evt->sinfo.dest
                                   ,evt->info.vip_evt.info.ifndVipAdd.infraFlag
                                   );
       {
          if ( rc == NCSCC_RC_FAILURE)
          {
             ifsv_vip_free_ip_list(&pVipdRec->ip_list);
             ifsv_vip_free_intf_list(&pVipdRec->intf_list);
             m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec);  
             return NCSCC_RC_VIP_INTERNAL_ERROR;
          }
          /* LOG: ADDED OWNER NODE TO VIPD CACHE */
          m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_ADDED_OWNER_NODE_TO_VIPD);
       } 

       /* TBD: Add IP TO THE IP ALLOCATED LIST */

       /* Since all the lists are updated, add vipd record to vip database */  

       pVipdNode = (NCS_PATRICIA_NODE *)pVipdRec;
       pVipdNode->key_info = (uns8 *)&pVipdRec->handle;

       /* check pointing VIPD : FULL RECORD */
       /* Send the trigger point to Standby IfD. */

       rc = ifd_a2s_async_update(cb, IFD_A2S_VIPD_REC_CREATE_MSG,
                                        (uns8 *)(pVipdRec));
       /*   Insert the code of failure of checkpoint*/ 
       rc = ifsv_vipd_vipdc_rec_add(&cb->vipDBase, pVipdNode);
       if (rc == NCSCC_RC_FAILURE )
       {
          /* Log Error message */
          ifsv_vip_free_ip_list(&pVipdRec->ip_list);
          ifsv_vip_free_intf_list(&pVipdRec->intf_list);
          ifsv_vip_free_owner_list(&pVipdRec->owner_list);
          m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec);
          m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_RECORD_ADDITION_TO_VIPD_FAILURE);
          return NCSCC_RC_VIP_INTERNAL_ERROR; 
       }
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_RECORD_ADDITION_TO_VIPD_SUCCESS);
   }  /* end of else part */ 


    /* form a IFD_VIPD_INFO_ADD_RESP response event and send to IfND */

    memset (&respEvt,0,sizeof(IFSV_EVT));

    respEvt.type = IFD_VIPD_INFO_ADD_REQ_RESP;
    respEvt.info.vip_evt.info.ifdVipAddResp.err = NCSCC_RC_SUCCESS;

    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,
                        IFSV_VIP_SENDING_IFD_VIPD_INFO_ADD_REQ_RESP);

    rc =  ifsv_mds_send_rsp(cb->my_mds_hdl,
                             NCSMDS_SVC_ID_IFD,&evt->sinfo,
                             &respEvt);
    return rc;
}


/****************************************************************************
 * Name          : ifd_vip_evt_process
 *
 * Description   : This function used to parse the events received by IfD
 *                 
 *
 * Arguments     : cb : This is IFSV control block
 *                 evt: this is the ptr to the event received by IfD    
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

/* this function will be called from ifd event process code */
uns32 ifd_vip_evt_process(IFSV_CB *cb, IFSV_EVT *evt)
{
   uns32        rc = NCSCC_RC_SUCCESS;
   IFSV_EVT     errEvt;
   switch(evt->type)
   {
       case IFND_VIPD_INFO_ADD_REQ:
          rc = ifd_ifnd_proc_vipd_info_add(cb,evt);
          if( rc != NCSCC_RC_SUCCESS) 
          m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_ifnd_proc_vipd_info_add() returned failure,rc:",rc);
          break;
       case IFND_VIP_FREE_REQ:
          rc = ifd_ifnd_proc_vipd_free(cb,evt );
          if( rc != NCSCC_RC_SUCCESS) 
          m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_ifnd_proc_vipd_free() returned failure,rc:",rc);
          break;
       case  IFND_VIP_MARK_VIPD_STALE:
          rc = ifd_vip_mark_entry_stale(cb,evt);
          if( rc != NCSCC_RC_SUCCESS) 
          m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_vip_mark_entry_stale() returned failure,rc:",rc);
          break;
       case IFND_GET_IP_FROM_STALE_ENTRY_REQ:
          rc = ifd_proc_ifnd_get_ip_from_stale_entry(cb,evt);
          if( rc != NCSCC_RC_SUCCESS) 
          m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_proc_ifnd_get_ip_from_stale_entry() returned failure,rc:",rc);
          break;
       default:
          rc = NCSCC_RC_FAILURE;
          break;
   }

   if (rc != NCSCC_RC_SUCCESS && rc != NCSCC_RC_FAILURE) 
   {
      memset(&errEvt,0,sizeof(IFSV_EVT));
      errEvt.type = IFSV_VIP_ERROR;
      errEvt.info.vip_evt.info.errEvt.err = rc;

      /* Send an error event to the ifnd waiting for response */
      rc =  ifsv_mds_send_rsp(cb->my_mds_hdl,
                             NCSMDS_SVC_ID_IFD,&evt->sinfo,
                             &errEvt);
   }
   return rc;

}

/*
  *Note::
      The function below is used in the VIP CheckPointing::
      During warm Sync
*/


/****************************************************************************
 * Name          : ifd_all_vip_rec_del
 *
 * Description   : This function will be called when the IfD needs to be 
 *                 shutdown. This will cleanup all the interfaces record.
 *
 * Arguments     : ifsv_cb     - pointer to the interface Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Here we assume that there would be atleast one IfD would
 *                 be active always, so this should not inform the IfNDs about
 *                 the cleaning of the record.
 *****************************************************************************/

uns32
ifd_all_vip_rec_del (IFSV_CB *ifsv_cb)
{
   uns32 res = NCSCC_RC_SUCCESS;
   IFSV_IFD_VIPD_RECORD *p_vip_db_rec;
   NCS_IFSV_VIP_INT_HDL vip_handle;   
   
   memset(&vip_handle, 0, sizeof(NCS_IFSV_VIP_INT_HDL));

   p_vip_db_rec = (IFSV_IFD_VIPD_RECORD*)ncs_patricia_tree_getnext(&ifsv_cb->vipDBase, (uns8*)&vip_handle);

   while (p_vip_db_rec != IFSV_NULL)
   {
      
      res = ifsv_vipd_vipdc_rec_del(&ifsv_cb->vipDBase, (NCS_PATRICIA_NODE *)p_vip_db_rec, IFSV_VIP_REC_TYPE_VIPD);

      memcpy(&vip_handle, &p_vip_db_rec->handle, sizeof(NCS_IFSV_VIP_INT_HDL));
      p_vip_db_rec = (IFSV_IFD_VIPD_RECORD* )ncs_patricia_tree_getnext(&ifsv_cb->vipDBase, (uns8*)&vip_handle);
      
   }

   return (res);
}



/*
NOTE : In phase-11 we need to take care of active interface
as there are multiple interfaces for the same application
*/

#endif
