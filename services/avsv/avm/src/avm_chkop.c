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

  DESCRIPTION: This module does the initialisation of MBCSV and provides
  callback functions at Availability Manager. It contains both the
  callbacks to encode the structures at active and decode along with
  building the structures at standby availability directors. All this
  functionality happens in the context of the MBCSV.


..............................................................................

  FUNCTIONS INCLUDED in this module:

avm_mbcsv_cb
avm_mbcsv_process_enc_cb
avm_mbcsv_process_peer_info_cb
avm_mbcsv_process_notify
avm_mbc_init
avm_mbc_open_ckpt
avm_mbc_get_sel_obj
avm_mbc_close_ckpt
******************************************************************************
*/

#include "avm.h"

static uns32 avm_mbcsv_cb(NCS_MBCSV_CB_ARG *arg);

static uns32 avm_mbcsv_process_enc_cb(AVM_CB_T             *cb,
                                      NCS_MBCSV_CB_ARG     *arg);   

static uns32 avm_mbcsv_process_dec_cb(AVM_CB_T            *cb,
                                       NCS_MBCSV_CB_ARG    *arg);

static uns32 
avm_mbcsv_process_peer_info_cb(
                               AVM_CB_T           *cb,
                               NCS_MBCSV_CB_ARG   *arg
                              );
static uns32
avm_mbcsv_process_notify(
                            AVM_CB_T           *cb,
                            NCS_MBCSV_CB_ARG   *arg
                        );
static uns32
avm_mbcsv_process_en_notify_cb(
                            AVM_CB_T           *cb,
                            NCS_MBCSV_CB_ARG   *arg
                        );


static uns32 avm_mbc_init(AVM_CB_T *cb);
static uns32 avm_mbc_open_ckpt(AVM_CB_T  *cb);
static uns32 avm_mbc_get_sel_obj(AVM_CB_T *cb);
static uns32 avm_mbc_close_ckpt(AVM_CB_T *cb);

const AVM_ENCODE_CKPT_DATA_FUNC_PTR
               avm_enc_ckpt_data_func_list[AVM_CKPT_MSG_MAX];

const AVM_DECODE_CKPT_DATA_FUNC_PTR
               avm_dec_ckpt_data_func_list[AVM_CKPT_MSG_MAX];

/**********************************************************************
 ******
 * Name          : avm_mbc_init
 *
 * Description   : This function initializes MBCSv interface
 *
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *********************************************************************/
static uns32
avm_mbc_init(AVM_CB_T *cb)
{
   NCS_MBCSV_ARG mbcsv_arg;

   m_AVM_LOG_FUNC_ENTRY("avm_mbcsv_initialize");
   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_INITIALIZE;
   mbcsv_arg.info.initialize.i_service    = NCS_SERVICE_ID_AVM;
   mbcsv_arg.info.initialize.i_mbcsv_cb   = avm_mbcsv_cb;
   mbcsv_arg.info.initialize.i_version    = AVM_MBCSV_SUB_PART_VERSION;

   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }
   
   cb->mbc_hdl = mbcsv_arg.info.initialize.o_mbcsv_hdl;
   return NCSCC_RC_SUCCESS;
   
}

/**********************************************************************
 ******
 * Name          : avm_mbc_open_ckpt
 *
 * Description   : This function is used to open MBCS ckpt
 *
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 **********************************************************
 *****/
static uns32 
avm_mbc_open_ckpt(AVM_CB_T *cb)
{
   NCS_MBCSV_ARG mbcsv_arg;
   
   m_AVM_LOG_FUNC_ENTRY("avm_mbcsv_open_ckpt");

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
   
   mbcsv_arg.i_op = NCS_MBCSV_OP_OPEN;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;
   mbcsv_arg.info.open.i_pwe_hdl    = cb->vaddr_pwe_hdl;
   mbcsv_arg.info.open.i_client_hdl = cb->cb_hdl;

   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   cb->ckpt_hdl = mbcsv_arg.info.open.o_ckpt_hdl;
   return NCSCC_RC_SUCCESS;
}

