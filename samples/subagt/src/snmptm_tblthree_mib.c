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
  MODULE NAME: SNMPTM_TBLTHREE_MIB.C

..............................................................................
  
  DESCRIPTION:  This module contains functions used by the SNMPTM Subsystem for
                SNMP operations on the SNMPTM Entp TBLTHREE table.
..............................................................................

  snmptm_tblthree_tbl_req .......... Process the Request of an entp tblthree table
  ncstesttablethreeentry_get ....... Get the object from the tblthree table
  ncstesttablethreeentry_set ....... Set the object in the tblthree table
  ncstesttablethreeentry_next ...... Get the next object from the tblthree table
  ncstesttablethreeentry_setrow .... Set the complete row of the tblthree table

******************************************************************************
*/

#include "snmptm.h"

#if(NCS_SNMPTM == 1) 

#ifndef SNMPTM_TBLTHREE_TBL_INST_LEN
#define SNMPTM_TBLTHREE_TBL_INST_LEN   4 /*  ip_addr (4)  */
#endif

/* function proto-types */
static uns32 get_tblthree_entry(SNMPTM_CB *snmptm,
                                NCSMIB_ARG *arg, 
                                SNMPTM_TBLTHREE **tblthree);
static uns32 get_next_tblthree_entry(SNMPTM_CB   *snmptm,
                                     NCSMIB_ARG  *arg,
                                     uns32       *io_next_inst_id,
                                     SNMPTM_TBLTHREE **tblthree);   
static void make_key_from_instance_id(SNMPTM_TBL_KEY *key,
                                      const uns32 *inst_id,
                                      uns32 inst_len);
static void make_instance_id_from_key(SNMPTM_TBL_KEY *key, uns32 *inst_id);


/****************************************************************************
  Name          :  snmptm_tblthree_tbl_req
  
  Description   :  High Level MIB Access Routine (Request function)for SNMPTM 
                   TBLTHREE table.
 
  Arguments     :  struct ncsmib_arg*
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_tblthree_tbl_req(struct ncsmib_arg *args)
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
                   searching the TBLTHREE Patricia Tree.
 
  Arguments     :  key - pointer to the SNMPTM_TBL_KEY structure                       
                   inst_id - pointer to the instance_id structure
 
  Return Values :  Nothing

  Notes         :  
****************************************************************************/
void make_key_from_instance_id(SNMPTM_TBL_KEY *key, const uns32 *inst_id, uns32 inst_len)
{
   uns32  count = 0, temp = 0;

   /* All in network address in the tblthree key */
   for (count = 0; count < inst_len ; count ++)
   {
      temp = (*(inst_id + count)) << ((4 - (count + 1)) * 8);
      key->ip_addr.info.v4 = key->ip_addr.info.v4 | temp;
   }

   key->ip_addr.info.v4 = htonl(key->ip_addr.info.v4);
   key->ip_addr.type = NCS_IP_ADDR_TYPE_IPV4;

   return;
}


/****************************************************************************
  Name          :  make_instance_id_from_key
 
  Description   :  From the key in the tblthree form the instance id  
 
  Arguments     :  SNMPTM_TBL_KEY *key,                       
                   uns32             *inst_id
 
  Return Values :  Nothing
 
  Notes         :  
****************************************************************************/
void make_instance_id_from_key(SNMPTM_TBL_KEY* key, uns32* inst_id)
{
   uns32  count = 0, ip_addr;

   ip_addr = ntohl(key->ip_addr.info.v4);

   for (count = 0; count < 4; count ++)
   {
      *(inst_id + count) = (ip_addr >> ((4 - (count + 1)) * 8)) & 0x000000ff;
   }

   return;
}


