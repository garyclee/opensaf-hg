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
FILE NAME: VIP_MIB.C
******************************************************************************/
/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*                             ~*  DESCRIPTION  *~                          *~*
*~*          This module contains functions used by the VIP Subsystem        *~*
*~*               for VIP operations on the VIP table.                       *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*                                                                          *~*
*~* ncsvipentry_get ....... Get the object from the table                    *~*
*~* ncsvipentry_next ...... Get the next object from table                   *~*
*~* ncsvipentry_extract ... Extract the data for that                        *~*
*~*                          particular Object ID                            *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

#if(NCS_VIP == 1) 

#include "ncsdlib.h"
#include "vip_mib.h"
#include "vip_mapi.h"

/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*                          *~ MACRO'S Declaration ~*                       *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

#define m_VIP_STR_TO_PARAM(param,str) \
{\
   param->i_fmat_id = NCSMIB_FMAT_OCT; \
   param->i_length = strlen(str); \
   param->info.i_oct = (uns8 *)str; \
}
#define m_VIP_INTEGER_TO_PARAM(param,val) \
{\
   param->i_fmat_id = NCSMIB_FMAT_INT; \
   param->i_length = sizeof(uns32); \
   param->info.i_int = m_NCS_OS_HTONL(val); \
}

#ifndef NCSMIB_TBL_VIP_TBLONE_TBL_INST_LEN
#define NCSMIB_TBL_VIP_TBLONE_MIN_INST_LEN 0
/* Since the Index Contains Variable Length Strings Both MIN and Max are given*/
#define NCSMIB_TBL_VIP_TBLONE_MAX_INST_LEN 128
#endif

/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*            *~ Forward Declaration of Static Functions ~*                 *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

static uns32 get_tbl_entry(IFSV_CB *vip_ifsv_cb,
                              NCSMIB_ARG *arg, 
                              IFSV_IFD_VIPD_RECORD **p_vip_rec);
static uns32 get_next_tbl_entry(IFSV_CB   *vip_ifsv_cb,
                                   NCSMIB_ARG  *arg,
                                   uns32       *io_next_inst_id,
                                   uns32       *io_next_inst_len,
                                   IFSV_IFD_VIPD_RECORD **p_vip_rec);   
static void make_key_from_instance_id(NCS_IFSV_VIP_INT_HDL *p_vip_handle, 
                                      VIP_IP_INTF_DATA *p_i_intf_data, 
                                      const uns32 *inst_id);
static void make_instance_id_from_key(NCS_IFSV_VIP_INT_HDL vip_handle, 
                                      VIP_IP_INTF_DATA ip_intf_data,
                                      uns32* inst_id,
                                      uns32* inst_len);


/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* Name          :  make_key_from_instance_id                               *~*
*~* Description   :  From the instance ids provided, prepares the key for    *~*
*~*                  searching the TBLONE Patricia Tree                      *~*
*~* Arguments     :  p_vip_handle - pointer to the VIPDB handle            *~*
*~*               :  p_ip_intf_data - pointer to the ip and Interface data   *~*
*~*                  inst_id - pointer to the instance_id structur           *~*
*~* Return Values :  Nothing                                                 *~*
*~* Notes         :  FOR NCS-VIP-MIB ServiceName, handle, IPAddress, Masklen,*~*
*~*                  and Interface Name are part of the Index(Key)           *~*
*~*                  where as for VIPDB only ServiceName handle and          *~*
*~*                  InterfaceType form a key to search in VIPDatabase.      *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
void make_key_from_instance_id(NCS_IFSV_VIP_INT_HDL *p_vip_handle, 
                               VIP_IP_INTF_DATA *p_ip_intf_data, 
                               const uns32 *inst_id)
{
   uns32 slen_ctr;
   uns32 ilen_ctr;
   uns32 service_len;
   uns32 interface_len;

   service_len = inst_id[0];
   /* INDEX:: 1*/
   for(slen_ctr=0; slen_ctr < service_len; slen_ctr++)
   {
     p_vip_handle->vipApplName[slen_ctr] = (uns8)inst_id[slen_ctr+1];
   }
   p_vip_handle->vipApplName[slen_ctr] = '\0';/*String Delimiter */

   /* INDEX:: 2*/
   p_vip_handle->poolHdl = inst_id[++slen_ctr];

   /* INDEX:: 3*/
   ((uns8*)&p_ip_intf_data->ipaddress)[0] = (uns8)inst_id[++slen_ctr];
   ((uns8*)&p_ip_intf_data->ipaddress)[1] = (uns8)inst_id[++slen_ctr];
   ((uns8*)&p_ip_intf_data->ipaddress)[2] = (uns8)inst_id[++slen_ctr];
   ((uns8*)&p_ip_intf_data->ipaddress)[3] = (uns8)inst_id[++slen_ctr];

   /* INDEX:: 4*/
   p_ip_intf_data->mask_len = (uns8 )inst_id[++slen_ctr];/*For Mask Len*/

   /* INDEX:: 5*/
   interface_len = inst_id[++slen_ctr];/*For Length of Interface Name*/
   for(ilen_ctr=0; ilen_ctr < interface_len; ilen_ctr++)
   {
      p_ip_intf_data->interface_name[ilen_ctr] = (uns8)inst_id[++slen_ctr];
   }
   p_ip_intf_data->interface_name[ilen_ctr] = '\0';

   /*IPType INDEX :: 6*/
   p_vip_handle->ipPoolType = inst_id[++slen_ctr];

   return;
}

