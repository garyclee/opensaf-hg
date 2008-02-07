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
  FILE NAME: IFSVIFAP.C

  DESCRIPTION: This file has the IFAP module, which allocates the ifindex and
               maintains the ifindex globally accross the box.

  FUNCTIONS INCLUDED in this module:
  ifd_ifap_init ........... IFAP init.  
  ifd_ifap_destroy ........ IFAP Destroy.  
  ifd_ifap_ifindex_alloc... Allocate Ifindex for the intf created on IfD.  
  ifd_ifap_ifindex_free.... Free Ifindex for the intf created on IfD.  

******************************************************************************/

#include "ifsvifap.h"
#include "ncssysf_def.h"
#include "ifd.h"
struct ifap_info g_ifap_if_pool_list;

#define m_IFAP_GET_IF_POOL_LIST (&g_ifap_if_pool_list)

static uns32 ifap_ifindex_cmp(uns8 *key1, uns8 *key2);

/****************************************************************************
 * Name          : ifd_ifap_init
 *
 * Description   : This is the function which initializes IFAP.
 *                 
 *
 * Arguments     : info  - Info passed for IFAP to be initalized. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifap_init(NCSCONTEXT info)
{
   IFAP_INFO *ifap_info = m_IFAP_GET_IF_POOL_LIST;
   NCS_DB_LINK_LIST *db_list = &ifap_info->free_if_pool;

   m_NCS_MEMSET(ifap_info,0,sizeof(IFAP_INFO));
   /* initialize the doubly link list for interface resolving que*/
   db_list->order                        = NCS_DBLIST_ASSCEND_ORDER;
   db_list->cmp_cookie                   = ifap_ifindex_cmp;
   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifd_ifap_destroy
 *
 * Description   : This is the function which destroy IFAP.
 *                 
 *
 * Arguments     : info  - Info passed for IFAP to be initalized. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifap_destroy (NCSCONTEXT info)
{
   IFAP_INFO *ifap_info = m_IFAP_GET_IF_POOL_LIST;
   NCS_DB_LINK_LIST db_list = ifap_info->free_if_pool;
   NCS_DB_LINK_LIST_NODE *node;

   while((node = ncs_db_link_list_pop(&db_list)) != NULL)
   {
      m_IFAP_IFPOOL_FREE(node);
   }   
   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifap_ifindex_cmp
 *
 * Description   : This is the function which compare the ifindex passed.
 *
 * Arguments     : key1  - 1st key.
 *                 key2  - 2nd key.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifap_ifindex_cmp (uns8 *key1, uns8 *key2)
{   
   uns32 ifindex1 = *((uns32*)key1);
   uns32 ifindex2 = *((uns32*)key2);

   if (ifindex1  > ifindex2)
      return(1);
   else if (ifindex1  < ifindex2)
      return(2);
   else
      return(0);   
}

/****************************************************************************
 * Name          : ifd_ifap_ifindex_alloc
 *
 * Description   : This is the function which allocates ifindex for the given 
 *                 slot, port and type on IfD. Since this is the target 
 *                 service, any kind of ifindex allocator can be replace this
 *                 
 *
 * Arguments     : spt_map    - Slot Port Type Vs Ifindex maping
 *               : info       - IFAP info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifap_ifindex_alloc (NCS_IFSV_SPT_MAP *spt_map, 
                        NCSCONTEXT info)
{   
   IFAP_IFINDEX_FREE_POOL *ifindex_pool;   
   IFAP_INFO *ifap_info = m_IFAP_GET_IF_POOL_LIST;   
 
   ifindex_pool = (IFAP_IFINDEX_FREE_POOL *)
         ncs_db_link_list_pop(&ifap_info->free_if_pool);

   if (ifindex_pool == NULL)
   {
      /* there is no free ifindex so increment the max ifindex allocated at present */         
      if (ifap_info->max_ifindex == 0xffffffff)
      {
         return (NCSCC_RC_FAILURE);
      }
      ifap_info->max_ifindex++;
      spt_map->if_index = ifap_info->max_ifindex;         
   } else
   {
      spt_map->if_index = ifindex_pool->if_index;
      m_IFAP_IFPOOL_FREE(ifindex_pool);
   }       
   return (NCSCC_RC_SUCCESS);
}



/****************************************************************************
 * Name          : ifd_ifap_ifindex_free
 *
 * Description   : This is the function which frees ifindex allocated. This 
 *                 function will keep the ifindex which is freed in the ifindex
 *                 free pool, so that next time, it would allocate the ifindex
 *                 from this free pool. 
 *                 
 *
 * Arguments     : ifindex      - ifindex to be freed.
 *               : info         - Ifap Info to be passed.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifap_ifindex_free (NCS_IFSV_IFINDEX ifindex,
                       NCSCONTEXT info)
{
   IFAP_INFO *ifap_info = m_IFAP_GET_IF_POOL_LIST;
   IFAP_IFINDEX_FREE_POOL *ifindex_pool;
   uns32 res;
   NCS_DB_LINK_LIST_NODE *del_node = NULL;

   if(ifindex == 0)
   { 
     m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFINDEX_FREE_FAILURE,ifindex,0);
     return(NCSCC_RC_FAILURE);
   }

   /* Check that this ifindex is already in the free list. */
   del_node = ncs_db_link_list_find(&ifap_info->free_if_pool,(uns8 *)&ifindex);

   if (del_node != NULL)
   {
     m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFINDEX_FREE_FAILURE,ifindex,1);
     return(NCSCC_RC_SUCCESS);
   }

   if(ifindex > ifap_info->max_ifindex)
   {
     m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFINDEX_FREE_FAILURE,ifindex,ifap_info->max_ifindex);
     return(NCSCC_RC_SUCCESS);
   }

   /* put the ifindex in the free ifindex pool */
   ifindex_pool = m_IFAP_IFPOOL_ALLOC;   
   if (ifindex_pool == NULL)
      return(NCSCC_RC_FAILURE);

   m_NCS_MEMSET(ifindex_pool, 0, sizeof(IFAP_IFINDEX_FREE_POOL));
   ifindex_pool->if_index = ifindex;
   ifindex_pool->q_node.key = (uns8*)&ifindex_pool->if_index;
   res = ncs_db_link_list_add(&ifap_info->free_if_pool, &ifindex_pool->q_node);
   return (res);
}

