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

  This file contains routines to manage the pending-callback list.

..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/
#include "avnd.h"


/*** static function declarations */

static AVND_COMP_CBK *avnd_comp_cbq_rec_add (AVND_CB *, AVND_COMP *, 
                                        AVSV_AMF_CBK_INFO *, MDS_DEST *, SaTimeT);


/****************************************************************************
  Name          : avnd_evt_ava_csi_quiescing_compl
 
  Description   : This routine processes the AMF CSI Quiescing Complete 
                  message from AvA.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_ava_csi_quiescing_compl (AVND_CB *cb, AVND_EVT *evt)
{
   AVSV_AMF_API_INFO              *api_info = &evt->info.ava.msg->info.api_info;
   AVSV_AMF_CSI_QUIESCING_COMPL_PARAM *qsc = &api_info->param.csiq_compl;
   AVND_COMP           *comp = 0;
   AVND_COMP_CBK       *cbk_rec = 0;
   AVND_COMP_CSI_REC   *csi = 0;
   AVND_ERR_INFO       err_info;
   uns32               rc = NCSCC_RC_SUCCESS;

   /* 
    * Get the comp. As comp-name is derived from the AvA lib, it's 
    * non-existence is a fatal error.
    */
   comp = m_AVND_COMPDB_REC_GET(cb->compdb, qsc->comp_name_net);
   m_AVSV_ASSERT(comp);

   /* npi comps dont interact with amf */
   if ( !m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(comp) ) goto done;

   /* get the matching entry from the cbk list */
   m_AVND_COMP_CBQ_INV_GET(comp, qsc->inv, cbk_rec);
   
   /* it can be a responce from a proxied, see any proxied comp are there*/
   if (!cbk_rec && comp->pxied_list.n_nodes != 0) 
   {
      AVND_COMP_PXIED_REC *rec = 0;
         
      /* parse thru all proxied comp, look in the list of inv handle of each of them*/
      for(rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_FIRST(&comp->pxied_list); rec;
            rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_NEXT(&rec->comp_dll_node) )
      {
         m_AVND_COMP_CBQ_INV_GET(rec->pxied_comp, qsc->inv, cbk_rec);
         if(cbk_rec) break;
      }
      
      /* its a responce from proxied, update the comp ptr */
      if(cbk_rec) comp = rec->pxied_comp;
   }
     
   if (!cbk_rec) return rc;

   if (SA_AIS_OK != qsc->err)
   {
      /* process comp-failure */
      err_info.src = AVND_ERR_SRC_CBK_CSI_SET_FAILED;
      err_info.rcvr = comp->err_info.def_rec;
      rc = avnd_err_process(cb, comp, &err_info);
   }
   else
   {
      if (!m_AVSV_SA_NAME_IS_NULL(cbk_rec->cbk_info->param.csi_set.csi_desc.csiName))
      {
         csi = m_AVND_COMPDB_REC_CSI_GET(*comp,
                  cbk_rec->cbk_info->param.csi_set.csi_desc.csiName);
         m_AVSV_ASSERT(csi);
      }

      /* indicate that this csi-assignment is over */
      rc = avnd_comp_csi_assign_done(cb, comp, csi);
      if (NCSCC_RC_SUCCESS != rc) goto done;
   }

done:
   return rc;
}


