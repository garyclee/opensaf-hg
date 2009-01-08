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


  MODULE NAME: NCSMIBLIB.C

..............................................................................


  DESCRIPTION: Contains the common MIB LIB routines that can be used by
               all NetPlane subsystems.

******************************************************************************
*/

#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"


#include "ncs_mib.h"
#include "mib_mem.h"
#include "ncsmiblib.h"
#include "ncssysf_mem.h"
#include "usrbuf.h"


/* Some MIB LIB defines */
#define  NCSMIB_MAX_INST_LEN         200
#define  NCSMIB_MAX_NUM_OBJS_IN_TBL  100
#define  NCSMIB_MAX_CUML_PARAM_SIZE  1024
#define  NCSMIB_MAX_VAR_LEN          1024 

/* The data-structure that stores the table info. of all tables registered
 * with MIB lib. Currently, the valid table_id range is 
 * (1..MIB_UD_TBL_ID_END)
 */

static NCSMIB_OBJ_INFO* miblib_obj_info[MIB_UD_TBL_ID_END];

/* MIB LIB internal functions. Application developers don't need to 
 * be educated about these.
 */
EXTERN_C uns32 miblib_validate_mib_obj(NCSMIB_OBJ_INFO *val_obj, 
                                       NCSMIB_ARG *arg);
EXTERN_C uns32 miblib_validate_set_mib_obj(NCSMIB_OBJ_INFO* val_obj, 
                                           NCSMIB_PARAM_VAL* param);

EXTERN_C uns32 miblib_validate_row_status(uns32 *row_status, 
                                          NCSCONTEXT row);

EXTERN_C uns32 miblib_process_getrow_nextrow(NCSCONTEXT cb, 
                                             NCSMIB_ARG* args,
                                             uns32* next_inst_id, 
                                             uns32* next_inst_id_len);

EXTERN_C uns32 miblib_process_mib_op_req(NCSCONTEXT cb, NCSMIB_ARG* args);

EXTERN_C uns32 miblib_process_register_req(NCSMIB_OBJ_INFO* obj_info);

EXTERN_C uns32 miblib_process_setrow_testrow(NCSCONTEXT cb,
                                             NCSMIB_ARG* args);

EXTERN_C uns32 miblib_process_setallrows(NCSCONTEXT cb, NCSMIB_ARG* args);

EXTERN_C uns32 miblib_process_removerows(NCSCONTEXT cb, NCSMIB_ARG* args);

EXTERN_C uns32 miblib_set_obj_value(NCSMIB_PARAM_VAL *param_val, 
                                    NCSMIB_VAR_INFO *var_info, 
                                    NCSCONTEXT data, 
                                    NCS_BOOL *val_same_flag);

EXTERN_C uns32 miblib_validate_index(NCSMIB_OBJ_INFO *val_obj, 
                                     NCSMIB_IDX *idx,
                                     NCS_BOOL next_flg);

EXTERN_C uns32 miblib_validate_param_val(NCSMIB_VAR_INFO* var_info, 
                                         NCSMIB_PARAM_VAL* param,
                                         NCS_BOOL next_flg);


/**************************************************************************
 * Function:    miblib_get_obj_val
 * Description: MIB LIB internal function for getting the object value from
 *                a data-structure provided by the subsystem
 * Input:       NCSMIB_PARAM_VAL - Pointer to the param_val structure which
 *                needs to be populated by this function
 *              NCSMIB_VAR_INFO - pointer to the parameter's var info struct
 *              NCSCONTEXT - Pointer to the structure from which value needs
 *                to be extracted
 * Returns:     NCSCC_RC_SUCCESS/FAILURE
 * Notes:       NCSCONTEXT buffer - this argument has been added so that the
 *              prototype of this fucntion is identical to the "extract"
 *              function, the idea being that subsystems can register this
 *              function as their extract function. Note that this function
 *              does not use the temporary buffer.
 **************************************************************************/

uns32 ncsmiblib_get_obj_val(NCSMIB_PARAM_VAL *param_val, NCSMIB_VAR_INFO *var_info, 
                           NCSCONTEXT data, NCSCONTEXT buffer)
{
   switch(var_info->obj_type)
   {
   case NCSMIB_INT_RANGE_OBJ_TYPE:
   case NCSMIB_INT_DISCRETE_OBJ_TYPE:
   case NCSMIB_OTHER_INT_OBJ_TYPE:
      param_val->i_fmat_id = NCSMIB_FMAT_INT;
      param_val->i_length  = sizeof(uns32);
      switch(var_info->len)      
      {
      case sizeof(uns8):
         param_val->info.i_int = 
            (uns32)(*((uns8*)((uns8*)data + var_info->offset)));
         break;
      case sizeof(uns16):
         param_val->info.i_int = 
            (uns32)(*((uns16*)((uns8*)data + var_info->offset)));
         break;
      case sizeof(uns32):
         param_val->info.i_int= 
            (uns32)(*((uns32*)((uns8*)data + var_info->offset)));
         break;
      default:
         break;
      }
      break;
      
   case NCSMIB_OCT_OBJ_TYPE:
   case NCSMIB_OTHER_OCT_OBJ_TYPE:
      param_val->i_fmat_id = NCSMIB_FMAT_OCT;                  
      if(var_info->obj_spec.stream_spec.min_len == var_info->obj_spec.stream_spec.max_len) /* fixed length octet string */
          param_val->i_length  = var_info->obj_spec.stream_spec.max_len;
      else /* variable length octet string */
          return NCSCC_RC_FAILURE;

      param_val->info.i_oct = ((uns8*)data + var_info->offset);
      break;
         
   default: /* Discrete octet string object also not retrieved. so return failure. */
      return NCSCC_RC_FAILURE;
   } /* switch */
   
   return NCSCC_RC_SUCCESS;
}


/**************************************************************************
 * Function:    miblib_set_obj_value
 * Description: MIB LIB internal function for setting the object value. 
 * Input:       NCSMIB_PARAM_VAL* pointer to a struct containing the param
 *                 value to set.
 *              NCSMIB_ARG*  pointer to mib argument structure
 *              NCSMIB_VAR_INFO  pointer to the var info structure 
 *              NCSCONTEXT - Pointer to the subsystem's data-structure into 
 *                 which the parameter needs to be set                     
 *              NCS_BOOL*  - Flag returned by reference.
 * Returns:     NCSCC_RC_SUCCESSS/FAILURE
 *              NCS_BOOL* val_same_flag : Flag returned by reference. Will be
 *                 set to TRUE if the value to set is the same as the value
 *                 that exists in the subsystems data-structure. Will be
 *                 set to FALSE otherwise
 *
 * Notes:       In the case of OCTET type parameters, it is assumed that
 *              the subsystem would have allocated a buffer that is 
 *              sufficient to store the max number of bytes.
 **************************************************************************/
