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

  This module contains procedures related to System Monitoring operations

..............................................................................

  FUNCTIONS INCLUDED in this module:

  ncssysm_lm                    - PUBLIC: system monitor layer management entry point
  ncssysm_ss                    - PUBLIC: system monitor subscription entry point

******************************************************************************/

/** Get compile time options...**/
#include "ncs_opt.h"

/** Get general definitions...**/
#include "gl_defs.h"
#include "ncs_osprm.h"


/** Get the key abstractions for this thing...**/
#include "ncs_stack.h"
#include "ncs_sysm.h"
#include "ncssysf_def.h"
#include "ncssysf_ipc.h"
#include "ncssysf_tsk.h"
#include "sysfsysm.h"
#include "sysf_ipc.h"

#if (NCS_USE_SYSMON == 1)
#if 0 /* IR00059629 */
extern int ignore_subsystem[NCS_SERVICE_ID_MAX];
#endif

extern UB_POOL_MGR gl_ub_pool_mgr;
/* extern NCS_LOCK     mmgr_mpool_lock; */
extern NCS_MMGR     mmgr;

uns32 sm_init(void);
uns32 sm_terminate(void);
uns32 sm_reg_lmt_cb(NCSSYSM_REG_LMT_CB* info);
uns32 sm_get(NCSSYSM_GET* info);
uns32 sm_set(NCSSYSM_SET* info);
uns32 sm_iget(NCSSYSM_IGET* info);
uns32 sm_iset(NCSSYSM_ISET* info);

uns32 sm_mem_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_MEM_REG* info);
uns32 sm_buf_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_BUF_REG* info);
uns32 sm_tmr_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_TMR_REG* info);
uns32 sm_ipc_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_IPC_REG* info);
uns32 sm_cpu_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_CPU_REG* info);
uns32 sm_ipra_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_IPRA_REG* info);
uns32 sm_mem_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_MEM_UNREG* info);
uns32 sm_buf_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_BUF_UNREG* info);
uns32 sm_tmr_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_TMR_UNREG* info);
uns32 sm_ipc_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_IPC_UNREG* info);
uns32 sm_cpu_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_CPU_UNREG* info);
uns32 sm_ipra_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_IPRA_UNREG* info);

uns32 sm_mem_stats (NCSSYSM_MEM_STATS*  info);
uns32 sm_cpu_stats (NCSSYSM_CPU_STATS*  info);
uns32 sm_buf_stats (NCSSYSM_BUF_STATS*  info);
uns32 sm_ipc_stats (NCSSYSM_IPC_STATS*  info);
uns32 sm_tmr_stats (NCSSYSM_TMR_STATS*  info);
uns32 sm_task_stats(NCSSYSM_TASK_STATS* info);
uns32 sm_lock_stats(NCSSYSM_LOCK_STATS* info);

uns32 sm_mem_rpt_thup(NCSSYSM_MEM_RPT_THUP*  info);
uns32 sm_mem_rpt_ssup(NCSSYSM_MEM_RPT_SSUP*  info);
uns32 sm_mem_rpt_wo  (NCSSYSM_MEM_RPT_WO*    info);
uns32 sm_mem_rpt_wos (NCSSYSM_MEM_RPT_WOS*   info);    
uns32 sm_buf_rpt_wo  (NCSSYSM_BUF_RPT_WO*    info);
uns32 sm_buf_rpt_wos (NCSSYSM_BUF_RPT_WOS*   info); 

void sysmon_task   ( SYSF_MBX*  mbx   );
void cpu_watch_task( void*      unused);
void mem_watch_task( void*      unused);
void buf_watch_task( void*      unused);
void ipc_watch_task( void*      unused);

void ipra_watch_trigger(void* arg);

void sm_pulse_tmr_fnc(void* arg);

#if ((NCSSYSM_MEM_DBG_ENABLE == 1) && (NCSSYSM_STKTRACE == 1))
uns32 sm_stktrace_add(NCSSYSM_MEM_STK_ADD* info);
uns32 sm_stktrace_flush(NCSSYSM_MEM_STK_FLUSH* info);
uns32 sm_stktrace_report(NCSSYSM_MEM_STK_RPT* info);
#endif


/*****************************************************************************/

NCS_SYSMON gl_sysmon; /* OS services system monitor instance */

/*****************************************************************************/


void sm_pulse_tmr_fnc(void* arg)
{
  SM_MSG* msg = m_MMGR_ALLOC_SM_MSG;


  if(msg != NULL)
  {

    msg->type = SMMT_PULSE;
    msg->data = arg;

    if(NCSCC_RC_SUCCESS != m_NCS_IPC_SEND(&gl_sysmon.sysmon_mbx,
                                        msg,
                                        NCS_IPC_PRIORITY_NORMAL))
    {
        m_MMGR_FREE_SM_MSG(msg);
        m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }
  }
}

void sysmon_task( SYSF_MBX*  mbx)
{
#if 0
  SM_MSG* msg;  

  while ((msg = (SM_MSG*)m_NCS_IPC_RECEIVE(mbx,msg)) != NULL)
  {
   switch(msg->type)
   {
   case SMMT_PULSE:
     {
       SM_PULSE* pulse = (SM_PULSE*)msg->data;
       m_MMGR_FREE_SM_MSG(msg);

       switch(pulse->type)
       {
       case SMPT_MEM:
         {
           NCS_MPOOL        *mp;
           NCS_MPOOL_ENTRY  *me;
           
           for (mp = gl_sysmon.mmgr->mpools; ; mp++)
           {
             m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE);
             me = mp->inuse;
             while (me != NULL)
             {
               me->age++;
               me = me->next;
             }
             if(mp->size == 0) 
               {
               m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE);
               break;
               }
             m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE);
           }


         }
         break;
       case SMPT_BUF:
         if(pulse->data > UB_MAX_POOLS)
         {
           m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         }
         else
         {
           m_PMGR_LK(&gl_ub_pool_mgr.lock);
           
           if(gl_sysmon.ub_pool_mgr->pools[pulse->data].mem_age != NULL)
             gl_sysmon.ub_pool_mgr->pools[pulse->data].mem_age();

           m_PMGR_UNLK(&gl_ub_pool_mgr.lock);
         }
         break;
       default:
         break;
       }

       m_NCS_TMR_START (pulse->tmr_id,
                       pulse->period,
                       pulse->cb_fnc,
                      (void*)pulse);
     }
     break;
   default:
     break;
   }
   m_NCS_TASK_SLEEP(1000);
  }
#endif
}


/*****************************************************************************
 * UTILITY FUNCTIONS: 
 *****************************************************************************/

typedef uns32 (*SM_COND_FNC)(int i,int limit); /* conditional function prototype */

typedef void (*SM_IT_FNC)(int* i);               /* iterator function prototype */

#if ((NCSSYSM_BUF_WATCH_ENABLE == 1)||(NCSSYSM_MEM_WATCH_ENABLE == 1)||(NCSSYSM_IPC_WATCH_ENABLE == 1)||(NCSSYSM_CPU_WATCH_ENABLE == 1))

static void sm_it_fnc_inc(int* i)
{
  (*i)++;
}

static void sm_it_fnc_dec(int* i)
{
  (*i)--;
}

static uns32 sm_cond_fnc_ge(int i,int limit)
{
  return (i >= limit);
}

static uns32 sm_cond_fnc_le(int i,int limit)
{
  return (i <= limit);
}

static NCS_BOOL sm_cb_cond_fnc(uns32 watch_percent,
                       uns32 last_percent,
                       uns32 curr_percent,
                       NCSSYSM_DIR dir)
{
  if(dir == NCSSYSM_DIR_UP)
  {
    if((watch_percent > last_percent) && (watch_percent <= curr_percent))
      return TRUE;
    else
      return FALSE;
  }
  else
  {
    if((watch_percent < last_percent) && (watch_percent >= curr_percent))
      return TRUE;
    else
      return FALSE;
  }

  return FALSE;
}

#endif

/***************************************************************************/

void buf_watch_task( void*  unused)
{
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
  uns32            curr_bkt;
  uns32            curr_mem;
  uns32            last_mem;
  int            i = 0;
  int            limit;
  SM_COND_FNC      cond_fnc;
  SM_IT_FNC        it_fnc;
  NCSSYSM_DIR       dir;
  NCSSYSM_BUF_EVT   evt;
  uns32            j = 0;
  uns32            pool_id = 0;
  uns32            one_pcnt = 0;

  while (1)
  {
    m_NCS_TASK_SLEEP(gl_sysmon.buf_wrr == 0 ? 1000 : gl_sysmon.buf_wrr);

   for(j = 0;j < UB_MAX_POOLS; j++)
   {
      if(gl_sysmon.buf_ttl[j] == 0xffffffff)
        continue;

      m_PMGR_LK(&gl_ub_pool_mgr.lock);

     curr_mem = gl_sysmon.ub_pool_mgr->pools[j].stats.curr;

     if((curr_mem == gl_sysmon.buf_last[j]) ||
       (gl_sysmon.buf_ttl[j] < curr_mem))
     {
       /* no changes... */
       m_PMGR_UNLK(&gl_ub_pool_mgr.lock);
       continue;
     }
     
     pool_id  = gl_sysmon.ub_pool_mgr->pools[j].stats.pool_id;
     last_mem = gl_sysmon.buf_last[j];

     curr_bkt = curr_mem / gl_sysmon.buf_bkt_size[j];
     if(curr_bkt == SM_BKT_CNT)
       curr_bkt--;

     if(curr_bkt >= gl_sysmon.buf_bkt_last[j])
     {
       limit = curr_bkt;
       i = gl_sysmon.buf_bkt_last[j];
       it_fnc = sm_it_fnc_inc;
       cond_fnc = sm_cond_fnc_le;
       
       if(curr_bkt == gl_sysmon.buf_bkt_last[j])
       {
         if(curr_mem > gl_sysmon.buf_last[j])
         {
           dir = NCSSYSM_DIR_UP;
         }
         else
         {
           dir = NCSSYSM_DIR_DOWN;
         }
       }
       else
       {
         dir = NCSSYSM_DIR_UP;
       }
     }
     else
     {
       i = gl_sysmon.buf_bkt_last[j];
       limit = curr_bkt;
       it_fnc = sm_it_fnc_dec;
       cond_fnc = sm_cond_fnc_ge;
       dir = NCSSYSM_DIR_DOWN;
     }
     
     m_PMGR_UNLK(&gl_ub_pool_mgr.lock);

     one_pcnt = gl_sysmon.buf_bkt_size[j] / (100 / SM_BKT_CNT);

     for(;(*cond_fnc)(i,limit);(*it_fnc)(&i))
     {
       SM_BUF_WATCH* watch = gl_sysmon.buf_watches[i][j];

       if(((uns32)i == gl_sysmon.buf_bkt_last[j]) || ((uns32)i == curr_bkt))
       {
         for(;watch != NULL;watch = watch->next)
         {
           if((sm_cb_cond_fnc(watch->percent,
                              last_mem / one_pcnt,
                              curr_mem / one_pcnt,
                              dir) == TRUE) &&
              (watch->pool_id == pool_id))
           {
             evt.i_dir = dir;
             evt.i_opaque = watch->opaque;
             evt.i_subscr_pcnt = watch->percent;
             evt.i_cur_pcnt = curr_mem / one_pcnt;
             evt.i_usr_key = watch->usr_key;
             evt.i_vrtr_id = gl_sysmon.vrtr_id;
             evt.i_pool_id = pool_id;
             
             (*(watch->cb_fnc))(&evt);
           }
         }
       }
       else
       {
         for(;watch != NULL;watch = watch->next)
         {
           if(watch->pool_id == pool_id)
           {
             evt.i_dir = dir;
             evt.i_opaque = watch->opaque;
             evt.i_subscr_pcnt = watch->percent;
             evt.i_cur_pcnt = curr_mem / one_pcnt;
             evt.i_usr_key = watch->usr_key;
             evt.i_vrtr_id = gl_sysmon.vrtr_id;
             evt.i_pool_id = pool_id;
             
             (*(watch->cb_fnc))(&evt);
           }
         }
       }
     }

     gl_sysmon.buf_last[j] = curr_mem;
     gl_sysmon.buf_bkt_last[j] = curr_bkt;
   }
  }
#endif
}

