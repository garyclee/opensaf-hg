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
  FILE NAME: IFSV_DB.C

  DESCRIPTION: Function used to acess database maintained by IfD/IfND.

  FUNCTIONS INCLUDED in this module:
  ifsv_intf_rec_find .............. Find the interface record.
  ifsv_intf_rec_add ............... Add interface record.
  ifsv_intf_rec_modify ............ Modify interface record.
  ifsv_intf_rec_marked_del ........ Modify or delete the interface record.
  ifsv_intf_rec_del ............... Delete interface record.
  ifsv_cleanup_tmr_find ........... Find the Cleanup timer for the given slot/port.

  adding subslot changes for the below functions.
     ifsv_get_ifindex_from_spt ....... Find shelf/slot/subslot/port/scope Vs Ifindex map.
     ifsv_ifindex_spt_map_add ........ Add shelf/slot/subslot/port/scope Vs Ifindex map.
     ifsv_ifindex_spt_map_del ........ Del shelf/slot/subslot/port/scope Vs Ifindex map.
     ifsv_ifindex_spt_map_del_all .... Del all shelf/slot/subslot/port/scope Vs Ifindex map.  
  Subslot changes end 
     ifsv_rms_if_rec_send ............ Send an RMS message.  
     ifsv_evt_destroy ................ Destroy events send/rcv by Ifd/IfND.  

******************************************************************************/
#include "ifsv.h"

static uns32 ifsv_reg_ifmib_row(IFSV_CB *cb, 
                             uns32 ifindex, 
                             NCSCONTEXT *row_hdl);

static uns32 ifsv_unreg_ifmib_row(IFSV_CB *cb, NCSCONTEXT row_hdl);
static uns32 ifsv_check_valid_bind_interface(IFSV_INTF_DATA intf_data,IFSV_CB* ifsv_cb);

/**** global variale *****/
IFSVDLL_API uns32 gl_ifsv_hdl;
char g_ifd_comp_name[128];
char g_ifnd_comp_name[128];

uns32 gl_ifd_cb_hdl;
uns32 gl_ifnd_cb_hdl;

/****************************************************************************
 * Name          : ifsv_intf_rec_find
 *
 * Description   : This function gets the interface record for the given 
 *                 ifindex.
 *
 * Arguments     : ifindex - This is the key to find the record.
 *               : CB  - Pointer to the CB.
 *
 * Return Values : NULL (on failure) or IFSV_INTF_DATA pointer.
 *
 * Notes         : This Routine won't take the Lock, It is all the application 
 *                 which is using this function needs to take the LOCK. 
 *                 This function is going to return the exact pointers which is
 *                 present in the tree.
 *****************************************************************************/

IFSV_INTF_DATA * 
ifsv_intf_rec_find (uns32 ifindex,
                    IFSV_CB *cb)                    
{
   NCS_PATRICIA_TREE *p_tree = &cb->if_tbl;
   IFSV_INTF_DATA *intf_data;
   IFSV_INTF_REC *intf_rec;   

   intf_rec = (IFSV_INTF_REC *)ncs_patricia_tree_get(p_tree, 
      (uns8 *)&ifindex);   
   if (intf_rec != NULL)
   {
      m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_TBL_LOOKUP_SUCCESS,ifindex,0); 
      intf_data = &intf_rec->intf_data;
   } else
   {
      m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_TBL_LOOKUP_FAILURE,ifindex,0); 
      intf_data = NULL;
   }
   return (intf_data);
}


/****************************************************************************
 * Name          : ifsv_intf_rec_add
 *
 * Description   : This function adds the interface record for the given 
 *                 ifindex.
 *                 This function adds the record to the patrica tree. 
 *                 Depending on the component it sends the interface rec 
 *                 information to IfD/IfND. RMS Checkpointing is also done.
 *
 * Arguments     : i_rec_data     - pointer to the interface record data to be 
 *                                  added in to the interface table.                
 *                 cb             - Interface Control Block Pointer. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : 
 *****************************************************************************/
uns32 
ifsv_intf_rec_add (IFSV_INTF_DATA *i_rec_data, 
                   IFSV_CB *cb)                   
{
   NCS_PATRICIA_TREE *p_tree   = &cb->if_tbl;   
   NCS_LOCK          *intf_rec_lock = &cb->intf_rec_lock;
   NCS_IFSV_SPT_MAP  spt_map;
   IFSV_INTF_REC     *rec;   
   uns32  res = NCSCC_RC_SUCCESS;   
   uns32 ifindex = 0;

   if ((rec = m_MMGR_ALLOC_IFSV_INTF_REC) == IFSV_NULL)
   {
      m_IFSV_LOG_SYS_CALL_FAIL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
               IFSV_LOG_MEM_ALLOC_FAIL,0);
      return (NCSCC_RC_FAILURE);
   }
   
   m_NCS_MEMSET(rec,0,sizeof(IFSV_INTF_REC));
   memcpy(&rec->intf_data, i_rec_data, sizeof(IFSV_INTF_DATA));
   rec->intf_data.active = TRUE;

   /* While adding the record, set all the attributes to TRUE */
   m_IFSV_ALL_ATTR_SET(rec->intf_data.if_info.if_am);

   rec->node.key_info = (uns8*)&rec->intf_data.if_index;
   ifindex = rec->intf_data.if_index;
   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);

   do
   {
     /* Add the record to the interface rec table */
     if(ncs_patricia_tree_get(p_tree, (uns8 *)&ifindex) == NULL)
     {
      if (ncs_patricia_tree_add (p_tree, &rec->node) != NCSCC_RC_SUCCESS)
      {
         m_MMGR_FREE_IFSV_INTF_REC(rec);
         m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_TBL_ADD_FAILURE,rec->intf_data.if_index,0);
         res = NCSCC_RC_FAILURE;
         break;
      }
     } 
     else
     {
       m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_TBL_ALREADY_EXIST,ifindex,0); 
         res = NCSCC_RC_FAILURE;
         break;
     }
      if (cb->comp_type == IFSV_COMP_IFND)
      {
         m_NCS_MEMSET(&spt_map, 0, sizeof(NCS_IFSV_SPT_MAP));
         spt_map.spt      = rec->intf_data.spt_info;
         spt_map.if_index = rec->intf_data.if_index;
         /* Add the mapping b/w s/s/ss/p/t and Ifindex to interface map table
          * This adding is done only in IfND b/c In IfD this maping would
          * have been added during resolving the ifindex.
          */
         if (ifsv_ifindex_spt_map_add(&spt_map, cb) 
            != NCSCC_RC_SUCCESS)
         {
            m_IFSV_LOG_STR_2_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifindex_spt_map_add() returned failure"," ");
            res = NCSCC_RC_FAILURE;
            ncs_patricia_tree_del(p_tree, &rec->node);
            m_MMGR_FREE_IFSV_INTF_REC(rec);
            break;
         }
      }
      
      /* Register this row with MAB, only for external interfaces. And also 
         register for those rows, you are the owner, this is useful in avoiding
         to register both IfD and IfND for the same row evenif only one of
         these is the owner. */
      if ((NCS_IFSV_SUBSCR_EXT == rec->intf_data.spt_info.subscr_scope) && 
          (m_NCS_NODE_ID_FROM_MDS_DEST(rec->intf_data.current_owner_mds_destination) == m_NCS_NODE_ID_FROM_MDS_DEST(cb->my_dest)))
            res = ifsv_reg_ifmib_row(cb, rec->intf_data.if_index, 
             &rec->intf_data.mab_rec_row_hdl);

      if(res != NCSCC_RC_SUCCESS)
      {
         m_IFSV_LOG_STR_2_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_FUNC_RET_FAIL,"ifsv_reg_ifmib_row() returned failure"," ");
         ncs_patricia_tree_del(p_tree, &rec->node);

         /* delete the slot port maping */
         ifsv_ifindex_spt_map_del(spt_map.spt, cb);

         m_MMGR_FREE_IFSV_INTF_REC(rec);
         break;
      }
#if(NCS_IFSV_BOND_INTF == 1)
/* Process this at only IFD as record modify message received by IFNDs 
   handles this. The following requires modification of bond_data,
   master and slave. This could be a problem during cold sync at IFNDs if handled here.
*/
      if((cb->comp_type != IFSV_COMP_IFND) && (rec->intf_data.spt_info.type == NCS_IFSV_INTF_BINDING))
      { 
         if((res = ifsv_check_valid_bind_interface(rec->intf_data,cb)) == NCSCC_RC_SUCCESS)
         res=ifsv_bonding_assign_bonding_ifindex_to_master(cb,rec->intf_data.if_index);

         if(res != NCSCC_RC_SUCCESS)
         {
            m_IFSV_LOG_STR_2_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_FUNC_RET_FAIL,"ifsv_bonding_assign_bonding_ifindex_to_master  returned failure()"," "); 
            ncs_patricia_tree_del(p_tree, &rec->node);
            /* delete the slot port maping */
            ifsv_ifindex_spt_map_del(spt_map.spt, cb);
            m_MMGR_FREE_IFSV_INTF_REC(rec);
            break;
         }
      }
