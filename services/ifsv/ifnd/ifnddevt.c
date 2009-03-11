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
  FILE NAME: IFNDDEVT.C

  DESCRIPTION: IDIM event received and send related routines.

  FUNCTIONS INCLUDED in this module:
  idim_send_ifnd_up_evt .... IDIM send IFND UP event to the drv coming up.
  idim_recv_hw_drv_msg ..... IDIM received hardware drv message.
  idim_get_hw_stats... ..... IDIM received "get Hardware stats" from IfA.
  idim_saf_health_chk. ..... IDIM received SAF health check from IfA.
  idim_set_hw_param ........ IDIM receives hardware parameter chng.
  idim_recv_hw_stats........ IDIM received Hardware stats from Hw Drv.
  idim_recv_hw_port_reg..... IDIM received Port register from Hw Drv.
  idim_recv_hw_port_status . IDIM received Port status(UP/DOWN) from Hw Drv.
  idim_send_hw_drv_msg. .... Send the hardware msg to hardware driver.
  idim_send_ifnd_evt........ Send the hardware msg to IfND.
  idim_process_mbx.......... Process IDIM mail BOX.
  idim_evt_destroy.......... Destroy received IDIM event structure.  

******************************************************************************/


static uns32 idim_recv_hw_drv_msg(IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb);
static uns32 idim_get_hw_stats(IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb);
static uns32 idim_set_hw_param (IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb);
static uns32 idim_saf_health_chk(IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb);

static uns32 idim_recv_hw_stats(IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb);
static uns32 idim_recv_hw_port_reg(IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb);
static uns32 idim_recv_hw_port_status(IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb);

extern uns32 idim_send_ifnd_evt (void* info, IFSV_EVT_TYPE evt_type, IFSV_IDIM_CB *cb);

const IFSV_IDIM_EVT_HANDLER ifsv_idim_evt_dispatch_tbl
[(IFSV_IDIM_EVT_MAX-IFSV_IDIM_EVT_BASE)] =
{
   idim_recv_hw_drv_msg,
   idim_get_hw_stats,
   idim_set_hw_param,
   idim_saf_health_chk
};

/****************************************************************************
 * Name          : idim_send_ifnd_up_evt
 *
 * Description   : This function called when any hardware driver is UP. This
 *                 will send a NCS_IFSV_HW_DRV_IFND_UP event to the hardware 
 *                 driver so that it would send all the port information which
 *                 it is having with it.
 *
 * Arguments     : to_dest  - ifnd DEstination address.
 *                 cb       - IfND Control Block
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
idim_send_ifnd_up_evt (MDS_DEST *to_dest, IFSV_CB *cb)
{
   NCS_IFSV_HW_DRV_REQ drv_evt;
   uns32 res = NCSCC_RC_FAILURE;
   
   m_NCS_MEMSET(&drv_evt, 0, sizeof(NCS_IFSV_HW_DRV_REQ));
   drv_evt.port_type.port_id = 0;
   drv_evt.port_type.type    = 0;   
   drv_evt.req_type          = NCS_IFSV_HW_DRV_IFND_UP;
/*EXT_INT */
   drv_evt.subscr_scope      = cb->ifmib_rec.spt_info.subscr_scope;
   drv_evt.info.ifnd_info.ifnd_addr = cb->my_dest;   
   drv_evt.info.ifnd_info.vrid   = cb->vrid;
   drv_evt.info.ifnd_info.nodeid = cb->my_node_id;
   
   res = idim_send_hw_drv_msg ((NCSCONTEXT)&drv_evt, NCS_IFSV_HW_DRV_IFND_UP, 
      to_dest, cb->my_mds_hdl);      
   return (res);
}