/*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* Name          :  make_instance_id_from_key                               *~*
*~* Description   :  From the Instance Id's form's the Key                   *~*
*~*                  (To Search the VIP DataBase)                            *~*
*~* Arguments     :  NCS_IFSV_VIP_INT_HDL vip_handle                       *~*
*~*               :  VIP_IP_INTF_DATA ip_intf_data                         *~*
*~*                  uns32          *inst_id                                 *~*
*~*                  uns32          *inst_len                                *~*
*~* Return Values :  Nothing                                                 *~*
*~* Notes         :                                                          *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
void make_instance_id_from_key(NCS_IFSV_VIP_INT_HDL vip_handle, 
                               VIP_IP_INTF_DATA ip_intf_data,
                               uns32* inst_id,
                               uns32* inst_len)
{
    uns32 service_length;
    uns32 interface_length;
    uns32 horder_dest;
    uns32 idx_len;
    uns32 intf_len;
    uns32 index;
    uns32 slen;
    service_length = strlen(vip_handle.vipApplName);
    inst_id[0] = service_length;/* INDEX::  1 */
    for(index = 1, slen = 0; index <= service_length; index++,slen++)
    {
       inst_id[index] = vip_handle.vipApplName[slen];
    }

    inst_id[index++] = vip_handle.poolHdl;/*INDEX::   2 */

    /*IPAddress Fixed length */
    horder_dest = ip_intf_data.ipaddress;/*INDEX::  3*/
    inst_id[index++] = (horder_dest >> 24) & 0x000000ff;
    inst_id[index++] = (horder_dest >> 16) & 0x000000ff;
    inst_id[index++] = (horder_dest >>  8) & 0x000000ff;
    inst_id[index++] = (horder_dest      ) & 0x000000ff;

    inst_id[index++] = ip_intf_data.mask_len;/*INDEX::  4*/

    interface_length = strlen(ip_intf_data.interface_name);
    inst_id[index++] = interface_length;
    for(intf_len = 0, idx_len = index; intf_len <= interface_length; intf_len++, idx_len++)/*INDEX::  5 */
    {
        inst_id[idx_len] = ip_intf_data.interface_name[intf_len];
    }

    inst_id[idx_len] = vip_handle.ipPoolType;/*INDEX::   6 */


    *inst_len = idx_len;

    return;
}


