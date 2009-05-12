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
  FILE NAME: IFDPROC.C

  DESCRIPTION: IfD initalization and destroy routines. Create/Modify/
               Destory interface record operations.

  FUNCTIONS INCLUDED in this module:
  ifd_intf_create ......... Creation/Modification of interface record.
  ifd_intf_delete ......... Delete interface record.
  ifd_all_intf_rec_del .... Delete all the record in the interface db.  
  ifd_same_dst_all_intf_rec_mark_del... Marks delete to all the record belong 
                                        to the same IfND dest.
  ifd_same_node_id_all_intf_rec_del... Delete the intf rec created by IfND.
  ifd_clear_mbx ........... Clean the IfND mail box.
  ifd_amf_init...... ...... Inialize AMF for IfD.
  ifd_init_cb...... ....... IfD CB inialize.    
  ifsv_ifd_ifindex_alloc ...... Allocates Ifindex.
  ifsv_ifindex_free ....... Free Ifindex.

******************************************************************************/

#include "ifd.h"
#include "ifd_red.h"
static void ifd_check_if_master_slave(NCS_IFSV_IFINDEX ifindex,IFSV_CB* ifsv_cb);

/****************************************************************************
 * Name          : ifd_intf_create
 *
 * Description   : This function will be called when the interface is created
 *                 or the interface parameter gets modified.
 *
 * Arguments     : ifsv_cb     - pointer to the interface Control Block.
 *               : create_intf - This is the strucutre used to carry 
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
ifd_intf_create (IFSV_CB *ifsv_cb, IFSV_INTF_CREATE_INFO *create_intf,
                 IFSV_SEND_INFO *sinfo)
{
   NCS_IFSV_SPT_MAP   spt_map;
   IFSV_INTF_DATA     *rec_data;   
   NCS_LOCK           *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   IFSV_INTF_REC      *rec;
   uns32              ifindex = 0, temp_ifindex = 0;
   uns32              res = NCSCC_RC_SUCCESS, res1=NCSCC_RC_SUCCESS;   
   IFSV_EVT     send_evt;
   memset(&send_evt, 0, sizeof(IFSV_EVT));

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   /* this is the case where the interface record is already in the 
    * interface record */
   if ((ifsv_get_ifindex_from_spt(&ifindex, create_intf->intf_data.spt_info,
      ifsv_cb) == NCSCC_RC_SUCCESS) &&
      ((rec_data = ifsv_intf_rec_find(ifindex, ifsv_cb)) != IFSV_NULL))
   {    
      res = ifsv_intf_rec_modify(rec_data, &create_intf->intf_data,
                                 &create_intf->if_attr, ifsv_cb);           

     if(res == NCSCC_RC_SUCCESS)
     {
      /* Send the trigger point to Standby IfD. */
      res1 = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                 (uns8 *)(&create_intf->intf_data));
     }

      send_evt.type = IFND_EVT_INTF_CREATE_RSP;

      if(res == NCSCC_RC_SUCCESS)
          send_evt.error = NCS_IFSV_NO_ERROR;
      else
          send_evt.error = NCS_IFSV_INTF_MOD_ERROR;

      send_evt.info.ifnd_evt.info.spt_map.spt_map.spt = 
                                   create_intf->intf_data.spt_info;
      send_evt.info.ifnd_evt.info.spt_map.spt_map.if_index = 
                                   create_intf->intf_data.if_index;

      /* Sync resp to IfND.*/
      ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl,
                           NCSMDS_SVC_ID_IFD, sinfo,
                           &send_evt);
      if(res == NCSCC_RC_SUCCESS)
      /* Send modified record to all IfD.*/
      ifd_bcast_to_ifnds(&create_intf->intf_data, IFSV_INTF_REC_MODIFY, 
                            create_intf->if_attr, ifsv_cb);
   } else
   {
      /* This is the case where the interface record is created newly, which 
       * might have came from interface driver or from the IFND where IFD 
       * stores other interfaces present in other cards */
      
      if ((m_NCS_NODE_ID_FROM_MDS_DEST(create_intf->intf_data.originator_mds_destination) ==
          m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest)) && (create_intf->intf_data.originator ==
          NCS_IFSV_EVT_ORIGN_IFD))
      {         
         /* If am the owner of this record, here assume that the ifindex
          * will be resolved asynchronously always 
          */

         /* Fill the current owner information. */
         create_intf->intf_data.current_owner = NCS_IFSV_OWNER_IFD;
         create_intf->intf_data.current_owner_mds_destination = ifsv_cb->my_dest;

         memset(&spt_map,0,sizeof(NCS_IFSV_SPT_MAP));
         spt_map.spt = create_intf->intf_data.spt_info;
         if ((res = ifsv_ifd_ifindex_alloc(&spt_map,ifsv_cb)) == NCSCC_RC_SUCCESS)
         {            

            create_intf->intf_data.if_index = spt_map.if_index;

            /* add the interface record in to the interface database */
            if (ifsv_intf_rec_add(&create_intf->intf_data, ifsv_cb)
               != NCSCC_RC_SUCCESS)
            {
               m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_intf_rec_add() returned failure"," ");
               res = NCSCC_RC_FAILURE;               
            } else if (ifsv_ifindex_spt_map_add(&spt_map, ifsv_cb)
               != NCSCC_RC_SUCCESS)
            {
               m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifindex_spt_map_add() returned failure"," ");
               res = NCSCC_RC_FAILURE;
               rec = ifsv_intf_rec_del(create_intf->intf_data.if_index,
                  ifsv_cb);     
               if (rec != IFSV_NULL)
               {
                  m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_intf_rec_del() returned failure"," ");
                  m_MMGR_FREE_IFSV_INTF_REC(rec);
               } else
               {
                  res = NCSCC_RC_FAILURE;
               }
            }
            
            if(res == NCSCC_RC_SUCCESS) 
            {
             m_IFSV_LOG_SPT_INFO(IFSV_LOG_IF_TBL_ADD_SUCCESS,&spt_map,"ifd_intf_create : ifd owner");

             /* Send the trigger point to Standby IfD. */
             res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_SPT_MAP_MAP_CREATE_MSG,
                                       (uns8 *)(&spt_map));
             if(res != NCSCC_RC_SUCCESS)
             {
               /* Log the error */
                  m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_async_update() returned failure"," ");
             }
               
             /* Send the trigger point to Standby IfD. */
             res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_CREATE_MSG,
                                        (uns8 *)(&create_intf->intf_data));
             if(res != NCSCC_RC_SUCCESS)
             {
               /* Log the error. */
                  m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_async_update() returned failure"," ");
             }
             /* Send modified record to all IfD.*/
             ifd_bcast_to_ifnds(&create_intf->intf_data, IFSV_INTF_REC_ADD, 
                                0, ifsv_cb);
           }
         } else
         {
            res = NCSCC_RC_FAILURE;
         }

      } else
      {
         /* If am not the owner of the record */
         /* add the interface record in to the interface database */
         /* First check that whether the SPT MAP related to this intf
            record is there or not. It might not be there in the case :
            1. If Index Alloc message comes to IfD from IFND, IfD allocates IfIndex.
            2. then next message is Warm Sync req, IfD removes one extra
               SPT MAP in function ifd_mbcsv_db_ifrec_sptmap_mismatch_correction().
            3. Then IfND sends Intf Rec Add to IFD corresponding to IfIndex allocated.
               Here, SPT MAP may not be present.
            So, we will not add IfRec in our data base, we will return NCS_IFSV_SYNC_TIMEOUT
            as error to IfND. At IfND, this error will be sent to IfA, where IfA will send try again
            message to Application.
         */
         if (ifsv_get_ifindex_from_spt(&temp_ifindex, create_intf->intf_data.spt_info,
            ifsv_cb) != NCSCC_RC_SUCCESS)
         {
              /* This SPT MAP is not in the record. So we will reject this request. */
                send_evt.type = IFND_EVT_INTF_CREATE_RSP;
                send_evt.error = NCS_IFSV_SYNC_TIMEOUT;

                send_evt.info.ifnd_evt.info.spt_map.spt_map.spt
                   = create_intf->intf_data.spt_info;
                send_evt.info.ifnd_evt.info.spt_map.spt_map.if_index =
                     create_intf->intf_data.if_index;
                m_IFD_LOG_FUNC_ENTRY_INFO(IFSV_LOG_IFD_MSG, "Error : No Spt Map for Intf Rec with Ifindex, Shelf, Slot, Port, Type, Scope, mds destination",create_intf->intf_data.if_index, create_intf->intf_data.spt_info.shelf, create_intf->intf_data.spt_info.slot, create_intf->intf_data.spt_info.port, create_intf->intf_data.spt_info.type, create_intf->intf_data.spt_info.subscr_scope, create_intf->intf_data.originator_mds_destination);
                printf(" error: ifindex - %d ***** No spt map for Intf s/s/ss/p/t/s- %d/%d/%d/%d/%d/%d \n",  create_intf->intf_data.if_index, create_intf->intf_data.spt_info.shelf, create_intf->intf_data.spt_info.slot, create_intf->intf_data.spt_info.subslot, create_intf->intf_data.spt_info.port, create_intf->intf_data.spt_info.type, create_intf->intf_data.spt_info.subscr_scope);
 
               /* Sync resp to IfND.*/
               ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl,
                                 NCSMDS_SVC_ID_IFD, sinfo,
                                 &send_evt);
                m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
                return (res); 
         }

         res = ifsv_intf_rec_add(&create_intf->intf_data, ifsv_cb);          

         if(res == NCSCC_RC_SUCCESS)
         {
           /* The following code is for logging */
           memset(&spt_map, 0, sizeof(NCS_IFSV_SPT_MAP));
           spt_map.spt      = create_intf->intf_data.spt_info;
           spt_map.if_index = create_intf->intf_data.if_index;
           m_IFSV_LOG_SPT_INFO(IFSV_LOG_IF_TBL_ADD_SUCCESS,&spt_map,"ifd_intf_create owner not ifd");
          /* Till here */

          /* Send the trigger point to Standby IfD. */
          res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_CREATE_MSG,
                                    (uns8 *)(&create_intf->intf_data));
         }

         send_evt.type = IFND_EVT_INTF_CREATE_RSP;
         if(res == NCSCC_RC_SUCCESS)
            send_evt.error = NCS_IFSV_NO_ERROR;
         else
            send_evt.error = NCS_IFSV_INT_ERROR;

         send_evt.info.ifnd_evt.info.spt_map.spt_map.spt
                                    = create_intf->intf_data.spt_info;
         send_evt.info.ifnd_evt.info.spt_map.spt_map.if_index =
                                   create_intf->intf_data.if_index;

         /* Sync resp to IfND.*/
         ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl,
                           NCSMDS_SVC_ID_IFD, sinfo,
                           &send_evt);

         if(res == NCSCC_RC_SUCCESS)
         {
          ifd_bcast_to_ifnds(&create_intf->intf_data,IFSV_INTF_REC_ADD, 0, ifsv_cb);
#if(NCS_IFSV_BOND_INTF == 1)
          ifd_check_if_master_slave(create_intf->intf_data.if_index,ifsv_cb);
#endif
         }
      }
