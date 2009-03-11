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

  pxy_pxd_amf_init........................Creates & starts AMF interface task.
  pxy_pxd_amf_process.....................Entry point for the AMF interface task.
  proxy_csi_set_callback............ Proxy CSI-Set callback.
  proxy_csi_remove_callback......... Proxy CSI-Remove callback.
  proxy_healthcheck_callback........ Proxy Component-Healthcheck callback.
  proxy_comp_terminate_callback..... Proxy Component-Terminate callback.
  proxied_csi_set_callback............ Proxied CSI-Set callback.
  proxied_csi_remove_callback......... Proxied CSI-Remove callback.
  proxied_healthcheck_callback........ Proxied Component-Healthcheck callback.
  proxied_comp_terminate_callback..... Proxied Component-Terminate callback.


******************************************************************************
*/


/* Common header files */
#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncssysf_tsk.h>

/* SAF header files */
#include "saAis.h"
#include "saAmf.h"
#include "pxy_pxd.h"

/*############################################################################
                       Static Function Decalarations
############################################################################*/

/* Entry level routine AMF Interface task */
static void pxy_pxd_amf_process(void);
static void pxy_pxd_proxy_initialize(void);
static uns32 pxy_pxd_proxy_amf_init(void);
static uns32 pxy_pxd_proxied_amf_init(uns32 index);

/* 'CSI Set' callback that is registered with AMF */
static void proxy_csi_set_callback(SaInvocationT, 
                                      const SaNameT *,
                                      SaAmfHAStateT,
                                      SaAmfCSIDescriptorT);

/* 'CSI Remove' callback that is registered with AMF */
static void proxy_csi_remove_callback(SaInvocationT, 
                                         const SaNameT *,
                                         const SaNameT *,
                                         SaAmfCSIFlagsT);

/* 'HealthCheck' callback that is registered with AMF */
static void proxy_healthcheck_callback(SaInvocationT, 
                                          const SaNameT *,
                                          SaAmfHealthcheckKeyT *);

/* 'Component Terminate' callback that is registered with AMF */
static void proxy_comp_terminate_callback(SaInvocationT, const SaNameT *);


/* 'CSI Set' callback that is registered with AMF */
static void proxied_csi_set_callback(SaInvocationT, 
                                      const SaNameT *,
                                      SaAmfHAStateT,
                                      SaAmfCSIDescriptorT);

/* 'CSI Remove' callback that is registered with AMF */
static void proxied_csi_remove_callback(SaInvocationT, 
                                         const SaNameT *,
                                         const SaNameT *,
                                         SaAmfCSIFlagsT);

/* 'HealthCheck' callback that is registered with AMF */
static void proxied_healthcheck_callback(SaInvocationT, 
                                          const SaNameT *,
                                          SaAmfHealthcheckKeyT *);

/* 'Component Terminate' callback that is registered with AMF */
static void proxied_comp_terminate_callback(SaInvocationT, const SaNameT *);

/* 'Protection Group' callback that is registered with AMF */
static void proxied_protection_group_callback(const SaNameT *,
                                               SaAmfProtectionGroupNotificationBufferT *,
                                               SaUint32T,
                                               SaAisErrorT);
static void proxied_comp_instantiate_callback(SaInvocationT inv, 
                                        const SaNameT *comp_name);
static void proxied_comp_cleanup_callback(SaInvocationT inv,
                                        const SaNameT *comp_name);

