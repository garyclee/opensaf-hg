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
  FILE NAME: IFND_EVT.C

  DESCRIPTION: IfND events received and send related routines.

  FUNCTIONS INCLUDED in this module:
  ifnd_intf_create_modify ............ Create or modify interface record.
  ifnd_intf_destroy .................. Destroy interface record.
  ifnd_intf_init_done .................IfND Init done.  
  ifnd_rcv_ifd_up_evt ................ Received IfD UP Event;
  ifnd_intf_info_get...... ............Get Interface Information (from IfA).
  ifnd_intf_stats_resp...... ..........Stats response got from the Hardware
  ifnd_tmr_exp............. ...........Any timer expiry.
  ifnd_process_evt......... ...........Process IfND Events.  
  ifnd_mds_msg_send......... ..........MDS msg send from IfND.
  ifnd_idim_evt_send......... .........Send Event to IDIM.
  ifnd_evt_send.............. .........Send Event to IfND.  

******************************************************************************/
#include "ifnd.h"

static uns32 ifnd_intf_create_modify(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_intf_destroy(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_intf_init_done(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_intf_info_get(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_intf_stats_resp(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_tmr_exp(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_svcd_upd_from_ifa_proc (IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_svcd_upd_from_ifd_proc (IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_svcd_get_proc (IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_ifd_down(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_ifa_down_in_oper_state(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_ifa_down_in_data_ret_state(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_ifa_down_in_mds_dest_get_state(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_ifdrv_down_in_oper_state(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_ifdrv_down_in_data_ret_state(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_ifdrv_down_in_mds_dest_state(IFSV_EVT* evt, IFSV_CB *cb);
static uns32 ifnd_ifd_up_process(IFSV_EVT* evt, IFSV_CB *cb);

extern uns32 ifnd_mds_ifd_up_event (IFSV_CB *ifsv_cb);

/* IfD dispatch table */
const IFSV_EVT_HANDLER ifnd_evt_dispatch_tbl[IFND_EVT_MAX - IFND_EVT_BASE] =
{
   ifnd_intf_create_modify,
   ifnd_intf_destroy,
   ifnd_intf_init_done,   
   ifnd_intf_info_get,
   ifnd_intf_stats_resp,
   ifnd_tmr_exp,
   ifnd_svcd_upd_from_ifa_proc,
   ifnd_svcd_upd_from_ifd_proc,
   ifnd_svcd_get_proc,
   ifnd_ifd_down,
   ifnd_ifa_down_in_oper_state,
   ifnd_ifa_down_in_data_ret_state,
   ifnd_ifa_down_in_mds_dest_get_state,
   ifnd_ifdrv_down_in_oper_state,
   ifnd_ifdrv_down_in_data_ret_state,
   ifnd_ifdrv_down_in_mds_dest_state,
   ifnd_ifd_up_process,
};

/****************************************************************************
 * Name          : ifnd_intf_create_modify
 *
 * Description   : This event callback function which would be called when 
 *                 IfND receives IFND_EVT_INTF_CREATE event. This function
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
ifnd_intf_create_modify (IFSV_EVT* evt, IFSV_CB *cb)
{  
   uns32 res = NCSCC_RC_FAILURE;
   IFSV_INTF_CREATE_INFO *intf_create;
/*   char log_info[45]; */
   IFSV_EVT send_evt;
   
   intf_create = &evt->info.ifnd_evt.info.intf_create;

   m_IFND_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFSV_CREATE_MOD_MSG ,
                                 "From : s/s/p/t/s/ originator",
                                 intf_create->intf_data.spt_info.shelf,
                                 intf_create->intf_data.spt_info.slot,
                                 intf_create->intf_data.spt_info.port,
                                 intf_create->intf_data.spt_info.type,
                                 intf_create->intf_data.spt_info.subscr_scope,
                                 intf_create->intf_data.originator,
                                 0);


   if(cb->ifnd_state == NCS_IFSV_IFND_DATA_RETRIVAL_STATE)
   {
       m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0);
       res = ifnd_data_retrival_from_ifd(cb, intf_create, &evt->sinfo);
   }  
   else if(cb->ifnd_state == NCS_IFSV_IFND_OPERATIONAL_STATE)
   {
       m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0); 
       res = ifnd_intf_create(cb, intf_create, &evt->sinfo);
   }
   else
   {
    if(intf_create->evt_orig == NCS_IFSV_EVT_ORIGN_IFA)
    {     
     /* Now send sync resp to the IfA. */
     m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IFSV_EVT));
     send_evt.type = IFA_EVT_INTF_CREATE_RSP;
     send_evt.error = NCS_IFSV_IFND_RESTARTING_ERROR;
     send_evt.info.ifa_evt.info.if_add_rsp_idx =
                                intf_create->intf_data.if_index;
     /* Sync resp */
     res = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                               &evt->sinfo, &send_evt);     
    }
   }
   return (res);
}

/****************************************************************************
 * Name          : ifnd_intf_destroy
 *
 * Description   : This event callback function which would be called when 
 *                 IfND receives IFND_EVT_INTF_DESTROY event. This function
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
ifnd_intf_destroy (IFSV_EVT* evt, IFSV_CB *cb)
{  
   uns32 res = NCSCC_RC_FAILURE;
   IFSV_INTF_DESTROY_INFO *dest_info;
   char log_info[45];
   IFSV_EVT send_evt;

   dest_info = &evt->info.ifnd_evt.info.intf_destroy;

   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0); 
   m_IFND_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFND_EVT_INTF_DESTROY_RCV ,
                    "s/s/p/t/s/ originator owner_dest",
                    dest_info->spt_type.shelf,
                    dest_info->spt_type.slot,
                    dest_info->spt_type.port,
                    dest_info->spt_type.type,
                    dest_info->spt_type.subscr_scope,
                    dest_info->orign,
                    dest_info->own_dest);
/*
   m_IFND_LOG_EVT_L(IFSV_LOG_IFND_EVT_INTF_DESTROY_RCV,\
      ifsv_log_spt_string(dest_info->spt_type,log_info),\
      m_NCS_NODE_ID_FROM_MDS_DEST(dest_info->own_dest));      
*/
   if(cb->ifnd_state != NCS_IFSV_IFND_OPERATIONAL_STATE)
   {
    if(dest_info->orign == NCS_IFSV_EVT_ORIGN_IFA)
    {     
     m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IFSV_EVT));
     /* Now send sync resp to the IfA. */
     send_evt.type = IFA_EVT_INTF_DESTROY_RSP;
     send_evt.error = NCS_IFSV_IFND_RESTARTING_ERROR;
     send_evt.info.ifa_evt.info.spt_map.spt_map.spt = dest_info->spt_type;

     /* Sync resp */
     res = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                        &evt->sinfo, &send_evt);
    }
   }
   else
   {
     res = ifnd_intf_delete(cb, dest_info, &evt->sinfo);
   }
   return (res);
}

/****************************************************************************
 * Name          : ifnd_intf_init_done
 *
 * Description   : This event callback function which would be called when 
 *                 IfND receives IFND_EVT_INIT_DONE event. This 
 *                 function set the init done variable to True and makes the
 *                 IfND operationally up. This function also informs the IfSv
 *                 driver module about the IfSv comming UP.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifnd_intf_init_done (IFSV_EVT* evt, IFSV_CB *cb)
{

   IFSV_EVT_INTF_REC_SYNC rec_sync;
   uns32                  res=NCSCC_RC_SUCCESS;

   cb->init_done = evt->info.ifnd_evt.info.init_done.init_done;
   
   do
   {
      m_IFND_LOG_EVT_INIT(IFSV_LOG_IFND_EVT_INIT_DONE_RCV, cb->init_done);   
      m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0);
      if (cb->init_done == TRUE)
      {
         /* need to spawn a IDIM thread */
         if (cb->idim_up == FALSE)
         {
            if (ifnd_idim_init(cb) != NCSCC_RC_SUCCESS)
            {
            /* give a alarm to AvSv to restart the IfND due to the failure of IDIM 
            * LOG the error). - TBD.
               */
               m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                     "IfND idim init failed : ifnd_intf_init_done() "," ");
 
               res  = NCSCC_RC_FAILURE;
               break;
            }
         }
         if((cb->ifnd_state == NCS_IFSV_IFND_DATA_RETRIVAL_STATE) ||
            (cb->ifnd_state == NCS_IFSV_IFND_OPERATIONAL_STATE))
         {
           /* This means that when IfND reached in this state, IfD was not UP*/
           /* Send a sync up event to IfD */
           rec_sync.ifnd_vcard_id = cb->my_dest;
         
           res = ifnd_mds_msg_send((NCSCONTEXT)&rec_sync, 
                                    IFD_EVT_INTF_REC_SYNC, cb);
           if (res != NCSCC_RC_SUCCESS)
           {
             /* LOG */
             m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                   "MDS msg send failed from IfND to IfD : ifnd_intf_init_done() "," ");
             res  = NCSCC_RC_FAILURE;
             break;
           }
         } /* if(cb->ifnd_state == NCS_IFSV_IFND_DATA_RETRIVAL_STATE) */
      } /* if (cb->init_done == TRUE) */
   } while(0);

   return (res);
}

