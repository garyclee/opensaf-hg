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

  DESCRIPTION:

  This file includes definitions related to CLM APIs.
  
******************************************************************************
*/

#ifndef AVSV_CLMPARAM_H
#define AVSV_CLMPARAM_H

#include <saClm.h>

/* CLM API enums */
typedef enum avsv_clm_api_type {
	AVSV_CLM_INITIALIZE = 1,
	AVSV_CLM_FINALIZE,
	AVSV_CLM_TRACK_START,
	AVSV_CLM_TRACK_STOP,
	AVSV_CLM_NODE_GET,
	AVSV_CLM_NODE_ASYNC_GET,
	AVSV_CLM_SEL_OBJ_GET,
	AVSV_CLM_DISPATCH,
	AVSV_CLM_API_MAX
} AVSV_CLM_API_TYPE;

/* CLM Callback API enums */
typedef enum avsv_clm_cbk_type {
	AVSV_CLM_CBK_TRACK = 1,
	AVSV_CLM_CBK_NODE_ASYNC_GET,
	AVSV_CLM_CBK_MAX
} AVSV_CLM_CBK_TYPE;

/* 
 * API & callback API parameter definitions.
 * These definitions are used for 
 * 1) encoding & decoding messages between AvND & CLA.
 * 2) storing the callback parameters in the pending callback list (on CLA).
 */

/*** API Parameter definitions ***/

/* initialize - not reqd */
typedef struct avsv_clm_init_param_tag {
	void *dummy;
} AVSV_CLM_INIT_PARAM;

/* selection object get - not reqd */
typedef struct avsv_clm_sel_obj_get_param_tag {
	void *dummy;
} AVSV_CLM_SEL_OBJ_GET_PARAM;

/* dispatch  - not reqd */
typedef struct avsv_clm_dispatch_param_tag {
	void *dummy;
} AVSV_CLM_DISPATCH_PARAM;

/* finalize */
typedef struct avsv_clm_finalize_param_tag {
	SaClmHandleT hdl;	/* CLM handle */
} AVSV_CLM_FINALIZE_PARAM;

/* track start */
typedef struct avsv_clm_track_start_param_tag {
	SaClmHandleT hdl;	/* CLM handle */
	SaUint8T flags;		/* track flags */
	SaUint64T viewNumber;	/* view number */
} AVSV_CLM_TRACK_START_PARAM;

/* track stop */
typedef struct avsv_clm_track_stop_param_tag {
	SaClmHandleT hdl;	/* CLM handle */
} AVSV_CLM_TRACK_STOP_PARAM;

/* node get */
typedef struct avsv_clm_node_get_param_tag {
	SaClmHandleT hdl;	/* CLM handle */
	SaClmNodeIdT node_id;	/* node-id */
} AVSV_CLM_NODE_GET_PARAM;

/* node async get */
typedef struct avsv_clm_node_async_get_param_tag {
	SaClmHandleT hdl;	/* CLM handle */
	SaInvocationT inv;	/* invocation value */
	SaClmNodeIdT node_id;	/* node-id */
} AVSV_CLM_NODE_ASYNC_GET_PARAM;

/*** Callback Parameter definitions ***/

/* clm track */
typedef struct avsv_clm_cbk_track_param_tag {
	SaClmClusterNotificationBufferT notify;	/* notification buffer */
	SaUint32T mem_num;	/* number of members */
	SaAisErrorT err;	/* error */
} AVSV_CLM_CBK_TRACK_PARAM;

/* node async get */
typedef struct avsv_clm_cbk_node_async_get_param_tag {
	SaInvocationT inv;	/* invocation value */
	SaClmClusterNodeT node;	/* cluster node */
	SaAisErrorT err;	/* error */
} AVSV_CLM_CBK_NODE_ASYNC_GET_PARAM;

/* wrapper structure for all the callbacks */
typedef struct avsv_clm_cbk_info {
	uns32 hdl;		/* CLM handle */
	AVSV_CLM_CBK_TYPE type;	/* callback type */
	union {
		AVSV_CLM_CBK_TRACK_PARAM track;
		AVSV_CLM_CBK_NODE_ASYNC_GET_PARAM node_get;
	} param;
} AVSV_CLM_CBK_INFO;

#endif   /* !AVSV_CLMPARAM_H */