/****************************************************************************
  Name          : pxy_pxd_amf_init
 
  Description   : This routine creates & starts the AMF interface task.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 pxy_pxd_amf_init(void)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Create AMF interface task */
   rc = m_NCS_TASK_CREATE ( (NCS_OS_CB)pxy_pxd_amf_process, (void *)0, "PXY-PXD",
                            PXY_PXD_TASK_PRIORITY, PXY_PXD_STACKSIZE, 
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
  Name          : pxy_pxd_amf_process
 
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
void pxy_pxd_amf_process (void)
{
   SaAisErrorT        rc;
   NCS_SEL_OBJ_SET    wait_sel_objs, all_sel_objs;
   SaSelectionObjectT pxy_amf_sel_obj,pxd_amf_sel_obj[PXY_PXD_NUM_OF_PROXIED_COMP];
   NCS_SEL_OBJ        pxy_amf_ncs_sel_obj,pxd_amf_ncs_sel_obj[PXY_PXD_NUM_OF_PROXIED_COMP];
   NCS_SEL_OBJ        highest_sel_obj,wait_sel_fd;
   int ret = 0;
   uns32 comp_inv_hc_time_out = 500;
   uns32 index = 0;

   /* this is to allow to establish MDS session with AvSv */
   m_NCS_TASK_SLEEP(3000);

   pxy_pxd_proxy_initialize();   

   rc = pxy_pxd_proxy_amf_init();

   if (SA_AIS_OK != rc)
      return;
   for(index = 0; index < PXY_PXD_NUM_OF_PROXIED_COMP; index++)
     rc = pxy_pxd_proxied_amf_init(index);

   if (SA_AIS_OK != rc)
      return;

   /* Get the selection object corresponding to this AMF handle */
   rc = saAmfSelectionObjectGet(pxy_pxd_cb.pxy_info.amfHandle, &pxy_amf_sel_obj);
   if (SA_AIS_OK != rc)
   {
      saAmfFinalize(pxy_pxd_cb.pxy_info.amfHandle);
      return;
   }
   
   for(index = 0; index < PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
     /* Get the selection object corresponding to this AMF handle */
     rc = saAmfSelectionObjectGet(pxy_pxd_cb.pxd_info[index].amfHandle, 
                                  &pxd_amf_sel_obj[index]);
     if (SA_AIS_OK != rc)
     {
       saAmfFinalize(pxy_pxd_cb.pxy_info.amfHandle);
       m_NCS_CONS_PRINTF("\n AMF Selection Object Get Failed for index %d !!!\n",index);
       return;
     }
   }

   m_NCS_CONS_PRINTF("\n AMF Selection Object Get Successful !!! \n");

   rc = saAmfComponentNameGet(pxy_pxd_cb.pxy_info.amfHandle, 
                                 &pxy_pxd_cb.pxy_info.compName);
   if (SA_AIS_OK != rc)
   {
      saAmfFinalize(pxy_pxd_cb.pxy_info.amfHandle);
      return;
   }

   m_NCS_CONS_PRINTF("\n Component Name Get Successful !!! \n CompName: %s \n",
                      pxy_pxd_cb.pxy_info.compName.value);


   rc = saAmfComponentRegister(pxy_pxd_cb.pxy_info.amfHandle, 
                               &pxy_pxd_cb.pxy_info.compName, 0);
   if (SA_AIS_OK != rc)
   {
      saAmfFinalize(pxy_pxd_cb.pxy_info.amfHandle);
      return;
   }

   m_NCS_CONS_PRINTF("\n Proxy Component Registered !!! \n");

   /* Reset the wait select objects */
   m_NCS_SEL_OBJ_ZERO(&all_sel_objs);

   m_SET_FD_IN_SEL_OBJ((uns32)pxy_amf_sel_obj, pxy_amf_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(pxy_amf_ncs_sel_obj, &all_sel_objs);
   highest_sel_obj = pxy_amf_ncs_sel_obj;

   for(index = 0; index < PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
    m_SET_FD_IN_SEL_OBJ((uns32)pxd_amf_sel_obj[index], pxd_amf_ncs_sel_obj[index]);
    m_NCS_SEL_OBJ_SET(pxd_amf_ncs_sel_obj[index], &all_sel_objs);
    highest_sel_obj = m_GET_HIGHER_SEL_OBJ(highest_sel_obj, pxd_amf_ncs_sel_obj[index]);
   }

   wait_sel_objs = all_sel_objs;
   wait_sel_fd  = highest_sel_obj;

   /* Now wait forever */
   while ((ret = m_NCS_SEL_OBJ_SELECT(wait_sel_fd, &wait_sel_objs, NULL, NULL,
           &comp_inv_hc_time_out)) != -1)
   {
    comp_inv_hc_time_out = 500;
    /* Check if it's an event from AMF */
    if(m_NCS_SEL_OBJ_ISSET(pxy_amf_ncs_sel_obj, &wait_sel_objs))
    {
      m_NCS_CONS_PRINTF("\n Got Proxy AMF event!!! \n");
      /* There is an AMF callback waiting to be be processed. Process it */
      rc = saAmfDispatch(pxy_pxd_cb.pxy_info.amfHandle, SA_DISPATCH_ALL);
      if (SA_AIS_OK != rc)
      {
         m_NCS_CONS_PRINTF("\n Proxy saAmfDispatch failed !!! \n");
         break;
      }
    }
   for(index = 0; index < PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
    if(TRUE == pxy_pxd_cb.pxd_info[index].reg_pxied_comp)
    {
      m_NCS_CONS_PRINTF("\n Before registering pxd[%d]!!!\n",index);
      /* Need to register proxied component */
      rc = saAmfComponentRegister(pxy_pxd_cb.pxd_info[index].amfHandle,
                                   &pxy_pxd_cb.pxd_info[index].compName,
                                   &pxy_pxd_cb.pxy_info.compName);
      if (SA_AIS_OK == rc)
      {
        m_NCS_CONS_PRINTF("\n Proxied[%d] Component Registeration Successful. "
                              "Comp %s. rc is %d\n",index,
                              pxy_pxd_cb.pxd_info[index].compName.value,rc);
        pxy_pxd_cb.pxd_info[index].health_start = TRUE;
        pxy_pxd_cb.pxd_info[index].reg_pxied_comp = FALSE;
      }
      else if (SA_AIS_ERR_TRY_AGAIN == rc)
      {
        m_NCS_CONS_PRINTF("\n Proxied[%d] Component Registeration failed. TRY_AGAIN"
                              " Comp %s. rc is %d\n",index,
                              pxy_pxd_cb.pxd_info[index].compName.value,rc);
      }
      else
      {
        m_NCS_CONS_PRINTF("\n Proxied[%d] Component Registeration failed."
                              " Comp %s. rc is %d\n",index,
                              pxy_pxd_cb.pxd_info[index].compName.value,rc);
        pxy_pxd_cb.pxd_info[index].reg_pxied_comp = FALSE;
      }
     }
    } /* if(TRUE == pxy_pxd_cb.reg_pxied_comp) */

   /* Section 1 Starts : Health Check : SA_AMF_HEALTHCHECK_AMF_INVOKED */
   {
    SaAmfHealthcheckInvocationT    hc_inv;
    SaAmfRecommendedRecoveryT      rec_rcvr;

     for(index = 0; index < PXY_PXD_NUM_OF_PROXIED_COMP; index++)
     {
       if(TRUE == pxy_pxd_cb.pxd_info[index].health_start)
       { 
        m_NCS_CONS_PRINTF("\n Before registering hc [%d]!!!\n",index);
        hc_inv = SA_AMF_HEALTHCHECK_AMF_INVOKED;
        rec_rcvr = SA_AMF_COMPONENT_FAILOVER;

        rc = saAmfHealthcheckStart(pxy_pxd_cb.pxd_info[index].amfHandle, 
                                   &pxy_pxd_cb.pxd_info[index].compName, 
                                   &pxy_pxd_cb.pxd_info[index].healthcheck_key,
                                   hc_inv, rec_rcvr);
        if (SA_AIS_OK == rc )
        {
          m_NCS_CONS_PRINTF("Started AMF-Initiated HealthCheck (with Component "
                          "Failover Recommended Recovery) \n Comp: %s \n" 
                          "HealthCheckKey: %s \n",
                          pxy_pxd_cb.pxd_info[index].compName.value, 
                          pxy_pxd_cb.pxd_info[index].healthcheck_key.key);
          pxy_pxd_cb.pxd_info[index].health_start = FALSE;   
          pxy_pxd_cb.pxd_info[index].health_start_comp_inv = TRUE;   
        }
        else if(SA_AIS_ERR_TRY_AGAIN == rc)
        {
          m_NCS_CONS_PRINTF(" AMF-Initiated HealthCheck TRY_AGAIN (with Component"
                            " Failover Recommended Recovery) \n Comp: %s \n" 
                            "HealthCheckKey: %s. Res %d \n",
                            pxy_pxd_cb.pxd_info[index].compName.value, 
                            pxy_pxd_cb.pxd_info[index].healthcheck_key.key, rc);
        }
        else
        {
          m_NCS_CONS_PRINTF(" AMF-Initiated HealthCheck failed (with Component "
                            "Failover Recommended Recovery) \n Comp: %s \n" 
                            "HealthCheckKey: %s. Res %d \n",
                            pxy_pxd_cb.pxd_info[index].compName.value, 
                            pxy_pxd_cb.pxd_info[index].healthcheck_key.key, rc);
          pxy_pxd_cb.pxd_info[index].health_start = FALSE;   
        }
       } /* if(TRUE == pxy_pxd_cb.pxd_info[index].health_start)*/ 
     }
   }
   /* Section 1 Ends : Health Check : SA_AMF_HEALTHCHECK_AMF_INVOKED */

  if(TRUE == pxy_pxd_cb.unreg_pxied_comp)
  {
   for(index = 0; index < PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
      m_NCS_CONS_PRINTF("\n Before unregistering PXD[%d]!!! \n",index);
      /* Need to unregister proxied component */
       rc = saAmfComponentUnregister(pxy_pxd_cb.pxd_info[index].amfHandle,
                                     &pxy_pxd_cb.pxd_info[index].compName,
                                     &pxy_pxd_cb.pxy_info.compName);
       if (SA_AIS_OK != rc)
       {
          m_NCS_CONS_PRINTF("\n Proxied Component unreg failed. Comp %s, Pxy Comp %s, Res %d\n",
                                pxy_pxd_cb.pxd_info[index].compName.value,
                                pxy_pxd_cb.pxy_info.compName.value,rc);
       }
       else
          m_NCS_CONS_PRINTF("\n Proxied Component unreg Succ. Comp %s, Pxy Comp %s\n",
                                pxy_pxd_cb.pxd_info[index].compName.value,
                                pxy_pxd_cb.pxy_info.compName.value);
       pxy_pxd_cb.unreg_pxied_comp = FALSE;
       pxy_pxd_cb.pxd_info[index].health_start_comp_inv = FALSE;
   }
  }

   for(index = 0; index < PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
    /* Check if it's an event from AMF */
    if(m_NCS_SEL_OBJ_ISSET(pxd_amf_ncs_sel_obj[index], &wait_sel_objs))
    {
      m_NCS_CONS_PRINTF("\n Before Pxd dispatch PXD[%d] !!!\n",index);
      /* There is an AMF callback waiting to be be processed. Process it */
      rc = saAmfDispatch(pxy_pxd_cb.pxd_info[index].amfHandle, SA_DISPATCH_ONE);
      if (SA_AIS_OK != rc)
      {
         m_NCS_CONS_PRINTF("\n PXD[%d] saAmfDispatch failed !!! \n",index);
         break;
      }
    }
   }
      /* Again set AMF select object to wait for another callback */
      wait_sel_objs = all_sel_objs;
      wait_sel_fd  = highest_sel_obj;
   }/* While loop */

   m_NCS_CONS_PRINTF("\n\n Proxy Exited, ret %d!!!\n\n",ret);

   return;
}


/****************************************************************************
  Name          : proxy_csi_set_callback
 
  Description   : This routine is a callback to set (add/modify) the HA state
                  of a CSI (or all the CSIs) that is newly/already assigned 
                  to the component. It is specified as a part of AMF 
                  initialization.
 
  Arguments     : inv       - particular invocation of this callback function
                  comp_name - ptr to the component name
                  ha_state  - ha state to be assumed by the CSI (or all the 
                              CSIs)
                  csi_desc  - CSI descriptor

  Return Values : None.
 
  Notes         : None. 
******************************************************************************/
void proxy_csi_set_callback(SaInvocationT       inv, 
                               const SaNameT       *comp_name,
                               SaAmfHAStateT       ha_state,
                               SaAmfCSIDescriptorT csi_desc)
{
   SaAmfHAStateT ha_st;
   SaAisErrorT      rc = SA_AIS_OK;
   SaNameT *csi_name;
   uns8    trk_flags;
   uns32   index = 0;

   m_NCS_CONS_PRINTF("\n'Proxy CSI Set' Callback \n Component: %s \n CSIName: %s \n HAState: %s \n CSIFlags: %s \n", 
                      comp_name->value, csi_desc.csiName.value, 
                      ha_state_str[ha_state], csi_flag_str[csi_desc.csiFlags]);

   switch(ha_state)
   {
   case SA_AMF_HA_ACTIVE:
      /* We need to register all the proxied component when becoming ACT */
      pxy_pxd_cb.pxy_info.haState = ha_state;
      for(index=0; index<PXY_PXD_NUM_OF_PROXIED_COMP; index++)
        pxy_pxd_cb.pxd_info[index].reg_pxied_comp = TRUE;
      break;

   case SA_AMF_HA_QUIESCED: 
      /* We need to unregister to all proxied components, if going to STDBY 
         from ACT.*/
      if(SA_AMF_HA_ACTIVE == pxy_pxd_cb.pxy_info.haState)
      {
         /* If previous HA state is ACT, then unregister all proxied comp. */
         pxy_pxd_cb.unreg_pxied_comp = TRUE;
      }
      pxy_pxd_cb.pxy_info.haState = ha_state;
      break;
   case SA_AMF_HA_STANDBY:
      pxy_pxd_cb.pxy_info.haState = ha_state;
      break;
   default:
      rc=SA_AIS_ERR_FAILED_OPERATION;
      break;
   };

   rc = saAmfResponse(pxy_pxd_cb.pxy_info.amfHandle, inv, rc);
   if ( SA_AIS_OK != rc )
   {
      m_NCS_CONS_PRINTF("\nsaAmfResponse returned failure for CSI Set. Result %d\n",rc);
      saAmfComponentUnregister(pxy_pxd_cb.pxy_info.amfHandle, 
                               &pxy_pxd_cb.pxy_info.compName, 0);
      saAmfFinalize(pxy_pxd_cb.pxy_info.amfHandle);
      return;
   }

   return;
}

/****************************************************************************
  Name          : proxy_csi_remove_callback
 
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
void proxy_csi_remove_callback(SaInvocationT  inv, 
                                  const SaNameT  *comp_name,
                                  const SaNameT  *csi_name,
                                  SaAmfCSIFlagsT csi_flags)
{
   SaAisErrorT rc;

   m_NCS_CONS_PRINTF("\n Proxy 'CSI Remove' Callback \n Component: %s \n CSI: %s \n CSIFlags: %s \n", 
                      comp_name->value, csi_name->value, csi_flag_str[csi_flags]);

   /* Reset the ha state */
   pxy_pxd_cb.pxy_info.haState = 0;

   /* Respond immediately */
   rc = saAmfResponse(pxy_pxd_cb.pxy_info.amfHandle, inv, SA_AIS_OK);
   if ( SA_AIS_OK != rc )
   {
      saAmfComponentUnregister(pxy_pxd_cb.pxy_info.amfHandle, &pxy_pxd_cb.pxy_info.compName, 0);
      saAmfFinalize(pxy_pxd_cb.pxy_info.amfHandle);
      m_NCS_CONS_PRINTF("\nsaAmfResponse returned failure for CSI Remove. Result %d\n",rc);
   }

   return;
}


/****************************************************************************
  Name          : proxy_healthcheck_callback
 
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
void proxy_healthcheck_callback(SaInvocationT        inv, 
                                   const SaNameT        *comp_name,
                                   SaAmfHealthcheckKeyT *health_check_key)
{
   SaAisErrorT rc;
   static int healthcheck_count = 0;

   m_NCS_CONS_PRINTF("\n Dispatched 'HealthCheck' Callback \n Component: %s \n HealthCheckKey: %s \n", 
                      comp_name->value, health_check_key->key);

   /* Respond immediately */
   rc = saAmfResponse(pxy_pxd_cb.pxy_info.amfHandle, inv, SA_AIS_OK);
   if ( SA_AIS_OK != rc )
   {
      saAmfComponentUnregister(pxy_pxd_cb.pxy_info.amfHandle, &pxy_pxd_cb.pxy_info.compName, 0);
      saAmfFinalize(pxy_pxd_cb.pxy_info.amfHandle);
      m_NCS_CONS_PRINTF("\nsaAmfResponse returned failure for Health Check. Result %d\n",rc);
      return;
   }

   /* Increment healthcheck count */
   healthcheck_count++;

   return;
}


