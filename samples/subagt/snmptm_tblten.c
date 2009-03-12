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

  MODULE NAME: SNMPTM_TBLTEN.C 

..............................................................................

  DESCRIPTION:  Defines the APIs to add/del SNMPTM TBLTEN entries and the 
                procedure that does the clean-up of SNMPTM tblten list.
..............................................................................


******************************************************************************/

#include "snmptm.h"

#if (NCS_SNMPTM == 1)


#ifndef SNMPTM_TBLTEN_TBL_INST_LEN
#define SNMPTM_TBLTEN_TBL_INST_LEN 2
#endif

/* function proto-types */
static uns32 get_tblten_entry(SNMPTM_CB *snmptm,
                              NCSMIB_ARG *arg,
                              SNMPTM_TBLTEN **tblten);
static uns32 get_next_tblten_entry(SNMPTM_CB   *snmptm,
                                   NCSMIB_ARG  *arg,
                                   uns32       *io_next_inst_id,
                                   SNMPTM_TBLTEN **tblten);


/****************************************************************************
  Name          :  snmptm_tblten_tbl_req
  
  Description   :  High Level MIB Access Routine (Request function)for SNMPTM 
                   TBLTEN table.
 
  Arguments     :  struct ncsmib_arg*
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_tblten_tbl_req(struct ncsmib_arg *args)
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
  Name          :  snmptm_create_tblten_entry

  Description   :  This function creates an entry into SNMPTM TBLTEN table.
                   Basically it adds a tblten node to a tblten patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tblten_key - pointer to the TBLTEN request info struct.

  Return Values :  tblten - Upon successful addition of tblten entry
                   NULL   - Upon failure of adding tblten entry

  Notes         :
*****************************************************************************/
SNMPTM_TBLTEN *snmptm_create_tblten_entry(SNMPTM_CB *snmptm,
                                          SNMPTM_TBLTEN_KEY *tblten_key)
{

   SNMPTM_TBLTEN *tblten = NULL;

   /* Alloc the memory for TBLTEN entry */
   if ((tblten = (SNMPTM_TBLTEN*)m_MMGR_ALLOC_SNMPTM_TBLTEN)
        == SNMPTM_TBLTEN_NULL)
   {
      m_NCS_CONS_PRINTF("\nNot able to alloc the memory for TBLTEN \n");
      return NULL;
   }

   memset((char *)tblten, '\0', sizeof(SNMPTM_TBLTEN));

   /* Copy the key contents to tblten struct */
   tblten->tblten_key.tblten_unsigned32 = tblten_key->tblten_unsigned32;
   tblten->tblten_key.tblten_int = tblten_key->tblten_int;

   tblten->tblten_pat_node.key_info = (uns8 *)&((tblten->tblten_key));

   /* Add to the tblten entry/node to tblten patricia tree */
   if(NCSCC_RC_SUCCESS != ncs_patricia_tree_add(&(snmptm->tblten_tree),
                                                &(tblten->tblten_pat_node)))
   {
      m_NCS_CONS_PRINTF("\nNot able add TBLTEN node to TBLTEN tree.\n");

      /* Free the alloc memory of TBLTEN */
      m_MMGR_FREE_SNMPTM_TBLTEN(tblten);

      return NULL;
   }

   return tblten;
}

/*****************************************************************************
  Name          :  snmptm_delete_tblten_entry

  Description   :  This function deletes an entry from TBLTEN table. Basically
                   it deletes a tblten node from a tblten patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tblten - pointer to the TBLTEN struct.

  Return Values :  Nothing

  Notes         :
*****************************************************************************/
void snmptm_delete_tblten_entry(SNMPTM_CB *snmptm,
                                SNMPTM_TBLTEN *tblten)
{

   /* Delete the tblten entry from the tblten patricia tree */
   ncs_patricia_tree_del(&(snmptm->tblten_tree), &(tblten->tblten_pat_node));

   /* Free the memory of TBLTEN */
   m_MMGR_FREE_SNMPTM_TBLTEN(tblten);

   return;
}

