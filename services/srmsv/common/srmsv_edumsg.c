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

  DESCRIPTION:  This file contains encode & decode SRMSv specific EDU routines 
                
..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

#include "srmsv.h"


/****************************************************************************
                    Static Function Prototypes 
****************************************************************************/
static int srmsv_value_test_type_fnc(NCSCONTEXT );
static int srma_monitor_test_type_fnc(NCSCONTEXT );
static int srma_condition_test_type_fnc(NCSCONTEXT );
static int srma_user_test_type_fnc(NCSCONTEXT );
static int srma_rsrc_test_type_fnc(NCSCONTEXT );
static int srma_msg_test_type_fnc(NCSCONTEXT );
static int srmnd_msg_test_type_fnc(NCSCONTEXT );
static int srma_get_wm_rsrc_test_type_fnc(NCSCONTEXT );
static int srmnd_test_wm_type_fnc(NCSCONTEXT );
static int srmnd_test_wm_exist_fnc(NCSCONTEXT );


static uns32 srmsv_edp_value(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                             EDU_BUF_ENV *,  EDP_OP_TYPE , EDU_ERR *);
static uns32 srmsv_edp_threshold(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                 EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srmsv_edp_leaky_bucket(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                    EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srmsv_edp_rsrc_info(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                 EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srmsv_edp_mon_data(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srmsv_edp_create_rsrc_mon(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                       EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srma_edp_enc_all_rsrc(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                   EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srma_edp_dec_all_rsrc(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                   EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srmsv_edp_agent_dec_bulk_create(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                         EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srmnd_edp_enc_all_rsrc(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                    EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srmnd_edp_dec_all_rsrc(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                    EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srmsv_edp_nd_dec_bulk_created(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                       EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srmnd_edp_enc_all_srma_rsrc(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                         EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srma_edp_enc_all_appl_rsrc(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                        EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);
static uns32 srmsv_edp_get_watermark(EDU_HDL *, EDU_TKN *, NCSCONTEXT , uns32 *,
                                     EDU_BUF_ENV *, EDP_OP_TYPE , EDU_ERR *);

/****************************************************************************
  Name           :  srmsv_value_test_type_fnc
 
  Description    :  This function returns the appropriate next edp offset to 
                    be processed based on the type of the VALUE.
 
  Return Values  :  respective EDP offset value / EDU_EXIT / EDU_FAIL
 
  Notes          :  None.
******************************************************************************/
int srmsv_value_test_type_fnc(NCSCONTEXT arg)
{
   typedef enum {
       LCL_TEST_JUMP_OFFSET_FLOAT = 1,
       LCL_TEST_JUMP_OFFSET_INT32,
       LCL_TEST_JUMP_OFFSET_UNS32,
       LCL_TEST_JUMP_OFFSET_INT64,
       LCL_TEST_JUMP_OFFSET_UNS64
   } LCL_JMP_OFFSET;

   NCS_SRMSV_VAL_TYPE val_type;

   if (arg == NULL)
      return (uns32)EDU_EXIT;
  
   
   val_type = *(NCS_SRMSV_VAL_TYPE *)arg;
   
   /* since value type is zero, there is no value to be encoded. 
      Hence, the .value-encode. edu should EXIT instead of FAIL */
   if(val_type == 0)
     return (uns32)EDU_EXIT;

   switch (val_type)
   {
   case NCS_SRMSV_VAL_TYPE_FLOAT:
       return LCL_TEST_JUMP_OFFSET_FLOAT;

   case NCS_SRMSV_VAL_TYPE_INT32:
       return LCL_TEST_JUMP_OFFSET_INT32;

   case NCS_SRMSV_VAL_TYPE_UNS32:
       return LCL_TEST_JUMP_OFFSET_UNS32;

   case NCS_SRMSV_VAL_TYPE_INT64:
       return LCL_TEST_JUMP_OFFSET_INT64;
 
   case NCS_SRMSV_VAL_TYPE_UNS64:
       return LCL_TEST_JUMP_OFFSET_UNS64;

   default:
       break;
   }

   return (uns32)EDU_FAIL;
}


/****************************************************************************
  Name           :  srmsv_edp_value
 
  Description    :  This is the function which is used to encode decode 
                    NCS_SRMSV_VALUE structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_value(EDU_HDL *edu_hdl,
                      EDU_TKN *edu_tkn,
                      NCSCONTEXT ptr,
                      uns32 *ptr_data_len,
                      EDU_BUF_ENV *buf_env,
                      EDP_OP_TYPE op,
                      EDU_ERR *o_err)
{
   uns32      rc = NCSCC_RC_SUCCESS;
   NCS_SRMSV_VALUE  *struct_ptr = NULL, **d_ptr = NULL;


   EDU_INST_SET srmsv_val_rules[ ] = {
        {EDU_START, srmsv_edp_value, 0, 0, 0, 
                    sizeof(NCS_SRMSV_VALUE), 0, NULL},

        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_VALUE*)0)->val_type, 0, NULL},

        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_VALUE*)0)->scale_type, 0, NULL},

        {EDU_TEST, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_VALUE*)0)->val_type, 0, 
                                srmsv_value_test_type_fnc},

        {EDU_EXEC, ncs_edp_double, 0, 0, EDU_EXIT, 
            (uns32)&((NCS_SRMSV_VALUE*)0)->val.f_val, 0, NULL},

        {EDU_EXEC, ncs_edp_int32, 0, 0, EDU_EXIT, 
            (uns32)&((NCS_SRMSV_VALUE*)0)->val.i_val32, 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, EDU_EXIT, 
            (uns32)&((NCS_SRMSV_VALUE*)0)->val.u_val32, 0, NULL},
        
        {EDU_EXEC, ncs_edp_int64, 0, 0, EDU_EXIT, 
            (uns32)&((NCS_SRMSV_VALUE*)0)->val.i_val64, 0, NULL},

        {EDU_EXEC, ncs_edp_uns64, 0, 0, EDU_EXIT, 
            (uns32)&((NCS_SRMSV_VALUE*)0)->val.u_val64, 0, NULL},
        
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   /* Take appropriate actions based on the operation (enc & dec) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (NCS_SRMSV_VALUE *)ptr;
       break;

   case EDP_OP_TYPE_DEC:
       d_ptr = (NCS_SRMSV_VALUE **)ptr;
       if (*d_ptr == NULL)
       {
          *o_err = EDU_ERR_MEM_FAIL;
          return NCSCC_RC_FAILURE;          
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }

   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srmsv_val_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);
   return rc;
}

                   /***************************************
                                SRMA Specific 
                    **************************************/

/****************************************************************************
  Name           :  srma_monitor_test_type_fnc
 
  Description    :  This function returns the appropriate next edp offset to 
                    be processed based on the type of the monitor.
 
  Return Values  :  respective EDP offset value / EDU_EXIT / EDU_FAIL
 
  Notes          :  None.
******************************************************************************/
int srma_monitor_test_type_fnc(NCSCONTEXT arg)
{    
   typedef enum {
       LCL_TEST_JUMP_OFFSET_THRESHOLD = 1,
       LCL_TEST_JUMP_OFFSET_WATERMARK,
       LCL_TEST_JUMP_OFFSET_LEAKY_BUCKET,
       LCL_TEST_JUMP_OFFSET_EVT_SEVERITY,
       LCL_TEST_JUMP_OFFSET_CONDITION_MAX
   } LCL_JMP_OFFSET;

   NCS_SRMSV_MONITOR_TYPE  mon_type;

   if (arg == NULL)
      return (uns32)EDU_EXIT;
  
   mon_type = *(NCS_SRMSV_MONITOR_TYPE *)arg;
   switch (mon_type)
   {
   case NCS_SRMSV_MON_TYPE_THRESHOLD:
       return LCL_TEST_JUMP_OFFSET_THRESHOLD;

   case NCS_SRMSV_MON_TYPE_WATERMARK:
       return LCL_TEST_JUMP_OFFSET_WATERMARK;

   case NCS_SRMSV_MON_TYPE_LEAKY_BUCKET:
       return LCL_TEST_JUMP_OFFSET_LEAKY_BUCKET;

   default:
       return LCL_TEST_JUMP_OFFSET_EVT_SEVERITY;
   }

   return (uns32)EDU_FAIL;
}


/****************************************************************************
  Name           :  srma_condition_test_type_fnc
 
  Description    :  This function returns the appropriate next edp offset to 
                    be processed based on the type of the threshold condition.
 
  Return Values  :  respective EDP offset value / EDU_EXIT / EDU_FAIL
 
  Notes          :  None.
******************************************************************************/
int srma_condition_test_type_fnc(NCSCONTEXT arg)
{    
   typedef enum {
       LCL_TEST_JUMP_OFFSET_CONDITION = 1,
       LCL_TEST_JUMP_OFFSET_CONDITION_MAX
   } LCL_JMP_OFFSET;

   NCS_SRMSV_THRESHOLD_TEST_CONDITION  condition;

   if (arg == NULL)
      return (uns32)EDU_EXIT;
  
   condition = *(NCS_SRMSV_THRESHOLD_TEST_CONDITION *)arg;
   switch (condition)
   {
   case NCS_SRMSV_THRESHOLD_VAL_IS_AT:
   case NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT:
       return LCL_TEST_JUMP_OFFSET_CONDITION;

   case NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE:
   case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE:
   case NCS_SRMSV_THRESHOLD_VAL_IS_BELOW:
   case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW:
      return (uns32)EDU_EXIT;

   default:
      return (uns32)EDU_EXIT;
   }

   return (uns32)EDU_FAIL;
}


/****************************************************************************
  Name           :  srma_user_test_type_fnc
 
  Description    :  This function returns the appropriate next edp offset to 
                    be processed based on the type of the USER.
 
  Return Values  :  respective EDP offset value / EDU_EXIT / EDU_FAIL
 
  Notes          :  None.
******************************************************************************/
int srma_user_test_type_fnc(NCSCONTEXT arg)
{    
   typedef enum {
       LCL_TEST_JUMP_OFFSET_USER_REQUESTOR = 1,
       LCL_TEST_JUMP_OFFSET_USER_MAX
   } LCL_JMP_OFFSET;

   NCS_SRMSV_USER_TYPE user_type;

   if (arg == NULL)
      return (uns32)EDU_EXIT;
  
   user_type = *(NCS_SRMSV_USER_TYPE *)arg;
   switch (user_type)
   {
   case NCS_SRMSV_USER_REQUESTOR:
   case NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR:
       return LCL_TEST_JUMP_OFFSET_USER_REQUESTOR;

   case NCS_SRMSV_USER_SUBSCRIBER:
      return (uns32)EDU_EXIT;

   default:
       break;
   }

   return (uns32)EDU_FAIL;
}


/****************************************************************************
  Name           :  srma_get_wm_rsrc_test_type_fnc
 
  Description    :  This function returns the appropriate next edp offset to 
                    be processed based on the type of the RESOURCE.
 
  Return Values  :  respective EDP offset value / EDU_EXIT / EDU_FAIL
 
  Notes          :  None.
******************************************************************************/
int srma_get_wm_rsrc_test_type_fnc(NCSCONTEXT arg)
{    
   typedef enum {
       LCL_TEST_JUMP_OFFSET_PROC_RSRC = 1,
       LCL_TEST_JUMP_OFFSET_RSRC_MAX
   } LCL_JMP_OFFSET;

   NCS_SRMSV_RSRC_TYPE rsrc_type;

   if (arg == NULL)
      return (uns32)EDU_EXIT;
  
   rsrc_type = *(NCS_SRMSV_RSRC_TYPE *)arg;
   switch (rsrc_type)
   {
   case NCS_SRMSV_RSRC_PROC_MEM:
   case NCS_SRMSV_RSRC_PROC_CPU:
       return LCL_TEST_JUMP_OFFSET_PROC_RSRC;

   default:
       break;
   }

   return (uns32)EDU_EXIT;  
}


/****************************************************************************
  Name           :  srma_rsrc_test_type_fnc
 
  Description    :  This function returns the appropriate next edp offset to 
                    be processed based on the type of the RESOURCE.
 
  Return Values  :  respective EDP offset value / EDU_EXIT / EDU_FAIL
 
  Notes          :  None.
******************************************************************************/
int srma_rsrc_test_type_fnc(NCSCONTEXT arg)
{    
   typedef enum {
       LCL_TEST_JUMP_OFFSET_PROC_EXIT = 1,
       LCL_TEST_JUMP_OFFSET_PROC_RSRC,
       LCL_TEST_JUMP_OFFSET_RSRC_MAX
   } LCL_JMP_OFFSET;

   NCS_SRMSV_RSRC_TYPE rsrc_type;

   if (arg == NULL)
      return (uns32)EDU_EXIT;
  
   rsrc_type = *(NCS_SRMSV_RSRC_TYPE *)arg;
   switch (rsrc_type)
   {
   case NCS_SRMSV_RSRC_PROC_EXIT:
       return LCL_TEST_JUMP_OFFSET_PROC_EXIT;

   case NCS_SRMSV_RSRC_PROC_MEM:
   case NCS_SRMSV_RSRC_PROC_CPU:
       return LCL_TEST_JUMP_OFFSET_PROC_RSRC;

   case NCS_SRMSV_RSRC_CPU_UTIL:
   case NCS_SRMSV_RSRC_CPU_KERNEL:
   case NCS_SRMSV_RSRC_CPU_USER:
   case NCS_SRMSV_RSRC_CPU_UTIL_ONE:
   case NCS_SRMSV_RSRC_CPU_UTIL_FIVE:
   case NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN:
   case NCS_SRMSV_RSRC_MEM_PHYSICAL_USED:
   case NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE:
   case NCS_SRMSV_RSRC_SWAP_SPACE_USED:
   case NCS_SRMSV_RSRC_SWAP_SPACE_FREE:
   case NCS_SRMSV_RSRC_USED_BUFFER_MEM:
   case NCS_SRMSV_RSRC_USED_CACHED_MEM:
       return (uns32)EDU_EXIT;  

   default:
       break;
   }

   return (uns32)EDU_FAIL;
}


/****************************************************************************
  Name           :  srmsv_edp_threshold
 
  Description    :  This is the function which is used to encode decode 
                    SRMA_CREATE_RSRC_MON structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_threshold(EDU_HDL *edu_hdl,
                          EDU_TKN *edu_tkn,
                          NCSCONTEXT ptr,
                          uns32 *ptr_data_len,
                          EDU_BUF_ENV *buf_env,
                          EDP_OP_TYPE op,
                          EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    NCS_SRMSV_THRESHOLD_CFG  *struct_ptr = NULL, **d_ptr = NULL;


    EDU_INST_SET srmnd_threshold_rules[ ] = {
        {EDU_START, srmsv_edp_threshold, 0, 0, 0, 
                    sizeof(NCS_SRMSV_THRESHOLD_CFG), 0, NULL},
    
        /* resource statistics samples to be consider for threshold level checks */
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                   (uns32)&((NCS_SRMSV_THRESHOLD_CFG*)0)->samples, 0, NULL},

        /* Threshold level value */
        {EDU_EXEC, srmsv_edp_value, 0, 0, 0, 
                   (uns32)&((NCS_SRMSV_THRESHOLD_CFG*)0)->threshold_val, 0, NULL},

        /* Condition applied on threshold conditions */
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                   (uns32)&((NCS_SRMSV_THRESHOLD_CFG*)0)->condition, 0, NULL},

        /* Test Condition on threshold level condition defined */
        {EDU_TEST, ncs_edp_int, 0, 0, 0, 
                   (uns32)&((NCS_SRMSV_THRESHOLD_CFG*)0)->condition, 0,
                   srma_condition_test_type_fnc},
        
        /* Tolerable value to be consider in case of above condition is 
           AT or NOT_AT */
        {EDU_EXEC, srmsv_edp_value, 0, 0, 0, 
                   (uns32)&((NCS_SRMSV_THRESHOLD_CFG*)0)->tolerable_val, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Take appropriate actions based on the operation (enc & dec) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (NCS_SRMSV_THRESHOLD_CFG *)ptr;
       break;

   case EDP_OP_TYPE_DEC:
       d_ptr = (NCS_SRMSV_THRESHOLD_CFG **)ptr;
       if (*d_ptr == NULL)
       {
          *o_err = EDU_ERR_MEM_FAIL;
          return NCSCC_RC_FAILURE;          
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }

   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl, 
                            edu_tkn,
                            srmnd_threshold_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);
   return rc;
}


/****************************************************************************
  Name           :  srmsv_edp_leaky_bucket
 
  Description    :  This is the function which is used to encode decode 
                    NCS_SRMSV_LEAKY_BUCKET_CFG structure.
 
   Return Values :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_leaky_bucket(EDU_HDL *edu_hdl,
                             EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr,
                             uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env,
                             EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    NCS_SRMSV_LEAKY_BUCKET_CFG  *struct_ptr = NULL, **d_ptr = NULL;


    EDU_INST_SET srmnd_leaky_bucket_rules[ ] = {
        {EDU_START, srmsv_edp_leaky_bucket, 0, 0, 0, 
                    sizeof(NCS_SRMSV_LEAKY_BUCKET_CFG), 0, NULL},
        
        {EDU_EXEC, srmsv_edp_value, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_LEAKY_BUCKET_CFG*)0)->bucket_size, 0, NULL},

        {EDU_EXEC, srmsv_edp_value, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_LEAKY_BUCKET_CFG*)0)->fill_size, 0, NULL},
        
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Take appropriate actions based on the operation (enc & dec) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (NCS_SRMSV_LEAKY_BUCKET_CFG *)ptr;
       break;

   case EDP_OP_TYPE_DEC:
       d_ptr = (NCS_SRMSV_LEAKY_BUCKET_CFG **)ptr;
       if (*d_ptr == NULL)
       {
          *o_err = EDU_ERR_MEM_FAIL;
          return NCSCC_RC_FAILURE;          
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }

   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl, 
                            edu_tkn,
                            srmnd_leaky_bucket_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);
   return rc;
}


/****************************************************************************
  Name           :  srmsv_edp_rsrc_info
 
  Description    :  This is the function which is used to encode decode 
                    NCS_SRMSV_RSRC_INFO structure.
 
   Return Values :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_rsrc_info(EDU_HDL *edu_hdl,
                          EDU_TKN *edu_tkn,
                          NCSCONTEXT ptr,
                          uns32 *ptr_data_len,
                          EDU_BUF_ENV *buf_env,
                          EDP_OP_TYPE op,
                          EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    NCS_SRMSV_RSRC_INFO  *struct_ptr = NULL, **d_ptr = NULL;


    EDU_INST_SET srmnd_rsrc_info_rules[ ] = {
        {EDU_START, srmsv_edp_rsrc_info, 0, 0, 0, 
                    sizeof(NCS_SRMSV_RSRC_INFO), 0, NULL},
        
        /* Resource type */
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_RSRC_INFO*)0)->rsrc_type, 0, NULL},

        /* User type */
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_RSRC_INFO*)0)->usr_type, 0, NULL},

        /* Test Condition on rsrc-type */
        {EDU_TEST, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_RSRC_INFO*)0)->rsrc_type, 0,
                    srma_rsrc_test_type_fnc},

        /* child level for process specific resource type */
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_RSRC_INFO*)0)->child_level, 0, NULL},

        /* PID for process specific resource type */
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_RSRC_INFO*)0)->pid, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Take appropriate actions based on the operation (encode & decode) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (NCS_SRMSV_RSRC_INFO *)ptr;
       break;

   case EDP_OP_TYPE_DEC:
       d_ptr = (NCS_SRMSV_RSRC_INFO **)ptr;
       if (*d_ptr == NULL)
       {
          *o_err = EDU_ERR_MEM_FAIL;
          return NCSCC_RC_FAILURE;          
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srmnd_rsrc_info_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);
   return rc;
}


