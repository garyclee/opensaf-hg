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

   MODULE NAME:  CLIIO.C
  
..............................................................................
    
   DESCRIPTION:
      
   Source file for CLI input and output and its utility functions.

******************************************************************************
*/

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  Common Include Files.
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "cli.h"

#if(NCS_CLI == 1)
#if (NCS_PC_CLI == 1)
NCSCONTEXT gl_pcode_sem_id = 0;
extern NCS_BOOL   gl_cli_valid;
#endif
EXTERN_C uns32 sysf_get_char_map(uns32 ch);
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  Variables 
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/* Global error string array */
static int8 *cli_con_err_str[] =
{
   "Invalid input detected at '^' marker.",
   "Incomplete command.",
   "Ambiguous command",
   "Password: ",
   "Bad secrets",
   "Unrecognized command",
   "File Open Error",
   "File Read Error",
   "Login command encountered",
   "This is mode change command.But it's respective mode is not registered." 
};

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  INPUT/OUTPUT FUNCTIONS
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*****************************************************************************
   PROCEDURE NAME :  cli_stricmp
   DESCRIPTION    :  This function converts the input string to lower case and 
                     compares the string.
   ARGUMENTS      :  i_str1 - Source string
                     i_str2 - Compare string
   RETURNS        :  none
   NOTES          :          
*****************************************************************************/
uns32 cli_stricmp(int8 *i_str1, int8 *i_str2)
{
   int8  *inbuf = 0;
   uns32 i=0, len = 0, rc = 0;
   
   for(i=0; i<strlen(i_str1); i++) {
      if(i_str1[i] == CLI_CONS_BLANK_SPACE) break;    
   }
   
   len = i + 1;
   if(0 == (inbuf = (int8 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(len)))
      return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   
   memset(inbuf, '\0', len);   
   for(i=0; i<len-1; i++) inbuf[i] = (int8)tolower(i_str1[i]);   
   inbuf[i] = '\0';
   
   rc = strcmp(inbuf, i_str2);
   m_MMGR_FREE_CLI_DEFAULT_VAL(inbuf);   
   return rc;
}

#define m_CLI_REDISPLAY_CMD(pCli, str)\
{\
   if(0 != pCli->ctree_cb.htryMrkr.cmd_node)\
      strncpy(str, pCli->ctree_cb.htryMrkr.cmd_node->pCmdStr,sizeof(str)-1);\
}


void cli_clean_history(CLI_CMD_HISTORY *node)
{
   CLI_CMD_HISTORY *prev = 0, *curr = 0;
   
   for(curr=node; curr;) {
      prev = curr;
      curr = curr->next;
      m_MMGR_FREE_CLI_DEFAULT_VAL(prev->pCmdStr);
      m_MMGR_FREE_CLI_CMD_HISTORY(prev);
   }
}

/*****************************************************************************
   PROCEDURE NAME :  cli_set_cmd_into_history
   DESCRIPTION    :
   ARGUMENTS      :  pCli     - Pointer of Control block
                     i_cmdstr - Adds the command into command history list
   RETURNS        :  none
   NOTES          :                  
*****************************************************************************/
void cli_set_cmd_into_history(CLI_CB *pCli, int8 *i_cmdstr)
{
   CLI_CMD_HISTORY   *pCurr = 0;
   CLI_CMD_HISTORY   *pPrev = 0;   
   
   if(!i_cmdstr) return;

   if(0 == pCli->ctree_cb.cmdHtry) {
      if(0 == (pCli->ctree_cb.cmdHtry = m_MMGR_ALLOC_CLI_CMD_HISTORY))
         return;
      
      memset(pCli->ctree_cb.cmdHtry, 0, sizeof(CLI_CMD_HISTORY));
      memset(&pCli->ctree_cb.htryMrkr, 0, sizeof(CLI_CMD_HISTORY_MARKER));

      pCli->ctree_cb.cmdHtry->pCmdStr = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(i_cmdstr)+1);
      if(!pCli->ctree_cb.cmdHtry->pCmdStr) return;
      strncpy(pCli->ctree_cb.cmdHtry->pCmdStr, i_cmdstr,strlen(i_cmdstr));      
      
      pCli->ctree_cb.htryMrkr.cmd_node = pCli->ctree_cb.cmdHtry;
      pCli->ctree_cb.htryMrkr.cmd_count++;
   }
   else {
      if((pCli->ctree_cb.htryMrkr.cmd_count + 1) > CLI_CMD_HISTORY_SIZE) {
         pCurr = pCli->ctree_cb.cmdHtry;
         pCli->ctree_cb.cmdHtry = pCli->ctree_cb.cmdHtry->next;
         pCli->ctree_cb.cmdHtry->prev = 0;
         pCli->ctree_cb.htryMrkr.cmd_count--;
         
         if(pCli->ctree_cb.htryMrkr.cmd_node == pCurr)
            pCli->ctree_cb.htryMrkr.cmd_node = 0;         
         m_MMGR_FREE_CLI_DEFAULT_VAL(pCurr->pCmdStr);
         m_MMGR_FREE_CLI_CMD_HISTORY(pCurr);
         pCurr = 0;           
      }
      
      pCurr = pCli->ctree_cb.cmdHtry;
      while(pCurr) {                
         if(0 == strcmp(pCurr->pCmdStr, i_cmdstr))
         {
            pCli->ctree_cb.htryMrkr.cmd_node = pCurr;
            return;
         }
         
         pPrev = pCurr;
         pCurr = pCurr->next;
      }
      
      pCurr = pPrev->next = m_MMGR_ALLOC_CLI_CMD_HISTORY;
      if(!pCurr) return;
      
      memset(pCurr, 0, sizeof(CLI_CMD_HISTORY));      
      pCurr->pCmdStr = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(i_cmdstr)+1);
      if(!pCurr->pCmdStr) return;
      strncpy(pCurr->pCmdStr, i_cmdstr,strlen(i_cmdstr));
      
      pCurr->prev = pPrev;                
      pCli->ctree_cb.htryMrkr.cmd_node = pCurr;
      pCli->ctree_cb.htryMrkr.cmd_count++;
   }
}

/*****************************************************************************
   PROCEDURE NAME :  cli_get_cmd_from_history
   DESCRIPTION    :     

   ARGUMENTS      :  pCli        - Pointer of Control block
                     i_str       - Adds the command into command history list
                     i_direction - Direction of movement ie Forward/Backward
   RETURNS        :  none
   NOTES          :                  
*****************************************************************************/
void cli_get_cmd_from_history(CLI_CB *pCli, int8 *i_str, uns32 i_direction)
{
   if(!pCli->ctree_cb.htryMrkr.cmd_node) return;

   if(CLI_HIS_BWD_MVMT == i_direction) {               
      strncpy(i_str, pCli->ctree_cb.htryMrkr.cmd_node->pCmdStr,CLI_BUFFER_SIZE-1);
      
      if(0 != pCli->ctree_cb.htryMrkr.cmd_node->prev) {
         pCli->ctree_cb.htryMrkr.cmd_node = 
            pCli->ctree_cb.htryMrkr.cmd_node->prev;                
      }
   }     
   else if(CLI_HIS_FWD_MVMT == i_direction) {        
      if(0 != pCli->ctree_cb.htryMrkr.cmd_node->next) {
         pCli->ctree_cb.htryMrkr.cmd_node = pCli->ctree_cb.htryMrkr.cmd_node->next;                
         strncpy(i_str, pCli->ctree_cb.htryMrkr.cmd_node->pCmdStr,CLI_BUFFER_SIZE-1);
      }
   }   
}

