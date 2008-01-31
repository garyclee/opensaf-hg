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



 
..............................................................................

  DESCRIPTION:

  This module contains structures related to System Monitoring operations

..............................................................................


******************************************************************************/

#ifndef SYSFSYSM_H
#define SYSFSYSM_H

#include "ncssysf_tmr.h"
#include "ncsusrbuf.h"
#include "usrbuf.h"

#define SM_BKT_CNT 20  /* each bucket represents 5 percent */


typedef struct sm_mem_watch
{
  struct sm_mem_watch* next;
  NCS_KEY               usr_key;
  uns32                percent;
  NCSCONTEXT            opaque;
  NCSMEM_EVT            cb_fnc;

} SM_MEM_WATCH;

typedef struct sm_cpu_watch
{
  struct sm_cpu_watch* next;
  NCS_KEY               usr_key;
  uns32                percent;
  NCSCONTEXT            opaque;
  NCSCPU_EVT            cb_fnc;

} SM_CPU_WATCH;

typedef struct sm_ipra_watch
{
  struct sm_ipra_watch* next;
  NCS_KEY               usr_key;
  NCSCONTEXT            opaque;
  NCSIPRA_EVT           cb_fnc;

} SM_IPRA_WATCH;

typedef struct sm_buf_watch
{
  struct sm_buf_watch* next;
  NCS_KEY               usr_key;
  uns32                pool_id;
  uns32                percent;
  NCSCONTEXT            opaque;
  NCSBUF_EVT            cb_fnc;

} SM_BUF_WATCH;

typedef struct sm_ipc_watch
{
  struct sm_ipc_watch* next;
  NCS_KEY               usr_key;
  struct ncs_ipc_stats* ipcq;
  uns32                percent;
  NCSCONTEXT            opaque;
  NCSIPC_EVT            cb_fnc;

} SM_IPC_WATCH;

typedef enum sm_pulse_type 
{ 
  SMPT_MEM,
  SMPT_BUF

} SM_PULSE_TYPE;

typedef struct sm_pulse
{
  tmr_t          tmr_id;
  uns32          period;
  TMR_CALLBACK   cb_fnc;
  SM_PULSE_TYPE  type;
  uns32          data;

} SM_PULSE;

typedef enum sm_msg_type
{
  SMMT_PULSE

} SM_MSG_TYPE;

typedef struct sm_msg
{
  struct sm_msg* next;
  SM_MSG_TYPE    type;
  NCSCONTEXT      data;

} SM_MSG;

#define SM_EXIST_MARKER  0x10721801

typedef struct ncs_sysmon
{
  uns32            exist_marker;     /* used to identify if the instance exists*/
  NCS_VRID          vrtr_id;          /* Virtual Router ID                      */
  NCS_KEY           usr_key;          /* user key                               */
  RES_LMT_CB       res_lmt_cb;       /* Resource Limit Callback function       */ 
  void*            sysmon_mbx;       /* MBX for the main SYSMON task           */
  char             sysmon_task_name[12];
  NCSCONTEXT        sysmon_task_hdl;

  /* M E M O R Y  M O N I T O R I N G : */

#if ((NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))
  uns32            mem_ttl;          /* MAX heap space allowed alloc'ed       */
  NCS_MMGR*         mmgr;             /* MEMORY MANAGER */
#endif
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
  uns32            mem_wrr;          /* Watch Refresh Rate for Memory watches */
  uns32            mem_bkt_size;     /* mem watch bucket size: mem_ttl / SM_BKT_CNT */
  SM_MEM_WATCH*    mem_watches[SM_BKT_CNT];   /* memory watches */
  uns32            mem_last;         /* last alloc'ed memory */
  uns32            mem_bkt_last;     /* last mem watch bucket */
  NCSCONTEXT        sm_mem_watch_task_hdl;
#endif
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
  NCS_BOOL          mem_ignore;       /* hide/expose memory currently out      */
  NCS_BOOL          mem_ss_ignore;    /* hide/expose current memory by subsys. */
  SM_PULSE         mem_pulse;        /* Increment MEM OUT every X seconds     */
#endif

  /* U S R B U F  M O N I T O R I N G : */

#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || ((NCSSYSM_BUF_STATS_ENABLE == 1)))
  uns32            buf_ttl[UB_MAX_POOLS];/* MAX USRBUF space allowed alloc'ed */
  UB_POOL_MGR*     ub_pool_mgr;          /* USRBUF pool manager               */
#endif
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
  uns32            buf_wrr;          /* Watch Refresh Rate for USRBUF watches */
  uns32            buf_bkt_size[UB_MAX_POOLS];/* buf watch bucket size: buf_ttl / SM_BKT_CNT */
  SM_BUF_WATCH*    buf_watches[SM_BKT_CNT][UB_MAX_POOLS]; /* buf watches      */
  uns32            buf_last[UB_MAX_POOLS]; /* last buf space alloc'ed         */
  uns32            buf_bkt_last[UB_MAX_POOLS]; /* last buf watch bucket       */
  NCSCONTEXT        sm_buf_watch_task_hdl; 
#endif
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
  NCS_BOOL          buf_ignore[UB_MAX_POOLS];/* hide/expose buffers currently out*/
  SM_PULSE         buf_pulse[UB_MAX_POOLS];/* Increment BUF OUT every X seconds*/
#endif

  /* C P U  M O N I T O R I N G : */

#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
  uns32            cpu_wrr;          /* Watch Refresh Rate for CPU watches */
  SM_CPU_WATCH*    cpu_watches[SM_BKT_CNT];   /* cpu watches                  */
  uns32            cpu_last;         /* last cpu usage %                      */
  uns32            cpu_bkt_last;     /* last cpu watch bucket                 */
  NCSCONTEXT        sm_cpu_watch_task_hdl;
#endif

  /* I P  R E S O U R C E  A V A I L A B I L I T Y  M O N I T O R I N G : */

#if (NCSSYSM_IPRA_WATCH_ENABLE == 1)
  SM_IPRA_WATCH*   ipra_watches;     /* ipra watches                          */
#endif

  /* I P C  M O N I T O R I N G : */

#if ((NCSSYSM_IPC_WATCH_ENABLE == 1) || (NCSSYSM_IPC_STATS_ENABLE == 1))
  uns32            ipc_ttl;          /* MAX depth any given IPC queue can go  */
  struct ncs_ipc_stats* ipc_stats;    /* ipc stats                             */
  uns32            ipc_bkt_size;     /* ipc watch bucket size: ipc_ttl / SM_BKT_CNT */
#endif
#if (NCSSYSM_IPC_WATCH_ENABLE == 1)
  uns32            ipc_wrr;          /* Watch Refresh Rate for IPC watches    */ 

  SM_IPC_WATCH*    ipc_watches[SM_BKT_CNT]; /* ipc watches                    */
  NCSCONTEXT        sm_ipc_watch_task_hdl;
#endif
#if (NCSSYSM_IPC_STATS_ENABLE == 1)
  NCS_BOOL          ipc_ignore;       /* hide/expose current ipc queue depth   */
#endif

  /* T I M E R  M O N I T O R I N G : */

#if ((NCSSYSM_TMR_WATCH_ENABLE == 1) || (NCSSYSM_TMR_STATS_ENABLE == 1))
  uns32            tmr_ttl;          /* MAX num tmrs allowed to run at once   */
#endif
#if (NCSSYSM_TMR_WATCH_ENABLE == 1)
  uns32            tmr_wrr;          /* Watch Refresh Rate for Timer watches  */
  NCSCONTEXT        sm_tmr_watch_task_hdl;
#endif
#if (NCSSYSM_TMR_STATS_ENABLE == 1)
  NCS_BOOL          tmr_ignore;       /* hide/expose outstanding tmrs          */
#endif

/* ... */

} NCS_SYSMON;


