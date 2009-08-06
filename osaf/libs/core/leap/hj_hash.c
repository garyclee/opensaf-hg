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
  Hash Library search/add/delete/get-next functions.

******************************************************************************
*/
/** Global Declarations...
 **/
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncssysf_mem.h"
#include "ncs_hash.h"

/*******************************************************************
 * Function:  ncs_hash_init()
 * Purpose:   Initialize the Hash Table, by allocating the number of
 *            buckets entered by the user and initialise all the 
 *            buckets.
 *            Note: While calling this init function, All the Hash Table
 *             Init data structure should be initialized. (i.e) unused 
 *             pointer or variables should be initialized to NULL or 0.
 *
 * Arguments:  NCS_HASH_TABLE*: User Registered Hash Table structure
 *             
 *
 * Returns:    NCS_HASH_TABLE*:  Allocated Hash Table pointer which is
 *                              to be reffered when ever the user  
 *                              needs to access the Hash Table.
 *******************************************************************/
NCS_HASH_TABLE *ncs_hash_init(NCS_HASH_TABLE *pHashInitTable)
{

	NCS_HASH_TABLE *pHashTable;

	if (pHashInitTable->hashKeyOffset == 0) {
		pHashInitTable->hashKeyOffset = m_NCS_HASH_DEFAULT_KEY_OFFSET;
	}
	if (pHashInitTable->sizeOfHashTable == 0) {
		pHashInitTable->hashKeyOffset = m_NCS_HASH_DEFAULT_HASH_SIZE;
	}
	/* if user insert data call back is not register, then add the default
	 * insert data call back
	 */
	if (pHashInitTable->pHashKeyInsertProc == NULL) {
		pHashInitTable->pHashKeyInsertProc = hash_cbk_insert_data;
	}
	pHashTable = (NCS_HASH_TABLE *)m_NCS_HASH_ALLOC(sizeof(NCS_HASH_TABLE));
	/* In the case of memory allocation failure, it returns error */
	if (pHashTable == NULL) {
		return NULL;
	}

	m_NCS_HASH_MEM_CPY(pHashTable, pHashInitTable, sizeof(NCS_HASH_TABLE));
	/*allocate the hash buckets */
	pHashTable->ppHashTablePtr = (NCS_DB_LINK_PTR)m_NCS_HASH_ALLOC(pHashTable->sizeOfHashTable *
								       sizeof(NCS_DB_LINK_PTR));

	/* In the case of memory allocation failure, it returns error */
	if (pHashTable->ppHashTablePtr == NULL) {
		return NULL;
	}
	/*initialize the hash buckets */
	m_NCS_MEM_SET(pHashTable->ppHashTablePtr, 0, (pHashTable->sizeOfHashTable * sizeof(NCS_DB_LINK_PTR)));
	return (pHashTable);
}

/*******************************************************************
 * Function:  ncs_hash_key_generate()
 * Purpose:   Generate a key for the given data using User defined 
 *            Algorithm.
 *
 * Arguments:  NCS_HASH_TABLE*: Hash Table pointer
 *             void *        : Data pointer for which key has to be 
 *                             generated.
 *             
 *
 * Returns:    NCS_HASH_KEY:  Key generated after using Hashing
 *                           algorithm on the Data.
 *******************************************************************/
NCS_HASH_KEY ncs_hash_key_generate(NCS_HASH_TABLE *pHashTable, void *pDataPtr)
{
	if (pHashTable->pHashKeyProc != NULL)
		return (pHashTable->pHashKeyProc(pDataPtr) % pHashTable->sizeOfHashTable);
	else
		return (0);

}

/*******************************************************************
 * Function:  ncs_hash_insert_data()
 * Purpose:   Insert the given Data in the Hash Table, accroding to
 *            the user defined format.
 *
 * Arguments:  NCS_HASH_TABLE*: Hash Table pointer
 *             NCS_HASH_KEY   : Hash Key for the given data
 *             void *        : Data pointer which has to be inserted 
 *                             in the Hash Table.
 *             
 *
 * Returns:    int:  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *******************************************************************/
