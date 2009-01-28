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




  DESCRIPTION:
   The DTSv Service Policy table.
  ..............................................................................

  FUNCTIONS INCLUDED in this module:
  dtsv_svc_policy_table_register       - Register Service Policy Table.
  dtsv_populate_svc_policy_tbl_info    - Service Policy Table Info.
  dtsv_populate_svc_policy_var_info    - Service Policy Table var info.
  ncsdtsvservicelogpolicyentry_set     - Service policy set operation.
  ncsdtsvservicelogpolicyentry_get     - Service Policy get operation.
  ncsdtsvservicelogpolicyentry_next    - Service Policy Next operation.
  ncsdtsvservicelogpolicyentry_setrow  - Service Policy set-row operation.
  ncsdtsvservicelogpolicyentry_rmvrow  - Service Policy rmv-row operation.
  ncsdtsvservicelogpolicyentry_extract - Service Policy extract operation.
  dtsv_svc_filtering_policy_change     - Service filtering policy change.
******************************************************************************/

#include "dts.h"

/* Local routines for this table */
static uns32 
dts_service_row_status_set(DTS_CB           *inst,
                           DTS_SVC_REG_TBL  *node);

static uns32 
dts_handle_service_param_set(DTS_CB           *inst,
                          DTS_SVC_REG_TBL  *node,
                          NCSMIB_PARAM_ID   paramid,
                          uns8              old_log_device,
                          uns32             old_buff_size);

/**************************************************************************
 Function: dtsv_svc_policy_table_register
 Purpose:  Service Policy table register routine
 Input:    void
 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
uns32 dtsv_svc_policy_table_register(void)
{
   uns32             rc = NCSCC_RC_SUCCESS;

   rc = ncsdtsvservicelogpolicyentry_tbl_reg();

   return rc;  
}

/**************************************************************************
* Function: ncsdtsvservicelogpolicyentry_set 
* Purpose:  Service Policy table's set object routine
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer
*           var_info : varinfo pointer
*           flag     : Tells the operation to be performed           
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 
ncsdtsvservicelogpolicyentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                  NCSMIB_VAR_INFO *var_info, NCS_BOOL flag)
{
   DTS_CB               *inst = (DTS_CB *)cb;
   DTS_SVC_REG_TBL      *service;
   SVC_KEY               key, nt_key;
   NCS_BOOL              sameval = FALSE,
                         create = FALSE;   
   NCSMIBLIB_REQ_INFO    reqinfo;
   NCSMIB_SET_REQ        *set_req = &arg->req.info.set_req;
   uns32                rc = NCSCC_RC_SUCCESS, old_buff_size = 0;
   uns8                  old_log_device = 0;

   m_NCS_OS_MEMSET(&reqinfo, 0, sizeof(reqinfo));
   m_NCS_OS_MEMSET(&key, 0, sizeof(SVC_KEY));
   m_NCS_OS_MEMSET(&nt_key, 0, sizeof(SVC_KEY));
    
   /* Validate the index length */
   if(SVC_INST_ID_LEN != arg->i_idx.i_inst_len)
      return NCSCC_RC_NO_INSTANCE;

   key.node = arg->i_idx.i_inst_ids[0];
   key.ss_svc_id = arg->i_idx.i_inst_ids[1];
   /* IR 60411 - Network order key added */
   nt_key.node = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[0]);
   nt_key.ss_svc_id = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[1]);

 
   service = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&inst->svc_tbl,(const uns8*)&nt_key);
   if(service == NULL)
      create = TRUE;

   if(ncsDtsvServiceRowStatus_ID == set_req->i_param_val.i_param_id)
   {
      /* Do the Row Status validation, if PSSv is not playing-back. */
      if((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
      {
         reqinfo.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP;
         reqinfo.info.i_val_status_util_info.row_status = 
            &set_req->i_param_val.info.i_int;

         if ((service != NULL) && (service->row_exist == FALSE))
             reqinfo.info.i_val_status_util_info.row = (NCSCONTEXT)NULL;
         else
             reqinfo.info.i_val_status_util_info.row = (NCSCONTEXT)service;

         rc = ncsmiblib_process_req(&reqinfo);
         if(NCSCC_RC_SUCCESS != rc) return rc;
      }
      else
      {
         /* If PSS is playing back row status to DTS, set the rowstatus 
          * accordingly.
          */
         switch(set_req->i_param_val.info.i_int)
         {
         case NCSMIB_ROWSTATUS_CREATE_GO:
            set_req->i_param_val.info.i_int = NCSMIB_ROWSTATUS_ACTIVE;
            break;
         case NCSMIB_ROWSTATUS_CREATE_WAIT:
            set_req->i_param_val.info.i_int = NCSMIB_ROWSTATUS_NOTINSERVICE;
            break;
         default:
            break;
         }
      }  
   }

   if(flag)
      return rc; 

   if(create)
   {
       rc = dts_create_new_pat_entry(inst, &service, key.node, key.ss_svc_id, NCS_SNMP_FALSE);
       if(rc != NCSCC_RC_SUCCESS)
       {
          /*log ERROR*/
          ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", DTS_EV_SVC_REG_ENT_ADD_FAIL, key.ss_svc_id, key.node, 0);
          ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", DTS_SET_FAIL, key.ss_svc_id, key.node, 0);
          return rc;
       }
       /* Smik - Send Async add update to stby */
       m_DTSV_SEND_CKPT_UPDT_ASYNC(inst, NCS_MBCSV_ACT_ADD, (MBCSV_REO_HDL)(long)service, DTSV_CKPT_DTS_SVC_REG_TBL_CONFIG);
   }

   /* DTS_PSSV integration - Check for BITS type MIBs.
    *                        If yes, then don't call miblib_req, call
    *                        dts specific function instead.
    */
   switch(set_req->i_param_val.i_param_id)
   {
      case ncsDtsvServiceLogDevice_ID:
          old_log_device = service->svc_policy.log_dev;
          /* IR 59525 - Check for valid value of log device */
          /* IR 60732-First check for null i_oct for the above-mentined check*/
          if(set_req->i_param_val.info.i_oct == NULL)
             return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "ncsdtsvservicelogpolicyentry_set: NULL pointer for i_oct in SET request received");
          else if(*(uns8*)set_req->i_param_val.info.i_oct < DTS_LOG_DEV_VAL_MAX)
          {
             if((*(uns8*)set_req->i_param_val.info.i_oct == 0) && 
                (old_log_device != 0))
                m_LOG_DTS_EVT(DTS_LOG_DEV_NONE, service->my_key.ss_svc_id, service->my_key.node, 0);
             else if(*(uns8*)set_req->i_param_val.info.i_oct != 0)
                return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_node_policy_set_obj: Log Device bit map beyond permissible values");
          }

      case ncsDtsvServiceCategoryBitMap_ID:
      case ncsDtsvServiceSeverityBitMap_ID:
          /* Call DTS's own SET functionality for BITS datatype */
          rc = dtsv_svc_policy_set_oct(cb, arg, &service->svc_policy);
          /* Jump to dts_handle_service_param_set, no need to call miblib_req */
          goto post_set;

      case ncsDtsvServiceCircularBuffSize_ID:
          old_buff_size = service->svc_policy.cir_buff_size;
          break;
      default:
          /* Do nothing*/
          break;
   }

   /* DO the SET operation */         
   reqinfo.req = NCSMIBLIB_REQ_SET_UTIL_OP;
   reqinfo.info.i_set_util_info.data = (NCSCONTEXT)service;
   reqinfo.info.i_set_util_info.param = &set_req->i_param_val;
   reqinfo.info.i_set_util_info.same_value = &sameval;   
   reqinfo.info.i_set_util_info.var_info = var_info;   
   rc = ncsmiblib_process_req(&reqinfo); 

post_set:   
   if(NCSCC_RC_SUCCESS != rc) 
   {
      ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", DTS_SET_FAIL, key.ss_svc_id, key.node, 0);
      return rc;
   }
 
   if(service != NULL)
      service->row_exist = TRUE;
 
   /* Do not proceed further if value to be set is same as that of the 
    * already existing value 
   */
   if(sameval)
      return rc;   

   /* Do processing specific to the param objects after set */
   rc = dts_handle_service_param_set(inst, service, set_req->i_param_val.i_param_id,
          old_log_device, old_buff_size);
   
   return rc;   
}

