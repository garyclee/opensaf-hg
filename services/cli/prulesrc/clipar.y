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

  MODULE NAME:  CLIPAR.Y
 
$Header: $ 
..............................................................................

  DESCRIPTION:

  Parser file for symantic validation of token.

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
static CLI_CB *parCli = 0;

/* Global variables */
int32 yylineno;

/* External defined functions */
EXTERN_C int8 *yytext;
EXTERN_C void yyerror(int8 *);
EXTERN_C int32 yylex();

static void
get_range_values(CLI_CMD_ELEMENT *, CLI_TOKEN_ATTRIB, int8 *, int8 *);
static void do_lbrace_opr(CLI_CB *);
static void do_rbarce_opr(CLI_CB *);
static void do_lcurlybrace_opr(CLI_CB *);
static void do_rcurlybrace_opr(CLI_CB *);
                 
#define m_SET_TOKEN_ATTRIBUTE(val)\
{\
    cli_set_token_attrib(parCli,\
        parCli->par_cb.tokList[parCli->par_cb.tokCnt - 1],\
        val, yytext);\
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                Token constants

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
%}
%token CLI_TOK_KEYWORD
%token CLI_TOK_PARAM
%token CLI_TOK_NUMBER
%token CLI_TOK_PASSWORD
%token CLI_TOK_CIDRv4
%token CLI_TOK_IPADDRv4
%token CLI_TOK_IPADDRv6
%token CLI_TOK_MASKv4
%token CLI_TOK_CIDRv6
%token CLI_TOK_RANGE
%token CLI_TOK_DEFAULT_VAL
%token CLI_TOK_HELP_STR
%token CLI_TOK_MODE_CHANGE
%token CLI_TOK_LCURLYBRACE
%token CLI_TOK_RCURLYBRACE
%token CLI_TOK_LBRACE
%token CLI_TOK_RBRACE
%token CLI_TOK_LESS_THAN
%token CLI_TOK_GRTR_THAN
%token CLI_TOK_OR
%token CLI_TOK_CONTINOUS_EXP
%token CLI_TOK_COMMUNITY
%token CLI_TOK_WILDCARD
%token CLI_TOK_MACADDR
%start Command_String

%%
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                Grammar rules

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
Command_String  :   token_param 
                ; 

token_param     :   param_type    
                |   param_type token_param
                |   param_type 
                    CLI_TOK_OR
                    {
                        /* set the OR flag */
                        parCli->par_cb.orFlag = TRUE;                                               
                    }
                    token_param
                ;
                
param_type      :   tokens
                |   expression              
                ;
                
expression      :   left_delimiter delimiter_option right_delimiter 
                ;

delimiter_option    :   tokens
                    |   tokens delimiter_option
                    |   tokens 
                        CLI_TOK_OR
                        {
                            /* set the OR flag */
                            parCli->par_cb.orFlag = TRUE;
                        }
                        delimiter_option
                    |   left_delimiter delimiter_option right_delimiter
                    |   left_delimiter delimiter_option right_delimiter delimiter_option
                    |   left_delimiter delimiter_option right_delimiter 
                        CLI_TOK_OR
                        {
                            /* set the OR flag */
                            parCli->par_cb.orFlag = TRUE;                       
                        }
                        delimiter_option              
                    ;
                              
left_delimiter  :   CLI_TOK_LESS_THAN               
                |   CLI_TOK_LBRACE 
                    {       
						do_lbrace_opr(parCli);
                    }                
                |   CLI_TOK_LCURLYBRACE 
                    {   
						do_lcurlybrace_opr(parCli);                        
                    }
                ;
                           
right_delimiter :   CLI_TOK_GRTR_THAN                   
                |   CLI_TOK_RBRACE  
                    {   
						do_rbarce_opr(parCli);
                    }                                   
                |   CLI_TOK_RCURLYBRACE                 
                    {                     
						do_rcurlybrace_opr(parCli);
                    }
                ;

tokens          :   keyword                     
                |   parameters  
                |   continous_exp            
                ;
                
