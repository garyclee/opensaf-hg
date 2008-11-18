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
 */

/*****************************************************************************
..............................................................................

  MODULE NAME: SNMPTM_TBLONE.C 

..............................................................................

  DESCRIPTION:  Defines the API to add/del SNMPTM TBLONE entries and the 
                procedure that does the clean-up of SNMPTM tblone tree.
..............................................................................


******************************************************************************/

#include "snmptm.h"

#if (NCS_SNMPTM == 1)

static SaEvtEventPatternT snmptm_trap_pattern_array[SNMPTM_TRAP_PATTERN_ARRAY_LEN] = {
       {6, 6, (SaUint8T *) "snmptm"},
       {0, 0, (SaUint8T *) NULL},
       {9, 9, (SaUint8T *) "SNMP_TRAP"}
};

/*****************************************************************************
  Name          :  snmptm_create_tblone_entry

  Description   :  This function creates an entry into SNMPTM TBLONE table. 
                   Basically it adds a tblone node to a tblone patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tblone_key - pointer to the TBLONE request info struct.
        
  Return Values :  tblone - Upon successful addition of tblone entry  
                   NULL   - Upon failure of adding tblone entry  
    
  Notes         :  
*****************************************************************************/
SNMPTM_TBLONE *snmptm_create_tblone_entry(SNMPTM_CB *snmptm,
                                          SNMPTM_TBL_KEY *tblone_key)
{

   SNMPTM_TBLONE *tblone = NULL;
   uns32         ip_addr = 0;

   /* Alloc the memory for TBLONE entry */
   if ((tblone = (SNMPTM_TBLONE*)m_MMGR_ALLOC_SNMPTM_TBLONE)
        == SNMPTM_TBLONE_NULL)   
   {
      m_NCS_CONS_PRINTF("\nNot able to alloc the memory for TBLONE \n");
      return NULL;
   }

   m_NCS_OS_MEMSET((char *)tblone, '\0', sizeof(SNMPTM_TBLONE));
   
   /* Copy the key contents to tblone struct */
   tblone->tblone_key.ip_addr = tblone_key->ip_addr;

   tblone->tblone_pat_node.key_info = (uns8 *)&((tblone->tblone_key));    

   /* Add to the tblone entry/node to tblone patricia tree */
   if(NCSCC_RC_SUCCESS != ncs_patricia_tree_add(&(snmptm->tblone_tree),
                                                &(tblone->tblone_pat_node)))
   {
      m_NCS_CONS_PRINTF("\nNot able add TBLONE node to TBLONE tree.\n");

      /* Free the alloc memory of TBLONE */
      m_MMGR_FREE_SNMPTM_TBLONE(tblone);

      return NULL;
   }

   tblone->tblone_row_status = NCSMIB_ROWSTATUS_ACTIVE; 
  
   ip_addr = ntohl(tblone_key->ip_addr.info.v4);
   m_NCS_CONS_PRINTF("\n\n ROW created in the TBLONE, INDEX: %d.%d.%d.%d", (uns8)(ip_addr >> 24),
                                            (uns8)(ip_addr >> 16),
                                            (uns8)(ip_addr >> 8),
                                            (uns8)(ip_addr)); 
      

   return tblone;
}


/*****************************************************************************
  Name          :  snmptm_delete_tblone_entry

  Description   :  This function deletes an entry from TBLONE table. Basically
                   it deletes a tblone node from a tblone patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tblone - pointer to the TBLONE struct.
        
  Return Values :  Nothing 
    
  Notes         :  
*****************************************************************************/
void snmptm_delete_tblone_entry(SNMPTM_CB *snmptm,
                                SNMPTM_TBLONE *tblone)
{

   /* Delete the tblone entry from the tblone patricia tree */
   ncs_patricia_tree_del(&(snmptm->tblone_tree), &(tblone->tblone_pat_node));
  
   /* Free the memory of TBLONE */
   m_MMGR_FREE_SNMPTM_TBLONE(tblone);

   return;
}