/****************************************************************************
  Name          : proxy_comp_terminate_callback
 
  Description   : This routine is a callback to terminate the component. It 
                  is specified as a part of AMF initialization.
 
  Arguments     : inv             - particular invocation of this callback 
                                    function
                  comp_name       - ptr to the component name
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void proxy_comp_terminate_callback(SaInvocationT inv, 
                                      const SaNameT *comp_name)
{
   SaAisErrorT rc;

   m_NCS_CONS_PRINTF("\n Proxy 'Component Terminate' Callback \n Component: %s \n", 
                      comp_name->value);

   rc = saAmfResponse(pxy_pxd_cb.pxy_info.amfHandle, inv, SA_AIS_OK);
   if ( SA_AIS_OK != rc )
   {
      saAmfComponentUnregister(pxy_pxd_cb.pxy_info.amfHandle, 
                               &pxy_pxd_cb.pxy_info.compName, 0);
      saAmfFinalize(pxy_pxd_cb.pxy_info.amfHandle);
      m_NCS_CONS_PRINTF("\nsaAmfResponse returned failure for Comp Terminate. Result %d\n",rc);
   }

   return;
}

/****************************************************************************
  Name          : proxy_comp_pg_callback

  Description   : This routine is a callback to notify the application of any
                  changes in the protection group.

  Arguments     : csi_name - ptr to the csi-name
                  not_buf  - ptr to the notification buffer
                  mem_num  - number of components that belong to this
                             protection group
                  err      - error code

  Return Values : None.

  Notes         : None.
******************************************************************************/
void proxy_comp_pg_callback(const SaNameT *csi_name,
                  SaAmfProtectionGroupNotificationBufferT *not_buf,
                  SaUint32T mem_num,
                  SaAisErrorT err)
{
   uns32  item_count;
   SaAisErrorT  rc;

   m_NCS_CONS_PRINTF("\n Dispatched 'Protection Group' Callback \n CSI: %s \n No. of Members: %d \n",
                      csi_name->value, (uns32)mem_num);

   if ( SA_AIS_OK != err )
   {
      m_NCS_CONS_PRINTF("\n Error Returned is %d\n",err);   
      return;
   }

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

return;

}

