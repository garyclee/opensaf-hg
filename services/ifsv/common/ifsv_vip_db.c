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
#include "ifsv.h"
#include "vip.h"

/*
 * Start of Static Function Declarations
 */
static uns32 ifsv_vip_free_ip_list_node(NCS_DB_LINK_LIST_NODE *ipListNode);
static uns32 ifsv_vip_free_intf_list_node(NCS_DB_LINK_LIST_NODE *intfListNode);
static uns32 ifsv_vip_free_owner_list_node(NCS_DB_LINK_LIST_NODE *ownerListNode);
static uns32 ifsv_vip_free_allocated_ip_list_node(NCS_DB_LINK_LIST_NODE *allocIpListNode);
static uns32 ifsv_vip_comp_mds_dest(uns8 * valInDb, uns8 * key);
static uns32 ifsv_vip_comp_ip_and_intf(uns8 * valInDb, uns8 *key);
static uns32 ifsv_vip_comp_intf(uns8 * valInDb, uns8 *key); 

/*
NCS_DB_LINK_LIST_NODE * ifsv_vip_search_infra_owner(NCS_DB_LINK_LIST * ownerList);
uns32 ifsv_vip_free_ip_list(NCS_DB_LINK_LIST *ipList);
uns32 ifsv_vip_free_intf_list(NCS_DB_LINK_LIST *intfList);
uns32 ifsv_vip_free_alloc_ip_list(NCS_DB_LINK_LIST *allocIpList);
uns32 ifsv_vip_free_owner_list(NCS_DB_LINK_LIST *ownerList);
*/

/*
 * End of Static Function Declarations
 */

/*
 * Start of Extern Function Declarations
 */
extern uns32 ifnd_ipxs_proc_ifip_upd(IPXS_CB *, IPXS_EVT *, IFSV_SEND_INFO *);

uns32  m_ncs_install_vip_and_send_garp(NCS_IPPFX *ipAddr,uns8 * intf)  
{
   uns32  rc = NCSCC_RC_SUCCESS;
   uns8 install[100];
   uns8 sendGARP[100];

   sprintf(install, "/sbin/ip addr add %d.%d.%d.%d/%d dev %s",
                                                         (((ipAddr->ipaddr.info.v4) & 0xff000000) >> 24),
                                                         (((ipAddr->ipaddr.info.v4) & 0xff0000) >> 16),
                                                         (((ipAddr->ipaddr.info.v4) & 0xff00) >> 8),
                                                         ((ipAddr->ipaddr.info.v4) & 0x000000ff),
                                                         (uns32)(ipAddr->mask_len),
                                                         (intf));   
   system(install);
   /* Send Gratituous ARP */
   sprintf(sendGARP,"/usr/sbin/arping -U -c 3 -I %s %d.%d.%d.%d",intf,
                                                         (((ipAddr->ipaddr.info.v4) & 0xff000000) >> 24),
                                                         (((ipAddr->ipaddr.info.v4) & 0xff0000) >> 16),
                                                         (((ipAddr->ipaddr.info.v4) & 0xff00) >> 8),
                                                         ((ipAddr->ipaddr.info.v4) & 0x000000ff));
   system(sendGARP);
   return rc;

}



/**************************************************************************
* Routing for fetching ifindex with given interface name
***************************************************************************/

NCS_IFSV_IFINDEX    ifsv_vip_get_global_ifindex(IFSV_CB *cb,uns8 *str)
{
   NCS_PATRICIA_NODE   *if_node;
   IFSV_INTF_REC       *intfRec;
   IFSV_INTF_DATA      *intfData;
   NCS_IFSV_IFINDEX     ifIndex;

   memset(&ifIndex,0, sizeof(NCS_IFSV_IFINDEX));
   if_node = ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)0);
   intfRec = (IFSV_INTF_REC*)if_node;
   intfData = &intfRec->intf_data;
   while (intfRec != NULL)
   {
     memcpy(&ifIndex,&intfData->if_index,sizeof(NCS_IFSV_IFINDEX));
     /* embedding subslot changes */
     if((intfData->spt_info.shelf == cb->shelf) &&
                (intfData->spt_info.slot == cb->slot) &&
                (intfData->spt_info.subslot == cb->subslot) && 
                (intfData->spt_info.type == NCS_IFSV_INTF_ETHERNET))
     {
        if ((strcmp(&intfData->if_info.if_name,str))== 0)
           return intfData->if_index;
     }
     if_node = ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)&ifIndex);
     intfRec = (IFSV_INTF_REC*)if_node;
     intfData = &intfRec->intf_data;
   }
   return 0;

}

