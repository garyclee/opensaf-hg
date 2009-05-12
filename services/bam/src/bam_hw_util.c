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

  This file captures all the routines to parse and build the Hardware 
  Validation Structures in the BAM module. There is a seperate Parser 
  Instance associated with the Validation BOM.
  
******************************************************************************
*/

#include <config.h>

#include "bamHWEntities.h"
#include "bam_log.h"

EXTERN_C uns32 gl_ncs_bam_hdl;

/*****************************************************************************
  PROCEDURE NAME: get_entity_type_from_text
  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : To get the enum from the string parsed from XML
*****************************************************************************/

SaHpiEntityTypeT
get_entity_type_from_text(char *str)
{
   SaHpiEntityTypeT type = SAHPI_ENT_UNSPECIFIED;

   if(!str)
      return type;
   else
   {
      if(strcmp(str, "SAHPI_ENT_SYSTEM_BOARD") == 0)
         type = SAHPI_ENT_SYSTEM_BOARD;
      
      else if(strcmp(str, "SAHPI_ENT_PROCESSOR_MODULE") == 0)
         type = SAHPI_ENT_PROCESSOR_MODULE;

      else if(strcmp(str, "SAHPI_ENT_PROCESSOR") == 0)
         type = SAHPI_ENT_PROCESSOR;

      else if(strcmp(str, "SAHPI_ENT_POWER_SUPPLY") == 0)
         type = SAHPI_ENT_POWER_SUPPLY;

      else if(strcmp(str, "SAHPI_ENT_PROCESSOR_BOARD") == 0)
         type = SAHPI_ENT_PROCESSOR_BOARD;

      else if(strcmp(str, "SAHPI_ENT_SYSTEM_CHASSIS") == 0)
         type = SAHPI_ENT_SYSTEM_CHASSIS;

      else if(strcmp(str, "SAHPI_ENT_ADVANCEDTCA_CHASSIS") == 0)
         type = SAHPI_ENT_ADVANCEDTCA_CHASSIS;
		 
      else if(strcmp(str, "SAHPI_ENT_RACK") == 0)
         type = SAHPI_ENT_RACK;

#ifndef HAVE_HPI_A01
      else if(strcmp(str, "SAHPI_ENT_SYSTEM_BLADE") == 0)
         type = SAHPI_ENT_SYSTEM_BLADE;
#endif

      else if(strcmp(str, "SAHPI_ENT_IO_BLADE") == 0)
         type = SAHPI_ENT_IO_BLADE;

      else if(strcmp(str, "SAHPI_ENT_IO_SUBBOARD") == 0)
         type = SAHPI_ENT_IO_SUBBOARD;

      else if(strcmp(str, "SAHPI_ENT_SBC_SUBBOARD") == 0)
         type = SAHPI_ENT_SBC_SUBBOARD;

      else if(strcmp(str, "SAHPI_ENT_SUBBOARD_CARRIER_BLADE") == 0)
         type = SAHPI_ENT_SUBBOARD_CARRIER_BLADE;

      else if(strcmp(str, "SAHPI_ENT_SBC_BLADE") == 0)
         type = SAHPI_ENT_SBC_BLADE;
#ifndef HAVE_HPI_A01
      else if(strcmp(str, "SAHPI_ENT_PICMG_FRONT_BLADE") == 0)
         type = SAHPI_ENT_PICMG_FRONT_BLADE;

      else if(strcmp(str, "SAHPI_ENT_SWITCH_BLADE") == 0)
         type = SAHPI_ENT_SWITCH_BLADE;

      else if(strcmp(str, "AMC_SUB_SLOT_TYPE") == 0)
         type = AMC_SUB_SLOT_TYPE;
#endif
      else if(strcmp(str, "SAHPI_ENT_PHYSICAL_SLOT") == 0)
#ifdef HAVE_HPI_A01
         type = SAHPI_ENT_SYSTEM_SLOT;
#else
         type = SAHPI_ENT_PHYSICAL_SLOT;
#endif
   }

   return type;
}


