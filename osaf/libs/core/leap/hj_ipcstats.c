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

 */

/*
** I_AM_IPCSTATS_MOD tells cmndiagenv.h that if NCS_IPCQUE_DEBUG is also defined
** it should create global structures. Other modules including this file will
** not define this and will get an extern reference to these structures if
** NCS_IPCQUE_DEBUG is defined.
*/
#define  I_AM_IPCSTATS_MOD

#include "ncs_ipcstats.h"
#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"

#if (NCS_IPCQUE_DEBUG != 0)
/*
** Prototypes defined elsewere
*/
uns32 expose_flush(NCS_IPC *, NCS_IPC_CB, void *);
static NCS_LOCK ncs_ipc_stats_lock;
/*
** following functions are internal, just hide them
*/
void ncs_init_stats()
{
	m_NCS_LOCK_INIT(&ncs_ipc_stats_lock);
	ncs_ipc_stats.stats_lock = (void *)&ncs_ipc_stats_lock;
	ncs_ipcDiagStats = &ncs_ipc_stats;
}

void ncs_add_stats_element(struct ipc_stats_elem *statEnvelope, void *resource)
{
	/*
	 ** We are inside of a lock
	 */

	static unsigned short newResourceId = 1;	/*used to id a resource in the list */

	struct ncs_diag_stat *diagStats;

	statEnvelope->next = NULL;
	m_NCS_OS_GET_TIME_STAMP(statEnvelope->creationTime);
	statEnvelope->currDepth = 0;
	statEnvelope->hiPriCnt = 0;
	statEnvelope->failureCnt = 0;
	statEnvelope->highWaterMark = 0;
	statEnvelope->refCount = 0;

	/*
	 ** used to identify resouce to target message dumps.
	 */

	statEnvelope->resource_id = newResourceId++;

	/*
	 ** if this is the first attempt to access the list, some intialization
	 ** is needed, similar code should exist for each resource (e.g. tmr, etc )
	 */

	if (ncs_ipcDiagStats == NULL)
		ncs_init_stats();
	/*
	 ** setup local pointer
	 */
	diagStats = ncs_ipcDiagStats;
	statEnvelope->resource = resource;

	if (diagStats->head == NULL) {	/* list empty set head and tail */
		diagStats->head = diagStats->tail = statEnvelope;
	} else {
		diagStats->tail->next = statEnvelope;
		diagStats->tail = statEnvelope;
	}

}

void ncs_rem_stats_element(struct ipc_stats_elem *statEnvelope)
{
	/*
	 ** This function removes a statistics element from an active list
	 */

	struct ipc_stats_elem *curr, *last;
	struct ncs_diag_stat *stats;

	stats = &ncs_ipc_stats;

	curr = last = stats->head;

	while (curr) {
		if (curr->resource == statEnvelope) {

			if (curr == last) {	/* Only true at head of list */
				if ((stats->head = curr->next) == NULL)
					stats->tail = NULL;
			} else {
				/*
				 ** cut element off the list, and redo tail if needed
				 */

				if ((last->next = curr->next) == NULL)
					stats->tail = last;
			}
			break;
		}
		last = curr;
		curr = curr->next;
	}
}

void printInternalMsgDump(int cnt, void *msg)
{
	/*
	 ** Print formatted message
	 */
	int i;
	unsigned char *tmp = msg;
	printf("--Message Dump--\n");

	for (i = 0; i < cnt; i++) {
		printf("%2x ", *tmp++);
		if (((i + 1) % 30) == 0)
			printf("\n");
	}
	printf("\n");
}

NCS_BOOL internalDumpMsg(void *arg, void *msg)
{

	int cnt;

	cnt = min(MIN_MSG_LEN, *((int *)arg));

	printInternalMsgDump(cnt, msg);
	/*
	 ** A false return here will guarantee that the element is not
	 ** pulled out of the queue.
	 */

	return (FALSE);
}

/*
** end of internal functions, show interface for all others
*/
#endif

