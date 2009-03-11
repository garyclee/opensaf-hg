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


#include "ifsv_app.h"

uns32 gifsv_dt_total_test_app_created = 0;
static void
ifsv_dt_test_app_process(NCSCONTEXT info);
static int8 * 
intf_dt_phase_token( int8 *s, const int8 *delim);
extern uns32 ifsv_dt_extract_phy_addr (int8 *arg, uns8 pPhyMAC[6]);
static void print_if_attr(NCS_IFSV_INTF_INFO *if_info);

extern uns32 ncs_ifsv_red_demo_flag;

/****************************************************************************
 * Name          : print_if_attr
 *
 * Description   : This is the function which is used to print the interface 
 *                 attributes.
 *
 * Arguments     : if_info  - Pointer to the interface structure. 
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void print_if_attr(NCS_IFSV_INTF_INFO *if_info)
{
   if((if_info->if_am & NCS_IFSV_IAM_MTU) != 0)
   {
      m_NCS_CONS_PRINTF("MTU - %d \n ", if_info->mtu);
   }
   
   if((if_info->if_am & NCS_IFSV_IAM_IFSPEED) != 0)
   {
      m_NCS_CONS_PRINTF("IF SPEED - %d \n ", if_info->if_speed);
   }
   
   if((if_info->if_am & NCS_IFSV_IAM_PHYADDR) != 0)
   {
      m_NCS_CONS_PRINTF("physical Addr: - %x:%x:%x:%x:%x:%x \n", (uns32)if_info->phy_addr[0],(uns32)if_info->phy_addr[1],(uns32)if_info->phy_addr[2],(uns32)if_info->phy_addr[3],(uns32)if_info->phy_addr[4],(uns32)if_info->phy_addr[5]);
   }
   
   if((if_info->if_am & NCS_IFSV_IAM_ADMSTATE) != 0)
   {
      m_NCS_CONS_PRINTF("ADMIN STATUS - %d\n",if_info->admin_state);
   }
   
   if((if_info->if_am & NCS_IFSV_IAM_OPRSTATE) != 0)
   {
      m_NCS_CONS_PRINTF("OPERATION STATUS - %d\n",if_info->oper_state);
   }
   
   if((if_info->if_am & NCS_IFSV_IAM_NAME) != 0)
   {
      m_NCS_CONS_PRINTF("INTERFACE NAME - %s \n", if_info->if_name);
   }
   
   if((if_info->if_am & NCS_IFSV_IAM_DESCR) != 0)
   {
      m_NCS_CONS_PRINTF(" INTF DESC - %s \n",if_info->if_descr);
   }
   
   if((if_info->if_am & NCS_IFSV_IAM_LAST_CHNG) != 0)
   {
      m_NCS_CONS_PRINTF("LAST Change %d\n", (int)if_info->last_change);
   }

   if((if_info->if_am & NCS_IFSV_IAM_CHNG_MASTER) != 0)
   {
      m_NCS_CONS_PRINTF("Master ifindex %d\n", (int)if_info->bind_master_ifindex);
      m_NCS_CONS_PRINTF("slave ifindex %d\n", (int)if_info->bind_slave_ifindex);
    }

}
/****************************************************************************
 * Name          : ifsv_dt_test_app_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task which binds with IfA and prints the 
 *                 response from IfA.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void
ifsv_dt_test_app_process(NCSCONTEXT info)
{
   IFSV_DT_TEST_APP_CB *cb = (IFSV_DT_TEST_APP_CB*)info;
   IFSV_DT_TEST_APP_EVT *evt;
   NCS_IFSV_SVC_RSP     *rsp;
   NCS_IFSV_INTF_REC    *intf;
   NCS_IFSV_INTF_STATS  *stat;
   NCS_IFSV_INTF_INFO   *if_info;
   while((evt = (IFSV_DT_TEST_APP_EVT*)m_NCS_IPC_RECEIVE(&cb->mbx,NULL)) != NULL)
   {
      rsp = &evt->rsp;
 
      switch(evt->evt_type)
      {
         case IFSV_DT_TEST_APP_EVT_ADD_INTF:
            intf = &rsp->info.ifadd_ntfy;
            if_info = &intf->if_info;
            m_NCS_CONS_PRINTF("App%d received Intf Add Info \n", cb->app_no);
            /* subslot changes need modification */
            m_NCS_CONS_PRINTF("Shelf: %d Slot: %d Subslot: %d Port: %d Type %d\n ",  intf->spt_info.shelf, intf->spt_info.slot, intf->spt_info.subslot, intf->spt_info.port, intf->spt_info.type);
            m_NCS_CONS_PRINTF("IFINDEX: %d \n", intf->if_index);
            print_if_attr(&intf->if_info);
            break;
         case IFSV_DT_TEST_APP_EVT_DEL_INTF:
            m_NCS_CONS_PRINTF("App%d received Intf Del for IFINDEX %d\n",
               cb->app_no,rsp->info.ifdel_ntfy);
            break;
         case IFSV_DT_TEST_APP_EVT_MOD_INTF:            
            intf = &rsp->info.ifupd_ntfy;
            if_info = &intf->if_info;
            m_NCS_CONS_PRINTF("App%d received Intf Update Info \n", cb->app_no);
            /* subslot changes not implemented */
            m_NCS_CONS_PRINTF("Shelf: %d Slot: %d Subslot: %d Port: %d Type %d\n ",  intf->spt_info.shelf, intf->spt_info.slot,intf->spt_info.slot, intf->spt_info.port, intf->spt_info.type);
            m_NCS_CONS_PRINTF("IFINDEX: %d \n", intf->if_index);
            print_if_attr(&intf->if_info);
                        break;

         case IFSV_DT_TEST_APP_EVT_INTF_INFO:
            intf = &rsp->info.ifget_rsp.if_rec;
            if_info = &intf->if_info;
            m_NCS_CONS_PRINTF("App%d received Intf Resp \n", cb->app_no);
            /* subslot changes not implemented */
            m_NCS_CONS_PRINTF("Shelf: %d Slot: %d Subslot: %d Port: %d Type %d\n ",  intf->spt_info.shelf, intf->spt_info.slot, intf->spt_info.subslot, intf->spt_info.port, intf->spt_info.type);
            m_NCS_CONS_PRINTF("IFINDEX: %d \n", intf->if_index);
            if(rsp->info.ifget_rsp.error == NCSCC_RC_SUCCESS)
               print_if_attr(&intf->if_info);
            else
               m_NCS_CONS_PRINTF("ERROR - Error Code %d", rsp->info.ifget_rsp.error);

            break;
         case IFSV_DT_TEST_APP_BIND_GET_LOCAL_INTF:
            intf = &rsp->info.ifget_rsp.if_rec;
            if_info = &intf->if_info;
            m_NCS_CONS_PRINTF("App%d received Intf Resp \n", cb->app_no);
            /* subslot changes not implemented */
            m_NCS_CONS_PRINTF("Shelf: %d Slot: %d Subslot: %d Port: %d Type %d\n ",  intf->spt_info.shelf, intf->spt_info.slot, intf->spt_info.subslot, intf->spt_info.port, intf->spt_info.type);
            m_NCS_CONS_PRINTF("IFINDEX: %d \n", intf->if_index);
            if(rsp->info.ifget_rsp.error == NCSCC_RC_SUCCESS)
               m_NCS_CONS_PRINTF("INTERFACE INDEX on local shelf/slot is %d\n",if_info->bind_master_ifindex);
            else
               m_NCS_CONS_PRINTF("ERROR - Error Code %d", rsp->info.ifget_rsp.error);

            break;
         case IFSV_DT_TEST_APP_EVT_STAT_INFO:
            intf = &rsp->info.ifget_rsp.if_rec;
            stat = &intf->if_stats;
            m_NCS_CONS_PRINTF("App%d Received Statistics Get Resp \n", cb->app_no);
                        /* subslot changes not implemented */
            m_NCS_CONS_PRINTF("Shelf: %d Slot: %d Subslot: %d Port: %d Type %d\n ",  intf->spt_info.shelf, intf->spt_info.slot, intf->spt_info.subslot, intf->spt_info.port, intf->spt_info.type);
            m_NCS_CONS_PRINTF("IFINDEX: %d \n", intf->if_index);
            if(rsp->info.ifget_rsp.error == NCSCC_RC_SUCCESS)
               m_NCS_CONS_PRINTF("App%d last_chg - %d in_octs - %d in_upkts - %d in_dscrds - %d in_errs - %d \n",stat->last_chg,stat->in_octs,stat->in_upkts,stat->in_nupkts,stat->in_dscrds,stat->in_errs);
            else
               m_NCS_CONS_PRINTF("ERROR - Error Code %d", rsp->info.ifget_rsp.error);
            break;
         default:
            m_NCS_CONS_PRINTF("Test app - %d received unknown event - %d\n",cb->app_no,evt->evt_type);
            break;
      }
   }
   m_NCS_CONS_PRINTF("sorry App%d is quiting bc of mail box problem pls destroy it\n",cb->app_no);
}