/****************************************************************************
  Name          :  srmsv_edp_mon_data
 
  Description   :  This is the function which is used to encode decode 
                   SRMA_CREATE_RSRC_MON structure.
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmsv_edp_mon_data(EDU_HDL *edu_hdl,
                         EDU_TKN *edu_tkn,
                         NCSCONTEXT ptr,
                         uns32 *ptr_data_len,
                         EDU_BUF_ENV *buf_env,
                         EDP_OP_TYPE op,
                         EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    NCS_SRMSV_MON_DATA  *struct_ptr = NULL, **d_ptr = NULL;


    EDU_INST_SET srmnd_mon_data_rules[] = {
        {EDU_START, srmsv_edp_mon_data, 0, 0, 0, 
                    sizeof(NCS_SRMSV_MON_DATA), 0, NULL},
        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_MON_DATA*)0)->monitoring_rate, 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_MON_DATA*)0)->monitoring_interval, 0, NULL},

        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_MON_DATA*)0)->monitor_type, 0, NULL},

        {EDU_TEST, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_MON_DATA*)0)->monitor_type, 0,
                    srma_monitor_test_type_fnc},
        
        {EDU_EXEC, srmsv_edp_threshold, 0, 0, 8, 
                    (uns32)&((NCS_SRMSV_MON_DATA*)0)->mon_cfg.threshold, 0, NULL},
        
        {EDU_EXEC, ncs_edp_int, 0, 0, 8, 
                    (uns32)&((NCS_SRMSV_MON_DATA*)0)->mon_cfg.watermark_type, 0, NULL},

        {EDU_EXEC, srmsv_edp_leaky_bucket, 0, 0, 8, 
                    (uns32)&((NCS_SRMSV_MON_DATA*)0)->mon_cfg.leaky_bucket, 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_MON_DATA*)0)->evt_severity, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Take appropriate actions based on the operation (enc & dec) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (NCS_SRMSV_MON_DATA *)ptr;
       break;

   case EDP_OP_TYPE_DEC:
       d_ptr = (NCS_SRMSV_MON_DATA **)ptr;
       if (*d_ptr == NULL)
       {
          *o_err = EDU_ERR_MEM_FAIL;
          return NCSCC_RC_FAILURE;          
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srmnd_mon_data_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);   
   return rc;
}


/****************************************************************************
  Name           :  srmsv_edp_get_watermark
 
  Description    :  This is the function which is used to encode decode 
                    SRMA_GET_WATERMARK structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_get_watermark(EDU_HDL *edu_hdl,
                              EDU_TKN *edu_tkn, 
                              NCSCONTEXT ptr,
                              uns32 *ptr_data_len, 
                              EDU_BUF_ENV *buf_env,
                              EDP_OP_TYPE op, 
                              EDU_ERR *o_err)
{
   uns32  rc = NCSCC_RC_SUCCESS;
   SRMA_GET_WATERMARK *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET srma_get_wm_rules[ ] = {
        {EDU_START, srmsv_edp_get_watermark, 0, 0, 0, 
                    sizeof(SRMA_GET_WATERMARK), 0, NULL},

        /* Resource type */
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMA_GET_WATERMARK*)0)->rsrc_type, 0, NULL},

        /* Watermark type */
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMA_GET_WATERMARK*)0)->wm_type, 0, NULL},

        /* Test Condition on rsrc-type */
        {EDU_TEST, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMA_GET_WATERMARK*)0)->rsrc_type, 0,
                    srma_get_wm_rsrc_test_type_fnc},

        /* PID for process specific resource type */
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((NCS_SRMSV_RSRC_INFO*)0)->pid, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   /* Take appropriate actions based on the operation (enc & dec) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (SRMA_GET_WATERMARK *)ptr;
       break;

   case EDP_OP_TYPE_DEC:
       d_ptr = (SRMA_GET_WATERMARK **)ptr;
       if (*d_ptr == NULL)
       {
          *o_err = EDU_ERR_MEM_FAIL;
          return NCSCC_RC_FAILURE;          
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srma_get_wm_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);
   return rc;
}


/****************************************************************************
  Name           :  srmsv_edp_create_rsrc_mon
 
  Description    :  This is the function which is used to encode decode 
                    SRMA_CREATE_RSRC_MON structure.
 
   Return Values :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_create_rsrc_mon(EDU_HDL *edu_hdl,
                                EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr,
                                uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env,
                                EDP_OP_TYPE op, 
                                EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    SRMA_CREATE_RSRC_MON  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET srma_create_rsrc_rules[ ] = {
        {EDU_START, srmsv_edp_create_rsrc_mon, 0, 0, 0, 
                    sizeof(SRMA_CREATE_RSRC_MON), 0, NULL},

        /* Resource specific SRMA handle */
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((SRMA_CREATE_RSRC_MON*)0)->rsrc_hdl, 0, NULL},

        /* EDP procedure for rsrc_info */
        {EDU_EXEC, srmsv_edp_rsrc_info, 0, 0, 0, 
                    (uns32)&((SRMA_CREATE_RSRC_MON*)0)->mon_info.rsrc_info, 0, NULL},

        /* Test Condition on user type */
        {EDU_TEST, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMA_CREATE_RSRC_MON*)0)->mon_info.rsrc_info.usr_type, 0,
                    srma_user_test_type_fnc},

        /* ED procedure for mon-data */
        {EDU_EXEC, srmsv_edp_mon_data, 0, 0, 0, 
                    (uns32)&((SRMA_CREATE_RSRC_MON*)0)->mon_info.monitor_data, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Take appropriate actions based on the operation (enc & dec) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (SRMA_CREATE_RSRC_MON *)ptr;
       break;

   case EDP_OP_TYPE_DEC:
       d_ptr = (SRMA_CREATE_RSRC_MON **)ptr;
       if (*d_ptr == NULL)
       {
          *o_err = EDU_ERR_MEM_FAIL;
          return NCSCC_RC_FAILURE;          
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srma_create_rsrc_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);
   return rc;
}


