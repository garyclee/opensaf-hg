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

  MODULE NAME: SNMPTM_TBLEIGHT.C 

..............................................................................

  DESCRIPTION:  Defines the APIs to add/del SNMPTM TBLEIGHT entries and the 
                procedure that does the clean-up of SNMPTM tbleight list.
..............................................................................


******************************************************************************/

#include "snmptm.h"

#if (NCS_SNMPTM == 1)


#ifndef SNMPTM_TBLEIGHT_TBL_INST_LEN
#define SNMPTM_TBLEIGHT_TBL_INST_LEN 3
#endif

/* function proto-types */
static uns32 get_tbleight_entry(SNMPTM_CB *snmptm,
                              NCSMIB_ARG *arg,
                              SNMPTM_TBLEIGHT **tbleight);
static uns32 get_next_tbleight_entry(SNMPTM_CB   *snmptm,
                                   NCSMIB_ARG  *arg,
                                   uns32       *io_next_inst_id,
                                   SNMPTM_TBLEIGHT **tbleight);


/****************************************************************************
  Name          :  snmptm_tbleight_tbl_req
  
  Description   :  High Level MIB Access Routine (Request function)for SNMPTM 
                   TBLEIGHT table.
 
  Arguments     :  struct ncsmib_arg*
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_tbleight_tbl_req(struct ncsmib_arg *args)
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
   
   memset(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 
   miblib_req.req = NCSMIBLIB_REQ_MIB_OP; 
   miblib_req.info.i_mib_op_info.cb = snmptm; 

   miblib_req.info.i_mib_op_info.args = args; 
       
   m_SNMPTM_LOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);
       
   /* call the miblib routine handler */ 
   status = ncsmiblib_process_req(&miblib_req); 

   m_SNMPTM_UNLOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);

   /* Release SNMPTM CB handle */
   ncshm_give_hdl(cb_hdl);
   
   return status; 
}

/*****************************************************************************
  Name          :  snmptm_create_tbleight_entry

  Description   :  This function creates an entry into SNMPTM TBLEIGHT table.
                   Basically it adds a tbleight node to a tbleight patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tbleight_key - pointer to the TBLEIGHT request info struct.

  Return Values :  tbleight - Upon successful addition of tbleight entry
                   NULL   - Upon failure of adding tbleight entry

  Notes         :
*****************************************************************************/
SNMPTM_TBLEIGHT *snmptm_create_tbleight_entry(SNMPTM_CB *snmptm,
                                          SNMPTM_TBLEIGHT_KEY *tbleight_key)
{

   SNMPTM_TBLEIGHT *tbleight = NULL;

   /* Alloc the memory for TBLEIGHT entry */
   if ((tbleight = (SNMPTM_TBLEIGHT*)m_MMGR_ALLOC_SNMPTM_TBLEIGHT)
        == SNMPTM_TBLEIGHT_NULL)
   {
      m_NCS_CONS_PRINTF("\nNot able to alloc the memory for TBLEIGHT \n");
      return NULL;
   }

   memset((char *)tbleight, '\0', sizeof(SNMPTM_TBLEIGHT));

   /* Copy the key contents to tbleight struct */
   tbleight->tbleight_key.ifIndex = tbleight_key->ifIndex;
   tbleight->tbleight_key.tbleight_unsigned32 = tbleight_key->tbleight_unsigned32;
   tbleight->tbleight_key.tblfour_unsigned32 = tbleight_key->tblfour_unsigned32;

   tbleight->tbleight_pat_node.key_info = (uns8 *)&((tbleight->tbleight_key));

   /* Add to the tbleight entry/node to tbleight patricia tree */
   if(NCSCC_RC_SUCCESS != ncs_patricia_tree_add(&(snmptm->tbleight_tree),
                                                &(tbleight->tbleight_pat_node)))
   {
      m_NCS_CONS_PRINTF("\nNot able add TBLEIGHT node to TBLEIGHT tree.\n");

      /* Free the alloc memory of TBLEIGHT */
      m_MMGR_FREE_SNMPTM_TBLEIGHT(tbleight);

      return NULL;
   }

   tbleight->tbleight_row_status = NCSMIB_ROWSTATUS_ACTIVE;

   return tbleight;
}

