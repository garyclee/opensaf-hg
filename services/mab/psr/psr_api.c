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

#include <configmake.h>

/*****************************************************************************
..............................................................................


  DESCRIPTION:

  This file contains all Public APIs for the Persistent Store & Restore (PSS) portion
  of the MIB Acess Broker (MAB) subsystem.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#if (NCS_MAB == 1)
#if (NCS_PSR == 1)
#include "psr.h"


static void pss_free_individual_var_info_pointers(PSS_MIB_TBL_INFO * tbl_info);
extern void pss_stdby_oaa_down_list_dump(PSS_STDBY_OAA_DOWN_BUFFER_NODE *pss_stdby_oaa_down_buffer,FILE *fh);


/*****************************************************************************

                     PSS LAYER MANAGEMENT IMPLEMENTAION

  PROCEDURE NAME:    ncspss_lm

  DESCRIPTION:       Core API for all Persistent Store Restore layer management 
                     operations used by a target system to instantiate and
                     control a PSS instance. Its operations include:

                     CREATE  a PSS instance
                     DESTROY a PSS instance
                     SET     a PSS configuration object
                     GET     a PSS configuration object

  RETURNS:           SUCCESS - All went well
                     FAILURE - internal processing didn't like something.
                               enable m_MAB_DBG_SINK() for details.

*****************************************************************************/
uns32 ncspss_lm( NCSPSS_LM_ARG* arg)
{
   uns32 rc = NCSCC_RC_FAILURE; 

   /* validate the input */ 
   if (arg == NULL)
        return m_MAB_DBG_SINK(rc);
   
   switch(arg->i_op)
   {
      case NCSPSS_LM_OP_CREATE:
         rc = pss_svc_create(&arg->info.create);
         break; 

      case  NCSPSS_LM_OP_DESTROY:
         rc = pss_svc_destroy(arg->i_usr_key);
         break; 

      default:
       break;
   }
   return m_MAB_DBG_SINK(rc);
}

/*****************************************************************************

                     PSS SUBSYSTEM INTERFACE IMPLEMENTATION

  PROCEDURE NAME:    ncspss_ss

  DESCRIPTION:       Core API used by Subcomponents that wish to participate
                     in MAB services. Such subcomponents must make ownership
                     claims reguarding MIB Tables. These claims are

                     TBL_LOAD claim MIB Table ID ownership

  RETURNS:           SUCCESS - All went well
                     FAILURE - internal processing didn't like something.
                               enable m_MAB_DBG_SINK() for details.

*****************************************************************************/
uns32 ncspss_ss(NCSPSS_SS_ARG* arg)
{
  PSS_CB *inst = NULL;
  uns32 rc = NCSCC_RC_FAILURE;

  if ((inst = (PSS_CB*)arg->i_cntxt) == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

  switch(arg->i_op)
  {
  case NCSPSS_SS_OP_TBL_LOAD:
    rc = pss_ss_tbl_reg((PSS_CB*)inst, &arg->info.tbl_info);
    break;

  case NCSPSS_SS_OP_TBL_UNLOAD:
    rc = pss_ss_tbl_gone((PSS_CB*)inst, arg->info.tbl_id);
    break;

  default:
    break;
  }

  return rc;
}



/*****************************************************************************
 *****************************************************************************

     P R I V A T E  Support Functions for PSS APIs are below 

*****************************************************************************
*****************************************************************************/


/*****************************************************************************

  PROCEDURE NAME:    pss_ss_tbl_reg

  DESCRIPTION:       Register MIB Table Information

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/
uns32 pss_ss_tbl_reg(PSS_CB *inst, NCSPSS_TBL_ARG_INFO *tbl_arg_info)
{
    uns32     tbl_id;
    uns32     row_length = 0;
    uns32     i;
    uns32     key_length = 0;
    uns32     bitmap_length = 0;
    uns32     retval = NCSCC_RC_SUCCESS;
    uns32     num_inst_ids = 0;
    uns32     lcl_len = 0;
    NCSMIB_OBJ_INFO *tbl_owned = NULL;
    /* Local-store to collect relative-location of index elements(practically, 
       always less than 512) */
    uns16     index_loc[512], k = 0;

    PSS_MIB_TBL_INFO * tbl_info = NULL;
    PSS_VAR_INFO     * var_info = NULL;

    if((tbl_arg_info == NULL) || ((tbl_owned = tbl_arg_info->mib_tbl) == NULL))
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    m_LOG_PSS_API(PSS_API_TBL_REG);

    tbl_id = tbl_owned->tbl_info.table_id;
    if (tbl_id >= MIB_UD_TBL_ID_END)
    {
        m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_ID_TOO_LARGE);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
    }
    if(tbl_owned->tbl_info.is_persistent == FALSE)
    {
       /* m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, 
             PSS_HDLN2_TBL_ID_NOT_TO_BE_MADE_PERSISTENT); */
       /* This table is not required to be made persistent. So, return normally
          from here. */
       return NCSCC_RC_SUCCESS;
    }

    if (inst->mib_tbl_desc[tbl_id] != NULL)
    {
        m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_ALREADY_REG);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
    }

    tbl_info = m_MMGR_ALLOC_PSS_TBL_INFO;
    if (tbl_info == NULL)
    {
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_TBL_INFO_ALLOC_FAIL,  "pss_ss_tbl_reg()"); 
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    memset(tbl_info, '\0', sizeof(PSS_MIB_TBL_INFO));

    var_info = m_MMGR_ALLOC_PSS_VAR_INFO(tbl_owned->tbl_info.num_objects);
    if (var_info == NULL)
    {
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_VAR_INFO_ALLOC_FAIL,  "pss_ss_tbl_reg()"); 
        m_MMGR_FREE_PSS_TBL_INFO(tbl_info);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    inst->mib_tbl_desc[tbl_id] = tbl_info;
    tbl_info->ptbl_info = m_MMGR_ALLOC_PSS_NCSMIB_TBL_INFO;
    if(tbl_info->ptbl_info == NULL)
    {
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_NCSMIB_TBL_INFO_ALLOC_FAIL,  "pss_ss_tbl_reg()");
        m_MMGR_FREE_PSS_VAR_INFO(var_info);
        m_MMGR_FREE_PSS_TBL_INFO(tbl_info);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    memset(tbl_info->ptbl_info, '\0', sizeof(PSS_NCSMIB_TBL_INFO));
    tbl_info->ptbl_info->num_objects = tbl_owned->tbl_info.num_objects;
    tbl_info->ptbl_info->table_of_scalars = tbl_owned->tbl_info.table_of_scalars;