/* If the newly created interface at IFD is binding interface, master's master and slave's slave
   at standby IFD and IFNDs should also be updated
   We could have handled this as part of data creation handler at standby and interface creation at
   IFNDs. But we r doing this way because, when standby IFD or IFND is coming up and cold sync happens
   if the binding index is smaller then master/slave ifindex, it is the one which gets added first than
   master/slave. This will return a failure as master/slave record is not created yet and we r trying to modify
   it.
*/
#if(NCS_IFSV_BOND_INTF == 1)
            if(res==NCSCC_RC_SUCCESS && create_intf->intf_data.spt_info.type == NCS_IFSV_INTF_BINDING)
            {
               IFSV_INTF_DATA* bond_data;
               IFSV_INTF_ATTR nd_attr = 0;
               bond_data = ifsv_intf_rec_find(create_intf->intf_data.if_index,ifsv_cb);
               if(bond_data != NULL)
               {
                bond_data->if_info.if_am = NCS_IFSV_BINDING_CHNG_IFINDEX;
                res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                          (uns8 *)(bond_data));
                if( res!=  NCSCC_RC_SUCCESS)
                m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_a2s_async_update() returned failure for DATA_MODIFY"," ");
                nd_attr = NCS_IFSV_BINDING_CHNG_IFINDEX;
                ifd_bcast_to_ifnds(bond_data, IFSV_INTF_REC_MODIFY,nd_attr, ifsv_cb);
               }
            }

