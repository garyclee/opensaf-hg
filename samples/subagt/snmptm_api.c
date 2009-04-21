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
  MODULE NAME: SNMPTM_API.C

..............................................................................

  DESCRIPTION: Contains definitions of SNMPTM LM Request API

    ncssnmptm_lm_request .................. Process SNMPTM LM Requests
    snmptm_create ......................... Internal SNMPTM create routine
    snmptm_destroy ........................ Internal SNMPTM destroy routine

******************************************************************************
*/
#include "snmptm.h"

#if(NCS_SNMPTM == 1)

uns32  g_snmptm_hdl = 0;
extern NCS_BOOL gl_snmptm_eda_init;

static uns32 snmptm_create(NCSSNMPTM_LM_SNMPTM_CREATE_REQ_INFO *);
static uns32 snmptm_destroy(uns32);


/*****************************************************************************
  Name          :  ncssnmptm_lm_request

  Description   :  This function process create & destroy LM (Layer Management)
                   requests. This is the entry point for 
                     -   creation and deletion of SNMPTM

  Arguments     :  request: Pointer to structure containing info on the
                            request

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 ncssnmptm_lm_request(NCSSNMPTM_LM_REQUEST_INFO *request)
{
   /* SNMPTM instance is already existing, return if SNMPTM create request
      arrives */
   if((request->i_req == NCSSNMPTM_LM_REQ_SNMPTM_CREATE) &&
      (g_snmptm_hdl != 0))
   {   
      /* Wrong call, snmptm already exists ! */
      printf("\nSNMPTM Control Block already created\n");
      return NCSCC_RC_FAILURE;
   }
   
   /* SNMPTM instance doesn't exists, return if SNMPTM delete request
      arrives */
   if((request->i_req != NCSSNMPTM_LM_REQ_SNMPTM_CREATE) &&
      (g_snmptm_hdl == 0))
   {
      /* Wrong call, snmptm does not exists ! */
      printf("\nSNMPTM Control Block doesn't exists for deletion\n");
      return NCSCC_RC_FAILURE;
   }

   /* Process the request */ 
   switch(request->i_req)
   {   
   case NCSSNMPTM_LM_REQ_SNMPTM_CREATE: /* Create SNMPTM instance */
        if(snmptm_create(&request->io_create_snmptm) == NCSCC_RC_FAILURE)
        {
           printf("\n NOT ABLE TO CREATE SNMPTM INSTANCE\n");
           return NCSCC_RC_FAILURE;
        }

        printf("\n SNMPTM CREATED SUCCESSFULLY\n");
        break;
      
   case NCSSNMPTM_LM_REQ_SNMPTM_DESTROY: /* Destroy SNMPTM instance */
        if(snmptm_destroy(g_snmptm_hdl) == NCSCC_RC_FAILURE)
        {
           printf("\n NOT ABLE TO DESTROY SNMPTM INSTANCE\n");
           return NCSCC_RC_FAILURE;
        }
        break;       
      
   default:
       break;
      
    }

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************
  Name          :  snmptm_create

  Description   :  This function creates an SNMPTM Instance. Creates SNMPTM
                   Control Block, register SNMPTM with handle manager, init
                   EDA, register SNMPTM with OpenSAF MibLib as well as with OAC 
                   (MAB). SAF EDA is used in sending the TRAP to OpenSAF subAgent.

  Arguments     :  create_info - Info. containing the bindery elements

  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         :
