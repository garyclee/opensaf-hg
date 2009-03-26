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

  This file captures the initialization of BOM Access Manager.
  
******************************************************************************
*/

#include <configmake.h>

/* NCS specific include files */

#include <gl_defs.h>
#include <t_suite.h>
#include <ncs_mib_pub.h>
#include <ncs_lib.h>
#include <ncsgl_defs.h>
#include <mds_papi.h>
#include <mds_adm.h>
#include <ncs_mda_pvt.h>

#include "bam.h"
#include "bamParser.h"
#include "bam_log.h"

uns32 gl_ncs_bam_hdl;
void *gl_ncs_bam_task_hdl = NULL;
uns32 gl_comp_type;
uns32 gl_bam_avd_cfg_msg_num;
uns32 gl_bam_avm_cfg_msg_num;

/****************************************************************************
 * Name          : ncs_bam_search_argv_list
 *
 * Description   : To search the list for arguments for the interested argument
 *
 * Arguments     : argc  - This is the Number of arguments received.
 *                 argv  - string received.
 *                 arg_prefix  - the argument name for which the search is ON
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
static char *
ncs_bam_search_argv_list(int argc, char *argv[], char *arg_prefix)
{
   char *tmp;

   /* Check   argv[argc-1] through argv[1] */
   for ( ; argc > 1; argc -- )
   {
      tmp = strstr(argv[argc-1], arg_prefix);
      if (tmp != NULL)
         return tmp;
   }
   return NULL;
}

/****************************************************************************
 * Name          : ncs_bam_extract_ncs_filename
 *
 * Description   : To extract the XML filename from the argument list
 *
 * Arguments     : argc  - This is the Number of arguments received.
 *                 argv  - string received.
 *                 filename - Name to be filled for parsing the file.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ncs_bam_extract_ncs_filename(int argc, char *argv[], char *filename)
{
   char                 *p_field;

   p_field = ncs_bam_search_argv_list(argc, argv, "NCS_XML_FILE=");
   if (p_field == NULL)
   {      
      m_NCS_DBG_PRINTF("\nBAM: Using NCS_XML_FILE from standard path \n");
      return NCSCC_RC_FAILURE;
   }
   if (sscanf(p_field + strlen("NCS_XML_FILE="), "%s", filename) != 1)
   {      
      m_NCS_DBG_PRINTF("\nBAM: Error in NCS_XML_FILE file argument\n");
      return NCSCC_RC_FAILURE;
   }
   else
      m_NCS_DBG_PRINTF("\nBAM: NCS_XML_FILE = %s\n", filename);

   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ncs_bam_extract_hw_filename
 *
 * Description   : To extract the XML filename for HARDWARE configuration
 *                 from the argument list
 *
 * Arguments     : argc  - This is the Number of arguments received.
 *                 argv  - string received.
 *                 filename - Name to be filled for parsing the file.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ncs_bam_extract_hw_filename(int argc, char *argv[], char *filename)
{
   char                 *p_field;

   p_field = ncs_bam_search_argv_list(argc, argv, "HW_VLD_FILE=");
   if (p_field == NULL)
   {      
      m_NCS_DBG_PRINTF("\nBAM: using HW_VLD_FILE from standard path");

      return NCSCC_RC_FAILURE;  

   }
   if (sscanf(p_field + strlen("HW_VLD_FILE="), "%s", filename) != 1)
   {      
      m_NCS_DBG_PRINTF("\nBAM: Error in HW_VLD_FILE file argument\n");

      return NCSCC_RC_FAILURE; 

   }

   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ncs_bam_extract_app_filename
 *
 * Description   : To extract the XML filename for applications configuration
 *                 from the argument list
 *
 * Arguments     : argc  - This is the Number of arguments received.
 *                 argv  - string received.
 *                 filename - Name to be filled for parsing the file.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ncs_bam_extract_app_filename(int argc, char *argv[], char *filename)
{
   char                 *p_field;

   p_field = ncs_bam_search_argv_list(argc, argv, "APP_XML_FILE=");
   if (p_field == NULL)
   {      
      m_NCS_DBG_PRINTF("\nBAM: Using APP_XML_FILE from standard path  \n");
      return NCSCC_RC_FAILURE;
   }
   if (sscanf(p_field + strlen("APP_XML_FILE="), "%s", filename) != 1)
   {      
      m_NCS_DBG_PRINTF("\nBAM: Error in APP_XML_FILE file argument\n");
      return NCSCC_RC_FAILURE;
   }
   else
      m_NCS_DBG_PRINTF("BAM: APP_XML_FILE = %s\n", filename);

   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ncs_bam_process_message
 *
 * Description   : To process the PSR playback message or AVM mibsets
 *
 * Arguments     : evt  - This is the BAM_EVT indicating the evt posted in mbx.
 *
 * Return Values : None
 *
 * Notes         : parse the message and look into it in this function, 
 *                 Ideally it will be great to add a routine if message from PSR 
 *                 and push the routine to bam_psr.c file.
 *****************************************************************************/
