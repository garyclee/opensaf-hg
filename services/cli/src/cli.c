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

  MODULE NAME:  CLI.C
  
$Header: /ncs/software/leap/base/products/cli/src/cli.c 7     3/28/01 12:01p Claytom $
..............................................................................
    
DESCRIPTION:
      
Source file for external pubished API of CLI
******************************************************************************
*/

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                        Common Include Files.  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "cli.h"

#if (NCS_CLI == 1)

/*****************************************************************************
         L O C A L   F U N C T I O N    D E C E L A R T I O N
*****************************************************************************/
static NCSCONTEXT cli_mode_get(CLI_CB *, uns16);
static NCSCLI_SUBSYS_CB *cli_get_subsys_cb(CLI_CB *);
static uns32 cli_more_time(CLI_CB *, uns32);

/****************************************************************************\
   PROCEDURE NAME :  cli_inst_request
   DESCRIPTION    :  This function process all LM requests. This is the entry
                     point for
                     - creation of CLI instance
                     - deletion of CLI instance     
                     - begin of CLI instance
   ARGUMENTS      :  i_request - Pointer to structure containing info on the
                                 request   
   RETURNS        :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES          :  
\*****************************************************************************/
uns32 cli_inst_request(CLI_INST_INFO *i_request)
{
   CLI_CB   *pCli = 0;
   uns32    rc = NCSCC_RC_SUCCESS;
   
   /* Validate the CLI handle */
   pCli = (CLI_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLI, (uns32)i_request->i_hdl);
   
   if((CLI_INST_REQ_CREATE == i_request->i_req) && (pCli)) {
      /* Wrong call, cli already exists ! */
      m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_CB_ALREADY_EXISTS);
      ncshm_give_hdl(pCli->cli_hdl);
      return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   if((CLI_INST_REQ_CREATE != i_request->i_req) && (!pCli)) {
      /* Wrong call, cli does not exists ! */      
      return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   switch(i_request->i_req) {
   case CLI_INST_REQ_CREATE:
      rc = cli_create(&i_request->info.io_create, &pCli);
      if(NCSCC_RC_SUCCESS != rc) return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);      
      
      m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_CREATE_SUCCESS);
      break;
      
   case CLI_INST_REQ_DESTROY: 
      ncshm_give_hdl(pCli->cli_hdl);
      cli_destroy(&pCli);      
      break;
   
   case CLI_INST_REQ_BEGIN:
      cli_read_input(pCli, i_request->info.i_begin.vr_id);
      ncshm_give_hdl(pCli->cli_hdl);
      break;

   default:
      break;         
   }             
   return rc;
}

/****************************************************************************\
   PROCEDURE NAME :  ncscli_opr_request
   DESCRIPTION    :  This function process all OP requests. This is the entry
                     point for                     
                     - registering CLI request
                     - deregistering CLI request
                     - done CLI request
                     - more time CLI request
                     - mode information CLI request
                     - subsystem data info CLI request
                     - display CLI request
   ARGUMENTS      :  i_request - Pointer to structure containing info on the
                                 request   
   RETURNS        :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES          :  
\****************************************************************************/
uns32 ncscli_opr_request(NCSCLI_OP_INFO *i_request)
{
   CLI_CB   *pCli = 0;
   uns32    rc = NCSCC_RC_SUCCESS;
   
   /* Validate the CLI handle */
   pCli = (CLI_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLI, (uns32)i_request->i_hdl);
   if(!pCli) return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);   
   
   switch(i_request->i_req) {      
   case NCSCLI_OPREQ_REGISTER:          
      rc = cli_register_cmds(pCli, &i_request->info.i_register);      

      if(NCSCC_RC_SUCCESS == rc) m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_REG_CMDS_SUCCESS);
      break;
      
   case NCSCLI_OPREQ_DEREGISTER:      
      rc = cli_deregister_cmds(pCli, &i_request->info.i_deregister);      

      if(NCSCC_RC_SUCCESS == rc) m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_REG_CMDS_SUCCESS);
      break;      

   case NCSCLI_OPREQ_DONE:
      rc = cli_done(pCli, &i_request->info.i_done);
      break;

   case NCSCLI_OPREQ_MORE_TIME:
      rc = cli_more_time(pCli, i_request->info.i_more.i_delay);      
      break;

   case NCSCLI_OPREQ_DISPLAY:
      rc = cli_display(pCli, i_request->info.i_display.i_str);
      break;

   case NCSCLI_OPREQ_GET_DATA:
      i_request->info.i_data.o_info = cli_get_subsys_cb(pCli);
      break;

   case NCSCLI_OPREQ_GET_MODE:
      i_request->info.i_mode.o_data = cli_mode_get(pCli, i_request->info.i_mode.i_lvl);
      break;

   default:
      break;         
   }            

   ncshm_give_hdl(pCli->cli_hdl);
   if(NCSCC_RC_SUCCESS != rc) return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   return rc;
}