/***************************************************************************
* End of Routine for fetching ifindex with given interface name
***************************************************************************/

/*************************************************************************
* Routine for getting a VIPD or VIPDC record
****************************************************************************/


NCS_PATRICIA_NODE * ifsv_vipd_vipdc_rec_get(NCS_PATRICIA_TREE *pTree,NCS_IFSV_VIP_INT_HDL *pHandle)
{
   if (pTree == IFSV_NULL || pHandle == NULL)
       /* Log error */
       return IFSV_NULL;

   return ncs_patricia_tree_get(pTree,(uns8 *)pHandle);
}

/*************************************************************************
* Routine for adding VIPD or VIPDC record.
****************************************************************************/
uns32    ifsv_vipd_vipdc_rec_add(NCS_PATRICIA_TREE *pPatTree,NCS_PATRICIA_NODE *pNode)
{
   if (pPatTree == IFSV_NULL || pNode == IFSV_NULL)
      return NCSCC_RC_FAILURE;

   return ncs_patricia_tree_add(pPatTree, pNode );

}
/*************************************************************************
* Routine for deleting  VIPD or VIPDC record.
****************************************************************************/
uns32    ifsv_vipd_vipdc_rec_del(NCS_PATRICIA_TREE *pPatTree,NCS_PATRICIA_NODE *pNode,uns32 recordType)
{
   NCS_DB_LINK_LIST_NODE *startPtr,*tmpPtr;
   IFSV_IFND_VIPDC       *pVipdc;
   IFSV_IFD_VIPD_RECORD  *pVipdRec;
   uns32 rc;

   if (pPatTree == IFSV_NULL || pNode == IFSV_NULL)
      return NCSCC_RC_FAILURE;
  
   if (recordType == IFSV_VIP_REC_TYPE_VIPD)
   {
      pVipdRec = (IFSV_IFD_VIPD_RECORD *)pNode;
      /* Free IP LIST */
      startPtr = m_NCS_DBLIST_FIND_FIRST(&pVipdRec->ip_list);
      while (startPtr)
      {
         tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);   
         if (tmpPtr == IFSV_NULL)
         {
            tmpPtr = startPtr;
            startPtr = IFSV_NULL;
         }
         ncs_db_link_list_delink(&pVipdRec->ip_list,tmpPtr);
         rc = ifsv_vip_free_ip_list_node(tmpPtr);
      }
      /* Freeing Interface List */
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
         rc = ifsv_vip_free_intf_list_node(tmpPtr);
      }
      /* Freeing Owner List */
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
         ifsv_vip_free_owner_list_node(tmpPtr);
      }

      /* Free alloc IP LIST */
      startPtr = m_NCS_DBLIST_FIND_FIRST(&pVipdRec->alloc_ip_list);
      while (startPtr)
      {
         tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);   
         if (tmpPtr == IFSV_NULL)
         {
            tmpPtr = startPtr;
            startPtr = IFSV_NULL;
         }
         ncs_db_link_list_delink(&pVipdRec->alloc_ip_list,tmpPtr);
         ifsv_vip_free_allocated_ip_list_node(tmpPtr);
      }
      /* free all the lists associated with that record */
 
   }else if (recordType == IFSV_VIP_REC_TYPE_VIPDC)
   {
      pVipdc = (IFSV_IFND_VIPDC *)pNode;
      startPtr = m_NCS_DBLIST_FIND_FIRST(&pVipdc->ip_list);
      while (startPtr)
      {
         tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);   
         if (tmpPtr == IFSV_NULL)
         {
            tmpPtr = startPtr;
            startPtr = IFSV_NULL;
         }
         ncs_db_link_list_delink(&pVipdc->ip_list,tmpPtr);
         rc = ifsv_vip_free_ip_list_node(tmpPtr);
      }
      /* Freeing Interface List */
      startPtr = m_NCS_DBLIST_FIND_FIRST(&pVipdc->intf_list);
      while (startPtr)
      {
         tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);   
         if (tmpPtr == IFSV_NULL)
         {
            tmpPtr = startPtr;
            startPtr = IFSV_NULL;
         }
         ncs_db_link_list_delink(&pVipdc->intf_list,tmpPtr);
         rc = ifsv_vip_free_intf_list_node(tmpPtr);
      }
      /* Freeing Owner List */
      startPtr = m_NCS_DBLIST_FIND_FIRST(&pVipdc->owner_list);
      while (startPtr)
      {
         tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);   
         if (tmpPtr == IFSV_NULL)
         {
            tmpPtr = startPtr;
            startPtr = IFSV_NULL;
         }
         ncs_db_link_list_delink(&pVipdc->owner_list,tmpPtr);
         ifsv_vip_free_owner_list_node(tmpPtr);
      }
   } /* end of else if condition */ 
  

   ncs_patricia_tree_del(pPatTree, pNode );
   if (recordType == IFSV_VIP_REC_TYPE_VIPD)
   {
      m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec);
   }
   else if (recordType == IFSV_VIP_REC_TYPE_VIPDC)
   {
      m_MMGR_FREE_IFSV_VIPDC_REC(pVipdc);
   }
 
   return NCSCC_RC_SUCCESS;

}
/*************************************************************************
* Routine for adding IP Node into IP LIST
**************************************************************************/

