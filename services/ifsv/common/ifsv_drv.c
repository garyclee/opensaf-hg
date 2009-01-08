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

#include <config.h>

/*****************************************************************************
  FILE NAME: IFSV_DRV.C

  DESCRIPTION: IfSv driver module, which registers with MDS and stores the 
               registered port info.

  FUNCTIONS INCLUDED in this module:
  ncs_ifsv_drv_svc_req ........... SE API used by the driver to init/dest/send.
  ifsv_register_hw_driver ........ used by the driver to register port.
  ifsv_send_intf_info ............ used by driver to send any info to IDIM.
  ifsv_drv_port_comp ............. Compare the port type.
  ifsv_drv_init .................. Interface driver module init.  
  ifsv_drv_destroy ............... Interface driver module destroy.
  ifsv_drv_mds_msg_send .......... Send MDS message to IDIM.
  ifsv_drv_check_update_reg_info . update port reg info.  
  ifsv_drv_get_stats_evt ......... Receive "get statistic" from IDIM .
  ifsv_drv_port_sync_up .......... Receive "Port Syncup" from IDIM .  
  ifsv_drv_ifnd_up_evt ........... Receive "IfND UP" from IDIM .
  ifsv_drv_set_hw_param_evt ...... Set the hw parameters.
  ifsv_drv_mds_install ........... MDS install
  ifsv_drv_svc_evt ............... Monitors MDS service going UP/DOWN.
  ifsv_drv_mds_rcv ............... receives MDS message from IDIM.
  ifsv_drv_mds_enc_stats ......... MDS encode Statistic data.
  ifsv_drv_mds_enc_port_reg ...... MDS Encode Port register.
  ifsv_drv_mds_enc_port_status ... MDS Encode Port status.
  ifsv_drv_mds_enc ............... General MDS Encode to be send.
  ifsv_drv_mds_dec ............... General MDS decode received.
  ifsv_drv_mds_cpy ............... General MDS copy routine.
  ifsv_drv_mds_cpy ............... General MDS copy routine.

******************************************************************************/
#include "ifsv_drv.h"
#include "ncs_main_pvt.h"

uns32  glifsv_drv_hdl;

/* embedding subslot changes for backward compatibility */ 
MDS_CLIENT_MSG_FORMAT_VER
      DRV_WRT_IFND_MSG_FMT_ARRAY[DRV_WRT_IFND_SUBPART_VER_RANGE]={
           1 /*msg format version for  subpart version 1*/, 2 /* embedding subslot changes for backward compatibility */};

static uns32 
ifsv_drv_mds_install(IFSV_DRV_CB *drv_cb);
static uns32 
ifsv_drv_mds_shut (IFSV_DRV_CB *cb);
static uns32 
ifsv_drv_mds_enc(MDS_CALLBACK_ENC_INFO *enc, uns32 drv_hdl);
static uns32 
ifsv_drv_mds_dec(MDS_CALLBACK_DEC_INFO *dec, uns32 drv_hdl);
static uns32 
ifsv_drv_mds_cpy(MDS_CALLBACK_COPY_INFO *cpy, uns32 drv_hdl);
static uns32 
ifsv_drv_mds_rcv(MDS_CALLBACK_RECEIVE_INFO *rcv_info, uns32 drv_hdl);
static uns32 
ifsv_drv_svc_evt_info(MDS_CALLBACK_SVC_EVENT_INFO *evt_info, uns32 drv_hdl);
static uns32 ifsv_drv_mds_callback (NCSMDS_CALLBACK_INFO *info);
static uns32
ifsv_drv_port_comp(uns8 *key1, uns8 *key2);
static uns32
ifsv_drv_mds_msg_send(NCSCONTEXT info, NCS_IFSV_HW_DRV_MSG_TYPE msg_type, 
                      NCS_IFSV_PORT_TYPE *port_type, IFSV_DRV_CB *drv_cb);
static uns32
ifsv_drv_check_update_reg_info(IFSV_DRV_PORT_REG_TBL *found_reg_tbl, 
                                NCS_IFSV_PORT_REG *reg_msg, 
                                IFSV_DRV_CB *drv_cb);
static uns32
ifsv_drv_mds_normal_send(MDS_HDL mds_hdl, 
                          NCSMDS_SVC_ID from_svc, 
                          NCSCONTEXT evt, MDS_DEST dest, 
                          NCSMDS_SVC_ID to_svc);
static uns32
ifsv_clean_all_port_reg(IFSV_DRV_CB *drv_cb);

static uns32
ifsv_drv_get_stats_evt (NCS_IFSV_HW_DRV_REQ *evt, IFSV_DRV_CB *drv_cb);

static uns32
ifsv_drv_port_sync_up (NCS_IFSV_HW_DRV_REQ *evt, IFSV_DRV_CB *drv_cb);

static uns32
ifsv_drv_set_hw_param_evt (NCS_IFSV_HW_DRV_REQ *evt, IFSV_DRV_CB *drv_cb);

static uns32
ifsv_drv_ifnd_up_evt (NCS_IFSV_HW_DRV_REQ *evt, IFSV_DRV_CB *drv_cb);

static uns32 
ifsv_drv_mds_adest_get (IFSV_DRV_CB *cb);

static NCS_IFSV_SUBSCR_SCOPE
ifsv_drv_scope_update(uns8 *name);

static
uns32 get_word_from_file(FILE **fp, char *o_chword);

static
uns32 go_to_next_line(FILE **fp);


/****************************************************************************
 * Name          : ncs_ifsv_drv_svc_req
 *
 * Description   : This is the API which the hardware driver needs to call 
 *                 whenever any ports gets initalized.
 *
 * Arguments     : info  - information needed for the request.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ncs_ifsv_drv_svc_req(NCS_IFSV_DRV_SVC_REQ *info)
{
   uns32 res = NCSCC_RC_FAILURE;
   if (info == NULL)
   {
      return NCSCC_RC_FAILURE;
   }
   switch(info->req_type)
   {
   case NCS_IFSV_DRV_INIT_REQ:
      res = ifsv_drv_init(NCS_IFSV_DEF_VRID,NCS_HM_POOL_ID_COMMON);
      break;
   case NCS_IFSV_DRV_DESTROY_REQ:
      res = ifsv_drv_destroy(NCS_IFSV_DEF_VRID);
      break;
   case NCS_IFSV_DRV_PORT_REG_REQ:
      if (info->info.port_reg != NULL)
      {
         res = ifsv_register_hw_driver(info->info.port_reg);
      }
      break;
   case NCS_IFSV_DRV_SEND_REQ:
      if (info->info.hw_info != NULL)
      {
         res = ifsv_send_intf_info(info->info.hw_info);
      }
      break;
   default:      
      break;
   }
   return res;
}


/****************************************************************************
 * Name          : ifsv_drv_port_comp
 *
 * Description   : This is the function which is used to compare the give ports
 *
 * Arguments     : key1  - key which is already available in the list
 *                 key2  - new key to be compared
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : here we are not going to add the entry in the asscending 
 *                 order or descending order so we are not checking for 
 *                 greater or lesser.
 *****************************************************************************/
static uns32
ifsv_drv_port_comp (uns8 *key1, uns8 *key2)
{   
   NCS_IFSV_PORT_TYPE *port1 = (NCS_IFSV_PORT_TYPE*)key1;
   NCS_IFSV_PORT_TYPE *port2 = (NCS_IFSV_PORT_TYPE*)key2;

   if ((port1 == NULL) || (port2 == NULL))
   {
      return (0xffff);
   }   
   if ((port1->port_id == port2->port_id) && (port1->type == port2->type))
   {
      return (0);
   } else
   {
      return (0xffff);
   }   
}

/****************************************************************************
 * Name          : ifsv_drv_init
 *
 * Description   : Initializing the ifsv driver lib, which will be used by the
 *                 Interface Hardware driver.
 *
 * Arguments     : vrid  - Virtual router ID.
 *                 pool_id - Pool ID
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : Whenever this function returns failure then don't even try
 *                 to create the driver threads.
 *****************************************************************************/
