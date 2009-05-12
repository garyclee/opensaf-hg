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
  FILE NAME: IFD_EVT.C

  DESCRIPTION: IfD events received and send related routines.

  FUNCTIONS INCLUDED in this module:
  ifd_intf_create_modify ............ Create or modify interface record.
  ifd_intf_destroy .................. Destroy interface record.
  ifd_intf_init_done .................IfND Init done.  
  ifd_intf_ifindex_req................Resolve the request for ifindex.
  ifd_intf_ifindex_cleanup............Free the allocated ifindex.
  ifd_tmr_exp.........................Any timer Expired.
  ifd_intf_rec_sync...................Sync UP the database with the IfND.
  ifd_process_evt.....................IfD Event Processing.  
  ifd_evt_send........................Send Event to IfD.

******************************************************************************/
#include "ifd.h"

static uns32 ifd_quisced_process(IFSV_EVT* evt, IFSV_CB* cb);
static uns32 ifd_ifnd_down(IFSV_EVT* evt, IFSV_CB* cb);
static uns32 ifd_intf_create_modify(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifd_intf_destroy(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifd_intf_init_done(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifd_intf_ifindex_req(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifd_intf_ifindex_cleanup(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifd_tmr_exp(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifd_intf_rec_sync(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifd_svcd_upd_from_ifnd_proc(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifd_mds_msg_send_sync_resp (NCSCONTEXT msg, IFSV_EVT_TYPE msg_type,
                       IFSV_SEND_INFO *send_info, IFSV_CB *ifsv_cb,
                       NCS_IFSV_SYNC_SEND_ERROR error);
static uns32 ifd_intf_rec_send_no_ret_tmr (MDS_DEST mds_dest, IFSV_CB *cb);
static uns32 ifd_intf_rec_send_ret_tmr_running (MDS_DEST mds_dest, IFSV_CB *cb);

uns32 ifd_svcd_upd_from_ifnd_proc_data (IFSV_EVT* evt, IFSV_CB *cb);
void get_next_info_no_ret_tmr(IFSV_CB* cb,IFSV_INTF_REC **intf_rec,
                          #if(NCS_IFSV_IPXS == 1)
                                           IPXS_CB          *ipxs_cb,
                                           IPXS_IFIP_INFO   **ip_info,
                          #endif
                              NCS_IFSV_IFINDEX   *ifindex);

/* IfD dispatch table */
const IFSV_EVT_HANDLER ifd_evt_dispatch_tbl[IFD_EVT_MAX - IFD_EVT_BASE] =
{
   ifd_intf_create_modify,
   ifd_intf_destroy,
   ifd_intf_init_done,   
   ifd_intf_ifindex_req,
   ifd_intf_ifindex_cleanup,
   ifd_tmr_exp,
   ifd_intf_rec_sync,
   ifd_svcd_upd_from_ifnd_proc,
   ifd_quisced_process,
   ifd_ifnd_down 
};

/****************************************************************************
 * Name          : ifd_intf_create_modify
 *
 * Description   : This event callback function which would be called when 
 *                 IfD receives IFD_EVT_INTF_CREATE event. This function
 *                 will create or modifies the interface depending on the 
 *                 presence of the record.
 *
* Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
ifd_intf_create_modify (IFSV_EVT* evt, IFSV_CB *cb)
{  
   uns32 res;
   IFSV_INTF_CREATE_INFO *intf_create;
   char log_info[45];
   
   intf_create = &evt->info.ifd_evt.info.intf_create;
   m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFSV_CREATE_MOD_MSG ,
                                 "From : s/s/ss/p/t/s/ originator",
                                 intf_create->intf_data.spt_info.shelf,
                                 intf_create->intf_data.spt_info.slot,
                                 intf_create->intf_data.spt_info.subslot,
                                 intf_create->intf_data.spt_info.port,
                                 intf_create->intf_data.spt_info.type,
                                 intf_create->intf_data.spt_info.subscr_scope,
                                 intf_create->intf_data.originator);
                                 
   m_IFD_LOG_EVT_LL(IFSV_LOG_IFD_EVT_INTF_CREATE_RCV,\
      ifsv_log_spt_string(intf_create->intf_data.spt_info,log_info),\
      intf_create->if_attr,intf_create->intf_data.originator);
   res = ifd_intf_create(cb, intf_create, &evt->sinfo);
   return (res);
}

/****************************************************************************
 * Name          : ifd_intf_destroy
 *
 * Description   : This event callback function which would be called when 
 *                 IfD receives IFD_EVT_INTF_DESTROY event. This function
 *                 will destroy the interface.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
ifd_intf_destroy (IFSV_EVT* evt, IFSV_CB *cb)
{  
   uns32 res;
   IFSV_INTF_DESTROY_INFO *dest_info;
   uns32 ifindex = 0;


   dest_info = &evt->info.ifd_evt.info.intf_destroy;
   ifsv_get_ifindex_from_spt(&ifindex, dest_info->spt_type,
      cb);
   m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_EVT_INTF_DESTROY_RCV ,
                                 "From : s/s/p/t/s/ originator ifindex",
                                 dest_info->spt_type.shelf,
                                 dest_info->spt_type.slot,
                                 dest_info->spt_type.port,
                                 dest_info->spt_type.type,
                                 dest_info->spt_type.subscr_scope,
                                 dest_info->orign,
                                 ifindex);
   
    printf("ifd_intf_destroy: From: s/s/ss/p/t/s --  %d/%d/%d/%d/%d/%d \n", dest_info->spt_type.shelf,
                                 dest_info->spt_type.slot,
                                 dest_info->spt_type.subslot,
                                 dest_info->spt_type.port,
                                 dest_info->spt_type.type,
                                 dest_info->spt_type.subscr_scope);
/*
   m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_INTF_DESTROY_RCV,\
      ifsv_log_spt_string(dest_info->spt_type,log_info),\
      m_NCS_NODE_ID_FROM_MDS_DEST(dest_info->own_dest));      
*/
   res = ifd_intf_delete(cb, dest_info, &evt->sinfo);
   return (res);
}

/****************************************************************************
 * Name          : ifd_intf_init_done
 *
 * Description   : This event callback function which would be called when 
 *                 IfD receives IFD_EVT_INIT_DONE event. This 
 *                 function set the init done varibale to True and makes the
 *                 ifd operationally up.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifd_intf_init_done (IFSV_EVT* evt, IFSV_CB *cb)
{
   SaAisErrorT error;
   SaAmfHealthcheckKeyT  Healthy;
   SaNameT  SaCompName;
   char *phlth_ptr;
   char hlth_str[32];

   cb->init_done = evt->info.ifd_evt.info.init_done.init_done;
   /** start the AMF health check **/   
   memset(&SaCompName,0,sizeof(SaCompName));
   strcpy(SaCompName.value,cb->comp_name);
   SaCompName.length = strlen(cb->comp_name);

   memset(&Healthy,0,sizeof(Healthy));
   phlth_ptr = getenv("IFSV_ENV_HEALTHCHECK_KEY");
   if (phlth_ptr == NULL)
   {
      /** default health check key **/
      strcpy(hlth_str,"123");
   } else
   {
      strncpy(hlth_str,phlth_ptr, SA_AMF_HEALTHCHECK_KEY_MAX-1);
   }
   strncpy(Healthy.key,hlth_str, SA_AMF_HEALTHCHECK_KEY_MAX-1);
   Healthy.keyLen=strlen(Healthy.key);
  
   error = saAmfHealthcheckStart(cb->amf_hdl,&SaCompName,&Healthy,
      SA_AMF_HEALTHCHECK_AMF_INVOKED,SA_AMF_COMPONENT_RESTART);
   if (error != SA_AIS_OK)
   {
      m_IFD_LOG_API_L(IFSV_LOG_AMF_HEALTH_CHK_START_FAILURE,cb->comp_type);
      return NCSCC_RC_FAILURE;
   } else
   {
      m_IFD_LOG_API_L(IFSV_LOG_AMF_HEALTH_CHK_START_DONE,cb->comp_type);
   }
   m_IFD_LOG_EVT_INIT(IFSV_LOG_IFD_EVT_INIT_DONE_RCV, cb->init_done);   
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifd_intf_ifindex_req
 *
 * Description   : This event callback function which would be called when 
 *                 IfD receives IFD_EVT_IFINDEX_REQ event. This function
 *                 will allocate the ifindex requested by the IfND from IAPS.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : If the Ifindex is alreasy in the Map table then it will 
 *                 give that ifindex back to the requestor.
 *****************************************************************************/
static uns32 
ifd_intf_ifindex_req (IFSV_EVT* evt, IFSV_CB *cb)
{  
   uns32 res;
   IFSV_EVT_SPT_MAP_INFO *spt_map;
   uns32 ifindex = 0;
   char log_info[45];
   NCS_IFSV_SYNC_SEND_ERROR error  = NCS_IFSV_NO_ERROR;

   spt_map = &evt->info.ifd_evt.info.spt_map;
   
   m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_IFINDEX_REQ_RCV,\
      ifsv_log_spt_string(spt_map->spt_map.spt,log_info),spt_map->app_svc_id);

   if (ifsv_get_ifindex_from_spt(&ifindex, spt_map->spt_map.spt, 
      cb) != NCSCC_RC_SUCCESS)
   {
      if (ifsv_ifd_ifindex_alloc(&spt_map->spt_map, cb) != NCSCC_RC_SUCCESS)
      {
         m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifd_ifindex_alloc() returned failure"," ");
         spt_map->spt_map.if_index = 0;
         error = NCS_IFSV_INT_ERROR;
      } else {
         m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_EVT_IFINDEX_REQ_RCV ,
                                 "Ifindex allocated : s/s/ss/p/t/s/ ifindex",
                                 spt_map->spt_map.spt.shelf,
                                 spt_map->spt_map.spt.slot,
                                 spt_map->spt_map.spt.subslot,
                                 spt_map->spt_map.spt.port,
                                 spt_map->spt_map.spt.type,
                                 spt_map->spt_map.spt.subscr_scope,
                                 spt_map->spt_map.if_index);

      }
       
   } else
   {
      spt_map->spt_map.if_index = ifindex;
   }

   /* this is the function used to send an MDS message to IfND */ 
   res = ifd_mds_msg_send_sync_resp((void*)spt_map, IFND_EVT_IFINDEX_RESP, 
                                    &evt->sinfo, cb, error);   
   return (res);
}

/****************************************************************************
 * Name          : ifd_intf_ifindex_cleanup
 *
 * Description   : This event callback function which would be called when 
 *                 IfD receives IFD_EVT_IFINDEX_CLEANUP event. This function
 *                 will allocate the ifindex requested by the IfND from IAPS.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifd_intf_ifindex_cleanup (IFSV_EVT* evt, IFSV_CB *cb)
{  
   uns32 res = NCSCC_RC_SUCCESS;   
   uns32 res1 = NCSCC_RC_SUCCESS;   
   IFSV_INTF_REC       *rec = NULL;
   IFSV_INTF_DATA *rec_data = NULL;
   
   IFSV_EVT_SPT_MAP_INFO *spt_map = &evt->info.ifd_evt.info.spt_map;

   m_IFD_LOG_EVT_IFINDEX(IFSV_LOG_IFD_EVT_IFINDEX_CLEANUP_RCV,\
      spt_map->spt_map.if_index);
   /* Some times record may be added in IfD even if IfND gets TIME OUT
      during intf additioni req. So, delete it also if it is there.
      Afterall we want the same data base at all the places.
    */
    m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_EVT_IFINDEX_CLEANUP_RCV ,
                               "Ifindex : s/s/ss/p/t/s/ifindex",
                               spt_map->spt_map.spt.shelf,
                               spt_map->spt_map.spt.slot,
                               spt_map->spt_map.spt.subslot,
                               spt_map->spt_map.spt.port,
                               spt_map->spt_map.spt.type,
                               spt_map->spt_map.spt.subscr_scope,
                               spt_map->spt_map.if_index);
   rec_data = ifsv_intf_rec_find(spt_map->spt_map.if_index, cb); 
   if(rec_data != NULL)
   {
      rec = ifsv_intf_rec_del (spt_map->spt_map.if_index, cb);

      if (rec == IFSV_NULL)
         return NCSCC_RC_FAILURE;

      /* Send the trigger point to Standby IfD. */
      res1 = ifd_a2s_async_update(cb, IFD_A2S_DATA_DELETE_MSG,
                                         (uns8 *)(&rec->intf_data));
      /* Indicate others that it has been deleted. */
       ifd_bcast_to_ifnds(&rec->intf_data, IFSV_INTF_REC_DEL, 0,
                           cb);

      if (rec != NULL)
      {
        m_MMGR_FREE_IFSV_INTF_REC(rec);
      }
   }
   
   /* delete the slot port maping */
   res = ifsv_ifindex_spt_map_del(spt_map->spt_map.spt, cb);

   if(res == NCSCC_RC_SUCCESS)
   {
     /* Send the trigger point to Standby IfD. */
     res1 = ifd_a2s_async_update(cb, IFD_A2S_SPT_MAP_DELETE_MSG,
                               (uns8 *)(&spt_map->spt_map));
    
     res = ifsv_ifindex_free(cb, &spt_map->spt_map);

     if(res == NCSCC_RC_SUCCESS)
     {
       /* Send the trigger point to Standby IfD. */
       res1 = ifd_a2s_async_update(cb, IFD_A2S_IFINDEX_DEALLOC_MSG,
                                 (uns8 *)(&spt_map->spt_map.if_index));
     }
   }

   return (res);
}

/****************************************************************************
 * Name          : ifd_tmr_exp
 *
 * Description   : This event callback function which would be called when 
 *                 Interface aging timer is expired. This routine will 
 *                 delete the interface record and the interface maping info.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifd_tmr_exp (IFSV_EVT* evt, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_SUCCESS;
   uns32 ii;

   if(evt == NULL)
    return NCSCC_RC_FAILURE;
   switch(evt->info.ifd_evt.info.tmr_exp.tmr_type)
   {
     case NCS_IFSV_IFD_EVT_INTF_AGING_TMR:
     {
       uns32 ifindex = evt->info.ifd_evt.info.tmr_exp.info.ifindex;   
       NCS_IFSV_SPT_MAP    spt_map;
       IFSV_INTF_REC       *rec;
       IFSV_INTF_DATA *rec_data;
       do
       {
         m_IFD_LOG_EVT_IFINDEX(IFSV_LOG_IFD_EVT_AGING_TMR_RCV, ifindex);
         rec_data = ifsv_intf_rec_find(ifindex, cb);

         if(rec_data != NULL)
         {
          /* It might have happened that the interface had been coming UP and
           timer expired and a timer expiry event was lying in the mail box
           and the interface has become UP. */
          if(rec_data->marked_del == FALSE)
            return res;
         }

         rec = ifsv_intf_rec_del (ifindex, cb);
         if (rec != NULL)
         {
           m_IFSV_LOG_NORMAL(cb->my_svc_id,\
                        IFSV_LOG_IF_TBL_DEL_SUCCESS,ifindex,0);

           /* delete the slot port maping */
           res = ifsv_ifindex_spt_map_del(rec->intf_data.spt_info, cb);         
           if(res == NCSCC_RC_SUCCESS)
             m_IFSV_LOG_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
                   IFSV_LOG_IF_MAP_TBL_DEL_SUCCESS,rec->intf_data.spt_info.slot,rec->intf_data.spt_info.port);
           
           /* Send the trigger point to Standby IfD. */
           res = ifd_a2s_async_update(cb, IFD_A2S_DATA_DELETE_MSG,
                                         (uns8 *)(&rec->intf_data));

           /* free the ifindex */
           spt_map.spt = rec->intf_data.spt_info;
           spt_map.if_index = rec->intf_data.if_index;

           /* Send the trigger point to Standby IfD. */
           res = ifd_a2s_async_update(cb, IFD_A2S_SPT_MAP_DELETE_MSG,
                                      (uns8 *)(&spt_map));
           res = ifsv_ifindex_free(cb, &spt_map);
           m_MMGR_FREE_IFSV_INTF_REC(rec);
         }
       } while(0);
      break; 
     } /* case NCS_IFSV_IFD_EVT_INTF_AGING_TMR  */ 

     case NCS_IFSV_IFD_EVT_RET_TMR:
     {
       m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_RET_TMR_RCV,"Node id : ",evt->info.ifd_evt.info.tmr_exp.info.node_id);

       IFND_NODE_ID_INFO * node_id_info = NULL;
       /* Delete the intf record created by this IfND. */ 
       node_id_info = ifd_ifnd_node_id_info_get(evt->info.ifd_evt.info.tmr_exp.info.node_id, cb); 
       if(NULL == node_id_info)
       {
         res = NCSCC_RC_FAILURE;         
         return res;
       }
       /* Delete the all interface record created by the IfND. */ 
       ifd_same_node_id_all_intf_rec_del(node_id_info->ifnd_node_id, cb);
       /* Now delete the node from the patricia tree.*/
       res = ifd_ifnd_node_id_info_del(node_id_info->ifnd_node_id, cb); 

       break;
     } /* case NCS_IFSV_IFD_EVT_RET_TMR */ 

     case NCS_IFSV_IFD_IFND_REC_FLUSH_TMR:
     {
        printf("Recived IFND REC FLUSH TIMER expiry event \n");
        if(cb->ha_state == SA_AMF_HA_ACTIVE)
        {
           printf("State is Active Now \n");
           for (ii = 0; ii < MAX_IFND_NODES; ii++)
           {
              if (cb->ifnd_mds_addr[ii].valid == TRUE)
              {
                 /* Mark DEL to all interfaces created by this IfND. */
                 printf("Deleting the %dth Entry \n",ii);
                 ifd_same_dst_all_intf_rec_mark_del(&cb->ifnd_mds_addr[ii].ifndAddr,cb);
                 cb->ifnd_mds_addr[ii].valid = FALSE;
              }
           }
        }
        else 
        {
           printf("State is Standby \n");
        }
        memset(cb->ifnd_mds_addr,0,(MAX_IFND_NODES * sizeof(DOWN_IFND_ADDR)));
        cb->ifnd_rec_flush_tmr = IFSV_NULL;
        break;
     } 

     default:
     {
       res = NCSCC_RC_FAILURE;
       break;
     }
   } /* switch(evt->info.ifd_evt.info.tmr_exp.tmr_type)  */  

   return (res);
}

