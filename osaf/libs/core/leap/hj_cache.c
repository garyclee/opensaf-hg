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

  MODULE NAME:      ncs_cache.c

  REVISION HISTORY:

  Date     Version  Name          Description
  -------- -------  ------------  --------------------------------------------
  07-10-97 1.00A    H&J (BAF)     Original

..............................................................................

  DESCRIPTION:   This module contains routines that operate on a
                 generic cache.  Each cache can be tailored with
   specific information but these are the generic
   routines that will operate on the cache.

..............................................................................

  FUNCTIONS INCLUDED in this module:

*******************************************************************************
*/

#include "ncs_opt.h"		/* Compile time options           */
#include "gl_defs.h"		/* General definitions            */
#include "ncs_osprm.h"
#include "ncs_cache.h"		/* NCS Cache Facility header file */
#include "ncs_tmr.h"

#if (NCS_CACHING == 1)

/*****************************************************************************

  PROCEDURE NAME:    ncs_create_cache

  DESCRIPTION:       This function creates the cache.

  ARGUMENTS:
   hash_size:   Size of the hash table (it must be 2**n)
 num_entries: Number of possible entries in the cache table.
     This will allow you to preallocate the entries
     of a table in one chunk of memory.
 size_entry:  Size of an individual entry.
 hash_rtn:    Routine to be used to find hash entries.
 create_rtn:  Routine to be used to create hash entries.
 delete_rtn:  Routine to be used to delete hash entries.

  RETURNS:
 CACHE_CTXT:    value of CACHE_CTXT_NULL is returned if
                       create fails

  NOTES:

*****************************************************************************/

