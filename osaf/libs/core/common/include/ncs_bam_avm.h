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

  This file lists definitions for messages between AVM and BAM modules.
  
******************************************************************************
*/

#ifndef NCS_BAM_AVM_H
#define NCS_BAM_AVM_H

#include <SaHpi.h>

#define MAX_POSSIBLE_PARENTS 8
#define MAX_POSSIBLE_LOC_RANGES 12	/* For a 16 slot chassis this is MAX */
#define NCS_MAX_INDEX_LEN 256

/* Message format versions  */
#define AVSV_AVM_BAM_MSG_FMT_VER_1    1

typedef struct ncs_hw_ent_type_desc NCS_HW_ENT_TYPE_DESC;

typedef struct ncs_hw_ent_valid_location {
	uns8 min[MAX_POSSIBLE_LOC_RANGES];
	uns8 max[MAX_POSSIBLE_LOC_RANGES];
} NCS_HW_ENT_VALID_LOCATION;

typedef struct ncs_hw_ent_loc_range {
	char parent_ent[NCS_MAX_INDEX_LEN];
	uns32 length;

	NCS_HW_ENT_VALID_LOCATION valid_location;

} NCS_HW_ENT_LOC_RANGE;

struct ncs_hw_ent_type_desc {
	char entity_name[NCS_MAX_INDEX_LEN];
	uns8 ent_name_length;

	SaHpiEntityTypeT entity_type;

	NCS_BOOL isNode;

	NCS_HW_ENT_LOC_RANGE location_range[MAX_POSSIBLE_PARENTS];
	uns8 num_possible_parents;

	NCS_BOOL is_fru;	/* is FRU Activation Policy Applicable */
	char fru_product_name[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	char fru_product_version[SAHPI_MAX_TEXT_BUFFER_LENGTH];

	NCS_HW_ENT_TYPE_DESC *next;
	/*FaultDomainT          fault_domain; */

};

#define NCS_HW_ENT_TYPE_DESC_NULL ((NCS_HW_ENT_TYPE_DESC*)0x0)

typedef enum {
	NCS_SERVICE_BAM_AVM_SUB_ID_MSG
} NCS_SERVICE_BAM_AVM_SUB_ID;

typedef enum {
	BAM_AVM_HW_ENT_INFO,
	BAM_AVM_CFG_DONE_MSG,
	BAM_AVM_MSG_MAX
} BAM_AVM_MSG_TYPE_T;

/* This message will be same for both BAM and PSR */
typedef struct {
	uns32 check_sum;	/* number of cfg messages sent */
} AVM_CFG_DONE_T;

/* Message structure used by AvM for communication between
 * the active AvM and BAM.
 */
typedef struct {
	BAM_AVM_MSG_TYPE_T msg_type;
	union {
		AVM_CFG_DONE_T msg;
		NCS_HW_ENT_TYPE_DESC *ent_info;
	} msg_info;
} BAM_AVM_MSG_T;

#define BAM_AVM_MSG_NULL ((BAM_AVM_MSG_T *)0x0)

#define m_MMGR_ALLOC_BAM_AVM_MSG (BAM_AVM_MSG_T *)m_NCS_MEM_ALLOC(sizeof(BAM_AVM_MSG_T), \
                                     NCS_MEM_REGION_PERSISTENT,\
                                     NCS_SERVICE_ID_BAM_AVM,\
                                     NCS_SERVICE_BAM_AVM_SUB_ID_MSG)

#define m_MMGR_FREE_BAM_AVM_MSG(p) m_NCS_MEM_FREE(p, \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_BAM_AVM,\
                                     NCS_SERVICE_BAM_AVM_SUB_ID_MSG)

#endif