/*****e**********************************************************************
 * Name          : ifd_intf_rec_sync
 *
 * Description   : This event callback function which would be called when 
 *                 IfND sends a IFD_EVT_INTF_REC_SYNC event. When IfD
 *                 receives this event it would send all the interface record
 *                 which it has at present in the interface table to the 
 *                 requested IfND.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : At present we are sending record by record but later we 
 *                 need to pack multiple records in a same event and send.
 *****************************************************************************/
static uns32 
ifd_intf_rec_sync (IFSV_EVT* evt, IFSV_CB *cb)
{
   NODE_ID             node_id = 
            m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.ifd_evt.info.rec_sync.ifnd_vcard_id);
   MDS_DEST           dest = evt->info.ifd_evt.info.rec_sync.ifnd_vcard_id;
   uns32              res = NCSCC_RC_SUCCESS;
   uns32              res1 = NCSCC_RC_SUCCESS;
   IFND_NODE_ID_INFO *node_id_info = NULL;

   m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_REC_SYNC_RCV,"Node id : ",node_id);

   /* Send the trigger point to Standby IfD. */
   res1 = ifd_a2s_async_update(cb, IFD_A2S_IFND_UP_MSG, (uns8 *)(&node_id));

   /* Check whether any retention timer is running or not. */
   node_id_info = ifd_ifnd_node_id_info_get(node_id, cb);  
   if(node_id_info == NULL)   
   {
     /* There is no retention timer running. */
     res = ifd_intf_rec_send_no_ret_tmr(dest, cb);
     if(res != NCSCC_RC_SUCCESS)
     {
        m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_intf_rec_send_no_ret_tmr() returned failure"," ");
        return NCSCC_RC_FAILURE;
     }
   }
   else
   {
     /* Retention timer is running. So, stop the timer and delete the Patricia
        tree node.*/
     ifsv_tmr_stop(&node_id_info->tmr, cb); 
     ifd_ifnd_node_id_info_del(node_id, cb);

     res = ifd_intf_rec_send_ret_tmr_running(dest, cb);
     if(res != NCSCC_RC_SUCCESS)
     {
        m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_intf_rec_send_ret_tmr_running() returned failure"," ");
        return NCSCC_RC_FAILURE;
     }
   }

   return (res);
}
/****************************************************************************
 * Name          : get_next_info_no_ret_tmr
 *
 * Description   : This function retreives the next intf rec info for an 
 *                 interface for which no retention timer is running.
 *                 Interfaces for which retention timer is running need
 *                 not be sent to IFND.
 *
 * Notes         : None.
 *****************************************************************************/
