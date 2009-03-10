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
  FILE NAME: mqsv_proc.c

  DESCRIPTION: MQND Processing routines.

******************************************************************************/
#include "mqnd.h"

/*static uns32 mqnd_queue_create_attr_get(MQND_CB *cb, uns32 qhdl,
                                 MDS_DEST *qdest,
                                 SaMsgQueueCreationAttributesT *o_qattr);*/
/*static uns32 mqnd_set_queue_owner(MQND_CB *cb, MQND_QUEUE_NODE   *qnode, MDS_DEST* rcvr_mqa);*/
/*static uns32 mqnd_queue_dereg_with_mqd(MQND_CB *cb, MQND_QUEUE_NODE *qnode);*/

/****************************************************************************
 * Name          : mqnd_queue_create
 *
 * Description   : Function to get queue status (from any MQND)
 *
 * Arguments     : MQND_CB *cb - MQND CB pointer
 *                 SaMsgQueueHandleT qhdl - Queue Handle
 *                 MDS_DEST *qdest - MDS Destination of queue.
                   SaMsgQueueHandleT - Queue Handle
                   MQP_TRANSFERQ_RSP - Transfer Response
                   SaAisErrorT - To fill in proper error message
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *                 SaMsgQueueStatusT *o_status - Queu Status
 *
 * Notes         : None.
 *****************************************************************************/
