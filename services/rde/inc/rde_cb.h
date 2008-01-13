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


  MODULE NAME: rde_cb.h

$Header: $

..............................................................................

  DESCRIPTION: Contains definitions of the RDE control block sturctures

..............................................................................



******************************************************************************
*/

#ifndef RDE_CB_H
#define RDE_CB_H

#include "rde_rda_common.h"
#include "rde_amf.h"
#include "rde_rda.h"
#include "rda_papi.h"
#include "rde_rde.h"


/*****************************************************************************\
 *                                                                             *
 *   Structure Definitions                                                     *
 *                                                                             *
\*****************************************************************************/

/*
** Forward declarations
**
*/

#define   RDE_DEFAULT_PID_FILE                  "/var/run/rde.pid"
#define   RDE_DEFAULT_LOG_LEVEL             5
#define   RDE_DEFAULT_SHELF_NUMBER          2
#define   RDE_DEFAULT_SLOT_NUMBER           1
#define   RDE_DEFAULT_SITE_NUMBER           1

/*
**  RDE_OPTIONS
**
**  Structure containing all the options that may
**  be specified at startup time
**
*/

typedef struct
{
   const char        *pid_file        ;
   uns32              shelf_number    ;
   uns32              slot_number     ;
   uns32              site_number     ;
   uns32              log_level       ;
   NCS_BOOL           is_daemon       ;

} RDE_OPTIONS;



/*
**  RDE_CONTROL_BLOCK
**
**  Structure containing all state information for RDE
**
*/

typedef struct
{

   RDE_OPTIONS    options;
   const char    *prog_name;
   NCSCONTEXT     task_handle;
   NCS_BOOL       task_terminate;
   NCS_BOOL    fabric_interface;
   NCS_OS_SEM     semaphore;
   uns32          select_timeout;

   PCS_RDA_ROLE   ha_role;

   RDE_RDA_CB     rde_rda_cb;
   RDE_AMF_CB     rde_amf_cb;
   NCS_LOCK      lock;
   RDE_RDE_CB    rde_rde_cb;

} RDE_CONTROL_BLOCK;


/*****************************************************************************\
 *                                                                             *
 *                       Function Prototypes                                   *
 *                                                                             *
\*****************************************************************************/

EXTERN_C     RDE_CONTROL_BLOCK * rde_get_control_block (void);
EXTERN_C     uns32               rde_get_options       (RDE_CONTROL_BLOCK * context,
                                                        int                 argc,
                                                        char              * argv[]) ;

#endif      /* RDE_CB_H */

