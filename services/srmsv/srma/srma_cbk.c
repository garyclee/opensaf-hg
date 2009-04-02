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

  MODULE NAME: SRMA_CBK.C
 
..............................................................................

  DESCRIPTION: This file contains SRMSv pending callback implementation
               supported functions

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srma.h"

extern uns32 gl_srma_hdl;
/***************************************************************************
          Static Function Prototypes 
***************************************************************************/
static void srma_process_cbk_dispatch(SRMA_CB *srma,
                                      SRMA_USR_APPL_NODE *appl,
                                      SRMA_PEND_CBK_REC  *cbk_rec,
                                      NCS_BOOL finalize);

/****************************************************************************
  Name          :  srma_update_sync_cbk_info
 
  Description   :  This routine adds the SRMND application notify message to
                   callback list of the application.

  Arguments     :  srma - ptr to the SRMA Control Block
                   srmnd_dest - MDS dest of SRMND from which the msg arrived

  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
uns32 srma_update_sync_cbk_info(SRMA_CB *srma,
                                MDS_DEST *srmnd_dest)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SRMA_USR_APPL_NODE  *appl = NULL;
   SRMA_PEND_CBK_REC   *cbk_rec;
   SRMA_SRMND_INFO     *srmnd;
   SRMA_SRMND_USR_NODE *srmnd_usr = NULL;

   /* Check whether the respective SRMND record is already created */
   if ((srmnd = srma_check_srmnd_exists(srma, m_NCS_NODE_ID_FROM_MDS_DEST(*srmnd_dest))) == NULL)
   {
      /* If not exists, just create the respective SRMND record and update
         the corresponding srmnd specific data */
      srmnd = srma_add_srmnd_node(srma, m_NCS_NODE_ID_FROM_MDS_DEST(*srmnd_dest));
      if (srmnd == NULL)
         return NCSCC_RC_FAILURE;       
   }

   /* Update the SRMND dest info */
   srmnd->srmnd_dest  = *srmnd_dest;
   srmnd->is_srmnd_up = TRUE;
   srmnd_usr = srmnd->start_usr_node;

   /* Create and add cbk info for all the subscribed application of SRMND */
   while (srmnd_usr)
   {
      appl = srmnd_usr->usr_appl;
      if (!(appl))
      {
         srmnd_usr = srmnd_usr->next_usr_node;
         continue;
      }
     
      /* Allocate the memory for callback record */
      if ((cbk_rec = m_MMGR_ALLOC_SRMA_PEND_CBK_REC) == NULL)
      {
         m_SRMA_LOG_MEM(SRMA_MEM_CBK_REC,
                        SRMA_MEM_ALLOC_FAILED,
                        NCSFL_SEV_CRITICAL);

         return NCSCC_RC_FAILURE;
      }

      memset((char *)cbk_rec, 0, sizeof(SRMA_PEND_CBK_REC));
            
      /* So this cbk info. is for SRMSv agent only, not for application */
      cbk_rec->for_srma = TRUE;
      cbk_rec->info.srmnd_dest = *srmnd_dest;

      /* Add the cbk record to cbk list of an application */
      m_SRMA_ADD_APPL_CBK_INFO(appl, cbk_rec);

      /* send an indication to the application */
      if ((rc = m_NCS_SEL_OBJ_IND(appl->sel_obj)) != NCSCC_RC_SUCCESS)
      {
         m_SRMA_LOG_SEL_OBJ(SRMA_LOG_SEL_OBJ_IND_SEND,
                            SRMA_LOG_SEL_OBJ_FAILURE,
                            NCSFL_SEV_CRITICAL);

         assert(0);
      }

      srmnd_usr = srmnd_usr->next_usr_node;
   }

   return rc;
}