uns32 
ifsv_drv_init (uns32 vrid, uns8 pool_id)
{   
   uns32 res = NCSCC_RC_FAILURE;
   IFSV_DRV_CB *drv_cb;
   uns32       drv_hdl;
   
   do
   {
      /* allocate the ifsv driver CB*/
      drv_cb = m_MMGR_ALLOC_IFSV_DRV_CB;
      if (drv_cb == NULL)
      {
         m_NCS_CONS_PRINTF("\nMemory failure\n");
         break;
      }
      m_NCS_MEMSET(drv_cb, 0, sizeof(IFSV_DRV_CB));

      drv_cb->vrid     = vrid;

      /* initialize the doubly link list for port registeration que */
      drv_cb->port_reg_tbl.order      = NCS_DBLIST_ANY_ORDER;
      drv_cb->port_reg_tbl.cmp_cookie = ifsv_drv_port_comp;

      /* Initialize the Encode decode routine */
      m_NCS_EDU_HDL_INIT(&drv_cb->edu_hdl);

      /* Initialization of LOCK */
      res = m_NCS_OS_LOCK(&drv_cb->port_reg_lock, NCS_OS_LOCK_CREATE, 0);
      if ( res == NCSCC_RC_FAILURE)
      {     
         m_NCS_CONS_PRINTF("\nLock create failure\n");
         goto ifsv_drv_lock_fail;
         break;
      }

      /* create an handle manager */
      drv_hdl = ncshm_create_hdl(pool_id, NCS_SERVICE_ID_IFDRV, 
         (NCSCONTEXT)drv_cb);
      if (drv_hdl == 0)
      {
         m_NCS_CONS_PRINTF("\nncshm_create_hdl create failure\n");
         res = NCSCC_RC_FAILURE;
         goto ifsv_drv_hdl_fail;
         break;
      }

      /* initialize the MDS */
      drv_cb->cb_hdl = drv_hdl;
      res = ifsv_drv_mds_install(drv_cb);
      if (res != NCSCC_RC_FAILURE)
      {
         m_IFSV_DRV_STORE_HDL(drv_hdl);
      } else
      {
         goto ifsv_drv_mds_fail;
      }      
      m_NCS_CONS_PRINTF("\nDriver Init Done\n");
      goto ifsv_drv_init_done;
   } while(0);


ifsv_drv_mds_fail:
   ncshm_destroy_hdl(NCS_SERVICE_ID_IFDRV, drv_hdl);
ifsv_drv_hdl_fail:
   m_NCS_OS_LOCK(&drv_cb->port_reg_lock, NCS_OS_LOCK_RELEASE, 0);
ifsv_drv_lock_fail:
   /* EDU cleanup */
   m_NCS_EDU_HDL_FLUSH(&drv_cb->edu_hdl);
   m_MMGR_FREE_IFSV_DRV_CB(drv_cb);
   m_NCS_CONS_PRINTF("\nDriver Init failure\n");
   
ifsv_drv_init_done:
   return (res);
}

/****************************************************************************
 * Name          : ifsv_drv_destroy
 *
 * Description   : Destroy the ifsv driver lib.
 *
 * Arguments     : vrid  - Virtual router ID. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
 *****************************************************************************/
uns32
ifsv_drv_destroy(uns32 vrid)
{
   IFSV_DRV_CB *drv_cb;
   uns32       drv_hdl;

   drv_hdl = m_IFSV_DRV_GET_HDL;
   if ((drv_cb = (IFSV_DRV_CB*) ncshm_take_hdl(NCS_SERVICE_ID_IFDRV, 
      (uns32)drv_hdl)) != NULL)
   {
      /** cleanup all the port register records **/
      ifsv_clean_all_port_reg(drv_cb);
      ncshm_give_hdl(drv_hdl);
      ncshm_destroy_hdl(NCS_SERVICE_ID_IFDRV, drv_hdl);
      m_NCS_OS_LOCK(&drv_cb->port_reg_lock, NCS_OS_LOCK_RELEASE, 0);   
      /*** shut the MDS session **/
      ifsv_drv_mds_shut(drv_cb);
      /* EDU cleanup */
      m_NCS_EDU_HDL_FLUSH(&drv_cb->edu_hdl);
      m_MMGR_FREE_IFSV_DRV_CB(drv_cb);
      m_NCS_CONS_PRINTF("\nDriver destroy done\n");
      return(NCSCC_RC_SUCCESS);
   }
   else
   {
     m_NCS_CONS_PRINTF("\nDriver destroy failure\n");
     return NCSCC_RC_FAILURE;
   }
}

/****************************************************************************
 * Name          : ifsv_register_hw_driver
 *
 * Description   : This is the API which the hardware driver needs to call 
 *                 whenever any ports gets initalized.
 *
 * Arguments     : reg_msg  - This is the register message which the driver 
 *                            needs to pass during its registeration.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifsv_register_hw_driver (NCS_IFSV_PORT_REG *reg_msg)
{
   uns32 res = NCSCC_RC_FAILURE;
   IFSV_DRV_PORT_REG_TBL     *reg_tbl;
   IFSV_DRV_PORT_REG_TBL     *found_reg_tbl;
   NCS_IFSV_PORT_TYPE        port_type;
   IFSV_DRV_CB               *drv_cb;
   uns32                     drv_hdl = m_IFSV_DRV_GET_HDL;   
  
   if(reg_msg->hw_get_set_cb == NULL)
     return res;

   if(((reg_msg->port_info.if_am & NCS_IFSV_IAM_MTU) == NCS_IFSV_IAM_MTU ) ||
      ((reg_msg->port_info.if_am & NCS_IFSV_IAM_IFSPEED) == 
        NCS_IFSV_IAM_IFSPEED) ||
      ((reg_msg->port_info.if_am & NCS_IFSV_IAM_PHYADDR) ==  
        NCS_IFSV_IAM_PHYADDR) ||
      ((reg_msg->port_info.if_am & NCS_IFSV_IAM_ADMSTATE) ==  
        NCS_IFSV_IAM_ADMSTATE) ||
      ((reg_msg->port_info.if_am & NCS_IFSV_IAM_OPRSTATE) ==  
        NCS_IFSV_IAM_OPRSTATE) ||
      ((reg_msg->port_info.if_am & NCS_IFSV_IAM_NAME) == NCS_IFSV_IAM_NAME) ||
      ((reg_msg->port_info.if_am & NCS_IFSV_IAM_DESCR) == NCS_IFSV_IAM_DESCR) ||
      ((reg_msg->port_info.if_am & NCS_IFSV_IAM_LAST_CHNG) == 
        NCS_IFSV_IAM_LAST_CHNG) ||
      ((reg_msg->port_info.if_am & NCS_IFSV_IAM_SVDEST) == NCS_IFSV_IAM_SVDEST))
    {
       res = NCSCC_RC_SUCCESS;
    }
   
   if(res != NCSCC_RC_SUCCESS)
       return res;


   if (reg_msg != NULL)
   {
      if ((drv_cb = (IFSV_DRV_CB*) ncshm_take_hdl(NCS_SERVICE_ID_IFDRV, 
         drv_hdl)) != NULL)
      {
         m_NCS_LOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);
         do
         {            
            port_type.port_id = reg_msg->port_info.port_type.port_id;
            port_type.type    = reg_msg->port_info.port_type.type;
            /* assign the vcard ID */
            reg_msg->port_info.dest = drv_cb->my_dest;
            /* check whether the port has been already registered */
            found_reg_tbl = (IFSV_DRV_PORT_REG_TBL*)
               ncs_db_link_list_find(&drv_cb->port_reg_tbl, (uns8*)&port_type);
            if (found_reg_tbl == NULL)
            {
               /* allocate the port register information */
               reg_tbl = m_MMGR_ALLOC_IFSV_DRV_PORT_TBL;
               if (reg_tbl == NULL)
               {
                  m_NCS_CONS_PRINTF("\nMemory failure\n");
                  break;
               }
               m_NCS_MEMSET(reg_tbl, 0, sizeof(IFSV_DRV_PORT_REG_TBL));
               m_NCS_MEMCPY(&reg_tbl->port_reg, reg_msg, 
                  sizeof(NCS_IFSV_PORT_REG));
               /* store the register information */
               reg_tbl->q_node.key = (uns8*)&reg_tbl->port_reg.port_info.port_type;
               ncs_db_link_list_add(&drv_cb->port_reg_tbl, &reg_tbl->q_node); 

               /* send the register info to the coresponding IfND (If it is UP)*/
               res = ifsv_drv_mds_msg_send((NCSCONTEXT)&reg_msg->port_info, 
                  NCS_IFSV_HW_DRV_PORT_REG, &reg_msg->port_info.port_type,
                  drv_cb);

            } else
            {
               /* check whether there is any change in the register information, 
                * if so than send a register message to IDIM 
                */
               ifsv_drv_check_update_reg_info(found_reg_tbl, reg_msg, drv_cb);               
            }
         } while(0);
         m_NCS_UNLOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);
         ncshm_give_hdl(drv_hdl);
      }
   }
