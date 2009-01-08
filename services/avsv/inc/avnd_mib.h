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
  
  AvND-MAB and AvND-MIBLIB interaction related definitions.
    
******************************************************************************
*/

#ifndef AVND_MIB_H
#define AVND_MIB_H

/* MAB & MIB related routines */
EXTERN_C uns32 avnd_mab_reg_tbl_rows(AVND_CB *cb, NCSMIB_TBL_ID tbl_id,
                                     SaNameT  *, SaNameT *, SaClmNodeIdT *,
                                     uns32 *row_hdl, uns32);
EXTERN_C uns32 avnd_mab_unreg_rows(AVND_CB *cb);
EXTERN_C uns32 avnd_mab_unreg_tbl_rows(AVND_CB *cb, NCSMIB_TBL_ID tbl_id, uns32 row_hdl,
                                       uns32);
EXTERN_C uns32 avnd_miblib_init (AVND_CB *cb);
EXTERN_C uns32 avnd_req_mib_func(struct ncsmib_arg *args);
EXTERN_C uns32 avnd_tbls_reg_with_mab(AVND_CB *cb);
EXTERN_C uns32 avnd_tbls_unreg_with_mab(AVND_CB *cb);
EXTERN_C uns32 avnd_tbls_reg_with_mab_for_vdest(AVND_CB *cb);
EXTERN_C uns32 avnd_tbls_unreg_with_mab_for_vdest(AVND_CB *cb);

/* AVND Table registration routines with MIBLIB */
EXTERN_C uns32 ncsswtableentry_tbl_reg(void);
EXTERN_C uns32 ncssndtableentry_tbl_reg(void);
EXTERN_C uns32 ncsssutableentry_tbl_reg(void);
EXTERN_C uns32 ncsscomptableentry_tbl_reg(void);
EXTERN_C uns32 saamfscompcsitableentry_tbl_reg(void);
                     
#endif /* !AVND_MIB_H */

