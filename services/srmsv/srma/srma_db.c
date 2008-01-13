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

  MODULE NAME: SRMA_PROC.C
 
..............................................................................

  DESCRIPTION: This file contains SRMSV API implementation supported functions

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srma.h"


/****************************************************************************
  Name          :  srma_appl_delete_rsrc_mon
 
  Description   :  This request modifies the application requested resource 
                   monitor data. On getting the request, SRMSv restarts
                   monitoring process on the requested resources based on the
                   modified data.

  Arguments     :  srma - ptr to the SRMA CB structure
                   rsrc_mon_hdl - Handle of the created resource monitor record
                   mon_info     - Resource Monitor Data
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR srma_appl_delete_rsrc_mon(SRMA_CB *srma,
                                        NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl,
                                        NCS_BOOL send_msg)
{
   NCS_SRMSV_ERR  rc = SA_AIS_OK;
   SRMA_RSRC_MON  *rsrc_mon = SRMA_RSRC_MON_NULL;


   if (!(rsrc_mon_hdl))
      return SA_AIS_ERR_INVALID_PARAM;

   /* retrieve application specific SRMSv record */   
   rsrc_mon = (SRMA_RSRC_MON*)ncs_patricia_tree_get(&srma->rsrc_mon_tree, (uns8*)&rsrc_mon_hdl);
   if (!(rsrc_mon))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }

   /* Got the association record for the given rsrc_mon_hdl */
   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* Delete rsrc from the timer List if it associates */
   if (rsrc_mon->tmr_id)
      srma_del_rsrc_from_monitoring(srma, rsrc_mon, TRUE);

   /* send rsrc DEL event to SRMND */
   if (send_msg)
   { 
      if (rsrc_mon->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
      {
         srma_send_rsrc_msg(srma,
                            rsrc_mon->bcast_appl, 
                            NULL,
                            SRMA_DEL_RSRC_MON_MSG,
                            rsrc_mon);         
      }
      else
      if (rsrc_mon->srmnd_usr->srmnd_info->is_srmnd_up == TRUE)
      {
         srma_send_rsrc_msg(srma,
                            rsrc_mon->usr_appl->usr_appl, 
                            &rsrc_mon->srmnd_usr->srmnd_info->srmnd_dest, 
                            SRMA_DEL_RSRC_MON_MSG,
                            rsrc_mon);         
      }
   }

   /* Delete the respective resource mon node from the resource-mon tree */
   ncs_patricia_tree_del(&srma->rsrc_mon_tree, &rsrc_mon->rsrc_pat_node);

   /* Any cbk recs.. just delete them from cbk list */
   if (send_msg)
      srma_free_cbk_rec(rsrc_mon);

   /* Delete the rsrc from the bcast list of SRMA */
   if (rsrc_mon->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
   {
      srma_del_rsrc_mon_from_bcast_list(rsrc_mon, rsrc_mon->bcast_appl);
   }
   else
   {
      /* Delete it from the user application node */
      if (rsrc_mon->usr_appl)
         srma_del_rsrc_mon_from_appl_list(srma, rsrc_mon);
      
      /* Delete it from the SRMND node */
      if (rsrc_mon->srmnd_usr)
         srma_del_rsrc_mon_from_srmnd_list(srma, rsrc_mon);
   } 

   /* Destroy the hdl mgr for resource-mon bindings */
   ncshm_destroy_hdl(NCS_SERVICE_ID_SRMA, rsrc_mon->rsrc_mon_hdl);
               
   /* Free the allocated memory for resource-mon node */
   m_MMGR_FREE_SRMA_RSRC_MON(rsrc_mon);

   return rc;
}


/****************************************************************************
  Name          :  srma_usr_appl_delete
 
  Description   :  This routine deletes the application specific resource 
                   monitor data from SRMA database. Basically the application
                   and its corresponding resource monitor de-registers from
                   SRMSv service.
                   
  Arguments     :  srma - ptr to the SRMA CB structure
                   srmsv_handle - SRMSv handle
                   appl_del - flag indicates whether the application specific
                              node to deleted or not.
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR srma_usr_appl_delete(SRMA_CB *srma,
                                   NCS_SRMSV_HANDLE srmsv_hdl,
                                   NCS_BOOL appl_del)
{
   SRMA_SRMND_USR_NODE *usr_srmnd, *next_usr_srmnd;
   SRMA_RSRC_MON       *rsrc_mon, *next_rsrc_mon;
   SRMA_USR_APPL_NODE  *appl = SRMA_USR_APPL_NODE_NULL;
   NCS_SRMSV_ERR       rc = SA_AIS_OK;   

   /* acquire cb write lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   /* retrieve application specific SRMSv record */
   if (!(appl = (SRMA_USR_APPL_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                                     srmsv_hdl)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_BAD_HANDLE;
   }
   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   if (appl->bcast_rsrcs)
   {
      /* Send unregister event to all SRMNDs */
      /* release cb write lock, in order to avoid deadlock situation that
         occurs while performing MDS Op */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      if (srma_send_appl_msg(srma,
                             SRMA_UNREGISTER_MSG,
                             appl->user_hdl,
                             NULL) != NCSCC_RC_SUCCESS)
      {
         /* return application specific SRMSv handle */
         ncshm_give_hdl(srmsv_hdl);
         return SA_AIS_ERR_LIBRARY;
      }

      /* acquire cb write lock */
      m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);
   }

   usr_srmnd = appl->start_srmnd_node;   

   /* Traverse the complete application specific SRMND list and delete 
      its resource mon nodes */
   while (usr_srmnd)
   {
      /* Send unregister event to the respective SRMND */
      /* Even if the destination SRMND is not UP.. delete the respective 
         application specific resource monitor data from SRMA */
      if ((usr_srmnd->srmnd_info->is_srmnd_up) && (appl->bcast_rsrcs == NULL))
      {
         /* release cb write lock, in order to avoid deadlock situation that
            occurs while performing MDS Op */
         m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

         if (srma_send_appl_msg(srma,
                                SRMA_UNREGISTER_MSG,
                                appl->user_hdl,
                                usr_srmnd->srmnd_info) != NCSCC_RC_SUCCESS)
         {
            /* return application specific SRMSv handle */
            ncshm_give_hdl(srmsv_hdl);
            return SA_AIS_ERR_LIBRARY;
         }

         /* acquire cb write lock */
         m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);
      }

      /* Hold the ptr of next application specific SRMND node */
      next_usr_srmnd = usr_srmnd->next_usr_node;
      
      /* Get the first node of the resource-mon tree */
      rsrc_mon = usr_srmnd->start_rsrc_mon;

      /* Traverse the complete application specific resource-mon list and
         delete its resource mon nodes */
      while (rsrc_mon)
      {  
         next_rsrc_mon = rsrc_mon->next_usr_rsrc_mon;

         /* Delete the resource monitor record from SRMA database */
         rc = srma_appl_delete_rsrc_mon(srma, rsrc_mon->rsrc_mon_hdl, FALSE);
         if (rc != SA_AIS_OK)
         {          
            /* return application specific SRMSv handle */
            ncshm_give_hdl(srmsv_hdl);

            /* release cb write lock */
            m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

            return SA_AIS_ERR_LIBRARY;
         }

         /* Continue deleting the next resource-mon node */
         rsrc_mon = next_rsrc_mon;
      }

      /* Continue deleting the application specific next SRMND node */
      usr_srmnd = next_usr_srmnd;
   }  /* End of while usr_srmnd loop */

   if (appl->bcast_rsrcs != NULL)
   {
      rsrc_mon = appl->bcast_rsrcs;
      while (rsrc_mon)
      {
         next_rsrc_mon = rsrc_mon->next_srmnd_rsrc;

         /* Delete the resource monitor record from SRMA database */
         rc = srma_appl_delete_rsrc_mon(srma, rsrc_mon->rsrc_mon_hdl, TRUE);
         if (rc != SA_AIS_OK)
         {          
            /* return application specific SRMSv handle */
            ncshm_give_hdl(srmsv_hdl);

            /* release cb write lock */
            m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

            return SA_AIS_ERR_LIBRARY;
         }
        
         rsrc_mon = next_rsrc_mon;
      }

      appl->bcast_rsrcs = NULL;
   }

   appl->start_srmnd_node = NULL;

   /* Delete the cbk data of an application */
   srma_hdl_cbk_dispatch_all(srma, appl, TRUE);

   /* destroy the selection object */
   if (m_GET_FD_FROM_SEL_OBJ(appl->sel_obj))
      m_NCS_SEL_OBJ_DESTROY(appl->sel_obj);

   /* return application specific SRMSv handle */
   ncshm_give_hdl(srmsv_hdl);

   /* Destroy the hdl mgr for user-application bindings */
   ncshm_destroy_hdl(NCS_SERVICE_ID_SRMA, appl->user_hdl);

   /* Free the allocated memory for application node */
   if (appl_del)
      srma_del_usr_appl_node(srma, appl);

   /* release cb write lock */
   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

   return rc;
}