/****************************************************************************
 * Name          : ifsv_dt_test_ifa_sub_cb
 *
 * Description   : This is the callback function which is is registered with
 *                 IfA.
 *
 * Arguments     : rsp  - This is the IfA response structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifsv_dt_test_ifa_sub_cb(NCS_IFSV_SVC_RSP *rsp)
{
   IFSV_DT_TEST_APP_CB *app_cb = (IFSV_DT_TEST_APP_CB*)(long)rsp->usrhdl;
   IFSV_DT_TEST_APP_EVT *evt;
   evt = (IFSV_DT_TEST_APP_EVT*)malloc(sizeof(IFSV_DT_TEST_APP_EVT));   
   memset(evt,0,sizeof(IFSV_DT_TEST_APP_EVT));

   switch(rsp->rsp_type)
   {
   case NCS_IFSV_IFREC_ADD_NTFY:
      evt->evt_type = IFSV_DT_TEST_APP_EVT_ADD_INTF;
      break;
   case NCS_IFSV_IFREC_DEL_NTFY:
      evt->evt_type = IFSV_DT_TEST_APP_EVT_DEL_INTF;
      break;
   case NCS_IFSV_IFREC_UPD_NTFY:
      evt->evt_type = IFSV_DT_TEST_APP_EVT_MOD_INTF;
      break;
   case NCS_IFSV_IFREC_GET_RSP:            
      if (rsp->info.ifget_rsp.if_rec.info_type == NCS_IFSV_IF_INFO)
      {
         evt->evt_type = IFSV_DT_TEST_APP_EVT_INTF_INFO;
      } 
      else if (rsp->info.ifget_rsp.if_rec.info_type == NCS_IFSV_BIND_GET_LOCAL_INTF)
      {
         evt->evt_type = IFSV_DT_TEST_APP_BIND_GET_LOCAL_INTF;
      }
      else if (rsp->info.ifget_rsp.if_rec.info_type == NCS_IFSV_IFSTATS_INFO)
      {
         evt->evt_type = IFSV_DT_TEST_APP_EVT_STAT_INFO;
      } else if (rsp->info.ifget_rsp.if_rec.info_type == (NCS_IFSV_IF_INFO | NCS_IFSV_IFSTATS_INFO))
      {
         evt->evt_type = IFSV_DT_TEST_APP_EVT_INFO_ALL;
      }      
      break;
   default:
      m_NCS_CONS_PRINTF("App%d received wrong event type %d\n",app_cb->app_no,rsp->rsp_type);
      free(evt);
      return(NCSCC_RC_FAILURE);
   }

   /* Cpy the response */
   evt->rsp = *rsp;
   
   /*send an event to the IfSv Thread */
   m_NCS_IPC_SEND(&app_cb->mbx,evt,NCS_IPC_PRIORITY_NORMAL);

   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : IfsvDtTestAppCreate
 *
 * Description   : This is the function used to create test application. 
 *                 IfA.
 *
 * Arguments     : rsp  - This is the IfA response structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppCreate(uns32 shelf, uns32 slot, uns32 subslot)
{
   uns32 test_app_index = 0;
   IFSV_DT_TEST_APP_CB      *app_cb;   
   char  task_name[24];
   char  tmp_char[10];
   uns32 res;
   
   if (gifsv_dt_total_test_app_created != IFSV_DT_MAX_TEST_APP)
   {
      /* check for the free application CB index */
      while(gifsv_dt_test_app[test_app_index].inited == TRUE)
         test_app_index++;
      app_cb = &gifsv_dt_test_app[test_app_index];

      app_cb->app_no = test_app_index;
      gifsv_dt_test_app[test_app_index].inited = TRUE;
      gifsv_dt_test_app[test_app_index].shelf_no = shelf;
      gifsv_dt_test_app[test_app_index].slot_no  = slot;
      /* embedding subslot changes */
      gifsv_dt_test_app[test_app_index].subslot_no  = subslot;
      /* Create a Mail Box */
      if ((res = m_NCS_IPC_CREATE(&app_cb->mbx)) != NCSCC_RC_SUCCESS)
      {
         return(NCSCC_RC_FAILURE);
         
      }      
      
      if ((res = m_NCS_IPC_ATTACH(&app_cb->mbx)) != NCSCC_RC_SUCCESS)
      {
         m_NCS_IPC_RELEASE(&app_cb->mbx,NULL);
         return(NCSCC_RC_FAILURE);
         
      }
      strcpy(task_name,"ifsv_app");
      sprintf(tmp_char,"%d",test_app_index);
      m_NCS_STRCAT(task_name,tmp_char);
      
      /* Create a application thread */
      if ((res = m_NCS_TASK_CREATE ((NCS_OS_CB)ifsv_dt_test_app_process, 
         (NCSCONTEXT)app_cb, task_name, IFSV_DT_TEST_APP_PRIORITY, IFSV_DT_TEST_APP_STACKSIZE,
         &app_cb->task_hdl)) != NCSCC_RC_SUCCESS)         
      {
         m_NCS_IPC_RELEASE(&app_cb->mbx,NULL);
         return(NCSCC_RC_FAILURE);
         
      }
                  
      if ((res = m_NCS_TASK_START(app_cb->task_hdl))
         != NCSCC_RC_SUCCESS)         
      {
         m_NCS_IPC_RELEASE(&app_cb->mbx,NULL);
         m_NCS_TASK_RELEASE(app_cb->task_hdl);
         return(NCSCC_RC_FAILURE);
         
      }
      
      
      app_cb->inited = TRUE;
      m_NCS_CONS_PRINTF("App Num - %d, This number should be given for             destroy\n",test_app_index);
   } else
   {
      m_NCS_CONS_PRINTF("Sorry You have already created enough test applications\n");
   }
   return(NCSCC_RC_SUCCESS);
   
}
/****************************************************************************
 * Name          : IfsvDtTestAppIfaSub
 *
 * Description   : This is the function used to subscribe test application  
 *                 with IfA.
 *
 * Arguments     : app_no  - This is the application number.
 *                 evt_attr - what are all event attributes the application 
 *                            is interested.
 *                 rec_attr - what are all record attributes the application 
 *                            is interested.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppIfaSub(uns32 app_no, uns32 evt_attr, uns32 rec_attr)
{
   NCS_IFSV_SVC_REQ      svc_req;
   IFSV_DT_TEST_APP_CB      *app_cb;
   if (app_no < IFSV_DT_MAX_TEST_APP)
   {
      app_cb = &gifsv_dt_test_app[app_no];      
      /* Register with the application with IfA */
      memset(&svc_req,0,sizeof(NCS_IFSV_SVC_REQ));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_SUBSCR;
      svc_req.info.i_subr.i_ifsv_cb = ifsv_dt_test_ifa_sub_cb;
      svc_req.info.i_subr.i_sevts   = evt_attr;
      svc_req.info.i_subr.i_smap    = rec_attr;
    if(ncs_ifsv_red_demo_flag)
        svc_req.info.i_subr.subscr_scope = NCS_IFSV_SUBSCR_INT;

      svc_req.info.i_subr.i_usrhdl = (long)(NCSCONTEXT)app_cb;
      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {         
         m_NCS_CONS_PRINTF("App%d failed to register with IfA\n",app_no);
         return(NCSCC_RC_FAILURE);
         
      }
      app_cb->IfA_hdl = svc_req.info.i_subr.o_subr_hdl;
   } else
   {
      m_NCS_CONS_PRINTF("App%d not found\n",app_no);
      return(NCSCC_RC_FAILURE);
   }
   
   return(NCSCC_RC_SUCCESS);
  
}

