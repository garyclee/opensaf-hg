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
FILE NAME: ifd_red.C
DESCRIPTION: Ifd redundancy routines.
******************************************************************************/

#include "ifsv.h"
#include "ifd_red.h"
#include "ifsvifap.h"
/*****************************************************************************
 *                  Functions prototypes 
 *****************************************************************************/
extern uns32 ifd_a2s_iaps_sync_resp (IFSV_CB *cb, IFAP_INFO_LIST_A2S *msg);
extern uns32 ifd_a2s_ifindex_spt_map_handler(IFD_A2S_IFINDEX_SPT_MAP_EVT *info,
                                             IFSV_CB *cb);
static uns32 ifd_a2s_ifindex_spt_map_create_handler 
                  (NCS_IFSV_SPT_MAP * evt, IFSV_CB *cb);
static uns32 ifd_a2s_ifindex_spt_map_delete_handler 
                  (NCS_IFSV_SPT_MAP * evt, IFSV_CB *cb);
extern uns32 ifd_a2s_intf_data_handler (IFD_A2S_INTF_DATA_EVT *info, 
                                        IFSV_CB *cb);
static uns32 ifd_a2s_intf_data_create_handler (IFSV_INTF_DATA *evt, 
                                               IFSV_CB *cb, uns32 oper_status);
static uns32 ifd_a2s_intf_data_modify_handler 
                  (IFSV_INTF_DATA * evt, IFSV_CB *cb);
static uns32 ifd_a2s_intf_data_marked_delete_handler 
                  (IFSV_INTF_DATA * evt, IFSV_CB *cb);
static uns32 ifd_a2s_intf_data_delete_handler 
                  (IFSV_INTF_DATA * evt, IFSV_CB *cb);
extern uns32 ifd_a2s_svc_dest_upd_handler(IFD_A2S_SVC_DEST_UPD_EVT *info, 
                                          IFSV_CB *cb);
#if(NCS_IFSV_IPXS == 1)
extern uns32 ifd_a2s_ipxs_intf_info_handler(IFD_A2S_IPXS_INTF_INFO_EVT *info, 
                                            IFSV_CB *cb);
extern uns32 ifd_ipxs_proc_data_ifip_info(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                          NCS_IFSV_IFINDEX *if_index);
#endif
extern uns32 ifd_a2s_ifnd_up_down_info_handler(IFD_A2S_IFND_UP_DOWN_EVT *info, 
                                            IFSV_CB *cb);
extern uns32 ifd_a2s_ifindex_upd_handler(IFD_A2S_IFINDEX_UPD_EVT *info,
                                  IFSV_CB *cb);
extern uns32 ifd_ifnd_node_id_info_del (NODE_ID node_id, IFSV_CB *cb);
extern IFND_NODE_ID_INFO *ifd_ifnd_node_id_info_get (NODE_ID node_id, IFSV_CB *cb);
extern uns32 ifd_ifnd_node_id_info_destroy_all (IFSV_CB *cb);
extern uns32 ifd_ifnd_node_id_info_add (IFD_IFND_NODE_ID_INFO_REC **rec, NODE_ID *node_id, IFSV_CB *cb);
static void ifd_stdby_error_handler (IFD_STDBY_ERROR_TYPE error);
extern uns32 ifsv_tmr_start (IFSV_TMR *tmr, IFSV_CB *cb);
extern void  ifsv_tmr_stop (IFSV_TMR *tmr,IFSV_CB *cb);
extern uns32 ifd_svcd_upd_from_ifnd_proc_data (IFSV_EVT* evt, IFSV_CB *cb);


