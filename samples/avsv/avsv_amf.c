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

  This file contains the AMF interface routines for the AvSv toolkit sample 
  application. It demonstrates the following:
  a) usage of AMF APIs.
  b) certain AvSv features:
      i)  AMF-invoked healthcheck.
      ii) Failover from an active to the standby entity (triggered through 
          Error Report).
..............................................................................

  FUNCTIONS INCLUDED in this module:

  avsv_amf_init........................Creates & starts AMF interface task.
  avsv_amf_process.....................Entry point for the AMF interface task.
  avsv_amf_csi_set_callback............CSI-Set callback.
  avsv_amf_csi_remove_callback.........CSI-Remove callback.
  avsv_amf_healthcheck_callback........Component-Healthcheck callback.
  avsv_amf_comp_terminate_callback.....Component-Terminate callback.
  avsv_amf_protection_group_callback...Protection Group callback.


******************************************************************************
*/


/* Common header files */
#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncssysf_tsk.h>

/* SAF header files */
#include <saAis.h>
#include <saAmf.h>


/*############################################################################
                            Global Variables
############################################################################*/

/* AvSv AMF interface task handle */
NCSCONTEXT gl_amf_task_hdl = 0;

/* AMF Handle */
SaAmfHandleT gl_amf_hdl = 0;

/* Component Name (provided by AMF) */
SaNameT gl_comp_name;

/* HealthCheck Key on which healthcheck is started */
SaAmfHealthcheckKeyT gl_healthcheck_key = {"A9FD64E12C", 10};

/* HA state of the application */
SaAmfHAStateT gl_ha_state = 0;

/* Canned strings for HA State */
const char *ha_state_str[] =
{
   "None",
   "Active",    /* SA_AMF_HA_ACTIVE       */
   "Standby",   /* SA_AMF_HA_STANDBY      */
   "Quiesced",  /* SA_AMF_HA_QUIESCED     */
   "Quiescing"  /* SA_AMF_HA_QUIESCING    */
};

/* Canned strings for CSI Flags */
const char *csi_flag_str[] =
{
   "None",
   "Add One",    /* SA_AMF_CSI_ADD_ONE    */
   "Target One", /* SA_AMF_CSI_TARGET_ONE */
   "None",
   "Target All", /* SA_AMF_CSI_TARGET_ALL */
};

/* Canned strings for Transition Descriptor */
const char *trans_desc_str[] =
{
   "None",
   "New Assign",   /* SA_AMF_CSI_NEW_ASSIGN   */
   "Quiesced",     /* SA_AMF_CSI_QUIESCED     */
   "Not Quiesced", /* SA_AMF_CSI_NOT_QUIESCED */
   "Still Active"  /* SA_AMF_CSI_STILL_ACTIVE */
};

/* Canned strings for Protection Group Change */
const char *pg_change_str[] =
{
   "None",
   "No Change",    /* SA_AMF_PROTECTION_GROUP_NO_CHANGE    */
   "Added",        /* SA_AMF_PROTECTION_GROUP_ADDED        */
   "Removed",      /* SA_AMF_PROTECTION_GROUP_REMOVED      */
   "State Change"  /* SA_AMF_PROTECTION_GROUP_STATE_CHANGE */
};



/*############################################################################
                            Macro Definitions
############################################################################*/

/* AMF interface task related parameters  */
#define AVSV_AMF_TASK_PRIORITY   (5)
#define AVSV_AMF_STACKSIZE       NCS_STACKSIZE_HUGE

/* Macro to retrieve the AMF version */
#define m_AVSV_AMF_VER_GET(ver) \
{ \
   ver.releaseCode = 'B'; \
   ver.majorVersion = 0x01; \
   ver.minorVersion = 0x01; \
};

/* No of times healthcheck is performed */
#define AVSV_HEALTHCHECK_CALLBACK_MAX_COUNT 10


/*############################################################################
                       Extern Function Decalarations
############################################################################*/

