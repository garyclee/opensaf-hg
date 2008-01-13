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



  MODULE NAME: NCSMIBLIB.H

..............................................................................


  DESCRIPTION: Contains MIB LIB data structrues that can be used by
  all NetPlane subsystems.


******************************************************************************
*/

#ifndef NCSMIBLIB_H
#define NCSMIBLIB_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "ncs_mib_pub.h"

typedef enum
{
   NCSMIB_ADMIN_STATUS_ENABLE = 1,
   NCSMIB_ADMIN_STATUS_DISABLE
}NCSMIB_ADMIN_STATUS;


typedef enum
{
   NCSMIB_OPER_STATUS_ENABLE = 1,
   NCSMIB_OPER_STATUS_DISABLE
}NCSMIB_OPER_STATUS;

/* Row status enums - map one-to-one to SMIv2 defined row statuses */
typedef enum 
{
   NCSMIB_ROWSTATUS_ACTIVE = 1,
   NCSMIB_ROWSTATUS_NOTINSERVICE,
   NCSMIB_ROWSTATUS_NOTREADY,
   NCSMIB_ROWSTATUS_CREATE_GO,
   NCSMIB_ROWSTATUS_CREATE_WAIT,
   NCSMIB_ROWSTATUS_DESTROY
}NCSMIB_ROWSTATUS;

/* Syntax type enums. Note: These do not map one to one with SMIv2 defined
 * Syntax types. That's because we need subsystems to provide additional
 * information that will help MIB LIB in doing better param validation
 */
typedef enum
{
   NCSMIB_INT_RANGE_OBJ_TYPE = 1,
   NCSMIB_INT_DISCRETE_OBJ_TYPE,
   NCSMIB_TRUTHVAL_OBJ_TYPE,
   NCSMIB_OCT_OBJ_TYPE,
   NCSMIB_OCT_DISCRETE_OBJ_TYPE,
   NCSMIB_OTHER_INT_OBJ_TYPE,
   NCSMIB_OTHER_OCT_OBJ_TYPE
}NCSMIB_OBJ_SYNTAX_TYPE;

/* Enum for access type - map one-to-one with SMIv2 defined access types */
typedef enum
{
   NCSMIB_ACCESS_NOT_ACCESSIBLE = 1,
   NCSMIB_ACCESS_ACCESSIBLE_FOR_NOTIFY,
   NCSMIB_ACCESS_READ_ONLY,
   NCSMIB_ACCESS_READ_WRITE,
   NCSMIB_ACCESS_READ_CREATE
}NCSMIB_ACCESS_TYPE;

/* Enum for object status type */
typedef enum
{
   NCSMIB_OBJ_CURRENT = 1,
   NCSMIB_OBJ_DEPRECATE,
   NCSMIB_OBJ_OBSOLETE
}NCSMIB_OBJ_STATUS_TYPE;

/* This structure specifies the range of a integer object that
 * has a continuous range of valid values
 */
typedef struct ncsmib_int_obj_range
{
   uns32             min;           /* Min value of the integer object */
   uns32             max;           /* Max value of the integer object */
} NCSMIB_INT_OBJ_RANGE;

/* This structure specifies the valid values for integer object that
 * can take a discrete set of values.
 */
typedef struct ncsmib_int_obj_discrete
{
   uns32                num_values;    /* The number of values */
   NCSMIB_INT_OBJ_RANGE *values;        /* Pointer to an array of integers that 
                                     * represent the values that the integer
                                     * object can take
                                     */
} NCSMIB_INT_OBJ_DISCRETE;

typedef struct ncsmib_truthval_obj_spec
{
   uns32             dummy;    /* Don't need any extra info. for validating
                                * an object of type, TRUTHVAL 
                                */
} NCSMIB_TRUTHVAL_OBJ;

/* This structure specifies the min and max lengths of an OCTET type object */
typedef struct ncsmib_oct_obj
{
   uns32             min_len;        /* Min length of the OCTET type object */
   uns32             max_len;        /* Max length of the OCTET type object */
} NCSMIB_OCT_OBJ;

/* This structure specifies the valid values for octetstring object that
 * can be of discrete set of lengths.
 */
typedef struct ncsmib_oct_obj_discrete
{
   uns32                num_values;    /* The number of values */
   NCSMIB_OCT_OBJ       *values;       /* Pointer to an array of NCSMIB_OCT_OBJ */
} NCSMIB_OCT_OBJ_DISCRETE;