/****************************************************************************
 * Name          : ifd_a2s_ifindex_spt_map_handler 
 *
 * Description   : This function is responsible for handling the event related 
 *                 to shelf/slot/port/type/scope to interface index. Calls the
 *                 corresponding handling function as per evt type.
 *
 * Arguments     : info  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifd_a2s_ifindex_spt_map_handler (IFD_A2S_IFINDEX_SPT_MAP_EVT *info, IFSV_CB *cb)
{
  uns32 res = NCSCC_RC_FAILURE;
  
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,\
                      "IfIndex Spt Map Update. Type is :",\
                      info->type);

  switch (info->type)
  {
    case IFD_A2S_IFINDEX_SPT_MAP_EVT_CREATE:
    {
      if(cb->cold_or_warm_sync_on == FALSE)
      {
         res = ifd_a2s_ifindex_spt_map_create_handler(&info->info, cb);
      }
      else
      {
        m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,\
        "Cann't add spt_map. Cold or warm sync is going on. Index : ",\
               info->info.if_index);
      }
      break;
    }
    case IFD_A2S_IFINDEX_SPT_MAP_EVT_DELETE:
    { 
      res = ifd_a2s_ifindex_spt_map_delete_handler 
                      (&info->info, cb);
     break;
    }
   default:
    break;
  } /* info->type */
  return (res);
} /* function ifd_a2s_ifindex_spt_map_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_ifindex_spt_map_create_handler 
 *
 * Description   : This function adds shelf/slot/port/type/scope to interface
 *                 index mapping to the standby IfD data base. 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifd_a2s_ifindex_spt_map_create_handler (NCS_IFSV_SPT_MAP *evt, 
                                          IFSV_CB *cb)
{  
   uns32 res = NCSCC_RC_FAILURE;
   /* Create the memory */
   res = ifsv_ifindex_spt_map_add (evt, cb);
   
   if (res != NCSCC_RC_SUCCESS)
   {
    m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifindex_spt_map_add():ifd_a2s_ifindex_spt_map_create_handler returned failure"," ");
    ifd_stdby_error_handler (IFD_STDBY_IFINDEX_SPT_MAP_ADD_FAILURE);
   }

   return (res);

} /* function ifd_a2s_ifindex_spt_map_create_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_ifindex_spt_map_delete_handler 
 *
 * Description   : This function deletes shelf/slot/port/type/scope to 
 *                 interface index mapping from the standby IfD data base. 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifd_a2s_ifindex_spt_map_delete_handler (NCS_IFSV_SPT_MAP *evt,
                                          IFSV_CB *cb)
{  
   uns32 res = NCSCC_RC_FAILURE;
   
   res = ifsv_ifindex_spt_map_del (evt->spt, cb);
   
   if (res != NCSCC_RC_SUCCESS)
    {
    m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifindex_spt_map_del():ifd_a2s_ifindex_spt_map_delete_handler returned failure"," ");
    ifd_stdby_error_handler (IFD_STDBY_IFINDEX_SPT_MAP_DELETE_FAILURE);
    }
   return (res);
} /* function ifd_a2s_ifindex_spt_map_delete_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_intf_data_handler
 *
 * Description   :  This function is responsible for handling the event 
 *                  related to interface data record. Calls the corresponding 
 *                  handling function as per evt type.
 *
 * Arguments     : info  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifd_a2s_intf_data_handler (IFD_A2S_INTF_DATA_EVT *info, IFSV_CB *cb)
{  
   uns32 res = NCSCC_RC_FAILURE;
   m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,\
                   "Interface Data Update. Type is :",\
                   info->type);
   switch (info->type)
   {
   case IFD_A2S_INTF_DATA_EVT_CREATE:
   {
      if(cb->cold_or_warm_sync_on == FALSE)
      {
         res = ifd_a2s_intf_data_create_handler (&info->info, cb, TRUE);
      }
      else
      {  
        m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,\
        "Cann't add intf_data. Cold or warm sync is going on. Index : ",\
               info->info.if_index);
      }
    break;
   }
   case IFD_A2S_INTF_DATA_EVT_MODIFY:
   {
     res = ifd_a2s_intf_data_modify_handler (&info->info, cb);
     break;
   }
   case IFD_A2S_INTF_DATA_EVT_MARKED_DELETE:
   {     
    res = ifd_a2s_intf_data_marked_delete_handler (&info->info, cb);
    break;
   }
   case IFD_A2S_INTF_DATA_EVT_DELETE:
   {
    res = ifd_a2s_intf_data_delete_handler (&info->info, cb);
    break;
   }
   default:
    break;
   } /* info->type */

  return (res);

} /* function ifd_a2s_intf_data_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_intf_data_create_handler 
 *
 * Description   : This function adds the interface data record, newly created 
 *                 on the active IfD, into its data record. 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifd_a2s_intf_data_create_handler (IFSV_INTF_DATA * evt, IFSV_CB *cb, uns32 oper_status)
{  
   uns32 res = NCSCC_RC_SUCCESS;;
   NCS_PATRICIA_TREE *p_tree   = &cb->if_tbl;
   IFSV_INTF_REC     *rec;
   uns32 ifindex = 0;
   IFSV_INTF_DATA     *rec_data;
   NCS_LOCK  *intf_rec_lock = &cb->intf_rec_lock;

   /* Always check wether the interface record exists or not. */
   if ((ifsv_get_ifindex_from_spt(&ifindex, evt->spt_info, cb) ==
          NCSCC_RC_SUCCESS) && ((rec_data = ifsv_intf_rec_find(ifindex, cb))
          != IFSV_NULL))
    {
      return NCSCC_RC_FAILURE;
    }
 
   if ((rec = m_MMGR_ALLOC_IFSV_INTF_REC) == IFSV_NULL)
   {
     m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
     return (NCSCC_RC_FAILURE);
   }
   
   m_NCS_MEMSET(rec, 0, sizeof(IFSV_INTF_REC));
   memcpy(&rec->intf_data, evt, sizeof(IFSV_INTF_DATA));
   if(oper_status == TRUE)
     rec->intf_data.active = TRUE;
   else
     rec->intf_data.active = FALSE;
   /* While adding the record, set all the attributes to TRUE */
   m_IFSV_ALL_ATTR_SET(rec->intf_data.if_info.if_am);
   rec->node.key_info = (uns8*)&rec->intf_data.if_index;
   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   
   /* Add the record to the interface rec table */
   if (ncs_patricia_tree_add (p_tree, &rec->node) != NCSCC_RC_SUCCESS)
   {
    m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncs_patricia_tree_add():ifd_a2s_intf_data_create_handler returned failure"," ");
     m_MMGR_FREE_IFSV_INTF_REC(rec);
     res = NCSCC_RC_FAILURE;
   } else
   {
#if(NCS_IFSV_IPXS == 1)
   res = ipxs_update_ifadd(cb->my_svc_id, &rec->intf_data);
#endif
   }
m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
   
   if (res != NCSCC_RC_SUCCESS)
   {
    m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_stdby_intf_data_create_failure"," ");
    ifd_stdby_error_handler (IFD_STDBY_INTF_DATA_CREATE_FAILURE);
   }

   return (res);

} /* function ifd_a2s_intf_data_create_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_intf_data_modify_handler 
 *
 * Description   : This function modifies the interface data record into its 
 *                 data record as per the information. 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifd_a2s_intf_data_modify_handler (IFSV_INTF_DATA *evt, IFSV_CB *cb)
{  
   uns32 res = NCSCC_RC_FAILURE;
   uns32              ifindex = 0;
   IFSV_INTF_DATA     *rec_data;
   NCS_LOCK  *intf_rec_lock = &cb->intf_rec_lock;

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   
   if ((ifsv_get_ifindex_from_spt(&ifindex, evt->spt_info, cb) == 
          NCSCC_RC_SUCCESS) && ((rec_data = ifsv_intf_rec_find(ifindex, cb)) 
          != IFSV_NULL))
   {
     res = ifsv_intf_rec_modify(rec_data, evt, &evt->if_info.if_am, cb);
     res = NCSCC_RC_SUCCESS;
   }
   
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
   if (res != NCSCC_RC_SUCCESS)
   {
   m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_stdby_intf_data_modify_failure"," ");
   ifd_stdby_error_handler (IFD_STDBY_INTF_DATA_MODIFY_FAILURE);
   }

   return(res);

} /* function ifd_a2s_intf_data_modify_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_intf_data_marked_delete_handler 
 *
 * Description   : This function marks delete the interface data record.  
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifd_a2s_intf_data_marked_delete_handler (IFSV_INTF_DATA *evt, 
                                           IFSV_CB *cb)
{  
   uns32 res = NCSCC_RC_FAILURE;
   uns32              ifindex = 0;
   IFSV_INTF_DATA     *rec_data;
   NCS_LOCK  *intf_rec_lock = &cb->intf_rec_lock;

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   
   if ((ifsv_get_ifindex_from_spt(&ifindex, evt->spt_info, cb) 
         == NCSCC_RC_SUCCESS) && 
       ((rec_data = ifsv_intf_rec_find(ifindex, cb)) != IFSV_NULL))
   {
     rec_data->active = FALSE;
     rec_data->marked_del = TRUE;
     rec_data->if_info.oper_state = NCS_STATUS_DOWN;
     res = NCSCC_RC_SUCCESS;
   }
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   if (res != NCSCC_RC_SUCCESS)
     {
     m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_stdby_intf_data_marked_delete_failure"," ");
     ifd_stdby_error_handler (IFD_STDBY_INTF_DATA_MARKED_DELETE_FAILURE);
     }
  
  return(res);

} /* function ifd_a2s_intf_data_marked_delete_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_intf_data_delete_handler 
 *
 * Description   : This function deletes the interface data record from its 
 *                 data record. 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifd_a2s_intf_data_delete_handler (IFSV_INTF_DATA * evt, IFSV_CB *cb)
{  
   uns32 res = NCSCC_RC_FAILURE;
   NCS_LOCK  *intf_rec_lock = &cb->intf_rec_lock;
   IFSV_INTF_DATA     *rec_data;
   IFSV_INTF_REC      *rec;

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);

   if ((rec_data = ifsv_intf_rec_find (evt->if_index, cb)) != IFSV_NULL)
   {
     rec = ifsv_intf_rec_del (evt->if_index, cb);            
      if (rec != NULL)
      {             
       #if(NCS_IFSV_BOND_INTF == 1)      
            if(rec_data->spt_info.type == NCS_IFSV_INTF_BINDING)
             {
               ifsv_delete_binding_interface(cb,evt->if_index);
             }
       #endif
         m_MMGR_FREE_IFSV_INTF_REC(rec);
         res = NCSCC_RC_SUCCESS;
      }
    }

    m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
    
    if (res != NCSCC_RC_SUCCESS)
       { 
       m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_stdby_intf_data_delete_failure"," ");
       ifd_stdby_error_handler (IFD_STDBY_INTF_DATA_DELETE_FAILURE);
       }
  return(res);

} /* function ifd_a2s_intf_data_delete_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_svc_dest_upd_handler
 *
 * Description   : This function is responsible for handling the event when 
 *                 the mapping of IfIndex & MDS Service id and Virtual 
 *                 Destination is added/modified/deleted at active IfD. 
 *
 * Arguments     : info  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifd_a2s_svc_dest_upd_handler (IFD_A2S_SVC_DEST_UPD_EVT *info, IFSV_CB *cb)
{  
   uns32 res = NCSCC_RC_FAILURE;
   IFSV_EVT *temp_evt;
   m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,\
                   "SVCD Dest Update.",0);

   temp_evt->info.ifnd_evt.info.svcd = info->info;

   res = ifd_svcd_upd_from_ifnd_proc_data(temp_evt, cb);

   if (res != NCSCC_RC_SUCCESS)
       { 
       m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_svcd_upd_from_ifnd_proc_data():ifd_a2s_svc_dest_upd_handler returned failure"," ");
       ifd_stdby_error_handler (IFD_STDBY_SVC_DEST_UPD_FAILURE);
       }

  return(res);

} /* function ifd_a2s_svc_dest_upd_handler() ends here */