int ncs_dumpIpcStatsToConsole(void)
{
	int ret = 1;
#if (NCS_IPCQUE_DEBUG != 0)
	struct ipc_stats_elem *curr;
	/*
	 ** if this is the first attempt to access the list, some intialization
	 ** is needed, similar code should exist for each resource (e.g. tmr, etc )
	 */

	if (ncs_ipcDiagStats == NULL)
		ncs_init_stats();
	/*
	   m_NCS_LOCK ((NCS_LOCK *)&ncs_ipc_stats.stats_lock, NCS_LOCK_WRITE );
	 */
	if ((curr = ncs_ipc_stats.head) == NULL)
		printf("---- Statistics List contains no IPC Resources!-----\n");
	else {
		int i = 1;
		unsigned long now;
		m_NCS_OS_GET_TIME_STAMP(now);

		printf("|---|---+------+-------+------+----------+----------+---------------|\n");
		printf("|  #|Ref|high  |Current|Fail  |Q-counts  | Lifetime | Resource      |\n");
		printf("|   |cnt|water |Depth  |Count |Hi-p,Lo-p | in ticks |  Address      |\n");
		printf("|---|---+------+-------+------+----------+----------+---------------+\n");
		while (curr) {
			printf("%4d%4d%7d%8d%7d%5d %5d%11d%11x\n",
			       i++,
			       curr->refCount,
			       curr->highWaterMark,
			       curr->currDepth,
			       curr->failureCnt,
			       curr->hiPriCnt,
			       curr->currDepth - curr->hiPriCnt, now - curr->creationTime, curr->resource);
			curr = curr->next;
		}

	}
	/*
	   m_NCS_UNLOCK ((NCS_LOCK *)&ncs_ipc_stats.stats_lock, NCS_LOCK_WRITE);
	 */
#endif
	return (ret);
}

int ncs_dumpIpcStatsToFile(char *filename)
{
	int ret = 0;
	USE(filename);
#if (NCS_IPCQUE_DEBUG != 0)
#endif
	return (ret);
}

int ncs_walkAndDumpIpcQue(int id, int cnt)
{
	/*
	 ** Warning!! this allows for a peek into the queues content,
	 ** do not go over min msg lenght or access error will occur.
	 ** also be aware that this function is S L O W, to be used mostly
	 ** for post-mortem analysis.
	 */

	int ret = 0;
	USE(id);
	USE(cnt);
#if (NCS_IPCQUE_DEBUG != 0)
	struct ipc_stats_elem *curr;

	/*
	 ** lock the list
	 */

	/*  m_NCS_LOCK ((NCS_LOCK *)&ncs_ipc_stats.stats_lock, NCS_LOCK_WRITE ); */
	if ((curr = ncs_ipc_stats.head) == NULL)
		printf("---- Statistics List contains no IPC Resources!----\n");
	else {
		while (curr) {
			if (curr->resource_id == id) {
				NCS_IPC *tmp;
				tmp = (NCS_IPC *)curr->resource;

				/*
				 ** First lock the queue
				 */

				m_NCS_LOCK(&tmp->queue_lock, NCS_LOCK_WRITE);

				/*
				 ** This is a little tricky, we will call the flush
				 ** function with a call back function that copies
				 ** "cnt" bytes of content and returns false so that element
				 ** is not taken of queue. See Warning above.
				 */

				if (expose_flush((NCS_IPC *)curr->resource, internalDumpMsg, (void *)&cnt)
				    != NCSCC_RC_SUCCESS)
					printf("Could not access resource %d\n", id);

				/*
				 ** Release queue
				 */

				m_NCS_UNLOCK(&tmp->queue_lock, NCS_LOCK_WRITE);
				break;

			}
			curr = curr->next;
		}
		if (curr == NULL)
			printf("resource id %d not found\n", id);
	}
	/*
	 ** Release list
	 */
	/* m_NCS_UNLOCK ((NCS_LOCK *)&ncs_ipc_stats.stats_lock, NCS_LOCK_WRITE); */
#endif
	return (ret);
}
