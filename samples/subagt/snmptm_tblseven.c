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

  MODULE NAME: SNMPTM_TBLSEVEN.C 

..............................................................................

  DESCRIPTION:  Defines the APIs to add/del SNMPTM TBLSEVEN entries and the 
                procedure that does the clean-up of SNMPTM tblseven list.
..............................................................................


******************************************************************************/

#include "snmptm.h"

#if (NCS_SNMPTM == 1)

/*********************** function protoypes ********************************/
static SNMPTM_TBLSEVEN* snmptm_tblseven_find(SNMPTM_TBLSEVEN** head, NCSMIB_ARG *arg);
static SNMPTM_TBLSEVEN* snmptm_tblseven_find_next(SNMPTM_TBLSEVEN** head, NCSMIB_ARG *arg);


static SNMPTM_TBLSEVEN*
snmptm_tblseven_register_avsv_strs(SNMPTM_CB   *snmptm, char *str1, char *str2); 

static SNMPTM_TBLSEVEN*
snmptm_tblseven_register_avsv_strs(SNMPTM_CB   *snmptm, char *str1, char *str2) 
{

   uns32  i, j, inst_len, str2_len;
   uns32  i_exact_idx[128] = {0};
   SNMPTM_TBLSEVEN  *tblseven = NULL;
   NCSOAC_SS_ARG  mab_arg;

   /* copy the index filed */ 
   i_exact_idx[0] = strlen(str1); 
   for (i=0; i<i_exact_idx[0]; i++)
      i_exact_idx[i+1] = (uns32) str1[i];

   i = i+1; 
   i_exact_idx[i] = str2_len = strlen(str2); 
   for (j=0; j<str2_len; j++)
      i_exact_idx[i+j+1] = (uns32) str2[j];

   inst_len = 1+i_exact_idx[0]+1+str2_len; 
      
   /* There shall be some data forwarded from the staging area to 
      put it into the data structure.  Data can be send from the 
      staging area along with the MOVEROW request. -- TBD */

   m_NCS_OS_MEMSET(&mab_arg,0,sizeof(mab_arg));

   mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
   mab_arg.i_oac_hdl = snmptm->oac_hdl;
   mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSEVEN;

   /* register the exact filter type */ 
   mab_arg.info.row_owned.i_fltr.type =  NCSMAB_FLTR_EXACT;

   mab_arg.info.row_owned.i_fltr.fltr.exact.i_bgn_idx = 0; 
   mab_arg.info.row_owned.i_fltr.fltr.exact.i_idx_len = inst_len;
   mab_arg.info.row_owned.i_fltr.fltr.exact.i_exact_idx = i_exact_idx;

   if (ncsoac_ss(&mab_arg) == NCSCC_RC_FAILURE)
   {
      m_NCS_CONS_PRINTF("snmptm_tblseven_register_avsv_strs(): ncsoac_ss() failed to register a row.\n");
      return NULL;
   }

   tblseven = m_MMGR_ALLOC_SNMPTM_TBLSEVEN;
   if (tblseven == NULL)
   {
        return NULL; 
   }

   m_NCS_OS_MEMSET(tblseven, 0, sizeof(SNMPTM_TBLSEVEN));
   /* get the index into the entry */ 
   for (i=0; i<inst_len; i++)
       tblseven->string[i] = (uns8)i_exact_idx[i]; 
   tblseven->string[inst_len] = '\0';

   tblseven->length = inst_len; 
   tblseven->status = NCSMIB_ROWSTATUS_ACTIVE;

   return tblseven; 
}
/****************************************************************************
  Name          :  snmptm_tblseven_tbl_req
  
  Description   :  High Level MIB Access Routine (Request function)for SNMPTM 
                   TBLSEVEN table.
 
  Arguments     :  struct ncsmib_arg*
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_tblseven_tbl_req(struct ncsmib_arg *args)
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


/****************************************************************************
  Name          :  snmptm_oac_tblseven_register
  
  Description   :  Register the table (TBLSEVEN) with OAC. 
 
  Arguments     :  snmptm - pointer to the SNMPMTM CB.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_oac_tblseven_register(SNMPTM_CB *snmptm)
{
   NCSOAC_SS_ARG       mab_arg;

   /* register the table */
   m_NCS_OS_MEMSET(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
  
   /* register the tblseven table with the OAC */
   mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
   mab_arg.i_oac_hdl = snmptm->oac_hdl;
   mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSEVEN;
   mab_arg.info.tbl_owned.i_mib_req = snmptm_tblseven_tbl_req;
   mab_arg.info.tbl_owned.i_mib_key = snmptm->hmcb_hdl;
   mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_SNMPTM;
            
   if (ncsoac_ss(&mab_arg) == NCSCC_RC_FAILURE)
   {
      m_NCS_CONS_PRINTF("snmptm_oac_tblseven_register(): ncsoac_ss() failed to register a table.\n");
      return NCSCC_RC_FAILURE;
   }

   /* Only for SNMPTM_VCARD_ID1, ROW owned registration should happen. */
   if(m_MDS_GET_VDEST_ID_FROM_MDS_DEST(snmptm->vcard) != SNMPTM_VCARD_ID1) 
      return NCSCC_RC_SUCCESS;

    /* register the exact filter filter with OAA */ 

    snmptm->tblseven = snmptm_tblseven_register_avsv_strs(snmptm, "a", "c");
    snmptm->tblseven->next = snmptm_tblseven_register_avsv_strs(snmptm, "a", "xyz");
    snmptm->tblseven->next->next = snmptm_tblseven_register_avsv_strs(snmptm, "abc", "x");

    return NCSCC_RC_SUCCESS;
}
 
  
/*****************************************************************************
  Name          :  snmptm_tblseven_find
  
  Description   :  Search & retrieve the tblfor node from the tblseven list.
 
  Arguments     :  head - pointer to the tblseven head node of the tblseven list.
                   index - index value of the tblseven to be retrieved from the 
                           tblseven list.
    
  Return Values :  ret - pointer to the TBLSEVEN node to be removed. 
 
  Notes         : 
*****************************************************************************/
SNMPTM_TBLSEVEN* snmptm_tblseven_find(SNMPTM_TBLSEVEN** head, NCSMIB_ARG *arg)
{
   SNMPTM_TBLSEVEN *entry;
   SNMPTM_TBLSEVEN *ret = NULL;
   uns8 string[128] = {0};

   /* If index mentioned, get it */
   if ((arg->i_idx.i_inst_len != 0) &&
       (arg->i_idx.i_inst_ids != NULL))
   {
      uns32 i; 

      for (i=0; i<arg->i_idx.i_inst_len; i++)
         string[i] = (uns8)(*((arg->i_idx.i_inst_ids)+i)); 
      string[i] = '\0';
   }

   if (head == NULL)
   {
      return ((SNMPTM_TBLSEVEN*)NULL);
   }
   
   entry = *head;

   if (entry != NULL)
   {
      if (strcmp(entry->string, string) == 0)
      {
         return entry;
      }
      else
     {
         while(entry->next != NULL)
         {
            if (strcmp(entry->next->string, string) == 0)
            {
               ret = entry->next;
               return ret;
            }
            entry = entry->next;
         }
     } 
   }
  
   return ret;
}