/*
    if((tbl_owned->tbl_info.table_rank < (uns8)NCSPSS_MIB_TBL_RANK_MIN) ||
       (tbl_owned->tbl_info.table_rank > (uns8)NCSPSS_MIB_TBL_RANK_MAX))
*/
    if(tbl_owned->tbl_info.table_rank < (uns8)NCSPSS_MIB_TBL_RANK_MIN)
    {
       tbl_info->ptbl_info->table_rank = NCSPSS_MIB_TBL_RANK_DEFAULT;
    }
    else
    {
       tbl_info->ptbl_info->table_rank = tbl_owned->tbl_info.table_rank;
    }

    tbl_info->ptbl_info->status_field = tbl_owned->tbl_info.status_field;
    if(m_NCS_STRLEN(tbl_owned->tbl_info.mib_tbl_name) >= (PSS_MIB_TBL_NAME_LEN_MAX-1))
    {
       /* Truncate and store */
       m_NCS_STRNCPY((char*)&tbl_info->ptbl_info->mib_tbl_name, (char*)tbl_owned->tbl_info.mib_tbl_name, (PSS_MIB_TBL_NAME_LEN_MAX-1));
    }
    else
    {
       m_NCS_STRCPY((char*)&tbl_info->ptbl_info->mib_tbl_name, (char*)tbl_owned->tbl_info.mib_tbl_name);
    }

    if(tbl_arg_info->objects_local_to_tbl != NULL)
    {
       /* This table is augmented. */
       tbl_info->ptbl_info->objects_local_to_tbl  = (uns16*)m_MMGR_ALLOC_PSS_OCT(sizeof(uns16) * (tbl_arg_info->objects_local_to_tbl[0] + 1));
       if(tbl_info->ptbl_info->objects_local_to_tbl == NULL)
       {
          m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_NCSMIB_TBL_INFO_ALLOC_FAIL,  "pss_ss_tbl_reg()");
          m_MMGR_FREE_PSS_NCSMIB_TBL_INFO(tbl_info->ptbl_info);
          m_MMGR_FREE_PSS_VAR_INFO(var_info);
          m_MMGR_FREE_PSS_TBL_INFO(tbl_info);
          return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       }
       memset(tbl_info->ptbl_info->objects_local_to_tbl, '\0', (sizeof(uns16) * (tbl_arg_info->objects_local_to_tbl[0] + 1)));
       memcpy(tbl_info->ptbl_info->objects_local_to_tbl, tbl_arg_info->objects_local_to_tbl, (sizeof(uns16) * (tbl_arg_info->objects_local_to_tbl[0] + 1)));
    }
    else
    {
       /* Even though this is a normal/scalar table, generate the "objects_local_to_tbl" so that PSSv always uses the relative-object-ids
          for storing and retrieving MIB data from the persistent store. */
       tbl_info->ptbl_info->objects_local_to_tbl = (uns16*)m_MMGR_ALLOC_PSS_OCT(sizeof(uns16) * (tbl_owned->tbl_info.num_objects + 1)); 
       memset(tbl_info->ptbl_info->objects_local_to_tbl, '\0', (sizeof(uns16) * (tbl_owned->tbl_info.num_objects + 1)));
       tbl_info->ptbl_info->objects_local_to_tbl[0] = tbl_owned->tbl_info.num_objects;
       for(i = 0; i < tbl_owned->tbl_info.num_objects; i++)
       {
          tbl_info->ptbl_info->objects_local_to_tbl[i+1] = i;
       }
    }

    tbl_info->capability = tbl_owned->tbl_info.capability;
    tbl_info->pfields   = var_info;

    /* initialize with the default version */
    tbl_info->i_tbl_version = tbl_owned->tbl_info.table_version;

    /* Calculate the required bitmap length for all the MIB Objects of this table */
    bitmap_length = tbl_info->ptbl_info->num_objects / 8;
    if ((tbl_info->ptbl_info->num_objects % 8) != 0)
       bitmap_length ++;
    tbl_info->bitmap_length  = bitmap_length;
    row_length = bitmap_length;

    for (i = 0, index_loc[0] = 0, k = 1; i < tbl_info->ptbl_info->num_objects; i++)
    {
        if((tbl_owned->var_info[i].mib_obj_name == NULL) ||
           (tbl_owned->var_info[i].param_id == 0))
           continue;

        /* Store data for each field of the table */
        var_info[i].var_info  = tbl_owned->var_info[i];

        var_info[i].var_info.mib_obj_name = NULL;
        m_NCS_STRCPY((char*)&var_info[i].var_name, (char*)tbl_owned->var_info[i].mib_obj_name);

        /* Need allocate memory for mib_obj_name from NCSMIB_VAR_INFO. */ 
        var_info[i].offset     = row_length;
        var_info[i].var_length = FALSE;
        /* Calculate the row length only for settable fields */
        
        if (tbl_owned->var_info[i].obj_type == NCSMIB_OCT_OBJ_TYPE)
        {
           if((tbl_owned->var_info[i].obj_spec.stream_spec.min_len) !=
              (tbl_owned->var_info[i].obj_spec.stream_spec.max_len))
           {
              var_info[i].var_length = TRUE;
              row_length += sizeof(uns16);
              if (tbl_owned->var_info[i].is_index_id == TRUE)
              {
                 key_length += sizeof(uns16);
              }
           }

           /* Soft-limiting the unbounded OCTET objects to NCS_PSS_UNBOUNDED_OCTET_SOFT_LIMITED_SIZE bytes. */
           if(tbl_owned->var_info[i].obj_spec.stream_spec.max_len > NCS_PSS_UNBOUNDED_OCTET_SOFT_LIMITED_SIZE)
           {
              row_length += NCS_PSS_UNBOUNDED_OCTET_SOFT_LIMITED_SIZE;
              if (tbl_owned->var_info[i].is_index_id == TRUE)
                 key_length += NCS_PSS_UNBOUNDED_OCTET_SOFT_LIMITED_SIZE;
              var_info[i].var_info.obj_spec.stream_spec.max_len = NCS_PSS_UNBOUNDED_OCTET_SOFT_LIMITED_SIZE;
           }
           else
           {
              row_length += tbl_owned->var_info[i].obj_spec.stream_spec.max_len;
              if (tbl_owned->var_info[i].is_index_id == TRUE)
                 key_length += tbl_owned->var_info[i].obj_spec.stream_spec.max_len;
           }
        }
        else if(tbl_owned->var_info[i].obj_type == NCSMIB_OCT_DISCRETE_OBJ_TYPE)
        {

           var_info[i].var_length = TRUE;
           row_length += sizeof(uns16); /* To identify exact length of the range'd object */
           key_length += sizeof(uns16);
           lcl_len = 
            ncsmiblib_get_max_len_of_discrete_octet_string(&tbl_owned->var_info[i]);
           row_length += lcl_len;
           if (tbl_owned->var_info[i].is_index_id == TRUE)
           {
              key_length += lcl_len;
           }
           if((var_info[i].var_info.obj_spec.disc_stream_spec.values = 
               m_MMGR_ALLOC_PSS_NCSMIB_OCT_OBJ(var_info[i].var_info.obj_spec.disc_stream_spec.num_values))
              == NULL)
           {
              pss_free_individual_var_info_pointers(tbl_info);
              if(tbl_info->ptbl_info->objects_local_to_tbl != NULL)
                 m_MMGR_FREE_PSS_OCT(tbl_info->ptbl_info->objects_local_to_tbl);
              m_MMGR_FREE_PSS_VAR_INFO(var_info);
              m_MMGR_FREE_PSS_TBL_INFO(tbl_info);
              return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
           }
           memset(var_info[i].var_info.obj_spec.disc_stream_spec.values, '\0', 
              (sizeof(NCSMIB_OCT_OBJ) * var_info[i].var_info.obj_spec.disc_stream_spec.num_values));
           memcpy(var_info[i].var_info.obj_spec.disc_stream_spec.values, 
              tbl_owned->var_info[i].obj_spec.disc_stream_spec.values,
              (sizeof(NCSMIB_OCT_OBJ) * var_info[i].var_info.obj_spec.disc_stream_spec.num_values));
        }
        else
        {
           row_length += tbl_owned->var_info[i].len;
           if (tbl_owned->var_info[i].is_index_id == TRUE)
           {
              key_length += tbl_owned->var_info[i].len;
           }
           if(tbl_owned->var_info[i].obj_type == NCSMIB_INT_DISCRETE_OBJ_TYPE)
           {
              if((var_info[i].var_info.obj_spec.value_spec.values = 
                  m_MMGR_ALLOC_PSS_NCSMIB_INT_OBJ_RANGE(var_info[i].var_info.obj_spec.value_spec.num_values))
                 == NULL)
              {
                 pss_free_individual_var_info_pointers(tbl_info);
                 if(tbl_info->ptbl_info->objects_local_to_tbl != NULL)
                    m_MMGR_FREE_PSS_OCT(tbl_info->ptbl_info->objects_local_to_tbl);
                 m_MMGR_FREE_PSS_VAR_INFO(var_info);
                 m_MMGR_FREE_PSS_TBL_INFO(tbl_info);
                 return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
              }
              memset(var_info[i].var_info.obj_spec.value_spec.values, '\0', 
                 (sizeof(NCSMIB_INT_OBJ_RANGE) * var_info[i].var_info.obj_spec.value_spec.num_values));
              memcpy(var_info[i].var_info.obj_spec.value_spec.values, 
                 tbl_owned->var_info[i].obj_spec.value_spec.values,
                 (sizeof(NCSMIB_INT_OBJ_RANGE) * var_info[i].var_info.obj_spec.value_spec.num_values));
           }
        }

        if (tbl_owned->var_info[i].is_index_id == TRUE)
        {
            ++index_loc[0];
            /* Here, truncating NCS_PARAM_ID to uns16 is ok, since the number of param-ids per MIB table
               won't cross (2^16-1). */
            index_loc[k++] = i;  /* Store the relative-index-location here */

            if ((tbl_owned->var_info[i].fmat_id == NCSMIB_FMAT_INT) ||
                (tbl_owned->var_info[i].fmat_id == NCSMIB_FMAT_BOOL))
                num_inst_ids += 1;
            else if(tbl_owned->var_info[i].obj_type == NCSMIB_OCT_OBJ_TYPE)
            {
               if((tbl_owned->var_info[i].obj_spec.stream_spec.min_len) !=
                  (tbl_owned->var_info[i].obj_spec.stream_spec.max_len))
               { 
                  num_inst_ids++;
               } 
               num_inst_ids += tbl_owned->var_info[i].obj_spec.stream_spec.max_len;
            }
            else if(tbl_owned->var_info[i].obj_type == NCSMIB_OCT_DISCRETE_OBJ_TYPE)
            {
               num_inst_ids ++; 
               num_inst_ids += lcl_len; 
            }
            else
            {
               num_inst_ids += tbl_owned->var_info[i].len;
            }
        }
    }

    /* Now collect the index-locations into tbl_info->ptbl_info->idx_data. This is required by PSS to 
       perform MIB validations on received SET events from OAA. */
    if(index_loc[0] != 0)
    {
       tbl_info->ptbl_info->idx_data = (uns16*)m_MMGR_ALLOC_PSS_OCT(sizeof(uns16) * (index_loc[0] + 1));
       if(tbl_info->ptbl_info->idx_data == NULL)
       {
          m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_NCSMIB_TBL_INFO_ALLOC_FAIL,  "pss_ss_tbl_reg() - idx_data ");
          inst->mib_tbl_desc[tbl_id] = NULL;
          if(tbl_info->ptbl_info->objects_local_to_tbl != NULL)
             m_MMGR_FREE_PSS_OCT(tbl_info->ptbl_info->objects_local_to_tbl);
          pss_free_individual_var_info_pointers(tbl_info);
          m_MMGR_FREE_PSS_VAR_INFO(var_info);
          m_MMGR_FREE_PSS_TBL_INFO(tbl_info);
          return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       }
       memset(tbl_info->ptbl_info->idx_data, '\0', (sizeof(uns16) * (index_loc[0] + 1)));

       memcpy(tbl_info->ptbl_info->idx_data, index_loc, (sizeof(uns16) * (index_loc[0] + 1)));
    }

    tbl_info->max_row_length = row_length;
    tbl_info->max_key_length = key_length;
    m_LOG_PSS_TBL_REG_INFO(NCSFL_SEV_INFO, PSS_TBL_DESC_INFO, tbl_info->ptbl_info->mib_tbl_name, \
       tbl_id, tbl_info->ptbl_info->table_rank, tbl_info->ptbl_info->num_objects, \
       num_inst_ids, tbl_info->max_key_length, tbl_info->max_row_length, \
       tbl_info->capability, bitmap_length);
    tbl_info->num_inst_ids   = num_inst_ids;
    for (i = 0; i < tbl_info->ptbl_info->num_objects; i++)
    {
       NCSMIB_VAR_INFO *lvar = &var_info[i].var_info;
       m_LOG_PSS_VAR_INFO(NCSFL_SEV_INFO, PSS_VAR_INFO_DETAILS, tbl_id, \
        var_info[i].offset, lvar->param_id, lvar->offset, lvar->len, \
        lvar->is_index_id, lvar->access, lvar->status, lvar->set_when_down, \
        lvar->fmat_id, lvar->obj_type, lvar->obj_spec.range_spec.min, \
        lvar->obj_spec.range_spec.max, lvar->obj_spec.stream_spec.min_len, \
        lvar->obj_spec.stream_spec.max_len, lvar->is_readonly_persistent);
    }

    /* Rank of the MIB in the linked-list. Maybe this can be deprecated later on, if found to be redundant. */
    retval = pss_mib_tbl_rank_add(inst, tbl_id, tbl_info->ptbl_info->table_rank);
    if (retval != NCSCC_RC_SUCCESS)
    {
        inst->mib_tbl_desc[tbl_id] = NULL;
        if(tbl_info->ptbl_info->idx_data != NULL)
           m_MMGR_FREE_PSS_OCT(tbl_info->ptbl_info->idx_data);
        if(tbl_info->ptbl_info->objects_local_to_tbl != NULL)
           m_MMGR_FREE_PSS_OCT(tbl_info->ptbl_info->objects_local_to_tbl);
        pss_free_individual_var_info_pointers(tbl_info);
        m_MMGR_FREE_PSS_VAR_INFO(var_info);
        m_MMGR_FREE_PSS_TBL_INFO(tbl_info);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_free_individual_var_info_pointers

  DESCRIPTION:       Free only the pointers hanging off the var_info fields.

  RETURNS:           void

*****************************************************************************/
static void pss_free_individual_var_info_pointers(PSS_MIB_TBL_INFO * tbl_info)
{
   uns32            i = 0;

   if(tbl_info == NULL)
      return;

   for (i = 0; i < tbl_info->ptbl_info->num_objects; i++)
   {
      if(tbl_info->pfields[i].var_info.obj_type == NCSMIB_OCT_DISCRETE_OBJ_TYPE)
      {
         if(tbl_info->pfields[i].var_info.obj_spec.disc_stream_spec.values != NULL)
         {
            m_MMGR_FREE_PSS_NCSMIB_OCT_OBJ(tbl_info->pfields[i].var_info.obj_spec.disc_stream_spec.values);
            tbl_info->pfields[i].var_info.obj_spec.disc_stream_spec.values = NULL;
         }
      }
      else if(tbl_info->pfields[i].var_info.obj_type == NCSMIB_INT_DISCRETE_OBJ_TYPE)
      {
         if(tbl_info->pfields[i].var_info.obj_spec.value_spec.values != NULL)
         {
            m_MMGR_FREE_PSS_NCSMIB_INT_OBJ_RANGE(tbl_info->pfields[i].var_info.obj_spec.value_spec.values);
            tbl_info->pfields[i].var_info.obj_spec.value_spec.values = NULL;
         }
      }
   }
   return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_ss_tbl_gone

  DESCRIPTION:       UnRegister MIB Table Information

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/
uns32 pss_ss_tbl_gone(PSS_CB *inst, uns32 tbl_id)
{
    uns8      rank;
    PSS_MIB_TBL_RANK * tbl_rank, *tmp;
    PSS_MIB_TBL_INFO * tbl_info;
    PSS_VAR_INFO     * var_info;

    m_PSS_LK_INIT;

    m_LOG_PSS_API(PSS_API_TBL_UNREG);

    if (tbl_id >= MIB_UD_TBL_ID_END)
    {
        m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_ID_TOO_LARGE);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
    }

    tbl_info = inst->mib_tbl_desc[tbl_id];
    if (tbl_info == NULL)
    {
        m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_DESC_NULL);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
    }

    var_info = tbl_info->pfields;
    rank = tbl_info->ptbl_info->table_rank;

    if(tbl_info->ptbl_info->objects_local_to_tbl != NULL)
       m_MMGR_FREE_PSS_OCT(tbl_info->ptbl_info->objects_local_to_tbl);
    pss_free_individual_var_info_pointers(tbl_info);

    if (var_info != NULL)
        m_MMGR_FREE_PSS_VAR_INFO(var_info);

    if(tbl_info->ptbl_info->idx_data != NULL)
        m_MMGR_FREE_PSS_OCT(tbl_info->ptbl_info->idx_data);

    m_MMGR_FREE_PSS_NCSMIB_TBL_INFO(tbl_info->ptbl_info);
    m_MMGR_FREE_PSS_TBL_INFO(tbl_info);
    inst->mib_tbl_desc[tbl_id] = NULL;

    /* Delete the entry of this MIB in the rank-linked-list */
    tbl_rank = inst->mib_tbl_rank[rank];
    if (tbl_rank != NULL)
    {
        if (tbl_rank->tbl_id == tbl_id)
        {
            inst->mib_tbl_rank[rank] = tbl_rank->next;
            m_MMGR_FREE_PSS_TBL_RNK(tbl_rank);
        }
        else
        {
            tmp = tbl_rank->next;
            while (tmp != NULL)
            {
                if (tmp->tbl_id == tbl_id)
                {
                    tbl_rank->next = tmp->next;
                    m_MMGR_FREE_PSS_TBL_RNK(tmp);
                    break;
                }

                tbl_rank = tmp;
                tmp = tmp->next;
            }
        }
    }

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_flush_all_mib_tbl_info

  DESCRIPTION:       UnRegister all MIB Table Information

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/
uns32 pss_flush_all_mib_tbl_info(PSS_CB *inst)
{
    PSS_MIB_TBL_RANK * tbl_rank, *tmp;
    PSS_MIB_TBL_INFO * tbl_info;
    PSS_VAR_INFO     * var_info;
    uns32 i = 0;

    for (i = 0; i < MIB_UD_TBL_ID_END; i++)
    {
        tbl_info = inst->mib_tbl_desc[i];
        if (tbl_info == NULL)
        {
           continue; 
        }

        if(tbl_info->ptbl_info->objects_local_to_tbl != NULL)
           m_MMGR_FREE_PSS_OCT(tbl_info->ptbl_info->objects_local_to_tbl);

        var_info = tbl_info->pfields;
        pss_free_individual_var_info_pointers(tbl_info);
        if (var_info != NULL)
        {
           m_MMGR_FREE_PSS_VAR_INFO(var_info);
        }

        if(tbl_info->ptbl_info->idx_data != NULL)
           m_MMGR_FREE_PSS_OCT(tbl_info->ptbl_info->idx_data);

        m_MMGR_FREE_PSS_NCSMIB_TBL_INFO(tbl_info->ptbl_info);
        m_MMGR_FREE_PSS_TBL_INFO(tbl_info);

        inst->mib_tbl_desc[i] = NULL;
    }
    for (i = 0; i < (NCSPSS_MIB_TBL_RANK_MAX + 1); i++)
    {
        tbl_rank = inst->mib_tbl_rank[i];
        while(tbl_rank != NULL)
        {
           tmp = tbl_rank->next;
           m_MMGR_FREE_PSS_TBL_RNK(tbl_rank);
           tbl_rank = tmp;
        }
        inst->mib_tbl_rank[i] = NULL;
    }

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_svc_create

  DESCRIPTION:       Create an instance of PSS, set configuration profile to
                     default, install this PSS with MDS and subscribe to MAB
                     events.

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_svc_create(NCSPSS_CREATE* create)
{
    /* Create a new structure and initialize all its fields */
    PSS_CB *           inst;
    uns32              retval;
    NCS_PSSTS_ARG      pssts_arg;
    NCS_SPIR_REQ_INFO  spir_info;

    m_PSS_LK_INIT;

    /* make sure there is a callback to report errors to?  */ 
    if (create->i_lm_cbfnc == NULL)
    {
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    /* check for the availability of the target services API */ 
    if (create->i_pssts_api == NULL)
    {
        m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_PSSTS_API_NULL);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    
    /* create the transaction manager */ 
    if (ncsmib_tm_create() != NCSCC_RC_SUCCESS)
    {
        m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_NCSMIBTM_INIT_ERR);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    /* function to be used to play the data */ 
    if (create->i_mib_req_func == NULL)
    {
        /* ##### Add a log here */
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    /* get the PSS configuration */ 
    pssts_arg.i_op = NCS_PSSTS_OP_GET_PSS_CONFIG;
    pssts_arg.ncs_hdl = create->i_pssts_hdl;
    retval = (create->i_pssts_api)(&pssts_arg);
    if (retval != NCSCC_RC_SUCCESS)
    {
        m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_PSSTS_CONFIG_ERR);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    /* allocate the CB */ 
    inst = m_MMGR_ALLOC_PSS_CB;
    if (inst == NULL)
    {
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PSS_CB_ALLOC_FAIL,  "pss_svc_create()");
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    /* reset */  
    memset(inst,0,sizeof(PSS_CB));

    /* intialize lock */ 
    m_PSS_LK_CREATE(&inst->lock);
    m_LOG_PSS_LOCK(PSS_LK_CREATED,&inst->lock);

    m_PSS_LK(&inst->lock);
    m_LOG_PSS_LOCK(PSS_LK_LOCKED,&inst->lock);

    /* fill in the version */
    inst->version = (uns16)m_PSS_SERVICE_VERSION;


    /* copy the default current profile name */ 
    m_NCS_STRCPY((char *)inst->current_profile, pssts_arg.info.pss_config.current_profile_name);

    /* get the OAA handle with the help of SPRR */ 
    memset(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO));
    spir_info.type = NCS_SPIR_REQ_LOOKUP_CREATE_INST;
    spir_info.i_sp_abstract_name = m_OAA_SP_ABST_NAME;
    m_MDS_FIXED_VDEST_TO_INST_NAME(PSR_VDEST_ID, &spir_info.i_instance_name);
    spir_info.i_environment_id = m_PSS_DEFAULT_ENV_ID;
    retval = ncs_spir_api(&spir_info);
    if (retval != NCSCC_RC_SUCCESS)
    {
       /* log the failure */
       m_LOG_PSS_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
                         PSS_ERR_SPIR_OAA_LOOKUP_CREATE_FAILED,
                         retval, m_PSS_DEFAULT_ENV_ID);
       m_PSS_UNLK(&inst->lock);
       m_LOG_PSS_LOCK(PSS_LK_UNLOCKED,&inst->lock);
       m_PSS_LK_DLT(&inst->lock);
       m_LOG_PSS_LOCK(PSS_LK_DELETED,&inst->lock);
       m_MMGR_FREE_PSS_CB(inst);
       return m_MAB_DBG_SINK(retval);
    }
    inst->oac_key   = (uns32)spir_info.info.lookup_create_inst.o_handle;
   
    inst->mds_hdl   = create->i_mds_hdl;
    inst->lm_cbfnc  = create->i_lm_cbfnc;
    inst->pssts_api = create->i_pssts_api;
    inst->hmpool_id = create->i_hmpool_id;
    inst->pssts_hdl = create->i_pssts_hdl;
    inst->save_type = create->i_save_type;
    inst->mib_req_func = create->i_mib_req_func;
    inst->mbx       = create->i_mbx; 

    pss_mib_tbl_rank_init(inst);
    pss_mib_tbl_desc_init(inst);

    if(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE)
    {
        m_NCS_PSSTS_SET_PSS_CONFIG(inst->pssts_api, inst->pssts_hdl, retval, "current");
        if(retval != NCSCC_RC_SUCCESS)
        {
            m_PSS_UNLK(&inst->lock);
            m_LOG_PSS_LOCK(PSS_LK_UNLOCKED,&inst->lock);
            m_PSS_LK_DLT(&inst->lock);
            m_LOG_PSS_LOCK(PSS_LK_DELETED,&inst->lock);
            m_MMGR_FREE_PSS_CB(inst);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }
    }

    m_NCS_OS_LOG_SPRINTF((char*)&inst->lib_conf_file, m_PSS_LIB_CONF_FILE_NAME);
    m_NCS_OS_LOG_SPRINTF((char*)&inst->spcn_list_file, m_PSS_SPCN_LIST_FILE_NAME);

    /* Read pssv_lib_conf file, to load the application MIB definition libraries. */
    if(pss_read_lib_conf_info(inst, (char*)&inst->lib_conf_file) != NCSCC_RC_SUCCESS)
    {
       m_PSS_UNLK(&inst->lock);
       m_LOG_PSS_LOCK(PSS_LK_UNLOCKED,&inst->lock);
       m_PSS_LK_DLT(&inst->lock);
       m_LOG_PSS_LOCK(PSS_LK_DELETED,&inst->lock);
       m_MMGR_FREE_PSS_CB(inst);
       return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    /* Read/create the "spcn_list" configuration file. */
    if(pss_read_create_spcn_config_file(inst) != NCSCC_RC_SUCCESS)
    {
       pss_destroy_spcn_list(inst->spcn_list);
       m_PSS_UNLK(&inst->lock);
       m_LOG_PSS_LOCK(PSS_LK_UNLOCKED,&inst->lock);
       m_PSS_LK_DLT(&inst->lock);
       m_LOG_PSS_LOCK(PSS_LK_DELETED,&inst->lock);
       m_MMGR_FREE_PSS_CB(inst);
       return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    inst->profile_tbl_row_hdl = 0;
    inst->trigger_scl_row_hdl = 0;
    inst->mem_in_store = 0; 
    inst->bam_req_cnt = 0; 

    /* update the handler */ 
    inst->hm_hdl = ncshm_create_hdl(inst->hmpool_id, NCS_SERVICE_ID_PSS,
                                   (NCSCONTEXT)inst);
    create->o_pss_hdl = inst->hm_hdl;

#if (NCS_PSS_RED == 1)
    m_NCS_EDU_HDL_INIT(&inst->edu_hdl);

    if(pss_mbcsv_register(inst) != NCSCC_RC_SUCCESS)
    {
        /*m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_OAC_REG_FAILURE);*/
        m_NCS_EDU_HDL_FLUSH(&inst->edu_hdl);
        ncshm_destroy_hdl(NCS_SERVICE_ID_PSS, inst->hm_hdl);
        pss_destroy_spcn_list(inst->spcn_list);
        pss_amf_csi_remove_all();
        m_PSS_UNLK(&inst->lock);
        m_LOG_PSS_LOCK(PSS_LK_UNLOCKED,&inst->lock);
        m_PSS_LK_DLT(&inst->lock);
        m_LOG_PSS_LOCK(PSS_LK_DELETED,&inst->lock);
        m_MMGR_FREE_PSS_CB(inst);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
#endif

    gl_pss_amf_attribs.pss_cb = inst;

    m_PSS_UNLK(&inst->lock);
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_pwe_cb_init

  DESCRIPTION:       Create a instance of PWE in PSS.

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_pwe_cb_init(uns32 pss_cb_handle, PSS_PWE_CB *pwe_cb, PW_ENV_ID envid)
{
   uns32             status; 
   PSS_CB            *inst; 
   NCS_SPIR_REQ_INFO spir_info;

   if((inst = (PSS_CB*)ncshm_take_hdl(NCS_SERVICE_ID_PSS, pss_cb_handle)) == NULL)
   { 
       /* add logging */ 
       return NCSCC_RC_FAILURE; 
   }

   memset(pwe_cb, 0, sizeof(PSS_PWE_CB));

   /* update the environement id */ 
   pwe_cb->pwe_id = envid;
   pwe_cb->p_pss_cb = inst;

   /* call into SPRR interface of MDS to get the PWE handle */ 
   memset(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO));
   spir_info.type = NCS_SPIR_REQ_LOOKUP_CREATE_INST;
   spir_info.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
   m_MDS_FIXED_VDEST_TO_INST_NAME(PSR_VDEST_ID, &spir_info.i_instance_name);
   spir_info.i_environment_id =  envid;
   status = ncs_spir_api(&spir_info);
   if (status != NCSCC_RC_SUCCESS)
   {
      /* log the failure */
      m_LOG_PSS_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
                        PSS_ERR_SPIR_MDS_LOOKUP_CREATE_FAILED,
                        status, envid);
      ncshm_give_hdl(pss_cb_handle);
      return status;
   }

   /* store the PWE handle */ 
   pwe_cb->mds_pwe_handle = (MDS_HDL)spir_info.info.lookup_create_inst.o_handle;

   /* get the MAA handle */ 
   memset(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO));
   spir_info.type = NCS_SPIR_REQ_LOOKUP_CREATE_INST;
   spir_info.i_sp_abstract_name = m_MAA_SP_ABST_NAME;
   spir_info.i_instance_name.length = 0;
   spir_info.i_environment_id =  envid;
   status = ncs_spir_api(&spir_info);
   if (status != NCSCC_RC_SUCCESS)
   {
      /* log the failure */
      m_LOG_PSS_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
                        PSS_ERR_SPIR_MAA_LOOKUP_CREATE_FAILED,
                        status, envid);

      /* remove the MDS reference */ 
      ncshm_give_hdl(pss_cb_handle);
      return status;
   }

   /* store the MAA handle */ 
   pwe_cb->mac_key = (uns32)spir_info.info.lookup_create_inst.o_handle;
   pwe_cb->mbx = inst->mbx; 

   /* initialize the miscellaneous stuff */ 
   pwe_cb->is_mas_alive = FALSE;
   pwe_cb->hm_hdl = ncshm_create_hdl(inst->hmpool_id, NCS_SERVICE_ID_PSS,
                                   (NCSCONTEXT)pwe_cb);
   
   /* Create the client_table, which hosts the PCN entries of applications */
   memset(&pwe_cb->params_client, 0, sizeof(NCS_PATRICIA_PARAMS));
   pwe_cb->params_client.key_size = sizeof(PSS_CLIENT_KEY);
   if (ncs_patricia_tree_init(&pwe_cb->client_table, &pwe_cb->params_client) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_PAT_TREE_FAILURE);
      ncshm_destroy_hdl(NCS_SERVICE_ID_PSS, pwe_cb->hm_hdl);
      ncshm_give_hdl(pss_cb_handle);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   m_NCS_MEM_DBG_LOC(pwe_cb->client_table.root_node.key_info);

   /* Create the oaa_tree, which hosts the MDS_DESTs from where the applications 
      register their MIBs with PSS. */
   memset(&pwe_cb->params_oaa, 0, sizeof(NCS_PATRICIA_PARAMS));
   pwe_cb->params_oaa.key_size = sizeof(PSS_OAA_KEY);
   if (ncs_patricia_tree_init(&pwe_cb->oaa_tree, &pwe_cb->params_oaa) != NCSCC_RC_SUCCESS)
   {
      pss_index_tree_destroy(&pwe_cb->client_table);
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_PAT_TREE_FAILURE);
      ncshm_destroy_hdl(NCS_SERVICE_ID_PSS, pwe_cb->hm_hdl);
      ncshm_give_hdl(pss_cb_handle);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   m_NCS_MEM_DBG_LOC(pwe_cb->oaa_tree.root_node.key_info);
   
   ncshm_give_hdl(pss_cb_handle); 

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_read_lib_conf_info

  DESCRIPTION:       Read the config file inst->lib_conf_file and
                     loads the application MIB information into PSS.

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_read_lib_conf_info(PSS_CB *inst, char *conf_file)
{
    FILE *fh = NULL;
    char libname[256], fullname[256], appname[64], funcname[256];
    uns32 read = 0;
    uns32 status = NCSCC_RC_FAILURE;
    uns32 (*app_routine)(NCSCONTEXT) = NULL; 
    NCS_OS_DLIB_HDL     *lib_hdl = NULL;
    int8  *dl_error = NULL;

    /* Removes all earlier load MIB description info, if any */
    pss_flush_all_mib_tbl_info(inst);

    /* The "pssv_lib_conf" file is mandatory for PSS to start up */
    fh = sysf_fopen(conf_file, "r");
    if(fh == NULL)
    {
        m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_LIB_CONF_FILE_OPEN_FAIL);
        return NCSCC_RC_FAILURE;
    }

    memset(&libname, '\0', sizeof(libname));
    memset(&appname, '\0', sizeof(appname));
    while(((read = fscanf(fh, "%s %s", (char*)&libname, (char*)&appname)) == 2) &&
          (read != EOF))
    {
        m_LOG_PSS_LIB_INFO(NCSFL_SEV_INFO, PSS_LIB_INFO_CONF_FILE_ENTRY, &libname, &appname);
        /* Load the library, and invoke the lib-register-function */
        memset(&fullname, '\0', sizeof(fullname));
        sysf_sprintf((char*)&fullname, "%s", (char*)&libname);
        lib_hdl = m_NCS_OS_DLIB_LOAD(fullname, m_NCS_OS_DLIB_ATTR); 
        if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
        {
            /* log the error returned from dlopen() */
            m_LOG_PSS_LIB_OP(NCSFL_SEV_ERROR, PSS_LIB_OP_CONF_FILE_OPEN_FAIL, dl_error);
            continue;
        }

        /* Get the registration symbol "__<appname>_pssv_reg" from the appname */
        memset(&funcname, '\0', sizeof(funcname));
        sysf_sprintf((char*)&funcname, "__%s_pssv_reg", (char*)&appname);
        app_routine = m_NCS_OS_DLIB_SYMBOL(lib_hdl, funcname); 
        if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
        {
            /* log the error returned from dlopen() */
            m_LOG_PSS_LIB_OP(NCSFL_SEV_ERROR, PSS_LIB_OP_SYM_LKUP_ERROR, dl_error);
            m_NCS_OS_DLIB_CLOSE(lib_hdl);
            continue;
        }

        /* Invoke the registration symbol */
        status = (*app_routine)((NCSCONTEXT)inst); 
        if (status != NCSCC_RC_SUCCESS)
        {
            m_LOG_PSS_LIB_OP(NCSFL_SEV_ERROR, PSS_LIB_OP_SYM_RET_ERROR, dl_error);
            m_NCS_OS_DLIB_CLOSE(lib_hdl);
            continue;
        }
        m_LOG_PSS_LIB_INFO(NCSFL_SEV_INFO, PSS_LIB_INFO_CONF_FILE_ENTRY_LOADED, &libname, &funcname);
        m_NCS_OS_DLIB_CLOSE(lib_hdl);

        memset(&libname, '\0', sizeof(libname));
        memset(&appname, '\0', sizeof(appname));
    }   /* while loop */
    sysf_fclose(fh);

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_read_create_spcn_config_file

  DESCRIPTION:       Read the config file inst->spcn_list_file and
                     populate the list "pss_cb->spcn_list".

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_read_create_spcn_config_file(PSS_CB *inst)
{
    PSS_SPCN_LIST *list_head = NULL, *list = NULL;

    FILE *fh = NULL;
    char pcn[NCSMIB_PCN_LENGTH_MAX], source[64];
    uns32 read = 0, boolean = 0;


    pss_destroy_spcn_list(inst->spcn_list);
    inst->spcn_list = NULL;

 
    /* Open the "pssv_spcn_list" file. PSS is the only owner to this file. */
    fh = sysf_fopen((char*)&inst->spcn_list_file, "r");
    if(fh == NULL)
    {
       /* If file does not exist, create it. */
       fh = sysf_fopen((char*)&inst->spcn_list_file, "a+");
       if(fh == NULL)
       {
          m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_SPCN_LIST_FILE_OPEN_FAIL);
          return NCSCC_RC_FAILURE;
       }

       /* After creating the file, continue to the next job. */
       sysf_fclose(fh);
       return NCSCC_RC_SUCCESS;
    }

    memset(&pcn, '\0', sizeof(pcn));
    while(((read = fscanf(fh, "%s %s", (char*)&pcn, (char *)&source)) == 2) &&
          (read != EOF))
    {
        uns32 str_len = 0, src_str_len = 0;

        str_len = m_NCS_STRLEN(&pcn);
        src_str_len = m_NCS_STRLEN(&source);
        if(src_str_len == 0) /* Ignore this line */
           continue;
        if(m_NCS_STRCMP(&source, m_PSS_SPCN_SOURCE_BAM) == 0)
           boolean = TRUE;
        else
           boolean = FALSE;

        if(list == NULL)
        {
            if((list_head = list = m_MMGR_ALLOC_MAB_PSS_SPCN_LIST) == NULL)
            {
                m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PSS_SPCN_LIST,
                    "pss_read_create_spcn_config_file()");
                goto go_fail;
            }
        }
        else
        {
            if((list->next = m_MMGR_ALLOC_MAB_PSS_SPCN_LIST) == NULL)
            {
                m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PSS_SPCN_LIST,
                    "pss_read_create_spcn_config_file()");
                goto go_fail;
            }
            list = list->next;
        }
        memset(list, '\0', sizeof(PSS_SPCN_LIST));
        if((list->pcn = m_MMGR_ALLOC_MAB_PCN_STRING(str_len + 1)) == NULL)
        {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PCN_STRING_ALLOC_FAIL,
                "pss_read_create_spcn_config_file()");
            m_MMGR_FREE_MAB_PSS_SPCN_LIST(list);
            goto go_fail;
        }
        memset(list->pcn, '\0', str_len + 1);
        m_NCS_STRCPY(list->pcn, &pcn);
        list->plbck_frm_bam = (NCS_BOOL)boolean;

        m_LOG_PSS_INFO(NCSFL_SEV_INFO, PSS_INFO_SPCN, &pcn, boolean);
        memset(&pcn, '\0', sizeof(pcn));
    }   /* while loop */
    sysf_fclose(fh);


    inst->spcn_list = list_head;
    return NCSCC_RC_SUCCESS;