#endif

      /* ADD API call is for technology specific call to Add tech specific
       * info
       */
#if(NCS_IFSV_IPXS == 1)
      res = ipxs_update_ifadd(cb->my_svc_id, &rec->intf_data);
      if(res != NCSCC_RC_SUCCESS)
      {
         m_IFSV_LOG_STR_2_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_FUNC_RET_FAIL,"ipxs_update_ifadd() returned failure"," ");
         ncs_patricia_tree_del(p_tree, &rec->node);

         /* delete the slot port maping */
         ifsv_ifindex_spt_map_del(spt_map.spt, cb);

         /* Unregister this row with MAB, only for external interfaces */
         if (NCS_IFSV_SUBSCR_EXT == rec->intf_data.spt_info.subscr_scope)
           ifsv_unreg_ifmib_row(cb, rec->intf_data.mab_rec_row_hdl);
         m_MMGR_FREE_IFSV_INTF_REC(rec);
         break;
      }
#endif

   } while (0);   
   
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   return (res);
}


/****************************************************************************
 * Name           : ifsv_bonding_check_ifindex_is_master
 *
 * Description    : checks if the interface with index ifindex is a master if
 *                  ifindex. If so, returns NCSCC_RC_SUCCESS,
 *                  otherwise returns NCSCC_RC_FAILURE.
 *
 * Arguments     : ifsv_cb  - The ifsv control block.
 *                 ifindex  - The index to be checked
 *                 bonding_ifindex - To be filled with corresponding binding ifindex
 *                                   master ifindex
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes            : None.
 *****************************************************************************/
uns32
ifsv_bonding_check_ifindex_is_master(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX ifindex,
                                                                        NCS_IFSV_IFINDEX *bonding_ifindex)
{
     IFSV_INTF_DATA *master_intf_data;
     IFSV_INTF_DATA *bond_intf_data;
     NCS_IFSV_IFINDEX temp_ifindex;

    /* Get the intf record of the ifindex */
    master_intf_data = ifsv_intf_rec_find (ifindex, ifsv_cb);
    if(master_intf_data == NULL)
    {
       return NCSCC_RC_FAILURE;
    }
/* If it is the bonding interface return */
    if(master_intf_data->spt_info.type == NCS_IFSV_INTF_BINDING)
    {
       return NCSCC_RC_FAILURE;
    }


    /* Get its corresponding bonding interface record.. the same variable is resued for this purpose */
    temp_ifindex = master_intf_data->if_info.bind_master_ifindex;
    if(temp_ifindex == 0)
    {
       return NCSCC_RC_FAILURE;
    }

   /* Get the intf record of the bonding interface */
    bond_intf_data = ifsv_intf_rec_find (temp_ifindex, ifsv_cb);
    if(bond_intf_data == NULL)
    {
       return NCSCC_RC_FAILURE;
    }
    if(bond_intf_data->spt_info.type != NCS_IFSV_INTF_BINDING)
    {
       return NCSCC_RC_FAILURE;
    }

    /* Check if this is the master ifindex of the bonding interface */
    if(ifindex == bond_intf_data->if_info.bind_master_ifindex)
    {
        *bonding_ifindex = temp_ifindex;
     return NCSCC_RC_SUCCESS;
    }
    return NCSCC_RC_FAILURE;
}


/****************************************************************************
 * Name          : ifsv_intf_rec_modify
 *
 * Description   : This function modifies the given data for the attribute 
 *                 mensioned for the given ifindex. 
 *
 * Arguments     : actual_data   - the interface data which needs to be 
 *                                 modified.
 *               : mod_data      - interface information which needs to be 
 *                                 modified with.
 *               : attr          - This is bit wise attribute which needs 
 *                                 to be modified in the record.
 *                 cb            - Interface Control Block Pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : If the data in the interface record is a pointer, than it is 
 *                 the responsibility of the application which is using this 
 *                 function needs to allocate the memory. Assume that interface 
 *                 database lock has been already taken. Here Tunnel attribute
 *                 modification needs to be taken care (TBD).
 *****************************************************************************/
uns32 
ifsv_intf_rec_modify (IFSV_INTF_DATA *actual_data,
                      IFSV_INTF_DATA *mod_data,                      
                      IFSV_INTF_ATTR *attr,                       
                      IFSV_CB *cb)
                      