/*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* Name          :  get_tbl_entry                                           *~* 
*~* Description   :  Get the Instance of tbl requested                       *~* 
*~* Arguments     :  IFSV_CB     *vip_ifsv_cb,                               *~* 
*~*                  NCSMIB_ARG    *arg,                                     *~* 
*~*                  IFSV_IFD_VIPD_RECORD **p_p_vip_rec                      *~* 
*~* Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE /                  *~* 
*~*                  NCSCC_RC_NO_INSTANCE                                    *~* 
*~* Notes         :                                                          *~* 
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

uns32 
get_tbl_entry(IFSV_CB  *vip_ifsv_cb, NCSMIB_ARG  *arg,
                 IFSV_IFD_VIPD_RECORD  **p_p_vip_rec)
{
   /*
    *The below 2 constitute the Key for mib
   */
    NCS_IFSV_VIP_INT_HDL vip_handle;
    VIP_IP_INTF_DATA ip_intf_data;

    memset(&vip_handle, 0, sizeof(NCS_IFSV_VIP_INT_HDL));
    memset(&ip_intf_data, 0, sizeof(VIP_IP_INTF_DATA));

    if(arg->i_idx.i_inst_ids == NULL)
    {
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_GET_NULL_INDEX_RECEIVED);
        return NCSCC_RC_NO_INSTANCE;
    }

    if((arg->i_idx.i_inst_len <= NCSMIB_TBL_VIP_TBLONE_MIN_INST_LEN) || 
            (arg->i_idx.i_inst_len > NCSMIB_TBL_VIP_TBLONE_MAX_INST_LEN))
    {
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_GET_INVALID_INDEX_RECEIVED);
        return NCSCC_RC_NO_INSTANCE;
    }

    /* Prepare the key from the instant ID */
    make_key_from_instance_id(&vip_handle, 
                              &ip_intf_data, 
                              arg->i_idx.i_inst_ids);

    /* The Key Can't be NULL Ignoring the Iptype*/
    if((strlen(vip_handle.vipApplName) == 0) && (vip_handle.poolHdl) == 0)
    {
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_GET_INVALID_INDEX_RECEIVED);
        return NCSCC_RC_NO_INSTANCE;
    }
    /*
     Check with Kishore whether AppName and poolHdl are enough to do a search 
     or even IpType is Required
    */
    if(((*p_p_vip_rec =
         (IFSV_IFD_VIPD_RECORD*) 
          ncs_patricia_tree_get(&vip_ifsv_cb->vipDBase,
                                (uns8*) &vip_handle)) == NULL))
         return NCSCC_RC_NO_INSTANCE;

    printf("NCSVIP-TBLONE_ENTRY key is: \n");

    return NCSCC_RC_SUCCESS;
}

