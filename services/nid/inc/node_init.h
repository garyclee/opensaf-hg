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

#include <config.h>

/*****************************************************************************
..............................................................................

  $Header:

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
#define NID_MAXSNAME                 15
#define NID_MAXAPPTYPE_LEN           1
#define NID_MAXPARMS                 50
#define NID_MAXARGS                  30
#define NID_MAX_TIMEOUT_LEN          7
#define NID_MAX_PRIO_LEN             4
#define NID_MAX_RESP_LEN             1
#define NID_MAX_REST_LEN             1



#define NID_PLAT_CONF_PATH          SYSCONFDIR
#define NID_PLAT_CONF               "nodeinit.conf"
#define NID_NCSLOGPATH              LOCALSTATEDIR "log/nid/"
#define NID_PID_FILE                PIDPATH "nodeinit.pid"
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
}NID_PLATCONF_PARS;


/******************************************************************
*       File used to store the reset count before going for reset *
******************************************************************/

struct nid_resetinfo{
   NID_SVC_CODE faild_serv_code;
   uns32 count;
};

/******************************************************************
*       List of valied file types, used while recovery            *
******************************************************************/
typedef enum nid_app_type{
   NID_APPERR = -1,
   NID_EXEC=0,
   NID_SCRIPT,
   NID_DAEMN,
   NID_MAXEXECTYP
}NID_APP_TYPE;


/******************************************************************
*       List of valied recovery options                           *
******************************************************************/
typedef enum nid_recovery_opt {
   NID_RESPAWN=0,
   NID_RESET,
   NID_MAXREC
}NID_RECOVERY_OPT;


typedef struct nid_spawn_info NID_SPAWN_INFO;

typedef uns32 (*NID_FUNC)(NID_SPAWN_INFO *,uns8 *);
typedef int32 (*NID_FORK_FUNC)(NID_SPAWN_INFO *, uns8 *, char * args[],uns8 *, uns8 *); /*DEL*/



/*****************************************************************
 *       Structures Used by nodeinitd to store the parsed info   *
 *       from SYSCONFDIR/nodeinit.conf, and used while    *
 *       spawning                                                *
 *****************************************************************/
typedef struct nid_recovery_list {
   uns32 retry_count;   /* Retry count against each action*/
   NID_FUNC action;         /* Recovery action to be taken */
} NID_RECOVERY_LIST;


struct nid_spawn_info {
   uns32 pid;
   NID_SVC_CODE servcode;
   NID_APP_TYPE app_type;
   uns8 s_name[NID_MAXSFILE];                        /* Service name to be spawned*/
   uns8 cleanup_file[NID_MAXSFILE];                  /* Cleanup for the service spawned*/
   uns32 time_out;                                   /* Timeout for spawned service*/
   int32 priority;                                   /* Process priority&*/
   NID_RECOVERY_LIST recovery_matrix[NID_MAXREC];    /* recovery action list */
   uns8 s_parameters[NID_MAXPARMS];                  /* Parameters for spawned service*/
   char * serv_args[NID_MAXARGS];                    /* pointers to '\0' seperated arguments in s_parameters */
   uns8 cleanup_parms[NID_MAXPARMS];                 /* Parameters for cleaning appl*/
   char * clnup_args[NID_MAXARGS];                   /* pointers to \0 sperated arguments in cleanup_parms */
   struct nid_spawn_info *next;
};


typedef struct nid_child_list {
   NID_SPAWN_INFO *head;                 /* Head of the spawn list*/
   NID_SPAWN_INFO *tail;                 /* Tail of the spawn list*/
   uns32 count;                          /* Total nodes in the list*/
} NID_CHILD_LIST;



/*********************************************************************
*       Structures and definitions related to firmware progress      *
*       sensor. The definitions below are very much dependent on     *
*       the sequence and number of NID_SVC_CODES Defined in nid_api.h*
*********************************************************************/

#define NID_NOTIFY_ENABLE   1
#define NID_NOTIFY_DISABLE  0

#define NID_PHOENIXBIOS_UPGRADED 100

/* vivek_nid */
#define MOTO_FIRMWARE_BOOT_SUCCESS      0x80
#define MOTO_FIRMWARE_SYSINIT_SUCCESS   0x81
#define MOTO_FIRMWARE_PHOENIXBIOS_UPGRADE_SUCCESS 0x82

/*********************************************************
*       OpenSAF Specific System Firmware Progress Codes *
*       May need to go into hisv headder.                *
*********************************************************/
#define  MOTO_FIRMWARE_OEM_CODE                 0xFD /* motorola OEM code */