continous_exp   :   CLI_TOK_CONTINOUS_EXP
                    {
                        /* Evaluate the level and relation type of token */                      
                        cli_evaluate_token(NCSCLI_CONTINOUS_EXP);
                        
                        /* Set the attribute of the token */  
                        m_SET_TOKEN_ATTRIBUTE(CLI_CONTINOUS_RANGE);                      
                        
                        parCli->par_cb.is_tok_cont = TRUE;                        
                    }  
                |   continous_exp
                    CLI_TOK_HELP_STR
                    {
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_HELP_STR);                        
                    }
                ;             
keyword         :   CLI_TOK_KEYWORD 
                    {   
                        /* Evaluate the level and relation type of token */                      
                        cli_evaluate_token(NCSCLI_KEYWORD);                                              
                    }                                   
                |   keyword 
                    CLI_TOK_RANGE 
                    {
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_RANGE_VALUE);                                                
                    }
                |   keyword 
                    CLI_TOK_DEFAULT_VAL 
                    {
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_DEFALUT_VALUE);                        
                    }
                |   keyword 
                    CLI_TOK_HELP_STR 
                    {
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_HELP_STR);                        
                    }
                |   keyword 
                    CLI_TOK_MODE_CHANGE 
                    {
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_MODE_CHANGE);                        
                    }
                ;
                
parameters      :   parameter_type 
                |   parameters 
                    CLI_TOK_RANGE   
                    {
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_RANGE_VALUE);                        
                    }       
                |   parameters 
                    CLI_TOK_DEFAULT_VAL 
                    {
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_DEFALUT_VALUE);                        
                    }
                |   parameters 
                    CLI_TOK_HELP_STR 
                    {
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_HELP_STR);                        
                    }
                |   parameters 
                    CLI_TOK_MODE_CHANGE 
                    {
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_MODE_CHANGE);                        
                    }
                ;
                
parameter_type  :   CLI_TOK_PARAM
                    {   
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_STRING);
                    }                               
                |   CLI_TOK_NUMBER      
                    {   
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_NUMBER);
                    }
                |   CLI_TOK_CIDRv4
                    {
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_CIDRv4);
                    } 
                |   CLI_TOK_IPADDRv4              
                    {   
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_IPv4);
                    }
                |   CLI_TOK_IPADDRv6
                    {
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_IPv6);                 
                    }    
                |   CLI_TOK_MASKv4
                    {   
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_MASKv4);
                    }
                |   CLI_TOK_CIDRv6
                    {   
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_CIDRv6);
                    }
                |   CLI_TOK_PASSWORD
                    {
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_PASSWORD);
                    } 
                |   CLI_TOK_COMMUNITY
                    {
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_COMMUNITY);
                    }       
                |   CLI_TOK_WILDCARD                
                    {
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_WILDCARD);
                    }        
                |   CLI_TOK_MACADDR
                    {
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_MACADDR);
                    }
                ;               
%%

/*****************************************************************************
 Procedure NAME   : do_lbrace_opr
 DESCRIPTION	   : Routine thats does the sematic matching when left barce 
				        is encountered
 ARGUMENTS		   : CLI_CB *
 RETURNS		      : none 
*****************************************************************************/
static void do_lbrace_opr(CLI_CB *pCli)
{
	/*  increment the Optional brace type counter */
	pCli->par_cb.optCntr++;

	/* Set the level */                 
	if(pCli->par_cb.orFlag == TRUE) {
		pCli->par_cb.optLvlCntr = pCli->par_cb.tokLvlCntr;

		if(pCli->par_cb.grpCntr == 0) {   
			if(pCli->par_cb.optCntr > 1)
				pCli->par_cb.relnType = CLI_OPT_SIBLING_TOKEN;
			else
				pCli->par_cb.relnType = CLI_SIBLING_TOKEN;                            
		}
		else pCli->par_cb.relnType = CLI_GRP_SIBLING_TOKEN;
	}
	else {
		pCli->par_cb.optLvlCntr = (uns16)(pCli->par_cb.tokLvlCntr + 1);
    
		if(pCli->par_cb.grpCntr == 0) {   
			if(pCli->par_cb.optCntr > 1)
				pCli->par_cb.relnType = CLI_OPT_CHILD_TOKEN;
			else
				pCli->par_cb.relnType = CLI_CHILD_TOKEN;                                                  
		}
		else pCli->par_cb.relnType = CLI_GRP_CHILD_TOKEN;
	}
                
	/* push the current level into the optional stack */                                                
	pCli->par_cb.optStack[++pCli->par_cb.optStackPtr] = pCli->par_cb.optLvlCntr;

	pCli->par_cb.brcType = CLI_OPT_BRACE;                       
	pCli->par_cb.brcStack[++pCli->par_cb.brcStackPtr] = CLI_OPT_BRACE;

	/*Process token*/
	cli_process_token(pCli->par_cb.optLvlCntr, pCli->par_cb.relnType, NCSCLI_OPTIONAL);                     
	pCli->par_cb.firstOptChild = TRUE;	
}

