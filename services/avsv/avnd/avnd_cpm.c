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
  
  This module deals with the creation, accessing and deletion of the passive 
  monitoring records and lists on the AVND. 

..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

#include "avnd.h"


/*****************************************************************************
 *                                                                           *
 *          FUNCTIONS FOR MAINTAINING SRM_REQ LIST                           *
 *                                                                           *
 * Note: These functions should not be called from any other contexts other  *
 *       than from the context of AVND_PM_REC maintanance functions.         *
 *       This table can be used to reach the corressponding AVND_PM_REC.     *
 *       Creation / deletion of the PM_REC will automatically update the     *
 *       corresponding SRM_REQ Table.                                         *
 *                                                                           *
 *****************************************************************************/

/****************************************************************************
  Name          : avnd_srm_req_list_init
 
  Description   : This routine initializes the srm_req_list.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_srm_req_list_init(AVND_CB *cb)
{
   NCS_DB_LINK_LIST *srm_req_list = &cb->srm_req_list;
   uns32       rc = NCSCC_RC_SUCCESS;

   /* initialize the srm req dll list */
   srm_req_list->order = NCS_DBLIST_ANY_ORDER;
   srm_req_list->cmp_cookie = avsv_dblist_uns32_cmp;
   srm_req_list->free_cookie = avnd_srm_req_free;

   return rc;
}


/****************************************************************************
  Name          : avnd_srm_req_list_destroy
 
  Description   : This routine destroys the entire srm_req_list. It deletes 
                  all the records in the list.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : This dosen't destroy the records directly. It parses thru 
                  the srm_req's and get the corresponding PM_REC, del the 
                  PM_REC and as result the srm_req table entry will get
                  deleted.
******************************************************************************/
uns32 avnd_srm_req_list_destroy(AVND_CB *cb)
{
   NCS_DB_LINK_LIST *srm_req_list;
   AVND_SRM_REQ     *srm_req = 0;
   uns32        rc = NCSCC_RC_SUCCESS;

   /* get srm_req_list */
   srm_req_list = &cb->srm_req_list;

   /* traverse & delete all the SRM records and their corresponding PM_REC */
   while ( 0 != (srm_req = 
                (AVND_SRM_REQ *)m_NCS_DBLIST_FIND_FIRST(srm_req_list)) )
   {
      m_AVSV_ASSERT(srm_req->pm_rec);
      
      avnd_comp_pm_rec_del(cb, srm_req->pm_rec->comp, srm_req->pm_rec);
   }

   return rc;

}


/****************************************************************************
  Name          : avnd_srm_req_add
 
  Description   : This routine adds a request (node) to the srm_req list. If 
                  the record is already present, it is modified with the new 
                  parameters.
 
  Arguments     : cb        - ptr to the AvND control block
                  rsrc_hdl  - srm rsrc req habdle
                  pm_rec    - pointer to component PM_REC
 
  Return Values : ptr to the newly added/modified record
 
  Notes         : This will be called from the pm_rec_add function only.
******************************************************************************/
AVND_SRM_REQ *avnd_srm_req_add (AVND_CB *cb, uns32 rsrc_hdl, AVND_COMP_PM_REC *pm_rec)
{
   NCS_DB_LINK_LIST *srm_req_list = &cb->srm_req_list;
   AVND_SRM_REQ *srm_req = 0;
   uns32        rc = NCSCC_RC_SUCCESS;

   /* get the record, if any */
   srm_req = avnd_srm_req_get(cb, rsrc_hdl);
   if (!srm_req) 
   {
      /* a new record.. alloc & link it to the dll */
      srm_req = m_MMGR_ALLOC_AVND_SRM_REQ;
      if (srm_req)
      {
         m_NCS_OS_MEMSET(srm_req, 0, sizeof(AVND_SRM_REQ));

         /* update the record key */
         srm_req->rsrc_hdl = rsrc_hdl;
         srm_req->cb_dll_node.key = (uns8 *)&srm_req->rsrc_hdl;

         rc = ncs_db_link_list_add(srm_req_list, &srm_req->cb_dll_node);
         if (NCSCC_RC_SUCCESS != rc) goto done;
      }
      else 
      {
         rc = NCSCC_RC_FAILURE;
         goto done;
      }
   }

   /* update the params */
     srm_req->pm_rec = pm_rec;

done:
   if (NCSCC_RC_SUCCESS != rc)
   {
      if (srm_req)
      {
         avnd_srm_req_free(&srm_req->cb_dll_node);
         srm_req = 0;
      }
   }

   return srm_req;
}