/****************************************************************************
  Name           :  srma_edp_enc_all_rsrc
 
  Description    :  This is the function which is used to encode decode 
                    SRMA_RSRC_MON structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srma_edp_enc_all_rsrc(EDU_HDL *edu_hdl,
                            EDU_TKN *edu_tkn,
                            NCSCONTEXT ptr,
                            uns32 *ptr_data_len,
                            EDU_BUF_ENV *buf_env,
                            EDP_OP_TYPE op,
                            EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    SRMA_RSRC_MON  *struct_ptr = NULL;


    /* EDP rules defined for encoding bulk rsrc created */
    EDU_INST_SET srma_enc_all_rsrc_rules[ ] = {
        {EDU_START, srma_edp_enc_all_rsrc, EDQ_LNKLIST, 0, 0, 
                    sizeof(SRMA_RSRC_MON), 0, NULL},
        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                   (uns32)&((SRMA_RSRC_MON*)0)->rsrc_mon_hdl, 0, NULL},     
                    
        {EDU_EXEC, srmsv_edp_rsrc_info, 0, 0, 0, 
                   (uns32)&((SRMA_RSRC_MON*)0)->rsrc_info, 0, NULL},

        {EDU_EXEC, srmsv_edp_mon_data, 0, 0, 0, 
                   (uns32)&((SRMA_RSRC_MON*)0)->monitor_data, 0, NULL},
        
        {EDU_TEST_LL_PTR, srma_edp_enc_all_rsrc, 0, 0, 0, 
                (uns32)&((SRMA_RSRC_MON*)0)->next_srmnd_rsrc, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Always encoding only */
   struct_ptr = (SRMA_RSRC_MON *)ptr;   
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srma_enc_all_rsrc_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);   
   return rc;
}


