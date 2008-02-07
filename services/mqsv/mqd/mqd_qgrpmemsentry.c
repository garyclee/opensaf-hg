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

#include "mqd.h"


static NCS_BOOL mqd_obj_compare(void* key, void* elem);


/********************************************************************
 * Name        : samsgqueuegroupmembersentry_set
 *
 * Description : 
 *
 *******************************************************************/

uns32 samsgqueuegroupmembersentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSMIB_VAR_INFO* var_info,
                              NCS_BOOL test_flag)
{
   return NCSCC_RC_FAILURE;
}



/***********************************************************************
 * Name        : samsgqueuegroupmembersentry_setrow
 *
 * Description :
 *
 **********************************************************************/

uns32 samsgqueuegroupmembersentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                 NCSMIB_SETROW_PARAM_VAL* params,
                                 struct ncsmib_obj_info* obj_info,
                                 NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}



/**************************************************************************
 * Name         :  samsgqueuegroupmembersentry_extract
 *
 * Description  :
 *
 **************************************************************************/

uns32 samsgqueuegroupmembersentry_extract(NCSMIB_PARAM_VAL* param,
                                  NCSMIB_VAR_INFO* var_info,
                                  NCSCONTEXT data,
                                  NCSCONTEXT buffer)
{

   uns32 rc = NCSCC_RC_SUCCESS;
   MQD_OBJ_INFO *qnode=0;
   qnode = data;
   if(var_info->param_id ==saMsgQueueGroupMembersQueueName_ID )
   {
      param->i_fmat_id = NCSMIB_FMAT_OCT;
      param->i_length  = qnode->name.length;
      m_NCS_MEMCPY((uns8 *)buffer,qnode->name.value,param->i_length);
      param->info.i_oct     = (uns8*)buffer;
   }
   else
      rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
   return rc;
}

/***************************************************************************************
 * Name        :  samsgqueuegroupmembersentry_get
 *
 * Description :  
 *
 **************************************************************************************/


