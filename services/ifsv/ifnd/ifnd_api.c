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
  FILE NAME: IFND_API.C

  DESCRIPTION: API's used to init and create PWE of IfND.

  FUNCTIONS INCLUDED in this module:
  ifnd_lib_req ............ SE API to init or create PWE.
  ifnd_lib_init ........... library used to init IfND.  
  ifnd_lib_destroy ........ library used to destroy IfND.    
  ifnd_cb_destroy ......... Destroy CB.    
  ifnd_main_process ....... main process which is given as thread input.  
  ifnd_extract_input_info . extract the command line inputs received.
  ifnd_search_argv_list ... Search the data for the given delimter.

******************************************************************************/

#include "ifsv.h"
#include "ifnd.h"

IFSVLIB_STRUCT gl_ifsvlib;
uns32 gl_ifsv_hdl;
uns32 gl_ifnd_cb_hdl;

static void ifnd_main_process(NCSCONTEXT mbx);
static uns32 ifnd_cb_destroy (IFSV_CB *ifsv_cb);
static uns32 
ifnd_extract_input_info(int argc, char *argv[], IFSV_CREATE_PWE *ifsv_info);
static char *ifnd_search_argv_list(int argc, char *argv[], char *arg_prefix);

/****************************************************************************
 * Name          : ifnd_lib_req
 *
 * Description   : This is the NCS SE API which is used to init/destroy or 
 *                 Create/destroy PWE's. This will be called by SBOM.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_lib_req (NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;
   IFSV_CREATE_PWE pwe_param;

   switch (req_info->i_op)
   {      
      case NCS_LIB_REQ_CREATE:
         if (ifnd_extract_input_info(req_info->info.create.argc, 
            req_info->info.create.argv, &pwe_param) == NCSCC_RC_FAILURE)
         {
            break;
         }
         res = ifnd_lib_init(&pwe_param);
         break;
      case NCS_LIB_REQ_DESTROY:         
         res = ifnd_lib_destroy(m_IFSV_DEF_VRID, IFSV_COMP_IFND);
         break;
      default:
         break;
   }
   return (res);
}

/****************************************************************************
 * Name          : ifnd_lib_init
 *
 * Description   : This is the function which initalize the ifsv libarary.
 *                 This function creates an IPC mail Box and spawns IfD/IfND
 *                 thread.
 *                 This function initializes the CB, handle manager MDS, RMS 
 *                 and Registers with AMF with respect to the component Type 
 *                 (IfND).
 *
 * Arguments     : vrid      - This is the virtual router ID.
 *                 comp_type - component type.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifnd_lib_init (IFSV_CREATE_PWE *pwe_param)
{
   IFSV_COMP_TYPE comp_type = pwe_param->comp_type;   
   IFSV_CB        *ifsv_cb;
   uns32          res = NCSCC_RC_SUCCESS;
   uns8 comp_name[IFSV_MAX_COMP_NAME];
   SaAisErrorT       amf_error = SA_AIS_OK;
   NCS_SERVICE_ID svc_id;
   SaNameT        sname;
   SaAmfHealthcheckKeyT healthy;
   int8*       health_key;
#if (NCS_IFSV_IPXS == 1)
   IPXS_LIB_REQ_INFO ipxs_info;
#endif
 
   do
   {      
      strcpy(comp_name, m_IFND_COMP_NAME);
      svc_id = NCS_SERVICE_ID_IFND;      

      /* register with the Flex log service */   

      if ((res = ifsv_flx_log_reg(comp_type)) != NCSCC_RC_SUCCESS)
         {
           break;
         }
      /* allocate a CB  */
      ifsv_cb = m_MMGR_ALLOC_IFSV_CB;
      if (ifsv_cb == IFSV_NULL)
      {
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Unable to allocate CB :ifnd_lib_init "," ");

         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
         res = NCSCC_RC_FAILURE;
         break;
      }
      
      memset(ifsv_cb, 0, sizeof(IFSV_CB));
      ifsv_cb->hm_pid    = pwe_param->pool_id;
      ifsv_cb->comp_type = comp_type;
      ifsv_cb->my_svc_id = svc_id;
      ifsv_cb->shelf     = pwe_param->shelf_no;
      ifsv_cb->slot      = pwe_param->slot_no;
      /* embedding subslot changes */
      ifsv_cb->subslot   = pwe_param->subslot_no;         
      strcpy(ifsv_cb->comp_name, comp_name);         
      ifsv_cb->vrid      = pwe_param->vrid;
      ifsv_cb->oac_hdl   = pwe_param->oac_hdl;

      printf ("ifnd_api info - shelf: %d slot: %d subslot: %d svc_id - %d \n",  pwe_param->shelf_no,  pwe_param->slot_no, pwe_param->subslot_no, svc_id);