/**********************************************************************
 ******
 * Name          : avm_set_ckpt_role
 *
 * Description   : This function is used to set MBCSv role
 *
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 *****/
uns32 avm_set_ckpt_role(AVM_CB_T *cb,
                        uns32     role)
{
   NCS_MBCSV_ARG mbcsv_arg;
   
   m_AVM_LOG_FUNC_ENTRY("avm_set_ckpt_role");
   
   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
   
   mbcsv_arg.i_op = NCS_MBCSV_OP_CHG_ROLE;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;
   mbcsv_arg.info.chg_role.i_ckpt_hdl = cb->ckpt_hdl;
   mbcsv_arg.info.chg_role.i_ha_state = role;

   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {   
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }
   
   return NCSCC_RC_SUCCESS;
} 

/**********************************************************************
 ******
 * Name          : avm_mbc_get_sel_obj
 *
 * Description   : This function is used to set MBCSv sel obj 
 *
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 *****/
static uns32 avm_mbc_get_sel_obj(AVM_CB_T *cb)
{

   NCS_MBCSV_ARG mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op        = NCS_MBCSV_OP_SEL_OBJ_GET;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;

   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }
   cb->mbc_sel_obj = mbcsv_arg.info.sel_obj_get.o_select_obj;
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
*  PROCEDURE NAME:   avm_mbc_register
*
*  DESCRIPTION:      This function encapsulates all the AvM r
*                    egistration with MBCSv which includes 
*                    Initializing, Openning checkpoint chan
*                    nels, getting selection objects.   
*
*  Input:             cb - AvM control block pointer. 
*
*  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
extern uns32 
avm_mbc_register(AVM_CB_T *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   memset(&cb->async_updt_cnt, 0, sizeof(AVM_ASYNC_CNT_T));

   /* Intializing MBCSv interface */
   if(NCSCC_RC_SUCCESS != avm_mbc_init(cb))
   {
     m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
     return NCSCC_RC_FAILURE;
   }

   /* Opening Check Point channel */
   if(NCSCC_RC_SUCCESS != avm_mbc_open_ckpt(cb))
   {
       m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
       avm_mbc_finalize(cb);
       return NCSCC_RC_FAILURE;
   }

   /* Getting MBCSv selection Object */
   if(NCSCC_RC_SUCCESS != avm_mbc_get_sel_obj(cb))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      avm_mbc_close_ckpt(cb);
      avm_mbc_finalize(cb);
      return NCSCC_RC_FAILURE;
   }

   /* Setting MBCsv init role */
   if(NCSCC_RC_SUCCESS != avm_set_ckpt_role(cb, cb->ha_state))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      avm_mbc_close_ckpt(cb);
      avm_mbc_finalize(cb);
      return NCSCC_RC_FAILURE;
   }


   return rc;
}
                        
/*****************************************************************************
*  Function   :  avm_mbc_dispatch
*
*  DESCRIPTION:  This function is used to perform dispatch operation
*                on MBCSV selection object. 
*
*  Input:        cb   - AvM control block pointer.
*                flag - Dipatach flag either SA_DISPATCH_ONE or SA_DISPATCH_ALL
*
*
*  RETURNS:     NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
*
*****************************************************************************/
uns32 
avm_mbc_dispatch(
                  AVM_CB_T *cb,
                  uns32     flag
                )
{
   NCS_MBCSV_ARG mbcsv_arg;
   uns32 rc = NCSCC_RC_SUCCESS;


   m_AVM_LOG_FUNC_ENTRY("avm_mbc_dispatch");

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_DISPATCH;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;
   mbcsv_arg.info.dispatch.i_disp_flags = flag;

   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   return rc;
}