/* This struct will be used in setrow/testrow processing. The received USRBUF
 * is decoded into an array of struct of this type first. Then the setrow/
 * testrow subsystem function will be called.
 */
typedef struct ncsmib_setrow_param_val
{
    NCS_BOOL         set_flag;       /* Specifies whether this param included
                                     * in the bulk set or not 
                                     */
    NCSMIB_PARAM_VAL param;          /* The new value for the param */

} NCSMIB_SETROW_PARAM_VAL;


/* MIB's playback event capability. Used by PSSv to decide on the event
   to be used during playback session. */
typedef enum
{
    NCSMIB_CAPABILITY_SET,
    NCSMIB_CAPABILITY_SETROW,
    NCSMIB_CAPABILITY_SETALLROWS,
    NCSMIB_CAPABILITY_MAX
}NCSMIB_CAPABILITY;

/* The MIB-ranks(range) required for persistency support are defined here.
   The lower the value of the rank, higher the priority given during 
   persistency playback. The default value of the rank is taken to be
   the maximum rank-value, i.e., NCSMIB_TBL_RANK_MAX.
*/ 
#define NCSMIB_TBL_RANK_MIN                     1
#define NCSMIB_TBL_RANK_MAX                     255
#define NCSMIB_TBL_RANK_DEFAULT                 NCSMIB_TBL_RANK_MAX

struct ncsmib_var_info;
/****************************************************************************
 * 
 * The prototype for "validate obj" function that applications need to 
 * provide for SYNTAX types that MIB LIB cannot validate
 *
 * ARGUMENTS :
 *    struct ncsmib_var_info* : The pointer to the var info struct of 
 *                   the parameter
 *    NCSMIB_PARAM_VAL: The param val that was provided by the caller
 *                     within the NCSMIB_ARG struct
 * RETURN VALUE :
 *    uns32        : The status returned by the operation. 
 *
 ****************************************************************************/
typedef uns32 (* NCSMIB_VALIDATE_OBJ) (struct ncsmib_var_info* obj_info, 
                                      NCSMIB_PARAM_VAL* param);

typedef struct ncsmib_var_info
{
  NCSMIB_PARAM_ID    param_id;/* Parameter id of this field */
  uns16             offset;  /* Offset into struct for the parameter */
  uns16             len;     /* This is of significance only if you want to use 
                              * the SET_UTIL_OP. This param is the len of the 
                              * variable. INTEGER value could be stored as uns8, 
                              * uns16 or uns32 by the subsystem. Set len according 
                              * to the data-type used for storage. If you want to 
                              * use SET_UTIL_OP for OCT types, set it to the max 
                              * len of the obj. Also, make sure max_len bytes are 
                              * allocated for the OCT type, in the data-struct
                              */
  NCS_BOOL            is_index_id;   /* When TRUE, implies that parameter is an index field */
  NCSMIB_ACCESS_TYPE access;         /* Specifies whether obj is not-accessible,
                                     * read-only, read-create or read-write
                                     */

  NCSMIB_OBJ_STATUS_TYPE    status;  /* Specifies whether obj is current,
                                     * deprecated or obsolete
                                     */
  NCS_BOOL           set_when_down;  /* When TRUE implies that parameter can be  
                                     * set only when row status is "down" This 
                                     * param will be used only by subsystem
                                     * specific set routines
                                     */
  NCSMIB_FMAT_ID     fmat_id;        /* The format id. as defined in ncs_mib.h */
  NCSMIB_OBJ_SYNTAX_TYPE  obj_type;  /* Should be one of :
                                     *  NCSMIB_INT_RANGE_OBJ_TYPE - value belongs
                                     *   to a continuous range of integers. For
                                     *   example, 1..5
                                     *  NCSMIB_INT_DISCRETE_OBJ_TYPE - obj value 
                                     *   can take a set of integers. For ex.
                                     *   (1,3,5,15,1008)
                                     *  NCSMIB_OCT_OBJ_TYPE - The object is a 
                                     *   stream of octets, 
                                     *  NCSMIB_OTHER_INT_OBJ_TYPE - An integer
                                     *   object type that cannot categorized by 
                                     *   the above INT types
                                     *  NCSMIB_OTHER_OCT_OBJ_TYPE - An integer
                                     *   object type that cannot categorized by 
                                     *   the above OCT types
                                     */ 
  
  /* This union provides details for some checks such as range validation,
   * in case of INTEGERs, and validation of length in case of OCTET objects
   */
  union
  {
     NCSMIB_INT_OBJ_RANGE    range_spec;
     NCSMIB_INT_OBJ_DISCRETE value_spec;
     NCSMIB_TRUTHVAL_OBJ     truthval_spec;
     NCSMIB_OCT_OBJ          stream_spec;
     NCSMIB_OCT_OBJ_DISCRETE disc_stream_spec;
     NCSMIB_VALIDATE_OBJ     validate_obj; /* Pointer to a subsystem provided
                                           * function that validates the param
                                           * value. Needed when syntax type is
                                           * an "OTHER" type.
                                           */
  }  obj_spec;
  
  NCS_BOOL  is_readonly_persistent;   /* For Read-only MIB Objects,
                                        whether Object need be made persistent. */
                        
  int8      *mib_obj_name;  /* name of the object as mentioned in the MIB */ 

} NCSMIB_VAR_INFO;

