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


#include "ifnd.h"

/*****************************************************************************
  FILE NAME: IFNDPROC.C

  DESCRIPTION: IfND initalization and destroy routines. Create/Modify/
               Destory interface record operations.

  FUNCTIONS INCLUDED in this module:
  ifnd_intf_create ......... Creation/Modification of interface record.
  ifnd_intf_delete ......... Delete interface record.
  ifnd_all_intf_rec_del .... Delete all the record in the interface db.
  ifnd_same_dst_intf_rec_del Delete all the record for the same destination.
  ifnd_same_drv_intf_rec_del Delete all the record created by its driver.
  ifnd_clear_mbx ........... Clean the IfND mail box.
  ifnd_amf_init...... ...... Inialize AMF for IfND.
  ifnd_init_cb...... ....... IfND CB inialize.  
  ifnd_ifap_ifindex_alloc... Allocate Ifindex for the intf created on IfND
  ifnd_ifap_ifindex_free.... Free Ifindex for the intf created on IfND
  ifsv_ifnd_ifindex_alloc ....... Allocates Ifindex.
  ifnd_intf_ifinfo_send .... Response to Interface Get request from app.

******************************************************************************/
#if(NCS_VIP == 1)
static uns32 ifnd_init_vip_db(IFSV_CB *cb);
#endif
#if 0
/****************************************************************************
 * Name          : ifnd_send_link_update_request
 *
 * Description   : This function will be called when ever any new interface 
 *                 is created.
 *
 * Arguments     : ifsv_cb     - pointer to the interface Control Block.
 *               : create_intf - This is the structure used to carry 
 *                 information about the newly created interface or the 
 *                 modified parameters. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Whenever the configuration management or the interface 
 *                 driver  is sending the interface creation, We need to inform
 *                 Netlink to update information regarding that interface.
 *****************************************************************************/

void ifnd_send_link_update_request ()
{

   IPXS_CB          *ipxs_cb;
   uns32             ipxs_hdl;
   IFSV_EVT         *ifsv_evt;
   SYSF_MBX         *mbx;

   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);

   if (ipxs_cb->netlink_updated != TRUE)
   {
      ncshm_give_hdl(ipxs_hdl);
      return;
   }


   ifsv_evt =  m_MMGR_ALLOC_IFSV_EVT;
   if (ifsv_evt == IFSV_NULL)
   {
      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                          IFSV_VIP_MEM_ALLOC_FAILED);
      ncshm_give_hdl(ipxs_hdl);
       m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "ifnd_send_link_update_request : Memory alloc failed, ifsv_evt = ",ifsv_evt);

      return;
   }
   m_NCS_MEMSET(ifsv_evt,0,sizeof(IFSV_EVT));
   ifsv_evt->type = IFSV_EVT_NETLINK_LINK_UPDATE;
   
   /* Put it in IFND's Event Queue */
   mbx = &ipxs_cb->mbx;
   m_NCS_CONS_PRINTF("mbx is 0x%x\n", mbx);

   if(m_IFND_EVT_SEND(mbx, ifsv_evt, NCS_IPC_PRIORITY_HIGH)
      == NCSCC_RC_FAILURE)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_SEND_FAIL,mbx);
      m_MMGR_FREE_IFSV_EVT(ifsv_evt);
      m_NCS_CONS_PRINTF("Posted Event failed\n");
      ncshm_give_hdl(ipxs_hdl);
      m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "ifnd_send_link_update_request : m_IFND_EVT_SEND failed", 0);
      return ;
   }

   m_NCS_CONS_PRINTF("Posted Event Success\n");
   m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,
                       IFSV_VIP_CREATED_IFND_IFND_VIP_DEL_VIPD_EVT);
   ncshm_give_hdl(ipxs_hdl);

  return;

}
#endif
/****************************************************************************
 * Name          : ifnd_intf_create
 *
 * Description   : This function will be called when the interface is created
 *                 or the interface parameter gets modified.
 *
 * Arguments     : ifsv_cb     - pointer to the interface Control Block.
 *               : create_intf - This is the structure used to carry 
 *                 information about the newly created interface or the 
 *                 modified parameters. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Whenever the configuration management or the interface 
 *                 driver  is sending the interface configuration, than it 
 *                 should send the record's owner FALSE.
 *****************************************************************************/

