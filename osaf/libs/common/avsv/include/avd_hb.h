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

  This module is the include file for Availability Directors heartbeat
  processing.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_HB_H
#define AVD_HB_H

struct avd_evt_tag;

EXTERN_C uns32 avd_init_heartbeat(AVD_CL_CB *cb);
EXTERN_C void avd_rcv_heartbeat(void);
EXTERN_C void avd_d2d_heartbeat_msg_func(AVD_CL_CB *cb);
EXTERN_C void avd_tmr_snd_hb_func(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C void avd_tmr_rcv_hb_d_func(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C void avd_tmr_rcv_hb_init_func(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C void avd_mds_avd_up_func(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C void avd_mds_avd_down_func(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C void avd_standby_tmr_rcv_hb_d_func(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C void avd_standby_avd_down_func(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C uns32 avd_d2d_msg_rcv(uns32 cb_hdl, AVD_D2D_MSG *rcv_msg);

#endif