/* Top level routine to start AMF Interface task */
extern uns32 avsv_amf_init(void);

/* Routine to initiate initial data read on standby assignment */
extern void avsv_ckpt_data_read(void);

/*############################################################################
                       Static Function Decalarations
############################################################################*/

/* Entry level routine AMF Interface task */
static void avsv_amf_process(void);

/* 'CSI Set' callback that is registered with AMF */
static void avsv_amf_csi_set_callback(SaInvocationT, 
                                      const SaNameT *,
                                      SaAmfHAStateT,
                                      SaAmfCSIDescriptorT);

/* 'CSI Remove' callback that is registered with AMF */
static void avsv_amf_csi_remove_callback(SaInvocationT, 
                                         const SaNameT *,
                                         const SaNameT *,
                                         SaAmfCSIFlagsT);

/* 'HealthCheck' callback that is registered with AMF */
static void avsv_amf_healthcheck_callback(SaInvocationT, 
                                          const SaNameT *,
                                          SaAmfHealthcheckKeyT *);

/* 'Component Terminate' callback that is registered with AMF */
static void avsv_amf_comp_terminate_callback(SaInvocationT, const SaNameT *);

/* 'Protection Group' callback that is registered with AMF */
static void avsv_amf_protection_group_callback(const SaNameT *,
                                               SaAmfProtectionGroupNotificationBufferT *,
                                               SaUint32T,
                                               SaAisErrorT);



/****************************************************************************
  Name          : avsv_amf_init
 
  Description   : This routine creates & starts the AMF interface task.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avsv_amf_init(void)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Create AMF interface task */
   rc = m_NCS_TASK_CREATE ( (NCS_OS_CB)avsv_amf_process, (void *)0, "AMF-INTF",
                            AVSV_AMF_TASK_PRIORITY, AVSV_AMF_STACKSIZE, 
                            &gl_amf_task_hdl );
   if ( NCSCC_RC_SUCCESS != rc )
   {
      goto err;
   }

   /* Start the task */
   rc = m_NCS_TASK_START (gl_amf_task_hdl);
   if ( NCSCC_RC_SUCCESS != rc )
   {
      goto err;
   }

   m_NCS_CONS_PRINTF("\n\n AMF-INTF TASK CREATION SUCCESS !!! \n\n");

   return rc;

err:
   /* destroy the task */
   if (gl_amf_task_hdl) m_NCS_TASK_RELEASE(gl_amf_task_hdl);
   m_NCS_CONS_PRINTF("\n\n AMF-INTF TASK CREATION FAILED !!! \n\n");

   return rc;
}


