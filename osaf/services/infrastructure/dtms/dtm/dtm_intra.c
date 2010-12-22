/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2010 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.z
 *
 * Author(s): GoAhead Software
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ncs_main_papi.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"
#include "ncssysf_mem.h"
#include "dtm.h"
#include "dtm_cb.h"
#include "dtm_intra.h"
#include "dtm_intra_disc.h"
#include "dtm_intra_trans.h"
#include "dtm_inter_trans.h"

DTM_INTRANODE_CB *dtm_intranode_cb = NULL;

#define DTM_INTRANODE_MAX_PROCESSES   100
#define DTM_INTRANODE_POLL_TIMEOUT 20000
#define DTM_RECV_BUFFER_SIZE 1500
#define DTM_INTRANODE_TASKNAME  "DTM_INTRANODE"
#define DTM_INTRANODE_TASK_PRIORITY  NCS_TASK_PRIORITY_4
#define DTM_INTRANODE_STACKSIZE  NCS_STACKSIZE_HUGE

static struct pollfd dtm_intranode_pfd[DTM_INTRANODE_MAX_PROCESSES];
static struct pollfd pfd_list[DTM_INTRANODE_MAX_PROCESSES];

static int  dtm_intranode_max_fd;

static uns32 dtm_intra_processing_init(void);
static void dtm_intranode_processing(void);
static uns32 dtm_intranode_add_poll_fdlist(int fd, uns16 event);
static uns32 dtm_intranode_create_rcv_task(int task_hdl);
static uns32 dtm_intranode_process_incoming_conn(void);
static uns32 dtm_intranode_del_poll_fdlist(int fd);

static uns32 dtm_intranode_fill_fd_set(void);



/**
 * Function to init the service descovery
 *
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
uns32 dtm_service_discovery_init(void)
{
	return dtm_intra_processing_init();
}

#define DTM_INTRANODE_SOCK_SIZE 64000

/**
 * Function to init the intranode processing
 *
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
uns32 dtm_intra_processing_init(void)
{

	int servlen, sock_opt, size = DTM_INTRANODE_SOCK_SIZE;	/* For socket fd and server len */
	struct sockaddr_un serv_addr;	/* For Unix Sock address */
	char server_ux_name[255];
	NCS_PATRICIA_PARAMS pat_tree_params;

	TRACE_ENTER();
	if (NULL == (dtm_intranode_cb = calloc(1, sizeof(DTM_INTRANODE_CB)))) {
		syslog(LOG_ERR, "\nMemory allocation failed for dtm_intranode_cb");
		return NCSCC_RC_FAILURE;
	}

	/* Open a socket, If socket opens to fail return Error */
	dtm_intranode_cb->server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (dtm_intranode_cb->server_sockfd < 0) {
		syslog(LOG_ERR, "\nSocket creation failed");
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}

	/*Make the socket Non-Blocking for accepting */
	sock_opt = fcntl(dtm_intranode_cb->server_sockfd, F_SETFL, O_NONBLOCK);

	if (sock_opt != 0) {
		/*Non-Blocking Options hasnt been set, what shall we do now */
		syslog(LOG_ERR, "\nSocket NON Block set failed");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}

	/* Increase the socket buffer size */
	if (setsockopt(dtm_intranode_cb->server_sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) != 0) {
		syslog(LOG_ERR, "DTM: Unable to set the SO_RCVBUF ");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}
	if (setsockopt(dtm_intranode_cb->server_sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) != 0) {
		syslog(LOG_ERR, "DTM: Unable to set the SO_SNDBUF ");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}

	dtm_intranode_cb->nodeid = m_NCS_GET_NODE_ID;

	bzero((char *)&serv_addr, sizeof(serv_addr));