/**************************************************************************
* Function: ncsdtsvservicelogpolicyentry_get 
* Purpose:  Service Policy table's get object routine
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer*        
*           data     : Pointer of the data structure   
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 
ncsdtsvservicelogpolicyentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
   DTS_CB              *inst = (DTS_CB *)cb;
   SVC_KEY              nt_key;
   DTS_SVC_REG_TBL     *service;
  
   /* Validate the index length */
   if(SVC_INST_ID_LEN != arg->i_idx.i_inst_len)
      return NCSCC_RC_NO_INSTANCE;

   /* IR 60411 - Network order key added */
   nt_key.node = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[0]);
   nt_key.ss_svc_id = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[1]);

   if (((service = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&inst->svc_tbl, 
       (const uns8*)&nt_key)) == NULL) ||
       (service->row_exist == FALSE))
   {
      return NCSCC_RC_NO_INSTANCE;
   }
   /* Set the GET data pointer */
   *data = (NCSCONTEXT)service;
   return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: ncsdtsvservicelogpolicyentry_next 
* Purpose:  Service Policy table's next object routine
* Input:    cb    : DTS_CB pointer
*           arg   : Mib arg
*           data  : Pointer to the data structure
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 
ncsdtsvservicelogpolicyentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data, 
                            uns32 *next_inst_id, uns32 *next_inst_len)
{
   DTS_CB              *inst = (DTS_CB *)cb;
   SVC_KEY              nt_key;
   DTS_SVC_REG_TBL     *service;

   /* Validate the index length */
   /* IR 60411 - Network order key added */
   if(0 == arg->i_idx.i_inst_len)
   {
       nt_key.node = 0;
       nt_key.ss_svc_id = 0;
   }
   else if(1 == arg->i_idx.i_inst_len)
   {
       nt_key.node = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[0]);
       nt_key.ss_svc_id = 0;
   }
   else if(SVC_INST_ID_LEN == arg->i_idx.i_inst_len)
   {
       nt_key.node = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[0]);
       nt_key.ss_svc_id = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[1]);
   }
   else
      return NCSCC_RC_NO_INSTANCE;

   while ((service = (DTS_SVC_REG_TBL *)dts_get_next_svc_entry(&inst->svc_tbl, &nt_key)) != NULL)
   {
       if (service->row_exist == TRUE)
           break;

       nt_key.node = service->ntwk_key.node;
       nt_key.ss_svc_id = service->ntwk_key.ss_svc_id;
   }

   if (service == NULL)
       return NCSCC_RC_NO_INSTANCE;

   /* IR 60411 - Don't use ntwk_key, because htonl will be done on GET for the
    *            oids returned 
    */
   next_inst_id[0]  = service->my_key.node;
   next_inst_id[1]  = service->my_key.ss_svc_id;
   *next_inst_len  = 2;
   /* Set the GET data pointer */
   *data = (NCSCONTEXT)service;

   return NCSCC_RC_SUCCESS;   
}

