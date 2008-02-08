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

#include "mqnd.h"

/********************************************************************
 * Name        : samsgqueueentry_set
 *
 * Description : 
 *
 *******************************************************************/

uns32 samsgqueueentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSMIB_VAR_INFO* var_info,
                              NCS_BOOL test_flag)
{
   return NCSCC_RC_FAILURE;
}



/***********************************************************************
 * Name        : samsgqueueentry_setrow
 *
 * Description :
 *
 **********************************************************************/

uns32 samsgqueueentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                 NCSMIB_SETROW_PARAM_VAL* params,
                                 struct ncsmib_obj_info* obj_info,
                                 NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}



/**************************************************************************
 * Name         :  samsgqueueentry_extract
 *
 * Description  :
 *
 **************************************************************************/
uns32 samsgqueueentry_extract(NCSMIB_PARAM_VAL* param,
                                  NCSMIB_VAR_INFO* var_info,
                                  NCSCONTEXT data,
                                  NCSCONTEXT buffer)
{
   uns32            i,rc = NCSCC_RC_SUCCESS, offset;
   MQND_QUEUE_INFO  *pNode=(MQND_QUEUE_INFO *)data;
   SaUint64T        temp64=0; 
   MQND_QUEUE_CKPT_INFO *shm_base_addr;
   uns32    cb_hdl = m_MQND_GET_HDL( );
   MQND_CB          *cb=0;

   /* Get the CB from the handle */
   cb = ncshm_take_hdl(NCS_SERVICE_ID_MQND, cb_hdl);

   if(cb == NULL)
   {
      return NCSCC_RC_NO_INSTANCE; 
   }

   switch(param->i_param_id)
   {
      case saMsgQueueSize_ID:
        {
           pNode->totalQueueSize =0;
           for (i=SA_MSG_MESSAGE_HIGHEST_PRIORITY; i <= SA_MSG_MESSAGE_LOWEST_PRIORITY; i++)
              pNode->totalQueueSize += pNode->size[i];

           temp64  = pNode->totalQueueSize;
           m_MQSV_SNMPTM_UNS64_TO_PARAM(param,buffer,(SaUint64T)temp64);
           break;
        }
      case saMsgQueueUsedSize_ID:
        {
           shm_base_addr = cb->mqnd_shm.shm_base_addr; 
           offset = pNode->shm_queue_index;
           temp64 = shm_base_addr[offset].QueueStatsShm.totalQueueUsed; 
           m_MQSV_SNMPTM_UNS64_TO_PARAM(param,buffer,(SaUint64T)temp64);
           break;
        }
      case saMsgQueueRetentionTime_ID:
        {
           temp64 = pNode->queueStatus.retentionTime;
           m_MQSV_SNMPTM_UNS64_TO_PARAM(param,buffer,(SaUint64T)temp64);
           break;
        }
      case saMsgQueueCreateTime_ID:
       {
           temp64 = pNode->creationTime;
           m_MQSV_SNMPTM_UNS64_TO_PARAM(param,buffer,(SaUint64T)temp64);
           break;
        }
      case saMsgQueueIsPersistent_ID:
        {
           param->info.i_int = (pNode->queueStatus.creationFlags == 1)?1 :2;
           param->i_length =4;
           param->i_fmat_id = NCSMIB_FMAT_INT;
           break;
        } 
      case saMsgQueueIsBusy_ID:
        {
           if(pNode->owner_flag == MQSV_QUEUE_OWN_STATE_ORPHAN)
             param->info.i_int = 2;
           else 
             param->info.i_int = 1;
           param->i_length =1;
           param->i_fmat_id = NCSMIB_FMAT_INT;
           break;
        }     
      case saMsgQueueNumMsgs_ID:
        {
           shm_base_addr = cb->mqnd_shm.shm_base_addr;
           offset = pNode->shm_queue_index;
           param->info.i_int =shm_base_addr[offset].QueueStatsShm.totalNumberOfMessages;
           param->i_length =4;
           param->i_fmat_id = NCSMIB_FMAT_INT;
           break;
        }  
      case saMsgQueueNumMemberQueueGroup_ID:
        {
         /*Send an evt to MQD to get the no of members of Queue Group */
           MQSV_EVT req,*rsp = NULL;
 
           m_NCS_OS_MEMSET(&req, 0, sizeof(MQSV_EVT));

           req.type = MQSV_EVT_MQD_CTRL;
           req.msg.mqd_ctrl.type = MQD_QGRP_CNT_GET;
           req.msg.mqd_ctrl.info.qgrp_cnt_info.info.queueName = pNode->queueName;

           /* Send the MDS sync request to remote MQND */
           rc = mqnd_mds_msg_sync_send(cb, NCSMDS_SVC_ID_MQD, cb->mqd_dest,
                                        &req, &rsp, MQSV_WAIT_TIME);

           if((rsp) && (rsp->type == MQSV_EVT_MQND_CTRL) &&
                 (rsp->msg.mqd_ctrl.type == MQND_CTRL_EVT_QGRP_CNT_RSP) &&
                 (rsp->msg.mqd_ctrl.info.qgrp_cnt_info.error == SA_AIS_OK))
            {
             param->info.i_int = rsp->msg.mqd_ctrl.info.qgrp_cnt_info.info.noOfQueueGroupMemOf;
             param->i_length = 4;      
             param->i_fmat_id = NCSMIB_FMAT_INT;
            }
            else
             rc = NCSCC_RC_FAILURE;

           if(rsp)
             m_MMGR_FREE_MQSV_EVT(rsp,NCS_SERVICE_ID_MQND);
           
           break;
        } 
      default:  
          rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
    }
   return rc;
}
/***************************************************************************************
 * Name        :  samsgqueueentry_get
 *
 * Description :  
 *
 **************************************************************************************/

