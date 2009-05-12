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
  
  DESCRIPTION: This file has the extern definitions for the functions 
                used in the MAB Interface.  
  ***************************************************************************/ 
#ifndef SUBAGT_MAB_H
#define SUBAGT_MAB_H

#ifdef  __cplusplus
extern "C" {
#endif
  
/* Optiroute specific includes */
#include "ncsgl_defs.h"
#include "ncs_mib_pub.h"
#include "ncs_ubaid.h"
#include "mac_papi.h"

/* Following are from NET_SNMP Library  */
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/library/asn1.h"
#include "net-snmp/library/snmp.h"
#include "net-snmp/library/snmp_api.h"
#include "net-snmp/library/callback.h" /* required for NET-SNMP-5.1.1 */
#include "net-snmp/library/data_list.h"
#include "net-snmp/library/snmp_impl.h"
#include "net-snmp/library/int64.h"
#include "net-snmp/agent/snmp_agent.h"
#include "net-snmp/agent/agent_handler.h"
#include "net-snmp/agent/snmp_vars.h"
#include "net-snmp/agent/var_struct.h"
#include "net-snmp/agent/agent_registry.h"
#include "net-snmp/agent/scalar_group.h" /* reqd for scalars */

/* How much time should the subagent wait for a response from MAC? */
#define NCS_SNMPSUBAGT_MAC_TIMEOUT  500 /* 5 seconds */

EXTERN_C
NCSMIB_PARAM_VAL    rsp_param_val;

EXTERN_C
uns32               g_subagt_mac_hdl; 

EXTERN_C
U64                 g_counter64;

#if 0
/* to set the MAC Handle */ /* TBD This is defined in subagt_mac.h file */
#define     m_SUBAGT_MAC_HDL_SET(hdl)\
            g_subagt_mac_hdl = hdl
#endif

/* to Get the MAC Handle */
#define     m_SUBAGT_MAC_HDL_GET    g_subagt_mac_hdl

EXTERN_C uns32
snmpsubagt_mab_mac_msg_send(NCSMIB_ARG *io_mib_arg, uns32 i_table_id, uns32 *i_inst,
        uns32 i_inst_len, uns32 i_param_id, uns32 i_param_type, uns32 i_req_type,
        void  *i_set_val, uns32 i_set_val_len, uns32 i_time_val, NCSMEM_AID   *ma, 
        uns8   *space, uns32  space_size, struct variable *obj_details, uns32 obj_count,
        NCS_BOOL is_sparse_table);

EXTERN_C uns32
snmpsubagt_mab_mib_param_fill(NCSMIB_PARAM_VAL *io_param_val,
                   NCSMIB_PARAM_ID i_param_id, uns32 i_param_type,
                   void *i_set_val, uns32 i_set_val_len, uns8 *counter64_in_octs);

EXTERN_C NCSMIB_FMAT_ID
snmpsubagt_mab_param_type_get(uns32 i_param_type);

EXTERN_C uns32
snmpsubagt_mab_index_extract(oid *i_name, 
                uns32 i_name_len,
                oid   *base_oid,
                uns32 base_oid_len,
                uns32 *o_instance,
                uns32 *o_instance_len);

EXTERN_C uns32
snmpsubagt_mab_oid_compose(oid           *io_oid,
                        size_t          *io_oid_length,
                        oid             *i_base_oid,
                        uns32           i_base_oid_len,
                        NCSMIB_NEXT_RSP *i_next_rsp);

EXTERN_C unsigned char*
snmpsubagt_mab_mibarg_resp_process(NCSMIB_PARAM_VAL    *i_rsp_param_val, 
                                size_t              *var_len, uns32 i_param_type); 

/* function to validate the given oid */
uns32 snmpsubagt_given_oid_validate(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method);

EXTERN_C uns32 
ncs_snmpsubagt_init_deinit_msg_post(uns8* init_deinit_routine);

/* function to convert the error code from MAB to NET-SNMP */
EXTERN_C unsigned char
snmpsubagt_mab_error_code_map(uns32 mab_err_code, uns32 req_type);

/* for de-registration of the MIBs with the Agent */  
EXTERN_C int32
snmpsubagt_mab_unregister_mib(netsnmp_handler_registration *reginfo); 
    

/* for de-registration of the MIBs with the Agent */  
EXTERN_C int32
snmpsubagt_mab_unregister_mib(netsnmp_handler_registration *reginfo); 

#ifdef  __cplusplus
}
#endif

#endif


