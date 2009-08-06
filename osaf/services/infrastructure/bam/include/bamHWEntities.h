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

  This module defines Data Structures related to HardWare Validation BOM.
  
******************************************************************************
*/

#ifndef BAMHWENTITIES_H
#define BAMHWENTITIES_H

#include <SaHpi.h>
#include "bam.h"
#include "ncs_bam_avm.h"

typedef struct bam_ent_deploy_desc {
	NCS_PATRICIA_NODE tree_node;
	char desc_name[NCS_MAX_INDEX_LEN];
	char ent_name[NCS_MAX_INDEX_LEN];
	char parent_ent_name[NCS_MAX_INDEX_LEN];
	uns8 location;
	char ncs_node_name[NCS_MAX_INDEX_LEN];
	char ent_path[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	char depends_on_for_act[NCS_MAX_INDEX_LEN];
	uns8 isActivationSourceNCS;
	uns8 netBoot;
	char tftpServIp[NCS_MAX_INDEX_LEN];
	char label1Name[NCS_MAX_INDEX_LEN];
	char label1FileName[NCS_MAX_INDEX_LEN];
	char label2Name[NCS_MAX_INDEX_LEN];
	char label2FileName[NCS_MAX_INDEX_LEN];
	char preferredLabel[NCS_MAX_INDEX_LEN];
	SaHpiEntityTypeT ent_type;

} BAM_ENT_DEPLOY_DESC;

typedef struct bam_ent_deploy_desc_list_node {
	BAM_ENT_DEPLOY_DESC *node;
	struct bam_ent_deploy_desc_list_node *next;
} BAM_ENT_DEPLOY_DESC_LIST_NODE;

/* External function declarations */
EXTERN_C SaHpiEntityTypeT get_entity_type_from_text(char *);
EXTERN_C NCS_BOOL bam_is_entity_valid(BAM_ENT_DEPLOY_DESC *, char *);
EXTERN_C void ent_path_from_type_location(BAM_ENT_DEPLOY_DESC *, char *);
EXTERN_C void bam_clear_and_destroy_deploy_tree(void);
EXTERN_C void bam_clear_and_destroy_deploy_list(void);
EXTERN_C void bam_add_deploy_ent_list(NCS_BAM_CB *, BAM_ENT_DEPLOY_DESC *);

#endif
