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

 MODULE NAME:  CLICTREE.C

$Header: $
..............................................................................

  DESCRIPTION:

  Source file for Command Tree and its utility functions.

******************************************************************************
*/

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "cli.h"

#if (NCS_CLI == 1)
static void cli_push_bindery(CLI_BINDERY_LIST **, NCSCLI_BINDERY *);
static void cli_free_bindery(CLI_BINDERY_LIST **);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Command Tree Variables

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/* Extern variables */
EXTERN_C FILE *yyin;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        COMMAND TREE BUILDING FUNCTIONS

This section defines the functions that are used by the command tree for 
building the tree with the token passed by the Parser and the utily functions 
that are used for manpulation and travering the tree.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*****************************************************************************
  Function NAME   :  cli_reset_all_marker
  DESCRIPTION     :  The function resets all the global marker that are being 
                     used for building command tree.                    
  ARGUMENTS       :  pCli - Control Block pointer    
  RETURNS         :  none
  NOTES           :
*****************************************************************************/
void cli_reset_all_marker(CLI_CB *pCli)
{
    pCli->ctree_cb.ctxtMrkr = 0;
    pCli->ctree_cb.cmdMrkr = 0;
    pCli->ctree_cb.lvlMrkr = 0;
    pCli->ctree_cb.optMrkr = 0;
    pCli->ctree_cb.orMrkr = 0;
    pCli->ctree_cb.currMrkr = 0;
    pCli->ctree_cb.contMrkr = 0;
}