void get_next_info_no_ret_tmr(IFSV_CB* cb,IFSV_INTF_REC **intf_rec,
                              #if(NCS_IFSV_IPXS == 1)
                                           IPXS_CB          *ipxs_cb,
                                           IPXS_IFIP_INFO   **ip_info,
                              #endif
                              NCS_IFSV_IFINDEX   *ifindex)
{
   NCS_PATRICIA_NODE  *if_node;
 #if(NCS_IFSV_IPXS == 1)
   IPXS_IFIP_NODE   *ifip_node = NULL;
 #endif
   do
   {
     NODE_ID node_id;
     IFND_NODE_ID_INFO *node_id_info = NULL;

     if_node = ncs_patricia_tree_getnext(&cb->if_tbl,
                                      (uns8*)ifindex);
     *intf_rec = (IFSV_INTF_REC*)if_node;

     #if(NCS_IFSV_IPXS == 1)
       ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                                   (&ipxs_cb->ifip_tbl, (uns8*)ifindex);
       *ip_info = (IPXS_IFIP_INFO *)&ifip_node->ifip_info;
    #endif

     if((*intf_rec == NULL)
     #if(NCS_IFSV_IPXS == 1)
      || (*ip_info == IFSV_NULL)
     #endif
      )
     {
      break;
     }

     *ifindex = (*intf_rec)->intf_data.if_index;
     node_id = m_NCS_NODE_ID_FROM_MDS_DEST(
                     (*intf_rec)->intf_data.current_owner_mds_destination);
     node_id_info = ifd_ifnd_node_id_info_get(node_id, cb);
     if(node_id_info == NULL)
     break;
    }while(1);

}