/****************************************************************************
 * Name          : IfsvDtTestAppIfaUnsu
 *
 * Description   : This is the function used to unsubscribe test application  
 *                 with IfA.
 *
 * Arguments     : app_no  - This is the application number.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppIfaUnsub(uns32 app_no)
{

   NCS_IFSV_SVC_REQ      svc_req;
   IFSV_DT_TEST_APP_CB      *app_cb;

   if (app_no < IFSV_DT_MAX_TEST_APP)
   {
      app_cb = &gifsv_dt_test_app[app_no];      
      /* Register with the application with IfA */
      memset(&svc_req,0,sizeof(NCS_IFSV_SVC_REQ));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_UNSUBSCR;
      svc_req.info.i_unsubr.i_subr_hdl = app_cb->IfA_hdl;      
      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {         
         m_NCS_CONS_PRINTF("App%d failed to deregister with IfA\n",app_no);
         return(NCSCC_RC_FAILURE);
      }
   } else
   {
      m_NCS_CONS_PRINTF("App%d not found\n",app_no);
      return(NCSCC_RC_FAILURE);
   }

   return(NCSCC_RC_SUCCESS);
   
}

/****************************************************************************
 * Name          : IfsvDtTestAppGetStats
 *
 * Description   : This is the wrapper function to get the statistic 
 *                 information from the test application.
 *
 * Arguments     : app_no     - This is the application number.
 *                 shelf      - shelf number.
 *                 slot       - slot number.
 *                 port       - port number.
 *                 port_type  - port type.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppGetStats(uns32 app_no, uns32 shelf, uns32 slot, uns32 subslot, uns32 port, 
                             uns32 port_type)
{
   NCS_IFSV_SVC_REQ      svc_req;
   IFSV_DT_TEST_APP_CB      *app_cb;

   if (app_no < IFSV_DT_MAX_TEST_APP)
   {
      app_cb = &gifsv_dt_test_app[app_no];      
      /* Register with the application with IfA */
      memset(&svc_req,0,sizeof(NCS_IFSV_SVC_REQ));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_GET;      
      svc_req.info.i_ifget.i_info_type = NCS_IFSV_IFSTATS_INFO;      
      svc_req.info.i_ifget.i_rsp_type  = NCS_IFSV_GET_RESP_ASYNC;
      svc_req.info.i_ifget.i_subr_hdl  = app_cb->IfA_hdl;
      svc_req.info.i_ifget.i_key.type  = NCS_IFSV_KEY_SPT;
      svc_req.info.i_ifget.i_key.info.spt.shelf = shelf;
      svc_req.info.i_ifget.i_key.info.spt.slot = slot;
      /* embedding subslot changes */
      svc_req.info.i_ifget.i_key.info.spt.subslot = subslot;
      svc_req.info.i_ifget.i_key.info.spt.port = port;

      svc_req.info.i_ifget.i_key.info.spt.type = port_type;

      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {         
         m_NCS_CONS_PRINTF("App%d failed to get status with IfA\n",app_no);
         return(NCSCC_RC_FAILURE);
         
      }
   } else
   {
      m_NCS_CONS_PRINTF("App%d not found\n",app_no);
      return(NCSCC_RC_FAILURE);
      
   }

   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : IfsvDtTestAppGetIfinfo
 *
 * Description   : This is the wrapper function to get the interface
 *                 information from the test application.
 *
 * Arguments     : app_no     - This is the application number.
 *                 shelf      - shelf number.
 *                 slot       - slot number.
 *                 port       - port number.
 *                 port_type  - port type.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppGetIfinfo(uns32 app_no, uns32 shelf, uns32 slot, uns32 subslot, uns32 port ,uns32 port_type)
{
   NCS_IFSV_SVC_REQ      svc_req;
   IFSV_DT_TEST_APP_CB      *app_cb;

   if (app_no < IFSV_DT_MAX_TEST_APP)
   {
      app_cb = &gifsv_dt_test_app[app_no];      
      /* Register with the application with IfA */
      memset(&svc_req,0,sizeof(NCS_IFSV_SVC_REQ));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_GET;      
      svc_req.info.i_ifget.i_info_type = NCS_IFSV_IF_INFO;      
      svc_req.info.i_ifget.i_rsp_type  = NCS_IFSV_GET_RESP_ASYNC;
      svc_req.info.i_ifget.i_subr_hdl  = app_cb->IfA_hdl;
      svc_req.info.i_ifget.i_key.type  = NCS_IFSV_KEY_SPT;
      svc_req.info.i_ifget.i_key.info.spt.shelf = shelf;
      svc_req.info.i_ifget.i_key.info.spt.slot = slot;
      /* embedding subslot changes */
      svc_req.info.i_ifget.i_key.info.spt.subslot = subslot;
      svc_req.info.i_ifget.i_key.info.spt.port = port;
      svc_req.info.i_ifget.i_key.info.spt.type = port_type;

      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {         
         m_NCS_CONS_PRINTF("App%d failed to get ifinfo with IfA\n",app_no);
         return(NCSCC_RC_FAILURE);
         
      }
   } else
   {
      m_NCS_CONS_PRINTF("App%d not found\n",app_no);
      return(NCSCC_RC_FAILURE);
      
   }

   return(NCSCC_RC_SUCCESS);
   
}