/*
   if(res == NCSCC_RC_SUCCESS)
     m_NCS_CONS_PRINTF("\nDriver port reg done\n");
   else
     m_NCS_CONS_PRINTF("\nDriver port reg failure\n");
*/
    
   return (res);
}

/****************************************************************************
 * Name          : ifsv_clean_all_port_reg
 *
 * Description   : This is the routine which cleans up all the port 
 *                 registeration informations.
 *
 * Arguments     : drv_cb        - driver Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifsv_clean_all_port_reg(IFSV_DRV_CB *drv_cb)
{
   NCS_DB_LINK_LIST_NODE *reg_tbl;
   m_NCS_LOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);
   while(1)
   {
      reg_tbl = ncs_db_link_list_dequeue(&drv_cb->port_reg_tbl);
      if (reg_tbl == NULL)
         break;
      m_MMGR_FREE_IFSV_DRV_PORT_TBL(reg_tbl);
   }
   m_NCS_UNLOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);
   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifsv_drv_mds_shut
 *
 * Description   : This function un-registers the IFSV_DRV Service with MDS.
 *
 * Arguments     : cb   : IFD control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifsv_drv_mds_shut (IFSV_DRV_CB *cb)
{
   NCSMDS_INFO          arg;
   uns32                rc;

   /* Un-install your service into MDS. 
   No need to cancel the services that are subscribed*/
   m_NCS_OS_MEMSET(&arg,0,sizeof(NCSMDS_INFO));

   arg.i_mds_hdl        = cb->mds_hdl;
   arg.i_svc_id         = NCSMDS_SVC_ID_IFDRV;
   arg.i_op             = MDS_UNINSTALL;

   rc = ncsmds_api(&arg);
   return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_mds_msg_send
 *
 * Description   : This function sends the message to the IDIM from interface 
 *                 driver.
 *
 * Arguments     : info      - the information which needs to be send to IDIM.
 *                 msg_type  - message type.
 *                 port_type - port and type;
 *                 cb        - driver Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifsv_drv_mds_msg_send (NCSCONTEXT info, NCS_IFSV_HW_DRV_MSG_TYPE msg_type, 
                       NCS_IFSV_PORT_TYPE *port_type, IFSV_DRV_CB *drv_cb)
{
   uns32 res = NCSCC_RC_SUCCESS;
   NCS_IFSV_HW_INFO *hw_info;
   IFSV_DRV_PORT_REG_TBL     *found_reg_tbl = NULL;

   if (drv_cb->ifnd_up == TRUE)
   {
      hw_info = m_MMGR_ALLOC_IFSV_DRV_IDIM_MSG;
      if (hw_info == NULL)
      {
         m_NCS_CONS_PRINTF("\nMemory failure\n");
         return (NCSCC_RC_FAILURE);
      }
      m_NCS_MEMSET(hw_info, 0, sizeof(NCS_IFSV_HW_INFO));
      
      hw_info->msg_type  = msg_type;
      hw_info->port_type.port_id   = port_type->port_id;      
      hw_info->port_type.type      = port_type->type;      
/* Changes for INT/EXT scope for subscr_scope overwrite. EXT_INT */
      found_reg_tbl = (IFSV_DRV_PORT_REG_TBL*)
           ncs_db_link_list_find(&drv_cb->port_reg_tbl, (uns8*)port_type);

      if(found_reg_tbl != NULL)
      {
         hw_info->subscr_scope = ifsv_drv_scope_update(found_reg_tbl->port_reg.port_info.if_name);
      }
      else
      {
         hw_info->subscr_scope = NCS_IFSV_SUBSCR_EXT;
      }

      
      
      switch (msg_type)
      {
      case NCS_IFSV_HW_DRV_STATS:
         m_NCS_MEMCPY(&hw_info->info.stats, info, sizeof(NCS_IFSV_PORT_STATS));
         m_NCS_CONS_PRINTF("\nDRV Send Hardware Stats To IDIM." 
                           " port_type->port_id = %d,port_type->type = %d\n",
                           port_type->port_id,port_type->type);
         break;
      case NCS_IFSV_HW_DRV_PORT_REG:
         m_NCS_MEMCPY(&hw_info->info.reg_port, info, 
            sizeof(NCS_IFSV_PORT_INFO));
         m_NCS_CONS_PRINTF("\nDRV Send Port Reg To IDIM." 
                           " port_type->port_id = %d,port_type->type = %d\n",
                           port_type->port_id,port_type->type);
         break;   
      case NCS_IFSV_HW_DRV_PORT_STATUS:
         m_NCS_MEMCPY(&hw_info->info.port_status, info, 
            sizeof(NCS_IFSV_PORT_STATUS));
         m_NCS_CONS_PRINTF("\nDRV Send Port Status To IDIM." 
                           " port_type->port_id = %d,port_type->type = %d\n",
                           port_type->port_id,port_type->type);
         break;
      default:
         m_MMGR_FREE_IFSV_DRV_IDIM_MSG(hw_info);
         return (NCSCC_RC_FAILURE);
         break;
      }   
      res = ifsv_drv_mds_normal_send(drv_cb->mds_hdl, NCSMDS_SVC_ID_IFDRV,
         (NCSCONTEXT)hw_info, drv_cb->ifnd_dest, NCSMDS_SVC_ID_IFND);      
      
      m_MMGR_FREE_IFSV_DRV_IDIM_MSG(hw_info);
   }
   return(res);
}

/****************************************************************************
 * Name          : ifsv_drv_check_update_reg_info
 *
 * Description   : This function checks for any change in the parameter with
 *                 the already available port registeration and send the 
 *                 message to IDIM correspondingly.
 *
 * Arguments     : found_reg_tbl - Table info which is already registered.
 *                 reg_msg       - New register info. 
 *                 drv_cb        - driver Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifsv_drv_check_update_reg_info (IFSV_DRV_PORT_REG_TBL *found_reg_tbl, 
                                NCS_IFSV_PORT_REG *reg_msg, 
                                IFSV_DRV_CB *drv_cb)
{
   NCS_IFSV_PORT_REG *pres_port_tbl = &found_reg_tbl->port_reg;
   NCS_BOOL          chng_flag      = FALSE;
   uns32             res = NCSCC_RC_SUCCESS;

   reg_msg->port_info.if_am = 0;

   if (pres_port_tbl->port_info.admin_state != reg_msg->port_info.admin_state)
   {
      chng_flag = TRUE;
      reg_msg->port_info.if_am |= NCS_IFSV_IAM_ADMSTATE;
      pres_port_tbl->port_info.admin_state = reg_msg->port_info.admin_state;
   }

   if (pres_port_tbl->port_info.oper_state != reg_msg->port_info.oper_state)
   {
      chng_flag = TRUE;
      reg_msg->port_info.if_am |= NCS_IFSV_IAM_OPRSTATE;
      pres_port_tbl->port_info.oper_state = reg_msg->port_info.oper_state;
   }

   if (pres_port_tbl->port_info.mtu != reg_msg->port_info.mtu)
   {
      chng_flag = TRUE;
      reg_msg->port_info.if_am |= NCS_IFSV_IAM_MTU;
      pres_port_tbl->port_info.mtu = reg_msg->port_info.mtu;
   }

   if (pres_port_tbl->port_info.speed != reg_msg->port_info.speed)
   {
      chng_flag = TRUE;
      reg_msg->port_info.if_am |= NCS_IFSV_IAM_IFSPEED;
      pres_port_tbl->port_info.speed = reg_msg->port_info.speed;
   }
   
   if (m_NCS_MEMCMP(&pres_port_tbl->port_info.phy_addr, 
      &reg_msg->port_info.phy_addr, (6*sizeof(uns8))) != 0)
   {
      chng_flag = TRUE;
      reg_msg->port_info.if_am |= NCS_IFSV_IAM_PHYADDR;
      m_NCS_MEMCPY(&pres_port_tbl->port_info.phy_addr, 
         &reg_msg->port_info.phy_addr, (6*sizeof(uns8)));
   }

   if (m_NCS_STRCMP(&pres_port_tbl->port_info.if_name, 
      &reg_msg->port_info.if_name) != 0)
   {
      chng_flag = TRUE;
      reg_msg->port_info.if_am |= NCS_IFSV_IAM_NAME;
      m_NCS_STRCPY(&pres_port_tbl->port_info.if_name, 
         &reg_msg->port_info.if_name);
   }
   
   pres_port_tbl->drv_data       = reg_msg->drv_data;
   pres_port_tbl->hw_get_set_cb = reg_msg->hw_get_set_cb;

   if (chng_flag == TRUE)
   {
      /* send a message to IDIM about the registeration */
      res = ifsv_drv_mds_msg_send ((NCSCONTEXT)&reg_msg->port_info, 
                  NCS_IFSV_HW_DRV_PORT_REG, &reg_msg->port_info.port_type,
                  drv_cb);
   }

   return (res);
}

