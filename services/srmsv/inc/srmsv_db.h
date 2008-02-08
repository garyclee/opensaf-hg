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

  MODULE NAME: SRMSV_DB.H

$Header: 
..............................................................................

  DESCRIPTION: This file has the data structure definitions defined for SRMSV
               service will be used by SRMA and SRMND.
  
******************************************************************************/

#ifndef SRMSV_DB_H
#define SRMSV_DB_H

typedef struct ncs_srmsv_mon_aging_bucket
{
    /* void ptr to accomodate for SRMA & SRMND resource monitor data */
    void    *start_rsrc_mon_node; 

    /* Timestamp of this aging bucket when it suppose to get expire.
       (Current time stamp + monitoring rate) */
    time_t  aging_out; 

    /* Ptrs to the prev & next aging bucket in the aging list */
    struct srm_mon_aging_bucket *prev_aging_bucket;
    struct srm_mon_aging_bucket *next_aging_bucket;
} NCS_SRMSV_MON_AGING_BUCKET;

#define  NCS_SRMSV_MON_AGING_BUCKET_NULL  ((NCS_SRMSV_MON_AGING_BUCKET*)0)

#endif /* SRMSV_DB_H */



