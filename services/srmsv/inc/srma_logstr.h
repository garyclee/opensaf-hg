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

  This module contains the extern declaration for registering SRMSv Agent's
  canned strings with Dtsv server.
  
******************************************************************************
*/

#ifndef SRMA_LOGSTR_H
#define SRMA_LOGSTR_H

#if (NCS_DTS == 1)

EXTERN_C uns32 srma_logstr_reg(void);
EXTERN_C uns32 srma_logstr_unreg(void);

#endif /* (NCS_DTS == 1) */

#endif /* !SRMA_LOGSTR_H */