uns32  samsgqueueentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
   MQND_CB           *pcb=0;
   MQND_QUEUE_NODE   *qNode=0;
   MQND_QNAME_NODE   *pNode=0;
   SaNameT           queuename;
   uns32            i;

   m_NCS_OS_MEMSET(&queuename, 0, sizeof(SaNameT));
   pcb = (MQND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQND, arg->i_mib_key);

   if (pcb == NULL)
      return NCSCC_RC_NO_INSTANCE;
   if(arg->i_idx.i_inst_len ==0)
      return NCSCC_RC_NO_INSTANCE;

   queuename.length = (SaUint16T) arg->i_idx.i_inst_ids[0];
   for(i = 0; i<queuename.length; i++)
   {
      queuename.value[i] = (uns8) (arg->i_idx.i_inst_ids[i+1]);
   }

   mqnd_qname_node_get(pcb,queuename,&pNode);
   
   if(pNode == NULL)
   {
       ncshm_give_hdl(pcb->cb_hdl);
       return NCSCC_RC_NO_INSTANCE;
   }
   mqnd_queue_node_get(pcb,pNode->qhdl,&qNode);

   if(qNode == NULL)
    {
       ncshm_give_hdl(pcb->cb_hdl);
       return NCSCC_RC_NO_INSTANCE;
    }
  /* Checking for an unlinked queue */
   if(qNode->qinfo.sendingState == SA_MSG_QUEUE_UNAVAILABLE)
   {
       ncshm_give_hdl(pcb->cb_hdl);
       return NCSCC_RC_NO_INSTANCE;
   }
 
   *data = (NCSCONTEXT)&qNode->qinfo;

   ncshm_give_hdl(pcb->cb_hdl);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************************
 * Name        : samsgqueueentry_next
 *
 * Description :
 *
 ***************************************************************************************/

uns32  samsgqueueentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
                                NCSCONTEXT *data, uns32* next_inst_id,
                                uns32 *next_inst_id_len)
{
   MQND_CB          *pcb=0;
   MQND_QUEUE_NODE  *qNode=0;
   SaNameT          queuename;
   MQND_QNAME_NODE  *pNode=0;
   uns32             i;
   pcb = (MQND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQND, arg->i_mib_key);

   if (pcb == NULL)
      return NCSCC_RC_NO_INSTANCE;
   m_NCS_MEMSET(&queuename,'\0',sizeof(SaNameT));
   if(arg->i_idx.i_inst_len ==0)
   {
      mqnd_qname_node_getnext(pcb,queuename,&pNode);
      
      if(pNode == NULL)
      {
        ncshm_give_hdl(pcb->cb_hdl);
        return NCSCC_RC_NO_INSTANCE;
      }
   }
   else
   {
      queuename.length = (SaUint16T) arg->i_idx.i_inst_ids[0];
      for(i = 0; i<queuename.length; i++)
      {
        queuename.value[i] = (uns8) (arg->i_idx.i_inst_ids[i+1]);
      }
      mqnd_qname_node_getnext(pcb,queuename,&pNode);

      if(pNode == NULL)
      {
        ncshm_give_hdl(pcb->cb_hdl);
        return NCSCC_RC_NO_INSTANCE;
      }
  }
   mqnd_queue_node_get(pcb,pNode->qhdl,&qNode); 
   if(qNode == NULL)
   {
      ncshm_give_hdl(pcb->cb_hdl);
      return NCSCC_RC_NO_INSTANCE;
   }
   /* For an unlinked queue get the next queue which is not unlinked */
   while(qNode->qinfo.sendingState == SA_MSG_QUEUE_UNAVAILABLE)
   {
     queuename= qNode->qinfo.queueName;  
     mqnd_qname_node_getnext(pcb,queuename,&pNode);
     if(pNode == NULL)
     {
       ncshm_give_hdl(pcb->cb_hdl);
       return NCSCC_RC_NO_INSTANCE;
     }
     mqnd_queue_node_get(pcb,pNode->qhdl,&qNode);
     if(qNode == NULL)
     {
       ncshm_give_hdl(pcb->cb_hdl);
       return NCSCC_RC_NO_INSTANCE;
     }
   }    
   *data = (NCSCONTEXT) &qNode->qinfo;
   *next_inst_id_len= qNode->qinfo.queueName.length+1;
   next_inst_id[0] =qNode->qinfo.queueName.length;
   for(i=0;i< qNode->qinfo.queueName.length;i++)
   {
      *(next_inst_id+i+1) = (uns32)qNode->qinfo.queueName.value[i];
   }
   ncshm_give_hdl(pcb->cb_hdl);
   return NCSCC_RC_SUCCESS;
}

