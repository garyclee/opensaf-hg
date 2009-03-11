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

   Vinay Khanna

..............................................................................

  DESCRIPTION:
   This module deals with the creation, accessing and deletion of the component
   database in the AVND. It also deals with all the MIB operations like get,
   getnext etc related to the component table on the AvND.

..............................................................................

  FUNCTIONS:

  
******************************************************************************
*/

#include "avnd.h"


/*****************************************************************************
 ****  Component Part of AVND AMF Configuration Database Layout           **** 
 *****************************************************************************
 
                   AVND_COMP
                   ---------------- 
   AVND_CB        | Stuff...       |
   -----------    | Attrs          |
  | COMP-Tree |-->|                |
  | ....      |   |                |
  | ....      |   |                |
   -----------    |                |
                  |                |
                  |                |
                  | Proxy ---------|-----> AVND_COMP (Proxy)
   AVND_SU        |                |
   -----------    |                |
  | Child     |-->|-SU-Comp-Next---|-----> AVND_COMP (Next)
  | Comp-List |   |                |
  |           |<--|-Parent SU      |       AVND_COMP_CSI_REC
   -----------    |                |       -------------
                  | CSI-Assign-List|----->|             |
                  |                |      |             |
                   ----------------        -------------


****************************************************************************/


/****************************************************************************
  Name          : avnd_compdb_init
 
  Description   : This routine initializes the component database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_compdb_init(AVND_CB *cb)
{
   NCS_PATRICIA_PARAMS params;
   uns32               rc = NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&params, 0, sizeof(NCS_PATRICIA_PARAMS));

   params.key_size = sizeof(SaNameT);
   rc = ncs_patricia_tree_init(&cb->compdb, &params);
   if (NCSCC_RC_SUCCESS == rc)
      m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_CREATE, AVND_LOG_COMP_DB_SUCCESS, 
                         0, 0, NCSFL_SEV_INFO);
   else
      m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_CREATE, AVND_LOG_COMP_DB_FAILURE, 
                         0, 0, NCSFL_SEV_CRITICAL);

   return rc;
}


/****************************************************************************
  Name          : avnd_compdb_destroy
 
  Description   : This routine destroys the component database. It deletes 
                  all the component records in the database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_compdb_destroy(AVND_CB *cb)
{
   AVND_COMP *comp = 0;
   uns32     rc = NCSCC_RC_SUCCESS;

   /* scan & delete each comp */
   while ( 0 != (comp = 
            (AVND_COMP *)ncs_patricia_tree_getnext(&cb->compdb, (uns8 *)0)) )
   {
      /* unreg the row from mab */
      avnd_mab_unreg_tbl_rows(cb, NCSMIB_TBL_AVSV_NCS_COMP_STAT, comp->mab_hdl,
           (comp->su->su_is_external?cb->avnd_mbcsv_mab_hdl:cb->mab_hdl));

      /* delete the record 
      m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, comp, AVND_CKPT_COMP_CONFIG);
      AvND is going down, but don't send any async update even for 
      external components, otherwise external components will be deleted
      from ACT.*/
      rc = avnd_compdb_rec_del(cb, &comp->name_net);
      if ( NCSCC_RC_SUCCESS != rc ) goto err;
   }

   /* finally destroy patricia tree */
   rc = ncs_patricia_tree_destroy(&cb->compdb);
   if ( NCSCC_RC_SUCCESS != rc ) goto err;

   m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_DESTROY, AVND_LOG_COMP_DB_SUCCESS, 
                      0, 0, NCSFL_SEV_INFO);
   return rc;

err:
   m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_DESTROY, AVND_LOG_COMP_DB_FAILURE, 
                      0, 0, NCSFL_SEV_CRITICAL);
   return rc;
}