/*****************************************************************************
*  Function:  avm_mbc_warm_sync_off
*
*  DESCRIPTION:  This function disables warm sync 
*
*  Input:        cb - AvM control block pointer.
*
*  Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*
*  NOTES:
*   
*
*****************************************************************************/
extern uns32 
avm_mbc_warm_sync_off( AVM_CB_T *cb)
{
   NCS_MBCSV_ARG mbcsv_arg;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_mbc_warm_sync_off");

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_OBJ_SET;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;
   mbcsv_arg.info.obj_set.i_ckpt_hdl = cb->ckpt_hdl;
   mbcsv_arg.info.obj_set.i_obj = NCS_MBCSV_OBJ_WARM_SYNC_ON_OFF;
   mbcsv_arg.info.obj_set.i_val = FALSE;

   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   return rc;
}

/*****************************************************************************
*  Function   : avm_send_ckpt_data
*
*  Purpose    :  This function is used to send checkpoint data.
*
* Input: cb        - AvM control block pointer.
*        action    - Action to be perform (add, remove or update)
*        reo_hdl   - Redudant object handle.
*        reo_type  - Redudant object type.
*        send_type - Send type to be used.
*
* Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*
* NOTES:
*
*****************************************************************************/
extern uns32 
avm_send_ckpt_data(AVM_CB_T    *cb,
                   uns32       action,
                   MBCSV_REO_HDL reo_hdl,
                   uns32       reo_type,
                   uns32       send_type)
{
   NCS_MBCSV_ARG mbcsv_arg;
   
   if(SA_AMF_HA_STANDBY == cb->ha_state)
   {
      return NCSCC_RC_SUCCESS;
   }
   
   m_AVM_LOG_FUNC_ENTRY("avm_send_ckpt_data");

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
   
   mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_CKPT;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;
   mbcsv_arg.info.send_ckpt.i_action = action;
   mbcsv_arg.info.send_ckpt.i_ckpt_hdl  = cb->ckpt_hdl;
   mbcsv_arg.info.send_ckpt.i_reo_hdl   = reo_hdl;
   mbcsv_arg.info.send_ckpt.i_reo_type  = reo_type;
   mbcsv_arg.info.send_ckpt.i_send_type = send_type;
   
   switch(reo_type)
   {
      case AVM_CKPT_ENT_CFG:
         cb->async_updt_cnt.ent_cfg_updt++;
         break;

      case AVM_CKPT_ENT_STATE:
         cb->async_updt_cnt.ent_updt++;
         break;

      case AVM_CKPT_ENT_STATE_SENSOR:
         cb->async_updt_cnt.ent_sensor_updt++;
         break;

      case AVM_CKPT_ENT_ADD_DEPENDENT:
         cb->async_updt_cnt.ent_dep_updt++;
         break;

      case AVM_CKPT_ENT_ADM_OP:
         cb->async_updt_cnt.ent_adm_op_updt++;
         break;

      case AVM_CKPT_EVT_ID:
         cb->async_updt_cnt.evt_id_updt++;
         break;

      case AVM_CKPT_ENT_DHCP_CONF_CHG:
         cb->async_updt_cnt.ent_dhconf_updt++;
         break;

      case AVM_CKPT_ENT_DHCP_STATE_CHG:
         cb->async_updt_cnt.ent_dhstate_updt++;
         break;

      case AVM_CKPT_ENT_UPGD_STATE_CHG:
         cb->async_updt_cnt.ent_upgd_state_updt++;
         break;

      default:
         return NCSCC_RC_SUCCESS;
   }
   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
   }
   return NCSCC_RC_SUCCESS;
}      

