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

  MODULE NAME: SRMSV_DEFS.H

..............................................................................

  DESCRIPTION: This file comtains the MACRO definitions and enum definitions
               defined for SRMSv service, used by both SRMA and SRMND.

******************************************************************************/

#ifndef SRMSV_DEFS_H
#define SRMSV_DEFS_H

#define NCS_SRMSV_ERR  SaAisErrorT
#define SRMSV_DEF_MONITORING_RATE  5
#define SRMSV_MAX_PID_DESCENDANTS  100

#define m_SRMSV_ASSERT(cond)  if(!(cond))  assert(0)

typedef enum
{
    SRMSV_ND_OPER_STATUS_UP = 1,
    SRMSV_ND_OPER_STATUS_DOWN,
    SRMSV_ND_OPER_STATUS_END
} NCS_SRMSV_ND_OPER_STATUS;

#endif /* SRMSV_DEFS_H */