/*****************************************************************************
  PROCEDURE NAME  :  CLI_ADD_CMD_NODE
  DESCRIPTION     :  A function is called by the parser for adding token node 
                     into the command tree.                    
  ARGUMENTS       :  pCli           - Pointer of the control block
                     i_node         - Pointer to a CLI_CMD_ELEMENT information block.   
                     i_relationship - Relationship of the node with the prevoius node.
  RETURNS         :  Sucess or Failure
  NOTES           :  1. Scan relationship of the token.
                     2. Add the node into the command tree as per the relationship of the token.
                     3. Reset the corresponding markers.
*****************************************************************************/
void cli_add_cmd_node(CLI_CB  *pCli, CLI_CMD_ELEMENT *i_node, 
                      CLI_TOKEN_RELATION i_relationship)
{   
   /*Initilize local variables */   
   pCli->ctree_cb.nodeFound = FALSE;    
   
   switch(i_relationship) {
   /* CLI_CHILD_TOKEN: specifies that the token node that has been 
    * passed by the parser is a mandatory node and needs to added as the 
    * child of the previous token node 
   */
   case CLI_CHILD_TOKEN:
      /* Set the level of the node */
      i_node->prntTokLvl = i_node->tokLvl;
      
      if(CLI_START_CMD == i_node->tokLvl) {
         /* Reset the level marker to 0 */
         pCli->ctree_cb.lvlMrkr = 0;         
         pCli->ctree_cb.cmdMrkr = pCli->ctree_cb.ctxtMrkr->pCmdl;
         
         if(!pCli->ctree_cb.cmdMrkr) {
            pCli->ctree_cb.ctxtMrkr->pCmdl = i_node;                     
            pCli->ctree_cb.cmdMrkr = i_node;
            i_node->cmd_access_level = i_node->cmd_access_level | pCli->ctree_cb.cmd_access_level; /*Fix for 59359 */
            
            /* Assign the old node as parent of the new node */
            i_node->node_ptr.pParent = pCli->ctree_cb.ctxtMrkr->pCmdl;
         }
         else
         {
            /* Check if the token node already exist or not */                    
            pCli->ctree_cb.nodeFound = (uns8)cli_cmd_ispresent(&pCli->ctree_cb.cmdMrkr,
               i_node->tokName);
            
            if(FALSE == pCli->ctree_cb.nodeFound) {   
               if(CLI_CMD_ADDED_PARTIALL == cli_append_node(
                  pCli->ctree_cb.ctxtMrkr->pCmdl, pCli->ctree_cb.cmdMrkr, &i_node)) {
                  pCli->ctree_cb.ctxtMrkr->pCmdl = i_node;
                  i_node->node_ptr.pParent = pCli->ctree_cb.ctxtMrkr->pCmdl;
               }
               pCli->ctree_cb.cmdMrkr = i_node;                        
            i_node->cmd_access_level = i_node->cmd_access_level | pCli->ctree_cb.cmd_access_level; /*Fix for 59359 */
            } 
            else {
               pCli->ctree_cb.lvlMrkr = pCli->ctree_cb.cmdMrkr;
               pCli->ctree_cb.cmdMrkr->cmd_access_level = pCli->ctree_cb.cmdMrkr->cmd_access_level|pCli->ctree_cb.cmd_access_level; /*fix for 59359 */
               cli_free_cmd_element(&i_node);
               i_node = pCli->ctree_cb.cmdMrkr;               
            }
         }
         
         m_LOG_NCSCLI_COMMENTS(i_node->tokName, NCSCLI_REL_CHILDOF, 
            pCli->ctree_cb.ctxtMrkr->name);
      }
      else
      {
         /* Add the new node as child of the above level */
         if(0 == pCli->ctree_cb.lvlMrkr->node_ptr.pChild)
            pCli->ctree_cb.lvlMrkr->node_ptr.pChild = i_node;
         else {
            pCli->ctree_cb.lvlMrkr = pCli->ctree_cb.lvlMrkr->node_ptr.pChild;                            
            pCli->ctree_cb.nodeFound = (uns8)cli_cmd_ispresent(
               &pCli->ctree_cb.lvlMrkr, i_node->tokName);
            
            if(FALSE == pCli->ctree_cb.nodeFound)
               pCli->ctree_cb.lvlMrkr->node_ptr.pSibling = i_node;                                            
         }         
         
         /* Assign the old node as parent of the new node */
         if(FALSE == pCli->ctree_cb.nodeFound) {
            m_LOG_NCSCLI_COMMENTS(i_node->tokName, NCSCLI_REL_CHILDOF, 
               pCli->ctree_cb.lvlMrkr->tokName);            
            i_node->node_ptr.pParent = pCli->ctree_cb.lvlMrkr;                
            i_node->cmd_access_level = i_node->cmd_access_level|pCli->ctree_cb.cmd_access_level;  /*Fix for 59359 */
         }
         else {
            m_LOG_NCSCLI_COMMENTS(i_node->tokName, NCSCLI_REL_CHILDOF, 
               pCli->ctree_cb.lvlMrkr->node_ptr.pParent->tokName);
               pCli->ctree_cb.lvlMrkr->cmd_access_level = pCli->ctree_cb.lvlMrkr->cmd_access_level|pCli->ctree_cb.cmd_access_level; /*fix for 59359 */
            cli_free_cmd_element(&i_node);
            i_node = pCli->ctree_cb.lvlMrkr;            
         }
      }                
      
      /* Set the level pointer to this new node */
      if(FALSE == pCli->ctree_cb.nodeFound) pCli->ctree_cb.lvlMrkr = i_node;            
      
      /* Reset all pointer to new level pointer */
      pCli->ctree_cb.optMrkr = pCli->ctree_cb.lvlMrkr;
      pCli->ctree_cb.orMrkr = pCli->ctree_cb.lvlMrkr;            
      break;
        
   /* CLI_SIBLING_TOKEN : specifies that new token node that has been passed by
    * the parser is an mandatory node and has to be OR with the previous node in 
    * that level. The new node is added as the sibling node of that last node in
    * that level 
   */
   case CLI_SIBLING_TOKEN:
      /* Add the new node as sibling of the previous node in that level */
      pCli->ctree_cb.orMrkr = pCli->ctree_cb.lvlMrkr;
      
      pCli->ctree_cb.nodeFound = (uns8)cli_cmd_ispresent(&pCli->ctree_cb.orMrkr,
         i_node->tokName);
      
      if(FALSE == pCli->ctree_cb.nodeFound) {
         pCli->ctree_cb.orMrkr->node_ptr.pSibling = i_node;         
         m_LOG_NCSCLI_COMMENTS(i_node->tokName, NCSCLI_REL_SIBLINGOF, 
            pCli->ctree_cb.orMrkr->tokName);
         
         i_node->node_ptr.pParent = pCli->ctree_cb.orMrkr;
         i_node->cmd_access_level = i_node->cmd_access_level|pCli->ctree_cb.cmd_access_level; /* Fix for 59359 */
         pCli->ctree_cb.orMrkr = i_node;
         
         /* reset all pointers */
         pCli->ctree_cb.optMrkr = pCli->ctree_cb.orMrkr;            
      }
      else {
          pCli->ctree_cb.orMrkr->cmd_access_level = pCli->ctree_cb.orMrkr->cmd_access_level|pCli->ctree_cb.cmd_access_level; /*Fix for 59359 */
         cli_free_cmd_element(&i_node);
         i_node = pCli->ctree_cb.orMrkr;         
      }
      break;      
           
   /* CLI_OPT_CHILD_TOKEN : specifies that new token node that has been passed by 
    * the parser is an optional node and has to added under the optional dummy node.
    * All the optional token nodes has to be data of the optional dummy node 
   */
   case CLI_OPT_CHILD_TOKEN:
   case CLI_GRP_CHILD_TOKEN:      
      /* check if the data pointer of the optional dummy node is null */
      if(((NCSCLI_OPTIONAL == pCli->ctree_cb.optMrkr->tokType) || 
         (NCSCLI_GROUP == pCli->ctree_cb.optMrkr->tokType)) &&
         0 == pCli->ctree_cb.optMrkr->node_ptr.pDataPtr) {    
         if((NCSCLI_GROUP == pCli->ctree_cb.optMrkr->tokType) &&
            FALSE == i_node->isMand) {
            pCli->ctree_cb.optMrkr->isMand = FALSE;
         }
         
         pCli->ctree_cb.optMrkr->node_ptr.pDataPtr = i_node;  
         i_node->cmd_access_level = i_node->cmd_access_level|pCli->ctree_cb.cmd_access_level; /* Fix for 59359 */
         if(i_node->isMand == FALSE) {
            m_LOG_NCSCLI_COMMENTS(i_node->tokName, NCSCLI_REL_OPTCHILDOF, 
               pCli->ctree_cb.optMrkr->tokName);
         }
         else {                    
            m_LOG_NCSCLI_COMMENTS(i_node->tokName, NCSCLI_REL_CHILDOF, 
               pCli->ctree_cb.optMrkr->tokName);
         }
      }
      else {
         /* Add the new node as child of the above level */
         if(pCli->ctree_cb.optMrkr->tokLvl != i_node->tokLvl) {
            if(i_node->tokType != NCSCLI_OPTIONAL)
               i_node->isMand = TRUE;
            
            pCli->ctree_cb.optMrkr->node_ptr.pChild = i_node;                    
            i_node->cmd_access_level = i_node->cmd_access_level|pCli->ctree_cb.cmd_access_level; /* Fix for 59359 */
         }
         else {
            pCli->ctree_cb.optMrkr = pCli->ctree_cb.optMrkr->node_ptr.pDataPtr;                
            pCli->ctree_cb.nodeFound = (uns8)cli_cmd_ispresent(
               &pCli->ctree_cb.optMrkr, i_node->tokName);
            
            if(FALSE == pCli->ctree_cb.nodeFound)
               pCli->ctree_cb.optMrkr->node_ptr.pSibling = i_node;                        
         }                
         
         if(NCSCLI_OPTIONAL == i_node->tokType) {
            m_LOG_NCSCLI_COMMENTS(i_node->tokName, NCSCLI_REL_OPTCHILDOF, 
               pCli->ctree_cb.optMrkr->tokName);
         }
         else {                    
            m_LOG_NCSCLI_COMMENTS(i_node->tokName, NCSCLI_REL_CHILDOF, 
               pCli->ctree_cb.optMrkr->tokName);
         }     
      }            
      
      if(FALSE == pCli->ctree_cb.nodeFound) {
         i_node->node_ptr.pParent = pCli->ctree_cb.optMrkr;
         i_node->cmd_access_level = i_node->cmd_access_level|pCli->ctree_cb.cmd_access_level; /*Fix for 59359 */
         pCli->ctree_cb.optMrkr = i_node;
      }
      else {
         pCli->ctree_cb.optMrkr->cmd_access_level = pCli->ctree_cb.optMrkr->cmd_access_level|pCli->ctree_cb.cmd_access_level; /*Fix for 59359 */
         cli_free_cmd_element(&i_node);
         i_node = pCli->ctree_cb.optMrkr;         
      }
      
      if(TRUE == i_node->isCont) pCli->ctree_cb.contMrkr = i_node;
      break;
      
   /* CLI_OPT_SIBLING_TOKEN : specifies that the new node passed by the parser 
    * is an optional node and has to be added as the sibling to last node 
    * linked with the data pointer of the optional node. 
   */
   case CLI_OPT_SIBLING_TOKEN:
   case CLI_GRP_SIBLING_TOKEN:
      {
         CLI_CMD_ELEMENT   *tempMarker = 0;         
         
         /* Add the new node as sibling of the previous node in that level */                   
         pCli->ctree_cb.optMrkr = pCli->ctree_cb.dummyStack[pCli->ctree_cb.dummyStackMrkr];         
         if(TRUE == i_node->isCont) tempMarker = pCli->ctree_cb.contMrkr;           
         else {
            tempMarker = pCli->ctree_cb.optMrkr;
            tempMarker = tempMarker->node_ptr.pDataPtr;
         }
         
         pCli->ctree_cb.nodeFound = (uns8)cli_cmd_ispresent(
            &tempMarker, i_node->tokName);
         
         if(FALSE == pCli->ctree_cb.nodeFound) tempMarker->node_ptr.pSibling = i_node;
         
         if(FALSE == i_node->isMand) {   
            m_LOG_NCSCLI_COMMENTS(i_node->tokName, NCSCLI_REL_OPTSIBLINGOF, 
               tempMarker->tokName);
         }
         else {                
            m_LOG_NCSCLI_COMMENTS(i_node->tokName, NCSCLI_REL_SIBLINGOF, 
               tempMarker->tokName);
         }
         
         if(FALSE == pCli->ctree_cb.nodeFound) {
            i_node->node_ptr.pParent = tempMarker;
            i_node->cmd_access_level = i_node->cmd_access_level|pCli->ctree_cb.cmd_access_level; /*Fix for 59359 */
            pCli->ctree_cb.optMrkr = i_node;
            pCli->ctree_cb.contMrkr = i_node;
         }
         else {
             tempMarker->cmd_access_level = tempMarker->cmd_access_level|pCli->ctree_cb.cmd_access_level; /*Fix for 59359 */
            cli_free_cmd_element(&i_node);
            i_node = tempMarker;            
         }                  
      }
      break;
      
   default:
      break;
   }
    
   /* Put the dummy node into stack */
   if(NCSCLI_OPTIONAL == i_node->tokType || NCSCLI_GROUP == i_node->tokType)
      cli_set_dummy_marker(pCli);
   
   pCli->ctree_cb.currMrkr = i_node;    

   /* Push the node into the deregistrtion stack if the DEREGISTRATION is set */
   if(pCli->ctree_cb.dereg)
      pCli->ctree_cb.dregStack[++pCli->ctree_cb.dregStackMrkr] = i_node;
}