#define UX_SOCK_NAME_PREFIX "/tmp/dtm_intra_server"

	sprintf(server_ux_name, "%s", UX_SOCK_NAME_PREFIX);
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, server_ux_name);

	unlink(serv_addr.sun_path);

	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	/* Bind the created socket here with the address  NODEID, 
	 *  if bind fails return error by the closing the
	 * created socket*/

	if (bind(dtm_intranode_cb->server_sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
		syslog(LOG_ERR, "\nBind failed");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}
	if (chmod(UX_SOCK_NAME_PREFIX,(S_IROTH | S_IWOTH ))< 0 ) { 
		syslog(LOG_ERR, "Unable to set the permission to unix server sock");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}

	listen(dtm_intranode_cb->server_sockfd, 20);
	memset(&pat_tree_params, 0, sizeof(pat_tree_params));
	pat_tree_params.key_size = sizeof(uns32);
	if (NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&dtm_intranode_cb->dtm_intranode_pid_list, &pat_tree_params)) {
		syslog(LOG_ERR, "\n ncs_patricia_tree_init failed for dtm_intranode_pid_list");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}

	pat_tree_params.key_size = sizeof(int);
	if (NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&dtm_intranode_cb->dtm_intranode_fd_list, &pat_tree_params)) {
		syslog(LOG_ERR, "\n ncs_patricia_tree_init failed for dtm_intranode_pid_list");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}

	pat_tree_params.key_size = sizeof(uns32);
	if (NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&dtm_intranode_cb->dtm_svc_subscr_list, &pat_tree_params)) {
		syslog(LOG_ERR, "\n ncs_patricia_tree_init failed for dtm_intranode_pid_list");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}

	pat_tree_params.key_size = sizeof(uns32);
	if (NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&dtm_intranode_cb->dtm_svc_install_list, &pat_tree_params)) {
		syslog(LOG_ERR, "\n ncs_patricia_tree_init failed for dtm_intranode_pid_list");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}

	dtm_intranode_add_self_node_to_node_db(dtm_intranode_cb->nodeid);

	if (m_NCS_IPC_CREATE(&dtm_intranode_cb->mbx) != NCSCC_RC_SUCCESS) {
		/* Mail box creation failed */
		syslog(LOG_ERR, "\nDTM : Intranode Mailbox Creation failed");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	} else {

		NCS_SEL_OBJ obj;

		/* Code added for attaching the mailbox, to eliminate the DBG Print at sysf_ipc.c: 640 */
		if (NCSCC_RC_SUCCESS != m_NCS_IPC_ATTACH(&dtm_intranode_cb->mbx)) {
			m_NCS_IPC_RELEASE(&dtm_intranode_cb->mbx, NULL);
			close(dtm_intranode_cb->server_sockfd);
			free(dtm_intranode_cb);
			syslog(LOG_ERR, "\nDTM: Intranode Mailbox  Attach failed");
			return NCSCC_RC_FAILURE;
		}

		obj = m_NCS_IPC_GET_SEL_OBJ(&dtm_intranode_cb->mbx);

		/* retreive the corresponding fd for mailbox and fill it in cb */
		dtm_intranode_cb->mbx_fd = m_GET_FD_FROM_SEL_OBJ(obj);	/* extract and fill value needs to be extracted */
	}

	dtm_intranode_add_poll_fdlist(dtm_intranode_cb->server_sockfd, POLLIN);
	dtm_intranode_add_poll_fdlist(dtm_intranode_cb->mbx_fd, POLLIN);

	if (dtm_intranode_create_rcv_task(dtm_intranode_cb->task_hdl) != NCSCC_RC_SUCCESS) {
		syslog(LOG_ERR, "\nMDS:MDTM: Receive Task Creation Failed in MDTM_INIT\n");
		close(dtm_intranode_cb->server_sockfd);
		free(dtm_intranode_cb);
		return NCSCC_RC_FAILURE;
	}

	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;

}

