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

  MODULE NAME: SRMND_EVT.C
 
..............................................................................

  DESCRIPTION: This file contains SRMND event specific routines

..............................................................................

  FUNCTIONS INCLUDED in this module:
  
******************************************************************************/

#include "srmnd.h"

/****************************************************************************
  Name          :  srmnd_evt_create

  Description   :  Allocate and partially populate a SRMND_EVT.

  Arguments     :  appl_hdl - User Application Handle
                    event - One of enum SRMND_EVT_TYPE, the event to send

  Return Values :  srmnd_evt: Success, a ptr to a newly created SRMND_EVT
                   NULL:     Failure

  Notes         :
******************************************************************************/
SRMND_EVT *srmnd_evt_create(uns32 cb_hdl,
                            MDS_DEST *dest,
                            NCSCONTEXT msg,
                            SRMND_EVT_TYPE evt)
{
   SRMND_EVT *srmnd_evt = NULL;

   if ((srmnd_evt = m_MMGR_ALLOC_SRMND_EVT) == NULL)
   {
      m_SRMND_LOG_MEM(SRMND_MEM_EVENT,
                      SRMND_MEM_ALLOC_FAILED,
                      NCSFL_SEV_CRITICAL);

      return NULL;
   }

   memset((char *)srmnd_evt, 0, sizeof(SRMND_EVT));

   /* Update the event type */
   srmnd_evt->evt_type = evt;

   /* dest address is non-NULL so update it */
   if (dest)
      srmnd_evt->dest = *dest;

   /* CB handle (either of SRMA_CB or SRMND_CB */
   srmnd_evt->cb_handle = cb_hdl;

   switch (evt)
   {
   case SRMND_SRMA_EVT_TYPE:
       srmnd_evt->info.srma_msg = (SRMA_MSG *)msg;
       break;

   case SRMND_AMF_EVT_TYPE: 
       srmnd_evt->info.invocation = *(SaInvocationT *)msg;
       break;

   case SRMND_SIG_EVT_TYPE: 
   case SRMND_TMR_EVT_TYPE: 
   case SRMND_SRMA_DN_EVT_TYPE:
       break;

   default:
       m_MMGR_FREE_SRMND_EVT(srmnd_evt);
       srmnd_evt = NULL;
       break;
   }

   return srmnd_evt;
}


/****************************************************************************
  Name          :  srmnd_evt_process
 
  Description   :  This routine processes the SRMND event.
 
  Arguments     :  mbx - ptr to the mailbox
 
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void srmnd_evt_process(SRMND_EVT *evt)
{
   SRMND_CB *srmnd = NULL;
   
   /* validate the event type */
   if ((evt->evt_type < SRMND_SRMA_EVT_TYPE) ||
       (evt->evt_type >= SRMND_MAX_EVT_TYPE))
   {      
      /* free the event */
      srmnd_evt_destroy(evt);
      return;
   }

   /* retrieve SRMND cb */
   if ((srmnd = (SRMND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                           evt->cb_handle)) == NULL)
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_ERROR);
      srmnd_evt_destroy(evt);
      return;
   }
      
   /* acquire cb write lock */
   m_NCS_LOCK(&srmnd->cb_lock, NCS_LOCK_WRITE);

   switch (evt->evt_type)
   {
   case SRMND_SRMA_EVT_TYPE:
       /* process the SRMA message */
       srmnd_process_srma_msg(srmnd, evt->info.srma_msg, &evt->dest);
       break;

   case SRMND_TMR_EVT_TYPE:
       m_NCS_RP_TMR_EXP(srmnd->srmnd_tmr_cb);       
       break;

   case SRMND_AMF_EVT_TYPE:
       /* send the SA_AIS_OK response to the AVA */ 
       saAmfResponse(srmnd->amf_handle, evt->info.invocation, SA_AIS_OK); 
       break;

   case SRMND_HC_START_EVT_TYPE:
       srmnd_healthcheck_start(srmnd); 
       break;

   case SRMND_SRMA_DN_EVT_TYPE:
       /* SRMA is down, ok clean up the corresponding database from SRMND */
       srmnd_del_srma(srmnd, &evt->dest);
       break;

   case SRMND_SIG_EVT_TYPE:
       srmnd_db_dump(srmnd);
       break;

   default:
       break;
   }
   
   /* release cb write lock */
   m_NCS_UNLOCK(&srmnd->cb_lock, NCS_LOCK_WRITE);

   /* return srmnd cb */
   ncshm_give_hdl(evt->cb_handle);

   /* free the event */
   srmnd_evt_destroy(evt);

   return;
}


/****************************************************************************
  Name          :  srmnd_evt_destroy
 
  Description   :  This routine frees SRMND event.
 
  Arguments     :  evt - ptr to the SRMND event
 
  Return Values :  None.
 
  Notes         :  None.
******************************************************************************/
void srmnd_evt_destroy (SRMND_EVT *evt)
{
   uns32 type = evt->evt_type;

   if (!evt) return;

   switch (type)
   {
   case SRMND_SRMA_EVT_TYPE:
       srmnd_del_srma_msg(evt->info.srma_msg);
       break;

   default:
       break;
   }

   /* Free the event now */
   m_MMGR_FREE_SRMND_EVT(evt);

   return;
}
