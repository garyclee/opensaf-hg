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
MODULE NAME: glsv_test_app.c  (GLSv Test Functions)

  .............................................................................
  DESCRIPTION:
  
    GLSv routines required for Demo Applications.
    
      
******************************************************************************/
#include <opensaf/ncsgl_defs.h>
#include <opensaf/os_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncssysf_tsk.h>

#include <saAis.h>
#include <saLck.h>

void glsv_test_sync_app1_process(NCSCONTEXT info);
void glsv_test_neagtive_handle_process(NCSCONTEXT info);
void glsv_test_neagtive_resource_handle_process(NCSCONTEXT info);
void glsv_test_sync_resource_open_app1_process(NCSCONTEXT info);
void glsv_test_sync_app1_pre_purge_process(NCSCONTEXT info);
void glsv_test_sync_app1_post_purge_process(NCSCONTEXT info); 
void glsv_test_sync_app_res_timeout_process(NCSCONTEXT info); 
void glsv_test_sync_app_lock_timeout_non_master_process(NCSCONTEXT info);
void glsv_test_sync_app_lock_timeout_master_process(NCSCONTEXT info);
void glsv_test_sync_app_unlock_timeout_process(NCSCONTEXT info);
void glsv_test_sync_master_change_process(NCSCONTEXT info);
void glsv_test_sync_big_app1_process(NCSCONTEXT info);

static void App1_ResourceOpenCallbackT(SaInvocationT invocation,
                                       SaLckResourceHandleT resourceId,
                                       SaAisErrorT error)
{
   SaLckResourceHandleT   *my_res_id=(SaLckResourceHandleT*)((long)invocation);
   if(error == SA_AIS_OK)
   {
      *my_res_id = resourceId;
      m_NCS_CONS_PRINTF(" App1- resource Open Callback Success - resid %llu \n",resourceId);
   }
   else
      m_NCS_CONS_PRINTF(" App1- resource Open Callback Failed - Error - %d\n",error); 

}


static void App1_LockGrantCallbackT(SaInvocationT invocation,
                                    SaLckLockStatusT lockStatus,
                                    SaAisErrorT error)
{
   SaLckLockIdT   *lock_id = (SaLckLockIdT*)((long)invocation);
   if(error == SA_AIS_OK && lockStatus == SA_LCK_LOCK_GRANTED)
   {
      m_NCS_CONS_PRINTF(" App1- Lock Grant Callback Success - lockid %p\n",lock_id);
   }
   else
      m_NCS_CONS_PRINTF(" App1- Lock Grant Callback  Failed - status %d, error %d \n",
      lockStatus, error);
}

static void App1_LockWaiterCallbackT(SaLckWaiterSignalT invocation,
                                     SaLckLockIdT lockId,
                                     SaLckLockModeT modeHeld,
                                     SaLckLockModeT modeRequested)
{
   m_NCS_CONS_PRINTF(" App1- Lock Waiter Callback - lockid %llu ",lockId);
   if(modeHeld == SA_LCK_PR_LOCK_MODE)
      m_NCS_CONS_PRINTF(" ModeHeld - Shared");
   else
      m_NCS_CONS_PRINTF(" ModeHeld - Write");
   
   if(modeRequested == SA_LCK_PR_LOCK_MODE)
      m_NCS_CONS_PRINTF(" ModeRequested - Shared");
   else
      m_NCS_CONS_PRINTF(" ModeRequested - Write");
   
}

static void App1_ResourceUnlockCallbackT(SaInvocationT invocation,
                                         SaAisErrorT error)
{
   if(error == SA_AIS_OK)
      m_NCS_CONS_PRINTF(" App1- UnLock Callback Success ");
   else
      m_NCS_CONS_PRINTF(" App1- UnLock Callback Failed ");
   
}


