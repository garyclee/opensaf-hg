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

 

  REVISION HISTORY:

  Date        Description
  ---------   ----------------------------------------------------------------
  9/14/2000   Fixed ncs_stree_release_db() to check for incoming NULL db to 
              avoid crashes. (Kyle C Quest)
  
..............................................................................

  DESCRIPTION:

  This module contains the routines to add, delete and fetch associated data
  entries from the NCS_STREE_ENTRY database, which is made up of unique octet
  strings (such as ATM Addresses).

..............................................................................

  FUNCTIONS INCLUDED in this module:

  ncs_stree_lookup.....fetch data associated with a target string
  ncs_stree_add........install a target string and associated data into DB
  ncs_stree_del........delete a target string and associated data from DB
  ncs_stree_release_db.Function to clear an entire NCS_STREE_ENTRY DB
  ncs_stree_create_db..Initialize a simple tree DB

******************************************************************************
*/

#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"


#include "ncs_stree.h"
#include "sysf_def.h"




/** Set up NCS_LOCK for all S-TREE databases.
 **/
static NCS_LOCK l_lock;

#define m_LOCAL_LOCK_LEGACY_INIT             
#define m_LOCAL_LOCK_INIT(lock)         m_NCS_LOCK_INIT (lock)
#define m_LOCAL_LOCK(lock,flag)         m_NCS_LOCK (lock,flag)
#define m_LOCAL_UNLOCK(lock,flag )      m_NCS_UNLOCK (lock,flag)
#define m_LOCAL_LOCK_DESTROY(lock)      m_NCS_LOCK_DESTROY (lock)

static int LockUseCount = 0;



/** Utility function to recursively and unconditionally free the Simple
 ** tree elements...
 **/
static void
stree_free_tree (NCS_STREE_ENTRY *stree)
{

  if (stree != NULL)
    {
      stree_free_tree (stree->child);
      stree_free_tree (stree->peer);
      ncs_stree_free_entry (stree);
    }

}



/** Utility function to recursively and conditionally free the simple
 ** tree target elements to delete 
 **/
static void
stree_free_octets( NCS_STREE_ENTRY **anchor, uns8 *trgt, uns8 trgt_len)
{
  NCS_STREE_ENTRY *stree;
  NCS_STREE_ENTRY **pcs;
  
  if (trgt_len == 0)
    return;

  /** Work our way to the bottom of the octet string
   **/
  for (stree = *anchor, pcs = anchor;
       stree != NULL;
       pcs = &stree->peer, stree = stree->peer)
    {

      /** Not a match, continue to peer.
       **/
      if (stree->key != *trgt)
 continue;

      /** Descend even further, if you can.
       **/
      if (stree->child != NULL)
 stree_free_octets (&stree->child, trgt+1,(uns8)(trgt_len-1));

      /** Free octet string entry if no children and (entry does not
       ** represent another (smaller length) octet string OR the entry
       ** represents the last octet struct for this octet string)...
       **/
      if ((stree->child == NULL) &&
   ((stree->result == NULL) || (trgt_len == 1)))
 {
   /** Reattach upper link to this entry's peer.
    ** If this is the first time through this loop, 
    ** then we are attaching the peer node to the parent of 
    ** the node we are freeing.
    ** Otherwise, we are attaching the peer node to the previous
    ** peer.
    **/
   *pcs = stree->peer;

   /** Release the node.
    **/
   ncs_stree_free_entry (stree);
 }
      return;

    }

}