/****************************************************************************
 * Name          : idim_send_hw_drv_msg
 *
 * Description   : This function sends a message to the Interface hardware 
 *                 driver which it got it from IfND.
 *
 * Arguments     : info     - This is the message pointer need to be send to the 
 *                            IfND. 
 *                 msg_type - message type.
 *                 to_card  - Vcard ID to which it needs to send.
 *                 mds_hdl  - MDS handle.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 
idim_send_hw_drv_msg (void* info, NCS_IFSV_HW_DRV_MSG_TYPE msg_type, 
                      MDS_DEST *to_card, MDS_HDL mds_hdl)
{
   NCS_IFSV_HW_DRV_REQ *pMsg;
   uns32 res;
   
   pMsg = m_MMGR_ALLOC_IFSV_HW_DRV_EVT;
   if (pMsg == IFSV_NULL)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return (NCSCC_RC_FAILURE);
   }
   m_NCS_MEMSET(pMsg, 0, sizeof(NCS_IFSV_HW_DRV_REQ));
   
   switch(msg_type)
   {
   case NCS_IFSV_HW_DRV_STATS:
      memcpy(pMsg, info, sizeof(NCS_IFSV_HW_DRV_REQ));

      m_IFND_LOG_EVT_LL(IFSV_LOG_IDIM_EVT_INTF_DESTROY_SND,"IDIM Evt",\
         pMsg->port_type.port_id,pMsg->port_type.type);

      break;

   case NCS_IFSV_HW_DRV_IFND_UP:
      memcpy(pMsg, info, sizeof(NCS_IFSV_HW_DRV_REQ));

      m_IFND_LOG_EVT_L(IFSV_LOG_IDIM_EVT_IFND_UP_SND,"IDIM Evt",\
         pMsg->req_type);

      break;
   case NCS_IFSV_HW_DRV_SET_PARAM:
      memcpy(pMsg, info, sizeof(NCS_IFSV_HW_DRV_REQ));

      m_IFND_LOG_EVT_L(IFSV_LOG_IDIM_EVT_SET_HW_DRV_PARAM_SND,"IDIM Evt",\
         pMsg->req_type);

      break;
      
   default:
      m_IFND_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT,"IDIM Evt",pMsg->req_type);
      m_MMGR_FREE_IFSV_HW_DRV_EVT(pMsg);
      return (NCSCC_RC_FAILURE);
      break;
   }

   if (to_card != NULL)
   {
      res = ifsv_mds_normal_send(mds_hdl, NCSMDS_SVC_ID_IFND, 
         (NCSCONTEXT)pMsg, *to_card, NCSMDS_SVC_ID_IFDRV); 
   } else
   {
      res = ifsv_mds_scoped_send (mds_hdl, NCSMDS_SCOPE_INTRANODE,
                      NCSMDS_SVC_ID_IFND, (NCSCONTEXT)pMsg, 
                      NCSMDS_SVC_ID_IFDRV);      
   }
   
   /* Destroy the Message structure always */
   m_MMGR_FREE_IFSV_HW_DRV_EVT(pMsg);
   return (res);
}

