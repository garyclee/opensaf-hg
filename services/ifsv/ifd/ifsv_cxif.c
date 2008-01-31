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
FILE NAME: IFSV_CXIF.C
DESCRIPTION: Interface Agent Routines.
******************************************************************************/


/******************************************************************************
  Function :  ncsifsvifxentry_get
  
  Purpose  :  Get function for Interface MIB 
 
  Input    :  NCSCONTEXT hdl,     - IFSV_CB pointer
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get request
                                  will be serviced

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvifxentry_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   IFSV_CB           *cb = NULL;
   uns32             rc = NCSCC_RC_SUCCESS;
   NCS_IFSV_IFINDEX  ifkey = 0;
   
   /* Get the CB pointer from the CB handle */
   cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);

   if(cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   /* Validate instance ID and get Key from Inst ID */
   rc = ifsv_ifkey_from_instid(arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len, 
      &ifkey);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifkey_from_instid() : ncsifsvifxentry_get returned failure"," ");
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   /* Reset the global if_rec stored in CB */
   m_NCS_OS_MEMSET(&cb->ifmib_rec, 0, sizeof(NCS_IFSV_INTF_REC));

   /* Get the record for the given ifKey */
   rc = ifsv_ifrec_get(cb, ifkey, &cb->ifmib_rec);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifrec_get() : ncsifsvifxentry_get returned failure"," ");
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   *data = &cb->ifmib_rec;

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


/******************************************************************************
  Function :  ncsifsvifxentry_next
  
  Purpose  :  Get Next function for Interface MIB 
 
  Input    :  NCSCONTEXT hdl,     - IFSV_CB pointer
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get next 
                                  request will be serviced.

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvifxentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                   NCSCONTEXT *data, uns32* next_inst_id,
                                      uns32 *next_inst_id_len)
{
   IFSV_CB           *cb;
   uns32             rc = NCSCC_RC_SUCCESS;
   NCS_IFSV_IFINDEX  ifkey = 0;

   /* Get the CB pointer from the CB handle */
   cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);

   if(arg->i_idx.i_inst_len)
   {
      /* Validate instance ID and get Key from Inst ID */
      rc = ifsv_ifkey_from_instid(arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len, 
         &ifkey);

      if(rc != NCSCC_RC_SUCCESS)
      {
         /* RSR:TBD Log the error */
         m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifkey_from_instid() : ncsifsvifxentry_next returned failure"," ");
         ncshm_give_hdl(cb->cb_hdl);
         return rc;
      }
   }
   /* Reset the global if_rec stored in CB */
   m_NCS_OS_MEMSET(&cb->ifmib_rec, 0, sizeof(NCS_IFSV_INTF_REC));

   /* Get the record for the given ifKey */
   rc = ifsv_ifrec_getnext(cb, ifkey, &cb->ifmib_rec);
   

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifrec_getnext() : ncsifsvifxentry_next returned failure"," ");
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   *data = &cb->ifmib_rec;

   /* Populate next instance ID */
   *next_inst_id_len = IFSV_IFINDEX_INST_LEN;
   next_inst_id[0] = cb->ifmib_rec.if_index;

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


/******************************************************************************
  Function :  ncsifsvifxentry_set
  
  Purpose  :  Set function for IFSV interface table objects 
 
  Input    :  NCSCONTEXT hdl,            - CB handle
              NCSMIB_ARG *arg,           - MIB argument (input)
              NCSMIB_VAR_INFO* var_info, - Object properties/info(i)
              NCS_BOOL test_flag         - set/test operation (input)
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvifxentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
   return NCSCC_RC_FAILURE;
}

uns32 ncsifsvifxentry_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer)
{
   uns32 rc = NCSCC_RC_FAILURE;

   rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);

   return rc;
}

uns32 ncsifsvifxentry_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}

uns32 ncsifsvifxentry_verify_instance(NCSMIB_ARG* args)
{
   /* TBD:RSR Validation Checks */
   return NCSCC_RC_SUCCESS;
}