uns32
ifnd_intf_create (IFSV_CB *ifsv_cb, 
                  IFSV_INTF_CREATE_INFO *create_intf, IFSV_SEND_INFO *sinfo)
{
   NCS_IFSV_SPT_MAP  spt_map;
   IFSV_INTF_DATA    *rec_data;   
   uns32             ifindex = 0;
   uns32             res = NCSCC_RC_SUCCESS;
   NCS_LOCK          *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   IFSV_IDIM_EVT_SET_HW_PARAM_INFO hw_param;
   IFSV_EVT send_evt, *msg_rcvd;
   IFSV_INTF_ATTR attr;
   uns32 error = NCS_IFSV_NO_ERROR;
   IFSV_INTF_ATTR if_attr; 
   IFSV_TMR *tmr = NULL;
   IFSV_INTF_REC *p_intf_rec = NULL;
   
   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   /* this is the case where the interface record is already in the 
    * interface record */
   if (ifsv_get_ifindex_from_spt(&ifindex, create_intf->intf_data.spt_info,
      ifsv_cb) == NCSCC_RC_SUCCESS)
   {      
      if ((rec_data = ifsv_intf_rec_find (ifindex, ifsv_cb))
         != IFSV_NULL)
      {
          create_intf->intf_data.if_index = ifindex;
         /* here if any physical interface attributes has changed then it 
         * should inform the hardware driver about the change in the 
         * physical parameter. The record modification will be done only 
         * after the hardware driver sends the update back to the IfND 
            */
           if ((rec_data->originator == NCS_IFSV_EVT_ORIGN_HW_DRV) &&
              ((create_intf->intf_data.current_owner == NCS_IFSV_OWNER_IFND) && 
               (m_NCS_NODE_ID_FROM_MDS_DEST(create_intf->intf_data.current_owner_mds_destination) == 
                m_NCS_NODE_ID_FROM_MDS_DEST(rec_data->current_owner_mds_destination))) && 
               (create_intf->intf_data.originator != NCS_IFSV_EVT_ORIGN_HW_DRV))
            {
            /* send an event for the modifed parameters to the hardware
             * driver */
               m_NCS_MEMSET(&hw_param,0,sizeof(hw_param));
               m_NCS_MEMCPY(&hw_param.hw_param, &create_intf->intf_data.if_info,
                  sizeof(NCS_IFSV_INTF_INFO));
               hw_param.hw_param.if_am  = create_intf->if_attr;
               hw_param.slot_port       = rec_data->spt_info;
               res = ifnd_idim_evt_send((NCSCONTEXT)&hw_param,
                  IFSV_IDIM_EVT_SET_HW_PARM, ifsv_cb);
              goto end; 
            } else
            {
              attr = create_intf->if_attr; 
              if((create_intf->evt_orig == NCS_IFSV_EVT_ORIGN_IFA)||
                 (create_intf->evt_orig == NCS_IFSV_EVT_ORIGN_HW_DRV))
              {
                /* Before sending it to IfD, let us see whether the existing intf rec
                   is marked as DEL? */
                if(rec_data->marked_del == TRUE)
                {
                   /* If this is the case, then it means that some appl is adding
                      then intf rec. So delete this intf and add it again. */ 
                      tmr = ifsv_cleanup_tmr_find(&create_intf->intf_data.spt_info,                                                                                      &ifsv_cb->if_map_tbl);
                      if(tmr != NULL)
                        ifsv_tmr_stop(tmr,ifsv_cb);

                      p_intf_rec = ifsv_intf_rec_del(ifindex, ifsv_cb);
                      if( p_intf_rec != NULL )
                      {
                         m_MMGR_FREE_IFSV_INTF_REC( p_intf_rec );
                         m_IFND_LOG_STR_NORMAL(IFSV_LOG_IF_TBL_DEL_SUCCESS,
                          "Ifindex = ",ifindex);  
/*
                       m_IFSV_LOG_HEAD_LINE(ifsv_cb->my_svc_id,\
                                            IFSV_LOG_IF_TBL_DEL_SUCCESS,ifindex,0);
*/
                      }

                      m_NCS_MEMSET(&spt_map,0,sizeof(NCS_IFSV_SPT_MAP));
                      spt_map.spt = create_intf->intf_data.spt_info;
                      spt_map.if_index = ifindex;

                      /* Fill the current owner information. */
                      create_intf->intf_data.current_owner = NCS_IFSV_OWNER_IFND;
                      create_intf->intf_data.current_owner_mds_destination =
                                                           ifsv_cb->my_dest;
                      /* Fill the if index in the record. */
                      create_intf->intf_data.if_index = spt_map.if_index;

                      /* Send the INTF CREATE info to IfD. */
                      res = ifnd_sync_send_to_ifd(&create_intf->intf_data,
                                                IFSV_INTF_REC_ADD, 0, ifsv_cb, &msg_rcvd);
                      if(res == NCSCC_RC_SUCCESS)
                      {
                       /* MDS can return success, but there may be problem on the IfD
                           internal data processing. So, better check for that also.*/
                       if(msg_rcvd->error == NCS_IFSV_NO_ERROR)
                       {
                        res = ifsv_intf_rec_add(&create_intf->intf_data, ifsv_cb);

                        if(res == NCSCC_RC_SUCCESS)
                        {
                          m_IFND_LOG_STR_NORMAL(IFSV_LOG_IF_TBL_ADD_SUCCESS,
                             "Ifindex = ",create_intf->intf_data.if_index);  
/*
                          m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(ifsv_cb->comp_type),\
                                        IFSV_LOG_IF_TBL_ADD_SUCCESS,create_intf->intf_data.if_index,0);
*/
                          m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(ifsv_cb->comp_type),\
                          IFSV_LOG_IF_MAP_TBL_ADD_SUCCESS,create_intf->intf_data.if_index,create_intf->intf_data.spt_info.port);
                         /* Now send sync resp to the IfA. */
                          send_evt.type = IFA_EVT_INTF_CREATE_RSP;
                          send_evt.error = NCS_IFSV_NO_ERROR;
                          send_evt.info.ifa_evt.info.if_add_rsp_idx =
                                                            spt_map.if_index;
                          if(create_intf->evt_orig == NCS_IFSV_EVT_ORIGN_IFA)
                          {
                           /* Sync resp */
                           res = ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl,NCSMDS_SVC_ID_IFND,
                                                   sinfo, &send_evt);
                          }
                         /* Inform the application for the received change */
                         ifsv_ifa_app_if_info_indicate(&create_intf->intf_data,
                                               IFSV_INTF_REC_ADD, 0, ifsv_cb);
                          /***************FREEING IFSV_EVT Here after **********/
                          if( msg_rcvd != NULL )
                          {
                              m_MMGR_FREE_IFSV_EVT(msg_rcvd);
                          }

                         goto end;
                        }
                        else
                        {
                          error = NCS_IFSV_INT_ERROR;
                          /* Record was not added, so send IfD IF INDEX CLEANUP event. */
                          ifnd_mds_msg_send((NCSCONTEXT) &spt_map.spt,
                                            IFD_EVT_IFINDEX_CLEANUP, ifsv_cb);
                          m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                          "ifnd_intf_create : IFA/IFDRV : marked_del : ifsv_intf_rec_add failed res = ",res);
                          res = NCSCC_RC_FAILURE;
                        }
                      } /* if(msg_rcvd->error == NCS_IFSV_NO_ERROR)  */
                      else
                      {
                        /* Record was not added, so send IfD IF INDEX CLEANUP event. */
                        ifnd_mds_msg_send((NCSCONTEXT) &spt_map.spt,
                                          IFD_EVT_IFINDEX_CLEANUP, ifsv_cb);
                      /* We could not get the correct response. So, send the error back
                         to IfA. */
                      error = msg_rcvd->error;
                      m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                          "ifnd_intf_create : IFA/IFDRV : marked_del : ifnd_sync_send_to_ifd returned msg_rcvd->error as Error = ",error);
                      res = NCSCC_RC_FAILURE;
                     }
                      /***************FREEING IFSV_EVT Here after **********/
                     if( msg_rcvd != NULL )
                     {
                         m_MMGR_FREE_IFSV_EVT(msg_rcvd);
                     }

                    } /* if(res == NCSCC_RC_SUCCESS)  */
                    else
                    {
                      /* Record was not added, so send IfD IF INDEX CLEANUP event. */
                         ifnd_mds_msg_send((NCSCONTEXT) &spt_map.spt,
                                            IFD_EVT_IFINDEX_CLEANUP, ifsv_cb);
                     if(res == NCSCC_RC_FAILURE)
                       error = NCS_IFSV_EXT_ERROR;
                     else
                       error = NCS_IFSV_SYNC_TIMEOUT;
                     m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                          "ifnd_intf_create :IFA/IFDRV : marked_del : ifnd_sync_send_to_ifd failed res = ",res);
                     /* We could not get the index. */
                     res = NCSCC_RC_FAILURE;
                   }
                } /* rec_data->marked_del == TRUE) */
                else
                { /* rec_data->marked_del == FALSE. */
                /* Send it to IfD.*/
                res = ifnd_sync_send_to_ifd(&create_intf->intf_data, IFSV_INTF_REC_MODIFY, 
                                         attr, ifsv_cb, &msg_rcvd);
               if(res != NCSCC_RC_SUCCESS)
               {
                 if(res == NCSCC_RC_FAILURE)
                    error = NCS_IFSV_EXT_ERROR;
                 else
                    error = NCS_IFSV_SYNC_TIMEOUT;
                 m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                      "ifnd_intf_create :IFA/IFDRV : Modification : ifnd_sync_send_to_ifd failed res = ",res);
                 /* We could not get the index. */
                 res = NCSCC_RC_FAILURE;
               }
               else/* MDS SYNC SEND SUCCESS */
               {
                 /* MDS can return success, but there may be problem on the IfD 
                    internal data processing. So, better check for that also.*/
                if((msg_rcvd->error == NCS_IFSV_NO_ERROR) || (msg_rcvd->error == NCS_IFSV_INTF_MOD_ERROR))
                {
                  m_IFSV_ALL_ATTR_SET(if_attr);
                  if_attr = 0;
                  if_attr = create_intf->if_attr; 
                  /* No retention timer running, so this intf will be 
                     modified.*/
                  res = ifsv_intf_rec_modify(rec_data, &create_intf->intf_data,
                                             &if_attr,ifsv_cb);
                 /* Now send sync resp to the IfA. */
                 m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IFSV_EVT));
                 send_evt.type = IFA_EVT_INTF_CREATE_RSP;
                 if(res == NCSCC_RC_SUCCESS)
                  send_evt.error = NCS_IFSV_NO_ERROR;
                 else
                  send_evt.error = NCS_IFSV_INTF_MOD_ERROR;
                 send_evt.info.ifa_evt.info.if_add_rsp_idx =
                                           create_intf->intf_data.if_index;

                 if(create_intf->evt_orig == NCS_IFSV_EVT_ORIGN_IFA)
                 {
                  /* Sync resp */
                  ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl, 
                                          NCSMDS_SVC_ID_IFND, sinfo, &send_evt);
                 }
                 /* Inform the application for the received change */
                 if(res == NCSCC_RC_SUCCESS)
                  ifsv_ifa_app_if_info_indicate(rec_data, IFSV_INTF_REC_MODIFY, 
                                              if_attr, ifsv_cb);
                  /***************FREEING IFSV_EVT Here after **********/
                  if( msg_rcvd != NULL )
                  {
                     m_MMGR_FREE_IFSV_EVT(msg_rcvd);
                  }

                 goto end; 
                } /* if(msg_rcvd->error == NCS_IFSV_NO_ERROR)  */
                else
                {
                 /* We could not get the correct response. So, send the error 
                    back to IfA. */
                  error = msg_rcvd->error;
                  m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                      "ifnd_intf_create :IFA/IFDRV : Modification : ifnd_sync_send_to_ifd returned msg_rcvd->error as Error = ",error);
                  res = NCSCC_RC_FAILURE;
                 }
                 /***************FREEING IFSV_EVT Here after **********/
                  if( msg_rcvd != NULL )
                  {
                     m_MMGR_FREE_IFSV_EVT(msg_rcvd);
                  }
                } /*END SUCCESS */
               }/* rec_data->marked_del == FALSE  */ 
              } /* if(create_intf->evt_orig == NCS_IFSV_EVT_ORIGN_IFA)||  */
              else
              {
               /* It is from IfD. */
               /* Check whether this intf was marked as del? */
               if(rec_data->marked_del == TRUE)
               {
                  /* This is a new record, as when the appl goes down at one node, the rec
                     at other nodes gets deleted, but rec at the local node 
                     remains with marked_del flag as TRUE. So, if 
                     another appl comes at another node and tries to add the 
                     intf rec with the same SPT then the REC ADD will come from IfD
                     to the local node. So, in this case delete existing intf rec and add it again.*/
                     
                     /* This is the case when appl has come up over another node and added
                        the intf rec with the same SPT. */
                       tmr = ifsv_cleanup_tmr_find(&create_intf->intf_data.spt_info,                                                                                      &ifsv_cb->if_map_tbl);
                       if(tmr != NULL)
                         ifsv_tmr_stop(tmr,ifsv_cb);

                     /* Delete the old record. */
                     p_intf_rec = ifsv_intf_rec_del(ifindex, ifsv_cb);
                     if(p_intf_rec  != NULL)
                     {
                         m_MMGR_FREE_IFSV_INTF_REC( p_intf_rec );
                      
                       m_IFND_LOG_STR_NORMAL(IFSV_LOG_IF_TBL_DEL_SUCCESS,
                                   "Ifindex = ",ifindex); 
/*
                       m_IFSV_LOG_HEAD_LINE(ifsv_cb->my_svc_id,\
                                     IFSV_LOG_IF_TBL_DEL_SUCCESS,ifindex,0);
*/
                     }

                     /* add the interface record in to the interface database */
                     res = ifsv_intf_rec_add(&create_intf->intf_data, ifsv_cb);
                     if(res == NCSCC_RC_SUCCESS)
                     {
                      m_IFND_LOG_STR_NORMAL(IFSV_LOG_IF_TBL_ADD_SUCCESS,
                          "Ifindex = ",create_intf->intf_data.if_index); 
/* 
                      m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(ifsv_cb->comp_type),\
                                        IFSV_LOG_IF_TBL_ADD_SUCCESS,create_intf->intf_data.if_index,0);
*/
                      m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(ifsv_cb->comp_type),\
                      IFSV_LOG_IF_MAP_TBL_ADD_SUCCESS,create_intf->intf_data.if_index,create_intf->intf_data.spt_info.port);
                     }
                     else
                     {
                        m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                        "ifnd_intf_create : IFD : marked_del : ifsv_intf_rec_add failed res = ",res);
                     }

                     /* send a message to all the application which has registered
                        * with the IFA */
                     ifsv_ifa_app_if_info_indicate(&create_intf->intf_data,
                                       IFSV_INTF_REC_ADD, create_intf->if_attr, ifsv_cb);

                     goto end;

               } 
               if_attr = 0;
               if_attr = create_intf->intf_data.if_info.if_am;
               res = ifsv_intf_rec_modify (rec_data, &create_intf->intf_data,
                                              &if_attr, ifsv_cb);
              /* if the componant part is IFND than inform the application 
                 for the received change */
              if(res == NCSCC_RC_SUCCESS)
               ifsv_ifa_app_if_info_indicate(rec_data, IFSV_INTF_REC_MODIFY, 
                                            create_intf->if_attr, ifsv_cb);

              goto end; 
              }

            }
      } else
      {
         m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "ifnd_intf_create : SPT MAP exists, intf rec doesn't IfIndex = ",ifindex);
         res = NCSCC_RC_FAILURE;
      }      
   } else
   {
      /* This is the case where the interface record is created newly, which 
       * might have came from interface driver or from the IFA where IFND 
       * stores other interfaces present in other cards */
      
      if ((m_NCS_NODE_ID_FROM_MDS_DEST(create_intf->intf_data.originator_mds_destination) == 
          m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest)) && ((create_intf->intf_data.originator == 
          NCS_IFSV_EVT_ORIGN_IFA) ||
          (create_intf->intf_data.originator == NCS_IFSV_EVT_ORIGN_HW_DRV) || 
          (create_intf->intf_data.originator == NCS_IFSV_EVT_ORIGN_IFND))) 
      {
         /* If am the owner of this record, here assume that the ifindex
          * will be resolved asynchronously always 
          */
         m_NCS_MEMSET(&spt_map,0,sizeof(NCS_IFSV_SPT_MAP));
         spt_map.spt = create_intf->intf_data.spt_info;
         
         if ((res = ifsv_ifnd_ifindex_alloc(&spt_map,ifsv_cb, 
                    (NCSCONTEXT *)&msg_rcvd)) == NCSCC_RC_SUCCESS)
         {
           /* MDS can return success, but there may be problem on the IfD 
              internal data processing. So, better check for that also.*/
           if(msg_rcvd->error == NCS_IFSV_NO_ERROR)
           {
             spt_map.if_index = 
                      msg_rcvd->info.ifnd_evt.info.spt_map.spt_map.if_index;
             if( msg_rcvd != NULL )
             {
                 m_MMGR_FREE_IFSV_EVT(msg_rcvd);
             }
           /* Got the index successfully. Add the record in the data base. 
              And send a sync resp to IfA. */
           /* Fill the current owner information. */
           create_intf->intf_data.current_owner = NCS_IFSV_OWNER_IFND;
           create_intf->intf_data.current_owner_mds_destination = 
                                                           ifsv_cb->my_dest;
           /* Fill the if index in the record. */
           create_intf->intf_data.if_index = spt_map.if_index;

           /* Send the INTF CREATE info to IfD. */           
           res = ifnd_sync_send_to_ifd(&create_intf->intf_data, 
                                     IFSV_INTF_REC_ADD, 0, ifsv_cb, &msg_rcvd); 
           if(res == NCSCC_RC_SUCCESS)
           {
            /* MDS can return success, but there may be problem on the IfD 
              internal data processing. So, better check for that also.*/
            if(msg_rcvd->error == NCS_IFSV_NO_ERROR)
            {
             res = ifsv_intf_rec_add(&create_intf->intf_data, ifsv_cb);
            
             if(res == NCSCC_RC_SUCCESS)
             {
              m_IFND_LOG_STR_NORMAL(IFSV_LOG_IF_TBL_ADD_SUCCESS,
                   "Ifindex = ",create_intf->intf_data.if_index); 
/*
              m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(ifsv_cb->comp_type),\
                                        IFSV_LOG_IF_TBL_ADD_SUCCESS,create_intf->intf_data.if_index,0);
*/
              /* Now send sync resp to the IfA. */
               send_evt.type = IFA_EVT_INTF_CREATE_RSP;
               send_evt.error = NCS_IFSV_NO_ERROR;
               send_evt.info.ifa_evt.info.if_add_rsp_idx =  
                                                 spt_map.if_index;
               if(create_intf->evt_orig == NCS_IFSV_EVT_ORIGN_IFA)
               {
                /* Sync resp */
                res = ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl,NCSMDS_SVC_ID_IFND, 
                                        sinfo, &send_evt);
               }
               /* Inform the application for the received change */
               ifsv_ifa_app_if_info_indicate(&create_intf->intf_data,
                                             IFSV_INTF_REC_ADD, 0, ifsv_cb);
               if( msg_rcvd != NULL )
               {
                  m_MMGR_FREE_IFSV_EVT(msg_rcvd);
               }
#if 0
               if(create_intf->evt_orig == NCS_IFSV_EVT_ORIGN_HW_DRV)
               {
                 /* Send an Event to Netlink for updating the ip addresses for newly created intf */
                 ifnd_send_link_update_request ();
               }
#endif

               goto end; 
             }   
             else
             {
               error = NCS_IFSV_INT_ERROR;
               /* Record was not added, so send IfD IF INDEX CLEANUP event. */
               ifnd_mds_msg_send((NCSCONTEXT) &spt_map.spt,
                                 IFD_EVT_IFINDEX_CLEANUP, ifsv_cb);
               m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                    "ifnd_intf_create : New Rec : ifsv_intf_rec_add failed res = ",res);
               res = NCSCC_RC_FAILURE;
               if( msg_rcvd != NULL )
               {
                  m_MMGR_FREE_IFSV_EVT(msg_rcvd);
               }

              }
            } /* if(msg_rcvd->error == NCS_IFSV_NO_ERROR)  */
            else
            {
               /* Record was not added, so send IfD IF INDEX CLEANUP event. */
               ifnd_mds_msg_send((NCSCONTEXT) &spt_map.spt,
                                 IFD_EVT_IFINDEX_CLEANUP, ifsv_cb);
             /* We could not get the correct response. So, send the error back
                to IfA. */
             error = msg_rcvd->error;
             m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
               "ifnd_intf_create : New Rec : ifnd_sync_send_to_ifd failed, msg_rcvd->error = ",error);
             res = NCSCC_RC_FAILURE;
           /***************FREEING IFSV_EVT Here after **********/
               if( msg_rcvd != NULL )
               {
                  m_MMGR_FREE_IFSV_EVT(msg_rcvd);
               }
            }

           } /* if(res == NCSCC_RC_SUCCESS)  */
           else
           {
               /* Record was not added, so send IfD IF INDEX CLEANUP event. */
               ifnd_mds_msg_send((NCSCONTEXT) &spt_map.spt,
                                 IFD_EVT_IFINDEX_CLEANUP, ifsv_cb);
              if(res == NCSCC_RC_FAILURE)
                error = NCS_IFSV_EXT_ERROR;
               else
                error = NCS_IFSV_SYNC_TIMEOUT;
             m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
               "ifnd_intf_create : New Rec : ifnd_sync_send_to_ifd failed, res = ",res);
              /* We could not get the index. */
              res = NCSCC_RC_FAILURE;
           }
           } /* if(msg_rcvd->error == NCS_IFSV_NO_ERROR)  */
           else
           { 
             /* We could not get the correct response. So, send the error back
                to IfA. */
             if( msg_rcvd != NULL )
             {
                 m_MMGR_FREE_IFSV_EVT(msg_rcvd);
             }
             error = msg_rcvd->error;
             m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
               "ifnd_intf_create : New Rec : Index Alloc failed, msg_rcvd->error = ",error);
             res = NCSCC_RC_FAILURE;
           }
          }
          else
          {
              if(res == NCSCC_RC_FAILURE)
                error = NCS_IFSV_EXT_ERROR;
               else
                error = NCS_IFSV_SYNC_TIMEOUT;
              m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
               "ifnd_intf_create : New Rec : Index Alloc failed, res = ",res);
              /* We could not get the index. */
              res = NCSCC_RC_FAILURE;
          }
         
      } else
      {
         /* If am not the owner of the record */
         /* add the interface record in to the interface database */
         res = ifsv_intf_rec_add(&create_intf->intf_data, ifsv_cb);
         
         if(res != NCSCC_RC_SUCCESS)
         {
           m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "ifnd_intf_create : New Rec : IFD : ifsv_intf_rec_add failed res = ",res);
         }
         /* send a message to all the application which has registered 
         * with the IFA 
         */
         if(res == NCSCC_RC_SUCCESS)
         ifsv_ifa_app_if_info_indicate(&create_intf->intf_data, 
            IFSV_INTF_REC_ADD, create_intf->if_attr, ifsv_cb);

         goto end; 
      }
      
   }

   /* Send the sync response to Sender (IFA) in case if ifindex is
      already avilable or some processing error accours*/
   if(ifindex || (res != NCSCC_RC_SUCCESS))
   {
      IFSV_EVT     send_evt;
      m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IFSV_EVT));

      send_evt.type = IFA_EVT_INTF_CREATE_RSP;
      send_evt.error = error;
      send_evt.info.ifa_evt.info.if_add_rsp_idx = ifindex;

      /* Sync resp */
      ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl, 
                             NCSMDS_SVC_ID_IFND, sinfo, 
                             &send_evt);
   }

