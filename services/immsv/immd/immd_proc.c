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
  FILE NAME: immd_proc.c

  DESCRIPTION: IMMD Event handling routines
******************************************************************************/

#include "immd.h"

void immd_proc_immd_reset(IMMD_CB *cb, NCS_BOOL active)
{
    TRACE_ENTER();
    /* Reset relevant parts of of IMMD CB so we get a cluster wide restart
       of IMMNDs. 
    */

    if(active) {
        LOG_ER("Active IMMD has to restart the IMMSv. "
            "All IMMNDs will restart");
    } else {
        LOG_ER("Standby IMMD recieved reset message. "
            "All IMMNDs will restart.");
    }
    cb->mRulingEpoch = 0;
    cb->immnd_coord = 0;
    /*immd_immnd_info_tree_cleanup(cb);*/
    /*Let the immnd tree keep track of what is hapening. IMMNDs that have
     just restarted may not need to restart and re-attach again.*/
    TRACE_LEAVE();
}

int immd_proc_elect_coord(IMMD_CB *cb)
{
    IMMSV_EVT       send_evt;
    IMMD_MBCSV_MSG  mbcp_msg;
    MDS_DEST key;
    IMMD_IMMND_INFO_NODE *immnd_info_node=NULL;

    TRACE_ENTER();
    memset(&key, 0, sizeof(MDS_DEST));
    unsigned int maxEpoch=0;

    /*First verify that we dont have any coord.*/
    immd_immnd_info_node_getnext(&cb->immnd_tree, &key, 
                                 &immnd_info_node);
    while (immnd_info_node)
    {
        key = immnd_info_node->immnd_dest;
        if (maxEpoch < immnd_info_node->epoch)
        {
            maxEpoch = immnd_info_node->epoch;
        }
        if (immnd_info_node->isCoord)
        {
            if (maxEpoch > immnd_info_node->epoch)
            {
                /* TODO: this check is not even reliable since maxEpoch is
                 not obtained first. */
                LOG_ER("Epoch(%u) for current coord is less than "
                       "the epoch of some other node (%u)",
                       immnd_info_node->epoch, maxEpoch);
                immd_proc_immd_reset(cb, TRUE);
                TRACE_LEAVE();
                return -1; //avoid assert?
            }
            break; //out of while
        }
        immd_immnd_info_node_getnext(&cb->immnd_tree, &key, 
                                     &immnd_info_node);
    }

    if (immnd_info_node)
    {
        TRACE_5("Elect coord: Coordinator node already exists: %x",
                immnd_info_node->immnd_key);
        assert(cb->mRulingEpoch == immnd_info_node->epoch);
    }
    else
    {
        /* Try to elect a new coord. */
        memset(&key, 0, sizeof(MDS_DEST));
        immd_immnd_info_node_getnext(&cb->immnd_tree, &key, 
                                     &immnd_info_node);
        while (immnd_info_node)
        {
            key = immnd_info_node->immnd_dest;
            if ((immnd_info_node->isOnController) &&
                (immnd_info_node->epoch == cb->mRulingEpoch))
            {
                /*We found a new candidate for cordinator*/
                immnd_info_node->isCoord = TRUE;
                break;
            }

            immd_immnd_info_node_getnext(&cb->immnd_tree, &key,
                                         &immnd_info_node);
        }

        if (!immnd_info_node)
        {
            IMMSV_EVT send_evt;
            uns32 proc_rc;
            LOG_ER("Failed to find candidate for new IMMND coordinator");
            immd_proc_immd_reset(cb, TRUE);

            /* Make standby aware of IMMND reset order. */
            memset(&mbcp_msg, 0, sizeof(IMMD_MBCSV_MSG));
            mbcp_msg.type = IMMD_A2S_MSG_RESET;

            /*Checkpoint the dissapointing reset message to standby
              director. Syncronous call=>waits for ack. We do not check
              the result because we can not do anything about an error in
              notifying IMMD standby here. The immd_mbcsv_sync_update 
              will also log the error. 
            */
            immd_mbcsv_sync_update(cb,&mbcp_msg);

            /*Restart any IMMNDs that are not in epoch 0. */
            memset(&send_evt, 0, sizeof(IMMSV_EVT));
            send_evt.type = IMMSV_EVT_TYPE_IMMND;
            send_evt.info.immnd.type = IMMND_EVT_D2ND_RESET;
            proc_rc = immd_mds_bcast_send(cb, &send_evt, NCSMDS_SVC_ID_IMMND);
            if(proc_rc != NCSCC_RC_SUCCESS)
            {
                LOG_ER("Failed to broadcast RESET message to IMMNDs.");
                exit(1);
            }
            
            TRACE_LEAVE();
            return(-1);
        }

        TRACE_5("New coord elected, resides at %x", 
            immnd_info_node->immnd_key);
        cb->immnd_coord = immnd_info_node->immnd_key;
        if (!cb->is_rem_immnd_up)
        {
            if (cb->immd_remote_id && 
                m_IMMND_IS_ON_SCXB(cb->immd_remote_id,
                immd_get_slot_and_subslot_id_from_mds_dest(immnd_info_node->immnd_dest)))
            {
                cb->is_rem_immnd_up = TRUE; /*ABT BUGFIX 080905*/
                cb->rem_immnd_dest = immnd_info_node->immnd_dest;
                TRACE_5("Corrected: IMMND process is on STANDBY Controller");
            }
        }

        memset(&send_evt, 0, sizeof(IMMSV_EVT));
        memset(&mbcp_msg, 0, sizeof(IMMD_MBCSV_MSG));
        send_evt.type = IMMSV_EVT_TYPE_IMMND;
        /*This event type should be given a more appropriate name. */
        send_evt.info.immnd.type = IMMND_EVT_D2ND_INTRO_RSP;
        send_evt.info.immnd.info.ctrl.nodeId = 
            immnd_info_node->immnd_key;
        send_evt.info.immnd.info.ctrl.rulingEpoch = cb->mRulingEpoch;
        send_evt.info.immnd.info.ctrl.canBeCoord = 
            immnd_info_node->isOnController;
        send_evt.info.immnd.info.ctrl.ndExecPid = 
            immnd_info_node->immnd_execPid;
        send_evt.info.immnd.info.ctrl.isCoord = TRUE;

        mbcp_msg.type = IMMD_A2S_MSG_INTRO_RSP;
        mbcp_msg.info.ctrl = send_evt.info.immnd.info.ctrl;
        /*Checkpoint the new coordinator message to standby director. 
        Syncronous call=>wait for ack*/
        if (immd_mbcsv_sync_update(cb,&mbcp_msg) != NCSCC_RC_SUCCESS)
        {
            LOG_ER("Mbcp message designating new IMMD coord failed => "
                   "standby IMMD could not be notified. Failover to STBY.");
            exit(1);
        }

        /*Notify the remining controller based IMMND that it is the new 
          IMMND coordinator. */
        if (immd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMND, 
                              immnd_info_node->immnd_dest, &send_evt) !=
            NCSCC_RC_SUCCESS)
        {
            LOG_ER("Failed to send MDS message designating new IMMND coord.");
            exit(1);
        }
    }

    immd_cb_dump();
    TRACE_LEAVE();
    return 0;
}