void mem_watch_task( void*  unused)
{
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
  volatile uns32          curr_bkt;
  volatile uns32          curr_mem;
  volatile uns32          last_mem;
  int          i = 0;
  int          limit;
  SM_COND_FNC    cond_fnc;
  SM_IT_FNC      it_fnc;
  NCSSYSM_DIR     dir;
  NCSSYSM_MEM_EVT evt;
  uns32          one_pcnt = 0;

  while (1)
  {
    m_NCS_TASK_SLEEP(gl_sysmon.mem_wrr == 0 ? 1000 : gl_sysmon.mem_wrr);

    if(gl_sysmon.mem_ttl == 0xffffffff)
      continue;

   /* SMM the global _mpool_lock is gone. Here we read without any guarentee
    * that some other thread is writing..
    */

   curr_mem = gl_sysmon.mmgr->stats.curr;

   if (gl_sysmon.mem_ttl < curr_mem)
   {
     /* mem currently allocated has somehow exceeded the total mem value !!!*/
     continue;
   }

   last_mem = gl_sysmon.mem_last;   
   
   curr_bkt = curr_mem / gl_sysmon.mem_bkt_size;
   if(curr_bkt == SM_BKT_CNT)
     curr_bkt--;

   if(curr_bkt >= gl_sysmon.mem_bkt_last)
   {
     limit = curr_bkt;
     i = gl_sysmon.mem_bkt_last;
     it_fnc = sm_it_fnc_inc;
     cond_fnc = sm_cond_fnc_le;

     if(curr_bkt == gl_sysmon.mem_bkt_last)
     {
       if(curr_mem > gl_sysmon.mem_last)
       {
         dir = NCSSYSM_DIR_UP;
       }
       else
       {
         dir = NCSSYSM_DIR_DOWN;
       }
     }
     else
     {
       dir = NCSSYSM_DIR_UP;
     }
   }
   else
   {
     i = gl_sysmon.mem_bkt_last;
     limit = curr_bkt;
     it_fnc = sm_it_fnc_dec;
     cond_fnc = sm_cond_fnc_ge;
     dir = NCSSYSM_DIR_DOWN;
   }

   one_pcnt = gl_sysmon.mem_bkt_size / (100 / SM_BKT_CNT);

   for(;(*cond_fnc)(i,limit);(*it_fnc)(&i))
   {
     SM_MEM_WATCH* watch = gl_sysmon.mem_watches[i];

     if(((uns32)i == gl_sysmon.mem_bkt_last) || ((uns32)i == curr_bkt))
     {
       for(;watch != NULL;watch = watch->next)
       {
         if(sm_cb_cond_fnc(watch->percent,
                           last_mem / one_pcnt,
                           curr_mem / one_pcnt,
                           dir) == TRUE)
         {
           evt.i_dir = dir;
           evt.i_opaque = watch->opaque;
           evt.i_subscr_pcnt = watch->percent;
           evt.i_cur_pcnt = curr_mem / one_pcnt;
           evt.i_usr_key = watch->usr_key;
           evt.i_vrtr_id = gl_sysmon.vrtr_id;
           
           (*(watch->cb_fnc))(&evt);
         }
       }
     }
     else
     {
       for(;watch != NULL;watch = watch->next)
       {
         evt.i_dir = dir;
         evt.i_opaque = watch->opaque;
         evt.i_subscr_pcnt = watch->percent;
         evt.i_cur_pcnt = curr_mem / one_pcnt;
         evt.i_usr_key = watch->usr_key;
         evt.i_vrtr_id = gl_sysmon.vrtr_id;
         
         (*(watch->cb_fnc))(&evt);
       }
     }
   }

   gl_sysmon.mem_last = curr_mem;
   gl_sysmon.mem_bkt_last = curr_bkt;
  }
#endif
}

void ipc_watch_task( void*  unused)
{
#if (NCSSYSM_IPC_WATCH_ENABLE == 1)
  uns32          curr_bkt;
  uns32          curr_depth;
  uns32          last_depth;
  int            i = 0;
  int          limit;
  SM_COND_FNC    cond_fnc;
  SM_IT_FNC      it_fnc;
  NCSSYSM_DIR     dir;
  NCSSYSM_IPC_EVT evt;
  NCS_IPC_STATS*  stats;
  uns16          res_id = 0;
  uns32          one_pcnt = 0;

  while (1)
  {
    m_NCS_TASK_SLEEP(gl_sysmon.ipc_wrr == 0 ? 1000 : gl_sysmon.ipc_wrr);

    if(gl_sysmon.ipc_ttl == 0xffffffff)
      continue;

   for(stats = gl_sysmon.ipc_stats;stats != NULL;stats = stats->next)
   {
       m_NCS_LOCK(&stats->resource->queue_lock, NCS_LOCK_WRITE);

       curr_depth = stats->curr_depth;

       if((curr_depth == stats->last_depth) ||
         ((stats->max_depth < curr_depth)))
       {
         /* no changes... */
       m_NCS_UNLOCK(&stats->resource->queue_lock, NCS_LOCK_WRITE);
         continue;
       }

       res_id = stats->resource_id;
       
       last_depth = stats->last_depth;

       curr_bkt = curr_depth / gl_sysmon.ipc_bkt_size;
       if(curr_bkt == SM_BKT_CNT)
         curr_bkt--;       

       if(curr_bkt >= stats->last_bkt)
       {
         limit = curr_bkt;
         i = stats->last_bkt;
         it_fnc = sm_it_fnc_inc;
         cond_fnc = sm_cond_fnc_le;
         
         if(curr_bkt == stats->last_bkt)
         {
           if(curr_depth > stats->last_depth)
           {
             dir = NCSSYSM_DIR_UP;
           }
           else
           {
             dir = NCSSYSM_DIR_DOWN;
           }
         }
         else
         {
           dir = NCSSYSM_DIR_UP;
         }
       }
       else
       {
         i = stats->last_bkt;
         limit = curr_bkt;
         it_fnc = sm_it_fnc_dec;
         cond_fnc = sm_cond_fnc_ge;
         dir = NCSSYSM_DIR_DOWN;
       }

       m_NCS_UNLOCK(&stats->resource->queue_lock, NCS_LOCK_WRITE);

       one_pcnt = gl_sysmon.ipc_bkt_size / (100 / SM_BKT_CNT);

       for(;(*cond_fnc)(i,limit);(*it_fnc)(&i))
       {
         SM_IPC_WATCH* watch = gl_sysmon.ipc_watches[i];
         
         if(((uns32)i == stats->last_bkt) || ((uns32)i == curr_bkt))
         {
           for(;watch != NULL;watch = watch->next)
           {
             if((watch->ipcq->resource_id == res_id) &&
                (sm_cb_cond_fnc(watch->percent, 
                                last_depth / one_pcnt,
                                curr_depth / one_pcnt,
                                dir) == TRUE))
             {
               evt.i_dir = dir;
               evt.i_opaque = watch->opaque;
               evt.i_subscr_pcnt = watch->percent;
               evt.i_cur_pcnt = curr_depth / one_pcnt;
               evt.i_usr_key = watch->usr_key;
               evt.i_vrtr_id = gl_sysmon.vrtr_id;
               
               (*(watch->cb_fnc))(&evt);
             }
           }
         }
         else
         {
           
           for(;watch != NULL;watch = watch->next)
           {
             if(watch->ipcq->resource_id == res_id)
             {
               evt.i_dir = dir;
               evt.i_opaque = watch->opaque;
               evt.i_subscr_pcnt = watch->percent;
               evt.i_cur_pcnt = curr_depth / one_pcnt;
               evt.i_usr_key = watch->usr_key;
               evt.i_vrtr_id = gl_sysmon.vrtr_id;
               
               (*(watch->cb_fnc))(&evt);
             }
           }
         }
       }
       
       stats->last_depth = curr_depth;
       stats->last_bkt = curr_bkt;
   }
  }
#endif
}

void ipra_watch_trigger(void* arg)
{
#if (NCSSYSM_IPRA_WATCH_ENABLE == 1)
  NCSSYSM_IPRA_EVT evt;
  SM_IPRA_WATCH*  watch; 
  NCSSYSM_DIR      dir; 
  
  if(gl_sysmon.exist_marker == SM_EXIST_MARKER)
  {
    watch = gl_sysmon.ipra_watches;
    dir = (NCSSYSM_DIR)arg;
    for(;watch != NULL;watch = watch->next)
    { 
      evt.i_dir = dir;
      evt.i_opaque = watch->opaque;
      evt.i_usr_key = watch->usr_key;
      evt.i_vrtr_id = gl_sysmon.vrtr_id;
      
      (*(watch->cb_fnc))(&evt);
    }
  }
#endif
}

void cpu_watch_task( void*  unused)
{
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
  uns32          curr_bkt;
  uns32          curr_cpu_val;
  uns32          last_cpu_val;
  int            i = 0;
  int            limit;
  SM_COND_FNC    cond_fnc;
  SM_IT_FNC      it_fnc;
  NCSSYSM_DIR     dir;
  NCSSYSM_CPU_EVT evt;

  while (1)
  {
    m_NCS_TASK_SLEEP(gl_sysmon.cpu_wrr == 0 ? 1000 : gl_sysmon.cpu_wrr);

   curr_cpu_val = m_NCS_CUR_CPU_USAGE;

   if(curr_cpu_val == gl_sysmon.cpu_last)
   {
     /* no changes... */
     continue;
   }

   last_cpu_val = gl_sysmon.cpu_last; 

   curr_bkt = curr_cpu_val / (100 / SM_BKT_CNT);
   if(curr_bkt == SM_BKT_CNT)
     curr_bkt--;

   if(curr_bkt >= gl_sysmon.cpu_bkt_last)
   {
     limit = curr_bkt;
     i = gl_sysmon.cpu_bkt_last;
     it_fnc = sm_it_fnc_inc;
     cond_fnc = sm_cond_fnc_le;

     if(curr_bkt == gl_sysmon.cpu_bkt_last)
     {
       if(curr_cpu_val > gl_sysmon.cpu_last)
       {
         dir = NCSSYSM_DIR_UP;
       }
       else
       {
         dir = NCSSYSM_DIR_DOWN;
       }
     }
     else
     {
       dir = NCSSYSM_DIR_UP;
     }
   }
   else
   {
     i = gl_sysmon.cpu_bkt_last;
     limit = curr_bkt;
     it_fnc = sm_it_fnc_dec;
     cond_fnc = sm_cond_fnc_ge;
     dir = NCSSYSM_DIR_DOWN;
   }

   for(;(*cond_fnc)(i,limit);(*it_fnc)(&i))
   {
     SM_CPU_WATCH* watch = gl_sysmon.cpu_watches[i];

     if(((uns32)i == gl_sysmon.cpu_bkt_last) || ((uns32)i == curr_bkt))
     {
       for(;watch != NULL;watch = watch->next)
       {
         if(sm_cb_cond_fnc(watch->percent,
                           last_cpu_val,
                           curr_cpu_val,
                           dir) == TRUE)
         {
           evt.i_dir = dir;
           evt.i_opaque = watch->opaque;
           evt.i_subscr_pcnt = watch->percent;
           evt.i_cur_pcnt = curr_cpu_val;
           evt.i_usr_key = watch->usr_key;
           evt.i_vrtr_id = gl_sysmon.vrtr_id;
           
           (*(watch->cb_fnc))(&evt);
         }
       }
     }
     else
     {
       for(;watch != NULL;watch = watch->next)
       {
         evt.i_dir = dir;
         evt.i_opaque = watch->opaque;
         evt.i_subscr_pcnt = watch->percent;
         evt.i_cur_pcnt = curr_cpu_val;
         evt.i_usr_key = watch->usr_key;
         evt.i_vrtr_id = gl_sysmon.vrtr_id;
         
         (*(watch->cb_fnc))(&evt);
       }
     }
   }

   gl_sysmon.cpu_last = curr_cpu_val;
   gl_sysmon.cpu_bkt_last = curr_bkt;
  }
#endif
}

/*****************************************************************************

  PROCEDURE NAME:   ncssysm_lm

  DESCRIPTION:
     .

  RETURNS:
    SUCCESS     - Success.
    FAILURE     - Something went wrong.

*****************************************************************************/

