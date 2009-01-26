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

  This module contains target system specific declarations related to
  System Timer services.

..............................................................................
*/

#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncs_tasks.h"
#include "ncs_tmr.h"
#include "sysf_def.h"
#include "ncssysf_sem.h"
#include "ncssysf_tmr.h"
#include "ncssysf_tsk.h"
#include "ncspatricia.h"

#ifndef SYSF_TMR_LOG
#define SYSF_TMR_LOG  0
#endif

#if (SYSF_TMR_LOG == 1)
#define m_SYSF_TMR_LOG_ERROR(str,num)   m_NCS_CONS_PRINTF("%s::0x%x\n",(str),(unsigned int)(num))
#define m_SYSF_TMR_LOG_INFO(str,num)   m_NCS_CONS_PRINTF("%s::0x%x\n",(str),(unsigned int)(num))
#else
#define m_SYSF_TMR_LOG_ERROR(str,num)
#define m_SYSF_TMR_LOG_INFO(str,num)   
#endif

#define TIMESPEC_DIFF_IN_NS(later, before)  \
                  (((later).tv_sec - (before).tv_sec)*1000000000LL + \
                  (((later).tv_nsec - (before).tv_nsec)))
#ifndef ENABLE_SYSLOG_TMR_STATS
#define ENABLE_SYSLOG_TMR_STATS 0
#endif

#if (NEWEST_TMR == 1)

#if 0
/* This is the size of the timer tick ring. */
#define SYSF_TMR_RING_SIZE         1024
#endif


/* This is the period of the timer tick we request from the OS. */

#define NCS_MILLISECONDS_PER_TICK    100

/* SYSF_TMR  state of a timer during its lifetime */

#define  TMR_STATE_CREATE          0x01
#define  TMR_STATE_START           0x02
#define  TMR_STATE_DORMANT         0x04
#define  TMR_STATE_DESTROY         0x08

#define  TMR_SET_STATE(t,s)  (t->state=s)
#define  TMR_TEST_STATE(t,s) (t->state&s)

/*         LEGAL TRANSITIONS (for this reference implementation)
 *
 *              +------------+
 *              |            |
 *              v   +---->expired(dormant)----+
 *  create--->start-+                         +-->destroy 
 *              ^   +----> stop(dormant)  ----+
 *              |            |
 *              +------------+
 */

/* SYSF_TMR the thing given to a create/start client */

#if( NCS_TMR_DBG_ENABLE == 1)

/* Timer Debug data points to see who left timers dangling               */

typedef struct sysf_tmr_leak
  {
  uns16               isa;   /* validation marker for SYSF_TMR instances */
  char*               file;  /* File name of CREATE or START operation   */
  uns32               line;  /* File line of CREATE or START operation   */
  uns32               stamp; /* Marks loop count to help determine age   */

  } TMR_DBG_LEAK;

#define TMR_ISA  0x3434 /* existence marker 44 in ASCII */

#define TMR_DBG_TICK(s)           (s.tick++) 
#define TMR_DBG_STAMP(t,v)        (t->dbg.stamp=v)
#define TMR_DBG_SET(d,f,l)        {d.isa=TMR_ISA;d.file=f;d.line=l;}
#define TMR_DBG_ASSERT_ISA(d)     {if(d.isa!=TMR_ISA)m_LEAP_DBG_SINK_VOID(0);}

#else

#define TMR_DBG_LEAK uns32

#define TMR_DBG_TICK(s)
#define TMR_DBG_STAMP(t,v)
#define TMR_DBG_ASSERT_ISA(t)

#endif

#if ( NCS_TMR_DBG_ENABLE == 1)
#define TMR_DBG_ASSERT_STATE(t,s) {if(!(t->state&s)){m_LEAP_DBG_SINK_VOID(0);m_NCS_OS_ASSERT((t->state&s));}}
#else
#define TMR_DBG_ASSERT_STATE(t,s)
#endif 

#if NCSSYSM_TMR_STATS_ENABLE == 1

/* SMM : NOTE some statistics have suspicious answers at this point. I   */
/*       suspect some of the macros are not properly sprinkled now.      */

typedef struct tmr_stats
  {
  uns32               cnt;        /* a place to keep a count */
  uns32               ring_hwm;   /* Most svc'ed in one bucket */
  uns32               tmr_hwm;    /* Most timers in svc at once */
  uns32               start_cnt;  /* raw count of started tmrs */
  uns32               expiry_cnt; /* raw count of expired tmrs */
  uns32               stop_cnt;   /* raw count of cancelled tmrs */
  uns32               free_hwm;   /* free tmr objects in pool hwm */
  uns32               free_now;   /* currently free in pool */
  uns32               ttl_active; /* currently active ..do math when asked */
  uns32               ttl_tmrs;   /* timers malloc'ed/in TmrSvc */

  } TMR_STATS;

#define TMR_SET_CNT(s)         s.cnt=0
#define TMR_INC_CNT(s)         s.cnt++
#define TMR_STAT_RING_HWM(s)   {if(s.cnt>s.ring_hwm) s.ring_hwm=s.cnt;}
#define TMR_STAT_TMR_HWM(c,s)  {if(c>s.tmr_hwm)  s.tmr_hwm=c;}
#define TMR_STAT_STARTS(s)     s.start_cnt++
#define TMR_STAT_EXPIRY(s)     s.expiry_cnt++
#define TMR_STAT_CANCELLED(s)  s.stop_cnt++
#define TMR_STAT_FREE_HWM(s)   {if(s.free_now>s.free_hwm) s.free_hwm=s.free_now;}
#define TMR_STAT_ADD_FREE(s)   s.free_now++
#define TMR_STAT_RMV_FREE(s)   s.free_now--
#define TMR_STAT_TTL_TMRS(s)   s.ttl_tmrs++

#else

#if ENABLE_SYSLOG_TMR_STATS
typedef struct tmr_stats
{
  uns32 cnt;
  uns32 ring_hwm;
} TMR_STATS;
#else
#define TMR_STATS  uns32
#endif

#define TMR_SET_CNT(s)
#define TMR_INC_CNT(s)
#define TMR_STAT_RING_HWM(s)
#define TMR_STAT_TMR_HWM(c,s)
#define TMR_STAT_STARTS(s)
#define TMR_STAT_EXPIRY(s)
#define TMR_STAT_CANCELLED(s)
#define TMR_STAT_FREE_HWM(s)
#define TMR_STAT_ADD_FREE(s)
#define TMR_STAT_RMV_FREE(s)
#define TMR_STAT_TTL_TMRS(s)

#endif

/* SYSF_TMR holds expiry info and state                                  */

typedef struct sysf_tmr
{
  struct sysf_tmr    *next; /* Must be first field !!!*/
  struct sysf_tmr    *keep; /* just to know where you are !! */

  uns8                state;
  uns64               key;
  TMR_CALLBACK        tmrCB;
  NCSCONTEXT           tmrUarg;
  TMR_DBG_LEAK        dbg;

} SYSF_TMR;

/* SYSF_TMR_PAT_NODE holds timer list info available in a pat node */
typedef struct sysf_tmr_pat_node
{
 NCS_PATRICIA_NODE   pat_node; 
 uns64               key; 
 SYSF_TMR            *tmr_list_start;
 SYSF_TMR            *tmr_list_end;
} SYSF_TMR_PAT_NODE;


/* TMR_SAFE the part of the timer svc within the critical region        */

typedef struct tmr_safe
{
  NCS_LOCK            enter_lock;          /* protect list of new timers */
  NCS_LOCK            free_lock;     /* protect list of free pool timers */
  SYSF_TMR           dmy_free;
  SYSF_TMR           dmy_keep;

} TMR_SAFE;

/* SYSF_TMR_CB the master structure for the timer service */

typedef struct sysf_tmr_cb
{
  uns32               tick;       /* Times TmrExpiry has been called     */

  NCSLPG_OBJ           persist;     /* guard against fleeting destruction */
  TMR_SAFE            safe;        /* critical region stuff              */
  NCS_PATRICIA_TREE   tmr_pat_tree;
  TMR_STATS           stats;
  void*               p_tsk_hdl;   /* expiry task handle storage         */
  NCS_SEL_OBJ         sel_obj;
  uns32               msg_count;

} SYSF_TMR_CB;

/* gl_tcb  ...  The global instance of the SYSF_TMR_CB                   */


void ncs_tmr_signal(void *uarg);
static SYSF_TMR_CB gl_tcb     = { 0 };
static NCS_BOOL tmr_destroying = FALSE;
static NCS_SEL_OBJ tmr_destroy_syn_obj;

