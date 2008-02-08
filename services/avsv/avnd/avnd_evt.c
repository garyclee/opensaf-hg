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



..............................................................................

  DESCRIPTION:

  This file contains AvND event routines. 
..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

#include "avnd.h"


/****************************************************************************
  Name          : avnd_evt_create
 
  Description   : This routine allocates & populates the AvND event structure.
 
  Arguments     : cb          - ptr to the AvND control block
                  type        - event type
                  mds_ctxt    - ptr to the mds-context (used for response 
                                match by MDS)
                  mds_dest    - ptr to the MDS dest
                  info        - ptr to evt-info
                  clc         - ptr to the CLC event
                  comp_fsm    - ptr to comp-fsm event
 
  Return Values : ptr to the AvND event
 
  Notes         : None.
******************************************************************************/
AVND_EVT *avnd_evt_create (AVND_CB           *cb, 
                           AVND_EVT_TYPE     type, 
                           MDS_SYNC_SND_CTXT *mds_ctxt, 
                           MDS_DEST          *mds_dest,
                           void              *info,
                           AVND_CLC_EVT      *clc,
                           AVND_COMP_FSM_EVT *comp_fsm)
{
   AVND_EVT *evt = 0;

   /* alloc avnd event */
   evt = m_MMGR_ALLOC_AVND_EVT;
   if (!evt)
   {
      /* log */
      goto done;
   }

   m_NCS_OS_MEMSET(evt, 0, sizeof(AVND_EVT));

   /* fill the common fields */
   evt->cb_hdl = cb->cb_hdl;
   if (mds_ctxt) evt->mds_ctxt = *mds_ctxt;
   evt->type = type;
   evt->priority = NCS_IPC_PRIORITY_NORMAL; /* default priority */

   /* fill the event specific fields */
   switch (type)
   {
   /* AvD event types */
   case AVND_EVT_AVD_NODE_UPDATE_MSG:
   case AVND_EVT_AVD_NODE_UP_MSG:
   case AVND_EVT_AVD_REG_HLT_MSG:
   case AVND_EVT_AVD_REG_SU_MSG:
   case AVND_EVT_AVD_REG_COMP_MSG:
   case AVND_EVT_AVD_INFO_SU_SI_ASSIGN_MSG:
   case AVND_EVT_AVD_PG_TRACK_ACT_RSP_MSG:
   case AVND_EVT_AVD_PG_UPD_MSG:
   case AVND_EVT_AVD_OPERATION_REQUEST_MSG:
   case AVND_EVT_AVD_HB_INFO_MSG:
   case AVND_EVT_AVD_SU_PRES_MSG:
   case AVND_EVT_AVD_VERIFY_MSG:
   case AVND_EVT_AVD_ACK_MSG:
   case AVND_EVT_AVD_NODE_ON_FOVER:
   case AVND_EVT_AVD_SHUTDOWN_APP_SU_MSG:
   case AVND_EVT_AVD_SET_LEDS_MSG:
      evt->info.avd = (AVSV_DND_MSG *)info;
      break;
      
   /* AvA event types */
   case AVND_EVT_AVA_FINALIZE:
   case AVND_EVT_AVA_COMP_REG:
   case AVND_EVT_AVA_COMP_UNREG:
   case AVND_EVT_AVA_PM_START:
   case AVND_EVT_AVA_PM_STOP:
   case AVND_EVT_AVA_HC_START:
   case AVND_EVT_AVA_HC_STOP:
   case AVND_EVT_AVA_HC_CONFIRM:
   case AVND_EVT_AVA_CSI_QUIESCING_COMPL:
   case AVND_EVT_AVA_HA_GET:
   case AVND_EVT_AVA_PG_START:
   case AVND_EVT_AVA_PG_STOP:
   case AVND_EVT_AVA_ERR_REP:
   case AVND_EVT_AVA_ERR_CLEAR:
   case AVND_EVT_AVA_RESP:
      evt->info.ava.mds_dest = *mds_dest;
      evt->info.ava.msg = (AVSV_NDA_AVA_MSG *)info;
      break;
      
   /* CLA event types */
   case AVND_EVT_CLA_FINALIZE:
   case AVND_EVT_CLA_TRACK_START:
   case AVND_EVT_CLA_TRACK_STOP:
   case AVND_EVT_CLA_NODE_GET:
   case AVND_EVT_CLA_NODE_ASYNC_GET:
      evt->info.cla.mds_dest = *mds_dest;
      evt->info.cla.msg = (AVSV_NDA_CLA_MSG *)info;
      break;
      
   /* timer event types */
   case AVND_EVT_TMR_HC:
   case AVND_EVT_TMR_CBK_RESP:
   case AVND_EVT_TMR_SND_HB:
   case AVND_EVT_TMR_RCV_MSG_RSP:
   case AVND_EVT_TMR_CLC_COMP_REG:
   case AVND_EVT_TMR_SU_ERR_ESC:
   case AVND_EVT_TMR_NODE_ERR_ESC:
   case AVND_EVT_TMR_CLC_PXIED_COMP_INST:
   case AVND_EVT_TMR_CLC_PXIED_COMP_REG:
      evt->priority = NCS_IPC_PRIORITY_HIGH; /* bump up the priority */
      evt->info.tmr.opq_hdl = *(uns32 *)info;
      break;
      
   /* mds event types */
   case AVND_EVT_MDS_AVD_UP:
   case AVND_EVT_MDS_AVD_DN:
   case AVND_EVT_MDS_AVA_DN:
   case AVND_EVT_MDS_CLA_DN:
      evt->priority = NCS_IPC_PRIORITY_HIGH; /* bump up the priority */
      evt->info.mds.mds_dest = *mds_dest;
      break;
      
   /* clc event types */
   case AVND_EVT_CLC_RESP:
      m_NCS_OS_MEMCPY(&evt->info.clc, clc, sizeof(AVND_CLC_EVT));
      break;
      
   /* internal event types */
   case AVND_EVT_COMP_PRES_FSM_EV:
      m_NCS_OS_MEMCPY(&evt->info.comp_fsm, comp_fsm, sizeof(AVND_COMP_FSM_EVT));
      break;
      
   case AVND_EVT_LAST_STEP_TERM:
      /* nothing to be copied, evt type should do. */
      break;

   default:
      m_AVSV_ASSERT(0);
   }

done:
   /* log */
   if (evt)
      m_AVND_LOG_EVT(type, AVND_LOG_EVT_CREATE, AVND_LOG_EVT_SUCCESS, NCSFL_SEV_INFO);
   else
      m_AVND_LOG_EVT(type, AVND_LOG_EVT_CREATE, AVND_LOG_EVT_FAILURE, NCSFL_SEV_CRITICAL);

   return evt;
}


