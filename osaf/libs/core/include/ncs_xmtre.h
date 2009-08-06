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

  DESCRIPTION:

NCS_XMTREE.H

This is an include file which forms the basis of NetPlane's new 
'find best match' library.

The basic premise is that this library allows an appication to maintain
a set of PREFIXES, and to perform best-match lookups.

There are two 'housekeeping' functions, ncs_xmtree_init() and
ncs_xmtree_destroy() which are intended to be called ONCE at startup
and shutdown.  Certain configuration about each tree is presented in the
'create' API such as total key size in octets
(4 for IPv4 addresses, 16 for IPv6) and max key bit length (32 for IPv4).

If this is used for IP address lookup, there is an assumption that there
would be different 'trees' created for IPv4 and IPv6 address/masks.

Also, the keys presented as an uns8 array. If this application is
used to match IPv4 addresses, frequently IPv4 Addresses are stored in an
uns32.  It is okay to cast a pointer to the key as an uns8*, but
ONLY IF THE UNS32 IS ORDERED IN 'NETWORK ORDER'.  This is the responsibility
of the calling application.  Otherwise 'endian' problems will ensue.

Associated with (usually inserted within) an application's structure
will be a NCS_MNODE structure, which is the only part (of the
application's structure) which this library uses.  This is similar to 
the current patricia tree.  Since the 'lookup' produces a pointer to
the NCS_MNODE structure, a certain amount of pointer arithmetic
may be necessary in the application code to compute the pointer to
the enclosing structure (if the NCS_MNODE is not the 1st element
in the application structure.)

This library DOES NOT support duplicate entries with the exact same
prefix & length.  THIS MEANS THAT SUPPORT FOR EQUAL ROUTES MUST BE KNOWN
ONLY BY THE APPLICATION AND NOT BY THIS LIBRARY.  
This could be implemented as a linked-list
of structures 'hanging off' of the structure containing the NCS_MATCHNODE.

Functions are included in this library to add/find a prefix to a tree, delete
one from the tree, and to do best-match (i.e. longest match) lookup.  

Also supported is a get_next_best match.  This can be used by the application 
to get LESS SPECIFIC matches.  For example, when the database contains
0.0.0.0/0, 10.3.0.0/16, 10.3.1.0/24, and a 'best' lookup on IP address
10.3.1.200 returns the MNODE for 10.3.1.0/24, then this MNODE
can be fed into the 'get_next_best' API, which will return the 10.3.0.0/16
node.

Support for 'get-next' is also possible, although
this takes more memory, so the 'get-next' capability is specified as
a trait of the tree when the tree is created.  

'Getnext' (when implemented and enabled) returns all shorter prefixes
before any longer ones.  Thus using IPv4 Addresses as an example,
  0.0.0.0/0 < 128.0.0.0/8 < 10.3.3.0/24 < 355.255.255.255/32

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef _NCS_XMTREE_H_
#define _NCS_XMTREE_H_

#ifdef  __cplusplus
extern "C" {
#endif

/******************************************************************************
  
************************************************************************/

/********* Structures **************************************/
#if (NCS_MTREE_PAT_STYLE_SUPPORTED != 0)
#include "ncs_mtree.h"

/* Macros to be used to access by the user while using xmtree*/
/* The below mensioned macros are portable stuff, so this might be 
 * moved to some other Os specific portable file later
 */

/* xmtree initializtion macro */
#define m_NCS_XMTREE_INIT(tree,conf) ncs_xmtree_init(tree,conf)
/* xmtree destory.., At present this is a dummy function*/
#define m_NCS_XMTREE_DESTROY(tree) ncs_xmtree_destroy(tree)
/* Add the node to the xmtree*/
#define m_NCS_XMTREE_ADD(tree,node) ncs_xmtree_add(tree,node)
/* Find the given node in the xmtree, if not add to the xmtree*/
#define m_NCS_XMTREE_FIND_ADD(tree,pKey,keySize,callBkFun,Cookie) \
                  ncs_xmtree_find_add(tree,pKey,keySize,callBkFun,Cookie)
/* delete a node from the xmtree*/
#define m_NCS_XMTREE_DEL(tree,node) ncs_xmtree_del(tree,node)
/* find next best node from the xmtree*/
#define m_NCS_XMTREE_FIND_NEXTBEST(tree,betNode) ncs_xmtree_find_nextbest(tree,betNode)
/* find the best match in the xmtree*/
#define m_NCS_XMTREE_FIND_BEST(tree,key,keySize,mSpcBit) \
                          ncs_xmtree_find_best(tree,key,keySize,mSpcBit)
/* find's the next available node, for the first time the key will be NULL*/
#define m_NCS_XMTREE_NEXT(tree,key,keySize) ncs_xmtree_next(tree,key,keySize)
/* find the next available node (differs after the mask) */
#define m_NCS_XMTREE_GET_NEXT_WITH_MASK(tree, key, maskLen, bitsize) \
                     xmtree_get_next_key(tree, key, maskLen, bitsize)

/* max 32 byte key length can be used, it can be extended. */
#define NCS_XMTREE_MAX_KEY_BYTE (32)
#define m_XMTREE_GREATER_VALUE (1)
#define m_XMTREE_LESSER_VALUE  (2)

	unsigned int
	 xmtree_init(NCS_MTREE *const pTree);

	void
	 xmtree_dest(NCS_MTREE *const pTree);

	NCS_MTREE_NODE *xmtree_findadd(NCS_MTREE *const pTree,
				       /*changed pKey to testKey for testing */
				       const uns32 *const pTargetKey,
				       const short KeyBitSize,
				       NCS_MTREE_NODE *(*pFunc) (const void *pTargetKey,
								 short KeyBitSize, void *Cookie), void *const Cookie);

	void
	 xmtree_del(NCS_MTREE *const pTree, NCS_MTREE_NODE *const pNode);

	NCS_MTREE_NODE *xmtree_best(NCS_MTREE *const pTree,
				    const uns32 *const tempKey, short KeyBitSize, int *const pMoreSpecificExists);

	NCS_MTREE_NODE *xmtree_nextbest(NCS_MTREE *const pTree, const NCS_MTREE_NODE *const pBetterNode);

	NCS_MTREE_NODE *xmtree_next(NCS_MTREE *const pTree, const uns32 *const pKey, short KeyBitSize);

	NCS_MTREE_NODE *xmtree_get_next_key(NCS_MTREE *const pTree,
					    const uns32 *const pKey, uns32 maskLen, short KeyBitSize);

	EXTERN_C LEAPDLL_API unsigned int
	 ncs_xmtree_init(NCS_MTREE *const pTree, const NCS_MTREE_CFG *const pCfg);

	EXTERN_C LEAPDLL_API void
	 ncs_xmtree_destroy(NCS_MTREE *const pTree);

	EXTERN_C LEAPDLL_API unsigned int
	 ncs_xmtree_add(NCS_MTREE *const pTree, NCS_MTREE_NODE *const pNode);

	EXTERN_C LEAPDLL_API NCS_MTREE_NODE *ncs_xmtree_find_add(NCS_MTREE *const pTree,
								 const void *const pKey,
								 const short KeyBitSize,
								 NCS_MTREE_NODE *(*pFunc) (const void *pKey,
											   short KeyBitSize,
											   void *Cookie),
								 void *const Cookie);

	EXTERN_C LEAPDLL_API void
	 ncs_xmtree_del(NCS_MTREE *const pTree, NCS_MTREE_NODE *const pNode);

	EXTERN_C LEAPDLL_API NCS_MTREE_NODE *ncs_xmtree_find_nextbest(NCS_MTREE *const pTree,
								      const NCS_MTREE_NODE *const pBetterNode);

	EXTERN_C LEAPDLL_API NCS_MTREE_NODE *ncs_xmtree_find_best(NCS_MTREE *const pTree,
								  const void *const pKey,
								  uns16 KeyBitSize, int *const pMoreSpecificExists);

	EXTERN_C LEAPDLL_API NCS_MTREE_NODE *ncs_xmtree_next(NCS_MTREE *const pTree,
							     const void *const pKey, short KeyBitSize);
#endif   /*NCS_MTREE_PAT_STYLE_SUPPORTED */

#ifdef  __cplusplus
}
#endif

#endif   /*_NCS_XMTREE_H_*/