end:
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
   return (res); 
}

/****************************************************************************
 * Name          : ifnd_intf_delete
 *
 * Description   : This function will be called when the interface operation 
 *                 status is set to DOWN or the configuration management 
 *                 deletes the interface. 
 *
 * Arguments     : ifsv_cb     - pointer to the interface Control Block.
 *               : dest_info   - This is the strucutre used to delete an 
 *                               interface.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Whenever the configuration management or the interface 
 *                 driver needs to send a interface delete or operation 
 *                 status down, than it would send the vcard ID as "0".
 *****************************************************************************/

uns32
ifnd_intf_delete (IFSV_CB *ifsv_cb, IFSV_INTF_DESTROY_INFO *dest_info, 
                  IFSV_SEND_INFO *sinfo)
{
   IFSV_INTF_REC     *rec;
   IFSV_INTF_DATA    *rec_data;
   NCS_LOCK          *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   uns32 res = NCSCC_RC_SUCCESS;
   uns32 ifindex;
   IFSV_EVT send_evt, *msg_rcvd;
   IFSV_INTF_ATTR attr = 0;
   uns32 error = NCS_IFSV_NO_ERROR;

   if (ifsv_get_ifindex_from_spt(&ifindex, dest_info->spt_type, 
      ifsv_cb) == NCSCC_RC_SUCCESS)
   {
      m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
      if ((rec_data = ifsv_intf_rec_find(ifindex, ifsv_cb))
         != IFSV_NULL)
      {
#if(NCS_IFSV_BOND_INTF == 1)
               NCS_IFSV_IFINDEX bonding_ifindex;
               uns32 res = NCSCC_RC_SUCCESS;
    #if(NCS_IFSV_IPXS == 1)
               IPXS_CB  *ipxs_cb = NULL;
               uns32    ipxs_hdl;
               IPXS_IFIP_NODE   *ifip_node = NULL;
               IPXS_IFIP_INFO   *ip_info = NULL;
    #endif
              /* If Binding interface is getting deleted reset master's master and slave's slave to 0 */
       if(rec_data->spt_info.type == NCS_IFSV_INTF_BINDING)
             {
                ifsv_delete_binding_interface(ifsv_cb,ifindex);
             }
       else if((res = ifsv_bonding_check_ifindex_is_master(ifsv_cb,rec_data->if_index,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
         {
               IFSV_INTF_DATA* bond_intf_data;
          if((bond_intf_data = ifsv_binding_delete_master_ifindex(ifsv_cb,bonding_ifindex)) != NULL)
               {
          /*when master interface is deleted, the ip's associated with it are also deleted so add ip to new master*/
#if(NCS_IFSV_IPXS == 1)
                  ipxs_hdl = m_IPXS_CB_HDL_GET( );
                  ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);
                 if(ipxs_cb)
                 {
                   ifip_node = ipxs_ifip_record_get(&ipxs_cb->ifip_tbl, bonding_ifindex);
                   if(ifip_node)
                   {
                       ip_info = (IPXS_IFIP_INFO *)&ifip_node->ifip_info;
                       if(ip_info)
                       {
                       ifsv_add_to_os(bond_intf_data,ip_info->ipaddr_list,ip_info->ipaddr_cnt,ifsv_cb);
                       }
                   }
                 }
                 ncshm_give_hdl(ipxs_hdl);
#endif
     /* Send master change message to applications. If the slave is not active or is marked del
         , master and slave are not swapped*/
                  if(bond_intf_data->if_info.bind_master_ifindex != 0)
                   {
                    attr = NCS_IFSV_IAM_CHNG_MASTER;
                    ifsv_ifa_app_if_info_indicate(bond_intf_data, IFSV_INTF_REC_MODIFY,
                                                       attr, ifsv_cb);
                   }

               }
          }
          else if((res = ifsv_binding_check_ifindex_is_slave(ifsv_cb,ifindex,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
          {
              IFSV_INTF_DATA* bond_intf_data;
              bond_intf_data = ifsv_intf_rec_find(bonding_ifindex,ifsv_cb);
              if( bond_intf_data != IFSV_NULL )
              {
                  bond_intf_data->if_info.bind_slave_ifindex = 0;
              }

          }
#endif

       if (m_NCS_NODE_ID_FROM_MDS_DEST(rec_data->originator_mds_destination) == 
           m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest))
         {     
            /* This is to check whether the orignator is the actuall 
             * destroyer, here the Application which is creating is 
             * the responsible for deletion 
             */
            if((((dest_info->orign == NCS_IFSV_EVT_ORIGN_HW_DRV) &&
               (rec_data->originator == NCS_IFSV_EVT_ORIGN_HW_DRV)) ||
               ((dest_info->orign == NCS_IFSV_EVT_ORIGN_IFA) &&
               (m_NCS_IFSV_IS_MDS_DEST_SAME(&dest_info->own_dest,
               &rec_data->originator_mds_destination) == TRUE))))
            {
               /* this is the case where the delete command belongs to the 
                  same ifnd*/
               /* Send the INTF DEL info to IfD. */
               res = ifnd_sync_send_to_ifd(rec_data, IFSV_INTF_REC_DEL, 
                                          NCS_IFSV_IAM_OPRSTATE, ifsv_cb, 
                                          &msg_rcvd);
               if(res == NCSCC_RC_SUCCESS)
               {
                  /* MDS can return success, but there may be problem on the IfD
                     internal data processing. So, better check for that also.*/
                  if(msg_rcvd->error == NCS_IFSV_NO_ERROR)
                  {
                      attr = NCS_IFSV_IAM_OPRSTATE;
                      res = ifsv_intf_rec_marked_del(rec_data, &attr, ifsv_cb);
                     if(res == NCSCC_RC_SUCCESS)
                     {
                         /* Now send sync resp to the IfA. */
                         send_evt.type = IFA_EVT_INTF_DESTROY_RSP;
                         send_evt.error = NCS_IFSV_NO_ERROR;
                         send_evt.info.ifa_evt.info.spt_map.spt_map.spt = 
                                                           dest_info->spt_type;

                         if(rec_data->evt_orig == NCS_IFSV_EVT_ORIGN_IFA)
                         {
                           /* Sync resp */
                           res = ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl, 
                                                   NCSMDS_SVC_ID_IFND,sinfo,&send_evt);
                         }
                         /* Inform the application for the received change */
                         ifsv_ifa_app_if_info_indicate(rec_data, IFSV_INTF_REC_DEL,
                                                       attr, ifsv_cb);
                         if( msg_rcvd != NULL )
                         {
                            m_MMGR_FREE_IFSV_EVT(msg_rcvd);
                         }
                         m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
                         return NCSCC_RC_SUCCESS;
                    }
                    else
                    {
                        m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                         "ifnd_intf_delete : ifsv_intf_rec_marked_del failed res = ",res);
                        error = NCS_IFSV_INT_ERROR;
                        res = NCSCC_RC_FAILURE;
                    }
                 } /* if(msg_rcvd->error == NCS_IFSV_NO_ERROR)  */
                 else
                 {
                     /* We could not get the correct response. So, send the error back
                        to IfA. */
                     error = msg_rcvd->error;
                     m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                         "ifnd_intf_delete : ifnd_sync_send_to_ifd failed, msg_rcvd->error = ",error);
                     res = NCSCC_RC_FAILURE;
                 }
                 /***********FREEING */
                 if( msg_rcvd != NULL )
                 {
                     m_MMGR_FREE_IFSV_EVT(msg_rcvd);
                 }

              } /* if(res == NCSCC_RC_SUCCESS)  */
              else
              {
                 if(res == NCSCC_RC_FAILURE)
                   error = NCS_IFSV_EXT_ERROR;
                  else
                   error = NCS_IFSV_SYNC_TIMEOUT;
                   m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                         "ifnd_intf_delete : ifnd_sync_send_to_ifd failed, res = ",res);
                 /* We could not get the index. */
                 res = NCSCC_RC_FAILURE;
              }
          }
          else
          {
           if(dest_info->orign == NCS_IFSV_EVT_ORIGN_IFD)
           {
              /* This is the case when DEL comes back to IfND, who sent to IfD.*/
              m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
              return res;
           }
           else
           {
              error = NCS_IFSV_INT_ERROR;
           }
            
          }

           /* Now send sync resp to the IfA. */
           send_evt.type = IFA_EVT_INTF_DESTROY_RSP;
           send_evt.error = error;
           send_evt.info.ifa_evt.info.spt_map.spt_map.spt = 
                                                      dest_info->spt_type;
           if(rec_data->evt_orig == NCS_IFSV_EVT_ORIGN_IFA)
           {
             /* Sync resp */
             res = ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                                    sinfo, &send_evt);
           }
           m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
           return res;
         } else
         {
            /* this is the case where the delete command belongs to the different
             * ifnd
             */
            /* First check whether the intf delete command came from IfA or IFD. */
             if(dest_info->orign == NCS_IFSV_EVT_ORIGN_IFA)
              {
                /* This is the case of Delete from other IFA not at some other node. So, return failure.*/
                /* Send sync resp to the IfA. */
                   send_evt.type = IFA_EVT_INTF_DESTROY_RSP;
                   send_evt.error = NCS_IFSV_INT_ERROR;
                   send_evt.info.ifa_evt.info.spt_map.spt_map.spt =
                               dest_info->spt_type;
                   /* Sync resp */
                    res = ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl,
                                       NCSMDS_SVC_ID_IFND,sinfo,&send_evt);
                  
                    m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
                    m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                         "ifnd_intf_delete : Intf Rec belongs to diff node, res = ",res);
                    return (res);                
              }
            /* indicate all the application regarding the deletion of the 
             * interface record 
             */
            ifsv_ifa_app_if_info_indicate(rec_data, IFSV_INTF_REC_DEL, 
               0, ifsv_cb);

            rec = ifsv_intf_rec_del(ifindex, ifsv_cb);

            if (rec != NULL)
            {
               
               /* delete the slot port maping */
               ifsv_ifindex_spt_map_del(rec->intf_data.spt_info, ifsv_cb);
               
               m_MMGR_FREE_IFSV_INTF_REC(rec);
            }
           m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
           return res;
         }
      }
      m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
   }
    /* Now send sync resp to the IfA. */
     send_evt.type = IFA_EVT_INTF_DESTROY_RSP;
     send_evt.error = NCS_IFSV_INT_ERROR;
     send_evt.info.ifa_evt.info.spt_map.spt_map.spt = 
                                dest_info->spt_type;

     if(dest_info->orign == NCS_IFSV_EVT_ORIGN_IFA)
     {
         /* Sync resp */
         res = ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl, 
                                 NCSMDS_SVC_ID_IFND,sinfo,&send_evt);
     }
   return (res);
}