/*****************************************************************************
  PROCEDURE NAME: ent_path_from_type_location
  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : From the ent_name field in the BAM_ENT_DEPLOY_DESC structure. 
                  Get the corresponding entity from the PAT_TREE and get 
                  the entity_type. Now add to the existing ent_path.
                  If the ent_path is NULL this will be the first one.

*****************************************************************************/
void
ent_path_from_type_location(BAM_ENT_DEPLOY_DESC *ent, char *ent_path)
{
   char        tmpStr[6];

   if((NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return ;
   }

   ncshm_give_hdl(gl_ncs_bam_hdl);   

   if(ent->ent_type == 0)
   {
      return ;
   }

   memset(ent->ent_path, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
   strncpy(ent->ent_path, "{", SAHPI_MAX_TEXT_BUFFER_LENGTH-1);

   /* put the type and location */

   sprintf(tmpStr, "%d", ent->ent_type);
   strncat(ent->ent_path, tmpStr, SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);

   strncat(ent->ent_path, ",", SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);

   sprintf(tmpStr, "%d", ent->location);
   strncat(ent->ent_path, tmpStr, SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);

   strncat(ent->ent_path, "}", SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);
   
   /* now pile the parent path */
   if( (ent_path != NULL) && (strlen(ent_path)) )
   {
      strncat(ent->ent_path, ",", SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);
      strncat(ent->ent_path, ent_path, SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);
   }
   else
   { 
      /* Add root elem towards the end */
      sprintf(tmpStr, "%d", SAHPI_ENT_ROOT);
      strncat(ent->ent_path, ",{", SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);
      strncat(ent->ent_path, tmpStr, SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);

      strncat(ent->ent_path, ",", SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);

      sprintf(tmpStr, "%d", 0);
      strncat(ent->ent_path, tmpStr, SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);

      strncat(ent->ent_path, "}", SAHPI_MAX_TEXT_BUFFER_LENGTH-strlen(ent->ent_path)-1);
   }

   return ;
}


/*****************************************************************************
  PROCEDURE NAME: bam_clear_and_destroy_deploy_tree
  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : This routine helps to free the tree of all deployment
                  entities after the mibsets have been sent to the AvM.
*****************************************************************************/

void bam_clear_and_destroy_deploy_tree(void)
{
   NCS_BAM_CB           *bam_cb = NULL;
   BAM_ENT_DEPLOY_DESC  *ent = NULL;
   char                 *ent_name = NULL;

   if((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return ;
   }

   do
   {
      ent = (BAM_ENT_DEPLOY_DESC *)
            ncs_patricia_tree_getnext( &bam_cb->deploy_ent_anchor,
                                        (uns8 *)ent_name); 

      if(ent != NULL)
      {
         ent_name = ent->desc_name;
         ncs_patricia_tree_del(&bam_cb->deploy_ent_anchor, &ent->tree_node);
      }
   }while(ent != NULL);

   ncshm_give_hdl(gl_ncs_bam_hdl);   
   return;
}

/*****************************************************************************
  PROCEDURE NAME: bam_add_deploy_ent_list
  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : This routine helps to add the deployment entity to the
                  list so that at mib set time the sets can be sent in 
                  order.
*****************************************************************************/

void bam_add_deploy_ent_list(NCS_BAM_CB* bam_cb, BAM_ENT_DEPLOY_DESC *ent)
{
   BAM_ENT_DEPLOY_DESC_LIST_NODE *tmp = NULL;
   BAM_ENT_DEPLOY_DESC_LIST_NODE *end = NULL, *prev=NULL;

   if(ent == NULL)
      return;

   tmp = (BAM_ENT_DEPLOY_DESC_LIST_NODE *)
          m_MMGR_ALLOC_BAM_DEFAULT_VAL(sizeof(BAM_ENT_DEPLOY_DESC_LIST_NODE));

   if(tmp == NULL)
   {
      m_LOG_BAM_MEMFAIL(BAM_ALLOC_FAILED);
      return;
   }

   tmp->node = ent;
   tmp->next = NULL;

   end = bam_cb->deploy_list;
   while(end != NULL)
   {
      prev = end;
      end = end->next;
   }

   if(prev == NULL)
   {
      /* first node */
      bam_cb->deploy_list = tmp;
   }
   else
   {
      prev->next = tmp;
   }

   return;
}


/*****************************************************************************
  PROCEDURE NAME: bam_clear_and_destroy_deploy_list
  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : This list is to send the MIB sets in order. This routine
                  helps to delete the entries after the mibsets have been
                  sent.
*****************************************************************************/

void bam_clear_and_destroy_deploy_list(void)
{
   NCS_BAM_CB *bam_cb = NULL;
   BAM_ENT_DEPLOY_DESC_LIST_NODE *tmp= NULL, *next=NULL;

   if((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return ;
   }

   tmp = bam_cb->deploy_list;
   next = tmp;
   while(next != NULL)
   {
      next = tmp->next;
      m_MMGR_FREE_BAM_DEFAULT_VAL(tmp);
      tmp = next;
   }
   bam_cb->deploy_list = NULL;

   ncshm_give_hdl(gl_ncs_bam_hdl);   
   return;
}

/*****************************************************************************
  PROCEDURE NAME: bam_delete_hw_ent_list
  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : This routine deletes the entity list after the message has
                  been sent to AvM.
*****************************************************************************/

void bam_delete_hw_ent_list(void)
{
   NCS_BAM_CB *bam_cb = NULL;
   NCS_HW_ENT_TYPE_DESC    *next= NULL, *tmp =NULL;

   if((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return ;
   }

   tmp = bam_cb->hw_entity_list;
   next = tmp;
   while(next != NULL)
   {
      next = tmp->next;
      m_MMGR_FREE_BAM_DEFAULT_VAL(tmp);
      tmp = next;
   }
   bam_cb->hw_entity_list = NULL;

   ncshm_give_hdl(gl_ncs_bam_hdl);   
   return;
}


/*****************************************************************************
  PROCEDURE NAME: bam_avm_send_validation_config
  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : This routine sends all the validation configuration
                  information as one message to the AvM. 
*****************************************************************************/

uns32 bam_avm_send_validation_config(void)
{
   NCS_BAM_CB *bam_cb = NULL;
   NCS_HW_ENT_TYPE_DESC    *entity_desc = NULL;
   BAM_AVM_MSG_T *snd_msg = m_MMGR_ALLOC_BAM_AVM_MSG;

   if((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   entity_desc = bam_cb->hw_entity_list;
   if(entity_desc != NULL)
   {
      /* Fill in the message */
      memset(snd_msg,'\0',sizeof(BAM_AVM_MSG_T));
      snd_msg->msg_type = BAM_AVM_HW_ENT_INFO;
      snd_msg->msg_info.ent_info = entity_desc;

      if(bam_avm_snd_message(snd_msg) != NCSCC_RC_SUCCESS)
      {
         m_MMGR_FREE_BAM_AVM_MSG(snd_msg);
      }
   }
   return NCSCC_RC_SUCCESS;
}