#if 1
      ifsv_cb->ha_state    = SA_AMF_HA_ACTIVE;
#endif      
      
      if ((res = ifnd_init_cb(ifsv_cb)) == NCSCC_RC_FAILURE)
         goto ifnd_cb_init_fail;         
      
      if((ifsv_cb->cb_hdl = ncshm_create_hdl(ifsv_cb->hm_pid, 
         ifsv_cb->my_svc_id, (NCSCONTEXT)ifsv_cb)) == 0)
      {
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_HDL_MGR_FAIL,comp_type);      
         res = NCSCC_RC_FAILURE;
         goto ifnd_hdl_fail;            
      }                        

      m_IFND_STORE_HDL(ifsv_cb->cb_hdl);
      m_IFSV_STORE_HDL(pwe_param->vrid,comp_type,ifsv_cb->cb_hdl);
      /* initialize all the AvSv callback */
      if ((res = ifnd_amf_init(ifsv_cb)) == NCSCC_RC_FAILURE)         
      {
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Amf Init failed :ifnd_lib_init "," ");
         goto ifnd_amf_init_fail;         
      }

      /* create a mail box */
      if ((res = m_NCS_IPC_CREATE(&ifsv_cb->mbx)) != NCSCC_RC_SUCCESS)         
      {
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Mailbox creation Failed :ifnd_lib_init "," ");
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_CREATE_FAIL,0);
         goto ifnd_ipc_create_fail;
      }
      
      /* store the vrid in the mailbox struct - TBD */
      
      if ((res = m_NCS_IPC_ATTACH(&ifsv_cb->mbx)) != NCSCC_RC_SUCCESS)
      {
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_CREATE_FAIL,0);
         goto ifnd_ipc_att_fail;
      }
      
      
      /* get the component name */
      memset(&sname,0,sizeof(sname));

      amf_error = saAmfComponentNameGet(ifsv_cb->amf_hdl,&sname);
      if (amf_error != SA_AIS_OK)
      {
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Amf Component Get Name Failed :ifnd_lib_init "," ");
         goto ifnd_mds_fail;
      }
      strncpy(ifsv_cb->comp_name, sname.value,IFSV_MAX_COMP_NAME-1);
      strncpy(m_IFND_COMP_NAME, sname.value,sizeof(m_IFND_COMP_NAME)-1);
      if ((res = ifnd_mds_init(ifsv_cb)) != NCSCC_RC_SUCCESS)
      {
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "MDS Init Failed :ifnd_lib_init "," ");
         m_IFND_LOG_API_L(IFSV_LOG_MDS_INIT_FAILURE,0xff);
         goto ifnd_mds_fail;
      }

     /* Register with MIBLIB */
     if((res = ifnd_reg_with_miblib()) != NCSCC_RC_SUCCESS)         
     {
        m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Registration with miblib Failed :ifnd_lib_init "," ");
        goto ifnd_mab_fail;
     }

      /* register with MASv */
      if((res = ifnd_reg_with_mab(ifsv_cb)) != NCSCC_RC_SUCCESS)         
      {
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Registration with mab Failed :ifnd_lib_init "," ");
         goto ifnd_mab_fail;
      }
      /* register IfSv component with AvSv */
      sname.length = strlen(ifsv_cb->comp_name);
     /* strcpy(sname.value,comp_name); */

      amf_error = saAmfComponentRegister(ifsv_cb->amf_hdl, &sname, 
         (SaNameT*)IFSV_NULL);
      if (amf_error != SA_AIS_OK)
      {            
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Amf Component Registration Failed :ifnd_lib_init "," ");
         m_IFND_LOG_API_L(IFSV_LOG_AMF_REG_FAILURE,comp_type);
         res = NCSCC_RC_FAILURE;
         goto ifnd_amf_reg_fail;
      }
