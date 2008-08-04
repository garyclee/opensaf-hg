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
  provided by vds_logstr.c
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef VDS_LOGSTR_H
#define VDS_LOGSTR_H

#if (NCS_DTS == 1)
EXTERN_C const NCSFL_STR vds_mds_set[];
EXTERN_C const NCSFL_STR vds_lock_set[];
EXTERN_C const NCSFL_STR vds_mem_set[];
EXTERN_C const NCSFL_STR vds_pat_set[];
EXTERN_C const NCSFL_STR vds_cb_set[];
EXTERN_C const NCSFL_STR vds_hdl_set[];
EXTERN_C const NCSFL_STR vds_mbx_set[];
EXTERN_C const NCSFL_STR vds_amf_set[];
EXTERN_C const NCSFL_STR vds_tim_set[];
EXTERN_C const NCSFL_STR vds_misc_set[];
EXTERN_C const NCSFL_STR vds_ckpt_set[];
EXTERN_C const NCSFL_STR vds_amf_set[];
EXTERN_C const NCSFL_STR vds_evt_set[];

EXTERN_C uns32 vds_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
EXTERN_C uns32 vds_logstr_reg(void);
EXTERN_C uns32 vds_logstr_unreg(void);

#endif /* (NCS_DTS == 1) */
#endif /* VDS_LOGSTR_H */

