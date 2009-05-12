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

  This file contains AvND - SRM interaction routines. 
..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

#include "avnd.h"

/****************************************************************************
  Name          : avnd_srm_reg
 
  Description   : This routine registers the AVND  with SRMSv. 
 
  Arguments     : cb - ptr to the AVND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_srm_reg (AVND_CB *cb)
{
   NCS_SRMSV_CALLBACK srm_cbk = avnd_srm_callback;
   uns32 rc = NCSCC_RC_SUCCESS;
   SaAisErrorT ncs_err = SA_AIS_OK;

   /* initialize srmsv service */
   ncs_err = ncs_srmsv_initialize(srm_cbk, (NCS_SRMSV_HANDLE *)&cb->srm_hdl);
   if ( SA_AIS_OK != ncs_err) goto done;

   /* get the srmsv selection obj. */
   ncs_err = ncs_srmsv_selection_object_get (cb->srm_hdl,&cb->srm_sel_obj);
   if ( SA_AIS_OK != ncs_err) goto done;

   m_AVND_LOG_SRM(AVSV_LOG_SRM_REG, AVSV_LOG_SRM_SUCCESS, NCSFL_SEV_INFO);

   return rc;

done:
   if ( SA_AIS_OK != ncs_err)
      rc = avnd_srm_unreg(cb);

   m_AVND_LOG_SRM(AVSV_LOG_SRM_REG, AVSV_LOG_SRM_FAILURE, NCSFL_SEV_INFO);
   return rc;
}


/****************************************************************************
  Name          : avnd_srm_unreg
 
  Description   : This routine unregisters the AVND from SRMSv. 
 
  Arguments     : cb - ptr to the AVND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/

uns32 avnd_srm_unreg (AVND_CB *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SaAisErrorT saf_err = SA_AIS_OK;

   if(!cb->srm_hdl)
   {
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   saf_err = ncs_srmsv_finalize(cb->srm_hdl);

   if( SA_AIS_OK != saf_err)
   {
      m_AVND_LOG_SRM(AVSV_LOG_SRM_FINALIZE, AVSV_LOG_SRM_FAILURE, NCSFL_SEV_WARNING);
      rc = NCSCC_RC_FAILURE;
   }
   return rc;
}


/****************************************************************************
  Name          : avnd_srm_callback
 
  Description   : This is the callback routine called by SRMSv. 
 
  Arguments     : cb - ptr to the AVND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
void avnd_srm_callback(NCS_SRMSV_RSRC_CBK_INFO *rsrc_cbk_info)
{
   AVND_CB                 *cb = 0;
   AVND_SRM_REQ            *srm_req = 0;
   AVND_COMP_PM_REC        *pm_rec = 0;
   AVND_ERR_INFO           err;
   AVND_COMP               *comp;
   uns32                   rc = NCSCC_RC_SUCCESS;

   if(!rsrc_cbk_info) goto done;

   /* retrieve avnd cb */
   if( 0 == (cb = (AVND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVND, (uns32)gl_avnd_hdl)) )
   {
      m_AVND_LOG_CB(AVSV_LOG_CB_RETRIEVE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      goto done;
   }

   m_NCS_LOCK(&cb->lock,NCS_LOCK_WRITE);

   /* search and get the srm_req info from the map table */
   srm_req = avnd_srm_req_get(cb, rsrc_cbk_info->rsrc_mon_hdl);
   if( !srm_req ) goto done;

   /* COMP_PM_REC */
   pm_rec = srm_req->pm_rec;
  
   assert(pm_rec);

   /* store the info */
   err.src = AVND_ERR_SRC_PM;
   err.rcvr = pm_rec->rec_rcvr;
   comp = pm_rec->comp;

   /* free up the rec */
   avnd_comp_pm_rec_del(cb, comp, pm_rec);
   
   /*** process the error ***/
   rc = avnd_err_process(cb, comp, &err);

   /* unlock and give the handle*/
   m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE); 
   ncshm_give_hdl(gl_avnd_hdl);

   m_AVND_LOG_SRM(AVSV_LOG_SRM_CALLBACK, AVSV_LOG_SRM_SUCCESS, NCSFL_SEV_INFO);
   return;

done:
   /* unlock and give the handle*/
   if(cb != 0)
       m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE); 
   ncshm_give_hdl(gl_avnd_hdl);
   m_AVND_LOG_SRM(AVSV_LOG_SRM_CALLBACK, AVSV_LOG_SRM_FAILURE, NCSFL_SEV_WARNING);
   return;
}