/*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* Name          :  get_next_tbl_entry                                      *~*
*~* Description   :  Find the Instance of tbl requested                      *~*
*~* Arguments     :  IFSV_CB     *vip_ifsv_cb,                               *~*
*~*                  NCSMIB_ARG  *arg,                                       *~*
*~*                  uns32       *io_next_inst_id,                           *~*
*~*                  uns32       *io_next_inst_len,                          *~*
*~*                  IFSV_IFD_VIPD_RECORD **p_p_vip_rec                      *~*
*~* Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE /                  *~*
*~*                  NCSCC_RC_NO_INSTANCE                                    *~*
*~* Notes         :                                                          *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

uns32 get_next_tbl_entry(IFSV_CB *cb,
                         NCSMIB_ARG  *arg, 
                         uns32  *io_next_inst_id,/* The Index for next Row*/
                         uns32  *io_next_inst_len,/* The Index for next Row*/
                         IFSV_IFD_VIPD_RECORD **p_vip_rec) 
              /*This specifies the exact VIP entry in the whole VIPDatabase */
{
   
   NCS_IFSV_VIP_IP_LIST *ip_list;
   NCS_IFSV_VIP_INTF_LIST *intf_list;
   IFSV_IFD_VIPD_RECORD *pVipdb;
   NCS_IPPFX ipAddr;
   uns32 horder_dest;
   pVipdb = *p_vip_rec;
  
   NCS_DB_LINK_LIST_NODE *ip_node;
   NCS_DB_LINK_LIST_NODE *intf_node;
   NCS_IFSV_VIP_INT_HDL vip_handle;
   VIP_IP_INTF_DATA ip_intf_data;
   /*
   uns32 *inst_ids;
   inst_ids = arg->i_idx.i_inst_ids;
   */
    memset(&vip_handle,0, sizeof(NCS_IFSV_VIP_INT_HDL));
    memset(&ip_intf_data,0, sizeof(VIP_IP_INTF_DATA));
   
   if ((arg->i_idx.i_inst_len == 0) &&
       (arg->i_idx.i_inst_ids == NULL))
   {
      /* 
       *~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
       *~*                   This is the case of vip dump for CLI.           *~*
       *~*                   First tbl entry is requested.                   *~*
       *~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
      */
      if ((*p_vip_rec =
           (IFSV_IFD_VIPD_RECORD *)ncs_patricia_tree_getnext(&cb->vipDBase, 
                                                      (uns8*)0)) != NULL) 
      {
          if( (ip_node = m_NCS_DBLIST_FIND_FIRST(&(*p_vip_rec)->ip_list)) != NULL )
          {
              ip_list = (NCS_IFSV_VIP_IP_LIST *)ip_node;
          }
          else
              return NCSCC_RC_FAILURE;
          
          if( (intf_node = m_NCS_DBLIST_FIND_FIRST(&(*p_vip_rec)->intf_list)) != NULL)
          {
              intf_list = (NCS_IFSV_VIP_INTF_LIST *)intf_node;
          }
          else
              return NCSCC_RC_FAILURE;

         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_GET_NEXT_INITIALLY_RECORD_FOUND);
      }
      else
      {
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_GET_NEXT_INITIALLY_NO_RECORD_FOUND);
         /*
          *~*ERROR::
             If there are no entries in VIP DataBase
         */
         return IFSV_VIP_NO_RECORDS_IN_VIP_DATABASE;
      }
   }
   else
   { 
      if((arg->i_idx.i_inst_len <= NCSMIB_TBL_VIP_TBLONE_MIN_INST_LEN) ||
         (arg->i_idx.i_inst_len > NCSMIB_TBL_VIP_TBLONE_MAX_INST_LEN))
      {
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_GET_NEXT_INVALID_INDEX_RECEIVED);
         /*
          *~*ERROR::
             Invalid Index
         */
         return NCSCC_RC_NO_INSTANCE;
      }

       /* Prepare the key(HANDLE and IP&intf data) from the instant ID */
       make_key_from_instance_id(&vip_handle, 
                                 &ip_intf_data, 
                                 arg->i_idx.i_inst_ids);

       /* Can't be NULL */
       if((strlen(vip_handle.vipApplName) == 0) || (vip_handle.poolHdl) == 0)
       {
          m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_GET_NEXT_INVALID_INDEX_RECEIVED);
          return NCSCC_RC_NO_INSTANCE;
       }
   
       if( (ip_intf_data.ipaddress == 0) && (strlen(ip_intf_data.interface_name) == 0) )
       {
               /*Case of Single Entry Show in CLI */
          if((*p_vip_rec = 
                    (IFSV_IFD_VIPD_RECORD *)ncs_patricia_tree_get(&cb->vipDBase,
                      (uns8 *)&vip_handle))!= NULL) 
          {
               /* 
                From VIP Record Extract the Key Info and then Prepare 
                Instance ID from it
               */
               if(ip_intf_data.ipaddress == 0)
               {
                  if( (ip_node = m_NCS_DBLIST_FIND_FIRST(&(*p_vip_rec)->ip_list)) != NULL)
                  {
                      ip_list = (NCS_IFSV_VIP_IP_LIST *)ip_node;
                  }
                  else
                      return NCSCC_RC_FAILURE;
               }
               else
               {
                   horder_dest = ip_intf_data.ipaddress;
                   ipAddr.ipaddr.info.v4 = horder_dest;
                   ipAddr.ipaddr.type = NCS_IP_ADDR_TYPE_IPV4;
                   ipAddr.mask_len = ip_intf_data.mask_len;
      
                   if( (ip_node = ncs_db_link_list_find(&(*p_vip_rec)->ip_list,
                                                     (uns8 *)(&ipAddr))) != NULL)
                   {
                      ip_list = (NCS_IFSV_VIP_IP_LIST *)ip_node;
                   }
                   else
                      return NCSCC_RC_FAILURE;
               }
      
               if(strlen(ip_intf_data.interface_name) == 0)
               {
                  if( (intf_node = m_NCS_DBLIST_FIND_FIRST(&(*p_vip_rec)->intf_list)) != NULL)
                  {
                      intf_list = (NCS_IFSV_VIP_INTF_LIST *)intf_node;
                  }
                  else
                      return NCSCC_RC_FAILURE;
               }
               else
               {
                  if( (intf_node = ncs_db_link_list_find(&(*p_vip_rec)->intf_list,
                                            (uns8 *)(&ip_intf_data.interface_name)))!= NULL)
                  {
                      intf_list = (NCS_IFSV_VIP_INTF_LIST *)intf_node;
                  }
                  else
                      return NCSCC_RC_FAILURE;
               }
          }
          else
              return NCSCC_RC_FAILURE;
       }
       else if((*p_vip_rec = 
                  (IFSV_IFD_VIPD_RECORD *)ncs_patricia_tree_getnext(&cb->vipDBase,
                   (uns8 *)&vip_handle))!= NULL) 
        {
              /*Case of Show ALL  VIP Entries in CLI */
         /* 
          From VIP Record Extract the Key Info and then Prepare 
          Instance ID from it
         */
                if( (ip_node = m_NCS_DBLIST_FIND_FIRST(&(*p_vip_rec)->ip_list)) != NULL)
                {
                    ip_list = (NCS_IFSV_VIP_IP_LIST *)ip_node;
                }
                else
                    return NCSCC_RC_FAILURE;
   
                 if( (intf_node = m_NCS_DBLIST_FIND_FIRST(&(*p_vip_rec)->intf_list)) != NULL)
                 {
                     intf_list = (NCS_IFSV_VIP_INTF_LIST *)intf_node;
                 }
                 else
                     return NCSCC_RC_FAILURE;
        }
        /* Do a getnext based on KEY or Handle */
        else
        {
           /* Error:: NO RECORD FOUND*/
           return NCSCC_RC_NO_INSTANCE;
        }
   }

    /* The Next Entry's IPAddress && InterfaceName */
    strcpy(vip_handle.vipApplName,&(*p_vip_rec)->handle.vipApplName); 
    vip_handle.poolHdl = (*p_vip_rec)->handle.poolHdl;
    vip_handle.ipPoolType = (*p_vip_rec)->handle.ipPoolType;
    ip_intf_data.mask_len = ip_list->ip_addr.mask_len;
    ip_intf_data.ipaddress = ip_list->ip_addr.ipaddr.info.v4;
    strcpy(ip_intf_data.interface_name, ip_list->intfName);
        
  
    printf("The Handle Elements are ::: :-)");
    make_instance_id_from_key(vip_handle,
                              ip_intf_data,
                              io_next_inst_id,
                              io_next_inst_len); 

    return NCSCC_RC_SUCCESS;
}


