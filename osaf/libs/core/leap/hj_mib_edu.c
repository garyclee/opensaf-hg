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
 */

/** Get compile time options...**/
#include "ncs_opt.h"

/** Get general definitions...**/
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncs_stack.h"
#include "ncs_mib.h"
#include "ncsmib_mem.h"
#include "ncssysf_def.h"
#include "ncssysf_mem.h"
#include "ncssysf_sem.h"
#include "ncssysf_tmr.h"
#include "ncssysfpool.h"
#include "ncsencdec_pub.h"
#include "usrbuf.h"
#include "ncs_ubaid.h"
#include "ncs_edu.h"
#include "ncs_trap.h"

uns32 ncs_tlvsize_for_ncs_trap_get(NCS_TRAP *trap)
{
	uns32 tsize = 0;
	NCS_TRAP_VARBIND *p_varbind = NULL;

	if (trap == NULL)
		return 0;

	p_varbind = trap->i_trap_vb;
	while (p_varbind != NULL) {
		tsize += EDU_TLV_HDR_SIZE + 4;	/* for i_tbl_id */
		tsize += EDU_TLV_HDR_SIZE + 4;	/* for i_param_val.i_param_id */
		tsize += EDU_TLV_HDR_SIZE + 4;	/* for i_param_val.i_fmat_id */
		tsize += EDU_TLV_HDR_SIZE + 2;	/* for i_param_val.i_length */
		if (p_varbind->i_param_val.i_fmat_id == NCSMIB_FMAT_INT) {
			tsize += EDU_TLV_HDR_SIZE + 4;	/* for i_param_val.info.i_int */
		} else {
			tsize += EDU_TLV_HDR_SIZE + p_varbind->i_param_val.i_length;
			/* for i_param_val.info.i_oct */
			tsize += EDU_TLV_HDR_SIZE + 2;	/* EDU-internal pointer space */
		}
		tsize += EDU_TLV_HDR_SIZE + 4;	/* for i_idx.i_inst_len */
		if (p_varbind->i_idx.i_inst_len != 0) {
			tsize += EDU_TLV_HDR_SIZE + (p_varbind->i_idx.i_inst_len * 4);
			/* for i_idx.i_inst_ids */
		}
		tsize += EDU_TLV_HDR_SIZE + 2;
		/* EDU-internal pointer space for i_idx.i_inst_ids */
		p_varbind = p_varbind->next_trap_varbind;
	}
	tsize += EDU_TLV_HDR_SIZE + 2;
	/* EDU-internal space for linked-list count */

	tsize += EDU_TLV_HDR_SIZE + 4;	/* for i_trap_tbl_id */
	tsize += EDU_TLV_HDR_SIZE + 4;	/* for i_trap_id */
	tsize += EDU_TLV_HDR_SIZE + 4;	/* for i_inform_mgr */

	return tsize;
}
