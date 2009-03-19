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

  MODULE NAME: SNMPTM_TBLCMD.C 

..............................................................................

  DESCRIPTION:  Defines GET/SET operations on the specified tables based on the
                command-id arrived. Basically the operations facilitates to 
                test 'CE Via MASv' feature.
..............................................................................

******************************************************************************/

#include "snmptm.h"

#if (NCS_SNMPTM == 1)

/****************************************************************************
  Name          :  snmptm_oac_tblcmd_register
  
  Description   :  Register the table (TBLFOUR) with OAC. 
 
  Arguments     :  snmptm - pointer to the SNMPMTM CB.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_oac_tblcmd_register(SNMPTM_CB *snmptm)
{
   NCSOAC_SS_ARG  oac_arg;
   char           inst_str[SNMPTM_SERVICE_NAME_LEN];
   uns32          instance_idx[7]; /* SNMPTM_SERVICE_NAME_LEN */
   uns8           i;

   /* register the table */
   memset(&oac_arg, 0, sizeof(NCSOAC_SS_ARG));
  
   /* register the tblfour table with the OAC */
   oac_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
   oac_arg.i_oac_hdl = snmptm->oac_hdl;
   oac_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_COMMAND;

   /* Table registration */
   oac_arg.info.tbl_owned.i_mib_req = snmptm_tblcmd_tbl_req;
   oac_arg.info.tbl_owned.i_mib_key = snmptm->hmcb_hdl;
   oac_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_SNMPTM;
            
   if (ncsoac_ss(&oac_arg) == NCSCC_RC_FAILURE)
   {
      m_NCS_CONS_PRINTF("snmptm_oac_tblcmd_register(): ncsoac_ss() failed to register a table.\n");
      return NCSCC_RC_FAILURE;
   }

   memcpy(inst_str, SNMPTM_SERVICE_NAME, SNMPTM_SERVICE_NAME_LEN); 

   /* Copy the service name to the index part */
   for (i=0; i< SNMPTM_SERVICE_NAME_LEN; i++)
      instance_idx[i] = inst_str[i];
   
   /* Fill up the node id also */
   instance_idx[i] = snmptm->node_id;
       
   /* Row registration */ 
   oac_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
   oac_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_RANGE;
   oac_arg.info.row_owned.i_fltr.fltr.range.i_bgn_idx = 0;
   oac_arg.info.row_owned.i_fltr.fltr.range.i_idx_len = SNMPTM_SERVICE_NAME_LEN + 1;
   oac_arg.info.row_owned.i_fltr.fltr.range.i_max_idx_fltr = instance_idx;
   oac_arg.info.row_owned.i_fltr.fltr.range.i_min_idx_fltr = instance_idx;

   if (ncsoac_ss(&oac_arg) == NCSCC_RC_FAILURE)
   {
      m_NCS_CONS_PRINTF("snmptm_oac_tblcmd_register(): ncsoac_ss() failed to register a Row/instance idx info.\n");
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptm_tblcmd_create_row
  
  Description   :  This function creates a ROW in the specified table if the
                   row is not existing in it. 
 
  Arguments     :  snmptm - pointer to the SNMPMTM CB.
                   arg - pointer to the NCSMIB_ARG structure
                   tbl_id - table id value.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
static uns32  snmptm_tblcmd_create_row(SNMPTM_CB *snmptm, NCSMIB_ARG *arg, uns32 tbl_id)
{
   USRBUF *buff;
   uns8   *buff_ptr;
   uns32  ip_addr = 0;      
   uns8   local_buff[4];
   SNMPTM_TBL_KEY tbl_key;

   memset(&tbl_key, '\0', sizeof(SNMPTM_TBL_KEY));
 
   buff = arg->req.info.cli_req.i_usrbuf;
   if (buff == NULL)
   {
      /* Send back the response, */
      return NCSCC_RC_FAILURE;
   }
  
   if ((buff_ptr = (uns8 *)m_MMGR_DATA_AT_START(buff,
                                                sizeof(uns32),
                                                local_buff)) == (uns8 *)0)
      return NCSCC_RC_FAILURE;

   ip_addr = m_NCS_OS_NTOHL_P(buff_ptr);
   buff_ptr += 4;
    
   /* Frame the key */  
   tbl_key.ip_addr.type = NCS_IP_ADDR_TYPE_IPV4;
   tbl_key.ip_addr.info.v4 = htonl(ip_addr);
  
   if (tbl_id == NCSMIB_TBL_SNMPTM_TBLONE) 
   {
      SNMPTM_TBLONE *tblone = SNMPTM_TBLONE_NULL;

      /* Check whether the entry exists in TBLONE table with the same key?? */
      tblone = (SNMPTM_TBLONE*)ncs_patricia_tree_get(&snmptm->tblone_tree,
                                                     (uns8*)&tbl_key); 
      /* If row not exists then create it */
      if (tblone == NULL)
      {
         tblone = snmptm_create_tblone_entry(snmptm, &tbl_key);
         
         /* Not able to create a tblone entry */
         if (tblone == NULL)
         {
            /* Not able to Create Row */
            return NCSCC_RC_FAILURE;
         }
         
         /* Sample counter update with some random value */
         tblone->tblone_cntr32 = (rand() % 10000);
      }
      else
      {
         m_NCS_CONS_PRINTF("\n\n ROW exists in TBLONE, INDEX: %d.%d.%d.%d", (uns8)(ip_addr >> 24),
                                            (uns8)(ip_addr >> 16),
                                            (uns8)(ip_addr >> 8),
                                            (uns8)(ip_addr)); 
      }
   }
   else if (tbl_id == NCSMIB_TBL_SNMPTM_TBLFIVE)
   {
      SNMPTM_TBLFIVE *tblfive = SNMPTM_TBLFIVE_NULL;

      /* Check whether the entry exists in TBLFIVE table with the same key?? */
      tblfive = (SNMPTM_TBLFIVE*)ncs_patricia_tree_get(&snmptm->tblfive_tree,
                                                     (uns8*)&tbl_key); 

      /* If row not exists then create it */
      if (tblfive == NULL)
      {
         tblfive = snmptm_create_tblfive_entry(snmptm, &tbl_key);

         /* Not able to create a tblfive entry */
         if (tblfive == NULL)
         {
            /* Not able to Create Row */
            return NCSCC_RC_FAILURE;
         }
      }
      else
      {
         m_NCS_CONS_PRINTF("\n\n ROW exists in TBLFIVE, INDEX: %d.%d.%d.%d", (uns8)(ip_addr >> 24),
                                            (uns8)(ip_addr >> 16),
                                            (uns8)(ip_addr >> 8),
                                            (uns8)(ip_addr)); 
      }
   }
   else
   {
      m_NCS_CONS_PRINTF("Invalid Tbl-ID specified for create-row command....\n");
      return NCSCC_RC_FAILURE;
   }
   
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  snmptm_pack_tblone_data
  
  Description   :  This function packs the complete data of TBLONE in USRBUF
                   for display purposes.
 
  Arguments     :  snmptm - pointer to the SNMPMTM CB.
                   arg - pointer to the NCSMIB_ARG structure
                   tbl_id - table id value.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
static uns32 snmptm_pack_tblone_data(SNMPTM_CB *snmptm, NCSMIB_ARG *arg, NCS_BOOL partial)
{
   USRBUF *buff = NULL;
   uns8   *buff_ptr = NULL;
   uns32  tbl_id = NCSMIB_TBL_SNMPTM_TBLONE;
   uns32  inst_id = snmptm->node_id;
   SNMPTM_TBLONE *tblone;
   SNMPTM_TBL_KEY tbl_key;
   uns32  ip_addr = 0;
   
   memset(&tbl_key, '\0', sizeof(SNMPTM_TBL_KEY));
 
   for (tblone = (SNMPTM_TBLONE*)ncs_patricia_tree_getnext(&snmptm->tblone_tree,
                                                     (uns8*)&tbl_key); 
        tblone != NULL;
        tblone = (SNMPTM_TBLONE*)ncs_patricia_tree_getnext(&snmptm->tblone_tree,
                                                     (uns8*)&tbl_key))
   {
      /* Copy the key info */
      tbl_key = tblone->tblone_key;
   
      /* Framing a buffer not yet started??, so create it and start
         framing */   
      if (buff == NULL)
      {
         /* Alloc the memory for USRBUF */
         if ((buff = m_MMGR_ALLOC_BUFR(sizeof(USRBUF))) == NULL)
         {
            m_NCS_CONS_PRINTF("\n\nsnmptm_pack_tblone_data(): MEM  alloc for USRBUF failed");
            return NCSCC_RC_FAILURE;
         }

         m_BUFR_STUFF_OWNER(buff);
         buff_ptr = m_MMGR_RESERVE_AT_END(&buff,
                                          SNMPTM_HDR_SIZE,
                                          uns8*);
         if (buff_ptr == NULL)
         {
            m_MMGR_FREE_BUFR_LIST(buff);
            m_NCS_CONS_PRINTF("\n\nsnmptm_pack_tblone_data(): Reserve MEM for HDR failed");
            return NCSCC_RC_FAILURE;
         }   /* USRBUF Malloc */

         m_NCS_OS_HTONL_P(buff_ptr, inst_id);
         buff_ptr += 4;

         m_NCS_OS_HTONL_P(buff_ptr, tbl_id);
      } /* end buff == NULL */

      buff_ptr = m_MMGR_RESERVE_AT_END(&buff,
                                       SNMPTM_TBLONE_ENTRY_SIZE,
                                       uns8*);
      if (buff_ptr == NULL)
      {
         m_MMGR_FREE_BUFR_LIST(buff);
         m_NCS_CONS_PRINTF("\n\nsnmptm_pack_tblone_data(): Reserve MEM for TBLONE entry failed");
         return NCSCC_RC_FAILURE;
      } 
     
      ip_addr =  ntohl(tblone->tblone_key.ip_addr.info.v4);
      m_NCS_OS_HTONL_P(buff_ptr, ip_addr);
      buff_ptr += 4;
      
      m_NCS_OS_HTONL_P(buff_ptr, tblone->tblone_row_status);
      buff_ptr += 4;

      m_NCS_OS_HTONL_P(buff_ptr, tblone->tblone_cntr32);

   }  /* End of for loop */

   arg->rsp.info.cli_rsp.o_partial = partial;
   arg->rsp.info.cli_rsp.o_answer = buff;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptm_pack_tblfive_data
  
  Description   :  This function packs the complete data of TBLFIVE in USRBUF
                   for display purposes.
 
  Arguments     :  snmptm - pointer to the SNMPMTM CB.
                   arg - pointer to the NCSMIB_ARG structure
                   tbl_id - table id value.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
static uns32 snmptm_pack_tblfive_data(SNMPTM_CB *snmptm, NCSMIB_ARG *arg, NCS_BOOL partial)
{
   USRBUF *buff = NULL;
   uns8   *buff_ptr = NULL;
   uns32  tbl_id = NCSMIB_TBL_SNMPTM_TBLFIVE;
   uns32  inst_id = snmptm->node_id;
   SNMPTM_TBL_KEY tbl_key;
   SNMPTM_TBLFIVE *tblfive;
   uns32  ip_addr = 0;

   memset(&tbl_key, '\0', sizeof(SNMPTM_TBL_KEY));
 
   for (tblfive = (SNMPTM_TBLFIVE*)ncs_patricia_tree_getnext(&snmptm->tblfive_tree,
                                                     (uns8*)&tbl_key); 
        tblfive != NULL;
        tblfive = (SNMPTM_TBLFIVE*)ncs_patricia_tree_getnext(&snmptm->tblfive_tree,
                                                     (uns8*)&tbl_key))
   {
      /* Copy the key info */
      tbl_key = tblfive->tblfive_key;
   
      /* Framing a buffer not yet started??, so create it and start
         framing */   
      if (buff == NULL)
      {
         /* Alloc the memory for USRBUF */
         if ((buff = m_MMGR_ALLOC_BUFR(sizeof(USRBUF))) == NULL)
         {
            /* Log the message saying USRBUF malloc failed */
            return NCSCC_RC_FAILURE;
         }

         m_BUFR_STUFF_OWNER(buff);
         buff_ptr = m_MMGR_RESERVE_AT_END(&buff,
                                          SNMPTM_HDR_SIZE,
                                          uns8*);
         if (buff_ptr == NULL)
         {
            m_MMGR_FREE_BUFR_LIST(buff);
            /* Log the message, not able to reserve the memory for the USRBUF */
            return NCSCC_RC_FAILURE;
         }   /* USRBUF Malloc */

         m_NCS_OS_HTONL_P(buff_ptr, inst_id);
         buff_ptr += 4;

         m_NCS_OS_HTONL_P(buff_ptr, tbl_id);
      } /* end buff == NULL */

      buff_ptr = m_MMGR_RESERVE_AT_END(&buff,
                                       SNMPTM_TBLFIVE_ENTRY_SIZE,
                                       uns8*);
      if (buff_ptr == NULL)
      {
         m_MMGR_FREE_BUFR_LIST(buff);
         /* Log the message, not able to reserve the memory for the USRBUF */
         return NCSCC_RC_FAILURE;
      } 
      
      ip_addr =  ntohl(tblfive->tblfive_key.ip_addr.info.v4);
      m_NCS_OS_HTONL_P(buff_ptr, ip_addr);
      buff_ptr += 4;
      
      m_NCS_OS_HTONL_P(buff_ptr, tblfive->tblfive_row_status);
   }  /* End of for loop */

   arg->rsp.info.cli_rsp.o_partial = partial;
   arg->rsp.info.cli_rsp.o_answer = buff;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  snmptm_tblcmd_show_tbl_data
  
  Description   :  This function packs the complete data of a specified table
                   in USRBUF for display purposes.
 
  Arguments     :  snmptm - pointer to the SNMPMTM CB.
                   arg - pointer to the NCSMIB_ARG structure
                   tbl_id - table id value.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
static uns32 snmptm_tblcmd_show_tbl_data(SNMPTM_CB *snmptm, NCSMIB_ARG *arg, uns32 tbl_id)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCSMIB_ARG *temp_arg;

   switch(tbl_id) 
   {
   case NCSMIB_TBL_SNMPTM_TBLONE:
       rc = snmptm_pack_tblone_data(snmptm, arg, FALSE);
       break;

   case NCSMIB_TBL_SNMPTM_TBLFIVE:
       rc = snmptm_pack_tblfive_data(snmptm, arg, FALSE);
       break;

   default: /* Need to send both the table information */
       {
          arg->rsp.info.cli_rsp.i_cmnd_id = arg->req.info.cli_req.i_cmnd_id;

          /* Get the copy of arg */
          temp_arg = ncsmib_memcopy(arg);
          if (temp_arg == NULL)
          {
             /* Not able to get the copy of NCSMIB_ARG */
             return NCSCC_RC_FAILURE;  
          }

          /* Send the CLI-RESP with o_partial flag set to TRUE */
          rc = snmptm_pack_tblone_data(snmptm, arg, TRUE);
          arg->rsp.i_status = rc;
          arg->i_op = m_NCSMIB_REQ_TO_RSP(arg->i_op);
          if (arg->i_rsp_fnc(arg))
          {
             m_MMGR_FREE_BUFR_LIST(arg->rsp.info.cli_rsp.o_answer);
          }

          /* Send the CLI-RESP with o_partial flag set to FALSE */
          rc = snmptm_pack_tblfive_data(snmptm, temp_arg, FALSE);
          temp_arg->rsp.i_status = rc;
          temp_arg->i_op = m_NCSMIB_REQ_TO_RSP(temp_arg->i_op);
          if (temp_arg->i_rsp_fnc(temp_arg))
          {
             m_MMGR_FREE_BUFR_LIST(temp_arg->rsp.info.cli_rsp.o_answer);
          }
       }
       break;
   }
   
   return rc;
}


/****************************************************************************
  Name          :  snmptm_tblcmd_tbl_req
  
  Description   :  High Level MIB Access Routine (Request function)for SNMPTM 
                   TBLCMD table.
 
  Arguments     :  struct ncsmib_arg*
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_tblcmd_tbl_req(struct ncsmib_arg *args)
{
   uns32  cb_hdl = args->i_mib_key;
   uns32  rc = NCSCC_RC_SUCCESS;
   uns8   command_id;
   SNMPTM_CB *snmptm = SNMPTM_CB_NULL;

   /* Get the CB from the handle manager */
   if ((snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             cb_hdl)) == NULL)
   {
      args->rsp.i_status = NCSCC_RC_NO_INSTANCE;
      args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
      args->i_rsp_fnc(args);
   
      return NCSCC_RC_FAILURE;
   }

   command_id = args->req.info.cli_req.i_cmnd_id;

   /* Based on the command id.. process the CLI request */   
   switch (command_id)
   {
   case tblone_create_row_cmdid: 
       rc = snmptm_tblcmd_create_row(snmptm, args, NCSMIB_TBL_SNMPTM_TBLONE);
       break;
  
   case tblfive_create_row_cmdid:
       rc = snmptm_tblcmd_create_row(snmptm, args, NCSMIB_TBL_SNMPTM_TBLFIVE);
       break;
  
   case tblone_show_all_rows_cmdid:
       rc = snmptm_tblcmd_show_tbl_data(snmptm, args, NCSMIB_TBL_SNMPTM_TBLONE);
       break;
  
   case tblfive_show_all_rows_cmdid:
       rc = snmptm_tblcmd_show_tbl_data(snmptm, args, NCSMIB_TBL_SNMPTM_TBLFIVE);
       break;
  
   case tblone_tblfive_show_data_cmdid:
   case tblone_tblfive_show_glb_data_cmdid:
       rc = snmptm_tblcmd_show_tbl_data(snmptm, args, 0);
       /* Release the SNMPTM CB handle */
       ncshm_give_hdl(cb_hdl);
       return rc;
       break;
       
   default:
       rc = NCSCC_RC_NO_INSTANCE;
       break;
   }

   /* Call the response funstion */
   args->rsp.i_status = rc;
   args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
   args->rsp.info.cli_rsp.i_cmnd_id = command_id;
   
   if (args->i_rsp_fnc(args))
   {
      switch (command_id)
      {
      case tblone_show_all_rows_cmdid:
      case tblfive_show_all_rows_cmdid:
          m_MMGR_FREE_BUFR_LIST(args->rsp.info.cli_rsp.o_answer);
          break;

      default:
          break;
      }
   }

   /* Release the SNMPTM CB handle */
   ncshm_give_hdl(cb_hdl);

   return NCSCC_RC_SUCCESS;
}

#endif /* (NCS_SNMPTM == 1) */

