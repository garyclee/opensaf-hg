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

..............................................................................

  DESCRIPTION:

  This module contains declarations pertaining to implementation of 
  patricia tree search/add/delete functions.

******************************************************************************
*/

/* Get compile time options...
 */
#include "ncs_opt.h"

/** Global Declarations...
 **/
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncssysf_def.h"
#include "ncs_mtree.h"
#include "ncs_xmtre.h"

/*****************************************************************************
 * PRIVATE (static) FUNCTIONS
 *****************************************************************************/

#if (USE_LITTLE_ENDIAN == 0)
static const uns32 P32Masks[33] = {
	0x00000000,
	0x80000000,
	0xc0000000,
	0xe0000000,
	0xf0000000,
	0xf8000000,
	0xfc000000,
	0xfe000000,
	0xff000000,
	0xff800000,
	0xffc00000,
	0xffe00000,
	0xfff00000,
	0xfff80000,
	0xfffc0000,
	0xfffe0000,
	0xffff0000,
	0xffff8000,
	0xffffc000,
	0xffffe000,
	0xfffff000,
	0xfffff800,
	0xfffffc00,
	0xfffffe00,
	0xffffff00,
	0xffffff80,
	0xffffffc0,
	0xffffffe0,
	0xfffffff0,
	0xfffffff8,
	0xfffffffc,
	0xfffffffe,
	0xffffffff,
};

static const uns32 P32Bits[33] = {
	0x00000000,
	0x80000000,
	0x40000000,
	0x20000000,
	0x10000000,
	0x08000000,
	0x04000000,
	0x02000000,
	0x01000000,
	0x00800000,
	0x00400000,
	0x00200000,
	0x00100000,
	0x00080000,
	0x00040000,
	0x00020000,
	0x00010000,
	0x00008000,
	0x00004000,
	0x00002000,
	0x00001000,
	0x00000800,
	0x00000400,
	0x00000200,
	0x00000100,
	0x00000080,
	0x00000040,
	0x00000020,
	0x00000010,
	0x00000008,
	0x00000004,
	0x00000002,
	0x00000001,
};
#else				/* if (USE_LITTLE_ENDIAN != 0) */
static const uns32 P32Masks[33] = {
	0x00000000,
	0x00000080,
	0x000000c0,
	0x000000e0,
	0x000000f0,
	0x000000f8,
	0x000000fc,
	0x000000fe,
	0x000000ff,
	0x000080ff,
	0x0000c0ff,
	0x0000e0ff,
	0x0000f0ff,
	0x0000f8ff,
	0x0000fcff,
	0x0000feff,
	0x0000ffff,
	0x0080ffff,
	0x00c0ffff,
	0x00e0ffff,
	0x00f0ffff,
	0x00f8ffff,
	0x00fcffff,
	0x00feffff,
	0x00ffffff,
	0x80ffffff,
	0xc0ffffff,
	0xe0ffffff,
	0xf0ffffff,
	0xf8ffffff,
	0xfcffffff,
	0xfeffffff,
	0xffffffff,
};

static const uns32 P32Bits[33] = {
	0x00000000,
	0x00000080,
	0x00000040,
	0x00000020,
	0x00000010,
	0x00000008,
	0x00000004,
	0x00000002,
	0x00000001,
	0x00008000,
	0x00004000,
	0x00002000,
	0x00001000,
	0x00000800,
	0x00000400,
	0x00000200,
	0x00000100,
	0x00800000,
	0x00400000,
	0x00200000,
	0x00100000,
	0x00080000,
	0x00040000,
	0x00020000,
	0x00010000,
	0x80000000,
	0x40000000,
	0x20000000,
	0x10000000,
	0x08000000,
	0x04000000,
	0x02000000,
	0x01000000,
};
#endif   /* USE_LITTLE_ENDIAN */

#define m_MTREE_P32_BIT_IS_SET(k, b) (((k) & P32Bits[b+1]) != 0)
#define m_MTREE_P32_KEY_CMP(k1, k2) ((*(const uns32*)(k1)) == (*(const uns32*)(k2)))
#define m_MTREE_P32_KEY_MASK_CMP(k1, k2, kl) (((kl) >= 0) && (((k1 ^ k2) & P32Masks[kl]) == 0))
/*****************************************    Code ************************************/

/************************************************************
 * Function:  mtree_p32_init
 * Purpose:   Initialize a patricia tree style mtree.
 ***************************************************************/