/****************************************************************************
  Name          : pxy_pxd_proxy_initialize

  Description   : This routine initializes CB data. 

  Arguments     : None. 

  Return Values : None.

  Notes         : None.
******************************************************************************/
void pxy_pxd_proxy_initialize(void)
{
  pxy_pxd_cb.pxy_info.healthcheck_key.keyLen = 10;
  strcpy((uns8 *)pxy_pxd_cb.pxy_info.healthcheck_key.key, "A9FD64E12D");
  /* External Component initialization. */
  strcpy((uns8 *)pxy_pxd_cb.pxd_info[0].compName.value, 
                      "safComp=CompT_EXT,safEsu=SuT_EXT1");
  pxy_pxd_cb.pxd_info[0].compName.length = 
     m_NCS_STRLEN("safComp=CompT_EXT,safEsu=SuT_EXT1");
  pxy_pxd_cb.pxd_info[0].healthcheck_key.keyLen = 6;
  strcpy((uns8 *)pxy_pxd_cb.pxd_info[0].healthcheck_key.key, "ABCD20");

  strcpy((uns8 *)pxy_pxd_cb.pxd_info[1].compName.value, 
                      "safComp=CompT_EXT,safEsu=SuT_EXT2");
  pxy_pxd_cb.pxd_info[1].compName.length = 
     m_NCS_STRLEN("safComp=CompT_EXT,safEsu=SuT_EXT2");
  pxy_pxd_cb.pxd_info[1].healthcheck_key.keyLen = 6;
  strcpy((uns8 *)pxy_pxd_cb.pxd_info[1].healthcheck_key.key, "ABCD20");

  /* Internode Component initialization. */
  strcpy((uns8 *)pxy_pxd_cb.pxd_info[2].compName.value,
                      "safComp=CompT_PXD,safSu=SuT_PL_PXD1,safNode=PL_2_3");
  pxy_pxd_cb.pxd_info[2].compName.length =
     m_NCS_STRLEN("safComp=CompT_PXD,safSu=SuT_PL_PXD1,safNode=PL_2_3");
  pxy_pxd_cb.pxd_info[2].healthcheck_key.keyLen = 10;
  strcpy((uns8 *)pxy_pxd_cb.pxd_info[2].healthcheck_key.key, "A9FD64E12E");

  strcpy((uns8 *)pxy_pxd_cb.pxd_info[3].compName.value,
                      "safComp=CompT_PXD,safSu=SuT_PL_PXD2,safNode=PL_2_3");
  pxy_pxd_cb.pxd_info[3].compName.length =
     m_NCS_STRLEN("safComp=CompT_PXD,safSu=SuT_PL_PXD2,safNode=PL_2_3");
  pxy_pxd_cb.pxd_info[3].healthcheck_key.keyLen = 10;
  strcpy((uns8 *)pxy_pxd_cb.pxd_info[3].healthcheck_key.key, "A9FD64E12E");

}