/****************************************************************************
  Name          : avnd_srm_start
 
  Description   : This routines starts the srmsv rsrc mon for the comp. 
 
  Arguments     : cb      - ptr to the AVND control block
                  rec     - ptr to COMP PM_REC
                  srm_err - ptr to error ncs_err
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_srm_start (AVND_CB *cb, AVND_COMP_PM_REC *rec, NCS_SRMSV_ERR *srm_err)
{
   NCS_SRMSV_MON_INFO  mon_info;
   time_t              mon_rate = 1; /* 1 sec, `the max value' -:srmsv */
   time_t              mon_int  = 0; /* for ever */
   uns32               rc = NCSCC_RC_SUCCESS;



   memset(&mon_info, '\0', sizeof(NCS_SRMSV_MON_INFO));
   *srm_err = SA_AIS_OK;


   mon_info.rsrc_info.usr_type                = NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR;
   mon_info.rsrc_info.srmsv_loc               = NCS_SRMSV_NODE_LOCAL;
   mon_info.rsrc_info.pid                     = rec->pid;
   mon_info.rsrc_info.rsrc_type               = NCS_SRMSV_RSRC_PROC_EXIT;
   mon_info.rsrc_info.child_level             = rec->desc_tree_depth;
   mon_info.monitor_data.monitoring_rate      = mon_rate;
   mon_info.monitor_data.monitoring_interval  = mon_int;
  /* mon_info.monitor_data.monitor_type       = NCS_SRMSV_MON_TYPE_THRESHOLD;*/
   mon_info.monitor_data.evt_severity         = NCSFL_SEV_DEBUG;
   
   *srm_err = ncs_srmsv_start_rsrc_mon(cb->srm_hdl, &mon_info, (NCS_SRMSV_RSRC_HANDLE *)&rec->rsrc_hdl);
   if(SA_AIS_OK != *srm_err)
      rc = NCSCC_RC_FAILURE;
 
  /* log */
   if(rc == NCSCC_RC_SUCCESS)
   m_AVND_LOG_SRM(AVSV_LOG_SRM_MON_START, AVSV_LOG_SRM_SUCCESS, NCSFL_SEV_INFO);
   else
   m_AVND_LOG_SRM(AVSV_LOG_SRM_MON_START, AVSV_LOG_SRM_FAILURE, NCSFL_SEV_WARNING);

   return rc;
}


/****************************************************************************
  Name          : avnd_srm_stop
 
  Description   : This routines stops the srmsv rsrc mon for the comp. 
 
  Arguments     : cb      - ptr to the AVND control block
                  rec     - ptr to the COMP PM_REC
                  srm_err - ptr to srm_err
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_srm_stop (AVND_CB *cb, AVND_COMP_PM_REC *rec, NCS_SRMSV_ERR *srm_err)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   *srm_err = ncs_srmsv_stop_rsrc_mon(rec->rsrc_hdl);
   if( SA_AIS_OK != *srm_err)
      rc = NCSCC_RC_FAILURE;

   if(rc == NCSCC_RC_SUCCESS)
   m_AVND_LOG_SRM(AVSV_LOG_SRM_MON_STOP, AVSV_LOG_SRM_SUCCESS, NCSFL_SEV_INFO);
   else
   m_AVND_LOG_SRM(AVSV_LOG_SRM_MON_STOP, AVSV_LOG_SRM_FAILURE, NCSFL_SEV_WARNING);
   return rc;
}


/****************************************************************************
  Name          : avnd_srm_process
 
  Description   : This routine processes the SRMSv event.
 
  Arguments     : cb_hdl.
 
  Return Values : None.
 
  Notes         : It just calls the srmsv dispatch
******************************************************************************/
void avnd_srm_process(uns32 cb_hdl)
{
   NCS_SRMSV_ERR srm_error = SA_AIS_OK;
   NCS_SRMSV_HANDLE srmsv_hanle = 0;
   AVND_CB *cb;

   /* retrieve avnd cb */
   if ( 0 == (cb = (AVND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVND, cb_hdl)) )
   {
      m_AVND_LOG_CB(AVSV_LOG_CB_RETRIEVE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return;
   }

   srmsv_hanle = cb->srm_hdl;

   srm_error = ncs_srmsv_dispatch(srmsv_hanle,SA_DISPATCH_ALL);

   if( SA_AIS_OK != srm_error)
   {
      m_AVND_LOG_SRM(AVSV_LOG_SRM_DISPATCH, AVSV_LOG_SRM_FAILURE, NCSFL_SEV_WARNING);
      ncshm_give_hdl(cb_hdl);
      return;
   }
   m_AVND_LOG_SRM(AVSV_LOG_SRM_DISPATCH, AVSV_LOG_SRM_SUCCESS, NCSFL_SEV_INFO);
   ncshm_give_hdl(cb_hdl);
   return;
}
