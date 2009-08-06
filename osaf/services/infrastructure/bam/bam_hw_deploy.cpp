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
  deployment BOM. The values are validated and then the MIB-SETS are generated. 

  The dependsOnForActivation clause points to a entity deployment instance
  which might or might not have been parsed. However, the MIB for this expects
  a ent_path as opposed to name. We build the ent_path as we parse each entity
  deployment instance and hence the ent_path for the dependent entity 
  might or might not be available to do the mib-set. This forces us to 
  store the info in data structures and then do the Validation and MIB-sets.

******************************************************************************
*/

/* NCS specific include files */

#include <ncs_opt.h>
#include <gl_defs.h>
#include <t_suite.h>
#include <ncs_mib_pub.h>
#include <ncsmiblib.h>
#include "bamHWEntities.h"
#include "bam_log.h"
#include "bamParser.h"
#include "bamError.h"

EXTERN_C uns32 gl_ncs_bam_hdl;

static SaAisErrorT
bam_send_hw_deployment_mibs(void);

SaAisErrorT 
parseDeploymentInstance(DOMNode *, char *, char *);

void
parse_net_boot_childs(DOMNode *, BAM_ENT_DEPLOY_DESC *);

SaAisErrorT
bam_fill_dependant_ent_name(DOMNode *node,  BAM_ENT_DEPLOY_DESC *deploy_ent)
{
   DOMNamedNodeMap *attributesNodes = node->getAttributes();

   if(attributesNodes->getLength() )
   {
      for(unsigned int x=0; x < attributesNodes->getLength(); x++)
      {
         char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
         char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());

         if(strcmp(tag, "EntityDeploymentInstanceName") == 0)
         {
            strncpy(deploy_ent->depends_on_for_act, val, NCS_MAX_INDEX_LEN-1);
         }
         XMLString::release(&tag);
         XMLString::release(&val);
      }
   } /* End of parsing Attributes */

   return SA_AIS_OK;
}

static SaAisErrorT
bam_parse_child_entities(DOMNode *node, char *ent_path, char *parent_ent)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode                 *tmpNode = NULL;
   /* for each child node call the deployment instance function */
   DOMNodeList * children = node->getChildNodes();

   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No child deployment entities found. ");
      return rc;
   }

   for(unsigned x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         
         if(strcmp(tmpString, "EntityDeploymentInstance") == 0)
         {
            rc = parseDeploymentInstance(tmpNode, ent_path, parent_ent);
         }
         
         XMLString::release(&tmpString);
      }
   }/* End of parsing child elements */
   return rc;
}