/* Prototypes for functions that subsystems need to register with MIB LIB */

/****************************************************************************
 * 
 * The prototype for the "set" function that applications need to provide
 * ARGUMENTS :
 *    NCSCONTEXT cb : The pointer to the control block of the subsystem
 *    NCSMIB_ARG*   : The MIB arg that was provided by the caller
 *    NCSMIB_VAR_INFO*   : Pointer to the var_info struct for the parameter
 *    NCS_BOOL test_flag : Set this to TRUE to perform a "test" operation
 *
 * RETURN VALUE :
 *    uns32        : The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 ****************************************************************************/
typedef uns32 (* NCSMIB_SET_FUNC) (NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSMIB_VAR_INFO* var_info,
                                  NCS_BOOL test_flag);

/****************************************************************************
 * 
 * The prototype for the "get" function that applications need to provide
 * ARGUMENTS :
 *    NCSCONTEXT cb : The pointer to the control block of the subsystem
 *    NCSMIB_ARG    : The MIB arg that was provided by the caller
 *    NCSCONTEXT data : The pointer to the data-structure containing the object
 *                     is returned by reference
 * RETURN VALUE :
 *    uns32        : The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context

 *
 ****************************************************************************/
typedef uns32 (* NCSMIB_GET_FUNC) (NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);

/****************************************************************************
 * 
 * The prototype for the "extract" function that applications need to provide.
 * This function is expected to retrieve a certain parameter from the data-
 * structure that contains the param.
 * ARGUMENTS :
 *    NCSMIB_PARAM_VAL : param->i_param_id indicates the parameter to extract
 *                      The remaining elements of the param need to be filled
 *                      by the subystem's extract function
 *    NCSMIB_VAR_INFO* : Pointer to the var_info structure for the param
 *    NCSCONTEXT       : Pointer to the subsystem data-structure that contains 
 *                         the param
 * RETURN VALUE :
 *    uns32        : The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 ****************************************************************************/
typedef uns32 (* NCSMIB_EXTRACT_FUNC) (NCSMIB_PARAM_VAL* param, 
                                      NCSMIB_VAR_INFO* var_info,
                                      NCSCONTEXT data,
                                      NCSCONTEXT buffer);

/****************************************************************************
 * 
 * The prototype for the "next" function that applications need to provide
 * ARGUMENTS :
 *    NCSCONTEXT cb    : The pointer to the control block of the subsystem
 *    NCSMIB_ARG  args : The MIB arg that was provided by the caller
 *              
 * RETURN VALUE :
 *    uns32        : The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *    NCSCONTEXT data : The pointer to the data-structure containing the object
 *                     is returned by reference
 *    uns32* next_inst_id: The next instance id will be filled in this buffer
 *                     and returned by reference.
 *
 * NOTES: 1. The subsystem's next function can assume that sufficient memory 
 *           has been allocated to the next_inst_id
 *           pointer for setting the instance id of the next parameter
 *        2. The subsystem's next function is responsible for setting the 
 *           args->rsp.info.next_rsp.i_next.i_inst_len parameter to the 
 *           length of the instance id of the next parameter
 *          
 ****************************************************************************/
typedef uns32 (* NCSMIB_NEXT_FUNC) (NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                   NCSCONTEXT* data, uns32* next_inst_id,
                                   uns32 *next_inst_id_len);

/* NOTE: The application need not provide getrow() and nextrow() functions. 
 * MIB LIB uses the get() and next() functions to do the getrow()  and 
 * nextrow() operations respectively
 */
