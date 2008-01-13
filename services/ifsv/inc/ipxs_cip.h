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
FILE NAME: IPXS_CIP.H
DESCRIPTION: Enums, Structures and Function prototypes for IPXS
******************************************************************************/

#ifndef IPXS_CIP_H
#define IPXS_CIP_H

#define IPXS_IPENTRY_INST_MIN_LEN   5
#define IPXS_IPENTRY_INST_MAX_LEN   17
#define IPXS_IFIPENTRY_INST_LEN     1

uns32 ncsifsvipentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
uns32 ncsifsvipentry_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data);
uns32 ncsifsvipentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                       NCSCONTEXT *data, uns32* next_inst_id,
                       uns32 *next_inst_id_len);
uns32 ncsifsvipentry_extract (NCSMIB_PARAM_VAL* param, 
                           NCSMIB_VAR_INFO* var_info,
                           NCSCONTEXT data, NCSCONTEXT buffer);
uns32 ncsifsvipentry_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                          NCSMIB_SETROW_PARAM_VAL* params,
                          struct ncsmib_obj_info* obj_info,
                          NCS_BOOL testrow_flag);

uns32 ncsifsvifipentry_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data);
uns32 ncsifsvifipentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                            NCSCONTEXT *data, uns32* next_inst_id,
                            uns32 *next_inst_id_len);
uns32 ncsifsvifipentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                           NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
uns32 ncsifsvifipentry_extract (NCSMIB_PARAM_VAL* param, 
                                NCSMIB_VAR_INFO* var_info,
                                NCSCONTEXT data, NCSCONTEXT buffer);
uns32 ncsifsvifipentry_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag);
uns32 ncsifsvifipentry_verify_instance(NCSMIB_ARG* args);

/* To register the 'ncsifsvifipentry' table information with MibLib */
EXTERN_C uns32  ncsifsvifipentry_tbl_reg(void);

/* To register the 'ncsifsvipentry' table information with MibLib */
EXTERN_C uns32  ncsifsvipentry_tbl_reg(void);

EXTERN_C uns32 ipxs_ifiprec_get(IPXS_CB *cb, uns32 ifkeyNet,
                                     IPXS_IFIP_INFO **ifip_info);
#endif /* IPXS_CIP_H */
