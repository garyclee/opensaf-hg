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
  MODULE NAME: SNMPTM_CLI.C

..............................................................................

  DESCRIPTION: Contains CLI definitions for SNMPTM 

******************************************************************************
*/
#include "snmptm.h"

#if(NCS_SNMPTM_CLI == 1)

static uns32 *snmptm_get_inst_idx(NCSCLI_CEF_DATA *cef_data);

/****************************************************************************
  Name          :  snmptm_cef_conf_mode
  
  Description   :  This function stores the command mode (snmptm) information
                   i.e. svc_instance name and the instance_id which is required
                   for the commands (used as an instance idx) that are going
                   to be defined in this mode.
 
  Arguments     :  arg_list - Argument list given in command
                   cef_data - Command related DATA
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_cef_conf_mode(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   char  svc_name[SNMPTM_SERVICE_NAME_LEN];
   uns8  i;
   SNMPTM_CEF_CMD_DATA *pcmd_data;

   if (((cef_data->i_subsys->i_cef_mode) = pcmd_data =
                  m_MMGR_ALLOC_SNMPTM_CMD_DATA) == NULL)
      return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(svc_name, 0, SNMPTM_SERVICE_NAME_LEN);
   memcpy(svc_name, SNMPTM_SERVICE_NAME, SNMPTM_SERVICE_NAME_LEN);

   m_NCS_OS_MEMSET(pcmd_data, '\0', sizeof(SNMPTM_CEF_CMD_DATA));

   /* Copy the svc_name + pwe_id to make it an index to identify the 
      destination OAC */
   for (i=0; i<SNMPTM_SERVICE_NAME_LEN; i++)
       pcmd_data->svc_instance[i] = (uns32)svc_name[i]; 

   pcmd_data->instance_id = arg_list->i_arg_record[1].cmd.intval;

   snmptm_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptm_get_inst_idx
  
  Description   :  This function returns the svc_instance string (idx) which
                   was stored under snmptm command mode.
 
  Arguments     :  arg_list - Argument list given in command
                   cef_data - Command related DATA
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
static uns32 *snmptm_get_inst_idx(NCSCLI_CEF_DATA *cef_data)
{
   SNMPTM_CEF_CMD_DATA *pcmd_data;
   uns32  *inst_idx = NULL;
   NCSCLI_SUBSYS_CB *data = NULL;

   /* Get the subsys cmd data pointer */
   data = snmptmcli_get_subsys_cb(cef_data->i_bindery->i_cli_hdl);
   if (data == NULL)
      return inst_idx;

   pcmd_data = (SNMPTM_CEF_CMD_DATA *)data->i_cef_mode;
   if (pcmd_data != NULL)
      inst_idx = pcmd_data->svc_instance;

   return inst_idx;
}


