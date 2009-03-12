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

  MODULE NAME: SRMA_CB.C
 
..............................................................................

  DESCRIPTION: This file contains SRMA CB specific routines

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/
#include "srma.h"

/* global cb handle */
uns32 gl_srma_hdl = 0;
static uns32 srma_use_count = 0;

/* SRMA Agent creation specific LOCK */
static uns32 srma_agent_lock_create = 0;
NCS_LOCK srma_agent_lock;

#define m_SRMA_AGENT_LOCK                       \
   if (!srma_agent_lock_create++)               \
   {                                            \
      m_NCS_LOCK_INIT(&srma_agent_lock);        \
   }                                            \
   srma_agent_lock_create = 1;                  \
   m_NCS_LOCK(&srma_agent_lock, NCS_LOCK_WRITE);

#define m_SRMA_AGENT_UNLOCK m_NCS_UNLOCK(&srma_agent_lock, NCS_LOCK_WRITE)

/***************************************************************************
                       Static Function Prototypes 
***************************************************************************/
static uns32 srma_create(NCS_LIB_CREATE *);
static uns32 srma_destroy(void);
static void  srma_cb_delete(SRMA_CB *srma);
static void  srma_db_delete(SRMA_CB *srma);

/****************************************************************************
  Name          :  srma_create
 
  Description   :  This function initializes the SRMSv for the invoking 
                   process.
 
  Arguments     :  create_info - ptr to the NCS_LIB_CREATE structure.
 
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_create(NCS_LIB_CREATE *create_info)
{
   SRMA_CB *srma = SRMA_CB_NULL;
   uns32   rc  = NCSCC_RC_SUCCESS;
   NCS_PATRICIA_PARAMS params;
   
   /* register with dtsv */
#if (NCS_SRMA_LOG == 1)
   rc = srma_log_reg();
#endif
   
   /* allocate SRMA CB */
   if (!(srma = m_MMGR_ALLOC_SRMA_CB))
   {
      m_SRMA_LOG_MEM(SRMA_MEM_SRMA_CB,
                     SRMA_MEM_ALLOC_FAILED,
                     NCSFL_SEV_CRITICAL);

      return NCSCC_RC_FAILURE;      
   }

   m_SRMA_LOG_CB(SRMSV_LOG_CB_CREATE,
                 SRMSV_LOG_CB_SUCCESS,
                 NCSFL_SEV_INFO);

   memset(srma, 0, sizeof(SRMA_CB));

   /* assign the SRMA pool-id (used by hdl-mngr) */
   srma->pool_id = NCS_HM_POOL_ID_COMMON;

   /* Update the local NODE ID info */
   srma->node_num = m_NCS_GET_NODE_ID;

   /* create the association with hdl-mngr */
   if (!(srma->cb_hdl = ncshm_create_hdl(srma->pool_id, NCS_SERVICE_ID_SRMA, 
                                         (NCSCONTEXT)srma)))
   {
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_CREATE_CB,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);

      /* Delete SRMA Control Block */
      srma_cb_delete(srma);
      return NCSCC_RC_FAILURE;      
   }

   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_CREATE_CB,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* initialize the SRMA cb lock */
   m_NCS_LOCK_INIT(&srma->lock);   

   /* EDU initialisation */
   m_NCS_EDU_HDL_INIT(&srma->edu_hdl);
   
   /* Initialize Resource Mon Tree */
   memset(&params, 0, sizeof(NCS_PATRICIA_PARAMS));
   params.key_size = sizeof(uns32);
   params.info_size = 0;
   rc = ncs_patricia_tree_init(&srma->rsrc_mon_tree, &params);
   if (rc != NCSCC_RC_SUCCESS)
   {   
      /* Delete SRMA Control Block */
      srma_cb_delete(srma);
      return NCSCC_RC_FAILURE;      
   }   

   /* Versioning changes */
   srma->srma_mds_ver = SRMA_SVC_PVT_VER;

   /* register with MDS */
   if (srma_mds_reg(srma) != NCSCC_RC_SUCCESS)
   {
      /* Delete SRMA Control Block */
      srma_cb_delete(srma);

      return NCSCC_RC_FAILURE;            
   }
   
   /* everything went off well.. store the cb hdl in the global variable */
   gl_srma_hdl = srma->cb_hdl;

   return rc;
}


