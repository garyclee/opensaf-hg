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

/*****************************************************************************
..............................................................................

..............................................................................

  DESCRIPTION: This demo code demonstrates two Applications 1 & 2 talking
  to each other using SYSF_MBX's selection object mechanism.
    - Application-2 comes up first
    - Then, Application-1 comes up. It sends a Request to Application-2
    - Application-2 processes this request, and sends a response to
      back to Application-1
    - Application-1 processes the response
  
  CONTENTS:


****************************************************************************/
#include "ncsgl_defs.h"
#include "ncs_osprm.h"

#include "ncssysf_mem.h"
#include "ncssysf_def.h"
#include "ncssysf_ipc.h"
#include "ncssysf_tsk.h"
#include "leaptest.h"

typedef int32   LtHandle;   /* Unique handle for application */

typedef struct test_app_cb
{
   NCSCONTEXT           task_hdl;
   SYSF_MBX             mbx;

   LtHandle             resource_hdl;   /* Some resource */
} LT_TEST_APP_CB;

typedef enum {
    LT_TEST_EVT_APP1_REQ = 1,
    LT_TEST_EVT_APP2_RESPONSE,
    LT_TEST_EVT_MAX
}LT_TEST_EVT_TYPE;

typedef struct lt_evt_tag
{
   struct lt_evt_tag *next; /* This is never used in the demo, though!!! */
   LT_TEST_EVT_TYPE    evt;
   union {
        uns32   app1_req;
        uns32   app2_resp;
   }info;
}LT_EVT;

typedef  void (*LT_TEST_APPLICATION)(SYSF_MBX *mbx);    /* Demo part */

LT_TEST_APP_CB      gl_app1_cb, gl_app2_cb;
#define m_LT_TEST_APP_PRIORITY     (2)
#define m_LT_TEST_APP_STACKSIZE NCS_STACKSIZE_HUGE

#define m_MMGR_ALLOC_LT_EVT m_NCS_MEM_ALLOC(sizeof(LT_EVT), \
                                               NCS_MEM_REGION_PERSISTENT,\
                                               UD_SERVICE_ID_STUB1,     \
                                               0)

#define m_MMGR_FREE_LT_EVT(p) m_NCS_MEM_FREE(p, \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               UD_SERVICE_ID_STUB1,     \
                                               0)

/* Prototypes */
static uns32 create_app_stuff(int test_num);
static void lt_test1_app1_process_main(SYSF_MBX *mbx);
static void lt_test1_app2_process_main(SYSF_MBX *mbx);
static void test1_app1_process_mbx(LT_TEST_APP_CB *cb, SYSF_MBX *mbx);
static void test1_app2_process_mbx(LT_TEST_APP_CB *cb, SYSF_MBX *mbx);
static NCS_BOOL lt_leave_on_queue_cb (void *arg1, void *arg2);

uns32 lt_test1_common_process_evt(LT_TEST_APP_CB    *cb, 
                              LT_EVT   *evt);
uns32 lt_test_app2_handle_req(LT_TEST_EVT_TYPE evt, uns32 data);
uns32 lt_test_app1_handle_response(LT_TEST_EVT_TYPE evt, uns32 data);


/************************** CODE STARTS HERE **************************/
int lt_mailbox_ops(int argc, char **argv)
{
    create_app_stuff(1);
    return NCSCC_RC_SUCCESS;
}

