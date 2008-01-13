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
#ifndef SUBAGT_OID_DB_H
#define SUBAGT_OID_DB_H

#ifdef  __cplusplus
extern "C" {
#endif
  
#include "ncsgl_defs.h"
#include "ncspatricia.h"
#include "dta_papi.h"  
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/library/asn1.h"
#include "net-snmp/library/snmp.h"
#include "net-snmp/library/snmp_api.h"
#include "net-snmp/library/callback.h" /* required for NET-SNMP-5.1.1 */
#include "net-snmp/library/data_list.h"
#include "net-snmp/library/snmp_impl.h"
#include "net-snmp/agent/snmp_agent.h"
#include "net-snmp/agent/agent_handler.h"
#include "net-snmp/agent/snmp_vars.h"
#include "net-snmp/agent/var_struct.h"

/* OID Database key */
typedef struct ncsSa_oid_database_key
{
    uns32        i_table_id;
}NCSSA_OID_DATABASE_KEY;

/* Node data structure for the OID Database */
typedef struct ncssa_oid_database_node
{
    NCS_PATRICIA_NODE       PatNode;
    NCSSA_OID_DATABASE_KEY  key;
    uns32                   base_oid_len;
    oid                     *base_oid;
    struct variable         *objects_details;
    uns32                   number_of_objects;
}NCSSA_OID_DATABASE_NODE;

/* Routine to add the table-id and base-oid mapping */
EXTERN_C uns32
snmpsubagt_table_oid_add(uns32 table_id,
                    oid *oid_base, uns32 oid_len, 
                    struct variable *object_details, 
                    uns32   number_of_objects);

/* Routine to delete the table-id and base-oid mapping */
EXTERN_C uns32
snmpsubagt_table_oid_del(uns32 table_id);

EXTERN_C void
snmpsubagt_table_oid_destroy(NCS_PATRICIA_TREE   *oid_db);

/* log routines used in the generated code */
#define m_SNMPSUBAGT_GEN_STR(string)\
    snmpsubagt_gen_str_log(string) 

#define m_SNMPSUBAGT_GEN_ERR(string, error)\
    snmpsubagt_gen_err_log(string, error)

#define m_SNMPSUBAGT_GEN_OID_LOG(oid) \
    snmpsubagt_gen_oid_log(oid)

EXTERN_C void
snmpsubagt_gen_str_log(char*);

EXTERN_C void
snmpsubagt_gen_err_log(char*, uns32);

EXTERN_C void
snmpsubagt_gen_oid_log(NCSFL_MEM);

#ifdef  __cplusplus
}
#endif

#endif