/****************************************************************************
 * Name          : ifnd_all_intf_rec_del
 *
 * Description   : This function will be called when the IfND needs to be 
 *                 shutdown. This will cleanup all the interfaces belonging to 
 *                 that card and free up all other records.
 *
 * Arguments     : ifsv_cb     - pointer to the interface Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Here since IfND forcibly wants to delete all it routes,
 *                 so it needs to send a free ifindex to IfD for which it has
 *                 allocated.
 *****************************************************************************/
uns32
ifnd_all_intf_rec_del (IFSV_CB *ifsv_cb)
{
   uns32 ifindex = 0;   
   IFSV_INTF_REC       *intf_rec;
   NCS_IFSV_SPT_MAP    spt;
   NCS_PATRICIA_NODE   *if_node;  
   NCS_LOCK            *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   uns32  res = NCSCC_RC_SUCCESS;
   IFSV_EVT *msg_rcvd = NULL;

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
      (uns8*)&ifindex);
   intf_rec = (IFSV_INTF_REC*)if_node;
   
   while (intf_rec != IFSV_NULL)
   {
      ifindex = intf_rec->intf_data.if_index;
      
      if (ifsv_intf_rec_del(ifindex, ifsv_cb) == NULL)
      {         
         m_IFND_LOG_STR_NORMAL(IFSV_LOG_IF_TBL_DEL_ALL_FAILURE,
                    "ifindex = ",ifindex);
         res = NCSCC_RC_FAILURE;
/*
         m_IFND_LOG_HEAD_LINE(IFSV_LOG_IF_TBL_DEL_ALL_FAILURE,ifindex,0);
*/
         break;
      }
      else
         m_IFND_LOG_STR_NORMAL(IFSV_LOG_IF_TBL_DEL_SUCCESS,
                    "ifindex = ",ifindex);
/*
      m_IFSV_LOG_HEAD_LINE(ifsv_cb->my_svc_id,\
         IFSV_LOG_IF_TBL_DEL_SUCCESS,ifindex,0);
*/
      
      /* delete the slot port maping */
      res= ifsv_ifindex_spt_map_del(intf_rec->intf_data.spt_info, ifsv_cb);      
      if(res == NCSCC_RC_SUCCESS)
      {
        m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(ifsv_cb->comp_type),\
         IFSV_LOG_IF_MAP_TBL_DEL_SUCCESS,intf_rec->intf_data.spt_info.slot,intf_rec->intf_data.spt_info.port);
      }
      else 
      {
        m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(ifsv_cb->comp_type),\
         IFSV_LOG_IF_MAP_TBL_DEL_FAILURE,intf_rec->intf_data.spt_info.slot,intf_rec->intf_data.spt_info.port);
      }

      /* check for the same Vcard ID */
      if (m_NCS_NODE_ID_FROM_MDS_DEST(intf_rec->intf_data.originator_mds_destination) == m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest))
      {         
         /* Send an MDS message for delete */
         res = ifnd_sync_send_to_ifd(&intf_rec->intf_data, IFSV_INTF_REC_DEL, 0, 
                               ifsv_cb, &msg_rcvd);
         if( res == NCSCC_RC_SUCCESS )
         {
             /********** FREEING */
             if( msg_rcvd != NULL )
             {
                m_MMGR_FREE_IFSV_EVT(msg_rcvd);
             }
         }
         else 
         {
          
           m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                    "ifnd_all_intf_rec_del : ifnd_sync_send_to_ifd failed, ifindex = ",ifindex);
         }
        /* send a ifindex free message to IfD for which this IfND has request 
         * for the ifindex allocates
         */
         spt.spt      = intf_rec->intf_data.spt_info;
         spt.if_index = intf_rec->intf_data.if_index;
         ifsv_ifindex_free (ifsv_cb, &spt); 

      }
      
      /* if the componant part is IFND than inform the application for the 
      * received change */
      ifsv_ifa_app_if_info_indicate(&intf_rec->intf_data, IFSV_INTF_REC_DEL,
         0, ifsv_cb);
      /* free the interface record */
      m_MMGR_FREE_IFSV_INTF_REC(intf_rec);      
      
      if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
         (uns8*)&ifindex);
      
      intf_rec = (IFSV_INTF_REC*)if_node;
   }
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifnd_same_dst_intf_rec_del
 *
 * Description   : This function will be called when the IfND founds any IfA
 *                 goes down. This will cleanup all the interfaces record 
 *                 belonging to that IfA. Here there won't be a aging timer
 *                 invoked for the deleting interface, it just blindly deletes
 *                 all the interface record and sends a message to all the 
 *                 IfNDs.
 *
 * Arguments     : ifsv_cb     - pointer to the interface Control Block.
 *                 dest        - check for the destination which needs to 
 *                               be removed (dest of the app going down).
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Here we assume that the application which has created the
 *                 interface would once again create the interface, if that
 *                 application goes down and come up.
 *****************************************************************************/