static uns32 create_app_stuff(int test_num)
{
    char  task_name[24];
    LT_TEST_APPLICATION test_application;
    uns32 res = NCSCC_RC_SUCCESS;
    
    switch(test_num)
    {
    case 1:
        /* Application 2 */
        m_NCS_MEMSET(&gl_app2_cb, '\0', sizeof(gl_app2_cb));
        test_application = lt_test1_app2_process_main;
        if ((res = m_NCS_IPC_CREATE(&gl_app2_cb.mbx)) != NCSCC_RC_SUCCESS)
        {
            return NCSCC_RC_FAILURE;
        }      

        if ((res = m_NCS_IPC_ATTACH(&gl_app2_cb.mbx)) != NCSCC_RC_SUCCESS)
        {
            m_NCS_IPC_RELEASE(&gl_app2_cb.mbx, lt_leave_on_queue_cb);
            return NCSCC_RC_FAILURE;
        }

        m_NCS_STRCPY(task_name,"Application-2");
        /* Create a application thread */
        if ((res = m_NCS_TASK_CREATE ((NCS_OS_CB)test_application, 
            (NCSCONTEXT)&gl_app2_cb.mbx, task_name, m_LT_TEST_APP_PRIORITY, 
            m_LT_TEST_APP_STACKSIZE, &gl_app2_cb.task_hdl)) != NCSCC_RC_SUCCESS)         
        {
            (void)m_NCS_IPC_DETACH(&gl_app2_cb.mbx, NULL, NULL);
            m_NCS_IPC_RELEASE(&gl_app2_cb.mbx, lt_leave_on_queue_cb);
            return NCSCC_RC_FAILURE;
        }

        if ((res = m_NCS_TASK_START(gl_app2_cb.task_hdl)) != NCSCC_RC_SUCCESS)         
        {
            (void)m_NCS_IPC_DETACH(&gl_app2_cb.mbx, NULL, NULL);
            m_NCS_IPC_RELEASE(&gl_app2_cb.mbx, lt_leave_on_queue_cb);
            m_NCS_TASK_RELEASE(gl_app2_cb.task_hdl);
            return NCSCC_RC_FAILURE;
        }

        /* Application 1 */
        m_NCS_MEMSET(&gl_app1_cb, '\0', sizeof(gl_app1_cb));
        test_application = lt_test1_app1_process_main;
        if ((res = m_NCS_IPC_CREATE(&gl_app1_cb.mbx)) != NCSCC_RC_SUCCESS)
        {
            (void)m_NCS_IPC_DETACH(&gl_app2_cb.mbx, NULL, NULL);
            m_NCS_IPC_RELEASE(&gl_app2_cb.mbx, lt_leave_on_queue_cb);
            m_NCS_TASK_RELEASE(gl_app2_cb.task_hdl);
            return NCSCC_RC_FAILURE;
        }      
        if ((res = m_NCS_IPC_ATTACH(&gl_app1_cb.mbx)) != NCSCC_RC_SUCCESS)
        {
            (void)m_NCS_IPC_DETACH(&gl_app2_cb.mbx, NULL, NULL);
            m_NCS_IPC_RELEASE(&gl_app2_cb.mbx, lt_leave_on_queue_cb);
            m_NCS_TASK_RELEASE(gl_app2_cb.task_hdl);
            m_NCS_IPC_RELEASE(&gl_app1_cb.mbx, lt_leave_on_queue_cb);
            return NCSCC_RC_FAILURE;
        }

        m_NCS_STRCPY(task_name,"Application-1");
        /* Create a application thread */
        if ((res = m_NCS_TASK_CREATE ((NCS_OS_CB)test_application, 
            (NCSCONTEXT)&gl_app1_cb.mbx, task_name, m_LT_TEST_APP_PRIORITY, 
            m_LT_TEST_APP_STACKSIZE, &gl_app1_cb.task_hdl)) != NCSCC_RC_SUCCESS)         
        {
            (void)m_NCS_IPC_DETACH(&gl_app2_cb.mbx, NULL, NULL);
            m_NCS_IPC_RELEASE(&gl_app2_cb.mbx, lt_leave_on_queue_cb);
            m_NCS_TASK_RELEASE(gl_app2_cb.task_hdl);
            (void)m_NCS_IPC_DETACH(&gl_app1_cb.mbx, NULL, NULL);
            m_NCS_IPC_RELEASE(&gl_app1_cb.mbx, lt_leave_on_queue_cb);
            return NCSCC_RC_FAILURE;
        }
        
        if ((res = m_NCS_TASK_START(gl_app1_cb.task_hdl)) != NCSCC_RC_SUCCESS)         
        {
            (void)m_NCS_IPC_DETACH(&gl_app2_cb.mbx, NULL, NULL);
            m_NCS_IPC_RELEASE(&gl_app2_cb.mbx, lt_leave_on_queue_cb);
            m_NCS_TASK_RELEASE(gl_app2_cb.task_hdl);
            (void)m_NCS_IPC_DETACH(&gl_app1_cb.mbx, NULL, NULL);
            m_NCS_IPC_RELEASE(&gl_app1_cb.mbx, lt_leave_on_queue_cb);
            m_NCS_TASK_RELEASE(gl_app1_cb.task_hdl);
            return NCSCC_RC_FAILURE;
        }
        
    m_NCS_TASK_SLEEP(5000);
    m_NCS_CONS_PRINTF("*** Waited for 5 seconds before destroying the tasks/mailboxes...\n");

        m_NCS_TASK_STOP(gl_app1_cb.task_hdl);
        m_NCS_TASK_RELEASE(gl_app1_cb.task_hdl);
        (void)m_NCS_IPC_DETACH(&gl_app1_cb.mbx, NULL, NULL);
        m_NCS_IPC_RELEASE(&gl_app1_cb.mbx, lt_leave_on_queue_cb);

        m_NCS_TASK_STOP(gl_app2_cb.task_hdl);
        m_NCS_TASK_RELEASE(gl_app2_cb.task_hdl);
        (void)m_NCS_IPC_DETACH(&gl_app2_cb.mbx, NULL, NULL);
        m_NCS_IPC_RELEASE(&gl_app2_cb.mbx, lt_leave_on_queue_cb);

    m_NCS_CONS_PRINTF("Destroyed the tasks for Applications 1 & 2...\n");
    m_NCS_CONS_PRINTF("Destroyed the mailboxes for Applications 1 & 2...\n");
    m_NCS_CONS_PRINTF("Returning from the test...\n");
        break;

    default:
        m_NCS_CONS_PRINTF("Test Num is wrong \n");
        return NCSCC_RC_FAILURE;
    }
    
    return NCSCC_RC_SUCCESS;
}

