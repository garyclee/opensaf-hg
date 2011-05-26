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

  MODULE NAME:  SYSF_LOG.H

..............................................................................

  DESCRIPTION:

  Header file for LEAP implementation of MDS

******************************************************************************
*/

#include "ncs_mds_def.h"

#ifndef SYSF_LOG_H
#define SYSF_LOG_H

 /************************************************************************

  M C G    F l e x l o g    S e r v i c e 

  This header file exposes that part of the Flexlog service that can be 
  taken over or manipulated by the target service. This includes:

  A set of macros that map to the MCG reference implementation found
     in sysf_log.c. The macro names themselves are used in NetPlane base
     code and cannot be changed.

  A Debug configuration flag that causes the Flexlog reference 
     implementation to validate information both at registration time and
     at runtime. When logging has been shown to be proper, this flag should
     be turned off for production code.

 ************************************************************************/

/************************************************************************

  M C G   F l e x l o g  S e r v i c e  D e f i n i t i o n
  
*************************************************************************/

#define SYSF_FL_LOG_BUFF_SIZE  20000
#define SYSF_FL_LOG_SIZE       15000

/************************************************************************

  These externs map to Flexlog Target Service reference implementation.
  
*************************************************************************/

uns32 dts_log_init(void);

uns32 dts_ascii_spec_register(NCSFL_ASCII_SPEC *spec);

uns32 dts_ascii_spec_deregister(SS_SVC_ID ss_id, uns16 version);

uns32 dts_log_msg_to_str(DTA_LOG_MSG *msg, char *str,
				  NODE_ID node, uns32 proc_id, uns32 *len, NCSFL_ASCII_SPEC *spec);

void dts_to_lowercase(char *str);

/************************************************************************

  F l e x l o g     S e r v i c e   D e f i n i t i o n

  The following macros are mapped to the Fexlog TargetService reference
  implementation. The macros are used in MCG Subystem base code.
  
  This allows a more formal and full featured FlexLog to be easily mapped
  to these same macros so that no base code has to change.

  The following functions are required:

  m_NCSFL_INIT()     - Must be called early in the life of the system 
                      before any logging or subsystem Flexlog registration 
                      can happen.

  m_NCSFL_REGISTER() - A subystem must register its logging output format 
                      strings with Flexlog. If there are multiple instances
                      of the subsystem (the virtual router case), each can
                      register with flexlog if the design calls for it as
                      Flexlog keeps a use-cnt to know how many should 
                      deregister.
                      
                      NOTE that with a real flexlog implementation, this 
                      registration only needs to happen in the context of the 
                      process that actually formats (and probably outputs) 
                      log strings.

 m_NCSFL_DEREGISTER()- A subystem must deregister. Again, if each instance 
                      registers, then each instance must deregister.

  m_NCSFT_LOG()      - Subsystems assume that this macro can be invoked
                      locally to deliver their normalized logging info.
                      The MCG porting 'reference implementation' 
                      simply formats the strings and does standard file I/O.
                      A real Flexlog service might do a host of clever
                      filtering, re-directing, sorting, etc. activities
                      based on the 'hdr' information in the logging message.
                
  
*************************************************************************/

#define m_NCSFL_INIT()                    dts_log_init()
#define m_NCSFL_REGISTER(s)               dts_ascii_spec_register(s)
#define m_NCSFL_DEREGISTER(i)             dts_ascii_spec_deregister(i)
#define m_NCSFL_LOG(m)                    ncs_dta_log_msg(m)

/* Every subsystem gets a slot in the gl_specs array.. */
typedef struct sysf_ascii_specs {
	NCS_PATRICIA_NODE svcid_node;
	/*SS_SVC_ID          svc_id; svc_id will now be in network order */
	ASCII_SPEC_INDEX key;	/* New key to index into the patricia tree */
	NCSFL_ASCII_SPEC *ss_spec;
	uns32 use_count;	/* use count to monitor no. of DTAs registering with the particular this particular ASCII_SPEC table */
} SYSF_ASCII_SPECS;

typedef enum dts_spec_action {
	DTS_SPEC_UNLOAD,
	DTS_SPEC_LOAD
} DTS_SPEC_ACTION;

#endif