/****************************************************************************
 * Name          : immd_process_immnd_down
 *
 * Description   : This routine will process the IMMND down event.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 immd_process_immnd_down(IMMD_CB *cb, IMMD_IMMND_INFO_NODE *immnd_info,
    NCS_BOOL active)
{
    IMMSV_EVT send_evt;
    NCS_UBAID uba;
    char* tmpData = NULL;
    int res = 0;
    NCS_BOOL possible_fo = FALSE;
    TRACE_ENTER();

    TRACE_5("immd_process_immnd_down pid:%u on-active:%u "
            "cb->immnd_coord:%x", 
            immnd_info->immnd_execPid, active,
            cb->immnd_coord);
    if (active)
    {
        uba.start=NULL;

        if (immnd_info->isCoord)
        {
            LOG_WA("IMMND coordinator at %x apparently crashed => "
                "electing new coord", immnd_info->immnd_key);
            immnd_info->isCoord = 0;
            immnd_info->isOnController = 0;
            cb->immnd_coord = 0;
            res = immd_proc_elect_coord(cb);
        }
    } else {
          /* Check if it was the IMMND on the active controller that went down.*/
        if(immd_get_slot_and_subslot_id_from_node_id(immnd_info->immnd_key) ==
        cb->immd_remote_id)
        {
            LOG_WA("IMMND DOWN on active controller %x "
                   "detected at standby immd!! %x. "
                   "Possible failover",
                    immd_get_slot_and_subslot_id_from_node_id(immnd_info->immnd_key), 
                    cb->immd_self_id);
            possible_fo = TRUE;
        }
    }

    if(active || possible_fo) 
    {
        /* 
        ** HAFE - Let IMMND subscribe for IMMND up/down events instead?
        ** ABT - Not for now. IMMND up/down are only subscribed ny IMMD.
        ** This is the cleanest solution with respect to FEVS and works fine
        ** for all cases except IMMND down at the active controller. 
        ** That case has to be handled in a special way. Currently we do it
        ** by having both the standby and active IMMD broadcast the same 
        ** DISCARD_NODE message. IMMNDs have to cope with the possibly 
        ** redundant message, which they do in fevs_receive discarding
        ** any duplicate (same sequence no).
        */
        if (res == 0)
        {
            if(!active) 
            {
                IMMSV_FEVS* old_msg=NULL;
                uns16 back_count = 2;
                TRACE_5("Re-broadcast the last two fevs messages received over mbcpsv");
                do {
                    old_msg = immd_db_get_fevs(cb, back_count);
                    if(old_msg) {
                        TRACE_5("Resend message no %llu", old_msg->sender_count);
                        memset(&send_evt, 0, sizeof(IMMSV_EVT));
                        send_evt.type = IMMSV_EVT_TYPE_IMMD;
                        send_evt.info.immd.type = 0;
                        send_evt.info.immd.info.fevsReq.sender_count = old_msg->sender_count;
                        send_evt.info.immd.info.fevsReq.reply_dest = old_msg->reply_dest;
                        send_evt.info.immd.info.fevsReq.client_hdl = old_msg->client_hdl;
                        send_evt.info.immd.info.fevsReq.msg.size = old_msg->msg.size;
                        send_evt.info.immd.info.fevsReq.msg.buf = old_msg->msg.buf;

                        if (immd_evt_proc_fevs_req(cb, &(send_evt.info.immd), NULL, FALSE) 
                            != NCSCC_RC_SUCCESS)
                        {
                            LOG_ER("Failed to re-send FEVS message %llu", old_msg->sender_count);
                        }
                    }

                    --back_count;
                } while (back_count != 0);
            }
            
            TRACE_5("Notify all remaining IMMNDs of the departed IMMND");
            memset(&send_evt, 0, sizeof(IMMSV_EVT));
            send_evt.type = IMMSV_EVT_TYPE_IMMND;
            send_evt.info.immnd.type = IMMND_EVT_D2ND_DISCARD_NODE;
            send_evt.info.immnd.info.ctrl.nodeId = immnd_info->immnd_key;
            send_evt.info.immnd.info.ctrl.ndExecPid = immnd_info->immnd_execPid;

            assert(ncs_enc_init_space(&uba) == NCSCC_RC_SUCCESS);
            assert(immsv_evt_enc(&send_evt, &uba) == NCSCC_RC_SUCCESS);

            int32 size = uba.ttl;
            tmpData = malloc(size);
            assert(tmpData);
            char* data = m_MMGR_DATA_AT_START(uba.start, size, tmpData);

            memset(&send_evt, 0, sizeof(IMMSV_EVT)); /*No ponters=>no leak*/
            send_evt.type = IMMSV_EVT_TYPE_IMMD;
            send_evt.info.immd.type = 0;
            send_evt.info.immd.info.fevsReq.msg.size = size;
            send_evt.info.immd.info.fevsReq.msg.buf = data;

            if (immd_evt_proc_fevs_req(cb, &(send_evt.info.immd), NULL, FALSE) 
                != NCSCC_RC_SUCCESS)
            {
                LOG_ER("Failed to send discard node message over FEVS");
            }

            free(tmpData);
        }
    }

    /*We remove the node for the lost IMMND on both active and standby. */
    immd_immnd_info_node_delete(cb, immnd_info);
    immd_cb_dump();
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : immd_cb_dump
 *
 * Description   : This routine is used for debugging.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