/****************************************************************************
  Name          : avsv_amf_process
 
  Description   : This routine is an entry point for the AMF interface task.
                  It demonstrates the use of following AMF APIs
                  a) saAmfInitialize
                  b) saAmfSelectionObjectGet
                  c) saAmfCompNameGet
                  d) saAmfComponentRegister
                  e) saAmfDispatch
 
  Arguments     : None.
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void avsv_amf_process (void)
{
   SaAmfCallbacksT    reg_callback_set;
   SaVersionT         ver;
   SaAisErrorT        rc;
   NCS_SEL_OBJ_SET    wait_sel_objs;
   SaSelectionObjectT amf_sel_obj;
   NCS_SEL_OBJ        amf_ncs_sel_obj;
   NCS_SEL_OBJ        highest_sel_obj;

   /* this is to allow to establish MDS session with AvSv */
   m_NCS_TASK_SLEEP(3000);

   /*#########################################################################
                     Demonstrating the use of saAmfInitialize()
   #########################################################################*/

   /* Fill the callbacks that are to be registered with AMF */
   m_NCS_OS_MEMSET(&reg_callback_set, 0, sizeof(SaAmfCallbacksT));
   reg_callback_set.saAmfCSISetCallback = avsv_amf_csi_set_callback;
   reg_callback_set.saAmfCSIRemoveCallback = avsv_amf_csi_remove_callback;
   reg_callback_set.saAmfHealthcheckCallback = avsv_amf_healthcheck_callback;
   reg_callback_set.saAmfComponentTerminateCallback = avsv_amf_comp_terminate_callback;
   reg_callback_set.saAmfProtectionGroupTrackCallback = avsv_amf_protection_group_callback;

   /* Fill the AMF version */
   m_AVSV_AMF_VER_GET(ver);

   /* Initialize AMF */
   rc = saAmfInitialize(&gl_amf_hdl, &reg_callback_set, &ver);
   if (SA_AIS_OK != rc)
      return;

   m_NCS_CONS_PRINTF("\n AMF Initialization Done !!! \n AmfHandle: %d \n", (uns32)gl_amf_hdl);

   /*#########################################################################
                  Demonstrating the use of saAmfSelectionObjectGet()
   #########################################################################*/

   m_NCS_TASK_SLEEP(2000);

   /* Get the selection object corresponding to this AMF handle */
   rc = saAmfSelectionObjectGet(gl_amf_hdl, &amf_sel_obj);
   if (SA_AIS_OK != rc)
   {
      saAmfFinalize(gl_amf_hdl);
      return;
   }

   m_NCS_CONS_PRINTF("\n AMF Selection Object Get Successful !!! \n");

   /*#########################################################################
                   Demonstrating the use of saAmfCompNameGet()
   #########################################################################*/

   m_NCS_TASK_SLEEP(2000);

   rc = saAmfComponentNameGet(gl_amf_hdl, &gl_comp_name);
   if (SA_AIS_OK != rc)
   {
      saAmfFinalize(gl_amf_hdl);
      return;
   }

   m_NCS_CONS_PRINTF("\n Component Name Get Successful !!! \n CompName: %s \n",
                      gl_comp_name.value);

   /*#########################################################################
                 Demonstrating the use of saAmfComponentRegister()
   #########################################################################*/

   m_NCS_TASK_SLEEP(2000);

   rc = saAmfComponentRegister(gl_amf_hdl, &gl_comp_name, 0);
   if (SA_AIS_OK != rc)
   {
      saAmfFinalize(gl_amf_hdl);
      return;
   }

   m_NCS_CONS_PRINTF("\n Component Registered !!! \n");

   /***** Now wait (select) on AMF selection object *****/

   /* Reset the wait select objects */
   m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);

   /* derive the fd for AMF selection object */
   m_SET_FD_IN_SEL_OBJ((uns32)amf_sel_obj, amf_ncs_sel_obj);

   /* Set the AMF select object on which AvSv toolkit application waits */
   m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &wait_sel_objs);

   /* Get the highest select object in the set */
   highest_sel_obj = amf_ncs_sel_obj;

   /* Now wait forever */
   while (m_NCS_SEL_OBJ_SELECT(highest_sel_obj, &wait_sel_objs, NULL, NULL, NULL) != -1)
   {
      /*######################################################################
                      Demonstrating the use of saAmfDispatch()
      ######################################################################*/

      /* There is an AMF callback waiting to be be processed. Process it */
      rc = saAmfDispatch(gl_amf_hdl, SA_DISPATCH_ONE);
      if (SA_AIS_OK != rc)
      {
         saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
         saAmfFinalize(gl_amf_hdl);
         break;
      }

      /* Again set AMF select object to wait for another callback */
      m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &wait_sel_objs);
   }

   m_NCS_CONS_PRINTF("\n\n DEMO OVER !!! \n\n");

   return;
}


