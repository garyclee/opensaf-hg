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

 _Public_ Distributed Tracing Agent (DTA) abstractions and function prototypes

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef DTA_PAPI_H
#define DTA_PAPI_H

#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncs_iplib.h"
#include "os_defs.h"
#include "ncs_main_papi.h"
#include "ncssysf_tsk.h"

#include "ncs_svd.h"
#include "ncs_ubaid.h"
#include "ncssysf_def.h"
#include "ncs_lib.h"
#include "ncssysf_def.h"

#include "ncs_log.h"

#define DEFAULT_INST_ID  1
/***************************************************************************
 * De-Register service with dtsv
 ***************************************************************************/

typedef struct ncs_unbind_svc {
	SS_SVC_ID svc_id;	/* Service ID of the service wants to register with dtsv */

} NCS_UNBIND_SVC;
/***************************************************************************
 * Register service with dtsv
 ***************************************************************************/

typedef struct ncs_bind_svc {
	SS_SVC_ID svc_id;	/* Service ID of the service wants to register with dtsv */
	uns16 version;		/* version of the ASCII_SPEC service intends to use */
	char svc_name[15];	/* Service name as is given in corr ASCII_SPEC table */

} NCS_BIND_SVC;

/***************************************************************************
 * The operations set that a DTA instance supports
 ***************************************************************************/

typedef enum ncs_dtsv_op {
	NCS_DTSV_OP_BIND,
	NCS_DTSV_OP_UNBIND
} NCS_DTSV_OP;

/***************************************************************************
 * The DTSV API single entry point for all services
 ***************************************************************************/
typedef struct ncs_dtsv_rq {
	NCS_DTSV_OP i_op;	/* Operation; Bind, Un-Bind, log msg */

	union {
		NCS_BIND_SVC bind_svc;
		NCS_UNBIND_SVC unbind_svc;

	} info;

} NCS_DTSV_RQ;

/***************************************************************************
 * DTA public API's for binding service and logging message.
 ***************************************************************************/

uns32 ncs_dtsv_su_req(NCS_DTSV_RQ *arg);

uns32 ncs_logmsg(SS_SVC_ID svc_id,
				     uns8 fmat_id,
				     uns8 str_table_id, uns32 category, uns8 severity, char *fmat_type, ...);

uns32 ncs_logmsg_v2(SS_SVC_ID svc_id,
					uns32 inst_id,
					uns8 fmat_id,
					uns8 str_table_id, uns32 category, uns8 severity, char *fmat_type, ...);

#endif   /* DTA_PAPI_H */