/*****************************************************************************
  Name          :  snmptm_delete_tbleight_entry

  Description   :  This function deletes an entry from TBLEIGHT table. Basically
                   it deletes a tbleight node from a tbleight patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tbleight - pointer to the TBLEIGHT struct.

  Return Values :  Nothing

  Notes         :
*****************************************************************************/
void snmptm_delete_tbleight_entry(SNMPTM_CB *snmptm,
                                SNMPTM_TBLEIGHT *tbleight)
{

   /* Delete the tbleight entry from the tbleight patricia tree */
   ncs_patricia_tree_del(&(snmptm->tbleight_tree), &(tbleight->tbleight_pat_node));

   /* Free the memory of TBLEIGHT */
   m_MMGR_FREE_SNMPTM_TBLEIGHT(tbleight);

   return;
}

/*****************************************************************************
  Name          :  snmptm_delete_tbleight

  Description   :  This function deletes all the tbleight entries (nodes) from
                   tbleight tree and destroys a tbleight tree.

  Arguments     :  snmptm  - Pointer to the SNMPTM CB structure

  Return Values :

  Notes         :
*****************************************************************************/
void  snmptm_delete_tbleight(SNMPTM_CB *snmptm)
{
   SNMPTM_TBLEIGHT *tbleight;

   /* Delete all the TBLEIGHT entries */
   while(SNMPTM_TBLEIGHT_NULL != (tbleight = (SNMPTM_TBLEIGHT*)
                        ncs_patricia_tree_getnext(&snmptm->tbleight_tree, 0)))
   {
       /* Delete the node from the tbleight tree */
       ncs_patricia_tree_del(&snmptm->tbleight_tree, &tbleight->tbleight_pat_node);

       /* Free the memory */
       m_MMGR_FREE_SNMPTM_TBLEIGHT(tbleight);
   }

   /* Destroy TBLEIGHT tree */
   ncs_patricia_tree_destroy(&snmptm->tbleight_tree);

   return;
}