go_fail:
    sysf_fclose(fh);
    return NCSCC_RC_FAILURE;

}

/*****************************************************************************

  PROCEDURE NAME:    pss_destroy_spcn_list

  DESCRIPTION:       Destroy the list "pss_cb->spcn_list".

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
void pss_destroy_spcn_list(PSS_SPCN_LIST *list)
{
    PSS_SPCN_LIST *tmp = NULL;

    while(list != NULL)
    {
        tmp = list->next;

        if(list->pcn)
            m_MMGR_FREE_MAB_PCN_STRING(list->pcn);
        m_MMGR_FREE_MAB_PSS_SPCN_LIST(list);

        list = tmp;
    }
    return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_svc_destroy

  DESCRIPTION:       Destroy an instance of PSS. Withdraw from MDS and free
                     this PSS_CB and tend to other resource recovery issues.

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_svc_destroy(uns32 usr_key)
{
    PSS_CB        *inst;
    uns32         retval = NCSCC_RC_SUCCESS;
    SaAisErrorT   saf_status = SA_AIS_OK; 

    m_PSS_LK_INIT;

    if ((inst = (PSS_CB*)m_PSS_VALIDATE_HDL(usr_key)) == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    m_LOG_PSS_API(PSS_API_SVC_DESTROY);

    m_PSS_LK(&inst->lock);

    /* Unregister PSS-MIB with OAA */
    (void)pss_unregister_with_oac(inst);

    /* clean up all the PWEs and VDEST */ 
    retval = pss_amf_prepare_will(); 

    if (gl_pss_amf_attribs.amf_attribs.amfHandle != 0)
    {
        /* finalize the interface with AMF */ 
        saf_status = ncs_app_amf_finalize(&gl_pss_amf_attribs.amf_attribs); 
        if (saf_status != SA_AIS_OK)
        {
            /* log that AMF interface finalize failed */ 
            m_LOG_PSS_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                                 PSS_ERR_APP_AMF_FINALIZE_FAILED, saf_status); 
        }
    }

    pss_clean_inst_cb(inst);

    m_PSS_UNLK(&inst->lock);
    m_PSS_LK_DLT(&inst->lock);

    ncshm_give_hdl(usr_key);
    ncshm_destroy_hdl(NCS_SERVICE_ID_PSS, inst->hm_hdl);
    m_MMGR_FREE_PSS_CB(inst);

    return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_clean_inst_cb

  DESCRIPTION:       Delete/Free global data in PSS_CB.

  RETURNS:           void

