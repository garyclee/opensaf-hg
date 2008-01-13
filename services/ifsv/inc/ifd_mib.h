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
FILE NAME: IFD_MIB.H
DESCRIPTION: Function prototypes used for IfD used For MIB access.
******************************************************************************/

/* Function Prototype */
#ifndef IFD_MIB_H
#define IFD_MIB_H

uns32 ifd_mib_tbl_req (struct ncsmib_arg *args);

uns32 ifd_reg_with_mab(IFSV_CB *cb);

uns32 ifd_unreg_with_mab(IFSV_CB *cb);

uns32 ifd_reg_with_miblib(void);
 #if (NCS_VIP == 1)
   uns32 vip_miblib_reg(void );
   uns32 vip_reg_with_oac(IFSV_CB *cb);
   uns32 vip_unreg_with_oac(IFSV_CB *cb);
 #endif

EXTERN_C uns32 ifmibobjects_tbl_reg(void);

EXTERN_C uns32 intf_tbl_reg(void);

EXTERN_C uns32  ncsifsvifmapentry_tbl_reg(void);

EXTERN_C uns32  ncsifsvifxentry_tbl_reg(void);

uns32 ifmibobjects_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data);
uns32 ifmibobjects_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                   NCSCONTEXT *data, uns32* next_inst_id,
                   uns32 *next_inst_id_len);
uns32 ifmibobjects_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
uns32 ifmibobjects_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer);
uns32 ifmibobjects_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag);

uns32 intf_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data);
uns32 intf_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                   NCSCONTEXT *data, uns32* next_inst_id,
                   uns32 *next_inst_id_len);
uns32 intf_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
uns32 intf_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer);
uns32 intf_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag);


uns32 ncsifsvifxentry_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data);
uns32 ncsifsvifxentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                           NCSCONTEXT *data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
uns32 ncsifsvifxentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                          NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
uns32 ncsifsvifxentry_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                               NCSCONTEXT data, NCSCONTEXT buffer);
uns32 ncsifsvifxentry_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag);
uns32 ncsifsvifxentry_verify_instance(NCSMIB_ARG* args);

uns32 ncsifsvifmapentry_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data);
uns32 ncsifsvifmapentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                             NCSCONTEXT *data, uns32* next_inst_id,
                             uns32 *next_inst_id_len);
uns32 ncsifsvifmapentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                            NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
uns32 ncsifsvifmapentry_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag);
uns32 ncsifsvifmapentry_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer);

#if(NCS_IFSV_BOND_INTF == 1)
   uns32 ncsifsvbindifentry_tbl_reg(void);
#endif


#endif