/*****************************************************************************

  PROCEDURE NAME:    ncs_stree_lookup

  DESCRIPTION:

  An NCS_STREE database function to lookup the associated data for a
  given target octet string

  ARGUMENTS:
        NCS_STREE_ENTRY*  ---   Root of a tree to search
        "uns8 *trgt"     ---   target octets (opaque octet stream)
        "uns8 trgt_len"  ---   Length of target octets

  RETURNS:

        An opaque identifier that specifies the result of the lookup
        NULL: Lookup Failure.

  NOTES:

****************************************************************************/
static NCSCONTEXT
ncs_do_tree_lookup (NCS_STREE_ENTRY *anchor, uns8 *trgt, uns32 trgt_len)
{
  void           *result;
  NCS_STREE_ENTRY *stree;
  uns32 lookup_len = 0;

  result = NULL;  /* Init result */

  /** Sanity check request and database anchor
   **/
  if (trgt_len == 0)
    return NULL;

  /** Scan all children at all peer-levels...
   **/
  for (stree = anchor; (stree != NULL);)
    {
      if (stree->key == *trgt)
        {
        lookup_len++;

          /** Found a match...update result.
           **/
   result = stree->result;

          /** If no more to scan, return result...
           **/
          if ((stree->child == NULL) || (lookup_len >= trgt_len))
            {
       return result;
            }

   /** More children to scan...
    **/
   stree = stree->child;
   trgt++;

        }
      else
        {
   /** Terminate search if no hope of finding a match...
    **/
   if (stree->key > *trgt)
            {
       return NULL;
            }

          /** Scan the next peer...
           **/
          stree = stree->peer;
        }
    }

  return result;
}


/*****************************************************************************

  PROCEDURE NAME:    ncs_stree_add

  DESCRIPTION:

  An NCS_STREE  database function to add associated data given a target octet
  string.

  ARGUMENTS:
        NCS_STREE_ENTRY* ---   Root of a tree to add an octet string
        "uns8 *trgt"     ---  target octet string
        "uns8 trgt_len"  ---  length of target octet string
        NCSCONTEXT        ---  associated data

  RETURNS:

        NCSCC_RC_SUCCESS:    Successful.
        NCSCC_RC_FAILURE:    Failure.  (Invalid parameters or Duplicate Entry)
 NCSCC_RC_OUT_OF_MEM: Failure.  (Allocation Failure)

  NOTES:

****************************************************************************/
static uns32
ncs_do_tree_add (NCS_STREE_ENTRY **anchor, 
       uns8 *trgt, 
       uns8 trgt_len, 
       NCSCONTEXT result)
{

  uns8 clen, stree_count, i;
  NCS_STREE_ENTRY *stree, **ppstree, *stree_array[20];
  NCS_STREE_ENTRY *old_stree = NULL;
  NCS_STREE_ENTRY **pold_stree;

  /****************************
 Error Checking
   ****************************/
  
  /** Sanity check request and database anchor
   **/
  if ((trgt_len == 0) || (result==NULL))
    return NCSCC_RC_FAILURE;

  /*******************************************
     Install target octet string into the DB
   ******************************************/
  
  /** Spin down the database tree scanning for re-usable octet string entries...
   **/
  ppstree = anchor;

  for(stree = *ppstree, clen = trgt_len; 
      (clen > 0) && (stree != NULL);  )
    {
       old_stree = stree;  /* used for partial match */ 

      if (stree->key == *trgt)
 {
   /** This level already has an approp entry...
    ** Scan the next level.
    **/
   trgt++;   /* Next octet to match  */
   clen--;   /* Decr residue count   */
   ppstree = &stree->child; /* Save append point    */
   stree = stree->child; /* Move down one level  */
 }
      else
 {
   if (stree->key > *trgt) /* Terminate pointless search */
     break;

   ppstree = &stree->peer; /* Save append point */
   stree = stree->peer;
 }
    }

  /** Deny duplicate entry...
   **/
  if (clen == 0)
    {
    if ((old_stree->result == NULL) &&
        ((old_stree->child != NULL) || (old_stree->peer != NULL)))
       {
       old_stree->result = result;
       return NCSCC_RC_SUCCESS;
       }
    else
      {
      return NCSCC_RC_FAILURE;
      }
    }

  /** Save the "old" octet entry at the append point...
   **/
  pold_stree = ppstree;
  old_stree = *ppstree;


  /** Create new stree entries, if necesary, for remainder of the octet str
   **/
  for (stree_count = 0, i=0; i < clen; i++)
    {
      if ((stree = ncs_stree_alloc_entry()) == NULL)
 {
   /** Allocation failure. Free entries allocated up till now...
    **/
   for (i=0; i < stree_count; i++)
     {
       ncs_stree_free_entry (stree_array[i]);
     }

   /** Restore old link...
    **/
   *pold_stree = old_stree;
   
   return NCSCC_RC_OUT_OF_MEM;

 }

      /** Save failure recovery info...
       **/
      stree_array[i] = stree;
      stree_count++;

      /** Install NCS_STREE_ENTRY entry...
       ** "ppstree" points to the "append" location.
       **/
      if (i == 0)
 stree->peer = old_stree;
      stree->key    = *trgt++;
      *ppstree      = stree;
      ppstree       = &stree->child;
    }
  
  /** For the final stree entry, mark the associated data...
   **/
  stree->result = result;
  
  return NCSCC_RC_SUCCESS;
  
}