/****************************************************************************
 * Name          : ifsv_send_intf_info
 *
 * Description   : This is the API which the hardware driver needs to call 
 *                 whenever it needs to send message to IDIM.
 *
 * Arguments     : intf_hw_info  - This is the interface Hardware information
 *                                 which it needs to inform to IDIM.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : This is the synchronous call.
 *****************************************************************************/
uns32 
ifsv_send_intf_info (NCS_IFSV_HW_INFO *intf_hw_info)
{
   uns32            res = NCSCC_RC_SUCCESS;
   NCS_IFSV_HW_INFO *msg;
   IFSV_DRV_CB      *drv_cb;
   uns32            drv_hdl = m_IFSV_DRV_GET_HDL;   
   IFSV_DRV_PORT_REG_TBL     *found_reg_tbl, *reg_tbl = NULL;
   NCS_IFSV_PORT_TYPE        port_type;

   msg = m_MMGR_ALLOC_IFSV_DRV_IDIM_MSG;
   if ((msg == NULL) || (intf_hw_info == NULL))
   {
      m_NCS_CONS_PRINTF("\nMemory failure\n");
      return (NCSCC_RC_FAILURE);
   }
   m_NCS_MEMSET(msg, 0, sizeof(NCS_IFSV_HW_INFO));
   
   if ((drv_cb = (IFSV_DRV_CB*) ncshm_take_hdl(NCS_SERVICE_ID_IFDRV, 
      drv_hdl)) != NULL)
   {        
      /* check whether the interface hardware needs to send the operational 
       * status of the port 
       */
      if (intf_hw_info->msg_type == NCS_IFSV_HW_DRV_PORT_STATUS)
      {
         m_NCS_LOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);
         /* check whether the port has been already registered */
         port_type = intf_hw_info->port_type;         

         found_reg_tbl = (IFSV_DRV_PORT_REG_TBL*)
            ncs_db_link_list_find(&drv_cb->port_reg_tbl, (uns8*)&port_type);

         if (found_reg_tbl != NULL)
         {
            found_reg_tbl->port_reg.port_info.oper_state = 
               intf_hw_info->info.port_status.oper_state;
         }
         m_NCS_UNLOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);
      }

      switch(intf_hw_info->msg_type) 
      {
       case NCS_IFSV_HW_DRV_PORT_STATUS:
       case NCS_IFSV_HW_DRV_STATS:
           break;
       default :
         ncshm_give_hdl(drv_hdl);
         m_MMGR_FREE_IFSV_DRV_IDIM_MSG(msg);
         return NCSCC_RC_FAILURE;
      }
      
      
      /* copy the hardware message */
      m_NCS_MEMCPY(msg, intf_hw_info, sizeof(NCS_IFSV_HW_INFO));
      /* Changes for INT/EXT scope for subscr_scope overwrite. EXT_INT */
      /* Ex: Takes the port Num and Type :: ethernet is 26 and PortNum is 0 for eth0
         on the basis of portno && Type, the below given api will search and overwrite the SubscriptionScope */
      reg_tbl = (IFSV_DRV_PORT_REG_TBL*)
           ncs_db_link_list_find(&drv_cb->port_reg_tbl, (uns8*)&port_type);

      if(reg_tbl != NULL)
      {
         msg->subscr_scope = ifsv_drv_scope_update(reg_tbl->port_reg.port_info.if_name);
      }
      else
      {
         msg->subscr_scope = NCS_IFSV_SUBSCR_EXT;
      }
      /* Changes for INT/EXT scope for subscr_scope overwrite. EXT_INT */
      

      res = ifsv_drv_mds_normal_send(drv_cb->mds_hdl, NCSMDS_SVC_ID_IFDRV, 
                 (NCSCONTEXT)msg, drv_cb->ifnd_dest, NCSMDS_SVC_ID_IFND);      

      ncshm_give_hdl(drv_hdl);
      m_NCS_CONS_PRINTF("\nSend failed from Drv to IDIM\n");
   } else
   {
      m_NCS_CONS_PRINTF("\nSend success from Drv to IDIM\n");
      res = NCSCC_RC_FAILURE;
   }
   m_MMGR_FREE_IFSV_DRV_IDIM_MSG(msg);

   return (res);
}


/****************************************************************************
 * Name          : ifsv_drv_get_stats_evt
 *
 * Description   : This is the event which driver got from the IfND to fetch
 *                 the statistics of the particular port.
 *
 * Arguments     : evt - event pointer, that has been received.
 *                 drv_cb        - driver Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifsv_drv_get_stats_evt (NCS_IFSV_HW_DRV_REQ *evt, IFSV_DRV_CB *drv_cb)
{
   IFSV_DRV_PORT_REG_TBL     *reg_tbl;
   NCS_IFSV_PORT_TYPE        port_type;
   NCS_IFSV_PORT_STATS       port_stats;

   port_type.port_id = evt->port_type.port_id;
   port_type.type    = evt->port_type.type;

   m_NCS_CONS_PRINTF("\nDrv receive get stats."
                     "port_type.port_id =  %d and port_type.type = %d\n",
                     port_type.port_id, port_type.type);

   m_NCS_LOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);
   reg_tbl = (IFSV_DRV_PORT_REG_TBL*)
      ncs_db_link_list_find(&drv_cb->port_reg_tbl, (uns8*)&port_type);
   if (reg_tbl == NULL)
   {
      m_NCS_MEMSET(&port_stats, 0, sizeof(NCS_IFSV_PORT_STATS));      
      /* send a failure message to IfND(IDIM) */
      ifsv_drv_mds_msg_send ((NCSCONTEXT)&port_stats, 
         NCS_IFSV_HW_DRV_STATS, &port_type, drv_cb);
   } else
   {
      evt->drv_data = reg_tbl->port_reg.drv_data;
      /* call the callback registered by the driver */
      if (reg_tbl->port_reg.hw_get_set_cb != NULL)
      {
         /* this call should be calling "ncs_ifsv_drv_svc_req()" 
          * function to send the information to the IDIM again 
          */
         /**** The below call prototype should be checked and changed TBD ****/
         reg_tbl->port_reg.hw_get_set_cb(evt);
      } else
      {
         /* OOPS driver dosen't registered the callback -LOG the error*/
      }
   }
   m_NCS_UNLOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);

   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifsv_drv_set_hw_param_evt
 *
 * Description   : This is the event which driver got from the IfND to set
 *                 the interface parameters.
 *
 * Arguments     : evt - event pointer, that has been received.
 *                 drv_cb        - driver Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifsv_drv_set_hw_param_evt (NCS_IFSV_HW_DRV_REQ *evt, IFSV_DRV_CB *drv_cb)
{
   IFSV_DRV_PORT_REG_TBL     *reg_tbl;
   NCS_IFSV_PORT_TYPE        port_type;

   port_type.port_id = evt->port_type.port_id;
   port_type.type    = evt->port_type.type;

   m_NCS_CONS_PRINTF("\nDrv receive set stats." 
                     "port_type.port_id = %d and port_type.type = %d\n",
                     port_type.port_id, port_type.type);

   m_NCS_LOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);
   reg_tbl = (IFSV_DRV_PORT_REG_TBL*)
      ncs_db_link_list_find(&drv_cb->port_reg_tbl, (uns8*)&port_type);
   if (reg_tbl != NULL)
   {      
      evt->drv_data = reg_tbl->port_reg.drv_data;
      /* call the callback registered by the driver */
      if (reg_tbl->port_reg.hw_get_set_cb != NULL)
      {
        /* this call should be calling "ncs_ifsv_drv_svc_req()" 
         * function to send the information to the IDIM again 
         */
         /**** The below call prototype should be checked and changed TBD ****/
         reg_tbl->port_reg.hw_get_set_cb(evt);
      } else
      {
         /* OOPS driver dosen't registered the callback -LOG the error*/
      }
   }
   m_NCS_UNLOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);

   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifsv_drv_port_sync_up
 *
 * Description   : This is the event which driver got from the IfND to syncup
 *                 port information which it has at present.
 *
 * Arguments     : evt     - event pointer, that has been received.
 *                 drv_cb  - driver Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifsv_drv_port_sync_up (NCS_IFSV_HW_DRV_REQ *evt, IFSV_DRV_CB *drv_cb)
{
   IFSV_DRV_PORT_REG_TBL     *reg_tbl;
   NCS_IFSV_PORT_TYPE        port_type;
   uns32 res = NCSCC_RC_SUCCESS;

   port_type.port_id = evt->port_type.port_id;
   port_type.type    = evt->port_type.type;

   m_NCS_CONS_PRINTF("\nDrv receive port sync req." 
                     "port_type.port_id = %d and port_type.type = %d",
                     port_type.port_id, port_type.type);

   reg_tbl = (IFSV_DRV_PORT_REG_TBL*)
      ncs_db_link_list_find(&drv_cb->port_reg_tbl, (uns8*)&port_type);
   if (reg_tbl != NULL)
   {
      /* send the register message again to the IFND */
      res = ifsv_drv_mds_msg_send ((NCSCONTEXT)&reg_tbl->port_reg.port_info, 
         NCS_IFSV_HW_DRV_PORT_REG, &port_type, drv_cb);
   }
   return (res);
}


