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
#include <opensaf/ncssysf_tsk.h>
#include <opensaf/ncssysfpool.h>
#include "leaptest.h"

/********************************************************************

********************************************************************/




NCS_LOCK gl_lock;
static int tsk1_has_lock=0; /* 0 = no, 1 = yes */
static int tsk2_has_lock=0; /* 0 = no, 1 = yes */
static int tsk2_failed=0;
static int tsk1_failed=0;
static int tsk2_started = 0;
static int task1_is_in_tp = 0; /* test point # */
static int task2_is_in_tp = 0; /* test point # */

static void
tsklck_oscb_entry(void * dummy_handle) /* this routine serves the next test function */
{
    int rc = 0;

    tsk2_started = 1;

    /* Test Point 1: let task 1 get the lock */
    while (tsk1_has_lock == 0)
        m_NCS_TASK_SLEEP(500);


    /* Test Point 2: try to get the lock that task 1 currently owns.. */
    printf("Test Point 2: task 2 will block trying to get the lock...");

    if ((rc = m_NCS_LOCK(&gl_lock, NCS_LOCK_WRITE)) != NCSCC_RC_SUCCESS) /* task 2 should block here */
    {
        printf("Failed!....return error::%d\n", rc);
        tsk2_failed=1; /* tell task 1 about failure */
        return;        /* let's get out */
    }

    /* at this point, Task 2 obtained lock ownership */

    if (tsk1_has_lock == 1) /* if task 1 still thinks it has ownership ? */
    {
        printf("Failed!....obtained lock.\n");
        tsk2_failed = 1; /* tell task 1 about failure */
        return;
    }


    /* Test Point 3:  task 1 is releasing lock - wait for task 1 to complete printfs */
    while (task1_is_in_tp < 4) /* wait until task 1 gets into test point 4 */
        m_NCS_TASK_SLEEP(500);


    /* Test Point 4:  task 2 is to take lock ownership  - of course, if here, it already has it */
    tsk2_has_lock = 1; /* tell task 1 that task 2 has the lock */
    while (task1_is_in_tp < 5) /* wait until task 1 proceeds */
        m_NCS_TASK_SLEEP(500);


    /* Test Point 5: task 2 will nest the lock */
    printf("Test Point 5: task 2 will nest its lock ownership...");
    if ((rc = m_NCS_LOCK(&gl_lock, NCS_LOCK_WRITE)) != NCSCC_RC_SUCCESS)
    {
        printf("Failed! when nesting lock, return::%d\n", rc);
        tsk2_failed=1; /* tell task 1 about failure */
        return;
    }
    /* start unlocking - part of test point 5 */
    if ((rc = m_NCS_UNLOCK(&gl_lock, NCS_LOCK_WRITE)) != NCSCC_RC_SUCCESS)
    {
        printf("Failed! when unlocking the nested lock, return::%d\n", rc);
        tsk2_failed=1; /* tell task 1 about failure */
        return;
    }
    task2_is_in_tp = 5; /* tell task 1 where task 2 is */
    m_NCS_TASK_SLEEP(2000); /* allow task 1 to try to get lock - it should block */
    if (tsk1_has_lock == 1) /* if task 1 got the lock .. */
    {
        printf("Failed! task 1 got the lock\n");
        tsk2_failed=1; /* tell task 1 about failure */
        return;
    }
    if (tsk1_failed != 1) /* if task 1 failed, don't say "Passed" */
        printf("Passed.\n");


    /* Test Point 6: task 2 will release the lock */
    printf("Test Point 6: task 2 will release its lock ownership...");
    tsk2_has_lock = 0; /* let task 1 know that task 2 does not own lock; must do prior to releasing */
    if ((rc = m_NCS_UNLOCK(&gl_lock, NCS_LOCK_WRITE)) != NCSCC_RC_SUCCESS)
        printf("Failed! return::%d\n", rc);
    else
        printf("Passed.\n");
    task2_is_in_tp = 6;

    return;
}

/****************************************************************************************
*
* TEST: lt_lockManager
*
* Parameters:
*       none
*
****************************************************************************************/