/****************************************************************************
  Name          : avnd_srm_req_del
 
  Description   : This routine deletes (unlinks & frees) the specified record
                  (node) from the srm_req list.
 
  Arguments     : cb      - ptr to the AvND control block
                  srm_hdl - srm resource handle of the req node that is to be
                            deleted
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_srm_req_del(AVND_CB *cb, uns32 rsrc_hdl)
{
   NCS_DB_LINK_LIST  *srm_req_list = &cb->srm_req_list;
   uns32        rc = NCSCC_RC_SUCCESS;

   rc = ncs_db_link_list_del(srm_req_list, (uns8 *)&rsrc_hdl);

   return rc;
}


/****************************************************************************
  Name          : avnd_srm_req_get
 
  Description   : This routine retrives the specified record (node) from the 
                  srm_req list.
 
  Arguments     : cb      - ptr to the AvND control block
                  srm_hdl - handle of the srmsv req that is to be retrived
 
  Return Values : ptr to the specified record (if present)
 
  Notes         : None.
******************************************************************************/
AVND_SRM_REQ *avnd_srm_req_get(AVND_CB *cb, uns32 rsrc_hdl)
{
   NCS_DB_LINK_LIST  *srm_req_list = &cb->srm_req_list;

   return (AVND_SRM_REQ *)ncs_db_link_list_find(srm_req_list, 
                                               (uns8 *)&rsrc_hdl);
}


/****************************************************************************
  Name          : avnd_srm_req_free
 
  Description   : This routine free the memory alloced to the specified 
                  record in the srm_req list.
 
  Arguments     : node - ptr to the dll node
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_srm_req_free(NCS_DB_LINK_LIST_NODE *node)
{
   AVND_SRM_REQ *srm_req = (AVND_SRM_REQ *)node;

   m_MMGR_FREE_AVND_SRM_REQ(srm_req);

   return NCSCC_RC_SUCCESS;
}





/*****************************************************************************
 *                                                                           *
 *          FUNCTIONS FOR MAINTAINING COMP_PM_REC LIST                       *
 *                                                                           *
 *****************************************************************************/


/****************************************************************************
  Name          : avnd_pm_list_init
 
  Description   : This routine initializes the pm_list.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : None
 
  Notes         : None.
******************************************************************************/
void avnd_pm_list_init(AVND_COMP *comp)
{
   NCS_DB_LINK_LIST *pm_list = &comp->pm_list;

   /* initialize the pm_list dll  */
   pm_list->order = NCS_DBLIST_ANY_ORDER;
   pm_list->cmp_cookie = avsv_dblist_uns64_cmp;
   pm_list->free_cookie = avnd_pm_rec_free;
}



/****************************************************************************
  Name          : avnd_pm_rec_free
 
  Description   : This routine free the memory alloced to the specified 
                  record in the pm_list.
 
  Arguments     : node - ptr to the dll node
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_pm_rec_free(NCS_DB_LINK_LIST_NODE *node)
{
   AVND_COMP_PM_REC *pm_rec = (AVND_COMP_PM_REC *)node;

   m_MMGR_FREE_AVND_COMP_PM_REC(pm_rec);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : avnd_comp_pm_rec_add
 
  Description   : This routine adds the pm_rec to the corresponding component's 
                  pm_list.
 
  Arguments     : node - ptr to the dll node
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_comp_pm_rec_add(AVND_CB *cb, AVND_COMP *comp, AVND_COMP_PM_REC *rec)
{
   AVND_SRM_REQ *srm_req = 0;
   uns32        rc = NCSCC_RC_SUCCESS;

   /*update the key */
   rec->comp_dll_node.key = (uns8 *)&rec->pid; 

   /* add rec */
   rc = ncs_db_link_list_add(&comp->pm_list, &rec->comp_dll_node);
         if (NCSCC_RC_SUCCESS != rc) return rc;

   /* add the record to the srm req list */
   srm_req = avnd_srm_req_add(cb, rec->rsrc_hdl, rec);
   if(!srm_req)
   {
      ncs_db_link_list_remove(&comp->pm_list,rec->comp_dll_node.key);
      return NCSCC_RC_FAILURE;
   }

   return rc;
}


