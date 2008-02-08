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
 
  This module contains routines to create, modify, delete & fetch the SU-SI
  and component CSI relationship records.

..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

#include "avnd.h"


/* static function declarations */

static AVND_COMP_CSI_REC *avnd_su_si_csi_rec_add (AVND_CB *, AVND_SU *, 
                                                  AVND_SU_SI_REC *, 
                                                  AVND_COMP_CSI_PARAM *,
                                                  uns32 *);

static uns32 avnd_su_si_csi_rec_modify(AVND_CB *, AVND_SU *, AVND_SU_SI_REC *,
                                       AVND_COMP_CSI_PARAM *);


static uns32 avnd_su_si_csi_all_modify(AVND_CB *, AVND_SU *, 
                                       AVND_COMP_CSI_PARAM *);

static uns32 avnd_su_si_csi_rec_del (AVND_CB *, AVND_SU *, 
                                    AVND_SU_SI_REC *, AVND_COMP_CSI_REC *);

static uns32 avnd_su_si_csi_del (AVND_CB *, AVND_SU *, AVND_SU_SI_REC *);


/* macro to add a csi-record to the si-csi list */
#define m_AVND_SU_SI_CSI_REC_ADD(si, csi, rc) \
{ \
   (csi).si_dll_node.key = (uns8 *)&(csi).rank; \
   rc = ncs_db_link_list_add(&(si).csi_list, &(csi).si_dll_node); \
};

/* macro to remove a csi-record from the si-csi list */
#define m_AVND_SU_SI_CSI_REC_REM(si, csi) \
           ncs_db_link_list_delink(&(si).csi_list, &(csi).si_dll_node)


/* macro to add a susi record to the beginning of the susi queue */
#define m_AVND_SUDB_REC_SIQ_ADD(su, susi, rc) \
           (rc) = ncs_db_link_list_add(&(su).siq, &(susi).su_dll_node);


/****************************************************************************
  Name          : avnd_su_si_rec_add
 
  Description   : This routine creates an su-si relationship record. It 
                  updates the su-si record & creates the records in the 
                  comp-csi list. If an su-si relationship record already
                  exists, nothing is done.
 
  Arguments     : cb    - ptr to AvND control block
                  su    - ptr to the AvND SU
                  param - ptr to the SI parameters
                  rc    - ptr to the operation result
 
  Return Values : ptr to the su-si relationship record
 
  Notes         : None
******************************************************************************/
AVND_SU_SI_REC *avnd_su_si_rec_add (AVND_CB          *cb, 
                                    AVND_SU          *su, 
                                    AVND_SU_SI_PARAM *param,
                                    uns32            *rc)
{
   AVND_SU_SI_REC      *si_rec = 0;
   AVND_COMP_CSI_PARAM *csi_param = 0;

   *rc = NCSCC_RC_SUCCESS;

   /* verify if su-si relationship already exists */
   if ( 0 != avnd_su_si_rec_get(cb, &param->su_name_net, &param->si_name_net) )
   {
      *rc = AVND_ERR_DUP_SI;
      goto err;
   }

   /* a fresh si... */
   si_rec = m_MMGR_ALLOC_AVND_SU_SI_REC;
   if (!si_rec)
   {
      *rc = AVND_ERR_NO_MEMORY;
      goto err;
   }

   m_NCS_OS_MEMSET(si_rec, 0, sizeof(AVND_SU_SI_REC));

   /*
    * Update the supplied parameters.
    */
   /* update the si-name (key) */
   m_NCS_OS_MEMCPY(&si_rec->name_net, &param->si_name_net, sizeof(SaNameT));
   si_rec->curr_state = param->ha_state;

   /*
    * Update the rest of the parameters with default values.
    */
   m_AVND_SU_SI_CURR_ASSIGN_STATE_SET(si_rec, AVND_SU_SI_ASSIGN_STATE_UNASSIGNED);

   /*
    * Add the csi records.
    */
   /* initialize the csi-list (maintained by si) */
   si_rec->csi_list.order = NCS_DBLIST_ASSCEND_ORDER;
   si_rec->csi_list.cmp_cookie = avsv_dblist_uns32_cmp;
   si_rec->csi_list.free_cookie = 0;

   /* now add the csi records */
   csi_param = param->list;
   while ( 0 != csi_param )
   {
      avnd_su_si_csi_rec_add(cb, su, si_rec, csi_param, rc);
      if ( NCSCC_RC_SUCCESS != *rc ) goto err;

      csi_param = csi_param->next;
   }

   /*
    * Add to the si-list (maintained by su)
    */
   m_AVND_SUDB_REC_SI_ADD(*su, *si_rec, *rc);
   if ( NCSCC_RC_SUCCESS != *rc )
   {
      *rc = AVND_ERR_DLL;
      goto err;
   }

   /*
    * Update links to other entities.
    */
   si_rec->su = su;

   m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_SI_ADD, AVND_LOG_SU_DB_SUCCESS, 
                    &param->su_name_net, &param->si_name_net,
                    NCSFL_SEV_INFO);
   return si_rec;

