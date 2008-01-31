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

  This module is the include file for Availability Directors BAM message
  processing.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_BAM_H
#define AVD_BAM_H


EXTERN_C uns32 avd_bam_msg_rcv(uns32 , AVD_BAM_MSG *);
EXTERN_C void avsv_bam_msg_free(AVD_BAM_MSG *);
EXTERN_C void avd_bam_cfg_done_func(AVD_CL_CB *, AVD_EVT *);
EXTERN_C uns32 avd_bam_snd_restart(AVD_CL_CB *);
EXTERN_C void avd_tmr_cfg_exp_func(AVD_CL_CB *, AVD_EVT *);
EXTERN_C uns32 avd_bam_mds_cpy(MDS_CALLBACK_COPY_INFO *);
EXTERN_C void avd_post_restart_evt(AVD_CL_CB *cb);

#endif