/*****************************************************************************
*  Function:  avm_send_data_req
*
*  DESCRIPTION:  This function sends data req to Active 
*
*  Input:        cb - AvM control block pointer.
*
*  Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*
*  NOTES:
*   
*****************************************************************************/
extern uns32
avm_send_data_req(AVM_CB_T *cb)
{
   NCS_MBCSV_ARG  mbcsv_arg;
   NCS_UBAID      *uba;
      
   m_AVM_LOG_FUNC_ENTRY("avm_send_data_req");
   
   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
   
   mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_DATA_REQ;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;
   uba = &mbcsv_arg.info.send_data_req.i_uba;
   memset(uba, '\0', sizeof(NCS_UBAID));
   mbcsv_arg.info.send_data_req.i_ckpt_hdl = cb->ckpt_hdl;
   
   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}
 

/*****************************************************************************
*  Function : avm_mbcsv_process_dec_cb
*
*  Purpose  :  This function is used to decode
*  Input    : arg - MBCSV callback argument pointer.
*
*  Returns  : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*
*  NOTES:
*
*****************************************************************************/
static uns32 avm_mbcsv_process_dec_cb(AVM_CB_T           *cb,
                                      NCS_MBCSV_CB_ARG *arg)   
{
   uns32 status = NCSCC_RC_SUCCESS;
   uns32 rc     = NCSCC_RC_SUCCESS;
   
   m_AVM_LOG_FUNC_ENTRY("avm_mbcsv_process_dec_cb");

   m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_DEC,  AVM_MBCSV_IGNORE, arg->info.decode.i_msg_type, NCSFL_SEV_INFO);


   switch(arg->info.decode.i_msg_type)
   {
      case NCS_MBCSV_MSG_ASYNC_UPDATE:
      {
         status = avm_dec_ckpt_data_func_list[arg->info.decode.i_reo_type](cb, &arg->info.decode);
         m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_ASYNC_DEC,  status + AVM_MBCSV_RES_BASE, arg->info.decode.i_reo_type, NCSFL_SEV_INFO);
      }
      break;
      
      case NCS_MBCSV_MSG_COLD_SYNC_REQ:
      {
         if(cb->cfg_state != AVM_CONFIG_DONE)
         {
            status = NCSCC_RC_FAILURE;
         }
      }
      break;

      case NCS_MBCSV_MSG_COLD_SYNC_RESP:
      case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
      {
         status =  avm_decode_cold_sync_rsp(cb, &arg->info.decode);
         if(NCSCC_RC_SUCCESS != status)
         {
            m_AVM_LOG_INVALID_VAL_FATAL(status);
            rc = avm_cleanup_db(cb);
            if(NCSCC_RC_SUCCESS != rc)
            {
               m_AVM_LOG_INVALID_VAL_FATAL(rc);
            }
            break;
         }

         if((NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE == arg->info.decode.i_msg_type) && (NCSCC_RC_SUCCESS  == status))
         {
            cb->cold_sync = TRUE;
         }
         m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_COLD_DEC,  status + AVM_MBCSV_RES_BASE, arg->info.decode.i_reo_type, NCSFL_SEV_NOTICE);
      }
      break;

      case NCS_MBCSV_MSG_WARM_SYNC_REQ:
      {
         m_AVM_LOG_CKPT_OP(AVM_MBCSV_MSG_WARM_SYNC_REQ, AVM_MBCSV_SUCCESS, NCSFL_SEV_INFO);
      }
      break;

      case NCS_MBCSV_MSG_WARM_SYNC_RESP:
      case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
      {
        /* Decode Warm Sync Response messge */
        
        status = avm_decode_warm_sync_rsp(cb, &arg->info.decode); 
         
         if(NCSCC_RC_FAILURE == status)
         {
            m_AVM_LOG_CKPT_OP(AVM_MBCSV_MSG_WARM_SYNC_RESP, AVM_MBCSV_FAILURE, NCSFL_SEV_ERROR);
            cb->stby_in_sync = FALSE;
         }

         m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_WARM_SYNC_RESP, AVM_MBCSV_FAILURE, arg->info.decode.i_reo_type, NCSFL_SEV_INFO);
         
      }
      break;
      
      case NCS_MBCSV_MSG_DATA_RESP:
      case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
      {  
         m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_DATA_RESP, AVM_MBCSV_SUCCESS, arg->info.decode.i_reo_type, NCSFL_SEV_WARNING);
         
         status = avm_decode_data_sync_rsp(cb, &arg->info.decode);
         if(NCSCC_RC_SUCCESS != status)
         {
            m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_DATA_RESP, AVM_MBCSV_FAILURE, arg->info.decode.i_reo_type, NCSFL_SEV_ERROR);
         
            if(NCSCC_RC_SUCCESS != avm_cleanup_db(cb))
            {
               m_AVM_LOG_INVALID_VAL_FATAL(arg->info.decode.i_reo_type);
               break;
            }

            /* Send Data req now to make standby in sync with Active */
            if(NCSCC_RC_SUCCESS != avm_send_data_req(cb))
            {
               m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
               break;
            }
            break;
         }
      
         if(NCS_MBCSV_MSG_DATA_RESP_COMPLETE == arg->info.decode.i_msg_type)
         {
            cb->stby_in_sync = TRUE;
            m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_DATA_RESP_COMPLETE, AVM_MBCSV_SUCCESS, arg->info.decode.i_reo_type, NCSFL_SEV_WARNING);
         }
      }
      break;
      
      case NCS_MBCSV_MSG_DATA_REQ:
      {
         /*Decode data request message */
         /* Nothing to decode here */
         m_AVM_LOG_CKPT_OP(AVM_MBCSV_MSG_DATA_REQ, AVM_MBCSV_SUCCESS, NCSFL_SEV_INFO);

      }
      break;

      default:   
      {
         m_AVM_LOG_INVALID_VAL_FATAL(arg->info.decode.i_msg_type);
         status = NCSCC_RC_FAILURE;
         break;
      }
   }
   return status;
}