/****************************************************************************
  Name           :  srma_edp_dec_all_rsrc
 
  Description    :  This is the function which is used to encode decode 
                    SRMA_CREATE_RSRC_MON_NODE structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srma_edp_dec_all_rsrc(EDU_HDL *edu_hdl,
                            EDU_TKN *edu_tkn,
                            NCSCONTEXT ptr,
                            uns32 *ptr_data_len,
                            EDU_BUF_ENV *buf_env,
                            EDP_OP_TYPE op,
                            EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    SRMA_CREATE_RSRC_MON_NODE  *struct_ptr = NULL, **d_ptr = NULL;


    /* EDP rules defined for encoding bulk rsrc created */
    EDU_INST_SET srmnd_dec_all_rsrc_rules[ ] = {
        {EDU_START, srmnd_edp_dec_all_rsrc, EDQ_LNKLIST, 0, 0, 
                    sizeof(SRMA_CREATE_RSRC_MON_NODE), 0, NULL},        
                    
        {EDU_EXEC, srmsv_edp_create_rsrc_mon, 0, 0, 0, 
                   (uns32)&((SRMA_CREATE_RSRC_MON_NODE*)0)->rsrc, 0, NULL},

        {EDU_TEST_LL_PTR, srmnd_edp_dec_all_rsrc, 0, 0, 0, 
                (uns32)&((SRMA_CREATE_RSRC_MON_NODE*)0)->next_rsrc_mon, 0, NULL},
                    
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Always decoding only */
   d_ptr = (SRMA_CREATE_RSRC_MON_NODE **)ptr;
   if (*d_ptr == NULL)
   {
      *d_ptr = m_MMGR_ALLOC_SRMA_CREATE_RSRC;
      if (*d_ptr == NULL)
      {
         *o_err = EDU_ERR_MEM_FAIL;
         return NCSCC_RC_FAILURE;
      }

      m_NCS_MEMSET(*d_ptr, '\0', sizeof(SRMA_CREATE_RSRC_MON_NODE));      
   }
   
   struct_ptr = *d_ptr;
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srmnd_dec_all_rsrc_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);   
   return rc;
}