{     
   IFSV_INTF_ATTR modify_attr;   
   NCS_BOOL modify = FALSE;
   NCS_BOOL admin_modify = FALSE;
   IFSV_TMR *tmr;
   uns32 rc = NCSCC_RC_SUCCESS;
#if(NCS_IFSV_IPXS == 1)
   IPXS_CB  *ipxs_cb = NULL;
   uns32    ipxs_hdl;
   IPXS_IFIP_NODE   *ifip_node = NULL;
   IPXS_IFIP_INFO   *ip_info = NULL;
#endif
#if(NCS_IFSV_BOND_INTF == 1)
   NCS_IFSV_IFINDEX bonding_ifindex;
   uns32 res = NCSCC_RC_SUCCESS;
   NCS_IPPFX   *ippfx;
#endif
   uns32       ii;

   /* Go ahead, only if there is any attribute to be chnaged. This will happen
       when IfD broadcast Intf Create message back to all IfNDs or IfND sends
       Intf Create message back to IfD. */
   m_IFSV_COPY_ATTR(modify_attr, *attr);      
   
   if (m_IFSV_IS_MTU_ATTR_SET(modify_attr) == TRUE)
   {
      m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_TBL_UPDATE,NCS_IFSV_IAM_MTU,mod_data->if_info.mtu);
      if (actual_data->if_info.mtu != mod_data->if_info.mtu)
      {
              actual_data->if_info.mtu = mod_data->if_info.mtu;
              modify = TRUE;
      }
   }
   
   if (m_IFSV_IS_IFSPEED_ATTR_SET(modify_attr) == TRUE)
   {
      m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_TBL_UPDATE,NCS_IFSV_IAM_IFSPEED,mod_data->if_info.if_speed);
      if (actual_data->if_info.if_speed != mod_data->if_info.if_speed)
      {
              actual_data->if_info.if_speed = mod_data->if_info.if_speed;      
              modify = TRUE;
      }
   }
   
   if (m_IFSV_IS_PHYADDR_ATTR_SET(modify_attr) == TRUE)
   {
      m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_TBL_UPDATE,NCS_IFSV_IAM_PHYADDR,mod_data->if_info.phy_addr[0]);
      if (0 != m_NCS_MEMCMP ((char *)&actual_data->if_info.phy_addr, 
              (char *)&mod_data->if_info.phy_addr, 6))
      {
              memcpy(actual_data->if_info.phy_addr, 
                           mod_data->if_info.phy_addr, 6);      
              modify = TRUE;
      }
   }
   
   if (m_IFSV_IS_ADMSTATE_ATTR_SET(modify_attr) == TRUE)
   {
      m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
          IFSV_LOG_IF_TBL_UPDATE,NCS_IFSV_IAM_ADMSTATE,mod_data->if_info.admin_state);
    do{
      if (actual_data->if_info.admin_state != mod_data->if_info.admin_state)
      {  
        admin_modify = TRUE;
        if(actual_data->if_info.admin_state == NCS_STATUS_UP)
        {
                /* This means that admin state is going DOWN from UP. So we need to make 
                 * Oper state down also. But if the oper state is down already,
                 * which is possible either by application API or via MIB or via Driver or
                 * when application is going down, then we need to take some more decision.
                 * In case of oper change to down by appl, driver or MIB, we will make admin 
                 * state down, but if this oper state is down because of appl going down, then
                 * no need to make admin state down as this will be deleted after 10 secs, if before
                 * getting deleted appl again comes up and adds this intf then the intf record will be 
                 * new, in which old intf rec will be deleted and added new as per appl.*/ 
           if(actual_data->if_info.oper_state == NCS_STATUS_DOWN)
           {
              if((actual_data->marked_del == TRUE) && (actual_data->active == FALSE))
               {
                 /* This means the intf is being deleted, so we shouldn't make any change in adm state.*/
                  break; 
               }
               else
               {
                 /* This means the intf is down because of other resons, so go ahead.*/
               }
           }
           else
           {
             actual_data->if_info.oper_state = NCS_STATUS_DOWN; 
             actual_data->active = FALSE; 
             *attr = (*attr | NCS_IFSV_IAM_OPRSTATE);
           }
       }
       else
       {
         /* Admin state is going UP from Down. So, oper state should be made UP. If oper state 
            is already UP, then its fine, otherwise make it UP. If oper state is already DOWN,
            then if this DOWN is because of Intf going down then we cann't make it UP as intf 
            will be deleted within 10 secs as explained above.*/
           if(actual_data->if_info.oper_state == NCS_STATUS_DOWN)
           {
              if((actual_data->marked_del == TRUE) && (actual_data->active == FALSE))
               {
                 /* This means the intf is being deleted, so we shouldn't make any change in adm state.*/
                  break;
               }
               else
               {
                 actual_data->if_info.oper_state = NCS_STATUS_UP;
                 actual_data->active = TRUE;
                 *attr = (*attr | NCS_IFSV_IAM_OPRSTATE);
#if(NCS_IFSV_BOND_INTF == 1)
                if((res = ifsv_bonding_check_ifindex_is_master(cb,actual_data->if_index,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
                 {
                   IFSV_INTF_DATA *bond_intf_data;
                   bond_intf_data = ifsv_intf_rec_find (bonding_ifindex, cb);
                   bond_intf_data->if_info.oper_state = NCS_STATUS_UP;
                 }
#endif
               }
            }
            else
            {
                 /* This means the oper state is UP, so go ahead.*/
            }
       }
#if(NCS_IFSV_BOND_INTF == 1)
              if(actual_data->if_info.admin_state == NCS_STATUS_UP)
              {
               if((res = ifsv_bonding_check_ifindex_is_master(cb,actual_data->if_index,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
               {
                IFSV_INTF_DATA *bond_intf_data;
                NCS_IFSV_IFINDEX temp_ifindex;
                 /* Get the intf record of the ifindex */
                bond_intf_data = ifsv_intf_rec_find (bonding_ifindex, cb);
                if(bond_intf_data->if_info.bind_slave_ifindex !=0)
                {
#if(NCS_IFSV_IPXS == 1)
             if(cb->comp_type == IFSV_COMP_IFND)
             {
                /* Get the IPXS CB */
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
                         /* Delete the Ipaddresses from the Previous Master and add to the New Master */
                         /*ifsv_del_from_os(bond_intf_data,ip_info->ipaddr_list,ip_info->ipaddr_cnt,cb);*/
                         ippfx = IFSV_NULL;
                         for (ii = 0; ii < ip_info->ipaddr_cnt; ii++)
                         {
                            ippfx = &ip_info->ipaddr_list[ii].ipaddr;
                            /* since we are deleting a single ip at a time, cnt shud be 1 */
                            ifsv_del_from_os(bond_intf_data,ippfx,1,cb);
                         }
                      }
                   }
                }
             }
#endif
             temp_ifindex = bond_intf_data->if_info.bind_slave_ifindex;
             bond_intf_data->if_info.bind_slave_ifindex = bond_intf_data->if_info.bind_master_ifindex;
             bond_intf_data->if_info.bind_master_ifindex = temp_ifindex;
             ifsv_bonding_assign_bonding_ifindex_to_master(cb,bonding_ifindex);
             /*Send master change notification of applications*/
             if(cb->comp_type == IFSV_COMP_IFND)
             {
             IFSV_INTF_ATTR master_change_attr = 0;
             master_change_attr = NCS_IFSV_IAM_CHNG_MASTER;
             ifsv_ifa_app_if_info_indicate(bond_intf_data, IFSV_INTF_REC_MODIFY,master_change_attr, cb);
             }
#if(NCS_IFSV_IPXS == 1)
             if(cb->comp_type == IFSV_COMP_IFND)
             {
                if(ip_info)
                {
                    ifsv_add_to_os(bond_intf_data,ip_info->ipaddr_list,ip_info->ipaddr_cnt,cb);
                }
                ncshm_give_hdl(ipxs_hdl);
             }

#endif
                }
               
               }               
              }
#endif
              actual_data->if_info.admin_state = mod_data->if_info.admin_state; 
              modify = TRUE;
      }
    }while(0);
   }

#if(NCS_IFSV_BOND_INTF == 1)


   if (m_IFSV_IS_MASTER_CHNG_ATTR_SET(modify_attr) == TRUE)
   {
      m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
          IFSV_LOG_IF_TBL_UPDATE,NCS_IFSV_IAM_CHNG_MASTER,mod_data->if_info.admin_state);
      if (actual_data->if_info.bind_master_ifindex == mod_data->if_info.bind_slave_ifindex &&
       actual_data->if_info.bind_slave_ifindex == mod_data->if_info.bind_master_ifindex)        
      {  
              /* The master and slave are swapped by the application runnig on actual_data.slave interface
                   node. So we have to change the master interface so that the application can use 
                   the index for its bonding interface */
             /* This function is to check we dont assign another bonded master to present bonded intf */
          if(ifsv_bonding_assign_bonding_ifindex_to_master(cb,actual_data->if_index) == NCSCC_RC_SUCCESS)
          {
#if(NCS_IFSV_IPXS == 1)
             if(cb->comp_type == IFSV_COMP_IFND)
             {
                /* Get the IPXS CB */
                ipxs_hdl = m_IPXS_CB_HDL_GET( );
                ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);
                if(ipxs_cb)
                {
                   ifip_node = ipxs_ifip_record_get(&ipxs_cb->ifip_tbl, actual_data->if_index);
                   if(ifip_node)
                   {
                      ip_info = (IPXS_IFIP_INFO *)&ifip_node->ifip_info;
                      if(ip_info)
                      {
                         /* Delete the Ipaddresses from the Previous Master and add to the New Master */
                         /* ifsv_del_from_os(actual_data,ip_info->ipaddr_list,ip_info->ipaddr_cnt,cb);*/
                         ippfx = IFSV_NULL;
                         for (ii = 0; ii < ip_info->ipaddr_cnt; ii++)
                         {
                            ippfx = &ip_info->ipaddr_list[ii].ipaddr;
                            /* since we are deleting a single ip at a time, cnt shud be 1 */
                            ifsv_del_from_os(actual_data,ippfx,1,cb);
                         }
                      }
                   }
                }
             }   
#endif 
             actual_data->if_info.bind_master_ifindex = mod_data->if_info.bind_master_ifindex;
             actual_data->if_info.bind_slave_ifindex = mod_data->if_info.bind_slave_ifindex;
             ifsv_bonding_assign_bonding_ifindex_to_master(cb,actual_data->if_index);
             modify = TRUE;

#if(NCS_IFSV_IPXS == 1)
             if(cb->comp_type == IFSV_COMP_IFND)
             {
                if(ip_info)
                {
                    ifsv_add_to_os(actual_data,ip_info->ipaddr_list,ip_info->ipaddr_cnt,cb);
                }
                ncshm_give_hdl(ipxs_hdl);
             }

#endif
       }

    }
  }