static uns32
ncs_tmr_add_pat_node(SYSF_TMR*  tmr)
{
    SYSF_TMR_PAT_NODE *temp_tmr_pat_node = NULL;
   
    temp_tmr_pat_node  =(SYSF_TMR_PAT_NODE *)ncs_patricia_tree_get
                (&gl_tcb.tmr_pat_tree,(uns8 *)&tmr->key);

    if(temp_tmr_pat_node  == (SYSF_TMR_PAT_NODE *)NULL)
    {    
        temp_tmr_pat_node = (SYSF_TMR_PAT_NODE*) m_NCS_MEM_ALLOC(sizeof(SYSF_TMR_PAT_NODE),
                                                             NCS_MEM_REGION_PERSISTENT,
                                                             NCS_SERVICE_ID_LEAP_TMR,0);         
        m_NCS_MEMSET (temp_tmr_pat_node, '\0', sizeof (SYSF_TMR_PAT_NODE));
        temp_tmr_pat_node->key = tmr->key;
        temp_tmr_pat_node->pat_node.key_info = (uns8*)&temp_tmr_pat_node->key;   
        ncs_patricia_tree_add (&gl_tcb.tmr_pat_tree,
                          (NCS_PATRICIA_NODE *)&temp_tmr_pat_node->pat_node);
    }
    
    if(temp_tmr_pat_node->tmr_list_start == NULL)  
    {
        temp_tmr_pat_node->tmr_list_end = temp_tmr_pat_node->tmr_list_start = tmr;
    }
    else
    {
        temp_tmr_pat_node->tmr_list_end->next = tmr;
        temp_tmr_pat_node->tmr_list_end = tmr;    
    }

    return NCSCC_RC_SUCCESS;
}

/* This routine returns the time elapsed in units of NCS_MILLISECONDS_PER_TICK */
static uns64
get_time_elapsed_in_ticks(struct timespec *temp_ts_start)
{
  uns64 time_elapsed = 0;
  struct timespec ts_current = {0,0};

  if(clock_gettime (CLOCK_MONOTONIC, &ts_current))
   {
     perror("clock_gettime with MONOTONIC Failed \n");
   }
  time_elapsed = ((((ts_current.tv_sec - temp_ts_start->tv_sec)*(1000LL))+
                             ((ts_current.tv_nsec - temp_ts_start->tv_nsec)/1000000))/NCS_MILLISECONDS_PER_TICK);

  return time_elapsed;
}

 /****************************************************************************
  * Function Name: sysfTmrExpiry
  *
  * Purpose: Service all timers that live in this expiry bucket
  *
  ****************************************************************************/

static NCS_BOOL
sysfTmrExpiry (SYSF_TMR_PAT_NODE *tmp )
{
  SYSF_TMR*        now_tmr;
  SYSF_TMR         dead_inst;
  SYSF_TMR*        dead_tmr      = &dead_inst;
  SYSF_TMR*        start_dead    = &dead_inst;

  /* get these guys one behind to start as well */
  dead_tmr->next    = NULL;
  
  /* Confirm and secure tmr service is/will persist */
  if (ncslpg_take(&gl_tcb.persist) == FALSE)
    return FALSE; /* going or gone away.. Lets leave */
  
  /* IR00082954 */
  if(tmr_destroying == TRUE)
  {
    /* Raise An indication */
    m_NCS_SEL_OBJ_IND(tmr_destroy_syn_obj);

    /*If thread canceled here, It had no effect on timer thread destroy */
    ncslpg_give(&gl_tcb.persist,0);

    /* returns TRUE if thread is going to be  destroyed otherwise return FALSE(normal flow) */
    return TRUE;
  }
  
  TMR_DBG_TICK(gl_tcb);

  dead_tmr->next = tmp->tmr_list_start;

  TMR_SET_CNT(gl_tcb.stats);

  while(dead_tmr->next != NULL)/* process old and new */
  {
      now_tmr = dead_tmr->next;
      TMR_INC_CNT(gl_tcb.stats);      
      /* SMM states CREATE, EXPIRED, illegal assert */
      
      if ((TMR_TEST_STATE(now_tmr,TMR_STATE_DORMANT)) ||
          (TMR_TEST_STATE(now_tmr,TMR_STATE_DESTROY))    )
      {
        TMR_STAT_CANCELLED(gl_tcb.stats);
        TMR_STAT_ADD_FREE(gl_tcb.stats);
        TMR_STAT_FREE_HWM(gl_tcb.stats);
        TMR_DBG_STAMP(now_tmr,gl_tcb.tick);

        dead_tmr = now_tmr;                          /* move on to next one */
      }
      else
      {
#if 0   /* See IR00058320: For details */ 
        TMR_DBG_ASSERT_STATE(now_tmr,TMR_STATE_START);
#endif
        TMR_SET_STATE(now_tmr,TMR_STATE_DORMANT);     /* mark it dormant */
        TMR_STAT_EXPIRY(gl_tcb.stats);
        TMR_STAT_ADD_FREE(gl_tcb.stats);
        TMR_STAT_FREE_HWM(gl_tcb.stats);
        TMR_DBG_STAMP(now_tmr,gl_tcb.tick);

        /* EXPIRY HAPPENS RIGHT HERE !!..................................*/      
        if (now_tmr->tmrCB != ((TMR_CALLBACK)0x0ffffff))
        {
#if ENABLE_SYSLOG_TMR_STATS
           gl_tcb.stats.cnt--;
           if(gl_tcb.stats.cnt == 0)
           {
              m_NCS_SYSLOG(NCS_LOG_INFO,"NO Timers Active in Expiry PID %u \n",m_NCS_OS_PROCESS_GET_ID());
           }
#endif
           now_tmr->tmrCB (now_tmr->tmrUarg);    /* OK this is it! Expire ! */
        }

        dead_tmr = now_tmr;                       /* move on to next one */
      }
  }
  
  TMR_STAT_RING_HWM(gl_tcb.stats);
    
  /* Now replenish the free pool */

  if (start_dead->next != NULL)
  {
    m_NCS_LOCK (&gl_tcb.safe.free_lock, NCS_LOCK_WRITE); /* critical region START */
    dead_tmr->next   = gl_tcb.safe.dmy_free.next;  /* append old free list to end of that  */
    gl_tcb.safe.dmy_free.next = start_dead->next;  /* put start of collected dead in front */
    m_NCS_UNLOCK (&gl_tcb.safe.free_lock, NCS_LOCK_WRITE);  /* critical region END */ 
  }

  ncs_patricia_tree_del(&gl_tcb.tmr_pat_tree, (NCS_PATRICIA_NODE *)tmp);
  m_NCS_MEM_FREE(tmp,NCS_MEM_REGION_PERSISTENT,NCS_SERVICE_ID_LEAP_TMR,0);
 
  ncslpg_give(&gl_tcb.persist,0);
  return FALSE;
}

 /****************************************************************************
  * Function Name: ncs_tmr_signal
  *
  * Purpose: signal the ncs_tmr_wait function by wacking the shared semaphore.
  *
  ****************************************************************************/
void
ncs_tmr_signal(void *uarg)
{
  USE(uarg);

  if (ncslpg_take(&gl_tcb.persist) == FALSE) /* ensure the tmr svc is still open */
    return;                                 /* going or gone away.. Lets leave  */
#if 0
  m_NCS_SEM_GIVE(gl_tcb.p_sem_hdl);          /* wack the shared semaphore        */
#endif

  ncslpg_give(&gl_tcb.persist,0);
}


static  struct timespec ts_start;


static uns32 ncs_tmr_select_intr_process(struct timeval *tv,struct 
                                          timespec *ts_current,uns64 next_delay)
{
   uns64 tmr_restart = 0;
   uns64 time_left = 0;
   struct timespec ts_curr = *ts_current;
   struct timespec ts_eint = {0,0};

   tv->tv_sec =  tv->tv_usec = 0;

   if(next_delay == 0)
   {
      tv->tv_sec = 0xffffff;
      tv->tv_usec = 0;
      return NCSCC_RC_SUCCESS;
    }

    if(clock_gettime (CLOCK_MONOTONIC, &ts_eint))
    {
        perror("clock_gettime with MONOTONIC Failed \n");
        return NCSCC_RC_FAILURE;
    }
    else
    {
         tmr_restart = TIMESPEC_DIFF_IN_NS(ts_eint,ts_curr);
         time_left = ((next_delay * 1000000LL * NCS_MILLISECONDS_PER_TICK) - (tmr_restart));
         if(time_left >0)
         {
              tv->tv_sec = time_left/1000000000LL;
              tv->tv_usec = ((time_left%1000000000LL) /1000);
         }
     }
    
    return NCSCC_RC_SUCCESS; 
}