/****************************************************************************
  Name           :  srma_edp_enc_all_appl_rsrc
 
  Description    :  This is the function which is used to encode decode 
                    SRMND_MON_SRMA_USR_NODE structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srma_edp_enc_all_appl_rsrc(EDU_HDL *edu_hdl,
                                 EDU_TKN *edu_tkn,
                                 NCSCONTEXT ptr,
                                 uns32 *ptr_data_len,
                                 EDU_BUF_ENV *buf_env,
                                 EDP_OP_TYPE op,
                                 EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    SRMA_SRMND_USR_NODE  *struct_ptr = NULL;


    /* EDP rules defined for encoding bulk rsrc created */
    EDU_INST_SET srma_appl_enc_all_rsrc_rules[ ] = {
        {EDU_START, srma_edp_enc_all_appl_rsrc, 0, 0, 0, 
                    sizeof(SRMA_SRMND_USR_NODE), 0, NULL},
        
        {EDU_EXEC, srma_edp_enc_all_rsrc, EDQ_POINTER, 0, 0, 
                   (uns32)&((SRMA_SRMND_USR_NODE*)0)->start_rsrc_mon, 0, NULL},     
                            
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Always encoding only */
   struct_ptr = (SRMA_SRMND_USR_NODE *)ptr;   
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srma_appl_enc_all_rsrc_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);   
   return rc;
}


