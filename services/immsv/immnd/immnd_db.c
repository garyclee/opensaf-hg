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
 * Author(s): Ericsson AB
 *
 */

/*****************************************************************************
  FILE NAME: immcn_evt.c

  DESCRIPTION: IMMND Routines to operate on client_info_db etc.
               (Routines to add/delete nodes into database, routines to 
                destroy the databases)

******************************************************************************/

#include "immnd.h"


/****************************************************************************
 * Name          : immnd_client_node_get
 *
 * Description   : Function to get the client node from client db Tree.
 *
 * Arguments     : IMMND_CB *cb, - IMMND Control Block
 *               : uns32 imm_client_hdl - Client Handle.
 *                 
 * Return Values : IMMND_IMM_CLIENT_NODE** imm_client_node
 *
 * Notes         : None.
 *****************************************************************************/
void immnd_client_node_get(IMMND_CB *cb, SaImmHandleT imm_client_hdl, 
    IMMND_IMM_CLIENT_NODE** imm_client_node)
{
    *imm_client_node = (IMMND_IMM_CLIENT_NODE *)
        ncs_patricia_tree_get(&cb->client_info_db, (uns8*)&imm_client_hdl);
    return;
}

/****************************************************************************
 * Name          : immnd_client_node_getnext
 *
 * Description   : Function to get next clinet node from client db Tree.
 *
 * Arguments     : IMMND_CB *cb, - IMMND Control Block
 *                 uns32 imm_client_hdl - Client handle
 *                 
 * Return Values : IMMND_IMM_CLIENT_NODE** imm_client_node 
 *
 * Notes         : None.
 *****************************************************************************/
void immnd_client_node_getnext(IMMND_CB *cb, SaImmHandleT imm_client_hdl, 
    IMMND_IMM_CLIENT_NODE** imm_client_node)
{
    if(imm_client_hdl)
        *imm_client_node = (IMMND_IMM_CLIENT_NODE *)
            ncs_patricia_tree_getnext(&cb->client_info_db, (uns8*)&imm_client_hdl);
    else
        *imm_client_node = (IMMND_IMM_CLIENT_NODE *)
            ncs_patricia_tree_getnext(&cb->client_info_db, (uns8*)NULL);
    return;
}

/****************************************************************************
 * Name          : immnd_client_node_add
 *
 * Description   : Function to Add the IMM Client node into client db Tree.
 *
 * Arguments     : IMMND_CB *cb, - IMMND Control Block
 *                 
 * Return Values : 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 immnd_client_node_add(IMMND_CB *cb, IMMND_IMM_CLIENT_NODE *cl_node)
{
    uns32 rc=NCSCC_RC_FAILURE;

    cl_node->patnode.key_info = (uns8 *)&cl_node->imm_app_hdl;

    rc = ncs_patricia_tree_add(&cb->client_info_db,
        (NCS_PATRICIA_NODE *)&cl_node->patnode);
    return rc;
}

/****************************************************************************
 * Name          : immnd_client_node_del
 *
 * Description   : Function to Delete the Client Node into Client Db Tree.
 *
 * Arguments     : IMMND_CB *cb, - IMMND Control Block.
 *               : IMMND_IMM_CLIENT_NODE* imm_client_node - IMM Client Node. 
 *                 
 * Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS
 *
 * Notes         : None.
 *****************************************************************************/
uns32 immnd_client_node_del(IMMND_CB *cb, 
    IMMND_IMM_CLIENT_NODE *imm_client_node)
{
    uns32 rc=NCSCC_RC_FAILURE;

    rc = ncs_patricia_tree_del(&cb->client_info_db,
        (NCS_PATRICIA_NODE *)&imm_client_node->patnode);
    return rc;
}


/****************************************************************************
 * Name          :  immnd_client_node_tree_init
 *
 * Description   :  Function to Initialise client tree
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 immnd_client_node_tree_init(IMMND_CB *cb)
{
    NCS_PATRICIA_PARAMS     param;
    memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));
    param.key_size = sizeof(SaCkptHandleT);
    if (ncs_patricia_tree_init(&cb->client_info_db,&param) != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_FAILURE;
    }

    return NCSCC_RC_SUCCESS;

}


/****************************************************************************
 * Name          : immnd_client_node_tree_cleanup
 * Description   : Function to cleanup client db info
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 * Return Values : NCSCC_RC_SUCCESS/Error.
 * Notes         : None.
 *****************************************************************************/
void immnd_client_node_tree_cleanup(IMMND_CB *cb)
{

    IMMND_IMM_CLIENT_NODE *cl_node;

    while((cl_node = (IMMND_IMM_CLIENT_NODE *) 
              ncs_patricia_tree_getnext(&cb->client_info_db,(uns8*)0)))
    {
        ncs_patricia_tree_del(&cb->client_info_db,
            (NCS_PATRICIA_NODE *)&cl_node->patnode);
        free(cl_node);
    }
 
    return;
}
/****************************************************************************
 * Name          : immnd_client_node_tree_destroy
 * Description   : Function to destroy client db tree
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 * Return Values : NCSCC_RC_SUCCESS/Error.
 * Notes         : None.
 *****************************************************************************/
