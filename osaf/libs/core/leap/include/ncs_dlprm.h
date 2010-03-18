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

  MODULE NAME:  NCS_DLPRM.H

..............................................................................

  DESCRIPTION:

  CONTENTS:

******************************************************************************
*/

#ifndef NCS_DLPRM_H
#define NCS_DLPRM_H

#define SYSF_DL_MAX_LAYER_HANDLE_LEN   (sizeof(void *))
#define SYSF_DL_MAX_CLIENT_HANDLE_LEN  (sizeof(void *))

#include "sckt_defs.h"
#include "ncs_osprm.h"

#include "ncsgl_defs.h"
#include "ncssysf_lck.h"
#include "ncsusrbuf.h"
#include "ncs_dl.h"
#include "ncs_queue.h"
#include "ncssysf_def.h"
#include "ncspatricia.h"

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 **                                                                        **
 **                                                                        **
 **                       General Definitions                              **
 **                                                                        **
 **                                                                        **
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ***************************************************************************/

	typedef struct sysf_dl_dispatch_info_tag {

		union {
			struct {
				NCS_L2_ADDR i_local_addr;
				NCS_L2_ADDR i_remote_addr;
				uns32 i_if_index;

			} rcv;
		} info;

	} SYSF_DL_DISPATCH_INFO;

	typedef struct ncs_802_2_eth_hdr {
		uns8 dest_addr[6];
		uns8 src_addr[6];
		uns16 len;
		uns8 dsap;
		uns8 ssap;
		uns8 ctrl;
	} NCS_802_2_ETH_HDR;

/**************************************************************/

	typedef enum {
		NCS_L2SOCKET_STATE_IDLE,
		NCS_L2SOCKET_STATE_ACTIVE,
		NCS_L2SOCKET_STATE_DISCONNECT,
		NCS_L2SOCKET_STATE_CLOSING,
		NCS_L2SOCKET_STATE_MAX = NCS_L2SOCKET_STATE_CLOSING
	} NCS_L2SOCKET_STATE;

	typedef enum {
		NCS_L2SOCKET_EVENT_OPEN,
		NCS_L2SOCKET_EVENT_CLOSE,
		NCS_L2SOCKET_EVENT_MULTICAST_JOIN,
		NCS_L2SOCKET_EVENT_MULTICAST_LEAVE,
		NCS_L2SOCKET_EVENT_SEND_DATA,
		NCS_L2SOCKET_EVENT_READ_INDICATION,
		NCS_L2SOCKET_EVENT_WRITE_INDICATION,
		NCS_L2SOCKET_EVENT_ERROR_INDICATION,
		NCS_L2SOCKET_EVENT_MAX = NCS_L2SOCKET_EVENT_ERROR_INDICATION
	} NCS_L2SOCKET_EVENT;

	struct ncs_l2socket_entry_tag;

	typedef uns32 (*NCS_L2SOCKET_HANDLER) (struct ncs_l2socket_entry_tag * se, NCSCONTEXT arg);

	typedef struct ncs_l2socket_entry_tag {

		struct ncs_l2socket_entry_tag *next;
		struct ncs_l2socket_entry_tag *prev;
		const NCS_L2SOCKET_HANDLER *dispatch;
		struct ncs_l2socket_context_tag *socket_context;
		NCS_LOCK lock;
		SOCKET client_socket;
		uns32 hm_hdl;
		int state;
		NCS_DL_TYPE dl_type;
		uns16 protocol;	/* eg: IP, ARP, etc only if DGRAM */
		uns32 mtu;
		NCSCONTEXT user_handle;	/* ISIS */
		NCSCONTEXT user_connection_handle;	/* ISIS_INTF */
		NCS_L2_ADDR local_addr;
		NCS_L2_ADDR remote_addr;
		NCS_BOOL shared;	/* shared = allow multiple opens 
					   for the same ethertype with 
					   distinct L2 addresses.
					   Shared = False, implies
					   only one open for a ethertype
					   allowed with no filter on
					   L2 address 

					 */

		uns32 if_index;
		uns8 if_name[NCS_IF_NAMESIZE];

		NCS_DL_INDICATION dl_data_indication;
		NCS_DL_INDICATION dl_ctrl_indication;
		NCS_BOOL bindtodevice;	/* If 1, use BIND to Device option - Raw Only */
		uns32 refcount;

	} NCS_L2SOCKET_ENTRY;

	typedef struct ncs_l2socket_list_tag {
		fd_set readfds;
		fd_set writefds;
		int socket_entries_freed;
		NCS_L2SOCKET_ENTRY *first;
		NCS_L2SOCKET_ENTRY *last;
		NCS_LOCK lock;
		int num_in_list;
	} NCS_L2SOCKET_LIST;

	typedef struct ncs_l2socket_context_tag {
		NCS_L2SOCKET_LIST ReceiveQueue;
		NCS_LOCK lock;
		int select_timeout;
		int max_connections;
		int stop_flag;
		int socket_timeout_flag;
		void *task_handle;
		NCS_PATRICIA_TREE socket_tree;
		NCS_PATRICIA_PARAMS tree_params;
		/* Added for IROOO59000 BUG */
		NCS_SEL_OBJ fast_open_sel_obj;
	} NCS_L2SOCKET_CONTEXT;

	typedef struct key_tag {
		uns16 protocol;
		uns8 addr[6];
		uns32 if_index;
	} NCS_L2FILTER_KEY;

	typedef struct ncs_l2filter_entry_tag {
		NCS_PATRICIA_NODE node;
		NCS_L2FILTER_KEY key;

		NCSCONTEXT user_connection_handle;
		NCS_L2SOCKET_ENTRY *se;
		uns32 hm_hdl;

	} NCS_L2FILTER_ENTRY;

#ifndef m_NCSSOCK_SOERROR
#if (NCS_USE_BSD_STYLE_ERROR == 1)
#define m_NCSSOCK_SOERROR(se) m_NCSSOCK_ERROR
	NCSSOCK_ERROR_TYPES ncs_bsd_sock_error(void);
#else
#define m_NCSSOCK_SOERROR(se) NCSSOCK_ERROR_TYPES_UNKNOWN
#endif
#endif

#ifdef  __cplusplus
}
#endif

#endif   /* NCS_DLPRM_H */
