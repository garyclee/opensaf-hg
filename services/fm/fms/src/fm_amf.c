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

/*****************************************************************************
..............................................................................

  $$


  MODULE NAME: FM 

  AUTHOR: vishal soni (vishal.soni@emerson.com)

..............................................................................


  DESCRIPTION: fm SAF callback routines.

  FUNCTIONS INCLUDED in this module:
  fm_amf_take_hdl.............................. Get FM handle
  fm_amf_give_hdl.............................. Release FM handle
  fm_saf_CSI_set_callback.................... fm SAF HA state callback.
  fm_saf_health_chk_callback................. fm SAF Health Check callback.  
  fm_saf_CSI_rem_callback   ................. fm SAF CSI remove callback.  
  fm_saf_comp_terminate_callback............. fm SAF Terminate comp callback.  
  fm_amf_init................................ fm AMF initialization method.
  fm_amf_register............................ fm AMF registration method.
  fm_amf_unregister.......................... fm AMF unregistration method
  fm_amf_healthcheck_start................... fm AMF health check start method
  fm_amf_lib_init............................ fm NCS lib init method
  fm_amf_lib_destroy......................... fm NCS lib destroy method
  fm_pipe_open............................... fm named pipe open method
  fm_pipe_close.............................. fm named pipe close method
  fm_amf_open................................ fm named pipe open method
  fm_amf_close............................... fm named pipe close method
  fm_amf_pipe_process_msg.................... fm pipe process method 
  fm_amf_process_msg......................... fm amf process message method


******************************************************************************/

#include "fm.h"
EXTERN_C uns32 gl_fm_hdl;

static uns32 fm_amf_init       (FM_AMF_CB *fm_amf_cb);
static uns32 fm_amf_register   (FM_AMF_CB *fm_amf_cb);
static uns32 fm_amf_unregister (FM_AMF_CB *fm_amf_cb);
static uns32 fm_amf_lib_init   (FM_AMF_CB *fm_amf_cb);
static uns32 fm_amf_lib_destroy(FM_AMF_CB *fm_amf_cb);
static FM_AMF_CB* fm_amf_take_hdl (void);
static void fm_amf_give_hdl (void);
/**************************************************************************\

       L O C A L      D A T A    S T R U C T U R E S 

\**************************************************************************/

/****************************************************************************
 * Name          : fm_amf_take_hdl
 *
 * Description   : Get the poiter to FM AMF control block.
 *
 * Arguments     : None.
 *
 * Return Values : FM_AMF_CB control block pointer.
 *
 * Notes         : None.
 *****************************************************************************/
FM_AMF_CB* fm_amf_take_hdl (void)
{
   FM_CB *fm_cb = NULL;
   /* Take handle */
   fm_cb = ncshm_take_hdl(NCS_SERVICE_ID_GFM, gl_fm_hdl);

   return &fm_cb->fm_amf_cb;
}

/****************************************************************************
 * Name          : fm_amf_give_hdl
 *
 * Description   : Get the poiter to FM AMF control block.
 *
 * Arguments     : None.
 *
 * Return Values : FM_AMF_CB control block pointer.
 *
 * Notes         : None.
 *****************************************************************************/
void fm_amf_give_hdl (void)
{
   /* Give CB hdl */
   ncshm_give_hdl(gl_fm_hdl);

   return;
}