/****************************************************************************
  Name          : avsv_amf_csi_set_callback
 
  Description   : This routine is a callback to set (add/modify) the HA state
                  of a CSI (or all the CSIs) that is newly/already assigned 
                  to the component. It is specified as a part of AMF 
                  initialization. It demonstrates the use of following 
                  AMF APIs:
                  a) saAmfHAStateGet()
                  b) saAmfHealthcheckStart()
                  c) saAmfProtectionGroupTrack()
                  d) saAmfComponentUnregister()
                  e) saAmfFinalize()

 
  Arguments     : inv       - particular invocation of this callback function
                  comp_name - ptr to the component name
                  ha_state  - ha state to be assumed by the CSI (or all the 
                              CSIs)
                  csi_desc  - CSI descriptor

  Return Values : None.
 
  Notes         : This routine starts health check on the active entity. If
                  the CSI transitions from standby to active, the demo is 
                  considered over & then the usage of 'Component Unregister' &
                  'AMF Finalize' is illustrated.
******************************************************************************/
void avsv_amf_csi_set_callback(SaInvocationT       inv, 
                               const SaNameT       *comp_name,
                               SaAmfHAStateT       ha_state,
                               SaAmfCSIDescriptorT csi_desc)
{
   SaAmfHAStateT ha_st;
   SaAisErrorT      rc;
   SaAmfHealthcheckInvocationT    hc_inv;
   SaAmfRecommendedRecoveryT      rec_rcvr;
   SaNameT *csi_name;
   uns8    trk_flags;
   SaAmfProtectionGroupNotificationBufferT *not_buf;

   m_NCS_CONS_PRINTF("\n Dispatched 'CSI Set' Callback \n Component: %s \n CSIName: %s \n HAState: %s \n CSIFlags: %s \n", 
                      comp_name->value, csi_desc.csiName.value, 
                      ha_state_str[ha_state], csi_flag_str[csi_desc.csiFlags]);

   /* Store the ha state */
   gl_ha_state = ha_state;

   /* Respond immediately */
   rc = saAmfResponse(gl_amf_hdl, inv, SA_AIS_OK);
   if ( SA_AIS_OK != rc )
   {
      saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
      saAmfFinalize(gl_amf_hdl);
      return;
   }

   /* 
    * If ha-state is active & it's a new assignment, demonstrate the usage 
    * of ha-state-get & start healthcheck 
    */
   if ( (SA_AMF_HA_ACTIVE == ha_state) && 
        (SA_AMF_CSI_NEW_ASSIGN == 
           csi_desc.csiStateDescriptor.activeDescriptor.transitionDescriptor) )
   {
      /*######################################################################
                 Demonstrating the use of saAmfHAStateGet()
      ######################################################################*/

      m_NCS_CONS_PRINTF("\n\n INVOKING saAmfHAStateGet() API !!! \n");
      m_NCS_TASK_SLEEP(2000);

      rc = saAmfHAStateGet(gl_amf_hdl, &gl_comp_name, &csi_desc.csiName, &ha_st);
      if ( SA_AIS_OK != rc )
      {
         saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
         saAmfFinalize(gl_amf_hdl);
         return;
      }

      m_NCS_CONS_PRINTF(" CompName: %s \n CSIName: %s \n HAState: %s \n", 
                         gl_comp_name.value, csi_desc.csiName.value, 
                         ha_state_str[ha_st]);

      /*######################################################################
                  Demonstrating the use of saAmfHealthcheckStart()
      ######################################################################*/

      m_NCS_CONS_PRINTF("\n\n DEMONSTRATING AMF-INITIATED HEALTHCHECK !!! \n\n");
      m_NCS_TASK_SLEEP(2000);

      /* Fill the healthcheck parameters */
      hc_inv = SA_AMF_HEALTHCHECK_AMF_INVOKED;
      rec_rcvr = SA_AMF_COMPONENT_FAILOVER;

      /* Start the Healthcheck */
      rc = saAmfHealthcheckStart(gl_amf_hdl, &gl_comp_name, &gl_healthcheck_key,
                                 hc_inv, rec_rcvr);
      if ( SA_AIS_OK != rc )
      {
         saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
         saAmfFinalize(gl_amf_hdl);
         return;
      }

      m_NCS_CONS_PRINTF(" Started AMF-Initiated HealthCheck (with Component Failover Recommended Recovery) \n Comp: %s \n HealthCheckKey: %s \n", 
                         gl_comp_name.value, gl_healthcheck_key.key);
   }

   /* 
    * If ha-state is standby, start tracking the protection group 
    * associated with this CSI & initiate counter reading from the active
    */
   if ( SA_AMF_HA_STANDBY == ha_state )
   {
      /*######################################################################
                  Demonstrating the use of saAmfProtectionGroupTrack()
      ######################################################################*/

      /* Fill the protection group track parameters */
      csi_name = &csi_desc.csiName;
      trk_flags = SA_TRACK_CHANGES_ONLY;
      not_buf = NULL;

      /* Start Tracking the Protection Group */
      rc = saAmfProtectionGroupTrack(gl_amf_hdl, csi_name, trk_flags, not_buf);
      if ( SA_AIS_OK != rc )
      {
         saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
         saAmfFinalize(gl_amf_hdl);
         return;
      }

      m_NCS_CONS_PRINTF("\n Started Protection Group Tracking \n CSI: %s \n Track Flags: Changes Only \n",
                         csi_name->value);

      /* Initiate initial data read */
      avsv_ckpt_data_read();
   }

   /* If ha-state is active & it is a result of failover, stop the demo */
   if ( (SA_AMF_HA_ACTIVE == ha_state) && 
        (SA_AMF_CSI_NEW_ASSIGN != 
           csi_desc.csiStateDescriptor.activeDescriptor.transitionDescriptor) )
   {
      m_NCS_CONS_PRINTF("\n\n DEMO OVER (UNREGISTER & FINALIZE THE COMPONENT) !!! \n\n");

      /*######################################################################
                 Demonstrating the use of saAmfComponentUnregister()
      ######################################################################*/

      m_NCS_TASK_SLEEP(15000);

      rc = saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
      if ( SA_AIS_OK != rc ) 
         return;

      m_NCS_CONS_PRINTF(" Component UnRegistered !!! \n");

      /*######################################################################
                     Demonstrating the use of saAmfFinalize()
      ######################################################################*/

      m_NCS_TASK_SLEEP(2000);

      rc = saAmfFinalize(gl_amf_hdl);
      if ( SA_AIS_OK != rc ) 
         return;

      m_NCS_CONS_PRINTF(" AMF Finalize Done !!! \n");

      /* Reset the ha state */
      gl_ha_state = 0;
   }

   return;
}

