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
  MODULE NAME: AVD_TRAP.C 

..............................................................................

  DESCRIPTION:   This file contains routines to generate traps .

******************************************************************************/

#include "avd.h"

static SaEvtEventPatternT amf_traps_pattern_array[AVSV_TRAP_PATTERN_ARRAY_LEN] = {
       {10, 10, (SaUint8T *) "AVSV_TRAPS"},
       {17, 17, (SaUint8T *) "SAF_AMF_MIB_TRAPS"}
};

static SaEvtEventPatternT clm_traps_pattern_array[AVSV_TRAP_PATTERN_ARRAY_LEN] = {
       {10, 10, (SaUint8T *) "AVSV_TRAPS"},
       {17, 17, (SaUint8T *) "SAF_CLM_MIB_TRAPS"}
};

static SaEvtEventPatternT ncs_traps_pattern_array[AVSV_TRAP_PATTERN_ARRAY_LEN] = {
       {10, 10, (SaUint8T *) "AVSV_TRAPS"},
       {18, 18, (SaUint8T *) "NCS_AVSV_MIB_TRAPS"}
};


/*****************************************************************************
  Name          :  avd_amf_alarm_service_impaired_trap

  Description   :  This function generates a trap when the AMF service is not 
                   able to provide its service.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   err -    Error Code

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_amf_alarm_service_impaired_trap(AVD_CL_CB *avd_cb,
                                         SaAisErrorT err)
{
   NCS_TRAP           trap;
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_amf_alarm_service_impaired_trap");

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap.i_trap_id = saAmfAlarmServiceImpaired_ID;
   trap.i_inform_mgr = TRUE;

   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_OBJ;
   i_trap_varbind->i_param_val.i_param_id = saAmfErrorCode_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   i_trap_varbind->i_param_val.i_length = 4;
   i_trap_varbind->i_param_val.info.i_int = err;

   /* Add to the trap */
   trap.i_trap_vb = i_trap_varbind;

   status = avd_create_and_send_trap(avd_cb, &trap, &amf_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(trap.i_trap_vb);

   return status;
}


/*****************************************************************************
  Name          :  avd_gen_cluster_reset_trap

  Description   :  This function generates a cluster reset trap as designated 
                   component failed and a cluster reset recovery as recommended
                   by the component is being done.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   comp - Pointer to the AVD_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_cluster_reset_trap(AVD_CL_CB *avd_cb,
                                 AVD_COMP *comp)
{
   NCS_TRAP           trap;
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_gen_cluster_reset_trap");

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap.i_trap_id = saAmfAlarmClusterReset_ID;
   trap.i_inform_mgr = TRUE;

   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));
   
   inst_id_len = m_NCS_OS_NTOHS(comp->comp_info.name_net.length); 

   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_COMP;
   i_trap_varbind->i_param_val.i_param_id = saAmfCompName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = comp->comp_info.name_net.value;

   /* Prepare the instant ID from the comp name */
   m_NCS_OS_MEMSET(inst_id, 0, SA_MAX_NAME_LENGTH);

   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   
   inst_id[0]= inst_id_len;

   for(i = 0; i < inst_id_len; i++)
   {
      inst_id[i+1] = (uns32)(comp->comp_info.name_net.value[i]);
   }
  
   i_trap_varbind->i_idx.i_inst_ids = inst_id;
   

   /* Add to the trap */
   trap.i_trap_vb = i_trap_varbind;

   status = avd_create_and_send_trap(avd_cb, &trap, &amf_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(trap.i_trap_vb);

   return status;
}


