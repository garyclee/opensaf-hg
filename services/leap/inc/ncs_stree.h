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

  This module contains declarations pertaining to a NCS_S[imple]TREE Database.

****************************************************************************** */

/*
 * Module Inclusion Control...
 */
#ifndef NCS_STREE_H
#define NCS_STREE_H

#include "ncssysfpool.h"

/** The NCS Simple TREE Database Entry structure...
 **/
typedef struct ncs_stree_entry
{

  struct ncs_stree_entry *peer;
  struct ncs_stree_entry *child;
  unsigned int           key;
  NCSCONTEXT              result;  
} NCS_STREE_ENTRY;



/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                         ANSI FUNCTION PROTOTYPES

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/****************************************************************************/
/**                             ncsstree.c                                   */
/****************************************************************************/

EXTERN_C LEAPDLL_API NCSCONTEXT ncs_stree_lookup(NCS_STREE_ENTRY*, uns8*, uns32);
EXTERN_C LEAPDLL_API uns32     ncs_stree_add   (NCS_STREE_ENTRY**,uns8*,uns8,NCSCONTEXT);
EXTERN_C LEAPDLL_API uns32     ncs_stree_del   (NCS_STREE_ENTRY**,uns8*,uns8);
EXTERN_C LEAPDLL_API uns32     ncs_stree_release_db (NCS_STREE_ENTRY**);
EXTERN_C LEAPDLL_API uns32     ncs_stree_create_db  (NCS_STREE_ENTRY**);
EXTERN_C LEAPDLL_API NCS_STREE_ENTRY *ncs_stree_alloc_entry (void);
EXTERN_C LEAPDLL_API void          ncs_stree_free_entry (NCS_STREE_ENTRY*);
EXTERN_C LEAPDLL_API unsigned int  ncs_stree_lock_stree(void);
EXTERN_C LEAPDLL_API unsigned int  ncs_stree_unlock_stree(void);


/****************************************************************************/
/**                            MACROS                                       */
/****************************************************************************/


#define m_STREE_LOCK_STREE   ncs_stree_lock_stree()

#define m_STREE_UNLOCK_STREE   ncs_stree_unlock_stree()

/***************************************************************************/
/**                                                                        */
/**   Multiple lock tree support                                           */
/**                                                                        */
/***************************************************************************/

EXTERN_C LEAPDLL_API NCSCONTEXT ncs_mltree_lookup     (NCS_LOCK* lock, NCS_STREE_ENTRY*, uns8*, uns32);
EXTERN_C LEAPDLL_API uns32     ncs_mltree_add        (NCS_LOCK* lock, NCS_STREE_ENTRY**,uns8*,uns8,NCSCONTEXT);
EXTERN_C LEAPDLL_API uns32     ncs_mltree_del        (NCS_LOCK* lock, NCS_STREE_ENTRY**,uns8*,uns8);
EXTERN_C LEAPDLL_API uns32     ncs_mltree_release_db (NCS_LOCK* lock, NCS_STREE_ENTRY**);
EXTERN_C LEAPDLL_API uns32     ncs_mltree_create_db  (NCS_LOCK* lock, NCS_STREE_ENTRY**);

EXTERN_C LEAPDLL_API unsigned int  ncs_mltree_lock_mltree(NCS_LOCK* lock);
EXTERN_C LEAPDLL_API unsigned int  ncs_mltree_unlock_mltree(NCS_LOCK* lock);
#define m_MLTREE_LOCK_MLTREE(lock)     ncs_mltree_lock_mltree(lock)
#define m_MLTREE_UNLOCK_MLTREE(lock)   ncs_mltree_unlock_mltree(lock)


#endif

