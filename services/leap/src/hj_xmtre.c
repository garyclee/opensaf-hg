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

#if (NCS_MTREE_PAT_STYLE_SUPPORTED != 0)
/* Get compile time options...
 */
#include "ncs_opt.h"


/** Global Declarations...
 **/
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncs_xmtre.h"

static const uns8 xmtreeBits[8]         = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
static const uns8 xmtreeBitMasks[8]     = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80};

#define m_XMTREE_BIT_IS_SET(k, b,ml) ((b < 0) ? FALSE : (((*(((const uns8*)k) + \
               (m_XMTREE_BYTE_POSITION(b/8,ml)))) & xmtreeBits[(short)(b & 0x07)]) != 0))
#define m_XMTREE_KEY_FIRST_DIFF(a,b,c) m_xmtree_find_first_diff((char *)a,b,c)
#define m_XMTREE_KEY_CMP(a,b,c) memcmp(a,b,c)
#define m_XMTREE_KEY_MASK_CMP(k1, k2, kl, mkl) m_xtree_compare_with_mask((char *)k1,\
                  (char *)k2, kl,mkl)
#define m_XMTREE_MASK_KEY(k1,k2,kl,mk) m_xmtree_mask_key((char *)k1, (char*)k2,kl,mk)
#define m_XMTREE_INVERT_MASK_AND_OR_KEY(k1,k2,kl,mk) m_xmtree_invert_mask_and_or_key(\
                                       (char *)k1, (char*)k2,kl,mk)
#define m_XMTREE_CLEAR_BIT(k,b) (k[b/8]=(char)((k[b/8]) & ((char)(~xmtreeBits[b%8]))))
#define m_XMTREE_COPY_KEY_VALUE(dk,sk,kl) m_xtree_copy_key_value(dk,sk,kl)
#define m_XMTREE_GEN_MASK(m,ml,mkl) m_xmtree_gen_mask(m,ml,mkl)
#define m_XMTREE_SET_BIT(k,kl,mkl) m_xmtree_set_bit(k,kl,mkl)
#define m_XMTREE_INC_KEY(dk,sk,kl) m_xmtree_increment_key(dk,sk,kl)
#define m_XMTREE_AND_KEYS(dk,sk1,sk2,kl) m_xmtree_and_keys((char *)dk,(char *)sk1,\
                                    (char *)sk2,kl)
#define m_XMTREE_IS_KEY_ZERO(k,kl) m_xmtree_is_key_zero(k,kl)

/* removed the USE_LITTLE_ENDIAN check */
  #define m_XMTREE_BYTE_POSITION(kp, mkl) (kp)
  #define m_XMTREE_BYTE_INC_POSITION(kp, mkl) ((mkl-kp)-1)

/*******************************************************************
 * Function:  m_xmtree_set_bit()
 * Purpose:   Sets the particular bit for the given Key
 *******************************************************************/
static void
m_xmtree_set_bit (char *pKey, uns32 keyLen, uns32 maxKeyLen)
{

    uns32 byteCount;
    
    keyLen = (keyLen-1);
    byteCount = m_XMTREE_BYTE_POSITION((keyLen/8), maxKeyLen);
    pKey[byteCount] = (char)(pKey[byteCount] | xmtreeBits[(keyLen%8)]);
    return;
}


/*******************************************************************
 * Function:  m_xmtree_is_key_zero()
 * Purpose:   Set the Keys value to zero
 *******************************************************************/
static int 
m_xmtree_is_key_zero (char *pKey, uns32 maxKeyLen)
{
    uns32 byteCount = 0;
    for (byteCount = 0; byteCount < (maxKeyLen); byteCount++)
    {
        if (pKey[byteCount] != 0)
        {
            return FALSE;
        }
    }
    return TRUE;

}

/*******************************************************************
 * Function:  m_xmtree_mask_key()
 * Purpose:   Masks the Key value for the given Mask Bits
 *******************************************************************/

static void
m_xmtree_mask_key (char *pDestKey, char *pSrcKey, int maskLen, uns32 maxKeyLen)
{
    uns32 i;

    memcpy (pDestKey, pSrcKey, maxKeyLen);
    pDestKey[m_XMTREE_BYTE_POSITION((maskLen/8),maxKeyLen)] = 
             (char)(pDestKey[(maskLen/8)] & xmtreeBitMasks[(8-(maskLen%8))]);
   /* removed the USE_LITTLE_ENDIAN check */
for (i= m_XMTREE_BYTE_POSITION(((maskLen/8)+1),maxKeyLen); i< (maxKeyLen); i++)
    {
         pDestKey[i] = 0x0;
    }

}

/*******************************************************************
 * Function:  m_xmtree_invert_mask_and_or_key()
 * Purpose:   Masks the Key value for the given Mask Bits
 *******************************************************************/

static void
m_xmtree_invert_mask_and_or_key (char *pDestKey, char *pSrcKey, int maskLen, uns32 maxKeyLen)
{
    uns32 i;
    int j=0xff;

    memcpy (pDestKey, pSrcKey, maxKeyLen);
    pDestKey[m_XMTREE_BYTE_POSITION((maskLen/8),maxKeyLen)] = 
             (char)(pDestKey[(maskLen/8)] | (~(char)(xmtreeBitMasks[(8-(maskLen%8))])));
   /* removed the USE_LITTLE_ENDIAN check */
    for (i= m_XMTREE_BYTE_POSITION(((maskLen/8)+1),maxKeyLen); i< (maxKeyLen); i++)
    { 
         pDestKey[i] = (char)j;
    }

}

/*******************************************************************
 * Function:  m_xmtree_gen_mask()
 * Purpose:   Generate the Mask for the given bits
 *******************************************************************/
static void
m_xmtree_gen_mask (char *pMask, uns32 maskLen, uns32 maxKeyLen)
{
    uns32 byteCount        = 0x0;
    uns32 tempCount        = 0x0;
    unsigned char temp_var = 0xff;

    /* make the value 0*/
    for (tempCount =0; tempCount <maxKeyLen; tempCount++)
    {
        pMask[tempCount] = 0x00;

    }
    
    if (maskLen >0 ) {
        
        for (byteCount = 0; byteCount < (maskLen/8); byteCount++)
        {
            pMask[m_XMTREE_BYTE_POSITION(byteCount,maxKeyLen)] = (char)temp_var;
        }
        if ((maskLen%8) != 0)
            pMask[m_XMTREE_BYTE_POSITION(byteCount,maxKeyLen)] = 
                   xmtreeBitMasks[(8-(maskLen%8))]; 
    }
    return;
}