/****************************************************************************
  Name          :  srma_usr_appl_create
 
  Description   :  This function creates application specific SRMSv record,
                   registers the callback function and returns the 
                   corresponding handle to the application.
 
  Arguments     :  srma - Ptr to the SRMA control block
                   srmsv_cbk - ptr to a NCS_SRMSV_CALLBACK function ptr
                   srmsv_hdl - handle return of the created application 
                               specific SRMSv record.
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR srma_usr_appl_create(SRMA_CB *srma,
                                   NCS_SRMSV_CALLBACK srmsv_cbk,
                                   NCS_SRMSV_HANDLE   *srmsv_hdl)
{
   NCS_SRMSV_ERR  rc = SA_AIS_OK;
   SRMA_USR_APPL_NODE *appl = NULL;

   if ((appl = m_MMGR_ALLOC_SRMA_USR_APPL_NODE) == NULL)
   {     
      m_SRMA_LOG_MEM(SRMA_MEM_APPL_INFO,
                     SRMA_MEM_ALLOC_FAILED,
                     NCSFL_SEV_CRITICAL);

      return SA_AIS_ERR_NO_MEMORY;
   }

   m_NCS_OS_MEMSET((char *)appl, 0, sizeof(SRMA_USR_APPL_NODE));

   /* create the association with hdl-mngr */
   if (!(appl->user_hdl = ncshm_create_hdl(srma->pool_id,
                                           NCS_SERVICE_ID_SRMA, 
                                           (NCSCONTEXT)appl)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_CREATE_USER,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_CRITICAL);

      /* Delete SRMA Control Block */
      m_MMGR_FREE_SRMA_USR_APPL_NODE(appl);

      return SA_AIS_ERR_LIBRARY;      
   }

   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_CREATE_USER,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* Update the callback function ptr */
   appl->srmsv_call_back = srmsv_cbk;

   /* create the selection object & store it in hdl rec */
   m_NCS_SEL_OBJ_CREATE(&appl->sel_obj);
   if (!m_GET_FD_FROM_SEL_OBJ(appl->sel_obj))
   {
      m_SRMA_LOG_API(SRMA_LOG_API_SEL_OBJ_GET,
                     SRMA_LOG_API_ERR_SA_LIBRARY,
                     NCSFL_SEV_CRITICAL);

      /* remove the association with hdl-mngr */
      ncshm_destroy_hdl(NCS_SERVICE_ID_SRMA, appl->user_hdl);

      /* Free the allocated memory for Application Node */
      m_MMGR_FREE_SRMA_USR_APPL_NODE(appl);

      return SA_AIS_ERR_LIBRARY;
   }
     
   /* Add the respective usr application node to the usr application list
      of SRMA */
   srma_add_usr_appl_node(srma, appl);
   
   /* Return the application specific handle to the user */
   *srmsv_hdl = appl->user_hdl;

   return rc;
}