#if (NCS_IFSV_IPXS == 1)
      memset(&ipxs_info, 0, sizeof(IPXS_LIB_REQ_INFO));
      ipxs_info.op = NCS_LIB_REQ_CREATE;
      ipxs_info.info.create.oac_hdl = ifsv_cb->oac_hdl;
      ipxs_info.info.create.pool_id = ifsv_cb->hm_pid;
      ipxs_info.info.create.svc_id = ifsv_cb->my_svc_id;
      ipxs_info.info.create.mds_hdl = ifsv_cb->my_mds_hdl;
      ipxs_info.info.create.ifsv_hdl = ifsv_cb->cb_hdl;
      res = ipxs_ifnd_lib_req(&ipxs_info);

      if(res != NCSCC_RC_SUCCESS)
      {
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Ifnd IPXS Init Failed :ifnd_lib_init "," ");
         goto ifnd_ipxs_init_fail;
      }
#endif

      if ((res = m_NCS_TASK_CREATE ((NCS_OS_CB)ifnd_main_process,
         (NCSCONTEXT)ifsv_cb,
         m_IFSV_TASKNAME(comp_type),
         m_IFSV_PRIORITY,
         m_IFSV_STACKSIZE,
         &m_IFSV_GET_TASK_HDL(vrid,comp_type))) != NCSCC_RC_SUCCESS)         
      {
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Ifnd Task Create Failed :ifnd_lib_init "," ");
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_TASK_CREATE_FAIL,0);
         goto ifnd_task_create_fail;
      }
      
      if ((res = m_NCS_TASK_START(m_IFSV_GET_TASK_HDL(vrid,comp_type)))
         != NCSCC_RC_SUCCESS)         
      {
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "Ifnd Task Create Failed :ifnd_lib_init "," ");
         m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_TASK_CREATE_FAIL,\
            (long)m_IFSV_GET_TASK_HDL(vrid,comp_type));
         goto ifnd_task_start_fail;            
      }

      /*   start the AMF Health Check  */
      /** start the AMF health check **/
      memset(&sname,0,sizeof(sname));
      strcpy(sname.value,ifsv_cb->comp_name);
      sname.length = strlen(ifsv_cb->comp_name);

      memset(&healthy,0,sizeof(healthy));

      health_key = getenv("IFSV_ENV_HEALTHCHECK_KEY");
      if(health_key == NULL)
      {
         strcpy(healthy.key,"C3D4");
         /* TBD Log the info */
      }
      else
      {
         strncpy(healthy.key,health_key, SA_AMF_HEALTHCHECK_KEY_MAX -1);
      }
      healthy.keyLen=strlen(healthy.key);

      amf_error = saAmfHealthcheckStart(ifsv_cb->amf_hdl,&sname,&healthy,
         SA_AMF_HEALTHCHECK_AMF_INVOKED,SA_AMF_COMPONENT_RESTART);
      if (amf_error != SA_AIS_OK)
      {
         /* TBD Log the error */
         m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "AMF Health check start Failed :ifnd_lib_init "," ");
      }
      /* map the handle in a table with respect to component type and 
      * vrid 
      */
      m_IFND_LOG_API_L(IFSV_LOG_SE_INIT_DONE,comp_type);

      break;

ifnd_task_start_fail:      
      m_NCS_TASK_RELEASE(m_IFSV_GET_TASK_HDL(vrid,comp_type));