/****************************************************************************
 * Name          : ifsv_drv_ifnd_up_evt
 *
 * Description   : This is the event which driver got from the IfND which 
 *                 indicates that IfND is up. If the Node ID and the Vrid 
 *                 matches then it will make the IfND status as UP and sends
 *                 all the port register information to the IfND.
 *
 * Arguments     : evt - event pointer, that has been received.
 *                 drv_cb        - driver Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifsv_drv_ifnd_up_evt (NCS_IFSV_HW_DRV_REQ *evt, IFSV_DRV_CB *drv_cb)
{   
   IFSV_DRV_PORT_REG_TBL *port_reg_tbl;
   uns32                 res = NCSCC_RC_SUCCESS;

   m_NCS_CONS_PRINTF("\nDrv receive IfND UP evt. Node_id = %d",
                   m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.ifnd_info.ifnd_addr));
   
   if (drv_cb->vrid == evt->info.ifnd_info.vrid)      
   {      
      /* store the IfND's Vcard Id and send register message to the IfND */
      drv_cb->ifnd_up   = TRUE;
      drv_cb->ifnd_dest = evt->info.ifnd_info.ifnd_addr;

      port_reg_tbl = (IFSV_DRV_PORT_REG_TBL*)
         m_NCS_DBLIST_FIND_FIRST((&drv_cb->port_reg_tbl));
      if (port_reg_tbl != NULL)
      {
         m_NCS_LOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);
         do
         {      
            if (port_reg_tbl->port_reg.port_info.oper_state == NCS_STATUS_UP)
            {
               res =ifsv_drv_mds_msg_send((NCSCONTEXT)&port_reg_tbl->port_reg, 
                  NCS_IFSV_HW_DRV_PORT_REG, 
                  &port_reg_tbl->port_reg.port_info.port_type, drv_cb);
               if (res == NCSCC_RC_FAILURE)
                  break;
            }
            port_reg_tbl = (IFSV_DRV_PORT_REG_TBL*)
               m_NCS_DBLIST_FIND_NEXT((&port_reg_tbl->q_node));
         } while(port_reg_tbl != NULL);
         m_NCS_UNLOCK(&drv_cb->port_reg_lock, NCS_LOCK_WRITE);
      }
   }   
   return (res);
}

/****************************************************************************
 * Name          : ifsv_drv_mds_adest_get
 *
 * Description   : This function Gets the Handles of local MDS destination
 *
 * Arguments     : cb   : IFND Driver control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifsv_drv_mds_adest_get (IFSV_DRV_CB *cb)
{
   NCSADA_INFO   arg;
   uns32         rc;

   m_NCS_OS_MEMSET(&arg,0,sizeof(NCSADA_INFO));

   arg.req                             = NCSADA_GET_HDLS;

   rc = ncsada_api(&arg);

   if(rc != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("\nmds_adest_get failed\n");
      return rc;
   }

   cb->mds_hdl   = arg.info.adest_get_hdls.o_mds_pwe1_hdl;

   return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_mds_callback
 *
 * Description   : Call back function provided to MDS. MDS will call this
 *                 function for enc/dec/cpy/rcv/svc_evt operations.
 *
 * Arguments     : NCSMDS_CALLBACK_INFO *info: call back info.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifsv_drv_mds_callback (NCSMDS_CALLBACK_INFO *info)
{
   uns32        cb_hdl;   
   uns32    res = NCSCC_RC_SUCCESS;

   if(info == NULL)
      return res;

   cb_hdl = (uns32) info->i_yr_svc_hdl;
   
   switch(info->i_op)
   {
   case MDS_CALLBACK_ENC:
   case MDS_CALLBACK_ENC_FLAT:
      res = ifsv_drv_mds_enc(&info->info.enc,cb_hdl);      
      break;   

   case MDS_CALLBACK_DEC:
   case MDS_CALLBACK_DEC_FLAT:
      res = ifsv_drv_mds_dec(&info->info.dec,cb_hdl);
      break;

   case MDS_CALLBACK_RECEIVE:
      res = ifsv_drv_mds_rcv(&info->info.receive,cb_hdl);
      break;

   case MDS_CALLBACK_COPY:
      res = ifsv_drv_mds_cpy(&info->info.cpy,cb_hdl);
      break;

   case MDS_CALLBACK_SVC_EVENT:
      res = ifsv_drv_svc_evt_info(&info->info.svc_evt,cb_hdl);
      break;

   default:
      /* IFND will not get copy and callback event */      
      break;
   }

   if(res != NCSCC_RC_SUCCESS)
   {
      /* RSR;TBD Log, */
   }
   return res;

}

/****************************************************************************
 * Name          : ifsv_drv_mds_install
 *
 * Description   : This is the function which installs the mds for Interface 
 *                 driver mod.
 *
 * Arguments     : drv_cb        - driver Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifsv_drv_mds_install (IFSV_DRV_CB *drv_cb)
{
   NCSMDS_INFO          arg;   
   MDS_SVC_ID           svc_id;
   uns32                res = NCSCC_RC_SUCCESS;

   /* Install your service into MDS */
   m_NCS_OS_MEMSET(&arg,0,sizeof(NCSMDS_INFO));


   /* Create the virtual Destination for IFD */
   res = ifsv_drv_mds_adest_get(drv_cb);
   if (res != NCSCC_RC_SUCCESS)
      return res;

   arg.i_mds_hdl        = drv_cb->mds_hdl;
   arg.i_svc_id         = NCSMDS_SVC_ID_IFDRV;
   arg.i_op             = MDS_INSTALL;

   arg.info.svc_install.i_yr_svc_hdl      = drv_cb->cb_hdl;
   arg.info.svc_install.i_install_scope     = NCSMDS_SCOPE_INTRANODE;
   arg.info.svc_install.i_svc_cb          = ifsv_drv_mds_callback;
   arg.info.svc_install.i_mds_q_ownership = FALSE;
   arg.info.svc_install.i_mds_svc_pvt_ver = IFSV_DRV_PVT_SUBPART_VERSION;

   res = ncsmds_api(&arg);

   if (res != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("\nmds install failed\n");
      goto drv_install_err;
   }

   /* Store the self destination */
   drv_cb->my_dest = arg.info.svc_install.o_dest;  
   drv_cb->node_id = m_NCS_NODE_ID_FROM_MDS_DEST(drv_cb->my_dest);

   /* IFND is subscribing for IFD MDS service */
   arg.i_op                            = MDS_SUBSCRIBE;
   arg.info.svc_subscribe.i_scope        = NCSMDS_SCOPE_INTRANODE;
   arg.info.svc_subscribe.i_num_svcs   = 1;
   svc_id = NCSMDS_SVC_ID_IFND;
   arg.info.svc_subscribe.i_svc_ids    = &svc_id;

   res = ncsmds_api(&arg);

   if (res == NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("\nmds subscription Succeeded\n");
      return (NCSCC_RC_SUCCESS);      
   }

drv_install_err:
      /* Uninstall the service */
   arg.i_op   = MDS_UNINSTALL;
   ncsmds_api(&arg);
   return (res);
}


/****************************************************************************
 * Name          : ifsv_drv_mds_enc
 *
 * Description   : This is the function used to encode all message going out.
 *
 * Arguments     : enc        - encoded info.
 *                 drv_hdl    - driver handle. 
 *
 * Return Values : (NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE)
 *
 * Notes         : None
 *****************************************************************************/
