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
..............................................................................



..............................................................................

  DESCRIPTION: This file includes following routines:

   mqd_evt_process.......................MQD Event handler
   mqd_ctrl_evt_hdlr.....................MQD Control event handler
   mqd_user_evt_process..................MQD User Event handler
   mqd_user_evt_track_delete.............MQD User Event handler common function
   mqd_comp_evt_process..................MQD Comp specificmessage handler
   mqd_nd_status_evt_process.............MQD Nd Status event handler
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#include "mqd.h"

extern MQDLIB_INFO gl_mqdinfo;

MQD_EVT_HANDLER mqd_evt_tbl[MQSV_EVT_MAX] =
{
   mqd_asapi_evt_hdlr,
   0,
   0,   
   0,
   mqd_ctrl_evt_hdlr,
};

static uns32 mqd_user_evt_process(MQD_CB *, MDS_DEST *);
static uns32 mqd_comp_evt_process(MQD_CB *, NCS_BOOL *);
static uns32 mqd_quisced_process(MQD_CB* pMqd,MQD_QUISCED_STATE_INFO *quisced_info);
static uns32 mqd_qgrp_cnt_get_evt_process(MQD_CB * pMqd,MQSV_EVT *pevt);
static uns32 mqd_nd_status_evt_process(MQD_CB *pMqd,MQD_ND_STATUS_INFO *nd_info);
static uns32 mqd_cb_dump(void);
static void  mqd_dump_timer_info(MQD_TMR tmr);
static void  mqd_dump_obj_node(MQD_OBJ_NODE *qnode);
static void  mqd_dump_nodedb_node(MQD_ND_DB_NODE *qnode);
static NCS_BOOL mqd_obj_next_validate(MQD_CB *pMqd, SaNameT *name, MQD_OBJ_NODE **o_node);
static NCS_BOOL mqd_nodedb_next_validate(MQD_CB *pMqd, NODE_ID *node_id, MQD_ND_DB_NODE **o_node);

/****************************************************************************\
 PROCEDURE NAME : mqd_evt_process
  
 DESCRIPTION    : This is the function which is called when MQD receives any
                  event. Depending on the MQD events it received, the 
                  corresponding callback will be called.

 ARGUMENTS      : evt  - This is the pointer which holds the event structure.

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
uns32 mqd_evt_process(MQSV_EVT *pEvt)
{
   MQD_EVT_HANDLER   hdlr = 0;
   MQD_CB            *pMqd = 0;
   uns32             rc = NCSCC_RC_SUCCESS;
   
   /* Get the Controll block */
   pMqd = ncshm_take_hdl(NCS_SERVICE_ID_MQD, gl_mqdinfo.inst_hdl);
   if(!pMqd) {
      m_LOG_MQSV_D(MQD_DONOT_EXIST,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return NCSCC_RC_FAILURE;
   }

   /* Get the Event Handler */
   hdlr = mqd_evt_tbl[pEvt->type - 1];
   if(hdlr) 
    {
      if(SA_AMF_HA_ACTIVE == pMqd->ha_state)
       {
         rc = hdlr(pEvt, pMqd);
       }
      else
       if(SA_AMF_HA_STANDBY == pMqd->ha_state && 
           (MQD_ND_STATUS_INFO_TYPE == pEvt->msg.mqd_ctrl.type || MQD_MSG_TMR_EXPIRY == pEvt->msg.mqd_ctrl.type ))
        {
         rc = hdlr(pEvt, pMqd);
        }  
    }

   /* Destroy the event */ 
   if((MQSV_EVT_ASAPI == pEvt->type) && (pEvt->msg.asapi)) {
      asapi_msg_free(&pEvt->msg.asapi); 
   }
   m_MMGR_FREE_MQSV_EVT(pEvt, pMqd->my_svc_id);
   
   ncshm_give_hdl(pMqd->hdl);
   return rc;
} /* End of mqd_evt_process() */

