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
 * Name        : samsgqueuepriorityentry_set
 *
 * Description : 
 *
 *******************************************************************/

uns32 samsgqueuepriorityentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSMIB_VAR_INFO* var_info,
                              NCS_BOOL test_flag)
{
   return NCSCC_RC_FAILURE;
}



/***********************************************************************
 * Name        : samsgqueuepriorityentry_setrow
 *
 * Description :
 *
 **********************************************************************/

uns32 samsgqueuepriorityentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                 NCSMIB_SETROW_PARAM_VAL* params,
                                 struct ncsmib_obj_info* obj_info,
                                 NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}



/**************************************************************************
 * Name         :  samsgqueuepriorityentry_extract
 *
 * Description  :
 *
 **************************************************************************/

uns32 samsgqueuepriorityentry_extract(NCSMIB_PARAM_VAL* param,
                                  NCSMIB_VAR_INFO* var_info,
                                  NCSCONTEXT data,
                                  NCSCONTEXT buffer)
{

   uns32 rc = NCSCC_RC_SUCCESS;
   SaMsgQueueUsageT *qUsage=0;
   switch(param->i_param_id)
   {
     case saMsgQueuePriorityQSize_ID:
          qUsage = (SaMsgQueueUsageT *)data;
          m_MQSV_SNMPTM_UNS64_TO_PARAM(param,buffer,(SaUint64T)qUsage->queueSize);
          break;
     case saMsgQueuePriorityQUsed_ID:
          qUsage = (SaMsgQueueUsageT *)data;
          m_MQSV_SNMPTM_UNS64_TO_PARAM(param,buffer,(SaUint64T)qUsage->queueUsed);
          break;
     case saMsgQueuePriorityQNumMessages_ID:
          qUsage = (SaMsgQueueUsageT *)data;
          param->info.i_int = qUsage->numberOfMessages;;
          param->i_length =4;
          param->i_fmat_id = NCSMIB_FMAT_INT;
          break;
            
     case saMsgQueuePriorityQNumFullErrors_ID:
     default:
          rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
   }
   return rc;
}

/***************************************************************************************
 * Name        :  samsgqueuepriorityentry_get
 *
 * Description :  
 *
 **************************************************************************************/