/****************************************************************************
  Name          :  srma_hdl_cbk_dispatch
 
  Description   :  This function updates the pending data to the application
                   through registered callback function.

  Arguments     :  srma - Ptr to the SRMA control block
                   srmsv_handle - application specific SRMSv handle
                   srmsv_dispatch_flags - one of the flag defined in 
                                          SaDispatchFlagsT
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR srma_hdl_cbk_dispatch(SRMA_CB *srma,
                                    NCS_SRMSV_HANDLE srmsv_handle,
                                    SaDispatchFlagsT dispatch_flags)
{
   SRMA_USR_APPL_NODE *appl;
   uns32 rc = NCSCC_RC_SUCCESS;
  
 
   /* retrieve SRMA CB */
   if (!(appl = (SRMA_USR_APPL_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                                     srmsv_handle)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }
   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);
   
   /* What kind dispatch requested for?? */
   switch (dispatch_flags)
   {
   case SA_DISPATCH_ONE:
       /* Dispatch one pending record */
       srma_hdl_cbk_dispatch_one(srma, appl);       

       /* return SRMA CB */
       ncshm_give_hdl(srmsv_handle);
       break;

   case SA_DISPATCH_ALL:
       /* Dispatch all pending records */
       srma_hdl_cbk_dispatch_all(srma, appl, FALSE);       

       /* return SRMA CB */
       ncshm_give_hdl(srmsv_handle);
       break;

   case SA_DISPATCH_BLOCKING:
       /* return SRMA CB */
       ncshm_give_hdl(srmsv_handle);

       /* Blocks forever for receiving indications from SRMND. The routine 
          returns when SRMSv finalize is executed on the handle. */
       /* "appl" pointer passed to this function is not protected by a
           "take_hdl" instead it is protected by SRMA-CB lock */
       rc = srma_hdl_cbk_dispatch_blocking(srma, appl);
       break;

   default:
       /* return SRMA CB */
       ncshm_give_hdl(srmsv_handle);

       /* Junk flag so.. */
       m_SRMSV_ASSERT(0);
   } /* switch */

   if (rc != NCSCC_RC_SUCCESS)
      return SA_AIS_ERR_LIBRARY;
    
   return SA_AIS_OK;
}


/****************************************************************************
  Name          :  srma_appl_start_mon
 
  Description   :  On getting this request, SRMSv starts the monitoring 
                   process on all the resources (that are in stop mode)
                   requested by the application.

  Arguments     :  srmsv_handle   - ptr to the SRMSv handle                   
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR srma_appl_start_mon(SRMA_CB *srma, NCS_SRMSV_HANDLE srmsv_handle)
{
   NCS_SRMSV_ERR  rc = SA_AIS_OK;
   SRMA_USR_APPL_NODE *appl;
   SRMA_SRMND_USR_NODE *usr_srmnd = SRMA_SRMND_USR_NODE_NULL;
   SRMA_SRMND_INFO *srmnd_info = SRMA_SRMND_INFO_NULL;
   
   /* retrieve application specific SRMSv record */
   if (!(appl = (SRMA_USR_APPL_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                                     srmsv_handle)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }

   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* Check whether it is in STOP mode or not */
   if (appl->stop_monitoring != TRUE)
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_ALREADY_START_MON,
                      SRMA_LOG_MISC_NOTHING,
                      NCSFL_SEV_ERROR);

      /* return application specific SRMSv handle */
      ncshm_give_hdl(srmsv_handle);

      return  SA_AIS_ERR_BAD_OPERATION;
   }

   if (appl->bcast_rsrcs)
   {
      /* Send unregister event to all SRMNDs */
      if (srma_send_appl_msg(srma,
                             SRMA_START_MON_MSG,
                             appl->user_hdl,
                             NULL) != NCSCC_RC_SUCCESS)
      {
         /* return application specific SRMSv handle */
         ncshm_give_hdl(srmsv_handle);
         return SA_AIS_ERR_LIBRARY;
      }
   }
   else
   {
      usr_srmnd = appl->start_srmnd_node;

      /* Traverse through all subscribed SRMNDs and notify them about
         START monitoring the application resources */
      while (usr_srmnd)
      {
         srmnd_info = usr_srmnd->srmnd_info;

         /* Even if the destination SRMND is not UP.. go ahead in setting
            up the respective flag at SRMA */
         if (srmnd_info)
         {
            if (srmnd_info->is_srmnd_up)
            {
               if (srma_send_appl_msg(srma,
                                      SRMA_START_MON_MSG,
                                      appl->user_hdl,
                                      srmnd_info) != NCSCC_RC_SUCCESS)
               {
                  /* return application specific SRMSv handle */
                  ncshm_give_hdl(srmsv_handle);
                  return SA_AIS_ERR_LIBRARY;
               }
            }
         }

         usr_srmnd = usr_srmnd->next_usr_node;

      } /* End of appl while loop */
   }

   /* Reset to FALSE */
   appl->stop_monitoring = FALSE;

   /* release application specific SRMSv handle */
   ncshm_give_hdl(srmsv_handle);

   return rc;
}