static uns32
ncs_tmr_engine(struct timeval *tv,uns64 *next_delay)
{
   uns64 next_expiry = 0;
   uns64 ticks_elapsed = 0;
   SYSF_TMR_PAT_NODE *tmp = NULL;

#if ENABLE_SYSLOG_TMR_STATS
   /* To measure avg. timer expiry gap */
   struct timespec tmr_exp_prev_finish = {0,0};
   struct timespec tmr_exp_curr_start = {0,0};
   uns32           tot_tmr_exp = 0; 
   uns64           sum_of_tmr_exp_gaps = 0; /* Avg = sum_of_tmr_exp_gaps/tot_tmr_exp*/
#endif

      while(TRUE)
      {
          tmp = (SYSF_TMR_PAT_NODE *)ncs_patricia_tree_getnext(&gl_tcb.tmr_pat_tree,
                                               (uns8*)NULL);
          ticks_elapsed  = get_time_elapsed_in_ticks(&ts_start);
          if(tmp != NULL)
          {
              next_expiry = m_NCS_OS_NTOHLL_P(&tmp->key);  
          }
          else
          {
               tv->tv_sec = 0xffffff;
               tv->tv_usec = 0;
               *next_delay = 0;
               return NCSCC_RC_SUCCESS;
          }

          if(ticks_elapsed >= next_expiry)
          {
#if ENABLE_SYSLOG_TMR_STATS
              if(tot_tmr_exp != 0) 
              {
                  tmr_exp_curr_start.tv_sec = tmr_exp_curr_start.tv_nsec = 0;
                  if (clock_gettime (CLOCK_MONOTONIC, &tmr_exp_curr_start))
                  {
                     perror("clock_gettime with MONOTONIC Failed \n");
                  }
                  sum_of_tmr_exp_gaps += TIMESPEC_DIFF_IN_NS(tmr_exp_curr_start, tmr_exp_prev_finish);
                  if (tot_tmr_exp >= 100)
                  {
                      m_NCS_CONS_PRINTF("\nTotal active timers: %d\n",gl_tcb.stats.cnt);
                      m_NCS_CONS_PRINTF("Average timer-expiry gap (last %d expiries) = %lld\n",
                                                  tot_tmr_exp, sum_of_tmr_exp_gaps/tot_tmr_exp);
                      tot_tmr_exp = 0;
                      sum_of_tmr_exp_gaps = 0;
                  }
              }
#endif

              if(TRUE  == sysfTmrExpiry(tmp))                   /* call expiry routine          */
              {
                  return NCSCC_RC_FAILURE;
              }

#if ENABLE_SYSLOG_TMR_STATS
              tot_tmr_exp++;
              tmr_exp_prev_finish.tv_sec = tmr_exp_prev_finish.tv_nsec = 0;
              if(clock_gettime (CLOCK_MONOTONIC, &tmr_exp_prev_finish))
              {
                   perror("clock_gettime with MONOTONIC Failed \n");
                   return NCSCC_RC_FAILURE;
              }
#endif
          }
          else
          { 
              break;
          }
      } /* while loop end */

      (*next_delay) = next_expiry - ticks_elapsed;

      /* Convert the next_dealy intto the below structure */
      tv->tv_sec = ((*next_delay)*NCS_MILLISECONDS_PER_TICK)/1000;
      tv->tv_usec = (((*next_delay)%(1000/NCS_MILLISECONDS_PER_TICK)) * (1000 * NCS_MILLISECONDS_PER_TICK)); 
      
      return NCSCC_RC_SUCCESS;
}


 /****************************************************************************
  * Function Name: ncs_tmr_wait
  *
  * Purpose: Goto counting semephore and block until tmr_pulse is felt. This
  *          means the actual expiry task no longer services the sysfTmrExpiry
  *          function any more. This should eliminate timer drift.
  *
  ****************************************************************************/

static uns32
ncs_tmr_wait(void)
{

   int rc = 0;
   int inds_rmvd;
   int save_errno = 0;   

   uns64 next_delay = 0;

   NCS_SEL_OBJ         mbx_fd = gl_tcb.sel_obj;
   NCS_SEL_OBJ         highest_sel_obj;
   NCS_SEL_OBJ_SET     all_sel_obj;
   struct timeval tv = {0xffffff,0};
   struct timespec ts_current = {0,0};

   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   highest_sel_obj = mbx_fd;
 
   if(clock_gettime (CLOCK_MONOTONIC, &ts_start))
   {
     perror("clock_gettime with MONOTONIC Failed \n");  
     return NCSCC_RC_FAILURE;
   }

   ts_current = ts_start;
                                       
   while(TRUE)
   {
   select_sleep:
      m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);
      rc =  select(highest_sel_obj.rmv_obj+1, &all_sel_obj, NULL, NULL, &tv);
      save_errno = errno;
      m_NCS_LOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);
         
      if(rc < 0) 
      {
          if(save_errno != EINTR)
              m_NCS_OS_ASSERT(0);  

          if( ncs_tmr_select_intr_process(&tv,&ts_current,next_delay) == NCSCC_RC_SUCCESS)
          {
              m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);           
              goto select_sleep;
          }
          else
          {
              m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);           
              return NCSCC_RC_FAILURE;
          }
      }   
      else if(rc == 1)
      { 
        /* if select returned because of indication on sel_obj from sysfTmrDestroy */
        if(tmr_destroying == TRUE)
        {
             /* Raise An indication */
             m_NCS_SEL_OBJ_IND(tmr_destroy_syn_obj);
             m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);           
             return NCSCC_RC_SUCCESS;
        }

         gl_tcb.msg_count--;

         if (gl_tcb.msg_count == 0)
         {
               inds_rmvd = m_NCS_SEL_OBJ_RMV_IND(gl_tcb.sel_obj, TRUE, TRUE);
               if ( inds_rmvd <= 0)
               {
                     if (inds_rmvd != -1)
                     {
                       /* The object has not been destroyed and it has no indication
                       raised on it inspite of msg_count being non-zero.
                        */
                       m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);
                       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
                     }

                     m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);

                    /* The mbox must have been destroyed */
                     return NCSCC_RC_FAILURE;
               }
         }
      } 

      rc = ncs_tmr_engine(&tv,&next_delay);
      if(rc == NCSCC_RC_FAILURE)
      {
         m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);           
         return NCSCC_RC_FAILURE;
      }
      
      m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);           
      ts_current.tv_sec = ts_current.tv_nsec = 0;

      if(clock_gettime (CLOCK_MONOTONIC, &ts_current))
      {
          perror("clock_gettime with MONOTONIC Failed \n");  
          return NCSCC_RC_FAILURE;
      }
   }

   return NCSCC_RC_SUCCESS;
}

 /****************************************************************************
  * Function Name: sysfTmrCreate 
  *
  * Purpose: Create the timer service.Get all resources in a known state.
  *
  ****************************************************************************/

static NCS_BOOL ncs_tmr_create_done = FALSE;

NCS_BOOL
sysfTmrCreate(void)
{     
   NCS_PATRICIA_PARAMS pat_param;
   uns32 rc = NCSCC_RC_SUCCESS;

    /* IR00060372 */
   if (ncs_tmr_create_done == FALSE)
       ncs_tmr_create_done  = TRUE;
   else
       return TRUE;

   /* Empty Timer Service control block.*/
   m_NCS_MEMSET (&gl_tcb, '\0', sizeof (SYSF_TMR_CB));

   /* put local persistent guard in start state */
   ncslpg_create(&gl_tcb.persist);
   
   /* Initialize the locks */
   m_NCS_LOCK_INIT(&gl_tcb.safe.enter_lock);
   m_NCS_LOCK_INIT(&gl_tcb.safe.free_lock);

   /* create semaphore for expiry thread */
 #if 0 
   m_NCS_SEM_CREATE(&gl_tcb.p_sem_hdl);
 #endif
   
   m_NCS_OS_MEMSET((void *) &pat_param, 0, sizeof(NCS_PATRICIA_PARAMS));

   pat_param.key_size = sizeof(uns64);

   rc = ncs_patricia_tree_init(&gl_tcb.tmr_pat_tree,&pat_param);
   if( rc != NCSCC_RC_SUCCESS)
   {
      return  NCSCC_RC_FAILURE;
   }

   rc = m_NCS_SEL_OBJ_CREATE(&gl_tcb.sel_obj);
   if(rc != NCSCC_RC_SUCCESS)
   {
      ncs_patricia_tree_destroy(&gl_tcb.tmr_pat_tree);
      return NCSCC_RC_FAILURE;
   }
   tmr_destroying = FALSE;

   /* create expiry thread */

   if (m_NCS_TASK_CREATE ((NCS_OS_CB)ncs_tmr_wait,
                          0,
                          NCS_TMR_TASKNAME,
                          NCS_TMR_PRIORITY,
                          NCS_TMR_STACKSIZE,
                          &gl_tcb.p_tsk_hdl) != NCSCC_RC_SUCCESS)
  {
#if 0
      m_NCS_SEM_RELEASE(gl_tcb.p_sem_hdl);
#endif
     ncs_patricia_tree_destroy(&gl_tcb.tmr_pat_tree);
     m_NCS_SEL_OBJ_DESTROY(gl_tcb.sel_obj);
     return FALSE;
  }

  if (m_NCS_TASK_START (gl_tcb.p_tsk_hdl) != NCSCC_RC_SUCCESS)
  {
      m_NCS_TASK_RELEASE(gl_tcb.p_tsk_hdl);
#if 0
      m_NCS_SEM_RELEASE(gl_tcb.p_sem_hdl);
#endif
     ncs_patricia_tree_destroy(&gl_tcb.tmr_pat_tree);
     m_NCS_SEL_OBJ_DESTROY(gl_tcb.sel_obj);
     return FALSE;
  }

#if ((NCS_MMGR_DEBUG  == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1)) 

   /* Tmr maintains its own memory pool. It will get its memory from  */
   /* sysfpool.c. If MEM DBG stuff is enabled, we do not want TMR mem */
   /* to appear as a LEAK from sysfpool's point of view, so we will   */
   /* permenently ignore/hide all mem allocated by the TMR 'subsystem'*/
  
   ncs_mem_ignore_subsystem(TRUE, NCS_SERVICE_ID_LEAP_TMR);

#endif
   return TRUE;
}
 

 /****************************************************************************
  * Function Name: sysfTmrDestroy
  *
  * Purpose: Disengage timer services. Memory is not given back to HEAP as
  *          it is not clear what the state of outstanding timers is.
  *
  ****************************************************************************/