struct ncsmib_obj_info;
/****************************************************************************
 * 
 * The prototype for the "setrow" function that applications need to provide
 * ARGUMENTS :
 *    NCSCONTEXT cb : The pointer to the control block of the subsystem
 *    NCSMIB_ARG*   : The MIB arg that was provided by the caller
 *    struct ncsmib_tbl_info* obj_info: Pointer to the OBJ info struct for the 
 *                   table
 *    NCS_BOOL testrow_flag : Set this to TRUE to perform a "testrow" operation
 *
 * RETURN VALUE :
 *    uns32        : The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 ****************************************************************************/
typedef uns32 (* NCSMIB_SETROW_FUNC) (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag);

/****************************************************************************
 * 
 * The prototype for the "removerow" function that applications need to provide
 * ARGUMENTS :
 *    NCSCONTEXT cb : The pointer to the control block of the subsystem
 *    NCSMIB_IDX*   : The MIB arg that was provided by the caller
 *
 * RETURN VALUE :
 *    uns32        : The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 ****************************************************************************/
typedef uns32 (* NCSMIB_REMOVEROW_FUNC) (NCSCONTEXT cb, NCSMIB_IDX *idx);

/****************************************************************************
 * 
 * The prototype for the "validate index" function that applications need to 
 * provide. Note that this function needs to be provided only in the case 
 * where not all indices of the table belong to the same table, i.e there is
 * at least one index that belongs to a different table.
 *
 * ARGUMENTS :
 *    NCSMIB_ARG    : The MIB arg that was provided by the caller
 *
 * RETURN VALUE :
 *    uns32        : SUCCESS/FAILURE
 *
 ****************************************************************************/
typedef uns32 (* NCSMIB_VALIDATE_INDEX) (NCSMIB_ARG *arg);

typedef struct ncsmib_index_param_ids
{
   uns16          num_index_ids;   /* Number of objects that form the index */
   uns16*         index_param_ids; /* Array containing the param ids of the 
                                    * objects that form the index
                                    */
} NCSMIB_INDEX_PARAM_IDS;


/* Table information */
typedef struct ncsmib_tbl_info
{
   uns32          table_id;
   uns8           table_rank;        /* Used by PSR, should be from 0 - 255 */
   NCS_BOOL        is_static;         /* If TRUE, the rows in this table are modified by
                                      * SNMP SET requests only. If FALSE, this information
                                      * is created as a side-effect of run-time operations.
                                      */
   NCS_BOOL        table_of_scalars;  /* Scalars and modeled as tables. Such tables
                                      * need special processing since index has no
                                      * significance in any of the MIB operations.
                                      * Set to TRUE if the table comprises scalars
                                      */
   NCS_BOOL        index_in_this_tbl; /* Set this to true if all the objects that
                                      * constitute the index belong to this tbl.
                                      */   

   NCS_BOOL        is_persistent; /* Used for making the entire MIB persistent, and. 
                                     send Table-bind event to PSS(from OAA).
                                     This flag is used to make all write-able 
                                     MIB Objects persistent. */

   union
   {
      NCSMIB_INDEX_PARAM_IDS index_info;  /* Provide this param if 
                                          * index_in_this_tbl is set to TRUE 
                                          */
      NCSMIB_VALIDATE_INDEX validate_idx; /* Subsystem provided function for 
                                          * validating the index. Provide this
                                          * function if index_in_this_tbl is 
                                          * set to FALSE
                                          */
   } idx_data;
   uns16          num_objects;     /* Number of objects in this table */
   uns16          status_field;    /* Index of the status_field in the NCSMIB_VAR_INFO array */
   uns16          status_offset;   /* ?? */
   uns8           sparse_table;    /* Is this a sparse table?? By Default it is set to FALSE */  
   NCS_BOOL        row_in_one_data_struct; 
                                   /* This should be set to TRUE if the entire 
                                    * row is stored in a single data-structre. 
                                    * The MIB lib will use this info. in the
                                    * getrow operation
                                    */
   NCSMIB_SET_FUNC       set;       /* Will be used for TEST operation too */
   NCSMIB_GET_FUNC       get;       /* This fnc will be used for GETROW op too */
   NCSMIB_NEXT_FUNC      next;      /* This fnc will be used for NEXTROW op too*/
   NCSMIB_SETROW_FUNC    setrow;    /* Will be used for TESTROW operation too */
   NCSMIB_REMOVEROW_FUNC rmvrow;    /* Will be used to delete row for MIB without
                                       ROWSTATUS MIB Object */
   NCSMIB_EXTRACT_FUNC   extract;   /* This function extracts a param from a
                                    * subsystem data-structure 
                                    */
  int8   *mib_module_name;       /* name of the MIB module */ 
  int8   *mib_tbl_name;          /* name of the object as mentioned in the MIB */ 
  uns32  *base_oid;              /* OID of this table */ 
  uns32  base_oid_len;           /* length of the base OID */ 
  NCSMIB_CAPABILITY  capability; /* SET/SETROW/SETALLROWS capability of the service user */  
  int8              table_version;
} NCSMIB_TBL_INFO;