/****************************************************************************
  Name           :  srmsv_edp_agent_enc_bulk_create
 
  Description    :  This is the function which is used to encode decode 
                    SRMA_MSG structure.
 
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_agent_enc_bulk_create(EDU_HDL *edu_hdl,
                                      EDU_TKN *edu_tkn,
                                      NCSCONTEXT ptr,
                                      uns32 *ptr_data_len,
                                      EDU_BUF_ENV *buf_env,
                                      EDP_OP_TYPE op,
                                      EDU_ERR *o_err)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SRMA_MSG *struct_ptr = NULL;

   /*************************************************************
          EDP rules defined for ENCODING bulk rsrc created 
   *************************************************************/
   EDU_INST_SET srma_bulk_create_enc_rules[ ] = {
        {EDU_START, srmsv_edp_agent_enc_bulk_create, 0, 0, 0, 
                    sizeof(SRMA_MSG), 0, NULL},
        
        /* Event message type */
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMA_MSG*)0)->msg_type, 0, NULL},

        /* Boolean to represents whether it is a broadcast message */
        {EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0, 
                    (uns32)&((SRMA_MSG*)0)->bcast, 0, NULL},

        /* User specific SRMA handle */
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((SRMA_MSG*)0)->handle, 0, NULL},

        {EDU_EXEC, srma_edp_enc_all_appl_rsrc, EDQ_POINTER, 0, 0, 
                    (uns32)&((SRMA_MSG*)0)->info.bulk_create_mon.srmnd_usr_rsrc, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   /* Take appropriate actions based on the operation (ENC & DEC) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (SRMA_MSG *)ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }
    
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srma_bulk_create_enc_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);

   return rc;
}


/****************************************************************************
  Name           :  srmsv_edp_agent_dec_bulk_create
 
  Description    :  This is the function which is used to encode decode 
                    SRMA_BULK_CREATE_RSRC_MON structure.
 
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_agent_dec_bulk_create(EDU_HDL *edu_hdl,
                                  EDU_TKN *edu_tkn,
                                  NCSCONTEXT ptr,
                                  uns32 *ptr_data_len,
                                  EDU_BUF_ENV *buf_env,
                                  EDP_OP_TYPE op,
                                  EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    SRMA_BULK_CREATE_RSRC_MON *struct_ptr = NULL, **d_ptr = NULL;

    /*************************************************************
       EDP rules defined for DECODING bulk rsrc created     
    *************************************************************/
    EDU_INST_SET srma_bulk_create_dec_rules[ ] = {
        {EDU_START, srmsv_edp_agent_dec_bulk_create, 0, 0, 0, 
                    sizeof(SRMA_BULK_CREATE_RSRC_MON), 0, NULL},
        
        {EDU_EXEC, srma_edp_dec_all_rsrc, EDQ_POINTER, 0, 0, 
                   (uns32)&((SRMA_BULK_CREATE_RSRC_MON*)0)->bulk_rsrc, 0, NULL},        

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Take appropriate actions based on the operation (ENC & DEC) type */
   switch (op)
   {
   case EDP_OP_TYPE_DEC:
       d_ptr = (SRMA_BULK_CREATE_RSRC_MON **)ptr;
       if (*d_ptr == NULL)
       {
          *o_err = EDU_ERR_MEM_FAIL;
          return NCSCC_RC_FAILURE;          
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }
    
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srma_bulk_create_dec_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);

   return rc;
}

                   /***************************************
                                SRMND Specific 
                    **************************************/

/****************************************************************************
  Name           :  srma_msg_test_type_fnc
 
  Description    :  This function returns the appropriate next edp offset to 
                    be processed based on the type of the SRMA message.
 
  Return Values  : respective EDP offset value / EDU_EXIT / EDU_FAIL
 
  Notes          :  None.
******************************************************************************/
int srma_msg_test_type_fnc(NCSCONTEXT arg)
{    
   typedef enum {
       SRMA_LCL_TEST_JUMP_OFFSET_CREATE_RSRC_MON = 1,
       SRMA_LCL_TEST_JUMP_OFFSET_DELETE_RSRC_MON,
       SRMA_LCL_TEST_JUMP_OFFSET_BULK_CREATE_RSRC_MON,
       SRMA_LCL_TEST_JUMP_OFFSET_GET_WATERMARK_MON, 
       SRMA_LCL_TEST_JUMP_OFFSET_MAX
   } LCL_JMP_OFFSET;

   SRMA_MSG_TYPE msg_type;

   if (arg == NULL)
      return (uns32)EDU_EXIT;
  
   msg_type = *(SRMA_MSG_TYPE *)arg;
   switch (msg_type)
   {
   case SRMA_UNREGISTER_MSG:
   case SRMA_START_MON_MSG:
   case SRMA_STOP_MON_MSG:
       return (uns32)EDU_EXIT;
       
   case SRMA_CREATE_RSRC_MON_MSG:
   case SRMA_MODIFY_RSRC_MON_MSG:
       return SRMA_LCL_TEST_JUMP_OFFSET_CREATE_RSRC_MON;

   case SRMA_DEL_RSRC_MON_MSG:
       return SRMA_LCL_TEST_JUMP_OFFSET_DELETE_RSRC_MON;

   case SRMA_BULK_CREATE_MON_MSG:
       return SRMA_LCL_TEST_JUMP_OFFSET_BULK_CREATE_RSRC_MON;

   case SRMA_GET_WATERMARK_MSG:
       return SRMA_LCL_TEST_JUMP_OFFSET_GET_WATERMARK_MON;
      
   default:
       break;
   }

   return (uns32)EDU_FAIL;
}


/****************************************************************************
  Name           :  srmnd_msg_test_type_fnc
 
  Description    :  This function returns the appropriate next edp offset to 
                    be processed based on the type of the SRMND message.
 
  Return Values  :  respective EDP offset value / EDU_EXIT / EDU_FAIL
 
  Notes          :  None.
******************************************************************************/
int srmnd_msg_test_type_fnc(NCSCONTEXT arg)
{    
   typedef enum {
       LCL_TEST_JUMP_OFFSET_CREATED = 1,
       LCL_TEST_JUMP_OFFSET_NOTIF = 3,
       LCL_TEST_JUMP_OFFSET_BULK_CREATED = 5,
       LCL_TEST_JUMP_OFFSET_PROC_EXIT,
       LCL_TEST_JUMP_OFFSET_WM_EXIST = 6,
       LCL_TEST_JUMP_OFFSET_WATERMARK,
       LCL_TEST_JUMP_OFFSET_MAX
   } LCL_JMP_OFFSET;

   SRMND_MSG_TYPE msg_type;

   if (arg == NULL)
      return (uns32)EDU_EXIT;
  
   msg_type = *(SRMND_MSG_TYPE *)arg;
   switch (msg_type)
   {
   case SRMND_CREATED_MON_MSG:
       return LCL_TEST_JUMP_OFFSET_CREATED;

   case SRMND_APPL_NOTIF_MSG:
       return LCL_TEST_JUMP_OFFSET_NOTIF;

   case SRMND_BULK_CREATED_MON_MSG:
       return LCL_TEST_JUMP_OFFSET_BULK_CREATED;

   case SRMND_WATERMARK_VAL_MSG:
       return LCL_TEST_JUMP_OFFSET_WATERMARK;

   case SRMND_SYNCHRONIZED_MSG:
       return (uns32)EDU_EXIT;

   case SRMND_PROC_EXIT_MSG:
       return LCL_TEST_JUMP_OFFSET_PROC_EXIT;

   case SRMND_WATERMARK_EXIST_MSG:
       return LCL_TEST_JUMP_OFFSET_WM_EXIST;

   default:
       break;
   }

   return (uns32)EDU_FAIL;
}


/****************************************************************************
  Name           :  srmnd_test_wm_exist_fnc
 
  Description    :  This function returns the appropriate next edp offset to 
                    be processed based on the exist of the watermark.
 
  Return Values  :  respective EDP offset value / EDU_EXIT / EDU_FAIL
 
  Notes          :  None.
******************************************************************************/
int srmnd_test_wm_exist_fnc(NCSCONTEXT arg)
{    
   typedef enum {
       LCL_TEST_JUMP_OFFSET_WATERMARK_EXIST = 1,
   } LCL_JMP_OFFSET;

   NCS_BOOL wm_exist;

   if (arg == NULL)
      return (uns32)EDU_EXIT;
  
   wm_exist = *(NCS_BOOL*)arg;
   switch (wm_exist)
   {
   case TRUE:
       return LCL_TEST_JUMP_OFFSET_WATERMARK_EXIST;

   default:
      return (uns32)EDU_EXIT;
   }

   return (uns32)EDU_FAIL;
}


/****************************************************************************
  Name           :  srmnd_test_wm_type_fnc
 
  Description    :  This function returns the appropriate next edp offset to 
                    be processed based on the type of the watermark.
 
  Return Values  :  respective EDP offset value / EDU_EXIT / EDU_FAIL
 
  Notes          :  None.
******************************************************************************/
int srmnd_test_wm_type_fnc(NCSCONTEXT arg)
{    
   typedef enum {
       LCL_TEST_JUMP_OFFSET_DUAL_WATERMARK = 1,
       LCL_TEST_JUMP_OFFSET_HIGH_WATERMARK,
       LCL_TEST_JUMP_OFFSET_LOW_WATERMARK,
   } LCL_JMP_OFFSET;

   NCS_SRMSV_WATERMARK_TYPE wm_type;

   if (arg == NULL)
      return (uns32)EDU_EXIT;
  
   wm_type = *(NCS_SRMSV_WATERMARK_TYPE*)arg;
   switch (wm_type)
   {
   case NCS_SRMSV_WATERMARK_DUAL:
       return LCL_TEST_JUMP_OFFSET_DUAL_WATERMARK;

   case NCS_SRMSV_WATERMARK_HIGH:
       return LCL_TEST_JUMP_OFFSET_HIGH_WATERMARK;

   case NCS_SRMSV_WATERMARK_LOW:
       return LCL_TEST_JUMP_OFFSET_LOW_WATERMARK;

   default:
      return (uns32)EDU_EXIT;
   }

   return (uns32)EDU_FAIL;
}


/****************************************************************************
  Name           :  srmnd_edp_enc_all_rsrc
 
  Description    :  This is the function which is used to encode decode 
                    SRMND_RSRC_MON_NODE structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmnd_edp_enc_all_rsrc(EDU_HDL *edu_hdl,
                             EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr,
                             uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env,
                             EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    SRMND_RSRC_MON_NODE  *struct_ptr = NULL;


    /* EDP rules defined for encoding bulk rsrc created */
    EDU_INST_SET srmnd_enc_all_rsrc_rules[ ] = {
        {EDU_START, srmnd_edp_enc_all_rsrc, EDQ_LNKLIST, 0, 0, 
                    sizeof(SRMND_RSRC_MON_NODE), 0, NULL},
        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((SRMND_RSRC_MON_NODE*)0)->srma_rsrc_hdl, 0, NULL},     

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((SRMND_RSRC_MON_NODE*)0)->rsrc_mon_hdl, 0, NULL},

        {EDU_TEST_LL_PTR, srmnd_edp_enc_all_rsrc, 0, 0, 0, 
                (uns32)&((SRMND_RSRC_MON_NODE*)0)->next_srma_usr_rsrc, 0, NULL},
                    
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Always encoding only */
   struct_ptr = (SRMND_RSRC_MON_NODE *)ptr;   
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srmnd_enc_all_rsrc_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);   
   return rc;
}