/****************************************************************************
 * Name          : glsv_test_sync_app1_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_sync_app1_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;
   SaLckLockStatusT     status;
   SaLckLockIdT         lockid;

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'B';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"sample",7);

   m_NCS_CONS_PRINTF("Lock Initialising being called ....\t");
   rc = saLckInitialize(&hdl1,&callbk,&version);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");

   m_NCS_CONS_PRINTF("Lock selection object get  being called ....");
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");
   
   m_NCS_CONS_PRINTF("Resource Open  being called ....");
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED res_id = %lu\n",res_id);
   else
      m_NCS_CONS_PRINTF("Failed \n");

   m_NCS_CONS_PRINTF("Resource Lock for Exclusive lock being called ....");
   rc = saLckResourceLock(res_id,&lockid,2,0,0,10000000000ll,&status);
   if(rc == SA_AIS_OK && status == SA_LCK_LOCK_GRANTED)
      m_NCS_CONS_PRINTF("PASSED lock_id = %lu\n",lockid);
   else
      m_NCS_CONS_PRINTF("Failed \n");
   
   m_NCS_CONS_PRINTF("Waiting for 5 seconds....\n");
   m_NCS_TASK_SLEEP(5000);

   m_NCS_CONS_PRINTF("Resource unlock being called ....");
   rc = saLckResourceUnlock(lockid,10000000000ll);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");
   
   m_NCS_CONS_PRINTF("resource close being called ....");
   rc = saLckResourceClose(res_id);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");
   
   m_NCS_CONS_PRINTF("Lock Finalize being called ....");
   rc = saLckFinalize(hdl1);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");

   m_NCS_TASK_SLEEP(2000);
}


/****************************************************************************
 * Name          : glsv_test_neagtive_handle_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_neagtive_handle_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaVersionT           wrong_version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;
   SaLckLockStatusT     status;
   SaLckLockIdT         lockid;


   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   wrong_version.releaseCode= 'A';
   wrong_version.majorVersion = 1;
   wrong_version.minorVersion = 0;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"Ripple",7);

   m_NCS_CONS_PRINTF("\nCalling  Lock Initialization with a wrong inputs \n");
   rc = saLckInitialize(NULL,NULL,NULL);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Initialization with a wrong Version \n");
   rc = saLckInitialize(&hdl1,&callbk,&wrong_version);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Initialization with a Correct Version \n");
   rc = saLckInitialize(&hdl1,&callbk,&version);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);
   
   m_NCS_CONS_PRINTF("\nCalling  Lock Selection with a wrong handle \n");
   rc = saLckSelectionObjectGet(hdl1+1, &obj1);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Selection with a wrong input \n");
   rc = saLckSelectionObjectGet(hdl1, NULL);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Selection with a correct handle \n");
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource open with a wrong name \n");
   rc = saLckResourceOpen(hdl1,NULL,SA_LCK_RESOURCE_CREATE,10000000000ll,NULL);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource open with a wrong handle \n");
   rc = saLckResourceOpen(hdl1+1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource Async open with a wrong handle \n");
   rc = saLckResourceOpenAsync(hdl1+1,100,&res_name,SA_LCK_RESOURCE_CREATE);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource Async open with a wrong name \n");
   rc = saLckResourceOpenAsync(hdl1,100,NULL,SA_LCK_RESOURCE_CREATE);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource open with Zero Timeout \n");
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,0,&res_id);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource open with Correct handle \n");
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource Async open with InCorrect handle \n");
   rc = saLckResourceLockAsync(res_id+1,100,&lockid,0,0,0);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource Lock with NULL params \n");
   rc = saLckResourceLock(res_id,NULL,4,0,0,10000000000ll,NULL);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource Lock with Incorrect lockmode \n");
   rc = saLckResourceLock(res_id,&lockid,4,0,0,10000000000ll,&status);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource Async open with InCorrect lockmode \n");
   rc = saLckResourceLockAsync(res_id,100,&lockid,4,0,0);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource Lock with Incorrect LockFlags \n");
   rc = saLckResourceLock(res_id,&lockid,2,10,0,10000000000ll,&status);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource Async open with InCorrect LockFlags \n");
   rc = saLckResourceLockAsync(res_id,100,&lockid,0,10,0);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource Lock with correct parameters \n");
   rc = saLckResourceLock(res_id,&lockid,2,0,0,10000000000ll,&status);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource UnLock with Incorrect handle \n");
   rc = saLckResourceUnlock(lockid+1,10000000000ll);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource UnLock with Incorrect handle \n");
   rc = saLckResourceUnlockAsync(100,lockid);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Resource UnLock with correct handle \n");
   rc = saLckResourceUnlock(lockid,10000000000ll);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Finalize with a wrong handle \n");
   rc = saLckFinalize(hdl1+1);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Lock Finalize with a correct handle \n");
   rc = saLckFinalize(hdl1);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("sorry Negative Handle App is quiting bc of mail box problem pls destroy it\n");
}

/****************************************************************************
 * Name          : glsv_test_neagtive_resource_handle_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_neagtive_resource_handle_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;
   SaLckLockStatusT     status;
   SaLckLockIdT         lockid;

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"sample",7);

   rc = saLckInitialize(&hdl1,&callbk,&version);
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
   
   m_NCS_CONS_PRINTF("\nCalling  Resource Lock with a wrong resource handle \n");
   rc = saLckResourceLock(res_id+1,&lockid,2,0,0,10000000000ll,&status);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Resource Lock with a correct resource handle \n");
   rc = saLckResourceLock(res_id,&lockid,2,0,0,10000000000ll,&status);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Resource Close with a wrong resource handle \n");
   rc = saLckResourceClose(res_id+1);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   m_NCS_CONS_PRINTF("\nCalling  Resource Close with a correct resource handle \n");
   rc = saLckResourceClose(res_id);
   m_NCS_CONS_PRINTF("Status - %d\n",rc);

   rc = saLckFinalize(hdl1);

   m_NCS_CONS_PRINTF("sorry Negative Resource handle App is quiting !\n");
}


/****************************************************************************
 * Name          : glsv_test_sync_resource_open_app1_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_sync_resource_open_app1_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"sample",7);

   rc = saLckInitialize(&hdl1,&callbk,&version);
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
}


/****************************************************************************
 * Name          : glsv_test_sync_app1_pre_purge_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_sync_app1_pre_purge_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;
   SaLckLockStatusT     status;
   SaLckLockIdT         lockid;

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"sample",7);

   rc = saLckInitialize(&hdl1,&callbk,&version);
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
   rc = saLckResourceLock(res_id,&lockid,2,0x0,0,10000000000ll,&status);
   m_NCS_TASK_SLEEP(10000);
   rc = saLckResourceClose(res_id);
   rc = saLckFinalize(hdl1);

   m_NCS_CONS_PRINTF("sorry Sync App is quiting bc of mail box problem pls destroy it\n");
}

/****************************************************************************
 * Name          : glsv_test_sync_app1_post_purge_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_sync_app1_post_purge_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"sample",7);

   rc = saLckInitialize(&hdl1,&callbk,&version);
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
   rc = saLckLockPurge(res_id);
   rc = saLckResourceClose(res_id);
   rc = saLckFinalize(hdl1);

   m_NCS_CONS_PRINTF("sorry Sync App is quiting bc of mail box problem pls destroy it\n");
}

/****************************************************************************
 * Name          : glsv_test_sync_app_res_timeout_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_sync_app_res_timeout_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"sample",7);

   rc = saLckInitialize(&hdl1,&callbk,&version);
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,1000000,&res_id);
   rc = saLckFinalize(hdl1);

   m_NCS_CONS_PRINTF("sorry Sync resource timeout App is quiting bc of mail box problem pls destroy it\n");
}


/****************************************************************************
 * Name          : glsv_test_sync_app_lock_timeout_non_master_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_sync_app_lock_timeout_non_master_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;
   SaLckLockStatusT     status;
   SaLckLockIdT         lockid;

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"sample",7);

   rc = saLckInitialize(&hdl1,&callbk,&version);
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
   rc = saLckResourceLock(res_id,&lockid,2,0,0,1000000,&status);
   rc = saLckResourceClose(res_id);
   rc = saLckFinalize(hdl1);

   m_NCS_CONS_PRINTF("sorry lock timeout App is quiting bc of mail box problem pls destroy it\n");
}

/****************************************************************************
 * Name          : glsv_test_sync_app_lock_timeout_master_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_sync_app_lock_timeout_master_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;
   SaLckLockStatusT     status;
   SaLckLockIdT         lockid;

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"sample",7);

   rc = saLckInitialize(&hdl1,&callbk,&version);
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
   rc = saLckResourceLock(res_id,&lockid,2,0x0,0,1000000,&status);
   m_NCS_CONS_PRINTF("\n Calling the same lock again with a timeout to test the tmr failure \n");
   rc = saLckResourceLock(res_id,&lockid,2,0,0,1000000,&status);
   rc = saLckResourceUnlock(lockid,10000000000ll);
   rc = saLckResourceClose(res_id);
   rc = saLckFinalize(hdl1);

   m_NCS_CONS_PRINTF("sorry lock timeout App is quiting bc of mail box problem pls destroy it\n");
}


/****************************************************************************
 * Name          : glsv_test_sync_app_unlock_timeout_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_sync_app_unlock_timeout_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;
   SaLckLockStatusT     status;
   SaLckLockIdT         lockid;

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"sample",7);

   rc = saLckInitialize(&hdl1,&callbk,&version);
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
   rc = saLckResourceLock(res_id,&lockid,2,0,0,10000000000ll,&status);
   m_NCS_TASK_SLEEP(10000);
   rc = saLckResourceUnlock(lockid,1000000);
   rc = saLckResourceClose(res_id);
   rc = saLckFinalize(hdl1);

   m_NCS_CONS_PRINTF("sorry Sync App is quiting bc of mail box problem pls destroy it\n");
}


/****************************************************************************
 * Name          : glsv_test_sync_master_change_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_sync_master_change_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1;
   SaLckResourceHandleT     res_id;
   SaLckLockStatusT     status;
   SaLckLockIdT         lockid;

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name, 0, sizeof(res_name));
   res_name.length = 7;
   memcpy(res_name.value,"sample",7);

   rc = saLckInitialize(&hdl1,&callbk,&version);
   rc = saLckSelectionObjectGet(hdl1, &obj1);
   rc = saLckResourceOpen(hdl1,&res_name,SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id);
   rc = saLckResourceLock(res_id,&lockid,2,0x0,0,1000000000000ll,&status);
   m_NCS_TASK_SLEEP(50000);
   rc = saLckResourceUnlock(lockid,10000000000ll);
   rc = saLckResourceClose(res_id);
   rc = saLckFinalize(hdl1);

   m_NCS_CONS_PRINTF("sorry Sync App is quiting bc of mail box problem pls destroy it\n");
}


/****************************************************************************
 * Name          : glsv_test_sync_big_app1_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during 
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glsv_test_sync_big_app1_process(NCSCONTEXT info)
{
   SaLckHandleT         hdl1,hdl2;
   SaLckCallbacksT      callbk;
   SaVersionT           version;
   SaNameT              res_name[5]; 
   SaAisErrorT             rc;
   SaSelectionObjectT   obj1,obj2;
   SaLckResourceHandleT     res_id[5];
   SaLckLockStatusT     status;
   SaLckLockIdT         lockid[5];

   
   callbk.saLckResourceOpenCallback = App1_ResourceOpenCallbackT;
   callbk.saLckLockGrantCallback  = App1_LockGrantCallbackT;
   callbk.saLckLockWaiterCallback = App1_LockWaiterCallbackT;
   callbk.saLckResourceUnlockCallback = App1_ResourceUnlockCallbackT;

   version.releaseCode= 'A';
   version.majorVersion = 1;
   version.minorVersion = 1;

   memset(&res_name[0], 0, sizeof(SaNameT));
   res_name[0].length = 7;
   memcpy(res_name[0].value,"sample",7);

   memset(&res_name[1], 0, sizeof(SaNameT));
   res_name[1].length = 7;
   memcpy(res_name[1].value,"simple",7);

   memset(&res_name[2], 0, sizeof(SaNameT));
   res_name[2].length = 16;
   memcpy(res_name[2].value,"AsJunkAsItCanBe",16);

   memset(&res_name[3], 0, sizeof(SaNameT));
   res_name[3].length = 4;
   memcpy(res_name[3].value,"cat",4);

   memset(&res_name[4], 0, sizeof(SaNameT));
   res_name[4].length = 10;
   memcpy(res_name[4].value,"BootyBump",10);


   rc = saLckInitialize(&hdl1,&callbk,&version);
   rc = saLckInitialize(&hdl2,&callbk,&version);

   rc = saLckSelectionObjectGet(hdl1, &obj1);
   rc = saLckSelectionObjectGet(hdl2, &obj2);

   rc = saLckResourceOpen(hdl1,&res_name[0],SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id[0]);
   rc = saLckResourceOpen(hdl1,&res_name[1],SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id[1]);
   rc = saLckResourceOpen(hdl1,&res_name[2],SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id[2]);
   rc = saLckResourceOpen(hdl1,&res_name[3],SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id[3]);
   rc = saLckResourceOpen(hdl1,&res_name[4],SA_LCK_RESOURCE_CREATE,10000000000ll,&res_id[4]);

   rc = saLckResourceLock(res_id[0],&lockid[0],1,0,0,10000000000ll,&status);
   rc = saLckResourceLock(res_id[1],&lockid[1],1,0,0,10000000000ll,&status);
   rc = saLckResourceLock(res_id[2],&lockid[2],1,0,0,10000000000ll,&status);
   rc = saLckResourceLock(res_id[3],&lockid[3],2,0,0,10000000000ll,&status);
   rc = saLckResourceLock(res_id[4],&lockid[4],2,0,0,10000000000ll,&status);

   m_NCS_TASK_SLEEP(10000);

   rc = saLckResourceUnlock(lockid[0],10000000000ll);
   rc = saLckResourceUnlock(lockid[1],10000000000ll);
   rc = saLckResourceUnlock(lockid[2],10000000000ll);
   rc = saLckResourceUnlock(lockid[3],10000000000ll);
   rc = saLckResourceUnlock(lockid[4],10000000000ll);

   rc = saLckResourceClose(res_id[0]);
   rc = saLckResourceClose(res_id[1]);
   rc = saLckResourceClose(res_id[2]);
   rc = saLckResourceClose(res_id[3]);
   rc = saLckResourceClose(res_id[4]);


   rc = saLckFinalize(hdl1);

   m_NCS_CONS_PRINTF("sorry BIG Sync App is quiting bc of mail box problem pls destroy it\n");
}