/*******************************************************************
 * Function:  m_xmtree_and_keys()
 * Purpose:   AND operation on the give set keys
 *******************************************************************/
static void
m_xmtree_and_keys (char *pDestKey, char *pSrcKey1, char *pSrcKey2, uns32 maxKeyLen)
{
    uns32 tempCount;

    for (tempCount = 0; tempCount < maxKeyLen; tempCount++)
    {
        pDestKey[tempCount] = (unsigned char)(pSrcKey1[tempCount] & pSrcKey2[tempCount]);
    }
    return;
}

/*******************************************************************
 * Function:  m_xmtree_increment_key()
 * Purpose:   Increment the given key by one
 *******************************************************************/
static void 
m_xmtree_increment_key (char *pDestKey, char *pSrcKey, uns32 maxKeyLen)
{
    uns32 tempKeyLen =0;
    uns32 bytePosition;

    bytePosition = m_XMTREE_BYTE_INC_POSITION(tempKeyLen, maxKeyLen);

    while (((unsigned char)pSrcKey[bytePosition] == 0xff)){
       pDestKey[bytePosition] = 0;
       tempKeyLen++;
       bytePosition = m_XMTREE_BYTE_INC_POSITION(tempKeyLen, maxKeyLen);
       if(tempKeyLen == maxKeyLen)
           break;
    };

    if (tempKeyLen < maxKeyLen)
    {
         pDestKey[bytePosition] = (char) (pSrcKey[bytePosition] + 1);
    }
    return;
}

/*******************************************************************
 * Function:  m_xtree_compare_with_mask()
 * Purpose:   Compare the give sets of keys.
 * Return :   -1                     -  Invalid key length
 *             0                     -  if the keys are equal
 *            m_XMTREE_GREATER_VALUE -  if the pKey1 > pKey2 
 *            m_XMTREE_LESSER_VALUE  -  if the pKey1 < pKey2
 *******************************************************************/
static int 
m_xtree_compare_with_mask (char *pKey1, char *pKey2, int keyLength, uns32 maxKeyLen)
{
    int bitCount;
    int byteCount;
    int tempByteCount;
    char tempKey1;
    char tempKey2;

    if(keyLength >= 0)
    {
        if (keyLength == 0)
            return 0;
        /* checking for the byte boundry*/
        for (byteCount = 0; byteCount < (keyLength/8); byteCount++)
        {
            tempByteCount = m_XMTREE_BYTE_POSITION(byteCount,maxKeyLen);
            if (pKey1[tempByteCount] != pKey2[tempByteCount])
            {
                if (pKey1[tempByteCount] > pKey2[tempByteCount])
                    return m_XMTREE_GREATER_VALUE;
                else
                    return m_XMTREE_LESSER_VALUE;
            }
        }

        /*check for bit boundry*/
        bitCount = (keyLength%8);

        if (bitCount != 0)
        {
            tempByteCount = m_XMTREE_BYTE_POSITION(byteCount,maxKeyLen);
            tempKey1 = (char)(pKey1[tempByteCount] & xmtreeBitMasks[(8 - bitCount)]);
            tempKey2 = (char)(pKey2[tempByteCount] & xmtreeBitMasks[(8 - bitCount)]);
            if (tempKey1 != tempKey2)
            {
                if (tempKey1 > tempKey2)
                {
                    return m_XMTREE_GREATER_VALUE;
                }else
                {
                    return m_XMTREE_LESSER_VALUE;
                }
            } else 
            {
                return 0;
            }
        } else
        {
            return 0;
        }
    }else
    {
        return(-1);
    }

}

/*******************************************************************
 * Function:  m_xmtree_find_first_diff()
 * Purpose:   Finds the first bit possition where the given sets 
              of keys differs
 *******************************************************************/
static int
m_xmtree_find_first_diff(char *pKey1, char *pKey2, uns32 keyMaxsize)
{
    uns32 byteCounter = 0;
    uns32 bitCounter  = 0;
    uns32 tempByteCount;

    for(byteCounter=0; (byteCounter < keyMaxsize); byteCounter++) 
    {
        tempByteCount = m_XMTREE_BYTE_POSITION(byteCounter,keyMaxsize);
        if ((pKey1[tempByteCount] ^ pKey2[tempByteCount]) != 0) 
        {
            for (bitCounter=0; bitCounter<8; bitCounter++) 
            {
                if (((((pKey1[tempByteCount]&xmtreeBits[bitCounter])^(pKey2[tempByteCount]))
                     &xmtreeBits[bitCounter])) != 0) 
                {
                    return ((tempByteCount*8)+bitCounter);
                }
            }
        }
    }
    return 0;
}


/************************************************************
 * Function:  xmtree_init
 * Purpose:   Initialize a patricia tree style mtree.
 ***************************************************************/
unsigned int
xmtree_init(NCS_MTREE                 *const pTree)
{
   static const uns32 AlwaysZero = 0;

   /* Do additional tests. */
   if (pTree->Cfg.KeyOctetSize > NCS_XMTREE_MAX_KEY_BYTE)
   {
      return NCSCC_RC_FAILURE;
   }
   
/* Initialize the root node, which is actually part of the tree structure. */
   pTree->Style.Pat.Root.pKey = &AlwaysZero;
   pTree->Style.Pat.Root.KeyBitSize = -1;  /* designates the root */
   pTree->Style.Pat.Root.Style.Pat.Bit = -1;
   pTree->Style.Pat.Root.Style.Pat.pLeft =
   pTree->Style.Pat.Root.Style.Pat.pRight = 
   &pTree->Style.Pat.Root;
   pTree->Style.Pat.Root.Style.Pat.pDownToMe =
   pTree->Style.Pat.Root.Style.Pat.pMoreSpecific =
   pTree->Style.Pat.Root.Style.Pat.pLessSpecific =
   NCS_MTREE_NODE_NULL;

   pTree->Style.Pat.pRoot = &pTree->Style.Pat.Root;

   return NCSCC_RC_SUCCESS;
}