/****************************************************************************
  Name          : pxy_pxd_proxy_amf_init

  Description   : This routine initializes amf interface.

  Arguments     : None.

  Return Values : SUCC/FAILURE

  Notes         : None.
******************************************************************************/
uns32 pxy_pxd_proxy_amf_init(void)
{
   SaAisErrorT        rc;
   SaAmfCallbacksT    reg_callback_set;
   SaVersionT         ver;

   /* Fill the callbacks that are to be registered with AMF */
   memset(&reg_callback_set, 0, sizeof(SaAmfCallbacksT));
   reg_callback_set.saAmfCSISetCallback = proxy_csi_set_callback;
   reg_callback_set.saAmfCSIRemoveCallback = proxy_csi_remove_callback;
   reg_callback_set.saAmfHealthcheckCallback = proxy_healthcheck_callback;
   reg_callback_set.saAmfComponentTerminateCallback = proxy_comp_terminate_callback;
   reg_callback_set.saAmfProtectionGroupTrackCallback = proxy_comp_pg_callback;

   /* Fill the AMF version */
   m_PXY_PXD_VER_GET(ver);

   /* Initialize AMF */
   rc = saAmfInitialize(&pxy_pxd_cb.pxy_info.amfHandle, &reg_callback_set, &ver);
   if (SA_AIS_OK != rc)
   {
      m_NCS_CONS_PRINTF("\n Proxy AMF Initialization Failed !!! \n AmfHandle: %lld \n", 
                       pxy_pxd_cb.pxy_info.amfHandle);
      return rc;
   }

   m_NCS_CONS_PRINTF("\n Proxy AMF Initialization Done !!! \n AmfHandle: %lld \n", 
                       pxy_pxd_cb.pxy_info.amfHandle);
   return rc;

}