static uns32 
ifsv_drv_mds_enc (MDS_CALLBACK_ENC_INFO *enc_info, uns32 drv_hdl)
{   
   EDU_ERR   ederror = 0;
   IFSV_DRV_CB *drv_cb;   
   uns32     rc = NCSCC_RC_FAILURE;

   enc_info->o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(enc_info->i_rem_svc_pvt_ver,
                               DRV_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT,
                               DRV_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT,
                               DRV_WRT_IFND_MSG_FMT_ARRAY);
   if(0 == enc_info->o_msg_fmt_ver)
   {
   m_NCS_CONS_PRINTF("\n rem ver not supported:%d",enc_info->i_rem_svc_pvt_ver);
   m_NCS_CONS_PRINTF("message not encoded\n");
   return NCSCC_RC_FAILURE;
   } 
   if ((drv_cb = (IFSV_DRV_CB*) ncshm_take_hdl(NCS_SERVICE_ID_IFDRV, 
      (uns32)drv_hdl)) != NULL)
   {
      /* embedding subslot changes for backward compatibility*/
      rc = m_NCS_EDU_VER_EXEC(&drv_cb->edu_hdl, ifsv_drv_edp_idim_hw_rcv_info, enc_info->io_uba, 
         EDP_OP_TYPE_ENC, (NCS_IFSV_HW_INFO*)enc_info->i_msg, &ederror, enc_info->o_msg_fmt_ver);
      ncshm_give_hdl(drv_hdl);
      if(rc != NCSCC_RC_SUCCESS)
      {
         /* Free calls to be added here. */
         m_NCS_CONS_PRINTF("\nmds enc failed\n");
         m_NCS_EDU_PRINT_ERROR_STRING(ederror);
         return rc;
      }
   }
   else
   {
      m_NCS_CONS_PRINTF("\nmds enc failed\n");
   }
   return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_mds_dec
 *
 * Description   : This is the function used to decode all message going out.
 *
 * Arguments     : dec        - decoded info.
 *                 drv_hdl    - driver handle. 
 *
 * Return Values : (NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE)
 *
 * Notes         : None
 *****************************************************************************/
static uns32 
ifsv_drv_mds_dec (MDS_CALLBACK_DEC_INFO *dec_info, uns32 drv_hdl)
{   
   EDU_ERR   ederror = 0;
   IFSV_DRV_CB *drv_cb;   
   uns32     rc = NCSCC_RC_FAILURE;

   if( 0 == m_NCS_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                                      DRV_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT,
                                      DRV_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT,
                                      DRV_WRT_IFND_MSG_FMT_ARRAY))
   {
    m_NCS_CONS_PRINTF("\nmsg fmt ver:%d not supported",dec_info->i_msg_fmt_ver);
    m_NCS_CONS_PRINTF("msg not decoded\n");
    return NCSCC_RC_FAILURE;
   }

   if ((drv_cb = (IFSV_DRV_CB*) ncshm_take_hdl(NCS_SERVICE_ID_IFDRV, 
      (uns32)drv_hdl)) != NULL)
   {
      /* decode the message received from the driver */
      dec_info->o_msg = m_MMGR_ALLOC_IFSV_DRV_REQ_MSG;
      if(dec_info->o_msg == NULL)
      {
         m_NCS_CONS_PRINTF("\nMemory failure\n");
         ncshm_give_hdl(drv_hdl);
         return(NCSCC_RC_FAILURE);
      }

      /* embedding subslot changes for backward compatibility*/
      rc = m_NCS_EDU_VER_EXEC(&drv_cb->edu_hdl, ifsv_drv_edp_idim_hw_req_info, dec_info->io_uba, 
         EDP_OP_TYPE_DEC, (NCS_IFSV_HW_DRV_REQ**)&dec_info->o_msg, &ederror, dec_info->i_msg_fmt_ver);
      ncshm_give_hdl(drv_hdl);
      if(rc != NCSCC_RC_SUCCESS)
      {         
         m_MMGR_FREE_IFSV_DRV_REQ_MSG(dec_info->o_msg);
         m_NCS_EDU_PRINT_ERROR_STRING(ederror);
         m_NCS_CONS_PRINTF("\nmds dec failed\n");
         return rc;
      }
   }
   else
   {
      m_NCS_CONS_PRINTF("\nmds dec failed\n");
   }
   return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_mds_cpy
 *
 * Description   : This is the function used to copy all message going out.
 *
 * Arguments     : cpy        - copy info.
 *                 drv_hdl    - driver handle. 
 *
 * Return Values : (NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE)
 *
 * Notes         : we assume that IDIM/IfND should not lie on the same 
 *                 process, so hitting this function is not possible, any way
 *                 for the compatibility we have written this callback 
 *                 function.
 *****************************************************************************/
static uns32 
ifsv_drv_mds_cpy (MDS_CALLBACK_COPY_INFO *cpy, uns32 drv_hdl)
{ 
   uns8*  stream;
   stream = (uns8*)m_MMGR_ALLOC_IFSV_DRV_IDIM_MSG;

   /* cpy->o_msg_fmt_ver is not filled as we assume that driver and ifnd-idim will not lie in the same process and so this function is not called*/ 
   if(stream == NULL)
   {
      m_NCS_CONS_PRINTF("\nMemory failure\n");
      return NCSCC_RC_FAILURE;
   } 

   USE (cpy->i_msg);
   
   m_NCS_OS_STRCPY((char*)stream,(char*)cpy->i_msg);
   cpy->o_cpy = (void*)stream;
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : ifsv_drv_mds_rcv
 *
 * Description   : This is the function used to receive messages.
 *
 * Arguments     : rcv_info   - Receive info.
 *                 drv_hdl    - driver handle. 
 *
 * Return Values : (NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE)
 *
 * Notes         : None
 *****************************************************************************/
static uns32 
ifsv_drv_mds_rcv (MDS_CALLBACK_RECEIVE_INFO *rcv_info, uns32 drv_hdl)
{   
   NCS_IFSV_HW_DRV_REQ *pMsg = (NCS_IFSV_HW_DRV_REQ*)rcv_info->i_msg;
   IFSV_DRV_CB *drv_cb;
   uns32 res = NCSCC_RC_SUCCESS;
   
   if ((drv_cb = (IFSV_DRV_CB*) ncshm_take_hdl(NCS_SERVICE_ID_IFDRV, 
      (uns32)drv_hdl)) != NULL)
   {
      /* service all the events here it self */
      switch(pMsg->req_type)
      {
      case NCS_IFSV_HW_DRV_IFND_UP:
         res = ifsv_drv_ifnd_up_evt(pMsg, drv_cb);
         break;
      case NCS_IFSV_HW_DRV_STATS:
         res = ifsv_drv_get_stats_evt(pMsg, drv_cb);
         break;
      case NCS_IFSV_HW_DRV_SET_PARAM:
         res = ifsv_drv_set_hw_param_evt(pMsg, drv_cb);
         break;
      case NCS_IFSV_HW_DRV_PORT_SYNC_UP:
         res = ifsv_drv_port_sync_up(pMsg, drv_cb);
         break;
      default:         
         break;
      }
      m_MMGR_FREE_IFSV_DRV_REQ_MSG(pMsg);
      ncshm_give_hdl(drv_hdl);
   }   
   return (res);
}

/****************************************************************************
 * Name          : ifsv_drv_svc_evt_info
 *
 * Description   : This is the function used to copy all message going out.
 *
 * Arguments     : dec        - decoded info.
 *                 drv_hdl    - driver handle. 
 *
 * Return Values : (NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE)
 *
 * Notes         : None
 *****************************************************************************/
static uns32 
ifsv_drv_svc_evt_info (MDS_CALLBACK_SVC_EVENT_INFO *evt_info, uns32 drv_hdl)
{   
   IFSV_DRV_CB *drv_cb;

   switch (evt_info->i_change) /* Review change type */
   {
   case NCSMDS_DOWN:
      
      if ((drv_cb = (IFSV_DRV_CB*) ncshm_take_hdl(NCS_SERVICE_ID_IFDRV, 
         (uns32)drv_hdl)) != NULL)
      {
         if (evt_info->i_svc_id == NCSMDS_SVC_ID_IFND)
         {
            if ((drv_cb->ifnd_up == TRUE) &&
             (m_NCS_IFSV_IS_MDS_DEST_SAME(&evt_info->i_dest,&drv_cb->ifnd_dest)
               == TRUE))
            {
               drv_cb->ifnd_up = FALSE;
            }
         }         
         ncshm_give_hdl(drv_hdl);
      }
      
      break;
   default:
      break;      
   }
   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifsv_drv_mds_normal_send
 *
 * Description   : This is the function which is used to send the message 
 *                 using MDS.
 *
 * Arguments     : mds_hdl  - MDS handle 
 *                 from_svc - From Serivce ID.
 *                 evt      - Event to be sent.
 *                 dest     - MDS destination to be sent to.
 *                 to_svc   - To Service ID.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifsv_drv_mds_normal_send (MDS_HDL mds_hdl, 
                          NCSMDS_SVC_ID from_svc, 
                          NCSCONTEXT evt, MDS_DEST dest, 
                          NCSMDS_SVC_ID to_svc)
{
   
   NCSMDS_INFO   info;
   uns32         res;
   
   m_NCS_MEMSET(&info, 0, sizeof(info));
   
   info.i_mds_hdl  = mds_hdl;
   info.i_op       = MDS_SEND;
   info.i_svc_id   = from_svc;
   
   info.info.svc_send.i_msg = evt;
   info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
   info.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
   info.info.svc_send.i_to_svc   = to_svc;
   
   m_NCS_MEMSET(&info.info.svc_send.info.snd.i_to_dest, 0, sizeof(MDS_DEST));
   info.info.svc_send.info.snd.i_to_dest = dest;
   
   res = ncsmds_api(&info);
   return(res);
}

/************* MDS decode/ encode *******************/
/****************************************************************************
 * Name          : ifsv_drv_hw_resp_evt_test_type_fnc
 *
 * Description   : This is the function which is used to test the event type
 *                 received from the hardware driver.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
int ifsv_drv_hw_resp_evt_test_type_fnc(NCSCONTEXT arg)
{
   typedef enum 
   {
       LCL_TEST_JUMP_OFFSET_STATS = 1, 
       LCL_TEST_JUMP_OFFSET_PORT_REG,
       LCL_TEST_JUMP_OFFSET_PORT_STATUS
   }LCL_TEST_JUMP_OFFSET;
   NCS_IFSV_HW_DRV_MSG_TYPE    evt;
   
   if(arg == NULL)
      return EDU_FAIL;
   
   evt = *(NCS_IFSV_HW_DRV_MSG_TYPE *)arg;
   switch(evt)
   {
   case NCS_IFSV_HW_DRV_STATS:
      return LCL_TEST_JUMP_OFFSET_STATS;
   case NCS_IFSV_HW_DRV_PORT_REG:
      return LCL_TEST_JUMP_OFFSET_PORT_REG;
   case NCS_IFSV_HW_DRV_PORT_STATUS:
      return LCL_TEST_JUMP_OFFSET_PORT_STATUS;
   default:
      return EDU_EXIT;
   }
   return EDU_FAIL;
}

/****************************************************************************
 * Name          : ifsv_drv_hw_req_evt_test_type_fnc
 *
 * Description   : This is the function which is used to test the event type
 *                 which IDIM request the hardware.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
int ifsv_drv_hw_req_evt_test_type_fnc(NCSCONTEXT arg)
{
   typedef enum 
   {
       LCL_TEST_JUMP_OFFSET_STATS = 1, 
       LCL_TEST_JUMP_OFFSET_IFND_UP,
       LCL_TEST_JUMP_OFFSET_SET_PARAM
   }LCL_TEST_JUMP_OFFSET;
   NCS_IFSV_HW_DRV_MSG_TYPE    evt;
   
   if(arg == NULL)
      return EDU_FAIL;
   
   evt = *(NCS_IFSV_HW_DRV_MSG_TYPE *)arg;
   switch(evt)
   {
   case NCS_IFSV_HW_DRV_STATS:
      return LCL_TEST_JUMP_OFFSET_STATS;
   case NCS_IFSV_HW_DRV_SET_PARAM:
      return LCL_TEST_JUMP_OFFSET_SET_PARAM;
   case NCS_IFSV_HW_DRV_IFND_UP:
      return LCL_TEST_JUMP_OFFSET_IFND_UP;
   default:
      return EDU_EXIT;
   }
   return EDU_FAIL;
}

/****************************************************************************
 * Name          : ifsv_drv_edp_intf_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IFSV_INTF_INFO structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_drv_edp_intf_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                         NCSCONTEXT ptr, uns32 *ptr_data_len, 
                         EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                         EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_INTF_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_intf_rules[ ] = {
        {EDU_START, ifsv_drv_edp_intf_info, 0, 0, 0, sizeof(NCS_IFSV_INTF_INFO), 0, NULL},        

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->if_am, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->if_descr, IFSV_IF_DESC_SIZE, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->if_name, IFSV_IF_NAME_SIZE, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->mtu, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->if_speed, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->phy_addr, 6, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->admin_state, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->oper_state, 0, NULL},
        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->last_change, 0, NULL},
        
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_INTF_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_INTF_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_INTF_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_intf_rules, struct_ptr, ptr_data_len, 
        buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_edp_idim_ifnd_up_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IFND_UP_INFO structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_drv_edp_idim_ifnd_up_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                 NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                 EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                 EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFND_UP_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_hw_ifnd_up_rules[ ] = {
        {EDU_START, ifsv_drv_edp_idim_ifnd_up_info, 0, 0, 0, sizeof(NCS_IFND_UP_INFO), 0, NULL},                
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFND_UP_INFO*)0)->vrid, 0, NULL},         
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFND_UP_INFO*)0)->nodeid, 0, NULL},                 
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_IFND_UP_INFO*)0)->ifnd_addr, 0, NULL},                
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFND_UP_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFND_UP_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFND_UP_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_hw_ifnd_up_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_edp_idim_hw_req
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IFSV_HW_REQ_INFO structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_drv_edp_idim_hw_req(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_HW_REQ_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_hw_ifnd_up_rules[ ] = {
        {EDU_START, ifsv_drv_edp_idim_ifnd_up_info, 0, 0, 0, sizeof(NCS_IFSV_HW_REQ_INFO), 0, NULL},                
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_REQ_INFO*)0)->svc_id, 0, NULL},                         
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_IFSV_HW_REQ_INFO*)0)->dest_addr, 0, NULL},         
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_REQ_INFO*)0)->app_hdl, 0, NULL},         
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_HW_REQ_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_HW_REQ_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_HW_REQ_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_hw_ifnd_up_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_drv_edp_idim_port_type
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IFSV_PORT_TYPE structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_drv_edp_idim_port_type(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                              NCSCONTEXT ptr, uns32 *ptr_data_len, 
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                              EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_PORT_TYPE   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_port_type_rules[ ] = {
        {EDU_START, ifsv_drv_edp_idim_port_type, 0, 0, 0, sizeof(NCS_IFSV_PORT_TYPE), 0, NULL},        

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_TYPE*)0)->port_id, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_TYPE*)0)->type, 0, NULL},        
        
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_PORT_TYPE *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_PORT_TYPE **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_PORT_TYPE));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_port_type_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_edp_idim_hw_req_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IFSV_HW_DRV_REQ structure, which the IDIM sends the request
 *                 to the hardware drv.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_drv_edp_idim_hw_req_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_HW_DRV_REQ   *struct_ptr = NULL, **d_ptr = NULL;        

    EDU_INST_SET    ifsv_idim_hw_req_rules[ ] = {
        {EDU_START, ifsv_drv_edp_idim_hw_req_info, 0, 0, 0, sizeof(NCS_IFSV_HW_DRV_REQ), 0, NULL},                
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_DRV_REQ*)0)->req_type, 0, NULL},         
   {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_DRV_REQ*)0)->subscr_scope, 0, NULL}, /*EXT_INT*/ 
        {EDU_EXEC, ifsv_drv_edp_idim_port_type, 0, 0, 0, (long)&((NCS_IFSV_HW_DRV_REQ*)0)->port_type, 0, NULL},         
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_DRV_REQ*)0)->drv_data, 0, NULL},         

        {EDU_TEST, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_DRV_REQ*)0)->req_type, 0, ifsv_drv_hw_req_evt_test_type_fnc},

        /* For NCS_IFSV_HW_DRV_STATS */
        {EDU_EXEC, ifsv_drv_edp_idim_hw_req, 0, 0, EDU_EXIT, 
            (long)&((NCS_IFSV_HW_DRV_REQ*)0)->info.req_info, 0, NULL},        

      /* For NCS_IFSV_HW_DRV_IFND_UP */
        {EDU_EXEC, ifsv_drv_edp_idim_ifnd_up_info, 0, 0, EDU_EXIT, 
            (long)&((NCS_IFSV_HW_DRV_REQ*)0)->info.ifnd_info, 0, NULL},        

            /* For NCS_IFSV_HW_DRV_SET_PARAM */
        {EDU_EXEC, ifsv_drv_edp_intf_info, 0, 0, EDU_EXIT, 
            (long)&((NCS_IFSV_HW_DRV_REQ*)0)->info.set_param, 0, NULL},            
              
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_HW_DRV_REQ *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_HW_DRV_REQ **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_HW_DRV_REQ));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_idim_hw_req_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_edp_idim_port_status
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IFSV_PORT_STATUS structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_drv_edp_idim_port_status(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_PORT_STATUS   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_port_status_rules[ ] = {
        {EDU_START, ifsv_drv_edp_idim_port_status, 0, 0, 0, sizeof(NCS_IFSV_PORT_STATUS), 0, NULL},                
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATUS*)0)->oper_state, 0, NULL},         
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_PORT_STATUS *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_PORT_STATUS **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_PORT_STATUS));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_port_status_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_edp_idim_port_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IFSV_PORT_INFO structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_drv_edp_idim_port_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                              NCSCONTEXT ptr, uns32 *ptr_data_len, 
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                              EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_PORT_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_port_info_rules[ ] = {
        {EDU_START, ifsv_drv_edp_idim_port_info, 0, 0, 0, sizeof(NCS_IFSV_PORT_INFO), 0, NULL},        

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->if_am, 0, NULL},
        {EDU_EXEC, ifsv_drv_edp_idim_port_type, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->port_type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->phy_addr, 6, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->oper_state, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->admin_state, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->mtu, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->if_name, IFSV_IF_NAME_SIZE, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->speed, 0, NULL},                
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->dest, 0, NULL},
        
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_PORT_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_PORT_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_PORT_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_port_info_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_drv_edp_stats_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IFSV_INTF_STATS structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_drv_edp_stats_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                          NCSCONTEXT ptr, uns32 *ptr_data_len, 
                          EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                          EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_INTF_STATS   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_stats_info_rules[ ] = {
        {EDU_START, ifsv_drv_edp_stats_info, 0, 0, 0, sizeof(NCS_IFSV_INTF_STATS), 0, NULL},        
        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->last_chg, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_octs, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_upkts, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_nupkts, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_dscrds, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_errs, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_unknown_prots, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_octs, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_upkts, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_nupkts, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_dscrds, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_errs, 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_qlen, 0, NULL},        
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_INTF_STATS *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_INTF_STATS **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_INTF_STATS));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_stats_info_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_edp_idim_port_stats
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IFSV_PORT_STATS structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_drv_edp_idim_port_stats(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_PORT_STATS   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_port_stats_rules[ ] = {
        {EDU_START, ifsv_drv_edp_idim_port_stats, 0, 0, 0, sizeof(NCS_IFSV_PORT_STATS), 0, NULL},                
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATS*)0)->status, 0, NULL},         
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATS*)0)->app_svc_id, 0, NULL},                 
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATS*)0)->app_dest, 0, NULL},
        {EDU_EXEC, ifsv_drv_edp_stats_info, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATS*)0)->stats, 0, NULL},         
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATS*)0)->usr_hdl, 0, NULL},         
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_PORT_STATS *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_PORT_STATS **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_PORT_STATS));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_port_stats_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_drv_edp_idim_hw_rcv_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IFSV_HW_INFO structure, which the IDIM receives.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_drv_edp_idim_hw_rcv_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_HW_INFO   *struct_ptr = NULL, **d_ptr = NULL;        

    EDU_INST_SET    ifsv_idim_hw_info_rules[ ] = {
        {EDU_START, ifsv_drv_edp_idim_hw_rcv_info, 0, 0, 0, sizeof(NCS_IFSV_HW_INFO), 0, NULL},                
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_INFO*)0)->msg_type, 0, NULL},         
   {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_INFO*)0)->subscr_scope, 0, NULL},/*EXT_INT */
        {EDU_EXEC, ifsv_drv_edp_idim_port_type, 0, 0, 0, (long)&((NCS_IFSV_HW_INFO*)0)->port_type, 0, NULL},        
        
        {EDU_TEST, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_INFO*)0)->msg_type, 0, ifsv_drv_hw_resp_evt_test_type_fnc},

        /* For NCS_IFSV_HW_DRV_STATS */
        {EDU_EXEC, ifsv_drv_edp_idim_port_stats, 0, 0, EDU_EXIT, 
            (long)&((NCS_IFSV_HW_INFO*)0)->info.stats, 0, NULL},        

      /* For NCS_IFSV_HW_DRV_PORT_REG */
        {EDU_EXEC, ifsv_drv_edp_idim_port_info, 0, 0, EDU_EXIT, 
            (long)&((NCS_IFSV_HW_INFO*)0)->info.reg_port, 0, NULL},        

            /* For NCS_IFSV_HW_DRV_PORT_STATUS */
        {EDU_EXEC, ifsv_drv_edp_idim_port_status, 0, 0, EDU_EXIT, 
            (long)&((NCS_IFSV_HW_INFO*)0)->info.port_status, 0, NULL},            
              
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_HW_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_HW_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_HW_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_idim_hw_info_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_drv_scope_update
 *
 * Description   : This is the function which is used to pick the Subscription
 *                 Scope from Configuration file in SYSCONFDIR/ncs_ifsv_drv_conf.
 *
 * Return Value  : Returns  NCS_IFSV_SUBSCR_EXT/NCS_IFSV_SUBSCR_INT
 *
 * Notes         : None.
 *****************************************************************************/