static unsigned int mtree_p32_init(NCS_MTREE *const pTree)
{
	static const uns32 AlwaysZero = 0;

	/* Do additional tests. */
	if (pTree->Cfg.KeyOctetSize != sizeof(uns32)) {
		return (unsigned int)m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

/* Initialize the root node, which is actually part of the tree structure. */
	pTree->Style.p32.Root.pKey = &AlwaysZero;
	pTree->Style.p32.Root.KeyBitSize = -1;	/* designates the root */
	pTree->Style.p32.Root.Style.p32.Bit = -1;
	pTree->Style.p32.Root.Style.p32.pLeft = pTree->Style.p32.Root.Style.p32.pRight = &pTree->Style.p32.Root;
	pTree->Style.p32.Root.Style.p32.pDownToMe =
	    pTree->Style.p32.Root.Style.p32.pMoreSpecific =
	    pTree->Style.p32.Root.Style.p32.pLessSpecific = NCS_MTREE_NODE_NULL;

	pTree->Style.p32.pRoot = &pTree->Style.p32.Root;

	return NCSCC_RC_SUCCESS;
}

/***************************************************************
 * Function:  mtree_p32_dest()
 * Purpose:   Frees a 'tree' and all subsequent structures for patricia style
 *****************************************************************/

static void mtree_p32_dest(NCS_MTREE *const pTree)
{
}

#if (NCS_XMTREE_WRAPPERS == 0)
/*******************************************************************
 * Function:  mtree_p32_findadd()
 * Purpose:   Finds an exact match.  If not found, makes a
 *            'callback' to the invoker to allow creation of a 
 *            NCS_MNODE.  If callback returns non-NULL, then the
 *            new node will be added.
 *
 * Arguments:  NCS_MTREE*:      key to search
 *             uns32*:         pointer to a key (size= KeyOctetSize)
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
static NCS_MTREE_NODE *mtree_p32_findadd(NCS_MTREE *const pTree,
					 const uns32 *const pKey,
					 const short KeyBitSize,
					 NCS_MTREE_NODE *(*pFunc) (const void *pKey,
								   short KeyBitSize, void *Cookie), void *const Cookie)
{
	NCS_MTREE_NODE *pSrch;
	NCS_MTREE_NODE *pPrev;
	NCS_MTREE_NODE *pNode;

	pSrch = pTree->Style.p32.pRoot;

	do {
		pPrev = pSrch;

		if (m_MTREE_P32_BIT_IS_SET(*(uns32 *)pKey, pSrch->Style.p32.Bit)) {
			pSrch = pSrch->Style.p32.pRight;
		} else {
			pSrch = pSrch->Style.p32.pLeft;
		}
	} while (pSrch->Style.p32.Bit > pPrev->Style.p32.Bit);

	if (m_MTREE_P32_KEY_CMP(pKey, pSrch->pKey)) {
		/* We may have a duplicate key, or we may have an equal key with a different 
		 * bit length.  (for example, 10.3.0.0/16 and 10.3.0.0/24 would encounter this. )
		 */
		if (pSrch->KeyBitSize < KeyBitSize) {
			/* The tree contains 10.3.0.0/16 and we're adding 10.3.0.0/24.
			 * This is tricky.  We will add a new node at the head end of the
			 * list.  This one will become the REAL member of the Pat Tree!
			 */
			if ((pNode = (*pFunc) (pKey, KeyBitSize, Cookie)) != NCS_MTREE_NODE_NULL) {
				/* The caller has conveniently supplied us with a new one to add.
				 * The caller has filled its pKey and KeyBitSize fields.
				 * Initialize its patricia fields now!
				 */

				pNode->Style.p32.Bit = pSrch->Style.p32.Bit;
				pNode->Style.p32.pLeft = ((pSrch->Style.p32.pLeft == pSrch) ?
							  pNode : pSrch->Style.p32.pLeft);
				if (pNode->Style.p32.pLeft->Style.p32.pDownToMe == pSrch) {
					pNode->Style.p32.pLeft->Style.p32.pDownToMe = pNode;
				}

				pNode->Style.p32.pRight = ((pSrch->Style.p32.pRight == pSrch) ?
							   pNode : pSrch->Style.p32.pRight);
				if (pNode->Style.p32.pRight->Style.p32.pDownToMe == pSrch) {
					pNode->Style.p32.pRight->Style.p32.pDownToMe = pNode;
				}

				if (pPrev->Style.p32.pLeft == pSrch) {
					pPrev->Style.p32.pLeft = pNode;
				} else {
					pPrev->Style.p32.pRight = pNode;
				}

				pPrev = pSrch->Style.p32.pDownToMe;
				if (pPrev != NCS_MTREE_NODE_NULL) {
					if (pPrev->Style.p32.pLeft == pSrch) {
						pPrev->Style.p32.pLeft = pNode;
					} else {
						pPrev->Style.p32.pRight = pNode;
					}
				}

				pNode->Style.p32.pDownToMe = pPrev;

				pNode->Style.p32.pLessSpecific = pSrch;	/* It becomes 2nd in list. */
				pNode->Style.p32.pMoreSpecific = NCS_MTREE_NODE_NULL;

				pSrch->Style.p32.pMoreSpecific = pNode;

				/* The old head-of-list is no longer in the REAL patricia Tree. */
				pSrch->Style.p32.pLeft =
				    pSrch->Style.p32.pRight = pSrch->Style.p32.pDownToMe = NCS_MTREE_NODE_NULL;

				/* Special case for the root node. */
				if (pTree->Style.p32.pRoot == pSrch) {
					pTree->Style.p32.pRoot = pNode;	/* Become the root node. */
				}

				/* pNode is fully installed. */
			}

			return pNode;
		}
		/* if (pSrch->KeyBitSize < KeyBitSize) */
		do {
			if (pSrch->KeyBitSize == KeyBitSize) {
				/* Found an exact match! */
				return pSrch;
			}
			if ((pSrch->Style.p32.pLessSpecific == NCS_MTREE_NODE_NULL) ||
			    (pSrch->Style.p32.pLessSpecific->KeyBitSize < KeyBitSize)
			    ) {
				break;
			}
			pSrch = pSrch->Style.p32.pLessSpecific;
		} while (TRUE);

		/* We must insert a new node AFTER pSrch. */

		if ((pNode = (*pFunc) (pKey, KeyBitSize, Cookie)) != NCS_MTREE_NODE_NULL) {
			/* The caller has conveniently supplied us with a new one to add.
			 * The caller has filled its pKey and KeyBitSize fields.
			 * Initialize its patricia fields now!
			 */
			pNode->Style.p32.Bit = pSrch->Style.p32.Bit;
			pNode->Style.p32.pRight =
			    pNode->Style.p32.pLeft = pNode->Style.p32.pDownToMe = NCS_MTREE_NODE_NULL;
			pNode->Style.p32.pLessSpecific = pSrch->Style.p32.pLessSpecific;
			pNode->Style.p32.pMoreSpecific = pSrch;

			pSrch->Style.p32.pLessSpecific = pNode;

			if (pNode->Style.p32.pLessSpecific != NCS_MTREE_NODE_NULL) {
				pNode->Style.p32.pLessSpecific->Style.p32.pMoreSpecific = pNode;
			}
			/* New node is fully installed. */
		}
		return pNode;
	}

	/* if (exact match) */
	/* If we get here, this will become a new node in the patricia tree.  */
	if ((pNode = (*pFunc) (pKey, KeyBitSize, Cookie)) != NCS_MTREE_NODE_NULL) {
		register int Bit;
		register uns32 Diff;
		/* The caller has conveniently supplied us with a new one to add.
		 * The caller has filled its pKey and KeyBitSize fields.
		 * Initialize its patricia fields now!
		 */
		Bit = 0;
		Diff = *(uns32 *)pSrch->pKey ^ *(uns32 *)pKey;	/* bitwise xor */

		/* Find the first point of difference between the new key and 'pSrch' */
		while (!m_MTREE_P32_BIT_IS_SET(Diff, Bit)) {
			Bit++;
		}

		/* Search for the correct insert point. 
		 * Maybe we don't need to start all the way at the root. 
		 */
		if (Bit < (int)pSrch->Style.p32.Bit) {
			pSrch = pTree->Style.p32.pRoot;	/* Insert point is 'above' us in the tree */
		}

		do {
			pPrev = pSrch;

			if (m_MTREE_P32_BIT_IS_SET(*pKey, pSrch->Style.p32.Bit)) {
				pSrch = pSrch->Style.p32.pRight;
			} else {
				pSrch = pSrch->Style.p32.pLeft;
			}

			if (pSrch->Style.p32.Bit <= pPrev->Style.p32.Bit) {
				/* Found the insert point.
				 * It is below 'pPrev' and 'pPrev' is below 'pSrch'
				 */
				break;
			}
			if (Bit < (int)pSrch->Style.p32.Bit) {
				/* Found the insert point.
				 * It is below 'pPrev' and 'pPrev is ABOVE 'pSrch' 
				 */
				pSrch->Style.p32.pDownToMe = pNode;
				break;
			}
		} while (TRUE);

		pNode->Style.p32.pDownToMe = pPrev;

		pNode->Style.p32.Bit = (short)Bit;

		pNode->Style.p32.pLessSpecific = pNode->Style.p32.pMoreSpecific = NCS_MTREE_NODE_NULL;

		if (m_MTREE_P32_BIT_IS_SET(*pKey, Bit)) {
			pNode->Style.p32.pLeft = pSrch;

			pNode->Style.p32.pRight = pNode;
		} else {
			pNode->Style.p32.pLeft = pNode;

			pNode->Style.p32.pRight = pSrch;
		}

		if (m_MTREE_P32_BIT_IS_SET(*pKey, pPrev->Style.p32.Bit)) {
			pPrev->Style.p32.pRight = pNode;
		} else {
			pPrev->Style.p32.pLeft = pNode;
		}

	}

	return pNode;
}