/****************************************************************************
 * Name          : fm_saf_CSI_set_callback
 *
 * Description   : This function SAF callback function which will be called 
 *                 when there is any change in the HA state.
 *
 * Arguments     : invocation     - This parameter designated a particular 
 *                                  invocation of this callback function. The 
 *                                  invoke process return invocation when it 
 *                                  responds to the Avilability Management 
 *                                  FrameWork using the saAmfResponse() 
 *                                  function.
 *                 compName       - A pointer to the name of the component 
 *                                  whose readiness stae the Availability 
 *                                  Management Framework is setting.
 *                 haState        - The new HA state to be assumeb by the 
 *                                  component service instance identified by 
 *                                  csiName. 
 *                 TODO transitionDesc - This will indicate whether or not the 
 *                                  component service instance for 
 *                                  ativeCompName went through quiescing.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void fm_saf_CSI_set_callback(SaInvocationT invocation,
                         const SaNameT *compName,
                         SaAmfHAStateT haState,
                         SaAmfCSIDescriptorT csiDescriptor)
{
   FM_AMF_CB *fm_amf_cb;
   SaAisErrorT error = SA_AIS_OK;

   m_NCS_CONS_PRINTF ("fm_saf_CSI_set_callback invoked %s\n",  compName->value);
   fm_amf_cb = fm_amf_take_hdl();
   if (fm_amf_cb != NULL)   
   {
      error = saAmfResponse(fm_amf_cb->amf_hdl,invocation, error);
      m_NCS_CONS_PRINTF ("fm_saf_CSI_set_callback response sent: amf_rc %d\n",  error);
      m_NCS_CONS_PRINTF ("Component Name: %s\n",  fm_amf_cb->comp_name);
   } 
   fm_amf_give_hdl();
   
   return;
}


/****************************************************************************
 * Name          : fm_saf_health_chk_callback
 *
 * Description   : This function SAF callback function which will be called 
 *                 when the AMF framework needs to health for the component.
 *
 * Arguments     : invocation     - This parameter designated a particular 
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it 
 *                                  responds to the Avilability Management 
 *                                  FrameWork using the saAmfResponse() 
 *                                  function.
 *                 compName       - A pointer to the name of the component 
 *                                  whose readiness stae the Availability 
 *                                  Management Framework is setting.
 *                 checkType      - The type of healthcheck to be executed. 
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/
void fm_saf_health_chk_callback (SaInvocationT invocation,
                             const SaNameT *compName,
                             SaAmfHealthcheckKeyT *checkType)
{
   FM_AMF_CB *fm_amf_cb;
   SaAisErrorT error = SA_AIS_OK;

   m_NCS_CONS_PRINTF ("fm_saf_health_chk_callback invoked %s\n",  compName->value);
   fm_amf_cb = fm_amf_take_hdl();
   if (fm_amf_cb != NULL)
   {
      error = saAmfResponse(fm_amf_cb->amf_hdl,invocation, error);
      m_NCS_CONS_PRINTF ("fm_saf_health_chk_callback response sent: amf_rc %d\n",  error);
      m_NCS_CONS_PRINTF ("Component Name: %s\n",  fm_amf_cb->comp_name);
   }
   fm_amf_give_hdl();

   return;
}

/****************************************************************************
 * Name          : fm_saf_CSI_rem_callback
 *
 * Description   : This function SAF callback function which will be called 
 *                 when the AMF framework needs remove the CSI assignment.
 *
 * Arguments     : invocation     - This parameter designated a particular 
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it 
 *                                  responds to the Avilability Management 
 *                                  FrameWork using the saAmfResponse() 
 *                                  function.
 *                 compName       - A pointer to the name of the component 
 *                                  whose readiness stae the Availability 
 *                                  Management Framework is setting.
 *                 csiName        - A pointer to the name of the new component
 *                                  service instance to be supported by the 
 *                                  component or of an alreadt supported 
 *                                  component service instance whose HA state 
 *                                  is to be changed.
 *                 csiFlags       - A value of the choiceflag type which 
 *                                  indicates whether the HA state change must
 *                                  be applied to a new component service 
 *                                  instance or to all component service 
 *                                  instance currently supported by the 
 *                                  component.
 *
 * Return Values : None
 *
 * Notes         : This is not completed - TBD.
 *****************************************************************************/
void fm_saf_CSI_rem_callback (SaInvocationT invocation,
                               const SaNameT *compName,
                               const SaNameT *csiName,
                               const SaAmfCSIFlagsT csiFlags)
{
   FM_AMF_CB *fm_amf_cb;
   SaAisErrorT error = SA_AIS_OK;

   m_NCS_CONS_PRINTF ("fm_saf_CSI_rem_callback invoked %s\n",  compName->value);

   fm_amf_cb = fm_amf_take_hdl();
   if (fm_amf_cb != NULL)   
   {
      error = saAmfResponse(fm_amf_cb->amf_hdl,invocation, error);
      m_NCS_CONS_PRINTF ("fm_saf_CSI_rem_callback response sent: amf_rc %d\n",  error);
      m_NCS_CONS_PRINTF ("Component Name: %s\n",  fm_amf_cb->comp_name);

   }  
   fm_amf_give_hdl();
   return;
   
}

