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
FILE NAME: ifd_mbcsv.C
DESCRIPTION: Ifd redundancy routines related to MBCSV.
******************************************************************************/
#include "ifd.h"

/*****************************************************************************
 *                  Functions prototypes 
 *****************************************************************************/
extern uns32  ifd_mbcsv_callback(NCS_MBCSV_CB_ARG *arg);
extern uns32 ifd_a2s_async_update (IFSV_CB *cb, IFD_A2S_MSG_TYPE type, void *msg);
extern uns32 ifd_mbcsv_register(IFSV_CB *cb); 
extern uns32  ifd_mbcsv_selobj_get(IFSV_CB *cb);  
extern uns32  ifd_mbcsv_finalize(IFSV_CB *cb);  
extern uns32  ifd_mbcsv_chgrole(IFSV_CB *cb);  
extern uns32  ifd_mbcsv_dispatch(IFSV_CB *cb);  
extern uns32 ifd_a2s_iaps_sync_resp (IFSV_CB *cb, IFAP_INFO_LIST_A2S *msg);

static uns32  ifd_mbcsv_db_ifrec_sptmap_mismatch_correction(IFSV_CB *ifsv_cb);
static uns32  ifd_mbcsv_db_mismatch_correction(IFSV_CB *ifsv_cb, uns32 num_max_ifindex);
static uns32  ifd_find_index_from_sptmap(uns32 ifindex,IFSV_CB *cb);
static uns32  ifd_mbcsv_init(IFSV_CB *cb); 
static uns32  ifd_mbcsv_open(IFSV_CB *cb); 
static uns32  ifd_mbcsv_warm_sync_timer_set(IFSV_CB *cb); 
static uns32  ifd_mbcsv_close(IFSV_CB *cb); 
static uns32 ifd_a2s_async_spt_map_create_info_send(IFD_A2S_MSG *a2s_msg, 
                                                    char *msg);
static uns32  ifd_mbcsv_encode_proc(NCS_MBCSV_CB_ARG *arg);  
static uns32  ifd_mbcsv_decode_proc(NCS_MBCSV_CB_ARG *arg);
static uns32  ifd_mbcsv_peer_info_cbk_handler(NCS_MBCSV_CB_ARG *arg);
static uns32  ifd_mbcsv_enc_async_update(IFSV_CB *cb, NCS_MBCSV_CB_ARG *arg);
static uns32  ifd_mbcsv_enc_warm_sync_resp(IFSV_CB *cb, NCS_MBCSV_CB_ARG *arg);
static uns32  ifd_mbcsv_dec_async_update(IFSV_CB *cb, NCS_MBCSV_CB_ARG *arg);
static uns32  ifd_mbcsv_dec_warm_sync_resp(IFSV_CB *cb, NCS_MBCSV_CB_ARG *arg);
static uns32 ifd_a2s_async_spt_map_delete_info_send(IFD_A2S_MSG *a2s_msg,
                                                    char *msg);
static uns32 ifd_a2s_async_data_create_info_send(IFD_A2S_MSG *a2s_msg,
                                                 char *msg);
static uns32 ifd_a2s_async_data_modify_info_send(IFD_A2S_MSG *a2s_msg,
                                                 char *msg);
static uns32 ifd_a2s_async_data_marked_delete_info_send(IFD_A2S_MSG *a2s_msg,
                                                        char *msg);
static uns32 ifd_a2s_async_data_delete_info_send(IFD_A2S_MSG *a2s_msg, 
                                                 char *msg);
static uns32 ifd_a2s_async_svc_upd_info_send(IFD_A2S_MSG *a2s_msg, char *msg);
static uns32 ifd_a2s_ifnd_down_send(IFD_A2S_MSG *a2s_msg, char *msg);
static uns32 ifd_a2s_ifnd_up_send(IFD_A2S_MSG *a2s_msg, char *msg);
static uns32 ifd_a2s_ifindex_alloc_send(IFD_A2S_MSG *a2s_msg, char *msg);
static uns32 ifd_a2s_ifindex_dealloc_send(IFD_A2S_MSG *a2s_msg, char *msg);
#if(NCS_IFSV_IPXS == 1)    
static uns32 ifd_a2s_ipxs_info_send(IFD_A2S_MSG *a2s_msg, char *msg);
#endif
static uns32 ifd_mbcsv_async_update(IFSV_CB *cb, IFD_A2S_MSG *msg);

#if(NCS_VIP == 1)    
static uns32  ifd_mbcsv_enc_vip_data_resp(void *cb, NCS_MBCSV_CB_ARG *arg);
static uns32  ifd_mbcsv_dec_vip_data_resp(void *ifsv_cb, NCS_MBCSV_CB_ARG *arg);
static uns32
ifd_a2s_vip_rec_create_send(IFD_A2S_MSG *a2s_msg, char *msg);
static uns32
ifd_a2s_vip_rec_modify_send(IFD_A2S_MSG *a2s_msg, char *msg);
static uns32
ifd_a2s_vip_rec_del_send(IFD_A2S_MSG *a2s_msg, char *msg);

#endif


/****************************************************************************
 * Name          : ifd_a2s_async_update 
 *
 * Description   : Function to send async message from active to standby IfD.
 *
 * Arguments     : cb   - This is the interace control block.
 *                 type - message type to be sent to Stdby IfD.
 *                 msg - pointer to the message.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifd_a2s_async_update (IFSV_CB *cb, IFD_A2S_MSG_TYPE type, void *msg)
{
  uns32 res = NCSCC_RC_FAILURE;
  IFD_A2S_MSG a2s_msg;
  
  m_NCS_OS_MEMSET(&a2s_msg , '\0' , sizeof(IFD_A2S_MSG));

  switch (type)
  {
    case IFD_A2S_SPT_MAP_MAP_CREATE_MSG:
    {
      res = ifd_a2s_async_spt_map_create_info_send(&a2s_msg, msg);
      break;
    }
    
    case IFD_A2S_SPT_MAP_DELETE_MSG:
    {
      res = ifd_a2s_async_spt_map_delete_info_send(&a2s_msg, msg);
      break;
    }
    
    case IFD_A2S_DATA_CREATE_MSG:
    {
      res = ifd_a2s_async_data_create_info_send(&a2s_msg, msg);
      break;
    }
    
    case IFD_A2S_DATA_MODIFY_MSG:
    {
      res = ifd_a2s_async_data_modify_info_send(&a2s_msg, msg);
      break;
    }
    
    case IFD_A2S_DATA_MARKED_DELETE_MSG:
    {
      res = ifd_a2s_async_data_marked_delete_info_send(&a2s_msg, msg);
      break;
    }
    
    case IFD_A2S_DATA_DELETE_MSG:
    {
      res = ifd_a2s_async_data_delete_info_send(&a2s_msg, msg);
      break;
    }
    
    case IFD_A2S_SVC_DEST_UPD_MSG:
    {
      res = ifd_a2s_async_svc_upd_info_send(&a2s_msg, msg);
      break;
    }

    case IFD_A2S_IFND_DOWN_MSG:
    {
      res = ifd_a2s_ifnd_down_send(&a2s_msg, msg);
      break;
    }

    case IFD_A2S_IFND_UP_MSG:
    {
      res = ifd_a2s_ifnd_up_send(&a2s_msg, msg);
      break;
    }

    case IFD_A2S_IFINDEX_ALLOC_MSG:
    {
      res = ifd_a2s_ifindex_alloc_send(&a2s_msg, msg);
      break;
    }

    case IFD_A2S_IFINDEX_DEALLOC_MSG:
    {
      res = ifd_a2s_ifindex_dealloc_send(&a2s_msg, msg);
      break;
    }

#if(NCS_IFSV_IPXS == 1)    
    case IFD_A2S_IPXS_MSG:
    {
      res = ifd_a2s_ipxs_info_send(&a2s_msg, msg);
      break;
    }
#endif

#if(NCS_VIP == 1)    
    case IFD_A2S_VIPD_REC_CREATE_MSG:
    {
      res = ifd_a2s_vip_rec_create_send(&a2s_msg, msg);
      break;
    }

    case IFD_A2S_VIPD_REC_MODIFY_MSG:
    {
      res = ifd_a2s_vip_rec_modify_send(&a2s_msg, msg);
      break;
    }

    case IFD_A2S_VIPD_REC_DEL_MSG:
    {
      res = ifd_a2s_vip_rec_del_send(&a2s_msg, msg);
      break;
    }
#endif

    default:
    break;
  } /* evt->type */

  /* Send the aync_msg to the standby IFD using the MBCSv_SEND function */
  res = ifd_mbcsv_async_update(cb, &a2s_msg);

  if(res == NCSCC_RC_SUCCESS)
     cb->ifd_async_cnt++;  

  return (res);
} /* function ifd_a2s_async_update() ends here */

/****************************************************************************
 * Name          : ifd_a2s_async_spt_map_create_info_send 
 *
 * Description   : Function to send spt map create info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to 
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_async_spt_map_create_info_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  m_IFD_LOG_EVT_LLL(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,\
             "IfIndex Spt Map Create. if_index, shelf and slot : ",\
             ((NCS_IFSV_SPT_MAP *)msg)->if_index,\
             ((NCS_IFSV_SPT_MAP *)msg)->spt.shelf,\
             ((NCS_IFSV_SPT_MAP *)msg)->spt.slot);

  a2s_msg->type = IFD_A2S_EVT_IFINDEX_SPT_MAP;
  a2s_msg->info.ifd_a2s_ifindex_spt_map_evt.type = 
                                   IFD_A2S_IFINDEX_SPT_MAP_EVT_CREATE;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_ifindex_spt_map_evt.info, msg, 
               sizeof(NCS_IFSV_SPT_MAP));

  return NCSCC_RC_SUCCESS;

} /* function ifd_a2s_async_spt_map_create_info_send() ends here. */

/****************************************************************************
 * Name          : ifd_a2s_async_spt_map_delete_info_send 
 *
 * Description   : Function to send spt map delete info.
 *                 a2s_msg -- IFD_A2S_MSG async message to 
 *                 standby IFD
 *
 * Arguments     : msg - pointer to the message.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_async_spt_map_delete_info_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  m_IFD_LOG_EVT_LLL(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,\
             "IfIndex Spt Map Delete. if_index, shelf and slot : ",\
             ((NCS_IFSV_SPT_MAP *)msg)->if_index,\
             ((NCS_IFSV_SPT_MAP *)msg)->spt.shelf,\
             ((NCS_IFSV_SPT_MAP *)msg)->spt.slot);

  a2s_msg->type = IFD_A2S_EVT_IFINDEX_SPT_MAP;
  a2s_msg->info.ifd_a2s_ifindex_spt_map_evt.type = 
                                   IFD_A2S_IFINDEX_SPT_MAP_EVT_DELETE;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_ifindex_spt_map_evt.info, msg,
               sizeof(NCS_IFSV_SPT_MAP));

  return NCSCC_RC_SUCCESS;

} /* function ifd_a2s_async_spt_map_delete_info_send() ends here. */

/****************************************************************************
 * Name          : ifd_a2s_async_data_create_info_send 
 *
 * Description   : Function to send intf data create info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to 
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_async_data_create_info_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,\
                  "Interface Create Info. Index : ",\
                  ((IFSV_INTF_DATA *)msg)->if_index);
  a2s_msg->type = IFD_A2S_EVT_INTF_DATA;
  a2s_msg->info.ifd_a2s_intf_data_evt.type = IFD_A2S_INTF_DATA_EVT_CREATE;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_intf_data_evt.info, msg, 
               sizeof(IFSV_INTF_DATA));

  return NCSCC_RC_SUCCESS;

} /* function ifd_a2s_async_data_create_info_send() ends here. */

/****************************************************************************
 * Name          : ifd_a2s_async_data_modify_info_send 
 *
 * Description   : Function to send intf data modify info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to 
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_async_data_modify_info_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,\
                  "Interface Modify Info. Index : ",\
                  ((IFSV_INTF_DATA *)msg)->if_index);
  a2s_msg->type = IFD_A2S_EVT_INTF_DATA;
  a2s_msg->info.ifd_a2s_intf_data_evt.type = IFD_A2S_INTF_DATA_EVT_MODIFY;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_intf_data_evt.info, msg, 
               sizeof(IFSV_INTF_DATA));

  return NCSCC_RC_SUCCESS;

} /* function ifd_a2s_async_data_modify_info_send() ends here. */

/****************************************************************************
 * Name          : ifd_a2s_async_data_marked_delete_info_send 
 *
 * Description   : Function to send intf data marked delete info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to 
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_async_data_marked_delete_info_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,\
                  "Interface Data Marked Delete Info. Index : ",\
                  ((IFSV_INTF_DATA *)msg)->if_index);
  a2s_msg->type = IFD_A2S_EVT_INTF_DATA;
  a2s_msg->info.ifd_a2s_intf_data_evt.type = 
                                        IFD_A2S_INTF_DATA_EVT_MARKED_DELETE;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_intf_data_evt.info, msg, 
               sizeof(IFSV_INTF_DATA));

  return NCSCC_RC_SUCCESS;

} /* function ifd_a2s_async_data_marked_delete_info_send() ends here. */

/****************************************************************************
 * Name          : ifd_a2s_async_data_delete_info_send 
 *
 * Description   : Function to send intf data delete info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to 
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_async_data_delete_info_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,\
                  "Interface Data Delete Info. Index : ",\
                  ((IFSV_INTF_DATA *)msg)->if_index);
  a2s_msg->type = IFD_A2S_EVT_INTF_DATA;
  a2s_msg->info.ifd_a2s_intf_data_evt.type = IFD_A2S_INTF_DATA_EVT_DELETE;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_intf_data_evt.info, msg, 
               sizeof(IFSV_INTF_DATA));

  return NCSCC_RC_SUCCESS;

} /* function ifd_a2s_async_data_delete_info_send() ends here. */