/****************************************************************************
  Name          : avnd_evt_ava_resp
 
  Description   : This routine processes the AMF Response message from AvA.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_ava_resp (AVND_CB *cb, AVND_EVT *evt)
{
   AVSV_AMF_API_INFO   *api_info = &evt->info.ava.msg->info.api_info;
   AVSV_AMF_RESP_PARAM *resp = &api_info->param.resp;
   AVND_COMP           *comp = 0;
   AVND_COMP_CBK       *cbk_rec = 0;
   AVND_COMP_HC_REC    *hc_rec = 0;
   AVND_COMP_CSI_REC   *csi = 0;
   AVND_ERR_INFO       err_info;
   uns32               rc = NCSCC_RC_SUCCESS;

   /* 
    * Get the comp. As comp-name is derived from the AvA lib, it's 
    * non-existence is a fatal error.
    */
   comp = m_AVND_COMPDB_REC_GET(cb->compdb, resp->comp_name_net);
   m_AVSV_ASSERT(comp);

   /* npi comps except for proxied dont interact with amf */
   if ( !m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(comp) && 
         !m_AVND_COMP_TYPE_IS_PROXIED(comp))
      goto done;

   /* get the matching entry from the cbk list */
   m_AVND_COMP_CBQ_INV_GET(comp, resp->inv, cbk_rec);

   /* it can be a responce from a proxied, see any proxied comp are there*/
   if (!cbk_rec && comp->pxied_list.n_nodes != 0) 
   {
      AVND_COMP_PXIED_REC *rec = 0;
         
      /* parse thru all proxied comp, look in the list of inv handle of each of them*/
      for(rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_FIRST(&comp->pxied_list); rec;
            rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_NEXT(&rec->comp_dll_node) )
      {
         m_AVND_COMP_CBQ_INV_GET(rec->pxied_comp, resp->inv, cbk_rec);
         if(cbk_rec) break;
      }
      
      /* its a responce from proxied, update the comp ptr */
      if(cbk_rec) comp = rec->pxied_comp;
   }
      
   if(!cbk_rec) return rc;

   switch (cbk_rec->cbk_info->type)
   {
      case AVSV_AMF_HC:
         {
            AVND_COMP_HC_REC tmp_hc_rec;
            m_NCS_MEMSET(&tmp_hc_rec,'\0',sizeof(AVND_COMP_HC_REC));
            tmp_hc_rec.key = cbk_rec->cbk_info->param.hc.hc_key;
            tmp_hc_rec.req_hdl = cbk_rec->cbk_info->hdl; 
            hc_rec = m_AVND_COMPDB_REC_HC_GET(*comp, tmp_hc_rec);
            if (SA_AIS_OK != resp->err)
            {
               /* process comp-failure */
               if (hc_rec)
               {
                  err_info.rcvr = hc_rec->rec_rcvr;
                  err_info.src = AVND_ERR_SRC_CBK_HC_FAILED;
                  rc = avnd_err_process(cb, comp, &err_info);
               }
            }
            else
            {
               /* comp is healthy.. remove the cbk record */
               avnd_comp_cbq_rec_pop_and_del(cb, comp, cbk_rec);

               if(hc_rec)
               {
                  if(hc_rec->status == AVND_COMP_HC_STATUS_SND_TMR_EXPD)
                  {
                     /* send another hc cbk */
                     hc_rec->status = AVND_COMP_HC_STATUS_STABLE;
                     avnd_comp_hc_rec_start(cb, comp, hc_rec);
                  }
                 else 
                     hc_rec->status = AVND_COMP_HC_STATUS_STABLE;
                 }
            }
            break;
         }

      case AVSV_AMF_COMP_TERM:
         /* trigger comp-fsm & delete the record */
         if(SA_AIS_OK != resp->err)
            m_AVND_COMP_TERM_FAIL_SET(comp);

         rc = avnd_comp_clc_fsm_run(cb, comp, (SA_AIS_OK == resp->err) ? 
                                     AVND_COMP_CLC_PRES_FSM_EV_TERM_SUCC : 
                                     AVND_COMP_CLC_PRES_FSM_EV_CLEANUP);
         avnd_comp_cbq_rec_pop_and_del(cb, comp, cbk_rec);
         break;

      case AVSV_AMF_CSI_SET:

         if(m_AVND_COMP_TYPE_IS_PROXIED(comp) &&
               !m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(comp))
         {
            /* trigger comp-fsm & delete the record */
            rc = avnd_comp_clc_fsm_run(cb, comp, (SA_AIS_OK == resp->err) ? 
                                  AVND_COMP_CLC_PRES_FSM_EV_INST_SUCC : 
                                  AVND_COMP_CLC_PRES_FSM_EV_INST_FAIL);
            avnd_comp_cbq_rec_pop_and_del(cb, comp, cbk_rec);
            if (NCSCC_RC_SUCCESS != rc) goto done;
            break;
         }

         /* Validate this assignment response */

         /* get csi rec */
         if (!m_AVSV_SA_NAME_IS_NULL(cbk_rec->cbk_info->param.csi_set.csi_desc.csiName))
         {
            csi = m_AVND_COMPDB_REC_CSI_GET(*comp,
                     cbk_rec->cbk_info->param.csi_set.csi_desc.csiName);
            m_AVSV_ASSERT(csi);
         }

         /* check, if the older assignment was overriden by new one, if so trash this resp */
         if(!csi)
         {
            AVND_COMP_CSI_REC * temp_csi = NULL;
            temp_csi = m_AVND_COMPDB_REC_CSI_GET_FIRST(*comp);

            if(cbk_rec->cbk_info->param.csi_set.ha != temp_csi->si->curr_state)
            {
               
               avnd_comp_cbq_rec_pop_and_del(cb, comp, cbk_rec);
               break;
            }
         }
         else if(cbk_rec->cbk_info->param.csi_set.ha != csi->si->curr_state)
         {
            avnd_comp_cbq_rec_pop_and_del(cb, comp, cbk_rec);
            break;
         }
         else if(m_AVND_COMP_IS_ALL_CSI(comp))
         {
            avnd_comp_cbq_rec_pop_and_del(cb, comp, cbk_rec);
            break;
         }
            

         if (SA_AIS_OK != resp->err)
         {
            if (SA_AMF_HA_QUIESCED == cbk_rec->cbk_info->param.csi_set.ha)
            {
               /* Console Print to help debugging */
               if(comp->su->is_ncs == TRUE) 
               {
                   m_NCS_DBG_PRINTF( "\nAvSv: %s got failure for qsd cbk\n",
                         comp->name_net.value);
                   m_NCS_SYSLOG(NCS_LOG_ERR,"NCS_AvSv: %s got failure for qsd cbk",
                         comp->name_net.value);
               }

               /* => quiesced assignment failed.. dont treat it as a comp failure */
               rc = avnd_comp_csi_qscd_assign_fail_prc(cb, comp, csi);
               if (NCSCC_RC_SUCCESS != rc) goto done;
            }
            else
            {
               /* process comp-failure */
               err_info.src = AVND_ERR_SRC_CBK_CSI_SET_FAILED;
               err_info.rcvr = comp->err_info.def_rec;
               rc = avnd_err_process(cb, comp, &err_info);
            }
         }
         else
         {
            if (SA_AMF_HA_QUIESCING != cbk_rec->cbk_info->param.csi_set.ha)
            {
               /* indicate that this csi-assignment is over */
               rc = avnd_comp_csi_assign_done(cb, comp, csi);
               if (NCSCC_RC_SUCCESS != rc) goto done;
            }
            else
            {
              /* Just stop the callback timer, Quiescing complete will come after some time */ 
               if(m_AVND_TMR_IS_ACTIVE(cbk_rec->resp_tmr))
                  m_AVND_TMR_COMP_CBK_RESP_STOP(cb,*cbk_rec)
            }
             
         }
         break;

      case AVSV_AMF_CSI_REM:

         if(m_AVND_COMP_TYPE_IS_PROXIED(comp) &&
               !m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(comp))
         {
            /* trigger comp-fsm & delete the record */
            rc = avnd_comp_clc_fsm_run(cb, comp, (SA_AIS_OK == resp->err) ? 
                                     AVND_COMP_CLC_PRES_FSM_EV_TERM_SUCC : 
                                     AVND_COMP_CLC_PRES_FSM_EV_CLEANUP);
            avnd_comp_cbq_rec_pop_and_del(cb, comp, cbk_rec);
            break;
         }

         if (!m_AVSV_SA_NAME_IS_NULL(cbk_rec->cbk_info->param.csi_rem.csi_name_net))
         {
            csi = m_AVND_COMPDB_REC_CSI_GET(*comp,
                     cbk_rec->cbk_info->param.csi_rem.csi_name_net);
            m_AVSV_ASSERT(csi);
         }

         /* perform err prc if resp fails */
         if (SA_AIS_OK != resp->err)
         {
            err_info.src = AVND_ERR_SRC_CBK_CSI_REM_FAILED;
            err_info.rcvr = comp->err_info.def_rec;
            rc = avnd_err_process(cb, comp, &err_info);
         }

         /* indicate that this csi-removal is over */
         if (comp->csi_list.n_nodes)
         {
            rc = avnd_comp_csi_remove_done(cb, comp, csi);
            if (NCSCC_RC_SUCCESS != rc) goto done;
         }
         break;

      case AVSV_AMF_PXIED_COMP_INST:
         /* trigger comp-fsm & delete the record */
         rc = avnd_comp_clc_fsm_run(cb, comp, (SA_AIS_OK == resp->err) ? 
                                     AVND_COMP_CLC_PRES_FSM_EV_INST_SUCC : 
                                     AVND_COMP_CLC_PRES_FSM_EV_INST_FAIL);
         avnd_comp_cbq_rec_pop_and_del(cb, comp, cbk_rec);
         break;

      case AVSV_AMF_PXIED_COMP_CLEAN:
         /* trigger comp-fsm & delete the record */
         if (SA_AIS_OK != resp->err)
            avnd_gen_comp_clean_failed_trap(cb,comp);

         rc = avnd_comp_clc_fsm_run(cb, comp, (SA_AIS_OK == resp->err) ? 
                                     AVND_COMP_CLC_PRES_FSM_EV_CLEANUP_SUCC : 
                                     AVND_COMP_CLC_PRES_FSM_EV_CLEANUP_FAIL);
         avnd_comp_cbq_rec_pop_and_del(cb, comp, cbk_rec);
         break;

      case AVSV_AMF_PG_TRACK:
      default:
         m_AVSV_ASSERT(0);
   } /* switch */

