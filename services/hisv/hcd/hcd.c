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
*                                                                            *
*  MODULE NAME:  hcd.c                                                       *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the functionality of HPI Chassis Director. HCD       *
*  initializes and opens a session on a HPI session. It discovers the        *
*  resources on a default domain identifier and then creates the threads     *
*  for HSM, ShIM and HAM.It passes the HPI session and domain identifier     *
*  to these threads.                                                         *
*                                                                            *
*****************************************************************************/


#include "hcd.h"
#include "hcd_amf.h"

/* global cb handle */
uns32 gl_hcd_hdl;

static uns32 hisv_hcd_init(NCS_LIB_REQ_INFO *req_info);
static uns32 hisv_hcd_destroy (NCS_LIB_REQ_INFO *req_info);

/****************************************************************************
 * Name          : ncs_hisv_hcd_lib_req
 *
 * Description   : This is the NCS SE API which is used to init/destroy
 *                 HISv HCD module.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which SBOM gives.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ncs_hisv_hcd_lib_req (NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = hisv_hcd_init(req_info);
         if (res == NCSCC_RC_SUCCESS)
            m_LOG_HISV_DTS_CONS("ncs_hisv_hcd_lib_req: HCD create success\n")
         else
            m_LOG_HISV_DTS_CONS("ncs_hisv_hcd_lib_req: HCD create failed\n");
         break;

      case NCS_LIB_REQ_DESTROY:
         res = hisv_hcd_destroy(req_info);
         if (res == NCSCC_RC_SUCCESS)
            m_LOG_HISV_DTS_CONS("ncs_hisv_hcd_lib_req: HCD create success\n")
         else
            m_LOG_HISV_DTS_CONS("ncs_hisv_hcd_lib_req: HCD create failed\n");
#if (NCS_HISV_LOG == 1)
   /*
    * Un-Subscribe with DTSv for logging.
    */
   if (NCSCC_RC_SUCCESS != hisv_log_unbind())
   {
      m_LOG_HISV_DTS_CONS("ncs_hisv_hcd_lib_req: HISv Failed to unbind with DTSv\n");
   }
#endif
         break;

      default:
         m_LOG_HISV_DTS_CONS("ncs_hisv_hcd_lib_req: Unknown request in ncs_hisv_hcd_lib_req\n");
         break;
   }
   m_LOG_HISV_DTS_CONS("ncs_hisv_hcd_lib_req: Done\n");
   return (res);
}

/****************************************************************************
 * Name          : hisv_hcd_init
 *
 * Description   : This function is used to start the main HCD process. It
 *                 opens the HPI session, discover the resources and creates
 *                 the threads for HSM, ShIM and HAM.
 *
 * Arguments     : chassis-id - identfier of chassis managed by this HCD.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static
uns32 hisv_hcd_init(NCS_LIB_REQ_INFO *req_info)
{
   SaErrorT           err;
   SaHpiSessionIdT    session_id = 0;
   SaHpiRptEntryT     entry;
   /* SaHpiSelInfoT   Info; */
   SaAisErrorT    amf_error;

   HPI_SESSION_ARGS * dom_args;
   HSM_CB *hsm_cb;
   SIM_CB *sim_cb;
   HAM_CB *ham_cb;
   HCD_CB    *hcd_cb;
   uns32 rc = NCSCC_RC_FAILURE; 
#ifdef HPI_A
   uns32 retry;
   SaHpiVersionT      version;
#endif

   /* Register with Logging subsystem */
#if (NCS_HISV_LOG == 1)
   /*
    * Subscribe with DTSv for logging.
    */
   if (NCSCC_RC_SUCCESS != hisv_log_bind())
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: HISv Failed to bind to the DTSv logging service\n");
      return NCSCC_RC_FAILURE;
   }