ifnd_task_create_fail:
      m_NCS_IPC_DETACH(&m_IFSV_GET_MBX_HDL(ifsv_cb->vrid,ifsv_cb->comp_type)
         ,ifnd_clear_mbx,ifsv_cb);      

#if (NCS_IFSV_IPXS == 1)
ifnd_ipxs_init_fail:
#endif

ifnd_amf_reg_fail:
      /*MAB destroy -  */
      ifnd_unreg_with_mab(ifsv_cb);
ifnd_mab_fail:
      ifnd_mds_shut(ifsv_cb);
ifnd_mds_fail:  
      m_NCS_TASK_STOP(m_IFSV_GET_TASK_HDL(vrid,comp_type));

ifnd_ipc_att_fail:
      
      m_NCS_IPC_RELEASE(&m_IFSV_GET_MBX_HDL(vrid,comp_type), NULL);
ifnd_ipc_create_fail:
      saAmfFinalize(ifsv_cb->amf_hdl);
ifnd_amf_init_fail:
      ncshm_destroy_hdl(ifsv_cb->my_svc_id, ifsv_cb->cb_hdl);
ifnd_hdl_fail:            
      ifnd_cb_destroy(ifsv_cb);
ifnd_cb_init_fail:
      ifsv_flx_log_dereg(comp_type);
      m_MMGR_FREE_IFSV_CB(ifsv_cb);
      m_IFND_LOG_API_L(IFSV_LOG_SE_INIT_FAILURE,comp_type);      
      
   } while (0);
   return (res);
}

/****************************************************************************
 * Name          : ifnd_lib_destroy
 *
 * Description   : This is the function which destroy the ifsv libarary.
 *                 This function releases the Task and the IPX mail Box.
 *                 This function unregisters with AMF, destroies handle 
 *                 manager, CB and clean up all the component specific 
 *                 databases.
 *
 * Arguments     : vrid      - This is the virtual router ID.
 *                 comp_type - component type.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifnd_lib_destroy (uns32 vrid, uns32 comp_type)
{ 
   IFSV_CB *ifsv_cb;
   uns32   ifsv_hdl;
   SaNameT sname;
#if (NCS_IFSV_IPXS == 1)
   IPXS_LIB_REQ_INFO ipxs_info;
#endif
    
   ifsv_hdl = m_IFSV_GET_HDL(vrid,comp_type);
   if ((ifsv_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFND, ifsv_hdl))
      == IFSV_CB_NULL)
   {
      m_IFND_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,comp_type);
      return (NCSCC_RC_FAILURE);
   } else
   {  
      sname.length = strlen(ifsv_cb->comp_name);
      strcpy(sname.value,ifsv_cb->comp_name);

      saAmfComponentUnregister(ifsv_cb->amf_hdl, &sname, (SaNameT*)IFSV_NULL);
      saAmfFinalize(ifsv_cb->amf_hdl);      
      
      m_NCS_IPC_DETACH(&m_IFSV_GET_MBX_HDL(ifsv_cb->vrid,ifsv_cb->comp_type)
         ,ifnd_clear_mbx,ifsv_cb);      
      m_NCS_TASK_STOP(m_IFSV_GET_TASK_HDL(vrid,comp_type));
      m_NCS_TASK_RELEASE(m_IFSV_GET_TASK_HDL(vrid,comp_type));
      ifnd_cb_destroy(ifsv_cb);  
      ncshm_give_hdl(ifsv_hdl);
      ncshm_destroy_hdl(ifsv_cb->my_svc_id, ifsv_cb->cb_hdl);
      ifnd_mds_shut(ifsv_cb);      
      m_NCS_IPC_RELEASE(&m_IFSV_GET_MBX_HDL(vrid,comp_type), NULL);
      m_MMGR_FREE_IFSV_CB(ifsv_cb);

#if (NCS_IFSV_IPXS == 1)
      memset(&ipxs_info, 0, sizeof(IPXS_LIB_REQ_INFO));
      ipxs_info.op = NCS_LIB_REQ_DESTROY;
      ipxs_ifnd_lib_req(&ipxs_info);
#endif

      m_IFND_LOG_API_L(IFSV_LOG_SE_DESTROY_DONE,comp_type);
      /* register with the Flex log service */   
      ifsv_flx_log_dereg(comp_type);            
      return (NCSCC_RC_SUCCESS);
   }
}