/****************************************************************************
 * Name          : ifd_ifap_ifindex_list_get
 *
 * Description   : This is the function, which sends information related to 
 *                 interface indexes, which are allocated and which are free, 
 *                 to Standby IfD in the cold/warm sync.
 *
 * Arguments     : msg - Pointer to the memory allocated by the calling 
 *                       function.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifap_ifindex_list_get (uns8 *msg)
{
 IFAP_INFO_LIST_A2S *list;
 IFAP_INFO *ifap_info = m_IFAP_GET_IF_POOL_LIST;
 NCS_DB_LINK_LIST_NODE *start_ptr = NULL;
 uns32 *free_members = NULL;
 uns32 num_free_msg = 0;

 list = (IFAP_INFO_LIST_A2S *)msg;

 start_ptr = ifap_info->free_if_pool.start_ptr;
 free_members = list->free_list;
 num_free_msg = list->num_free_ifindex;

 /* Fill the list. */
 list->max_ifindex = ifap_info->max_ifindex;

 while(num_free_msg)
 {
   num_free_msg--;
   /* Fill the free indexes. */
   *free_members = *((uns32 *)(start_ptr->key)); 
   /* Increase the start_ptr. */
   start_ptr = start_ptr->next;
   /* Increase the free member pointer. */
   free_members++;
 }/* while(start_ptr) */
  
 return NCSCC_RC_SUCCESS;

} /* The function ifd_ifap_ifindex_list_get() ends here. */

/****************************************************************************
 * Name          : ifd_ifap_max_num_free_ifindex_get
 *
 * Description   : This function returns the max number of interface indexes
 *                 currently present in the free pool. This function is called 
 *                 when cold/warm sync response is tobe sent and memory has to 
 *                 allocated for storing the freed if indexes.
 *
 * Arguments     : Pointer to the variable, which will store the max num of 
 *                 free if indexes.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifap_max_num_free_ifindex_get (uns32 *msg)
{
 IFAP_INFO *ifap_info = m_IFAP_GET_IF_POOL_LIST;
 NCS_DB_LINK_LIST_NODE *start_ptr = NULL;
 
 if(ifap_info == NULL)
   return NCSCC_RC_FAILURE;

 /* Fill the value to zero. */
 *msg = 0;

 start_ptr = ifap_info->free_if_pool.start_ptr;

 while(start_ptr)
 {
   /* Increase the start_ptr. */
   start_ptr = start_ptr->next;

   /* Increament the number of mesages. */
   (*msg)++;
 }/* while(start_ptr) */

 return NCSCC_RC_SUCCESS;

} /* The function ifd_ifap_max_num_free_ifindex_get() ends here. */

/****************************************************************************
 * Name          : ifd_ifap_max_num_ifindex_and_num_free_ifindex_get
 *
 * Description   : This function returns the max number of interface indexes
 *                 currently present in the free pool and max ifindex, which 
 *                 has been allocated till now. This function is called
 *                 when warm sync response is to be sent.
 *
 * Arguments     : Pointers to the variable, which will store the max num of
 *                 free if indexes and max ifindex.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifap_max_num_ifindex_and_num_free_ifindex_get(uns32 *max_ifindex, uns32 *num_free_ifindex)
{
 IFAP_INFO *ifap_info = m_IFAP_GET_IF_POOL_LIST;
 
 if(ifap_info == NULL)
   return NCSCC_RC_FAILURE;

 *max_ifindex = ifap_info->max_ifindex;
 *num_free_ifindex = ifap_info->free_if_pool.n_nodes;

  return NCSCC_RC_SUCCESS;
} /* The function ifd_ifap_max_num_free_ifindex_get() ends here. */

/****************************************************************************
 * Name          : ifd_ifap_ifindex_free_find
 *
 * Description   : This function finds whether ifindex is in free list. 
 *
 * Arguments     : ifindex 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifap_ifindex_free_find(uns32 ifindex)
{
 IFAP_INFO *ifap_info = m_IFAP_GET_IF_POOL_LIST;
 NCS_DB_LINK_LIST_NODE *del_node = NULL;

 if(ifap_info == NULL)
   return NCSCC_RC_FAILURE;

 /* Check that this ifindex is already in the free list. */
 del_node = ncs_db_link_list_find(&ifap_info->free_if_pool,(uns8 *)&ifindex);

 if (del_node != NULL)
 {
   return(NCSCC_RC_SUCCESS);
 }
  return NCSCC_RC_FAILURE;
} /* The function ifd_ifap_ifindex_free_find() ends here. */

