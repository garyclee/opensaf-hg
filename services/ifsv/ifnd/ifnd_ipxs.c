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
#if(NCS_IFSV_USE_NETLINK == 1)
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#endif
#include "ncs_iplib.h"

NCSCONTEXT gl_ifsv_netlink_task_hdl = 0;


static uns32 ipxs_ifnd_lib_create (IPXS_LIB_CREATE *create);

static uns32 ipxs_ifnd_lib_destroy (void);

static uns32 ifnd_ipxs_proc_get_ifip_req(IPXS_CB *cb, IPXS_EVT *ipxs_evt, 
                                                         IFSV_SEND_INFO *sinfo);


static uns32 ifnd_ipxs_proc_ifip_info(IPXS_CB *cb, IPXS_EVT *ipxs_evt);

static uns32 ipxs_ifnd_ifip_info_bcast(IPXS_CB *cb, uns32 if_index, 
                                              NCS_IPXS_IPINFO *ip_info);

static void ifnd_ipxs_get_ifindex_from_key(IPXS_CB *cb, NCS_IPXS_IFKEY *key, 
                                           NCS_IFSV_IFINDEX *if_index);
uns32 ifnd_ipxs_proc_ifip_upd(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                     IFSV_SEND_INFO *sinfo);

static uns32 ifnd_ipxs_proc_isloc(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                         IFSV_SEND_INFO *sinfo);

static uns32 ifnd_ipxs_node_rec_get(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                         IFSV_SEND_INFO *sinfo);

static void ifnd_ipxs_get_ifindex_from_spt(NCS_IFSV_SPT *spt, 
                                           NCS_IFSV_IFINDEX *if_index);

static uns32 ifnd_ipxs_proc_is_ipaddr(IPXS_CB *cb, NCS_IP_ADDR ipxs_ipaddr);

static IPXS_NETLINK_RETVAL ipxs_post_netlink_evt_to_ifnd(struct sockaddr_nl *who,
                               struct nlmsghdr *n, IPXS_CB *ipxs_cb,
                               IFSV_CB *ifsv_cb);

static IPXS_NETLINK_RETVAL ipxs_netlink_dispatch(IFSV_CB *ifsv_cb, IPXS_CB *ipxs_cb);

static void ifnd_netlink_process(NCSCONTEXT info);

static int 
parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len);

static uns32 ipxs_ifnd_gen_getlink_req(IPXS_CB *ipxs_cb);

static uns32 ipxs_ifnd_gen_getv4addr_req(IPXS_CB *ipxs_cb);

static uns32 ifnd_ipxs_get_ifndx_for_interface_number(IFSV_CB *ifsv_cb, IPXS_CB *ipxs_cb,
           int link_index, NCS_IFSV_IFINDEX *ifindex, char *ifname);

/****************************************************************************
 * Name          : ipxs_ifnd_lib_req
 *
 * Description   : This is the NCS SE API which is used to Create/destroy library
 *                 of the IPXS.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ipxs_ifnd_lib_req (IPXS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   switch (req_info->op)
   {      
      case NCS_LIB_REQ_CREATE:
         rc = ipxs_ifnd_lib_create(&req_info->info.create);
         break;
      case NCS_LIB_REQ_DESTROY:
         rc = ipxs_ifnd_lib_destroy( );
         break;
      default:
         break;
   }
   return (rc);
}

/****************************************************************************
 * Name          : ipxs_ifnd_lib_create
 *
 * Description   : This is the function which initalize the IPXS databases
 *                 and library.
 *                 
 * Arguments     : pointer to NCS_LIB_CREATE
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ipxs_ifnd_lib_create (IPXS_LIB_CREATE *create)
{

   IPXS_CB                 *cb = NULL;
   uns32                   rc;
   NCS_PATRICIA_PARAMS     params;
#if(NCS_IFSV_USE_NETLINK == 1)
   struct sockaddr_nl      local;
#endif

   /* Malloc the Control Block for IPXS */
   cb = m_MMGR_ALLOC_IPXS_CB;

   if(cb == NULL)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_MEMSET(cb, 0, sizeof(IPXS_CB));

   cb->hm_pid     = create->pool_id;
   cb->my_svc_id  = create->svc_id;
   cb->oac_hdl    = create->oac_hdl;
   cb->my_mds_hdl    = create->mds_hdl;

   m_NCS_MEMSET(&cb->nl_addr_info, '\0', sizeof(cb->nl_addr_info));

   /* Initialize the IF-IP Table in IPXS CB */
   params.key_size = sizeof(uns32);
   params.info_size = 0;
   if ((ncs_patricia_tree_init(&cb->ifip_tbl, &params)) 
                                                != NCSCC_RC_SUCCESS)
   {
      rc = NCSCC_RC_FAILURE;
      goto ipxs_db_init_fail;
   }
   cb->ifip_tbl_up = TRUE;


   /* Handle Manager Init Routines */
   if((cb->cb_hdl = ncshm_create_hdl(cb->hm_pid, 
               cb->my_svc_id, (NCSCONTEXT)cb)) == 0)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_HDL_MGR_FAIL,IFSV_COMP_IFND);
      rc = NCSCC_RC_FAILURE;
      goto ipxs_hdl_fail;            
   }

   m_IPXS_STORE_HDL(cb->cb_hdl);
   /* Initialization of LOCK */
   if(m_NCS_OS_LOCK(&cb->ipxs_db_lock, NCS_OS_LOCK_CREATE, 0) == NCSCC_RC_FAILURE)
   {
       rc = NCSCC_RC_FAILURE;
       m_NCS_CONS_PRINTF("\nLock create failure\n");
       m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_LOCK_CREATE_FAIL,(long)cb);
       goto ipxs_lock_create_fail;
   }

#if(NCS_IFSV_USE_NETLINK == 1)
   /* Create netlink-thread of IFND */
   cb->netlink_fd = m_NCS_TS_SOCK_SOCKET(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
   if(cb->netlink_fd < 0)
   {
      rc = NCSCC_RC_FAILURE;
      goto ipxs_netlink_socket_create_fail;            
   }

   m_NCS_MEMSET(&local, '\0', sizeof(local));
   local.nl_family = AF_NETLINK;
   local.nl_pid = getpid();
   local.nl_groups = (RTMGRP_IPV4_IFADDR); 
   if(m_NCS_TS_SOCK_BIND(cb->netlink_fd, (struct sockaddr*)&local, sizeof(local)) < 0)
   {
      rc = NCSCC_RC_FAILURE;
      goto ipxs_netlink_socket_bind_fail;            
   }

   if ((rc = m_NCS_TASK_CREATE ((NCS_OS_CB)ifnd_netlink_process,
         (NCSCONTEXT)((long)create->ifsv_hdl),
         NCS_IFSV_NETLINK_TASKNAME,
         NCS_IFSV_NETLINK_PRIORITY,
         NCS_IFSV_NETLINK_STACKSIZE,
         &gl_ifsv_netlink_task_hdl)) != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_TASK_CREATE_FAIL,0);
      goto ipxs_task_create_fail;
   }

   if ((rc = m_NCS_TASK_START(gl_ifsv_netlink_task_hdl))
        != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_TASK_CREATE_FAIL,\
         (long)gl_ifsv_netlink_task_hdl);
      goto ipxs_task_start_fail;
   }
#endif /* #if(NCS_IFSV_USE_NETLINK == 1) */
   
   return NCSCC_RC_SUCCESS;

#if(NCS_IFSV_USE_NETLINK == 1)
ipxs_task_start_fail:
   /*m_NCS_TASK_STOP(gl_ifsv_netlink_task_hdl); */
   m_NCS_TASK_RELEASE(gl_ifsv_netlink_task_hdl);

ipxs_task_create_fail:
   gl_ifsv_netlink_task_hdl = 0;

ipxs_netlink_socket_bind_fail:
ipxs_netlink_socket_create_fail:
   m_NCS_TS_SOCK_CLOSE(cb->netlink_fd);
   m_NCS_OS_LOCK(&cb->ipxs_db_lock, NCS_OS_LOCK_RELEASE, 0);
#endif /* #if(NCS_IFSV_USE_NETLINK == 1) */
ipxs_lock_create_fail:
   ncshm_destroy_hdl(cb->my_svc_id, cb->cb_hdl);
ipxs_hdl_fail:
   /* Free the database */
   if(cb->ifip_tbl_up)
      ncs_patricia_tree_destroy(&cb->ifip_tbl);
ipxs_db_init_fail:
   /* Free the Control Block */
   m_MMGR_FREE_IPXS_CB(cb);

   return rc;
}

/****************************************************************************
 * Name          : ipxs_ifnd_lib_destroy
 *
 * Description   : This is the function which destroy the IPXS libarary.
 *                
 *
 * Arguments     : pointer to NCS_LIB_DESTROY
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ipxs_ifnd_lib_destroy ( )
{ 
   uns32    ipxs_hdl;
   IPXS_CB   *cb = NULL;
   uns32     svc_id;

   /* Get the Handle Manager Handle */
   ipxs_hdl = m_IPXS_CB_HDL_GET();

   cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);

   if (cb == NULL)
   {
      m_IFND_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,IFSV_COMP_IFND);
      return (NCSCC_RC_FAILURE);
   }

   svc_id = cb->my_svc_id;
   
#if(NCS_IFSV_USE_NETLINK == 1)
   /* Close the netlink-interface of IFND */
   m_NCS_TS_SOCK_CLOSE(cb->netlink_fd); /* This will cause the thread to get EINTR, 
                                           which triggers the thread to exit normally. */
   gl_ifsv_netlink_task_hdl = 0;
