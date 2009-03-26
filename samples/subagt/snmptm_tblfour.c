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

  MODULE NAME: SNMPTM_TBLFOUR.C 

..............................................................................

  DESCRIPTION:  Defines the APIs to add/del SNMPTM TBLFOUR entries and the 
                procedure that does the clean-up of SNMPTM tblfour list.
..............................................................................


******************************************************************************/

#include "snmptm.h"

#if (NCS_SNMPTM == 1)

/*********************** function protoypes ********************************/
static uns32 snmptm_tblfour_staging_area_handler(SNMPTM_CB* snmptm,NCSMIB_ARG* arg);
static uns32 snmptm_tblfour_moverow_rsp_handler(struct ncsmib_arg* rsp);
static uns32 snmptm_oac_cb(NCSOAC_SS_CB_ARG* cbarg);
static uns32 ncstesttablefourentry_moverow_reg(SNMPTM_CB* snmptm, struct ncsmib_arg *mib_arg, NCSCONTEXT *row_hdl);
static uns32 ncstesttablefourentry_moverow_unreg(SNMPTM_CB* snmptm, struct ncsmib_arg *mib_arg, NCSCONTEXT row_hdl);
static uns32 ncstesttablefourentry_moverow(SNMPTM_CB* snmptm, struct ncsmib_arg *mib_arg, USRBUF *buff);
static void  snmptm_tblfour_put(SNMPTM_TBLFOUR** head, SNMPTM_TBLFOUR* new_entry);
static SNMPTM_TBLFOUR* snmptm_tblfour_rmv(SNMPTM_TBLFOUR** head, uns32 index);
static SNMPTM_TBLFOUR* snmptm_tblfour_find(SNMPTM_TBLFOUR** head, NCSMIB_ARG *arg);


NCS_BOOL snmptm_finish_unreg = FALSE;
NCS_BOOL snmptm_idx_deleted = FALSE;
static NCS_BOOL move_row_flg = FALSE;

/****************************************************************************
  Name          :  snmptm_tblfour_tbl_req
  
  Description   :  High Level MIB Access Routine (Request function)for SNMPTM 
                   TBLONE table.
 
  Arguments     :  struct ncsmib_arg*
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_tblfour_tbl_req(struct ncsmib_arg *args)
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

   if (args->i_op != NCSMIB_OP_REQ_MOVEROW)
   {
      miblib_req.info.i_mib_op_info.args = args; 
       
      m_SNMPTM_LOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);
       
      /* call the miblib routine handler */ 
      status = ncsmiblib_process_req(&miblib_req); 

      m_SNMPTM_UNLOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);
   }
   else
   {
      printf("SNMPTM TBLFOUR: Received MOVEROW request \n");
      
      move_row_flg = TRUE;
       
      /* MOVEROW is the same of the SETROW operation, except MOVEROW will have 
         an extra functionality of REGISTERING ROW with OAC */
      args->i_op = NCSMIB_OP_REQ_MOVEROW; 
      args->req.info.setrow_req.i_usrbuf = args->req.info.moverow_req.i_usrbuf;
      miblib_req.info.i_mib_op_info.args = args; 
       
      m_SNMPTM_LOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);
       
      /* call the miblib routine handler */ 
      status = ncsmiblib_process_req(&miblib_req); 

      m_SNMPTM_UNLOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);

      move_row_flg = FALSE;
   }

   /* Release SNMPTM CB handle */
   ncshm_give_hdl(cb_hdl);
   
   return status; 
}