done:
   return rc;
}


/****************************************************************************
  Name          : avnd_evt_tmr_cbk_resp
 
  Description   : This routine processes the callback response timer expiry.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_tmr_cbk_resp (AVND_CB *cb, AVND_EVT *evt)
{
   AVND_TMR_EVT      *tmr = &evt->info.tmr;
   AVND_COMP_CBK     *rec = 0;
   AVND_COMP_HC_REC  *hc_rec = 0;
   AVND_ERR_INFO     err_info;
   AVND_COMP_CSI_REC *csi = 0;
   uns32             rc = NCSCC_RC_SUCCESS;

   /* retrieve callback record */
   rec = (AVND_COMP_CBK *)ncshm_take_hdl(NCS_SERVICE_ID_AVND, tmr->opq_hdl);
   if (!rec) goto done;

   /* 
    * the record may be deleted as a part of the expiry processing. 
    * hence returning the record to the hdl mngr.
    */
   ncshm_give_hdl(tmr->opq_hdl);
   
   if ( (AVSV_AMF_CSI_SET == rec->cbk_info->type) &&
        (SA_AMF_HA_QUIESCED == rec->cbk_info->param.csi_set.ha) )
   {
      /* => quiesced assignment failed.. process it */
      csi = m_AVND_COMPDB_REC_CSI_GET(*(rec->comp), 
               rec->cbk_info->param.csi_set.csi_desc.csiName);

      /* Console Print to help debugging */
      if(rec->comp->su->is_ncs == TRUE) 
      {
          m_NCS_DBG_PRINTF( "\nAvSv: %s got qsd cbk timeout\n",
                rec->comp->name_net.value);
          m_NCS_SYSLOG(NCS_LOG_ERR, "NCS_AvSv: %s got qsd cbk timeout",
                rec->comp->name_net.value);
      }

      rc = avnd_comp_csi_qscd_assign_fail_prc(cb, rec->comp, csi);
   }
   else if(AVSV_AMF_PXIED_COMP_INST == rec->cbk_info->type) 
   {
      rc = avnd_comp_clc_fsm_run(cb, rec->comp, AVND_COMP_CLC_PRES_FSM_EV_INST_FAIL);
   }
   else if(AVSV_AMF_PXIED_COMP_CLEAN == rec->cbk_info->type) 
   {
      rc = avnd_comp_clc_fsm_run(cb, rec->comp, AVND_COMP_CLC_PRES_FSM_EV_CLEANUP_FAIL);
   }
   else if(AVSV_AMF_COMP_TERM == rec->cbk_info->type) 
   {
      m_AVND_COMP_TERM_FAIL_SET(rec->comp);
      rc = avnd_comp_clc_fsm_run(cb, rec->comp, AVND_COMP_CLC_PRES_FSM_EV_CLEANUP);
   }
   else
   {
      switch( rec->cbk_info->type)
      {
         case AVSV_AMF_HC :
            err_info.src = AVND_ERR_SRC_CBK_HC_TIMEOUT;
            break;
         case AVSV_AMF_CSI_SET :
            err_info.src = AVND_ERR_SRC_CBK_CSI_SET_TIMEOUT;
            break;
         case AVSV_AMF_CSI_REM :
            err_info.src = AVND_ERR_SRC_CBK_CSI_REM_TIMEOUT;
            break;
         default :
            m_AVND_LOG_INVALID_VAL_FATAL(rec->cbk_info->type);
            err_info.src = AVND_ERR_SRC_CBK_CSI_SET_TIMEOUT;
            break;
      }
      /* treat it as comp failure (determine the recommended recovery) */
      if ( AVSV_AMF_HC == rec->cbk_info->type )
      {
         AVND_COMP_HC_REC tmp_hc_rec;
         m_NCS_MEMSET(&tmp_hc_rec,'\0',sizeof(AVND_COMP_HC_REC));
         tmp_hc_rec.key = rec->cbk_info->param.hc.hc_key;
         tmp_hc_rec.req_hdl = rec->cbk_info->hdl; 
         hc_rec = m_AVND_COMPDB_REC_HC_GET(*(rec->comp), tmp_hc_rec);
         if (!hc_rec) goto done;
         err_info.rcvr = hc_rec->rec_rcvr;
      }
      else
         err_info.rcvr = rec->comp->err_info.def_rec;
      
      rc = avnd_err_process(cb, rec->comp, &err_info);
   }