#endif

   /* Allocate and initialize the control block*/
   hcd_cb = m_MMGR_ALLOC_HISV_HCD_CB;

   if (hcd_cb == NULL)
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: malloc error\n");
      m_LOG_HISV_MEMFAIL(HCD_CB_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_OS_MEMSET(hcd_cb, 0,sizeof(HCD_CB));

   hcd_cb->hm_poolid = NCS_HM_POOL_ID_COMMON;
   hcd_cb->my_hdl = ncshm_create_hdl(hcd_cb->hm_poolid,
                                     NCS_SERVICE_ID_HCD, (NCSCONTEXT)hcd_cb);
   gl_hcd_hdl = hcd_cb->my_hdl;

   if(0 == hcd_cb->my_hdl)
   {
      m_LOG_HISV_HEADLINE(HCD_CREATE_HANDLE_FAILED, NCSFL_SEV_CRITICAL);
      m_MMGR_FREE_HISV_HCD_CB(hcd_cb);
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error creating hcd_cb handle\n");
      return NCSCC_RC_FAILURE;
   }
   /* allocate memory for domain arguments */
   if ( NULL == (dom_args = m_MMGR_ALLOC_HPI_SESSION_ARGS))
   {
      /* log */
      rc = NCSCC_RC_FAILURE;
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: malloc error for domain args\n");
      return rc;
   }
   m_NCS_OS_MEMSET(dom_args, 0, sizeof(HPI_SESSION_ARGS));
   hcd_cb->args = dom_args;

   /* Initialize the cb parameters */
   if (hcd_cb_init(hcd_cb) != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_HISV_HCD_CB(hcd_cb);
      m_MMGR_FREE_HPI_SESSION_ARGS(dom_args);
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error hcd_cb_init\n");
      return NCSCC_RC_FAILURE;
   }
   /* Initialize the mail box */
   if ((m_NCS_IPC_CREATE(&hcd_cb->mbx) != NCSCC_RC_SUCCESS) ||
       (m_NCS_IPC_ATTACH(&hcd_cb->mbx) != NCSCC_RC_SUCCESS))
   {
      m_MMGR_FREE_HISV_HCD_CB(hcd_cb);
      m_MMGR_FREE_HPI_SESSION_ARGS(dom_args);
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error creating mail box\n");
      return (NCSCC_RC_FAILURE);
   }
   m_LOG_HISV_DTS_CONS("hisv_hcd_init: Starting HPI Chassis Director...\n");

#ifdef HPI_A
   retry = 0;
   /* first step in openhpi */
   m_LOG_HISV_DTS_CONS("hisv_hcd_init: HPI Initialization...\n");

   while (retry++ < HPI_INIT_MAX_RETRY)
   {
      err = saHpiInitialize(&version);
      if ((SA_OK != err) && (err != SA_ERR_HPI_DUPLICATE))
      {
         m_LOG_HISV_DEBUG("hisv_hcd_init: saHpiInitialize Failed - May need to do E-Keying\n");
         m_NCS_CONS_PRINTF("hisv_hcd_init: saHpiInitialize Error %d: May need to do E-keying of interface to shelf manager\n", err);
         m_LOG_HISV_DTS_CONS("hisv_hcd_init: Re-Trying...\n");
         saHpiFinalize();
         continue;
      }
      else
         break;

   }
   if (retry >= HPI_INIT_MAX_RETRY)
   {
      m_LOG_HISV_DEBUG("hisv_hcd_init: saHpiInitialize Failed - May need to do E-Keying\n");
      m_NCS_CONS_PRINTF("hisv_hcd_init: saHpiInitialize Error %d: May need to do E-keying of interface to shelf manager\n", err);
      m_LOG_HISV_DTS_CONS("hsm_rediscover: This is expected if standby SCXB does not have connectivity to shelf manager\n");
      dom_args->session_valid = 0;
      dom_args->rediscover = 1;
      /* m_MMGR_FREE_HPI_SESSION_ARGS(dom_args); */
      /* return NCSCC_RC_FAILURE; */
   }
   else
   {
#endif
      /* Every domain requires a new session */
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: Opening HPI Session...\n");
#ifdef HPI_A
      err = saHpiSessionOpen(SAHPI_DEFAULT_DOMAIN_ID, &session_id, NULL);
#else
      err = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &session_id, NULL);
#endif
      if (SA_OK != err)
      {
         m_LOG_HISV_DTS_CONS("hisv_hcd_init; saHpiSessionOpen\n");
         dom_args->session_valid = 0;
         dom_args->rediscover = 1;
         /* m_MMGR_FREE_HPI_SESSION_ARGS(dom_args); */
         /* return NCSCC_RC_FAILURE; */
      }
      else
      {
         /* store the HPI session information */
#ifdef HPI_A
         dom_args->domain_id = SAHPI_DEFAULT_DOMAIN_ID;
#else
         dom_args->domain_id = SAHPI_UNSPECIFIED_DOMAIN_ID;
#endif
         dom_args->session_id = session_id;
         dom_args->chassis_id = 2;  /* don't mind, it will be discovered internally */
         dom_args->session_valid = 1;

         m_NCS_MEMCPY(&dom_args->entry, &entry, sizeof(SaHpiRptEntryT));

         m_LOG_HISV_DTS_CONS("hisv_hcd_init: HPI Discovering Resources...\n");
         /* discover the HPI resources */
         if (NCSCC_RC_FAILURE == discover_domain(dom_args))
         {
            dom_args->session_valid = 0;
            dom_args->rediscover = 1;
            /* m_MMGR_FREE_HPI_SESSION_ARGS(dom_args); */
            /* return NCSCC_RC_FAILURE; */
         }
         if (dom_args->discover_domain_err == TRUE)
            m_LOG_HISV_DTS_CONS("hisv_hcd_init: discover_domain_err TRUE\n");

         m_LOG_HISV_DTS_CONS("hisv_hcd_init: Initializing HSM...\n");
      }
#ifdef HPI_A
   }