/****************************************************************************
 * Name          : fm_saf_comp_terminate_callback
 *
 * Description   : This function SAF callback function which will be called 
 *                 when the AMF needs to terminate the component.
 *
 * Arguments     : invocation     - This parameter designated a particular 
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it 
 *                                  responds to the Avilability Management 
 *                                  FrameWork using the saAmfResponse() 
 *                                  function.
 *                 compName       - A pointer to the name of the component 
 *                                  whose readiness stae the Availability 
 *                                  Management Framework is setting.
 *
 * Return Values : None
 *
 * Notes         : This is not completed - TBD.
 *****************************************************************************/
void fm_saf_comp_terminate_callback (SaInvocationT invocation,
                                      const SaNameT *compName)
{
   FM_AMF_CB *fm_amf_cb;
   SaAisErrorT error = SA_AIS_OK;

   m_NCS_CONS_PRINTF ("fm_saf_comp_terminate_callback invoked %s\n",  compName->value);
   fm_amf_cb = fm_amf_take_hdl();
   if (fm_amf_cb != NULL)   
   {
      error = saAmfResponse(fm_amf_cb->amf_hdl,invocation, error);
      m_NCS_CONS_PRINTF ("fm_saf_comp_terminate_callback response sent: amf_rc %d\n",  error);
      m_NCS_CONS_PRINTF ("Component Name: %s\n",  fm_amf_cb->comp_name);
   }  
   fm_amf_give_hdl();
   return;

}

/****************************************************************************
 * Name          : fm_amf_init
 *
 * Description   : FM initializes AMF for involking process and registers 
 *                 the various callback functions.
 *
 * Arguments     : fm_amf_cb  - FM control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 fm_amf_init (FM_AMF_CB *fm_amf_cb)
{
   SaAmfCallbacksT amfCallbacks;
   SaVersionT      amf_version;   
   SaAisErrorT     amf_error;

   m_NCS_MEMSET(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));

   amfCallbacks.saAmfHealthcheckCallback        = fm_saf_health_chk_callback;
   amfCallbacks.saAmfCSISetCallback             = fm_saf_CSI_set_callback;
   amfCallbacks.saAmfCSIRemoveCallback          = fm_saf_CSI_rem_callback;
   amfCallbacks.saAmfComponentTerminateCallback = fm_saf_comp_terminate_callback;

   m_FM_GET_AMF_VER(amf_version);

   amf_error = saAmfInitialize(&fm_amf_cb->amf_hdl, &amfCallbacks, &amf_version);
   if (amf_error != SA_AIS_OK)
   {         
         m_NCS_CONS_PRINTF ("FM_COND_AMF_INIT_FAILED: amf_rc %d\n",  amf_error);
         return NCSCC_RC_FAILURE;
   }
   m_NCS_CONS_PRINTF ("FM_COND_AMF_INIT_DONE\n");

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : fm_amf_register
 *
 * Description   : FM registers with AMF to tell AMF that it is ready 
 *
 * Arguments     : fm_amf_cb  - FM control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 fm_amf_register (FM_AMF_CB *fm_amf_cb)
{
   SaAisErrorT    amf_error;
   uns32          res = NCSCC_RC_SUCCESS;
   SaNameT        sname;
   
   sname.length = m_NCS_STRLEN(fm_amf_cb->comp_name);
   m_NCS_STRCPY(sname.value,fm_amf_cb->comp_name);

   /* 
    * register FM component with AvSv 
    */
   amf_error = saAmfComponentRegister(fm_amf_cb->amf_hdl, &sname, (SaNameT*)NULL);
   if (amf_error != SA_AIS_OK)
   {            
      m_NCS_CONS_PRINTF ("FM_COND_AMF_REG_FAILED: amf_rc %d\n",  amf_error);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_CONS_PRINTF ("FM_COND_AMF_REG_DONE\n");
   return (res);
}