SaAisErrorT
parseDeploymentInstance(DOMNode *node, char *ent_path, char *parent_ent)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode                 *tmpNode = NULL;
   DOMNode                 *containsNode = NULL;
   BAM_ENT_DEPLOY_DESC     *deploy_ent;
   NCS_BAM_CB              *bam_cb = NULL;

   /* Check to make sure that this is an ELEMENT NODE */
   if(node->getNodeType() != DOMNode::ELEMENT_NODE)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
          "Error in processing the DOM Tree. Expected Deployment Instance.");
      return SA_AIS_ERR_INVALID_PARAM;
   }

   /* Allocate memory for the entity instance */
   deploy_ent = (BAM_ENT_DEPLOY_DESC *) 
                  m_MMGR_ALLOC_BAM_DEFAULT_VAL(sizeof(BAM_ENT_DEPLOY_DESC));

   if(!deploy_ent)
   {
      m_LOG_BAM_MEMFAIL(BAM_ALLOC_FAILED);
      return SA_AIS_ERR_NO_MEMORY;
   }

   memset(deploy_ent, 0, sizeof(BAM_ENT_DEPLOY_DESC));
   /* set defaults */
   deploy_ent->isActivationSourceNCS = 2;

   DOMNodeList * children = node->getChildNodes();

   if(children->getLength() == 0)
   {
      m_MMGR_FREE_BAM_DEFAULT_VAL(deploy_ent);
      return rc;
   }

   for(unsigned x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tag = XMLString::transcode(tmpNode->getNodeName());
         char *val = NULL;
         
         /* NOTE:-  XML will send the value 1 if netBoot and 2 if diskBoot else nothing in the mib*/
         if(strcmp(tag, "netBoot") == 0)
         { /* Call the function to parse the all child of netBoot tag here 
              as well set the netBoot tag value in data structure as 1 */

            deploy_ent->netBoot = 1;
            parse_net_boot_childs(tmpNode, deploy_ent);
         }
         else if(strcmp(tag, "diskBoot") == 0)
         {
           /* set the netBoot tag value in data structure as 2 */
            deploy_ent->netBoot = 2;
         }
         else if(strcmp(tag, "contains") == 0)
         {
            containsNode = tmpNode;
            XMLString::release(&tag);
            continue;
         }
         else if(strcmp(tag, "dependsOnForActivation") == 0)
         {
            rc = bam_fill_dependant_ent_name(tmpNode, deploy_ent);
         }
         else
         {
            val = XMLString::transcode(tmpNode->getTextContent());

            if(strcmp(tag, "EntityTypeInstanceName") == 0)
            {
               strncpy(deploy_ent->ent_name, val, NCS_MAX_INDEX_LEN-1);
            }
            else if(strcmp(tag, "EntityLocation") == 0)
            {
               deploy_ent->location = atoi(val);
            }
            else if(strcmp(tag, "NodeName") == 0)
            {
               strncpy(deploy_ent->ncs_node_name, val, NCS_MAX_INDEX_LEN-1);
            }
            else if(strcmp(tag, "Name") == 0)
            {
               strncpy(deploy_ent->desc_name, val, NCS_MAX_INDEX_LEN-1);
            }
            else if(strcmp(tag, "HPIEntityType") == 0)
            {
               deploy_ent->ent_type = get_entity_type_from_text(val);
            }
            else if(strcmp(tag, "isActivationSourceNCS") == 0)
            {
               if(strcmp(val,"1") == 0)
               {
                  deploy_ent->isActivationSourceNCS =  1;
               }
               else if(strcmp(val,"3") == 0)
               {
                  deploy_ent->isActivationSourceNCS =  3;
               }
               else
               {
                  deploy_ent->isActivationSourceNCS =  2;
               }
            }

            XMLString::release(&val);
         }
         
         XMLString::release(&tag);
      }
   } /* End of parsing child elements */
   
   /* Construct the entity path and then add to the tree */
   ent_path_from_type_location(deploy_ent, ent_path);

   /* If the control reach here the deploy_ent structure is
   ** populated. Add parent_ent and Add to the patricia tree ??
   */
   
   if(parent_ent)
   {
      strncpy(deploy_ent->parent_ent_name, parent_ent, NCS_MAX_INDEX_LEN-1);
   }
     
   if((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      m_MMGR_FREE_BAM_DEFAULT_VAL(deploy_ent);
      return SA_AIS_ERR_BAD_HANDLE;
   }

   deploy_ent->tree_node.key_info = (uns8*)(deploy_ent->desc_name);
   deploy_ent->tree_node.bit   = 0;
   deploy_ent->tree_node.left  = NCS_PATRICIA_NODE_NULL;
   deploy_ent->tree_node.right = NCS_PATRICIA_NODE_NULL;

   if( ncs_patricia_tree_add(&bam_cb->deploy_ent_anchor,
                          &deploy_ent->tree_node) != NCSCC_RC_SUCCESS)
   {
      m_LOG_BAM_MSG_TIC(BAM_ENT_PAT_ADD_FAIL, NCSFL_SEV_ERROR, 
          "Error ADDING entity to Tree .");

      m_MMGR_FREE_BAM_DEFAULT_VAL(deploy_ent);
      return SA_AIS_ERR_INVALID_PARAM;
   }

   m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                      "Parsed ENTITY TYPE INSTANCE Successfully. ");

   /* Now add it to the list also */
   bam_add_deploy_ent_list(bam_cb, deploy_ent);

   ncshm_give_hdl(gl_ncs_bam_hdl);   

   /* Now call the recursive function to handle the contained list */

   if(containsNode)
      return bam_parse_child_entities(containsNode, deploy_ent->ent_path,
                                      deploy_ent->ent_name);

   return SA_AIS_OK;
}