/*****************************************************************************

  PROCEDURE NAME:    ncs_stree_del

  DESCRIPTION:

  An NCS_STREE_ENTRY simple tree database function to delete a target octet
  string and the associated data. Notice this function takes no responsibility
  for the memory it takes to house the associated data.

  ARGUMENTS:
        NCS_STREE_ENTRY* ---   Root of a tree to search
        "uns8 *trgt"     ---   target octet string
        "uns8 trgt_len"  ---   Length of octet string

  RETURNS:

        NCSCC_RC_SUCCESS:  Successful.
        NCSCC_RC_FAILURE:  Failure.

  NOTES:

****************************************************************************/
uns32
ncs_stree_del (NCS_STREE_ENTRY **anchor, uns8 *trgt, uns8 trgt_len)
{

  /** for legacy locking paradigm
  **/
  m_LOCAL_LOCK_LEGACY_INIT;   /* for legacy locking paradigm ................*/


  /****************************
 Error Checking
   ****************************/
  
  /** Sanity check request and database anchor
   **/
  if (trgt_len == 0) 
    return NCSCC_RC_FAILURE;

  m_LOCAL_LOCK (&l_lock, NCS_LOCK_WRITE);

  /********************************
     De-Install NCS_STREE_ENTRY
   *******************************/

  if ((anchor != NULL) && (*anchor != NULL))
    stree_free_octets (anchor, trgt, trgt_len);

  m_LOCAL_UNLOCK (&l_lock, NCS_LOCK_WRITE);

  return NCSCC_RC_SUCCESS;

}



/*****************************************************************************

  PROCEDURE NAME:    ncs_stree_release_db

  DESCRIPTION:

  An NCS_STREE_ENTRY simple tree database function to clear a NCS_STREE_ENTRY
  database.

  ARGUMENTS:

        NCS_STREE_ENTRY* ---   Root of a tree to release

  RETURNS:

        NCSCC_RC_SUCCESS       On success.
        NCSCC_RC_FAILURE       On failure.

  NOTES:

****************************************************************************/
uns32
ncs_stree_release_db (NCS_STREE_ENTRY **anchor)
{
  int UseCount;

  if(anchor == NULL)
   return NCSCC_RC_FAILURE;

  /** for legacy locking paradigm
  **/
  m_LOCAL_LOCK_LEGACY_INIT;


  m_LOCAL_LOCK (&l_lock, NCS_LOCK_WRITE);

  /** Clear the simple tree database...
   **/
  stree_free_tree (*anchor);

  *anchor = NULL;

  UseCount = --LockUseCount;  /* decrement use count... */

  m_LOCAL_UNLOCK (&l_lock, NCS_LOCK_WRITE);

  /** free lock resources
   **/
  if (UseCount == 0)
    {
      m_LOCAL_LOCK_DESTROY (&l_lock);
    }

 return NCSCC_RC_SUCCESS;

}


/*****************************************************************************

  PROCEDURE NAME:    ncs_stree_create_db

  DESCRIPTION:

  This function will initialize a NCS_STREE_ENTRY simple tree database.


  ARGUMENTS:

        NCS_STREE_ENTRY* ---   Root of a tree to create

  RETURNS:

        Nothing.

  NOTES:

****************************************************************************/
uns32
ncs_stree_create_db (NCS_STREE_ENTRY **anchor)
{

  /** Create our local lock
   **/
  if (LockUseCount++ == 0)
    {
      m_LOCAL_LOCK_INIT (&l_lock);
    }

  /** Initialize list anchor...
   **/
  *anchor = NULL;

 return NCSCC_RC_SUCCESS;

}