***************************************************************************/
uns32 snmptm_create(NCSSNMPTM_LM_SNMPTM_CREATE_REQ_INFO *create_info)
{
   SNMPTM_CB *snmptm = SNMPTM_CB_NULL;   
   uns32     rc  = NCSCC_RC_SUCCESS;

   /* Create the SNMPTM Control Block */   
   if (SNMPTM_CB_NULL == (snmptm = snmptm_cb_create()))
   {
      printf("\n NOT ABLE CREATE SNMPTM CB");
      return NCSCC_RC_FAILURE;
   }
   
   /* Acquire CB Lock */
   m_SNMPTM_LOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_WRITE); 
   strcpy((char*)&snmptm->pcn_name, (char*)&create_info->i_pcn);
   
   /* Copy given information into SNMPTM control block */
   snmptm->oac_hdl   = create_info->i_oac_hdl;
   snmptm->mds_vdest = create_info->i_mds_vdest;
   snmptm->vcard     = create_info->i_vdest;
   snmptm->node_id   = create_info->i_node_id;
   snmptm->hmpool_id = create_info->i_hmpool_id;
   
   /* Create Handle Manager and get the handle for the CB */
   snmptm->hmcb_hdl = g_snmptm_hdl = ncshm_create_hdl(snmptm->hmpool_id,
                                                  NCS_SERVICE_ID_SNMPTM,
                                                  (NCSCONTEXT)snmptm);
   
   /* EDU_HDL init */
   m_NCS_EDU_HDL_INIT(&snmptm->edu_hdl);
               
   strcpy(snmptm->publisherName.value, SNMPTM_EDA_EVT_PUBLISHER_NAME);
   snmptm->publisherName.length = strlen(snmptm->publisherName.value); 

   /* 1. Initialize the filter string */
   strcpy(snmptm->evtPatternStr, SNMPTM_EDA_EVT_FILTER_PATTERN);
                  
   /* 2. Initialize the pattern structure */
   snmptm->evtFilter.filterType = SA_EVT_EXACT_FILTER;
                          
   snmptm->evtFilter.filter.pattern = snmptm->evtPatternStr;
   snmptm->evtFilter.filter.patternSize = SNMPTM_EDA_EVT_FILTER_PATTERN_LEN;
                                  
   /* 3. initialize the pattern Array */
   snmptm->evtFilters.filtersNumber = 1;
   snmptm->evtFilters.filters = &snmptm->evtFilter;
                                              
   /* update the Channel Name */
   strcpy(snmptm->evtChannelName.value, SNMPTM_EDA_EVT_CHANNEL_NAME);
   snmptm->evtChannelName.length = strlen(snmptm->evtChannelName.value);

   /* Initialize EDA */
   if(gl_snmptm_eda_init)
   {
      rc = snmptm_eda_initialize(snmptm);
      if (rc != NCSCC_RC_SUCCESS)
      {
         m_SNMPTM_UNLOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_WRITE);
      
         snmptm_cb_destroy(snmptm);
         return NCSCC_RC_FAILURE;
      }
      /* Initialize and register with OpenSAF MibLib */
   }
   rc = snmptm_miblib_reg();
     
   /* Register SNMPTM with OAC (MAB). */
   if (rc == NCSCC_RC_SUCCESS)
   {
      rc = snmptm_reg_with_oac(snmptm);
   }

   /* If not success, unlock CB and destroy SNMPTM instance */
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SNMPTM_UNLOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_WRITE);
      
      snmptm_cb_destroy(snmptm);
      return NCSCC_RC_FAILURE;
   }   
 
   m_SNMPTM_UNLOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_WRITE);
   
   return NCSCC_RC_SUCCESS;
}


/***************************************************************************
  Name          :  snmptm_destroy

  Description   :  This function destroys an SNMPTM Instance.

  Arguments     :  cb_hdl  -  SNMPTM CB handle

  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         :
***************************************************************************/
uns32 snmptm_destroy(uns32 cb_hdl)
{
   SNMPTM_CB     *snmptm = NULL;
   SaEvtHandleT  nullEvtHandle;
   MDS_DEST   snmptm_vdest;


   /* Get the CB from the handle manager */
   if ((snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             cb_hdl)) == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   m_SNMPTM_LOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_WRITE);
   
   /* Try an ordered shutdown */    
   /* Delete all the created trees of SNMPTM */
   snmptm_delete_tblone(snmptm);    
   snmptm_delete_tblthree(snmptm);    
   snmptm_delete_tblfive(snmptm);    
   snmptm_delete_tblsix(snmptm);    
   
   /* Unregister with MAB */
   snmptm_unreg_with_oac(snmptm);
   
   /* EDU_HDL reset */
   m_NCS_EDU_HDL_FLUSH(&snmptm->edu_hdl);

   memset(&nullEvtHandle, 0 , sizeof(SaEvtHandleT));
   /* Free the EVT related data */
   if (memcmp(&snmptm->evtHandle, &nullEvtHandle, sizeof(SaEvtHandleT))!= 0)
   {
      snmptm_eda_finalize(snmptm);
   }
   
   {
      NCSVDA_INFO     vda_info;
      uns32 status;
      
      if (snmptm->node_id == 1)
          m_NCS_SET_VDEST_ID_IN_MDS_DEST(snmptm_vdest, SNMPTM_VCARD_ID1);
      else 
          m_NCS_SET_VDEST_ID_IN_MDS_DEST(snmptm_vdest, SNMPTM_VCARD_ID2);

      /* destroy the VDEST */
      memset(&vda_info, 0, sizeof(NCSVDA_INFO));
      vda_info.req = NCSVDA_VDEST_DESTROY;
      vda_info.info.vdest_destroy.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
      vda_info.info.vdest_destroy.i_vdest = snmptm_vdest;
      vda_info.info.vdest_destroy.i_make_vdest_non_persistent = FALSE;
      status = ncsvda_api(&vda_info);
      if (status != NCSCC_RC_SUCCESS)
      {
          /* log that VDEST destroy failed */
          return status;
      }
   }

   m_SNMPTM_UNLOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_WRITE);
  
   /* Release the SNMPTM CB handle */
   ncshm_give_hdl(cb_hdl);

   /* Free the content of SNMPTM CB */ 
   snmptm_cb_destroy(snmptm);   
 
   return NCSCC_RC_SUCCESS;
}

#else /* NCS_SNMPTM */
extern int dummy;
#endif  /* NCS_SNMPTM */