/**************************************************************************
 Function: ncsdtsvservicelogpolicyentry_setrow 
 Purpose:  Service Policy table's setrow routine
 Input:    cb       : DTS_CB pointer
           arg      : Mib arg pointer
           params   : Poinet to param structure
           objinfo  : Pointer to object info structure
           flag     : Flase to perform setrow operation other wise testrow
 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
uns32 
ncsdtsvservicelogpolicyentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *arg,
                     NCSMIB_SETROW_PARAM_VAL *params,
                     struct ncsmib_obj_info *objinfo,
                     NCS_BOOL flag)
{
   DTS_CB               *inst = (DTS_CB *)cb;
   DTS_SVC_REG_TBL      *service;
   SVC_KEY               key, nt_key;
   NCS_BOOL              sameval = FALSE,
                         create = FALSE;   
   NCSMIBLIB_REQ_INFO    reqinfo;
   uns32                 paramid = 0, old_buff_size = 0,
                         rc = NCSCC_RC_SUCCESS;
   uns8                  old_log_device = 0;

   m_NCS_OS_MEMSET(&reqinfo, 0, sizeof(reqinfo));
   m_NCS_OS_MEMSET(&key, 0, sizeof(SVC_KEY));
   m_NCS_OS_MEMSET(&nt_key, 0, sizeof(SVC_KEY));

   /* Validate the index length */
   if(SVC_INST_ID_LEN != arg->i_idx.i_inst_len)
      return NCSCC_RC_NO_INSTANCE;

   key.node = arg->i_idx.i_inst_ids[0];
   key.ss_svc_id = arg->i_idx.i_inst_ids[1];
   /* IR 60411 - Network order key added */
   nt_key.node = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[0]);
   nt_key.ss_svc_id = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[1]);

   service = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&inst->svc_tbl,(const uns8*)&nt_key);
   if(service == NULL)
      create = TRUE;
 
   if(params[ncsDtsvServiceRowStatus_ID - 1].set_flag)
   {
      /* Do the Row Status validation, if PSSv is not playing-back. */
      if((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
      {
         reqinfo.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP;
         reqinfo.info.i_val_status_util_info.row_status = 
            &params[ncsDtsvServiceRowStatus_ID - 1].param.info.i_int;
         if ((service != NULL) && (service->row_exist == FALSE))
             reqinfo.info.i_val_status_util_info.row = (NCSCONTEXT)NULL;
         else
             reqinfo.info.i_val_status_util_info.row = (NCSCONTEXT)service;
         rc = ncsmiblib_process_req(&reqinfo);
         if(NCSCC_RC_SUCCESS != rc) return rc;
      }
      else
      {
         /* If PSSv is playing back, set rowstatus accordingly */
         switch(params[ncsDtsvServiceRowStatus_ID - 1].param.info.i_int)
         {
         case NCSMIB_ROWSTATUS_CREATE_GO:
            params[ncsDtsvServiceRowStatus_ID - 1].param.info.i_int = NCSMIB_ROWSTATUS_ACTIVE;
            break;
         case NCSMIB_ROWSTATUS_CREATE_WAIT:
            params[ncsDtsvServiceRowStatus_ID - 1].param.info.i_int = NCSMIB_ROWSTATUS_NOTINSERVICE;
            break;
         default:
            break;
         } 
      }
   }

   if(flag)
      return rc;

   if(create)
   {
      rc = dts_create_new_pat_entry(inst, &service, key.node, key.ss_svc_id, NCS_SNMP_FALSE);
      if(rc != NCSCC_RC_SUCCESS)
      {
         /*log ERROR*/
         ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", DTS_EV_SVC_REG_ENT_ADD_FAIL, key.ss_svc_id, key.node, 0);
         ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", DTS_SET_FAIL, key.ss_svc_id, key.node, 0);
         return rc;
      }
      /* Smik - Send Async add update to stby */
      m_DTSV_SEND_CKPT_UPDT_ASYNC(inst, NCS_MBCSV_ACT_ADD, (MBCSV_REO_HDL)(long)service, DTSV_CKPT_DTS_SVC_REG_TBL_CONFIG);
   }

   if(service != NULL)
      service->row_exist = TRUE;

   /* Do Set Row Operation Now */
   for (paramid = ncsDtsvServiceIndexNode_ID; paramid < ncsDtsvServiceLogPolicyEntryMax_ID; paramid++)
   {
      if(params[paramid-1].set_flag)
      {
         switch(paramid)
         {
            /* DTS_PSSV integration - Check for BITS type MIBs.
             *                        If yes, then don't call miblib_req, call
             *                        dts specific function instead.
             */
            case ncsDtsvServiceLogDevice_ID:
               old_log_device = service->svc_policy.log_dev;
               /* IR 59525 - Check for valid value of log device */
               /* IR 60732-First check for null i_oct for the above-mentined check*/
               if(params[paramid-1].param.info.i_oct == NULL)
                  return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "ncsdtsvservicelogpolicyentry_setrow: NULL pointer for i_oct in SETROW request received");
               else if(*(uns8*)params[paramid-1].param.info.i_oct < DTS_LOG_DEV_VAL_MAX)
               {
                  if((*(uns8*)params[paramid-1].param.info.i_oct == 0) &&
                     (old_log_device != 0))
                     m_LOG_DTS_EVT(DTS_LOG_DEV_NONE, service->my_key.ss_svc_id, service->my_key.node, 0);
                  else if(*(uns8*)params[paramid-1].param.info.i_oct != 0)
                     return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_setrow_svc_policy_obj: Log Device bit map beyond permissible values");
               }

            case ncsDtsvServiceCategoryBitMap_ID:
            case ncsDtsvServiceSeverityBitMap_ID:
               m_NCS_MEMSET(&arg->req.info.set_req.i_param_val, '\0', sizeof(NCSMIB_PARAM_VAL));
               /* Fill NCSMIB_ARG according to params struct before calling*/
               m_NCS_MEMCPY(&arg->req.info.set_req.i_param_val, &params[paramid-1].param, sizeof(NCSMIB_PARAM_VAL));
               /* Call DTS's own SET function for BITS MIB type */
               rc = dtsv_svc_policy_set_oct(cb, arg, &service->svc_policy);
               break;

            case ncsDtsvServiceCircularBuffSize_ID:
               old_buff_size = service->svc_policy.cir_buff_size;
            default:
               sameval = FALSE;
               reqinfo.req = NCSMIBLIB_REQ_SET_UTIL_OP;
               reqinfo.info.i_set_util_info.data = (NCSCONTEXT)service;
               reqinfo.info.i_set_util_info.param = &params[paramid-1].param;
               reqinfo.info.i_set_util_info.same_value = &sameval;
               reqinfo.info.i_set_util_info.var_info = &objinfo->var_info[paramid-1];   
               rc = ncsmiblib_process_req(&reqinfo);
               break;
         } 

         if(NCSCC_RC_SUCCESS != rc) 
         {
            ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", DTS_SET_FAIL, key.ss_svc_id, key.node, 0);
            return rc;
         }

         /* Do not proceed further if value to be set is same as that of the 
          * already existing value 
          */
         if(sameval)
            continue;   

         /* Do processing specific to the param objects after set */
         rc = dts_handle_service_param_set(inst, service, paramid,
          old_log_device, old_buff_size);
      }
   }        
   return rc;
}


