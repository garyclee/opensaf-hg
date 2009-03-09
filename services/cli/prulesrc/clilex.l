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

 MODULE NAME:  CLILEX.L
 
..............................................................................

  DESCRIPTION:

  Lexer file for tokenising the input

******************************************************************************
*/
%{

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "cli.h"

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
						Variable declaration.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
static CLI_CB *lexCli = 0;
EXTERN_C int32 yylineno;

#define m_CHECK_CONTINOUS_EXP_ERR() \
{\
    if(0 == lexCli->par_cb.grpCntr &&\
       TRUE == lexCli->par_cb.cont_expFlag)\
    {\
        par_log_error(CLI_TOK_CONTINOUS_EXP);\
        lexCli->par_cb.errFlag = TRUE;\
    }\
}

%}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
						Token declaration.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
number      [0-9]+  
keyword     [*0-9a-z_-]+
param       [A-Z_-]+
ipaddr      [0-9]{1,3}([\.][0-9]{1,3}[\.][0-9]{1,3}[\.][0-9]{1,3})
lbrace      [\[]
rbrace      [\]]
lcurlybrace [\{]
rcurlybrace [\}]
less_than   [\<]
grtr_than   [\>]
roundsbrace [\(]
roundebrace [\)]
percentage  [%]
range       {less_than}[ a-zA-Z_-]*[ 0-9]*[\.]{2}[ a-z_-]*[ A-Z_-]*[ 0-9]*{grtr_than}
default_val {percentage}[ a-zA-Z_-]*[ 0-9]*{ipaddr}*{percentage}
help_delimiter [ \t\b\n.:;?,/@#$%^&|=`*~'"0-9a-zA-Z_-]
help_sub_tok1 {lbrace}{help_delimiter}*{rbrace}
help_sub_tok2 {lcurlybrace}{help_delimiter}*{rcurlybrace}
help_sub_tok3 {less_than}{help_delimiter}*{grtr_than}
help_sub_tok4 {roundsbrace}{help_delimiter}*{roundebrace}
help_str    [\!]({help_delimiter}+|{help_sub_tok1}+|{help_sub_tok2}+|{help_sub_tok3}+|{help_sub_tok4}+)*[\!]
mode_change [\@][/0-9a-zA-Z_-]*[\@]
continous_exp {roundsbrace}([0-9a-zA-Z]+...[*0-9a-zA-Z]+){roundebrace}
or          [\|]

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				               Token Action.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
%%
NCSCLI_WILDCARD  {
                    /* token - Parameter (wildcard) */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    if(CLI_COMMAND_MODE == lexCli->par_cb.mode)
                        return (CLI_TOK_WILDCARD);
                }
                
NCSCLI_MACADDR   {
                    /* token - Parameter (MacAddress) */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    if(CLI_COMMAND_MODE == lexCli->par_cb.mode)
                        return (CLI_TOK_MACADDR);
                }
                
NCSCLI_COMMUNITY {
                    /* token - Parameter (Ipv4Address/Ipv4Mask) */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    if(CLI_COMMAND_MODE == lexCli->par_cb.mode)
                        return (CLI_TOK_COMMUNITY);
                }
                
NCSCLI_CIDRv4    |
NCSCLI_CIDRV4    {
                    /* token - Parameter (Ipv4Address/Ipv4Mask) */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    if(CLI_COMMAND_MODE == lexCli->par_cb.mode)
                        return (CLI_TOK_CIDRv4);
                }
NCSCLI_IPv6 |            
NCSCLI_IPV6      {
                    /* token - Parameter(IP-Addressv6) */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);                            
                    if(CLI_COMMAND_MODE == lexCli->par_cb.mode)
                        return (CLI_TOK_IPADDRv6);
                }   
NCSCLI_IPv4 |
NCSCLI_IPV4 |
{ipaddr}        {
                    /* token - Parameter(IP-Addressv4) */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    if(CLI_COMMAND_MODE == lexCli->par_cb.mode)
                        return (CLI_TOK_IPADDRv4);
                }

NCSCLI_MASKv4 |
NCSCLI_MASKV4   {
                    /* token - Parameter(IP-Address Maskv4) */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);                 
                    return (CLI_TOK_MASKv4);
                }
                 
NCSCLI_CIDRv6 |
NCSCLI_CIDRV6   {
                    /* token - Parameter(IP-Address Maskv6) */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return (CLI_TOK_CIDRv6);
                }                      
 
NCSCLI_NUMBER |
{number}        {
                    /* token - Parameter(Number) */                    
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    if(CLI_COMMAND_MODE == lexCli->par_cb.mode)
                        return (CLI_TOK_NUMBER);                        
                }

{keyword}       { 
                    /* token - Keyword  */
                    /* A keyword is any sequence of characters in 
                       lower case */
                    m_CHECK_CONTINOUS_EXP_ERR();
                       
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);   
                    if(CLI_COMMAND_MODE == lexCli->par_cb.mode)
                        return (CLI_TOK_KEYWORD);                   
                }