/****************************************************************************
 * Name          : ifd_a2s_async_svc_upd_info_send 
 *
 * Description   : Function to send SVC update info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to 
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_async_svc_upd_info_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,\
                  "SVCD Update. Index : ",\
                  ((NCS_IFSV_SVC_DEST_UPD *)msg)->i_ifindex);

  NCS_IFSV_SVC_DEST_UPD *info;
  uns32 res = NCSCC_RC_SUCCESS;

  info = (NCS_IFSV_SVC_DEST_UPD *)msg;

  a2s_msg->type = IFD_A2S_EVT_SVC_DEST_UPD;
  a2s_msg->info.ifd_a2s_svc_dest_upd_evt.type = IFD_A2S_SVC_DEST_UPD_EVT_BASE;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_svc_dest_upd_evt.info, msg, 
               sizeof(NCS_IFSV_SVC_DEST_UPD));
  return res;

} /* function ifd_a2s_async_svc_upd_info_send() ends here. */

#if(NCS_IFSV_IPXS == 1)    
/****************************************************************************
 * Name          : ifd_a2s_ipxs_info_send
 *
 * Description   : Function to send IPXS info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_ipxs_info_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  IPXS_EVT_IF_INFO *info;
  uns32 res = NCSCC_RC_SUCCESS;
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,\
                  "IPXS Info update. Index : ",\
                  ((IPXS_EVT_IF_INFO *)msg)->if_index);

  info = (IPXS_EVT_IF_INFO *)msg;

  a2s_msg->type = IFD_A2S_EVT_IPXS_INTF_INFO;
  a2s_msg->info.ifd_a2s_ipxs_intf_info_evt.type = 
                                            IFD_A2S_IPXS_INTF_INFO_EVT_BASE;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_ipxs_intf_info_evt.info, msg, 
               sizeof(IPXS_EVT_IF_INFO));
  return res;

} /* function ifd_a2s_ipxs_info_send() ends here. */
#endif

#if(NCS_VIP == 1)

/****************************************************************************
 * Name          : ifd_populate_vip_redundancy_rec
 *
 * Description   : Function to send VIP info for checkpointing.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_populate_vip_redundancy_rec(IFSV_IFD_VIPD_RECORD  *p_vipd_rec_info, VIP_REDUNDANCY_RECORD *p_vip_redRec)
{
  uns32 res = NCSCC_RC_SUCCESS;
  NCS_DB_LINK_LIST_NODE *ip_node, *intf_node, *owner_node;
  NCS_IFSV_VIP_IP_LIST *ip_list;
  NCS_IFSV_VIP_INTF_LIST *intf_list;
  NCS_IFSV_VIP_OWNER_LIST *owner_list;
  VIP_RED_IP_NODE *p_temp_ip_rec;
  VIP_RED_INTF_NODE *p_temp_intf_rec;
  VIP_RED_OWNER_NODE *p_temp_owner_rec;
  uns32 ip_cnt, intf_cnt, owner_cnt;

  m_NCS_MEMSET(p_vip_redRec, 0, sizeof(VIP_REDUNDANCY_RECORD));


  m_NCS_STRCPY(p_vip_redRec->handle.vipApplName, p_vipd_rec_info->handle.vipApplName);
  p_vip_redRec->handle.poolHdl = p_vipd_rec_info->handle.poolHdl;
  p_vip_redRec->handle.ipPoolType = p_vipd_rec_info->handle.ipPoolType;
  p_vip_redRec->vip_entry_attr = p_vipd_rec_info->vip_entry_attr;
  p_vip_redRec->ref_cnt = p_vipd_rec_info->ref_cnt;

  /* Filling the IPAddress Related Info */
  p_vip_redRec->ip_list_cnt = ((NCS_DB_LINK_LIST *)&p_vipd_rec_info->ip_list)->n_nodes;
  if(p_vip_redRec->ip_list_cnt)
  {
     p_vip_redRec->ipInfo = m_MMGR_ALLOC_IFSV_VIP_RED_IP_REC(p_vip_redRec->ip_list_cnt);
     if( p_vip_redRec->ipInfo != IFSV_NULL )
     {
        m_NCS_MEMSET(p_vip_redRec->ipInfo, 0,(p_vip_redRec->ip_list_cnt)* sizeof(VIP_RED_IP_NODE));
        p_temp_ip_rec = p_vip_redRec->ipInfo;

        ip_node = m_NCS_DBLIST_FIND_FIRST(&(p_vipd_rec_info)->ip_list); 
        if( ip_node != IFSV_NULL )
        {
           for(ip_cnt = 0; ip_cnt < p_vip_redRec->ip_list_cnt; ip_cnt++)
           {
               ip_list = (NCS_IFSV_VIP_IP_LIST *)ip_node;
               p_temp_ip_rec ->ip_addr.ipaddr.info.v4 = ip_list->ip_addr.ipaddr.info.v4;
               p_temp_ip_rec->ip_addr.ipaddr.type = NCS_IP_ADDR_TYPE_IPV4;
               p_temp_ip_rec->ip_addr.mask_len = ip_list->ip_addr.mask_len;
               p_temp_ip_rec->ipAllocated =  m_IFSV_IS_VIP_ALLOCATED(p_vipd_rec_info->vip_entry_attr);
               m_NCS_STRCPY(p_temp_ip_rec->intfName, ip_list->intfName);
      
               ip_node = ncs_db_link_list_find_next(&(p_vipd_rec_info)->ip_list, (uns8 *)(&p_temp_ip_rec->ip_addr) );
               if(ip_node != IFSV_NULL)
                  p_temp_ip_rec++;
               else
                  break;
           }/* for */
       }/* if( ip_node != NULL ) */
       else
          return NCSCC_RC_FAILURE;
     }/*if( p_vip_redRec->ipInfo != NULL )*/
     else
       return NCSCC_RC_FAILURE;

  }/*if(p_vip_redRec->ip_list_cnt)*/
  else
    return NCSCC_RC_FAILURE;

  /* Filling the Interface Related Info */
  p_vip_redRec->intf_list_cnt = ((NCS_DB_LINK_LIST *)&p_vipd_rec_info->intf_list)->n_nodes;
  if(p_vip_redRec->intf_list_cnt)
  {
    p_vip_redRec->intfInfo = m_MMGR_ALLOC_IFSV_VIP_RED_INTF_REC(p_vip_redRec->intf_list_cnt);
    if( p_vip_redRec->intfInfo != IFSV_NULL)
    {
        m_NCS_MEMSET(p_vip_redRec->intfInfo, 0, (p_vip_redRec->intf_list_cnt)*sizeof(VIP_RED_INTF_NODE));
        p_temp_intf_rec = p_vip_redRec->intfInfo;

        intf_node = m_NCS_DBLIST_FIND_FIRST(&(p_vipd_rec_info)->intf_list);
        if( intf_node != NULL )
        {
           for(intf_cnt = 0; intf_cnt < p_vip_redRec->intf_list_cnt; intf_cnt++)
           {
               intf_list = (NCS_IFSV_VIP_INTF_LIST *)intf_node;

               /*p_temp_intf_rec->active_standby =*/ /*TBD::KISHORE*/
               m_NCS_STRCPY(p_temp_intf_rec->intfName, intf_list->intf_name);
         
               intf_node = ncs_db_link_list_find_next(&(p_vipd_rec_info)->intf_list, (uns8 *)(&p_temp_intf_rec->intfName) );
               if(intf_node != IFSV_NULL)
                  p_temp_intf_rec++;
               else
                  break;
           }/*for */
        }/* if( intf_node != NULL ) */
        else
          res = NCSCC_RC_FAILURE;
     }/*if( p_vip_redRec->intfInfo != IFSV_NULL)*/
     else
        res = NCSCC_RC_FAILURE;
  }
  else
     res = NCSCC_RC_FAILURE;

if( res == NCSCC_RC_FAILURE )
{
        m_MMGR_FREE_IFSV_VIP_RED_IP_REC(p_vip_redRec->ipInfo);
        return NCSCC_RC_FAILURE;
}


 /* Filling the OwnerList Related Info */
  p_vip_redRec->owner_list_cnt = ((NCS_DB_LINK_LIST *)&p_vipd_rec_info->owner_list)->n_nodes;
  if(p_vip_redRec->owner_list_cnt)
  {
    p_vip_redRec->ownerInfo = m_MMGR_ALLOC_IFSV_VIP_RED_OWNER_REC(p_vip_redRec->owner_list_cnt);
    if(p_vip_redRec->ownerInfo != IFSV_NULL)
    {
       m_NCS_MEMSET(p_vip_redRec->ownerInfo, 0, (p_vip_redRec->owner_list_cnt)*sizeof(VIP_RED_OWNER_NODE));
       p_temp_owner_rec = p_vip_redRec->ownerInfo;
     
       owner_node = m_NCS_DBLIST_FIND_FIRST(&(p_vipd_rec_info)->owner_list);
       if( owner_node != NULL )
       {
          for(owner_cnt = 0; owner_cnt < p_vip_redRec->owner_list_cnt; owner_cnt++)
          {
              owner_list = (NCS_IFSV_VIP_OWNER_LIST *)owner_node;
              p_temp_owner_rec->owner = owner_list->owner;
      
              owner_node = ncs_db_link_list_find_next(&(p_vipd_rec_info)->owner_list, (uns8 *)(&p_temp_owner_rec->owner) );
              if(owner_node != IFSV_NULL)
                 p_temp_owner_rec++;
              else
                 break;
          }/* for */

       }/* if( owner_node != NULL ) */
       else
         res = NCSCC_RC_FAILURE;

     } /* if(p_vip_redRec->ownerInfo != IFSV_NULL)*/
     else
     res = NCSCC_RC_FAILURE;

  }/*if(p_vip_redRec->owner_list_cnt) */
  else
     res = NCSCC_RC_FAILURE;

   if( res == NCSCC_RC_FAILURE )
   {
       m_MMGR_FREE_IFSV_VIP_RED_IP_REC(p_vip_redRec->ipInfo);
       m_MMGR_FREE_IFSV_VIP_RED_INTF_REC(p_vip_redRec->intfInfo);
       return NCSCC_RC_FAILURE;
   }

   return res;

} /* function ifd_populate_vip_redundancy_rec() ends here. */



/****************************************************************************
 * Name          : ifd_a2s_vip_rec_create_send
 *
 * Description   : Function to send VIP info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_vip_rec_create_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  IFSV_IFD_VIPD_RECORD  *info;
  uns32 res = NCSCC_RC_SUCCESS;
/* TBD:::LOGS
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_VIP_CREATE,\
                  "VIP rec data create.",\
                  0);
*/

  info = (IFSV_IFD_VIPD_RECORD *)msg;

  a2s_msg->type = IFD_A2S_EVT_VIP_REC_INFO;
  a2s_msg->info.ifd_a2s_vip_rec_info_evt.type =
                                            IFD_A2S_VIP_REC_CREATE;
  /* Convert the VIP rec to send.*/
  res = ifd_populate_vip_redundancy_rec(info, &a2s_msg->info.ifd_a2s_vip_rec_info_evt.info);
  /* Do an MDS send */
  return res;

} /* function ifd_a2s_vip_rec_create_send() ends here. */

/****************************************************************************
 * Name          : ifd_a2s_vip_rec_modify_send
 *
 * Description   : Function to send VIP info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_vip_rec_modify_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  IFSV_IFD_VIPD_RECORD  *p_vipd_rec;
  uns32 res = NCSCC_RC_SUCCESS;
  /* TBD :: LOGS
    m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_VIP_MODIFY,\
                    "VIP rec data Modify.",\
                    0);
   */

  p_vipd_rec = (IFSV_IFD_VIPD_RECORD *)msg;

  a2s_msg->type = IFD_A2S_EVT_VIP_REC_INFO;
  a2s_msg->info.ifd_a2s_vip_rec_info_evt.type =
                                            IFD_A2S_VIP_REC_MODIFY;
  /* Convert the VIp rec to send.*/
  res = ifd_populate_vip_redundancy_rec(p_vipd_rec, &a2s_msg->info.ifd_a2s_vip_rec_info_evt.info);
  return res;

} /* function ifd_a2s_vip_rec_modify_send() ends here. */


/****************************************************************************
 * Name          : ifd_a2s_vip_rec_del_send
 *
 * Description   : Function to send VIP info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_vip_rec_del_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  IFSV_IFD_VIPD_RECORD  *p_vipd_rec;
  uns32 res = NCSCC_RC_SUCCESS;
  /* TBD :: LOGS
    m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_VIP_CREATE,\
                    "VIP rec data create.",\
                    0);
   */

  p_vipd_rec = (IFSV_IFD_VIPD_RECORD *)msg;

  a2s_msg->type = IFD_A2S_EVT_VIP_REC_INFO;
  a2s_msg->info.ifd_a2s_vip_rec_info_evt.type =
                                            IFD_A2S_VIP_REC_DEL;
  /* Convert the VIp rec to send.*/
  res = ifd_populate_vip_redundancy_rec(p_vipd_rec, &a2s_msg->info.ifd_a2s_vip_rec_info_evt.info);
  return res;

} /* function ifd_a2s_vip_rec_del_send() ends here. */

#endif

/****************************************************************************
 * Name          : ifd_a2s_ifnd_down_send
 *
 * Description   : Function to send IFND DOWN info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_ifnd_down_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  a2s_msg->type = IFD_A2S_EVT_IFND_UP_DOWN;
  a2s_msg->info.ifd_a2s_ifnd_up_down_evt.type = IFD_A2S_IFND_DOWN_EVT;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_ifnd_up_down_evt.mds_dest, msg, 
               sizeof(MDS_DEST));

  return NCSCC_RC_SUCCESS;

} /* function ifd_a2s_ifnd_down_send() ends here. */

