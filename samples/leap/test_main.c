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

#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncssysf_ipc.h>
#include <opensaf/ncssysf_mem.h>
#include <opensaf/ncssysf_tsk.h>
#include <opensaf/ncssysf_tmr.h>
#include <opensaf/ncssysfpool.h>
#include <opensaf/ncsusrbuf.h>

#include "leaptest.h"

/********************************************************************

********************************************************************/
int lt_consoleIO(int argc, char **argv);
int lt_timestamp(int argc, char **argv);
int lt_memManager(int argc, char **argv);
int lt_bufferManager(int argc, char **argv);
int lt_taskManager2(int argc, char **argv);
int lt_taskManager(int argc, char **argv);
int lt_UserBufferQueue(int argc, char **argv);
int lt_Timer(int argc, char **argv);
int lt_fileIO(int argc, char **argv);
int consoleMain (int argc, char **argv);
int lt_taskprio(int argc, char **argv);
int lt_malloc(int argc, char **argv);

void ipc_lat_report_func(void);
static int ipc_begin_lat_report(char *name);
static int ipc_lat_report(void);

/****************************************************************************************
*
* TEST: lt_consoleIO
*
* Parameters:
*       none
*
****************************************************************************************/

int
lt_consoleIO(int argc, char **argv)
{
    int c = 0;

    m_NCS_CONS_PRINTF("--Entering I/O Macro Test-\n");
    m_NCS_CONS_PRINTF("Testing m_NCS_CONS_PRINTF:\n");
    m_NCS_CONS_PRINTF("    integer (8)    = %d\n", 8);
#if (NCS_NOFLOAT == 0)
    m_NCS_CONS_PRINTF("    float (5.2)    = %f\n", 5.2);
#endif
    m_NCS_CONS_PRINTF("    character (b)  = %c\n", 'b');
    m_NCS_CONS_PRINTF("    string (Hello) = %s\n", "Hello");

    m_NCS_CONS_PRINTF("Testing m_NCS_CONS_GETCHAR: please enter a character >");
    c = (char) m_NCS_CONS_GETCHAR();
    m_NCS_CONS_PRINTF("You entered a '%c'\n", c);

    while (c != '\n') /* empty input buffer */
        c = (char) m_NCS_CONS_GETCHAR();

    m_NCS_CONS_PRINTF("Testing m_NCS_CONS_GETCHAR: please enter a second character >");
    c = (char) m_NCS_CONS_GETCHAR();
    m_NCS_CONS_PRINTF("You entered a '%c'\n", c);

    while (c != '\n') /* empty input buffer */
        c = (char) m_NCS_CONS_GETCHAR();

    m_NCS_CONS_PRINTF("Testing m_NCS_CONS_UNBUF_GETCHAR: please enter a third character (unbuffered) >");

    c = (char) m_NCS_CONS_UNBUF_GETCHAR();
    m_NCS_CONS_PRINTF("\n");
    m_NCS_CONS_PRINTF("You entered a '%c'\n", c);

    m_NCS_CONS_PRINTF("Testing m_NCS_CONS_PUTCHAR...");
    m_NCS_CONS_PUTCHAR('a');
    m_NCS_CONS_PUTCHAR('b');
    m_NCS_CONS_PUTCHAR('c');
    m_NCS_CONS_PUTCHAR('d');
    m_NCS_CONS_PUTCHAR('e');
    m_NCS_CONS_PUTCHAR('.');
    m_NCS_CONS_PUTCHAR('\n');
    m_NCS_CONS_PRINTF("-Exiting I/O Macro Test-\n\n");
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************************
*
* TEST: lt_timestamp
*
* Parameters:
*       none
*
****************************************************************************************/

int
lt_timestamp(int argc, char **argv)
{
    time_t now;
    char   ascii_time[32];

    /* need to be changed; the two timestamp macros are not designed to work together,
    the test should verify them separately */

    m_NCS_CONS_PRINTF("-Entering Timestamp Macro Test-\n");
    m_NCS_CONS_PRINTF("...getting current time...");
    m_GET_TIME_STAMP(now);
    m_GET_ASCII_TIME_STAMP(now, ascii_time);
    m_NCS_CONS_PRINTF("...it's %s\n", ascii_time);
    m_NCS_CONS_PRINTF("...waiting 10 seconds...");
    m_NCS_TASK_SLEEP(10000); /* wait 10 seconds */
    m_GET_TIME_STAMP(now);
    m_GET_ASCII_TIME_STAMP(now, ascii_time);
    m_NCS_CONS_PRINTF(".it's now %s\n", ascii_time);
    m_NCS_CONS_PRINTF("-Exiting Timestamp Macro Test-\n\n");
    return NCSCC_RC_SUCCESS;
}


/****************************************************************************************
*
* TEST: lt_memManager
*
* Parameters:
*       none
*
****************************************************************************************/
int
lt_memManager(int argc, char **argv)
{
    char *mptr[5];
    int i;

    ncs_mem_create ();

    m_NCS_CONS_PRINTF("-Entering Memory Manager Macro Test-\n");

    for(i=0; i<5; i++){
        mptr[i] = m_NCS_MEM_ALLOC(10, 0, 0, 0);
        m_NCS_CONS_PRINTF(" Allocated Memory\n");
    }

    if (mptr == NULL)
    {
        m_NCS_CONS_PRINTF("  !!Failed to allocate memory!!\n");
    }

    i=2;
#if 1 /* set this to 0 to create a memory leak */
    for(i=0; i<5; i++)
#endif
        m_NCS_MEM_FREE(mptr[i], 0, 0, 0);

    m_NCS_CONS_PRINTF("-Exiting Memory Manager Macro Test-\n\n");

    ncs_mem_destroy ();

    return NCSCC_RC_SUCCESS;
}



/****************************************************************************************
*
* TEST: lt_bufferManager
*
* Parameters:
*       none
*
****************************************************************************************/

int lt_bufferManager(int argc, char **argv)
{
    USRBUF *ub;
    char store[10];

    ncs_mem_create ();
    m_NCS_CONS_PRINTF("-Entering Buffer Manager Macro Test-\n");

    ub = m_MMGR_ALLOC_BUFR(0);
    if (ub == BNULL)
        m_NCS_CONS_PRINTF("  !!Failed to allocate a USRBUF!!\n");
    else
    {
        m_NCS_CONS_PRINTF("Allocated USRBUF - okay\n");

        /* try to read from empty usrbuf */
        if (m_MMGR_DATA_AT_END (ub, 10, store) != NULL)
            m_NCS_CONS_PRINTF("  !!Failure; incorrectly read from empty usrbuf\n");
        m_MMGR_FREE_BUFR_LIST(ub);
    }

    ncs_mem_destroy ();
    m_NCS_CONS_PRINTF("-Exiting Buffer Manager Macro Test-\n\n");
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************************
*
* TEST: lt_taskManager2
*
* Parameters:
*       none
*
****************************************************************************************/
/*lint -e767*/
#define MAX_TASKS 10
/*lint +e767*/

int gl_task_test2_kickoff=0;


static NCS_OS_CB
task_test2_even(void * arg) /* this routine serves the next test function */
{
    m_NCS_CONS_PRINTF("-Entered task even %p...wait for global kickoff\n", arg);

    while (gl_task_test2_kickoff==0) /* wait for kickoff */
        m_NCS_TASK_SLEEP(500); /* allow 1/2 second for other tasks */

    m_NCS_CONS_PRINTF("-Task %p has received the kickoff\n", arg);

    gl_task_test2_kickoff++; /* count me in */

    while (1) /*lint !e716 until main task deletes me ... */
        m_NCS_TASK_SLEEP(500); /* allow 1/2 second for other tasks */
}

static NCS_OS_CB
task_test2_odd(void * arg) /* this routine serves the next test function */
{
    m_NCS_CONS_PRINTF("-Entered task odd %p...wait for global kickoff\n", arg);

    while (gl_task_test2_kickoff==0) /* wait for kickoff */
        m_NCS_TASK_SLEEP(500); /* allow 1/2 second for other tasks */

    m_NCS_CONS_PRINTF("-Task %p dhas received the kickoff\n", arg);

    gl_task_test2_kickoff++; /* count me in */

    while (1) /*lint !e716 until main task deletes me ... */
        m_NCS_TASK_SLEEP(500); /* allow 1/2 second for other tasks */
}


int
lt_taskManager2(int argc, char **argv)
{
    int   tsk_num;
    void *taskhandles[MAX_TASKS];

    m_NCS_CONS_PRINTF("-Entering Task Manager Macro Test #2-\n");

    gl_task_test2_kickoff=0;
    for (tsk_num=0; tsk_num<MAX_TASKS; tsk_num++)
    {
        if (m_NCS_TASK_CREATE ((0==tsk_num%2)?(NCS_OS_CB)task_test2_odd:(NCS_OS_CB)task_test2_even,
                              (void *)(long) tsk_num,
                              "TSKT",
                              NCS_TASK_PRIORITY_4,
                              NCS_STACKSIZE_HUGE,
                              &taskhandles[tsk_num]) != NCSCC_RC_SUCCESS)
        {
            m_NCS_CONS_PRINTF("Failed! to create task %d\n", tsk_num);
            break;
        }
        if (m_NCS_TASK_START (taskhandles[tsk_num]) != NCSCC_RC_SUCCESS)
        {
            m_NCS_CONS_PRINTF("Failed! to start task %d\n", tsk_num);
            break;
        }
        m_NCS_CONS_PRINTF("Created & started task %d with handle %lx\n",
                         tsk_num, (long) taskhandles[tsk_num]);
    }

    gl_task_test2_kickoff=1;
    /* wait for all tasks (including this one) to count off */
    while (gl_task_test2_kickoff <= MAX_TASKS)
        m_NCS_TASK_SLEEP(500); /* allow 1/2 second for other tasks */


    for (tsk_num--; tsk_num >= 0; tsk_num--)
    {
        if (m_NCS_TASK_STOP(taskhandles[tsk_num]) != NCSCC_RC_SUCCESS)
            m_NCS_CONS_PRINTF("Failed! to stop task %d\n", tsk_num);
        if (m_NCS_TASK_RELEASE(taskhandles[tsk_num]) != NCSCC_RC_SUCCESS)
            m_NCS_CONS_PRINTF("Failed! to release task %d\n", tsk_num);
        else
            m_NCS_CONS_PRINTF("Released task %d\n", tsk_num);
    }

    m_NCS_CONS_PRINTF("-Exiting Task Manager Macro Test #2-\n\n");
    return NCSCC_RC_SUCCESS;
}



/****************************************************************************************
*
* TEST: lt_taskManager
*
* Parameters:
*       none
*
****************************************************************************************/

int       gl_task_oscb_done;
void     *gl_dummy_task_hdl;
SYSF_MBX  gl_dummy_mbx;


static NCS_OS_CB
task_oscb_entry(void * arg) /* this routine serves the next test function */
{
    void         *temp_task_hdl = 0;
    NCS_IPC_MSG   *msg;

    m_NCS_CONS_PRINTF("Entered second task with arg %p.\n", arg);
    m_NCS_CONS_PRINTF ("Call to m_NCS_TASK_CURRENT(..) - ");

    m_NCS_TASK_CURRENT (&temp_task_hdl);

    if (temp_task_hdl == gl_dummy_task_hdl)
        m_NCS_CONS_PRINTF ("Success\n");
    else
    {
        m_NCS_CONS_PRINTF ("Failed\n");
        m_NCS_CONS_PRINTF ("Task Handles: original = %lx,  temp = %lx\n",
                          (long) gl_dummy_task_hdl, (long) temp_task_hdl);
    }

    msg = (NCS_IPC_MSG*)m_NCS_IPC_RECEIVE(&gl_dummy_mbx, NULL);
    m_NCS_CONS_PRINTF("msg coming out 0x%lx\n", (long)msg);

    while (1) /*lint !e716 */
    {
        m_NCS_CONS_PRINTF("Wait for main task to delete this second test.\n");
        gl_task_oscb_done = 1; /* tell main task that this second task is running */

        m_NCS_TASK_SLEEP(10000); /* 10 seconds */
    }
}


int
lt_taskManager(int argc, char **argv)
{
    uns32         rc = NCSCC_RC_FAILURE;
    NCS_IPC_MSG    msg;

    gl_task_oscb_done = FALSE;
    gl_dummy_task_hdl = 0;
    gl_dummy_mbx = 0;

    m_NCS_CONS_PRINTF("-Entering Task Manager Macro Test-\n");

    m_NCS_CONS_PRINTF("msg going in 0x%lx\n", (long)&msg);

    if (m_NCS_IPC_CREATE(&gl_dummy_mbx) != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    if (m_NCS_TASK_CREATE ((NCS_OS_CB)task_oscb_entry,
                          &gl_dummy_mbx,
                          "TSKT",
                          NCS_TASK_PRIORITY_4,
                          NCS_STACKSIZE_HUGE,
                          &gl_dummy_task_hdl) != NCSCC_RC_SUCCESS)
    {
        m_NCS_IPC_RELEASE(&gl_dummy_mbx, NULL);
        return NCSCC_RC_FAILURE;
    }

    m_NCS_CONS_PRINTF("Task Handle = %lx\n", (long) gl_dummy_task_hdl);

    if (m_NCS_TASK_START (gl_dummy_task_hdl) != NCSCC_RC_SUCCESS)
    {
        m_NCS_TASK_RELEASE(gl_dummy_task_hdl);
        m_NCS_IPC_RELEASE(&gl_dummy_mbx, NULL);
        return NCSCC_RC_FAILURE;
    }
    m_NCS_CONS_PRINTF("Created & started task with handle %lx\n",
                     (long) gl_dummy_task_hdl);

    m_NCS_IPC_ATTACH_EXT(&gl_dummy_mbx, "TSKT");
    ipc_begin_lat_report("TSKT");
    m_NCS_IPC_SEND(&gl_dummy_mbx, &msg, NCS_IPC_PRIORITY_NORMAL);

    while (gl_task_oscb_done == 0) /* wait for other to start running */
        m_NCS_TASK_SLEEP(1000); /* 1 second */

    if ((rc = m_NCS_TASK_STOP(gl_dummy_task_hdl)) != NCSCC_RC_SUCCESS)
        m_NCS_CONS_PRINTF("Failure when stopping task\n");

    ipc_lat_report();
    m_NCS_IPC_DETACH(&gl_dummy_mbx, NULL, "TSKT");

    if ((rc = m_NCS_TASK_RELEASE(gl_dummy_task_hdl)) != NCSCC_RC_SUCCESS)
        m_NCS_CONS_PRINTF("Failure when releasing task\n");
    m_NCS_IPC_RELEASE(&gl_dummy_mbx, NULL);

    m_NCS_CONS_PRINTF("-Exiting Task Manager Macro Test-\n\n");
    return NCSCC_RC_SUCCESS;
}



/****************************************************************************************
*
* TEST: lt_UserBufferQueue
*
* Parameters:
*       none
*
****************************************************************************************/

int
lt_UserBufferQueue(int argc, char **argv)
{
    SYSF_UBQ q;
    USRBUF   *ub;
    unsigned int count;

    m_NCS_CONS_PRINTF("-Entering User Buffer Queue Test-\n");

    m_MMGR_UBQ_CREATE(q);
    ncs_mem_create ();

    ub = m_MMGR_ALLOC_BUFR(0);
    if (ub == BNULL)
        m_NCS_CONS_PRINTF("  !!Failed to allocate a USRBUF!!\n");
    else
    {
        if ((count=m_MMGR_UBQ_COUNT(q)) != 0){
            m_NCS_CONS_PRINTF("FAILED! Queue Count = %d; it should be 0.\n", count);
        }
        else if(count== 0){
            m_NCS_CONS_PRINTF("Queue Count = %d; it should be 0. Passed!\n", count);
        }

        m_MMGR_UBQ_NQ_TAIL(q, ub);
        if ((count=m_MMGR_UBQ_COUNT(q)) != 1){
            m_NCS_CONS_PRINTF("FAILED! Queue Count = %d; it should be 1.\n", count);
        }
        else if(count== 1){
            m_NCS_CONS_PRINTF("Queue Count = %d; it should be 1. Passed!\n", count);
        }

        ub = m_MMGR_UBQ_DQ_HEAD(q);
        if ((count=m_MMGR_UBQ_COUNT(q)) != 0){
            m_NCS_CONS_PRINTF("FAILED! Queue Count = %d; it should be 0.\n", count);
        }
        else if(count== 0){
           m_NCS_CONS_PRINTF("Queue Count = %d; it should be 0. Passed!\n", count);
        }

        m_MMGR_FREE_BUFR_LIST(ub);
    }

    m_MMGR_UBQ_RELEASE(q);
    ncs_mem_destroy ();

    m_NCS_CONS_PRINTF("-Exiting User Buffer Queue Test-\n\n");
    return NCSCC_RC_SUCCESS;
}


/****************************************************************************************
*
* TEST: lt_Timer
*
* Parameters:
*       none
*
****************************************************************************************/

static void *
test_timer_cb(void * t_arg)
{
    time_t now;
    char   ascii_time[32];

    m_GET_TIME_STAMP(now);
    m_GET_ASCII_TIME_STAMP(now, ascii_time);

    m_NCS_CONS_PRINTF("Timer Callback: timer arg = %p,  it's %s\n", t_arg, ascii_time);
    * (int *) t_arg = 1;
    return (0);
}

int
lt_Timer(int argc, char **argv)
{
    tmr_t test_tmr_handle;
    int timer_cb_done=0;

    ncs_mem_create ();

    m_NCS_CONS_PRINTF("-Entering Timer Test-\n");

    m_NCS_TMR_CREATE(test_tmr_handle, 100, test_timer_cb, 0);

    m_NCS_TMR_START (test_tmr_handle, 100, (TMR_CALLBACK) test_timer_cb, (void *) &timer_cb_done);


    while (timer_cb_done==0)
        m_NCS_TASK_SLEEP(500); /* 1/2 second */


    m_NCS_TMR_DESTROY(test_tmr_handle);

    m_NCS_CONS_PRINTF("-Exiting Timer Test-\n\n");

    ncs_mem_destroy ();

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************************
*
* TEST: lt_timer_remaining
*
* Parameters:
*  argv[0]      - the amount of msec to set the timer at.
*  argv[1]      - the number of msec to perform the periodic test for time remaining
*
****************************************************************************************/

int
lt_timer_remaining(int argc, char **argv)
{
    uns32 timer_period, check_period;
    tmr_t test_tmr_handle;
    int timer_cb_done=0;
    uns32 remaining = 0;
    time_t now;
    char   ascii_time[32];

    m_NCS_CONS_PRINTF("-Entering Time Remaining Test-\n");
    if (argc < 2)
    {
        m_NCS_CONS_PRINTF ("Invalid number of parameters\n");
        return NCSCC_RC_FAILURE;
    }

    sscanf(argv[0], "%ud", (uns32 *) &timer_period);
    sscanf(argv[1], "%ud", (uns32 *) &check_period);

    m_NCS_TMR_CREATE(test_tmr_handle, timer_period/10, test_timer_cb, 0);

    m_GET_TIME_STAMP(now);
    m_GET_ASCII_TIME_STAMP(now, ascii_time);
    m_NCS_CONS_PRINTF("...it's %s\n", ascii_time);

    m_NCS_TMR_START (test_tmr_handle, timer_period/10, (TMR_CALLBACK) test_timer_cb, (void *) &timer_cb_done);


    while (timer_cb_done==0)
    {
        m_NCS_TASK_SLEEP(check_period);
        if (NCSCC_RC_SUCCESS == m_NCS_TMR_MSEC_REMAINING (test_tmr_handle, &remaining))
            m_NCS_CONS_PRINTF ("  time left = %d msec\n", remaining);
        else
            m_NCS_CONS_PRINTF ("  timer is no longer valid\n");
    }


    m_NCS_TMR_DESTROY(test_tmr_handle);

    m_NCS_CONS_PRINTF("-Exiting Time Remaining Test-\n\n");
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************************
*
* TEST: lt_malloc
*
* Parameters:
*       none
*
****************************************************************************************/

int
lt_malloc(int argc, char **argv)
{
  if (argc == 0){
     m_NCS_CONS_PRINTF("Please enter a memory size value to allocate.\n\n");
  }
  else{
     unsigned int  nbytes = (unsigned int)atoi(argv[0]);     
     m_NCS_MEM_ALLOC(nbytes, 0,0,0);
     m_NCS_CONS_PRINTF("-Allocated %d bytes from memory-\n\n", nbytes);
  }
  return 0;
}

/****************************************************************************************
*
* TEST: lt_fileIO
*
* Parameters:
*       none
*
****************************************************************************************/

int
lt_fileIO(int argc, char **argv)
{
    FILE  *fp;
    int    rc = 0;
    char   testString[] = "Testing file I/O\n";
    char   readString[32];
    char *rsptr;
    int  c;

    m_NCS_CONS_PRINTF("-Entering File I/O Test-\n");

    if ((fp = sysf_fopen(argv[0], "w")) == NULL)
    {
        m_NCS_CONS_PRINTF("Failed! could not open file for write\n");
        goto lt_fileIO_exit;
    }
    if ((rc = sysf_fprintf(fp, "%s", testString)) < 0)
        m_NCS_CONS_PRINTF("Failed! printing to  file returned %d\n", rc);
    if ((rc = sysf_fclose(fp)) != 0)
        m_NCS_CONS_PRINTF("Failed! closing file returned %d\n", rc);


    if ((fp = sysf_fopen(argv[0], "r")) == 0)
    {
        m_NCS_CONS_PRINTF("Failed! could not open file for read\n");
        goto lt_fileIO_exit;
    }

    rsptr = (char *)readString;
    while ((c = sysf_getc(fp)) != EOF)
    {
      *rsptr++ = (char) c;
    }
    *rsptr = '\0';

    if (strcmp((char*)testString, (char*)readString) != 0)
    {
        m_NCS_CONS_PRINTF("Failed! reading file did not match writing file\nFile Contents:\n%s\n", readString);
    }

    if ((rc = sysf_fclose(fp)) != 0)
        m_NCS_CONS_PRINTF("Failed! closing file returned %d\n", rc);

lt_fileIO_exit:

    m_NCS_CONS_PRINTF("-Exiting File I/O Test-\n\n");
    return 0;
}



static int
mem_ignore(int argc, char **argv)
{
    if(1 == argc)
    {
        ncs_mem_ignore((argv[0][0]=='1')?1:0);
    }
    return 0;
}

static int
mem_whatsout_dump(int argc, char **argv)
{
    ncs_mem_whatsout_dump();
    return 0;
}

static int
mem_whatsout_summary(int argc, char **argv)
{
    if(1 == argc)
    {
        ncs_mem_whatsout_summary(argv[0]);
    }
    return 0;
}

static int
mem_stktrace_add(int argc, char **argv)
{
#if (NCSSYSM_STKTRACE)
    if(2 == argc)
    {
        NCSSYSM_LM_ARG   lm_arg;

        memset(&lm_arg, '\0', sizeof(NCSSYSM_LM_ARG));
        lm_arg.i_op = NCSSYSM_LM_OP_MEM_STK_ADD;
        lm_arg.op.mem_stk_add.i_file = argv[1];
        lm_arg.op.mem_stk_add.i_line = atoi(argv[0]);

        if(ncssysm_lm(&lm_arg) != NCSCC_RC_SUCCESS)
        {
          m_NCS_CONS_PRINTF("\nncssysm_lm(): NCSSYSM_LM_OP_MEM_STK_ON failed\n");
          return NCSCC_RC_FAILURE;
        }
    }
#endif
    return 0;
}

static int
mem_stktrace_flush(int argc, char **argv)
{
#if (NCSSYSM_STKTRACE)
    NCSSYSM_LM_ARG   lm_arg;
 
    memset(&lm_arg, '\0', sizeof(NCSSYSM_LM_ARG));
    lm_arg.i_op = NCSSYSM_LM_OP_MEM_STK_FLUSH;
 
    if(ncssysm_lm(&lm_arg) != NCSCC_RC_SUCCESS)
    {
      m_NCS_CONS_PRINTF("\nncssysm_lm(): NCSSYSM_LM_OP_MEM_STK_ON failed\n");
      return NCSCC_RC_FAILURE;
    }
#endif
    return 0;
}

#define STKTRACE_REPORT_SIZE   2048
static int
mem_stktrace_report(int argc, char **argv)
{
#if (NCSSYSM_STKTRACE)
    static NCS_BOOL once = TRUE;
    uns8 outstr[STKTRACE_REPORT_SIZE];
    NCSSYSM_LM_ARG   lm_arg;

    memset(&lm_arg, '\0', sizeof(NCSSYSM_LM_ARG));
    do
    {
        lm_arg.i_op = NCSSYSM_LM_OP_MEM_STK_RPT;
        lm_arg.op.mem_stk_rpt.i_file = argv[1];
        lm_arg.op.mem_stk_rpt.i_line = atoi(argv[0]);
        lm_arg.op.mem_stk_rpt.i_ticks_min = 0;
        lm_arg.op.mem_stk_rpt.i_ticks_max = 0xFFFFFFFF;
        lm_arg.op.mem_stk_rpt.io_rsltsize = STKTRACE_REPORT_SIZE;
        lm_arg.op.mem_stk_rpt.io_results = outstr;
        lm_arg.op.mem_stk_rpt.io_marker = lm_arg.op.mem_stk_rpt.io_marker;
        lm_arg.op.mem_stk_rpt.o_records = lm_arg.op.mem_stk_rpt.o_records;

        if(4 == argc)
        {
            lm_arg.op.mem_stk_rpt.i_ticks_min = atoi(argv[2]);
            lm_arg.op.mem_stk_rpt.i_ticks_max = atoi(argv[3]);
        }
        if(2 == argc || 4 == argc)
        {
            if(ncssysm_lm(&lm_arg) != NCSCC_RC_SUCCESS)
            {
              m_NCS_CONS_PRINTF("\nncssysm_lm(): NCSSYSM_LM_OP_MEM_STK_ON failed\n");
              return NCSCC_RC_FAILURE;
            }
    
            m_NCS_CONS_PRINTF((char*)outstr);
        }
        
    } while(TRUE == lm_arg.op.mem_stk_rpt.o_more);
#endif
    return 0;
}



static int
lt_time_ms(int argc, char **argv)
{
    m_NCS_CONS_PRINTF("Testing getting time in milliseconds\n");
    if(argc < 3)
       return NCSCC_RC_FAILURE;
    int iterations = atoi(argv[0]);
    int start = atoi(argv[1]);
    int increment = atoi(argv[2]);
    int index;

    for(index = 0; index < iterations; index++, start+=increment)
    {
        int64 t1, t2;
        t1 = m_NCS_GET_TIME_MS;
        m_NCS_TASK_SLEEP (start);
        t2 = m_NCS_GET_TIME_MS;
        m_NCS_CONS_PRINTF("t1==%lld t2==%lld diff==%lld (%u)\n", t1, t2, t2-t1, start);
    }

    return 0;
}



static int
crash_me(int argc, char **argv)
{
    uns8 (*ptr)() = NULL;

    (*ptr)();

    return 0;
}

void 
ipc_lat_report_func(void)
{
    m_NCS_CONS_PRINTF("\nIPC Report is ready\n");
    return;
}

static int 
ipc_begin_lat_report(char *name)
{
#if (NCSSYSM_IPC_REPORT_ACTIVITY == 1)
   NCSSYSM_LM_ARG   lm_arg;

   memset(&lm_arg, '\0', sizeof(NCSSYSM_LM_ARG));
   lm_arg.i_op = NCSSYSM_LM_OP_IPC_RPT_LBGN;
   lm_arg.op.ipc_rpt_lat_bgn.i_name = name;
   lm_arg.op.ipc_rpt_lat_bgn.i_cbfunc = ipc_lat_report_func;

   if(ncssysm_lm(&lm_arg) == NCSCC_RC_SUCCESS)
   {
     m_NCS_CONS_PRINTF("\nncssysm_lm(): NCSSYSM_LM_OP_RPT_LBGN depth=%d percentile=%d\n", lm_arg.op.ipc_rpt_lat_bgn.o_cr_depth, lm_arg.op.ipc_rpt_lat_bgn.o_cr_pcnt);
     return NCS_LT_TEST_RC_FAILURE;
   }
   else
   {
      m_NCS_CONS_PRINTF("\nncssysm_lm(): NCSSYSM_LM_OP_MEM_STK_ON failed\n");
      return NCS_LT_TEST_RC_SUCCESS;
   }
#endif
    return NCS_LT_TEST_RC_SUCCESS;
}


static int 
ipc_lat_report(void)
{
#if (NCSSYSM_IPC_REPORT_ACTIVITY == 1)
   NCSSYSM_LM_ARG   lm_arg;

   memset(&lm_arg, '\0', sizeof(NCSSYSM_LM_ARG));
   lm_arg.i_op = NCSSYSM_LM_OP_IPC_RPT_LTCY;

   if(ncssysm_lm(&lm_arg) == NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("\nncssysm_lm(): NCSSYSM_LM_OP_IPC_RPT_LTCY name %s\nstart time=%u finish time=%u (diff=%u)\nstart depth=%d finish depth=%d\n",
                             lm_arg.op.ipc_rpt_lat.o_name, 
                             lm_arg.op.ipc_rpt_lat.o_st_time,
                             lm_arg.op.ipc_rpt_lat.o_fn_time, 
                             lm_arg.op.ipc_rpt_lat.o_fn_time - lm_arg.op.ipc_rpt_lat.o_st_time,
                             lm_arg.op.ipc_rpt_lat.o_st_depth,
                             lm_arg.op.ipc_rpt_lat.o_fn_depth);
      return NCS_LT_TEST_RC_SUCCESS;
   }
   else
   {
      m_NCS_CONS_PRINTF("\nncssysm_lm(): NCSSYSM_LM_OP_IPC_RPT_LTCY failed\n");
      return NCS_LT_TEST_RC_FAILURE;
   }
#endif
    return NCS_LT_TEST_RC_SUCCESS;
}

/****************************************************************************************
*
* list of Tests that can be run with leaptest
*
****************************************************************************************/

typedef struct ncs_lt_testinfo_tag
{
    char               *testName;
    NCS_LT_TESTFUNCTION  testfunc;
    char               *usage_str;
} NCS_LT_TESTINFO;



static NCS_LT_TESTINFO tests[] =
{
   /* Test Name               test function             Usage String
      ---------               -------------             ------------ */
   /* 0*/ { "Atomic Counting Suite",     atomicCounting_testSuite, "" },
   /* 1*/ { "Memory Diagnostic",         lt_memdiag,               "" },
   /* 2 { "TCP IP",                    lt_tcp,                   "<local ip addr> <remote ip addr> <c/s>" }, */
   /* 3 { "UDP IP",                    lt_udp,                   "<local ip addr> <remote ip addr>" }, */
   /* 4 { "RAW IP",                    lt_rawip,                 "<local ip addr> <remote ip addr> <if index> <usrframe|usrbuf>" }, */
   /* 5 { "Multicast",                 lt_multicast,             "<local ip addr> <remote ip addr> <if index>" }, */
   /* 6*/ { "Timer Suite",               timerService_testSuite,   "" },
   /* 7*/ { "File I/O",                  lt_fileIO,                "<filename>" },
   /* 8*/ { "Timer",                     lt_Timer,                 "" },
   /* 9*/ { "Locks",                     lt_lockManager,           "" },
   /*10*/ { "Task 1",                    lt_taskManager,           "" },
   /*11*/ { "Task 2",                    lt_taskManager2,          "" },
   /*12*/ { "Buffers",                   lt_bufferManager,         "" },
   /*13*/ { "Buffer Suite",              bufferManager_testSuite,  "" },
   /*14*/ { "Buffer Queue",              lt_UserBufferQueue,       "" },
   /*15*/ { "Console I/O",               lt_consoleIO,             "" },
   /*16*/ { "TimeStamp",                 lt_timestamp,             "" },
   /*17*/ { "Memory Manager",            lt_memManager,            "" },
   /*18 { "TCP Listen",                lt_tcp_listen,            "<local ip addr> <remote ip addr> <c/s>" }, */
   /*19*/ { "Timer Remaining",           lt_timer_remaining,       "<init time (ms)> <check time (ms)>" },
   /*20 { "Next Hop IP",               lt_nexthopip,             "<Node A/B/C/D> <A> <B> <C> <D>" }, */
   /*21*/ { "MMGR:Mem Ignore",           mem_ignore,               "<1|0>" },
   /*22*/ { "MMGR:What is out",          mem_whatsout_dump,        "" },
   /*23*/ { "MMGR:What's out summary",   mem_whatsout_summary,     "" },
   /*24*/ { "SYSM:stktrace add",         mem_stktrace_add,         "<line> <filename>" },
   /*25*/ { "SYSM:stktrace flush",       mem_stktrace_flush,       "" },
   /*26*/ { "SYSM:stktrace report",      mem_stktrace_report,      "<line> <filename> [<min> <max>]" },
   /*27*/ { "Time MS",                   lt_time_ms,               "iterations start increment" },
   /*28*/ { "crash me",                  crash_me,                 "" },
   /*29*/ { "test task scheduling",      lt_taskprio,              "" },
   /*30*/ { "test malloc from pool 0",   lt_malloc,                "<nbytes>" },
   /*31*/ { "file operations",           lt_file_ops,              "" },
   /*32*/ { "encode/decode",             lt_encdec_ops,            ""},
   /*33*/ { "process-library",           lt_processlib_ops,        ""},
   /*34*/ { "patricia tree",             lt_patricia_ops,          ""},
   /*35*/ { "SYSFMBX enhancements",      lt_mailbox_ops,           ""}

    

};



static void
test_usage(unsigned int tNum)
{
    m_NCS_CONS_PRINTF("\n%s Test Usage:\nlt> %d %s\n\n", tests[tNum].testName, tNum, tests[tNum].usage_str);
}


static void
leaptest_usage(void)
{
    m_NCS_CONS_PRINTF("\nLeaptest Usage:\nlt> <test #> <other parameters as needed by test>\n");
    m_NCS_CONS_PRINTF("For a lists of tests, type '?' at the prompt.\n\n");
}


static void
Show_tests()
{
    int i;
    int n;
    char disp[28];
    n = sizeof(tests) / sizeof(NCS_LT_TESTINFO);

    m_NCS_CONS_PRINTF("\nAvailable leaptests:");
    m_NCS_CONS_PRINTF("\nName                     #   Parameters");
    m_NCS_CONS_PRINTF("\n----                    ---  ----------");
    for(i = 0; i < n; i ++)
    {
        memset(disp, 0X20, sizeof(disp));
        disp[27] = 0;
        strncpy(disp, tests[i].testName, strlen(tests[i].testName));
        m_NCS_CONS_PRINTF("\n%s %3d  %s", disp, i, tests[i].usage_str);
    }
    m_NCS_CONS_PRINTF("\n");
}



/****************************************************************************************
*
* Parameters:
*     cmd           an array provided by caller to store user command.
*     pv            
*
****************************************************************************************/

#define CMD_CH_MAX 100
#define PARAMS_MAX 10

static int lt_get_usercmdline(char * cmd, char **pv)
{
    int pc = 0;

    char *t;

    m_NCS_CONS_PRINTF("lt> ");
    sysf_fgets(cmd, CMD_CH_MAX, stdin);
    t = strtok(cmd, " \n");
    while (t != NULL)
    {
        pv[pc++] = t;
        t = strtok(NULL, " \n");
    }

    return pc;
}

static NCS_OS_CB
test_thread_2()
{ 
  int i; 
  
  for(i=0;i<300;i++){
    printf("+");       
  }

  return NULL;
}


static NCS_OS_CB
test_thread_1()
{ 
   int i;   
  
   for(i=0;i<300;i++){       
     printf("-");
   } 

   return NULL;
}

int
lt_taskprio(int argc, char **argv)
{
    void *taskhandle1;
    void *taskhandle2;  
    
    m_NCS_CONS_PRINTF("-Entering Task Priority Test -\n");
    m_NCS_CONS_PRINTF("\nBoth tests set to same priority level: \n\n");

    /* The tasks below are set to the same priority  */
    /* to allow equal CPU time. Lower priority level */
    /* on one of the two tasks to demonstrate the    */
    /* OS's priority scheduling.                     */
    /* 0  = High Priority                            */
    /* 16 = Low Priority (Windows)                   */
    /* 31 = Low Priority (Linux)                     */

    /* Create Task 1 with lowest priority */
    if (m_NCS_TASK_CREATE ((NCS_OS_CB)test_thread_1,
                            NULL,
                            "test_thread_1",
                            NCS_OS_TASK_PRIORITY_16,
                            NCS_STACKSIZE_HUGE,
                            &taskhandle1) != NCSCC_RC_SUCCESS)
    {
       m_NCS_CONS_PRINTF("Failed! to create task 1\n");
    }
    
    /* Create Task 2 with lowest priority */
    if (m_NCS_TASK_CREATE ((NCS_OS_CB)test_thread_2,
                            NULL,
                            "test_thread_2",
                            NCS_OS_TASK_PRIORITY_16,
                            NCS_STACKSIZE_HUGE,
                            &taskhandle2) != NCSCC_RC_SUCCESS)
    {
       m_NCS_CONS_PRINTF("Failed! to create task 2\n");
    }   
     
    if (m_NCS_TASK_START (taskhandle1) == NCSCC_RC_SUCCESS)
    {
       m_NCS_CONS_PRINTF("  Created & started task 1 (-) with handle %lx\n",
                      (long) taskhandle1);       
    }
    else{
       m_NCS_CONS_PRINTF("Failed! to start task 1\n");
    }
   
    if (m_NCS_TASK_START (taskhandle2) == NCSCC_RC_SUCCESS)
    {
       m_NCS_CONS_PRINTF("  Created & started task 2 (+) with handle %lx\n\n",
                        (long) taskhandle2);
    }
    else{
       m_NCS_CONS_PRINTF("Failed! to start task\n");
    }
       
    /* Let's Pause to allow tasks to run momentarily */
    m_NCS_TASK_SLEEP(50);

    m_NCS_CONS_PRINTF("\n\n-Exiting Task Priority Test -\n");
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************************
*
* argv[0]:   "leaptest"
* argv[1]:   test number
* argv[2-n]: other argument are test specific - see each test function
*
****************************************************************************************/
int
consoleMain (int argc, char **argv)
{
    uns32 testNum;
    char  cmd[CMD_CH_MAX];
    char *pv[PARAMS_MAX];
    int32 testing = TRUE;
    int32 pc;

    /* Initialize LEAP environment */
    if(leap_env_init() != NCSCC_RC_SUCCESS)
       return 1;

    m_NCS_CONS_PRINTF("Starting Leap Test");
    m_NCS_CONS_PRINTF("Press Enter to start\n");
    m_NCS_CONS_GETCHAR();

    if(argc > 1) /* run test from command line, skipping lt prompt */
    {
        testNum = (uns32)atoi(argv[1]);
        if (testNum > (sizeof (tests) / sizeof (NCS_LT_TESTINFO)))
        {
            test_usage(testNum);
        }
        else
        {
            for(pc=0; pc<argc-1; pc++)
            {
                pv[pc] = argv[pc+2];
            }
            pc = argc - 1;
            if(tests[testNum].testfunc(pc, pv) == NCS_LT_TEST_RC_INVALID_USAGE)/*lint !e661*/
            {
                test_usage(testNum);
                return 0;
            }
        }
    }
    else /* run test(s) from lt prompt */
    {
        while (testing == TRUE)
        {
            pc = lt_get_usercmdline(cmd, pv);

            if (cmd[0] == '\n')
                continue;

            if ((cmd[0] == 'q') || (cmd[0] == 'Q'))
            {
                testing = FALSE;
                continue;
            }

            if (cmd[0] == '?')
            {
                Show_tests();
                continue;
            }

            if (sscanf(pv[0], "%d", (unsigned int *) &testNum) == 0)
            {
               leaptest_usage();
               continue;
            }

            if (testNum >= (sizeof (tests) / sizeof (NCS_LT_TESTINFO)))
            {
                leaptest_usage();
                continue;
            }

            m_NCS_ASSERT(testNum < (sizeof(tests)/sizeof(NCS_LT_TESTINFO)));

            if (tests[testNum].testfunc(pc-1, &pv[1]) == NCS_LT_TEST_RC_INVALID_USAGE) /*lint !e661 !e662*/
            {
                test_usage (testNum);
                continue;
            }
        }
    }

    /* destroy LEAP environment */
    leap_env_destroy();

    return 0;
}



#ifndef _NCS_IGNORE_MAIN_
int main(int argc, char **argv)
{
    consoleMain(argc, argv);
    return 0;
}
#endif


