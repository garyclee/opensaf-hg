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

#include "mds_dt.h"
#include "mds_log.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"
#include "ncssysf_mem.h"
#include "mds_dt_tcp.h"
#include "mds_dt_tcp_disc.h"
#include "mds_dt_tcp_trans.h"

#include <sys/poll.h>
#include <poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <netdb.h>

#define MDS_MDTM_SOCKET_LOC "/tmp/mdtm_process"
#define MDS_MDTM_SUN_PATH 255
#define MDS_SND_RCV_SIZE 64000
#define MDS_MDTM_CONNECT_PATH "/tmp/dtm_intra_server"

/*  mds_indentifire + mds_version +   msg_type + node_id + process_id   */
#define MDS_MDTM_DTM_PID_SIZE 14	/* 4 + 1 + 1 + 4 + 4 */

/* Send_buffer_size + MDS_MDTM_DTM_PID_BUFFER_SIZE   */
#define MDS_MDTM_DTM_PID_BUFFER_SIZE (2 + MDS_MDTM_DTM_PID_SIZE)

uns32 socket_domain = AF_INET;
uns16 port = 0;

/* Get the pid of the process */
pid_t pid;

static void mds_mdtm_enc_init(MDS_MDTM_DTM_MSG * init, uns8 *buff);
static uns32 mdtm_create_rcv_task(void);
uns32 mdtm_process_recv_events_tcp(void);


/**
 * Initialize the tcp interface creation of sockets
 *
 * @param nodeid, mds_tcp_ref 
 *
 * @return NCSCC_RC_SUCCESS 
 * @return NCSCC_RC_FAILURE
 *
 */
