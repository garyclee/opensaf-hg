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
  FILE NAME: IFNDDPRC.C

  DESCRIPTION: IDIM initalization and destroy routines.

  FUNCTIONS INCLUDED in this module:
  idim_port_no_comp ..... Compare port numbers.
  ifnd_idim_init  ........IDIM initalization.
  idim_clear_mbx ..... ...Clean IDIM Mail box routine.
  ifnd_idim_destroy.......IDIM destroy..

******************************************************************************/

IFSV_IDIMLIB_STRUCT gl_ifsv_idimlib;

uns32 gl_ifsv_idim_hdl;
static uns32
idim_port_no_comp (uns8 *key1, uns8 *key2);
static NCS_BOOL
idim_clear_mbx(NCSCONTEXT arg, NCSCONTEXT msg);

/****************************************************************************
 * Name          : idim_port_no_comp
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
idim_port_no_comp (uns8 *key1, uns8 *key2)
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
 * Name          : ifnd_idim_init
 *
 * Description   : This is the function which is used to initialize the IDIM.
 *
 * Arguments     : ifsv_cb  - key which is already available in the list. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : 
 *****************************************************************************/
uns32
ifnd_idim_init (IFSV_CB *ifsv_cb)
{
   uns32 res = NCSCC_RC_SUCCESS;
   IFSV_IDIM_CB  *idim_cb;
   uns32 idim_hdl;

   if (ifsv_cb->idim_up == FALSE)
   {
      do
      {
         /* allocate a mail box */
         if (m_NCS_IPC_CREATE(&m_IFSV_IDIM_GET_MBX_HDL) != NCSCC_RC_SUCCESS)
         {
             m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Mail box create failed :ifnd_idim_init() "," ");
            m_IFND_LOG_API_L(IFSV_LOG_IDIM_INIT_FAILURE,(long)ifsv_cb);
            m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_CREATE_FAIL,(long)ifsv_cb);
            return (NCSCC_RC_FAILURE);
         }
         
         if (m_NCS_IPC_ATTACH(&m_IFSV_IDIM_GET_MBX_HDL) != NCSCC_RC_SUCCESS)
         {
             m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Mail box Attach failed :ifnd_idim_init() "," ");
            m_IFND_LOG_API_L(IFSV_LOG_IDIM_INIT_FAILURE,(long)ifsv_cb);
            m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_CREATE_FAIL,(long)ifsv_cb);
            return (NCSCC_RC_FAILURE);
         }
         
         /* allocate a idim CB */
         idim_cb = m_MMGR_ALLOC_IFND_IDIM_CB;
         
         if (idim_cb == IFSV_NULL)
         {
             m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Idim CB alloc failed :ifnd_idim_init() "," ");
            /* Log for memory failure */
            m_NCS_IPC_RELEASE(&m_IFSV_IDIM_GET_MBX_HDL, NULL);
            res = NCSCC_RC_FAILURE;
            m_IFND_LOG_API_L(IFSV_LOG_IDIM_INIT_FAILURE,(long)ifsv_cb);
            m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,\
               0);
            break;
         }     

         /* initialize the CB */
         m_NCS_MEMSET(idim_cb, 0, sizeof(IFSV_IDIM_CB));
         
         idim_cb->shelf   = ifsv_cb->shelf;
         idim_cb->slot    = ifsv_cb->slot;
         /* embedding subslot changes */
         idim_cb->subslot    = ifsv_cb->subslot;
         idim_cb->ifnd_addr    = ifsv_cb->my_dest;
         idim_cb->ifnd_mbx     = ifsv_cb->mbx;
         idim_cb->ifnd_hdl     = ifsv_cb->cb_hdl;
         idim_cb->mds_hdl      = ifsv_cb->my_mds_hdl;
         
         idim_cb->ifnd_addr = ifsv_cb->my_dest;
         
         /* initialize the doubly link list for port registeration que */
         idim_cb->port_reg.order      = NCS_DBLIST_ANY_ORDER;
         idim_cb->port_reg.cmp_cookie = idim_port_no_comp;      
         
         
         /* create a thread for idim*/
         if (m_NCS_TASK_CREATE ((NCS_OS_CB)idim_process_mbx,
            &m_IFSV_IDIM_GET_MBX_HDL,
            NCS_IFSV_IDIM_TASKNAME,
            NCS_IFSV_IDIM_PRIORITY,
            NCS_IFSV_IDIM_STACKSIZE,
            &m_IFSV_IDIM_GET_TASK_HDL) != NCSCC_RC_SUCCESS)
         {
             m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Idim process mailbox thread creation failed :ifnd_idim_init() "," ");
            m_IFND_LOG_API_L(IFSV_LOG_IDIM_INIT_FAILURE,(long)ifsv_cb);
            m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_TASK_CREATE_FAIL,(long)idim_cb);
            m_NCS_IPC_RELEASE(&m_IFSV_IDIM_GET_MBX_HDL, NULL);
            m_MMGR_FREE_IFND_IDIM_CB(idim_cb);
            return (NCSCC_RC_FAILURE);
         }
         
         if (m_NCS_TASK_START (m_IFSV_IDIM_GET_TASK_HDL) != NCSCC_RC_SUCCESS)
         {
             m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Idim mailbox thread start failed :ifnd_idim_init() "," ");
            m_IFND_LOG_API_L(IFSV_LOG_IDIM_INIT_FAILURE,(long)ifsv_cb);
            m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_TASK_CREATE_FAIL,(long)idim_cb);
            m_NCS_TASK_RELEASE(m_IFSV_IDIM_GET_TASK_HDL);
            m_NCS_IPC_RELEASE(&m_IFSV_IDIM_GET_MBX_HDL, NULL);
            m_MMGR_FREE_IFND_IDIM_CB(idim_cb);
            return (NCSCC_RC_FAILURE);
         }
         
         /* create a handle manager */
         idim_hdl = ncshm_create_hdl(ifsv_cb->hm_pid, NCS_SERVICE_ID_IFND, 
            (NCSCONTEXT)idim_cb);
         
         if(0 == idim_hdl)
         {
             m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Idim handle manager failed :ifnd_idim_init() "," ");
            m_IFND_LOG_API_L(IFSV_LOG_IDIM_INIT_FAILURE,(long)ifsv_cb);
            m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_HDL_MGR_FAIL,ifsv_cb->comp_type);
            m_NCS_TASK_RELEASE(m_IFSV_IDIM_GET_TASK_HDL);
            m_NCS_IPC_RELEASE(&m_IFSV_IDIM_GET_MBX_HDL, NULL);
            m_MMGR_FREE_IFND_IDIM_CB(idim_cb);
            res = NCSCC_RC_FAILURE;
            break;
         }
         
         /* store the idim info globally */
         m_IFSV_IDIM_STORE_HDL(idim_hdl);
         
         /* fill the required paramaters in the ifsv CB */
         ifsv_cb->idim_hdl  = idim_hdl;
         ifsv_cb->idim_up   = TRUE;
         ifsv_cb->idim_mbx  = m_IFSV_IDIM_GET_MBX_HDL;
         ifsv_cb->idim_task = m_IFSV_IDIM_GET_TASK_HDL;
         m_IFND_LOG_API_L(IFSV_LOG_IDIM_INIT_DONE,idim_hdl);
      } while (0);
   }

   return (res);
}