/****************************************************************************
 * Name          : ifd_intf_rec_send_no_ret_tmr
 *
 * Description   : This function is called when all data record has to be sent
 *                 to IfND, which has come up.
 *
 * Arguments     : mds_dest-This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_intf_rec_send_no_ret_tmr (MDS_DEST mds_dest, IFSV_CB *cb)
{
   NCS_LOCK           *intf_rec_lock = &cb->intf_rec_lock;
   IFSV_INTF_REC      *intf_rec;
   NCS_IFSV_IFINDEX   ifindex = 0;
   uns32 res = NCSCC_RC_SUCCESS;
   IFSV_EVT    *evt = NULL;
   NCS_BOOL no_data = TRUE; /* No data in the data base. */

#if(NCS_IFSV_IPXS == 1)
   IPXS_CB  *ipxs_cb;
   uns32    ipxs_hdl;
   IPXS_IFIP_INFO   *ip_info = NULL;
   NCS_IPXS_IPINFO  ipxs_ip_info;
#endif

   /* Allocate the event data structure */
   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
   {
      m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return NCSCC_RC_FAILURE;
   }

   memset(evt, 0, sizeof(IFSV_EVT));
   evt->type = IFND_EVT_INTF_CREATE;
   evt->vrid = cb->vrid;

#if(NCS_IFSV_IPXS == 1)
   /* Get the IPXS CB */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, ipxs_hdl);
   if (ipxs_cb == IFSV_NULL)
   {
      ifsv_evt_destroy(evt); 
      return NCSCC_RC_FAILURE;
   }
#endif

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);

   get_next_info_no_ret_tmr(cb,&intf_rec,ipxs_cb,
                         &ip_info,&ifindex);


   while ((intf_rec != IFSV_NULL)
#if(NCS_IFSV_IPXS == 1)
 && (ip_info != IFSV_NULL)
#endif
)
   {
      no_data = FALSE;
      ifindex = intf_rec->intf_data.if_index;

      memcpy(&evt->info.ifnd_evt.info.intf_create.intf_data, 
                   &intf_rec->intf_data, sizeof(IFSV_INTF_DATA));

      evt->info.ifnd_evt.info.intf_create.intf_data.no_data = FALSE;

      if(ncs_patricia_tree_getnext(&cb->if_tbl,
         (uns8*)&ifindex) == NULL)
      {
        /* Seems last message. */
        evt->info.ifnd_evt.info.intf_create.intf_data.last_msg = TRUE;
      }

      evt->info.ifnd_evt.info.intf_create.evt_orig = 
                                   NCS_IFSV_EVT_ORIGN_IFD;
      /* send a IfND create event to the particular IfND */
      res = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD, 
                                (NCSCONTEXT)evt, mds_dest, NCSMDS_SVC_ID_IFND);