static void
ncs_bam_process_message(BAM_EVT *evt)
{
   NCS_BAM_CB   *bam_cb = NULL;
   PSS_BAM_MSG  *psr_msg = NULL;
   PSS_BAM_WARMBOOT_REQ   *warmboot_req = NULL;
   uns32       rc=NCSCC_RC_SUCCESS;
   NCS_OS_FILE file_info;

   if((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      if(evt->msg.pss_msg)
      {
         m_MMGR_FREE_PSS_BAM_MSG(evt->msg.pss_msg);
      }
      return ;
   }

   if(evt->evt_type == NCS_BAM_PSR_PLAYBACK)
   {
      psr_msg = evt->msg.pss_msg;
      if(psr_msg == NULL)
      {
         /*
          * Nothing to be done. log error and Return
          * serious enough to be logged to CONS as parsing 
          *  will be stalled 
          */
         syslog(LOG_ERR,"NCS_AvSv: PSSV asked to playback nothing.");
         m_LOG_BAM_MSG_TIC(BAM_PSS_NAME, NCSFL_SEV_ERROR, "PSSV asked to playback nothing");
         return ;
      }
      if(psr_msg->i_evt != PSS_BAM_EVT_WARMBOOT_REQ)
      {
         m_MMGR_FREE_PSS_BAM_MSG(evt->msg.pss_msg);
         return;
      }
      
      warmboot_req = &psr_msg->info.warmboot_req;

      do
      {
         if(warmboot_req->pcn == NULL)
         {
            warmboot_req = warmboot_req->next;
            continue;
         }
         
         if(strncmp(warmboot_req->pcn, "AVD", 3) == 0)
         {
            /* catch the error and return ? or continue ? */
            gl_bam_avd_cfg_msg_num = 0;
     
            m_LOG_BAM_MSG_TII(BAM_START_PARSE, BAM_NCS_CONFIG, NCSFL_SEV_NOTICE ); 

            rc = parse_and_build_DOMDocument(bam_cb->ncs_filename, BAM_PARSE_NCS);
            if(rc != NCSCC_RC_SUCCESS)
            {
               m_LOG_BAM_MSG_TII(BAM_NCS_CONFIG, BAM_PARSE_ERROR, NCSFL_SEV_ERROR ); 
            }
            else
            {
               m_LOG_BAM_MSG_TII(BAM_NCS_CONFIG, BAM_PARSE_SUCCESS, NCSFL_SEV_NOTICE ); 
            }
            
            memset(&file_info,'\0',sizeof(NCS_OS_FILE)); 
            file_info.info.open.i_file_name = (uns8 *)bam_cb->app_filename;
            file_info.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;

            if( m_NCS_OS_FILE(&file_info, NCS_OS_FILE_OPEN) == NCSCC_RC_SUCCESS)
            {
               file_info.info.close.i_file_handle = file_info.info.open.o_file_handle;
               m_NCS_OS_FILE(&file_info, NCS_OS_FILE_CLOSE);

               m_LOG_BAM_MSG_TII(BAM_START_PARSE, BAM_APP_CONFIG, NCSFL_SEV_NOTICE);
 
               /* proceed to parse the app */
               if(parse_and_build_DOMDocument(bam_cb->app_filename, BAM_PARSE_APP) != NCSCC_RC_SUCCESS)
               {
                  m_LOG_BAM_MSG_TII(BAM_APP_CONFIG, BAM_PARSE_ERROR, NCSFL_SEV_ERROR ); 
               }
               else
               {
                  m_LOG_BAM_MSG_TII(BAM_APP_CONFIG, BAM_PARSE_SUCCESS, NCSFL_SEV_NOTICE);
               }
            }
            else
            {

               m_LOG_BAM_MSG_TII(BAM_APP_FILE, BAM_APP_NOT_PRES, NCSFL_SEV_NOTICE);
            }

            /* Send config done only if successfully parsed */
            if(rc == NCSCC_RC_SUCCESS)
            {
               ncs_bam_send_cfg_done_msg();
               pss_bam_send_cfg_done_msg(BAM_PARSE_NCS);
               m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_NOTICE, "Config Done sent for s/w(amf) configuration");
            }
         }
         else if(strncmp(warmboot_req->pcn, "AVM", 3) == 0)
         {
            /* catch the error and return ? or continue ? */
            gl_bam_avm_cfg_msg_num = 0;

            m_LOG_BAM_MSG_TII(BAM_START_PARSE, BAM_HW_DEP_CONFIG, NCSFL_SEV_NOTICE);

            rc = parse_and_build_DOMDocument(bam_cb->ncs_filename, BAM_PARSE_AVM);

            if(rc != NCSCC_RC_SUCCESS)
            {
               m_LOG_BAM_MSG_TII(BAM_HW_DEP_CONFIG, BAM_PARSE_ERROR, NCSFL_SEV_ERROR);
            }
            else
            {
               ncs_bam_avm_send_cfg_done_msg();
               pss_bam_send_cfg_done_msg(BAM_PARSE_AVM);
               bam_delete_hw_ent_list();
               m_LOG_BAM_MSG_TII(BAM_HW_DEP_CONFIG, BAM_PARSE_SUCCESS, NCSFL_SEV_NOTICE);
            }
         }
         warmboot_req = warmboot_req->next;
      }while(warmboot_req != NULL);

      m_MMGR_FREE_PSS_BAM_MSG(evt->msg.pss_msg);
   }else if(evt->evt_type == NCS_BAM_AVM_UP)
   {
      bam_parse_hardware_config();
   }

   ncshm_give_hdl(gl_ncs_bam_hdl);   
   return ;
}