/******************************************************************
 * Function:  mtree_p32_del()
 * Purpose:   removes a node from the best-match tree.
 ******************************************************************/
static void mtree_p32_del(NCS_MTREE *const pTree, NCS_MTREE_NODE *const pNode)
{
	NCS_MTREE_NODE *pNext;
	NCS_MTREE_NODE **pNextLeg;
	NCS_MTREE_NODE **pLegDownToNode;
	NCS_MTREE_NODE *pTemp;
	NCS_MTREE_NODE *pPrev;
	NCS_MTREE_NODE **pPrevLeg;
	int UpWentRight;

	/* First the easy case.  If this node is part of a chain of 
	 * equal-key-but-different-key-len's then we can de-link this
	 * node.
	 */
	if ((pTemp = pNode->Style.p32.pMoreSpecific) != NCS_MTREE_NODE_NULL) {
		/* This guy isn't even in the patricia tree. 
		 * Just de-link him.  Sweet!
		 */
		if ((pTemp->Style.p32.pLessSpecific = pNode->Style.p32.pLessSpecific)
		    != NCS_MTREE_NODE_NULL) {
			pNode->Style.p32.pLessSpecific->Style.p32.pMoreSpecific = pNode->Style.p32.pMoreSpecific;
		}
		return;
	}
	if ((pTemp = pNode->Style.p32.pLessSpecific) != NCS_MTREE_NODE_NULL) {
		/* A little more complicated.
		 * The node to be removed is part of the patricia tree, but it
		 * is the head of the linked list of equal-key nodes, and there is
		 * another one (pSrch) to take its place in the tree.
		 */
		pTemp->Style.p32.Bit = pNode->Style.p32.Bit;

		pTemp->Style.p32.pLeft = pNode->Style.p32.pLeft;
		pTemp->Style.p32.pRight = pNode->Style.p32.pRight;

		if (m_MTREE_P32_BIT_IS_SET(*(uns32 *)pNode->pKey, pNode->Style.p32.Bit)) {
			pNext = pNode->Style.p32.pRight;
			if (pNext == pNode) {
				pTemp->Style.p32.pRight = pTemp;
				pNext = NCS_MTREE_NODE_NULL;
			}
		} else {
			pNext = pNode->Style.p32.pLeft;
			if (pNext == pNode) {
				pTemp->Style.p32.pLeft = pTemp;
				pNext = NCS_MTREE_NODE_NULL;
			}
		}
		if (pNext != NCS_MTREE_NODE_NULL) {
			do {
				if (m_MTREE_P32_BIT_IS_SET(*(uns32 *)pNode->pKey, pNext->Style.p32.Bit)) {
					pNextLeg = &pNext->Style.p32.pRight;
				} else {
					pNextLeg = &pNext->Style.p32.pLeft;
				}
				pNext = *pNextLeg;
				if (pNext == pNode) {
					/* We've found the pointer which is pointing 'up' to pNode. */
					*pNextLeg = pTemp;
					break;	/* from 'do' */
				}
			} while (TRUE);
		}

		/* Adjust the node (if any) which is pointing down to us */
		if ((pTemp->Style.p32.pDownToMe = pNode->Style.p32.pDownToMe) != NCS_MTREE_NODE_NULL) {
			if (pTemp->Style.p32.pDownToMe->Style.p32.pLeft == pNode) {
				pTemp->Style.p32.pDownToMe->Style.p32.pLeft = pTemp;
			} else {
				pTemp->Style.p32.pDownToMe->Style.p32.pRight = pTemp;
			}
		} else {
			/* Special cases for removal of root node. (0.0.0.0/0, for example) */
			pTree->Style.p32.pRoot = pTemp;	/* New root! */
			pTemp->Style.p32.pRight = pTemp;
		}

		/* Set up-pointers of any nodes below us.  */
		if (pNode->Style.p32.pLeft->Style.p32.Bit > pNode->Style.p32.Bit) {
			pNode->Style.p32.pLeft->Style.p32.pDownToMe = pTemp;
		}
		if (pNode->Style.p32.pRight->Style.p32.Bit > pNode->Style.p32.Bit) {
			pNode->Style.p32.pRight->Style.p32.pDownToMe = pTemp;
		}

		pTemp->Style.p32.pMoreSpecific = NCS_MTREE_NODE_NULL;

		/* 'Node' is safely out of the patricia tree. */
		return;
	}

	/* We need to actually remove the node and reorganize the pat tree. */
	pPrev = pNode->Style.p32.pDownToMe;
	pLegDownToNode = ((pPrev->Style.p32.pLeft == pNode) ? &pPrev->Style.p32.pLeft : &pPrev->Style.p32.pRight);
	pPrevLeg = pLegDownToNode;
	pNext = pNode;

	/* Keep going 'down' until we find the leg which points back to pNode. */
	do {
		UpWentRight = m_MTREE_P32_BIT_IS_SET(*(uns32 *)(pNode->pKey), pNext->Style.p32.Bit);
		pNextLeg = ((UpWentRight) ? &pNext->Style.p32.pRight : &pNext->Style.p32.pLeft);
		pTemp = *pNextLeg;

		if (pTemp == pNode) {
			break;
		}

		if (pTemp->Style.p32.Bit < pNext->Style.p32.Bit) {
			/* Inconsistency in pat tree?   Panic? */
			return;
		}

		/* loop again */
		pPrev = pNext;
		pNext = pTemp;
		pPrevLeg = pNextLeg;
	} while (TRUE);

	/* At this time:
	 * pNode->pDownToMe is the node pointing DOWN to the node to delete (pNode)
	 * pLegDownToNode points to the down-leg within that node.
	 * pNext is the one pointing UP to the node to delete (pNode)
	 * pPrev points to the node which is pointing DOWN to pNext.
	 * pPrevLeg points to to the down-leg within pPrev which points to pNextNode.
	 * UpWentRight is the direction which pNext took (in the UP
	 *   direction) to get to the node to delete.
	 */

	/* We're ready to rearrange the tree.
	 * BE CAREFUL.  The order of the following statements
	 * is critical.
	 */
	pNext->Style.p32.Bit = pNode->Style.p32.Bit;	/* It gets the 'bit' value of the evacuee. */
	pNode->Style.p32.Bit = 0x7fff;	/* Extremely big */
	*pLegDownToNode = pNext;
	pNext->Style.p32.pDownToMe = pNode->Style.p32.pDownToMe;

	*pPrevLeg = ((UpWentRight) ? pNext->Style.p32.pLeft : pNext->Style.p32.pRight);
	if ((*pPrevLeg)->Style.p32.Bit > pPrev->Style.p32.Bit) {
		(*pPrevLeg)->Style.p32.pDownToMe = pPrev;
	}
	pNext->Style.p32.pRight = pNode->Style.p32.pRight;
	if (pNext->Style.p32.pRight->Style.p32.Bit > pNext->Style.p32.Bit) {
		pNext->Style.p32.pRight->Style.p32.pDownToMe = pNext;
	}
	pNext->Style.p32.pLeft = pNode->Style.p32.pLeft;
	if (pNext->Style.p32.pLeft->Style.p32.Bit > pNext->Style.p32.Bit) {
		pNext->Style.p32.pLeft->Style.p32.pDownToMe = pNext;
	}
}