NCS_BOOL
sysfTmrDestroy(void)
{
   SYSF_TMR*   tmr;
   SYSF_TMR*   free_tmr;
   uns32 timeout = 2000; /* 20seconds */
   SYSF_TMR_PAT_NODE *tmp = NULL;
 

/* There is only ever one timer per instance */
#if 0
   /* Disable local persistent guard; no new clients can come in */   
   if (ncslpg_destroy(&gl_tcb.persist) == NCSCC_RC_FAILURE)
     return FALSE;          /* already 'de-commissioned' */
#endif

  m_NCS_LOCK (&gl_tcb.safe.free_lock, NCS_LOCK_WRITE);

  gl_tcb.safe.dmy_free.next = NULL; 

  m_NCS_UNLOCK (&gl_tcb.safe.free_lock, NCS_LOCK_WRITE);
  
  /* IR00060372 */ 
  m_NCS_LOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);
  

   /* IR00082954 */
  /* Create selection object */
  m_NCS_SEL_OBJ_CREATE(&tmr_destroy_syn_obj);

  tmr_destroying = TRUE;

  m_NCS_SEL_OBJ_IND(gl_tcb.sel_obj);

  /* Unlock the lock */
  m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);  /* critical region END */ 

  /* Wait on Poll object */
  /* IR00082954 */
  m_NCS_SEL_OBJ_POLL_SINGLE_OBJ(tmr_destroy_syn_obj, &timeout);

  m_NCS_LOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);
  tmr = &gl_tcb.safe.dmy_keep;
  while (tmr->keep != NULL)
  {
      free_tmr  = tmr->keep;
      tmr->keep = tmr->keep->keep;
      /* IR00082954*/
      m_NCS_MEM_FREE(free_tmr,NCS_MEM_REGION_PERSISTENT,NCS_SERVICE_ID_LEAP_TMR,0);
  }
  while((tmp = (SYSF_TMR_PAT_NODE *)ncs_patricia_tree_getnext(&gl_tcb.tmr_pat_tree,
                         (uns8 *)0)) != NULL)
   {
          ncs_patricia_tree_del(&gl_tcb.tmr_pat_tree, (NCS_PATRICIA_NODE *)tmp);
          m_NCS_MEM_FREE(tmp,NCS_MEM_REGION_PERSISTENT,NCS_SERVICE_ID_LEAP_TMR,0);
   }
   
   ncs_patricia_tree_destroy(&gl_tcb.tmr_pat_tree); 
   m_NCS_SEL_OBJ_DESTROY(gl_tcb.sel_obj);
 
  /* Now we tell the memmgr to no longer ignore the memory..........*/

#if ((NCS_MMGR_DEBUG  == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1)) 
  
   ncs_mem_ignore_subsystem(FALSE, NCS_SERVICE_ID_LEAP_TMR);

#endif

   /* Stop the dedicated thread that runs out of ncs_tmr_wait() */

   m_NCS_TASK_RELEASE(gl_tcb.p_tsk_hdl);

   /* NOTE: Free-ing the semaphore seems like its dangerous... This  */
   /* implementation does not RELEASE it out of fear, though nothing */
   /* bad might come of it depending on the target OS .....          */

   /*       m_NCS_SEM_RELEASE(gl_tcb.p_sem_hdl);                      */
#if 0 
   /* IR00060372 */
   m_NCS_SEM_RELEASE(gl_tcb.p_sem_hdl);
#endif

   tmr_destroying = FALSE;

   m_NCS_SEL_OBJ_DESTROY(tmr_destroy_syn_obj);

   m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);  /* critical region END */ 

   /* don't destroy the lock (but remember that you could!).
   * m_NCS_LOCK_DESTROY (&l_tcb.lock); 
   */

    /* IR00060372 */
    m_NCS_LOCK_DESTROY(&gl_tcb.safe.enter_lock);

    m_NCS_LOCK_DESTROY(&gl_tcb.safe.free_lock);

    /* IR00060372 */
    ncs_tmr_create_done = FALSE; 
    
    return TRUE;
}


/*****************************************************************************
 *
 *                  Timer Add, Stop, and Expiration Handler
 *
 *****************************************************************************/

 /****************************************************************************
  * Function Name: sysfTmrAlloc
  *
  * Purpose: Either fetch an existing Tmr block or get one off the HEAP
  *
  ****************************************************************************/

tmr_t
ncs_tmr_alloc (char* file, uns32  line)
{
  SYSF_TMR*  tmr;
  SYSF_TMR*  back;

   /* IR00082954 */
  if (tmr_destroying == TRUE)
    return NULL;   
     
  if (ncslpg_take(&gl_tcb.persist) == FALSE) /* guarentee persistence */
    return NULL;
  
  m_NCS_LOCK (&gl_tcb.safe.free_lock, NCS_LOCK_WRITE);
   
  back = &gl_tcb.safe.dmy_free;        /* see if we have a free one */
  tmr  = back->next;

  while (tmr != NULL)
  {
#if 0   /* See IR00058320: For details */ 
    TMR_DBG_ASSERT_STATE(tmr,(TMR_STATE_DORMANT|TMR_STATE_DESTROY));
#endif

    if (TMR_TEST_STATE(tmr,TMR_STATE_DESTROY))
    {
      TMR_STAT_RMV_FREE(gl_tcb.stats);
      back->next = tmr->next; /* and 'tmr' is our answer */
      break;
    }
    else
    {
      back = tmr;
      tmr  = tmr->next;
    }
  }

  if (tmr == NULL)
  {
    tmr = (SYSF_TMR*) m_NCS_MEM_ALLOC(sizeof(SYSF_TMR), 
                                     NCS_MEM_REGION_PERSISTENT,
                                     NCS_SERVICE_ID_LEAP_TMR,0);
    m_NCS_MEMSET (tmr, '\0', sizeof (SYSF_TMR));
    if (tmr == NULL)
      m_LEAP_DBG_SINK_VOID(0);                    /* can't allocate memory?? */
    else
    {
      TMR_STAT_TTL_TMRS(gl_tcb.stats);
      /* IR00060372 DEBUG enable flag removed */
      tmr->keep = gl_tcb.safe.dmy_keep.keep;     /* put it on keep list */
      gl_tcb.safe.dmy_keep.keep = tmr;
    }
  }
  
  m_NCS_UNLOCK (&gl_tcb.safe.free_lock, NCS_LOCK_WRITE);

  if (tmr != NULL)
  {
    tmr->next = NULL;                         /* put it in start state */
    TMR_SET_STATE(tmr,TMR_STATE_CREATE);

#if (NCS_TMR_DBG_ENABLE == 1)
    TMR_DBG_SET(tmr->dbg,file,line);
#endif

#if ((NCS_MMGR_DEBUG == 1) && (NCS_TMR_DBG_ENABLE == 1))
      ncs_mem_dbg_loc(tmr,line,file);    /* put owner in sysfpool record */
#endif
  }

  ncslpg_give(&gl_tcb.persist,0);
  return (tmr_t) tmr;
}

 /****************************************************************************
  * Function Name: sysfTmrStart
  *
  * Purpose: Take the passed Timer traits and engage/register with TmrSvc
  *
  ****************************************************************************/