/****************************************************************************
 * Name          : idim_send_ifnd_evt
 *
 * Description   : This function will gives the information to IfND which 
 *                 ever it got from the hardware driver module.
 *
 * Arguments     : info     - This is the message pointer need to be send to the 
 *                            IfND.
 *                 evt_type - event type.
 *                 cb       - This is the IDIM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 
idim_send_ifnd_evt (void* info, IFSV_EVT_TYPE evt_type, IFSV_IDIM_CB *cb)
{
   uns32 res = NCSCC_RC_FAILURE;
   IFSV_EVT   *evt;
   NCS_IPC_PRIORITY priority = NCS_IPC_PRIORITY_NORMAL;   
   IFSV_INTF_CREATE_INFO *intf_create = (IFSV_INTF_CREATE_INFO*)info;
   char log_info[45];
   
   do
   {
      evt = m_MMGR_ALLOC_IFSV_EVT;
      if (evt == IFSV_NULL)
      {
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
         break;
      }
      evt->type = evt_type;
      switch (evt_type)
      {
      case IFND_EVT_INTF_CREATE:         
         
         memcpy(&evt->info.ifnd_evt.info.intf_create, info, 
            sizeof(IFSV_INTF_CREATE_INFO));
         evt->info.ifnd_evt.info.intf_create.evt_orig = 
                                                NCS_IFSV_EVT_ORIGN_HW_DRV;
         evt->info.ifnd_evt.info.intf_create.intf_data.originator = 
            NCS_IFSV_EVT_ORIGN_HW_DRV;
         evt->info.ifnd_evt.info.intf_create.intf_data.current_owner = NCS_IFSV_OWNER_IFND; 

         m_IFND_LOG_EVT_LL(IFSV_LOG_IDIM_EVT_INTF_CREATE_SND,\
            ifsv_log_spt_string(intf_create->intf_data.spt_info,log_info),\
            intf_create->if_attr, intf_create->intf_data.current_owner);

         break;
      case IFND_EVT_INTF_DESTROY:
         memcpy(&evt->info.ifnd_evt.info.intf_destroy, info, 
            sizeof(IFSV_INTF_DESTROY_INFO));
         evt->info.ifnd_evt.info.intf_destroy.orign = 
            NCS_IFSV_EVT_ORIGN_HW_DRV;

         m_IFND_LOG_EVT_L(IFSV_LOG_IDIM_EVT_INTF_DESTROY_SND,\
            ifsv_log_spt_string(((IFSV_INTF_DESTROY_INFO*)info)->spt_type,log_info),\
            m_NCS_NODE_ID_FROM_MDS_DEST(((IFSV_INTF_DESTROY_INFO*)info)->own_dest));
         break;
      case IFND_EVT_IDIM_STATS_RESP:
         memcpy(&evt->info.ifnd_evt.info.stats_info, info, 
            sizeof(IFSV_EVT_STATS_INFO));

         m_IFND_LOG_EVT_L(IFSV_LOG_IDIM_EVT_INTF_STATS_RESP_SND,\
            ifsv_log_spt_string(((IFSV_EVT_STATS_INFO*)info)->spt_type,log_info),\
            m_NCS_NODE_ID_FROM_MDS_DEST(((IFSV_EVT_STATS_INFO*)info)->dest));
         break;
      default:
         m_IFND_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT,"IDIM Send Evt",evt_type);
         break;
      }
      evt->cb_hdl = cb->ifnd_hdl;

      /* post the event to the IfND thread */
      res = m_NCS_IPC_SEND(&cb->ifnd_mbx, evt, priority);   
      return(res);      
   } while (0);
   return(res);
}

/****************************************************************************
 * Name          : idim_recv_hw_stats
 *
 * Description   : This function receives hardware status information from 
 *                 hardware driver, which would be send to IfND.
 *
 * Arguments     : evt  - This is the pointer which holds the IDIM event struct
 *                 cb   - This is the IDIM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
idim_recv_hw_stats (IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb)
{
   IFSV_EVT_STATS_INFO stats;
   char log_info[45];
  

   m_NCS_MEMSET(&stats, 0, sizeof(IFSV_EVT_STATS_INFO));
   stats.spt_type.shelf = cb->shelf; 
   stats.spt_type.slot = cb->slot;
   /* embedding subslot changes */
   stats.spt_type.subslot = cb->subslot;
   stats.spt_type.port = evt->info.hw_info.port_type.port_id;
   stats.spt_type.type = evt->info.hw_info.port_type.type;
/* Changes for INT/EXT scope for subscr_scope overwrite. EXT_INT */
   stats.spt_type.subscr_scope = evt->info.hw_info.subscr_scope;
   stats.status              = evt->info.hw_info.info.stats.status;
   stats.dest                = evt->info.hw_info.info.stats.app_dest;
   stats.svc_id              = evt->info.hw_info.info.stats.app_svc_id;
   stats.usr_hdl             = evt->info.hw_info.info.stats.usr_hdl;
   memcpy(&stats.stat_info, &evt->info.hw_info.info.stats.stats, 
      sizeof(NCS_IFSV_INTF_STATS));

   m_IFND_LOG_EVT_L(IFSV_LOG_IDIM_EVT_HW_STATS_RCV,\
            ifsv_log_spt_string(stats.spt_type,log_info),stats.status);

   return (idim_send_ifnd_evt((void*)&stats, IFND_EVT_IDIM_STATS_RESP, cb));
}