/*****************************************************************************
 * Function: ncs_bam_main_loop
 *
 * Purpose:  Main message processing loop for BAM
 *
 * Input:    Pointer to BAM Mail Box.
 *
 * Returns:  None
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static void 
ncs_bam_main_loop(SYSF_MBX *bam_mbx)
{
  BAM_EVT *evt;
  while ((evt = (BAM_EVT *)m_NCS_BAM_MSG_RCV(bam_mbx, msg)) != NULL)
  {
    ncs_bam_process_message(evt) ;
    m_MMGR_FREE_BAM_DEFAULT_VAL(evt);
  }
  m_LOG_BAM_MSG_TIC(BAM_IPC_RECV_FAIL,NCSFL_SEV_CRITICAL, 
         "Bam Msg recv failed");
  syslog(LOG_CRIT,"NCS_AvSv: Bam Msg ipc Rcv Failure");
  return ;
}

/*****************************************************************************
 * Function: ncs_bam_init_proc
 *
 * Purpose:  This function is the entry point for BAM thread. This function 
 *           will initialise BAM's CB. Once initialisation is done the function 
 *           will call a message processing routine for BAM thread.
 *
 * Input:    Pointer to control Block handle
 *
 * Returns:  Returns in case of failure/ Never otherwise.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static void 
ncs_bam_init_proc(uns32 *bam_hdl) 
{
   NCS_BAM_CB *bam_cb = NULL ;
   NCS_PATRICIA_PARAMS  patricia_params;
   uns32       rc=0;

   /* Sanitise the value of the global pointer  */
   if (bam_hdl == NULL)
      return;

   if (*bam_hdl != gl_ncs_bam_hdl)
      return;

   /* Retrieve the CB from the handle manager */
   if ((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM,*bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return;
   }

   /* verify that the mailbox is set */
   if (bam_cb->bam_mbx == NULL)
   {
      ncshm_give_hdl(*bam_hdl);
      return;
   }

   if (NCSCC_RC_FAILURE == m_BAM_CB_LOCK_INIT(bam_cb))
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_LOCK_FAILED, NCSFL_SEV_ERROR);
      ncshm_give_hdl(*bam_hdl);
      return;
   }

   if((rc = ncs_bam_rda_reg(bam_cb)) != NCSCC_RC_SUCCESS)
   {
      ncshm_give_hdl(*bam_hdl);
      /* Add log here */
      return ;
   }
   
   if(SA_AMF_HA_ACTIVE == bam_cb->ha_state)
   {
      if ((rc = ncs_bam_mds_reg(bam_cb)) != NCSCC_RC_SUCCESS) 
      {
	 ncshm_give_hdl(*bam_hdl);
	 m_LOG_BAM_SVC_PRVDR(BAM_MDS_SUBSCRIBE_FAIL, NCSFL_SEV_ERROR);
	 return ;
      }
      m_LOG_BAM_SVC_PRVDR(BAM_MDS_SUBSCRIBE_SUCCESS, NCSFL_SEV_NOTICE);
   }

   patricia_params.key_size = BAM_MAX_INDEX_LEN * sizeof(char);
   if( ncs_patricia_tree_init(&bam_cb->deploy_ent_anchor, &patricia_params) 
                            != NCSCC_RC_SUCCESS)
   {
      ncshm_give_hdl(*bam_hdl);
      m_LOG_BAM_MSG_TIC(BAM_ENT_PAT_INIT_FAIL,NCSFL_SEV_ERROR, 
          "Error init deployment Tree .");
      return;
   }

   /* give back the handle */
   ncshm_give_hdl(*bam_hdl);

   /* Initiate call to main message processing routine*/
   
   ncs_bam_main_loop(bam_cb->bam_mbx);

   return;
}

