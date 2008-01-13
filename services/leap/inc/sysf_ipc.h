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

  H&J InterProcess Communication Facility
  -------------------------------------
  Interprocess and Intraprocess Event Posting.  We use this for posting events
  (received pdu's, API requests) at sub-system  boundaries.  It is also used
  for timer expirations, and posting of events within subsystems.

******************************************************************************
*/

/** Module Inclusion Control...
 **/
#ifndef SYSF_IPC_H
#define SYSF_IPC_H

#include "ncssysf_ipc.h"
#include "ncssysfpool.h"

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            H&J Internal Messaging Facility Definitions

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/**
 **
 ** Posting Events or IPC Messages.
 **
 ** The set of macros provided here can be modified at integration time to 
 ** make use of a Target System's IPC or Queueing facilities already in 
 ** existance.
 **
 **/

#if ((NCSSYSM_IPC_WATCH_ENABLE == 1) || (NCSSYSM_IPC_STATS_ENABLE == 1))

#include "ncs_sysm.h"
#include "sysfsysm.h"


typedef struct ipc_rpt_qlat
{
    uns32   start_time;
    uns32   start_depth;
    uns32   start_percentile;
    uns32   finish_time;
    uns32   finish_depth;
    uns32   finish_percentile;
    NCSIPC_LTCY_EVT callback;
}IPC_RPT_QLAT;

extern struct ncs_sysmon gl_sysmon;

struct tag_ncs_ipc;

typedef struct ncs_ipc_stats
{
    struct ncs_ipc_stats* next;
    struct tag_ncs_ipc*   resource;           /* Back pointer to the IPC struct  */  
    uns16                resource_id;        /* Identifier internally generated */
    uns32                curr_depth;         /* CurrentQ Depth                  */
    uns32                last_depth;         /* Last Q Depth                    */
    uns32                max_depth;          /* reference to MAX Q depth allowed*/
    uns32                last_bkt;           /* Last watch bucket               */

    uns32                hwm;                /* Max Depth reached               */
    uns32                enqed;              /* Enqueued msgs so far            */
    uns32                deqed;              /* Dequeued msgs so far            */

#if (NCSSYSM_IPC_REPORT_ACTIVITY == 1)
    struct ipc_rpt_qlat  ipc_qlat;
#endif

} NCS_IPC_STATS;


#define m_NCS_SM_IPC_ELEM_ADD(a)           ncssm_ipc_elem_add(&a->stats,a)

#define m_NCS_SM_IPC_ELEM_DEL(a)           ncssm_ipc_elem_del(&a->stats)

#define m_NCS_SM_IPC_ELEM_CUR_DEPTH_INC(a) \
{if(a->stats.curr_depth++ >= a->stats.hwm) \
{a->stats.hwm = a->stats.curr_depth;} \
  a->stats.enqed++; \
if((a->stats.curr_depth >= a->stats.max_depth) && (gl_sysmon.res_lmt_cb != NULL)) \
{NCSSYSM_RES_LMT_EVT evt; \
  evt.i_vrtr_id = gl_sysmon.vrtr_id; \
  evt.i_obj_id = NCSSYSM_OID_IPC_TTL; \
  evt.i_ttl = a->stats.max_depth;\
  (*gl_sysmon.res_lmt_cb)(&evt);}}

#define m_NCS_SM_IPC_ELEM_CUR_DEPTH_DEC(a) {a->stats.curr_depth--;a->stats.deqed++;}

void ncssm_ipc_elem_add(struct ncs_ipc_stats* stats, struct tag_ncs_ipc* ipc);
void ncssm_ipc_elem_del(struct ncs_ipc_stats* stats);

#else  

#define m_NCS_SM_IPC_ELEM_ADD(a)      
#define m_NCS_SM_IPC_ELEM_DEL(a)          
#define m_NCS_SM_IPC_ELEM_CUR_DEPTH_INC(a) 
#define m_NCS_SM_IPC_ELEM_CUR_DEPTH_DEC(a) 

#endif  


typedef struct ncs_ipc_queue
{
   NCS_IPC_MSG *head;
   NCS_IPC_MSG *tail;
} NCS_IPC_QUEUE;


typedef struct tag_ncs_ipc
{
    NCS_LOCK      queue_lock;    /* to protect access to queue */

    NCS_IPC_QUEUE queue[NCS_IPC_PRIO_LEVELS];      /* element 0 for queueing HIGH priority IPC messages */
                                /* element 1 for queueing NORMAL priority IPC messages */
                                /* element 2 for queueing LOW priority IPC messages */

   /* IR00084436 */
   uns32  no_of_msgs [NCS_IPC_PRIO_LEVELS]; /* (priority level message count, used to compare 
                                              with the corresponding threshold value) */

   uns32  max_no_of_msgs [NCS_IPC_PRIO_LEVELS]; /* (threshold value configured through 
                                                m_NCS_IPC_CONFIG_MAX_MSGS otherwise initialized to zero) */
   uns32  *usr_counters[NCS_IPC_PRIO_LEVELS]; /* user given 32bit counters are accessed through these pointers */

    unsigned int active_queue;  /* the next queue to check for an IPC message */

    /* Changes to migrate to SAF model:PM:7/Jan/03. */
    NCS_SEL_OBJ   sel_obj;   
    /* msg_count: Keeps a count of messages queued. When this count is 
                  non-zero, "sel_obj" will have an "indication" raised against
                  it. The "indication" on "sel_obj" will be removed when
                  the count goes down to zero.

                  This way there need not be an indication raised for every
                  message. An indication is raised only per "burst of 
                  messages"
    */
    uns32         msg_count; 

    /* If "sel_obj" is put to use, the "sem_handle" member will be removed. 
    For now it stays */
    void        *sem_handle;    /* for blocking/waking IPC msg receiver */
    NCS_BOOL      releasing;     /* flag from ncs_ipc_release to ncs_ipc_recv */
    uns32        ref_count;     /* reference count - number of instances attached
                                 * to this IPC */
    char*        name;          /* mbx task name */

#if ((NCSSYSM_IPC_WATCH_ENABLE == 1) || (NCSSYSM_IPC_STATS_ENABLE == 1))
    NCS_IPC_STATS stats;
#endif

} NCS_IPC;




#if (NCS_USE_SYSMON == 1) && (NCSSYSM_IPC_REPORT_ACTIVITY == 1)

LEAPDLL_API uns32 ncssysm_lm_op_ipc_lbgn(NCSSYSM_IPC_RPT_LBGN *info);
LEAPDLL_API uns32 ncssysm_lm_op_ipc_ltcy(NCSSYSM_IPC_RPT_LTCY *info);

#define m_NCS_SYSM_LM_OP_IPC_LBGN(info)  ncssysm_lm_op_ipc_lbgn(info)
#define m_NCS_SYSM_LM_OP_IPC_LTCY(info)  ncssysm_lm_op_ipc_ltcy(info)

LEAPDLL_API uns32 ipc_get_queue_depth(char * name);

#define m_NCS_SET_ST_QLAT()  gl_sysmon.ipc_stats->ipc_qlat.start_depth = ipc_get_queue_depth(gl_sysmon.ipc_stats->resource->name);  \
                            gl_sysmon.ipc_stats->ipc_qlat.start_time  = m_NCS_GET_TIME_MS;                    

#define m_NCS_SET_FN_QLAT()   gl_sysmon.ipc_stats->ipc_qlat.finish_depth = ipc_get_queue_depth(gl_sysmon.ipc_stats->resource->name);    \
                             gl_sysmon.ipc_stats->ipc_qlat.finish_time  = m_NCS_GET_TIME_MS;                                       \
                             (gl_sysmon.ipc_stats->ipc_qlat.callback)();

#else /* (NCS_USE_SYSMON == 1) && (NCSSYSM_IPC_REPORT_ACTIVITY == 1) */

#define m_NCS_SYSM_LM_OP_IPC_LBGN(info)   NCSCC_RC_FAILURE
#define m_NCS_SYSM_LM_OP_IPC_LTCY(info)   NCSCC_RC_FAILURE

#define m_NCS_SET_ST_QLAT()
#define m_NCS_SET_FN_QLAT()


#endif /* (NCS_USE_SYSMON == 1) && (NCSSYSM_IPC_REPORT_ACTIVITY == 1) */




#endif /* SYSF_IPC_H */