/****************************************************************************
 * Name          :  IfsvDtTestAppGetAll
 *
 * Description   : This is the wrapper function to get the all the interface
 *                 information from the test application unsing ifindex.
 *
 * Arguments     : app_no     - This is the application number.
 *                 ifindex    - Interface index.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppGetAll(uns32 app_no, uns32 ifindex)
{
   NCS_IFSV_SVC_REQ      svc_req;
   IFSV_DT_TEST_APP_CB      *app_cb;  

   if (app_no < IFSV_DT_MAX_TEST_APP)
   {
      app_cb = &gifsv_dt_test_app[app_no];      
      /* Register with the application with IfA */
      memset(&svc_req,0,sizeof(NCS_IFSV_SVC_REQ));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_GET;      
      svc_req.info.i_ifget.i_info_type = NCS_IFSV_IF_INFO | NCS_IFSV_IFSTATS_INFO;      
      svc_req.info.i_ifget.i_rsp_type  = NCS_IFSV_GET_RESP_ASYNC;
      svc_req.info.i_ifget.i_subr_hdl  = app_cb->IfA_hdl;
      svc_req.info.i_ifget.i_key.type  = NCS_IFSV_KEY_IFINDEX;
      svc_req.info.i_ifget.i_key.info.iface = ifindex;      

      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {         
         m_NCS_CONS_PRINTF("App%d failed to get all with IfA\n",app_no);
         return(NCSCC_RC_FAILURE);
         
      } else
      {
          m_NCS_CONS_PRINTF("App%d not found\n",app_no);
          return(NCSCC_RC_FAILURE);
      
      }
   }
   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          :  IfsvDtTestAppDestroy
 *
 * Description   : This is the wrapper function used to destroy the test 
 *                 application.
 *
 * Arguments     : app_no     - This is the application number.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppDestroy(uns32 app_no)
{
   if (app_no < IFSV_DT_MAX_TEST_APP)
   {
      if (gifsv_dt_test_app[app_no].inited == TRUE)
      {
         /* Deregister with the application with IfA*/
         /* Detach the Mail Box */
         /* Delete the application thread */
         gifsv_dt_test_app[app_no].inited = FALSE;
         gifsv_dt_total_test_app_created--;
         return(NCSCC_RC_SUCCESS);
      } else
      {
         m_NCS_CONS_PRINTF("Sorry You have entered wrong test app number %d\n",app_no);
      }
   } else
   {
      m_NCS_CONS_PRINTF("Sorry You have entered wrong test app number %d\n",app_no);
      return(NCSCC_RC_FAILURE);
   }   
   return(NCSCC_RC_SUCCESS);
}
/****************************************************************************
 * Name          :  IfsvDtTestAppAddIntf
 *
 * Description   : This is the wrapper function used to add interface 
 *                 information from test application.
 *
 * Arguments     : app_num     - This is the application number.
 *                 if_name     - Interface name.
 *                 port_num    - port number.
 *                 port_type   - port type.
 *                 *i_phy      - MAC address.
 *                 oper_state  - operational status.
 *                 MTU         - MAX Transmition Unit.
 *                 speed       - Interface speed.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppAddIntf(uns32 app_num, char *if_name, uns32 port_num, 
                            uns32 port_type, uns8 *iphy, uns32 oper_state, 
                            uns32 MTU, uns32 speed)
{

   uns8  phy[6];

   IFSV_DT_TEST_APP_CB *app_cb;
   NCS_IFSV_SVC_REQ svc_req;
   NCS_IFSV_PORT_TYPE type;

   /* extract the MAC address */
   if (ifsv_dt_extract_phy_addr(iphy, phy) == NCSCC_RC_FAILURE)
   {
      m_NCS_CONS_PRINTF("Phy extract failed \n");
      return(NCSCC_RC_FAILURE);
      
   }
   type.port_id = port_num;
   type.type    = port_type;

   if(app_num < IFSV_DT_MAX_TEST_APP)
   { 
      app_cb = &gifsv_dt_test_app[app_num];
      memset(&svc_req,0,sizeof(svc_req));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_ADD;
      svc_req.info.i_ifadd.spt_info.port  = port_num;
      svc_req.info.i_ifadd.spt_info.shelf = app_cb->shelf_no;
      svc_req.info.i_ifadd.spt_info.slot  = app_cb->slot_no;
      /* embedding subslot changes */
      svc_req.info.i_ifadd.spt_info.subslot  = app_cb->subslot_no;
      svc_req.info.i_ifadd.spt_info.type  = port_type;
      svc_req.info.i_ifadd.if_info.admin_state =  TRUE;
      svc_req.info.i_ifadd.if_info.if_am       = (NCS_IFSV_IAM_MTU|NCS_IFSV_IAM_IFSPEED|NCS_IFSV_IAM_PHYADDR|NCS_IFSV_IAM_ADMSTATE|NCS_IFSV_IAM_OPRSTATE);
      strcpy(svc_req.info.i_ifadd.if_info.if_descr,"haha dummy intf");
      strcpy(svc_req.info.i_ifadd.if_info.if_name,if_name);
      svc_req.info.i_ifadd.if_info.if_speed = speed;
      svc_req.info.i_ifadd.if_info.mtu      = MTU;
      svc_req.info.i_ifadd.if_info.oper_state = oper_state;
      memcpy(svc_req.info.i_ifadd.if_info.phy_addr,phy,(6*sizeof(uns8)));
      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {
         m_NCS_CONS_PRINTF("IfA could not able to add the interface\n");
      } else
      {
         m_NCS_CONS_PRINTF("IfA added the interface successfully\n");
      }
      return(NCSCC_RC_SUCCESS);
   } else
   {
      m_NCS_CONS_PRINTF("Wrong App Number - %d\n",app_num);
      return(NCSCC_RC_FAILURE);
   }     
   
}
/****************************************************************************
 * Name          :  IfsvDtTestAppModIntfStatus
 *
 * Description   : This is the wrapper function used to Modify interface 
 *                 status information from test application.
 *
 * Arguments     : app_num     - This is the application number.
 *                 port_num    - port number.
 *                 port_type   - port type.
 *                 oper_state  - operational status.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppModIntfStatus(uns32 app_num, uns32 port_num, 
                                   uns32 port_type, uns32 oper_state)
{
   IFSV_DT_TEST_APP_CB *app_cb;
   NCS_IFSV_SVC_REQ svc_req;

    if(app_num < IFSV_DT_MAX_TEST_APP)
    { 
      app_cb = &gifsv_dt_test_app[app_num];
      memset(&svc_req,0,sizeof(svc_req));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_ADD;
      svc_req.info.i_ifadd.spt_info.port  = port_num;
      svc_req.info.i_ifadd.spt_info.shelf = app_cb->shelf_no;
      svc_req.info.i_ifadd.spt_info.slot  = app_cb->slot_no;
      /* embedding subslot changes */
      svc_req.info.i_ifadd.spt_info.subslot  = app_cb->subslot_no;
      svc_req.info.i_ifadd.spt_info.type  = port_type;      
      svc_req.info.i_ifadd.if_info.if_am       = NCS_IFSV_IAM_OPRSTATE;            
      svc_req.info.i_ifadd.if_info.oper_state = oper_state;      
      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {
         m_NCS_CONS_PRINTF("IfA could not able to Modify operation status\n");
      } else
      {
         m_NCS_CONS_PRINTF("IfA Modified operation status successfully\n");
      }
      return(NCSCC_RC_SUCCESS);
   } else
   {
      m_NCS_CONS_PRINTF("Wrong App Number - %d\n",app_num);
      return(NCSCC_RC_FAILURE);
   }     
   
}