/****************************************************************************
  Name           :  srmnd_edp_dec_all_rsrc
 
  Description    :  This is the function which is used to encode decode 
                    SRMND_CREATED_RSRC_MON structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmnd_edp_dec_all_rsrc(EDU_HDL *edu_hdl,
                             EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr,
                             uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env,
                             EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    SRMND_CREATED_RSRC_MON  *struct_ptr = NULL, **d_ptr = NULL;


    /* EDP rules defined for encoding bulk rsrc created */
    EDU_INST_SET srmnd_dec_all_rsrc_rules[ ] = {
        {EDU_START, srmnd_edp_dec_all_rsrc, EDQ_LNKLIST, 0, 0, 
                    sizeof(SRMND_CREATED_RSRC_MON), 0, NULL},
        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((SRMND_CREATED_RSRC_MON*)0)->srma_rsrc_hdl, 0, NULL},      

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((SRMND_CREATED_RSRC_MON*)0)->srmnd_rsrc_hdl, 0, NULL},

        {EDU_TEST_LL_PTR, srmnd_edp_dec_all_rsrc, 0, 0, 0, 
                (uns32)&((SRMND_CREATED_RSRC_MON*)0)->next_srmnd_rsrc_mon, 0, NULL},
                    
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Always decoding only */
   d_ptr = (SRMND_CREATED_RSRC_MON **)ptr;
   if (*d_ptr == NULL)
   {
      *d_ptr = m_MMGR_ALLOC_SRMND_CREATED_RSRC;
      if (*d_ptr == NULL)
      {
         *o_err = EDU_ERR_MEM_FAIL;
         return NCSCC_RC_FAILURE;
      }

      m_NCS_MEMSET(*d_ptr, '\0', sizeof(SRMND_CREATED_RSRC_MON));      
   }
   
   struct_ptr = *d_ptr;
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srmnd_dec_all_rsrc_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);   
   return rc;
}


/****************************************************************************
  Name           :  srmnd_edp_enc_all_srma_rsrc
 
  Description    :  This is the function which is used to encode decode 
                    SRMND_MON_SRMA_USR_NODE structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmnd_edp_enc_all_srma_rsrc(EDU_HDL *edu_hdl,
                                  EDU_TKN *edu_tkn,
                                  NCSCONTEXT ptr,
                                  uns32 *ptr_data_len,
                                  EDU_BUF_ENV *buf_env,
                                  EDP_OP_TYPE op,
                                  EDU_ERR *o_err)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    SRMND_MON_SRMA_USR_NODE  *struct_ptr = NULL;


    /* EDP rules defined for encoding bulk rsrc created */
    EDU_INST_SET srmnd_srma_enc_all_rsrc_rules[ ] = {
        {EDU_START, srmnd_edp_enc_all_srma_rsrc, 0, 0, 0, 
                    sizeof(SRMND_MON_SRMA_USR_NODE), 0, NULL},
        
        {EDU_EXEC, srmnd_edp_enc_all_rsrc, EDQ_POINTER, 0, 0, 
                   (uns32)&((SRMND_MON_SRMA_USR_NODE*)0)->start_rsrc_mon_node, 0, NULL},     
                            
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Always encoding only */
   struct_ptr = (SRMND_MON_SRMA_USR_NODE *)ptr;   
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srmnd_srma_enc_all_rsrc_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);   
   return rc;
}



/****************************************************************************
  Name           :  srmsv_edp_nd_enc_bulk_created
 
  Description    :  This is the function which is used to encode decode 
                    SRMND_BULK_CREATED_RSRC_MON structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_nd_enc_bulk_created(EDU_HDL *edu_hdl,
                                EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr,
                                uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env,
                                EDP_OP_TYPE op,
                                EDU_ERR *o_err)
{
   uns32      rc = NCSCC_RC_SUCCESS;
   SRMND_MSG  *struct_ptr = NULL;


   /* EDP rules defined for encoding bulk rsrc created */
   EDU_INST_SET srmnd_bulk_created_enc_rules[ ] = {
        {EDU_START, srmsv_edp_nd_enc_bulk_created, 0, 0, 0, 
                    sizeof(SRMND_MSG), 0, NULL},
        
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMND_MSG*)0)->msg_type, 0, NULL},

        {EDU_EXEC, srmnd_edp_enc_all_srma_rsrc, EDQ_POINTER, 0, 0, 
                    (uns32)&((SRMND_MSG*)0)->info.bulk_rsrc_mon.srma_usr_node,
                    0, NULL},       

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   /* Take appropriate actions based on the operation (enc & dec) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (SRMND_MSG*)ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }
    
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srmnd_bulk_created_enc_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);

   return rc;
}


/****************************************************************************
  Name           :  srmsv_edp_nd_dec_bulk_created
 
  Description    :  This is the function which is used to encode decode 
                    SRMND_BULK_CREATED_RSRC_MON structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_nd_dec_bulk_created(EDU_HDL *edu_hdl,
                                    EDU_TKN *edu_tkn,
                                    NCSCONTEXT ptr,
                                    uns32 *ptr_data_len,
                                    EDU_BUF_ENV *buf_env,
                                    EDP_OP_TYPE op,
                                    EDU_ERR *o_err)
{
    uns32      rc = NCSCC_RC_SUCCESS;
    SRMND_BULK_CREATED_RSRC_MON  *struct_ptr = NULL, **d_ptr = NULL;


    /* EDP rules defined for decoding bulk rsrc created */
    EDU_INST_SET srmnd_bulk_created_dec_rules[ ] = {
        {EDU_START, srmsv_edp_nd_dec_bulk_created, 0, 0, 0, 
                    sizeof(SRMND_BULK_CREATED_RSRC_MON), 0, NULL},
        
        {EDU_EXEC, srmnd_edp_dec_all_rsrc, EDQ_POINTER, 0, 0, 
                   (uns32)&((SRMND_BULK_CREATED_RSRC_MON*)0)->bulk_rsrc_mon, 0, NULL},      

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };


   /* Take appropriate actions based on the operation (enc & dec) type */
   switch (op)
   {
   case EDP_OP_TYPE_DEC:
       d_ptr = (SRMND_BULK_CREATED_RSRC_MON **)ptr;
       if (*d_ptr == NULL)
       {
          *o_err = EDU_ERR_MEM_FAIL;
          return NCSCC_RC_FAILURE;          
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }
    
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srmnd_bulk_created_dec_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);

   return rc;
}