/****************************************************************************
 * Name          : ifnd_rcv_ifd_up_evt
 *
 * Description   : This callback function which is called when IFD service 
 *                 is coming up. This function will check for the readiness 
 *                 state of the component and sends init event to itself.
 *
 * Arguments     : vcard_id  - This is the interace control block.
 *                 cb        - Ifnd Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifnd_rcv_ifd_up_evt (MDS_DEST *ifd_dest,IFSV_CB *cb)
{
   IFSV_EVT_INIT_DONE_INFO init_done;   
   uns32                   res = NCSCC_RC_SUCCESS;

   /* got the vcard info of the IfD, so store it in the CB */   
   cb->ifd_dest        = *ifd_dest;   

   if (cb->ifd_card_up == FALSE)
   {
      cb->ifd_card_up     = TRUE;
      init_done.init_done = TRUE;      
      res = ifnd_evt_send((NCSCONTEXT)&init_done, 
         IFND_EVT_INIT_DONE, cb);           
   } else
   {
      cb->ifd_card_up     = TRUE;      
   }
   
   return (res);
}

/****************************************************************************
 * Name          : ifnd_mds_evt
 *
 * Description   : This callback function which is called when IFD/IFA/IFDRV 
 *                 service down. It will send a message to itself. 
 *
 * Arguments     : vcard_id  - This is the interace control block.
 *                 cb        - Ifnd Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_mds_evt(MDS_DEST *ifd_dest, IFSV_CB *cb, IFSV_EVT_TYPE evt_type)
{
   MDS_DEST mds_dest;
   uns32 res = NCSCC_RC_SUCCESS;
      
   m_NCS_MEMCPY(&mds_dest, ifd_dest, sizeof(MDS_DEST));

   res = ifnd_evt_send((NCSCONTEXT)&mds_dest, evt_type, cb);

   return (res);
} /* The function ifnd_mds_evt() ends here. */

/****************************************************************************
 * Name          : ifnd_intf_info_get
 *
 * Description   : This function is called when IfA Requests for the interface
 *                 information of particular/all interface record(s)
 *
* Arguments     :  evt  - This is the pointer which holds the event structure.
 *                 cb   - Pointer to the interface control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifnd_intf_info_get (IFSV_EVT* evt, IFSV_CB *cb)
{   
   uns32 res = NCSCC_RC_SUCCESS;
   IFSV_IDIM_EVT_GET_HW_STATS_INFO  stats;
   IFSV_EVT_INTF_INFO_GET     *if_get=NULL;
   IFSV_EVT_STATS_INFO        stats_info;
   IFSV_SPT_REC               *spt_if_map;

   if_get = &evt->info.ifnd_evt.info.if_get;

   m_IFND_LOG_EVT_LL(IFSV_LOG_IFND_EVT_INTF_INFO_GET_RCV,\
                   "if_get->get_type and ifget->info_type are :", \
                    if_get->get_type, if_get->info_type);      
   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0);

   m_NCS_MEMSET(&stats, 0, sizeof(IFSV_IDIM_EVT_GET_HW_STATS_INFO));
 
   if (cb->idim_up == TRUE)
   {
      if(if_get->info_type == NCS_IFSV_IFSTATS_INFO)
      {
         /* Check for the state, go ahead only if IfND is in Oper state. */
         if(cb->ifnd_state != NCS_IFSV_IFND_OPERATIONAL_STATE)
         {
           goto stats_error;
         }
         /* service the request by sending a "IFSV_IDIM_EVT_GET_HW_STATS"
            event to IDIM */

         /* IDIM will fill these value in its resp */
         stats.app_svc_id  = if_get->my_svc_id;
         stats.app_dest    = if_get->my_dest;
         stats.usr_hdl     = if_get->usr_hdl;   

         /* Fill the SPT in the stats req info*/
         if(if_get->ifkey.type == NCS_IFSV_KEY_IFINDEX)
         {
            res =ifsv_get_spt_from_ifindex(if_get->ifkey.info.iface, 
                                                   &stats.slot_port, cb);
            if(res != NCSCC_RC_SUCCESS)
               goto stats_error;
            if_get->ifkey.info.spt = stats.slot_port;
         }
         else
         {
            stats.slot_port = if_get->ifkey.info.spt;
         }

         /* Before sending the event, validate the request */
         spt_if_map = (IFSV_SPT_REC *)ncs_patricia_tree_get(&cb->if_map_tbl, 
                                          (uns8 *)&if_get->ifkey.info.spt);

         /* Send the Request to IDIM only if the entry is present in the MAP Tbl */
         if(spt_if_map)
            res = ifnd_idim_evt_send(&stats, IFSV_IDIM_EVT_GET_HW_STATS, cb);
         else
            goto stats_error;

         if(res != NCSCC_RC_SUCCESS)
            goto stats_error;
         return res;
      }
   }
      if(if_get->info_type == NCS_IFSV_IF_INFO)
      {
         res = ifnd_intf_ifinfo_send(cb, if_get, &evt->sinfo);

         return res;
      }