#endif
   }
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
   return (res); 
}


/****************************************************************************
 * Name          : ifd_intf_delete
 *
 * Description   : This function will be called when the interface operation 
 *                 status is set to DOWN or the configuration management 
 *                 deletes the interface. 
 *
 * Arguments     : ifsv_cb     - pointer to the interface Control Block.
 *               : create_intf - This is the strucutre used to carry 
 *                 information about the newly created interface or the 
 *                 modified parameters. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Whenever the configuration management or the interface 
 *                 driver needs to send a interface delete or operation 
 *                 status down, than it would send the vcard ID as "0".
 *****************************************************************************/

uns32
ifd_intf_delete (IFSV_CB *ifsv_cb, 
                 IFSV_INTF_DESTROY_INFO *dest_info,
                 IFSV_SEND_INFO *sinfo)
{
   IFSV_INTF_REC      *rec;
   IFSV_INTF_DATA *rec_data;
   NCS_LOCK       *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   uns32 res = NCSCC_RC_SUCCESS;
   uns32 ifindex;   
   IFSV_INTF_ATTR attr = 0;
#if(NCS_IFSV_BOND_INTF == 1)
   IFSV_INTF_DATA *bonding_data;
   NCS_IFSV_IFINDEX bonding_ifindex;
#endif

   if (ifsv_get_ifindex_from_spt(&ifindex, dest_info->spt_type, 
      ifsv_cb) == NCSCC_RC_SUCCESS)
   {
      m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
      if ((rec_data = ifsv_intf_rec_find (ifindex, ifsv_cb))
         != IFSV_NULL)
      {
#if(NCS_IFSV_BOND_INTF == 1)      
          if(rec_data->spt_info.type == NCS_IFSV_INTF_BINDING)
             {
   /*If binding interface is getting deleted, master's master and slave's slave should be set to 0 */
               ifsv_delete_binding_interface(ifsv_cb,ifindex);
             }
         else if((res = ifd_binding_check_ifindex_is_master(ifsv_cb,ifindex,&bonding_ifindex)) == NCSCC_RC_SUCCESS) 
          {
              /* If the master is getting deleted, make current slave the new master, the slave ifindex is made 0
              as it is getting deleted */
               if((bonding_data = ifsv_binding_delete_master_ifindex(ifsv_cb,bonding_ifindex)) != NULL) 
               {
                   /* Send the trigger point to Standby IfD. */
                  bonding_data->if_info.if_am = NCS_IFSV_BINDING_CHNG_IFINDEX;
                  res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                         (uns8 *)(bonding_data));
                 /* no need to bcast data modify to IFNDs as this being handled as part of delete record at IFNDs
                     IFND has to send MASTER_CHNG message to applications  */
               }
          }
        else if((res = ifsv_binding_check_ifindex_is_slave(ifsv_cb,ifindex,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
          {
             /* if slave is getting deleted , make slave_ifindex of bonding data 0 */
              bonding_data = ifsv_intf_rec_find(bonding_ifindex,ifsv_cb);
              if( bonding_data != IFSV_NULL )
              {
                  bonding_data->if_info.bind_slave_ifindex = 0;
                  bonding_data->if_info.if_am = NCS_IFSV_BINDING_CHNG_IFINDEX;
                  res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                         (uns8 *)(bonding_data));
                  /* no need to bcast data modify to IFNDs as this being handled as part of delete record at IFNDs*/
              }

          }

#endif        
     
         if (m_NCS_NODE_ID_FROM_MDS_DEST(rec_data->originator_mds_destination) == m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest))
         {
            /* This is the case where the delete command belongs to the same ifd*/
            /* Here only possible case where IfD gets the create/delete 
             * interface directly will be through CLI, so before deleting 
             * the record we need to check whether the deltion has been 
             * triggered by CLI 
             */
            if (m_NCS_IFSV_IS_MDS_DEST_SAME(&rec_data->originator_mds_destination,
               &dest_info->own_dest) == TRUE)
            {
               attr = NCS_IFSV_IAM_OPRSTATE;
               res = ifsv_intf_rec_marked_del (rec_data, &attr, ifsv_cb);

               /* Send the trigger point to Standby IfD. */
               res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MARKED_DELETE_MSG,
                                         (uns8 *)(rec_data));

               if(res == NCSCC_RC_SUCCESS)
                ifd_bcast_to_ifnds(rec_data, IFSV_INTF_REC_DEL, attr, ifsv_cb); 
            }
         } else
         {
            rec = ifsv_intf_rec_del (ifindex, ifsv_cb);            
            if (rec != NULL)
            {
              IFSV_EVT     send_evt;
              memset(&send_evt, 0, sizeof(IFSV_EVT));
              /* Send the trigger point to Standby IfD. */
               m_IFD_LOG_STR_NORMAL(IFSV_LOG_IF_TBL_DEL_SUCCESS," Ifindex :",ifindex);
/*
               m_IFSV_LOG_HEAD_LINE(ifsv_cb->my_svc_id,\
                                    IFSV_LOG_IF_TBL_DEL_SUCCESS,ifindex,0);
*/
              res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_DELETE_MSG,
                                         (uns8 *)(rec_data));

              send_evt.type = IFND_EVT_INTF_DESTROY_RSP;

              if(res == NCSCC_RC_SUCCESS)
                 send_evt.error = NCS_IFSV_NO_ERROR;
              else
                 send_evt.error = NCS_IFSV_INT_ERROR;

              send_evt.info.ifnd_evt.info.spt_map.spt_map.spt = 
                                   rec_data->spt_info;
              send_evt.info.ifnd_evt.info.spt_map.spt_map.if_index = 
                                   rec_data->if_index;

              /* Sync resp to IfND.*/
              ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl, NCSMDS_SVC_ID_IFD, 
                                sinfo,  &send_evt);

              /* Slot/Port Vs Ifindex maping will be removed only if the 
               * aging timer gets expired */
              /* Send an MDS message for delete */
              ifd_bcast_to_ifnds(&rec->intf_data, IFSV_INTF_REC_DEL, 0,
                                 ifsv_cb);
               
              m_MMGR_FREE_IFSV_INTF_REC(rec);
            }
         }
      }
      else
      {
        /* The down interface has come up before the aging timer expiry of IfND
           So, send the resp. */
           IFSV_EVT     send_evt;
           memset(&send_evt, 0, sizeof(IFSV_EVT));

           send_evt.type = IFND_EVT_INTF_DESTROY_RSP;
           send_evt.error = NCS_IFSV_INT_ERROR;

           send_evt.info.ifnd_evt.info.spt_map.spt_map.spt =
                                   dest_info->spt_type;
           send_evt.info.ifnd_evt.info.spt_map.spt_map.if_index =
                                   ifindex;

            /* Sync resp to IfND.*/
            ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl, NCSMDS_SVC_ID_IFD,
                                sinfo,  &send_evt);
      }
      m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
   }
   return (res);
}