err:
   if (si_rec)
   {
      avnd_su_si_csi_del(cb, su, si_rec);
      m_MMGR_FREE_AVND_SU_SI_REC(si_rec);
   }

   m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_SI_ADD, AVND_LOG_SU_DB_FAILURE, 
                    &param->su_name_net, &param->si_name_net,
                    NCSFL_SEV_CRITICAL);
   return 0;
}


/****************************************************************************
  Name          : avnd_su_si_csi_rec_add
 
  Description   : This routine creates a comp-csi relationship record & adds 
                  it to the 2 csi lists (maintained by si & comp).
 
  Arguments     : cb     - ptr to AvND control block
                  su     - ptr to the AvND SU
                  si_rec - ptr to the SI record
                  param  - ptr to the CSI parameters
                  rc     - ptr to the operation result
 
  Return Values : ptr to the comp-csi relationship record
 
  Notes         : None
******************************************************************************/
AVND_COMP_CSI_REC *avnd_su_si_csi_rec_add (AVND_CB             *cb, 
                                           AVND_SU             *su, 
                                           AVND_SU_SI_REC      *si_rec,
                                           AVND_COMP_CSI_PARAM *param,
                                           uns32               *rc)
{
   AVND_COMP_CSI_REC *csi_rec = 0;
   AVND_COMP         *comp = 0;

   *rc = NCSCC_RC_SUCCESS;

   /* verify if csi record already exists */
   if ( 0 != avnd_compdb_csi_rec_get(cb, &param->comp_name_net, 
                                     &param->csi_name_net) )
   {
      *rc = AVND_ERR_DUP_CSI;
      goto err;
   }

   /* get the comp */
   comp = m_AVND_COMPDB_REC_GET(cb->compdb, param->comp_name_net);
   if (!comp)
   {
      *rc = AVND_ERR_NO_COMP;
      goto err;
   }

   /* a fresh csi... */
   csi_rec = m_MMGR_ALLOC_AVND_COMP_CSI_REC;
   if (!csi_rec)
   {
      *rc = AVND_ERR_NO_MEMORY;
      goto err;
   }

   m_NCS_OS_MEMSET(csi_rec, 0, sizeof(AVND_COMP_CSI_REC));

   /*
    * Update the supplied parameters.
    */
   /* update the csi-name & csi-rank (keys to comp-csi & si-csi lists resp) */
   m_NCS_OS_MEMCPY(&csi_rec->name_net, &param->csi_name_net, sizeof(SaNameT));
   csi_rec->rank = param->csi_rank;

   /* update the assignment related parameters */
   m_NCS_OS_MEMCPY(&csi_rec->act_comp_name_net, &param->active_comp_name_net, 
                   sizeof(SaNameT));
   csi_rec->trans_desc = param->active_comp_dsc;
   csi_rec->standby_rank = param->stdby_rank;

   /* update the csi-attrs.. steal it from param */
   csi_rec->attrs.number = param->attrs.number;
   csi_rec->attrs.list = param->attrs.list;
   param->attrs.number = 0;
   param->attrs.list = 0;

   /*
    * Update the rest of the parameters with default values.
    */
   m_AVND_COMP_CSI_CURR_ASSIGN_STATE_SET(csi_rec, AVND_COMP_CSI_ASSIGN_STATE_UNASSIGNED);
   m_AVND_COMP_CSI_PRV_ASSIGN_STATE_SET(csi_rec, AVND_COMP_CSI_ASSIGN_STATE_UNASSIGNED);

   /*
    * Add to the csi-list (maintained by si).
    */
   m_AVND_SU_SI_CSI_REC_ADD(*si_rec, *csi_rec, *rc);
   if ( NCSCC_RC_SUCCESS != *rc )
   {
      *rc = AVND_ERR_DLL;
      goto err;
   }

   /*
    * Add to the csi-list (maintained by comp).
    */
   m_AVND_COMPDB_REC_CSI_ADD(*comp, *csi_rec, *rc);
   if ( NCSCC_RC_SUCCESS != *rc )
   {
      *rc = AVND_ERR_DLL;
      goto err;
   }

   /*
    * Update links to other entities.
    */
   csi_rec->si = si_rec;
   csi_rec->comp = comp;

   /* register the comp-csi row with mab */
   *rc = avnd_mab_reg_tbl_rows(cb, NCSMIB_TBL_AVSV_AMF_COMP_CSI, 
                               &csi_rec->comp->name_net, &csi_rec->name_net, 
                               0, &csi_rec->mab_hdl);
   if ( NCSCC_RC_SUCCESS != *rc ) goto err;

   m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_CSI_ADD, AVND_LOG_COMP_DB_SUCCESS, 
                      &param->comp_name_net, &param->csi_name_net, 
                      NCSFL_SEV_INFO);
   return csi_rec;