#if (NCS_IFSV_BOND_INTF==1)
      if(if_get->info_type == NCS_IFSV_BIND_GET_LOCAL_INTF)
      {
         res = ifnd_intf_ifinfo_send(cb, if_get, &evt->sinfo);

         return res;
      }
#endif

stats_error:
   /* need to send an stats resp error to the Application*/
   m_NCS_MEMSET(&stats_info, 0, sizeof(stats_info));

   stats_info.status    = NCS_IFSV_IFGET_RSP_NO_REC;
   stats_info.dest      = if_get->my_dest;
   stats_info.svc_id    = if_get->my_svc_id;
   stats_info.usr_hdl   = if_get->usr_hdl;
   stats_info.spt_type  = if_get->ifkey.info.spt;

   res = ifnd_intf_statsinfo_send(cb, &stats_info);

   return (res);
}

/****************************************************************************
 * Name          : ifnd_intf_stats_resp
 *
 * Description   : This event callback function which would be called when 
 *                 IfND receives IFND_EVT_IDIM_STATS_RESP event. This
 *                 response got from IDIM for geting the latest statistics of
 *                 the interface.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifnd_intf_stats_resp (IFSV_EVT* evt, IFSV_CB *cb)
{
   IFSV_EVT_STATS_INFO *stats_info;
   char log_info[45];
   uns32  res = NCSCC_RC_SUCCESS;

   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0);

   /* Check for the state, go ahead only if IfND is in Oper state. */
   if(cb->ifnd_state != NCS_IFSV_IFND_OPERATIONAL_STATE)
   {
      m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
         "IfND state is not Operational : ifnd_intf_stats_resp() "," ");
      return NCSCC_RC_FAILURE;
   }

   stats_info = &evt->info.ifnd_evt.info.stats_info;

   /* Send the information to requested application */
   res = ifnd_intf_statsinfo_send(cb, stats_info);
   
   m_IFND_LOG_IF_STATS_INFO(IFSV_LOG_IF_STATS_INFO,\
      ifsv_log_spt_string(stats_info->spt_type,log_info),stats_info->status,\
      stats_info->stat_info.in_octs,stats_info->stat_info.in_dscrds,\
      stats_info->stat_info.out_octs,stats_info->stat_info.out_dscrds);
   
   return (res);
}

/****************************************************************************
 * Name          : ifnd_tmr_exp
 *
 * Description   : This event callback function which would be called when 
 *                 Interface aging timer is expired. This would delete the 
 *                 interface record form the IfND and free the allocated 
 *                 ifindex.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifnd_tmr_exp (IFSV_EVT* evt, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_SUCCESS;
   if(evt == NULL)
      return NCSCC_RC_FAILURE; 

   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0);

   switch(evt->info.ifd_evt.info.tmr_exp.tmr_type)
   { 
     case NCS_IFSV_IFND_EVT_INTF_AGING_TMR:
     {
       uns32 ifindex = evt->info.ifd_evt.info.tmr_exp.info.ifindex;
       NCS_IFSV_SPT_MAP    spt;
       IFSV_INTF_REC       *rec;
   
       do
       {
         m_IFND_LOG_EVT_IFINDEX(IFSV_LOG_IFND_EVT_AGING_TMR_RCV, ifindex);
         rec = ifsv_intf_rec_del (ifindex, cb);
         if (rec != NULL)
         {   
/* 
           m_IFSV_LOG_HEAD_LINE_NORMAL(cb->my_svc_id,\
                              IFSV_LOG_IF_TBL_DEL_SUCCESS,ifindex,0);
*/
           /* delete the slot port maping 
           This is being done in ifsv_ifindex_free below TBD
           */ 
           res = ifsv_ifindex_spt_map_del(rec->intf_data.spt_info, cb);
           if(res == NCSCC_RC_SUCCESS)
            m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
                IFSV_LOG_IF_MAP_TBL_DEL_SUCCESS,rec->intf_data.spt_info.slot,rec->intf_data.spt_info.port);
           /* free the ifindex */
           spt.spt = rec->intf_data.spt_info;
           spt.if_index = rec->intf_data.if_index;
         
           res = ifsv_ifindex_free (cb, &spt); 
           m_MMGR_FREE_IFSV_INTF_REC(rec);
         }
       } while(0);
      break;
     } /* case NCS_IFSV_IFND_EVT_INTF_AGING_TMR */  
     
     case NCS_IFSV_IFND_MDS_DEST_GET_TMR :
     {
       m_IFND_LOG_EVT_L(IFSV_LOG_IFND_EVT_MDS_DEST_GET_TMR_RCV,\
                       "None", 0);      
       if(cb->ifnd_state == NCS_IFSV_IFND_MDS_DEST_GET_STATE)
       {
          cb->ifnd_state = NCS_IFSV_IFND_DATA_RETRIVAL_STATE;
          /* Till now IfD must have come up, just checking for it. */
          if(cb->ifd_card_up != TRUE)
          {
            /* This means that IfD is not UP yet, so we would be dependent on
               IfD to come up. When IfD comes up, then ifnd_intf_init_done()
               will be called and this will invoke the SYNC process. */
               res = NCSCC_RC_SUCCESS; 
          }
          else
          {
            /* IfD is up, IDIM lib has been initialized. Now go ahead and 
               start the Sync process with it. */
            IFSV_EVT_INTF_REC_SYNC rec_sync;
            /* send a sync up event to IfD */
            rec_sync.ifnd_vcard_id = cb->my_dest;
            /* Send IfD a SYNC message. */
            res = ifnd_mds_msg_send((NCSCONTEXT)&rec_sync, 
                                     IFD_EVT_INTF_REC_SYNC, cb);
            if (res != NCSCC_RC_SUCCESS)
            {
              /* LOG */
              m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                "MDS Msg send failed from IfND to IfD : ifnd_tmr_exp() "," ");
              res  = NCSCC_RC_FAILURE;
            }
            
          }
       }/* if(cb->ifnd_state == NCS_IFSV_IFND_MDS_DEST_GET_STATE) */
       else
         res = NCSCC_RC_FAILURE;
      break;
     }
     
     default:
     {
       res = NCSCC_RC_FAILURE;
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                "Received Unknown Timer event type : ifnd_tmr_exp() "," ");
       m_IFND_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT, "Unknown Timer Event Type", 
                       evt->info.ifd_evt.info.tmr_exp.tmr_type);      
      break;
     }
   } /* switch(evt->info.ifd_evt.info.tmr_exp.tmr_type)  */ 

   return (res);
}