#endif   

   
   if (m_IFSV_IS_OPRSTATE_ATTR_SET(modify_attr) == TRUE)
   {
     m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
        IFSV_LOG_IF_TBL_UPDATE,NCS_IFSV_IAM_OPRSTATE,mod_data->if_info.oper_state);
     do
     {
      if((admin_modify == TRUE) || (actual_data->if_info.admin_state == NCS_STATUS_DOWN))
      {
        /* This means that operstate has been already modified. So, ignore it. */
           break;
      }
      if (actual_data->if_info.oper_state != mod_data->if_info.oper_state)
      {
        modify = TRUE;
        tmr = ifsv_cleanup_tmr_find (&actual_data->spt_info, 
            &cb->if_map_tbl);
        if ((actual_data->if_info.oper_state == NCS_STATUS_DOWN) &&
            (mod_data->if_info.oper_state == NCS_STATUS_UP))
        {
          /* stop the interface clean timer*/
          if (tmr != NULL)
           {
              ifsv_tmr_stop(tmr,cb);
              actual_data->marked_del = FALSE;
              actual_data->active = TRUE;
           }
#if(NCS_IFSV_BOND_INTF == 1)
          if((res = ifsv_bonding_check_ifindex_is_master(cb,actual_data->if_index,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
           {
                IFSV_INTF_DATA *bond_intf_data;
                bond_intf_data = ifsv_intf_rec_find (bonding_ifindex, cb);
                bond_intf_data->if_info.oper_state = NCS_STATUS_UP;
           }
#endif

        } 
        else
        {
          /* Going down from UP, then just mark it DOWN. */
          actual_data->active = FALSE;
#if(NCS_IFSV_BOND_INTF == 1)
               if((res = ifsv_bonding_check_ifindex_is_master(cb,actual_data->if_index,&bonding_ifindex)) == NCSCC_RC_SUCCESS)
               {
                IFSV_INTF_DATA *bond_intf_data;
                NCS_IFSV_IFINDEX temp_ifindex;
                 /* Get the intf record of the ifindex */
                bond_intf_data = ifsv_intf_rec_find (bonding_ifindex, cb);
                if(bond_intf_data->if_info.bind_slave_ifindex !=0)
                {
#if(NCS_IFSV_IPXS == 1)
             if(cb->comp_type == IFSV_COMP_IFND)
             {
                /* Get the IPXS CB */
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
                         /* Delete the Ipaddresses from the Previous Master and add to the New Master */
                         /* ifsv_del_from_os(bond_intf_data,ip_info->ipaddr_list,ip_info->ipaddr_cnt,cb);*/
                         ippfx = IFSV_NULL;
                         for (ii = 0; ii < ip_info->ipaddr_cnt; ii++)
                         {
                            ippfx = &ip_info->ipaddr_list[ii].ipaddr;
                            /* since we are deleting a single ip at a time, cnt shud be 1 */
                            ifsv_del_from_os(bond_intf_data,ippfx,1,cb);
                         }
                      }
                   }
                }
             }
#endif
             temp_ifindex = bond_intf_data->if_info.bind_slave_ifindex;
             bond_intf_data->if_info.bind_slave_ifindex = bond_intf_data->if_info.bind_master_ifindex;
             bond_intf_data->if_info.bind_master_ifindex = temp_ifindex;
             ifsv_bonding_assign_bonding_ifindex_to_master(cb,bonding_ifindex);
             if(cb->comp_type == IFSV_COMP_IFND)
             {
             IFSV_INTF_ATTR master_change_attr = 0;
             master_change_attr = NCS_IFSV_IAM_CHNG_MASTER;
             ifsv_ifa_app_if_info_indicate(bond_intf_data, IFSV_INTF_REC_MODIFY,master_change_attr, cb);
             }
#if(NCS_IFSV_IPXS == 1)
             if(cb->comp_type == IFSV_COMP_IFND)
             {
                if(ip_info)
                {
                    ifsv_add_to_os(bond_intf_data,ip_info->ipaddr_list,ip_info->ipaddr_cnt,cb);
                }
                ncshm_give_hdl(ipxs_hdl);
             }

#endif
                }
               
               }               
#endif
      }
       actual_data->if_info.oper_state = mod_data->if_info.oper_state; 
     }
    }while(0);
   }/* if (m_IFSV_IS_OPRSTATE_ATTR_SET(modify_attr) == TRUE)  */
#if(NCS_IFSV_BOND_INTF == 1)
/*
  BINDING_IFINDEX attribute is set when binding interface is created, when an 
  interface which is a master/slave of a binding interface is deleted, when a master/slave
  of a binding interface which is deleted is created again.
  This modies the database at standby IFD and IFNDs on receiving modify message from Active IFD
  MASTER_CHNG modify cant handle this because here master and slave interfaces are not swapped
*/
   if (m_IFSV_IS_BINDING_IFINDEX_ATTR_SET(modify_attr) == TRUE)
   {/* The ifindex of created master/slave may be same as before it was deleted and created again.
       even in this case the master/slave_binding_ifindex of master/slave has to be updated*/
          if(mod_data->spt_info.type == NCS_IFSV_INTF_BINDING)
         {
            IFSV_INTF_DATA* master_intf_data;
            IFSV_INTF_DATA* slave_intf_data;
            actual_data->if_info.bind_master_ifindex = mod_data->if_info.bind_master_ifindex;
            actual_data->if_info.bind_master_info.node_id = mod_data->if_info.bind_master_info.node_id;
            m_NCS_OS_STRNCPY(actual_data->if_info.bind_master_info.if_name,mod_data->if_info.bind_master_info.if_name,20);

            actual_data->if_info.bind_slave_ifindex = mod_data->if_info.bind_slave_ifindex;
            actual_data->if_info.bind_slave_info.node_id = mod_data->if_info.bind_slave_info.node_id;
            m_NCS_OS_STRNCPY(actual_data->if_info.bind_slave_info.if_name,mod_data->if_info.bind_slave_info.if_name,20);

            master_intf_data = ifsv_intf_rec_find(mod_data->if_info.bind_master_ifindex,cb);
            if(master_intf_data != NULL)
            {
              master_intf_data->if_info.bind_master_ifindex = actual_data->if_index;
              #if(NCS_IFSV_IPXS == 1)
                               if(cb->comp_type == IFSV_COMP_IFND)
                               {
                                  /* Get the IPXS CB */
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
                                            ifsv_add_to_os(actual_data,ip_info->ipaddr_list,ip_info->ipaddr_cnt,cb);
                                        }
                                     }
                                  }
                                   ncshm_give_hdl(ipxs_hdl);
                               }
              #endif
            }
            slave_intf_data = ifsv_intf_rec_find(mod_data->if_info.bind_slave_ifindex,cb);
            if(slave_intf_data != NULL)
            {
             slave_intf_data->if_info.bind_slave_ifindex = actual_data->if_index;
            }
            actual_data->if_info.oper_state = master_intf_data->if_info.oper_state;
            m_NCS_OS_STRNCPY(actual_data->if_info.if_name,master_intf_data->if_info.if_name,IFSV_IF_NAME_SIZE);

   }
   }
#endif


   if(modify == TRUE)
   {
     /* update the last change timer **/
     *attr = (*attr | NCS_IFSV_IAM_LAST_CHNG);  
   }
   else
     rc = NCSCC_RC_FAILURE;
 
   return (rc);
} /* End of function ifsv_intf_rec_modify() */

/****************************************************************************
 * Name          : ifsv_intf_rec_marked_del
 *
 * Description   : This function marks for deletion the given data 
 *                 for the attribute mensioned for the given ifindex. 
 *
 * Arguments     : actual_data   - the interface data which needs to be 
 *                                 modified.
 *               : attr          - This is bit wise attribute which needs 
 *                                 to be modified in the record.
 *                 cb            - Interface Control Block Pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : If the data in the interface record is a pointer, than it is 
 *                 the responsibility of the application which is using this 
 *                 function needs to allocate the memory. Assume that interface 
 *                 database lock has been already taken. Here Tunnel attribute
 *                 modification needs to be taken care (TBD).
 *****************************************************************************/
uns32 
ifsv_intf_rec_marked_del (IFSV_INTF_DATA *actual_data,
                      IFSV_INTF_ATTR *attr,                       
                      IFSV_CB *cb)
                      
{     
   IFSV_INTF_REC_EVT   intf_evt = IFSV_INTF_REC_MODIFY;
   IFSV_TMR *tmr;
   uns32 rc = NCSCC_RC_SUCCESS;
   
   if (actual_data->if_info.oper_state == NCS_STATUS_DOWN)
   {
     /* Operational status is down due to modification or it is marked 
        deleted and aging timer is running. */
      /*When comp is IfND, it gets the same message from IfD again when IfD 
        broadcast it, but no need to worry, in that case, we will return from 
        here */
        if(actual_data->marked_del == TRUE)    
         return rc;
   }
   
   tmr = ifsv_cleanup_tmr_find (&actual_data->spt_info, 
                                &cb->if_map_tbl);
   if (tmr != NULL)
   {
      /* Start the interface clean timer.*/
      if(cb->comp_type == IFSV_COMP_IFD)
           tmr->tmr_type = NCS_IFSV_IFD_EVT_INTF_AGING_TMR; 
      else
           tmr->tmr_type = NCS_IFSV_IFND_EVT_INTF_AGING_TMR; 
  
      tmr->svc_id = cb->my_svc_id; 
      tmr->info.ifindex = actual_data->if_index;
      ifsv_tmr_start(tmr, cb);

      /* No aging timer, so mark this interface delete.*/
      actual_data->active = FALSE;
      actual_data->marked_del = TRUE;
      intf_evt = IFSV_INTF_REC_DEL;
      actual_data->if_info.oper_state = NCS_STATUS_DOWN;
   }
   else
       return NCSCC_RC_FAILURE;
   
   /* update the last change timer **/
   *attr = (*attr | NCS_IFSV_IAM_LAST_CHNG);
   m_GET_TIME_STAMP(actual_data->if_info.last_change);

   if(rc != NCSCC_RC_SUCCESS)
     return NCSCC_RC_FAILURE;
   
   return (rc);
} /* End of function ifsv_intf_rec_marked_del() */