/****************************************************************************
 * Name          : fm_amf_unregister
 *
 * Description   : FM unregisters with AMF to tell AMF that it is not ready 
 *
 * Arguments     : fm_amf_cb  - FM control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 fm_amf_unregister (FM_AMF_CB *fm_amf_cb)
{
   SaAisErrorT    amf_error;
   uns32          res = NCSCC_RC_SUCCESS;
   SaNameT        sname;
   
   sname.length = m_NCS_STRLEN(fm_amf_cb->comp_name);
   m_NCS_STRCPY(sname.value, fm_amf_cb->comp_name);

   /* 
    * Unregister FM component with AvSv 
    */
   amf_error = saAmfComponentUnregister(fm_amf_cb->amf_hdl, &sname, (SaNameT*)NULL);
   if (amf_error != SA_AIS_OK)
   {    
      m_NCS_CONS_PRINTF ("FM_COND_AMF_UNREG_FAILED: amf_rc %d\n",  amf_error);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_CONS_PRINTF ("FM_COND_AMF_UNREG_DONE\n");
   return (res);
}

/****************************************************************************
 * Name          : fm_amf_healthcheck_start
 *
 * Description   : FM informs AMF to stat healthcheck 
 *
 * Arguments     : fm_amf_cb  - FM control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 fm_amf_healthcheck_start (FM_AMF_CB *fm_amf_cb)
{
   SaAisErrorT          amf_error;
   SaAmfHealthcheckKeyT Healthy;
   SaNameT              SaCompName;
   char                 *phlth_ptr;
   char                 hlth_str[256];

   /*
   ** Start the AMF health check 
   */   
   m_NCS_MEMSET(&SaCompName,0,sizeof(SaCompName));
   m_NCS_STRCPY(SaCompName.value, fm_amf_cb->comp_name);
   SaCompName.length = m_NCS_STRLEN(fm_amf_cb->comp_name);

   m_NCS_MEMSET(&Healthy, 0, sizeof(Healthy));
   phlth_ptr = m_NCS_OS_PROCESS_GET_ENV_VAR("FM_HA_ENV_HEALTHCHECK_KEY");
   if (phlth_ptr == NULL)
   {
      /*
      ** default health check key 
      */
      m_NCS_STRCPY(hlth_str, "BAD10");
   } 
   else
   {
      m_NCS_STRCPY(hlth_str, phlth_ptr);
   }
   m_NCS_STRCPY(Healthy.key, hlth_str);
   Healthy.keyLen = m_NCS_OS_STRLEN(Healthy.key);

   m_NCS_CONS_PRINTF ("Healthcheck key: %s\n", Healthy.key);
  
   amf_error = saAmfHealthcheckStart(fm_amf_cb->amf_hdl,&SaCompName,&Healthy,
      SA_AMF_HEALTHCHECK_AMF_INVOKED,SA_AMF_COMPONENT_RESTART);
   if (amf_error != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF ("FM_COND_AMF_HEALTH_CHK_START_FAIL: amf_rc %d\n",  amf_error);
       
   } 
   else
   {
      m_NCS_CONS_PRINTF ("FM_COND_AMF_HEALTH_CHK_START_DONE\n");
   }
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : fm_amf_lib_init
 *
 * Description   : This is the function which initalize the agents and AMF.
 *
 * Arguments     : fm_amf_cb - FM control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 fm_amf_lib_init (FM_AMF_CB *fm_amf_cb)
{
   uns32             rc = NCSCC_RC_SUCCESS;
   SaAisErrorT       amf_error = SA_AIS_OK;
   SaNameT           sname;
   
   do
   {
      /* 
      ** Initialize all the AvSv callback 
      */
      rc = fm_amf_init(fm_amf_cb);
      if (rc != NCSCC_RC_SUCCESS)         
         break;
         
      /* 
      ** Get the component name 
      */
      m_NCS_MEMSET(&sname, 0, sizeof(sname));
      amf_error = saAmfComponentNameGet(fm_amf_cb->amf_hdl, &sname);
      if (amf_error != SA_AIS_OK)
      {
         m_NCS_CONS_PRINTF ("FM_COND_AMF_GET_NAME_FAILED: amf_rc %d\n",  amf_error);
         rc = NCSCC_RC_FAILURE;
         break;
      }  
      m_NCS_STRCPY(fm_amf_cb->comp_name, sname.value);

      /* 
      ** Get the AMF selection object 
      */
      amf_error = saAmfSelectionObjectGet(fm_amf_cb->amf_hdl, &fm_amf_cb->amf_fd);
      if (amf_error != SA_AIS_OK)
      {     
         m_NCS_CONS_PRINTF ("FM_COND_AMF_GET_OBJ_FAILED: amf_rc %d\n",  amf_error);
         break;
      }

      /* 
      ** Register FM with AMF 
      */
      rc = fm_amf_register (fm_amf_cb);
      if (rc != NCSCC_RC_SUCCESS)
          break;

      /* 
      ** Start component healthcheck 
      */
      rc = fm_amf_healthcheck_start (fm_amf_cb);
      if (rc != NCSCC_RC_SUCCESS)
          break;
     
   }while (0);

   if (rc != NCSCC_RC_SUCCESS)
   {
      /* 
      ** Destroy the lib 
      */
      fm_amf_lib_destroy (fm_amf_cb);
   }
   
   return (rc);
}