/**
 * Function to create the rcv task thread
 *
 * @param task_hdl
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
static uns32 dtm_intranode_create_rcv_task(int task_hdl)
{
	/*
	   STEP 1: Create a recv task which will accept the connections, recv data from the local nodes */

	TRACE_ENTER();
	if (m_NCS_TASK_CREATE((NCS_OS_CB)dtm_intranode_processing,
			      (NCSCONTEXT)(long)task_hdl,
			      DTM_INTRANODE_TASKNAME,
			      DTM_INTRANODE_TASK_PRIORITY, DTM_INTRANODE_STACKSIZE,
			      &dtm_intranode_cb->dtm_intranode_hdl_task) != NCSCC_RC_SUCCESS) {
		syslog(LOG_ERR, "\nIntr NODE Task Creation-failed");
		return NCSCC_RC_FAILURE;
	}

	/* Start the created task,
	 *   if start fails,
	 *        release the task by calling the NCS task release function*/
	if (m_NCS_TASK_START(dtm_intranode_cb->dtm_intranode_hdl_task) != NCSCC_RC_SUCCESS) {
		syslog(LOG_ERR, "\nStart of the Created Task-failed");
		m_NCS_TASK_RELEASE(dtm_intranode_cb->dtm_intranode_hdl_task);
		return NCSCC_RC_FAILURE;
	}
	/* return NCS success */
	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