#if(NCS_IFSV_IPXS == 1)
/****************************************************************************
 * Name          : ifd_a2s_ipxs_intf_info_handler
 *
 * Description   : This function is responsible for handling IPXS message.
 *
 * Arguments     : info  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_a2s_ipxs_intf_info_handler (IFD_A2S_IPXS_INTF_INFO_EVT *info, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   NCS_IFSV_IFINDEX  if_index;
   IPXS_EVT ipxs_evt;
   IPXS_CB  *ipxs_cb;
   uns32    ipxs_hdl;

   m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,"IPXS Info Update.",0);
 
   /* Get the IPXS CB */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, ipxs_hdl);

   ipxs_evt.info.d.ndtod_upd = info->info;

   res = ifd_ipxs_proc_data_ifip_info(ipxs_cb, &ipxs_evt, &if_index);

   if (res != NCSCC_RC_SUCCESS)
       {
       m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_ipxs_proc_data_ifip_info():ifd_a2s_ipxs_intf_info_handler returned failure"," ");
       ifd_stdby_error_handler (IFD_STDBY_IPXS_INTF_INFO_UPD_FAILURE);
       }
   return(res);

} /* function ifd_a2s_ipxs_intf_info_handler() ends here */
#endif

#if(NCS_VIP == 1)
/****************************************************************************
 * Name          : ifd_a2s_vip_rec_handler
 *
 * Description   : This function is responsible for handling the event related
 *                 to VIP information. Calls the
 *                 corresponding handling function as per evt type.
 *
 * Arguments     : info  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_a2s_vip_rec_handler (IFD_A2S_VIP_REC_INFO_EVT *p_evt, IFSV_CB *cb)
{
  uns32 res = NCSCC_RC_FAILURE;

  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,\
                      "VIP Update. Type is :",\
                      p_evt->type);

  switch (p_evt->type)
  {
    case IFD_A2S_VIP_REC_CREATE:
    {
      if(cb->cold_or_warm_sync_on == FALSE)
      {
         res = ifd_a2s_vip_rec_create_handler(&p_evt->info, cb);
      }
      else
      {
        m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,\
        "Cann't add vip. Cold or warm sync is going on. Index : ",\
               p_evt->type);
      }
      break;
    }
    case IFD_A2S_VIP_REC_MODIFY:
    {
      res = ifd_a2s_vip_rec_modify_handler
                      (&p_evt->info, cb);
     break;
    }
    case IFD_A2S_VIP_REC_DEL:
    {
      res = ifd_a2s_vip_rec_delete_handler
                      (&p_evt->info, cb);
     break;
    }
   default:
    break;
  } /* info->type */
  return (res);
} /* function ifd_a2s_vip_rec_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_vip_rec_create_handler
 *
 * Description   : This function is responsible for handling VIP Rec create message.
 *
 * Arguments     : info  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None. Need to free the Corresponding Memory Allocations 
 *                 in this Function
 *****************************************************************************/