/**************************************************************************
 Function: dtsv_svc_filtering_policy_change

 Purpose:  Change in the Node Filtering policy will affect the service.
         Also, policy change will be sent to all the services.

 Input:    cb       : DTS_CB pointer
           param_id : Parameter for which change is done.
         node_id  : Node id of the node.
         svc_id   : Service ID of the service to configure.

 Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

 Notes:  
**************************************************************************/
uns32
dtsv_svc_filtering_policy_change(DTS_CB *inst, DTS_SVC_REG_TBL  *service, 
                                 NCSMIB_PARAM_ID param_id, 
                                 uns32 node_id, SS_SVC_ID svc_id)
{
   DTA_DEST_LIST        *dta;
   DTA_ENTRY            *dta_entry;   
   /* Configure services for this change */
   dta_entry = service->v_cd_list;
   while(dta_entry != NULL)
   {
      dta = dta_entry->dta;
      dts_send_filter_config_msg(inst, service, dta);
      dta_entry = dta_entry->next_in_svc_entry; 
   }

   return NCSCC_RC_SUCCESS;
}

/**************************************************************************
 Function: dts_service_row_status_set

 Purpose:  Function used for handeling the row status changes. 

 Input:    inst      : DTS control block.
           node      : Registration table entry
        
 Returns:  None.

 Notes:  
**************************************************************************/
static uns32 
dts_service_row_status_set(DTS_CB           *inst,
                           DTS_SVC_REG_TBL  *service)
{
    OP_DEVICE *dev = &service->device; 
    /*
     * If row status is changed to destroyed, we will have to remove 
     * patritia tree entry provided none of the service registered 
     * with that service and node id.
     */
    if (service->row_status == NCSMIB_ROWSTATUS_DESTROY)
    {
        dts_log_policy_change(service, &service->svc_policy, &inst->dflt_plcy.svc_dflt.policy);
        /* IR 60427 - Changing the filtering policy as well. don't return
         *            Print a DBG SINK. Rowstatus deletion must occur 
         */
        dts_filter_policy_change(service, &service->svc_policy, &inst->dflt_plcy.svc_dflt.policy);
    
        if (service->dta_count == 0)
        {
            /* IR 61143 - No need of policy handles */
            /*ncshm_destroy_hdl(NCS_SERVICE_ID_DTSV, service->svc_hdl);*/
            dts_circular_buffer_free(&dev->cir_buffer);
            ncs_patricia_tree_del(&inst->svc_tbl, (NCS_PATRICIA_NODE *)service);
            /* Cleanup the DTS_FILE_LIST datastructure for svc */
            m_DTS_FREE_FILE_LIST(dev);
            /* Cleanup the console devices associated with the node */
            m_DTS_RMV_ALL_CONS(dev);
            /* Send RMV updt here itself, cuz service is going to be deleted */
            m_DTSV_SEND_CKPT_UPDT_ASYNC(inst, NCS_MBCSV_ACT_RMV, (MBCSV_REO_HDL)(long)service, DTSV_CKPT_DTS_SVC_REG_TBL_CONFIG);

            if (NULL != service)
                m_MMGR_FREE_SVC_REG_TBL(service);
            /* Set service to NULL, since service would be used later in
             * dts_handle_service_param_set() 
             */
            service = NULL;
        }
        else
        {
            /* We are here means service registration is present on the node
             * So just change the row status and set row_exist to flase. */
            service->row_status = NCSMIB_ROWSTATUS_NOTINSERVICE;
            service->row_exist = FALSE;
        }
    }
    else if (service->row_status == NCSMIB_ROWSTATUS_ACTIVE)
    {
        dts_log_policy_change(service, &inst->dflt_plcy.svc_dflt.policy, &service->svc_policy);
        dts_filter_policy_change(service, &inst->dflt_plcy.svc_dflt.policy, &service->svc_policy);
    }
    else
    {
        dts_log_policy_change(service, &service->svc_policy, &inst->dflt_plcy.svc_dflt.policy);
        dts_filter_policy_change(service, &service->svc_policy, &inst->dflt_plcy.svc_dflt.policy);
    }

    return NCSCC_RC_SUCCESS;
}

