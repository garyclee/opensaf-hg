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

  This module is the include file for handling Availability Directors 
  component service Instance structure and its relationship structures.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_CSI_H
#define AVD_CSI_H

#include <avd_comp.h>

/* The attribute value structure for the CSIs. */
typedef struct avd_csi_attr_tag {
	NCS_AVSV_ATTR_NAME_VAL name_value;	/* attribute name & value */
	struct avd_csi_attr_tag *attr_next;	/* the next attribute in the list of attributes in the CSI. */
} AVD_CSI_ATTR;

/* Availability directors Component service in.stance structure(AVD_CSI):
 * This data structure lives in the AvD and reflects data points
 * associated with the CSI on the AvD.
 */
typedef struct avd_csi_tag {
	NCS_PATRICIA_NODE tree_node;	/* key will be the CSI name */

	SaNameT name;
	SaNameT saAmfCSType;
	SaNameT **saAmfCSIDependencies;

	uns32 rank;		/* The rank of the CSI in the SI 
				 * Checkpointing - Sent as a one time update.
				 */

	AVD_SI *si;		/* SI encompassing this csi */

	uns32 num_attributes;	/* The number of attributes in the list. */
	AVD_CSI_ATTR *list_attributes;	/* list of all the attributes of this CSI. */

	NCS_DB_LINK_LIST pg_node_list;	/* list of nodes on which pg is 
					 * tracked for this csi */
	struct avd_csi_tag *si_list_of_csi_next;	/* the next CSI in the list of  component service
							 * instances in the Service instance  */
	struct avd_comp_csi_rel_tag *list_compcsi;	/* The list of compcsi relationship
							 * wrt to this CSI. */
	uns32 compcsi_cnt;	/* no of comp-csi rels */
	struct avd_csi_tag *csi_list_cs_type_next;
	struct avd_cstype *cstype;
} AVD_CSI;

typedef struct avd_cstype {
	NCS_PATRICIA_NODE tree_node;	/* key is name */
	SaNameT name;		/* name of the CSType */
	SaStringT *saAmfCSAttrName;
	AVD_CSI *list_of_csi;
} avd_cstype_t;

/* This data structure lives in the AvD and reflects relationship
 * between the component and CSI on the AvD.
 */
typedef struct avd_comp_csi_rel_tag {

	AVD_CSI *csi;		/* CSI to which this relationship with
				 * component exists */
	AVD_COMP *comp;		/* component to which this relationship
				 * with CSI exists */
	struct avd_su_si_rel_tag *susi;	/* bk ptr to the su-si rel */
	struct avd_comp_csi_rel_tag *susi_csicomp_next;	/* The next element in the list w.r.t to
							 * susi relationship structure */
	struct avd_comp_csi_rel_tag *csi_csicomp_next;	/* The next element in the list w.r.t to
							 * CSI structure */
} AVD_COMP_CSI_REL;

/* Finds CSI structure from the database.*/
EXTERN_C AVD_CSI *avd_csi_find(const SaNameT *csi_name);

/* creates and adds compCSI structure to the list of compCSI in SUSI. */
EXTERN_C AVD_COMP_CSI_REL *avd_compcsi_create(AVD_CL_CB *cb, struct avd_su_si_rel_tag *susi, AVD_CSI *, AVD_COMP *);
/* deletes and frees all compCSI structure from
 * the list of compCSI in SUSI.
 */
EXTERN_C uns32 avd_compcsi_delete(AVD_CL_CB *cb, struct avd_su_si_rel_tag *susi, NCS_BOOL ckpt);

extern SaAisErrorT avd_cstype_config_get(void);
extern SaAisErrorT avd_csi_config_get(const SaNameT *si_name, AVD_SI *si);

EXTERN_C void avd_si_add_csi_list(AVD_CL_CB *cb, AVD_CSI *csi);
EXTERN_C void avd_si_del_csi_list(AVD_CL_CB *cb, AVD_CSI *csi);
extern void avd_csi_constructor(void);

#endif
