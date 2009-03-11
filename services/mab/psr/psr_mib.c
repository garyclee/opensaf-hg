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

    This has the mib specific functions of the Persistent Store Restore (PSR), 
    a subcomponent of the MIB Access Broker (MAB) subystem.This file 
    contains these groups of private functions

  - Table registration function for registering mib table and filters with OAC
  - OAC callback function for processing MIB requests

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#if (NCS_MAB == 1)
#if (NCS_PSR == 1)
#include "psr.h"


/*****************************************************************************

  PROCEDURE NAME:    pss_register_with_oac

  DESCRIPTION:       This function registers PSS's own MIBs with OAA.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 pss_register_with_oac(PSS_CB * inst)
{
    NCSOAC_SS_ARG   mab_arg;

    /* Register the PSS Profile-MIB table with OAA */
    memset(&mab_arg, 0, sizeof(mab_arg));
    mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
    mab_arg.i_tbl_id = NCSMIB_TBL_PSR_PROFILES;
    mab_arg.i_oac_hdl = inst->oac_key;
    mab_arg.info.tbl_owned.i_mib_req = pss_mib_request;
    mab_arg.info.tbl_owned.i_mib_key = inst->hm_hdl;
    mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_PSS;
    if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;          
    mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
    mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;
    if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    inst->profile_tbl_row_hdl = mab_arg.info.row_owned.o_row_hdl;

    /* Register the PSS Trigger-MIB table with OAA */
    memset(&mab_arg, 0, sizeof(mab_arg));
    mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
    mab_arg.i_tbl_id = NCSMIB_SCLR_PSR_TRIGGER;
    mab_arg.i_oac_hdl = inst->oac_key;
    mab_arg.info.tbl_owned.i_mib_req = pss_mib_request;
    mab_arg.info.tbl_owned.i_mib_key = inst->hm_hdl;
    mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_PSS;
    if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;          
    mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
    mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;
    if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    inst->trigger_scl_row_hdl = mab_arg.info.row_owned.o_row_hdl;

    /* Register the PSS-command table with OAA */
    memset(&mab_arg, 0, sizeof(mab_arg));
    mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
    mab_arg.i_tbl_id = NCSMIB_TBL_PSR_CMD;
    mab_arg.i_oac_hdl = inst->oac_key;
    mab_arg.info.tbl_owned.i_mib_req = pss_mib_request;
    mab_arg.info.tbl_owned.i_mib_key = inst->hm_hdl;
    mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_PSS;
    if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;          
    mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
    mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;
    if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    inst->cmd_tbl_row_hdl = mab_arg.info.row_owned.o_row_hdl;

    /* Register MIBs with MIBLIB */
    ncspssvscalars_tbl_reg();
    ncspssvprofiletableentry_tbl_reg();

    return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:    pss_unregister_with_oac

  DESCRIPTION:       This function deregisters PSS's own MIBs with OAA.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 pss_unregister_with_oac(PSS_CB * inst)
{
    NCSOAC_SS_ARG   mab_arg;

    if (inst->profile_tbl_row_hdl != 0)
    {
        memset(&mab_arg, 0, sizeof(mab_arg));
        mab_arg.i_op = NCSOAC_SS_OP_ROW_GONE;
        mab_arg.i_tbl_id = NCSMIB_TBL_PSR_PROFILES;
        mab_arg.i_oac_hdl = inst->oac_key;
        mab_arg.info.row_gone.i_row_hdl = inst->profile_tbl_row_hdl;
        if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

        inst->profile_tbl_row_hdl = 0;

        mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;
        mab_arg.info.tbl_gone.i_meaningless = (void *)0x1234;
        if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    if (inst->trigger_scl_row_hdl != 0)
    {
        memset(&mab_arg, 0, sizeof(mab_arg));
        mab_arg.i_op = NCSOAC_SS_OP_ROW_GONE;
        mab_arg.i_tbl_id = NCSMIB_SCLR_PSR_TRIGGER;
        mab_arg.i_oac_hdl = inst->oac_key;
        mab_arg.info.row_gone.i_row_hdl = inst->trigger_scl_row_hdl;
        if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

        inst->trigger_scl_row_hdl = 0;

        mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;
        mab_arg.info.tbl_gone.i_meaningless = (void *)0x1234;
        if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    
    if (inst->cmd_tbl_row_hdl != 0)
    {
        memset(&mab_arg, 0, sizeof(mab_arg));
        mab_arg.i_op = NCSOAC_SS_OP_ROW_GONE;
        mab_arg.i_tbl_id = NCSMIB_TBL_PSR_CMD;
        mab_arg.i_oac_hdl = inst->oac_key;
        mab_arg.info.row_gone.i_row_hdl = inst->cmd_tbl_row_hdl;
        if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

        inst->cmd_tbl_row_hdl = 0;

        mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;
        mab_arg.info.tbl_gone.i_meaningless = (void *)0x1234;
        if(ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:    pss_mib_request

  DESCRIPTION:       MIB handler function 

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 pss_mib_request (struct ncsmib_arg *mib_args)
{
    PSS_CB * inst;
    MAB_MSG * mab_msg;
    uns32     hm_hdl;

    if (mib_args == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    hm_hdl = (uns32)mib_args->i_mib_key;
    if ((inst = (PSS_CB*)m_PSS_VALIDATE_HDL(mib_args->i_mib_key)) == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    mab_msg = m_MMGR_ALLOC_MAB_MSG;
    if (mab_msg == NULL)
    {
        ncshm_give_hdl(hm_hdl);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    memset(mab_msg, 0, sizeof(MAB_MSG));
    mab_msg->yr_hdl  = NCS_INT32_TO_PTR_CAST(inst->hm_hdl);
    mab_msg->op      = MAB_PSS_MIB_REQUEST;

    mab_msg->data.data.snmp = ncsmib_memcopy(mib_args);
    if (mab_msg->data.data.snmp == NULL)
    {
        ncshm_give_hdl(hm_hdl);
        m_MMGR_FREE_MAB_MSG(mab_msg);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    /* Put it in PSR's work queue */

    if(m_PSS_SND_MSG(inst->mbx, mab_msg) == NCSCC_RC_FAILURE)
    {
        ncshm_give_hdl(hm_hdl);
        ncsmib_memfree(mab_msg->data.data.snmp);
        m_MMGR_FREE_MAB_MSG(mab_msg);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    ncshm_give_hdl(hm_hdl);
    return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:    ncspssvprofiletableentry_get

  DESCRIPTION:       This function processes a get request


  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 ncspssvprofiletableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                  NCSCONTEXT* data)
{
    /* Get Request will be used only for profile description */
    PSS_CB *inst = NULL;
    uns32 retval = NCSCC_RC_SUCCESS;
    uns8  profile_name[NCS_PSS_MAX_PROFILE_NAME];

    inst = (PSS_CB*)cb;
    if((arg->req.info.get_req.i_param_id != ncsPSSvProfileDesc_ID) &&
       (arg->req.info.get_req.i_param_id != ncsPSSvProfileTableStatus_ID))
    {
        return NCSCC_RC_NO_INSTANCE;
    }

    if (arg->i_idx.i_inst_len != 0)
    {
        pss_get_profile_from_inst_ids(profile_name, arg->i_idx.i_inst_len,
                                  arg->i_idx.i_inst_ids);
    }
    else
    {
        /* m_NCS_PSSTS_GET_NEXT_PROFILE(inst->pssts_api, inst->pssts_hdl, retval,
                                    NULL, sizeof(profile_name), profile_name); */
        return NCSCC_RC_INV_VAL;
    }
    if (retval != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_NO_INSTANCE;
    }

    memset(&inst->profile_extract, '\0', sizeof(inst->profile_extract));
    strcpy((char*)&inst->profile_extract, (char*)&profile_name);

    *data = (NCSCONTEXT)inst;

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 *
 * PROCEDURE NAME:    ncspssvprofiletableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * Profile table. The name of this function is generated by the MIBLIB tool. This 
 * function will be called by MIBLIB after calling the get call to get data structure.
 * This function fills the value information in the param filed structure. For
 * octate information the buffer field will be used for filling the information.
 * MIBLIB will provide the memory and pointer to the buffer. For only objects that 
 * have a direct value(i.e their offset is not 0 in VAR INFO) in the structure
 * the data field is filled using the VAR INFO provided by MIBLIB, for others based
 * on the OID the value is filled accordingly.
 * 
 * Input:  param     -  param->i_param_id indicates the parameter to extract
 *                      The remaining elements of the param need to be filled
 *                      by the subystem's extract function
 *         var_info  - Pointer to the var_info structure for the param.
 *         data      - The pointer to the data-structure containing the object
 *                     value which we have already provided to MIBLIB from get call.
 *         buffer    - The buffer pointer provided by MIBLIB for filling the octate
 *                     type data.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 * NOTES:  This function works in conjunction with other functions to provide the
 * get,getnext and getrow functionality.
 *
 *
 **************************************************************************/
uns32 ncspssvprofiletableentry_extract(NCSMIB_PARAM_VAL* param,
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   PSS_CB *inst = (PSS_CB *)data;
   uns32 retval = NCSCC_RC_SUCCESS;
   uns8  profile_name[NCS_PSS_MAX_PROFILE_NAME];
   uns8  profile_desc[NCS_PSS_MAX_PROFILE_DESC];
   NCS_BOOL profile_exists = FALSE, desc_exists = FALSE;
 
   if (inst == NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   if((param->i_param_id != ncsPSSvProfileTableStatus_ID) &&
      (param->i_param_id != ncsPSSvProfileDesc_ID))
   {
      return NCSCC_RC_NO_INSTANCE;
   }

   memset(profile_name, '\0', sizeof(profile_name));
   memset(profile_desc, '\0', sizeof(profile_desc));
   m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl,
                              retval, inst->profile_extract, profile_exists);

   if((retval != NCSCC_RC_SUCCESS) || (profile_exists == FALSE))
   {
      return NCSCC_RC_NO_OBJECT;
   }
   if(param->i_param_id == ncsPSSvProfileTableStatus_ID)
   {
      param->i_fmat_id  = NCSMIB_FMAT_INT;
      param->i_length  = 4;
      param->info.i_int = NCSMIB_ROWSTATUS_ACTIVE;
      return NCSCC_RC_SUCCESS;
   }

   m_NCS_PSSTS_GET_DESC(inst->pssts_api, inst->pssts_hdl, retval,
                        inst->profile_extract, sizeof(profile_desc), profile_desc, desc_exists);
   if (retval != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }

   param->info.i_oct = NULL;
   if(desc_exists == TRUE)
   {
      param->i_length = (uns16) strlen((const char *) profile_desc);
      memcpy((uns8*)buffer, (uns8*)profile_desc, param->i_length); /* Using buffer passed from MIBLIB to return OCT value. */
      param->i_fmat_id = NCSMIB_FMAT_OCT;
      param->info.i_oct = (uns8 *)buffer; /* This is not required to be freed. */
   }
   else
   {
      ((uns8*)buffer)[0] = '\0';
      param->i_length = 0;
      param->i_fmat_id = NCSMIB_FMAT_OCT;
      param->info.i_oct = NULL; /* This is not used anyway */
   }

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncspssvprofiletableentry_set

  DESCRIPTION:       This function processes a set request


  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 ncspssvprofiletableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
    /* The Set request will contain the description for the profile or
     * The row status will be set to CREATE_GO or DESTROY to create or
     * delete a profile respectively. */
    PSS_CB *inst = (PSS_CB*)cb;
    uns32 retval = NCSCC_RC_SUCCESS;
    uns8  profile_name[NCS_PSS_MAX_PROFILE_NAME];
    uns8  desc_text[NCS_PSS_MAX_PROFILE_DESC];
    NCS_BOOL profile_exists;

    memset(profile_name, '\0', sizeof(profile_name));
    pss_get_profile_from_inst_ids(profile_name, arg->i_idx.i_inst_len,
                                  arg->i_idx.i_inst_ids);

    switch (arg->req.info.set_req.i_param_val.i_param_id)
    {
    case ncsPSSvProfileDesc_ID:
        {
            arg->i_op = NCSMIB_OP_RSP_SET;
            m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl,
                              retval, profile_name, profile_exists);

            if (retval != NCSCC_RC_SUCCESS)
            {
                return NCSCC_RC_INV_VAL;
            }
            if(arg->req.info.set_req.i_param_val.i_length == 0)
            {
                   /* Don't allow NULL string set operation for Description. */
                   return NCSCC_RC_INV_VAL;
            }
            if(test_flag == TRUE)
            {
                return NCSCC_RC_SUCCESS;
            }

            if (profile_exists == FALSE)
            {
                /* Save the current profile to the disk */
                retval = pss_save_current_configuration(inst);
                if (retval != NCSCC_RC_SUCCESS)
                {
                   return NCSCC_RC_INV_VAL;
                }
                /* Create a new profile */
                m_NCS_PSSTS_PROFILE_COPY(inst->pssts_api, inst->pssts_hdl,
                                          retval, profile_name, inst->current_profile);

                if (retval != NCSCC_RC_SUCCESS)
                {
                   return NCSCC_RC_INV_VAL;
                }
            }

            memcpy(&desc_text, (uns8*)arg->req.info.set_req.i_param_val.info.i_oct,
               arg->req.info.set_req.i_param_val.i_length);
            m_NCS_PSSTS_SET_DESC(inst->pssts_api, inst->pssts_hdl, retval, profile_name, (char*)&desc_text);
            if (retval != NCSCC_RC_SUCCESS)
            {
               return NCSCC_RC_INV_VAL;
            }

            memcpy(&arg->rsp.info.set_rsp.i_param_val,
                        &arg->req.info.set_req.i_param_val,
                        sizeof(NCSMIB_PARAM_VAL));
        }
        break;

    case ncsPSSvProfileTableStatus_ID: /* Either create or destroy a profile */
        {
            arg->i_op = NCSMIB_OP_RSP_SET;
            switch (arg->req.info.set_req.i_param_val.info.i_int)
            {
            case NCSMIB_ROWSTATUS_CREATE_GO:
                {
                    if(strcmp(&inst->current_profile, profile_name) == 0)
                    {
                       return NCSCC_RC_INV_VAL;
                    }
                    m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl,
                                      retval, profile_name, profile_exists);

                    if (retval != NCSCC_RC_SUCCESS)
                    {
                       return NCSCC_RC_FAILURE;
                    }

                    if (profile_exists == TRUE)
                    {
                       return NCSCC_RC_INV_VAL;
                    }
                    else if(test_flag == TRUE)
                       return NCSCC_RC_SUCCESS;

                    retval = pss_save_current_configuration(inst);
                    if (retval != NCSCC_RC_SUCCESS)
                    {
                       return NCSCC_RC_FAILURE;
                    }

                    m_NCS_PSSTS_PROFILE_COPY(inst->pssts_api, inst->pssts_hdl,
                                            retval, profile_name,
                                            inst->current_profile);
                    if (retval != NCSCC_RC_SUCCESS)
                    {
                       return NCSCC_RC_FAILURE;
                    }

                    memcpy(&arg->rsp.info.set_rsp.i_param_val,
                                &arg->req.info.set_req.i_param_val,
                                sizeof(NCSMIB_PARAM_VAL));
                }
                break;

            case NCSMIB_ROWSTATUS_DESTROY:
                {
                    if(strcmp(&inst->current_profile, profile_name) == 0)
                    {
                       return NCSCC_RC_INV_VAL;
                    }
                    m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl,
                                      retval, profile_name, profile_exists);

                    if (retval != NCSCC_RC_SUCCESS)
                    {
                       return NCSCC_RC_INV_VAL;
                    }

                    if(test_flag == TRUE)
                       return NCSCC_RC_SUCCESS;
                    if (profile_exists == FALSE)
                    {
                       return NCSCC_RC_INV_VAL;
                    }

                    m_NCS_PSSTS_PROFILE_DELETE(inst->pssts_api, inst->pssts_hdl,
                                              retval, profile_name);
                    if (retval != NCSCC_RC_SUCCESS)
                    {
                       return NCSCC_RC_INV_VAL;
                    }

                    memcpy(&arg->rsp.info.set_rsp.i_param_val,
                                &arg->req.info.set_req.i_param_val,
                                sizeof(NCSMIB_PARAM_VAL));
                }
                break;

            default:
                return NCSCC_RC_INV_VAL;
            } /* switch (arg->req.info.set_req.i_param_val.info.i_int) */
        }
        break;

    default:
        return NCSCC_RC_INV_VAL;
    }

    return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:    ncspssvprofiletableentry_next

  DESCRIPTION:       This function processes a next request


  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 ncspssvprofiletableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
    PSS_CB *inst = (PSS_CB*)cb;
    uns32 retval = NCSCC_RC_SUCCESS;
    uns8  profile_name[NCS_PSS_MAX_PROFILE_NAME];
    uns8  profile_next[NCS_PSS_MAX_PROFILE_NAME];
    uns8  profile_desc[NCS_PSS_MAX_PROFILE_DESC];
    uns32 len, i;

    memset(profile_name, '\0', sizeof(profile_name));
    memset(profile_next, '\0', sizeof(profile_next));
    memset(profile_desc, '\0', sizeof(profile_desc));

    arg->i_op = NCSMIB_OP_RSP_NEXT;
    if((arg->req.info.next_req.i_param_id != ncsPSSvProfileDesc_ID) &&
       (arg->req.info.next_req.i_param_id != ncsPSSvProfileTableStatus_ID))
    {
       return NCSCC_RC_NO_INSTANCE;
    }
    if (arg->i_idx.i_inst_len != 0)
    {
        pss_get_profile_from_inst_ids(profile_name, arg->i_idx.i_inst_len,
                                  arg->i_idx.i_inst_ids);
        m_NCS_PSSTS_GET_NEXT_PROFILE(inst->pssts_api, inst->pssts_hdl, retval,
                                    profile_name, sizeof(profile_next), profile_next);
    }
    else
        m_NCS_PSSTS_GET_NEXT_PROFILE(inst->pssts_api, inst->pssts_hdl, retval,
                                    NULL, sizeof(profile_next), profile_next);
    if (retval != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_NO_INSTANCE;
    }

    *data = (NCSCONTEXT)inst;
    strcpy((char*)&inst->profile_extract, (char*)&profile_next);

    len = strlen((const char *)profile_next);
    next_inst_id[0] = len;
    *next_inst_id_len = len + 1; /* Including the length - first oid */
    for (i = 0; i < len; i++)
        next_inst_id[i + 1] = (uns32) profile_next[i];

    return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:    ncspssvprofiletableentry_setrow

  DESCRIPTION:       This function processes a setrow request


  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 ncspssvprofiletableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}


/****************************************************************************
  Name          :  ncspssvscalars_set
  
  Description   :  Set function for PSSv scalar table objects 

  Arguments     :  NCSCONTEXT cb,             - PSS_CB control block (input/output)
                   NCSMIB_ARG *arg,           - MIB argument (input)
                   NCSMIB_VAR_INFO* var_info, - Object properties/info(i)
                   NCS_BOOL test_flag         - set/test operation (input)

  Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE

  Notes         : 
****************************************************************************/
uns32 ncspssvscalars_set(NCSCONTEXT cb,
                         NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO *var_info,
                         NCS_BOOL test_flag)
{
   PSS_CB          *inst = (PSS_CB *)cb;
   uns32           retval = NCSCC_RC_SUCCESS;

   switch (arg->req.info.set_req.i_param_val.i_param_id)
   {
   case ncsPSSvExistingProfile_ID:
        if(arg->req.info.set_req.i_param_val.i_length == 0)
        {
           /* Don't allow NULL string set operation. */
           return NCSCC_RC_INV_VAL;
        }
        if(test_flag)
        {
           m_LOG_PSS_HEADLINE2(NCSFL_SEV_INFO, PSS_HDLN2_TEST_EXST_PRO);
        }
        else
        {
           m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_SET_EXST_PRO);
           memcpy((char *)inst->existing_profile,
                    (char *)arg->req.info.set_req.i_param_val.info.i_oct,
                    arg->req.info.set_req.i_param_val.i_length);
           if(arg->req.info.set_req.i_param_val.i_length < sizeof(inst->existing_profile))
              memset(((char *)inst->existing_profile + arg->req.info.set_req.i_param_val.i_length),
                 '\0', (sizeof(inst->existing_profile) - arg->req.info.set_req.i_param_val.i_length));

           memcpy(&arg->rsp.info.set_rsp.i_param_val,
                    &arg->req.info.set_req.i_param_val,
                    sizeof(NCSMIB_PARAM_VAL));
        }
        break;

   case ncsPSSvNewProfile_ID:
        if(arg->req.info.set_req.i_param_val.i_length == 0)
        {
           /* Don't allow NULL string set operation. */
           return NCSCC_RC_INV_VAL;
        }
        if(test_flag)
        {
           m_LOG_PSS_HEADLINE2(NCSFL_SEV_INFO, PSS_HDLN2_TEST_NEW_PRO);
        }
        else
        {
           m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_SET_NEW_PRO);
           memset((char *)inst->new_profile, '\0', sizeof(inst->new_profile));
           memcpy((char *)inst->new_profile,
                    (char *)arg->req.info.set_req.i_param_val.info.i_oct,
                    arg->req.info.set_req.i_param_val.i_length);
           memcpy(&arg->rsp.info.set_rsp.i_param_val,
                    &arg->req.info.set_req.i_param_val,
                    sizeof(NCSMIB_PARAM_VAL));
        }
        break;

   case ncsPSSvTriggerOperation_ID:
        if(test_flag)
        {
           m_LOG_PSS_HEADLINE2(NCSFL_SEV_INFO, PSS_HDLN2_TEST_TRIGGER_OP);
           retval = NCSCC_RC_SUCCESS;
        }
        else
        {
           m_LOG_PSS_HEADLINE2(NCSFL_SEV_INFO, PSS_HDLN_SET_CUR_PRO);
           retval = pss_process_trigger_op(inst,
                                        arg->req.info.set_req.i_param_val.info.i_int);
           memcpy(&arg->rsp.info.set_rsp.i_param_val,
                    &arg->req.info.set_req.i_param_val,
                    sizeof(NCSMIB_PARAM_VAL));
        }
        break;

   case ncsPSSvCurrentProfile_ID:
        if(test_flag)
        {
           m_LOG_PSS_HEADLINE2(NCSFL_SEV_INFO, PSS_HDLN2_TEST_CUR_PRO);
        }
        else
        {
           m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_SET_CUR_PRO);
           memset((char *)inst->current_profile, '\0', sizeof(inst->current_profile));
           memcpy((char *)inst->current_profile,
                    (char *)arg->req.info.set_req.i_param_val.info.i_oct,
                    arg->req.info.set_req.i_param_val.i_length);
           m_NCS_PSSTS_SET_PSS_CONFIG(inst->pssts_api, inst->pssts_hdl, retval,
                                  inst->current_profile);
           memcpy(&arg->rsp.info.set_rsp.i_param_val,
                    &arg->req.info.set_req.i_param_val,
                    sizeof(NCSMIB_PARAM_VAL));
        }
        break;

   default:
        return NCSCC_RC_INV_VAL;
   }

   return retval;
}

/****************************************************************************
  Name          :  ncspssvscalars_extract

  Description   :  Get the pointer of the data-structure from which the data to be
                   extracted. 

  Arguments     :  

  Return Values :  NCSCC_RC_SUCCESSS
 
  Notes         :
****************************************************************************/
uns32 ncspssvscalars_extract(NCSMIB_PARAM_VAL *param, 
                             NCSMIB_VAR_INFO *var_info,
                             NCSCONTEXT data,
                             NCSCONTEXT buffer)

{
   PSS_CB *inst = (PSS_CB*)data;

    switch (param->i_param_id)
    {
    case ncsPSSvExistingProfile_ID:
       param->i_fmat_id = NCSMIB_FMAT_OCT;
       param->i_length = strlen(inst->existing_profile);
       memcpy((uns8 *)buffer, inst->existing_profile, param->i_length);
       param->info.i_oct = (uns8 *)buffer;
       break;

    case ncsPSSvNewProfile_ID:
       param->i_fmat_id = NCSMIB_FMAT_OCT;
       param->i_length = strlen(inst->new_profile);
       memcpy((uns8 *)buffer, inst->new_profile, param->i_length);
       param->info.i_oct = (uns8 *)buffer;
       break;

    case ncsPSSvCurrentProfile_ID:
       param->i_fmat_id = NCSMIB_FMAT_OCT;
       param->i_length = strlen(inst->current_profile);
       memcpy((uns8 *)buffer, inst->current_profile, param->i_length);
       param->info.i_oct = (uns8 *)buffer;
       break;

    case ncsPSSvTriggerOperation_ID:
       param->i_fmat_id  = NCSMIB_FMAT_INT;
       param->info.i_int = ncsPSSvTriggerNoop;
       param->i_length = 4;
       break;

    default:
       return NCSCC_RC_NO_OBJECT;
    }

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncspssvscalars_get
  
  Description   :  Get the pointer of the data-structure from which the data to be
                   extracted. 

  Arguments     :  

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
  
  Notes         :
****************************************************************************/
uns32 ncspssvscalars_get(NCSCONTEXT inst, 
                         NCSMIB_ARG *arg,
                         NCSCONTEXT *data)
{
   /* CB itself is a data-structure to extract the data from, so return
      the pointer of SNMPTM CB */
   *data = inst;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          :  ncspssvscalars_next
 * 
 * Purpose:  This function is the next processing for objects in
 * PSSv Scalar table. This is a dummy function as sub agent shouldnt
 * call next operation for scalar objects.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context
 *
 * NOTES: This function works in conjunction with extract function to provide the
 * getnext functionality.
 **************************************************************************/
uns32 ncspssvscalars_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   /* Invalid instance */
   return NCSCC_RC_NO_INSTANCE;
}

/*****************************************************************************
 * Function: ncspssvscalars_setrow
 *
 * Purpose:  This function is the setrow processing for scalar objects in
 * PSSv Scalar table. This is a dummy function as sub agent shouldnt allow
 * row operations on scalar.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 **************************************************************************/
uns32 ncspssvscalars_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{  
   /* Invalid instance */
   return NCSCC_RC_NO_INSTANCE;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_trigger_op

  DESCRIPTION:       This function process SNMP requests on the trigger scalar


  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 pss_process_trigger_op(PSS_CB * inst, PSR_TRIGGER_VALUES val)
{
    uns32 retval = NCSCC_RC_SUCCESS;
    NCS_BOOL profile_exists;
#if (NCS_PSS_RED == 1)
    PSS_PWE_CB *pwe_cb = NULL;
#endif

    switch(val)
    {
    case ncsPSSvTriggerNoop:
        m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_CLI_REQ_NOOP);
        return NCSCC_RC_SUCCESS;

    case ncsPSSvTriggerLoad:
        m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_CLI_REQ_LOAD);
        m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval,
                                  inst->existing_profile, profile_exists);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        if (profile_exists == FALSE)
            return m_MAB_DBG_SINK(NCSCC_RC_NO_INSTANCE);

        if(strcmp(&inst->current_profile, &inst->existing_profile) == 0)
        {
           /* We can't overwrite current profile over current profile */
           return m_MAB_DBG_SINK(NCSCC_RC_INV_VAL);
        }
#if (NCS_PSS_RED == 1)
        {
           pwe_cb = gl_pss_amf_attribs.csi_list->pwe_cb;

           /* Write this request information into pwe_cb the first time. */
           memset(&pwe_cb->curr_plbck_ssn_info, '\0', sizeof(PSS_CURR_PLBCK_SSN_INFO));
           strcpy(&pwe_cb->curr_plbck_ssn_info.info.alt_profile, 
              inst->existing_profile);
           pwe_cb->curr_plbck_ssn_info.plbck_ssn_in_progress = TRUE;
           pwe_cb->curr_plbck_ssn_info.is_warmboot_ssn = FALSE;
           m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);
        }
#endif
        /* accessing the global variable */ 
        retval = pss_on_demand_playback(inst, gl_pss_amf_attribs.csi_list, inst->existing_profile);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(retval);
        return retval;

    case ncsPSSvTriggerSave:
        m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_CLI_REQ_SAVE);
        retval = pss_save_current_configuration(inst);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(retval);
        return retval;

    case ncsPSSvTriggerCopy:
        m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_CLI_REQ_COPY);
        /* First check if the first profile exists */
        m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval,
                                  inst->existing_profile, profile_exists);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        if (profile_exists == FALSE)
            return m_MAB_DBG_SINK(NCSCC_RC_NO_INSTANCE);
        if (inst->new_profile[0] == '\0')
            return m_MAB_DBG_SINK(NCSCC_RC_INV_VAL);
        if(strcmp(&inst->new_profile, &inst->existing_profile) == 0)
        {
           /* We can't overwrite the profile over itself */
           return m_MAB_DBG_SINK(NCSCC_RC_INV_VAL);
        }

        /* Then check if the new profile exists */
        m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval,
                                  inst->new_profile, profile_exists);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        if (profile_exists == TRUE)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

        m_NCS_PSSTS_PROFILE_COPY(inst->pssts_api, inst->pssts_hdl, retval,
                                inst->new_profile, inst->existing_profile);
        memset(inst->existing_profile, '\0', sizeof(inst->existing_profile));
        memset(inst->new_profile, '\0', sizeof(inst->new_profile));
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(retval);
        return retval;

    case ncsPSSvTriggerRename:
        m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_CLI_REQ_RENAME);
        /* First check if the first profile exists */
        m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval,
                                  inst->existing_profile, profile_exists);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        if (profile_exists == FALSE)
            return m_MAB_DBG_SINK(NCSCC_RC_NO_INSTANCE);
        if (inst->new_profile[0] == '\0')
            return m_MAB_DBG_SINK(NCSCC_RC_INV_VAL);
        if((strcmp(&inst->new_profile, &inst->existing_profile) == 0) ||
           (strcmp(&inst->current_profile, &inst->existing_profile) == 0))
        {
           /* We can't rename the profile to itself */
           return m_MAB_DBG_SINK(NCSCC_RC_INV_VAL);
        }

        m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval,
                                  inst->new_profile, profile_exists);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        if (profile_exists == TRUE)
            return m_MAB_DBG_SINK(NCSCC_RC_NO_OBJECT);

        m_NCS_PSSTS_PROFILE_MOVE(inst->pssts_api, inst->pssts_hdl, retval,
                                inst->new_profile, inst->existing_profile);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(retval);
        return retval;

    case ncsPSSvTriggerReplace:
        m_LOG_PSS_HEADLINE2(NCSFL_SEV_INFO, PSS_HDLN2_CLI_REQ_REPLACE);
        /* First check if the existing profile exists */
        if (inst->existing_profile[0] == '\0')
            return m_MAB_DBG_SINK(NCSCC_RC_INV_VAL);
        m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval,
                                  inst->existing_profile, profile_exists);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        if (profile_exists == FALSE)
            return m_MAB_DBG_SINK(NCSCC_RC_NO_INSTANCE);
        if(strcmp(&inst->current_profile, &inst->existing_profile) == 0)
        {
           /* We can't replace the profile with itself */
           return m_MAB_DBG_SINK(NCSCC_RC_INV_VAL);
        }

        m_NCS_PSSTS_PROFILE_DELETE(inst->pssts_api, inst->pssts_hdl, retval,
                                inst->current_profile);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(retval);

        m_NCS_PSSTS_PROFILE_COPY(inst->pssts_api, inst->pssts_hdl, retval,
                                inst->current_profile, inst->existing_profile);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(retval);
        return retval;

    case ncsPSSvTriggerReloadLibConf:
        m_LOG_PSS_HEADLINE2(NCSFL_SEV_INFO, PSS_HDLN2_CLI_REQ_RELOADLIBCONF);
        retval = pss_read_lib_conf_info(inst, (char*)&inst->lib_conf_file);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(retval);
        m_PSS_RE_RELOAD_PSSVLIBCONF(gl_pss_amf_attribs.csi_list->pwe_cb);
        return retval;

    case ncsPSSvTriggerReloadSpcnList:
        m_LOG_PSS_HEADLINE2(NCSFL_SEV_INFO, PSS_HDLN2_CLI_REQ_RELOADSPCNLIST);
        retval = pss_read_create_spcn_config_file(inst);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(retval);
        m_PSS_RE_RELOAD_PSSVSPCNLIST(gl_pss_amf_attribs.csi_list->pwe_cb);
        return retval;

    default:
        m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_CLI_REQ_INVALID);
        return m_MAB_DBG_SINK(NCSCC_RC_INV_VAL);
    }

    return retval;
}

#endif /* (NCS_PSR == 1) */
#endif /* (NCS_MAB == 1) */