#endif

   m_NCS_OS_LOCK(&cb->ipxs_db_lock, NCS_OS_LOCK_RELEASE, 0);
   
   /* Free the databases */
   if(cb->ifip_tbl_up)
      ipxs_ifip_db_destroy(&cb->ifip_tbl, &cb->ifip_tbl_up);

   /* Give the Handle */
   ncshm_give_hdl(ipxs_hdl);

   /* Destroy the Handle */
   ncshm_destroy_hdl(svc_id, ipxs_hdl);

   /* Free the Control Block */
   m_MMGR_FREE_IPXS_CB(cb);

   return (NCSCC_RC_SUCCESS);
}

uns32 ifnd_ipxs_evt_process(IFSV_CB *cb, IFSV_EVT *evt)
{
   uns32    rc = NCSCC_RC_SUCCESS;
   IPXS_CB  *ipxs_cb;
   uns32    ipxs_hdl;
   IPXS_EVT *ipxs_evt = (IPXS_EVT *) evt->info.ipxs_evt;

   if(!ipxs_evt)
      return rc;

   /* Get the IPXS CB */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);

   switch(ipxs_evt->type)
   {
   case IPXS_EVT_DTOND_IFIP_INFO: /* From D */
   case IPXS_EVT_OSTOND_IFIP_INFO: /* From OS, source is netlink. */
      rc = ifnd_ipxs_proc_ifip_info(ipxs_cb, ipxs_evt);
      break;
   case IPXS_EVT_ATOND_IFIP_GET:
      rc = ifnd_ipxs_proc_get_ifip_req(ipxs_cb, ipxs_evt, &evt->sinfo);
      break;
   case IPXS_EVT_ATOND_IFIP_UPD:
      rc = ifnd_ipxs_proc_ifip_upd(ipxs_cb, ipxs_evt, &evt->sinfo);
      break;
   case IPXS_EVT_ATOND_IS_LOCAL:
      rc = ifnd_ipxs_proc_isloc(ipxs_cb, ipxs_evt, &evt->sinfo);
      break;
   case IPXS_EVT_ATOND_NODE_REC_GET:
      rc = ifnd_ipxs_node_rec_get(ipxs_cb, ipxs_evt, &evt->sinfo);
      break;
   default:
      rc = NCSCC_RC_FAILURE;
      break;
   }

   /* Free the received event */
    m_MMGR_FREE_IPXS_EVT(ipxs_evt); 
    evt->info.ipxs_evt = NULL;

   /* Give the Handle */
   ncshm_give_hdl(ipxs_hdl);

   return rc;
}

#if(NCS_IFSV_USE_NETLINK == 1)

int count = 0;


static uns32
ipxs_ifnd_update_new_socket(IPXS_CB  *ipxs_cb,NCS_SEL_OBJ_SET *readfds,
                                     NCS_SEL_OBJ *netlink_fd,
                                     NCS_SEL_OBJ *numfds ,
                                     NCS_SEL_OBJ *mbx_fd,
                                     struct sockaddr_nl *local)
{
    uns32 fd = 0;
    uns32 rc;

    m_NCS_SEL_OBJ_CLR(*netlink_fd, readfds);
    m_NCS_CONS_PRINTF("Closing the previous socket \n");
    m_NCS_TS_SOCK_CLOSE(ipxs_cb->netlink_fd);
    m_NCS_CONS_PRINTF("Opening a New socket \n");
    ipxs_cb->netlink_fd = m_NCS_TS_SOCK_SOCKET(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if(ipxs_cb->netlink_fd < 0)
    {
        rc = NCSCC_RC_FAILURE;
        m_NCS_CONS_PRINTF("Socket create failed \n");
/*      goto ipxs_netlink_socket_create_fail; */
    }
    m_NCS_MEMSET(local, '\0', sizeof(struct sockaddr_nl));
    local->nl_family = AF_NETLINK;
    local->nl_pid = getpid();
    local->nl_groups = (RTMGRP_IPV4_IFADDR);
    if(m_NCS_TS_SOCK_BIND(ipxs_cb->netlink_fd, (struct sockaddr*)local, sizeof(struct sockaddr_nl)) < 0)
    {
        rc = NCSCC_RC_FAILURE;
        m_NCS_CONS_PRINTF("Socket Bind Failed \n");
/*      goto ipxs_netlink_socket_bind_fail; */
    }
    fd = ipxs_cb->netlink_fd;
    *netlink_fd = m_NCS_IPC_GET_SEL_OBJ(&fd);

    numfds->raise_obj = 0;
    numfds->rmv_obj = 0;
    m_NCS_SEL_OBJ_ZERO(readfds);
    return NCSCC_RC_SUCCESS;
}




/****************************************************************************
 * Name          : ifnd_netlink_process
 *
 * Description   : This is the main function being run by the netlink thread
 *                 of IFND. This is a separate NETLINK thread SPAWNED by IFND.
 *                 This is a separate NETLINK thread SPAWNED by IFND. It takes
 *                 the IPXS-CB handle, along with handle from IFSV_CB(i.e., 
 *                 IFND control block).
 *                 To update IFND with latest interface information, since it
 *                 has the IFND control block access, it would post an 
 *                 event(IFSV_EVENT) to the IFND thread on the IFND mailbox.
 *                 IFND would update the database appropriately.
 *
 * Arguments     : info - IFND's control block handle, type-casted to NCSCONTEXT.
 *
 * Return Values : void
 *
 * Notes         : None.
 *****************************************************************************/
static void ifnd_netlink_process(NCSCONTEXT info)
{
   IFSV_CB             *ifsv_cb = NULL;
   unsigned long       cb_hdl = (long)info, ipxs_hdl = 0,res;
   SYSF_MBX *fd;
   NCS_SEL_OBJ_SET     readfds;
   NCS_SEL_OBJ         netlink_fd, numfds , mbx_fd;
   IPXS_CB             *ipxs_cb = NULL;
   int                 rc = 0;
   IPXS_NETLINK_RETVAL retval;
   struct sockaddr_nl  local;
   NCS_BOOL            is_full_netlink_updt_in_progress = FALSE;
   IFSV_EVT            *evt = IFSV_NULL;
   SYSF_MBX            mbx;  

   sleep(10);

   /* Take IFND control block here. */
   ifsv_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, cb_hdl);

   /* Get the IPXS CB to retrieve the netlink_fd. */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);
   if(ipxs_cb == NULL)
   {
      ncshm_give_hdl(cb_hdl);
      return;
   }

   /* create a mail box for netlink updates*/
   if ((res = m_NCS_IPC_CREATE(&ipxs_cb->mbx)) != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_CREATE_FAIL,0);
/*      goto ifnd_ipc_create_fail; */
      return;
   }

   /* store the vrid in the mailbox struct - TBD */

   if ((res = m_NCS_IPC_ATTACH(&ipxs_cb->mbx)) != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_CREATE_FAIL,0);