/**
 * Function to handle the intranode processing
 *
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
static void dtm_intranode_processing(void)
{
	TRACE_ENTER();
	while (1) {
		int poll_ret_val = 0, j = 0;
		for (j = 0; j < dtm_intranode_max_fd; j++)
			pfd_list[j] = dtm_intranode_pfd[j];

		poll_ret_val = poll(pfd_list, dtm_intranode_max_fd, DTM_INTRANODE_POLL_TIMEOUT);

		if (poll_ret_val > 0) {
			int num_fd_checked = 0, i = 0;
			for (i = 0; i < dtm_intranode_max_fd; i++) {
				if (pfd_list[i].revents & POLLIN) {
					num_fd_checked++;
					if (pfd_list[i].fd == dtm_intranode_cb->server_sockfd) {
						/* Read indication on server listening socket */
						/* Accept the incoming connection */
						dtm_intranode_process_incoming_conn();
					} else if (pfd_list[i].fd == dtm_intranode_cb->mbx_fd) {
						/* Message process from internode */
						DTM_RCV_MSG_ELEM *msg_elem = NULL;

						msg_elem =
						    (DTM_RCV_MSG_ELEM
						     *) (m_NCS_IPC_NON_BLK_RECEIVE(&dtm_intranode_cb->mbx, NULL));

						if (NULL == msg_elem) {
							syslog(LOG_ERR,
							       "\nDTM: Intra Node Mailbox IPC_NON_BLK_RECEIVE Failed");
							continue;
						} else if (DTM_MBX_UP_TYPE == msg_elem->type) {
							dtm_process_internode_service_up_msg(msg_elem->info.svc_event.
											     buffer,
											     msg_elem->info.svc_event.
											     len,
											     msg_elem->info.svc_event.
											     node_id);
						} else if (DTM_MBX_DOWN_TYPE == msg_elem->type) {
							dtm_process_internode_service_down_msg(msg_elem->info.svc_event.
											       buffer,
											       msg_elem->info.svc_event.
											       len,
											       msg_elem->info.svc_event.
											       node_id);
						} else if (DTM_MBX_NODE_UP_TYPE == msg_elem->type) {
							dtm_intranode_process_node_up(msg_elem->info.node.node_id,
										      msg_elem->info.node.node_name,
										      msg_elem->info.node.mbx);
						} else if (DTM_MBX_NODE_DOWN_TYPE == msg_elem->type) {
							dtm_intranode_process_node_down(msg_elem->info.node.node_id);
						} else if (DTM_MBX_MSG_TYPE == msg_elem->type) {
							dtm_process_rcv_internode_data_msg(msg_elem->info.data.buffer,
											   msg_elem->info.data.dst_pid,
											   msg_elem->info.data.len);
						} else {
							syslog(LOG_ERR, "\nDTM Intranode :Invalid evt type from mbx");
						}
						free(msg_elem);

					} else {
						/* Data to be received on accepted connections */
						uns8 putbuf[2];
						uns8 *data;
						int recd_bytes = 0, put_len = 2;

						recd_bytes = recv(pfd_list[i].fd, putbuf, put_len, 0);

						if (0 == recd_bytes) {
							/*  recd_bytes==  0 indicates a connection close */
							/* Get the pat node from fd tree, 
							   Send the SVC down to subscriber list 
							   delete the entry from the fd list */
							dtm_intranode_process_pid_down(pfd_list[i].fd);
							dtm_intranode_del_poll_fdlist(pfd_list[i].fd);

						} else if (2 == recd_bytes) {
							DTM_INTRANODE_RCV_MSG_TYPES msg_type = 0;
							uns32 identifier = 0;
							uns8 version = 0, *inbuf = NULL;
							uns16 length = 0;

							data = putbuf;
							length = ncs_decode_16bit(&data);

							if (NULL == (inbuf = calloc(1, (length + 3)))) {
								/* Length + 2 is done to reuse the same buffer while sending to other nodes */
								syslog(LOG_ERR,
								       "\nMemory allocation failed in dtm_intranode_processing");
								assert(0);
								continue;
							}

							recd_bytes = recv(pfd_list[i].fd, &inbuf[2], length, 0);

							if (length != recd_bytes) {
								/* can happen only in two cases, system call interrupt or half data,
								   half data not possible as per design */
								syslog(LOG_ERR, "len mismatch len = %d, rcv bytes =%d",
								       length, recd_bytes);
								assert(0);
							}

							inbuf[length + 2] = '\0';

							data = &inbuf[2];
							/* Decode the message */
							identifier = ncs_decode_32bit(&data);
							version = ncs_decode_8bit(&data);

							if ((DTM_INTRANODE_RCV_MSG_IDENTIFIER != identifier)
							    || (DTM_INTRANODE_RCV_MSG_VER != version)) {
								syslog(LOG_ERR,
								       "\nMalformed packet recd, Ident = %d, ver = %d",
								       identifier, version);
								free(inbuf);
								continue;
							}

							msg_type = ncs_decode_8bit(&data);

							if (DTM_INTRANODE_RCV_PID_TYPE == msg_type) {
								dtm_intranode_process_pid_msg(&inbuf[8],
											      pfd_list[i].fd);
								free(inbuf);
							} else if (DTM_INTRANODE_RCV_BIND_TYPE == msg_type) {
								dtm_intranode_process_bind_msg(&inbuf[8],
											       pfd_list[i].fd);
								free(inbuf);
							} else if (DTM_INTRANODE_RCV_UNBIND_TYPE == msg_type) {
								dtm_intranode_process_unbind_msg(&inbuf[8],
												 pfd_list[i].fd);
								free(inbuf);
							} else if (DTM_INTRANODE_RCV_SUBSCRIBE_TYPE == msg_type) {
								dtm_intranode_process_subscribe_msg(&inbuf[8],
												    pfd_list[i].fd);
								free(inbuf);
							} else if (DTM_INTRANODE_RCV_UNSUBSCRIBE_TYPE == msg_type) {
								dtm_intranode_process_unsubscribe_msg(&inbuf[8],
												      pfd_list[i].fd);
								free(inbuf);
							} else if (DTM_INTRANODE_RCV_NODE_SUBSCRIBE_TYPE == msg_type) {
								dtm_intranode_process_node_subscribe_msg(&inbuf[8],
													 pfd_list[i].
													 fd);
								free(inbuf);
							} else if (DTM_INTRANODE_RCV_NODE_UNSUBSCRIBE_TYPE == msg_type) {
								dtm_intranode_process_node_unsubscribe_msg(&inbuf[8],
													   pfd_list[i].
													   fd);
								free(inbuf);
							} else if (DTM_INTRANODE_RCV_MESSAGE_TYPE == msg_type) {
								/* Get the Destination Node ID */
								NODE_ID dst_nodeid = 0;
								uns32 dst_processid = 0;
								dst_nodeid = ncs_decode_32bit(&data);
								dst_processid = ncs_decode_32bit(&data);
								if (dtm_intranode_cb->nodeid == dst_nodeid) {
									/* local node message */
									dtm_intranode_process_rcv_data_msg(inbuf,
													   dst_processid,
													   (recd_bytes +
													    2));
								} else {
									/* remote node message */
									dtm_add_to_msg_dist_list(inbuf,
												 (recd_bytes + 2),
												 dst_nodeid);
								}
							} else {
								/* msg_type not supported, log error */
								syslog(LOG_ERR,
								       "\nRecd msg_type unknown, dropping the message");
								free(inbuf);
							}
						} else {
							/* Some error, let it we check the data again */
						}
					}
				} else if (pfd_list[i].revents & POLLOUT) {
					num_fd_checked++;
					dtm_intranode_process_pollout(pfd_list[i].fd);
				} else if (pfd_list[i].revents & POLLHUP) {
					num_fd_checked++;
					dtm_intranode_process_pid_down(pfd_list[i].fd);
					dtm_intranode_del_poll_fdlist(pfd_list[i].fd);
				}
				if (num_fd_checked == poll_ret_val) {
					/* No more FD's to be checked */
					break;
				}
			}
		}
	}			/* While loop */
	TRACE_LEAVE();
}

