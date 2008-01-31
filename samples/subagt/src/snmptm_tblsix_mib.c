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
  MODULE NAME: SNMPTM_TBLSIX_MIB.C

..............................................................................
  
  DESCRIPTION:  This module contains functions used by the SNMPTM Subsystem for
                SNMP operations on the SNMPTM Entp TBLSIX table.
..............................................................................

  snmptm_tblsix_tbl_req .......... Process the Request of an entp tblsix table
  ncstesttablesixentry_rmvrow ..... Remove a row in the tblsix table
  ncstesttablesixentry_get ....... Get the object from the tblsix table
  ncstesttablesixentry_set ....... Set the object in the tblsix table
  ncstesttablesixentry_next ...... Get the next object from the tblsix table
  ncstesttablesixentry_setrow .... Set the complete row of the tblsix table

******************************************************************************
*/

#include "snmptm.h"

#if(NCS_SNMPTM == 1) 

#ifndef SNMPTM_TBLSIX_TBL_INST_LEN
#define SNMPTM_TBLSIX_TBL_INST_LEN   1 /*  ip_addr (4)  */
#endif

/* function proto-types */
static uns32 get_tblsix_entry(SNMPTM_CB *snmptm,
                               NCSMIB_ARG *arg, 
                               SNMPTM_TBLSIX **tblsix);
static uns32 get_next_tblsix_entry(SNMPTM_CB   *snmptm,
                                    NCSMIB_ARG  *arg,
                                    uns32       *io_next_inst_id,
                                    SNMPTM_TBLSIX **tblsix);   
static void make_key_from_instance_id(SNMPTM_TBLSIX_KEY *key,
                                      const uns32 *inst_id);
static void make_instance_id_from_key(SNMPTM_TBLSIX_KEY *key, uns32 *inst_id);


/****************************************************************************
  Name          :  snmptm_tblsix_tbl_req
  
  Description   :  High Level MIB Access Routine (Request function)for SNMPTM 
                   TBLSIX table.
 
  Arguments     :  struct ncsmib_arg*
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_tblsix_tbl_req(struct ncsmib_arg *args)
{
   SNMPTM_CB           *snmptm = SNMPTM_CB_NULL;
   uns32               status = NCSCC_RC_SUCCESS; 
   NCSMIBLIB_REQ_INFO  miblib_req; 
   uns32               cb_hdl = args->i_mib_key;

   /* Get the CB from the handle manager */
   if ((snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             cb_hdl)) == NULL)
   {
      args->rsp.i_status = NCSCC_RC_NO_INSTANCE;
      args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
      args->i_rsp_fnc(args);
   
      return NCSCC_RC_FAILURE;
   }  
   
   if((args->i_policy & NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER) ==
         NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER)
   {
      m_NCS_CONS_PRINTF("For TBLSIX, last playback update from PSS received...]n");
   }
   m_NCS_OS_MEMSET(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP; 
   miblib_req.info.i_mib_op_info.args = args; 
   miblib_req.info.i_mib_op_info.cb = snmptm; 
   
   m_SNMPTM_LOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);
   
   /* call the miblib routine handler */ 
   status = ncsmiblib_process_req(&miblib_req); 

   m_SNMPTM_UNLOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);

   /* Release SNMPTM CB handle */
   ncshm_give_hdl(cb_hdl);
   
   return status; 
}


/****************************************************************************
  Name          :  make_key_from_instance_id
  
  Description   :  From the instance ids provided, prepares the key for 
                   searching the TBLSIX Patricia Tree.
 
  Arguments     :  key - pointer to the SNMPTM_TBLSIX_KEY structure                       
                   inst_id - pointer to the instance_id structure
 
  Return Values :  Nothing

  Notes         :  
****************************************************************************/
void make_key_from_instance_id(SNMPTM_TBLSIX_KEY *key, const uns32 *inst_id)
{
   key->count = *inst_id;

   return;
}


/****************************************************************************
  Name          :  make_instance_id_from_key
 
  Description   :  From the key in the tblsix form the instance id  
 
  Arguments     :  SNMPTM_TBLSIX_KEY *key,                       
                   uns32             *inst_id
 
  Return Values :  Nothing
 
  Notes         :  
****************************************************************************/
void make_instance_id_from_key(SNMPTM_TBLSIX_KEY* key, uns32* inst_id)
{
   *(inst_id) = key->count;

   return;
}