/****************************************************************************
 * Name          : idim_recv_hw_port_reg
 *
 * Description   : when any new port gets registered with the IDIM then it 
 *                 will add the port registeration and send a informs to IfND
 *                 about the new port coming up.
 *
 * Arguments     : evt  - This is the pointer which holds the IDIM event struct
 *                 cb   - This is the IDIM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
idim_recv_hw_port_reg (IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb)
{   
   IFSV_INTF_CREATE_INFO       intf_info;
   IFSV_IDIM_PORT_TYPE_REG_TBL *reg_tbl;   
   NCS_IFSV_PORT_TYPE          port_type;
   uns32                       res = NCSCC_RC_SUCCESS;

   /* register with IDIM database */
   port_type.port_id = evt->info.hw_info.port_type.port_id;
   port_type.type    = evt->info.hw_info.port_type.type;
   m_IFND_LOG_EVT_LLL(IFSV_LOG_IDIM_EVT_HW_PORT_REG_RCV,"None",\
      port_type.port_id,port_type.type,evt->info.hw_info.info.reg_port.oper_state);
   reg_tbl = (IFSV_IDIM_PORT_TYPE_REG_TBL*)ncs_db_link_list_find(&cb->port_reg,
      (uns8*)&port_type);
   if (reg_tbl == IFSV_NULL)
   {
      /* since it is not available in the register table, add it */
      reg_tbl = m_MMGR_ALLOC_PORT_REG_TBL;
      if (reg_tbl == IFSV_NULL)
      {
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
         return (NCSCC_RC_FAILURE);
      }     
      m_NCS_MEMSET(reg_tbl, 0, sizeof(IFSV_IDIM_PORT_TYPE_REG_TBL));      
      reg_tbl->port_type.port_id = evt->info.hw_info.port_type.port_id;
      reg_tbl->port_type.type    = evt->info.hw_info.port_type.type;
      reg_tbl->q_node.key = (uns8*)&reg_tbl->port_type;
      ncs_db_link_list_add(&cb->port_reg, &reg_tbl->q_node);
   } 
   
   reg_tbl->drv_dest = evt->info.hw_info.info.reg_port.dest;

   m_NCS_MEMSET(&intf_info, 0, sizeof(IFSV_INTF_CREATE_INFO));
   intf_info.if_attr = evt->info.hw_info.info.reg_port.if_am;
   intf_info.intf_data.spt_info.shelf = cb->shelf; 
   intf_info.intf_data.spt_info.slot = cb->slot;
   /* embedding subslot changes */
   intf_info.intf_data.spt_info.subslot = cb->subslot;
   intf_info.intf_data.spt_info.port = evt->info.hw_info.port_type.port_id;
   intf_info.intf_data.spt_info.type = evt->info.hw_info.port_type.type;      
/* Changes for INT/EXT scope for subscr_scope overwrite. EXT_INT */
   intf_info.intf_data.spt_info.subscr_scope = evt->info.hw_info.subscr_scope;


   intf_info.intf_data.if_info.if_am = evt->info.hw_info.info.reg_port.if_am;
   intf_info.intf_data.originator_mds_destination = evt->info.hw_info.info.reg_port.dest;
   intf_info.intf_data.if_info.mtu        = evt->info.hw_info.info.reg_port.mtu;
   intf_info.intf_data.if_info.if_speed   = evt->info.hw_info.info.reg_port.speed;
   intf_info.intf_data.if_info.oper_state = evt->info.hw_info.info.reg_port.oper_state;
   intf_info.intf_data.if_info.admin_state = evt->info.hw_info.info.reg_port.admin_state;

   memcpy(intf_info.intf_data.if_info.phy_addr, 
      evt->info.hw_info.info.reg_port.phy_addr, (6*sizeof(uns8)));
   m_NCS_STRCPY(intf_info.intf_data.if_info.if_name, 
      evt->info.hw_info.info.reg_port.if_name);

   res = idim_send_ifnd_evt((void*)&intf_info, IFND_EVT_INTF_CREATE, cb);

   return(res);
}

