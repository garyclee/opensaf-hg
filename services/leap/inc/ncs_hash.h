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

  This is the generic hash library, which can be used by any of the module
  which needs to use hashing data strucutre.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#ifndef _NCS_HASH_H
#define _NCS_HASH_H

#define m_NCS_HASH_DEFAULT_KEY_OFFSET  (1)
#define m_NCS_HASH_DEFAULT_HASH_SIZE   (16)
#define m_NCS_HASH_SERVICE_INDEX (1)
 
/* This is temporarly used service, It might change */
#define NCS_SERVICE_ID_HASH_LIB (0)

#define m_NCS_HASH_ALLOC(size) m_NCS_MEM_ALLOC(size, NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_HASH_LIB,     \
                                               m_NCS_HASH_SERVICE_INDEX)
#define m_NCS_HASH_FREE(ptr)  m_NCS_MEM_FREE(ptr, NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_HASH_LIB,     \
                                               m_NCS_HASH_SERVICE_INDEX)
#define m_NCS_HASH_MEM_CPY memcpy
#define m_NCS_MEM_SET memset

/* Macros for Hash Library API's. This Macros will be moved to the new
 * portable file location later
 */

#define m_NCS_HASH_INIT(intiTable) ncs_hash_init(intiTable)
#define m_NCS_HASH_KEY_GENERATE(hashTable, dataPtr) \
                         ncs_hash_key_generate(hashTable, dataPtr)
#define m_NCS_HASH_INSERT_DATA(hashTable,key,dataPtr) \
                         ncs_hash_insert_data(hashTable,key,dataPtr)
#define m_NCS_HASH_KEY_LOOKUP(hashTable,key,dataPtr) \
                         ncs_hash_key_lookup(hashTable,key,dataPtr)
#define m_NCS_HASH_KEY_GET_NEXT(hashTable,key,dataPtr) \
                         ncs_hash_key_get_next(hashTable,key,dataPtr)
#define m_NCS_HASH_KEY_DELETE(hashTable,key,dataPtr) \
                         ncs_hash_key_delete(hashTable,key,dataPtr)
#define m_NCS_HASH_DELETE_TABLE(hashTable,freeUserData) \
                         ncs_hash_delete_table(hashTable,freeUserData)



/*At present the maximum number of hash key is 2 pow 32*/
typedef unsigned int NCS_HASH_KEY; 

/*
 *structure for doubly link list. This should be at the begining of all
 *user defined data structures.
 */
typedef struct doubly_link_list
{
 struct doubly_link_list *pNext;
 struct doubly_link_list *pPrev;
}NCS_HASH_DB_LINK_LIST, **NCS_DB_LINK_PTR;

typedef int (*NCS_HASH_KEY_GEN)(void *pDataPtr);
typedef void *(*NCS_HASH_KEY_LOOKUP)(NCS_HASH_DB_LINK_LIST *pLinkPtr, 
         void *pDataKey);
typedef int (*NCS_HASH_KEY_INSERT)(NCS_HASH_DB_LINK_LIST *pLinkPtr, 
          void *pDataPtr);
typedef NCS_HASH_DB_LINK_LIST *(*NCS_HASH_KEY_DELETE)(NCS_HASH_DB_LINK_LIST 
             *pLinkPtr, void *pDataKey);

/*structure for hash table*/
typedef struct hash_main_table 
{
 NCS_DB_LINK_PTR     ppHashTablePtr;
 uns32              sizeOfHashTable;
 uns32              hashKeyOffset;
 NCS_HASH_KEY_GEN    pHashKeyProc;
 NCS_HASH_KEY_INSERT pHashKeyInsertProc;
 NCS_HASH_KEY_LOOKUP pHashKeyLookupProc;
 NCS_HASH_KEY_DELETE pHashKeyDeleteProc;
}NCS_HASH_TABLE;
 

/*function prototypes - User API's*/
NCS_HASH_TABLE * ncs_hash_init (NCS_HASH_TABLE *pHashInitTable);
NCS_HASH_KEY ncs_hash_key_generate (NCS_HASH_TABLE *pHashTable, void *pDataPtr);
int ncs_hash_insert_data (NCS_HASH_TABLE *pHashTable, NCS_HASH_KEY hashKey, void *pDataPtr);
void *ncs_hash_key_lookup (NCS_HASH_TABLE *pHashTable, NCS_HASH_KEY hashKey, void *pDataKey);
void *ncs_hash_key_get_next (NCS_HASH_TABLE *pHashTable, NCS_HASH_KEY *hashKey, void *pDataPtr);
int ncs_hash_key_delete (NCS_HASH_TABLE *pHashTable, NCS_HASH_KEY hashKey, void *pDataKey);
int ncs_hash_delete_table (NCS_HASH_TABLE *pHashTable, NCS_BOOL freeUserData);

int 
hash_cbk_insert_data (NCS_HASH_DB_LINK_LIST *pLinkPtr, void *pDataPtr);

#endif /*_NCS_HASH_H*/