/*      goto ifnd_ipc_att_fail; */
      return;
   }

   ipxs_cb->netlink_updated = FALSE;

   mbx = ipxs_cb->mbx;
   mbx_fd = m_NCS_IPC_GET_SEL_OBJ(&ipxs_cb->mbx);

   fd = (SYSF_MBX *)(&ipxs_cb->netlink_fd);
   ncshm_give_hdl(cb_hdl);
   ncshm_give_hdl(ipxs_hdl);

   numfds.raise_obj = 0;
   numfds.rmv_obj = 0;
   netlink_fd = m_NCS_IPC_GET_SEL_OBJ(fd);
   m_NCS_SEL_OBJ_ZERO(&readfds);
   m_NCS_SEL_OBJ_SET(netlink_fd,&readfds);
   m_NCS_SEL_OBJ_SET(mbx_fd,&readfds);
   numfds = m_GET_HIGHER_SEL_OBJ(netlink_fd, numfds);
   m_NCS_CONS_PRINTF("Event1 numfds.rmv_obj = %d \n", numfds.rmv_obj);
   numfds = m_GET_HIGHER_SEL_OBJ(mbx_fd, numfds);
   m_NCS_CONS_PRINTF("Event2 numfds.rmv_obj = %d \n", numfds.rmv_obj);
 
   
  (void)ipxs_ifnd_gen_getv4addr_req(ipxs_cb);


   while ((rc = m_NCS_TS_SOCK_SELECT(numfds.rmv_obj+1,&readfds,0,0,NULL)) >= 0)
   {
      if(rc == 0)
      {
         /* This is EOF. This means, the netlink_fd is closed. So, the 
            thread has to terminate. */
         return;
      }
      /* process if event from ifnd regarding newly added intf */
      if (m_NCS_SEL_OBJ_ISSET(mbx_fd, &readfds))
      {
         m_NCS_CONS_PRINTF("Got an event in mailbox for new addition of intf in &ipxs_cb->mbx 0x%x\n", &ipxs_cb->mbx);
         if (IFSV_NULL != (evt = (IFSV_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(&ipxs_cb->mbx,evt)))
         {
            /* now got the IPC mail box event */
            if(evt->type == IFSV_EVT_NETLINK_LINK_UPDATE)
            {

               ipxs_hdl = m_IPXS_CB_HDL_GET( );
               ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);
/******************/
               printf("Updating socket when received event from mailbox \n");

               ipxs_ifnd_update_new_socket(ipxs_cb,&readfds,&netlink_fd,
                                     &numfds,&mbx_fd,&local);
               numfds = m_GET_HIGHER_SEL_OBJ(netlink_fd, numfds);
               numfds = m_GET_HIGHER_SEL_OBJ(mbx_fd, numfds);
               /* Mark full netlink update in progress as true */
               ipxs_cb->is_full_netlink_updt_in_progress = TRUE;
               (void)ipxs_ifnd_gen_getlink_req(ipxs_cb);

               ncshm_give_hdl(ipxs_hdl);
/******************/

            }    /* End of if evt type is netlink update */
         }       /* end of if IFSV!= NULL */
      }          /* End of IF mail box fd is set */
      /* process all the Netlink events */
      if (m_NCS_SEL_OBJ_ISSET(netlink_fd, &readfds))
      {
         m_NCS_SEL_OBJ_CLR(netlink_fd, &readfds);

         /* dispatch all the Netlink function */
         ifsv_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, cb_hdl);
         if(ifsv_cb == NULL)
         {
            printf("ifsv_cb is NULL\n");
            continue;
         }
         ipxs_hdl = m_IPXS_CB_HDL_GET( );
         ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, ipxs_hdl);
         if(ipxs_cb == NULL)
         {
            printf("ipxs_cb is NULL\n");
            ncshm_give_hdl(cb_hdl);
            continue;
         }
         is_full_netlink_updt_in_progress = ipxs_cb->is_full_netlink_updt_in_progress;
         printf("Dispatching events \n");
         retval = ipxs_netlink_dispatch(ifsv_cb, ipxs_cb);
         if(retval == IPXS_NETLINK_NEW_INTF)
         {
            printf("retval is IPXS_NETLINK_NEW_INTF\n");
            ncshm_give_hdl(ipxs_hdl);
            ncshm_give_hdl(cb_hdl);
            m_NCS_SEL_OBJ_CLR(mbx_fd, &readfds);
            m_NCS_SEL_OBJ_SET(netlink_fd,&readfds);
            m_NCS_SEL_OBJ_SET(mbx_fd,&readfds);
            continue; 
         }
         if(retval == IPXS_NETLINK_REFRESH_CACHE)
         {
            /* Compose the GETLINK request to netlink, after flushing the cache. */
            printf("Sending Request to update Links \n");
            m_NCS_MEMSET(&ipxs_cb->ifndx_cache, '\0', sizeof(ipxs_cb->ifndx_cache));
            printf("Updating socket from retval == IPXS_NETLINK_REFRESH_CACHE cond \n");
            ipxs_ifnd_update_new_socket(ipxs_cb,&readfds,&netlink_fd,
                                        &numfds,&mbx_fd,&local);
            numfds = m_GET_HIGHER_SEL_OBJ(netlink_fd, numfds);
            numfds = m_GET_HIGHER_SEL_OBJ(mbx_fd, numfds);
            (void)ipxs_ifnd_gen_getlink_req(ipxs_cb);
            printf("Sent Request to update Links \n");
         }
         if(is_full_netlink_updt_in_progress)
         {
            /* Before returning, reset the flag so that subsequent events are 
               processed normally. Also, issue a GETADDR for IPv4 so that we
               have the latest image of all addresses with us. */
            ipxs_cb->is_full_netlink_updt_in_progress = FALSE;

            /* Since no interfaces are updated in the IFND table, don't request
               for address update from the kernel. */
            m_NCS_CONS_PRINTF("About to request netlink to update addresses \n");
            if(ifsv_cb->if_tbl.n_nodes != 0)
            {
/******************/
                printf("Updating socket from ifsv_cb->if_tbl.n_nodes != 0 \n");
                ipxs_ifnd_update_new_socket(ipxs_cb,&readfds,&netlink_fd,
                                        &numfds,&mbx_fd,&local);
                numfds = m_GET_HIGHER_SEL_OBJ(netlink_fd, numfds);
                numfds = m_GET_HIGHER_SEL_OBJ(mbx_fd, numfds);
/******************/               
               (void)ipxs_ifnd_gen_getv4addr_req(ipxs_cb);
               m_NCS_CONS_PRINTF("Requested Netlink to update addresses \n");
            }
         }
         ncshm_give_hdl(ipxs_hdl);
         ncshm_give_hdl(cb_hdl);
      }
      m_NCS_SEL_OBJ_SET(netlink_fd,&readfds);
      m_NCS_SEL_OBJ_SET(mbx_fd,&readfds);
   }

   return;
}

/****************************************************************************
 * Name          : ipxs_netlink_dispatch
 *
 * Description   : This is the dispatch handler which the netlink thread of 
 *                 IFND will invoke to process netlink events.
 *
 * Arguments     : ifsv_cb - Pointer to IFND's IFSV_CB
 *                 ipxs_cb - Pointer to IPXS_CB
 *
 * Return Values : of enumeration type IPXS_NETLINK_RETVAL
 *
 * Notes         : None.
 *****************************************************************************/
static IPXS_NETLINK_RETVAL ipxs_netlink_dispatch(IFSV_CB *ifsv_cb, IPXS_CB *ipxs_cb)
{
   char buf[8192];
   struct sockaddr_nl nladdr;
   struct iovec iov = { buf, sizeof(buf) };
   int status;
   struct nlmsghdr *h = NULL; /**prev_h = NULL;*/
   struct msghdr msg = {
         (void*)&nladdr, sizeof(nladdr), &iov, 1, NULL, 0, 0
   };
   IPXS_NETLINK_RETVAL retval = IPXS_NETLINK_OK;

   memset(&buf, '\0', sizeof(buf));
   status = recvmsg(ipxs_cb->netlink_fd, &msg, 0);
   if(status < 0)
   {
      if(errno == EINTR)
         return IPXS_NETLINK_EINTR;
   }
   else if(status == 0)
   {
      return IPXS_NETLINK_EOF;
   }
   if(msg.msg_namelen != sizeof(nladdr))
   {
      /* fprintf(stderr, "sender address length = %d\n", msg.msg_namelen); */
      return IPXS_NETLINK_SENDER_ADDR_LEN_MISMATCH;
   }

   h = (struct nlmsghdr*)buf;
   while(NLMSG_OK(h, status))
   {
      retval = ipxs_post_netlink_evt_to_ifnd(&nladdr, h, ipxs_cb, ifsv_cb);
      if(retval == IPXS_NETLINK_NEW_INTF)
      {
         return retval;
      }
      if(retval == IPXS_NETLINK_REFRESH_CACHE)
      {
         /* This means, we got an address update for an interface which is not known. */
         return retval;
      }
      if(h->nlmsg_type == NLMSG_ERROR)
      {
         struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
         if(h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr)))
         {
            /*fprintf(stderr, "ERROR truncated\n");*/
         }
         else
         {
            errno = -err->error;
            /*perror("RTNETALINK answers");*/
         }
         return IPXS_NETLINK_EOF;
      }
     
 /*     prev_h = h; */

      h = NLMSG_NEXT(h, status);
      if(h->nlmsg_type == NLMSG_DONE)
      {
/*         ipxs_cb->netlink_updated = TRUE; */
         m_NCS_CONS_PRINTF("Received End of Netlink Message \n");
         return retval;
      }
   }
   ipxs_cb->netlink_updated = TRUE; 
   return IPXS_NETLINK_OK;

}

/****************************************************************************
 * Name          : ipxs_post_netlink_evt_to_ifnd
 *
 * Description   : This is the function which composes the netlink information
 *                 and posts to the IFND's mailbox.
 *
 * Arguments     : ifsv_cb - Pointer to IFND's IFSV_CB
 *                 ipxs_cb - Pointer to IPXS_CB
 *
 * Return Values : of enumeration type IPXS_NETLINK_RETVAL
 *
 * Notes         : None.
 *****************************************************************************/