/******************************************************************
 * Function:  mtree_p32_best()
 * Purpose:   Finds best-match (i.e. longest) matching prefix
 *******************************************************************/
static NCS_MTREE_NODE *mtree_p32_best(NCS_MTREE *const pTree,
				      const uns32 *const pKey, short KeyBitSize, int *const pMoreSpecificExists)
{
	uns32 Target;
	uns32 NewTarget;
	NCS_MTREE_NODE *pSrch;
	NCS_MTREE_NODE *pPrev;
	NCS_MTREE_NODE *pSrchSave;
	int Dummy;

	*pMoreSpecificExists = FALSE;	/* Assume not. */
	Target = *pKey;

	pSrch = pTree->Style.p32.pRoot;

	do {
		pPrev = pSrch;

		if (m_MTREE_P32_BIT_IS_SET(Target, pSrch->Style.p32.Bit)) {
			pSrch = pSrch->Style.p32.pRight;
		} else {
			pSrch = pSrch->Style.p32.pLeft;
		}

		if (pSrch->Style.p32.Bit <= pPrev->Style.p32.Bit) {
			if ((pSrch->KeyBitSize > KeyBitSize) &&
			    (m_MTREE_P32_KEY_MASK_CMP(Target, *(const uns32 *)pSrch->pKey, KeyBitSize))
			    ) {
				*pMoreSpecificExists = TRUE;
			}

			if (pSrch == pPrev) {
				pPrev = pSrch->Style.p32.pDownToMe;
				pSrchSave = pSrch;
			} else {
				pPrev = pPrev->Style.p32.pDownToMe;
				pSrchSave = NCS_MTREE_NODE_NULL;
			}

			do {
				if ((pSrch->KeyBitSize <= KeyBitSize) &&
				    (m_MTREE_P32_KEY_MASK_CMP(Target, *(const uns32 *)pSrch->pKey, pSrch->KeyBitSize))
				    ) {
					/* This is it! */

					/* Special case here... 
					 * It's possible we have not visited any of the more-specific keys...
					 */
					if ((pSrchSave != NCS_MTREE_NODE_NULL) &&
					    (pSrch == pSrchSave) && (pSrch->KeyBitSize <= KeyBitSize)
					    ) {
						if (m_MTREE_P32_BIT_IS_SET(Target, pSrchSave->Style.p32.Bit)) {
							pSrchSave = pSrch->Style.p32.pLeft;	/* go the OPPOSITE WAY */
						} else {
							pSrchSave = pSrch->Style.p32.pRight;	/* go the OPPOSITE WAY */
						}

						if ((pSrchSave->Style.p32.Bit > pSrch->Style.p32.Bit) &&
						    (m_MTREE_P32_KEY_MASK_CMP
						     (Target, *(const uns32 *)pSrchSave->pKey, KeyBitSize))) {
							*pMoreSpecificExists = TRUE;
						}
					}

					return pSrch;
				}
				pSrch = pSrch->Style.p32.pLessSpecific;
			} while (pSrch != NCS_MTREE_NODE_NULL);

			/* Get out of this loop, and start looking for less-specific
			 * matches.
			 */
			goto Lookback;	/* I got a headache trying to eliminate this!  (S.A.) */
		}

		/* Maybe we can continue to go down, maybe not.
		 * Does the target match all bit postions up to
		 * (but not including) 'Bit'?
		 */
		if (!m_MTREE_P32_KEY_MASK_CMP(Target, *(const uns32 *)pSrch->pKey, pSrch->Style.p32.Bit)) {
			/* There's no point in going forward. */
			break;

		}

		if ((pSrch->KeyBitSize > KeyBitSize) &&
		    (m_MTREE_P32_KEY_MASK_CMP(Target, *(const uns32 *)pSrch->pKey, KeyBitSize))
		    ) {
			*pMoreSpecificExists = TRUE;
		}
	} while (TRUE);

	if (pPrev == NCS_MTREE_NODE_NULL) {
		return NCS_MTREE_NODE_NULL;
	}

	while (pSrch->KeyBitSize >= pPrev->Style.p32.Bit) {
		if ((pSrch->KeyBitSize <= pSrch->Style.p32.Bit) &&
		    (pSrch->KeyBitSize <= KeyBitSize) &&
		    (m_MTREE_P32_KEY_MASK_CMP(Target, *(const uns32 *)pSrch->pKey, pSrch->KeyBitSize))
		    ) {
			return pSrch;
		}
		pSrch = pSrch->Style.p32.pLessSpecific;
		if (pSrch == NCS_MTREE_NODE_NULL) {
			break;
		}
	}

	/*
	 * Try backing up and trying a less-specific mask.
	 */

 Lookback:

	while (pPrev != NCS_MTREE_NODE_NULL) {
		if (m_MTREE_P32_BIT_IS_SET(Target, pPrev->Style.p32.Bit)) {
			pSrch = pPrev->Style.p32.pRight;
		} else {
			pSrch = pPrev->Style.p32.pLeft;
		}

		if (KeyBitSize > pSrch->Style.p32.Bit) {
			KeyBitSize = (short)((int)pSrch->Style.p32.Bit);
		}

		while (KeyBitSize > pPrev->Style.p32.Bit) {
			if (m_MTREE_P32_KEY_MASK_CMP(Target, *(const uns32 *)pSrch->pKey, KeyBitSize)) {
				NewTarget = Target & P32Masks[KeyBitSize];
				if (NewTarget != Target) {
					return (mtree_p32_best(pTree, &NewTarget, KeyBitSize, &Dummy));
				}
			}
			KeyBitSize -= 1;
		}

		pPrev = pPrev->Style.p32.pDownToMe;
	}

	return NCS_MTREE_NODE_NULL;
}

