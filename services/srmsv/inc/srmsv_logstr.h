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

  This module contains the extern declaration for string arrays 
  provided by srmsv_logstr.c
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef SRMSV_LOGSTR_H
#define SRMSV_LOGSTR_H

#if (NCS_DTS == 1)
EXTERN_C const NCSFL_STR srmsv_seapi_set[];
EXTERN_C const NCSFL_STR srmsv_mds_set[];
EXTERN_C const NCSFL_STR srmsv_edu_set[];
EXTERN_C const NCSFL_STR srmsv_lock_set[];
EXTERN_C const NCSFL_STR srmsv_mem_set[];
EXTERN_C const NCSFL_STR srmsv_tmr_set[];
EXTERN_C const NCSFL_STR srmsv_tim_set[];
EXTERN_C const NCSFL_STR srmsv_pat_set[];
EXTERN_C const NCSFL_STR srmsv_cb_set[];
EXTERN_C const NCSFL_STR srmsv_hdl_set[];
EXTERN_C const NCSFL_STR srmsv_rsrc_mon_set[];
EXTERN_C const NCSFL_STR srmsv_mbx_set[];
EXTERN_C uns32 srma_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
EXTERN_C uns32 srmnd_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
EXTERN_C uns32 srmsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
#endif /* (NCS_DTS == 1) */

#endif /* SRMSV_LOGSTR_H */