static IPXS_NETLINK_RETVAL ipxs_post_netlink_evt_to_ifnd(struct sockaddr_nl *who, struct nlmsghdr *n,
                              IPXS_CB *ipxs_cb, IFSV_CB *ifsv_cb)
{
   struct ifaddrmsg    *ifa = NULL;
   struct ifinfomsg    *ifi = NULL;
   int                 len = n->nlmsg_len;
   NCS_IFSV_IFINDEX    lcl_ifindex = 0;
   char                lcl_ifname[IFSV_IF_NAME_SIZE];
   struct rtattr       *rta_tb[IFA_MAX + 1];
   IPXS_IFIP_IP_INFO   *temp_list = (IPXS_IFIP_IP_INFO *)NULL;
   NCS_IPPFX           del_ipinfo;
   IPXS_EVT            *ipxs_evt = NULL;
   uns32             rc=NCSCC_RC_SUCCESS;
   NCS_IP_ADDR       ipaddr;

   /* Check the message type */
   if((n->nlmsg_type != RTM_NEWADDR) &&
      (n->nlmsg_type != RTM_DELADDR) &&
      (n->nlmsg_type != RTM_NEWLINK) &&
      (n->nlmsg_type != RTM_DELLINK))
      return IPXS_NETLINK_OK;

   if((n->nlmsg_type == RTM_NEWLINK) ||
      (n->nlmsg_type == RTM_DELLINK))
   {
      ifi = NLMSG_DATA(n);

      /* Learn the new interfaces here, and map them to the ifindices of IFSv */
      if (n->nlmsg_len < NLMSG_LENGTH(sizeof(ifi))) {
         return IPXS_NETLINK_WRONG_NLMSG_LEN;
      }

      /* Parse the attribute */
      memset(rta_tb, '\0', sizeof(rta_tb));
      parse_rtattr(rta_tb, IFLA_MAX, IFLA_RTA(ifi), IFLA_PAYLOAD(n));

      if (rta_tb[IFLA_IFNAME] == NULL)
         return IPXS_NETLINK_WRONG_NLMSG_LEN;

      m_NCS_MEMSET(&lcl_ifname, '\0', sizeof(lcl_ifname));
      /* m_NCS_MEMCPY(&lcl_ifname, RTA_DATA(rta_tb[IFLA_IFNAME]), IFNAMSIZ); */
      m_NCS_STRCPY(&lcl_ifname, (char*)RTA_DATA(rta_tb[IFLA_IFNAME]));

      if(ifnd_ipxs_get_ifndx_for_interface_number(ifsv_cb, ipxs_cb, ifi->ifi_index,
            &lcl_ifindex, (char*)&lcl_ifname) != NCSCC_RC_SUCCESS)
      {
         /* Don't bother. IFSv doesn't know this interface yet. So, discard and continue. */
/*
         if(ipxs_cb->netlink_updated == TRUE)
         {
             This means that this is a new interface. If index 
               is not updated for it. so wait till we get an event
               from ifnd saying the interface is added to database 
            return IPXS_NETLINK_NEW_INTF;
         }
*/
         m_NCS_CONS_PRINTF("NETLINK:At this time IfSV doesn't know abt intf %s \n",lcl_ifname);
         return IPXS_NETLINK_OK;
      }
      ipxs_cb->ifndx_cache[ifi->ifi_index].interface_number = ifi->ifi_index; /* Link number */
      ipxs_cb->ifndx_cache[ifi->ifi_index].if_index = lcl_ifindex; /* IFSv specific */
      m_NCS_STRCPY(&ipxs_cb->ifndx_cache[ifi->ifi_index].ifname, &lcl_ifname);
      m_NCS_CONS_PRINTF("IfIndex allocated for interface %s is %d \n",lcl_ifname,ifi->ifi_index);

      return IPXS_NETLINK_OK;
   }

   if((n->nlmsg_type == RTM_NEWADDR) ||
      (n->nlmsg_type == RTM_DELADDR))
   {
      ifa = NLMSG_DATA(n);
      m_NCS_CONS_PRINTF("We Are In Second Comparision!!!! \n");

      /* Sanity check */
      len -= NLMSG_LENGTH(sizeof(*ifa));
      if(len < 0)
      {
         m_NCS_CONS_PRINTF("IPXS_NETLINK_WRONG_NLMSG_LEN \n");
         return IPXS_NETLINK_WRONG_NLMSG_LEN;
      }

      /* Check whether it is IPv4 */
      if(ifa->ifa_family != AF_INET)
      {
         m_NCS_CONS_PRINTF("Address is not IPv4 \n");
         return IPXS_NETLINK_OK;
      }

      if((ipxs_cb->ifndx_cache[ifa->ifa_index].interface_number == 0) ||
         (ipxs_cb->ifndx_cache[ifa->ifa_index].if_index == 0))
      {
         /* We haven't learnt about this interface yet. So, discard this event,
            and request for a full update of GET_LINK and GET_ADDR from netlink. */
         if(ipxs_cb->is_full_netlink_updt_in_progress)
         {
            m_NCS_CONS_PRINTF("Full update in progress \n");
            return IPXS_NETLINK_OK;
         }
         if(ifsv_cb->ifd_card_up == FALSE)
         {
            m_NCS_CONS_PRINTF("IfD Still not up \n");
            /* It makes sense to request for IF updates from kernel only when IFD is up. */
            return IPXS_NETLINK_OK;
         }
         if(ipxs_cb->netlink_updated == TRUE)
         {
            /* By this time we already learnt interfaces so no refresh */ 
            m_NCS_CONS_PRINTF("New Addr Added on a Newly Created Intf %s :\n", ipxs_cb->ifndx_cache[ifa->ifa_index].ifname);
            return IPXS_NETLINK_NEW_INTF;
         }
         m_NCS_CONS_PRINTF("marking full netlink update in progress : True \n");
         ipxs_cb->is_full_netlink_updt_in_progress = TRUE;
         return IPXS_NETLINK_REFRESH_CACHE;
      }

      /* Parse the attribute */
      memset(rta_tb, '\0', sizeof(rta_tb));
      parse_rtattr(rta_tb, IFA_MAX, IFA_RTA(ifa), len);

      if(!rta_tb[IFA_LOCAL])
         rta_tb[IFA_LOCAL] = rta_tb[IFA_ADDRESS];

      if(rta_tb[IFA_LOCAL])
      {
         u_char *src = RTA_DATA(rta_tb[IFA_LOCAL]);
         uint32_t lcl_addr = 0;

         /* NCS can use a better way of getting the address in host-order !!! */
         lcl_addr = (src[0]<<24) + (src[1] << 16) + (src[2] << 8) + src[3];
         ipaddr.type = NCS_IP_ADDR_TYPE_IPV4;
         ipaddr.info.v4 = lcl_addr;

         if(n->nlmsg_type == RTM_NEWADDR)
         { 
            m_NCS_LOCK(&ipxs_cb->ipxs_db_lock, NCS_LOCK_READ);
            if(ifnd_ipxs_proc_is_ipaddr(ipxs_cb, ipaddr))
            {
               /* IP Address already exists, so return from here.*/
               m_NCS_CONS_PRINTF("IP address already exists: %s %d \n",__FILE__,__LINE__);
               m_NCS_UNLOCK(&ipxs_cb->ipxs_db_lock, NCS_LOCK_READ);
               return NCSCC_RC_FAILURE; 
            }
            m_NCS_UNLOCK(&ipxs_cb->ipxs_db_lock, NCS_LOCK_READ);
         }
         
         temp_list = ncs_ipxs_ippfx_list_alloc(1);
         m_NCS_MEMSET(temp_list, '\0', sizeof(IPXS_IFIP_IP_INFO));
         m_NCS_MEMSET(&del_ipinfo, '\0', sizeof(NCS_IPPFX));

         temp_list->ipaddr.ipaddr.type = NCS_IP_ADDR_TYPE_IPV4;
         temp_list->ipaddr.ipaddr.info.v4 = lcl_addr;
         temp_list->ipaddr.mask_len = ifa->ifa_prefixlen;

         del_ipinfo.ipaddr.type = NCS_IP_ADDR_TYPE_IPV4;
         del_ipinfo.ipaddr.info.v4 = lcl_addr;
         del_ipinfo.mask_len = ifa->ifa_prefixlen;

         if((ipxs_evt = m_MMGR_ALLOC_IPXS_EVT) == NULL)
         {
            ncs_ipxs_ippfx_list_free(temp_list);
            m_NCS_CONS_PRINTF("IPXS Evt allocation failed !!!! \n");
            return IPXS_NETLINK_ALLOC_IPXS_EVT_FAIL;
         }

         m_NCS_MEMSET(ipxs_evt, '\0', sizeof(IPXS_EVT));

         ipxs_evt->netlink_msg = TRUE;
         ipxs_evt->info.nd.atond_upd.if_index = ipxs_cb->ifndx_cache[ifa->ifa_index].if_index;

         m_NCS_IPXS_IPAM_ADDR_SET(ipxs_evt->info.nd.atond_upd.ip_info.ip_attr);
         if(n->nlmsg_type == RTM_NEWADDR)
         {
            ipxs_evt->info.nd.atond_upd.ip_info.addip_cnt ++;
            ipxs_evt->info.nd.atond_upd.ip_info.addip_list = temp_list;
         }
         else
         {
            ipxs_evt->info.nd.atond_upd.ip_info.delip_cnt ++;
            ipxs_evt->info.nd.atond_upd.ip_info.delip_list = &del_ipinfo;
         }
         
         m_NCS_CONS_PRINTF("Sending IP ADDR UPDATE \n");
/*       m_NCS_CONS_PRINTF("Sending IP addr update to ifnd for ip : %x \n",temp_list->ipaddr.ipaddr.info.v4); */
         /* Displaying the decimal format of the IP address */
         m_NCS_CONS_PRINTF("Sending IP addr update to ifnd for ip : %d.%d.%d.%d \n",((temp_list->ipaddr.ipaddr.info.v4)&(0xff000000))>>24,
                                                                                    ((temp_list->ipaddr.ipaddr.info.v4)&(0x00ff0000))>>16,
                                                                                    ((temp_list->ipaddr.ipaddr.info.v4)&(0x0000ff00))>>8,
                                                                                    (temp_list->ipaddr.ipaddr.info.v4)&(0x000000ff));
         if((rc = ifnd_ipxs_proc_ifip_upd(ipxs_cb, ipxs_evt, NULL))
            == NCSCC_RC_FAILURE)
         {
            m_NCS_CONS_PRINTF("Ip addition failed %s %d \n",__FILE__,__LINE__);
            ncs_ipxs_ippfx_list_free(temp_list);
            m_MMGR_FREE_IPXS_EVT(ipxs_evt);
            return IPXS_NETLINK_SEND_IFSV_EVT_TO_MBX_FAIL;
         }
         else
         if( ipxs_evt != NULL )
         {
             ncs_ipxs_ippfx_list_free(temp_list);
             m_MMGR_FREE_IPXS_EVT( ipxs_evt );
         }
      }
   }

   return IPXS_NETLINK_OK;
}
#endif /* #if(NCS_IFSV_USE_NETLINK == 1) */

/*****************************************************************************
  PROCEDURE NAME    :   ifnd_ipxs_proc_ifip_info
  DESCRIPTION       :   Function to process the interface & IP information 
                        received from IFD.
*****************************************************************************/
static uns32 ifnd_ipxs_proc_ifip_info(IPXS_CB *cb, IPXS_EVT *ipxs_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_IPXS_IPINFO *rcv_ifip_info;
   m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,\
                     "Got IFIP info from IfD, index",\
                     ipxs_evt->info.nd.dtond_upd.if_index);

   rc = ifnd_ipxs_data_proc_ifip_info(cb, ipxs_evt, &rcv_ifip_info);

   if(rc != NCSCC_RC_SUCCESS)
      return rc;

   /* Broadcast the IP info to all the agents */
   rc = ipxs_ifnd_ifip_info_bcast(cb, 
              ipxs_evt->info.nd.dtond_upd.if_index, rcv_ifip_info);
                   
  return rc;
}

