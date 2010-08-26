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
#ifndef AVND_CKPT_UPDT_H
#define AVND_CKPT_UPDT_H

/* Function Definations of avnd_ckpt_updt.c */
EXTERN_C uns32 avnd_ext_comp_data_clean_up(AVND_CB *cb, NCS_BOOL);
EXTERN_C uns32 avnd_ckpt_add_rmv_updt_su_data(AVND_CB *cb, AVND_SU *su, NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avnd_ckpt_add_rmv_updt_comp_data(AVND_CB *cb, AVND_COMP *comp, NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avnd_ckpt_add_rmv_updt_csi_data(AVND_CB *cb, AVND_COMP_CSI_REC *csi, NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avnd_ckpt_add_rmv_updt_hlt_data(AVND_CB *cb, AVND_HC *hlt, NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avnd_ckpt_add_rmv_updt_comp_hlt_rec(AVND_CB *cb, AVND_COMP_HC_REC *hlt, NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avnd_ckpt_add_rmv_updt_comp_cbk_rec(AVND_CB *cb, AVND_COMP_CBK *cbk, NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avnd_ckpt_add_rmv_updt_su_si_rec(AVND_CB *cb, AVND_SU_SI_REC *su_si_ckpt, NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 avnd_ckpt_add_rmv_updt_su_siq_rec(AVND_CB *cb, AVND_SU_SIQ_REC *su_siq_ckpt, NCS_MBCSV_ACT_TYPE action);

/* macro to remove a csi-record from the si-csi list */
#define m_AVND_SU_SI_CSI_REC_REM(si, csi) \
           ncs_db_link_list_delink(&(si).csi_list, &(csi).si_dll_node)

#endif