/****************************************************************************
 Name          :  get_tbleight_entry

 Description   :  Get the Instance of tbleight requested

 Arguments     :  SNMPTM_CB     *snmptm,
                  NCSMIB_ARG    *arg,
                  SNMPTM_TBLEIGHT **tbleight

 Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE / NCSCC_RC_NO_INSTANCE

 Notes         :
****************************************************************************/
uns32 get_tbleight_entry(SNMPTM_CB  *snmptm, NCSMIB_ARG  *arg, SNMPTM_TBLEIGHT  **tbleight)
{
   SNMPTM_TBLEIGHT_KEY  tbleight_key;

   memset(&tbleight_key, '\0', sizeof(tbleight_key));

   if(arg->i_idx.i_inst_len != SNMPTM_TBLEIGHT_TBL_INST_LEN)
      return NCSCC_RC_FAILURE;

   if(arg->i_idx.i_inst_ids == NULL)
      return NCSCC_RC_FAILURE;

   /* Prepare the key from the instant ID */
   tbleight_key.ifIndex = arg->i_idx.i_inst_ids[0];
   tbleight_key.tbleight_unsigned32 = arg->i_idx.i_inst_ids[1];
   tbleight_key.tblfour_unsigned32 = arg->i_idx.i_inst_ids[2];

   /* Can't be NULL */

   /* The tbleight is numbered, do a get based on address */
   if((*tbleight = (SNMPTM_TBLEIGHT*) ncs_patricia_tree_get(&snmptm->tbleight_tree,
         (uns8*) &tbleight_key)) == NULL)
         return NCSCC_RC_NO_INSTANCE;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  get_next_tbleight_entry

  Description   :  Find the Instance of tbleight requested

  Arguments     :  SNMPTM_CB     *snmptm,
                   NCSMIB_ARG  *arg,
                   uns32      *io_next_inst_id,
                   SNMPTM_TBLEIGHT   **tbleight

  Return Values :  NCSCC_RC_SUCCESSS /
                   NCSCC_RC_FAILURE /
                   NCSCC_RC_NO_INSTANCE

  Notes         :
****************************************************************************/
uns32 get_next_tbleight_entry(SNMPTM_CB *snmptm,
                            NCSMIB_ARG  *arg,
                            uns32  *io_next_inst_id,
                            SNMPTM_TBLEIGHT  **tbleight)
{
   SNMPTM_TBLEIGHT_KEY  tbleight_key;

   memset(&tbleight_key, '\0', sizeof(tbleight_key));

   if ((arg->i_idx.i_inst_len == 0) ||
       (arg->i_idx.i_inst_ids == NULL))
   {
      /* First tbleight entry is requested */
      if ((*tbleight = (SNMPTM_TBLEIGHT*)ncs_patricia_tree_getnext(&snmptm->tbleight_tree,
                                                      (uns8*)0)) == NULL)
         return NCSCC_RC_NO_INSTANCE;
   }
   else
   {
      if (arg->i_idx.i_inst_len > SNMPTM_TBLEIGHT_TBL_INST_LEN)
         return NCSCC_RC_NO_INSTANCE;

      /* Prepare the tbleight key from the instance ID */
      tbleight_key.ifIndex = arg->i_idx.i_inst_ids[0];
      if (arg->i_idx.i_inst_len ==1)
      {
         tbleight_key.tbleight_unsigned32 = 0;
         tbleight_key.tblfour_unsigned32 = 0;
      }
      else if (arg->i_idx.i_inst_len ==2)
      {
         tbleight_key.tbleight_unsigned32 = arg->i_idx.i_inst_ids[1];
         tbleight_key.tblfour_unsigned32 = 0;
      }
      else
      {
         tbleight_key.tbleight_unsigned32 = arg->i_idx.i_inst_ids[1];
         tbleight_key.tblfour_unsigned32 = arg->i_idx.i_inst_ids[2];
      }
      

      /* Can't be NULL */

      /* The tbleight is numbered, do a getnext based on address */
      if (((*tbleight = (SNMPTM_TBLEIGHT*) ncs_patricia_tree_getnext(&snmptm->tbleight_tree,
            (uns8*) &tbleight_key)) == NULL))
         return NCSCC_RC_NO_INSTANCE;
   }

   /* Make an instance id from tbleight_key */
   io_next_inst_id[0] = (*tbleight)->tbleight_key.ifIndex;
   io_next_inst_id[1] = (*tbleight)->tbleight_key.tbleight_unsigned32;
   io_next_inst_id[2] = (*tbleight)->tbleight_key.tblfour_unsigned32;
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstesttableeightentry_get 
  
  Description   :  Get function for SNMPTM tbleight table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get 
                                       request will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32  ncstesttableeightentry_get(NCSCONTEXT cb,
                                NCSMIB_ARG *arg,
                                NCSCONTEXT* data)
{
   SNMPTM_CB     *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLEIGHT *tbleight = SNMPTM_TBLEIGHT_NULL;
   uns32         ret_code = NCSCC_RC_SUCCESS;


   m_NCS_CONS_PRINTF("\nncsTestTableEightEntry:  Received SNMP GET request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg);

   if((ret_code = get_tbleight_entry(snmptm, arg, &tbleight)) != NCSCC_RC_SUCCESS)
       return ret_code;

   *data = tbleight;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstesttableeightentry_next
  
  Description   :  Next function for SNMPTM tbleight table 
 
  Arguments     :  NCSCONTEXT snmptm,   -  SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,     -  MIB argument (input)
                   NCSCONTEXT* data     -  pointer to the data from which get
                                           request will be serviced
                   uns32* next_inst_id  -  instance id of the object

  Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttableeightentry_next(NCSCONTEXT snmptm,
                                NCSMIB_ARG *arg,
                                NCSCONTEXT* data,
                                uns32* next_inst_id,
                                uns32* next_inst_id_len)
{
   SNMPTM_TBLEIGHT *tbleight = SNMPTM_TBLEIGHT_NULL;
   uns32         ret_code = NCSCC_RC_SUCCESS;


   m_NCS_CONS_PRINTF("\nncsTestTableEightEntry:  Received SNMP NEXT request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg);

   if ((ret_code = get_next_tbleight_entry(snmptm, arg, next_inst_id, &tbleight))
       != NCSCC_RC_SUCCESS)
       return ret_code;

   *data = tbleight;
   *next_inst_id_len = SNMPTM_TBLEIGHT_TBL_INST_LEN;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstesttableeightentry_set
  
  Description   :  Set function for SNMPTM tbleight table objects 
 
  Arguments     :  NCSCONTEXT snmptm         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg           - MIB argument (input)
                   NCSMIB_VAR_INFO* var_info - Object properties/info(i)
                   NCS_BOOL test_flag        - set/test operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttableeightentry_set(NCSCONTEXT cb,
                               NCSMIB_ARG *arg,
                               NCSMIB_VAR_INFO *var_info,
                               NCS_BOOL test_flag)
{
   SNMPTM_CB           *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLEIGHT       *tbleight = SNMPTM_TBLEIGHT_NULL;
   SNMPTM_TBLEIGHT_KEY      tbleight_key;
   NCS_BOOL            val_same_flag = FALSE;
   NCSMIB_SET_REQ      *i_set_req = &arg->req.info.set_req;
   uns32               rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  temp_mib_req;
   uns8                create_flag = FALSE;


   m_NCS_CONS_PRINTF("\nncsTestTableEightEntry:  Received SNMP SET request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg);

   memset(&tbleight_key, '\0', sizeof(SNMPTM_TBLEIGHT_KEY));
   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   /* Prepare the key from the instant ID */
   tbleight_key.ifIndex = arg->i_idx.i_inst_ids[0];
   tbleight_key.tbleight_unsigned32 = arg->i_idx.i_inst_ids[1];
   tbleight_key.tblfour_unsigned32 = arg->i_idx.i_inst_ids[2];
   
   /* Check whether the entry exists in TBLEIGHT table with the same key?? */
   tbleight = (SNMPTM_TBLEIGHT*)ncs_patricia_tree_get(&snmptm->tbleight_tree,
                                                  (uns8*)&tbleight_key);

   /* Validate row status */
   if (i_set_req->i_param_val.i_param_id == ncsTestTableEightRowStatus_ID)
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
      temp_mib_req.info.i_val_status_util_info.row = tbleight;

      /* call the mib routine handler */
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS)
      {
         return rc;
      }
   }

   /* If the corresponding entry is not exists, and if the object is of
      read-create type then create a tbleight entry. */
   if ((tbleight == NULL) && (test_flag != TRUE))
   {
      if (var_info->access == NCSMIB_ACCESS_READ_CREATE)
      {
         if ((i_set_req->i_param_val.i_param_id != ncsTestTableEightRowStatus_ID) ||
             ((i_set_req->i_param_val.i_param_id == ncsTestTableEightRowStatus_ID)
              && (create_flag != FALSE)))
         {
             tbleight = snmptm_create_tbleight_entry(snmptm, &tbleight_key);

             /* Not able to create a tbleight entry */
             if (tbleight == NULL)
               return NCSCC_RC_FAILURE;
         }
      }
      else
         return NCSCC_RC_NO_INSTANCE;
   }


   /* All the tests have been done now set the value */
   if (test_flag != TRUE)
   {
      if (tbleight == NULL)
          return NCSCC_RC_NO_INSTANCE;

      if ((i_set_req->i_param_val.i_param_id == ncsTestTableEightRowStatus_ID)
           && (i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_DESTROY))
      {
         /* Delete the entry from the TBLEIGHT */
         snmptm_delete_tbleight_entry(snmptm, tbleight);
      }
      else
      {
         memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

         temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP;
         temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
         temp_mib_req.info.i_set_util_info.var_info = var_info;
         temp_mib_req.info.i_set_util_info.data = tbleight;
         temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

         /* call the mib routine handler */
         if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS)
         {
            return rc;
         }
      }
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstesttableeightentry_setrow
  
  Description   :  Setrow function for SNMPTM tbleight table 
 
  Arguments     :  NCSCONTEXT snmptm,         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,           - MIB argument (input)
                   NCSMIB_SETROW_PARAM_VAL    - array of params to be set
                   ncsmib_obj_info* obj_info, - Object and table properties/info(i)
                   NCS_BOOL test_flag         - setrow/testrow operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttableeightentry_setrow(NCSCONTEXT cb,
                                  NCSMIB_ARG* arg,
                                  NCSMIB_SETROW_PARAM_VAL* tbleight_param,
                                  struct ncsmib_obj_info* obj_info,
                                  NCS_BOOL testrow_flag)
{
   SNMPTM_CB           *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLEIGHT       *tbleight = SNMPTM_TBLEIGHT_NULL;
   SNMPTM_TBLEIGHT_KEY   tbleight_key;
   NCSMIBLIB_REQ_INFO  temp_mib_req;


   m_NCS_CONS_PRINTF("\nncsTestTableEightEntry: Received SNMP SETROW request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg);

   memset(&tbleight_key, '\0', sizeof(SNMPTM_TBLEIGHT_KEY));
   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   /* Prepare the key from the instant ID */
   tbleight_key.ifIndex = arg->i_idx.i_inst_ids[0];
   tbleight_key.tbleight_unsigned32 = arg->i_idx.i_inst_ids[1];
   tbleight_key.tblfour_unsigned32 = arg->i_idx.i_inst_ids[2];

   /* Check whether the entry exists in TBLEIGHT table with the same key?? */
   tbleight = (SNMPTM_TBLEIGHT*)ncs_patricia_tree_get(&snmptm->tbleight_tree,
                                                  (uns8*)&tbleight_key);

   /* Validate row status */
   if (tbleight_param[ncsTestTableEightRowStatus_ID - 1].set_flag == TRUE)
   {
      uns32  rc = NCSCC_RC_SUCCESS;

      temp_mib_req.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP;
      temp_mib_req.info.i_val_status_util_info.row_status =
             &(tbleight_param[ncsTestTableEightRowStatus_ID - 1].param.info.i_int);
      temp_mib_req.info.i_val_status_util_info.row = tbleight;

      /* call the mib routine handler */
      if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS)
      {
         return rc;
      }
   }
   /* If the corresponding entry is not exists, and if the object is of
      read-create type then create a tbleight entry. */
   if (tbleight == NULL)
   {
      if (tbleight_param[ncsTestTableEightRowStatus_ID - 1].set_flag == TRUE)
      {
         tbleight = snmptm_create_tbleight_entry(snmptm, &tbleight_key);

         /* Not able to create a tbleight entry */
         if (tbleight == NULL)
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

      for(param_id = ncsTestTableEightRowStatus_ID;
          param_id < ncsTestTableEightEntryMax_ID;
          param_id++)
          {
             if (tbleight_param[param_id - 1].set_flag == TRUE)
             {
                 memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

                 temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP;
                 temp_mib_req.info.i_set_util_info.param = &(tbleight_param[param_id - 1].param);
                 temp_mib_req.info.i_set_util_info.var_info = &obj_info->var_info[param_id-1];
                 temp_mib_req.info.i_set_util_info.data = tbleight;
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
  Name          : ncstesttableeightentry_extract 
  
  Description   :  Get function for SNMPTM tbleight table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get request
                                       will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttableeightentry_extract(NCSMIB_PARAM_VAL *param, 
                                    NCSMIB_VAR_INFO  *var_info,
                                    NCSCONTEXT data,
                                    NCSCONTEXT buffer)
{
   return ncsmiblib_get_obj_val(param, var_info, data, buffer);
}

uns32 ncstesttableeightentry_verify_instance (NCSMIB_ARG *mib_arg)
{
   return 1;
}

#endif