/*****************************************************************************
  PROCEDURE NAME: parse_net_boot_childs
  DESCRIPTION   :
  ARGUMENTS     : 

  RETURNS       : 

  NOTES         : 

*****************************************************************************/
void
parse_net_boot_childs(DOMNode *node, BAM_ENT_DEPLOY_DESC *deploy_ent)
{
   DOMNode *tmpNode=NULL;

   DOMNodeList * children = node->getChildNodes();
   
   for(unsigned x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tag = XMLString::transcode(tmpNode->getNodeName());
         char *val = XMLString::transcode(tmpNode->getTextContent());

         if(strcmp(tag, "tftpServIP") == 0)
         {
            uns32 int_val = inet_addr(val);
            char ptr[5];
            memcpy(ptr, (char *)&int_val, sizeof(uns32));
            ptr[4] = '\0';
            
            strncpy(deploy_ent->tftpServIp, ptr, NCS_MAX_INDEX_LEN-1);
         }
         else if(strcmp(tag, "label1Name") == 0)
         {
            strncpy(deploy_ent->label1Name, val, NCS_MAX_INDEX_LEN-1);
         }
         else if(strcmp(tag, "label1FileName") == 0)
         {
            strncpy(deploy_ent->label1FileName, val, NCS_MAX_INDEX_LEN-1);
         }
         else if(strcmp(tag, "label2Name") == 0)
         {
            strncpy(deploy_ent->label2Name, val, NCS_MAX_INDEX_LEN-1);
         }
         else if(strcmp(tag, "label2FileName") == 0)
         {
             strncpy(deploy_ent->label2FileName, val, NCS_MAX_INDEX_LEN-1);
         }
         else if(strcmp(tag, "preferredLabel") == 0)
         {
            strncpy(deploy_ent->preferredLabel, val, NCS_MAX_INDEX_LEN-1);
         }
 
      }
   }  
}


/*****************************************************************************
  PROCEDURE NAME: parse_hw_deploy_config
  DESCRIPTION   : 
  ARGUMENTS     : parsed DOM node.
                  
  RETURNS       : 0 for SUCCESS else non-zero for FAILURE

  NOTES         : This handles the hardware deployemnt BOM.

*****************************************************************************/

SaAisErrorT
parse_hw_deploy_config(DOMNode *node)
{
   SaAisErrorT     rc = SA_AIS_OK;
   DOMNode *tmpNode = NULL;
   DOMNodeList *children = node->getChildNodes();

   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No deployment entities found . ");
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         
         if(strcmp(tmpString, "EntityDeploymentInstance") == 0)
            rc = parseDeploymentInstance(tmpNode, NULL, NULL);
         
         if(rc != SA_AIS_OK)
            m_LOG_BAM_MSG_TICL(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
                        "Error encountered parsing: ", tmpString);

         XMLString::release(&tmpString);
      }
   }

   m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "Succesfully parsed HW Deployment BOM: ");

   /* Now send all the mibsets walking down the tree */
   if(rc == SA_AIS_OK)
   {
      rc = bam_send_hw_deployment_mibs();
   }

   /* now delete the tree and the list */
   bam_clear_and_destroy_deploy_tree();
   bam_clear_and_destroy_deploy_list();
   return rc;
}

