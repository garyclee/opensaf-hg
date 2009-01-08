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

 MODULE NAME:  SHIMMAB.H

$Header: /ncs/software/leap/base/products/cli/inc/shimmab.h 8     6/19/01 3:20p Agranos $
..............................................................................

  DESCRIPTION:

  Source file for external pubished API of SHIM Layer that mimic MAB
******************************************************************************
*/

#ifndef SHIMMAB_H
#define SHIMMAB_H

#if (NCS_MAB != 1)

/** LEAP Include Files.
 **/
#include "ncs_opt.h"
#include "gl_defs.h"
#include "t_suite.h"
#include "ncs_svd.h"
#include "ncs_stack.h"
#include "ncs_mib.h"

/** This is the SHIM Message Block...
 **/
typedef struct shim_msg
{
    struct shim_msg     *next;
    NCS_SERVICE_ID       event;          /* Indentifies the subsystem */
    NCSMIB_ARG           *mib_arg;       /* Mib structure for the subsystem */
}SHIM_MSG;

/*****************************************************************************

  PROCEDURE NAME: shim_mbx_handler


  DESCRIPTION:   Main entry Point

  PARAMETERS:
    mbx:    Pointer to SYSF_MBX for communication Subsystem .

  RETURNS:

    Nothing

  NOTES:
    This function conforms to the NCS_OS_CB function prototype.


*****************************************************************************/
EXTERN_C void shim_mbx_handler(void *mbx);

/*****************************************************************************

  Function NAME: ncsshim_create


  DESCRIPTION:    Creates the shim layer that is used when MAB is not present


  PARAMETERS:
    demo_mbx:   Pointer to SYSF_MBX for communication with Demo task.

  RETURNS:
    NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES:
    This function conforms to the NCS_OS_CB function prototype.


*****************************************************************************/
EXTERN_C uns32 ncsshim_create(void);

/****************************************************************************
  Function NAME:   ncsshim_mib_request

  DESCRIPTION:

  Hign level entry point into SHIM Layer for MIB requests if MAB not in use

  ARGUMENTS:

        mib_arg:  Arguments for request

  RETURNS:
        NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE


  NOTES:
*****************************************************************************/
EXTERN_C uns32 ncsshim_mib_request(struct ncsmib_arg *mib_args);

/*****************************************************************************

  Function NAME: ncsshim_destroy


  DESCRIPTION:    Destroy the shim layer that is used when MAB is not present


  PARAMETERS:
    demo_mbx:   Pointer to SYSF_MBX for communication with Demo task.

  RETURNS:
    NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES:
    This function conforms to the NCS_OS_CB function prototype.


*****************************************************************************/
EXTERN_C uns32 ncsshim_destroy(void);

#endif
#endif