uns32 ncssysm_lm(NCSSYSM_LM_ARG* info)
{
  if(info == NULL)
    return NCSCC_RC_FAILURE;

  switch(info->i_op)
  {
  case NCSSYSM_LM_OP_INIT:
    return sm_init();
  case NCSSYSM_LM_OP_TERMINATE:
    return sm_terminate();
    break;
    
  case NCSSYSM_LM_OP_GET:
    return sm_get(&info->op.get);
  case NCSSYSM_LM_OP_SET:
    return sm_set(&info->op.set);
  case NCSSYSM_LM_OP_IGET:
    return sm_iget(&info->op.iget);
  case NCSSYSM_LM_OP_ISET:
    return sm_iset(&info->op.iset);
  case NCSSYSM_LM_OP_REG_LMT_CB:
    return sm_reg_lmt_cb(&info->op.reg_lmt_cb); 
  case NCSSYSM_LM_OP_MEM_STATS:
    return sm_mem_stats(&info->op.mem_stats);
  case NCSSYSM_LM_OP_CPU_STATS:
    return sm_cpu_stats(&info->op.cpu_stats);
  case NCSSYSM_LM_OP_BUF_STATS:
    return sm_buf_stats(&info->op.buf_stats);
  case NCSSYSM_LM_OP_TMR_STATS:
    return sm_tmr_stats(&info->op.tmr_stats);
  case NCSSYSM_LM_OP_TASK_STATS:
    return sm_task_stats(&info->op.task_stats);
  case NCSSYSM_LM_OP_IPC_STATS:
    return sm_ipc_stats(&info->op.ipc_stats);
  case NCSSYSM_LM_OP_LOCK_STATS:
    return sm_lock_stats(&info->op.lock_stats);
  case NCSSYSM_LM_OP_MEM_RPT_THUP:
    return sm_mem_rpt_thup(&info->op.mem_rpt_thup);
  case NCSSYSM_LM_OP_MEM_RPT_SSUP:
    return sm_mem_rpt_ssup(&info->op.mem_rpt_ssup);
  case NCSSYSM_LM_OP_MEM_RPT_WO:
    return sm_mem_rpt_wo(&info->op.mem_rpt_wo);
  case NCSSYSM_LM_OP_MEM_RPT_WOS:
    return sm_mem_rpt_wos(&info->op.mem_rpt_wos);    
  case NCSSYSM_LM_OP_BUF_RPT_WO:
    return sm_buf_rpt_wo(&info->op.buf_rpt_wo);
  case NCSSYSM_LM_OP_BUF_RPT_WOS:
    return sm_buf_rpt_wos(&info->op.buf_rpt_wos);

#if ((NCSSYSM_MEM_DBG_ENABLE == 1) && (NCSSYSM_STKTRACE == 1))
  case NCSSYSM_LM_OP_MEM_STK_ADD:
    return sm_stktrace_add(&info->op.mem_stk_add);
  case NCSSYSM_LM_OP_MEM_STK_FLUSH:
    return sm_stktrace_flush(&info->op.mem_stk_flush);
  case NCSSYSM_LM_OP_MEM_STK_RPT:
    return sm_stktrace_report(&info->op.mem_stk_rpt);
#endif

  case NCSSYSM_LM_OP_IPC_RPT_LBGN:
     return m_NCS_SYSM_LM_OP_IPC_LBGN(&info->op.ipc_rpt_lat_bgn);
  case NCSSYSM_LM_OP_IPC_RPT_LTCY:
     return m_NCS_SYSM_LM_OP_IPC_LTCY(&info->op.ipc_rpt_lat);

  default:
    return NCSCC_RC_FAILURE;
    
  }

   return NCSCC_RC_SUCCESS;
}


uns32 gl_sysm_created = 0;

uns32 sm_init()
{
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
  uns32 i;
#endif

  gl_sysm_created++;
  if(gl_sysm_created > 1)
     return NCSCC_RC_SUCCESS;

  m_NCS_MEMSET(&gl_sysmon,0,sizeof(gl_sysmon));
  gl_sysmon.vrtr_id = 1; /* fix later */

#if (NCSSYSM_IPRA_WATCH_ENABLE == 1)
  gl_sysmon.exist_marker = SM_EXIST_MARKER;
  gl_sysmon.ipra_watches = NULL;
#endif

#if ((NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))
  gl_sysmon.mmgr = &mmgr;
  gl_sysmon.mem_ttl = 0xffffffff;
  gl_sysmon.mmgr->stats.max = gl_sysmon.mem_ttl;
#endif

#if (NCSSYSM_MEM_STATS_ENABLE == 1)
  gl_sysmon.mem_ignore = FALSE;
  gl_sysmon.mem_pulse.cb_fnc = sm_pulse_tmr_fnc;
  gl_sysmon.mem_pulse.data = 0;
  gl_sysmon.mem_pulse.period = 60 * SYSF_TMR_TICKS;
  gl_sysmon.mem_pulse.type = SMPT_MEM;
  m_NCS_TMR_CREATE(gl_sysmon.mem_pulse.tmr_id,
                  gl_sysmon.mem_pulse.period,
                  gl_sysmon.mem_pulse.cb_fnc,
                  (void*)&gl_sysmon.mem_pulse);
  m_NCS_TMR_START (gl_sysmon.mem_pulse.tmr_id,
                  gl_sysmon.mem_pulse.period,
                  gl_sysmon.mem_pulse.cb_fnc,
                  (void*)&gl_sysmon.mem_pulse);
#endif


#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
  gl_sysmon.ub_pool_mgr = &gl_ub_pool_mgr;
  for(i = 0;i < UB_MAX_POOLS;i++)
  {
    gl_sysmon.buf_ttl[i] = 0xffffffff;
    gl_sysmon.ub_pool_mgr->pools[i].stats.max = gl_sysmon.buf_ttl[i];
  }
#endif

#if (NCSSYSM_BUF_STATS_ENABLE == 1)
  for(i = 0;i < UB_MAX_POOLS;i++)
  {
    gl_sysmon.buf_ignore[i] = FALSE;
  gl_sysmon.buf_pulse[i].cb_fnc = sm_pulse_tmr_fnc;
  gl_sysmon.buf_pulse[i].data = i;
  gl_sysmon.buf_pulse[i].period = 60 * SYSF_TMR_TICKS;
  gl_sysmon.buf_pulse[i].type = SMPT_BUF;
  m_NCS_TMR_CREATE(gl_sysmon.buf_pulse[i].tmr_id,
                  gl_sysmon.buf_pulse[i].period,
                  gl_sysmon.buf_pulse[i].cb_fnc,
                  (void*)&gl_sysmon.buf_pulse[i]);
  m_NCS_TMR_START (gl_sysmon.buf_pulse[i].tmr_id,
                  gl_sysmon.buf_pulse[i].period,
                  gl_sysmon.buf_pulse[i].cb_fnc,
                  (void*)&gl_sysmon.buf_pulse[i]);

  }
#endif


#if ((NCSSYSM_IPC_WATCH_ENABLE == 1) || (NCSSYSM_IPC_STATS_ENABLE == 1))
  gl_sysmon.ipc_ttl = 0xffffffff;
#endif

#if (NCSSYSM_IPC_STATS_ENABLE == 1)
  gl_sysmon.ipc_ignore = FALSE;
#endif


#if (NCSSYSM_TMR_STATS_ENABLE == 1)
  gl_sysmon.tmr_ignore = FALSE;
#endif


  /* Create SYSMON MAIN TASK and MBX */
  m_NCS_STRCPY(gl_sysmon.sysmon_task_name,"SYSMON_TASK");

  if (m_NCS_IPC_CREATE(&gl_sysmon.sysmon_mbx) != NCSCC_RC_SUCCESS)
    return NCSCC_RC_FAILURE;

   m_NCS_IPC_ATTACH_EXT(&gl_sysmon.sysmon_mbx,gl_sysmon.sysmon_task_name);

  if (m_NCS_TASK_CREATE ((NCS_OS_CB)sysmon_task,
      &gl_sysmon.sysmon_mbx,
      gl_sysmon.sysmon_task_name,
      0,
      NCS_STACKSIZE_HUGE,
      &gl_sysmon.sysmon_task_hdl) != NCSCC_RC_SUCCESS)
    {
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
      return NCSCC_RC_FAILURE;
    }

  if (m_NCS_TASK_START (gl_sysmon.sysmon_task_hdl) != NCSCC_RC_SUCCESS)
    {
      m_NCS_TASK_RELEASE(gl_sysmon.sysmon_task_hdl);
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
      return NCSCC_RC_FAILURE;
    }

#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
  /* Create SYSMON MEM WATCH TASK */
  if (m_NCS_TASK_CREATE ((NCS_OS_CB)mem_watch_task,
      NULL,
      "SM_MEM_WATCH",
      0,
      NCS_STACKSIZE_HUGE,
      &gl_sysmon.sm_mem_watch_task_hdl) != NCSCC_RC_SUCCESS)
    {
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
      m_NCS_TASK_RELEASE(gl_sysmon.sysmon_task_hdl);
      return NCSCC_RC_FAILURE;
    }

  if (m_NCS_TASK_START (gl_sysmon.sm_mem_watch_task_hdl) != NCSCC_RC_SUCCESS)
    {
      m_NCS_TASK_RELEASE(gl_sysmon.sysmon_task_hdl);
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
      m_NCS_TASK_RELEASE(gl_sysmon.sm_mem_watch_task_hdl);
      return NCSCC_RC_FAILURE;
    }
#endif

#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
  m_NCS_INIT_CPU_MON;

  /* Create SYSMON CPU WATCH TASK */
  if (m_NCS_TASK_CREATE ((NCS_OS_CB)cpu_watch_task,
      NULL,
      "SM_CPU_WATCH",
      0,
      NCS_STACKSIZE_HUGE,
      &gl_sysmon.sm_cpu_watch_task_hdl) != NCSCC_RC_SUCCESS)
    {
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
      m_NCS_TASK_RELEASE(gl_sysmon.sysmon_task_hdl);
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_mem_watch_task_hdl);
#endif
      return NCSCC_RC_FAILURE;
    }

  if (m_NCS_TASK_START (gl_sysmon.sm_cpu_watch_task_hdl) != NCSCC_RC_SUCCESS)
    {
      m_NCS_TASK_RELEASE(gl_sysmon.sysmon_task_hdl);
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_mem_watch_task_hdl);
#endif
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_cpu_watch_task_hdl);
#endif
      return NCSCC_RC_FAILURE;
    }
#endif

#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
  /* Create SYSMON BUF WATCH TASK */
  if (m_NCS_TASK_CREATE ((NCS_OS_CB)buf_watch_task,
      NULL,
      "SM_BUF_WATCH",
      0,
      NCS_STACKSIZE_HUGE,
      &gl_sysmon.sm_buf_watch_task_hdl) != NCSCC_RC_SUCCESS)
    {
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
      m_NCS_TASK_RELEASE(gl_sysmon.sysmon_task_hdl);
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_mem_watch_task_hdl);
#endif
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_cpu_watch_task_hdl);
#endif
      return NCSCC_RC_FAILURE;
    }

  if (m_NCS_TASK_START (gl_sysmon.sm_buf_watch_task_hdl) != NCSCC_RC_SUCCESS)
    {
      m_NCS_TASK_RELEASE(gl_sysmon.sysmon_task_hdl);
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_mem_watch_task_hdl);
#endif
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_buf_watch_task_hdl);
#endif
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_cpu_watch_task_hdl);
#endif
      return NCSCC_RC_FAILURE;
    }
#endif

#if (NCSSYSM_IPC_WATCH_ENABLE == 1)
  /* Create SYSMON IPC WATCH TASK */
  if (m_NCS_TASK_CREATE ((NCS_OS_CB)ipc_watch_task,
      NULL,
      "SM_IPC_WATCH",
      0,
      NCS_STACKSIZE_HUGE,
      &gl_sysmon.sm_ipc_watch_task_hdl) != NCSCC_RC_SUCCESS)
    {
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
      m_NCS_TASK_RELEASE(gl_sysmon.sysmon_task_hdl);
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_mem_watch_task_hdl);
#endif
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_buf_watch_task_hdl);
#endif
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_cpu_watch_task_hdl);
#endif
      return NCSCC_RC_FAILURE;
    }

  if (m_NCS_TASK_START (gl_sysmon.sm_ipc_watch_task_hdl) != NCSCC_RC_SUCCESS)
    {
      m_NCS_TASK_RELEASE(gl_sysmon.sysmon_task_hdl);
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_mem_watch_task_hdl);
#endif
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_buf_watch_task_hdl);
#endif
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_cpu_watch_task_hdl);
#endif
      m_NCS_TASK_RELEASE(gl_sysmon.sm_ipc_watch_task_hdl);
      return NCSCC_RC_FAILURE;
    }
#endif

  return NCSCC_RC_SUCCESS;
}

uns32 sm_terminate()
{
      m_NCS_TASK_RELEASE(gl_sysmon.sysmon_task_hdl);
      m_NCS_IPC_RELEASE(&gl_sysmon.sysmon_mbx, NULL);
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_mem_watch_task_hdl);
#endif
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_buf_watch_task_hdl);
#endif
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
      m_NCS_SHUTDOWN_CPU_MON;
      m_NCS_TASK_RELEASE(gl_sysmon.sm_cpu_watch_task_hdl);
#endif
#if (NCSSYSM_IPC_WATCH_ENABLE == 1)
      m_NCS_TASK_RELEASE(gl_sysmon.sm_ipc_watch_task_hdl);
#endif
  return NCSCC_RC_SUCCESS;
}

uns32 sm_reg_lmt_cb(NCSSYSM_REG_LMT_CB* info)
{
  gl_sysmon.res_lmt_cb = info->i_lmt_cb_fnc;
  return NCSCC_RC_SUCCESS;
}