/****************************************************************************
  Name          : pxy_pxd_proxied_amf_init

  Description   : This routine initializes amf interface for proxied comp.

  Arguments     : None.

  Return Values : None.

  Notes         : None.
******************************************************************************/
uns32 pxy_pxd_proxied_amf_init(uns32 index)
{
   SaAisErrorT        rc;
   SaAmfCallbacksT    reg_callback_set;
   SaVersionT         ver;

   /* Fill the callbacks that are to be registered with AMF */
   memset(&reg_callback_set, 0, sizeof(SaAmfCallbacksT));


   reg_callback_set.saAmfCSISetCallback = proxied_csi_set_callback;
   reg_callback_set.saAmfCSIRemoveCallback = proxied_csi_remove_callback;
   reg_callback_set.saAmfHealthcheckCallback = proxied_healthcheck_callback;
   reg_callback_set.saAmfComponentTerminateCallback = proxied_comp_terminate_callback;
   reg_callback_set.saAmfProxiedComponentInstantiateCallback = 
                                           proxied_comp_instantiate_callback;
    reg_callback_set.saAmfProxiedComponentCleanupCallback = 
                                           proxied_comp_cleanup_callback;

   /* Fill the AMF version */
   m_PXY_PXD_VER_GET(ver);

   /* Initialize AMF */
   rc = saAmfInitialize(&pxy_pxd_cb.pxd_info[index].amfHandle, &reg_callback_set, &ver);
   if (SA_AIS_OK != rc)
   {
      m_NCS_CONS_PRINTF("\n Proxied[%d] AMF Initialization Failed !!! \n AmfHandle: %lld \n", 
                       index,pxy_pxd_cb.pxd_info[index].amfHandle);
      return rc;
   }
   m_NCS_CONS_PRINTF("\n Proxied[%d] AMF Initialization Done !!! \n AmfHandle: %lld \n", 
                     index,pxy_pxd_cb.pxd_info[index].amfHandle);

   return rc;
}