/**
 * Function to add the fd to fdlist for intranode
 *
 * @param fd event
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
static uns32 dtm_intranode_add_poll_fdlist(int fd, uns16 event)
{
	DTM_INTRANODE_POLLFD_LIST *alloc_ptr = NULL, *head_ptr = NULL, *tail_ptr = NULL;

	TRACE_ENTER();
	if (NULL == (alloc_ptr = calloc(1, sizeof(DTM_INTRANODE_POLLFD_LIST)))) {
		return NCSCC_RC_FAILURE;
	}
	alloc_ptr->dtm_intranode_fd.fd = fd;
	alloc_ptr->dtm_intranode_fd.events = event;

	head_ptr = dtm_intranode_cb->fd_list_ptr_head;
	tail_ptr = dtm_intranode_cb->fd_list_ptr_tail;

	alloc_ptr->next = NULL;

	if (head_ptr == NULL) {
		dtm_intranode_cb->fd_list_ptr_head = alloc_ptr;
		dtm_intranode_cb->fd_list_ptr_tail = alloc_ptr;
	} else {
		tail_ptr->next = alloc_ptr;
		dtm_intranode_cb->fd_list_ptr_tail = alloc_ptr;
	}
	dtm_intranode_max_fd++;
	dtm_intranode_fill_fd_set();
	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

/**
 * Function to delete the fd from fdlist
 *
 * @param fd
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
static uns32 dtm_intranode_del_poll_fdlist(int fd)
{
	DTM_INTRANODE_POLLFD_LIST *back = NULL, *mov_ptr = NULL;

	TRACE_ENTER();
	for (back = NULL, mov_ptr = dtm_intranode_cb->fd_list_ptr_head; mov_ptr != NULL;
	     back = mov_ptr, mov_ptr = mov_ptr->next) {
		if (fd == mov_ptr->dtm_intranode_fd.fd) {
			/* STEP: Detach "mov_ptr" from linked-list */
			if (back == NULL) {
				/* The head node is being deleted */
				dtm_intranode_cb->fd_list_ptr_head = mov_ptr->next;
				dtm_intranode_cb->fd_list_ptr_tail = mov_ptr->next;

			} else {
				back->next = mov_ptr->next;
				if (NULL == mov_ptr->next) {
					dtm_intranode_cb->fd_list_ptr_tail = back;
				}
			}

			/* STEP: Detach "mov_ptr" from linked-list */
			free(mov_ptr);
			mov_ptr = NULL;
			dtm_intranode_max_fd--;
			dtm_intranode_fill_fd_set();
			TRACE("Successfully deleted fd list");
			return NCSCC_RC_SUCCESS;
		}
	}
	syslog(LOG_ERR, "\nNo matching entry found in fd list");
	TRACE_LEAVE();
	return NCSCC_RC_FAILURE;
}