uns32 sm_iget(NCSSYSM_IGET* info)
{

  switch(info->i_obj_id)
  {
  case NCSSYSM_OID_BUF_TTL:
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
    info->o_obj_val = gl_sysmon.buf_ttl[info->i_inst_id];
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_BUF_CB_IGNORE:
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
    info->o_obj_val = (uns32)gl_sysmon.buf_ignore[info->i_inst_id];
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_MEM_SS_IGNORE:
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
#if 0 /* IR00059629 */
 if (info->i_inst_id < (uns32)NCS_SERVICE_ID_MAX)
 {
   info->o_obj_val = ignore_subsystem[info->i_inst_id];
 }
 else
#endif
   info->o_obj_val = 0;

#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_BUF_PULSE:
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
    info->o_obj_val = gl_sysmon.buf_pulse[info->i_inst_id].period / SYSF_TMR_TICKS;
#else
    info->o_obj_val = 0;
#endif
    break;
  default:
    return NCSCC_RC_FAILURE;
  }

  return NCSCC_RC_SUCCESS;
}


uns32 sm_get(NCSSYSM_GET* info)
{

  switch(info->i_obj_id)
  {
  case NCSSYSM_OID_MEM_TTL:
#if ((NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))
    info->o_obj_val = gl_sysmon.mem_ttl;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_TMR_TTL:
#if ((NCSSYSM_TMR_WATCH_ENABLE == 1) || (NCSSYSM_TMR_STATS_ENABLE == 1))
    info->o_obj_val = gl_sysmon.tmr_ttl;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_IPC_TTL:
#if ((NCSSYSM_IPC_WATCH_ENABLE == 1) || (NCSSYSM_IPC_STATS_ENABLE == 1))
    info->o_obj_val = gl_sysmon.ipc_ttl;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_MEM_CM_IGNORE:
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
     info->o_obj_val = (uns32)gl_sysmon.mem_ignore;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_TMR_IGNORE:
#if (NCSSYSM_TMR_STATS_ENABLE == 1)
    info->o_obj_val = (uns32)gl_sysmon.tmr_ignore;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_IPC_IGNORE:
#if (NCSSYSM_IPC_STATS_ENABLE == 1)
    info->o_obj_val = (uns32)gl_sysmon.ipc_ignore;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_MEM_PULSE:
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
    info->o_obj_val = gl_sysmon.mem_pulse.period / SYSF_TMR_TICKS;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_MEM_WRR:
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
    info->o_obj_val = gl_sysmon.mem_wrr;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_CPU_WRR:
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
    info->o_obj_val = gl_sysmon.cpu_wrr;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_BUF_WRR:
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
    info->o_obj_val = gl_sysmon.buf_wrr;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_TMR_WRR:
#if (NCSSYSM_TMR_WATCH_ENABLE == 1)
    info->o_obj_val = gl_sysmon.tmr_wrr;
#else
    info->o_obj_val = 0;
#endif
    break;
  case NCSSYSM_OID_IPC_WRR:
#if (NCSSYSM_IPC_WATCH_ENABLE == 1)
    info->o_obj_val = gl_sysmon.ipc_wrr;
#else
    info->o_obj_val = 0;
#endif
    break;

  default:
    return NCSCC_RC_FAILURE;
  }

  return NCSCC_RC_SUCCESS;
}

uns32 sm_iset(NCSSYSM_ISET* info)
{

  switch(info->i_obj_id)
  {
  case NCSSYSM_OID_BUF_TTL:
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
    gl_sysmon.buf_bkt_size[info->i_inst_id] = info->i_obj_val / SM_BKT_CNT;
#endif
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
    gl_sysmon.buf_ttl[info->i_inst_id] = info->i_obj_val;
    gl_sysmon.ub_pool_mgr->pools[info->i_inst_id].stats.max = gl_sysmon.buf_ttl[info->i_inst_id];
#endif
    break;
  case NCSSYSM_OID_BUF_CB_IGNORE:
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
    gl_sysmon.buf_ignore[info->i_inst_id] = (NCS_BOOL)info->i_obj_val;

    m_PMGR_LK(&gl_ub_pool_mgr.lock);
    if(gl_sysmon.ub_pool_mgr->pools[info->i_inst_id].mem_ignore == NULL)
    {
      m_PMGR_UNLK(&gl_ub_pool_mgr.lock);
      break;
    }
    gl_sysmon.ub_pool_mgr->pools[info->i_inst_id].mem_ignore((NCS_BOOL)info->i_obj_val);

    m_PMGR_UNLK(&gl_ub_pool_mgr.lock);

#endif
    break;
  case NCSSYSM_OID_MEM_SS_IGNORE:
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
    gl_sysmon.mem_ss_ignore = info->i_obj_val;
#if 0 /* IR00059629 */
    if (info->i_inst_id < (uns32)NCS_SERVICE_ID_MAX)
    {
      ignore_subsystem[info->i_inst_id] = info->i_obj_val;
    }
    else
      return NCSCC_RC_FAILURE;
#endif
#endif
    break;
  case NCSSYSM_OID_BUF_PULSE:
    {
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
      SM_PULSE* pulse = &gl_sysmon.buf_pulse[info->i_inst_id];
      pulse->period = info->i_obj_val * SYSF_TMR_TICKS;
#endif
    }
    break;
  default:
    return NCSCC_RC_FAILURE;
  }

  return NCSCC_RC_SUCCESS;
}


uns32 sm_set(NCSSYSM_SET* info)
   {

  switch(info->i_obj_id)
  {
  case NCSSYSM_OID_MEM_TTL:
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
    {
    gl_sysmon.mem_bkt_size= info->i_obj_val / SM_BKT_CNT;
#if 0
    {
    float old_bkt_size = (float)(gl_sysmon.mem_bkt_size);
    if (gl_sysmon.mem_bkt_size)
        gl_sysmon.mem_bkt_last = gl_sysmon.mem_bkt_last * ((float)old_bkt_size / (float)gl_sysmon.mem_bkt_size);
    }
#endif
    }
#endif
#if ((NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))
    gl_sysmon.mem_ttl = info->i_obj_val;
    gl_sysmon.mmgr->stats.max = gl_sysmon.mem_ttl;
#endif
    break;
  case NCSSYSM_OID_TMR_TTL:
#if ((NCSSYSM_TMR_WATCH_ENABLE == 1) || (NCSSYSM_TMR_STATS_ENABLE == 1))
    gl_sysmon.tmr_ttl = info->i_obj_val;
#endif
    break;
  case NCSSYSM_OID_IPC_TTL:
    {
#if ((NCSSYSM_IPC_WATCH_ENABLE == 1) || (NCSSYSM_IPC_STATS_ENABLE == 1))
      NCS_IPC_STATS* ipc_stats;
      gl_sysmon.ipc_bkt_size = info->i_obj_val / SM_BKT_CNT;
      gl_sysmon.ipc_ttl = info->i_obj_val;

      for(ipc_stats = gl_sysmon.ipc_stats; ipc_stats != NULL;ipc_stats = ipc_stats->next)
      {
        ipc_stats->max_depth = gl_sysmon.ipc_ttl;
      }
#endif
    }
    break;
  case NCSSYSM_OID_MEM_CM_IGNORE:
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
    {
    NCS_MPOOL        *mp;
    NCS_MPOOL_ENTRY  *me;
    
    gl_sysmon.mem_ignore = (NCS_BOOL)info->i_obj_val;
    
    for (mp = gl_sysmon.mmgr->mpools; ; mp++)
    {
      me = mp->inuse;
      m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE);
      while (me != NULL)
      {
        me->ignore = gl_sysmon.mem_ignore;
        me = me->next;
      }
      /* This conditional is moved out of the for loop so that
      the last pool (size 0) is included
      */
      m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE);
      if(mp->size == 0) break;
    }
    }
#endif
    break;
  case NCSSYSM_OID_TMR_IGNORE:
#if (NCSSYSM_TMR_STATS_ENABLE == 1)
    gl_sysmon.tmr_ignore = (NCS_BOOL)info->i_obj_val;
    if(gl_sysmon.tmr_ignore == TRUE)
    {
      /* reset report info */
    }
#endif
    break;
  case NCSSYSM_OID_IPC_IGNORE:
#if (NCSSYSM_IPC_STATS_ENABLE == 1)
    gl_sysmon.ipc_ignore = (NCS_BOOL)info->i_obj_val;
    if(gl_sysmon.ipc_ignore == TRUE)
    {
      /* reset report info */
    }
#endif
    break;
  case NCSSYSM_OID_MEM_PULSE:
    {
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
      SM_PULSE* pulse = &gl_sysmon.mem_pulse;
      pulse->period = info->i_obj_val * SYSF_TMR_TICKS;
#endif
    }
    break;
  case NCSSYSM_OID_MEM_WRR:
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
    gl_sysmon.mem_wrr = info->i_obj_val;
#endif
    break;
  case NCSSYSM_OID_CPU_WRR:
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
    gl_sysmon.cpu_wrr = info->i_obj_val;
#endif
    break;
  case NCSSYSM_OID_BUF_WRR:
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
    gl_sysmon.buf_wrr = info->i_obj_val;
#endif
    break;
  case NCSSYSM_OID_TMR_WRR:
#if (NCSSYSM_TMR_WATCH_ENABLE == 1)
    gl_sysmon.tmr_wrr = info->i_obj_val;
#endif
    break;
  case NCSSYSM_OID_IPC_WRR:
#if (NCSSYSM_IPC_WATCH_ENABLE == 1)
    gl_sysmon.ipc_wrr = info->i_obj_val;
#endif
    break;
  default:
    return NCSCC_RC_FAILURE;
  }

  return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncssysm_ss

  DESCRIPTION:
     .

  RETURNS:
    SUCCESS     - Success.
    FAILURE     - Something went wrong.

*****************************************************************************/

uns32 ncssysm_ss(NCSSYSM_SS_ARG* info)
{
  /* use info->i_vrtr_id to find the sysmon inst */
  NCS_SYSMON* sysmon = &gl_sysmon;

  switch(info->i_op)
  {
  case NCSSYSM_SS_OP_MEM_REG:
    return sm_mem_watch_reg(sysmon, info->i_ss_key,&info->op.reg.i_memory);
  case NCSSYSM_SS_OP_BUF_REG:
    return sm_buf_watch_reg(sysmon,info->i_ss_key,&info->op.reg.i_buffer);
  case NCSSYSM_SS_OP_TMR_REG:
    return sm_tmr_watch_reg(sysmon,info->i_ss_key,&info->op.reg.i_timer);
  case NCSSYSM_SS_OP_IPC_REG:
    return sm_ipc_watch_reg(sysmon,info->i_ss_key,&info->op.reg.i_queue);
  case NCSSYSM_SS_OP_CPU_REG:
    return sm_cpu_watch_reg(sysmon, info->i_ss_key,&info->op.reg.i_cpu);
  case NCSSYSM_SS_OP_IPRA_REG:
    return sm_ipra_watch_reg(sysmon, info->i_ss_key,&info->op.reg.i_ipra);
  case NCSSYSM_SS_OP_MEM_UNREG:
    return sm_mem_watch_unreg(sysmon,&info->op.unreg.i_memory);
  case NCSSYSM_SS_OP_BUF_UNREG:
    return sm_buf_watch_unreg(sysmon,&info->op.unreg.i_buffer);
  case NCSSYSM_SS_OP_TMR_UNREG:
    return sm_tmr_watch_unreg(sysmon,&info->op.unreg.i_timer);
  case NCSSYSM_SS_OP_IPC_UNREG:
    return sm_ipc_watch_unreg(sysmon,&info->op.unreg.i_queue);  
  case NCSSYSM_SS_OP_CPU_UNREG:
    return sm_cpu_watch_unreg(sysmon,&info->op.unreg.i_cpu);    
  case NCSSYSM_SS_OP_IPRA_UNREG:
    return sm_ipra_watch_unreg(sysmon,&info->op.unreg.i_ipra); 
  default:
    return NCSCC_RC_FAILURE;
  }
   return NCSCC_RC_SUCCESS;
}


uns32 sm_ipra_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_IPRA_REG* info)
{
#if (NCSSYSM_IPRA_WATCH_ENABLE == 1)
  SM_IPRA_WATCH* iwatch = NULL;

  iwatch = m_MMGR_ALLOC_SM_IPRA_WATCH;
  if(iwatch == NULL)
    return NCSCC_RC_FAILURE;

  if(usr_key == NULL)
  {
    m_SET_HDL_KEY((iwatch->usr_key),0,0,0);
  }
  else
    iwatch->usr_key = *usr_key;

  iwatch->cb_fnc = info->i_cb_func;
  iwatch->opaque = info->i_opaque;

  iwatch->next = sysmon->ipra_watches;
  sysmon->ipra_watches = iwatch;

  info->o_hdl = (NCSCONTEXT)iwatch;
#else
  info->o_hdl = (NCSCONTEXT)NULL;
#endif

  return NCSCC_RC_SUCCESS;
}