int ncs_hash_insert_data(NCS_HASH_TABLE *pHashTable, NCS_HASH_KEY hashKey, void *pDataPtr)
{
	uns32 bucketOffset;
	NCS_HASH_DB_LINK_LIST *pDbLinkList;

	if (pHashTable->pHashKeyInsertProc != NULL) {
		bucketOffset = (hashKey / pHashTable->hashKeyOffset);
		/*check for the out of boundry */
		if (bucketOffset >= pHashTable->sizeOfHashTable) {
			return (NCSCC_RC_FAILURE);
		}
		/* calculate the bucket offset and retrive the doubly link list
		 * start pointer 
		 */
		pDbLinkList = (NCS_HASH_DB_LINK_LIST *)(*(pHashTable->ppHashTablePtr + bucketOffset));
		/* There are 2 possible cases in inserting the node
		 * 1. It can be the first entry to the bucket
		 * 2. It can be the multiple instance of that hash key
		 */
		if (pDbLinkList == NULL) {
			*(pHashTable->ppHashTablePtr + bucketOffset) = (NCS_HASH_DB_LINK_LIST *)pDataPtr;
		} else {
			pHashTable->pHashKeyInsertProc(pDbLinkList, pDataPtr);
		}
		return (NCSCC_RC_SUCCESS);
	} else {
		return (NCSCC_RC_FAILURE);
	}
}

/*******************************************************************
 * Function:  ncs_hash_key_lookup()
 * Purpose:   Search for the user input key in the Hash Table.
 *
 * Arguments:  NCS_HASH_TABLE*: Hash Table pointer.
 *             NCS_HASH_KEY   : Hash Key for the data to be searched.
 *             void *        : User defined key to be searched.
 *             
 *
 * Returns:    void * :  Data pointer which has got while doing a
 *                       lookup (or) NULL when the data is not found.
 *******************************************************************/
void *ncs_hash_key_lookup(NCS_HASH_TABLE *pHashTable, NCS_HASH_KEY hashKey, void *pDataKey)
{

	uns32 bucketOffset;
	NCS_HASH_DB_LINK_LIST *pDbLinkList;

	if (pHashTable->pHashKeyLookupProc != NULL) {
		bucketOffset = (hashKey / pHashTable->hashKeyOffset);
		/*check for the out of boundry */
		if (bucketOffset >= pHashTable->sizeOfHashTable) {
			return (NULL);
		}
		/* Finding out the bucket with respect to the hash key */
		pDbLinkList = (NCS_HASH_DB_LINK_LIST *)(*(pHashTable->ppHashTablePtr + bucketOffset));
		return (pHashTable->pHashKeyLookupProc(pDbLinkList, pDataKey));
	} else {
		return (NULL);
	}
}

/*******************************************************************
 * Function:  ncs_hash_key_get_next()
 * Purpose:   Get the next entry from the Hash Table. The intial fetch
 *            will be done with *hashKey = 0 and pDataPtr = NULL.
 *
 * Arguments:  NCS_HASH_TABLE*: Hash Table pointer.
 *             NCS_HASH_KEY*  : Hash Key for the data to be searched.
 *                             This variable acts as both input and 
 *                             output, Once the data is fetched
 *                             to fetch the next data the Hash Key is
 *                             sent back as an output.
 *             void *        : Previous data pointer(If first search
 *                             then the value would be NULL).
 *             
 *
 * Returns:    void * :  Data pointer which has got while doing a
 *                       get-next operation (or)
 *                       NULL when the data is not found.
 *******************************************************************/
