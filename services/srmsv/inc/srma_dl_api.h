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

  MODULE NAME: SRMA_DL_API.H

$Header: 
..............................................................................

  DESCRIPTION:  SRMSv agent specific DL API function prototype

******************************************************************************/

#ifndef SRMA_DL_API_H
#define SRMA_DL_API_H

EXTERN_C LEAPDLL_API uns32 srma_lib_req (NCS_LIB_REQ_INFO *);
EXTERN_C unsigned int ncs_srma_startup(void);
EXTERN_C unsigned int ncs_srma_shutdown(void);

#endif /* SRMA_DL_API_H */