/****************************************************************************
 * Name          : ifd_same_dst_all_intf_rec_mark_del
 *
 * Description   : This function will be called when the IfD found any IfND 
 *                 goes down. This will mark all the interfaces record 
 *                 belonging to that IfND as DEL. It will star retention timer
 *                 for the deleting interface rec created by that IfND. After 
 *                 marking DEL to a rec, it sends DEL event to all the 
 *                 IfNDs for this record.
 *
 * Arguments     : ifsv_cb     - pointer to the interface Control Block.
 *                 dest        - check for the destination which needs to 
 *                               be removed (dest of the app going down).
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Here we won't be deleting the ifindex Vs 
 *                 Shelf/slot/port/type/scope mapping, assuming that this IfND 
 *                 would come later (here we are making this assumption b/c 
 *                 this MDS down might come b/c of some MDS flapping, so there 
 *                 could be a possible case where IfND could come back again).
 *****************************************************************************/
uns32
ifd_same_dst_all_intf_rec_mark_del (MDS_DEST *dest, IFSV_CB *ifsv_cb)
{
   uns32 ifindex = 0;   
   NODE_ID node_id;
   uns32 res = NCSCC_RC_SUCCESS;
   uns32 res1 = NCSCC_RC_SUCCESS;
   IFSV_INTF_REC       *intf_rec;
   NCS_PATRICIA_NODE   *if_node;   
   NCS_LOCK            *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   IFD_IFND_NODE_ID_INFO_REC *rec = NULL;
   MDS_DEST mds_dest;

   memcpy(&mds_dest,dest, sizeof(MDS_DEST));
   node_id = m_NCS_NODE_ID_FROM_MDS_DEST(mds_dest);

   /* Add the node id of the DOWN IfND in the Patricia tree.*/ 
   res = ifd_ifnd_node_id_info_add(&rec, &node_id, ifsv_cb);

   if((res != NCSCC_RC_SUCCESS) || (rec == IFSV_NULL))
   {
     m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_ifnd_node_id_info_add() returned failure"," ");
     return (NCSCC_RC_FAILURE);
   }
   
   /* Plugin for marking vipd entries corresponding to crashed IfND
      as stale entries */
   ifd_vip_process_ifnd_crash(dest,ifsv_cb); 

   /* Send the trigger point to Standby IfD. */
   res1 = ifd_a2s_async_update(ifsv_cb, IFD_A2S_IFND_DOWN_MSG, (uns8 *)(dest));

   /* Starts a retention timer for this IfND.*/
   rec->info.tmr.tmr_type = NCS_IFSV_IFD_EVT_RET_TMR;
   rec->info.tmr.svc_id = ifsv_cb->my_svc_id;
   rec->info.tmr.info.node_id = node_id;
   ifsv_tmr_start(&rec->info.tmr, ifsv_cb);
   
   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
      (uns8*)&ifindex);
   intf_rec = (IFSV_INTF_REC*)if_node;

   while (intf_rec != IFSV_NULL)
   {
      ifindex = intf_rec->intf_data.if_index;
      if((node_id == 
          m_NCS_NODE_ID_FROM_MDS_DEST(intf_rec->intf_data.current_owner_mds_destination)) && 
         (intf_rec->intf_data.current_owner == NCS_IFSV_OWNER_IFND))
      {
         intf_rec->intf_data.active = FALSE;
         intf_rec->intf_data.if_info.oper_state = NCS_STATUS_DOWN;
         /* Send an MDS message for delete */
         ifd_bcast_to_ifnds(&intf_rec->intf_data, IFSV_INTF_REC_DEL, 0,
                             ifsv_cb);
      }
      
      if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
         (uns8*)&ifindex);
      
      intf_rec = (IFSV_INTF_REC*)if_node;
   }
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   return (res);
}

/****************************************************************************
 * Name          : ifd_same_node_id_all_intf_rec_del
 *
 * Description   : This function is called when retention timer for IfND node 
 *                 id expires, it deletes all the rec created by this IfND.
 *
 * Arguments     : node id  - node id of IfND.
 *                 ifsv_cb  - pointer to the interface Control Block.
 * 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *                 
 *****************************************************************************/