/*****************************************************************************
  PROCEDURE NAME  :  cli_parse_node
  DESCRIPTION:       This function is invoked by the registration API of CLI at
                     time of registration of protcol with the CLI, to create a 
                     new protocol node in the commnd tree and set the command 
                     marker to the protocol node so that all the commands 
                     associated with the protocol will be added under this new 
                     node.         
  ARGUMENTS       :  pCli  - Control Block pointer   
                     pList - Pointer to NCSCLI_CMD_LIST structure that include 
                             path to parse.
                     cookie  Tells whether to set the value after parsing, 
                              TRUE   : Parse and set the value
                              FALSE  : Parse only   
  RETURNS         :  none
  NOTES           :  1. Parse each token delimited by slash '/'
                     2. Invoke cli_find_add_node function to add the node into 
                        the command tree.
*****************************************************************************/
uns32 cli_parse_node(CLI_CB *pCli, NCSCLI_CMD_LIST *pList, NCS_BOOL cookie)
{
   int8 *token = 0;
   int8 cmd_path[CLI_BUFFER_SIZE];
   
   /* Copy the path string into an array */
   m_NCS_OS_STRCPY(cmd_path, pList->i_node);    
   
   /* extract the root node from the path */   
   token = sysf_strtok(cmd_path, CLI_TOK_SEPARATOR);
   
   if(!pCli->ctree_cb.rootMrkr && cookie) {
      pCli->ctree_cb.rootMrkr = m_MMGR_ALLOC_CLI_CMD_NODE;
      if(!pCli->ctree_cb.rootMrkr) return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
      
      m_NCS_OS_MEMSET(pCli->ctree_cb.rootMrkr, 0, sizeof(CLI_CMD_NODE));
      m_NCS_OS_STRCPY(pCli->ctree_cb.rootMrkr->name, token);            
   }
   
   /* Set the context pointer to the start of the tree*/
   pCli->ctree_cb.ctxtMrkr = pCli->ctree_cb.rootMrkr;
   
   /* extract the root node from the path */   
   token = sysf_strtok(0, CLI_TOK_SEPARATOR);
   
   /* Traverse the tree to the specified node */
   while(0 != token) {
      cli_find_add_node(pCli, token, cookie);       
      token = sysf_strtok(0, CLI_TOK_SEPARATOR);
      
      if(!cookie && !pCli->ctree_cb.nodeFound)
         return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
      
      if(TRUE == pCli->ctree_cb.nodeFound)
         pCli->ctree_cb.nodeFound = FALSE;
   }
   
   if(!cookie) return NCSCC_RC_SUCCESS;
   
   /*Set the pasword with the node */
   if(pList->i_access_passwd) 
      pCli->ctree_cb.ctxtMrkr->accPswd = *(pList->i_access_passwd);   
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME  :  cli_set_cmd_to_parse
  DESCRIPTION     :  This function is invoked by the registration API of CLI at
                     time of registration of protcol with the CLI. Registration
                     API calles this function to fill the input buffer of the 
                     parser for each of the command associated with the protocol,
                     that defined in Netplane Command Language.
  ARGUMENTS       :  pCli     - Control Block pointer
                     src      - Pointer to the buffer. This may be either a 
                                int8 * or FILE *.    
                     buffType - Buffer type.   
  RETURNS         :  none
  NOTES           :  1. Populate the input buffer of the Parser. 
                     2. Depending upon the bfrType
*****************************************************************************/
void cli_set_cmd_to_parse(CLI_CB *pCli, int8 *src, CLI_BUFFER_TYPE buffType)
{   
   /* Reset the buffer contents to null */
   m_NCS_OS_MEMSET(pCli->par_cb.ipbuffer, 0, CLI_BUFFER_SIZE * sizeof(char));
   
   /* Ccheck buffer type */
   switch(buffType)
   {
   /* Copy contents into command buffer if buffer type is CLI_CMD_BUFFER */
   case CLI_CMD_BUFFER:
      if(src) {                
         m_NCS_OS_STRCPY(pCli->par_cb.ipbuffer, src);
         strcat(pCli->par_cb.ipbuffer, CLI_TOK_EOC);
      }
      else m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_INVALID_PTR);      
      
      /* Set the buffer type to command buffer */
      pCli->par_cb.bfrType = buffType;
      break;   
      
   /* Assign the file pointer to parser file variable if buffer type
    * is CLI_FILE_BUFFER 
   */
   case CLI_FILE_BUFFER:
      /* Open the specified file */
      if(0 != src) yyin = fopen(src, "r+");               
      else {    
         m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_INVALID_PTR);
      }
      
      /* Set the buffer type to file buffer */
      pCli->par_cb.bfrType = buffType;
      break;

   default:
      break;      
   }
   
   /* Assign the control block pointer to tokeniser & parser */
   cli_lcb_set(pCli);   
   cli_pcb_set(pCli);
   
   /* Invoke parser to parse the command defined in Netplane Command Language */
   yyparse();
}