/****************************************************************************\
 PROCEDURE NAME : mqd_ctrl_evt_hdlr

 DESCRIPTION    : This is the callback handler for MQD Controll events.

 ARGUMENTS      : pEvt - This is the pointer which holds the event structure.
                  pMqd - MQD Controll block

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
uns32 mqd_ctrl_evt_hdlr(MQSV_EVT *pEvt, MQD_CB *pMqd)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Check the message type and handle the message */
   if(MQD_MSG_USER == pEvt->msg.mqd_ctrl.type) {
      rc = mqd_user_evt_process(pMqd, &pEvt->msg.mqd_ctrl.info.user);
   }
   else if(MQD_MSG_COMP == pEvt->msg.mqd_ctrl.type) {
      rc = mqd_comp_evt_process(pMqd, &pEvt->msg.mqd_ctrl.info.init);
   }   
   if(MQD_MSG_TMR_EXPIRY == pEvt->msg.mqd_ctrl.type) {
      rc = mqd_timer_expiry_evt_process(pMqd, &pEvt->msg.mqd_ctrl.info.tmr_info.nodeid);
   } 
   else if(MQD_ND_STATUS_INFO_TYPE == pEvt->msg.mqd_ctrl.type) {
     rc = mqd_nd_status_evt_process(pMqd, &pEvt->msg.mqd_ctrl.info.nd_info);
   }else if (MQD_QUISCED_STATE_INFO_TYPE== pEvt->msg.mqd_ctrl.type){
        rc = mqd_quisced_process(pMqd,&pEvt->msg.mqd_ctrl.info.quisced_info);
   } else if( MQD_CB_DUMP_INFO_TYPE == pEvt->msg.mqd_ctrl.type){
           rc = mqd_cb_dump();
   } else if(MQD_QGRP_CNT_GET == pEvt->msg.mqd_ctrl.type){
       rc = mqd_qgrp_cnt_get_evt_process(pMqd,pEvt);     
   }
   return rc;
} /* End of mqd_ctrl_evt_hdlr() */

/****************************************************************************\
 PROCEDURE NAME : mqd_user_evt_process

 DESCRIPTION    : This routine process the user specific message.

 ARGUMENTS      : pMqd - MQD Control block pointer
                  dest - MDS_DEST pointer                  

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
static uns32 mqd_user_evt_process(MQD_CB *pMqd, MDS_DEST *dest)
{
   MQD_A2S_USER_EVENT_INFO user_evt;
   if ( (pMqd->active) && (pMqd->ha_state == SA_AMF_HA_ACTIVE ) )
   {  
      /* We need to scan the entire database and remove the track inforamtion
      * pertaining to the user
      */
      mqd_user_evt_track_delete(pMqd,dest);    
      memcpy(&user_evt.dest,dest,sizeof(MDS_DEST));
      /* Send async update to the stand by for MQD redundancy */
      mqd_a2s_async_update(pMqd, MQD_A2S_MSG_TYPE_USEREVT, (void*)&user_evt);

      return NCSCC_RC_SUCCESS; 
   }   
   return NCSCC_RC_FAILURE; 
} /* End of mqd_user_evt_process() */


/****************************************************************************\
 PROCEDURE NAME : mqd_user_evt_track_delete

 DESCRIPTION    : This routine updates the database.It deletes the
                  track information of the given destination.

 ARGUMENTS      : pMqd - MQD Control block pointer
                  dest - MDS_DEST pointer                  

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
uns32 mqd_user_evt_track_delete(MQD_CB *pMqd, MDS_DEST *dest)
{
   MQD_OBJ_NODE *pNode = 0;
   /* We need to scan the entire database and remove the track inforamtion
    * pertaining to the user
    */
  for(pNode = (MQD_OBJ_NODE *)ncs_patricia_tree_getnext(&pMqd->qdb, (uns8 *)0); pNode;
    pNode = (MQD_OBJ_NODE *)ncs_patricia_tree_getnext(&pMqd->qdb, pNode->node.key_info)) 
    {
      
       /* Delete the track information for the user */
       mqd_track_del(&pNode->oinfo.tlist, dest);             
    }
   return NCSCC_RC_SUCCESS;      
} /* End of mqd_user_evt_track_delete() */


/****************************************************************************\
 PROCEDURE NAME : mqd_comp_evt_process

 DESCRIPTION    : This routine process the comp specific message.

 ARGUMENTS      : pMqd - MQD Control block pointer
                  init - Component Initializtion flag                  

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
static uns32 mqd_comp_evt_process(MQD_CB *pMqd, NCS_BOOL *init)
{
   /* Set the Component status to ACTIVE */
   pMqd->active = *init;
   return NCSCC_RC_SUCCESS;
} /* End of mqd_comp_evt_process() */

