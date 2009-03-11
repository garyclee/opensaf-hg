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
   The DTSv Node Policy table.
  ..............................................................................

  FUNCTIONS INCLUDED in this module:

  dtsv_node_policy_table_register       - Register Node Policy Table.
  dtsv_populate_node_policy_tbl_info    - Node Policy Table Info.
  dtsv_populate_node_policy_var_info    - Node Policy Table var info.
  ncsdtsvnodelogpolicyentry_set         - Node policy set operation.
  ncsdtsvnodelogpolicyentry_get         - Node Policy get operation.
  ncsdtsvnodelogpolicyentry_next        - Node Policy Next operation.
  ncsdtsvnodelogpolicyentry_setrow      - Node Policy set-row operation.
  ncsdtsvnodelogpolicyentry_rmvrow      - Node Policy remove row operation.
  ncsdtsvnodelogpolicyentry_extract     - Node Policy extract operation.
  dtsv_node_policy_change               - Node policy change.
  dts_log_policy_change                 - Handle Log policy change.

******************************************************************************/

#include "dts.h"


/* Local routines for this table */
static uns32
dtsv_node_policy_change(DTS_CB *inst, DTS_SVC_REG_TBL *node,
                                  NCSMIB_PARAM_ID param_id, 
                                  uns32 node_id);

static void 
dts_node_row_status_set(DTS_CB           *inst,
                        DTS_SVC_REG_TBL  *node);

static uns32 
dts_handle_node_param_set(DTS_CB           *inst,
                          DTS_SVC_REG_TBL  *node,
                          NCSMIB_PARAM_ID   paramid,
                          uns8              old_log_device,
                          uns32             old_buff_size);