/****************************************************************************
  Name          :  snmptm_oac_tblfour_register
  
  Description   :  Register the table (TBLFOUR) with OAC. 
 
  Arguments     :  snmptm - pointer to the SNMPMTM CB.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_oac_tblfour_register(SNMPTM_CB *snmptm)
{
   NCSOAC_SS_ARG       mab_arg;

   /* register the table */
   memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
  
   /* register the tblfour table with the OAC */
   mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
   mab_arg.i_oac_hdl = snmptm->oac_hdl;
   mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLFOUR;
   mab_arg.info.tbl_owned.i_mib_req = snmptm_tblfour_tbl_req;
   mab_arg.info.tbl_owned.i_mib_key = snmptm->hmcb_hdl;
   mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_SNMPTM;
            
   if (ncsoac_ss(&mab_arg) == NCSCC_RC_FAILURE)
   {
      printf("snmptm_oac_tblfour_register(): ncsoac_ss() failed to register a table.\n");
      return NCSCC_RC_FAILURE;
   }

   /* Only for SNMPTM_VCARD_ID1, ROW owned registration should happen. */
   if (m_MDS_GET_VDEST_ID_FROM_MDS_DEST(snmptm->vcard) != SNMPTM_VCARD_ID1) 
      return NCSCC_RC_SUCCESS;
  
   /* register the DEFAULT filter */
   memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
  
   mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
   mab_arg.i_oac_hdl = snmptm->oac_hdl;
   mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLFOUR;
  
   mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_DEFAULT;
   mab_arg.info.row_owned.i_fltr.fltr.def.meaningless = 0x111;

   mab_arg.info.row_owned.i_ss_cb = snmptm_oac_cb;
   mab_arg.info.row_owned.i_ss_hdl = NCS_PTR_TO_UNS64_CAST(snmptm);

   if (ncsoac_ss(&mab_arg) == NCSCC_RC_FAILURE)
   {
      printf("snmptm_oac_tblfour_register(): ncsoac_ss() failed to register a row.\n");
      return NCSCC_RC_FAILURE;
   }
   
   return NCSCC_RC_SUCCESS;
}
 
  
/*****************************************************************************
  Name          :  ncstesttablefourentry_moverow
  
  Description   :  This function sends a MOVEROW request from SNMPTM_VCARD_ID1
                   to SNMPTM_VCARD_ID2.
 
  Arguments     :  snmptm - pointer to the SNMPMTM CB.
                   mib_arg - Pointer to the MIB_ARG struct.
                   buff - pointer to the USRBUF, which contains the ROW data 
                          to be moved.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
*****************************************************************************/
static uns32 ncstesttablefourentry_moverow(SNMPTM_CB* snmptm, 
                                           struct ncsmib_arg *mib_arg,
                                           USRBUF *buff)
{
   NCSOAC_SS_ARG ssarg;
   NCSMIB_ARG    marg;
   uns32         idx_inst[1] = {0};
   NCS_BOOL      moverow_self = SNMPTM_MOVEROW_SELF;
          
           
   memset(&ssarg, 0, sizeof(ssarg));
   memset(&marg, 0, sizeof(marg));

   idx_inst[0] = mib_arg->i_idx.i_inst_ids[0];

   ncsmib_init(&marg);

   marg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLFOUR;
   marg.i_usr_key = snmptm->hmcb_hdl; /* doesn't matter ? */
   marg.i_mib_key = snmptm->oac_hdl;  /* doesn't matter ? */
   marg.i_op  = NCSMIB_OP_REQ_MOVEROW;
   marg.i_idx.i_inst_ids = idx_inst;
   marg.i_idx.i_inst_len = 1;
   marg.i_rsp_fnc = snmptm_tblfour_moverow_rsp_handler;
   
   /* Compose the buffer  if the staging area wants to send some parameter info */
   marg.req.info.moverow_req.i_usrbuf = buff;
 
   /* In this application, if moverow_self is set to TRUE then we will
    * be not sending any info to the destination OAC (sent to self OAC).
    * If moverow_self set to FALSE then  we will be sending any info to the
    * destination OAC viz.. from VCARD_ID1 to VCARD_ID2.
    *
    * Functionality will be decided at compilation time through SNMPTM_MOVEROW_SELF 
    * macro. 
    */
   if (moverow_self)
      marg.req.info.moverow_req.i_move_to = snmptm->vcard;
   else
      m_NCS_SET_VDEST_ID_IN_MDS_DEST(marg.req.info.moverow_req.i_move_to, SNMPTM_VCARD_ID2);

   ssarg.i_op = NCSOAC_SS_OP_ROW_MOVE;
   ssarg.i_oac_hdl = snmptm->oac_hdl; 
   ssarg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLFOUR;
   ssarg.info.row_move.i_move_req = &marg;

   if (ncsoac_ss(&ssarg) == NCSCC_RC_FAILURE)
   {
      printf("ncstesttablefourentry_moverow(): ncsoac_ss() failed MOVEROW.\n");
      return NCSCC_RC_FAILURE;
   }

   printf("SNMPTM TBLFOUR: MOVEROW request has been sent\n");

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  Name          :  snmptm_tblfour_staging_area_handler
  
  Description   :  Staging area for the table four
 
  Arguments     :  snmptm - pointer to the SNMPMTM CB.
                   mib_arg - Pointer to the MIB_ARG struct.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
*****************************************************************************/
static uns32 snmptm_tblfour_staging_area_handler(SNMPTM_CB* snmptm,
                                                 NCSMIB_ARG* arg)
{
   NCSMIBLIB_REQ_INFO  miblib_req; 
   uns32               status = NCSCC_RC_SUCCESS;


   printf("snmptm_tblfour_staging_area_handler(): Arrived\n");

   switch(arg->i_op)
   {
     /* handle only SET requests... (for MOVEROW demonstration ...) */
   case NCSMIB_OP_REQ_SET:
   case NCSMIB_OP_REQ_TEST:
   case NCSMIB_OP_REQ_GET:
   case NCSMIB_OP_REQ_NEXT:
   case NCSMIB_OP_REQ_SETROW:
   case NCSMIB_OP_REQ_TESTROW:
       memset(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

       miblib_req.req = NCSMIBLIB_REQ_MIB_OP; 
       miblib_req.info.i_mib_op_info.args = arg; 
       miblib_req.info.i_mib_op_info.cb = snmptm; 
       
       m_SNMPTM_LOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);
       
       /* call the miblib routine handler */ 
       status = ncsmiblib_process_req(&miblib_req); 

       m_SNMPTM_UNLOCK(&snmptm->snmptm_cb_lock, NCS_LOCK_READ);
       break;
       
   default:
       arg->i_op = m_NCSMIB_REQ_TO_RSP(arg->i_op);
       arg->rsp.i_status = NCSCC_RC_FAILURE;
       arg->i_rsp_fnc(arg);
       break;
   }

   return status;
}


/*****************************************************************************
  Name          :  snmptm_tblfour_moverow_rsp_handler
  
  Description   :  Response handler function for the sent MOVEROW request
 
  Arguments     :  rsp - Pointer to the MIB_ARG struct.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
*****************************************************************************/
static uns32 snmptm_tblfour_moverow_rsp_handler(struct ncsmib_arg* rsp)
{
   printf("\n\nsnmptm_tblfour_moverow_rsp_handler():rsp status:%s.\n",rsp->rsp.i_status == NCSCC_RC_SUCCESS ? "SUCCESS" : "FAILURE");
   if (rsp->rsp.info.moverow_rsp.i_usrbuf != NULL)
   { 
      /*
      m_MMGR_FREE_BUFR_LIST(rsp->rsp.info.moverow_rsp.i_usrbuf);
      */
      rsp->rsp.info.moverow_rsp.i_usrbuf = NULL;
   }
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  Name          :  snmptm_oac_cb
  
  Description   :  Callback function to be supplied when a default filter is
                   registered
 
  Arguments     :  cbarg - Pointer to the NCSOAC_SS_CB__ARG struct.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
*****************************************************************************/
static uns32 snmptm_oac_cb(NCSOAC_SS_CB_ARG* cbarg)
{
   SNMPTM_CB  *snmptm;

   if ((snmptm = (SNMPTM_CB*)NCS_INT64_TO_PTR_CAST(cbarg->i_ss_hdl)) == NULL)
      return NCSCC_RC_FAILURE;

   switch(cbarg->i_op)
    {
    case NCSOAC_SS_CB_OP_IDX_FREE:
        printf("\nsnmptm_oac_cb():deallocated index:%u (tbl:%u)\n",
                           cbarg->info.idx_free->idx_free_data.range_idx_free.i_max_idx_fltr[0],
                           cbarg->info.idx_free->idx_tbl_id);

       if (snmptm_finish_unreg == TRUE) 
       {
          NCSOAC_SS_ARG mab_arg;

          memset(&mab_arg, 0, sizeof(mab_arg));
        
          /* finally, unregister the tblfour table */
          mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;
          mab_arg.i_oac_hdl = snmptm->oac_hdl;
          mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLFOUR;
          mab_arg.info.tbl_gone.i_meaningless = (NCSCONTEXT)0x1111;
            
          if (ncsoac_ss(&mab_arg) == NCSCC_RC_FAILURE)
          {
             printf("snmptm_oac_cb(): ncsoac_ss() failed to unregister a table.\n");
             return NCSCC_RC_FAILURE;
          }
       }
       else
       {
          SNMPTM_TBLFOUR  *tblfour = NULL;
          uns32           index = 0;

          snmptm_idx_deleted = TRUE;
          
          index = *(cbarg->info.idx_free->idx_free_data.range_idx_free.i_min_idx_fltr);
          tblfour = snmptm_tblfour_rmv(&snmptm->tblfour, index);
          if (tblfour != NULL)
             m_MMGR_FREE_SNMPTM_TBLFOUR(tblfour);
       }
      break;

   case NCSOAC_SS_CB_OP_SA_DATA:
       return snmptm_tblfour_staging_area_handler(snmptm, cbarg->info.mib_req);
       break;
 
   default:
       break;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  Name          :  snmptm_tblfour_put
  
  Description   :  Adds the tblfor node to the tblfour list.
 
  Arguments     :  head - pointer to the tblfour head node of the tblfour list.
                   new_entry - new tblfour node to be added to the tblfour list.
    
  Return Values :  Nothing 
 
  Notes         : 
*****************************************************************************/
void snmptm_tblfour_put(SNMPTM_TBLFOUR** head, SNMPTM_TBLFOUR* new_entry)
{
   SNMPTM_TBLFOUR *tblfour;

   if (head == NULL)
      return;
  
   if (*head == NULL)
   {
      new_entry->next = NULL;
      *head = new_entry;
   }
   else
   {  
      tblfour = *head;

      /* Insertion as a starting node of the list */
      if (tblfour->index > new_entry->index)
      {
         new_entry->next = *head;
         *head = new_entry;
      }
      else
      {
         while(tblfour->next != NULL)
         {
            if (tblfour->next->index > new_entry->index)
            {
               new_entry->next = tblfour->next;
               tblfour->next = new_entry;
               return;
            }
               
            tblfour = tblfour->next;
         }
      
         if (tblfour->next == NULL)
         {  /* Insert as the last node of the list */
            new_entry->next = NULL;
            tblfour->next = new_entry;
         }
      }
   }
    
   return;
}


/*****************************************************************************
  Name          :  snmptm_tblfour_rmv
  
  Description   :  Remove the tblfor node from the tblfour list.
 
  Arguments     :  head - pointer to the tblfour head node of the tblfour list.
                   index - index value of the tblfour to be removed from the 
                           tblfour list.
    
  Return Values :  ret - pointer to the TBLFOUR node to be removed. 
 
  Notes         : 
*****************************************************************************/
SNMPTM_TBLFOUR* snmptm_tblfour_rmv(SNMPTM_TBLFOUR** head, uns32 index)
{
   SNMPTM_TBLFOUR* entry;
   SNMPTM_TBLFOUR* ret = NULL;
    
      
   if (head == NULL)
      return NULL;

   entry = *head;
    
   if (entry != NULL)
   {
      if (entry->index == index)
      {
         *head = entry->next;
         ret = entry;
      }
      else
      {
         while(entry->next != NULL)
         {
            if (entry->next->index == index)
            {
               ret = entry->next;
               entry->next = entry->next->next;
               return ret;
            }
            entry = entry->next;
         }
      }
   }
  
   return ret;
}


/*****************************************************************************
  Name          :  snmptm_tblfour_find
  
  Description   :  Search & retrieve the tblfor node from the tblfour list.
 
  Arguments     :  head - pointer to the tblfour head node of the tblfour list.
                   index - index value of the tblfour to be retrieved from the 
                           tblfour list.
    
  Return Values :  ret - pointer to the TBLFOUR node to be removed. 
 
  Notes         : 
*****************************************************************************/
SNMPTM_TBLFOUR* snmptm_tblfour_find(SNMPTM_TBLFOUR** head, NCSMIB_ARG *arg)
{
   SNMPTM_TBLFOUR *entry;
   SNMPTM_TBLFOUR *ret = NULL;
   uns32          index = 0;


   /* If index mentioned, get it */
   if ((arg->i_idx.i_inst_len != 0) &&
       (arg->i_idx.i_inst_ids != NULL))
   {
      index = *(arg->i_idx.i_inst_ids);
   }

   if (head == NULL)
   {
      return ((SNMPTM_TBLFOUR*)NULL);
   }
   
   entry = *head;

   if (entry != NULL)
   {
      if (entry->index == index)
      {
         return entry;
      }
      else
     {
         while(entry->next != NULL)
         {
            if (entry->next->index == index)
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


/****************************************************************************
  Name          :  ncstesttablefourentry_get 
  
  Description   :  Get function for SNMPTM tblfour table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get 
                                       request will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32  ncstesttablefourentry_get(NCSCONTEXT cb, 
                                 NCSMIB_ARG *arg, 
                                 NCSCONTEXT* data)
{
   SNMPTM_CB      *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLFOUR *tblfour = NULL;

   /* get the node */
   tblfour =  snmptm_tblfour_find(&snmptm->tblfour, arg);
   if (tblfour == NULL)
   {
      *data = NULL; 
      return NCSCC_RC_NO_INSTANCE; 
   }

   *data = tblfour;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstesttablefourentry_next
  
  Description   :  Next function for SNMPTM tblfour table 
 
  Arguments     :  NCSCONTEXT snmptm,   -  SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,     -  MIB argument (input)
                   NCSCONTEXT* data     -  pointer to the data from which get
                                           request will be serviced
                   uns32* next_inst_id  -  instance id of the object

  Return Values :  NCSCC_RC_SUCCESSS / NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablefourentry_next(NCSCONTEXT cb, 
                                 NCSMIB_ARG *arg, 
                                 NCSCONTEXT* data, 
                                 uns32* next_inst_id,
                                 uns32* next_inst_id_len) 
{
   SNMPTM_CB      *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLFOUR *tblfour = NULL;

   /* If no index defined then send first entry of the list */
   if ((arg->i_idx.i_inst_len == 0) ||
       (arg->i_idx.i_inst_ids == NULL))
   {
      if (snmptm->tblfour != NULL)
      {
         *data = snmptm->tblfour;
         *next_inst_id = snmptm->tblfour->index;
         *next_inst_id_len = 1;
         return NCSCC_RC_SUCCESS;
      }
      else
      {
         *data = NULL; 
         return NCSCC_RC_NO_INSTANCE; 
      }
   }
   
   /* get the node */
   tblfour =  snmptm_tblfour_find(&snmptm->tblfour, arg);

   if (tblfour == NULL)
   {
      *data = NULL; 
      return NCSCC_RC_NO_INSTANCE; 
   }

   if (tblfour->next != NULL)
   {
      *data = tblfour->next;
      *next_inst_id = tblfour->next->index;
      *next_inst_id_len = 1;
   }
   else
   {
      *data = NULL; 
      return NCSCC_RC_NO_INSTANCE;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  ncstesttablefourentry_set
  
  Description   :  Set function for SNMPTM tblfour table objects 
 
  Arguments     :  NCSCONTEXT snmptm         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg           - MIB argument (input)
                   NCSMIB_VAR_INFO* var_info - Object properties/info(i)
                   NCS_BOOL test_flag        - set/test operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablefourentry_set(NCSCONTEXT cb, 
                                NCSMIB_ARG *arg, 
                                NCSMIB_VAR_INFO *var_info,
                                NCS_BOOL test_flag)
{
   SNMPTM_CB           *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLFOUR      *tblfour = NULL;
   NCS_BOOL            val_same_flag = FALSE;
   NCSMIB_SET_REQ      *i_set_req = &arg->req.info.set_req; 
   uns32               rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  temp_mib_req;
   uns8                create_flag = FALSE;


   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 
  
   /* Check whether the entry exists in TBLONE table with the same key?? */
   tblfour =  snmptm_tblfour_find(&snmptm->tblfour, arg);

   /* Validate row status */
   if (i_set_req->i_param_val.i_param_id == ncsTestTableFourRowStatus_ID)
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
      temp_mib_req.info.i_val_status_util_info.row = tblfour;
      
      /* call the mib routine handler */ 
      if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      { 
         return rc;
      }
   }

   if (test_flag != TRUE) 
   {
      /* If the corresponding entry is not exists, and if the object is of 
         read-create type then create a tblfour entry. */
      if (tblfour == NULL) 
      {
         if (var_info->access == NCSMIB_ACCESS_READ_CREATE) 
         { 
            if ((i_set_req->i_param_val.i_param_id != ncsTestTableFourRowStatus_ID) ||
               ((i_set_req->i_param_val.i_param_id == ncsTestTableFourRowStatus_ID)
               && (create_flag != FALSE)))
            {
                tblfour = m_MMGR_ALLOC_SNMPTM_TBLFOUR;
                if (tblfour == NULL)
                {
                    return NCSCC_RC_FAILURE; 
                }

                memset(tblfour, 0, sizeof(SNMPTM_TBLFOUR));
                tblfour->index = *(arg->i_idx.i_inst_ids);
                tblfour->status = NCSMIB_ROWSTATUS_ACTIVE;

                /* add to the list */
                snmptm_tblfour_put(&snmptm->tblfour, tblfour);
            }
         }
         else
            return NCSCC_RC_NO_INSTANCE;
      }

      /* All the tests have been done now set the value */
      if ((i_set_req->i_param_val.i_param_id == ncsTestTableFourRowStatus_ID)
           && (i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_DESTROY))
      {
         if (tblfour == NULL)
            return NCSCC_RC_NO_INSTANCE;

         /* Delete the entry from the TBLFOUR */
         if ((arg->i_idx.i_inst_len != 0) ||
             (arg->i_idx.i_inst_ids != NULL))
         {
             uns32 index;

             index = *(arg->i_idx.i_inst_ids);
             tblfour = snmptm_tblfour_rmv(&snmptm->tblfour, index);
             if (m_MDS_GET_VDEST_ID_FROM_MDS_DEST(snmptm->vcard) == SNMPTM_VCARD_ID2) 
             {
                ncstesttablefourentry_moverow_unreg(snmptm, arg, tblfour->o_row_hdl);
             }
             if (tblfour != NULL)
                 m_MMGR_FREE_SNMPTM_TBLFOUR(tblfour);
         }
      }
      else
      {
         memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

         temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
         temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
         temp_mib_req.info.i_set_util_info.var_info = var_info;
         temp_mib_req.info.i_set_util_info.data = tblfour;
         temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

         /* call the mib routine handler */ 
         if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
         {
            return rc;
         }
      }
   }

   /* Send a MOVEROW request, if there is a SET happen for 
      ncsTestTableFourUInteger object on the process which is running on VCARD_1*/
   if ((i_set_req->i_param_val.i_param_id == ncsTestTableFourUInteger_ID) &&
       (test_flag != TRUE) && (m_MDS_GET_VDEST_ID_FROM_MDS_DEST(snmptm->vcard) == SNMPTM_VCARD_ID1)) 
   {
      NCSPARM_AID       rsp_pa;
      NCSMIB_PARAM_VAL  param_val;
      USRBUF            *buff = NULL;
      NCSMIB_PARAM_ID   param_id = 0;
      NCSMIB_VAR_INFO   *temp_var_info = NULL;

      /* Check whether the row related stucture content exists.. if not exists just
         say no such instance to move the ROW */
      if (!tblfour)
         return NCSCC_RC_NO_INSTANCE;

      /* Initialize the encoding param_val buffer */     
      ncsparm_enc_init(&rsp_pa);

      for(param_id = ncsTestTableFourIndex_ID; 
          param_id < ncsTestTableFourEntryMax_ID; 
          param_id++)
      {
          if (param_id == ncsTestTableFourIndex_ID)
              continue; 
               
          memset(&param_val, 0, sizeof(NCSMIB_PARAM_VAL)); 

          /* As SET operation just holds a var_info of a particular object, need to
             do '+' or '-' on this var_info to get the appropriate var_info field */
          if (param_id >= i_set_req->i_param_val.i_param_id)
          {
             temp_var_info = (NCSMIB_VAR_INFO *)(var_info + 
                             (param_id - i_set_req->i_param_val.i_param_id)); 
          }
          else
          {
             temp_var_info = (NCSMIB_VAR_INFO *)(var_info -
                              (i_set_req->i_param_val.i_param_id - param_id)); 
          }

          /* Extract the object data into param_val struct */ 
          ncsmiblib_get_obj_val(&param_val, temp_var_info, tblfour, NULL);
          param_val.i_param_id = param_id;

          /* If you're buffering the ROWSTATUS object then MOVEROW should consists
           * create & go to mean as a fresh request to create it */
          if (param_id == ncsTestTableFourRowStatus_ID)
          {
             param_val.info.i_int =  NCSMIB_ROWSTATUS_CREATE_GO;
          }

          /* Encode the param_val data into buffer */
          ncsparm_enc_param(&rsp_pa, &param_val);
      }

      /* Done with param_val buffering, return the pointer of the 
         respective USRBUF */
      buff = ncsparm_enc_done(&rsp_pa);
     
      /* Form a MOVEROW request */ 
      if (buff)
      {
         printf("SNMPTM TBLFOUR: Sending MOVEROW request \n");
         ncstesttablefourentry_moverow(snmptm, arg, buff);
      }
   }

   return NCSCC_RC_SUCCESS; 
}


/****************************************************************************
  Name          :  ncstesttablefourentry_setrow
  
  Description   :  Setrow function for SNMPTM tblfour table 
 
  Arguments     :  NCSCONTEXT snmptm,         - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg,           - MIB argument (input)
                   NCSMIB_SETROW_PARAM_VAL    - array of params to be set
                   ncsmib_obj_info* obj_info, - Object and table properties/info(i)
                   NCS_BOOL test_flag         - setrow/testrow operation (input)
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablefourentry_setrow(NCSCONTEXT cb, 
                                   NCSMIB_ARG* arg,
                                   NCSMIB_SETROW_PARAM_VAL* tblfour_param,
                                   struct ncsmib_obj_info* obj_info,
                                   NCS_BOOL testrow_flag)
{
   SNMPTM_CB           *snmptm = (SNMPTM_CB *)cb;
   SNMPTM_TBLFOUR      *tblfour = SNMPTM_TBLFOUR_NULL;
   NCSMIBLIB_REQ_INFO  temp_mib_req;   
   NCS_BOOL            row_owned = TRUE;
   uns32               rc = NCSCC_RC_SUCCESS;

   printf("\nncsTestTableFourEntry: Received SNMP SETROW request\n");
   
   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

   /* Check whether the entry exists in TBLONE table with the same key?? */
   tblfour =  snmptm_tblfour_find(&snmptm->tblfour, arg);

   /* Validate row status */
   if (tblfour_param[ncsTestTableFourRowStatus_ID - 1].set_flag == TRUE)
   { 
      temp_mib_req.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP; 
      temp_mib_req.info.i_val_status_util_info.row_status =
             &(tblfour_param[ncsTestTableFourRowStatus_ID - 1].param.info.i_int);
      temp_mib_req.info.i_val_status_util_info.row = tblfour;
      
      /* call the mib routine handler */ 
      if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      { 
         return rc;
      }
   } 
 
   /* If the corresponding entry is not exists, and if the object is of 
      read-create type then create a tblfour entry. */
   if (tblfour == NULL) 
   {
      row_owned = FALSE;

      if (tblfour_param[ncsTestTableFourRowStatus_ID - 1].set_flag == TRUE)
      {
         tblfour = m_MMGR_ALLOC_SNMPTM_TBLFOUR;
         if (tblfour == NULL)
         {
            return NCSCC_RC_FAILURE; 
         }

         memset(tblfour, 0, sizeof(SNMPTM_TBLFOUR));
         tblfour->index = *(arg->i_idx.i_inst_ids);
         tblfour->status = NCSMIB_ROWSTATUS_ACTIVE; 

         /* add to the list */
         snmptm_tblfour_put(&snmptm->tblfour, tblfour);
      }
      else
         return NCSCC_RC_NO_INSTANCE;
   }

   if (testrow_flag != TRUE)
   {
      NCS_BOOL         val_same_flag = FALSE;
      NCSMIB_PARAM_ID  param_id = 0;

      for(param_id = ncsTestTableFourIndex_ID; 
          param_id < ncsTestTableFourEntryMax_ID; 
          param_id++)
      { 
          /* By-pass index to SET */ 
          if (param_id == ncsTestTableFourIndex_ID)
             continue;

          if (tblfour_param[param_id - 1].set_flag == TRUE)
          {
             memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 
               
             temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
             temp_mib_req.info.i_set_util_info.param = &(tblfour_param[param_id - 1].param);
             temp_mib_req.info.i_set_util_info.var_info = &obj_info->var_info[param_id - 1];
             temp_mib_req.info.i_set_util_info.data = tblfour;
             temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;
                   
             /* call the mib routine handler */ 
             if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
             {
                return rc;
             }
                   
             val_same_flag = FALSE;
          }
      }
   }

   if (move_row_flg == TRUE)
   {
      /* Register the ROW with OAC */
      rc = ncstesttablefourentry_moverow_reg(snmptm, arg, &tblfour->o_row_hdl);
      if (rc != NCSCC_RC_SUCCESS)
         return rc;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : ncstesttablefourentry_extract 
  
  Description   :  Get function for SNMPTM tblfour table objects 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)
                   NCSCONTEXT* data  - pointer to the data from which get request
                                       will be serviced

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 ncstesttablefourentry_extract(NCSMIB_PARAM_VAL *param, 
                                    NCSMIB_VAR_INFO  *var_info,
                                    NCSCONTEXT data,
                                    NCSCONTEXT buffer)
{
   return ncsmiblib_get_obj_val(param, var_info, data, buffer);
}


/****************************************************************************
  Name          :  ncstesttablefourentry_moverow_reg 
  
  Description   :  Doew ROW OWNED operation for MOVEROW on table four
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
static uns32 ncstesttablefourentry_moverow_reg(SNMPTM_CB* snmptm,
                                               NCSMIB_ARG *arg,
                                               NCSCONTEXT *row_hdl)
{
   uns32  index;
   uns32  min_mask[1] = {0};
   uns32  max_mask[1] = {0};
   NCSOAC_SS_ARG  mab_arg;


   index = (uns32) arg->i_idx.i_inst_ids[0];
      
   /* There shall be some data forwarded from the staging area to 
      put it into the data structure.  Data can be send from the 
      staging area along with the MOVEROW request. -- TBD */

   memset(&mab_arg,0,sizeof(mab_arg));

   mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
   mab_arg.i_oac_hdl = snmptm->oac_hdl;
   mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLFOUR;

   min_mask[0] = index;
   max_mask[0] = index;

   mab_arg.info.row_owned.i_fltr.type =  NCSMAB_FLTR_RANGE;

   mab_arg.info.row_owned.i_fltr.fltr.range.i_bgn_idx = 0; 
   mab_arg.info.row_owned.i_fltr.fltr.range.i_idx_len = 1;
   mab_arg.info.row_owned.i_fltr.fltr.range.i_max_idx_fltr = max_mask;
   mab_arg.info.row_owned.i_fltr.fltr.range.i_min_idx_fltr = min_mask;

   mab_arg.info.row_owned.i_fltr.is_move_row_fltr = TRUE;

   if (ncsoac_ss(&mab_arg) == NCSCC_RC_FAILURE)
   {
      printf("ncstesttablefourentry_moverow_reg(): ncsoac_ss() failed to register a row.\n");
      return NCSCC_RC_FAILURE;
   }

   *row_hdl = NCS_UNS32_TO_PTR_CAST(mab_arg.info.row_owned.o_row_hdl);

   printf("ncstesttablefourentry_moverow_reg(): ROW has been registered with the new OAC.\n");

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncstesttablefourentry_moverow_unreg 
  
  Description   : Does ROW GONE operation for table four 
 
  Arguments     :  NCSCONTEXT snmptm - SNMPTM control block (input/output)
                   NCSMIB_ARG *arg   - MIB argument (input)

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
static uns32 ncstesttablefourentry_moverow_unreg(SNMPTM_CB* snmptm,
                                                 NCSMIB_ARG *arg, 
                                                 NCSCONTEXT row_hdl)
{
   NCSOAC_SS_ARG  mab_arg;
   memset(&mab_arg,0,sizeof(mab_arg));

   mab_arg.i_op = NCSOAC_SS_OP_ROW_GONE;
   mab_arg.i_oac_hdl = snmptm->oac_hdl;
   mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLFOUR;

   mab_arg.info.row_gone.i_row_hdl =  NCS_PTR_TO_UNS32_CAST(row_hdl);

   if (ncsoac_ss(&mab_arg) == NCSCC_RC_FAILURE)
   {
      printf("ncstesttablefourentry_moverow_unreg(): ncsoac_ss() failed to de-register a row.\n");
      return NCSCC_RC_FAILURE;
   }

   printf("ncstesttablefourentry_moverow_unreg(): ROW has been de-registered with the new OAC.\n");

   return NCSCC_RC_SUCCESS;
}

#endif


