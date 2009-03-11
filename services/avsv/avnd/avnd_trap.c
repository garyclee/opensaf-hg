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
  MODULE NAME: AVND_TRAP.C 

..............................................................................

  DESCRIPTION:   This file contains routines to generate traps .

******************************************************************************/

#include "avnd.h"

static SaEvtEventPatternT amf_traps_pattern_array[AVSV_TRAP_PATTERN_ARRAY_LEN] = {
       {10, 10, (SaUint8T *) "AVSV_TRAPS"},
       {17, 17, (SaUint8T *) "SAF_AMF_MIB_TRAPS"}
};
static SaEvtEventPatternT ncs_traps_pattern_array[AVSV_TRAP_PATTERN_ARRAY_LEN] = {
       {10, 10, (SaUint8T *) "AVSV_TRAPS"},
       {18, 18, (SaUint8T *) "NCS_AVSV_MIB_TRAPS"}
};


/*****************************************************************************
  Name          :  avnd_gen_comp_inst_failed_trap

  Description   :  This function generates a component instantiation failed trap.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_comp_inst_failed_trap(AVND_CB *avnd_cb,
                                   AVND_COMP *comp)
{
   NCS_TRAP           trap;
   
   memset(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_id = saAmfAlarmCompInstantiationFailed_ID;
  
   /* Log the componet insantiation failed info */ 
   m_AVND_LOG_CLC_TRAPS(AVND_LOG_TRAP_INSTANTIATION, &(comp->name_net));

   return avnd_populate_comp_traps(avnd_cb, comp, &trap, FALSE);
}

/*****************************************************************************
  Name          :  avnd_gen_comp_term_failed_trap

  Description   :  This function generates a component termination failed trap.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_comp_term_failed_trap(AVND_CB *avnd_cb,
                                   AVND_COMP *comp)
{
   NCS_TRAP           trap;

   memset(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap object id */
   trap.i_trap_id = saAmfAlarmCompTerminationFailed_ID;

   /* Log the componet termination failed info */
   m_AVND_LOG_CLC_TRAPS(AVND_LOG_TRAP_TERMINATION, &(comp->name_net));

   return avnd_populate_comp_traps(avnd_cb, comp, &trap, FALSE);
}

/*****************************************************************************
  Name          :  avnd_gen_comp_clean_failed_trap

  Description   :  This function generates a component cleanup failed trap.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_comp_clean_failed_trap(AVND_CB *avnd_cb,
                                   AVND_COMP *comp)
{
   NCS_TRAP           trap;

   memset(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_id = saAmfAlarmCompCleanupFailed_ID;

   /* Log the cleanup insantiation failed info */
   m_AVND_LOG_CLC_TRAPS(AVND_LOG_TRAP_CLEANUP, &(comp->name_net));

   return avnd_populate_comp_traps(avnd_cb, comp, &trap, FALSE);
}


/*****************************************************************************
  Name          :  avnd_gen_comp_proxied_orphaned_trap

  Description   :  This function generates a proxied component orphane trap.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_comp_proxied_orphaned_trap(AVND_CB *avnd_cb,
                                        AVND_COMP *comp)
{
   NCS_TRAP           trap;

   memset(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_id = saAmfStateChgProxiedCompOrphaned_ID;

   /* Log the component orphaned info  */
   if(comp->pxy_comp) 
   m_AVND_LOG_PROXIED_ORPHANED_TRAP(&(comp->name_net), &(comp->pxy_comp->name_net));

   return avnd_populate_comp_traps(avnd_cb, comp, &trap, TRUE);
}