/*****************************************************************************
 Procedure NAME	: do_lcurlybrace_opr
 DESCRIPTION   	: Routine thats does the sematic matching when left curly barce 
				        is encountered
 ARGUMENTS		   : CLI_CB *
 RETURNS		      : none 
*****************************************************************************/
static void do_lcurlybrace_opr(CLI_CB *pCli)
{
	pCli->par_cb.grpCntr++;
                        
	/* Set the level */                 
	if(pCli->par_cb.orFlag == TRUE) {
		pCli->par_cb.grpLvlCntr = pCli->par_cb.tokLvlCntr;
    
		if(pCli->par_cb.optCntr == 0) {
			if(pCli->par_cb.grpCntr > 1)
				pCli->par_cb.relnType = CLI_GRP_SIBLING_TOKEN;
			else
				pCli->par_cb.relnType = CLI_SIBLING_TOKEN;                            
		}
		else pCli->par_cb.relnType = CLI_OPT_SIBLING_TOKEN;                            
	}
	else {
		pCli->par_cb.grpLvlCntr = (uns16)(pCli->par_cb.tokLvlCntr + 1);
    
		if(pCli->par_cb.optCntr == 0) {
			if(pCli->par_cb.grpCntr > 1)
				pCli->par_cb.relnType = CLI_GRP_CHILD_TOKEN;
			else 
				pCli->par_cb.relnType = CLI_CHILD_TOKEN;                                                  
		}
		else pCli->par_cb.relnType = CLI_OPT_CHILD_TOKEN;                          
	}
                
	/* push the current level into the optional stack */                                                
	pCli->par_cb.grpStack[++pCli->par_cb.grpStackPtr] = pCli->par_cb.grpLvlCntr;
                                                    
	pCli->par_cb.brcType = CLI_GRP_BRACE;                       
	pCli->par_cb.brcStack[++pCli->par_cb.brcStackPtr] = CLI_GRP_BRACE;                      

	/*Process token */
	cli_process_token(pCli->par_cb.grpLvlCntr, pCli->par_cb.relnType, NCSCLI_GROUP);
	pCli->par_cb.firstGrpChild = TRUE;
}

/*****************************************************************************
 Procedure NAME	: do_rbrace_opr
 DESCRIPTION	   : Routine thats does the sematic matching when right barce 
				        is encountered
 ARGUMENTS		   : CLI_CB *
 RETURNS		      : none 
*****************************************************************************/
static void do_rbarce_opr(CLI_CB *pCli)
{
	/* decrement the optional counter */
	pCli->par_cb.optCntr--;

	/* pop the stord level from the stack */
	pCli->par_cb.tokLvlCntr = (uns16)pCli->par_cb.optStack[pCli->par_cb.optStackPtr];    
	pCli->par_cb.optStackPtr--;                   

	pCli->par_cb.optTokCnt = 0;

	--pCli->par_cb.brcStackPtr;
	if(pCli->par_cb.brcStackPtr > -1)
		pCli->par_cb.brcType = pCli->par_cb.brcStack[pCli->par_cb.brcStackPtr];
	else
		pCli->par_cb.brcType = CLI_NONE_BRACE;
    
	/*Process token */
	cli_process_token(pCli->par_cb.grpLvlCntr, CLI_NO_RELATION, NCSCLI_BRACE_END);
}