err:
   if (csi_rec)
   {
      /* remove from comp-csi & si-csi lists */
      ncs_db_link_list_delink(&si_rec->csi_list, &csi_rec->si_dll_node);
      m_AVND_COMPDB_REC_CSI_REM(*comp, *csi_rec);
      m_MMGR_FREE_AVND_COMP_CSI_REC(csi_rec);
   }

   m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_CSI_ADD, AVND_LOG_COMP_DB_FAILURE, 
                      &param->comp_name_net, &param->csi_name_net, 
                      NCSFL_SEV_CRITICAL);
   return 0;
}


/****************************************************************************
  Name          : avnd_su_si_rec_modify
 
  Description   : This routine modifies an su-si relationship record. It 
                  updates the su-si record & modifies the records in the 
                  comp-csi list.
 
  Arguments     : cb    - ptr to AvND control block
                  su    - ptr to the AvND SU
                  param - ptr to the SI parameters
                  rc    - ptr to the operation result
 
  Return Values : ptr to the modified su-si relationship record
 
  Notes         : None
******************************************************************************/
AVND_SU_SI_REC *avnd_su_si_rec_modify(AVND_CB          *cb, 
                                      AVND_SU          *su, 
                                      AVND_SU_SI_PARAM *param,
                                      uns32            *rc)
{
   AVND_SU_SI_REC *si_rec = 0;

   *rc = NCSCC_RC_SUCCESS;

   /* get the su-si relationship record */
   si_rec = avnd_su_si_rec_get(cb, &param->su_name_net, &param->si_name_net);
   if (!si_rec)
   {
      *rc = AVND_ERR_NO_SI;
      goto err;
   }

   /* store the prv state & update the new state */
   si_rec->prv_state = si_rec->curr_state;
   si_rec->curr_state = param->ha_state;

   /* store the prv assign-state & update the new assign-state */
   si_rec->prv_assign_state = si_rec->curr_assign_state;
   m_AVND_SU_SI_CURR_ASSIGN_STATE_SET(si_rec, AVND_SU_SI_ASSIGN_STATE_UNASSIGNED);

   /* now modify the csi records */
   *rc = avnd_su_si_csi_rec_modify(cb, su, si_rec,
                                  ( (SA_AMF_HA_QUIESCED == param->ha_state) || 
                                    (SA_AMF_HA_QUIESCING == param->ha_state) ) ? 
                                       0: param->list);
   if ( *rc != NCSCC_RC_SUCCESS ) goto err;

   return si_rec;

err:
   return 0;
}