/****************************************************************************\
 PROCEDURE NAME : mqd_timer_expiry_evt_process

 DESCRIPTION    : This routine process the comp specific message.

 ARGUMENTS      : pMqd - MQD Control block pointer
                  nodeid - NODE ID

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
uns32 mqd_timer_expiry_evt_process(MQD_CB *pMqd, NODE_ID *nodeid)
{
   MQD_ND_DB_NODE *pNdNode=0;
   MQD_OBJ_NODE *pNode = 0;
   MQD_A2S_MSG msg;
   uns32 rc= NCSCC_RC_SUCCESS;

   m_LOG_MQSV_D(MQD_TMR_EXPIRED,NCSFL_LC_TIMER,NCSFL_SEV_NOTICE,NCS_PTR_TO_UNS32_CAST(nodeid), __FILE__,__LINE__);
   pNdNode = (MQD_ND_DB_NODE *)ncs_patricia_tree_get(&pMqd->node_db, (uns8 *)nodeid);
  
   /* We need to scan the entire database and remove the track inforamtion
    * pertaining to the user
    */

   if(pNdNode==NULL)
    {
      rc= NCSCC_RC_FAILURE;
      return rc;
    }
   if(pMqd->ha_state == SA_AMF_HA_ACTIVE)
    {
     if(pNdNode->info.timer.tmr_id != TMR_T_NULL)
      {
       m_NCS_TMR_DESTROY(pNdNode->info.timer.tmr_id);
       pNdNode->info.timer.tmr_id = TMR_T_NULL;
      }
   
     pNode = (MQD_OBJ_NODE *)ncs_patricia_tree_getnext(&pMqd->qdb, (uns8 *)0); 
     while(pNode)
      {
       ASAPi_DEREG_INFO dereg;
       SaNameT   name;

       memset(&dereg, 0, sizeof(ASAPi_DEREG_INFO));
  
       name = pNode->oinfo.name;

       if(m_NCS_NODE_ID_FROM_MDS_DEST(pNode->oinfo.info.q.dest) == pNdNode->info.nodeid)
        {
         dereg.objtype = ASAPi_OBJ_QUEUE;
         dereg.queue   = pNode->oinfo.name;
         rc = mqd_asapi_dereg_hdlr(pMqd, &dereg, NULL);
        }

       pNode = (MQD_OBJ_NODE *)ncs_patricia_tree_getnext(&pMqd->qdb,(uns8 *) &name);
      }
     /* Send an async Update to the standby */
     memset(&msg, 0 , sizeof(MQD_A2S_MSG) );
     msg.type= MQD_A2S_MSG_TYPE_MQND_TIMER_EXPEVT;
     msg.info.nd_tmr_exp_evt.nodeid=pNdNode->info.nodeid;
     /* Send async update to the standby for MQD redundancy */
     mqd_a2s_async_update(pMqd, MQD_A2S_MSG_TYPE_MQND_TIMER_EXPEVT,(void *)(&msg.info.nd_tmr_exp_evt));
    
     if(pNdNode)
       mqd_red_db_node_del(pMqd, pNdNode);
     #ifdef NCS_MQD 
      m_NCS_CONS_PRINTF("MQND TMR EXPIRY PROCESSED ON ACTIVE\n"); 
     #endif
    }
    else
     if(pMqd->ha_state == SA_AMF_HA_STANDBY) 
      {
       pNdNode->info.timer.is_expired = TRUE; 
       #ifdef NCS_MQD
        m_NCS_CONS_PRINTF("MQND TMR EXPIRY PROCESSED ON STANDBY\n"); 
       #endif
      }
    return rc;
} /* End of mqd_timer_expiry_evt_process() */