/****************************************************************************
 * Name          : ifd_a2s_ifnd_up_send
 *
 * Description   : Function to send IFND UP info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_ifnd_up_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  a2s_msg->type = IFD_A2S_EVT_IFND_UP_DOWN;
  a2s_msg->info.ifd_a2s_ifnd_up_down_evt.type = IFD_A2S_IFND_UP_EVT;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_ifnd_up_down_evt.mds_dest, msg, 
               sizeof(MDS_DEST));

  return NCSCC_RC_SUCCESS;

} /* function ifd_a2s_ifnd_up_send() ends here. */

/****************************************************************************
 * Name          : ifd_a2s_ifindex_alloc_send
 *
 * Description   : Function to send IFINDEX ALLOC info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_ifindex_alloc_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,\
                  "If Index allocation. Index : ",\
                  *((NCS_IFSV_IFINDEX *)msg));
  a2s_msg->type = IFD_A2S_EVT_IFINDEX_UPD;
  a2s_msg->info.ifd_a2s_ifindex_upd_evt.type = IFD_A2S_IFINDEX_EVT_ALLOC;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_ifindex_upd_evt.ifindex, msg, 
               sizeof(NCS_IFSV_IFINDEX));

  return NCSCC_RC_SUCCESS;

} /* function ifd_a2s_ifindex_alloc_send() ends here. */

/****************************************************************************
 * Name          : ifd_a2s_ifindex_dealloc_send 
 *
 * Description   : Function to send IFINDEX DEALLOC info.
 *
 * Arguments     : msg - pointer to the message.
 *                 a2s_msg -- IFD_A2S_MSG async message to
 *                 standby IFD
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_a2s_ifindex_dealloc_send(IFD_A2S_MSG *a2s_msg, char *msg)
{
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,\
                  "If Index Deallocation. Index : ",\
                  *((NCS_IFSV_IFINDEX *)msg));
  a2s_msg->type = IFD_A2S_EVT_IFINDEX_UPD;
  a2s_msg->info.ifd_a2s_ifindex_upd_evt.type = IFD_A2S_IFINDEX_EVT_DEALLOC;
  m_NCS_MEMCPY(&a2s_msg->info.ifd_a2s_ifindex_upd_evt.ifindex, msg, 
               sizeof(NCS_IFSV_IFINDEX));

  return NCSCC_RC_SUCCESS;

} /* function ifd_a2s_ifindex_dealloc_send() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_async_update 
 *
 * Description   : To process the async update from active ifd to standby.
 *
 * Arguments     : cb: IFD_CB pointer.
 *                 msg: Active to standby message pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_mbcsv_async_update(IFSV_CB *cb, IFD_A2S_MSG *msg)
{
   NCS_MBCSV_ARG  arg;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&arg,'\0',sizeof(NCS_MBCSV_ARG));
   arg.i_op                      = NCS_MBCSV_OP_SEND_CKPT;
   arg.i_mbcsv_hdl               = cb->mbcsv_hdl;
   arg.info.send_ckpt.i_ckpt_hdl = cb->o_ckpt_hdl;
   arg.info.send_ckpt.i_send_type= NCS_MBCSV_SND_SYNC;
   arg.info.send_ckpt.i_reo_type = msg->type;
   arg.info.send_ckpt.i_reo_hdl  = (long)msg;
   arg.info.send_ckpt.i_action   = NCS_MBCSV_ACT_UPDATE;
   
   if(ncs_mbcsv_svc(&arg) != SA_AIS_OK)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Sync Send Error",0);
      rc = NCSCC_RC_FAILURE;
   }

   m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                        "Mbcsv Sync Send Success",0);

   return rc;
} /* function ifd_mbcsv_async_update() ends here. */

/*****************************************************************************
 *                  MBCSV Functions 
 *****************************************************************************/

/****************************************************************************
 * Name          : ifd_mbcsv_register 
 *
 * Description   : Function to initialze the mbcsv instance and open a 
 *                 session and select.
 *
 * Arguments     : cb : cb pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifd_mbcsv_register(IFSV_CB *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Put an MBCSV instance in start state. */
   rc = ifd_mbcsv_init(cb);
   if(rc != NCSCC_RC_SUCCESS)
    {
    m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_init():ifd_mbcsv_register returned failure,rc:",rc);
    return rc;
    }

   /* Establish contact with peer entity. */
   rc = ifd_mbcsv_open(cb);
   if(rc != NCSCC_RC_SUCCESS)
   {
    /* Log the error. */
     m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_open():ifd_mbcsv_register returned failure,rc:",rc);
     goto open_error;
   }

   /* Set Warm Sync Timer to 5 min. */
   rc = ifd_mbcsv_warm_sync_timer_set(cb);
   if(rc != NCSCC_RC_SUCCESS)
   {
    /* Log the error. */
     m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_warm_sync_timer_set() in ifd_mbcsv_register returned failure,rc:",rc);
     goto open_error;
   }

   /* Selection object get */
   rc = ifd_mbcsv_selobj_get(cb);
   if(rc != NCSCC_RC_SUCCESS)
   {
    /* Log the error. */
     m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_selobj_get():ifd_mbcsv_register returned failure,rc:",rc);
     goto obj_get_error;
   }

  /* If every thing is OK, then return from here. */ 
  return rc;
   

obj_get_error:
  if(NCSCC_RC_SUCCESS != ifd_mbcsv_close(cb));
  /* Log the error. */
open_error:
  if(NCSCC_RC_SUCCESS != ifd_mbcsv_finalize(cb));
   /* Log the error. */

   return rc; 

} /* function ifd_mbcsv_register() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_init 
 *
 * Description   : This function initialzes the mbcsv instance.
 *
 * Arguments     : cb : Control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_init(IFSV_CB *cb)
{
   NCS_MBCSV_ARG arg;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&arg,'\0',sizeof(NCS_MBCSV_ARG));

   arg.i_op                       = NCS_MBCSV_OP_INITIALIZE;
   arg.info.initialize.i_mbcsv_cb = ifd_mbcsv_callback;
   arg.info.initialize.i_version  = IFD_MBCSV_VERSION;
   arg.info.initialize.i_service  = NCS_SERVICE_ID_IFD;

   if(ncs_mbcsv_svc(&arg) != SA_AIS_OK)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Init Error",0);
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   cb->mbcsv_hdl = arg.info.initialize.o_mbcsv_hdl;
   m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Init Success",0);
   return rc;
} /* function ifd_mbcsv_init() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_open 
 *
 * Description   : This function establishes contact with peer entity.
 *
 * Arguments     : cb : Control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_open(IFSV_CB *cb)
{
   NCS_MBCSV_ARG arg;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&arg,'\0',sizeof(NCS_MBCSV_ARG));

   arg.i_op                  = NCS_MBCSV_OP_OPEN;
   arg.i_mbcsv_hdl           = cb->mbcsv_hdl;
   arg.info.open.i_pwe_hdl   = (uns32)cb->my_mds_hdl;
   arg.info.open.i_client_hdl= cb->cb_hdl;
   
   if(ncs_mbcsv_svc(&arg) != SA_AIS_OK)
   {
      /* Log the error */
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Open Error",0);
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
   
   cb->o_ckpt_hdl= arg.info.open.o_ckpt_hdl;
   m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Open Success",0);
   return rc;
} /* function ifd_mbcsv_open() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_warm_sync_timer_set
 *
 * Description   : This function sets Warm Sync Timer to 5 min.
 *
 * Arguments     : cb : Control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_warm_sync_timer_set(IFSV_CB *cb)
{
   NCS_MBCSV_ARG arg;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&arg,'\0',sizeof(NCS_MBCSV_ARG));

   arg.i_op                  = NCS_MBCSV_OP_OBJ_SET;
   arg.i_mbcsv_hdl           = cb->mbcsv_hdl;
   arg.info.obj_set.i_ckpt_hdl  = cb->o_ckpt_hdl;
   arg.info.obj_set.i_obj       = NCS_MBCSV_OBJ_TMR_WSYNC;
   arg.info.obj_set.i_val       = 30000; /* 5 min */

   if(ncs_mbcsv_svc(&arg) != SA_AIS_OK)
   {
      /* Log the error */
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Warm Sync Timer Set Error",0);
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Warm Sync Timer Set Success",0);
   return rc;
} /* function ifd_mbcsv_warm_sync_timer_set() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_close
 *
 * Description   : This function closes contact with peer entity.
 *
 * Arguments     : cb : Control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_close(IFSV_CB *cb)
{
   NCS_MBCSV_ARG arg;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&arg,'\0',sizeof(NCS_MBCSV_ARG));

   arg.i_op                  = NCS_MBCSV_OP_CLOSE;
   arg.i_mbcsv_hdl           = cb->mbcsv_hdl;

   if(ncs_mbcsv_svc(&arg) != SA_AIS_OK)
   {
      /* Log the error */
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Close Error",0);
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
 
   m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Close Success",0);
   return rc;
} /* function ifd_mbcsv_close() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_finalize 
 *
 * Description   : This function close the servise instance of MBCSV.
 *
 * Arguments     : cb : Control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Closes the association, represented by i_mbc_hdl, between 
 *                 the invoking process and MBCSV.
 *****************************************************************************/
uns32  ifd_mbcsv_finalize(IFSV_CB *cb)
{
   NCS_MBCSV_ARG arg;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&arg,'\0',sizeof(NCS_MBCSV_ARG));

   arg.i_op                  = NCS_MBCSV_OP_FINALIZE;
   arg.i_mbcsv_hdl           = cb->mbcsv_hdl;
   
   if(ncs_mbcsv_svc(&arg) != SA_AIS_OK)
   {
      /* Log the error */
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Finalized Error",0);
      rc = NCSCC_RC_FAILURE;
   }

   m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Finalized Success",0);
   return rc;
} /* function ifd_mbcsv_finalize() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_selobj_get 
 *
 * Description   : This function gets selection obj to detect pending call
 *                 backs.
 *
 * Arguments     : cb : Control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32  ifd_mbcsv_selobj_get(IFSV_CB *cb)
{
    NCS_MBCSV_ARG arg;
    uns32 rc = NCSCC_RC_SUCCESS;

    m_NCS_MEMSET(&arg,'\0',sizeof(NCS_MBCSV_ARG));

    arg.i_op                           =  NCS_MBCSV_OP_SEL_OBJ_GET;
    arg.i_mbcsv_hdl                    =  cb->mbcsv_hdl;
    arg.info.sel_obj_get.o_select_obj  =  0;

    if(ncs_mbcsv_svc(&arg) != SA_AIS_OK)
    {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Obj Get Error",0);
      rc = NCSCC_RC_FAILURE;
      return rc;
    }

    cb->mbcsv_sel_obj  = arg.info.sel_obj_get.o_select_obj;

    m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Obj Get Success",0);
    return rc;
} /* function ifd_mbcsv_selobj_get() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_chg_role 
 *
 * Description   : Set the role of MBCSV instance depending on hastate of the 
 *                 client.
 *
 * Arguments     : cb : Control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32  ifd_mbcsv_chgrole(IFSV_CB *cb)
{
   NCS_MBCSV_ARG  arg;
   uns32 rc = NCSCC_RC_SUCCESS;
   m_NCS_MEMSET(&arg,'\0',sizeof(NCS_MBCSV_ARG));

   arg.i_op                      = NCS_MBCSV_OP_CHG_ROLE;
   arg.i_mbcsv_hdl               = cb->mbcsv_hdl;
   arg.info.chg_role.i_ckpt_hdl  = cb->o_ckpt_hdl;
   /* ha_state is assigned at the time of amf_init where csi_set_callback will 
      assign the state */
   arg.info.chg_role.i_ha_state  = cb->ha_state;

   if(ncs_mbcsv_svc(&arg) != SA_AIS_OK )
   {
      /* Log the error */
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Change Role Error",0);
      rc = NCSCC_RC_FAILURE;
   }

    m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Change Role Success",0);
  return rc;
} /* function ifd_mbcsv_chg_role() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_dispatch 
 *
 * Description   : This function asks for dispatching the pending callbacks.
 *
 * Arguments     : cb : Control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32  ifd_mbcsv_dispatch(IFSV_CB *cb)
{
   NCS_MBCSV_ARG  arg;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&arg,'\0',sizeof(NCS_MBCSV_ARG));

   arg.i_op                      = NCS_MBCSV_OP_DISPATCH;
   arg.i_mbcsv_hdl               = cb->mbcsv_hdl;
   arg.info.dispatch.i_disp_flags= SA_DISPATCH_ALL;

   if(ncs_mbcsv_svc(&arg) != SA_AIS_OK )
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Dispatch Error",0);
      rc = NCSCC_RC_FAILURE;
   }

    m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Mbcsv Dispatch Success",0);

   return rc;

} /* function ifd_mbcsv_dispatch() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_callback 
 *
 * Description   : This function is called when there are any pending 
 *                 callbacks.
 *
 * Arguments     : arg - Pointer to the structure containing information 
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32  ifd_mbcsv_callback(NCS_MBCSV_CB_ARG *arg)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   if(arg == NULL)
   {
     m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                     "Mbcsv Callback, NULL pointer",0);
     rc = NCSCC_RC_FAILURE;
     /* log the message */
     return rc;
   }

   switch(arg->i_op)
   {
     case NCS_MBCSV_CBOP_ENC:
       rc = ifd_mbcsv_encode_proc(arg);
       if(rc != NCSCC_RC_SUCCESS)
       m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_encode_proc():ifd_mbcsv_callback returned failure,rc:",rc);
       break;

     case NCS_MBCSV_CBOP_DEC:
       rc = ifd_mbcsv_decode_proc(arg);
       if(rc != NCSCC_RC_SUCCESS)
       m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_decode_proc():ifd_mbcsv_callback returned failure,rc:",rc);
       break;

     case NCS_MBCSV_CBOP_PEER:
       rc = ifd_mbcsv_peer_info_cbk_handler(arg);
       break;

     case NCS_MBCSV_CBOP_NOTIFY:
       break;

     case NCS_MBCSV_CBOP_ERR_IND:
       break;

     default :
       rc = NCSCC_RC_FAILURE;
       break;
   } /* switch(arg->i_op) */

   return rc;

} /* function ifd_mbcsv_callback() ends here. */