uns32
ifd_same_node_id_all_intf_rec_del (NODE_ID node_id, IFSV_CB *ifsv_cb)
{
   uns32 ifindex = 0;   
   uns32 res = NCSCC_RC_SUCCESS;
   IFSV_INTF_REC       *intf_rec;
   NCS_PATRICIA_NODE   *if_node;   
   NCS_LOCK            *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   NCS_IFSV_SPT_MAP    spt_map;
#if(NCS_IFSV_BOND_INTF == 1)
   IFSV_INTF_DATA *bonding_data;
   NCS_IFSV_IFINDEX bonding_ifindex;
#endif
   
   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
      (uns8*)&ifindex);
   intf_rec = (IFSV_INTF_REC*)if_node;

   while (intf_rec != IFSV_NULL)
   {
      ifindex = intf_rec->intf_data.if_index;
      if((node_id == m_NCS_NODE_ID_FROM_MDS_DEST(intf_rec->intf_data.current_owner_mds_destination))
        && (intf_rec->intf_data.current_owner == NCS_IFSV_OWNER_IFND))
      {
#if(NCS_IFSV_BOND_INTF == 1)
          if((res = ifd_binding_check_ifindex_is_master(ifsv_cb,ifindex,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
          {
             /* If the master is getting deleted, make current slave the new master, the slave ifindex is made 0
              as it is getting deleted */
               if((bonding_data = ifsv_binding_delete_master_ifindex(ifsv_cb,bonding_ifindex)) != NULL)
               {
                   /* Send the trigger point to Standby IfD. */
                  bonding_data->if_info.if_am = NCS_IFSV_BINDING_CHNG_IFINDEX;
                  res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                         (uns8 *)(bonding_data));
                    /* no need to bcast data modify to IFNDs as this being handled as part of delete record at IFNDs
                     IFND has to send MASTER_CHNG message to applications  */
               }
           }
          else if((res = ifsv_binding_check_ifindex_is_slave(ifsv_cb,ifindex,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
          {
             /* if slave is getting deleted , make slave_ifindex of bonding data 0 */
             printf("\nGeeth %d ifindex is slave %d is bbobdibg if index",ifindex,bonding_ifindex);
              bonding_data = ifsv_intf_rec_find(bonding_ifindex,ifsv_cb);
              if( bonding_data != IFSV_NULL )
              {
                  bonding_data->if_info.bind_slave_ifindex = 0;
                  bonding_data->if_info.if_am = NCS_IFSV_BINDING_CHNG_IFINDEX;
                  res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                         (uns8 *)(bonding_data));
                  /* no need to bcast data modify to IFNDs as this being handled as part of delete record at IFNDs*/
              }

          }
#endif

         ifsv_intf_rec_del(ifindex, ifsv_cb);
        
         if (intf_rec != NULL)
         {         
            m_IFD_LOG_STR_NORMAL(IFSV_LOG_IF_TBL_DEL_SUCCESS," Ifindex :",ifindex);
/*
            m_IFSV_LOG_HEAD_LINE(ifsv_cb->my_svc_id,\
                                 IFSV_LOG_IF_TBL_DEL_SUCCESS,ifindex,0);
*/
           /* delete the slot port maping */
           res = ifsv_ifindex_spt_map_del(intf_rec->intf_data.spt_info, ifsv_cb);         
           if(res == NCSCC_RC_SUCCESS)
           {
               m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(ifsv_cb->comp_type),\
                      IFSV_LOG_IF_MAP_TBL_DEL_SUCCESS,intf_rec->intf_data.spt_info.slot,intf_rec->intf_data.spt_info.port);

            /* free the ifindex */
            spt_map.spt = intf_rec->intf_data.spt_info;
            spt_map.if_index = intf_rec->intf_data.if_index;
            res = ifsv_ifindex_free(ifsv_cb, &spt_map);
           }
           m_MMGR_FREE_IFSV_INTF_REC(intf_rec);
         } 
         
      }
      
      if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
         (uns8*)&ifindex);
      
      intf_rec = (IFSV_INTF_REC*)if_node;
   }
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   return (res);

} /* The function ifd_same_node_id_all_intf_rec_del() ends here. */

/****************************************************************************
 * Name          : ifd_all_intf_rec_del
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
ifd_all_intf_rec_del (IFSV_CB *ifsv_cb)
{
   uns32 ifindex = 0;   
   uns32 res = NCSCC_RC_SUCCESS;
   IFSV_INTF_REC       *intf_rec;
   NCS_PATRICIA_NODE   *if_node;   
   NCS_LOCK            *intf_rec_lock = &ifsv_cb->intf_rec_lock;
#if(NCS_IFSV_BOND_INTF == 1)
   IFSV_INTF_DATA *bonding_data;
   NCS_IFSV_IFINDEX bonding_ifindex;
#endif

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
      (uns8*)&ifindex);
   intf_rec = (IFSV_INTF_REC*)if_node;

   while (intf_rec != IFSV_NULL)
   {
      ifindex = intf_rec->intf_data.if_index;
#if(NCS_IFSV_BOND_INTF == 1)
     if((res = ifd_binding_check_ifindex_is_master(ifsv_cb,ifindex,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
          {
              /* If the master is getting deleted, make current slave the new master, the slave ifindex is made 0
              as it is deleted */
               if((bonding_data = ifsv_binding_delete_master_ifindex(ifsv_cb,bonding_ifindex)) != NULL)
              {
                   /* Send the trigger point to Standby IfD. */
                  bonding_data->if_info.if_am = NCS_IFSV_BINDING_CHNG_IFINDEX;
                  res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                         (uns8 *)(bonding_data));

                  /* no need to bcast data modify to IFNDs as this being handled as part of delete record at IFNDs
                     IFND has to send MASTER_CHNG message to applications  */
               }
          }
    else if((res = ifsv_binding_check_ifindex_is_slave(ifsv_cb,ifindex,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
          {
             /* if slave is getting deleted , make slave_ifindex of bonding data 0 */
              bonding_data = ifsv_intf_rec_find(bonding_ifindex,ifsv_cb);
              if( bonding_data != IFSV_NULL )
              {
                  bonding_data->if_info.bind_slave_ifindex = 0;
                  bonding_data->if_info.if_am = NCS_IFSV_BINDING_CHNG_IFINDEX;
                  res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                         (uns8 *)(bonding_data));
                  /* no need to bcast data modify to IFNDs as this being handled as part of delete record at IFNDs*/
              }

          }
#endif
      if(ifsv_intf_rec_del(ifindex, ifsv_cb)!= NULL)
      {
        m_IFSV_LOG_HEAD_LINE(ifsv_cb->my_svc_id,\
         IFSV_LOG_IF_TBL_DEL_SUCCESS,ifindex,0);
      }
      /* delete the slot port maping */
      res = ifsv_ifindex_spt_map_del(intf_rec->intf_data.spt_info, ifsv_cb);
      if(res == NCSCC_RC_SUCCESS)  
         m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(ifsv_cb->comp_type),\
         IFSV_LOG_IF_MAP_TBL_DEL_SUCCESS,intf_rec->intf_data.spt_info.slot,intf_rec->intf_data.spt_info.port);
   
      /* free the interface record */
      m_MMGR_FREE_IFSV_INTF_REC(intf_rec);      
      
      if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
         (uns8*)&ifindex);
      
      intf_rec = (IFSV_INTF_REC*)if_node;
   }
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   return (res);
}