*****************************************************************************/
void pss_clean_inst_cb(PSS_CB *inst)
{
    /* Destroy the rank and description tables */
    pss_mib_tbl_rank_dest(inst);
    pss_mib_tbl_desc_dest(inst);

    pss_destroy_spcn_list(inst->spcn_list);
    inst->spcn_list = NULL;

    ncsmib_tm_destroy();

#if (NCS_PSS_RED == 1)
    m_NCS_EDU_HDL_FLUSH(&inst->edu_hdl);
#endif

    return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_mib_tbl_rank_init

  DESCRIPTION:       Initialize the mib-rank field in the PSS_CB.

  RETURNS:           void 

*****************************************************************************/
/* Initialize the Table Rank Array */
void pss_mib_tbl_rank_init(PSS_CB * inst)
{
    uns32 i;

    for (i = 0; i < (NCSPSS_MIB_TBL_RANK_MAX + 1); i++)
        inst->mib_tbl_rank[i] = NULL;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_mib_tbl_desc_init

  DESCRIPTION:       Initialize the MIB Table Description Array in the PSS_CB.

  RETURNS:           void 

*****************************************************************************/
void pss_mib_tbl_desc_init(PSS_CB * inst)
{
    uns32 i;

    for (i = 0; i < MIB_UD_TBL_ID_END; i++)
        inst->mib_tbl_desc[i] = NULL;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_mib_tbl_rank_add

  DESCRIPTION:       Add a table id to the MIB Table Rank Array in PSS_CB

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 pss_mib_tbl_rank_add(PSS_CB * inst, uns32 tbl_id, uns8 rank)
{
    PSS_MIB_TBL_RANK * rec     = inst->mib_tbl_rank[rank];
    PSS_MIB_TBL_RANK * new_rec = NULL;

    /* First check if the table is already present */
    while (rec)
    {
        if (rec->tbl_id == tbl_id)
            return NCSCC_RC_SUCCESS;
        rec = rec->next;
    }

    /* Now add a new entry */
    new_rec = m_MMGR_ALLOC_PSS_TBL_RNK;
    if (new_rec == NULL)
    {
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PSS_TBL_RNK_ALLOC_FAIL,  "pss_mib_tbl_rank_add()"); 
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    new_rec->tbl_id = tbl_id;
    new_rec->next   = inst->mib_tbl_rank[rank];
    inst->mib_tbl_rank[rank] = new_rec;

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_mib_tbl_rank_dest

  DESCRIPTION:       Destroy the contents of the MIB Table Rank Array in PSS_CB 

  RETURNS:           void

*****************************************************************************/
void pss_mib_tbl_rank_dest(PSS_CB * inst)
{
    uns32 i;
    PSS_MIB_TBL_RANK * rec;
    PSS_MIB_TBL_RANK * next_rec;

    /* Visit each rank and destroy the linked list */
    for ( i = 0 ; i < (NCSPSS_MIB_TBL_RANK_MAX + 1); i ++)
    {
        rec = inst->mib_tbl_rank[i];

        while(rec)
        {
            next_rec = rec->next;
            m_MMGR_FREE_PSS_TBL_RNK(rec);
            rec = next_rec;
        }

        inst->mib_tbl_rank[i] = NULL;
    }
}

/*****************************************************************************

  PROCEDURE NAME:    pss_mib_tbl_desc_dest

  DESCRIPTION:       Destroy the contents of the MIB Table Description Array in PSS_CB 

  RETURNS:           void

*****************************************************************************/
void pss_mib_tbl_desc_dest(PSS_CB * inst)
{
    uns32 i;
    PSS_MIB_TBL_INFO * tbl_info;
    PSS_VAR_INFO     * var_info;

    /* Visit each table and destroy the contents */
    for ( i = 0 ; i < MIB_UD_TBL_ID_END; i ++)
    {
        tbl_info = inst->mib_tbl_desc[i];
        if (tbl_info != NULL)
        {
            var_info = tbl_info->pfields;
            pss_free_individual_var_info_pointers(tbl_info);
            if (var_info != NULL)
                m_MMGR_FREE_PSS_VAR_INFO(var_info);

            if(tbl_info->ptbl_info->idx_data != NULL)
                m_MMGR_FREE_PSS_OCT(tbl_info->ptbl_info->idx_data);

            m_MMGR_FREE_PSS_NCSMIB_TBL_INFO(tbl_info->ptbl_info);
            m_MMGR_FREE_PSS_TBL_INFO(tbl_info);

            inst->mib_tbl_desc[i] = NULL;
        }
    }
}

/*****************************************************************************

  PROCEDURE NAME:    pss_index_tree_destroy

  DESCRIPTION:       Routine for destroying the Client_table database of PSS_CB.

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
void pss_index_tree_destroy(NCS_PATRICIA_TREE * pTree)
{
    NCS_PATRICIA_NODE * pNode = NULL;

    /* Walk through each element of the tree and free the memory */
    while (TRUE)
    {
        pNode = ncs_patricia_tree_getnext(pTree, NULL);
        if (pNode == NULL) /* No more nodes */
        {
            ncs_patricia_tree_destroy(pTree);
            memset(pTree, '\0', sizeof(NCS_PATRICIA_TREE));
            return;
        }

        if(ncs_patricia_tree_del(pTree, pNode) != NCSCC_RC_SUCCESS)
            return;

        if (pNode->key_info != NULL)
            m_MMGR_FREE_PSS_CLIENT_ENTRY(pNode->key_info);

        m_MMGR_FREE_PSS_PAT_NODE(pNode);
    }
    return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_pwe_cb_destroy

  DESCRIPTION:       Routine for destroying all clients of PSS.

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_pwe_cb_destroy(PSS_PWE_CB *pwe_cb)
{
    NCS_SPIR_REQ_INFO spir_info;
    uns32            status; 

    if (pwe_cb == NULL)
    {
       return NCSCC_RC_SUCCESS; 
    }

    /* destory the handle */ 
    ncshm_destroy_hdl(NCS_SERVICE_ID_PSS, pwe_cb->hm_hdl);

    /* destroy the MAA handle */ 
    memset(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO));
    spir_info.type = NCS_SPIR_REQ_REL_INST;
    spir_info.i_sp_abstract_name = m_MAA_SP_ABST_NAME;
    spir_info.i_instance_name.length = 0;
    spir_info.i_environment_id =  pwe_cb->pwe_id;
    status = ncs_spir_api(&spir_info);
    if (status != NCSCC_RC_SUCCESS)
    {
       /* log the failure */
       m_LOG_PSS_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
                         PSS_ERR_SPIR_MAA_REL_INST_FAILED,
                         status, pwe_cb->pwe_id);
    }
    m_LOG_PSS_HDLN_I(NCSFL_SEV_INFO, PSS_HDLN_MAA_REL_INST_SUCCESS, 
                     pwe_cb->pwe_id);
    
    /* destroy the PWE (with the help of SPRR API of MDS) */ 
    memset(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO));
    spir_info.type = NCS_SPIR_REQ_REL_INST;
    spir_info.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
    m_MDS_FIXED_VDEST_TO_INST_NAME(PSR_VDEST_ID, &spir_info.i_instance_name);
    spir_info.i_environment_id =  pwe_cb->pwe_id;
    status = ncs_spir_api(&spir_info);
    if (status != NCSCC_RC_SUCCESS)
    {
       /* log the failure */
       m_LOG_PSS_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
                         PSS_ERR_SPIR_MDS_REL_INST_FAILED,
                         status, pwe_cb->pwe_id);
    }
    m_LOG_PSS_HDLN_I(NCSFL_SEV_INFO, PSS_HDLN_MDS_REL_INST_SUCCESS, 
                     pwe_cb->pwe_id);

    /* Free the OAA_tree first */
    status = pss_destroy_pwe_oaa_tree(&pwe_cb->oaa_tree, TRUE);
    if(status != NCSCC_RC_SUCCESS)
    {
       m_LOG_PSS_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
                         PSS_ERR_OAA_TREE_DESTROY_FAILED,
                         status,  pwe_cb->pwe_id);
    }
    else 
    {
       m_LOG_PSS_HDLN_I(NCSFL_SEV_INFO, 
               PSS_HDLN_PWE_CB_OAA_TREE_DESTROY_SUCCESS, pwe_cb->pwe_id);
    }

    /* Destroy the client_table next */
    status = pss_destroy_pwe_client_table(&pwe_cb->client_table, TRUE);
    if(status != NCSCC_RC_SUCCESS)
    {
       m_LOG_PSS_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
                         PSS_ERR_CLIENT_TABLE_TREE_DESTROY_FAILED,
                         status,  pwe_cb->pwe_id);
    }
    else 
    {
       m_LOG_PSS_HDLN_I(NCSFL_SEV_INFO, 
               PSS_HDLN_PWE_CB_CLNT_TREE_DESTROY_SUCCESS, pwe_cb->pwe_id);
    }