/*****************************************************************************
  Name          :  avnd_populate_comp_traps

  Description   :  This function populate and send the component realted traps.

  Arguments     :  avnd_cb -    Pointer to the AVND_CB structure
                   comp -       Pointer to the AVND_COMP structure
                   trap -       Pointer to the NCS_TRAP structure
                   isProxTrap - Flag

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         : Set isProxTrap flag to true if this is saAmfStateChgProxiedCompOrphaned
                  trap otherwise set it false.
*****************************************************************************/
uns32 avnd_populate_comp_traps(AVND_CB *avnd_cb,
                                    AVND_COMP *comp, 
                                    NCS_TRAP *trap,
                                    NCS_BOOL isProxTrap)
{
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *prox_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;
   
   /* Fill in the trap details */
   trap->i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap->i_inform_mgr = TRUE;

   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;

   if (i_trap_varbind == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   memset(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   inst_id_len = m_NCS_OS_NTOHS(comp->name_net.length);
  
   /* Prepare the instant ID from the comp name */
   memset(inst_id, 0, SA_MAX_NAME_LENGTH);
 
   inst_id[0]= inst_id_len;
   
   for(i = 0; i < inst_id_len; i++)
   {
      inst_id[i+1] = (uns32)(comp->name_net.value[i]);
   }
   
   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_COMP;
   i_trap_varbind->i_param_val.i_param_id = saAmfCompName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = comp->name_net.value;

   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   i_trap_varbind->i_idx.i_inst_ids = inst_id;

   /* Add to the trap */
   trap->i_trap_vb = i_trap_varbind;
   
   if(isProxTrap)
   {
      /* allocate memory */
      prox_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;
      if(prox_trap_varbind == NULL)
      {
         m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
         return NCSCC_RC_FAILURE;
      }

      memset(prox_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

      /* Fill in the object details */
      prox_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_COMP;
      prox_trap_varbind->i_param_val.i_param_id = saAmfCompCurrProxyName_ID;
      prox_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
      prox_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
      prox_trap_varbind->i_idx.i_inst_ids = inst_id;

      if(comp->pxy_comp)
      {
         prox_trap_varbind->i_param_val.i_length = m_NCS_OS_NTOHS(comp->pxy_comp->name_net.length);
         prox_trap_varbind->i_param_val.info.i_oct = comp->pxy_comp->name_net.value;
      }
      else
      {
         prox_trap_varbind->i_param_val.i_length = 0;
         prox_trap_varbind->i_param_val.info.i_oct = NULL;
      }
      /* Add to the trap */
      trap->i_trap_vb = prox_trap_varbind;
      prox_trap_varbind->next_trap_varbind = i_trap_varbind;
   }  
 

   status = avnd_create_and_send_trap(avnd_cb, trap, &amf_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
   
   if(isProxTrap)
     m_MMGR_NCS_TRAP_VARBIND_FREE(prox_trap_varbind);

   return status;
}


/*****************************************************************************
  Name          :  avnd_gen_su_oper_chg_trap

  Description   :  This function generates a su oper state change trap.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   su - Pointer to the AVND_SU struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_su_oper_state_chg_trap(AVND_CB *avnd_cb,
                                      AVND_SU *su)
{
   NCS_TRAP           trap;

   memset(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_id = saAmfStateChgSUOper_ID;
   
   /* Log the SU oper state  */
   m_AVND_LOG_SU_OPER_STATE_TRAP(su->oper, &(su->name_net));

   return avnd_populate_su_traps(avnd_cb, su, &trap, TRUE);
}



/*****************************************************************************
  Name          :  avnd_gen_su_pres_state_chg_trap

  Description   :  This function generates a su presense state change trap.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   su - Pointer to the AVND_SU struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_su_pres_state_chg_trap(AVND_CB *avnd_cb,
                                      AVND_SU *su)
{
   NCS_TRAP           trap;

   memset(&trap, 0, sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_id = saAmfStateChgSUPresence_ID;
   
   /* Log the SU oper state  */
   m_AVND_LOG_SU_PRES_STATE_TRAP(su->pres, &(su->name_net));

   return avnd_populate_su_traps(avnd_cb, su, &trap, FALSE);
}




/*****************************************************************************
  Name          :  avnd_populate_su_traps

  Description   :  This function populate and send the su realted traps.

  Arguments     :  avnd_cb -    Pointer to the AVND_CB structure
                   su -         Pointer to the AVND_SU structure
                   trap -       Pointer to the NCS_TRAP structure
                   isOper -     Flag

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         : Set isOper flag to true if this is saAmfStateChgSUOper trap
                  otherwise set it false.
*****************************************************************************/
uns32 avnd_populate_su_traps(AVND_CB *avnd_cb,
                                    AVND_SU *su, 
                                    NCS_TRAP *trap,
                                    NCS_BOOL isOper)
{
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *s_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;
   
   /* Fill in the trap details */
   trap->i_trap_tbl_id = NCSMIB_TBL_AVSV_AMF_TRAP_TBL;
   trap->i_inform_mgr = TRUE;

   /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;
   if (i_trap_varbind == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   memset(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));

   inst_id_len = m_NCS_OS_NTOHS(su->name_net.length);
  
   /* Prepare the instant ID from the su name */
   memset(inst_id, 0, SA_MAX_NAME_LENGTH);
 
   inst_id[0]= inst_id_len;
   
   for(i = 0; i < inst_id_len; i++)
   {
      inst_id[i+1] = (uns32)(su->name_net.value[i]);
   }
   
   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SU;
   i_trap_varbind->i_param_val.i_param_id = saAmfSUName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = su->name_net.value;

   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   i_trap_varbind->i_idx.i_inst_ids = inst_id;

   /* Add to the trap */
   trap->i_trap_vb = i_trap_varbind;
   
   /* allocate memory */
   s_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;
   if(s_trap_varbind == NULL)
   {
      m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
      return NCSCC_RC_FAILURE;
   }

   memset(s_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND));
   
   /* Fill in the object details */
   s_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_AMF_SU;
   s_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   s_trap_varbind->i_param_val.i_length = 4;
   s_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   s_trap_varbind->i_idx.i_inst_ids = inst_id;

   i_trap_varbind->next_trap_varbind = s_trap_varbind;
   
   if(isOper)
   {
      s_trap_varbind->i_param_val.i_param_id = saAmfSUOperState_ID; 
      s_trap_varbind->i_param_val.info.i_int = su->oper;
   }
   else
   {
      s_trap_varbind->i_param_val.i_param_id = saAmfSUPresenceState_ID; 
      s_trap_varbind->i_param_val.info.i_int = su->pres;
   }  
 

   status = avnd_create_and_send_trap(avnd_cb, trap, &amf_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
   m_MMGR_NCS_TRAP_VARBIND_FREE(s_trap_varbind);

   return status;
}

/*****************************************************************************
  Name          :  avnd_gen_comp_fail_on_node_trap

  Description   :  This function generates a component failed trap.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   comp    - Pointer to the AVND_COMP struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_gen_comp_fail_on_node_trap(AVND_CB *avnd_cb,
                                     AVND_ERR_SRC errSrc,
                                     AVND_COMP *comp)
{
   NCS_TRAP           trap;
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *s_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *l_trap_varbind = NULL;
   uns32              inst_id_len = 0;
   uns32              inst_id_nd[SA_MAX_NAME_LENGTH];
   uns32              inst_id_comp[SA_MAX_NAME_LENGTH];
   uns32              i = 0;
   uns32              status = NCSCC_RC_FAILURE;


   m_AVND_LOG_COMP_FAIL_TRAP(avnd_cb->clmdb.node_info.nodeId, &(comp->name_net), errSrc);

   memset(&trap, '\0', sizeof(NCS_TRAP));

   /* Fill in the trap details */
   trap.i_trap_tbl_id = NCSMIB_TBL_AVSV_NCS_TRAP_TBL;
   trap.i_trap_id = ncsAlarmCompFailOnNode_ID;
   trap.i_inform_mgr = TRUE;


   /* Fill the node ID info into varbind */
   inst_id_len = avnd_cb->clmdb.node_info.nodeName.length;

   /* Prepare the instant ID from the node name */
   memset(inst_id_nd, '\0', SA_MAX_NAME_LENGTH);

   inst_id_nd[0]= inst_id_len;

   for(i = 0; i < inst_id_len; i++)
   {
      inst_id_nd[i+1] = (uns32)(avnd_cb->clmdb.node_info.nodeName.value[i]);
   }

    /* allocate memory */
   s_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;
   if(s_trap_varbind == NULL)
   {
      m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
      return NCSCC_RC_FAILURE;
   }
   memset(s_trap_varbind, '\0', sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   s_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_NCS_NODE;
   s_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   s_trap_varbind->i_param_val.i_length = 4;
   s_trap_varbind->i_param_val.i_param_id = ncsNDNodeId_ID;
   s_trap_varbind->i_param_val.info.i_int = avnd_cb->clmdb.node_info.nodeId;
   s_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   s_trap_varbind->i_idx.i_inst_ids = inst_id_nd;


   inst_id_len = 0;
   /* Fill the component name info into varbind */
   inst_id_len = m_NCS_OS_NTOHS(comp->name_net.length);

   /* Prepare the instant ID from the comp name */
   memset(inst_id_comp, '\0', SA_MAX_NAME_LENGTH);
   inst_id_comp[0]= inst_id_len;

   for(i = 0; i < inst_id_len; i++)
   {
      inst_id_comp[i+1] = (uns32)(comp->name_net.value[i]);
   }

    /* allocate memory */
   i_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;
   if (i_trap_varbind == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   memset(i_trap_varbind, '\0', sizeof(NCS_TRAP_VARBIND));

   /* Fill in the object details */
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_NCS_COMP_STAT;
   i_trap_varbind->i_param_val.i_param_id = ncsSCompCompName_ID;
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   i_trap_varbind->i_param_val.i_length = inst_id_len;
   i_trap_varbind->i_param_val.info.i_oct = comp->name_net.value;

   i_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   i_trap_varbind->i_idx.i_inst_ids = inst_id_comp;

   s_trap_varbind->next_trap_varbind = i_trap_varbind;

    /* allocate memory */
   l_trap_varbind = m_MMGR_NCS_TRAP_VARBIND_ALLOC;
   if(l_trap_varbind == NULL)
   {
      m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
      m_MMGR_NCS_TRAP_VARBIND_FREE(s_trap_varbind);
      return NCSCC_RC_FAILURE;
   }
   memset(l_trap_varbind, '\0', sizeof(NCS_TRAP_VARBIND));

   l_trap_varbind->i_tbl_id = NCSMIB_TBL_AVSV_NCS_COMP_STAT;
   l_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   l_trap_varbind->i_param_val.i_length = 4;
   l_trap_varbind->i_param_val.i_param_id = ncsSCompErrSrc_ID;
   l_trap_varbind->i_param_val.info.i_int = errSrc;
   l_trap_varbind->i_idx.i_inst_len = inst_id_len + 1;
   l_trap_varbind->i_idx.i_inst_ids = inst_id_comp;

   i_trap_varbind->next_trap_varbind = l_trap_varbind;

   l_trap_varbind->next_trap_varbind = NULL;
   
   /* Add to the trap */
   trap.i_trap_vb = s_trap_varbind;

   status = avnd_create_and_send_trap(avnd_cb, &trap, &ncs_traps_pattern_array[0]);

   m_MMGR_NCS_TRAP_VARBIND_FREE(i_trap_varbind);
   m_MMGR_NCS_TRAP_VARBIND_FREE(s_trap_varbind);
   m_MMGR_NCS_TRAP_VARBIND_FREE(l_trap_varbind);

   return status;
}


/*****************************************************************************
  Name          :  avnd_create_and_send_trap

  Description   :  This function encode the trap data, allocate and publish the event.

  Arguments     :  avnd_cb - Pointer to the AVND_CB structure
                   trap    - Pointer to the NCS_TRAP struct
                   pattern - Pointer to the array of patterns

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
*****************************************************************************/
uns32 avnd_create_and_send_trap(AVND_CB *avnd_cb,
                                   NCS_TRAP *trap, 
                                   SaEvtEventPatternT *pattern)
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


   memset(&patternArray, 0, sizeof(SaEvtEventPatternArrayT));
   memset(&eventHandle, 0, sizeof(SaEvtEventHandleT));
   memset(&eventId, 0, sizeof(SaEvtEventIdT));

   if(avnd_eda_initialize(avnd_cb) != NCSCC_RC_SUCCESS)
   {
      return status;
   }

   if(!trap)
      return NCSCC_RC_FAILURE;

   /* Fill trap version */
   trap->i_version = m_NCS_TRAP_VERSION;
   
   /* Encode the trap */
   tlv_size = ncs_tlvsize_for_ncs_trap_get(trap);

   /* allocate buffer to encode data  */
   encoded_buffer = m_MMGR_ALLOC_AVND_EDA_TLV_MSG(tlv_size);

   if (encoded_buffer == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   memset(encoded_buffer, '\0', tlv_size);

   /* call the EDU macro to encode with buffer pointer and size */
   status = m_NCS_EDU_TLV_EXEC(&avnd_cb->edu_hdl, ncs_edp_ncs_trap, encoded_buffer,
                         tlv_size, EDP_OP_TYPE_ENC, trap, &o_err);

   if (status != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_AVND_EDA_TLV_MSG(encoded_buffer);
      return status;
   }

   eventDataSize = (SaSizeT)tlv_size;

   /* Allocate memory for the event */
   saStatus = saEvtEventAllocate(avnd_cb->evtChannelHandle, &eventHandle);

   if (saStatus != SA_AIS_OK)
   {
      m_MMGR_FREE_AVND_EDA_TLV_MSG(encoded_buffer);
      return NCSCC_RC_FAILURE;
   }

   patternArray.patternsNumber = AVSV_TRAP_PATTERN_ARRAY_LEN;
   patternArray.patterns = pattern;

   /* Set all the writeable event attributes */
   saStatus = saEvtEventAttributesSet(eventHandle,
                                      &patternArray,
                                      priority,
                                      retentionTime,
                                      &avnd_cb->publisherName);
   if (saStatus != SA_AIS_OK)
   {
      m_MMGR_FREE_AVND_EDA_TLV_MSG(encoded_buffer);
     
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

      m_MMGR_FREE_AVND_EDA_TLV_MSG(encoded_buffer);
 
      m_AVND_LOG_TRAP_EVT(AVND_LOG_EDA_EVT_PUBLISH_FAILED, saStatus);

     /* Free the allocated memory for the event */
      saStatus = saEvtEventFree(eventHandle);

      return NCSCC_RC_FAILURE;
   }

   /* Free the allocated memory for the event */
   saStatus = saEvtEventFree(eventHandle);
   if (saStatus != SA_AIS_OK)
   {
      m_NCS_DBG_PRINTF("\nsaEvtEventFree failed\n");
      
      m_MMGR_FREE_AVND_EDA_TLV_MSG(encoded_buffer);

      return NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVND_EDA_TLV_MSG(encoded_buffer);

   return status;
}

/******************************************************************************
 Name      :  avnd_eda_initialize

 Purpose   :  To intialize the session with EDA Agent
                 - To register the callbacks

 Arguments :  AVND_CB *avnd_cb - AVND control block

 Returns   :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE 

 Notes     :
******************************************************************************/
uns32 avnd_eda_initialize(AVND_CB  *avnd_cb)
{
   uns32           channelFlags = 0;
   SaAisErrorT        status = SA_AIS_OK;
   SaTimeT         timeout = AVSV_TRAP_CHANNEL_OPEN_TIMEOUT;
   SaEvtCallbacksT callbacks = {NULL, NULL};
   SaVersionT      saf_version = {'B', 1, 1};


   if (avnd_cb == NULL)
   {
      /* log the error here */
      return NCSCC_RC_FAILURE;
   }
  
   if(!avnd_cb->evtHandle)
   {
      m_NCS_STRCPY(avnd_cb->publisherName.value, AVND_EDA_EVT_PUBLISHER_NAME);
      avnd_cb->publisherName.length = m_NCS_STRLEN(avnd_cb->publisherName.value);

      /* update the Channel Name */
      m_NCS_STRCPY(avnd_cb->evtChannelName.value, m_SNMP_EDA_EVT_CHANNEL_NAME);
      avnd_cb->evtChannelName.length = m_NCS_STRLEN(avnd_cb->evtChannelName.value);


      /* initialize the eda  */
      status = saEvtInitialize(&avnd_cb->evtHandle,
                               &callbacks,
                               &saf_version);

      if (status != SA_AIS_OK)
      {
         /* log the error code here */
         m_AVND_LOG_TRAP_EVT(AVND_LOG_EDA_INIT_FAILED, status);
         return NCSCC_RC_FAILURE;
      }
   }
 
   if(!avnd_cb->evtChannelHandle)
   {
      /* Open event channel with remote server (EDS)  */
      channelFlags = SA_EVT_CHANNEL_CREATE | SA_EVT_CHANNEL_PUBLISHER;

      status = saEvtChannelOpen(avnd_cb->evtHandle,
                             &avnd_cb->evtChannelName,
                             channelFlags,
                             timeout,
                             &avnd_cb->evtChannelHandle);
      if (status != SA_AIS_OK)
      {
         /* log the error code here */
         m_AVND_LOG_TRAP_EVT(AVND_LOG_EDA_CHNL_OPEN_FAILED, status);
         return NCSCC_RC_FAILURE;
      }
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 Name      :  avnd_eda_finalize

 Purpose   :  To Finalize the session with the EDA

 Arguments :  AVND_CB *avnd_cb - AVND control block

 Returns   :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

 Notes     :
*****************************************************************************/
uns32  avnd_eda_finalize(AVND_CB *avnd_cb)
{
   SaAisErrorT  status = SA_AIS_OK;


   /* validate the inputs */
   if (avnd_cb == NULL)
   {
      /* log the error here */
      return NCSCC_RC_FAILURE;
   }

   /* terminate the communication channels */
   status = saEvtChannelClose(avnd_cb->evtChannelHandle);
   if (status != SA_AIS_OK)
   {
      /* log the error here */
      return NCSCC_RC_FAILURE;
   }

   /* Close all the associations with EDA, close the deal */
   status = saEvtFinalize(avnd_cb->evtHandle);
   if (status != SA_AIS_OK)
   {
      /* log the error here */
      return NCSCC_RC_FAILURE;
   }

   return status;
}