uns32 sm_cpu_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_CPU_REG* info)
{
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
  SM_CPU_WATCH* cwatch = NULL;
  uns32         bucket;

  cwatch = m_MMGR_ALLOC_SM_CPU_WATCH;
  if(cwatch == NULL)
    return NCSCC_RC_FAILURE;

  if(usr_key == NULL)
  {
    m_SET_HDL_KEY((cwatch->usr_key),0,0,0);
  }
  else
    cwatch->usr_key = *usr_key;
  cwatch->cb_fnc = info->i_cb_func;
  cwatch->opaque = info->i_opaque;
  cwatch->percent = info->i_percent;

  bucket = cwatch->percent / (100 / SM_BKT_CNT);
  if(bucket == SM_BKT_CNT)
    bucket--;

  cwatch->next = sysmon->cpu_watches[bucket];
  sysmon->cpu_watches[bucket] = cwatch;

  info->o_hdl = (NCSCONTEXT)cwatch;
#else
  info->o_hdl = (NCSCONTEXT)NULL;
#endif

  return NCSCC_RC_SUCCESS;
}

uns32 sm_mem_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_MEM_REG* info)
{
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
  SM_MEM_WATCH* mwatch = NULL;
  uns32         bucket;

  mwatch = m_MMGR_ALLOC_SM_MEM_WATCH;
  if(mwatch == NULL)
    return NCSCC_RC_FAILURE;

  if(usr_key == NULL)
  {
    m_SET_HDL_KEY((mwatch->usr_key),0,0,0);
  }
  else
    mwatch->usr_key = *usr_key;
  mwatch->cb_fnc = info->i_cb_func;
  mwatch->opaque = info->i_opaque;
  mwatch->percent = info->i_percent;

  bucket = mwatch->percent / (100 / SM_BKT_CNT);
  if(bucket == SM_BKT_CNT)
    bucket--;

  mwatch->next = sysmon->mem_watches[bucket];
  sysmon->mem_watches[bucket] = mwatch;

  info->o_hdl = (NCSCONTEXT)mwatch;
#else
  info->o_hdl = (NCSCONTEXT)NULL;
#endif

  return NCSCC_RC_SUCCESS;
}


uns32 sm_buf_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_BUF_REG* info)
{
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
  SM_BUF_WATCH* bwatch = NULL;
  uns32         bucket;

  bwatch = m_MMGR_ALLOC_SM_BUF_WATCH;
  if(bwatch == NULL)
    return NCSCC_RC_FAILURE;

  if(usr_key == NULL)
  {
    m_SET_HDL_KEY((bwatch->usr_key),0,0,0);
  }
  else
    bwatch->usr_key = *usr_key;
  bwatch->cb_fnc = info->i_cb_func;
  bwatch->opaque = info->i_opaque;
  bwatch->percent = info->i_percent;
  bwatch->pool_id = info->i_pool_id;

  bucket = bwatch->percent / (100 / SM_BKT_CNT);
  if(bucket == SM_BKT_CNT)
    bucket--;

  bwatch->next = sysmon->buf_watches[bucket][bwatch->pool_id];
  sysmon->buf_watches[bucket][bwatch->pool_id] = bwatch;

  info->o_hdl = (NCSCONTEXT)bwatch;
#else
  info->o_hdl = (NCSCONTEXT)NULL;
#endif

  return NCSCC_RC_SUCCESS;
}

uns32 sm_tmr_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_TMR_REG* info)
{
  return NCSCC_RC_SUCCESS;
}

uns32 sm_ipc_watch_reg(NCS_SYSMON* sysmon,NCS_KEY* usr_key,NCSSYSM_IPC_REG* info)
{
#if (NCSSYSM_IPC_WATCH_ENABLE == 1)
  SM_IPC_WATCH* iwatch = NULL;
  uns32         bucket;
  NCS_IPC_STATS* ipc_stats = NULL;

  if(info->i_name == NULL)
    return NCSCC_RC_FAILURE;

  for(ipc_stats = sysmon->ipc_stats;ipc_stats != NULL;ipc_stats = ipc_stats->next)
  {
    if(m_NCS_STRCMP((const char* )info->i_name,ipc_stats->resource->name) == 0)
      break;
  }

  if(ipc_stats == NULL)
    return NCSCC_RC_FAILURE;

  iwatch = m_MMGR_ALLOC_SM_IPC_WATCH;
  if(iwatch == NULL)
    return NCSCC_RC_FAILURE;

  if(usr_key == NULL)
  {
    m_SET_HDL_KEY((iwatch->usr_key),0,0,0);
  }
  else
    iwatch->usr_key = *usr_key;
  iwatch->cb_fnc = info->i_cb_func;
  iwatch->opaque = info->i_opaque;
  iwatch->percent = info->i_percent;

  iwatch->ipcq = ipc_stats;

  bucket = iwatch->percent / (100 / SM_BKT_CNT);
  if(bucket == SM_BKT_CNT)
    bucket--;

  iwatch->next = sysmon->ipc_watches[bucket];
  sysmon->ipc_watches[bucket] = iwatch;

  info->o_hdl = (NCSCONTEXT)iwatch;
#else
  info->o_hdl = (NCSCONTEXT)NULL;
#endif

  return NCSCC_RC_SUCCESS;
}


uns32 sm_ipra_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_IPRA_UNREG* info)
{
#if (NCSSYSM_IPRA_WATCH_ENABLE == 1)
  SM_IPRA_WATCH* iwatch = NULL;
  SM_IPRA_WATCH* iwatch_it = NULL;

  if(info->i_hdl == NULL)
    return NCSCC_RC_FAILURE;

  iwatch = (SM_IPRA_WATCH*)info->i_hdl;
  iwatch_it = sysmon->ipra_watches;


  if(iwatch_it == iwatch)
  {
    sysmon->ipra_watches = iwatch_it->next;
    m_MMGR_FREE_SM_IPRA_WATCH(iwatch);
    return NCSCC_RC_SUCCESS;
  }
  for(;iwatch_it->next != NULL; iwatch_it = iwatch_it->next)
  {
    if(iwatch_it->next == iwatch)
    {
      iwatch_it->next = iwatch->next;
      m_MMGR_FREE_SM_IPRA_WATCH(iwatch);
      return NCSCC_RC_SUCCESS;
    }
  }

  return NCSCC_RC_FAILURE;
#else
  return NCSCC_RC_SUCCESS;
#endif
}


uns32 sm_cpu_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_CPU_UNREG* info)
{
#if (NCSSYSM_CPU_WATCH_ENABLE == 1)
  SM_CPU_WATCH* cwatch = NULL;
  SM_CPU_WATCH* cw_list = NULL;
  uns32         bucket;

  if(info->i_hdl == NULL)
    return NCSCC_RC_FAILURE;

  cwatch = (SM_CPU_WATCH*)info->i_hdl;
  if(cwatch->percent > 100)
    return NCSCC_RC_FAILURE;

  bucket = cwatch->percent / (100 / SM_BKT_CNT);
  if(bucket == SM_BKT_CNT)
    bucket--;

  cw_list = sysmon->cpu_watches[bucket];
  if(cw_list == cwatch)
  {
    sysmon->cpu_watches[bucket] = cw_list->next;
    m_MMGR_FREE_SM_CPU_WATCH(cwatch);
    return NCSCC_RC_SUCCESS;
  }
  for(;cw_list->next != NULL; cw_list = cw_list->next)
  {
    if(cw_list->next == cwatch)
    {
      cw_list->next = cwatch->next;
      m_MMGR_FREE_SM_CPU_WATCH(cwatch);
      return NCSCC_RC_SUCCESS;
    }
  }

  return NCSCC_RC_FAILURE;
#else
  return NCSCC_RC_SUCCESS;
#endif
}

uns32 sm_mem_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_MEM_UNREG* info)
{
#if (NCSSYSM_MEM_WATCH_ENABLE == 1)
  SM_MEM_WATCH* mwatch = NULL;
  SM_MEM_WATCH* mw_list = NULL;
  uns32         bucket;

  if(info->i_hdl == NULL)
    return NCSCC_RC_FAILURE;

  mwatch = (SM_MEM_WATCH*)info->i_hdl;
  if(mwatch->percent > 100)
    return NCSCC_RC_FAILURE;

  bucket = mwatch->percent / (100 / SM_BKT_CNT);
  if(bucket == SM_BKT_CNT)
    bucket--;

  mw_list = sysmon->mem_watches[bucket];
  if(mw_list == mwatch)
  {
    sysmon->mem_watches[bucket] = mw_list->next;
    m_MMGR_FREE_SM_MEM_WATCH(mwatch);
    return NCSCC_RC_SUCCESS;
  }
  for(;mw_list->next != NULL; mw_list = mw_list->next)
  {
    if(mw_list->next == mwatch)
    {
      mw_list->next = mwatch->next;
      m_MMGR_FREE_SM_MEM_WATCH(mwatch);
      return NCSCC_RC_SUCCESS;
    }
  }

  return NCSCC_RC_FAILURE;
#else
  return NCSCC_RC_SUCCESS;
#endif
}



uns32 sm_buf_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_BUF_UNREG* info)
{
#if (NCSSYSM_BUF_WATCH_ENABLE == 1)
  SM_BUF_WATCH* bwatch = NULL;
  SM_BUF_WATCH* bw_list = NULL;
  uns32         bucket;

  if(info->i_hdl == NULL) 
    return NCSCC_RC_FAILURE;

  bwatch = (SM_BUF_WATCH*)info->i_hdl;
  if(bwatch->percent > 100)
    return NCSCC_RC_FAILURE;

  bucket = bwatch->percent / (100 / SM_BKT_CNT);
  if(bucket == SM_BKT_CNT)
    bucket--;

  if(bwatch->pool_id > UB_MAX_POOLS)
    return NCSCC_RC_FAILURE;

  bw_list = sysmon->buf_watches[bucket][bwatch->pool_id];
  if(bw_list == bwatch)
  {
    sysmon->buf_watches[bucket][bwatch->pool_id] = bw_list->next;
    m_MMGR_FREE_SM_BUF_WATCH(bwatch);
    return NCSCC_RC_SUCCESS;
  }
  for(;bw_list->next != NULL; bw_list = bw_list->next)
  {
    if(bw_list->next == bwatch)
    {
      bw_list->next = bwatch->next;
      m_MMGR_FREE_SM_BUF_WATCH(bwatch);
      return NCSCC_RC_SUCCESS;
    }
  }

  return NCSCC_RC_FAILURE;
#else
  return NCSCC_RC_SUCCESS;
#endif
}

uns32 sm_tmr_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_TMR_UNREG* info)
{
  return NCSCC_RC_SUCCESS;
}

uns32 sm_ipc_watch_unreg(NCS_SYSMON* sysmon,NCSSYSM_IPC_UNREG* info)
{
#if (NCSSYSM_IPC_WATCH_ENABLE == 1)
  SM_IPC_WATCH* iwatch = NULL;
  SM_IPC_WATCH* iw_list = NULL;
  uns32         bucket;

  if(info->i_hdl == NULL)
    return NCSCC_RC_FAILURE;

  iwatch = (SM_IPC_WATCH*)info->i_hdl;
  if(iwatch->percent > 100)
    return NCSCC_RC_FAILURE;

  bucket = iwatch->percent / (100 / SM_BKT_CNT);
  if(bucket == SM_BKT_CNT)
    bucket--;
    
  iw_list = sysmon->ipc_watches[bucket];
  if(iw_list == iwatch)
  {
    sysmon->ipc_watches[bucket] = iw_list->next;
    m_MMGR_FREE_SM_IPC_WATCH(iwatch);
    return NCSCC_RC_SUCCESS;
  }
  for(;iw_list->next != NULL; iw_list = iw_list->next)
  {
    if(iw_list->next == iwatch)
    {
      iw_list->next = iwatch->next;
      m_MMGR_FREE_SM_IPC_WATCH(iwatch);
      return NCSCC_RC_SUCCESS;
    }
  }
#endif
  return NCSCC_RC_SUCCESS;
}


uns32 sm_cpu_stats(NCSSYSM_CPU_STATS* info)
{
#if (NCSSYSM_CPU_STATS_ENABLE == 1)
  NCS_BOOL         to_std = FALSE;
  NCS_BOOL         to_file = FALSE;
  char            output_string[128];
  char            asc_tod[33];
  time_t          tod;

  info->o_curr = m_NCS_CUR_CPU_USAGE;

  to_std = info->i_opp.opp_bits & OPP_TO_STDOUT ? TRUE : FALSE;
  to_file = info->i_opp.opp_bits & OPP_TO_FILE ? TRUE : FALSE;

  if((to_std == TRUE) || (to_file == TRUE))
  {
    FILE* fh = NULL;
    
    if(to_file == TRUE)
    {  
      if(info->i_opp.fname != NULL)
      {
        if(m_NCS_STRLEN(info->i_opp.fname) != 0) 
        {
          fh = sysf_fopen( info->i_opp.fname, "at");
          if(fh == NULL) 
          {
            m_NCS_CONS_PRINTF("Unable to open %s\n", info->i_opp.fname);
            return NCSCC_RC_FAILURE;
          }
        }
        else
        {
          return NCSCC_RC_FAILURE;
        }
      }
    }

    
    asc_tod[0] = '\0';
    m_GET_ASCII_TIME_STAMP(tod, asc_tod);
    sysf_sprintf(output_string, "%s\n", asc_tod);
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    
    
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|   C P U  U T I L I Z A T I O N  S T A T S  |\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "       Current %%: %d\n",m_NCS_CUR_CPU_USAGE);
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);


    if (((to_file == TRUE)) && fh) 
    {
      sysf_fclose(fh);
    }

  }

  if(info->i_opp.opp_bits & OPP_TO_MASTER)
  {
    /* TBD */
  }
 