/****************************************************************************
 * Name          : ifnd_svcd_upd_from_ifa_proc
 *
 * Description   : This event callback function which would be called when 
 *                 SVC-MDS dest info is received from application
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifnd_svcd_upd_from_ifa_proc (IFSV_EVT* evt, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   IFSV_EVT send_evt, *o_evt;
   uns32 error = NCS_IFSV_NO_ERROR;

   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0);
   m_IFND_LOG_EVT_LLL(IFSV_LOG_IFND_EVT_SVCD_UPD_FROM_IFA_RCV,\
                      "Params i_type, i_ifindex and svcid are :",\
                      evt->info.ifnd_evt.info.svcd.i_type,\
                      evt->info.ifnd_evt.info.svcd.i_ifindex,\
                      evt->info.ifnd_evt.info.svcd.i_svdest.svcid);
   
   if(cb->ifnd_state != NCS_IFSV_IFND_OPERATIONAL_STATE)
   {

    /* Send the information to IFD */
     m_NCS_MEMSET(&send_evt, 0, sizeof(IFSV_EVT));
     send_evt.type = IFA_EVT_SVCD_UPD_RSP;
     send_evt.error = NCS_IFSV_IFND_RESTARTING_ERROR;
     send_evt.info.ifa_evt.info.svcd_sync_resp = evt->info.ifnd_evt.info.svcd;

    /* Sync resp */
    res = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                            &evt->sinfo, &send_evt);
    return res;

   }

   if((evt->info.ifnd_evt.info.svcd.i_type == NCS_IFSV_SVCD_ADD) ||
       (evt->info.ifnd_evt.info.svcd.i_type == NCS_IFSV_SVCD_DEL))
   {
     /* Send the information to IFD */
     m_NCS_MEMSET(&send_evt, 0, sizeof(IFSV_EVT));

     send_evt.type = IFD_EVT_SVCD_UPD;
     send_evt.info.ifd_evt.info.svcd = evt->info.ifnd_evt.info.svcd;

     /* send the information to the IfD */
     res = ifsv_mds_msg_sync_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                              NCSMDS_SVC_ID_IFD, cb->ifd_dest,
                              (NCSCONTEXT)&send_evt, (NCSCONTEXT)&o_evt,
                              IFSV_MDS_SYNC_TIME);


     if(res != NCSCC_RC_SUCCESS)
     {
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                "MDS Sync Send Failed from IfND to IfD : ifnd_svcd_upd_from_ifa_proc() "," ");
       if(res == NCSCC_RC_FAILURE)
          error = NCS_IFSV_EXT_ERROR;
       else
          error = NCS_IFSV_SYNC_TIMEOUT;
       /* We could not get the index. */
       res = NCSCC_RC_FAILURE;
     }
     else
     {
      if(o_evt->error == NCS_IFSV_NO_ERROR)
      {
        /* If no error, then no problem, the sync resp will be sent at the end*/
        error = NCS_IFSV_NO_ERROR;
      }
      else
      {
        /* We could not get the correct response. So, send the error
           back to IfA. */
         error = o_evt->error;
         res = NCSCC_RC_FAILURE;
      }
    }   

    send_evt.type = IFA_EVT_SVCD_UPD_RSP;
    send_evt.error = error;
    send_evt.info.ifa_evt.info.svcd_sync_resp = evt->info.ifnd_evt.info.svcd;

    /* Sync resp */
    res = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                            &evt->sinfo, &send_evt);
   }

   return res;
}

/****************************************************************************
 * Name          : ifnd_svcd_upd_from_ifd_proc
 *
 * Description   : This event callback function which would be called when 
 *                 SVC-MDS dest info is received from IFD
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifnd_svcd_upd_from_ifd_proc (IFSV_EVT* evt, IFSV_CB *cb)
{
   NCS_IFSV_SVC_DEST_UPD   *svc_dest=0;
   uns32                   ifindex, rc=NCSCC_RC_SUCCESS;
   IFSV_INTF_DATA          *intf_data=0;
   IFSV_INTF_REC           *intf_rec=0;
   
   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0);
   m_IFND_LOG_EVT_LLL(IFSV_LOG_IFND_EVT_SVCD_UPD_FROM_IFD_RCV,\
                      "Params i_type, i_ifindex, svcid are :",\
                      evt->info.ifnd_evt.info.svcd.i_type,\
                      evt->info.ifnd_evt.info.svcd.i_ifindex,\
                      evt->info.ifnd_evt.info.svcd.i_svdest.svcid);
   
   /* Check for the state, go ahead only if IfND is in Oper state. */
   if(cb->ifnd_state != NCS_IFSV_IFND_OPERATIONAL_STATE)
   {
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                "IfND State is not Operational : ifnd_svcd_upd_from_ifd_proc() "," ");
      return NCSCC_RC_FAILURE;
   }

   svc_dest = &evt->info.ifnd_evt.info.svcd;

   if((svc_dest->i_type == NCS_IFSV_SVCD_ADD) &&
      (svc_dest->i_ifindex == 0))
   {
      /* Log the error */
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
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                "IfSv modify serv dest list faile  : ifnd_svcd_upd_from_ifd_proc() "," ");
         return rc;
      }

      rc = ifsv_ifa_app_svd_info_indicate(cb, intf_data, svc_dest);
   
      if(rc != NCSCC_RC_SUCCESS)
      {
         /* Go ahead and clean the rest of the things */
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                "Ifsv ifa app svd info indicate failed : ifnd_svcd_upd_from_ifd_proc() "," ");
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
   
   return rc;
}