/****************************************************************************
 * Name          : idim_recv_hw_port_status
 *
 * Description   : This function after geting the information about the port 
 *                 status from hardware driver, it will informs IfND.
 *
 * Arguments     : evt  - This is the pointer which holds the IDIM event struct
 *                 cb   - This is the IDIM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
idim_recv_hw_port_status (IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb)
{
   uns32 res = NCSCC_RC_SUCCESS;   
   IFSV_INTF_CREATE_INFO  create_info;
   IFSV_INTF_DESTROY_INFO dest_info;

   m_IFND_LOG_EVT_LL(IFSV_LOG_IDIM_EVT_HW_PORT_STATUS_RCV,"None",\
      evt->info.hw_info.port_type.port_id,\
      evt->info.hw_info.info.port_status.oper_state);

   if ((evt->info.hw_info.info.port_status.oper_state == NCS_STATUS_DOWN) || 
      (evt->info.hw_info.info.port_status.oper_state == NCS_STATUS_LL_DOWN))
   {
      m_NCS_MEMSET(&dest_info, 0, sizeof(IFSV_INTF_DESTROY_INFO));
      dest_info.spt_type.shelf = cb->shelf;
      dest_info.spt_type.slot  = cb->slot;
      /* embedding subslot changes */
      dest_info.spt_type.subslot  = cb->subslot;
      dest_info.spt_type.port  = evt->info.hw_info.port_type.port_id;
      dest_info.spt_type.type  = evt->info.hw_info.port_type.type;    
 /* Changes for INT/EXT scope for subscr_scope overwrite. EXT_INT */
      dest_info.spt_type.subscr_scope  = evt->info.hw_info.subscr_scope;
      res = idim_send_ifnd_evt((void*)&dest_info, IFND_EVT_INTF_DESTROY, cb);      
   } else
   {
      m_NCS_MEMSET(&create_info, 0, sizeof(IFSV_INTF_CREATE_INFO));
      create_info.if_attr = NCS_IFSV_IAM_OPRSTATE;
      create_info.intf_data.spt_info.shelf = cb->shelf; 
      create_info.intf_data.spt_info.slot = cb->slot;
      /* embedding subslot changes */
      create_info.intf_data.spt_info.subslot = cb->subslot;
      create_info.intf_data.spt_info.port = evt->info.hw_info.port_type.port_id;
      create_info.intf_data.spt_info.type = evt->info.hw_info.port_type.type;
      /* Changes for INT/EXT scope for subscr_scope overwrite. EXT_INT */
      create_info.intf_data.spt_info.subscr_scope = evt->info.hw_info.subscr_scope;
      create_info.intf_data.if_info.oper_state = 
         evt->info.hw_info.info.port_status.oper_state;
      res = idim_send_ifnd_evt((void*)&create_info, IFND_EVT_INTF_CREATE, cb);
   }  
   return (res);
}