/****************************************************************************
 * Name          : ifd_amf_init
 *
 * Description   : IfD initializes AMF for involking process and registers 
 *                 the various callback functions.
 *
 * Arguments     : ifsv_cb  - Ifsv control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_amf_init (IFSV_CB *ifsv_cb)
{
   SaAmfCallbacksT amfCallbacks;
   SaVersionT      amf_version;   
   SaAisErrorT     error;
   uns32           res = NCSCC_RC_SUCCESS;

   memset(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));

   amfCallbacks.saAmfHealthcheckCallback = ifd_saf_health_chk_callback;
   amfCallbacks.saAmfCSISetCallback = ifd_saf_CSI_set_callback;
   amfCallbacks.saAmfCSIRemoveCallback= 
      ifd_saf_CSI_rem_callback;
   amfCallbacks.saAmfComponentTerminateCallback= 
      ifd_saf_comp_terminate_callback;

   m_IFSV_GET_AMF_VER(amf_version);

   error = saAmfInitialize(&ifsv_cb->amf_hdl, &amfCallbacks, &amf_version);
   do
   {
      if (error != SA_AIS_OK)
      {         
         /* Log */
         res = NCSCC_RC_FAILURE;
         m_IFD_LOG_API_L(IFSV_LOG_AMF_INIT_FAILURE,(long)ifsv_cb);
         break;
      }
      m_IFD_LOG_API_L(IFSV_LOG_AMF_INIT_DONE,(long)ifsv_cb);
   } while(0);
   return (res);
}

/****************************************************************************
 * Name          : ifd_clear_mbx
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
ifd_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg)
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
 * Name          : ifd_init_cb
 *
 * Description   : This is the function which initializes all the data 
 *                 structures and locks used belongs to IfD. 
 *
 * Arguments     : ifsv_cb  - Ifsv control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_init_cb (IFSV_CB *ifsv_cb)
{
   NCS_PATRICIA_PARAMS      params;   
   uns32                   res = NCSCC_RC_SUCCESS;

   do
   {
      ifsv_cb->my_svc_id = NCS_SERVICE_ID_IFD;
                  
      if (m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_CREATE, 0) 
         == NCSCC_RC_FAILURE)
      {         
         res = NCSCC_RC_FAILURE;
         m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_LOCK_CREATE_FAIL,(long)ifsv_cb);
         break;
      }
      m_IFD_LOG_LOCK(IFSV_LOG_LOCK_CREATE,(long)(&ifsv_cb->intf_rec_lock));
      
      /* initialze interface tree */
      
      params.key_size = sizeof(uns32);
      params.info_size = 0;
      if ((ncs_patricia_tree_init(&ifsv_cb->if_tbl, &params))
         != NCSCC_RC_SUCCESS)
      {         
         m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);
         res = NCSCC_RC_FAILURE;
         m_IFD_LOG_HEAD_LINE(IFSV_LOG_IF_TBL_CREATE_FAILURE,(long)ifsv_cb,0);
         break;           
      }
      m_IFD_LOG_HEAD_LINE(IFSV_LOG_IF_TBL_CREATED,(long)(&ifsv_cb->if_tbl),0);
      
      /* initialze shelf/slot/port/type/scope tree */
      
      params.key_size = sizeof(NCS_IFSV_SPT);
      params.info_size = 0;
      if ((ncs_patricia_tree_init(&ifsv_cb->if_map_tbl, &params))
         != NCSCC_RC_SUCCESS)
      {         
         m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);
         ncs_patricia_tree_destroy(&ifsv_cb->if_tbl);
         res = NCSCC_RC_FAILURE;
         m_IFD_LOG_HEAD_LINE(IFSV_LOG_IF_MAP_TBL_CREATE_FAILURE,(long)ifsv_cb,0);
         break;         
      }
      m_IFD_LOG_HEAD_LINE(IFSV_LOG_IF_MAP_TBL_CREATED,(long)(&ifsv_cb->if_map_tbl),0);

      params.key_size = sizeof(NODE_ID);
      params.info_size = 0;
      if ((ncs_patricia_tree_init(&ifsv_cb->ifnd_node_id_tbl, &params))
         != NCSCC_RC_SUCCESS)
      {
         m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);
         ncs_patricia_tree_destroy(&ifsv_cb->if_tbl);
         ncs_patricia_tree_destroy(&ifsv_cb->if_map_tbl);
         res = NCSCC_RC_FAILURE;
         m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFND_NODE_ID_TBL_CREATE_FAILURE,(long)ifsv_cb,0);
         break;
      }
       m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFND_NODE_ID_TBL_CREATED,(long)ifsv_cb,0);

      params.key_size = sizeof(uns32);
      params.info_size = 0;
      if ((ncs_patricia_tree_init(&ifsv_cb->ifsv_bind_mib_rec, &params))
         != NCSCC_RC_SUCCESS)
      {
         m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);
         ncs_patricia_tree_destroy(&ifsv_cb->if_tbl);
         ncs_patricia_tree_destroy(&ifsv_cb->if_map_tbl);
         ncs_patricia_tree_destroy(&ifsv_cb->ifnd_node_id_tbl);
         res = NCSCC_RC_FAILURE;
         /* Need to Log this Failure */
         break;
      }


     /* Initializing IP Address virtualization provision database */
#if (NCS_VIP == 1)
      res = ifd_init_vip_db(ifsv_cb);  
      if (res == NCSCC_RC_FAILURE)
      {
         ncs_patricia_tree_destroy(&ifsv_cb->if_tbl);
         ncs_patricia_tree_destroy(&ifsv_cb->if_map_tbl);
         res = NCSCC_RC_FAILURE;
         break;
      }