/****************************************************************************
 * Name          : ifsv_intf_rec_del
 *
 * Description   : This function deletes the interface record for the given 
 *                 if_index.
 *                 This function removes the interface record from the 
 *                 patricia tree and return the record pointer to the 
 *                 called function.
 *                 
 *
 * Arguments     : ifindex   - Ifindex for which interface record needs to be 
 *                             deleted.
 *               : cb        - interface service control block 
 *
 * Return Values : IFSV_INTF_REC * recorded pointer which has been removed from
 *                  the interface tree.
 *
 * Notes         : The Caller has to take care of free this pointer.
 *****************************************************************************/
IFSV_INTF_REC * 
ifsv_intf_rec_del (uns32 ifindex, IFSV_CB *cb)
{
   IFSV_INTF_REC      *rec = NULL;
   NCS_PATRICIA_NODE   *if_node;
   NCS_PATRICIA_TREE   *p_tree = &cb->if_tbl;   
   NCS_LOCK      *intf_rec_lock = &cb->intf_rec_lock;
   
   
   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   do
   {
      
      if_node = ncs_patricia_tree_get (p_tree, (uns8 *)&ifindex);
      
      if (if_node == IFSV_NULL)
      {
         m_IFSV_LOG_STR_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_IF_TBL_DEL_FAILURE,
           "Lookup failed for intf record with ifindex : ",ifindex);
   /*      m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_TBL_DEL_FAILURE,ifindex,0);
*/
         break;
      }
      
      rec = (IFSV_INTF_REC *)if_node;
      rec->intf_data.active = FALSE;

      /* first delete the spt map record then IFSV_INTF_REC*/
     
/*
      ifsv_ifindex_spt_map_del(rec->intf_data.spt_info, cb);
*/

      /* Unregister this record from MAB, only for external interfaces */
      if (NCS_IFSV_SUBSCR_EXT == rec->intf_data.spt_info.subscr_scope)
              ifsv_unreg_ifmib_row(cb, rec->intf_data.mab_rec_row_hdl);

      if (ncs_patricia_tree_del (p_tree, if_node) != NCSCC_RC_SUCCESS)
      {
         m_IFSV_LOG_STR_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_IF_TBL_DEL_FAILURE,
           "Unable to delete : ifindex: ",ifindex);
/*         m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_TBL_DEL_FAILURE,ifindex,0);*/
         rec = IFSV_NULL;
         break;
      }
/*
      else
      {
          m_MMGR_FREE_IFSV_INTF_REC( if_node );
          if_node = NULL;
      }
*/

#if(NCS_IFSV_IPXS == 1)
       ipxs_update_ifdel(cb->my_svc_id, rec->intf_data.if_index);
#endif
      
      m_IFSV_LOG_STR_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_DEL_INTF_REC,
        "Ifindex : ",ifindex);
/*
      m_IFSV_LOG_DEL_IF_REC(cb->my_svc_id,IFSV_LOG_DEL_INTF_REC,\
         ifindex,0);*/
      
   } while (0);
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);

   return (rec);
}
                      
/****************************************************************************
 * Name          : ifsv_cleanup_tmr_find
 *
 * Description   : This function used to find the timer pointer for the given
 *                 shelf/slot/port/type/scope
 *                 
 *
 * Arguments     : spt         - shelf/slot/port/type/scope as a key to search.
 *               : p_tree      - Patricia tree pointer Ifindex map table.
 *
 * Return Values : IFSV_TMR * - (NULL on failure)
 *
 * Notes         : None.
 *****************************************************************************/
IFSV_TMR *
ifsv_cleanup_tmr_find (NCS_IFSV_SPT  *spt, 
                       NCS_PATRICIA_TREE *p_tree)
{
   IFSV_SPT_REC *spt_if_map;
   IFSV_TMR *tmr;

   spt_if_map = (IFSV_SPT_REC *)ncs_patricia_tree_get(p_tree, 
      (uns8 *)spt);

   if (spt_if_map == IFSV_NULL)
   {
      return (NULL);
   }
   
   tmr = &spt_if_map->tmr;
   return (tmr);
}

/****************************************************************************
 * Name          : ifsv_get_ifindex_from_spt
 *
 * Description   : This function used to find the corresponding ifindex for 
 *                 the given shelf/slot/port/type/scope.
 *                 
 *
 * Arguments     : o_ifindex   - Ifindex which is found for the given 
 *                               shelf/slot/port/type/scope.
 *                 spt         - shelf/slot/port/type/scope as a key to search.
 *               : cb          - Control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifsv_get_ifindex_from_spt (uns32 *o_ifindex, 
                           NCS_IFSV_SPT  spt, 
                           IFSV_CB *cb)                                
{
   IFSV_SPT_REC *spt_if_map;
   NCS_PATRICIA_TREE *p_tree = &cb->if_map_tbl;

   spt_if_map = (IFSV_SPT_REC *)ncs_patricia_tree_get(p_tree, 
      (uns8 *)&spt);

   if ((spt_if_map == IFSV_NULL) || (o_ifindex == IFSV_NULL))
   {
      m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_MAP_TBL_LOOKUP_FAILURE,spt.slot,spt.port); 
      return (NCSCC_RC_FAILURE);
   }
   
   *o_ifindex = spt_if_map->spt_map.if_index;
   m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_SPT_TO_INDEX_LOOKUP_SUCCESS,spt_if_map->spt_map.if_index,0); 
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifsv_ifindex_spt_map_add
 *
 * Description   : This function used to add the shelf/slot/subslot/port/type/scope Vs 
 *                 ifindex mapping.
 *                 
 *
 * Arguments     : spt     - shelf/slot/port/type/scope Vs ifindex mapping needed to 
 *                           added in the tree. 
 *               : cb      - CB pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifsv_ifindex_spt_map_add (NCS_IFSV_SPT_MAP *spt, 
                          IFSV_CB *cb)                               
{
   NCS_PATRICIA_TREE *p_tree = &cb->if_map_tbl;
   IFSV_SPT_REC *spt_if_map;

   spt_if_map = m_MMGR_ALLOC_IFSV_S_P_T_INDEX_MAP;

   if (spt_if_map == IFSV_NULL)
   {
      m_IFSV_LOG_SYS_CALL_FAIL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
               IFSV_LOG_MEM_ALLOC_FAIL,0);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_MEMSET(spt_if_map, 0, sizeof(IFSV_SPT_REC));

   spt_if_map->spt_map.if_index  = spt->if_index;   
   spt_if_map->spt_map.spt.shelf = spt->spt.shelf;
   spt_if_map->spt_map.spt.slot  = spt->spt.slot;   
   /* embedding subslot changes */
   spt_if_map->spt_map.spt.subslot  = spt->spt.subslot;
   spt_if_map->spt_map.spt.port  = spt->spt.port;   
   spt_if_map->spt_map.spt.type  = spt->spt.type;
   spt_if_map->spt_map.spt.subscr_scope  = spt->spt.subscr_scope;
   spt_if_map->pat_node.key_info = (uns8*)&spt_if_map->spt_map.spt;

   /* This is the idea of find and add */
   if (ncs_patricia_tree_get(p_tree, (uns8*)&spt->spt) == NULL)
   {
      if (ncs_patricia_tree_add (p_tree, &spt_if_map->pat_node) != NCSCC_RC_SUCCESS)
      {
         m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
            IFSV_LOG_IF_MAP_TBL_ADD_FAILURE,spt->if_index,spt->spt.port);
         m_MMGR_FREE_IFSV_S_P_T_INDEX_MAP(spt_if_map);
         /* fail to add the S/P/T Vs ifindex map - Log*/
         return (NCSCC_RC_FAILURE);
      }
   }
   else
   {
    m_MMGR_FREE_IFSV_S_P_T_INDEX_MAP(spt_if_map);
   }
   return (NCSCC_RC_SUCCESS);
}