/****************************************************************************
  Name          :  snmptm_cef_tbl_create_row
  
  Description   :  This function creates a row for the table mentioned in the 
                   command input.
 
  Arguments     :  arg_list - Argument list given in command
                   cef_data - Command related DATA
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_cef_tbl_create_row(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   NCSMIB_ARG  arg;
   USRBUF      *buff;
   uns8        *buff_ptr;
   uns8        tbl_id = 0;
   uns32       idx_ip_addr = 0;
   uns8        space[2048];
   NCSMEM_AID  ma;


   m_NCS_OS_MEMSET(&arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   /* Get the table-id and the row index (ip_address) value */
   tbl_id = arg_list->i_arg_record[2].cmd.intval;
   idx_ip_addr = arg_list->i_arg_record[4].cmd.intval;

   /* Check whether you've got the right tbl-id */
   if (tbl_id == 1)
      arg.req.info.cli_req.i_cmnd_id = tblone_create_row_cmdid;
   else if (tbl_id == 5)
      arg.req.info.cli_req.i_cmnd_id = tblfive_create_row_cmdid;
   else
   {
      m_NCS_CONS_PRINTF("\n\nsnmptm_cef_tbl_create_row(): Invalid table number: %d", tbl_id);
      snmptm_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }
   
   /* Fill the MIB_ARG structure */
   arg.i_op = NCSMIB_OP_REQ_CLI;
   arg.i_tbl_id = NCSMIB_TBL_SNMPTM_COMMAND;
   arg.i_rsp_fnc = snmptm_cli_set_resp;

   /* Update the indices info */
   arg.i_idx.i_inst_ids = snmptm_get_inst_idx(cef_data);
   arg.i_idx.i_inst_len = (SNMPTM_SERVICE_NAME_LEN + 1);
   
   /* Update CLI_REQ structue of MIB_ARG */
   arg.req.info.cli_req.i_wild_card = FALSE;

   if ((buff = m_MMGR_ALLOC_BUFR(sizeof(USRBUF))) == NULL)
   {
      m_NCS_CONS_PRINTF("\n\nIsnmptm_cef_tbl_create_row(): USRBUF malloc failed");
      snmptm_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE; 
   }

   m_BUFR_STUFF_OWNER(buff);
   buff_ptr = m_MMGR_RESERVE_AT_END(&buff,
                                    sizeof(uns32),
                                    uns8*);

   if (buff_ptr == NULL)
   {
      m_NCS_CONS_PRINTF("\n\nIsnmptm_cef_tbl_create_row(): USRBUF reserve at end failed");
      snmptm_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE; 
   }
   
   m_NCS_OS_HTONL_P(buff_ptr, idx_ip_addr);
   
   /* TBD: Framing the USRBUF */
   arg.req.info.cli_req.i_usrbuf = buff;

   ncsstack_init(&arg.stack, NCSMIB_STACK_SIZE); 

   if ((snmptm_send_mib_req(&arg, cef_data)) == NCSCC_RC_FAILURE)
   {
      snmptm_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }
   
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  snmptm_cef_show_tbl_data
  
  Description   :  This function displays the data of the table mentioned in
                   the command input.
 
  Arguments     :  arg_list - Argument list given in command
                   cef_data - Command related DATA
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_cef_show_tbl_data(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   NCSMIB_ARG  arg;
   uns8        tbl_id = 0;
   uns8        space[2048];
   NCSMEM_AID  ma;

   m_NCS_OS_MEMSET(&arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   tbl_id = arg_list->i_arg_record[2].cmd.intval;

   /* Check whether you've got the right tbl-id */
   if (tbl_id == 1)
      arg.req.info.cli_req.i_cmnd_id = tblone_show_all_rows_cmdid;
   else if (tbl_id == 5)
      arg.req.info.cli_req.i_cmnd_id = tblfive_show_all_rows_cmdid;
   else
   {
      m_NCS_CONS_PRINTF("\n\nsnmptm_cef_show_tbl_data(): Invalid table number: %d", tbl_id);
      snmptm_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }
   
   /* Fill the MIB_ARG structure */
   arg.i_op = NCSMIB_OP_REQ_CLI;
   arg.i_tbl_id = NCSMIB_TBL_SNMPTM_COMMAND;
   arg.i_rsp_fnc = snmptm_cli_show_resp;

   /* Update the indices info */
   arg.i_idx.i_inst_ids = snmptm_get_inst_idx(cef_data);
   arg.i_idx.i_inst_len = (SNMPTM_SERVICE_NAME_LEN + 1);
   
   /* Update CLI_REQ structue of MIB_ARG */
   arg.req.info.cli_req.i_wild_card = FALSE;

   ncsstack_init(&arg.stack, NCSMIB_STACK_SIZE); 

   if ((snmptm_send_mib_req(&arg, cef_data)) == NCSCC_RC_FAILURE)
   {
      snmptm_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptm_cef_show_tbls_data
  
  Description   :  This function displays the data of all the tables of an 
                   application instance. Basically it is meant to test the 
                   partial response feature for the CLI-REQ.
 
  Arguments     :  arg_list - Argument list given in command
                   cef_data - Command related DATA
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_cef_show_tbls_data(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   NCSMIB_ARG  arg;
   uns8        space[2048];
   NCSMEM_AID  ma;

   m_NCS_OS_MEMSET(&arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   /* Fill the MIB_ARG structure */
   arg.i_op = NCSMIB_OP_REQ_CLI;
   arg.i_tbl_id = NCSMIB_TBL_SNMPTM_COMMAND;
   arg.i_rsp_fnc = snmptm_cli_show_resp;

   /* Update the indices info */
   arg.i_idx.i_inst_ids = snmptm_get_inst_idx(cef_data);
   arg.i_idx.i_inst_len = (SNMPTM_SERVICE_NAME_LEN + 1);
   
   /* Update CLI_REQ structue of MIB_ARG */
   arg.req.info.cli_req.i_wild_card = FALSE;
   arg.req.info.cli_req.i_cmnd_id = tblone_tblfive_show_data_cmdid;

   ncsstack_init(&arg.stack, NCSMIB_STACK_SIZE); 

   if ((snmptm_send_mib_req(&arg, cef_data)) == NCSCC_RC_FAILURE)
   {
      snmptm_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptm_cef_show_glb_data
  
  Description   :  This function displays the data of all the tables of all 
                   the instances of a particular application. Basically it 
                   is meant to test the wild_card request feature.
 
  Arguments     :  arg_list - Argument list given in command
                   cef_data - Command related DATA
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_cef_show_glb_data(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   NCSMIB_ARG  arg;
   uns8        space[2048];
   NCSMEM_AID  ma;

   m_NCS_OS_MEMSET(&arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   /* Fill the MIB_ARG structure */
   arg.i_op = NCSMIB_OP_REQ_CLI;
   arg.i_tbl_id = NCSMIB_TBL_SNMPTM_COMMAND;
   arg.i_rsp_fnc = snmptm_cli_show_resp;

   /* Note: Updating the indices into arg is not required, as it is a wild-card 
      request need to traverse through all the instances of SNMPTM */

   /* Update CLI_REQ structue of MIB_ARG */
   arg.req.info.cli_req.i_wild_card = TRUE;
   arg.req.info.cli_req.i_cmnd_id = tblone_tblfive_show_glb_data_cmdid;

   ncsstack_init(&arg.stack, NCSMIB_STACK_SIZE); 

   if ((snmptm_send_mib_req(&arg, cef_data)) == NCSCC_RC_FAILURE)
   {
      snmptm_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptm_cef_exit
  
  Description   :  This function exits from the snmptm mode.
 
  Arguments     :  arg_list - Argument list given in command
                   cef_data - Command related DATA
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_cef_exit(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   /* Free the allocated CMD data */
   m_MMGR_FREE_SNMPTM_CMD_DATA(cef_data->i_subsys->i_cef_mode);

   /* Close the CLI session */
   snmptm_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);

   return NCSCC_RC_SUCCESS;
}

#endif  /* NCS_SNMPTM_CLI */

