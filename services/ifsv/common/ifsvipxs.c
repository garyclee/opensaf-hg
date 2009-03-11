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

#if(NCS_IFSV_USE_NETLINK == 1)
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "ncs_iplib.h"
#endif


static uns32 ipxs_ipinfo_cpy_for_del(IPXS_IFIP_INFO *src, NCS_IPXS_IPINFO *dest);

uns32 gl_ipxs_cb_hdl;

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ifip_record_get
  DESCRIPTION       :   Get IF-IP Record
  ARGUMENTS         :   NCS_PATRICIA_TREE *ifip_tbl  :  Pointer to IP Database
                    :   uns32 ifindex Interface Index
  RETURNS           :   Pointer to IPXS_IFIP_INFO.
  NOTES             :   
*****************************************************************************/
IPXS_IFIP_NODE* 
ipxs_ifip_record_get(NCS_PATRICIA_TREE *ifip_tbl, uns32 ifindex)
{
   IPXS_IFIP_NODE *node=0;

   node = (IPXS_IFIP_NODE *) ncs_patricia_tree_get(ifip_tbl, (uns8 *)&ifindex);

   return node;
}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ifip_record_add
  DESCRIPTION       :   Add IF-IP Record
  ARGUMENTS         :   NCS_PATRICIA_TREE *ifip_tbl  :  Pointer to IP Database
                    :   Pointer to IPXS_IFIP_INFO.
  NOTES             :   
*****************************************************************************/
uns32  
ipxs_ifip_record_add(IPXS_CB *cb, NCS_PATRICIA_TREE *ifip_tbl, IPXS_IFIP_NODE *ifip_node)
{
   uns32 rc = NCSCC_RC_FAILURE;
   ifip_node->patnode.key_info = (uns8*)&ifip_node->ifip_info.ifindexNet;
   if(cb->ifip_tbl_up)
   {
      if(cb->my_svc_id == NCS_SERVICE_ID_IFND)
        m_NCS_LOCK(&cb->ipxs_db_lock, NCS_LOCK_WRITE); 
      rc = ncs_patricia_tree_add(ifip_tbl, &ifip_node->patnode);
      if(rc == NCSCC_RC_SUCCESS)
          m_IFSV_LOG_HEAD_LINE_NORMAL(cb->my_svc_id,IFSV_LOG_IFIP_NODE_ID_TBL_ADD_SUCCESS,\
                              ifip_node->ifip_info.ifindexNet, 0);
      else
          m_IFSV_LOG_HEAD_LINE(cb->my_svc_id,IFSV_LOG_IFIP_NODE_ID_TBL_ADD_FAILURE,\
                              ifip_node->ifip_info.ifindexNet, 0);
      if(cb->my_svc_id == NCS_SERVICE_ID_IFND)
       m_NCS_UNLOCK(&cb->ipxs_db_lock, NCS_LOCK_WRITE);
      return rc;
   }
   else
   {
      m_IFSV_LOG_HEAD_LINE(cb->my_svc_id,IFSV_LOG_IFIP_NODE_ID_TBL_ADD_FAILURE,\
                              ifip_node->ifip_info.ifindexNet, 0);
      return NCSCC_RC_FAILURE;
   }

}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ifip_record_del
  DESCRIPTION       :   Add IF-IP Record
  ARGUMENTS         :   NCS_PATRICIA_TREE *ifip_tbl  :  Pointer to IP Database
                    :   Pointer to IPXS_IFIP_INFO.
  NOTES             :   
