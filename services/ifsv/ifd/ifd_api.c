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
  FILE NAME: IFD_API.C

  DESCRIPTION: API's used to init and create PWE of IfD.

  FUNCTIONS INCLUDED in this module:
  ifd_lib_req ............ SE API to init or create PWE.
  ifd_lib_init ........... library used to init IfD/IfND.  
  ifd_lib_destroy ........ library used to destroy IfD/IfND.    
  ifd_cb_destroy ......... Destroy CB.  
  ifd_main_process ........ main process which is given as thread input.  
  ifd_extract_input_info .. extract the command line inputs received.
  ifd_search_argv_list .... Search the data for the given delimter.

******************************************************************************/

#include "ifd.h"
#include "ifd_red.h"

IFSVLIB_STRUCT gl_ifsvlib;
uns32 gl_ifsv_hdl;
uns32 gl_ifd_cb_hdl;

static void ifd_main_process(NCSCONTEXT mbx);
static uns32 ifd_cb_destroy (IFSV_CB *ifsv_cb);
static uns32 
ifd_extract_input_info(int argc, char *argv[], IFSV_CREATE_PWE *ifsv_info);
static char *ifd_search_argv_list(int argc, char *argv[], char *arg_prefix);


/****************************************************************************
 * Name          : ifd_lib_req
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
ifd_lib_req (NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;
   IFSV_CREATE_PWE pwe_param;

   switch (req_info->i_op)
   {      
      case NCS_LIB_REQ_CREATE:         
         if (ifd_extract_input_info(req_info->info.create.argc, 
            req_info->info.create.argv, &pwe_param) == NCSCC_RC_FAILURE)
         {
            break;
         }
         res = ifd_lib_init(&pwe_param);
         break;
      case NCS_LIB_REQ_DESTROY:         
         res = ifd_lib_destroy(m_IFSV_DEF_VRID, IFSV_COMP_IFD);
         break;
      default:
         break;
   }
   return (res);
}

/****************************************************************************
 * Name          : ifd_lib_init
 *
 * Description   : This is the function which initalize the ifsv libarary.
 *                 This function creates an IPC mail Box and spawns IfD
 *                 thread.
 *                 This function initializes the CB, handle manager MDS, RMS 
 *                 and Registers with AMF with respect to the component Type 
 *                 (IfD/IfND).
 *
 * Arguments     : vrid      - This is the virtual router ID.
 *                 comp_type - component type.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifd_lib_init (IFSV_CREATE_PWE *pwe_param)
{
   IFSV_COMP_TYPE comp_type = pwe_param->comp_type;   
   IFSV_CB        *ifsv_cb;
   uns32          res = NCSCC_RC_SUCCESS;
   SaAisErrorT       amf_error = SA_AIS_OK;
   NCS_SERVICE_ID svc_id;
   SaNameT        sname;
   uns32          ii;
#if (NCS_VIP == 1)
   uns32 rc;
#endif
#if (NCS_IFSV_IPXS == 1)
   IPXS_LIB_REQ_INFO ipxs_info;
#endif
   
   do
   {      
      svc_id = NCS_SERVICE_ID_IFD;    

      /* register with the Flex log service */   

      if ((res = ifsv_flx_log_reg(comp_type)) != NCSCC_RC_SUCCESS)
      {
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_flx_log_reg():ifd_lib_init failed,res:",res);
         printf("ifsv_flx_log_reg failed\n");
         break;
      }
      
      /* allocate a CB  */
      ifsv_cb = m_MMGR_ALLOC_IFSV_CB;
      if (ifsv_cb == IFSV_NULL)
      {
         m_IFSV_LOG_SYS_CALL_FAIL(ifsv_log_svc_id_from_comp_type(comp_type),\
            IFSV_LOG_MEM_ALLOC_FAIL,0);
         res = NCSCC_RC_FAILURE;
         printf("m_MMGR_ALLOC_IFSV_CB failed\n");
         goto ifd_cb_alloc_fail;
      }
      
      memset(ifsv_cb, 0, sizeof(IFSV_CB));
      ifsv_cb->ifd_mbcsv_data_resp_enc_func_ptr = ifd_mbcsv_enc_data_resp;
      ifsv_cb->ifd_mbcsv_data_resp_dec_func_ptr = ifd_mbcsv_dec_data_resp;
      ifsv_cb->hm_pid    = pwe_param->pool_id;
      ifsv_cb->comp_type = comp_type;
      ifsv_cb->my_svc_id = svc_id;
      ifsv_cb->shelf     = pwe_param->shelf_no;
      ifsv_cb->slot      = pwe_param->slot_no;         
      /* embedding subslot changes */
      ifsv_cb->subslot   = pwe_param->subslot_no; 
      ifsv_cb->vrid      = pwe_param->vrid;
      ifsv_cb->oac_hdl   = pwe_param->oac_hdl;

      for (ii = 0; ii < MAX_IFND_NODES; ii++)
      {
         ifsv_cb->ifnd_mds_addr[ii].valid = FALSE;
         ifsv_cb->ifnd_mds_addr[ii].ifndAddr = 0;
      }

