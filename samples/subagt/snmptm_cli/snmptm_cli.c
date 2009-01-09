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

/* Static function proto-type declarations */
static char *snmptm_print_status_str(uns8 status);
static void snmptm_show_tbl_data(USRBUF *buff, uns32 entry_count, uns32 tbl_id);
static void snmptm_show_glb_tbls_data(USRBUF *buff, uns32 entry_count, uns32 tbl_id, uns32 inst_id);
static void snmptm_show_tbls_data(USRBUF *buff, uns32 entry_count, uns32 tbl_id, uns32 inst_id, uns32 *cmd_tbl_id);


/****************************************************************************
  Name          :  snmptm_cli_register
  
  Description   :  This function registers all the snmptm commands with the
                   CLI.
 
  Arguments     :  bindery  - pointer to the NCSCLI_BINDERY contains the info.
                              required to interact with the CLI.
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : This function will be getting called in the AKE context. 
****************************************************************************/
uns32 snmptm_cli_register(NCSCLI_BINDERY *bindery)
{
   NCSCLI_CMD_LIST data;
   NCSCLI_OP_INFO  reg_info;
   
   /*Register the Protocol with CLI*/
   m_NCS_OS_MEMSET(&data, 0, sizeof(data));
   m_NCS_OS_MEMSET(&reg_info, 0, sizeof(reg_info));

   reg_info.i_hdl = bindery->i_cli_hdl;
   reg_info.i_req = NCSCLI_OPREQ_REGISTER; 
   reg_info.info.i_register.i_bindery = bindery;

   data.i_node = "root/exec";
   data.i_command_mode = "exec";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = 2;

   /* snmptm <number>, treating snmptm instance number and pwe_id are same */
   data.i_command_list[0].i_cmdstr = "snmptm !SNMPTM Instance! @root/exec/snmptm@ NCSCLI_NUMBER !Instance Number!";
   data.i_command_list[0].i_cmd_exec_func = snmptm_cef_conf_mode;

   /* Checks wild-card request */
   data.i_command_list[1].i_cmdstr = "show !Show running system info.!snmptm !SNMPTM Instances! data !Displays the table contents of all the SNMPTM instances!";  
   data.i_command_list[1].i_cmd_exec_func = snmptm_cef_show_glb_data;

   reg_info.info.i_register.i_cmdlist = &data;
   if (NCSCC_RC_SUCCESS !=  ncscli_opr_request(&reg_info))
     return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(&data, 0, sizeof(data));

   data.i_node = "root/exec/snmptm";
   data.i_command_mode = "snmptm";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = 4;

   /*------------------------------------------------------------------------
       Following three commands applies for both tblone & tblfive table's
    ------------------------------------------------------------------------*/
   /* Creates a particular row in a table */
   data.i_command_list[0].i_cmdstr = "create-row !Create an entry! table !Table Id! NCSCLI_NUMBER !Table number 1/5! index !Index of the row! NCSCLI_IPV4 !Ip address: x.x.x.x!";

   /* Displays the data of the table, exact request */
   data.i_command_list[1].i_cmdstr = "show !Display's SNMPTM table data! table !Displays the table contents! NCSCLI_NUMBER !table number 1/5!";

   /* Display the data of the 2 tbls in 2 USRBUFs to check the functionality of the 
     o_partial flag */
   data.i_command_list[2].i_cmdstr = "show !Display's SNMPTM table data! all-table!Displays the snmptm table 1 & 5 contents!";   

   /* Exit from the snmptm mode */
   data.i_command_list[3].i_cmdstr = "exit!Exit from the SNMPTM mode!";


   /* CEF functions */
   /* CEF functions */
   data.i_command_list[0].i_cmd_exec_func = snmptm_cef_tbl_create_row;
   data.i_command_list[1].i_cmd_exec_func = snmptm_cef_show_tbl_data;
   data.i_command_list[2].i_cmd_exec_func = snmptm_cef_show_tbls_data;
   data.i_command_list[3].i_cmd_exec_func = snmptm_cef_exit;

   reg_info.info.i_register.i_cmdlist = &data;
   if (NCSCC_RC_SUCCESS !=  ncscli_opr_request(&reg_info))
     return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  snmptm_cli_done 
  
  Description   :  This function closes the CLI session.
 
  Arguments     :  cli_hdl -  CLI handle info
                   rc - status info
    
  Return Values :  NCSCC_RC_SUCCESSS
 
  Notes         : 
****************************************************************************/
uns32 snmptm_cli_done(uns32 cli_hdl, uns32 rc)
{
   NCSCLI_OP_INFO      done_info;
   SNMPTM_CEF_CMD_DATA *cmd_data;
   NCSCLI_SUBSYS_CB    *data = NULL;
  
   data = snmptmcli_get_subsys_cb(cli_hdl);
   cmd_data = (SNMPTM_CEF_CMD_DATA *)data->i_cef_mode;

   if (cmd_data != NULL)
      cmd_data->tbl_id = 0;

   m_NCS_OS_MEMSET(&done_info, 0, sizeof(NCSCLI_OP_INFO));
        
   done_info.i_hdl = cli_hdl;
   done_info.i_req = NCSCLI_OPREQ_DONE;
   done_info.info.i_done.i_status = rc;
            
   ncscli_opr_request(&done_info);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptmcli_get_subsys_cb
  
  Description   :  This function registers all the snmptm commands with the
                   CLI.
 
  Arguments     :  cli_hdl - CLI handle data
    
  Return Values :  NCSCLI_SUBSYS_CB 
 
  Notes         : 
****************************************************************************/
NCSCLI_SUBSYS_CB *snmptmcli_get_subsys_cb(uns32 cli_hdl)
{
   NCSCLI_OP_INFO get_info;
  
   m_NCS_OS_MEMSET(&get_info, 0, sizeof(NCSCLI_OP_INFO));
        
   get_info.i_hdl = cli_hdl;
   get_info.i_req = NCSCLI_OPREQ_GET_DATA;
   ncscli_opr_request(&get_info);

   return get_info.info.i_data.o_info;
}


/****************************************************************************
  Name          :  snmptm_send_mib_req
  
  Description   :  This function registers all the snmptm commands with the
                   CLI.
 
  Arguments     :  bindery  - CLI bindery data
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_send_mib_req(NCSMIB_ARG *mib_arg, NCSCLI_CEF_DATA *cef_data)
{
   if (cef_data == NULL)
      return NCSCC_RC_FAILURE;

   mib_arg->i_usr_key = cef_data->i_bindery->i_cli_hdl;
   mib_arg->i_mib_key = cef_data->i_bindery->i_mab_hdl;

   if (ncsmib_timed_request(mib_arg, ncsmac_mib_request, 100)
       != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;
      
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptm_cli_set_resp
  
  Description   :  This function registers all the snmptm commands with the
                   CLI.
 
  Arguments     :  bindery  - CLI bindery data
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_cli_set_resp(NCSMIB_ARG *resp)
{
   uns32 rc;

   /* Get the pointer for the command data in CLI control block */
   if (resp->i_usr_key == 0)
      return NCSCC_RC_FAILURE;

   /* Check the Response */
   if (resp->rsp.i_status != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("\nCommand Failed.");
      rc = NCSCC_RC_FAILURE;
   }
   else
      rc = NCSCC_RC_SUCCESS;

   snmptm_cli_done(resp->i_usr_key, rc);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptm_cli_show_resp
  
  Description   :  This function registers all the snmptm commands with the
                   CLI.
 
  Arguments     :  bindery - CLI bindery data
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 snmptm_cli_show_resp(NCSMIB_ARG *resp)
{
   uns8 rsp_completed = FALSE;

   /* Get the pointer for the command data in CLI control block */
   if (resp->i_usr_key == 0)
      return NCSCC_RC_FAILURE;

   /* For a wild-card request, close the CLI session once it receives the 
      response with NCSMIB_OP_RSP_CLI_DONE operation. For an Exact request,
      close the CLI session once it receives the response with o_partial flag
      set to TRUE */ 
   if ((resp->i_op == NCSMIB_OP_RSP_CLI_DONE) ||
       ((resp->i_op == NCSMIB_OP_RSP_CLI) && 
        (resp->rsp.info.cli_rsp.o_partial == FALSE)))
      rsp_completed = TRUE;

   switch(resp->rsp.i_status)
   {
   case NCSCC_RC_FAILURE:
       m_NCS_CONS_PRINTF("Command Failed");
       snmptm_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
       break;

   case NCSCC_RC_SUCCESS:
       {
          SNMPTM_CEF_CMD_DATA *cmd_data;
          NCSCLI_SUBSYS_CB    *data = NULL;
          USRBUF              *buff = resp->rsp.info.cli_rsp.o_answer;
          uns32               buff_len = 0;
          uns32               entry_count = 0;
          uns32               table_id = 0;
          uns32               inst_id = 0;
          char                snmptm_hdr[SNMPTM_HDR_SIZE];
          uns8                *buff_ptr;


          /* Not required to get cmd_mode data for wild-card request */ 
          if (resp->rsp.info.cli_rsp.i_cmnd_id != tblone_tblfive_show_glb_data_cmdid)
          {
             data = snmptmcli_get_subsys_cb(resp->i_usr_key);
             cmd_data = (SNMPTM_CEF_CMD_DATA *)data->i_cef_mode;

             if (cmd_data == NULL)
             {
                m_NCS_CONS_PRINTF("\n\n Not able to find command mode data");
                snmptm_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                return NCSCC_RC_FAILURE;  
             }
          }

          /* Get the size of the buffer */
          buff_len = m_MMGR_LINK_DATA_LEN(buff);

          /* Buffer doesn't has enough data.. incorrect */
          if (buff_len < SNMPTM_HDR_SIZE)
          {
             m_NCS_CONS_PRINTF("\n\n USRBUF doesn't have enough data in it");
             snmptm_cli_done(resp->i_usr_key, NCSCC_RC_SUCCESS);
             return NCSCC_RC_SUCCESS;
          }
         
          if ((buff_ptr = (uns8 *)m_MMGR_DATA_AT_START(buff,
                                                     SNMPTM_HDR_SIZE,
                                                     snmptm_hdr)) == (uns8 *)0) 
          {
             snmptm_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
             return NCSCC_RC_SUCCESS;
          }

          /* Get the instance id */
          inst_id = m_NCS_OS_NTOHL_P(buff_ptr);
          buff_ptr += 4;

          /* This check should be done only for an exact-request,
             instance ids should match */
          if ((cmd_data->instance_id != inst_id) && 
              (resp->rsp.info.cli_rsp.i_cmnd_id != tblone_tblfive_show_glb_data_cmdid))
          {
             m_NCS_CONS_PRINTF("\n\n Wrong Instance Id placed in the arrived buffer");
             snmptm_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
             return NCSCC_RC_FAILURE; 
          }
              
          /* Get the table id */
          table_id = m_NCS_OS_NTOHL_P(buff_ptr);
          buff_ptr += 4;

          m_MMGR_REMOVE_FROM_START(&buff, SNMPTM_HDR_SIZE);
          buff_len -= SNMPTM_HDR_SIZE;

          /* Wrong table id */
          if ((table_id != NCSMIB_TBL_SNMPTM_TBLONE) && 
              (table_id != NCSMIB_TBL_SNMPTM_TBLFIVE))
          {
             m_NCS_CONS_PRINTF("\n\nPacket has wrong TABLE-ID");
             snmptm_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
             return NCSCC_RC_FAILURE;
          }

          /* No data in the USRBUF, just return */
          if (buff_len == 0)
          {
             m_NCS_CONS_PRINTF("\n\n USRBUF doesn't have enough data in it");
             snmptm_cli_done(resp->i_usr_key, NCSCC_RC_SUCCESS);
             return NCSCC_RC_FAILURE;
          }

          switch(table_id)
          {
          case NCSMIB_TBL_SNMPTM_TBLONE:
             /* Incorrect alignment of the buffer format,
                should be exact to the multiple number of SNMPTM_TBL_ENTRY_SIZE */ 
             if (buff_len % SNMPTM_TBLONE_ENTRY_SIZE) 
             {
                m_NCS_CONS_PRINTF("\n\n USRBUF size is not aligned to TBLONE entries");
                snmptm_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                return NCSCC_RC_FAILURE;
             }

             entry_count = (buff_len/SNMPTM_TBLONE_ENTRY_SIZE);
             break;

          case NCSMIB_TBL_SNMPTM_TBLFIVE:
             /* Incorrect alignment of the buffer format,
                should be exact to the multiple number of SNMPTM_TBL_ENTRY_SIZE */ 
             if (buff_len % SNMPTM_TBLFIVE_ENTRY_SIZE) 
             {
                m_NCS_CONS_PRINTF("\n\n USRBUF size is not aligned to TBLFIVE entries");
                snmptm_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                return NCSCC_RC_FAILURE;
             }

             entry_count = (buff_len/SNMPTM_TBLFIVE_ENTRY_SIZE);
             break;

          default:
             snmptm_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
             return NCSCC_RC_FAILURE;
          }

          /* No entries in the USRBUF, just return */
          if (entry_count == 0)
          {
             m_NCS_CONS_PRINTF("\n\n USRBUF doesn't have any entries in it.");
             snmptm_cli_done(resp->i_usr_key, NCSCC_RC_SUCCESS);
             return NCSCC_RC_FAILURE;
          }
       
          switch(resp->rsp.info.cli_rsp.i_cmnd_id)
          {
          case tblone_show_all_rows_cmdid:
              if (table_id != NCSMIB_TBL_SNMPTM_TBLONE)
              {
                 snmptm_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                 return NCSCC_RC_FAILURE;
              }
                 
              if (cmd_data->tbl_id != table_id)
              { 
                 m_NCS_CONS_PRINTF("\n\n          +----------------------------------+");
                 m_NCS_CONS_PRINTF("\n          + TABLE-ONE DATA (Instance ID# %d)  +", inst_id);
                 m_NCS_CONS_PRINTF("\n          +----------------------------------+");
                 m_NCS_CONS_PRINTF("\nIp Address                    RowStatus         Counters");
                 m_NCS_CONS_PRINTF("\n===========================================================");

                 cmd_data->tbl_id = table_id;
              }

              snmptm_show_tbl_data(buff, entry_count, table_id);
              break;    
              
          case tblfive_show_all_rows_cmdid:
              if (table_id != NCSMIB_TBL_SNMPTM_TBLFIVE)
              {
                 snmptm_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                 return NCSCC_RC_FAILURE;
              }

              if (cmd_data->tbl_id != table_id)
              {
                 m_NCS_CONS_PRINTF("\n\n +-----------------------------------+");
                 m_NCS_CONS_PRINTF("\n + TABLE-FIVE DATA (Instance ID# %d)  +", inst_id);
                 m_NCS_CONS_PRINTF("\n +-----------------------------------+");
                 m_NCS_CONS_PRINTF("\nIp Address                   RowStatus");
                 m_NCS_CONS_PRINTF("\n========================================");

                 cmd_data->tbl_id = table_id;
              }

              snmptm_show_tbl_data(buff, entry_count, table_id);
              break;    
              
          case tblone_tblfive_show_data_cmdid:
              snmptm_show_tbls_data(buff, entry_count, table_id, inst_id,  &cmd_data->tbl_id); 
              break;    
              
          case tblone_tblfive_show_glb_data_cmdid:
              snmptm_show_glb_tbls_data(buff, entry_count, table_id, inst_id);
              break;    
              
          default:
              break;    
          }
       }
       break;

   default:
       break;
   }
 
   /* Request processing is completes */ 
   if (rsp_completed == TRUE)
   {
      snmptm_cli_done(resp->i_usr_key, NCSCC_RC_SUCCESS);
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptm_print_status_str
  
  Description   :  This function returns the appropriate string for the given
                   row_status value.
 
  Arguments     :  status - Row Status value
    
  Return Values :  row_status string.
 
  Notes         : 
****************************************************************************/
static char *snmptm_print_status_str(uns8 status)
{
   switch(status)
   {
   case NCSMIB_ROWSTATUS_ACTIVE:
       return "Active";
       
   case NCSMIB_ROWSTATUS_NOTINSERVICE:
       return "Not in Service";
       
   case NCSMIB_ROWSTATUS_NOTREADY:
       return "Not Ready";

   case NCSMIB_ROWSTATUS_CREATE_GO:
       return "Create & Go";
       
   case NCSMIB_ROWSTATUS_CREATE_WAIT:
       return "Create & Wait";
       
   case NCSMIB_ROWSTATUS_DESTROY:
       return "Destroy";

   default:
     break;
   }

   return "Unknown";
}


/****************************************************************************
  Name          :  snmptm_show_tbl_data
  
  Description   :  This function prints the contents of the USRBUF in 
                   specific format based on the table id value.
 
  Arguments     :  buff - pointer to the USRBUF  
                   entry_count - no of tbl specific entries exists in the USRBUF
                   tbl_id - table id of the table whose data is decoding for 
                            print
                   
  Return Values :  - nothing - 
 
  Notes         : 
****************************************************************************/
static void snmptm_show_tbl_data(USRBUF *buff, uns32 entry_count, uns32 tbl_id)
{
   char  local_buff[SNMPTM_TBLONE_ENTRY_SIZE];
   char  addr_str[18];
   uns32 i;
   uns32 ip_addr = 0;
   uns32 cntr = 0;
   uns8  status = 0;
   uns8  *buff_ptr;
   
   for (i=0; i<entry_count; i++)
   {
      if (tbl_id != NCSMIB_TBL_SNMPTM_TBLONE)
      {
         if ((buff_ptr = (uns8*)m_MMGR_DATA_AT_START(buff,
                                              SNMPTM_TBLFIVE_ENTRY_SIZE,
                                              local_buff)) == (uns8 *)0) 
            return;
      }
      else
      {
         if ((buff_ptr = (uns8*)m_MMGR_DATA_AT_START(buff,
                                              SNMPTM_TBLONE_ENTRY_SIZE,
                                              local_buff)) == (uns8 *)0) 
            return;
      }

      ip_addr = m_NCS_OS_NTOHL_P(buff_ptr);
      buff_ptr += 4;

      status = (uns8)m_NCS_OS_NTOHL_P(buff_ptr);
      buff_ptr += 4;

      if (tbl_id == NCSMIB_TBL_SNMPTM_TBLONE)
         cntr = m_NCS_OS_NTOHL_P(buff_ptr);
 
      m_NCS_OS_MEMSET(&addr_str, 0, sizeof(addr_str));
     
      sysf_sprintf(addr_str, "%d.%d.%d.%d", (uns8)(ip_addr >> 24),
                                            (uns8)(ip_addr >> 16),
                                            (uns8)(ip_addr >> 8),
                                            (uns8)(ip_addr)); 
      
      if (tbl_id != NCSMIB_TBL_SNMPTM_TBLONE)
      {
         m_NCS_CONS_PRINTF("\n %-16s            %s", addr_str, snmptm_print_status_str(status));
         m_MMGR_REMOVE_FROM_START(&buff, SNMPTM_TBLFIVE_ENTRY_SIZE);
      }
      else
      {
         m_NCS_CONS_PRINTF("\n %-16s              %s               %d",
                         addr_str, snmptm_print_status_str(status), cntr);
         m_MMGR_REMOVE_FROM_START(&buff, SNMPTM_TBLONE_ENTRY_SIZE);
      }
   } /* End of for() loop */
   
   m_NCS_CONS_PRINTF("\n");

   return;
}


/****************************************************************************
  Name          :  snmptm_show_tbls_data
  
  Description   :  This function prints the contents of the USRBUF in 
                   specific format based on the table id value.
 
  Arguments     :  buff - pointer to the USRBUF  
                   entry_count - no of tbl specific entries exists in the USRBUF
                   tbl_id - table id of the table whose data is decoding for 
                            print
                   cmd_tbl_id - table-id of the earlier/first response of the 
                                request.
                   
  Return Values :  - nothing - 
 
  Notes         : 
****************************************************************************/
static void snmptm_show_tbls_data(USRBUF *buff, uns32 entry_count, uns32 tbl_id, uns32 inst_id, uns32 *cmd_tbl_id)
{
   /* If it is a first response for the request made then print the following 
      display headings */
   if (*cmd_tbl_id != tbl_id)
   {
      if (tbl_id == NCSMIB_TBL_SNMPTM_TBLONE)
      {
         m_NCS_CONS_PRINTF("\n\n          +----------------------------------+");
         m_NCS_CONS_PRINTF("\n          + TABLE-ONE DATA (Instance ID# %d)  +", inst_id);
         m_NCS_CONS_PRINTF("\n          +----------------------------------+");
         m_NCS_CONS_PRINTF("\nIp Address                     RowStatus            Counters");
         m_NCS_CONS_PRINTF("\n==============================================================");
         *cmd_tbl_id = tbl_id;
      }
      else
      {
         m_NCS_CONS_PRINTF("\n\n  +-----------------------------------+");
         m_NCS_CONS_PRINTF("\n  + TABLE-FIVE DATA (Instance ID# %d)  +", inst_id);
         m_NCS_CONS_PRINTF("\n  +-----------------------------------+");
         m_NCS_CONS_PRINTF("\nIp Address                   RowStatus");
         m_NCS_CONS_PRINTF("\n========================================");
         *cmd_tbl_id = tbl_id;
      }
   }

   snmptm_show_tbl_data(buff, entry_count, tbl_id);

   return;
}


/****************************************************************************
  Name          :  snmptm_show_gbl_tbls_data
  
  Description   :  This function prints the contents of the USRBUF in 
                   specific format based on the table id value.
 
  Arguments     :  buff - pointer to the USRBUF  
                   entry_count - no of tbl specific entries exists in the USRBUF
                   tbl_id - table id of the table whose data is decoding for 
                            print
                   
  Return Values :  - nothing - 
 
  Notes         : 
*****************************************************************************/
static void  snmptm_show_glb_tbls_data(USRBUF *buff, uns32 entry_count, uns32 tbl_id, uns32 inst_id)
{
   if (tbl_id == NCSMIB_TBL_SNMPTM_TBLONE)
   {
      m_NCS_CONS_PRINTF("\n\n          +----------------------------------+");
      m_NCS_CONS_PRINTF("\n          + TABLE-ONE DATA (Instance ID# %d)  +", inst_id);
      m_NCS_CONS_PRINTF("\n          +----------------------------------+");
      m_NCS_CONS_PRINTF("\nIp Address                    RowStatus           Counters");
      m_NCS_CONS_PRINTF("\n============================================================");
   }
   else
   {
      m_NCS_CONS_PRINTF("\n\n  +-----------------------------------+");
      m_NCS_CONS_PRINTF("\n  + TABLE-FIVE DATA (Instance ID# %d)  +", inst_id);
      m_NCS_CONS_PRINTF("\n  +-----------------------------------+");
      m_NCS_CONS_PRINTF("\nIp Address                  RowStatus");
      m_NCS_CONS_PRINTF("\n=======================================");
   }

   snmptm_show_tbl_data(buff, entry_count, tbl_id);

   return;
}

#endif /* NCS_SNMPTM_CLI */