*****************************************************************************/
uns32  
ipxs_ifip_record_del(IPXS_CB *cb, NCS_PATRICIA_TREE *ifip_tbl, IPXS_IFIP_NODE *ifip_node)
{
   /* Delete the internal pointers */
   if(ifip_node == NULL)
      return NCSCC_RC_SUCCESS;
   if(cb->my_svc_id == NCS_SERVICE_ID_IFND)
      m_NCS_LOCK(&cb->ipxs_db_lock, NCS_LOCK_WRITE);

   if(ifip_node->ifip_info.ipaddr_cnt)
   {
      ipxs_ip_record_list_del(cb, &ifip_node->ifip_info);
   }

   /* Delete the node from Tree */
   if(cb->ifip_tbl_up)
      ncs_patricia_tree_del(ifip_tbl, &ifip_node->patnode);

   m_IFSV_LOG_HEAD_LINE_NORMAL(cb->my_svc_id,IFSV_LOG_IFIP_NODE_ID_TBL_DEL_SUCCESS,\
                              ifip_node->ifip_info.ifindexNet, 0);

   /* Free the node */
   m_MMGR_FREE_IPXS_IFIP_NODE(ifip_node);

   if(cb->my_svc_id == NCS_SERVICE_ID_IFND)
    m_NCS_UNLOCK(&cb->ipxs_db_lock, NCS_LOCK_WRITE);


   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ifip_ippfx_add
  DESCRIPTION       :   Get IF-IP Record
  ARGUMENTS         :   NCS_PATRICIA_TREE *ifip_tbl  :  Pointer to IP Database
                    :   uns32 ifindex Interface Index
  RETURNS           :   Pointer to IPXS_IFIP_INFO.
  NOTES             :   
*****************************************************************************/
uns32 
ipxs_ifip_ippfx_add(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info, IPXS_IFIP_IP_INFO *ippfx,
                    uns32 cnt, NCS_BOOL *add_flag)
{
   IPXS_IFIP_IP_INFO      *old_list=0, *new_list=0;
   uns32          old_cnt=0, new_cnt=0;

   if(cb->my_svc_id == NCS_SERVICE_ID_IFND)
     m_NCS_LOCK(&cb->ipxs_db_lock, NCS_LOCK_WRITE);


   if(ifip_info->ipaddr_cnt)
   {
      old_cnt = ifip_info->ipaddr_cnt;
      old_list = ifip_info->ipaddr_list;
   }

   if(cnt)
   {
      /* Allocate the new list */

      new_list = (IPXS_IFIP_IP_INFO *) m_MMGR_ALLOC_IPXS_DEFAULT((old_cnt+cnt)*sizeof(IPXS_IFIP_IP_INFO));

      if(!new_list)
      {
         m_IFSV_LOG_SYS_CALL_FAIL(cb->my_svc_id,IFSV_LOG_MEM_ALLOC_FAIL,0);
         if(cb->my_svc_id == NCS_SERVICE_ID_IFND)
          m_NCS_UNLOCK(&cb->ipxs_db_lock, NCS_LOCK_WRITE);
         return NCSCC_RC_FAILURE;
      }
      if(old_cnt)
      {
         memcpy(new_list, old_list, (sizeof(IPXS_IFIP_IP_INFO)*old_cnt));
         new_cnt += old_cnt;
      }

      memcpy(&new_list[new_cnt], ippfx, (sizeof(IPXS_IFIP_IP_INFO)*cnt));
      new_cnt += cnt;
      
      ifip_info->ipaddr_list = new_list;
      ifip_info->ipaddr_cnt = (uns8) new_cnt;

      *add_flag = TRUE;
   }

   if(*add_flag)
   {
      /* Free the old list */
      if(old_list)
         m_MMGR_FREE_IPXS_DEFAULT(old_list);
   }

    if(cb->my_svc_id == NCS_SERVICE_ID_IFND)
       m_NCS_UNLOCK(&cb->ipxs_db_lock, NCS_LOCK_WRITE);

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ifip_ippfx_del
  DESCRIPTION       :   Get IF-IP Record
  ARGUMENTS         :   NCS_PATRICIA_TREE *ifip_tbl  :  Pointer to IP Database
                    :   uns32 ifindex Interface Index
  RETURNS           :   Pointer to IPXS_IFIP_INFO.
  NOTES             :   
*****************************************************************************/
uns32 
ipxs_ifip_ippfx_del(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info, IPXS_IFIP_IP_INFO *ippfx,
                    uns32 cnt, NCS_BOOL *del_flag)
{
   IPXS_IFIP_IP_INFO      *old_list=0, *temp_list=0;
   uns32          old_cnt=0, temp_cnt=0;
   uns32          rc = NCSCC_RC_SUCCESS, i=0, j=0;

   if(cb->my_svc_id == NCS_SERVICE_ID_IFND)
        m_NCS_LOCK(&cb->ipxs_db_lock, NCS_LOCK_WRITE);

   if(ifip_info->ipaddr_cnt)
   {
      old_cnt = ifip_info->ipaddr_cnt;
      old_list = ifip_info->ipaddr_list;

      temp_list = (IPXS_IFIP_IP_INFO *) 
             m_MMGR_ALLOC_IPXS_DEFAULT((old_cnt)*sizeof(IPXS_IFIP_IP_INFO));
      if(temp_list == NULL)
      {
         m_IFSV_LOG_SYS_CALL_FAIL(cb->my_svc_id,IFSV_LOG_MEM_ALLOC_FAIL,0);
         if(cb->my_svc_id == NCS_SERVICE_ID_IFND)
          m_NCS_UNLOCK(&cb->ipxs_db_lock, NCS_LOCK_WRITE);
         return NCSCC_RC_FAILURE;
      }

      for(i=0; i<old_cnt; i++)
      {
         for(j=0; j<cnt; j++)
         {
            /*
            if((m_NCS_CMP_IP_ADDR(&old_list[i].ipaddr, &ippfx[j].ipaddr)) == 0) TBD:RSR */
            if(old_list[i].ipaddr.ipaddr.info.v4 == ippfx[j].ipaddr.ipaddr.info.v4) /* Temp fix */
            {
               /* Delete this */
               *del_flag = TRUE;
               break;
            }
         }
         if(j == cnt)
         {
            /* Not found in the delete list, donot delete */
            temp_list[temp_cnt] = old_list[i];
            temp_cnt++;
         }
      }

      if(del_flag == FALSE)
      {
         /* Not deleted */
         rc = NCSCC_RC_SUCCESS;
         goto free_mem;
      }
       
      if(temp_cnt != old_cnt)
      {
         if (temp_cnt)
         {
            /* Allocate the new count */
            ifip_info->ipaddr_list = (IPXS_IFIP_IP_INFO *)
               m_MMGR_ALLOC_IPXS_DEFAULT((temp_cnt)*sizeof(IPXS_IFIP_IP_INFO));

            if(!ifip_info->ipaddr_list)
            {
               ifip_info->ipaddr_list = old_list;
               m_IFSV_LOG_SYS_CALL_FAIL(cb->my_svc_id,IFSV_LOG_MEM_ALLOC_FAIL,0);
               rc = NCSCC_RC_FAILURE;
               goto free_mem;
            }

            memcpy(ifip_info->ipaddr_list, temp_list, 
                                        (temp_cnt)*sizeof(IPXS_IFIP_IP_INFO));
         }
         else
         {
            ifip_info->ipaddr_list = NULL;
         }
         ifip_info->ipaddr_cnt = temp_cnt;
         m_MMGR_FREE_IPXS_DEFAULT(old_list);
      }
      
free_mem:
      m_MMGR_FREE_IPXS_DEFAULT(temp_list);

   }
   if(cb->my_svc_id == NCS_SERVICE_ID_IFND)
     m_NCS_UNLOCK(&cb->ipxs_db_lock, NCS_LOCK_WRITE); 
   return rc;
}
/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ifip_ippfx_add_to_os
  DESCRIPTION       :   Add IF Address to OS
  ARGUMENTS         :   cb - Pointer to IPXS_CB
                    :   ifip_info - Pointer to IPXS_IFIP_INFO
                    :   ippfx - Pointer to NCS_IPPFX
                    :   uns32 count - number of addresses being added
  RETURNS           :   NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  NOTES             :   
*****************************************************************************/
uns32 
ipxs_ifip_ippfx_add_to_os(IPXS_CB *cb, NCS_IFSV_IFINDEX ifindex,IPXS_IFIP_IP_INFO *ippfx, uns32 cnt)
{


  IFSV_CB *ifsv_cb;
  uns32   ifsv_hdl;
  IFSV_INTF_DATA *intf_data;
  uns32  rc = NCSCC_RC_SUCCESS;

  ifsv_hdl = m_IFSV_GET_HDL(0, 0);

   if ((ifsv_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFND, ifsv_hdl))
      == 0)
   {
      m_IFND_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,IFSV_COMP_IFND);
      return rc;
   }
   intf_data = ifsv_intf_rec_find(ifindex, ifsv_cb);
   if(intf_data == NULL)
   {
      ncshm_give_hdl(ifsv_hdl);
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,"Coudn't get IFIP rec",0);
      return rc;
   }


   rc = ifsv_add_to_os(intf_data, ippfx, cnt, ifsv_cb);

   ncshm_give_hdl(ifsv_hdl);

   return rc;

}