/*****************************************************************************
   PROCEDURE NAME :  cli_move_char_backward
   DESCRIPTION    :  The function is used to move one character back
   ARGUMENTS      :  i_count - Current cursor position
   RETURNS        :  Current position of character after movement
   NOTES          :
*****************************************************************************/
uns32 cli_move_char_backward(uns32 i_count)
{    
   putchar(8);
   i_count--;    
   
   return i_count;
}

/*****************************************************************************
   Function NAME  :  cli_move_char_forward
   DESCRIPTION    :  The function is used to move one character forward
   ARGUMENTS      :  i_buffer - Buffer containing character
                     i_count  - Current position of character before movement
   RETURNS        :  Current position of character after movement
   NOTES          :
*****************************************************************************/
uns32 cli_move_char_forward(int8 *i_buffer, uns32 i_count)
{    
   uns32 data_len = strlen(i_buffer);
   
   if(i_count < data_len) {
      putchar(i_buffer[i_count]);
      i_count++;
   }
   else putchar(CLI_CONS_BEEP);   
   return i_count;
}

/*****************************************************************************
   PROCEDURE NAME :  cli_delete_char
   DESCRIPTION    :  Delete single character
   ARGUMENTS      :  i_buffer - Buffer containing character
                     i_count  - Current position of character before movement
   RETURNS        :  Current position of character after movement
   NOTES          :
*****************************************************************************/
uns32 cli_delete_char(int8 *i_buffer, uns32 i_count)
{    
   putchar(8);
   putchar(32);
   putchar(8);
   i_buffer[i_count - 1] = 0;
   i_count--;
   
   return i_count;
}

/*****************************************************************************
   PROCEDURE NAME :  CLI_DELETE_WORD
   DESCRIPTION    :  Delete the whole word
   ARGUMENTS      :  i_buffer - Buffer containing character
                     i_count  - Current position of character before movement
   RETURNS        :  Current position of character after movement
   NOTES          :          
*****************************************************************************/
uns32 cli_delete_word(int8 *i_buffer, uns32 i_count)
{
   /* If trailing chars are spaces, then delete all spaces */
   if(CLI_CONS_BLANK_SPACE == i_buffer[i_count - 1]) {
      while(CLI_CONS_BLANK_SPACE == i_buffer[i_count - 1] && i_count) {
         cli_delete_char(i_buffer, i_count);
         i_count--;
      }
   }
   /* Now delete all characters until a space is found */        
   while(CLI_CONS_BLANK_SPACE != i_buffer[i_count - 1] && i_count) {
      cli_delete_char(i_buffer, i_count);
      i_count--;
   }   
   return i_count;
}

/*****************************************************************************
   PROCEDURE NAME :  cli_delete_beginning_of_line
   DESCRIPTION    :  Delete all characters to the begining of line
   ARGUMENTS      :  i_buffer - Buffer containing character
                     i_count  - Current position of character before movement
   RETURNS        :  Current position of character after movement
   NOTES          :          
*****************************************************************************/
uns32 cli_delete_beginning_of_line(int8 *i_buffer, uns32 i_count)
{
   while(*i_buffer && i_count) {
      cli_delete_char(i_buffer, i_count);
      i_count--;
   }       
   return i_count;
}

/*****************************************************************************
   PROCEDURE NAME :  cli_move_beginning_of_line
   DESCRIPTION    :  Moves the cursor to the begining of line
   ARGUMENTS      :  i_count - Current position of character before movement
   RETURNS        :  Current position of character after movement
   NOTES          :          
*****************************************************************************/
uns32 cli_move_beginning_of_line(uns32 i_count)
{
   while(i_count) i_count = cli_move_char_backward(i_count);   
   return i_count;
}

/*****************************************************************************
   PROCEDURE NAME :  CLI_MOVE_END_OF_LINE
   DESCRIPTION    :  Moves the cursor to the end of line
   ARGUMENTS      :  i_buffer - Buffer containing character
                     count    - Current position of character before movement      
   RETURNS        :  Current position of character after movement        
   NOTES          :          
*****************************************************************************/
uns32 cli_move_end_of_line(int8 *i_buffer, uns32 i_count)
{
   uns32 data_len = strlen(i_buffer);
   
   while(i_count < data_len) i_count = cli_move_char_forward(i_buffer, i_count);           
   return i_count;
}