/*****************************************************************************
 Function: avm_mbcsv_process_enc_cb

 Purpose :  This function is used to encode
 
 Input  : arg - MBCSV callback argument pointer.
 
 Return : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
 NOTES  :
  
*****************************************************************************/
static uns32 avm_mbcsv_process_enc_cb(AVM_CB_T         *cb,
                                      NCS_MBCSV_CB_ARG *arg)   
{
   uns32 status = NCSCC_RC_SUCCESS;
   
   m_AVM_LOG_FUNC_ENTRY("avm_mbcsv_process_enc_cb");

   m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_ENC, AVM_MBCSV_IGNORE, arg->info.encode.io_msg_type, NCSFL_SEV_INFO);

   switch(arg->info.encode.io_msg_type)
   {
      case NCS_MBCSV_MSG_ASYNC_UPDATE:
      {
         status = avm_enc_ckpt_data_func_list[arg->info.encode.io_reo_type](cb, &arg->info.encode);
         m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_ASYNC_ENC, status + AVM_MBCSV_RES_BASE, arg->info.encode.io_reo_type, NCSFL_SEV_INFO);

      }
      break;
      
      case NCS_MBCSV_MSG_COLD_SYNC_REQ:
      {
      }
      break;

      case NCS_MBCSV_MSG_COLD_SYNC_RESP:
      {
         status = avm_encode_cold_sync_rsp(cb, &arg->info.encode);
         m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_COLD_ENC,  status + AVM_MBCSV_RES_BASE, arg->info.encode.io_reo_type, NCSFL_SEV_INFO);
      }  
      break;

      case NCS_MBCSV_MSG_WARM_SYNC_REQ:
      {
         /* Encode Warm Sync Request message */ 
         
         m_AVM_LOG_CKPT_OP(AVM_MBCSV_MSG_WARM_SYNC_REQ, AVM_MBCSV_SUCCESS, NCSFL_SEV_INFO);
      }
      break;

      case NCS_MBCSV_MSG_WARM_SYNC_RESP:
      {
         /* Encode Warm Sync Response message */

         status = avm_encode_warm_sync_rsp(cb, &arg->info.encode);
         
         m_AVM_LOG_CKPT_OP(AVM_MBCSV_MSG_WARM_SYNC_RESP, AVM_MBCSV_SUCCESS, NCSFL_SEV_INFO);
         
         arg->info.encode.io_msg_type = NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE;
      }
      break;
      
      case NCS_MBCSV_MSG_DATA_RESP:
      {
         /*Encode Data Response messge */
         status = avm_encode_data_sync_rsp(cb, &arg->info.encode);
         m_AVM_LOG_CKPT_EVT(AVM_MBCSV_MSG_DATA_RESP, status + AVM_MBCSV_RES_BASE, arg->info.encode.io_msg_type, NCSFL_SEV_WARNING);
      }
      break;

      case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
      case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
      case NCS_MBCSV_MSG_DATA_REQ:
      case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
      default:   
      {
         m_AVM_LOG_INVALID_VAL_FATAL(arg->info.encode.io_msg_type);
         status = NCSCC_RC_FAILURE;
         break;
      }
   }
   return status;
}