/****************************************************************************
  Name          :  srma_free_cbk_rec

  Description   :  This routine searches for the cbk records having the same
                   rsrc_mon_hdl and deletes them from the cbk_list.

  Arguments     :  appl - pointer to the SRMA_USR_APPL_NODE struct
                   rsrc_mon_hdl - rsrc mon handle

  Return Values :  Nothing

  Notes         :  None
******************************************************************************/
void srma_free_cbk_rec(SRMA_RSRC_MON *rsrc_mon)
{
   SRMA_USR_APPL_NODE *appl = NULL;
   SRMA_PEND_CBK_REC  *cbk_rec;
   SRMA_PEND_CBK_REC  *tmp_cbk_rec = NULL, *prev_cbk_rec = NULL;
   uns32 rsrc_mon_hdl = rsrc_mon->rsrc_mon_hdl;

   if (rsrc_mon->rsrc_info.srmsv_loc == NCS_SRMSV_NODE_ALL)
      appl = rsrc_mon->bcast_appl;
   else
   {
      if (!(rsrc_mon->usr_appl))
         return;

      appl = rsrc_mon->usr_appl->usr_appl;
   }

   if (appl == NULL)
      return;

   cbk_rec = appl->pend_cbk.head;

   /* searches adest_list for a given adest */
   while (cbk_rec)
   {
      if ((cbk_rec->info.cbk_info.rsrc_mon_hdl == rsrc_mon_hdl) &&
          (cbk_rec->info.cbk_info.notif_type != SRMSV_CBK_NOTIF_RSRC_MON_EXPIRED))
      {
         tmp_cbk_rec = cbk_rec;
         /* First node */
         if (cbk_rec == appl->pend_cbk.head)
         {
            appl->pend_cbk.head = cbk_rec->next;
            if (appl->pend_cbk.head == NULL)
               appl->pend_cbk.tail = NULL;
         }
         else /* Last node */
         if (cbk_rec == appl->pend_cbk.tail)
         {
            prev_cbk_rec->next = NULL;
            appl->pend_cbk.tail = prev_cbk_rec;
         }
         else
            prev_cbk_rec->next = cbk_rec->next;

         cbk_rec = cbk_rec->next;

         /* remove an indication to the application */
         if (m_NCS_SEL_OBJ_RMV_IND(appl->sel_obj, TRUE, TRUE) == -1)
         {
            assert(0);
         }

         /* Mem free the cbk rec */
         m_MMGR_FREE_SRMA_PEND_CBK_REC(tmp_cbk_rec);
         tmp_cbk_rec = NULL; /* precaution */

         appl->pend_cbk.rec_num--;
      }
      else
      {
         prev_cbk_rec = cbk_rec;
         cbk_rec = cbk_rec->next;
      }
   } /* End of while loop */

   return;
}


/****************************************************************************
  Name          :  srma_update_appl_cbk_info
 
  Description   :  This routine adds the SRMND application notify message to
                   callback list of the application.

  Arguments     :  srma - ptr to the SRMA Control Block
                   cbk_info - ptr to the NCS_SRMSV_RSRC_CBK_INFO struct
                   appl - ptr to the appl rec. to which the cbk info belongs.
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
uns32 srma_update_appl_cbk_info(SRMA_CB *srma,
                                NCS_SRMSV_RSRC_CBK_INFO *cbk_info,
                                SRMA_USR_APPL_NODE *appl)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SRMA_PEND_CBK_REC  *cbk_rec;

   /* Allocate the memory for callback record */
   if ((cbk_rec = m_MMGR_ALLOC_SRMA_PEND_CBK_REC) == NULL)
   {
      m_SRMA_LOG_MEM(SRMA_MEM_CBK_REC,
                     SRMA_MEM_ALLOC_FAILED,
                     NCSFL_SEV_CRITICAL);

      return NCSCC_RC_FAILURE;
   }

   memset((char *)cbk_rec, 0, sizeof(SRMA_PEND_CBK_REC));
  
   /* Update the Callback data */
   cbk_rec->info.cbk_info = *cbk_info;

   /* Add the cbk record to cbk list of an application */
   m_SRMA_ADD_APPL_CBK_INFO(appl, cbk_rec);

   /* send an indication to the application */
   if ((rc = m_NCS_SEL_OBJ_IND(appl->sel_obj)) != NCSCC_RC_SUCCESS)
   {
      m_SRMA_LOG_SEL_OBJ(SRMA_LOG_SEL_OBJ_IND_SEND,
                         SRMA_LOG_SEL_OBJ_FAILURE,
                         NCSFL_SEV_CRITICAL);

      assert(0);
   }

   return rc;
}