#if(NCS_IFSV_IPXS == 1)
      /* This is IPXS block, data processing for IPXS. */
      {
        /* Send IPXS event to IfND. */
        IPXS_EVT          ipxs_evt;
        IFSV_EVT          evt;
        memset(&evt, 0, sizeof(IFSV_EVT));
        memset(&ipxs_evt, 0, sizeof(IPXS_EVT));

        /* Fill the pointers */
        evt.info.ipxs_evt = (NCSCONTEXT)&ipxs_evt;
        evt.type = IFSV_IPXS_EVT;
        ipxs_evt.type = IPXS_EVT_DTOND_IFIP_INFO;      
        ipxs_evt.info.nd.dtond_upd.if_index = ifindex;
        /* Fill the IPXS information. */
        res = ipxs_ifsv_ifip_info_attr_cpy(ip_info, &ipxs_ip_info);

        /* Copy the IP info & its internal pointers */
        memcpy(&ipxs_evt.info.nd.dtond_upd.ip_info, 
                                  &ipxs_ip_info, sizeof(NCS_IPXS_IPINFO));
        res = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD, 
                                (NCSCONTEXT)&evt, mds_dest, NCSMDS_SVC_ID_IFND);
      }
#endif

      get_next_info_no_ret_tmr(cb,&intf_rec,ipxs_cb,
                         &ip_info,&ifindex);

 
   }

   if(no_data)
   {
      /* If no data is there, then send empty message. */
      evt->info.ifnd_evt.info.intf_create.intf_data.no_data = TRUE;
      evt->info.ifnd_evt.info.intf_create.intf_data.last_msg = TRUE;

      evt->info.ifnd_evt.info.intf_create.evt_orig = 
                                   NCS_IFSV_EVT_ORIGN_IFD;
      /* send a IfND create event to the particular IfND */
      res = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD,
                                (NCSCONTEXT)evt, mds_dest, NCSMDS_SVC_ID_IFND);
   }

   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
   ifsv_evt_destroy(evt);
   m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_ALL_REC_SND, "Node id : ", m_NCS_NODE_ID_FROM_MDS_DEST(mds_dest));
   return (res);
} /* The function ifd_intf_rec_send_no_ret_tmr() ends here. */

/****************************************************************************
 * Name          : ifd_intf_rec_send_ret_tmr_running
 *
 * Description   : This function is called when data record has to be sent
 *                 to IfND, which has come up, while retention timer was 
 *                 running. It marks the operational state UP of those records
 *                 which have been created by this IfND and broadcast it. For
 *                 the records other than created by this IfND, it simply 
 *                 sends to the corresponding IfND.
 *
 * Arguments     : mds_dest-This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_intf_rec_send_ret_tmr_running (MDS_DEST mds_dest, IFSV_CB *cb)
{
   NCS_LOCK           *intf_rec_lock = &cb->intf_rec_lock;
   IFSV_INTF_REC      *intf_rec;
   NCS_PATRICIA_NODE  *if_node;
   NCS_IFSV_IFINDEX   ifindex = 0;
   uns32 res = NCSCC_RC_SUCCESS;
   IFSV_EVT    *evt = NULL;
   NCS_BOOL no_data = TRUE; /* No data in the data base. */

#if(NCS_IFSV_IPXS == 1)
   IPXS_CB  *ipxs_cb;
   uns32    ipxs_hdl;
   IPXS_IFIP_NODE   *ifip_node = NULL;
   IPXS_IFIP_INFO   *ip_info = NULL;
   NCS_IPXS_IPINFO  ipxs_ip_info;
#endif
   /* Allocate the event data structure */
   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
   {
      m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return NCSCC_RC_FAILURE;
   }

   memset(evt, 0, sizeof(IFSV_EVT));
   evt->type = IFND_EVT_INTF_CREATE;
   evt->vrid = cb->vrid;

#if(NCS_IFSV_IPXS == 1)
   /* Get the IPXS CB */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, ipxs_hdl);
   if (ipxs_cb == IFSV_NULL)
   {
      ifsv_evt_destroy(evt);
      return NCSCC_RC_FAILURE;
   }
#endif

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);

   if_node = ncs_patricia_tree_getnext(&cb->if_tbl,
      (uns8*)&ifindex);
   intf_rec = (IFSV_INTF_REC*)if_node;
   
#if(NCS_IFSV_IPXS == 1)
   ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                                  (&ipxs_cb->ifip_tbl, (uns8*)&ifindex);
   ip_info = (IPXS_IFIP_INFO *)&ifip_node->ifip_info;
#endif

   while ((intf_rec != IFSV_NULL)
#if(NCS_IFSV_IPXS == 1)
 && (ip_info != IFSV_NULL)
#endif
)
   {
      no_data = FALSE;
      ifindex = intf_rec->intf_data.if_index;
      if(m_NCS_NODE_ID_FROM_MDS_DEST(mds_dest) ==
         m_NCS_NODE_ID_FROM_MDS_DEST(intf_rec->intf_data.current_owner_mds_destination))
      {
       /* This intf rec has been created by this IfND. So, mark the Operational
          Status UP of this record and broadcast to all IfNDs. */
         if(intf_rec->intf_data.if_info.oper_state == NCS_STATUS_DOWN)
         {
           intf_rec->intf_data.if_info.oper_state = NCS_STATUS_UP;
         }
         else
         {
           m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
           return NCSCC_RC_FAILURE;
         }
         memcpy(&evt->info.ifnd_evt.info.intf_create.intf_data,
                      &intf_rec->intf_data, sizeof(IFSV_INTF_DATA));

         evt->info.ifnd_evt.info.intf_create.intf_data.no_data = FALSE;

         /* Broadcast IFSV_INTF_REC_ADD event to all IfNDs */
         res = ifd_bcast_to_ifnds(&intf_rec->intf_data, IFSV_INTF_REC_ADD, 0, 
                                  cb); 

         if(ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)&ifindex) == NULL)
         {
           /* Seems last message. */
           evt->info.ifnd_evt.info.intf_create.intf_data.last_msg = TRUE;
           /* send a IfND create event to the particular IfND. last_msg as TRUE
              should go to the IfND, which is getting rebooted. */
           evt->info.ifnd_evt.info.intf_create.evt_orig = 
                                   NCS_IFSV_EVT_ORIGN_IFD;
           res = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD,
                                (NCSCONTEXT)evt, mds_dest, NCSMDS_SVC_ID_IFND);
         }

