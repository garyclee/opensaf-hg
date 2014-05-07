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

  DESCRIPTION: Service Unit class definition
  
******************************************************************************
*/

#ifndef AVD_SU_H
#define AVD_SU_H

#include <saAmf.h>
#include <def.h>
#include <cb.h>
#include <node.h>
#include <sg.h>
#include <amf_defs.h>
#include <msg.h>
#include <comp.h>
#include "include/db_template.h"

/**
 * AMF director Service Unit representation.
 */
//TODO: all attributes that have a setter should probably have an getter
class AVD_SU {
 public:
	SaNameT name;
	SaNameT saAmfSUType;
	uint32_t saAmfSURank;
	SaNameT saAmfSUHostNodeOrNodeGroup;
	bool saAmfSUFailover;
	/* true when user has configured saAmfSUFailover */
	bool saAmfSUFailover_configured;
	SaNameT saAmfSUMaintenanceCampaign;

	/* runtime attributes */
	SaBoolT saAmfSUPreInstantiable;
	SaAmfOperationalStateT saAmfSUOperState;
	SaAmfAdminStateT saAmfSUAdminState;
	SaAmfReadinessStateT saAmfSuReadinessState;
	SaAmfPresenceStateT saAmfSUPresenceState;
	SaNameT saAmfSUHostedByNode;
	SaUint32T saAmfSUNumCurrActiveSIs;
	SaUint32T saAmfSUNumCurrStandbySIs;
	SaUint32T saAmfSURestartCount;

	AVD_ADMIN_OPER_CBK pend_cbk;	/* Stores zero invocation value of imm adm cbk
					 * when no admin operation is going on.
					 */

	bool term_state;	/* admin state to terminate the
				 * service unit.
				 * Checkpointing - Updated independently.
				 */

	SaToggleState su_switch;	/* The field that indicates if
					 * the active SIs assigned to 
					 * this SU needs to be Toggled.
					 * Checkpointing - Updated independently.
					 */

	bool su_is_external;	/* indicates if this SU is external */

	int su_act_state; // not used, kept for EDU, remove later

	struct avd_sg_tag *sg_of_su;	/* the service group of this SU */
	struct avd_avnd_tag *su_on_node;	/*  the node on which this SU resides */
	struct avd_su_si_rel_tag *list_of_susi;	/* the list of su si relationship elements */

	// TODO: use some container for the comp list
	struct avd_comp_tag *list_of_comp;	/* the list of  components in this SU */

	AVD_SU *sg_list_su_next;	/* the next SU in the SG */
	AVD_SU *avnd_list_su_next;	/* the next SU in the AvND */
	struct avd_sutype *su_type;
	AVD_SU *su_list_su_type_next;

	AVD_SU() {};
	AVD_SU(const SaNameT *dn);
	~AVD_SU() {};
	void set_su_failover(bool value);
	void dec_curr_stdby_si();
	void inc_curr_stdby_si();
	void inc_curr_act_si();
	void dec_curr_act_si();
	int hastate_assignments_count(SaAmfHAStateT ha_state);
	void add_comp(struct avd_comp_tag *comp);
	void remove_comp(struct avd_comp_tag *comp);
	void set_admin_state(SaAmfAdminStateT admin_state);
	void set_pres_state(SaAmfPresenceStateT state);
	void set_readiness_state(SaAmfReadinessStateT readiness_state);
	void set_oper_state(SaAmfOperationalStateT state);
	void set_oper_state(uint32_t state) {
		set_oper_state(static_cast<SaAmfOperationalStateT>(state));
	};
	void delete_all_susis(void);
	void set_all_susis_assigned_quiesced(void);
	void set_all_susis_assigned(void);
	void set_term_state(bool state);
	void remove_from_model();

 private:
	void send_attribute_update(AVSV_AMF_SU_ATTR_ID attrib_id);

	// disallow copy and assign, TODO(hafe) add common macro for this
	AVD_SU(const AVD_SU&);
	void operator=(const AVD_SU&);
};

extern AmfDb<AVD_SU> *su_db;

typedef struct {
	NCS_PATRICIA_NODE tree_node;	/* key is name */
	SaNameT name;
	SaUint32T saAmfSutMaxNumComponents;
	SaUint32T saAmfSutMinNumComponents;

	SaUint32T curr_num_components;

} AVD_SUTCOMP_TYPE;

#define m_AVD_SET_SU_SWITCH(cb,su,state) {\
su->su_switch = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVSV_CKPT_SU_SWITCH);\
}

#define m_AVD_APP_SU_IS_INSVC(i_su,su_node_ptr) \
((su_node_ptr->saAmfNodeAdminState == SA_AMF_ADMIN_UNLOCKED) && \
(su_node_ptr->saAmfNodeOperState == SA_AMF_OPERATIONAL_ENABLED) && \
(i_su->sg_of_su->saAmfSGAdminState == SA_AMF_ADMIN_UNLOCKED) &&\
(i_su->saAmfSUAdminState == SA_AMF_ADMIN_UNLOCKED) &&\
(i_su->saAmfSUOperState == SA_AMF_OPERATIONAL_ENABLED)\
)

#define m_AVD_GET_SU_NODE_PTR(avd_cb,i_su,su_node_ptr)  \
 if(true == i_su->su_is_external) su_node_ptr = avd_cb->ext_comp_info.local_avnd_node; \
 else su_node_ptr = i_su->su_on_node;

/**
 * Get SUs from IMM and create internal objects
 * 
 * @return SaAisErrorT
 */
extern SaAisErrorT avd_su_config_get(const SaNameT *sg_name, struct avd_sg_tag *sg);

/**
 * Class constructor, must be called before any other function
 */
extern void avd_su_constructor(void);

/**
 * Get SaAmfSutCompType object from DB using given key
 * 
 * @param dn
 * 
 * @return AVD_SUTCOMP_TYPE*
 */
extern AVD_SUTCOMP_TYPE *avd_sutcomptype_get(const SaNameT *dn);

/**
 * Get configuration for all SaAmfSutCompType objects from IMM and
 * create AVD internal objects.
 * @param cb
 * 
 * @return int
 */
extern SaAisErrorT avd_sutcomptype_config_get(SaNameT *sutype_name, struct avd_sutype *sut);

/**
 * Class constructor, must be called before any other function
 */
extern void avd_sutcomptype_constructor(void);

extern AVD_SU *avd_su_get_or_create(const SaNameT *dn);

#endif