/****************************************************************************\
 PROCEDURE NAME : mqd_nd_status_evt_process

 DESCRIPTION    : This routine process the MQND status event.

 ARGUMENTS      : pMqd - MQD Control block pointer
                  nd_info - ND information                  

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
static uns32 mqd_nd_status_evt_process(MQD_CB *pMqd,MQD_ND_STATUS_INFO *nd_info)
{
   uns32          rc= NCSCC_RC_SUCCESS;
   void           *ptr=0;
   MQD_ND_DB_NODE *pNdNode=0;
   SaTimeT        timeout= m_NCS_CONVERT_SATIME_TO_TEN_MILLI_SEC(MQD_ND_EXPIRY_TIME_STANDBY);
   NODE_ID        node_id=0;
   MQD_A2S_MSG    msg;
   
   #ifdef NCS_MQD
    m_NCS_CONS_PRINTF("MQND status:MDS EVT :processing %d\n", m_NCS_NODE_ID_FROM_MDS_DEST(nd_info->dest));
   #endif

   /* Process MQND Related events */
   if(nd_info->is_up == FALSE)
    {
     pNdNode =m_MMGR_ALLOC_MQD_ND_DB_NODE;
     if(pNdNode ==NULL)
      {
       rc = NCSCC_RC_FAILURE;
       m_LOG_MQSV_D(MQD_MEMORY_ALLOC_FAIL,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
       return rc;
      }
     memset(pNdNode, 0, sizeof(MQD_ND_DB_NODE));
     pNdNode->info.nodeid = m_NCS_NODE_ID_FROM_MDS_DEST(nd_info->dest);
     pNdNode->info.is_restarted= FALSE;
     pNdNode->info.timer.type= MQD_ND_TMR_TYPE_EXPIRY;
     pNdNode->info.timer.tmr_id=0;
     pNdNode->info.timer.nodeid=pNdNode->info.nodeid;
     pNdNode->info.timer.uarg= pMqd->hdl;
     pNdNode->info.timer.is_active = FALSE;
     mqd_red_db_node_add(pMqd, pNdNode);

     mqd_tmr_start(&pNdNode->info.timer,timeout);

     if(pMqd->ha_state == SA_AMF_HA_ACTIVE)
      mqd_nd_down_update_info(pMqd,nd_info->dest);
     #ifdef NCS_MQD
      m_NCS_CONS_PRINTF("MDS DOWN PROCESSED ON %d DONE\n",pMqd->ha_state);
     #endif
    }
   else
    {
     node_id= m_NCS_NODE_ID_FROM_MDS_DEST(nd_info->dest);
     pNdNode = (MQD_ND_DB_NODE *)ncs_patricia_tree_get( &pMqd->node_db, (uns8 *)&node_id);
     if(pNdNode)
      {
       mqd_tmr_stop(&pNdNode->info.timer);
       pNdNode->info.is_restarted= TRUE;
       pNdNode->info.dest = nd_info->dest; 
       if(pMqd->ha_state == SA_AMF_HA_ACTIVE)
        {
         mqd_red_db_node_del(pMqd, pNdNode);
         mqd_nd_restart_update_dest_info(pMqd,nd_info->dest);
         /* Send an async update event to standby MQD*/         
         memset(&msg, 0, sizeof(MQD_A2S_MSG));
         msg.type=MQD_A2S_MSG_TYPE_MQND_STATEVT;
         msg.info.nd_stat_evt.nodeid =m_NCS_NODE_ID_FROM_MDS_DEST(nd_info->dest);
         msg.info.nd_stat_evt.is_restarting= nd_info->is_up;
         msg.info.nd_stat_evt.downtime= nd_info->event_time;
         ptr= &(msg.info.nd_stat_evt);
         mqd_a2s_async_update(pMqd,MQD_A2S_MSG_TYPE_MQND_STATEVT, (void *)(&msg.info.nd_stat_evt));
        }
      }
     #ifdef NCS_MQD
      m_NCS_CONS_PRINTF("MDS UP PROCESSED ON %d DONE\n",pMqd->ha_state);
     #endif
    }
   return rc;
}