#if (NCS_IFSV_BOND_INTF==1)
      ifsv_cb->bondif_max_portnum = 30;
#endif
      
      if ((res = ifd_init_cb(ifsv_cb)) == NCSCC_RC_FAILURE)
      {
         printf("ifd_init_cb failed\n");
         goto ifd_cb_init_fail;   
      }     
      
      if((ifsv_cb->cb_hdl = ncshm_create_hdl(ifsv_cb->hm_pid, 
         ifsv_cb->my_svc_id, (NCSCONTEXT)ifsv_cb)) == 0)
      {
         m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_HDL_MGR_FAIL,comp_type);
         res = NCSCC_RC_FAILURE;
         printf("ncshm_create_hdl failed\n");
         goto ifd_hdl_fail;            
      }      
      
      m_IFD_STORE_HDL(ifsv_cb->cb_hdl);
      m_IFSV_STORE_HDL(pwe_param->vrid,comp_type,ifsv_cb->cb_hdl);
  
      /* initialize all the AvSv callback */
      if ((res = ifd_amf_init(ifsv_cb)) == NCSCC_RC_FAILURE)         
        {
         printf("ifd_amf_init failed\n");
         goto ifd_amf_init_fail;         
        }

      /* get the component name */
      memset(&sname,0,sizeof(sname));

      amf_error = saAmfComponentNameGet(ifsv_cb->amf_hdl,&sname);
      if (amf_error != SA_AIS_OK)
      {        
         printf("saAmfComponentNameGet failed\n");
         goto ifd_mds_fail;
      }
      strcpy(ifsv_cb->comp_name, sname.value);         
      strcpy(m_IFD_COMP_NAME, sname.value);         


      if ((res = ifd_mds_init(ifsv_cb)) != NCSCC_RC_SUCCESS)
      {
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mds_init failed,res:",res);
         m_IFD_LOG_API_L(IFSV_LOG_MDS_INIT_FAILURE,0xff);
         printf("ifd_mds_init failed\n");
         goto ifd_mds_fail;
      }

      /* Register with MIBLIB */
       if((res = ifd_reg_with_miblib( )) != NCSCC_RC_SUCCESS)         
        {
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_reg_with_miblib failed,res:",res);
         printf("ifd_reg_with_miblib failed\n");
         goto ifd_mab_fail;
        }

      /* register with MASv */
      if((res = ifd_reg_with_mab(ifsv_cb)) != NCSCC_RC_SUCCESS)         
       {
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_reg_with_mab failed,res:",res);
         printf("ifd_reg_with_mab failed\n");
         goto ifd_mab_fail;
        }

    #if (NCS_VIP == 1)
      /* Register with MIBLIB */
      /* Register with OAC */
      rc = vip_miblib_reg();
      /* Register VIP with OAC (MAB). */
      if (rc == NCSCC_RC_SUCCESS)
      {
         rc = vip_reg_with_oac(ifsv_cb);
         if(rc != NCSCC_RC_SUCCESS)
         {
            m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"vip_reg_with_oac() failed,rc:",rc);
            printf("vip_reg_with_oac failed\n");
            goto vip_oac_reg_fail;
         }
      }
      else
         printf("vip_miblib_reg failed\n");
    #endif
     
      /* register IfSv component with AvSv */
      amf_error = saAmfComponentRegister(ifsv_cb->amf_hdl, &sname, 
         (SaNameT*)IFSV_NULL);
      if (amf_error != SA_AIS_OK)
      {            
         m_IFD_LOG_API_L(IFSV_LOG_AMF_REG_FAILURE,comp_type);
         res = NCSCC_RC_FAILURE;
         printf("saAmfComponentRegister failed\n");
         goto ifd_amf_reg_fail;
      }

      /*   Initialise with the MBCSV service  */
      if(ifd_mbcsv_register(ifsv_cb)!=NCSCC_RC_SUCCESS)
      {
         printf("ifd_mbcsv_register failed\n");
         goto ifd_amf_reg_fail;
      } 

      /* create a mail box */
      if ((res = m_NCS_IPC_CREATE(&ifsv_cb->mbx))
         != NCSCC_RC_SUCCESS)
      {
         m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_CREATE_FAIL,0);
         printf("m_NCS_IPC_CREATE failed\n");
         goto ifd_ipc_create_fail;
      }

      /* store the vrid in the mailbox struct - TBD */

      if ((res = m_NCS_IPC_ATTACH(&ifsv_cb->mbx)) != NCSCC_RC_SUCCESS)
      {
         m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_CREATE_FAIL,0);
         printf("m_NCS_IPC_ATTACH failed \n");
         goto ifd_ipc_att_fail;
      }
     