uns32 IfsvDtTestAppGetBondLocalIfinfo(uns32 app_no, uns32 shelf, uns32 slot, uns32 subslot, uns32 port ,uns32 port_type)
{
   NCS_IFSV_SVC_REQ      svc_req;
   IFSV_DT_TEST_APP_CB      *app_cb;

   if (app_no < IFSV_DT_MAX_TEST_APP)
   {
      app_cb = &gifsv_dt_test_app[app_no];      
      /* Register with the application with IfA */
      memset(&svc_req,0,sizeof(NCS_IFSV_SVC_REQ));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_GET;      
      svc_req.info.i_ifget.i_info_type = NCS_IFSV_BIND_GET_LOCAL_INTF;      
      svc_req.info.i_ifget.i_rsp_type  = NCS_IFSV_GET_RESP_ASYNC;
      svc_req.info.i_ifget.i_subr_hdl  = app_cb->IfA_hdl;
      svc_req.info.i_ifget.i_key.type  = NCS_IFSV_KEY_SPT;
      svc_req.info.i_ifget.i_key.info.spt.shelf = shelf;
      svc_req.info.i_ifget.i_key.info.spt.slot = slot;
      /* embedding subslot changes */
      svc_req.info.i_ifget.i_key.info.spt.subslot = subslot;
      svc_req.info.i_ifget.i_key.info.spt.port = port;
      svc_req.info.i_ifget.i_key.info.spt.type = port_type;
      svc_req.info.i_ifget.i_key.info.spt.subscr_scope = NCS_IFSV_SUBSCR_INT;

      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {         
         m_NCS_CONS_PRINTF("App%d failed to get ifinfo with IfA\n",app_no);
         return(NCSCC_RC_FAILURE);
 
      }
   } else
   {
      m_NCS_CONS_PRINTF("App%d not found\n",app_no);
      return(NCSCC_RC_FAILURE);
      
   }

   return(NCSCC_RC_SUCCESS);
   
}