uns32
ifd_a2s_vip_rec_create_handler (VIP_REDUNDANCY_RECORD *pVipChkptPkt, IFSV_CB *cb)
{
   IFSV_IFD_VIPD_RECORD    *pVipdRec;
   NCS_PATRICIA_NODE        *pVipdNode;
   uns32                    ii;
   uns32                    rc;
   NCS_IFSV_VIP_INT_HDL      vipHandle;
   
   if(pVipChkptPkt == IFSV_NULL)
      return NCSCC_RC_FAILURE;
  
   m_NCS_MEMSET(&vipHandle,0,sizeof(NCS_IFSV_VIP_INT_HDL));


/*
   memcpy(&vipHandle,&pVipChkptPkt->handle,
                                           sizeof(NCS_IFSV_VIP_INT_HDL));

   pVipdNode = ifsv_vipd_vipdc_rec_get(&cb->vipDBase,&vipHandle);
   if (pVipdNode != IFSV_NULL)
   {

      pVipdRec = (IFSV_IFD_VIPD_RECORD *)pVipdNode;

      printf("Marking Entry as stale with ApplName : %s and handle: %d",
                  vipHandle.vipApplName,vipHandle.poolHdl);
      
      if(pVipChkptPkt->vip_entry_attr != pVipdRec->vip_entry_attr)
         * making the entry as stale *
         pVipdRec->vip_entry_attr = pVipChkptPkt->vip_entry_attr;
   }
   else
   {
*/
  
     /* Since this will be the first time allocate a vipd record */

    pVipdRec = m_MMGR_ALLOC_IFSV_VIPD_REC;

    if (pVipdRec == IFSV_NULL)
    {
       /* Log Error message */
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_MEM_ALLOC_FAILED);
       return NCSCC_RC_FAILURE;
    }

    m_NCS_MEMSET(pVipdRec,0,sizeof(IFSV_IFD_VIPD_RECORD));

    m_NCS_STRCPY(&pVipdRec->handle.vipApplName,
             &pVipChkptPkt->handle.vipApplName);
 
    pVipdRec->handle.poolHdl =
            pVipChkptPkt->handle.poolHdl;

    pVipdRec->handle.ipPoolType =
            pVipChkptPkt->handle.ipPoolType;

    pVipdRec->ref_cnt = pVipChkptPkt->ref_cnt;

    pVipdRec->vip_entry_attr = pVipChkptPkt->vip_entry_attr;

    /* Check for the Initialization of this Linked Lists */
    rc = ifsv_vip_init_ip_list(&pVipdRec->ip_list);
    if (rc == NCSCC_RC_FAILURE)
    {
       /* NEED TO FREE THE RECORD */
       m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec); 
       return NCSCC_RC_FAILURE;
    }
    rc = ifsv_vip_init_intf_list(&pVipdRec->intf_list);
    if (rc == NCSCC_RC_FAILURE)
    {
       m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec); 
       /* NEED TO FREE THE RECORD AND THE INITIALIZED LISTS */
       return NCSCC_RC_FAILURE;
    }
    rc = ifsv_vip_init_owner_list(&pVipdRec->owner_list);
    if (rc == NCSCC_RC_FAILURE)
    {
       m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec); 
       /* NEED TO FREE THE RECORD AND INITIALIZED LISTS */
       return NCSCC_RC_FAILURE;
    }
    rc = ifsv_vip_init_allocated_ip_list(&pVipdRec->alloc_ip_list);
    if (rc == NCSCC_RC_FAILURE)
    {
       /* NEED TO FREE THE RECORD AND INITIALIZED LISTS */
       m_MMGR_FREE_IFSV_VIPD_REC(pVipdRec); 
       return NCSCC_RC_FAILURE;
    }
/*************************************************************************/
    /* Add IP NODE, INTF NODE and OWNER NODE to the respective lists */
    for (ii = 0; ii < pVipChkptPkt->ip_list_cnt; ii++)
    {
        VIP_RED_IP_NODE *ipNode;
        ipNode = &pVipChkptPkt->ipInfo[ii];
 
        rc = ifsv_vip_add_ip_node(&pVipdRec->ip_list,
                              &ipNode->ip_addr,
                              (uns8 *)&ipNode->intfName);
        {
           if ( rc == NCSCC_RC_FAILURE)
           {
              /* LOG ERROR MESSAGE */
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_INTERNAL_ERROR);
              return NCSCC_RC_FAILURE;
           }
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_ADDED_IP_NODE_TO_VIPD);
        }  
    }
/***************************************************************************/
       /* Adding Intf node to the intf list of the vipd cache*/
    for (ii = 0; ii < pVipChkptPkt->intf_list_cnt; ii++)
    {
        VIP_RED_INTF_NODE *intfNode;
        intfNode = &pVipChkptPkt->intfInfo[ii];
        rc = ifsv_vip_add_intf_node (&pVipdRec->intf_list,
                                     (uns8 *)&intfNode->intfName);
        { 
           if ( rc == NCSCC_RC_FAILURE)
          {
            /* LOG ERROR MESSAGE */
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_INTERNAL_ERROR);
              return NCSCC_RC_FAILURE;
           }
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_ADDED_INTERFACE_NODE_TO_VIPD);
        }
    }

    for (ii = 0; ii < pVipChkptPkt->owner_list_cnt; ii++)
    {
        VIP_RED_OWNER_NODE *ownerNode;
        ownerNode = &pVipChkptPkt->ownerInfo[ii];
 
       /* Adding owner node to the owner list */
        rc = ifsv_vip_add_owner_node(&pVipdRec->owner_list,
                                    &ownerNode->owner
#if (VIP_HALS_SUPPORT == 1)
                                    ,ownerNode->infraFlag
#endif
                                    );
        {
           if ( rc == NCSCC_RC_FAILURE)
           {
              m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_INTERNAL_ERROR);
              return NCSCC_RC_FAILURE;
           }
           /* LOG: ADDED OWNER NODE TO VIPD CACHE */
           m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_ADDED_OWNER_NODE_TO_VIPD);
        }
    }
 
        /* TBD: Add IP TO THE IP ALLOCATED LIST */
 
        /* Since all the lists are updated, add vipd record to vip database */
 
        pVipdNode = (NCS_PATRICIA_NODE *)pVipdRec;
        pVipdNode->key_info = (uns8 *)&pVipdRec->handle;
 
        rc = ifsv_vipd_vipdc_rec_add(&cb->vipDBase, pVipdNode);
        if (rc == NCSCC_RC_FAILURE )
        {
           /* Log Error message */
           m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_RECORD_ADDITION_TO_VIPD_FAILURE);
           return NCSCC_RC_FAILURE;
        }
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_RECORD_ADDITION_TO_VIPD_SUCCESS);
    