#if (NCS_IFSV_IPXS == 1)
      memset(&ipxs_info, 0, sizeof(IPXS_LIB_REQ_INFO));
      ipxs_info.op = NCS_LIB_REQ_CREATE;
      ipxs_info.info.create.oac_hdl = ifsv_cb->oac_hdl;
      ipxs_info.info.create.pool_id = ifsv_cb->hm_pid;
      ipxs_info.info.create.svc_id = ifsv_cb->my_svc_id;
      ipxs_info.info.create.mds_hdl = ifsv_cb->my_mds_hdl;
      res = ipxs_ifd_lib_req(&ipxs_info);

      if(res != NCSCC_RC_SUCCESS)
      {
         printf("ipxs_ifd_lib_req failed\n");
         goto ifd_ipxs_init_fail;
      }
#endif

      if ((res = m_NCS_TASK_CREATE ((NCS_OS_CB)ifd_main_process,
         (NCSCONTEXT)ifsv_cb,
         m_IFSV_TASKNAME(comp_type),
         m_IFSV_PRIORITY,
         m_IFSV_STACKSIZE,
         &m_IFSV_GET_TASK_HDL(vrid,comp_type))) != NCSCC_RC_SUCCESS)
      {
         printf("m_NCS_TASK_CREATE failed\n");
         m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_TASK_CREATE_FAIL,0);
         goto ifd_task_create_fail;
      }

      if ((res = m_NCS_TASK_START(m_IFSV_GET_TASK_HDL(vrid,comp_type)))
         != NCSCC_RC_SUCCESS)
      {
         printf("m_NCS_TASK_START  failed\n");
         m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_TASK_CREATE_FAIL,\
         (long)m_IFSV_GET_TASK_HDL(vrid,comp_type));
         goto ifd_task_start_fail;
      }


      /* map the handle in a table with respect to component type and 
      * vrid 
      */
      m_IFSV_LOG_API_L(ifsv_log_svc_id_from_comp_type(comp_type),\
         IFSV_LOG_SE_INIT_DONE,comp_type);
         printf("\n\nIFD INITIALIZATION SUCCESS\n\n");
      break;

ifd_task_start_fail:      
      m_NCS_TASK_RELEASE(m_IFSV_GET_TASK_HDL(vrid,comp_type));