/****************************************************************************
  Name          :  srma_process_cbk_dispatch
 
  Description   :  This routine does the dispatch operation on the given 
                   callback record.

  Arguments     :  srma - ptr to the SRMA Control Block
                   appl - ptr to the application specific record
                   cbk_rec - ptr to the callback record

  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srma_process_cbk_dispatch(SRMA_CB *srma,
                               SRMA_USR_APPL_NODE *appl,
                               SRMA_PEND_CBK_REC *cbk_rec,
                               NCS_BOOL finalize)
{   
   /* Call the user application cbk routine */
   if ((appl->stop_monitoring != TRUE) && (finalize != TRUE) && (appl->srmsv_call_back))
      appl->srmsv_call_back(&cbk_rec->info.cbk_info);

   /* If the resource has been expired then delete it from SRMA database */
   if ((cbk_rec->info.cbk_info.notif_type == SRMSV_CBK_NOTIF_RSRC_MON_EXPIRED) &&
       (finalize != TRUE))
   {
      /* Delete the rsrc from SRMA database */
      srma_appl_delete_rsrc_mon(srma, 
                                cbk_rec->info.cbk_info.rsrc_mon_hdl,
                                FALSE);   
   }
      
   /* Now delete the cbk_rec from the pending cbk list */
   m_SRMA_DEL_APPL_CBK_INFO(appl, cbk_rec);
   
   return;
}


/****************************************************************************
  Name          :  srma_hdl_cbk_dispatch_one
 
  Description   :  This routine dispatches one pending callback

  Arguments     :  srma - ptr to the SRMA Control Block
                   appl - ptr to the application specific record
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srma_hdl_cbk_dispatch_one(SRMA_CB *srma,
                               SRMA_USR_APPL_NODE *appl)
{
   SRMA_PEND_CBK_REC  *cbk_rec = NULL;
 
   /* Get the cbk record (head) from the pending cbk queue */
   cbk_rec = appl->pend_cbk.head;
   
   /* going to use MDS svc or going to call cbk function,
      so for the time being release cb write lock */
   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

   /* Process dispatch specific activity */
   while (cbk_rec)
   {
      /* You have cbk data.. so go ahead */
      /* remove an indication to the application */
      if (m_NCS_SEL_OBJ_RMV_IND(appl->sel_obj, TRUE, TRUE) == -1)
      {
         assert(0);
      }

      /* Is it for SRMA, not for application?? */  
      if (cbk_rec->for_srma == TRUE)
      {
         /* Oh! this dispatch is not for application :-(, it is for SRMA to 
            synchronize application specific resource mon data with SRMND */
         srma_send_srmnd_rsrc_mon_data(srma,
                                       &cbk_rec->info.srmnd_dest,
                                       appl->user_hdl);
         
         /* Now delete the cbk_rec from the pending cbk list */
         m_SRMA_DEL_APPL_CBK_INFO(appl, cbk_rec);
         cbk_rec = appl->pend_cbk.head;
         continue;
      }
      else
      {
         srma_process_cbk_dispatch(srma, appl, cbk_rec, FALSE);
         break;
      }
   }

   /* now again acquire cb write lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   return;
}


/****************************************************************************
  Name          :  srma_hdl_cbk_dispatch_all
 
  Description   :  This routine dispatches all the pending callbacks

  Arguments     :  srma - ptr to the SRMA Control Block
                   appl - ptr to the application specific record
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srma_hdl_cbk_dispatch_all(SRMA_CB *srma, 
                               SRMA_USR_APPL_NODE *appl,
                               NCS_BOOL finalize)
{
   SRMA_PEND_CBK_REC *cbk_rec = NULL;

   /* Get the cbk record (head) from the pending cbk queue */
   cbk_rec = appl->pend_cbk.head;

   /* Process dispatch specific activity on all the pending cbk records */
   while (cbk_rec)
   {
      /* You have cbk data.. so go ahead */
      /* remove an indication to the application */
      if (m_NCS_SEL_OBJ_RMV_IND(appl->sel_obj, TRUE, TRUE) == -1)
      {
         assert(0);
      }

      /* going to use MDS svc, so for the time being release cb write lock */
      m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);

      /* Is it for SRMA, not for application?? */  
      if ((cbk_rec->for_srma == TRUE) && (finalize != TRUE))
      {
         /* Oh! this dispatch is not for application :-(, it is for SRMA to 
            synchronize application specific resource mon data with SRMND */
         srma_send_srmnd_rsrc_mon_data(srma,
                                       &cbk_rec->info.srmnd_dest,
                                       appl->user_hdl);

         /* Now delete the cbk_rec from the pending cbk list */
         m_SRMA_DEL_APPL_CBK_INFO(appl, cbk_rec);
      }
      else
      {
         srma_process_cbk_dispatch(srma, appl, cbk_rec, finalize);
      }

      /* now again acquire cb write lock */
      m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

      /* Process the next call back record in the cbk list */
      cbk_rec = appl->pend_cbk.head;
   }

   return;
}


