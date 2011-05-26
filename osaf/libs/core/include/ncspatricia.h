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

 @@@@@     @@     @@@@@  @@@@@      @     @@@@      @      @@            @    @
 @    @   @  @      @    @    @     @    @    @     @     @  @           @    @
 @    @  @    @     @    @    @     @    @          @    @    @          @@@@@@
 @@@@@   @@@@@@     @    @@@@@      @    @          @    @@@@@@   @@@    @    @
 @       @    @     @    @   @      @    @    @     @    @    @   @@@    @    @
 @       @    @     @    @    @     @     @@@@      @    @    @   @@@    @    @

..............................................................................

  DESCRIPTION:

  This module contains declarations pertaining to implementation of 
  patricia tree search/add/delete functions.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef _NCSPATRICIA_H_
#define _NCSPATRICIA_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include "ncsgl_defs.h"

	typedef struct ncs_patricia_params {
		int key_size;	/* 1..NCS_PATRICIA_MAX_KEY_SIZE - in OCTETS */
		int info_size;	/* NOT USED!  Present for backward-compatibility only! */
		int actual_key_size;	/* NOT USED!  Present for backward-compatibility only! */
		int node_size;	/* NOT USED!  Present for backward compatibitity only! */
	} NCS_PATRICIA_PARAMS;

#define NCS_PATRICIA_MAX_KEY_SIZE 600	/* # octets */

	typedef struct ncs_patricia_node {
		int bit;	/* must be signed type (bits start at -1) */
		struct ncs_patricia_node *left;
		struct ncs_patricia_node *right;
		uns8 *key_info;
	} NCS_PATRICIA_NODE;

#define NCS_PATRICIA_NODE_NULL ((NCS_PATRICIA_NODE *)0)

	typedef uns8 NCS_PATRICIA_LEXICAL_STACK;	/* ancient history... */

	typedef struct ncs_patricia_tree {
		NCS_PATRICIA_NODE root_node;	/* A tree always has a root node. */
		NCS_PATRICIA_PARAMS params;
		unsigned int n_nodes;
	} NCS_PATRICIA_TREE;

	unsigned int ncs_patricia_tree_init(NCS_PATRICIA_TREE *const pTree,
								 const NCS_PATRICIA_PARAMS *const pParams);
	unsigned int ncs_patricia_tree_destroy(NCS_PATRICIA_TREE *const pTree);
	void ncs_patricia_tree_clear(NCS_PATRICIA_TREE *const pTree);
	unsigned int ncs_patricia_tree_add(NCS_PATRICIA_TREE *const pTree,
								NCS_PATRICIA_NODE *const pNode);
	unsigned int ncs_patricia_tree_del(NCS_PATRICIA_TREE *const pTree,
								NCS_PATRICIA_NODE *const pNode);
	NCS_PATRICIA_NODE *ncs_patricia_tree_get(const NCS_PATRICIA_TREE *const pTree,
								      const uns8 *const pKey);
	NCS_PATRICIA_NODE *ncs_patricia_tree_get_best(const NCS_PATRICIA_TREE *const pTree, const uns8 *const pKey, uns16 KeyLen);	/* Length of key (in BITS) */
	NCS_PATRICIA_NODE *ncs_patricia_tree_getnext(NCS_PATRICIA_TREE *const pTree, const uns8 *const pKey);	/* NULL means get 1st */

	int ncs_patricia_tree_size(const NCS_PATRICIA_TREE *const pTree);

#ifdef  __cplusplus
}
#endif

#endif