uns32 mds_mdtm_init_tcp(NODE_ID nodeid, uns32 *mds_tcp_ref)
{
	uns32 flags;
	uns32 size = MDS_SND_RCV_SIZE;
	struct hostent *host;
	struct sockaddr_un client_addr_un, dhserver_addr_un;
	struct sockaddr_in client_addr_in, server_addr_in;
	uns8 buffer[MDS_MDTM_DTM_PID_BUFFER_SIZE];
	char sun_path[MDS_MDTM_SUN_PATH];
	char sun_path_connect[MDS_MDTM_SUN_PATH];

	host = gethostbyname("127.0.0.1");
	pid = getpid();

	MDS_MDTM_DTM_MSG send_evt;

	NCS_PATRICIA_PARAMS pat_tree_params;

	memset(&client_addr_un, 0, sizeof(struct sockaddr_un));
	memset(&dhserver_addr_un, 0, sizeof(struct sockaddr_un));
	memset(&client_addr_in, 0, sizeof(struct sockaddr_in));
	memset(&server_addr_in, 0, sizeof(struct sockaddr_in));
	memset(&send_evt, 0, sizeof(MDS_MDTM_DTM_MSG));
	memset(&buffer, 0, MDS_MDTM_DTM_PID_BUFFER_SIZE);
	memset(&sun_path, 0, MDS_MDTM_SUN_PATH);
	memset(&sun_path_connect, 0, MDS_MDTM_SUN_PATH);

	tcp_cb = (MDTM_TCP_CB *) malloc(sizeof(MDTM_TCP_CB));
	if (tcp_cb == NULL) {
		syslog(LOG_ERR, "MDS:MDTM: InSufficient Memory !!\n");
		return NCSCC_RC_FAILURE;
	}

	memset(tcp_cb, 0, sizeof(MDTM_TCP_CB));

	memset(&pat_tree_params, 0, sizeof(pat_tree_params));
	pat_tree_params.key_size = sizeof(MDTM_REASSEMBLY_KEY);
	if (NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&mdtm_reassembly_list, &pat_tree_params)) {
		syslog(LOG_ERR, "MDS:MDTM: ncs_patricia_tree_init failed MDTM_INIT\n");
		return NCSCC_RC_FAILURE;
	}

	/* Create the sockets required for Binding, Send, receive and Discovery */

	tcp_cb->DBSRsock = socket(socket_domain, SOCK_STREAM, 0);
	if (tcp_cb->DBSRsock < 0) {
		syslog(LOG_ERR, "MDS:MDTM: DBSRsock Socket creation failed in MDTM_INIT\n");
		return NCSCC_RC_FAILURE;
	}

	/* Increase the socket buffer size */
	if (setsockopt(tcp_cb->DBSRsock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) != 0) {
		syslog(LOG_ERR, "MDS:MDTM: Unable to set the SO_RCVBUF for DBSRs ck");
		close(tcp_cb->DBSRsock);
		return NCSCC_RC_FAILURE;
	}

	if (setsockopt(tcp_cb->DBSRsock, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) != 0) {
		syslog(LOG_ERR, "MDS:MDTM: Unable to set the SO_SNDBUF for DBSRsock");
		close(tcp_cb->DBSRsock);
		return NCSCC_RC_FAILURE;
	}

	flags = fcntl(tcp_cb->DBSRsock, F_GETFD, 0);
	if ((flags < 0) || (flags > 1)) {
		syslog(LOG_ERR, "MDS:MDTM: Unable to get the CLOEXEC Flag on DBSRsock");
		close(tcp_cb->DBSRsock);
		return NCSCC_RC_FAILURE;
	} else {
		if (fcntl(tcp_cb->DBSRsock, F_SETFD, (flags | FD_CLOEXEC)) == (-1)) {
			syslog(LOG_ERR, "MDS:MDTM: Unable to set the CLOEXEC Flag on DBSRsock");
			close(tcp_cb->DBSRsock);
			return NCSCC_RC_FAILURE;
		}
	}

	tcp_cb->adest = ((uns64)(nodeid)) << 32;
	tcp_cb->adest |= pid;
	tcp_cb->node_id = nodeid;

	*mds_tcp_ref = pid;

	if (socket_domain == AF_UNIX) {
		int servlen = 0;

		sprintf(sun_path, "%s_%d", MDS_MDTM_SOCKET_LOC, pid);
		sprintf(sun_path_connect, "%s", MDS_MDTM_CONNECT_PATH);

		dhserver_addr_un.sun_family = AF_UNIX;
		strcpy(dhserver_addr_un.sun_path, sun_path);

		servlen = strlen(dhserver_addr_un.sun_path) + sizeof(dhserver_addr_un.sun_family);

		/* Unlink as it may link to other process */
		unlink(sun_path);

		if (bind(tcp_cb->DBSRsock, (struct sockaddr *)&dhserver_addr_un, servlen) == -1) {
			syslog(LOG_ERR, "MDS:MDTM: Unable to bind on DBSRsock");
			close(tcp_cb->DBSRsock);
			return NCSCC_RC_FAILURE;
		}

		client_addr_un.sun_family = AF_UNIX;
		strcpy(client_addr_un.sun_path, sun_path_connect);

		servlen = strlen(client_addr_un.sun_path) + sizeof(client_addr_un.sun_family);
		/* Blocking Connect */
		if (connect(tcp_cb->DBSRsock, (struct sockaddr *)&client_addr_un, servlen) == -1) {
			syslog(LOG_ERR, "MDS:MDTM: DBSRsock unable to connect");
			close(tcp_cb->DBSRsock);
			return NCSCC_RC_FAILURE;
		}
	} else {
		/* Case for AF_INET */
		server_addr_in.sin_family = AF_INET;
		server_addr_in.sin_port = port;
		server_addr_in.sin_addr = *((struct in_addr *)host->h_addr);
		bzero(&(server_addr_in.sin_zero), 8);

		/* Blocking Connect */
		if (connect(tcp_cb->DBSRsock, (struct sockaddr *)&server_addr_in, sizeof(struct sockaddr)) == -1) {
			syslog(LOG_ERR, "MDS:MDTM: DBSRsock unable to connect");
			close(tcp_cb->DBSRsock);
			return NCSCC_RC_FAILURE;
		}
	}

	/* Send the Message to DH Server "Packet-type Node-id Pid" total of 9 bytes */
	send_evt.size = MDS_MDTM_DTM_PID_SIZE;
	send_evt.mds_version = MDS_SND_VERSION;
	send_evt.mds_indentifire = MDS_IDENTIFIRE;
	send_evt.type = MDS_MDTM_DTM_PID_TYPE;
	send_evt.info.pid.node_id = nodeid;
	send_evt.info.pid.process_id = pid;

	/* Convert into the encoded buffer before send */
	mds_mdtm_enc_init(&send_evt, buffer);

	/* This is still a blocking send */
	if (send(tcp_cb->DBSRsock, buffer, MDS_MDTM_DTM_PID_BUFFER_SIZE, 0) != MDS_MDTM_DTM_PID_BUFFER_SIZE) {
		syslog(LOG_ERR, "MDS:MDTM: Send on DBSRsock failed");
		close(tcp_cb->DBSRsock);
		return NCSCC_RC_FAILURE;
	}

	/* Convert the socket to a Non-blocking socket */
	flags = fcntl(tcp_cb->DBSRsock, F_SETFD, O_NONBLOCK);
	if ((flags < 0) || (flags > 1)) {
		syslog(LOG_ERR, "MDS:MDTM: Unable to get the CLOEXEC Flag on DBSRsock");
		close(tcp_cb->DBSRsock);
		return NCSCC_RC_FAILURE;
	}

	/* Code for Tmr Mailbox Creation used for Tmr Msg Retrival */

	if (m_NCS_IPC_CREATE(&tcp_cb->tmr_mbx) != NCSCC_RC_SUCCESS) {
		/* Mail box creation failed */
		syslog(LOG_ERR, "MDS:MDTM: Tmr Mailbox Creation failed:\n");
		close(tcp_cb->DBSRsock);
		return NCSCC_RC_FAILURE;
	} else {

		NCS_SEL_OBJ obj;

		if (NCSCC_RC_SUCCESS != m_NCS_IPC_ATTACH(&tcp_cb->tmr_mbx)) {
			m_NCS_IPC_RELEASE(&tcp_cb->tmr_mbx, NULL);
			syslog(LOG_ERR, "MDS:MDTM: Tmr Mailbox  Attach failed:\n");
			close(tcp_cb->DBSRsock);
			return NCSCC_RC_FAILURE;
		}

		obj = m_NCS_IPC_GET_SEL_OBJ(&tcp_cb->tmr_mbx);

		/* retreive the corresponding fd for mailbox and fill it in cb */
		tcp_cb->tmr_fd = m_GET_FD_FROM_SEL_OBJ(obj);	/* extract and fill value needs to be extracted */
		mdtm_attach_mbx(tcp_cb->tmr_mbx);
		mdtm_set_transport(MDTM_TX_TYPE_TCP);
	}

	/* Create a task to receive the events and data */
	if (mdtm_create_rcv_task() != NCSCC_RC_SUCCESS) {
		syslog(LOG_ERR, "MDS:MDTM: Receive Task Creation Failed in MDTM_INIT\n");
		close(tcp_cb->DBSRsock);
		m_NCS_IPC_RELEASE(&tcp_cb->tmr_mbx, NULL);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/**
 * Start the rcv thread
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
static uns32 mdtm_create_rcv_task(void)
{
	/*
	   STEP 1: Create a recv task which will recv data and
	   captures the discovery events as well */

	if (m_NCS_TASK_CREATE((NCS_OS_CB)mdtm_process_recv_events_tcp,
			      (NCSCONTEXT)NULL,
			      NCS_MDTM_TASKNAME,
			      NCS_MDTM_PRIORITY, NCS_MDTM_STACKSIZE, &tcp_cb->mdtm_hdle_task) != NCSCC_RC_SUCCESS) {
		m_MDS_LOG_ERR("MDTM: Task Creation-failed:\n");
		return NCSCC_RC_FAILURE;
	}

	/* Start the created task,
	 *   if start fails,
	 *        release the task by calling the NCS task release function*/
	if (m_NCS_TASK_START(tcp_cb->mdtm_hdle_task) != NCSCC_RC_SUCCESS) {
		m_MDS_LOG_ERR("MDTM: Start of the Created Task-failed:\n");
		m_NCS_TASK_RELEASE(tcp_cb->mdtm_hdle_task);
		m_MDS_LOG_ERR("MDTM: START of created task failed");
		return NCSCC_RC_FAILURE;
	}

	/* return NCS success */
	return NCSCC_RC_SUCCESS;
}

/**
 * Destroy the tcp interface close the sockets
 *
 *
 * @return NCSCC_RC_SUCCESS
 * @return NCSCC_RC_FAILURE
 *
 */
uns32 mds_mdtm_destroy_tcp(void)
{

	return NCSCC_RC_SUCCESS;
}

/**
 * Encode the send message
 *
 * @param Send buffer, Data packet
 *
 */
static void mds_mdtm_enc_init(MDS_MDTM_DTM_MSG * init, uns8 *data)
{
	uns8 *buff = data;
	ncs_encode_16bit(&buff, init->size);
	ncs_encode_32bit(&buff, init->mds_indentifire);
	ncs_encode_8bit(&buff, init->mds_version);
	ncs_encode_8bit(&buff, init->type);
	ncs_encode_32bit(&buff, init->info.pid.node_id);
	ncs_encode_32bit(&buff, init->info.pid.process_id);
}
