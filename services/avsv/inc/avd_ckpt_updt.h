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

  This module is the include file for Availability Directors checkpointing.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_CKPT_UPDT_H
#define AVD_CKPT_UPDT_H

/* Function Definations of avd_ckpt_updt.c */
EXTERN_C uns32  avsv_ckpt_add_rmv_updt_avnd(AVD_CL_CB *cb, AVD_AVND *avnd, 
                                   NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32  avsv_ckpt_add_rmv_updt_sg_data(AVD_CL_CB *cb, AVD_SG *sg, 
                                      NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32  avsv_ckpt_add_rmv_updt_su_data(AVD_CL_CB *cb, AVD_SU *su, 
                                      NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32  avsv_ckpt_add_rmv_updt_si_data(AVD_CL_CB *cb, AVD_SI *si,
                                      NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avsv_decode_add_rmv_su_oper_list(AVD_CL_CB *cb, AVD_SU *su_ptr,
                                        NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avsv_update_sg_admin_si(AVD_CL_CB *cb, NCS_UBAID   *uba,
                               NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avsv_updt_su_si_rel(AVD_CL_CB *cb, AVSV_SU_SI_REL_CKPT_MSG  *su_si_ckpt,
                           NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avsv_ckpt_add_rmv_updt_comp_data(AVD_CL_CB *cb, AVD_COMP *comp, 
                                        NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avsv_ckpt_add_rmv_updt_csi_data(AVD_CL_CB *cb, AVD_CSI *csi, 
                                       NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avsv_ckpt_add_rmv_updt_hlt_data(AVD_CL_CB *cb, AVD_HLT *hlt, 
                                       NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32  avsv_ckpt_add_rmv_updt_sus_per_si_rank_data(AVD_CL_CB *cb, 
                                       AVD_SUS_PER_SI_RANK *su_si_rank,
                                       NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32  avsv_ckpt_add_rmv_updt_comp_cs_type_data(AVD_CL_CB *cb, 
                                       AVD_COMP_CS_TYPE *comp_cs_type,
                                       NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32  avsv_ckpt_add_rmv_updt_cs_type_param_data(AVD_CL_CB *cb, 
                                       AVD_CS_TYPE_PARAM *cs_type_param,
                                       NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avd_data_clean_up(AVD_CL_CB *cb);

EXTERN_C uns32  avsv_ckpt_add_rmv_updt_si_dep_data(AVD_CL_CB *cb, AVD_SI_SI_DEP *si_dep,
                                      NCS_MBCSV_ACT_TYPE action);
#endif