/****************************************************************************
 Name          :  get_tblthree_entry

 Description   :  Get the Instance of tblthree requested 
 
 Arguments     :  SNMPTM_CB     *snmptm,
                  NCSMIB_ARG    *arg, 
                  SNMPTM_TBLTHREE **tblthree
 
 Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE / NCSCC_RC_NO_INSTANCE

 Notes         :   
****************************************************************************/
uns32 get_tblthree_entry(SNMPTM_CB  *snmptm, NCSMIB_ARG  *arg, SNMPTM_TBLTHREE  **tblthree)
{
   SNMPTM_TBL_KEY  tblthree_key;

   m_NCS_OS_MEMSET(&tblthree_key, '\0', sizeof(tblthree_key));

   if(arg->i_idx.i_inst_len != SNMPTM_TBLTHREE_TBL_INST_LEN) 
      return NCSCC_RC_FAILURE;
   
   if(arg->i_idx.i_inst_ids == NULL)
      return NCSCC_RC_FAILURE;

   /* Prepare the key from the instant ID */
   make_key_from_instance_id(&tblthree_key, arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len);

   /* Can't be NULL */
   if(tblthree_key.ip_addr.info.v4 == (NCS_IPV4_ADDR) NULL)
      return NCSCC_RC_NO_INSTANCE;
   
   /* The tblthree is numbered, do a get based on address */
   if(((*tblthree = (SNMPTM_TBLTHREE*) ncs_patricia_tree_get(&snmptm->tblthree_tree,
         (uns8*) &tblthree_key)) == NULL) ||
         ((*tblthree)->tblthree_key.ip_addr.info.v4 !=
                             tblthree_key.ip_addr.info.v4))
         return NCSCC_RC_NO_INSTANCE;

   printf("TBLTHREE_ENTRY key address is: 0x%x\n", (*tblthree)->tblthree_key.ip_addr.info.v4);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  get_next_tblthree_entry
 
  Description   :  Find the Instance of tblthree requested 
 
  Arguments     :  SNMPTM_CB     *snmptm,
                   NCSMIB_ARG  *arg, 
                   uns32      *io_next_inst_id,
                   SNMPTM_TBLTHREE   **tblthree
 
  Return Values :  NCSCC_RC_SUCCESSS /
                   NCSCC_RC_FAILURE /
                   NCSCC_RC_NO_INSTANCE 
 
  Notes         :  
****************************************************************************/               
uns32 get_next_tblthree_entry(SNMPTM_CB *snmptm,
                            NCSMIB_ARG  *arg, 
                            uns32  *io_next_inst_id,
                            SNMPTM_TBLTHREE  **tblthree)
{
   SNMPTM_TBL_KEY  tblthree_key;
   
   m_NCS_OS_MEMSET(&tblthree_key, '\0', sizeof(tblthree_key));
   
   if ((arg->i_idx.i_inst_len == 0) ||
       (arg->i_idx.i_inst_ids == NULL))
   {
      /* First tblthree entry is requested */
      if ((*tblthree = (SNMPTM_TBLTHREE*)ncs_patricia_tree_getnext(&snmptm->tblthree_tree, 
                                                      (uns8*)0)) == NULL) 
         return NCSCC_RC_NO_INSTANCE;
   }
   else
   { 
      if (arg->i_idx.i_inst_len > SNMPTM_TBLTHREE_TBL_INST_LEN)
         return NCSCC_RC_NO_INSTANCE;

      /* Prepare the tblthree key from the instant ID */
      make_key_from_instance_id(&tblthree_key, arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len);

      /* Can't be NULL */
      if (tblthree_key.ip_addr.info.v4 == (NCS_IPV4_ADDR) NULL)
         return NCSCC_RC_NO_INSTANCE;
   
      /* The tblthree is numbered, do a getnext based on address */
      if (((*tblthree = (SNMPTM_TBLTHREE*) ncs_patricia_tree_getnext(&snmptm->tblthree_tree,
            (uns8*) &tblthree_key)) == NULL)) 
         return NCSCC_RC_NO_INSTANCE;
   }
  
   printf("TBLTHREE_ENTRY IP address is: 0x%x\n", (*tblthree)->tblthree_key.ip_addr.info.v4);
   /* Make an instance id from tblthree_key */
   make_instance_id_from_key(&((*tblthree)->tblthree_key), io_next_inst_id); 

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstesttablethreeentry_set
  
  Description   :  Set function for SNMPTM tblthree table objects 
 
  Arguments     :  NCSCONTEXT snmptm         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg           - MIB argument (input)
                   NCSMIB_VAR_INFO* var_info - Object properties/info(i)
                   NCS_BOOL test_flag        - set/test operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablethreeentry_set(NCSCONTEXT cb, 
                               NCSMIB_ARG *arg, 
                               NCSMIB_VAR_INFO *var_info,
                               NCS_BOOL test_flag)
{
   SNMPTM_CB           *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLTHREE     *tblthree = SNMPTM_TBLTHREE_NULL;
   SNMPTM_TBL_KEY      tblthree_key;
   NCS_BOOL            val_same_flag = FALSE;
   NCSMIB_SET_REQ      *i_set_req = &arg->req.info.set_req; 
   uns32               rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  temp_mib_req;
   uns8                create_flag = FALSE;

   m_NCS_CONS_PRINTF("\nncsTestTableThreeEntry:  Received SNMP SET request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg); 

   m_NCS_OS_MEMSET(&tblthree_key, '\0', sizeof(SNMPTM_TBL_KEY));    
   m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 
  
   /* Prepare the key from the instant ID */
   make_key_from_instance_id(&tblthree_key, arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len);

   /* Check whether the entry exists in TBLTHREE table with the same key?? */
   tblthree = (SNMPTM_TBLTHREE*)ncs_patricia_tree_get(&snmptm->tblthree_tree,
                                                      (uns8*)&tblthree_key);

   /* Validate row status */
   if (i_set_req->i_param_val.i_param_id == ncsTestTableThreeRowStatus_ID)
   {
      uns32  rc = NCSCC_RC_SUCCESS;

      if ((i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_CREATE_GO) ||
          (i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_CREATE_WAIT))
      {
         create_flag = TRUE;
      }
      
      temp_mib_req.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP; 
      temp_mib_req.info.i_val_status_util_info.row_status =
                                      &(i_set_req->i_param_val.info.i_int);
      temp_mib_req.info.i_val_status_util_info.row = tblthree;
      
      /* call the mib routine handler */ 
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      {
         return rc;
      }
   }

   /* If the corresponding entry is not exists, and if the object is of 
      read-create type then create a tblthree entry. */
   if ((tblthree == NULL) && (test_flag != TRUE))
   {
      if (var_info->access == NCSMIB_ACCESS_READ_CREATE) 
      {
         if ((i_set_req->i_param_val.i_param_id != ncsTestTableThreeRowStatus_ID) ||
             ((i_set_req->i_param_val.i_param_id == ncsTestTableThreeRowStatus_ID)
              && (create_flag != FALSE)))
         {
             tblthree = snmptm_create_tblthree_entry(snmptm, &tblthree_key);

             /* Not able to create a tblthree entry */
             if (tblthree == NULL)
               return NCSCC_RC_FAILURE;
         }
      }
      else
         return NCSCC_RC_NO_INSTANCE;
   }

   /* All the tests have been done now set the value */
   if (test_flag != TRUE) 
   {
      if (tblthree == NULL)
          return NCSCC_RC_NO_INSTANCE;

      if ((i_set_req->i_param_val.i_param_id == ncsTestTableThreeRowStatus_ID)
           && (i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_DESTROY))
      {
         /* Delete the entry from the TBLTHREE */
         snmptm_delete_tblthree_entry(snmptm, tblthree);
      }
      else
      {
         m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

         temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
         temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
         temp_mib_req.info.i_set_util_info.var_info = var_info;
         temp_mib_req.info.i_set_util_info.data = tblthree;
         temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

         /* call the mib routine handler */ 
         if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
         {
            return rc;
         }
      }
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : ncstesttablethreeentry_extract 
  
  Description   :  Get function for SNMPTM tblthree table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get request
                                       will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablethreeentry_extract(NCSMIB_PARAM_VAL *param, 
                                   NCSMIB_VAR_INFO  *var_info,
                                   NCSCONTEXT data,
                                   NCSCONTEXT buffer)
{
   return ncsmiblib_get_obj_val(param, var_info, data, buffer);
}


/****************************************************************************
  Name          :  ncstesttablethreeentry_get 
  
  Description   :  Get function for SNMPTM tblthree table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get request
                                       will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32  ncstesttablethreeentry_get(NCSCONTEXT cb, 
                                NCSMIB_ARG *arg, 
                                NCSCONTEXT* data)
{
   SNMPTM_CB     *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLTHREE *tblthree = SNMPTM_TBLTHREE_NULL;
   uns32         ret_code = NCSCC_RC_SUCCESS;


   m_NCS_CONS_PRINTF("\nncsTestTableThreeEntry:  Received SNMP GET request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg); 
   
   if((ret_code = get_tblthree_entry(snmptm, arg, &tblthree)) != NCSCC_RC_SUCCESS)
       return ret_code;

   *data = tblthree;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstesttablethreeentry_next
  
  Description   :  Next function for SNMPTM tblthree table 
 
  Arguments     :  NCSCONTEXT snmptm,   -  SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,     -  MIB argument (input)
                   NCSCONTEXT* data     -  pointer to the data from which get
                                           request will be serviced
                   uns32* next_inst_id  -  instance id of the object

  Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablethreeentry_next(NCSCONTEXT snmptm, 
                                NCSMIB_ARG *arg, 
                                NCSCONTEXT* data, 
                                uns32* next_inst_id,
                                uns32* next_inst_id_len) 
{
   SNMPTM_TBLTHREE *tblthree = SNMPTM_TBLTHREE_NULL;
   uns32         ret_code = NCSCC_RC_SUCCESS;
 
  
   m_NCS_CONS_PRINTF("\nncsTestTableThreeEntry:  Received SNMP NEXT request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg); 
   
   if ((ret_code = get_next_tblthree_entry(snmptm, arg, next_inst_id, &tblthree))
       != NCSCC_RC_SUCCESS)
       return ret_code;

   *data = tblthree;
   *next_inst_id_len = SNMPTM_TBLTHREE_TBL_INST_LEN;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstesttablethreeentry_setrow
  
  Description   :  Setrow function for SNMPTM tblthree table 
 
  Arguments     :  NCSCONTEXT snmptm,         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,           - MIB argument (input)
                   NCSMIB_SETROW_PARAM_VAL    - array of params to be set
                   ncsmib_obj_info* obj_info, - Object and table properties/info(i)
                   NCS_BOOL test_flag         - setrow/testrow operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablethreeentry_setrow(NCSCONTEXT cb, 
                                  NCSMIB_ARG* arg,
                                  NCSMIB_SETROW_PARAM_VAL* tblthree_param,
                                  struct ncsmib_obj_info* obj_info,
                                  NCS_BOOL testrow_flag)
{
   SNMPTM_CB           *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLTHREE       *tblthree = SNMPTM_TBLTHREE_NULL;
   SNMPTM_TBL_KEY   tblthree_key;
   NCSMIBLIB_REQ_INFO  temp_mib_req;   


   m_NCS_CONS_PRINTF("\nncsTestTableThreeEntry: Received SNMP SETROW request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg); 
   
   m_NCS_OS_MEMSET(&tblthree_key, '\0', sizeof(SNMPTM_TBL_KEY));
   m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

   /* Prepare the key from the instant ID */
   make_key_from_instance_id(&tblthree_key, arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len);

   /* Check whether the entry exists in TBLTHREE table with the same key?? */
   tblthree = (SNMPTM_TBLTHREE*)ncs_patricia_tree_get(&snmptm->tblthree_tree,
                                                  (uns8*)&tblthree_key);
   /* Validate row status */
   if (tblthree_param[ncsTestTableThreeRowStatus_ID - 1].set_flag == TRUE)
   { 
      uns32  rc = NCSCC_RC_SUCCESS;
  
      temp_mib_req.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP; 
      temp_mib_req.info.i_val_status_util_info.row_status =
             &(tblthree_param[ncsTestTableThreeRowStatus_ID - 1].param.info.i_int);
      temp_mib_req.info.i_val_status_util_info.row = tblthree;
      
      /* call the mib routine handler */ 
      if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      { 
         return rc;
      }
   } 
 
   /* If the corresponding entry is not exists, and if the object is of 
      read-create type then create a tblthree entry. */
   if (tblthree == NULL) 
   {
      if (tblthree_param[ncsTestTableThreeRowStatus_ID - 1].set_flag == TRUE)
      {
         tblthree = snmptm_create_tblthree_entry(snmptm, &tblthree_key);

         /* Not able to create a tblthree entry */
         if (tblthree == NULL)
             return NCSCC_RC_SUCCESS;
      }
      else
         return NCSCC_RC_NO_INSTANCE;
   }

   if(testrow_flag != TRUE)
   {
      NCS_BOOL         val_same_flag = FALSE;
      NCSMIB_PARAM_ID  param_id = 0;
      uns32            rc = 0;

      for(param_id = ncsTestTableThreeIpAddress_ID; 
          param_id < ncsTestTableThreeEntryMax_ID; 
          param_id++)
          {  
             if(param_id == ncsTestTableThreeIpAddress_ID)
                continue;

             if (tblthree_param[param_id - 1].set_flag == TRUE)
             {
                 m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 
                   
                 temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
                 temp_mib_req.info.i_set_util_info.param = &(tblthree_param[param_id - 1].param);
                 temp_mib_req.info.i_set_util_info.var_info = &obj_info->var_info[param_id - 1];
                 temp_mib_req.info.i_set_util_info.data = tblthree;
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

#endif /* NCS_SNMPTM */