#if(NCS_IFSV_IPXS == 1)
         /* This is IPXS block, data processing for IPXS. */
         {
            /* Send IPXS event to IfND. */
            IPXS_EVT          ipxs_evt;
            IFSV_EVT          evt;
            memset(&evt, 0, sizeof(IFSV_EVT));
            memset(&ipxs_evt, 0, sizeof(IPXS_EVT));

            /* Fill the pointers */
            evt.info.ipxs_evt = (NCSCONTEXT)&ipxs_evt;
            evt.type = IFSV_IPXS_EVT;
            ipxs_evt.type = IPXS_EVT_DTOND_IFIP_INFO;
            ipxs_evt.info.nd.dtond_upd.if_index = ifindex;
            /* Fill the IPXS information. */
            res = ipxs_ifsv_ifip_info_attr_cpy(ip_info, &ipxs_ip_info);

            /* Copy the IP info & its internal pointers */
            memcpy(&ipxs_evt.info.nd.dtond_upd.ip_info,
            &ipxs_ip_info, sizeof(NCS_IPXS_IPINFO));
            res = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD,
            (NCSCONTEXT)&evt, mds_dest, NCSMDS_SVC_ID_IFND);
         }
#endif
      }
      else
      {
        /* This intf has not been created by this IfND, so simply send to it */
        memcpy(&evt->info.ifnd_evt.info.intf_create.intf_data,
                    &intf_rec->intf_data, sizeof(IFSV_INTF_DATA));

        evt->info.ifnd_evt.info.intf_create.intf_data.no_data = FALSE;

        if(ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)&ifindex) == NULL)
        {
          /* Seems last message. */
          evt->info.ifnd_evt.info.intf_create.intf_data.last_msg = TRUE;
        }

        evt->info.ifnd_evt.info.intf_create.evt_orig = 
                                   NCS_IFSV_EVT_ORIGN_IFD;
        /* send a IfND create event to the particular IfND */
        res = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD,
                                (NCSCONTEXT)evt, mds_dest, NCSMDS_SVC_ID_IFND);
#if(NCS_IFSV_IPXS == 1)
        /* This is IPXS block, data processing for IPXS. */
        {
           /* Send IPXS event to IfND. */
           IPXS_EVT          ipxs_evt;
           IFSV_EVT          evt;
           memset(&evt, 0, sizeof(IFSV_EVT));
           memset(&ipxs_evt, 0, sizeof(IPXS_EVT));

           /* Fill the pointers */
           evt.info.ipxs_evt = (NCSCONTEXT)&ipxs_evt;
           evt.type = IFSV_IPXS_EVT;
           ipxs_evt.type = IPXS_EVT_DTOND_IFIP_INFO;
           ipxs_evt.info.nd.dtond_upd.if_index = ifindex;
           /* Fill the IPXS information. */
           res = ipxs_ifsv_ifip_info_attr_cpy(ip_info, &ipxs_ip_info);

           /* Copy the IP info & its internal pointers */
           memcpy(&ipxs_evt.info.nd.dtond_upd.ip_info,
           &ipxs_ip_info, sizeof(NCS_IPXS_IPINFO));
           res = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD,
           (NCSCONTEXT)&evt, mds_dest, NCSMDS_SVC_ID_IFND);
        }
#endif

      }

       if_node = ncs_patricia_tree_getnext(&cb->if_tbl,
          (uns8*)&ifindex);
       intf_rec = (IFSV_INTF_REC*)if_node;
#if(NCS_IFSV_IPXS == 1)
      ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                                  (&ipxs_cb->ifip_tbl, (uns8*)&ifindex);
      ip_info = (IPXS_IFIP_INFO *)&ifip_node->ifip_info;
#endif

   }/* While */

   if(no_data)
   {
      /* If no data is there, then send empty message. */
      evt->info.ifnd_evt.info.intf_create.intf_data.no_data = TRUE;
      evt->info.ifnd_evt.info.intf_create.intf_data.last_msg = TRUE;

      evt->info.ifnd_evt.info.intf_create.evt_orig =
                                   NCS_IFSV_EVT_ORIGN_IFD;
      /* send a IfND create event to the particular IfND */
      res = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD,
                                (NCSCONTEXT)evt, mds_dest, NCSMDS_SVC_ID_IFND);
   }

   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_ALL_REC_SND, "Node id : ", m_NCS_NODE_ID_FROM_MDS_DEST(mds_dest));
   if( evt != IFSV_NULL )
   {
      m_MMGR_FREE_IFSV_EVT(evt);
   }
   return (res);
} /* The function ifd_intf_rec_send_ret_tmr_running() ends here. */

/****************************************************************************
 * Name          : ifd_svcd_upd_from_ifnd_proc
 *
 * Description   : This event callback function which would be called IFD
 *                 Receives SVC-MDSDEST info from IFND
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 *****************************************************************************/
static uns32 
ifd_svcd_upd_from_ifnd_proc (IFSV_EVT* evt, IFSV_CB *cb)
{
   IFSV_EVT send_evt;
   uns32    rc = NCSCC_RC_SUCCESS;
   uns32    rc1 = NCSCC_RC_SUCCESS;

   m_IFD_LOG_EVT_LLL(IFSV_LOG_IFD_EVT_SVC_UPD_RCV,\
                      "Params i_type, i_ifindex and svcid are :",\
                      evt->info.ifd_evt.info.svcd.i_type,\
                      evt->info.ifd_evt.info.svcd.i_ifindex,\
                      evt->info.ifd_evt.info.svcd.i_svdest.svcid);

   rc = ifd_svcd_upd_from_ifnd_proc_data(evt, cb);
   
   if(rc == NCSCC_RC_SUCCESS)
    /* Send the trigger point to Standby IfD. */
    rc1 = ifd_a2s_async_update(cb, IFD_A2S_SVC_DEST_UPD_MSG,
                              (uns8 *)(&evt->info.ifd_evt.info.svcd));

    if(rc1 != NCSCC_RC_SUCCESS) 
    {
      m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_async_update() returned failure"," ");
     /* Log the error. */
    }

   /* Send the sync resp to IFND */
   memset(&send_evt, 0, sizeof(IFSV_EVT));


   if(rc == NCSCC_RC_SUCCESS)
   {
      send_evt.error = NCS_IFSV_NO_ERROR;
   }
   else
   {
      send_evt.error = NCS_IFSV_INT_ERROR;
   }

   send_evt.type = IFND_EVT_SVCD_UPD_RSP_FROM_IFD;
   send_evt.info.ifnd_evt.info.svcd = evt->info.ifd_evt.info.svcd;

   /* Sync resp to IfND.*/
   rc = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD,
                          &evt->sinfo, &send_evt);

   /* Send the information to IFND */
   memset(&send_evt, 0, sizeof(IFSV_EVT));

   send_evt.type = IFND_EVT_SVCD_UPD_FROM_IFD;
   send_evt.info.ifnd_evt.info.svcd = evt->info.ifd_evt.info.svcd;

   rc = ifsv_mds_bcast_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD, 
                      (NCSCONTEXT)&send_evt, NCSMDS_SVC_ID_IFND); 

   return (rc);
}