void *ncs_hash_key_get_next(NCS_HASH_TABLE *pHashTable, NCS_HASH_KEY *hashKey, void *pDataPtr)
{
	uns32 bucketOffset;
	uns32 hashBucketCount = 0;
	NCS_HASH_DB_LINK_LIST *pDbLinkList;
	NCS_HASH_DB_LINK_LIST *pTempDbLinkList;
	NCS_HASH_DB_LINK_LIST *pUserDbLinkList = (NCS_HASH_DB_LINK_LIST *)pDataPtr;

	/*this is the first iteration of the get next operation */
	if (pDataPtr == NULL) {
		pDbLinkList = (NCS_HASH_DB_LINK_LIST *)(*(pHashTable->ppHashTablePtr));
		/* Traversing the buckets to find out the first entry */
		do {
			hashBucketCount++;
			/* checks for the presence of entry in the hash table or 
			 * it might have reached the last point of the hash table
			 */
			if ((pDbLinkList != NULL) || (hashBucketCount >= pHashTable->sizeOfHashTable)) {
				*hashKey = (hashBucketCount - 1);
				return ((void *)pDbLinkList);
			}
			/*search in the next bucket */

			pDbLinkList = (NCS_HASH_DB_LINK_LIST *)(*(pHashTable->ppHashTablePtr + hashBucketCount));
		} while (TRUE);
	} else {
		bucketOffset = (*hashKey / pHashTable->hashKeyOffset);
		/*check for the out of boundry */
		if (bucketOffset >= pHashTable->sizeOfHashTable) {
			return (NULL);
		}
		pDbLinkList = (NCS_HASH_DB_LINK_LIST *)(*(pHashTable->ppHashTablePtr + bucketOffset));
		/* Traversing the Doubly link list of the given data, to find whether
		 * there is any next data found
		 */
		for (pTempDbLinkList = pDbLinkList;
		     ((pTempDbLinkList != pUserDbLinkList) && (pTempDbLinkList != NULL));
		     pTempDbLinkList = pTempDbLinkList->pNext) ;

		if (pTempDbLinkList == NULL) {
			return (pTempDbLinkList);
		}
		pTempDbLinkList = pTempDbLinkList->pNext;
		if (pTempDbLinkList == NULL) {
			hashBucketCount = (bucketOffset + 1);
			/* since it couldn't able to find the next data in the same bucket
			 * move to the other buckets and find out whether any data is
			 * present or not
			 */
			do {
				if ((pTempDbLinkList != NULL) || (hashBucketCount >= pHashTable->sizeOfHashTable)) {
					*hashKey = (hashBucketCount - 1);
					return ((void *)pTempDbLinkList);
				}
				/*search in the next bucket */
				pTempDbLinkList = (NCS_HASH_DB_LINK_LIST *)
				    (*(pHashTable->ppHashTablePtr + hashBucketCount));
				hashBucketCount++;

			} while (TRUE);
		}
		*hashKey = bucketOffset;
		return (pTempDbLinkList);
	}

}

/*******************************************************************
 * Function:  ncs_hash_key_delete()
 * Purpose:   Delete the node from the Hash Table for the user defined
 *            key.
 *
 * Arguments:  NCS_HASH_TABLE*: Hash Table pointer.
 *             NCS_HASH_KEY   : Hash Key for the data to be deleted.
 *             void *        : User defined key to be deleted.
 *             
 *
 * Returns:    int:  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *******************************************************************/
int ncs_hash_key_delete(NCS_HASH_TABLE *pHashTable, NCS_HASH_KEY hashKey, void *pDataKey)
{
	uns32 bucketOffset;
	NCS_HASH_DB_LINK_LIST *pResult;
	NCS_HASH_DB_LINK_LIST *pDbLinkList;
	NCS_HASH_DB_LINK_LIST *pSecondLinkPtr;

	if (pHashTable->pHashKeyLookupProc != NULL) {
		bucketOffset = (hashKey / pHashTable->hashKeyOffset);
		/*check for the out of boundry */
		if (bucketOffset >= pHashTable->sizeOfHashTable) {
			return (NCSCC_RC_FAILURE);
		}
		pDbLinkList = (NCS_HASH_DB_LINK_LIST *)(*(pHashTable->ppHashTablePtr + bucketOffset));
		if (pDbLinkList != NULL) {
			pSecondLinkPtr = (void *)pDbLinkList->pNext;
			pResult = pHashTable->pHashKeyDeleteProc(pDbLinkList, pDataKey);
			if (pResult == NULL) {
				return (NCSCC_RC_FAILURE);
			}
			/*
			 *the doubly Link list pointer in the bucket has been deleted 
			 *(first element in the bucket)
			 */
			if (pResult == pDbLinkList) {
				*(pHashTable->ppHashTablePtr + bucketOffset) = pSecondLinkPtr;
			}
			return (NCSCC_RC_SUCCESS);
		} else {
			return (NCSCC_RC_FAILURE);
		}
	} else {
		return (NCSCC_RC_FAILURE);
	}
}