/****************************************************************************
 * Name          : ifnd_cb_destroy
 *
 * Description   : This is the function depending on the component type, it 
 *                 will call the corresponding CB destroy functions.
 *
 * Arguments     : ifsv_cb  - Ifsv control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifnd_cb_destroy (IFSV_CB *ifsv_cb)
{     
   m_NCS_SEM_RELEASE(ifsv_cb->health_sem);
   ifnd_all_intf_rec_del (ifsv_cb);
   
   if (ifsv_cb->idim_up == TRUE)
   {
      /* if already the idim driver is up, then destory that thread */
      ifnd_idim_destroy(ifsv_cb);
   }
   
   ncs_patricia_tree_destroy(&ifsv_cb->if_tbl);
   
   /* delete all the left over slot/port Vs Ifindex mapping*/
   ifsv_ifindex_spt_map_del_all(ifsv_cb);
   
   ncs_patricia_tree_destroy(&ifsv_cb->if_map_tbl);   
   
   ncs_patricia_tree_destroy(&ifsv_cb->mds_dest_tbl);   

   m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);

   /* EDU cleanup */
   m_NCS_EDU_HDL_FLUSH(&ifsv_cb->edu_hdl);
   
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifnd_main_process
 *
 * Description   : This is the function which is given as a input to the 
 *                 IfND task.
 *                 This function will be select of both the FD's (AMF FD and
 *                 Mail Box FD), depending on which FD has been selected, it
 *                 will call the corresponding routines.
 *
 * Arguments     : mbx  - This is the mail box pointer on which IfD/IfND is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void
ifnd_main_process(NCSCONTEXT info)
{   
   IFSV_CB             *ifsv_cb = (IFSV_CB*)info;      
   uns32               comp_type;
   NCS_SEL_OBJ_SET     all_sel_obj;
   NCS_SEL_OBJ         mbx_fd;
   SaSelectionObjectT  amf_sel_obj;
   SaAisErrorT         amf_error;   
   NCS_SEL_OBJ         ams_ncs_sel_obj;
   NCS_SEL_OBJ         high_sel_obj;
   SaAmfHandleT        amf_hdl;
   SYSF_MBX            mbx = ifsv_cb->mbx;
   IFSV_EVT            *evt = IFSV_NULL;

   mbx_fd = m_NCS_IPC_GET_SEL_OBJ(&ifsv_cb->mbx);
   amf_hdl = ifsv_cb->amf_hdl;
   comp_type = ifsv_cb->comp_type;
   
   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj); 
   amf_error = saAmfSelectionObjectGet(ifsv_cb->amf_hdl, &amf_sel_obj);

   if (amf_error != SA_AIS_OK)
   {     
      m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
          "AMF Component Name get Failed :ifnd_main_process "," ");
      m_IFND_LOG_API_L(IFSV_LOG_AMF_GET_OBJ_FAILURE,comp_type);
      return;
   }
   m_SET_FD_IN_SEL_OBJ((uns32)amf_sel_obj,ams_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(ams_ncs_sel_obj, &all_sel_obj);
   
   high_sel_obj = m_GET_HIGHER_SEL_OBJ(ams_ncs_sel_obj,mbx_fd);
   
   while (m_NCS_SEL_OBJ_SELECT(high_sel_obj,&all_sel_obj,0,0,NULL) != -1)
   {      
      /* process all the AMF messages */
      if (m_NCS_SEL_OBJ_ISSET(ams_ncs_sel_obj,&all_sel_obj))
      {
         /* dispatch all the AMF pending function */
         amf_error = saAmfDispatch(amf_hdl, SA_DISPATCH_ALL);
         if (amf_error != SA_AIS_OK)
         {
            m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
                 "AMF Dispatch Failed :ifnd_main_process "," ");
            m_IFND_LOG_API_L(IFSV_LOG_AMF_DISP_FAILURE,comp_type);
         }
      }
      /* process the IfSv Mail box */
      if (m_NCS_SEL_OBJ_ISSET(mbx_fd,&all_sel_obj))
      {        
         
         if (IFSV_NULL != (evt = (IFSV_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(&mbx,evt)))
         {
            /* now got the IPC mail box event */
            if ((evt->type >= IFND_EVT_BASE) && (evt->type < IFND_EVT_MAX))
            {
               /* This event belongs to IfND */
               ifnd_process_evt(evt);
            } 
#if(NCS_IFSV_IPXS == 1)
            else if(evt->type == IFSV_IPXS_EVT)
            {
              ifnd_process_evt(evt);
            }
#endif         
#if (NCS_VIP == 1)
            else if((evt->type > IFSV_IPXS_EVT_MAX) && (evt->type <= IFSV_VIP_EVT_MAX))
            {
               ifnd_process_evt(evt);
            }
#endif 
            else
            {
               m_IFND_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT,"None",evt->type);
               m_MMGR_FREE_IFSV_EVT(evt);         
            }
         }
      }
      /*** set the fd's again ***/
      m_NCS_SEL_OBJ_SET(ams_ncs_sel_obj,&all_sel_obj);
      m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);   
   }
   return;
}