static void lt_test1_app1_process_main(SYSF_MBX *mbx)
{
    NCS_SEL_OBJ         mbx_fd = m_NCS_IPC_GET_SEL_OBJ(mbx);
    NCS_SEL_OBJ         highest_sel_obj;
    NCS_SEL_OBJ_SET     all_sel_obj;
    LT_EVT          *sreq = NULL;

    m_NCS_CONS_PRINTF("Application-1 thread created...\n");

    m_NCS_CONS_PRINTF("Application-1 : sending REQUEST to Application-2\n");
    sreq = m_MMGR_ALLOC_LT_EVT;
    m_NCS_MEMSET(sreq, '\0', sizeof(LT_EVT));
    sreq->evt = LT_TEST_EVT_APP1_REQ;
    sreq->info.app1_req = 0x01020304;
    if(m_NCS_IPC_SEND(&gl_app2_cb.mbx, sreq, NCS_IPC_PRIORITY_NORMAL) != NCSCC_RC_SUCCESS)
    {
    m_NCS_CONS_PRINTF("Application-1 : Evt post to Application-2 mailbox failed...\n");
    }

    /* Then start waiting for events on its own mailbox */
    m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
    m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);
    highest_sel_obj = mbx_fd;

    while (m_NCS_SEL_OBJ_SELECT(highest_sel_obj,&all_sel_obj,0,0,0) != -1)
    {
        m_NCS_CONS_PRINTF("Application-1 : Selection Object for Application-1 returned\n");
    if (m_NCS_SEL_OBJ_ISSET(mbx_fd,&all_sel_obj))
    {
            /* now got the IPC mail box event */
            test1_app1_process_mbx(&gl_app1_cb, mbx);
        }
    else
    {
            m_NCS_CONS_PRINTF("Application-1 : no fd set for select( )\n");
    }
    /* do the fd set for the select obj */
    m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);
    }
    return;
}

void test1_app1_process_mbx(LT_TEST_APP_CB *cb, SYSF_MBX *mbx)
{
  LT_EVT    *evt = NULL;

  /* We have multiple options here. We can process only one event
     at a time, or finish all events that are lying in the mailbox
     now itself. */
  while((evt = (LT_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt)))
  {
        m_NCS_CONS_PRINTF("Application-1 : NonBlock receive on mailbox successful\n");
    if(lt_test1_common_process_evt(cb, evt) != NCSCC_RC_SUCCESS)
    {
        /* Show up a DBG SINK. */
        (void)m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }
  }
  return;
}