/*******************************************************************
 * Function:  mtree_p32_find_nextbest
 * Purpose:   Given a MNODE, find the next-best matching prefix
 *            in the tree.  Used to find alternate matching entries.
 *******************************************************************/
static NCS_MTREE_NODE *mtree_p32_nextbest(NCS_MTREE *const pTree, const NCS_MTREE_NODE *const pBetterNode)
{
	uns32 Target;
	int MaxBitSize;
	int Dummy;

	if (pBetterNode->Style.p32.pLessSpecific != NCS_MTREE_NODE_NULL) {
		/* Don't return the fake node. */
		if (pBetterNode->Style.p32.pLessSpecific != &pTree->Style.p32.Root) {
			return pBetterNode->Style.p32.pLessSpecific;
		}
		return NCS_MTREE_NODE_NULL;
	}

	Target = *(const uns32 *)pBetterNode->pKey;
	MaxBitSize = ((int)pBetterNode->KeyBitSize) - 1;

	/* Find the least-specific bit which is '1'.  
	 * We know there is at least one. 
	 */
	while (!m_MTREE_P32_BIT_IS_SET(Target, MaxBitSize)) {
		MaxBitSize--;
	}
	Target &= P32Masks[MaxBitSize];	/* Clear that bit. */

	return (mtree_p32_best(pTree, &Target, (short)MaxBitSize, &Dummy));
}