static uns32 
avm_mbcsv_process_peer_info_cb(
                               AVM_CB_T           *cb,
                               NCS_MBCSV_CB_ARG   *arg
                              )
{
   m_AVM_LOG_FUNC_ENTRY("avm_mbcsv_process_peer_info_cb");   
   return NCSCC_RC_SUCCESS;
}
                              

/*****************************************************************************
*  Function : avm_mbcsv_cb
*
*  Purpose  :  This function is MBCSv callback
*
*  Input: arg - MBCSV callback argument pointer. 
*
*  Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
*  NOTES 
*
*****************************************************************************/
static uns32 
avm_mbcsv_cb(NCS_MBCSV_CB_ARG *arg)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVM_CB_T *cb;   

   m_AVM_LOG_FUNC_ENTRY("avm_mbcsv_cb");

   if((cb = (AVM_CB_T*)ncshm_take_hdl(NCS_SERVICE_ID_AVM, arg->i_client_hdl)) == AVM_CB_NULL)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   switch(arg->i_op)
   {
      case NCS_MBCSV_CBOP_ENC:
      {
         status =  avm_mbcsv_process_enc_cb(cb, arg);
      }
      break;

      case NCS_MBCSV_CBOP_DEC:
      {
         status = avm_mbcsv_process_dec_cb(cb, arg);
      }
      break;

      case NCS_MBCSV_CBOP_PEER:
      {
         status = avm_mbcsv_process_peer_info_cb(cb, arg);
      }
      break;
      
      case NCS_MBCSV_CBOP_NOTIFY:
      {
         status = avm_mbcsv_process_notify(cb, arg);
      }
      break;

      case NCS_MBCSV_CBOP_ENC_NOTIFY:
      {
         status = avm_mbcsv_process_en_notify_cb(cb, arg);
      }
      break;

      case NCS_MBCSV_CBOP_ERR_IND:
      break;

      default:
      {
         m_AVM_LOG_INVALID_VAL_FATAL(arg->i_op);
         status = NCSCC_RC_FAILURE;
      } 
      break;
   }

   ncshm_give_hdl(arg->i_client_hdl);   
   return status;
}