/****************************************************************************\
 PROCEDURE NAME : mqd_quisced_process

 DESCRIPTION    : This routine process the Quisced ack event.

 ARGUMENTS      : pMqd - MQD Control block pointer
                  quisced_info - MQD_QUISCED_STATE_INFO structure pointer

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
static uns32 mqd_quisced_process(MQD_CB* pMqd,MQD_QUISCED_STATE_INFO *quisced_info)
{
  SaAisErrorT saErr= SA_AIS_OK; 
  uns32 rc= NCSCC_RC_SUCCESS;
  if(pMqd && pMqd->is_quisced_set)
  {
     pMqd->ha_state = SA_AMF_HA_QUIESCED;
     rc= mqd_mbcsv_chgrole(pMqd);
     if(rc!= NCSCC_RC_SUCCESS)
     {
       m_LOG_MQSV_D(MQD_EVT_QUISCED_PROCESS_MBCSVCHG_ROLE_FAILURE,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
       return rc;
     }
     saAmfResponse(pMqd->amf_hdl, quisced_info->invocation, saErr);
     pMqd->is_quisced_set = FALSE;
     m_LOG_MQSV_D(MQD_EVT_QUISCED_PROCESS_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
  }
  else
     m_LOG_MQSV_D(MQD_EVT_UNSOLICITED_QUISCED_ACK,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
     
  return rc;
}


/****************************************************************************\
 PROCEDURE NAME : mqd_cb_dump

 DESCRIPTION    : This routine prints the Control Block elements.

 ARGUMENTS      :
                 

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
static uns32 mqd_cb_dump(void)
{
  MQD_CB            *pMqd = NULL;
  MQD_OBJ_NODE      *qnode=NULL;
  MQD_ND_DB_NODE    *qNdnode=NULL;
  SaNameT           qname;
  NODE_ID           nodeid=0;
  memset(&qname, 0,sizeof(SaNameT));
  /* Get the Controll block */
  pMqd = ncshm_take_hdl(NCS_SERVICE_ID_MQD, gl_mqdinfo.inst_hdl);
  if(!pMqd) 
  {
    m_LOG_MQSV_D(MQD_DONOT_EXIST,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__);
    return NCSCC_RC_FAILURE;
  }
  m_NCS_OS_PRINTF("************ MQD CB Dump  *************** \n");
  m_NCS_OS_PRINTF("**** Global Control Block Details ***************\n\n");

  m_NCS_OS_PRINTF(" Self MDS Handle is : %u \n",(uns32)pMqd->my_mds_hdl);
  m_NCS_OS_PRINTF("MQD MDS dest Nodeid is : %u\n", m_NCS_NODE_ID_FROM_MDS_DEST(pMqd->my_dest));
  m_NCS_OS_PRINTF("MQD MDS dest is        : %llu \n", pMqd->my_dest);
  m_NCS_OS_PRINTF("Service ID of MQD is : %u \n",pMqd->my_svc_id);
  m_NCS_OS_PRINTF("Component name of MQD: %s \n",pMqd->my_name);
  m_NCS_OS_PRINTF("Async update counter : %u \n",pMqd->mqd_sync_updt_count);
  m_NCS_OS_PRINTF(" Last Recorded name  : %s \n",pMqd->record_qindex_name.value);
  m_NCS_OS_PRINTF(" ColdSyn or WarmSync on :%u \n",pMqd->cold_or_warm_sync_on); 
  m_NCS_OS_PRINTF("AMF Handle is        :%llu  \n",pMqd->amf_hdl);
  m_NCS_OS_PRINTF("My HA State is       :%u  \n",pMqd->ha_state);
  m_NCS_OS_PRINTF("MBCSV HAndle is      :%u  \n",pMqd->mbcsv_hdl);
  m_NCS_OS_PRINTF("MBCSV Opened Checkpoint Handle is :%u \n",pMqd->o_ckpt_hdl);
  m_NCS_OS_PRINTF("MBCSV Selection Object is :%llu \n",pMqd->mbcsv_sel_obj);
  m_NCS_OS_PRINTF("CB Structure Handle is :%u \n",pMqd->hdl);
  m_NCS_OS_PRINTF("OAC Handle is          :%u \n",pMqd->oac_hdl);
  m_NCS_OS_PRINTF("Component Active flag is:%u \n",pMqd->active);
  m_NCS_OS_PRINTF(" Invocation of Quisced State is:%llu \n",pMqd->invocation);
  m_NCS_OS_PRINTF(" IS the invocation from the Quisced state set :%u \n",pMqd->is_quisced_set);

  m_NCS_OS_PRINTF( "********* Printing the Queue Data base********** \n");
  if(pMqd->qdb_up)
  {
    m_NCS_OS_PRINTF("Queue Data Base is Ready\n ");
    m_NCS_OS_PRINTF("Total number of nodes in main database :%u\n ",pMqd->qdb.n_nodes);
    mqd_obj_next_validate(pMqd, NULL, &qnode);
    while(qnode)
    {
      mqd_dump_obj_node(qnode);
      qname= qnode->oinfo.name;  
      mqd_obj_next_validate(pMqd, &qname, &qnode); 
    }
  }
  else
    m_NCS_OS_PRINTF("Queue Data Baseis not  Ready\n");
  
  if(pMqd->node_db_up)
  {
    m_NCS_OS_PRINTF("Queue Node Data Base is Ready\n ");
    mqd_nodedb_next_validate(pMqd, &nodeid, &qNdnode);
    while(qNdnode)
    {
      mqd_dump_nodedb_node(qNdnode);
      nodeid= qNdnode->info.nodeid;
      mqd_nodedb_next_validate(pMqd,&nodeid, &qNdnode);   
    }
  }
  else
    m_NCS_OS_PRINTF("Queue Node Data Baseis not  Ready\n");
  
  ncshm_give_hdl(gl_mqdinfo.inst_hdl);
 
  return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 PROCEDURE NAME : mqd_qgrp_cnt_get_evt_process 

 DESCRIPTION    : This routine process the qcount get event .

 ARGUMENTS      : pMqd - MQD Control block pointer
                  pEvt - Event structure

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
static uns32 mqd_qgrp_cnt_get_evt_process(MQD_CB *pMqd,MQSV_EVT *pevt)
{
   MQD_OBJ_NODE  *pObjNode = 0;
   MQSV_EVT       rsp;
   uns32   rc = NCSCC_RC_SUCCESS;
   MQSV_CTRL_EVT_QGRP_CNT *qgrp_cnt_info = &pevt->msg.mqd_ctrl.info.qgrp_cnt_info;

   memset(&rsp,0,sizeof(MQSV_EVT));

   if(pMqd->ha_state == SA_AMF_HA_ACTIVE) 
    {
     rsp.type = MQSV_EVT_MQND_CTRL;
     rsp.msg.mqnd_ctrl.type = MQND_CTRL_EVT_QGRP_CNT_RSP;
     rsp.msg.mqnd_ctrl.info.qgrp_cnt_info.info.queueName = qgrp_cnt_info->info.queueName;

     pObjNode = (MQD_OBJ_NODE *)ncs_patricia_tree_get(&pMqd->qdb, (uns8 *)&qgrp_cnt_info->info.queueName);
     if(pObjNode)
      {
       rsp.msg.mqnd_ctrl.info.qgrp_cnt_info.error = SA_AIS_OK;
       rsp.msg.mqnd_ctrl.info.qgrp_cnt_info.info.noOfQueueGroupMemOf = pObjNode->oinfo.ilist.count;
      }
     else
      {
       rsp.msg.mqnd_ctrl.info.qgrp_cnt_info.error = SA_AIS_ERR_NOT_EXIST;
       rsp.msg.mqnd_ctrl.info.qgrp_cnt_info.info.noOfQueueGroupMemOf = 0;
      }
     rc = mqd_mds_send_rsp(pMqd,&pevt->sinfo,&rsp);
     if(rc != NCSCC_RC_SUCCESS) 
       m_LOG_MQSV_D(MQD_REG_HDLR_MIB_EVT_SEND_FAILED,NCSFL_LC_MQSV_QGRP_MGMT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
     else
       m_LOG_MQSV_D(MQD_REG_HDLR_MIB_EVT_SEND_SUCCESS,NCSFL_LC_MQSV_QGRP_MGMT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
    }
   return rc;
}
   

/****************************************************************************\
 PROCEDURE NAME : mqd_obj_next_validate

 DESCRIPTION    : This routine will give the next node of the MQD Queue database.

 ARGUMENTS      :pMqd - Control Block pointer
                 name - Name of the queue or the queuegroup
                 o_node- The pointer to the next node if available

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/

static NCS_BOOL
mqd_obj_next_validate(MQD_CB *pMqd, SaNameT *name, MQD_OBJ_NODE **o_node)
{
   MQD_OBJ_NODE   *pObjNode = 0;

   /* Get hold of the MQD controll block */
   /*m_HTON_SANAMET_LEN(name->length); */
   pObjNode = (MQD_OBJ_NODE *)ncs_patricia_tree_getnext(&pMqd->qdb, (char *)name);
   /*m_NTOH_SANAMET_LEN(name->length); */

   *o_node = pObjNode;
   if(pObjNode) return TRUE;
   return FALSE;
} /* End of mqd_obj_next_validate() */

/****************************************************************************\

 DESCRIPTION    : mqd_nodedb_next_validate

 ARGUMENTS      :pMqd - Control block pointer
                 node_id- Node id of the node
                 o_node - Pointer to the next Queue Node database node 

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/

static NCS_BOOL
mqd_nodedb_next_validate(MQD_CB *pMqd, NODE_ID *node_id, MQD_ND_DB_NODE **o_node)
{
   MQD_ND_DB_NODE   *pNdNode = 0;

   /* Get hold of the MQD controll block */
   pNdNode = (MQD_ND_DB_NODE *)ncs_patricia_tree_getnext(&pMqd->node_db,(uns8 *)node_id);

   *o_node = pNdNode;
   if(pNdNode) return TRUE;
   return FALSE;
} /* End of mqd_nodedb_next_validate() */