/*****************************************************************************
   PROCEDURE NAME :  cli_read_input
  
   DESCRIPTION    :  This is main function that implements Getc/Putc action. It
                     waits in a while loop to carry out the required operation 
                     from the console.    
   ARGUMENTS      :  pCli  - Pointer of Control block
                     vr_if - Virtual router id      
   RETURNS        :  none        
   NOTES          :          
*****************************************************************************/
void cli_read_input(CLI_CB *pCli, NCS_VRID vr_id)
{       
   uns32 ch, chCnt = 0, index = 0;
   int8                 buffer[CLI_BUFFER_SIZE];
   CLI_SESSION_INFO     session_info;    
   CLI_CMD_NODE         *context_node = 0;
   CLI_EXECUTE_PARAM    param;
   uns32                cmd_len = 0;
   NCS_BOOL              display_flag = FALSE,
                        passwd_flag = FALSE,   
                        cmd_help    = FALSE;
   int8                 error_pos,
                        display_str[256];  
   NCS_BOOL              is_arrow = FALSE;
#if (NCS_PC_CLI == 1)
   NCS_BOOL              pcode_active = TRUE;
#endif
   FILE    *input; 
   struct termios initial_settings;
   struct termios new_settings;
   
   memset(&param, 0, sizeof(param));
/*   
#if (NCS_PC_CLI == 1)
    Do not prompt - PowerCode is active 
#else
   m_CLI_SET_PROMPT(CLI_ROUTER_NAME, FALSE);
#endif
   */
   
   /*Reset the buffer */
   memset(buffer, 0, CLI_BUFFER_SIZE);    
   strcpy(session_info.prompt_string, CLI_ROUTER_NAME);
   
   m_CLI_SET_CURRENT_CONTEXT(pCli, TRUE);
   
   /* Put the root node in command mode stack */
   pCli->ctree_cb.modeStack[++pCli->ctree_cb.modeStackPtr] = 
      pCli->ctree_cb.trvMrkr;
   
   m_GET_TIME_STAMP(pCli->cli_last_active_time); /*Aded to fix the bug 58609 */
   cli_timer_start(pCli);               /* Added to fix the bug 58609 */
   if(pCli->user_access_level == NCSCLI_USER_FIND_ERROR) {
       cli_lib_shut_except_task_release();
       exit(0);
   }
   if(pCli->user_access_level == (NCSCLI_USER_MASK>>(NCSCLI_ACCESS_END-NCSCLI_SUPERUSER_ACCESS-1)))
   {
       cli_display(pCli,"ncs cli user is an authenticated super user.");
       m_LOG_NCSCLI_STR(NCSFL_LC_HEADLINE,NCSFL_SEV_NOTICE,NCSCLI_HDLN_CLI_USER,"super user");
   }
   else if(pCli->user_access_level == (NCSCLI_USER_MASK>>(NCSCLI_ACCESS_END-NCSCLI_ADMIN_ACCESS-1)))
   {
       cli_display(pCli,"ncs cli user is an authenticated admin user.");
       m_LOG_NCSCLI_STR(NCSFL_LC_HEADLINE,NCSFL_SEV_NOTICE,NCSCLI_HDLN_CLI_USER,"admin user");
   }
   else if(pCli->user_access_level == (NCSCLI_USER_MASK>>(NCSCLI_ACCESS_END-NCSCLI_VIEWER_ACCESS-1)))
   {
       cli_display(pCli,"ncs cli user is an authenticated viewer user.");
       m_LOG_NCSCLI_STR(NCSFL_LC_HEADLINE,NCSFL_SEV_NOTICE,NCSCLI_HDLN_CLI_USER,"viewer user");
   }
#if (NCS_PC_CLI == 1)
   /* Do not prompt - PowerCode is active */
#else
   m_CLI_SET_PROMPT(CLI_ROUTER_NAME, FALSE);
#endif
   {
       if (!isatty(fileno(stdout))) {
           fprintf(stderr,"You are not a terminal, OK.\n");
       }

      input = fopen("/dev/tty", "r");
      if(!input) {
          fprintf(stderr, "Unable to open /dev/tty\n");
          exit(1);
      }
   }

   do {
#if (NCS_PC_CLI == 1)
      if(pcode_active == TRUE) {
         /* Force CLI task to block to ensure PowerCode
          * task gets a chance to run
         */
         m_NCS_TASK_SLEEP(100); 
         
         /* Block untill PowerCode is done */
         m_NCS_SEM_TAKE(gl_pcode_sem_id);
         
         /* PowerCode is done */
         pcode_active = FALSE;
         m_CLI_SET_PROMPT(session_info.prompt_string, display_flag);
      } 
#endif
       
      param.i_execcmd = TRUE;  /* execute the command */
      {
          memset(&initial_settings, '\0', sizeof(initial_settings));
          memset(&new_settings, '\0', sizeof(new_settings));
          if(tcgetattr(fileno(input),&initial_settings)){
             fprintf(stderr,"tcgetattr: could not get attributes, reason: %s\n", strerror(errno));
          }
          new_settings = initial_settings;
          new_settings.c_lflag &= ~ICANON;
          new_settings.c_lflag &= ~ECHO;
          new_settings.c_cc[VMIN] = 1;
          new_settings.c_cc[VTIME] = 0;
          new_settings.c_lflag &= ~ISIG;
        tcnewset:
          if(tcsetattr(fileno(input), TCSANOW, &new_settings) != 0) {
             fprintf(stderr,"tcsetattr: could not set attributes, reason: %s\n",strerror(errno));
             if(EINTR == errno){
                goto tcnewset;
             }
             else {
                cli_lib_shut_except_task_release();
                exit(1);
             }
          }
          ch = sysf_get_char_map(fgetc(input));
        tcinitset: 
          if(tcsetattr(fileno(input),TCSANOW,&initial_settings) != 0) {
             fprintf(stderr,"tcsetattr: could not reset attributes, reason: %s\n", strerror(errno));
             if(EINTR == errno){
                goto tcinitset;
             }
             else {
                cli_lib_shut_except_task_release();
                exit(1);
             }
          }
      }

      m_GET_TIME_STAMP(pCli->cli_last_active_time);/*Added to fix bug 58609*/
      switch (ch) {
      case 0:
         is_arrow = TRUE;         
         {
             fclose(input);

             if (!isatty(fileno(stdout))) {
                  fprintf(stderr,"You are not a terminal, OK.\n");
             }

             input = fopen("/dev/tty", "r");
             if(!input) {
                 fprintf(stderr, "Unable to open /dev/tty\n");
                 exit(1);
             }
         }
         break; /* 0 */
         
      /* redisplay the current command line */
      case CONTROL(CLI_CONS_REDISPLAY_CMD):  
         if(passwd_flag) goto default_val;  
         
         pCli->cefTmrFlag = FALSE;
         if(0 != chCnt) chCnt = cli_delete_beginning_of_line(buffer, chCnt);
         
         m_CLI_REDISPLAY_CMD(pCli, buffer);                
         cli_display(pCli, buffer);
         chCnt = strlen(buffer);
         break; /* CONTROL(CLI_CONS_REDISPLAY_CMD) */
         
      /* delete all chars from cursor to end of cmd line */
      case CONTROL(CLI_CONS_DELETE_REST):   
         if(passwd_flag) goto default_val;  
         
         index = chCnt;
         chCnt = cli_move_end_of_line(buffer, chCnt);         
         
         while(chCnt != index) chCnt = cli_delete_char(buffer, chCnt);         
         break; /* CONTROL(CLI_CONS_DELETE_REST) */
         
      /* delete all char from cursor to beginning of cmd line */
      case CONTROL(CLI_CONS_CLEAR_ALL):   
         if(passwd_flag) goto default_val;  
         
         if(0 != chCnt) chCnt = cli_delete_beginning_of_line(buffer, chCnt);
         else putchar(CLI_CONS_BEEP);         
         break; /* CONTROL(CLI_CONS_CLEAR_ALL) */
         
      /* delete the word to left of cursor */
      case CONTROL(CLI_CONS_DELETE_WORD):  
         if(passwd_flag) goto default_val;  
         
         if(0 != chCnt) chCnt = cli_delete_word(buffer, chCnt);
         else putchar(CLI_CONS_BEEP);         
         break; /* CONTROL(CLI_CONS_DELETE_WORD) */
         
      /* Move the cursor to beginning of the line */
      case CONTROL(CLI_CONS_START_OF_LINE):  
         if(passwd_flag) goto default_val;  
         
         if(0 != chCnt) chCnt = cli_move_beginning_of_line(chCnt);
         else putchar(CLI_CONS_BEEP);         
         break; /* CONTROL(CLI_CONS_START_OF_LINE) */
         
      /* Move the cursor to end of the line */
      case CONTROL(CLI_CONS_END_OF_LINE):  
         if(passwd_flag) goto default_val;  
         
         chCnt = cli_move_end_of_line(buffer, chCnt);
         break; /* CONTROL(CLI_CONS_END_OF_LINE) */
         
      /* Get command from history in forward direction from this point */
      case CLI_CONS_DOWN_ARROW:
         if(!is_arrow || passwd_flag) goto default_val;
      case CONTROL(CLI_CONS_HISTORY_FWD):         
         if(passwd_flag) goto default_val;  
         pCli->cefTmrFlag = FALSE;
         if(0 != chCnt) chCnt = cli_delete_beginning_of_line(buffer, chCnt);
         
         cli_get_cmd_from_history(pCli, buffer, CLI_HIS_FWD_MVMT);                 
         cli_display(pCli, buffer);
         chCnt = strlen(buffer);           
         if(is_arrow) is_arrow = FALSE;
         break; /* CONTROL(CLI_CONS_HISTORY_FWD) */
         
      /* Get command from history in backward direction from this point */
      case CLI_CONS_UP_ARROW:
         if(!is_arrow || passwd_flag) goto default_val;
      case CONTROL(CLI_CONS_HISTORY_BWD):                      
         if(passwd_flag) goto default_val;  
         
         pCli->cefTmrFlag = FALSE;
         if(0 != chCnt) chCnt = cli_delete_beginning_of_line(buffer, chCnt);
         
         cli_get_cmd_from_history(pCli, buffer, CLI_HIS_BWD_MVMT);                 
         cli_display(pCli, buffer);
         chCnt = strlen(buffer);           
         if(is_arrow) is_arrow = FALSE;
         break; /* CONTROL(CLI_CONS_HISTORY_BWD) */
         
      /* Exit key */
      case CONTROL(CLI_CONS_EXIT_MODE):
         cli_current_mode_exit(pCli, &session_info, &display_flag);
         if(passwd_flag) {
            passwd_flag = FALSE;                                       
            m_CLI_SET_CURRENT_CONTEXT(pCli, TRUE);
         }
         
         /*Reset the buffer */                    
         memset(buffer, 0, CLI_BUFFER_SIZE);                        
         chCnt = 0;         
         break; /* CONTROL(CLI_CONS_EXIT_MODE) */
         
      /* Should exit from CLI */
      case CONTROL(CLI_CONS_END): 
          cli_lib_shut_except_task_release(); 
          exit(0);
          /*return;   CONTROL(CLI_CONS_END) */
      
         
      /* Carriage return */
      case CLI_CONS_RETURN:
      case CLI_CONS_EOL:  
         if(chCnt+1 >= CLI_BUFFER_SIZE) {           
            m_CLI_DBG_SINK_VOID(NCSCC_RC_FAILURE);
            memset(buffer, '\0', CLI_BUFFER_SIZE);
            chCnt = 0;
            break;
         }
         
         cmd_len = strlen(buffer);
         if (cmd_len > 0)
         {
            for(index=cmd_len-1; index>=1; index--) {
               if(buffer[index] == CLI_CONS_BLANK_SPACE) {
                 buffer[index] = '\0';
                 chCnt--;
               }
               else break;
            }
         }
         
         cmd_len = strlen(buffer);
         if(0 == cmd_len) {                    
            m_CLI_SET_PROMPT(session_info.prompt_string, display_flag);
            break;
         }
#if (NCSCLI_FILE == 1)
         /* Check if user has requested cmds executed from file */
         if(m_NCSCLI_EXEC_FILE == buffer[0] || m_NCSCLI_VERIFY_FILE == buffer[0])
         {               
            buffer[cmd_len] = '\0';  
            
            m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
            if(0 == context_node) {                    
               m_CLI_SET_CURRENT_CONTEXT(pCli, TRUE);
               m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
            }
            
            pCli->ctree_cb.cmdElement = context_node->pCmdl;
            param.i_cmdbuf = buffer;
            if ( m_NCSCLI_VERIFY_FILE == buffer[0])
                param.i_execcmd = FALSE;  
            /* Execute command from file */
            cli_exec_cmd_from_file(pCli, &param, &session_info);
            
            /* check status */
            switch(param.o_status) {
            case CLI_ERR_FILEOPEN:
               error_pos = (int8)(strlen(session_info.prompt_string) + param.o_errpos + 1);
               m_CLI_DISPLAY_ERROR_MARKER(error_pos);  
               m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_FILE_OPEN_ERROR]);                        
               break;
               
            case CLI_ERR_FILEREAD:
               error_pos = (int8)(strlen(session_info.prompt_string) + param.o_errpos + 1);
               m_CLI_DISPLAY_ERROR_MARKER(error_pos);  
               m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_FILE_READ_ERROR]);                        
               break;
               
            case CLI_PASSWD_REQD:
               error_pos = (int8)(strlen(session_info.prompt_string) + param.o_errpos + 1);
               m_CLI_DISPLAY_ERROR_MARKER(error_pos);  
               m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_LOGIN_CMD]);                       
               break;

            default:
               break;
            }
            
            /*Push the command into the the command history list */
            if ( m_NCSCLI_EXEC_FILE == buffer[0])
                  cli_set_cmd_into_history(pCli, buffer);
            
            m_CLI_SET_PROMPT(session_info.prompt_string, display_flag);
            
            /*Reset the buffer after execution of command */
            memset(buffer, '\0', CLI_BUFFER_SIZE);
            memset(param.o_hotkey.tabstring, 0, sizeof(param.o_hotkey.tabstring));
            chCnt = 0;            
            break;
         }
