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
 
  DESCRIPTION:This module crates an AvM thread and registers AvM with MDS, EDSv
  MAB services.It alsoc creats AvM Control Block and intializes it.
  ..............................................................................

  Function Included in this Module:

  avm_lib_req       -
  avm_init          -
  avm_init_proc     -
  avm_init_db       -
 
******************************************************************************
*/


/*
 * Module Inclusion Control...
 */

#include "avm.h"

/* the global handle holder that is used for verfying multiple initialisations
 * of AvM.
 */

uns32 g_avm_hdl = 0;
void *g_avm_task_hdl = NULL;
uns32 count = 0;

static uns32 
avm_init(NCS_LIB_REQ_INFO *req_info);

static void 
avm_init_proc(uns32 *avm_init_hdl);

static uns32 
avm_destroy( AVM_DESTROY_T destroy);

static uns32
avm_hpl_init();

static uns32
avm_hpl_destroy();

/**************************************************************************
* Function: avm_hpl_init
*
* Purpose: Establishes HPL session with HISv.
*
* Input: 
*
* Returns: None.
*
* NOTES:
*
**************************************************************************/
static uns32
avm_hpl_init()
{
   NCS_LIB_REQ_INFO req_info;
 
   m_NCS_MEMSET(&req_info, '\0', sizeof(req_info));
   req_info.i_op = NCS_LIB_REQ_CREATE;
 
   /* request to initialize HPL library */
   return ncs_hpl_lib_req(&req_info);
}
/**************************************************************************
* Function: avm_hpl_destroy
*
* Purpose: Disconnects HPL session with HISv.
*
* Input: 
*
* Returns: None.
*
* NOTES:
*
**************************************************************************/
static uns32
avm_hpl_destroy()
{
   NCS_LIB_REQ_INFO req_info;
   uns32 rc;
 
   m_NCS_MEMSET(&req_info, '\0', sizeof(req_info));
   req_info.i_op = NCS_LIB_REQ_DESTROY;
 
   /* request to initialize HPL library */
   rc = ncs_hpl_lib_req(&req_info);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_HPL(AVM_LOG_HPL_DESTROY, AVM_LOG_HPL_FAILURE, NCSFL_SEV_CRITICAL);
   }
   m_AVM_LOG_HPL(AVM_LOG_HPL_DESTROY, AVM_LOG_HPL_SUCCESS, NCSFL_SEV_INFO);

   return rc;
}
/***********************************************************************
 ******
 * Name          : avm_init_db
 *
 * Description   : This function is initializes the Patricia Trees in AvM
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 a Reset Req has been issued from AvD.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
 ************************************************************************
 *****/
