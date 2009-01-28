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

 MODULE NAME:  CLICTREE.H

..............................................................................

  DESCRIPTION:

  Header file for Command Tree and its utility functions.

******************************************************************************
*/

#ifndef CLICTREE_H
#define CLICTREE_H

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                    Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
typedef struct cli_bindery_list {
   NCSCLI_BINDERY  *bindery;
   struct cli_bindery_list *next;
} CLI_BINDERY_LIST;


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        COMMAND TREE BUILDING FUNCTIONS

This section defines the functions that are used by the command tree for buil-
-ding the tree with the token passed by the Parser and the utily functions that
are used for manpulation and travering the tree.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
EXTERN_C void cli_add_cmd_node(CLI_CB *, CLI_CMD_ELEMENT *, CLI_TOKEN_RELATION);
EXTERN_C uns32 cli_parse_node(CLI_CB *, NCSCLI_CMD_LIST *, NCS_BOOL);
EXTERN_C void cli_set_cmd_to_parse(CLI_CB *, int8 *, CLI_BUFFER_TYPE);
EXTERN_C void cli_find_add_node(CLI_CB *, int8 *, NCS_BOOL);
EXTERN_C void cli_reset_dummy_marker(CLI_CB *);
EXTERN_C void cli_set_dummy_marker(CLI_CB *);
EXTERN_C void cli_reset_all_marker(CLI_CB *);
EXTERN_C NCS_BOOL cli_cmd_ispresent(CLI_CMD_ELEMENT **, int8 *);
EXTERN_C uns32 cli_append_node(CLI_CMD_ELEMENT *, CLI_CMD_ELEMENT *, CLI_CMD_ELEMENT **);
EXTERN_C void cli_scan_token(CLI_CB *, CLI_CMD_ELEMENT *, CLI_TOKEN_RELATION);
EXTERN_C void cli_set_cef(CLI_CB *);
EXTERN_C void cli_clean_ctree(CLI_CB *);
EXTERN_C void cli_clean_cmd_element(CLI_CMD_ELEMENT *);
EXTERN_C void cli_free_cmd_element(CLI_CMD_ELEMENT **);
EXTERN_C void cli_clean_cmd_node(CLI_CMD_NODE *);
#endif