tmr_t
ncs_tmr_start (tmr_t        tid,
              uns32        tmrDelay, /* timer period in number of 10ms units */
              TMR_CALLBACK tmrCB,
              void        *tmrUarg,
              char        *file,
              uns32        line)
{
  SYSF_TMR*    tmr;
  SYSF_TMR*    new_tmr;
  uns64        scaled;
  uns64        temp_key_value;
  uns32 rc = NCSCC_RC_SUCCESS;
#if 0
  /* used to add extra tick to m */
  uns32 extra_tick = 0;
#endif
  

  if (((tmr = (SYSF_TMR*)tid) == NULL)||(tmr_destroying == TRUE))      /* NULL tmrs are no good! */
    return NULL;

 TMR_DBG_ASSERT_ISA(tmr->dbg);              /* confirm that its TMR memory */

 TMR_DBG_ASSERT_STATE(tmr,(TMR_STATE_DORMANT|TMR_STATE_CREATE));

 if (ncslpg_take(&gl_tcb.persist) == FALSE) /* guarentee persistence */
 return NULL; 

 if (TMR_TEST_STATE(tmr,TMR_STATE_DORMANT)) /* If client is re-using timer */
 {
    m_NCS_TMR_CREATE(new_tmr,tmrDelay,tmrCB,tmrUarg);    /* get a new one */
    if (new_tmr == NULL)      
      {
      ncslpg_give(&gl_tcb.persist, 0);  
      return NULL;
      }

    TMR_SET_STATE(tmr,TMR_STATE_DESTROY); /* TmrSvc ignores 'old' one */
    tmr = new_tmr;
 }
#if 0
  extra_tick = ((tmrDelay * 10)%NCS_MILLISECONDS_PER_TICK)?1:0;
  scaled = (tmrDelay*10/NCS_MILLISECONDS_PER_TICK)+ extra_tick + (get_time_elapsed_in_ticks(&ts_start));
#endif
  scaled = (tmrDelay*10/NCS_MILLISECONDS_PER_TICK)+1+(get_time_elapsed_in_ticks(&ts_start));
  
  /* Do some up front initialization as if all will go well */
  tmr->tmrCB    = tmrCB;
  tmr->tmrUarg  = tmrUarg;
  TMR_SET_STATE(tmr,TMR_STATE_START);
  
  /* Lock the enter wheel in the safe area */
  m_NCS_LOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);
  
  tmr->next = NULL;
  m_NCS_OS_HTONLL_P(&temp_key_value,scaled);
  tmr->key = temp_key_value;

  rc = ncs_tmr_add_pat_node(tmr);
  if(rc == NCSCC_RC_FAILURE)
  {
    /* Free the timer created */
    m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);
    return NULL;
  }

#if ENABLE_SYSLOG_TMR_STATS
  gl_tcb.stats.cnt++;
  if (gl_tcb.stats.cnt == 1)
  {
     m_NCS_SYSLOG(NCS_LOG_INFO,"At least one timer started\n");
  }
#endif
  if (gl_tcb.msg_count == 0)
  {
      /* There are no messages queued, we shall raise an indication
      on the "sel_obj".  */
      if (m_NCS_SEL_OBJ_IND(gl_tcb.sel_obj) != NCSCC_RC_SUCCESS)
      {
         /* We would never reach here! */
         m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);
         m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         return NULL;
       }
   }
   gl_tcb.msg_count++;

/*
  1)Take Lock
  2)Caliculate the key value based on (A+1)+T
           T is Timer ticks elapsed  and A is ticks 
   ncs_tmr_add_pat_node(root_node,tmr,(A+1)+T,)
  3)See whether patricia Node is already present in patricia Tree with the 
    caliculated Key 
    If not present 
      Create patricia Node and add it the patricia tree 
    If present
      Add the tmr data structure to the double linked list as FIFO.
      i.e adds at the last of the double linked list.
  4) Raise an indication on the selection object.
     if (write(i_ind_obj.raise_obj, "A", 1) != 1)
        return NCSCC_RC_FAILURE;
*/

  TMR_STAT_STARTS(gl_tcb.stats);
  
  m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE);

#if (NCS_TMR_DBG_ENABLE == 1)
  TMR_DBG_SET(tmr->dbg,file,line);
#endif

  ncslpg_give(&gl_tcb.persist, 0);  

  return tmr;
}


 /****************************************************************************
  * Function Name: ncs_tmr_stop_v2
  *
  * Purpose:   Mark this timer as DORMANT  
  *            if timer is already in DORMANT state returns 
  *            NCSCC_RC_TMR_STOPPED
  *
  * Arguments:
  *               tmrID     :    tmr id
  *               **tmr_arg :    void double pointer  
  * Return values:
  *     NCSCC_RC_FAILURE    :    Any validations fails (or)
                                 If this timer is in CREATE or DESTROY state
  *     NCSCC_RC_SUCCESS    :    This timer START state and is set to DORMANT state 
  *     NCSCC_RC_TMR_STOPPED:    If the timer is already in DORMANT state 
  *
  ****************************************************************************/
uns32
ncs_tmr_stop_v2(tmr_t tmrID,void  **o_tmr_arg)
{
  SYSF_TMR  *tmr;
 
  if (((tmr = (SYSF_TMR*)tmrID) == NULL)|| (tmr_destroying == TRUE) || (o_tmr_arg == NULL))
    return NCSCC_RC_FAILURE;
 
#if 0   /* See IR00058320: For details */
     TMR_DBG_ASSERT_STATE(tmr,(TMR_STATE_CREATE|TMR_STATE_START));
#endif

  m_NCS_LOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE); /* critical region START */
 
  /* Test tmr is already expired */
  if(TMR_TEST_STATE(tmr,TMR_STATE_DORMANT))
  {
      m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE); /* critical region END */
      return NCSCC_RC_TMR_STOPPED;
  }else
  if(TMR_TEST_STATE(tmr,TMR_STATE_START))
  {
#if ENABLE_SYSLOG_TMR_STATS
     gl_tcb.stats.cnt--;
     if(gl_tcb.stats.cnt == 0)
     {         
        m_NCS_SYSLOG(NCS_LOG_INFO,"NO Timers Active STOP_V2 PID %u \n",m_NCS_OS_PROCESS_GET_ID()); 
     }
#endif
        /* set tmr to DORMANT state */
      TMR_SET_STATE(tmr,TMR_STATE_DORMANT);
  }else
  {
     m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE); /* critical region END */
     return NCSCC_RC_FAILURE;
  }

  *o_tmr_arg =tmr->tmrUarg;
 
  m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE); /* critical region END */
 
  return NCSCC_RC_SUCCESS;
}


 /****************************************************************************
  * Function Name: ncs_tmr_stop
  *
  * Purpose:   Mark this timer as DORMANT.
  *
  ****************************************************************************/

void
ncs_tmr_stop (tmr_t tmrID)
{
  SYSF_TMR  *tmr;
  
  if (((tmr = (SYSF_TMR*)tmrID) == NULL) || (tmr_destroying == TRUE))
    return;

  TMR_DBG_ASSERT_ISA(tmr->dbg);              /* confirm that its TMR memory */

#if 0   /* See IR00058320: For details */ 
  TMR_DBG_ASSERT_STATE(tmr,(TMR_STATE_CREATE|TMR_STATE_START)); 
#endif
#if ENABLE_SYSLOG_TMR_STATS
  if(!TMR_TEST_STATE(tmr,TMR_STATE_DORMANT))
  {
     gl_tcb.stats.cnt--;
     if(gl_tcb.stats.cnt == 0)
     {
        m_NCS_SYSLOG(NCS_LOG_INFO,"NO Timers Active STOP PID %u \n",m_NCS_OS_PROCESS_GET_ID()); 
     }
  }
#endif

  TMR_SET_STATE(tmr,TMR_STATE_DORMANT);
}

 /****************************************************************************
  * Function Name: ncs_tmr_free
  *
  * Purpose: Mark this timer as DESTOYED.
  *
  ****************************************************************************/

void
ncs_tmr_free (tmr_t tmrID)
{
  SYSF_TMR  *tmr;

  if (((tmr = (SYSF_TMR*)tmrID) == NULL)|| (tmr_destroying == TRUE))
    return;

  TMR_DBG_ASSERT_ISA(tmr->dbg);               /* confirm that its timer memory */

#if 0   /* See IR00058320: For details */ 
  TMR_DBG_ASSERT_STATE(tmr,(TMR_STATE_DORMANT|TMR_STATE_START)); /* legal states */
#endif

  m_NCS_LOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE); /* critical region START */

#if ENABLE_SYSLOG_TMR_STATS
  if((TMR_TEST_STATE(tmr,TMR_STATE_START)))
  {
      gl_tcb.stats.cnt--;
      if(gl_tcb.stats.cnt == 0)
      {
          m_NCS_SYSLOG(NCS_LOG_INFO,"NO Timers Active Destroy PID %s \n", __LINE__);
      }
  }
#endif
  TMR_SET_STATE(tmr,TMR_STATE_DESTROY);

  /* here we can only selectively 0xff out memory fields */
  tmr->tmrCB   = (TMR_CALLBACK)0x0ffffff;
  tmr->tmrUarg = (void*)       0x0ffffff;
  m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE); /* critical region END */
}


 /****************************************************************************
  * Function Name: ncs_tmr_remaining
  *
  * Purpose:   This function calculates how much time is remaining for a
  *            particular timer.
  *
  ****************************************************************************/