uns32 ifsv_add_to_os(IFSV_INTF_DATA *intf_data,IPXS_IFIP_IP_INFO *ippfx,uns32 cnt,IFSV_CB *ifsv_cb )
{
  uns32 j;
  uns32  rc = NCSCC_RC_SUCCESS;
  uns8 install[100];
  uns8 sendGARP[100];


   if(intf_data == NULL)
   {
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,"Coudn't get IFIP rec",0);
      return rc;
   }

   if(intf_data->spt_info.type == NCS_IFSV_INTF_BINDING)
   { 
      intf_data = ifsv_intf_rec_find(intf_data->if_info.bind_master_ifindex, ifsv_cb);
      if(intf_data == NULL)
      {
         return rc;
      }
   }
  /* embedding subslot changes */ 
  if((intf_data->spt_info.shelf ==ifsv_cb->shelf) && (intf_data->spt_info.slot == ifsv_cb->slot) && (intf_data->spt_info.subslot == ifsv_cb->subslot))
  {
   
   for(j=0;j<cnt;j++)
   {
     
     sprintf(install, "/sbin/ip addr add %d.%d.%d.%d/%d dev %s",
                                                         (((ippfx[j].ipaddr.ipaddr.info.v4) & 0xff000000) >> 24),
                                                         (((ippfx[j].ipaddr.ipaddr.info.v4) & 0xff0000) >> 16), 
                                                         (((ippfx[j].ipaddr.ipaddr.info.v4) & 0xff00) >> 8),
                                                         ((ippfx[j].ipaddr.ipaddr.info.v4) & 0x000000ff),
                                                         (uns32)(ippfx[j].ipaddr.mask_len),
                                                         (intf_data->if_info.if_name));
     printf("Calling %s\n",install);
     system(install);
     /* Send Gratituous ARP */
     sprintf(sendGARP,"/usr/sbin/arping -U -c 3 -I %s %d.%d.%d.%d",(intf_data->if_info.if_name),
                                                         (((ippfx[j].ipaddr.ipaddr.info.v4) & 0xff000000) >> 24),
                                                         (((ippfx[j].ipaddr.ipaddr.info.v4) & 0xff0000) >> 16),
                                                         (((ippfx[j].ipaddr.ipaddr.info.v4) & 0xff00) >> 8),
                                                         ((ippfx[j].ipaddr.ipaddr.info.v4) & 0x000000ff));
     system(sendGARP);
    }
  }
  return rc;
}




/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ifip_ippfx_del_from_os
  DESCRIPTION       :   Delete IF Address from OS
  ARGUMENTS         :   cb - Pointer to IPXS_CB
                    :   ifip_info - Pointer to IPXS_IFIP_INFO
                    :   ippfx - Pointer to NCS_IPPFX
                    :   uns32 count - number of addresses being deleted
  RETURNS           :   NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  NOTES             :   
*****************************************************************************/
uns32 
ipxs_ifip_ippfx_del_from_os(IPXS_CB *cb,NCS_IFSV_IFINDEX ifindex ,
                            NCS_IPPFX *ippfx, uns32 cnt)
{

  IFSV_CB *ifsv_cb;
  uns32   ifsv_hdl;
  IFSV_INTF_DATA *intf_data;
  uns32  rc = NCSCC_RC_SUCCESS; 
  ifsv_hdl = m_IFSV_GET_HDL(0, 0);

   if ((ifsv_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFND, ifsv_hdl))
      == 0)
   {
      m_IFND_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,IFSV_COMP_IFND);
      return rc;
   }

   intf_data = ifsv_intf_rec_find(ifindex, ifsv_cb);

   if(intf_data == NULL)
   {
      ncshm_give_hdl(ifsv_hdl);
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,"Coudn't get IFIP rec",0);
      return rc;
   }
   rc = ifsv_del_from_os(intf_data, ippfx, cnt, ifsv_cb);
   ncshm_give_hdl(ifsv_hdl);


 return NCSCC_RC_SUCCESS;
}

uns32 ifsv_del_from_os(IFSV_INTF_DATA *intf_data,NCS_IPPFX *ippfx,uns32 cnt,IFSV_CB *ifsv_cb)
{
  uns32 j;
  uns32  rc = NCSCC_RC_SUCCESS;
  uns8 install[100];

  if(intf_data == NULL)
  {
     m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,"Coudn't get IFIP rec",0);
     return rc;
  }
  
  if(intf_data->spt_info.type == NCS_IFSV_INTF_BINDING)
  {
      intf_data = ifsv_intf_rec_find(intf_data->if_info.bind_master_ifindex, ifsv_cb);
      if(intf_data == NULL)
      {
         return rc;
      }
  }
 /* embedding subslot changes */
  if((intf_data->spt_info.shelf == ifsv_cb->shelf) && (intf_data->spt_info.slot == ifsv_cb->slot) && (intf_data->spt_info.subslot == ifsv_cb->subslot))
  {


   for(j=0;j<cnt;j++)
   {
     sprintf(install, "/sbin/ip addr del %d.%d.%d.%d/%d dev %s",
                                                         (((ippfx[j].ipaddr.info.v4) & 0xff000000) >> 24),
                                                         (((ippfx[j].ipaddr.info.v4) & 0xff0000) >> 16),
                                                         (((ippfx[j].ipaddr.info.v4) & 0xff00) >> 8),
                                                         ((ippfx[j].ipaddr.info.v4) & 0x000000ff),
                                                         (uns32)(ippfx[j].mask_len),
                                                         (intf_data->if_info.if_name));
     printf("Calling %s\n",install);
     system(install);
    }
  }
  return rc;
}
/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ifip_db_destroy
  DESCRIPTION       :   Destroys IF-IP Database 
  ARGUMENTS         :   IPXS_IPDB *iptbl  :  Pointer to IP Database
  RETURNS           :   Nothing
  NOTES             :   
*****************************************************************************/
void ipxs_ifip_db_destroy(NCS_PATRICIA_TREE *ifip_tbl, NCS_BOOL *up)
{
   if(*up)
   {
      /* RSR:TBD Destroy the nodes */

      ncs_patricia_tree_destroy(ifip_tbl);
      *up = FALSE;
      return;
   }
}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_db_index_from_addrtype_get
  DESCRIPTION       :   Get the database index from IP address type 
  ARGUMENTS         :   type : address type
  RETURNS           :   index DB index
  NOTES             :   
*****************************************************************************/
void ipxs_db_index_from_addrtype_get (uns32 *index, NCS_IP_ADDR_TYPE type)
{
   if (type == NCS_IP_ADDR_TYPE_IPV4) 
      *index = 0;
#if(NCS_IPV6 == 1)
   else if(type == NCS_IP_ADDR_TYPE_IPV6)
      *index = 1;
#endif
   else
    *index = IPXS_AFI_MAX;
}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_pfx_keylen_set
  DESCRIPTION       :   Get the key lenth from IP address type 
  ARGUMENTS         :   type : address type
  RETURNS           :   key length
  NOTES             :   
