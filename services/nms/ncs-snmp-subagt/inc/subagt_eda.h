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

  
  .....................................................................  
  
  DESCRIPTION: This file describes the EDA Interface
  
  ***************************************************************************/ 
#ifndef SUBAGT_EDA_H
#define SUBAGT_EDA_H

struct ncsSa_cb;

/* subscription id for traps */
#define m_SNMPSUBAGT_EDA_SUBSCRIPTION_ID_TRAPS m_SNMP_EDA_SUBSCRIPTION_ID_TRAPS

/* Event Filter pattern */
#define m_SNMPSUBAGT_EDA_EVT_FILTER_PATTERN m_SNMP_TRAP_FILTER_PATTERN

/* Filter pattern length */
#define m_SNMPSUBAGT_EDA_EVT_FILTER_PATTERN_LEN (strlen(m_SNMPSUBAGT_EDA_EVT_FILTER_PATTERN))

/* Event Channel Name */
#define m_SNMPSUBAGT_EDA_EVT_CHANNEL_NAME m_SNMP_EDA_EVT_CHANNEL_NAME

#define m_SNMPSUBAGT_EDA_TIMEOUT          3000000000U /* 3 sec in nano seconds */
#define m_SNMPSUBAGT_EDA_TIMEOUT_IN_SEC   (m_SNMPSUBAGT_EDA_TIMEOUT/1000000000) 
#define m_SNMPSUBAGT_SNMPD_TIMEOUT_IN_SEC  1

#define m_SNMPSUBAGT_EDA_INIT_NOT_STARTED  0
#define m_SNMPSUBAGT_EDA_INIT_RETRY        1
#define m_SNMPSUBAGT_EDA_INIT_COMPLETED    2


/* to initialize the EDA coommunication, register the callback etc.. */
EXTERN_C SaAisErrorT
snmpsubagt_eda_initialize(struct ncsSa_cb  *pSacb);

/* Finalize the session with the EDA */
EXTERN_C uns32
snmpsubagt_eda_finalize(struct ncsSa_cb  *pSacb);

#if 0
/* to free the trap varbinds */
EXTERN_C uns32
snmpsubagt_eda_trap_varbinds_free(NCS_TRAP_VARBIND *trap_varbinds);
#endif

/* Forward declaration for the event handling callback routine */
EXTERN_C void
snmpsubagt_eda_callback( SaEvtSubscriptionIdT       subscriptionid,
                        SaEvtEventHandleT           eventHandle, 
                        const SaSizeT               eventDataSize); 

/* Function to convert the event to trap and send it to the Agent */
EXTERN_C uns32
snmpsubagt_eda_trapevt_to_agentxtrap_populate(EDU_HDL *edu_hdl,
                        NCS_PATRICIA_TREE *oid_db,/*in */
                        uns8    *evtData, /* in */
                        uns32   evtDataSize); /* in */

/* Function to start the timer to retry the eda initialisation. */
EXTERN_C uns32 snmpsubagt_eda_init_timer_start (struct ncsSa_cb *pSacb);

#endif


