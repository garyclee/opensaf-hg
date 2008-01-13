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

  MODULE NAME:  CLI_DEF.H
  
$Header: /ncs/software/Work/cli12bf/targsvcs/products/cli/cli_def.h 2     9/17/01 1:31p Agranos $
  ..............................................................................
    
  DESCRIPTION:
      
  Header file for MACRO defined in the target services area (CLI).
        
******************************************************************************
*/

#ifndef CLI_DEF_H
#define CLI_DEF_H


struct ncscli_notify_info_tag;
/***************************************************************************
                    Mapper structure
****************************************************************************/
typedef struct clikey_to_clicb_map
{
    NCS_KEY      key;
    NCSCONTEXT   cb; 
} CLIKEY_TO_CLICB_MAP;

extern CLIKEY_TO_CLICB_MAP   cli_mapper[];

#define CLI_TMR_EXPIRED             "\tUnable to complete CEF with the time period"

/* Router Prompt that is diplayed by CLI */
#define CLI_ROUTER_NAME             "NCS"

/* Constant used for executing script file.
   User can type m_NCSCLI_EXEC_FILE 'script file' to execute commands
   from a file */

#define m_NCSCLI_EXEC_FILE           '@'

/* Constant used for verifying script file.
   User can type m_NCSCLI_VERIFY_FILE 'script file' to verify commands
   from a file */

#define m_NCSCLI_VERIFY_FILE           '$'  /* Fix for the bug 59119 */

/* The following value is treated as comment in CLI Script files */
#define m_NCSCLI_COMMENT             '!'

/*****************************************************************************
                        ANSI FILE FUNCTION

The following functions are used as part of CLI for reading commands
frpm script files.
****************************************************************************/
#define m_NCSCLI_FILE_OPEN       fopen
#define m_NCSCLI_FILE_READ       getc
#define m_NCSCLI_FILE_CLOSE      fclose
#define m_NCSCLI_FILE_EOF        feof
#define m_NCSCLI_FILE_ERROR      ferror


/*****************************************************************************

  MACRO NAME:    m_NCSCLI_GETCHAR
  
  DESCRIPTION:   Windows NT uses _getch() to read a single 
                 character from console. getchar() in WindowsNT
                 buffers the input until 'Enter' is pressed.
       
  ARGUMENTS:
    none
  
  RETURNS: 
    none         Character read
        
  NOTES:        
          
*****************************************************************************/
#define m_NCSCLI_GETCHAR(vr_id)         m_NCS_OS_UNBUF_GETCHAR()


/*****************************************************************************

  MACRO NAME:    m_CLI_FLUSH_STR
  
  DESCRIPTION:   This macro writes the o/p string to console. It writes
                 to the console represented by console Id.
    
  ARGUMENTS:
    prompt_str   The o/p string
    console_id   Console id of the console/
      
  RETURNS: 
    none
        
  NOTES:         Presently there is no mapping between console and CLI.
                 CLI prints the output to default console provided by OS.
          
*****************************************************************************/
#define m_CLI_FLUSH_STR(prompt_str, console_id)\
{\
    uns32 index,len = m_NCS_OS_STRLEN(prompt_str);\
    for(index=0; index<len; index++)\
    m_NCS_CONS_PUTCHAR(prompt_str[index]);\
}


/*****************************************************************************

  MACRO NAME:    m_GET_CONSOLE
  
  DESCRIPTION:   This macro is used to get the console Id of the console
                 that is associated with Virtual Router.
    
  ARGUMENTS:
    key          CLI key
    console_id   Console id
      
  RETURNS: 
    none
        
  NOTES:        Presently there is no mapping between console and CLI.
                CLI prints the output to default console provided by OS.
*****************************************************************************/
#define m_GET_CONSOLE(key, console_id)     console_id = 0


/*****************************************************************************

  MACRO NAME:    m_CLI_VALIDATE_CLI_KEY
  
  DESCRIPTION:   Gets the Control block of CLI instance associated
                 with the key.
    
  ARGUMENTS:
    key          CLI key
      
  RETURNS: 
    CB *         Control Block pointer
        
  NOTES:         Definition for key validation, target dependant 
         
*****************************************************************************/
#define m_CLI_VALIDATE_CLI_KEY(cli_key) ((struct cli_cb*) cli_mapper[cli_key.val.num].cb)


/*****************************************************************************

  MACRO NAME:    m_CLI_ADD_KEY_TO_CB_MAP
  
  DESCRIPTION:   Registers CLIs Control block to CLI key.   
    
  ARGUMENTS:
    key          CLI key
    CB *         Control Block pointer
             
  RETURNS: 
    none
        
  NOTES:        Strategy to allow the the macro m_CLI_VALIDATE_CLI_KEY(key)
                to get the appropriate CLI Control Block. 
            
*****************************************************************************/
#define m_CLI_ADD_KEY_TO_CB_MAP(int_key, handle)\
            cli_mapper[int_key].cb = (void*) handle;\
            cli_mapper[int_key].key.val.num = int_key;


#if (NCS_CLI == 1) && (NCSCLI_FILE == 1)
EXTERN_C NCS_BOOL sysf_readln_from_file(int8 *);

EXTERN_C uns32 sysf_open_file(char *);

EXTERN_C uns32 sysf_close_file(void);
#endif

EXTERN_C CLIDLL_API void sysf_cli_notify(struct ncscli_notify_info_tag *i_notify_info);
EXTERN_C CLIDLL_API uns32 sysf_getchar(NCS_VRID id);

/************************************************************************
 * 
 * m_CLI_DBG_SINK
 *
 * If CLI fails an if-conditional or other test that we would not expect
 * under normal conditions, it will call this macro. 
 * 
 * If NCSCLI_DEBUG == 1, fall into the sink function. A customer can put
 * a breakpoint there or leave the CONSOLE print statement, etc.
 *
 * If NCSCLI_DEBUG == 0, just echo the value passed. It is typically a 
 * return code or a NULL.
 *
 * NCSCLI_DEBUG can be enabled in cli_opt,h
 */


#if (NCSCLI_DEBUG == 1)

EXTERN_C uns32 cli_dbg_sink (uns32,char*,uns32);

#define m_CLI_DBG_SINK(r)  cli_dbg_sink(__LINE__,__FILE__,(uns32)r)
#define m_CLI_DBG_SINK_VOID(r)  cli_dbg_sink(__LINE__,__FILE__,(uns32)r)

#else

#define m_CLI_DBG_SINK(r)  r
#define m_CLI_DBG_SINK_VOID(r)

#endif
#endif