/*****************************************************************************
  PROCEDURE NAME  :  CLI_FIND_ADD_NODE
  DESCRIPTION     :  The function is invoked by ncscli_parseNodePath function for
                     adding the protocol node into the commnad tree. The function 
                     checks the presence of the protocol node before adding the
                     node into the commmand tree.

  ARGUMENTS       :  pCli   - Control Block pointer
   int8:             tok    - Node name that is to be created and added into the
                              command tree.
                     cookie - Tells whether to create the node if not found, 
                                 TRUE   : Find and create if doesn't exist
                                 FALSE  : Find only   
  RETURNS         :  none
  NOTES           :  1. Looks for the presence of the token that is to be added
                     2. Ignores the token if it already exists
                     3. Create a new node and adds it to the command tree.
                     4. Reset the context marker to the current node.
*****************************************************************************/
void cli_find_add_node(CLI_CB *pCli, int8 *tok, NCS_BOOL cookie)
{
   uns32           ret_value = 0;
   CLI_CMD_NODE    *parent_ptr = 0;
   
   if(0 != pCli->ctree_cb.ctxtMrkr->pChild)
   {
      /* set the marker to the child node */
      pCli->ctree_cb.ctxtMrkr = pCli->ctree_cb.ctxtMrkr->pChild;
      
      /* loop to check the exsistence of the node */
      while(0 != pCli->ctree_cb.ctxtMrkr)
      {
         ret_value = m_NCS_OS_STRCMP(pCli->ctree_cb.ctxtMrkr->name, tok);
         
         /* check for the command match */
         if(!ret_value) {
            pCli->ctree_cb.nodeFound = TRUE;                       
            break;
         }
         else {
            parent_ptr = pCli->ctree_cb.ctxtMrkr;
            pCli->ctree_cb.ctxtMrkr = pCli->ctree_cb.ctxtMrkr->pSibling;
         }
      }
      
      /* create a new node for the token and add to the command tree */
      if(FALSE == pCli->ctree_cb.nodeFound) {   
         pCli->ctree_cb.ctxtMrkr = parent_ptr;
         pCli->ctree_cb.ctxtMrkr->pSibling = m_MMGR_ALLOC_CLI_CMD_NODE;
         if(!pCli->ctree_cb.ctxtMrkr->pSibling) return;
         
         if(!cookie) return;  /* Do not create the node */
         
         m_NCS_OS_MEMSET(pCli->ctree_cb.ctxtMrkr->pSibling, 0, sizeof(CLI_CMD_NODE)); 
         m_NCS_OS_STRCPY(pCli->ctree_cb.ctxtMrkr->pSibling->name, tok);
         pCli->ctree_cb.ctxtMrkr->pSibling->pParent = pCli->ctree_cb.ctxtMrkr;
         pCli->ctree_cb.ctxtMrkr = pCli->ctree_cb.ctxtMrkr->pSibling;
      } 
   }
   else {
      if(!cookie) return;  /* Do not create the node */
      pCli->ctree_cb.ctxtMrkr->pChild = m_MMGR_ALLOC_CLI_CMD_NODE;        
      if(!pCli->ctree_cb.ctxtMrkr->pChild) return;
      m_NCS_OS_MEMSET(pCli->ctree_cb.ctxtMrkr->pChild, 0, sizeof(CLI_CMD_NODE));    
      m_NCS_OS_STRCPY(pCli->ctree_cb.ctxtMrkr->pChild->name, tok);
      
      /* Reset the context marker */
      pCli->ctree_cb.ctxtMrkr->pChild->pParent = pCli->ctree_cb.ctxtMrkr;
      pCli->ctree_cb.ctxtMrkr = pCli->ctree_cb.ctxtMrkr->pChild;
   }   
}