#if (NCS_PSS_RED == 1)
    pss_mbcsv_close_ckpt(pwe_cb->ckpt_hdl);
#endif

    pss_pwe_destroy_spcn_wbreq_pend_list(pwe_cb->spcn_wbreq_pend_list);
    pwe_cb->spcn_wbreq_pend_list = NULL;
    pss_free_refresh_tbl_list(pwe_cb);
    pwe_cb->refresh_tbl_list = NULL;

    /* log that PWE_CB for Environment is deleted successfully */ 
    m_LOG_PSS_HDLN_I(NCSFL_SEV_INFO, PSS_HDLN_PWE_CB_DESTROY_SUCCESS, pwe_cb->pwe_id);
    
    m_MMGR_FREE_PSS_PWE_CB(pwe_cb);
       
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_pwe_destroy_spcn_wbreq_pend_list

  DESCRIPTION:       Delete/Free Pending Warmboot-request(to be sent to BAM) list 
                     in PSS_PWE_CB.

  RETURNS:           void

*****************************************************************************/
uns32 pss_pwe_destroy_spcn_wbreq_pend_list(PSS_SPCN_WBREQ_PEND_LIST *spcn_wbreq_pend_list)
{
   PSS_SPCN_WBREQ_PEND_LIST *list = NULL, *tmp = NULL;

   list = spcn_wbreq_pend_list;
   while(list != NULL)
   {
      tmp = list;
      list = list->next;
      m_MMGR_FREE_MAB_PCN_STRING(tmp->pcn);
      m_MMGR_FREE_PSS_SPCN_WBREQ_PEND_LIST(tmp);
   }

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_table_tree_destroy

  DESCRIPTION:       Routine for destroying MIB entries of a particular MIB.
                     If the "retain_tree" boolean is set to FALSE, destroy
                     the patricia-tree data structure as well. 

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_table_tree_destroy(NCS_PATRICIA_TREE * pTree, NCS_BOOL retain_tree)
{
    NCS_PATRICIA_NODE * pNode = NULL;
    uns32              retval;
    PSS_MIB_TBL_DATA * tbl_data = NULL;

    /* Walk through each element of the tree and free the memory */
    while (TRUE)
    {
        pNode = ncs_patricia_tree_getnext(pTree, NULL);
        if (pNode == NULL) /* No more nodes */
        {
            if(!retain_tree)
            { 
               ncs_patricia_tree_destroy(pTree);
               memset(pTree, '\0', sizeof(NCS_PATRICIA_TREE));
            } 
            return NCSCC_RC_SUCCESS;
        }

        retval = ncs_patricia_tree_del(pTree, pNode);
        if (retval != NCSCC_RC_SUCCESS)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

        tbl_data = (PSS_MIB_TBL_DATA *) pNode;
        m_MMGR_FREE_PSS_OCT(tbl_data->key);
        m_MMGR_FREE_PSS_OCT(tbl_data->data);
        m_MMGR_FREE_PSS_MIB_TBL_DATA(tbl_data);
    }
}

/*****************************************************************************

  PROCEDURE NAME:    pss_destroy_pwe_oaa_tree

  DESCRIPTION:       Routine for destroying OAA_tree of PSS_PWE_CB.

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_destroy_pwe_oaa_tree(NCS_PATRICIA_TREE * pTree, NCS_BOOL destroy)
{
   NCS_PATRICIA_NODE * pNode = NULL;
   PSS_OAA_ENTRY    *oaa_node = NULL;
   PSS_OAA_CLT_ID   *oaa_clt_node = NULL, *next_oaa_clt_node = NULL;
   PSS_OAA_KEY      oaa_key;
   uns32            rc = NCSCC_RC_SUCCESS; 

   memset(&oaa_key, '\0', sizeof(oaa_key));
   while(TRUE)
   {
      uns32 j = 0; 

      pNode = ncs_patricia_tree_getnext(pTree, (const uns8 *)&oaa_key);
      if (pNode == NULL)
         break;

      oaa_node = (PSS_OAA_ENTRY*) pNode;
      oaa_key = oaa_node->key;

      for(j=0; j<MAB_MIB_ID_HASH_TBL_SIZE; j++)
      {
         oaa_clt_node = oaa_node->hash[j];
         while(oaa_clt_node != NULL)
         {
            next_oaa_clt_node = oaa_clt_node->next;
            m_MMGR_FREE_MAB_PSS_OAA_CLT_ID(oaa_clt_node);
           oaa_clt_node = next_oaa_clt_node;
         } 
      }
      ncs_patricia_tree_del(pTree, pNode);
      m_MMGR_FREE_PSS_OAA_ENTRY(oaa_node);
   } /* end of freeing OAA tree */ 
   if(destroy)
   {
      rc = ncs_patricia_tree_destroy(pTree);
      memset(pTree, '\0', sizeof(NCS_PATRICIA_TREE));
   }

   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_destroy_pwe_client_table

  DESCRIPTION:       Routine for destroying client_table of PSS_PWE_CB.

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_destroy_pwe_client_table(NCS_PATRICIA_TREE * pTree, NCS_BOOL destroy)
{
   NCS_PATRICIA_NODE * pNode = NULL;
   PSS_TBL_REC      *rec = NULL, *nxt_rec = NULL;
   PSS_CLIENT_ENTRY *entry = NULL;
   PSS_CLIENT_KEY   clt_key;
   uns32            rc = NCSCC_RC_SUCCESS; 

   if(pTree == NULL)
   {
      return NCSCC_RC_SUCCESS; 
   }
   memset(&clt_key, '\0', sizeof(clt_key));
   while(TRUE)
   {
      uns32 tbl_cnt = 0, j = 0; 

      pNode = ncs_patricia_tree_getnext(pTree, (const uns8 *)&clt_key);
      if (pNode == NULL)
         break;

      entry = (PSS_CLIENT_ENTRY*) pNode;
      clt_key = entry->key;

      tbl_cnt = entry->tbl_cnt;
      for(j=0; j<MAB_MIB_ID_HASH_TBL_SIZE; j++)
      {
         rec = entry->hash[j];
         while(rec != NULL)
         {
           nxt_rec = rec->next;
           if(rec->is_scalar)
           {
               if(rec->info.scalar.data != NULL)
               {
                   m_MMGR_FREE_PSS_OCT(rec->info.scalar.data);
                   rec->info.scalar.data = NULL;
               }
           }
           else
           {
               (void)pss_table_tree_destroy(&(rec->info.other.data), 
                         FALSE /* Don't retain the pat-tree */);
           }
           m_MMGR_FREE_PSS_TBL_REC(rec);

           tbl_cnt --;
           rec = nxt_rec; 
         }
        if(tbl_cnt == 0)
           break; 
      }
      ncs_patricia_tree_del(pTree, pNode);
      m_MMGR_FREE_PSS_CLIENT_ENTRY(entry);
   }
   if(destroy)
   {
      rc = ncs_patricia_tree_destroy(pTree);
      memset(pTree, '\0', sizeof(NCS_PATRICIA_TREE));
   }

   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_flush_mbx_msg

  DESCRIPTION:       Routine for flush individual message in the mailbox.

  RETURNS:           TRUE - If message flushed.
                     FALSE - otherwise.

*****************************************************************************/
NCS_BOOL pss_flush_mbx_msg(void *arg, void *msg)
{
   USE(arg);

   pss_free_mab_msg(msg, TRUE);

   return TRUE;
}

void pss_cb_data_dump( )
{
   FILE *fh = NULL;
   PSS_CSI_NODE *csi = NULL;
   PSS_PWE_CB *pwe_cb = NULL;
   PSS_OAA_ENTRY *oaa_entry = NULL;
   PSS_OAA_KEY   oaa_key;
   PSS_OAA_CLT_ID *oaa_clt_entry = NULL;
   NCS_PATRICIA_NODE *pNode = NULL;
   PSS_TBL_REC *trec = NULL;
   PSS_CLIENT_KEY client_key;
   PSS_CLIENT_ENTRY *clt_node = NULL;
   uns32 i = 0, j = 0;
   PSS_SPCN_LIST *list = NULL;
   int8            asc_tod[70]={0};
   time_t          tod;
   int8            tmp_file[90]={0};

   if(gl_pss_amf_attribs.pss_cb == NULL)
      return;

   m_GET_ASCII_DATE_TIME_STAMP(tod, asc_tod);
   m_NCS_STRCPY(tmp_file, "/tmp/ncs_pssv_dump");
   m_NCS_STRCAT(tmp_file, asc_tod);
   m_NCS_STRCAT(tmp_file, ".txt");

   fh = sysf_fopen(tmp_file, "w");
   if(fh == NULL)
      return;

   sysf_fprintf(fh, "*************PSS DATA STRUCTS DUMP START(%s)*************\n", asc_tod);

   sysf_fprintf(fh, "PERSISTENT-STORE-LOCATION: " OSAF_LOCALSTATEDIR "pssv_store \n");

   sysf_fprintf(fh, "MIB_DESC_INFO-DB:*** DUMP START***\n");
   fflush(fh);
   for(i = 0; i < MIB_UD_TBL_ID_END; i++)
   {
      if(gl_pss_amf_attribs.pss_cb->mib_tbl_desc[i] == NULL)  
         continue;

      sysf_fprintf(fh, "\tTable[%d]:%s, Capability=%d, Rank=%d loaded\n", 
         i, gl_pss_amf_attribs.pss_cb->mib_tbl_desc[i]->ptbl_info->mib_tbl_name,
         gl_pss_amf_attribs.pss_cb->mib_tbl_desc[i]->capability,
         gl_pss_amf_attribs.pss_cb->mib_tbl_desc[i]->ptbl_info->table_rank);
      /* pss_dump_mib_desc(gl_pss_amf_attribs.pss_cb->mib_tbl_desc[i]); */
      fflush(fh);
   }
   sysf_fprintf(fh, "MIB_DESC_INFO-DB:*** DUMP END***\n");
   fflush(fh);

   for(list = gl_pss_amf_attribs.pss_cb->spcn_list; list != NULL; list = list->next)
   {
      if(list == gl_pss_amf_attribs.pss_cb->spcn_list)
      {
         sysf_fprintf(fh, "SPCN-LIST:*** DUMP START***\n");
         fflush(fh);
      }

      sysf_fprintf(fh, "\tClient:%s, Source=%s\n", list->pcn, 
         (list->plbck_frm_bam?"XML":"PSS"));
      fflush(fh);
   }
   if(gl_pss_amf_attribs.pss_cb->spcn_list != NULL)
   {
      sysf_fprintf(fh, "SPCN-LIST:*** DUMP END***\n");
      fflush(fh);
   }

   for(csi = gl_pss_amf_attribs.csi_list; csi != NULL; csi = csi->next)
   {
      pwe_cb = csi->pwe_cb;
      sysf_fprintf(fh, "***CSI:%d: *** DUMP START***\n", pwe_cb->pwe_id);
      
      sysf_fprintf(fh, "OAA-DB:*** DUMP START***\n");
      fflush(fh);
      memset(&oaa_key, '\0', sizeof(oaa_key));
      while((pNode = ncs_patricia_tree_getnext(&pwe_cb->oaa_tree,
                  (const uns8 *)&oaa_key)) != NULL)
      {
         oaa_entry  = (PSS_OAA_ENTRY *) pNode;
         oaa_key = oaa_entry->key;

         sysf_fprintf(fh, "\t#OAA-MDS_DEST:%x:%x:%x:%x:%x:%x:%x:%x:\n",
            ((uns8*)&oaa_key.mds_dest)[0], ((uns8*)&oaa_key.mds_dest)[1],
            ((uns8*)&oaa_key.mds_dest)[2], ((uns8*)&oaa_key.mds_dest)[3],
            ((uns8*)&oaa_key.mds_dest)[4], ((uns8*)&oaa_key.mds_dest)[5],
            ((uns8*)&oaa_key.mds_dest)[6], ((uns8*)&oaa_key.mds_dest)[7]
            );
         sysf_fprintf(fh, "\t - Table-Count : %d\n", oaa_entry->tbl_cnt);
         fflush(fh);

         for(i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++)
         {
             sysf_fprintf(fh, "\t - hash[%d]: START POINTER=%lx\n", i, (long)(oaa_entry->hash[i]));
             fflush(fh);

             for(j = 0, oaa_clt_entry = oaa_entry->hash[i]; 
                 oaa_clt_entry != NULL;
                 oaa_clt_entry = oaa_clt_entry->next, ++j)
             {
                sysf_fprintf(fh, "\t\t - [%d] - tbl_rec = %lx, tbl_id = %d, tbl_name = %s\n", 
                   j, (long)oaa_clt_entry->tbl_rec, oaa_clt_entry->tbl_rec->tbl_id,
                   gl_pss_amf_attribs.pss_cb->mib_tbl_desc[oaa_clt_entry->tbl_rec->tbl_id]->ptbl_info->mib_tbl_name);
                fflush(fh);
             }
         }
      }
      sysf_fprintf(fh, "OAA-DB:*** DUMP END***\n");
      fflush(fh);
 
      sysf_fprintf(fh, "CLIENT_TABLE-DB:*** DUMP START***\n");
      fflush(fh);
      memset(&client_key, '\0', sizeof(client_key));
      while((pNode = ncs_patricia_tree_getnext(&pwe_cb->client_table,
                  (const uns8 *)&client_key)) != NULL)
      {
         clt_node = (PSS_CLIENT_ENTRY *) pNode;
         client_key = clt_node->key;

         sysf_fprintf(fh, "\t#CLIENT_ENTRY:key:%s\n", (char*)client_key.pcn);
         sysf_fprintf(fh, "\t - Table-Count : %d\n", clt_node->tbl_cnt);
         fflush(fh);

         for(i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++)
         {
             sysf_fprintf(fh, "\t - hash[%d]: START POINTER=%lx\n", i, 
                (long)clt_node->hash[i]);
             fflush(fh);

             for(j = 0, trec = clt_node->hash[i]; trec != NULL; trec = trec->next, ++j)
             {
                if(gl_pss_amf_attribs.pss_cb->mib_tbl_desc[trec->tbl_id] != NULL) 
                {
                   sysf_fprintf(fh, "\t\t - [%d] - tbl_rec = %lx, tbl_id = %d, tbl_name = %s\n", 
                      j, (long)trec, trec->tbl_id,
                      gl_pss_amf_attribs.pss_cb->mib_tbl_desc[trec->tbl_id]->ptbl_info->mib_tbl_name);
                   sysf_fprintf(fh, "\t\t\t\t - dirty:%d, is_scalar:%d,\n", trec->dirty, trec->is_scalar);
                   sysf_fprintf(fh, "\t\t\t\t - client_key:%s\n", trec->pss_client_key->pcn);
                   sysf_fprintf(fh, "\t\t\t\t - oaa_entry-pointer:%lx\n", (long)trec->p_oaa_entry);
                   sysf_fprintf(fh, "\t\t\t\t -  Related OAA-MDS_DEST:%x:%x:%x:%x:%x:%x:%x:%x:\n",
                      ((uns8*)&trec->p_oaa_entry->key.mds_dest)[0], 
                      ((uns8*)&trec->p_oaa_entry->key.mds_dest)[1],
                      ((uns8*)&trec->p_oaa_entry->key.mds_dest)[2], 
                      ((uns8*)&trec->p_oaa_entry->key.mds_dest)[3],
                      ((uns8*)&trec->p_oaa_entry->key.mds_dest)[4], 
                      ((uns8*)&trec->p_oaa_entry->key.mds_dest)[5],
                      ((uns8*)&trec->p_oaa_entry->key.mds_dest)[6], 
                      ((uns8*)&trec->p_oaa_entry->key.mds_dest)[7]
                   );
                   fflush(fh);
                }
                else  
                {
                   sysf_fprintf(fh, "\t\t - [%d] - tbl_rec = %lx, tbl_id = %d, tbl_description = NULL!!!\n", j, (long)trec, trec->tbl_id);
                   fflush(fh);
                }
             }
         }
      }
      sysf_fprintf(fh, "CLIENT_TABLE-DB:*** DUMP END***\n");
 
      sysf_fprintf(fh, "***CSI:%d: *** DUMP END***\n", csi->pwe_cb->pwe_id);
      fflush(fh);
   }
 
   /* dump the standby psr oaa down list contents */
   sysf_fprintf(fh, "*************STANDBY PSS OAA DOWN LIST DUMP START*************\n");
   for(csi = gl_pss_amf_attribs.csi_list; csi != NULL; csi = csi->next)
   {
       pwe_cb = csi->pwe_cb;
       pss_stdby_oaa_down_list_dump(pwe_cb->pss_stdby_oaa_down_buffer,fh);
       fflush(fh);
   }
   sysf_fprintf(fh, "*************STANDBY PSS OAA DOWN LIST DUMP END*************\n");

   sysf_fprintf(fh, "*************PSS DATA STRUCTS DUMP END(%s)*************\n", asc_tod);
   fflush(fh);
   sysf_fclose(fh);
   return;
}

#endif /* (NCS_PSS == 1) */
#endif /* (NCS_MAB == 1) */