/****************************************************************************
  Name          : avnd_su_si_csi_rec_modify
 
  Description   : This routine modifies a comp-csi relationship record.
 
  Arguments     : cb     - ptr to AvND control block
                  su     - ptr to the AvND SU
                  si_rec - ptr to the SI record
                  param  - ptr to the CSI parameters
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_su_si_csi_rec_modify(AVND_CB             *cb, 
                                AVND_SU             *su, 
                                AVND_SU_SI_REC      *si_rec,
                                AVND_COMP_CSI_PARAM *param)
{
   AVND_COMP_CSI_PARAM *curr_param = 0;
   AVND_COMP_CSI_REC   *curr_csi = 0;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* pick up all the csis belonging to the si & modify them */
   if (!param)
   {
      for (curr_csi = (AVND_COMP_CSI_REC *)m_NCS_DBLIST_FIND_FIRST(&si_rec->csi_list);
           curr_csi;
           curr_csi = (AVND_COMP_CSI_REC *)m_NCS_DBLIST_FIND_NEXT(&curr_csi->si_dll_node))
      {
         /* store the prv assign-state & update the new assign-state */
         curr_csi->prv_assign_state = curr_csi->curr_assign_state;
         m_AVND_COMP_CSI_CURR_ASSIGN_STATE_SET(curr_csi, 
            AVND_COMP_CSI_ASSIGN_STATE_UNASSIGNED);
      } /* for */
   }

   /* pick up the csis belonging to the comps specified in the param-list */
   for (curr_param = param; curr_param; curr_param = curr_param->next)
   {
      /* get the comp & csi */
      curr_csi = avnd_compdb_csi_rec_get(cb, &curr_param->comp_name_net, 
                                         &curr_param->csi_name_net);
      if ( !curr_csi || (curr_csi->comp->su != su) )
      {
         rc = NCSCC_RC_FAILURE;
         goto done;
      }
      
      /* update the assignment related parameters */
      curr_csi->act_comp_name_net = curr_param->active_comp_name_net;
      curr_csi->trans_desc = curr_param->active_comp_dsc;
      curr_csi->standby_rank = curr_param->stdby_rank;
         
      /* store the prv assign-state & update the new assign-state */
      curr_csi->prv_assign_state = curr_csi->curr_assign_state;
      m_AVND_COMP_CSI_CURR_ASSIGN_STATE_SET(curr_csi, 
               AVND_COMP_CSI_ASSIGN_STATE_UNASSIGNED);
   } /* for */

done:
   return rc;
}