/****************************************************************************
 Name          :  get_tblsix_entry

 Description   :  Get the Instance of tblsix requested 
 
 Arguments     :  SNMPTM_CB     *snmptm,
                  NCSMIB_ARG    *arg, 
                  SNMPTM_TBLSIX **tblsix
 
 Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE / NCSCC_RC_NO_INSTANCE

 Notes         :   
****************************************************************************/
uns32 get_tblsix_entry(SNMPTM_CB  *snmptm, NCSMIB_ARG  *arg, SNMPTM_TBLSIX  **tblsix)
{
   SNMPTM_TBLSIX_KEY  tblsix_key;

   m_NCS_OS_MEMSET(&tblsix_key, '\0', sizeof(tblsix_key));

   if(arg->i_idx.i_inst_len != SNMPTM_TBLSIX_TBL_INST_LEN) 
      return NCSCC_RC_FAILURE;
   
   if(arg->i_idx.i_inst_ids == NULL)
      return NCSCC_RC_FAILURE;

   /* Prepare the key from the instant ID */
   make_key_from_instance_id(&tblsix_key, arg->i_idx.i_inst_ids);

   /* The tblsix is numbered, do a get based on address */
   if(((*tblsix = (SNMPTM_TBLSIX*) ncs_patricia_tree_get(&snmptm->tblsix_tree,
         (uns8*) &tblsix_key)) == NULL) ||
         ((*tblsix)->tblsix_key.count != tblsix_key.count))
         return NCSCC_RC_NO_INSTANCE;

   printf("TBLSIX_ENTRY key is: %d\n", (*tblsix)->tblsix_key.count);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  get_next_tblsix_entry
 
  Description   :  Find the Instance of tblsix requested 
 
  Arguments     :  SNMPTM_CB     *snmptm,
                   NCSMIB_ARG  *arg, 
                   uns32      *io_next_inst_id,
                   SNMPTM_TBLSIX   **tblsix
 
  Return Values :  NCSCC_RC_SUCCESSS /
                   NCSCC_RC_FAILURE /
                   NCSCC_RC_NO_INSTANCE 
 
  Notes         :  
****************************************************************************/               
uns32 get_next_tblsix_entry(SNMPTM_CB *snmptm,
                            NCSMIB_ARG  *arg, 
                            uns32  *io_next_inst_id,
                            SNMPTM_TBLSIX  **tblsix)
{
   SNMPTM_TBLSIX_KEY  tblsix_key;
   
   m_NCS_OS_MEMSET(&tblsix_key, '\0', sizeof(tblsix_key));
   
   if ((arg->i_idx.i_inst_len == 0) ||
       (arg->i_idx.i_inst_ids == NULL))
   {
      /* First tblsix entry is requested */
      if ((*tblsix = (SNMPTM_TBLSIX*)ncs_patricia_tree_getnext(&snmptm->tblsix_tree, 
                                                      (uns8*)0)) == NULL) 
         return NCSCC_RC_NO_INSTANCE;
   }
   else
   { 
      if (arg->i_idx.i_inst_len != SNMPTM_TBLSIX_TBL_INST_LEN)
         return NCSCC_RC_NO_INSTANCE;

      /* Prepare the tblsix key from the instant ID */
      make_key_from_instance_id(&tblsix_key, arg->i_idx.i_inst_ids);

      /* The tblsix is numbered, do a getnext based on address */
      if (((*tblsix = (SNMPTM_TBLSIX*) ncs_patricia_tree_getnext(&snmptm->tblsix_tree,
            (uns8*) &tblsix_key)) == NULL)) 
         return NCSCC_RC_NO_INSTANCE;
   }
  
   printf("TBLSIX_ENTRY key is: %d\n", (*tblsix)->tblsix_key.count);
   /* Make an instance id from tblsix_key */
   make_instance_id_from_key(&((*tblsix)->tblsix_key), io_next_inst_id); 

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstesttablesixentry_set
  
  Description   :  Set function for SNMPTM tblsix table objects 
 
  Arguments     :  NCSCONTEXT snmptm         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg           - MIB argument (input)
                   NCSMIB_VAR_INFO* var_info - Object properties/info(i)
                   NCS_BOOL test_flag        - set/test operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablesixentry_set(NCSCONTEXT cb, 
                               NCSMIB_ARG *arg, 
                               NCSMIB_VAR_INFO *var_info,
                               NCS_BOOL test_flag)
{
   SNMPTM_CB           *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLSIX      *tblsix = SNMPTM_TBLSIX_NULL;
   SNMPTM_TBLSIX_KEY      tblsix_key;
   NCS_BOOL            val_same_flag = FALSE;
   NCSMIB_SET_REQ      *i_set_req = &arg->req.info.set_req; 
   uns32               rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  temp_mib_req;

   m_NCS_CONS_PRINTF("\nncsTestTableSixEntry:  Received SNMP SET request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg); 

   m_NCS_OS_MEMSET(&tblsix_key, '\0', sizeof(SNMPTM_TBLSIX_KEY));    
   m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 
  
   /* Prepare the key from the instant ID */
   make_key_from_instance_id(&tblsix_key, arg->i_idx.i_inst_ids);

   /* Check whether the entry exists in TBLSIX table with the same key?? */
   tblsix = (SNMPTM_TBLSIX*)ncs_patricia_tree_get(&snmptm->tblsix_tree,
                                                      (uns8*)&tblsix_key);

   /* If the corresponding entry is not exists, and if the object is of 
      read-create type then create a tblsix entry. */
   if ((tblsix == NULL) && (test_flag != TRUE))
   {
         tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);

         /* Not able to create a tblsix entry */
         if (tblsix == NULL)
            return NCSCC_RC_FAILURE;
   }

   /* All the tests have been done now set the value */
   if (test_flag != TRUE) 
   {
      if (tblsix == NULL)
         return NCSCC_RC_NO_INSTANCE;

         m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

         temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
         temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
         temp_mib_req.info.i_set_util_info.var_info = var_info;
         temp_mib_req.info.i_set_util_info.data = tblsix;
         temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

         /* call the mib routine handler */ 
         if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
         {
            return rc;
         }
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : ncstesttablesixentry_extract 
  
  Description   :  Get function for SNMPTM tblsix table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get request
                                       will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablesixentry_extract(NCSMIB_PARAM_VAL *param, 
                                   NCSMIB_VAR_INFO  *var_info,
                                   NCSCONTEXT data,
                                   NCSCONTEXT buffer)
{
   return ncsmiblib_get_obj_val(param, var_info, data, buffer);
}


/****************************************************************************
  Name          :  ncstesttablesixentry_get 
  
  Description   :  Get function for SNMPTM tblsix table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get request
                                       will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32  ncstesttablesixentry_get(NCSCONTEXT cb, 
                                NCSMIB_ARG *arg, 
                                NCSCONTEXT* data)
{
   SNMPTM_CB     *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLSIX *tblsix = SNMPTM_TBLSIX_NULL;
   uns32         ret_code = NCSCC_RC_SUCCESS;


   m_NCS_CONS_PRINTF("\nncsTestTableSixEntry:  Received SNMP GET request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg); 
   
   if((ret_code = get_tblsix_entry(snmptm, arg, &tblsix)) != NCSCC_RC_SUCCESS)
       return ret_code;

   *data = tblsix;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstesttablesixentry_next
  
  Description   :  Next function for SNMPTM tblsix table 
 
  Arguments     :  NCSCONTEXT snmptm,   -  SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,     -  MIB argument (input)
                   NCSCONTEXT* data     -  pointer to the data from which get
                                           request will be serviced
                   uns32* next_inst_id  -  instance id of the object

  Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablesixentry_next(NCSCONTEXT snmptm, 
                                NCSMIB_ARG *arg, 
                                NCSCONTEXT* data, 
                                uns32* next_inst_id,
                                uns32* next_inst_id_len) 
{
   SNMPTM_TBLSIX *tblsix = SNMPTM_TBLSIX_NULL;
   uns32         ret_code = NCSCC_RC_SUCCESS;
 
  
   m_NCS_CONS_PRINTF("\nncsTestTableSixEntry:  Received SNMP NEXT request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg); 
   
   if ((ret_code = get_next_tblsix_entry(snmptm, arg, next_inst_id, &tblsix))
       != NCSCC_RC_SUCCESS)
       return ret_code;

   *data = tblsix;
   *next_inst_id_len = SNMPTM_TBLSIX_TBL_INST_LEN;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstesttablesixentry_setrow
  
  Description   :  Setrow function for SNMPTM tblsix table 
 
  Arguments     :  NCSCONTEXT snmptm,         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,           - MIB argument (input)
                   NCSMIB_SETROW_PARAM_VAL    - array of params to be set
                   ncsmib_obj_info* obj_info, - Object and table properties/info(i)
                   NCS_BOOL test_flag         - setrow/testrow operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablesixentry_setrow(NCSCONTEXT cb, 
                                  NCSMIB_ARG* arg,
                                  NCSMIB_SETROW_PARAM_VAL* tblsix_param,
                                  struct ncsmib_obj_info* obj_info,
                                  NCS_BOOL testrow_flag)
{
   SNMPTM_CB           *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLSIX       *tblsix = SNMPTM_TBLSIX_NULL;
   SNMPTM_TBLSIX_KEY   tblsix_key;
   NCSMIBLIB_REQ_INFO  temp_mib_req;   


   m_NCS_CONS_PRINTF("\nncsTestTableSixEntry: Received SNMP SETROW request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg); 
   
   m_NCS_OS_MEMSET(&tblsix_key, '\0', sizeof(SNMPTM_TBLSIX_KEY));
   m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

   /* Prepare the key from the instant ID */
   make_key_from_instance_id(&tblsix_key, arg->i_idx.i_inst_ids);

   /* Check whether the entry exists in TBLSIX table with the same key?? */
   tblsix = (SNMPTM_TBLSIX*)ncs_patricia_tree_get(&snmptm->tblsix_tree,
                                                  (uns8*)&tblsix_key);

   if(testrow_flag != TRUE)
   {
      NCS_BOOL         val_same_flag = FALSE;
      NCSMIB_PARAM_ID  param_id = 0;
      uns32            rc = 0;

      for(param_id = ncsTestTableSixCount_ID; 
          param_id < ncsTestTableSixEntryMax_ID; 
          param_id++)
          {  
             if(param_id == ncsTestTableSixCount_ID)
                continue;

             if (tblsix_param[param_id - 1].set_flag == TRUE)
             {
                 m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 
                   
                 /* If the corresponding entry does not exist, and if the object is 
                    writeable and the SET has come from PSS, then create a tblsix entry now. */
                 if((tblsix == NULL) && 
                    ((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) == NCSMIB_POLICY_PSS_BELIEVE_ME))
                 {
                    tblsix = snmptm_create_tblsix_entry(snmptm, &tblsix_key);
                    if (tblsix == NULL)
                       return NCSCC_RC_SUCCESS;
                 }

                 temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
                 temp_mib_req.info.i_set_util_info.param = &(tblsix_param[param_id - 1].param);
                 temp_mib_req.info.i_set_util_info.var_info = &obj_info->var_info[param_id - 1];
                 temp_mib_req.info.i_set_util_info.data = tblsix;
                 temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

                 /* call the mib routine handler */ 
                 if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
                 {
                    return rc;
                 }

                 val_same_flag = FALSE;
             }
          }
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstesttablesixentry_rmvrow
  
  Description   :  Remove-row function handler for SNMPTM tblsix table 
 
  Arguments     :  NCSCONTEXT snmptm,         - SNMPTM control block (input/output)
                   NCSMIB_IDX *idx,           - Row MIB index to be deleted(input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablesixentry_rmvrow(NCSCONTEXT cb_hdl, NCSMIB_IDX *idx)
{
   SNMPTM_CB           *snmptm = SNMPTM_CB_NULL;
   SNMPTM_TBLSIX      *tblsix = SNMPTM_TBLSIX_NULL;
   SNMPTM_TBLSIX_KEY      tblsix_key;
   uns32               status = NCSCC_RC_SUCCESS; 

   /* Get the CB from the handle manager */
   if ((snmptm = (SNMPTM_CB *)cb_hdl) == NULL)
   {
      return NCSCC_RC_FAILURE;
   }  

   /* Now, delete the row from the tblsix */
   m_NCS_OS_MEMSET(&tblsix_key, '\0', sizeof(SNMPTM_TBLSIX_KEY));    
  
   /* Prepare the key from the instant ID */
   make_key_from_instance_id(&tblsix_key, idx->i_inst_ids);

   /* Check whether the entry exists in TBLSIX table with the same key?? */
   tblsix = (SNMPTM_TBLSIX*)ncs_patricia_tree_get(&snmptm->tblsix_tree,
                                                      (uns8*)&tblsix_key);
   if(tblsix != NULL)
   {
      snmptm_delete_tblsix_entry(snmptm, tblsix);
   }
   else
   {
      status = NCSCC_RC_FAILURE;
   }
  
   return status;
}

#endif /* NCS_SNMPTM */