/****************************************************************************
 * Name          : ifnd_svcd_get_proc
 *
 * Description   : This event callback function which would be called when 
 *                 IFND receives SVCD get request
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifnd_svcd_get_proc (IFSV_EVT* evt, IFSV_CB *cb)
{
   NCS_IFSV_SVC_DEST_GET   *svc_get=0;
   uns32                   ifindex, rc, i;
   IFSV_INTF_DATA          *rec_data;
   IFSV_EVT                send_evt;
   
   svc_get = &evt->info.ifnd_evt.info.svcd_get;

   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0);
   m_IFND_LOG_EVT_LLL(IFSV_LOG_IFND_EVT_SVCD_GET_REQ_RCV,\
                      "Params are i_ifindex, i_svcid and node_id :",\
                      evt->info.ifnd_evt.info.svcd_get.i_ifindex,\
                      evt->info.ifnd_evt.info.svcd_get.i_svcid,\
                      m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.ifnd_evt.info.svcd_get.o_dest));
   
   m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IFSV_EVT));

   send_evt.type = IFA_EVT_SVCD_GET_RSP;
   send_evt.error = NCS_IFSV_NO_ERROR;
   send_evt.info.ifa_evt.info.svcd_rsp.i_ifindex = svc_get->i_ifindex;
   send_evt.info.ifa_evt.info.svcd_rsp.i_svcid = svc_get->i_svcid;
   send_evt.info.ifa_evt.info.svcd_rsp.o_answer = FALSE;

   /* Check for the state, go ahead only if IfND is in Oper state. */
   if(cb->ifnd_state != NCS_IFSV_IFND_OPERATIONAL_STATE)
   {
     send_evt.error = NCS_IFSV_IFND_RESTARTING_ERROR;
     goto send_rsp;
   }

   if(svc_get->i_ifindex == 0)
   {
      goto send_rsp;
   }

  ifindex = svc_get->i_ifindex;

  rec_data = ifsv_intf_rec_find(ifindex, cb);

  if(!rec_data)
  {
      goto send_rsp;
  }

  for(i=0; i<rec_data->if_info.addsvd_cnt; i++)
  {
      if(svc_get->i_svcid == rec_data->if_info.addsvd_list[i].svcid)
      {
         send_evt.info.ifa_evt.info.svcd_rsp.o_answer = TRUE;
         send_evt.info.ifa_evt.info.svcd_rsp.o_dest
                         = rec_data->if_info.addsvd_list[i].dest;
         break;
      }
  }

send_rsp:
   /* Sync resp */
   rc = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, &evt->sinfo, &send_evt);

   return (rc);
}

/****************************************************************************
 * Name          : ifnd_ifd_down
 *
 * Description   : This event callback function which would be called when
 *                 IFND receives IFND down request.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_ifd_down(IFSV_EVT* evt, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   res = ifnd_all_intf_rec_del(cb);
   return res;
} /* The function ifnd_ifd_down() ends here. */

/****************************************************************************
 * Name          : ifnd_ifa_down_in_oper_state
 *
 * Description   : This event callback function which would be called when
 *                 IFND receives IFA down request in OPER STATE.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_ifa_down_in_oper_state(IFSV_EVT* evt, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   ifnd_same_dst_intf_rec_del(&evt->info.ifnd_evt.info.mds_svc_info.
                              mds_dest, cb); 
   return res;
} /* The function ifnd_ifa_down_in_oper_state() ends here. */

/****************************************************************************
 * Name          : ifnd_ifa_down_in_data_ret_state
 *
 * Description   : This event callback function which would be called when
 *                 IFND receives IFA down request in DATA RETRIEVAL STATE.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_ifa_down_in_data_ret_state(IFSV_EVT* evt, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   ifnd_ifa_same_dst_intf_rec_and_mds_dest_del(&evt->info.ifnd_evt.info.
                            mds_svc_info.mds_dest, cb); 
   return res;
} /* The function ifnd_ifa_down_in_data_ret_state() ends here. */

/****************************************************************************
 * Name          : ifnd_ifa_down_in_mds_dest_get_state
 *
 * Description   : This event callback function which would be called when
 *                 IFND receives IFA down request in MDS DEST GET STATE.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_ifa_down_in_mds_dest_get_state(IFSV_EVT* evt, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   ifnd_mds_dest_del(evt->info.ifnd_evt.info.mds_svc_info.mds_dest, cb);
   return res;
} /* The function ifnd_ifa_down_in_mds_dest_get_state() ends here. */

/****************************************************************************
 * Name          : ifnd_ifdrv_down_in_oper_state
 *
 * Description   : This event callback function which would be called when
 *                 IFND receives IFDRV down request in OPER STATE.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_ifdrv_down_in_oper_state(IFSV_EVT* evt, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   ifnd_same_drv_intf_rec_del(cb);
   return res;
} /* The function ifnd_ifdrv_down_in_oper_state() ends here. */

/****************************************************************************
 * Name          : ifnd_ifdrv_down_in_data_ret_state
 *
 * Description   : This event callback function which would be called when
 *                 IFND receives IFDRV down request in DATA RETRIEVAL STATE.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_ifdrv_down_in_data_ret_state(IFSV_EVT* evt, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   ifnd_drv_same_dst_intf_rec_and_mds_dest_del(&evt->info.ifnd_evt.info.
                            mds_svc_info.mds_dest, cb);
   return res;
} /* The function ifnd_ifdrv_down_in_data_ret_state() ends here. */

/****************************************************************************
 * Name          : ifnd_ifdrv_down_in_mds_dest_state
 *
 * Description   : This event callback function which would be called when
 *                 IFND receives IFDRV down request in MDS DEST GET STATE.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_ifdrv_down_in_mds_dest_state(IFSV_EVT* evt, IFSV_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   ifnd_mds_dest_del(evt->info.ifnd_evt.info.mds_svc_info.mds_dest, cb);
   return res;
} /* The function ifnd_ifdrv_down_in_mds_dest_state() ends here. */