/****************************************************************************
 * Name          : idim_clear_mbx
 *
 * Description   : This is the function which is used to cleanup the idim 
 *                 mailbox.
 *
 * Arguments     : arg  - argument.
 *                 msg  - message starting pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : 
 *****************************************************************************/
static NCS_BOOL
idim_clear_mbx(NCSCONTEXT arg, NCSCONTEXT msg)
{   
   IFSV_IDIM_EVT  *pEvt = (IFSV_IDIM_EVT *)msg;
   IFSV_IDIM_EVT  *pnext;
   pnext = pEvt;
   while (pnext)
   {
      pnext = pEvt->next;
      m_MMGR_FREE_IFSV_IDIM_EVT(pEvt);  
      pEvt = pnext;
   }
   return TRUE;
}
/****************************************************************************
 * Name          : ifnd_idim_destroy
 *
 * Description   : This is the function which is used to destroy the IDIM.
 *
 * Arguments     : ifsv_cb  - key which is already available in the list. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : 
 *****************************************************************************/
uns32
ifnd_idim_destroy (IFSV_CB *ifsv_cb)
{
   IFSV_IDIM_PORT_TYPE_REG_TBL *reg_tbl;   
   IFSV_IDIM_CB                *idim_cb;
   uns32                       idim_hdl;

   idim_hdl = m_IFSV_IDIM_GET_HDL;
   
   if ((idim_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFND, 
      idim_hdl)) != IFSV_NULL)
   {
      ifsv_cb->idim_up = FALSE;

      /* detach the mail box */
      m_NCS_IPC_DETACH(&ifsv_cb->idim_mbx,idim_clear_mbx,ifsv_cb);
      /* release the mail box */   
      m_NCS_IPC_RELEASE(&ifsv_cb->idim_mbx,NULL);
      
      /* release the task */
      m_NCS_TASK_RELEASE(m_IFSV_IDIM_GET_TASK_HDL);
      
      /* free the port registeration que */
      reg_tbl = (IFSV_IDIM_PORT_TYPE_REG_TBL *)
            ncs_db_link_list_dequeue(&idim_cb->port_reg);
      
      while(reg_tbl != IFSV_NULL)
      {         
         m_MMGR_FREE_PORT_REG_TBL(reg_tbl);
         reg_tbl = (IFSV_IDIM_PORT_TYPE_REG_TBL *)
            ncs_db_link_list_dequeue(&idim_cb->port_reg);
      }
      
      /* send a message to the hardware drv process to destroy it - TBD*/
      
      ncshm_give_hdl(idim_hdl);
      ncshm_destroy_hdl(NCS_SERVICE_ID_IFND,idim_hdl);
      /* free the IDIM control block */
      m_MMGR_FREE_IFND_IDIM_CB(idim_cb);
   }
   m_IFND_LOG_API_L(IFSV_LOG_IDIM_DESTROY_DONE,0);
   return (NCSCC_RC_SUCCESS);
}