uns32
ncs_tmr_remaining(tmr_t tmrID, uns32 * p_tleft)
{
  SYSF_TMR   *tmr;
  uns32       total_ticks_left;
  uns32       ticks_elapsed;
  uns32       ticks_to_expiry;
  
  if (((tmr = (SYSF_TMR*)tmrID) == NULL) ||(tmr_destroying == TRUE) || (p_tleft == NULL))
    return NCSCC_RC_FAILURE;

  *p_tleft = 0;
  TMR_DBG_ASSERT_ISA(tmr->dbg);             /* confirm that its timer memory */
  
  if (ncslpg_take(&gl_tcb.persist) == FALSE) /* guarentee persistence */
    return NCSCC_RC_FAILURE;

  m_NCS_LOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE); /* critical region START */
  if(!TMR_TEST_STATE(tmr,TMR_STATE_START))
  {
    m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE); /* critical region START */
    return NCSCC_RC_FAILURE;
  }
  m_NCS_UNLOCK (&gl_tcb.safe.enter_lock, NCS_LOCK_WRITE); /* critical region START */
  ticks_elapsed = get_time_elapsed_in_ticks(&ts_start); 
  ticks_to_expiry = m_NCS_OS_NTOHLL_P(&tmr->key);  
  total_ticks_left = (ticks_to_expiry - ticks_elapsed);

  *p_tleft = total_ticks_left * NCS_MILLISECONDS_PER_TICK;
  
  return ncslpg_give(&gl_tcb.persist, NCSCC_RC_SUCCESS);
}

 /****************************************************************************
  * Function Name: ncs_tmr_whatsout
  *
  * Purpose: Show all Timer Blocks that are in the free pool but are still
  *          in the dormant state (indicating that they have been STOPPED or
  *          have expired, but have not yet been DESTROYED.
  *
  * NOTE: This is quick output scheme. Needs to be fit into sysmon model.
  *
  ****************************************************************************/
#if (NCS_TMR_DBG_ENABLE == 0)  

uns32 ncs_tmr_whatsout (void){return NCSCC_RC_SUCCESS;}

#else

static char* gl_tmr_states[] = {"illegal",
                                "CREATE",
                                "START",
                                "illegal",
                                "DORMANT",
                                "illegal",
                                "illegal",
                                "illegal",
                                "DESTROY"};
uns32       
ncs_tmr_whatsout (void)
  {
  SYSF_TMR* free;
  uns32     cnt = 1;
  char      pBuf[100];
  
  m_NCS_CONS_PRINTF ("|---+----+-----+-----------+-----------------------------|\n");
  m_NCS_CONS_PRINTF ("|            O U T S T A N D I N G   T M R S             |\n");
  m_NCS_CONS_PRINTF ("|---+----+-----+-----------+------------+----------------|\n");
  m_NCS_CONS_PRINTF ("|  #|age |Owner|Owner file |    state   |       pointer  |\n");
  m_NCS_CONS_PRINTF ("|  #|    | line|           |            |                |\n");
  m_NCS_CONS_PRINTF ("|---|----+-----+-----------+------------+----------------|\n");
      
  if ((ncslpg_take(&gl_tcb.persist) == FALSE) || (tmr_destroying == TRUE))
    {
    m_NCS_CONS_PRINTF ("< . . . TMR SVC DESTROYED: .CLEANUP ALREADY DONE..>\n");
    return NCSCC_RC_FAILURE; /* going or gone away.. Lets leave */
    }

  m_NCS_LOCK(&gl_tcb.safe.free_lock, NCS_LOCK_WRITE); /* critical region START */
  
  free = gl_tcb.safe.dmy_keep.keep;  /* get start of keep list; may be NULL  */
  while(free != NULL)
    {
    if (!TMR_TEST_STATE(free,TMR_STATE_DESTROY)) /* some may be in transition, but report all */
      {
      sysf_sprintf (pBuf, "%4d%5d%6d%12s%12s%16lx\n",
                    cnt++,                          /* Nmber  */
                    gl_tcb.tick - free->dbg.stamp,  /* age    */
                    free->dbg.line,                 /* OwnrL  */
                    ncs_fname(free->dbg.file),       /* OwnrF  */
                    gl_tmr_states[free->state],     /* state  */
                    (long)free);                          /* pointr */
      m_NCS_CONS_PRINTF (pBuf);
      }
    free = free->keep;
    }  
  m_NCS_UNLOCK (&gl_tcb.safe.free_lock, NCS_LOCK_WRITE);  /* critical region END */ 
  return ncslpg_give(&gl_tcb.persist, NCSCC_RC_SUCCESS);
  }
#endif

 /****************************************************************************
  * Function Name: ncs_tmr_getstats
  *
  * Purpose: Show the current statistics associated with the timer service.
  *
  * NOTE: This is quick output scheme. Needs to be fit into sysmon model.
  ****************************************************************************/

#if (NCSSYSM_TMR_STATS_ENABLE == 0) 

uns32 ncs_tmr_getstats (void) {return NCSCC_RC_SUCCESS;}

#else

uns32       
ncs_tmr_getstats (void)
  {
  m_NCS_CONS_PRINTF ("|---------------------------------------|\n");
  m_NCS_CONS_PRINTF ("|   T I M E R      S T A T I S T I C S  |\n");
  m_NCS_CONS_PRINTF ("|----------------------+----------------|\n");
  m_NCS_CONS_PRINTF (" worst ring hwm        :   %d\n", gl_tcb.stats.ring_hwm);
  m_NCS_CONS_PRINTF (" ttl timers hwm        :   %d\n", gl_tcb.stats.ring_hwm);
  m_NCS_CONS_PRINTF (" raw started tmrs      :   %d\n", gl_tcb.stats.start_cnt);
  m_NCS_CONS_PRINTF (" raw expired tmrs      :   %d\n", gl_tcb.stats.expiry_cnt);
  m_NCS_CONS_PRINTF (" raw cancelled tmrs    :   %d\n", gl_tcb.stats.stop_cnt);
  m_NCS_CONS_PRINTF (" free pool hwm         :   %d\n", gl_tcb.stats.free_hwm);
  m_NCS_CONS_PRINTF (" free pool now         :   %d\n", gl_tcb.stats.free_now);
  m_NCS_CONS_PRINTF (" ttl active tmrs       :   %d\n", gl_tcb.stats.ttl_active);
  m_NCS_CONS_PRINTF (" ttl tmr-blks in sys   :   %d\n", gl_tcb.stats.ttl_tmrs);

  return NCSCC_RC_SUCCESS;
  }

#endif

/****************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************/

#elif (CURRENT_TMR == 1)


/** This is the size of the timer tick ring.
 **/
#define SYSF_TMR_RING_SIZE         1024


/** This is the period of the timer tick we request from the OS.
 **/
#define NCS_MILLISECONDS_PER_TICK 100

typedef struct sysf_tmr
{
  struct sysf_tmr    *next;
  struct sysf_tmr    *prev;
  struct sysf_tmr   **slot;
  uns32               laps;
  TMR_CALLBACK        tmrCB;
  void               *tmrUarg;
  struct sysf_tmr_cb *p_tcb;
} sysfTmr;


#define m_MMGR_ALLOC_TMR (sysfTmr *)m_NCS_MEM_ALLOC(sizeof(sysfTmr), \
                     NCS_MEM_REGION_PERSISTENT, \
                     NCS_SERVICE_ID_OS_SVCS, 4)
#define m_MMGR_FREE_TMR(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_OS_SVCS, 4)



typedef struct sysf_tmr_cb
{
   void       *id;
   NCS_LOCK     lock;
   uns32         ring_index;
   sysfTmr    *ring [SYSF_TMR_RING_SIZE];
} sysfTmrCB;


/** Global variables used by the timer subsystem...
 **/
static sysfTmrCB l_tcb;
uns32 gl_tmr_milliseconds;


/** Periodic expiration of out system timer.
 ** First version - walk list of existing timers and expire them.
 **/