/***************************************************************
 * Function:  xmtree_dest()
 * Purpose:   Frees a 'tree' and all subsequent structures for patricia style
 *****************************************************************/
void
xmtree_dest(NCS_MTREE *const pTree)
{
}

/*******************************************************************
 * Function:  xmtree_findadd()
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

NCS_MTREE_NODE *
xmtree_findadd(NCS_MTREE       *const      pTree,
                  const uns32 *const      pTargetKey,
                  const short             KeyBitSize,
                  NCS_MTREE_NODE *         (*pFunc)(const void* pTargetKey, 
                                                   short KeyBitSize,
                                                   void *Cookie),
                  void          *const    Cookie)
{
   NCS_MTREE_NODE *pSrch;
   NCS_MTREE_NODE *pPrev;
   NCS_MTREE_NODE *pNode;
   char *pKey = (char *)pTargetKey;

   pSrch = pTree->Style.Pat.pRoot;

   do 
   {
      pPrev = pSrch;
      if (m_XMTREE_BIT_IS_SET(pKey, pSrch->Style.Pat.Bit,pTree->Cfg.KeyOctetSize))     
      {
         pSrch = pSrch->Style.Pat.pRight;
      }
      else
      {
         pSrch = pSrch->Style.Pat.pLeft;
      }

   } while (pSrch->Style.Pat.Bit > pPrev->Style.Pat.Bit);

   if (m_XMTREE_KEY_CMP(pKey, pSrch->pKey, pTree->Cfg.KeyOctetSize) == 0)
   {
      /* We may have a duplicate key, or we may have an equal key with a different 
       * bit length.  (for example, 10.3.0.0/16 and 10.3.0.0/24 would encounter this. )
       */
      if (pSrch->KeyBitSize < KeyBitSize)
      {
         /* The tree contains 10.3.0.0/16 and we're adding 10.3.0.0/24.
          * This is tricky.  We will add a new node at the head end of the
          * list.  This one will become the REAL member of the Pat Tree!
          */
         if ((pNode = (*pFunc)(pKey, KeyBitSize, Cookie)) != NCS_MTREE_NODE_NULL)
         {
            /* The caller has conveniently supplied us with a new one to add.
            * The caller has filled its pKey and KeyBitSize fields.
            * Initialize its patricia fields now!
            */

            pNode->Style.Pat.Bit = pSrch->Style.Pat.Bit;
            pNode->Style.Pat.pLeft = ((pSrch->Style.Pat.pLeft == pSrch) ?
                                      pNode :
                                      pSrch->Style.Pat.pLeft);
            if (pNode->Style.Pat.pLeft->Style.Pat.pDownToMe == pSrch)
            {
               pNode->Style.Pat.pLeft->Style.Pat.pDownToMe = pNode;
            }

            pNode->Style.Pat.pRight = ((pSrch->Style.Pat.pRight == pSrch) ?
                                       pNode :
                                       pSrch->Style.Pat.pRight);
            if (pNode->Style.Pat.pRight->Style.Pat.pDownToMe == pSrch)
            {
               pNode->Style.Pat.pRight->Style.Pat.pDownToMe = pNode;
            }


            if (pPrev->Style.Pat.pLeft == pSrch)
            {
               pPrev->Style.Pat.pLeft = pNode;
            }
            else
            {
               pPrev->Style.Pat.pRight = pNode;
            }

            pPrev = pSrch->Style.Pat.pDownToMe;
            if (pPrev != NCS_MTREE_NODE_NULL)
            {
               if (pPrev->Style.Pat.pLeft == pSrch)
               {
                  pPrev->Style.Pat.pLeft = pNode;
               }
               else
               {
                  pPrev->Style.Pat.pRight = pNode;
               }
            }

            pNode->Style.Pat.pDownToMe     = pPrev;
            pNode->Style.Pat.pLessSpecific = pSrch;  /* It becomes 2nd in list. */
            pNode->Style.Pat.pMoreSpecific = NCS_MTREE_NODE_NULL;

            pSrch->Style.Pat.pMoreSpecific = pNode;

            /* The old head-of-list is no longer in the REAL patricia Tree. */
            pSrch->Style.Pat.pLeft =
            pSrch->Style.Pat.pRight =
            pSrch->Style.Pat.pDownToMe =
            NCS_MTREE_NODE_NULL;

            /* Special case for the root node. */
            if (pTree->Style.Pat.pRoot == pSrch)
            {
               pTree->Style.Pat.pRoot = pNode;  /* Become the root node. */
            }

            /* pNode is fully installed. */
         }
         return pNode;
      }  /* if (pSrch->KeyBitSize < KeyBitSize) */

      do
      {
         if (pSrch->KeyBitSize == KeyBitSize)
         {
            /* Found an exact match! */
            return pSrch;
         }
         if ((pSrch->Style.Pat.pLessSpecific == NCS_MTREE_NODE_NULL) ||
             (pSrch->Style.Pat.pLessSpecific->KeyBitSize < KeyBitSize)
            )
         {
            break;
         }
         pSrch = pSrch->Style.Pat.pLessSpecific;
      } while (TRUE);

      /* We must insert a new node AFTER pSrch. */

      if ((pNode = (*pFunc)(pKey, KeyBitSize, Cookie)) != NCS_MTREE_NODE_NULL)
      {
         /* The caller has conveniently supplied us with a new one to add.
         * The caller has filled its pKey and KeyBitSize fields.
         * Initialize its patricia fields now!
         */
         pNode->Style.Pat.Bit       = pSrch->Style.Pat.Bit;
         pNode->Style.Pat.pRight    =
         pNode->Style.Pat.pLeft     =
         pNode->Style.Pat.pDownToMe =
         NCS_MTREE_NODE_NULL;
         pNode->Style.Pat.pLessSpecific = pSrch->Style.Pat.pLessSpecific;
         pNode->Style.Pat.pMoreSpecific = pSrch;

         pSrch->Style.Pat.pLessSpecific = pNode;

         if (pNode->Style.Pat.pLessSpecific != NCS_MTREE_NODE_NULL)
         {
            pNode->Style.Pat.pLessSpecific->Style.Pat.pMoreSpecific = pNode;
         }
         /* New node is fully installed. */
      }
      return pNode;
   }  /* if (exact match) */

   /* If we get here, this will become a new node in the patricia tree.  */

   if ((pNode = (*pFunc)(pKey, KeyBitSize, Cookie)) != NCS_MTREE_NODE_NULL)
   {
      register int  Bit;
      /* The caller has conveniently supplied us with a new one to add.
      * The caller has filled its pKey and KeyBitSize fields.
      * Initialize its patricia fields now!
      */
      Bit = m_XMTREE_KEY_FIRST_DIFF(pSrch->pKey, pKey, pTree->Cfg.KeyOctetSize);

      /* Search for the correct insert point. 
       * Maybe we don't need to start all the way at the root. 
       */
      if (Bit < (int)pSrch->Style.Pat.Bit)
      {
         pSrch = pTree->Style.Pat.pRoot;  /* Insert point is 'above' us in the tree*/
      }

      do 
      {
         pPrev = pSrch;
         if (m_XMTREE_BIT_IS_SET(pKey, pSrch->Style.Pat.Bit,pTree->Cfg.KeyOctetSize))
         {
            pSrch = pSrch->Style.Pat.pRight;
         }
         else
         {
            pSrch = pSrch->Style.Pat.pLeft;
         }

         if (pSrch->Style.Pat.Bit <= pPrev->Style.Pat.Bit)
         {
            /* Found the insert point.
             * It is below 'pPrev' and 'pPrev' is below 'pSrch'
             */
            break;
         }
         if (Bit < (int)pSrch->Style.Pat.Bit)
         {
            /* Found the insert point.
            * It is below 'pPrev' and 'pPrev is ABOVE 'pSrch' 
            */
            pSrch->Style.Pat.pDownToMe = pNode;
            break;
         }
      } while (TRUE);

      pNode->Style.Pat.pDownToMe = pPrev;

      pNode->Style.Pat.Bit = (short)Bit;

      pNode->Style.Pat.pLessSpecific =
      pNode->Style.Pat.pMoreSpecific =
      NCS_MTREE_NODE_NULL;
      if (m_XMTREE_BIT_IS_SET(pKey, Bit,pTree->Cfg.KeyOctetSize))
      {
         pNode->Style.Pat.pLeft = pSrch;

         pNode->Style.Pat.pRight = pNode;
      }
      else
      {
         pNode->Style.Pat.pLeft = pNode;

         pNode->Style.Pat.pRight = pSrch;
      }

      if (m_XMTREE_BIT_IS_SET(pKey, pPrev->Style.Pat.Bit,pTree->Cfg.KeyOctetSize))
      {
         pPrev->Style.Pat.pRight = pNode;
      }
      else
      {
         pPrev->Style.Pat.pLeft = pNode;
      }

   }

   return pNode;
}

