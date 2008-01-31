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

#ifndef NCS_IPCSTATS_MOD
#define NCS_IPCSTATS_MOD

#if (NCS_IPCQUE_DEBUG != 0 )

#ifndef REMOVE_ASAP
/* 
** This must be based on a real value for now arbitrarily set for testing 
*/

#define MIN_MSG_LEN 20 
#endif

typedef struct ipc_stats_elem 
{
    /*
    ** Common fields among resources.
    */

    struct ipc_stats_elem *next;
    void *   resource;                 
    unsigned short owner_id;           /* Subsystem id            */
    unsigned short resource_id;        /* Identifier internally generated */

    unsigned int    highWaterMark;    /* Max Depth reached */
    unsigned int    currDepth;        /* Current Depth     */
    unsigned int    refCount;         /* Reference count   */
    unsigned int    failureCnt;       /* Bumped on errors  */
    unsigned int    hiPriCnt;         /* Hi priority part  */
    unsigned long   creationTime;     /* Creation time     */
} NCS_CMN_STATS;

typedef struct ncs_diag_stat 
{
    struct ipc_stats_elem  *head;
    struct ipc_stats_elem  *tail;
    /*
    ** Cannot use lock definition since it is defined by sysf_ipc.h
    ** and we must be included before it is.
    */

    void *stats_lock;
    /*
    ** Can keep general stats for creations and deletion of 
    ** resource here, for now just leave the linkage.
    */

} NCS_DIAG_STAT;

/*
** The following are macros specific to the IPC queue resource.
*/


/* Add a statistics element into list */
#define m_NCS_STATS_IPC_ADD_ENV(a){ncs_add_stats_element(&a->ipc_stats_element,a);} 
/* Remove statistics element from list */
#define m_NCS_STATS_IPC_REM_ENV(a)(ncs_rem_stats_element(&a->ipc_stats_element))
/* Increment Priority count */
#define m_NCS_STATS_IPC_BUMP_HI_PRI(a)(a->ipc_stats_element.hiPriCnt++)
/* Increment current depth count */
#define m_NCS_STATS_IPC_BUMP_CUR_DEPTH(a,b){if(a->ipc_stats_element.currDepth++ >= a->ipc_stats_element.highWaterMark){a->ipc_stats_element.highWaterMark = a->ipc_stats_element.currDepth;}if(b==0){a->ipc_stats_element.hiPriCnt++;}}
/* Increment Failure Count   */
#define m_NCS_STATS_IPC_BUMP_FAIL_CNT(a)(a->ipc_stats_element.failureCnt++)
/* Increment reference count */
#define m_NCS_STATS_IPC_BUMP_REF_CNT(a)(a->ipc_stats_element.refCount++)
/* Decrement reference count */
#define m_NCS_STATS_IPC_DEC_REF_CNT(a)(a->ipc_stats_element.refCount--)
/* Decrement current depth count */
#define m_NCS_STATS_IPC_DEC_CUR_DEPTH(a,b){if(b==0){a->ipc_stats_element.hiPriCnt--;}a->ipc_stats_element.currDepth--;}
/* Decrement Failure count   */
#define m_NCS_STATS_IPC_DEC_FAIL_CNT(a)(a->ipc_stats_element.failureCnt--)

#ifdef I_AM_IPCSTATS_MOD /*defined ONLY in companion c-module to expand glbs */
struct ncs_diag_stat ncs_ipc_stats = {0, 0, 0}, *ncs_ipcDiagStats = 0;
#else
extern struct ncs_diag_stat ncs_ipc_stats;
#endif
/*
** Function Prototypes:
** Should be inline if allowed by compiler.
*/

void ncs_add_stats_element(struct ipc_stats_elem *, void *);
void ncs_rem_stats_element(struct ipc_stats_elem *);
#else  /* ! NCS_IPCQUEUE_DEBUG */
/*
** Define macro stubs 
*/
#define m_NCS_STATS_IPC_ADD_ENV( a ) 
#define m_NCS_STATS_IPC_REM_ENV( a ) 

#define m_NCS_STATS_IPC_BUMP_CUR_DEPTH(a,b)
#define m_NCS_STATS_IPC_BUMP_FAIL_CNT(a)
#define m_NCS_STATS_IPC_BUMP_REF_CNT(a)

#define m_NCS_STATS_IPC_DEC_CUR_DEPTH(a,b)
#define m_NCS_STATS_IPC_DEC_REF_CNT(a)
#define m_NCS_STATS_IPC_DEC_FAIL_CNT(a)

#endif  /* NCS_IPCQUEUE_DEBUG */

/* Function Prototypes */
int ncs_dumpIpcStatsToConsole( void );
int ncs_dumpIpcStatsToFile( char * filename );
int ncs_walkAndDumpIpcQue( int id, int cnt );

#endif  /* ifndef NCS_IPCSTATS_MOD gate keeper */