/****************************************************************************
 * Name          : ifsv_ifindex_spt_map_del
 *
 * Description   : This function used to delete the shelf/slot/port/type/scope Vs 
 *                 ifindex mapping from the tree.
 *                 This function stop the aging timer, if it is running and
 *                 frees the record from the database.
 *                 
 *
 * Arguments     : spt     - shelf/slot/port/type/scope to be deleted from the ptree.
 *               : cb  - CB pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifsv_ifindex_spt_map_del (NCS_IFSV_SPT spt, 
                          IFSV_CB *cb)                               
{
   NCS_PATRICIA_TREE *p_tree = &cb->if_map_tbl;
   NCS_PATRICIA_NODE   *map_node;
   IFSV_SPT_REC       *spt_rec;

   map_node = ncs_patricia_tree_get (p_tree, (uns8 *)&spt);
   if (map_node == IFSV_NULL)
   {
      m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_MAP_TBL_DEL_FAILURE,spt.slot,spt.port);
      return (NCSCC_RC_FAILURE);
   }   
   
   ncs_patricia_tree_del (p_tree, map_node);   
   /* stop the timer */
   spt_rec = (IFSV_SPT_REC*)map_node;
   /* stop the cleanp timer if it is running */
   ifsv_tmr_stop(&spt_rec->tmr,cb);
   m_MMGR_FREE_IFSV_S_P_T_INDEX_MAP(map_node);
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifsv_ifindex_spt_map_del_all
 *
 * Description   : This is the function which deletes all the slot/port 
 *                 mapping from the map table.
 *                 
 *
 * Arguments     : cb        - interface service control block 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifsv_ifindex_spt_map_del_all (IFSV_CB *cb)
{
   NCS_IFSV_SPT        spt;
   NCS_PATRICIA_NODE   *if_node;
   IFSV_SPT_REC        *spt_info;
   uns32 res = NCSCC_RC_FAILURE;

   m_NCS_MEMSET(&spt, 0, sizeof(NCS_IFSV_SPT));
   if_node = ncs_patricia_tree_getnext(&cb->if_map_tbl, (uns8*)&spt);
   spt_info = (IFSV_SPT_REC*)if_node;
   while (spt_info != NULL)
   {      
      memcpy(&spt, &spt_info->spt_map.spt, 
         sizeof(NCS_IFSV_SPT));
      ifsv_ifindex_spt_map_del(spt_info->spt_map.spt, cb);            
      if_node = ncs_patricia_tree_getnext(&cb->if_map_tbl, (uns8*)&spt);
      spt_info = (IFSV_SPT_REC*)if_node;
      res = NCSCC_RC_SUCCESS;
   }

   if(res == NCSCC_RC_SUCCESS)
   {
     m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_IF_MAP_TBL_DEL_ALL_SUCCESS,0,0);
   }

   return(res);
}

/****************************************************************************
 * Name          : ifsv_get_spt_from_ifindex
 *
 * Description   : This function used to find the corresponding 
 *                 the shelf/slot/port/type/scope for a given ifindex.
 *                 
 *
 * Arguments     : ifindex   - Ifindex which is found for the given 
 *                               shelf/slot/port/type/scope.
 *                 spt         - shelf/slot/port/type/scope as a key to search.
 *               : cb          - Control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifsv_get_spt_from_ifindex (uns32 ifindex, NCS_IFSV_SPT  *o_spt, IFSV_CB *cb)
{

   IFSV_INTF_DATA *rec_data;

   if ((rec_data = ifsv_intf_rec_find (ifindex, cb)) == NULL)
   {
     m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_SPT_TO_INDEX_LOOKUP_FAILURE, ifindex, 0);
     return NCSCC_RC_FAILURE;
   }

   m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
         IFSV_LOG_SPT_TO_INDEX_LOOKUP_SUCCESS, ifindex, 0);

   *o_spt = rec_data->spt_info;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : ifsv_rms_if_rec_send
 *
 * Description   : This is the function which is used to checkpointing the data.
 *                 
 *
 * Arguments     : intf_data  - Interface data to be checkpointed.
 *               : rec_evt    - Action on the interface record. 
 *                 attr       - Attribute of the rec data which has changed.
 *                 cb         - IFSV Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : This function will be active only for IfD (TBD).
 *****************************************************************************/
uns32
ifsv_rms_if_rec_send (IFSV_INTF_DATA *intf_data, IFSV_INTF_REC_EVT rec_evt,
                      uns32 attr, IFSV_CB *cb)
{
   return(NCSCC_RC_SUCCESS);

}

/****************************************************************************
 * Name          : ifsv_evt_destroy
 *
 * Description   : This is the function which is used to free all event 
 *                 pointer which it has received/Send by IfD/IfND.
 *
 * Arguments     : evt  - This is the pointer which holds the  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_evt_destroy (IFSV_EVT *evt)
{
   switch(evt->type)
   {
   case IFD_EVT_INTF_CREATE:
      /* free the specific entry for this events - TBD*/
      m_MMGR_FREE_IFSV_EVT(evt);
      break;
   case IFD_EVT_INTF_DESTROY:         
   case IFD_EVT_IFINDEX_REQ:      
   case IFD_EVT_IFINDEX_CLEANUP:      
   case IFD_EVT_INIT_DONE:
   case IFD_EVT_TMR_EXP:
      m_MMGR_FREE_IFSV_EVT(evt);
      break;
      /* free the IFND events */
   case IFND_EVT_INTF_CREATE:
      /* free the specific entry for this events - TBD*/
      m_MMGR_FREE_IFSV_EVT(evt);
      break;
   case IFND_EVT_INTF_DESTROY:      
   case IFND_EVT_INIT_DONE:      
   case IFND_EVT_INTF_INFO_GET:   
      m_MMGR_FREE_IFSV_EVT(evt);
      break;
   /* NOTE: NCS_VIP related event are being
    * Taken care in the default case */
   default:
      m_MMGR_FREE_IFSV_EVT(evt);
      break;
   }
   return;
}


/***************************************************************************
 * Function :  ifsv_reg_ifmib_row
 *
 * Purpose  :  This function registers IFMIB table rows with MAB. 
 *
 * Input    :  cb         IFSV Control Block.
 *             ifindex    Inteface Index.
 *             row_hdl    The row handle returned by MAB.
 *
 * Returns  :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES    :  If NCSCC_RC_SUCCESS row_hdl will contain the handle
 *             returned by MAB.
 * 
 **************************************************************************/
static uns32 ifsv_reg_ifmib_row(IFSV_CB *cb, 
                             uns32 ifindex, 
                             NCSCONTEXT *row_hdl)
{
   NCSOAC_SS_ARG  ifsv_oac_arg;
   uns32          rc = NCSCC_RC_SUCCESS;
   NCSMAB_RANGE   *range;
   uns32  min_fltr[1] ;
   uns32  max_fltr[1] ;

   min_fltr[0]=ifindex;
   max_fltr[0]=ifindex;

   m_NCS_MEMSET(&ifsv_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

   /* Register for IFSV table rows*/
   ifsv_oac_arg.i_oac_hdl = cb->oac_hdl;
   ifsv_oac_arg.i_op      = NCSOAC_SS_OP_ROW_OWNED;
   ifsv_oac_arg.i_tbl_id  = NCSMIB_TBL_IFSV_IFTBL;

   ifsv_oac_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_RANGE;
   ifsv_oac_arg.info.row_owned.i_fltr.is_move_row_fltr = FALSE;
   range = &ifsv_oac_arg.info.row_owned.i_fltr.fltr.range;
   range->i_bgn_idx = 0;
   range->i_idx_len = IFSV_IFINDEX_INST_LEN;
   range->i_max_idx_fltr = max_fltr;
   range->i_min_idx_fltr = min_fltr;
   ifsv_oac_arg.info.row_owned.i_ss_cb = (NCSOAC_SS_CB)NULL;
   ifsv_oac_arg.info.row_owned.i_ss_hdl = cb->cb_hdl;

   if (ncsoac_ss(&ifsv_oac_arg) != NCSCC_RC_SUCCESS)
   {
      m_IFSV_LOG_STR_2_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_FUNC_RET_FAIL,"in function ifsv_reg_ifmib_row() ncsoac_ss()returned failure"," ");
      /* Log Error about skipping the table rows */
      rc = NCSCC_RC_FAILURE;
   }

   *row_hdl = (NCSCONTEXT)(long)ifsv_oac_arg.info.row_owned.o_row_hdl;

   return rc;
}


/***************************************************************************
 * Function :  ifsv_unreg_ifmib_row
 *
 * Purpose  :  This function unregisters IFSV IFMIB table rows with MAB. 
 *
 * Input    :  IFSV Control Block. 
 *             The row handle of the row that needs to be deleted.
 *
 * Returns  :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES    :  None.
 * 
 **************************************************************************/
