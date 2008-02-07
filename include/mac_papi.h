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

  _Public_ MIB Access Client (MAC) abstractions and function prototypes

*******************************************************************************/
/*
 * Module Inclusion Control...
 */

#ifndef MAC_PAPI_H
#define MAC_PAPI_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "ncsgl_defs.h"
#include "ncs_mib_pub.h"

#include "mab_penv.h"

/* Get general definitions.....*/

/***************************************************************************
 * The MAC API for initiating SNMP-like Requests per Virtual Router
 *
 * Conforms to NetPlane Common MIB Access API Prototype; 
 * See LEAP docs on this function prototype
 *
 *    See ncs_mib.h, func prototype (*NCSMIB_REQ_FNC)().. They are the same!!
 ***************************************************************************/

/* well known name (documented in Management Access User guide) to be used
 * with SPRR interface for MAA
 */
#define m_MAA_SP_ABST_NAME "NCS_MAA"

EXTERN_C MABMAC_API uns32 ncsmac_mib_request   ( NCSMIB_ARG* req );
EXTERN_C MABMAC_API uns32 gl_mac_handle;


#ifdef  __cplusplus
}
#endif

#endif /* MAC_PAPI_H */