/**************************************************************************
 Function: dtsv_node_policy_table_register
 Purpose:  Node Policy table register routine
 Input:    void
 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
uns32 dtsv_node_policy_table_register(void)
{
   uns32             rc = NCSCC_RC_SUCCESS;

   rc = ncsdtsvnodelogpolicyentry_tbl_reg();

   return rc;  
}

/**************************************************************************
* Function: ncsdtsvnodelogpolicyentry_set 
* Purpose:  Node Policy table's set object routine
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer
*           var_info : varinfo pointer
*           flag     : Tells the operation to be performed           
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 
ncsdtsvnodelogpolicyentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                  NCSMIB_VAR_INFO *var_info, NCS_BOOL flag)
{
   DTS_CB               *inst = (DTS_CB *)cb;
   DTS_SVC_REG_TBL       *node;
   NCS_BOOL              sameval = FALSE,
                         create = FALSE;   
   NCSMIBLIB_REQ_INFO    reqinfo;
   NCSMIB_SET_REQ        *set_req = &arg->req.info.set_req;
   uns32                rc = NCSCC_RC_SUCCESS, old_buff_size = 0;
   uns8                   log_device = 0;
   SVC_KEY                key, nt_key;

   memset(&reqinfo, 0, sizeof(reqinfo));
   memset(&key, 0, sizeof(SVC_KEY));
  
   /* Validate the index length */
   if(NODE_INST_ID_LEN != arg->i_idx.i_inst_len)
      return NCSCC_RC_NO_INSTANCE;

   if(flag)
      return rc;
 
   key.node = arg->i_idx.i_inst_ids[0];
   key.ss_svc_id = 0; 
   /*  Network order key added */
   nt_key.node = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[0]);
   nt_key.ss_svc_id = 0;
   
   node = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&inst->svc_tbl,(const uns8*)&nt_key);

   if(node == NULL)
      create = TRUE;

   if(ncsDtsvNodeRowStatus_ID == set_req->i_param_val.i_param_id)
   {
      /* Do the Row Status validation, if PSSv is not playing-back. */
      if((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
      {
         reqinfo.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP;
         reqinfo.info.i_val_status_util_info.row_status = 
            &set_req->i_param_val.info.i_int;
         if ((node != NULL) && (node->row_exist == FALSE))
            reqinfo.info.i_val_status_util_info.row = (NCSCONTEXT)NULL;
         else
            reqinfo.info.i_val_status_util_info.row = (NCSCONTEXT)node;
         rc = ncsmiblib_process_req(&reqinfo);
         if(NCSCC_RC_SUCCESS != rc) return rc;
      }
      else
      {
         /* If PSSv is playing back, set rowstatus accordingly */
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
       rc = dts_create_new_pat_entry(inst, &node, key.node, key.ss_svc_id, NODE_LOGGING);
       if(rc != NCSCC_RC_SUCCESS)
       {
          /*log ERROR*/
          ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", DTS_EV_SVC_REG_ENT_ADD_FAIL, key.ss_svc_id, key.node, 0);
          ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", DTS_SET_FAIL, key.ss_svc_id, key.node, 0);
          return rc; 
       }   
       /* Send Async add to stby DTS */
       m_DTSV_SEND_CKPT_UPDT_ASYNC(inst, NCS_MBCSV_ACT_ADD, (MBCSV_REO_HDL)(long)node, DTSV_CKPT_DTS_SVC_REG_TBL_CONFIG);
   }

   /* DTS_PSSV integration - Check for BITS type MIBs.
    *                        If yes, then don't call miblib_req, call
    *                        dts specific function instead.
    */
   switch(set_req->i_param_val.i_param_id)
   {
      case ncsDtsvNodeLogDevice_ID:
          log_device = node->svc_policy.log_dev;
          /* Check for valid value of log device */
          /* First check for null i_oct for the above-mentined check*/
          if(set_req->i_param_val.info.i_oct == NULL)
             return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "ncsdtsvnodelogpolicyentry_set: NULL pointer for i_oct in SET request received");
          else if(*(uns8*)set_req->i_param_val.info.i_oct < DTS_LOG_DEV_VAL_MAX)
          {
             if( (*(uns8*)set_req->i_param_val.info.i_oct == 0) &&
                 (log_device != 0))
               m_LOG_DTS_EVT(DTS_LOG_DEV_NONE, node->my_key.ss_svc_id, node->my_key.node, 0);
             else if(*(uns8*)set_req->i_param_val.info.i_oct != 0)
               return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_node_policy_set_obj: Log Device bit map beyond permissible values");
          }
 
      case ncsDtsvNodeCategoryBitMap_ID:
      case ncsDtsvNodeSeverityBitMap_ID:
          /* Call DTS's own SET functionality for BITS datatype */
          rc = dtsv_node_policy_set_oct(cb, arg, &node->svc_policy);
          /* Jump to dts_handle_node_param_set, no need to call miblib_req */
          goto post_set;

      case ncsDtsvNodeCircularBuffSize_ID:
          old_buff_size = node->svc_policy.cir_buff_size;
          break;

      default:
          /* Do nothing */
          break;
   }

   /* DO the SET operation */         
   reqinfo.req = NCSMIBLIB_REQ_SET_UTIL_OP;
   reqinfo.info.i_set_util_info.data = (NCSCONTEXT)node;
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

   if(node != NULL)
      node->row_exist = TRUE;
   
   /* Do not proceed further if value to be set is same as that of the 
    * already existing value 
    */
   if(sameval)
      return rc;

   /* Do processing specific to the param objects after set */
   rc = dts_handle_node_param_set(inst, node, set_req->i_param_val.i_param_id,
       log_device, old_buff_size);
   
   return rc;   
}

/**************************************************************************
* Function: ncsdtsvnodelogpolicyentry_get 
* Purpose:  Node Policy table's get object routine
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer*        
*           data     : Pointer of the data structure   
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 
ncsdtsvnodelogpolicyentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
   DTS_CB              *inst = (DTS_CB *)cb;
   DTS_SVC_REG_TBL      *node;
   SVC_KEY               nt_key;

   /* Validate the index length */
   if(NODE_INST_ID_LEN != arg->i_idx.i_inst_len)
      return NCSCC_RC_NO_INSTANCE;

   /*  Network order key added */
   nt_key.node = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[0]);
   nt_key.ss_svc_id = 0;

   if (((node = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&inst->svc_tbl, 
                (const uns8*)&nt_key)) == NULL) || (node->row_exist == FALSE))
   {
       return NCSCC_RC_NO_INSTANCE;
   }
   /* Set the GET data pointer */
   *data = (NCSCONTEXT)node;
   return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: ncsdtsvnodelogpolicyentry_next 