uns32 ifsv_vip_add_ip_node (NCS_DB_LINK_LIST *list, NCS_IPPFX *ipAddr,uns8 *str)
{
    NCS_IFSV_VIP_IP_LIST    *pIpList;
    NCS_DB_LINK_LIST_NODE   *pIpNode;
    
    pIpList = m_MMGR_ALLOC_IFSV_IP_NODE;
    if (pIpList == IFSV_NULL)
    {
       /* LOG ERROR MESSAGE */
       return NCSCC_RC_FAILURE;
    }
    memset(pIpList,0,sizeof(NCS_IFSV_VIP_IP_LIST));
    pIpList->ip_addr.ipaddr.type = ipAddr->ipaddr.type;
    pIpList->ip_addr.ipaddr.info.v4 = ipAddr->ipaddr.info.v4;
    pIpList->ip_addr.mask_len = ipAddr->mask_len;
    pIpList->ipAllocated = TRUE;
    strcpy(&pIpList->intfName,str);

    pIpNode = (NCS_DB_LINK_LIST_NODE *)pIpList;
    pIpNode->key = (uns8 *)&pIpList->ip_addr;
    ncs_db_link_list_add(list,pIpNode);
    return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
* Routine for adding Intf Node into Intf List 
******************************************************************************/

uns32 ifsv_vip_add_intf_node (NCS_DB_LINK_LIST *list,uns8 *str)
{
   NCS_IFSV_VIP_INTF_LIST     *pIntfList;
   NCS_DB_LINK_LIST_NODE      *pIntfNode;

   pIntfList = m_MMGR_ALLOC_IFSV_INTF_NODE;
   if  ( pIntfList == IFSV_NULL)
   {
      /* LOG ERROR MESSAGE */
      return NCSCC_RC_FAILURE;
   }
   memset(pIntfList,0,sizeof(NCS_IFSV_VIP_INTF_LIST));
   strcpy(&pIntfList->intf_name,str);
   
   pIntfNode = (NCS_DB_LINK_LIST_NODE *)pIntfList;
   pIntfNode->key = (uns8 *)&pIntfList->intf_name;

   ncs_db_link_list_add(list,pIntfNode);
   return NCSCC_RC_SUCCESS;
}

/*************************************************************************
* Routine for adding Owner Node to OWNER LIST
**************************************************************************/

uns32 ifsv_vip_add_owner_node (NCS_DB_LINK_LIST *list, MDS_DEST *dest
#if (VIP_HALS_SUPPORT == 1)
                               ,uns32 flag
#endif
)
{

    NCS_IFSV_VIP_OWNER_LIST    *pOwnerList;
    NCS_DB_LINK_LIST_NODE      *pOwnerNode;

    pOwnerList = m_MMGR_ALLOC_IFSV_OWNER_NODE;
    if (pOwnerList == IFSV_NULL)
    {
       /* LOG ERROR MESSAGE */
       return NCSCC_RC_FAILURE;
    }
    memset(pOwnerList,0,sizeof(NCS_IFSV_VIP_OWNER_LIST));
    memcpy(&pOwnerList->owner,dest,sizeof(MDS_DEST));
#if (VIP_HALS_SUPPORT == 1)
    pOwnerList->infraFlag = flag;
#endif
   
    pOwnerNode = (NCS_DB_LINK_LIST_NODE *)pOwnerList;
    pOwnerNode->key = (uns8 *)&pOwnerList->owner;

    ncs_db_link_list_add(list,pOwnerNode);
    return NCSCC_RC_SUCCESS;

}


/*******************************************************************************
* Routine for comparing MDS destinations
* Used from IfND and IfD routines
*******************************************************************************/

uns32 ifsv_vip_comp_mds_dest(uns8 * valInDb, uns8 * key)
{
   uns32 is_same = 0;
   uns32 rc = 1;
   is_same = m_NCS_IFSV_IS_MDS_DEST_SAME(valInDb,key);
   if(is_same == TRUE)
     rc = 0;

   return rc;
}

#if (VIP_HALS_SUPPORT == 1)

/*********************************************************************************
* Routine for searching the owner node with infra structual flag in owner list
*
*********************************************************************************/
NCS_DB_LINK_LIST_NODE *
ifsv_vip_search_infra_owner(NCS_DB_LINK_LIST * ownerList)
{
   NCS_DB_LINK_LIST_NODE      *pNode,*pTmpNode;
   NCS_IFSV_VIP_OWNER_LIST    *pOwnerNode;

   pNode = m_NCS_DBLIST_FIND_FIRST(ownerList);
   while (pNode) 
   {
      pTmpNode = pNode;
      pOwnerNode = (NCS_IFSV_VIP_OWNER_LIST *)pNode;
      if (pOwnerNode->infraFlag)
         return pNode;
      pNode = m_NCS_DBLIST_FIND_NEXT(pTmpNode);
   }
   return IFSV_NULL;
}
#endif
/**********************************************************************************
* Routine for comparing ip and interface given
*
**********************************************************************************/

uns32 ifsv_vip_comp_ip_and_intf(uns8 * valInDb, uns8 *key)
{
   uns32 rc = 1;
   NCS_IPPFX *dbVal,*inVal;
   dbVal = (NCS_IPPFX *)valInDb;
   inVal = (NCS_IPPFX *)key;

   if ((dbVal->ipaddr.info.v4 == inVal->ipaddr.info.v4) && (dbVal->mask_len == inVal->mask_len))
       rc = 0;

   return rc;

}
/**********************************************************************************
* Routine for comparing interface given
*
**********************************************************************************/

uns32 ifsv_vip_comp_intf(uns8 * valInDb, uns8 *key)
{
   uns32 rc = 1;

   if (strcmp(valInDb,key) == 0)
       rc = 0;

   return rc;

}
/************************************************************************************
* routine for freeing IP LIST node 
************************************************************************************/

uns32 ifsv_vip_free_ip_list_node(NCS_DB_LINK_LIST_NODE *ipListNode)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   m_MMGR_FREE_IFSV_IP_NODE(ipListNode);
   return rc;
}
/************************************************************************************
* routine for freeing Interface list node
************************************************************************************/

