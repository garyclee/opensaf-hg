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
  service Instance structure and its relationship structures.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_SI_H
#define AVD_SI_H

#include <saAmf.h>
#include <saImm.h>
#include <ncspatricia.h>
#include <app.h>
#include <sg.h>
#include <su.h>
#include <sg.h>
#include <amf_defs.h>
#include <ckpt_msg.h>

/* Enum values defines different SI-SI dependency FSM states. */
typedef enum {
	AVD_SI_NO_DEPENDENCY = 1,
	AVD_SI_SPONSOR_UNASSIGNED,
	AVD_SI_ASSIGNED,
	AVD_SI_TOL_TIMER_RUNNING,
	AVD_SI_READY_TO_UNASSIGN,
	AVD_SI_UNASSIGNING_DUE_TO_DEP,
	AVD_SI_FAILOVER_UNDER_PROGRESS,
	AVD_SI_READY_TO_ASSIGN,
	AVD_SI_DEP_MAX_STATE
} AVD_SI_DEP_STATE;

typedef struct avd_sirankedsu {
	SaNameT suname;
	uint32_t saAmfRank;
	struct avd_sirankedsu *next;
} avd_sirankedsu_t;

/* Availability directors Service Instance structure(AVD_SI):
 * This data structure lives in the AvD and reflects data points 
 * associated with the Service Instance (SI) on the AvD.
 */
typedef struct avd_si_tag {

	NCS_PATRICIA_NODE tree_node;	/* key will be the SI name */
	SaNameT name;

	/******************** B.04 model *************************************************/
	SaNameT saAmfSvcType;
	SaNameT saAmfSIProtectedbySG;
	uint32_t saAmfSIRank;
	char **saAmfSIActiveWeight;
	char **saAmfSIStandbyWeight;
	uint32_t saAmfSIPrefActiveAssignments;  /* only applicable for the N-way active redundancy model */
	uint32_t saAmfSIPrefStandbyAssignments; /* only applicable for the N-way active redundancy model */
	SaAmfAdminStateT saAmfSIAdminState;
	SaAmfAssignmentStateT saAmfSIAssignmentState;
	uint32_t saAmfSINumCurrActiveAssignments;
	uint32_t saAmfSINumCurrStandbyAssignments;
	/******************** B.04 model *************************************************/

	SaToggleState si_switch;	/* The field that indicates if
					 * the SI needs to be Toggled.
					 * Checkpointing - Updated independently.
					 */

	struct avd_sg_tag *sg_of_si;	/* the service group of this SI */
	struct avd_csi_tag *list_of_csi;	/* The list of CSIs in the SI */
	struct avd_si_tag *sg_list_of_si_next;	/* next SI in the SG list of SIs */
	struct avd_su_si_rel_tag *list_of_sisu;	/* the list of su si relationship elements */
	AVD_SI_DEP_STATE si_dep_state;	/* SI-SI dep state of this SI */
	struct avd_spons_si_tag *spons_si_list;
	uint32_t num_dependents; /* number of dependent SIs */
	uint32_t tol_timer_count;
	struct avd_amf_svc_type_tag *svc_type;
	struct avd_si_tag *si_list_svc_type_next;
	struct avd_app_tag *app;
	struct avd_si_tag *si_list_app_next;
	struct avd_sus_per_si_rank_tag *list_of_sus_per_si_rank;
	avd_sirankedsu_t *rankedsu_list_head;
	SaInvocationT invocation;
	
	uint32_t alarm_sent; /* SI unassigned alarm has been sent */
} AVD_SI;

typedef struct avd_amf_svc_type_tag {

	NCS_PATRICIA_NODE tree_node;	/* key will be svc type name */
	SaNameT name;
	char **saAmfSvcDefActiveWeight;
	char **saAmfSvcDefStandbyWeight;
	struct avd_si_tag *list_of_si;

} AVD_SVC_TYPE;

typedef struct {
	NCS_PATRICIA_NODE tree_node;	/* key is name */
	SaNameT name;
	SaUint32T saAmfSvctMaxNumCSIs;

	SaUint32T curr_num_csis;

	struct avd_amf_svc_type_tag *cs_type_on_svc_type;
	struct avd_svc_type_cs_type_tag *cs_type_list_svc_type_next;

} AVD_SVC_TYPE_CS_TYPE;

#define AVD_SI_NULL ((AVD_SI *)0)
#define m_AVD_SI_ACTV_MAX_SU(l_si) (l_si)->saAmfSIPrefActiveAssignments
#define m_AVD_SI_ACTV_CURR_SU(l_si) (l_si)->saAmfSINumCurrActiveAssignments

#define m_AVD_SI_STDBY_MAX_SU(l_si)       (l_si)->saAmfSIPrefStandbyAssignments
#define m_AVD_SI_STDBY_CURR_SU(l_si)      (l_si)->saAmfSINumCurrStandbyAssignments

#define m_AVD_SET_SI_SWITCH(cb,si,state) {\
si->si_switch = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, si, AVSV_CKPT_SI_SWITCH);\
}

extern void avd_si_add_csi(struct avd_csi_tag* csi);
extern void avd_si_remove_csi(struct avd_csi_tag *csi);
extern AVD_SI *avd_si_new(const SaNameT *dn);
extern void avd_si_delete(AVD_SI *si);
extern void avd_si_db_add(AVD_SI *si);
extern AVD_SI *avd_si_get(const SaNameT *si_name);
extern AVD_SI *avd_si_getnext(const SaNameT *si_name);
extern SaAisErrorT avd_si_config_get(struct avd_app_tag *app);
extern void avd_si_constructor(void);

extern AVD_SVC_TYPE *avd_svctype_get(const SaNameT *dn);
extern SaAisErrorT avd_svctype_config_get(void);
extern void avd_svctype_add_si(AVD_SI *si);
extern void avd_svctype_remove_si(AVD_SI *si);
extern void avd_svctype_constructor(void);

extern SaAisErrorT avd_svctypecstypes_config_get(SaNameT *svctype_name);
extern AVD_SVC_TYPE_CS_TYPE *avd_svctypecstypes_get(const SaNameT *svctypecstypes_name);
extern void avd_svctypecstypes_constructor(void);
extern void avd_si_inc_curr_act_ass(AVD_SI *si);
extern void avd_si_dec_curr_act_ass(AVD_SI *si);
extern void avd_si_inc_curr_stdby_ass(AVD_SI *si);
extern void avd_si_dec_curr_stdby_ass(AVD_SI *si);
extern void avd_si_inc_curr_stdby_dec_act_ass(AVD_SI *si);
extern void avd_si_inc_curr_act_dec_std_ass(AVD_SI *si);
extern void avd_si_admin_state_set(AVD_SI* si, SaAmfAdminStateT state);
extern void avd_si_assignments_delete(AVD_CL_CB *cb, AVD_SI *si);
extern void avd_si_add_rankedsu(AVD_SI *si, const SaNameT *suname, uint32_t saAmfRank);
extern void avd_si_remove_rankedsu(AVD_SI *si, const SaNameT *suname);
extern avd_sirankedsu_t *avd_si_getnext_rankedsu(const AVD_SI *si, uint32_t saAmfRank);

#endif
