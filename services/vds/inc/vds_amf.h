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

  MODULE NAME: VDS_AMF.H

$Header: 
..............................................................................

  DESCRIPTION: This file contains AMF related data structures and function
               prototypes defined for VDS.

******************************************************************************/

#ifndef VDS_AMF_H
#define VDS_AMF_H

struct vds_cb;

/* VDS HA State */
typedef enum vds_ha_state
{
   VDS_HA_STATE_NULL      = 0, 
   VDS_HA_STATE_ACTIVE    = SA_AMF_HA_ACTIVE, 
   VDS_HA_STATE_STDBY     = SA_AMF_HA_STANDBY,
   VDS_HA_STATE_QUIESCED  = SA_AMF_HA_QUIESCED,
   VDS_HA_STATE_QUIESCING = SA_AMF_HA_QUIESCING,
   VDS_HA_STATE_MAX 
} VDS_HA_STATE;

/* AMF specific attributes */
typedef struct vds_amf_attrib
{
   SaInvocationT   invocation;
   NCS_BOOL        health_check_started;   /* healthcheck start flag */
   SaNameT         comp_name;              /* component Name */
   VDS_HA_STATE    ha_cstate;              /* HA Current state */ 
   SaAmfHandleT    amf_handle;             /* AMF Handle */ 
   SaSelectionObjectT  amf_sel_obj;
   SaAmfHealthcheckKeyT  health_check_key; /* Health check key info */ 
} VDS_AMF_ATTRIB;

#define VDS_AMF_HEALTH_CHECK_KEY  "B9B9" 

#define m_VDS_GET_AMF_VER(amf_ver) \
   amf_ver.releaseCode='B';        \
   amf_ver.majorVersion=0x01;      \
   amf_ver.minorVersion=0x01;

#define VDS_HEALTHCHECK_AMF_INVOKED  SA_AMF_HEALTHCHECK_AMF_INVOKED 
#define VDS_AMF_COMPONENT_FAILOVER   SA_AMF_COMPONENT_FAILOVER

/* Function prototypes */
typedef uns32 (*vds_readiness_ha_state_process)(struct vds_cb *cb, 
                                                SaInvocationT invocation);
EXTERN_C uns32 vds_amf_initialize(struct vds_cb *vds);
EXTERN_C uns32 vds_amf_finalize(struct vds_cb *vds);
EXTERN_C uns32 vds_healthcheck_start(struct vds_cb *cb);
EXTERN_C uns32 vds_amf_healthcheck_start_msg_post(struct vds_cb *cb); 
EXTERN_C uns32 vds_healthcheck_start(struct vds_cb *cb);

#endif /* VDS_AMF_H */