/****************************************************************************
 * Name          : idim_recv_hw_drv_msg
 *
 * Description   : This event callback function which would be called when 
 *                 IDIM receives IFSV_IDIM_EVT_RECV_HW_DRV_MSG event from the 
 *                 hardware driver. This function gets the statistics/port info
 *                 and send it to IfND. 
 *
 * Arguments     : evt  - This is the pointer which holds the IDIM event struct
 *                 cb   - This is the IDIM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
idim_recv_hw_drv_msg (IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb)
{
   NCS_IFSV_HW_INFO *hw_info;
   uns32 res = NCSCC_RC_FAILURE;

   hw_info = &evt->info.hw_info;

   switch(hw_info->msg_type)
   {
      case NCS_IFSV_HW_DRV_STATS:
         res = idim_recv_hw_stats(evt, cb);
      break;
      case NCS_IFSV_HW_DRV_PORT_REG:
         res = idim_recv_hw_port_reg(evt, cb);
      break;
      case NCS_IFSV_HW_DRV_PORT_STATUS:
         res = idim_recv_hw_port_status(evt, cb);
      break;
      default:
      break;
   }
   return (res);
}


/****************************************************************************
 * Name          : idim_get_hw_stats
 *
 * Description   : This event callback function which would be called when 
 *                 IDIM receives IFSV_IDIM_EVT_GET_HW_STATS event from 
 *                 IfND. This function gives the trigger to the hardware driver
 *                 to get the statistics.
 *
 * Arguments     : evt  - This is the pointer which holds the IDIM event struct
 *                 cb   - This is the IDIM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
idim_get_hw_stats (IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb)
{
   NCS_IFSV_HW_DRV_REQ          drv_evt;
   NCS_IFSV_PORT_TYPE           port_type;
   IFSV_EVT_STATS_INFO          stats;   
   IFSV_IDIM_PORT_TYPE_REG_TBL  *reg_tbl;
   uns32                   res = NCSCC_RC_SUCCESS;
   char log_info[45];

   m_IFND_LOG_EVT_L(IFSV_LOG_IDIM_EVT_GET_HW_STATS_RCV,\
      ifsv_log_spt_string(evt->info.get_stats.slot_port,log_info),0);

   /* find whether the port and type req is already register with the IDIM */
   port_type.port_id = evt->info.get_stats.slot_port.port;
   port_type.type    = evt->info.get_stats.slot_port.type;
   reg_tbl = (IFSV_IDIM_PORT_TYPE_REG_TBL*)ncs_db_link_list_find(&cb->port_reg,
      (uns8*)&port_type);
   if (reg_tbl != IFSV_NULL)
   {
      m_NCS_MEMSET(&drv_evt,0,sizeof(drv_evt));
      drv_evt.port_type.port_id  = evt->info.get_stats.slot_port.port;
      drv_evt.port_type.type     = evt->info.get_stats.slot_port.type;
/* EXT_INT */
      drv_evt.subscr_scope       = evt->info.get_stats.slot_port.subscr_scope;
      drv_evt.info.req_info.dest_addr = evt->info.get_stats.app_dest;
      drv_evt.info.req_info.svc_id    = evt->info.get_stats.app_svc_id;
      drv_evt.info.req_info.app_hdl = evt->info.get_stats.usr_hdl;
      drv_evt.req_type              = NCS_IFSV_HW_DRV_STATS;
      /* send a message to the hardware driver to fetch the statisctics */
      res = idim_send_hw_drv_msg((void*)&drv_evt, NCS_IFSV_HW_DRV_STATS,
         &reg_tbl->drv_dest, cb->mds_hdl);
   } else
   {
      m_NCS_MEMSET(&stats, 0, sizeof(IFSV_EVT_STATS_INFO));
      /* send a failure message to the IfND */
      stats.spt_type.shelf = cb->shelf;
      stats.spt_type.slot  = cb->slot;
      /* embedding subslot changes */
      stats.spt_type.subslot  = cb->subslot;
      stats.spt_type.port  = evt->info.get_stats.slot_port.port;
      stats.spt_type.type  = evt->info.get_stats.slot_port.type;
/* EXT_INT */
      stats.spt_type.subscr_scope = evt->info.get_stats.slot_port.subscr_scope;
      stats.status    = FALSE;
      stats.dest      = evt->info.get_stats.app_dest;      
      stats.svc_id    = evt->info.get_stats.app_svc_id;

      /* send a message to IfND about the failure to fetch the statistic */
      idim_send_ifnd_evt((void*)&stats, IFND_EVT_IDIM_STATS_RESP, cb);
   }
   
   return (res);
}

/****************************************************************************
 * Name          : idim_set_hw_param
 *
 * Description   : This event callback function which would be called when 
 *                 IDIM receives IFSV_IDIM_EVT_SET_HW_PARM event from 
 *                 IfND. This function gives the trigger to the hardware driver
 *                 to set the changed interface paramaters.
 *
 * Arguments     : evt  - This is the pointer which holds the IDIM event struct
 *                 cb   - This is the IDIM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
idim_set_hw_param (IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb)
{
   NCS_IFSV_HW_DRV_REQ             drv_evt;
   NCS_IFSV_PORT_TYPE              port_type;
   IFSV_IDIM_PORT_TYPE_REG_TBL     *reg_tbl;
   IFSV_IDIM_EVT_SET_HW_PARAM_INFO *hw_parm;
   uns32                   res = NCSCC_RC_SUCCESS;
   char log_info[45];

   hw_parm = &evt->info.set_hw_parm;
   m_IFND_LOG_EVT_L(IFSV_LOG_IDIM_EVT_SET_HW_PARAM_RCV,\
      ifsv_log_spt_string(hw_parm->slot_port,log_info),0);

   /* find whether the port and type req is already register with the IDIM */   
   port_type.port_id = hw_parm->slot_port.port;
   port_type.type    = hw_parm->slot_port.type;
   reg_tbl = (IFSV_IDIM_PORT_TYPE_REG_TBL*)ncs_db_link_list_find(&cb->port_reg, 
      (uns8*)&port_type);
   if (reg_tbl != IFSV_NULL)
   {
      m_NCS_MEMSET(&drv_evt,0,sizeof(drv_evt));
      drv_evt.port_type.port_id = port_type.port_id;
      drv_evt.port_type.type    = port_type.type;            
      drv_evt.req_type          = NCS_IFSV_HW_DRV_SET_PARAM;
/*EXT_INT */
      drv_evt.subscr_scope      = evt->info.get_stats.slot_port.subscr_scope;
      memcpy(&drv_evt.info.set_param,&hw_parm->hw_param,
         sizeof(NCS_IFSV_INTF_INFO));
      /* send a message to the hardware driver to fetch the statisctics */
      res = idim_send_hw_drv_msg((void*)&drv_evt, NCS_IFSV_HW_DRV_SET_PARAM,
         &reg_tbl->drv_dest, cb->mds_hdl);
   }
   
   return (res);
}


