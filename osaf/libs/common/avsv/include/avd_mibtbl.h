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

  This module is the include file for handling the structures related to MAB.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_MIBTBL_H
#define AVD_MIBTBL_H

EXTERN_C void avd_req_mib_func(AVD_CL_CB *cb, AVD_EVT *evt);
EXTERN_C void avd_qsd_req_mib_func(AVD_CL_CB *cb, AVD_EVT *evt);

EXTERN_C uns32 avsvscalars_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
EXTERN_C uns32 avsvscalars_extract(NCSMIB_PARAM_VAL *param,
				   NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 avsvscalars_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 avsvscalars_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32 avsvscalars_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				  NCSMIB_SETROW_PARAM_VAL *params,
				  struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

EXTERN_C uns32 avsvscalars_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);
EXTERN_C uns32 saamfscalars_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);
EXTERN_C uns32 safclmscalarobject_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);

EXTERN_C uns32 saamfscalars_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
EXTERN_C uns32 saamfscalars_extract(NCSMIB_PARAM_VAL *param,
				    NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 saamfscalars_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfscalars_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				 NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32 saamfscalars_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				   NCSMIB_SETROW_PARAM_VAL *params,
				   struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

EXTERN_C uns32 safclmscalarobject_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
EXTERN_C uns32 safclmscalarobject_extract(NCSMIB_PARAM_VAL *param,
					  NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 safclmscalarobject_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 safclmscalarobject_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				       NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32 safclmscalarobject_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
					 NCSMIB_SETROW_PARAM_VAL *params,
					 struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

/* The following are the MIB LIB created functions for which the forward declaration
 * is provided here
 */
EXTERN_C uns32 avsvscalars_tbl_reg(void);
EXTERN_C uns32 safclmscalarobject_tbl_reg(void);
EXTERN_C uns32 saamfscalars_tbl_reg(void);
EXTERN_C uns32 saclmnodetableentry_tbl_reg(void);
EXTERN_C uns32 ncsndtableentry_tbl_reg(void);
EXTERN_C uns32 saamfnodetableentry_tbl_reg(void);
EXTERN_C uns32 ncssgtableentry_tbl_reg(void);
EXTERN_C uns32 saamfsgtableentry_tbl_reg(void);
EXTERN_C uns32 ncssutableentry_tbl_reg(void);
EXTERN_C uns32 saamfsutableentry_tbl_reg(void);
EXTERN_C uns32 ncssitableentry_tbl_reg(void);
EXTERN_C uns32 saamfsitableentry_tbl_reg(void);
EXTERN_C uns32 saamfsuspersirankentry_tbl_reg(void);
EXTERN_C uns32 saamfsgsirankentry_tbl_reg(void);
EXTERN_C uns32 saamfsgsurankentry_tbl_reg(void);
EXTERN_C uns32 saamfsisideptableentry_tbl_reg(void);
EXTERN_C uns32 saamfcomptableentry_tbl_reg(void);
EXTERN_C uns32 saamfcompcstypesupportedtableentry_tbl_reg(void);
EXTERN_C uns32 saamfcsitableentry_tbl_reg(void);
EXTERN_C uns32 saamfcsinamevaluetableentry_tbl_reg(void);
EXTERN_C uns32 saamfcstypeparamentry_tbl_reg(void);
EXTERN_C uns32 saamfsusitableentry_tbl_reg(void);
EXTERN_C uns32 saamfhealthchecktableentry_tbl_reg(void);

#endif