/*****************************************************************************
  Name          :  avd_gen_node_admin_state_changed_trap

  Description   :  This function generates a node admin state changed trap

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   node - Pointer to the AVD_AVND struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_node_admin_state_changed_trap(AVD_CL_CB *avd_cb,
                                            AVD_AVND *node)
{
   NCS_TRAP           trap;
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *s_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_gen_node_admin_state_changed_trap");
 
   m_AVD_LOG_ADMIN_STATE_TRAPS(node->su_admin_state, &(node->node_info.nodeName));

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));
  
   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap.i_trap_id = saAmfStateChgNodeAdmin_ID;
   trap.i_inform_mgr = TRUE;

   inst_id_len = m_NCS_OS_NTOHS(node->node_info.nodeName.length);
   
   /* Prepare the instant ID from the node name */
   m_NCS_OS_MEMSET(inst_id, 0, SA_MAX_NAME_LENGTH);

   inst_id[0]= inst_id_len;

   for(i = 0; i < inst_id_len; i++)
   {
      inst_id[i+1] = (uns32)(node->node_info.nodeName.value[i]);
   }
   
   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_NODE;
   i_trap_varbind->i_param_val.i_param_id = saAmfNodeName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = node->node_info.nodeName.value;
   
   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   i_trap_varbind->i_idx.i_inst_ids = inst_id;

    /* allocate memory */
   s_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;
   if(s_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(s_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   s_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_NODE;
   s_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   s_trap_varbind->i_param_val.i_length = 4;
   s_trap_varbind->i_param_val.i_param_id = saAmfNodeAdminState_ID;
   s_trap_varbind->i_param_val.info.i_int = node->su_admin_state;

   s_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   s_trap_varbind->i_idx.i_inst_ids = inst_id;

   i_trap_varbind->next_trap_varbind = s_trap_varbind;

   /* Add to the trap */
   trap.i_trap_vb = i_trap_varbind;

   status = avd_create_and_send_trap(avd_cb, &trap, &amf_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
   m_MMGR_NCS_TRAP_VARBIND_FREE(s_trap_varbind);

   return status;
}


/*****************************************************************************
  Name          :  avd_gen_sg_admin_state_changed_trap

  Description   :  This function generates a sg admin state changed trap

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   sg - Pointer to the AVD_SG struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_sg_admin_state_changed_trap(AVD_CL_CB *avd_cb,
                                         AVD_SG *sg)
{
   NCS_TRAP           trap;
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *s_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_gen_sg_admin_state_changed_trap");
  
   m_AVD_LOG_ADMIN_STATE_TRAPS(sg->admin_state, &(sg->name_net));

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap.i_trap_id = saAmfStateChgSGAdmin_ID;
   trap.i_inform_mgr = TRUE;
   
   inst_id_len = m_NCS_OS_NTOHS(sg->name_net.length);

  /* Prepare the instant ID from the sg name */
   m_NCS_OS_MEMSET(inst_id, 0, SA_MAX_NAME_LENGTH);

   inst_id[0]= inst_id_len;

   for(i = 0; i < inst_id_len; i++)
   {
      inst_id[i+1] = (uns32)(sg->name_net.value[i]);
   }

   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));


   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SG;
   i_trap_varbind->i_param_val.i_param_id = saAmfSGName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = sg->name_net.value;

   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   i_trap_varbind->i_idx.i_inst_ids = inst_id;

    /* allocate memory */
   s_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if(s_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(s_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   s_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SG;
   s_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   s_trap_varbind->i_param_val.i_length = 4;
   s_trap_varbind->i_param_val.i_param_id = saAmfSGAdminState_ID;
   s_trap_varbind->i_param_val.info.i_int = sg->admin_state;

   s_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   s_trap_varbind->i_idx.i_inst_ids = inst_id;

   i_trap_varbind->next_trap_varbind = s_trap_varbind;

   /* Add to the trap */
   trap.i_trap_vb = i_trap_varbind;

   status = avd_create_and_send_trap(avd_cb, &trap, &amf_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
   m_MMGR_NCS_TRAP_VARBIND_FREE(s_trap_varbind);

   return status;
}



/*****************************************************************************
  Name          :  avd_gen_su_admin_state_changed_trap

  Description   :  This function generates a su admin state changed trap

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   su - Pointer to the AVD_SU struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_su_admin_state_changed_trap(AVD_CL_CB *avd_cb,
                                         AVD_SU *su)
{
   NCS_TRAP           trap;
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *s_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_gen_su_admin_state_changed_trap");

   m_AVD_LOG_ADMIN_STATE_TRAPS(su->admin_state, &(su->name_net));

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap.i_trap_id = saAmfStateChgSUAdmin_ID;
   trap.i_inform_mgr = TRUE;

   inst_id_len = m_NCS_OS_NTOHS(su->name_net.length);

  /* Prepare the instant ID from the su name */
   m_NCS_OS_MEMSET(inst_id, 0, SA_MAX_NAME_LENGTH);

   inst_id[0]= inst_id_len;

   for(i = 0; i < inst_id_len; i++)
   {
      inst_id[i+1] = (uns32)(su->name_net.value[i]);
   }

   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));


   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SU;
   i_trap_varbind->i_param_val.i_param_id = saAmfSUName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = su->name_net.value;

   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   i_trap_varbind->i_idx.i_inst_ids = inst_id;

    /* allocate memory */
   s_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if(s_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(s_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   s_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SU;
   s_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   s_trap_varbind->i_param_val.i_length = 4;
   s_trap_varbind->i_param_val.i_param_id = saAmfSUAdminState_ID;
   s_trap_varbind->i_param_val.info.i_int = su->admin_state;

   s_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   s_trap_varbind->i_idx.i_inst_ids = inst_id;

   i_trap_varbind->next_trap_varbind = s_trap_varbind;

   /* Add to the trap */
   trap.i_trap_vb = i_trap_varbind;

   status = avd_create_and_send_trap(avd_cb, &trap, &amf_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
   m_MMGR_NCS_TRAP_VARBIND_FREE(s_trap_varbind);

   return status;
}


/*****************************************************************************
  Name          :  avd_gen_si_unassigned_trap

  Description   :  This function generates a si unassigned trap

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   si - Pointer to the AVD_SI struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_si_unassigned_trap(AVD_CL_CB *avd_cb,
                                         AVD_SI *si)
{
   NCS_TRAP           trap;
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_gen_si_unassigned_trap");

  m_AVD_LOG_SI_UNASSIGNED_TRAP(&(si->name_net));

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap.i_trap_id = saAmfAlarmSIUnassigned_ID;
   trap.i_inform_mgr = TRUE;

   inst_id_len = m_NCS_OS_NTOHS(si->name_net.length);
   
   /* Prepare the instant ID from the si name */
   m_NCS_OS_MEMSET(inst_id, 0, SA_MAX_NAME_LENGTH);

   inst_id[0]= inst_id_len;

   for(i = 0; i < inst_id_len; i++)
   {
      inst_id[i+1] = (uns32)(si->name_net.value[i]);
   }

   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SI;
   i_trap_varbind->i_param_val.i_param_id = saAmfSIName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = si->name_net.value;

   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   i_trap_varbind->i_idx.i_inst_ids = inst_id;

   /* Add to the trap */
   trap.i_trap_vb = i_trap_varbind;

   status = avd_create_and_send_trap(avd_cb, &trap, &amf_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(trap.i_trap_vb);

   return status;
}


/*****************************************************************************
  Name          :  avd_gen_si_oper_state_chg_trap

  Description   :  This function generates a si oper state change trap.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   si - Pointer to the AVD_SI struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_si_oper_state_chg_trap(AVD_CL_CB *avd_cb,
                                     AVD_SI *si)
{
   NCS_TRAP           trap;

   m_AVD_LOG_FUNC_ENTRY("avd_gen_si_oper_state_chg_trap");

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap.i_trap_id = saAmfStateChgSIOper_ID;
   trap.i_inform_mgr = TRUE;

   return avd_populate_si_state_traps(avd_cb, si, &trap,TRUE);
}


/*****************************************************************************
  Name          :  avd_gen_si_admin_state_chg_trap

  Description   :  This function generates a si admin state change trap.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   si - Pointer to the AVD_SI struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_si_admin_state_chg_trap(AVD_CL_CB *avd_cb,
                                     AVD_SI *si)
{
   NCS_TRAP           trap;

   m_AVD_LOG_FUNC_ENTRY("avd_gen_si_admin_state_chg_trap");

   m_AVD_LOG_ADMIN_STATE_TRAPS(si->admin_state, &(si->name_net));

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap.i_trap_id = saAmfStateChgSIAdmin_ID;
   trap.i_inform_mgr = TRUE;

   return avd_populate_si_state_traps(avd_cb, si, &trap,FALSE);
}



/*****************************************************************************
  Name          :  avd_populate_si_state_traps

  Description   :  This function populate and send the si admin state and oper 
                   state realted traps.

  Arguments     :  avd_cb -    Pointer to the AVD_CL_CB structure
                   si -         Pointer to the AVD_SI structure
                   trap -       Pointer to the NCS_TRAP structure
                   isOper -     Flag

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         : Set isOper flag to true if this is saAmfStateChgSIOper trap
                  otherwise set it false if this is saAmfStateChgSIAdmin trap.
*****************************************************************************/
uns32 avd_populate_si_state_traps(AVD_CL_CB *avd_cb,
                                    AVD_SI *si,
                                    NCS_TRAP *trap,
                                    NCS_BOOL isOper)
{
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *s_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_populate_si_state_traps");

   /* Fill in the trap details */
   trap->i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap->i_inform_mgr = TRUE;

   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;
   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   inst_id_len = m_NCS_OS_NTOHS(si->name_net.length);

   /* Prepare the instant ID from the su name */
   m_NCS_OS_MEMSET(inst_id, 0, SA_MAX_NAME_LENGTH);

   inst_id[0]= inst_id_len;

   for(i = 0; i < inst_id_len; i++)
   {
      inst_id[i+1] = (uns32)(si->name_net.value[i]);
   }

   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SI;
   i_trap_varbind->i_param_val.i_param_id = saAmfSIName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = si->name_net.value;

   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   i_trap_varbind->i_idx.i_inst_ids = inst_id;

   /* Add to the trap */
   trap->i_trap_vb = i_trap_varbind;

   /* allocate memory */
   s_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;
   if(s_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(s_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   s_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SI;
   s_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   s_trap_varbind->i_param_val.i_length = 4;
   s_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   s_trap_varbind->i_idx.i_inst_ids = inst_id;

   i_trap_varbind->next_trap_varbind = s_trap_varbind;

   if(isOper)
   {
      s_trap_varbind->i_param_val.i_param_id = saAmfSIOperState_ID;

      if(si->list_of_sisu != AVD_SU_SI_REL_NULL)
      {
         m_AVD_LOG_OPER_STATE_TRAPS(NCS_OPER_STATE_ENABLE, &(si->name_net));
         s_trap_varbind->i_param_val.info.i_int = NCS_OPER_STATE_ENABLE;
      }
      else
      {
         m_AVD_LOG_OPER_STATE_TRAPS(NCS_OPER_STATE_DISABLE, &(si->name_net));
         s_trap_varbind->i_param_val.info.i_int = NCS_OPER_STATE_DISABLE;
      }
   }
   else
   {
      s_trap_varbind->i_param_val.i_param_id = saAmfSIAdminState_ID;
      s_trap_varbind->i_param_val.info.i_int = si->admin_state;
   }


   status = avd_create_and_send_trap(avd_cb, trap, &amf_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
   m_MMGR_NCS_TRAP_VARBIND_FREE(s_trap_varbind);

   return status;
}


/*****************************************************************************
  Name          :  avd_gen_su_ha_state_changed_trap

  Description   :  This function generates a su ha state changed trap

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   susi - Pointer to the AVD_SU_SI_REL struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  (NCS-AVSV-MIB Trap)
*****************************************************************************/
uns32 avd_gen_su_ha_state_changed_trap(AVD_CL_CB *avd_cb,
                                      AVD_SU_SI_REL *susi)
{
   m_AVD_LOG_FUNC_ENTRY("avd_gen_su_ha_state_changed_trap");

   return avd_fill_and_send_su_si_ha_state_trap(avd_cb, susi, FALSE);
}


/*****************************************************************************
  Name          :  avd_gen_su_si_assigned_trap

  Description   :  This function generates a su ha state changed trap only when a si 
                   assignment is done to a su.  

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   susi - Pointer to the AVD_SU_SI_REL struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :  (SAF-AMF-MIB Trap)
*****************************************************************************/
uns32 avd_gen_su_si_assigned_trap(AVD_CL_CB *avd_cb,
                                  AVD_SU_SI_REL *susi)
{
   m_AVD_LOG_FUNC_ENTRY("avd_gen_su_si_assigned_trap");
   return avd_fill_and_send_su_si_ha_state_trap(avd_cb, susi, TRUE);

}


/*****************************************************************************
  Name          :  avd_fill_and_send_su_si_ha_state_trap

  Description   :  This function use to fill details and sending a trap related to 
                   ha state

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   susi - Pointer to the AVD_SU_SI_REL struct
                   isAmfmibTrap - TRUE if this is AMF trap else FALE

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_fill_and_send_su_si_ha_state_trap(AVD_CL_CB *avd_cb,
                                            AVD_SU_SI_REL *susi, NCS_BOOL isAmfmibTrap)
{
   NCS_TRAP           trap;
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *s_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *l_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              i_si = 0;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_SUSI_HA_TRAPS(susi->state, &(susi->su->name_net), &(susi->si->name_net), isAmfmibTrap);

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Prepare the instant ID from the su and si name */
   m_NCS_OS_MEMSET(inst_id, 0, SA_MAX_NAME_LENGTH);

   inst_id_len = m_NCS_OS_NTOHS(susi->si->name_net.length) +
                 m_NCS_OS_NTOHS(susi->su->name_net.length) + 2;

   inst_id[0] = m_NCS_OS_NTOHS(susi->su->name_net.length);
   for(i = 0; i < inst_id[0]; i++)
      inst_id[i + 1] = (uns32)(susi->su->name_net.value[i]);

   inst_id[i + 1] = m_NCS_OS_NTOHS(susi->si->name_net.length);
   i_si = i + 2;
   for(i = 0; i < inst_id[i_si - 1]; i++)
      inst_id[i + i_si] = (uns32)(susi->si->name_net.value[i]);


   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));


   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SU_SI;
   i_trap_varbind->i_param_val.i_param_id = saAmfSUSISuName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = m_NCS_OS_NTOHS(susi->su->name_net.length);
   i_trap_varbind->i_param_val.info.i_oct = susi->su->name_net.value;

   i_trap_varbind->i_idx.i_inst_len = inst_id_len;
   i_trap_varbind->i_idx.i_inst_ids = inst_id;

    /* allocate memory */
   s_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if(s_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(s_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   s_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SU_SI;
   s_trap_varbind->i_param_val.i_param_id = saAmfSUSISiName_ID;
   s_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   s_trap_varbind->i_param_val.i_length = m_NCS_OS_NTOHS(susi->si->name_net.length);
   s_trap_varbind->i_param_val.info.i_oct = susi->si->name_net.value;

   s_trap_varbind->i_idx.i_inst_len = inst_id_len;
   s_trap_varbind->i_idx.i_inst_ids = inst_id;
   
   i_trap_varbind->next_trap_varbind = s_trap_varbind;

    /* allocate memory */
   l_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if(l_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
      m_MMGR_NCS_TRAP_VARBIND_FREE(s_trap_varbind);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(l_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   l_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SU_SI;
   l_trap_varbind->i_param_val.i_param_id = saAmfSUSIHAState_ID;
   l_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   l_trap_varbind->i_param_val.i_length = 4;
   l_trap_varbind->i_param_val.info.i_int = susi->state;

   l_trap_varbind->i_idx.i_inst_len = inst_id_len;
   l_trap_varbind->i_idx.i_inst_ids = inst_id;


   s_trap_varbind->next_trap_varbind = l_trap_varbind;

   /* Add to the trap */
   trap.i_trap_vb = i_trap_varbind;

   if(isAmfmibTrap)
   {
      /* Fill in the trap details */
      trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
      trap.i_trap_id = saAmfStateChgSUHaState_ID;
      trap.i_inform_mgr = TRUE;
      status = avd_create_and_send_trap(avd_cb, &trap, &amf_traps_pattern_array[0]);
   }
   else
   {
      /* Fill in the trap details */
      trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_NCS_TRAP_TBL;
      trap.i_trap_id = ncsAlarmStateChgStartSUHaState_ID;
      trap.i_inform_mgr = TRUE;
      status = avd_create_and_send_trap(avd_cb, &trap, &ncs_traps_pattern_array[0]);
   }

   m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
   m_MMGR_NCS_TRAP_VARBIND_FREE(s_trap_varbind);
   m_MMGR_NCS_TRAP_VARBIND_FREE(l_trap_varbind);

   return status;
}


/*****************************************************************************
  Name          :  avd_clm_alarm_service_impaired_trap

  Description   :  This function generates a trap when the CLM service is not
                   able to provide its service.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   err -    Error Code

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_clm_alarm_service_impaired_trap(AVD_CL_CB *avd_cb,
                                         SaAisErrorT err)
{
   NCS_TRAP           trap;
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_clm_alarm_service_impaired_trap");

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_CLM_TRAP_TBL;
   trap.i_trap_id = saClmAlarmServiceImpaired_ID;
   trap.i_inform_mgr = TRUE;

   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_CLM_TRAP_OBJ;
   i_trap_varbind->i_param_val.i_param_id = saClmErrorCode_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   i_trap_varbind->i_param_val.i_length = 4;
   i_trap_varbind->i_param_val.info.i_int = err;

   /* Add to the trap */
   trap.i_trap_vb = i_trap_varbind;

   status = avd_create_and_send_trap(avd_cb, &trap, &clm_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(trap.i_trap_vb);
  
   return status;
}


/*****************************************************************************
  Name          :  avd_clm_node_join_trap

  Description   :  This function generate a node join trap

  Arguments     :  avd_cb -    Pointer to the AVD_CL_CB structure
                   node -      Pointer to the AVD_AVND structure

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         : 
*****************************************************************************/
uns32 avd_clm_node_join_trap(AVD_CL_CB *avd_cb,
                             AVD_AVND *node)
{
   NCS_TRAP           trap;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_clm_node_join_trap");
  
   m_AVD_LOG_CLM_NODE_TRAPS(&(node->node_info.nodeName), AVD_TRAP_JOINED);

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_id = saClmStateChgClusterNodeEntry_ID;

   status = avd_populate_clm_node_traps(avd_cb, node, &trap);

   return status;
}


/*****************************************************************************
  Name          :  avd_clm_node_exit_trap

  Description   :  This function generate a node exit trap

  Arguments     :  avd_cb -    Pointer to the AVD_CL_CB structure
                   node -      Pointer to the AVD_AVND structure

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_clm_node_exit_trap(AVD_CL_CB *avd_cb,
                             AVD_AVND *node)
{
   NCS_TRAP           trap;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_clm_node_exit_trap");

   m_AVD_LOG_CLM_NODE_TRAPS(&(node->node_info.nodeName), AVD_TRAP_EXITED);

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_id = saClmStateChgClusterNodeExit_ID;

   status = avd_populate_clm_node_traps(avd_cb, node, &trap);

   return status;
}


/*****************************************************************************
  Name          :  avd_clm_node_reconfiured_trap

  Description   :  This function generate a node reconfiured trap

  Arguments     :  avd_cb -    Pointer to the AVD_CL_CB structure
                   node -      Pointer to the AVD_AVND structure

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_clm_node_reconfiured_trap(AVD_CL_CB *avd_cb,
                                   AVD_AVND *node)
{
   NCS_TRAP           trap;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_clm_node_reconfiured_trap");

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_id = saClmStateChgClusterNodeReconfigured_ID;

   status = avd_populate_clm_node_traps(avd_cb, node, &trap);

   return status;
}


/*****************************************************************************
  Name          :  avd_populate_clm_node_traps

  Description   :  This function populate and send CLM node realted traps

  Arguments     :  avd_cb -    Pointer to the AVD_CL_CB structure
                   node -      Pointer to the AVD_AVND structure
                   trap -      Pointer to the NCS_TRAP structure

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_populate_clm_node_traps(AVD_CL_CB *avd_cb,
                                 AVD_AVND *node,
                                 NCS_TRAP *trap)
{
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_populate_clm_node_traps");

   /* Fill in the trap details */
   trap->i_trap_tbl_id = NCSMIB_TBL_AVSV_CLM_TRAP_TBL;
   trap->i_inform_mgr = TRUE;

   inst_id_len = m_NCS_OS_NTOHS(node->node_info.nodeName.length);

   /* Prepare the instant ID from the node name */
   m_NCS_OS_MEMSET(inst_id, 0, SA_MAX_NAME_LENGTH);

   inst_id[0]= inst_id_len;

   for(i = 0; i < inst_id_len; i++)
   {
      inst_id[i+1] = (uns32)(node->node_info.nodeName.value[i]);
   }

   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_CLM_NODE;
   i_trap_varbind->i_param_val.i_param_id = saClmNodeName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = node->node_info.nodeName.value;

   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   i_trap_varbind->i_idx.i_inst_ids = inst_id;

   /* Add to the trap */
   trap->i_trap_vb = i_trap_varbind;

   status = avd_create_and_send_trap(avd_cb, trap, &clm_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);

   return status;
}


/*****************************************************************************
  Name          :  avd_gen_ncs_init_success_trap

  Description   :  This function generates a ncs initialization sucessful trap

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   node -   Pointer to the AVD_AVND struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_gen_ncs_init_success_trap(AVD_CL_CB *avd_cb,
                                    AVD_AVND *node)
{
   NCS_TRAP           trap;
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *s_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;

   m_AVD_LOG_FUNC_ENTRY("avd_gen_ncs_init_success_trap");
 
   m_AVD_LOG_NCS_INIT_TRAP(node->node_info.nodeId);

   m_NCS_OS_MEMSET(&trap, 0, sizeof(NCS_TRAP));
  
   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_NCS_TRAP_TBL;
   trap.i_trap_id = ncsInitSuccessOnNode_ID;
   trap.i_inform_mgr = TRUE;

   inst_id_len = m_NCS_OS_NTOHS(node->node_info.nodeName.length);
   
   /* Prepare the instant ID from the node name */
   m_NCS_OS_MEMSET(inst_id, 0, SA_MAX_NAME_LENGTH);

   inst_id[0]= inst_id_len;

   for(i = 0; i < inst_id_len; i++)
   {
      inst_id[i+1] = (uns32)(node->node_info.nodeName.value[i]);
   }
   
   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_NCS_NODE;
   i_trap_varbind->i_param_val.i_param_id = ncsNDNodeName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = node->node_info.nodeName.value;
   
   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   i_trap_varbind->i_idx.i_inst_ids = inst_id;

    /* allocate memory */
   s_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;
   if(s_trap_varbind == NULL)
   {
      m_AVD_LOG_MEM_FAIL(AVD_TRAP_VAR_BIND_ALLOC_FAILED);
      m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(s_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   s_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_NCS_NODE;
   s_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   s_trap_varbind->i_param_val.i_length = 4;
   s_trap_varbind->i_param_val.i_param_id = ncsNDNodeId_ID;
   s_trap_varbind->i_param_val.info.i_int = node->node_info.nodeId;

   s_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   s_trap_varbind->i_idx.i_inst_ids = inst_id;

   s_trap_varbind->next_trap_varbind = i_trap_varbind;

   /* Add to the trap */
   trap.i_trap_vb = s_trap_varbind;

   status = avd_create_and_send_trap(avd_cb, &trap, &ncs_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
   m_MMGR_NCS_TRAP_VARBIND_FREE(s_trap_varbind);

   return status;
}


/*****************************************************************************
  Name          :  avd_create_and_send_trap

  Description   :  This function encode the trap data, allocate and publish the event.

  Arguments     :  avd_cb - Pointer to the AVD_CL_CB structure
                   trap   - Pointer to the NCS_TRAP struct
                   pattern- Poinetr to the array of patterns

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avd_create_and_send_trap(AVD_CL_CB *avd_cb,
                                   NCS_TRAP *trap, SaEvtEventPatternT *pattern)
{
   uns32              status = NCSCC_RC_FAILURE;
   uns32              tlv_size = 0;
   uns8*              encoded_buffer = NULL;
   EDU_ERR            o_err = EDU_NORMAL;
   SaEvtEventHandleT  eventHandle;
   SaEvtEventPriorityT priority = SA_EVT_HIGHEST_PRIORITY;
   SaTimeT            retentionTime = AVSV_TRAP_RETENTION_TIMEOUT;
   SaEvtEventIdT      eventId;
   SaAisErrorT        saStatus = SA_AIS_OK;
   SaSizeT            eventDataSize;
   SaEvtEventPatternArrayT  patternArray;

   m_AVD_LOG_FUNC_ENTRY("avd_create_and_send_trap");

   m_NCS_OS_MEMSET(&patternArray, 0, sizeof(SaEvtEventPatternArrayT));
   m_NCS_OS_MEMSET(&eventHandle, 0, sizeof(SaEvtEventHandleT));
   m_NCS_OS_MEMSET(&eventId, 0, sizeof(SaEvtEventIdT));

   if(!trap)
      return NCSCC_RC_FAILURE;

   /* Fill Trap version */
   trap->i_version = m_NCS_TRAP_VERSION;

   if(avd_eda_initialize(avd_cb) != NCSCC_RC_SUCCESS)
   {
      return status;
   }

   /* Encode the trap */
   tlv_size = ncs_tlvsize_for_ncs_trap_get(trap);

   /* allocate buffer to encode data  */
   encoded_buffer = m_MMGR_ALLOC_AVD_EDA_TLV_MSG(tlv_size);

   if (encoded_buffer == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   m_NCS_MEMSET(encoded_buffer, '\0', tlv_size);

   /* call the EDU macro to encode with buffer pointer and size */
   status = m_NCS_EDU_TLV_EXEC(&avd_cb->edu_hdl, ncs_edp_ncs_trap, encoded_buffer,
                         tlv_size, EDP_OP_TYPE_ENC, trap, &o_err);

   if (status != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_AVD_EDA_TLV_MSG(encoded_buffer);
      return status;
   }

   eventDataSize = (SaSizeT)tlv_size;

   /* Allocate memory for the event */
   saStatus = saEvtEventAllocate(avd_cb->evtChannelHandle, &eventHandle);

   if (saStatus != SA_AIS_OK)
   {
      m_MMGR_FREE_AVD_EDA_TLV_MSG(encoded_buffer);
      return NCSCC_RC_FAILURE;
   }

   patternArray.patternsNumber = AVSV_TRAP_PATTERN_ARRAY_LEN;
   patternArray.patterns = pattern;

   /* Set all the writeable event attributes */
   saStatus = saEvtEventAttributesSet(eventHandle,
                                      &patternArray,
                                      priority,
                                      retentionTime,
                                      &avd_cb->publisherName);
   if (saStatus != SA_AIS_OK)
   {
      m_MMGR_FREE_AVD_EDA_TLV_MSG(encoded_buffer);
     
      /* Free the allocated memory for the event */
      saStatus = saEvtEventFree(eventHandle);

      return NCSCC_RC_FAILURE;
   }

   /* Publish an event on the channel */
   saStatus = saEvtEventPublish(eventHandle,
                                encoded_buffer,
                                eventDataSize,
                                &eventId);
   if (saStatus != SA_AIS_OK)
   {
      m_NCS_DBG_PRINTF("\nsaEvtEventPublish failed\n");

      m_MMGR_FREE_AVD_EDA_TLV_MSG(encoded_buffer);
     
      m_AVD_LOG_TRAP_EVT(AVD_TRAP_EDA_EVT_PUBLISH_FAILED, saStatus);

     /* Free the allocated memory for the event */
      saStatus = saEvtEventFree(eventHandle);

      return NCSCC_RC_FAILURE;
   }

   /* Free the allocated memory for the event */
   saStatus = saEvtEventFree(eventHandle);
   if (saStatus != SA_AIS_OK)
   {
      m_NCS_DBG_PRINTF("\nsaEvtEventFree failed\n");
      
      m_MMGR_FREE_AVD_EDA_TLV_MSG(encoded_buffer);

      return NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVD_EDA_TLV_MSG(encoded_buffer);

   return status;
}

/******************************************************************************
 Name      :  avd_eda_initialize

 Purpose   :  To intialize the session with EDA Agent
                 - To register the callbacks

 Arguments :  AVD_CL_CB *avd_cb - AVD control block

 Returns   :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE 

 Notes     :
******************************************************************************/
uns32 avd_eda_initialize(AVD_CL_CB  *avd_cb)
{
   uns32           channelFlags = 0;
   SaAisErrorT        status = SA_AIS_OK;
   SaTimeT         timeout = AVSV_TRAP_CHANNEL_OPEN_TIMEOUT;
   SaEvtCallbacksT callbacks = {NULL, NULL};
   SaVersionT      saf_version = {'B', 1, 1};

   m_AVD_LOG_FUNC_ENTRY("avd_eda_initialize");

   if (avd_cb == NULL)
   {
      /* log the error here */
      return NCSCC_RC_FAILURE;
   }
   
   if(!avd_cb->evtHandle)
   {
         m_NCS_STRCPY(avd_cb->publisherName.value, AVD_EDA_EVT_PUBLISHER_NAME);
         avd_cb->publisherName.length = m_NCS_STRLEN(avd_cb->publisherName.value);

         /* update the Channel Name */
         m_NCS_STRCPY(avd_cb->evtChannelName.value, m_SNMP_EDA_EVT_CHANNEL_NAME);
         avd_cb->evtChannelName.length = m_NCS_STRLEN(avd_cb->evtChannelName.value);

         /* initialize the eda  */
         status = saEvtInitialize(&avd_cb->evtHandle,
                               &callbacks,
                               &saf_version);

         if (status != SA_AIS_OK)
         {
            /* log the error code here */
            m_AVD_LOG_TRAP_EVT(AVD_TRAP_EDA_INIT_FAILED, status);
            return NCSCC_RC_FAILURE;
         }
   }

   if(!avd_cb->evtChannelHandle)
   {
      /* Open event channel with remote server (EDS)  */
      channelFlags = SA_EVT_CHANNEL_CREATE | SA_EVT_CHANNEL_PUBLISHER;

      status = saEvtChannelOpen(avd_cb->evtHandle,
                             &avd_cb->evtChannelName,
                             channelFlags,
                             timeout,
                             &avd_cb->evtChannelHandle);
      if (status != SA_AIS_OK)
      {
         /* log the error code here */
         m_AVD_LOG_TRAP_EVT(AVD_TRAP_EDA_CHNL_OPEN_FAILED, status);
         return NCSCC_RC_FAILURE;
      }
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 Name      :  avd_eda_finalize

 Purpose   :  To Finalize the session with the EDA

 Arguments :  AVD_CL_CB *avd_cb - AVD control block
 Returns   :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

 Notes     :
*****************************************************************************/
uns32  avd_eda_finalize(AVD_CL_CB *avd_cb)
{
   SaAisErrorT  status = SA_AIS_OK;

   m_AVD_LOG_FUNC_ENTRY("avd_eda_finalize");

   /* validate the inputs */
   if (avd_cb == NULL)
   {
      /* log the error here */
      return NCSCC_RC_FAILURE;
   }

   /* terminate the communication channels */
   status = saEvtChannelClose(avd_cb->evtChannelHandle);
   if (status != SA_AIS_OK)
   {
      /* log the error here */
      return NCSCC_RC_FAILURE;
   }

   /* Close all the associations with EDA, close the deal */
   status = saEvtFinalize(avd_cb->evtHandle);
   if (status != SA_AIS_OK)
   {
      /* log the error here */
      return NCSCC_RC_FAILURE;
   }

   return status;
}