done:
   return rc;
}


/****************************************************************************
  Name          : avnd_comp_cbq_send
 
  Description   : This routine sends AMF callback parameters to the component.
 
  Arguments     : cb        - ptr to the AvND control block 
                  comp      - ptr to the component
                  dest      - ptr to the mds-dest of the prc to which the msg 
                              is sent. If 0, the msg is sent to the 
                              registering process.
                  hdl       - AMF handle (0 if reg hdl)
                  cbk_info  - ptr to the callback parameter
                  timeout   - time period within which the appl should 
                              respond.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  Notes         : The calling process allocates memory for callback parameter.
                  It is used in the pending callback list & freed when the 
                  corresponding callback record is freed. However, if this
                  routine returns failure, then the calling routine frees the
                  memory.
******************************************************************************/
uns32 avnd_comp_cbq_send (AVND_CB           *cb,
                          AVND_COMP         *comp, 
                          MDS_DEST          *dest,
                          SaAmfHandleT      hdl,
                          AVSV_AMF_CBK_INFO *cbk_info,
                          SaTimeT           timeout)
{
   AVND_COMP_CBK *rec = 0;
   MDS_DEST mds_dest;
   uns32    rc = NCSCC_RC_SUCCESS;

   /* determine the mds-dest */
   mds_dest = (dest) ? *dest : comp->reg_dest;
   
   /* add & send the new record */
   if ( (0 != (rec = avnd_comp_cbq_rec_add(cb, comp, cbk_info, 
                                           &mds_dest, timeout))) )
   {
      /* assign inv & hdl values */
      rec->cbk_info->inv = rec->opq_hdl;
      rec->cbk_info->hdl = ((!hdl) ? comp->reg_hdl : hdl);
      
      /* send the request if comp is not in orphaned state.
       in case of orphaned component we will send it later when
       comp moves back to instantiated. we will free it, if the comp
       doesnt move to instantiated*/
      if(!m_AVND_COMP_PRES_STATE_IS_ORPHANED(comp))
         rc = avnd_comp_cbq_rec_send(cb, comp, rec);
   }
   else rc = NCSCC_RC_FAILURE;

   if (NCSCC_RC_SUCCESS != rc && rec)
   {
      /* pop & delete */
      uns32 found;
      
      m_AVND_COMP_CBQ_REC_POP(comp, rec, found);
      rec->cbk_info = 0;
      if(found)
      avnd_comp_cbq_rec_del(cb, comp, rec);
   }

   return rc;
}


