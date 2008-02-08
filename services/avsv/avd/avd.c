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

  DESCRIPTION:

  This module is the main module for Availability Director. This module
  deals with the initialisation of AvD.


..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_init_cmplt - completes all the initialisations after role detrmination.
  avd_init_proc - entry function to AvD thread or task.
  avd_initialize - creation and starting of AvD task/thread.
  avd_tmr_cl_init_func - the event handler for AMF cluster timer expiry.
  avd_destroy - the destruction of the AVD module.
  avd_lib_req - DL SE API for init and destroy of the AVD module

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"


/* the global handle holder that is used for verfying multiple initialisations
 * of AvD.
 */
uns32 g_avd_hdl = 0;
void *g_avd_task_hdl = NULL;
void *g_avd_hb_task_hdl = NULL;
static void avd_hb_task_create(void);

/*****************************************************************************
 * Function: avd_init_cmplt
 *
 * Purpose:  This module completes all the initialisations that need to be
 * done by the AvD after the role is determined.
 *
 * Input: cb - cb - the AVD control block
 *
 * Returns: NONE.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_init_cmplt(AVD_CL_CB *cb)
{

   m_AVD_LOG_FUNC_ENTRY("avd_init_cmplt");
   
   /* Initialise MDS */
   if (avd_mds_reg(cb) != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   /* Initialise EDS */
   if (avd_evd_init(cb) != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }   

   /*
    * Register with MBCSV.
    */
   if (NCSCC_RC_FAILURE == avsv_mbcsv_register(cb))
   {
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   /* Initialise MAB */ 
   if (avd_mab_init(cb) != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }
   
   return;
}


/*****************************************************************************
 * Function: avd_init_proc
 *
 * Purpose: This is entry function to AvD thread or task. This function
 * will not return forever if no errors in initialization. It calls all
 * initialize routines that don't need role and then calls the process
 * module.
 *
 * Input: avd_hdl_ptr - pointer to the Control block handle.
 *
 * Returns: None.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static void avd_init_proc(uns32 *avd_hdl_ptr)
{
   AVD_CL_CB *cb;
   NCS_PATRICIA_PARAMS  patricia_params;
   NCS_SEL_OBJ     mbx_sel_obj;
   NCS_SEL_OBJ     hb_mbx_sel_obj;
   NCS_SEL_OBJ     mbcsv_ncs_sel_obj;

   m_AVD_LOG_FUNC_ENTRY("avd_init_proc");
   
   /* check if the value of g_avd_hdl matches the avd_hdl passed  */
   if (avd_hdl_ptr == NULL)
   {
      m_AVD_LOG_INVALID_VAL_FATAL(0);
      return;
   }

   if (*avd_hdl_ptr != g_avd_hdl)
   {
      m_AVD_LOG_INVALID_VAL_FATAL(*avd_hdl_ptr);
      return;
   }

   /* get the CB from the handle manager */
   if ((cb = ncshm_take_hdl(NCS_SERVICE_ID_AVD,*avd_hdl_ptr)) ==
        AVD_CL_CB_NULL)
   {
      m_AVD_LOG_INVALID_VAL_FATAL(AVD_CL_CB_NULL);
      return;
   }

   /* Initialize all the locks and trees in the CB */

   if (NCSCC_RC_FAILURE == m_AVD_CB_LOCK_INIT(cb))
   {
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_LOCK_ERROR(AVSV_LOG_LOCK_INIT);
      return;
   }

    if (NCSCC_RC_FAILURE == m_AVD_CB_AVND_TBL_LOCK_INIT(cb))
   {
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_LOCK_ERROR(AVSV_LOG_LOCK_INIT);
      return;
   }

  m_AVD_LOG_LOCK_SUCC(AVSV_LOG_LOCK_INIT);


   cb->init_state = AVD_INIT_BGN;
   cb->cluster_admin_state = NCS_ADMIN_STATE_UNLOCK;
   cb->rcv_hb_intvl = AVSV_RCV_HB_INTVL;
   cb->snd_hb_intvl = AVSV_SND_HB_INTVL;
   cb->amf_init_intvl = AVSV_CLUSTER_INIT_INTVL;
   cb->role_switch = SA_FALSE;
   cb->stby_sync_state = AVD_STBY_IN_SYNC;
   cb->sync_required = TRUE;
   cb->avd_hrt_beat_rcvd = FALSE;

    /* set the mailbox selection object to the list */
   mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(&cb->avd_mbx);
   m_NCS_SEL_OBJ_ZERO(&cb->sel_obj_set);
   m_NCS_SEL_OBJ_SET(mbx_sel_obj, &cb->sel_obj_set);
   cb->sel_high = mbx_sel_obj;

    /* set the health check mailbox selection object to the list */
   hb_mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(&cb->avd_hb_mbx);
   m_NCS_SEL_OBJ_ZERO(&cb->hb_sel_obj_set);
   m_NCS_SEL_OBJ_SET(hb_mbx_sel_obj, &cb->hb_sel_obj_set);
   cb->hb_sel_high = hb_mbx_sel_obj;

   patricia_params.key_size = sizeof(SaClmNodeIdT);
   if( ncs_patricia_tree_init(&cb->node_list, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   patricia_params.key_size = sizeof(SaClmNodeIdT);
   if( ncs_patricia_tree_init(&cb->avnd_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   patricia_params.key_size = sizeof(SaNameT);
   if( ncs_patricia_tree_init(&cb->avnd_anchor_name, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);      
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }


   patricia_params.key_size = sizeof(SaNameT);
   if( ncs_patricia_tree_init(&cb->sg_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);      
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   patricia_params.key_size = sizeof(SaNameT);
   if( ncs_patricia_tree_init(&cb->su_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   patricia_params.key_size = sizeof(SaNameT);
   if( ncs_patricia_tree_init(&cb->si_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   patricia_params.key_size = sizeof(SaNameT);
   if( ncs_patricia_tree_init(&cb->comp_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   patricia_params.key_size = sizeof(SaNameT);
   if( ncs_patricia_tree_init(&cb->csi_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncs_patricia_tree_destroy(&cb->comp_anchor);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }


   patricia_params.key_size = ( sizeof(AVSV_HLT_KEY) -  sizeof(SaUint16T));
   if( ncs_patricia_tree_init(&cb->hlt_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncs_patricia_tree_destroy(&cb->csi_anchor);
      ncs_patricia_tree_destroy(&cb->comp_anchor);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   patricia_params.key_size = sizeof(AVD_SUS_PER_SI_RANK_INDX);
   if( ncs_patricia_tree_init(&cb->su_per_si_rank_anchor, &patricia_params)
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->csi_anchor);
      ncs_patricia_tree_destroy(&cb->comp_anchor);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }


   patricia_params.key_size = sizeof(AVD_SG_SI_RANK_INDX);
   if( ncs_patricia_tree_init(&cb->sg_si_rank_anchor, &patricia_params)
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->csi_anchor);
      ncs_patricia_tree_destroy(&cb->comp_anchor);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncs_patricia_tree_destroy(&cb->su_per_si_rank_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

  
   patricia_params.key_size = sizeof(AVD_COMP_CS_TYPE_INDX);
   if( ncs_patricia_tree_init(&cb->comp_cs_type_anchor, &patricia_params)
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->csi_anchor);
      ncs_patricia_tree_destroy(&cb->comp_anchor);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncs_patricia_tree_destroy(&cb->su_per_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->sg_si_rank_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }


   patricia_params.key_size = sizeof(AVD_SG_SU_RANK_INDX);
   if( ncs_patricia_tree_init(&cb->sg_su_rank_anchor, &patricia_params)
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->csi_anchor);
      ncs_patricia_tree_destroy(&cb->comp_anchor);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncs_patricia_tree_destroy(&cb->su_per_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->sg_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->comp_cs_type_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }


   patricia_params.key_size = sizeof(AVD_CS_TYPE_PARAM_INDX);
   if( ncs_patricia_tree_init(&cb->cs_type_param_anchor, &patricia_params)
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->csi_anchor);
      ncs_patricia_tree_destroy(&cb->comp_anchor);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncs_patricia_tree_destroy(&cb->su_per_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->sg_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->comp_cs_type_anchor);
      ncs_patricia_tree_destroy(&cb->sg_su_rank_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }


   /* Initialize MIBLIB routines */
   if (avd_miblib_init(cb) != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncs_patricia_tree_destroy(&cb->hlt_anchor);
      ncs_patricia_tree_destroy(&cb->csi_anchor);
      ncs_patricia_tree_destroy(&cb->comp_anchor);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncs_patricia_tree_destroy(&cb->su_per_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->sg_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->comp_cs_type_anchor);
      ncs_patricia_tree_destroy(&cb->sg_su_rank_anchor);
      ncs_patricia_tree_destroy(&cb->cs_type_param_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   /* Initialise HPI and get local node info */
   if (avd_hpi_init(cb) != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncs_patricia_tree_destroy(&cb->hlt_anchor);
      ncs_patricia_tree_destroy(&cb->csi_anchor);
      ncs_patricia_tree_destroy(&cb->comp_anchor);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncs_patricia_tree_destroy(&cb->su_per_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->sg_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->comp_cs_type_anchor);
      ncs_patricia_tree_destroy(&cb->sg_su_rank_anchor);
      ncs_patricia_tree_destroy(&cb->cs_type_param_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   /* Initialise MDS */
   if (avd_mds_reg_def(cb) != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->node_list);
      ncs_patricia_tree_destroy(&cb->hlt_anchor);
      ncs_patricia_tree_destroy(&cb->csi_anchor);
      ncs_patricia_tree_destroy(&cb->comp_anchor);
      ncs_patricia_tree_destroy(&cb->si_anchor);
      ncs_patricia_tree_destroy(&cb->su_anchor);
      ncs_patricia_tree_destroy(&cb->sg_anchor);
      ncs_patricia_tree_destroy(&cb->avnd_anchor_name);
      ncs_patricia_tree_destroy(&cb->avnd_anchor);
      ncs_patricia_tree_destroy(&cb->su_per_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->sg_si_rank_anchor);
      ncs_patricia_tree_destroy(&cb->comp_cs_type_anchor);
      ncs_patricia_tree_destroy(&cb->sg_su_rank_anchor);
      ncs_patricia_tree_destroy(&cb->cs_type_param_anchor);
      ncshm_give_hdl(*avd_hdl_ptr);
      m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   /* get the node id of the node on which the AVD is running.
    */
   cb->node_id_avd = m_NCS_GET_NODE_ID;
   m_AVD_LOG_RCVD_VAL(cb->node_id_avd);

   /* send heartbeat to the other AvD on the obsolute address */

   /* start AVD_TMR_RCV_HB_INIT timer */

   /* temporary code to test AVD with single systemcontroller.
    * make this AVD instance as active
    */
   avd_init_cmplt(cb);

   /* Set the selection object fields by taking MBCSV selection object into account 
   */
   m_SET_FD_IN_SEL_OBJ((uns32)cb->mbcsv_sel_obj, mbcsv_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(mbcsv_ncs_sel_obj, &cb->sel_obj_set);

   cb->sel_high = m_GET_HIGHER_SEL_OBJ(mbcsv_ncs_sel_obj, mbx_sel_obj); 


   /* give back the handle */
   ncshm_give_hdl(*avd_hdl_ptr);

   avd_hb_task_create();

   /* call the main processing module that will not return*/
   
   avd_main_proc(cb);


}


/*****************************************************************************
 * Function: avd_initialize
 *
 * Purpose: This is the routine that does the creation and starting of
 * AvD task/thread.
 * 
 *
 * Input: req_info - pointer to the DL SE API request structure.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static uns32 avd_initialize(NCS_LIB_REQ_INFO *req_info)
{
   AVD_CL_CB *cb;

   m_AVD_LOG_FUNC_ENTRY("avd_initialize");
   
   /* check if g_avd_hdl is 0, if not AvD is
    * already initialised return now */
   if (g_avd_hdl != 0)
   {
      m_AVD_LOG_SEAPI_INIT_INFO(AVSV_LOG_SEAPI_FAILURE);
      return NCSCC_RC_FAILURE;
   }

#if (NCS_AVD_LOG == 1)
   /* Register with Logging subsystem. This is an agent call and
   ** could succeed even if the DTS server is not available 
   */
   avd_flx_log_reg();
#endif /* (NCS_AVD_LOG == 1) */

   /* Allocate the  control block and fill g_avd_hdl with
    * the handle returned by handle manager */

   cb = m_MMGR_ALLOC_AVD_CL_CB;

   if (AVD_CL_CB_NULL == cb)
   {
      m_AVD_LOG_MEM_FAIL(AVD_CB_ALLOC_FAILED);
      m_AVD_LOG_SEAPI_INIT_INFO(AVSV_LOG_SEAPI_FAILURE);
      return NCSCC_RC_FAILURE;
   }
   
   m_AVD_LOG_RCVD_VAL((long)cb);

   m_NCS_MEMSET(cb,'\0',sizeof(AVD_CL_CB));

   if ((g_avd_hdl = 
      ncshm_create_hdl(NCS_HM_POOL_ID_COMMON,NCS_SERVICE_ID_AVD,cb))
      == 0)
   {
      m_MMGR_FREE_AVD_CL_CB(cb);
      m_AVD_LOG_SEAPI_INIT_INFO(AVSV_LOG_SEAPI_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   cb->cb_handle = g_avd_hdl;
   m_AVD_LOG_RCVD_VAL((uns32)g_avd_hdl);
   
   /* create a mailbox . */

   if (m_NCS_IPC_CREATE (&cb->avd_mbx) != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_CREATE);
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVD,g_avd_hdl);
      g_avd_hdl = 0;
      m_MMGR_FREE_AVD_CL_CB(cb);
      m_AVD_LOG_SEAPI_INIT_INFO(AVSV_LOG_SEAPI_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_CREATE);

   m_NCS_IPC_ATTACH(&cb->avd_mbx);

   m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_ATTACH);

   /* create a mailbox for heart beat thread. */
   if (m_NCS_IPC_CREATE (&cb->avd_hb_mbx) != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_CREATE);
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVD,g_avd_hdl);
      g_avd_hdl = 0;
      m_NCS_IPC_RELEASE(&cb->avd_mbx, NULL);
      m_MMGR_FREE_AVD_CL_CB(cb);
      m_AVD_LOG_SEAPI_INIT_INFO(AVSV_LOG_SEAPI_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_CREATE);

   m_NCS_IPC_ATTACH(&cb->avd_hb_mbx);

   m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_ATTACH);


   /* create and start the AvD thread with the cb handle argument */

   if (m_NCS_TASK_CREATE ((NCS_OS_CB)avd_init_proc,
                    &g_avd_hdl,
                    NCS_AVD_NAME_STR,
                    NCS_AVD_PRIORITY,
                    NCS_AVD_STCK_SIZE,
                    &g_avd_task_hdl) != NCSCC_RC_SUCCESS)
   {
      m_NCS_IPC_RELEASE(&cb->avd_mbx, NULL);
      m_NCS_IPC_RELEASE(&cb->avd_hb_mbx, NULL);
      m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_DETACH);
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVD,g_avd_hdl);
      g_avd_hdl = 0;
      m_MMGR_FREE_AVD_CL_CB(cb);
      m_AVD_LOG_SEAPI_INIT_INFO(AVSV_LOG_SEAPI_FAILURE);
      return NCSCC_RC_FAILURE;
   }


   if (m_NCS_TASK_START (g_avd_task_hdl) != NCSCC_RC_SUCCESS)
   {
      m_NCS_TASK_RELEASE(g_avd_task_hdl);
      m_NCS_IPC_RELEASE(&cb->avd_mbx, NULL);
      m_NCS_IPC_RELEASE(&cb->avd_hb_mbx, NULL);
      m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_DETACH);
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVD,g_avd_hdl);
      g_avd_hdl = 0;
      m_MMGR_FREE_AVD_CL_CB(cb);
      m_AVD_LOG_SEAPI_INIT_INFO(AVSV_LOG_SEAPI_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   m_AVD_LOG_SEAPI_INIT_INFO(AVSV_LOG_SEAPI_SUCCESS);
   return NCSCC_RC_SUCCESS;

}



/****************************************************************************
 *  Name          : avd_tmr_cl_init_func
 * 
 *  Description   : This routine is the AMF initialisation timer expiry
 *                  routine handler. This routine calls the SG FSM
 *                  handler for each of the application SG in the AvSv
 *                  database. At the begining of the processing the AvD state
 *                  is changed to AVD_APP_STATE, the stable state.
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  evt        -  ptr to the received event
 * 
 *  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 * 
 *  Notes         : None.
 ***************************************************************************/

void avd_tmr_cl_init_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
   SaNameT  lsg_name_net;
   AVD_SG *i_sg;
   
   m_AVD_LOG_FUNC_ENTRY("avd_tmr_cl_init_func");
   
   if (evt->info.tmr.type != AVD_TMR_CL_INIT)
   {
      /* log error that a wrong timer type value */
      m_AVD_LOG_INVALID_VAL_FATAL(evt->info.tmr.type);
      return;
   }

   if (cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* log error that a cluster is admin down */
      m_AVD_LOG_INVALID_VAL_ERROR(cb->cluster_admin_state);
      return;
   }
   
   if (cb->init_state != AVD_INIT_DONE)
   {
      /* This is a error situation. Without completing
       * initialisation AVD will not start cluster timer.
       */

      m_AVD_LOG_INVALID_VAL_FATAL(cb->init_state);
      return;
   }


   /* change state to application state. */
   cb->init_state = AVD_APP_STATE;
   m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, cb, AVSV_CKPT_AVD_CB_CONFIG);
   
   /* call the realignment routine for each of the SGs in the
    * system that are not NCS specific.
    */

   lsg_name_net.length = 0;
   for (i_sg = avd_sg_struc_find_next(cb,lsg_name_net,FALSE);
      i_sg != AVD_SG_NULL;
      i_sg = avd_sg_struc_find_next(cb,lsg_name_net,FALSE))
   {
      lsg_name_net = i_sg->name_net;
      
      if ((i_sg->row_status != NCS_ROW_ACTIVE) ||
         (i_sg->list_of_su == AVD_SU_NULL) ||
         (i_sg->sg_ncs_spec == SA_TRUE))
      {
         continue;
      }
      
      switch(i_sg->su_redundancy_model)
      {
      case AVSV_SG_RED_MODL_2N:
         avd_sg_2n_realign_func(cb,i_sg);
         break;

      case AVSV_SG_RED_MODL_NWAY:
         avd_sg_nway_realign_func(cb,i_sg);
         break;

      case AVSV_SG_RED_MODL_NWAYACTV:
         avd_sg_nacvred_realign_func(cb,i_sg);
         break;
      
      case AVSV_SG_RED_MODL_NPM:
         avd_sg_npm_realign_func(cb,i_sg);
         break;

      case AVSV_SG_RED_MODL_NORED:
      default:
         avd_sg_nored_realign_func(cb,i_sg);
         break;
      }
      
   }

   return;
}



/*****************************************************************************
 * Function: avd_destroy
 *
 * Purpose: This is the routine that does the destruction of the AVD
 * module. This functionality is yet to be determined.
 * 
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static uns32 avd_destroy(void)
{

   m_AVD_LOG_SEAPI_DSTR_INFO(AVSV_LOG_SEAPI_SUCCESS);
   return NCSCC_RC_SUCCESS;

}



/*****************************************************************************
 * Function: avd_lib_req
 *
 * Purpose: This is the routine that is exposed to the outside world
 * for both initialization and destruction of the AVD module. This
 * will inturn call the initialisation and destroy routines of AVD.
 * 
 *
 * Input: req_info - pointer to the DL SE API request structure.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 avd_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   m_AVD_LOG_FUNC_ENTRY("avd_lib_req");
   if (req_info == NULL)
   {
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return NCSCC_RC_FAILURE;
   }

   if (req_info->i_op == NCS_LIB_REQ_CREATE)
   {
      return avd_initialize(req_info);
   }else if (req_info->i_op == NCS_LIB_REQ_DESTROY)
   {
      return avd_destroy();
   }

   m_AVD_LOG_INVALID_VAL_ERROR(req_info->i_op);

   return NCSCC_RC_FAILURE;
   
}


/*****************************************************************************
 * Function: avd_hb_task_create 
 *
 * Purpose: This routine will create another thread which has another mailbox
 *          and will process only AVD-AVD & AVD-AVND heart beat msg.
 *
 * Returns: None. 
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static void avd_hb_task_create()
{


   /* create and start the AvD thread with the cb handle argument */

   if (m_NCS_TASK_CREATE ((NCS_OS_CB)avd_hb_init_proc,
                    &g_avd_hdl,
                    NCS_AVD_HB_NAME_STR,
                    NCS_AVD_HB_PRIORITY,
                    NCS_AVD_HB_STCK_SIZE,
                    &g_avd_hb_task_hdl) != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_SEAPI_INIT_INFO(AVSV_LOG_SEAPI_FAILURE);
      m_AVD_LOG_INVALID_VAL_FATAL(0);
      return;
   }


   if (m_NCS_TASK_START (g_avd_hb_task_hdl) != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_SEAPI_INIT_INFO(AVSV_LOG_SEAPI_FAILURE);
      m_AVD_LOG_INVALID_VAL_FATAL(0);
      return;
   }


return;
}

/*****************************************************************************
 * Function: avd_hb_init_proc 
 *
 * Purpose: This is the infinite loop (for hb_thread) in which both the active
 * and standby AvDs execute waiting for heartbeat events to happen. When woken
 * up due to an event, based on the HA state it moves to either the active
 * or standby processing modules. Even in Init state the same arrays are used.
 *
 * Input: cb - AVD control block
 *
 * Returns: NONE.
 *
 * NOTES: This function will never return execept in case of init errors.
 *
 * 
 **************************************************************************/

void avd_hb_init_proc(uns32 *avd_hdl_ptr)
{
   NCS_SEL_OBJ_SET sel_obj_set;
   NCS_SEL_OBJ     sel_high,mbx_sel_obj;
   AVD_CL_CB       *cb;
   AVD_EVT         *evt;
   uns32           msg;
   
   m_AVD_LOG_FUNC_ENTRY("avd_hb_init_proc");

   /* check if the value of g_avd_hdl matches the avd_hdl passed  */
   if (avd_hdl_ptr == NULL)
   {
      m_AVD_LOG_INVALID_VAL_FATAL(0);
      return;
   }

   if (*avd_hdl_ptr != g_avd_hdl)
   {
      m_AVD_LOG_INVALID_VAL_FATAL(*avd_hdl_ptr);
      return;
   }

   /* get the CB from the handle manager */
   if ((cb = ncshm_take_hdl(NCS_SERVICE_ID_AVD,g_avd_hdl)) == AVD_CL_CB_NULL)
   {
      /* log the problem */
      m_AVD_LOG_INVALID_VAL_FATAL(g_avd_hdl);
      return;
   }
      

   USE(msg);
   mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(&cb->avd_hb_mbx);
   sel_high = cb->hb_sel_high;
   sel_obj_set = cb->hb_sel_obj_set;

 
   /* give back the handle */
   ncshm_give_hdl(g_avd_hdl);

   /* start of the infinite loop */

   /* Do a wait(select) for one of the different modules to send
    * a message to work
    */
   while (m_NCS_SEL_OBJ_SELECT(sel_high, &sel_obj_set, NULL, NULL, NULL) != -1 )
   {
      /* get the CB from the handle manager */
      if ((cb = ncshm_take_hdl(NCS_SERVICE_ID_AVD,g_avd_hdl)) ==
        AVD_CL_CB_NULL)
      {
         /* log the problem */
         m_AVD_LOG_INVALID_VAL_FATAL(g_avd_hdl);
         continue;
      }
      
      m_AVD_LOG_RCVD_VAL((long)cb);

      if (m_NCS_SEL_OBJ_ISSET (mbx_sel_obj, &sel_obj_set))
      {
         evt = (AVD_EVT *)m_NCS_IPC_NON_BLK_RECEIVE(&cb->avd_hb_mbx,msg);

         if ( (evt != AVD_EVT_NULL) && (evt->rcv_evt > AVD_EVT_INVALID) && (evt->rcv_evt < AVD_EVT_MAX)
               && (evt->cb_hdl == cb->cb_handle))
         {
            /* We will get only timer expiry and MDS msg  
             */
            if ((evt->rcv_evt == AVD_EVT_TMR_SND_HB) ||
                (evt->rcv_evt == AVD_EVT_HEARTBEAT_MSG) ||
                (evt->rcv_evt == AVD_EVT_D_HB)) 
            {
               /* Process the event */
               avd_process_hb_event(cb, evt);
            }
            else
               m_AVD_LOG_INVALID_VAL_FATAL(evt->rcv_evt);
            
         }

      } /* if (m_NCS_SEL_OBJ_ISSET (mbx_sel_obj, &sel_obj_set)) */


      /* reinitialize the selection object list and the highest
       * selection object value from the CB
       */
      sel_high = cb->hb_sel_high;
      sel_obj_set = cb->hb_sel_obj_set;

      /* give back the handle */
      ncshm_give_hdl(g_avd_hdl);

   } /* end of the infinite loop */

  m_AVD_LOG_INVALID_VAL_FATAL(0);
  m_NCS_SYSLOG(NCS_LOG_CRIT,"NCS_AvSv: Avd-HB Thread Failed");

} /* avd_main_proc */