/*********************************************************************
 * Function:  mtree_p32_next()
 * Purpose:   Finds 'next' key.
 ***********************************************************************/
static NCS_MTREE_NODE *mtree_p32_next(NCS_MTREE *const pTree, const uns32 *const pKey, short KeyBitSize)
{
	uns32 Target;
	NCS_MTREE_NODE *pSrch;
	NCS_MTREE_NODE *pPrev;

	if (pKey == NULL) {	/* Start from beginning? */
		Target = 0;
		KeyBitSize = -1;
	} else {
		Target = *pKey;
	}

	pSrch = pTree->Style.p32.pRoot;
	do {
		pPrev = pSrch;

		if (m_MTREE_P32_BIT_IS_SET(Target, pSrch->Style.p32.Bit)) {
			pSrch = pSrch->Style.p32.pRight;
		} else {
			pSrch = pSrch->Style.p32.pLeft;
		}

		if (pSrch->Style.p32.Bit <= pPrev->Style.p32.Bit) {
			if (((uns32)(ntohl(Target)) <= ((uns32)ntohl(*(const uns32 *)pSrch->pKey))) &&
			    (m_MTREE_P32_KEY_MASK_CMP(Target, *(const uns32 *)pSrch->pKey, 1 + pPrev->Style.p32.Bit))
			    ) {
				/* HOORAY!  We're almost done!  
				 * Find the last one which is greater.
				 */
				if (Target != *(uns32 *)pSrch->pKey) {
					KeyBitSize = -1;
				}

				if (pSrch->KeyBitSize > KeyBitSize) {
					while ((pSrch->Style.p32.pLessSpecific != NCS_MTREE_NODE_NULL) &&
					       (pSrch->Style.p32.pLessSpecific->KeyBitSize > KeyBitSize)
					    ) {
						pSrch = pSrch->Style.p32.pLessSpecific;
					}
					return pSrch;
				}
				/* We found a match on 'target' but there are no (more) keys with
				 * length greater than 'Target'.
				 */
			}

 backup:

			KeyBitSize = -1;

			do {

				if (pSrch == pPrev->Style.p32.pLeft) {
					/* We went left */
					/* We went left to get here */
					if (pPrev->Style.p32.Bit < 0) {
						return NCS_MTREE_NODE_NULL;
					}
					pSrch = pPrev;
					Target = (((*(uns32 *)pSrch->pKey) &
						   P32Masks[pSrch->Style.p32.Bit]) | P32Bits[1 + pSrch->Style.p32.Bit]);
					break;
				} else {
					/* We went right to get here */

					if (pPrev->Style.p32.Bit <= 0) {
						return NCS_MTREE_NODE_NULL;
					}

					Target =
					    htonl(ntohl(((*(uns32 *)pPrev->pKey) | ~P32Masks[pPrev->Style.p32.Bit])) +
						  1);
					if (Target == 0) {	/* Wraparound to 0 would cause infinite loop */
						return NCS_MTREE_NODE_NULL;
					}

					pSrch = pPrev;
					pPrev = pSrch->Style.p32.pDownToMe;

					if (m_MTREE_P32_KEY_MASK_CMP
					    (Target, *(const uns32 *)pSrch->pKey, pSrch->Style.p32.Bit))
						break;
				}

			} while (TRUE);

		} /* if (pSrch->Bit <= pPrev->Bit) */
		else {
			/* We're still going 'down'... but make sure we haven't gone down too far. */
			if (!m_MTREE_P32_KEY_MASK_CMP(Target, *(const uns32 *)pSrch->pKey, pSrch->Style.p32.Bit)) {
				register uns32 NewTarget;

				NewTarget = (*(uns32 *)pSrch->pKey) & P32Masks[pSrch->Style.p32.Bit];

				if (((uns32)ntohl(Target)) > ((uns32)ntohl(NewTarget))) {
					goto backup;
				}

				Target = NewTarget;
				KeyBitSize = -1;
			}
		}
	} while (TRUE);

}
#else				/*#if (NCS_XMTREE_WRAPPERS == 0) */
/*wrappers for testing*/