CACHE_CTXT
ncs_create_cache(uns16 hash_size, uns16 num_entries, uns16 size_entry,
		 HASH_FUNC_PTR hash_rtn, CREATE_FUNC_PTR create_rtn, DELETE_FUNC_PTR delete_rtn)
{
	CACHE *cache;
	uns16 i;

	if ((cache = m_MMGR_ALLOC_CACHE) == CACHE_CTXT_NULL) {
		return CACHE_CTXT_NULL;
	}

	if ((cache->hash_tbl = m_MMGR_ALLOC_HASH_TBL(hash_size)) == (CACHE_ENTRY *)0) {
		m_MMGR_FREE_CACHE(cache);
		return CACHE_CTXT_NULL;
	}

	m_CREATE_CACHE_LOCK(cache);
	m_LOCK_CACHE(cache);

	for (i = 0; i < hash_size; i++) {
		cache->hash_tbl[i].next = CACHE_ENTRY_NULL;
		cache->hash_tbl[i].bucket_num = i;
	}

	cache->size_entry = size_entry;
	cache->num_entries = num_entries;
	cache->hash_func = hash_rtn;
	cache->hash_size = hash_size;
	cache->create_func = create_rtn;
	cache->delete_func = delete_rtn;

	/* The free entries and the entry size must be filled in outside this */
	/* routine.  It is not necessary to preallocate the cache entries.    */

	cache->free_entries = (CACHE_ENTRY *)0;

	/* Allow for the preallocation of cache entries */

	if (m_NCS_CACHE_PREALLOC_ENTRIES(cache) == NCSCC_RC_SUCCESS) {
		m_UNLOCK_CACHE(cache);
		return cache;
	} else {
		m_UNLOCK_CACHE(cache);
		return CACHE_CTXT_NULL;
	}
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_delete_cache

  DESCRIPTION:       This function deletes the cache.

  ARGUMENTS:
   cache:       Pointer to allocated cache

  RETURNS:
 NCSCC_RC_SUCCESS:    if entry exists in Ingress cache
        NCSCC_RC_OUT_OF_MEM: not enough memory to allocate cache

  NOTES:

*****************************************************************************/

unsigned int ncs_delete_cache(CACHE_CTXT cache)
{
	CACHE_ENTRY *mom, *son;
	uns16 i;

	m_LOCK_CACHE(cache);

	for (i = 0; i < cache->hash_size; i++) {
		mom = &cache->hash_tbl[i];
		while (mom->next != CACHE_ENTRY_NULL) {
			son = mom->next;
			mom->next = son->next;
			cache->delete_func(son);
		}
	}

	/* Allow for freeing of preallocated block */
	m_NCS_CACHE_FREE_PREALLOC_ENTRIES(cache);

	m_MMGR_FREE_HASH_TBL(cache->hash_tbl);
	m_DELETE_CACHE_LOCK(cache);
	m_MMGR_FREE_CACHE(cache);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_find_cache_entry

  DESCRIPTION:       This function invokes this cache's hash routine
                     to determine if the entry exists.

  ARGUMENTS:
   cache:     Pointer to the Cache
 arg(s):    Arguments to hash function

  RETURNS:
        pointer to entry or
 CACHE_ENTRY_NULL:   if entry does not exist

  NOTES:

*****************************************************************************/

CACHE_ENTRY *ncs_find_cache_entry(CACHE *cache, void *arg1, void *arg2, void *arg3, void *arg4)
{
	CACHE_ENTRY *mom;
	CACHE_ENTRY *ret;

	m_LOCK_CACHE(cache);

	if (cache->hash_func(cache, &mom, arg1, arg2, arg3, arg4) == NCSCC_RC_SUCCESS)
		ret = mom->next;
	else
		ret = CACHE_ENTRY_NULL;

	m_UNLOCK_CACHE(cache);

	return (ret);

}

/*****************************************************************************

  PROCEDURE NAME:    ncs_add_cache_entry

  DESCRIPTION:       This function adds the new entry into the cache table.

  ARGUMENTS:
    cache:   Pointer to the Cache
   mom:         Pointer to the parent cache entry
 new_entry:   Pointer to new cache entry

  RETURNS:
 NCSCC_RC_SUCCESS:          Done!

  NOTES:

*****************************************************************************/

unsigned int ncs_add_cache_entry(CACHE *cache, CACHE_ENTRY *mom, CACHE_ENTRY *new_entry)
{
	m_LOCK_CACHE(cache);

	new_entry->next = mom->next;	/* insert new entry */
	mom->next = new_entry;

	new_entry->bucket_num = mom->bucket_num;

	m_UNLOCK_CACHE(cache);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_move_cache_entry

  DESCRIPTION:       This function removes the passed in entry from the hash
                     table and then re-inserts it in a new spot.  This
                     function is useful when one of the hashing parameters
                     changes.

  ARGUMENTS:
    cache:   Pointer to the Cache
 new_entry:   Pointer to cache entry to be moved
 arg(s):      Arguments to hash function

  RETURNS:
 NCSCC_RC_SUCCESS:          If move was successful
    NCSCC_RC_FAILURE:          If unable to find cache entry in original spot
                                or unable to put entry into new spot

  NOTES:

*****************************************************************************/

unsigned int ncs_move_cache_entry(CACHE *cache, CACHE_ENTRY *ce, void *arg1, void *arg2, void *arg3, void *arg4)
{
	CACHE_ENTRY *temp;
	CACHE_ENTRY *mom;

	m_LOCK_CACHE(cache);

	/* Remove entry from it's current bucket */
	if ((temp = cache->hash_tbl[ce->bucket_num].next) == ce) {
		cache->hash_tbl[ce->bucket_num].next = ce->next;
	} else {
		while (temp) {
			if (temp->next == ce)
				break;
			temp = temp->next;
		}
		/* if temp is zero, we didn't find entry */
		if (temp)
			temp->next = ce->next;
		else {
			m_UNLOCK_CACHE(cache);
			return (unsigned int)m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
	}

	/* Put entry into new bucket */
	/*
	 ** Get the mom by doing a hash lookup.  If we find an entry the
	 ** entry we are trying to put in would be ambiguous, so return failure.
	 */
	if (cache->hash_func(cache, &mom, arg1, arg2, arg3, arg4) == NCSCC_RC_SUCCESS)
		return (unsigned int)m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	/* Have insertion point in mom so add entry */
	ncs_add_cache_entry(cache, mom, ce);

	m_UNLOCK_CACHE(cache);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_find_or_create_cache_entry

  DESCRIPTION:       This function invokes this cache's hash routine
                     to determine if the entry exists.  If the entry
      does not exist, the cache's create function is
      called and the new entry is added to the cache.

  ARGUMENTS:
   cache:     Pointer to the Cache
 arg(s):    Arguments to hash function

  RETURNS:
        pointer to entry or
 CACHE_ENTRY_NULL:   if entry does not exist

  NOTES:

*****************************************************************************/

CACHE_ENTRY *ncs_find_or_create_cache_entry(CACHE *cache, void *arg1, void *arg2, void *arg3, void *arg4)
{
	CACHE_ENTRY *mom;
	CACHE_ENTRY *me;
	CACHE_ENTRY *ret;

	m_LOCK_CACHE(cache);

	if (cache->hash_func(cache, &mom, arg1, arg2, arg3, arg4) == NCSCC_RC_SUCCESS)
		ret = mom->next;
	else {
		if (cache->create_func(cache, &me, arg1, arg2, arg3, arg4) == NCSCC_RC_SUCCESS) {
			ncs_add_cache_entry(cache, mom, me);
			ret = me;
		} else
			ret = CACHE_ENTRY_NULL;
	}
	m_UNLOCK_CACHE(cache);
	return ret;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_delete_cache_entry

  DESCRIPTION:       This function removes the entry from the cache table.
      This function then calls the delete function associated
      with the cache to free the entry.

  ARGUMENTS:
   cache:       Pointer to the Cache
 mom:   Pointer to parent of old cache entry ( if known )
 old_entry:   Pointer to old cache entry

  RETURNS:
    NCSCC_RC_FAILURE:          entry was not found
 NCSCC_RC_SUCCESS:          Done!

*****************************************************************************/

unsigned int ncs_delete_cache_entry(CACHE *cache, CACHE_ENTRY *mom, CACHE_ENTRY *old_entry)
{
	CACHE_ENTRY *temp;

	m_LOCK_CACHE(cache);

	if ((mom == (CACHE_ENTRY *)0) || (mom == old_entry) || (mom->bucket_num != old_entry->bucket_num)) {
		if ((temp = cache->hash_tbl[old_entry->bucket_num].next) == old_entry)
			cache->hash_tbl[old_entry->bucket_num].next = old_entry->next;
		else {
			while (temp) {
				if (temp->next == old_entry)
					break;
				temp = temp->next;
			}
			/* if temp is zero, we didn't find entry */
			if (temp)
				temp->next = old_entry->next;
			else {
				m_UNLOCK_CACHE(cache);
				return (unsigned int)m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
			}
		}
	} else
		mom->next = old_entry->next;	/* remove entry from cache */

	cache->delete_func(old_entry);

	m_UNLOCK_CACHE(cache);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_get_first_cache_entry

  DESCRIPTION:       This routine is used for walking all the entries
                     in the cache.  This routine is called first.

  ARGUMENTS:
   cache:       Pointer to the Cache

  RETURNS:
   CACHE_ENTRY: Pointer to the first entry in the Cache

  NOTES:

*****************************************************************************/

CACHE_ENTRY *ncs_get_first_cache_entry(CACHE *cache)
{
	unsigned int i;
	CACHE_ENTRY *ret;

	m_LOCK_CACHE(cache);

	ret = CACHE_ENTRY_NULL;

	for (i = 0; i < cache->hash_size; i++) {
		if (cache->hash_tbl[i].next != CACHE_ENTRY_NULL) {
			ret = cache->hash_tbl[i].next;
			break;
		}
	}
	m_UNLOCK_CACHE(cache);

	return ret;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_get_next_cache_entry

  DESCRIPTION:       This routine is used for walking all the entries
                     in the cache.  This routine is passed the last entry
       received and returns the next entry in the cache.

  ARGUMENTS:
   cache:       Pointer to the Cache
   entry:       Last entry returned from the cache

  RETURNS:
   CACHE_ENTRY: Next entry in the cache or CACHE_ENTRY_NULL if
              that was the last entry in the cache.

  NOTES:

*****************************************************************************/

CACHE_ENTRY *ncs_get_next_cache_entry(CACHE *cache, CACHE_ENTRY *entry)
{
	unsigned int i;
	CACHE_ENTRY *ret;

	m_LOCK_CACHE(cache);

	if (entry->next == CACHE_ENTRY_NULL) {
		ret = CACHE_ENTRY_NULL;

		for (i = entry->bucket_num + 1; i < cache->hash_size; i++) {
			if (cache->hash_tbl[i].next != CACHE_ENTRY_NULL) {
				ret = cache->hash_tbl[i].next;
				break;
			}
		}
		m_UNLOCK_CACHE(cache);
		return ret;
	} else {
		ret = entry->next;
		m_UNLOCK_CACHE(cache);
		return ret;
	}
}
#else

/*****************************************************************************

  Below are dummy versions of the same functions so that the NT .DLL file
  will be satisfied when the flag configuration is ... (NCS_CACHE != 1)

*****************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:  ncs_create_cache

*****************************************************************************/

CACHE_CTXT
ncs_create_cache(uns16 hash_size, uns16 num_entries, uns16 size_entry,
		 HASH_FUNC_PTR hash_rtn, CREATE_FUNC_PTR create_rtn, DELETE_FUNC_PTR delete_rtn)
{
	USE(hash_size);
	USE(num_entries);
	USE(size_entry);
	USE(hash_rtn);
	USE(create_rtn);
	USE(delete_rtn);

	return CACHE_CTXT_NULL;

}

/*****************************************************************************

  PROCEDURE NAME:    ncs_delete_cache

*****************************************************************************/

unsigned int ncs_delete_cache(CACHE_CTXT cache)
{
	USE(cache);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_find_cache_entry

*****************************************************************************/

CACHE_ENTRY *ncs_find_cache_entry(CACHE *cache, void *arg1, void *arg2, void *arg3, void *arg4)
{
	USE(cache);
	USE(arg1);
	USE(arg2);
	USE(arg3);
	USE(arg4);

	return CACHE_ENTRY_NULL;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_add_cache_entry

*****************************************************************************/

unsigned int ncs_add_cache_entry(CACHE *cache, CACHE_ENTRY *mom, CACHE_ENTRY *new_entry)
{
	USE(cache);
	USE(mom);
	USE(new_entry);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_move_cache_entry

*****************************************************************************/

unsigned int ncs_move_cache_entry(CACHE *cache, CACHE_ENTRY *ce, void *arg1, void *arg2, void *arg3, void *arg4)
{
	USE(cache);
	USE(ce);
	USE(arg1);
	USE(arg2);
	USE(arg3);
	USE(arg4);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_find_or_create_cache_entry

*****************************************************************************/

CACHE_ENTRY *ncs_find_or_create_cache_entry(CACHE *cache, void *arg1, void *arg2, void *arg3, void *arg4)
{
	USE(cache);
	USE(arg1);
	USE(arg2);
	USE(arg3);
	USE(arg4);

	return CACHE_ENTRY_NULL;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_delete_cache_entry

*****************************************************************************/

unsigned int ncs_delete_cache_entry(CACHE *cache, CACHE_ENTRY *mom, CACHE_ENTRY *old_entry)
{
	USE(cache);
	USE(mom);
	USE(old_entry);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_get_first_cache_entry

*****************************************************************************/

CACHE_ENTRY *ncs_get_first_cache_entry(CACHE *cache)
{
	USE(cache);

	return CACHE_ENTRY_NULL;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_get_next_cache_entry

*****************************************************************************/

CACHE_ENTRY *ncs_get_next_cache_entry(CACHE *cache, CACHE_ENTRY *entry)
{
	USE(cache);
	USE(entry);

	return CACHE_ENTRY_NULL;
}

#endif
