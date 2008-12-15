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
FILE NAME: IFSVIFAP.H
DESCRIPTION: Function prototypes and data structures for allocating interface 
             index. This is the target service implementation.
******************************************************************************/

#ifndef IFSVIFAP_H
#define IFSVIFAP_H

#include "ncsdlib.h"
#include "ncs_lib.h"
#include "ifsv_papi.h"

#define m_IFD_IFAP_INIT(info) ifd_ifap_init((NCSCONTEXT)info)
#define m_IFD_IFAP_DESTROY(info) ifd_ifap_destroy((NCSCONTEXT)info)
#define m_IFD_IFAP_IFINDEX_ALLOC(slo_port_map,cb) ifd_ifap_ifindex_alloc(slo_port_map,(NCSCONTEXT)cb)
#define m_IFD_IFAP_IFINDEX_FREE(ifindex,cb) ifd_ifap_ifindex_free(ifindex,(NCSCONTEXT)cb)

#define m_IFAP_IFPOOL_ALLOC (IFAP_IFINDEX_FREE_POOL*)m_NCS_MEM_ALLOC(sizeof(IFAP_IFINDEX_FREE_POOL),NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_IFAP, 1)
#define m_IFAP_IFPOOL_FREE(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_IFAP, 1)

/*****************************************************************************
 * Data structure used to store information used by IFAP.
 *****************************************************************************/
typedef struct ifap_info
{
   NCS_IFSV_IFINDEX     max_ifindex;     /* Max Interface present, this should be moved to target service - TBD */   
   NCS_DB_LINK_LIST     free_if_pool;    /* Free ifindex pool maintained here, this should be moved to the target service area. (IAPS) - TBD */
} IFAP_INFO;

/*****************************************************************************
 * Data structure used to store the free ifindex which can be allocated.  
 *****************************************************************************/
typedef struct ifap_port_free_pool
{
   NCS_DB_LINK_LIST_NODE   q_node;
   NCS_IFSV_IFINDEX        if_index;
} IFAP_IFINDEX_FREE_POOL;

/*****************************************************************************
 * Data structure used to send information related to interface indexes, which   * are allocated and which are free, to Standby IfD in the cold/warm sync.
 *****************************************************************************/
typedef struct ifap_info_list_a2s
{
  NCS_IFSV_IFINDEX     max_ifindex; /* Max Interface present */
  uns32                num_free_ifindex; /* Number of free if indexes. */
  NCS_IFSV_IFINDEX     *free_list; /*Pointer containing the free if indexes.*/ 
} IFAP_INFO_LIST_A2S;

/******* Function Prototypes */
uns32 ifd_ifap_ifindex_alloc (NCS_IFSV_SPT_MAP *spt_map, NCSCONTEXT info);
uns32 ifd_ifap_ifindex_free (NCS_IFSV_IFINDEX ifindex, NCSCONTEXT info);
uns32 ifd_ifap_init(NCSCONTEXT info);
uns32 ifd_ifap_destroy(NCSCONTEXT info);
uns32 ifd_ifap_max_num_free_ifindex_get (uns32 *msg);
uns32 ifd_ifap_ifindex_list_get (uns8 *msg);
uns32 ifd_ifap_max_num_ifindex_and_num_free_ifindex_get(uns32 *max_ifindex, uns32 *num_free_ifindex); 
uns32 ifd_ifap_ifindex_free_find(uns32 ifindex);


#endif /* IFSVIFAP_H*/