uns32 ifsv_vip_free_intf_list_node(NCS_DB_LINK_LIST_NODE *intfListNode)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   m_MMGR_FREE_IFSV_INTF_NODE(intfListNode);
   return rc;
}
/************************************************************************************
* routine for freeing owner list node
************************************************************************************/

uns32 ifsv_vip_free_owner_list_node(NCS_DB_LINK_LIST_NODE *ownerListNode)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   m_MMGR_FREE_IFSV_OWNER_NODE(ownerListNode);
   return rc;
}
/************************************************************************************
* routine for freeing allocated ip list node 
************************************************************************************/

uns32 ifsv_vip_free_allocated_ip_list_node(NCS_DB_LINK_LIST_NODE *allocIpListNode)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   m_MMGR_FREE_IFSV_ALLOC_IP_NODE(allocIpListNode);
   return rc;
}
/************************************************************************************
* routine for initializing IP LIST for VIPD cache or VIP database
************************************************************************************/

uns32 ifsv_vip_init_ip_list(NCS_DB_LINK_LIST *ipList)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   ipList->order = NCS_DBLIST_ASSCEND_ORDER; 
   ipList->cmp_cookie = ifsv_vip_comp_ip_and_intf;
   ipList->free_cookie =  ifsv_vip_free_ip_list_node;
   return rc;
}
/************************************************************************************
* routine for initializing  INTERFACE LIST for VIPD cache or VIP database
************************************************************************************/

