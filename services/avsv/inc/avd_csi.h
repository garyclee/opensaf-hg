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
  component service Instance structure and its relationship structures.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_CSI_H
#define AVD_CSI_H

struct avd_su_si_rel_tag;

/* The param value  structure for the CSIs. */
typedef struct avd_csi_param_tag {
   NCS_AVSV_ATTR_NAME_VAL         param;   /* param value */
   NCS_ROW_STATUS            row_status;   /* row status of this MIB row */
   struct avd_csi_param_tag *param_next;   /* the next param in the list of 
                                            * params in the CSI.  */
} AVD_CSI_PARAM;

/* Availability directors Component service in.stance structure(AVD_CSI):
 * This data structure lives in the AvD and reflects data points
 * associated with the CSI on the AvD.
 */
typedef struct avd_csi_tag {
   NCS_PATRICIA_NODE   tree_node;            /* key will be the CSI name */

   SaNameT             name_net;             /* name of the CSI with the length
                                              * field in the network order.
                                              *  It is used as the index
                                              * Checkpointing - Sent as a one time update.
                                              */

   SaNameT             csi_type;             /* the prototype name of the CSI
                                              * to which the instance belongs.
                                              * Checkpointing - Sent as a one time update.
                                              */

   uns32               rank;                 /* The rank of the CSI in the SI 
                                              * Checkpointing - Sent as a one time update.
                                              */

   AVD_SI              *si;                  /* SI encompassing this csi */  

   NCS_ROW_STATUS      row_status;           /* row status of this MIB row
                                              * Checkpointing - Sent as a one time update.
                                              */
   /* Generated at standby */
   uns32               num_active_params;    /* Ths number of active params
                                              * in the list. */

   AVD_CSI_PARAM       *list_param;          /* list of all the params of this.
                                              * CSI.
                                              * Checkpointing - Sent as a one time update.
                                              */

   NCS_DB_LINK_LIST    pg_node_list;         /* list of nodes on which pg is 
                                              * tracked for this csi */
   struct avd_csi_tag  *si_list_of_csi_next; /* the next CSI in the list of  component service
                                              * instances in the Service instance  */
   struct avd_comp_csi_rel_tag *list_compcsi;/* The list of compcsi relationship
                                              * wrt to this CSI. */
   uns32                       compcsi_cnt;  /* no of comp-csi rels */
   
} AVD_CSI;

/* This data structure lives in the AvD and reflects relationship
 * between the component and CSI on the AvD.
 */
typedef struct avd_comp_csi_rel_tag {

   AVD_CSI                     *csi;               /* CSI to which this relationship with
                                                    * component exists */
   AVD_COMP                    *comp;              /* component to which this relationship
                                                    * with CSI exists */
   struct avd_su_si_rel_tag    *susi;              /* bk ptr to the su-si rel */
   struct avd_comp_csi_rel_tag *susi_csicomp_next; /* The next element in the list w.r.t to
                                                    * susi relationship structure */
   struct avd_comp_csi_rel_tag *csi_csicomp_next; /* The next element in the list w.r.t to
                                                    * CSI structure */
} AVD_COMP_CSI_REL;


/* CSTypeParam table index structure */
typedef struct avd_cs_type_param_index_tag {

   SaNameT                   type_name_net;     /* Name of the CSI Type with the length
                                                 * field in the network order. */
   SaNameT                   param_name_net;    /* Name of the param with the length
                                                 * field in the network order. */ 

}AVD_CS_TYPE_PARAM_INDX;


/* Availability directors CSI Type and Param relationship. 
 * This Data structure lives in the AVD and is maintained as a patricia tree 
 * from the AVD Control Block.
 */
typedef struct avd_cs_type_param_tag {

   NCS_PATRICIA_NODE         tree_node;         /* key will be the CSI Type name and param name */
   AVD_CS_TYPE_PARAM_INDX    indx;              /* Table index */
   NCS_ROW_STATUS            row_status;        /* row status of this MIB row */
   
}AVD_CS_TYPE_PARAM;

#define AVD_CS_TYPE_PARAM_NULL ((AVD_CS_TYPE_PARAM *)0)
#define AVD_CSI_NULL ((AVD_CSI *)0)
#define AVD_CSI_PARAM_NULL ((AVD_CSI_PARAM *)0)
#define AVD_COMP_CSI_REL_NULL ((AVD_COMP_CSI_REL *)0)