/*****************************************************************************
  PROCEDURE NAME  :  cli_reset_dummy_marker
  DESCRIPTION     :  This function is used for manipulation of dummy node marker. 
                     The function resets the marker to the previous marker from 
                     the stack.
  ARGUMENTS       :  pCli - Control Block pointer   
  RETURNS         :  none
  NOTES           :  1. Pop the dummy node marker from the dummy node stack
                     2. Reset the previous dummy node marker to this marker.
*****************************************************************************/
void cli_reset_dummy_marker(CLI_CB *pCli)
{
   pCli->ctree_cb.optMrkr = pCli->ctree_cb.dummyStack[pCli->ctree_cb.dummyStackMrkr--];
}

/*****************************************************************************
  PROCEDURE NAME  :  cli_set_dummy_marker
  DESCRIPTION     :  This function is used for manipulation of dummy node marker. 
                     The function sets the marker to the current dummy command 
                     node and adds that into the stack.                   
  ARGUMENTS       :  pCli - Control Block pointer
  RETURNS         :  none
  NOTES           :  1. Push the current dummy node marker into the dummy node 
                        stack.
*****************************************************************************/
void cli_set_dummy_marker(CLI_CB *pCli)
{
    pCli->ctree_cb.dummyStack[++pCli->ctree_cb.dummyStackMrkr] = pCli->ctree_cb.optMrkr;
}