/****************************************************************************
 * Name          : ifd_mbcsv_encode_proc
 *
 * Description   : This function encodes the async update, cold sync, warm 
 *                 sync and data response.
 *
 * Arguments     : arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static 
uns32  ifd_mbcsv_encode_proc(NCS_MBCSV_CB_ARG *arg)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   IFSV_CB *cb;
   uns32   ifsv_hdl;
   uns16 msg_fmt_version;
  
   msg_fmt_version = m_NCS_MBCSV_FMT_GET(arg->info.encode.i_peer_version,
                                         IFD_MBCSV_VERSION,
                                         IFD_MBCSV_VERSION_MIN); 
   if(0 == msg_fmt_version) 
   {
    m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
          "Encode peer version not supported",arg->info.encode.i_peer_version);
    return NCSCC_RC_FAILURE;
   }

   ifsv_hdl = m_IFD_CB_HDL_GET();

   cb = (IFSV_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, ifsv_hdl);
   
   if(cb == NULL)
   {
     m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                     "Encode Failure, NULL CB",0);
     return (NCSCC_RC_FAILURE);
   }
  
   /* embedding subslot changes for backward compatibility*/
   cb->edu_hdl.to_version = msg_fmt_version;

   switch(arg->info.encode.io_msg_type)
   {

      case NCS_MBCSV_MSG_ASYNC_UPDATE:
         rc = ifd_mbcsv_enc_async_update(cb, arg);
         if(rc != NCSCC_RC_SUCCESS)
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_enc_async_update():ifd_mbcsv_encode_proc returned failure,rc:",rc);
         break;

      case NCS_MBCSV_MSG_COLD_SYNC_REQ:
         cb->cold_or_warm_sync_on = TRUE;
         m_NCS_CONS_PRINTF("Cold sync started\n");
         m_IFSV_LOG_HEAD_LINE(cb->my_svc_id,IFSV_WARM_COLD_SYNC_START, 0, 0);
         break;

      case NCS_MBCSV_MSG_COLD_SYNC_RESP:
         rc =  ifd_mbcsv_enc_data_resp((void *)cb, arg);
         if(rc != NCSCC_RC_SUCCESS)
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_enc_data_resp():ifd_mbcsv_encode_proc returned failure,rc:",rc);
         break;

      case NCS_MBCSV_MSG_WARM_SYNC_REQ:
         break;

      case NCS_MBCSV_MSG_WARM_SYNC_RESP:
         rc = ifd_mbcsv_enc_warm_sync_resp(cb, arg);
         if(rc != NCSCC_RC_SUCCESS)
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_enc_warm_sync_resp():ifd_mbcsv_encode_proc returned failure,rc:",rc);
         break;

      case NCS_MBCSV_MSG_DATA_REQ:
         break;

      case NCS_MBCSV_MSG_DATA_RESP:
         rc = ifd_mbcsv_enc_data_resp(cb, arg);
         if(rc != NCSCC_RC_SUCCESS)
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_enc_data_resp():ifd_mbcsv_encode_proc returned failure,rc:",rc);
         break;

      default:
         rc = NCSCC_RC_FAILURE;
         break;
     } /* switch (arg->info.encode.io_msg_type)  */


    return rc;
} /* function ifd_mbcsv_encode_proc() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_enc_async_update
 *
 * Description   : This function encodes the async update message.
 *
 * Arguments     : cb - Control block
 *                 arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_enc_async_update(IFSV_CB *cb, NCS_MBCSV_CB_ARG *arg)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   EDU_ERR   ederror = 0;
   IFD_A2S_MSG *msg;
  
   switch(arg->info.encode.io_reo_type)
   {
      
      case IFD_A2S_EVT_IFINDEX_SPT_MAP:
      {
         msg = (IFD_A2S_MSG *)(long)arg->info.encode.io_reo_hdl;
         rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
              &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, msg, &ederror, cb->edu_hdl.to_version);
         if(rc != NCSCC_RC_SUCCESS)
         {
            m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"encode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_IFINDEX_SPT_MAP:ifd_mbcsv_enc_async_update rc:",rc);
            m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Encode Failure, Spt Map",0);
            /*LOG */
            rc = NCSCC_RC_FAILURE;
         }      
       break; 
      }
      
      case IFD_A2S_EVT_INTF_DATA:
      {
         msg = (IFD_A2S_MSG *)(long)arg->info.encode.io_reo_hdl;
         rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
              &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, msg, &ederror, cb->edu_hdl.to_version);
         if(rc != NCSCC_RC_SUCCESS)
         {
            /*LOG */
            m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"encode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_INTF_DATA:ifd_mbcsv_enc_async_update rc:",rc);
            m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Encode Failure, Intf Data",0);
            rc = NCSCC_RC_FAILURE;
         }      
       break; 
      }
      
      case IFD_A2S_EVT_SVC_DEST_UPD:
      {
         msg = (IFD_A2S_MSG *)(long)arg->info.encode.io_reo_hdl;
         rc = m_NCS_EDU_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
              &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, msg, &ederror);
         if(rc != NCSCC_RC_SUCCESS)
         {
            /*LOG */
            m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"encode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_SVC_DEST_UPD:ifd_mbcsv_enc_async_update rc:",rc);
            m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Encode Failure, SVCD Dest",0);
            rc = NCSCC_RC_FAILURE;
         }      
       break; 
      }

     case IFD_A2S_EVT_IFND_UP_DOWN:
     {
         msg = (IFD_A2S_MSG *)(long)arg->info.encode.io_reo_hdl;
         rc = m_NCS_EDU_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
              &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, msg, &ederror);
         if(rc != NCSCC_RC_SUCCESS)
         {
           /*LOG */
           m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"encode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_IFND_UP_DOWN:ifd_mbcsv_enc_async_update rc:",rc);
           m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Encode Failure, IfND UP/DOWN",0);
           rc = NCSCC_RC_FAILURE;
         }
       break;
     }

     case IFD_A2S_EVT_IFINDEX_UPD:
      {
         msg = (IFD_A2S_MSG *)(long)arg->info.encode.io_reo_hdl;
         rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
              &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, msg, &ederror, cb->edu_hdl.to_version);
         if(rc != NCSCC_RC_SUCCESS)
         {
            /*LOG */
           m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"encode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_IFINDEX_UPD:ifd_mbcsv_enc_async_update rc:",rc);
           m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Encode Failure, If Index UPD",0);
           rc = NCSCC_RC_FAILURE;
         }
       break;
      }
#if (NCS_IFSV_IPXS == 1)
     case IFD_A2S_EVT_IPXS_INTF_INFO:
      {
         msg = (IFD_A2S_MSG *)(long)arg->info.encode.io_reo_hdl;
         rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
              &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, msg, &ederror, cb->edu_hdl.to_version);
         if(rc != NCSCC_RC_SUCCESS)
         {
            /*LOG */
           m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"encode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_IPXS_INTF_INFO:ifd_mbcsv_enc_async_update rc:",rc);
           m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Encode Failure, IPXS Update",0);
           rc = NCSCC_RC_FAILURE;
         }
       break;
      }

#endif  
#if (NCS_VIP == 1)
     case IFD_A2S_EVT_VIP_REC_INFO:
      {
         msg = (IFD_A2S_MSG *)(long)arg->info.encode.io_reo_hdl;
         rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
              &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, msg, &ederror, cb->edu_hdl.to_version);
         if(rc != NCSCC_RC_SUCCESS)
         {
            /*LOG */
           m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"encode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_VIP_REC_INFO:ifd_mbcsv_enc_async_update rc:",rc);
           m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Encode Failure, VIP Update",0);
           rc = NCSCC_RC_FAILURE;
         }
       break;
      }

#endif  
      default :
          m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Encode Failure, Unkown msg type : ",\
                           arg->info.encode.io_reo_type);

        rc = NCSCC_RC_FAILURE;
        break;
      
   } /* arg->info.encode.io_reo_type  */

   return rc;

} /* function ifd_mbcsv_enc_async_update() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_enc_warm_sync_resp
 *
 * Description   : This function encodes the warm sync resp message.
 *
 * Arguments     : cb - Control block
 *                 arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_enc_warm_sync_resp(IFSV_CB *cb, NCS_MBCSV_CB_ARG *arg)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns8 *wsync_ptr = NULL, *free_ifindex_ptr = NULL, *max_ifindex_ptr = NULL;
   uns32 num_free_ifindex = 0, num_max_ifindex = 0, num_sptmap_present = 0, num_ifrec_present =0;
   NCS_BOOL db_mismatch_flag = FALSE;
   /* Reserve space to send the async update counter */
   wsync_ptr = ncs_enc_reserve_space(&arg->info.encode.io_uba,sizeof(uns32));

   if(wsync_ptr == NULL)
   {
      /*TBD */
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Warm Sync Encode Failure",0);
      return NCSCC_RC_FAILURE;
   }
 
   /* SEND THE ASYNC UPDATE COUNTER */   
   ncs_enc_claim_space(&arg->info.encode.io_uba,sizeof(uns32));
   ncs_encode_32bit(&wsync_ptr, cb->ifd_async_cnt);

   ifd_ifap_max_num_ifindex_and_num_free_ifindex_get(&num_max_ifindex, &num_free_ifindex);

   num_ifrec_present = cb->if_tbl.n_nodes; 
   num_sptmap_present = cb->if_map_tbl.n_nodes; 
   if(num_ifrec_present != num_sptmap_present)
   {
     db_mismatch_flag = TRUE;
     /* This means that SPT map is there but Intf Record is not there. */
     m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Active IfD DB Mismatch between IfRec and SPT Map: num_ifrec_present, num_sptmap_present, num_max_ifindex,num_free_ifindex",num_ifrec_present, num_sptmap_present, num_max_ifindex,num_free_ifindex,0,0,0);
     ifd_mbcsv_db_ifrec_sptmap_mismatch_correction(cb);
     /* Check whether the data base is correct or not. */
      ifd_ifap_max_num_ifindex_and_num_free_ifindex_get(&num_max_ifindex, &num_free_ifindex);
      num_ifrec_present = cb->if_tbl.n_nodes; 
      num_sptmap_present = cb->if_map_tbl.n_nodes; 
     m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "After DB correction at Active IFD between IfRec and SPT Map : num_ifrec_present, num_sptmap_present, num_max_ifindex, num_free_ifindex",num_ifrec_present, num_sptmap_present, num_max_ifindex,num_free_ifindex,0,0,0);
     if(num_ifrec_present != num_sptmap_present)
     {
       m_IFD_LOG_FUNC_ENTRY_CRITICAL_INFO(IFSV_LOG_IFD_MSG, "After DB correction at Active IFD between IfRec and SPT Map, again Mismatch in Act IFD DB : num_ifrec_present, num_sptmap_present, num_max_ifindex, num_free_ifindex",num_ifrec_present, num_sptmap_present, num_max_ifindex,num_free_ifindex,0,0,0);
     }
   } /* if(num_ifrec_present != num_sptmap_present) */

   ifd_ifap_max_num_ifindex_and_num_free_ifindex_get(&num_max_ifindex, &num_free_ifindex);

   if(num_sptmap_present != (num_max_ifindex - num_free_ifindex))  
   {
     db_mismatch_flag = TRUE;
     /* This means that IfD has more IfIndex allocated than the intf record present. */
     m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Act IfD DB Mismatch : num_max_ifindex, num_free_ifindex, num_sptmap_present ",num_max_ifindex, num_free_ifindex, num_sptmap_present,0,0,0,0);
     ifd_mbcsv_db_mismatch_correction(cb,num_max_ifindex);
     /* Check whether the data base is correct or not. */
     num_max_ifindex = 0;
     num_free_ifindex = 0; 
     ifd_ifap_max_num_ifindex_and_num_free_ifindex_get(&num_max_ifindex, &num_free_ifindex);
     num_sptmap_present = cb->if_map_tbl.n_nodes; 
     m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "After DB correction : num_max_ifindex, num_free_ifindex, num_sptmap_present",num_max_ifindex, num_free_ifindex, num_sptmap_present,0,0,0,0);
     if(num_sptmap_present != (num_max_ifindex - num_free_ifindex))
     {
       m_IFD_LOG_FUNC_ENTRY_CRITICAL_INFO(IFSV_LOG_IFD_MSG, "After DB correction, again Mismatch in Act IFD DB : num_max_ifindex, num_free_ifindex, num_sptmap_present",num_max_ifindex, num_free_ifindex, num_sptmap_present,0,0,0,0);
     }
   }

   if(db_mismatch_flag == TRUE)
   {
     /*  Force STDBY IFD to download DB again from ACT IFD. */
     num_max_ifindex = 0;
     num_free_ifindex = 0;
   }
     /* Reserve space to send the async update counter */
   max_ifindex_ptr = ncs_enc_reserve_space(&arg->info.encode.io_uba,sizeof(uns32));

   if(max_ifindex_ptr == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Warm Sync Encode Failure : max_ifindex_ptr",0);
      return NCSCC_RC_FAILURE;
   }

   ncs_enc_claim_space(&arg->info.encode.io_uba,sizeof(uns32));
   ncs_encode_32bit(&max_ifindex_ptr, num_max_ifindex);
   /* Reserve space to send the num_free_ifindex counter */
   free_ifindex_ptr = ncs_enc_reserve_space(&arg->info.encode.io_uba,sizeof(uns32));

   if(free_ifindex_ptr == NULL)
   {
      /*TBD */
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Warm Sync Encode Failure : free_ifindex_ptr",0);
      return NCSCC_RC_FAILURE;
   }

   ncs_enc_claim_space(&arg->info.encode.io_uba,sizeof(uns32));
   ncs_encode_32bit(&free_ifindex_ptr, num_free_ifindex);
 

   arg->info.encode.io_msg_type = NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE;

   return rc;

} /* function ifd_mbcsv_enc_warm_sync_resp() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_enc_data_resp
 *
 * Description   : This function encodes data resp message. Each 
 *                 interface record, includes SVC DEST info to Standby IfD. It
 *                 sends FIXED no. of record at each time, if no. of records to
 *                 to be sent is more than FIXED no., else the no. of records
 *                 left.   
 *
 * Arguments     : ifsv_cb - Control block
 *                 arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Each ckpy record has INTF record + IPXS data. After the last
 *                 message, it will send the IAPS Data as ckpt rec. If there is
 *                 no record, then it will send only IAPS Data as ckpt rec.
 *                 After that it will start sending VIP rec. At the end of VIP
 *                 Rec, it will send as COLD SYNC COMPLETE. 
 |----------|-------|-------------------|--------------|-----------|------|-----------|-----------| 
 |  If Rec/ | Last  | No. of Ckpts      |ckpt record 1 |ckpt rec 2 |...   |ckpt rec n | async upd |
 |  VIP Rec | Msg   | that will be sent |              |           |      |           | cnt       |
 |----------|-------|-------------------|------------- |-----------|------|-----------|-----------|
 *****************************************************************************/