/*****************************************************************************
* Function: avm_mbcsv_finalize
*
* Purpose :  This function is used to finalize MBCSv inter-*            face
* 
* Input  : arg - MBCSV callback argument pointer.
* 
* Return : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
* 
* NOTES  :
*****************************************************************************/
extern uns32
avm_mbc_finalize(AVM_CB_T *cb)
{
   NCS_MBCSV_ARG mbcsv_arg;
   
   m_AVM_LOG_FUNC_ENTRY("avm_mbcsv_finalize");

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
   
   mbcsv_arg.i_op = NCS_MBCSV_OP_FINALIZE;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;
   
   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 Function: avm_mbcsv_close_ckpt

 Purpose :  This function is used to close Check point 
 
 Input  : cb -AvM CB pointer.
 
 Return : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
 NOTES  :
  
*****************************************************************************/
static uns32
avm_mbc_close_ckpt(AVM_CB_T *cb)
{
   NCS_MBCSV_ARG mbcsv_arg;
   
   m_AVM_LOG_FUNC_ENTRY("avm_mbc_colse_ckpt");
   
   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
   
   mbcsv_arg.i_op = NCS_MBCSV_OP_CLOSE;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;
   mbcsv_arg.info.close.i_ckpt_hdl = cb->ckpt_hdl;
   
   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
*  Functiom: avm_mbcsv_process_notify
*
*  Purpose:  This function is used to send notify msgs
*
* Input: arg - MBCSV callback argument pointer.
*
* Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*
* NOTES:
*
*
*****************************************************************************/
static uns32
avm_mbcsv_process_notify(
                            AVM_CB_T        *cb,
                            NCS_MBCSV_CB_ARG   *arg
                        )
{
   uns8   *data;
   uns8    data_buff[21];
   uns32   rc =  NCSCC_RC_SUCCESS;

   AVM_EVT_T        evt; 
   NCS_UBAID *uba = &arg->info.notify.i_uba;
   
   data = ncs_dec_flatten_space(uba, data_buff, 3 * sizeof(uns32));

   evt.evt.avm_role_msg.msg_type = ncs_decode_32bit(&data);

   switch(evt.evt.avm_role_msg.msg_type)
   {
      case AVM_ROLE_CHG:
      {
         evt.evt.avm_role_msg.role_info.role_chg.role = ncs_decode_32bit(&data);

         ncs_dec_skip_space(uba, 3 * sizeof(uns32));
         evt.fsm_evt_type = AVM_ROLE_EVT_AVM_CHG;
         rc = avm_role_fsm_handler(cb, &evt);
      }
      break;

      case AVM_ROLE_RSP:
      {
         evt.evt.avm_role_msg.role_info.role_rsp.role = ncs_decode_32bit(&data);
         evt.evt.avm_role_msg.role_info.role_rsp.role_rsp = ncs_decode_32bit(&data); 
         ncs_dec_skip_space(uba, 3 * sizeof(uns32));
         evt.fsm_evt_type = AVM_ROLE_EVT_AVM_RSP;
         rc = avm_role_fsm_handler(cb, &evt);
      }
      break;

      default:
      {
         m_AVM_LOG_INVALID_VAL_FATAL(evt.evt.avm_role_msg.msg_type);
         rc = NCSCC_RC_FAILURE;
      }
   }

   return rc;
}   


/*****************************************************************************
*  Functiom: avm_mbcsv_process_en_notify_cb 
*
*  Purpose:  This function is the encode callback for all mbcsv notify 
*            messages. It will know, how to handle different peer versions.
*
* Input: arg - MBCSV callback argument pointer.
*
* Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*
* NOTES: None.
*
*
*****************************************************************************/
static uns32
avm_mbcsv_process_en_notify_cb(
                            AVM_CB_T        *cb,
                            NCS_MBCSV_CB_ARG   *arg
                        )
{
   NCS_UBAID      *uba = NULL;
   uns8           *data;
   AVM_ROLE_MSG_T *role_notify_msg = NULL;
   uns32          notify_type = 0;
   uns32          rc = NCSCC_RC_SUCCESS;
   
   m_AVM_LOG_FUNC_ENTRY("avm_mbcsv_process_en_notify_cb");
   /***** peer knows me - so just send as it is ******/
      
   uba = &arg->info.notify.i_uba;
   role_notify_msg = (AVM_ROLE_MSG_T *)arg->info.notify.i_msg;
   notify_type = role_notify_msg->msg_type;

   if(NCSCC_RC_SUCCESS != ncs_enc_init_space(uba))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   data = ncs_enc_reserve_space(uba, 3 * sizeof(uns32));

   /* Fill param according to notify type */
   if(notify_type == AVM_ROLE_CHG)
   {
      ncs_encode_32bit(&data, AVM_ROLE_CHG);
      ncs_encode_32bit(&data, role_notify_msg->role_info.role_chg.role);
      ncs_encode_32bit(&data, NCSCC_RC_SUCCESS);
   }
   else if(notify_type == AVM_ROLE_RSP)
   {
      ncs_encode_32bit(&data, AVM_ROLE_RSP);
      ncs_encode_32bit(&data, role_notify_msg->role_info.role_rsp.role);
      ncs_encode_32bit(&data, role_notify_msg->role_info.role_rsp.role_rsp);
   }
   else
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);

   ncs_enc_claim_space(uba, 3 * sizeof(uns32));

   return rc; 
}

extern uns32 
avm_send_role_chg_notify_msg(
                              AVM_CB_T           *cb,
                              SaAmfHAStateT       role
                          )
{
   NCS_MBCSV_ARG   mbcsv_arg;
   AVM_ROLE_MSG_T  role_notify_msg;

   m_AVM_LOG_FUNC_ENTRY("avm_send_role_chg_notify_msg");
   m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_AVM_ROLE, cb->ha_state, NCSFL_SEV_INFO);

   /* fill role notify msg */
   memset(&role_notify_msg, '\0', sizeof(AVM_ROLE_MSG_T));
   role_notify_msg.msg_type = AVM_ROLE_CHG;
   role_notify_msg.role_info.role_chg.role = role;

   /* fill mbcsv arg struct */
   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
   mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_NOTIFY;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;
   mbcsv_arg.info.send_notify.i_ckpt_hdl = cb->ckpt_hdl;
   mbcsv_arg.info.send_notify.i_msg_dest = NCS_MBCSV_ALL_PEERS;
   mbcsv_arg.info.send_notify.i_msg = (NCSCONTEXT)&role_notify_msg;


   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }   
   
   m_AVM_LOG_ROLE(AVM_LOG_SND_ROLE_CHG, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_INFO);
 
   return NCSCC_RC_SUCCESS;
}