#endif         
         /* Exiting from CLI */
         if(0 == cli_stricmp(buffer, CLI_SHUT)) 
         {
            cli_lib_shut_except_task_release(); 
            exit(0);
         }
/* quit command is valid only for pcode. For other cases it is invalidated .  */
#if (NCS_PC_CLI == 1)
         if(0 == cli_stricmp(buffer, CLI_QUIT)) 
         {            
            /*Reset the buffer */                    
            memset(buffer, '\0', CLI_BUFFER_SIZE);                        
            chCnt = 0;            
            
            /* Unblock PowerCode task */
            pcode_active = TRUE;
            m_NCS_SEM_GIVE(gl_pcode_sem_id);            
            break; 
         }
 /* #else
             return; */ /* this is commanted to invalidate quit command. quit command is being used only in pcode.so the flag NCS_PC_CLI is moved above if condition  and else part is commented */
#endif
         
         if(0 == cli_stricmp(buffer, CLI_HELP_DESC)) {
            
            /* User has  typed Help, equivalent to CLI_CONS_HELP */
            ch = CLI_CONS_HELP;
            memset(buffer, '\0', CLI_BUFFER_SIZE);
            chCnt = 0;       
            cmd_help = TRUE;
         }
         else
         {
            buffer[cmd_len] = '\0';  
            
            m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
            if(!context_node) {                    
               m_CLI_SET_CURRENT_CONTEXT(pCli, TRUE);
               m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
            }
            
            pCli->ctree_cb.cmdElement = context_node->pCmdl;
            if(!pCli->ctree_cb.cmdElement) {
               m_CLI_CONS_PUTLINE(CLI_DEFAULT_HELP_STR);
               m_CLI_SET_PROMPT(session_info.prompt_string, display_flag);                  
               buffer[strlen(buffer) - 1] = '\0';
               chCnt = 0;
               break;
            }
            
            param.i_cmdtype = CLI_EXECUTE;
            param.i_cmdbuf = buffer;              
            cli_execute_command(pCli, &param, &session_info);
            
            /* Check if user has  typed Exit */
            if((0 == cli_stricmp(buffer, CLI_EXIT)) && 
               (NCSCC_RC_CALL_PEND != pCli->cefStatus)) {
               cli_current_mode_exit(pCli, &session_info, &display_flag);
               memset(buffer, '\0', CLI_BUFFER_SIZE);
               chCnt = 0;
               break;
            }
            
            /*Set the current prompt */
            switch(param.o_status)
            {
            case CLI_NO_MODE:  
               m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_NO_MODE]);

               /*Push the command into the the command history list */
               cli_set_cmd_into_history(pCli, buffer);
               break;

            case CLI_NO_MATCH:
               error_pos = (int8)(strlen(session_info.prompt_string) + param.o_errpos + 1);
               m_CLI_DISPLAY_ERROR_MARKER(error_pos);  
               m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_INVALID_CMD]);
               
               /*Push the command into the the command history list */
               cli_set_cmd_into_history(pCli, buffer);
               break;
               
            case CLI_PASSWD_REQD:
               buffer[chCnt] = ' ';
               chCnt++;
               
               m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_PASSWORD]);                        
               passwd_flag = TRUE;
               pCli->loginFlag = TRUE;
               continue;
               
            case CLI_INVALID_PASSWD:
               m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_INVALID_PASSWORD]);
               /* Invalid password, so reset the Login Flag */
               pCli->loginFlag = FALSE;
               passwd_flag = FALSE;
               break;
               
            case CLI_PASSWORD_MATCHED:
               passwd_flag = FALSE;
               pCli->loginFlag = FALSE;
               if(FALSE == display_flag)
                  display_flag = TRUE;
               break;
               
            case CLI_PARTIAL_MATCH:
               m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_INCOMPLETE_CMD]);
               cli_set_cmd_into_history(pCli, buffer);
               break;
               
            case CLI_AMBIGUOUS_MATCH:
               m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_AMBIGUOUS_CMD]);
               
               /*Push the command into the the command history list */
               cli_set_cmd_into_history(pCli, buffer);
               break;
               
            case CLI_SUCCESSFULL_MATCH:
               if(CLI_HELP_NEXT_LEVEL == param.i_cmdtype) {
                  switch(param.o_status) {
                  case CLI_NO_MATCH:
                     error_pos = (int8)(strlen(
                        session_info.prompt_string) +
                        param.o_errpos + 1);
                     m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_UNRECOGNISED_CMD]);
                     break;
                     
                  case CLI_INVALID_PASSWD:
                     m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_INVALID_PASSWORD]);
                     passwd_flag = FALSE;
                     break;
                     
                  case CLI_AMBIGUOUS_MATCH:
                     m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_AMBIGUOUS_CMD]);
                     break;
                     
                  case CLI_PARTIAL_MATCH:
                  case CLI_SUCCESSFULL_MATCH:
                     /* Display the command */
                     for(index=0; index<param.o_hotkey.hlpstr.count; index++) {
                        sprintf(display_str, "\n %-25s %s", 
                           param.o_hotkey.hlpstr.helpargs[index].cmdstring,
                           param.o_hotkey.hlpstr.helpargs[index].helpstr);
                        
                        cli_display(pCli, display_str);                            
                     }
                     break;
                  default:
                     break;
                  }
               }
               /*Push the command into the the command history list */
               cli_set_cmd_into_history(pCli, buffer);
               break;

            default:
               break;
            }                
            if(strcmp(pCli->ctree_cb.trvMrkr->mode , "exec") == 0) 
                display_flag = TRUE; /*For suppressing for passwd for enable */
            m_CLI_SET_PROMPT(session_info.prompt_string, display_flag);
            
            /*Reset the buffer after execution of command */
            memset(buffer, '\0', CLI_BUFFER_SIZE);
            memset(param.o_hotkey.tabstring, 0, sizeof(param.o_hotkey.tabstring));
            chCnt = 0;
            break;
         }
         
      /* Help */
      case CLI_CONS_HELP:
         if(passwd_flag) goto default_val;                    
         if (!cmd_help) putchar(ch);
         
         cmd_help = FALSE;
         buffer[chCnt] = (int8)ch;
         chCnt++;  
         
         param.i_cmdtype = CLI_HELP;
         param.i_cmdbuf = buffer;
         m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
         
         if(!context_node) {                    
            m_CLI_SET_CURRENT_CONTEXT(pCli, TRUE);
            m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
         }
            
         pCli->ctree_cb.cmdElement = context_node->pCmdl;
         if(0 == pCli->ctree_cb.cmdElement) {
            m_CLI_CONS_PUTLINE(CLI_DEFAULT_HELP_STR);
            m_CLI_SET_PROMPT(session_info.prompt_string, display_flag);                  
            buffer[strlen(buffer) - 1] = '\0';
            chCnt = 0;
            break;
         }
         
         cli_execute_command(pCli, &param, &session_info);
         switch(param.o_status) {
         case CLI_NO_MODE:  
            m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_NO_MODE]);

            /*Push the command into the the command history list */
            cli_set_cmd_into_history(pCli, buffer);
            break;

         case CLI_NO_MATCH:
            error_pos = (int8)(strlen(
               session_info.prompt_string) +
               param.o_errpos + 1);
            m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_UNRECOGNISED_CMD]);
            break;
            
         case CLI_INVALID_PASSWD:
            m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_INVALID_PASSWORD]);
            passwd_flag = FALSE;
            break;
            
         case CLI_AMBIGUOUS_MATCH:
            m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_AMBIGUOUS_CMD]);
            break;
            
         case CLI_PARTIAL_MATCH:
         case CLI_SUCCESSFULL_MATCH:
            /* Display the command */
            for(index=0; index<param.o_hotkey.hlpstr.count; index++) {
               sprintf(display_str, "\n %-25s %s", 
                  param.o_hotkey.hlpstr.helpargs[index].cmdstring,
                  param.o_hotkey.hlpstr.helpargs[index].helpstr);
               
               cli_display(pCli, display_str);                            
            }
            break;

         default:
            break;
         }
         
         /*Push the command into the the command history list */
         buffer[strlen(buffer)-1] = ' ';
         cli_set_cmd_into_history(pCli, buffer);
         
         m_CLI_SET_PROMPT(session_info.prompt_string, display_flag);
         
         /*Set the current prompt */
         if(CLI_PARTIAL_MATCH == param.o_status || 
            CLI_SUCCESSFULL_MATCH == param.o_status) {
            
            buffer[strlen(buffer) - 1] = '\0';
            cli_display(pCli, buffer);
            chCnt--;
         }
         else {
            memset(buffer, '\0', CLI_BUFFER_SIZE);                
            chCnt = 0;
         }
         break;
         
      /* Tab */
      case CLI_CONS_TAB:
         if(TRUE == passwd_flag) break;
         if(chCnt > 0)
         {
            if(' ' == buffer[chCnt - 1]) {
               putchar(CLI_CONS_BEEP);
               break;
            }
         }
         
         buffer[chCnt] = '\0';    
         m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
         
         if(!context_node) {                    
            m_CLI_SET_CURRENT_CONTEXT(pCli, TRUE);
            m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
         }
         
         pCli->ctree_cb.cmdElement = context_node->pCmdl;
         if(0 == pCli->ctree_cb.cmdElement) {
            m_CLI_CONS_PUTLINE(CLI_DEFAULT_HELP_STR);
            m_CLI_SET_PROMPT(session_info.prompt_string, display_flag);                  
            buffer[strlen(buffer) - 1] = '\0';
            chCnt = 0;
            break;
         }
         
         param.i_cmdtype = CLI_TAB;
         param.i_cmdbuf = buffer;
         cli_execute_command(pCli, &param, &session_info);                  
         if(param.o_status == CLI_SUCCESSFULL_MATCH ||
            param.o_status == CLI_PARTIAL_MATCH) {
            strcat(param.o_hotkey.tabstring, " ");
            cli_display(pCli, param.o_hotkey.tabstring);                    
            chCnt += strlen(param.o_hotkey.tabstring);
            strcat(buffer, param.o_hotkey.tabstring);                    
         }
         else putchar(CLI_CONS_BEEP);               
         break;
            
      /* delete one char backwards */
      case CONTROL(CLI_CONS_DELETE_CHAR):                 
      case CLI_CONS_BACWARD_SLASH:
         if(0 != chCnt && FALSE == passwd_flag) chCnt = cli_delete_char(buffer, chCnt);
         else putchar(CLI_CONS_BEEP);
         break;
         
      case CLI_CONS_LEFT_ARROW:
         if(!is_arrow  || passwd_flag) goto default_val;
         
         if(0 != chCnt) chCnt = cli_move_char_backward(chCnt);
         else putchar(CLI_CONS_BEEP);
         
         if(is_arrow) is_arrow = FALSE;
         break;
         
      case CLI_CONS_RIGHT_ARROW:
         if(!is_arrow  || passwd_flag) goto default_val;
         
         chCnt = cli_move_char_forward(buffer, chCnt);
         if(is_arrow) is_arrow = FALSE;               
         break;                                    
         
      default:
default_val:     
         if(passwd_flag) putchar(CLI_CONS_PWD_MARK);  
         else putchar(ch);  
         
         buffer[chCnt] = (int8)ch;
         chCnt++;  
         break;
      } 
   } while(ch != CONTROL(CLI_CONS_END));    
   fclose(input); 
}


/*****************************************************************************
PROCEDURE NAME  :  cli_current_mode_exit
DESCRIPTION     :  Internal function used to exit from the current mode.
ARGUMENTS       :  pCli              - Pointer of control block
io_session_info   - Pointer of CLI_SESSION_INFO structure.
io_display_flag   - Pointer of NCS_BOOL         
RETURNS         :  none
NOTES           :        
*****************************************************************************/
void cli_current_mode_exit(CLI_CB *pCli, CLI_SESSION_INFO *io_session_info, 
                       NCS_BOOL *io_display_flag)
{
   NCS_BOOL         context_flag = FALSE;
   int8            *prev_mode = 0;
   CLI_CMD_NODE    *context_node = 0;
   
   /* node change so free existing subsys specific memory */
   if((pCli->ctree_cb.trvMrkr) && (pCli->ctree_cb.trvMrkr->pData)) {
      m_MMGR_FREE_CLI_DEFAULT_VAL(pCli->ctree_cb.trvMrkr->pData);
      pCli->ctree_cb.trvMrkr->pData = 0;
   }
   pCli->subsys_cb.i_cef_mode = 0;

   m_CLI_SET_CURRENT_CONTEXT(pCli, context_flag);
   m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
   if((context_node) && (0 != strcmp(context_node->name, CLI_ROOT_NODE))) {
      prev_mode = strrchr(io_session_info->prompt_string, '-');
      if(0 != prev_mode)
         io_session_info->prompt_string[strlen(io_session_info->prompt_string)
         - strlen(prev_mode)] = '\0';
      m_CLI_SET_PROMPT(io_session_info->prompt_string, io_display_flag);
   }
   else {                
      *io_display_flag = FALSE;
      m_CLI_SET_PROMPT(io_session_info->prompt_string, *io_display_flag);
   }  
}