/*****************************************************************************
 * Function: ncs_bam_initialize
 *
 * Purpose: This is the routine that does the creation and starting of
 * AvD task/thread.
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

uns32 ncs_bam_initialize(NCS_LIB_REQ_INFO *req_info)
{
   NCS_BAM_CB *cb;
   char filename[BAM_MAX_INDEX_LEN];

#if ( (NCS_DTA == 1) && ( NCS_BAM_LOG == 1) )
   bam_flx_log_reg();   
#endif

   /* check if gl_ncs_bam_hdl is 0, if not BAM is
   ** already initialised return now 
   */
   if (gl_ncs_bam_hdl != 0)
   {
      m_LOG_BAM_API(BAM_SE_API_CREATE_FAILED, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   /* Allocate the  control block and fill gl_ncs_bam_hdl with
    * the handle returned by handle manager */

   cb = m_MMGR_ALLOC_NCS_BAM_CB;

   if (cb == NULL)
   {
      m_LOG_BAM_MEMFAIL(BAM_CB_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   memset(cb,'\0',sizeof(NCS_BAM_CB));

   if ((gl_ncs_bam_hdl = 
      ncshm_create_hdl(NCS_HM_POOL_ID_COMMON, NCS_SERVICE_ID_BAM, cb))
      == 0)
   {
      m_LOG_BAM_HEADLINE(BAM_CREATE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      m_MMGR_FREE_NCS_BAM_CB(cb);
      return NCSCC_RC_FAILURE;
   }

   cb->cb_handle = gl_ncs_bam_hdl;
   
   /* create a mailbox . */

   if ((cb->bam_mbx = (SYSF_MBX *)m_MMGR_ALLOC_BAM_DEFAULT_VAL(sizeof(SYSF_MBX))) == NULL)
   {
      m_LOG_BAM_MEMFAIL(BAM_MBX_ALLOC_FAILED);
      ncshm_destroy_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl);
      gl_ncs_bam_hdl = 0;
      m_MMGR_FREE_NCS_BAM_CB(cb);
      return NCSCC_RC_FAILURE;
   }

   if (m_NCS_IPC_CREATE (cb->bam_mbx) != NCSCC_RC_SUCCESS)
   {
      m_LOG_BAM_HEADLINE(BAM_IPC_TASK_INIT, NCSFL_SEV_ERROR);
      m_MMGR_FREE_BAM_DEFAULT_VAL(cb->bam_mbx);
      ncshm_destroy_hdl(NCS_SERVICE_ID_BAM,gl_ncs_bam_hdl);
      gl_ncs_bam_hdl = 0;
      m_MMGR_FREE_NCS_BAM_CB(cb);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_IPC_ATTACH(cb->bam_mbx);

   /*
   ** Fill in the control block with the path and filename of NCS and APP
   ** XML file.
   ** The actual parsing will be done at the trigger. The filename 
   ** need to be stored from the arguments, hence store it in CB.
   */
   if(ncs_bam_extract_ncs_filename(req_info->info.create.argc, 
            req_info->info.create.argv, filename) == NCSCC_RC_FAILURE)
   {
/*      m_LOG_BAM_HEADLINE(BAM_FILE_ARG_IGNORED, NCSFL_SEV_ERROR); */

      /* set the default */
      strcpy(filename, OSAF_SYSCONFDIR "NCSSystemBOM.xml");
   }

   /* Now set the NCS filename inthe CB */
   strcpy(cb->ncs_filename, filename);

   /* repeat the above for APP XML file */
   if(ncs_bam_extract_app_filename(req_info->info.create.argc, 
            req_info->info.create.argv, filename) == NCSCC_RC_FAILURE)
   {
/*      m_LOG_BAM_HEADLINE(BAM_FILE_ARG_IGNORED, NCSFL_SEV_ERROR); */

      /* set the default */
      strcpy(filename, OSAF_SYSCONFDIR "AppConfig.xml");
   }

   /* Now set the filename inthe CB */
   strcpy(cb->app_filename, filename);

   /*
   ** Fill in the control block with the path and filename of XML file
   ** The actual parsing will be done at the trigger. The filename 
   ** need to be stored from the arguments, hence store it in CB.
   */
   if(ncs_bam_extract_hw_filename(req_info->info.create.argc, 
            req_info->info.create.argv, filename) == NCSCC_RC_FAILURE)
   {
/*      m_LOG_BAM_HEADLINE(BAM_FILE_ARG_IGNORED, NCSFL_SEV_ERROR); */

      /* set the default */
      strcpy(filename, OSAF_SYSCONFDIR "ValidationConfig.xml");
   }

   /* Now set the filename inthe CB */
   strcpy(cb->hw_filename, filename);


   /* create and start the BAM thread with the cb handle argument */

   if (m_NCS_TASK_CREATE ((NCS_OS_CB)ncs_bam_init_proc,
                    &gl_ncs_bam_hdl,
                    NCS_BAM_NAME_STR,
                    NCS_BAM_PRIORITY,
                    NCS_BAM_STCK_SIZE,
                    &gl_ncs_bam_task_hdl) != NCSCC_RC_SUCCESS)
   {
      m_LOG_BAM_HEADLINE(BAM_IPC_TASK_INIT, NCSFL_SEV_ERROR);
      m_NCS_IPC_RELEASE(cb->bam_mbx, NULL);
      m_MMGR_FREE_BAM_DEFAULT_VAL(cb->bam_mbx);
      ncshm_destroy_hdl(NCS_SERVICE_ID_BAM,gl_ncs_bam_hdl);
      gl_ncs_bam_hdl = 0;
      m_MMGR_FREE_NCS_BAM_CB(cb);
      return NCSCC_RC_FAILURE;
   }


   if (m_NCS_TASK_START (gl_ncs_bam_task_hdl) != NCSCC_RC_SUCCESS)
   {
      m_LOG_BAM_HEADLINE(BAM_IPC_TASK_INIT, NCSFL_SEV_ERROR);
      m_NCS_TASK_RELEASE(gl_ncs_bam_task_hdl);
      m_NCS_IPC_RELEASE(cb->bam_mbx, NULL);
      m_MMGR_FREE_BAM_DEFAULT_VAL(cb->bam_mbx);
      ncshm_destroy_hdl(NCS_SERVICE_ID_BAM,gl_ncs_bam_hdl);
      gl_ncs_bam_hdl = 0;
      m_MMGR_FREE_NCS_BAM_CB(cb);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_BAM_API(BAM_SE_API_CREATE_SUCCESS, NCSFL_SEV_INFO);
   return NCSCC_RC_SUCCESS;


}

uns32
ncs_bam_destroy()
{
   return 0;
}
/*****************************************************************************
 * Function: ncs_bam_dl_func
 *
 * Purpose: This is the routine that is exposed to the outside world
 * for both initialization and destruction of the BAM module. This
 * will inturn call the initialisation and destroy routines of BAM.
 * 
 *
 * Input: req_info - pointer to the DL SE API request structure.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 ncs_bam_dl_func(NCS_LIB_REQ_INFO *req_info)
{
   if (req_info == NULL)
      return NCSCC_RC_FAILURE;

   if (req_info->i_op == NCS_LIB_REQ_CREATE)
   {
      return ncs_bam_initialize(req_info);
   }else if (req_info->i_op == NCS_LIB_REQ_DESTROY)
   {
      return ncs_bam_destroy();
   }

   return NCSCC_RC_FAILURE;
   
}

/*****************************************************************************
 * Function: ncs_bam_restart
 *
 * Purpose:  This function unregisters BAM with MDS and then registers. 
 *
 * Input: NONE.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
uns32 ncs_bam_restart()
{
   return NCSCC_RC_SUCCESS;
}