/***************************************************************************/
/**                                                                        */
/**   ncs_stree_lookup() add a tree entry                                      */
/**                                                                        */
/***************************************************************************/
NCSCONTEXT ncs_stree_lookup (NCS_STREE_ENTRY *anchor, uns8 *trgt, uns32 trgt_len)
{
   void  *result;

   /** for legacy locking paradigm
 **/
   m_LOCAL_LOCK_LEGACY_INIT;
   m_LOCAL_LOCK (&l_lock, NCS_LOCK_WRITE);
   
   result = ncs_do_tree_lookup(anchor, trgt, trgt_len);
   
   m_LOCAL_UNLOCK (&l_lock, NCS_LOCK_WRITE);

   return result;
}


/***************************************************************************/
/**                                                                        */
/**   ncs_stree_add() add a tree entry                                      */
/**                                                                        */
/***************************************************************************/
uns32 ncs_stree_add (NCS_STREE_ENTRY **anchor, 
       uns8 *trgt, 
       uns8 trgt_len, 
       NCSCONTEXT result)
{
   uns32 return_code;   

   /** for legacy locking paradigm
 **/
   m_LOCAL_LOCK_LEGACY_INIT;   /* for legacy locking paradigm ................*/

   m_LOCAL_LOCK (&l_lock, NCS_LOCK_WRITE);

   return_code = ncs_do_tree_add(anchor, trgt, trgt_len, result);
     
   m_LOCAL_UNLOCK (&l_lock, NCS_LOCK_WRITE);

   return return_code;
}



/*****************************************************************************

  PROCEDURE NAME:    ncs_stree_alloc_entry

  DESCRIPTION:

  An NCS_STREE_ENTRY simple tree database function to alloc a simple tree entry

  ARGUMENTS:

  RETURNS:
   Newly allocated NCS_STREE_ENTRY  or
 NULL if failure.

  NOTES:

****************************************************************************/
NCS_STREE_ENTRY *
ncs_stree_alloc_entry ()
{
  NCS_STREE_ENTRY *stree;

  if ((stree = (NCS_STREE_ENTRY *)m_MMGR_ALLOC_NCS_STREE_ENTRY) != NULL)
    {
      stree->key    = 0;
      stree->peer   = NULL;
      stree->child  = NULL;
      stree->result = NULL;
    }

  return stree;

}

/*****************************************************************************

  PROCEDURE NAME:    ncs_stree_free_entry

  DESCRIPTION:

  An NCS_STREE_ENTRY simple tree database function to free a simple tree entry

  ARGUMENTS:
        NCS_STREE_ENTRY* ---   entry to free

  RETURNS:

        NCSCC_RC_SUCCESS:  Successful.
        NCSCC_RC_FAILURE:  Failure.

  NOTES:

****************************************************************************/
void
ncs_stree_free_entry (NCS_STREE_ENTRY *stree)
{
  if (stree != NULL)
    m_MMGR_FREE_NCS_STREE_ENTRY(stree);
}



/*****************************************************************************

  PROCEDURE NAME:    ncs_stree_lock_stree

  DESCRIPTION:       allow caller to lock stree database

  ARGUMENTS:

  RETURNS:
        NCSCC_RC_SUCCESS:  Successful.

  NOTES:

****************************************************************************/