/****************************************************************************
  Name          : avsv_amf_csi_remove_callback
 
  Description   : This routine is a callback to remove the CSI (or all the 
                  CSIs) that is/are assigned to the component. It is specified
                  as a part of AMF initialization.
 
  Arguments     : inv       - particular invocation of this callback function
                  comp_name - ptr to the component name
                  csi_name  - ptr to the CSI name that is being removed
                  csi_flags - specifies if one or more CSIs are affected

  Return Values : None.
 
  Notes         : None
******************************************************************************/
void avsv_amf_csi_remove_callback(SaInvocationT  inv, 
                                  const SaNameT  *comp_name,
                                  const SaNameT  *csi_name,
                                  SaAmfCSIFlagsT csi_flags)
{
   SaAisErrorT rc;

   m_NCS_CONS_PRINTF("\n Dispatched 'CSI Remove' Callback \n Component: %s \n CSI: %s \n CSIFlags: %s \n", 
                      comp_name->value, csi_name->value, csi_flag_str[csi_flags]);

   /* Reset the ha state */
   gl_ha_state = 0;

   /* Respond immediately */
   rc = saAmfResponse(gl_amf_hdl, inv, SA_AIS_OK);
   if ( SA_AIS_OK != rc )
   {
      saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
      saAmfFinalize(gl_amf_hdl);
   }

   return;
}


