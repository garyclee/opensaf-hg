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

#include "ncsgl_defs.h"
#include "ncs_mib_pub.h"
#include "ncs_ubaid.h"
#include "ncs_svd.h"
#include "ncspatricia.h"
#include "ncs_iplib.h"
#include "ncsmiblib.h"
#include "ncs_edu_pub.h"
#include "oac_papi.h"
#include "ncssysf_tsk.h"
#include "ncssysf_lck.h"
#include "ncs_main_papi.h"
#include "ncssysf_def.h"
#include "ncs_hdl_pub.h"
#include "ncssysfpool.h"
#include "ncssysf_mem.h"
#include "ncs_cli.h"
#include "mac_papi.h"
#include "ncs_trap.h"


#include "ifsv_ir_mapi.h"
#include "ifd.h"

#define NCS_IFSV_BIND_INTF_REC_NULL ((IFSV_BIND_INFO*)0)

uns32  ncsifsvbindifentry_get(NCSCONTEXT cb, NCSMIB_ARG *p_get_req, NCSCONTEXT* data);

uns32 ncsifsvbindifentry_extract(NCSMIB_PARAM_VAL *param, NCSMIB_VAR_INFO  *var_info, NCSCONTEXT data, NCSCONTEXT buffer);

uns32 ncsifsvbindifentry_next(NCSCONTEXT cb,
                       NCSMIB_ARG *p_next_req,
                       NCSCONTEXT* data,
                       uns32* o_next_inst_id,
                       uns32* o_next_inst_id_len);

uns32 ncsifsvbindifentry_setrow(NCSCONTEXT hdl, NCSMIB_ARG *arg,
                                NCSMIB_SETROW_PARAM_VAL* params,
                                NCSMIB_OBJ_INFO *var_info,
                                NCS_BOOL test_flag);

uns32 ncsifsvbindifentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);

