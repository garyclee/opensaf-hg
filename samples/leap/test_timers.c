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
 */


/********************************************************************

********************************************************************/

#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncssysf_tsk.h>
#include <opensaf/ncssysf_tmr.h>
#include <opensaf/ncssysfpool.h>
#include "leaptest.h"


static void
timer_accuracy_cb_func(void * arg)
{
   * ((uns64 *) arg) = m_NCS_GET_TIME_MS;
}


static void
timer_accuracy_test(void)
{ /* set timers, one at a time (let each one expire before testing next value)
   for all values between 100 ms to 5 minutes (300,000 ms),
   at 100 ms increments, and compare accuracy */

   tmr_t  timer;
   uns64  period, start, expired, diff;
   float p;

   printf("\nTimer Accuracy Test.\n");

   srand (m_NCS_GET_TIME_MS); /* seed the generator used to keep out of sync with timer service */

   m_NCS_TMR_CREATE (timer, 0, 0, (void *) 0);

   for (period = 0; period <= 300000; )
   {
       expired = 0;
#if NCS_HAVE_FLOATINGPOINT
       printf ("Measuring : %5.1f : Seconds...   ", (float) period/1000);
#else
       printf ("Measuring %d Seconds...   ", period/1000);
#endif
      m_NCS_TASK_SLEEP (rand() % 100); /* keep out of sync with timer service tick */
      start  = m_NCS_GET_TIME_MS;
      m_NCS_TMR_START (timer, period/10, timer_accuracy_cb_func, (void *) &expired);


      while (expired == 0)      /* wait until timer has expired time is recorded */
         m_NCS_TASK_SLEEP (10);

      diff = expired - start;
            
      p = ( period ? ((float) diff * 100 / period) : 100 );
#if NCS_HAVE_FLOATINGPOINT
      printf ("Deviation: (ms) = %3lld, (%%) = %4.1f\n", diff-period, p-100);
#else
      printf ("Deviation: (ms) = %3lld, (%%) = %d\n", diff-period, p-100);
#endif
      if (period < 1000)       /* less than 1 second ? */
         period += 100;        /*        inc by 100 ms */
      else if (period < 10000) /* less than 10 seconds ? */
         period += 1000;       /*        inc by 1 second */
      else if (period < 60000) /* less than 1 minute ? */
         period += 10000;      /*    inc by 10 seconds */
      else period += 60000;    /* else inc by 1 minute */
   }

   m_NCS_TMR_DESTROY (timer);

}

struct test_tmr {
   struct test_tmr *next;
   struct test_tmr *prev;
   tmr_t  timer;
   uns32  period;
   uns64  start_time;
   uns64  expire_time;
};




static volatile uns32 total_timers;

static void
timer_ringload_cb_func(void * arg)
{
   * ((uns64 *) arg) = m_NCS_GET_TIME_MS;
   total_timers--;
}

