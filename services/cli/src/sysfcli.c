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

 MODULE NAME:  CLI_DEF.C

..............................................................................

  DESCRIPTION:

  Source file for Target Definitions functions
******************************************************************************
*/

#include "cli.h"
#include "clilog.h"

#if (NCS_CLI == 1)
CLIKEY_TO_CLICB_MAP   cli_mapper[5];
#define CLI_START_OF_LINE        0

#if (NCSCLI_FILE == 1)
FILE  *script;
#endif

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            ANSI Function Prototypes

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
EXTERN_C uns32 sysf_get_char_map(uns32 ch);

/*****************************************************************************
  
  PROCEDURE NAME:    SYSF_CLI_NOTIFY

  DESCRIPTION:       Notify function used for notification of errors.

  ARGUMENTS:
   NCSCLI_NOTIFY_INFO_TAG * 
                    Pointer to NCSCLI_NOTIFY_INFO_TAG structure
   
  RETURNS: 
   uns32             NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES:         

*****************************************************************************/
void sysf_cli_notify(NCSCLI_NOTIFY_INFO_TAG *i_notify_info)
{
    switch(i_notify_info->i_evt_type)
    {
    case NCSCLI_NOTIFY_EVENT_INVALID_CLI_CB:
        printf("Invalid Control Bolck: NCSCLI_NOTIFY_EVENT_INVALID_CLI_CB \n");
        break;

    case NCSCLI_NOTIFY_EVENT_MALLOC_CLI_CB_FAILED:
        printf("Control Bolck Malloc Failed: NCSCLI_NOTIFY_EVENT_MALLOC_CLI_CB_FAILED \n");
        break;

    case NCSCLI_NOTIFY_EVENT_CEF_TMR_EXP:
        m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_CEF_TMR_ELAPSED);
        break;

    case NCSCLI_NOTIFY_EVENT_CLI_INVALID_NODE:
        m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_INVALID_NODE);
        break;

    default:
        break;
    }
}

/*****************************************************************************
  
  PROCEDURE NAME:    SYSF_GETCHAR

  DESCRIPTION:       This function reads the input from the console. This 
                     returns characters without echo and without buffering.

  ARGUMENTS:
                     NCS_VRID id
   
  RETURNS: 
   char              Read character

  NOTES:         

*****************************************************************************/
uns32 sysf_getchar(NCS_VRID id)
{
   return sysf_get_char_map(m_NCSCLI_GETCHAR(NCS_VRID id));
}

/*****************************************************************************
  
  PROCEDURE NAME:    sysf_get_char_map

  DESCRIPTION:       This function maps the keys to Netplane defined values.
                     This is an reference code for windows os for Arrow keys.
                     

  ARGUMENTS:         uns32 : ASCII val of the key that is to be mapped into 
                             Netplane key value.
   
  RETURNS: 
   uns32             Netplane mapping.

  NOTES:         

*****************************************************************************/
uns32 sysf_get_char_map(uns32 ch)
{
   switch(ch)
   {
   case 224:
   case 27:
   case 127:      /* To Fix the bug 58884 */
   case 8:        /* To Fix the bug 58884 */
   case 0:        /* To Fix the bug 58884 */
      return 0;
#ifdef WIN32
   case CLI_CONS_UP_ARROW:      
     return CLI_CONS_UP_ARROW;
   case CLI_CONS_DOWN_ARROW:
      return CLI_CONS_DOWN_ARROW;
   case CLI_CONS_LEFT_ARROW:
      return CLI_CONS_LEFT_ARROW;
   case CLI_CONS_RIGHT_ARROW:
      return CLI_CONS_RIGHT_ARROW;
#endif

   default:
      return ch;
   }
}

#if (NCSCLI_FILE == 1)
/*****************************************************************************
  
  PROCEDURE NAME:    SYSF_READLN_FROM_FILE

  DESCRIPTION:       Function reads one line from the file ie till it encounters
                     newline charcter.

  ARGUMENTS:
                     buffer 
   
  RETURNS: 
   uns32             TRUE/FALSE

  NOTES:         

*****************************************************************************/
NCS_BOOL sysf_readln_from_file(int8 * buffer)
{
   uns32 ch=0, i=0;    
   NCS_BOOL cmt_flag = FALSE, start_of_line = FALSE;      
   
   /* read one line after another from file */        
   while(i < CLI_BUFFER_SIZE)
   {
      ch = m_NCSCLI_FILE_READ(script);
      if((ch == EOF) || (ch == CLI_CONS_EOL)) break;            
      
      /* Ignore white character at the start of the line */
      if(CLI_CONS_BLANK_SPACE == ch && !start_of_line) continue;
      else if(CLI_CONS_BLANK_SPACE != ch && !start_of_line) start_of_line = TRUE;

      /* If comment then ignore line */
      if((m_NCSCLI_COMMENT == ch) && (CLI_START_OF_LINE == i)) cmt_flag = TRUE;        
           
      buffer[i] = (char)ch;                     
      i++;
   }
   
   /* Terminate the buffer's content */
   buffer[i] = '\0';

   /* If comment or EOL with out any command then continue */
   if(cmt_flag || ((0 == i) && (ch == CLI_CONS_EOL))) return FALSE;   
   return TRUE;
}

/*****************************************************************************
  
  PROCEDURE NAME:    SYSF_OPEN_FILE

  DESCRIPTION:       Function open the specified file in read only mode.

  ARGUMENTS:
                     file  
   
  RETURNS: 
   uns32             NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES:         

*****************************************************************************/
uns32 
sysf_open_file(char *fname)
{
   /* Open for read will fail if file does not exist) */
   if ((script  = m_NCSCLI_FILE_OPEN(fname, "r")) == NULL)
   {
      return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   }    
   
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  
  PROCEDURE NAME:    SYSF_CLOSE_FILE

  DESCRIPTION:       Function closes the specified file

  ARGUMENTS:
                     file  
   
  RETURNS: 
   uns32             NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES:         

*****************************************************************************/
uns32
sysf_close_file(void)
{
   /* Close file */
   if (m_NCSCLI_FILE_CLOSE(script))
   {
      return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   }
   return NCSCC_RC_SUCCESS;
} 
#endif  

#if (NCSCLI_DEBUG == 1)
/* 
 * D E B U G   assist stuff
 *
 * CLI is entirely instrumented to fall into this debug sink function
 * if it hits any if-conditional that fails, but should not fail. This
 * is the default implementation of that SINK macro.
 *
 * The customer is invited to redefine the macro (in mds_dbg.h SMM ) or 
 * re-populate the function here.
 */

uns32 cli_dbg_sink(uns32 l, char* f, uns32 code)
  {

  printf ("IN CLI_DBG_SINK: line %d, file %s\n",l,f);

  return code;
  }

#endif
#endif 