/****************************************************************************
  Name           :  srmsv_edp_nd_msg
 
  Description    :  This is the function which is used to encode decode 
                    SRMND_MSG structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_nd_msg(EDU_HDL *edu_hdl,
                       EDU_TKN *edu_tkn,
                       NCSCONTEXT ptr,
                       uns32 *ptr_data_len,
                       EDU_BUF_ENV *buf_env,
                       EDP_OP_TYPE op,
                       EDU_ERR *o_err)
{
    uns32      rc = NCSCC_RC_SUCCESS;
    SRMND_MSG  *struct_ptr = NULL, **d_ptr = NULL;


    EDU_INST_SET srmnd_msg_rules[ ] = {
        {EDU_START, srmsv_edp_nd_msg, 0, 0, 0, 
                    sizeof(SRMND_MSG), 0, NULL},

        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMND_MSG*)0)->msg_type, 0, NULL},

        {EDU_TEST, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMND_MSG*)0)->msg_type, 0, 
                                srmnd_msg_test_type_fnc},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((SRMND_MSG*)0)->srma_rsrc_hdl, 0, NULL},

        /* Update it for SRMND_CREATED_MON_MSG */
        {EDU_EXEC, ncs_edp_uns32, 0, 0, EDU_EXIT, 
            (uns32)&((SRMND_MSG*)0)->info.srmnd_rsrc_mon_hdl, 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((SRMND_MSG*)0)->srma_rsrc_hdl, 0, NULL},

        /* Update it for SRMND APPL-NOTIF MSG */
        {EDU_EXEC, srmsv_edp_value, 0, 0, EDU_EXIT, 
            (uns32)&((SRMND_MSG*)0)->info.notify_val, 0, NULL},

        /* Update it for SRMND APPL-NOTIF MSG */
        {EDU_EXEC, srmsv_edp_nd_dec_bulk_created, 0, 0, EDU_EXIT, 
            (uns32)&((SRMND_MSG*)0)->info.bulk_rsrc_mon, 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, EDU_EXIT, 
                    (uns32)&((SRMND_MSG*)0)->srma_rsrc_hdl, 0, NULL},

        /*Update watermark msg data */
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (uns32)&((SRMND_MSG*)0)->srma_rsrc_hdl, 0, NULL},

        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (uns32)&((SRMND_MSG*)0)->info.wm_val.rsrc_type, 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (uns32)&((SRMND_MSG*)0)->info.wm_val.pid, 0, NULL},

        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (uns32)&((SRMND_MSG*)0)->info.wm_val.wm_type, 0, NULL},

        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (uns32)&((SRMND_MSG*)0)->info.wm_val.wm_exist, 0, NULL},

        {EDU_TEST, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMND_MSG*)0)->info.wm_val.wm_exist, 0, 
                                srmnd_test_wm_exist_fnc},

        {EDU_TEST, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMND_MSG*)0)->info.wm_val.wm_type, 0, 
                                srmnd_test_wm_type_fnc},

        {EDU_EXEC, srmsv_edp_value, 0, 0, 0, 
            (uns32)&((SRMND_MSG*)0)->info.wm_val.low_val, 0, NULL},

        {EDU_EXEC, srmsv_edp_value, 0, 0, EDU_EXIT, 
            (uns32)&((SRMND_MSG*)0)->info.wm_val.high_val, 0, NULL},

        {EDU_EXEC, srmsv_edp_value, 0, 0, EDU_EXIT, 
            (uns32)&((SRMND_MSG*)0)->info.wm_val.low_val, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Take appropriate actions based on the operation (enc & dec) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (SRMND_MSG *)ptr;
       break;

   case EDP_OP_TYPE_DEC:
       d_ptr = (SRMND_MSG **)ptr;
       if (*d_ptr == NULL)
       {
          *d_ptr = m_MMGR_ALLOC_SRMND_MSG;
          if (*d_ptr == NULL)
          {
             *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
          }
          m_NCS_MEMSET(*d_ptr, '\0', sizeof(SRMND_MSG));
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srmnd_msg_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);
   return rc;
}


/****************************************************************************
  Name           :  srmsv_edp_agent_msg
 
  Description    :  This is the function which is used to encode decode 
                    SRMA_MSG structure.
 
  Return Values  :  NCSCC_RC_SUCCESS (or)
                    NCSCC_RC_FAILURE
 
  Notes          :  None.
******************************************************************************/
uns32 srmsv_edp_agent_msg(EDU_HDL *edu_hdl, 
                          EDU_TKN *edu_tkn,
                          NCSCONTEXT ptr,
                          uns32 *ptr_data_len,
                          EDU_BUF_ENV *buf_env,
                          EDP_OP_TYPE op,
                          EDU_ERR *o_err)
{
    uns32    rc = NCSCC_RC_SUCCESS;
    SRMA_MSG *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET srma_msg_rules[ ] = {
        {EDU_START, srmsv_edp_agent_msg, 0, 0, 0, 
                    sizeof(SRMA_MSG), 0, NULL},

        /* Event message type */
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMA_MSG*)0)->msg_type, 0, NULL},

        /* Boolean to represents whether it is a broadcast message */
        {EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0, 
                    (uns32)&((SRMA_MSG*)0)->bcast, 0, NULL},

        /* User specific SRMA handle */
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
                    (uns32)&((SRMA_MSG*)0)->handle, 0, NULL},

        /* Test Condition on message type */
        {EDU_TEST, ncs_edp_int, 0, 0, 0, 
                    (uns32)&((SRMA_MSG*)0)->msg_type, 0, 
                                srma_msg_test_type_fnc},
        
        /* EDP procedure for Create request */ 
        {EDU_EXEC, srmsv_edp_create_rsrc_mon, 0, 0, EDU_EXIT, 
                    (uns32)&((SRMA_MSG*)0)->info.create_mon, 0, NULL},

        /* EDP procedure for Delete request */ 
        {EDU_EXEC, ncs_edp_uns32, 0, 0, EDU_EXIT, 
                    (uns32)&((SRMA_MSG*)0)->info.del_rsrc_mon.srmnd_rsrc_hdl, 0, NULL},

        /* EDP procedure for Bulk Create request */ 
        {EDU_EXEC, srmsv_edp_agent_dec_bulk_create, 0, 0, EDU_EXIT, 
                    (uns32)&((SRMA_MSG*)0)->info.bulk_create_mon, 0, NULL},

        /* EDP procedure for Create request */ 
        {EDU_EXEC, srmsv_edp_get_watermark, 0, 0, EDU_EXIT, 
                    (uns32)&((SRMA_MSG*)0)->info.get_wm, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

   /* Take appropriate actions based on the operation (encode & decode) type */
   switch (op)
   {
   case EDP_OP_TYPE_ENC:
       struct_ptr = (SRMA_MSG *)ptr;
       break;

   case EDP_OP_TYPE_DEC:
       d_ptr = (SRMA_MSG **)ptr;
       if (*d_ptr == NULL)
       {
          *d_ptr = m_MMGR_ALLOC_SRMA_MSG;
          if (*d_ptr == NULL)
          {
             *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
          }
          m_NCS_MEMSET(*d_ptr, '\0', sizeof(SRMA_MSG));
       }
       struct_ptr = *d_ptr;
       break;

   default:
       struct_ptr = ptr;
       break;
   }
    
   /* Run EDU on the above defined rules */
   rc = m_NCS_EDU_RUN_RULES(edu_hdl,
                            edu_tkn,
                            srma_msg_rules,
                            struct_ptr,
                            ptr_data_len,
                            buf_env,
                            op,
                            o_err);
   return rc;
}


/****************************************************************************
  Name           :  srmsv_agent_nd_msg_free
 
  Description    :  This is the function is used to free the allocated memory
                    for SRMND message.
 
  Return Values  :  Nothing
 
  Notes          :  None.
******************************************************************************/
void  srmsv_agent_nd_msg_free(SRMND_MSG *msg)
{
   /* Clean up all the rsrc's records, if it BULK msg from  SRMND */
   if (msg->msg_type == SRMND_BULK_CREATED_MON_MSG)
   {
      SRMND_CREATED_RSRC_MON *bulk_rsrc = msg->info.bulk_rsrc_mon.bulk_rsrc_mon;
      SRMND_CREATED_RSRC_MON *next_rsrc;

      while (bulk_rsrc)
      {   
         next_rsrc = bulk_rsrc->next_srmnd_rsrc_mon;

         /* Free the memory */
         m_MMGR_FREE_SRMND_CREATED_RSRC(bulk_rsrc);

         bulk_rsrc = next_rsrc;
      }
   }

   /* Now free the SRMND message */
   m_MMGR_FREE_SRMND_MSG(msg);

   return;
}


