static uns32 ifsv_unreg_ifmib_row(IFSV_CB *cb, NCSCONTEXT row_hdl)
{
   NCSOAC_SS_ARG  ifsv_oac_arg;
   uns32          rc = NCSCC_RC_SUCCESS ;


   m_NCS_MEMSET(&ifsv_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

   /* Unregister for IFSV table rows */
   ifsv_oac_arg.i_oac_hdl = cb->oac_hdl;
   ifsv_oac_arg.i_op      = NCSOAC_SS_OP_ROW_GONE;
   ifsv_oac_arg.i_tbl_id  = NCSMIB_TBL_IFSV_IFTBL;
   ifsv_oac_arg.info.row_gone.i_row_hdl = (long) row_hdl;


   if (ncsoac_ss(&ifsv_oac_arg) != NCSCC_RC_SUCCESS)
   {
      /* Log Error about skipping the table rows*/
      rc = NCSCC_RC_FAILURE;
   }

   return rc;
}


uns32 ifsv_modify_svcdest_list(NCS_IFSV_SVC_DEST_UPD *svcd, IFSV_INTF_DATA *dest, IFSV_CB *cb)
{
   NCS_SVDEST  *old_svdlist=0;
   uns32    cnt = 0, old_svdcnt=0, i=0;

   old_svdlist = dest->if_info.addsvd_list;
   old_svdcnt = dest->if_info.addsvd_cnt;

   if(svcd->i_type == NCS_IFSV_SVCD_ADD)
   {
      /* If the service is already present, just change the MDS Dest */
      for(i=0; i<old_svdcnt; i++)
      {
         if(old_svdlist[i].svcid == svcd->i_svdest.svcid)
         {
            old_svdlist[i].dest = svcd->i_svdest.dest;
            return NCSCC_RC_SUCCESS;
         }
      }

      cnt = dest->if_info.addsvd_cnt+1;
      dest->if_info.addsvd_list = m_MMGR_ALLOC_IFSV_NCS_SVDEST(cnt);
      if(dest->if_info.addsvd_list == 0)
      {
        m_IFSV_LOG_SYS_CALL_FAIL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
               IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_FAILURE;
      }
      /* Restore the old list */
      if(old_svdcnt)
      {
         memcpy(&dest->if_info.addsvd_list[0], 
            &old_svdlist[0], 
            (old_svdcnt*sizeof(NCS_SVDEST)));
      }
      
      /* Append the old list */
      memcpy(&dest->if_info.addsvd_list[old_svdcnt], 
            &svcd->i_svdest, sizeof(NCS_SVDEST));

      dest->if_info.addsvd_cnt = (uns8) cnt;
      dest->if_info.if_am = dest->if_info.if_am | NCS_IFSV_IAM_SVDEST;

   }
   else if(svcd->i_type == NCS_IFSV_SVCD_DEL)
   {
      for(i=0; i<old_svdcnt; i++)
      {
         if((old_svdlist[i].svcid == svcd->i_svdest.svcid) &&
            (m_NCS_MDS_DEST_EQUAL(&old_svdlist[i].dest, &svcd->i_svdest.dest)))
         {
            break;
         }
      }

      if(i == old_svdcnt)
      {
         /* No match, Do not free the list */
         old_svdlist = 0;
      }
      else if(old_svdcnt != 1)
      {
         /* Otherwise Replace the i th slot with the last slot */
         old_svdlist[i] = old_svdlist[old_svdcnt-1];
         old_svdcnt--;
         dest->if_info.addsvd_cnt = old_svdcnt;

         /* Do not free the list */
         old_svdlist = 0;
      }
      else if(old_svdcnt == 1)
      {
        dest->if_info.addsvd_cnt = 0;
        dest->if_info.addsvd_list = NULL;
      }
      /* In all other cases free the list */
   }

      /* Free the old list */
   if(old_svdlist)
      m_MMGR_FREE_IFSV_NCS_SVDEST(old_svdlist);
   return NCSCC_RC_SUCCESS;
}


uns32 ifsv_intf_info_cpy(NCS_IFSV_INTF_INFO *src, 
                         NCS_IFSV_INTF_INFO *dest, 
                         IFSV_MALLOC_USE_TYPE purpose)
{

   uns32 rc;

   /* Copy the record */
   memcpy(dest, src, sizeof(NCS_IFSV_INTF_INFO));

   rc = ifsv_intf_info_svd_list_cpy(src, dest, purpose);

   return rc;

}

uns32 ifsv_intf_info_svd_list_cpy(NCS_IFSV_INTF_INFO *src, 
                                  NCS_IFSV_INTF_INFO *dest, 
                                  IFSV_MALLOC_USE_TYPE purpose)
{
      /* Internal Use */
   if(src->addsvd_cnt)
   {
      dest->addsvd_list = (NCS_SVDEST *) m_MMGR_ALLOC_IFSV_NCS_SVDEST(src->addsvd_cnt);
      if(dest->addsvd_list == 0)
         return NCSCC_RC_OUT_OF_MEM;
   }

   if(src->delsvd_cnt)
   {
      dest->delsvd_list = (NCS_SVDEST *) m_MMGR_ALLOC_IFSV_NCS_SVDEST(src->delsvd_cnt);

      if(dest->delsvd_list == 0)
      { 
        m_MMGR_FREE_IFSV_NCS_SVDEST(dest->addsvd_list);
        return NCSCC_RC_OUT_OF_MEM;
      }
   }

   /* Copy the internal pointers */
   if(dest->addsvd_list)
   {
      memcpy(dest->addsvd_list, src->addsvd_list,
                     (src->addsvd_cnt * sizeof(NCS_SVDEST)));
   }

   dest->addsvd_cnt = src->addsvd_cnt;

   if(dest->delsvd_list)
   {
      memcpy(dest->delsvd_list, src->delsvd_list,
                     (src->delsvd_cnt * sizeof(NCS_SVDEST)));
   }

   dest->delsvd_cnt = src->delsvd_cnt;

   return NCSCC_RC_SUCCESS;
}


void ifsv_intf_info_free(NCS_IFSV_INTF_INFO *intf_info, IFSV_MALLOC_USE_TYPE purpose)
{

   if(intf_info->addsvd_list)
   {
      m_MMGR_FREE_IFSV_NCS_SVDEST(intf_info->addsvd_list);
   }

   if(intf_info->delsvd_list)
   {
      m_MMGR_FREE_IFSV_NCS_SVDEST(intf_info->delsvd_list);
   }

   return;
}

#if(NCS_IFSV_BOND_INTF == 1)
uns32
ifsv_bonding_assign_bonding_ifindex_to_master(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX bondingifindex)
{

     IFSV_INTF_DATA *master_intf_data;
     IFSV_INTF_DATA *slave_intf_data;
     IFSV_INTF_DATA *bond_intf_data;

    /* Get the intf record of the ifindex */
    bond_intf_data = ifsv_intf_rec_find (bondingifindex, ifsv_cb);
    if(bond_intf_data == NULL)
    {
       return NCSCC_RC_FAILURE;
    }
/*
    if(bond_intf_data->if_info.bind_master_ifindex == 0)
    {
     fprintf(stderr,"master index is 0, so exiting\n");
     return NCSCC_RC_FAILURE;
    }
*/

    master_intf_data = ifsv_intf_rec_find (bond_intf_data->if_info.bind_master_ifindex, ifsv_cb);
    if(master_intf_data == NULL)
    {
       return NCSCC_RC_FAILURE;
    }

     /* Donot allow to bond more than one bonding interface for a given master */
    if((master_intf_data->if_info.bind_master_ifindex != 0) && (master_intf_data->if_info.bind_master_ifindex != bondingifindex))
    {
            return NCSCC_RC_FAILURE;
    }

    master_intf_data->if_info.bind_master_ifindex = bondingifindex;
    master_intf_data->if_info.bind_slave_ifindex = 0;
    bond_intf_data->if_info.bind_master_info.node_id = m_NCS_NODE_ID_FROM_MDS_DEST(master_intf_data->originator_mds_destination);
    m_NCS_OS_STRNCPY(bond_intf_data->if_info.bind_master_info.if_name,master_intf_data->if_info.if_name,20);

    bond_intf_data->if_info.oper_state = master_intf_data->if_info.oper_state;
    m_NCS_OS_STRNCPY(bond_intf_data->if_info.if_name,master_intf_data->if_info.if_name,IFSV_IF_NAME_SIZE);


    slave_intf_data = ifsv_intf_rec_find (bond_intf_data->if_info.bind_slave_ifindex, ifsv_cb);
    if(slave_intf_data == NULL)
    {
       /* sometime may not be available */
       return NCSCC_RC_SUCCESS;
    }

    if((slave_intf_data->if_info.bind_slave_ifindex != 0) && (slave_intf_data->if_info.bind_slave_ifindex != bondingifindex))
    {
       return NCSCC_RC_FAILURE;
    }

    slave_intf_data->if_info.bind_master_ifindex = 0;
    slave_intf_data->if_info.bind_slave_ifindex = bondingifindex;
    bond_intf_data->if_info.bind_slave_info.node_id = m_NCS_NODE_ID_FROM_MDS_DEST(slave_intf_data->originator_mds_destination);
    m_NCS_OS_STRNCPY(bond_intf_data->if_info.bind_slave_info.if_name,slave_intf_data->if_info.if_name,20);
    return NCSCC_RC_SUCCESS;
}
/* This function updates bonding interface data and slave interface data before deleting
 * the master
 */
IFSV_INTF_DATA *
ifsv_binding_delete_master_ifindex(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX bonding_ifindex)
{
     IFSV_INTF_DATA *slave_intf_data;
     IFSV_INTF_DATA *bond_intf_data;
     NCS_IFSV_IFINDEX tmp_ifindex;
     NCS_IFSV_NODEID_IFNAME tmp_info;

     bond_intf_data = ifsv_intf_rec_find (bonding_ifindex, ifsv_cb);
     if (bond_intf_data == NULL )
     {
       return NULL;
     }
     slave_intf_data = ifsv_intf_rec_find (bond_intf_data->if_info.bind_slave_ifindex,ifsv_cb);
     if(slave_intf_data == NULL)
     {
       bond_intf_data->if_info.bind_master_ifindex = 0;
       bond_intf_data->if_info.bind_slave_ifindex = 0;
       bond_intf_data->if_info.oper_state = NCS_STATUS_DOWN;
       return bond_intf_data;
     }
     if(!slave_intf_data->active || slave_intf_data->marked_del)
     {
         /* We have a problem. The slave interface is down and master is also down.. 
              We dont do anything.. Let master come up.. */
        bond_intf_data->if_info.bind_master_ifindex = 0;
        bond_intf_data->if_info.oper_state = NCS_STATUS_DOWN;
        return bond_intf_data;
     }
     tmp_ifindex = bond_intf_data->if_info.bind_slave_ifindex;
     tmp_info.node_id= bond_intf_data->if_info.bind_slave_info.node_id;
     m_NCS_OS_STRNCPY(tmp_info.if_name,bond_intf_data->if_info.bind_slave_info.if_name,20);

     bond_intf_data->if_info.bind_slave_ifindex = 0; /* because master is getting deleted */ 
     bond_intf_data->if_info.bind_slave_info.node_id = bond_intf_data->if_info.bind_master_info.node_id;
     m_NCS_OS_STRNCPY( bond_intf_data->if_info.bind_slave_info.if_name,bond_intf_data->if_info.bind_master_info.if_name,20);

     bond_intf_data->if_info.bind_master_ifindex = tmp_ifindex;
     bond_intf_data->if_info.bind_master_info.node_id = tmp_info.node_id;
     m_NCS_OS_STRNCPY(bond_intf_data->if_info.bind_master_info.if_name,tmp_info.if_name,20);

     bond_intf_data->if_info.oper_state = slave_intf_data->if_info.oper_state;
     m_NCS_OS_STRNCPY(bond_intf_data->if_info.if_name,slave_intf_data->if_info.if_name,IFSV_IF_NAME_SIZE);


     slave_intf_data->if_info.bind_master_ifindex = bonding_ifindex;
     slave_intf_data->if_info.bind_slave_ifindex = 0;

     /* no need to modify the master as it is anyway getting deleted */
     return bond_intf_data;
}


/*This function resets the master's master and slave's slave to 0 
* before deleting binding interface
*/
void ifsv_delete_binding_interface(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX bonding_ifindex)
{
       IFSV_INTF_DATA* master_data;
       IFSV_INTF_DATA* slave_data;
       IFSV_INTF_DATA* rec_data;
#if(NCS_IFSV_IPXS == 1)
       IPXS_CB  *ipxs_cb = NULL;
       uns32    ipxs_hdl;
       IPXS_IFIP_NODE   *ifip_node = NULL;
       IPXS_IFIP_INFO   *ip_info = NULL;
       NCS_IPPFX   *ippfx;
       uns32       ii;
#endif
          rec_data = ifsv_intf_rec_find(bonding_ifindex,ifsv_cb);
          if( rec_data == IFSV_NULL )
          return;

          master_data=ifsv_intf_rec_find(rec_data->if_info.bind_master_ifindex,ifsv_cb);
          if(master_data!=IFSV_NULL && master_data->if_info.bind_master_ifindex == bonding_ifindex)
          master_data->if_info.bind_master_ifindex = 0;

          slave_data=ifsv_intf_rec_find(rec_data->if_info.bind_slave_ifindex,ifsv_cb);
          if(slave_data!=IFSV_NULL && slave_data->if_info.bind_slave_ifindex == bonding_ifindex)
          slave_data->if_info.bind_slave_ifindex = 0;
#if(NCS_IFSV_IPXS == 1)
             if(ifsv_cb->comp_type == IFSV_COMP_IFND)
             {
                /* Get the IPXS CB */
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
                         ippfx = IFSV_NULL;
                         for (ii = 0; ii < ip_info->ipaddr_cnt; ii++)
                         {
                            ippfx = &ip_info->ipaddr_list[ii].ipaddr;
                            /* since we are deleting a single ip at a time, cnt shud be 1 */
                            ifsv_del_from_os(rec_data,ippfx,1,ifsv_cb);
                         }
                      }
                   }
                }
             }