/****************************************************************************
  Name          : avnd_compdb_rec_add
 
  Description   : This routine adds a component record to the component 
                  database. If a component is already present, nothing is 
                  done.
 
  Arguments     : cb   - ptr to the AvND control block
                  info - ptr to the component params (comp-name -> nw order)
                  rc   - ptr to the operation result
 
  Return Values : ptr to the component record, if success
                  0, otherwise
 
  Notes         : None.
******************************************************************************/
AVND_COMP *avnd_compdb_rec_add(AVND_CB *cb, AVND_COMP_PARAM *info, uns32 *rc)
{
   AVND_COMP *comp = 0;
   AVND_SU   *su = 0;
   SaNameT   su_name_net;

   *rc = NCSCC_RC_SUCCESS;

   /* verify if this component is already present in the db */
   if ( 0 != m_AVND_COMPDB_REC_GET(cb->compdb, info->name_net) )
   {
      *rc = AVND_ERR_DUP_COMP;
      goto err;
   }

   /*
    * Determine if the SU is present
    */
   /* extract the su-name from comp dn */
   m_NCS_MEMSET(&su_name_net, 0, sizeof(SaNameT));
   avsv_cpy_SU_DN_from_DN(&su_name_net, &info->name_net);
   su_name_net.length = m_NCS_OS_HTONS(su_name_net.length);
   
   /* get the su record */
   su = m_AVND_SUDB_REC_GET(cb->sudb, su_name_net);
   if (!su)
   {
      *rc = AVND_ERR_NO_SU;
      goto err;
   }
   
   /* a fresh comp... */
   comp = m_MMGR_ALLOC_AVND_COMP;
   if (!comp) 
   {
      *rc = AVND_ERR_NO_MEMORY;
      goto err;
   }

   m_NCS_OS_MEMSET(comp, 0, sizeof(AVND_COMP));
   
   /*
    * Update the config parameters.
    */
   /* update the comp-name (patricia key) */
   memcpy(&comp->name_net, &info->name_net, sizeof(SaNameT));
   
   /* update the component attributes */
   comp->inst_level = info->inst_level;

   comp->is_am_en   = info->am_enable;
 
   switch (info->category)
   {
   case NCS_COMP_TYPE_SA_AWARE:
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_SAAWARE);
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PREINSTANTIABLE);
      break;

   case NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE:
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_SAAWARE);
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PREINSTANTIABLE);
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
      break;

   case NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE:
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_SAAWARE);
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
      break;

   case NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE:
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PREINSTANTIABLE);
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
      break;

   case NCS_COMP_TYPE_EXTERNAL_NON_PRE_INSTANTIABLE:
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_PROXIED);
      break;

   case NCS_COMP_TYPE_NON_SAF:
      m_AVND_COMP_TYPE_SET(comp, AVND_COMP_TYPE_LOCAL);
      break;

   default:
      break;
   } /* switch */

   m_AVND_COMP_RESTART_EN_SET(comp, (info->comp_restart == TRUE) ? FALSE : TRUE);

   comp->cap = info->cap;
   comp->node_id = cb->clmdb.node_info.nodeId;
   
   /* update CLC params */
   comp->clc_info.inst_retry_max = info->max_num_inst;
   comp->clc_info.am_start_retry_max = info->max_num_amstart;
   
   /* instantiate cmd params */
   memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].cmd, 
      info->init_info, info->init_len);
   comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].len = 
      info->init_len;
   comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].timeout = 
      info->init_time;
   
   /* terminate cmd params */
   memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].cmd, 
      info->term_info, info->term_len);
   comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].len = 
      info->term_len;
   comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].timeout = 
      info->term_time;

   /* cleanup cmd params */
   memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].cmd, 
      info->clean_info, info->clean_len);
   comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].len = 
      info->clean_len;
   comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_CLEANUP - 1].timeout = 
      info->clean_time;
   
   /* am-start cmd params */
   memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].cmd, 
      info->amstart_info, info->amstart_len);
   comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].len = 
      info->amstart_len;
   comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTART - 1].timeout = 
      info->amstart_time;
   
   /* am-stop cmd params */
   memcpy(comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].cmd, 
      info->amstop_info, info->amstop_len);
   comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].len = 
      info->amstop_len;
   comp->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_AMSTOP - 1].timeout = 
      info->amstop_time;

   /* update the callback response time out values */
   if (info->terminate_callback_timeout)
      comp->term_cbk_timeout = info->terminate_callback_timeout;
   else
      comp->term_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

   if (info->csi_set_callback_timeout)
      comp->csi_set_cbk_timeout = info->csi_set_callback_timeout;
   else
      comp->csi_set_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

   if (info->quiescing_complete_timeout)
      comp->quies_complete_cbk_timeout = info->quiescing_complete_timeout;
   else
      comp->quies_complete_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

   if (info->csi_rmv_callback_timeout)
      comp->csi_rmv_cbk_timeout = info->csi_rmv_callback_timeout;
   else
      comp->csi_rmv_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

   if (info->proxied_inst_callback_timeout)
      comp->pxied_inst_cbk_timeout = info->proxied_inst_callback_timeout;
   else
      comp->pxied_inst_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

   if (info->proxied_clean_callback_timeout)
      comp->pxied_clean_cbk_timeout = info->proxied_clean_callback_timeout;
   else
      comp->pxied_clean_cbk_timeout = ((SaTimeT)AVND_COMP_CBK_RESP_TIME) * 1000000;

   /* update the default error recovery param */
   comp->err_info.def_rec = info->def_recvr;
   
   /*
    * Update the rest of the parameters with default values.
    */
   if ( m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(comp) )
      m_AVND_COMP_OPER_STATE_SET(comp, NCS_OPER_STATE_DISABLE);
   else
      m_AVND_COMP_OPER_STATE_SET(comp, NCS_OPER_STATE_ENABLE);

   comp->avd_updt_flag = FALSE;


   /* synchronize comp oper state */
   m_AVND_COMP_OPER_STATE_AVD_SYNC(cb, comp, *rc);
   if ( NCSCC_RC_SUCCESS != *rc ) goto err;
   
   comp->pres = NCS_PRES_UNINSTANTIATED;
   
   /* create the association with hdl-mngr */
   if ( (0 == (comp->comp_hdl = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_AVND,
                                                 (NCSCONTEXT)comp))) )
   {
      *rc = AVND_ERR_HDL;
      goto err;
   }

   /* 
    * Initialize the comp-hc list.
    */
   comp->hc_list.order = NCS_DBLIST_ANY_ORDER;
   comp->hc_list.cmp_cookie = avnd_dblist_hc_rec_cmp;
   comp->hc_list.free_cookie = 0;
   
   /* 
    * Initialize the comp-csi list.
    */
   comp->csi_list.order = NCS_DBLIST_ASSCEND_ORDER;
   comp->csi_list.cmp_cookie = avsv_dblist_saname_cmp;
   comp->csi_list.free_cookie = 0;

   /* 
    * Initialize the pm list.
    */
   avnd_pm_list_init(comp); 
   
   /*
    * initialize proxied list
    */
   avnd_pxied_list_init(comp); 
   
   /*
    * Add to the patricia tree.
    */
   comp->tree_node.bit = 0;
   comp->tree_node.key_info = (uns8*)&comp->name_net;
   *rc = ncs_patricia_tree_add(&cb->compdb, &comp->tree_node);
   if ( NCSCC_RC_SUCCESS != *rc )
   {
      *rc = AVND_ERR_TREE;
      goto err;
   }
   
   /*
    * Add to the comp-list (maintained by su)
    */
   m_AVND_SUDB_REC_COMP_ADD(*su, *comp, *rc);
   if ( NCSCC_RC_SUCCESS != *rc )
   {
      *rc = AVND_ERR_DLL;
      goto err;
   }
   
   /*
    * Update su bk ptr.
    */
   comp->su = su;

   if(TRUE == su->su_is_external)
   {
     m_AVND_COMP_TYPE_SET_EXT_CLUSTER(comp);
   }
   else
     m_AVND_COMP_TYPE_SET_LOCAL_NODE(comp);


   m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_REC_ADD, AVND_LOG_COMP_DB_SUCCESS, 
                      &info->name_net, 0, NCSFL_SEV_INFO);
   return comp;