/****************************************************************************
  Name          : avnd_su_si_all_modify
 
  Description   : This routine modifies all the SU-SI & comp-csi records in 
                  the database.
 
  Arguments     : cb    - ptr to AvND control block
                  su    - ptr to the AvND SU
                  param - ptr to the SI parameters
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_su_si_all_modify(AVND_CB *cb, AVND_SU *su, AVND_SU_SI_PARAM *param)
{
   AVND_SU_SI_REC *curr_si = 0;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* modify all the si records */
   for ( curr_si = (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_FIRST(&su->si_list);
         curr_si;
         curr_si = (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_NEXT(&curr_si->su_dll_node) )
   {
       /* store the prv state & update the new state */
       curr_si->prv_state = curr_si->curr_state;
       curr_si->curr_state = param->ha_state;
            
       /* store the prv assign-state & update the new assign-state */
       curr_si->prv_assign_state = curr_si->curr_assign_state;
       m_AVND_SU_SI_CURR_ASSIGN_STATE_SET(curr_si, AVND_SU_SI_ASSIGN_STATE_UNASSIGNED);
   } /* for */

   /* now modify the comp-csi records */
   rc = avnd_su_si_csi_all_modify(cb, su, ( (SA_AMF_HA_QUIESCED == param->ha_state) || 
                                            (SA_AMF_HA_QUIESCING == param->ha_state) ) ? 
                                            0: param->list);
   return rc;
}


/****************************************************************************
  Name          : avnd_su_si_csi_all_modify
 
  Description   : This routine modifies the csi records.
 
  Arguments     : cb     - ptr to AvND control block
                  su     - ptr to the AvND SU
                  param  - ptr to the CSI parameters (if 0, => all the CSIs 
                           belonging to all the SIs in the SU are modified. 
                           Else all the CSIs belonging to all the components 
                           in the SU are modified)
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_su_si_csi_all_modify(AVND_CB             *cb, 
                                AVND_SU             *su, 
                                AVND_COMP_CSI_PARAM *param)
{
   AVND_COMP_CSI_PARAM *curr_param = 0;
   AVND_COMP_CSI_REC   *curr_csi = 0;
   AVND_SU_SI_REC      *curr_si = 0;
   AVND_COMP           *curr_comp = 0;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* pick up all the csis belonging to all the sis & modify them */
   if (!param)
   {
      for (curr_si = (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_FIRST(&su->si_list);
           curr_si;
           curr_si = (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_NEXT(&curr_si->su_dll_node))
      {
         for (curr_csi = (AVND_COMP_CSI_REC *)m_NCS_DBLIST_FIND_FIRST(&curr_si->csi_list);
              curr_csi;
              curr_csi = (AVND_COMP_CSI_REC *)m_NCS_DBLIST_FIND_NEXT(&curr_csi->si_dll_node))
         {
            /* store the prv assign-state & update the new assign-state */
            curr_csi->prv_assign_state = curr_csi->curr_assign_state;
            m_AVND_COMP_CSI_CURR_ASSIGN_STATE_SET(curr_csi, 
                                     AVND_COMP_CSI_ASSIGN_STATE_UNASSIGNED);
         } /* for */
      } /* for */
   }

   /* pick up all the csis belonging to the comps specified in the param-list */
   for (curr_param = param; curr_param; curr_param = curr_param->next)
   {
      /* get the comp */
      curr_comp = m_AVND_COMPDB_REC_GET(cb->compdb, curr_param->comp_name_net);
      if ( !curr_comp || (curr_comp->su != su) )
      {
         rc = NCSCC_RC_FAILURE;
         goto done;
      }
      
      /* modify all the csi-records */
      for ( curr_csi = m_AVND_CSI_REC_FROM_COMP_DLL_NODE_GET(m_NCS_DBLIST_FIND_FIRST(&curr_comp->csi_list));
            curr_csi;
            curr_csi = m_AVND_CSI_REC_FROM_COMP_DLL_NODE_GET(m_NCS_DBLIST_FIND_NEXT(&curr_csi->comp_dll_node)) )
      {
         /* update the assignment related parameters */
         curr_csi->act_comp_name_net = curr_param->active_comp_name_net;
         curr_csi->trans_desc = curr_param->active_comp_dsc;
         curr_csi->standby_rank = curr_param->stdby_rank;
         
         /* store the prv assign-state & update the new assign-state */
         curr_csi->prv_assign_state = curr_csi->curr_assign_state;
         m_AVND_COMP_CSI_CURR_ASSIGN_STATE_SET(curr_csi, 
            AVND_COMP_CSI_ASSIGN_STATE_UNASSIGNED);
      } /* for */
   } /* for */

done:
   return rc;
}


/****************************************************************************
  Name          : avnd_su_si_rec_del
 
  Description   : This routine deletes a su-si relationship record. It 
                  traverses the entire csi-list and deletes each comp-csi 
                  relationship record.
 
  Arguments     : cb          - ptr to AvND control block
                  su_name_net - ptr to the su-name (n/w order)
                  si_name_net - ptr to the si-name (n/w order)
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_su_si_rec_del (AVND_CB *cb, 
                          SaNameT *su_name_net, 
                          SaNameT *si_name_net)
{
   AVND_SU        *su = 0;
   AVND_SU_SI_REC *si_rec = 0;
   uns32          rc = NCSCC_RC_SUCCESS;

   /* get the su record */
   su = m_AVND_SUDB_REC_GET(cb->sudb, *su_name_net);
   if (!su)
   {
      rc = AVND_ERR_NO_SU;
      goto err;
   }

   /* get the si record */
   si_rec = avnd_su_si_rec_get(cb, su_name_net, si_name_net);
   if (!si_rec)
   {
      rc = AVND_ERR_NO_SI;
      goto err;
   }

   /*
    * Delete the csi-list.
    */
   rc = avnd_su_si_csi_del(cb, su, si_rec);
   if ( NCSCC_RC_SUCCESS != rc ) goto err;

   /*
    * Detach from the si-list (maintained by su).
    */
   rc = m_AVND_SUDB_REC_SI_REM(*su, *si_rec);
   if ( NCSCC_RC_SUCCESS != rc ) goto err;

   m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_SI_DEL, AVND_LOG_SU_DB_SUCCESS, 
                    su_name_net, si_name_net,
                    NCSFL_SEV_INFO);

   /* free the memory */
   m_MMGR_FREE_AVND_SU_SI_REC(si_rec);

   return rc;

err:
   m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_SI_DEL, AVND_LOG_SU_DB_FAILURE, 
                    su_name_net, si_name_net,
                    NCSFL_SEV_CRITICAL);
   return rc;
}


