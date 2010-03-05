/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2009 The OpenSAF Foundation
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
 * Author(s): Hewlett-Packard Development Company, L.P.
 *
 */

/*
 * PLMc header file
 *
 * This header file is used by the plmc_boot, plmc_d, and plmc_lib
 * implementations.  Users of the PLMc library should not include
 * this header file.  Users of the PLmc library should instead 
 * include the plmc_lib.h header file.
 *
 */

#ifndef PLMC_H
#define PLMC_H

/* Success and Failure return codes. */
#define PLMC_EXIT_SUCCESS	0
#define PLMC_EXIT_FAILURE	1

/* Tag value and message data lengths. */
#define PLMC_MAX_TAG_LEN	256
#define PLMC_MAX_PID_LEN	128
#define PLMC_MAX_SERVICES 	128
#define PLMC_MAX_UDP_DATA_LEN   1024
#define PLMC_MAX_TCP_DATA_LEN   1024

/* Default timeout values for PLMc daemon operations. */
#define PLMC_TCP_TIMEOUT_SECS			5
#define PLMC_CHILD_THREAD_WATCHDOG_PERIOD_SECS	2

/* Command return values. */
#define PLMC_ACK_MSG_RECD	"PLMC_ACK_MSG_RECD"
#define PLMC_ACK_SUCCESS	"PLMC_ACK_SUCCESS"
#define PLMC_ACK_FAILURE	"PLMC_ACK_FAILURE"
#define PLMC_ACK_TIMEOUT	"PLMC_ACK_TIMEOUT"

/* Default location of the plmc.conf configuration file. */
#define PLMC_CONFIG_FILE_LOC	"/etc/opensaf/plmc.conf"

/* Default location of the plmc daemon pid file. */
#define PLMC_D_PID_FILE_LOC	"/var/run/plmc_d.pid"

/* UDP message strings. */
#define PLMC_BOOT_START_MSG	"EE_INSTANTIATING"
#define PLMC_BOOT_STOP_MSG	"EE_TERMINATED"
#define PLMC_D_START_MSG	"EE_INSTANTIATED"
#define PLMC_D_STOP_MSG		"EE_TERMINATING"

/* These are the start/stop actions that the PLMc daemon has implemented. */
typedef enum
{
    PLMC_START = 0,
    PLMC_STOP = 1,
    PLMC_RESTART = 2
} PLMC_action;

/* These are the numerical values of the tags found in the plmc.conf */
/* configuration file.                                               */
typedef enum
{
    PLMC_EE_ID = 1,
    PLMC_MSG_PROTOCOL_VERSION = 2,
    PLMC_CONTROLLER_1_IP = 3,
    PLMC_CONTROLLER_2_IP = 4,
    PLMC_SERVICES_TO_START = 5,
    PLMC_SERVICES_TO_STOP = 6,
    PLMC_OSAF_TO_START = 7,
    PLMC_OSAF_TO_STOP = 8,
    PLMC_TCP_PLMS_LISTENING_PORT = 9,
    PLMC_UDP_BROADCAST_PORT = 10,
    PLMC_OS_TYPE = 11,
    PLMC_CMD_TIMEOUT_SECS = 12,
    PLMC_OS_REBOOT_CMD = 13,
    PLMC_OS_SHUTDOWN_CMD = 14
} PLMC_config_tags;

/* This struct holds the contents of the plmc.conf configuration file. */
typedef struct {
    char ee_id[PLMC_MAX_TAG_LEN];
    char msg_protocol_version[PLMC_MAX_TAG_LEN];
    char controller_1_ip[PLMC_MAX_TAG_LEN];
    char controller_2_ip[PLMC_MAX_TAG_LEN];
    char services_to_start[PLMC_MAX_TAG_LEN][PLMC_MAX_SERVICES];
    int  num_services_to_start;
    char services_to_stop[PLMC_MAX_TAG_LEN][PLMC_MAX_SERVICES];
    int  num_services_to_stop;
    char osaf_to_start[PLMC_MAX_TAG_LEN];
    char osaf_to_stop[PLMC_MAX_TAG_LEN];
    char tcp_plms_listening_port[PLMC_MAX_TAG_LEN];
    char udp_broadcast_port[PLMC_MAX_TAG_LEN];
    char os_type[PLMC_MAX_TAG_LEN];
    int  plmc_cmd_timeout_secs;
    char os_reboot_cmd[PLMC_MAX_TAG_LEN];
    char os_shutdown_cmd[PLMC_MAX_TAG_LEN];
} PLMC_config_data;

/* The PLMC daemon command numerical index. */
typedef enum
{
  PLMC_NOOP_CMD = 0,
  PLMC_GET_ID_CMD = 1,
  PLMC_GET_PROTOCOL_VER_CMD = 2,
  PLMC_GET_OSINFO_CMD = 3,
  PLMC_SA_PLM_ADMIN_UNLOCK_CMD = 4,
  PLMC_SA_PLM_ADMIN_LOCK_INSTANTIATION_CMD = 5,
  PLMC_SA_PLM_ADMIN_LOCK_CMD = 6,
  PLMC_SA_PLM_ADMIN_RESTART_CMD = 7,
  PLMC_OSAF_START_CMD = 8,
  PLMC_OSAF_STOP_CMD = 9,
  PLMC_OSAF_SERVICES_START_CMD = 10,
  PLMC_OSAF_SERVICES_STOP_CMD = 11,
  PLMC_PLMCD_RESTART_CMD = 12,
  PLMC_LAST_CMD = 12
} PLMC_cmd_idx;

#endif  /* PLMC_H */