/*  }  end of else */
 
   return NCSCC_RC_SUCCESS;
} /* The function ifd_a2s_vip_rec_create_handler() ends here. */
 
 /****************************************************************************
 * Name          : ifd_a2s_vip_rec_modify_handler
 *
 * Description   : This function is responsible for handling VIP Rec modify message.
 *
 * Arguments     : info  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_a2s_vip_rec_modify_handler (VIP_REDUNDANCY_RECORD *info, IFSV_CB *cb)
{
    NCS_PATRICIA_NODE            *pPatEntry;
    uns32                         rc;

    if (cb == IFSV_NULL || info == IFSV_NULL)
    {
        return NCSCC_RC_FAILURE;
     }
    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,
                        IFSV_VIP_RECEIVED_IFND_VIP_FREE_REQ);

    pPatEntry = ifsv_vipd_vipdc_rec_get(&cb->vipDBase,
                          &info->handle);
    if (pPatEntry == IFSV_NULL)
    {
     /*LOGS:: TBD
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,IFSV_VIP_VIPD_LOOKUP_FAILED);
     */
       return NCSCC_RC_FAILURE;
    }
    else
    {
      /* Data rec is already here, so we need to delete and create a new record. */
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,IFSV_VIP_VIPD_LOOKUP_SUCCESS);
       rc = ifd_a2s_vip_rec_delete_handler(info,cb);
       rc = ifd_a2s_vip_rec_create_handler(info, cb);
    }     

    return rc;

} /* The function ifd_a2s_vip_rec_modify_handler() ends here. */

 /****************************************************************************
 * Name          : ifd_a2s_vip_rec_delete_handler
 *
 * Description   : This function is responsible for handling VIP Rec Del message.
 *
 * Arguments     : info  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_a2s_vip_rec_delete_handler (VIP_REDUNDANCY_RECORD *info, IFSV_CB *cb)
{
    NCS_PATRICIA_NODE            *pPatEntry;
    IFSV_IFD_VIPD_RECORD         *pVipd;
    uns32                         rc;

    if (cb == IFSV_NULL || info == IFSV_NULL)
    {
        return NCSCC_RC_FAILURE;
     }
    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,
                        IFSV_VIP_RECEIVED_IFND_VIP_FREE_REQ);

    pPatEntry = ifsv_vipd_vipdc_rec_get(&cb->vipDBase,
                          &info->handle);
    if (pPatEntry == IFSV_NULL)
    {
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,IFSV_VIP_VIPD_LOOKUP_FAILED);
       return NCSCC_RC_FAILURE;
    }
    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD,IFSV_VIP_VIPD_LOOKUP_SUCCESS);

    pVipd = (IFSV_IFD_VIPD_RECORD *)pPatEntry;
    rc = ifsv_vipd_vipdc_rec_del(&cb->vipDBase, (NCS_PATRICIA_NODE *)pVipd, IFSV_VIP_REC_TYPE_VIPD);
    if (rc == NCSCC_RC_FAILURE) /* IF VIPD is not cleared */
    {
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_RECORD_DELETION_FAILED);
       return NCSCC_RC_FAILURE;
    }

    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_RECORD_DELETION_SUCCESS);

    return NCSCC_RC_SUCCESS;

} /* The function ifd_a2s_vip_rec_delete_handler() ends here. */

#endif

/****************************************************************************
 * Name          : ifd_a2s_ifnd_up_down_info_handler
 *
 * Description   : This function is responsible for handling IFND DOWN message.
 *
 * Arguments     : info  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_a2s_ifnd_up_down_info_handler (IFD_A2S_IFND_UP_DOWN_EVT *info, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   MDS_DEST mds_dest;
   NODE_ID  node_id;
   uns32 ifindex = 0;   
   IFSV_INTF_REC       *intf_rec;
   NCS_PATRICIA_NODE   *if_node;   
   NCS_LOCK            *intf_rec_lock = &cb->intf_rec_lock;
   IFD_IFND_NODE_ID_INFO_REC *rec = NULL;

   m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,"IFND UP/DOWN Info. Type is :", info->type);
   
  switch (info->type)
  {
    case IFD_A2S_IFND_DOWN_EVT:
    {
 
     mds_dest = info->mds_dest;
     node_id = m_NCS_NODE_ID_FROM_MDS_DEST(mds_dest);
     
     /* Plugin for marking the vipd entries corresponding to crashed
        ifnd as stale entries */
     ifd_vip_process_ifnd_crash(&mds_dest,cb);

     /* Add the node id of the DOWN IfND in the Patricia tree.*/ 
     res = ifd_ifnd_node_id_info_add(&rec, &node_id, cb);

     if((res != NCSCC_RC_SUCCESS) && (rec == IFSV_NULL))
     {
       m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_ifnd_node_id_info_add():ifd_a2s_ifnd_up_down_info_handler returned failure"," ");
       return (NCSCC_RC_FAILURE);
     }

     /* Starts a retention timer for this IfND.*/
     rec->info.tmr.tmr_type = NCS_IFSV_IFD_EVT_RET_TMR;
     rec->info.tmr.svc_id = cb->my_svc_id;
     rec->info.tmr.info.node_id = node_id;
     ifsv_tmr_start(&rec->info.tmr, cb);
   
     m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
     if_node = ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)&ifindex);
     intf_rec = (IFSV_INTF_REC*)if_node;

     while (intf_rec != IFSV_NULL)
     {
       ifindex = intf_rec->intf_data.if_index;

       if((m_NCS_IFSV_IS_MDS_DEST_SAME(&mds_dest, &intf_rec->intf_data.
          current_owner_mds_destination) == TRUE) && 
         (intf_rec->intf_data.current_owner == NCS_IFSV_OWNER_IFND))
       {
         intf_rec->intf_data.active = FALSE;
         intf_rec->intf_data.if_info.oper_state = NCS_STATUS_DOWN;
       }
      
       if_node = ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)&ifindex);
      
       intf_rec = (IFSV_INTF_REC*)if_node;
     }
     m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

     if (res != NCSCC_RC_SUCCESS)
         {
          m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_stdby_ifnd_down_failure"," ");
         ifd_stdby_error_handler (IFD_STDBY_IFND_DOWN_FAILURE);
         }
     break;
   } /* case IFD_A2S_IFND_DOWN_EVT */

   case IFD_A2S_IFND_UP_EVT:
   {
     IFND_NODE_ID_INFO *node_id_info = NULL;
     node_id = m_NCS_NODE_ID_FROM_MDS_DEST(info->mds_dest);

     /* Check whether any retention timer is running or not. */
     node_id_info = ifd_ifnd_node_id_info_get(node_id, cb);   

     if(node_id_info == NULL)   
     {
       /* Do nothing. */ 
     }
     else
     {
       /* Retention timer is running. So, stop the timer and delete the Patricia
        tree node.*/
       ifsv_tmr_stop(&node_id_info->tmr, cb); 
       ifd_ifnd_node_id_info_del(node_id, cb);
       
       m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
       if_node = ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)&ifindex);
       intf_rec = (IFSV_INTF_REC*)if_node;

       while (intf_rec != IFSV_NULL)
       {
         ifindex = intf_rec->intf_data.if_index;

         if(node_id == 
            m_NCS_NODE_ID_FROM_MDS_DEST(intf_rec->intf_data.current_owner_mds_destination))
         {
            /* This intf rec has been created by this IfND. So, mark the 
               Operational Status UP of this record and broadcast to all 
               IfNDs. */
            if(intf_rec->intf_data.if_info.oper_state == NCS_STATUS_DOWN)
            {
              intf_rec->intf_data.if_info.oper_state = NCS_STATUS_UP;
            }
            else
            {
              m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
              return NCSCC_RC_FAILURE;
            }
         }

         if_node = ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)&ifindex);

         intf_rec = (IFSV_INTF_REC*)if_node;
       } /* while (intf_rec != IFSV_NULL) */
       
       m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

       if (res != NCSCC_RC_SUCCESS)
        {
         m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_stdby_ifnd_up_failure"," ");
         ifd_stdby_error_handler (IFD_STDBY_IFND_UP_FAILURE);
        }

       
     } /* node_id_info != NULL  */

    break;
   } /* case IFD_A2S_IFND_UP_EVT */

   default:
     break;
  }
   return(res);

} /* function ifd_a2s_ifnd_up_down_info_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_ifindex_upd_handler
 *
 * Description   : This function is responsible for handling the IAPS data
 *                 changes in async update.
 *
 * Arguments     : info  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifd_a2s_ifindex_upd_handler (IFD_A2S_IFINDEX_UPD_EVT *info, IFSV_CB *cb)
{
  uns32 res = NCSCC_RC_FAILURE;
  NCS_IFSV_SPT_MAP  spt_map;

  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,"Ifindex Update Info. Index is :",info->type);
   

  m_NCS_MEMSET(&spt_map, 0, sizeof(NCS_IFSV_SPT_MAP));

  switch (info->type)
  {
    case IFD_A2S_IFINDEX_EVT_ALLOC:
    {
      /* When cold or warm sync is on, then no addition of index. */
      if(cb->cold_or_warm_sync_on == TRUE)
      {
        m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,"Couldn't allocated. Cold or warm sync going on.", info->ifindex);
         return res;
      } 
      if (m_IFD_IFAP_IFINDEX_ALLOC(&spt_map, cb) == NCSCC_RC_FAILURE)
      {
         m_IFSV_LOG_SPT_INFO(IFSV_LOG_IFINDEX_ALLOC_FAILURE, &spt_map,"ifd_a2s_ifindex_upd_handler");
        res = NCSCC_RC_FAILURE;
      }
      else
      {
         m_IFSV_LOG_SPT_INFO(IFSV_LOG_IFINDEX_ALLOC_SUCCESS,&spt_map,"ifd_a2s_ifindex_upd_handler");
         /* add the Slot/Port/Type Vs Ifindex mapping in to the tree */
         res = NCSCC_RC_SUCCESS;
      }

      if(res != NCSCC_RC_SUCCESS)
          {
          m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"failure in ifd_a2s_ifindex_evt_alloc:ifd_a2s_ifindex_upd_handler"," ");
          ifd_stdby_error_handler (IFD_A2S_IFINDEX_ALLOC_MSG);
          }
      break;
    }

    case IFD_A2S_IFINDEX_EVT_DEALLOC:
    {
      spt_map.if_index = info->ifindex;
      
      if (m_IFD_IFAP_IFINDEX_FREE(spt_map.if_index, cb) 
               == NCSCC_RC_FAILURE)
      {
         m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFINDEX_FREE_FAILURE,spt_map.if_index,\
                             spt_map.spt.port);
         res = NCSCC_RC_FAILURE;
      }      
      else
      {
         m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFINDEX_FREE_SUCCESS,spt_map.if_index,\
                             spt_map.spt.port);
         res = NCSCC_RC_SUCCESS;
      }
      if(res != NCSCC_RC_SUCCESS)
           {
           m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"failure in ifd_a2s_ifindex_evt_dealloc:ifd_a2s_ifindex_upd_handler"," ");
           ifd_stdby_error_handler (IFD_A2S_IFINDEX_DEALLOC_MSG);
           }
      break;
    }

   default:
    break;
  } /* info->type */

  return res; 

} /* function ifd_a2s_ifindex_upd_handler() ends here */