NCSCLI_PASSWORD  {
                    /* token - Password */
                    /* A alfha numeric characters */
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    if(CLI_COMMAND_MODE == lexCli->par_cb.mode)
                        return (CLI_TOK_PASSWORD);              
                }               
                
{param}         {   
                    /* token - Parameter(String) */
                    /* A string parameter is any sequrnce of characters
                       in upper case */
                    m_CHECK_CONTINOUS_EXP_ERR();
                       
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);   
                    if(CLI_COMMAND_MODE == lexCli->par_cb.mode)
                        return (CLI_TOK_PARAM);                 
                }               
                
{range}         {
                    /* token - Range */ 
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return (CLI_TOK_RANGE);
                }
                    
{default_val}   {
                    /* Default value associated with the token */
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return(CLI_TOK_DEFAULT_VAL);
                }
                
{help_str}      {               
                    /* Help string associated with the token */
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);                    
                    return (CLI_TOK_HELP_STR);
                }
                
{mode_change}   {
                    /* Node change string associated with the token */              
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return (CLI_TOK_MODE_CHANGE);
                }

{continous_exp} {
                    /* Continous Expression */                      
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    if(NCSCLI_GROUP != 
                        lexCli->par_cb.tokList[lexCli->par_cb.tokCnt-1]->tokType)                    
                    {
                        par_log_error(CLI_TOK_CONTINOUS_EXP);       
                        lexCli->par_cb.errFlag = TRUE;
                    }
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    lexCli->par_cb.cont_expFlag = TRUE;
                    return (CLI_TOK_CONTINOUS_EXP);             
                }
                     
[\t ]+          {
                    /* token - white space */                       
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    ; /* ignore white space */
                }

[\r]            {
                    /* token - Carrage return */                    
                    ; /* ignore carrage return */
                }              
  
[\n]            {                                          
                    /* End of Command */               
                    if(0 != lexCli->par_cb.grpCntr)
                    {
                        lexCli->par_cb.errFlag = TRUE;                        
                        par_log_error(CLI_TOK_LCURLYBRACE);     
                    }
                    
                    if(0 != lexCli->par_cb.optCntr)
                    {
                        lexCli->par_cb.errFlag = TRUE;
                        par_log_error(CLI_TOK_LBRACE);      
                    }
                    
                    if(FALSE == lexCli->par_cb.errFlag)
                    {
                        uns32   Index = 0;
                        for(Index=0; Index<lexCli->par_cb.tokCnt; Index++)
						{
                            cli_scan_token(lexCli, lexCli->par_cb.tokList[Index],
                                lexCli->par_cb.tokList[Index]->tokRel);
						}
                        
                        cli_set_token_attrib(lexCli, lexCli->par_cb.tokList[lexCli->par_cb.tokCnt - 1],
                                             CLI_DO_FUNC, "do function for the command");                                                                 
                        lexCli->par_cb.tokCnt = 0;                        
                    }
                    else
                    {
                        uns32   Index = 0;
                        for(Index=0; Index<lexCli->par_cb.tokCnt; Index++)
                            cli_free_cmd_element(&lexCli->par_cb.tokList[Index]);
                            
                        lexCli->par_cb.tokCnt = 0; 
                        lexCli->par_cb.errFlag = FALSE;                        
                    }
                    
                    /* reset all counter that are being used by yacc
                           at the end of command */                                                                             
                    lexCli->par_cb.chCnt = 0;
                    lexCli->par_cb.errPos = 0;  
                    lexCli->par_cb.is_tok_cont = FALSE;
                    lexCli->par_cb.cont_expFlag = FALSE;     
                    lexCli->par_cb.orFlag = FALSE;  
                    lexCli->par_cb.resetLvlFlag = FALSE;
                    lexCli->par_cb.tokLvlCntr = 0;
                    lexCli->par_cb.optLvlCntr = 0;
                    lexCli->par_cb.grpLvlCntr = 0;
                    lexCli->par_cb.optCntr = 0;
                    lexCli->par_cb.optTokCnt = 0;
                    lexCli->par_cb.grpTokCnt = 0;
                    lexCli->par_cb.grpCntr = 0;
                    lexCli->par_cb.grpStackPtr = -1;                
                    lexCli->par_cb.optStackPtr = -1;
                    lexCli->par_cb.brcStackPtr = -1;
                    lexCli->par_cb.brcType = CLI_NONE_BRACE;    

                    m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_NEWCMD);

                }               
                            
{lcurlybrace}   {
                    /* token - Group start brace */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return (CLI_TOK_LCURLYBRACE);
                }                           
                    
{rcurlybrace}   {
                    /* token - Group end brace */                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return (CLI_TOK_RCURLYBRACE);
                }   

{lbrace}        {
                    /* token - Optional start brace */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return (CLI_TOK_LBRACE);
                }                           
                    
{rbrace}        {
                    /* token - Optional end brace */                                                                
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return (CLI_TOK_RBRACE);                    
                }                                           
                