/******************************************************************
 * Function:  xmtree_del()
 * Purpose:   removes a node from the best-match tree.
 ******************************************************************/
void
xmtree_del(NCS_MTREE      *const pTree,
              NCS_MTREE_NODE *const pNode)
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
   if ((pTemp = pNode->Style.Pat.pMoreSpecific) != NCS_MTREE_NODE_NULL)
   {
      /* This guy isn't even in the patricia tree. 
      * Just de-link him.  Sweet!
      */
      if ((pTemp->Style.Pat.pLessSpecific = pNode->Style.Pat.pLessSpecific)
          != NCS_MTREE_NODE_NULL)
      {
         pNode->Style.Pat.pLessSpecific->Style.Pat.pMoreSpecific = 
             pNode->Style.Pat.pMoreSpecific;
      }
      return;
   }
   if ((pTemp = pNode->Style.Pat.pLessSpecific) != NCS_MTREE_NODE_NULL)
   {
      /* A little more complicated.
      * The node to be removed is part of the patricia tree, but it
      * is the head of the linked list of equal-key nodes, and there is
      * another one (pSrch) to take its place in the tree.
      */
      pTemp->Style.Pat.Bit = pNode->Style.Pat.Bit;

      pTemp->Style.Pat.pLeft = pNode->Style.Pat.pLeft;
      pTemp->Style.Pat.pRight = pNode->Style.Pat.pRight;
      if (m_XMTREE_BIT_IS_SET(pNode->pKey, pNode->Style.Pat.Bit,pTree->Cfg.KeyOctetSize))
      {
         pNext = pNode->Style.Pat.pRight;
         if (pNext == pNode)
         {
            pTemp->Style.Pat.pRight = pTemp;
            pNext = NCS_MTREE_NODE_NULL;
         }
      }
      else
      {
         pNext = pNode->Style.Pat.pLeft;
         if (pNext == pNode)
         {
            pTemp->Style.Pat.pLeft = pTemp;
            pNext = NCS_MTREE_NODE_NULL;
         }
      }
      if (pNext != NCS_MTREE_NODE_NULL)
      {
         do
         {
            if (m_XMTREE_BIT_IS_SET(pNode->pKey, pNext->Style.Pat.Bit,pTree->Cfg.KeyOctetSize))
            {
               pNextLeg = &pNext->Style.Pat.pRight;
            }
            else
            {
               pNextLeg = &pNext->Style.Pat.pLeft;
            }
            pNext = *pNextLeg;
            if (pNext == pNode)
            {
               /* We've found the pointer which is pointing 'up' to pNode. */
               *pNextLeg = pTemp;
               break;  /* from 'do' */
            }
         } while (TRUE);
      }

      /* Adjust the node (if any) which is pointing down to us */
      if ((pTemp->Style.Pat.pDownToMe = pNode->Style.Pat.pDownToMe) != NCS_MTREE_NODE_NULL)
      {
         if (pTemp->Style.Pat.pDownToMe->Style.Pat.pLeft == pNode)
         {
            pTemp->Style.Pat.pDownToMe->Style.Pat.pLeft = pTemp; 
         }
         else
         {
            pTemp->Style.Pat.pDownToMe->Style.Pat.pRight = pTemp; 
         }
      }
      else
      {
         /* Special cases for removal of root node. (0.0.0.0/0, for example) */
         pTree->Style.Pat.pRoot = pTemp;  /* New root! */
         pTemp->Style.Pat.pRight = pTemp;
      }

      /* Set up-pointers of any nodes below us.  */
      if (pNode->Style.Pat.pLeft->Style.Pat.Bit > pNode->Style.Pat.Bit)
      {
         pNode->Style.Pat.pLeft->Style.Pat.pDownToMe = pTemp;
      }
      if (pNode->Style.Pat.pRight->Style.Pat.Bit > pNode->Style.Pat.Bit)
      {
         pNode->Style.Pat.pRight->Style.Pat.pDownToMe = pTemp;
      }

      pTemp->Style.Pat.pMoreSpecific = NCS_MTREE_NODE_NULL;

      /* 'Node' is safely out of the patricia tree. */
      return;
   }

   /* We need to actually remove the node and reorganize the pat tree. */
   pPrev = pNode->Style.Pat.pDownToMe;
   pLegDownToNode = ((pPrev->Style.Pat.pLeft == pNode) ? 
                     &pPrev->Style.Pat.pLeft :
                     &pPrev->Style.Pat.pRight);
   pPrevLeg = pLegDownToNode;
   pNext = pNode;

   /* Keep going 'down' until we find the leg which points back to pNode. */
   do 
   {
      UpWentRight = m_XMTREE_BIT_IS_SET(pNode->pKey, 
          pNext->Style.Pat.Bit,pTree->Cfg.KeyOctetSize);
      pNextLeg    = ((UpWentRight) ?
                      &pNext->Style.Pat.pRight :
                      &pNext->Style.Pat.pLeft);
      pTemp = *pNextLeg;

      if (pTemp == pNode)
      {
         break;
      }

      if (pTemp->Style.Pat.Bit < pNext->Style.Pat.Bit)
      {
         /* Inconsistency in pat tree?   Panic? */
         return;
      }

      /* loop again */
      pPrev    = pNext;
      pNext    = pTemp;
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
   pNext->Style.Pat.Bit = pNode->Style.Pat.Bit;  /* It gets the 'bit' value of the evacuee. */
   pNode->Style.Pat.Bit = 0x7fff;     /* Extremely big */
   *pLegDownToNode      = pNext;
   pNext->Style.Pat.pDownToMe = pNode->Style.Pat.pDownToMe;

   *pPrevLeg = ((UpWentRight) ?
                pNext->Style.Pat.pLeft :
                pNext->Style.Pat.pRight);
   if ((*pPrevLeg)->Style.Pat.Bit > pPrev->Style.Pat.Bit)
   {
      (*pPrevLeg)->Style.Pat.pDownToMe = pPrev;
   }
   pNext->Style.Pat.pRight = pNode->Style.Pat.pRight;
   if (pNext->Style.Pat.pRight->Style.Pat.Bit > pNext->Style.Pat.Bit)
   {
      pNext->Style.Pat.pRight->Style.Pat.pDownToMe = pNext;
   }
   pNext->Style.Pat.pLeft = pNode->Style.Pat.pLeft;
   if (pNext->Style.Pat.pLeft->Style.Pat.Bit > pNext->Style.Pat.Bit)
   {
      pNext->Style.Pat.pLeft->Style.Pat.pDownToMe = pNext;
   }
}