uns32 IfsvDtTestAppSwapBondIntf(uns32 app_num, uns32 port_num, 
                                   uns32 port_type, uns32 master_ifindex, uns32 slave_ifindex)
{
   IFSV_DT_TEST_APP_CB *app_cb;
   NCS_IFSV_SVC_REQ svc_req;

    if(app_num < IFSV_DT_MAX_TEST_APP)
    { 
      app_cb = &gifsv_dt_test_app[app_num];
      memset(&svc_req,0,sizeof(svc_req));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_ADD;
      svc_req.info.i_ifadd.spt_info.port  = port_num;
      svc_req.info.i_ifadd.spt_info.shelf = IFSV_BINDING_SHELF_ID;
      svc_req.info.i_ifadd.spt_info.slot  = IFSV_BINDING_SLOT_ID;
      /* embedding subslot changes */
      svc_req.info.i_ifadd.spt_info.subslot  = IFSV_BINDING_SUBSLOT_ID;
      svc_req.info.i_ifadd.spt_info.type  = port_type;      
      svc_req.info.i_ifadd.spt_info.subscr_scope  = NCS_IFSV_SUBSCR_INT;      
      svc_req.info.i_ifadd.if_info.if_am       = NCS_IFSV_IAM_CHNG_MASTER;            
      svc_req.info.i_ifadd.if_info.bind_master_ifindex = master_ifindex;      
      svc_req.info.i_ifadd.if_info.bind_slave_ifindex = slave_ifindex;            
      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {
         m_NCS_CONS_PRINTF("Ifa not able to add bind interface\n");
      } else
      {
         m_NCS_CONS_PRINTF("Ifa successfully added bind interface\n");
      }
      return(NCSCC_RC_SUCCESS);
   } else
   {
      m_NCS_CONS_PRINTF("Wrong App Number - %d\n",app_num);
      return(NCSCC_RC_FAILURE);
   }     
   
}