#endif


      m_IFD_IFAP_INIT(NULL);

      /* EDU initialisation */
      m_NCS_EDU_HDL_INIT(&ifsv_cb->edu_hdl);
      
   } while (0);
   return (res);
}




/****************************************************************************
 * Name          : ifsv_ifd_ifindex_alloc
 *
 * Description   : This is the function which allocates ifindex for the given 
 *                  shelf, slot, port, type and scope.
 *                 
 *
 * Arguments     : spt_map  - Slot Port Type Vs Ifindex maping
 *               : ifsv_cb        - interface service control block 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifsv_ifd_ifindex_alloc (NCS_IFSV_SPT_MAP *spt_map, IFSV_CB *ifsv_cb)
{   
   uns32 res = NCSCC_RC_SUCCESS;
   uns32 res1 = NCSCC_RC_SUCCESS;
   IFSV_INTF_DATA *rec_data = NULL;
   
   if (m_IFD_IFAP_IFINDEX_ALLOC(spt_map,ifsv_cb) == NCSCC_RC_FAILURE)
   {
      m_IFSV_LOG_SPT_INFO(IFSV_LOG_IFINDEX_ALLOC_FAILURE,spt_map,"ifsv_ifd_ifindex_alloc");
      res = NCSCC_RC_FAILURE;
   } else
   {
      /* Check whether this ifindex has been allocated to any other intfaces. */
      if ((rec_data = ifsv_intf_rec_find (spt_map->if_index, ifsv_cb)) != NULL)
      {
        /* This means that ifindex has already been allocated to another intf. */
        m_IFD_LOG_FUNC_ENTRY_CRITICAL_INFO(IFSV_LOG_IFD_MSG, "Error :Intf Index for Shelf, Slot, Port, Type, Scope has already been allocated.",spt_map->if_index,spt_map->spt.shelf,spt_map->spt.slot,spt_map->spt.port,spt_map->spt.type,spt_map->spt.subscr_scope,0);
        m_IFD_LOG_FUNC_ENTRY_CRITICAL_INFO(IFSV_LOG_IFD_MSG, "Error : Previously Intf Index had been allocated to Shelf, Slot, Port, Type, Scope.",spt_map->if_index,rec_data->spt_info.shelf,rec_data->spt_info.slot,rec_data->spt_info.port,rec_data->spt_info.type,rec_data->spt_info.subscr_scope,0);

         printf("Error :Intf Index %d for Shelf %d, Slot %d, Port %d, Type %d Scope %d has already been allocated to Shelf %d, Slot %d, Port %d, Type %d Scope %d, originator %d, originator_mds_destination 0x%x, current_owner %d, current_owner_mds_destination 0x%x, marked_del %d\n", spt_map->if_index,spt_map->spt.shelf,spt_map->spt.slot,spt_map->spt.port,spt_map->spt.type,spt_map->spt.subscr_scope,rec_data->spt_info.shelf,rec_data->spt_info.slot,rec_data->spt_info.port,rec_data->spt_info.type,rec_data->spt_info.subscr_scope,rec_data->originator,rec_data->originator_mds_destination,rec_data->current_owner,rec_data->current_owner_mds_destination,rec_data->marked_del);
        return NCSCC_RC_FAILURE;
      }
 

      /* Send the trigger point to Standby IfD. */
      res1 = ifd_a2s_async_update(ifsv_cb, IFD_A2S_IFINDEX_ALLOC_MSG,
                              (uns8 *)(spt_map));
      if(res1 != NCSCC_RC_SUCCESS)
           m_IFSV_LOG_SPT_INFO(IFSV_LOG_FUNC_RET_FAIL,spt_map,"ifd_a2s_async_update in ifsv_ifd_ifindex_alloc for ifindex");

      /* add the Slot/Port/Type Vs Ifindex mapping in to the tree */
      res = ifsv_ifindex_spt_map_add(spt_map, ifsv_cb);

      if(res != NCSCC_RC_SUCCESS)
      {
        m_IFSV_LOG_SPT_INFO(IFSV_LOG_FUNC_RET_FAIL,spt_map,"ifsv_ifindex_spt_map_add in ifsv_ifd_ifindex_alloc");
        ifsv_ifindex_free(ifsv_cb, spt_map);
        return res;
      }
 
      /* Send the trigger point to Standby IfD. */
      res1 = ifd_a2s_async_update(ifsv_cb, IFD_A2S_SPT_MAP_MAP_CREATE_MSG,
                              (uns8 *)(spt_map));
      if(res1 != NCSCC_RC_SUCCESS)
           m_IFSV_LOG_SPT_INFO(IFSV_LOG_FUNC_RET_FAIL,spt_map,"ifd_a2s_async_update in ifsv_ifd_ifindex_alloc for SPT");
   }
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
 * Arguments     : spt_map  - Slot Port Type Vs Ifindex maping
 *               : ifsv_cb  - interface service control block 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifsv_ifindex_free (IFSV_CB *ifsv_cb, NCS_IFSV_SPT_MAP *spt_map)
{     
   if (m_IFD_IFAP_IFINDEX_FREE(spt_map->if_index,ifsv_cb) 
     == NCSCC_RC_FAILURE)
   {
      m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFINDEX_FREE_FAILURE,spt_map->if_index,\
         spt_map->spt.port);
      return (NCSCC_RC_FAILURE);
   }      
   
   m_IFD_LOG_HEAD_LINE_NORMAL(IFSV_LOG_IFINDEX_FREE_SUCCESS,spt_map->if_index,\
                       spt_map->spt.port);
   
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifd_bcast_to_ifnds
 *
 * Description   : This is the function which is used to bcast information to
 *                 all IfNDs from IfD.
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
ifd_bcast_to_ifnds (IFSV_INTF_DATA *intf_data, IFSV_INTF_REC_EVT rec_evt,
                    uns32 attr, IFSV_CB *cb)
{
   IFSV_EVT  *evt;
   uns32 res = NCSCC_RC_FAILURE;
   char log_info[45];

   evt = m_MMGR_ALLOC_IFSV_EVT;

   if (evt == IFSV_NULL)
   {
      m_IFSV_LOG_SYS_CALL_FAIL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
               IFSV_LOG_MEM_ALLOC_FAIL,0);
      return (NCSCC_RC_FAILURE);
   }

   memset(evt, 0, sizeof(IFSV_EVT));

   if ((rec_evt == IFSV_INTF_REC_ADD) || (rec_evt == IFSV_INTF_REC_MODIFY))
   {
     evt->type = IFND_EVT_INTF_CREATE;
     memcpy(&evt->info.ifnd_evt.info.intf_create.intf_data,
     intf_data, sizeof(IFSV_INTF_DATA));

     evt->info.ifnd_evt.info.intf_create.if_attr = attr;
     evt->info.ifnd_evt.info.intf_create.evt_orig = NCS_IFSV_EVT_ORIGN_IFD;

     m_IFD_LOG_EVT_LL(IFSV_LOG_IFND_EVT_INTF_CREATE_SND,\
     ifsv_log_spt_string(intf_data->spt_info,log_info),\
     intf_data->if_index,attr);
    } 
    else
    {
       evt->type = IFND_EVT_INTF_DESTROY;
       evt->info.ifnd_evt.info.intf_destroy.spt_type = intf_data->spt_info;
       evt->info.ifnd_evt.info.intf_destroy.orign    = NCS_IFSV_EVT_ORIGN_IFD;
       evt->info.ifnd_evt.info.intf_destroy.own_dest =
       intf_data->originator_mds_destination;

        m_IFD_LOG_EVT_L(IFSV_LOG_IFND_EVT_INTF_DESTROY_SND,\
        ifsv_log_spt_string(intf_data->spt_info,log_info),\
        intf_data->if_index);
    }

    /* Broadcast the information to all the IfND's */
    res = ifsv_mds_bcast_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD,
                             (NCSCONTEXT)evt, NCSMDS_SVC_ID_IFND);

   m_MMGR_FREE_IFSV_EVT(evt);
   return(res);
} /* The function ifd_bcast_to_ifnds() ends here. */