/*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* Name          :  ncsvipentry_extract                                     *~*
*~* Description   :  Get function for VIP tbl table objects                  *~*
*~* Arguments     :  NCSMIB_PARAM_VAL *param,                                *~*
*~*                  NCSMIB_VAR_INFO  *var_info,                             *~*
*~*                  NCSCONTEXT data,                                        *~*
*~*                  NCSCONTEXT buffer                                       *~*
*~* Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE                    *~*
*~* Notes         :                                                          *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

uns32 ncsvipentry_extract(NCSMIB_PARAM_VAL *param, 
                                   NCSMIB_VAR_INFO  *var_info,
                                   NCSCONTEXT data,
                                   NCSCONTEXT buffer)
{
    uns32 range=0;
    uns32 row_status=1;
    switch(param->i_param_id)
    {
        case ncsVIPEntryType_ID:
        {
            IFSV_IFD_VIPD_RECORD *p_vip_rec = (IFSV_IFD_VIPD_RECORD *)data;
            m_VIP_INTEGER_TO_PARAM(param, 
                   p_vip_rec->vip_entry_attr);
        }
        break;

        case ncsVIPInstalledInterfaceName_ID:
        {
           NCS_IFSV_VIP_IP_LIST *p_ip_node = (NCS_IFSV_VIP_IP_LIST *)data;
           m_VIP_STR_TO_PARAM(param,p_ip_node->intfName);
        }
        break;

        case ncsVIPIPRange_ID:
        {
            IFSV_IFD_VIPD_RECORD *p_vip_rec = (IFSV_IFD_VIPD_RECORD *)data;
            if(m_IFSV_IS_VIP_CONTINUOUS(p_vip_rec->vip_entry_attr))
            {
                m_VIP_INTEGER_TO_PARAM(param,1);
            }
            else
                m_VIP_INTEGER_TO_PARAM(param,range);
        }
        break;

        case ncsVIPOperStatus_ID:
        {
           IFSV_IFD_VIPD_RECORD *p_vip_rec = (IFSV_IFD_VIPD_RECORD *)data;
           m_VIP_INTEGER_TO_PARAM(param, 
                   m_IFSV_IS_VIP_ALLOCATED(p_vip_rec->vip_entry_attr));
        }
        break;

        case ncsVIPRowStatus_ID:
        {
           m_VIP_INTEGER_TO_PARAM(param, row_status);
        }
        break;

       default:
          return ncsmiblib_get_obj_val(param, var_info, data, buffer);
    }

   return NCSCC_RC_SUCCESS;
}

/*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* Name          :  ncsvipentry_get                                         *~* 
*~* Description   :  Get function for VIP tbl table objects                  *~*
*~* Arguments     :  NCSCONTEXT cb,                                          *~*
*~*                  NCSMIB_ARG *p_get_req,                                  *~*
*~*                  NCSCONTEXT* data,                                       *~*
*~* Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE                    *~*
*~* Notes         :  Get Call is being used for SNMP GET, so full index is   *~*
                     given by user, need not fetch the way we r doing in     *~*
                     get_next implementation.                                *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

uns32  ncsvipentry_get(NCSCONTEXT cb, 
                       NCSMIB_ARG *p_get_req, 
                       NCSCONTEXT* data)
{
    IFSV_CB     *vip_ifsv_cb = (IFSV_CB *)cb;
    IFSV_IFD_VIPD_RECORD *p_vip_rec = IFSV_IFD_VIPD_RECORD_NULL;
    uns32         ret_code = NCSCC_RC_SUCCESS;
    NCS_IFSV_VIP_INT_HDL vip_handle;
    VIP_IP_INTF_DATA ip_intf_data;
    NCS_IPPFX ipAddr;

    if ((p_get_req == NULL) || (data == NULL))
        return NCSCC_RC_FAILURE;

    memset(&vip_handle,0, sizeof(NCS_IFSV_VIP_INT_HDL));
    memset(&ip_intf_data,0, sizeof(VIP_IP_INTF_DATA));
    printf("\nncsTestTableOneEntry:  Received SNMP GET request\n");
    /* Pretty print the contents of NCSMIB_ARG */
    ncsmib_pp(p_get_req); 
   
    if((ret_code = get_tbl_entry(vip_ifsv_cb, p_get_req, &p_vip_rec)) != 
            NCSCC_RC_SUCCESS)
    {
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_GET_NO_RECORD_FOUND);
        return ret_code;
    }
    else
    {
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_GET_RECORD_FOUND);
       make_key_from_instance_id(&vip_handle, &ip_intf_data,
                                 p_get_req->i_idx.i_inst_ids);
       ipAddr.ipaddr.info.v4 = ip_intf_data.ipaddress;
       ipAddr.ipaddr.type = NCS_IP_ADDR_TYPE_IPV4;
       ipAddr.mask_len = ip_intf_data.mask_len;

       switch(p_get_req->req.info.get_req.i_param_id)
       {
           case ncsVIPEntryType_ID:
           {
              *data = p_vip_rec; 
              /*
               * This value is equivalent to vip_entry_attr "CONFIGURED bit" 
               * in the VIPD Database
              */
           }
           break;

           case ncsVIPInstalledInterfaceName_ID:
           {
               *data = ncs_db_link_list_find(&p_vip_rec->ip_list,
                                                (uns8 *)(&ipAddr));
               /*
                * This is Present along with the IpList so search for the 
                * IPAddress Node and find this in the IP Node
               */
           }
           break;

           case ncsVIPIPRange_ID:
           {
              *data = p_vip_rec; 
              /*
               * This value is equivalent to vip_entry_attr "CONTINUOUS bit"
               * in the VIPD Database
              */
           }
           break;

           case ncsVIPOperStatus_ID:
              *data = p_vip_rec; 
              /*
               * This value is equivalent to vip_entry_attr "ALLOCATED bit" 
               * in the VIPD Database
              */
           break;

           case ncsVIPRowStatus_ID:
           {
              *data = p_vip_rec;
           }
           break;
   
           default:
              return NCSCC_RC_INV_VAL;
        }
    }
    return ret_code;
}