/**************************************************************************
 Function: dts_handle_service_param_set

 Purpose:  Function used for handeling node parameter changes. 

 Input:    inst      : DTS control block.
           node      : Registration table entry
        
 Returns:  None.

 Notes:  
**************************************************************************/
static uns32 
dts_handle_service_param_set(DTS_CB           *inst,
                          DTS_SVC_REG_TBL     *service,
                          NCSMIB_PARAM_ID      paramid,
                          uns8                 old_log_device,
                          uns32                old_buff_size)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_BOOL  destroy_op = FALSE;

   switch(paramid)
   {
   case ncsDtsvServiceRowStatus_ID:
      /* Handle row status change */
      /*Set cli_bit_map to paramid incase svc entry is deleted in 
       * dts_service_row_status_set() */ 
      inst->cli_bit_map = paramid;
      if(service->row_status == NCSMIB_ROWSTATUS_DESTROY)
         destroy_op = TRUE;
      rc = dts_service_row_status_set(inst, service);         
      break;

   case ncsDtsvServiceLogDevice_ID:
      /* Handle log device change */
      if(service->row_status == NCSMIB_ROWSTATUS_ACTIVE) 
         rc = dts_log_device_set(&service->svc_policy, &service->device, old_log_device); 
      break;

   case ncsDtsvServiceLogFileSize_ID:
      break;

   case ncsDtsvServiceFileLogCompFormat_ID:
      if(service->row_status == NCSMIB_ROWSTATUS_ACTIVE)
         service->device.file_log_fmt_change = TRUE; /*TBD */
      break;

   case ncsDtsvServiceCircularBuffSize_ID:
       /* Handle Circular buffer size change */
       if(service->row_status == NCSMIB_ROWSTATUS_ACTIVE)
          rc = dts_buff_size_set(&service->svc_policy, 
                                 &service->device, old_buff_size);
      break;

   case ncsDtsvServiceCirBuffCompFormat_ID:
      if(service->row_status == NCSMIB_ROWSTATUS_ACTIVE)
         service->device.buff_log_fmt_change = TRUE; /*TBD */
      break;

   case ncsDtsvServiceCategoryBitMap_ID:
       if(service->row_status == NCSMIB_ROWSTATUS_ACTIVE)
       {
          service->svc_policy.category_bit_map = ntohl(service->svc_policy.category_bit_map);
          /* Filtering policy is changed. So send the message to all 
           * DTA's and tell them the correct filtering policies */
           rc = dtsv_svc_filtering_policy_change(inst, service, 
                paramid, service->my_key.node, service->my_key.ss_svc_id);
       }
      break;

   case ncsDtsvServiceLoggingState_ID:
   case ncsDtsvServiceSeverityBitMap_ID:
       /* Filtering policy is changed. So send the message to all 
        * DTA's and tell them the correct filtering policies */
       if(service->row_status == NCSMIB_ROWSTATUS_ACTIVE) 
          rc = dtsv_svc_filtering_policy_change(inst, service, 
             paramid, service->my_key.node, service->my_key.ss_svc_id);
      break;

   default:
      break;
   }      

   /* Smik - Send Async update for DTS_SVC_REG_TBL to stby DTS */
   if((rc != NCSCC_RC_FAILURE) && (service != NULL) && (destroy_op != TRUE))
   {
      m_DTSV_SEND_CKPT_UPDT_ASYNC(inst, NCS_MBCSV_ACT_UPDATE, (MBCSV_REO_HDL)(long)service, DTSV_CKPT_DTS_SVC_REG_TBL_CONFIG);
   }

   /* Re-set cli_bit_map */
   inst->cli_bit_map = 0;

   return rc;
}