/****************************************************************************
  Name          : avnd_comp_cbq_rec_send
 
  Description   : This routine sends AMF callback parameters to the 
                  appropriate process in the component. It allocates the AvA 
                  message & callback info. The callback parameters are 
                  copied from the callbk record & then sent to AvA.
 
  Arguments     : cb   - ptr to the AvND control block
                  comp - ptr to the component
                  rec  - ptr to the callback record
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  Notes         : None
******************************************************************************/
uns32 avnd_comp_cbq_rec_send (AVND_CB   *cb, 
                              AVND_COMP *comp, 
                              AVND_COMP_CBK  *rec)
{
   AVND_MSG msg;
   uns32    rc = NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&msg, 0, sizeof(AVND_MSG));

   /* allocate ava message */
   if ( 0 == (msg.info.ava = m_MMGR_ALLOC_AVSV_NDA_AVA_MSG) )
   {
      rc = NCSCC_RC_FAILURE;
      goto done;
   }
   m_NCS_OS_MEMSET(msg.info.ava, 0, sizeof(AVSV_NDA_AVA_MSG));

   /* populate the msg */
   msg.type = AVND_MSG_AVA;
   msg.info.ava->type = AVSV_AVND_AMF_CBK_MSG;
   rc = avsv_amf_cbk_copy(&msg.info.ava->info.cbk_info, rec->cbk_info);
   if (NCSCC_RC_SUCCESS != rc) goto done;

   /* send the message to AvA */
   rc = avnd_mds_send(cb, &msg, &rec->dest, 0);

   if (NCSCC_RC_SUCCESS == rc)
   {
      /* start the callback response timer */
      m_AVND_TMR_COMP_CBK_RESP_START(cb, *rec, rec->timeout, rc);

      msg.info.ava = 0;
   }