/****************************************************************************
  Name          :  srma_appl_stop_mon
 
  Description   :  On getting this request, SRMSv stops the monitoring
                   process on all the resources requested by an application.

  Arguments     :  srma - ptr to the SRMA_CB structure
                   srmsv_handle - SRMSv handle                   
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR srma_appl_stop_mon(SRMA_CB *srma, NCS_SRMSV_HANDLE srmsv_handle)
{
   NCS_SRMSV_ERR  rc = SA_AIS_OK;
   SRMA_USR_APPL_NODE  *appl;
   SRMA_SRMND_USR_NODE *usr_srmnd = SRMA_SRMND_USR_NODE_NULL;
   
   /* retrieve application specific SRMSv record */
   if (!(appl = (SRMA_USR_APPL_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                                     srmsv_handle)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);

      return SA_AIS_ERR_BAD_HANDLE;
   }

   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* Check whether it is in STOP mode or not */
   if (appl->stop_monitoring == TRUE)
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_ALREADY_STOP_MON,
                      SRMA_LOG_MISC_NOTHING,
                      NCSFL_SEV_ERROR);

      /* return application specific SRMSv handle */
      ncshm_give_hdl(srmsv_handle);
      return  SA_AIS_ERR_BAD_OPERATION;
   }

   if (appl->bcast_rsrcs)
   {
      /* Send unregister event to all SRMNDs */
      if (srma_send_appl_msg(srma,
                             SRMA_STOP_MON_MSG,
                             appl->user_hdl,
                             NULL) != NCSCC_RC_SUCCESS)
      {
         /* return application specific SRMSv handle */
         ncshm_give_hdl(srmsv_handle);
         return SA_AIS_ERR_LIBRARY;
      }
   }
   else
   {
      usr_srmnd = appl->start_srmnd_node;

      /* Traverse through all subscribed SRMNDs and notify them about
         STOP monitoring the application resources */
      while (usr_srmnd)
      {
         /* Even if the destination SRMND is not UP.. go ahead in setting
            up the respective flag at SRMA */
         if (usr_srmnd->srmnd_info->is_srmnd_up)
         {
            if (srma_send_appl_msg(srma,
                                   SRMA_STOP_MON_MSG,
                                   appl->user_hdl,
                                   usr_srmnd->srmnd_info) != NCSCC_RC_SUCCESS)
            {
               /* return application specific SRMSv handle */
               ncshm_give_hdl(srmsv_handle);
               return SA_AIS_ERR_LIBRARY;
            }
         }

         usr_srmnd = usr_srmnd->next_usr_node;

      } /* End of appl while loop */
   }

   /* Reset to FALSE */
   appl->stop_monitoring = TRUE;

   /* acquire cb write lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   /* Delete the cbk data of an application */
   srma_hdl_cbk_dispatch_all(srma, appl, FALSE);

   /* release cb write lock */
   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

   /* release application specific SRMSv handle */
   ncshm_give_hdl(srmsv_handle);

   return rc;
}