uns32  ifd_mbcsv_enc_data_resp(void *cb, NCS_MBCSV_CB_ARG *arg)
{
   IFSV_INTF_REC       *intf_rec;
   IFSV_CB             *ifsv_cb = (IFSV_CB *)cb;
   NCS_PATRICIA_NODE   *if_node;   
   NCS_LOCK            *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 num_of_ckpts = 0;
   EDU_ERR ederror = 0;   
   uns8 *header = NULL, *sync_cnt_ptr = NULL, *ifsv_vip_rec_ptr = NULL, *last_msg= NULL;
   uns32 ifindex = 0, last_msg_flag = FALSE;
   IFAP_INFO_LIST_A2S ifap_info; 

#if(NCS_IFSV_IPXS == 1)    
   IPXS_CB  *ipxs_cb;
   uns32    ipxs_hdl;
   IPXS_IFIP_NODE   *ifip_node = NULL;
   IPXS_IFIP_INFO   *ip_info = NULL;
   NCS_IPXS_IPINFO  send_ip_info;
#endif

   /* COLD_SYNC_RESP IS DONE BY THE ACTIVE */
   if(ifsv_cb->ha_state == SA_AMF_HA_STANDBY)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                      "Data Encode Failure, IfD is in STDBY mode",0);
      rc = NCSCC_RC_FAILURE;
      return rc;
   }      

#if(NCS_IFSV_IPXS == 1)    
   /* Get the IPXS CB */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, ipxs_hdl);
#endif

   /*First reserve space to store the IfSv rec or Vip Rec that will be sent*/
   ifsv_vip_rec_ptr = ncs_enc_reserve_space(&arg->info.encode.io_uba, sizeof(uns32));
   if(ifsv_vip_rec_ptr == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                 "Data Encode Failure, ncs_enc_reserve_space returned NULL",0);
      m_NCS_CONS_PRINTF("Data Encode Failure, ncs_enc_reserve_space returned NULL : ifsv_vip_rec_ptr: ifd_mbcsv_enc_data_resp");
      return NCSCC_RC_FAILURE;
   }
    
   ncs_enc_claim_space(&arg->info.encode.io_uba, sizeof(uns32));
   ncs_encode_32bit(&ifsv_vip_rec_ptr, IFD_DATA_RES_IFSV_REC);

   /* First reserve space to store the last message.*/
   last_msg = ncs_enc_reserve_space(&arg->info.encode.io_uba, sizeof(uns32));
   if(last_msg == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                 "Data Encode Failure, ncs_enc_reserve_space returned NULL",0);
      m_NCS_CONS_PRINTF("Data Encode Failure, ncs_enc_reserve_space returned NULL last_msg:ifd_mbcsv_enc_data_resp");
      return NCSCC_RC_FAILURE;
   }

   ncs_enc_claim_space(&arg->info.encode.io_uba, sizeof(uns32));

   /*First reserve space to store the number of checkpoints that will be sent*/
   header = ncs_enc_reserve_space(&arg->info.encode.io_uba, sizeof(uns32));
   if(header == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                 "Data Encode Failure, ncs_enc_reserve_space returned NULL",0);
      m_NCS_CONS_PRINTF("Data Encode Failure, ncs_enc_reserve_space returned NULL : header : ifd_mbcsv_enc_data_resp");
      return NCSCC_RC_FAILURE;
   }

   ncs_enc_claim_space(&arg->info.encode.io_uba, sizeof(uns32));
   ifindex = ifsv_cb->record_ifindex;

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);

   if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
      (uns8*)&ifindex);
#if(NCS_IFSV_IPXS == 1)    
   ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                                  (&ipxs_cb->ifip_tbl, (uns8*)&ifindex);
#endif
   while ((if_node != IFSV_NULL) 
#if(NCS_IFSV_IPXS == 1)    
&& (ifip_node != IFSV_NULL)
#endif
)
   {
      intf_rec = (IFSV_INTF_REC*)if_node;
      ip_info = (IPXS_IFIP_INFO *)&ifip_node->ifip_info; 

      ifindex = intf_rec->intf_data.if_index;

      m_NCS_EDU_VER_EXEC(&ifsv_cb->edu_hdl, ifsv_edp_intf_data, 
           &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, &intf_rec->intf_data, 
           &ederror, ifsv_cb->edu_hdl.to_version);

      num_of_ckpts++; 

#if(NCS_IFSV_IPXS == 1)    
       /* Fill the IP information */
      rc = ipxs_ifsv_ifip_info_attr_cpy(&ifip_node->ifip_info, 
                                        &send_ip_info);
      m_NCS_EDU_VER_EXEC(&ifsv_cb->edu_hdl, ipxs_edp_ifip_info, 
           &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, &send_ip_info, &ederror, ifsv_cb->edu_hdl.to_version);
#endif
      if(num_of_ckpts == MAX_NO_IFD_MSGS_A2S)
      {
         break;       
      }

      if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl, (uns8*)&ifindex);
#if(NCS_IFSV_IPXS == 1)    
      ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                                  (&ipxs_cb->ifip_tbl, (uns8*)&ifindex);
#endif
   }/* while */

   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   ifsv_cb->record_ifindex = ifindex;
   if(num_of_ckpts < MAX_NO_IFD_MSGS_A2S)
   {
     /* This is a condition for the last message. So, we need to add IAPS info.
        and then mark this as a last message. */ 
     uns32 num_free_index;
     
     if(ifd_ifap_max_num_free_ifindex_get(&num_free_index) != NCSCC_RC_SUCCESS)
     {
       m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"Data Encode Failure, ifd_ifap_max_num_free_ifindex_get returned FAILURE : ifd_ifap_max_num_free_ifindex_get",0);
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
      "Data Encode Failure, ifd_ifap_max_num_free_ifindex_get returned FAILURE",0);
      m_NCS_CONS_PRINTF("Data Encode Failure, ifd_ifap_max_num_free_ifindex_get returned FAILURE : ifd_ifap_max_num_free_ifindex_get");
       return NCSCC_RC_FAILURE;
     }     
     
     m_NCS_OS_MEMSET(&ifap_info, 0, sizeof(IFAP_INFO_LIST_A2S));

     if(num_free_index)
     {
       ifap_info.free_list = m_MMGR_ALLOC_IFAP_INFO(num_free_index);     

       if(ifap_info.free_list == NULL)
       {
         m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
         m_NCS_CONS_PRINTF("Memory alloc failure in enc the data for ifap_info.free_list\n");
         return NCSCC_RC_FAILURE;
       }
     }

     ifap_info.num_free_ifindex = num_free_index;
     /* Call the IAPS function to fill the information. */
     ifd_ifap_ifindex_list_get((uns8 *)&ifap_info); 

     num_of_ckpts++; 
     m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Num of ckpts being sent, Max ifindex, Num free index, Async count",num_of_ckpts, ifap_info.max_ifindex, num_free_index,ifsv_cb->ifd_async_cnt,0,0,0);
     m_NCS_EDU_VER_EXEC(&ifsv_cb->edu_hdl, ifsv_edp_ifd_a2s_iaps_evt, 
           &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, &ifap_info, &ederror, ifsv_cb->edu_hdl.to_version);

     if(num_free_index)
       m_MMGR_FREE_IFAP_INFO(ifap_info.free_list);

#if (NCS_VIP != 1)
     if(arg->info.encode.io_msg_type == NCS_MBCSV_MSG_COLD_SYNC_RESP)
        arg->info.encode.io_msg_type = NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE;
     else
        if(arg->info.encode.io_msg_type == NCS_MBCSV_MSG_DATA_RESP)
           arg->info.encode.io_msg_type = NCS_MBCSV_MSG_DATA_RESP_COMPLETE; 
#endif
      /* Now reset the ifindex counter for the next warm sync. */
      ifsv_cb->record_ifindex = 0;
      last_msg_flag = TRUE;

#if (NCS_VIP == 1)
      ifsv_cb->ifd_mbcsv_data_resp_enc_func_ptr = ifd_mbcsv_enc_vip_data_resp;
#endif
   } /* if(num_of_ckpts < MAX_NO_IFD_MSGS_A2S) */

   ncs_encode_32bit(&header, num_of_ckpts);

   /* This will have the count of async updates that have been sent,
      this will be 0 initially*/
   sync_cnt_ptr = ncs_enc_reserve_space(&arg->info.encode.io_uba,sizeof(uns32));
   if(sync_cnt_ptr == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                 "Data Encode Failure, ncs_enc_reserve_space returned NULL",0);
      m_NCS_CONS_PRINTF("Data Encode Failure, ncs_enc_reserve_space returned NULL for sync_cnt_ptr:ifd_mbcsv_enc_data_resp");
      /*TBD */
      return NCSCC_RC_FAILURE;
   }

   ncs_enc_claim_space(&arg->info.encode.io_uba, sizeof(uns32));
   ncs_encode_32bit(&sync_cnt_ptr, ifsv_cb->ifd_async_cnt);
   ncs_encode_32bit(&last_msg, last_msg_flag);

   m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Data Encode Success",0);
   ifd_mbcsv_enc_vip_data_resp((void *)cb, arg);
   return rc;

} /* function () ends here. */

#if(NCS_VIP == 1)
/****************************************************************************
 * Name          : ifd_mbcsv_enc_vip_data_resp
 *
 * Description   : This function encodes data resp message. 
 *                 It sends FIXED no. of record at each time, if no. of records to
 *                 to be sent is more than FIXED no., else the no. of records
 *                 left. 
 *
 * Arguments     : ifsv_cb - Control block
 *                 arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Each ckpy record has VIP record. If there is
 *                 no record, then it will send number of rec only.
 *                 After that it will start sending VIP rec. 
 *                 It will send COLD SYNC COMPLETE in the end. 
 *                 Async update count is filled only when the last
 *                 record is IAPS, else it goes as zero.
 |----------|-------|-------------------|--------------|-----------|------|-----------|-----------| 
 |  If Rec/ | Last  | No. of Ckpts      |ckpt record 1 |ckpt rec 2 |...   |ckpt rec n | async upd |
 |  VIP Rec | Msg   | that will be sent |              |           |      |           | cnt       |
 |----------|-------|-------------------|------------- |-----------|------|-----------|-----------|
 *****************************************************************************/
