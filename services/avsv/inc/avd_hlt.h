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

  This module is the include file for handling Availability Directors 
  health check structure.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_HLT_H
#define AVD_HLT_H



/* Availability directors health check record structure(AVD_HLT):
 * This data structure lives in the AvD and reflects data points
 * associated with the health check record on the AvD.
 */
typedef struct avd_hlt_tag {

   NCS_PATRICIA_NODE         tree_node;         /* key will be the concatination
                                                 * of key_len_net value field 
                                                 * of health check key name. */   
   /* Checkpointing - All fields are  updated one time */
   AVSV_HLT_KEY               key_name;          /* key name structure */

   SaTimeT                   period;            /* the periods at which health check
                                                 * is done */
   SaTimeT                   max_duration;      /* The duration within which the health
                                                 * check needs to be completed. */
   NCS_ROW_STATUS            row_status;        /* row status of this MIB row */
  
   struct avd_hlt_tag       *next_hlt_on_node;  /* reference to next helth check for a particular node */
   
} AVD_HLT;


#define AVD_HLT_NULL ((AVD_HLT *)0)

EXTERN_C AVD_HLT * avd_hlt_struc_crt(AVD_CL_CB *cb,AVSV_HLT_KEY lhlt_chk, NCS_BOOL ckpt);
EXTERN_C AVD_HLT * avd_hlt_struc_find(AVD_CL_CB *cb,AVSV_HLT_KEY lhlt_chk);
EXTERN_C AVD_HLT * avd_hlt_struc_find_next(AVD_CL_CB *cb,AVSV_HLT_KEY lhlt_chk);
EXTERN_C uns32 avd_hlt_struc_del(AVD_CL_CB *cb,AVD_HLT *hlt_chk);
EXTERN_C struct avd_avnd_tag * avd_hlt_node_find(SaNameT comp_name_net, AVD_CL_CB *avd_cb);
EXTERN_C uns32 avd_hlt_node_del_hlt_from_list(AVD_HLT *hlt_chk, struct avd_avnd_tag *node );

EXTERN_C uns32 saamfhealthchecktableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 saamfhealthchecktableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 saamfhealthchecktableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfhealthchecktableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 saamfhealthchecktableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
EXTERN_C void avd_hlt_ack_msg(AVD_CL_CB *cb,AVD_DND_MSG *ack_msg);

#endif
