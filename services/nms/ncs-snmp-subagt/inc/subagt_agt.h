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
  
  DESCRIPTION: This file describes the routines for the Agentx master 
                agent interface
  
  ***************************************************************************/ 
#ifndef SUBAGT_AGT_H
#define SUBAGT_AGT_H
struct ncsSa_cb;

/* Modes to wait for the requests from the Agent */
#define SNMPSUBAGT_NONBLOCK_MODE 0
#define SNMPSUBAGT_BLOCK_MODE 1

#define SNMPSUBAGT_LOG_FILE "/var/log/ncsSnmpSubagt.log"

/* To initialize the communication with the Agentx Agent */
EXTERN_C uns32
snmpsubagt_netsnmp_lib_initialize(struct ncsSa_cb  *cb);

/* To finalize the session with the Agent, this frees the OID tree also */
uns32
snmpsubagt_netsnmp_lib_finalize(struct ncsSa_cb  *cb);

#if 0
/* to deinitilize the session with the Agent */
EXTERN_C uns32
snmpsubagt_netsnmp_lib_deinit(struct ncsSa_cb  *cb);
#endif

/* Handling Multiple Interfaces */
/* 1. Interface with AMF
 * 2. Interface with Agentx Agent
 * 3. Interface with EDA
 */
uns32
snmpsubagt_request_process(struct ncsSa_cb  *cb);

uns32 
snmpsubagt_agt_startup_params_process(int32    argc, 
                                      uns8     **argv);

#if ( NET_SNMP_5_2_2_SUPPORT == 1)
 /*From Net-Snmp version 5.3.X  onwards , the equivalent call 
  "agentx_config_init" is invoked in "init_agent" API"*/

/* Function prototype of init_agentx_config declared here to avoid agentx header file inclusion. */
void init_agentx_config(void); 
#endif

/* IR00061409 */
EXTERN_C void subagt_process_sig_usr1_signal(struct ncsSa_cb *cb);
EXTERN_C void snmpsubagt_check_and_flush_buffered_traps(struct ncsSa_cb *cb);
#endif