/****************************************************************************
  Name          :  srma_appl_create_rsrc_mon
 
  Description   :  This request configures the application requested resource
                   monitor data with SRMSv. On getting this request, SRMSv 
                   starts monitoring process on the specified resources.

  Arguments     :  srma - ptr to the SRMA CB structure
                   srmsv_handle - SRMSv handle
                   mon_info     - Requested Resource Monitor Data
                   rsrc_mon_hdl - returns the handle of the created resource
                                  monitor record
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR srma_appl_create_rsrc_mon(SRMA_CB *srma, 
                                        uns32 srmsv_handle,
                                        NCS_SRMSV_MON_INFO *mon_info,
                                        NCS_SRMSV_RSRC_HANDLE *rsrc_mon_hdl)
{
   NCS_SRMSV_ERR rc = SA_AIS_OK;
   SRMA_RSRC_MON *rsrc = SRMA_RSRC_MON_NULL;
   NCS_BOOL      bcast = FALSE;
   NODE_ID       *node_id = &mon_info->rsrc_info.node_num; 
   uns32         monitoring_interval = 0;
   SRMA_USR_APPL_NODE *appl = SRMA_USR_APPL_NODE_NULL;

   /* acquire cb write lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   /* retrieve application specific SRMSv record */
   if (!(appl = (SRMA_USR_APPL_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                                     srmsv_handle)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);
      
      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_BAD_HANDLE;
   }
   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* It should not be of subscriber type */
   if (mon_info->rsrc_info.usr_type == NCS_SRMSV_USER_SUBSCRIBER)
   {
      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);
      return SA_AIS_ERR_INVALID_PARAM;
   }

   /* Validate the Monitor info */
   if (srma_validate_mon_info(mon_info, srma->node_num) != NCSCC_RC_SUCCESS)
   {
      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_INVALID_PARAM;
   }

   /* Check whether the rsrc-mon configuration is duplicate */
   if (srma_check_duplicate_mon_info(appl, mon_info, FALSE))
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_DUP_RSRC_MON,
                      SRMA_LOG_MISC_NOTHING,
                      NCSFL_SEV_WARNING);

      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_INVALID_PARAM;
   }

   /* Alloc the memory for rsrc-mon record */
   if (!(rsrc = m_MMGR_ALLOC_SRMA_RSRC_MON))
   {      
      m_SRMA_LOG_MEM(SRMA_MEM_RSRC_INFO,
                     SRMA_MEM_ALLOC_FAILED,
                     NCSFL_SEV_CRITICAL);

      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_NO_MEMORY;
   }

   m_NCS_OS_MEMSET((char *)rsrc, 0, sizeof(SRMA_RSRC_MON));

   /* create the association with hdl-mngr */
   if (!(rsrc->rsrc_mon_hdl = ncshm_create_hdl(srma->pool_id, 
                                               NCS_SERVICE_ID_SRMA, 
                                               (NCSCONTEXT)rsrc)))
   { 
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_CREATE_RSRC_MON,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_CRITICAL); 

      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);
      
      /* Delete SRMA Control Block */
      m_MMGR_FREE_SRMA_RSRC_MON(rsrc);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_INVALID_PARAM;      
   }
   
   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_CREATE_RSRC_MON,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* add the record to the resource mon tree of SRMA */
   rsrc->rsrc_pat_node.key_info = (uns8 *)&rsrc->rsrc_mon_hdl;
   if (ncs_patricia_tree_add(&(srma->rsrc_mon_tree), &(rsrc->rsrc_pat_node))
       != NCSCC_RC_SUCCESS)
   {
      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);
      
      /* Destroy the hdl mgr for resource-mon bindings */
      ncshm_destroy_hdl(NCS_SERVICE_ID_SRMA, rsrc->rsrc_mon_hdl);

      /* Delete SRMA Control Block */
      m_MMGR_FREE_SRMA_RSRC_MON(rsrc);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_LIBRARY; 
   }

   rsrc->rsrc_info = mon_info->rsrc_info;
   rsrc->monitor_data = mon_info->monitor_data;

   /* rsrc-mon to be broadcasted?? so add it to the bcast list of SRMA */ 
   if (rsrc->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
   {
      srma_add_rsrc_mon_to_bcast_list(rsrc, appl);
      bcast = TRUE;
   }
   else
   {
      /* Add the resource monitor record to the respective SRMND list */
      if (srma_add_rsrc_mon_to_srmnd_list(srma, rsrc, appl, *node_id) != NCSCC_RC_SUCCESS)
      {
         /* release Application specific handle */
         ncshm_give_hdl(srmsv_handle);
      
         /* Delete the respective rsrc mon node from the resource-mon tree */
         ncs_patricia_tree_del(&srma->rsrc_mon_tree, &rsrc->rsrc_pat_node);

         /* Destroy the hdl mgr for resource-mon bindings */
         ncshm_destroy_hdl(NCS_SERVICE_ID_SRMA, rsrc->rsrc_mon_hdl);

         /* Delete SRMA Control Block */
         m_MMGR_FREE_SRMA_RSRC_MON(rsrc);

         /* release cb write lock */
         m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

         return SA_AIS_ERR_LIBRARY; 
      }

      /* Add the resource monitor record to the application specific list */
      if (srma_add_rsrc_mon_to_appl_list(srma, rsrc, appl) != NCSCC_RC_SUCCESS)
      {
         /* Deleting the rsrc from SRMA database */
         srma_appl_delete_rsrc_mon(srma, rsrc->rsrc_mon_hdl, FALSE);

         /* release Application specific handle */
         ncshm_give_hdl(srmsv_handle);

         /* release cb write lock */
         m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

         return SA_AIS_ERR_LIBRARY; 
      }
   }

   /* Hand over the rsrc_mon_hdl to the user application */
   *rsrc_mon_hdl = rsrc->rsrc_mon_hdl;

   monitoring_interval = mon_info->monitor_data.monitoring_interval;

   /* If monitoring interval is not equal to 0, then calculate monitory expiry
      time (Current time stamp + monitoring_interval) and update it */
   if (monitoring_interval)
   {
      time_t now;

      m_GET_TIME_STAMP(now);
      rsrc->rsrc_mon_expiry = now + monitoring_interval;

      if (srma->srma_tmr_cb == NULL)
      {
         /* Register with RP timer */
         if (srma_timer_init(srma) != NCSCC_RC_SUCCESS)
         {
            /* Deleting the rsrc from SRMA database */
            srma_appl_delete_rsrc_mon(srma, rsrc->rsrc_mon_hdl, FALSE);

            /* release Application specific handle */
            ncshm_give_hdl(srmsv_handle);
          
            /* release cb write lock */
            m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);
           
            return SA_AIS_ERR_LIBRARY; 
         }
      }

      /* Monitor the rsrc-mon record for expiry */
      srma_add_rsrc_for_monitoring(srma,
                                   rsrc,
                                   monitoring_interval,
                                   TRUE);
   }

   /* release cb write lock */
   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

   /* Send the SRMA message to the respective SRMND(s) */
   if (bcast == TRUE)
      srma_send_rsrc_msg(srma, appl, NULL, SRMA_CREATE_RSRC_MON_MSG, rsrc);
   else
   if (rsrc->srmnd_usr->srmnd_info->is_srmnd_up == TRUE) 
   {
      srma_send_rsrc_msg(srma,
                         appl,
                         &rsrc->srmnd_usr->srmnd_info->srmnd_dest,
                         SRMA_CREATE_RSRC_MON_MSG,
                         rsrc);
   }
   else
   {
      rc = SA_AIS_ERR_NOT_EXIST;
      m_SRMA_LOG_API(SRMA_LOG_API_START_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_NOT_EXIST,
                     NCSFL_SEV_WARNING);
   }
   
   /* release application specific SRMSv handle */
   ncshm_give_hdl(srmsv_handle);

   return rc;
}