uns32
ifnd_same_dst_intf_rec_del (MDS_DEST *dest, IFSV_CB *ifsv_cb)
{
   uns32 ifindex = 0;   
   uns32 res = NCSCC_RC_SUCCESS;
   IFSV_INTF_REC       *intf_rec;
   NCS_PATRICIA_NODE   *if_node;   
   IFSV_EVT *msg_rcvd = NULL;
   IFSV_INTF_ATTR attr;
   NCS_LOCK            *intf_rec_lock = &ifsv_cb->intf_rec_lock;

#if (NCS_VIP == 1)
   /* do the Virtual IP Addresses related processing */
   ifnd_process_ifa_crash(ifsv_cb,dest);   

#endif



   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
      (uns8*)&ifindex);
   intf_rec = (IFSV_INTF_REC*)if_node;

   while (intf_rec != IFSV_NULL)
   {
      ifindex = intf_rec->intf_data.if_index;
      if((m_NCS_IFSV_IS_MDS_DEST_SAME(dest,&intf_rec->intf_data.originator_mds_destination) == TRUE) && (intf_rec->intf_data.originator == 
         NCS_IFSV_EVT_ORIGN_IFA))
      {
         if (m_NCS_NODE_ID_FROM_MDS_DEST(intf_rec->intf_data.originator_mds_destination) == m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest))
         {
            /* Send an MDS message for delete */
            res = ifnd_sync_send_to_ifd(&intf_rec->intf_data, IFSV_INTF_REC_DEL,
                                        0, ifsv_cb, &msg_rcvd);

            if(res == NCSCC_RC_SUCCESS)
            {
               /************FREEING */
               if( msg_rcvd != NULL )
               {
                  m_MMGR_FREE_IFSV_EVT(msg_rcvd);
               }

              res = ifsv_intf_rec_marked_del(&intf_rec->intf_data, &attr, 
                                             ifsv_cb);
            }
            else
            { 
               m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                    "ifnd_same_dst_intf_rec_del : ifnd_sync_send_to_ifd failed, ifindex = ",ifindex);
               break;
            }
         }

         /* if the componant part is IFND than inform the application for the 
          * received change */
          
         ifsv_ifa_app_if_info_indicate(&intf_rec->intf_data, IFSV_INTF_REC_MARKED_DEL,
            0, ifsv_cb);
      }
      
      if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
         (uns8*)&ifindex);
      
      intf_rec = (IFSV_INTF_REC*)if_node;
   }
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   return (res);
}

/****************************************************************************
 * Name          : ifnd_same_drv_intf_rec_del
 *
 * Description   : This function will be called when the IfND founds any drv
 *                 goes down on that card. This will cleanup all the interfaces
 *                 record created by that driver.
 *
 * Arguments     : ifsv_cb     - pointer to the interface Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32
ifnd_same_drv_intf_rec_del (IFSV_CB *ifsv_cb)
{
   uns32 ifindex = 0;   
   uns32 res = NCSCC_RC_SUCCESS;
   IFSV_INTF_REC       *intf_rec;
   NCS_PATRICIA_NODE   *if_node;   
   IFSV_EVT *msg_rcvd = NULL;
   NCS_LOCK            *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   IFSV_INTF_ATTR attr;

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
      (uns8*)&ifindex);
   intf_rec = (IFSV_INTF_REC*)if_node;

   while (intf_rec != IFSV_NULL)
   {
      ifindex = intf_rec->intf_data.if_index;
      if ((m_NCS_NODE_ID_FROM_MDS_DEST(intf_rec->intf_data.originator_mds_destination) ==  m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest)) && 
         (intf_rec->intf_data.originator == NCS_IFSV_EVT_ORIGN_HW_DRV))
      {
         if (m_NCS_NODE_ID_FROM_MDS_DEST(intf_rec->intf_data.originator_mds_destination) == m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest))
         {
            /* Send an MDS message for delete */
           res = ifnd_sync_send_to_ifd(&intf_rec->intf_data, 
                                       IFSV_INTF_REC_DEL, 0,ifsv_cb,&msg_rcvd);
            if(res == NCSCC_RC_SUCCESS)
            {
               /************FREEING */
               if( msg_rcvd != NULL )
               {
                  m_MMGR_FREE_IFSV_EVT(msg_rcvd);
               }
              res = ifsv_intf_rec_marked_del(&intf_rec->intf_data, &attr,
                                             ifsv_cb);
            }
            else
            { 
               m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                    "ifnd_same_drv_intf_rec_del : ifnd_sync_send_to_ifd failed, ifindex = ",ifindex);
               break;
            }

         }

         /* if the componant part is IFND than inform the application for the 
          * received change 
          */
         ifsv_ifa_app_if_info_indicate(&intf_rec->intf_data, IFSV_INTF_REC_DEL,
            0, ifsv_cb);
      }
      
      if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
         (uns8*)&ifindex);
      
      intf_rec = (IFSV_INTF_REC*)if_node;
   }
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   return (res);
}

/****************************************************************************
 * Name          : ifnd_clear_mbx
 *
 * Description   : This is the function which deletes all the messages from 
 *                 the mail box.
 *
 * Arguments     : arg     - argument to be passed.
 *                 msg     - Event start pointer.
 *
 * Return Values : TRUE/FALSE
 *
 * Notes         : None.
 *****************************************************************************/