/****************************************************************************
  Name          : avnd_comp_pm_rec_del
 
  Description   : This routine deletes the passive monitoring record  
                  maintained by the component.
 
  Arguments     : cb   - ptr to the AvND control block
                  comp - ptr the the component
                  rec  - ptr to healthcheck record
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_comp_pm_rec_del (AVND_CB *cb, AVND_COMP *comp, AVND_COMP_PM_REC *rec)
{
   uns32 rsrc_hdl = rec->rsrc_hdl;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* delete the PM_REC from pm_list */ 
   rc = ncs_db_link_list_del(&comp->pm_list,(uns8 *)&rec->pid);
   if(NCSCC_RC_SUCCESS != rc)
   {
      /* log this problem */
      ;
   }
   rec = NULL; /* rec is no more, dont use it */

    /* remove the corresponding element from srm_req list */
   rc = avnd_srm_req_del(cb, rsrc_hdl);
   if( NCSCC_RC_SUCCESS != rc)
      m_NCS_ASSERT(0);

   return;
}


/****************************************************************************
  Name          : avnd_comp_pm_rec_del_all
 
  Description   : This routine clears all the pm records.
 
  Arguments     : cb   - ptr to the AvND control block
                  comp - ptr to the component
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_comp_pm_rec_del_all (AVND_CB *cb, AVND_COMP *comp)
{
   AVND_COMP_PM_REC *rec = 0;

   /* scan & delete each pm_rec record */
   while ( 0 != (rec = 
                (AVND_COMP_PM_REC *)m_NCS_DBLIST_FIND_FIRST(&comp->pm_list)) )
      avnd_comp_pm_rec_del(cb, comp, rec);

   return;
}



/*****************************************************************************
 *                                                                           *
 *          FUNCTIONS FOR MANAGING PMSTART AND PM STOP EVT                   *
 *                                                                           *
 *****************************************************************************/

/****************************************************************************
  Name          : avnd_comp_pm_stop_process
 
  Description   : This routine processes the pm stop by stoping the mon. and
                  deleting the tables and records  
 
  Arguments     : cb       - ptr to the AvND control block
                  comp     - ptr the the component
                  pm_stop  - ptr to the pm stop parameters
                  sa_err   - ptr to sa_err
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  Notes         : As of Now AvSv does not validate the stop qualifiers and
                  other param. It just stops the PM for the matching PID
******************************************************************************/
uns32 avnd_comp_pm_stop_process (AVND_CB                 *cb,
                                 AVND_COMP               *comp,
                                 AVSV_AMF_PM_STOP_PARAM  *pm_stop,
                                 SaAisErrorT             *sa_err)
{
   AVND_COMP_PM_REC *rec = 0;
   SaUint64T        pid;
   uns32       rc = NCSCC_RC_SUCCESS;

   /* get the pid */
   pid = pm_stop->pid;

   /* search the existing list for the pid in the comp's PM_rec */
   rec = (AVND_COMP_PM_REC *)ncs_db_link_list_find(&comp->pm_list,(uns8 *)&pid);

   /* this rec has to be present */
   m_NCS_ASSERT(rec);

   /* stop monitoring */
   rc = avnd_srm_stop(cb, rec, sa_err);

   /* del the pm_rec and srm_req */
   avnd_comp_pm_rec_del(cb, comp, rec);

   return rc;   
}


