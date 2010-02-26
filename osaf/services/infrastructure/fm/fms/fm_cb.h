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

#ifndef FM_CB_H
#define FM_CB_H

#define FM_PID_FILE PKGPIDDIR "fms.pid"

EXTERN_C uns32 gl_fm_hdl;

typedef enum {
	FM_TMR_TYPE_MIN,
	FM_TMR_PROMOTE_ACTIVE,
	FM_TMR_RESET_RETRY,
	FM_TMR_TYPE_MAX
} FM_TMR_TYPE;

typedef enum {
	FM_TMR_STATUS_MIN,
	FM_TMR_RUNNING,
	FM_TMR_STOPPED,
	FM_TMR_STATUS_MAX
} FM_TMR_STATUS;

typedef struct fm_tmr {
	tmr_t tmr_id;
	FM_TMR_TYPE type;
	FM_TMR_STATUS status;
} FM_TMR;

/* FM control block */
typedef struct fm_cb {

	uns8 shelf;
	uns8 slot;
	uns8 sub_slot;
	uns32 cb_hdl;
	SYSF_MBX mbx;

	/* FM AMF CB */
	FM_AMF_CB fm_amf_cb;

	uns8 peer_shelf;
	uns8 peer_slot;
	uns8 peer_sub_slot;
	MDS_DEST peer_adest;	/* will be 0 if peer is not up */

	/* Holds own role. */
	PCS_RDA_ROLE role;

	/* MDS handles. */
	MDS_DEST adest;
	MDS_HDL adest_hdl;
	MDS_HDL adest_pwe1_hdl;

	/* Timers */
	FM_TMR promote_active_tmr;
	FM_TMR reset_retry_tmr;

	/* Time in terms of one hundredth of seconds (500 for 5 secs.) */
	uns32 active_promote_tmr_val;
	uns32 reset_retry_tmr_val;
	NCS_BOOL is_platform;
} FM_CB;

extern char *role_string[];

/*****************************************************************
 *         Prototypes for extern functions                       *
 *****************************************************************/
EXTERN_C uns32 fm_rda_init(FM_CB *);
EXTERN_C uns32 fm_rda_finalize(FM_CB *);
EXTERN_C void fm_rda_callback(uns32, PCS_RDA_CB_INFO *, PCSRDA_RETURN_CODE);
EXTERN_C uns32 fm_rda_set_role(FM_CB *, PCS_RDA_ROLE);

#endif