static uns32
avm_init_db()
{
   AVM_CB_T             *cb;
   NCS_PATRICIA_PARAMS  patricia_params;

   m_AVM_LOG_FUNC_ENTRY("avm_init_db");

   if (AVM_CB_NULL == (cb = ncshm_take_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl)))
   {    
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   patricia_params.key_size = AVM_MAX_INDEX_LEN;
   if( ncs_patricia_tree_init(&cb->db.valid_info_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_PATRICIA(AVM_LOG_PAT_INIT, AVM_LOG_PAT_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
   m_AVM_LOG_PATRICIA(AVM_LOG_PAT_INIT, AVM_LOG_PAT_SUCCESS, NCSFL_SEV_INFO);

   patricia_params.key_size = sizeof(SaHpiEntityPathT);
   if( ncs_patricia_tree_init(&cb->db.ent_info_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->db.valid_info_anchor);
      m_AVM_LOG_PATRICIA(AVM_LOG_PAT_INIT, AVM_LOG_PAT_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
   m_AVM_LOG_PATRICIA(AVM_LOG_PAT_INIT, AVM_LOG_PAT_SUCCESS, NCSFL_SEV_INFO);

   patricia_params.key_size = sizeof(AVM_ENT_PATH_STR_T);
   if( ncs_patricia_tree_init(&cb->db.ent_info_str_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->db.valid_info_anchor);
      ncs_patricia_tree_destroy(&cb->db.ent_info_anchor);
      m_AVM_LOG_PATRICIA(AVM_LOG_PAT_INIT, AVM_LOG_PAT_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
   m_AVM_LOG_PATRICIA(AVM_LOG_PAT_INIT, AVM_LOG_PAT_SUCCESS, NCSFL_SEV_INFO);

   patricia_params.key_size = SA_MAX_NAME_LENGTH;
   if( ncs_patricia_tree_init(&cb->db.node_name_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&cb->db.valid_info_anchor);
      ncs_patricia_tree_destroy(&cb->db.ent_info_anchor);
      ncs_patricia_tree_destroy(&cb->db.ent_info_str_anchor);
      m_AVM_LOG_PATRICIA(AVM_LOG_PAT_INIT, AVM_LOG_PAT_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
   m_AVM_LOG_PATRICIA(AVM_LOG_PAT_INIT, AVM_LOG_PAT_SUCCESS, NCSFL_SEV_INFO);

   return NCSCC_RC_SUCCESS;
 
}

/*****************************************************************************
 * Function: avm_init_proc
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

static void 
avm_init_proc(uns32 *avm_init_hdl)
{
   AVM_CB_T             *cb = AVM_CB_NULL;
   NCS_SEL_OBJ          mbx_sel_obj;
   NCS_SEL_OBJ          mbc_sel_obj;

   uns8                 script_buf[AVM_SCRIPT_BUF_LEN];
   uns8                 dhcp_initialise = 2;

   uns32                rc;

   m_AVM_LOG_FUNC_ENTRY("avm_init_proc");
   
   if (NULL == avm_init_hdl)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(0); 
      return ;
   }

   if (*avm_init_hdl != g_avm_hdl)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(*avm_init_hdl);
      return ;
   }

   /* get the AvM Control Block from the handle manager */
   if (AVM_CB_NULL == (cb = ncshm_take_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl)))
   {    
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return ;
   }
  
   cb->dummy_parent_cnt  = 0;
   cb->valid_info_state  = AVM_VALID_INFO_NOT_DONE;
   cb->config_state      = AVM_CONFIG_NOT_DONE;
   cb->config_cnt        = 0;  
   cb->eda_init           = AVM_EDA_NOT_DONE;
   cb->eda_init_tmr.tmr_id = TMR_T_NULL;
   cb->ssu_tmr.tmr_id = TMR_T_NULL;
   cb->ssu_tmr.status = AVM_TMR_NOT_STARTED;

   /* Get the mailbox selection object */
   mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(&cb->mailbox);

   /* reset the select objects */
   m_NCS_SEL_OBJ_ZERO(&cb->sel_obj_set);

   /* set the mailbox sel obj in cb selection object set */
   m_NCS_SEL_OBJ_SET(mbx_sel_obj, &cb->sel_obj_set);

   cb->sel_high    = mbx_sel_obj;
   cb->mbx_sel_obj = mbx_sel_obj;

   rc = avm_init_db(cb, g_avm_hdl);   
   if(NCSCC_RC_SUCCESS != rc)
   {
      avm_destroy(AVM_DESTROY_CB);
      ncshm_give_hdl(g_avm_hdl);
      g_avm_hdl = 0;
      return;
   }

   /* Create EDU handle */
   
   m_NCS_EDU_HDL_INIT(&cb->edu_hdl);

   if(NCSCC_RC_SUCCESS != avm_rda_initialize(cb))
   {
      m_AVM_LOG_RDE(AVM_LOG_RDE_REG, AVM_LOG_RDE_FAILURE, NCSFL_SEV_CRITICAL);
      ncshm_give_hdl(g_avm_hdl);
      g_avm_hdl = 0;
      return ;
   }
   
   /* Initialise /etc/dhcpd.conf */
   if(cb->ha_state == SA_AMF_HA_ACTIVE)
   {
      /* Initialise the configuration file. except dhcp_initialise, other parameters are dummy  */
      sprintf(script_buf, "%s %s %s %d %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x %s %s %d",
              AVM_SSU_DHCONF_SCRIPT, "host1" , "host2", 1, 
              1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
              "filename", "tftpserver", dhcp_initialise);

      /* Invoke script to initialise the DHCP Configuration file */
      if (m_NCS_SYSTEM(script_buf))
      {
          /* Fix for IR00084625 */
          /* ncshm_give_hdl(g_avm_hdl);
             g_avm_hdl = 0;
             return;
          */  
          m_AVM_LOG_DEBUG("AVM-SSU: Failed to execute AVM_SSU_DHCONF_SCRIPT to initialise DHCP conf file", NCSFL_SEV_ERROR); 
      }
   }
   if(cb->ha_state == SA_AMF_HA_STANDBY)
   {
      /*ssu_tmr is not needed if initially coming up as standby */
      cb->ssu_tmr.status = AVM_TMR_EXPIRED;
   }

   /* Initialize with FMA */
   if(NCSCC_RC_SUCCESS != avm_fma_initialize(cb))
   {
      m_AVM_LOG_FM_INFO(AVM_LOG_FMA_INIT_FAILED, NCSFL_SEV_CRITICAL, NCSFL_LC_FUNC_RET_FAIL);
      ncshm_give_hdl(g_avm_hdl);
      g_avm_hdl = 0;
      return ;
   }

   /* Initialise MDS */
   if (NCSCC_RC_SUCCESS != avm_mds_initialize(cb))
   {
      m_AVM_LOG_MDS(AVM_LOG_MDS_REG, AVM_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
      avm_destroy(AVM_DESTROY_PATRICIA);
      ncshm_give_hdl(g_avm_hdl);
      g_avm_hdl = 0;
      return;
   }
   m_AVM_LOG_MDS(AVM_LOG_MDS_REG, AVM_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);

   if(NCSCC_RC_FAILURE == avm_mbc_register(cb))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return;
   }

   m_SET_FD_IN_SEL_OBJ((uns32)cb->mbc_sel_obj, mbc_sel_obj);
   m_NCS_SEL_OBJ_SET(mbc_sel_obj, &cb->sel_obj_set);
   cb->sel_high = m_GET_HIGHER_SEL_OBJ(mbc_sel_obj, cb->sel_high);

   /* Initialize MIBLIB routines */
   if (NCSCC_RC_SUCCESS != avm_miblib_init(cb))
   {
      m_AVM_LOG_MAS(AVM_LOG_MIBLIB_REGISTER, AVM_LOG_MAS_FAILURE, NCSFL_SEV_CRITICAL);
      avm_destroy(AVM_DESTROY_PATRICIA);
      ncshm_give_hdl(g_avm_hdl);
      g_avm_hdl = 0;
      return;
   }
   m_AVM_LOG_MAS(AVM_LOG_MIBLIB_REGISTER, AVM_LOG_MAS_FAILURE, NCSFL_SEV_INFO);

   /* Initialise MAB */
   if (avm_mab_init(cb) != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_MAS(AVM_LOG_OAC_REGISTER, AVM_LOG_MAS_FAILURE, NCSFL_SEV_CRITICAL);
      avm_destroy(AVM_DESTROY_MDS);
      ncshm_give_hdl(g_avm_hdl);
      g_avm_hdl = 0;
      return;
   }

   /*Initialise EDS */
   rc = avm_eda_initialize(cb);
   if(AVM_EDA_FAILURE == rc)
   {
     m_AVM_INIT_EDA_TMR_START(cb);
   }else     
   {
     if(NCSCC_RC_FAILURE == rc)
     {
        m_AVM_LOG_EDA(AVM_LOG_EDA_CREATE, AVM_LOG_EDA_FAILURE, NCSFL_SEV_CRITICAL);
        avm_destroy(AVM_DESTROY_MAB); 
     }else
     {
        cb->eda_init  = AVM_EDA_DONE;
        m_AVM_LOG_EDA(AVM_LOG_EDA_CREATE, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);
     }
   }
   rc = avm_hpl_init();
   if(NCSCC_RC_FAILURE == rc)
   {
      m_AVM_LOG_HPL(AVM_LOG_HPL_INIT, AVM_LOG_HPL_FAILURE, NCSFL_SEV_CRITICAL);
   }else
   {
      m_AVM_LOG_HPL(AVM_LOG_HPL_INIT, AVM_LOG_HPL_SUCCESS, NCSFL_SEV_INFO);
   }
   cb->cfg_state = AVM_CONFIG_DONE;

   /* give back the handle */
   ncshm_give_hdl(g_avm_hdl);

   /* call the main processing module that will not return*/
   avm_proc();

}


/*****************************************************************************
 * Function: avm_init
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

static uns32 
avm_init(NCS_LIB_REQ_INFO *req_info)
{
   AVM_CB_T    *cb;

   m_AVM_LOG_FUNC_ENTRY("avm_init");
   
   /* check if g_avm_hdl is 0, if not AvM is
    * already initialised return now */
   if (0 != g_avm_hdl )
   {
      m_AVM_LOG_SEAPI(AVM_LOG_SEAPI_CREATE, AVM_LOG_SEAPI_FAILURE, NCSFL_SEV_DEBUG);
      return NCSCC_RC_FAILURE;
   }

#if (NCS_AVM_LOG == 1)
   /* 
    * Register with Logging subsystem. This is an agent call and
    * could succeed even if the DTS server is not available 
    */
   avm_flx_log_reg();
#endif /* (NCS_AVM_LOG == 1) */

   /* Create Control Block and fill g_avm_hdl with
    * the handle returned by handle manager */
   cb = m_MMGR_ALLOC_AVM_CB;
   if (AVM_CB_NULL == cb)
   {
      m_AVM_LOG_CB(AVM_LOG_CB_CREATE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_CB(AVM_LOG_CB_CREATE, AVM_LOG_CB_SUCCESS,  NCSFL_SEV_INFO);

   m_NCS_MEMSET(cb, '\0', sizeof(AVM_CB_T));
   if ((g_avm_hdl = ncshm_create_hdl(NCS_HM_POOL_ID_COMMON, NCS_SERVICE_ID_AVM, cb)) == 0)
   {
        m_MMGR_FREE_AVM_CB(cb);
        m_AVM_LOG_CB(AVM_LOG_CB_HDL_ASS_CREATE, AVM_LOG_SEAPI_FAILURE, NCSFL_SEV_CRITICAL);
        return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_CB(AVM_LOG_CB_HDL_ASS_CREATE, AVM_LOG_SEAPI_SUCCESS, NCSFL_SEV_INFO);

   cb->cb_hdl = g_avm_hdl;

   /* Create a MailBox */
   if (m_NCS_IPC_CREATE (&cb->mailbox) != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_MBX(AVM_LOG_MBX_CREATE, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl);
      g_avm_hdl = 0;
      m_MMGR_FREE_AVM_CB(cb);
      return NCSCC_RC_FAILURE;
   }
   m_AVM_LOG_MBX(AVM_LOG_MBX_CREATE, AVM_LOG_MBX_SUCCESS, NCSFL_SEV_INFO);

   /* Create a SSU MailBox */
   if (m_NCS_IPC_CREATE (&cb->ssu_mbx) != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_MBX(AVM_LOG_MBX_CREATE, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl);
      g_avm_hdl = 0;
      m_MMGR_FREE_AVM_CB(cb);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_IPC_ATTACH(&cb->mailbox);

   m_AVM_LOG_MBX(AVM_LOG_MBX_ATTACH, AVM_LOG_MBX_SUCCESS, NCSFL_SEV_INFO);

   m_NCS_IPC_ATTACH(&cb->ssu_mbx);

   /* create and start the AvM thread with the cb handle argument */
   if (m_NCS_TASK_CREATE ((NCS_OS_CB)avm_init_proc,
                    &g_avm_hdl,
                    NCS_AVM_NAME,
                    NCS_AVM_PRIORITY,
                    NCS_AVM_STACK_SIZE,
                    &g_avm_task_hdl) != NCSCC_RC_SUCCESS)
   {
      m_NCS_IPC_RELEASE(&cb->mailbox, NULL);
      m_NCS_IPC_RELEASE(&cb->ssu_mbx, NULL);
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl);
      g_avm_hdl = 0;
      m_MMGR_FREE_AVM_CB(cb);
      m_AVM_LOG_TASK(AVM_LOG_TASK_CREATE, AVM_LOG_TASK_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_TASK(AVM_LOG_TASK_CREATE, AVM_LOG_TASK_SUCCESS, NCSFL_SEV_INFO);

   if (m_NCS_TASK_START (g_avm_task_hdl) != NCSCC_RC_SUCCESS)
   {
      m_NCS_TASK_RELEASE(g_avm_task_hdl);
      m_NCS_IPC_RELEASE(&cb->mailbox, NULL);
      m_NCS_IPC_RELEASE(&cb->ssu_mbx, NULL);
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl);
      g_avm_hdl = 0;
      m_MMGR_FREE_AVM_CB(cb);
      m_AVM_LOG_TASK(AVM_LOG_TASK_START, AVM_LOG_TASK_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   m_AVM_LOG_TASK(AVM_LOG_TASK_START, AVM_LOG_TASK_SUCCESS, NCSFL_SEV_INFO);
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avm_destroy
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

static uns32 
avm_destroy(AVM_DESTROY_T     destroy)
{
   AVM_CB_T  *cb;

   if(0 == g_avm_hdl)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(0);
      return NCSCC_RC_FAILURE;
   }

   if (AVM_CB_NULL == (cb = ncshm_take_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl)))
   {    
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   switch(destroy)
   {
      case AVM_DESTROY_HPL:
      {
         avm_hpl_destroy();   
      }
      case AVM_DESTROY_EDA:
      {
         avm_eda_finalize(cb);
      }
      case AVM_DESTROY_MAB:
      {
         avm_mab_destroy(cb);
      }
      case AVM_DESTROY_MDS:
      {
         avm_mds_finalize(cb);
      }
      case AVM_DESTROY_PATRICIA:
      {
         ncs_patricia_tree_destroy(&cb->db.ent_info_anchor);
         ncs_patricia_tree_destroy(&cb->db.ent_info_str_anchor);
         ncs_patricia_tree_destroy(&cb->db.node_name_anchor);
      }
      case AVM_DESTROY_TASK:
      {
      }
      case AVM_DESTROY_MBX:
      {
      }
      case AVM_DESTROY_CB:
      {
      }
      default:
      {
         m_AVM_LOG_INVALID_VAL_ERROR(destroy);
      }
   }
   
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: avm_lib_req
 *
 * Purpose: This is the routine that is exposed to the outside world
 * for both initialization and destruction of the AVM module.
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
uns32 avm_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   
   uns32 rc;   
  
   m_AVM_LOG_FUNC_ENTRY("avm_lib_req");
 
   if (NULL == req_info)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(0);
      return NCSCC_RC_FAILURE;
   }
   
   switch(req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
      {
         rc =  avm_init(req_info);

         if(NCSCC_RC_SUCCESS == rc)
         {
            m_AVM_LOG_SEAPI(AVM_LOG_SEAPI_CREATE, AVM_LOG_SEAPI_SUCCESS, NCSFL_SEV_INFO);
         }else
         {
            m_AVM_LOG_SEAPI(AVM_LOG_SEAPI_CREATE, AVM_LOG_SEAPI_FAILURE, NCSFL_SEV_CRITICAL);
         }
      }
      break;

      case NCS_LIB_REQ_DESTROY:
      {
         rc = avm_destroy(AVM_DESTROY_HPL);

         if(NCSCC_RC_SUCCESS == rc)
         {
            m_AVM_LOG_SEAPI(AVM_LOG_SEAPI_DESTROY, AVM_LOG_SEAPI_SUCCESS, NCSFL_SEV_INFO);
         }else
         {
            m_AVM_LOG_SEAPI(AVM_LOG_SEAPI_DESTROY, AVM_LOG_SEAPI_FAILURE, NCSFL_SEV_CRITICAL);
         }
      }
      break;

      default:
      {
         m_AVM_LOG_INVALID_VAL_FATAL(req_info->i_op);
         rc = NCSCC_RC_FAILURE;
      }
    }
    return rc;      
}