/*****************************************************************************
 Procedure NAME	: do_rcurlybrace_opr
 DESCRIPTION	   : Routine thats does the sematic matching when right curly barce 
				        is encountered
 ARGUMENTS		   : CLI_CB *
 RETURNS		      : none 
*****************************************************************************/
static void do_rcurlybrace_opr(CLI_CB *pCli)
{
	/* decrement the optional counter */
	pCli->par_cb.grpCntr--;

	/* pop the stord level from the stack */
	pCli->par_cb.tokLvlCntr = (uns16)pCli->par_cb.grpStack[pCli->par_cb.grpStackPtr];    
	pCli->par_cb.grpStackPtr--;

	pCli->par_cb.grpTokCnt = 0;    
	--pCli->par_cb.brcStackPtr;
	if(pCli->par_cb.brcStackPtr > -1)
		pCli->par_cb.brcType = pCli->par_cb.brcStack[pCli->par_cb.brcStackPtr];
	else
		pCli->par_cb.brcType = CLI_NONE_BRACE;
		    
	/*Process token */
	pCli->par_cb.is_tok_cont = FALSE;
	cli_process_token(pCli->par_cb.grpLvlCntr, CLI_NO_RELATION, NCSCLI_BRACE_END);
}

/*****************************************************************************
  Procedure NAME	:	yygettoken
  DESCRIPTION		:	The yygettoken function is called by the parser whenever 
						   it needs a new token. The function returns a zero or 
						   negative value if there are no more tokens, otherwise a 
						   positive value indicating the next token. The default 
						   implementation of yygettoken simply calls yylex.

  ARGUMENTS			:   none
  RETURNS			:	Zero or negative if there are no more tokens for the 
						   parser, otherwise a positive value indicating the next 
						   token.
  NOTES				:
*****************************************************************************/
uns32 yygettoken(void)
{
    return yylex(); 
}

/*****************************************************************************
  Procedure NAME	:	cli_evaluate_token
  DESCRIPTION		:	A function is called by the parser when it encounters any
						   keyword, parameter which may be either NCSCLI_NUMBER, 
						   NCSCI_IP_ADDRESS etc. to evaluate the level of the token and
						   the other necessary attributes that is required by the
						   command tree.
  ARGUMENTS			:	Token type
  RETURNS			:	none
  NOTES				:
*****************************************************************************/
void cli_evaluate_token(NCSCLI_TOKEN_TYPE i_token_type)
{
   if(TRUE == parCli->par_cb.orFlag) {
      parCli->par_cb.orFlag = FALSE;

      switch(parCli->par_cb.brcType) {
      case CLI_OPT_BRACE:                                     
         if(TRUE == parCli->par_cb.firstOptChild) {
            parCli->par_cb.firstOptChild = FALSE;
            parCli->par_cb.relnType  = CLI_OPT_CHILD_TOKEN;                 
            parCli->par_cb.optTokCnt++;
         }
         else parCli->par_cb.relnType  = CLI_OPT_SIBLING_TOKEN;                                    
         break;

      case CLI_GRP_BRACE:
         if(TRUE == parCli->par_cb.firstGrpChild) {
            parCli->par_cb.firstGrpChild = FALSE;
            parCli->par_cb.relnType  = CLI_GRP_CHILD_TOKEN;                 
            parCli->par_cb.grpTokCnt++;
         }
         else parCli->par_cb.relnType = CLI_GRP_SIBLING_TOKEN;                                                
         break;

      case CLI_NONE_BRACE:
         parCli->par_cb.relnType = CLI_SIBLING_TOKEN;
         break;              

      default:
         break;            
      }       
   }
   else {
      parCli->par_cb.tokLvlCntr++;
      parCli->par_cb.optLvlCntr = parCli->par_cb.tokLvlCntr;
      parCli->par_cb.grpLvlCntr = parCli->par_cb.tokLvlCntr;                          
  
      switch(parCli->par_cb.brcType) {
      case CLI_OPT_BRACE:                                     
         parCli->par_cb.optTokCnt++;
				                  
         if(TRUE == parCli->par_cb.firstOptChild)
            parCli->par_cb.firstOptChild = FALSE;

         if(parCli->par_cb.optCntr > 0)
            parCli->par_cb.relnType = CLI_OPT_CHILD_TOKEN;            
         break;

      case CLI_GRP_BRACE:
         parCli->par_cb.grpTokCnt++;
			      
         if(TRUE == parCli->par_cb.firstGrpChild)
            parCli->par_cb.firstGrpChild = FALSE;

         if(parCli->par_cb.grpCntr > 0)
            parCli->par_cb.relnType = CLI_GRP_CHILD_TOKEN;
         break;              

      case CLI_NONE_BRACE:
         parCli->par_cb.relnType = CLI_CHILD_TOKEN;              
         break;                  

      default:
         break;
      }
   }
            
   cli_process_token(parCli->par_cb.tokLvlCntr, parCli->par_cb.relnType, i_token_type);     
}

