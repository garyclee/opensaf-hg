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

  DESCRIPTION:

  This file contains the CKPT interface routines for the AvSv toolkit sample 
  application. 
..............................................................................

  FUNCTIONS INCLUDED in this module:

  avsv_ckpt_init......................Creates & starts CKPT interface task.
  avsv_ckpt_process...................Entry point for the CKPT interface task.


******************************************************************************
*/


/* Common header files */
#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncssysf_tsk.h>
#include <opensaf/cpsv_papi.h>

/* SAF header files */
#include <saAis.h>
#include <saCkpt.h>
#include <saAmf.h>

/*############################################################################
                            Global Variables
############################################################################*/

/* AvSv CKPT interface task handle */
NCSCONTEXT gl_ckpt_task_hdl = 0;

/* CKPT intialization Handle */
SaCkptHandleT gl_ckpt_hdl = 0;

/* Checkpoint Handle */
SaCkptCheckpointHandleT checkpointHandle = 0;

/* CheckPoint Iter Handle */
SaCkptSectionIterationHandleT sectionIterationHandle=0;

/* CheckPoint Section Descriptor*/
SaCkptSectionDescriptorT secDesc;

/* CheckPoint Expiration Time */
SaTimeT timeout = 2000000000; /* expiration time */

/* Top level routine to start CKPT Interface task */
uns32 avsv_ckpt_init(void);

/* Routine to write the Data into CheckPoint */
void avsv_ckpt_data_write(uns32 *,uns32 *);

/* Routine to Read the Data from the CheckPoint */
void avsv_ckpt_data_read(void);

/* Arrival Callback Routine to Read the Data from CheckPoint */
void avsv_ckpt_ArrivalCallback(const SaCkptCheckpointHandleT ,
                                  SaCkptIOVectorElementT *,
                                  SaUint32T );

/*############################################################################
                            Macro Definitions
############################################################################*/

/* CKPT interface task related parameters  */
#define AVSV_CKPT_TASK_PRIORITY   (5)
#define AVSV_CKPT_STACKSIZE       NCS_STACKSIZE_HUGE

/* Macro to retrieve the CKPT version */
#define m_AVSV_CKPT_VER_GET(ver) \
{ \
   ver.releaseCode = 'B'; \
   ver.majorVersion = 0x02; \
   ver.minorVersion = 0x02; \
};
                                                                                                                             
#define APP_CPSV_CKPT_RET_TIME 0 /* In Nano Seconds (=60 Seconds) */
#define APP_CPSV_CKPT_SIZE 40       /* In Bytes */
#define APP_CPSV_SEC_MAX_SIZE 40     /* In Bytes */
                                                                                                                             
/*############################################################################
                       Extern Function Decalarations
############################################################################*/

extern uns32 gl_count1;
extern uns32 gl_count2;
extern SaAmfHAStateT gl_ha_state;

/*############################################################################
                       Static Function Decalarations
############################################################################*/

/* Entry level routine CKPT Interface task */
static void avsv_ckpt_process(void);


/****************************************************************************
  Name          : avsv_ckpt_init
 
  Description   : This routine creates & starts the CKPT interface task.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avsv_ckpt_init(void)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Create CKPT interface task */
   rc = m_NCS_TASK_CREATE ( (NCS_OS_CB)avsv_ckpt_process, (void *)0, "CKPT-INTF",
                            AVSV_CKPT_TASK_PRIORITY, AVSV_CKPT_STACKSIZE, 
                            &gl_ckpt_task_hdl );
   if ( NCSCC_RC_SUCCESS != rc )
   {
      goto err;
   }

   /* Start the task */
   rc = m_NCS_TASK_START (gl_ckpt_task_hdl);
   if ( NCSCC_RC_SUCCESS != rc )
   {
      goto err;
   }

   m_NCS_CONS_PRINTF("\n\n CKPT :: CKPT-INTF TASK CREATION SUCCESS !!! \n\n");

   return rc;

err:
   /* destroy the task */
   if (gl_ckpt_task_hdl) m_NCS_TASK_RELEASE(gl_ckpt_task_hdl);
   m_NCS_CONS_PRINTF("\n\n CKPT :: CKPT-INTF TASK CREATION FAILED !!! \n\n");

   return rc;
}


