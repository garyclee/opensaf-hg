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
  FILE NAME: IFD_IR.C

  DESCRIPTION: Functions for Interface redudancy.

******************************************************************************/

#if(NCS_IFSV_BOND_INTF == 1)

#include "ifd.h"


/****************************************************************************
 * Name           : ifd_binding_intf_create
 *
 * Description    : 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes            : None.
 *****************************************************************************/

uns32
ifd_binding_mib_send_evt (IFSV_CB *cb, uns8 portnum, NCS_IFSV_IFINDEX masterIfindex, 
                                                     NCS_IFSV_IFINDEX slaveIfIndex)
{
     IFSV_EVT *evt;
     IFSV_INTF_DATA *temp_intf_data;    
     int32 rc;


    /* Allocate the event structure */
    evt = m_MMGR_ALLOC_IFSV_EVT;
    if (evt == IFSV_NULL)
      return NCSCC_RC_FAILURE;

    m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));
    evt->cb_hdl = cb->cb_hdl;


    /* Send an event to IFD to create a bonding interface */
      evt->type = IFD_EVT_INTF_CREATE;
      m_IFSV_ALL_ATTR_SET(evt->info.ifd_evt.info.intf_create.if_attr);

      evt->info.ifd_evt.info.intf_create.intf_data.if_index = 0;

      evt->info.ifd_evt.info.intf_create.intf_data.originator = NCS_IFSV_EVT_ORIGN_IFD;
      evt->info.ifd_evt.info.intf_create.intf_data.originator_mds_destination = cb->my_dest;
     
      evt->info.ifd_evt.info.intf_create.intf_data.current_owner = NCS_IFSV_OWNER_IFD;

   /* SPT INFO of BINDING Interface - A hack */  
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.shelf  = IFSV_BINDING_SHELF_ID;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.slot   =  IFSV_BINDING_SLOT_ID;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.port  =  portnum;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.type  =  NCS_IFSV_INTF_BINDING;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.subscr_scope = NCS_IFSV_SUBSCR_INT;

      temp_intf_data = ifsv_intf_rec_find (masterIfindex, cb);

      evt->info.ifd_evt.info.intf_create.intf_data.if_info   = temp_intf_data->if_info;
      evt->info.ifd_evt.info.intf_create.intf_data.if_info.bind_master_ifindex =
                                                                                       masterIfindex;
      evt->info.ifd_evt.info.intf_create.intf_data.if_info.bind_slave_ifindex   =
                                                                                       slaveIfIndex;
      evt->info.ifd_evt.info.intf_create.intf_data.rec_type = NCS_IFSV_IF_INFO;
     
      /* Send the event */
      rc = m_NCS_IPC_SEND(&cb->mbx, evt, NCS_IPC_PRIORITY_NORMAL);
     return rc;

}
/****************************************************************************
 * Name           : ifd_binding_check_ifindex_is_master
 *
 * Description    : 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes            : None.
 *****************************************************************************/
