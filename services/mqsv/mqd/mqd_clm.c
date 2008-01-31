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
   
   mqd_saf_hlth_chk_cb................MQD health check callback 
******************************************************************************
*/

#if 0
#include "mqd.h"

extern MQDLIB_INFO gl_mqdinfo;
extern uns32 mqd_timer_expiry_evt_process(MQD_CB *pMqd, NODE_ID *nodeid);
           
/****************************************************************************
 PROCEDURE NAME : mqd_clm_cluster_track_callback

 DESCRIPTION    : This CLM callback function will be called whenever there is 
                  any change in c;ister membership or any attribute of a cluster
                  node changes
 
 ARGUMENTS      : notificationBuffer - A pointer to a buffer of cluster node structures
                  numberOfMembers    - The current number of members in the cluster membership
                  error              - This parameter indicates whether the Cluster Membership 
                                       Service was able to perform the operation
  RETURNS       : None 
*****************************************************************************/
void mqd_clm_cluster_track_callback(const SaClmClusterNotificationBufferT *notificationBuffer,
                                     SaUint32T numberOfMembers,
                                     SaAisErrorT error)
{
   uns32 counter = 0;
   MQD_CB  *pMqd = NULL;
   MQD_ND_DB_NODE *pNdNode=0;
   NCS_PHY_SLOT_ID phy_slot;
   if (error != SA_AIS_OK) {
      /* Log this error */
      return;
   }

   pMqd = ncshm_take_hdl(NCS_SERVICE_ID_MQD, gl_mqdinfo.inst_hdl); if (pMqd == NULL) {
            /* Log this error */
      return;
   }

   m_NCS_CONS_PRINTF("CLM TRACKCALLBACK INVOKED\n"); 
   /* Cycle through all the items in the notificationBuffer[] and call the timer expiry 
      routine that cleans iup the MQD QDB for all those nodes which have left the cluster */
   for(counter = 0; counter < notificationBuffer->numberOfItems; counter++) 
   {
      if (notificationBuffer->notification[counter].clusterChange == SA_CLM_NODE_LEFT) 
      {
         m_NCS_CONS_PRINTF(" DOWN PROCESSING ");
         if (pMqd->ha_state == SA_AMF_HA_ACTIVE)
         {
             pNdNode = (MQD_ND_DB_NODE *)ncs_patricia_tree_get( &pMqd->node_db, 
                                                       (uns8 *)&notificationBuffer->notification[counter].clusterNode.nodeId);
             if(pNdNode)
             {
                 /* Already timer started because of MDS down, clean it */
                 mqd_tmr_stop(&pNdNode->info.timer);
             }

             m_NCS_CONS_PRINTF(" I AM IN ACTIVE ");
             mqd_timer_expiry_evt_process(pMqd,&notificationBuffer->notification[counter].clusterNode.nodeId);
/* This node is getting freed at the above routine itself, noneed to free outside again */ 
#if 0
             if(pNdNode)
                mqd_red_db_node_del(pMqd, pNdNode);
#endif

             m_NCS_GET_PHYINFO_FROM_NODE_ID(notificationBuffer->notification[counter].clusterNode.nodeId, NULL,&phy_slot,NULL);
             m_NCS_CONS_PRINTF("CLM_CLBK ::: LEFT ::: node_status %d NODE DOWN", phy_slot);
         }
         else
         {

             m_NCS_CONS_PRINTF(" I AM IN STAND BY ");
             pNdNode = (MQD_ND_DB_NODE *)ncs_patricia_tree_get( &pMqd->node_db, 
                                                       (uns8 *)&notificationBuffer->notification[counter].clusterNode.nodeId);
             if(pNdNode==NULL)
             {
                 pNdNode = m_MMGR_ALLOC_MQD_ND_DB_NODE;
                 if(pNdNode ==NULL)
                 {
                     m_LOG_MQSV_D(MQD_MEMORY_ALLOC_FAIL,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,NCSCC_RC_OUT_OF_MEM,__FILE__,__LINE__);
                     continue;
                  }
                  m_NCS_OS_MEMSET(pNdNode, 0, sizeof(MQD_ND_DB_NODE));
                  pNdNode->info.is_clm_down_processed = FALSE;
                  pNdNode->info.nodeid = notificationBuffer->notification[counter].clusterNode.nodeId;

                  mqd_red_db_node_add(pMqd, pNdNode);

                  m_NCS_CONS_PRINTF("CLM DOWN PROCESS FLAG SET TO FALSE\n"); 
             }
          }
      }
      else
         m_NCS_CONS_PRINTF(" IGNORED : NOT A DOWN \n ");
   }

done:
   ncshm_give_hdl(gl_mqdinfo.inst_hdl);

   return;
}i
#endif