/****************************************************************************
  Name          : avsv_amf_healthcheck_callback
 
  Description   : This routine is a callback to perform the healthcheck and 
                  report any healthcheck failure to AMF. It is specified as 
                  a part of AMF initialization. It demonstrates the use of 
                  following AMF APIs:
                  a) saAmfHealthcheckStop()
                  b) saAmfComponentErrorReport()
 
  Arguments     : inv              - particular invocation of this callback 
                                     function
                  comp_name        - ptr to the component name
                  health_check_key - ptr to the healthcheck key for which the
                                     healthcheck is to be performed.
 
  Return Values : None.
 
  Notes         : This routine responds to the healhcheck callbacks for 
                  AVSV_HEALTHCHECK_CALLBACK_MAX_COUNT times after which it sends 
                  an error report.
******************************************************************************/
void avsv_amf_healthcheck_callback(SaInvocationT        inv, 
                                   const SaNameT        *comp_name,
                                   SaAmfHealthcheckKeyT *health_check_key)
{
   SaAisErrorT rc;
   static int healthcheck_count = 0;

   m_NCS_CONS_PRINTF("\n Dispatched 'HealthCheck' Callback \n Component: %s \n HealthCheckKey: %s \n", 
                      comp_name->value, health_check_key->key);

   /* Respond immediately */
   rc = saAmfResponse(gl_amf_hdl, inv, SA_AIS_OK);
   if ( SA_AIS_OK != rc )
   {
      saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
      saAmfFinalize(gl_amf_hdl);
      return;
   }

   /* Increment healthcheck count */
   healthcheck_count++;

   /* Send the Error Report */
   if (AVSV_HEALTHCHECK_CALLBACK_MAX_COUNT == healthcheck_count)
   {
      /*######################################################################
                  Demonstrating the use of saAmfHealthcheckStop()
      ######################################################################*/

      rc = saAmfHealthcheckStop(gl_amf_hdl, &gl_comp_name, &gl_healthcheck_key);
      if ( SA_AIS_OK != rc )
      {
         saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
         saAmfFinalize(gl_amf_hdl);
         return;
      }

      m_NCS_CONS_PRINTF("\n Stopped HealthCheck for Comp: %s with HealthCheckKey: %s \n", 
                         gl_comp_name.value, gl_healthcheck_key.key);

      /*######################################################################
                Demonstrating the use of saAmfComponentErrorReport()
      ######################################################################*/

      m_NCS_CONS_PRINTF("\n\n DEMONSTRATING COMPONENT FAILOVER THROUGH ERROR REPORT !!! \n\n");
      m_NCS_TASK_SLEEP(2000);

      rc = saAmfComponentErrorReport(gl_amf_hdl, &gl_comp_name, 0, SA_AMF_COMPONENT_FAILOVER, 0);
      if ( SA_AIS_OK != rc )
      {
         saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
         saAmfFinalize(gl_amf_hdl);
         return;
      }

      m_NCS_CONS_PRINTF("\n Sent Error Report for Comp: %s with CompFailover as the recommended recovery \n", 
                         gl_comp_name.value);

      /* Reset the ha state */
      gl_ha_state = 0;
   }

   return;
}


/****************************************************************************
  Name          : avsv_amf_comp_terminate_callback
 
  Description   : This routine is a callback to terminate the component. It 
                  is specified as a part of AMF initialization.
 
  Arguments     : inv             - particular invocation of this callback 
                                    function
                  comp_name       - ptr to the component name
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void avsv_amf_comp_terminate_callback(SaInvocationT inv, 
                                      const SaNameT *comp_name)
{
   SaAisErrorT rc;

   m_NCS_CONS_PRINTF("\n Dispatched 'Component Terminate' Callback \n Component: %s \n", 
                      comp_name->value);

   /* Respond immediately */
   rc = saAmfResponse(gl_amf_hdl, inv, SA_AIS_OK);
   if ( SA_AIS_OK != rc )
   {
      saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
      saAmfFinalize(gl_amf_hdl);
   }

   return;
}

