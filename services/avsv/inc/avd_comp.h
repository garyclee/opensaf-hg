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
  component structure.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_COMP_H
#define AVD_COMP_H


/* Availability directors Component structure(AVD_COMP): 
 * This data structure lives in the AvD and reflects data points
 * associated with the component on the AvD.
 */
typedef struct avd_cmp_tag {

   NCS_PATRICIA_NODE               tree_node;    /* key will be the component name */
   /* Detailed as in data structure definition */
   AVSV_COMP_INFO                  comp_info;    /* component name field with 
                                                  * the length field in the 
                                                  * network order is used as the
                                                  * index. */
   SaTimeT                      inst_retry_delay;/* Delay interval after which
                                                  * the component is reinstantiated.
                                                  * Checkpointing - Sent as a one time update.
                                                  */

   NCS_BOOL                 nodefail_cleanfail;  /* If flag set to true node will
                                                  * be considered failed when the
                                                  * cleanup script fails.
                                                  * Checkpointing - Sent as a one time update.
                                                  */

   uns32                     max_num_inst_delay; /* the maximum number of times
                                                  * AMF tries to instantiate
                                                  * the component with delay.
                                                  * Checkpointing - Sent as a one time update.
                                                  */

   AVD_SU                          *su;          /* SU to which this component belongs */ 

   NCS_ROW_STATUS                  row_status;   /* row status of this MIB row 
                                                  * Checkpointing - Sent as a one time update.
                                                  */

   SaUint32T                    max_num_csi_actv;/* number of CSI relationships that can be
                                                  * assigned active to this component 
                                                  * Checkpointing - Sent as a one time update.
                                                  */

   SaUint32T                   max_num_csi_stdby;/* number of CSI relationships that can be
                                                  * assigned standby to this component 
                                                  * Checkpointing - Sent as a one time update.
                                                  */

   SaUint32T                   curr_num_csi_actv;/* the number of CSI relationships that have
                                                  * been assigned active to this component
                                                  * Checkpointing - Sent update independently.
                                                  */

   SaUint32T                  curr_num_csi_stdby;/* the number of CSI relationships that have
                                                  * been assigned standby to this component
                                                  * Checkpointing - Sent update independently.
                                                  */

   NCS_OPER_STATE                  oper_state;   /* component operational state
                                                  * Checkpointing - Sent update independently.
                                                  */
   NCS_READINESS_STATE        readiness_state;   /* component readiness state */

   NCS_PRES_STATE                  pres_state;   /* component presence state.
                                                  * Checkpointing - Sent update independently.
                                                  */

   struct avd_cmp_tag              *su_comp_next;/* the next component in list of  components
                                                  * in this SU */
   SaNameT                   proxy_comp_name_net;/* Name of the current proxy component.
                                                  * only valid if the component is proxied 
                                                  * component,the len field is in n/w order */

   uns32                           restart_cnt;  /* The number times the 
                                                  * component restarted.
                                                  * Checkpointing - Sent update independently.
                                                  */
   
   NCS_BOOL                         assign_flag;  /* Flag used while assigning. to mark this
                                                  * comp has been assigned a CSI from
                                                  * current SI being assigned
                                                  */  
          
} AVD_COMP;


typedef struct avd_comp_cs_type_indx_tag {
  
   SaNameT           comp_name_net;             /* component name field with 
                                                  * the length field in the 
                                                  * network order is used as the
                                                  * primary index. */
   SaNameT           csi_type_name_net;         /* CSI Type name field with 
                                                  * the length field in the 
                                                  * network order is used as the
                                                  * secondary index. */
}AVD_COMP_CS_TYPE_INDX;


typedef struct avd_comp_cs_type_tag {

    NCS_PATRICIA_NODE               tree_node;    /* key will be the component name and 
                                                   * CSI Type Name */
    AVD_COMP_CS_TYPE_INDX           indx;         /* Index */
    NCS_ROW_STATUS                  row_status;   /* row status of this MIB row */

}AVD_COMP_CS_TYPE;


#define AVD_COMP_CS_TYPE_NULL  ((AVD_COMP_CS_TYPE *)0)

#define AVD_COMP_NULL ((AVD_COMP *)0)


EXTERN_C AVD_COMP * avd_comp_struc_crt(AVD_CL_CB *cb,SaNameT comp_name,NCS_BOOL ckpt);
EXTERN_C AVD_COMP * avd_comp_struc_find(AVD_CL_CB *cb,SaNameT comp_name,NCS_BOOL host_order);
EXTERN_C AVD_COMP * avd_comp_struc_find_next(AVD_CL_CB *cb,SaNameT comp_name,NCS_BOOL host_order);
EXTERN_C uns32 avd_comp_struc_del(AVD_CL_CB *cb,AVD_COMP *comp);
EXTERN_C void avd_comp_del_su_list(AVD_CL_CB *cb,AVD_COMP *comp);
EXTERN_C uns32 saamfcomptableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 saamfcomptableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 saamfcomptableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfcomptableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 saamfcomptableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
EXTERN_C void avd_comp_ack_msg(AVD_CL_CB *cb,AVD_DND_MSG *ack_msg);



EXTERN_C AVD_COMP_CS_TYPE * avd_comp_cs_type_struc_crt(AVD_CL_CB *cb, AVD_COMP_CS_TYPE_INDX indx);

EXTERN_C AVD_COMP_CS_TYPE * avd_comp_cs_type_struc_find(AVD_CL_CB *cb, AVD_COMP_CS_TYPE_INDX indx);

EXTERN_C AVD_COMP_CS_TYPE * avd_comp_cs_type_struc_find_next(AVD_CL_CB *cb, AVD_COMP_CS_TYPE_INDX indx);

EXTERN_C uns32 avd_comp_cs_type_struc_del(AVD_CL_CB *cb, AVD_COMP_CS_TYPE *cst);

EXTERN_C uns32  avd_comp_cs_type_find_match(AVD_CL_CB *cb, struct avd_csi_tag *csi,AVD_COMP *comp);

EXTERN_C uns32 saamfcompcstypesupportedtableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                                     NCSCONTEXT* data);
EXTERN_C uns32 saamfcompcstypesupportedtableentry_extract(NCSMIB_PARAM_VAL* param,
                                       NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                                       NCSCONTEXT buffer);
EXTERN_C uns32 saamfcompcstypesupportedtableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                       NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfcompcstypesupportedtableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                       NCSCONTEXT* data, uns32* next_inst_id,
                                       uns32 *next_inst_id_len);
EXTERN_C uns32 saamfcompcstypesupportedtableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                       NCSMIB_SETROW_PARAM_VAL* params,
                                       struct ncsmib_obj_info* obj_info,
                                       NCS_BOOL testrow_flag);

#endif