/****************************************************************************
  Name          :  srma_hdl_cbk_dispatch_blocking
 
  Description   :  This routine dispatches all the pending callbacks

  Arguments     :  srma - ptr to the SRMA Control Block
                   appl - ptr to the application specific record
                   
  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
uns32 srma_hdl_cbk_dispatch_blocking(SRMA_CB *srma, SRMA_USR_APPL_NODE *appl)
{
   NCS_SEL_OBJ_SET    wait_sel_objs;
   NCS_SEL_OBJ        srmsv_sel_obj;
   SaSelectionObjectT get_srmsv_sel_obj;
   uns32              appl_hdl = appl->user_hdl;
   SRMA_USR_APPL_NODE *usr_appl = NULL;
   uns32              rc = NCSCC_RC_SUCCESS;


   /* everything's fine.. pass the sel obj to the appl */
   get_srmsv_sel_obj = (SaSelectionObjectT)m_GET_FD_FROM_SEL_OBJ(appl->sel_obj);

   if (!(get_srmsv_sel_obj))
       return NCSCC_RC_FAILURE;

   /* reset the wait select objects */
   m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);
   m_SET_FD_IN_SEL_OBJ((uns32)get_srmsv_sel_obj, srmsv_sel_obj);
   m_NCS_SEL_OBJ_SET(srmsv_sel_obj, &wait_sel_objs);

   m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);  /* Unlock SRMA CB lock */

   /* now wait forever */
   while (m_NCS_SEL_OBJ_SELECT(srmsv_sel_obj, &wait_sel_objs, NULL, NULL, NULL) != -1)
   {      
      /* Process if SRMA selection object is set */
      if (m_NCS_SEL_OBJ_ISSET(srmsv_sel_obj, &wait_sel_objs))
      {
         /* acquire cb write lock */
         m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

         if (!(usr_appl = (SRMA_USR_APPL_NODE *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA,
                                                               appl_hdl)))
         {
            m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);  /* Unlock SRMA CB lock */
            break;
         }

         /* dispatch all the pending function records */
         srma_hdl_cbk_dispatch_one(srma, usr_appl);         
      
         ncshm_give_hdl(appl_hdl);  /* Release appl hdl */
         m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);  /* Unlock SRMA CB lock */
      }         

      /* reset the wait select objects */
      m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);

      /* again set the wait select objs */      
      m_NCS_SEL_OBJ_SET(srmsv_sel_obj, &wait_sel_objs);
   }

   /* acquire cb write lock */
   m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);

   return rc;
}