/*****************************************************************************
  Procedure NAME	:	cli_process_token
  DESCRIPTION		:	A function is called by the parser when it encounters any
						valid token. The function populates the nessary attribute 
						of the token so that it can be added into the command tree
						depending upon the attributes.                    
  ARGUMENTS			:	i_tokenLevel	 - The level of the token.
						i_token_relation - Realtionship of the token
						i_token_type	 - Token type
  RETURNS			:   none
  NOTES				:
*****************************************************************************/
void cli_process_token(int32 i_token_level, CLI_TOKEN_RELATION i_token_relation, 
					   NCSCLI_TOKEN_TYPE i_token_type)
{   
    /* Create token node */ 
    CLI_CMD_ELEMENT *cmd_node = m_MMGR_ALLOC_CLI_CMD_ELEMENT;
    
    if(0 == cmd_node) return;        
    memset(cmd_node, 0, sizeof(CLI_CMD_ELEMENT));
    
    /* Set the token type */
    cmd_node->tokType  = i_token_type;
    cmd_node->tokLvl = (uns8)i_token_level;          
    
    if(TRUE == parCli->par_cb.is_tok_cont)
        cmd_node->isCont = TRUE;        
    else
        cmd_node->isCont = FALSE;         
        
    /* Set the token type ie Mandatory or Optional */
   switch(i_token_relation) {           
   case CLI_CHILD_TOKEN:
      if(NCSCLI_OPTIONAL == i_token_type)
         cmd_node->isMand = FALSE;
      else                        
         cmd_node->isMand = TRUE;                                           
      break;

   case CLI_OPT_CHILD_TOKEN:
      cmd_node->isMand = FALSE;                              
      break;          

   case CLI_GRP_CHILD_TOKEN:
   cmd_node->isMand = TRUE;                   
      if(1 == parCli->par_cb.grpTokCnt && parCli->par_cb.optCntr > 0)
         cmd_node->isMand = FALSE;
      else if(i_token_type == NCSCLI_OPTIONAL)
         cmd_node->isMand = FALSE;
      break;

   case CLI_SIBLING_TOKEN:
      if(NCSCLI_OPTIONAL == i_token_type)
         cmd_node->isMand = FALSE;
      else            
         cmd_node->isMand = TRUE;                                       
      break;

   case CLI_OPT_SIBLING_TOKEN:     
      cmd_node->isMand = FALSE;                      
      break;

   case CLI_GRP_SIBLING_TOKEN:                                 
      cmd_node->isMand = TRUE;               
      break;              

   case CLI_NO_RELATION:
      break;      

   default:
      break;
   }       
        
   if(CLI_NO_RELATION != i_token_relation)
   {   
      if(TRUE == parCli->par_cb.firstOptChild)
         parCli->par_cb.firstOptChild = FALSE;       
        
      if(TRUE == parCli->par_cb.firstGrpChild)
         parCli->par_cb.firstGrpChild = FALSE;       
        
      /* Assign the token name */         
      switch(i_token_type) {           
      case NCSCLI_KEYWORD:
		   cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(yytext)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
			m_NCS_OS_STRCPY(cmd_node->tokName, yytext);                
         break;

      case NCSCLI_CONTINOUS_EXP:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_CONTINOUS_EXP)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_CONTINOUS_EXP);                
         break;    

      case NCSCLI_GROUP:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_GRP_NODE)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_GRP_NODE);
         break;

      case NCSCLI_OPTIONAL:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_OPT_NODE)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_OPT_NODE);
         break;      

      case NCSCLI_STRING:
		   cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_STRING)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_STRING);
         break;

      case NCSCLI_NUMBER:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_NUMBER)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_NUMBER);
         break;

      case NCSCLI_CIDRv4:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_CIDRv4)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_CIDRv4);
         break;

      case NCSCLI_IPv4:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_IPADDRESSv4)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_IPADDRESSv4);
         break;

      case NCSCLI_IPv6:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_IPADDRESSv6)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_IPADDRESSv6);
         break;

      case NCSCLI_MASKv4:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_IPMASKv4)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_IPMASKv4);
         break;

      case NCSCLI_CIDRv6:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_CIDRv6)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_CIDRv6);
         break;                

      case NCSCLI_PASSWORD:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_PASSWORD)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_PASSWORD);
         break;

      case NCSCLI_COMMUNITY:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_COMMUNITY)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_COMMUNITY);
         break;  

      case NCSCLI_WILDCARD:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_WILDCARD)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_WILDCARD);
         break;

      case NCSCLI_MACADDR:
			cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(CLI_MACADDR)+1);
			if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         m_NCS_OS_STRCPY(cmd_node->tokName, CLI_MACADDR);
         break;

      default:
         break;
      }
        
		sysf_sprintf(parCli->par_cb.str, "OF LEVEL (%d)", i_token_level);     
      m_LOG_NCSCLI_COMMENTS(cmd_node->tokName, (TRUE == cmd_node->isMand)?
                     NCSCLI_PAR_MANDATORY_TOKEN:NCSCLI_PAR_OPTIONAL_TOKEN, 
                     parCli->par_cb.str);             
   }
        
   /* Add token into the command tree */
   cmd_node->tokRel = i_token_relation;
   parCli->par_cb.tokList[parCli->par_cb.tokCnt] = cmd_node;
   parCli->par_cb.tokCnt++;   
}