/****************************************************************************
 * Name          : ifd_a2s_sync_resp
 *
 * Description   : This function is responsible for handling the data record 
 *                 when from cold syn or warm sync process.
 *
 * Arguments     : msg  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_a2s_sync_resp (IFSV_CB *cb, uns8 *intf_msg
#if(NCS_IFSV_IPXS == 1)
, uns8 *ipxs_msg
#endif  
)
{
  uns32 res = NCSCC_RC_FAILURE;
  NCS_IFSV_SPT_MAP spt_map;
  IFSV_INTF_REC  *rec;
  IFSV_INTF_DATA *info = (IFSV_INTF_DATA *)intf_msg;
  static int count = 0;

  count++;

#if(NCS_IFSV_IPXS == 1)
  /* For IPXS. */
  NCS_IFSV_IFINDEX  if_index;
  IPXS_EVT ipxs_evt;
  IPXS_CB  *ipxs_cb;
  uns32    ipxs_hdl;

   /* Get the IPXS CB */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, ipxs_hdl);

#endif  

  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_SYNC_RESP_RCV,"If data and ipxs data.",0);
   
  spt_map.if_index = info->if_index;
  spt_map.spt = info->spt_info;

  res = ifd_a2s_ifindex_spt_map_create_handler (&spt_map, cb);   

  if(res == NCSCC_RC_SUCCESS)
  {
    if (info->if_info.oper_state == NCS_STATUS_UP)
        res = ifd_a2s_intf_data_create_handler(info, cb, TRUE);
    else
        res = ifd_a2s_intf_data_create_handler(info, cb, FALSE);
  } 
  else /* when res is FAILURE. */
  {
     /* If spt info was not added, then go to end. */
     goto end;
  }

  /* If the record has been added, add IPXS data. */ 
  if(res == NCSCC_RC_SUCCESS)
  {

#if(NCS_IFSV_IPXS == 1)
      /* Now add the IPXS. */
      ipxs_evt.info.d.ndtod_upd.if_index = spt_map.if_index;
      memcpy(&ipxs_evt.info.d.ndtod_upd.ip_info, ipxs_msg, 
                   sizeof(NCS_IPXS_IPINFO));

      res = ifd_ipxs_proc_data_ifip_info(ipxs_cb, &ipxs_evt, &if_index);
      if(res != NCSCC_RC_SUCCESS)
      {
       m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_ipxs_proc_data_ifip_info:ifd_a2s_sync_resp"," ");
         rec = ifsv_intf_rec_del(info->if_index, cb);
         if (rec != NULL)
         {
           /* Problem in adding the SVC DEST, may be memory failure. */
           ifsv_ifindex_spt_map_del (info->spt_info, cb);
           m_MMGR_FREE_IFSV_INTF_REC(rec);
           res = NCSCC_RC_FAILURE;
           goto end;
         }

      } /* if(res != NCSCC_RC_SUCCESS) */
#endif
  } /* if(res == NCSCC_RC_SUCCESS)  */
  else
  {
     /* We could not add the interface, so delete the map. */
     ifsv_ifindex_spt_map_del (info->spt_info, cb);
     goto end;
  }