done:
   /* free the contents of avnd message */
   avnd_msg_content_free(cb, &msg);
   return rc;
}


/****************************************************************************
  Name          : avnd_comp_cbq_del
 
  Description   : This routine clears the pending callback list.
 
  Arguments     : cb   - ptr to the AvND control block
                  comp - ptr to the component
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_comp_cbq_del (AVND_CB *cb, AVND_COMP *comp)
{
   AVND_COMP_CBK *rec = 0;

   do
   {
      /* pop the record */
      m_AVND_COMP_CBQ_START_POP(comp, rec);
      if (!rec) break;

      /* delete the record */
      avnd_comp_cbq_rec_del(cb, comp, rec);
   } while (1);

   return;
}


/****************************************************************************
  Name          : avnd_comp_cbq_rec_pop_and_del
 
  Description   : This routine pops & deletes a record from the pending 
                  callback list.
 
  Arguments     : cb   - ptr to the AvND control block
                  comp - ptr to the component
                  rec  - ptr to the callback record that is to be popped & 
                         deleted.
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_comp_cbq_rec_pop_and_del (AVND_CB *cb, AVND_COMP *comp, AVND_COMP_CBK *rec)
{
   uns32 found;

   /* pop the record */
   m_AVND_COMP_CBQ_REC_POP(comp, rec, found);
   
   /* free the record */
   if(found)
   avnd_comp_cbq_rec_del(cb, comp, rec);
}


/****************************************************************************
  Name          : avnd_comp_cbq_rec_add
 
  Description   : This routine adds a record to the pending callback list.
 
  Arguments     : cb   - ptr to the AvND control block
                  comp - ptr to the component
                  cbk_info - ptr to the callback info
                  dest     - ptr to the mds-dest of the process to which 
                             callback params are sent
                  timeout  - timeout value
 
  Return Values : ptr to the newly added callback record
 
  Notes         : None.
******************************************************************************/
AVND_COMP_CBK *avnd_comp_cbq_rec_add (AVND_CB           *cb, 
                                      AVND_COMP         *comp, 
                                      AVSV_AMF_CBK_INFO *cbk_info,
                                      MDS_DEST          *dest,
                                      SaTimeT           timeout)
{
   AVND_COMP_CBK *rec = 0;

   if ( (0 == (rec = m_MMGR_ALLOC_AVND_COMP_CBK)) )
      goto error;

   m_NCS_OS_MEMSET(rec, 0, sizeof(AVND_COMP_CBK));
   
   /* create the association with hdl-mngr */
   if ( (0 == (rec->opq_hdl = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_AVND,
                                          (NCSCONTEXT)rec))) )
      goto error;
   
   /* assign the record params */
   rec->comp = comp;
   rec->cbk_info = cbk_info;
   rec->dest = *dest;
   rec->timeout = timeout;
   
   /* push the record to the pending callback list */
   m_AVND_COMP_CBQ_START_PUSH(comp, rec);

   return rec;