/****************************************************************************
 * Name          : ifnd_process_evt
 *
 * Description   : This is the function which is called when IfND receives any
 *                 event. Depending on the IfD/IfA/IDIM events it received, the
 *                 corresponding callback will be called.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32
ifnd_process_evt (IFSV_EVT* evt)
{
   IFSV_CB *cb;
   uns32   ifsv_hdl;
   uns32 rc = NCSCC_RC_SUCCESS;
   IFSV_EVT_HANDLER ifsv_evt_hdl = IFSV_NULL;

   ifsv_hdl = evt->cb_hdl;

   if ((cb = (IFSV_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFND, ifsv_hdl))
            != IFSV_CB_NULL)
   {  
      if(evt->type < IFSV_EVT_MAX)
      {
            ifsv_evt_hdl = 
               ifnd_evt_dispatch_tbl[evt->type - IFND_EVT_BASE];
            if (ifsv_evt_hdl != IFSV_NULL)
            {
               if (ifsv_evt_hdl(evt, cb) != NCSCC_RC_SUCCESS)
               {
                  /* Log the event for the failure */
                  rc = NCSCC_RC_FAILURE;
               }
            }
      }
#if(NCS_IFSV_IPXS == 1)
      else if(evt->type == IFSV_IPXS_EVT) 
      {
            /* Process the IPXS Event */
            if(ifnd_ipxs_evt_process(cb, evt) != NCSCC_RC_SUCCESS)
            {
               /* Log the Event */
               rc = NCSCC_RC_FAILURE;
            }
       }
#endif
#if (NCS_VIP == 1)
         else if((evt->type > IFSV_IPXS_EVT_MAX) && (evt->type <= IFSV_VIP_EVT_MAX))
         {

            if (ifnd_vip_evt_process(cb,evt) != NCSCC_RC_SUCCESS)
            {
               /* log error message */
               rc = NCSCC_RC_FAILURE;
            }
         }
#endif
      ncshm_give_hdl(ifsv_hdl);
   }
   ifsv_evt_destroy(evt);
   return(rc);
}

/****************************************************************************
 * Name          : ifnd_mds_msg_send
 *
 * Description   : This function used to send a message from IfND to 
 *                 IfD.
 *
 * Arguments     : msg      - This is the message pointer need to be send to the 
 *                            IfD.
 *                 msg_type - event type.
 *                 ifsv_cb  - This is the IDIM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_mds_msg_send (NCSCONTEXT msg, IFSV_EVT_TYPE msg_type, IFSV_CB *ifsv_cb)
{
   IFSV_EVT  *evt;
   SS_SVC_ID to_svc = NCSMDS_SVC_ID_IFD;
   MDS_DEST  to_dest = ifsv_cb->ifd_dest;
   uns32 res;

   evt = m_MMGR_ALLOC_IFSV_EVT;

   if (evt == IFSV_NULL)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return (NCSCC_RC_FAILURE);
   }
   
   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));
   evt->type = msg_type;
   evt->vrid = ifsv_cb->vrid;

   switch (msg_type)
   {
   case IFD_EVT_IFINDEX_CLEANUP:
      m_NCS_MEMCPY(&evt->info.ifd_evt.info.spt_map, msg, 
         sizeof(IFSV_EVT_SPT_MAP_INFO));

      m_IFND_LOG_EVT_IFINDEX(IFSV_LOG_IFND_EVT_IFINDEX_CLEANUP_SND,\
      ((IFSV_EVT_SPT_MAP_INFO*)msg)->spt_map.if_index);

      break;
   case IFD_EVT_INTF_REC_SYNC:
      m_NCS_MEMCPY(&evt->info.ifd_evt.info.rec_sync, msg, 
         sizeof(IFSV_EVT_INTF_REC_SYNC));

      m_IFND_LOG_EVT_INIT(IFSV_LOG_IFND_EVT_REC_SYNC_SND,\
      m_NCS_NODE_ID_FROM_MDS_DEST(((IFSV_EVT_INTF_REC_SYNC*)msg)->ifnd_vcard_id));

      break;      
   default:
      m_IFND_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT,"Unknwon msg type to send msg from IfND to IfD", evt->type);
      m_MMGR_FREE_IFSV_EVT(evt);
      return (NCSCC_RC_FAILURE);
      break;
   }

   res = ifsv_mds_normal_send(ifsv_cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, 
      (NCSCONTEXT)evt, to_dest, to_svc);   

   /* free the event allocated */
   m_MMGR_FREE_IFSV_EVT(evt);
   return(res);
}