/*****************************************************************************
  PROCEDURE NAME  :  cli_cmd_ispresent
  DESCRIPTION     :  The function check the presence of the token in the command
                     tree.
  ARGUMENTS       :  node - Pointer to pointer of CLI_CMD_ELEMENT structure
                     tok  - Token name   
  RETURNS         :  TRUE or FALSE
  NOTES           :
*****************************************************************************/
NCS_BOOL cli_cmd_ispresent(CLI_CMD_ELEMENT **node, int8* tok)
{
   CLI_CMD_ELEMENT *temp_node = *node;
   NCS_BOOL         flag = FALSE;        
   
   /* Check if the token node already exist or not */
   while(0 != (*node)) {        
      if(0 != m_NCS_OS_STRCMP((*node)->tokName, tok)) {            
         temp_node = *node;
         *node = (*node)->node_ptr.pSibling;            
      }
      else {
         flag = TRUE;                        
         break;
      }
   }
   
   /*sort data bfero the node is to added in case it doen't exist */
   if(FALSE == flag) *node = temp_node;
   return flag;
}

/*****************************************************************************
  Function NAME   :  cli_append_node
  DESCRIPTION     :  The function append a node into the tree in a sorted manner                    
  ARGUMENTS       :  root  - Pointer of CLI_CMD_ELEMENT structure(root node)
                     cnode - Pointer of CLI_CMD_ELEMENT structure(current node)
                     i_node - Pointer to pointer of CLI_CMD_ELEMENT structure(nodeto
                             be added)      
  RETURNS         :  CLI_CMD_ADDED_PARTIALL or CLI_CMD_ADDED_COMPLETE
  NOTES           :  1. Get the direction of search
                     2. Search for the match and find the position to insert
                     3. Insert the node and reset the pointers
*****************************************************************************/
uns32 cli_append_node(CLI_CMD_ELEMENT *root, 
                     CLI_CMD_ELEMENT  *cnode,
                     CLI_CMD_ELEMENT  **i_node)
{         
   NCS_BOOL         done = FALSE;   
   CLI_CMD_ELEMENT *temp = 0;   
   
   if(m_NCS_OS_STRCMP((*i_node)->tokName, cnode->tokName) < 0) {
      while(m_NCS_OS_STRCMP((*i_node)->tokName, cnode->tokName) < 0) {   
         done = TRUE;

         if(cnode == root) {
            done = FALSE;
            break;
         }         
         cnode = cnode->node_ptr.pParent;            
      }

      if(TRUE == done) {  
         temp = cnode->node_ptr.pSibling;
         cnode->node_ptr.pSibling = *i_node;
         (*i_node)->node_ptr.pParent = cnode;            
         (*i_node)->node_ptr.pSibling = temp;
         temp->node_ptr.pParent = *i_node;
      }       
      else {   
         (*i_node)->node_ptr.pSibling = cnode; 
         cnode->node_ptr.pParent = *i_node;                                    
         return CLI_CMD_ADDED_PARTIALL;
      }
   }
   else if(m_NCS_OS_STRCMP((*i_node)->tokName, cnode->tokName) > 0) {
      cnode->node_ptr.pSibling = *i_node;    
      (*i_node)->node_ptr.pParent = cnode;
   }   
   return CLI_CMD_ADDED_COMPLETE;
}

