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

  MODULE NAME: EDSV_LOGSTR.H

..............................................................................

  DESCRIPTION: Defines Log register/deregister functions prototype.

******************************************************************************/

#ifndef EDSV_LOGSTR_H
#define EDSV_LOGSTR_H
#ifndef NCSFL_TYPE_TCLILL
#define  NCSFL_TYPE_TCLILL        "TCLILL"	/* tme, C,u32,idx, u32,u32  added for edsv */
#endif
#ifndef NCSFL_TYPE_TCLILLF
#define  NCSFL_TYPE_TCLILLF       "TCLILLF"	/* tme, C,u32,idx, u32,u32,u64  added for edsv */
#endif

/* New categories specific to EDSV Logging */
#define NCSFL_LC_EDSV_DATA       0x00008000	/* EDSV Publish and Subscribe logging */
#define NCSFL_LC_EDSV_CONTROL    0x00004000	/* EDSV Misc APIs logging */
#define NCSFL_LC_EDSV_INIT       0x00002000	/* EDSV Registrations with other services */

uns32 eda_flx_log_ascii_set_reg(void);
uns32 eda_flx_log_ascii_set_dereg(void);

uns32 eds_flx_log_ascii_set_reg(void);
uns32 eds_flx_log_ascii_set_dereg(void);

uns32 eda_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
uns32 eds_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
uns32 edsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);

#endif