static NCS_MTREE_NODE *mtree_p32_findadd(NCS_MTREE *const pTree,
					 const uns32 *const pKey,
					 const short KeyBitSize,
					 NCS_MTREE_NODE *(*pFunc) (const void *pKey,
								   short KeyBitSize, void *Cookie), void *const Cookie)
{
	NCS_MTREE_NODE *testNode;
	testNode = xmtree_findadd(pTree,
				  /*changed pKey to testKey for testing */
				  pKey, KeyBitSize, pFunc, Cookie);

	return (testNode);

}

static void mtree_p32_del(NCS_MTREE *const pTree, NCS_MTREE_NODE *const pNode)
{
	xmtree_del(pTree, pNode);
}

static NCS_MTREE_NODE *mtree_p32_best(NCS_MTREE *const pTree,
				      const uns32 *const pKey, short KeyBitSize, int *const pMoreSpecificExists)
{
	NCS_MTREE_NODE *testNode;
	testNode = xmtree_best(pTree, pKey, KeyBitSize, pMoreSpecificExists);
	return (testNode);

}

static NCS_MTREE_NODE *mtree_p32_nextbest(NCS_MTREE *const pTree, const NCS_MTREE_NODE *const pBetterNode)
{
	NCS_MTREE_NODE *testNode;
	testNode = xmtree_nextbest(pTree, pBetterNode);
	return (testNode);

}

static NCS_MTREE_NODE *mtree_p32_next(NCS_MTREE *const pTree, const uns32 *const pKey, short KeyBitSize)
{
	NCS_MTREE_NODE *testNode;
	testNode = xmtree_next(pTree, pKey, KeyBitSize);
	return (testNode);

}
#endif   /*#if (NCS_XMTREE_WRAPPERS == 0) */

static const struct {
	unsigned int (*pInitFunc) (NCS_MTREE *const pTree);
	void (*pDestFunc) (NCS_MTREE *const pTree);
	NCS_MTREE_NODE *(*pFindAddFunc) (NCS_MTREE *const pTree,
					 const uns32 *const pKey,
					 const short KeyBitSize,
					 NCS_MTREE_NODE *(*pFunc) (const void *pKey,
								   short KeyBitSize, void *Cookie), void *const Cookie);
	void (*pDelFunc) (NCS_MTREE *const pTree, NCS_MTREE_NODE *const pNode);
	NCS_MTREE_NODE *(*pBestFunc) (NCS_MTREE *const pTree,
				      const uns32 *const pKey, short KeyBitSize, int *const pMoreSpecificExists);
	NCS_MTREE_NODE *(*pNextBestFunc) (NCS_MTREE *const pTree, const NCS_MTREE_NODE *const pBetterNode);
	NCS_MTREE_NODE *(*pNextFunc) (NCS_MTREE *const pMTree, const uns32 *const pKey, short KeyBitSize);
	uns16 MaxKeyOctetSize;

} MtreeActions[NCS_MTREE_STYLE_MAX] = {
	/* Functions for Patricia style */
	{
	mtree_p32_init,
		    mtree_p32_dest,
		    mtree_p32_findadd,
		    mtree_p32_del, mtree_p32_best, mtree_p32_nextbest, mtree_p32_next, sizeof(uns32),},
#if (NCS_MTREE_PAT_STYLE_SUPPORTED != 0)
	    /* Functions for Patricia style */
	{
	xmtree_init,
		    xmtree_dest,
		    xmtree_findadd, xmtree_del, xmtree_best, xmtree_nextbest, xmtree_next, NCS_XMTREE_MAX_KEY_BYTE,},
#endif   /* (NCS_MTREE_PAT_STYLE_SUPPORTED != 0) */
};