{or}            {
                    /* token - or */ 
                    if(lexCli->par_cb.grpCntr == 0 && lexCli->par_cb.cont_expFlag) 
                        lexCli->par_cb.cont_expFlag = FALSE; 
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return (CLI_TOK_OR);
                }
                
{less_than}     {
                    /* token - Less than */
                    m_CHECK_CONTINOUS_EXP_ERR();
                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return (CLI_TOK_LESS_THAN);
                }
                
{grtr_than}     {
                    /* token - Greater than */                    
                    lexCli->par_cb.chCnt += m_NCS_OS_STRLEN(yytext);
                    return (CLI_TOK_GRTR_THAN);
                }
                
\<[^\>\n]*$     {
                    /* token - Error for unterminated range delimiter*/
                    par_log_error(CLI_TOK_RANGE);       
                    lexCli->par_cb.errFlag = TRUE;
                }
                    
\%[^\%\n]*$     {
                    /* token - Error for unterminated default value delimiter */
                    par_log_error(CLI_TOK_DEFAULT_VAL);     
                    lexCli->par_cb.errFlag = TRUE;
                }
                                            
\@[^\@\n]*$     {
                    /* token - Error for unterminated mode delimiter */
                    par_log_error(CLI_TOK_MODE_CHANGE);     
                    lexCli->par_cb.errFlag = TRUE;
                }               

\![^\!\n]*$     {
                    /* token - Error for unterminated help delimiter */
                    par_log_error(CLI_TOK_HELP_STR);
                    lexCli->par_cb.errFlag = TRUE;
                }              
%%

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			                Lexer Functions.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
int yywrap(void)
{
	return 1;
}

void cli_lcb_set(CLI_CB *pCli)
{
    lexCli = pCli;  
}
 
void par_log_error(uns32 sel)
{
    switch(sel) {
    case CLI_TOK_RANGE:
        lexCli->par_cb.errPos = (uns16)lexCli->par_cb.chCnt;      
        sysf_sprintf(lexCli->par_cb.str, "AT POSITION (%d)", lexCli->par_cb.errPos);        
        m_LOG_NCSCLI_COMMENTS(yytext, NCSCLI_PAR_INVALID_RANGE, lexCli->par_cb.str);
        break;    
        
    case CLI_TOK_DEFAULT_VAL:
        lexCli->par_cb.errPos = (uns16)lexCli->par_cb.chCnt;
        sysf_sprintf(lexCli->par_cb.str, "AT POSITION (%d)", lexCli->par_cb.errPos);                            
        m_LOG_NCSCLI_COMMENTS(yytext, NCSCLI_PAR_INVALID_DEFVAL, lexCli->par_cb.str);
        break;    
        
    case CLI_TOK_HELP_STR:
	    lexCli->par_cb.errPos = (uns16)lexCli->par_cb.chCnt;          
        sysf_sprintf(lexCli->par_cb.str, "AT POSITION (%d)", lexCli->par_cb.errPos);                            
        m_LOG_NCSCLI_COMMENTS(yytext, NCSCLI_PAR_INVALID_HELPSTR, lexCli->par_cb.str);
        break;    
        
    case CLI_TOK_MODE_CHANGE:    
        lexCli->par_cb.errPos = (uns16)lexCli->par_cb.chCnt;                  
        sysf_sprintf(lexCli->par_cb.str, "AT POSITION (%d)", lexCli->par_cb.errPos);        
        m_LOG_NCSCLI_COMMENTS(yytext, NCSCLI_PAR_INVALID_MODECHG, lexCli->par_cb.str);
        break;    
        
    case CLI_TOK_LCURLYBRACE:    
        lexCli->par_cb.errPos = (uns16)lexCli->par_cb.chCnt;      
        sysf_sprintf(lexCli->par_cb.str, "AT POSITION (%d)", lexCli->par_cb.errPos);                
        m_LOG_NCSCLI_COMMENTS(yytext, NCSCLI_PAR_UNTERMINATED_GRPBRACE, lexCli->par_cb.str);
        break;                                              
        
    case CLI_TOK_LBRACE:
        lexCli->par_cb.errPos = (uns16)lexCli->par_cb.chCnt;
        sysf_sprintf(lexCli->par_cb.str, "AT POSITION (%d)", lexCli->par_cb.errPos);        
        m_LOG_NCSCLI_COMMENTS(yytext, NCSCLI_PAR_UNTERMINATED_OPTBRACE, lexCli->par_cb.str);
        break;                                   
    
    case CLI_TOK_CONTINOUS_EXP:
        lexCli->par_cb.errPos = (uns16)lexCli->par_cb.chCnt;
        sysf_sprintf(lexCli->par_cb.str, "AT POSITION (%d)", lexCli->par_cb.errPos);        
        m_LOG_NCSCLI_COMMENTS(yytext, NCSCLI_PAR_INVALID_CONTINOUS_EXP, lexCli->par_cb.str);
        break;           
    }
}