/******************************************************************
 * Function:  xmtree_best()
 * Purpose:   Finds best-match (i.e. longest) matching prefix
 *******************************************************************/
NCS_MTREE_NODE* 
xmtree_best(NCS_MTREE *const pTree,
               const uns32 *const pKey,
               short      KeyBitSize,
               int  *const   pMoreSpecificExists)
{
   char          NewTarget[NCS_XMTREE_MAX_KEY_BYTE];   


   NCS_MTREE_NODE *pSrch;
   NCS_MTREE_NODE *pPrev;
   NCS_MTREE_NODE *pSrchSave;
   int           Dummy = 0;
   int           firstMoreSpecExist =0;   
   char pTarget[NCS_XMTREE_MAX_KEY_BYTE];


   /*copy the pKey value to the temp variable*/
   memcpy ((void *)pTarget, (void *)pKey, pTree->Cfg.KeyOctetSize);

start_searching_best:

   if (Dummy == 1)
   {
       firstMoreSpecExist = *pMoreSpecificExists;
   }

   *pMoreSpecificExists = FALSE;  /* Assume not. */
   pSrch                = pTree->Style.Pat.pRoot;



   do
   {
      pPrev = pSrch;
      if (m_XMTREE_BIT_IS_SET(pTarget, pSrch->Style.Pat.Bit,pTree->Cfg.KeyOctetSize))
      {
         pSrch = pSrch->Style.Pat.pRight;
      }
      else
      {
         pSrch = pSrch->Style.Pat.pLeft;
      }

      if (pSrch->Style.Pat.Bit <= pPrev->Style.Pat.Bit)
      {
          if ((pSrch->KeyBitSize > KeyBitSize) &&
             (!m_XMTREE_KEY_MASK_CMP(pTarget, pSrch->pKey, KeyBitSize, 
               pTree->Cfg.KeyOctetSize)))
         {
            *pMoreSpecificExists = TRUE;
         }

         if (pSrch == pPrev)
         {
            pPrev = pSrch->Style.Pat.pDownToMe;
            pSrchSave = pSrch;
         }
         else
         {
            pPrev = pPrev->Style.Pat.pDownToMe;
            pSrchSave = NCS_MTREE_NODE_NULL;
         }

         do
         {
                         
             if ((pSrch->KeyBitSize <= KeyBitSize) &&
                 (!m_XMTREE_KEY_MASK_CMP(pTarget, pSrch->pKey,
                    pSrch->KeyBitSize, pTree->Cfg.KeyOctetSize)))
             {
               /* This is it! */

               /* Special case here... 
               * It's possible we have not visited any of the more-specific keys...
               */
               if ((pSrchSave != NCS_MTREE_NODE_NULL) &&
                   (pSrch == pSrchSave) &&
                   (pSrch->KeyBitSize <= KeyBitSize)
                  )
               {
                  if (m_XMTREE_BIT_IS_SET(pTarget, pSrchSave->Style.Pat.Bit,
                      pTree->Cfg.KeyOctetSize))
                  {
                     pSrchSave = pSrch->Style.Pat.pLeft;  /* go the OPPOSITE WAY */
                  }
                  else
                  {
                     pSrchSave = pSrch->Style.Pat.pRight;  /* go the OPPOSITE WAY */
                  }

                  if ((pSrchSave->Style.Pat.Bit > pSrch->Style.Pat.Bit) &&
                      (!m_XMTREE_KEY_MASK_CMP(pTarget, pSrchSave->pKey, 
                       KeyBitSize,pTree->Cfg.KeyOctetSize)))
                  {
                     *pMoreSpecificExists = TRUE;
                  }
               }
               
               if (Dummy != 0) 
                   *pMoreSpecificExists = firstMoreSpecExist;
               return pSrch;
            }
            pSrch = pSrch->Style.Pat.pLessSpecific;
         } while (pSrch != NCS_MTREE_NODE_NULL);

         /* Get out of this loop, and start looking for less-specific
               * matches.
               */
         goto Lookback;  /* I got a headache trying to eliminate this!  (S.A.) */
      }

      /* Maybe we can continue to go down, maybe not.
       * Does the target match all bit postions up to
       * (but not including) 'Bit'?
       */

      if (m_XMTREE_KEY_MASK_CMP(pTarget,
                                    pSrch->pKey,
                                    pSrch->Style.Pat.Bit, pTree->Cfg.KeyOctetSize))
      {
         /* There's no point in going forward. */
         break;

      }

      if ((pSrch->KeyBitSize > KeyBitSize) &&
          (!m_XMTREE_KEY_MASK_CMP(pTarget, pSrch->pKey,
                                    KeyBitSize, pTree->Cfg.KeyOctetSize))
         )
      {
         *pMoreSpecificExists = TRUE;
      }
   } while (TRUE);


   if (pPrev == NCS_MTREE_NODE_NULL)
   {

      if (Dummy != 0) 
        *pMoreSpecificExists = firstMoreSpecExist;
      return NCS_MTREE_NODE_NULL;
   }

   while (pSrch->KeyBitSize >= pPrev->Style.Pat.Bit)
   {
      if ((pSrch->KeyBitSize <= pSrch->Style.Pat.Bit) &&
          (pSrch->KeyBitSize <= KeyBitSize) &&
          (!m_XMTREE_KEY_MASK_CMP(pTarget, pSrch->pKey, pSrch->KeyBitSize,
          pTree->Cfg.KeyOctetSize))
         )
      {
         if (Dummy != 0) 
            *pMoreSpecificExists = firstMoreSpecExist;
         return pSrch;
      }
      pSrch = pSrch->Style.Pat.pLessSpecific;
      if (pSrch == NCS_MTREE_NODE_NULL)
      {
         break;
      }
   } 

   /*
    * Try backing up and trying a less-specific mask.
    */

   Lookback:

   while (pPrev != NCS_MTREE_NODE_NULL)
   {
      if (m_XMTREE_BIT_IS_SET(pTarget, pPrev->Style.Pat.Bit,pTree->Cfg.KeyOctetSize))
      {
         pSrch = pPrev->Style.Pat.pRight;
      }
      else
      {
         pSrch = pPrev->Style.Pat.pLeft;
      }

      if (KeyBitSize > pSrch->Style.Pat.Bit)
      {
         KeyBitSize = (short)((int)pSrch->Style.Pat.Bit);
      }

      while (KeyBitSize > pPrev->Style.Pat.Bit)
      {
         if (!m_XMTREE_KEY_MASK_CMP(pTarget, pSrch->pKey, KeyBitSize, 
             pTree->Cfg.KeyOctetSize))
         {


             m_XMTREE_GEN_MASK(NewTarget,KeyBitSize,pTree->Cfg.KeyOctetSize);
             m_XMTREE_AND_KEYS(NewTarget,pTarget,NewTarget,pTree->Cfg.KeyOctetSize);

            if (m_XMTREE_KEY_CMP(NewTarget, pTarget, pTree->Cfg.KeyOctetSize) != 0)
            {
               memcpy ((void *)pTarget, (void *)NewTarget, pTree->Cfg.KeyOctetSize);        
               Dummy++ ;
               goto start_searching_best;
            }
         }
         KeyBitSize -= 1;
      }

      pPrev = pPrev->Style.Pat.pDownToMe;
   }

   if (Dummy != 0) 
    *pMoreSpecificExists = firstMoreSpecExist;
   return NCS_MTREE_NODE_NULL;
}