* Purpose:  Node Policy table's next object routine
* Input:    cb    : DTS_CB pointer
*           arg   : Mib arg
*           data  : Pointer to the data structure
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 
ncsdtsvnodelogpolicyentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data, 
                            uns32 *next_inst_id, uns32 *next_inst_len)
{
   DTS_CB              *inst = (DTS_CB *)cb;
   DTS_SVC_REG_TBL     *node;
   SVC_KEY             nt_key;

   /* Validate the index length */
   if (arg->i_idx.i_inst_len == 0)
   {
       nt_key.node = 0;
   }
   else if (arg->i_idx.i_inst_len == NODE_INST_ID_LEN)
   {
     /*  Network order key added */
       nt_key.node = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[0]);
   }
   else
      return NCSCC_RC_NO_INSTANCE;

   nt_key.ss_svc_id = 0;

   while ((node = (DTS_SVC_REG_TBL *)dts_get_next_node_entry(&inst->svc_tbl, &nt_key)) != NULL)
   {
       if (node->row_exist == TRUE)
           break;

       nt_key.node = node->ntwk_key.node;
   }

   if (node == NULL)
       return NCSCC_RC_NO_INSTANCE;

   /* Don't use ntwk_key, because htonl will be done on GET for the
    *            oids returned 
    */
   next_inst_id[0]  = node->my_key.node;
   *next_inst_len  = NODE_INST_ID_LEN;
   /* Set the GET data pointer */
   *data = (NCSCONTEXT)node;

   return NCSCC_RC_SUCCESS;   
}