extern uns32 
avm_send_role_rsp_notify_msg(
                              AVM_CB_T           *cb,
                              SaAmfHAStateT       role,
                              uns32               status   
                           )
{
   NCS_MBCSV_ARG   mbcsv_arg;
   AVM_ROLE_MSG_T  role_notify_msg;

   m_AVM_LOG_FUNC_ENTRY("avm_send_role_rsp_notify_msg");
   m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_AVM_ROLE, cb->ha_state, NCSFL_SEV_INFO);
   m_AVM_LOG_ROLE_OP(AVM_LOG_ROLE_CHG, status, NCSFL_SEV_INFO);

   /* fill role notify msg */
   memset(&role_notify_msg, '\0', sizeof(AVM_ROLE_MSG_T));
   role_notify_msg.msg_type = AVM_ROLE_RSP;
   role_notify_msg.role_info.role_rsp.role = role;
   role_notify_msg.role_info.role_rsp.role_rsp = status;

   /* fill mbcsv arg struct */
   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
   mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_NOTIFY;
   mbcsv_arg.i_mbcsv_hdl = cb->mbc_hdl;
   mbcsv_arg.info.send_notify.i_ckpt_hdl = cb->ckpt_hdl;
   mbcsv_arg.info.send_notify.i_msg_dest = NCS_MBCSV_ALL_PEERS;
   mbcsv_arg.info.send_notify.i_msg = (NCSCONTEXT)&role_notify_msg;

   if(NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }   
 
   m_AVM_LOG_ROLE_OP(AVM_LOG_SND_ROLE_RSP, cb->ha_state, NCSFL_SEV_INFO);
   return NCSCC_RC_SUCCESS;
}