/**************************************************************************
* Function: dtsv_service_conf_console
* Purpose:  Service device's console configuration
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer
*           flag     : Tells the operation to be performed.
*                      TRUE is add, FALSE is remove.
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 dtsv_service_conf_console(DTS_CB *cb, NCSMIB_ARG *arg, NCS_BOOL flag)
{
   OP_DEVICE *dev = NULL;
   uns8       bit_map = 0;
   uns16      str_len;
   NCS_UBAID  uba;
   USRBUF     *buf = arg->req.info.cli_req.i_usrbuf;
   uns8       data_buff[DTSV_CLI_MAX_SIZE]="", *buf_ptr = NULL;
   uns32      node_id, svc_id, rc = NCSCC_RC_SUCCESS;
   SVC_KEY    nt_key;
   DTS_SVC_REG_TBL *svc_ptr;

   m_NCS_MEMSET(&uba, '\0', sizeof(uba));

   ncs_dec_init_space(&uba, buf);
   arg->req.info.cli_req.i_usrbuf = NULL;

   if(flag == TRUE)
      buf_ptr = ncs_dec_flatten_space(&uba, data_buff, DTSV_ADD_SVC_CONS_SIZE);
   else
      buf_ptr = ncs_dec_flatten_space(&uba, data_buff, DTSV_RMV_SVC_CONS_SIZE);

   if(buf_ptr == NULL)
      return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: ncs_dec_flatten_space returns NULL");

   /* Decode the severity bit_map, node_id, svc_id & strlen passed */
   if(flag == TRUE)
   {
      bit_map = ncs_decode_8bit(&buf_ptr);
      ncs_dec_skip_space(&uba, sizeof(uns8));
   }
   node_id = ncs_decode_32bit(&buf_ptr);
   ncs_dec_skip_space(&uba, sizeof(uns32));
   svc_id =  ncs_decode_32bit(&buf_ptr);
   ncs_dec_skip_space(&uba, sizeof(uns32));
   str_len = ncs_decode_8bit(&buf_ptr);
   ncs_dec_skip_space(&uba, sizeof(uns8));

   /* Now decode console device, to be kept in data_buff array */
   if(ncs_decode_n_octets_from_uba(&uba, (char*)&data_buff, str_len) != NCSCC_RC_SUCCESS)
   {
      return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: ncs_decode_n_octets_from_uba failed");
   }

   /* Find the node frm patricia tree */
   /* IR 60411 - Network order key added */
   nt_key.node = m_NCS_OS_HTONL(node_id);
   nt_key.ss_svc_id = m_NCS_OS_HTONL(svc_id);
   if((svc_ptr = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl, (const uns8*)&nt_key)) == NULL)
   {
      return NCSCC_RC_INV_SPECIFIC_VAL;
   }
   /* Get the device associated with the node */
   dev = &svc_ptr->device;

   if(flag == TRUE)
      /* Now add the device to global device */
      m_DTS_ADD_CONS(cb, dev, (char *)&data_buff, bit_map)
   else
   {
      if(m_NCS_STRCMP((char *)&data_buff, "all") == 0)
         m_DTS_RMV_ALL_CONS(dev)
      else
         m_DTS_RMV_CONS(cb, dev, (char *)&data_buff);
   }
   return rc; 
}

