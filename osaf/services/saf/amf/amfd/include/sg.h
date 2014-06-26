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
  Service group structure.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_SG_H
#define AVD_SG_H

#include <saAmf.h>
#include <ncspatricia.h>
#include <cb.h>
#include <def.h>
#include <sgtype.h>
#include <amf_defs.h>
#include <amf_d2nmsg.h>
#include "db_template.h"

class AVD_SU;
class AVD_SI;

/* The valid SG FSM states. */
typedef enum {
	AVD_SG_FSM_STABLE = 0,
	AVD_SG_FSM_SG_REALIGN,
	AVD_SG_FSM_SU_OPER,
	AVD_SG_FSM_SI_OPER,
	AVD_SG_FSM_SG_ADMIN
} AVD_SG_FSM_STATE;

/* The structure used for containing the list of SUs
 * undergoing operations on them.
 */
typedef struct avd_sg_oper_tag {
	AVD_SU *su;	/* SU undergoing operation */
	struct avd_sg_oper_tag *next;	/* The next SU undergoing operation. */
} AVD_SG_OPER;

/**
 * Service group abstract base class
 */
class AVD_SG {
public:
	AVD_SG();
	virtual ~AVD_SG() {};

	SaNameT name;		/* the service group DN used as the index.
				 * Checkpointing - Sent as a one time update.
				 */

	bool saAmfSGAutoRepair_configured; /* True when user configures saAmfSGAutoRepair else false */
   /******************** B.04 model *************************************************/
	SaNameT saAmfSGType;	/* Network order. */
	SaNameT saAmfSGSuHostNodeGroup;	/* Network order. */
	SaBoolT saAmfSGAutoRepair;
	SaBoolT saAmfSGAutoAdjust;

	SaUint32T saAmfSGNumPrefActiveSUs;	/* the N value in the redundancy model, where
						 * applicable.
						 * Checkpointing - Sent as a one time update.
						 */

	SaUint32T saAmfSGNumPrefStandbySUs;	/* the M value in the redundancy model, where
						 * applicable.
						 * Checkpointing - Sent as a one time update.
						 */

	SaUint32T saAmfSGNumPrefInserviceSUs;	/* The preferred number of in service
						 * SUs.
						 * Checkpointing - Sent as a one time update.
						 */

	SaUint32T saAmfSGNumPrefAssignedSUs;	/* The number of active SU assignments
						 * an SI can have.
						 * Checkpointing - Sent as a one time update.
						 */

	SaUint32T saAmfSGMaxActiveSIsperSU;	/* The maximum number of active 
						 * instance of Sis that can be 
						 * assigned to an SU.
						 * Checkpointing - Sent as a one time update.
						 */

	SaUint32T saAmfSGMaxStandbySIsperSU;	/* The maximum number of standby 
						 * instance of Sis that can be
						 * assigned to an SU.
						 * Checkpointing - Sent as a one time update.
						 */
	SaTimeT saAmfSGAutoAdjustProb;
	SaTimeT saAmfSGCompRestartProb;	/* component restart probation period
					 * Checkpointing - Sent as a one time update.
					 */
	SaUint32T saAmfSGCompRestartMax;	/* max component restart count 
						 * Checkpointing - Sent as a one time update.
						 */
	SaTimeT saAmfSGSuRestartProb;	/* SU restart probation period 
					 * Checkpointing - Sent as a one time update.
					 */

	SaUint32T saAmfSGSuRestartMax;	/* max SU restart count
					 * Checkpointing - Sent as a one time update.
					 */

	SaAmfAdminStateT saAmfSGAdminState;	/* admin state of the group.
						 * Checkpointing - Updated independently.
						 */

	SaUint32T saAmfSGNumCurrAssignedSUs;	/* Num of Sus that have been assigned a SI 
						 * Checkpointing - Updated independently.
						 */

	SaUint32T saAmfSGNumCurrInstantiatedSpareSUs;	/* Num of Sus that are in service but 
							 * not yet assigned a SI.
							 * Checkpointing - Updated independently.
							 */

	SaUint32T saAmfSGNumCurrNonInstantiatedSpareSUs;	/* Num of Sus that have been 
								 * configured but not yet instantiated.
								 * Checkpointing - Updated independently.
								 */