/****************************************************************************
 * Name          : fm_amf_lib_destroy
 *
 * Description   : This is the function which destroy the agents and 
 *                 finalize the AMF.
 *
 * Arguments     : fm_amf_cb - FM control block ponier.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 fm_amf_lib_destroy (FM_AMF_CB * fm_amf_cb)
{ 
   /* 
   ** Unregister FM with AMF 
   */
   fm_amf_unregister (fm_amf_cb);

   /* 
   ** Unintialize FM with AMF 
   */
   saAmfFinalize(fm_amf_cb->amf_hdl);
      
   return (NCSCC_RC_SUCCESS);
     
}

/****************************************************************************
 * Name          : fm_pipe_open
 *
 * Description   : Opens a named pipe to recieve msg from INSTANTIATE scipt 
 *
 * Arguments     : fm_amf_cb  - FM control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 fm_pipe_open (FM_AMF_CB *fm_amf_cb)
{
   int            result;

   /* 
   ** AMF fd and PIPE fd are opened on mutually exclusive basis 
   */
   fm_amf_cb->is_amf_up = FALSE;
      
   /* 
   ** Open the named pipe 
   */
   fm_amf_cb-> pipe_fd = open (FM_HA_COMP_NAMED_PIPE, O_RDONLY | O_NONBLOCK);
   if (fm_amf_cb-> pipe_fd == -1)
   {
      /* Create the pipe since FM comes up before NCS starts it */ 
      result = mkfifo (FM_HA_COMP_NAMED_PIPE, S_IWUSR | S_IRUSR | S_IRGRP);
      if (result != 0 && result != EEXIST)
      {
         return NCSCC_RC_FAILURE;
      }
      
      /* Try to open the pipe now */
      fm_amf_cb-> pipe_fd = open (FM_HA_COMP_NAMED_PIPE, O_RDONLY | O_NONBLOCK);
      if (fm_amf_cb-> pipe_fd == -1)
      {
        /* Unable to open the pipe */ 
        return NCSCC_RC_FAILURE;
      } 
   }

   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : fm_pipe_close
 *
 * Description   : Closes an opened pipe and unliks it.
 *
 * Arguments     : fm_amf_cb  - FM control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 fm_pipe_close (FM_AMF_CB *fm_amf_cb)
{
   int            result;

   if (fm_amf_cb-> pipe_fd == -1)
       return NCSCC_RC_SUCCESS ;

   /* 
   ** Close the named pipe 
   */
   close (fm_amf_cb-> pipe_fd);
   fm_amf_cb-> pipe_fd = -1;
       
   /* 
   ** Remove the named pipe 
   */
   result = unlink (FM_HA_COMP_NAMED_PIPE);
   if (result != 0)
   { 
       return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS ;
}

/****************************************************************************
 * Name          : fm_amf_open
 *
 * Description   : Opens AMF 
 *
 * Arguments     : fm_amf_cb  - FM control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 fm_amf_open (FM_AMF_CB *fm_amf_cb)
{
   if (fm_pipe_open(fm_amf_cb) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }

   /* 
   ** AMF fd and PIPE fd are opened on mutually exclusive basis
   */
   fm_amf_cb->is_amf_up = FALSE;

   return NCSCC_RC_SUCCESS ;
}