void immd_cb_dump(void)
{
    IMMD_CB *cb = immd_cb;

    TRACE_5("*****************Printing IMMD CB Dump****************");
    TRACE_5("  MDS Handle:             %x", (uns32)cb->mds_handle);
    TRACE_5("  IMMD State:  %d",cb->ha_state); 
    TRACE_5("  SYNC UPDATE COUNT %u",cb->immd_sync_cnt);

    if (cb->is_immnd_tree_up)
    {
        TRACE_5("+++++++++++++IMMND Tree is UP++++++++++++++++++++");
        TRACE_5("Number of nodes in IMMND Tree:  %u", 
                cb->immnd_tree.n_nodes);

        /* Print the IMMND Details */
        if (cb->immnd_tree.n_nodes >0)
        {
            MDS_DEST key;
            IMMD_IMMND_INFO_NODE *immnd_info_node;
            memset(&key, 0, sizeof(MDS_DEST));

            immd_immnd_info_node_getnext(&cb->immnd_tree, &key, 
                &immnd_info_node);
            while (immnd_info_node)
            {
                key = immnd_info_node->immnd_dest;
                TRACE_5("   -------------------------------------------");
                TRACE_5("   MDS Node ID:  = %x",
                        m_NCS_NODE_ID_FROM_MDS_DEST(immnd_info_node->
                                                    immnd_dest));
                TRACE_5("   Pid:%u Epoch:%u syncR:%u onCtrlr:%u isCoord:%u",
                        immnd_info_node->immnd_execPid,
                        immnd_info_node->epoch,
                        immnd_info_node->syncRequested,
                        immnd_info_node->isOnController,
                        immnd_info_node->isCoord);

                immd_immnd_info_node_getnext(&cb->immnd_tree, &key,
                                             &immnd_info_node);
            }
            TRACE_5(" End of IMMND Info");
        }
        TRACE_5(" End of IMMND info nodes");
    }

    TRACE_5("*****************End of IMMD CB Dump******************");
}