#else

  info->o_curr = 0;

#endif

  return NCSCC_RC_SUCCESS;
}


uns32 sm_mem_stats(NCSSYSM_MEM_STATS* info)
{
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
  NCS_BOOL         to_std = FALSE;
  NCS_BOOL         to_file = FALSE;
  char            output_string[128];
  char            asc_tod[33];
  time_t          tod;

/* SMM global lock is gone.. we are doing these operations without any
   guarentee that no thread is writing while this thread is reading...*/

  info->o_ttl = gl_sysmon.mem_ttl;
  info->o_hwm = gl_sysmon.mmgr->stats.hwm;
  info->o_new = gl_sysmon.mmgr->stats.alloced / 1000;
  info->o_free = gl_sysmon.mmgr->stats.freed / 1000;
  info->o_curr = gl_sysmon.mmgr->stats.curr;

  to_std = info->i_opp.opp_bits & OPP_TO_STDOUT ? TRUE : FALSE;
  to_file = info->i_opp.opp_bits & OPP_TO_FILE ? TRUE : FALSE;

  if((to_std == TRUE) || (to_file == TRUE))
  {
    FILE* fh = NULL;
    
    if(to_file == TRUE)
    {  
      if(info->i_opp.fname != NULL)
      {
        if(m_NCS_STRLEN(info->i_opp.fname) != 0) 
        {
          fh = sysf_fopen( info->i_opp.fname, "at");
          if(fh == NULL) 
          {
            m_NCS_CONS_PRINTF("Unable to open %s\n", info->i_opp.fname);
            return NCSCC_RC_FAILURE;
          }
        }
        else
        {
          return NCSCC_RC_FAILURE;
        }
      }
    }

    
    asc_tod[0] = '\0';
    m_GET_ASCII_TIME_STAMP(tod, asc_tod);
    sysf_sprintf(output_string, "%s\n", asc_tod);
    fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
      
      sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|     M E M     S T A T S     T O T A L S    |\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|   ttl  |  curr  | alloced| freed  |  hwm   |\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "| avail  | alloced| so far | so far |in bytes|\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "%9d%9d%9d%9d%9d\n",info->o_ttl,
        info->o_curr,
        gl_sysmon.mmgr->stats.alloced,
        gl_sysmon.mmgr->stats.freed,
        info->o_hwm);
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      

      if (((to_file == TRUE)) && fh) 
      {
        sysf_fclose(fh);
      }
      
  }

  if(info->i_opp.opp_bits & OPP_TO_MASTER)
  {
    /* TBD */
  }

#else

  info->o_ttl = 0;
  info->o_hwm = 0;
  info->o_new = 0;
  info->o_free = 0;
  info->o_curr = 0;

#endif

  return NCSCC_RC_SUCCESS;
}

uns32 sm_buf_stats(NCSSYSM_BUF_STATS* info)
{
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
  NCS_BOOL         to_std = FALSE;
  NCS_BOOL         to_file = FALSE;
  char            output_string[128];
  char            asc_tod[33];
  time_t          tod;

  if(info->i_pool_id > UB_MAX_POOLS)
    return NCSCC_RC_FAILURE;

  m_PMGR_LK(&gl_ub_pool_mgr.lock);

  info->o_ttl = gl_sysmon.buf_ttl[info->i_pool_id];
  info->o_new = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.alloced;
  info->o_free = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.freed;
  info->o_curr = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.curr;
  info->o_hwm = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.hwm;
  info->o_cnt = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.cnt;

  info->o_fc_1 = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.fc_1;
  info->o_fc_2 = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.fc_2;
  info->o_fc_3 = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.fc_3;
  info->o_fc_4 = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.fc_4;
  info->o_fc_gt = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.fc_gt;

  info->o_32b = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.s32b;
  info->o_64b = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.s64b;
  info->o_128b = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.s128b;
  info->o_256b = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.s256b;
  info->o_512b = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.s512b;
  info->o_1024b = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.s1024b;
  info->o_2048b = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.s2048b;
  info->o_big_b = gl_sysmon.ub_pool_mgr->pools[info->i_pool_id].stats.sbig_b;

  m_PMGR_UNLK(&gl_ub_pool_mgr.lock);

  to_std = info->i_opp.opp_bits & OPP_TO_STDOUT ? TRUE : FALSE;
  to_file = info->i_opp.opp_bits & OPP_TO_FILE ? TRUE : FALSE;


  if((to_std == TRUE) || (to_file == TRUE))
  {
    FILE* fh = NULL;
    
    if(to_file == TRUE)
    {  
      if(info->i_opp.fname != NULL)
      {
        if(m_NCS_STRLEN(info->i_opp.fname) != 0) 
        {
          fh = sysf_fopen( info->i_opp.fname, "at");
          if(fh == NULL) 
          {
            m_NCS_CONS_PRINTF("Unable to open %s\n", info->i_opp.fname);
            return NCSCC_RC_FAILURE;
          }
        }
        else
        {
          return NCSCC_RC_FAILURE;
        }
      }
    }
    

    asc_tod[0] = '\0';
    m_GET_ASCII_TIME_STAMP(tod, asc_tod);
    sysf_sprintf(output_string, "%s\n", asc_tod);
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    
    
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|             B U F  P O O L   S T A T S              |\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|                P O O L  I D : %d           |\n",    
                 info->i_pool_id);
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|   cnt  |   curr |  hwm   |  new   |  free  |  max   |\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    
    sysf_sprintf(output_string, "%9d%9d%9d%9d%9d%9d\n",
      info->o_cnt,
      info->o_curr,
      info->o_hwm,
      info->o_new,
      info->o_free,
      info->o_ttl);

    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);

    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|    F R E E   T I M E   S T A T S :                  |\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);

    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|    N U M B E R  O F  B U F F E R S  C H A I N E D   |\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);

    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "| 1 buf  | 2 bufs | 3 bufs | 4 bufs |5 / more|\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);

    sysf_sprintf(output_string, "%9d%9d%9d%9d%9d\n",
      info->o_fc_1,
      info->o_fc_2,
      info->o_fc_3,
      info->o_fc_4,
      info->o_fc_gt
      );

    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);

    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|                P A Y L O A D   S I Z E              |\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);

    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|< 32 b  |< 64 b  |< 128 b |< 256 b |< 512 b |< 1024 b|< 2048 b|> 2048 b|\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);

    sysf_sprintf(output_string, "%9d%9d%9d%9d%9d%9d%9d%9d\n",
      info->o_32b,
      info->o_64b,
      info->o_128b,
      info->o_256b,
      info->o_512b,
      info->o_1024b,
      info->o_2048b,
      info->o_big_b 
      );
    
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
          
      
      if (((to_file == TRUE)) && fh) 
      {
        sysf_fclose(fh);
      }
      
  }

  if(info->i_opp.opp_bits & OPP_TO_MASTER)
  {
    /* TBD */
  }

  info->o_new /= 1000; 
  info->o_free /= 1000;

#else

  info->o_ttl = 0;
  info->o_new = 0;
  info->o_free = 0;
  info->o_curr = 0;
  info->o_hwm = 0;
  info->o_cnt = 0;

#endif

  return NCSCC_RC_SUCCESS;
}

uns32 sm_ipc_stats(NCSSYSM_IPC_STATS* info)
{
#if (NCSSYSM_IPC_STATS_ENABLE == 1)
  NCS_BOOL         to_std = FALSE;
  NCS_BOOL         to_file = FALSE;
  char            output_string[128];
  char            asc_tod[33];
  time_t          tod;
  NCS_IPC_STATS*   ipc_stats = NULL;

  if(info->i_name == NULL)
    return NCSCC_RC_FAILURE;
  
  for(ipc_stats = gl_sysmon.ipc_stats;ipc_stats != NULL;ipc_stats = ipc_stats->next)
  {
    if(m_NCS_STRCMP(info->i_name,ipc_stats->resource->name) == 0)
      break;
  }
  
  if(ipc_stats == NULL)
    return NCSCC_RC_FAILURE;
  
  info->o_curr = ipc_stats->curr_depth;
  info->o_hwm = ipc_stats->hwm;
  info->o_off_q = ipc_stats->deqed;
  info->o_on_q = ipc_stats->enqed;
  info->o_ttl = ipc_stats->max_depth;
  
  to_std = info->i_opp.opp_bits & OPP_TO_STDOUT ? TRUE : FALSE;
  to_file = info->i_opp.opp_bits & OPP_TO_FILE ? TRUE : FALSE;
  
  if((to_std == TRUE) || (to_file == TRUE))
  {
    FILE* fh = NULL;
    
    if(to_file == TRUE)
    {  
      if(info->i_opp.fname != NULL)
      {
        if(m_NCS_STRLEN(info->i_opp.fname) != 0) 
        {
          fh = sysf_fopen( info->i_opp.fname, "at");
          if(fh == NULL) 
          {
            m_NCS_CONS_PRINTF("Unable to open %s\n", info->i_opp.fname);
            return NCSCC_RC_FAILURE;
          }
        }
        else
        {
          return NCSCC_RC_FAILURE;
        }
      }
    }
    
    asc_tod[0] = '\0';
    m_GET_ASCII_TIME_STAMP(tod, asc_tod);
    sysf_sprintf(output_string, "%s\n", asc_tod);
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    
    
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|         I P C  Q U E U E  S T A T S        |\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|   M B X : %9x  , N A M E :  |%9s|\n",
      (uns32)ipc_stats->resource,ipc_stats->resource->name);
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|   curr |  hwm   |  on_q  |  off_q |  max   |\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    
    sysf_sprintf(output_string, "%9d%9d%9d%9d%9d\n",
      info->o_curr,
      info->o_hwm,
      info->o_on_q,
      info->o_off_q,
      info->o_ttl);
    
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    
    if (((to_file == TRUE)) && fh) 
    {
      sysf_fclose(fh);
    }
    
  }
  
  if(info->i_opp.opp_bits & OPP_TO_MASTER)
  {
    /* TBD */
  }

#else

  info->o_curr = 0;
  info->o_hwm = 0;
  info->o_off_q = 0;
  info->o_on_q = 0;
  info->o_ttl = 0;

#endif

  return NCSCC_RC_SUCCESS;
}

uns32 sm_tmr_stats(NCSSYSM_TMR_STATS* info)
{

  return NCSCC_RC_SUCCESS;
}

uns32 sm_task_stats(NCSSYSM_TASK_STATS* info)
{

  return NCSCC_RC_SUCCESS;
}

uns32 sm_lock_stats(NCSSYSM_LOCK_STATS* info)
{

  return NCSCC_RC_SUCCESS;
}