/****************************************************************************
  Name          :  srma_appl_modify_rsrc_mon
 
  Description   :  This request modifies the application requested resource 
                   monitor data. On getting the request, SRMSv restarts
                   monitoring process on the requested resources based on the
                   modified data.

  Arguments     :  rsrc_mon_hdl - Handle of the created resource monitor record
                   mon_info     - Resource Monitor Data
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR srma_appl_modify_rsrc_mon(SRMA_CB *srma, 
                                        NCS_SRMSV_MON_INFO *mon_info,
                                        NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl)
{
   NCS_SRMSV_ERR rc = SA_AIS_OK;
   SRMA_RSRC_MON *rsrc = SRMA_RSRC_MON_NULL;
   NCS_BOOL      bcast = FALSE;
   NODE_ID       *node_id = &mon_info->rsrc_info.node_num;
   uns32         monitoring_interval = 0;
   
   /* acquire cb write lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   /* retrieve application specific SRMSv record */   
   rsrc = (SRMA_RSRC_MON*)ncs_patricia_tree_get(&srma->rsrc_mon_tree, (uns8*)&rsrc_mon_hdl);
   if (!(rsrc))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_CRITICAL);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_BAD_HANDLE;
   }

   /* Got the association record for the given rsrc_mon_hdl */
   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* Validate the Monitor info */
   if (srma_validate_mon_info(mon_info, srma->node_num) != NCSCC_RC_SUCCESS)
   {
      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_INVALID_PARAM;
   }

   /* Check whether the rsrc-mon configuration is duplicate */
   if (rsrc->rsrc_info.srmsv_loc != NCS_SRMSV_NODE_ALL)
      if (srma_check_duplicate_mon_info(rsrc->usr_appl->usr_appl, mon_info, FALSE))
      {
         m_SRMA_LOG_MISC(SRMA_LOG_MISC_DUP_RSRC_MON,
                         SRMA_LOG_MISC_NOTHING,
                         NCSFL_SEV_WARNING);

         /* As the same resourec monitor data is being monitored then this 
         resource monitor record will be deleted from SRMSv Database */
         srma_appl_delete_rsrc_mon(srma, rsrc_mon_hdl, FALSE);

         /* release cb write lock */
         m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

         return SA_AIS_OK;
      }

   /* Delete rsrc from the timer List if it associates */
   if (rsrc->tmr_id)
      srma_del_rsrc_from_monitoring(srma, rsrc, TRUE);

   /* Was the modification data.. forcing to monitor on the other node ?? */
   if (rsrc->rsrc_info.node_num != mon_info->rsrc_info.node_num)
   {
      /* Send rsrc DEL event to the respective SRMND */
      if (rsrc->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
      {
          /* release cb write lock */
          m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

         /* Send the SRMA message to the respective SRMND(s) */
         srma_send_rsrc_msg(srma, 
                            rsrc->bcast_appl, 
                            NULL,
                            SRMA_DEL_RSRC_MON_MSG,
                            rsrc);
         
         /* acquire cb write lock */
         m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);
      }
      else
      {
         if (rsrc->srmnd_usr->srmnd_info->is_srmnd_up == TRUE)
         {
             /* release cb write lock */
             m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

            /* Send the SRMA message to the respective SRMND(s) */
            srma_send_rsrc_msg(srma, 
                               rsrc->usr_appl->usr_appl, 
                               &rsrc->srmnd_usr->srmnd_info->srmnd_dest,
                               SRMA_DEL_RSRC_MON_MSG,
                               rsrc);

            /* acquire cb write lock */
            m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);
         }
      }

      /* Delete it from the SRMND node */
      if (rsrc->srmnd_usr)
         srma_del_rsrc_mon_from_srmnd_list(srma, rsrc);

      /* Update the resource structure with the modified data */
      rsrc->rsrc_info    = mon_info->rsrc_info;
      rsrc->monitor_data = mon_info->monitor_data;
       
      /* rsrc-mon to be broadcasted?? so add it to the bcast list of SRMA */ 
      if (rsrc->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
      {
         if (rsrc->usr_appl)
         {
            if (rsrc->usr_appl->usr_appl)
            {
               rsrc->bcast_appl = rsrc->usr_appl->usr_appl;

               srma_add_rsrc_mon_to_bcast_list(rsrc, rsrc->usr_appl->usr_appl);
               bcast = TRUE;

               /* Need to delete from appl list */
               srma_del_rsrc_mon_from_appl_list(srma, rsrc);
            } 
         }
      }
      else
      /* Add the resource monitor record to the respective SRMND list */
      if (srma_add_rsrc_mon_to_srmnd_list(srma,
                                          rsrc,
                                          rsrc->usr_appl->usr_appl,
                                          *node_id) != NCSCC_RC_SUCCESS)
      {
         /* release cb write lock */
         m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

         return SA_AIS_ERR_INVALID_PARAM;
      }
   }
   else
   {
      /* Update the resource structure with the modified data */
      rsrc->rsrc_info    = mon_info->rsrc_info;
      rsrc->monitor_data = mon_info->monitor_data;
   }

   monitoring_interval = mon_info->monitor_data.monitoring_interval;
   /* If monitoring interval is other than 0, then calculate monitory expiry
      time (Current time stamp + monitoring_interval) and update it */
   if (monitoring_interval)
   {
      time_t now;

      m_GET_TIME_STAMP(now);
      rsrc->rsrc_mon_expiry = now + monitoring_interval;

      if (srma->srma_tmr_cb == NULL)
      {
         /* Register with RP timer */
         if (srma_timer_init(srma) != NCSCC_RC_SUCCESS)
         {
            /* release cb write lock */
            m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

            return SA_AIS_ERR_INVALID_PARAM;
         }
      }

      /* Monitor the rsrc-mon record for expiry */
      srma_add_rsrc_for_monitoring(srma,
                                   rsrc,
                                   monitoring_interval,
                                   TRUE);
   }

   /* release cb write lock */
   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

   /* send rsrc MODIFY event to SRMND */
   if (bcast == TRUE)
   {
      srma_send_rsrc_msg(srma,
                         rsrc->bcast_appl,
                         NULL,
                         SRMA_MODIFY_RSRC_MON_MSG,
                         rsrc);
   }
   else
   if (rsrc->srmnd_usr)
   {
      if (rsrc->srmnd_usr->srmnd_info->is_srmnd_up == TRUE)
      {
         srma_send_rsrc_msg(srma,
                            rsrc->usr_appl->usr_appl,
                            &rsrc->srmnd_usr->srmnd_info->srmnd_dest,
                            SRMA_MODIFY_RSRC_MON_MSG,
                            rsrc);
      }
      else
         rc = SA_AIS_ERR_NOT_EXIST;
   }
   else
       rc = SA_AIS_ERR_NOT_EXIST;

   if (rc == SA_AIS_ERR_NOT_EXIST)
   { 
      m_SRMA_LOG_API(SRMA_LOG_API_START_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_NOT_EXIST,
                     NCSFL_SEV_WARNING);
   }

   return rc;
}