static void
sysfTmrExpiry (void *uarg)
{
  sysfTmr        *tmr;
  TMR_CALLBACK    tmrCB;
  void           *tmrUarg;
  sysfTmr    *expire_lane=0;

   USE(uarg);
  /** Get the lock on the timer subsystem...
   **/
  m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);


  /** If the timer subsystem has been shutdown, unlock and return...
   **/
  if (l_tcb.id == NULL)
  {
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
  }


  /** Update our elapsed time millisecond ticker...
   **/
  gl_tmr_milliseconds = gl_tmr_milliseconds + NCS_MILLISECONDS_PER_TICK;


  /** Now walk the list of timers in current slot.
   ** For each timer that has expired (lap counter is 0),
   **    move it to a list of timers to process (called the expire lane),
   **    otherwise, decrement the lap counter.
   **/
  for (tmr = l_tcb.ring[l_tcb.ring_index]; tmr != NULL;  )
  {

     if (tmr->laps == 0)  /* has this timer expired ? */
     {
         sysfTmr  *next_tmr = tmr->next;

        /** Ok, pull it from the linked list... **/
        if (tmr->prev != NULL)
           tmr->prev->next = tmr->next;
        else
           *tmr->slot = tmr->next;

        if (tmr->next != NULL)
           tmr->next->prev = tmr->prev;

        /** Now, add it to the head of the expire lane */
        tmr->slot         = &expire_lane;
        tmr->next         = *tmr->slot;
        if (tmr->next != NULL)
           tmr->next->prev = tmr;
        *tmr->slot        = tmr;
        tmr->prev         = NULL;

        tmr = next_tmr; /* go on to check next timer from ring slot */
     }
     else
     {
        tmr->laps--;
        tmr = tmr->next;
     }

  }
  /** Move on to the next slot in the timer ring.
   ** Do the wrap too.
   **/
  l_tcb.ring_index = (l_tcb.ring_index+1) % SYSF_TMR_RING_SIZE;

  /**
   ** Then, for each timer in the expire lane, make the call back.
   **/
  while ((tmr = expire_lane) != NULL)
  {

     /** This timer has expired. Pull it off the expire lane
      ** and invoke the callback.
      **/
     tmrCB             = tmr->tmrCB;
     tmrUarg           = tmr->tmrUarg;
     expire_lane       = tmr->next;
     if (tmr->next != NULL)
         tmr->next->prev = NULL;
     tmr->next         = NULL;

     /** Null out the backpointer to the timer sys cb,
      ** so we know we've expired it...
      **/
      tmr->p_tcb = NULL;

     /** Unlock, call the owner, lock and continue...
      **/
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      tmrCB (tmrUarg);
      m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);

      m_SYSF_TMR_LOG_INFO("sysfTmrExpiry",tmr);
  }   

  /** We are done, unlock and return...
   **/
  m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  return;
}


/** Start the Timer Process - it will manage all timers.
 ** tmrPeriod is number of 100 ms
 **/
NCS_BOOL
sysfTmrCreate ()
{
   NCS_OS_TIMER osprims_tmr;
   static int done = 0;

   if (done == 0)
       done = 1;
   else
       return TRUE;

   /** Start with an empty Timer Service control block.
    **/
   m_NCS_MEMSET (&l_tcb, '\0', sizeof (sysfTmrCB));


   /** Zero our global elapsed time millisecond counter.
    **/
   gl_tmr_milliseconds = 0;


   /** Initialize the lock, and then get it.
    **/
   m_NCS_LOCK_INIT(&l_tcb.lock);
   m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);


   /** Make a request to the OSPrims Timer api to get a 100 ms tick.
    **/
   osprims_tmr.info.create.o_handle         = NULL;
   osprims_tmr.info.create.i_callback       = sysfTmrExpiry;
   osprims_tmr.info.create.i_cb_arg           = NULL;
   osprims_tmr.info.create.i_period_in_ms   = NCS_MILLISECONDS_PER_TICK;
   m_NCS_OS_TIMER (&osprims_tmr, NCS_OS_TIMER_CREATE);


   /** Save the resulting handle.
    **/
   l_tcb.id = osprims_tmr.info.create.o_handle;
      
   m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);


   /** Return final status to caller.
    **/
   if (l_tcb.id == NULL)
      return FALSE;
   else
      return TRUE;

}



/** Stop the Timer Process - it will free all outstanding timers.
 **/

NCS_BOOL
sysfTmrDestroy ()
{
   NCS_OS_TIMER osprims_tmr;

  m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  /** Note that no attempt is made to free outstanding timers.
   ** It is the job of the memory manager to finger offending
   ** clients at shutdown time.
   **/

  /** Tell the OSPrims Timer API to stop the ticker.
   **/
  osprims_tmr.info.release.i_handle      = l_tcb.id;

  m_NCS_OS_TIMER (&osprims_tmr, NCS_OS_TIMER_RELEASE);
   
  l_tcb.id = NULL;

  m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  /* don't destroy the lock (but remember that you could!).
   * m_NCS_LOCK_DESTROY (&l_tcb.lock);
   */
  return TRUE;
}




/*****************************************************************************
 *
 *                  Timer Add, Stop, and Expiration Handler
 *
 *****************************************************************************/

tmr_t
sysfTmrAlloc ()
{
  sysfTmr *tmr;


  if (l_tcb.id == NULL)
  {
      /** timer subsystem has been shutdown.  return...
       **/
      return NULL;
  }


  if ((tmr = m_MMGR_ALLOC_TMR) == NULL)
    return NULL;

  tmr->p_tcb = NULL;
  m_SYSF_TMR_LOG_INFO("sysfTmrAlloc",tmr);
  return (tmr_t) tmr;

}


void*
sysfTmrStart (tmr_t        tid,
             uns32        tmrDelay, /* timer period in number of 10ms units */
             TMR_CALLBACK tmrCB,
             void        *tmrUarg)
{

  sysfTmr     *tmr;
  sysfTmr    **slot;
  uns32 ring_index;
  uns32 laps;
  uns32 scaled;

  /** We can't do anything with an NULL timer...
   **/
  if ((tmr = (sysfTmr *)tid) == NULL)   
    return NULL;

  /** Lock the timer subsystem
   **/
  m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);


  /** Do some up front math.  Scale the delay into ticks from now.
   **/
  scaled = (tmrDelay * 10) / NCS_MILLISECONDS_PER_TICK;


  /** Calculate the index into the ring for the first (or only lap)
   ** Don't forget the current ticker's slot in the ring...
   **/
  ring_index = (scaled+l_tcb.ring_index) % SYSF_TMR_RING_SIZE;


  /** Calculate the number of remaining laps...
   **/
  laps = scaled / SYSF_TMR_RING_SIZE;

  /** Get a pointer to the anchor for this ring slot...
   **/
  slot = &l_tcb.ring[ring_index];

  /** Populate the SYSF_TMR entry with all the data...
   **/
  tmr->tmrCB    = tmrCB;
  tmr->tmrUarg  = tmrUarg;
  tmr->laps     = laps;
  tmr->next     = NULL;
  tmr->prev     = NULL;
  tmr->slot     = slot;



  /** timer subsystem has been shutdown.  unlock and return...
   **/
  if (l_tcb.id == NULL)
  {
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return NULL;
  }


  /** Now we put this new timer onto the ring in the correct slot,
   ** for now, always at the front of the ring...
   **/
  tmr->next = *slot;
  *slot     = tmr;
  tmr->prev = NULL;
  if (tmr->next != NULL)
     tmr->next->prev = tmr;

  /** Ok, now we can back point to the timer service control block,
   ** unlock and return.
   **/
  tmr->p_tcb       = &l_tcb;
  m_SYSF_TMR_LOG_INFO("sysfTmrStart",tmr);
  m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  return (void*)tmr;

}



void
sysfTmrStop (tmr_t tmrID)
{
  sysfTmr  *tmr;

  m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  if ((tmr = (sysfTmr*)tmrID) == NULL)
    {
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
    }

  if (tmr->p_tcb != &l_tcb)
    {
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
    }

  tmr->p_tcb = NULL;


  /** timer subsystem has been shutdown.  unlock and return...
   **/
  if (l_tcb.id == NULL)
  {
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
  }

  /** Ok, pull it from the linked list...
   **/
  if (tmr->prev != NULL)
    tmr->prev->next = tmr->next;
  else
    *tmr->slot = tmr->next;

  if (tmr->next != NULL)
    {
      tmr->next->prev = tmr->prev;
    }

  m_SYSF_TMR_LOG_INFO("sysfTmrStop",tmr);

  m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);

}


void
sysfTmrFree (tmr_t tmrID)
{
  sysfTmr  *tmr;

    /** don't check for timer subsytem shutdown.  This allows
     ** timer control block memory to be freed.
     **/

  if ((tmr = (sysfTmr*)tmrID) == NULL)
      return;


  if (tmr->p_tcb != NULL)  /* in case App did not Stop timer first */
     sysfTmrStop (tmrID);


   m_MMGR_FREE_TMR (tmrID);

  m_SYSF_TMR_LOG_INFO("sysfTmrFree",tmr);
}



uns32
sysfTmrRemaining(tmr_t tmrID, uns32 * p_tleft)
{
  sysfTmr  *tmr, **p_current_index, **p_tmr_index;
  long index_diff;
  uns32 total_ticks_left;

  m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  if ((tmr = (sysfTmr*)tmrID) == NULL)
    {
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return NCSCC_RC_FAILURE;
    }

  if (tmr->p_tcb != &l_tcb)
    {
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return NCSCC_RC_FAILURE;
    }


  /** timer subsystem has been shutdown.  unlock and return...
   **/
  if (l_tcb.id == NULL)
  {
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return NCSCC_RC_FAILURE;
  }

  /* we have a recognized timer - calculate the time remaining */
  
  /* find the index currently ready for processing by timer service */
  p_current_index = &(l_tcb.ring[l_tcb.ring_index]);

  /* where (what index) is timer being queried about located ? */
  p_tmr_index = tmr->slot;

  /* how much ticks between indexes - account for wrap */
  index_diff = p_tmr_index - p_current_index;
  if (index_diff < 0)
     index_diff += SYSF_TMR_RING_SIZE;

  /* factor in laps */
  total_ticks_left = index_diff + (tmr->laps * SYSF_TMR_RING_SIZE);


  *p_tleft = total_ticks_left * NCS_MILLISECONDS_PER_TICK;


  m_SYSF_TMR_LOG_INFO("sysfTmrRemaining",tmr);

  m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  return NCSCC_RC_SUCCESS;
}