uns32 sm_mem_rpt_thup(NCSSYSM_MEM_RPT_THUP*  info)
{
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
  NCS_BOOL         to_std = FALSE;
  NCS_BOOL         to_file = FALSE;
  char            output_string[128];
  char            asc_tod[33];
  time_t          tod;
  uns32 i;
  uns32           bkt_cnt = NCSSYSM_MEM_BKT_CNT <= (MMGR_NUM_POOLS + 1) ? NCSSYSM_MEM_BKT_CNT : (MMGR_NUM_POOLS + 1);

  m_NCS_MEMSET(info->o_bkt,0,sizeof(info->o_bkt));

  to_std = info->i_opp.opp_bits & OPP_TO_STDOUT ? TRUE : FALSE;
  to_file = info->i_opp.opp_bits & OPP_TO_FILE ? TRUE : FALSE;

  if((to_std == TRUE) || (to_file == TRUE))
  {
    FILE* fh = NULL;
    
    if(to_file == TRUE)
    {  
      if(info->i_opp.fname != NULL)
      {
        if(m_NCS_STRLEN(info->i_opp.fname) != 0) 
        {
          fh = sysf_fopen( info->i_opp.fname, "at");
          if(fh == NULL) 
          {
            m_NCS_CONS_PRINTF("Unable to open %s\n", info->i_opp.fname);
            return NCSCC_RC_FAILURE;
          }
        }
        else
        {
          return NCSCC_RC_FAILURE;
        }
      }
    }

    
    asc_tod[0] = '\0';
    m_GET_ASCII_TIME_STAMP(tod, asc_tod);
    sysf_sprintf(output_string, "%s\n", asc_tod);
    fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
    
    /* SMM removed global _mpool_lock.. These pools should have their own lock!!?? */

    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|        |    M E M  P O O L S   R E P O R T    (all are raw counts)      |\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|   pool +--------+--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|   size |  in-use|   avail|  hwater|  allocs|   frees|  errors|  falloc|\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);
    sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+--------+--------+\n");
    if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
    if(to_file == TRUE) sysf_fprintf (fh, output_string);

      for (i = 0;i < bkt_cnt;i++)
      {
        info->o_bkt[i].o_bkt_size = gl_sysmon.mmgr->mpools[i].stat.size;
        info->o_bkt[i].o_inuse    = gl_sysmon.mmgr->mpools[i].stat.num_inuse;
        info->o_bkt[i].o_avail    = gl_sysmon.mmgr->mpools[i].stat.num_avail;
        info->o_bkt[i].o_hwm      = gl_sysmon.mmgr->mpools[i].stat.highwater;
        info->o_bkt[i].o_allocs   = gl_sysmon.mmgr->mpools[i].stat.allocs;
        info->o_bkt[i].o_frees    = gl_sysmon.mmgr->mpools[i].stat.frees;
        info->o_bkt[i].o_errors   = gl_sysmon.mmgr->mpools[i].stat.errors;
        info->o_bkt[i].o_null     = gl_sysmon.mmgr->mpools[i].stat.failed_alloc;

        sysf_sprintf(output_string, "%9d%9d%9d%9d%9d%9d%9d%9d\n",
          info->o_bkt[i].o_bkt_size, 
          info->o_bkt[i].o_inuse,
          info->o_bkt[i].o_avail,
          info->o_bkt[i].o_hwm,
          info->o_bkt[i].o_allocs,
          info->o_bkt[i].o_frees,
          info->o_bkt[i].o_errors,
          info->o_bkt[i].o_null);
          if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
          if(to_file == TRUE) sysf_fprintf (fh, output_string);
        
      }

      if (((to_file == TRUE)) && fh) 
      {
        sysf_fclose(fh);
      }   
  }

  if(info->i_opp.opp_bits & OPP_TO_MASTER)
  {
    /* TBD */
  }

#else

  m_NCS_MEMSET(info->o_bkt,0,sizeof(info->o_bkt));

#endif

  return NCSCC_RC_SUCCESS;
}



uns32 sm_mem_rpt_ssup(NCSSYSM_MEM_RPT_SSUP*  info)
   {
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
   NCS_MPOOL*       mp;
   NCS_MPOOL_ENTRY* me;
   NCS_BOOL         to_std = FALSE;
   NCS_BOOL         to_file = FALSE;
   char            output_string[128];
   char            asc_tod[33];
   time_t          tod;
   uns32           i;
   uns32           ss_id = 0;
   NCS_BOOL         ss_ignored = TRUE;
   NCS_BOOL         count_ignored = TRUE;
   uns32           ignored = 0;
   uns32           unignored = 0;
   uns32           bkt_cnt = NCSSYSM_MEM_BKT_CNT <= (MMGR_NUM_POOLS + 1) ? NCSSYSM_MEM_BKT_CNT : (MMGR_NUM_POOLS + 1);
   
   m_NCS_MEMSET(info->o_ssu,0,sizeof(info->o_ssu));
   
   to_std = info->i_opp.opp_bits & OPP_TO_STDOUT ? TRUE : FALSE;
   to_file = info->i_opp.opp_bits & OPP_TO_FILE ? TRUE : FALSE;
   
   if((to_std == TRUE) || (to_file == TRUE))
      {
      FILE* fh = NULL;
      
      if(to_file == TRUE)
         {  
         if(info->i_opp.fname != NULL)
            {
            if(m_NCS_STRLEN(info->i_opp.fname) != 0) 
               {
               fh = sysf_fopen( info->i_opp.fname, "at");
               if(fh == NULL) 
                  {
                  m_NCS_CONS_PRINTF("Unable to open %s\n", info->i_opp.fname);
                  return NCSCC_RC_FAILURE;
                  }
               }
            else
               {
               return NCSCC_RC_FAILURE;
               }
            }
         }
      
      
      asc_tod[0] = '\0';
      m_GET_ASCII_TIME_STAMP(tod, asc_tod);
      sysf_sprintf(output_string, "%s\n", asc_tod);
      fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
      
      
      sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "| S U B S Y S T E M S  M E M  P O O L S   R E P O R T |\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|------------+------------+------------+--------------+\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|  pool size |  inuse_uni |  inuse_ign |   inuse_ttl  |\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|------------+------------+------------+--------------+\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      
      ss_id = info->i_ss_id;
      count_ignored = (NCS_BOOL)info->i_ignored;
      
      #if 0 /* IR00059629 */
      ss_ignored = ignore_subsystem[ss_id] != 0 ? TRUE : FALSE;
      #endif
      
      for (i= 0,mp = mmgr.mpools;i < bkt_cnt; mp++,i++)
         {
         m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE);
         info->o_ssu[i].o_bkt_size = mp->stat.size;
         me = mp->inuse;
         while (me != NULL)
            {
            #if 0 /* IR00059629 */  
            if((me->ignore == TRUE) || (((uns32)me->service_id == ss_id) && (ss_ignored == TRUE)))
            #endif 
             
            if(me->ignore == TRUE)
               {
               if(count_ignored == TRUE)
                  ignored++;
               }
            else
               {
               unignored++;
               }
            me = me->next;
            }
         info->o_ssu[i].o_inuse_uni = unignored;
         info->o_ssu[i].o_inuse_ign = (count_ignored == TRUE ? ignored : 0);
         info->o_ssu[i].o_inuse_ttl = unignored + info->o_ssu[i].o_inuse_ign;
         
         
         sysf_sprintf(output_string, "%13d%13d%13d%15d\n",
                                     info->o_ssu[i].o_bkt_size, 
                                     info->o_ssu[i].o_inuse_uni,
                                     info->o_ssu[i].o_inuse_ign,
                                     info->o_ssu[i].o_inuse_ttl);
         
         if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
         if(to_file == TRUE) sysf_fprintf (fh, output_string);
         
         m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); 
         if(mp->size == 0) break;
         }
      
      
      if (((to_file == TRUE)) && fh) 
         {
         sysf_fclose(fh);
         }
      
      }
   
   if(info->i_opp.opp_bits & OPP_TO_MASTER)
      {
      /* TBD */
      }
   
#else
   
   m_NCS_MEMSET(info->o_ssu,0,sizeof(info->o_ssu));
   
#endif      
   
   return NCSCC_RC_SUCCESS;
}

uns32 sm_mem_rpt_wo  (NCSSYSM_MEM_RPT_WO*    info)
   {
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
   NCS_MPOOL*       mp;
   NCS_MPOOL_ENTRY* me;
   NCS_BOOL         to_std = FALSE;
   NCS_BOOL         to_file = FALSE;
   char            output_string[256];
   char            asc_tod[33];
   time_t          tod;
   uns32           j;
   uns32           ss_id = 0;
   NCS_BOOL         all_ss = FALSE;
   uns32           min_age = 0;
   
   m_NCS_MEMSET(info->o_wo,0,sizeof(info->o_wo));
   
   min_age = info->i_age;
   if(info->i_ss_id == 0)
      all_ss = TRUE;
   else
      ss_id = info->i_ss_id;
   
   to_std = info->i_opp.opp_bits & OPP_TO_STDOUT ? TRUE : FALSE;
   to_file = info->i_opp.opp_bits & OPP_TO_FILE ? TRUE : FALSE;
   
   if((to_std == TRUE) || (to_file == TRUE))
      {
      FILE* fh = NULL;
      
      if(to_file == TRUE)
         {  
         if(info->i_opp.fname != NULL)
            {
            if(m_NCS_STRLEN(info->i_opp.fname) != 0) 
               {
               fh = sysf_fopen( info->i_opp.fname, "at");
               if(fh == NULL) 
                  {
                  m_NCS_CONS_PRINTF("Unable to open %s\n", info->i_opp.fname);
                  return NCSCC_RC_FAILURE;
                  }
               }
            else
               {
               return NCSCC_RC_FAILURE;
               }
            }
         }
      
      
      asc_tod[0] = '\0';
      m_GET_ASCII_TIME_STAMP(tod, asc_tod);
      sysf_sprintf(output_string, "%s\n", asc_tod);
      fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
      
      
      sysf_sprintf(output_string, "|---|----+----+-----------+-----+-----------+--------+---+---+----------+----|\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|  N O N - I G N O R E D   O U T S T A N D I N G   M E M O R Y   R E P O R T |\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|---|----+----+-----------+-----+-----------+--------+---+---+----------+----|\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|  #| Age|Real|   alloc'ed|Owner|Ownr claims|  Real  |Svc|Sub|          |Bkt |\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|  #| Cnt|line|    in file| line|    in file|  size  | ID| ID| Ptr value|size|\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "|---|----+----+-----------+-----+-----------+--------+---+---+----------+----|\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      
      for (mp = gl_sysmon.mmgr->mpools;; mp++)
         {
         m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE);
         for(me = mp->inuse,j = 0;me != NULL;me = me->next)
            {
            if(me->ignore == TRUE)
               continue;
            
            if((all_ss == FALSE) && ((uns32)me->service_id != ss_id))
               continue;
            
            if(me->age < min_age)
               continue;
            
            sysf_sprintf (output_string, "%4d%5d%5d%12s%6d%12s%9d%4d%4d%11x%5d \n",
               j + 1,                                /* Nmber */
               me->age,                            /* age */
               me->line,                           /* Line  */
               ncs_fname(me->file),                 /* File  */
               me->loc_line,                       /* OwnrL */
               ncs_fname(me->loc_file),             /* OwnrF */
               me->real_size,                      /* Real Size  */
               me->service_id,                     /* SvcID */
               me->sub_id,                         /* SubID */
               (int)((char *)me + sizeof(NCS_MPOOL_ENTRY)),/* Ptr   */
               mp->size == 0 ? me->real_size : mp->size  /* Bucket Size  */);
            
            if(j < NCSSYSM_MEM_MAX_WO)
               {
               info->o_wo[j].o_aline = me->line;
               info->o_wo[j].o_afile = (uns8*)ncs_fname(me->file);
               info->o_wo[j].o_oline = me->loc_line;
               info->o_wo[j].o_ofile = (uns8*)ncs_fname(me->loc_file);
               info->o_wo[j].o_svcid = me->service_id;
               info->o_wo[j].o_subid = me->sub_id;
               info->o_wo[j].o_age = me->age;
               info->o_wo[j].o_ptr = (NCSCONTEXT)((char *)me + sizeof(NCS_MPOOL_ENTRY));
               info->o_wo[j].o_bkt_sz = mp->size;
               info->o_wo[j].o_act_sz = me->real_size;
               
               info->o_filled = j + 1;
               }
            
            j++;
            
            if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
            if(to_file == TRUE) sysf_fprintf (fh, output_string);
            
            }
         m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); 
         if(mp->size == 0) break;
         }
      
      if (((to_file == TRUE)) && fh) 
         {
         sysf_fclose(fh);
         }
      
  }
  
  if(info->i_opp.opp_bits & OPP_TO_MASTER)
     {
     /* TBD */
     }
  
#else
  
  info->o_filled = 0;
  m_NCS_MEMSET(info->o_wo,0,sizeof(info->o_wo));
  
#endif      
  
  return NCSCC_RC_SUCCESS;
}

/* A helper struct to keep track of our hits */

typedef struct ncs_mpool_hist
{
  struct ncs_mpool_hist* next;
  NCS_MPOOL_ENTRY*       me;
  uns32                 hit_cnt;

} NCS_MPOOL_HIST;

/* We will use memory from the stack of this big */

#define MMGR_HIST_SPACE 3000