NCS_IFSV_SUBSCR_SCOPE  ifsv_drv_scope_update(uns8 *name)
{
   FILE *fp;
   uns8 pres_word[10]={'\0'};
   uns32 rc = NCSCC_RC_SUCCESS; 

   fp = fopen(SYSCONFDIR "ncs_ifsv_drv_conf","r");
   if (fp == NULL)
   {
      m_NCS_CONS_PRINTF("\n " SYSCONFDIR "ncs_ifsv_drv_conf file failed to open!!DoesntExist ..\n");
      return 0;
   }

   while(1)
   {
       if((rc = get_word_from_file(&fp,pres_word)) == 0 )
       {
           if(strcmp(pres_word,name)==0)
           {
               rc = get_word_from_file(&fp,pres_word);
               if( rc == 0 )
               {
                 if(strcmp("INTERNAL", pres_word) == 0)
                   return NCS_IFSV_SUBSCR_INT;
                 else
                   return NCS_IFSV_SUBSCR_EXT;
               }
               else if (rc == NCS_MAIN_ENTER_CHAR )
               {
                 if(strcmp("INTERNAL", pres_word) == 0)
                   return NCS_IFSV_SUBSCR_INT;
               }
               else if (rc == NCS_MAIN_EOF)
                 return NCS_IFSV_SUBSCR_EXT;
           }
       }
       else if (rc == NCS_MAIN_EOF)
          return NCS_IFSV_SUBSCR_EXT;
   }
  fclose (fp);

}
/****************************************************************************
 * Name          : get_word_from_file
 *
 * Description   : This is a utility function which is used to
 *                 extract word from file.
 *
 * Return Val    : NCS_MAIN_ENTER_CHAR/ NCS_MAIN_EOF/ 0
 *
 * Notes         : None.
 *****************************************************************************/