uns32 mqnd_queue_create(MQND_CB *cb, MQP_OPEN_REQ *open, 
                                 MDS_DEST *rcvr_mqa, SaMsgQueueHandleT *qhdl, 
                                 MQP_TRANSFERQ_RSP *transfer_rsp, SaAisErrorT *err)
{
   uns32             rc = NCSCC_RC_FAILURE,index;
   MQND_QUEUE_NODE   *qnode=NULL;
   uns8              i=0;
   MQND_QNAME_NODE   *pnode=NULL; 
   uns32             counter=0;
   MQND_QUEUE_CKPT_INFO queue_ckpt_node;
   NCS_BOOL is_q_reopen = FALSE;

   qnode = m_MMGR_ALLOC_MQND_QUEUE_NODE;
   if(!qnode)
   {
     rc = NCSCC_RC_FAILURE;
     m_LOG_MQSV_ND(MQND_ALLOC_QUEUE_NODE_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
     goto free_mem;
   }


   pnode = m_MMGR_ALLOC_MQND_QNAME_NODE;
   if (!pnode) 
   {
     rc = NCSCC_RC_FAILURE;
     m_LOG_MQSV_ND(MQND_ALLOC_QNAME_NODE_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
     goto free_mem;
   }


   m_NCS_OS_MEMSET(qnode, 0, sizeof(MQND_QUEUE_NODE));

   /* Store the receiver Information */
   qnode->qinfo.msgHandle = open->msgHandle;
   qnode->qinfo.rcvr_mqa = *rcvr_mqa;

   /* Save the queue parameters */
   qnode->qinfo.queueName = open->queueName;
   qnode->qinfo.queueStatus.creationFlags = open->creationAttributes.creationFlags;
   qnode->qinfo.queueStatus.retentionTime = open->creationAttributes.retentionTime;

   for (i=SA_MSG_MESSAGE_HIGHEST_PRIORITY; i <= SA_MSG_MESSAGE_LOWEST_PRIORITY; i++) {
       qnode->qinfo.totalQueueSize+= open->creationAttributes.size[i]; 
       qnode->qinfo.size[i] = open->creationAttributes.size[i];
       qnode->qinfo.queueStatus.saMsgQueueUsage[i].queueSize = open->creationAttributes.size[i];
   }

   qnode->qinfo.queueStatus.closeTime = 0;
   qnode->qinfo.listenerHandle = 0;
   qnode->qinfo.sendingState = MSG_QUEUE_AVAILABLE;
   qnode->qinfo.owner_flag = MQSV_QUEUE_OWN_STATE_OWNED;
   

   /* Open the Message Queue */
   rc = mqnd_mq_create(&qnode->qinfo);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_MQSV_ND(MQND_QUEUE_CREATE_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      goto free_mem;
   }
   
   /* If RECEIVED_CALLBACK is enabled, create a listener queue */
   if (open->openFlags &  SA_MSG_QUEUE_RECEIVE_CALLBACK) {
      rc = mqnd_listenerq_create(&qnode->qinfo);
      if(rc != NCSCC_RC_SUCCESS) {
         m_LOG_MQSV_ND(MQND_LISTENERQ_CREATE_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
         goto free_mq;
      } 
   }

   /* To store the index in the shared memory for this queue*/
   rc =  mqnd_find_shm_ckpt_empty_section(cb, &index);
   if(rc != NCSCC_RC_SUCCESS)
   {
     m_LOG_MQSV_ND(MQND_QUEUE_CREATE_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
     goto free_listenerq;
   }

   qnode->qinfo.shm_queue_index = index;
   m_GET_TIME_STAMP(qnode->qinfo.creationTime);

   if(transfer_rsp)
      qnode->qinfo.creationTime = transfer_rsp->creationTime;/* When queue transfer happens, old creation time is retained*/
   /*Checkpoint new queue information*/
   m_NCS_OS_MEMSET(&queue_ckpt_node, 0, sizeof(MQND_QUEUE_CKPT_INFO));
   mqnd_cpy_qnodeinfo_to_ckptinfo(cb, qnode, &queue_ckpt_node);
   rc = mqnd_ckpt_queue_info_write(cb, &queue_ckpt_node, qnode->qinfo.shm_queue_index);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_MQSV_ND(MQND_QUEUE_CREATE_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      goto free_listenerq;
   }

   /* Add the Queue in Tree */
   rc = mqnd_queue_node_add(cb, qnode);
   if(rc != NCSCC_RC_SUCCESS)  {
      m_LOG_MQSV_ND(MQND_QNODE_ADD_DB_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      goto free_listenerq;
    }


   /* If Queue being created as part of queueTransfer, fill the queue with the messages in transferBuffer */
   if (transfer_rsp) {
      rc = mqnd_fill_queue_from_transfered_buffer(cb, qnode, transfer_rsp);

      if (rc != NCSCC_RC_SUCCESS)
      {
         m_LOG_MQSV_ND(MQND_QUEUE_CREATE_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
         goto qnode_destroy;
      }
   }

   /* Create & add qname structure used in MIBS */
   m_NCS_OS_MEMSET(pnode, 0, sizeof(MQND_QNAME_NODE));
   pnode->qname = qnode->qinfo.queueName;
   pnode->qhdl = (SaMsgQueueHandleT)qnode->qinfo.queueHandle;
   rc = mqnd_qname_node_add(cb, pnode);
   if (rc != NCSCC_RC_SUCCESS)
   {
       m_LOG_MQSV_ND(MQND_QUEUE_CREATE_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
       goto qnode_destroy;
   }

   /* Register with MQD */
   rc = mqnd_queue_reg_with_mqd(cb, qnode, err, is_q_reopen);
   if(rc != NCSCC_RC_SUCCESS)  {
      m_LOG_MQSV_ND(MQND_Q_REG_WITH_MQD_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      goto qname_destroy;
    }
   /* MIB stuff */
   rc = mqsv_reg_mqndmib_queue_tbl_row(cb,qnode->qinfo.queueName,&qnode->qinfo.mab_rec_row_hdl);
   if(rc != NCSCC_RC_SUCCESS) {
      m_LOG_MQSV_ND(MQND_Q_ROW_REG_WITH_MIB_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
   }

   for(counter=SA_MSG_MESSAGE_HIGHEST_PRIORITY;counter<SA_MSG_MESSAGE_LOWEST_PRIORITY+1;counter++) {
      rc = mqsv_reg_mqndmib_queue_priority_tbl_row(cb,qnode->qinfo.queueName,counter,
                                                  &qnode->qinfo.mab_rec_priority_row_hdl[counter]);
      if(rc != NCSCC_RC_SUCCESS) {
        m_LOG_MQSV_ND(MQND_QPR_TBL_ROW_REG_WITH_MIB_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      }
   }

   *qhdl = qnode->qinfo.queueHandle;
   m_LOG_MQSV_ND(MQND_QUEUE_CREATE_SUCCESS,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
   return NCSCC_RC_SUCCESS;



qname_destroy:
   mqnd_qname_node_del(cb, pnode);

qnode_destroy:
   mqnd_queue_node_del(cb, qnode);

free_listenerq:      /*may be the checkpoint is to be rewritten with listen handle zero*/
   mqnd_shm_queue_ckpt_section_invalidate(cb, qnode);
   mqnd_listenerq_destroy(&qnode->qinfo);

free_mq:
    mqnd_mq_destroy(&qnode->qinfo);

free_mem:
   if (pnode) m_MMGR_FREE_MQND_QNAME_NODE(pnode);
   if (qnode) m_MMGR_FREE_MQND_QUEUE_NODE(qnode);

   return rc;
}


/****************************************************************************
 * Name          : mqnd_compare_create_attr
 *
 * Description   : Function to match the received create attributes from the
 *                 existing create attributes.
 *
 * Arguments     : SaMsgQueueCreationAttributesT *open_ca - Attributes in Open.
 *                 SaMsgQueueCreationAttributesT *open_ca - Existing Attributes.
 *
 * Return Values : TRUE/FALSE
 *
 * Notes         : None.
 *****************************************************************************/
NCS_BOOL mqnd_compare_create_attr(SaMsgQueueCreationAttributesT *open_ca,
                                  SaMsgQueueCreationAttributesT *curr_ca)
{
   uns32 i=0;

   if(open_ca->creationFlags != curr_ca->creationFlags)
      return FALSE;
/* With the introduction of API to set the retention time dynamically
   check for retention time nolonger exists */
   for(i=0; i<SA_MSG_MESSAGE_LOWEST_PRIORITY; i++)
   {
      if(open_ca->size[i] != curr_ca->size[i])
         return FALSE;
   }

   return TRUE;
}


/****************************************************************************
 * Name          : mqnd_queue_reg_with_mqd
 *
 * Description   : Register the new Queue/ Queue Changes with MQD
 *
 * Arguments     : SaMsgQueueCreationAttributesT *open_ca - Attributes in Open.
 *                 SaMsgQueueCreationAttributesT *open_ca - Existing Attributes.
 *
 * Return Values : TRUE/FALSE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 mqnd_queue_reg_with_mqd(MQND_CB *cb, MQND_QUEUE_NODE *qnode, SaAisErrorT *err, NCS_BOOL is_q_reopen)
{
   uns32             rc = NCSCC_RC_SUCCESS;
   ASAPi_OPR_INFO    opr;

   /* Request the ASAPi (at MQD) for queue REG */
   m_NCS_OS_MEMSET(&opr, 0, sizeof(ASAPi_OPR_INFO));
   opr.type = ASAPi_OPR_MSG;
   opr.info.msg.opr = ASAPi_MSG_SEND;

   /* Fill MDS info */
   opr.info.msg.sinfo.to_svc = NCSMDS_SVC_ID_MQD;
   opr.info.msg.sinfo.dest = cb->mqd_dest;
   opr.info.msg.sinfo.stype = MDS_SENDTYPE_SNDRSP;

   opr.info.msg.req.msgtype = ASAPi_MSG_REG;
   opr.info.msg.req.info.reg.objtype = ASAPi_OBJ_QUEUE;
   opr.info.msg.req.info.reg.queue.name = qnode->qinfo.queueName;
   opr.info.msg.req.info.reg.queue.addr = cb->my_dest;
   opr.info.msg.req.info.reg.queue.hdl = qnode->qinfo.queueHandle;
   opr.info.msg.req.info.reg.queue.name = qnode->qinfo.queueName;
   opr.info.msg.req.info.reg.queue.owner = qnode->qinfo.owner_flag;
   opr.info.msg.req.info.reg.queue.retentionTime = qnode->qinfo.queueStatus.retentionTime;
   opr.info.msg.req.info.reg.queue.status = qnode->qinfo.sendingState;
   opr.info.msg.req.info.reg.queue.creationFlags = qnode->qinfo.queueStatus.creationFlags;

   m_NCS_MEMCPY(opr.info.msg.req.info.reg.queue.size, qnode->qinfo.size, sizeof(SaSizeT)*(SA_MSG_MESSAGE_LOWEST_PRIORITY+1));

   /* Request the ASAPi */
   rc = asapi_opr_hdlr(&opr);
   if(rc == SA_AIS_ERR_TIMEOUT)
   {
    /*This is the case when REG request is assumed to be lost or not proceesed at MQD becoz of failover.So
      to compensate for the lost request we send an async DEREG/REG request to MQD in case it if is registered
      and the standby update has not happened*/
   /* Request the ASAPi (at MQD) for queue DEREG */
    m_NCS_OS_MEMSET(&opr, 0, sizeof(ASAPi_OPR_INFO));
    opr.type = ASAPi_OPR_MSG;
    opr.info.msg.opr = ASAPi_MSG_SEND;

    /* Fill MDS info */
    opr.info.msg.sinfo.to_svc = NCSMDS_SVC_ID_MQD;
    opr.info.msg.sinfo.dest = cb->mqd_dest;
    opr.info.msg.sinfo.stype = MDS_SENDTYPE_SND;

    /*If the queue is a new one then in case of TIMEOUT, DEREG it & Delete the queue at MQND.If Q is orphan
     and an attempt to Register the queue(OWNED) failed with TIMEOUT then REG it to ORPHAN irrespective of 
     whether previous request has been served or not and Return TRY_AGAIN to MQA in Both cases*/ 
        
    if(!is_q_reopen)
    {
     #ifdef NCS_MQND
      m_NCS_CONS_PRINTF("QUEUE OPEN TIMEOUT :DEREG  \n"); 
     #endif
     opr.info.msg.req.msgtype = ASAPi_MSG_DEREG;
     opr.info.msg.req.info.dereg.objtype = ASAPi_OBJ_QUEUE;
     opr.info.msg.req.info.dereg.queue = qnode->qinfo.queueName;

     rc = asapi_opr_hdlr(&opr);  /*May be insert a log*/
     /* Free the response Event */
     if(opr.info.msg.resp) 
        asapi_msg_free(&opr.info.msg.resp);
     if(err) *err = SA_AIS_ERR_TRY_AGAIN;
     return NCSCC_RC_FAILURE;
    }
    else
    {
     #ifdef NCS_MQND
      m_NCS_CONS_PRINTF("QUEUE REOPEN TIMEOUT :REG TO ORPHAN\n"); 
     #endif
     opr.info.msg.req.msgtype = ASAPi_MSG_REG;
     opr.info.msg.req.info.reg.objtype = ASAPi_OBJ_QUEUE;
     opr.info.msg.req.info.reg.queue.name = qnode->qinfo.queueName;
     opr.info.msg.req.info.reg.queue.addr = cb->my_dest;
     opr.info.msg.req.info.reg.queue.hdl = qnode->qinfo.queueHandle;
     opr.info.msg.req.info.reg.queue.name = qnode->qinfo.queueName;
     opr.info.msg.req.info.reg.queue.owner = MQSV_QUEUE_OWN_STATE_ORPHAN;
     opr.info.msg.req.info.reg.queue.retentionTime = qnode->qinfo.queueStatus.retentionTime;
     opr.info.msg.req.info.reg.queue.status = qnode->qinfo.sendingState;
     opr.info.msg.req.info.reg.queue.creationFlags = qnode->qinfo.queueStatus.creationFlags;

     m_NCS_MEMCPY(opr.info.msg.req.info.reg.queue.size,qnode->qinfo.size, sizeof(SaSizeT)*(SA_MSG_MESSAGE_LOWEST_PRIORITY+1));
     rc = asapi_opr_hdlr(&opr);  /*May be insert a log*/
     /* Free the response Event */
     if(opr.info.msg.resp) 
       asapi_msg_free(&opr.info.msg.resp);
     if(err) *err = SA_AIS_ERR_TRY_AGAIN;
     return NCSCC_RC_FAILURE;
    }
   }
   else
   { 
    if ((rc!= SA_AIS_OK) || 
       (!opr.info.msg.resp) || (opr.info.msg.resp->info.rresp.err.flag)) 
    {
      rc = NCSCC_RC_FAILURE; 
      m_LOG_MQSV_ND(MQND_ASAPI_REG_HDLR_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
    }
   }

   /* Free the response Event */
   if(opr.info.msg.resp) 
     asapi_msg_free(&opr.info.msg.resp);

   return rc;
}


/***************************************************************************
 * Function :  mqsv_reg_mqndmib_queue_tbl_row
 *
 * Purpose  :  This function registers MQND MIB table rows with MAB.
 *
 * Input    :  cb         MQND Control Block.
 *             mqindex_name    Index name.
 *             row_hdl    The row handle returned by MAB.
 *
 * Returns  :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES    :  If NCSCC_RC_SUCCESS row_hdl will contain the handle
 *             returned by MAB.
 *
 **************************************************************************/

uns32 mqsv_reg_mqndmib_queue_tbl_row(MQND_CB *cb,
                             SaNameT mqindex_name,
                             uns32 *row_hdl)
{
   NCSOAC_SS_ARG  mqnd_oac_arg;
   uns32          rc = NCSCC_RC_SUCCESS;
   NCSMAB_EXACT   *exact=0;
   uns32  *index_name=0;
   uns32 i;

   index_name = (uns32 *)m_MMGR_ALLOC_MQND_DEFAULT((mqindex_name.length +1)*sizeof(uns32));
   if(!index_name) {
      rc = NCSCC_RC_FAILURE; 
      m_LOG_MQSV_ND(MQND_MEM_ALLOC_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return rc;
   }

   index_name[0]= mqindex_name.length;
   for(i=0;i< mqindex_name.length;i++)
   {
     index_name[i+1]=(uns32) mqindex_name.value[i];
   }  

   m_NCS_MEMSET(&mqnd_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

   /* Register for MQND table rows*/
   mqnd_oac_arg.i_oac_hdl = cb->oac_hdl;
   mqnd_oac_arg.i_op      = NCSOAC_SS_OP_ROW_OWNED;
   mqnd_oac_arg.i_tbl_id  = NCSMIB_TBL_MQSV_MSGQTBL;

   mqnd_oac_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_EXACT;
   mqnd_oac_arg.info.row_owned.i_fltr.is_move_row_fltr = FALSE;
   exact = &mqnd_oac_arg.info.row_owned.i_fltr.fltr.exact;
   exact->i_idx_len = mqindex_name.length +1;
   exact->i_bgn_idx = 0;
   exact->i_exact_idx = index_name;
   mqnd_oac_arg.info.row_owned.i_ss_cb = (NCSOAC_SS_CB)NULL;
   mqnd_oac_arg.info.row_owned.i_ss_hdl = cb->cb_hdl;

   if ((rc = ncsoac_ss(&mqnd_oac_arg)) != NCSCC_RC_SUCCESS)
      m_LOG_MQSV_ND(MQND_MSGQ_TBL_EXACT_FLTR_REGISTER_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
   else
      m_LOG_MQSV_ND(MQND_MSGQ_TBL_EXACT_FLTR_REGISTER_SUCCESS,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);

   *row_hdl = mqnd_oac_arg.info.row_owned.o_row_hdl;
   if(index_name)
      m_MMGR_FREE_MQND_DEFAULT(index_name);
   return rc;
}



/***************************************************************************
 * Function :  mqnd_unreg_mib_row
 *
 * Purpose  :  This function unregisters MQND MIB table rows with MAB.
 *
 * Input    :  MQND Control Block.
 *             tbl_id Table id to unregister             
 *             The row handle of the row that needs to be deleted.
 *
 * Returns  :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES    :  None.
 *
 **************************************************************************/
uns32 mqnd_unreg_mib_row(MQND_CB *cb,uns32 tbl_id, uns32 row_hdl)
{
   NCSOAC_SS_ARG  mqnd_oac_arg;
   uns32          rc = NCSCC_RC_SUCCESS ;

   m_NCS_MEMSET(&mqnd_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

   /* Unregister for MQND table rows */
   mqnd_oac_arg.i_oac_hdl = cb->oac_hdl;
   mqnd_oac_arg.i_op      = NCSOAC_SS_OP_ROW_GONE;
   mqnd_oac_arg.i_tbl_id  = tbl_id;
   mqnd_oac_arg.info.row_gone.i_row_hdl = row_hdl;

   if( (rc =ncsoac_ss(&mqnd_oac_arg)) != NCSCC_RC_SUCCESS)
   {
      m_LOG_MQSV_ND(MQND_MSGQ_TBL_EXACT_FLTR_UNREGISTER_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
   }
   else
      m_LOG_MQSV_ND(MQND_MSGQ_TBL_EXACT_FLTR_UNREGISTER_SUCCESS,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_INFO,rc,__FILE__,__LINE__);


   return rc;
}


/***************************************************************************
 * Function :  mqsv_reg_mqndmib_queue_priority_tbl_row
 *
 * Purpose  :  This function registers MQND MIB table rows with MAB.
 *
 * Input    :  cb         MQND Control Block.
 *             mqindex_name    Index name.
 *             row_hdl    The row handle returned by MAB.
 *
 * Returns  :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES    :  If NCSCC_RC_SUCCESS row_hdl will contain the handle
 *             returned by MAB.
 *
 **************************************************************************/

uns32 mqsv_reg_mqndmib_queue_priority_tbl_row(MQND_CB *cb,
                             SaNameT mqindex_name,uns32 priority_index,
                             uns32 *row_hdl)
{
   NCSOAC_SS_ARG  mqnd_oac_arg;
   uns32          rc = NCSCC_RC_SUCCESS;
   NCSMAB_EXACT   *exact=0;
   uns32  *index_name=0;
   uns32 i;

   index_name = (uns32 *)m_MMGR_ALLOC_MQND_DEFAULT((mqindex_name.length +2)*sizeof(uns32));
   if(!index_name) {
      rc = NCSCC_RC_FAILURE; 
      m_LOG_MQSV_ND(MQND_MEM_ALLOC_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return rc;
   } 
   index_name[0]= mqindex_name.length;
   for(i=0;i< mqindex_name.length;i++)
   {
     index_name[i+1]=(uns32) mqindex_name.value[i];
   }  
   index_name[i+1]= priority_index;


   m_NCS_MEMSET(&mqnd_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

   /* Register for MQND table rows*/
   mqnd_oac_arg.i_oac_hdl = cb->oac_hdl;
   mqnd_oac_arg.i_op      = NCSOAC_SS_OP_ROW_OWNED;
   mqnd_oac_arg.i_tbl_id  = NCSMIB_TBL_MQSV_MSGQPRTBL;

   mqnd_oac_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_EXACT;
   mqnd_oac_arg.info.row_owned.i_fltr.is_move_row_fltr = FALSE;
   exact = &mqnd_oac_arg.info.row_owned.i_fltr.fltr.exact;
   exact->i_idx_len = mqindex_name.length +2;
   exact->i_bgn_idx = 0;
   exact->i_exact_idx = index_name;
   mqnd_oac_arg.info.row_owned.i_ss_cb = (NCSOAC_SS_CB)NULL;
   mqnd_oac_arg.info.row_owned.i_ss_hdl = cb->cb_hdl;

   if ((rc = ncsoac_ss(&mqnd_oac_arg)) != NCSCC_RC_SUCCESS)
   {
      m_LOG_MQSV_ND(MQND_MSGQPR_TBL_EXACT_FLTR_REGISTER_FAILED,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
   }
   else
      m_LOG_MQSV_ND(MQND_MSGQPR_TBL_EXACT_FLTR_REGISTER_SUCCESS,NCSFL_LC_MQSV_Q_MGMT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);

   *row_hdl = mqnd_oac_arg.info.row_owned.o_row_hdl;
   if(index_name)
      m_MMGR_FREE_MQND_DEFAULT(index_name);
   return rc;
}