SNMPTM_TBLSEVEN* snmptm_tblseven_find_next(SNMPTM_TBLSEVEN** head, NCSMIB_ARG *arg)
{
   SNMPTM_TBLSEVEN *tblseven = NULL;
   int32           res = 0; 
   uns8 string[128] = {0};

   /* If index mentioned, get it */
   if ((arg->i_idx.i_inst_len == 0) ||
       (arg->i_idx.i_inst_ids == NULL))
   {
       return *head; 
   }

   /* If index mentioned, get it */
   if ((arg->i_idx.i_inst_len != 0) &&
       (arg->i_idx.i_inst_ids != NULL))
   {
      uns32 i; 

      for (i=0; i<arg->i_idx.i_inst_len; i++)
         string[i] = (uns8)(*((arg->i_idx.i_inst_ids)+i)); 
   }

   for(tblseven = *head; tblseven != NULL; tblseven = tblseven->next)
   {
        /* from here start finding the best place to fit, 
         * and do this till the length of the existing row is same 
         */ 
        if (tblseven->length >= arg->i_idx.i_inst_len)
        {
            /* find the best fit in this category */ 
            res = memcmp(tblseven->string, string, arg->i_idx.i_inst_len); 
            if (res > 0)
               break;
            else if ((res == 0) && (tblseven->length != arg->i_idx.i_inst_len))
                break; 
        }
   }
   return tblseven; 
}