/************************************************************
 * Function:  ncs_mtree_init()
 * Purpose:   Initialize a 'tree' for storing best-match lookup.
 *            Called once at startup for each tree.
 *            If used for IP Address match, we would expect one
 *            NCS_MTREE to be created for IPv4 addresses and
 *            another for IPv6 addresses.
 *
 * Arguments: NCS_MTREE_CFG*:  Properties of the tree.
 *
 * Returns:   NCSCC_RC_SUCCESS if success.
 ***************************************************************/
unsigned int ncs_mtree_init(NCS_MTREE *const pTree, const NCS_MTREE_CFG *const pCfg)
{
	if (((unsigned int)pCfg->Style) >= NCS_MTREE_STYLE_MAX) {
		return (unsigned int)m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (pCfg == NULL)
		return (unsigned int)m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	if ((pCfg->KeyOctetSize < 1) ||
	    (pCfg->KeyOctetSize > MtreeActions[pCfg->Style].MaxKeyOctetSize) ||
	    (pCfg->MaxKeyBitSize < 1) || (pCfg->MaxKeyBitSize > 8 * pCfg->KeyOctetSize)
	    ) {
		return (unsigned int)m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

	pTree->Cfg = *pCfg;

	/* Go to style-specific handler */

	return ((*MtreeActions[pCfg->Style].pInitFunc) (pTree));

}

/***************************************************************
 * Function:  ncs_mtree_destroy()
 * Purpose:   Frees a 'tree' and all subsequent structures.
 *            If the tree was not empty, it will remove all nodes.
 *
 * Arguments: struct ncs_mtree*:  a value returned from ncs_mtree_create()
 * 
 * Returns:   nothing.
 *****************************************************************/

void ncs_mtree_destroy(NCS_MTREE *const pTree)
{
	(*MtreeActions[pTree->Cfg.Style].pDestFunc) (pTree);
}

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

static NCS_MTREE_NODE *PrivateCallback(const void *pKey, short KeyBitSize, void *pNode)
{
	return ((NCS_MTREE_NODE *)pNode);
}

unsigned int ncs_mtree_add(NCS_MTREE *const pTree, NCS_MTREE_NODE *const pNode)
{
	NCS_MTREE_NODE *pRetNode;

	pRetNode = ncs_mtree_find_add(pTree, pNode->pKey, pNode->KeyBitSize, PrivateCallback, pNode);
	if (pRetNode == pNode) {
		return NCSCC_RC_SUCCESS;
	} else {
		return (unsigned int)m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
}

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
NCS_MTREE_NODE *ncs_mtree_find_add(NCS_MTREE *const pTree,
				   const void *const pKey,
				   const short KeyBitSize,
				   NCS_MTREE_NODE *(*pFunc) (const void *pKey,
							     short KeyBitSize, void *Cookie), void *const Cookie)
{
	return ((*MtreeActions[pTree->Cfg.Style].pFindAddFunc) (pTree, pKey, KeyBitSize, pFunc, Cookie));
}

/******************************************************************
 * Function:  ncs_mtree_del()
 * Purpose:   removes a node from the best-match tree.
 *
 * Arguments:NCS_MTREE*:  tree from which to remove.
 *           NCS_MNODE*:  as returned from ncs_mtree_add()
 *
 * Returns:  nothing.
 ******************************************************************/
void ncs_mtree_del(NCS_MTREE *const pTree, NCS_MTREE_NODE *const pNode)
{
	(*MtreeActions[pTree->Cfg.Style].pDelFunc) (pTree, pNode);
}

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
NCS_MTREE_NODE *ncs_mtree_find_nextbest(NCS_MTREE *const pTree, const NCS_MTREE_NODE *const pBetterNode)
{
	return ((*MtreeActions[pTree->Cfg.Style].pNextBestFunc) (pTree, pBetterNode));
}

/******************************************************************
 * Function:  ncs_mtree_find_best()
 * Purpose:   Finds best-match (i.e. longest) matching prefix
 *
 * Arguments: NCS_MTREE*:  Tree to search
 *            void    :  pointer to key (size=KeyOctetSize)
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
NCS_MTREE_NODE *ncs_mtree_find_best(NCS_MTREE *const pTree,
				    const void *const pKey, uns16 KeyBitSize, int *const pMoreSpecificExists)
{
	return ((*MtreeActions[pTree->Cfg.Style].pBestFunc) (pTree, pKey, KeyBitSize, pMoreSpecificExists));
}

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
NCS_MTREE_NODE *ncs_mtree_next(NCS_MTREE *const pTree, const void *const pKey, short KeyBitSize)
{
	return ((*MtreeActions[pTree->Cfg.Style].pNextFunc) (pTree, pKey, KeyBitSize));
}
