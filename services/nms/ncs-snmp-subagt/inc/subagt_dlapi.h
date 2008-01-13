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
  
  DESCRIPTION: This file describes the DL-SE APIs
  
  ***************************************************************************/ 
#ifndef SUBAGT_DLAPI_H
#define SUBAGT_DLAPI_H

struct ncsSa_cb;

/* Single entry API for the NCS-SNMP-SUBAGT */
EXTERN_C uns32
ncs_snmpsubagt_lib_req(NCS_LIB_REQ_INFO *io_lib_req);

/* CREATE DL-SE API prototype */
EXTERN_C uns32 
ncs_snmpsubagt_create (int32 argc, uns8  **argv);

/* DESTROY DL-SE API prototype */
EXTERN_C uns32 
ncs_snmpsubagt_destroy(void); 

EXTERN_C uns32
snmpsubagt_destroy(struct ncsSa_cb *cb, NCS_BOOL comp_unreg);

uns32 
ncs_snmpsubagt_init_deinit_msg_post(uns8* init_deinit_routine);

#if 0

/* ABOUT DL-SE API prototype */
EXTERN_C uns32 
ncs_ssnmpsubagt_about(NCS_LIB_ABOUT *about); 


/* DIAGNOSE  DL-SE API prototype */
EXTERN_C uns32 
ncs_snmpsubagt_diagnose(); 

#endif
#endif