unsigned int
ncs_stree_lock_stree (void)
{
  /** for legacy locking paradigm
   **/

  m_LOCAL_LOCK_LEGACY_INIT;   /* for legacy locking paradigm ................*/
 
  m_LOCAL_LOCK (&l_lock, NCS_LOCK_WRITE);

  return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:    ncs_stree_unlock_stree

  DESCRIPTION:       allow caller to unlock stree database

  ARGUMENTS:

  RETURNS:

        NCSCC_RC_SUCCESS:  Successful.

  NOTES:

****************************************************************************/

unsigned int
ncs_stree_unlock_stree(void)
{
  m_LOCAL_UNLOCK (&l_lock, NCS_LOCK_WRITE);

  return NCSCC_RC_SUCCESS;
}


/***************************************************************************/
/**                                                                        */
/**   Multiple lock tree support                                           */
/**                                                                        */
/***************************************************************************/

NCSCONTEXT ncs_mltree_lookup(NCS_LOCK* lock, NCS_STREE_ENTRY* anchor, uns8* trgt, uns32 trgt_len)
{
   void* result = NULL;
   
   m_LOCAL_LOCK (lock, NCS_LOCK_WRITE);

   if(anchor != NULL)
   {
      /* Look up entry with zero-length key*/
      if (trgt_len == 0)
      {
         if(trgt == NULL)
            result = anchor->result;
      }

      /* Look up normal entry */
      else
         result = ncs_do_tree_lookup(anchor->child, trgt, trgt_len);
   }
   
   m_LOCAL_UNLOCK (lock, NCS_LOCK_WRITE);

   return result;
}

/***************************************************************************/
/**                                                                        */
/**   ncs_mltree_add() add a tree entry                                     */
/**                                                                        */
/***************************************************************************/
uns32 ncs_mltree_add(NCS_LOCK* lock, NCS_STREE_ENTRY** anchor,uns8* trgt ,uns8 trgt_len,NCSCONTEXT result)
{
   uns32 return_code = NCSCC_RC_FAILURE;   

   m_LOCAL_LOCK (lock, NCS_LOCK_WRITE);

   if((anchor != NULL) && (*anchor != NULL))
   {
      /* Add entry with zero-length key */
      if(trgt_len == 0)
      {
         /* target must be NULL */
         if(trgt == NULL)
         {
            /* Allow only one entry */
            if((*anchor)->result == NULL && result != NULL)
            {
               (*anchor)->result = result;
               return_code = NCSCC_RC_SUCCESS;
            }
         }
      }

      /* Add normal entry */
      else
         return_code = ncs_do_tree_add(&(*anchor)->child, trgt, trgt_len, result);
   }
   
   m_LOCAL_UNLOCK (lock, NCS_LOCK_WRITE);

   return return_code;
}

/***************************************************************************/
/**                                                                        */
/**   ncs_mltree_del() delete a tree entry                                  */
/**                                                                        */
/***************************************************************************/
uns32 ncs_mltree_del(NCS_LOCK* lock, NCS_STREE_ENTRY** anchor,uns8* trgt,uns8 trgt_len)
{
   m_LOCAL_LOCK (lock, NCS_LOCK_WRITE);

   if((anchor != NULL) && (*anchor != NULL))
   {
      /* Delete entry with zero-length key */
      if(trgt_len == 0)
      {
         if(trgt == NULL)
            (*anchor)->result = NULL;
      }

      /* normal entry */
      else
      {
         stree_free_octets (&(*anchor)->child, trgt, trgt_len);
      }
   }

   m_LOCAL_UNLOCK (lock, NCS_LOCK_WRITE);

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************/
/**                                                                        */
/**   ncs_mltree_release_db() release the tree batabase                     */
/**                                                                        */
/***************************************************************************/
uns32 ncs_mltree_release_db(NCS_LOCK* lock, NCS_STREE_ENTRY** anchor)
{
   if(anchor == NULL)
    return NCSCC_RC_FAILURE;

   m_LOCAL_LOCK (lock, NCS_LOCK_WRITE);

   /** Clear the tree database...  **/
   stree_free_tree (*anchor);
   *anchor = NULL;

   m_LOCAL_UNLOCK (lock, NCS_LOCK_WRITE);

   m_LOCAL_LOCK_DESTROY (lock);

 return NCSCC_RC_SUCCESS;
}

/***************************************************************************/
/**                                                                        */
/**   ncs_mltree_create_db() Create the tree batabase                       */
/**                                                                        */
/***************************************************************************/
uns32 ncs_mltree_create_db(NCS_LOCK* lock, NCS_STREE_ENTRY** anchor)
{
   m_LOCAL_LOCK_INIT (lock);

   /* Create the root node for entry with zero-length key */
   *anchor = ncs_stree_alloc_entry();

 return NCSCC_RC_SUCCESS;
}

/***************************************************************************/
/**                                                                        */
/**   ncs_mltree_lock_mltree() Lock the tree lock                           */
/**                                                                        */
/***************************************************************************/
unsigned int  ncs_mltree_lock_mltree(NCS_LOCK* lock)
{
   m_LOCAL_LOCK (lock, NCS_LOCK_WRITE);
   return NCSCC_RC_SUCCESS;
}

/***************************************************************************/
/**                                                                        */
/**   ncs_mltree_unlock_mltree() Unlock the tree lock                       */
/**                                                                        */
/***************************************************************************/
unsigned int  ncs_mltree_unlock_mltree(NCS_LOCK* lock)
{
   m_LOCAL_UNLOCK(lock, NCS_LOCK_WRITE);
   return NCSCC_RC_SUCCESS;
}

