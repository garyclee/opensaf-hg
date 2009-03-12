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
MODULE NAME: cpsv_test_app.c  (CPSv Test Functions)

  .............................................................................
  DESCRIPTION:
  
    CPSv routines required for Demo Applications.
    
      
******************************************************************************/
#include <opensaf/ncsgl_defs.h>
#include <opensaf/os_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncssysf_tsk.h>

#include <saAis.h>
#include <saCkpt.h>

void AppCkptOpenCallback(SaInvocationT invocation, SaCkptCheckpointHandleT checkpointHandle, SaAisErrorT error);
void AppCkptSyncCallback(SaInvocationT invocation, SaAisErrorT error);
void cpsv_test_sync_app_process(NCSCONTEXT info);


void AppCkptOpenCallback(SaInvocationT invocation, SaCkptCheckpointHandleT checkpointHandle, SaAisErrorT error)
{
   if (error != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("Checkpoint Open Async callback unsuccessful\n");
      return;
   }
   else
   {
                
      m_NCS_CONS_PRINTF("Checkpoint Open Async callback success and ckpt_hdl %llu \n",checkpointHandle);
      return;
   }
}
void AppCkptSyncCallback(SaInvocationT invocation, SaAisErrorT error)
{
   if (error != SA_AIS_OK)
   {
      printf("Checkpoint Sync Callback unsuccessful\n");
      return;
   }
   else
   {
      printf("Checkpoint Sync Callback success\n");
      return;
   }
}

/****************************************************************************
 * Name          : cpsv_test_sync_app_process
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
void cpsv_test_sync_app_process(NCSCONTEXT info)
{
   SaCkptHandleT ckptHandle;
   SaCkptCheckpointHandleT  checkpointHandle;
   SaCkptCallbacksT callbk;
   SaVersionT version;
   SaNameT ckptName;
   SaAisErrorT rc;
   SaCkptCheckpointCreationAttributesT ckptCreateAttr;
   SaCkptCheckpointOpenFlagsT ckptOpenFlags;
   SaCkptSectionCreationAttributesT sectionCreationAttributes;
   SaCkptIOVectorElementT writeVector, readVector;
   SaUint32T erroneousVectorIndex;
   void *initialData = "Default data in the section";
   uns8 read_buff[100] = {0};
   SaTimeT timeout = 1000000000;
   uns32  temp_var = (uns32)(long)info; 


   memset(&ckptName, 0, sizeof(ckptName));
   ckptName.length = 7;
   memcpy(ckptName.value,"sample",7);

   callbk.saCkptCheckpointOpenCallback = AppCkptOpenCallback;
   callbk.saCkptCheckpointSynchronizeCallback = AppCkptSyncCallback;
   version.releaseCode= 'B';
   version.majorVersion = 2;
   version.minorVersion = 2;
   
    
   m_NCS_TASK_SLEEP(1000);                                                                                                                                                                

   m_NCS_CONS_PRINTF("Ckpt Initialising being called ....\t");
   rc = saCkptInitialize(&ckptHandle,&callbk,&version);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");

   ckptCreateAttr.creationFlags = SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA;
   ckptCreateAttr.checkpointSize = 1024;
   ckptCreateAttr.retentionDuration= 100000;
   ckptCreateAttr.maxSections= 2;
   ckptCreateAttr.maxSectionSize = 700;
   ckptCreateAttr.maxSectionIdSize = 4;

   ckptOpenFlags = SA_CKPT_CHECKPOINT_CREATE|SA_CKPT_CHECKPOINT_READ|SA_CKPT_CHECKPOINT_WRITE;
                                                                                                                                                                      
   m_NCS_CONS_PRINTF("Ckpt Open being called ....\t");
   rc = saCkptCheckpointOpen(ckptHandle,&ckptName,&ckptCreateAttr,ckptOpenFlags,timeout,&checkpointHandle);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");


   
   if(temp_var == 1)
   {
 
      m_NCS_CONS_PRINTF("Ckpt Active Replica Set being called ....\t");


      rc = saCkptActiveReplicaSet(checkpointHandle);
      if(rc == SA_AIS_OK)
         m_NCS_CONS_PRINTF("PASSED \n");
      else
         m_NCS_CONS_PRINTF("Failed \n");
    

   sectionCreationAttributes.sectionId = (SaCkptSectionIdT*) malloc(sizeof \
                                (SaCkptSectionIdT));
   sectionCreationAttributes.sectionId->id = "11";
   sectionCreationAttributes.sectionId->idLen = 2;
   sectionCreationAttributes.expirationTime = 3600000000000ll;   /* One Hour */

   m_NCS_CONS_PRINTF("Ckpt Section Create being called ....\t");
   rc = saCkptSectionCreate(checkpointHandle,&sectionCreationAttributes,initialData,28);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");

  
      writeVector.sectionId.id = "11";
      writeVector.sectionId.idLen = 2;
      writeVector.dataBuffer = "The Checkpoint Service provides a facility for processes to record checkpoint data";
      writeVector.dataSize = strlen(writeVector.dataBuffer);
      writeVector.dataOffset = 0;
      writeVector.readSize = 0;

      m_NCS_CONS_PRINTF("Ckpt Write being called ....\t");
      rc = saCkptCheckpointWrite(checkpointHandle,&writeVector,1,&erroneousVectorIndex);
      printf("%s\n",(char *)writeVector.dataBuffer);
      if(rc == SA_AIS_OK)
         m_NCS_CONS_PRINTF("PASSED \n");
      else
         m_NCS_CONS_PRINTF("Failed \n");
      m_NCS_TASK_SLEEP(10000); 
    }
    else
    {  
       m_NCS_TASK_SLEEP(4000);
       readVector.sectionId.id = "11";
       readVector.sectionId.idLen = 2;
       readVector.dataBuffer = read_buff;
       readVector.dataSize = 90;
       readVector.dataOffset = 0;                                                                                                                                                                 
       m_NCS_CONS_PRINTF("Ckpt Read being called ....\t");
       rc = saCkptCheckpointRead(checkpointHandle,&readVector,1,&erroneousVectorIndex);
       printf("%s\n",(char *)readVector.dataBuffer);
       if(rc == SA_AIS_OK)
          m_NCS_CONS_PRINTF("PASSED \n");
       else
          m_NCS_CONS_PRINTF("Failed \n");   
   }                                                                                                                                                                                                                                                       
   m_NCS_CONS_PRINTF("Ckpt Synchronize being called ....\t");
   rc = saCkptCheckpointSynchronize(checkpointHandle,timeout);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");
                                             
   if(temp_var==1)
   {
                                                                                                                        
      m_NCS_CONS_PRINTF("Ckpt Unlink being called ....\t");
      rc = saCkptCheckpointUnlink(ckptHandle,&ckptName);
      if(rc == SA_AIS_OK)
         m_NCS_CONS_PRINTF("PASSED \n");
      else
         m_NCS_CONS_PRINTF("Failed \n");
   }                                                                                                                                                                   

   m_NCS_CONS_PRINTF("Ckpt Close being called ....\t");
   rc = saCkptCheckpointClose(checkpointHandle);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");
                                                                                                                                                                      

   m_NCS_CONS_PRINTF("Ckpt Finalize being called ....\t");
   rc = saCkptFinalize(ckptHandle);
   if(rc == SA_AIS_OK)
      m_NCS_CONS_PRINTF("PASSED \n");
   else
      m_NCS_CONS_PRINTF("Failed \n");
                                                                                                                                                                      
   m_NCS_TASK_SLEEP(100000);
  return;
}