/**
 * Function to set the fdlist
 *
 * @param fd events
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
uns32 dtm_intranode_set_poll_fdlist(int fd, uns16 events)
{
	DTM_INTRANODE_POLLFD_LIST *mov_ptr = dtm_intranode_cb->fd_list_ptr_head;

	TRACE_ENTER();
	if (mov_ptr == NULL) {
		syslog(LOG_ERR, "\nUnable to set the event in the poll list");
		return NCSCC_RC_FAILURE;
	}
	while (mov_ptr != NULL) {
		if (fd == mov_ptr->dtm_intranode_fd.fd) {
			mov_ptr->dtm_intranode_fd.events = mov_ptr->dtm_intranode_fd.events | events;
			dtm_intranode_fill_fd_set();
			return NCSCC_RC_SUCCESS;
		}
		mov_ptr = mov_ptr->next;
	}
	syslog(LOG_ERR, "\nUnable to set the event in the poll list");
	TRACE_LEAVE();
	return NCSCC_RC_FAILURE;
}

/**
 * Function to reset the poll fdlist
 *
 * @param fd
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
uns32 dtm_intranode_reset_poll_fdlist(int fd)
{
	DTM_INTRANODE_POLLFD_LIST *mov_ptr = dtm_intranode_cb->fd_list_ptr_head;

	TRACE_ENTER();
	if (mov_ptr == NULL) {
		syslog(LOG_ERR, "\nUnable to set the event in the poll list");
		return NCSCC_RC_FAILURE;
	}
	while (mov_ptr != NULL) {
		if (fd == mov_ptr->dtm_intranode_fd.fd) {
			mov_ptr->dtm_intranode_fd.events = POLLIN;
			dtm_intranode_fill_fd_set();
			return NCSCC_RC_SUCCESS;
		}
		mov_ptr = mov_ptr->next;
	}
	syslog(LOG_ERR, "\nUnable to set the event in the poll list");
	TRACE_LEAVE();
	return NCSCC_RC_FAILURE;
}

/**
 * Function to fill the fd set for intranode
 *
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
static uns32 dtm_intranode_fill_fd_set(void)
{
	int i = 0;
	DTM_INTRANODE_POLLFD_LIST *mov_ptr = dtm_intranode_cb->fd_list_ptr_head;

	TRACE_ENTER();
	for (i = 0; i < dtm_intranode_max_fd; i++) {
		if (NULL != mov_ptr) {
			dtm_intranode_pfd[i].fd = mov_ptr->dtm_intranode_fd.fd;
			dtm_intranode_pfd[i].events = mov_ptr->dtm_intranode_fd.events;
			mov_ptr = mov_ptr->next;
		} else {
			syslog(LOG_ERR, "\nDTM Intranode dtm_intranode_max_fd and fd_list differ");
			return NCSCC_RC_FAILURE;
		}
	}
	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

/**
 * Function to process the incoming connection request
 *
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
static uns32 dtm_intranode_process_incoming_conn(void)
{
	/* Accept processing */
	int accept_fd = 0, sock_opt = 0, retry_count = 0, size = DTM_INTRANODE_SOCK_SIZE;
	socklen_t len = sizeof(struct sockaddr_un);
	struct sockaddr_un cli_addr;
	/* Accept should be non_block */
	TRACE_ENTER();
	accept_fd = accept(dtm_intranode_cb->server_sockfd, (struct sockaddr *)&cli_addr, &len);

	/* Error Checking */
	if (accept_fd < 0) {
		syslog(LOG_ERR, "\n Connection accept fail");
		return NCSCC_RC_FAILURE;
	}

 tryagain:
	/*Make the socket Non-Blocking for accepting */
	sock_opt = fcntl(accept_fd, F_SETFL, O_NONBLOCK);
	if (sock_opt != 0) {
		syslog(LOG_ERR, "\naccept_fd Non-Blocking hasnt been Set");
		retry_count++;
		/* Non-Blocking Options hasnt been set */
		if (retry_count > 3) {
			assert(0);
		} else {
			goto tryagain;
		}
	}
	if (setsockopt(accept_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) != 0) {
		syslog(LOG_ERR, "DTM: Unable to set the SO_RCVBUF ");
		close(accept_fd);
		return NCSCC_RC_FAILURE;
	}
	if (setsockopt(accept_fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) != 0) {
		syslog(LOG_ERR, "DTM: Unable to set the SO_SNDBUF ");
		close(accept_fd);
		return NCSCC_RC_FAILURE;
	}
	dtm_intranode_add_poll_fdlist(accept_fd, POLLIN);
	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}