/*******************************************************************
 * Function:  xmtree_nextbest
 * Purpose:   Given a MNODE, find the next-best matching prefix
 *            in the tree.  Used to find alternate matching entries.
 *******************************************************************/
NCS_MTREE_NODE*
xmtree_nextbest(NCS_MTREE *const pTree,
                   const NCS_MTREE_NODE *const pBetterNode)
{
   char  *Target;   
   int   MaxBitSize;
   int   Dummy;

   if (pBetterNode->Style.Pat.pLessSpecific != NCS_MTREE_NODE_NULL)
   {
      /* Don't return the fake node. */
      if (pBetterNode->Style.Pat.pLessSpecific != &pTree->Style.Pat.Root)
      {
         return pBetterNode->Style.Pat.pLessSpecific;
      }
      return NCS_MTREE_NODE_NULL;
   }

   Target = (char *)pBetterNode->pKey;
   MaxBitSize = ((int)pBetterNode->KeyBitSize) - 1;

   /* Find the least-specific bit which is '1'.  
    * We know there is at least one. 
    */
   while (!m_XMTREE_BIT_IS_SET(Target, MaxBitSize,pTree->Cfg.KeyOctetSize))
   {
      MaxBitSize--;
   }
   
   /*m_XMTREE_CLEAR_BIT(Target,MaxBitSize);   Clear that bit. */

   return (xmtree_best(pTree, (const uns32 *const)Target, (short)MaxBitSize, &Dummy));
}

/*********************************************************************
 * Function:  xmtree_next()
 * Purpose:   Finds 'next' key.
 ***********************************************************************/
