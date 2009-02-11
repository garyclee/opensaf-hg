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

#include <configmake.h>

/*****************************************************************************
..............................................................................

  
  .....................................................................  
  
  DESCRIPTION: This file describes AMF Interface 
*****************************************************************************/ 
#ifndef SUBAGT_AMF_H
#define SUBAGT_AMF_H

/* fwd declaration for SubAgent control block - to avoid warnings */
struct ncsSa_cb;

/* SubAgent HA State */
typedef enum snmpsubagt_ha_state
{
    SNMPSUBAGT_HA_STATE_NULL = 0, 
    SNMPSUBAGT_HA_STATE_ACTIVE = SA_AMF_HA_ACTIVE, 
    SNMPSUBAGT_HA_STATE_STANDBY = SA_AMF_HA_STANDBY,
    SNMPSUBAGT_HA_STATE_QUISCED = SA_AMF_HA_QUIESCED,
    SNMPSUBAGT_HA_STATE_QUISCING = SA_AMF_HA_QUIESCING,
    SNMPSUBAGT_HA_STATE_MAX 
}SNMPSUBAGT_HA_STATE;

#define m_SUBAGT_HB_INVOCATION_TYPE       SA_AMF_HEALTHCHECK_AMF_INVOKED

#define m_SUBAGT_RECOVERY                 SA_AMF_COMPONENT_RESTART

#define m_SNMPSUBAGT_AMF_HELATH_CHECK_KEY "A9FD64E12C" /* got the key from BAM */

#define SNMPSUBAGT_LIB_CONF OSAF_SYSCONFDIR "subagt_lib_conf" 

#define m_SUBAGENT_COMP_NAME_FILE OSAF_LOCALSTATEDIR "ncs_subagent_comp_name"  

#define m_SNMPSUBAGT_AMF_TIMEOUT_IN_SEC     1

#define m_SNMPSUBAGT_AMF_INIT_NOT_STARTED     0
#define m_SNMPSUBAGT_AMF_INIT_RETRY           1
#define m_SNMPSUBAGT_AMF_INIT_COMPLETED       2
#define m_SNMPSUBAGT_AMF_COMP_REG_RETRY       3
#define m_SNMPSUBAGT_AMF_COMP_REG_COMPLETED   4

/* routine to intialize the AMF */
uns32
snmpsubagt_amf_initialize(struct ncsSa_cb *cb);

/* Unregistration and Finalization with AMF Library */
uns32
snmpsubagt_amf_finalize(struct ncsSa_cb *cb);

/* Following functions may be moved to another common file -- NCS 2.0 ??? */

/* to register the MIBs supported in NCS of all the services */ 
EXTERN_C uns32 
snmpsubagt_pending_regs_cmp(uns8 *key1, uns8 *key2);

EXTERN_C uns32  
snmpsubagt_pending_regs_free(NCS_DB_LINK_LIST_NODE *free_node); 

/* process the pending registrations */
EXTERN_C uns32
snmpsubagt_pending_registrations_do(NCS_DB_LINK_LIST *pending_registrations);

EXTERN_C void  
snmpsubagt_pending_registrations_free(NCS_DB_LINK_LIST *pending_registrations);

/* to start the health check */ 
EXTERN_C uns32
snmpsubagt_amf_healthcheck_start_msg_post(struct ncsSa_cb *cb); 

/* to register the MIBs supported in NCS of all the services */
EXTERN_C uns32
snmpsubagt_appl_mibs_register(void);

/* to start the timer to retry amf initialisation. */ 
EXTERN_C uns32
snmpsubagt_amf_init_timer_start(struct ncsSa_cb *cb); 

/* to read the configuration file and to load/unload the 
 * application spec MIBs 
 */
EXTERN_C uns32
snmpsubagt_spa_job(uns8 *file_name, uns32 what_to_do); 

/* to reload the libraries in subagent configuration file */ 
EXTERN_C uns32
snmpsubagt_mibs_reload(struct ncsSa_cb *cb); 

/* unload the subagent integration libraries */ 
EXTERN_C uns32
snmpsubagt_mibs_unload(void); 
 

/* Go to ACTIVE State */
EXTERN_C uns32
snmpsubagt_ACTIVE_process (struct ncsSa_cb    *cb,
                           SaInvocationT       invocation);


/* Go to STANDBY State */
EXTERN_C uns32
snmpsubagt_STANDBY_process(struct ncsSa_cb    *cb,
                           SaInvocationT       invocation);
EXTERN_C uns32
subagt_rda_init_role_get(struct ncsSa_cb *cb);

#endif