   /******************** B.04 model *************************************************/

	SaAdjustState adjust_state;	/* Field to re adjust the SG.
					 * Checkpointing - Updated independently.
					 */

	bool sg_ncs_spec;	/* This is set to true if the SG
				 * is a NCS specific SG.
				 * Checkpointing - Sent as a one time update.
				 */

	AVD_SG_FSM_STATE sg_fsm_state;	/* The different flows of the SU SI
					 * transitions for the SUs and SIs
					 * in the SG is orchestrated based on
					 * this FSM.
					 * Checkpointing - Updated independently.
					 */

	AVD_SI *admin_si;	/* Applicable when sg_fsm_state has
				 * AVD_SG_FSM_SI_OPER.It will contain
				 * the SI undergoing admin
				 * operation.
				 */

	AVD_SG_OPER su_oper_list;	/* The list of SUs that have operations
					 * happening on them used in parallel
					 * with sg_fsm_state.
					 * Checkpointing - Sent as a one time update.
					 */
	SaAmfRedundancyModelT sg_redundancy_model;	/* the redundancy model in the service group 
							 * see sec 4.7 for values
							 * Checkpointing - Sent as a one time update.
							 */

	AVD_SU *list_of_su;	/* the list of service units in this
					 * group in the descending order of
					 * the rank.
					 */
	AVD_SI *list_of_si;	/* the list of service instances in 
				 * this group in the descending order 
				 * of the rank.
				 */

	AVD_SG *sg_list_sg_type_next;
	struct avd_amf_sg_type_tag *sg_type;
	AVD_SG *sg_list_app_next;
	struct avd_app_tag *app;
	bool equal_ranked_su; /* This flag is set when ranks of all SU is the same.
				     It is used in equal distribution of SIs on SU 
				     in Nway, N+M and Nway-Act Red models.*/
	AVD_SU *max_assigned_su;
	AVD_SU *min_assigned_su;
	AVD_SI *si_tobe_redistributed;
	uint32_t try_inst_counter; /* It should be used when amfd try to send
				      instantiate command to amfnd in a loop
				      for all those SUs hosted on a particular
				      node. It should be reset to zero after
				      use.*/


	/**
	 * Handle node failure and fail over assignments
	 * Called when the node hosting the SU has already failed and the SIs
	 * assigned to the specified SU needs to be failed over.
	 *
	 * @param cb
	 * @param su
	 */
	virtual void node_fail(AVD_CL_CB *cb, AVD_SU *su) = 0;

	/**
	 * Handle SG realign
	 * Assign SIs if needed. If any assigning is gets done it adds
	 * the SUs to the operation list and sets the SG FSM state to SG realign.
	 * If everything is fine, it calls the routine to bring the preferred
	 * number of SUs to inservice state and change the SG state to stable.
	 *
	 * @param cb
	 * @param sg
	 * @return
	 */
	virtual uint32_t realign(AVD_CL_CB *cb, AVD_SG *sg) = 0;

	/**
	 * Handle new SI or admin op UNLOCK of SI
	 * @param cb
	 * @param si
	 * @return
	 */
	virtual uint32_t si_assign(AVD_CL_CB *cb, AVD_SI *si) = 0;

	/**
	 * Handle SI admin op LOCK/SHUTDOWN
	 * @param cb
	 * @param si
	 * @return
	 */
	virtual uint32_t si_admin_down(AVD_CL_CB *cb, AVD_SI *si) = 0;

	/**
	 * Handle SI admin operation SWAP
	 * Default implementation in base class (not pure virtual)
	 * @param si
	 * @param invocation
	 * @return
	 */
	virtual SaAisErrorT si_swap(AVD_SI *si, SaInvocationT invocation);

	/**
	 * Handle SG admin op LOCK/SHUTDOWN
	 * @param cb
	 * @param sg
	 * @return
	 */
	virtual uint32_t sg_admin_down(AVD_CL_CB *cb, AVD_SG *sg) = 0;

	/**
	 * Handle SU inservice event, possibly assign the SU
	 * @param cb
	 * @param su
	 * @return
	 */
	virtual uint32_t su_insvc(AVD_CL_CB *cb, AVD_SU *su) = 0;