/**************************************************************************
* Function: dtsv_service_disp_conf_console
* Purpose:  Displays node's currently configured consoles
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer
*
* Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 dtsv_service_disp_conf_console(DTS_CB *cb, NCSMIB_ARG *arg)
{
   OP_DEVICE *dev;
   uns8       str_len, bit_map;
   NCS_UBAID  rsp_uba, uba;
   USRBUF     *buf = arg->req.info.cli_req.i_usrbuf;
   DTS_CONS_LIST *cons_ptr;
   uns8       *buff_ptr = NULL, def_num_cons=1;
   uns8       data_buff[DTSV_CLI_MAX_SIZE]="", *dec_ptr = NULL;
   uns32      count;
   SVC_KEY    nt_key;
   DTS_SVC_REG_TBL *node_ptr;

   m_NCS_MEMSET(&rsp_uba, '\0', sizeof(rsp_uba));
   m_NCS_MEMSET(&uba, '\0', sizeof(uba));

   /* Set parameters for response */
   arg->rsp.info.cli_rsp.i_cmnd_id = dtsvDispNodeConsole;
   arg->rsp.info.cli_rsp.o_partial = FALSE;

   /* Decode the req to get the node_id */
   ncs_dec_init_space(&uba, buf);
   arg->req.info.cli_req.i_usrbuf = NULL;
 
   dec_ptr = ncs_dec_flatten_space(&uba, data_buff, 2*sizeof(uns32));
   if(dec_ptr == NULL)
      return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_node_conf_console: ncs_dec_flatten_space returns NULL");

   /* IR 60411 - Network order key added */
   nt_key.node = m_NCS_OS_HTONL(ncs_decode_32bit(&dec_ptr));
   ncs_dec_skip_space(&uba, sizeof(uns32));

   nt_key.ss_svc_id = m_NCS_OS_HTONL(ncs_decode_32bit(&dec_ptr));
   ncs_dec_skip_space(&uba, sizeof(uns32));

   /* Find the node in the patricia tree */
   if((node_ptr = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl, (const uns8*)&nt_key)) == NULL)
      return NCSCC_RC_FAILURE;

   dev = &node_ptr->device;
   cons_ptr = dev->cons_list_ptr;

   /* Now encode the console device */
   if(ncs_enc_init_space(&rsp_uba) != NCSCC_RC_SUCCESS)
      return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: userbuf init failed");

   m_BUFR_STUFF_OWNER(rsp_uba.start);

   if(cons_ptr == NULL)
   {
      /* encode number of devices(which is 1 in this case) */
      buff_ptr = ncs_enc_reserve_space(&rsp_uba, sizeof(uns8));
      if(buff_ptr == NULL)
         return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: reserve space for encoding failed");

      ncs_encode_8bit(&buff_ptr, def_num_cons);
      ncs_enc_claim_space(&rsp_uba, sizeof(uns8));

      /* Now encode the serial device associated with DTS_CB */
      str_len = m_NCS_STRLEN((char *)cb->cons_dev);
      bit_map = cb->g_policy.g_policy.severity_bit_map;

      buff_ptr = ncs_enc_reserve_space(&rsp_uba, DTSV_RSP_CONS_SIZE);
      if(buff_ptr == NULL)
         return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: reserve space for encoding failed");

      ncs_encode_8bit(&buff_ptr, bit_map);
      ncs_encode_8bit(&buff_ptr, str_len);
      ncs_enc_claim_space(&rsp_uba, DTSV_RSP_CONS_SIZE);

      if(ncs_encode_n_octets_in_uba(&rsp_uba, (char *)cb->cons_dev,
                                     str_len) != NCSCC_RC_SUCCESS)
         return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: encode_n_octets_in_uba for serial console failed");

      /* Add uba to rsp's usrbuf */
      arg->rsp.info.cli_rsp.o_answer = rsp_uba.start;
   }/*end of if cons_ptr==NULL*/
   else
   {
      /* Encode number of console devices */
      buff_ptr = ncs_enc_reserve_space(&rsp_uba, sizeof(uns8));
      if(buff_ptr == NULL)
         return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: reserve space for encoding failed");

      ncs_encode_8bit(&buff_ptr, dev->num_of_cons_conf);
      ncs_enc_claim_space(&rsp_uba, sizeof(uns8));

      /* Now encode individual console devices */
      for(count = 0; count < dev->num_of_cons_conf; count++)
      {
         /* Encode the serial device associated with global policy */
         str_len = m_NCS_STRLEN(cons_ptr->cons_dev);
         bit_map = cons_ptr->cons_sev_filter;
         buff_ptr = ncs_enc_reserve_space(&rsp_uba, DTSV_RSP_CONS_SIZE);
         if(buff_ptr == NULL)
            return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: reserve space for encoding failed");

         ncs_encode_8bit(&buff_ptr, bit_map);
         ncs_encode_8bit(&buff_ptr, str_len);
         ncs_enc_claim_space(&rsp_uba, DTSV_RSP_CONS_SIZE);

         if(ncs_encode_n_octets_in_uba(&rsp_uba, cons_ptr->cons_dev, str_len) != NCSCC_RC_SUCCESS)
            return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: encode_n_octets_in_uba for console failed");

         /* Point to next device */
         cons_ptr = cons_ptr->next;
      }/*end of for*/

      /* Add uba to rsp's usrbuf */
      arg->rsp.info.cli_rsp.o_answer = rsp_uba.start;
   } /*end of else*/

   return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
 * Function: ncsdtsvservicelogpolicyentry_extract 
 *
 * Purpose:  This function is one of the RMVROW processing routines for objects
 * in DTSV_SCALARS_ID table.
 *
 * Input:  cb        - DTS control block.
 *         idx       - pointer to NCSMIB_IDX
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 **************************************************************************/
uns32 ncsdtsvservicelogpolicyentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx)
{
   /* Assign defaults for your scalars here. */
   return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: ncsdtsvservicelogpolicyentry_extract 
* Purpose:  Policy table's extract object value routine
* Input:    param_val: Param pointer
*           var_info : var_info pointer
*           data     : Pointer to the data structure
*           buffer   : Buffer to get the octect data
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 ncsdtsvservicelogpolicyentry_extract(NCSMIB_PARAM_VAL *param_val,
                             NCSMIB_VAR_INFO *var_info,
                             NCSCONTEXT data,
                             NCSCONTEXT buffer)
{
    if((param_val->i_param_id == ncsDtsvServiceLogDevice_ID) || (param_val->i_param_id == ncsDtsvServiceCategoryBitMap_ID) || (param_val->i_param_id == ncsDtsvServiceSeverityBitMap_ID))
    {
       return dtsv_svc_extract_oct(param_val, var_info, data, buffer);
    }
    else
    {
       return dtsv_extract_policy_obj(param_val, var_info, data, buffer);
    }

}

/**************************************************************************
* Function: dtsv_svc_policy_set_oct 
* Purpose:  SET operation for MIBS of type BITS for service policies.
*           
* Input:    cb       : DTS_CB pointer
*           arg      : NCSMIB_ARG pointer
*           policy   : POLICY pointer
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 dtsv_svc_policy_set_oct(DTS_CB *cb, NCSMIB_ARG *arg,
                               POLICY *policy)
{
   NCSMIB_PARAM_VAL *param_val = &arg->req.info.set_req.i_param_val;
   NCSCONTEXT  data = NULL;

   if(param_val->i_param_id == ncsDtsvServiceCategoryBitMap_ID)
       data = &policy->category_bit_map;
   else if(param_val->i_param_id == ncsDtsvServiceSeverityBitMap_ID)
       data = &policy->severity_bit_map;
   else if(param_val->i_param_id == ncsDtsvServiceLogDevice_ID)
       data = &policy->log_dev;

   return dtsv_policy_set_oct(arg, data);
}

/**************************************************************************
* Function: dtsv_svc_extract_oct 
* Purpose:  EXTRACT operation for MIBS of type BITS for service policies.
*           
* Input:    param_val    : NCSMIB_PARAM_VAL pointer
*           data         : DTS_SVC_REG_TBL pointer
*           buffer       : Buffer pointer
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 dtsv_svc_extract_oct(NCSMIB_PARAM_VAL *param_val, 
                           NCSMIB_VAR_INFO *var_info, NCSCONTEXT data,
                           NCSCONTEXT buffer)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   DTS_SVC_REG_TBL *svc = (DTS_SVC_REG_TBL *)data;
   uns32  tmp, nworder;

   param_val->i_fmat_id = var_info->fmat_id;
  
   switch(param_val->i_param_id)
   {
      case ncsDtsvServiceLogDevice_ID:
         param_val->i_length = sizeof(uns8); 
         /*m_NCS_OS_MEMCPY((uns8*)&tmp, &svc->svc_policy.log_dev, param_val->i_length);*/
         param_val->info.i_oct = (uns8*)&svc->svc_policy.log_dev;
         break;

      case ncsDtsvServiceCategoryBitMap_ID:
         param_val->i_length = sizeof(uns32);
         m_NCS_OS_MEMCPY((uns8*)&tmp, &svc->svc_policy.category_bit_map, param_val->i_length);
         nworder = htonl(tmp);
         m_NCS_OS_MEMCPY((uns8*)buffer, (uns8*)&nworder, param_val->i_length);
         param_val->info.i_oct = (uns8*)buffer;
         break;

      case ncsDtsvServiceSeverityBitMap_ID:
          param_val->i_length = sizeof(uns8);
          /*m_NCS_OS_MEMCPY((uns8*)&tmp, &svc->svc_policy.severity_bit_map, param_val->i_length);*/
          param_val->info.i_oct = (uns8*)&svc->svc_policy.severity_bit_map;
         break;

      default:
          return NCSCC_RC_FAILURE;
   }

   /*nworder = htonl(tmp);
   m_NCS_OS_MEMCPY((uns8*)buffer, (uns8*)&nworder, param_val->i_length);

   param_val->info.i_oct = (uns8*)buffer;*/

   return rc;
}

