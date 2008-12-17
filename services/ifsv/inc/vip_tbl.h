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

#include "vip_db.h"
#include "ifa_papi.h"


#define IFSV_CB_NULL ((IFSV_CB*)0) 
#define IFSV_IFD_VIPD_RECORD_NULL ((IFSV_IFD_VIPD_RECORD *)0)
#define NCS_VIP_MIB_ATTRIB_NULL ((NCS_VIP_MIB_ATTRIB *)0)
#define NCS_IFSV_VIP_IP_LIST_NULL ((NCS_IFSV_VIP_IP_LIST *)0)
#define NCS_IFSV_VIP_INTF_LIST_NULL ((NCS_IFSV_VIP_INTF_LIST *)0)

/* Not implemented setrow && set functionality in the current phase*/

EXTERN_C uns32 ncsvipentry_setrow(NCSCONTEXT cb,
                                  NCSMIB_ARG *args,
                                  NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                                  struct ncsmib_obj_info *obj_info,
                                  NCS_BOOL testrow_flag);
EXTERN_C uns32 ncsvipentry_set(NCSCONTEXT cb,
                               NCSMIB_ARG *arg,
                               NCSMIB_VAR_INFO *var_info,
                               NCS_BOOL test_flag);

EXTERN_C uns32 ncsvipentry_get(NCSCONTEXT cb,
                                        NCSMIB_ARG *arg,
                                        NCSCONTEXT *data);

EXTERN_C uns32 ncsvipentry_extract(NCSMIB_PARAM_VAL *param,
                                            NCSMIB_VAR_INFO  *var_info,
                                            NCSCONTEXT data,
                                            NCSCONTEXT buffer);


EXTERN_C uns32 ncsvipentry_next(NCSCONTEXT cb,
                                         NCSMIB_ARG *arg,
                                         NCSCONTEXT *data,
                                         uns32 *next_inst_id,
                                         uns32 *next_inst_id_len);