/*****************************************************************************
   Function NAME   : cli_init
   DESCRIPTION     : This is an internal function that is required to initailize 
                     the CLI when it comes up for the first time
   ARGUMENTS       : pCli - Pointer of CLI_CB   
   RETURNS         : none
   NOTES           : 1. Sets the CLI mode
                     2. Register CLI itself with control block
                     3. Displays the login prompt      
*****************************************************************************/
void cli_init(CLI_CB *pCli)
{   
   /* set the mode of CLI to command mode */
   pCli->par_cb.mode = CLI_COMMAND_MODE;    
   
   /* Reset the root marker of the command tree */
   pCli->ctree_cb.rootMrkr = 0;  

   /* default cli idle time */
   pCli->cli_idle_time = m_CLI_DEFAULT_IDLE_TIME_IN_SEC * 100; /* Added to fix the bug 58609 */
   
   /* create CEF specific timer */
   m_NCS_TMR_CREATE(pCli->cefTmr.tid, CLI_CEF_TIMEOUT, cli_cef_exp_tmr, 0);
   
   /* Create CEF specific semaphore for CLI to pend */
   m_NCS_SEM_CREATE(&pCli->cefSem);

   /* default cli user group names(59359) */
   strcpy(pCli->cli_user_group.ncs_cli_viewer,NCSCLI_VIEWER);
   strcpy(pCli->cli_user_group.ncs_cli_admin,NCSCLI_ADMIN);
   strcpy(pCli->cli_user_group.ncs_cli_superuser,NCSCLI_SUPERUSER);

}

/*****************************************************************************
   Function NAME   : cli_cef_exp_tmr
   DESCRIPTION     : This is the function that is set as the function pointer 
                     that is to be invoked when timer expires. This function 
                     habdels all the neccessary action that is to be done at
                     the timer expires.
   ARGUMENTS       : i_id - Timer id
   RETURNS         : none
   NOTES           :        
*****************************************************************************/
void cli_cef_exp_tmr(void *i_id)
{
   CLI_CB                  *pCli = 0;
   uns32                   rc = NCSCC_RC_SUCCESS;
   NCSCLI_OP_DONE          done;
   NCSCLI_NOTIFY_INFO_TAG  info_tag;
   
   /* Arg passed is CB */
   if(!((uns32 *)i_id)) return;
   
   /* Validate the CLI handle */
   pCli = (CLI_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLI, *((uns32 *)i_id));
   if(!pCli) return;
   
   /* Display timer elapsed */
   cli_display(pCli, CLI_TMR_EXPIRED);
   
   /* reset the timer expired flag to TRUE */
   pCli->cefTmrFlag = TRUE;    
   
   /* Display error and release semaphore */   
   done.i_status = NCSCC_RC_FAILURE;
   rc = cli_done(pCli, &done);       
   if(NCSCC_RC_SUCCESS != rc) {
      ncshm_give_hdl(pCli->cli_hdl);
      m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_DISPLAY_FAILED);
      return;
   }
   
   info_tag.i_notify_hdl = pCli->cli_notify_hdl;
   info_tag.i_hdl = pCli->cli_hdl;
   info_tag.i_cb = pCli;
   info_tag.i_evt_type = NCSCLI_NOTIFY_EVENT_CEF_TMR_EXP;
   pCli->cli_notify(&info_tag);
   ncshm_give_hdl(pCli->cli_hdl);
}