	/**
	 * Handle SU failure and switch over assignments
	 * @param cb
	 * @param su
	 * @return
	 */
	virtual uint32_t su_fault(AVD_CL_CB *cb, AVD_SU *su) = 0;

	// Handle SU admin op LOCK/SHUTDOWN
	uint32_t (*su_admin_down)(AVD_CL_CB *cb, AVD_SU *su,
			struct avd_avnd_tag *avnd);

	// Handle successful SUSI assignment
	uint32_t (*susi_success)(AVD_CL_CB *cb, AVD_SU *su,
			struct avd_su_si_rel_tag *susi, AVSV_SUSI_ACT act, SaAmfHAStateT state);

	// Handle failed SUSI assignment
	uint32_t (*susi_failed)(AVD_CL_CB *cb, AVD_SU *su,
			struct avd_su_si_rel_tag *susi, AVSV_SUSI_ACT act, SaAmfHAStateT state);

private:
	// disallow copy and assign, TODO(hafe) add common macro for this
	AVD_SG(const AVD_SG&);
	void operator=(const AVD_SG&);
};

extern AmfDb<std::string, AVD_SG> *sg_db;

/**
 * 2N redundancy model SG specialization
 */
class SG_2N : public AVD_SG {
public:
	~SG_2N();
	void node_fail(AVD_CL_CB*, AVD_SU*);
	uint32_t realign(AVD_CL_CB *cb, AVD_SG *sg);
	uint32_t si_assign(AVD_CL_CB *cb, AVD_SI *si);
	uint32_t si_admin_down(AVD_CL_CB *cb, AVD_SI *si);
	SaAisErrorT si_swap(AVD_SI *si, SaInvocationT invocation);
	uint32_t sg_admin_down(AVD_CL_CB *cb, AVD_SG *sg);
	uint32_t su_insvc(AVD_CL_CB *cb, AVD_SU *su);
	uint32_t su_fault(AVD_CL_CB *cb, AVD_SU *su);
};

/**
 * No redundancy specialization
 */
class SG_NORED : public AVD_SG {
public:
	~SG_NORED();
	void node_fail(AVD_CL_CB*, AVD_SU*);
	uint32_t realign(AVD_CL_CB *cb, AVD_SG *sg);
	uint32_t si_assign(AVD_CL_CB *cb, AVD_SI *si);
	uint32_t si_admin_down(AVD_CL_CB *cb, AVD_SI *si);
	uint32_t sg_admin_down(AVD_CL_CB *cb, AVD_SG *sg);
	uint32_t su_insvc(AVD_CL_CB *cb, AVD_SU *su);
	uint32_t su_fault(AVD_CL_CB *cb, AVD_SU *su);
};

/**
 * N+M redundancy specialization
 */
class SG_NPM : public AVD_SG {
public:
	~SG_NPM();
	void node_fail(AVD_CL_CB*, AVD_SU*);
	uint32_t realign(AVD_CL_CB *cb, AVD_SG *sg);
	uint32_t si_assign(AVD_CL_CB *cb, AVD_SI *si);
	uint32_t si_admin_down(AVD_CL_CB *cb, AVD_SI *si);
	uint32_t sg_admin_down(AVD_CL_CB *cb, AVD_SG *sg);
	uint32_t su_insvc(AVD_CL_CB *cb, AVD_SU *su);
	uint32_t su_fault(AVD_CL_CB *cb, AVD_SU *su);
};

/**
 * N-Way active specialization
 */
class SG_NACV : public AVD_SG {
public:
	~SG_NACV();
	void node_fail(AVD_CL_CB*, AVD_SU*);
	uint32_t realign(AVD_CL_CB *cb, AVD_SG *sg);
	uint32_t si_assign(AVD_CL_CB *cb, AVD_SI *si);
	uint32_t si_admin_down(AVD_CL_CB *cb, AVD_SI *si);
	uint32_t sg_admin_down(AVD_CL_CB *cb, AVD_SG *sg);
	uint32_t su_insvc(AVD_CL_CB *cb, AVD_SU *su);
	uint32_t su_fault(AVD_CL_CB *cb, AVD_SU *su);
};

/**
 * N-Way specialization
 */
