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

  This file contains the prototypes for utility operations.
  
******************************************************************************
*/

#ifndef AVSV_UTIL_H
#define AVSV_UTIL_H

EXTERN_C uns32 avsv_cpy_SU_DN_from_DN(SaNameT *, SaNameT *);
EXTERN_C uns32 avsv_cpy_node_DN_from_DN(SaNameT *, SaNameT *);
EXTERN_C NCS_BOOL avsv_is_external_DN(SaNameT *);
EXTERN_C uns32 avsv_cpy_SI_DN_from_DN(SaNameT *, SaNameT *);

EXTERN_C uns32 avsv_dblist_uns32_cmp(uns8 *, uns8 *);
EXTERN_C uns32 avsv_dblist_uns64_cmp(uns8 *, uns8 *);
EXTERN_C uns32 avsv_dblist_saname_cmp(uns8 *, uns8 *);
EXTERN_C uns32 avsv_dblist_sahckey_cmp(uns8 *, uns8 *);

EXTERN_C NCS_BOOL avsv_sa_name_is_null(SaNameT *);

/* macro to determine if name is null */
#define m_AVSV_SA_NAME_IS_NULL(n) avsv_sa_name_is_null(&(n))

#endif /* !AVSV_UTIL_H */