void immnd_client_node_tree_destroy(IMMND_CB *cb)
{
    /* cleanup the client tree */
    immnd_client_node_tree_cleanup(cb);
   
    /* destroy the tree */
    ncs_patricia_tree_destroy(&cb->client_info_db);
   
    return;
}


/****************************************************************************
 * Name            : immnd_get_phy_slot_id
 *
 * Description     : To get the physical slot id from node id
 *
 ****************************************************************************/
NCS_PHY_SLOT_ID  immnd_get_phy_slot_id(MDS_DEST dest)
{
    NCS_PHY_SLOT_ID phy_slot;

    m_NCS_GET_PHYINFO_FROM_NODE_ID(m_NCS_NODE_ID_FROM_MDS_DEST(dest),NULL,
        &phy_slot,NULL);
    return phy_slot;
}

/* FEVS MESSAGE QUEUEING */
/*************************************************************************
 * Name            : immnd_enqueue_fevs_msg
 *
 * Description     : Place a fevs message in backlog queue
 *
 *************************************************************************/
void immnd_enqueue_fevs_msg(IMMND_CB *cb, 
    SaUint64T msgNo,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest,
    IMMSV_OCTET_STRING* msg, 
    SaUint64T* next_expected, //out
    SaUint32T* andHowManyMore)//out
{
    IMMND_FEVS_MSG_NODE* tmp = cb->fevs_msg_list;

    //assert(0);
    LOG_ER("MESSAGE OUT OF ORDER => EXITING");
    exit(1);
    //We dont trust this code as it has not been tested yet.
    //Hopefully we will get this assert in system test...?

    //sort in ascending msgNo order.
    while(tmp && tmp->next && (tmp->next->msgNo < msgNo)) {
        tmp = tmp->next;
    }

    if(tmp && ((tmp->msgNo == msgNo) ||
           (tmp->next && (tmp->next->msgNo == msgNo)))) {
        goto expectations; //A duplicate already in queue, ignore it.
    }

    //Not a duplicate => allocate a new node.

    IMMND_FEVS_MSG_NODE* node= calloc(1, sizeof(IMMND_FEVS_MSG_NODE));
    assert(node);
    node->msgNo = msgNo;
    node->clnt_hdl = clnt_hdl;
    node->reply_dest = reply_dest;
    node->msg.size = msg->size;
    node->msg.buf = msg->buf;
    msg->buf = NULL;  //Steal the message buffer instad of allocating again.
    msg->size = 0;
    node->next = NULL;

    if(!tmp) {//first insertion in list
        cb->fevs_msg_list = node; 
        goto expectations;
    }

    if(tmp->msgNo > msgNo) {//insert at top of list
        assert(tmp == cb->fevs_msg_list); //remove this after component test
        node->next = tmp;
        cb->fevs_msg_list = node;  
        goto expectations;
    }

    if(tmp->next == NULL) {//isert at end.
        tmp->next = node;
        goto expectations;
    }

    //insert in the midle
    assert(tmp->next->msgNo > msgNo);
    assert(tmp->msgNo < msgNo);
    node->next = tmp->next;
    tmp->next = node;

 expectations:
    *next_expected = cb->highestProcessed + 1;
    *andHowManyMore = (cb->fevs_msg_list->msgNo) - (*next_expected);
}

/***************************************************************************
 * Name            : immnd_dequeue_fevs_msg
 *
 * Description     : Removes a fevs_msg from backlog.
 *
 *************************************************************************/
IMMSV_OCTET_STRING* immnd_dequeue_fevs_msg(IMMSV_OCTET_STRING* msg,
    IMMND_CB *cb, 
    SaUint64T msgNo,
    SaImmHandleT* clnt_hdl,
    MDS_DEST* reply_dest)
{
    /* Note: This function is currently not used. 
       It is work in progress for dealing with instability of mds broadcast.
       SC failover should be tested extensively to determine if there is an
       impact on MDS broadcast, such that it manages to send the bcast message
       to some IMMNDs but not others. Nodes that miss a message will then need
       to buffer messages here, request the missing messages from IMMD, and once
       the hole is filled, retreive these messages. 
     */
    assert(msg);
    assert(msgNo == cb->highestProcessed);
    IMMND_FEVS_MSG_NODE* tmp = cb->fevs_msg_list;
    ++msgNo;
    if(tmp && (tmp->msgNo == msgNo)) {//message wanted at front of queue 
        //First free the old message in msg
        //TODO: ABT This free is probably already done by the dispatch!!
        free(msg->buf);
        //Then assign the buffer in the first queued element
        msg->buf = tmp->msg.buf;
        *clnt_hdl = tmp->clnt_hdl;
        *reply_dest = tmp->reply_dest;
        tmp->msg.buf=NULL; //precaution
        //remove the list lelement
        cb->fevs_msg_list = tmp->next;
        tmp->next = NULL; //precaution
        free(tmp);
    }
    //TODO: ABT I think I leak the last message buffer returned.
    //walk the code case to verify.

    return msg;//Reusing the IMMSV_OCTET_STRING object that was input.
}