err:
   if ( AVND_ERR_DLL == *rc )
      ncs_patricia_tree_del(&cb->compdb, &comp->tree_node);

   if (comp) 
   {
      if (comp->comp_hdl) 
         ncshm_destroy_hdl(NCS_SERVICE_ID_AVND, comp->comp_hdl);

      m_MMGR_FREE_AVND_COMP(comp);
   }

   m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_REC_ADD, AVND_LOG_COMP_DB_FAILURE, 
                      &info->name_net, 0, NCSFL_SEV_CRITICAL);
   return 0;
}


/****************************************************************************
  Name          : avnd_compdb_rec_del
 
  Description   : This routine deletes a component record from the component 
                  database. 
 
  Arguments     : cb       - ptr to the AvND control block
                  name_net - ptr to the comp-name (in n/w order)
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : This routine expects a NULL comp-csi list.
******************************************************************************/
uns32 avnd_compdb_rec_del(AVND_CB *cb, SaNameT *name_net)
{
   AVND_COMP *comp = 0;
   AVND_SU   *su = 0;
   SaNameT   su_name_net;
   uns32     rc = NCSCC_RC_SUCCESS;

   /* get the comp */
   comp = m_AVND_COMPDB_REC_GET(cb->compdb, *name_net);
   if (!comp)
   {
      rc = AVND_ERR_NO_COMP;
      goto err;
   }

   /* comp should not be attached to any csi when it is being deleted */
   m_AVSV_ASSERT(!comp->csi_list.n_nodes);

   /*
    * Determine if the SU is present
    */
   /* extract the su-name from comp dn */
   m_NCS_MEMSET(&su_name_net, 0, sizeof(SaNameT));
   avsv_cpy_SU_DN_from_DN(&su_name_net, name_net);
   su_name_net.length = m_NCS_OS_HTONS(su_name_net.length);
   
   /* get the su record */
   su = m_AVND_SUDB_REC_GET(cb->sudb, su_name_net);
   if (!su)
   {
      rc = AVND_ERR_NO_SU;
      goto err;
   }

   /* 
    * Remove from the comp-list (maintained by su).
    */
   rc = m_AVND_SUDB_REC_COMP_REM(*su, *comp);
   if ( NCSCC_RC_SUCCESS != rc )
   {
      rc = AVND_ERR_DLL;
      goto err;
   }

   /* 
    * Remove from the patricia tree.
    */
   rc = ncs_patricia_tree_del(&cb->compdb, &comp->tree_node);
   if ( NCSCC_RC_SUCCESS != rc )
   {
      rc = AVND_ERR_TREE;
      goto err;
   }

   /* 
    * Delete the various lists (hc, pm, pg, cbk etc) maintained by this comp.
    */
   avnd_comp_hc_rec_del_all(cb, comp);
   avnd_comp_cbq_del(cb, comp, FALSE);
   avnd_comp_pm_rec_del_all(cb, comp);

   /* remove the association with hdl mngr */
   ncshm_destroy_hdl(NCS_SERVICE_ID_AVND, comp->comp_hdl);

   m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_REC_DEL, AVND_LOG_COMP_DB_SUCCESS, 
                      name_net, 0, NCSFL_SEV_INFO);

   /* free the memory */
   m_MMGR_FREE_AVND_COMP(comp);

   return rc;