#endif

}

/* This function makes the slave index of binding index to be zero, when master is deleted.
   This is to see that, if any other interface is created with the same ifindex as slave index,
   it is not assumed to be slave
 */
uns32 ifsv_binding_check_ifindex_is_slave(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX ifindex,NCS_IFSV_IFINDEX *bonding_ifindex)
{
     IFSV_INTF_DATA *bond_intf_data;
     IFSV_INTF_DATA *slave_intf_data;
     NCS_IFSV_IFINDEX temp_ifindex;

     slave_intf_data = ifsv_intf_rec_find (ifindex, ifsv_cb);
    if(slave_intf_data == NULL)
    {
       return NCSCC_RC_FAILURE;
    }

    /* If it is the bonding interface return */
    if(slave_intf_data->spt_info.type == NCS_IFSV_INTF_BINDING)
    {
       return NCSCC_RC_FAILURE;
    }
    temp_ifindex = slave_intf_data->if_info.bind_slave_ifindex;
    if(temp_ifindex == 0)
    {
       return NCSCC_RC_FAILURE;
    }
     /* Get the intf record of the bonding interface */
    bond_intf_data = ifsv_intf_rec_find (temp_ifindex, ifsv_cb);
    if(bond_intf_data == NULL)
    {
       return NCSCC_RC_FAILURE;
    }
    if(bond_intf_data->spt_info.type != NCS_IFSV_INTF_BINDING)
    {
       return NCSCC_RC_FAILURE;
    }

    /* Check if this is the master ifindex of the bonding interface */
    if(ifindex == bond_intf_data->if_info.bind_slave_ifindex)
    {
        *bonding_ifindex = temp_ifindex;
     return NCSCC_RC_SUCCESS;
    }
    return NCSCC_RC_FAILURE;
}


uns32 ifsv_check_valid_bind_interface(IFSV_INTF_DATA intf_data,IFSV_CB* ifsv_cb)
{
     IFSV_INTF_DATA *master_intf_data;
     IFSV_INTF_DATA *slave_intf_data;

   /* bind port num should be greater than 0 and less than bondif_max_portnum */
    if((intf_data.spt_info.port <=0) || (intf_data.spt_info.port > ifsv_cb->bondif_max_portnum))
    return NCSCC_RC_FAILURE;

    if((intf_data.spt_info.shelf != IFSV_BINDING_SHELF_ID) || 
       (intf_data.spt_info.slot != IFSV_BINDING_SLOT_ID) || 
       (intf_data.spt_info.subslot != IFSV_BINDING_SUBSLOT_ID) || 
       (intf_data.spt_info.subscr_scope == NCS_IFSV_SUBSCR_EXT))
    return NCSCC_RC_FAILURE;

    master_intf_data = ifsv_intf_rec_find(intf_data.if_info.bind_master_ifindex,ifsv_cb);
    if((master_intf_data == NULL) || (master_intf_data->spt_info.subscr_scope != NCS_IFSV_SUBSCR_EXT))
    return NCSCC_RC_FAILURE;

    slave_intf_data = ifsv_intf_rec_find(intf_data.if_info.bind_slave_ifindex,ifsv_cb);
    if((slave_intf_data != NULL) && (slave_intf_data->spt_info.subscr_scope != NCS_IFSV_SUBSCR_EXT))
    return NCSCC_RC_FAILURE;

    return NCSCC_RC_SUCCESS;
}


#endif