/****************************************************************************
  Name          : avsv_amf_protection_group_callback
 
  Description   : This routine is a callback to notify the application of any 
                  changes in the protection group. It demonstrates the use of 
                  following AMF APIs:
                  a) saAmfProtectionGroupTrackStop()
                  b) saAmfPmStart()
                  c) saAmfPmStop()
 
  Arguments     : csi_name - ptr to the csi-name
                  not_buf  - ptr to the notification buffer
                  mem_num  - number of components that belong to this 
                             protection group
                  err      - error code

  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avsv_amf_protection_group_callback(const SaNameT *csi_name,
                                        SaAmfProtectionGroupNotificationBufferT *not_buf,
                                        SaUint32T mem_num,
                                        SaAisErrorT err)
{
   uns32  item_count;
   uns32  pid;
   uns8   pm_err;
   SaAmfRecommendedRecoveryT rec_rcvr;
   SaAmfPmStopQualifierT     stop_qual;
   SaAisErrorT  rc;

   m_NCS_CONS_PRINTF("\n Dispatched 'Protection Group' Callback \n CSI: %s \n No. of Members: %d \n", 
                      csi_name->value, (uns32)mem_num);

   if ( SA_AIS_OK != err )
      return;

   /* Print the Protection Group members */
   for (item_count= 0; item_count < not_buf->numberOfItems; item_count++)
   {
      m_NCS_CONS_PRINTF(" CompName[%d]: %s \n", 
                        item_count, not_buf->notification[item_count].member.compName.value);
      m_NCS_CONS_PRINTF(" Rank[%d]    : %d \n", 
                        item_count, (uns32)not_buf->notification[item_count].member.rank);
      m_NCS_CONS_PRINTF(" HAState[%d] : %s \n", 
                        item_count, ha_state_str[not_buf->notification[item_count].member.haState]);
      m_NCS_CONS_PRINTF(" Change[%d]  : %s \n", 
                        item_count, pg_change_str[not_buf->notification[item_count].change]);

      m_NCS_CONS_PRINTF("\n"); 
   }


   /*######################################################################
           Demonstrating the use of saAmfProtectionGroupTrackStop()
   ######################################################################*/

   rc = saAmfProtectionGroupTrackStop(gl_amf_hdl, csi_name);
   if ( SA_AIS_OK != rc )
   {
      saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
      saAmfFinalize(gl_amf_hdl);
      return;
   }

   m_NCS_CONS_PRINTF("\n Stopped Protection Group Tracking for CSI: %s \n", 
                      csi_name->value);


   /*######################################################################
                  Demonstrating the use of saAmfPmStart()
   ######################################################################*/

   /* Fill the `Passive Monitoring Start' parameters */
   pid = getpid();
   pm_err = SA_AMF_PM_NON_ZERO_EXIT;
   rec_rcvr = SA_AMF_COMPONENT_RESTART;

   /* Start Passive Monitoring */
   rc = saAmfPmStart(gl_amf_hdl, &gl_comp_name, pid, 0, pm_err, rec_rcvr);
   if ( SA_AIS_OK != rc )
   {
      saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
      saAmfFinalize(gl_amf_hdl);
      return;
   }

   m_NCS_CONS_PRINTF("\n Started Passive Monitoring for Comp: %s \n", 
                      gl_comp_name.value);


   /*######################################################################
                  Demonstrating the use of saAmfPmStop()
   ######################################################################*/

   /* Fill the `Passive Monitoring Stop' parameters */
   pid = getpid();
   pm_err = SA_AMF_PM_NON_ZERO_EXIT;
   stop_qual = SA_AMF_PM_PROC;

   /* Start Passive Monitoring */
   rc = saAmfPmStop(gl_amf_hdl, &gl_comp_name, stop_qual, pid, pm_err);
   if ( SA_AIS_OK != rc )
   {
      saAmfComponentUnregister(gl_amf_hdl, &gl_comp_name, 0);
      saAmfFinalize(gl_amf_hdl);
      return;
   }

   m_NCS_CONS_PRINTF("\n Stopped Passive Monitoring for Comp: %s \n", 
                      gl_comp_name.value);


   return;
}