*****************************************************************************/
void ipxs_pfx_keylen_set(uns32 *keylen, NCS_IP_ADDR_TYPE type)
{
   if (type == NCS_IP_ADDR_TYPE_IPV4)
      *keylen = sizeof(NCS_IPV4_ADDR);
#if(NCS_IPV6 == 1)
   else if(type == NCS_IP_ADDR_TYPE_IPV6)
      *keylen = sizeof(NCS_IPV6_ADDR);
#endif
   else
      *keylen = 0;
}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ipdb_init
  DESCRIPTION       :   Initialize IP Database 
  ARGUMENTS         :   IPXS_IPDB *iptbl  :  Pointer to IP Database
  NOTES             :   
*****************************************************************************/
uns32 ipxs_ipdb_init(IPXS_IPDB *iptbl)
{
   NCS_PATRICIA_PARAMS     params;
   uns32          keylen=0, rc = NCSCC_RC_SUCCESS;
   NCS_IP_ADDR_TYPE type = iptbl->type;

   if (!iptbl->up)
   {
      m_IPXS_PFX_KEYLEN_SET(keylen, type);
      
      if (!keylen)
         return NCSCC_RC_FAILURE;

      params.key_size = keylen;
      params.info_size = 0;
      if ((ncs_patricia_tree_init(&iptbl->ptree, &params)) 
                                                   != NCSCC_RC_SUCCESS)
      {
         m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO, "IP DB not initiated",0);

         rc = NCSCC_RC_FAILURE;
      }
      if (rc == NCSCC_RC_SUCCESS)
         iptbl->up = TRUE;
   }

   return rc;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ipdb_destroy
  DESCRIPTION       :   Destroys IP Database 
  ARGUMENTS         :   IPXS_IPDB *iptbl  :  Pointer to IP Database
  RETURNS           :   Nothing
  NOTES             :   
*****************************************************************************/
void ipxs_ipdb_destroy(IPXS_IPDB *iptbl)
{
   if(iptbl->up)
   {
      /* RSR:TBD Destroy the nodes */

      ncs_patricia_tree_destroy(&iptbl->ptree);
      iptbl->up = FALSE;
   }

   return;
}

