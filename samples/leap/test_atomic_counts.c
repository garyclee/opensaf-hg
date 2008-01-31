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

#include "ncsgl_defs.h"

#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncssysf_lck.h"
#include "ncssysf_tsk.h"
#include "leaptest.h"


static uns32 atomic_task_kickoff;
static uns32 atomic_task_done;
static uns32 counter;

typedef enum
{
    INC_NON_ATOMIC =  1,
    INC_ATOMIC,
    DEC_NON_ATOMIC,
    DEC_ATOMIC,
    UNKNOWN
} TEST_TYPES;

static TEST_TYPES test_type = UNKNOWN;

/* IR00061531 */ 
#define ITERATIONS_TO_DO   0x000FFFF /* each iteration represents 10 increments/decrements */
#define MAX_TASKS          5


/*******************************************************************
* counting:
* Function to be used within a task to compete for the incrementing
* and decrementing of a single static variable with other tasks running
* this same function.
* A Non atomic increment & decrement may be interrupted by a competing
* task and thus throw off the resulting total.
********************************************************************
*/
static NCS_OS_CB
counting (void * arg)/*lint -e715 */
{
    int th_priority = 15;
    unsigned long iterations = ITERATIONS_TO_DO;

    /* This code will reduce the interference with  services
    thread priorities */
    nice(th_priority);

    m_NCS_ATOMIC_INC (&atomic_task_kickoff);

    while (atomic_task_kickoff < MAX_TASKS)
        m_NCS_TASK_SLEEP(20);

    /* increment or decrement 10 times within loop to increase probability of getting
         cpu time sliced away while in the middle of a single increment/decrement */
    switch (test_type)
    {
        case INC_NON_ATOMIC:
            do
            {
                counter++;  counter++;
                counter++;  counter++;
                counter++;  counter++;
                counter++;  counter++;
                counter++;  counter++;
            } while (--iterations > 0);
            break;

        case INC_ATOMIC:
            do
            {
                m_NCS_ATOMIC_INC (&counter);
                m_NCS_ATOMIC_INC (&counter);
                m_NCS_ATOMIC_INC (&counter);
                m_NCS_ATOMIC_INC (&counter);
                m_NCS_ATOMIC_INC (&counter);
                m_NCS_ATOMIC_INC (&counter);
                m_NCS_ATOMIC_INC (&counter);
                m_NCS_ATOMIC_INC (&counter);
                m_NCS_ATOMIC_INC (&counter);
                m_NCS_ATOMIC_INC (&counter);
            } while (--iterations > 0);

            break;

        case DEC_NON_ATOMIC:
            do
            {
                counter--;  counter--;
                counter--;  counter--;
                counter--;  counter--;
                counter--;  counter--;
                counter--;  counter--;
            } while (--iterations > 0);
            break;

        case DEC_ATOMIC:
            do
            {
                m_NCS_ATOMIC_DEC (&counter);
                m_NCS_ATOMIC_DEC (&counter);
                m_NCS_ATOMIC_DEC (&counter);
                m_NCS_ATOMIC_DEC (&counter);
                m_NCS_ATOMIC_DEC (&counter);
                m_NCS_ATOMIC_DEC (&counter);
                m_NCS_ATOMIC_DEC (&counter);
                m_NCS_ATOMIC_DEC (&counter);
                m_NCS_ATOMIC_DEC (&counter);
                m_NCS_ATOMIC_DEC (&counter);
            } while (--iterations > 0);
            break;

        case UNKNOWN:
        default:
            m_NCS_CONS_PRINTF("Failed in task function counting() ! invalid test_type %d\n", test_type);
            break;

    }

    m_NCS_ATOMIC_DEC (&atomic_task_done);
    nice(-th_priority);     
    return 0;
}