NCS_MTREE_NODE *
xmtree_next(NCS_MTREE               *const pTree,
               const uns32 *const   pTarget,
               short                    KeyBitSize)
{

   char  Target[NCS_XMTREE_MAX_KEY_BYTE] = {0};

   NCS_MTREE_NODE *pSrch;
   NCS_MTREE_NODE *pPrev;
   char *pKey = (char *)pTarget;

   if (pKey == NULL)   /* Start from beginning? */
   {
      KeyBitSize = -1;
   }
   else
   {
      memcpy ((void *)Target, (void *)pKey, pTree->Cfg.KeyOctetSize);
   }

   pSrch = pTree->Style.Pat.pRoot;

   if(!pSrch)
      return NCS_MTREE_NODE_NULL;

   do
   {
      pPrev = pSrch;

      if (m_XMTREE_BIT_IS_SET(Target, pSrch->Style.Pat.Bit,pTree->Cfg.KeyOctetSize))
      {
         pSrch = pSrch->Style.Pat.pRight;
      }
      else
      {
         pSrch = pSrch->Style.Pat.pLeft;
      }

      if (pSrch->Style.Pat.Bit <= pPrev->Style.Pat.Bit)
      {
          if ((m_XMTREE_KEY_CMP(Target,pSrch->pKey,pTree->Cfg.KeyOctetSize) <= 0) &&
              (!m_XMTREE_KEY_MASK_CMP(Target, pSrch->pKey, 1 + pPrev->Style.Pat.Bit,
              pTree->Cfg.KeyOctetSize)))
          
         {
            /* HOORAY!  We're almost done!  
             * Find the last one which is greater.
             */
            if (m_XMTREE_KEY_CMP(Target,pSrch->pKey,pTree->Cfg.KeyOctetSize) != 0)
            {
               KeyBitSize = -1;
            }

            if (pSrch->KeyBitSize > KeyBitSize)
            {
               while ((pSrch->Style.Pat.pLessSpecific != NCS_MTREE_NODE_NULL) &&
                      (pSrch->Style.Pat.pLessSpecific->KeyBitSize  > KeyBitSize)
                     )
               {
                  pSrch = pSrch->Style.Pat.pLessSpecific;
               }

               return pSrch;
            }
            /* We found a match on 'target' but there are no (more) keys with
             * length greater than 'Target'.
             */
         }

backup:

         KeyBitSize = -1;

         do
         {

            if (pSrch == pPrev->Style.Pat.pLeft)
            {
               /* We went left */
               /* We went left to get here */
               if (pPrev->Style.Pat.Bit < 0)
               {
                  return NCS_MTREE_NODE_NULL;
               }
               pSrch = pPrev;

               m_XMTREE_MASK_KEY(Target, pSrch->pKey, pSrch->Style.Pat.Bit, pTree->Cfg.KeyOctetSize);

               /*generating the mask for the given number of bits*/            
               /*set the particular bit */
               m_XMTREE_SET_BIT(Target, (1 + pSrch->Style.Pat.Bit), 
                   pTree->Cfg.KeyOctetSize);

               break;
            }
            else
            {
               /* We went right to get here */
                
               if (pPrev->Style.Pat.Bit <= 0)
               {
                  return NCS_MTREE_NODE_NULL;
               }

                m_XMTREE_INVERT_MASK_AND_OR_KEY(Target, pPrev->pKey, pPrev->Style.Pat.Bit, 
                    pTree->Cfg.KeyOctetSize);
                m_XMTREE_INC_KEY(Target, Target, pTree->Cfg.KeyOctetSize);
                          
               /* Wraparound to 0 would cause infinite loop */
               if (m_XMTREE_IS_KEY_ZERO(Target, pTree->Cfg.KeyOctetSize) == TRUE) 
               {
                  return NCS_MTREE_NODE_NULL;
               }

               pSrch = pPrev;
               pPrev = pSrch->Style.Pat.pDownToMe;

               if (!m_XMTREE_KEY_MASK_CMP(Target, pSrch->pKey, 
                   pSrch->Style.Pat.Bit, pTree->Cfg.KeyOctetSize))
                   break;
            }

         } while (TRUE);

      }  /* if (pSrch->Bit <= pPrev->Bit) */
      else
      {


         /* We're still going 'down'... but make sure we haven't gone down 
          * too far. */
         if (m_XMTREE_KEY_MASK_CMP(Target, pSrch->pKey, pSrch->Style.Pat.Bit, 
             pTree->Cfg.KeyOctetSize))
         {
             char NewTarget[NCS_XMTREE_MAX_KEY_BYTE] = {0};
                          
             m_XMTREE_MASK_KEY(NewTarget, pSrch->pKey, pSrch->Style.Pat.Bit, 
                 pTree->Cfg.KeyOctetSize);

            if (m_XMTREE_KEY_CMP(Target, NewTarget, pTree->Cfg.KeyOctetSize) > 0)
            {
               goto backup;
            }

            memcpy (Target, NewTarget, pTree->Cfg.KeyOctetSize);
            
            KeyBitSize = -1;
         }
      }
   } while (TRUE);

}

/*********************************************************************
 * Function:  xmtree_get_next_key()
 * Purpose:   Finds 'next' key.
 ***********************************************************************/
NCS_MTREE_NODE *
xmtree_get_next_key(NCS_MTREE               *const pTree,
               const uns32 *const   pKey,
               uns32                maskLen,
               short                    KeyBitSize)
{
    char pSearchKey[NCS_XMTREE_MAX_KEY_BYTE];
    char pInputKey[NCS_XMTREE_MAX_KEY_BYTE];
    NCS_MTREE_NODE *pSearchNode;

    pSearchNode = xmtree_next (pTree, pKey, KeyBitSize);
    while (pSearchNode != NULL)
    {
       m_XMTREE_MASK_KEY (pSearchKey, pSearchNode->pKey, maskLen, 
               pTree->Cfg.KeyOctetSize);
       m_XMTREE_MASK_KEY (pInputKey, pKey, maskLen, 
               pTree->Cfg.KeyOctetSize);
       if ( m_XMTREE_KEY_CMP (pSearchKey, pInputKey, 
                   pTree->Cfg.KeyOctetSize) == 0)
       {
           return pSearchNode;
       }

        pSearchNode = xmtree_next (pTree, pSearchNode->pKey,
           pSearchNode->KeyBitSize);
        
    }
    return NULL;
}

