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
 */

/*****************************************************************************
  DESCRIPTION:

  This file contains extern declarations for AvSv toolkit application.
  
******************************************************************************
*/

#ifndef EDSV_DEMO_APP_H
#define EDSV_DEMO_APP_H

/* Common header files */
#include<stdio.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/* SAF header files */
#include <saAis.h>
#include <saEvt.h>

#define NCS_SERVICE_ID_EDSVTM  (UD_SERVICE_ID_END + 2)

typedef enum
{
   NCS_SERVICE_EDSVTM_SUB_ID_SNMPTM_EVT_DATA = 1,
   NCS_SERVICE_EDSVTM_SUB_ID_EVT_PAT_ARRAY,
   NCS_SERVICE_EDSVTM_SUB_ID_EVT_PATTERNS,
   NCS_SERVICE_SNMPTM_SUB_ID_MAX
} NCS_SERVICE_EDSVTM_SUB_ID;


/* Top level routine to run EDSv demo */
extern unsigned int ncs_edsv_run(void);

#endif /* !EDSV_DEMO_APP_H */