/* creates and adds CSI structure to database. */
EXTERN_C AVD_CSI * avd_csi_struc_crt(AVD_CL_CB *cb,SaNameT csi_name,NCS_BOOL ckpt);
/* Finds CSI structure from the database.*/
EXTERN_C AVD_CSI * avd_csi_struc_find(AVD_CL_CB *cb,SaNameT csi_name,NCS_BOOL host_order);
/* Finds the next CSI structure from the database.*/
EXTERN_C AVD_CSI * avd_csi_struc_find_next(AVD_CL_CB *cb,SaNameT csi_name,NCS_BOOL host_order);
/* deletes and frees CSI structure from database. */
EXTERN_C uns32 avd_csi_struc_del(AVD_CL_CB *cb,AVD_CSI *csi);

EXTERN_C void avd_csi_add_si_list(AVD_CL_CB *cb,AVD_CSI *csi);
EXTERN_C void avd_csi_del_si_list(AVD_CL_CB *cb,AVD_CSI *csi);
EXTERN_C uns32 avd_csi_param_del(AVD_CSI *csi, AVD_CSI_PARAM *param);

/* creates and adds compCSI structure to the list of compCSI in SUSI. */
EXTERN_C AVD_COMP_CSI_REL * avd_compcsi_struc_crt(AVD_CL_CB *cb, 
                                                  struct avd_su_si_rel_tag *susi);
/* deletes and frees all compCSI structure from
 * the list of compCSI in SUSI.
 */
EXTERN_C uns32 avd_compcsi_list_del(AVD_CL_CB *cb,struct avd_su_si_rel_tag *susi,NCS_BOOL ckpt);
/* This function is one of the get processing
 * routines for objects in SA_AMF_C_S_I_TABLE_ENTRY_ID table.*/
EXTERN_C uns32 saamfcsitableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
/* This function is one of the get processing
 * function for objects in SA_AMF_C_S_I_TABLE_ENTRY_ID table.*/
EXTERN_C uns32 saamfcsitableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
/* This function is the set processing for objects in
 * SA_AMF_C_S_I_TABLE_ENTRY_ID table.*/
EXTERN_C uns32 saamfcsitableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
/* This function is the next processing for objects in
 * SA_AMF_C_S_I_TABLE_ENTRY_ID table.*/
EXTERN_C uns32 saamfcsitableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
/* This function is the setrow processing for
 * objects in SA_AMF_C_S_I_TABLE_ENTRY_ID table.*/
EXTERN_C uns32 saamfcsitableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);

/* This function is one of the get processing
 * routines for objects in SA_AMF_C_S_I_NAME_VALUE_TABLE_ENTRY_ID table.*/
EXTERN_C uns32 saamfcsinamevaluetableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
/* This function is one of the get processing
 * function for objects in SA_AMF_C_S_I_NAME_VALUE_TABLE_ENTRY_ID table.*/
EXTERN_C uns32 saamfcsinamevaluetableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
/* This function is the set processing for objects in
 * SA_AMF_C_S_I_NAME_VALUE_TABLE_ENTRY_ID table.*/
EXTERN_C uns32 saamfcsinamevaluetableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
/* This function is the next processing for objects in
 * SA_AMF_C_S_I_NAME_VALUE_TABLE_ENTRY_ID table.*/
EXTERN_C uns32 saamfcsinamevaluetableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
/* This function is the setrow processing for
 * objects in SA_AMF_C_S_I_NAME_VALUE_TABLE_ENTRY_ID table.*/
EXTERN_C uns32 saamfcsinamevaluetableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);






EXTERN_C AVD_CS_TYPE_PARAM * avd_cs_type_param_struc_crt(AVD_CL_CB *cb,
                                                         AVD_CS_TYPE_PARAM_INDX indx);

EXTERN_C AVD_CS_TYPE_PARAM * avd_cs_type_param_struc_find(AVD_CL_CB *cb,
                                                         AVD_CS_TYPE_PARAM_INDX indx);

EXTERN_C AVD_CS_TYPE_PARAM * avd_cs_type_param_struc_find_next(AVD_CL_CB *cb,
                                                         AVD_CS_TYPE_PARAM_INDX indx);

EXTERN_C uns32 avd_cs_type_param_struc_del(AVD_CL_CB *cb,
                                                         AVD_CS_TYPE_PARAM *type_param);

EXTERN_C uns32 avd_cs_type_param_find_match(AVD_CL_CB *cb, SaNameT csi_type, 
                                                         SaNameT param_name);



EXTERN_C uns32 saamfcstypeparamentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                  NCSCONTEXT* data);

EXTERN_C uns32 saamfcstypeparamentry_extract(NCSMIB_PARAM_VAL* param,
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);

EXTERN_C uns32 saamfcstypeparamentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);

EXTERN_C uns32 saamfcstypeparamentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);

EXTERN_C uns32 saamfcstypeparamentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
                             
#endif
