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
FILE NAME: IFSV_MIB.H
DESCRIPTION: Function prototypes used for IfD/IfND used For MIB access.
******************************************************************************/

/* Function Prototype */
#ifndef IFSV_MIB_H
#define IFSV_MIB_H

/* Instence ID lenght for interface key */
#define IFSV_IFINDEX_INST_LEN    1
#define IFSV_IFMAPTBL_INST_LEN   6

EXTERN_C uns32  ifentry_tbl_reg(void);

uns32 ifsv_ifrec_get(IFSV_CB *cb, uns32 ifkey, 
                          NCS_IFSV_INTF_REC *ifmib_rec);
uns32 ifsv_ifrec_getnext(IFSV_CB *cb, uns32 ifkey, 
                          NCS_IFSV_INTF_REC *ifmib_rec);

uns32 ifentry_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data);
uns32 ifentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                   NCSCONTEXT *data, uns32* next_inst_id,
                   uns32 *next_inst_id_len);
uns32 ifentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
uns32 ifentry_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer);
uns32 ifentry_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag);

uns32 ifsv_ifkey_from_instid(const uns32* i_inst_ids, uns32 i_inst_len
                                    , uns32 *ifkeyNet);


#endif