err:
   m_AVND_LOG_COMP_DB(AVND_LOG_COMP_DB_REC_DEL, AVND_LOG_COMP_DB_FAILURE, 
                      name_net, 0, NCSFL_SEV_CRITICAL);
   return rc;
}


/****************************************************************************
  Name          : avnd_compdb_csi_rec_get
 
  Description   : This routine gets the comp-csi relationship record from 
                  the csi-list (maintained on comp).
 
  Arguments     : cb            - ptr to AvND control block
                  comp_name_net - ptr to the comp-name (n/w order)
                  csi_name_net  - ptr to the CSI name (n/w order)
 
  Return Values : ptr to the comp-csi record (if any)
 
  Notes         : None
******************************************************************************/
AVND_COMP_CSI_REC *avnd_compdb_csi_rec_get (AVND_CB *cb, 
                                            SaNameT *comp_name_net, 
                                            SaNameT *csi_name_net)
{
   AVND_COMP_CSI_REC *csi_rec = 0;
   AVND_COMP         *comp = 0;

   /* get the comp & csi records */
   comp = m_AVND_COMPDB_REC_GET(cb->compdb, *comp_name_net);
   if (comp) csi_rec = m_AVND_COMPDB_REC_CSI_GET(*comp, *csi_name_net);

   return csi_rec;
}


/****************************************************************************
  Name          : avnd_compdb_csi_rec_get_next
 
  Description   : This routine gets the next comp-csi relationship record from 
                  the csi-list (maintained on comp).
 
  Arguments     : cb            - ptr to AvND control block
                  comp_name_net - ptr to the comp-name (n/w order)
                  csi_name_net  - ptr to the CSI name (n/w order)
 
  Return Values : ptr to the comp-csi record (if any)
 
  Notes         : None
******************************************************************************/
AVND_COMP_CSI_REC *avnd_compdb_csi_rec_get_next(AVND_CB *cb, 
                                                SaNameT *comp_name_net, 
                                                SaNameT *csi_name_net)
{
   AVND_COMP_CSI_REC *csi = 0;
   AVND_COMP         *comp = 0;

   /* get the comp  & the next csi */
   comp = m_AVND_COMPDB_REC_GET(cb->compdb, *comp_name_net);
   if (comp) 
   {
      if (csi_name_net->length)
         for ( csi = m_AVND_COMPDB_REC_CSI_GET_FIRST(*comp); 
               csi && !(m_CMP_NORDER_SANAMET(*csi_name_net, csi->name_net) < 0);
               csi = m_AVND_COMPDB_REC_CSI_NEXT(*comp, *csi) );
      else
         csi = m_AVND_COMPDB_REC_CSI_GET_FIRST(*comp);
   }

   /* found the csi */
   if (csi) goto done;

   /* find the csi in the remaining comp recs */
   for ( comp = m_AVND_COMPDB_REC_GET_NEXT(cb->compdb, *comp_name_net); comp;
         comp = m_AVND_COMPDB_REC_GET_NEXT(cb->compdb, comp->name_net) )
   {
      csi = m_AVND_COMPDB_REC_CSI_GET_FIRST(*comp);
      if (csi) break;
   }

done:
   return csi;
}