/****************************************************************************
 * Name          : ifnd_ipxs_data_proc_ifip_info
 *
 * Description   : This function updates the IfND data base.
 *
 * Arguments     : cb - Ifnd Control Block.
 *                 ipxs_evt - IPXS event pointer
 *                 rcv_ifip_info - Pointer to the IPINFO.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_ipxs_data_proc_ifip_info (IPXS_CB *cb, IPXS_EVT *ipxs_evt, 
                               NCS_IPXS_IPINFO **rcv_ifip_info)
{
   uns32 rc = NCSCC_RC_SUCCESS, i = 0;
   IPXS_IFIP_NODE  *ifip_node;
   NCS_BOOL    add_flag=FALSE, del_flag=FALSE;
   NCS_IPXS_IPINFO *temp_ptr = NULL;
   IPXS_IFIP_IP_INFO *updated_ipaddr_ptr = NULL;
   uns32             updated_ipaddr_counter = 0;
#if (NCS_VIP == 1)
   uns32 ii = 0;
   IPXS_IFIP_IP_INFO   *pIfipIpInfo;
#endif
 
   *rcv_ifip_info = &ipxs_evt->info.nd.dtond_upd.ip_info;
   temp_ptr = *rcv_ifip_info;

   ifip_node = ipxs_ifip_record_get(&cb->ifip_tbl, ipxs_evt->info.nd.dtond_upd.if_index);

   if(ifip_node)
   {
#if (NCS_VIP == 1)
      if((m_NCS_IPXS_IS_IPAM_VIP_REFCNT_SET(temp_ptr->ip_attr)) &&
             (m_NCS_IPXS_IS_IPAM_VIP_SET(temp_ptr->ip_attr))) 
 
      {
         for ( ii = 0; ii < ifip_node->ifip_info.ipaddr_cnt ; ii ++)
         { 
            pIfipIpInfo = &ifip_node->ifip_info.ipaddr_list[ii];

            if (( m_NCS_MEMCMP(&pIfipIpInfo->ipaddr,&temp_ptr->addip_list->ipaddr,
                                        sizeof(NCS_IPPFX)))== 0)
            {
               if(temp_ptr->addip_list->refCnt == 0)
               {
                  pIfipIpInfo->refCnt--;
               }
               else if(temp_ptr->addip_list->refCnt == 1)
               {
                  pIfipIpInfo->refCnt++;
               }
            }           /* end of MEMCMP */
         }              /* end of for loop */
      }                 /* end of REF_CNT_SET */
#endif

      /* Update the database with received information */
      if(m_NCS_IPXS_IS_IPAM_UNNMBD_SET(temp_ptr->ip_attr))
      {
         m_NCS_IPXS_IPAM_UNNMBD_SET(ifip_node->ifip_info.ipam);
         if(ifip_node->ifip_info.is_v4_unnmbrd == NCS_SNMP_TRUE)
         {
            /* This means that there is no IP address on this intf. */ 
            ifip_node->ifip_info.is_v4_unnmbrd = temp_ptr->is_v4_unnmbrd;
         }
         else if(ifip_node->ifip_info.is_v4_unnmbrd == NCS_SNMP_FALSE)
         {
           /* This means that there may or may not be IP address assinged. 
              So, if assigned then delete all the interfaces.*/
           if(temp_ptr->is_v4_unnmbrd == NCS_SNMP_TRUE)
              ipxs_ip_record_list_del(cb, &ifip_node->ifip_info);
           ifip_node->ifip_info.is_v4_unnmbrd = temp_ptr->is_v4_unnmbrd;
         }
      }

      if((m_NCS_IPXS_IS_IPAM_ADDR_SET(temp_ptr->ip_attr))
#if (NCS_VIP == 1)
         || ((m_NCS_IPXS_IS_IPAM_VIP_SET(temp_ptr->ip_attr)) && 
            (!m_NCS_IPXS_IS_IPAM_VIP_REFCNT_SET(temp_ptr->ip_attr)))
#endif
         )
      {
       if(ifip_node->ifip_info.is_v4_unnmbrd == NCS_SNMP_FALSE)
       { 
         /* This means that we are allowed to add the interfaces. */
         m_NCS_IPXS_IPAM_ADDR_SET(ifip_node->ifip_info.ipam);
         if(temp_ptr->addip_cnt)
         {
           /* We have to send update for those IP addresses, which has been added. */
           updated_ipaddr_ptr = 
              (IPXS_IFIP_IP_INFO *)m_MMGR_ALLOC_IPXS_DEFAULT((temp_ptr->addip_cnt)*sizeof(IPXS_IFIP_IP_INFO));  
           if(updated_ipaddr_ptr != NULL)
             m_NCS_OS_MEMSET(updated_ipaddr_ptr, 0 , (temp_ptr->addip_cnt)*sizeof(IPXS_IFIP_IP_INFO));   
           else
           {
             return NCSCC_RC_FAILURE;
           }

           for(i = 0; i < temp_ptr->addip_cnt; i++)
            if(!ifnd_ipxs_proc_is_ipaddr(cb, temp_ptr->addip_list[i].ipaddr.ipaddr))
            {
              /* Add IP address to ifip info */
              rc = ipxs_ifip_ippfx_add(cb, &ifip_node->ifip_info, 
                        &temp_ptr->addip_list[i],
                        1 , &add_flag);
              if(rc == NCSCC_RC_SUCCESS)
              {
               m_NCS_OS_MEMCPY(&updated_ipaddr_ptr[updated_ipaddr_counter], &temp_ptr->addip_list[i], 
                               sizeof(IPXS_IFIP_IP_INFO));
               updated_ipaddr_counter++;
              } 
            }
            if(ipxs_evt->type == IPXS_EVT_DTOND_IFIP_INFO)
            {
               if(ipxs_evt->netlink_msg == FALSE)
               {
               rc = ipxs_ifip_ippfx_add_to_os(cb, ipxs_evt->info.nd.dtond_upd.if_index, temp_ptr->addip_list, temp_ptr->addip_cnt);
               }
            }
            /* Now send update for those ip addresses, which got added */
             if(temp_ptr->addip_list)
             {
              m_MMGR_FREE_IPXS_DEFAULT(temp_ptr->addip_list);
              temp_ptr->addip_list = NULL;
             }

             temp_ptr->addip_cnt = updated_ipaddr_counter;

             if(updated_ipaddr_counter != 0)
               temp_ptr->addip_list = updated_ipaddr_ptr;
             else
               m_MMGR_FREE_IPXS_DEFAULT(updated_ipaddr_ptr);
             
         }

         if(rc != NCSCC_RC_SUCCESS)
         {
            return rc;
         }
      
         if(temp_ptr->delip_cnt)
         {
            for(i = 0; i < temp_ptr->delip_cnt; i++)
            /* Delete IP address to ifip info */
            rc = ipxs_ifip_ippfx_del(cb, &ifip_node->ifip_info, 
                        (IPXS_IFIP_IP_INFO *)&temp_ptr->delip_list[i],
                        1, &del_flag);
            if(ipxs_evt->type == IPXS_EVT_DTOND_IFIP_INFO)
            {
               if(ipxs_evt->netlink_msg == FALSE)
               {
               rc = ipxs_ifip_ippfx_del_from_os(cb, ipxs_evt->info.nd.dtond_upd.if_index,
                        temp_ptr->delip_list,
                        temp_ptr->delip_cnt);
               }
            }
         }
      }/* if(ifip_node->ifip_info.is_v4_unnmbrd == NCS_SNMP_FALSE)  */
     }

     if(m_NCS_IPXS_IS_IPAM_RRTRID_SET(temp_ptr->ip_attr))
     {
        m_NCS_IPXS_IPAM_RRTRID_SET(ifip_node->ifip_info.ipam);
        ifip_node->ifip_info.rrtr_rtr_id = temp_ptr->rrtr_rtr_id;
     }

     if(m_NCS_IPXS_IS_IPAM_RMT_AS_SET(temp_ptr->ip_attr))
     {
        m_NCS_IPXS_IPAM_RMT_AS_SET(ifip_node->ifip_info.ipam);
        ifip_node->ifip_info.rrtr_as_id = temp_ptr->rrtr_as_id;
     }

     if(m_NCS_IPXS_IS_IPAM_RMTIFID_SET(temp_ptr->ip_attr))
     {
        m_NCS_IPXS_IPAM_RMTIFID_SET(ifip_node->ifip_info.ipam);
        ifip_node->ifip_info.rrtr_if_id = temp_ptr->rrtr_if_id;
     }


     if(m_NCS_IPXS_IS_IPAM_UD_1_SET(temp_ptr->ip_attr))
     {
        m_NCS_IPXS_IPAM_UD_1_SET(ifip_node->ifip_info.ipam);
        ifip_node->ifip_info.ud_1 = temp_ptr->ud_1;
     }

#if (NCS_VIP == 1)
     if((m_NCS_IPXS_IS_IPAM_VIP_SET(temp_ptr->ip_attr)) &&
            (!m_NCS_IPXS_IS_IPAM_VIP_REFCNT_SET(temp_ptr->ip_attr)))
     {
        m_NCS_STRCPY(&ifip_node->ifip_info.intfName,&temp_ptr->intfName);
        ifip_node->ifip_info.shelfId = temp_ptr->shelfId;
        ifip_node->ifip_info.slotId = temp_ptr->slotId;
        /* embedding subslot changes */
        ifip_node->ifip_info.subslotId = temp_ptr->subslotId;
        ifip_node->ifip_info.nodeId = temp_ptr->nodeId;
     }
#endif


     /* The UD value is always sent to the client irrespective
        of modification - VPN Requirement */
     m_NCS_IPXS_IPAM_UD_1_SET(temp_ptr->ip_attr);
     temp_ptr->ud_1 =  ifip_node->ifip_info.ud_1;
   } /* if(ifip_node) */      
   else
   {
     m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,"Coudn't get IFIP rec",0);
     rc = NCSCC_RC_FAILURE;
   }
     return rc;
} /* function ifnd_ipxs_data_proc_ifip_info() ends here */