uns32  samsgqueuepriorityentry_get(NCSCONTEXT pcb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
   MQND_CB           *cb=0;
   MQND_QUEUE_NODE   *qNode=0;
   MQND_QNAME_NODE   *pNode=0;
   SaNameT           queuename;
   uns32             i;
   uns32             len,offset;
   uns32             priority_index=0;
   MQND_QUEUE_CKPT_INFO *shm_base_addr;

   memset(&queuename, 0, sizeof(SaNameT));
   cb = (MQND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQND, arg->i_mib_key);

   if (cb == NULL)
      return NCSCC_RC_NO_INSTANCE;
   if(arg->i_idx.i_inst_len ==0)
      return NCSCC_RC_FAILURE;

   len = queuename.length = (SaUint16T) arg->i_idx.i_inst_ids[0];
   for(i = 0; i<queuename.length; i++)
   {
      queuename.value[i] = (uns8) (arg->i_idx.i_inst_ids[i+1]);
   }
    
   mqnd_qname_node_get(cb,queuename,&pNode);
   if(pNode == NULL)
   {
       ncshm_give_hdl(cb->cb_hdl);
       return NCSCC_RC_FAILURE;
   }
   mqnd_queue_node_get(cb,pNode->qhdl,&qNode); 

   if (qNode == NULL)
   {
      ncshm_give_hdl(cb->cb_hdl);
      return NCSCC_RC_NO_INSTANCE;
   }

  /* Checking for an unlinked queue */
   if(qNode->qinfo.sendingState == MSG_QUEUE_UNAVAILABLE)
   {
       ncshm_give_hdl(cb->cb_hdl);
       return NCSCC_RC_NO_INSTANCE;
   }

   if(arg->i_idx.i_inst_len >len +1 )
   {
      priority_index = ((SaUint32T) arg->i_idx.i_inst_ids[i+1]);   } 

   if(priority_index > SA_MSG_MESSAGE_LOWEST_PRIORITY ||
                  priority_index < SA_MSG_MESSAGE_HIGHEST_PRIORITY)
       return NCSCC_RC_NO_INSTANCE;

   if(arg->req.info.get_req.i_param_id == saMsgQueuePriorityQNumFullErrors_ID)
   {
      *data = (NCSCONTEXT)&qNode->qinfo.numberOfFullErrors[priority_index];
   }
   else
       {
        shm_base_addr = cb->mqnd_shm.shm_base_addr;
        offset = qNode->qinfo.shm_queue_index; 
        *data = (NCSCONTEXT)&(shm_base_addr[offset].QueueStatsShm.saMsgQueueUsage[priority_index]);
       }
   ncshm_give_hdl(cb->cb_hdl);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************************
 * Name        : samsgqueuepriorityentry_next
 *
 * Description :
 *
 ***************************************************************************************/

uns32  samsgqueuepriorityentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
                                NCSCONTEXT *data, uns32* next_inst_id,
                                uns32 *next_inst_id_len)
{
   MQND_CB          *cb=0;
   MQND_QNAME_NODE  *pNode=0;
   MQND_QUEUE_NODE  *qNode=0;
   SaNameT          queuename;
   uns32            priority_index=0;
   uns32            len,i,offset;
   MQND_QUEUE_CKPT_INFO *shm_base_addr;

   memset(&queuename,0,sizeof(SaNameT));
   cb = (MQND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQND, arg->i_mib_key);

   if (cb == NULL)
      return NCSCC_RC_NO_INSTANCE;
   if(arg->i_idx.i_inst_len !=0)
   { 
      len = queuename.length =  arg->i_idx.i_inst_ids[0];
      for(i = 0; i<queuename.length; i++)
      {
        queuename.value[i] = (uns8) (arg->i_idx.i_inst_ids[i+1]);
      }
      if(arg->i_idx.i_inst_len >len +1 )
         priority_index = (SaUint32T) arg->i_idx.i_inst_ids[i+1];   
      if(priority_index > SA_MSG_MESSAGE_LOWEST_PRIORITY ||
                  priority_index < SA_MSG_MESSAGE_HIGHEST_PRIORITY)
         return NCSCC_RC_NO_INSTANCE;
      if(priority_index == SA_MSG_MESSAGE_LOWEST_PRIORITY)
      {
         mqnd_qname_node_getnext(cb,queuename,&pNode);
         priority_index = SA_MSG_MESSAGE_HIGHEST_PRIORITY;
      }
      else 
      {
         priority_index++; 
         mqnd_qname_node_get(cb,queuename,&pNode);
      }
   }
   else
   {
      mqnd_qname_node_getnext(cb,queuename,&pNode);
      priority_index = SA_MSG_MESSAGE_HIGHEST_PRIORITY;
   }  

 
   if(pNode == NULL)
   {
    ncshm_give_hdl(cb->cb_hdl);
    return NCSCC_RC_NO_INSTANCE;
   }

   mqnd_queue_node_get(cb,pNode->qhdl,&qNode);

   if(qNode == NULL)
   {
    ncshm_give_hdl(cb->cb_hdl);
    return NCSCC_RC_NO_INSTANCE;
   }

/* Checking for an unlinked queue */
   while(qNode->qinfo.sendingState == MSG_QUEUE_UNAVAILABLE)
   {
     queuename= qNode->qinfo.queueName;
     mqnd_qname_node_getnext(cb,queuename,&pNode);
     if(pNode == NULL)
     {
       ncshm_give_hdl(cb->cb_hdl);
       return NCSCC_RC_NO_INSTANCE;
     }
     mqnd_queue_node_get(cb,pNode->qhdl,&qNode);
     if(qNode == NULL)
     {
       ncshm_give_hdl(cb->cb_hdl);
       return NCSCC_RC_NO_INSTANCE;
     }
   }
/* checked above for an unlinked queue */
   if(arg->req.info.next_req.i_param_id == saMsgQueuePriorityQNumFullErrors_ID)
      *data = (NCSCONTEXT)&qNode->qinfo.numberOfFullErrors[priority_index];
   else
       {
        shm_base_addr = cb->mqnd_shm.shm_base_addr;
        offset = qNode->qinfo.shm_queue_index;
        *data = (NCSCONTEXT)&(shm_base_addr[offset].QueueStatsShm.saMsgQueueUsage[priority_index]);
       }
         
   *next_inst_id_len= qNode->qinfo.queueName.length+2;
   next_inst_id[0] =qNode->qinfo.queueName.length;
   for(i=0;i< qNode->qinfo.queueName.length;i++)
   {
      *(next_inst_id+i+1) = (uns32)qNode->qinfo.queueName.value[i];
   }
   *(next_inst_id+i+1)=priority_index;/* Size of the priority indexc*/
   ncshm_give_hdl(cb->cb_hdl);
   return NCSCC_RC_SUCCESS;
}

