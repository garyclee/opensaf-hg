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

  MODULE NAME: rde_rda.h

..............................................................................

  DESCRIPTION: Contains definitions of a Unix-domain Sockets implementation
               of the RDE RDA interface
               which is used for communication between RDE and and RDA.

..............................................................................

******************************************************************************
*/

#ifndef RDE_RDA_H
#define RDE_RDA_H

#include "rde_cb.h"
#include <sys/socket.h>
#include <sys/un.h>

/*****************************************************************************\
 *                                                                             *
 *   Constants and Enumerated Values                                           *
 *                                                                             *
\*****************************************************************************/

/* 
** Limits 
**
**
*/

typedef enum {
	RDE_RDA_SOCK_BUFFER_SIZE = 3200
} RDE_RDA_SOCK_LIMITS;

/*****************************************************************************\
 *                                                                             *
 *   Structure Definitions                                                     *
 *                                                                             *
\*****************************************************************************/

#define REPLY_SIZE               4096
#define MAX_RDA_CLIENTS 64

/*
 * Forward declarations
 */

typedef struct {

	int fd;
	bool is_async;

} RDE_RDA_CLIENT;

/*
 *  Socket Management related information
 */

typedef struct sockaddr_un rde_rda_sock_addr;

typedef struct {
	struct sockaddr_un sock_address;
	int fd;			/* File descriptor          */
	int flags;		/* Flags specified for open */
	int client_count;
	RDE_RDA_CLIENT clients[MAX_RDA_CLIENTS];

} RDE_RDA_CB;

/***************************************************************\
 *                                                               *
 *         RDE RDA interface function prototypes                 *
 *                                                               *
\***************************************************************/

const char *rde_rda_sock_name(RDE_RDA_CB *rde_rda_cb);
uint32_t rde_rda_open(const char *sockname, RDE_RDA_CB *rde_rda_cb);
uint32_t rde_rda_close(RDE_RDA_CB *rde_rda_cb);
uint32_t rde_rda_process_msg(RDE_RDA_CB *rde_rda_cb);
uint32_t rde_rda_client_process_msg(RDE_RDA_CB *rde_rda_cb, int index, int *disconnect);
uint32_t rde_rda_send_role(int role);

#endif   /* RDE_RDA_H */
