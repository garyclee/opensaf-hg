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

NCS_MTREE.H

This is an include file which forms the basis of NetPlane's new 
'find best match' library.

The basic premise is that this library allows an appication to maintain
a set of PREFIXES, and to perform best-match lookups.

There are two 'housekeeping' functions, ncs_mtree_init() and
ncs_mtree_destroy() which are intended to be called ONCE at startup
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
 
#ifndef _NCS_MTREE_H_
#define _NCS_MTREE_H_

#include "ncsgl_defs.h"

#ifdef  __cplusplus
extern "C" {
#endif

/******************************************************************************
  
************************************************************************/


/********* Structures **************************************/


/*** NCS_MTREE_NODE:  defines a node in the best-match tree */
typedef struct ncs_mtree_node
{
   /* The following fields are set by user-code before a node is
    * added to the tree.  These fields must not be modified by user-code
    * while the MATCHNODE is part of the MATCHTREE.
    */
   const void* pKey;               /* Set by caller before 'add':
                * Must point to area of size 
                * 'KeyOctetSize'
                */
   short       KeyBitSize;         /* Set by caller before 'add' */

   /* The following fields are for INTERNAL USE by the library, and
    * must not be accessed by user-code.
    */
   union
   {
      struct  /* for style 'patricia_uns32' */
      {
         short            Bit;

         struct ncs_mtree_node *pLeft;  /* during search, follow this if bit is '0' */
         struct ncs_mtree_node *pRight; /* during search, follow this if bit is '1' */
         struct ncs_mtree_node *pDownToMe;    /* node with 'Bit' < ours which points to us via Left or Right */
         struct ncs_mtree_node *pLessSpecific;  /* Ptr to node with equal key which supplies less-specific match */
         struct ncs_mtree_node *pMoreSpecific;  /* Ptr to node with equal key which supplies more-specific match */
      } p32;
#if (NCS_MTREE_PAT_STYLE_SUPPORTED != 0) 
      struct  /* for style 'patricia' */
      {
         short            Bit;

         struct ncs_mtree_node *pLeft;  /* during search, follow this if bit is '0' */
         struct ncs_mtree_node *pRight; /* during search, follow this if bit is '1' */
         struct ncs_mtree_node *pDownToMe;    /* node with 'Bit' < ours which points to us via Left or Right */
         struct ncs_mtree_node *pLessSpecific;  /* Ptr to node with equal key which supplies less-specific match */
         struct ncs_mtree_node *pMoreSpecific;  /* Ptr to node with equal key which supplies more-specific match */
      } Pat;
#endif
   } Style;
} NCS_MTREE_NODE;

#define NCS_MTREE_NODE_NULL ((NCS_MTREE_NODE*) NULL)

/* NCS_MTREE_STYLE:  In future, we may have different styles. */
typedef enum ncs_mtree_style
{
   NCS_MTREE_STYLE_PATRICIA_UNS32,
#if (NCS_MTREE_PAT_STYLE_SUPPORTED != 0)   
   NCS_MTREE_STYLE_PATRICIA,   /* Modified patricia tree */
#endif /* (NCS_MTREE_PAT_STYLE_SUPPORTED != 0) */

   NCS_MTREE_STYLE_MAX         /* Must be last. */
} NCS_MTREE_STYLE;


/* NCS_MATCHTREE_CFG:  Used when creating a MATCHTREE. */

typedef struct ncs_mtree_cfg
{
   uns16          KeyOctetSize;       /* All keys must occupy this length */
   short          MaxKeyBitSize;      /* Max #bits in key (max_bits <= 8*octet_size) */
   NCS_MTREE_STYLE Style;
} NCS_MTREE_CFG;



typedef struct ncs_mtree
{
   NCS_MTREE_CFG           Cfg;
   union
   {
      struct 
      {
         NCS_MTREE_NODE          Root;  /* A tree always has a root node. */
         NCS_MTREE_NODE          *pRoot; 
      } p32;
#if (NCS_MTREE_PAT_STYLE_SUPPORTED != 0)
      struct 
      {
         NCS_MTREE_NODE          Root;  /* A tree always has a root node. */
         NCS_MTREE_NODE          *pRoot;
      } Pat;
#endif /* (NCS_MTREE_PAT_STYLE_SUPPORTED != 0) */
   }Style;
} NCS_MTREE;

#define NCS_MTREE_NULL ((NCS_MTREE*) NULL)

#if (NCS_MTREE_PAT_STYLE_SUPPORTED)
#define NCS_MTREE_MAX_PAT_KEY_SIZE 64
#endif

/************************************************************
 * Function:  ncs_mtree_init()
 * Purpose:   Initialized a 'tree' for storing best-match lookup.
 *            Called once at startup for each tree.
 *            If used for IP Address match, we would expect one
 *            NCS_MTREE to be created for IPv4 addresses and
 *            another for IPv6 addresses.
 *
 * Arguments: NCS_MTREE*:      The tree to init.
 *            NCS_MTREE_CFG*:  Properties of the tree.
 *
 * Returns:   NCSCC_RC_SUCCESS if success.
 ***************************************************************/
EXTERN_C LEAPDLL_API unsigned int
ncs_mtree_init(NCS_MTREE                 *const pTree,
               const NCS_MTREE_CFG       *const pCfg); 


/***************************************************************
 * Function:  ncs_mtree_destroy()
 * Purpose:   Frees a 'tree' and all subsequent structures.
 *            If the tree was not empty, it will remove all nodes.
 *
 * Arguments: struct ncs_mtree*:  a value returned from ncs_mtree_create()
 * 
 * Returns:   nothing.
 *****************************************************************/

EXTERN_C LEAPDLL_API void
ncs_mtree_destroy(NCS_MTREE *const pTree);


/*****************************************************************
 * Function:  ncs_mtree_add()
 * Purpose:   adds a node with a specified prefix to the best-match tree.
 *
 * Arguments: NCS_MTREE*:  tree in which to insert
 *            NCS_MNODE*:  with pKey and KeyBitSize prefilled.
 *
 * Returns:  NCSCC_RC_SUCCESS:  all went okay
 *           anything else:    node not inserted
 ******************************************************************/
EXTERN_C LEAPDLL_API  unsigned int
ncs_mtree_add(NCS_MTREE      *const pTree,
              NCS_MTREE_NODE *const pNode);

/******************************************************************
 * Function:  ncs_mtree_del()
 * Purpose:   removes a node from the best-match tree.
 *
 * Arguments:NCS_MTREE*:  tree from which to remove.
 *           NCS_MNODE*:  as returned from ncs_mtree_add()
 *
 * Returns:  nothing.
 ******************************************************************/
EXTERN_C LEAPDLL_API void
ncs_mtree_del(NCS_MTREE      *const pTree,
              NCS_MTREE_NODE *const pNode);

/******************************************************************
 * Function:  ncs_mtree_find_best()
 * Purpose:   Finds best-match (i.e. longest) matching prefix
 *
 * Arguments: NCS_MTREE*:  Tree to search
 *            void    :  pointer to key (size=KeyOctetSize) of target
 *            uns16   :  bit-size of target.  No. of bits of significance
 *                       of the 'target' field.
 *            NCS_BOOL* : Pointer to boolean which will be set by
 *                       the function, indicating if there are any 
 *                       'more specific' nodes in the tree than the
 *                       given target.  We define a more-specific node
 *                       as one which matches the prefix defined by
 *                       the (key/bitsize) target, but which has a larger
 *                       bit size itself.
 *                       Will be set to TRUE if there exists at least
 *                       one more-specific node, or FALSE if not.
 *
 * Returns:   NCS_MNODE*:  NULL means 'no match'
 *******************************************************************/
EXTERN_C LEAPDLL_API NCS_MTREE_NODE *
ncs_mtree_find_best(NCS_MTREE * const pTree,
                    const void *const       pKey,
                    uns16      KeyBitSize,
                    int  *const   pMoreSpecificExists);

/*******************************************************************
 * Function:  ncs_mtree_find_nextbest
 * Purpose:   Given a MNODE, find the next-best matching prefix
 *            in the tree.  Used to find alternate matching entries.
 * 
 * Arguments: NCS_MTREE*:  tree to search
 *            NCS_MNODE*:  an entry in the tree
 *
 * Returns:   NCS_MNODE*:  The next most specific matching prefix
 *                        (with a shorter bit-length key),
 *                        NULL if none is found.
 *******************************************************************/
EXTERN_C LEAPDLL_API NCS_MTREE_NODE *
ncs_mtree_find_nextbest(NCS_MTREE *const pTree,
                       const NCS_MTREE_NODE *const pBetterNode);

/*******************************************************************
 * Function:  ncs_mtree_find_add()
 * Purpose:   Finds an exact match.  If not found, makes a
 *            'callback' to the invoker to allow creation of a 
 *            NCS_MNODE.  If callback returns non-NULL, then the
 *            new node will be added.
 *
 * Arguments:  NCS_MTREE*:      key to search
 *             void*:          pointer to a key (size= KeyOctetSize)
 *             uns16:          prefix size in bits
 *             func*:          Pointer to a function which will be invoked
 *                             if exact match not found.  Function can
 *                             allocate a structure and create a NCS_MNODE
 *                             to insert.
 *             void*           cookie to pass to function.
 *             
 *
 * Returns:    NCS_MNODE*:  NULL means 'not found'.
 *******************************************************************/
EXTERN_C LEAPDLL_API NCS_MTREE_NODE *
ncs_mtree_find_add(NCS_MTREE         *const pMTree,
                  const void *const       pKey,
                  const short             KeyBitSize,
                  NCS_MTREE_NODE *         (*pFunc)(const void* pKey, 
                                                   short KeyBitSize,
                                                   void *Cookie),
                  void             *const Cookie);

/*********************************************************************
 * Function:  ncs_mtree_next()
 * Purpose:   Finds 'next' key.
 *            Only valid if tree was created with 'getnext' capability.
 *            This function will return 'FAILURE' if the MTREE was
 *            created without the 'getnext' capability.
 *
 * Arguments: NCS_MTREE*:    the tree
 *            void*:        'Previous' key  (NULL means get 1st)
 *            uns16:          previous key's bitsize.
 *
 * Returns:   NCS_MNODE*:  NULL means end-of-list or error.
 ***********************************************************************/
EXTERN_C LEAPDLL_API NCS_MTREE_NODE *
ncs_mtree_next(NCS_MTREE               *const pTree,
              const void *const       pKey,
              short                   KeyBitSize);

#ifdef  __cplusplus
}
#endif

#endif /* _NCS_MTREE_H_ */