/* MIB objects variable info. and table info */
typedef struct ncsmib_obj_info
{
   NCSMIB_VAR_INFO *var_info;   /* Pointer to an arry of VAR_INFO structs */
   NCSMIB_TBL_INFO tbl_info;
} NCSMIB_OBJ_INFO;

typedef struct ncsmiblib_req_register_info
{
   NCSMIB_OBJ_INFO* obj_info;
} NCSMIBLIB_REQ_REGISTER_INFO;

typedef struct ncsmiblib_req_mib_op_info
{
   NCSCONTEXT  cb;
   NCSMIB_ARG* args;
} NCSMIBLIB_REQ_MIB_OP_INFO;

typedef struct ncsmiblib_req_set_util_info
{
   NCSMIB_PARAM_VAL* param;
   NCSMIB_VAR_INFO*  var_info;
   NCSCONTEXT        data; 
   NCS_BOOL*         same_value;
} NCSMIBLIB_REQ_SET_UTIL_INFO;

typedef struct ncsmiblib_validate_status_util_info
{
   uns32*    row_status;
   NCSCONTEXT row;
} NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_INFO;

typedef struct ncsmiblib_req_init_info
{
   uns32      dummy;
} NCSMIBLIB_REQ_INIT_INFO;

typedef enum ncsmiblib_req_type
{
   NCSMIBLIB_REQ_REGISTER,
   NCSMIBLIB_REQ_MIB_OP,
   NCSMIBLIB_REQ_SET_UTIL_OP,
   NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP,
   NCSMIBLIB_REQ_INIT_OP

} NCSMIBLIB_REQ_TYPE;


typedef struct ncsmiblib_req_info
{
   NCSMIBLIB_REQ_TYPE req;
   union
   {
      NCSMIBLIB_REQ_REGISTER_INFO i_reg_info;    
      NCSMIBLIB_REQ_MIB_OP_INFO   i_mib_op_info;    
      NCSMIBLIB_REQ_SET_UTIL_INFO i_set_util_info;
      NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_INFO i_val_status_util_info;
      NCSMIBLIB_REQ_INIT_INFO     i_init_info;
   }info;
} NCSMIBLIB_REQ_INFO;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            MIB LIB SE-API Function Prototype

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/* Prototype for the function that subsystem code can call to do all MIB 
 * LIB operations
 * The supported request types are : 
 *     NCSMIBLIB_REQ_REGISTER for registering table and var_info's for a table,
 *     NCSMIBLIB_REQ_MIB_OP for performing any MIB operation
 *     NCSMIBLIB_REQ_SET_UTIL_OP for using the MIB LIB set utility
 *     NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP for using the MIB LIB validate
 *                                          row status utility 
 *     NCSMIBLIB_REQ_INIT   to put MIB LIB in a start-state
 *
 * NOTES: 
 * 1. Subsystem specific functions such as locking the appropriate 
 * data-structre are expected to be done in the subsystem MIB routines, 
 * before this function is called
 *
 * 2. Subsystems are expected to use to register their MIB tables 
 * with MIB LIB as part of their initialization procedures.
 *
 * 3. It is assumed that obj_info that is registered, points to a statically
 * allocated structure so that the pointer will be valid even when the
 * subsystem shuts-down. 
 * 
 * 4. In the scenario where subsystems are re-initialized after a shut-down and
 * in the virtual routers scenario, MIB LIB may receive multiple registrations
 * for the same table. The redundant registrations are silently ignored by this
 * MIB LIB function.
 */
EXTERN_C LEAPDLL_API uns32 ncsmiblib_process_req(NCSMIBLIB_REQ_INFO*);


EXTERN_C LEAPDLL_API uns32 ncsmiblib_get_obj_val(NCSMIB_PARAM_VAL *param_val, 
                                    NCSMIB_VAR_INFO *var_info, 
                                    NCSCONTEXT data,
                                    NCSCONTEXT buffer);
EXTERN_C LEAPDLL_API uns32 ncsmiblib_get_max_len_of_discrete_octet_string(NCSMIB_VAR_INFO *var_info);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#ifdef  __cplusplus
}
#endif


#endif