uns32 ifsv_vip_init_intf_list(NCS_DB_LINK_LIST *intfList)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   intfList->order = NCS_DBLIST_ANY_ORDER;
   /* : TBD need to check if any function is to be added to this pointer */
   intfList->cmp_cookie = ifsv_vip_comp_intf; 
   intfList->free_cookie =  ifsv_vip_free_intf_list_node;
   return rc;
}
/************************************************************************************
* routine for initializing OWNER LIST for VIPD cache or VIP database
************************************************************************************/

uns32 ifsv_vip_init_owner_list(NCS_DB_LINK_LIST *ownerList)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   ownerList->order = NCS_DBLIST_ANY_ORDER;
   ownerList->cmp_cookie = ifsv_vip_comp_mds_dest;
   ownerList->free_cookie =  ifsv_vip_free_owner_list_node;
   return rc;
}
/************************************************************************************
* routine for initializing ALLOCATED IP LIST for VIPD cache or VIP database
************************************************************************************/

uns32 ifsv_vip_init_allocated_ip_list(NCS_DB_LINK_LIST *allocIpList)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   allocIpList->order = NCS_DBLIST_ASSCEND_ORDER;
/* TBD : Need to inverstigate the use of this function pointer */
/* ownerList.cmp_cookie = ifsv_vip_comp_mds_dest;*/
   return rc;
}


/****************************************************************************
* Routine for freeing the ip list
*****************************************************************************/
uns32 ifsv_vip_free_ip_list(NCS_DB_LINK_LIST *ipList)
{

   NCS_DB_LINK_LIST_NODE *startPtr,*tmpPtr;
   uns32 rc;

   /* Free IP LIST */
   startPtr = m_NCS_DBLIST_FIND_FIRST(ipList);
   while (startPtr)
   {
      tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);
      if (tmpPtr == IFSV_NULL)
      {
         tmpPtr = startPtr;
         startPtr = IFSV_NULL;
      }
      ncs_db_link_list_delink(ipList,tmpPtr);
      rc = ifsv_vip_free_ip_list_node(tmpPtr);
   }
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
* Routine for freeing the interface list
*****************************************************************************/
uns32 ifsv_vip_free_intf_list(NCS_DB_LINK_LIST *intfList)
{

   NCS_DB_LINK_LIST_NODE *startPtr,*tmpPtr;
   uns32 rc;

   startPtr = m_NCS_DBLIST_FIND_FIRST(intfList);
   while (startPtr)
   {
      tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);
      if (tmpPtr == IFSV_NULL)
      {
         tmpPtr = startPtr;
         startPtr = IFSV_NULL;
      }
      ncs_db_link_list_delink(intfList,tmpPtr);
      rc = ifsv_vip_free_intf_list_node(tmpPtr);
   }
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
* Routine for freeing owner list
*****************************************************************************/
uns32 ifsv_vip_free_owner_list(NCS_DB_LINK_LIST *ownerList)
{

   NCS_DB_LINK_LIST_NODE *startPtr,*tmpPtr;
   uns32 rc = NCSCC_RC_SUCCESS;

   startPtr = m_NCS_DBLIST_FIND_FIRST(ownerList);
   while (startPtr)
   {
      tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);
      if (tmpPtr == IFSV_NULL)
      {
         tmpPtr = startPtr;
         startPtr = IFSV_NULL;
      }
      ncs_db_link_list_delink(ownerList,tmpPtr);
      ifsv_vip_free_owner_list_node(tmpPtr);
   }
   return rc;
}


/****************************************************************************
* Routine for freeing allocated ip list
*****************************************************************************/
uns32 ifsv_vip_free_alloc_ip_list(NCS_DB_LINK_LIST *allocIpList)
{
   NCS_DB_LINK_LIST_NODE *startPtr,*tmpPtr;
   uns32 rc = NCSCC_RC_SUCCESS;

   startPtr = m_NCS_DBLIST_FIND_FIRST(allocIpList);
   while (startPtr)
   {
      tmpPtr = m_NCS_DBLIST_FIND_NEXT(startPtr);
      if (tmpPtr == IFSV_NULL)
      {
         tmpPtr = startPtr;
         startPtr = IFSV_NULL;
      }
      ncs_db_link_list_delink(allocIpList,tmpPtr);
      ifsv_vip_free_allocated_ip_list_node(tmpPtr);
   }
   return rc;
}





#endif