#endif
   /* initialize the HSM control block */
   if (NCSCC_RC_FAILURE == hsm_initialize(dom_args))
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: hsm_initialize failed\n");
      /* finalize HPI session */
      saHpiSessionClose(session_id);
      /* finalize the HPI session */
#ifdef HPI_A
      saHpiFinalize();
#endif
      return NCSCC_RC_FAILURE;
   }

   m_LOG_HISV_DTS_CONS("hisv_hcd_init: Initializing SIM...\n");
   /* Intialize ShIM controlblock */
   if ( (NCSCC_RC_SUCCESS != sim_initialize()))
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: Error intializing ShIM\n");
      /* stop and release HSM */
      hsm_finalize();
      return NCSCC_RC_FAILURE;
   }

   /* retrieve HSM CB */

   hsm_cb = (HSM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hsm_hdl);

   /* Create HCD-HSM thread */
   if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_CREATE((NCS_OS_CB)hcd_hsm,
                                          0, "HSM", HSM_TASK_PRIORITY,
                                          (NCS_STACKSIZE_HUGE*2),
                                          &hsm_cb->task_hdl
                                          )))
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error creating HSM thread\n");
      /* return HSM CB */
      ncshm_give_hdl(gl_hsm_hdl);
      hsm_finalize();
      return NCSCC_RC_FAILURE;
   }

   /* Put the HSM thread in the start state */
   if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_START(hsm_cb->task_hdl)))
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error starting HSM thread\n");
      ncshm_give_hdl(gl_hsm_hdl);
      hsm_finalize();
      return NCSCC_RC_FAILURE;
   }
   /* return HSM CB */
  ncshm_give_hdl(gl_hsm_hdl);

   /* retrieve ShIM CB */

   sim_cb = (SIM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_sim_hdl);

   /* Create ShIM thread */
   if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_CREATE((NCS_OS_CB)hcd_sim,
                                          0, "SIM", SIM_TASK_PRIORITY,
                                          NCS_STACKSIZE_HUGE,
                                          &sim_cb->task_hdl
                                          )))
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error creating ShIM thread\n");
      /* return SIM CB */
      ncshm_give_hdl(gl_sim_hdl);
      /* kill the created HSM task and control block */
      hsm_finalize();
      return NCSCC_RC_FAILURE;
   }

   /* Put the ShIM thread in the start state */
   if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_START(sim_cb->task_hdl)))
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error starting SIM thread\n");
      /* return SIM CB */
      ncshm_give_hdl(gl_sim_hdl);
      /* kill the created HSM & ShIM tasks and control blocks */
      hsm_finalize();
      sim_finalize();
      return NCSCC_RC_FAILURE;
   }
   /* return ShIM CB */
   ncshm_give_hdl(gl_sim_hdl);

   m_LOG_HISV_DTS_CONS("hisv_hcd_init: Initializing HAM...\n");
   /* Intialize HAM controlblock, and register it with MDS */
   if ( (NCSCC_RC_SUCCESS != ham_initialize(dom_args)))
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: Error intializing HAM\n");
      /* stop and release HSM, ShIM */
      hsm_finalize();
      sim_finalize();
      return NCSCC_RC_FAILURE;
   }
   /* retrieve HAM CB */

   ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl);

   /* Create HCD-HAM thread */
   if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_CREATE((NCS_OS_CB)hcd_ham,
                                          0, "HAM", HAM_TASK_PRIORITY,
                                          NCS_STACKSIZE_HUGEX2,
                                          &ham_cb->task_hdl
                                          )))
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error creating HAM thread\n");
      /* return HAM CB */
      ncshm_give_hdl(gl_ham_hdl);
      /* kill the created HSM, ShIM task and control block */
      hsm_finalize();
      sim_finalize();
      return NCSCC_RC_FAILURE;
   }

   /* Put the HAM thread in the start state */
   if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_START(ham_cb->task_hdl)))
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error starting HAM thread\n");
      /* return HAM CB */
      ncshm_give_hdl(gl_ham_hdl);
      /* kill the created HSM, ShIM & HAM tasks and control blocks */
      hsm_finalize();
      ham_finalize();
      sim_finalize();
      return NCSCC_RC_FAILURE;
   }
   /* return HAM CB */
   ncshm_give_hdl(gl_ham_hdl);

   m_LOG_HISV_DTS_CONS("hisv_hcd_init: HSM, HAM, ShIM threads created\n");