/******************************************************************************
  Function :  ipxs_iprec_get
  
  Purpose  :  To get the interface record & Fill it in  NCS_IFSV_INTF_REC
******************************************************************************/
uns32 ipxs_iprec_get(IPXS_CB *cb, IPXS_IP_KEY *ipkeyNet, 
                NCS_IP_ADDR_TYPE type, IPXS_IP_NODE  **ip_node, NCS_BOOL test)
{
   uns32 index = 0;
   IPXS_IP_INFO *ipinfo;

   /* Get the DB index from type */
   m_IPXS_DB_INDEX_FROM_ADDRTYPE_GET(index, type);

   if(index == IPXS_AFI_MAX)   /* This address type is not supported */
      return NCSCC_RC_NO_INSTANCE;

   *ip_node = (IPXS_IP_NODE *)ncs_patricia_tree_get(&cb->ip_tbl[index].ptree, 
      (uns8 *)ipkeyNet);


   if(test && (*ip_node == NULL))
      return NCSCC_RC_NO_INSTANCE;

   if(*ip_node == NULL)
   {
      /* Create the new node */
      *ip_node = m_MMGR_ALLOC_IPXS_IP_NODE;
      if(!*ip_node)
      {
         m_IFSV_LOG_SYS_CALL_FAIL(cb->my_svc_id,IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_FAILURE;
      }

      m_NCS_OS_MEMSET(*ip_node, 0 , sizeof(IPXS_IP_NODE));

      ipinfo = &((*ip_node)->ipinfo);

      ipinfo->keyNet = *ipkeyNet;
      
      ipinfo->addr.type = type;
      ipinfo->status = NCSMIB_ROWSTATUS_NOTREADY;
      if(ipinfo->addr.type == NCS_IP_ADDR_TYPE_IPV4)
         ipinfo->addr.info.v4 = ntohl(ipkeyNet->ip.v4);

      (*ip_node)->pat_node.key_info = (uns8*)&ipinfo->keyNet;

      if(ncs_patricia_tree_add (&cb->ip_tbl[index].ptree, (NCSCONTEXT)*ip_node) != NCSCC_RC_SUCCESS)
      {
         m_MMGR_FREE_IPXS_IP_NODE(*ip_node);
         m_IFSV_LOG_HEAD_LINE(cb->my_svc_id,IFSV_LOG_IP_TBL_ADD_FAILURE,\
                             index, 0);
         return NCSCC_RC_FAILURE;
      }
      else
         m_IFSV_LOG_HEAD_LINE_NORMAL(cb->my_svc_id,IFSV_LOG_IP_TBL_ADD_SUCCESS,\
                             index, 0);
   }

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ip_record_del
  DESCRIPTION       :   Add IF-IP Record
  ARGUMENTS         :   NCS_PATRICIA_TREE *ifip_tbl  :  Pointer to IP Database
                    :   Pointer to IPXS_IP_INFO.
  NOTES             :   
*****************************************************************************/
uns32  
ipxs_ip_record_del(IPXS_CB *cb, IPXS_IPDB *ip_db, IPXS_IP_NODE *ip_node)
{

   /* Delete the node from Tree */
   if((ip_db->up) && ip_node)
      ncs_patricia_tree_del(&ip_db->ptree, &ip_node->pat_node);

   /* Free the node */
   if(ip_node)
      m_MMGR_FREE_IPXS_IP_NODE(ip_node);

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ip_record_list_del
  DESCRIPTION       :   Remove the IP records for all the IP address present
                        in the ifip_info->addr_list
  NOTES             :   
*****************************************************************************/
void ipxs_ip_record_list_del(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info)
{
   uns32 i, index=0;
   NCS_IP_ADDR_TYPE  type=0;
   IPXS_IP_KEY       ip_key;
   IPXS_IP_NODE      *ip_node; 
   
   if(ifip_info->ipaddr_cnt == 0)
      return;       
   if(cb->my_svc_id == NCS_SERVICE_ID_IFD) 
   {
      for(i=0; i<ifip_info->ipaddr_cnt; i++)
      {
         type = ifip_info->ipaddr_list[i].ipaddr.ipaddr.type;

         ipxs_get_ipkey_from_ipaddr(&ip_key, &ifip_info->ipaddr_list[i].ipaddr.ipaddr);

         ipxs_iprec_get(cb, &ip_key, type, &ip_node, TRUE);

         /* Get the DB index from type */
         m_IPXS_DB_INDEX_FROM_ADDRTYPE_GET(index, type);

         if(index == IPXS_AFI_MAX)   /* This address type is not supported */
            continue;

         /* Delete the record */
         ipxs_ip_record_del(cb, &cb->ip_tbl[index], ip_node);
     }
   }

  /* Free the IP address list from IF-IP info */
  m_MMGR_FREE_IPXS_DEFAULT(ifip_info->ipaddr_list);
  ifip_info->ipaddr_cnt = 0;
  ifip_info->ipam = ifip_info->ipam & ~(NCS_IPXS_IPAM_ADDR);
  m_IFSV_LOG_HEAD_LINE_NORMAL(cb->my_svc_id,IFSV_LOG_IFIP_NODE_ID_TBL_DEL_SUCCESS,\
                              index, 0);

}

void ipxs_get_ipkey_from_ipaddr(IPXS_IP_KEY  *ip_key, NCS_IP_ADDR *addr)
{
   switch(addr->type)
   {
   case NCS_IP_ADDR_TYPE_IPV4:
      ip_key->ip.v4 = htonl(addr->info.v4);
      break;
#if (NCS_IPV6 == 1)
   case NCS_IP_ADDR_TYPE_IPV6:
      memcpy(&ip_key->ip.v6, &addr->info.v6, sizeof(NCS_IPV6_ADDR));
      break;
#endif
   default:
      break;
   }
}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_update_ifadd
  DESCRIPTION       :   Process interface Add Update from IFSV
*****************************************************************************/
uns32 ipxs_update_ifadd(NCS_SERVICE_ID svc_id, IFSV_INTF_DATA *intf_data)
{
   uns32             rc = NCSCC_RC_SUCCESS;
   IPXS_CB           *cb = NULL;
   uns32             ipxs_hdl;
   IPXS_IFIP_NODE    *ifip_node;
 
   /* Get the Handle Manager Handle */
   ipxs_hdl = m_IPXS_CB_HDL_GET();

   cb = ncshm_take_hdl(svc_id, ipxs_hdl);

   if (cb == NULL)
   {
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO, "ncshm_take_hdl returned NULL",0);
      return NCSCC_RC_FAILURE;
   }
 
   /* Add the record to IF-IP Table */
   ifip_node = m_MMGR_ALLOC_IPXS_IFIP_NODE;
   if(!ifip_node)
   {
      ncshm_give_hdl(ipxs_hdl);
      m_IFSV_LOG_SYS_CALL_FAIL(cb->my_svc_id,IFSV_LOG_MEM_ALLOC_FAIL,0);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_OS_MEMSET(ifip_node, 0, sizeof(IPXS_IFIP_NODE));

   ifip_node->ifip_info.ifindexNet = intf_data->if_index;
   ifip_node->ifip_info.is_v4_unnmbrd = NCS_SNMP_FALSE;

   rc = ipxs_ifip_record_add(cb, &cb->ifip_tbl,  ifip_node);

   if(rc != NCSCC_RC_SUCCESS)
   {
      ncshm_give_hdl(ipxs_hdl);
      m_MMGR_FREE_IPXS_IFIP_NODE(ifip_node);
      return rc;
   }

   ncshm_give_hdl(ipxs_hdl);
   return rc;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_update_ifdel
  DESCRIPTION       :   Process interface Add Update from IFSV
*****************************************************************************/
uns32 ipxs_update_ifdel(NCS_SERVICE_ID svc_id, uns32 if_index)
{
   uns32             rc = NCSCC_RC_SUCCESS;
   IPXS_CB           *cb = NULL;
   uns32             ipxs_hdl;
   IPXS_IFIP_NODE    *ifip_node;

   /* Get the Handle Manager Handle */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );

   cb = ncshm_take_hdl(svc_id, ipxs_hdl);

   if (cb == NULL)
   {
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO, "ncshm_take_hdl returned NULL",0);
      return NCSCC_RC_FAILURE;
   }
   ifip_node = ipxs_ifip_record_get(&cb->ifip_tbl, if_index);
   if(!ifip_node)
   {
      ncshm_give_hdl(ipxs_hdl);
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,\
                       "Couldn't find IFIP info for index : ",if_index);
      return NCSCC_RC_FAILURE;
   }

   ipxs_ifip_record_del(cb, &cb->ifip_tbl,  ifip_node);

   ncshm_give_hdl(ipxs_hdl);
   return rc;
}


/****************************************************************************
 * Name          : ipxs_ifa_app_if_info_indicate
 *
 * Description   : This is the function to inform IPXS applications about 
 *                 the IF info Changes
 *                 
 *
 * Arguments     : actual_data - The data to be indicated to the application. 
 *               : intf_evt    - Record event to be sent (Add/Modify/Delete).
 *                 attr        - Attributes which has been changed.
 *                 cb          - IfSv Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ipxs_ifa_app_if_info_indicate(IFSV_INTF_DATA *actual_data, 
                            IFSV_INTF_REC_EVT intf_evt, uns32 attr,
                            IFSV_CB *cb)
{
   IPXS_EVT          send_evt;
   IFSV_EVT          evt;
   NCS_IPXS_INTF_REC send_rec;
   uns32             rc = NCSCC_RC_SUCCESS;
   IPXS_IFIP_NODE    *ifip_node;
   IPXS_CB           *ipxs_cb;
   uns32             ipxs_hdl;

   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IPXS_EVT));
   m_NCS_OS_MEMSET(&send_rec, 0, sizeof(NCS_IPXS_INTF_REC));

   /* Fill the pointers */
   evt.info.ipxs_evt = (NCSCONTEXT)&send_evt;
   send_evt.info.agent.ip_add.i_ipinfo = &send_rec;
   evt.type = IFSV_IPXS_EVT;

   /* Is this interface is local to this node or Not */
   if(m_NCS_NODE_ID_FROM_MDS_DEST(actual_data->current_owner_mds_destination)==
       m_NCS_NODE_ID_FROM_MDS_DEST(cb->my_dest))
     send_rec.is_lcl_intf = TRUE;
   else
     send_rec.is_lcl_intf = FALSE;
   

   switch(intf_evt)
   {
   case IFSV_INTF_REC_ADD:
      send_evt.type = IPXS_EVT_NDTOA_IFIP_ADD;
      send_rec.if_index = actual_data->if_index;
      send_rec.spt = actual_data->spt_info;
      rc = ifsv_intf_info_cpy(&actual_data->if_info, 
                &send_rec.if_info, IFSV_MALLOC_FOR_MDS_SEND);
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,\
                      "Rec Add info being sent to App. Index is ",\
                       actual_data->if_index);

      break;

   case IFSV_INTF_REC_MODIFY:
      send_evt.type = IPXS_EVT_NDTOA_IFIP_UPD;
      send_rec.if_index = actual_data->if_index;
      send_rec.spt = actual_data->spt_info;
      send_rec.if_info.if_am = attr;
      rc = ifsv_intf_info_cpy(&actual_data->if_info, 
                &send_rec.if_info, IFSV_MALLOC_FOR_MDS_SEND);
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,\
                      "Rec Upd info being sent to App. Index is ",\
                       actual_data->if_index);

      break;

   case IFSV_INTF_REC_DEL:
   case IFSV_INTF_REC_MARKED_DEL:
      send_evt.type = IPXS_EVT_NDTOA_IFIP_DEL;
      send_rec.if_index = actual_data->if_index;
      send_rec.spt = actual_data->spt_info;
      rc = ifsv_intf_info_cpy(&actual_data->if_info, 
                &send_rec.if_info, IFSV_MALLOC_FOR_MDS_SEND);
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,\
                      "Rec Destroy info being sent to App. Index is ",\
                       actual_data->if_index);


      if(rc != NCSCC_RC_SUCCESS)
         break;

      /* Also copy the existing IP info */
      ipxs_hdl = m_IPXS_CB_HDL_GET( );
      ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);
      if(ipxs_cb)
         ifip_node = ipxs_ifip_record_get(&ipxs_cb->ifip_tbl, send_rec.if_index);
      else
      {
         ncshm_give_hdl(ipxs_hdl);
         m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,\
                          "ncshm_give_hdl returned NULL pointer", 0);
         rc = NCSCC_RC_FAILURE;
         break;
      }


      if(ifip_node)
      {
         rc = ipxs_ipinfo_cpy_for_del(&ifip_node->ifip_info, 
                                              &send_rec.ip_info);
      }
      if(rc != NCSCC_RC_SUCCESS)
      {
         ncshm_give_hdl(ipxs_hdl);
         break;
      }

      if(intf_evt == IFSV_INTF_REC_DEL)
      {
        /* Delete the IPXS IP Record */
        ipxs_ifip_record_del(ipxs_cb, &ipxs_cb->ifip_tbl,  ifip_node);
      }
       ncshm_give_hdl(ipxs_hdl);
       break;

   default:
         rc = NCSCC_RC_FAILURE;
         break;
   }

   if(rc != NCSCC_RC_SUCCESS)
      goto free_mem;
      
   /* Broadcast the event to all the IFAs in on the same node */
   ifsv_mds_scoped_send(cb->my_mds_hdl, NCSMDS_SCOPE_INTRANODE, 
         NCSMDS_SVC_ID_IFND, &evt, NCSMDS_SVC_ID_IFA);
