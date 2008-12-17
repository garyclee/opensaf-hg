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
 */

/*****************************************************************************
..............................................................................
  MODULE NAME: SNMPTM_SCALAR_MIB.C

..............................................................................
  
  DESCRIPTION:  This module contains functions used by the SNMPTM Subsystem for
                SNMP operations on the SNMPTM Enterprise SCALAR table.
..............................................................................

  snmptm_scalar_tbl_req .... Process the Request of an entp scalar table
  ncstestscalars_get ......... Get the object from the scalar table
  ncstestscalars_set ......... Set the object in the scalar table
  ncstestscalars_extract ..... Extract the object data from the scalar table.
  
******************************************************************************
*/

#include "snmptm.h"

#if(NCS_SNMPTM == 1) 

/****************************************************************************
  Name          :  snmptm_scalar_tbl_req
  
  Description   :  High Level MIB Access Routine for SNMPTM scalar table
 
  Arguments     :  struct ncsmib_arg*
    
  Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_scalar_tbl_req(struct ncsmib_arg *args)
{
   SNMPTM_CB           *snmptm = SNMPTM_CB_NULL;
   uns32               status  = NCSCC_RC_SUCCESS;
   uns32               cb_hdl = args->i_mib_key; 
   NCSMIBLIB_REQ_INFO  miblib_req; 


   /* Get the CB from the handle manager */
   if ((snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             cb_hdl)) == NULL)
   {
      args->rsp.i_status = NCSCC_RC_NO_INSTANCE;
      args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
      args->i_rsp_fnc(args);
   
      return NCSCC_RC_FAILURE;
   }  
   
   m_NCS_OS_MEMSET(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP; 
   miblib_req.info.i_mib_op_info.args = args; 
   miblib_req.info.i_mib_op_info.cb = snmptm; 
   
   m_SNMPTM_LOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);
   
   if((args->i_policy & NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER) ==
         NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER)
   {
      m_NCS_CONS_PRINTF("For TBLSCALAR, last playback update from PSS received...]n");
   }

   /* call the mib routine handler */ 
   status = ncsmiblib_process_req(&miblib_req); 
   
   m_SNMPTM_UNLOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);

   /* Release the SNMPTM CB handle */
   ncshm_give_hdl(cb_hdl);
   
   return status; 
}


/****************************************************************************
  Name          :  ncstestscalars_set
  
  Description   :  Set function for SNMPTM scalar table objects 
 
  Arguments     :  NCSCONTEXT cb,             - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,           - MIB argument (input)
                   NCSMIB_VAR_INFO* var_info, - Object properties/info(i)
                   NCS_BOOL test_flag         - set/test operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstestscalars_set(NCSCONTEXT cb, 
                         NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO *var_info,
                         NCS_BOOL test_flag)
{
   SNMPTM_CB       *snmptm = (SNMPTM_CB *)cb;
   NCS_BOOL        val_same_flag = FALSE;
   NCSMIB_SET_REQ  *i_set_req = &arg->req.info.set_req; 
   NCSMIB_SET_RSP  *io_set_rsp = &arg->rsp.info.set_rsp; 
   uns32           rc = NCSCC_RC_SUCCESS;

   m_NCS_CONS_PRINTF("\nncsTestScalars:  Received SNMP SET request\n\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg);

   if (test_flag != TRUE)
   { 
       NCSMIBLIB_REQ_INFO  temp_mib_req;         

       m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

       temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
       temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
       temp_mib_req.info.i_set_util_info.var_info = var_info;
       temp_mib_req.info.i_set_util_info.data = snmptm;
       temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

       /* call the mib routine handler */ 
       if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
           return rc;
   
       io_set_rsp->i_param_val = i_set_req->i_param_val;

   } /* test_flag */

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstestscalars_extract
  
  Description   :  Get the pointer of the data-structure from which the data to be 
                   extracted. 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get request
                                  will be serviced

  Return Values :  NCSCC_RC_SUCCESSS
 
  Notes         : 
****************************************************************************/
uns32 ncstestscalars_extract(NCSMIB_PARAM_VAL *param, 
                             NCSMIB_VAR_INFO *var_info,
                             NCSCONTEXT data,
                             NCSCONTEXT buffer)

{
   /* CB itself is a data-structure to extract the data from, so return
      the pointer of SNMPTM CB */
   return ncsmiblib_get_obj_val(param, var_info, data, buffer);
}


/****************************************************************************
  Name          :  ncstestscalars_get
  
  Description   :  Get the pointer of the data-structure from which the data to be 
                   extracted. 
 
  Arguments     :  param_val - pointer to NCSMIB_PARAM_VAL
                   var_info - pointer to NCSMIB_VAR_INFO
                   data - pointer to the data-structure from which the to be extracted
                   buffer - not used

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstestscalars_get(NCSCONTEXT snmptm, 
                         NCSMIB_ARG *arg,
                         NCSCONTEXT *data)
{
   /* CB itself is a data-structure to extract the data from, so return
      the pointer of SNMPTM CB */
   *data = snmptm;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstestscalars_setrow
  
  Description   :  Dummy function, it won't apply for this table since the table  
                   is of scalar objects.
 
  Arguments     :  NCSCONTEXT snmptm,            - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,           - MIB argument (input)
                   NCSMIB_SETROW_PARAM_VAL    - array of params to be set
                   ncsmib_obj_info* obj_info, - Object and table properties/info(i)
                   NCS_BOOL testrow_flag      - setrow/testrow operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS
 
  Notes         : 
****************************************************************************/
uns32 ncstestscalars_setrow(NCSCONTEXT cb, 
                            NCSMIB_ARG *args,
                            NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                            struct ncsmib_obj_info *obj_info,
                            NCS_BOOL testrow_flag)
{
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstestscalars_next
  
  Description   :  Dummy function, it won't apply for this table since the table  
                   is of scalar objects.
 
  Arguments     :  NCSCONTEXT snmptm,   - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,     - MIB argument (input)
                   NCSCONTEXT *data,    - pointer to the data from which next
                                          request will be serviced
                   uns32* next_inst_id  - next instance id of the object
                  
  Return Values :  NCSCC_RC_SUCCESSS
 
  Notes         : 
****************************************************************************/
uns32 ncstestscalars_next(NCSCONTEXT snmptm, 
                          NCSMIB_ARG *arg, 
                          NCSCONTEXT* data, 
                          uns32* next_inst_id,
                          uns32* next_inst_id_len) 
{
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstestscalars_rmvrow
                                                                                                                              
  Description   :  Remove-row function handler for SNMPTM Scalar table. This is
                   used to clear the scalar objects, before getting fresh MIB
                   SETs from PSSv.
                                                                                                                              
  Arguments     :  NCSCONTEXT snmptm,         - SNMPTM control block (input/output)
                   NCSMIB_IDX *idx,           - don't care
                                                                                                                              
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
                                                                                                                              
  Notes         :
****************************************************************************/
uns32 ncstestscalars_rmvrow(NCSCONTEXT cb_hdl, NCSMIB_IDX *idx)
{
   SNMPTM_CB           *snmptm = SNMPTM_CB_NULL;
   uns32               status = NCSCC_RC_SUCCESS;

   /* Get the CB from the handle manager */
   if ((snmptm = (SNMPTM_CB *)cb_hdl) == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   m_NCS_CONS_PRINTF("\nncsTestScalars:  Received SNMP REMOVEROWS request\n\n");

   /* Now, initialize(memset) the scalar row */
   m_NCS_OS_MEMSET(&snmptm->scalars, '\0', sizeof(snmptm->scalars));

   return status;
}

#endif /* NCS_SNMPTM */
