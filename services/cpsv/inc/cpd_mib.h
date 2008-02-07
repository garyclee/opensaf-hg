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

#ifndef CPD_MIB_H
#define CPD_MIB_H

/* To register the 'sackptcheckpointentry' table information with MibLib */
EXTERN_C uns32  sackptcheckpointentry_tbl_reg(void);
/* To register the 'sackptnodereplicalocentry' table information with MibLib */
EXTERN_C uns32  sackptnodereplicalocentry_tbl_reg(void);
/* To register the 'safckptscalarobject' table information with MibLib */
EXTERN_C uns32  safckptscalarobject_tbl_reg(void);

EXTERN_C uns32 cpd_reg_with_miblib(void);
EXTERN_C uns32 cpd_reg_with_mab(CPD_CB *cb);
EXTERN_C uns32 cpd_unreg_with_mab(CPD_CB *cb);


uns32 sackptcheckpointentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data, uns32* next_inst_id,uns32 *next_inst_id_len);

uns32 sackptcheckpointentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

uns32 sackptcheckpointentry_extract(NCSMIB_PARAM_VAL *param, NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);

uns32  sackptcheckpointentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,NCSMIB_VAR_INFO* var_info,NCS_BOOL test_flag);

uns32 sackptcheckpointentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                   NCSMIB_SETROW_PARAM_VAL* params,
                                   struct ncsmib_obj_info* obj_info,
                                   NCS_BOOL testrow_flag);

uns32 safckptscalarobject_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSMIB_VAR_INFO* var_info,
                              NCS_BOOL test_flag);
uns32 safckptscalarobject_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                 NCSMIB_SETROW_PARAM_VAL* params,
                                 struct ncsmib_obj_info* obj_info,
                                 NCS_BOOL testrow_flag);
uns32 safckptscalarobject_extract(NCSMIB_PARAM_VAL* param,
                                  NCSMIB_VAR_INFO* var_info,
                                  NCSCONTEXT data,
                                  NCSCONTEXT buffer);
uns32  safckptscalarobject_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
uns32  safckptscalarobject_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
                                NCSCONTEXT *data, uns32* next_inst_id,
                                uns32 *next_inst_id_len);
uns32 sackptnodecheckpointconstraintentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                              NCSMIB_VAR_INFO* var_info,
                                              NCS_BOOL test_flag);
uns32  sackptnodecheckpointconstraintentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,                                                   NCSMIB_SETROW_PARAM_VAL* params,                                                   struct ncsmib_obj_info* obj_info,
                                                  NCS_BOOL testrow_flag);
uns32  sackptnodecheckpointconstraintentry_extract(NCSMIB_PARAM_VAL* param,
                                                   NCSMIB_VAR_INFO* var_info,
                                                   NCSCONTEXT data,
                                                   NCSCONTEXT buffer);
uns32  sackptnodecheckpointconstraintentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                               NCSCONTEXT* data);
uns32  sackptnodecheckpointconstraintentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
                                NCSCONTEXT *data, uns32* next_inst_id,
                                uns32 *next_inst_id_len);

uns32 sackptnodereplicalocentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                                   NCSCONTEXT *data);
uns32 sackptnodereplicalocentry_extract(NCSMIB_PARAM_VAL *param, 
                                        NCSMIB_VAR_INFO *var_info, 
                                        NCSCONTEXT data, NCSCONTEXT buffer);
uns32 sackptnodereplicalocentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,                                NCSCONTEXT *data,uns32* next_inst_id,uns32 *next_inst_id_len);
uns32 sackptnodereplicalocentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                   NCSMIB_SETROW_PARAM_VAL* params,
                                   struct ncsmib_obj_info* obj_info,
                                   NCS_BOOL testrow_flag);
uns32 sackptnodereplicalocentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                  NCSMIB_VAR_INFO* var_info,NCS_BOOL test_flag);

#endif