end:
  return(res);

} /* function ifd_a2s_sync_resp() ends here */

/****************************************************************************
 * Name          : ifd_a2s_iaps_sync_resp
 *
 * Description   : This function is responsible for handling the IAPS data
 *                 when from cold syn or warm sync process happens.
 *
 * Arguments     : msg  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_a2s_iaps_sync_resp (IFSV_CB *cb, IFAP_INFO_LIST_A2S *msg)
{
  uns32 res = NCSCC_RC_SUCCESS;
  uns32 max_ifindex = 0;
  uns32 num_free_ifindex = 0;
  NCS_IFSV_SPT_MAP  spt_map; /* Some dummy value. */
  uns32 *free_list = msg->free_list;

  m_NCS_MEMSET(&spt_map,0,sizeof(NCS_IFSV_SPT_MAP));
  m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_A2S_SYNC_RESP_RCV,"IAPS data.",0);

  max_ifindex = msg->max_ifindex;
  num_free_ifindex = msg->num_free_ifindex;

  /* Add the interfaces indexes */
  while(max_ifindex)
  {
    if (m_IFD_IFAP_IFINDEX_ALLOC(&spt_map, cb) == NCSCC_RC_FAILURE)
    {
      m_IFSV_LOG_SPT_INFO(IFSV_LOG_IFINDEX_ALLOC_FAILURE, &spt_map,"ifd_a2s_iaps_sync_resp");
      res = NCSCC_RC_FAILURE;
    } 
    else
    {
      m_IFSV_LOG_SPT_INFO(IFSV_LOG_IFINDEX_ALLOC_SUCCESS, &spt_map,"ifd_a2s_iaps_sync_resp");
      /* add the Slot/Port/Type Vs Ifindex mapping in to the tree */
      res = NCSCC_RC_SUCCESS;
   }

   if(res != NCSCC_RC_SUCCESS)
    {
       m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"m_IFD_IFAP_IFINDEX_ALLOC():ifd_a2s_iaps_sync_resp returned failure"," ");
       return res;
    }
    max_ifindex--;
  }
 
  /* Now delete the interface indexes */ 
  while(num_free_ifindex)
  {
    spt_map.if_index = *free_list;

    if (m_IFD_IFAP_IFINDEX_FREE(spt_map.if_index, cb) 
          == NCSCC_RC_FAILURE)
    {
       m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFINDEX_FREE_FAILURE,spt_map.if_index,\
         spt_map.spt.port);
       return (NCSCC_RC_FAILURE);
    }      
    else
    {
       m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFINDEX_FREE_SUCCESS,spt_map.if_index,\
                           spt_map.spt.port);
    }

    num_free_ifindex--;
    free_list++; 
  }

  return res; 

} /* function ifd_a2s_iaps_sync_resp() ends here */

/****************************************************************************
 * Name          : ifd_stdby_error_handler 
 *
 * Description   : This function takes appropriate action based on error type. 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static void 
ifd_stdby_error_handler (IFD_STDBY_ERROR_TYPE error)
{
 switch(error)
 {
   case IFD_STDBY_IFINDEX_SPT_MAP_ADD_FAILURE:
   {
    break;
   }

   case IFD_STDBY_IFINDEX_SPT_MAP_DELETE_FAILURE:
   {
    break;
   }
   
   case IFD_STDBY_INTF_DATA_CREATE_FAILURE:
   {
    break;
   }
   
   case IFD_STDBY_INTF_DATA_MODIFY_FAILURE:
   {
    break;
   }
   
   case IFD_STDBY_INTF_DATA_MARKED_DELETE_FAILURE:
   {
    break;
   }
   
   case IFD_STDBY_INTF_DATA_DELETE_FAILURE:
   {
    break;
   }

   case IFD_STDBY_SVC_DEST_UPD_FAILURE:
   {
    break;
   }

   default :
   {
    break;
   }
  } /* switch(error) */
} /* function ifd_stdby_error_handler() ends here */

/****************************************************************************
 * Name          : ifd_ifnd_node_id_info_add
 *
 * Description   : This function will add IFD_IFND_NODE_ID_INFO_REC in its 
 *                 data base.
 *
 * Arguments     : rec - the pointer to be returned back after allocating 
 *                       memory and adding it in the patricia tree.
 *                 node_id  - This is the NODE_ID of the IfND.
 *                 cb       - Ifnd Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifnd_node_id_info_add (IFD_IFND_NODE_ID_INFO_REC **rec, NODE_ID *node_id, 
                           IFSV_CB *cb)
{
  NCS_PATRICIA_TREE  *p_tree = &cb->ifnd_node_id_tbl;
  IFD_IFND_NODE_ID_INFO_REC *ptr = NULL;

  *rec = m_MMGR_ALLOC_IFD_NODE_ID_INFO_REC;
  ptr = *rec;

  if(ptr == IFSV_NULL)
  {
    m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
    return (NCSCC_RC_FAILURE);
  }

  m_NCS_MEMSET(ptr, 0, sizeof(IFD_IFND_NODE_ID_INFO_REC));

  ptr->info.ifnd_node_id = *node_id;
  ptr->pat_node.key_info = (uns8*)&ptr->info.ifnd_node_id;
  /* This is the idea of find and add */
  if (ncs_patricia_tree_get(p_tree, (uns8 *)node_id) == NULL)
  {
     if (ncs_patricia_tree_add (p_tree, &ptr->pat_node) != NCSCC_RC_SUCCESS)
     {
        m_MMGR_FREE_IFD_NODE_ID_INFO_REC(ptr);
        m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_NODE_ID_TBL_ADD_FAILURE, *node_id, 0);

        *rec = NULL;
        return (NCSCC_RC_FAILURE);
     }
  }
  else
  {
    m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_ifnd_node_id_info_add(): node_id already exists",*node_id);
    m_MMGR_FREE_IFD_NODE_ID_INFO_REC(ptr);
    *rec = NULL;
    return (NCSCC_RC_FAILURE);
  }

   m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_NODE_ID_TBL_ADD_SUCCESS, *node_id, 0);
   return (NCSCC_RC_SUCCESS);

} /* function ifd_ifnd_node_id_info_add() ends here */