/****************************************************************************
  Name          :  srma_appl_subscr_rsrc_mon
 
  Description   :  Through this request application will subscribe to all the
                   SRMSv threshold events that are specific to a resource or
                   resources. On getting this request, SRMSv sends the resource
                   specific threshold events to the application.

  Arguments     :  srmsv_hdl     - SRMSv handle
                   i_rsrc_info   - RSRC Specific Data to which it subscribes
                   subscr_handle - Handle of the create subscriber record
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR srma_appl_subscr_rsrc_mon(SRMA_CB *srma,
                                        uns32 srmsv_handle,
                                        NCS_SRMSV_RSRC_INFO *rsrc_info,
                                        NCS_SRMSV_SUBSCR_HANDLE *subscr_handle)
{
   NCS_SRMSV_ERR rc = SA_AIS_OK;
   SRMA_RSRC_MON *rsrc = SRMA_RSRC_MON_NULL;
   NODE_ID       *node_id = &rsrc_info->node_num;
   NCS_BOOL      bcast = FALSE;
   SRMA_USR_APPL_NODE *appl = SRMA_USR_APPL_NODE_NULL;
   NCS_SRMSV_MON_INFO mon_info;

   /* acquire cb write lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   /* retrieve application specific SRMSv record */
   if (!(appl = (SRMA_USR_APPL_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                                     srmsv_handle)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_BAD_HANDLE;
   }
   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* Always the user type is SUBSCR only */
   rsrc_info->usr_type = NCS_SRMSV_USER_SUBSCRIBER;

   /* Validate the RSRC info content */
   if (srma_validate_rsrc_info(rsrc_info, 
                               srma->node_num) != NCSCC_RC_SUCCESS)
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_VALIDATE_RSRC,
                      SRMA_LOG_MISC_FAILED,
                      NCSFL_SEV_ERROR);

      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_INVALID_PARAM;
   }
 
   m_NCS_OS_MEMSET(&mon_info, 0, sizeof(NCS_SRMSV_MON_INFO));
      
   /* Copy the content of rsrc specific info */
   mon_info.rsrc_info = *rsrc_info;

   /* Check whether the rsrc-mon configuration is duplicate */
   if (srma_check_duplicate_mon_info(appl, &mon_info, TRUE))
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_DUP_RSRC_MON,
                      SRMA_LOG_MISC_NOTHING,
                      NCSFL_SEV_WARNING);

      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_INVALID_PARAM;
   }

   if (!(rsrc = m_MMGR_ALLOC_SRMA_RSRC_MON))
   {
      m_SRMA_LOG_MEM(SRMA_MEM_RSRC_INFO,
                     SRMA_MEM_ALLOC_FAILED,
                     NCSFL_SEV_CRITICAL);

      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_NO_MEMORY;
   }

   m_NCS_OS_MEMSET((char *)rsrc, 0, sizeof(SRMA_RSRC_MON));

   /* create the association with hdl-mngr */
   if (!(rsrc->rsrc_mon_hdl = ncshm_create_hdl(srma->pool_id, 
                                               NCS_SERVICE_ID_SRMA, 
                                               (NCSCONTEXT)rsrc)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_CREATE_RSRC_MON,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_CRITICAL); 
      
      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);
      
      /* Delete SRMA Control Block */
      m_MMGR_FREE_SRMA_RSRC_MON(rsrc);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_LIBRARY;      
   }   

   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_CREATE_RSRC_MON,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* add the record to the resource mon tree of SRMA */
   rsrc->rsrc_pat_node.key_info = (uns8 *)&rsrc->rsrc_mon_hdl;
   if (ncs_patricia_tree_add(&(srma->rsrc_mon_tree), &(rsrc->rsrc_pat_node))
       != NCSCC_RC_SUCCESS)
   {
      /* release Application specific handle */
      ncshm_give_hdl(srmsv_handle);
      
      /* Destroy the hdl mgr for resource-mon bindings */
      ncshm_destroy_hdl(NCS_SERVICE_ID_SRMA, rsrc->rsrc_mon_hdl);

      /* Delete SRMA Control Block */
      m_MMGR_FREE_SRMA_RSRC_MON(rsrc);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_LIBRARY; 
   }

   rsrc->rsrc_info = mon_info.rsrc_info;

   /* rsrc-mon to be broadcasted?? so add it to the bcast list of SRMA */ 
   if (rsrc->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
   {
      srma_add_rsrc_mon_to_bcast_list(rsrc, appl);
      bcast = TRUE;
   }
   else
   {
      /* Add the resource monitor record to the respective SRMND list */
      if (srma_add_rsrc_mon_to_srmnd_list(srma,
                                       rsrc,
                                       appl,
                                       *node_id) != NCSCC_RC_SUCCESS)
      {
         /* release Application specific handle */
         ncshm_give_hdl(srmsv_handle);
      
         /* Destroy the hdl mgr for resource-mon bindings */
         ncshm_destroy_hdl(NCS_SERVICE_ID_SRMA, rsrc->rsrc_mon_hdl);

         /* Delete SRMA Control Block */
         m_MMGR_FREE_SRMA_RSRC_MON(rsrc);

         /* release cb write lock */
         m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

         return SA_AIS_ERR_LIBRARY; 
      }

      /* Add the resource monitor record to the application specific list */
      if (srma_add_rsrc_mon_to_appl_list(srma, rsrc, appl) != NCSCC_RC_SUCCESS)
      {
         /* Deleting the rsrc from SRMA database */
         srma_appl_delete_rsrc_mon(srma, rsrc->rsrc_mon_hdl, FALSE);

         /* release Application specific handle */
         ncshm_give_hdl(srmsv_handle);

         /* release cb write lock */
         m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

         return SA_AIS_ERR_LIBRARY; 
      }
  } 
   /* Hand over the rsrc_mon_hdl to the user application */
   *subscr_handle = rsrc->rsrc_mon_hdl;

   /* release cb write lock */
   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

   /* Send the SRMA message to the respective SRMND(s) */
   if (bcast == TRUE)
      srma_send_rsrc_msg(srma, appl, NULL, SRMA_CREATE_RSRC_MON_MSG, rsrc);
   else
   if (rsrc->srmnd_usr->srmnd_info->is_srmnd_up == TRUE)
   {
      srma_send_rsrc_msg(srma,
                         appl, 
                         &rsrc->srmnd_usr->srmnd_info->srmnd_dest,
                         SRMA_CREATE_RSRC_MON_MSG,
                         rsrc);
   }
   else
   {
      rc = SA_AIS_ERR_NOT_EXIST;
      m_SRMA_LOG_API(SRMA_LOG_API_SUBSCR_RSRC_MON,
                     SRMA_LOG_API_ERR_SA_NOT_EXIST,
                     NCSFL_SEV_WARNING);
   }
   
   /* release application specific SRMSv handle */
   ncshm_give_hdl(srmsv_handle);

   return rc;
}