/*****************************************************************************
  Name          :  snmptm_delete_tblten

  Description   :  This function deletes all the tblten entries (nodes) from
                   tblten tree and destroys a tblten tree.

  Arguments     :  snmptm  - Pointer to the SNMPTM CB structure

  Return Values :

  Notes         :
*****************************************************************************/
void  snmptm_delete_tblten(SNMPTM_CB *snmptm)
{
   SNMPTM_TBLTEN *tblten;

   /* Delete all the TBLTEN entries */
   while(SNMPTM_TBLTEN_NULL != (tblten = (SNMPTM_TBLTEN*)
                        ncs_patricia_tree_getnext(&snmptm->tblten_tree, 0)))
   {
       /* Delete the node from the tblten tree */
       ncs_patricia_tree_del(&snmptm->tblten_tree, &tblten->tblten_pat_node);

       /* Free the memory */
       m_MMGR_FREE_SNMPTM_TBLTEN(tblten);
   }

   /* Destroy TBLTEN tree */
   ncs_patricia_tree_destroy(&snmptm->tblten_tree);

   return;
}



/****************************************************************************
 Name          :  get_tblten_entry

 Description   :  Get the Instance of tblten requested

 Arguments     :  SNMPTM_CB     *snmptm,
                  NCSMIB_ARG    *arg,
                  SNMPTM_TBLTEN **tblten

 Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE / NCSCC_RC_NO_INSTANCE

 Notes         :
****************************************************************************/
uns32 get_tblten_entry(SNMPTM_CB  *snmptm, NCSMIB_ARG  *arg, SNMPTM_TBLTEN  **tblten)
{
   SNMPTM_TBLTEN_KEY  tblten_key;

   memset(&tblten_key, '\0', sizeof(tblten_key));

   if(arg->i_idx.i_inst_len != SNMPTM_TBLTEN_TBL_INST_LEN)
      return NCSCC_RC_FAILURE;

   if(arg->i_idx.i_inst_ids == NULL)
      return NCSCC_RC_FAILURE;

   /* Prepare the key from the instant ID */
   tblten_key.tblten_unsigned32 = arg->i_idx.i_inst_ids[0];
   tblten_key.tblten_int = arg->i_idx.i_inst_ids[1];

   /* Can't be NULL */

   /* The tblten is numbered, do a get based on address */
   if((*tblten = (SNMPTM_TBLTEN*) ncs_patricia_tree_get(&snmptm->tblten_tree,
         (uns8*) &tblten_key)) == NULL)
         return NCSCC_RC_NO_INSTANCE;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  get_next_tblten_entry

  Description   :  Find the Instance of tblten requested

  Arguments     :  SNMPTM_CB     *snmptm,
                   NCSMIB_ARG  *arg,
                   uns32      *io_next_inst_id,
                   SNMPTM_TBLTEN   **tblten

  Return Values :  NCSCC_RC_SUCCESSS /
                   NCSCC_RC_FAILURE /
                   NCSCC_RC_NO_INSTANCE

  Notes         :
****************************************************************************/
uns32 get_next_tblten_entry(SNMPTM_CB *snmptm,
                            NCSMIB_ARG  *arg,
                            uns32  *io_next_inst_id,
                            SNMPTM_TBLTEN  **tblten)
{
   SNMPTM_TBLTEN_KEY  tblten_key;

   memset(&tblten_key, '\0', sizeof(tblten_key));

   if ((arg->i_idx.i_inst_len == 0) ||
       (arg->i_idx.i_inst_ids == NULL))
   {
      /* First tblten entry is requested */
      if ((*tblten = (SNMPTM_TBLTEN*)ncs_patricia_tree_getnext(&snmptm->tblten_tree,
                                                      (uns8*)0)) == NULL)
         return NCSCC_RC_NO_INSTANCE;
   }
   else
   {
      if (arg->i_idx.i_inst_len > SNMPTM_TBLTEN_TBL_INST_LEN)
         return NCSCC_RC_NO_INSTANCE;

      /* Prepare the tblten key from the instance ID */
      tblten_key.tblten_unsigned32 = arg->i_idx.i_inst_ids[0];
      if (arg->i_idx.i_inst_len == 1)
         tblten_key.tblten_int = 0;
      else
         tblten_key.tblten_int = arg->i_idx.i_inst_ids[1];

      /* Can't be NULL */

      /* The tblten is numbered, do a getnext based on address */
      if (((*tblten = (SNMPTM_TBLTEN*) ncs_patricia_tree_getnext(&snmptm->tblten_tree,
            (uns8*) &tblten_key)) == NULL))
         return NCSCC_RC_NO_INSTANCE;
   }

   /* Make an instance id from tblten_key */
   io_next_inst_id[0] = (*tblten)->tblten_key.tblten_unsigned32;
   io_next_inst_id[1] = (*tblten)->tblten_key.tblten_int;
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstesttabletenentry_get 
  
  Description   :  Get function for SNMPTM tblten table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get 
                                       request will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32  ncstesttabletenentry_get(NCSCONTEXT cb,
                                NCSMIB_ARG *arg,
                                NCSCONTEXT* data)
{
   SNMPTM_CB     *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLTEN *tblten = SNMPTM_TBLTEN_NULL;
   uns32         ret_code = NCSCC_RC_SUCCESS;


   m_NCS_CONS_PRINTF("\nncsTestTableTenEntry:  Received SNMP GET request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg);

   if((ret_code = get_tblten_entry(snmptm, arg, &tblten)) != NCSCC_RC_SUCCESS)
       return ret_code;

   *data = tblten;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstesttabletenentry_next
  
  Description   :  Next function for SNMPTM tblten table 
 
  Arguments     :  NCSCONTEXT snmptm,   -  SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,     -  MIB argument (input)
                   NCSCONTEXT* data     -  pointer to the data from which get
                                           request will be serviced
                   uns32* next_inst_id  -  instance id of the object

  Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttabletenentry_next(NCSCONTEXT snmptm,
                                NCSMIB_ARG *arg,
                                NCSCONTEXT* data,
                                uns32* next_inst_id,
                                uns32* next_inst_id_len)
{
   SNMPTM_TBLTEN *tblten = SNMPTM_TBLTEN_NULL;
   uns32         ret_code = NCSCC_RC_SUCCESS;


   m_NCS_CONS_PRINTF("\nncsTestTableTenEntry:  Received SNMP NEXT request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg);

   if ((ret_code = get_next_tblten_entry(snmptm, arg, next_inst_id, &tblten))
       != NCSCC_RC_SUCCESS)
       return ret_code;

   *data = tblten;
   *next_inst_id_len = SNMPTM_TBLTEN_TBL_INST_LEN;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstesttabletenentry_set
  
  Description   :  Set function for SNMPTM tblten table objects 
 
  Arguments     :  NCSCONTEXT snmptm         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg           - MIB argument (input)
                   NCSMIB_VAR_INFO* var_info - Object properties/info(i)
                   NCS_BOOL test_flag        - set/test operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttabletenentry_set(NCSCONTEXT cb,
                               NCSMIB_ARG *arg,
                               NCSMIB_VAR_INFO *var_info,
                               NCS_BOOL test_flag)
{
   SNMPTM_CB           *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLTEN       *tblten = SNMPTM_TBLTEN_NULL;
   SNMPTM_TBLTEN_KEY      tblten_key;
   NCS_BOOL            val_same_flag = FALSE;
   NCSMIB_SET_REQ      *i_set_req = &arg->req.info.set_req;
   uns32               rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  temp_mib_req;
   /*uns8                create_flag = FALSE;*/


   m_NCS_CONS_PRINTF("\nncsTestTableTenEntry:  Received SNMP SET request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg);

   memset(&tblten_key, '\0', sizeof(SNMPTM_TBLTEN_KEY));
   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   /* Prepare the key from the instant ID */
   tblten_key.tblten_unsigned32 = arg->i_idx.i_inst_ids[0];
   tblten_key.tblten_int = arg->i_idx.i_inst_ids[1];
   
   /* Check whether the entry exists in TBLTEN table with the same key?? */
   tblten = (SNMPTM_TBLTEN*)ncs_patricia_tree_get(&snmptm->tblten_tree,
                                                  (uns8*)&tblten_key);

   /* If the corresponding entry is not exists, and if the object is of
      read-create type then create a tblten entry. */
   if ((tblten == NULL) && (test_flag != TRUE))
   {
      if((var_info->access == NCSMIB_ACCESS_READ_CREATE) ||
         ((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) == NCSMIB_POLICY_PSS_BELIEVE_ME))
      {
          tblten = snmptm_create_tblten_entry(snmptm, &tblten_key);

          /* Not able to create a tblten entry */
          if (tblten == NULL)
             return NCSCC_RC_FAILURE;
      }
      else
         return NCSCC_RC_NO_INSTANCE;
   }


   /* All the tests have been done now set the value */
   if (test_flag != TRUE)
   {
      if (tblten == NULL)
          return NCSCC_RC_NO_INSTANCE;

      memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP;
      temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = tblten;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS)
      {
         return rc;
      }
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstesttabletenentry_setrow
  
  Description   :  Setrow function for SNMPTM tblten table 
 
  Arguments     :  NCSCONTEXT snmptm,         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,           - MIB argument (input)
                   NCSMIB_SETROW_PARAM_VAL    - array of params to be set
                   ncsmib_obj_info* obj_info, - Object and table properties/info(i)
                   NCS_BOOL test_flag         - setrow/testrow operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttabletenentry_setrow(NCSCONTEXT cb,
                                  NCSMIB_ARG* arg,
                                  NCSMIB_SETROW_PARAM_VAL* tblten_param,
                                  struct ncsmib_obj_info* obj_info,
                                  NCS_BOOL testrow_flag)
{
   SNMPTM_CB           *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLTEN       *tblten = SNMPTM_TBLTEN_NULL;
   SNMPTM_TBLTEN_KEY   tblten_key;
   NCSMIBLIB_REQ_INFO  temp_mib_req;


   m_NCS_CONS_PRINTF("\nncsTestTableTenEntry: Received SNMP SETROW request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg);

   memset(&tblten_key, '\0', sizeof(SNMPTM_TBLTEN_KEY));
   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   /* Prepare the key from the instant ID */
   tblten_key.tblten_unsigned32 = arg->i_idx.i_inst_ids[0];
   tblten_key.tblten_int = arg->i_idx.i_inst_ids[1];

   /* Check whether the entry exists in TBLTEN table with the same key?? */
   tblten = (SNMPTM_TBLTEN*)ncs_patricia_tree_get(&snmptm->tblten_tree,
                                                  (uns8*)&tblten_key);

   /* If the corresponding entry is not exists, and if the object is of
      read-create type then create a tblten entry. */
   if (tblten == NULL)
   {
      if((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) == NCSMIB_POLICY_PSS_BELIEVE_ME)
      {
         tblten = snmptm_create_tblten_entry(snmptm, &tblten_key);

         /* Not able to create a tblten entry */
         if (tblten == NULL)
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

      for(param_id = ncsTestTableTenUnsigned32_ID;
          param_id < ncsTestTableTenEntryMax_ID;
          param_id++)
          {
             if (tblten_param[param_id - 1].set_flag == TRUE)
             {
                 memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

                 temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP;
                 temp_mib_req.info.i_set_util_info.param = &(tblten_param[param_id - 1].param);
                 temp_mib_req.info.i_set_util_info.var_info = &obj_info->var_info[param_id-1];
                 temp_mib_req.info.i_set_util_info.data = tblten;
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
  Name          : ncstesttabletenentry_extract 
  
  Description   :  Get function for SNMPTM tblten table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get request
                                       will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttabletenentry_extract(NCSMIB_PARAM_VAL *param, 
                                    NCSMIB_VAR_INFO  *var_info,
                                    NCSCONTEXT data,
                                    NCSCONTEXT buffer)
{
   return ncsmiblib_get_obj_val(param, var_info, data, buffer);
}

/****************************************************************************
  Name          :  ncstesttabletenentry_rmvrow

  Description   :  Remove-row function handler for SNMPTM tblten table

  Arguments     :  NCSCONTEXT snmptm,         - SNMPTM control block (input/output)
                   NCSMIB_IDX *idx,           - Row MIB index to be deleted(input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

  Notes         :
****************************************************************************/
uns32 ncstesttabletenentry_rmvrow(NCSCONTEXT cb_hdl, NCSMIB_IDX *idx)
{
   SNMPTM_CB           *snmptm = SNMPTM_CB_NULL; 
   SNMPTM_TBLTEN      *tblten = SNMPTM_TBLTEN_NULL; 
   SNMPTM_TBLTEN_KEY      tblten_key;
   uns32               status = NCSCC_RC_SUCCESS; 

   /* Get the CB from the handle manager */
   if ((snmptm = (SNMPTM_CB *)cb_hdl) == NULL)
   {
      return NCSCC_RC_FAILURE;
   }
   
   /* Now, delete the row from the tblten */
   memset(&tblten_key, '\0', sizeof(SNMPTM_TBLTEN_KEY));
 
   /* Prepare the key from the instant ID */
   tblten_key.tblten_unsigned32 = idx->i_inst_ids[0];
   tblten_key.tblten_int = idx->i_inst_ids[1];

   /* Check whether the entry exists in TBLTEN table with the same key?? */
   tblten = (SNMPTM_TBLTEN*)ncs_patricia_tree_get(&snmptm->tblten_tree,
                                                      (uns8*)&tblten_key);
   if(tblten != NULL)
   {
      snmptm_delete_tblten_entry(snmptm, tblten);
   }
   else
   {
      status = NCSCC_RC_FAILURE;
   }

   return status;
}

#endif