ifd_task_create_fail:
#if (NCS_IFSV_IPXS == 1)
      memset(&ipxs_info, 0, sizeof(IPXS_LIB_REQ_INFO));
      ipxs_info.op = NCS_LIB_REQ_DESTROY;
      ipxs_ifd_lib_req(&ipxs_info);
ifd_ipxs_init_fail:
#endif
      m_NCS_IPC_DETACH(&m_IFSV_GET_MBX_HDL(ifsv_cb->vrid,ifsv_cb->comp_type)
                       ,ifd_clear_mbx,ifsv_cb);
ifd_ipc_att_fail:      
      m_NCS_IPC_RELEASE(&m_IFSV_GET_MBX_HDL(vrid,comp_type), NULL);
ifd_ipc_create_fail:      
ifd_amf_reg_fail:

#if (NCS_VIP == 1)
   vip_unreg_with_oac(ifsv_cb);
vip_oac_reg_fail:
#endif
      ifd_unreg_with_mab(ifsv_cb);

ifd_mab_fail:
      ifd_mds_shut(ifsv_cb);
ifd_mds_fail:
      saAmfFinalize(ifsv_cb->amf_hdl);
ifd_amf_init_fail:
      ncshm_destroy_hdl(ifsv_cb->my_svc_id, ifsv_cb->cb_hdl);
ifd_hdl_fail:      
      ifd_cb_destroy(ifsv_cb);
ifd_cb_init_fail:
      m_MMGR_FREE_IFSV_CB(ifsv_cb);      
ifd_cb_alloc_fail:      
      m_IFD_LOG_API_L(IFSV_LOG_SE_INIT_FAILURE,comp_type);
      ifsv_flx_log_dereg(comp_type);
      
   } while (0);
   return (res);
}

/****************************************************************************
 * Name          : ifd_lib_destroy
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
ifd_lib_destroy (uns32 vrid, uns32 comp_type)
{ 
   IFSV_CB *ifsv_cb;
   uns32   ifsv_hdl;
   SaNameT sname;

#if (NCS_IFSV_IPXS == 1)
   IPXS_LIB_REQ_INFO ipxs_info;
#endif
 
    
   ifsv_hdl = m_IFSV_GET_HDL(vrid,comp_type);
   if ((ifsv_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_IFD, ifsv_hdl))
      == IFSV_CB_NULL)
   {
      m_IFD_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,comp_type);
      return (NCSCC_RC_FAILURE);
   } else
   {      
      sname.length = strlen(ifsv_cb->comp_name);
      strcpy(sname.value,ifsv_cb->comp_name);
      saAmfComponentUnregister(ifsv_cb->amf_hdl, &sname, (SaNameT*)IFSV_NULL);
      saAmfFinalize(ifsv_cb->amf_hdl);      
      
      m_NCS_IPC_DETACH(&m_IFSV_GET_MBX_HDL(ifsv_cb->vrid,ifsv_cb->comp_type)
         ,ifd_clear_mbx,ifsv_cb);

      m_NCS_TASK_STOP(m_IFSV_GET_TASK_HDL(vrid,comp_type));
      m_NCS_TASK_RELEASE(m_IFSV_GET_TASK_HDL(vrid,comp_type));

      /* deregister with MBCSV */
      ifd_mbcsv_finalize(ifsv_cb);

      ifd_cb_destroy(ifsv_cb);
      ncshm_give_hdl(ifsv_hdl);
      ncshm_destroy_hdl(ifsv_cb->my_svc_id, ifsv_cb->cb_hdl);
      ifd_mds_shut(ifsv_cb);      
      m_NCS_IPC_RELEASE(&m_IFSV_GET_MBX_HDL(vrid,comp_type), NULL);
      m_MMGR_FREE_IFSV_CB(ifsv_cb);

#if (NCS_IFSV_IPXS == 1)
      memset(&ipxs_info, 0, sizeof(IPXS_LIB_REQ_INFO));
      ipxs_info.op = NCS_LIB_REQ_DESTROY;
      ipxs_ifd_lib_req(&ipxs_info);
