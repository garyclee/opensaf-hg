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

  MODULE NAME: SRMA_API.C 
..............................................................................

  DESCRIPTION: This file contains SRMSV API implementation 

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srma.h"

extern uns32 gl_srma_hdl;


/****************************************************************************
  Name          :  ncs_srmsv_initialize
 
  Description   :  This function initializes the SRMSv for the invoking 
                   process and registers the various callback functions.
 
  Arguments     :  srmsv_handle   - ptr to the SRMSv handle
                   srmsv_callback - ptr to a NCS_SRMSV_CALLBACK function ptr
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR  ncs_srmsv_initialize(NCS_SRMSV_CALLBACK srmsv_callback,
                                    NCS_SRMSV_HANDLE   *srmsv_handle)
{
   SRMA_CB       *srma = SRMA_CB_NULL;
   NCS_SRMSV_ERR rc = SA_AIS_OK;
   int argc = 0;
   char **argv = NULL;
  
   /* Initialize the environment */
   if (ncs_agents_startup(argc, argv) != NCSCC_RC_SUCCESS)
   {
      return SA_AIS_ERR_LIBRARY;
   }

   /* Create SRMA CB */
   if (ncs_srma_startup() != NCSCC_RC_SUCCESS)
   {
      ncs_agents_shutdown(argc, argv);
      return SA_AIS_ERR_LIBRARY;
   }

   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_INITIALIZE,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   if (!(srmsv_handle))
   {
      m_SRMA_LOG_API(SRMA_LOG_API_INITIALIZE,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);
      return SA_AIS_ERR_INVALID_PARAM;
   }
   
   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);
      return SA_AIS_ERR_LIBRARY;
   }
   
   /* acquire cb write lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   /* Register the callback function and returns the respective handle */
   rc = srma_usr_appl_create(srma, srmsv_callback, srmsv_handle);

   /* release cb write lock */
   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);
   
   /* return SRMA CB */
   m_SRMSV_GIVEUP_SRMA_CB;

   /* LOG the status of api processing */
   if (rc == SA_AIS_OK)
      m_SRMA_LOG_API(SRMA_LOG_API_INITIALIZE,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);
   return rc;
}


/****************************************************************************
  Name          :  ncs_srmsv_selection_object_get
 
  Description   :  This function returns the OS handle associated with the
                   handle srmsv_handle. The invoking process can use this
                   handle to detect pending callbacks.
 
  Arguments     :  srmsv_handle  - SRMSv handle value
                   srmsv_sel_obj - ptr to the selection object
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR  ncs_srmsv_selection_object_get(NCS_SRMSV_HANDLE   srmsv_handle,
                                              SaSelectionObjectT *srmsv_sel_obj)
{
   SRMA_CB       *srma = SRMA_CB_NULL;
   NCS_SRMSV_ERR rc = SA_AIS_OK;
   SRMA_USR_APPL_NODE *appl_node = SRMA_USR_APPL_NODE_NULL;

   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_SEL_OBJ_GET,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   if (!srmsv_handle)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_SEL_OBJ_GET,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);
      return SA_AIS_ERR_BAD_HANDLE;
   }

   if (!srmsv_sel_obj)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_SEL_OBJ_GET,
                     SRMA_LOG_API_ERR_SA_INVALID_PARAM,
                     NCSFL_SEV_ERROR);
      return SA_AIS_ERR_INVALID_PARAM;
   }
   
   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);
      return SA_AIS_ERR_LIBRARY; 
   }
   
   /* acquire cb read lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   /* retrieve hdl rec */
   if (!(appl_node = (SRMA_USR_APPL_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                                         srmsv_handle)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);

      /* release cb read lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      /* release SRMA CB handle */
      m_SRMSV_GIVEUP_SRMA_CB;

      m_SRMA_LOG_API(SRMA_LOG_API_SEL_OBJ_GET,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }

   /* everything's fine.. pass the sel obj to the appl */
   *srmsv_sel_obj = (SaSelectionObjectT)m_GET_FD_FROM_SEL_OBJ(appl_node->sel_obj);

   /* release cb read lock */
   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

   /* release SRMA CB handle */
   m_SRMSV_GIVEUP_SRMA_CB; 

   /* release Application specific handle */
   ncshm_give_hdl(srmsv_handle);
   
   if (!(*srmsv_sel_obj))
   {  /* Log the Error */
      m_SRMA_LOG_API(SRMA_LOG_API_SEL_OBJ_GET,
                     SRMA_LOG_API_ERR_SA_LIBRARY,
                     NCSFL_SEV_CRITICAL);
   }
   else
   {
      m_SRMA_LOG_API(SRMA_LOG_API_SEL_OBJ_GET,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);
   }

   return rc;
}