/****************************************************************************
  Name          : avnd_su_si_del
 
  Description   : This routine traverses the entire si-list and deletes each
                  record.
 
  Arguments     : cb          - ptr to AvND control block
                  su_name_net - ptr to the su-name (n/w order)
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_su_si_del (AVND_CB *cb, SaNameT *su_name_net)
{
   AVND_SU        *su = 0;
   AVND_SU_SI_REC *si_rec = 0;
   SaNameT        su_name;
   uns32          rc = NCSCC_RC_SUCCESS;

   /* get the su record */
   su = m_AVND_SUDB_REC_GET(cb->sudb, *su_name_net);
   if (!su)
   {
      rc = AVND_ERR_NO_SU;
      goto err;
   }

   /* scan & delete each si record */
   while ( 0 != (si_rec = 
                (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_FIRST(&su->si_list)) )
   {
      rc = avnd_su_si_rec_del(cb, su_name_net, &si_rec->name_net);
      if ( NCSCC_RC_SUCCESS != rc ) goto err;
   }

   if(cb->term_state == AVND_TERM_STATE_SHUTTING_APP_DONE)
   {
      /* check whether all NCS SI are removed */
      m_NCS_MEMSET(&su_name, 0, sizeof(SaNameT));
      su = m_AVND_SUDB_REC_GET_NEXT(cb->sudb, su_name);

      while(su && (su->si_list.n_nodes == 0))
      {
         su_name.length = su->name_net.length;
         m_NCS_MEMCPY(&su_name.value, &su->name_net.value, m_NCS_OS_NTOHS(su->name_net.length));
         su = m_AVND_SUDB_REC_GET_NEXT(cb->sudb, su_name);
      }

      /* All SI's are removed, Now we can safely terminate the SU's */
      if(su == AVND_SU_NULL)
         cb->term_state = AVND_TERM_STATE_SHUTTING_NCS_SI;

   }

   return rc;

err:
   return rc;
}


/****************************************************************************
  Name          : avnd_su_si_csi_del
 
  Description   : This routine traverses the each record in the csi-list 
                  (maintained by si) & deletes them.
 
  Arguments     : cb      - ptr to AvND control block
                  su      - ptr to the AvND SU
                  si_rec  - ptr to the SI record
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_su_si_csi_del (AVND_CB *cb, AVND_SU *su, AVND_SU_SI_REC *si_rec)
{
   AVND_COMP_CSI_REC *csi_rec = 0;
   uns32             rc = NCSCC_RC_SUCCESS;

   /* scan & delete each csi record */
   while ( 0 != (csi_rec = 
                (AVND_COMP_CSI_REC *)m_NCS_DBLIST_FIND_FIRST(&si_rec->csi_list)) )
   {
      rc = avnd_su_si_csi_rec_del(cb, si_rec->su, si_rec, csi_rec);
      if ( NCSCC_RC_SUCCESS != rc ) goto err;
   }

   return rc;

err:
   return rc;
}


/****************************************************************************
  Name          : avnd_su_si_csi_rec_del
 
  Description   : This routine deletes a comp-csi relationship record.
 
  Arguments     : cb      - ptr to AvND control block
                  su      - ptr to the AvND SU
                  si_rec  - ptr to the SI record
                  csi_rec - ptr to the CSI record
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_su_si_csi_rec_del (AVND_CB          *cb, 
                              AVND_SU           *su, 
                              AVND_SU_SI_REC    *si_rec, 
                              AVND_COMP_CSI_REC *csi_rec)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* remove from the comp-csi list */
   rc = m_AVND_COMPDB_REC_CSI_REM(*(csi_rec->comp), *csi_rec);
   if ( NCSCC_RC_SUCCESS != rc ) goto err;

   /* if all csi's are removed & the pres state is un-instantiated for
    * a  npi comp, its time to mark it as healthy 
    */
   if(m_AVND_COMP_IS_FAILED(csi_rec->comp) &&
            !m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(csi_rec->comp) &&
            m_AVND_COMP_PRES_STATE_IS_UNINSTANTIATED(csi_rec->comp) &&
            csi_rec->comp->csi_list.n_nodes == 0)
      m_AVND_COMP_FAILED_RESET(csi_rec->comp);

   /* remove from the si-csi list */
   rc = m_AVND_SU_SI_CSI_REC_REM(*si_rec, *csi_rec);
   if ( NCSCC_RC_SUCCESS != rc ) goto err;

   /* 
    * Free the memory alloced for this record.
    */
   /* free the csi attributes */
   if (csi_rec->attrs.list)
      m_MMGR_FREE_AVSV_COMMON_DEFAULT_VAL(csi_rec->attrs.list);

   /* free the pg list TBD */
   
   /* unregister the comp-csi row with mab */
   avnd_mab_unreg_tbl_rows(cb, NCSMIB_TBL_AVSV_AMF_COMP_CSI, csi_rec->mab_hdl);

   m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_CSI_DEL, AVND_LOG_COMP_DB_SUCCESS, 
                      &csi_rec->comp->name_net, &csi_rec->name_net,
                      NCSFL_SEV_INFO);

   /* finally free this record */
   m_MMGR_FREE_AVND_COMP_CSI_REC(csi_rec);

   return rc;