class SG_NWAY : public AVD_SG {
public:
	~SG_NWAY();
	void node_fail(AVD_CL_CB*, AVD_SU*);
	uint32_t realign(AVD_CL_CB *cb, AVD_SG *sg);
	uint32_t si_assign(AVD_CL_CB *cb, AVD_SI *si);
	uint32_t si_admin_down(AVD_CL_CB *cb, AVD_SI *si);
	uint32_t sg_admin_down(AVD_CL_CB *cb, AVD_SG *sg);
	uint32_t su_insvc(AVD_CL_CB *cb, AVD_SU *su);
	uint32_t su_fault(AVD_CL_CB *cb, AVD_SU *su);
};


#define m_AVD_SET_SG_ADJUST(cb,sg,state) {\
	TRACE("adjust_state %u => %u", sg->adjust_state, state); \
	sg->adjust_state = state;\
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, sg, AVSV_CKPT_SG_ADJUST_STATE);\
}

#define m_AVD_SET_SG_FSM(cb,sg,state) {\
	if (sg->sg_fsm_state != state) { \
		TRACE("sg_fsm_state %u => %u", sg->sg_fsm_state, state); \
		sg->sg_fsm_state = state;\
		m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, sg, AVSV_CKPT_SG_FSM_STATE);\
	}\
	if (state == AVD_SG_FSM_STABLE) {\
		osafassert(sg->su_oper_list.su == NULL); \
	}\
}

#define m_AVD_SET_SG_ADMIN_SI(cb,si) {\
	TRACE("admin_si set to %s", si->name.value); \
	si->sg_of_si->admin_si = si;\
	m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, (si->sg_of_si), AVSV_CKPT_AVD_SG_ADMIN_SI);\
}

#define m_AVD_CLEAR_SG_ADMIN_SI(cb,sg) {\
	if (sg->admin_si != AVD_SI_NULL) {\
		TRACE("admin_si cleared"); \
		m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, sg, AVSV_CKPT_AVD_SG_ADMIN_SI);\
		sg->admin_si = AVD_SI_NULL;\
	}\
}

/*****************************************************************************
 * Macro: m_AVD_CHK_OPLIST
 *
 * Purpose:  This macro will search the SU operation list in the SG to
 * identify if the given SU is present in the list
 *
 * Input: su - the pointer to the SU to be checked in the list.
 *        flag - the bool field indicating if found or not.
 *        
 *
 * Return: none.
 *
 * NOTES: 
 *
 * 
 **************************************************************************/
#define m_AVD_CHK_OPLIST(i_su,flag) \
{\
   AVD_SG_OPER *l_suopr;\
   flag = false;\
   if (i_su->sg_of_su->su_oper_list.su == i_su)\
   {\
      flag = true;\
   }else if (i_su->sg_of_su->su_oper_list.next != NULL)\
   {\
      l_suopr = i_su->sg_of_su->su_oper_list.next;\
      while (l_suopr != NULL)\
      {\
         if (l_suopr->su == i_su)\
         {\
            flag = true;\
            l_suopr = NULL;\
            continue; \
         }\
         l_suopr = l_suopr->next;\
      }\
   }\
}

extern void avd_sg_delete(AVD_SG *sg);
extern void avd_sg_db_add(AVD_SG *sg);
extern void avd_sg_db_remove(AVD_SG *sg);
extern void avd_sg_add_si(AVD_SG *sg, AVD_SI *si);
extern void avd_sg_remove_si(AVD_SG *sg, AVD_SI *si);
extern SaAisErrorT avd_sg_config_get(const SaNameT *app_dn, struct avd_app_tag *app);
extern void avd_sg_add_su(AVD_SU *su);
extern void avd_sg_remove_su(AVD_SU *su);
extern void avd_sg_constructor(void);

extern void avd_sg_admin_state_set(AVD_SG* sg, SaAmfAdminStateT state);
extern void avd_sg_nwayact_screening_for_si_distr(AVD_SG *avd_sg);
extern void avd_sg_nway_screen_si_distr_equal(AVD_SG *sg);
extern void avd_su_role_failover(AVD_SU *su, AVD_SU *stdby_su);
extern bool sg_is_tolerance_timer_running_for_any_si(AVD_SG *sg);
extern void avd_sg_adjust_config(AVD_SG *sg);
extern uint32_t sg_instantiated_su_count(const AVD_SG *sg);


#endif