/****************************************************************************
  Name          :  ncs_srmsv_dispatch
 
  Description   :  This function invokes, in the context of the calling thread,
                   pending callbacks for the srmsv_handle in a way that is
                   specified by the srmsv_dispatch_flags parameter.

  Arguments     :  srmsv_handle  -  SRMSv handle
                   srmsv_dispatch_flags - one of the flag defined in 
                                          SaDispatchFlagsT
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR  ncs_srmsv_dispatch(NCS_SRMSV_HANDLE srmsv_handle,
                                  SaDispatchFlagsT srmsv_dispatch_flags)
{
   SRMA_CB        *srma = SRMA_CB_NULL;
   NCS_SRMSV_ERR  rc = SA_AIS_OK;
   
   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_DISPATCH,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   /* Handle is not there?? :-( ok return as Invalid param */
   if (!srmsv_handle)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_DISPATCH,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);
      return SA_AIS_ERR_BAD_HANDLE;
   }

   if (!m_SRMA_DISPATCH_FLAG_IS_VALID(srmsv_dispatch_flags))
   {
      m_SRMA_LOG_API(SRMA_LOG_API_DISPATCH,
                     SRMA_LOG_API_ERR_SA_BAD_FLAGS,
                     NCSFL_SEV_ERROR);
      return SA_AIS_ERR_BAD_FLAGS;
   }
   
   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);

      return SA_AIS_ERR_LIBRARY; 
   }
      
   /* acquire cb write lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   /* Dispatches the application specific data */
   rc = srma_hdl_cbk_dispatch(srma, srmsv_handle, srmsv_dispatch_flags);

   /* release SRMA CB handle */
   m_SRMSV_GIVEUP_SRMA_CB;

   /* release cb write lock */
   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

   /* LOG the status of api processing */
   if (rc == SA_AIS_OK)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_DISPATCH,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);
   }
   else
   {
      m_SRMA_LOG_API(SRMA_LOG_API_DISPATCH,
                     SRMA_LOG_API_ERR_SA_LIBRARY,
                     NCSFL_SEV_ERROR);
   }

   return rc;
}


/****************************************************************************
  Name          :  ncs_srmsv_finalize
 
  Description   :  This request de-registers the application from SRMSv. 
                   After this call, application cannot avail any services of
                   SRMSv. On getting this request, SRMSv will free the 
                   application specific registered information.

  Arguments     :  srmsv_handle - SRMSv handle                   
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR ncs_srmsv_finalize(NCS_SRMSV_HANDLE srmsv_hdl)
{
   SRMA_CB *srma = NULL;
   NCS_SRMSV_ERR rc = SA_AIS_OK;

   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_FINALIZE,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   if (!srmsv_hdl)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_FINALIZE,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);
      return SA_AIS_ERR_BAD_HANDLE;
   }     

   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);

      return SA_AIS_ERR_LIBRARY;
   }
      
   /* Deletes all the acquired resources for an application and de-registers
      application from SRMSv service */
   rc = srma_usr_appl_delete(srma, srmsv_hdl, TRUE);

   /* release SRMA CB handle */
   m_SRMSV_GIVEUP_SRMA_CB;
  
   /* LOG the status of api processing */
   if (rc == SA_AIS_OK)
   {
      int argc = 0;
      char **argv = NULL;

      m_SRMA_LOG_API(SRMA_LOG_API_FINALIZE,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);

      ncs_srma_shutdown();
      ncs_agents_shutdown(argc, argv);
   }
   else
   {
      m_SRMA_LOG_API(SRMA_LOG_API_FINALIZE,
                     SRMA_LOG_API_ERR_SA_LIBRARY,
                     NCSFL_SEV_ERROR);
   }

   return rc;
}