/****************************************************************************
  Name          : proxied_csi_set_callback
 
  Description   : This routine is a callback to set (add/modify) the HA state
                  of a CSI (or all the CSIs) that is newly/already assigned 
                  to the component. It is specified as a part of AMF 
                  initialization.
 
  Arguments     : inv       - particular invocation of this callback function
                  comp_name - ptr to the component name
                  ha_state  - ha state to be assumed by the CSI (or all the 
                              CSIs)
                  csi_desc  - CSI descriptor

  Return Values : None.
 
  Notes         : None. 
******************************************************************************/
void proxied_csi_set_callback(SaInvocationT       inv, 
                               const SaNameT       *comp_name,
                               SaAmfHAStateT       ha_state,
                               SaAmfCSIDescriptorT csi_desc)
{
   SaAmfHAStateT ha_st;
   SaAisErrorT      rc = SA_AIS_OK;
   SaAmfHealthcheckInvocationT    hc_inv;
   SaAmfRecommendedRecoveryT      rec_rcvr;
   SaNameT *csi_name;
   uns8    trk_flags;
   SaAisErrorT error=SA_AIS_OK;
   uns32 index = 0;

   m_NCS_CONS_PRINTF("\n'Proxied CSI Set' Callback \n Component: %s \n CSIName: %s \n HAState: %s \n CSIFlags: %s \n", 
                      comp_name->value, csi_desc.csiName.value, 
                      ha_state_str[ha_state], csi_flag_str[csi_desc.csiFlags]);

   for(index=0; index<PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
      if(0 == strcmp(comp_name->value, pxy_pxd_cb.pxd_info[index].compName.value))
         break;
   }
   
   if(PXY_PXD_NUM_OF_PROXIED_COMP == index)
       m_NCS_CONS_PRINTF("\n Component not found, index\n");
   else
       m_NCS_CONS_PRINTF("\n Component %s, index %d\n",
                            pxy_pxd_cb.pxd_info[index].compName.value, index);

   switch(ha_state)
   {
   case SA_AMF_HA_ACTIVE:
      /* We need to register all the proxied component when becoming ACT */
      pxy_pxd_cb.pxd_info[index].haState = ha_state;
      break;

   case SA_AMF_HA_STANDBY:
   case SA_AMF_HA_QUIESCED: 
      /* We need to unregister to all proxied components, if going to STDBY 
         from ACT.*/
      pxy_pxd_cb.pxd_info[index].haState = ha_state;
      break;
   default:
      rc=SA_AIS_ERR_FAILED_OPERATION;
      break;
   };

   rc = saAmfResponse(pxy_pxd_cb.pxd_info[index].amfHandle, inv, error);
   if ( SA_AIS_OK != rc )
   {
      m_NCS_CONS_PRINTF("\nsaAmfResponse failure for CSI Set. Comp %s,Result %d\n",
                           pxy_pxd_cb.pxd_info[index].compName.value, rc);
      return;
   }

   return;
}

/****************************************************************************
  Name          : proxied_csi_remove_callback
 
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
void proxied_csi_remove_callback(SaInvocationT  inv, 
                                  const SaNameT  *comp_name,
                                  const SaNameT  *csi_name,
                                  SaAmfCSIFlagsT csi_flags)
{
   SaAisErrorT rc;
   uns32 index = 0;

   m_NCS_CONS_PRINTF("\n Proxied 'CSI Remove' Callback \n Component: %s \n CSI: %s \n CSIFlags: %s \n", 
                      comp_name->value, csi_name->value, csi_flag_str[csi_flags]);

   for(index=0; index<PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
      if(0 == strcmp(comp_name->value, pxy_pxd_cb.pxd_info[index].compName.value))
         break;
   }
   
   if(PXY_PXD_NUM_OF_PROXIED_COMP == index)
       m_NCS_CONS_PRINTF("\n Component not found, index\n");
   else
       m_NCS_CONS_PRINTF("\n Component %s, index %d\n",
                            pxy_pxd_cb.pxd_info[index].compName.value, index);
   /* Reset the ha state */
   pxy_pxd_cb.pxd_info[index].haState = 0;

   /* Respond immediately */
   rc = saAmfResponse(pxy_pxd_cb.pxd_info[index].amfHandle, inv, SA_AIS_OK);
   if ( SA_AIS_OK != rc )
   {
      m_NCS_CONS_PRINTF("\nsaAmfResponse failure for CSI Rem. Comp %s,Result %d\n",
                           pxy_pxd_cb.pxd_info[index].compName.value, rc);
   }

   return;
}


/****************************************************************************
  Name          : proxied_healthcheck_callback
 
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
void proxied_healthcheck_callback(SaInvocationT        inv, 
                                   const SaNameT        *comp_name,
                                   SaAmfHealthcheckKeyT *health_check_key)
{
   SaAisErrorT rc;
   static int healthcheck_count = 0;
   uns32 index = 0;

   m_NCS_CONS_PRINTF("\n Dispatched 'HealthCheck' Callback \n Component: %s \n HealthCheckKey: %s \n", 
                      comp_name->value, health_check_key->key);

   for(index=0; index<PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
      if(0 == strcmp(comp_name->value, pxy_pxd_cb.pxd_info[index].compName.value))
         break;
   }
   
   if(PXY_PXD_NUM_OF_PROXIED_COMP == index)
       m_NCS_CONS_PRINTF("\n Component not found, index\n");
   else
       m_NCS_CONS_PRINTF("\n Component %s, index %d\n",
                            pxy_pxd_cb.pxd_info[index].compName.value, index);

   /* Respond immediately */
   rc = saAmfResponse(pxy_pxd_cb.pxd_info[index].amfHandle, inv, SA_AIS_OK);
   if ( SA_AIS_OK != rc )
   {
      m_NCS_CONS_PRINTF("\nsaAmfResponse failure for HC. Comp %s,Result %d\n",
                           pxy_pxd_cb.pxd_info[index].compName.value, rc);
      return;
   }

   return;
}