uns32 sm_mem_rpt_wos (NCSSYSM_MEM_RPT_WOS*   info) 
   {
#if (NCSSYSM_MEM_STATS_ENABLE == 1)
   NCS_BOOL         found;
   NCSMEM_AID       ma;
   char            space[MMGR_HIST_SPACE];
   NCS_MPOOL_HIST*  hash[NCS_SERVICE_ID_MAX];
   NCS_MPOOL_HIST*  test;
   uns32           itr;
   
   NCS_MPOOL*       mp;
   NCS_MPOOL_ENTRY* me;
   NCS_BOOL         to_std = FALSE;
   NCS_BOOL         to_file = FALSE;
   char            output_string[128];
   char            asc_tod[33];
   time_t          tod;
   uns32           j;
   uns32           ss_id = 0;
   NCS_BOOL         all_ss = FALSE;
   uns32           min_age = 0;
   
   m_NCS_MEMSET(info->o_wos,0,sizeof(info->o_wos));
   
   ncsmem_aid_init (&ma, (uns8*)space, MMGR_HIST_SPACE);
   
   m_NCS_MEMSET (space, '\0', MMGR_HIST_SPACE);
   m_NCS_MEMSET (hash,  '\0', sizeof(hash));
   
   min_age = info->i_age;
   if(info->i_ss_id == 0)
      all_ss = TRUE;
   else
      ss_id = info->i_ss_id;
   
   to_std = info->i_opp.opp_bits & OPP_TO_STDOUT ? TRUE : FALSE;
   to_file = info->i_opp.opp_bits & OPP_TO_FILE ? TRUE : FALSE;
   
   if((to_std == TRUE) || (to_file == TRUE))
      {
      FILE* fh = NULL;
      
      if(to_file == TRUE)
         {  
         if(info->i_opp.fname != NULL)
            {
            if(m_NCS_STRLEN(info->i_opp.fname) != 0) 
               {
               fh = sysf_fopen( info->i_opp.fname, "at");
               if(fh == NULL) 
                  {
                  m_NCS_CONS_PRINTF("Unable to open %s\n", info->i_opp.fname);
                  return NCSCC_RC_FAILURE;
                  }
               }
            else
               {
               return NCSCC_RC_FAILURE;
               }
            }
         }
      
      for (mp = gl_sysmon.mmgr->mpools;; mp++)
         {   
         j = 0;
         m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE);
         for (me = mp->inuse;me != NULL;me = me->next)
            {
            if(me->ignore == TRUE)
               continue;
            if((all_ss == FALSE) && ((uns32)me->service_id != ss_id))
               continue;
            if(me->age < min_age)
               continue;
            
            found = FALSE;
            
            /** OK, next line confuses some; ptr to first ptr so I know there is always test->next 
            **/
            for (test = hash[me->service_id]; test != NULL; test = test->next)
               {
               if ( (test->me->line     == me->line)       && 
                  (test->me->service_id == me->service_id) &&
                  (test->me->sub_id     == me->sub_id)     && 
                  (test->me->age        == me->age)        &&
                  (m_NCS_STRCMP(me->file,test->me->file) == 0) )
                  {
                  test->hit_cnt++;
                  found = TRUE;
                  break;
                  }
               }
            
            if (found == FALSE)
               {
               if ((test = (NCS_MPOOL_HIST*)ncsmem_aid_alloc(&ma, sizeof(NCS_MPOOL_HIST))) == NULL)
                  continue; /* means we have maxed out on 'reports';  You must have enough !!.. */
               test->next           = hash[me->service_id];
               test->me             = me;
               hash[me->service_id] = test;
               test->hit_cnt++;
               }      
            }
         m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE);          
         if(mp->size == 0) break;
         }
      
      asc_tod[0] = '\0';
      m_GET_ASCII_TIME_STAMP(tod, asc_tod);
      sysf_sprintf(output_string, "%s\n", asc_tod);
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      
      
      sysf_sprintf(output_string, "%s","|------+-----+-------------+------+----+----+-----+-----|\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "%s","|         M E M  O U T   - S U M M A R Y          |\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "%s","|------+-----+-------------+------+----+----+-----+-----|\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "%s","|hit   |show |   alloc'ed  |Owner |Svc |Sub |real |bkt  |\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "%s","|Cnt   |Cnt  |    in file  | line | ID | ID |size |size |\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      sysf_sprintf(output_string, "%s","|------+-----+-------------+------+----+----+-----+-----|\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      
      
      j = 0;
      for (itr = 0; itr < NCS_SERVICE_ID_MAX; itr++)
         {
         for (test = hash[itr]; test != NULL; test = test->next)
            {
            sysf_sprintf(output_string, "|%6d|%5d|%13s|%6d|%4d|%4d|%5d|%5d|\n",
               test->hit_cnt,                   /* hits  */
               test->me->age,                   /* age */
               ncs_fname(test->me->file),        /* File  */
               test->me->line,                  /* Line  */
               test->me->service_id,            /* SvcID */
               test->me->sub_id,
               test->me->real_size, /* real size */
               test->me->pool->size /* bucket size */);
            
            if(j < NCSSYSM_MEM_MAX_WOS)
               {
               info->o_wos[j].o_inst = test->hit_cnt;
               info->o_wos[j].o_age = test->me->age;
               info->o_wos[j].o_aline = test->me->line;
               info->o_wos[j].o_afile = (uns8*)ncs_fname(test->me->file);
               info->o_wos[j].o_svcid = test->me->service_id;
               info->o_wos[j].o_subid = test->me->sub_id;
               info->o_wos[j].o_bkt_sz = test->me->pool->size;
               info->o_wos[j].o_act_sz = test->me->real_size;
               info->o_filled = j + 1;
               j++;
               }
            
            if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
            if(to_file == TRUE) sysf_fprintf (fh, output_string);
            }
         }
      
      sysf_sprintf(output_string,"|------+-----+-------------+------+----+----+-----+-----|\n");
      if(to_std == TRUE) m_NCS_CONS_PRINTF("%s", output_string);
      if(to_file == TRUE) sysf_fprintf (fh, output_string);
      
      if (((to_file == TRUE)) && fh) 
         {
         sysf_fclose(fh);
         }
      
  }
  
  if(info->i_opp.opp_bits & OPP_TO_MASTER)
     {
     /* TBD */
     }
  
#else
  
  info->o_filled = 0;
  m_NCS_MEMSET(info->o_wos,0,sizeof(info->o_wos));
  
#endif      
  
  return NCSCC_RC_SUCCESS;
}
 
uns32 sm_buf_rpt_wo  (NCSSYSM_BUF_RPT_WO*    info)
{
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
  NCSUB_POOL_RPT_ARG arg;
  uns32             pool_id = info->i_pool_id;

  if(pool_id > UB_MAX_POOLS)
    return NCSCC_RC_FAILURE;

  m_PMGR_LK(&gl_ub_pool_mgr.lock);

  if(gl_ub_pool_mgr.pools[pool_id].mem_rpt == NULL)
    return NCSCC_RC_FAILURE;

  arg.op = NCSUB_POOL_RPT_OP_WO;
  arg.info.wo = info;

  gl_ub_pool_mgr.pools[pool_id].mem_rpt(&arg);

  m_PMGR_UNLK(&gl_ub_pool_mgr.lock);

#else

  info->o_filled = 0;
  m_NCS_MEMSET(info->o_wo,0,sizeof(info->o_wo));

#endif

  return NCSCC_RC_SUCCESS;
}

uns32 sm_buf_rpt_wos (NCSSYSM_BUF_RPT_WOS*   info) 
{
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
  NCSUB_POOL_RPT_ARG arg;
  uns32             pool_id = info->i_pool_id;

  if(pool_id > UB_MAX_POOLS)
    return NCSCC_RC_FAILURE;

  m_PMGR_LK(&gl_ub_pool_mgr.lock);

  if(gl_ub_pool_mgr.pools[pool_id].mem_rpt == NULL)
  {
    m_PMGR_UNLK(&gl_ub_pool_mgr.lock);
    return NCSCC_RC_FAILURE;
  }

  arg.op = NCSUB_POOL_RPT_OP_WOS;
  arg.info.wos = info;

  gl_ub_pool_mgr.pools[pool_id].mem_rpt(&arg);

  m_PMGR_UNLK(&gl_ub_pool_mgr.lock);

#else

  info->o_filled = 0;
  m_NCS_MEMSET(info->o_wos,0,sizeof(info->o_wos));

#endif

  return NCSCC_RC_SUCCESS;
}


#if ((NCSSYSM_MEM_DBG_ENABLE == 1) && (NCSSYSM_STKTRACE == 1))
uns32 sm_stktrace_add(NCSSYSM_MEM_STK_ADD * info)
{
    uns32 index;

    for(index=0; index<NCSSYSM_STKTRACE_INFO_MAX; index++)
    {
        if(NULL == mmgr.stktrace_info[index].ste)
        {
            m_NCS_STRCPY(mmgr.stktrace_info[index].file, info->i_file);
            mmgr.stktrace_info[index].line = info->i_line;
            index++;
            break;
        }
    }

    return NCSCC_RC_SUCCESS;
}

uns32 sm_stktrace_flush(NCSSYSM_MEM_STK_FLUSH * info)
{
    uns32 index;

    USE(info);

    for(index=0; index<NCSSYSM_STKTRACE_INFO_MAX; index++)
    {
        NCS_OS_STKTRACE_ENTRY *ste;
        int count = 0;

        ste = mmgr.stktrace_info[index].ste;
        if(NULL != ste)
        {
            do
            {
                NCS_OS_STKTRACE_ENTRY *next = ste->next;
                m_NCS_OS_MEMFREE(ste, NCS_MEM_REGION_PERSISTENT);
                ste = next;
                count++;
            }while(NULL != ste);
            m_NCS_STRCPY(mmgr.stktrace_info[index].file, "");
            mmgr.stktrace_info[index].line = 0;
            mmgr.stktrace_info[index].ste = NULL;
        }
    }

    return NCSCC_RC_SUCCESS;
}

uns32 sm_stktrace_report(NCSSYSM_MEM_STK_RPT* info)
{
    uns32 index;
    char *pfile;
    uns32 maxlen = info->io_rsltsize;
    uns32 loutlen;
    uns8 *outstr = info->io_results;
    NCS_OS_STKTRACE_ENTRY *ste;
    uns32 count = info->io_marker;
    uns32 ticks_now = m_NCS_OS_GET_TIME_MS;
    uns32 ticks_diff;

    pfile = strrchr(info->i_file, '\\');
    if(NULL == pfile)
    {
        pfile = info->i_file;
    }
    else
    {
        pfile++;
    }

    loutlen = 0;

                /*+-------------------------------------------------------+*/
                /*|         MEM_ALLOC  STACK TRACE                        |*/
                /*|----------------------------------+--------------------|*/
                /*| Watched:                         | Ticks:             |*/
                /*|   Line:           657     |   Min:             0      |*/
                /*|   File:    sysmdemo.c     |   Max:            -1      |*/
                /*+----------------------------------+--------------------+*/

    sysf_sprintf((char *)outstr+loutlen,
                 "+-------------------------------------------------------+\n"\
                 "|                 MEM ALLOC  STACK TRACE                |\n"\
                 "|---------------------------+---------------------------|\n"\
                 "| Watched                   | Age Range                 |\n");
    loutlen = m_NCS_STRLEN((const char *)outstr);

    sysf_sprintf((char *)outstr+loutlen,
                 "|   Line: %13d     |   Min: %13d      |\n"\
                 "|   File: %13s     |   Max: %13d      |\n"\
                 "+---------------------------+---------------------------+\n",
                 info->i_line, info->i_ticks_min,
                 info->i_file, info->i_ticks_max);
    loutlen = m_NCS_STRLEN((const char *)outstr);

    for(index=0; index<NCSSYSM_STKTRACE_INFO_MAX; index++)
    {
       if (info->i_line == mmgr.stktrace_info[index].line)
       {
           if(0 == m_NCS_STRCMP(pfile, mmgr.stktrace_info[index].file))
           {
               /* if this is the first of the get/next cycle, then
                  clear the 'fetched' flag for each stktrace entry */
               if(0 == info->io_marker)
               {
                   info->o_records = 0;
                   ste = mmgr.stktrace_info[index].ste;
                   if(NULL != ste)
                   {
                       do
                       {
                           ste->flags = 0;
    
                           ste = ste->next;
    
                       } while(NULL != ste);
                   }
               }

               ste = mmgr.stktrace_info[index].ste;
               if(NULL != ste)
               {
                   do
                   {
                       ticks_diff = ticks_now - ste->ticks;
                       if( (info->i_ticks_min < ticks_diff)   /* ticks less than ticks since alloced */
                          && (ticks_diff < info->i_ticks_max) /* ticks greater than tick count since alloced */
                          && ((maxlen-loutlen-55) > (ste->addr_count*80))  /* still room in destination buffer */
                          && 0 == ste->flags)                           /* hasn't been fetched this get/next */
                       {
                           info->o_records++;
                           sysf_sprintf((char *)outstr+loutlen, "| Allocation: %d (ste=0x%08X)\tTicks: %d\n", info->o_records, (uns32)ste, ticks_diff);
                           loutlen = m_NCS_STRLEN((const char *)outstr);
                           m_NCS_OS_STACKTRACE_EXPAND(ste, outstr+loutlen, &loutlen);
                           loutlen = m_NCS_STRLEN((const char *)outstr);
                           m_NCS_ASSERT(maxlen > loutlen);
                           ste->flags = 1;

                           if(NULL == ste->next)
                           {
                               info->o_more = FALSE;
                           }
                           else
                           {
                               info->o_more = TRUE;
                           }
                       }

                       ste = ste->next;
                       count++;

                   } while(NULL != ste);
                   /* send the cont back to the app, so we can skip
                      the previous fetched stktrace entry */
                   info->io_marker = count;
               }
           }
       }
    }

    sysf_sprintf((char *)outstr+loutlen, "%s",
                 "+-------------------------------------------------------+\n");

    info->io_rsltsize = loutlen;

    return NCSCC_RC_SUCCESS;
}


#endif  /* NCSSYSM_STKTRACE == 1  && NCSSYSM_MEM_DBG_ENABLE == 1 */

#endif /* NCS_USE_SYSMON == 1 */

