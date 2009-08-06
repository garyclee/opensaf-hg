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

  DESCRIPTION:  This file contains the definitions of NCS_TRAP_VARBIND 
                and NCS_TRAP structures.

   
  ******************************************************************************
  */

/*
 * Module Inclusion Control...
 */
#ifndef NCS_TRAP_H
#define NCS_TRAP_H

#include "ncs_mib_pub.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* subscription id for traps */
#define m_SNMP_EDA_SUBSCRIPTION_ID_TRAPS 1

/* Event Filter pattern for SNMP Traps */
#define m_SNMP_TRAP_FILTER_PATTERN "SNMP_TRAP"

/* Filter pattern length */
#define m_SNMP_TRAP_FILTER_PATTERN_LEN (strlen(m_SNMP_TRAP_FILTER_PATTERN))

/* Channel Name for SNMP Traps*/
#define m_SNMP_EDA_EVT_CHANNEL_NAME    "NCS_TRAP"

/* version of the NCS_TRAP data structure */
#define m_NCS_TRAP_VERSION 1

/* Data structure to be used by the applications for sending traps*/
	typedef struct ncs_TRAP_VARBIND {
		/*Table id, in which the param_id is defined */
		NCSMIB_TBL_ID i_tbl_id;

		/* Detail of the parameter like id, type and value */
		NCSMIB_PARAM_VAL i_param_val;

		/* param instance */
		NCSMIB_IDX i_idx;

		/* next varbind in the trap */
		struct ncs_TRAP_VARBIND *next_trap_varbind;
	} NCS_TRAP_VARBIND;

	typedef struct ncs_trap {
		/* ncstrap version number */
		uns16 i_version;

		/* trap-tabl-id */
		/* I purposefully defined the table-id as uns32, to avoid the 
		 * dependency on NCS_MTBL.H file
		 */
		NCSMIB_TBL_ID i_trap_tbl_id;

		/* Trap identifier, as defined in the MIB */
		NCSMIB_PARAM_ID i_trap_id;

		/* A flag to be used by the SubAgent, to determine whether this 
		 * trap to be sent to the SNMP Agent or not?? 
		 * When this flag is set to TRUE, SubAgent will convert the
		 * event to SNMP Trap and sends it to the Agent to get it fwded
		 * to the Manger.  When this flag is set to FALSE, SubAgent
		 * discards the event. 
		 */
		NCS_BOOL i_inform_mgr;

		/* List of varbinds */
		NCS_TRAP_VARBIND *i_trap_vb;

	} NCS_TRAP;

/* to allocate memory for NCS_TRAP_VARBIND data structure */
#define m_MMGR_NCS_TRAP_VARBIND_ALLOC\
    (NCS_TRAP_VARBIND*)m_NCS_MEM_ALLOC(sizeof(NCS_TRAP_VARBIND), \
                                         NCS_MEM_REGION_PERSISTENT,\
                                         NCS_SERVICE_ID_OS_SVCS, \
                                         0)

/* to free the NCS_TRAP_VARBIND data structure */
#define m_MMGR_NCS_TRAP_VARBIND_FREE(p) m_NCS_MEM_FREE(p, \
                                         NCS_MEM_REGION_PERSISTENT,\
                                         NCS_SERVICE_ID_OS_SVCS, \
                                         0)

	EXTERN_C LEAPDLL_API uns32 ncs_tlvsize_for_ncs_trap_get(NCS_TRAP *o_trap);

/* to free the list of varbinds */
	EXTERN_C LEAPDLL_API uns32 ncs_trap_eda_trap_varbinds_free(NCS_TRAP_VARBIND *i_trap_varbinds);

#ifdef  __cplusplus
}
#endif

#endif