error:
   if (rec) avnd_comp_cbq_rec_del(cb, comp, rec);

   return 0;
}


/****************************************************************************
  Name          : avnd_comp_cbq_rec_del
 
  Description   : This routine clears the record in the pending callback list.
 
  Arguments     : cb   - ptr to the AvND control block
                  comp - ptr to the component
                  rec  - ptr to the callback record
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_comp_cbq_rec_del (AVND_CB *cb, AVND_COMP *comp, AVND_COMP_CBK *rec)
{
   /* remove the association with hdl-mngr */
   if (rec->opq_hdl)
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVND, rec->opq_hdl);

   /* stop the callback response timer */
   if ( m_AVND_TMR_IS_ACTIVE(rec->resp_tmr) )
      m_AVND_TMR_COMP_CBK_RESP_STOP(cb, *rec);

   /* free the callback info */
   if (rec->cbk_info)
      avsv_amf_cbk_free(rec->cbk_info);

   /* free the record */
   m_MMGR_FREE_AVND_COMP_CBK(rec);

   return;
}


/****************************************************************************
  Name          : avnd_comp_cbq_finalize
 
  Description   : This routine removes all the component callback records 
                  that share the specified AMF handle & the mds-dest. It is 
                  invoked when the application invokes saAmfFinalize for a 
                  certain handle.
 
  Arguments     : cb   - ptr to the AvND control block
                  comp - ptr the the component
                  hdl  - amf-handle
                  dest - ptr to mds-dest (of the prc that finalize)
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_comp_cbq_finalize (AVND_CB      *cb, 
                             AVND_COMP    *comp, 
                             SaAmfHandleT hdl, 
                             MDS_DEST     *dest)
{
   AVND_COMP_CBK *curr = (comp)->cbk_list, *prv = 0;

   /* scan the entire comp-cbk list & delete the matching records */
   while (curr)
   {
      if ( (curr->cbk_info->hdl == hdl) && 
           !m_NCS_OS_MEMCMP(&curr->dest, dest, sizeof(MDS_DEST)) )
      {
         if(curr->cbk_info && (curr->cbk_info->type == AVSV_AMF_COMP_TERM)
               && (!m_AVND_COMP_TYPE_IS_PROXIED(comp)))
         {
            m_AVND_COMP_TERM_FAIL_SET(comp);
            avnd_comp_clc_fsm_run(cb, comp, AVND_COMP_CLC_PRES_FSM_EV_CLEANUP);
         }

         avnd_comp_cbq_rec_pop_and_del(cb, comp, curr);
         curr = (prv) ? prv->next : comp->cbk_list;
      }
      else
      {
         prv = curr;
         curr = curr->next;
      }
   } /* while */

   return;
}


/****************************************************************************
  Name          : avnd_comp_cbq_csi_rec_del
 
  Description   : This routine removes a csi-set / csi-remove callback record
                  for the specified CSI.
 
  Arguments     : cb           - ptr to the AvND control block
                  comp         - ptr the the component
                  csi_name_net - ptr to the csi-name
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_comp_cbq_csi_rec_del (AVND_CB   *cb, 
                                AVND_COMP *comp, 
                                SaNameT   *csi_name_net)
{
   AVND_COMP_CBK *curr = comp->cbk_list, *prv = 0;
   AVSV_AMF_CBK_INFO *info = 0;
   NCS_BOOL to_del = FALSE;

   /* scan the entire comp-cbk list & delete the matching records */
   while (curr)
   {
      info = curr->cbk_info;

      if (!csi_name_net)
      {
         /* => remove all the csi-set & csi-rem cbks */
         if ( (AVSV_AMF_CSI_SET == info->type) || (AVSV_AMF_CSI_REM == info->type) )
            to_del = TRUE;
      }
      else
      {
         /* => remove only the matching csi-set & csi-rem cbk */
         if ( ( (AVSV_AMF_CSI_SET == info->type) &&
                (0 == m_CMP_NORDER_SANAMET(info->param.csi_set.csi_desc.csiName, *csi_name_net)) ) ||
              ( (AVSV_AMF_CSI_REM == info->type) &&
                (0 == m_CMP_NORDER_SANAMET(info->param.csi_rem.csi_name_net, *csi_name_net)) ) )
            to_del = TRUE;
      }

      if ( TRUE == to_del )
      {
         avnd_comp_cbq_rec_pop_and_del(cb, comp, curr);
         curr = (prv) ? prv->next : comp->cbk_list;
      }
      else
      {
         prv = curr;
         curr = curr->next;
      }
      to_del = FALSE;
   }

   return;
}