static
uns32 get_word_from_file(FILE **fp, char *o_chword)
{
   int temp_char;
   unsigned int temp_ctr=0;
try_again:
   temp_ctr = 0;

   m_NCS_MEMSET(o_chword, 0, 10);
   while(( temp_char = (char)getc(*fp) ) == '#')
      go_to_next_line(fp);
   while ((temp_char != EOF) && (temp_char != '\n') && (temp_char != ' ') && (temp_char != '\0') && (temp_char != '\t') )
   {
      o_chword[temp_ctr] = (char)temp_char;
      temp_char = getc(*fp);
      temp_ctr++;
   }
   o_chword[temp_ctr] = '\0';
   if (temp_char == EOF)
   {
      return(NCS_MAIN_EOF);
   }
   if (temp_char == '\n')
   {
      return(NCS_MAIN_ENTER_CHAR);
   }
   if(o_chword[0] == 0x0) 
   {
      goto try_again;
   }
   return(0);
}


/****************************************************************************
 * Name          : go_to_next_line
 *
 * Description   : This is a utility function which is used to
 *                 jump to next line in the file.
 *
 * Return Value  : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 * Notes         : None.
 *****************************************************************************/

static uns32
go_to_next_line(FILE **fp)
{
   int temp_char;
   if (*fp == NULL)
   {
      return NCSCC_RC_FAILURE;
   }
   temp_char = (char)getc(*fp);
   while((temp_char != EOF) && (temp_char != '\n'))
   {
      temp_char = getc(*fp);
   }
  return NCSCC_RC_SUCCESS;
}