/*****************************************************************************
  Procedure NAME	:	yyerror
  DESCRIPTION		:	A function is called by the parser when it encounters any
						parse error to notify the user specying the line no and 
						the error token.                    
  ARGUMENTS			:	The error string that is to flushed to the user.
  RETURNS			:   none
  NOTES				:
*****************************************************************************/
void yyerror(int8 *text)
{
    if(m_NCS_OS_STRLEN(yytext) != 0)
       m_NCS_CONS_PRINTF("Error on line %d at %s (%s)\n",yylineno, yytext, text);
}

/*****************************************************************************
  Procedure NAME	:	cli_pcb_set
  DESCRIPTION		:   A function is called by the parser when it encounters any
						parse error to notify the user specying the line no and 
						the error token.                    
  ARGUMENTS			:	Pointer to CLI control block
  RETURNS			:   none
  NOTES				:
*****************************************************************************/
void cli_pcb_set(CLI_CB *pCli)
{
    parCli = pCli;  
}

/*****************************************************************************
  PROCEDURE NAME	:	cli_set_token_attrib
  DESCRIPTION		:	The function assigns the diffrent attributes parsed by
                     the to the command node that is recently added into the
                     tree.        
                    
  ARGUMENTS			:	pCli - Pointer to CLI control block
						   i_node - Poinetr to the token node
						   i_token_attrib - Type of the attribute e.g. Help string,
                     range value,mode change etc.
						   i_value - Value of the attribute.          
  RETURNS			:  none
  NOTES				:	1. Scan the type of attribute.
                     2. covert the value of the attribute to there nessasery 
                        data type if required. e.g uns32, ipaddress etc.
                     3. Store the attribute in the corresponding value of 
                        the attribute in the node that is currently added
*****************************************************************************/
void cli_set_token_attrib(CLI_CB             *pCli, 
		                    CLI_CMD_ELEMENT    *i_node, 
                          CLI_TOKEN_ATTRIB   i_token_attrib, 
                          int8               *i_value)
{
   uns32   token_len;
   int8    *token = 0;    

   if(!i_node) return;

   switch(i_token_attrib) {
   case CLI_HELP_STR:
      token = sysf_strtok(i_value, CLI_HELP_IDENTIFIER);

		i_node->helpStr = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(token)+1);
		if(!i_node->helpStr) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
      m_NCS_OS_STRCPY(i_node->helpStr, token);            
      break;    

   case CLI_DEFALUT_VALUE:
      token = sysf_strtok(i_value, CLI_DEFAULT_IDENTIFIER);

      if(NCSCLI_NUMBER == i_node->tokType) {
         i_node->defVal = (uns32 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(uns32));
         if(!i_node->defVal) return;
         *((uns32 *)i_node->defVal) = atoi(token);
      }
      else {
         token_len = m_NCS_OS_STRLEN(token) + 1;
         i_node->defVal = (int8 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(int8) * token_len);
         if(!i_node->defVal) return;
         m_NCS_OS_STRCPY(((int8 *)i_node->defVal), token);
      }
      break;        

   case CLI_RANGE_VALUE:
	   {
		   int8    range_dilimeter[] = "<..>";           
			get_range_values(i_node, i_token_attrib, i_value, range_dilimeter);
		}
      break;        
    
    case CLI_CONTINOUS_RANGE:
		{
			int8    range_dilimeter[] = "(...)";   
			get_range_values(i_node, i_token_attrib, i_value, range_dilimeter);        
		}
		break;

   case CLI_MODE_CHANGE:
      token = sysf_strtok(i_value, CLI_MODE_IDENTIFIER);
   
      i_node->modChg = TRUE;
      i_node->nodePath = m_MMGR_ALLOC_CLI_DEFAULT_VAL(m_NCS_OS_STRLEN(token)+1);
		if(!i_node->nodePath) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
      m_NCS_OS_STRCPY(i_node->nodePath, token);            
      break;
    
   case CLI_DO_FUNC:
      cli_set_cef(pCli);
      break;        
    
   default:
      break;
   }
}