/***************************************************************
   Arrival Call back Function
   This function will be registered with the CKPT service after the
   initialize.
   In the Dispatch Routine, CPSv will invoke this routine on the
   arrival of the message
******************************************************************/
void avsv_ckpt_ArrivalCallback(const SaCkptCheckpointHandleT    ckptHandle,
                               SaCkptIOVectorElementT   *ioVector,
                               SaUint32T numberOfElements)
{
   SaCkptIOVectorElementT readVector[2];
   SaUint32T erroneousVectorIndex;
   uns32 read_buff1;
    uns32 read_buff2;
   SaAisErrorT rc;
                                                                                                                             
   /* Read the data only in standby state */
   if ( SA_AMF_HA_STANDBY != gl_ha_state )
      return;

   /*######################################################################
                      Demonstrating the use of saCkptCheckpointRead()
   ######################################################################*/
                                                                                                                             
   memset(&read_buff1, 0, sizeof(uns32));
   memset(readVector, 0, (sizeof(SaCkptIOVectorElementT) * 2));   
   memset(&erroneousVectorIndex, 0, sizeof(erroneousVectorIndex));
                                                                                                                             
   readVector[0].sectionId  = ioVector[0].sectionId;
   readVector[0].dataBuffer = (SaUint8T *)&read_buff1;
   readVector[0].dataSize   = ioVector[0].dataSize;
   readVector[0].dataOffset = ioVector[0].dataOffset;
  readVector[1].sectionId  = ioVector[1].sectionId;
   readVector[1].dataBuffer = (SaUint8T *)&read_buff2;
   readVector[1].dataSize   = ioVector[1].dataSize;
   readVector[1].dataOffset = ioVector[1].dataOffset;
                                                                                                                             
   rc = saCkptCheckpointRead(ckptHandle,readVector,2,&erroneousVectorIndex);
                                                                                                                             
   if(rc == SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("\n CKPT :: ReadVector -A ---%d from the CheckPoint", read_buff1);
    m_NCS_CONS_PRINTF("\n CKPT :: ReadVector -B ---%d from the CheckPoint", read_buff2);
   }

   gl_count1 = read_buff1;
     gl_count2 = read_buff2;
}

                                                                                                                             

/********************************************************************
 This function Reads the initial data avilable with CPSv on restart
 ********************************************************************/

void avsv_ckpt_data_read(void)
{
   SaCkptIOVectorElementT readVector[2];
   SaUint32T erroneousVectorIndex = 0;
   uns32 read_buff1;
    uns32 read_buff2;
   SaAisErrorT   rc;
   SaCkptSectionIdT sec_id={.idLen=8,.id="counter"};
                                                                                                                             
   /*######################################################################
                      Demonstrating the use of saCkptCheckpointRead()
   ######################################################################*/
                                                                                                                             
   readVector[0].sectionId = sec_id;
   readVector[0].dataBuffer =  (SaUint8T*)&read_buff1;
   readVector[0].dataSize   = sizeof(uns32);
   readVector[0].dataOffset = 0;
                                                                                                                             
   memset(&read_buff1, 0, sizeof(uns32));

   readVector[1].sectionId = sec_id;
   readVector[1].dataBuffer =  (SaUint8T*)&read_buff2;
   readVector[1].dataSize   = sizeof(uns32);
   readVector[1].dataOffset = 5;
   memset(&read_buff2, 0, sizeof(uns32));
   rc = saCkptCheckpointRead(checkpointHandle,
                     readVector,2,&erroneousVectorIndex);
                                                                                                                             
   if(rc == SA_AIS_OK)
   {  
      gl_count1 = read_buff1;
      m_NCS_CONS_PRINTF("\n CKPT :: ReadVector A --%d during initial read\n", read_buff1);
      gl_count2 = read_buff2;
      m_NCS_CONS_PRINTF("\n CKPT :: ReadVector B --%d during initial read\n", read_buff2);
   }
   else
     m_NCS_CONS_PRINTF("\n CKPT :: ReadVector A & B saCkptCheckpointRead fail -RC-%d during initial read\n", rc);

}