uns32
ifd_binding_check_ifindex_is_master(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX ifindex,
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
 * Name           : ifd_binding_check_ifindex_is_master
 *
 * Description    : 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes            : None.
 *****************************************************************************/
IFSV_INTF_DATA *
ifd_binding_change_master_ifindex(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX bonding_ifindex)
{
     IFSV_INTF_DATA *master_intf_data;    
     IFSV_INTF_DATA *slave_intf_data;        
     NCS_IFSV_IFINDEX slave_ifindex;    

    /* Get the intf record of the ifindex */    
    master_intf_data = ifsv_intf_rec_find (bonding_ifindex, ifsv_cb);
    if(master_intf_data == NULL)
    {
       return NULL;
    }


    /* Check whether slave interface is UP and running */
    slave_ifindex = master_intf_data->if_info.bind_slave_ifindex;
    slave_intf_data = ifsv_intf_rec_find (slave_ifindex, ifsv_cb);
    if(slave_intf_data == NULL)
    {
       return NULL;
    }

     if(!slave_intf_data->active || slave_intf_data->marked_del)
     {
         /* We have a problem. The slave interface is down and master is also down.. 
              We dont do anything.. Let master come up.. */
         return NULL;
     }

     /* Change the master of the bonding interface */
     slave_ifindex = master_intf_data->if_info.bind_master_ifindex;
     master_intf_data->if_info.bind_master_ifindex = 
                          master_intf_data->if_info.bind_slave_ifindex;
     master_intf_data->if_info.bind_slave_ifindex = slave_ifindex;

     /* update the reverse mapping of master to bonding interface */ 
     /* no need to update the current slave.. becz it is going to get deleted */
     slave_intf_data->if_info.bind_master_ifindex = bonding_ifindex;

     return master_intf_data;

}

/****************************************************************************
 * Name           : ifd_binding_change_all_master_intfs
 *
 * Description    : 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes            : None.
 *****************************************************************************/
void
ifd_binding_change_all_master_intfs(IFSV_CB *ifsv_cb)
{
   uns32 ifindex = 0;   
   uns32 slave_ifindex;
   IFSV_INTF_REC       *intf_rec;
   NCS_PATRICIA_NODE   *if_node;   
   NCS_LOCK            *intf_rec_lock = &ifsv_cb->intf_rec_lock;
   int32 res;

   m_NCS_LOCK(intf_rec_lock, NCS_LOCK_WRITE);
   if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
                                                      (uns8*)&ifindex);
   intf_rec = (IFSV_INTF_REC*)if_node;

   while (intf_rec != IFSV_NULL)
   {
      if(intf_rec->intf_data.spt_info.type == NCS_IFSV_INTF_BINDING) 
         {
            /* This is bonding interface */
            /* Check whether the slave interfaces lie on the current shelf and slot */         
      if(intf_rec->intf_data.spt_info.shelf == ifsv_cb->shelf   &&
          intf_rec->intf_data.spt_info.slot  == ifsv_cb->slot)
      {
                 /* Yes.. the slave interface is on the current slot. We have switch the master 
                      and slave of this bonding interface */
                 /* Change the master of the bonding interface */
                slave_ifindex = intf_rec->intf_data.if_info.bind_master_ifindex;
                intf_rec->intf_data.if_info.bind_master_ifindex = 
                          intf_rec->intf_data.if_info.bind_slave_ifindex;
                intf_rec->intf_data.if_info.bind_slave_ifindex = slave_ifindex;

                printf("To test whether update goes to standby!!!!\n");
                 /* Send the trigger point to Standby IfD. */
                res = ifd_a2s_async_update(ifsv_cb, IFD_A2S_DATA_MODIFY_MSG,
                                         (uns8 *)(intf_rec));

                if(res == NCSCC_RC_SUCCESS)
                     ifd_bcast_to_ifnds(&intf_rec->intf_data, IFSV_INTF_REC_MODIFY, NCS_IFSV_IAM_CHNG_MASTER, ifsv_cb); 
     
         }
         }
      if_node = ncs_patricia_tree_getnext(&ifsv_cb->if_tbl,
         (uns8*)&ifindex);
      
      intf_rec = (IFSV_INTF_REC*)if_node;
   }
   
   m_NCS_UNLOCK(intf_rec_lock, NCS_LOCK_WRITE);
     
}


#if 0

uns8
ifnd_bonding_install_ipaddr(IFSV_CB  *cb,IFSV_INTF_REC *rec)
{
   uns32 ipxs_hdl;
   IPXS_CB *ipxs_cb;
   IPXS_IFIP_NODE *ipif_node;

   ipxs_hdl = m_IPXS_CB_HDL_GET();

   ipxs_cb = ncshm_take_hdl(cb->svc_id, ipxs_hdl);

   if (ipxs_cb == NULL)
   {
      m_IFA_LOG_API_LL(IFSV_LOG_IFA_IPXS_EVT_INFO, "ncshm_take_hdl returned NULL",0);
      return NCSCC_RC_FAILURE;
   }

   /* Get the master interface IP address and install it on slave interface */
   ipif_node = (IPXS_IFIP_NODE *) ncs_patricia_tree_get(&ipxs_cb->ifip_tbl, 
                                                           (uns8 *)&rec->intf_data.if_info.bind_master_ifindex );
   
   /* Install the IP address on the interface */
   



   
  /* Get the Handle Manager Handle */

              sprintf(ip_cmd, "/sbin/ip addr add %s dev %s",

}

#endif

#endif