/****************************************************************************\
 PROCEDURE NAME : mqd_dump_nodedb_node

 DESCRIPTION    : This routine prints the Node Database node information.

 ARGUMENTS      : qnode - MQD Node database node
                 

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/

static void mqd_dump_nodedb_node(MQD_ND_DB_NODE *qnode)
{
  m_NCS_OS_PRINTF(" The node id is : %u\n",qnode->info.nodeid);
  m_NCS_OS_PRINTF(" Value of MQND is _restarted is : %u\n",qnode->info.is_restarted);
  m_NCS_OS_PRINTF("MQD Timer Information for this node\n ");
  mqd_dump_timer_info(qnode->info.timer);
  return;
}

/****************************************************************************\
 PROCEDURE NAME : mqnd_dump_timer_info

 DESCRIPTION    : This routine prints the MQD timer information of that node.

 ARGUMENTS      :tmr - MQD Timer structure
                 

 RETURNS        :
\*****************************************************************************/

static void mqd_dump_timer_info(MQD_TMR tmr)
{
   m_NCS_OS_PRINTF("\n Timer Type is : %d\n",tmr.type);
   m_NCS_OS_PRINTF("\n Tmr_t is : %p\n",tmr.tmr_id);
   m_NCS_OS_PRINTF("\n Queue Handle is : %u\n",tmr.nodeid);
   m_NCS_OS_PRINTF("\n uarg : %u\n",tmr.uarg);
   m_NCS_OS_PRINTF("\n Is Active : %d\n",tmr.is_active);
}