NCS_BOOL
ifnd_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg)
{   
   IFSV_EVT  *pEvt = (IFSV_EVT *)msg;
   IFSV_EVT  *pnext;
   pnext = pEvt;
   while (pnext)
   {
      pnext = pEvt->next;
      ifsv_evt_destroy(pEvt);  
      pEvt = pnext;
   }
   return TRUE;
}

/****************************************************************************
 * Name          : ifnd_amf_init
 *
 * Description   : IfND initializes AMF for involking process and registers 
 *                 the various callback functions.
 *
 * Arguments     : ifsv_cb  - Ifsv control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_amf_init (IFSV_CB *ifsv_cb)
{
   SaAmfCallbacksT amfCallbacks;
   SaVersionT      amf_version;   
   SaAisErrorT        error;
   uns32           res = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));

   amfCallbacks.saAmfHealthcheckCallback       = 
      ifnd_saf_health_chk_callback;
   amfCallbacks.saAmfCSISetCallback = ifnd_saf_csi_set_cb;
   amfCallbacks.saAmfCSIRemoveCallback= 
      ifnd_saf_CSI_rem_callback;
   amfCallbacks.saAmfComponentTerminateCallback= 
      ifnd_saf_comp_terminate_callback;

   m_IFSV_GET_AMF_VER(amf_version);

   error = saAmfInitialize(&ifsv_cb->amf_hdl, &amfCallbacks, &amf_version);
   if (error != SA_AIS_OK)
   {   
      /* Log */
      res = NCSCC_RC_FAILURE;
      m_IFND_LOG_API_L(IFSV_LOG_AMF_INIT_FAILURE,ifsv_cb);
      m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                 "ifnd_amf_init : saAmfInitialize failed, error = ",error);
   }
   return (res);
}


/****************************************************************************
 * Name          : ifnd_init_cb
 *
 * Description   : This is the function which initializes all the data 
 *                 structures and locks used belongs to IfND.
 *
 * Arguments     : ifsv_cb  - Ifsv control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_init_cb (IFSV_CB *ifsv_cb)
{
   NCS_PATRICIA_PARAMS     params;   
   uns32                   res = NCSCC_RC_SUCCESS;

   do
   {
      ifsv_cb->my_svc_id = NCS_SERVICE_ID_IFND;      
      
      if (m_NCS_SEM_CREATE(&ifsv_cb->health_sem) == NCSCC_RC_FAILURE)
      {         
         res = NCSCC_RC_FAILURE;
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_SEM_CREATE_FAIL,ifsv_cb);
         break;
      }
      
      if (m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_CREATE, 0) 
         == NCSCC_RC_FAILURE)
      {         
         m_NCS_SEM_RELEASE(ifsv_cb->health_sem);
         res = NCSCC_RC_FAILURE;
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_LOCK_CREATE_FAIL,ifsv_cb);
         break;
      }
      m_IFND_LOG_LOCK(IFSV_LOG_LOCK_CREATE,&ifsv_cb->intf_rec_lock);
            
      /* initialze interface tree */
      
      params.key_size = sizeof(uns32);
      params.info_size = 0;
      if ((ncs_patricia_tree_init(&ifsv_cb->if_tbl, &params))
         != NCSCC_RC_SUCCESS)
      {
         m_NCS_SEM_RELEASE(ifsv_cb->health_sem);
         m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);
         res = NCSCC_RC_FAILURE;
         m_IFND_LOG_HEAD_LINE(IFSV_LOG_IF_TBL_CREATE_FAILURE,ifsv_cb,0);
         break;            
      }
      m_IFD_LOG_HEAD_LINE(IFSV_LOG_IF_TBL_CREATED,&ifsv_cb->if_tbl,0);
      
      /* initialze shelf/slot/port/type/scope tree */
      
      params.key_size = sizeof(NCS_IFSV_SPT);
      params.info_size = 0;
      if ((ncs_patricia_tree_init(&ifsv_cb->if_map_tbl, &params))
         != NCSCC_RC_SUCCESS)
      {
         m_NCS_SEM_RELEASE(ifsv_cb->health_sem);         
         m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);
         ncs_patricia_tree_destroy(&ifsv_cb->if_tbl);
         res = NCSCC_RC_FAILURE;
         m_IFND_LOG_HEAD_LINE(IFSV_LOG_IF_MAP_TBL_CREATE_FAILURE,ifsv_cb,0);
         break;
      }

      params.key_size = sizeof(MDS_DEST);
      params.info_size = 0;
      if ((ncs_patricia_tree_init(&ifsv_cb->mds_dest_tbl, &params))
         != NCSCC_RC_SUCCESS)
      {
         m_NCS_SEM_RELEASE(ifsv_cb->health_sem);         
         m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);
         ncs_patricia_tree_destroy(&ifsv_cb->if_tbl);
         ncs_patricia_tree_destroy(&ifsv_cb->if_map_tbl);
         res = NCSCC_RC_FAILURE;
         m_IFND_LOG_HEAD_LINE(IFSV_LOG_IF_MAP_TBL_CREATE_FAILURE,ifsv_cb,0);
         break;
      }

      m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_IF_MAP_TBL_CREATED,&ifsv_cb->if_map_tbl,0);
      
      /* initialze VIP database cache tree */
#if (NCS_VIP == 1)      
         res = ifnd_init_vip_db(ifsv_cb);
         if (res == NCSCC_RC_FAILURE)
         {
            m_NCS_SEM_RELEASE(ifsv_cb->health_sem);
            m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);

            ncs_patricia_tree_destroy(&ifsv_cb->if_tbl);
            ncs_patricia_tree_destroy(&ifsv_cb->if_map_tbl);
            res = NCSCC_RC_FAILURE;
           break;            
         }
#endif
      
      /* EDU initialisation */
      m_NCS_EDU_HDL_INIT(&ifsv_cb->edu_hdl);
   } while (0);
   return (res);
}

/****************************************************************************
 * Name          : ifnd_ifap_ifindex_alloc
 *
 * Description   : This is the function which allocates ifindex for the given 
 *                 slot, port, type and scope on IFND. Since this is the target 
 *                 service, any kind of ifindex allocator can be replace this
 *                 
 *
 * Arguments     : spt_map  - Slot Port Type Scope Vs Ifindex maping
 *               : ifsv_cb  - interface service control block 
 *               : evt      - Pointer to return the sync resp
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_ifap_ifindex_alloc (NCS_IFSV_SPT_MAP *spt_map,
                         IFSV_CB *ifsv_cb, NCSCONTEXT *evt)
{
   IFSV_EVT_SPT_MAP_INFO spt_info;
   uns32 res = NCSCC_RC_SUCCESS;
   
   m_NCS_MEMSET(&spt_info, 0, sizeof(IFSV_EVT_SPT_MAP_INFO));
   spt_info.app_svc_id   = NCS_SERVICE_ID_IFND;
   spt_info.app_dest     = ifsv_cb->my_dest;
   spt_info.spt_map.spt  = spt_map->spt;
   /* send a sync interface index req to the IfD*/
   res = ifnd_mds_msg_sync_send((NCSCONTEXT)&spt_info, IFD_EVT_IFINDEX_REQ, 
                                 ifsv_cb, evt);

   return (res);
}

/****************************************************************************
 * Name          : ifnd_ifap_ifindex_free
 *
 * Description   : This is the function which frees ifindex for the given 
 *                 slot, port, type and scope, so that this Ifindex can be allocated 
 *                 later. This function will send an interface ifindex cleanup
 *                 event to the IfD.
 *                 
 *
 * Arguments     : spt_map  - Slot Port Type Scope Vs Ifindex maping
 *               : ifsv_cb        - interface service control block 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_ifap_ifindex_free (NCS_IFSV_SPT_MAP *spt_map, 
                        IFSV_CB *ifsv_cb)
{
   IFSV_EVT_SPT_MAP_INFO spt_map_info;
   uns32 res = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&spt_map_info, 0, sizeof(IFSV_EVT_SPT_MAP_INFO));
   spt_map_info.app_svc_id    = NCS_SERVICE_ID_IFND;
   spt_map_info.app_dest      = ifsv_cb->my_dest;
   m_NCS_MEMCPY(&spt_map_info.spt_map,spt_map, sizeof(NCS_IFSV_SPT_MAP));

   /* send an interface index cleanup to the IfD */
   res = ifnd_mds_msg_send((NCSCONTEXT)&spt_map_info, 
      IFD_EVT_IFINDEX_CLEANUP, ifsv_cb);   
   return (res);
}

/****************************************************************************
 * Name          : ifnd_intf_ifinfo_send
 *
 * Description   : This is the function to send a response to Interface Get
 *                 request from Applications
 *                 
 *
 * Arguments     : get_info   - Get Info.. 
 *               : intf_data  - Interface data.
 *                 attr       - Attributes which has been changed.
 *                 err        - Error.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 *****************************************************************************/