free_mem:
   if(rc == NCSCC_RC_SUCCESS)
   {
      ifsv_intf_info_free(&send_rec.if_info, IFSV_MALLOC_FOR_MDS_SEND);
      if(send_rec.ip_info.delip_cnt)
      {
        if(send_rec.ip_info.delip_list)
          ncs_ipxs_ippfx_list_free((void *)send_rec.ip_info.delip_list);
      }   
   }

   return (rc);
}


/****************************************************************************
 * Name          : ipxs_ifa_app_svd_info_indicate
 *
 * Description   : This is the function to inform IPXS applications about 
 *                 the SVDinfo Changes
 *                 
 *
 * Arguments     : actual_data - The data to be indicated to the application. 
 *               : intf_evt    - Record event to be sent (Add/Modify/Delete).
 *                 attr        - Attributes which has been changed.
 *                 cb          - IfSv Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ipxs_ifa_app_svd_info_indicate(IFSV_CB *cb, IFSV_INTF_DATA *actual_data, 
                            NCS_IFSV_SVC_DEST_UPD *svc_dest)
{
   IPXS_EVT          send_evt;
   IFSV_EVT          evt;
   NCS_IPXS_INTF_REC send_rec;
   uns32    rc = NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IPXS_EVT));
   m_NCS_OS_MEMSET(&send_rec, 0, sizeof(NCS_IPXS_INTF_REC));

   /* Fill the pointers */
   evt.info.ipxs_evt = (NCSCONTEXT)&send_evt;
   send_evt.info.agent.ip_add.i_ipinfo = &send_rec;
   evt.type = IFSV_IPXS_EVT;
   send_evt.type = IPXS_EVT_NDTOA_IFIP_UPD;
   send_rec.if_index = actual_data->if_index;
   send_rec.spt = actual_data->spt_info;
   send_rec.if_info.if_am = NCS_IFSV_IAM_SVDEST;

   m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,\
                      "SVCD info being sent to App. Index is ",\
                       actual_data->if_index);
   
   if(svc_dest->i_type == NCS_IFSV_SVCD_ADD)
   {
      send_rec.if_info.addsvd_cnt = 1;
      send_rec.if_info.addsvd_list = 
         (NCS_SVDEST *)m_MMGR_ALLOC_IFSV_NCS_SVDEST(1);

      if(send_rec.if_info.addsvd_list == 0)
      {
         m_IFSV_LOG_SYS_CALL_FAIL(cb->my_svc_id,IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_OUT_OF_MEM;
      }
      memcpy(send_rec.if_info.addsvd_list, 
                      &svc_dest->i_svdest, sizeof(NCS_SVDEST));
   }
   if(svc_dest->i_type == NCS_IFSV_SVCD_DEL)
   {
      send_rec.if_info.delsvd_cnt = 1;
      send_rec.if_info.delsvd_list = 
         (NCS_SVDEST *) m_MMGR_ALLOC_IFSV_NCS_SVDEST(1);
      if(send_rec.if_info.delsvd_list == 0)
      {
         m_IFSV_LOG_SYS_CALL_FAIL(cb->my_svc_id,IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_OUT_OF_MEM;
      } 
      memcpy(send_rec.if_info.delsvd_list, 
                      &svc_dest->i_svdest, sizeof(NCS_SVDEST));
   }
   else
      return NCSCC_RC_FAILURE;
      
   /* Broadcast the event to all the IFAs in on the same node */
   rc = ifsv_mds_scoped_send(cb->my_mds_hdl, NCSMDS_SCOPE_INTRANODE, 
         NCSMDS_SVC_ID_IFND, &evt, NCSMDS_SVC_ID_IFA);

   if(rc != NCSCC_RC_SUCCESS)
   {
      if(send_rec.if_info.addsvd_list)
         m_MMGR_FREE_IFSV_NCS_SVDEST(send_rec.if_info.addsvd_list);

      if(send_rec.if_info.delsvd_list)
         m_MMGR_FREE_IFSV_NCS_SVDEST(send_rec.if_info.delsvd_list);
   }

   return (rc);
}

/*****************************************************************************
  PROCEDURE NAME    :   ncs_ipxs_intf_rec_alloc
  DESCRIPTION       :   Function to Memory alloc the interface record.
*****************************************************************************/
NCS_IPXS_INTF_REC* ncs_ipxs_intf_rec_alloc( )
{
   NCS_IPXS_INTF_REC *rec;

   rec = m_MMGR_ALLOC_NCS_IPXS_INTF_REC;

   if(rec == NULL)
       m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);

   return(rec);
}