int
lt_lockManager(int argc, char **argv)
{
    SYSF_MBX     gl_dummy_mbx;
    void *gl_dummy_task_hdl;
    int rc = 0;


    printf("-Entering Lock Manager Macro Test-\n");
    ncs_mem_create ();

    /* Test Point 0: create lock */
    printf("Test Point 0: task 1 will create lock...");
    if ((rc = m_NCS_LOCK_INIT(&gl_lock)) != NCSCC_RC_SUCCESS)
    {
        printf("Failed! return::%d\n", rc);
        goto GET_OUT;
    }
    else
        printf("Passed.\n");



    /* create a second task to compete for the lock */
    if (m_NCS_IPC_CREATE(&gl_dummy_mbx) != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    if (m_NCS_TASK_CREATE ((NCS_OS_CB)tsklck_oscb_entry,
                          &gl_dummy_mbx,
                          "TSKT",
                          0,
                          NCS_STACKSIZE_HUGE,
                          &gl_dummy_task_hdl) != NCSCC_RC_SUCCESS)
    {
        m_NCS_IPC_RELEASE(&gl_dummy_mbx, NULL);
        return NCSCC_RC_FAILURE;
    }

    if (m_NCS_TASK_START (gl_dummy_task_hdl) != NCSCC_RC_SUCCESS)
    {
        m_NCS_TASK_RELEASE(gl_dummy_task_hdl);
        m_NCS_IPC_RELEASE(&gl_dummy_mbx, NULL);
        return NCSCC_RC_FAILURE;
    }

    /* wait for task to start */
    while (tsk2_started == 0)
        m_NCS_TASK_SLEEP(500);



    /* Test Point 1: get lock - task 2 will not compete here */
    printf("Test Point 1: task 1 will get lock ownership...");
    task1_is_in_tp = 1; /* tell task 2 where task 1 is */
    if ((rc = m_NCS_LOCK(&gl_lock, NCS_LOCK_WRITE)) != NCSCC_RC_SUCCESS)
    {
        printf("Failed! return::%d\n", rc);
        goto DESTROY_LOCK;
    }
    else
        printf("Passed.\n");
    tsk1_has_lock = 1; /* let task 2 know that task 1 has the lock */


    /* Test Point 2: allow time for task 2 to try to get lock...it should block */
    m_NCS_TASK_SLEEP(2000);

    if (tsk2_failed == 1) /* if task 2 did not block .. */
        goto DESTROY_LOCK;
    else
        printf("Passed.\n");



    /* Test Point 3: task 1 will release lock... */
    printf("Test Point 3: task 1 will release lock...");
    tsk1_has_lock = 0; /* let task 2 know that task 1 does not own lock; must do prior to releasing */
    task1_is_in_tp = 3; /* tell task 2 where task 1 is */
    if ((rc = m_NCS_UNLOCK(&gl_lock, NCS_LOCK_WRITE)) != NCSCC_RC_SUCCESS)
    {
        printf("Failed! return::%d\n", rc);
        goto DESTROY_LOCK;
    }
    else
        printf("Passed.\n");


    /* Test Point 4: task 2 will take lock ownership... */
    printf("Test Point 4: task 2 will take lock ownership...");
    task1_is_in_tp = 4;       /* tell task 2 it can proceed */
    m_NCS_TASK_SLEEP(2000); /* allow time for tasks to synchronize - just in case */

    if (tsk2_has_lock == 0) /* if task 2 does not have the lock by now .. */
    {
        printf("Failed! return::%d\n", rc);
        goto DESTROY_LOCK;
    }
    else
        printf("Passed.\n");



    /* Test Point 5: task 2 will nest the lock */
    task1_is_in_tp = 5;       /* tell task 2 where task 1 is */
    while (task2_is_in_tp < 5) /* wait for task to nest and then unnest lock */
    {
        if (tsk2_failed == 1) /* if task 2 fails to nest or unnest .. */
            goto DESTROY_LOCK;
        m_NCS_TASK_SLEEP(500);
    }

    /* make sure that even after task 2 unlocks nest, there is still a lock on */
    if ((rc = m_NCS_LOCK(&gl_lock, NCS_LOCK_WRITE)) != NCSCC_RC_SUCCESS) /* task 1 should block here */
    {
        printf("Failed!..task 1 trying to get lock, return error::%d\n", rc);
        tsk1_failed = 1; /* tell task 2 about failure */
    }
    else
        tsk1_has_lock = 1;   /* tell task 2 that task 1 obtained lock ownership */



    /* Test Point 6: task 2 will release the lock */
    while (task2_is_in_tp < 6) /* wait for task 2 to finish */
        m_NCS_TASK_SLEEP(500);


    /* Test Point 7: task 1 will release the lock */
    printf("Test Point 7: task 1 will release lock...");
    tsk1_has_lock = 0;
    task1_is_in_tp = 7;     /* tell task 2 where task 1 is */
    if ((rc = m_NCS_UNLOCK(&gl_lock, NCS_LOCK_WRITE)) != NCSCC_RC_SUCCESS)
        printf("Failed! return::%d\n", rc);
    else
        printf("Passed.\n");


DESTROY_LOCK:
    /* Test Point 8: task 1 will destroy the lock */
    printf("Test Point 8: task 1 will destroy the lock...");
    if ((rc = m_NCS_LOCK_DESTROY(&gl_lock)) != NCSCC_RC_SUCCESS)
        printf("Failed! return::%d\n", rc);
    else
        printf("Passed.\n");

    /* cleanup by removing task 2 */
    if (task2_is_in_tp < 6)
    {
        if (m_NCS_TASK_STOP(gl_dummy_task_hdl) != NCSCC_RC_SUCCESS)
            printf("Cleanup: Failed! stopping task 2 returned error\n");
        if (m_NCS_TASK_RELEASE(gl_dummy_task_hdl) != NCSCC_RC_SUCCESS)
            printf("Cleanup: Failed! releasing task 2 returned error\n");
    }
    m_NCS_IPC_RELEASE(&gl_dummy_mbx, NULL);

GET_OUT:
    ncs_mem_destroy ();
    printf("-Exiting Lock Manager Macro Test-\n\n");
    return NCSCC_RC_SUCCESS;
}