/****************************************************************************
 * Name          : ifnd_ipxs_proc_is_ipaddr
 *
 * Description   : This function checks whether the IP address being passed is 
 *                 assigned to any interface or not.
 *
 * Arguments     : cb - pointer to the control block.
 *                 ip addr - IP address
 *                 
 * Return Values : 0 if no IP addr else intf index 
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_ipxs_proc_is_ipaddr(IPXS_CB *cb, NCS_IP_ADDR ipxs_ipaddr)
{
  IPXS_IFIP_NODE    *ifip_node = NULL;
  IPXS_IFIP_INFO    *ifip_info = NULL;
  uns32 i = 0;

  /* Get the first node */
  ifip_node = (IPXS_IFIP_NODE *)ncs_patricia_tree_getnext
                          (&cb->ifip_tbl, (uns8*)NULL);

  while(ifip_node)
  {
      ifip_info = &ifip_node->ifip_info;

      for(i=0; i < ifip_info->ipaddr_cnt; i++)
      {
         /* Check agenest full address */
         if(m_NCS_CMP_IP_ADDR(&ifip_info->ipaddr_list[i].ipaddr.ipaddr,&ipxs_ipaddr)
                                == 0)
         {
              return (ifip_info->ifindexNet);
         }
      }

      /* Get the first node */
      ifip_node = (IPXS_IFIP_NODE *)ncs_patricia_tree_getnext
                          (&cb->ifip_tbl, (uns8*)&ifip_info->ifindexNet);
  } /* while  */ 

 return 0;

} /* End of the function ifnd_ipxs_proc_is_ipaddr() */

uns32 ifnd_ipxs_proc_ifa_app_add(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                         IFSV_SEND_INFO *sinfo)
{
   IPXS_EVT          send_evt;
   IFSV_EVT          evt;
   uns32             rc=NCSCC_RC_SUCCESS;
   uns32             ifindex=0;
   IPXS_IFIP_NODE    *ifip_node=0;
   NCS_IPXS_INTF_REC *cur_rec=0, *prev_rec=0;
   
   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IPXS_EVT));
   

   /* Fill the pointers */
   evt.info.ipxs_evt = (NCSCONTEXT)&send_evt;
   evt.type = IFSV_IPXS_EVT;

   send_evt.usrhdl = ipxs_evt->info.nd.get.usr_hdl;

   send_evt.type = IPXS_EVT_NDTOA_IFIP_ADD;

   ifindex = 0;
   ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                                  (&cb->ifip_tbl, (uns8*)&ifindex);
   if(ifip_node == 0)
   {
      /* No record */
      return rc;
   }
   while(ifip_node)
   {
      /* Malloc the IPXS interface rec */
      cur_rec = m_MMGR_ALLOC_NCS_IPXS_INTF_REC;   /* TBD Mem checks */
      if(cur_rec == NULL)
      {
        m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
        return NCSCC_RC_FAILURE;
      }
      m_NCS_OS_MEMSET(cur_rec, 0, sizeof(NCS_IPXS_INTF_REC));
      cur_rec->if_index = ifip_node->ifip_info.ifindexNet;
      ifindex = ifip_node->ifip_info.ifindexNet;

      /* Fill the IP information */
      rc = ipxs_ifsv_ifip_info_attr_cpy(&ifip_node->ifip_info, &cur_rec->ip_info);

      if(rc == NCSCC_RC_FAILURE)
         goto free_mem;

      /* Fill the Interface information */
      rc = ipxs_ifnd_if_info_attr_cpy(cb, ifindex, cur_rec);
      if(rc == NCSCC_RC_FAILURE)
         goto free_mem;
      

      if(prev_rec == 0)
      {
         send_evt.info.agent.ip_add.i_ipinfo = cur_rec;
      }
      else
      {
         prev_rec->next = cur_rec;
      }
      prev_rec = cur_rec;

      ifip_node = (IPXS_IFIP_NODE*)ncs_patricia_tree_getnext
                                  (&cb->ifip_tbl, (uns8*)&ifindex);
   }
   
   rc = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, (NCSCONTEXT)&evt, 
             sinfo->dest, sinfo->to_svc);
free_mem:
   ipxs_intf_rec_list_free(send_evt.info.agent.ip_add.i_ipinfo);
   /* RSR:TBD Free Mem */

   return rc;
}


/*****************************************************************************
  PROCEDURE NAME    :   ifnd_ipxs_proc_get_ifip_req
  DESCRIPTION       :   Function to process the interface & IP information 
                        received from IFD.
*****************************************************************************/
static uns32 ifnd_ipxs_proc_get_ifip_req(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                         IFSV_SEND_INFO *sinfo)
{
   uns32          rc;
   m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,\
                     "Got IFIP get req from IfA, usr_hdl : ",\
                     ipxs_evt->info.nd.get.usr_hdl);

   if(ipxs_evt->info.nd.get.get_type == IFSV_INTF_GET_ALL)
   {
      rc = ifnd_ipxs_proc_ifa_app_add(cb, ipxs_evt, sinfo);
   }
   else  /* Send the single record */
   {
      NCS_IFSV_IFINDEX  if_index=0;
      IPXS_IFIP_NODE *ifip_node=0;
      IPXS_EVT          send_evt;
      IFSV_EVT          evt;
      NCS_IPXS_INTF_REC send_rec;

      m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
      m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IPXS_EVT));
      m_NCS_OS_MEMSET(&send_rec, 0, sizeof(NCS_IPXS_INTF_REC));

      /* Fill the pointers */
      evt.info.ipxs_evt = (NCSCONTEXT)&send_evt;
      send_evt.info.agent.get_rsp.ipinfo = &send_rec;

      evt.type = IFSV_IPXS_EVT;
      send_evt.usrhdl = ipxs_evt->info.nd.get.usr_hdl;

      send_evt.type = IPXS_EVT_NDTOA_IFIP_RSP;
   
      ifnd_ipxs_get_ifindex_from_key(cb, &ipxs_evt->info.nd.get.key, &if_index);

      /* Get the IF-IP Node */
      if(if_index && cb->ifip_tbl_up)
         ifip_node = ipxs_ifip_record_get(&cb->ifip_tbl, if_index);


      /* Fill the rsp */
      if(ifip_node)
      {
         send_evt.info.agent.get_rsp.err = NCS_IFSV_IFGET_RSP_SUCCESS;
         send_rec.if_index = if_index;

         /* Fill the IP information */
         ipxs_ifsv_ifip_info_attr_cpy(&ifip_node->ifip_info, &send_rec.ip_info);

         /* Fill the IF & SPT info */
         rc = ipxs_ifnd_if_info_attr_cpy(cb, if_index, &send_rec);
      }
      else
         send_evt.info.agent.get_rsp.err = NCS_IFSV_IFGET_RSP_NO_REC;

      /* Send the resp to IFA */
      if(sinfo->stype == MDS_SENDTYPE_RSP)
      {
         /* Sync resp */
         rc = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, sinfo, &evt);
      }
      else
      {
         rc = ifsv_mds_normal_send(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, (NCSCONTEXT)&evt, 
             sinfo->dest, sinfo->to_svc);
      }
      if(ifip_node)
      {
        if(send_rec.ip_info.addip_cnt)
        {
          if(send_rec.ip_info.addip_list)
             ncs_ipxs_ippfx_list_free((void *)send_rec.ip_info.addip_list);
        }
      }
   }

   return rc;
}


static uns32 ifnd_ipxs_node_rec_get(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                         IFSV_SEND_INFO *sinfo)
{
   IPXS_EVT          send_evt;
   IFSV_EVT          evt;
    uns32            rc;
   m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,\
                  "Got IPXS_EVT_ATOND_NODE_REC_GET event from IfA",\
                   0);

   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IPXS_EVT));
 
   /* Fill the pointers */
   evt.info.ipxs_evt = (NCSCONTEXT)&send_evt;

   evt.type = IFSV_IPXS_EVT;
   send_evt.type = IPXS_EVT_NDTOA_NODE_REC;

   send_evt.info.agent.node_rec = cb->node_info;

   /* Sync resp */
   rc = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, sinfo, &evt);

   return rc;
}


static uns32 ifnd_ipxs_proc_isloc(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                         IFSV_SEND_INFO *sinfo)
{
   IPXS_IFIP_NODE    *ifip_node;
   IPXS_IFIP_INFO    *ifip_info;
   NCS_IPXS_ISLOCAL  *is_loc = 0;
   IPXS_EVT_ISLOCAL  *rcv_is_loc=0;
   uns32             i=0, rc;
   IPXS_EVT          send_evt;
   IFSV_EVT          evt;
   uns32             in_v4mask;
   
   rcv_is_loc = &ipxs_evt->info.nd.is_loc;
   m_IFND_LOG_EVT_LL(IFSV_LOG_IFND_IPXS_EVT_INFO,\
       "Got IPXS_EVT_ATOND_IS_LOCAL event from IfA, ipaddr type and V4 : ",\
                     rcv_is_loc->ip_addr.type,rcv_is_loc->ip_addr.info.v4);

   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IPXS_EVT));

   is_loc = &send_evt.info.agent.isloc_rsp;
   is_loc->o_answer = FALSE;
   is_loc->o_iface = 0;

   /* Get the first node */
   ifip_node = (IPXS_IFIP_NODE *)ncs_patricia_tree_getnext
                          (&cb->ifip_tbl, (uns8*)NULL);

   while(ifip_node)
   {
      ifip_info = &ifip_node->ifip_info;

      for(i=0; i < ifip_info->ipaddr_cnt; i++)
      {
        if(rcv_is_loc->obs)
        {
           /* Check agenest full address */
           if(m_NCS_CMP_IP_ADDR(&ifip_info->ipaddr_list[i].ipaddr.ipaddr, &rcv_is_loc->ip_addr) == 0)
           {
               is_loc->o_answer = TRUE;
               is_loc->o_iface = ifip_info->ifindexNet;
               break;
           }
        }
        else
        {
            if(ifip_info->ipaddr_list[i].ipaddr.ipaddr.type == NCS_IP_ADDR_TYPE_IPV4)
            {
               in_v4mask = (0xffffffff << (32 - rcv_is_loc->maskbits));
               if((ifip_info->ipaddr_list[i].ipaddr.ipaddr.info.v4 & in_v4mask) ==
                  (rcv_is_loc->ip_addr.info.v4 & in_v4mask))
               {
                  is_loc->o_answer = TRUE;
                  is_loc->o_iface = ifip_info->ifindexNet;
                  break;
               }
            }
         }
      }

      if(i < ifip_info->ipaddr_cnt)
         break;

      /* Get the first node */
      ifip_node = (IPXS_IFIP_NODE *)ncs_patricia_tree_getnext
                          (&cb->ifip_tbl, (uns8*)&ifip_info->ifindexNet);
   }

   /* Fill the pointers */
   evt.info.ipxs_evt = (NCSCONTEXT)&send_evt;

   evt.type = IFSV_IPXS_EVT;
   send_evt.type = IPXS_EVT_NDTOA_ISLOC_RSP;


   /* Sync resp */
   rc = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, sinfo, &evt);

   return rc;
}