/**************************************************************************
 Function: ncsdtsvnodelogpolicyentry_setrow 
 Purpose:  Node Policy table's setrow routine
 Input:    cb       : DTS_CB pointer
           arg      : Mib arg pointer
           params   : Poinet to param structure
           objinfo  : Pointer to object info structure
           flag     : Flase to perform setrow operation other wise testrow
 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
uns32 
ncsdtsvnodelogpolicyentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *arg,
                     NCSMIB_SETROW_PARAM_VAL *params,
                     struct ncsmib_obj_info *objinfo,
                     NCS_BOOL flag)
{
   DTS_CB               *inst = (DTS_CB *)cb;
   DTS_SVC_REG_TBL       *node;
   NCS_BOOL              sameval = FALSE,
                         create = FALSE;  
   uns32                 paramid = 0, old_buff_size = 0,
                         rc = NCSCC_RC_SUCCESS;
   uns8                  log_device = 0;
   NCSMIBLIB_REQ_INFO    reqinfo;
   SVC_KEY                key, nt_key;
   memset(&reqinfo, 0, sizeof(reqinfo));
 
   /* Validate the index length */
   if(NODE_INST_ID_LEN != arg->i_idx.i_inst_len)
      return NCSCC_RC_NO_INSTANCE;

   key.node = arg->i_idx.i_inst_ids[0];
   key.ss_svc_id = 0;
   
   /*  Network order key added */
   nt_key.node = m_NCS_OS_HTONL(arg->i_idx.i_inst_ids[0]);
   nt_key.ss_svc_id = 0;

   node = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&inst->svc_tbl,(const uns8*)&nt_key);
   if(node == NULL)
      create = TRUE;

   if(params[ncsDtsvNodeRowStatus_ID - 1].set_flag)
   {
      /* Do the Row Status validation, if PSSv is not playing-back. */
      if((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
      {
         reqinfo.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP;
         reqinfo.info.i_val_status_util_info.row_status = 
            &params[ncsDtsvNodeRowStatus_ID - 1].param.info.i_int;
         if ((node != NULL) && (node->row_exist == FALSE))
             reqinfo.info.i_val_status_util_info.row = (NCSCONTEXT)NULL;
         else
             reqinfo.info.i_val_status_util_info.row = (NCSCONTEXT)node;
         rc = ncsmiblib_process_req(&reqinfo);
         if(NCSCC_RC_SUCCESS != rc) return rc;
      }
      else
      {
         /* If PSSv is playing back, set rowstatus accordingly */
         switch(params[ncsDtsvNodeRowStatus_ID - 1].param.info.i_int)
         {
         case NCSMIB_ROWSTATUS_CREATE_GO:
            params[ncsDtsvNodeRowStatus_ID - 1].param.info.i_int = NCSMIB_ROWSTATUS_ACTIVE;
            break;
         case NCSMIB_ROWSTATUS_CREATE_WAIT:
            params[ncsDtsvNodeRowStatus_ID - 1].param.info.i_int = NCSMIB_ROWSTATUS_NOTINSERVICE;
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
       rc = dts_create_new_pat_entry(inst, &node, key.node, key.ss_svc_id, NODE_LOGGING);
       if(rc != NCSCC_RC_SUCCESS)
       {
          /*log ERROR*/
          ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", DTS_EV_SVC_REG_ENT_ADD_FAIL, key.ss_svc_id, key.node, 0);
          ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", DTS_SET_FAIL, key.ss_svc_id, key.node, 0);
          return rc;
       }
       /* Send Async add to stby DTS */
       m_DTSV_SEND_CKPT_UPDT_ASYNC(inst, NCS_MBCSV_ACT_ADD, (MBCSV_REO_HDL)(long)node, DTSV_CKPT_DTS_SVC_REG_TBL_CONFIG);
   }

   if(node != NULL)
      node->row_exist = TRUE;

   /* Do Set Row Operation Now */
   for (paramid = ncsDtsvNodeIndexNode_ID; paramid < ncsDtsvNodeLogPolicyEntryMax_ID; paramid++)
   {
      if(params[paramid-1].set_flag)
      {

         switch(paramid)
         {
            /* DTS_PSSV integration - Check for BITS type MIBs.
             *                        If yes, then don't call miblib_req, call
             *                        dts specific function instead.
             */
            case ncsDtsvNodeLogDevice_ID:
               log_device = node->svc_policy.log_dev;
               /* Check for valid value of log device */
               /* First check for null i_oct for the above-mentined check*/
               if(params[paramid-1].param.info.i_oct == NULL)
                  return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "ncsdtsvnodelogpolicyentry_setrow: NULL pointer for i_oct in SETROW request received");
               else if(*(uns8*)params[paramid-1].param.info.i_oct < DTS_LOG_DEV_VAL_MAX)
               {
                  if((*(uns8*)params[paramid-1].param.info.i_oct == 0) &&
                     (log_device != 0))
                     m_LOG_DTS_EVT(DTS_LOG_DEV_NONE, node->my_key.ss_svc_id, node->my_key.node, 0);
                  else if(*(uns8*)params[paramid-1].param.info.i_oct != 0)
                     return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_setrow_node_policy_obj: Log Device bit map beyond permissible values");
               }

            case ncsDtsvNodeCategoryBitMap_ID:
            case ncsDtsvNodeSeverityBitMap_ID:
               /* Fill NCSMIB_ARG according to params struct before calling*/
               memset(&arg->req.info.set_req.i_param_val, '\0', sizeof(NCSMIB_PARAM_VAL));
               memcpy(&arg->req.info.set_req.i_param_val, &params[paramid-1].param, sizeof(NCSMIB_PARAM_VAL));
               /* Call DTS's own SET function for BITS MIB type */
               rc = dtsv_node_policy_set_oct(cb, arg, &node->svc_policy);
               break;

            case ncsDtsvNodeCircularBuffSize_ID:
               old_buff_size = node->svc_policy.cir_buff_size;

            default:
               sameval = FALSE;
               reqinfo.req = NCSMIBLIB_REQ_SET_UTIL_OP;
               reqinfo.info.i_set_util_info.data = (NCSCONTEXT)node;
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
         if (NCSCC_RC_SUCCESS != dts_handle_node_param_set(inst, node, 
             paramid, log_device, old_buff_size))
             return NCSCC_RC_FAILURE;
      }
   }
   return rc;   
}

/**************************************************************************
 Function: dtsv_node_policy_change

 Purpose:  Change in the Node Filtering policy will affect the entire node.
           This function will set the filtering policies of all the  services 
           which are currently present in the service registration table. 
           Also, policy change will be sent to all the services.

 Input:    cb       : DTS_CB pointer
           param_id : Parameter for which change is done.
           node_id  : Node id of the node.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
**************************************************************************/
static uns32
dtsv_node_policy_change(DTS_CB *inst, 
                        DTS_SVC_REG_TBL *node,
                        NCSMIB_PARAM_ID param_id, 
                        uns32 node_id)
{
    SVC_KEY               nt_key;
    DTS_SVC_REG_TBL       *service;
    DTA_DEST_LIST        *dta;
    DTA_ENTRY            *dta_entry;

    /* Search through Service per node registration table, Set all the policies,
     * configure all the DTA's using this policy 
     */
    /*  Network order key added */
    nt_key.node  = m_NCS_OS_HTONL(node_id);
    nt_key.ss_svc_id = 0;

    while (((service = (DTS_SVC_REG_TBL *)dts_get_next_svc_entry(&inst->svc_tbl, &nt_key)) != NULL) &&
           (service->my_key.node == node_id))
    {
        /* Setup key for new search */
        nt_key.ss_svc_id = service->ntwk_key.ss_svc_id;
        
        /* Set the Node Policy as per the Node Policy */
        if (param_id == ncsDtsvNodeLoggingState_ID)
            service->svc_policy.enable           = node->svc_policy.enable;
        else if (param_id == ncsDtsvNodeCategoryBitMap_ID)
            service->svc_policy.category_bit_map = node->svc_policy.category_bit_map;
        else if (param_id == ncsDtsvNodeSeverityBitMap_ID)
            service->svc_policy.severity_bit_map = node->svc_policy.severity_bit_map;
        else if (param_id == ncsDtsvNodeMessageLogging_ID)
        {
            if (node->per_node_logging == NCS_SNMP_FALSE)
                service->device.new_file = TRUE;

            dts_circular_buffer_clear(&service->device.cir_buffer);
            m_LOG_DTS_CBOP(DTS_CBOP_CLEARED, service->my_key.ss_svc_id, service->my_key.node);
        }
        else
            return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_node_policy_change: Wrong param ID passed to the function dtsv_node_policy_change");
        
        /* Configure services for this change */
        dta_entry = service->v_cd_list;
 
        while(dta_entry != NULL)
        {
            dta = dta_entry->dta;
            dts_send_filter_config_msg(inst, service, dta);
            dta_entry = dta_entry->next_in_svc_entry; 
        }
    }

    return NCSCC_RC_SUCCESS;
}

/**************************************************************************
 Function: dts_node_row_status_set

 Purpose:  Function used for handeling the row status changes. 

 Input:    inst      : DTS control block.
           node      : Registration table entry
        
 Returns:  None.

 Notes:  
**************************************************************************/
static void 
dts_node_row_status_set(DTS_CB           *inst,
                        DTS_SVC_REG_TBL  *node)
{
    OP_DEVICE *dev;

    if (node->row_status == NCSMIB_ROWSTATUS_ACTIVE)
    {
        /*
         * First handle the changes in the logging policy due to row destroy.
         */
        dts_log_policy_change(node, &inst->dflt_plcy.node_dflt.policy, &node->svc_policy);

        /* Change the filtering policy aswell */
        dts_filter_policy_change(node, &inst->dflt_plcy.node_dflt.policy, &node->svc_policy);
   
        /*
         * Check if we need to change the logging policy. If yes then 
         * new logging policy handle should be sent the DTA for logging.
         */
        if (((node->per_node_logging == NCS_SNMP_TRUE) &&
            (inst->dflt_plcy.node_dflt.per_node_logging == NCS_SNMP_FALSE)) ||
            ((inst->dflt_plcy.node_dflt.per_node_logging == NCS_SNMP_TRUE) &&
            (node->per_node_logging == NCS_SNMP_FALSE)))
        {
            dtsv_node_policy_change(inst, node, ncsDtsvNodeMessageLogging_ID, 
                node->my_key.node);
        }
    }
    else
    {
        /*
         * First handle the changes in the logging policy due to row destroy.
         */
        dts_log_policy_change(node, &node->svc_policy, &inst->dflt_plcy.node_dflt.policy);
        /* Changing the filtering policy as well. don't return
         *            Print a DBG SINK. Rowstatus deletion must occur
         */
        dts_filter_policy_change(node, &node->svc_policy, &inst->dflt_plcy.node_dflt.policy);

        /*
         * Check if we need to change the logging policy. If yes then 
         * new logging policy handle should be sent the DTA for logging.
         */
        if (((node->per_node_logging == NCS_SNMP_TRUE) &&
            (inst->dflt_plcy.node_dflt.per_node_logging == NCS_SNMP_FALSE)) ||
            ((inst->dflt_plcy.node_dflt.per_node_logging == NCS_SNMP_TRUE) &&
            (node->per_node_logging == NCS_SNMP_FALSE)))
        {
            dtsv_node_policy_change(inst, node, ncsDtsvNodeMessageLogging_ID, 
                node->my_key.node);
        }

        if (node->row_status == NCSMIB_ROWSTATUS_DESTROY)
        {
            /* With Rowstatus Destroy, memory for node should be
             * freed up.
             */
            node->row_exist = FALSE;
            dev = &node->device;
            dts_circular_buffer_free(&dev->cir_buffer);
            ncs_patricia_tree_del(&inst->svc_tbl, (NCS_PATRICIA_NODE *)node);
            /* Cleanup the DTS_FILE_LIST datastructure for node */
            m_DTS_FREE_FILE_LIST(dev);
            /* Cleanup the console devices associated with the node */
            m_DTS_RMV_ALL_CONS(dev);
            /* Send RMV updt here itself, cuz node is going to be deleted */
            m_DTSV_SEND_CKPT_UPDT_ASYNC(inst, NCS_MBCSV_ACT_RMV, (MBCSV_REO_HDL)(long)node, DTSV_CKPT_DTS_SVC_REG_TBL_CONFIG);

            if (NULL != node)
                m_MMGR_FREE_SVC_REG_TBL(node);

            node = NULL;
        }
    }
    
}

/**************************************************************************
 Function: dts_handle_node_param_set

 Purpose:  Function used for handeling node parameter changes. 

 Input:    inst      : DTS control block.
           node      : Registration table entry
        
 Returns:  None.

 Notes:  
**************************************************************************/
static uns32 
dts_handle_node_param_set(DTS_CB           *inst,
                          DTS_SVC_REG_TBL  *node,
                          NCSMIB_PARAM_ID   paramid,
                          uns8              old_log_device,
                          uns32             old_buff_size) 
{
    uns32     rc = NCSCC_RC_SUCCESS;
    NCS_BOOL  destroy_op = FALSE;

    switch(paramid)
    {
    case ncsDtsvNodeRowStatus_ID:
        /*Set cli_bit_map to paramid incase node entry is deleted in 
         * dts_node_row_status_set() */
        inst->cli_bit_map = paramid;
        if(node->row_status == NCSMIB_ROWSTATUS_DESTROY)
           destroy_op = TRUE; 
        dts_node_row_status_set(inst, node);
        break;
    case ncsDtsvNodeLogDevice_ID:
        if(node->row_status == NCSMIB_ROWSTATUS_ACTIVE)
           rc = dts_log_device_set(&node->svc_policy,
                                   &node->device, old_log_device);
        break;
        
    case ncsDtsvNodeLogFileSize_ID:
        break;
        
    case ncsDtsvNodeFileLogCompFormat_ID:
        if(node->row_status == NCSMIB_ROWSTATUS_ACTIVE)
           node->device.file_log_fmt_change = TRUE;
        break;
        
    case ncsDtsvNodeCircularBuffSize_ID:
        if(node->row_status == NCSMIB_ROWSTATUS_ACTIVE)
           rc = dts_buff_size_set(&node->svc_policy, &node->device, old_buff_size);
        break;
        
    case ncsDtsvNodeCirBuffCompFormat_ID:
        if(node->row_status == NCSMIB_ROWSTATUS_ACTIVE)
           node->device.buff_log_fmt_change = TRUE;
        break;
        
    case ncsDtsvNodeMessageLogging_ID:
        if(node->row_status == NCSMIB_ROWSTATUS_ACTIVE)
        {
           if (node->per_node_logging == NCS_SNMP_TRUE)
               node->device.new_file = TRUE;
        
           /* Smik - Update the cli_bit_map field in DTS_CB */
           inst->cli_bit_map = paramid;
           dts_circular_buffer_clear(&node->device.cir_buffer);
           m_LOG_DTS_CBOP(DTS_CBOP_CLEARED, 0, node->my_key.node);
           rc = dtsv_node_policy_change(inst, node, 
                                        paramid, node->my_key.node);
        }
        break;
    
    case ncsDtsvNodeCategoryBitMap_ID:
        if(node->row_status == NCSMIB_ROWSTATUS_ACTIVE)
        {
           inst->cli_bit_map = paramid;
           node->svc_policy.category_bit_map = ntohl(node->svc_policy.category_bit_map);
           rc = dtsv_node_policy_change(inst, node, 
                                        paramid, node->my_key.node);
        }
        break;

    case ncsDtsvNodeLoggingState_ID:
    case ncsDtsvNodeSeverityBitMap_ID:
        if(node->row_status == NCSMIB_ROWSTATUS_ACTIVE)
        {
           inst->cli_bit_map = paramid;
           rc = dtsv_node_policy_change(inst, node, 
                                        paramid, node->my_key.node);
        }
        break;
        
    default:
        break;
    }

    if((rc != NCSCC_RC_FAILURE) && (destroy_op != TRUE))
    {
       /* Smik - Send Async update for DTS_SVC_REG_TBL to stby DTS */
       m_DTSV_SEND_CKPT_UPDT_ASYNC(inst, NCS_MBCSV_ACT_UPDATE, (MBCSV_REO_HDL)(long)node, DTSV_CKPT_DTS_SVC_REG_TBL_CONFIG);
    }

    /* Re-set cli_bit_map */
    inst->cli_bit_map = 0;

    return rc;
}


/**************************************************************************
* Function: dtsv_node_conf_console
* Purpose:  Node device's console configuration
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer
*           flag     : Tells the operation to be performed.
*                      TRUE is add, FALSE is remove.           
* Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 dtsv_node_conf_console(DTS_CB *cb, NCSMIB_ARG *arg, NCS_BOOL flag)
{
   OP_DEVICE *dev = NULL;
   uns8       bit_map = 0;
   uns16      str_len;
   NCS_UBAID  uba;
   USRBUF     *buf = arg->req.info.cli_req.i_usrbuf;
   uns8       data_buff[DTSV_CLI_MAX_SIZE]="", *buf_ptr = NULL;
   uns32      node_id, rc = NCSCC_RC_SUCCESS;
   SVC_KEY    nt_key;
   DTS_SVC_REG_TBL *node_ptr;

   memset(&uba, '\0', sizeof(uba));

   ncs_dec_init_space(&uba, buf);
   arg->req.info.cli_req.i_usrbuf = NULL;

   if(flag == TRUE)
      buf_ptr = ncs_dec_flatten_space(&uba, data_buff, DTSV_ADD_NODE_CONS_SIZE);
   else
      buf_ptr = ncs_dec_flatten_space(&uba, data_buff, DTSV_RMV_NODE_CONS_SIZE);

   if(buf_ptr == NULL)
      return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: ncs_dec_flatten_space returns NULL");

   /* Decode the severity bit_map, node_id & strlen passed */
   if(flag == TRUE)
   {
      bit_map = ncs_decode_8bit(&buf_ptr); 
      ncs_dec_skip_space(&uba, sizeof(uns8));
   }
   node_id = ncs_decode_32bit(&buf_ptr);
   ncs_dec_skip_space(&uba, sizeof(uns32));
   str_len = ncs_decode_8bit(&buf_ptr);
   ncs_dec_skip_space(&uba, sizeof(uns8));

   /* Now decode console device, to be kept in data_buff array */
   if(ncs_decode_n_octets_from_uba(&uba, (char*)&data_buff, str_len) != NCSCC_RC_SUCCESS)
   {
      return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: ncs_decode_n_octets_from_uba failed");
   }

   /* Find the node frm patricia tree */
   /*  Network order key added */
   nt_key.node = m_NCS_OS_HTONL(node_id);
   nt_key.ss_svc_id = 0;
   if((node_ptr = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl, (const uns8*)&nt_key)) == NULL)
   {
      return NCSCC_RC_INV_SPECIFIC_VAL;
   }   
   /* Get the device associated with the node */
   dev = &node_ptr->device;

   if(flag == TRUE)
      /* Now add the device to global device */
      m_DTS_ADD_CONS(cb, dev, (char *)&data_buff, bit_map)
   else
   {
      if(strcmp((char *)&data_buff, "all") == 0)
         m_DTS_RMV_ALL_CONS(dev)
      else
         m_DTS_RMV_CONS(cb, dev, (char *)&data_buff);
   }
   return rc;
}

/**************************************************************************
* Function: dtsv_node_disp_conf_console
* Purpose:  Displays node's currently configured consoles
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer
*
* Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 dtsv_node_disp_conf_console(DTS_CB *cb, NCSMIB_ARG *arg)
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

   memset(&rsp_uba, '\0', sizeof(rsp_uba));
   memset(&uba, '\0', sizeof(uba));

   /* Set parameters for response */
   arg->rsp.info.cli_rsp.i_cmnd_id = dtsvDispNodeConsole;
   arg->rsp.info.cli_rsp.o_partial = FALSE;

   /* Decode the req to get the node_id */
   ncs_dec_init_space(&uba, buf);
   arg->req.info.cli_req.i_usrbuf = NULL;

   dec_ptr = ncs_dec_flatten_space(&uba, data_buff, sizeof(uns32));
   if(dec_ptr == NULL)
      return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_node_conf_console: ncs_dec_flatten_space returns NULL");  

   /*  Network order key added */
   nt_key.node = m_NCS_OS_HTONL(ncs_decode_32bit(&dec_ptr));
   ncs_dec_skip_space(&uba, sizeof(uns32));

   nt_key.ss_svc_id = 0;

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
      str_len = strlen((char *)cb->cons_dev);
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
         str_len = strlen(cons_ptr->cons_dev);
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
 * Function: ncsdtsvnodelogpolicyentry_rmvrow 
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
 *************************************************************************/
uns32 ncsdtsvnodelogpolicyentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx)
{
   /* Assign defaults for your globals here. */
   return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: ncsdtsvnodelogpolicyentry_extract 
* Purpose:  Policy table's extract object value routine
* Input:    param_val: Param pointer
*           var_info : var_info pointer
*           data     : Pointer to the data structure
*           buffer   : Buffer to get the octect data
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 ncsdtsvnodelogpolicyentry_extract(NCSMIB_PARAM_VAL *param_val,
                             NCSMIB_VAR_INFO *var_info,
                             NCSCONTEXT data,
                             NCSCONTEXT buffer)
{
 
    if((param_val->i_param_id == ncsDtsvNodeLogDevice_ID) || (param_val->i_param_id ==ncsDtsvNodeCategoryBitMap_ID) || (param_val->i_param_id == ncsDtsvNodeSeverityBitMap_ID))
    {
       return dtsv_node_extract_oct(param_val, var_info, data, buffer);
    }
    else
    {   
       return dtsv_extract_policy_obj(param_val, var_info, data, buffer);
    }
}

/**************************************************************************
* Function: dtsv_node_policy_set_oct 
* Purpose:  SET operation for MIBS of type BITS for node policies.
*           
* Input:    cb       : DTS_CB pointer
*           arg      : NCSMIB_ARG pointer
*           policy   : POLICY pointer
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 dtsv_node_policy_set_oct(DTS_CB *cb, NCSMIB_ARG *arg,
                               POLICY *policy)
{
   NCSMIB_PARAM_VAL *param_val = &arg->req.info.set_req.i_param_val;
   NCSCONTEXT data = NULL;

   if(param_val->i_param_id == ncsDtsvNodeCategoryBitMap_ID)
       data = &policy->category_bit_map;
   else if(param_val->i_param_id == ncsDtsvNodeSeverityBitMap_ID)
       data = &policy->severity_bit_map;
   else if(param_val->i_param_id == ncsDtsvNodeLogDevice_ID)
       data = &policy->log_dev;

   return dtsv_policy_set_oct(arg, data);
}

/**************************************************************************
* Function: dtsv_node_extract_oct 
* Purpose:  EXTRACT operation for MIBS of type BITS for node policies.
*           
* Input:    param_val    : NCSMIB_PARAM_VAL pointer
*           data         : DTS_SVC_REG_TBL pointer
*           buffer       : Buffer pointer
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 dtsv_node_extract_oct(NCSMIB_PARAM_VAL *param_val, 
                            NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, 
                            NCSCONTEXT buffer)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   DTS_SVC_REG_TBL *node = (DTS_SVC_REG_TBL *)data;
   uns32  tmp, nworder;
  
   param_val->i_fmat_id = var_info->fmat_id;
 
   switch(param_val->i_param_id)
   {
      case ncsDtsvNodeLogDevice_ID:
         param_val->i_length = sizeof(uns8);
         /*memcpy((uns8*)&tmp, &node->svc_policy.log_dev, param_val->i_length);*/
         param_val->info.i_oct = (uns8*)&node->svc_policy.log_dev;
         break;
 
      case ncsDtsvNodeCategoryBitMap_ID:
         param_val->i_length = sizeof(uns32);
         memcpy((uns8*)&tmp, &node->svc_policy.category_bit_map, param_val->i_length);
         nworder = htonl(tmp);
         memcpy((uns8*)buffer, (uns8*)&nworder, param_val->i_length);
         param_val->info.i_oct = (uns8*)buffer;
         break;

      case ncsDtsvNodeSeverityBitMap_ID:
         param_val->i_length = sizeof(uns8);
         /*memcpy((uns8*)&tmp, &node->svc_policy.severity_bit_map, param_val->i_length);*/
         param_val->info.i_oct = (uns8*)&node->svc_policy.severity_bit_map; 
         break;

      default:
          return NCSCC_RC_FAILURE;
   }

   /*nworder = htonl(tmp);
   memcpy((uns8*)buffer, (uns8*)&nworder, param_val->i_length);

   param_val->info.i_oct = (uns8*)buffer;*/
  
   return rc;
}