/*****************************************************************************
 * Function: ncsscomptableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in NCS_S_COMP_TABLE_ENTRY_ID table. This is the status component table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function finds the corresponding data structure for the given
 * instance and returns the pointer to the structure.
 *
 * Input:  cb        - AVND control block.
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 * NOTES: This function works in conjunction with extract function to provide the
 * get functionality.
 *
 * 
 **************************************************************************/
uns32 ncsscomptableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   AVND_CB   *avnd_cb = (AVND_CB *)cb;
   SaNameT   comp_name;
   AVND_COMP *comp = NULL;
   uns32     i;
  
  
   if (avnd_cb->admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_OS_MEMSET(&comp_name, 0, sizeof(SaNameT));

   /* Prepare the component name from the instant ID */
   comp_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for (i = 0; i < comp_name.length; i++)
   {
       comp_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }
                             
   /* Check if this comp exists in the database */
   comp_name.length = m_NCS_OS_HTONS(comp_name.length);
   comp = m_AVND_COMPDB_REC_GET(avnd_cb->compdb, comp_name);
   if (comp == AVND_COMP_NULL)
   {
      /* Row doesn't exists */ 
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)comp;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: ncsscomptableentry_rmvrow
 *
 * Purpose:  This function is one of the RMVROW processing routines for objects
 * in NCS_CLM_TABLE_ENTRY_ID table. 
 *
 * Input:  cb        - AVD control block.
 *         idx       - pointer to NCSMIB_IDX 
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 **************************************************************************/
uns32 ncsscomptableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx)
{
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncsscomptableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * NCS_S_COMP_TABLE_ENTRY_ID table. This is the status component table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after calling the get call to get data structure.
 * This function fills the value information in the param filed structure. For
 * octate information the buffer field will be used for filling the information.
 * MIBLIB will provide the memory and pointer to the buffer. For only objects that
 * have a direct value(i.e their offset is not 0 in VAR INFO) in the structure
 * the data field is filled using the VAR INFO provided by MIBLIB, for others based
 * on the OID the value is filled accordingly.
 *
 * Input:  param     -  param->i_param_id indicates the parameter to extract
 *                      The remaining elements of the param need to be filled
 *                      by the subystem's extract function
 *         var_info  - Pointer to the var_info structure for the param.
 *         data      - The pointer to the data-structure containing the object
 *                     value which we have already provided to MIBLIB from get call.
 *         buffer    - The buffer pointer provided by MIBLIB for filling the octate
 *                     type data.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 * NOTES:  This function works in conjunction with other functions to provide the
 * get,getnext and getrow functionality.
 *
 * 
 **************************************************************************/
uns32 ncsscomptableentry_extract(NCSMIB_PARAM_VAL *param, 
                                 NCSMIB_VAR_INFO *var_info,
                                 NCSCONTEXT data,
                                 NCSCONTEXT buffer)
{
   AVND_COMP *comp = (AVND_COMP *)data;


   if (comp == AVND_COMP_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {
   case ncsSCompErrDetectTime_ID:    
       m_AVSV_UNS64_TO_PARAM(param, buffer, comp->err_info.detect_time);
       break;

   default:
       /* call the MIBLIB utility routine for standfard object types */
       if ((var_info != NULL) && (var_info->offset != 0))
          return ncsmiblib_get_obj_val(param, var_info, data, buffer);
       else
          return NCSCC_RC_NO_OBJECT;
       break;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncsscomptableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * NCS_S_COMP_TABLE_ENTRY_ID table. This is the status component table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for the objects that are settable. This same function can be used for test
 * operation also.
 *
 * Input:  cb        - AVND control block
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         var_info  - The VAR INFO structure pointer generated by MIBLIB for
 *                     the objects in this table.
 *         test_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: This table currently doesn't have any set objects so this function
 *        shouldn't be called. If called will return invalid error.
 *
 * 
 **************************************************************************/
uns32 ncsscomptableentry_set(NCSCONTEXT cb,
                             NCSMIB_ARG *arg, 
                             NCSMIB_VAR_INFO *var_info,
                             NCS_BOOL test_flag)
{
   /* This table has only read-only objects */ 
   return NCSCC_RC_INV_VAL;
}



/*****************************************************************************
 * Function: ncsscomptableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * NCS_S_COMP_TABLE_ENTRY_ID table. This is the status component table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function gets the next valid instance and its data structure
 * and it passes to the MIBLIB the values.
 *
 * Input: cb        - AVND control block.
 *        arg       - The pointer to the MIB arg that was provided by the caller.
 *        data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *     next_inst_id - The next instance id will be filled in this buffer
 *                     and returned by reference.
 * next_inst_id_len - The next instance id length.
 *
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context
 *
 * NOTES: This function works in conjunction with extract function to provide the
 * getnext functionality.
 *
 * 
 **************************************************************************/
uns32 ncsscomptableentry_next(NCSCONTEXT cb, 
                              NCSMIB_ARG *arg, 
                              NCSCONTEXT* data,
                              uns32* next_inst_id,
                              uns32 *next_inst_id_len)
{
   AVND_CB   *avnd_cb = (AVND_CB *)cb;
   SaNameT   comp_name;
   AVND_COMP *comp = NULL;
   uns32     i;
 
  
   if (avnd_cb->admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_OS_MEMSET(&comp_name, 0, sizeof(SaNameT));

   if (arg->i_idx.i_inst_len != 0)
   {
      /* Prepare the component name from the instant ID */
      comp_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for (i = 0; i < comp_name.length; i++)
      {
          comp_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }
                             
   /* Check if this comp exists in the database */
   comp_name.length = m_NCS_OS_HTONS(comp_name.length);
   comp = m_AVND_COMPDB_REC_GET_NEXT(avnd_cb->compdb, comp_name);
   if (comp == AVND_COMP_NULL)
   {
      /* Row doesn't exists */ 
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the component name */
   comp_name = comp->name_net;
   comp_name.length = m_NCS_OS_NTOHS(comp_name.length);

   *next_inst_id_len = comp_name.length + 1;

   next_inst_id[0] = comp_name.length;
   for(i = 0; i < comp_name.length; i++)
      next_inst_id[i + 1] = (uns32)(comp_name.value[i]);

   *data = (NCSCONTEXT)comp;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncsscomptableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * NCS_S_COMP_TABLE_ENTRY_ID table.This is the status component table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for all the objects that are settable as part of the setrow operation. 
 * This same function can be used for test row
 * operation also.
 *
 * Input:  cb        - AVND control block
 *         args      - The pointer to the MIB arg that was provided by the caller.
 *         params    - The List of object ids and their values.
 *         obj_info  - The VAR INFO structure array pointer generated by MIBLIB for
 *                     the objects in this table.
 *      testrow_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: This table currently doesn't have any set objects so this function
 *        shouldn't be called. If called will return invalid error.
 *
 * 
 **************************************************************************/
uns32 ncsscomptableentry_setrow(NCSCONTEXT cb,
                                NCSMIB_ARG* args,
                                NCSMIB_SETROW_PARAM_VAL* params,
                                struct ncsmib_obj_info* obj_info,
                                NCS_BOOL testrow_flag)
{
   /* This table has only read-only objects */ 
   return NCSCC_RC_INV_VAL;
}


/*****************************************************************************
 * Function: saamfscompcsitableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in NCS_S_COMP_CSI_TABLE_ENTRY_ID table. This is the status component CSI table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function finds the corresponding data structure for the given
 * instance and returns the pointer to the structure.
 *
 * Input:  cb        - AVND control block.
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 * NOTES: This function works in conjunction with extract function to provide the
 * get functionality.
 *
 * 
 **************************************************************************/
uns32 saamfscompcsitableentry_get(NCSCONTEXT cb,
                                NCSMIB_ARG *arg,
                                NCSCONTEXT *data)
{
   AVND_CB   *avnd_cb = (AVND_CB *)cb;
   AVND_COMP_CSI_REC *csi_rel = NULL;
   SaNameT   comp_name, csi_name;
   uns32     i, csi_index;


   if (avnd_cb->admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_OS_MEMSET(&comp_name, 0, sizeof(SaNameT));
   m_NCS_OS_MEMSET(&csi_name, 0, sizeof(SaNameT));

   /* Prepare the comp name from the instant ID */
   comp_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for (i = 0; i < comp_name.length; i++)
      comp_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   
   csi_index = comp_name.length + 1;

   /* Prepare the csi name from the instant ID */
   csi_name.length = (SaUint16T)arg->i_idx.i_inst_ids[csi_index];
   for (i = 0; i < comp_name.length; i++)
   {
      csi_index++;
      csi_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[csi_index]);
   }
   
   /* Get the csi rec */
   comp_name.length = m_NCS_OS_HTONS(comp_name.length);
   csi_name.length = m_NCS_OS_HTONS(csi_name.length);
   csi_rel = avnd_compdb_csi_rec_get(cb, &comp_name, &csi_name);

   if (csi_rel == AVND_COMP_CSI_REC_NULL) return NCSCC_RC_NO_INSTANCE;
   
   if(m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_REMOVED(csi_rel) || m_AVND_COMP_OPER_STATE_IS_DISABLED(csi_rel->comp))
      return NCSCC_RC_NO_INSTANCE;

   if((m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_UNASSIGNED(csi_rel) ||
            m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_ASSIGNING(csi_rel)) &&
         ((csi_rel->prv_assign_state == 0) || m_AVND_COMP_CSI_PRV_ASSIGN_STATE_IS_UNASSIGNED(csi_rel)))
      return NCSCC_RC_NO_INSTANCE;


   *data = (NCSCONTEXT)csi_rel;
   
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfscompcsitableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * NCS_S_COMP_CSI_TABLE_ENTRY_ID table. This is the status component CSI table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after calling the get call to get data structure.
 * This function fills the value information in the param filed structure. For
 * octate information the buffer field will be used for filling the information.
 * MIBLIB will provide the memory and pointer to the buffer. For only objects that
 * have a direct value(i.e their offset is not 0 in VAR INFO) in the structure
 * the data field is filled using the VAR INFO provided by MIBLIB, for others based
 * on the OID the value is filled accordingly.
 *
 * Input:  param     -  param->i_param_id indicates the parameter to extract
 *                      The remaining elements of the param need to be filled
 *                      by the subystem's extract function
 *         var_info  - Pointer to the var_info structure for the param.
 *         data      - The pointer to the data-structure containing the object
 *                     value which we have already provided to MIBLIB from get call.
 *         buffer    - The buffer pointer provided by MIBLIB for filling the octate
 *                     type data.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 * NOTES:  This function works in conjunction with other functions to provide the
 * get,getnext and getrow functionality.
 *
 * 
 **************************************************************************/
uns32 saamfscompcsitableentry_extract(NCSMIB_PARAM_VAL* param, 
                                    NCSMIB_VAR_INFO* var_info,
                                    NCSCONTEXT data,
                                    NCSCONTEXT buffer)
{
   AVND_COMP_CSI_REC *csi = (AVND_COMP_CSI_REC *)data;

   if (csi == AVND_COMP_CSI_REC_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {
   case saAmfSCompCsiHAState_ID:    
      param->i_fmat_id = NCSMIB_FMAT_INT; 
      param->i_length = 1;
      param->info.i_int = (m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_ASSIGNED(csi) ||
            (m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_REMOVING(csi))) ?
         csi->si->curr_state : csi->si->prv_state;
      break;

   default:
       /* call the MIBLIB utility routine for standfard object types */
       if ((var_info != NULL) && (var_info->offset != 0))
          return ncsmiblib_get_obj_val(param, var_info, data, buffer);
       else
          return NCSCC_RC_NO_OBJECT;
       break;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfscompcsitableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * NCS_S_COMP_CSI_TABLE_ENTRY_ID table. This is the status component CSI table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for the objects that are settable. This same function can be used for test
 * operation also.
 *
 * Input:  cb        - AVND control block
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         var_info  - The VAR INFO structure pointer generated by MIBLIB for
 *                     the objects in this table.
 *         test_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: This table currently doesn't have any set objects so this function
 *        shouldn't be called. If called will return invalid error.
 *
 * 
 **************************************************************************/
uns32 saamfscompcsitableentry_set(NCSCONTEXT cb,
                                NCSMIB_ARG *arg, 
                                NCSMIB_VAR_INFO* var_info,
                                NCS_BOOL test_flag)
{
   /* This table has only read-only objects */ 
   return NCSCC_RC_INV_VAL;
}



/*****************************************************************************
 * Function: saamfscompcsitableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * NCS_S_COMP_CSI_TABLE_ENTRY_ID table. This is the status component CSI table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function gets the next valid instance and its data structure
 * and it passes to the MIBLIB the values.
 *
 * Input: cb        - AVND control block.
 *        arg       - The pointer to the MIB arg that was provided by the caller.
 *        data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *     next_inst_id - The next instance id will be filled in this buffer
 *                     and returned by reference.
 * next_inst_id_len - The next instance id length.
 *
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context
 *
 * NOTES: This function works in conjunction with extract function to provide the
 * getnext functionality.
 *
 * 
 **************************************************************************/
uns32 saamfscompcsitableentry_next(NCSCONTEXT cb,
                                 NCSMIB_ARG *arg, 
                                 NCSCONTEXT* data,
                                 uns32* next_inst_id,
                                 uns32 *next_inst_id_len)
{
   AVND_CB           *avnd_cb = (AVND_CB *)cb;
   AVND_COMP_CSI_REC *csi = 0;
   SaNameT           comp_name, csi_name;
   uns32             i, csi_index;


   if (avnd_cb->admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_OS_MEMSET(&comp_name, 0, sizeof(SaNameT));
   m_NCS_OS_MEMSET(&csi_name, 0, sizeof(SaNameT));

   /* Prepare the SU name from the instant ID */
   if (arg->i_idx.i_inst_len != 0)
   {
      /* Prepare the comp name from the instant ID */
      comp_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for (i = 0; i < comp_name.length; i++)
      {
          comp_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
  
      if (arg->i_idx.i_inst_len > (uns32)(comp_name.length + 1))
      {
         csi_index = comp_name.length + 1;

         /* Prepare the csi name from the instant ID */
         csi_name.length = (SaUint16T)arg->i_idx.i_inst_ids[csi_index];
         for (i = 0; i < csi_name.length; i++)
         {
            csi_index++;
            csi_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[csi_index]);
         }
      }
   }

   comp_name.length = m_NCS_OS_HTONS(comp_name.length);
   csi_name.length = m_NCS_OS_HTONS(csi_name.length);
   while(NULL != (csi = avnd_compdb_csi_rec_get_next(cb, &comp_name, &csi_name)))
   {
      comp_name = csi->comp->name_net;
      csi_name = csi->name_net;
      if(m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_REMOVED(csi) || m_AVND_COMP_OPER_STATE_IS_DISABLED(csi->comp))
         continue;
      if((m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_UNASSIGNED(csi) ||
               m_AVND_COMP_CSI_CURR_ASSIGN_STATE_IS_ASSIGNING(csi)) &&
            ((csi->prv_assign_state == 0)  || m_AVND_COMP_CSI_PRV_ASSIGN_STATE_IS_UNASSIGNED(csi)))
         continue;

      break;
   }
   if (!csi) return NCSCC_RC_NO_INSTANCE;

   /* Prepare the instant ID from the component name and CSI name */
   comp_name.length = m_NCS_OS_NTOHS(comp_name.length);
   csi_name.length = m_NCS_OS_NTOHS(csi_name.length);
   
   *next_inst_id_len = csi_name.length + comp_name.length + 2;

   next_inst_id[0] = comp_name.length;
   for(i = 0; i < comp_name.length; i++)
      next_inst_id[i + 1] = (uns32)(comp_name.value[i]);

   next_inst_id[i + 1] = csi_name.length;
   csi_index = i + 2;
   for(i = 0; i < csi_name.length; i++)
      next_inst_id[i + csi_index] = (uns32)(csi_name.value[i]);

   *data = (NCSCONTEXT)csi;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfscompcsitableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * NCS_S_COMP_CSI_TABLE_ENTRY_ID table.This is the status component CSI table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for all the objects that are settable as part of the setrow operation. 
 * This same function can be used for test row
 * operation also.
 *
 * Input:  cb        - AVND control block
 *         args      - The pointer to the MIB arg that was provided by the caller.
 *         params    - The List of object ids and their values.
 *         obj_info  - The VAR INFO structure array pointer generated by MIBLIB for
 *                     the objects in this table.
 *      testrow_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: This table currently doesn't have any set objects so this function
 *        shouldn't be called. If called will return invalid error.
 *
 * 
 **************************************************************************/
uns32 saamfscompcsitableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   /* This table has only read-only objects */ 
   return NCSCC_RC_INV_VAL;
}

/*****************************************************************************
 * Function: saamfscompcsitableentry_rmvrow
 *
 * Purpose:  This function is one of the RMVROW processing routines for objects
 * in NCS_CLM_TABLE_ENTRY_ID table. 
 *
 * Input:  cb        - AVD control block.
 *         idx       - pointer to NCSMIB_IDX 
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 **************************************************************************/
uns32 saamfscompcsitableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx)
{
   return NCSCC_RC_SUCCESS;
}