/****************************************************************************
  Name          : avnd_evt_destroy
 
  Description   : This routine frees AvND event.
 
  Arguments     : evt - ptr to the AvND event
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_evt_destroy (AVND_EVT *evt)
{
   uns32 type = evt->type;

   if (!evt) return;

   switch (type)
   {
   /* AvD event types */
   case AVND_EVT_AVD_NODE_UPDATE_MSG:
   case AVND_EVT_AVD_NODE_UP_MSG:
   case AVND_EVT_AVD_REG_HLT_MSG:
   case AVND_EVT_AVD_REG_SU_MSG:
   case AVND_EVT_AVD_REG_COMP_MSG:
   case AVND_EVT_AVD_INFO_SU_SI_ASSIGN_MSG:
   case AVND_EVT_AVD_PG_TRACK_ACT_RSP_MSG:
   case AVND_EVT_AVD_PG_UPD_MSG:
   case AVND_EVT_AVD_OPERATION_REQUEST_MSG:
   case AVND_EVT_AVD_HB_INFO_MSG:
   case AVND_EVT_AVD_SU_PRES_MSG:
   case AVND_EVT_AVD_VERIFY_MSG:
   case AVND_EVT_AVD_ACK_MSG:
   case AVND_EVT_AVD_NODE_ON_FOVER:
   case AVND_EVT_AVD_SHUTDOWN_APP_SU_MSG:
   case AVND_EVT_AVD_SET_LEDS_MSG:
      if (evt->info.avd)
         avsv_dnd_msg_free(evt->info.avd);
      break;
      
   /* AvA event types */
   case AVND_EVT_AVA_FINALIZE:
   case AVND_EVT_AVA_COMP_REG:
   case AVND_EVT_AVA_COMP_UNREG:
   case AVND_EVT_AVA_PM_START:
   case AVND_EVT_AVA_PM_STOP:
   case AVND_EVT_AVA_HC_START:
   case AVND_EVT_AVA_HC_STOP:
   case AVND_EVT_AVA_HC_CONFIRM:
   case AVND_EVT_AVA_CSI_QUIESCING_COMPL:
   case AVND_EVT_AVA_HA_GET:
   case AVND_EVT_AVA_PG_START:
   case AVND_EVT_AVA_PG_STOP:
   case AVND_EVT_AVA_ERR_REP:
   case AVND_EVT_AVA_ERR_CLEAR:
   case AVND_EVT_AVA_RESP:
      if (evt->info.ava.msg)
         avsv_nda_ava_msg_free(evt->info.ava.msg);
      break;
      
   /* CLA event types */
   case AVND_EVT_CLA_FINALIZE:
   case AVND_EVT_CLA_TRACK_START:
   case AVND_EVT_CLA_TRACK_STOP:
   case AVND_EVT_CLA_NODE_GET:
   case AVND_EVT_CLA_NODE_ASYNC_GET:
      if (evt->info.cla.msg)
         avsv_nda_cla_msg_free(evt->info.cla.msg);
      break;
      
   /* timer event types */
   case AVND_EVT_TMR_HC:
   case AVND_EVT_TMR_CBK_RESP:
   case AVND_EVT_TMR_SND_HB:
   case AVND_EVT_TMR_RCV_MSG_RSP:
   case AVND_EVT_TMR_CLC_COMP_REG:
   case AVND_EVT_TMR_SU_ERR_ESC:
   case AVND_EVT_TMR_NODE_ERR_ESC:
   case AVND_EVT_TMR_CLC_PXIED_COMP_INST:
   case AVND_EVT_TMR_CLC_PXIED_COMP_REG:
      break;
      
   /* mds event types */
   case AVND_EVT_MDS_AVD_UP:
   case AVND_EVT_MDS_AVD_DN:
   case AVND_EVT_MDS_AVA_DN:
   case AVND_EVT_MDS_CLA_DN:
      break;
      
   /* clc event types */
   case AVND_EVT_CLC_RESP:
      break;
      
   /* internal event types */
   case AVND_EVT_COMP_PRES_FSM_EV:
      break;
      
   /* last step of termination */
   case AVND_EVT_LAST_STEP_TERM:
      break;

   default:
      m_AVSV_ASSERT(0);
   }

   /* free the avnd event */
   m_MMGR_FREE_AVND_EVT(evt);

   /* log */
   m_AVND_LOG_EVT(type, AVND_LOG_EVT_DESTROY, AVND_LOG_EVT_SUCCESS, NCSFL_SEV_INFO);

   return;
}


/****************************************************************************
  Name          : avnd_evt_send
 
  Description   : This routine enqueues the AvND event to the AvND mailbox.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_send (AVND_CB *cb, AVND_EVT *evt)
{
   AVND_EVT_TYPE type = evt->type;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* send the event */
   m_AVSV_MBX_SEND(cb, evt, evt->priority, rc);
   if ( NCSCC_RC_SUCCESS != rc)
   {
      m_AVND_LOG_MBX(AVSV_LOG_MBX_SEND, AVSV_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
      m_AVND_LOG_EVT(type, AVND_LOG_EVT_SEND, AVND_LOG_EVT_FAILURE, NCSFL_SEV_CRITICAL);
   }
   else
      m_AVND_LOG_EVT(type, AVND_LOG_EVT_SEND, AVND_LOG_EVT_SUCCESS, NCSFL_SEV_INFO);

   return rc;
}