/****************************************************************************
 * Name          : ifd_svcd_upd_from_ifnd_proc_data
 *
 * Description   : This function updates the data base.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 *****************************************************************************/
uns32 
ifd_svcd_upd_from_ifnd_proc_data (IFSV_EVT* evt, IFSV_CB *cb)
{
   NCS_IFSV_SVC_DEST_UPD   *svc_dest=0;
   uns32                   ifindex, rc;
   IFSV_INTF_DATA          *intf_data=0;
   IFSV_INTF_REC           *intf_rec=0;
   
   svc_dest = &evt->info.ifnd_evt.info.svcd;

   if((svc_dest->i_type == NCS_IFSV_SVCD_ADD) &&
      (svc_dest->i_ifindex == 0))
   {
      /* Log the error */
      m_IFD_LOG_EVT_LL(IFSV_LOG_IFD_EVT_SVC_UPD_FAILURE, "svc_dest->i_type and svc_dest->i_ifindex are : ",svc_dest->i_type,svc_dest->i_ifindex);
      return NCSCC_RC_FAILURE;
   }

   ifindex = svc_dest->i_ifindex;

   if(ifindex)
   {
      intf_data = ifsv_intf_rec_find(ifindex, cb);
   }
   else
   {
      intf_rec = (IFSV_INTF_REC *)
               ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)NULL);

      if(intf_rec)
      {
         intf_data = &intf_rec->intf_data;
         ifindex = intf_data->if_index;
      }
   }

   while(intf_data)
   {

      rc = ifsv_modify_svcdest_list(svc_dest, intf_data, cb);

      if(rc != NCSCC_RC_SUCCESS)
        {  
         m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_modify_svcdest_list() returned failure"," ");
         return rc;
        }

      /* Get the next record */
      if(svc_dest->i_ifindex == 0)
      {
         intf_rec = (IFSV_INTF_REC *)
            ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)&ifindex);
         if(intf_rec)
         {
            intf_data = &intf_rec->intf_data;
            ifindex = intf_data->if_index;
         }
         else
            intf_data = NULL;

      }
      else
         intf_data = NULL;
   }

   m_IFD_LOG_HEAD_LINE_NORMAL(IFSV_LOG_SVCD_UPD_SUCCESS, 0, 0);

   return (rc);
} /* ifd_svcd_upd_from_ifnd_proc_data */