/****************************************************************************
  Name          :  ncs_srmsv_restart_monitor
 
  Description   :  On getting this request, SRMSv restarts the monitoring 
                   process on all the resources (that are in stop mode)
                   requested by the application.

  Arguments     :  srmsv_handle   - ptr to the SRMSv handle                   
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR  ncs_srmsv_restart_monitor(NCS_SRMSV_HANDLE srmsv_hdl)
{
   SRMA_CB       *srma = 0;
   NCS_SRMSV_ERR rc = SA_AIS_OK;

   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_START_MON,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   if (!srmsv_hdl)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_START_MON,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }
      
   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);

      return SA_AIS_ERR_LIBRARY;
   }    
  
   /* SRMSv starts monitoring all the application resources */
   rc = srma_appl_start_mon(srma, srmsv_hdl);

   /* release SRMA CB handle */
   m_SRMSV_GIVEUP_SRMA_CB;
   
   /* LOG the status of api processing */
   if (rc == SA_AIS_OK)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_START_MON,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);
   }
   else
   {
      m_SRMA_LOG_API(SRMA_LOG_API_START_MON,
                     SRMA_LOG_API_ERR_SA_LIBRARY,
                     NCSFL_SEV_ERROR);
   }

   return rc;
}


/****************************************************************************
  Name          :  ncs_srmsv_stop_monitor
 
  Description   :  On getting this request, SRMSv stops the monitoring
                   process on all the resources requested by an application.

  Arguments     :  srmsv_handle - SRMSv handle                   
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR  ncs_srmsv_stop_monitor(NCS_SRMSV_HANDLE srmsv_hdl)
{
   SRMA_CB       *srma = 0;
   NCS_SRMSV_ERR rc = SA_AIS_OK;

   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_STOP_MON,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   if (!srmsv_hdl)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_STOP_MON,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }

   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);

      return SA_AIS_ERR_LIBRARY;
   }
      
   /* SRMSv stops monitoring all the application resources */
   rc = srma_appl_stop_mon(srma, srmsv_hdl);

   /* release SRMA CB handle */
   m_SRMSV_GIVEUP_SRMA_CB;

   /* LOG the status of api processing */
   if (rc == SA_AIS_OK)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_STOP_MON,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);
   }
   else
   {
      m_SRMA_LOG_API(SRMA_LOG_API_STOP_MON,
                     SRMA_LOG_API_ERR_SA_LIBRARY,
                     NCSFL_SEV_ERROR);
   }

   return rc;
}


/****************************************************************************
  Name          :  ncs_srmsv_start_rsrc_mon
 
  Description   :  This request configures the application requested resource
                   monitor data with SRMSv. On getting this request, SRMSv 
                   starts monitoring process on the specified resources.

  Arguments     :  srmsv_handle - SRMSv handle value
                   mon_info     - Requested Resource Monitor Data
                   rsrc_mon_hdl - returns the handle of the created resource
                                  monitor record
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR ncs_srmsv_start_rsrc_mon(NCS_SRMSV_HANDLE srmsv_hdl,
                                       NCS_SRMSV_MON_INFO *mon_info,
                                       NCS_SRMSV_RSRC_HANDLE *rsrc_mon_hdl)
{
   SRMA_CB       *srma = 0;
   NCS_SRMSV_ERR rc = SA_AIS_OK;

   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_START_RSRC_MON,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   if (!(srmsv_hdl))
   {
      m_SRMA_LOG_API(SRMA_LOG_API_START_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }

   if (!(rsrc_mon_hdl)) 
   {
      m_SRMA_LOG_API(SRMA_LOG_API_START_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_INVALID_PARAM;
   }

   if (!(mon_info))
   {
      m_SRMA_LOG_API(SRMA_LOG_API_START_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_INVALID_PARAM,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_INVALID_PARAM;
   }

   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);

      return SA_AIS_ERR_LIBRARY;
   }
      
   /* If rsrc_mon_hdl is 0, implies it as a create resource monitor request
      if it is non-NULL, implies it as a modify request */
   if (*rsrc_mon_hdl == 0)
      rc = srma_appl_create_rsrc_mon(srma, srmsv_hdl, mon_info, rsrc_mon_hdl);
   else
      rc = srma_appl_modify_rsrc_mon(srma, mon_info, *rsrc_mon_hdl);

   /* release SRMA CB handle */
   m_SRMSV_GIVEUP_SRMA_CB;

   /* LOG the status of api processing */
   if ((rc == SA_AIS_OK) || (rc == SA_AIS_ERR_NOT_EXIST))
   {
      m_SRMA_LOG_API(SRMA_LOG_API_START_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);
   }
   else
   {
      m_SRMA_LOG_API(SRMA_LOG_API_START_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_LIBRARY,
                     NCSFL_SEV_ERROR);
   }

   return rc;
}