/*****************************************************************************
  PROCEDURE NAME  :  cli_scan_token
  DESCRIPTION     :  The function scans the token before adding into the 
                     command tree.
  ARGUMENTS       :  pCli - Pointer of the control block
                     node - Pointer to a CLI_CMD_ELEMENT information block.   
                     rel  - Relationship of the node with the prevoius node.
  RETURNS         :  CLI_CMD_ADDED_PARTIALL or CLI_CMD_ADDED_COMPLETE
  NOTES           :                        
*****************************************************************************/
void cli_scan_token(CLI_CB *pCli, CLI_CMD_ELEMENT *node, CLI_TOKEN_RELATION rel)
{
   if(NCSCLI_BRACE_END == node->tokType) {
      cli_reset_dummy_marker(pCli);
      cli_free_cmd_element(&node);
   }
   else cli_add_cmd_node(pCli, node, rel);
}

/*****************************************************************************
  PROCEDURE NAME  :  cli_set_cef
  DESCRIPTION     :  The function sets the CEF with the last token in the main
                     level of the tree for the command
  ARGUMENTS       :  pCli - Pointer of the control block
  RETURNS         :  none
  NOTES           :
*****************************************************************************/
void cli_set_cef(CLI_CB *pCli)
{
   pCli->ctree_cb.lvlMrkr->bindery = pCli->ctree_cb.bindery;
   pCli->ctree_cb.lvlMrkr->cmd_exec_func = pCli->ctree_cb.execFunc;
   pCli->ctree_cb.lvlMrkr->cmd_access_level = pCli->ctree_cb.cmd_access_level; /*For 59359 */
}

/*****************************************************************************
  PROCEDURE NAME  :  cli_clean_ctree
  DESCRIPTION     :  The function cleans/destroys the complete command tree
                     and its related data.                    
  ARGUMENTS       :  pCli - Pointer of the control block       
  RETURNS         :  none
  NOTES           :                        
*****************************************************************************/
void cli_clean_ctree(CLI_CB *cb)
{
   CLI_CMD_NODE *root = cb->ctree_cb.rootMrkr;

   /* Check if root node is valid */
   if(root) {
      /* Clean all the mode configured */
      cli_clean_cmd_node(root);   
   }
}

/*****************************************************************************
  PROCEDURE NAME  :  cli_clean_cmd_node
  DESCRIPTION     :  The function recursivly cleans/destroys the command mode
                     and its related data.                    
  ARGUMENTS       :  node - root node   
  RETURNS         :  none
  NOTES           :                        
*****************************************************************************/
void cli_clean_cmd_node(CLI_CMD_NODE *node)
{
   CLI_CMD_NODE *ptr = node, *curr = 0;

   while(ptr)
   {
      /* Traverse child node if exist */
      if(ptr->pChild) {
         ptr = ptr->pChild;
         continue;
      }
      else if(ptr->pSibling) {
         ptr = ptr->pSibling;
         continue;
      }
      else if(ptr->pCmdl) {
         /* First clean all command elements under that mode */
         cli_clean_cmd_element(ptr->pCmdl);         
         goto free_mode;     
      }   
      
free_mode:
      /* Free the mode */
      curr = ptr;
      ptr = ptr->pParent;
      
      if(!ptr) {
         m_MMGR_FREE_CLI_CMD_NODE(curr);
         return;
      }

      if(ptr->pChild == curr) ptr->pChild = 0;
      else if(ptr->pSibling == curr) ptr->pSibling = 0;      

      if(curr == node) {
         m_MMGR_FREE_CLI_CMD_NODE(curr);
         break;
      }
      m_MMGR_FREE_CLI_CMD_NODE(curr);
   }
}