/****************************************************************************
  Name          :  srma_destroy
 
  Description   :  This function destroys the SRMSv service.                    
 
  Arguments     :  Nothing
 
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_destroy()
{
   SRMA_CB *srma = 0;
   
   /* retrieve SRMA CB */
   srma = m_SRMSV_RETRIEVE_SRMA_CB;
   if (!srma) 
   {
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                    SRMSV_LOG_HDL_SUCCESS,
                    NCSFL_SEV_CRITICAL);

      m_SRMA_LOG_MISC(SRMA_LOG_MISC_AGENT_DESTROY,
                      SRMA_LOG_MISC_FAILED,
                      NCSFL_SEV_CRITICAL);
      /* unregister with DTSv */
#if (NCS_SRMA_LOG == 1)
      srma_log_unreg();
#endif
      return NCSCC_RC_FAILURE;
   }

   /* unregister with MDS */
   srma_mds_unreg(srma);   

   /* EDU cleanup */
   m_NCS_EDU_HDL_FLUSH(&srma->edu_hdl);

   /* Delete the complete SRMA database */
   srma_db_delete(srma);

   /* De-register from NCS RP timer */
   srma_timer_destroy(srma);   

   /* return SRMA CB */
   m_SRMSV_GIVEUP_SRMA_CB;

   /* Delete the SRMA CB */
   srma_cb_delete(srma);

   /* reset the global cb handle */
   gl_srma_hdl = 0;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srma_lib_req
 
  Description   :  This routine is exported to the external entities & is
                   used to create & destroy the SRMA library.
  
  Arguments     :  req_info - ptr to the request info
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
uns32 srma_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   switch (req_info->i_op)
   {
   case NCS_LIB_REQ_CREATE:
      rc =  srma_create(&req_info->info.create);
      if (rc == NCSCC_RC_SUCCESS)
      {
         m_SRMA_LOG_MISC(SRMA_LOG_MISC_AGENT_CREATE,
                         SRMA_LOG_MISC_SUCCESS,
                         NCSFL_SEV_INFO);
      }
      else
      {
         m_SRMA_LOG_MISC(SRMA_LOG_MISC_AGENT_CREATE,
                         SRMA_LOG_MISC_FAILED,
                         NCSFL_SEV_CRITICAL);
      }
      break;
       
   case NCS_LIB_REQ_DESTROY:
      rc = srma_destroy();      
      break;
       
   default:
      break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srma_db_delete
 
  Description   :  This routine deletes the database that was acquired by 
                   SRMA.
 
  Arguments     :  srma - ptr to the SRMA control block
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
void srma_db_delete(SRMA_CB *srma)
{
   SRMA_USR_APPL_NODE  *appl = NULL, *next_appl = NULL;
   SRMA_SRMND_USR_NODE *srmnd_usr = NULL, *next_srmnd_usr = NULL;
   SRMA_SRMND_INFO *srmnd = NULL, *next_srmnd = NULL;
   
   appl  = srma->start_usr_appl_node;
   srmnd = srma->start_srmnd_node;

   /* Delete User Application specific records - All the resource mon records 
      are deleted from patricia tree and from the SRMA database */
   /* Traverse the complete application specific list and delete 
      its resource mon nodes */
   while (appl)
   {
      next_appl = appl->next_usr_appl_node;

      /* Delete the configured rsrc's of an application */
      srma_usr_appl_delete(srma, appl->user_hdl, FALSE);

      /* Free the allocated memory for application node */
      m_MMGR_FREE_SRMA_USR_APPL_NODE(appl);

      /* Continue deleting the application specific next SRMND node */
      appl = next_appl;
   }  /* appl */   

   /* Delete User SRMND specific records and the SRMND specific User
      Application records. Traverse the complete SRMND specific list */
   while (srmnd)
   {
       next_srmnd = srmnd->next_srmnd_node;
       srmnd_usr = srmnd->start_usr_node;

       while (srmnd_usr)
       {
          next_srmnd_usr = srmnd_usr->next_usr_node;
          /* Free the allocated memory for srmnd_usr */
          m_MMGR_FREE_SRMA_SRMND_APPL_NODE(srmnd_usr);
          srmnd_usr = next_srmnd_usr;
       }

       srmnd = next_srmnd;
   } /* srmnd */
   
   return;
}


/****************************************************************************
  Name          :  srma_cb_delete
 
  Description   :  This function cleans up the SRMA_CB contents.
 
  Arguments     :  srma - ptr to the SRMA_CB structure
 
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
static void srma_cb_delete(SRMA_CB *srma)
{
   /* remove the association with hdl-mngr */
   if (srma->cb_hdl)
   {
      ncshm_destroy_hdl(NCS_SERVICE_ID_SRMA, srma->cb_hdl);
      /* LOG appropriate message */
   }


   /* Destroy the initiates PAT resource-mon tree */
   ncs_patricia_tree_destroy(&srma->rsrc_mon_tree);

   /* destroy the lock */
   m_NCS_LOCK_DESTROY(&srma->lock);
  
   m_SRMA_LOG_MISC(SRMA_LOG_MISC_AGENT_DESTROY,
                   SRMA_LOG_MISC_SUCCESS,
                   NCSFL_SEV_INFO);

   /* unregister with DTSv */
#if (NCS_SRMA_LOG == 1)
   srma_log_unreg();
#endif

   /* free the control block */
   m_MMGR_FREE_SRMA_CB(srma);
      
   return;
}