/*****************************************************************************
  Name          :  snmptm_delete_tblone

  Description   :  This function deletes all the tblone entries (nodes) from
                   tblone tree and destroys a tblone tree.

  Arguments     :  snmptm  - Pointer to the SNMPTM CB structure

  Return Values :  

  Notes         :
*****************************************************************************/
void  snmptm_delete_tblone(SNMPTM_CB *snmptm)
{
   SNMPTM_TBLONE *tblone;

   /* Delete all the TBLONE entries */
   while(SNMPTM_TBLONE_NULL != (tblone = (SNMPTM_TBLONE*)
                        ncs_patricia_tree_getnext(&snmptm->tblone_tree, 0)))
   {
       /* Delete the node from the tblone tree */
       ncs_patricia_tree_del(&snmptm->tblone_tree, &tblone->tblone_pat_node);

       /* Free the memory */
       m_MMGR_FREE_SNMPTM_TBLONE(tblone);
   }

   /* Destroy TBLONE tree */
   ncs_patricia_tree_destroy(&snmptm->tblone_tree);

   return;
}


/*****************************************************************************
  Name          :  snmptm_tblone_gen_trap

  Description   :  This function generates a TBLONE trap. 

  Arguments     :  snmptm - Pointer to the SNMPTM CB structure
                   args   - Pointer to the NCSMIB_ARG struct
                   tblone - Pointer to the SNMPTM_TBL_ONE struct

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE 

  Notes         :
*****************************************************************************/
uns32 snmptm_tblone_gen_trap(SNMPTM_CB *snmptm, 
                             NCSMIB_ARG *args,
                             SNMPTM_TBLONE *tblone)
{
   NCS_TRAP           snmptm_tblone; 
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL; 
   NCS_TRAP_VARBIND   *temp_trap_varbind = NULL; 
   uns32              status = NCSCC_RC_FAILURE; 
   uns32              tlv_size = 0; 
   uns8*              encoded_buffer = NULL; 
   EDU_ERR            o_err = EDU_NORMAL;
   SaEvtEventHandleT  eventHandle;
   SaUint8T           priority = 1;
   SaTimeT            retentionTime = 1000;
   SaEvtEventIdT      eventId;
   SaAisErrorT           saStatus = SA_AIS_OK;
   SaSizeT            eventDataSize;
   SaEvtEventPatternArrayT  patternArray;
   char               temp_oid_stream[128*4];
             

   m_NCS_OS_MEMSET(&patternArray, 0, sizeof(SaEvtEventPatternArrayT)); 
   m_NCS_OS_MEMSET(&snmptm_tblone, 0, sizeof(NCS_TRAP)); 
   m_NCS_OS_MEMSET(&eventHandle, 0, sizeof(SaEvtEventHandleT)); 
   m_NCS_OS_MEMSET(&eventId, 0, sizeof(SaEvtEventIdT)); 

   /* Fill in the trap details */ 
   snmptm_tblone.i_version = m_NCS_TRAP_VERSION;
   snmptm_tblone.i_trap_tbl_id = NCSMIB_TBL_SNMPTM_NOTIF; 
   snmptm_tblone.i_trap_id = ncsTestNotificationTableObjs_ID;
   snmptm_tblone.i_inform_mgr = TRUE; 

   /* allocate memory for bgpPeerLastError */ 
   i_trap_varbind = (NCS_TRAP_VARBIND *)malloc(sizeof(NCS_TRAP_VARBIND)); 
   if (i_trap_varbind == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 

   /* Fill in the object ncsTestTableOneDisplayString details */ 
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_SNMPTM_TBLONE;

   i_trap_varbind->i_param_val.i_param_id = ncsTestTableOneDisplayString_ID; 
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
   
   if (tblone->tblone_disp_str) 
   {
      i_trap_varbind->i_param_val.i_length = strlen(tblone->tblone_disp_str); 
      i_trap_varbind->i_param_val.info.i_oct = tblone->tblone_disp_str; 
   }
   else
   {
      i_trap_varbind->i_param_val.i_length = 0;
      i_trap_varbind->i_param_val.info.i_oct = NULL; 
   }

   i_trap_varbind->i_idx.i_inst_len = args->i_idx.i_inst_len; 
   i_trap_varbind->i_idx.i_inst_ids = args->i_idx.i_inst_ids; 
   
   /* Add to the trap */
   snmptm_tblone.i_trap_vb = i_trap_varbind;
   temp_trap_varbind = snmptm_tblone.i_trap_vb;
                
   /* allocate memory for  ncsTestTableOneCounter32 */ 
   i_trap_varbind = (NCS_TRAP_VARBIND *)malloc(sizeof(NCS_TRAP_VARBIND)); 
   if (i_trap_varbind == NULL)
   {
      /* Probably need to call a function that free's the trap var bind list */
      free(snmptm_tblone.i_trap_vb);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 
                                                               
   /* Fill in the object ncsTestTableOneCounter32 details */ 
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_SNMPTM_TBLONE;
                                     
   i_trap_varbind->i_param_val.i_param_id = ncsTestTableOneCounter32_ID;  
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT; 
   i_trap_varbind->i_param_val.i_length = sizeof(tblone->tblone_cntr32); 
   i_trap_varbind->i_param_val.info.i_int = tblone->tblone_cntr32; 
                                                               
   i_trap_varbind->i_idx.i_inst_len = args->i_idx.i_inst_len; 
   i_trap_varbind->i_idx.i_inst_ids = args->i_idx.i_inst_ids; 
   /* Add to the trap */ 
   temp_trap_varbind->next_trap_varbind = i_trap_varbind;
   temp_trap_varbind = temp_trap_varbind->next_trap_varbind;
  
   /* allocate memory for  ncsTestTableOneRowStatus */ 
   i_trap_varbind = (NCS_TRAP_VARBIND *)malloc(sizeof(NCS_TRAP_VARBIND)); 
   if (i_trap_varbind == NULL)
   {
      /* Probably need to call a function that free's the trap var bind list */
      free(snmptm_tblone.i_trap_vb->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 
                                                               
   /* Fill in the object ncsTestTableOneRowStatus details */ 
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_SNMPTM_TBLONE;
                                     
   i_trap_varbind->i_param_val.i_param_id = ncsTestTableOneRowStatus_ID;  
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT; 
   i_trap_varbind->i_param_val.i_length = sizeof(tblone->tblone_row_status); 
   i_trap_varbind->i_param_val.info.i_int = tblone->tblone_row_status; 
                                                               
   i_trap_varbind->i_idx.i_inst_len = args->i_idx.i_inst_len; 
   i_trap_varbind->i_idx.i_inst_ids = args->i_idx.i_inst_ids; 
   /* Add to the trap */ 
   temp_trap_varbind->next_trap_varbind = i_trap_varbind;
   temp_trap_varbind = temp_trap_varbind->next_trap_varbind;

   /* to fix the crash in EDU */ 
   /* allocate memory for ncsTestScalarsUnsigned32 */ 
   i_trap_varbind = (NCS_TRAP_VARBIND *)malloc(sizeof(NCS_TRAP_VARBIND)); 
   if (i_trap_varbind == NULL)
   {
      /* Probably need to call a function that free's the trap var bind list */
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 
                                                               
   /* Fill in the object ncsTestTableOneRowStatus details */ 
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_SNMPTM_SCALAR;
                                     
   i_trap_varbind->i_param_val.i_param_id = ncsTestScalarsUnsigned32_ID;  
   i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT; 
   i_trap_varbind->i_param_val.i_length = sizeof(uns32); 
   i_trap_varbind->i_param_val.info.i_int = snmptm->scalars.scalar_uns32; 
                                                               
   i_trap_varbind->i_idx.i_inst_len = 0;
   i_trap_varbind->i_idx.i_inst_ids = 0;
   /* Add to the trap */ 
   temp_trap_varbind->next_trap_varbind = i_trap_varbind;
   temp_trap_varbind = temp_trap_varbind->next_trap_varbind;

   /* add the OBJECT IDENTIFIER as part of the trap */
   /* allocate memory for  ncsTestTableOneObjectIdentifier */ 
   i_trap_varbind = (NCS_TRAP_VARBIND *)malloc(sizeof(NCS_TRAP_VARBIND)); 
   if (i_trap_varbind == NULL)
   {
      /* Probably need to call a function that free's the trap var bind list */
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 
                                                               
   /* Fill in the object ncsTestTableOneRowStatus details */ 
   i_trap_varbind->i_tbl_id = NCSMIB_TBL_SNMPTM_TBLONE;
                                     
   i_trap_varbind->i_param_val.i_param_id = ncsTestTableOneObjectIdentifier_ID;  
   memset(temp_oid_stream, 0, sizeof(temp_oid_stream));
   ncsmib_oid_put(tblone->tblone_obj_id_len, tblone->tblone_obj_id, temp_oid_stream, &i_trap_varbind->i_param_val);
                                                               
   i_trap_varbind->i_idx.i_inst_len = args->i_idx.i_inst_len; 
   i_trap_varbind->i_idx.i_inst_ids = args->i_idx.i_inst_ids; 
   /* Add to the trap */ 
   temp_trap_varbind->next_trap_varbind = i_trap_varbind;
   temp_trap_varbind = temp_trap_varbind->next_trap_varbind;

   /* Encode the trap */ 
   /* 1. Get the length */ 
   tlv_size = ncs_tlvsize_for_ncs_trap_get(&snmptm_tblone); 

   /* Allocate the memory */
   encoded_buffer = (uns8*)malloc(tlv_size);
   if (encoded_buffer == NULL)
   {
      /* Probably need to call a function that free's the trap var bind list */
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_MEMSET(encoded_buffer, '\0', tlv_size);

   /* call the EDU macro to encode with buffer pointer and size */
   status = 
      m_NCS_EDU_TLV_EXEC(&snmptm->edu_hdl, ncs_edp_ncs_trap, encoded_buffer, 
                         tlv_size, EDP_OP_TYPE_ENC, &snmptm_tblone, &o_err);
   if (status != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("TLV-EXEC failed, error-value=%d...\n", o_err);
      /* Probably need to call a function that free's the trap var bind list */
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb);
      free(encoded_buffer);
      return status;
   }

   eventDataSize = (SaSizeT)tlv_size;
   
   /* Allocate memory for the event */
   saStatus = saEvtEventAllocate(snmptm->evtChannelHandle, &eventHandle);
   if (saStatus != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\nsaEvtEventAllocate failed\n");
      /* Probably need to call a function that free's the trap var bind list */
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb);
      free(encoded_buffer);
      return NCSCC_RC_FAILURE;
   }

   patternArray.patternsNumber = SNMPTM_TRAP_PATTERN_ARRAY_LEN;
   patternArray.patterns = snmptm_trap_pattern_array;

   /* Set all the writeable event attributes */
   saStatus = saEvtEventAttributesSet(eventHandle,
                                      &patternArray, 
                                      priority,
                                      retentionTime,
                                      &snmptm->publisherName);
   if (saStatus != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\nsaEvtEventAttributesSet failed\n");
      /* Probably need to call a function that free's the trap var bind list */
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb);
      free(encoded_buffer);
      return NCSCC_RC_FAILURE;
   }
   
   /* Publish an event on the channel */
   saStatus = saEvtEventPublish(eventHandle,
                                encoded_buffer,
                                eventDataSize,
                                &eventId);
   if (saStatus != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\nsaEvtEventPublish failed\n");
      /* Probably need to call a function that free's the trap var bind list */
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb);
      free(encoded_buffer);
      return NCSCC_RC_FAILURE;
   }

   /* Free the allocated memory for the event */
   saStatus = saEvtEventFree(eventHandle);
   if (saStatus != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\nsaEvtEventFree failed\n");
      /* Probably need to call a function that free's the trap var bind list */
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb->next_trap_varbind);
      free(snmptm_tblone.i_trap_vb);
      free(encoded_buffer);
      return NCSCC_RC_FAILURE;
   }

   /* Probably need to call a function that free's the trap var bind list */
   free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind->next_trap_varbind);
   free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind->next_trap_varbind);
   free(snmptm_tblone.i_trap_vb->next_trap_varbind->next_trap_varbind);
   free(snmptm_tblone.i_trap_vb->next_trap_varbind);
   free(snmptm_tblone.i_trap_vb);
   free(encoded_buffer);

   return status;
}

#endif