/****************************************************************************
  Name          :  srma_get_watermark_val
 
  Description   :  This request configures the application requested resource
                   monitor data with SRMSv. On getting this request, SRMSv 
                   starts monitoring process on the specified resources.

  Arguments     :  srma - ptr to the SRMA CB structure
                   srmsv_handle - SRMSv handle
                   rsrc_info    - Requested Resource Monitor Data
                   wm_type - Type of the watermark monitor
 
  Return Values :  Can be one of the following return codes
 
  Notes         :  None.
******************************************************************************/
NCS_SRMSV_ERR srma_get_watermark_val(SRMA_CB *srma, 
                                     NCS_SRMSV_HANDLE srmsv_hdl,
                                     NCS_SRMSV_RSRC_INFO *rsrc_info,
                                     NCS_SRMSV_WATERMARK_TYPE wm_type)
{
   NCS_SRMSV_ERR  rc = SA_AIS_OK;
   SRMA_RSRC_MON  rsrc;
   SRMA_USR_APPL_NODE *appl = SRMA_USR_APPL_NODE_NULL;
   SRMA_SRMND_INFO *srmnd = NULL;

   
   /* acquire cb write lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   /* retrieve application specific SRMSv record */
   if (!(appl = (SRMA_USR_APPL_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                                     srmsv_hdl)))
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_BAD_HANDLE;
   }
   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_USER,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   /* Always the user type is SUBSCR only */
   rsrc_info->usr_type = NCS_SRMSV_USER_SUBSCRIBER;

   /* Validate the RSRC info content */
   if (srma_validate_rsrc_info(rsrc_info, 
                               srma->node_num) != NCSCC_RC_SUCCESS)
   {
      m_SRMA_LOG_MISC(SRMA_LOG_MISC_VALIDATE_RSRC,
                      SRMA_LOG_MISC_FAILED,
                      NCSFL_SEV_ERROR);

      /* release Application specific handle */
      ncshm_give_hdl(srmsv_hdl);

      /* release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      return SA_AIS_ERR_INVALID_PARAM;
   }

   m_NCS_OS_MEMSET(&rsrc, 0, sizeof(SRMA_RSRC_MON));

   /* Update the rsrc_type & watermark_monitor type */
   rsrc.rsrc_info.rsrc_type = rsrc_info->rsrc_type;
   rsrc.monitor_data.mon_cfg.watermark_type = wm_type;

   /* Update the PID if rsrc to be monitor is of process specific */
   if ((rsrc_info->rsrc_type == NCS_SRMSV_RSRC_PROC_MEM) ||
       (rsrc_info->rsrc_type == NCS_SRMSV_RSRC_PROC_CPU))
      rsrc.rsrc_info.pid = rsrc_info->pid;

   /* Send the SRMA message to the respective SRMND(s) */
   if (rsrc_info->srmsv_loc == NCS_SRMSV_NODE_ALL)
      srma_send_rsrc_msg(srma, appl, NULL, SRMA_GET_WATERMARK_MSG, &rsrc);
   else
   {
      if (rsrc_info->srmsv_loc == NCS_SRMSV_NODE_REMOTE)
         srmnd = srma_check_srmnd_exists(srma, rsrc_info->node_num);
      else
         srmnd = srma_check_srmnd_exists(srma, srma->node_num);

      if ((srmnd != NULL) && (srmnd->is_srmnd_up == TRUE))
      {
         srma_send_rsrc_msg(srma, 
                            appl,
                            &srmnd->srmnd_dest,
                            SRMA_GET_WATERMARK_MSG,
                            &rsrc);
      }
      else
         rc = SA_AIS_ERR_NOT_EXIST;
   }
   
   /* release application specific SRMSv handle */
   ncshm_give_hdl(srmsv_hdl);

   /* release cb write lock */
   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

   return rc;
}


