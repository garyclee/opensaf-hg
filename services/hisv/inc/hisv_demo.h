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

  This file contains macros and function declarations for HISV demo

*****************************************************************************/

#ifndef HISV_DEMO_H
#define HISV_DEMO_H


/* HISV Demo Usage commands */
typedef enum
{
   HISV_DISPLAY_MENU,
   HISV_HCD_CONTROL,
   HISV_HPL_CONTROL,
   HISV_DEMO_EXIT

} HISV_COMMANDS;

/* HCD commands */
typedef enum
{
   HCD_DISPLAY_MENU,
   HCD_INITIALIZE,
   HCD_FINALIZE,
   HCD_FINALIZE_RET,
   HCD_RET_MAIN_MENU

} HISV_HCD_COMMANDS;

/* HPL commands */
typedef enum
{
   HPL_DISPLAY_MENU,
   HPL_INITIALIZE,
   HPL_RESOURCE_RESET,
   HPL_RESOURCE_POWER_SET,
   HPL_SEL_CLEAR,
   HPL_CONFIG_HOTSWAP,
   HPL_CONFIG_HS_STATE_GET,
   HPL_CONFIG_HS_INDICATOR,
   HPL_CONFIG_HS_AUTOEXTRACT,
   HPL_MANAGE_HOTSWAP,
   HPL_ALARM_ADD,
   HPL_ALARM_GET,
   HPL_ALARM_DELETE,
   HPL_EVENT_LOT_TIME,
   HPL_FINALIZE,
   HPL_FINALIZE_RET,
   HPL_RET_MAIN_MENU

} HISV_HPL_COMMANDS;

/* maximum length of entity path allowed by demo */
#define  MAX_ENTITY_PATH_LEN   400

/* extern function declarations */
EXTERN_C uns32 hcd_main(int chassis_id);

#endif /* !HISV_DEMO_H */
