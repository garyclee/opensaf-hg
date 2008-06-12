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


  MODULE NAME: rde_rde.h

$Header: $

..............................................................................

  DESCRIPTION: Contains definitions of a Unix-domain Sockets implementation
               of the RDE RDE interface
               which is used for communication between RDE and and RDE.

..............................................................................



******************************************************************************/


#ifndef RDE_RDE_H
#define RDE_RDE_H

/*#include "rde_cb.h"*/

/*****************************************************************************

          Constants Values

*****************************************************************************/

#define RDE_RDE_RC_SUCCESS 1
#define RDE_RDE_RC_FAILURE 0
#define RDE_RDE_MSG_WRT_SUCCESS 1
#define RDE_RDE_MSG_WRT_FAILURE 0
#define RDE_RDE_CONFIG_FILE "/etc/opt/opensaf/rde_rde_config_file"
#define RDE_RDE_CLIENT_ROLE_REQUEST 8 
#define RDE_RDE_CLIENT_ROLE_RESPONSE 9 
#define RDE_RDE_REBOOT_CMD 10 
#define RDE_RDE_CLIENT_SLOT_ID_EXCHANGE_REQ 11 
#define RDE_RDE_CLIENT_SLOT_ID_EXCHANGE_RESP 12

#define RDE_RDE_WAIT_CNT 4 
    

/***************************************************************************

         Structure Definitions

***************************************************************************/


typedef struct 
{
   struct sockaddr_in host_addr,serv_addr,cli_addr;
   int     fd;                    /* File descr32 rde_rde_write_msg (int fd, char *msg)  */
   int     clientfd;              
   int     flags;                 /* Flags specified for open */
   char    hostip[256];           /* Server IP Address */   
   int     hostportnum;           /* Port Number */
   char    servip[256];           /* Server IP Address */   
   int     servportnum;           /* Port Number */
   NCS_BOOL    connRecv;           /* If a the other rde client is connected to the current rde */
   NCS_BOOL    clientConnected;     /* flag whether this rde is connected to the other rde */
   int     clientReconnCount;   /* number of times the connect to the other rde was attempted */
   int     maxNoClientRetries;    /* maximum number of retries to connect to the other rde */
   int     recvFd;        /* The fd after the connection is accepted by the server */  
   int     count; /* Wait for ACT to send reboot cmd. Wait for 2 sec for 1 count as we don't have timer. */
   NCS_BOOL hb_loss; /* To store AVD if Heart Beat Loss has come to STDBY */
   NCS_BOOL retry; /* FALSE if there is only one blade. TRUE if there are both the blades and then 
                      connection goes down. */ 
   uns32 peer_slot_number;
   uns32 nid_ack_sent; /* To respond NID when it comes up. */
   NCS_BOOL conn_needed; /* If STDBY is waiting for Reboot command, there is not need to call connect. */
  
}RDE_RDE_CB;

/****************************************************************************
                                                                
         RDE RDE interface function prototypes                 
                                                                
\****************************************************************************/

uns32 rde_rde_open (RDE_RDE_CB *);
uns32 rde_rde_parse_config_file(void);
uns32 rde_rde_sock_open (RDE_RDE_CB *);
uns32 rde_rde_sock_init (RDE_RDE_CB *rde_rde_cb);
uns32 rde_rde_clCheckConnect(RDE_RDE_CB  * rde_rde_cb);
uns32 rde_count_testing (void);
uns32 rde_rde_connect(RDE_RDE_CB * );
uns32 rde_rde_process_msg (RDE_RDE_CB *rde_rde_cb);
uns32 rde_rde_close (RDE_RDE_CB *rde_rde_cb);
uns32 rde_rde_write_msg (int fd, char *msg);
uns32 rde_rde_read_msg (int fd, char *msg, int size);
uns32 rde_rde_client_process_msg(RDE_RDE_CB  * rde_rde_cb);
uns32 rde_rde_parse_msg(char * msg,RDE_RDE_CB  * rde_rde_cb);
uns32 rde_rde_process_send_role (void);
NCS_BOOL rde_rde_get_set_established(NCS_BOOL e);
uns32 rde_rde_client_socket_init(RDE_RDE_CB *rde_rde_cb);
uns32 rde_rde_client_read_role (void);
void rde_rde_strip(char* name);
uns32 rde_rde_update_config_file(PCS_RDA_ROLE role);
uns32 rde_rde_set_role (PCS_RDA_ROLE role);
uns32 rde_rde_hb_err (void);
uns32 rde_rde_sock_close (RDE_RDE_CB *rde_rde_cb);
/*uns32 rde_rde_process_hb_loss_stdby(RDE_CONTROL_BLOCK * rde_cb); */

#endif  /*  RDE_RDE_H  */