uns32 miblib_set_obj_value(NCSMIB_PARAM_VAL *param_val, 
                           NCSMIB_VAR_INFO *var_info, 
                           NCSCONTEXT data, NCS_BOOL *val_same_flag)
{
    uns32 max_len;
    /* Added below check to fix the bug 60022 */
    if (var_info == NULL || param_val == NULL)
    {
        return NCSCC_RC_FAILURE;
    }
    /* make sure the data types are same  */
    if (param_val->i_fmat_id != var_info->fmat_id)
        return NCSCC_RC_FAILURE; 

    switch(param_val->i_fmat_id)
    {
    case NCSMIB_FMAT_INT:
    case NCSMIB_FMAT_BOOL:
       switch (var_info->len)
       {
       case sizeof(uns8):
          if(*(uns8*)((uns8*)data + var_info->offset) ==
             (uns8)(param_val->info.i_int))
             *val_same_flag = TRUE;
          else
             *(uns8*)((uns8*)data + var_info->offset) =
             (uns8)(param_val->info.i_int);
          break;

       case sizeof(uns16):
          if(*(uns16*)((uns8*)data + var_info->offset) ==
             (uns16)param_val->info.i_int)
             *val_same_flag = TRUE;
          else
             *(uns16*)((uns8*)data + var_info->offset) = 
             (uns16)param_val->info.i_int;
          break;

       case sizeof(uns32):
          if(*(uns32*)((uns8*)data + var_info->offset) ==
             (uns32)param_val->info.i_int)
             *val_same_flag = TRUE;
          else
             *(uns32*)((uns8*)data + var_info->offset) = 
             (uns32)param_val->info.i_int;
          break;

       default:
          return NCSCC_RC_FAILURE;
       }
       break;

       case NCSMIB_FMAT_OCT:
          if(var_info->obj_type == NCSMIB_OCT_OBJ_TYPE)
          {
               max_len = var_info->obj_spec.stream_spec.max_len;
          }
          else /* Discrete octet strings */
          {
               max_len = ncsmiblib_get_max_len_of_discrete_octet_string(var_info);
          }

            /* Code to handle zero length string */
            if(param_val->i_length == 0)
            {
                 m_NCS_OS_MEMSET(((uns8*)data + var_info->offset), 0,
                             max_len);
                *val_same_flag = FALSE;
                break;
            }

            if(m_NCS_OS_MEMCMP(((uns8*)data + var_info->offset),
                param_val->info.i_oct, param_val->i_length) == 0)
            {
                 uns32 i;
                for (i = param_val->i_length; i < max_len; i++)
                {
                    if (*((uns8*)data + var_info->offset+i) != 0)
                        break;
                }
                if (i == max_len)
                    *val_same_flag = TRUE;
                else
                {
                    m_NCS_OS_MEMSET(((uns8*)data + var_info->offset), 0,
                                   max_len);
                    m_NCS_OS_MEMCPY(((uns8*)data + var_info->offset),
                    param_val->info.i_oct, param_val->i_length);
                }
            }
            else
            {
                m_NCS_OS_MEMSET(((uns8*)data + var_info->offset), 0,
                                max_len);
                m_NCS_OS_MEMCPY(((uns8*)data + var_info->offset),
                param_val->info.i_oct, param_val->i_length);
            }
          break;

       default:
          return NCSCC_RC_INV_VAL;
    }
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function:    miblib_validate_index
 * Description: MIB LIB internal function for validation of the index  
 * Input:       val_obj - Pointer to NCSMIB_OBJ_INFO structure for the 
 *                 appropriate MIB table
 *              idx  - pointer to a struct of the type NCSMIB_IDX. Contains
 *                 the number of octets in the index and a pointer to the 
 *                 actual index
 *              next_flg - Set to TRUE when this function is called for NEXT
 *                         (or) NEXTROW operation else set to FALSE.
 * Returns:     NCSCC_RC_SUCCESSS/FAILURE
 * Notes:       .
 *****************************************************************************/
uns32 miblib_validate_index(NCSMIB_OBJ_INFO *val_obj,
                            NCSMIB_IDX *idx,
                            NCS_BOOL next_flg)
{
    uns16  i;
    uns32  actual_inst_len = 0;
    uns32  stream_len;
    uns16  octet_len;
    uns32  remaining_len = idx->i_inst_len;
    const uns32* inst_ids = idx->i_inst_ids;
    NCSMIB_TBL_INFO* tbl_info = &(val_obj->tbl_info);
    NCSMIB_VAR_INFO* var_info;
    NCSMIB_PARAM_VAL param;
    uns16            param_id;

    for(i = 0; i < (tbl_info->idx_data.index_info.num_index_ids); i++)
    {
        /* Do the fancy index validation here */  
        if ((remaining_len == 0) && (next_flg == TRUE))
            break;

        param_id = tbl_info->idx_data.index_info.index_param_ids[i];
        var_info = &(val_obj->var_info[param_id-1]);
        switch(var_info->obj_type)
        {
        case NCSMIB_INT_RANGE_OBJ_TYPE:
        case NCSMIB_INT_DISCRETE_OBJ_TYPE:
        case NCSMIB_OTHER_INT_OBJ_TYPE:
            if(remaining_len >= 1)
            {

                /* check whether the integer value is in the right range */
                param.i_fmat_id = NCSMIB_FMAT_INT;
                param.i_length = 1;
                param.i_param_id = param_id;
                param.info.i_int = *inst_ids;
                if(miblib_validate_param_val(var_info, &param, next_flg) != NCSCC_RC_SUCCESS)
                {
                    /* Param val not in the valid range */
                    return NCSCC_RC_FAILURE;   
                }
                actual_inst_len++; /* Increment length by size of an INTEGER */
                remaining_len-- ;
                inst_ids++;   /* Move pointer by 4 bytes */
            }
            else
            {
                /* The key seems to be shorter than expected */
                return NCSCC_RC_FAILURE;
            }
            break;

        case NCSMIB_OCT_OBJ_TYPE:
        case NCSMIB_OCT_DISCRETE_OBJ_TYPE:
        case NCSMIB_OTHER_OCT_OBJ_TYPE:
            if(remaining_len >= 1)
            {
                /* check whether variable length octet or constant length octet */
                if (var_info->obj_spec.stream_spec.min_len == 
                   var_info->obj_spec.stream_spec.max_len)
                {
                   /* constant length octet string */
                   stream_len = var_info->obj_spec.stream_spec.max_len; 
                   if ((remaining_len < stream_len) && (next_flg == TRUE))
                      stream_len = remaining_len;
                   octet_len = (uns16)stream_len; 
                }
                else
                {
                   /* variable length octet string */
                   octet_len = (uns16)(*inst_ids);
                   /* Number of bytes required for the OCTET type index */
                   stream_len = (octet_len);                   
                   actual_inst_len++; /* Increment length by size of an INTEGER */
                   remaining_len--;
                   inst_ids++;   /* Move pointer by 4 bytes */
                }
            }
            else
            {
                /* The key seems to be shorter than expected */
                return NCSCC_RC_FAILURE;
            }

            if(remaining_len >= stream_len)
            {
                /* validate the OCTET type param */
                param.i_fmat_id = NCSMIB_FMAT_OCT;
                param.i_length = octet_len;
                param.i_param_id = param_id;
                /* For OCT types, only the length is validated. It's ok if 
                 * param.info field is not set
                 */
                if(miblib_validate_param_val(var_info, &param, next_flg) != NCSCC_RC_SUCCESS)
                {
                    /* Param val not in the valid range */
                    return NCSCC_RC_FAILURE;   
                }
                actual_inst_len += stream_len;
                remaining_len -= stream_len;
                inst_ids += stream_len;
            }
            else
            {
                if (next_flg == TRUE)
                {
                  param.i_fmat_id = NCSMIB_FMAT_OCT;
                  param.i_length = octet_len;
                  param.i_param_id = param_id; 
                  /* For OCT types, only the length is validated. It's ok if 
                   * param.info field is not set
                   */
                  if(miblib_validate_param_val(var_info, &param, next_flg) != NCSCC_RC_SUCCESS)
                  {
                      /* Param val not in the valid range */
                      return NCSCC_RC_FAILURE;   
                  }
                  return NCSCC_RC_SUCCESS;
                }
                else
                {
                   /* The key seems to be shorter than expected */
                   return NCSCC_RC_FAILURE;
                }
            }
            break;

        default:
            return NCSCC_RC_FAILURE;
        break;
        }
    }

    if (actual_inst_len == idx->i_inst_len)
    {
        return NCSCC_RC_SUCCESS;
    }
    else
    {
        /* Something wrong with the index */
        return NCSCC_RC_FAILURE;
    }
}

/*****************************************************************************
 * Function:    miblib_validate_mib_obj
 * Description: MIB LIB function for validation of the MIB objects. This
 *              function does the following:
 *                 - Validates the index.
 *                 - Validates whether obj is settable, based on 
 *                 -  
 * Input:       val_obj - is of NCSMIB_OBJ_INFO type,
 *              arg  - pointer ot the NCSMIB_ARG structure
 * Returns:     NCSCC_RC_SUCCESSS/FAILURE
 * Notes:       This function takes NCSMIB_OBJ_INFO structure which contains 
 *              the Variable specific information and table information to
 *              which this object belongs.
 *****************************************************************************/
uns32 miblib_validate_mib_obj(NCSMIB_OBJ_INFO *val_obj, NCSMIB_ARG *arg)
{
    NCSMIB_TBL_INFO    *tbl_info = &(val_obj->tbl_info);
    uns32             param_id = arg->req.info.get_req.i_param_id;
    uns32             status = NCSCC_RC_SUCCESS; 
    uns32             ret_code;

   /* Do the param id validations and access checks first */
   switch(arg->i_op)
   {
   case NCSMIB_OP_REQ_GET:
   case NCSMIB_OP_REQ_NEXT:
   
      /* Check the object id's range within that table */
      if (param_id > tbl_info->num_objects)
         return NCSCC_RC_NO_OBJECT;

      if (val_obj->var_info[param_id-1].status == NCSMIB_OBJ_OBSOLETE)   
         return NCSCC_RC_NO_OBJECT;

      if (val_obj->var_info[param_id-1].access == NCSMIB_ACCESS_NOT_ACCESSIBLE ||
         val_obj->var_info[param_id-1].access == NCSMIB_ACCESS_ACCESSIBLE_FOR_NOTIFY)
         return NCSCC_RC_NO_ACCESS;
      break;

   case NCSMIB_OP_REQ_SET:
      if((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
      {
         if ((status = miblib_validate_set_mib_obj(val_obj, 
                &(arg->req.info.set_req.i_param_val))) != NCSCC_RC_SUCCESS)
             return status;
      }
      else
         return NCSCC_RC_SUCCESS;
      break;

   case NCSMIB_OP_REQ_TEST:
      if ((status = miblib_validate_set_mib_obj(val_obj, 
                &(arg->req.info.set_req.i_param_val))) != NCSCC_RC_SUCCESS)
          return status;
      break;

   case NCSMIB_OP_REQ_GETROW:
   case NCSMIB_OP_REQ_NEXTROW:
   case NCSMIB_OP_REQ_SETROW:
   case NCSMIB_OP_REQ_TESTROW:
   case NCSMIB_OP_REQ_MOVEROW:
      break;

   default:
      return NCSCC_RC_INV_VAL;
   }

   /* Index has no significance for tables that represent scalars */
   if(tbl_info->table_of_scalars == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   /* Now validate the instance ids */
   if(tbl_info->index_in_this_tbl)
   {  
      /* All indices belong to this table. Since var info of those indices
       * is also available, we have enough info. to validate them
       */
      switch(arg->i_op)
      {
      case NCSMIB_OP_REQ_GET:
      case NCSMIB_OP_REQ_SET:
      case NCSMIB_OP_REQ_TEST:
      case NCSMIB_OP_REQ_GETROW:
      case NCSMIB_OP_REQ_SETROW:
      case NCSMIB_OP_REQ_MOVEROW:
      case NCSMIB_OP_REQ_TESTROW:
         if(miblib_validate_index(val_obj, &(arg->i_idx), FALSE) != NCSCC_RC_SUCCESS)
         {
            return NCSCC_RC_NO_INSTANCE;
         }
         break;

      case NCSMIB_OP_REQ_NEXT:
      case NCSMIB_OP_REQ_NEXTROW:
         if(arg->i_idx.i_inst_len != 0)
         {
            if(miblib_validate_index(val_obj, &(arg->i_idx), TRUE) != NCSCC_RC_SUCCESS)
            {
               return NCSCC_RC_NO_INSTANCE;
            }
         }
         break;

      default:
         break;
      }
   }
   else    
   {
      /* One or more of the indices belong to a different table. We
       * don't have info on those indices. So, call the user provided
       * function for doing the validation.
       */
      if ((ret_code = tbl_info->idx_data.validate_idx(arg)) != NCSCC_RC_SUCCESS)
      {
         return ret_code;
      }
   }

   return NCSCC_RC_SUCCESS;
}

/**************************************************************************
 * Function:    miblib_validate_row_status
 * Description: MIB LIB utility function for validating the row status.
 *              
 * Input:       Pointer to uns32, NCSCONTEXT of row
 * Returns:     NCSCC_RC_SUCCESSS/FAILURE
 * Notes:  
 **************************************************************************/
uns32 miblib_validate_row_status(uns32 *row_status, NCSCONTEXT row)
{
   switch(*row_status)
   {
   case NCSMIB_ROWSTATUS_ACTIVE:
   case NCSMIB_ROWSTATUS_NOTINSERVICE:
      if (row == NULL)
          return NCSCC_RC_INV_SPECIFIC_VAL;  
      break;
   case NCSMIB_ROWSTATUS_CREATE_GO:
      if(row != NULL)
         return NCSCC_RC_INV_SPECIFIC_VAL;
      *row_status = NCSMIB_ROWSTATUS_ACTIVE;
      break;
   case NCSMIB_ROWSTATUS_CREATE_WAIT:
      if(row != NULL)
         return NCSCC_RC_INV_SPECIFIC_VAL;
      *row_status = NCSMIB_ROWSTATUS_NOTINSERVICE;
      break;

   case NCSMIB_ROWSTATUS_DESTROY:
      if(row == NULL)
         return NCSCC_RC_NO_INSTANCE;
      break;
   case NCSMIB_ROWSTATUS_NOTREADY:
   default:
      return NCSCC_RC_INV_SPECIFIC_VAL;
   }
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function:    miblib_validate_param_val
 * Description: MIB LIB function for validation of the param value 
 * Input:       var_info - is the pointer to the NCSMIB_VAR_INFO struct for
 *                 the param
 *              param  - pointer to the PARAM_VAL structure
 *              next_flg - Set to TRUE when this function is called for NEXT
 *                         (or) NEXTROW operation else set to FALSE.
 * Returns:     NCSCC_RC_SUCCESSS/FAILURE
 * Notes:       
 *****************************************************************************/
uns32 miblib_validate_param_val(NCSMIB_VAR_INFO* var_info, 
                                NCSMIB_PARAM_VAL* param,
                                NCS_BOOL next_flg)
{
    uns32 ret_code = NCSCC_RC_SUCCESS;
    uns32 i;
    uns32   max_val;

    switch(var_info->obj_type)
    {
    case NCSMIB_INT_RANGE_OBJ_TYPE:
        if(next_flg==FALSE) /* get or set request.. */
        {
           if((param->info.i_int < var_info->obj_spec.range_spec.min) ||
                (param->info.i_int > var_info->obj_spec.range_spec.max))
            ret_code = NCSCC_RC_FAILURE;
        }
        else /* get-next request. */
        {
            if(param->info.i_int > var_info->obj_spec.range_spec.max)
                ret_code = NCSCC_RC_FAILURE;
        }
        break;

    case NCSMIB_INT_DISCRETE_OBJ_TYPE:
        for (i = 0; i < var_info->obj_spec.value_spec.num_values; i++)
        {
            if((param->info.i_int >= var_info->obj_spec.value_spec.values[i].min) && 
               (param->info.i_int <= var_info->obj_spec.value_spec.values[i].max))
            {
                ret_code = NCSCC_RC_SUCCESS;
                break;
            }
        }

        /* We're here because the param val was not found in the array of
         * valid param values
         */
        if (i == var_info->obj_spec.value_spec.num_values)
        {  
           if(next_flg == FALSE) /* get or set request. */
               ret_code = NCSCC_RC_FAILURE;
           else /* get-next request. */
           {
               max_val = var_info->obj_spec.value_spec.values[0].max;
               for(i=1; i<var_info->obj_spec.value_spec.num_values; i++)
                    if(var_info->obj_spec.value_spec.values[i].max > max_val)
                        max_val = var_info->obj_spec.value_spec.values[i].max;

               if(param->info.i_int > max_val) 
                   ret_code = NCSCC_RC_FAILURE;
           }
        }
        break;

    case NCSMIB_TRUTHVAL_OBJ_TYPE:
        if((param->info.i_int < NCS_SNMP_TRUE) ||
           (param->info.i_int > NCS_SNMP_FALSE))
            ret_code = NCSCC_RC_FAILURE;
        break;

    case NCSMIB_OCT_OBJ_TYPE:
        if(next_flg == FALSE) /* get or set request */
        {
            if((param->i_length < var_info->obj_spec.stream_spec.min_len) ||
               (param->i_length > var_info->obj_spec.stream_spec.max_len))
                ret_code = NCSCC_RC_FAILURE; 
        }
        else /* get-next request */
        {
            if(param->i_length > var_info->obj_spec.stream_spec.max_len)
                ret_code = NCSCC_RC_FAILURE;
        }
        break;
    
    case NCSMIB_OCT_DISCRETE_OBJ_TYPE:
        for (i = 0; i < var_info->obj_spec.disc_stream_spec.num_values; i++)
        {
            if((param->i_length >= var_info->obj_spec.disc_stream_spec.values[i].min_len) && 
               (param->i_length <= var_info->obj_spec.disc_stream_spec.values[i].max_len))
            {
                ret_code = NCSCC_RC_SUCCESS;
                break;
            }
        }

        /* We're here because the param val was not found in the array of
         * valid param values
         */
        if (i == var_info->obj_spec.disc_stream_spec.num_values)
        {
           if(next_flg == FALSE) /* get or set request. */
               ret_code = NCSCC_RC_FAILURE;
           else /* get-next request. */
           {
               max_val = ncsmiblib_get_max_len_of_discrete_octet_string(var_info);

               if(param->i_length > max_val) 
                   ret_code = NCSCC_RC_FAILURE;
           }
        }
        break;

        

    case NCSMIB_OTHER_INT_OBJ_TYPE:
    case NCSMIB_OTHER_OCT_OBJ_TYPE:
        /* MIB LIB has no clue how to validate such parameters. Call the 
         * subsystem provided function to do the honours
         */
        if(var_info->obj_spec.validate_obj != NULL)
        {
            ret_code = (var_info->obj_spec.validate_obj)(var_info, param);
        }
        break;

    default:
        ret_code = NCSCC_RC_FAILURE;
        break;
    }
    return ret_code;
}



/*****************************************************************************
 * Function:    miblib_validate_set_mib_obj
 * Description: Generic function for validation of the MIB objects in a 
 *              SET/SETROW request
 * Input:       val_obj - is of NCSMIB_OBJ_INFO type,
 *              param  - PARAM_VAL_ID structure
 * Returns:     NCSCC_RC_SUCCESSS/FAILURE
 * Notes:       This function takes NCSMIB_OBJ_INFO structure which contains 
 *              the Variable specific information and table information to
 *              which this object belongs.
 *****************************************************************************/
uns32 miblib_validate_set_mib_obj(NCSMIB_OBJ_INFO* val_obj, 
                                  NCSMIB_PARAM_VAL* param)
{
    NCSMIB_VAR_INFO    *var_info = &(val_obj->var_info[param->i_param_id - 1]);
    NCSMIB_TBL_INFO    *tbl_info = &(val_obj->tbl_info);

    /* Check the object id's range within that table */
    if(param->i_param_id > tbl_info->num_objects)
        return NCSCC_RC_NO_OBJECT;   
        
    if (var_info->status == NCSMIB_OBJ_OBSOLETE)   
         return NCSCC_RC_NO_OBJECT;

    /* Check whether the object is settable or not */
    if ((var_info->access == NCSMIB_ACCESS_NOT_ACCESSIBLE) ||
        (var_info->access == NCSMIB_ACCESS_ACCESSIBLE_FOR_NOTIFY))
        return NCSCC_RC_NO_ACCESS;

    if (var_info->access == NCSMIB_ACCESS_READ_ONLY)
        return NCSCC_RC_NOT_WRITABLE;

    /* Now validate the param value */
    if(miblib_validate_param_val(var_info, param, FALSE) != NCSCC_RC_SUCCESS)
        return NCSCC_RC_INV_VAL;

    return NCSCC_RC_SUCCESS;
}


/*===========================================================================
  Function :  miblib_process_getrow_nextrow
  
  Purpose  :  This function processes the getrow and nextrow requests.

 
  Input    :  NCSCONTEXT cb - pointer to the subsystem's control block. This 
                will be passed when calls are made to subsystem-provided MIB
                functions
              NCSMIB_ARG* 
              
    
  Returns  :  NCSCC_RC_SUCCESSS / FAILURE
 
  Notes    :  
===========================================================================*/
uns32 miblib_process_getrow_nextrow(NCSCONTEXT cb, 
                                    NCSMIB_ARG* args, 
                                    uns32* next_inst_id, 
                                    uns32* next_inst_id_len)
{
    NCSMIB_TBL_INFO* tbl_info = &(miblib_obj_info[args->i_tbl_id]->tbl_info);
    NCSMIB_VAR_INFO* var_info = miblib_obj_info[args->i_tbl_id]->var_info;

    NCSMIB_VAR_INFO* param_var_info;
    NCSMIB_GET_REQ*  i_get_req = &args->req.info.get_req;
    NCSMIB_NEXT_REQ* i_next_req  = &args->req.info.next_req;
    NCSMIB_GETROW_RSP* io_getrow_rsp = &args->rsp.info.getrow_rsp;
    NCSMIB_NEXTROW_RSP* io_nextrow_rsp = &args->rsp.info.nextrow_rsp;
    NCSMIB_PARAM_ID  param_id = 0;
    NCSMIB_PARAM_VAL param_val; 
    NCSPARM_AID      rsp_pa;
    NCSMIB_OP        orig_op = args->i_op;
    USRBUF**        p_rsp_usrbuf;
    NCSCONTEXT       data=NULL;
    uns32           ret_code;
    uns8            buffer[NCSMIB_MAX_VAR_LEN];  /* Temporary buffer in which
                                                 * the subsystem's extract 
                                                 * function can store the
                                                 * param value 
                                                 */

    ncsparm_enc_init(&rsp_pa);

    if(tbl_info->row_in_one_data_struct == TRUE)
    {
        /* get to the first accessible (read-only/read-create) parameter in this table */ 
        for (param_id = 1; param_id <= tbl_info->num_objects; param_id++)
        {
            param_var_info = &(var_info[param_id-1]);
            if (param_var_info->status != NCSMIB_OBJ_CURRENT)
                continue;

            if ((param_var_info->access == NCSMIB_ACCESS_NOT_ACCESSIBLE) || 
                (param_var_info->access == NCSMIB_ACCESS_ACCESSIBLE_FOR_NOTIFY))
                continue;
            
            break;
        }

        if (param_id > tbl_info->num_objects)
            return NCSCC_RC_NO_OBJECT;
        
        /* Get the pointer to the start of the data struct that 
        * contains the row. The subsystem get() should ignore
        * the get_req.param_id. Nevertheless, let us
        * initialize it with a valid value before calling get()
        */
        if(args->i_op == NCSMIB_OP_REQ_GETROW)
        {
            i_get_req->i_param_id = param_id;
            args->i_op = NCSMIB_OP_REQ_GET;
            p_rsp_usrbuf = &(io_getrow_rsp->i_usrbuf);
            ret_code = (tbl_info->get)(cb, args, &data);
        }
        else
        {
            i_next_req->i_param_id = param_id;
            args->i_op = NCSMIB_OP_REQ_NEXT;
            p_rsp_usrbuf = &(io_nextrow_rsp->i_usrbuf);
            ret_code = (tbl_info->next)(cb, args, &data, 
               next_inst_id, next_inst_id_len);
        }
        if(ret_code != NCSCC_RC_SUCCESS)
        {
            /* Could not fetch the row. Return failure */
            return ret_code;
        }

        /* Okay, we got the row. Now, encode the info and save 
        * it in the USRBUF 
        */
        for(param_id = 1; param_id <= tbl_info->num_objects;
            param_id++)
        {
            param_var_info = &(var_info[param_id-1]);
            /* Skip, if the parameter is obsolete or unsupported */
            if(param_var_info->status != NCSMIB_OBJ_CURRENT)
               continue;

            /* Skip, if the parameter is not accessible */
            if(param_var_info->access == NCSMIB_ACCESS_NOT_ACCESSIBLE ||
                param_var_info->access == NCSMIB_ACCESS_ACCESSIBLE_FOR_NOTIFY)
                continue;

            /*Fill the NCSMIB_PARAM_VAL structure */
            param_val.i_param_id = param_id;

            /* Get the object value from the subsystem data-structure */
            if((ret_code = (tbl_info->extract)(&param_val, param_var_info, 
                                               data, buffer))
                != NCSCC_RC_SUCCESS)
            {
                return ret_code;
            }

            /* Encode the param */
            if((ncsparm_enc_param(&rsp_pa, &param_val)) != NCSCC_RC_SUCCESS)
            {
                return NCSCC_RC_FAILURE;
            }

            m_NCS_OS_MEMSET(&param_val, 0, sizeof(NCSMIB_PARAM_VAL));
        }
    }
    else /* Obj in a row that is distributed across multiple data structs */
    {
        /* Initialize all fields reqd in the subsystem's get function */
        if(orig_op == NCSMIB_OP_REQ_GETROW)
        {
            args->i_op = NCSMIB_OP_REQ_GET;
            p_rsp_usrbuf = &(io_getrow_rsp->i_usrbuf);
        }
        else
        {
            args->i_op = NCSMIB_OP_REQ_NEXT;
            p_rsp_usrbuf = &(io_nextrow_rsp->i_usrbuf);
        }

        for(param_id = 1; param_id <= tbl_info->num_objects;
           param_id++)
        {
           param_var_info = &(var_info[param_id-1]);
           /* Skip, if the parameter is obsolete or unsupported */
           if(param_var_info->status != NCSMIB_OBJ_CURRENT)
               continue;

            /* Skip, if the parameter is not accessible */
            if(param_var_info->access == NCSMIB_ACCESS_NOT_ACCESSIBLE ||
                param_var_info->access == NCSMIB_ACCESS_ACCESSIBLE_FOR_NOTIFY)
                continue;

           if(orig_op == NCSMIB_OP_REQ_GETROW)
           {
               i_get_req->i_param_id = param_id;
               ret_code = (tbl_info->get)(cb, args, &data);
           }
           else
           {
               i_next_req->i_param_id = param_id;
               ret_code = (tbl_info->next)(cb, args, &data, 
                  next_inst_id, next_inst_id_len);
           }
           if(ret_code != NCSCC_RC_SUCCESS)
           {
               /* Could not fetch the row. Return failure */
               return ret_code;
           }

           /*Fill the NCSMIB_PARAM_VAL structure */
           param_val.i_param_id = param_id;

           /* Get the object value from the subsystem data-structure */
           if((ret_code = (tbl_info->extract)(&param_val, param_var_info, 
                                              data, buffer))
                != NCSCC_RC_SUCCESS)
           {
               return ret_code;
           }

           if((ncsparm_enc_param(&rsp_pa, &param_val)) != NCSCC_RC_SUCCESS)
           {
               return NCSCC_RC_FAILURE;
           }

           m_NCS_OS_MEMSET(&param_val, 0, sizeof(NCSMIB_PARAM_VAL));
        }
    }
    *p_rsp_usrbuf = ncsparm_enc_done(&rsp_pa);
    return NCSCC_RC_SUCCESS;
}

/*===========================================================================
  Function :  miblib_process_setrow_testrow
  
  Purpose  :  This function is called when a SETROW or TESTROW request it
              made. It decodes the param_vals from the USRBUF so that 
              the subsystem's setrow()/testrow() function can be provided
              with an array of decoded PARAM_VALs. It also validates the
              decoded params.
 
  Input    :  NCSCONTEXT cb - pointer to the subsystem's control block. This 
                will be passed when calls are made to subsystem-provided MIB
                functions
              NCSMIB_ARG* 
              
    
  Returns  :  NCSCC_RC_SUCCESSS / FAILURE
 
  Notes    :  
===========================================================================*/
uns32 miblib_process_setrow_testrow(NCSCONTEXT cb, NCSMIB_ARG* args) 
{
   NCSMIB_TBL_INFO* tbl_info = &(miblib_obj_info[args->i_tbl_id]->tbl_info);
   NCSMEM_AID         mem_aid;
   uns8              space[NCSMIB_MAX_CUML_PARAM_SIZE];
   NCSMIB_PARAM_VAL   param_val;
   NCSPARM_AID        req_pa;
   uns32             param_cnt, i;
   NCSMIB_SETROW_PARAM_VAL setrow_params[NCSMIB_MAX_NUM_OBJS_IN_TBL];
   USRBUF*           usrbuf;
   NCS_BOOL           test_flag;
   uns32             num_objs;
   uns32             ret_code;

   ncsmem_aid_init(&mem_aid, space, NCSMIB_MAX_CUML_PARAM_SIZE);
   m_NCS_OS_MEMSET(&param_val, 0, sizeof(NCSMIB_PARAM_VAL));

   if(args->i_op == NCSMIB_OP_REQ_SETROW)
   {
       usrbuf = args->req.info.setrow_req.i_usrbuf;
       args->req.info.setrow_req.i_usrbuf = NULL;
       args->rsp.info.setrow_rsp.i_usrbuf = m_MMGR_DITTO_BUFR(usrbuf);
       test_flag = FALSE;
   }
   else
   {
       usrbuf = args->req.info.setrow_req.i_usrbuf;
       args->rsp.info.testrow_rsp.i_usrbuf = m_MMGR_DITTO_BUFR(usrbuf);
       test_flag = TRUE;
   }

   /* Find the number of parameters encoded into usrbuf */
   param_cnt = ncsparm_dec_init(&req_pa, usrbuf);
   num_objs = tbl_info->num_objects;
  
   if(param_cnt > num_objs)
       return NCSCC_RC_NO_INSTANCE;

   m_NCS_OS_MEMSET(setrow_params, 0, (num_objs)*sizeof(NCSMIB_SETROW_PARAM_VAL));

   for(i = 1; i <= param_cnt; i++)
   {
      /* Decode the parameter */
      if(ncsparm_dec_parm(&req_pa, &param_val, &mem_aid) != NCSCC_RC_SUCCESS)
      {
         return NCSCC_RC_FAILURE;
      }

      /* validate the MIB object */
      if((args->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
      {
         if ((ret_code = miblib_validate_set_mib_obj(miblib_obj_info[args->i_tbl_id], 
                             &param_val)) != NCSCC_RC_SUCCESS)
         {
            return ret_code;
         }
      }

      (setrow_params[param_val.i_param_id-1]).param = param_val;
      (setrow_params[param_val.i_param_id-1]).set_flag = TRUE;

      m_NCS_OS_MEMSET(&param_val, 0, sizeof(NCSMIB_PARAM_VAL));
   }

   /* Call the subsystem's setrow function to do the rest of the processing */
   ret_code = (tbl_info->setrow)(cb, args, setrow_params,
                                 miblib_obj_info[args->i_tbl_id], 
                                 test_flag);

   return ret_code;

}
/*===========================================================================
  Function :  miblib_process_register_req
  
  Purpose  :  The action function that miblib_process_req calls for
              registering table and var info. for a table.

 
  Input    :  NCSCONTEXT cb - pointer to the subsystem's control block. This 
                will be passed when calls are made to subsystem-provided MIB
                functions
              NCSMIB_ARG* 
              
    
  Returns  :  NCSCC_RC_SUCCESSS / FAILURE
 
  Notes    :  
===========================================================================*/
uns32 miblib_process_register_req(NCSMIB_OBJ_INFO* obj_info)
{
    /* Below check is added to fix the bug 60022 */
    if(obj_info == NULL)
    {
        return NCSCC_RC_FAILURE;
    }
    if(obj_info->tbl_info.table_id >= MIB_UD_TBL_ID_END)
    {
        return NCSCC_RC_FAILURE;
    }
    else
    {
        if( (obj_info->tbl_info.extract == NULL) ||
            (obj_info->tbl_info.set == NULL)     ||
            (obj_info->tbl_info.get == NULL)     ||
            (obj_info->tbl_info.next == NULL)    ||
            (obj_info->tbl_info.setrow == NULL) )
        {
            return NCSCC_RC_FAILURE;
        }
        miblib_obj_info[obj_info->tbl_info.table_id] = obj_info;
    }
    return NCSCC_RC_SUCCESS;
}

/*===========================================================================
  Function :  miblib_process_setallrows
  
  Purpose  :  This function is called when a SETALLROWS request is
              made.
 
  Input    :  NCSCONTEXT cb - pointer to the subsystem's control block. This 
                will be passed when calls are made to subsystem-provided MIB
                functions
              NCSMIB_ARG* 
              
    
  Returns  :  NCSCC_RC_SUCCESSS / FAILURE
 
  Notes    :  
===========================================================================*/
uns32 miblib_process_setallrows(NCSCONTEXT cb, NCSMIB_ARG* args)
{
   NCSMIB_TBL_INFO      *tbl_info = &(miblib_obj_info[args->i_tbl_id]->tbl_info);
   NCSMEM_AID           mem_aid;
   NCSMIB_IDX           idx;
   uns8                 space[NCSMIB_MAX_CUML_PARAM_SIZE];
   NCSMIB_PARAM_VAL     param_val;
   NCSROW_AID           ra;
   uns32                row_cnt, param_cnt, i, j;
   NCSMIB_SETROW_PARAM_VAL setrow_params[NCSMIB_MAX_NUM_OBJS_IN_TBL];
   USRBUF*              usrbuf;
   uns32                num_objs;
   uns32                ret_code, retval;

   ncsmem_aid_init(&mem_aid, space, NCSMIB_MAX_CUML_PARAM_SIZE);
   m_NCS_OS_MEMSET(&param_val, 0, sizeof(NCSMIB_PARAM_VAL));

   /* The usrbuf is required in the response, to be sent to PSSv. */
   usrbuf = args->req.info.setallrows_req.i_usrbuf;
   args->rsp.info.setallrows_rsp.i_usrbuf = m_MMGR_DITTO_BUFR(usrbuf);

   /* Find the number of rows encoded into usrbuf */
   row_cnt = ncssetallrows_dec_init(&ra, usrbuf);
   if(row_cnt == 0)
   {
       return NCSCC_RC_NO_INSTANCE;
   }

   for(j = 1; j <= row_cnt; j++)
   {
       NCSMIB_ARG   lcl_setrow;

       /* Find the number of parameters encoded into usrbuf */
       param_cnt = ncsrow_dec_init(&ra);
       if(param_cnt == 0)
           return NCSCC_RC_FAILURE;

       idx.i_inst_ids = NULL;
       idx.i_inst_len = 0;
       retval = ncsrow_dec_inst_ids(&ra, &idx, NULL);
       if (retval != NCSCC_RC_SUCCESS)
           return NCSCC_RC_FAILURE;

       m_NCS_OS_MEMSET(&lcl_setrow, '\0', sizeof(lcl_setrow));

       num_objs = tbl_info->num_objects;
       if(param_cnt > num_objs)
       {
           m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
           return NCSCC_RC_NO_INSTANCE;
       }

       /* Fix for IR00090791 */   
#if 0    
       m_NCS_OS_MEMSET(setrow_params, 0, (param_cnt)*sizeof(NCSMIB_SETROW_PARAM_VAL));
#endif
       m_NCS_OS_MEMSET(setrow_params, 0,num_objs*sizeof(NCSMIB_SETROW_PARAM_VAL));
       
       for(i = 1; i <= param_cnt; i++)
       {
           /* Decode the parameter */
           if(ncsrow_dec_param(&ra, &param_val, &mem_aid) != NCSCC_RC_SUCCESS)
           {
               m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
               return NCSCC_RC_FAILURE;
           }
           
           /* validate the MIB object */
           if((args->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
           {
              if ((ret_code = miblib_validate_set_mib_obj(miblib_obj_info[args->i_tbl_id], 
                                &param_val)) != NCSCC_RC_SUCCESS)
              {
                 m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
                 return ret_code;
               }
           }
           
           (setrow_params[param_val.i_param_id-1]).param = param_val;
           (setrow_params[param_val.i_param_id-1]).set_flag = TRUE;
           
           m_NCS_OS_MEMSET(&param_val, 0, sizeof(NCSMIB_PARAM_VAL));
       }    /* for(i = 1; i <= param_cnt; i++) */

       /* Populate lcl_setrow structure appropriately. */
       lcl_setrow.i_idx = idx;
       lcl_setrow.i_mib_key = args->i_mib_key;
       lcl_setrow.i_op = NCSMIB_OP_REQ_SETROW;
       lcl_setrow.i_policy = args->i_policy;
       lcl_setrow.i_rsp_fnc = args->i_rsp_fnc;
       lcl_setrow.i_tbl_id = args->i_tbl_id;
       lcl_setrow.i_usr_key = args->i_usr_key;
       lcl_setrow.i_xch_id = args->i_xch_id;
       lcl_setrow.rsp.info.setrow_rsp.i_usrbuf = NULL;
       /* lcl_setrow.space = args->space;
       lcl_setrow.stack = args->stack; */

       /* Call the subsystem's setrow function to do the SETROW processing */
       ret_code = (tbl_info->setrow)(cb, &lcl_setrow, setrow_params,
           miblib_obj_info[args->i_tbl_id], FALSE);

       m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
       idx.i_inst_ids = NULL;

       retval = ncsrow_dec_done(&ra);
       if (retval != NCSCC_RC_SUCCESS)
           return NCSCC_RC_FAILURE;
   }    /* for(j = 1; j <= row_cnt; j++) */

   ret_code = NCSCC_RC_SUCCESS;
   return ret_code;

}
        
/*===========================================================================
  Function :  miblib_process_removerows
  
  Purpose  :  This function is called when a REMOVEROWS request is
              made.
 
  Input    :  NCSCONTEXT cb - pointer to the subsystem's control block. This 
                will be passed when calls are made to subsystem-provided MIB
                functions
              NCSMIB_ARG* 
              
    
  Returns  :  NCSCC_RC_SUCCESSS / FAILURE
 
  Notes    :  
===========================================================================*/
uns32 miblib_process_removerows(NCSCONTEXT cb, NCSMIB_ARG* args)
{
   NCSMIB_TBL_INFO      *tbl_info = &(miblib_obj_info[args->i_tbl_id]->tbl_info);
   NCSMIB_IDX           idx;
   NCSREMROW_AID        rra;
   uns32                row_cnt, j;
   USRBUF*              usrbuf;
   uns32                ret_code, retval;

   if(((args->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) == NCSMIB_POLICY_PSS_BELIEVE_ME) &&
      (tbl_info->table_of_scalars))
   {
      /* For Scalar MIB table, the "rmvrow" function is required to be supported
         for persistent MIBs. */
      if(tbl_info->rmvrow == NULL)
         return NCSCC_RC_FAILURE;

      ret_code = (tbl_info->rmvrow)(cb, NULL);
      return ret_code;
   }

   /* Why is the usrbuf required in the response???
      To be clarified. TBD. */
   usrbuf = args->req.info.removerows_req.i_usrbuf;
   args->rsp.info.removerows_rsp.i_usrbuf = m_MMGR_DITTO_BUFR(usrbuf);

   /* Find the number of rows encoded into usrbuf */
   row_cnt = ncsremrow_dec_init(&rra, usrbuf);
   if(row_cnt == 0)
   {
       return NCSCC_RC_NO_INSTANCE;
   }

   for(j = 1; j <= row_cnt; j++)
   {
       idx.i_inst_ids = NULL;
       idx.i_inst_len = 0;
       retval = ncsremrow_dec_inst_ids(&rra, &idx, NULL);
       if (retval != NCSCC_RC_SUCCESS)
           return NCSCC_RC_FAILURE;

       /* Call the subsystem's setrow function to do the SETROW processing */
       if(tbl_info->status_field == 0)
       {
          /* SP-MIB without rmvrow handler defined, is illegal. */
          if(tbl_info->rmvrow == NULL)
             return NCSCC_RC_FAILURE;

          ret_code = (tbl_info->rmvrow)(cb, &idx);
       }
       else
       {
           /* Right now, no requirement to support REMOVEROWS for normal
              MIBS having RowStatus object. So, this part of the code
              is left vacant. */
       }

       m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
       idx.i_inst_ids = NULL;
   }    /* for(j = 1; j <= row_cnt; j++) */

   retval = ncsremrow_dec_done(&rra);
   if (retval != NCSCC_RC_SUCCESS)
       return NCSCC_RC_FAILURE;

   ret_code = NCSCC_RC_SUCCESS;
   return ret_code;
}

/*===========================================================================
  Function :  miblib_process_mib_op_req
  
  Purpose  :  The wrapper function that applications can call for 
              processing all MIB requests. This function does the following:

 
  Input    :  NCSCONTEXT cb - pointer to the subsystem's control block. This 
                will be passed when calls are made to subsystem-provided MIB
                functions
              NCSMIB_ARG* 
              
    
  Returns  :  NCSCC_RC_SUCCESSS / FAILURE
 
  Notes    :  
===========================================================================*/
uns32 miblib_process_mib_op_req(NCSCONTEXT cb, NCSMIB_ARG* args)
{
    uns32           req_type = args->i_op; 
    /* Below inttializations of tbl_info & var_info are moved down as a part of fixing the bug 60022 */
    NCSMIB_TBL_INFO* tbl_info;
    NCSMIB_VAR_INFO* var_info;
    NCSMIB_SET_REQ*  i_set_req = &args->req.info.set_req;
    NCSMIB_SET_RSP*  io_set_rsp = &args->rsp.info.set_rsp;
    NCSMIB_TEST_REQ* i_test_req = &args->req.info.test_req;
    NCSMIB_TEST_RSP* io_test_rsp = &args->rsp.info.test_rsp;
    NCSMIB_GET_REQ*  i_get_req = &args->req.info.get_req;
    NCSMIB_GET_RSP*  io_get_rsp = &args->rsp.info.get_rsp;
    NCSMIB_NEXT_REQ* i_next_req  = &args->req.info.next_req;
    NCSMIB_NEXT_RSP* io_next_rsp = &args->rsp.info.next_rsp;
    NCSCONTEXT       data = NULL;
    uns32           ret_code;
    uns32           next_inst_id[NCSMIB_MAX_INST_LEN] = {0};
    uns32           next_inst_id_len = 0;
    uns8            buffer[NCSMIB_MAX_VAR_LEN];  /* Temporary buffer in which
                                                 * the subsystem's extract 
                                                 * function can store the
                                                 * param value 
                                                 */
    NCS_BOOL       hijack_setrow = FALSE; 

    /* Added below check before extracting tbl_info & var_info to fix the bug 60022 */
    if (miblib_obj_info[args->i_tbl_id] == NULL)
    {
        return NCSCC_RC_FAILURE;
    }
    tbl_info = &(miblib_obj_info[args->i_tbl_id]->tbl_info);
    var_info = miblib_obj_info[args->i_tbl_id]->var_info;
    /* Precautionary check to ensure tbl_id is in the valid range */
    if (args->i_tbl_id >= MIB_UD_TBL_ID_END)
    {
        return NCSCC_RC_FAILURE;
    }

    /* make sure the var_info and tbl_info are registered for this table */ 
    if (tbl_info == NULL)
        return NCSCC_RC_FAILURE;
    
    if (var_info == NULL)
        return NCSCC_RC_FAILURE;
    
    if((args->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
    {
        /* Validate the tbl_info & var_info, if the request is not from
           PSSv. */
        if((ret_code = miblib_validate_mib_obj(miblib_obj_info[args->i_tbl_id], 
            args)) 
            != NCSCC_RC_SUCCESS)
        {
            goto CALL_RESP;
        }
    }

    switch(req_type)
    {
    case NCSMIB_OP_REQ_GET:
        io_get_rsp->i_param_val.i_param_id = i_get_req->i_param_id;
        args->i_op = NCSMIB_OP_RSP_GET;

        /* Call the subsystem's get() function */
        if((ret_code = (tbl_info->get)(cb, args, &data)) !=
            NCSCC_RC_SUCCESS)
        {
            break;
        }

        /* Get the object value from the subsystem data-structure */
        ret_code = (tbl_info->extract)(&(io_get_rsp->i_param_val),
                                       &var_info[i_get_req->i_param_id - 1], 
                                       data,
                                       buffer);

        break;

    case NCSMIB_OP_REQ_SET:
        io_set_rsp->i_param_val = i_set_req->i_param_val;
        ret_code = (tbl_info->set)(cb, args, 
                                   &var_info[i_set_req->i_param_val.i_param_id - 1], 
                                   FALSE);
        break;

    case NCSMIB_OP_REQ_TEST:
        io_test_rsp->i_param_val = i_test_req->i_param_val;
        ret_code = (tbl_info->set)(cb, args, 
                                   &var_info[i_test_req->i_param_val.i_param_id - 1],
                                   TRUE);
        break;

    case NCSMIB_OP_REQ_NEXT:
        io_next_rsp->i_param_val.i_param_id = i_next_req->i_param_id;

        /* Call the subsystem's next() function */
        if((ret_code = (tbl_info->next)(cb, args, &data, 
           next_inst_id, &next_inst_id_len)) != NCSCC_RC_SUCCESS)
        {
           /* Set the next inst_id pointer to the array on the stack */
           args->rsp.info.next_rsp.i_next.i_inst_ids = NULL;
           
           /* set the length of the next instance index */
           args->rsp.info.next_rsp.i_next.i_inst_len = 0;
           break;
        }

        /* Set the next inst_id pointer to the array on the stack */
        args->rsp.info.next_rsp.i_next.i_inst_ids = next_inst_id;

        /* set the length of the next instance index */
        args->rsp.info.next_rsp.i_next.i_inst_len = next_inst_id_len;

        
        /* Get the object value from the subsystem data-structure */
        if (data != NULL)
            ret_code = (tbl_info->extract)(&(io_next_rsp->i_param_val),
                                       &var_info[i_next_req->i_param_id - 1], 
                                       data,
                                       buffer);

        break;

    case NCSMIB_OP_REQ_NEXTROW:
        ret_code = miblib_process_getrow_nextrow(cb, args, 
           next_inst_id, &next_inst_id_len);
        /* Set the next inst_id pointer to the array on the stack */
        if(ret_code == NCSCC_RC_SUCCESS) /* Fix for the bug IR00083690 */
        {
           args->rsp.info.nextrow_rsp.i_next.i_inst_ids = next_inst_id;
           args->rsp.info.nextrow_rsp.i_next.i_inst_len = next_inst_id_len;
        }
        break;

    case NCSMIB_OP_REQ_GETROW:
        ret_code = miblib_process_getrow_nextrow(cb, args, 
           next_inst_id, &next_inst_id_len);
        break;
    
    case NCSMIB_OP_REQ_MOVEROW:
        hijack_setrow = TRUE; 
        args->i_op = NCSMIB_OP_REQ_SETROW; 
        args->req.info.setrow_req.i_usrbuf = args->req.info.moverow_req.i_usrbuf;
        /* purposefully avoided the break to fall through */
    case NCSMIB_OP_REQ_SETROW:
    case NCSMIB_OP_REQ_TESTROW:
        ret_code = miblib_process_setrow_testrow(cb, args);
        if (hijack_setrow == TRUE)
            args->i_op = NCSMIB_OP_REQ_MOVEROW;
        break;

    case NCSMIB_OP_REQ_SETALLROWS:
        ret_code = miblib_process_setallrows(cb, args);
        break;

    case NCSMIB_OP_REQ_REMOVEROWS:
        ret_code = miblib_process_removerows(cb, args);
        break;

    default:
        ret_code = NCSCC_RC_FAILURE;
        break;
    }

CALL_RESP:
    args->rsp.i_status = ret_code;
    args->i_op = m_NCSMIB_REQ_TO_RSP(req_type);
    /* Call the response function */
    if((args->i_rsp_fnc(args)) != NCSCC_RC_SUCCESS)
    {
       switch(req_type)
       {
       case NCSMIB_OP_REQ_GETROW:
          m_MMGR_FREE_BUFR_LIST(args->rsp.info.getrow_rsp.i_usrbuf);
          break;
       case NCSMIB_OP_REQ_NEXTROW:
          m_MMGR_FREE_BUFR_LIST(args->rsp.info.nextrow_rsp.i_usrbuf);
          break;
       default:
          break;
       }
    }

    switch(req_type)
    {
    case NCSMIB_OP_REQ_NEXT:
        /* Set the next inst_id pointer to NULL */
        args->rsp.info.next_rsp.i_next.i_inst_ids = NULL;
           
        /* set the length of the next instance to 0 */
        args->rsp.info.next_rsp.i_next.i_inst_len = 0;

    case NCSMIB_OP_REQ_GET:
        io_get_rsp->i_param_val.i_length = 0;
        io_get_rsp->i_param_val.info.i_oct = NULL;
        break;
    case NCSMIB_OP_REQ_NEXTROW: /* Fix for the bug IR00083690 */
        args->rsp.info.nextrow_rsp.i_next.i_inst_ids = NULL; 
        args->rsp.info.nextrow_rsp.i_next.i_inst_len = 0;
        break;
    default:
          break;
    }
        

    return NCSCC_RC_SUCCESS; 
}


/*===========================================================================
  Function :  ncsmiblib_process_req
  
  Purpose  :  The wrapper function that applications can call for 
              processing all MIB requests. This function does the following:

 
  Input    :  NCSCONTEXT cb - pointer to the subsystem's control block. This 
                will be passed when calls are made to subsystem-provided MIB
                functions
              NCSMIB_ARG* 
              
    
  Returns  :  NCSCC_RC_SUCCESSS / FAILURE
 
  Notes    :  
===========================================================================*/
uns32 ncsmiblib_process_req(NCSMIBLIB_REQ_INFO* req_info)
{
    uns32 ret_code = NCSCC_RC_SUCCESS;

    switch(req_info->req)
    {
    case NCSMIBLIB_REQ_REGISTER:
        ret_code = miblib_process_register_req(req_info->info.i_reg_info.obj_info);       
        break;

    case NCSMIBLIB_REQ_MIB_OP:
        ret_code = miblib_process_mib_op_req(req_info->info.i_mib_op_info.cb, 
                                            req_info->info.i_mib_op_info.args);
        break;

    case NCSMIBLIB_REQ_SET_UTIL_OP:
        ret_code = miblib_set_obj_value(req_info->info.i_set_util_info.param,
                                        req_info->info.i_set_util_info.var_info,
                                        req_info->info.i_set_util_info.data,
                                        req_info->info.i_set_util_info.same_value);
        break;

    case NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP:
        ret_code = 
         miblib_validate_row_status(req_info->info.i_val_status_util_info.row_status,
                                    req_info->info.i_val_status_util_info.row);
        break;

    case NCSMIBLIB_REQ_INIT_OP:
        m_NCS_OS_MEMSET(miblib_obj_info, 0, 
               (MIB_UD_TBL_ID_END * sizeof(NCSMIB_OBJ_INFO*)));
        ret_code = NCSCC_RC_SUCCESS;
        break;

    default:
        ret_code = NCSCC_RC_FAILURE;
    }

    return ret_code;
}

/*===========================================================================
  Function :  ncsmiblib_get_max_len_of_discrete_octet_string
  
  Purpose  :  Function to get the maximum length defined for
              the discrete octet strings.
 
  Input    :  NCSMIB_VAR_INFO*
    
  Returns  :  uns32
 
  Notes    :  
===========================================================================*/
uns32 ncsmiblib_get_max_len_of_discrete_octet_string(NCSMIB_VAR_INFO* var_info)
{
  uns32 max_len, i;
  max_len = var_info->obj_spec.disc_stream_spec.values[0].max_len;
  for(i=1; i<var_info->obj_spec.disc_stream_spec.num_values; i++)
     if(var_info->obj_spec.disc_stream_spec.values[i].max_len > max_len)
       max_len = var_info->obj_spec.disc_stream_spec.values[i].max_len;
  return max_len;     
}