/*****************************************************************************
  PROCEDURE NAME    :   ncs_ipxs_intf_rec_free
  DESCRIPTION       :   Function to Free the interface record.
*****************************************************************************/
void ncs_ipxs_intf_rec_free(NCS_IPXS_INTF_REC *i_intf_rec)
{
   m_MMGR_FREE_NCS_IPXS_INTF_REC(i_intf_rec);
   return;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_intf_rec_attr_free
  DESCRIPTION       :   Function to Free the interface record & its internal
                        pointers.
*****************************************************************************/
void ipxs_intf_rec_attr_free(NCS_IPXS_INTF_REC *i_intf_rec, NCS_BOOL i_is_rec_free)
{
  if(m_NCS_IPXS_IS_IPAM_ADDR_SET(i_intf_rec->ip_info.ip_attr))
  {
    /* Free the internal record pointers */
    if(i_intf_rec->ip_info.addip_cnt)
    {
      if(i_intf_rec->ip_info.addip_list)
         ncs_ipxs_ippfx_list_free((void *)i_intf_rec->ip_info.addip_list);
    }
    
    if(i_intf_rec->ip_info.delip_cnt)
    {
      if(i_intf_rec->ip_info.delip_list)
        ncs_ipxs_ippfx_list_free((void *)i_intf_rec->ip_info.delip_list);
    }
  }
  
  if(m_IFSV_IS_SVDEST_ATTR_SET(i_intf_rec->if_info.if_am))
  {
    if(i_intf_rec->if_info.addsvd_cnt)
    {
     if(i_intf_rec->if_info.addsvd_list)
        ncs_ifsv_svdest_list_free(i_intf_rec->if_info.addsvd_list);
    }
  
    if(i_intf_rec->if_info.delsvd_cnt)
    {
      if(i_intf_rec->if_info.delsvd_list)
       ncs_ifsv_svdest_list_free(i_intf_rec->if_info.delsvd_list);
    }
  }

   /* Free the record pointer also */
   if(i_is_rec_free)
   {
      ncs_ipxs_intf_rec_free(i_intf_rec);
   }

   return;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_intf_rec_list_free
  DESCRIPTION       :   Function to Free the interface record list
*****************************************************************************/
void ipxs_intf_rec_list_free(NCS_IPXS_INTF_REC *i_intf_rec)
{
   NCS_IPXS_INTF_REC    *rec, *next_rec;

   rec = i_intf_rec;

   while(rec)
   {
      next_rec = rec->next;
      ipxs_intf_rec_attr_free(rec, TRUE);
      rec = next_rec;
   }

   return;
}


/*****************************************************************************
  PROCEDURE NAME    :   ncs_ipxs_ippfx_list_alloc
  DESCRIPTION       :   Function to alloc the IPPFX list.
*****************************************************************************/
IPXS_IFIP_IP_INFO* ncs_ipxs_ippfx_list_alloc(uns32 i_pfx_cnt)
{
   IPXS_IFIP_IP_INFO   *pfx_list;
   /* Allocate the IPPFX list */
   pfx_list = (IPXS_IFIP_IP_INFO *) m_MMGR_ALLOC_IPXS_DEFAULT(i_pfx_cnt*sizeof(IPXS_IFIP_IP_INFO));
   if(pfx_list == NULL)
       m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
   return pfx_list;
}

/*****************************************************************************
  PROCEDURE NAME    :   ncs_ipxs_ippfx_list_free
  DESCRIPTION       :   Function to free the IPPFX list.
*****************************************************************************/
void ncs_ipxs_ippfx_list_free(void  *i_pfx)
{
   m_MMGR_FREE_IPXS_DEFAULT(i_pfx);
   return;
}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_intf_rec_cpy
  DESCRIPTION       :   Function to copy the interface record.
*****************************************************************************/
uns32 ipxs_intf_rec_cpy(NCS_IPXS_INTF_REC *src, NCS_IPXS_INTF_REC *dest,
                        IFSV_MALLOC_USE_TYPE purpose)
{
   /* Copy the record */
   memcpy(dest, src, sizeof(NCS_IPXS_INTF_REC));

   /* Copy the IP info */
   ipxs_ipinfo_cpy(&src->ip_info, &dest->ip_info, purpose);


   /* Copy the internal pointer SVC Dest */
   ifsv_intf_info_svd_list_cpy(&src->if_info, &dest->if_info,
                                    purpose);
   
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ipinfo_cpy
  DESCRIPTION       :   Function to copy the ip information 
*****************************************************************************/
uns32 ipxs_ipinfo_cpy(NCS_IPXS_IPINFO *src, 
                      NCS_IPXS_IPINFO *dest, 
                      IFSV_MALLOC_USE_TYPE purpose)
{
   /* Copy the record */
   m_NCS_OS_MEMSET(dest, 0, sizeof(NCS_IPXS_IPINFO));

   memcpy(dest, src, sizeof(NCS_IPXS_IPINFO));
   if(m_NCS_IPXS_IS_IPAM_ADDR_SET(src->ip_attr))
   {
   /* Internal Use */
   if(src->addip_cnt)
   {
      dest->addip_list = (IPXS_IFIP_IP_INFO  *)
              m_MMGR_ALLOC_IPXS_DEFAULT(src->addip_cnt*sizeof(IPXS_IFIP_IP_INFO));

      if(dest->addip_list == 0)
      {
         m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_OUT_OF_MEM;
      }
   }

   if(src->delip_cnt)
   {
      dest->delip_list = (NCS_IPPFX *)
              m_MMGR_ALLOC_IPXS_DEFAULT(src->delip_cnt*sizeof(NCS_IPPFX));

      if(dest->delip_list == 0)
      {
         m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_OUT_OF_MEM;
      }
   }

   /* Copy the internal pointers */
   if(dest->addip_list)
   {
      memcpy(dest->addip_list, src->addip_list,
                     (src->addip_cnt * sizeof(IPXS_IFIP_IP_INFO)));
   }

   dest->addip_cnt = src->addip_cnt;

   if(dest->delip_list)
   {
      memcpy(dest->delip_list, src->delip_list,
                     (src->delip_cnt * sizeof(NCS_IPPFX)));
   }

   dest->delip_cnt = src->delip_cnt;
  }/* if(m_NCS_IPXS_IS_IPAM_ADDR_SET(src->ip_attr) */

   return NCSCC_RC_SUCCESS;
}


void ipxs_ipinfo_free(NCS_IPXS_IPINFO *ip_info, IFSV_MALLOC_USE_TYPE purpose)
{

   if(ip_info->addip_list)
   {
      m_MMGR_FREE_IPXS_DEFAULT(ip_info->addip_list);
      ip_info->addip_list = 0;
      ip_info->addip_cnt = 0;
   }

   if(ip_info->delip_list)
   {
      m_MMGR_FREE_IPXS_DEFAULT(ip_info->delip_list);
      ip_info->delip_list = 0;
      ip_info->delip_list = 0;
   }

   return;
}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ifsv_ifip_info_attr_cpy
  DESCRIPTION       :   Selectivey copies the attributes of structure
                        IPXS_IFIP_INFO.
*****************************************************************************/
uns32 ipxs_ifsv_ifip_info_attr_cpy(IPXS_IFIP_INFO *src, NCS_IPXS_IPINFO *dest)
{

   m_NCS_OS_MEMSET(dest, 0, sizeof(NCS_IPXS_IPINFO));

   dest->ip_attr = src->ipam;

   if(m_NCS_IPXS_IS_IPAM_ADDR_SET(src->ipam))
   {
      dest->addip_cnt = src->ipaddr_cnt;
      dest->addip_list = (IPXS_IFIP_IP_INFO *)
         m_MMGR_ALLOC_IPXS_DEFAULT(src->ipaddr_cnt*sizeof(IPXS_IFIP_IP_INFO));

      if(dest->addip_list)
          memcpy(dest->addip_list, src->ipaddr_list,
                   (src->ipaddr_cnt * sizeof(IPXS_IFIP_IP_INFO)));
      else
      {
         m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_OUT_OF_MEM;
      }
   }

   if(m_NCS_IPXS_IS_IPAM_UNNMBD_SET(src->ipam))
   {
      dest->is_v4_unnmbrd = src->is_v4_unnmbrd;
   }
   if(m_NCS_IPXS_IS_IPAM_RRTRID_SET(src->ipam))
   {
      dest->rrtr_rtr_id = src->rrtr_rtr_id;
   }

   if(m_NCS_IPXS_IS_IPAM_RMT_AS_SET(src->ipam))
   {
      dest->rrtr_as_id = src->rrtr_as_id;
   }

   if(m_NCS_IPXS_IS_IPAM_RMTIFID_SET(src->ipam))
   {
      dest->rrtr_if_id = src->rrtr_if_id;
   }

   if(m_NCS_IPXS_IS_IPAM_ADDR_SET(src->ipam))
   {
      dest->rrtr_ipaddr = src->rrtr_ipaddr;
   }

   if(m_NCS_IPXS_IS_IPAM_UD_1_SET(src->ipam))
   {
      dest->ud_1 = src->ud_1;
   }

   return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
  PROCEDURE NAME    :   ipxs_ipinfo_cpy_for_del
  DESCRIPTION       :   Function to copy the ip information for sending
                        the delete req to applications.
*****************************************************************************/
static uns32 ipxs_ipinfo_cpy_for_del(IPXS_IFIP_INFO *src, NCS_IPXS_IPINFO *dest)
{
   uns32 count=0;

   /* Copy the record */
   m_NCS_OS_MEMSET(dest, 0, sizeof(NCS_IPXS_IPINFO));

   dest->ip_attr = src->ipam;

   
   if(m_NCS_IPXS_IS_IPAM_ADDR_SET(src->ipam))
   {
      /* Copy the IP address into delete list */
      dest->delip_cnt = src->ipaddr_cnt;
      dest->delip_list = (NCS_IPPFX *)
         m_MMGR_ALLOC_IPXS_DEFAULT(src->ipaddr_cnt*sizeof(NCS_IPPFX));

      if(dest->delip_list)
      {
        /* Size of delip_list and ipaddr_list are different. */
        for(count=0; count < dest->delip_cnt; count++)
          memcpy(&dest->delip_list[count], &src->ipaddr_list[count],
                   (sizeof(NCS_IPPFX)));
      }
      else
      {
         m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_OUT_OF_MEM;
      }
   }

   if(m_NCS_IPXS_IS_IPAM_UNNMBD_SET(src->ipam))
   {
      dest->is_v4_unnmbrd = src->is_v4_unnmbrd;
   }

   if(m_NCS_IPXS_IS_IPAM_RRTRID_SET(src->ipam))
   {
      dest->rrtr_rtr_id = src->rrtr_rtr_id;
   }

   if(m_NCS_IPXS_IS_IPAM_RMT_AS_SET(src->ipam))
   {
      dest->rrtr_as_id = src->rrtr_as_id;
   }

   if(m_NCS_IPXS_IS_IPAM_RMTIFID_SET(src->ipam))
   {
      dest->rrtr_if_id = src->rrtr_if_id;
   }

   if(m_NCS_IPXS_IS_IPAM_ADDR_SET(src->ipam))
   {
      dest->rrtr_ipaddr = src->rrtr_ipaddr;
   }

   if(m_NCS_IPXS_IS_IPAM_UD_1_SET(src->ipam))
   {
      dest->ud_1 = src->ud_1;
   }

#if (NCS_VIP == 1)
   if(m_NCS_IPXS_IS_IPAM_VIP_SET(src->ipam))
   {
      m_NCS_STRCPY(&dest->intfName,&src->intfName);
      dest->shelfId    = src->shelfId;
      dest->slotId     = src->slotId;
      /* embedding subslot changes */
      dest->subslotId     = src->subslotId;
      dest->nodeId     = src->nodeId;
   }

#endif

   return NCSCC_RC_SUCCESS;
}
void ncs_ifsv_svdest_list_free(NCS_SVDEST *i_svd_list)
{
   m_MMGR_FREE_IFSV_NCS_SVDEST(i_svd_list);
   return;
}