uns32  samsgqueuegroupmembersentry_get(NCSCONTEXT pcb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
   MQD_CB           *cb;
   MQD_OBJ_NODE     *pNode=0;
   SaNameT           queuename,memberQueueName; 
   uns32             i,counter,max_value;
   MQD_OBJ_INFO      *qNode=0;

   cb = (MQD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQD, arg->i_mib_key);

   if (cb == NULL)
      return NCSCC_RC_NO_INSTANCE;
   if(arg->i_idx.i_inst_len ==0)
      return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(&queuename, '\0', sizeof(SaNameT));
   m_NCS_OS_MEMSET(&memberQueueName, '\0', sizeof(SaNameT));

   queuename.length = (SaUint16T) arg->i_idx.i_inst_ids[0];
   for(i = 0; i<queuename.length; i++)
   {
      queuename.value[i] = (uns8) (arg->i_idx.i_inst_ids[i+1]);
   }
   memberQueueName.length = (SaUint16T) arg->i_idx.i_inst_ids[i+1];
   
   i++;
   max_value =i+ memberQueueName.length;

   for(counter=0; i<max_value; counter++,i++)
   {
      memberQueueName.value[counter]= (uns8) (arg->i_idx.i_inst_ids[i+1]);
   }  
   pNode= (MQD_OBJ_NODE *)ncs_patricia_tree_get(&cb->qdb, (uns8 *)&queuename);
   if(pNode == NULL)
   {
       ncshm_give_hdl(cb->hdl);
       return NCSCC_RC_FAILURE;
   }
   if(pNode->oinfo.type == MQSV_OBJ_QUEUE)
   {
       ncshm_give_hdl(cb->hdl);
       return NCSCC_RC_NO_INSTANCE;
   }
   
   mqd_get_node_from_group_for_get(pNode, &qNode,memberQueueName);
   if(qNode == NULL)
   {
       ncshm_give_hdl(cb->hdl);
       return NCSCC_RC_FAILURE;
   }
     
   *data = (NCSCONTEXT)qNode;
   ncshm_give_hdl(cb->hdl);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************************
 * Name        : samsgqueuegroupmembersentry_next
 *
 * Description :
 *
 ***************************************************************************************/

uns32  samsgqueuegroupmembersentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
                                NCSCONTEXT *data, uns32* next_inst_id,
                                uns32 *next_inst_id_len)
{
   MQD_CB            *cb=0;
   MQD_OBJ_NODE      *pNode=0;
   MQD_OBJ_INFO      *qNode=0;
   SaNameT           queuename;
   SaNameT           memberQueueName;
   uns32             i,counter;

   cb = (MQD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQD, arg->i_mib_key);

   if (cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   m_NCS_OS_MEMSET(&queuename, '\0', sizeof(SaNameT));
   m_NCS_OS_MEMSET(&memberQueueName, '\0', sizeof(SaNameT));

   if(arg->i_idx.i_inst_len ==0)
   {
      pNode = (MQD_OBJ_NODE *) ncs_patricia_tree_getnext(&cb->qdb, (uns8 *)NULL);
      while(pNode)
       {
          if(pNode->oinfo.type == MQSV_OBJ_QUEUE)
            {
             queuename = pNode->oinfo.name;
             pNode = (MQD_OBJ_NODE *) ncs_patricia_tree_getnext(&cb->qdb, (uns8 *)&queuename);
            }
          else
            break;
       }
   }
   else
   {
      queuename.length = (SaUint16T) arg->i_idx.i_inst_ids[0];
      for(i = 0; i<queuename.length; i++)
      {
         queuename.value[i] = (uns8) (arg->i_idx.i_inst_ids[i+1]);
      } 
      i++;
      memberQueueName.length = (SaUint16T) arg->i_idx.i_inst_ids[i];

      for(counter=0; counter < memberQueueName.length; counter++,i++)
      {
         memberQueueName.value[counter]= (uns8) (arg->i_idx.i_inst_ids[i+1]);
      }
      pNode= (MQD_OBJ_NODE *)ncs_patricia_tree_get(&cb->qdb, (uns8 *)&queuename);   
      if(pNode == NULL)
      {
         ncshm_give_hdl(cb->hdl);
         return NCSCC_RC_NO_INSTANCE;
      }
   }
   if(pNode == NULL)
   {
      ncshm_give_hdl(cb->hdl);
      return NCSCC_RC_NO_INSTANCE;
   }
   
   mqd_get_node_from_group_for_getnext(cb,&pNode,&qNode,memberQueueName);
   if(qNode == NULL)
   {
      ncshm_give_hdl(cb->hdl);
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT) qNode;


  *next_inst_id_len= pNode->oinfo.name.length+ qNode->name.length+2;
   next_inst_id[0] =pNode->oinfo.name.length;
   for(counter=0;counter< pNode->oinfo.name.length;counter++)
   {
      *(next_inst_id+counter+1) = (uns32)pNode->oinfo.name.value[counter];
   }
   counter++;
   next_inst_id[counter] =qNode->name.length;
   for(i=0;i< qNode->name.length;i++,counter++)
   {
      *(next_inst_id+counter+1) = (uns32)qNode->name.value[i];
   }
   ncshm_give_hdl(cb->hdl);
   return NCSCC_RC_SUCCESS;
}


void mqd_get_node_from_group_for_get(MQD_OBJ_NODE *pNode,MQD_OBJ_INFO **qnode,SaNameT memberQueueName)
{ 
   NCS_QELEM* front  = (NCS_QELEM*) &pNode->oinfo.ilist.head;
   NCS_QELEM* behind = (NCS_QELEM*)&pNode->oinfo.ilist.head;
   MQD_OBJECT_ELEM  *front_ptr=0; 
   while(front != NCS_QELEM_NULL)
   {
      if (mqd_obj_compare((void *)&memberQueueName,front) == TRUE)
      {
         front_ptr = (MQD_OBJECT_ELEM *)front;
         *qnode = front_ptr->pObject;
         return;
      }
      behind = front;
      front  = front->next;
   }
   *qnode = NULL;
   return;
}

static NCS_BOOL mqd_obj_compare(void* key, void* elem)
{
   MQD_OBJECT_ELEM   *pOelm = (MQD_OBJECT_ELEM *)elem;
   SaNameT           *local_key = (SaNameT *)key;

   if(!m_NCS_OS_MEMCMP(pOelm->pObject->name.value, local_key->value, local_key->length)) 
   {
      return TRUE;
   }
   return FALSE;
} /* End of mqd_obj_compare() */


void mqd_get_node_from_group_for_getnext(MQD_CB *cb,MQD_OBJ_NODE **grpNode,MQD_OBJ_INFO **qNode,SaNameT memberQueueName)
{
   SaNameT          localQueueName;
   NCS_BOOL         found= FALSE;
   MQD_OBJ_NODE     *pNode= *grpNode;
   NCS_QUEUE        *queue = &pNode->oinfo.ilist;
   NCS_QELEM        *front= queue->head;
   MQD_OBJECT_ELEM  *localQueuePtr = NULL;
   MQD_OBJECT_ELEM  *localQueuePtr2 = NULL;
   MQD_OBJECT_ELEM  *frontptr=(MQD_OBJECT_ELEM *)front;
   m_NCS_OS_MEMSET(&localQueueName, 0, sizeof(SaNameT));
   localQueueName = memberQueueName;
   front= queue->head;

 while(pNode)
 { 
   while(front != NCS_QELEM_NULL)
   {
       frontptr = (MQD_OBJECT_ELEM *)front;
       if((m_NCS_MEMCMP(&frontptr->pObject->name,&memberQueueName,sizeof(SaNameT)))>0)
       {
           if(!localQueuePtr)
           {
             localQueueName = frontptr->pObject->name;
             localQueuePtr = (MQD_OBJECT_ELEM *)front;
             found= TRUE;
           }
           else if((m_NCS_MEMCMP(&frontptr->pObject->name,&localQueuePtr->pObject->name,sizeof(SaNameT)))<0)
           {
             localQueuePtr2 = localQueuePtr;
             localQueuePtr= frontptr;
           }
       }
      front = front->next;
   }
   if((found== TRUE)&&(localQueuePtr))
   {
      *qNode = localQueuePtr->pObject;
      *grpNode= pNode;
      return;
   }
   else 
   {
      localQueueName = pNode->oinfo.name;
      pNode = (MQD_OBJ_NODE *)ncs_patricia_tree_getnext(&cb->qdb,
                                            (uns8 *)&localQueueName);
      while(pNode)
      {
         if(pNode->oinfo.type == MQSV_OBJ_QUEUE)
         {
           localQueueName = pNode->oinfo.name;
           pNode = (MQD_OBJ_NODE *) ncs_patricia_tree_getnext(&cb->qdb, (uns8 *)&localQueueName);
         }
         else
             break;
      }
      found = FALSE;
      if(pNode ==NULL)
      {
         *grpNode= pNode;
         *qNode =NULL;
         return;
      }
      m_NCS_MEMSET(&memberQueueName, 0, sizeof(SaNameT));
      queue= &pNode->oinfo.ilist;
      front =queue->head;
      localQueuePtr=NULL; 
      localQueuePtr2=NULL;
   }
 }
}
   