/*****************************************************************************
  PROCEDURE NAME: bam_send_hw_deployment_mibs
  DESCRIPTION   : 
  ARGUMENTS     : NONE
                  
  RETURNS       : 0 for SUCCESS else non-zero for FAILURE

  NOTES         : This handles sending MIBS for the hardware deployemnt BOM.

*****************************************************************************/

static SaAisErrorT
bam_send_hw_deployment_mibs(void)
{
   NCS_BAM_CB           *bam_cb = NULL;
   BAM_ENT_DEPLOY_DESC  *ent = NULL;
   BAM_ENT_DEPLOY_DESC  *dep_ent = NULL;
   NCSMIB_IDX           mib_idx;
   char                 ent_path[BAM_MAX_INDEX_LEN];
   char                 dep_ent_path[BAM_MAX_INDEX_LEN];
   char                 *ent_name = NULL;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   char                 tmpVal[4];
   SaAisErrorT    	   rc=SA_AIS_OK;
   BAM_ENT_DEPLOY_DESC_LIST_NODE *list_node = NULL;

   if((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return SA_AIS_ERR_BAD_HANDLE;
   }

   do
   {

      ent = (BAM_ENT_DEPLOY_DESC *)
            ncs_patricia_tree_getnext( &bam_cb->deploy_ent_anchor,
                                        (uns8 *)ent_name); 

      if(ent != NULL)
      {
         /* send the related MIBS */
          memset(ent_path, 0, BAM_MAX_INDEX_LEN);
          strncpy(ent_path, "{", BAM_MAX_INDEX_LEN-1);
          strncat(ent_path, ent->ent_path, BAM_MAX_INDEX_LEN-strlen(ent_path)-1);
          strncat(ent_path, "}", BAM_MAX_INDEX_LEN-strlen(ent_path)-1);

	       ncs_bam_build_mib_idx(&mib_idx, ent_path, NCSMIB_FMAT_OCT);	

         /* Do it in order.... is there a better way?? */

         rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt, 
                              gl_hwDeploymentEnt_size,
                              "hwDeployment", 
                              "NodeName", &table_id, &param_id, &format);
                              
         if(rc == SA_AIS_OK)
            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 	
                              ent->ncs_node_name, format);

         rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt, 
                              gl_hwDeploymentEnt_size,
                              "hwDeployment", 
                              "EntName", &table_id, &param_id, &format);
                              
         if(rc == SA_AIS_OK)
            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 	
                              ent->ent_name, format);

         rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt, 
                              gl_hwDeploymentEnt_size,
                              "hwDeployment", 
                              "ParentEntName", &table_id, &param_id, &format);
                              
         if(rc == SA_AIS_OK)
            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 	
                              ent->parent_ent_name, format);

         rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt, 
                              gl_hwDeploymentEnt_size,
                              "hwDeployment", 
                              "isActivationSourceNCS", &table_id, &param_id, &format);
                              
         if(rc == SA_AIS_OK)
         {
            if (ent->isActivationSourceNCS == 1)
               ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 	
                                                  (char *)("1"), format);
            else if (ent->isActivationSourceNCS == 3)
               ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 	
                                                  (char *)("3"), format);
            else
               ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 	
                                                  (char *)("2"), format);
         }
     
         if((ent->netBoot == 1) || (ent->netBoot == 2) )
         {
            rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt,
                              gl_hwDeploymentEnt_size,
                              "hwDeployment",
                              "netBoot", &table_id, &param_id, &format);

            if((rc == SA_AIS_OK) && (ent->netBoot == 1))
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx,
                                 (char *)("1"), format);

            else if((rc == SA_AIS_OK) && (ent->netBoot == 2))
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx,
                                 (char *)("0"), format);
	
            if(ent->netBoot == 1)
            {
               rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt,
                              gl_hwDeploymentEnt_size,
                              "hwDeployment",
                              "tftpServIP", &table_id, &param_id, &format);

               if(rc == SA_AIS_OK)
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx,
                                 ent->tftpServIp, format);
              
               rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt,
                              gl_hwDeploymentEnt_size,
                              "hwDeployment",
                              "label1Name", &table_id, &param_id, &format);

               if(rc == SA_AIS_OK)
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx,
                                 ent->label1Name, format);

              rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt,
                              gl_hwDeploymentEnt_size,
                              "hwDeployment",
                              "label1FileName", &table_id, &param_id, &format);

               if(rc == SA_AIS_OK)
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx,
                                 ent->label1FileName, format);

               rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt,
                           gl_hwDeploymentEnt_size,
                           "hwDeployment",
                           "label2Name", &table_id, &param_id, &format);

               if(rc == SA_AIS_OK)
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx,
                                 ent->label2Name, format);
               rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt,
                           gl_hwDeploymentEnt_size,
                           "hwDeployment",
                           "label2FileName", &table_id, &param_id, &format);

               if(rc == SA_AIS_OK)
                   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx,
                              ent->label2FileName, format);

               rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt,
                              gl_hwDeploymentEnt_size,
                              "hwDeployment",
                              "preferredLabel", &table_id, &param_id, &format);

               if(rc == SA_AIS_OK)
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx,
                                 ent->preferredLabel, format);
            }
         }


         rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt, 
                              gl_hwDeploymentEnt_size,
                              "hwDeployment", 
                              "DependsOn", &table_id, &param_id, &format);
                              
         if(rc == SA_AIS_OK)
         {
            dep_ent = (BAM_ENT_DEPLOY_DESC *)
                       ncs_patricia_tree_get(&bam_cb->deploy_ent_anchor,
                                              (uns8 *)ent->depends_on_for_act);
            if(dep_ent != NULL)
            {
                memset(dep_ent_path, 0, BAM_MAX_INDEX_LEN);
                strncpy(dep_ent_path, "{", BAM_MAX_INDEX_LEN-1);
                strncat(dep_ent_path, dep_ent->ent_path, BAM_MAX_INDEX_LEN-strlen(dep_ent_path)-1);
                strncat(dep_ent_path, "}", BAM_MAX_INDEX_LEN-strlen(dep_ent_path)-1);

               if(strlen(dep_ent_path))
               {
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 	
                                    dep_ent_path, format);
               }
            }
         }

         ncs_bam_free(mib_idx.i_inst_ids);

         ent_name = ent->desc_name;
      }
   }while(ent != NULL);

   /* Now set all the row_status active for all the entities in the parent
   ** order first so that all the mibs can be validated
   */

   list_node = (BAM_ENT_DEPLOY_DESC_LIST_NODE *)bam_cb->deploy_list;

   while(list_node)
   {
      ent = list_node->node;

      memset(ent_path, 0, BAM_MAX_INDEX_LEN);
      strncpy(ent_path, "{", BAM_MAX_INDEX_LEN-1);
      strncat(ent_path, ent->ent_path, BAM_MAX_INDEX_LEN-strlen(ent_path)-1);
      strncat(ent_path, "}", BAM_MAX_INDEX_LEN-strlen(ent_path)-1);

      ncs_bam_build_mib_idx(&mib_idx, ent_path, NCSMIB_FMAT_OCT);	
      
      rc = ncs_bam_search_table_for_oid(gl_hwDeploymentEnt, 
                           gl_hwDeploymentEnt_size,
                           "hwDeployment", 
                           "rowStatus", &table_id, &param_id, &format);
                           
      if(rc == SA_AIS_OK)
      {
         sprintf(tmpVal, "%d", NCSMIB_ROWSTATUS_ACTIVE);
         ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 	
                           tmpVal, format);
      }

      ncs_bam_free(mib_idx.i_inst_ids);
      list_node = list_node->next;
   }

   ncshm_give_hdl(gl_ncs_bam_hdl);   
   return SA_AIS_OK;
}
