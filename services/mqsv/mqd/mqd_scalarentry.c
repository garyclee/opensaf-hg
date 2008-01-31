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

#include "mqd.h"

/********************************************************************
 * Name        : safmsgscalarobject_set
 *
 * Description : 
 *
 *******************************************************************/

uns32 safmsgscalarobject_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSMIB_VAR_INFO* var_info,
                              NCS_BOOL test_flag)
{
   MQD_CB          *pcb = (MQD_CB *)cb;
   NCS_BOOL        val_same_flag = FALSE;
   NCSMIB_SET_REQ  *i_set_req = &arg->req.info.set_req;
   NCSMIB_SET_RSP  *io_set_rsp = &arg->rsp.info.set_rsp;
   uns32           rc = NCSCC_RC_SUCCESS;


   if (test_flag != TRUE)
   {
       NCSMIBLIB_REQ_INFO  temp_mib_req;

       m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

       temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP;
       temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
       temp_mib_req.info.i_set_util_info.var_info = var_info;
       temp_mib_req.info.i_set_util_info.data = pcb;
       temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

       /* call the mib routine handler */
       if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS)
           return rc;

       io_set_rsp->i_param_val = i_set_req->i_param_val;

   } /* test_flag */

   return NCSCC_RC_SUCCESS;
}



/***********************************************************************
 * Name        : safmsgscalarobject_setrow
 *
 * Description :
 *
 **********************************************************************/

uns32 safmsgscalarobject_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                 NCSMIB_SETROW_PARAM_VAL* params,
                                 struct ncsmib_obj_info* obj_info,
                                 NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}



/**************************************************************************
 * Name         :  safmsgscalarobject_extract
 *
 * Description  :
 *
 **************************************************************************/

uns32 safmsgscalarobject_extract(NCSMIB_PARAM_VAL* param,
                                  NCSMIB_VAR_INFO* var_info,
                                  NCSCONTEXT data,
                                  NCSCONTEXT buffer)
{

   uns32 rc = NCSCC_RC_SUCCESS;
   MQD_CB *cb = (MQD_CB *)data;

   switch(var_info->param_id)
   {
     case safSpecVersion_ID:
        {   
      param->i_fmat_id = NCSMIB_FMAT_OCT;
      param->i_length  = cb->safSpecVer.length;
      m_NCS_MEMCPY((uns8 *)buffer,cb->safSpecVer.value,param->i_length);
      param->info.i_oct     = (uns8*)buffer;
      break;
        }
     case safAgentVendor_ID:
        {
      param->i_fmat_id = NCSMIB_FMAT_OCT;
      param->i_length  = cb->safAgtVen.length;
      m_NCS_MEMCPY((uns8 *)buffer,cb->safAgtVen.value,param->i_length);
      param->info.i_oct     = (uns8*)buffer;
      break;  
        }
     case safServiceStartEnabled_ID:
        {
      param->info.i_int = (cb->serv_enabled== 1)?1 :2;
      param->i_length =4;
      param->i_fmat_id = NCSMIB_FMAT_INT;
      break;
        }   
     default:
        {
      rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
        }
   }

   return rc;
}

/***************************************************************************************
 * Name        :  safmsgscalarobject_get
 *
 * Description :  
 *
 **************************************************************************************/


uns32  safmsgscalarobject_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
   MQD_CB *pcb;

   pcb = (MQD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQD, arg->i_mib_key);

   if (pcb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   *data = (NCSCONTEXT)pcb;
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************************
 * Name        : safmsgscalarobject_next
 *
 * Description :
 *
 ***************************************************************************************/

uns32  safmsgscalarobject_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
                                NCSCONTEXT *data, uns32* next_inst_id,
                                uns32 *next_inst_id_len)
{

    return NCSCC_RC_FAILURE;
}