/****************************************************************************\
 PROCEDURE NAME : mqnd_dump_timer_info

 DESCRIPTION    : This routine prints the MQD timer information of that node.

 ARGUMENTS      :tmr - MQD Timer structure
                 

 RETURNS        :
\*****************************************************************************/

static void mqd_dump_obj_node(MQD_OBJ_NODE *qnode)
{
   NCS_Q_ITR         itr;
   MQD_TRACK_OBJ     *pTrack = 0;
   MQD_OBJECT_ELEM      *pilist =0;
   if(qnode == NULL)
   {
     return;
   }
   m_NCS_OS_PRINTF(" The Qnode value is : %p \n",qnode);
   m_NCS_OS_PRINTF(" The Qnode Ohjinfo pointer value is : %p \n",&(qnode->oinfo));
   if(qnode->oinfo.type == MQSV_OBJ_QGROUP)
   {
     m_NCS_OS_PRINTF("Queue Group Name is : %s\n",qnode->oinfo.name.value);
     switch(qnode->oinfo.info.qgrp.policy)
     {
       case SA_MSG_QUEUE_GROUP_ROUND_ROBIN:
            m_NCS_OS_PRINTF("Policy is :Round Robini\n");
            break;
       case SA_MSG_QUEUE_GROUP_LOCAL_ROUND_ROBIN:
            m_NCS_OS_PRINTF("Policy is :Local Round Robini\n");
            break;
       case SA_MSG_QUEUE_GROUP_LOCAL_BEST_QUEUE:
            m_NCS_OS_PRINTF("Policy is :Local Best Queue\n"); 
            break;
        case SA_MSG_QUEUE_GROUP_BROADCAST:
            m_NCS_OS_PRINTF("Policy is :Group Broadcast\n");
            break;
        default:
            m_NCS_OS_PRINTF("Policy is :Unknown\n");
     }
   }
   else if(qnode->oinfo.type == MQSV_OBJ_QUEUE)
   { 
     m_NCS_OS_PRINTF("Queue Name is : %s\n",qnode->oinfo.name.value);
     if(qnode->oinfo.info.q.send_state ==MSG_QUEUE_AVAILABLE)
        m_NCS_OS_PRINTF("The sending state is : MSG_QUEUE_AVAILABLE \n");
     else if(qnode->oinfo.info.q.send_state ==MSG_QUEUE_UNAVAILABLE)
        m_NCS_OS_PRINTF("The sending state is : MSG_QUEUE_UNAVAILABLE \n");
     m_NCS_OS_PRINTF(" Retention Time is : %llu \n",qnode->oinfo.info.q.retentionTime);
     m_NCS_OS_PRINTF(" MDS Destination is :%llu \n",qnode->oinfo.info.q.dest);
     m_NCS_OS_PRINTF(" Node id from the MDS Destination of the queue is :%u \n",m_NCS_NODE_ID_FROM_MDS_DEST(qnode->oinfo.info.q.dest));
     switch(qnode->oinfo.info.q.owner)
     {
        case MQSV_QUEUE_OWN_STATE_ORPHAN:
             m_NCS_OS_PRINTF(" Ownership is: MQSV_QUEUE_OWN_STATE_ORPHAN \n");
             break;
        case MQSV_QUEUE_OWN_STATE_OWNED:
             m_NCS_OS_PRINTF(" Owner ship is: MQSV_QUEUE_OWN_STATE_OWNED\n");
             break;
        case MQSV_QUEUE_OWN_STATE_PROGRESS:
             m_NCS_OS_PRINTF(" Owner ship is:MQSV_QUEUE_OWN_STATE_PROGRESS\n");
             break;
        default:
             m_NCS_OS_PRINTF(" Owner ship is:Unknown \n");
     }  
     m_NCS_OS_PRINTF(" Queue Handle is : %u", qnode->oinfo.info.q.hdl);
     if(qnode->oinfo.info.q.adv)
       m_NCS_OS_PRINTF(" Advertisement flag is set \n");
     else
       m_NCS_OS_PRINTF(" Advertisement flag is not set \n");
     m_NCS_OS_PRINTF(" Is mqnd down for this queue is :%d \n",qnode->oinfo.info.q.is_mqnd_down);
     m_NCS_OS_PRINTF(" Creation time for this queue is :%llu \n",qnode->oinfo.creationTime);
  } 
  m_NCS_OS_PRINTF(" \n*********************Printing the ilist******************* \n");
  memset(&itr,0,sizeof(NCS_Q_ITR));
  itr.state = 0;
  while((pilist = (MQD_OBJECT_ELEM *)ncs_walk_items(&qnode->oinfo.ilist, &itr)))          
  {
    m_NCS_OS_PRINTF(" The ilist member pointer value is: %p \n",pilist->pObject);
    m_NCS_OS_PRINTF(" The ilist member Name is : %s \n",pilist->pObject->name.value);  
  }
  m_NCS_OS_PRINTF("\n********** End of the ilist************* \n");
  m_NCS_OS_PRINTF("\n********** Printing the track list************* \n");
  memset(&itr,0,sizeof(NCS_Q_ITR));
  itr.state = 0;
  while((pTrack = (MQD_TRACK_OBJ *)ncs_walk_items(&qnode->oinfo.tlist, &itr))) 
  {
      m_NCS_OS_PRINTF(" To service is :%u \n", pTrack->to_svc);
      m_NCS_OS_PRINTF(" The MDSdest destination of the track subscibed element is: %llu \n",pTrack->dest);
      m_NCS_OS_PRINTF(" The Nodeid from MDSdest of the track subscibed element is: %u \n",m_NCS_NODE_ID_FROM_MDS_DEST(pTrack->dest));
  }
  m_NCS_OS_PRINTF("\n********** End of the track list************* \n");
 
}