uns32 ifnd_ipxs_proc_ifip_upd(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                     IFSV_SEND_INFO *sinfo)
{
   NCS_IPXS_INTF_REC *intf_rec;
   IPXS_EVT          send_evt, *o_ipxs_evt = NULL;
   IFSV_EVT          evt, *o_evt = NULL;
   IFSV_CB           *ifsv_cb;
   uns32             ifsv_hdl, rc;
   uns32 error = NCS_IFSV_NO_ERROR;
   NCS_IPXS_INTF_REC ip_rec;

   ifsv_hdl = m_IFND_CB_HDL_GET();

   ifsv_cb = (IFSV_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFND, ifsv_hdl);

   if(ifsv_cb == NULL)
   {
      m_IFND_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,IFSV_COMP_IFND);
      return (NCSCC_RC_FAILURE);
   }
   intf_rec = &ipxs_evt->info.nd.atond_upd;
   
   m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,\
                     "Got IFIP up event from IfA, index : ",\
                     intf_rec->if_index);

   /* Take care of delete IP Address messages from NET LINK in switch over case
     when IFD is not available */
   if((ifsv_cb->ifd_card_up == FALSE) &&
      (sinfo == NULL)) /* sinfo is NULL means the message is comming from NET LINK*/
   {
     if((m_NCS_IPXS_IS_IPAM_ADDR_SET(intf_rec->ip_info.ip_attr)) && (intf_rec->ip_info.delip_cnt != 0) &&
         (intf_rec->ip_info.delip_list != NULL))
     {
/*       m_NCS_CONS_PRINTF("ifnd_ipxs_proc_ifip_upd : delip_list %x \n", intf_rec->ip_info.delip_list[0].ipaddr.info.v4); */
         m_NCS_CONS_PRINTF("ifnd_ipxs_proc_ifip_upd - delip_list : %d.%d.%d.%d \n", ((intf_rec->ip_info.delip_list[0].ipaddr.info.v4)&(0xff000000))>>24,
                                                                                    ((intf_rec->ip_info.delip_list[0].ipaddr.info.v4)&(0x00ff0000))>>16,
                                                                                    ((intf_rec->ip_info.delip_list[0].ipaddr.info.v4)&(0x0000ff00))>>8,
                                                                                    (intf_rec->ip_info.delip_list[0].ipaddr.info.v4)&(0x000000ff));        
       /* Store the IP Addresses in an array */
       cb->nl_addr_info.list[cb->nl_addr_info.num_ip_addr].if_index = intf_rec->if_index;
       m_NCS_OS_MEMCPY(&(cb->nl_addr_info.list[cb->nl_addr_info.num_ip_addr].ippfx),
                       intf_rec->ip_info.delip_list, sizeof(NCS_IPPFX));
       cb->nl_addr_info.num_ip_addr ++;
       goto end;
     }
   }


   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IPXS_EVT));
 
   /* Fill the pointers */
   evt.info.ipxs_evt = (NCSCONTEXT)&send_evt;

   evt.type = IFSV_IPXS_EVT;
   send_evt.type = IPXS_EVT_NDTOD_IFIP_INFO;
   send_evt.netlink_msg = ipxs_evt->netlink_msg;

   
   send_evt.info.d.ndtod_upd.if_index = intf_rec->if_index;

   /* Copy the IP info, and its internal pointers */
   m_NCS_OS_MEMCPY(&send_evt.info.d.ndtod_upd.ip_info, 
                   &intf_rec->ip_info, sizeof(NCS_IPXS_IPINFO));

    /* send the information to the IfD */
    rc = ifsv_mds_msg_sync_send(ifsv_cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                              NCSMDS_SVC_ID_IFD, ifsv_cb->ifd_dest,
                              (NCSCONTEXT)&evt, (NCSCONTEXT)&o_evt,
                              IFSV_MDS_SYNC_TIME);
    
   if(rc != NCSCC_RC_SUCCESS)
   {
     if(rc == NCSCC_RC_FAILURE)
        error = NCS_IFSV_EXT_ERROR;
     else
        error = NCS_IFSV_SYNC_TIMEOUT;
     /* We could not get the index. */
     rc = NCSCC_RC_FAILURE;
   }
   else
   {
      o_ipxs_evt = o_evt->info.ipxs_evt;
      if(o_ipxs_evt->error == NCS_IFSV_NO_ERROR)
      {
        /* If no error, then no problem, the sync resp will be sent at the end*/
        error = NCS_IFSV_NO_ERROR;
      }
      else
      {
        /* We could not get the correct response. So, send the error
           back to IfA. */
         error = o_ipxs_evt->error;
         rc = NCSCC_RC_FAILURE;
      }

         if( o_ipxs_evt != NULL )
         {
            m_MMGR_FREE_IPXS_EVT(o_ipxs_evt);
            o_ipxs_evt = NULL;
         }
          /**************FREEING o_evt after sync Call *****/
         if( o_evt != NULL )
         {
            m_MMGR_FREE_IFSV_EVT(o_evt);
            o_evt = NULL;
         }
   }

   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IPXS_EVT));
   m_NCS_OS_MEMSET(&ip_rec, 0, sizeof(NCS_IPXS_INTF_REC));

   /* Fill the pointers */
   evt.info.ipxs_evt = (NCSCONTEXT)&send_evt; 
   send_evt.info.agent.ip_upd.i_ipinfo = &ip_rec; 

   evt.type = IFSV_IPXS_EVT;
   send_evt.type = IPXS_EVT_NDTOA_IFIP_UPD_RESP;
   send_evt.error = error;
   ip_rec.if_index = intf_rec->if_index;
   ip_rec.ip_info = intf_rec->ip_info;

end :

   /* Sync resp */
   if(sinfo ==  NULL)
   {
      rc = NCSCC_RC_SUCCESS;
   }
   else
      rc = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFND, sinfo, &evt);

   if(rc != NCSCC_RC_SUCCESS)
   {
      ncshm_give_hdl(ifsv_hdl);
      /* ipxs_ipinfo_free(&send_evt.info.d.ndtod_upd.ip_info, IFSV_MALLOC_FOR_MDS_SEND);*/
      return rc;
   }

   ncshm_give_hdl(ifsv_hdl);

   return NCSCC_RC_SUCCESS;
}

static uns32 ipxs_ifnd_ifip_info_bcast(IPXS_CB *cb, uns32 if_index,
                                                  NCS_IPXS_IPINFO *ip_info)
{
   IPXS_EVT          ipxs_evt;
   IFSV_EVT          evt;
   NCS_IPXS_INTF_REC ip_rec;
   uns32             rc;
   IFSV_INTF_DATA *data;
   uns32   ifsv_hdl;
   IFSV_CB *ifsv_cb;
   
   ifsv_hdl = m_IFSV_GET_HDL(0, 0);

   if ((ifsv_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFND, ifsv_hdl))
      == 0)
   {
      m_IFND_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,IFSV_COMP_IFND);
      return (NCSCC_RC_FAILURE);
   }

   data = ifsv_intf_rec_find(if_index, ifsv_cb);

   if(data == 0)
   {
      ncshm_give_hdl(ifsv_hdl);
      return NCSCC_RC_FAILURE;
   }
   

   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&ipxs_evt, 0, sizeof(IPXS_EVT));
   m_NCS_OS_MEMSET(&ip_rec, 0, sizeof(NCS_IPXS_INTF_REC));

   evt.info.ipxs_evt = (NCSCONTEXT)&ipxs_evt;
   ipxs_evt.info.agent.ip_upd.i_ipinfo = &ip_rec;

   evt.type = IFSV_IPXS_EVT;
   
   ipxs_evt.type = IPXS_EVT_NDTOA_IFIP_UPD;
   /* Is this interface is local to this node or Not */
   if(m_NCS_NODE_ID_FROM_MDS_DEST(data->current_owner_mds_destination) == 
       m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest))
     ip_rec.is_lcl_intf = TRUE;
   else
     ip_rec.is_lcl_intf = FALSE;

   ncshm_give_hdl(ifsv_hdl);
   

   ip_rec.if_index = if_index;
   /* Copying the IP Info & its internal pointers */
   ip_rec.ip_info = *ip_info; 
   

   /* DO MDS broadcast to intra node IFAs */
   rc = ifsv_mds_scoped_send(cb->my_mds_hdl, NCSMDS_SCOPE_INTRANODE, 
              NCSMDS_SVC_ID_IFND, &evt, NCSMDS_SVC_ID_IFA);

   /* Free the internal pointers of the received info */
   if(ip_info->addip_list)
      m_MMGR_FREE_IPXS_DEFAULT(ip_info->addip_list);

   if(ip_info->delip_list)
      m_MMGR_FREE_IPXS_DEFAULT(ip_info->delip_list);

   return rc;
}

