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

  This file contains data structures related to EDSV MAB interface and 
  the definitions of MIB tables.
  
******************************************************************************
*/

#ifndef EDSV_MIB_H
#define EDSV_MIB_H

/*----------External Function Declaration------------------------- */

EXTERN_C uns32 edsv_mab_register(EDS_CB *cb);

EXTERN_C uns32 edsv_mab_unregister(EDS_CB *cb);

EXTERN_C uns32 edsv_mab_request(NCSMIB_ARG *args);

EXTERN_C uns32 edsv_table_register(void);

EXTERN_C uns32 safevtscalarobject_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

EXTERN_C uns32 safevtscalarobject_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);

EXTERN_C
    uns32 safevtscalarobject_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				  NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);

EXTERN_C
    uns32 safevtscalarobject_extract(NCSMIB_PARAM_VAL *param,
				     NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);

EXTERN_C
    uns32 safevtscalarobject_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				    NCSMIB_SETROW_PARAM_VAL *params, struct ncsmib_obj_info *obj_info, NCS_BOOL flag);

EXTERN_C uns32 saevtchannelentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);

EXTERN_C EDS_WORKLIST *get_channel_from_worklist(EDS_CB *cb, SaNameT chan_name);

EXTERN_C uns32 saevtchannelentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

EXTERN_C
    uns32 saevtchannelentry_extract(NCSMIB_PARAM_VAL *param,
				    NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);

EXTERN_C
    uns32 saevtchannelentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				 NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);

EXTERN_C
    uns32 saevtchannelentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				   NCSMIB_SETROW_PARAM_VAL *params,
				   struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

EXTERN_C uns32 saevtchannelentry_tbl_reg(void);

EXTERN_C uns32 safevtscalarobject_tbl_reg(void);

#endif   /* EDSV_MIB_H */