/****************************************************************************
  Name          : avsv_ckpt_process
 
  Description   : This routine is an entry point for the CKPT interface task.
 
  Arguments     : None.
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void avsv_ckpt_process (void)
{
   /* this is to allow to establish MDS session with AvSv */
   m_NCS_TASK_SLEEP(3000);
   
   SaCkptCallbacksT    *reg_callback_set=NULL;
   SaVersionT          ver;
   SaAisErrorT         rc;
   NCS_SEL_OBJ_SET     wait_sel_objs;
   SaSelectionObjectT  ckpt_sel_obj;
   NCS_SEL_OBJ         ckpt_ncs_sel_obj;
   NCS_SEL_OBJ         highest_sel_obj;
   SaNameT             ckptName;
   SaCkptCheckpointCreationAttributesT    ckptCreateAttr;
   SaCkptCheckpointOpenFlagsT             ckptOpenFlags;
   SaCkptSectionCreationAttributesT sectionCreationAttributes;
   
   /*sip_demo_ncs_agents_start();*/

   /*#########################################################################
                     Demonstrating the use of saCkptInitialize()
   #########################################################################*/
                                                                                                                             
                                                                                                                             
   /* Fill the CKPT version */
   m_AVSV_CKPT_VER_GET(ver);
                                                                                                                             
   /* Initialize CKPT */
   rc = saCkptInitialize(&gl_ckpt_hdl, reg_callback_set, &ver);
   if (rc != SA_AIS_OK)
      return;

   m_NCS_CONS_PRINTF("\n CKPT Initialization Done !!! \n CkptHandle: %d \n", (uns32)gl_ckpt_hdl);
                                                                                                                             
   /*#############################################################################
                Demonstrating the use of ncsCkptRegisterCkptArrivalCallback()
   ##############################################################################*/
   
   /* Register the Arrival Callback */
   rc = ncsCkptRegisterCkptArrivalCallback(gl_ckpt_hdl, avsv_ckpt_ArrivalCallback);
   if (rc != SA_AIS_OK)
      return;

   m_NCS_CONS_PRINTF("\n CKPT :: Registered Arrival Callback !!!\n ");

   /*#########################################################################
                  Demonstrating the use of saCkptCheckpointOpen()
   #########################################################################*/
                                                                                                                                
   /* Fill the Checkpoint Name */
   memset(&ckptName, 0, sizeof(ckptName));
   ckptName.length = 18;
   memcpy(ckptName.value, "ckpt_counter_info", 18);
                                                                                                                             

   /* Open the Checkpoint */
   ckptCreateAttr.creationFlags     = SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ALL_REPLICAS;
   ckptCreateAttr.checkpointSize    = APP_CPSV_CKPT_SIZE;
   ckptCreateAttr.retentionDuration = APP_CPSV_CKPT_RET_TIME;
   ckptCreateAttr.maxSections       = 2;
   ckptCreateAttr.maxSectionSize    = APP_CPSV_SEC_MAX_SIZE;
   ckptCreateAttr.maxSectionIdSize  = 10;
   SaCkptSectionIdT sec_id={.id="counter",.idLen=8};                                                                                                                          
   ckptOpenFlags = SA_CKPT_CHECKPOINT_CREATE|SA_CKPT_CHECKPOINT_READ|SA_CKPT_CHECKPOINT_WRITE;
   
   rc = saCkptCheckpointOpen(gl_ckpt_hdl, &ckptName, &ckptCreateAttr,
                             ckptOpenFlags, timeout, &checkpointHandle);
                                                                                                                             
   if(rc != SA_AIS_OK)
      return;

   m_NCS_CONS_PRINTF("\n CKPT :: Checkpoint Opened !!!\n");
#if 0 
  /* Fix for Ticket #11 */
   sectionCreationAttributes.sectionId = (SaCkptSectionIdT*) malloc(sizeof \
                                                        (SaCkptSectionIdT));