/****************************************************************************
  Name          :  ncs_srmsv_stop_rsrc_mon
 
  Description   :  This request configures the application requested resource
                   monitor data with SRMSv. On getting this request, SRMSv 
                   starts monitoring process on the specified resources.

  Arguments     :  rsrc_mon_hdl - Handle of the created resource monitor record
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR ncs_srmsv_stop_rsrc_mon(NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl)
{
   SRMA_CB       *srma = 0;
   NCS_SRMSV_ERR rc = SA_AIS_OK;

   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_STOP_RSRC_MON,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   if (!(rsrc_mon_hdl))
   {
      m_SRMA_LOG_API(SRMA_LOG_API_STOP_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }

   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);

      return SA_AIS_ERR_LIBRARY;
   }
      
   /* Deletes the configured resource monitor data from SRMSv service */
   rc = srma_appl_delete_rsrc_mon(srma, rsrc_mon_hdl, TRUE);
    
   /* return SRMA CB */
   m_SRMSV_GIVEUP_SRMA_CB;

   /* LOG the status of api processing */
   if (rc == SA_AIS_OK)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_STOP_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);
   }
   else
   {
      m_SRMA_LOG_API(SRMA_LOG_API_STOP_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_LIBRARY,
                     NCSFL_SEV_ERROR);
   }

   return rc;
}


/****************************************************************************
  Name          :  ncs_srmsv_subscr_rsrc_mon
 
  Description   :  Through this request application will subscribe to all the
                   SRMSv threshold events that are specific to a resource or
                   resources. On getting this request, SRMSv sends the resource
                   specific threshold events to the application.

  Arguments     :  srmsv_hdl     - SRMSv handle value
                   rsrc_info     - RSRC Specific Data to which it subscribes
                   subscr_handle - Handle of the create subscriber record
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR ncs_srmsv_subscr_rsrc_mon(NCS_SRMSV_HANDLE srmsv_hdl,
                                        NCS_SRMSV_RSRC_INFO *rsrc_info,
                                        NCS_SRMSV_SUBSCR_HANDLE *subscr_handle)
{
   SRMA_CB       *srma = 0;
   NCS_SRMSV_ERR rc = SA_AIS_OK;

   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_SUBSCR_RSRC_MON,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   if (!(srmsv_hdl))
      return SA_AIS_ERR_BAD_HANDLE;

   if (!(rsrc_info) || !(subscr_handle))
      return SA_AIS_ERR_INVALID_PARAM;

   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);

      return SA_AIS_ERR_LIBRARY;
   }
      
   /* Subscribes to a particular resource monitor event */
   rc = srma_appl_subscr_rsrc_mon(srma, srmsv_hdl, rsrc_info, subscr_handle);

   /* release SRMA CB handle */
   m_SRMSV_GIVEUP_SRMA_CB;

   /* LOG the status of api processing */
   if ((rc == SA_AIS_OK) || (rc == SA_AIS_ERR_NOT_EXIST))
   {
      m_SRMA_LOG_API(SRMA_LOG_API_SUBSCR_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);
   }
   else
   {
      m_SRMA_LOG_API(SRMA_LOG_API_SUBSCR_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_LIBRARY,
                     NCSFL_SEV_ERROR);
   }

   return rc;
}