/****************************************************************************
  Name          : proxied_comp_terminate_callback
 
  Description   : This routine is a callback to terminate the component. It 
                  is specified as a part of AMF initialization.
 
  Arguments     : inv             - particular invocation of this callback 
                                    function
                  comp_name       - ptr to the component name
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void proxied_comp_terminate_callback(SaInvocationT inv, 
                                      const SaNameT *comp_name)
{
   SaAisErrorT rc;
   uns32 index = 0;

   m_NCS_CONS_PRINTF("\n Proxied 'Component Terminate' Callback \n Component: %s \n", 
                      comp_name->value);

   for(index=0; index<PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
      if(0 == strcmp(comp_name->value, pxy_pxd_cb.pxd_info[index].compName.value))
         break;
   }
   
   if(PXY_PXD_NUM_OF_PROXIED_COMP == index)
       m_NCS_CONS_PRINTF("\n Component not found, index\n");
   else
       m_NCS_CONS_PRINTF("\n Component %s, index %d\n",
                            pxy_pxd_cb.pxd_info[index].compName.value, index);

   rc = saAmfResponse(pxy_pxd_cb.pxd_info[index].amfHandle, inv, SA_AIS_OK);
   if ( SA_AIS_OK != rc )
   {
      m_NCS_CONS_PRINTF("\nsaAmfResponse failure for Comp Term. Comp %s,Result %d\n",
                           pxy_pxd_cb.pxd_info[index].compName.value, rc);
   }

   return;
}

/****************************************************************************
  Name          : proxied_comp_instantiate_callback
 
  Description   : This routine is a callback to instantiate proxied comp. It
                  is specified as a part of AMF initialization.
 
  Arguments     : inv             - particular invocation of this callback 
                                    function
                  comp_name       - ptr to the component name
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void proxied_comp_instantiate_callback(SaInvocationT inv, 
                                        const SaNameT *comp_name)
{
   SaAisErrorT error=SA_AIS_OK; 
   SaAisErrorT rc;
   uns32 index = 0;

   /*error=SA_AIS_ERR_FAILED_OPERATION;*/
   m_NCS_CONS_PRINTF("\n Proxied 'Pxied Comp Inst ' Callback \n Component: %s \n", 
                      comp_name->value);

   for(index=0; index<PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
      if(0 == strcmp(comp_name->value, pxy_pxd_cb.pxd_info[index].compName.value))
         break;
   }
   
   if(PXY_PXD_NUM_OF_PROXIED_COMP == index)
       m_NCS_CONS_PRINTF("\n Component not found, index\n");
   else
       m_NCS_CONS_PRINTF("\n Component %s, index %d\n",
                            pxy_pxd_cb.pxd_info[index].compName.value, index);

   rc = saAmfResponse(pxy_pxd_cb.pxd_info[index].amfHandle, inv, error);
   if ( SA_AIS_OK != rc )
   {
      m_NCS_CONS_PRINTF("\nsaAmfResponse failure for Pxd Comp Inst. Comp %s,Result %d\n",
                           pxy_pxd_cb.pxd_info[index].compName.value, rc);
   }
return;
}
/****************************************************************************
  Name          : proxied_comp_cleanup_callback

  Description   : This routine is a callback to clean proxied comp. It
                  is specified as a part of AMF initialization.

  Arguments     : inv             - particular invocation of this callback
                                    function
                  comp_name       - ptr to the component name

  Return Values : None.

  Notes         : None
******************************************************************************/
void proxied_comp_cleanup_callback(SaInvocationT inv,
                                        const SaNameT *comp_name)
{
   SaAisErrorT error=SA_AIS_OK;
   SaAisErrorT rc;
   uns32 index = 0;

   m_NCS_CONS_PRINTF("\n Proxied 'Pxied Comp Cleanup ' Callback \n Component: %s \n",
                      comp_name->value);

   for(index=0; index<PXY_PXD_NUM_OF_PROXIED_COMP; index++)
   {
      if(0 == strcmp(comp_name->value, pxy_pxd_cb.pxd_info[index].compName.value))
         break;
   }
   
   if(PXY_PXD_NUM_OF_PROXIED_COMP == index)
       m_NCS_CONS_PRINTF("\n Component not found, index\n");
   else
       m_NCS_CONS_PRINTF("\n Component %s, index %d\n",
                            pxy_pxd_cb.pxd_info[index].compName.value, index);

   rc = saAmfResponse(pxy_pxd_cb.pxd_info[index].amfHandle, inv, error);
   if ( SA_AIS_OK != rc )
   {
      m_NCS_CONS_PRINTF("\nsaAmfResponse failure for Pxd Comp Term. Comp %s,Result %d\n",
                           pxy_pxd_cb.pxd_info[index].compName.value, rc);
   }
   return;
}