/****************************************************************************
 * Name          : ifnd_extract_input_info
 *
 * Description   : This is the NCS SE API which is used to init/destroy or 
 *                 Create/destroy PWE's. This will be called by SBOM.
 *
 * Arguments     : argc  - This is the Number of arguments received.
 *                 argv  - string received.
 *                 ifsv_info - Structure to be filled for initing the service.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ifnd_extract_input_info(int argc, char *argv[], IFSV_CREATE_PWE *ifsv_info)
{
   char                 *p_field;
   uns32                node_id=0;
   memset(ifsv_info,0,sizeof(IFSV_CREATE_PWE));

   p_field = ifnd_search_argv_list(argc, argv, "SHELF_ID=");
   if (p_field == NULL)
   {      
      printf("\nERROR:Problem in shelf_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   if (sscanf(p_field + strlen("SHELF_ID="), "%d", &ifsv_info->shelf_no) != 1)
   {      
      printf("\nERROR:Problem in shelf_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   p_field = ifnd_search_argv_list(argc, argv, "SLOT_ID=");
   if (p_field == NULL)
   {      
      printf("\nERROR:Problem in slot_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   if (sscanf(p_field + strlen("SLOT_ID="), "%d", &ifsv_info->slot_no) != 1)
   {      
      printf("\nERROR:Problem in slot_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   /* embedding subslot changes */
   p_field = ifnd_search_argv_list(argc, argv, "NODE_ID=");
   if (p_field == NULL)
   {
      printf("\nERROR:Problem in node_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   if (sscanf(p_field + strlen("NODE_ID="), "%d", &node_id) != 1)
   {
      printf("\nERROR:Problem in node_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   ifsv_info->subslot_no = (node_id & 0x0f);

   /* Since the Pool Id we need to have it as constant */
   ifsv_info->pool_id   = NCS_HM_POOL_ID_COMMON;
   ifsv_info->vrid      = m_IFSV_DEF_VRID;
   ifsv_info->comp_type = IFSV_COMP_IFND;
   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifnd_search_argv_list
 *
 * Description   : This is the NCS SE API which is used to init/destroy or 
 *                 Create/destroy PWE's. This will be called by SBOM.
 *
 * Arguments     : argc       - This is the Number of arguments received.
 *                 argv       - String received.
 *                 arg_prefix - This is the delimiter which is used to find 
 *                              where the actual data starts from.
 *
 * Return Values : Starting pointer to which it points to our req data.
 *
 * Notes         : None.
 *****************************************************************************/
static char *ifnd_search_argv_list(int argc, char *argv[], char *arg_prefix)
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