static const struct
{
   unsigned int (*pInitFunc)(NCS_MTREE *const pTree);
   void         (*pDestFunc)(NCS_MTREE *const pTree);
   NCS_MTREE_NODE* (*pFindAddFunc)(NCS_MTREE         *const pTree,
                                  const uns32 *const       pKey,
                                  const short             KeyBitSize,
                                  NCS_MTREE_NODE *         (*pFunc)(const void* pKey, 
                                                                   short KeyBitSize,
                                                                   void *Cookie),
                                  void             *const Cookie);
   void         (*pDelFunc)(NCS_MTREE *const pTree,
                            NCS_MTREE_NODE *const pNode);
   NCS_MTREE_NODE* (*pBestFunc)(NCS_MTREE *const pTree,
                               const uns32 *const  pKey,
                               short      KeyBitSize,
                               int  *const   pMoreSpecificExists);
   NCS_MTREE_NODE* (*pNextBestFunc)(NCS_MTREE *const pTree,
                                   const NCS_MTREE_NODE *const pBetterNode);
   NCS_MTREE_NODE* (*pNextFunc)(NCS_MTREE          *const pMTree,
                               const uns32 *const  pKey,
                               short        KeyBitSize);
   uns16           MaxKeyOctetSize;

} XmtreeActions=
{
    xmtree_init,
    xmtree_dest,
    xmtree_findadd,
    xmtree_del,
    xmtree_best,
    xmtree_nextbest,
    xmtree_next,
    NCS_XMTREE_MAX_KEY_BYTE,
};

/************************************************************
 * Function:  ncs_xmtree_init()
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
unsigned int
ncs_xmtree_init(NCS_MTREE                 *const pTree,
              const NCS_MTREE_CFG       *const pCfg)
{
   if (((unsigned int) pCfg->Style) >= NCS_MTREE_STYLE_MAX)
   {
      return NCSCC_RC_FAILURE;
   }

   if (pCfg == NULL)
      return NCSCC_RC_FAILURE;

   if (  (pCfg->KeyOctetSize < 1) || 
         (pCfg->KeyOctetSize > XmtreeActions.MaxKeyOctetSize) ||
         (pCfg->MaxKeyBitSize < 1) ||
         (pCfg->MaxKeyBitSize > 8 * pCfg->KeyOctetSize)
      )
   {
      return NCSCC_RC_FAILURE;
   }

   pTree->Cfg = *pCfg;

   /* Go to style-specific handler */

   return ((*XmtreeActions.pInitFunc)(pTree));

}

/***************************************************************
 * Function:  ncs_xmtree_destroy()
 * Purpose:   Frees a 'tree' and all subsequent structures.
 *            If the tree was not empty, it will remove all nodes.
 *
 * Arguments: struct ncs_mtree*:  a value returned from ncs_mtree_create()
 * 
 * Returns:   nothing.
 *****************************************************************/

void
ncs_xmtree_destroy(NCS_MTREE *const pTree)
{
   (*XmtreeActions.pDestFunc)(pTree);
}


/*********************************************************************
 * Function:  XmtreePrivateCallback()
 * Purpose:   Call back function, after adding the node to the xmtree
 * Argument:  const void*: key pointer
 *            short: key size
 *            void*: node pointer
 * Returns : NCS_MTREE_NODE *: node pointer after adding to the xmtree
 ***********************************************************************/
static NCS_MTREE_NODE *
XmtreePrivateCallback(const void* pKey,
                short       KeyBitSize,
                void*       pNode)
{
   return ((NCS_MTREE_NODE*)pNode);
}

/*****************************************************************
 * Function:  ncs_xmtree_add()
 * Purpose:   adds a node with a specified prefix to the best-match tree.
 *
 * Arguments: NCS_MTREE*:  tree in which to insert
 *            NCS_MNODE*:  with pKey and KeyBitSize prefilled.
 *
 * Returns:  NCSCC_RC_SUCCESS:  all went okay
 *           anything else:    node not inserted
 ******************************************************************/

unsigned int
ncs_xmtree_add(NCS_MTREE      *const pTree,
             NCS_MTREE_NODE *const pNode)
{
   NCS_MTREE_NODE *pRetNode;

   pRetNode = ncs_xmtree_find_add(pTree,
                                pNode->pKey,
                                pNode->KeyBitSize,
                                XmtreePrivateCallback,
                                pNode);
   if (pRetNode == pNode)
   {
      return NCSCC_RC_SUCCESS;
   }
   else
   {
      return NCSCC_RC_FAILURE;
   }
}

/*******************************************************************
 * Function:  ncs_xmtree_find_add()
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
 * Returns:    NCS_MTREE_NODE*:  NULL means 'not found'.
 *******************************************************************/
NCS_MTREE_NODE *
ncs_xmtree_find_add(NCS_MTREE         *const pTree,
                  const void *const       pKey,
                  const short             KeyBitSize,
                  NCS_MTREE_NODE *         (*pFunc)(const void* pKey, 
                                                   short KeyBitSize,
                                                   void *Cookie),
                  void             *const Cookie)
{
   return ((*XmtreeActions.pFindAddFunc)(pTree, pKey, KeyBitSize, pFunc, Cookie));
}


/******************************************************************
 * Function:  ncs_xmtree_del()
 * Purpose:   removes a node from the best-match tree.
 *
 * Arguments:NCS_MTREE*:  tree from which to remove.
 *           NCS_MTREE_NODE*:  as returned from ncs_mtree_add()
 *
 * Returns:  nothing.
 ******************************************************************/
void
ncs_xmtree_del(NCS_MTREE      *const pTree,
             NCS_MTREE_NODE *const pNode)
{
   (*XmtreeActions.pDelFunc)(pTree, pNode);
}

/*******************************************************************
 * Function:  ncs_xmtree_find_nextbest
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
NCS_MTREE_NODE *
ncs_xmtree_find_nextbest(NCS_MTREE *const pTree,
                       const NCS_MTREE_NODE *const pBetterNode)
{
   return ((*XmtreeActions.pNextBestFunc)(pTree, pBetterNode));
}

/******************************************************************
 * Function:  ncs_xmtree_find_best()
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
NCS_MTREE_NODE *
ncs_xmtree_find_best(NCS_MTREE *const pTree,
                   const void *const  pKey,
                   uns16      KeyBitSize,
                   int  *const   pMoreSpecificExists)
{
   return ((*XmtreeActions.pBestFunc)(pTree, 
                                                       pKey, 
                                                       KeyBitSize, 
                                                       pMoreSpecificExists));
}

/*********************************************************************
 * Function:  ncs_xmtree_next()
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
NCS_MTREE_NODE *
ncs_xmtree_next(NCS_MTREE               *const pTree,
              const void *const       pKey,
              short                   KeyBitSize)
{
   return ((*XmtreeActions.pNextFunc)(pTree, pKey, KeyBitSize));
}
#else 
 extern int dummy;
#endif /* (NCS_MTREE_PAT_STYLE_SUPPORTED != 0) */



/*end of actual Extended m-tree API*/