#if 1
  /* Initialize amf framework */
   m_LOG_HISV_DTS_CONS("hisv_hcd_init: Initializing AMF..\n");
   if (hcd_amf_init(hcd_cb) != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error initializing AMF\n");
      /* kill the created HSM, ShIM & HAM tasks and control blocks */
      hsm_finalize();
      ham_finalize();
      sim_finalize();
      return NCSCC_RC_FAILURE;
   }
    /* register HCD component with AvSv */
   m_LOG_HISV_DTS_CONS("hisv_hcd_init: Registering with AMF..\n");
   amf_error = saAmfComponentRegister(hcd_cb->amf_hdl,
               &hcd_cb->comp_name, (SaNameT*)NULL);
   if (amf_error != SA_AIS_OK)
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_init: error registering with AMF\n");
      m_LOG_HISV_SVC_PRVDR(HCD_AMF_REG_ERROR,NCSFL_SEV_CRITICAL);
      saAmfFinalize(hcd_cb->amf_hdl);
      /* kill the created HSM, ShIM & HAM tasks and control blocks */
      hsm_finalize();
      ham_finalize();
      sim_finalize();
      return NCSCC_RC_FAILURE;
   }
   m_LOG_HISV_DTS_CONS("hisv_hcd_init: AMF registration done\n");
   /** start the AMF health check **/
   m_LOG_HISV_DTS_CONS("hisv_hcd_init: Starting Health check with AMF..\n");
   rc = hisv_hcd_health_check(&hcd_cb->mbx);
#endif /* 1 */
   return rc;
}

/****************************************************************************
 * Name          : hisv_hcd_destroy
 *
 * Description   : Invoked to destroy the HCD
 *
 *
 * Arguments     :
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/

static
uns32 hisv_hcd_destroy (NCS_LIB_REQ_INFO *req_info)
{
   HCD_CB *hcd_cb;

   if ((hcd_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hcd_hdl))
                  == NULL)
   {
      m_LOG_HISV_DTS_CONS("hisv_hcd_destroy: error\n");
      m_LOG_HISV_HEADLINE(HCD_TAKE_HANDLE_FAILED, NCSFL_SEV_CRITICAL);
      return (NCSCC_RC_FAILURE);
   } else
   {
      saAmfComponentUnregister(hcd_cb->amf_hdl, &hcd_cb->comp_name,
         (SaNameT*)NULL);
      saAmfFinalize(hcd_cb->amf_hdl);

      ncshm_give_hdl(gl_hcd_hdl);
      ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, gl_hcd_hdl);
      m_NCS_IPC_DETACH(&hcd_cb->mbx , hcd_clear_mbx, hcd_cb);

      hcd_cb_destroy(hcd_cb);
      m_MMGR_FREE_HISV_HCD_CB(hcd_cb);
   }
   m_LOG_HISV_DTS_CONS("hisv_hcd_destroy: done\n");
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : hcd_cb_init
 *
 * Description   : This function is invoked at init time. Initiaziles all the
 *                 parameters in the CB
 *
 * Arguments     : hcd_cb  - HCDD control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
hcd_cb_init (HCD_CB *hcd_cb)
{
   uns32 rc = NCSCC_RC_FAILURE;
   rc = m_NCS_SEM_CREATE(&hcd_cb->p_s_handle);
   hcd_cb->args->chassis_id = 2;  /* don't mind, it will be discovered internally */
   hcd_cb->ha_state = SA_AMF_HA_STANDBY;
   hcd_cb->my_anc = V_DEST_QA_2;
   hcd_cb->mds_role = V_DEST_RL_STANDBY;
   hcd_cb->args->rediscover = 0;
   hcd_cb->args->session_valid = 0;
   return rc;
}

/****************************************************************************
 * Name          : hcd_cb_destroy
 *
 * Description   : This function is invoked at destroy time. This function will
 *                 free all the dynamically allocated memory
 *
 * Arguments     : hcd_cb  - HCD control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
hcd_cb_destroy (HCD_CB *hcd_cb)
{
   uns32 rc = NCSCC_RC_FAILURE;
   rc = m_NCS_SEM_RELEASE(hcd_cb->p_s_handle);
   return rc;
}

/****************************************************************************
 * Name          : hcd_clear_mbx
 *
 * Description   : This is the function which deletes all the messages from
 *                 the mail box.
 *
 * Arguments     : arg     - argument to be passed.
 *                 msg     - Event start pointer.
 *
 * Return Values : TRUE/FALSE
 *
 * Notes         : None.
 *****************************************************************************/
NCS_BOOL
hcd_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg)
{
   HISV_EVT  *pEvt = (HISV_EVT *)msg;
   HISV_EVT  *pnext;
   pnext = pEvt;
   while (pnext)
   {
      pnext = pEvt->next;
      m_MMGR_FREE_HISV_EVT(pEvt);
      pEvt = pnext;
   }
   return TRUE;
}