/****************************************************************************
 * Name          : ifnd_mds_msg_sync_send
 *
 * Description   : This function used to send sync message from IfND to 
 *                 IfD.
 *
 * Arguments     : msg      - This is the message pointer need to be send to the 
 *                            IfD.
 *                 msg_type - event type.
 *                 ifsv_cb  - This is the IDIM control block.
 *                 o_evt    - Pointer to message returned in sync req.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_mds_msg_sync_send (NCSCONTEXT msg, IFSV_EVT_TYPE msg_type, 
                        IFSV_CB *ifsv_cb, NCSCONTEXT *o_evt)
{
   IFSV_EVT  *evt = NULL;
   uns32 res;
   char log_info[45];
   

   evt = m_MMGR_ALLOC_IFSV_EVT;

   if (evt == IFSV_NULL)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return (NCSCC_RC_FAILURE);
   }
   
   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));
   evt->type = msg_type;
   evt->vrid = ifsv_cb->vrid;

   switch (msg_type)
   {
   case IFD_EVT_IFINDEX_REQ:
      m_NCS_MEMCPY(&evt->info.ifd_evt.info.spt_map, msg,
         sizeof(IFSV_EVT_SPT_MAP_INFO));

      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_EVT_IFINDEX_REQ_SND,\
      ifsv_log_spt_string(((IFSV_EVT_SPT_MAP_INFO*)msg)->spt_map.spt,log_info),\
      m_NCS_NODE_ID_FROM_MDS_DEST(((IFSV_EVT_SPT_MAP_INFO*)msg)->app_dest));

      break;
   default:
      m_IFND_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT, "Unknwon msg type to send sync msg from IfND to IfD", evt->type);
      m_MMGR_FREE_IFSV_EVT(evt);
      return (NCSCC_RC_FAILURE);
      break;
   }

   res = ifsv_mds_msg_sync_send(ifsv_cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, 
                              NCSMDS_SVC_ID_IFD, ifsv_cb->ifd_dest, 
                              (NCSCONTEXT)evt,(IFSV_EVT **) o_evt, 
                              IFSV_MDS_SYNC_TIME);   
   /* free the event allocated */
   m_MMGR_FREE_IFSV_EVT(evt);
   return(res);
} /* The function ifnd_mds_msg_sync_send() ends here.*/
#if 0
/****************************************************************************
 * Name          : ifnd_mds_msg_sync_resp_send
 *
 * Description   : This function used to send sync resp from IfND to IfA.
 *
 * Arguments     : msg      - This is the message pointer need to be send to the 
 *                            IfD.
 *                 msg_type - event type.
 *                 ifsv_cb  - This is the IDIM control block.
 *                 info     - Pointer to send information.
 *                 error    - Type of error to be sent.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_mds_msg_sync_resp_send (NCSCONTEXT msg, IFSV_EVT_TYPE msg_type, 
                             IFSV_CB *ifsv_cb, IFSV_SEND_INFO *sinfo,
                             NCS_IFSV_SYNC_SEND_ERROR error)
{
   IFSV_EVT  *evt;
   SS_SVC_ID to_svc = NCSMDS_SVC_ID_IFD;
   MDS_DEST  to_dest = ifsv_cb->ifd_dest;
   uns32 res;
   char log_info[45];

   evt = m_MMGR_ALLOC_IFSV_EVT;

   if (evt == IFSV_NULL)
   {
      /* memory failed - raise an alarm - LOG */
      return (NCSCC_RC_FAILURE);
   }
   
   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));
   evt->type = msg_type;
   evt->vrid = ifsv_cb->vrid;

   if(error)
    evt->error = error;
   else
    evt->error = NCS_IFSV_NO_ERROR;

   switch (msg_type)
   {
   case IFA_EVT_INTF_CREATE_RSP:
      m_NCS_MEMCPY(&evt->info.ifd_evt.info.intf_create.intf_data, msg, 
         sizeof(IFSV_INTF_DATA));

      m_IFND_LOG_EVT_LL(IFSV_LOG_IFND_EVT_INTF_CREATE_SND,\
      ifsv_log_spt_string(((IFSV_INTF_CREATE_INFO*)msg)->intf_data.spt_info,log_info),\
      ((IFSV_INTF_CREATE_INFO*)msg)->if_attr, ((IFSV_INTF_CREATE_INFO*)msg)->intf_data.current_owner);

      break;
   case IFD_EVT_INTF_DESTROY_RSP:
      m_NCS_MEMCPY(&evt->info.ifd_evt.info.intf_destroy, msg, 
         sizeof(IFSV_INTF_DESTROY_INFO));

      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_EVT_INTF_DESTROY_RCV,\
      ifsv_log_spt_string(((IFSV_INTF_DESTROY_INFO*)msg)->spt_type,log_info),\
      m_NCS_NODE_ID_FROM_MDS_DEST(((IFSV_INTF_DESTROY_INFO*)msg)->own_dest));

      break;
   case IFD_EVT_IFINDEX_REQ:
      m_NCS_MEMCPY(&evt->info.ifd_evt.info.spt_map, msg,
         sizeof(IFSV_EVT_SPT_MAP_INFO));

      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_EVT_IFINDEX_REQ_SND,\
      ifsv_log_spt_string(((IFSV_EVT_SPT_MAP_INFO*)msg)->spt_map.spt,log_info),\
      m_NCS_NODE_ID_FROM_MDS_DEST(((IFSV_EVT_SPT_MAP_INFO*)msg)->app_dest));

      break;
   case IFD_EVT_IFINDEX_CLEANUP:
      m_NCS_MEMCPY(&evt->info.ifd_evt.info.spt_map, msg, 
         sizeof(IFSV_EVT_SPT_MAP_INFO));

      m_IFND_LOG_EVT_IFINDEX(IFSV_LOG_IFND_EVT_IFINDEX_CLEANUP_SND,\
      ((IFSV_EVT_SPT_MAP_INFO*)msg)->spt_map.if_index);

      break;
   case IFD_EVT_INTF_REC_SYNC:
      m_NCS_MEMCPY(&evt->info.ifd_evt.info.rec_sync, msg, 
         sizeof(IFSV_EVT_INTF_REC_SYNC));

      m_IFND_LOG_EVT_INIT(IFSV_LOG_IFND_EVT_REC_SYNC_SND,\
      m_NCS_NODE_ID_FROM_MDS_DEST(((IFSV_EVT_INTF_REC_SYNC*)msg)->ifnd_vcard_id));

      break;      
   default:
      m_IFND_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT,"None",evt->type);
      m_MMGR_FREE_IFSV_EVT(evt);
      return (NCSCC_RC_FAILURE);
      break;
   }

   res = ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, sinfo,
                           evt); 

   /* free the event allocated */
   m_MMGR_FREE_IFSV_EVT(evt);
   return(res);
} /* The function ifnd_mds_msg_sync_resp_send() ends here.*/
#endif

/****************************************************************************
 * Name          : ifnd_idim_evt_send
 *
 * Description   : This function used to send an event to IDIM.
 *
 * Arguments     : msg      - This is the message pointer need to be send to the 
 *                            IDIM
 *                 msg_type - event type.
 *                 ifsv_cb  - This is the IFSV control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_idim_evt_send (NCSCONTEXT msg, IFSV_IDIM_EVT_TYPE msg_type, 
                         IFSV_CB *ifsv_cb)
{
   IFSV_IDIM_EVT *evt;
   NCS_IPC_PRIORITY priority;
   uns32 res = NCSCC_RC_FAILURE;
   char log_info[45];

   evt = m_MMGR_ALLOC_IFSV_IDIM_EVT;
   if (evt == IFSV_NULL)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,\
                  0);
      return (res);
   }
   m_NCS_MEMSET(evt,0,sizeof(IFSV_IDIM_EVT));
   evt->evt_type = msg_type;

   switch (msg_type)
   {
   case IFSV_IDIM_EVT_HEALTH_CHK:
      m_NCS_MEMCPY(&evt->info.health_chk, msg, sizeof(NCS_IFSV_HW_INFO));
      priority = NCS_IPC_PRIORITY_HIGH;

      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_EVT_IDIM_HEALTH_CHK_SND,"None",evt->evt_type);

      break;
   case IFSV_IDIM_EVT_GET_HW_STATS:
      m_NCS_MEMCPY(&evt->info.get_stats, msg, 
         sizeof(IFSV_IDIM_EVT_GET_HW_STATS_INFO));
      priority = NCS_IPC_PRIORITY_NORMAL;

      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_EVT_GET_HW_STATS_SND,\
         ifsv_log_spt_string(((IFSV_IDIM_EVT_GET_HW_STATS_INFO*)msg)->slot_port,log_info),\
         m_NCS_NODE_ID_FROM_MDS_DEST(((IFSV_IDIM_EVT_GET_HW_STATS_INFO*)msg)->app_dest));

      break;   
   case IFSV_IDIM_EVT_SET_HW_PARM:
      m_NCS_MEMCPY(&evt->info.set_hw_parm, msg, 
         sizeof(IFSV_IDIM_EVT_SET_HW_PARAM_INFO));
      priority = NCS_IPC_PRIORITY_NORMAL;
      
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_EVT_SET_HW_PARAM_SND,\
         ifsv_log_spt_string(((IFSV_IDIM_EVT_SET_HW_PARAM_INFO*)msg)->slot_port,log_info),\
         ((IFSV_IDIM_EVT_SET_HW_PARAM_INFO*)msg)->hw_param.if_am);
      
      break;   
   default:
      m_IFND_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT,"IDIM Event",evt->evt_type);
      m_MMGR_FREE_IFSV_IDIM_EVT(evt);
      return(NCSCC_RC_FAILURE);
      break;
   }
   /* post the event to the IDIM thread */
   res = m_NCS_IPC_SEND(&ifsv_cb->idim_mbx, evt, priority);   
   return(res);
}