/*****************************************************************************
   PROCEDURE NAME     :  cli_cb_main_lock
   DESCRIPTION        :  This function locks the CLI_CB
   ARGUMENTS          :  pCli  - Poiinter to cli structure
   RETURNS            :  NCSCC_RC_SUCCESS/FAILURE
   NOTES              :
*****************************************************************************/
uns32 cli_cb_main_lock(CLI_CB *pCli)
{
   if(!pCli) return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   
   /* LOGGING : Log "CLI MAIN IS LOCKED" */
   m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_MAIN_LOCKED);
   m_NCS_LOCK (&pCli->mainLock, NCS_LOCK_WRITE);  
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
   PROCEDURE NAME    :   cli_cb_main_unlock
   DESCRIPTION       :   This function unlocks the CLI_CB component  
   ARGUMENTS         :   pCli  - Poiinter to cli structure
   RETURNS           :   NCSCC_RC_SUCCESS/FAILURE
   NOTES:
*****************************************************************************/
uns32 cli_cb_main_unlock(CLI_CB *pCli)
{
   if(!pCli) return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   
   /* LOGGING : Log "CLI MAIN Is Released" */
   m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_MAIN_UNLOCKED);
   m_NCS_UNLOCK (&pCli->mainLock, NCS_LOCK_WRITE);
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
   PROCEDURE NAME    :   cli_ctree_lock
   DESCRIPTION       :   This function locks the CLI_CB 
   ARGUMENTS         :   pCli  - Poiinter to cli structure
   RETURNS           :   NCSCC_RC_SUCCESS/FAILURE
   NOTES:
*****************************************************************************/
uns32 cli_ctree_lock(CLI_CB *pCli)
{
   if(0 != pCli) {
      m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_CTREE_LOCKED);
      m_NCS_LOCK (&pCli->ctreeLock, NCS_LOCK_WRITE); 
      return NCSCC_RC_SUCCESS;
   }   
   return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
}

/*****************************************************************************
   PROCEDURE NAME    :   cli_ctree_unlock
   DESCRIPTION       :   This function unlocks the CLI_CB 
   ARGUMENTS         :   pCli  - Poiinter to cli structure
   RETURNS           :   NCSCC_RC_SUCCESS/FAILURE
   NOTES:
*****************************************************************************/
uns32 cli_ctree_unlock(CLI_CB *pCli)
{
   if(0 != pCli) {
      m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_CTREE_UNLOCKED);
      m_NCS_UNLOCK (&pCli->ctreeLock, NCS_LOCK_WRITE);
      return NCSCC_RC_SUCCESS;
   }
   return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
}

/*****************************************************************************
   PROCEDURE NAME  : cli_clean_cmds
   DESCRIPTION     : This function cleans up the indivisual command under a 
                     specific mode
   ARGUMENTS       : pCli - Control block of CLI
                     list - Command list
   RETURNS         : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES           :
*****************************************************************************/
uns32 cli_clean_cmds(CLI_CB *pCli, NCSCLI_DEREG_CMD_LIST *list)
{
   CLI_CMD_NODE      *mptr = 0;
   NCSCLI_CMD_LIST   cmdlist;
   uns32             cmdcnt = 0;
   CLI_CMD_ELEMENT   *stack[NCSCLI_ARG_COUNT];

   memset(&cmdlist, 0, sizeof(cmdlist));  
   memset(stack, 0, sizeof(stack));  
   cmdlist.i_node = list->i_node;
   
   /* Reset all marker */   
   cli_reset_all_marker(pCli);
   if(0 != strlen(list->i_node)) {
      /* Locate the Node subtree thats needs to be cleaned-up */
      if(NCSCC_RC_SUCCESS != cli_parse_node(pCli, &cmdlist, FALSE))
      {
         /* Invalid path of the node specified */
         cli_display(pCli, "\n...... Dergistration failed ..... \n");
         return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
      }
   }
   else return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);

   /* OK we have got the Node */
   mptr = pCli->ctree_cb.ctxtMrkr;

   /* Mark the deregistration flag before parsing the commands */
   pCli->ctree_cb.dereg = TRUE;
   pCli->ctree_cb.dregStack = stack;

   /* Parse each command string */
   for(cmdcnt=0; cmdcnt<list->i_cmd_count; cmdcnt++) {
      cli_set_cmd_to_parse(pCli, list->i_command_list[cmdcnt].i_cmdstr, CLI_CMD_BUFFER);

      /* Clean nodes from the stack */
      while(pCli->ctree_cb.dregStackMrkr >= 0) {
         CLI_CMD_ELEMENT *node = 0, *pptr = 0;
         node = pCli->ctree_cb.dregStack[pCli->ctree_cb.dregStackMrkr];
         pCli->ctree_cb.dregStackMrkr--;

         /* Clean up the node if its the LEAF node */
         if(!node->node_ptr.pChild && 
            !node->node_ptr.pDataPtr && !node->node_ptr.pSibling) {
            
            pptr = node->node_ptr.pParent;
            if(pptr == node) mptr->pCmdl = 0;            

            if((pptr->node_ptr.pChild) && (pptr->node_ptr.pChild == node))
               pptr->node_ptr.pChild = 0;
            
            if((pptr->node_ptr.pDataPtr) && (pptr->node_ptr.pDataPtr == node))
               pptr->node_ptr.pDataPtr = 0;
            
            if((pptr->node_ptr.pSibling) && (pptr->node_ptr.pSibling == node))
               pptr->node_ptr.pSibling = 0; 
            cli_free_cmd_element(&node);
            continue;
         }

         if(!node->node_ptr.pChild && 
            !node->node_ptr.pDataPtr && node->node_ptr.pSibling) {
            
            pptr = node->node_ptr.pParent;            
            if(pptr == node) {
               mptr->pCmdl = node->node_ptr.pSibling;
               node->node_ptr.pSibling->node_ptr.pParent = node->node_ptr.pSibling;
            }
            else {
               if((pptr->node_ptr.pChild) && (pptr->node_ptr.pChild == node))
                  pptr->node_ptr.pChild = node->node_ptr.pSibling;               
               
               if((pptr->node_ptr.pDataPtr) && (pptr->node_ptr.pDataPtr == node))
                  pptr->node_ptr.pDataPtr = node->node_ptr.pSibling;
               
               if((pptr->node_ptr.pSibling) && (pptr->node_ptr.pSibling == node))
                  pptr->node_ptr.pSibling = node->node_ptr.pSibling;
               node->node_ptr.pSibling->node_ptr.pParent = pptr;
            }                           
            cli_free_cmd_element(&node);
         }         
      }
   } 

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
   PROCEDURE NAME  : cli_clean_mode
   DESCRIPTION     : This function cleans up the all commands under a specific 
                     mode and wipes of the mode as well.
   ARGUMENTS       : pCli - Control block of CLI
                     node - node path that is to be cleaned
   RETURNS         : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES           :
