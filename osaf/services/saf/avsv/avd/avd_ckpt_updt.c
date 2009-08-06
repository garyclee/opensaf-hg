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

  DESCRIPTION: This module contain all the supporting functions for the
               checkpointing operation. 

..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"
static uns32 avd_remove_csi(AVD_CL_CB *cb, AVD_CSI *csi_ptr);

/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_avnd
 *
 * Purpose:  Add new AVND entry if action is ADD, remove node from the tree if 
 *           action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        avnd - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_ckpt_add_rmv_updt_avnd(AVD_CL_CB *cb, AVD_AVND *avnd, 
                                   NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_AVND     *avnd_ptr;
   SaNameT       node_name;

   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_avnd");

   memset(&node_name, '\0', sizeof(SaNameT));
   node_name.length = m_NCS_OS_NTOHS(avnd->node_info.nodeName.length);
   memcpy(node_name.value, avnd->node_info.nodeName.value, node_name.length); 
   avnd_ptr = avd_avnd_struc_find(cb, node_name);

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
      {
         if (NULL == avnd_ptr)
         {
            /*
             * add new AVND entry into pat tree.
             */
            if (AVD_AVND_NULL == (avnd_ptr = 
               avd_avnd_struc_crt(cb, avnd->node_info.nodeName, TRUE)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(avnd->node_info.nodeId);
               return NCSCC_RC_FAILURE;
            }
         }

         if(avnd_ptr->row_status != NCS_ROW_ACTIVE)
         {
            /* Update One time update fields */
            avnd_ptr->node_info.nodeAddress = avnd->node_info.nodeAddress;
            avnd_ptr->node_info.nodeId = avnd->node_info.nodeId;
            avnd_ptr->node_info.member = avnd->node_info.member;
            avnd_ptr->node_info.bootTimestamp = avnd->node_info.bootTimestamp;
            avnd_ptr->node_info.initialViewNumber = 
               avnd->node_info.initialViewNumber;
            avnd_ptr->adest = avnd->adest;
            avnd_ptr->row_status = avnd->row_status;
            avnd_ptr->type = avnd->type;

            if (NCSCC_RC_SUCCESS != avd_avnd_struc_add_nodeid(cb, avnd_ptr))
            {
               avnd_ptr->row_status = NCSMIB_ROWSTATUS_NOTINSERVICE;
               m_AVD_LOG_INVALID_VAL_FATAL(avnd->node_info.nodeId);
               return NCSCC_RC_FAILURE;
            }
         }
    }

      /* 
       * Don't break...continue updating data to this AVND 
       * as done during normal update request.
       */

   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL != avnd_ptr)
         {
           /*
            * Update all the data. Except node name which would have got
            * updated with ADD.
            */
            avnd_ptr->su_admin_state = avnd->su_admin_state;
            avnd_ptr->oper_state = avnd->oper_state;
            avnd_ptr->node_state = avnd->node_state;
            avnd_ptr->su_failover_prob = avnd->su_failover_prob;
            avnd_ptr->su_failover_max = avnd->su_failover_max;
            avnd_ptr->rcv_msg_id = avnd->rcv_msg_id;
            avnd_ptr->snd_msg_id = avnd->snd_msg_id;
            avnd_ptr->avm_oper_state = avnd->avm_oper_state;
            avnd_ptr->type = avnd->type;

            avnd_ptr->node_info.member = avnd->node_info.member;
            avnd_ptr->node_info.bootTimestamp = avnd->node_info.bootTimestamp;
            avnd_ptr->node_info.initialViewNumber = avnd->node_info.initialViewNumber;
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(avnd->node_info.nodeId);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != avnd_ptr)
         {
            /*
             * Remove this AVND from the list.
             */
            if (NCSCC_RC_SUCCESS != avd_avnd_struc_rmv_nodeid(cb, avnd_ptr))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(avnd->node_info.nodeId);
               return NCSCC_RC_FAILURE;
            }

            if (NCSCC_RC_SUCCESS != avd_avnd_struc_del(cb, avnd_ptr))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(avnd->node_info.nodeId);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(avnd->node_info.nodeId);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_sg_data
 *
 * Purpose:  Add new SG entry if action is ADD, remove SG from the tree if 
 *           action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        sg - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_ckpt_add_rmv_updt_sg_data(AVD_CL_CB *cb, AVD_SG *sg, 
                                      NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SG     *sg_ptr;
   
   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_sg_data");

   sg_ptr = avd_sg_struc_find(cb, sg->name_net, FALSE);

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
      {
         if (NULL == sg_ptr)
         {
            /*
             * add new SG entry into pat tree.
             */
            if (AVD_SG_NULL == (sg_ptr = 
               avd_sg_struc_crt(cb, sg->name_net, TRUE)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }

         if(sg_ptr->row_status != NCS_ROW_ACTIVE)
         {
            /* Update one time update fields */
            sg_ptr->su_redundancy_model = sg->su_redundancy_model;
            sg_ptr->pref_num_active_su = sg->pref_num_active_su;
            sg_ptr->pre_num_standby_su = sg->pre_num_standby_su;
            sg_ptr->pref_num_assigned_su = sg->pref_num_assigned_su;
            sg_ptr->su_max_active = sg->su_max_active;
            sg_ptr->su_max_standby = sg->su_max_standby;
            sg_ptr->sg_ncs_spec = sg->sg_ncs_spec;
            sg_ptr->row_status = sg->row_status;
         }
      }

      /* 
       * Don't break...continue updating data to this SG 
       * as done during normal update request.
       */

   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL != sg_ptr)
         {
           /*
            * Update all the data. Except SG name which would have got
            * updated with ADD.
            */
            sg_ptr->su_failback = sg->su_failback;
            sg_ptr->pref_num_insvc_su = sg->pref_num_insvc_su;
            sg_ptr->admin_state = sg->admin_state;
            sg_ptr->adjust_state = sg->adjust_state;
            sg_ptr->comp_restart_prob = sg->comp_restart_prob;
            sg_ptr->comp_restart_max = sg->comp_restart_max;
            sg_ptr->su_restart_prob = sg->su_restart_prob;
            sg_ptr->su_restart_max = sg->su_restart_max;
            sg_ptr->su_assigned_num = sg->su_assigned_num;
            sg_ptr->su_spare_num = sg->su_spare_num;
            sg_ptr->su_uninst_num = sg->su_uninst_num;
            sg_ptr->sg_fsm_state = sg->sg_fsm_state;
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != sg_ptr)
         {
            /*
             * Remove this SG from the list.
             */
            if (NCSCC_RC_SUCCESS != avd_sg_struc_del(cb, sg_ptr))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_su_data
 *
 * Purpose:  Add new SU entry if action is ADD, remove SU from the tree if 
 *           action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        su - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_ckpt_add_rmv_updt_su_data(AVD_CL_CB *cb, AVD_SU *su, 
                                      NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU     *su_ptr;
   SaNameT       temp_name;
   su_ptr = avd_su_struc_find(cb, su->name_net, FALSE);
   
   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_su_data");

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
      {
         if (NULL == su_ptr)
         {
            /*
             * add new SU entry into pat tree.
             */
            if (AVD_SU_NULL == (su_ptr = 
               avd_su_struc_crt(cb, su->name_net, TRUE)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }

         if(su_ptr->row_status != NCS_ROW_ACTIVE)
         {

            /* Update one time update fields */
            su_ptr->rank = su->rank;
            su_ptr->num_of_comp = su->num_of_comp;
            su_ptr->row_status = su->row_status;
            su_ptr->su_is_external = su->su_is_external;
            su_ptr->sg_name.length = su->sg_name.length;
            memcpy(su_ptr->sg_name.value, 
                   su->sg_name.value, su->sg_name.length);

            /* Add to SG list */
            if ((su_ptr->sg_of_su = avd_sg_struc_find(cb,
                  su_ptr->sg_name,TRUE)) == AVD_SG_NULL)
            {
               su_ptr->row_status = NCSMIB_ROWSTATUS_NOTINSERVICE;
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }

            avd_su_add_sg_list(cb, su_ptr);

          if(FALSE == su_ptr->su_is_external)
          {
            memset(&temp_name, '\0', sizeof(SaNameT));
            /* Add to ND list */
            /* First get the node name */
            avsv_cpy_node_DN_from_DN(&temp_name, &su_ptr->name_net);

            if ((su_ptr->su_on_node = avd_avnd_struc_find(cb, temp_name)) 
               == AVD_AVND_NULL)
            {
               su_ptr->row_status = NCSMIB_ROWSTATUS_NOTINSERVICE;
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }

            /* add to the list of AVND */ 
            if(su_ptr->sg_of_su->sg_ncs_spec == TRUE)
            {
               su_ptr->avnd_list_su_next = su_ptr->su_on_node->list_of_ncs_su;
               su_ptr->su_on_node->list_of_ncs_su = su_ptr;
            }
            else
            {
               su_ptr->avnd_list_su_next = su_ptr->su_on_node->list_of_su;
               su_ptr->su_on_node->list_of_su = su_ptr;
            }
          }/* if(FALSE == su->su_is_external) */
          else
          {
            if(NULL == cb->ext_comp_info.ext_comp_hlt_check)
            {
              /* This is an external SU and we need to create the
                 supporting info.*/
               cb->ext_comp_info.ext_comp_hlt_check =
                                        m_MMGR_ALLOC_AVD_AVND;
               if(NULL == cb->ext_comp_info.ext_comp_hlt_check)
               {
                 avd_su_del_sg_list(cb,su_ptr);
                 return NCSCC_RC_INV_VAL;
               }
               memset(cb->ext_comp_info.ext_comp_hlt_check, 0,
                            sizeof(AVD_AVND));
            } /* if(NULL == cb->ext_comp_info.ext_comp_hlt_check) */

               cb->ext_comp_info.local_avnd_node =
                  avd_avnd_struc_find_nodeid(cb, cb->node_id_avd);

               if(NULL == cb->ext_comp_info.local_avnd_node)
               {
                 m_AVD_PXY_PXD_ERR_LOG(
                "avsv_ckpt_add_rmv_updt_su_data:Local node unavailable:SU Name,node id are",
                 &su_ptr->name_net,cb->node_id_avd,0,0,0);
                 avd_su_del_sg_list(cb,su_ptr);
                 return NCSCC_RC_INV_VAL;
               }
          }/* else of if(FALSE == su_ptr->su_is_external) */

            if(su->row_status == NCS_ROW_ACTIVE)
            {
               /* add a row to the SG-SU Rank table */
               if(avd_sg_su_rank_add_row(cb, su_ptr) == AVD_SG_SU_RANK_NULL)
               {
                  su_ptr->row_status = NCSMIB_ROWSTATUS_NOTINSERVICE;
                  return NCSCC_RC_FAILURE;
               }
            }
         }
      }

      /* 
       * Don't break...continue updating data to this SU 
       * as done during normal update request.
       */

   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL != su_ptr)
         {
           /*
            * Update all the data. Except SU name which would have got
            * updated with ADD.
            */
            su_ptr->si_max_active = su->si_max_active;
            su_ptr->si_max_standby = su->si_max_standby;
            su_ptr->si_curr_active = su->si_curr_active;
            su_ptr->si_curr_standby = su->si_curr_standby;
            su_ptr->is_su_failover = su->is_su_failover;
            su_ptr->admin_state = su->admin_state;
            su_ptr->term_state = su->term_state;
            su_ptr->su_switch = su->su_switch;
            su_ptr->oper_state = su->oper_state;
            su_ptr->pres_state = su->pres_state;
            su_ptr->readiness_state = su->readiness_state;
            su_ptr->su_act_state = su->su_act_state;
            su_ptr->su_preinstan = su->su_preinstan;
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != su_ptr)
         {
           /* remove the SU from both the SG list and the
            * AVND list if present.
            */
            avd_su_del_sg_list(cb,su_ptr);
            avd_su_del_avnd_list(cb,su_ptr);

            /* remove the SU from SG-SU Rank table*/
            avd_sg_su_rank_del_row(cb, su_ptr);

            /*
             * Remove this SU from the list.
             */
            if (NCSCC_RC_SUCCESS != avd_su_struc_del(cb, su_ptr))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }

   return status;
}


/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_si_dep_data
 *
 * Purpose:  Add new SI_DEP entry if action is ADD, remove SI_DEP from the tree
 *           if action is to remove and update data if request is to update.
 *
 * Input: cb - CB pointer.
 *        si_dep - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 * 
\**************************************************************************/
uns32 avsv_ckpt_add_rmv_updt_si_dep_data(AVD_CL_CB *cb, AVD_SI_SI_DEP *si_dep,
                                         NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SI_SI_DEP *si_ptr_up;
   AVD_SI *spons_si = NULL;
   AVD_SI *dep_si = NULL;
   
   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_si_dep_data");

   si_ptr_up = avd_si_si_dep_find(cb, &si_dep->indx_mib, TRUE);

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
      {
         if (NULL == si_ptr_up)
         {
            /* Add new SI entry into pat tree. */
            if (NULL == (si_ptr_up = 
               avd_si_si_dep_struc_crt(cb, &si_dep->indx_mib)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }

         if (si_ptr_up->row_status != NCS_ROW_ACTIVE)
         {
            /* Update one time update fields */
            si_ptr_up->tolerance_time = si_dep->tolerance_time;
            si_ptr_up->row_status = si_dep->row_status;
         }

         /* Sponsor SI row Status should be active, if not return error*/
         if(((spons_si = avd_si_struc_find(cb, si_dep->indx_mib.si_name_prim,FALSE)) == AVD_SI_NULL) ||
             (spons_si->row_status != NCS_ROW_ACTIVE))
         {
            return NCSCC_RC_FAILURE;
         }

         /* Dependent SI row Status should be active, if not return error*/
         if(((dep_si = avd_si_struc_find(cb, si_dep->indx_mib.si_name_sec,FALSE)) == AVD_SI_NULL) ||
             (dep_si->row_status != NCS_ROW_ACTIVE) )
         {
            return NCSCC_RC_FAILURE;
         }

         /* Add the spons si to the spons_si_list of dependent SI */
         if (avd_si_dep_spons_list_add(cb, dep_si, spons_si) != NCSCC_RC_SUCCESS)
         {
            /* Delete the created SI dep records */
            avd_si_si_dep_del_row(cb, si_ptr_up);

            return NCSCC_RC_FAILURE;
         }
      }

   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL != si_ptr_up)
         {
           /* Update all the data. Except SU name which would have got
            * updated with ADD.
            */
            si_ptr_up->tolerance_time = si_dep->tolerance_time;
            si_ptr_up->row_status = si_dep->row_status;
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != si_ptr_up)
         {
            /* Delete the sponsor node from sponsor list of dependent SI */
            avd_si_dep_spons_list_del(cb, si_ptr_up);

            if (avd_si_si_dep_del_row(cb, si_ptr_up) != NCSCC_RC_SUCCESS)
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }

   return status;
}


/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_si_data
 *
 * Purpose:  Add new SI entry if action is ADD, remove SI from the tree if 
 *           action is to remove and update data if request is to update.
 *
 * Input: cb - CB pointer.
 *        si - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 * 
\**************************************************************************/
uns32  avsv_ckpt_add_rmv_updt_si_data(AVD_CL_CB *cb, AVD_SI *si,
                                      NCS_MBCSV_ACT_TYPE action)
{
   uns32  status = NCSCC_RC_SUCCESS;
   AVD_SI *si_ptr_up;
   
   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_si_data");

   si_ptr_up = avd_si_struc_find(cb, si->name_net, FALSE);

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
      {
         if (NULL == si_ptr_up)
         {
            /*
             * add new SI entry into pat tree.
             */
            if (AVD_SI_NULL == (si_ptr_up = 
               avd_si_struc_crt(cb, si->name_net, TRUE)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }

         if (si_ptr_up->row_status != NCS_ROW_ACTIVE)
         {
            /* Update one time update fields */
            si_ptr_up->rank = si->rank;
            si_ptr_up->row_status = si->row_status;
            si_ptr_up->sg_name.length = si->sg_name.length;
            memcpy(si_ptr_up->sg_name.value, 
                   si->sg_name.value, si->sg_name.length);

            /* Add to SG list */
            if ((si_ptr_up->sg_of_si = avd_sg_struc_find(cb,
               si_ptr_up->sg_name,TRUE)) == AVD_SG_NULL)
            {
               si_ptr_up->row_status = NCSMIB_ROWSTATUS_NOTINSERVICE;
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }

            /* add to the list of SG  */
            avd_si_add_sg_list(cb, si_ptr_up);
         
            /* add to SG-SI Rank Table */
            if(avd_sg_si_rank_add_row(cb, si) == AVD_SG_SI_RANK_NULL)
            {
               si_ptr_up->row_status = NCSMIB_ROWSTATUS_NOTINSERVICE;
               return NCSCC_RC_FAILURE;
            }
         }
      }

      /* 
       * Don't break...continue updating data to this AVND 
       * as done during normal update request.
       */

   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL != si_ptr_up)
         {
           /*
            * Update all the data. Except SU name which would have got
            * updated with ADD.
            */
            si_ptr_up->su_config_per_si = si->su_config_per_si;
            si_ptr_up->su_curr_active = si->su_curr_active;
            si_ptr_up->su_curr_standby = si->su_curr_standby;
            si_ptr_up->max_num_csi = si->max_num_csi;
            si_ptr_up->si_switch = si->si_switch;
            si_ptr_up->admin_state = si->admin_state;
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != si_ptr_up)
         {
           /* remove the SI from the SG list.
            */
            avd_si_del_sg_list(cb,si_ptr_up);

            /* Delete the SG-SI Rank entry from SG-SI Rank Table*/
            avd_sg_si_rank_del_row(cb, si_ptr_up);

            /* Delete the SI-SI dep records corresponding to this SI */
            avd_si_dep_delete(cb, si_ptr_up);

            /*
             * Remove this SI from the list.
             */
            if (NCSCC_RC_SUCCESS != avd_si_struc_del(cb, si_ptr_up))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_add_rmv_su_oper_list
 *
 * Purpose:  Add or Remove SU operation list.
 *
 * Input: cb  - CB pointer.
 *        dec - Data to be decoded
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_decode_add_rmv_su_oper_list(AVD_CL_CB *cb, AVD_SU *su_ptr,
                                        NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU  *su_struct;
   
   m_AVD_LOG_FUNC_ENTRY("avsv_decode_add_rmv_su_oper_list");

   su_struct = avd_su_struc_find(cb, su_ptr->name_net, FALSE);

   if (AVD_SU_NULL != su_struct)
   {
      if (NCS_MBCSV_ACT_ADD == action)
         avd_sg_su_oper_list_add(cb, su_struct, TRUE);
      else
         avd_sg_su_oper_list_del(cb, su_struct, TRUE);
   }
   else
   {
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_update_sg_admin_si
 *
 * Purpose:  Update SG Admin SI.
 *
 * Input: cb  - CB pointer.
 *        dec - Data to be decoded
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_update_sg_admin_si(AVD_CL_CB *cb, NCS_UBAID   *uba,
                               NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SI  *si, *si_ptr_up;
   AVD_SI   dec_si;
   EDU_ERR       ederror = 0;
   
   m_AVD_LOG_FUNC_ENTRY("avsv_update_sg_admin_si");

   si = &dec_si;
   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_si, uba,
      EDP_OP_TYPE_DEC, (AVD_SI**)&si, &ederror, 1, 1);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   si_ptr_up = avd_si_struc_find(cb, si->name_net, FALSE);
   if(si_ptr_up == AVD_SI_NULL)
   {
      /* log information error */
      m_AVD_LOG_INVALID_VAL_FATAL(si_ptr_up);
      return NCSCC_RC_FAILURE;
   }

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
      si_ptr_up->sg_of_si->admin_si = si_ptr_up;
      break;

   case NCS_MBCSV_ACT_RMV:
      si_ptr_up->sg_of_si->admin_si = NULL;
      break;

   case NCS_MBCSV_ACT_UPDATE:
   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_updt_su_si_rel
 *
 * Purpose:  Add new SU_SI_REL entry if action is ADD, remove SU_SI_REL from 
 *           the tree if action is to remove and update data if request is to 
 *           update.
 *
 * Input: cb  - CB pointer.
 *        si - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_updt_su_si_rel(AVD_CL_CB *cb, AVSV_SU_SI_REL_CKPT_MSG  *su_si_ckpt,
                           NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU_SI_REL *su_si_rel_ptr;
   AVD_SU   *su_ptr;
   AVD_SI   *si_ptr_up;
   
   m_AVD_LOG_FUNC_ENTRY("avsv_updt_su_si_rel");

   su_si_rel_ptr = avd_susi_struc_find(cb, su_si_ckpt->su_name_net, 
      su_si_ckpt->si_name_net, FALSE);

   su_ptr = avd_su_struc_find(cb, su_si_ckpt->su_name_net, FALSE);
   si_ptr_up = avd_si_struc_find(cb, su_si_ckpt->si_name_net, FALSE);

   if ((NULL == su_ptr) || (NULL == si_ptr_up))
   {
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
      {
         if (NULL == su_si_rel_ptr)
         {
            if (NCSCC_RC_SUCCESS != avd_new_assgn_susi(cb, su_ptr, si_ptr_up, 
               su_si_ckpt->state, TRUE, &su_si_rel_ptr))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      /* 
       * Don't break...continue updating data of this SU SI Relation 
       * as done during normal update request.
       */

   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL != su_si_rel_ptr)
         {
            su_si_rel_ptr->fsm = su_si_ckpt->fsm;
            su_si_rel_ptr->state = su_si_ckpt->state;
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != su_si_rel_ptr)
         {
            avd_susi_struc_del(cb, su_si_rel_ptr, TRUE);
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;


   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_comp_data
 *
 * Purpose:  Add new COMP entry if action is ADD, remove COMP from the tree if 
 *           action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        comp - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_ckpt_add_rmv_updt_comp_data(AVD_CL_CB *cb, AVD_COMP *comp, 
                                        NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_COMP     *comp_ptr;
   SaNameT       temp_name;
   
   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_comp_data");

   comp_ptr = avd_comp_struc_find(cb, comp->comp_info.name_net, FALSE);

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
      {
         if (NULL == comp_ptr)
         {
            /*
             * add new COMP entry into pat tree.
             */
            if (AVD_COMP_NULL == (comp_ptr = 
               avd_comp_struc_crt(cb, comp->comp_info.name_net, TRUE)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }

         if(comp_ptr->row_status != NCS_ROW_ACTIVE)
         {

            /* Update one time update fields */
            comp_ptr->comp_info.cap = comp->comp_info.cap;
            comp_ptr->comp_info.category = comp->comp_info.category;
            comp_ptr->row_status = comp->row_status;

            memset(&temp_name, '\0', sizeof(SaNameT));
            /* get the SU name*/
            avsv_cpy_SU_DN_from_DN(&temp_name, &comp_ptr->comp_info.name_net);

            if(temp_name.length == 0)
            {
               /* log information error */
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }

            /* check that the SU is present and is active.
            */

            if ((comp_ptr->su = avd_su_struc_find(cb,temp_name,TRUE)) 
                                          == AVD_SU_NULL)
            {
               comp_ptr->row_status = NCSMIB_ROWSTATUS_NOTINSERVICE;
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }

            /* add to the list of SU  */
            comp_ptr->su_comp_next = comp_ptr->su->list_of_comp;
            comp_ptr->su->list_of_comp = comp_ptr;

            /* update the SU information about the component */
            comp_ptr->su->curr_num_comp ++;
         }

      }

      /* 
       * Don't break...continue updating data to this COMP 
       * as done during normal update request.
       */

   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL != comp_ptr)
         {
           /*
            * Update all the data. Except COMP name which would have got
            * updated with ADD.
            */
            comp_ptr->comp_info.init_len = comp->comp_info.init_len;
            memcpy(comp_ptr->comp_info.init_info,
                         comp->comp_info.init_info, 
                         AVSV_MISC_STR_MAX_SIZE);
            comp_ptr->comp_info.init_time = comp->comp_info.init_time;
            comp_ptr->comp_info.term_len = comp->comp_info.term_len;
            memcpy(comp_ptr->comp_info.term_info, 
                         comp->comp_info.term_info,
                         AVSV_MISC_STR_MAX_SIZE);
            comp_ptr->comp_info.term_time = comp->comp_info.term_time;
            comp_ptr->comp_info.clean_len = comp->comp_info.clean_len;
            memcpy(comp_ptr->comp_info.clean_info, 
                         comp->comp_info.clean_info,
                         AVSV_MISC_STR_MAX_SIZE);
            comp_ptr->comp_info.clean_time = comp->comp_info.clean_time;
            comp_ptr->comp_info.amstart_len = comp->comp_info.amstart_len;
            memcpy(comp_ptr->comp_info.amstart_info,
                         comp->comp_info.amstart_info,
                         AVSV_MISC_STR_MAX_SIZE);
            comp_ptr->comp_info.amstart_time = comp->comp_info.amstart_time;
            comp_ptr->comp_info.amstop_len = comp->comp_info.amstop_len;
            memcpy(comp_ptr->comp_info.amstop_info, 
                         comp->comp_info.amstop_info,
                         AVSV_MISC_STR_MAX_SIZE);
            comp_ptr->comp_info.am_enable = comp->comp_info.am_enable;
            comp_ptr->comp_info.amstop_time = comp->comp_info.amstop_time;
            comp_ptr->comp_info.inst_level = comp->comp_info.inst_level;
            comp_ptr->comp_info.def_recvr = comp->comp_info.def_recvr;
            comp_ptr->comp_info.max_num_inst = comp->comp_info.max_num_inst;
            comp_ptr->comp_info.comp_restart = comp->comp_info.comp_restart;
            comp_ptr->proxy_comp_name_net = comp->proxy_comp_name_net;
            comp_ptr->inst_retry_delay = comp->inst_retry_delay;
            comp_ptr->nodefail_cleanfail = comp->nodefail_cleanfail;
            comp_ptr->max_num_inst_delay = comp->max_num_inst_delay;
            comp_ptr->comp_info.max_num_amstart = comp->comp_info.max_num_amstart;
            comp_ptr->comp_info.max_num_amstop = comp->comp_info.max_num_amstop;
            comp_ptr->max_num_csi_actv = comp->max_num_csi_actv;
            comp_ptr->max_num_csi_stdby = comp->max_num_csi_stdby;
            comp_ptr->curr_num_csi_actv = comp->curr_num_csi_actv;
            comp_ptr->curr_num_csi_stdby = comp->curr_num_csi_stdby;
            comp_ptr->oper_state = comp->oper_state;
            comp_ptr->pres_state = comp->pres_state;
            comp_ptr->restart_cnt = comp->restart_cnt;
            comp_ptr->comp_info.terminate_callback_timeout = comp->comp_info.terminate_callback_timeout;
            comp_ptr->comp_info.csi_set_callback_timeout = comp->comp_info.csi_set_callback_timeout;
            comp_ptr->comp_info.quiescing_complete_timeout = comp->comp_info.quiescing_complete_timeout;
            comp_ptr->comp_info.csi_rmv_callback_timeout = comp->comp_info.csi_rmv_callback_timeout;
            comp_ptr->comp_info.proxied_inst_callback_timeout = comp->comp_info.proxied_inst_callback_timeout;
            comp_ptr->comp_info.proxied_clean_callback_timeout = comp->comp_info.proxied_clean_callback_timeout;

         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != comp_ptr)
         {
           /* remove the COMP from SU list.
            */
            comp_ptr->su->curr_num_comp --;
            avd_comp_del_su_list(cb,comp_ptr);

            /*
             * Remove this COMP from the list.
             */
            if (NCSCC_RC_SUCCESS != avd_comp_struc_del(cb, comp_ptr))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }


   return status;
}

/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_csi_data
 *
 * Purpose:  Add new CSI entry if action is ADD, remove CSI from the tree if 
 *           action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        csi - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_ckpt_add_rmv_updt_csi_data(AVD_CL_CB *cb, AVD_CSI *csi, 
                                       NCS_MBCSV_ACT_TYPE action)
{
   uns32       status = NCSCC_RC_SUCCESS;
   AVD_CSI     *csi_ptr;
   SaNameT     temp_name;
   AVD_CSI_PARAM *i_csi_param = AVD_CSI_PARAM_NULL;
   AVD_CSI_PARAM *temp_csi_param = AVD_CSI_PARAM_NULL;
   
   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_csi_data");

   csi_ptr = avd_csi_struc_find(cb, csi->name_net, FALSE);

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
      {
         if (NULL == csi_ptr)
         {
            /*
             * add new CSI entry into pat tree.
             */
            if (AVD_CSI_NULL == (csi_ptr = 
               avd_csi_struc_crt(cb, csi->name_net, TRUE)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            if(csi_ptr->row_status != NCS_ROW_ACTIVE)
            {
               /*
               * Delete CSI param list.
               */
               i_csi_param = csi_ptr->list_param;
               while(i_csi_param != AVD_CSI_PARAM_NULL)
               {
                  temp_csi_param = i_csi_param;
                  i_csi_param = i_csi_param->param_next;
                  avd_csi_param_del(csi_ptr, temp_csi_param);
               }
            }
         }

         if(csi_ptr->row_status != NCS_ROW_ACTIVE)
         {

            /* Update one time update fields */
            csi_ptr->rank = csi->rank;
            csi_ptr->row_status = csi->row_status;
            csi_ptr->num_active_params = csi->num_active_params;
            csi_ptr->list_param = csi->list_param;

            csi_ptr->csi_type.length = csi->csi_type.length;
            memcpy(csi_ptr->csi_type.value, 
                   csi->csi_type.value, csi->csi_type.length);

            memset(&temp_name, '\0', sizeof(SaNameT));
            /* check that the SI is present and is active.
             */
            /* First get the SI name */
            avsv_cpy_SI_DN_from_DN(&temp_name, &csi->name_net);

            if(temp_name.length == 0)
            {
               /* log information error */
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               ncs_patricia_tree_del(&cb->csi_anchor,&csi_ptr->tree_node);
               m_MMGR_FREE_AVD_CSI(csi_ptr);
               return NCSCC_RC_INV_VAL;
            }

            /* 
             * Find the SI and link it to the structure.
            */
            if ((csi_ptr->si = avd_si_struc_find(cb, temp_name, TRUE)) 
                                                      == AVD_SI_NULL)
            {
               /* log information error */
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               ncs_patricia_tree_del(&cb->csi_anchor,&csi_ptr->tree_node);
               m_MMGR_FREE_AVD_CSI(csi_ptr);
               return NCSCC_RC_INV_VAL;
            }

            /* add to the list of SI  */
            avd_csi_add_si_list(cb, csi_ptr);
 
            /* update the SI information about the CSI */
            csi_ptr->si->num_csi ++;
         }  
      }

      /* 
       * Don't break...continue updating data to this CSI 
       * as done during normal update request.
       */

   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL != csi_ptr)
         {
           /*
            * Update all the data. Except CSI name which would have got
            * updated with ADD.
            */
            /*csi_ptr->pg_track = csi->pg_track;*/
         }
         else
         {
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != csi_ptr)
         {
           /* remove the CSI.
            */
            avd_remove_csi(cb, csi_ptr);
         }
         else
         {
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }

   return status;
}


/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_hlt_data
 *
 * Purpose:  Add new HLT entry if action is ADD, remove HLT from the tree if 
 *           action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        csi - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_ckpt_add_rmv_updt_hlt_data(AVD_CL_CB *cb, AVD_HLT *hlt, 
                                       NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_HLT     *hlt_ptr = AVD_HLT_NULL;
   AVD_AVND    *node_on_hlt = AVD_AVND_NULL;
   AVD_HLT     *temp_hlt_chk = AVD_HLT_NULL;
   AVD_SU      *su = NULL;
   SaNameT     temp_name;
   
   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_hlt_data");

   hlt_ptr = avd_hlt_struc_find(cb, hlt->key_name);

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL == hlt_ptr)
         {   

            /*
             * add new HLT entry into pat tree.
             */
            if (AVD_HLT_NULL == (hlt_ptr = 
               avd_hlt_struc_crt(cb, hlt->key_name, TRUE)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }

         if(hlt_ptr->row_status != NCS_ROW_ACTIVE)
         {
            /* get the SU name*/
            memset(&temp_name, 0, sizeof(SaNameT));
            avsv_cpy_SU_DN_from_DN(&temp_name, &hlt->key_name.comp_name_net);

            if(temp_name.length == 0)
            {
               m_AVD_LOG_INVALID_VAL_FATAL(temp_name.length);
               return NCSCC_RC_INV_VAL;
            }

            if ((su = avd_su_struc_find(cb,temp_name,TRUE))
                                          == AVD_SU_NULL)
            {
               m_AVD_LOG_INVALID_VAL_FATAL(su);
               return NCSCC_RC_INV_VAL;
            }

            if(TRUE == su->su_is_external)
            {
              node_on_hlt = cb->ext_comp_info.ext_comp_hlt_check;
            }
            else
            {
              node_on_hlt =  avd_hlt_node_find(hlt->key_name.comp_name_net, cb);
            }

           if(node_on_hlt == AVD_AVND_NULL)
           {
              avd_hlt_struc_del(cb, hlt);
              return NCSCC_RC_FAILURE;
           }

           /* Add the HLT into list of hlt on the node */
           temp_hlt_chk = node_on_hlt->list_of_hlt;
           node_on_hlt->list_of_hlt = hlt_ptr;
           hlt_ptr->next_hlt_on_node = temp_hlt_chk;
         }
        
         
         /* Update one time update fields */
         /*hlt_ptr->key_name.key_len_net = hlt->key_name.key_len_net;*/
         hlt_ptr->period = hlt->period;
         hlt_ptr->max_duration = hlt->max_duration;
         hlt_ptr->row_status = hlt->row_status;

            
           
        }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != hlt_ptr)
         {
            /* get the SU name*/
            memset(&temp_name, 0, sizeof(SaNameT));
            avsv_cpy_SU_DN_from_DN(&temp_name, &hlt_ptr->key_name.comp_name_net);

            if(temp_name.length == 0)
            {
               m_AVD_LOG_INVALID_VAL_FATAL(temp_name.length);
               return NCSCC_RC_INV_VAL;
            }

            if ((su = avd_su_struc_find(cb,temp_name,TRUE))
                                          == AVD_SU_NULL)
            {
               m_AVD_LOG_INVALID_VAL_FATAL(su);
               return NCSCC_RC_INV_VAL;
            }

            if(TRUE == su->su_is_external)
            {
              node_on_hlt = cb->ext_comp_info.ext_comp_hlt_check;
            }
            else
            {
              node_on_hlt =  avd_hlt_node_find(hlt_ptr->key_name.comp_name_net, cb);
            }

            if(node_on_hlt != AVD_AVND_NULL)
            {
               avd_hlt_node_del_hlt_from_list(hlt_ptr, node_on_hlt);
            }
            /*
             * Remove this HLT from the list.
             */
            if (NCSCC_RC_SUCCESS != avd_hlt_struc_del(cb, hlt_ptr))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }


   return status;
}


/****************************************************************************\
 * Function: avd_data_clean_up
 *
 * Purpose:  Function is used for cleaning the entire AVD data structure.
 *           Will be called from the standby AVD on failure of warm sync.
 *
 * Input: cb  - CB pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avd_data_clean_up(AVD_CL_CB *cb)
{
   AVD_HLT   *hlt_chk = AVD_HLT_NULL;
   AVSV_HLT_KEY hlt_name;

   AVD_SU        *su;
   SaNameT       su_name;
   /*AVD_SU_SI_REL *rel;*/

   AVD_CSI       *csi;
   SaNameT       csi_name;

   AVD_COMP      *comp;
   SaNameT       comp_name;

   SaNameT        sg_name_net;
   AVD_SG         *sg;

   AVD_SI        *si;
   SaNameT       si_name;

   SaClmNodeIdT  node_id = 0;
   AVD_AVND      *avnd;
 
   AVD_SUS_PER_SI_RANK *su_si_rank = AVD_SU_PER_SI_RANK_NULL;
   AVD_SUS_PER_SI_RANK_INDX su_si_rank_indx;

   AVD_COMP_CS_TYPE  *comp_cs = AVD_COMP_CS_TYPE_NULL;
   AVD_COMP_CS_TYPE_INDX comp_cs_indx;

   AVD_CS_TYPE_PARAM *cs_param = AVD_CS_TYPE_PARAM_NULL;
   AVD_CS_TYPE_PARAM_INDX cs_param_indx;
   
   m_AVD_LOG_FUNC_ENTRY("avd_data_clean_up");

   /* 
    * Walk through the entire HLT list.
    * Delete all HLT entries.
    */
   memset(&hlt_name, '\0', sizeof(AVSV_HLT_KEY));
   for (hlt_chk = avd_hlt_struc_find_next(cb, hlt_name); hlt_chk != NULL;
        hlt_chk = avd_hlt_struc_find_next(cb, hlt_name))
   {
      /*
       * Remove this HLT from the list.
       */
      if (NCSCC_RC_SUCCESS != avd_hlt_struc_del(cb, hlt_chk))
         return NCSCC_RC_FAILURE;
   }


   /* 
    * Walk through the entire SU list and find SU_SI_REL.
    * Delete all SU SI relationship entries.
    */
   su_name.length = 0;
   for (su = avd_su_struc_find_next(cb, su_name, FALSE); su != NULL;
        su = avd_su_struc_find_next(cb, su_name, FALSE))
   {
      while (su->list_of_susi != AVD_SU_SI_REL_NULL)
      {
         avd_compcsi_list_del(cb,su->list_of_susi, TRUE);
         avd_susi_struc_del(cb, su->list_of_susi, TRUE);
      }
      su_name = su->name_net;
   }
   
   /* 
    * Walk through the entire CSI list
    * Delete all CSI entries.
    */
   csi_name.length = 0;
   for (csi = avd_csi_struc_find_next(cb, csi_name, FALSE); csi != NULL;
        csi = avd_csi_struc_find_next(cb, csi_name, FALSE))
   {
      csi_name = csi->name_net;
      avd_remove_csi(cb, csi);
   }

   /* 
    * Walk through the entire COMP list.
    * Delete all component entries.
    */
   comp_name.length = 0;
   for (comp = avd_comp_struc_find_next(cb, comp_name, FALSE); comp != NULL;
        comp = avd_comp_struc_find_next(cb, comp_name, FALSE))
   {
      comp_name = comp->comp_info.name_net;
      /* remove the COMP from SU list.
       */

      if(comp->su)
         comp->su->curr_num_comp --;

      avd_comp_del_su_list(cb,comp);

      /*
       * Remove this COMP from the list.
       */
      if (NCSCC_RC_SUCCESS != avd_comp_struc_del(cb, comp))
         return NCSCC_RC_FAILURE;
   }

   /*
    * Delete entire operation SU's.
    */
   su_name.length = 0;
   for (su = avd_su_struc_find_next(cb, su_name, FALSE); su != NULL;
        su = avd_su_struc_find_next(cb, su_name, FALSE))
   {
      /* HACK FOR TESTING SHULD BE CHANGED LATER */
      if(su->sg_of_su == 0)
      {
         su_name = su->name_net;
         continue;
      }

      if(su->sg_of_su->su_oper_list.su != AVD_SU_NULL)
         avd_sg_su_oper_list_del(cb, su, TRUE);

      su_name = su->name_net;
   }

   /* 
    * Walk through the entire SI list
    * Delete all SI entries.
    */
   si_name.length = 0;
   for (si = avd_si_struc_find_next(cb, si_name, FALSE); si != NULL;
        si = avd_si_struc_find_next(cb, si_name, FALSE))
   {
      si_name = si->name_net;
      /* remove the SI from the SG list.
       */
       avd_si_del_sg_list(cb,si);

       /* remove the SG-SI Rank entry */
       avd_sg_si_rank_del_row(cb, si);

       /*
        * Remove this SI from the list.
        */
       if (NCSCC_RC_SUCCESS != avd_si_struc_del(cb, si))
          return NCSCC_RC_FAILURE;
   }

   /* 
    * Walk through the entire SU list..
    * Delete all SU entries.
    */
   su_name.length = 0;
   for (su = avd_su_struc_find_next(cb, su_name, FALSE); su != NULL;
        su = avd_su_struc_find_next(cb, su_name, FALSE))
   {
      su_name = su->name_net;
      /* remove the SU from both the SG list and the
       * AVND list if present.
       */
      avd_su_del_sg_list(cb,su);
      avd_su_del_avnd_list(cb,su);

      /* remove the SG-SU Rank entry */
      avd_sg_su_rank_del_row(cb, su);

      /*
       * Remove this SU from the list.
       */
      if (NCSCC_RC_SUCCESS != avd_su_struc_del(cb, su))
         return NCSCC_RC_FAILURE;
   }

   /* 
    * Walk through the entire SG list
    * Delete all the SG entries.
    */
   sg_name_net.length = 0;
   for (sg = avd_sg_struc_find_next(cb, sg_name_net,FALSE);
      sg != AVD_SG_NULL;
      sg = avd_sg_struc_find_next(cb, sg_name_net,FALSE))
   {
      sg_name_net = sg->name_net;
      /*
       * Remove this SG from the list.
       */
      if (NCSCC_RC_SUCCESS != avd_sg_struc_del(cb, sg))
         return NCSCC_RC_FAILURE;
   }

   /* 
    * Walk through the entire AVND list.
    * Delete all the AVND entries.
    */
   node_id = 0;
   while (NULL != (avnd = 
      (AVD_AVND *)ncs_patricia_tree_getnext(&cb->avnd_anchor,(uns8*)&node_id)))
   {
      /*
       * Remove this AVND from the list.
       */
      if (NCSCC_RC_SUCCESS != avd_avnd_struc_rmv_nodeid(cb, avnd))
         return NCSCC_RC_FAILURE;

      if (NCSCC_RC_SUCCESS != avd_avnd_struc_del(cb, avnd))
         return NCSCC_RC_FAILURE;
   }

   /*
    * Walk through the entire SUS_PER_SI_RANK list.
    * Delete all entries.
    */
   memset(&su_si_rank_indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));
   for (su_si_rank = avd_sus_per_si_rank_struc_find_next(cb, su_si_rank_indx); su_si_rank != NULL;
        su_si_rank = avd_sus_per_si_rank_struc_find_next(cb, su_si_rank_indx))
   {
      /*
       * Remove this entry from the list.
       */
      if (NCSCC_RC_SUCCESS != avd_sus_per_si_rank_struc_del(cb, su_si_rank))
         return NCSCC_RC_FAILURE;
   }
 
   /*
    * Walk through the entire COMP_CS_TYPE list.
    * Delete all entries.
    */
   memset(&comp_cs_indx, '\0', sizeof(AVD_COMP_CS_TYPE_INDX));
   for (comp_cs = avd_comp_cs_type_struc_find_next(cb, comp_cs_indx); comp_cs != NULL;
        comp_cs = avd_comp_cs_type_struc_find_next(cb, comp_cs_indx))
   {
      /*
       * Remove this entry from the list.
       */
      if (NCSCC_RC_SUCCESS != avd_comp_cs_type_struc_del(cb, comp_cs))
         return NCSCC_RC_FAILURE;
   }

   /*
    * Walk through the entire CS_TYPE_PARAM list.
    * Delete all entries.
    */
   memset(&cs_param_indx, '\0', sizeof(AVD_CS_TYPE_PARAM_INDX));
   for (cs_param = avd_cs_type_param_struc_find_next(cb, cs_param_indx); cs_param != NULL;
        cs_param = avd_cs_type_param_struc_find_next(cb, cs_param_indx))
   {
      /*
       * Remove this entry from the list.
       */
      if (NCSCC_RC_SUCCESS != avd_cs_type_param_struc_del(cb, cs_param))
         return NCSCC_RC_FAILURE;
   }

   /* Reset the cold sync done counter */
   cb->synced_reo_type = 0;

   return NCSCC_RC_SUCCESS;
}

static uns32 avd_remove_csi(AVD_CL_CB *cb, AVD_CSI *csi_ptr)
{
   AVD_CSI_PARAM *i_csi_param = AVD_CSI_PARAM_NULL;
   AVD_CSI_PARAM *temp_csi_param = AVD_CSI_PARAM_NULL;
      
   m_AVD_LOG_FUNC_ENTRY("avd_remove_csi");
   
   /*
   * Delete CSI param list.
   */
   i_csi_param = csi_ptr->list_param;
   while(i_csi_param != AVD_CSI_PARAM_NULL)
   {
      temp_csi_param = i_csi_param;
      i_csi_param = i_csi_param->param_next;
      avd_csi_param_del(csi_ptr, temp_csi_param);
   }
   
   /* decrement the active csi number of this SI */
   csi_ptr->si->num_csi --;               
   
   /* 
    * remove the CSI from SI list.
    */
   avd_csi_del_si_list(cb,csi_ptr);

   /*
   * Remove this CSI from the list.
   */
   if (NCSCC_RC_SUCCESS != avd_csi_struc_del(cb, csi_ptr))
      return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_sus_per_si_rank_data
 *
 * Purpose:  Add new SUS_PER_SI_RANK entry if action is ADD, remove SUS_PER_SI_RANK
 *           from the tree if action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        su_si_rank - Decoded structure.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uns32  avsv_ckpt_add_rmv_updt_sus_per_si_rank_data(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK *su_si_rank,
                                       NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SUS_PER_SI_RANK *su_si = AVD_SU_PER_SI_RANK_NULL;

   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_sus_per_si_rank_data");

   su_si = avd_sus_per_si_rank_struc_find(cb,su_si_rank->indx);

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL == su_si)
         {
             
            /*
             * add new entry into pat tree.
             */
            if (AVD_SU_PER_SI_RANK_NULL == (su_si =
               avd_sus_per_si_rank_struc_crt(cb, su_si_rank->indx)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         
         if(su_si->row_status != NCS_ROW_ACTIVE)
         {
            /* Update one time update fields */
            su_si->row_status = su_si_rank->row_status;
            su_si->su_name.length = su_si_rank->su_name.length;
            memcpy(su_si->su_name.value,
                   su_si_rank->su_name.value, su_si->su_name.length);
         }

      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != su_si)
         {
            /*
             * Remove this entry from the list.
             */
            if (NCSCC_RC_SUCCESS != avd_sus_per_si_rank_struc_del(cb, su_si))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }


   return status;
}


/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_comp_cs_type_data
 *
 * Purpose:  Add new COMP_CS_TYPE entry if action is ADD, remove COMP_CS_TYPE 
 *           from the tree if action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        comp_cs_type - Decoded structure.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uns32  avsv_ckpt_add_rmv_updt_comp_cs_type_data(AVD_CL_CB *cb, AVD_COMP_CS_TYPE *comp_cs_type,
                                       NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_COMP_CS_TYPE *comp_cs = AVD_COMP_CS_TYPE_NULL;

   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_comp_cs_type_data");

   comp_cs = avd_comp_cs_type_struc_find(cb, comp_cs_type->indx);

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL == comp_cs)
         {

            /*
             * add new entry into pat tree.
             */
            if (AVD_COMP_CS_TYPE_NULL == (comp_cs =
               avd_comp_cs_type_struc_crt(cb, comp_cs_type->indx)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }

         /* Update one time update fields */
         comp_cs->row_status = comp_cs_type->row_status;
            
      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != comp_cs)
         {
            /*
             * Remove this entry from the list.
             */
            if (NCSCC_RC_SUCCESS != avd_comp_cs_type_struc_del(cb, comp_cs))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }


   return status;
}


/****************************************************************************\
 * Function: avsv_ckpt_add_rmv_updt_cs_type_param_data
 *
 * Purpose:  Add new CS_TYPE_PARAM entry if action is ADD, remove CS_TYPE_PARAM
 *           from the tree if action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        cs_type_param - Decoded structure.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uns32  avsv_ckpt_add_rmv_updt_cs_type_param_data(AVD_CL_CB *cb, AVD_CS_TYPE_PARAM *cs_type_param,
                                       NCS_MBCSV_ACT_TYPE action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_CS_TYPE_PARAM *cs_param = AVD_CS_TYPE_PARAM_NULL;

   m_AVD_LOG_FUNC_ENTRY("avsv_ckpt_add_rmv_updt_cs_type_param_data");

   cs_param = avd_cs_type_param_struc_find(cb, cs_type_param->indx);

   switch(action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      {
         if (NULL == cs_param)
         {

            /*
             * add new entry into pat tree.
             */
            if (AVD_CS_TYPE_PARAM_NULL == (cs_param =
               avd_cs_type_param_struc_crt(cb, cs_type_param->indx)))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }

         /* Update one time update fields */
         cs_param->row_status = cs_type_param->row_status;

      }
      break;

   case NCS_MBCSV_ACT_RMV:
      {
         if (NULL != cs_param)
         {
            /*
             * Remove this entry from the list.
             */
            if (NCSCC_RC_SUCCESS != avd_cs_type_param_struc_del(cb, cs_param))
            {
               m_AVD_LOG_INVALID_VAL_FATAL(action);
               return NCSCC_RC_FAILURE;
            }
         }
         else
         {
            m_AVD_LOG_INVALID_VAL_FATAL(action);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(action);
      return NCSCC_RC_FAILURE;
   }


   return status;
}