/****************************************************************************
 * Name          :  IfsvDtTestAppModIntfMTU
 *
 * Description   : This is the wrapper function used to Modify interface 
 *                 MTU information from test application.
 *
 * Arguments     : app_num     - This is the application number.
 *                 port_num    - port number.
 *                 port_type   - port type.
 *                 MTU         - Maximum Tramission Unit.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppModIntfMTU(uns32 app_num, uns32 port_num, uns32 port_type,
                                uns32 MTU)
{
   IFSV_DT_TEST_APP_CB *app_cb;
   NCS_IFSV_SVC_REQ svc_req;

   if(app_num < IFSV_DT_MAX_TEST_APP)
   { 
      app_cb = &gifsv_dt_test_app[app_num];
      memset(&svc_req,0,sizeof(svc_req));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_ADD;
      svc_req.info.i_ifadd.spt_info.port  = port_num;
      svc_req.info.i_ifadd.spt_info.shelf = app_cb->shelf_no;
      svc_req.info.i_ifadd.spt_info.slot  = app_cb->slot_no;
      /* embedding subslot changes */
      svc_req.info.i_ifadd.spt_info.subslot  = app_cb->subslot_no;
      svc_req.info.i_ifadd.spt_info.type  = port_type;      
      svc_req.info.i_ifadd.if_info.if_am  = NCS_IFSV_IAM_MTU;            
      svc_req.info.i_ifadd.if_info.mtu    = MTU;      
      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {
         m_NCS_CONS_PRINTF("IfA could not able to Modify MTU\n");
      } else
      {
         m_NCS_CONS_PRINTF("IfA Modified MTU successfully\n");
      }
      return(NCSCC_RC_SUCCESS);
   } else
   {
      m_NCS_CONS_PRINTF("Wrong App Number - %d\n",app_num);
      return(NCSCC_RC_FAILURE);
   }     
   

}

/****************************************************************************
 * Name          :  IfsvDtTestAppModIntfSpeed
 *
 * Description   : This is the wrapper function used to Modify interface 
 *                 speed information from test application.
 *
 * Arguments     : app_num     - This is the application number.
 *                 port_num    - port number.
 *                 port_type   - port type.
 *                 speed       - interface speed.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppModIntfSpeed(uns32 app_num, uns32 port_num, 
                                  uns32 port_type, uns32 speed)
{
   IFSV_DT_TEST_APP_CB *app_cb;
   NCS_IFSV_SVC_REQ svc_req;

   if(app_num < IFSV_DT_MAX_TEST_APP)
   { 
      app_cb = &gifsv_dt_test_app[app_num];
      memset(&svc_req,0,sizeof(svc_req));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_ADD;
      svc_req.info.i_ifadd.spt_info.port  = port_num;
      svc_req.info.i_ifadd.spt_info.shelf = app_cb->shelf_no;
      svc_req.info.i_ifadd.spt_info.slot  = app_cb->slot_no;
      /* embedding subslot changes */
      svc_req.info.i_ifadd.spt_info.subslot  = app_cb->subslot_no;
      svc_req.info.i_ifadd.spt_info.type  = port_type;      
      svc_req.info.i_ifadd.if_info.if_am       = NCS_IFSV_IAM_IFSPEED;            
      svc_req.info.i_ifadd.if_info.if_speed    = speed;      
      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {
         m_NCS_CONS_PRINTF("IfA could not able to Modify interface speed\n");
      } else
      {
         m_NCS_CONS_PRINTF("IfA Modified interface speed successfully\n");
      }
      return(NCSCC_RC_SUCCESS);
   } else
   {
      m_NCS_CONS_PRINTF("Wrong App Number - %d\n",app_num);
      return(NCSCC_RC_FAILURE);
   }     
   
}

/****************************************************************************
 * Name          :  IfsvdtTestAppModIntfPhy
 *
 * Description   : This is the wrapper function used to Modify interface 
 *                 MAC address information from test application.
 *
 * Arguments     : app_num     - This is the application number.
 *                 port_num    - port number.
 *                 port_type   - port type.
 *                 temp_phy    - MAC address.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvdtTestAppModIntfPhy(uns32 app_num, uns32 port_num, uns32 port_type,
                                char *temp_phy )
{

   IFSV_DT_TEST_APP_CB *app_cb;
   NCS_IFSV_SVC_REQ svc_req;

   if(app_num < IFSV_DT_MAX_TEST_APP)
   { 
      app_cb = &gifsv_dt_test_app[app_num];
      memset(&svc_req,0,sizeof(svc_req));
      /* extract the MAC address */
      if (ifsv_dt_extract_phy_addr(temp_phy, svc_req.info.i_ifadd.if_info.phy_addr)
         == NCSCC_RC_FAILURE)
      {
         return(NCSCC_RC_FAILURE);
         
      }      

      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_ADD;
      svc_req.info.i_ifadd.spt_info.port  = port_num;
      svc_req.info.i_ifadd.spt_info.shelf = app_cb->shelf_no;
      svc_req.info.i_ifadd.spt_info.slot  = app_cb->slot_no;
      /* embedding subslot changes */
      svc_req.info.i_ifadd.spt_info.subslot  = app_cb->subslot_no;
      svc_req.info.i_ifadd.spt_info.type  = port_type;      
      svc_req.info.i_ifadd.if_info.if_am  = NCS_IFSV_IAM_PHYADDR;                  
      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {
         m_NCS_CONS_PRINTF("IfA could not able to Modify physical address\n");
      } else
      {
         m_NCS_CONS_PRINTF("IfA Modified physical address successfully\n");
      }
      return(NCSCC_RC_SUCCESS);
   } else
   {
      m_NCS_CONS_PRINTF("Wrong App Number - %d\n",app_num);
      return(NCSCC_RC_FAILURE);
   }     
   
}