uns32 
ifnd_intf_ifinfo_send (IFSV_CB *cb,  IFSV_EVT_INTF_INFO_GET  *if_get, 
                       IFSV_SEND_INFO *sinfo)
{
   IFSV_EVT    *evt = NULL;
   uns32       rc = NCSCC_RC_SUCCESS;
   uns32       ifindex=0;
   NCS_IFSV_IFGET_RSP   *ifget_rsp;
   NCS_IFSV_INTF_REC    *add_upd;
   IFSV_INTF_DATA       *intf_data=NULL;
   IFSV_INTF_REC        *rec;
#if (NCS_IFSV_BOND_INTF == 1)
   IFSV_INTF_DATA       *temp_intf_data=NULL;
#endif

   /* Allocate the event data structure */
   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
   {
       m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
       return NCSCC_RC_FAILURE;
   }

   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0);

   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));
   evt->info.ifa_evt.usrhdl = (NCSCONTEXT)if_get->usr_hdl;

   if(if_get->get_type == IFSV_INTF_GET_ALL)
   {
      
      /* Check for the state, go ahead only if IfND is in Oper state. */
      if(cb->ifnd_state != NCS_IFSV_IFND_OPERATIONAL_STATE)
      {
        rc = NCSCC_RC_FAILURE; 
        m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                 "Error : ifnd_intf_ifinfo_send : cb->ifnd_state is = ",cb->ifnd_state);
        goto free_mem;
      }

      /* Get All Request will come only in case of new application is comming 
      UP, in this case send the ADD notify events to applications */
      evt->type = IFA_EVT_INTF_CREATE;
      add_upd = &evt->info.ifa_evt.info.if_add_upd;

      ifindex = 0;
      rec = (IFSV_INTF_REC*)ncs_patricia_tree_getnext
                                  (&cb->if_tbl, (uns8*)&ifindex);

      while(rec)
      {
         add_upd->info_type = NCS_IFSV_IF_INFO;
         add_upd->if_index  = rec->intf_data.if_index;
         add_upd->spt_info  = rec->intf_data.spt_info;
         add_upd->if_info   = rec->intf_data.if_info;
         /* Fill scope, it will be used by Agent. */
         evt->info.ifa_evt.subscr_scope = rec->intf_data.spt_info.subscr_scope;

         rc = ifsv_intf_info_cpy(&rec->intf_data.if_info, 
                &add_upd->if_info, IFSV_MALLOC_FOR_MDS_SEND);
   
         rc = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, evt,
                 if_get->my_dest, if_get->my_svc_id);

         if(rc != NCSCC_RC_SUCCESS)
         {
            ifsv_intf_info_free(&add_upd->if_info, IFSV_MALLOC_FOR_MDS_SEND);
            m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                 "ifnd_intf_ifinfo_send : ifsv_mds_normal_send failed, ifindex ",add_upd->if_index);
            goto free_mem;
         }
         
         /* Get the next record */
         rec = (IFSV_INTF_REC*)ncs_patricia_tree_getnext
                          (&cb->if_tbl, (uns8*)&rec->intf_data.if_index);
      }

      goto free_mem;
   }
   else  /* Send One record */
   {
      evt->type = IFA_EVT_IFINFO_GET_RSP;
      ifget_rsp = &evt->info.ifa_evt.info.ifget_rsp;
      
      /* Check for the state, go ahead only if IfND is in Oper state. */
      if(cb->ifnd_state != NCS_IFSV_IFND_OPERATIONAL_STATE)
      {
         evt->error = NCS_IFSV_IFND_RESTARTING_ERROR;
         
         ifget_rsp->error = NCS_IFSV_IFGET_RSP_NO_REC;
         ifget_rsp->if_rec.info_type = NCS_IFSV_IF_INFO;
         if(if_get->ifkey.type == NCS_IFSV_KEY_SPT)
            ifget_rsp->if_rec.spt_info = if_get->ifkey.info.spt;
         else
            ifget_rsp->if_rec.if_index = if_get->ifkey.info.iface;
       /* Send the resp to IFA */
       if(sinfo->stype == MDS_SENDTYPE_RSP)
       {
         /* Sync resp */
         rc = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, sinfo, evt);
       }
       else 
         rc = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, evt,
                                  if_get->my_dest, if_get->my_svc_id); 
       goto free_mem;
      } /* if(cb->ifnd_state != NCS_IFSV_IFND_OPERATIONAL_STATE) */
      
      if(if_get->ifkey.type == NCS_IFSV_KEY_SPT)
      {
         rc = ifsv_get_ifindex_from_spt(&ifindex, if_get->ifkey.info.spt, cb);
      }
      else
      {
         ifindex = if_get->ifkey.info.iface;
      }

      if(ifindex)
      {
         intf_data = ifsv_intf_rec_find(ifindex, cb);
      }

#if (NCS_IFSV_BOND_INTF == 1)
      if(if_get->info_type == NCS_IFSV_BIND_GET_LOCAL_INTF)
      {
         if(intf_data && intf_data->spt_info.type == NCS_IFSV_INTF_BINDING)
         {
            ifget_rsp->error = NCS_IFSV_IFGET_RSP_SUCCESS;
            ifget_rsp->if_rec.info_type = NCS_IFSV_BIND_GET_LOCAL_INTF;
            ifget_rsp->if_rec.if_index = intf_data->if_index;
            ifget_rsp->if_rec.spt_info = intf_data->spt_info;
            ifget_rsp->if_rec.if_info = intf_data->if_info;
            /* Get the local interface and fill it in bond_master_interface */
            temp_intf_data = ifsv_intf_rec_find(intf_data->if_info.bind_master_ifindex, cb);
            if(temp_intf_data == NULL)
                /* set the local node to 0 not a valid scenario */
                ifget_rsp->if_rec.if_info.bind_master_ifindex = 0;
            else
            {
               if((cb->shelf == temp_intf_data->spt_info.shelf) &&
                  (cb->slot  == temp_intf_data->spt_info.slot))
               {
                   /* this means that the master interface is the local node */;
               }
               else
               {              
                  temp_intf_data = ifsv_intf_rec_find(intf_data->if_info.bind_slave_ifindex, cb);
                  if(temp_intf_data && (cb->shelf == temp_intf_data->spt_info.shelf) &&
                                    (cb->slot  == temp_intf_data->spt_info.slot))
                  {
                    /* the slave is the local node */
                    ifget_rsp->if_rec.if_info.bind_master_ifindex =  
                          ifget_rsp->if_rec.if_info.bind_slave_ifindex;
                  }
                  else
                  {
                    ifget_rsp->if_rec.if_info.bind_master_ifindex = 0;
                    ifget_rsp->if_rec.if_info.bind_slave_ifindex = 0;
                  }
               }
            }
            ifget_rsp->if_rec.if_info.bind_slave_ifindex = 0; 
            /* Fill scope, it will be used by Agent. */
            evt->info.ifa_evt.subscr_scope = intf_data->spt_info.subscr_scope;
         }
      }
      else
#endif
      if(intf_data)
      {
         ifget_rsp->error = NCS_IFSV_IFGET_RSP_SUCCESS;
         ifget_rsp->if_rec.info_type = NCS_IFSV_IF_INFO;
         ifget_rsp->if_rec.if_index = intf_data->if_index;
         ifget_rsp->if_rec.spt_info = intf_data->spt_info;
         ifget_rsp->if_rec.if_info = intf_data->if_info;
         /* Fill scope, it will be used by Agent. */
         evt->info.ifa_evt.subscr_scope = intf_data->spt_info.subscr_scope;
      }
      else
      {
         ifget_rsp->error = NCS_IFSV_IFGET_RSP_NO_REC;
         ifget_rsp->if_rec.info_type = NCS_IFSV_IF_INFO;
         if(if_get->ifkey.type == NCS_IFSV_KEY_SPT)
            ifget_rsp->if_rec.spt_info = if_get->ifkey.info.spt;
         else
            ifget_rsp->if_rec.if_index = if_get->ifkey.info.iface;
      }

      /* Send the resp to IFA */
      if(sinfo->stype == MDS_SENDTYPE_RSP)
      {
        /* Sync resp */
        evt->error = NCS_IFSV_NO_ERROR;
        rc = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, sinfo, evt);
      }
      else 
        rc = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, evt,
                                  if_get->my_dest, if_get->my_svc_id); 

      if(rc != NCSCC_RC_SUCCESS)
         goto free_mem;
   }
free_mem:
   m_MMGR_FREE_IFSV_EVT(evt);
   return (rc);
}

/****************************************************************************
 * Name          : ifnd_intf_statsinfo_send
 *
 * Description   : This is the function to send a response to Intf Stats Get
 *                 request from Applications
 *                 
 *
 * Arguments     : get_info   - Get Info.. 
 *               : intf_data  - Interface data.
 *                 attr       - Attributes which has been changed.
 *                 err        - Error.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 *****************************************************************************/
uns32 
ifnd_intf_statsinfo_send (IFSV_CB *cb,  IFSV_EVT_STATS_INFO  *stats_info)
{
   IFSV_EVT             *evt = NULL;
   uns32                rc = NCSCC_RC_SUCCESS;
   NCS_IFSV_IFGET_RSP   *ifget_rsp;

      /* Allocate the event data structure */
   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return NCSCC_RC_FAILURE;
   }

   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_STATE_INFO, cb->ifnd_state, 0);

   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));
   evt->info.ifa_evt.usrhdl = (NCSCONTEXT)stats_info->usr_hdl;
   evt->type = IFA_EVT_IFINFO_GET_RSP;

   ifget_rsp = &evt->info.ifa_evt.info.ifget_rsp;
   if(stats_info->status)
      ifget_rsp->error = stats_info->status;
   else
      ifget_rsp->error = NCS_IFSV_IFGET_RSP_NO_REC;
   ifget_rsp->if_rec.info_type = NCS_IFSV_IFSTATS_INFO;
   ifget_rsp->if_rec.spt_info  = stats_info->spt_type;
   /* Fill scope, which will be used by Agent */
   evt->info.ifa_evt.subscr_scope = stats_info->spt_type.subscr_scope;

   rc = ifsv_get_ifindex_from_spt(&ifget_rsp->if_rec.if_index, 
                                   ifget_rsp->if_rec.spt_info, cb);
   ifget_rsp->if_rec.if_stats = stats_info->stat_info;

   rc = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, evt,
                 stats_info->dest, stats_info->svc_id); 

   if(rc != NCSCC_RC_SUCCESS)
      goto free_mem;