/****************************************************************************
  Name          : avnd_comp_unreg_cbk_process 

  Description   : This routine deletes
                  uses .

  Arguments     : cb   - ptr to the AvND control block
                  comp - ptr to the proxy of reg comp

  Return Values : None.

  Notes         : None.
******************************************************************************/

void avnd_comp_unreg_cbk_process(AVND_CB *cb, AVND_COMP *comp)
{

   AVND_COMP_CBK     *cbk = 0, *temp_cbk_list = 0, *head = 0;
   AVND_COMP_CSI_REC *csi = 0;
   uns32 rc = NCSCC_RC_SUCCESS, found = 0;

   while((comp->cbk_list != NULL) && (comp->cbk_list != cbk))
   {
      cbk = comp->cbk_list; 

      switch(cbk->cbk_info->type)
      {
         case AVSV_AMF_HC :
         case AVSV_AMF_COMP_TERM :
         {
            /* pop this rec */
            m_AVND_COMP_CBQ_REC_POP(comp, cbk, found);
            cbk->next = NULL; 
            /* if found == 0 , log a fatal error */

            /*  add this rec on to temp_cbk_list */
            { 
               if(head == 0)
               {
                  head = cbk;
                  temp_cbk_list = cbk;
               }
               else
               {
                  temp_cbk_list->next = cbk;
                  temp_cbk_list = cbk;
               }
            }

         }
         break;
         
         case AVSV_AMF_CSI_SET :
         {
            csi = m_AVND_COMPDB_REC_CSI_GET(*comp, 
                  cbk->cbk_info->param.csi_set.csi_desc.csiName);

            /* check, if the older assignment was overriden by new one, if so trash this resp */
            if(!csi)
            {
               AVND_COMP_CSI_REC * temp_csi = NULL;
               temp_csi = m_AVND_COMPDB_REC_CSI_GET_FIRST(*comp);

               if(cbk->cbk_info->param.csi_set.ha != temp_csi->si->curr_state)
               {
                  avnd_comp_cbq_rec_pop_and_del (cb, comp, cbk);
                  break;
               }
            }
            else if(cbk->cbk_info->param.csi_set.ha != csi->si->curr_state)
            {
               avnd_comp_cbq_rec_pop_and_del (cb, comp, cbk);
               break;
            }
            else if(m_AVND_COMP_IS_ALL_CSI(comp))
            {
               avnd_comp_cbq_rec_pop_and_del (cb, comp, cbk);
               break;
            }

            rc = avnd_comp_csi_assign_done(cb, comp, csi);
         }
         break;
      
         case AVSV_AMF_CSI_REM :
         {
            csi = m_AVND_COMPDB_REC_CSI_GET(*comp, 
                  cbk->cbk_info->param.csi_rem.csi_name_net);
            if (comp->csi_list.n_nodes)
            {
               rc = avnd_comp_csi_remove_done(cb, comp, csi);
            }
            else
            {
               avnd_comp_cbq_rec_pop_and_del (cb, comp, cbk);
            }

         }
         break;
         
         case AVSV_AMF_PG_TRACK :
         case AVSV_AMF_PXIED_COMP_INST :
         case AVSV_AMF_PXIED_COMP_CLEAN :
                     
         default : 
         {
            /* pop and delete this records*/
            avnd_comp_cbq_rec_pop_and_del (cb, comp, cbk);
         }
         break;
      }
         
   } /* while */

   /* copy the health check callback's back to cbk_list */
   comp->cbk_list = head;
}