#elif (OLDEST_TMR == 1)


/*************************************************************************
 *
 * Old Timer Implementation
 *
 ************************************************************************/


typedef struct sysf_tmr
{
  struct sysf_tmr *next;
  struct sysf_tmr *prev;
  TMR_CALLBACK     tmrCB;
  void            *tmrUarg;
  uns32            tmrDelay;
  void            *p_tcb;
} sysfTmr;

#define sysfTmr_Null  ((sysfTmr *)0)

#define m_MMGR_ALLOC_TMR (sysfTmr *)m_NCS_MEM_ALLOC(sizeof(sysfTmr), \
                     NCS_MEM_REGION_PERSISTENT, \
                     NCS_SERVICE_ID_OS_SVCS, 4)
#define m_MMGR_FREE_TMR(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_OS_SVCS, 4)



typedef struct sysf_tmr_cb
{
   void     *id;
   sysfTmr  *anchor;
   NCS_LOCK   lock;
   NCS_OS_TIMER os;
} sysfTmrCB;

static sysfTmrCB l_tcb;

#define NCS_MILLISECONDS_PER_TICK 100

uns32 gl_tmr_milliseconds;


/** Periodic expiration of out system timer.
 ** First version - walk list of existing timers and expire them.
 **/

static void
sysfTmrExpiry (void *uarg)
{
  sysfTmr        *tmr;
  TMR_CALLBACK    tmrCB;
  void           *tmrUarg;

  m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  gl_tmr_milliseconds = gl_tmr_milliseconds + NCS_MILLISECONDS_PER_TICK;

  if (l_tcb.anchor == sysfTmr_Null)
    {

      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
    }

  if (l_tcb.id == NULL)
  {
      /** timer subsystem has been shutdown.  unlock and
       ** return...
       **/
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
  }

  if (l_tcb.anchor->tmrDelay > 0)
  {
      l_tcb.anchor->tmrDelay--;
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
  }

  while ((tmr = l_tcb.anchor) != sysfTmr_Null)
  {
      if (tmr->tmrDelay != 0)
          break;
      tmrCB            = tmr->tmrCB;
      tmrUarg          = tmr->tmrUarg;
      l_tcb.anchor = tmr->next;
      if (l_tcb.anchor != sysfTmr_Null)
          l_tcb.anchor->prev  = sysfTmr_Null;

      tmr->p_tcb = NULL;
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      tmrCB (tmrUarg);
      m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);
  }   

  m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
  return;
}




/** Start the Timer Process - it will manage all timers.
 ** tmrPeriod is number of 100 ms
 **/
NCS_BOOL
sysfTmrCreate ()
{
   static int done = 0;

   if (done == 0)
       done = 1;
   else
       return TRUE;

  l_tcb.anchor = sysfTmr_Null;

  m_NCS_LOCK_INIT(&l_tcb.lock);
  m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  gl_tmr_milliseconds = 0;
  l_tcb.os.info.create.o_handle = NULL;
  l_tcb.os.info.create.i_callback = sysfTmrExpiry;
  l_tcb.os.info.create.i_cb_arg   = NULL;
  l_tcb.os.info.create.i_period_in_ms   = NCS_MILLISECONDS_PER_TICK;
  m_NCS_OS_TIMER (&l_tcb.os, NCS_OS_TIMER_CREATE);
  l_tcb.id = l_tcb.os.info.create.o_handle;
      
  m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  return TRUE;

}



/** Stop the Timer Process - it will free all outstanding timers.
 **/

NCS_BOOL
sysfTmrDestroy ()
{
#if 0
  sysfTmr     *tmr;
#endif

  m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  /** don't free the timer control blocks - rely on the owner of
   ** the timer to do this.  That way subsystems which don't do
   ** a complete shutdown can be fingered!
   **/
#if 0
  while ((tmr = l_tcb.anchor) != sysfTmr_Null)
  {
      l_tcb.anchor = tmr->next;
      m_MMGR_FREE_TMR(tmr);
  }
#endif

  l_tcb.os.info.create.i_callback     = NULL;
  l_tcb.os.info.create.i_cb_arg       = NULL;
  l_tcb.os.info.create.i_period_in_ms = 0;

  l_tcb.os.info.release.i_handle = l_tcb.id;

  m_NCS_OS_TIMER (&l_tcb.os, NCS_OS_TIMER_RELEASE);
   
  l_tcb.id = NULL;

  m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
  /* don't destroy the lock (but remember that you could!).
   * m_NCS_LOCK_DESTROY (&l_tcb.lock);
   */
  return TRUE;
}




/*****************************************************************************
 *
 *                  Timer Add, Stop, and Expiration Handler
 *
 *****************************************************************************/







tmr_t
sysfTmrAlloc ()
{
  sysfTmr *tmr;


  if (l_tcb.id == NULL)
  {
      /** timer subsystem has been shutdown.  return...
       **/
      return NULL;
  }


  if ((tmr = m_MMGR_ALLOC_TMR) == NULL)
    return NULL;

  tmr->p_tcb = NULL;
  return (tmr_t) tmr;

}


void
sysfTmrStart (tmr_t        tid,
         uns32        tmrDelay, /* timer period in number of 10ms units */
         TMR_CALLBACK tmrCB,
         void        *tmrUarg)
{

  sysfTmr     *tmr, *tn, **tmrpp;
  unsigned int delay;
  uns32 scale;

  if ((tn = (sysfTmr *)tid) == NULL)   
    return;

  tn->tmrCB   = tmrCB;
  tn->tmrUarg = tmrUarg;

  /*  scale = tmrDelay / 10;   7/27/99 pgs - accomodate different ms/tick values */
  scale = (tmrDelay * 10) / NCS_MILLISECONDS_PER_TICK; /* calc # of ticks from delay */
  if (scale == 0)
    scale = 1;

  m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  if (l_tcb.id == NULL)
  {
      /** timer subsystem has been shutdown.  unlock and
       ** return...
       **/
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
  }

  if (l_tcb.anchor == sysfTmr_Null)
    {
      tn->prev         = sysfTmr_Null;
      tn->next         = sysfTmr_Null;
      tn->tmrDelay     = scale;
      l_tcb.anchor     = tn;
      tn->p_tcb        = (void *)&l_tcb;
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
    }

  tmrpp = &l_tcb.anchor;
  delay = 0;
  do
    {
      tmr   = *tmrpp;
      delay += tmr->tmrDelay;
      if (delay > scale)
   {
     tn->tmrDelay  = scale - (delay - tmr->tmrDelay);
     tmr->tmrDelay -= tn->tmrDelay;
     *tmrpp        = tn;
     tn->prev      = tmr->prev;
     tmr->prev     = tn;
     tn->next      = tmr;
     tn->p_tcb     = (void *)&l_tcb;
     m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
     return;
   }

      tmrpp = &tmr->next;
    } while (tmr->next != sysfTmr_Null);


  /* We are at the end of the list*/
  tmr->next    = tn;
  tn->next     = sysfTmr_Null;
  tn->prev     = tmr;
  tn->tmrDelay = scale - delay;
  tn->p_tcb    = (void *)&l_tcb;

  m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);

}



void
sysfTmrStop (tmr_t tmrID)
{
  sysfTmr  *tmr;

  m_NCS_LOCK (&l_tcb.lock, NCS_LOCK_WRITE);

  if ((tmr = (sysfTmr*)tmrID) == sysfTmr_Null)
    {
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
    }

  if (tmr->p_tcb != (void *)&l_tcb)
    {
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
    }

  tmr->p_tcb = NULL;

  if (l_tcb.id == NULL)
  {
      /** timer subsystem has been shutdown.  unlock and
       ** return...
       **/
      m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);
      return;
  }


  if (tmr->prev != sysfTmr_Null)
    tmr->prev->next = tmr->next;
  else
    l_tcb.anchor = tmr->next;

  if (tmr->next != sysfTmr_Null)
    {
      tmr->next->prev = tmr->prev;
      tmr->next->tmrDelay += tmr->tmrDelay;
    }


  m_NCS_UNLOCK (&l_tcb.lock, NCS_LOCK_WRITE);

}


void
sysfTmrFree (tmr_t tmrID)
{
  sysfTmr  *tmr;

    /** don't check for timer subsytem shutdown.  This allows
     ** timer control block memory to be freed.
     **/

  if ((tmr = (sysfTmr*)tmrID) == sysfTmr_Null)
      return;


  if (tmr->p_tcb != NULL)  /* in case App did not Stop timer first */
     sysfTmrStop (tmrID);


   m_MMGR_FREE_TMR (tmrID);
}


#endif /* Timer Vintage To Use */