#if(NCS_IFSV_BOND_INTF == 1)
 /* 
  for all binding interfaces possible from port num 1 to bondif_max_portnum
    if the interface added is the master of any binding interface 
        update bond_intf_data with the new index as bind_master_ifindex
        make bind_index the master's master
        send data modify async message to standby IFD
        broadcast datamodify to IFNDs
    endif
    if the interface added is the slave of any binding interface 
        update bond_intf_data with the new index as bind_slave_ifindex
        make bind_index the slave's slave
        send data modify async message to standby IFD
        broadcast datamodify to IFNDs
    endif
*/
 void ifd_check_if_master_slave(NCS_IFSV_IFINDEX ifindex,IFSV_CB* ifsv_cb)
            {
                 int32 bind_portnum=1;
                 uns32 res;
                 IFSV_INTF_ATTR attr = 0;
                 IFSV_INTF_DATA* create_intf;
                 create_intf = ifsv_intf_rec_find(ifindex,ifsv_cb);
                 if(create_intf == IFSV_NULL)
                 return;
                 while( bind_portnum <= ifsv_cb->bondif_max_portnum)
                 {
                   NCS_IFSV_SPT  spt_info;
                   uns32 bind_ifindex;
                   IFSV_INTF_DATA *bond_intf_data;
                    spt_info.port           =  bind_portnum;
                    spt_info.shelf          =  IFSV_BINDING_SHELF_ID;
                    spt_info.slot           =  IFSV_BINDING_SLOT_ID;
                    /* embedding subslot changes */
                    spt_info.subslot           =  IFSV_BINDING_SUBSLOT_ID;
                    spt_info.type           =  NCS_IFSV_INTF_BINDING;
                    spt_info.subscr_scope   =  NCS_IFSV_SUBSCR_INT;
                    if(ifsv_get_ifindex_from_spt (&bind_ifindex, spt_info, ifsv_cb) == NCSCC_RC_SUCCESS)
                    {
                      bond_intf_data = ifsv_intf_rec_find(bind_ifindex,ifsv_cb);
                      if(bond_intf_data != IFSV_NULL)
                      {
                        if(bond_intf_data->if_info.bind_master_info.node_id == m_NCS_NODE_ID_FROM_MDS_DEST(create_intf->originator_mds_destination))
                         {
                          if(strncmp(bond_intf_data->if_info.bind_master_info.if_name,create_intf->if_info.if_name,20) == 0)
                            {
                              bond_intf_data->if_info.bind_master_ifindex = create_intf->if_index;
                              bond_intf_data->if_info.oper_state = create_intf->if_info.oper_state;
                              strncpy(bond_intf_data->if_info.if_name,create_intf->if_info.if_name,IFSV_IF_NAME_SIZE);
                              create_intf->if_info.bind_master_ifindex = bind_ifindex;
                              bond_intf_data->if_info.if_am = NCS_IFSV_BINDING_CHNG_IFINDEX;
                              res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                         (uns8 *)(bond_intf_data));
                              attr = NCS_IFSV_BINDING_CHNG_IFINDEX;
                              ifd_bcast_to_ifnds(bond_intf_data, IFSV_INTF_REC_MODIFY, attr, ifsv_cb);
                            }
                         }
                       if(bond_intf_data->if_info.bind_slave_info.node_id == m_NCS_NODE_ID_FROM_MDS_DEST(create_intf->originator_mds_destination))
                        {
                         if(strncmp(bond_intf_data->if_info.bind_slave_info.if_name,create_intf->if_info.if_name,20) == 0)
                           {
                              bond_intf_data->if_info.bind_slave_ifindex = create_intf->if_index;
                              create_intf->if_info.bind_slave_ifindex = bind_ifindex;
                              bond_intf_data->if_info.if_am = NCS_IFSV_BINDING_CHNG_IFINDEX;
                              res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                         (uns8 *)(bond_intf_data));
                              attr = NCS_IFSV_BINDING_CHNG_IFINDEX;
                              ifd_bcast_to_ifnds(bond_intf_data, IFSV_INTF_REC_MODIFY, attr, ifsv_cb);
                           }
                        }
                      }
                    }
                    bind_portnum++;
                 }
            }
#endif
             