/****************************************************************************
 * Name          : idim_saf_health_chk
 *
 * Description   : This event callback function which would be called when 
 *                 IDIM receives IFSV_IDIM_EVT_HEALTH_CHK event from 
 *                 IfND. This function dose the SAF health check point for
 *                 IDIM.
 *
 * Arguments     : evt  - This is the pointer which holds the IDIM event struct
 *                 cb   - This is the IDIM control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 
idim_saf_health_chk (IFSV_IDIM_EVT *evt, IFSV_IDIM_CB *cb)
{
   m_IFND_LOG_EVT_L(IFSV_LOG_IDIM_EVT_HEALTH_CHK_RCV,"None",0);

   m_NCS_SEM_GIVE(evt->info.health_chk.sem);   
   return(NCSCC_RC_SUCCESS);
}


/****************************************************************************
 * Name          : idim_process_mbx
 *
 * Description   : This is the function which is given as the input to the 
 *                 idim task.
 *
  * Arguments     : mbx  - This is the mail box pointer on which IDIM is 
 *                        going to block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

void
idim_process_mbx (SYSF_MBX *mbx)
{
   IFSV_IDIM_EVT *evt;
   IFSV_IDIM_EVT_HANDLER idim_fnc_hdl = IFSV_NULL;
   IFSV_IDIM_CB *idim_cb;
   uns32        idim_hdl;

   while(IFSV_NULL != (evt = (IFSV_IDIM_EVT *)m_IFSV_MBX_RECV(mbx, evt)))
   {
      if ((evt->evt_type >= IFSV_IDIM_EVT_BASE) && 
         (evt->evt_type < IFSV_IDIM_EVT_MAX))
      {
         idim_fnc_hdl = ifsv_idim_evt_dispatch_tbl[evt->evt_type];
         if (idim_fnc_hdl == IFSV_NULL)
         {
            /* LOG for error */
            idim_evt_destroy(evt);
            continue;
         }
         idim_hdl = m_IFSV_IDIM_GET_HDL;
         if ((idim_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFND, 
            idim_hdl)) != IFSV_NULL)
         {
            idim_fnc_hdl(evt, idim_cb);
            ncshm_give_hdl(idim_hdl);
         } else
         {
            /* this is not a good case - LOG */
            idim_evt_destroy(evt);
            break;
         }
      }
      idim_evt_destroy(evt);
   }
   return;
}



/****************************************************************************
 * Name          : idim_evt_destroy
 *
 * Description   : This is the function which is used to free the event 
 *                 pointer which it has received.
 *
 * Arguments     : evt  - This is the pointer which holds the  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void
idim_evt_destroy (IFSV_IDIM_EVT *evt)
{
   switch (evt->evt_type)
   {
      case IFSV_IDIM_EVT_RECV_HW_DRV_MSG:      
      case IFSV_IDIM_EVT_GET_HW_STATS:      
      case IFSV_IDIM_EVT_HEALTH_CHK:
         m_MMGR_FREE_IFSV_IDIM_EVT(evt);
      break;
      default:
         m_MMGR_FREE_IFSV_IDIM_EVT(evt);
      break;
   }
   return;
}