*****************************************************************************/
uns32 cli_clean_mode(CLI_CB *pCli, int8 *node)
{
   CLI_CMD_NODE   *nptr = 0, *pptr = 0;
   NCSCLI_CMD_LIST cmdlist;
   
   memset(&cmdlist, 0, sizeof(cmdlist));  
   cmdlist.i_node = node;
   
   /* Reset all marker */   
   cli_reset_all_marker(pCli);
   if(0 != strlen(node)) {
      /* Locate the Node subtree thats needs to be cleaned-up */
      if(NCSCC_RC_SUCCESS != cli_parse_node(pCli, &cmdlist, FALSE)) {
         /* Invalid path of the node specified */
         cli_display(pCli, "\n...... Dergistration failed ..... \n");
         return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
      }
   }
   else return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);   
   
   /* OK we have got the Node, Detach the node from the tree */   
   nptr = pCli->ctree_cb.ctxtMrkr;   
   pptr = nptr->pParent;

   /* Now check which pointer of parent node was pointng to us */
   if(pptr) {
      if(pptr->pChild == nptr) /* Child */
         pptr->pChild = nptr->pSibling; /* Next Child */
      
      if(pptr->pSibling == nptr) /* Sibling */
         pptr->pSibling = nptr->pSibling; /* Next Child */
      
      if(nptr->pSibling) {
         nptr->pSibling->pParent = pptr;
         nptr->pSibling = 0; /* Remove the pointer referemce to sibling */
         nptr->pParent = 0;  /* Remove the pointer referemce to parent */ 
      }
   }     

   /* Clean the command mode and all the subsequent modes and command 
    * under that mode 
   */   
   cli_clean_cmd_node(nptr);       
   return NCSCC_RC_SUCCESS;
}

#if(NCSCLI_FILE == 1)
/*****************************************************************************
   Function NAME   : cli_exec_cmd_from_file
   DESCRIPTION     : Thie routines reads onle line of command from the file and 
                     executes it.
   ARGUMENTS       : pCli      - CLI Control block pointer
                     io_param  - Pointer to CLI_EXECUTE_PARAM structure
                     o_session - Pointer to CLI_SESSION_INFO structure   
   RETURNS         :  none
   NOTES           : 1 . This execution of commands stop if any Error is 
                         encountered 
                     2 . It echos the command while execting them
*****************************************************************************/
void 
cli_exec_cmd_from_file(CLI_CB *pCli, CLI_EXECUTE_PARAM *io_param, 
                       CLI_SESSION_INFO *o_session)
{
   int8 cmdStr[CLI_BUFFER_SIZE], buf[CLI_FILE_READ], *fname = 0;
   uns32 errpos = 0;
   int32 error_pos;
   CLI_CMD_NODE  *context_node = 0;
   NCS_BOOL display_flag = FALSE;
   
   memset(buf, 0, sizeof(buf));
   memset(cmdStr, 0, sizeof(cmdStr));
   
   fname = io_param->i_cmdbuf;
   
   /* Parse the bufer and get file name */   
   while (*fname == m_NCSCLI_EXEC_FILE ||*fname == m_NCSCLI_VERIFY_FILE ||  *fname == CLI_CONS_BLANK_SPACE)
   {
      fname++;
      errpos++;
   }
   
   /* Open for read will fail if file does not exist) */
   if(NCSCC_RC_SUCCESS != sysf_open_file(fname)) {
      io_param->o_errpos = errpos;
      io_param->o_status = CLI_ERR_FILEOPEN;
      
      m_LOG_NCSCLI_COMMENTS(fname, NCSCLI_SCRIPT_FILE_OPEN, "Unable to open file");
      return;
   }
   
   /* Cycle until end of file reached: */
   for(;;) {
      memset(cmdStr, 0, sizeof(cmdStr));
      if(!sysf_readln_from_file(cmdStr)) continue;
      m_GET_TIME_STAMP(pCli->cli_last_active_time);/*Added to fix bug 58609 */
      
      if(cmdStr == NULL) break;      
      if(strlen(cmdStr) == 0) break;
      
      m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
      if(!context_node) {                    
         m_CLI_SET_CURRENT_CONTEXT(pCli, TRUE);
         m_CLI_GET_CURRENT_CONTEXT(pCli, context_node);
      }
      
      /* Call cli_execute_command func, to execute cmd */
      pCli->ctree_cb.cmdElement = context_node->pCmdl;  
      if(!pCli->ctree_cb.cmdElement) {
         m_CLI_CONS_PUTLINE(CLI_DEFAULT_HELP_STR);
         m_CLI_SET_PROMPT(o_session->prompt_string, TRUE)
         continue;        
      }
      
      io_param->i_cmdtype = CLI_EXECUTE;
      io_param->i_cmdbuf = cmdStr;
      
      m_CLI_SET_PROMPT(o_session->prompt_string, TRUE)         
      cli_display(pCli, cmdStr);        
      
      /* Execute command */
      cli_execute_command(pCli, io_param, o_session);      

      /* Exiting from CLI */
      if(0 == cli_stricmp(cmdStr, CLI_SHUT))
      {
          cli_lib_shut_except_task_release(); 
          exit(0);
      }
      if((0 == cli_stricmp(cmdStr, CLI_EXIT)) && 
         (NCSCC_RC_CALL_PEND != pCli->cefStatus)) {
         cli_current_mode_exit(pCli, o_session, &display_flag);
         continue;
      }
      /* Check return status */
      switch(io_param->o_status) {
      case CLI_NO_MATCH:
         error_pos = (int8)(strlen(o_session->prompt_string)
            + io_param->o_errpos + 1);
         m_CLI_DISPLAY_ERROR_MARKER(error_pos);  
         m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_INVALID_CMD]);   
         m_CLI_CONS_PUTLINE("..... Error encountered !!! can't execute the file .....");
         goto endexec;
         break;
         
      case CLI_PARTIAL_MATCH:
         m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_INCOMPLETE_CMD]);                
         break;
         
      case CLI_AMBIGUOUS_MATCH:
         m_CLI_CONS_PUTLINE(cli_con_err_str[CLI_CONS_AMBIGUOUS_CMD]);         
         break;
         
      case CLI_SUCCESSFULL_MATCH:
         break;
         
      case CLI_PASSWD_REQD:
         break;
         
      default:
         break;
      }
      
      if(CLI_PASSWD_REQD == io_param->o_status)
         break;
   }
   
endexec:
   /* Close file */
   if(NCSCC_RC_SUCCESS != sysf_close_file()) {
      m_LOG_NCSCLI_COMMENTS(fname, NCSCLI_SCRIPT_FILE_CLOSE, "Unable to close the file");        
      return;
   }    
}
#endif /* (NCSCLI_FILE  == 1) */

#else
extern int dummy;
#endif /* (if NCS_CLI == 1) */