static uns32  ifd_mbcsv_enc_vip_data_resp(void *cb, NCS_MBCSV_CB_ARG *arg)
{
   IFSV_IFD_VIPD_RECORD *p_vipd_rec = NULL;
   IFSV_CB *ifsv_cb = (IFSV_CB *)cb;
   VIP_REDUNDANCY_RECORD vip_redRec;
   NCS_IFSV_VIP_INT_HDL vip_handle;
   NCS_LOCK            *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   uns32 rc = NCSCC_RC_SUCCESS, last_msg_flag = FALSE;
   uns32 num_of_ckpts = 0,res;
   EDU_ERR ederror = 0;   
   uns8 *header = NULL, *sync_cnt_ptr = NULL, *ifsv_vip_rec_ptr = NULL, *last_msg = NULL;

   m_NCS_OS_MEMSET(&vip_handle, 0, sizeof(NCS_IFSV_VIP_INT_HDL));



   /* COLD_SYNC_RESP IS DONE BY THE ACTIVE */
   if(ifsv_cb->ha_state == SA_AMF_HA_STANDBY)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                      "Data Encode Failure, IfD is in STDBY mode",0);
      rc = NCSCC_RC_FAILURE;
      return rc;
   }      
   m_NCS_CONS_PRINTF("\nVIP ifd_mbcsv_enc_vip_data_resp");
   /*First reserve space to store the IfSv rec or Vip Rec that will be sent*/
   ifsv_vip_rec_ptr = ncs_enc_reserve_space(&arg->info.encode.io_uba, sizeof(uns32));
   if(ifsv_vip_rec_ptr == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                 "Data Encode Failure, ncs_enc_reserve_space returned NULL",0);
      m_NCS_CONS_PRINTF("Data Encode Failure, ncs_enc_reserve_space returned NULL for ifsv_vip_rec_ptr : ifd_mbcsv_enc_vip_data_resp");
      return NCSCC_RC_FAILURE;
   }
    
   ncs_enc_claim_space(&arg->info.encode.io_uba, sizeof(uns32));
   ncs_encode_32bit(&ifsv_vip_rec_ptr, IFD_DATA_RES_VIP_REC);

   /*First reserve space to store the IfSv rec or Vip Rec that will be sent*/
   last_msg = ncs_enc_reserve_space(&arg->info.encode.io_uba, sizeof(uns32));
   if(last_msg == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                 "Data Encode Failure, ncs_enc_reserve_space returned NULL",0);
      m_NCS_CONS_PRINTF("Data Encode Failure, ncs_enc_reserve_space returned NULL:last_msg:ifd_mbcsv_enc_vip_data_resp");
      return NCSCC_RC_FAILURE;
   }

   ncs_enc_claim_space(&arg->info.encode.io_uba, sizeof(uns32));

   /*First reserve space to store the number of checkpoints that will be sent*/
   header = ncs_enc_reserve_space(&arg->info.encode.io_uba, sizeof(uns32));
   if(header == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                 "Data Encode Failure, ncs_enc_reserve_space returned NULL",0);
      m_NCS_CONS_PRINTF("Data Encode Failure, ncs_enc_reserve_space returned NULL:header:ifd_mbcsv_enc_vip_data_resp");
      return NCSCC_RC_FAILURE;
   }

   ncs_enc_claim_space(&arg->info.encode.io_uba, sizeof(uns32));


   m_NCS_MEMCPY(&vip_handle, &ifsv_cb->record_handle, sizeof(vip_handle));

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
  
   p_vipd_rec = (IFSV_IFD_VIPD_RECORD *)ncs_patricia_tree_getnext(&ifsv_cb->vipDBase,
                                              (uns8*)&vip_handle);
   while ((p_vipd_rec != IFSV_NULL))
   {

      m_NCS_OS_MEMSET(&vip_redRec, 0, sizeof(VIP_REDUNDANCY_RECORD));

      res = ifd_populate_vip_redundancy_rec(p_vipd_rec, &vip_redRec);
      m_NCS_MEMCPY(&vip_handle, &p_vipd_rec->handle, sizeof(NCS_IFSV_VIP_INT_HDL));

      m_NCS_EDU_VER_EXEC(&ifsv_cb->edu_hdl, ifsv_edp_vip_chk_pt_full_rec, 
           &arg->info.encode.io_uba, EDP_OP_TYPE_ENC, &vip_redRec, 
           &ederror, ifsv_cb->edu_hdl.to_version);

      num_of_ckpts++; 

      if(num_of_ckpts == MAX_NO_IFD_MSGS_A2S)
      {
         break;       
      }

      p_vipd_rec = (IFSV_IFD_VIPD_RECORD *)ncs_patricia_tree_getnext(&ifsv_cb->vipDBase, (uns8*)&vip_handle);
   }/* while */

   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   m_NCS_MEMCPY(&ifsv_cb->record_handle, &vip_handle, sizeof(vip_handle));
 
   if(num_of_ckpts < MAX_NO_IFD_MSGS_A2S)
   {

     if(arg->info.encode.io_msg_type == NCS_MBCSV_MSG_COLD_SYNC_RESP)
        arg->info.encode.io_msg_type = NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE;
     else
        if(arg->info.encode.io_msg_type == NCS_MBCSV_MSG_DATA_RESP)
           arg->info.encode.io_msg_type = NCS_MBCSV_MSG_DATA_RESP_COMPLETE; 

      /* Now reset the vip handle for the next warm sync. */
      m_NCS_OS_MEMSET(&ifsv_cb->record_handle,'\0',sizeof(vip_handle));

      ifsv_cb->ifd_mbcsv_data_resp_enc_func_ptr = ifd_mbcsv_enc_data_resp;
      last_msg_flag = TRUE;
   } /* if(num_of_ckpts < MAX_NO_IFD_MSGS_A2S) */
   m_NCS_CONS_PRINTF("VIP : num_of_ckpts being set = %d\n",num_of_ckpts);
   ncs_encode_32bit(&header, num_of_ckpts);

   /* This will have the count of async updates that have been sent,
      this will be 0 initially*/
   sync_cnt_ptr = ncs_enc_reserve_space(&arg->info.encode.io_uba,sizeof(uns32));
   if(sync_cnt_ptr == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                 "Data Encode Failure, ncs_enc_reserve_space returned NULL",0);
      m_NCS_CONS_PRINTF("Data Encode Failure, ncs_enc_reserve_space returned NULL:sync_cnt_ptr:ifd_mbcsv_enc_vip_data_resp");
      /*TBD */
      return NCSCC_RC_FAILURE;
   }

   ncs_enc_claim_space(&arg->info.encode.io_uba, sizeof(uns32));
   ncs_encode_32bit(&sync_cnt_ptr, ifsv_cb->ifd_async_cnt);
   ncs_encode_32bit(&last_msg, last_msg_flag);
   m_NCS_CONS_PRINTF("VIP : ifsv_cb->ifd_async_cnt being set = %d\n",ifsv_cb->ifd_async_cnt);
   m_NCS_CONS_PRINTF("VIP : last_msg_flag being set = %d\n",last_msg_flag);

   m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Data Encode Success",0);

   return rc;
} /* function ifd_mbcsv_enc_vip_data_resp() ends here. */
#endif
/****************************************************************************
 * Name          : ifd_mbcsv_decode_proc
 *
 * Description   : This function decodes the async update, cold sync, warm 
 *                 sync and data response.
 *
 * Arguments     : arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_decode_proc(NCS_MBCSV_CB_ARG *arg)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   IFSV_CB *cb;
   uns32   ifsv_hdl;
   uns16 msg_fmt_version;

   msg_fmt_version = m_NCS_MBCSV_FMT_GET(arg->info.decode.i_peer_version,
                                         IFD_MBCSV_VERSION,
                                         IFD_MBCSV_VERSION_MIN); 
   if(0 == msg_fmt_version) 
   {
     m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
       "Decode fail.peer versn not supported",arg->info.decode.i_peer_version);
     return NCSCC_RC_FAILURE;
   }
   ifsv_hdl = m_IFD_CB_HDL_GET();

   cb = (IFSV_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, ifsv_hdl);

   if(cb == NULL)
   {
     m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                     "Decode Failure, NULL CB",0);
     return (NCSCC_RC_FAILURE);
   }
   
   /* embedding subslot changes */
   cb->edu_hdl.to_version = msg_fmt_version;

   switch(arg->info.decode.i_msg_type)
   {
     case NCS_MBCSV_MSG_ASYNC_UPDATE:
           ifd_mbcsv_dec_async_update(cb, arg);
           break;

     case NCS_MBCSV_MSG_COLD_SYNC_REQ:
            break; 

     case NCS_MBCSV_MSG_COLD_SYNC_RESP:
     case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
           ifd_mbcsv_dec_data_resp(cb,arg);
           break;
       
     case NCS_MBCSV_MSG_WARM_SYNC_REQ:
           break;

     case NCS_MBCSV_MSG_WARM_SYNC_RESP:
     case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
           ifd_mbcsv_dec_warm_sync_resp(cb,arg);
           break;

     case NCS_MBCSV_MSG_DATA_REQ:
           break;

     case NCS_MBCSV_MSG_DATA_RESP:
     case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
           ifd_mbcsv_dec_data_resp(cb,arg);
           break;

     default:
           return NCSCC_RC_FAILURE;
   } /* switch (arg->info.encode.io_msg_type)  */

   return rc;

} /* function ifd_mbcsv_decode_proc() ends here. */
/****************************************************************************
 * Name          : ifd_mbcsv_peer_info_cbk_handler
 *
 * Description   : This function checks whether the peer version is 
 *                 compatible or not 
 *
 * Arguments     : arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static
uns32  ifd_mbcsv_peer_info_cbk_handler(NCS_MBCSV_CB_ARG *arg)
{
   uns16 peer_version;

   peer_version= arg->info.peer.i_peer_version;
   if(peer_version < IFD_MBCSV_VERSION_MIN)
   {
    m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
          "peer version not supported",peer_version);
    return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : ifd_mbcsv_dec_async_update
 *
 * Description   : This function decodes the async update message.
 *
 * Arguments     : cb - Control block
 *                 arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_dec_async_update(IFSV_CB *cb, NCS_MBCSV_CB_ARG *arg)
{
   uns8* ptr, data[sizeof(uns32)];
   IFD_A2S_MSG  *ifd_msg;     
   uns32 evt_type, rc = NCSCC_RC_SUCCESS;
   EDU_ERR   ederror=0;  
 
   ifd_msg = m_MMGR_ALLOC_IFD_MBCSV_MSG;

   if(ifd_msg == NULL)
   {
      m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   /* To store the value of Async Update received */
   cb->ifd_async_cnt++;

   /* In the decode.i_uba, the 1st parameter is the type, so first decode only 
      the first field and based on the type then decode the entire message*/

   ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba,data,sizeof(uns32));

   if(ptr == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                 "Data Decode Failure, ncs_dec_flatten_space returned NULL",0);
      rc = NCSCC_RC_FAILURE;
      goto end;
   }

   evt_type = ncs_decode_32bit(&ptr);
   
   switch(evt_type)
   {
       case IFD_A2S_EVT_IFINDEX_SPT_MAP:
        {
          /* The contents are decoded from i_uba to ifd_msg */
          rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt, 
               &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, &ifd_msg, &ederror, cb->edu_hdl.to_version);

          if(rc != NCSCC_RC_SUCCESS)
          {
              m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"decode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_IFINDEX_SPT_MAP:ifd_mbcsv_dec_async_update",rc);
              m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Decode Failure, Spt Map",0);
              rc = NCSCC_RC_FAILURE;
              goto end;              
          }

          rc = ifd_a2s_ifindex_spt_map_handler(&ifd_msg->info.ifd_a2s_ifindex_spt_map_evt, cb);

          if(rc != NCSCC_RC_SUCCESS)
          {
             m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_ifindex_spt_map_handler():ifd_mbcsv_dec_async_update returned failure,rc:",rc);
            goto end;
          }
          break;
        } /* case IFD_A2S_EVT_IFINDEX_SPT_MAP: */

       case IFD_A2S_EVT_INTF_DATA:
        {
          /* The contents are decoded from i_uba to ifd_msg */
          rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt, 
               &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, &ifd_msg, &ederror, cb->edu_hdl.to_version);

          if(rc != NCSCC_RC_SUCCESS)
          {
              m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"decode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_INTF_DATA:ifd_mbcsv_dec_async_update",rc);
              m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Decode Failure, Intf Data",0);
              rc = NCSCC_RC_FAILURE;
              goto end;              
          }

          rc = ifd_a2s_intf_data_handler(&ifd_msg->info.ifd_a2s_intf_data_evt, cb);

          if(rc != NCSCC_RC_SUCCESS)
          {
             m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_intf_data_handler():ifd_mbcsv_dec_async_update returned failure,rc:",rc);
            goto end;
          }
          break;
        } /* case IFD_A2S_EVT_INTF_DATA:  */

       case IFD_A2S_EVT_SVC_DEST_UPD:
        {
          /* The contents are decoded from i_uba to ifd_msg */
          rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt, 
               &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, &ifd_msg, &ederror, cb->edu_hdl.to_version);

          if(rc != NCSCC_RC_SUCCESS)
          {
              m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"decode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_SVC_DEST_UPD:ifd_mbcsv_dec_async_update",rc);
              m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Decode Failure, SVCD Dest",0);
              rc = NCSCC_RC_FAILURE;
              goto end;              
          }

          rc = ifd_a2s_svc_dest_upd_handler(&ifd_msg->info.ifd_a2s_svc_dest_upd_evt, cb);

          if(rc != NCSCC_RC_SUCCESS)
          {
             m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_svc_dest_upd_handler():ifd_mbcsv_dec_async_update returned failure,rc:",rc);
            goto end;
          }
          break;
        } /* case IFD_A2S_EVT_SVC_DEST_UPD: */

        case IFD_A2S_EVT_IFND_UP_DOWN:
        {
          /* The contents are decoded from i_uba to ifd_msg */
          rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
               &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, &ifd_msg, &ederror, cb->edu_hdl.to_version);

          if(rc != NCSCC_RC_SUCCESS)
          {
              m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"decode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_IFND_UP_DOWN:ifd_mbcsv_dec_async_update",rc);
              m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                              "Decode Failure, IfND UP/DOWN",0);
              rc = NCSCC_RC_FAILURE;
              goto end;
          }

          rc = ifd_a2s_ifnd_up_down_info_handler(&ifd_msg->info.
                                              ifd_a2s_ifnd_up_down_evt, cb);

          if(rc != NCSCC_RC_SUCCESS)
          {
            m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_ifnd_up_down_info_handler():ifd_mbcsv_dec_async_update returned failure,rc:",rc);
            goto end;
          }
          break;
        } /* case IFD_A2S_EVT_IFND_DOWN */

        case IFD_A2S_EVT_IFINDEX_UPD:
        {
          /* The contents are decoded from i_uba to ifd_msg */
          rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
               &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, &ifd_msg, &ederror, cb->edu_hdl.to_version);

          if(rc != NCSCC_RC_SUCCESS)
          {
           m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"decode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_IFINDEX_UPD:ifd_mbcsv_dec_async_update",rc);
           m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Decode Failure, If Index UPD",0);
           rc = NCSCC_RC_FAILURE;
           goto end;
          }

          rc = ifd_a2s_ifindex_upd_handler(&ifd_msg->info.
                                              ifd_a2s_ifindex_upd_evt, cb);

          if(rc != NCSCC_RC_SUCCESS)
          {
            m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_ifindex_upd_handler():ifd_mbcsv_dec_async_update returned failure,rc:",rc);
            goto end;
          }
          break;
        } /* case IFD_A2S_EVT_IFINDEX_UPD */

#if(NCS_IFSV_IPXS == 1)    
       case IFD_A2S_EVT_IPXS_INTF_INFO:
        {
          /* The contents are decoded from i_uba to ifd_msg */
          rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
               &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, &ifd_msg, &ederror, cb->edu_hdl.to_version);

          if(rc != NCSCC_RC_SUCCESS)
          {
              m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"decode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_IPXS_INTF_INFO:ifd_mbcsv_dec_async_update",rc);
              m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Encode Failure, IPXS Update",0);
              rc = NCSCC_RC_FAILURE;
              goto end;
          }

          rc = ifd_a2s_ipxs_intf_info_handler(&ifd_msg->info.
                                              ifd_a2s_ipxs_intf_info_evt, cb);

          if(rc != NCSCC_RC_SUCCESS)
          {
            m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_ipxs_intf_info_handler():ifd_mbcsv_dec_async_update returned failure,rc:",rc);
            goto end;
          }
          break;
        } /* case IFD_A2S_EVT_IPXS_INTF_INFO */