/*******************************************************************
* atomic_counting_test:
* Generic Function to be used for atomic/nonatomic inc/dec testing
* The parameter direction is either 1 or -1 for increment or decrement.
********************************************************************
*/
static NCS_BOOL
atomic_counting_test (int direction)
{
    int   tsk_num, i;
    void *taskhandles[MAX_TASKS];
    uns32 expected_count = 0;

    atomic_task_kickoff = 0;
    counter             = 0;
    atomic_task_done    = MAX_TASKS;

    for (tsk_num=0; tsk_num < MAX_TASKS; tsk_num++)
    {
        if (m_NCS_TASK_CREATE ((NCS_OS_CB) counting,
                              (void *)(long) tsk_num,
                              "TSKT",
                              16,
                              8192,
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
        m_NCS_CONS_PRINTF("Created & started task %d with handle %ld\n",
                         tsk_num, (long) taskhandles[tsk_num]);
    }

    /* wait for all counting tasks to count off */
    while (atomic_task_done > 0)
        m_NCS_TASK_SLEEP(500); /* allow 1/2 second for other tasks */

    for (i=0; i  <MAX_TASKS; i++)  /* overflow is okay */
    {
        expected_count += (ITERATIONS_TO_DO * 10 * direction);/*lint !e737 */
    }

    m_NCS_CONS_PRINTF("counter = %u and expected result = %u\n", counter, expected_count);

    return (counter == expected_count);
}

/*******************************************************************
* Test to verify the m_NCS_ATOMIC_INC macro
********************************************************************
*/
static int
atomic_inc_test(void)
{
    NCS_BOOL result;

    m_NCS_CONS_PRINTF("-Entering Atomic Increment Macro Test- it will take many minutes\n");


    m_NCS_CONS_PRINTF("\nFirst - perform nonatomic counting as a test reference - we want this to fail.\n");
    test_type = INC_NON_ATOMIC;
    result = atomic_counting_test (1);
    if (result == TRUE)
    {
        m_NCS_CONS_PRINTF("...Test may be invalid - nonatomic counting should not match...continuing anyway\n");
    }

    m_NCS_CONS_PRINTF("\nSecond - perform atomic counting.\n");
    test_type = INC_ATOMIC;
    result = atomic_counting_test (1);
    if (result == FALSE)
        m_NCS_CONS_PRINTF("...Test FAILED!\n");
    else
        m_NCS_CONS_PRINTF("...SUCCESS!\n");

    m_NCS_CONS_PRINTF("\n-Exiting Atomic Increment Macro Test-\n\n");
    return 0;
}



/*******************************************************************
* Test to verify the m_NCS_ATOMIC_DEC macro
********************************************************************
*/
static int
atomic_dec_test(void)
{
    NCS_BOOL result;

    m_NCS_CONS_PRINTF("-Entering Atomic Decrement Macro Test- it will take many minutes\n");

    m_NCS_CONS_PRINTF("\nFirst - perform nonatomic counting as a test reference - we want this to fail.\n");
    test_type = DEC_NON_ATOMIC;
    result = atomic_counting_test (-1);
    if (result == TRUE)
    {
        m_NCS_CONS_PRINTF("...Test may be invalid - nonatomic counting should not match...continuing anyway\n");
    }

    m_NCS_CONS_PRINTF("\nSecond - perform atomic counting.\n");
    test_type = DEC_ATOMIC;
    result = atomic_counting_test (-1);
    if (result == FALSE)
        m_NCS_CONS_PRINTF("...Test FAILED!\n");
    else
        m_NCS_CONS_PRINTF("...SUCCESS!\n");

    m_NCS_CONS_PRINTF("\n-Exiting Atomic Decrement Macro Test-\n\n");
    return 0;
}

static int start_atomic_tests(void) /* IR00058710 */
{
   atomic_inc_test();
   atomic_dec_test();
   return 0;
}

int
atomicCounting_testSuite(int argc, char **argv)/*lint -e715 */
{
   start_atomic_tests();
   return 0;
}

