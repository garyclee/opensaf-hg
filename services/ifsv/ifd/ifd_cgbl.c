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


#include "ifd.h"
/*****************************************************************************
FILE NAME: IFSV_CGBL.C
DESCRIPTION: Interface Agent Routines.
******************************************************************************/

static uns32 ifsv_gbl_object_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data);

static uns32 ifsv_gbl_object_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);

static uns32 ifsv_gbl_object_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                   NCSCONTEXT *data, uns32* next_inst_id,
                                      uns32 *next_inst_id_len);

/******************************************************************************
  Function :  ifsv_gbl_object_get
  
  Purpose  :  Get function for Interface MIB 
 
  Input    :  NCSCONTEXT hdl,     - IFSV_CB pointer
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get request
                                  will be serviced

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
static uns32 ifsv_gbl_object_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   IFSV_CB           *cb = NULL;
   uns32             rc = NCSCC_RC_SUCCESS;
   
   /* Get the CB pointer from the CB handle */
   (cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key)) == 0 ? (cb = 
      ncshm_take_hdl(NCS_SERVICE_ID_IFND, arg->i_mib_key)):(cb = cb);

   if(cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   *data = cb;

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


/* Get Routine Called by the MIBLIB for ifmibobjects Table */
uns32 ifmibobjects_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   return (ifsv_gbl_object_get(hdl, arg, data));
}

/* Get Routine Called by the MIBLIB for interfaces Table */
uns32 intf_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   return (ifsv_gbl_object_get(hdl, arg, data));
}

/******************************************************************************
  Function :  ifsv_gbl_object_next
  
  Purpose  :  Get Next function for Interface MIB 
 
  Input    :  NCSCONTEXT hdl,     - IFSV_CB pointer
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get next 
                                  request will be serviced.

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
static uns32 ifsv_gbl_object_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                   NCSCONTEXT *data, uns32* next_inst_id,
                                      uns32 *next_inst_id_len)
{
   /* Issues with get next on scalars, return failure for time being, */
   return NCSCC_RC_FAILURE;
}

/* GetNext Routine Called by the MIBLIB for ifmibobjects Table */
uns32 ifmibobjects_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                        NCSCONTEXT *data, uns32* next_inst_id,
                                      uns32 *next_inst_id_len)
{
   return ifsv_gbl_object_next(hdl, arg, data, next_inst_id, next_inst_id_len);
}

/* GetNext Routine Called by the MIBLIB for interfaces Table */
uns32 intf_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                      NCSCONTEXT *data, uns32* next_inst_id,
                                      uns32 *next_inst_id_len)
{
   return ifsv_gbl_object_next(hdl, arg, data, next_inst_id, next_inst_id_len);
}


/******************************************************************************
  Function :  ifsv_gbl_object_set
  
  Purpose  :  Set function for IFSV interface table objects 
 
  Input    :  NCSCONTEXT hdl,            - CB handle
              NCSMIB_ARG *arg,           - MIB argument (input)
              NCSMIB_VAR_INFO* var_info, - Object properties/info(i)
              NCS_BOOL test_flag         - set/test operation (input)
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
static uns32 ifsv_gbl_object_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
   IFSV_CB            *cb;
   uns32              rc = NCSCC_RC_SUCCESS;
   NCS_BOOL           val_same_flag = FALSE;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   NCSMIB_SET_REQ     *i_set_req = &arg->req.info.set_req; 


      /* Get the CB pointer from the CB handle */
   (cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key)) == 0 ? (cb = 
      ncshm_take_hdl(NCS_SERVICE_ID_IFND, arg->i_mib_key)):(cb = cb);

   if (cb == NULL)
      return NCSCC_RC_FAILURE; 

   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   /* Set the object */
   if(test_flag != TRUE)
   {
      memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
      temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = (NCSCONTEXT)cb;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */ 
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      {
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsmiblib_process_req() returned failure,rc:",rc);
         ncshm_give_hdl(cb->cb_hdl);
         return rc;
      }
   }

   ncshm_give_hdl(cb->cb_hdl);
   return NCSCC_RC_SUCCESS;
}

/* Set Routine Called by the MIBLIB for ifmibobjects Table */
uns32 ifmibobjects_set (NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
   return (ifsv_gbl_object_set(hdl, arg, var_info, test_flag));
}

/* Set Routine Called by the MIBLIB for interfaces Table */
uns32 intf_set (NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
   return (ifsv_gbl_object_set(hdl, arg, var_info, test_flag));
}

uns32 ifmibobjects_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer)
{
   uns32 rc = NCSCC_RC_FAILURE;

   rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);

   return rc;
}

uns32 intf_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer)
{
   uns32 rc = NCSCC_RC_FAILURE;

   rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);

   return rc;
}

uns32 ifmibobjects_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}

uns32 intf_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}

