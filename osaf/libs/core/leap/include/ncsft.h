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

  MODULE NAME: NCSFT.H  (Harris & Jeffries Fault Tolerance.h)

  REVISION HISTORY:

  Date     Version  Name          Description
  -------- -------  ------------  --------------------------------------------
  02-27-98 1.00A    H&J (PB)      Original

..............................................................................

  DESCRIPTION:

  This module contains declarations and structures related to Fault Tolerance
  that are generic to all subsystems.

*******************************************************************************/

/*
 * Module Inclusion Control...
 */
#ifndef NCSFT_H
#define NCSFT_H

typedef uns32
 (*TRIGGER_CALLBACK) (uint16_t svc_id, uns32 signal, void *entity_handle, void *api_ctxt);

/*
 ***********************
 * NCSFT ROLES
 ***********************
 */
#define NCSFT_ROLE_INIT                  0
#define NCSFT_ROLE_PRIMARY               1
#define NCSFT_ROLE_BACKUP                2
#define NCSFT_ROLE_WARM_BOOT_BACKUP    3
#define NCSFT_ROLE_STANDALONE_ACTIVE     4

#define NCSFT_ROLE_UNDEFINED             99

/*
 ***********************
 * NCSFT BACKUP POLICIES
 ***********************
 */

typedef enum ncsft_backup_policy {
	NCSFT_DO_NOTHING = 1,
	NCSFT_PENT_UP,
	NCSFT_PIGGY_BACK,
	NCSFT_SIDE_EFFECT
} NCS_FT_BACKUP_POLICY;

#endif