/****************************************************************************
  Name          : avnd_comp_pm_start_process
 
  Description   : This routine checks the param and either starts a new srm req  
                  or modify an existing srm req.
 
  Arguments     : cb        - ptr to the AvND control block
                  comp      - ptr the the component
                  pm_start  - ptr to the pm start parameters
                  sa_err    - ptr to sa_err
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  Notes         : None.
******************************************************************************/
uns32 avnd_comp_pm_start_process (AVND_CB                 *cb,
                                  AVND_COMP               *comp,
                                  AVSV_AMF_PM_START_PARAM *pm_start,
                                  SaAisErrorT             *sa_err)
{
   AVND_COMP_PM_REC *rec = 0;
   SaUint64T        pid;
   uns32       rc = NCSCC_RC_SUCCESS;
   *sa_err = SA_AIS_OK;

   /* get the pid */
   pid = pm_start->pid;

   /* search the existing list for the pid in the comp's PM_rec */
   rec = (AVND_COMP_PM_REC *)ncs_db_link_list_find(&comp->pm_list,(uns8 *)&pid);
   if (rec)/* is rec present or not */
   {
      /* compare the parametrs for PM start */
      if(0 != avnd_comp_pm_rec_cmp(pm_start, rec)) 
      {
         rc = avnd_comp_pmstart_modify(cb, pm_start, rec, sa_err);
         return rc; 
      }
      else
         return rc; /* nothing to be done */
   }

   /* no previous rec found, its a fresh srm mon request*/
   rec = (AVND_COMP_PM_REC *)avnd_comp_new_rsrc_mon(cb, comp, pm_start, sa_err);
   if(!rec)
      rc = NCSCC_RC_FAILURE;
       
    return rc;   
}


/****************************************************************************
  Name          : avnd_comp_pmstart_modify
 
  Description   : This routine modifies pm record for the component and calls
                  modify the srm mon req
  Arguments     : cb        - ptr to the AvND control block
                  pm_start  - ptr to the pm start parameters
                  rec       - ptr to the PM_REC 
                  sa_err    - ptr to sa_err
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  Notes         : None.
******************************************************************************/
uns32 avnd_comp_pmstart_modify (AVND_CB                 *cb,
                                AVSV_AMF_PM_START_PARAM *pm_start,
                                AVND_COMP_PM_REC        *rec,
                                SaAisErrorT             *sa_err)
{

         uns32 rc = NCSCC_RC_SUCCESS;

         /* check whether the hdl for start & modify match */
         if(pm_start->hdl != rec->req_hdl)
         {
            *sa_err = SA_AIS_ERR_BAD_HANDLE;
            rc = NCSCC_RC_FAILURE;
            return rc;
         }

         /* fill the modified parameters */
         rec->desc_tree_depth = pm_start->desc_tree_depth;
         rec->err             = pm_start->pm_err;
         rec->rec_rcvr        = pm_start->rec_rcvr;
         
         /* start or call srmsv rsrc mon with same rsrc handle*/
         rc = avnd_srm_start(cb, rec, sa_err);

         return rc;
}


/****************************************************************************
  Name          : avnd_comp_new_rsrc_mon
 
  Description   : This routine adds a new pm record for the component, starts .
                  srm rsrc mon

  Arguments     : cb        - ptr to the AvND control block
                  comp      - ptr the the component
                  pm_start  - ptr to the pm start parameters
                  sa_err    - ptr to sa_err
 
  Return Values : ptr to the passive monitor (pm) record.
 
  Notes         : None.
******************************************************************************/
AVND_COMP_PM_REC *avnd_comp_new_rsrc_mon (AVND_CB                 *cb,
                                          AVND_COMP               *comp,
                                          AVSV_AMF_PM_START_PARAM *pm_start,
                                          SaAisErrorT             *sa_err)
{

   AVND_COMP_PM_REC *rec = 0;  
   uns32 rc = NCSCC_RC_SUCCESS;
   *sa_err = SA_AIS_OK;

   if ( (0 == (rec = m_MMGR_ALLOC_AVND_COMP_PM_REC)) )
      return rec;

   m_NCS_OS_MEMSET(rec, 0, sizeof(AVND_COMP_PM_REC));

   /* assign the pm params */
   rec->desc_tree_depth = pm_start->desc_tree_depth;
   rec->err             = pm_start->pm_err;
   rec->rec_rcvr        = pm_start->rec_rcvr;
   rec->pid             = pm_start->pid;
   rec->req_hdl         = pm_start->hdl;
   /* store the comp bk ptr */
   rec->comp = comp;

   /* start srmsv rsrc mon */
   rc = avnd_srm_start(cb, rec, sa_err);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_MMGR_FREE_AVND_COMP_PM_REC(rec);
      rec = 0;
      return rec;
   }
 
   /* add the rec to comp's PM_REC */
   rc = avnd_comp_pm_rec_add(cb, comp, rec);
   if(NCSCC_RC_SUCCESS != rc)
   {
      rc = avnd_srm_stop(cb, rec, sa_err);
      m_MMGR_FREE_AVND_COMP_PM_REC(rec);
      rec = 0;
   }

   return rec;
}