#define  MOTO_FIRMWARE_HAPS_INIT_SUCCESS        0x01
#define  MOTO_FIRMWARE_NCS_INIT_SUCCESS         0x02

#if 0
#define  MOTO_FIRMWARE_HPM_INIT_FAIL                   0x80
/* vivek_nid */
#define  MOTO_FIRMWARE_IPMC_UPGRADE_FAIL               0x81
#define  MOTO_FIRMWARE_PHOENIXBIOS_UPGRADE_FAIL        0x82
#define  MOTO_FIRMWARE_HLFM_INIT_FAIL                  0x83
#define  MOTO_FIRMWARE_OPENHPI_INIT_FAIL               0x84
#define  MOTO_FIRMWARE_XND_INIT_FAIL                   0x85
#define  MOTO_FIRMWARE_LHCD_INIT_FAIL                  0x86
#define  MOTO_FIRMWARE_LHCR_INIT_FAIL                  0x87
#define  MOTO_FIRMWARE_NWNG_INIT_FAIL                  0x88
#define  MOTO_FIRMWARE_DRBD_INIT_FAIL                  0x89
#define  MOTO_FIRMWARE_TIPC_INIT_FAIL                  0x8A
#define  MOTO_FIRMWARE_IFSVDD_INIT_FAIL                0x8B
#define  MOTO_FIRMWARE_DLSV_INIT_FAIL                  0x8C
#define  MOTO_FIRMWARE_MASV_INIT_FAIL                  0x8D
#define  MOTO_FIRMWARE_PSSV_INIT_FAIL                  0x8E
#define  MOTO_FIRMWARE_GLND_INIT_FAIL                  0x8F
#define  MOTO_FIRMWARE_EDSV_INIT_FAIL                  0x90
#define  MOTO_FIRMWARE_SUBAGT_INIT_FAIL                0x91 /*61409*/
#define  MOTO_FIRMWARE_SNMPD_INIT_FAIL                 0x92
#define  MOTO_FIRMWARE_NCS_INIT_FAIL                   0x93
#endif

/* Moving the IPMC/BIOS codes to the end */
#define  MOTO_FIRMWARE_HPM_INIT_FAIL                   0x80
#define  MOTO_FIRMWARE_HLFM_INIT_FAIL                  0x81
#define  MOTO_FIRMWARE_OPENHPI_INIT_FAIL               0x82
#define  MOTO_FIRMWARE_XND_INIT_FAIL                   0x83
#define  MOTO_FIRMWARE_LHCD_INIT_FAIL                  0x84
#define  MOTO_FIRMWARE_LHCR_INIT_FAIL                  0x85
#define  MOTO_FIRMWARE_NWNG_INIT_FAIL                  0x86
#define  MOTO_FIRMWARE_DRBD_INIT_FAIL                  0x87
#define  MOTO_FIRMWARE_TIPC_INIT_FAIL                  0x88
#define  MOTO_FIRMWARE_IFSVDD_INIT_FAIL                0x89
#define  MOTO_FIRMWARE_DLSV_INIT_FAIL                  0x8A
#define  MOTO_FIRMWARE_MASV_INIT_FAIL                  0x8B
#define  MOTO_FIRMWARE_PSSV_INIT_FAIL                  0x8C
#define  MOTO_FIRMWARE_GLND_INIT_FAIL                  0x8D
#define  MOTO_FIRMWARE_EDSV_INIT_FAIL                  0x8E
#define  MOTO_FIRMWARE_SUBAGT_INIT_FAIL                0x8F /*61409*/
#define  MOTO_FIRMWARE_SNMPD_INIT_FAIL                 0x90
#define  MOTO_FIRMWARE_NCS_INIT_FAIL                   0x91
#define  MOTO_FIRMWARE_IPMC_UPGRADE_FAIL               0x92
#define  MOTO_FIRMWARE_PHOENIXBIOS_UPGRADE_FAIL        0x93

#define  MOTO_FIRMWARE_RDF_INIT_FAIL                   0x94


enum fw_prog_notify{
   NID_NOTIFY_SUCCESS=0,
   NID_NOTIFY_FAILURE,
   NID_MAX_NOTIFY
};


typedef struct {
   NCS_BOOL notify;
   uns8 data3;
   uns8 *diplay_msg;
} FW_PROG_SNSR;

#endif /* NODEINIT_H */