/*******************************************************************
 * Function:  ncs_hash_delete_table()
 * Purpose:   Delete the whole Hash Table.
 *
 * Arguments:  NCS_HASH_TABLE*: Hash Table pointer.
 *             NCS_BOOL   : TRUE - if the user want to free the 
 *                         user data.
 *                         FALSE - if user doesn't want to free
 *                         the user data.
 *             
 *
 * Returns:    int:  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *******************************************************************/
int ncs_hash_delete_table(NCS_HASH_TABLE *pHashTable, NCS_BOOL freeUserData)
{
	uns32 tempVar;
	NCS_HASH_DB_LINK_LIST *pDbLinkList;
	NCS_HASH_DB_LINK_LIST *pTempDbLinkList;

	if (freeUserData == TRUE) {
		/*traverese the bucket */
		for (tempVar = 0; tempVar < pHashTable->sizeOfHashTable; tempVar++) {
			pDbLinkList = (NCS_HASH_DB_LINK_LIST *)(*(pHashTable->ppHashTablePtr + tempVar));
			/*traverse the doubly linklist in the respective bucket */
			for (pTempDbLinkList = pDbLinkList; pTempDbLinkList != NULL;
			     pTempDbLinkList = pTempDbLinkList->pNext) {
				m_NCS_HASH_FREE(pTempDbLinkList);
			}
		}
	}
	m_NCS_HASH_FREE(pHashTable->ppHashTablePtr);
	m_NCS_HASH_FREE(pHashTable);
	return NCSCC_RC_SUCCESS;
}

/*******************************************************************
 * Function:  hash_cbk_insert_data()
 * Purpose:   Insert data at the end of the link list. If user wants to 
 *            add in their own format then, they should register the 
 *            function call during initializing.
 *
 * Arguments:  NCS_HASH_DB_LINK_LIST*: Start of the Doubly Link List 
 *                                    pointer.
 *             void *               : Data pointer which has to be 
 *                                    added in the Hash Table.
 *             
 *
 * Returns:    int:  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *******************************************************************/
int hash_cbk_insert_data(NCS_HASH_DB_LINK_LIST *pLinkPtr, void *pDataPtr)
{
	NCS_HASH_DB_LINK_LIST *pTempLinkPtr;
	NCS_HASH_DB_LINK_LIST *pPrevLinkPtr = NULL;
	NCS_HASH_DB_LINK_LIST *pUserData = (NCS_HASH_DB_LINK_LIST *)pDataPtr;

	for (pTempLinkPtr = pLinkPtr; pTempLinkPtr != NULL;
	     pPrevLinkPtr = pTempLinkPtr, pTempLinkPtr = pTempLinkPtr->pNext) ;

	/*This is the first node of the doubly linklist */
	if (pPrevLinkPtr == NULL) {
		pUserData->pPrev = NULL;
		pUserData->pNext = NULL;

	} else {
		pPrevLinkPtr->pNext = (NCS_HASH_DB_LINK_LIST *)pDataPtr;
		pUserData->pPrev = pPrevLinkPtr;
		pUserData->pNext = NULL;
	}

	return NCSCC_RC_SUCCESS;
}