/****************************************************************************
  Name          : avnd_comp_pm_rec_cmp
 
  Description   : This routine compares the information in the pm start param  
                  the component pm_rec.
 
  Arguments     : pm_start - ptr to the  pm_start_param
                  rec      - ptr to healthcheck record
 
  Return Values : returns zero if both are same else 1 .
 
  Notes         : This routine does not check the hdl with PM Start hdl.
                  If the handle differ, the modify routine will throw error.
******************************************************************************/
NCS_BOOL avnd_comp_pm_rec_cmp(AVSV_AMF_PM_START_PARAM *pm_start, AVND_COMP_PM_REC *rec)
{


   if(pm_start->desc_tree_depth != rec->desc_tree_depth)
      return 1;
           
   if(pm_start->pm_err != rec->err)
      return 1;
   
   if(pm_start->rec_rcvr != rec->rec_rcvr)
      return 1;

   return 0;

}


/****************************************************************************
  Name          : avnd_evt_ava_pm_start
 
  Description   : This routine processes the pm start message from 
                  AvA.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_ava_pm_start (AVND_CB *cb, AVND_EVT *evt)
{
   AVSV_AMF_API_INFO       *api_info = &evt->info.ava.msg->info.api_info;
   AVSV_AMF_PM_START_PARAM *pm_start = &api_info->param.pm_start;
   AVND_COMP               *comp = 0;
   uns32                   rc = NCSCC_RC_SUCCESS;
   SaAisErrorT             amf_rc = SA_AIS_OK;

   /* validate the pm start message */
   avnd_comp_pm_param_val(cb, AVSV_AMF_PM_START,
                (uns8 *)pm_start, &comp, 0, &amf_rc);


   /* try starting the srmsv monitor */
   if (SA_AIS_OK == amf_rc)
      rc = avnd_comp_pm_start_process(cb, comp, pm_start, &amf_rc); 


   /* send the response back to AvA */
   rc = avnd_amf_resp_send(cb, AVSV_AMF_PM_START, amf_rc, 0, 
                            &api_info->dest, &evt->mds_ctxt);



   return rc;
}


/****************************************************************************
  Name          : avnd_evt_ava_pm_stop
 
  Description   : This routine processes the pm stop message from 
                  AvA.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_ava_pm_stop (AVND_CB *cb, AVND_EVT *evt)
{
   AVSV_AMF_API_INFO      *api_info = &evt->info.ava.msg->info.api_info;
   AVSV_AMF_PM_STOP_PARAM *pm_stop = &api_info->param.pm_stop;
   AVND_COMP              *comp = 0;
   AVND_COMP_PM_REC       *rec = 0;
   uns32                  rc = NCSCC_RC_SUCCESS;
   SaAisErrorT            amf_rc = SA_AIS_OK;

   /* validate the pm stop message */
   avnd_comp_pm_param_val(cb, AVSV_AMF_PM_STOP, (uns8 *)pm_stop,
                          &comp, &rec, &amf_rc);

   /* stop the passive monitoring */
   if (SA_AIS_OK == amf_rc) 
      rc  = avnd_comp_pm_stop_process(cb, comp, pm_stop, &amf_rc);


   /* send the response back to AvA */
   rc = avnd_amf_resp_send(cb, AVSV_AMF_PM_STOP, amf_rc, 0, 
                            &api_info->dest, &evt->mds_ctxt);

   
   return rc;
}


