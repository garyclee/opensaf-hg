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
 */

/*****************************************************************************
..............................................................................
  MODULE NAME: SNMPTM_CB.C

..............................................................................

  DESCRIPTION:   This file contains routines to manage snmptm_cb datastructure.

    snmptm_cb_create ................ Create SNMPTM Control Block
    snmptm_cb_destroy ............... Frees up the SNMPTM Control Block

******************************************************************************
*/
#include "snmptm.h"

#if(NCS_SNMPTM == 1)

/****************************************************************************** 
 Name      :  snmptm_eda_initialize 
 
 Purpose   :  To intialize the session with EDA Agent
                 - To register the callbacks
                 
 Arguments :  SNMPTM_CB *snmptm_cb - SNMP TEST-MIB's control block
 
 Returns   :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE /
              NCSCC_RC_EDA_INTF_INIT_FAILURE / NCSCC_RC_EDA_CHANNEL_OPEN_FAILURE /
              NCSCC_RC_SUBSCRIBE_FAILURE / NCSCC_RC_EDA_SEL_OBJ_FAILURE
 Notes     :
******************************************************************************/
uns32 snmptm_eda_initialize(SNMPTM_CB  *snmptm_cb)
{
   uns32           channelFlags = 0;
   SaAisErrorT        status = SA_AIS_OK; 
   SaTimeT         timeout = 200000000; 
   SaEvtCallbacksT callbacks = {NULL, NULL};
   SaVersionT      saf_version = {'B', 1, 1};


   if (snmptm_cb == NULL)
   {
      m_NCS_CONS_PRINTF("\nSNMPTM ERROR: Not able to initialize EDA\n"); 
      return NCSCC_RC_FAILURE;
   }

   /* 1. Create the evt handle structure, set callbacks, etc */ 
   status = saEvtInitialize(&snmptm_cb->evtHandle,  
                            &callbacks, 
                            &saf_version); 
   if (status != SA_AIS_OK)
   {
      return NCSCC_RC_FAILURE; 
   } 

   /* 2. Create/Open event channel with remote server (EDS)  */
   /* To use the SA Event Service, a process must create an event channel.  
    * The process can then open the event channel to publish events and 
    * to subscribe to receive events.  
    * IMP: Publishers can also be subscribers on the same Channel 
    */
   channelFlags = SA_EVT_CHANNEL_CREATE | SA_EVT_CHANNEL_PUBLISHER;

   status = saEvtChannelOpen(snmptm_cb->evtHandle, 
                             &snmptm_cb->evtChannelName, 
                             channelFlags,   
                             timeout,       
                             &snmptm_cb->evtChannelHandle); 
   if (status != SA_AIS_OK)
   {
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 Name      :  snmptm_eda_finalize

 Purpose   :  To Finalize the session with the EDA 
 
 Arguments :  SNMPTM_CB *snmptm_cb - SNMP TEST-MIB control block 

 Returns   :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE 

 Notes     : 
*****************************************************************************/
uns32  snmptm_eda_finalize(SNMPTM_CB *snmptm_cb)
{
   SaAisErrorT  status = SA_AIS_OK;


   /* validate the inputs */
   if (snmptm_cb == NULL)
   {
      m_NCS_CONS_PRINTF("\nSNMPTM ERROR: Not able to Finalize EDA\n"); 
      return NCSCC_RC_FAILURE;
   }

   /* 1. terminate the communication channels 
    *
    * Channel Close causes to delete the event channel from the 
    * cluster name space
    *    -- courtesy SAF documentation 
    */
   status = saEvtChannelClose(snmptm_cb->evtChannelHandle);
   if (status != SA_AIS_OK)
   {
      return NCSCC_RC_FAILURE;
   }

   /* 2. Close all the associations with EDA, close the deal */
   status = saEvtFinalize(snmptm_cb->evtHandle);
   if (status != SA_AIS_OK)
   {
      return NCSCC_RC_FAILURE;
   }

   return status;
}


/*****************************************************************************
  Name          :  snmptm_cb_create

  Description   :  This function creates a SNMPTM Control Block

  Arguments     :  None
        
  Return Values :  non-NULL    -  SNMPTM Control Block instance 
                   NULL        -  Failure.  
    
  Notes         :  This function allocate the memory for snmptm_cb and 
                   initialises all the members of SNMPTM_CB structure to the
                   defaults ,if any. It initalises all the patricia trees 
                   in the SNMPTM_CB. 
*****************************************************************************/
SNMPTM_CB* snmptm_cb_create(void)
{
   SNMPTM_CB           *snmptm = SNMPTM_CB_NULL; 
   NCS_PATRICIA_PARAMS list_params;       

   /* Allocate SNMPTM_CB structure */   
   if ((snmptm = (SNMPTM_CB *)m_MMGR_ALLOC_SNMPTM_CB) == SNMPTM_CB_NULL)
   {
      m_NCS_CONS_PRINTF("\n MALLOC failed for SNMPTM Control Block\n");
      return SNMPTM_CB_NULL;   
   }

   memset(snmptm, 0, sizeof(SNMPTM_CB));

   /* Update the key information */
   list_params.key_size  = sizeof(SNMPTM_TBL_KEY);
   list_params.info_size = 0;  

   /* Initialise TBLONE tree. Key: ip_addr */
   if ((ncs_patricia_tree_init(&snmptm->tblone_tree, &list_params))
        != NCSCC_RC_SUCCESS)
   {
      /* Failed to initialise tree.Free the memory allocated till now */
      m_MMGR_FREE_SNMPTM_CB(snmptm);
      return SNMPTM_CB_NULL;
   }
  
   /* Initialise TBLTHREE tree. Key: ip_addr */
   if ((ncs_patricia_tree_init(&snmptm->tblthree_tree, &list_params))
        != NCSCC_RC_SUCCESS)
   {
      /* Failed to initialise tree.Free the memory allocated till now */
      ncs_patricia_tree_destroy(&snmptm->tblone_tree);
      m_MMGR_FREE_SNMPTM_CB(snmptm);
      return SNMPTM_CB_NULL;
   }
  
   /* Initialise TBLFIVE tree. Key: ip_addr */
   if ((ncs_patricia_tree_init(&snmptm->tblfive_tree, &list_params))
        != NCSCC_RC_SUCCESS)
   {
      /* Failed to initialise tree.Free the memory allocated till now */
      ncs_patricia_tree_destroy(&snmptm->tblone_tree);
      ncs_patricia_tree_destroy(&snmptm->tblthree_tree);
      m_MMGR_FREE_SNMPTM_CB(snmptm);
      return SNMPTM_CB_NULL;
   }
   
   /* Initialise TSIX tree. Key: count */
   /* Update the key information */
   list_params.key_size  = sizeof(SNMPTM_TBLSIX_KEY);
   list_params.info_size = 0;  
   if ((ncs_patricia_tree_init(&snmptm->tblsix_tree, &list_params))
        != NCSCC_RC_SUCCESS)
   {
      /* Failed to initialise tree.Free the memory allocated till now */
      ncs_patricia_tree_destroy(&snmptm->tblfive_tree);
      ncs_patricia_tree_destroy(&snmptm->tblone_tree);
      ncs_patricia_tree_destroy(&snmptm->tblthree_tree);
      m_MMGR_FREE_SNMPTM_CB(snmptm);
      return SNMPTM_CB_NULL;
   }

   /* Initialise TBLEIGHT tree. Key: IfIndex, tbleight_unsigned32, tblfour_unsigned32 */
   /* Update the key information */
   list_params.key_size  = sizeof(SNMPTM_TBLEIGHT_KEY);
   list_params.info_size = 0;  
   if ((ncs_patricia_tree_init(&snmptm->tbleight_tree, &list_params))
        != NCSCC_RC_SUCCESS)
   {
      /* Failed to initialise tree.Free the memory allocated till now */
      ncs_patricia_tree_destroy(&snmptm->tblsix_tree);
      ncs_patricia_tree_destroy(&snmptm->tblfive_tree);
      ncs_patricia_tree_destroy(&snmptm->tblone_tree);
      ncs_patricia_tree_destroy(&snmptm->tblthree_tree);
      m_MMGR_FREE_SNMPTM_CB(snmptm);
      return SNMPTM_CB_NULL;
   }

   /* Initialise TBLTEN tree. Key: tblten_unsigned32, tblten_int */
   /* Update the key information */
   list_params.key_size  = sizeof(SNMPTM_TBLTEN_KEY);
   list_params.info_size = 0;  
   if ((ncs_patricia_tree_init(&snmptm->tblten_tree, &list_params))
        != NCSCC_RC_SUCCESS)
   {
      /* Failed to initialise tree.Free the memory allocated till now */
      ncs_patricia_tree_destroy(&snmptm->tbleight_tree);
      ncs_patricia_tree_destroy(&snmptm->tblsix_tree);
      ncs_patricia_tree_destroy(&snmptm->tblfive_tree);
      ncs_patricia_tree_destroy(&snmptm->tblone_tree);
      ncs_patricia_tree_destroy(&snmptm->tblthree_tree);
      m_MMGR_FREE_SNMPTM_CB(snmptm);
      return SNMPTM_CB_NULL;
   }

   /* Add entries into TBLSIX */
   {
      SNMPTM_TBLSIX_KEY  tblsix_key;
      SNMPTM_TBLSIX *tblsix = NULL;

      memset(&tblsix_key, '\0', sizeof(tblsix_key));
      tblsix_key.count = 1;
      tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
      tblsix->tblsix_data = 101;
      strcpy(&tblsix->tblsix_name, "TBSIXROW1");

      memset(&tblsix_key, '\0', sizeof(tblsix_key));
      tblsix_key.count = 2;
      tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
      tblsix->tblsix_data = 102;
      strcpy(&tblsix->tblsix_name, "TBSIXROW2");

      memset(&tblsix_key, '\0', sizeof(tblsix_key));
      tblsix_key.count = 3;
      tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
      tblsix->tblsix_data = 103;
      strcpy(&tblsix->tblsix_name, "TBSIXROW3");

      memset(&tblsix_key, '\0', sizeof(tblsix_key));
      tblsix_key.count = 4;
      tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
      tblsix->tblsix_data = 104;
      strcpy(&tblsix->tblsix_name, "TBSIXROW4");

      memset(&tblsix_key, '\0', sizeof(tblsix_key));
      tblsix_key.count = 5;
      tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
      tblsix->tblsix_data = 105;
      strcpy(&tblsix->tblsix_name, "TBSIXROW5");

      memset(&tblsix_key, '\0', sizeof(tblsix_key));
      tblsix_key.count = 6;
      tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
      tblsix->tblsix_data = 106;
      strcpy(&tblsix->tblsix_name, "TBSIXROW6");

      memset(&tblsix_key, '\0', sizeof(tblsix_key));
      tblsix_key.count = 7;
      tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
      tblsix->tblsix_data = 107;
      strcpy(&tblsix->tblsix_name, "TBSIXROW7");

      memset(&tblsix_key, '\0', sizeof(tblsix_key));
      tblsix_key.count = 8;
      tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
      tblsix->tblsix_data = 108;
      strcpy(&tblsix->tblsix_name, "TBSIXROW8");

      memset(&tblsix_key, '\0', sizeof(tblsix_key));
      tblsix_key.count = 9;
      tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
      tblsix->tblsix_data = 109;
      strcpy(&tblsix->tblsix_name, "TBSIXROW9");

      memset(&tblsix_key, '\0', sizeof(tblsix_key));
      tblsix_key.count = 10;
      tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
      tblsix->tblsix_data = 110;
      strcpy(&tblsix->tblsix_name, "TBSIXROW10");
   }

   /* Add entries into TBLTEN*/
   {
      SNMPTM_TBLTEN_KEY  tblten_key;
      SNMPTM_TBLTEN *tblten = NULL;

      memset(&tblten_key, '\0', sizeof(tblten_key));
      tblten_key.tblten_unsigned32 = 1;
      tblten_key.tblten_int = 1;
      tblten = snmptm_create_tblten_entry(snmptm, &tblten_key);

      memset(&tblten_key, '\0', sizeof(tblten_key));
      tblten_key.tblten_unsigned32 = 2;
      tblten_key.tblten_int = 2;
      tblten = snmptm_create_tblten_entry(snmptm, &tblten_key);
   }

   /* Init CB Lock */
   m_NCS_LOCK_INIT(&snmptm->snmptm_cb_lock);
   
   return snmptm;
}



/*****************************************************************************
  Name          :  snmptm_cb_destroy

  Description   :  This function destroys a SNMPTM Control Block.

  Arguments     :  snmptm  - Pointer to SNMPTM CB structure

  Return Values :   

  Notes         :
*****************************************************************************/
void snmptm_cb_destroy(SNMPTM_CB *snmptm)
{
   /* Destroy CB lock */
   m_NCS_LOCK_DESTROY(&snmptm->snmptm_cb_lock);

   /* destroy the hdl mgr for SNMPTM CB */
   ncshm_destroy_hdl(NCS_SERVICE_ID_SNMPTM, snmptm->hmcb_hdl);

   /* Free the memory allotted for SNMPTM CB */
   m_MMGR_FREE_SNMPTM_CB(snmptm);   

   return;
}

#else   /* NCS_SNMPTM */
extern int dummy;
#endif  /* NCS_SNMPTM */