#endif
   sectionCreationAttributes.sectionId = &sec_id;
   sectionCreationAttributes.expirationTime = SA_TIME_END;
                                                                                                                             
   m_NCS_CONS_PRINTF("\n CKPT :: Ckpt Section Create being called ....\t");
   rc = saCkptSectionCreate(checkpointHandle,
                                  &sectionCreationAttributes,0,0);
   if(rc == SA_AIS_OK || rc == SA_AIS_ERR_EXIST )
      m_NCS_CONS_PRINTF("PASSED \n\n");
   else
      m_NCS_CONS_PRINTF("Failed rc = %d \n", rc);
                                                                                                                            

   /*#########################################################################
                  Demonstrating the use of saCkptSelectionObjectGet()
   #########################################################################*/

   m_NCS_TASK_SLEEP(2000);

   /* Get the selection object corresponding to this CKPT handle */
   rc = saCkptSelectionObjectGet(gl_ckpt_hdl, &ckpt_sel_obj);
   if (SA_AIS_OK != rc)
   {
      saCkptFinalize(gl_ckpt_hdl);
      return;
   }

   m_NCS_CONS_PRINTF("\n CKPT :: Selection Object Get Successful !!! \n");

   /***** Now wait (select) on CKPT selection object *****/

   /* Reset the wait select objects */
   m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);

   /* derive the fd for CKPT selection object */
   m_SET_FD_IN_SEL_OBJ((uns32)ckpt_sel_obj, ckpt_ncs_sel_obj);

   /* Set the CKPT select object on which AvSv toolkit application waits */
   m_NCS_SEL_OBJ_SET(ckpt_ncs_sel_obj, &wait_sel_objs);

   /* Get the highest select object in the set */
   highest_sel_obj = ckpt_ncs_sel_obj;

   /* Now wait forever */
   while (m_NCS_SEL_OBJ_SELECT(highest_sel_obj, &wait_sel_objs, NULL, NULL, NULL) != -1)
   {
      /*######################################################################
                      Demonstrating the use of saCkptDispatch()
      ######################################################################*/

      /* There is an CKPT callback waiting to be be processed. Process it */
      rc = saCkptDispatch(gl_ckpt_hdl, SA_DISPATCH_ONE);
      if (SA_AIS_OK != rc)
      {
         saCkptFinalize(gl_ckpt_hdl);
         break;
      }

      /* Again set CKPT select object to wait for another callback */
      m_NCS_SEL_OBJ_SET(ckpt_ncs_sel_obj, &wait_sel_objs);
   }

   m_NCS_CONS_PRINTF("\n\n DEMO OVER !!! \n\n");

   return;
}


/***************************************************************
 This routine writes the CKPT Data Received from the application
 ***************************************************************/

void avsv_ckpt_data_write(uns32 *write_buff1,uns32 *write_buff2)
{
   SaCkptIOVectorElementT writeVector[2];
   SaUint32T erroneousVectorIndex;
   SaAisErrorT rc;
   SaCkptSectionIdT sec_id = {.id="counter",.idLen=8};

   /*######################################################################
                      Demonstrating the use of saCkptCheckpointWrite()
   ######################################################################*/
                                                                                                                             
   /* Build the writeVector */
   writeVector[0].sectionId = sec_id;
   writeVector[0].dataBuffer = (SaUint8T*)write_buff1;
   writeVector[0].dataSize = sizeof(uns32);
   writeVector[0].dataOffset = 0;
   writeVector[0].readSize = 0;
   writeVector[1].sectionId = sec_id;
   writeVector[1].dataBuffer = (SaUint8T*)write_buff2;
   writeVector[1].dataSize = sizeof(uns32);
   writeVector[1].dataOffset = 5;
   writeVector[1].readSize = 0;
                                                                                                                             
   rc = saCkptCheckpointWrite(checkpointHandle,writeVector,2,&erroneousVectorIndex);
   if(rc == SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\n CKPT :: writeVector-A-- %d to the CheckPoint\n", *write_buff1);
      m_NCS_CONS_PRINTF("\n CKPT :: writeVectort-B-- %d to the CheckPoint\n", *write_buff2);
   }
   return;
}