static uns32
timer_ringload_test(void)
{ /* load the entire ring with at least 10,000 timers, to an even depth at each index
  and a "random within range of ring depth" lap counter
  (i.e. if each index has 10 timers, the laps are of values from 0-9) 
   and compare accuracy */

   struct test_tmr *anchor = NULL, **tp, *ttmr = NULL;
   uns32 ticklength, ringsize, testdepth, rdepth, rindex, lap_ms;



   printf("\nTimer Ringload Test.\n");

   printf("..test assumes timer service ticks are 100ms/tick and ringsize is 1024..\n");
   ticklength = 100;  /* each tick is 100 ms */
   ringsize   = 1024;
   lap_ms = ringsize * ticklength; /* time to get around ring once */

   testdepth  = 10;   /* select ring index depth to test */


   srand (m_NCS_GET_TIME_MS); /* seed the generator used for picking laps */
   total_timers = 0;
   tp = &anchor;
   for (rdepth = 0; rdepth < testdepth; rdepth++)    /* go around ring 'testdepth' times */
   {
      for (rindex = 1; rindex <= ringsize; rindex++) /* step through each ring index */
      {
         if ((ttmr = m_NCS_MEM_ALLOC (sizeof (struct test_tmr), 0, 0, 0)) == NULL)
         {
            printf ("Failed to allocate memory for test_tmr structure.\n");
            return NCSCC_RC_FAILURE;
         }
         m_NCS_TMR_CREATE (ttmr->timer, 0, 0, (void *) 0);
         if (ttmr->timer == NULL)
         {
            printf ("Failed to create timer.\n");
            return NCSCC_RC_FAILURE;
         }
         ttmr->expire_time = 0;
               /* use rand # of laps, add to current index */
         ttmr->period = (((rand() % testdepth) * lap_ms) + (rindex * ticklength));
         if (tp == &anchor)
            ttmr->prev = NULL;
         else
            ttmr->prev = *tp;
         *tp = ttmr;
         tp = &ttmr->next;
         total_timers++;
      }
   }
   if (ttmr != NULL)
      ttmr->next = NULL;


   ttmr = anchor;
   while (ttmr != NULL)
   {
      ttmr->start_time  = m_NCS_GET_TIME_MS;
      m_NCS_TMR_START (ttmr->timer, ttmr->period / 10, timer_ringload_cb_func, (void *) &ttmr->expire_time);
      ttmr = ttmr->next;
   }

   while (total_timers != 0)
   {
      printf (".%d.", total_timers);
      m_NCS_TASK_SLEEP (10000);
   }
   printf ("\n");

   ttmr = anchor;
   while (ttmr != NULL)
   {
      struct test_tmr *temp;
      float percent;
      uns64 diff;

      diff = ttmr->expire_time - ttmr->start_time;
      percent = (float) diff * 100 / ttmr->period;

      printf ("Results for  %5.1f Seconds.   ", (float) ttmr->period/1000);
      printf ("Deviation: (ms) = %3lld, (%%) = %4.1f\n", diff-ttmr->period, percent-100);

      m_NCS_TMR_DESTROY (ttmr->timer);
      temp = ttmr;
      ttmr = ttmr->next;
      m_NCS_MEM_FREE (temp, 0, 0, 0);
   }


   return NCSCC_RC_SUCCESS;
}

struct test_tmr2 {
   tmr_t  timer;
   uns32  period;
   uns64  start_time;
   uns64  expire_time;
};

struct stop_exp_test_cb_info {
   uns64  *expire_time;
   tmr_t  *timer_to_delete; 
};

static void
timer_stop_on_expire_cb_func(void * arg)
{
   struct stop_exp_test_cb_info *info = arg;

   *(info->expire_time) = m_NCS_GET_TIME_MS;
   m_NCS_TMR_STOP (*(info->timer_to_delete)); 
}