/****************************************************************************
  Name          :  ncstesttablesevenentry_get 
  
  Description   :  Get function for SNMPTM tblseven table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get 
                                       request will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32  ncstesttablesevenentry_get(NCSCONTEXT cb, 
                                 NCSMIB_ARG *arg, 
                                 NCSCONTEXT* data)
{
   SNMPTM_CB        *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLSEVEN  *tblseven = NULL;

   /* get the node */
   tblseven =  snmptm_tblseven_find(&snmptm->tblseven, arg);
   if (tblseven == NULL)
   {
      *data = NULL; 
      return NCSCC_RC_NO_INSTANCE; 
   }

   *data = tblseven;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstesttablesevenentry_next
  
  Description   :  Next function for SNMPTM tblseven table 
 
  Arguments     :  NCSCONTEXT snmptm,   -  SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,     -  MIB argument (input)
                   NCSCONTEXT* data     -  pointer to the data from which get
                                           request will be serviced
                   uns32* next_inst_id  -  instance id of the object

  Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablesevenentry_next(NCSCONTEXT cb, 
                                 NCSMIB_ARG *arg, 
                                 NCSCONTEXT* data, 
                                 uns32* next_inst_id,
                                 uns32* next_inst_id_len) 
{
   SNMPTM_CB      *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLSEVEN *tblseven = NULL;
   uns32            i; 

   printf("\nncsTestTableSevenEntry:  Received SNMP NEXT request\n");   
   ncsmib_pp(arg); 
   
   /* If no index defined then send first entry of the list */
   if ((arg->i_idx.i_inst_len == 0) ||
       (arg->i_idx.i_inst_ids == NULL))
   {
      if (snmptm->tblseven != NULL)
      {
         *data = snmptm->tblseven;
         *next_inst_id_len = snmptm->tblseven->length;
         for (i=0; i<*next_inst_id_len; i++)
             *(next_inst_id+i) = (uns32)snmptm->tblseven->string[i]; 
             printf("==================================================================\n"); 
         return NCSCC_RC_SUCCESS;
      }
      else
      {
         *data = NULL; 
         return NCSCC_RC_NO_INSTANCE; 
      }
   }

   
   /* get the node */
   tblseven =  snmptm_tblseven_find_next(&snmptm->tblseven, arg);

   if (tblseven == NULL)
   {
      *data = NULL; 
      return NCSCC_RC_NO_INSTANCE; 
   }
   else
   {
      *data = tblseven;
      *next_inst_id_len = tblseven->length;
      for (i=0; i<*next_inst_id_len; i++)
          *(next_inst_id+i) = (uns32)tblseven->string[i]; 
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstesttablesevenentry_set
  
  Description   :  Set function for SNMPTM tblseven table objects 
 
  Arguments     :  NCSCONTEXT snmptm         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg           - MIB argument (input)
                   NCSMIB_VAR_INFO* var_info - Object properties/info(i)
                   NCS_BOOL test_flag        - set/test operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablesevenentry_set(NCSCONTEXT cb, 
                                NCSMIB_ARG *arg, 
                                NCSMIB_VAR_INFO *var_info,
                                NCS_BOOL test_flag)
{
   /* Set request should not come on this table. */
   return NCSCC_RC_FAILURE; 
}


/****************************************************************************
  Name          :  ncstesttablesevenentry_setrow
  
  Description   :  Setrow function for SNMPTM tblseven table 
 
  Arguments     :  NCSCONTEXT snmptm,         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,           - MIB argument (input)
                   NCSMIB_SETROW_PARAM_VAL    - array of params to be set
                   ncsmib_obj_info* obj_info, - Object and table properties/info(i)
                   NCS_BOOL test_flag         - setrow/testrow operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablesevenentry_setrow(NCSCONTEXT cb, 
                                   NCSMIB_ARG* arg,
                                   NCSMIB_SETROW_PARAM_VAL* tblseven_param,
                                   struct ncsmib_obj_info* obj_info,
                                   NCS_BOOL testrow_flag)
{
   /* This request shouldn;t come here. */
   return NCSCC_RC_FAILURE;
}


/****************************************************************************
  Name          : ncstesttablesevenentry_extract 
  
  Description   :  Get function for SNMPTM tblseven table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get request
                                       will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablesevenentry_extract(NCSMIB_PARAM_VAL *param, 
                                    NCSMIB_VAR_INFO  *var_info,
                                    NCSCONTEXT data,
                                    NCSCONTEXT buffer)
{
   return ncsmiblib_get_obj_val(param, var_info, data, buffer);
}

uns32 ncstesttablesevenentry_rmvrow(NCSCONTEXT cb_hdl, NCSMIB_IDX *idx)
{
   return NCSCC_RC_SUCCESS;
}

#endif