/****************************************************************************
 * Name          : ifd_process_evt
 *
 * Description   : This is the function which is called when ifd receives any
 *                 event. Depending on the IfND events it received, the 
 *                 corresponding callback will be called.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32
ifd_process_evt (IFSV_EVT* evt)
{
   IFSV_CB *ifsv_cb;
   uns32   ifsv_hdl, rc;
   IFSV_EVT_HANDLER ifsv_evt_hdl = IFSV_NULL;

   ifsv_hdl = evt->cb_hdl;

   if ((ifsv_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFD, ifsv_hdl))
            != IFSV_CB_NULL)
   {      
      if (ifsv_cb->init_done == FALSE)
      {
         /* since init is not done, don't allow any events other than
          * init done evt 
          */
         if (evt->type == IFD_EVT_INIT_DONE)            
         {
            ifsv_evt_hdl = ifd_evt_dispatch_tbl[evt->type - IFD_EVT_BASE];
            if (ifsv_evt_hdl != IFSV_NULL)
            {
               if (ifsv_evt_hdl(evt, ifsv_cb) != NCSCC_RC_SUCCESS)
               {
                m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_evt_hdl() returned failure"," ");
                  /* Log the event for the failure */
               }
            }
         }
      }else
      {
         if(evt->type < IFD_EVT_MAX)

         {
            ifsv_evt_hdl = ifd_evt_dispatch_tbl[evt->type - IFD_EVT_BASE];
            if (ifsv_evt_hdl != IFSV_NULL)
            {
               if ((ifsv_cb->ha_state == SA_AMF_HA_ACTIVE) || 
                   (evt->type == IFD_EVT_TMR_EXP))
               {
                  if (ifsv_evt_hdl(evt, ifsv_cb) != NCSCC_RC_SUCCESS)
                  {
                     m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_evt_hdl() returned failure"," ");
                     /* Log the event for the failure */
                  }
               }
            }
         }
         else if(evt->type == IFSV_IPXS_EVT) 
         {
            /* Process the IPXS Event */
            if(ifd_ipxs_evt_process(ifsv_cb, evt) != NCSCC_RC_SUCCESS)
            {
               /* Log the Event */
               m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_ipxs_evt_process() returned failure"," ");
               rc = NCSCC_RC_FAILURE;
            }
         }
#if (NCS_VIP == 1)
         else if((evt->type > IFSV_IPXS_EVT_MAX) && (evt->type < IFSV_VIP_EVT_MAX))
         {
            if (ifd_vip_evt_process(ifsv_cb,evt) != NCSCC_RC_SUCCESS)
            {
               /* log error message */
               m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_vip_evt_process() returned failure"," ");
               rc = NCSCC_RC_FAILURE;
            }
         }
#endif
      }
      ncshm_give_hdl(ifsv_hdl);
   }
   ifsv_evt_destroy(evt);
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifd_mds_msg_send_sync_resp
 *
 * Description   : This function used to send sync message from IfD to 
 *                 IfND's.
 *
 * Arguments     : msg        - This is the message pointer need to be send to the 
 *                              IfND/IfD.
 *                 msg_type   - event type.
 *                 send_info -  Information to send message.
 *                 ifsv_cb    - This is the IDIM control block.
 *                 error      - Error type
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifd_mds_msg_send_sync_resp (NCSCONTEXT msg, IFSV_EVT_TYPE msg_type, 
                       IFSV_SEND_INFO *send_info, IFSV_CB *ifsv_cb, 
                       NCS_IFSV_SYNC_SEND_ERROR error)
{
   IFSV_EVT  *evt;
   uns32 res = NCSCC_RC_FAILURE;
   char log_info[45];

   evt = m_MMGR_ALLOC_IFSV_EVT;

   if (evt == IFSV_NULL)
   {
      m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return (res);
   }
   
   memset(evt, 0, sizeof(IFSV_EVT));
   evt->type = msg_type;
   evt->error = error;
   evt->vrid = ifsv_cb->vrid;
   switch (msg_type)
   {
   case IFND_EVT_INTF_CREATE:
      memcpy(&evt->info.ifnd_evt.info.intf_create.intf_data, msg, 
         sizeof(IFSV_INTF_DATA));
      /* whatever IfD wants to publish it assumes that it is the owner of
       * it 
       */
      m_IFD_LOG_EVT_LL(IFSV_LOG_IFD_EVT_INTF_CREATE_SND,\
      ifsv_log_spt_string(((IFSV_INTF_CREATE_INFO*)msg)->intf_data.spt_info,log_info),\
      ((IFSV_INTF_CREATE_INFO*)msg)->if_attr, ((IFSV_INTF_CREATE_INFO*)msg)->intf_data.current_owner);

      break;
   case IFND_EVT_INTF_DESTROY:
      memcpy(&evt->info.ifnd_evt.info.intf_destroy, msg, 
         sizeof(IFSV_INTF_DESTROY_INFO));

      m_IFD_LOG_EVT_L(IFSV_LOG_IFD_EVT_INTF_DESTROY_RCV,\
      ifsv_log_spt_string(((IFSV_INTF_DESTROY_INFO*)msg)->spt_type,log_info),\
      m_NCS_NODE_ID_FROM_MDS_DEST( ((IFSV_INTF_DESTROY_INFO*)msg)->own_dest));

      break;
   case IFND_EVT_IFINDEX_RESP:
      memcpy(&evt->info.ifnd_evt.info.spt_map, msg, 
         sizeof(IFSV_EVT_SPT_MAP_INFO));

      m_IFD_LOG_EVT_LL(IFSV_LOG_IFD_EVT_IFINDEX_RESP_SND,\
        ifsv_log_spt_string(((IFSV_EVT_SPT_MAP_INFO*)msg)->spt_map.spt,log_info),\
        ((IFSV_EVT_SPT_MAP_INFO*)msg)->app_svc_id,\
        ((IFSV_EVT_SPT_MAP_INFO*)msg)->spt_map.if_index);

      break;         
   default:
      m_IFD_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT,"None",evt->type);         
      m_MMGR_FREE_IFSV_EVT(evt);
      break;
   }
   
   /* Send sync response to ifND. */
   res = ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl, NCSMDS_SVC_ID_IFD, send_info,
                           (NCSCONTEXT)evt);      
   
   ifsv_evt_destroy(evt);   

   return(res);
} /* The function ifd_mds_msg_send_sync_resp() ends here. */

/****************************************************************************
 * Name          : ifd_evt_send
 *
 * Description   : This function used to send a Event to IfD. This is the 
 *                 function which sends the event to itself.
 *
 * Arguments     : msg        - Even message to be send.
 *                 evt_type   - event type. 
 *                 ifsv_cb    - This is the IDIM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_evt_send (NCSCONTEXT msg, IFSV_EVT_TYPE evt_type, IFSV_CB *ifsv_cb)
{   
   IFSV_EVT  *evt;
   NCS_IPC_PRIORITY priority = NCS_IPC_PRIORITY_NORMAL;
   uns32 res = NCSCC_RC_FAILURE;

   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
   {
      m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return (res);
   }
   
   memset(evt, 0, sizeof(IFSV_EVT));
   evt->type = evt_type;
   evt->vrid = ifsv_cb->vrid;
   evt->cb_hdl = ifsv_cb->cb_hdl;

   switch(evt_type)
   {
      case IFD_EVT_INIT_DONE:
         memcpy(&evt->info.ifd_evt.info.init_done, msg, 
         sizeof(IFSV_EVT_INIT_DONE_INFO));
         m_IFD_LOG_EVT_INIT(IFSV_LOG_IFD_EVT_INIT_DONE_RCV,\
            evt->info.ifd_evt.info.init_done.init_done);
      break;
      default:
         break;
   }

   res = m_NCS_IPC_SEND(&ifsv_cb->mbx, evt, priority);   
   return (res);
}

/****************************************************************************\
 PROCEDURE NAME : ifd_quisced_process

 DESCRIPTION    : This routine process the Quisced ack event.

 ARGUMENTS      : cb - IFSV Control block pointer
                  quisced_info - IFD_QUISCED_STATE_INFO structure pointer

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
static uns32 ifd_quisced_process(IFSV_EVT* evt, IFSV_CB* cb)
{
  SaAisErrorT saErr= SA_AIS_OK; 
  uns32 rc= NCSCC_RC_SUCCESS;
  if(cb && cb->is_quisced_set && evt)
  {
     cb->ha_state = SA_AMF_HA_QUIESCED;
     rc= ifd_mbcsv_chgrole(cb);
     if(rc!= NCSCC_RC_SUCCESS)
     {
       m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mbcsv_chgrole() returned failure"," ");
       m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
     }
     saAmfResponse(cb->amf_hdl, cb->invocation, saErr);
     cb->is_quisced_set = FALSE;
  }
  return rc;
}

/****************************************************************************\
 PROCEDURE NAME : ifd_ifnd_down

 DESCRIPTION    : This routine process ifnd down event.

 ARGUMENTS      : cb - IFSV Control block pointer
                  evt - IFSV Event structure pointer

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
static uns32 ifd_ifnd_down(IFSV_EVT* evt, IFSV_CB* cb)
{
  uns32 rc= NCSCC_RC_SUCCESS;
  MDS_DEST mds_dest;

  if(cb && evt)
  {
    mds_dest = evt->info.ifd_evt.info.mds_dest;
    /* Mark DEL to all interfaces created by this IfND. */
    ifd_same_dst_all_intf_rec_mark_del(&mds_dest,cb);
  }

  return rc;
}

