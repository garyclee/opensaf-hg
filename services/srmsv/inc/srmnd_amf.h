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

  MODULE NAME: SRMND_AMF.H

..............................................................................

  DESCRIPTION: This file contains AMF related data structures and function
               prototypes defined for SRMSv Node Director.

******************************************************************************/

#ifndef SRMND_AMF_H
#define SRMND_AMF_H

#define SRMND_HB_INVOCATION_TYPE       SA_AMF_HEALTHCHECK_AMF_INVOKED
#define SRMND_RECOVERY                 SA_AMF_COMPONENT_FAILOVER
#define SRMND_AMF_HEALTH_CHECK_KEY     "A9A9" /* Check with BOM */

/* Update AMF version details */
#define m_SRMSV_GET_AMF_VER(amf_ver) \
{ \
   m_NCS_OS_MEMSET(&amf_ver, 0, sizeof(SaVersionT));  \
   amf_ver.releaseCode='B';   \
   amf_ver.majorVersion=0x01; \
   amf_ver.minorVersion=0x01; \
}

/* Update the SRMSv specific AMF callback functions */
#define m_SRMSV_UPDATE_AMF_CBKS(amf_call_backs) \
{ \
   m_NCS_OS_MEMSET(&amf_call_backs, 0, sizeof(SaAmfCallbacksT));               \
   amf_call_backs.saAmfHealthcheckCallback = srmnd_amf_health_check_callback;  \
   amf_call_backs.saAmfComponentTerminateCallback =                            \
                                     srmnd_amf_component_terminate_callback;   \
   amf_call_backs.saAmfCSISetCallback = srmnd_amf_csi_set_callback;            \
   amf_call_backs.saAmfCSIRemoveCallback = srmnd_amf_csi_remove_callback;      \
}
  
/* Function prototypes */
typedef uns32 (*srmnd_readiness_ha_state_process)(SRMND_CB *cb, 
                                                  SaInvocationT invocation);
EXTERN_C uns32 srmnd_amf_initialize(SRMND_CB *srmnd);
EXTERN_C uns32 srmnd_amf_finalize(SRMND_CB *srmnd);
EXTERN_C uns32 srmnd_healthcheck_start(SRMND_CB *cb);

/* to start the health check */ 
EXTERN_C uns32 srmnd_amf_healthcheck_start_msg_post(SRMND_CB *cb); 
EXTERN_C uns32 srmnd_healthcheck_start(SRMND_CB *cb);

#endif /* SRMND_AMF_H */



