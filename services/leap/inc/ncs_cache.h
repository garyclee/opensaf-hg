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

  MODULE NAME:      ncs_cache.h


  REVISION HISTORY:

  Date     Version  Name          Description
  -------- -------  ------------  --------------------------------------------
  07-10-97 1.00A    H&J (BAF)     Original


..............................................................................

  DESCRIPTION:    This file includes declarations necessary to support 
                  the H&J Caching facility.


*******************************************************************************
*/
/*
 * Module Inclusion Control...
 */
#ifndef NCS_CACHE_H_
#define NCS_CACHE_H_

/* partial structure definition to keep compilers happy */
struct cache;
struct cache_entry;

/* typedef of the hashing function that returns the parent of the    */
/* requested entity.                                                 */

typedef unsigned int (*HASH_FUNC_PTR) (struct cache *, struct cache_entry **, 
           void *, void *, void *, void *);

typedef unsigned int (*CREATE_FUNC_PTR)(struct cache *, struct cache_entry **, 
           void *, void *, void *, void *);

typedef unsigned int (*DELETE_FUNC_PTR)(struct cache_entry *); 

/* All cache entries of any type should have a next pointer as their */
/* first element.                                                    */

typedef struct cache_entry
{
  struct cache_entry   *next;    /* for either free list or hash table ptr */
  uns32                 bucket_num;

  /* different values for the cache entry type will exist here */

} CACHE_ENTRY;

typedef struct cache_extent
{
 int mpc_placeholder;
} CACHE_EXTENT;

/* This is a generic cache structure that should be usable throughout */
/* Harris & Jeffries various subsystems.  The cache provides a hash   */
/* table of pointers to cache entries.  Each cache entry must have as */
/* its first element, a next pointer to the next entry.  This cache   */
/* structure will also maintain a free list and linked list of all    */
/* elements in the cache.   The size of the hash table must be 2**n.  */
/* The hash function returns a pointer to the parent of the requested */
/* entry whether the entry exists or not.                             */

typedef struct cache
{
  uns16          hash_size;        /* size of the hash table         */
  uns16          size_entry;       /* size of an entry in this cache */
  uns16    num_entries;      /* num of entries (for pre-alloc) */
  CACHE_ENTRY   *free_entries;     /* list of free entries           */
  CACHE_ENTRY   *hash_tbl;         /* table of ptrs to cache_entries */
  HASH_FUNC_PTR  hash_func;        /* function to find entry parent  */
  CREATE_FUNC_PTR create_func;     /* function to create entry       */
  DELETE_FUNC_PTR delete_func;    /* function to delete entry       */
  CACHE_EXTENT   cache_extent;     /* user definable extension       */
} CACHE;

#define CACHE_CTXT       CACHE *
#define CACHE_CTXT_NULL  ((CACHE *) 0)
#define CACHE_ENTRY_NULL ((CACHE_ENTRY *) 0)

/* Macros for Locking a Cache */

#define m_CREATE_CACHE_LOCK(c)
#define m_LOCK_CACHE(c)
#define m_UNLOCK_CACHE(c)
#define m_DELETE_CACHE_LOCK(c)


/* Prototypes... */

EXTERN_C LEAPDLL_API CACHE_CTXT   ncs_create_cache( uns16 hash_size, uns16 num_entries,
        uns16 size_entry, HASH_FUNC_PTR hash_rtn,
        CREATE_FUNC_PTR create_rtn,
        DELETE_FUNC_PTR delete_rtn);
EXTERN_C LEAPDLL_API unsigned int ncs_delete_cache( CACHE_CTXT cache );
EXTERN_C LEAPDLL_API unsigned int ncs_add_cache_entry( CACHE *cache, CACHE_ENTRY *mom, 
            CACHE_ENTRY *new_entry);
EXTERN_C LEAPDLL_API unsigned int ncs_delete_cache_entry( CACHE *cache, CACHE_ENTRY *mom, 
        CACHE_ENTRY *old_entry);
EXTERN_C LEAPDLL_API CACHE_ENTRY *ncs_find_cache_entry( CACHE *cache, void *arg1, 
      void *arg2, void *arg3, void *arg4);
EXTERN_C CACHE_ENTRY *ncs_find__or_create_cache_entry( CACHE *cache, void *arg1, 
      void *arg2, void *arg3, void *arg4);
EXTERN_C LEAPDLL_API CACHE_ENTRY *ncs_get_first_cache_entry( CACHE_CTXT cache );
EXTERN_C LEAPDLL_API CACHE_ENTRY *ncs_get_next_cache_entry( CACHE_CTXT cache, 
          CACHE_ENTRY *entry );
EXTERN_C LEAPDLL_API CACHE_ENTRY *ncs_find_or_create_cache_entry( CACHE *cache, void *arg1,
      void *arg2, void *arg3, void *arg4);
EXTERN_C LEAPDLL_API unsigned int ncs_move_cache_entry (CACHE *cache, CACHE_ENTRY *ce, 
                        void *arg1, void *arg2, void *arg3, void *arg4);
#endif