/****************************************************************************\
   PROCEDURE NAME :  cli_more_time
   DESCRIPTION    :  This function is used to reset the timer value. CLI starts 
                     the timer and invokes the CEF. The CEF/response function 
                     can restart the timer.
   ARGUMENTS      :  i_key       - CLI Key pointer
                     i_tmrDelay  - Timer Value
   RETURNS        :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE   
   NOTES          :  This function restarts the existing timer with delay            
\****************************************************************************/
uns32 cli_more_time(CLI_CB *pCli, uns32 i_tmrDelay)
{
   /* Stop Timer */
   m_NCS_TMR_STOP(pCli->cefTmr.tid);
   
   /* Restart the timer */
   m_NCS_TMR_START(pCli->cefTmr.tid, i_tmrDelay, (TMR_CALLBACK)pCli->cefTmr.tmrCB,
      &pCli->cli_hdl);
   m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_MORE_TIME);   
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
   Function NAME  :  cli_get_subsys_cb
   DESCRIPTION    :  This routine is used to get the pointer to subsystem data.
   ARGUMENTS      :  i_key - CLI Key pointer
   RETURNS        :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES          :  CLI provides two void pointers for sub systems to store 
                     any information they wanted. This is done because 
                     sub-systems does not have any CB and some commands require
                     instance specific information to be stored and used for 
                     latter commands.
\****************************************************************************/
NCSCLI_SUBSYS_CB *cli_get_subsys_cb(CLI_CB *pCli)
{  
   /* Return the control block of subsystem */   
   if(!pCli->subsys_cb.i_cef_mode)
      pCli->subsys_cb.i_cef_mode = pCli->ctree_cb.trvMrkr->pData;
   return &pCli->subsys_cb;
}

/****************************************************************************\
   PROCEDURE NAME :  cli_display
   DESCRIPTION    :  This routine is provided by CLI to print o/p strings to 
                     screen
   ARGUMENTS      :  pCli - CLI pointer
                     i_str - Used to display the formated string at CLI console.
   RETURNS        :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES          :  This API prints the o/p strings to console represented
                     by console Id which is stored in CB. The console Id is
                     obtained using target Svcs macro m_GET_CONSOLE.
\****************************************************************************/
uns32 cli_display(CLI_CB *pCli, int8 *i_str)
{   
   if(!pCli->cefTmrFlag) {
      m_CLI_FLUSH_STR(i_str, pCli->consoleId);      
      return NCSCC_RC_SUCCESS;
   }   
   return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
}

/****************************************************************************\
   PROCEDURE NAME :  cli_done
   DESCRIPTION    :  This routine is provided by CLI to give the semaphores and 
                     close the timer. The CLI which is pending on the semaphore
                     will wake up.
   ARGUMENTS      :  pCli  - CLI pointer 
                     done  - NCSCLI_DONE_INFO structure pointer
   RETURNS        :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES          :  CLI will block on a semaphore awaiting for the sub-system to
                     complete its task. The sub-system calls this API to indicate
                     that it has completed its processing. CLI will wake and returns
                     to accept commands from console
\****************************************************************************/
uns32 cli_done(CLI_CB *pCli, NCSCLI_OP_DONE *done)
{   
   /* Stop Timer */
   m_NCS_TMR_STOP(pCli->cefTmr.tid);
   
   pCli->cefStatus = done->i_status;
   if(NCSCC_RC_FAILURE == done->i_status)      
      m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_CEF_FAILED);   
   
   if(pCli->semUsgCnt == 1) {
      --pCli->semUsgCnt;         
      
      /* Give Semaphore */
      m_NCS_SEM_GIVE(pCli->cefSem);
   }   
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************\
   PROCEDURE NAME :  cli_mode_get
   DESCRIPTION    :  This routine is used to retrive the parent mode information 
                     based on the level 
   ARGUMENTS      :  pCli  : CLI pointer
                     level : Parent of which level
   RETURNS        :  NCSCONTEXT
   NOTES          :  Own mode is at level 0, parent is at level 1, grandparent 
                     at level 2 asnd so on.
\*****************************************************************************/
NCSCONTEXT cli_mode_get(CLI_CB *pCli, uns16 i_lvl)
{   
   CLI_CMD_NODE *pNode = 0;
   uns16        lvl = 0;
   
   /* Get the CLI control block associated with this cli key */   
   if(!pCli) {
      m_CLI_DBG_SINK_VOID(NCSCC_RC_FAILURE);
      return 0;
   }   

   /* Own mode information */
   if(!i_lvl) return pCli->ctree_cb.trvMrkr->pData;

   /* Parent mode information */
   pNode = pCli->ctree_cb.trvMrkr;
   for(lvl=1; (lvl<=i_lvl) && pNode; lvl++) pNode = pNode->pParent;   
      
   if(pNode) return pNode->pData;
   return 0;
}

#else
extern int dummy;
#endif /* (if NCS_CLI == 1) */
