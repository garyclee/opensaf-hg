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

  This module is the include file for Availability Directors timer module.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_TMR_H
#define AVD_TMR_H

#include <ncssysf_tmr.h>

/* timer type enums */
typedef enum avd_tmr_type {
	AVD_TMR_SND_HB,		/* heart beat send timer */
	AVD_TMR_RCV_HB_D,	/* heart beat receive from other
				 * director timer */
	AVD_TMR_RCV_HB_INIT,	/* heart beat received from other
				 * director during init phase timer */
	AVD_TMR_CL_INIT,	/* This is the AvD initialisation 
				 * timer after which AvD will assign
				 * SIs to application SU. */

	AVD_TMR_SI_DEP_TOL,	/* SI_SI dependency tolerance timer */
	AVD_TMR_MAX
} AVD_TMR_TYPE;

/* AVD Timer definition */
typedef struct avd_tmr_tag {
	tmr_t tmr_id;
	AVD_TMR_TYPE type;
	SaClmNodeIdT node_id;
	SaNameT spons_si_name;
	SaNameT dep_si_name;
	NCS_BOOL is_active;
} AVD_TMR;

/* macro to start the cluster init timer. The cb structure
 * is the input.
 */
#define m_AVD_CLINIT_TMR_START(cb) \
{\
   avd_log(NCSFL_SEV_NOTICE, "Starting cluster startup timer"); \
   cb->amf_init_tmr.is_active = FALSE; \
   cb->amf_init_tmr.type = AVD_TMR_CL_INIT; \
   avd_start_tmr(cb,&(cb->amf_init_tmr), avd_cluster->saAmfClusterStartupTimeout); \
}

/* macro to start the Send Heart beat timer. The cb structure
 * is the input.
 */
#define m_AVD_SND_HB_TMR_START(cb) \
{\
   cb->heartbeat_send_avd.is_active = FALSE; \
   cb->heartbeat_send_avd.type = AVD_TMR_SND_HB; \
   cb->heartbeat_send_avd.node_id = cb->node_id_avd_other; \
   avd_start_tmr(cb,&(cb->heartbeat_send_avd),cb->snd_hb_intvl); \
}

/* macro to start the Receive Heart beat timer. The cb structure
 * is the input.
 */
#define m_AVD_RCV_HB_TMR_START(cb) \
{\
   cb->heartbeat_rcv_avd.is_active = FALSE; \
   cb->heartbeat_rcv_avd.type = AVD_TMR_RCV_HB_D; \
   cb->heartbeat_rcv_avd.node_id = cb->node_id_avd_other; \
   avd_start_tmr(cb,&(cb->heartbeat_rcv_avd),cb->rcv_hb_intvl); \
}

#define m_AVD_SI_DEP_TOL_TMR_START(cb, si_dep_rec) \
{\
   si_dep_rec->si_dep_timer.is_active = FALSE; \
   si_dep_rec->si_dep_timer.type = AVD_TMR_SI_DEP_TOL; \
   si_dep_rec->si_dep_timer.spons_si_name = si_dep_rec->indx_imm.si_name_prim; \
   si_dep_rec->si_dep_timer.dep_si_name = si_dep_rec->indx_imm.si_name_sec; \
   avd_start_tmr(cb, &si_dep_rec->si_dep_timer, si_dep_rec->saAmfToleranceTime); \
}

/*** Extern function declarations ***/

struct cl_cb_tag;

EXTERN_C void avd_tmr_exp(void *);

EXTERN_C uns32 avd_start_tmr(struct cl_cb_tag *, AVD_TMR *, SaTimeT);

EXTERN_C void avd_stop_tmr(struct cl_cb_tag *, AVD_TMR *);

#endif