/****************************************************************************
  Name          : avnd_comp_pm_param_val
 
  Description   : This routine validates the component pm start & stop
                  parameters.
 
  Arguments     : cb       - ptr to the AvND control block
                  api_type - pm api
                  param    - ptr to the pm start/stop params
                  o_comp   - double ptr to the comp (o/p)
                  o_pm_rec - double ptr to the comp pm record (o/p)
                  o_amf_rc - double ptr to the amf-rc (o/p)
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void avnd_comp_pm_param_val(AVND_CB           *cb, 
                            AVSV_AMF_API_TYPE api_type,
                            uns8              *param, 
                            AVND_COMP         **o_comp, 
                            AVND_COMP_PM_REC  **o_pm_rec,
                            SaAisErrorT       *o_amf_rc)
{
   *o_amf_rc = SA_AIS_OK;
   NCS_DB_LINK_LIST *srm_req_list;
   AVND_SRM_REQ     *srm_req = 0;

   switch (api_type)
   {
   case AVSV_AMF_PM_START:
      {
         AVSV_AMF_PM_START_PARAM *pm_start = (AVSV_AMF_PM_START_PARAM *)param;

         /* get the comp */
         if ( 0 == (*o_comp = m_AVND_COMPDB_REC_GET(cb->compdb, 
                                                    pm_start->comp_name_net)) )
         {
            *o_amf_rc = SA_AIS_ERR_NOT_EXIST;
            return;
         }

         /* get srm_req_list */
         srm_req_list = &cb->srm_req_list;

         /* traverse & search all the SRM records and their corresponding PM_REC for duplicate pid*/
         for(srm_req = (AVND_SRM_REQ *)m_NCS_DBLIST_FIND_FIRST(srm_req_list); srm_req;
             srm_req = (AVND_SRM_REQ *)m_NCS_DBLIST_FIND_NEXT(&srm_req->cb_dll_node))
         {
            if(srm_req->pm_rec->pid == pm_start->pid &&
                m_NCS_STRCMP(srm_req->pm_rec->comp->name_net.value,
                                             pm_start->comp_name_net.value))
            {
               *o_amf_rc = SA_AIS_ERR_INVALID_PARAM;
               return;
            }
         }

         /* non-existing component should not interact with AMF */
         if(m_AVND_COMP_PRES_STATE_IS_UNINSTANTIATED(*o_comp)      ||
            m_AVND_COMP_PRES_STATE_IS_INSTANTIATIONFAILED(*o_comp) ||
            m_AVND_COMP_PRES_STATE_IS_TERMINATING(*o_comp)         ||
            m_AVND_COMP_PRES_STATE_IS_TERMINATIONFAILED(*o_comp))
         {
            *o_amf_rc = SA_AIS_ERR_TRY_AGAIN;
            return;
         }

         
      }
      break;

   case  AVSV_AMF_PM_STOP:
      {
         AVSV_AMF_PM_STOP_PARAM *pm_stop = (AVSV_AMF_PM_STOP_PARAM *)param;

         /* get the comp */
         if ( 0 == (*o_comp = m_AVND_COMPDB_REC_GET(cb->compdb, 
                                                    pm_stop->comp_name_net)) )
         {
            *o_amf_rc = SA_AIS_ERR_NOT_EXIST;
            return;
         }
         
         /* get the record from component passive monitoring list */
         *o_pm_rec = (AVND_COMP_PM_REC *)ncs_db_link_list_find(
                             &(*o_comp)->pm_list,(uns8 *)&pm_stop->pid);
                                                     
         if ( 0 == *o_pm_rec )
         {
            *o_amf_rc = SA_AIS_ERR_NOT_EXIST;
            return;
         }

         /* if Handle dosen't match with that of PM Start */
         if((*o_pm_rec)->req_hdl != pm_stop->hdl)
         {
            *o_amf_rc = SA_AIS_ERR_BAD_HANDLE;
            return;
         }
      }
      break;

   default:
      m_AVSV_ASSERT(0);
   } /* switch */

   return;
}


/****************************************************************************
  Name          : avnd_comp_pm_finalize
 
  Description   : This routine stops all pm req made by this comp
                  via this hdl and del the PM_REC and SRM_REQ.
                  if the hld is the reg hdl of the given comp, it
                  stops all PM associated with this comp.
 
  Arguments     : cb   - ptr to the AvND control block
                  comp - ptr to the component
                  hdl  - AMF handle 
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_comp_pm_finalize(AVND_CB *cb, AVND_COMP *comp, SaAmfHandleT hdl)
{
   AVND_COMP_PM_REC *rec = 0;
   SaAisErrorT      sa_err;

   while ( 0 != (rec = 
                (AVND_COMP_PM_REC *)m_NCS_DBLIST_FIND_FIRST(&comp->pm_list)) )
   {
      /* stop PM if comp's reg handle or hdl used for PM start is finalized */
      if(hdl == rec->req_hdl || hdl == comp->reg_hdl)
      {
         avnd_srm_stop(cb, rec, &sa_err);
         avnd_comp_pm_rec_del(cb, comp, rec);
      }
   }

   return;
}



/************************** END OF FILE ***************************************/