/*****************************************************************************/

#define SYSMON_MEM_WATCH    20
#define SYSMON_BUF_WATCH    21
#define SYSMON_IPC_WATCH    22
#define SYSMON_CPU_WATCH    23
#define SYSMON_SM_MSG       24
#define SYSMON_IPRA_WATCH   25


#define m_MMGR_ALLOC_SM_MEM_WATCH  (SM_MEM_WATCH *)m_NCS_MEM_ALLOC(sizeof(SM_MEM_WATCH), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_TARGSVCS, SYSMON_MEM_WATCH)

#define m_MMGR_ALLOC_SM_BUF_WATCH  (SM_BUF_WATCH *)m_NCS_MEM_ALLOC(sizeof(SM_BUF_WATCH), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_TARGSVCS, SYSMON_BUF_WATCH)

#define m_MMGR_ALLOC_SM_IPC_WATCH  (SM_IPC_WATCH *)m_NCS_MEM_ALLOC(sizeof(SM_IPC_WATCH), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_TARGSVCS, SYSMON_IPC_WATCH)

#define m_MMGR_ALLOC_SM_CPU_WATCH  (SM_CPU_WATCH *)m_NCS_MEM_ALLOC(sizeof(SM_CPU_WATCH), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_TARGSVCS, SYSMON_CPU_WATCH)

#define m_MMGR_ALLOC_SM_MSG        (SM_MSG*)m_NCS_MEM_ALLOC(sizeof(SM_MSG), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_TARGSVCS,SYSMON_SM_MSG)

#define m_MMGR_ALLOC_SM_IPRA_WATCH (SM_IPRA_WATCH *)m_NCS_MEM_ALLOC(sizeof(SM_IPRA_WATCH), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_TARGSVCS, SYSMON_IPRA_WATCH)


#define m_MMGR_FREE_SM_MEM_WATCH(p)    m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                       NCS_SERVICE_ID_TARGSVCS, SYSMON_MEM_WATCH)

#define m_MMGR_FREE_SM_BUF_WATCH(p)    m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                       NCS_SERVICE_ID_TARGSVCS, SYSMON_BUF_WATCH)

#define m_MMGR_FREE_SM_IPC_WATCH(p)    m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                       NCS_SERVICE_ID_TARGSVCS, SYSMON_IPC_WATCH)

#define m_MMGR_FREE_SM_CPU_WATCH(p)    m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                       NCS_SERVICE_ID_TARGSVCS, SYSMON_CPU_WATCH)

#define m_MMGR_FREE_SM_MSG(p)          m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                       NCS_SERVICE_ID_TARGSVCS, SYSMON_SM_MSG)

#define m_MMGR_FREE_SM_IPRA_WATCH(p)   m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                       NCS_SERVICE_ID_TARGSVCS, SYSMON_IPRA_WATCH)

/*****************************************************************************/



#endif