err:
   m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_CSI_DEL, AVND_LOG_COMP_DB_FAILURE, 
                      &csi_rec->comp->name_net, &csi_rec->name_net,
                      NCSFL_SEV_CRITICAL);
   return rc;
}


/****************************************************************************
  Name          : avnd_su_si_rec_get
 
  Description   : This routine gets the su-si relationship record from the
                  si-list (maintained on su).
 
  Arguments     : cb          - ptr to AvND control block
                  su_name_net - ptr to the su-name (n/w order)
                  si_name_net - ptr to the si-name (n/w order)
 
  Return Values : ptr to the su-si record (if any)
 
  Notes         : None
******************************************************************************/
AVND_SU_SI_REC *avnd_su_si_rec_get (AVND_CB *cb, 
                                    SaNameT *su_name_net, 
                                    SaNameT *si_name_net)
{
   AVND_SU_SI_REC *si_rec = 0;
   AVND_SU        *su = 0;

   /* get the su record */
   su = m_AVND_SUDB_REC_GET(cb->sudb, *su_name_net);
   if (!su) goto done;

   /* get the si record */
   si_rec = (AVND_SU_SI_REC *)ncs_db_link_list_find(&su->si_list, 
                                                    (uns8 *)si_name_net);

done:
   return si_rec;
}


/****************************************************************************
  Name          : avnd_su_siq_rec_add
 
  Description   : This routine buffers the susi assign message parameters in 
                  the susi queue.
 
  Arguments     : cb    - ptr to AvND control block
                  su    - ptr to the AvND SU
                  param - ptr to the SI parameters
                  rc    - ptr to the operation result
 
  Return Values : ptr to the si queue record
 
  Notes         : None
******************************************************************************/
AVND_SU_SIQ_REC *avnd_su_siq_rec_add (AVND_CB          *cb, 
                                      AVND_SU          *su, 
                                      AVND_SU_SI_PARAM *param,
                                      uns32            *rc)
{
   AVND_SU_SIQ_REC *siq = 0;

   *rc = NCSCC_RC_SUCCESS;

   /* alloc the siq rec */
   siq = m_MMGR_ALLOC_AVND_SU_SIQ_REC;
   if (!siq)
   {
      *rc = AVND_ERR_NO_MEMORY;
      goto err;
   }

   m_NCS_OS_MEMSET(siq, 0, sizeof(AVND_SU_SIQ_REC));

   /* Add to the siq (maintained by su) */
   m_AVND_SUDB_REC_SIQ_ADD(*su, *siq, *rc);
   if ( NCSCC_RC_SUCCESS != *rc )
   {
      *rc = AVND_ERR_DLL;
      goto err;
   }

   /* update the param */
   siq->info = *param;

   /* memory transferred to the siq-rec.. nullify it in param */
   param->list = 0;

   return siq;

err:
   if (siq) m_MMGR_FREE_AVND_SU_SIQ_REC(siq);

   return 0;
}


/****************************************************************************
  Name          : avnd_su_siq_rec_del
 
  Description   : This routine deletes the buffered susi assign message from 
                  the susi queue.
 
  Arguments     : cb  - ptr to AvND control block
                  su  - ptr to the AvND SU
                  siq - ptr to the si queue rec
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avnd_su_siq_rec_del (AVND_CB *cb, AVND_SU *su, AVND_SU_SIQ_REC *siq)
{
   AVSV_SUSI_ASGN *curr = 0;

   /* delete the comp-csi info */
   while ((curr = siq->info.list) != 0)
   {
      siq->info.list = curr->next;
      if (curr->attrs.list)
         m_MMGR_FREE_AVSV_COMMON_DEFAULT_VAL(curr->attrs.list);

      m_MMGR_FREE_AVSV_DND_MSG_INFO(curr);
   }

   /* free the rec */
   m_MMGR_FREE_AVND_SU_SIQ_REC(siq);

   return;
}


