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

#include <configmake.h>

/*****************************************************************************
..............................................................................

  ..............................................................................

  DESCRIPTION:
  This file contains the definitaions and declarations local to nodeinitd.
  This file is intended to be used only by nodeinitd.
  
******************************************************************************
*/

#ifndef NODEINIT_H
#define NODEINIT_H

#include "nid_api.h"
#include "ncs_log.h"
#include "saAmf.h"

/****************************************************************
*      Log levels for nodeinitd                                 *
****************************************************************/
#define NID_LOG2CONS        3
#define NID_LOG2FILE        2
#define NID_LOG2FILE_CONS   1

/****************************************************************
*      MAX limits used for fields in nodeinit.conf entries      *
****************************************************************/
#define NID_MAXSFILE                 100
#define NID_MAX_SVC_NAME_LEN         15
#define NID_MAXAPPTYPE_LEN           1
#define NID_MAXPARMS                 50
#define NID_MAXARGS                  30
#define NID_MAX_TIMEOUT_LEN          7
#define NID_MAX_PRIO_LEN             4
#define NID_MAX_RESP_LEN             1
#define NID_MAX_REST_LEN             1

#define NID_PLAT_CONF_PATH          PKGSYSCONFDIR
#define NID_PLAT_CONF               "nodeinit.conf"
#define NID_NCSLOGPATH              PKGLOCALSTATEDIR "/stdouts"
#define NID_PID_FILE                PKGPIDDIR "/nodeinit.pid"
#define NID_RUNNING_DIR             "/"
#define NID_CNSL                    "/dev/console"
#define NID_VT_MASTER               "/dev/tty0"

/*******************************************************************
*       States Used While Parsing nodeinit.conf                    *
*******************************************************************/
typedef enum nid_platconf_pars {
	NID_PLATCONF_SFILE,
	NID_PLATCONF_SNAME,
	NID_PLATCONF_CLNUP,
	NID_PLATCONF_APPTYP,
	NID_PLATCONF_TOUT,
	NID_PLATCONF_PRIO,
	NID_PLATCONF_RSP,
	NID_PLATCONF_RST,
	NID_PLATCONF_SPARM,
	NID_PLATCONF_CLNPARM,
	NID_PLATCONF_END
} NID_PLATCONF_PARS;

/******************************************************************
*       File used to store the reset count before going for reset *
******************************************************************/

struct nid_resetinfo {
	char faild_serv_name[NID_MAXSNAME];
	uns32 count;
};

/******************************************************************
*       List of valied file types, used while recovery            *
******************************************************************/
typedef enum nid_app_type {
	NID_APPERR = -1,
	NID_EXEC = 0,
	NID_SCRIPT,
	NID_DAEMN,
	NID_MAXEXECTYP
} NID_APP_TYPE;

/******************************************************************
*       List of valied recovery options                           *
******************************************************************/
typedef enum nid_recovery_opt {
	NID_RESPAWN = 0,
	NID_RESET,
	NID_MAXREC
} NID_RECOVERY_OPT;

typedef struct nid_spawn_info NID_SPAWN_INFO;

typedef uns32 (*NID_FUNC) (NID_SPAWN_INFO *, char *);
typedef int32 (*NID_FORK_FUNC) (NID_SPAWN_INFO *, char *, char *args[], char *, char *);

/*****************************************************************
 *       Structures Used by nodeinitd to store the parsed info   *
 *       from PKGSYSCONFDIR/nodeinit.conf, and used while    *
 *       spawning                                                *
 *****************************************************************/
typedef struct nid_recovery_list {
	uns32 retry_count;	/* Retry count against each action */
	NID_FUNC action;	/* Recovery action to be taken */
} NID_RECOVERY_LIST;

struct nid_spawn_info {
	uns32 pid;
	char serv_name[NID_MAXSNAME];	/* Service name to be spawned */
	NID_APP_TYPE app_type;
	char s_name[NID_MAXSFILE];	/* Service name to be spawned */
	char cleanup_file[NID_MAXSFILE];	/* Cleanup for the service spawned */
	uns32 time_out;		/* Timeout for spawned service */
	int32 priority;		/* Process priority& */
	NID_RECOVERY_LIST recovery_matrix[NID_MAXREC];	/* recovery action list */
	char s_parameters[NID_MAXPARMS];	/* Parameters for spawned service */
	char *serv_args[NID_MAXARGS];	/* pointers to '\0' seperated arguments in s_parameters */
	char cleanup_parms[NID_MAXPARMS];	/* Parameters for cleaning appl */
	char *clnup_args[NID_MAXARGS];	/* pointers to \0 sperated arguments in cleanup_parms */
	struct nid_spawn_info *next;
};

typedef struct nid_child_list {
	NID_SPAWN_INFO *head;	/* Head of the spawn list */
	NID_SPAWN_INFO *tail;	/* Tail of the spawn list */
	uns32 count;		/* Total nodes in the list */
} NID_CHILD_LIST;

/*********************************************************************
*       Structures and definitions related to firmware progress      *
*       sensor. The definitions below are very much dependent on     *
*       the sequence and number of NID_SVC_CODES Defined in nid_api.h*
*********************************************************************/

#define NID_NOTIFY_ENABLE   1
#define NID_NOTIFY_DISABLE  0

#endif   /* NODEINIT_H */