static void lt_test1_app2_process_main(SYSF_MBX *mbx)
{
    NCS_SEL_OBJ         mbx_fd = m_NCS_IPC_GET_SEL_OBJ(mbx);
    NCS_SEL_OBJ         highest_sel_obj;
    NCS_SEL_OBJ_SET     all_sel_obj;

    m_NCS_CONS_PRINTF("Application-2 thread created...\n");

    /* Start waiting for events on its own mailbox */
    m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
    m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);
    highest_sel_obj = mbx_fd;

    while (m_NCS_SEL_OBJ_SELECT(highest_sel_obj,&all_sel_obj,0,0,0) != -1)
    {
        m_NCS_CONS_PRINTF("Application-2 : Selection Object returned\n");
    if (m_NCS_SEL_OBJ_ISSET(mbx_fd,&all_sel_obj))
    {
            /* now got the IPC mail box event */
            test1_app2_process_mbx(&gl_app2_cb, mbx);
        }
    else
    {
            m_NCS_CONS_PRINTF("Application-2 : no fd set for select( )\n");
    }
    /* do the fd set for the select obj */
    m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);
    }
    return;
}

void test1_app2_process_mbx(LT_TEST_APP_CB *cb, SYSF_MBX *mbx)
{
  LT_EVT    *evt = NULL;

  /* We have multiple options here. We can process only one event
     at a time, or finish all events that are lying in the mailbox
     now itself. */
  while((evt = (LT_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt)))
  {
        m_NCS_CONS_PRINTF("Application-2 : NonBlock receive on mailbox successful\n");
    if(lt_test1_common_process_evt(cb, evt) != NCSCC_RC_SUCCESS)
    {
        /* Show up a DBG SINK. */
        (void)m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }
  }
}

uns32 lt_test1_common_process_evt(LT_TEST_APP_CB *cb, LT_EVT   *evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   /* invoke the corresponding callback */
   switch (evt->evt)
   {
   case LT_TEST_EVT_APP1_REQ:
    /* App2 receives this event. */
        m_NCS_CONS_PRINTF("Application-2 : received REQUEST from Application-1\n");
        rc = lt_test_app2_handle_req(evt->evt, evt->info.app1_req);
      break;
      
   case LT_TEST_EVT_APP2_RESPONSE:
    /* App1 receives this event. */
        m_NCS_CONS_PRINTF("Application-1 : received RESPONSE from Application-2\n");
       rc = lt_test_app1_handle_response(evt->evt, evt->info.app2_resp);
      break;
   default:
      break;
   }
   /* free the callback info */
   if(evt)
      m_MMGR_FREE_LT_EVT(evt);

   return rc;
}

uns32 lt_test_app2_handle_req(LT_TEST_EVT_TYPE evt, uns32 data)
{
    LT_EVT *resp = NULL;
    if(evt == LT_TEST_EVT_APP1_REQ)
    {
        /* Send response to App1 */
        resp = m_MMGR_ALLOC_LT_EVT;
        m_NCS_MEMSET(resp, '\0', sizeof(LT_EVT));
        resp->info.app2_resp = 0x02030405;
        resp->evt = LT_TEST_EVT_APP2_RESPONSE;
        m_NCS_IPC_SEND(&gl_app1_cb.mbx, resp, NCS_IPC_PRIORITY_NORMAL);

        m_NCS_CONS_PRINTF("Application-2 : sent RESPONSE to Application-1\n");
    }
    else
    {
        m_NCS_CONS_PRINTF("Application-2 : received INVALID event..\n");
    }
    return NCSCC_RC_SUCCESS;
}

uns32 lt_test_app1_handle_response(LT_TEST_EVT_TYPE evt, uns32 data)
{
    if(evt == LT_TEST_EVT_APP2_RESPONSE)
    {
        m_NCS_CONS_PRINTF("Application-1 : processed RESPONSE from Application-2\n");
    }
    else
    {
        m_NCS_CONS_PRINTF("Application-1 : received INVALID event..\n");
    }
    return NCSCC_RC_SUCCESS;
}

/* Function to free each pending message in the mailbox */
static NCS_BOOL
lt_leave_on_queue_cb (void *arg1, void *arg2)
{
    LT_EVT *msg;

    if ((msg = (LT_EVT *)arg2) != NULL)
    {
        m_MMGR_FREE_LT_EVT(msg);
    }
    return TRUE;
}