/****************************************************************************
 * Name          :  IfsvDtTestAppModIntfName
 *
 * Description   : This is the wrapper function used to Modify interface 
 *                 name information from test application.
 *
 * Arguments     : app_num     - This is the application number.
 *                 port_num    - port number.
 *                 port_type   - port type.
 *                 temp_phy    - MAC address.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppModIntfName(uns32 app_num, uns32 port_num, uns32 port_type,
                                 char *temp_name)
{
   IFSV_DT_TEST_APP_CB *app_cb;
   NCS_IFSV_SVC_REQ svc_req;

   if(app_num < IFSV_DT_MAX_TEST_APP)
   { 
      app_cb = &gifsv_dt_test_app[app_num];
      memset(&svc_req,0,sizeof(svc_req));

      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_ADD;
      svc_req.info.i_ifadd.spt_info.port  = port_num;
      svc_req.info.i_ifadd.spt_info.shelf = app_cb->shelf_no;
      svc_req.info.i_ifadd.spt_info.slot  = app_cb->slot_no;
      /* embedding subslot changes */
       svc_req.info.i_ifadd.spt_info.subslot  = app_cb->subslot_no;
      svc_req.info.i_ifadd.spt_info.type  = port_type;      
      svc_req.info.i_ifadd.if_info.if_am  = NCS_IFSV_IAM_NAME;                  
      strcpy(svc_req.info.i_ifadd.if_info.if_name,temp_name);
      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {
         m_NCS_CONS_PRINTF("IfA could not able to Modify physical address\n");
      } else
      {
         m_NCS_CONS_PRINTF("IfA Modified physical address successfully\n");
      }
      return(NCSCC_RC_SUCCESS);
   } else
   {
      m_NCS_CONS_PRINTF("Wrong App Number - %d\n",app_num);
      return(NCSCC_RC_FAILURE);
   }     
   
}

/****************************************************************************
 * Name          :  IfsvDtTestAppDelIntf
 *
 * Description   : This is the wrapper function used to delete interface 
 *                 information from test application.
 *
 * Arguments     : app_num     - This is the application number.
 *                 port_num    - port number.
 *                 port_type   - port type.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 IfsvDtTestAppDelIntf(uns32 app_num, uns32 port_num, uns32 port_type)
{

   IFSV_DT_TEST_APP_CB *app_cb;
   NCS_IFSV_SVC_REQ svc_req;

   if(app_num < IFSV_DT_MAX_TEST_APP)
   { 
      app_cb = &gifsv_dt_test_app[app_num];
      memset(&svc_req,0,sizeof(svc_req));
      svc_req.i_req_type = NCS_IFSV_SVC_REQ_IFREC_DEL;
      svc_req.info.i_ifdel.port  = port_num;
      svc_req.info.i_ifdel.shelf = app_cb->shelf_no;
      svc_req.info.i_ifdel.slot  = app_cb->slot_no;
      /* embedding subslot changes */
      svc_req.info.i_ifdel.subslot  = app_cb->subslot_no;
      svc_req.info.i_ifdel.type  = port_type;

      if (ncs_ifsv_svc_req(&svc_req) != NCSCC_RC_SUCCESS)
      {
         m_NCS_CONS_PRINTF("IfA could not able to del the interface\n");
      } else
      {
         m_NCS_CONS_PRINTF("IfA del the interface successfully\n");
      }
      return(NCSCC_RC_SUCCESS);
   } else
   {
      m_NCS_CONS_PRINTF("Wrong App Number - %d\n",app_num);
      return(NCSCC_RC_FAILURE);
   }     
}

/****************************************************************************
 *
 * Function Name: intf_dt_phase_token
 *
 * Purpose: This is the API to the parse the token.
 *    
 * Arguments: *s:  the string to be parsed
 *            *delim: delimitter to be parsed. 
 *
 * Return Value: NCSCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
static int8 * 
intf_dt_phase_token( int8 *s, const int8 *delim)
{
    static int8 *olds = 0;
    int8 *token;

    if (s == 0)
    {
        if (olds == 0)
        {
            return 0;
        } else
            s = olds;
    }

    /* Scan leading delimiters.  */
    s += strspn((char *)s, (const char *)delim);
    if (*s == '\0')
    {
        olds = 0;
        return 0;
    }

    /* Find the end of the token.  */
    token = s;
    s = strpbrk((char *)token, (const char *)delim);
    if (s == 0)
    {
        /* This token finishes the string.  */
        olds = 0;
    }
    else
    {
        /* Terminate the token and make OLDS point past it.  */
        *s = '\0';
        olds = s + 1;
    }
    return token;
}

/****************************************************************************
 *
 * Function Name: ifsv_dt_extract_phy_addr
 *
 * Purpose: This is the function used to extract physicall address from the
 *           string.
 *    
 * Arguments: *arg    : string to be parsed.
 *            *pPhyMAC: physical MAC address to be extracted. 
 *
 * Return Value: NCSCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
uns32 ifsv_dt_extract_phy_addr (int8 *arg, uns8 pPhyMAC[6])
{
   int32   tempVarCount = 0;
   int8    *token;

   do
   {
      token = intf_dt_phase_token( arg, ":" );
      if (((token == NULL) && (tempVarCount < 5)) ||
         ((token != NULL) && (tempVarCount > 5)))
      {
         return (NCSCC_RC_FAILURE);
      } else if (token == NULL)
      {
         break;
      }
      pPhyMAC[tempVarCount] = (uns8)atoi(token);
      arg = NULL;
      tempVarCount++;
   } while (TRUE);
   return (NCSCC_RC_SUCCESS);
}