static uns32
timer_stoptimer_test(void)
{ /* Eight parts: (understands the timer service implementation)
  stop a timer that is at front  of list at an index within ring,
  stop a timer that is at middle of list at an index within ring,
  stop a timer that is at end    of list at an index within ring,
  stop a timer that is    alone on  list at an index within ring,
  stop a timer that is at front  of expiring list,
  stop a timer that is at middle of expiring list,
  stop a timer that is at end    of expiring list,
  stop a timer that is    alone on  expiring list,
  and compare accuracy of other timers */

#define TMR_MAX       4 /* some test need 4 timers, some 3, some 1 */
#define TMR_PERIOD 5000 /* 5 seconds in ms  - enough time to stop before callback */

   struct test_tmr2 tmr_info[TMR_MAX];
   unsigned int tmrcnt, test_cntr;
   float percent;
   uns64 diff;

   printf("\nTimer StopTimer Test.\n");

   /* init tmr_info array */
   for (tmrcnt = 0; tmrcnt < TMR_MAX; tmrcnt++)
   {
      m_NCS_TMR_CREATE (tmr_info[tmrcnt].timer, 0, 0, (void *) 0);
      if (tmr_info[tmrcnt].timer == NULL)
      {
         printf ("Failed to create timer.\n");
         return NCSCC_RC_FAILURE;
      }
      tmr_info[tmrcnt].expire_time = 0;
      tmr_info[tmrcnt].period   = TMR_PERIOD;
   }

   /* perform 3 stop tests: 1) stop front, 2) stop middle, 3) stop end */
   for (test_cntr = 0; test_cntr < 3; test_cntr++)
   {
      /* start 3 timers at one index */
      for (tmrcnt = 0; tmrcnt < 3; tmrcnt++)
      {
         tmr_info[tmrcnt].expire_time = 0;
         tmr_info[tmrcnt].start_time  = m_NCS_GET_TIME_MS;
         m_NCS_TMR_START (tmr_info[tmrcnt].timer, tmr_info[tmrcnt].period / 10, timer_accuracy_cb_func, (void *) &tmr_info[tmrcnt].expire_time);
      }

      m_NCS_TASK_SLEEP (1000); /* get away from current timer service tick */
      m_NCS_TMR_STOP (tmr_info[test_cntr].timer);

      m_NCS_TASK_SLEEP (TMR_PERIOD + 1000); /* wait for all timers to expire */


      /* check results for each timer */
      for (tmrcnt = 0; tmrcnt < 3; tmrcnt++)
      {
         printf ("Timer %d: ", tmrcnt);
         if (tmr_info[tmrcnt].expire_time == 0) /* was there a call back ? */
         {
            if (tmrcnt != test_cntr)            /* is it not the timer that was stopped ? */
               printf ("FAILURE!  ");
            printf ("did not expire.\n");
         }
         else
         {                                     /* ... timer call back was executed */
            if (tmrcnt == test_cntr)           /* is it the timer that was stopped ? */
               printf ("FAILURE!  ");
            diff = tmr_info[tmrcnt].expire_time - tmr_info[tmrcnt].start_time;
            percent = (float) diff * 100 / tmr_info[tmrcnt].period;
            printf ("Results for  %5.1f Seconds.   ", (float) tmr_info[tmrcnt].period/1000);
            printf ("Deviation: (ms) = %3lld, (%%) = %4.1f\n", diff-tmr_info[tmrcnt].period, percent-100);
         }
      }
      printf("\n");
   }

   /* perform a lone timer stop tests */
   tmr_info[0].expire_time = 0;
   tmr_info[0].start_time  = m_NCS_GET_TIME_MS;
   m_NCS_TMR_START (tmr_info[0].timer, tmr_info[0].period / 10, timer_accuracy_cb_func, (void *) &tmr_info[0].expire_time);

   m_NCS_TASK_SLEEP (1000); /* get away from current timer service tick */
   m_NCS_TMR_STOP (tmr_info[0].timer);

   m_NCS_TASK_SLEEP (TMR_PERIOD + 1000); /* wait for timer to expire (if stop failed) */

   /* check result for timer */
   printf ("Timer Result: ");
   if (tmr_info[0].expire_time == 0) /* was there a call back ? */
      printf ("did not expire.\n");
   else
   {                                 /* ... timer call back was executed */
      printf ("FAILURE!  ");
      diff = tmr_info[0].expire_time - tmr_info[0].start_time;
      percent = (float) diff * 100 / tmr_info[0].period;
      printf ("Results for  %5.1f Seconds.   ", (float) tmr_info[0].period/1000);
      printf ("Deviation: (ms) = %3lld, (%%) = %4.1f\n", diff-tmr_info[0].period, percent-100);
   }
   printf ("\n");


   /* perform 3 expiring timer stop tests: 1) stop expired front, 2) stop expired middle, 3) stop expired end */
   for (test_cntr = 1; test_cntr <= 3; test_cntr++)
   {
      /* start 4 timers at one index - the call back for first timer will make the stop timer call,
         this way we know all timers are on expiring list */

      struct stop_exp_test_cb_info info;
      
      info.expire_time     = &tmr_info[0].expire_time;  /* first timer has special call back */
      info.timer_to_delete = &tmr_info[test_cntr].timer;   /* pass it the timer to stop */

      tmr_info[0].start_time  = m_NCS_GET_TIME_MS;
      m_NCS_TMR_START (tmr_info[0].timer, tmr_info[0].period / 10, timer_stop_on_expire_cb_func, (void *) &info);

      for (tmrcnt = 1; tmrcnt < 4; tmrcnt++) /* start the other 3 timers */
      {
         tmr_info[tmrcnt].expire_time = 0;
         tmr_info[tmrcnt].start_time  = m_NCS_GET_TIME_MS;
         m_NCS_TMR_START (tmr_info[tmrcnt].timer, tmr_info[tmrcnt].period / 10, timer_accuracy_cb_func, (void *) &tmr_info[tmrcnt].expire_time);
      }

      m_NCS_TASK_SLEEP (TMR_PERIOD + 2000); /* wait for all timers to expire */


      /* check results for each timer */
      for (tmrcnt = 0; tmrcnt < 4; tmrcnt++)
      {
         printf ("Timer %d: ", tmrcnt);
         if (tmr_info[tmrcnt].expire_time == 0) /* was there a call back ? */
         {
            if (tmrcnt != test_cntr)            /* is it not the timer that was stopped ? */
               printf ("FAILURE!  ");
            printf ("did not expire.\n");
         }
         else
         {                                     /* ... timer call back was executed */
            if (tmrcnt == test_cntr)           /* is it the timer that was stopped ? */
               printf ("FAILURE!  ");
            diff = tmr_info[tmrcnt].expire_time - tmr_info[tmrcnt].start_time;
            percent = (float) diff * 100 / tmr_info[tmrcnt].period;
            printf ("Results for  %5.1f Seconds.   ", (float) tmr_info[tmrcnt].period/1000);
            printf ("Deviation: (ms) = %3lld, (%%) = %4.1f\n", diff-tmr_info[tmrcnt].period, percent-100);
         }
      }
      printf("\n");
   }

   /* perform a lone expiring timer stop test */
   {
   /* start 2 timers at one index - the call back for first timer will make the stop timer call,
      this way we know both timers are on expiring list */

   struct stop_exp_test_cb_info info;
      
   info.expire_time     = &tmr_info[0].expire_time;  /* first timer has special call back */
   info.timer_to_delete = &tmr_info[1].timer;        /* pass it the timer to stop */ 

   tmr_info[0].start_time  = m_NCS_GET_TIME_MS;
   m_NCS_TMR_START (tmr_info[0].timer, tmr_info[0].period / 10, timer_stop_on_expire_cb_func, (void *) &info);

   /* start the other timer */
   tmr_info[1].expire_time = 0;
   tmr_info[1].start_time  = m_NCS_GET_TIME_MS;
   m_NCS_TMR_START (tmr_info[1].timer, tmr_info[1].period / 10, timer_accuracy_cb_func, (void *) &tmr_info[1].expire_time);

   m_NCS_TASK_SLEEP (TMR_PERIOD + 2000); /* wait for timers to expire */


   /* check results for each timer */
   for (tmrcnt = 0; tmrcnt < 2; tmrcnt++)
   {
      printf ("Timer %d: ", tmrcnt);
      if (tmr_info[tmrcnt].expire_time == 0) /* was there a call back ? */
      {
         if (tmrcnt != 1)            /* is it not the timer that was stopped ? */
            printf ("FAILURE!  ");
         printf ("did not expire.\n");
      }
      else
      {                                     /* ... timer call back was executed */
         if (tmrcnt == 1)           /* is it the timer that was stopped ? */
            printf ("FAILURE!  ");
         diff = tmr_info[tmrcnt].expire_time - tmr_info[tmrcnt].start_time;
         percent = (float) diff * 100 / tmr_info[tmrcnt].period;
         printf ("Results for  %5.1f Seconds.   ", (float) tmr_info[tmrcnt].period/1000);
         printf ("Deviation: (ms) = %3lld, (%%) = %4.1f\n", diff-tmr_info[tmrcnt].period, percent-100);
      }
   }
   printf("\n");
   }


   return NCSCC_RC_SUCCESS;
}



int
timerService_testSuite (int argc, char **argv)
{   
   printf("\n\n-Timer Manager Test Suite\n");
   ncs_mem_create ();

   timer_accuracy_test();
   timer_ringload_test();
   timer_stoptimer_test();
   
   ncs_mem_destroy ();
   printf("-Exiting Timer Manager Test Suite-\n\n");
   return 0;
}