/****************************************************************************
  Name          :  ncs_srmsv_unsubscr_rsrc_mon
 
  Description   :  Through this request application will unsubscribe to all 
                   the subscribed SRMSv threshold events of a resource or
                   resources. On getting this request, SRMSv stops sending
                   the resource specific threshold events to the application.
                   Through one request, application can unsubscribe from the
                   subscribed threshold events of multiple resources.

  Arguments     :  subscr_handle - Handle of the created subscriber record
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR ncs_srmsv_unsubscr_rsrc_mon(NCS_SRMSV_SUBSCR_HANDLE subscr_handle)
{
   SRMA_CB       *srma = 0;
   NCS_SRMSV_ERR rc = SA_AIS_OK;

   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_UNSUBSCR_RSRC_MON,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);

      return SA_AIS_ERR_LIBRARY;
   }
  
   if (!(subscr_handle))
   {
      m_SRMA_LOG_API(SRMA_LOG_API_UNSUBSCR_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }

   /* Deleting the respective subscription from SRMA database */
   rc = srma_appl_delete_rsrc_mon(srma, subscr_handle, TRUE);

   /* return SRMA CB */
   m_SRMSV_GIVEUP_SRMA_CB;
 
   /* LOG the status of api processing */
   if (rc == SA_AIS_OK)
   {
      m_SRMA_LOG_API(SRMA_LOG_API_UNSUBSCR_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);
   }
   else
   {
      m_SRMA_LOG_API(SRMA_LOG_API_UNSUBSCR_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_LIBRARY,
                     NCSFL_SEV_ERROR);
   }

   return rc;
}



/****************************************************************************
  Name          :  ncs_srmsv_get_watermark_val
 
  Description   :  This request gets the watermark values of a particular 
                   resource.

  Arguments     :  srmsv_handle - SRMSv handle value
                   rsrc_info    - Requested Resource Data
                   wm_type      - Type of the watermark (low/high/dual)
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR ncs_srmsv_get_watermark_val(NCS_SRMSV_HANDLE srmsv_hdl,
                                          NCS_SRMSV_RSRC_INFO *rsrc_info,
                                          NCS_SRMSV_WATERMARK_TYPE wm_type)
{
   SRMA_CB        *srma = 0;
   NCS_SRMSV_ERR  rc = SA_AIS_OK;

   /* LOG the entry of API */
   m_SRMA_LOG_API(SRMA_LOG_API_GET_WATERMARK_VAL,
                  SRMA_LOG_API_FUNC_ENTRY,
                  NCSFL_SEV_DEBUG);

   if (!(srmsv_hdl))
   {
      m_SRMA_LOG_API(SRMA_LOG_API_GET_WATERMARK_VAL,
                     SRMA_LOG_API_ERR_SA_BAD_HANDLE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }

   /* retrieve SRMA CB */
   if (!(srma = m_SRMSV_RETRIEVE_SRMA_CB))
   {
      m_SRMA_LOG_CB(SRMSV_LOG_CB_RETRIEVE,
                    SRMSV_LOG_CB_FAILURE,
                    NCSFL_SEV_CRITICAL);

      return SA_AIS_ERR_LIBRARY;
   }
     
   rc = srma_get_watermark_val(srma, srmsv_hdl, rsrc_info, wm_type);

   /* release SRMA CB handle */
   m_SRMSV_GIVEUP_SRMA_CB;

   /* LOG the status of api processing */
   if (rc == SA_AIS_OK) 
   {
      m_SRMA_LOG_API(SRMA_LOG_API_GET_WATERMARK_VAL,
                     SRMA_LOG_API_ERR_SA_OK,
                     NCSFL_SEV_INFO);
   }
   else
   {
      if (rc == SA_AIS_ERR_NOT_EXIST)
      {
         m_SRMA_LOG_API(SRMA_LOG_API_GET_WATERMARK_VAL,
                        SRMA_LOG_API_ERR_SA_NOT_EXIST,
                        NCSFL_SEV_WARNING);
      }
      else
         m_SRMA_LOG_API(SRMA_LOG_API_GET_WATERMARK_VAL,
                        SRMA_LOG_API_ERR_SA_LIBRARY,
                        NCSFL_SEV_ERROR);
   }

   return rc;
}