static void
get_range_values(CLI_CMD_ELEMENT    *i_node, 
                 CLI_TOKEN_ATTRIB   i_token_attrib, 
                 int8               *i_value,
                 int8				*delimiter)
{
   int8    *token = 0;
   uns32   token_len = 0;    
   int	   val = 0;		

   i_node->range = m_MMGR_ALLOC_CLI_RANGE;
   if(!i_node->range) return;

   memset(i_node->range, 0, sizeof(CLI_RANGE));
   token = sysf_strtok(i_value, delimiter);

   if(NCSCLI_NUMBER == i_node->tokType ||
   NCSCLI_CONTINOUS_EXP == i_node->tokType) {
      i_node->range->lLimit = (uns32 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(uns32));
   
      if(!i_node->range->lLimit) return;
	  sscanf(token, "%u", &val);
      *((uns32 *)i_node->range->lLimit) = val;
   }
   else {
      token_len = m_NCS_OS_STRLEN(token) + 1;
   
      i_node->range->lLimit = (int8 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(int8) * token_len);
      if(!i_node->range->lLimit) return;
      m_NCS_OS_STRCPY(((int8 *)i_node->range->lLimit), token);
   }

   while(0 != token) {
      token = sysf_strtok(0, delimiter);
   
      if(token) {               
         if(NCSCLI_NUMBER == i_node->tokType ||
            NCSCLI_CONTINOUS_EXP == i_node->tokType) {
            i_node->range->uLimit = (uns32 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(uns32));
            if(!i_node->range->uLimit) return;
			sscanf(token, "%u", &val);
            *((uns32 *)i_node->range->uLimit) = val;
         }
         else
         {
            token_len = m_NCS_OS_STRLEN(token) + 1;
            i_node->range->uLimit = (int8 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(int8) * token_len);
            if(!i_node->range->uLimit) return;
            m_NCS_OS_STRCPY(((int8 *)i_node->range->uLimit), token);
         }
      }
   }
}