#endif

#if(NCS_VIP == 1)
       case IFD_A2S_EVT_VIP_REC_INFO:
        {
          /* The contents are decoded from i_uba to ifd_msg */
          rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_msg_evt,
               &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, &ifd_msg, &ederror, cb->edu_hdl.to_version);

          if(rc != NCSCC_RC_SUCCESS)
          {
              m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"decode failure:ifsv_edp_ifd_a2s_msg_evt:IFD_A2S_EVT_VIP_REC_INFO:ifd_mbcsv_dec_async_update",rc);
              m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Encode Failure, VIP  Update",0);
              rc = NCSCC_RC_FAILURE;
              goto end;
          }

          rc = ifd_a2s_vip_rec_handler(&ifd_msg->info.
                                              ifd_a2s_vip_rec_info_evt, cb);

          if(rc != NCSCC_RC_SUCCESS)
          {
            m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_vip_rec_handler():ifd_mbcsv_dec_async_update returned failure,rc:",rc);
            goto end;
          }
          break;
        } /* case IFD_A2S_EVT_IPXS_INTF_INFO */
#endif

       default:
        rc = NCSCC_RC_FAILURE;
        m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,\
                           "Decode Failure, Unkown msg type : ",\
                           arg->info.encode.io_reo_type);
        goto end;

   }/* switch(evt_type) */

end: 
    m_MMGR_FREE_IFD_MBCSV_MSG(ifd_msg);  
    return rc;

} /* function ifd_mbcsv_dec_async_update() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_dec_warm_sync_resp
 *
 * Description   : This function decodes the warm sync resp message.
 *
 * Arguments     : cb - Control block
 *                 arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_dec_warm_sync_resp(IFSV_CB *cb, NCS_MBCSV_CB_ARG *arg)
{
    uns32 num_of_async_upd, rc = NCSCC_RC_SUCCESS, start_cold_sync = 0;
    uns8 data[16],*ptr;
    NCS_MBCSV_ARG ncs_arg;
   uns32 act_num_free_ifindex = 0, act_num_max_ifindex = 0;
   uns32 stdby_num_free_ifindex = 0, stdby_num_max_ifindex = 0, stdby_num_ifindex_present = 0;

    m_NCS_OS_MEMSET(&ncs_arg,'\0',sizeof(NCS_MBCSV_ARG));
       
    ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba,data,sizeof(int32));
    num_of_async_upd = ncs_decode_32bit(&ptr);
    ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));

    ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba,data,sizeof(int32));
    act_num_max_ifindex = ncs_decode_32bit(&ptr);
    ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));

    ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba,data,sizeof(int32));
    act_num_free_ifindex = ncs_decode_32bit(&ptr);
    ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));

    ifd_ifap_max_num_ifindex_and_num_free_ifindex_get(&stdby_num_max_ifindex, &stdby_num_free_ifindex);

    stdby_num_ifindex_present = cb->if_map_tbl.n_nodes;

    if(cb->ifd_async_cnt == num_of_async_upd)
    {

     if(stdby_num_ifindex_present != (stdby_num_max_ifindex - stdby_num_free_ifindex))
     {
       m_NCS_CONS_PRINTF("Error :DB Mismatch on STDBY : stdby_num_max_ifindex %d, stdby_num_free_ifindex %d, stdby_num_ifindex_present %d\n",stdby_num_max_ifindex, stdby_num_free_ifindex, stdby_num_ifindex_present);
       m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Error :DB Mismatch on STDBY : stdby_num_max_ifindex, stdby_num_free_ifindex, stdby_num_ifindex_present,stdby_num_of_async_cnt,act_num_max_ifindex,act_num_free_ifindex,act_num_of_async_upd",stdby_num_max_ifindex, stdby_num_free_ifindex, stdby_num_ifindex_present,cb->ifd_async_cnt,act_num_max_ifindex,act_num_free_ifindex,num_of_async_upd);
       start_cold_sync = TRUE;
     }
     else
     {

      /* Check the parammeters. */
      if((act_num_max_ifindex != stdby_num_max_ifindex) ||
         (act_num_free_ifindex != stdby_num_free_ifindex))
        {
         /* There is a problem, start the data base sync again. */
          m_NCS_CONS_PRINTF("Mismatch in Act and Stdby DB : ifd_mbcsv_dec_warm_sync_resp: act_num_max_ifindex = %d,stdby_num_max_ifindex = %d,act_num_free_ifindex = %d,stdby_num_free_ifindex = %d\n",act_num_max_ifindex,stdby_num_max_ifindex,act_num_free_ifindex,stdby_num_free_ifindex);
          m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Error : Mismatch in Act and Stdby DB : ifd_mbcsv_dec_warm_sync_resp: act_num_max_ifindex,stdby_num_max_ifindex,act_num_free_ifindex,stdby_num_free_ifindex,stdby_num_ifindex_present,act_num_of_async_upd,stdby_num_of_async_cnt",act_num_max_ifindex,stdby_num_max_ifindex,act_num_free_ifindex,stdby_num_free_ifindex,stdby_num_ifindex_present,num_of_async_upd,cb->ifd_async_cnt);
          start_cold_sync = TRUE;
        }
     }
   }
   else
   {
     start_cold_sync = TRUE;
     m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Error : Mismatch between act_num_of_async_upd and stdby_num_of_async_cnt : act_num_of_async_upd,stdby_num_of_async_cnt,stdby_num_max_ifindex, stdby_num_free_ifindex, stdby_num_ifindex_present,act_num_max_ifindex,act_num_free_ifindex",num_of_async_upd,cb->ifd_async_cnt,stdby_num_max_ifindex, stdby_num_free_ifindex, stdby_num_ifindex_present,act_num_max_ifindex,act_num_free_ifindex);
     m_NCS_CONS_PRINTF("cb->ifd_async_cnt and num_of_async_upd mismatch\n");
   }

    if(start_cold_sync == TRUE)
    {
       ifd_all_intf_rec_del(cb); 
       ifsv_ifindex_spt_map_del_all(cb);
       /* Clear the IAPS data base. */
       m_IFD_IFAP_DESTROY(NULL);
#if (NCS_VIP == 1)
       ifd_all_vip_rec_del(cb);
#endif
      /* Initialize the IAPS data base for fresh If index alloc. */
       m_IFD_IFAP_INIT(NULL);



#if (NCS_IFSV_IPXS == 1)
#endif
       /* Mark the flag on as MBCSV will start data download. */
       cb->cold_or_warm_sync_on = TRUE;
       m_IFSV_LOG_HEAD_LINE(cb->my_svc_id,IFSV_WARM_COLD_SYNC_START, 0, 0);
       ncs_arg.i_op = NCS_MBCSV_OP_SEND_DATA_REQ;
       ncs_arg.i_mbcsv_hdl =  cb->mbcsv_hdl;
       ncs_arg.info.send_data_req.i_ckpt_hdl = cb->o_ckpt_hdl;
       rc = ncs_mbcsv_svc(&ncs_arg);
       if(rc != NCSCC_RC_SUCCESS)
       {
          /* Log */
          m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncs_mbcsv_svc():ifd_mbcsv_dec_warm_sync_resp returned failure,rc:",rc);
          return rc;
       }

       return rc;
    }
return rc;
} /* function ifd_mbcsv_dec_warm_sync_resp() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_dec_data_resp
 *
 * Description   : This function decodes the data resp message.
 *
 * Arguments     : cb - Control block
 *                 arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32  ifd_mbcsv_dec_data_resp(void *ifsv_cb, NCS_MBCSV_CB_ARG *arg)
{
   uns8 *ptr, data[sizeof(uns32)];
   uns32 num_of_ckpts = 0;
   uns32 count=0, rc = NCSCC_RC_SUCCESS, num_of_async_upd = 0, ifsv_vip_rec = 0, last_msg = 0;
   IFSV_INTF_DATA *ckpt_data_intf = NULL;
   NCS_IPXS_IPINFO *ckpt_data_ipxs = NULL;
   EDU_ERR   ederror = 0;
   IFAP_INFO_LIST_A2S *ifap_info;
   IFSV_CB *cb = (IFSV_CB*)ifsv_cb;
   
   if(arg->info.decode.i_uba.ub == NULL) /* There is no data */
   {
      return NCSCC_RC_SUCCESS;
   }

   /* Get the async update count */
   ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba,data,sizeof(uns32));
   ifsv_vip_rec = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));

   if(ifsv_vip_rec != IFD_DATA_RES_IFSV_REC)
    {
     m_NCS_CONS_PRINTF("Not a IFSV Rec\n"); 
     return NCSCC_RC_FAILURE;
    }

   /* Get the async update count */
   ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba,data,sizeof(uns32));
   last_msg = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));

   /* Allocate the memory for INTF DATA */
   ckpt_data_intf = m_MMGR_ALLOC_INFTDATA;

   if(ckpt_data_intf == NULL)
   {
       m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
       rc = NCSCC_RC_FAILURE;
       m_NCS_CONS_PRINTF("Mem Alloc Failure :ckpt_data_intf:ifd_mbcsv_dec_data_resp\n"); 
       return rc;
   }

   /* Allocate the memory for IPXS */
   ckpt_data_ipxs = m_MMGR_ALLOC_IPXSDATA;

   if(ckpt_data_ipxs == NULL)
   {
       m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
       rc = NCSCC_RC_FAILURE;
       m_MMGR_FREE_INFTDATA(ckpt_data_intf); 
       m_NCS_CONS_PRINTF("Mem Alloc Failure :ckpt_data_ipxs:ifd_mbcsv_dec_data_resp\n"); 
       return rc;
   }

   /* Allocate the memory for IPXS */
   ifap_info = m_MMGR_ALLOC_IFAP_INFO_LIST;

   if(ifap_info == NULL)
   {
       m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
       rc = NCSCC_RC_FAILURE;
       m_MMGR_FREE_INFTDATA(ckpt_data_intf);
       m_MMGR_FREE_IPXSDATA(ckpt_data_ipxs);
       m_NCS_CONS_PRINTF("Mem Alloc Failure :ifap_info:ifd_mbcsv_dec_data_resp\n"); 
       return rc;
   }

   m_NCS_OS_MEMSET(ifap_info,'\0',sizeof(IFAP_INFO_LIST_A2S));

   /* 1. Decode the 1st uns8 region ,  we will get the num of ckpts*/
   ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba, data,sizeof(uns32));
   num_of_ckpts = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));
   m_NCS_CONS_PRINTF("num_of_ckpts rcved = %d\n",num_of_ckpts); 
   if((arg->info.decode.i_msg_type == NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE) ||
      (arg->info.decode.i_msg_type == NCS_MBCSV_MSG_DATA_RESP_COMPLETE) ||
      (last_msg == TRUE))
   {
     /* If this is the last message then the last record will be IAPS data. */
     num_of_ckpts = num_of_ckpts - 1;
#if (NCS_VIP == 1)
      cb->ifd_mbcsv_data_resp_dec_func_ptr = ifd_mbcsv_dec_vip_data_resp;
#endif
   }

   /* Decode the data */

   while(count < num_of_ckpts)
   {
      m_NCS_MEMSET(ckpt_data_intf, 0, sizeof(IFSV_INTF_DATA));
#if(NCS_IFSV_IPXS == 1) 
      m_NCS_MEMSET(ckpt_data_ipxs, 0, sizeof(NCS_IPXS_IPINFO));
#endif 

      rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_intf_data, 
           &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, &ckpt_data_intf,&ederror , cb->edu_hdl.to_version);

      if(rc != NCSCC_RC_SUCCESS)
      {
        m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"Decode Failure, Data Rsp : ifsv_edp_intf_data:ifd_mbcsv_dec_data_resp,rc:",rc);
        m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Decode Failure, Data Rsp",0);
        m_NCS_CONS_PRINTF("Decode Failure, Data Rsp : ifsv_edp_intf_data:ifd_mbcsv_dec_data_resp");
        m_MMGR_FREE_INFTDATA(ckpt_data_intf);
        m_MMGR_FREE_IPXSDATA(ckpt_data_ipxs);
        m_MMGR_FREE_IFAP_INFO_LIST(ifap_info);
        return rc;
      }

#if(NCS_IFSV_IPXS == 1)    
      rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ipxs_edp_ifip_info, 
           &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, 
           &ckpt_data_ipxs, &ederror, cb->edu_hdl.to_version);

      if(rc != NCSCC_RC_SUCCESS)
      {
        m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"Decode Failure, Data Rsp : ipxs_edp_ifip_info:ifd_mbcsv_dec_data_resp,rc:",rc);
        m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Decode Failure, Data Rsp",0);
        m_NCS_CONS_PRINTF("Decode Failure, Data Rsp : ipxs_edp_ifip_info:ifd_mbcsv_dec_data_resp");
        m_MMGR_FREE_INFTDATA(ckpt_data_intf);
        m_MMGR_FREE_IPXSDATA(ckpt_data_ipxs);
        m_MMGR_FREE_IFAP_INFO_LIST(ifap_info);
         return rc;
      }