#endif

      m_IFD_LOG_API_L(IFSV_LOG_SE_DESTROY_DONE,comp_type);
      /* register with the Flex log service */   
      ifsv_flx_log_dereg(comp_type);
      return (NCSCC_RC_SUCCESS);
   }   
}


/****************************************************************************
 * Name          : ifd_cb_destroy
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
ifd_cb_destroy (IFSV_CB *ifsv_cb)
{     
   /* delete all the records in the interface table*/
   if (ifsv_cb->comp_type == IFSV_COMP_IFD)
   {
      ifd_all_intf_rec_del (ifsv_cb);
      m_IFD_IFAP_DESTROY(ifsv_cb);
   } 
   ncs_patricia_tree_destroy(&ifsv_cb->if_tbl);

   /* delete all the left over slot/port Vs Ifindex mapping*/
   ifsv_ifindex_spt_map_del_all(ifsv_cb);

   ncs_patricia_tree_destroy(&ifsv_cb->if_map_tbl);   
   ncs_patricia_tree_destroy(&ifsv_cb->ifnd_node_id_tbl);   
   m_NCS_OS_LOCK(&ifsv_cb->intf_rec_lock, NCS_OS_LOCK_RELEASE, 0);

   /* EDU cleanup */
   m_NCS_EDU_HDL_FLUSH(&ifsv_cb->edu_hdl);

   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifd_main_process
 *
 * Description   : This is the function which is given as a input to the 
 *                 Ifd task.
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
ifd_main_process(NCSCONTEXT info)
{   
   IFSV_CB             *ifsv_cb = (IFSV_CB*)info;      
   uns32               comp_type;
   NCS_SEL_OBJ_SET     all_sel_obj;
   NCS_SEL_OBJ         mbx_fd;
   SaSelectionObjectT  amf_sel_obj;
   SaAisErrorT            amf_error;   
   NCS_SEL_OBJ         ams_ncs_sel_obj;
   NCS_SEL_OBJ         mbcsv_ncs_sel_obj;
   NCS_SEL_OBJ         high_sel_obj;
   SaAmfHandleT        amf_hdl;
   SYSF_MBX            mbx = ifsv_cb->mbx;
   IFSV_EVT            *evt = IFSV_NULL;
   NCS_MBCSV_ARG       mbcsv_arg;

   mbx_fd = m_NCS_IPC_GET_SEL_OBJ(&ifsv_cb->mbx);
   amf_hdl = ifsv_cb->amf_hdl;
   comp_type = ifsv_cb->comp_type;
   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);   

   amf_error = saAmfSelectionObjectGet(ifsv_cb->amf_hdl, &amf_sel_obj);

   if (amf_error != SA_AIS_OK)
   {     
      m_IFSV_LOG_API_L(ifsv_log_svc_id_from_comp_type(comp_type),\
            IFSV_LOG_AMF_GET_OBJ_FAILURE,comp_type);
      return;
   }
   m_SET_FD_IN_SEL_OBJ((uns32)amf_sel_obj,ams_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(ams_ncs_sel_obj, &all_sel_obj);

   m_SET_FD_IN_SEL_OBJ((uns32)ifsv_cb->mbcsv_sel_obj, mbcsv_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(mbcsv_ncs_sel_obj, &all_sel_obj);

   high_sel_obj = m_GET_HIGHER_SEL_OBJ(ams_ncs_sel_obj,mbx_fd);
   high_sel_obj = m_GET_HIGHER_SEL_OBJ(high_sel_obj, mbcsv_ncs_sel_obj);
   
   while (m_NCS_SEL_OBJ_SELECT(high_sel_obj,&all_sel_obj,0,0,NULL) != -1)
   {
      
      /* process all the AMF messages */
      if (m_NCS_SEL_OBJ_ISSET(ams_ncs_sel_obj,&all_sel_obj))
      {
         /* dispatch all the AMF pending function */
         amf_error = saAmfDispatch(amf_hdl, SA_DISPATCH_ALL);
         if (amf_error != SA_AIS_OK)
         {
            m_IFSV_LOG_API_L(ifsv_log_svc_id_from_comp_type(comp_type),\
               IFSV_LOG_AMF_DISP_FAILURE,comp_type);
         }
      }

           /* Process all the MBCSV messages  */
      if(m_NCS_SEL_OBJ_ISSET(mbcsv_ncs_sel_obj,&all_sel_obj))
      {
          /* dispatch all the MBCSV pending callbacks */
         mbcsv_arg.i_op        = NCS_MBCSV_OP_DISPATCH;
         mbcsv_arg.i_mbcsv_hdl = ifsv_cb->mbcsv_hdl;
         mbcsv_arg.info.dispatch.i_disp_flags = SA_DISPATCH_ALL;
         if(ncs_mbcsv_svc(&mbcsv_arg)!=SA_AIS_OK)
         {
         }
      }


      /* process the IfSv Mail box */
      if (m_NCS_SEL_OBJ_ISSET(mbx_fd,&all_sel_obj))
      {
         /* now got the IPC mail box event */
         if (IFSV_NULL != (evt = (IFSV_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(&mbx,evt)))
         {
            if ((evt->type >= IFD_EVT_BASE) && (evt->type < IFD_EVT_MAX))
            {
               /* This event belongs to IfD */
               ifd_process_evt(evt);
            } 
            else if(evt->type == IFSV_IPXS_EVT)
            {
               ifd_process_evt(evt);
            }
#if (NCS_VIP == 1)
            else if((evt->type > IFSV_IPXS_EVT_MAX) && (evt->type <=IFSV_VIP_EVT_MAX))
            {
               ifd_process_evt(evt);
            }
#endif
            else
            {
               m_IFD_LOG_EVT_L(IFSV_LOG_UNKNOWN_EVENT,"None",evt->type);
               m_MMGR_FREE_IFSV_EVT(evt);         
            }
         }
      }
      /*** set the fd's again ***/
      m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
      m_NCS_SEL_OBJ_SET(ams_ncs_sel_obj,&all_sel_obj);
      m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);   
      m_NCS_SEL_OBJ_SET(mbcsv_ncs_sel_obj,&all_sel_obj);

    }     
   return;
}

/****************************************************************************
 * Name          : ifd_extract_input_info
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
ifd_extract_input_info(int argc, char *argv[], IFSV_CREATE_PWE *ifsv_info)
{
   char                 *p_field;
   uns32                node_id=0;
   memset(ifsv_info,0,sizeof(IFSV_CREATE_PWE));

   p_field = ifd_search_argv_list(argc, argv, "SHELF_ID=");
   if (p_field == NULL)
   {      
      m_NCS_CONS_PRINTF("\nERROR:Problem in shelf_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   if (sscanf(p_field + strlen("SHELF_ID="), "%d", &ifsv_info->shelf_no) != 1)
   {      
      m_NCS_CONS_PRINTF("\nERROR:Problem in shelf_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   p_field = ifd_search_argv_list(argc, argv, "SLOT_ID=");
   if (p_field == NULL)
   {      
      m_NCS_CONS_PRINTF("\nERROR:Problem in slot_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   if (sscanf(p_field + strlen("SLOT_ID="), "%d", &ifsv_info->slot_no) != 1)
   {      
      m_NCS_CONS_PRINTF("\nERROR:Problem in slot_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
  /* embedding subslot changes */
   p_field = ifd_search_argv_list(argc, argv, "NODE_ID=");
   if (p_field == NULL)
   {
      m_NCS_CONS_PRINTF("\nERROR:Problem in node_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   if (sscanf(p_field + strlen("NODE_ID="), "%d", &node_id) != 1)
   {
      m_NCS_CONS_PRINTF("\nERROR:Problem in node_id argument\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   ifsv_info->subslot_no = (node_id & 0x0f);
   /* Since the Pool Id we need to have it as constant */
   ifsv_info->pool_id   = NCS_HM_POOL_ID_COMMON;
   ifsv_info->vrid      = m_IFSV_DEF_VRID;
   ifsv_info->comp_type = IFSV_COMP_IFD;
   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifd_search_argv_list
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
static char *ifd_search_argv_list(int argc, char *argv[], char *arg_prefix)
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