/****************************************************************************
 * Name          : ifd_ifnd_node_id_info_get
 *
 * Description   : This gets node id info from patricia tree.
 *
 * Arguments     : mds_dest - mds dest info.
 *                 cb   - This is the interace control block.
 *
 * Return Values : Pointer to the structure IFND_NODE_ID_INFO if found, else
 *                 NULL.
 *
 * Notes         : None.
 *****************************************************************************/
IFND_NODE_ID_INFO *
ifd_ifnd_node_id_info_get (NODE_ID node_id, IFSV_CB *cb)

{
  NCS_PATRICIA_TREE  *p_tree = &cb->ifnd_node_id_tbl;
  IFD_IFND_NODE_ID_INFO_REC *rec = NULL;
  IFND_NODE_ID_INFO *info = NULL;

  rec = (IFD_IFND_NODE_ID_INFO_REC *)ncs_patricia_tree_get(p_tree,
                                     (uns8 *)&node_id);

  if (rec != NULL)
  {
    info = (IFND_NODE_ID_INFO *)&rec->info;
  }
  else
  {
    info = NULL;
  }

   return (info);

} /* function ifd_ifnd_node_id_info_get() ends here */

/****************************************************************************
 * Name          : ifd_ifnd_node_id_info_destroy_all
 *
 * Description   : This is called to destroy all node id info
 *                 from the patricia tree.
 *
 * Arguments     : cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifnd_node_id_info_destroy_all (IFSV_CB *cb)
{
  NCS_PATRICIA_NODE  *node = NULL;
  IFD_IFND_NODE_ID_INFO_REC *rec = NULL;
  NODE_ID node_id;

  m_NCS_MEMSET(&node_id, 0, sizeof(NODE_ID));

  node = ncs_patricia_tree_getnext(&cb->ifnd_node_id_tbl, (uns8*)&node_id);

  rec = (IFD_IFND_NODE_ID_INFO_REC *)node;

  while (rec != NULL)
  {
    memcpy(&node_id, &rec->info.ifnd_node_id, sizeof(NODE_ID));

    ifd_ifnd_node_id_info_del(rec->info.ifnd_node_id, cb);

    node = ncs_patricia_tree_getnext(&cb->ifnd_node_id_tbl, (uns8*)&node_id);

    rec = (IFD_IFND_NODE_ID_INFO_REC *)node;
  }

  return(NCSCC_RC_SUCCESS);
} /* function ifd_ifnd_node_id_info_destroy_all() ends here */

/****************************************************************************
 * Name          : ifd_ifnd_node_id_info_del
 *
 * Description   : This is called to destroy node_id record from the
 *                 patricia tree.
 *
 * Arguments     : cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ifnd_node_id_info_del (NODE_ID node_id, IFSV_CB *cb)
{
  NCS_PATRICIA_TREE  *p_tree = &cb->ifnd_node_id_tbl;
  NCS_PATRICIA_NODE  *node = NULL;

  node = ncs_patricia_tree_get (p_tree, (uns8 *)&node_id);

  if (node == IFSV_NULL)
  {
      m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
        IFSV_LOG_NODE_ID_TBL_DEL_FAILURE, node_id, 0);

      return (NCSCC_RC_FAILURE);
  }

   ncs_patricia_tree_del (p_tree, node);

   /* stop the cleanp timer if it is running */
   m_MMGR_FREE_IFD_NODE_ID_INFO_REC(node);

   m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
        IFSV_LOG_NODE_ID_TBL_DEL_SUCCESS, node_id, 0);

   return (NCSCC_RC_SUCCESS);

} /* function ifd_ifnd_node_id_info_del() ends here */

/*********************  file ends here ************************************/

/************************************************************************
*****************   This function is for debugging in tough times    ****
*************************************************************************/

void data_dump_VIP_REDUNDANCY_RECORD(VIP_REDUNDANCY_RECORD *pVipChkptPkt)
{
  int i = 0;
  printf("\n Data Dump of VIP_REDUNDANCY_RECORD"); 
  printf("\n ========================================\n\n"); 
  printf("pVipChkptPkt->handle.poolHdl is %d\n", pVipChkptPkt->handle.poolHdl);
  printf("pVipChkptPkt->vip_entry_attr is %d\n", pVipChkptPkt->vip_entry_attr);
  printf("pVipChkptPkt->ref_cnt is %d\n", pVipChkptPkt->ref_cnt);
  printf("pVipChkptPkt->ip_list_cnt is %d\n\n", pVipChkptPkt->ip_list_cnt);
  for(i=0; i<pVipChkptPkt->ip_list_cnt; i++)
  {
    printf("pVipChkptPkt->ipInfo[%d].ip_addr.ipaddr.type is %d\n", i,pVipChkptPkt->ipInfo[i].ip_addr.ipaddr.type);
    printf("pVipChkptPkt->ipInfo[%d].ip_addr.ipaddr.info.v4 is %d\n", i,pVipChkptPkt->ipInfo[i].ip_addr.ipaddr.info.v4);
    printf("pVipChkptPkt->ipInfo[%d].ip_addr.mask_len is %d\n", i,pVipChkptPkt->ipInfo[i].ip_addr.mask_len);

    printf("pVipChkptPkt->ipInfo[%d].ipAllocated is %d\n",i, pVipChkptPkt->ipInfo[i].ipAllocated);
    printf("pVipChkptPkt->ipInfo[%d].intfName is %s\n\n", i,pVipChkptPkt->ipInfo[i].intfName);
    
  }
  printf("\npVipChkptPkt->intf_list_cnt is %d\n\n", pVipChkptPkt->intf_list_cnt);
  for(i=0; i<pVipChkptPkt->intf_list_cnt; i++)
  {
    printf("pVipChkptPkt->intfInfo[%d].intfName is %s\n", i, pVipChkptPkt->intfInfo[i].intfName);
    printf("pVipChkptPkt->intfInfo[%d].active_standby is %d\n\n", i, pVipChkptPkt->intfInfo[i].active_standby);
  }
     
  printf("pVipChkptPkt->alloc_ip_list_cnt is %d\n", pVipChkptPkt->alloc_ip_list_cnt);
  printf("pVipChkptPkt->owner_list_cnt is %d\n\n", pVipChkptPkt->owner_list_cnt);
  for(i=0; i<pVipChkptPkt->intf_list_cnt; i++)
  {
    printf("pVipChkptPkt->ownerInfo[%d].owner is %lld\n", i, pVipChkptPkt->ownerInfo[i].owner);
    printf("pVipChkptPkt->ownerInfo[%d].infraFlag is %d\n", i, pVipChkptPkt->ownerInfo[i].infraFlag);
  }
 printf("\n\n Data Dump Ends \n");
 printf(" ========================================\n\n"); 
return;
}