free_mem:
   m_MMGR_FREE_IFSV_EVT(evt);
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifsv_ifnd_ifindex_alloc
 *
 * Description   : This is the function which allocates ifindex for the given 
 *                  slot, port, type and scope.
 *                 
 *
 * Arguments     : spt_map  - Slot Port Type Scope Vs Ifindex maping
 *               : ifsv_cb  - interface service control block 
 *               : evt      - message pointer to return sync resp
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifsv_ifnd_ifindex_alloc (NCS_IFSV_SPT_MAP *spt_map, IFSV_CB *ifsv_cb, NCSCONTEXT* evt)
{   
   uns32 res = NCSCC_RC_SUCCESS;

   if ((res = m_IFND_IFAP_IFINDEX_ALLOC(spt_map, ifsv_cb, evt)) 
            != NCSCC_RC_SUCCESS)
   {
#if 0
      m_IFND_LOG_HEAD_LINE(IFSV_LOG_IFINDEX_ALLOC_FAILURE,spt_map->spt.slot,\
         spt_map->spt.port);
#else
       m_IFSV_LOG_SPT_INFO(IFSV_LOG_IFINDEX_ALLOC_FAILURE,spt_map,"ifsv_ifnd_ifindex_alloc");
#endif
      return (res);
   }      
   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_IFINDEX_ALLOC_SUCCESS, spt_map->spt.slot,\
         spt_map->spt.port);
   return (res);
}


/****************************************************************************
 * Name          : ifsv_ifindex_free
 *
 * Description   : This is the function give the ifindex to the ifindex free 
 *                 pool so that it can be allocated again for some other 
 *                 interfaces.
 *                 
 *
 * Arguments     : spt_map  - Slot Port Type Scope Vs Ifindex maping
 *               : ifsv_cb  - interface service control block 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifsv_ifindex_free (IFSV_CB *ifsv_cb, NCS_IFSV_SPT_MAP *spt_map)
{  
   
   if (m_IFND_IFAP_IFINDEX_FREE(spt_map,ifsv_cb) == NCSCC_RC_FAILURE)
   {
      m_IFND_LOG_HEAD_LINE(IFSV_LOG_IFINDEX_FREE_FAILURE,spt_map->if_index,\
         spt_map->spt.port);
      return (NCSCC_RC_FAILURE);
   }         

   m_IFND_LOG_HEAD_LINE_NORMAL(IFSV_LOG_IFINDEX_FREE_SUCCESS,spt_map->if_index,\
      spt_map->spt.port);
   return (NCSCC_RC_SUCCESS);
}


/****************************************************************************
 * Name          : ifsv_ifa_app_svd_info_indicate
 *
 * Description   : This is the function to inform applications (IFA) about 
 *                 the Interface Changes
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
ifsv_ifa_app_svd_info_indicate(IFSV_CB *cb, IFSV_INTF_DATA *actual_data, 
                            NCS_IFSV_SVC_DEST_UPD *svc_dest)
{
   IFSV_EVT evt;
   uns32    rc = NCSCC_RC_SUCCESS;
      
   m_NCS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
      
   evt.type = IFA_EVT_INTF_UPDATE;
   evt.info.ifa_evt.info.if_add_upd.if_index = actual_data->if_index;
   evt.info.ifa_evt.info.if_add_upd.info_type = NCS_IFSV_IF_INFO;
   evt.info.ifa_evt.info.if_add_upd.if_info.if_am = NCS_IFSV_IAM_SVDEST;
   evt.info.ifa_evt.info.if_add_upd.spt_info = actual_data->spt_info;

   if(svc_dest->i_type == NCS_IFSV_SVCD_ADD)
   {
      evt.info.ifa_evt.info.if_add_upd.if_info.addsvd_cnt = 1;
      evt.info.ifa_evt.info.if_add_upd.if_info.addsvd_list = 
        (NCS_SVDEST *) m_MMGR_ALLOC_IFSV_NCS_SVDEST(1);

      if(evt.info.ifa_evt.info.if_add_upd.if_info.addsvd_list == 0)
      {
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
         return NCSCC_RC_OUT_OF_MEM;
      }
      m_NCS_OS_MEMCPY(evt.info.ifa_evt.info.if_add_upd.if_info.addsvd_list, 
                      &svc_dest->i_svdest, sizeof(NCS_SVDEST));
   }
   else if(svc_dest->i_type == NCS_IFSV_SVCD_DEL)
   {
      evt.info.ifa_evt.info.if_add_upd.if_info.delsvd_cnt = 1;
      evt.info.ifa_evt.info.if_add_upd.if_info.delsvd_list = 
        (NCS_SVDEST *)  m_MMGR_ALLOC_IFSV_NCS_SVDEST(1);

      if(evt.info.ifa_evt.info.if_add_upd.if_info.delsvd_list == 0)
      {
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
         return NCSCC_RC_OUT_OF_MEM;
      }
      m_NCS_OS_MEMCPY(evt.info.ifa_evt.info.if_add_upd.if_info.delsvd_list, 
                      &svc_dest->i_svdest, sizeof(NCS_SVDEST));
   }
   else
      return NCSCC_RC_FAILURE;
      
   /* Broadcast the event to all the IFAs in on the same node */
   rc = ifsv_mds_scoped_send(cb->my_mds_hdl, NCSMDS_SCOPE_INTRANODE, 
         NCSMDS_SVC_ID_IFND, &evt, NCSMDS_SVC_ID_IFA);

   if(rc != NCSCC_RC_SUCCESS)
   {
      if(evt.info.ifa_evt.info.if_add_upd.if_info.addsvd_list)
         m_MMGR_FREE_IFSV_NCS_SVDEST(evt.info.ifa_evt.info.if_add_upd.if_info.addsvd_list);

      if(evt.info.ifa_evt.info.if_add_upd.if_info.delsvd_list)
         m_MMGR_FREE_IFSV_NCS_SVDEST(evt.info.ifa_evt.info.if_add_upd.if_info.delsvd_list);
   }
      
#if(NCS_IFSV_IPXS == 1)
      /* Inform the IPXS applications */
      rc = ipxs_ifa_app_svd_info_indicate(cb, actual_data, svc_dest);

#endif

   return (rc);
}

/****************************************************************************
 * Name          : ifnd_sync_send_to_ifd
 *
 * Description   : This is the function which is used to do sync send 
 *                 information from IfND to IfD.
 *
 * Arguments     : intf_data  - Interface data to be checkpointed.
 *               : rec_evt    - Action on the interface record.
 *                 attr       - Attribute of the rec data which has changed.
 *                 cb         - IFSV Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_sync_send_to_ifd (IFSV_INTF_DATA *intf_data, IFSV_INTF_REC_EVT rec_evt,
                    uns32 attr, IFSV_CB *cb, IFSV_EVT **o_evt)
{
   IFSV_EVT  *evt, *msg_rcvd = NULL;
   uns32 res = NCSCC_RC_FAILURE;
   char log_info[45];

   evt = m_MMGR_ALLOC_IFSV_EVT;

   if (evt == IFSV_NULL)
   {
      m_IFSV_LOG_SYS_CALL_FAIL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
               IFSV_LOG_MEM_ALLOC_FAIL,0);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));

   if ((rec_evt == IFSV_INTF_REC_ADD) || (rec_evt == IFSV_INTF_REC_MODIFY))
   {
     evt->type = IFD_EVT_INTF_CREATE;

     m_NCS_MEMCPY(&evt->info.ifd_evt.info.intf_create.intf_data,
                  intf_data, sizeof(IFSV_INTF_CREATE_INFO));
     /* Here while sending the record to IFD, it will keep IfND's
      * destination address in the record info, so that IfD will
      * delete all the records created by the node, when the node
      * goes down
      */
     evt->info.ifd_evt.info.intf_create.if_attr = attr;

     m_IFND_LOG_EVT_LL(IFSV_LOG_IFD_EVT_INTF_CREATE_SND,\
      ifsv_log_spt_string(intf_data->spt_info,log_info),\
      intf_data->if_index,attr);
   } else
   {
     evt->type = IFD_EVT_INTF_DESTROY;
     evt->info.ifd_evt.info.intf_destroy.spt_type = intf_data->spt_info;
     evt->info.ifd_evt.info.intf_destroy.own_dest =
                               intf_data->current_owner_mds_destination;

     m_IFND_LOG_EVT_L(IFSV_LOG_IFD_EVT_INTF_DESTROY_SND,\
     ifsv_log_spt_string(intf_data->spt_info,log_info),intf_data->if_index);
   }

   /* send the information to the IfD */
   res = ifsv_mds_msg_sync_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                              NCSMDS_SVC_ID_IFD, cb->ifd_dest,
                              (NCSCONTEXT)evt, &msg_rcvd,
                              (IFSV_MDS_SYNC_TIME));
   *o_evt = msg_rcvd;

   m_MMGR_FREE_IFSV_EVT(evt);
   return(res);
} /* The function ifnd_sync_send_to_ifd() ends here. */


/**************************************************************
* functions for processing individual event received by IfND
**************************************************************/
/****************************************************************************
 * Name          : ifnd_init_vip_db
 *
 * Description   : This function used to initialize the VIPDC
 *
 * Arguments     : cb - This is the ptr to IfND control block
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

#if(NCS_VIP == 1)
uns32 ifnd_init_vip_db(IFSV_CB *cb)
{
      NCS_PATRICIA_PARAMS params;
      m_NCS_MEMSET(&params,0,sizeof(NCS_PATRICIA_PARAMS));
      params.key_size = sizeof(NCS_IFSV_VIP_INT_HDL);
      params.info_size = 0;
      if ((ncs_patricia_tree_init(&cb->vipDBase, &params))
         != NCSCC_RC_SUCCESS)
      {
         /* LOG : VIPDC DATABASE CREATE FAIL */
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_VIPDC_DATABASE_CREATE_FAILED);
         m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                 "ifnd_init_vip_db : ncs_patricia_tree_init failed",0);

         return NCSCC_RC_FAILURE;
      }
      /* LOG : VIPDC database created */
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFND,IFSV_VIP_VIPDC_DATABASE_CREATED);
      return NCSCC_RC_SUCCESS;
}
#endif