/****************************************************************************
 * Name          : ifnd_evt_send
 *
 * Description   : This function used to send a Event to IfND.
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
ifnd_evt_send (NCSCONTEXT msg, IFSV_EVT_TYPE evt_type, IFSV_CB *ifsv_cb)
{   
   IFSV_EVT  *evt;
   NCS_IPC_PRIORITY priority = NCS_IPC_PRIORITY_NORMAL;
   uns32 res = NCSCC_RC_FAILURE;

   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return (res);
   }
   
   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));
   evt->type   = evt_type;
   evt->cb_hdl = ifsv_cb->cb_hdl;
   evt->vrid   = ifsv_cb->vrid;

   switch(evt_type)
   {
      case IFND_EVT_INIT_DONE:
         m_NCS_MEMCPY(&evt->info.ifnd_evt.info.init_done, msg, 
         sizeof(IFSV_EVT_INIT_DONE_INFO));

         m_IFND_LOG_EVT_INIT(IFSV_LOG_IFND_EVT_INIT_DONE_RCV,\
            evt->info.ifd_evt.info.init_done.init_done);
      break;

      case IFND_EVT_IFD_DOWN:
      case IFND_EVT_IFA_DOWN_IN_OPER_STATE:
      case IFND_EVT_IFA_DOWN_IN_DATA_RET_STATE:
      case IFND_EVT_IFA_DOWN_IN_MDS_DEST_STATE:
      case IFND_EVT_IFDRV_DOWN_IN_OPER_STATE:
      case IFND_EVT_IFDRV_DOWN_IN_DATA_RET_STATE:
      case IFND_EVT_IFDRV_DOWN_IN_MDS_DEST_STATE:
         m_NCS_MEMCPY(&evt->info.ifnd_evt.info.mds_svc_info.mds_dest, msg, 
         sizeof(MDS_DEST));
      break;

      default:
       m_IFND_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT,\
                       "Evt from IfND to IfD", evt_type);      
         break;
   }

   res = m_NCS_IPC_SEND(&ifsv_cb->mbx, evt, priority);   
   return (res);
}

/****************************************************************************
 * Name          : ifnd_mds_ifd_up_event
 *
 * Description   : This function used to send a Event to IfND.
 *
 * Arguments     : ifsv_cb    - This is the IFSV control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_mds_ifd_up_event (IFSV_CB *ifsv_cb)
{
   IFSV_EVT  *evt;
   NCS_IPC_PRIORITY priority = NCS_IPC_PRIORITY_NORMAL;
   uns32 res = NCSCC_RC_FAILURE;

   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return (res);
   }

   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));
   evt->type   = IFND_EVT_IFD_UP;
   evt->cb_hdl = ifsv_cb->cb_hdl;
   evt->vrid   = ifsv_cb->vrid;

   res = m_NCS_IPC_SEND(&ifsv_cb->mbx, evt, priority);
   return (res);
}

/****************************************************************************
 * Name          : ifnd_ifd_up_process
 *
 * Description   : This event callback function which would be called when
 *                 IFND receives IFD NCSMDS_NEW_ACTIVE request.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_ifd_up_process(IFSV_EVT* evt, IFSV_CB *cb)
{
   IPXS_EVT ipxs_evt; 
   NCS_IPPFX           del_ipinfo;
   uns32 ipxs_hdl = 0;
   IPXS_CB *ipxs_cb = NULL;
   uns32 rc = NCSCC_RC_SUCCESS, i = 0;

   /* Get the IPXS CB. */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);
   if(ipxs_cb == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   for(i = 0; i< ipxs_cb->nl_addr_info.num_ip_addr; i++)
   {
     m_NCS_MEMSET(&ipxs_evt, '\0', sizeof(IPXS_EVT));

     ipxs_evt.info.nd.atond_upd.if_index = ipxs_cb->nl_addr_info.list[i].if_index;

     m_NCS_IPXS_IPAM_ADDR_SET(ipxs_evt.info.nd.atond_upd.ip_info.ip_attr);

     ipxs_evt.info.nd.atond_upd.ip_info.delip_cnt ++;
     m_NCS_OS_MEMCPY(&del_ipinfo, &(ipxs_cb->nl_addr_info.list[i].ippfx),
                     sizeof(NCS_IPPFX)); 
     ipxs_evt.info.nd.atond_upd.ip_info.delip_list = &del_ipinfo;

     m_NCS_CONS_PRINTF("Sending IP ADDR UPDATE AFTER SWITCH OVER\n");
     m_NCS_CONS_PRINTF("IP : %x , Type %d, Len %d\n",del_ipinfo.ipaddr.info.v4, 
                        del_ipinfo.ipaddr.type, del_ipinfo.mask_len);
     if((rc= ifnd_ipxs_proc_ifip_upd(ipxs_cb, &ipxs_evt, NULL))
             != NCSCC_RC_SUCCESS)
     {
          m_NCS_CONS_PRINTF("Ip addition failed IP %x %s %d \n",del_ipinfo.ipaddr.info.v4,__FILE__,__LINE__);
          m_NCS_MEMSET(&ipxs_cb->nl_addr_info, '\0', sizeof(ipxs_cb->nl_addr_info));
          ncshm_give_hdl(ipxs_hdl);
          return NCSCC_RC_FAILURE;
     }
   } /* for loop */

  m_NCS_MEMSET(&ipxs_cb->nl_addr_info, '\0', sizeof(ipxs_cb->nl_addr_info));
  ncshm_give_hdl(ipxs_hdl);
  return NCSCC_RC_SUCCESS;
} /* The function ifnd_ifd_up_process() ends here. */