/****************************************************************************
  Name          :  ncs_srma_startup

  Description   :  This routine creates a SRMSv agent infrastructure to interface
                   with SRMSv service. Once the infrastructure is created from
                   then on use_count is incremented for every startup request.

  Arguments     :  - NIL-

  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         :  None
******************************************************************************/
unsigned int ncs_srma_startup(void)
{
   NCS_LIB_REQ_INFO lib_create;

   m_SRMA_AGENT_LOCK;
   if (srma_use_count > 0)
   {
      /* Already created, so just increment the use_count */
      srma_use_count++;
      m_SRMA_AGENT_UNLOCK;
      return NCSCC_RC_SUCCESS;
   }

   /*** Init SRMA ***/
   memset(&lib_create, 0, sizeof(lib_create));
   lib_create.i_op = NCS_LIB_REQ_CREATE;
   if (srma_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {
      m_SRMA_AGENT_UNLOCK;
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   else
   {
      m_NCS_DBG_PRINTF("\nSRMSV:SRMA:ON");
      srma_use_count = 1;
   }

   m_SRMA_AGENT_UNLOCK;
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncs_srma_shutdown 

  Description   :  This routine destroys the SRMSv agent infrastructure created 
                   to interface SRMSv service. If the registered users are > 1, 
                   it just decrements the use_count.   

  Arguments     :  - NIL -

  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         :  None
******************************************************************************/
unsigned int ncs_srma_shutdown(void)
{
   uns32 rc = NCSCC_RC_SUCCESS;


   m_SRMA_AGENT_LOCK;
   if (srma_use_count > 1)
   {
      /* Still users extis, so just decrement the use_count */
      srma_use_count--;
   }
   else if (srma_use_count == 1)
   {
      NCS_LIB_REQ_INFO  lib_destroy;

      memset(&lib_destroy, 0, sizeof(lib_destroy));
      lib_destroy.i_op = NCS_LIB_REQ_DESTROY;

      rc = srma_lib_req(&lib_destroy);

      srma_use_count = 0;
   }

   m_SRMA_AGENT_UNLOCK;
   return rc;
}