/*****************************************************************************
  PROCEDURE NAME  :  cli_clean_cmd_element
  DESCRIPTION     :  The function recursivly cleans/destroys the command 
                     element and its related data.                    
  ARGUMENTS       :  node - start of command element under certain mode   
  RETURNS         :  none
  NOTES           :                        
*****************************************************************************/
void cli_clean_cmd_element(CLI_CMD_ELEMENT *node)
{   
   CLI_CMD_ELEMENT    *ptr = node, *curr = 0;         
   CLI_BINDERY_LIST   *bdryl = 0;
   while(ptr) {
      
      /* Loop through each of the node */
      if(ptr->node_ptr.pChild) ptr = ptr->node_ptr.pChild;
      else if(ptr->node_ptr.pDataPtr) ptr = ptr->node_ptr.pDataPtr;
      else if(ptr->node_ptr.pSibling) ptr = ptr->node_ptr.pSibling;      
      else {
         
         /* This is the leaf node, need to delete this node */
         curr = ptr;
         ptr = ptr->node_ptr.pParent;         
         
         if(ptr->node_ptr.pChild == curr) ptr->node_ptr.pChild = 0;
         else if(ptr->node_ptr.pDataPtr == curr) ptr->node_ptr.pDataPtr = 0;
         else if(ptr->node_ptr.pSibling == curr) ptr->node_ptr.pSibling = 0;                    
         
         /* Push the bindery structre in list for freeing */
         if(curr->bindery) cli_push_bindery(&bdryl, curr->bindery);
        
        /* Fix for the bug IR00061049 */
         /*cli_free_cmd_element(&curr); */       
         if(curr != ptr) cli_free_cmd_element(&curr);        

         if(!ptr->node_ptr.pChild && !ptr->node_ptr.pDataPtr && 
            !ptr->node_ptr.pSibling) {  
            
            /* If the parent pointe and its woen pointer is same then 
             * it means we ahave free dall the command and need to return
            */
            if(ptr == ptr->node_ptr.pParent) {
               cli_free_cmd_element(&ptr);
               goto done;         
            }
         }
      }
   }  

done:     
   cli_free_bindery(&bdryl);
   return;
}

void cli_push_bindery(CLI_BINDERY_LIST **list, NCSCLI_BINDERY *node)
{
   CLI_BINDERY_LIST *ptr = 0;   

   for(ptr=*list; ptr; ptr=ptr->next) {
      if(ptr->bindery == node) return;
   }
   
   ptr = (CLI_BINDERY_LIST *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(CLI_BINDERY_LIST));
   if(!ptr) return;
   m_NCS_OS_MEMSET(ptr, 0, sizeof(CLI_BINDERY_LIST));   
   ptr->bindery = node;

   if(*list) ptr->next = *list;
   *list = ptr;
   return;
}

void cli_free_bindery(CLI_BINDERY_LIST **list)
{
   CLI_BINDERY_LIST *ptr = 0;

   while(*list) {
      ptr = *list;
      (*list) = (*list)->next;
      
      /* Free up the bindery information */        
      m_MMGR_FREE_NCSCLI_BINDERY(ptr->bindery);               
      m_MMGR_FREE_CLI_DEFAULT_VAL(ptr);
   }
}

void cli_free_cmd_element(CLI_CMD_ELEMENT **node)
{
   if((*node)->defVal) m_MMGR_FREE_CLI_DEFAULT_VAL((*node)->defVal);

   if((*node)->range) {
      if((*node)->range->lLimit)
         m_MMGR_FREE_CLI_DEFAULT_VAL((*node)->range->lLimit);
      if((*node)->range->uLimit)
         m_MMGR_FREE_CLI_DEFAULT_VAL((*node)->range->uLimit);
      m_MMGR_FREE_CLI_RANGE((*node)->range);
   }

   if((*node)->tokName) m_MMGR_FREE_CLI_DEFAULT_VAL((*node)->tokName);
   if((*node)->helpStr) m_MMGR_FREE_CLI_DEFAULT_VAL((*node)->helpStr);
   if((*node)->nodePath) m_MMGR_FREE_CLI_DEFAULT_VAL((*node)->nodePath);
   m_MMGR_FREE_CLI_CMD_ELEMENT((*node));
}

#else
extern int dummy;
#endif /* (if NCS_CLI == 1) */