/*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* Name          :  ncsvipentry_next                                        *~* 
*~* Description   :  Next function for VIP tbl table                         *~*
*~* Arguments     :  NCSCONTEXT ifsv_vip_cb,                                 *~*
*~*                  NCSMIB_ARG *p_next_req,                                 *~*
*~*                  uns32* next_inst_id,                                    *~*
*~*                  uns32* next_inst_id_len                                 *~*
*~* Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE                    *~*
*~* Notes         :                                                          *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

uns32 ncsvipentry_next(NCSCONTEXT ifsv_vip_cb,
                       NCSMIB_ARG *p_next_req,
                       NCSCONTEXT* data,
                       uns32* o_next_inst_id,
                       uns32* o_next_inst_id_len) 
{
    IFSV_IFD_VIPD_RECORD *p_vip_rec = IFSV_IFD_VIPD_RECORD_NULL;
    uns32 ret_code = NCSCC_RC_SUCCESS;
    NCS_IFSV_VIP_INT_HDL vip_handle;
    VIP_IP_INTF_DATA ip_intf_data;
    NCS_IPPFX ipAddr;
    uns32 horder_dest;
  
    memset(&vip_handle, 0, sizeof(NCS_IFSV_VIP_INT_HDL));
    memset(&ip_intf_data, 0, sizeof(VIP_IP_INTF_DATA));

    printf("\n ncsvipentry:  Received SNMP NEXT request\n");
    /* Pretty print the contents of NCSMIB_ARG */
    ncsmib_pp(p_next_req); 
                                                                                                                             
    if ((ret_code = get_next_tbl_entry(ifsv_vip_cb, 
                                       p_next_req,
                                       o_next_inst_id,
                                       o_next_inst_id_len,
                                       &p_vip_rec))
                   != NCSCC_RC_SUCCESS)
    {
       m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"get_next_tbl_entry():ncsvipentry_next returned failure"," ");
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_GET_NEXT_NO_RECORD_FOUND);
       return ret_code;
    }
    /*
     *Need to CHeck the error Cases returned by get_next_tbl_entry
    */

    make_key_from_instance_id(&vip_handle, &ip_intf_data, o_next_inst_id);
    horder_dest = ip_intf_data.ipaddress;
    ipAddr.ipaddr.info.v4 = horder_dest;
    ipAddr.ipaddr.type = NCS_IP_ADDR_TYPE_IPV4;
    ipAddr.mask_len = ip_intf_data.mask_len;

    switch(p_next_req->req.info.next_req.i_param_id)
    {
        case ncsVIPEntryType_ID:
        {
           *data = p_vip_rec; 
           /*
            * This value is equivqlent to vip_entry_attr "CONFIGURED bit"
            * in the VIPD Database
           */
        }
        break;

        case ncsVIPInstalledInterfaceName_ID:
        {
            if(ipAddr.ipaddr.info.v4 == 0)
            {
               *data = m_NCS_DBLIST_FIND_FIRST(&p_vip_rec->ip_list);
            }
            else
               *data = ncs_db_link_list_find(&p_vip_rec->ip_list,
                                             (uns8 *)(&ipAddr));
            /*
             * This is Present along with the IpList so search for the
             * IPAddress Node and find this in the IP Node
            */
        }
        break;

        case ncsVIPIPRange_ID:
        {
           *data = p_vip_rec;
           /*
            * This value is equivqlent to vip_entry_attr "CONTINUOUS bit"
            * in the VIPD Database
           */
        }
        break;

        case ncsVIPOperStatus_ID:
        {
           *data = p_vip_rec;
           /*
            * This value is equivqlent to vip_entry_attr "ALLOCATED bit"
            * in the VIPD Database
           */

        }
        break;

        case ncsVIPRowStatus_ID:
        {
           *data = p_vip_rec;
        }
        break;

        default:
        return NCSCC_RC_INV_VAL;
     }

     if( *data == NULL )
        return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}

uns32 ncsvipentry_setrow(NCSCONTEXT cb,
                                  NCSMIB_ARG *args,
                                  NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                                  struct ncsmib_obj_info *obj_info,
                                  NCS_BOOL testrow_flag)
{
   return NCSCC_RC_SUCCESS;
}
uns32 ncsvipentry_set(NCSCONTEXT cb,
                               NCSMIB_ARG *arg,
                               NCSMIB_VAR_INFO *var_info,
                               NCS_BOOL test_flag)
{
   return NCSCC_RC_SUCCESS;
}

#endif