/****************************************************************************
 * Name          : fm_amf_close
 *
 * Description   : Closes AMF.
 *
 * Arguments     : fm_amf_cb  - FM control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 fm_amf_close (FM_AMF_CB *fm_amf_cb)
{
   uns32 rc;
   if (fm_amf_cb->is_amf_up)
   {
       rc = fm_amf_lib_destroy (fm_amf_cb);
   }
   else
   {
       rc = fm_pipe_close (fm_amf_cb);

   }

   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:       fm_amf_pipe_process_msg

  DESCRIPTION:          Read and Process a message received on the pipe
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

*****************************************************************************/

uns32 fm_amf_pipe_process_msg (FM_AMF_CB *fm_amf_cb)
{
   uns32    rc             = NCSCC_RC_SUCCESS;
   int      msg_size       =  0;
   char     comp_name[256] = {0};
   char     *pc            = NULL;

   /***************************************************************\
    *                                                               *
    *         Read from pipe into input buffer                      *
    *                                                               *
   \***************************************************************/
   msg_size = read (fm_amf_cb-> pipe_fd, comp_name, sizeof(comp_name));
   
   if (msg_size < 0)
   {
      if (errno != EINTR && errno != EWOULDBLOCK)
          /* log Non-benign error */     
      return NCSCC_RC_FAILURE;
   }

   /*
   ** Remove the | that separates comp name and healthcheck key 
   */
   pc = strchr (comp_name, '|');
   if (pc != NULL)
      *pc = '\0';

   /*
   ** Check if component name is empty
   */
   if (m_NCS_OS_STRLEN (comp_name) == 0)
   {
       return NCSCC_RC_FAILURE;
   }


   pc++;

   /*
   ** Check if health check key is empty
   */
   if (m_NCS_OS_STRLEN (pc) == 0)
   {
       return NCSCC_RC_FAILURE;

   }


   /*
   ** Set SA_AMF_COMPONENT_NAME env variable 
   */
   if (m_NCS_OS_PROCESS_SET_ENV_VAR ("SA_AMF_COMPONENT_NAME", comp_name, 1) == -1)
   {
       return NCSCC_RC_FAILURE;
   }

   /*
   ** Set FM_HA_ENV_HEALTHCHECK_KEY env variable 
   */
   if (m_NCS_OS_PROCESS_SET_ENV_VAR ("FM_HA_ENV_HEALTHCHECK_KEY", pc, 1) == -1)
   {
       return NCSCC_RC_FAILURE;
   }
      
   /* 
   ** We recieved a signal on the pipe, init lib 
   */
   rc = fm_amf_lib_init (fm_amf_cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      /* Go back to initial state */
      fm_amf_cb->is_amf_up = FALSE;
      return rc;
   }       
   
   /* 
   ** Close the pipe 
   */
   fm_pipe_close (fm_amf_cb);

   /* 
   ** AMF is up 
   */
   fm_amf_cb->is_amf_up = TRUE;

   return rc; 
}
/*****************************************************************************

  PROCEDURE NAME:       fm_amf_process_msg

  DESCRIPTION:          Process a message received from AMF
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

*****************************************************************************/

uns32 fm_amf_process_msg (FM_AMF_CB *fm_amf_cb)
{
   SaAisErrorT amf_error = SA_AIS_OK;

   /* 
   ** Dispatch all the AMF pending function 
   */
   amf_error = saAmfDispatch(fm_amf_cb->amf_hdl, SA_DISPATCH_ALL);
   if (amf_error != SA_AIS_OK)
   {
      return NCSCC_RC_FAILURE;
   }
   
   return NCSCC_RC_SUCCESS ;
}

/**************************************************/