#endif
      ifd_a2s_sync_resp(cb, (uns8 *)ckpt_data_intf
#if(NCS_IFSV_IPXS == 1)    
, (uns8 *)ckpt_data_ipxs
#endif
);
      count++;   

   }  
   m_NCS_CONS_PRINTF("last_msg rcved = %d\n",last_msg); 
   if((arg->info.decode.i_msg_type == NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE) ||
      (arg->info.decode.i_msg_type == NCS_MBCSV_MSG_DATA_RESP_COMPLETE) ||
      (last_msg == TRUE))
   {
      /* Now decode IAPS data. */
      rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifd_a2s_iaps_evt, 
           &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, &ifap_info, &ederror, cb->edu_hdl.to_version);

     m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Received : num of ckpts, Max ifindex, Num free index",num_of_ckpts +1, ifap_info->max_ifindex, ifap_info->num_free_ifindex,0,0,0,0);
      /* Process the IAPS data. */
      ifd_a2s_iaps_sync_resp(cb, ifap_info);
   }

   /* Get the async update count */
   ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba,data,sizeof(uns32));
   num_of_async_upd = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));
   cb->ifd_async_cnt = num_of_async_upd;   
   m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Received : Async count",num_of_async_upd,0,0,0,0,0,0);
   m_NCS_CONS_PRINTF("num_of_async_upd rcved = %d\n",num_of_async_upd);
#if (NCS_VIP != 1)   
   if((arg->info.decode.i_msg_type == NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE) ||
      (arg->info.decode.i_msg_type == NCS_MBCSV_MSG_DATA_RESP_COMPLETE) ||
      (last_msg == TRUE))
   {
      cb->cold_or_warm_sync_on = FALSE;
      m_NCS_CONS_PRINTF("Cold Sync Compl: ifd_mbcsv_dec_data_resp()\n");
   }
#endif
    m_MMGR_FREE_INFTDATA(ckpt_data_intf);
    m_MMGR_FREE_IPXSDATA(ckpt_data_ipxs);
    m_MMGR_FREE_IFAP_INFO_LIST(ifap_info);

    m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Decode Success, Data Rsp",0);
    ifd_mbcsv_dec_vip_data_resp(ifsv_cb,arg);

   return rc;

} /* function ifd_mbcsv_dec_data_resp() ends here. */

#if (NCS_VIP == 1)
/****************************************************************************
 * Name          : ifd_mbcsv_dec_vip_data_resp
 *
 * Description   : This function decodes the VIP rec message.
 *
 * Arguments     : cb - Control block
 *                 arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_dec_vip_data_resp(void *ifsv_cb, NCS_MBCSV_CB_ARG *arg)
{
   uns8 *ptr, data[sizeof(uns32)];
   uns32 num_of_ckpts=0;
   uns32 count=0, rc = NCSCC_RC_SUCCESS, num_of_async_upd = 0, ifsv_vip_rec = 0, last_msg = 0;
   VIP_REDUNDANCY_RECORD *ckpt_data_vip = NULL;
   EDU_ERR   ederror = 0;
   IFSV_CB *cb = (IFSV_CB *)ifsv_cb;
   SaAisErrorT amf_error = SA_AIS_OK;
   
   if(arg->info.decode.i_uba.ub == NULL) /* There is no data */
   {
      return NCSCC_RC_SUCCESS;
   }

   /* Get the async update count */
   ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba,data,sizeof(uns32));
   ifsv_vip_rec = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));

   if(ifsv_vip_rec != IFD_DATA_RES_VIP_REC)
   {
     m_NCS_CONS_PRINTF("ifsv_vip_rec  = %d is not of type IFD_DATA_RES_VIP_REC\n",ifsv_vip_rec);
     return NCSCC_RC_FAILURE;
   }

   /* Get the async update count */
   ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba,data,sizeof(uns32));
   last_msg = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));

   /* Allocate the memory for VIP CheckPoint Data in ifsv_vip_mem.h TBD:: ACHYUT*/
   ckpt_data_vip = m_MMGR_ALLOC_IFSV_VIP_REDUNDANCY_REC;

   if(ckpt_data_vip == NULL)
   {
       m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
       m_NCS_CONS_PRINTF("Mem fail:ckpt_data_vip:ifd_mbcsv_dec_vip_data_resp\n");
       rc = NCSCC_RC_FAILURE;
       return rc;
   }

   /* 1. Decode the 1st uns8 region ,  we will get the num of ckpts*/
   ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba, data,sizeof(uns32));
   num_of_ckpts = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));
   m_NCS_CONS_PRINTF("VIP:num_of_ckpts rcved = %d\n",num_of_ckpts); 
   /* Decode the data */

   while(count < num_of_ckpts)
   {
      m_NCS_MEMSET(ckpt_data_vip, 0, sizeof(VIP_REDUNDANCY_RECORD));

      rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_vip_chk_pt_full_rec, 
           &arg->info.decode.i_uba, EDP_OP_TYPE_DEC, &ckpt_data_vip,&ederror, cb->edu_hdl.to_version);

      if(rc != NCSCC_RC_SUCCESS)
      {
        m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"Decode Failure, Data Rsp :ifsv_edp_vip_chk_pt_full_rec:ifd_mbcsv_dec_vip_data_resp,rc:",rc);
        m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Decode Failure, Data Rsp",0);
        m_NCS_CONS_PRINTF("Decode Failure, Data Rsp:ifsv_edp_vip_chk_pt_full_rec:ifd_mbcsv_dec_vip_data_resp\n");
        m_MMGR_FREE_INFTDATA(ckpt_data_vip);
        return rc;
      }
      /*
       Note: Build the VIP Database from the Obtained VIP records 
       */

      ifd_a2s_vip_rec_create_handler (ckpt_data_vip, cb);
      count++;   

   }  

   /* Get the async update count */
   ptr = ncs_dec_flatten_space(&arg->info.decode.i_uba,data,sizeof(uns32));
   num_of_async_upd = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&arg->info.decode.i_uba,sizeof(uns32));
   cb->ifd_async_cnt = num_of_async_upd;   
   m_NCS_CONS_PRINTF("VIP:num_of_async_upd rcved = %d\n",num_of_async_upd); 
   m_NCS_CONS_PRINTF("VIP:last_msg rcved = %d\n",last_msg); 
   
   if((arg->info.decode.i_msg_type == NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE) ||
      (arg->info.decode.i_msg_type == NCS_MBCSV_MSG_DATA_RESP_COMPLETE) ||
      (last_msg == TRUE))
   {
      cb->cold_or_warm_sync_on = FALSE;
      m_IFSV_LOG_HEAD_LINE(cb->my_svc_id,IFSV_WARM_COLD_SYNC_STOP, 0, 0);
      m_NCS_CONS_PRINTF("Cold sync compl : ifd_mbcsv_dec_vip_data_resp\n");
      cb->ifd_mbcsv_data_resp_dec_func_ptr = ifd_mbcsv_dec_data_resp;
      /* Check whether we need to send AMF resp here or not. */
      if(cb->amf_req_in_cold_syn.amf_req == TRUE)
      {
        /* AMF Req had come while cold syn was on, so Send AMF Resp */
          m_IFSV_LOG_HEAD_LINE(cb->my_svc_id,IFSV_AMF_RESP_AFTER_COLD_SYNC, 0, 0);
          saAmfResponse(cb->amf_hdl,cb->amf_req_in_cold_syn.invocation, amf_error);
          /* Now reset the amf_req_in_cold_syn structure. */
          cb->amf_req_in_cold_syn.amf_req = FALSE;
          cb->amf_req_in_cold_syn.invocation = 0;
      }
   }

    ckpt_data_vip = m_MMGR_ALLOC_IFSV_VIP_REDUNDANCY_REC;

    m_IFD_LOG_EVT_L(IFSV_LOG_MBCSV_MSG,"Decode Success, Data Rsp",0);

   return rc;

} /* function ifd_mbcsv_dec_vip_data_resp() ends here. */

/****************************************************************************
 * Name          : ifd_mbcsv_db_ifrec_sptmap_mismatch_correction
 *
 * Description   : This function does data base correction.
 *                 Why it is needed ? : Since intf rec creation is divided into
 *                 two parts from IfND to IFD, one is allocation of intf index and  
 *                 another is intf rec addition. If IfND has sent intf rec allocation
 *                 and IfND is killed/blade is down and couldn't sent intf rec addition.
 *                 In this case, ACT IfD has allocated ifindex and SPT map record but 
 *                 doesn't have intf record and so will be STDBY IFD. So, in warm sync
 *                 enc, IFD should correct its database. It should delete intf index
 *                 allocated and SPT map for which, it doesn't have intf records.    
 *                  
 * Arguments     : cb - Control block
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_db_ifrec_sptmap_mismatch_correction(IFSV_CB *ifsv_cb)
{
 NCS_IFSV_SPT_MAP    spt_map;
 NCS_IFSV_SPT        spt;
 NCS_PATRICIA_NODE   *if_node = NULL;
 IFSV_SPT_REC        *spt_info = NULL;
 IFSV_INTF_DATA *rec_data = NULL;
 uns32 res = NCSCC_RC_SUCCESS;
 uns32 ifindex = 0;

 m_NCS_MEMSET(&spt, 0, sizeof(NCS_IFSV_SPT));
 m_NCS_MEMSET(&spt_map, 0, sizeof(NCS_IFSV_SPT_MAP));
 
 if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_map_tbl, (uns8*)&spt);
 spt_info = (IFSV_SPT_REC*)if_node;  
 while(NULL != spt_info)
 {
   /* This intf rec corresponding to this ifindex should be available. */
      m_NCS_MEMCPY(&spt, &spt_info->spt_map.spt, sizeof(NCS_IFSV_SPT));
      ifindex = spt_info->spt_map.if_index;
      rec_data = ifsv_intf_rec_find (ifindex, ifsv_cb); 
      if(rec_data == NULL)
      {
         /* This means that the corresponding intf rec is not there.
          * So, delete the intf index and SPT map and inform STDBY IFD
          * for the same. */
          m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Intf Rec missing for s/s/ss/p/t/s/index",spt.shelf,spt.slot,spt.subslot,spt.port,spt.type,spt.subscr_scope,ifindex);

         /* Delete the slot port maping */
         res = ifsv_ifindex_spt_map_del(spt_info->spt_map.spt, ifsv_cb);

         if(res == NCSCC_RC_SUCCESS)
         {
           spt_map.if_index = ifindex;
           res = ifsv_ifindex_free(ifsv_cb, &spt_map);
         }
      } /* if(rec_data == NULL) */
      if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_map_tbl, (uns8*)&spt);
      spt_info = (IFSV_SPT_REC*)if_node;
 } /* while(NULL != spt_info) */

 return NCSCC_RC_SUCCESS;

}
/****************************************************************************
 * Name          : ifd_mbcsv_db_mismatch_correction
 *
 * Description   : This function does data base correction.
 *                 Why it is needed ? : Since STDBY IFD gets all the intf rec 
 *                 based on intf rec from IfD, if ACT IFD doesn't have  
 *                 intf rec corresponding to SPT MAP [see ifd_mbcsv_db_ifrec_sptmap_mismatch_correction] 
 *                 then STDBY IFD will not get intf rec and SPT map, but it will get
 *                 Max_ifindex incremented in IAPS data. So, before warm sync if somebody 
 *                 does a switchover then this function will correct the data. It will free
 *                 the ifindex that is not available in SPT and free list.
 * Arguments     : cb - Control block, max number of ifindex allocated.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_mbcsv_db_mismatch_correction(IFSV_CB *ifsv_cb, uns32 num_max_ifindex)
{
 NCS_IFSV_SPT_MAP     spt_map;
 uns32 res =          NCSCC_RC_SUCCESS;
 uns32 ifindex = 0;

 m_NCS_MEMSET(&spt_map, 0, sizeof(NCS_IFSV_SPT_MAP));

 for(ifindex = 1; ifindex <= num_max_ifindex; ifindex++) 
 {
  if((res = ifd_find_index_from_sptmap(ifindex,ifsv_cb)) == NCSCC_RC_FAILURE) 
  {
    /* Intf Rec is not present. So, this should be in free list.
       If this is not in the free list then we will put this in 
       the free list.*/
       if(ifd_ifap_ifindex_free_find(ifindex) == NCSCC_RC_FAILURE)
       {
         spt_map.if_index = ifindex;

         /* This ifindex is not in the free list, so put it in the free list. */ 
          m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Intf Index not having Intf Record",ifindex,0,0,0,0,0,0);
          res = ifsv_ifindex_free(ifsv_cb, &spt_map);
      }
  } /* f(rec_data) */
 }/* for */

 return NCSCC_RC_SUCCESS;

}

/****************************************************************************
 * Name          : ifd_find_index_from_sptmap
 *
 * Description   : This function checks whether any SPTMAP is with given ifindex.
 * Arguments     : intf index 
 *                 cb - Control block, max number of ifindex allocated.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32  ifd_find_index_from_sptmap(uns32 ifindex,IFSV_CB *cb)
{
   NCS_IFSV_SPT        spt;
   NCS_PATRICIA_NODE   *if_node;
   IFSV_SPT_REC        *spt_info;

   m_NCS_MEMSET(&spt, 0, sizeof(NCS_IFSV_SPT));

   if_node = ncs_patricia_tree_getnext(&cb->if_map_tbl, (uns8*)&spt);
   spt_info = (IFSV_SPT_REC*)if_node;
   while (spt_info != NULL)
   {      
      m_NCS_MEMCPY(&spt, &spt_info->spt_map.spt, 
        sizeof(NCS_IFSV_SPT));
      if(ifindex == spt_info->spt_map.if_index)
      {
        return NCSCC_RC_SUCCESS;
      }
      if_node = ncs_patricia_tree_getnext(&cb->if_map_tbl, (uns8*)&spt);
      spt_info = (IFSV_SPT_REC*)if_node;
   }
 return NCSCC_RC_FAILURE;
}

/****************************************************************************
 * Name          : ifd_a2s_sync_vip_rec_resp
 *
 * Description   : This function decodes the VIP rec message.
 *
 * Arguments     : cb - Control block
 *                 arg - Pointer to the structure containing information
 *                 pertaining to the callback being dispatched by MBCSV.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
#endif
/*********************  file ends here ************************************/