static void ifnd_ipxs_get_ifindex_from_spt(NCS_IFSV_SPT *spt, NCS_IFSV_IFINDEX *if_index)
{
   IFSV_CB  *ifsv_cb;
   uns32    ifsv_hdl;

   ifsv_hdl = m_IFSV_GET_HDL(0, 0);
   if ((ifsv_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFND, ifsv_hdl))
      == 0)
   {
      m_IFND_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,IFSV_COMP_IFND);
      return;
   }

   ifsv_get_ifindex_from_spt(if_index, *spt, ifsv_cb);

   ncshm_give_hdl(ifsv_hdl);

   return;
}

static void ifnd_ipxs_get_ifindex_from_ipaddr(IPXS_CB *cb, 
                                              NCS_IP_ADDR *addr, 
                                              NCS_IFSV_IFINDEX *if_index)
{

   IPXS_IFIP_NODE   *ifip_node;
   IPXS_IFIP_INFO   *ifip_info;
   uns32             i=0;
   
   /* Get the first node */
   ifip_node = (IPXS_IFIP_NODE *)ncs_patricia_tree_getnext
                          (&cb->ifip_tbl, (uns8*)NULL);

   while(ifip_node)
   {
      ifip_info = &ifip_node->ifip_info;

      for(i=0; i < ifip_info->ipaddr_cnt; i++)
      {
         /* Compare full address */
         if(m_NCS_CMP_IP_ADDR(&ifip_info->ipaddr_list[i].ipaddr.ipaddr, addr) == 0)
         {
            *if_index = ifip_info->ifindexNet;
            break;
         }
      }

      if(i < ifip_info->ipaddr_cnt)
         break;

      /* Get the first node */
      ifip_node = (IPXS_IFIP_NODE *)ncs_patricia_tree_getnext
                          (&cb->ifip_tbl, (uns8*)&ifip_info->ifindexNet);
   }
}

static void ifnd_ipxs_get_ifindex_from_key(IPXS_CB *cb,
                                           NCS_IPXS_IFKEY *key, 
                                           NCS_IFSV_IFINDEX *if_index)
{
   switch(key->type)
   {
   case NCS_IPXS_KEY_IFINDEX:
      *if_index = key->info.iface;
      break;
   case NCS_IPXS_KEY_SPT:
      ifnd_ipxs_get_ifindex_from_spt(&key->info.spt, if_index);
      break;
   case NCS_IPXS_KEY_IP_ADDR:
      ifnd_ipxs_get_ifindex_from_ipaddr(cb, &key->info.ip_addr, if_index);
      break;
   default:
      break;
   }
   return;
}

uns32 ipxs_ifnd_if_info_attr_cpy(IPXS_CB *cb, uns32 ifindex, 
                                              NCS_IPXS_INTF_REC *intf_rec)
{
   IFSV_CB *ifsv_cb;
   uns32   ifsv_hdl;
   IFSV_INTF_DATA *data;

   ifsv_hdl = m_IFSV_GET_HDL(0, 0);

   if ((ifsv_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFND, ifsv_hdl))
      == 0)
   {
      m_IFND_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,IFSV_COMP_IFND);
      return (NCSCC_RC_FAILURE);
   }

   data = ifsv_intf_rec_find(ifindex, ifsv_cb);

   if(data == 0)
   {
      ncshm_give_hdl(ifsv_hdl);
      m_IFND_LOG_EVT_L(IFSV_LOG_IFND_IPXS_EVT_INFO,"Coudn't get IFIP rec",0);
      return NCSCC_RC_FAILURE;
   }
   intf_rec->spt = data->spt_info;
   intf_rec->if_info = data->if_info;
   
   /* Is this interface is local to this node or Not */
   if(m_NCS_NODE_ID_FROM_MDS_DEST(data->current_owner_mds_destination) == 
       m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest))
      intf_rec->is_lcl_intf = TRUE;
   else
      intf_rec->is_lcl_intf = FALSE;

   /* RSR:TBD, Copy the internal pointers */

   ncshm_give_hdl(ifsv_hdl);
   return NCSCC_RC_SUCCESS;
}

#if(NCS_IFSV_USE_NETLINK == 1)
/****************************************************************************
 * Name          : parse_rtattr
 *
 * Description   : This is the netlink specific utility function.
 *
 * Arguments     : tb - array of struct type rtattr
 *                 max - integer specifying maximum number of attributes to parse
 *                 rta - Pointer to struct type rtattr to parse from
 *                 len - payload size
 *
 * Return Values : zero
 *
 * Notes         : None.
 *****************************************************************************/
static int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
   while(RTA_OK(rta, len))
   {
      if(rta->rta_type <= max)
         tb[rta->rta_type] = rta;
      rta = RTA_NEXT(rta, len);
   }
   if(len)
      fprintf(stderr, "!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);

   return 0;
}

/****************************************************************************
 * Name          : ipxs_ifnd_gen_getlink_req
 *
 * Description   : This is the netlink request, to get the available 
 *                 interface(link) details.
 *
 * Arguments     : ipxs_cb - Pointer to IPXS_CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ****************************************************************************/
static uns32 ipxs_ifnd_gen_getlink_req(IPXS_CB *ipxs_cb)
{
   struct rtgenmsg g;
   int err;
   struct nlmsghdr nlh;
   struct sockaddr_nl nladdr;
   struct iovec iov[2] = { { &nlh, sizeof(nlh) }, { &g, sizeof(g)} };
   struct msghdr msg = {
      (void*)&nladdr, sizeof(nladdr),
      iov, 2,
      NULL, 0,
      0
   };

   memset(&g, '\0', sizeof(g));
   g.rtgen_family = AF_UNSPEC;

   memset(&nladdr, 0, sizeof(nladdr));
   nladdr.nl_family = AF_NETLINK;
   nlh.nlmsg_len = NLMSG_LENGTH(sizeof(g));
   nlh.nlmsg_type = RTM_GETLINK;
   nlh.nlmsg_flags = NLM_F_ROOT|NLM_F_REQUEST|NLM_F_MATCH;
   nlh.nlmsg_pid = 0;
   nlh.nlmsg_seq = count++;
   err = sendmsg(ipxs_cb->netlink_fd, &msg, 0);
   if (err == nlh.nlmsg_len){
      m_NCS_CONS_PRINTF("ipxs_ifnd_gen_getlink_req succ \n");
      return NCSCC_RC_SUCCESS;
   }

   m_NCS_CONS_PRINTF("ipxs_ifnd_gen_getlink_req fail \n");
   return NCSCC_RC_FAILURE;
}

/****************************************************************************
 * Name          : ipxs_ifnd_gen_getv4addr_req
 *
 * Description   : This is the netlink request, to get the available 
 *                 addresses of the interfaces(links).
 *
 * Arguments     : ipxs_cb - Pointer to IPXS_CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ****************************************************************************/
static uns32 ipxs_ifnd_gen_getv4addr_req(IPXS_CB *ipxs_cb)
{
        struct {
                struct nlmsghdr nlh;
                struct rtgenmsg g;
        } req;
        struct sockaddr_nl nladdr;

        memset(&nladdr, 0, sizeof(nladdr));
        nladdr.nl_family = AF_NETLINK;

        req.nlh.nlmsg_len = sizeof(req);
        req.nlh.nlmsg_type = RTM_GETADDR;
        req.nlh.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
        req.nlh.nlmsg_pid = 0;
        req.nlh.nlmsg_seq = count++;
        req.g.rtgen_family = AF_INET;

        return sendto(ipxs_cb->netlink_fd, (void*)&req, sizeof(req), 0, (struct sockaddr*)&nladdr, sizeof(nladdr));

}

/****************************************************************************
 * Name          : ifnd_ipxs_get_ifndx_for_interface_number
 *
 * Description   : This is the utility function, which parses the IFND's if_tbl
 *                 looking for the interface with the name "ifname".
 *
 * Arguments     : ifsv_cb - Pointer to IFSV_CB
 *               : ipxs_cb - Pointer to IPXS_CB
 *               : link_index - integer specifying the Link number
 *               : ifindex - Pointer to NCS_IFSV_IFINDEX(result)
 *               : ifname - Name of the interface
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *               : (*ifindex) holds the interface-index, in case of success
 *
 * Notes         : None.
 ****************************************************************************/
static uns32 ifnd_ipxs_get_ifndx_for_interface_number(IFSV_CB *ifsv_cb, IPXS_CB *ipxs_cb,
           int link_index, NCS_IFSV_IFINDEX *ifindex, char *ifname)
{
   IFSV_INTF_REC *rec = NULL;
   NCS_IFSV_IFINDEX lcl_indx = 0;

   *ifindex = 0; /* Initialize the result. */

   while((rec = (IFSV_INTF_REC *)ncs_patricia_tree_getnext(&ifsv_cb->if_tbl, (uns8*)&lcl_indx))
         != NULL)
   {
      lcl_indx = rec->intf_data.if_index;
      /* Look for records which are owned by the MDS_DEST of the local IFND */
      if((m_NCS_STRCMP(ifname, &rec->intf_data.if_info.if_name) == 0) &&
         (m_NCS_MEMCMP((uns8*)&ifsv_cb->my_dest, 
             (uns8*)&rec->intf_data.current_owner_mds_destination,
             sizeof(MDS_DEST)) == 0))
      {
         *ifindex = rec->intf_data.if_index;
         return NCSCC_RC_SUCCESS;
      }
   }

   return NCSCC_RC_FAILURE;
}

/*
 ***************************************************************************
 *  PROCEDURE NAME    :   ifnd_ipxs_evt_destroy                             *
 *  DESCRIPTION       :   Function to free the  IPXSEVT, and its           *
 *                                     allocated sub memebers list.        *
 ***************************************************************************
*/



#endif /* #if(NCS_IFSV_USE_NETLINK == 1) */

