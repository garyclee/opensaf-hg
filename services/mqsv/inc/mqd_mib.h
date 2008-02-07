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

#ifndef MQD_MIB_H
#define MQD_MIB_H


uns32 safmsgscalarobject_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSMIB_VAR_INFO* var_info,
                              NCS_BOOL test_flag);


uns32 safmsgscalarobject_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                 NCSMIB_SETROW_PARAM_VAL* params,
                                 struct ncsmib_obj_info* obj_info,
                                 NCS_BOOL testrow_flag);



uns32 safmsgscalarobject_extract(NCSMIB_PARAM_VAL* param,
                                  NCSMIB_VAR_INFO* var_info,
                                  NCSCONTEXT data,
                                  NCSCONTEXT buffer);


uns32  safmsgscalarobject_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);


uns32  safmsgscalarobject_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
                                NCSCONTEXT *data, uns32* next_inst_id,
                                uns32 *next_inst_id_len);

uns32 samsgqueuegroupentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSMIB_VAR_INFO* var_info,
                              NCS_BOOL test_flag);


uns32 samsgqueuegroupentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                 NCSMIB_SETROW_PARAM_VAL* params,
                                 struct ncsmib_obj_info* obj_info,
                                 NCS_BOOL testrow_flag);


uns32 samsgqueuegroupentry_extract(NCSMIB_PARAM_VAL* param,
                                  NCSMIB_VAR_INFO* var_info,
                                  NCSCONTEXT data,
                                  NCSCONTEXT buffer);


uns32  samsgqueuegroupentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);


uns32  samsgqueuegroupentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
                                NCSCONTEXT *data, uns32* next_inst_id,
                                uns32 *next_inst_id_len);



uns32 samsgqueuegroupmembersentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSMIB_VAR_INFO* var_info,
                              NCS_BOOL test_flag);


uns32 samsgqueuegroupmembersentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                 NCSMIB_SETROW_PARAM_VAL* params,
                                 struct ncsmib_obj_info* obj_info,
                                 NCS_BOOL testrow_flag);
uns32 samsgqueuegroupmembersentry_extract(NCSMIB_PARAM_VAL* param,
                                  NCSMIB_VAR_INFO* var_info,
                                  NCSCONTEXT data,
                                  NCSCONTEXT buffer);

uns32  samsgqueuegroupmembersentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

uns32  samsgqueuegroupmembersentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
                                NCSCONTEXT *data, uns32* next_inst_id,
                                uns32 *next_inst_id_len);

void mqd_get_node_from_group_for_get(MQD_OBJ_NODE *pNode,MQD_OBJ_INFO **qnode,SaNameT memberQueueName);

void mqd_get_node_from_group_for_getnext(MQD_CB *cb,MQD_OBJ_NODE **pNode,MQD_OBJ_INFO **qNode,SaNameT memberQueueName);

uns32 safmsgscalarobject_tbl_reg();
uns32 samsgqueuegroupmembersentry_tbl_reg();
uns32 samsgqueuegroupentry_tbl_reg();

EXTERN_C uns32 mqd_reg_with_miblib(void);
EXTERN_C uns32 mqd_reg_with_mab(MQD_CB *pMqd);
EXTERN_C uns32 mqd_unreg_with_mab(MQD_CB *pMqd);
#endif







