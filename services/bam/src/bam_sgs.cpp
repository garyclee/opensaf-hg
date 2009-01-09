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

  This file captures all the routines to parse and generate the mibsets for the
  SAF specified configurable objects in the AMF.
  
******************************************************************************
*/


/* NCS specific include files */
#include <ncs_opt.h>
#include <gl_defs.h>
#include <t_suite.h>
#include <ncs_mib_pub.h>

#include <saAis.h>
#include "bam.h"
#include "bamParser.h"
#include "ncsmiblib.h"
#include "bam_log.h"

XERCES_CPP_NAMESPACE_USE

#define NCS_SG_DEFAULT_SU_TYPE 1
#define NCS_SI_DEFAULT_SU_TYPE 1

static SaAisErrorT
saAmfParseSISURankList(DOMNode *, char *);

static SaAisErrorT
saAmfParseCSIParamNameList(DOMNode *, char *);

/*****************************************************************************
  PROCEDURE NAME: saAmfParseSGErrorEsc

  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseSGErrorEsc(DOMNode *node, char *sgName)
{
   SaAisErrorT       rc = SA_AIS_OK;
   DOMNode        *tmpNode = NULL;
   NCSMIB_TBL_ID  table_id;   /* table id */
   uns32             param_id; 
   NCSMIB_FMAT_ID    format;
   NCSMIB_IDX        mib_idx;

   DOMNodeList * children = node->getChildNodes();

   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR,
          "Missing SG Error Escalation policy");
      return rc; /* sending SA_AIS_OK is ok i guess */
   }

   ncs_bam_build_mib_idx(&mib_idx, sgName, NCSMIB_FMAT_OCT);

   for(uns32 x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tag = XMLString::transcode(tmpNode->getNodeName());
         char *val = XMLString::transcode(tmpNode->getTextContent());

         rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
            gl_amfConfig_table_size,
            "SGInstance", 
            tag, &table_id, &param_id, &format);


         if(rc != SA_AIS_OK)
         {
            m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, tag);
            /*return rc; */
            XMLString::release(&tag);
            XMLString::release(&val);
            continue;
         }

         if((m_NCS_STRCMP(tag, "componentRestartProbation") == 0) ||
            (m_NCS_STRCMP(tag, "SURestartProbation") == 0))
         {
            ncs_bam_generate_counter64_mibset(table_id, param_id, 
                                              &mib_idx, val); 
         }
         else 
         {
            ncs_bam_build_and_generate_mibsets(table_id, param_id, 
                                            &mib_idx, val, format); 
         }

         XMLString::release(&tag);
         XMLString::release(&val);
      }
   }
   ncs_bam_free(mib_idx.i_inst_ids);
   return rc;
}

static SaAisErrorT
saAmfParseSGVendorExt(DOMNode *node, char *sgName)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNodeList          *children = NULL;
   DOMNode              *tmpNode = NULL;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_IDX           mib_idx;
   NCSMIB_FMAT_ID       format;
   char			isNCS[4];
   
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR,sgName);
      return SA_AIS_ERR_INVALID_PARAM;
   }
   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
             "No extensions found. Parsing DONE", sgName);
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         char *val = XMLString::transcode(tmpNode->getTextContent());

         if(m_NCS_STRCMP(tmpString, "isNCS") == 0)
         {
	    /* To synchronize with true/false supported by xml file as a value of isNCS element */
	    if( (m_NCS_STRCMP(val, "0") == 0) || (m_NCS_STRCMP(val, "false") == 0) )
	    {
	      sprintf(isNCS, "%d", FALSE);
	    }
	    else
	    {
	      sprintf(isNCS, "%d", TRUE);
	    }

            /* set the Sg name and isNCS in NCS SG Table!! */
            ncs_bam_build_mib_idx(&mib_idx, sgName, NCSMIB_FMAT_OCT);

            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, gl_amfConfig_table_size,
                                    "SGInstance", 
                                    tmpString, &table_id, &param_id, &format);
            
            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, isNCS, format); 
            ncs_bam_free(mib_idx.i_inst_ids);
         }

         XMLString::release(&tmpString);
         XMLString::release(&val);
      }
   }
   return SA_AIS_OK;
}

static SaAisErrorT
saAmfParseSURankList(DOMNode *node, char *sgName)
{
   SaAisErrorT       rc = SA_AIS_OK;
   DOMNode           *tmpNode = NULL;
   NCSMIB_TBL_ID     table_id;
   uns32             param_id; 
   NCSMIB_FMAT_ID    format;
   NCSMIB_IDX        mib_idx;
   char              suName[BAM_MAX_INDEX_LEN];
   char              suRank[4];

   m_NCS_MEMSET(suName, 0, BAM_MAX_INDEX_LEN);
   DOMNodeList * children = node->getChildNodes();

   for(uns32 x =0; x< children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(m_NCS_STRCMP(tmpString, "SU") == 0)
         {
            /* Get the Attributes for that node */
            DOMNamedNodeMap *attributesNodes = tmpNode->getAttributes();
            if(attributesNodes->getLength() )
            {
               for(unsigned int x=0; x < attributesNodes->getLength(); x++)
               {
                  char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
                  char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());
                  if(m_NCS_STRCMP(tag, "SUName") == 0)
                  {
                     m_NCS_MEMSET(suName, 0, BAM_MAX_INDEX_LEN);
                     m_NCS_STRCPY(suName, val);
                  }
                  else if(m_NCS_STRCMP(tag, "rank") == 0)
                  {
                     m_NCS_STRCPY(suRank, val);
                  }

                  XMLString::release(&tag);
                  XMLString::release(&val);
               }
            }

            /* Set the Rank and parent SG name in the SU table */
            ncs_bam_build_mib_idx(&mib_idx, suName, NCSMIB_FMAT_OCT);

            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
               gl_amfConfig_table_size, "SUInstance", 
               "parentSGName", &table_id, &param_id, &format);

            if(rc == SA_AIS_OK)
            {
               ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                               sgName, format);
            }

            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
               gl_amfConfig_table_size, "SUInstance", 
               "suRank", &table_id, &param_id, &format);

            if(rc == SA_AIS_OK)
            {
               ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                               suRank, format);
            }
         } /* End of parsing each SU */

         XMLString::release(&tmpString);
      }
   } /* end of complete SURankList */
   return SA_AIS_OK;
}



static SaAisErrorT
saAmfParseMoreRedElems(DOMNode *node, char *sgName, uns32 red_model)
{
   SaAisErrorT       rc = SA_AIS_OK;
   DOMNode           *tmpNode = NULL;
   NCSMIB_TBL_ID     table_id;   /* table id */
   uns32             param_id; 
   NCSMIB_FMAT_ID    format;
   NCSMIB_IDX        mib_idx;
   char              redVal[4];

   /* Set the redundancy model in the SG table for all the models
   ** Then check and parse further for only selected models
   */
   sprintf(redVal, "%d", red_model);

   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, gl_amfConfig_table_size,
               "SGInstance",
               "redundancyModel", &table_id, &param_id, &format);
   if(rc != SA_AIS_OK)
   {
      m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, "redundancyModel");
      return rc;
   }

   ncs_bam_build_mib_idx(&mib_idx, sgName, NCSMIB_FMAT_OCT);

   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, redVal, format);

#if 0  /* Attribute "N" is replaced by "numberofAssignedSUs" element in case of NWayActive model also*/

   if(red_model ==  NCS_BAM_SG_REDMODEL_NWAYACTIVE)
   {
      /* Get the Attributes for that node */
      DOMNamedNodeMap *attributesNodes = node->getAttributes();
      if(attributesNodes->getLength() )
      {
         for(unsigned int x=0; x < attributesNodes->getLength(); x++)
         {
            char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
            char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());
            if(m_NCS_STRCMP(tag, "N") == 0)
            {
               /* Set the N value in the SG table */
               rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
               gl_amfConfig_table_size, "SGInstance", 
               "valueN", &table_id, &param_id, &format);

               if(rc == SA_AIS_OK)
               {
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, 
                                       &mib_idx, val, format);
               }
            }
            XMLString::release(&val);
            XMLString::release(&tag);
         } /* End for loop */
      } /* end of attributes */
   } /* End of NWAY ACTIVE */

#endif

   /* Parse the SU Ranked list and other elements for the related
   ** Redundancy Models only 
   */
   DOMNodeList * children = node->getChildNodes();

   for(uns32 x =0; x< children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         char *val = XMLString::transcode(tmpNode->getTextContent());

         if(m_NCS_STRCMP(tmpString, "SURankList") == 0)
         {
            saAmfParseSURankList(tmpNode, sgName);
            XMLString::release(&tmpString);
            XMLString::release(&val);
            continue;
         }
         else if(m_NCS_STRCMP(tmpString, "failbackOption") == 0)
         {
            /* This is a boolean */
            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, gl_amfConfig_table_size,
                                    "SGInstance", 
                                    tmpString, &table_id, &param_id, &format);
            if(rc != SA_AIS_OK)
            {
               m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, 
                                       tmpString);
               XMLString::release(&tmpString);
               XMLString::release(&val);
               continue;
            }

            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                               val, format);

            XMLString::release(&tmpString);
            XMLString::release(&val);
            continue;
         }
         rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, gl_amfConfig_table_size,
               "SGInstance",
               tmpString, &table_id, &param_id, &format);
         if(rc != SA_AIS_OK)
         {
            m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, tmpString);
            XMLString::release(&tmpString);
            XMLString::release(&val);
            continue;
         }

         ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, val, format);

         XMLString::release(&tmpString);
         XMLString::release(&val);
      }
   }
   ncs_bam_free(mib_idx.i_inst_ids);

   return SA_AIS_OK;
}
/*****************************************************************************
  PROCEDURE NAME: saAmfParseRedModel

  DESCRIPTION   : In Phase I only the model is captured extend this 
                  function to capture other params by parsing more.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseRedModel(DOMNode *node, char *sgName)
{
   DOMNode           *tmpNode = NULL;
   uns32             red_model=0;

   DOMNodeList * children = node->getChildNodes();

   for(uns32 x =0; x< children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(m_NCS_STRCMP(tmpString, "model_2N") == 0)
         {
            red_model = NCS_BAM_SG_REDMODEL_2N;
         }

         else if(m_NCS_STRCMP(tmpString, "model_NplusM") == 0)
         {
            red_model = NCS_BAM_SG_REDMODEL_NPLUSM;
         }
         
         else if(m_NCS_STRCMP(tmpString, "model_NWay") == 0)
         {
            red_model = NCS_BAM_SG_REDMODEL_NWAY;
         }

         else if(m_NCS_STRCMP(tmpString, "model_NWayActive") == 0)
         {
            red_model = NCS_BAM_SG_REDMODEL_NWAYACTIVE;
         }

         else if(m_NCS_STRCMP(tmpString, "noRedundancy") == 0)
         {
            red_model = NCS_BAM_SG_REDMODEL_NONE;
         }

         XMLString::release(&tmpString);

         if(red_model)
         {
            return saAmfParseMoreRedElems(tmpNode, sgName, red_model);
         }
      }
   }
   return SA_AIS_OK;
}

#if 0
/*****************************************************************************
  PROCEDURE NAME: saAmfParseSUNamesList
  DESCRIPTION   : This routine is to parse the CSI Attributes
                  and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/
static SaAisErrorT
saAmfParseSUNamesList(DOMNode *node, char *sgName)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNodeList          *children = NULL;
   DOMNode              *tmpNode = NULL;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_IDX           mib_idx;
   NCSMIB_FMAT_ID       format;
   
   if(!node)
   {
       m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR,sgName);
      return SA_AIS_ERR_INVALID_PARAM;
   }
   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
             "No SU names found. Parsing DONE", sgName);
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         char *val = XMLString::transcode(tmpNode->getTextContent());

         if(m_NCS_STRCMP(tmpString, "SU") == 0)
         {
            /* set the parent Sg name and thats it!! */
            ncs_bam_build_mib_idx(&mib_idx, val, NCSMIB_FMAT_OCT);

            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, gl_amfConfig_table_size,
                                    "SUInstance", 
                                    "parentSGName", &table_id, &param_id, &format);
            
            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, sgName, format); 
            ncs_bam_free(mib_idx.i_inst_ids);
         }

         XMLString::release(&tmpString);
         XMLString::release(&val);
      }
   }
   return SA_AIS_OK;
}
#endif

/*****************************************************************************
  PROCEDURE NAME: saAmfParseCSIAttributes
  DESCRIPTION   : This routine is to parse the CSI Attributes
                  and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseCSIAttributes(DOMNode *node, char *csiName, char *siName)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *grandNode=NULL, *tmpNode = NULL;
   DOMNodeList          *grandChildren=NULL, *children = NULL;
   char                *tag, *val;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_IDX           mib_idx;
   NCSMIB_FMAT_ID       format;
   char                 nvName[BAM_MAX_INDEX_LEN];
   char                 nvValue[BAM_MAX_INDEX_LEN];
   char                 rowStatus[4];


   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
             "No CSI Attributes found", csiName);
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(!tmpNode)
      {
         return rc;
      }
      
      char *tmpString = XMLString::transcode(tmpNode->getNodeName());

      if(m_NCS_STRCMP(tmpString, "vendorExtensions") == 0)
      {
         XMLString::release(&tmpString);
         continue;
      }
      else if(m_NCS_STRCMP(tmpString, "nameValueList") == 0)
      {
         grandChildren = tmpNode->getChildNodes();
         
         if(grandChildren->getLength() == 0)
         {
            m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
             "No name value pairs found", csiName);
            XMLString::release(&tmpString);
            return rc;
         }
         
         for(unsigned int i=0; i < grandChildren->getLength(); i++)
         {
            grandNode = grandChildren->item(i);
            if(!grandNode)
            {
               XMLString::release(&tmpString);
               return rc;
            }

            XMLString::release(&tmpString);
            tmpString = XMLString::transcode(grandNode->getNodeName());
            
            if(m_NCS_STRCMP(tmpString, "nameValue") == 0)
            {

               DOMNamedNodeMap *attributes= grandNode->getAttributes();
               if(attributes->getLength() == 0)
               {
                  XMLString::release(&tmpString);
                  continue;
               }
               
               for(unsigned int x=0; x < attributes->getLength(); x++)
               {
                  tag = XMLString::transcode(attributes->item(x)->getNodeName());
                  val = XMLString::transcode(attributes->item(x)->getNodeValue());
                  if(m_NCS_STRCMP(tag, "name") == 0)
                  {
                     m_NCS_MEMSET(nvName, 0, BAM_MAX_INDEX_LEN);
                     m_NCS_STRCPY(nvName, val);
                  }
                  else if(m_NCS_STRCMP(tag, "value") == 0)
                  {
                     m_NCS_MEMSET(nvValue, 0, BAM_MAX_INDEX_LEN);
                     m_NCS_STRCPY(nvValue, val);
                  }
                  XMLString::release(&tag);
                  XMLString::release(&val);
               }

               ncs_bam_build_mib_idx(&mib_idx, csiName, NCSMIB_FMAT_OCT);
               ncs_bam_add_sec_mib_idx(&mib_idx, nvName, NCSMIB_FMAT_OCT );

               rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                                    gl_amfConfig_table_size,"CSIInstance", 
                                    "NameValueParamVal", &table_id, &param_id, &format);
               if(rc == SA_AIS_OK)
               {
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, 
                                          &mib_idx, nvValue, format); 
               }
                  

               /* Set the row status to active */
               rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                                    gl_amfConfig_table_size,"CSIInstance", 
                                    "NameValueRowStatus", &table_id, &param_id, &format);
               if(rc == SA_AIS_OK)
               {
                  sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, 
                                          &mib_idx, rowStatus, format); 
               }

               ncs_bam_free(mib_idx.i_inst_ids);
            }
            XMLString::release(&tmpString);
         }
      }
   }
   return SA_AIS_OK;
}

SaAisErrorT
saAmfParseCSIPrototype(char *csiName, char*siName)
{
   DOMNode        *csiPrototype;
   char           csiDName[BAM_MAX_INDEX_LEN];
   NCSMIB_IDX     mib_idx;
   NCSMIB_TBL_ID  table_id;
   uns32          param_id;
   NCSMIB_FMAT_ID format;
   SaAisErrorT    rc = SA_AIS_OK;
   

   csiPrototype = get_dom_node_by_name(csiName, "CSIPrototype");
   if(!csiPrototype)
   {
        /*Commented as fix for IR00084376*/
/*      m_LOG_BAM_MSG_TIC(BAM_NO_PROTO_MATCH, NCSFL_SEV_WARNING, csiName); */
      return SA_AIS_ERR_NOT_EXIST;
   }

   /* Add the prototype name */
   m_NCS_STRCPY(csiDName, csiName);
   m_NCS_STRCAT(csiDName, ",");
   m_NCS_STRCAT(csiDName, siName);

   ncs_bam_build_mib_idx(&mib_idx, csiDName, NCSMIB_FMAT_OCT);

   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
               gl_amfConfig_table_size,"CSIInstance", 
               "csiType", &table_id, &param_id, &format);
   if(rc != SA_AIS_OK)
   {
      m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, "csiType");
      return rc;
   }

   ncs_bam_free(mib_idx.i_inst_ids);

   saAmfParseCSIParamNameList(csiPrototype, csiName);
   return SA_AIS_OK;
}



/*****************************************************************************
  PROCEDURE NAME: saAmfParseCSIInstance
  DESCRIPTION   : This routine is to parse the CSI Instance
                  and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseCSIInstance(DOMNode *node, char *siName)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   char                *tag, *val;
   char                 csiName[BAM_MAX_INDEX_LEN];
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_IDX           mib_idx;
   NCSMIB_FMAT_ID       format;
   char                 rowStatus[4];
   
   m_NCS_MEMSET(csiName, 0, BAM_MAX_INDEX_LEN);
   
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, siName);
      return SA_AIS_ERR_INVALID_PARAM;
   }

   DOMNamedNodeMap *attributesNodes = node->getAttributes();
   if(attributesNodes->getLength() )
   {
      for(unsigned int x=0; x < attributesNodes->getLength(); x++)
      {
         tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
         val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());
         if(m_NCS_STRCMP(tag, "name") == 0)
         {
            m_NCS_MEMSET(csiName, 0, BAM_MAX_INDEX_LEN);
            m_NCS_STRCPY(csiName, val);
            m_NCS_STRCAT(csiName, ",");
            m_NCS_STRCAT(csiName, siName);
         }
         else /*if(m_NCS_STRCMP(tag, "rank") == 0) */
         {

            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                                    gl_amfConfig_table_size,"CSIInstance", 
                                    tag, &table_id, &param_id, &format);
            if(rc != SA_AIS_OK)
            {
               m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, tag);
               XMLString::release(&tag);
               XMLString::release(&val);
               continue;
            }
            /* its tricky here think about this and do it better way*/
            if(csiName[0] != 0)
            {
               ncs_bam_build_mib_idx(&mib_idx, csiName, NCSMIB_FMAT_OCT);
               
               ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, val, format); 
               ncs_bam_free(mib_idx.i_inst_ids);
	       
            }

         }
         XMLString::release(&tag);
         XMLString::release(&val);
      }
   }

   /* Now parse the instance */
   children = node->getChildNodes();

   /* Dont return here even if the length of child nodes is 0
   ** coz we need to set the row Status
   */
   
   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(m_NCS_STRCMP(tmpString, "prototypeName") == 0)
         {
            char *val = XMLString::transcode(tmpNode->getTextContent());
            rc = saAmfParseCSIPrototype(val, siName);

            /* Add the prototype name */
            ncs_bam_build_mib_idx(&mib_idx, csiName, NCSMIB_FMAT_OCT);
   
            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                        gl_amfConfig_table_size,"CSIInstance", 
                        "csiType", &table_id, &param_id, &format);
            if(rc != SA_AIS_OK)
            {
               m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, "csiType");
               return rc;
            }
   
            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, val, format); 

            ncs_bam_free(mib_idx.i_inst_ids);

            XMLString::release(&val);
         }
         else if(m_NCS_STRCMP(tmpString, "CSIAttributes") == 0)
            rc = saAmfParseCSIAttributes(tmpNode, csiName, siName);

         XMLString::release(&tmpString);
      }
   }

   /* now set the row status to active */
   ncs_bam_build_mib_idx(&mib_idx, csiName, NCSMIB_FMAT_OCT);

   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
            gl_amfConfig_table_size,
            "CSIInstance", 
            "rowStatus", &table_id, &param_id, &format);

   sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      rowStatus, format); 

   ncs_bam_free(mib_idx.i_inst_ids);
   return SA_AIS_OK;
}


/*****************************************************************************
  PROCEDURE NAME: saAmfParseCSIList
  DESCRIPTION   : This routine is to parse the list of SU Instances
                  and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseCSIList(DOMNode *node, char *siName)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   
   if(!node)
   {
       m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, siName);
      return SA_AIS_ERR_INVALID_PARAM;
   }
   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
         "No CSI Instances found", siName);
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(m_NCS_STRCMP(tmpString, "CSIInstance") != 0)
         {
            XMLString::release(&tmpString);
            return SA_AIS_ERR_NOT_SUPPORTED;
         }
         rc = saAmfParseCSIInstance(tmpNode, siName);
         if(rc != SA_AIS_OK)
         {
            XMLString::release(&tmpString);
            return rc;
         }
         XMLString::release(&tmpString);
      }
   }
   return SA_AIS_OK;
}        

/*****************************************************************************
  PROCEDURE NAME: ncs_bam_count_CSIs

  DESCRIPTION   : This function counts the number of CSIs listed under
                  the particular SI instance. 
  ARGUMENTS     :
                  
  RETURNS       : 
  NOTES         : 
*****************************************************************************/
static void
ncs_bam_count_CSIs(DOMNode *csiList, char *siName)
{
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
   DOMElement           *tmpElem = NULL;
   char                 count[4];
   DOMNodeList          *elements = NULL;
   SaAisErrorT          rc = SA_AIS_OK;

   /* the passed argument is an ELEMENT NODE */

   XMLCh *str = XMLString::transcode("CSIInstance");
   tmpElem =  static_cast<DOMElement*>(csiList);
   elements = tmpElem->getElementsByTagName(str);

   /* 
   ** Now set the number of CSIs in the SI row with the count
   */
   ncs_bam_build_mib_idx(&mib_idx, siName, NCSMIB_FMAT_OCT);
   
   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
              gl_amfConfig_table_size,
              "SIInstance", 
              "numCSIs", &table_id, &param_id, &format);
   
   sprintf(count, "%d", (int)elements->getLength());
   
   if(rc == SA_AIS_OK)
      ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
         count, format); 

   ncs_bam_free(mib_idx.i_inst_ids);

   return;
}

/*****************************************************************************
  PROCEDURE NAME: saAmfParseSIDepList
  DESCRIPTION   : This routine is to parse the list of dependent SI Instances
                  (for SI-SI dependency def's/conf's)and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/
static SaAisErrorT
saAmfParseSIDepList (DOMNode *node, char *siName)
{
   SaAisErrorT    rc = SA_AIS_OK;
   DOMNode        *tmpNode = NULL;
   DOMNodeList    *children = NULL;
   char           siDepName[BAM_MAX_INDEX_LEN];
   char           tolTime[BAM_MAX_INDEX_LEN];
#if 0
   NCSMIB_TBL_ID  table_id;
   uns32          param_id;
   NCSMIB_FMAT_ID format;
   NCSMIB_IDX     mib_idx;
   char           rowStatus[4];
#endif
   char           *tag, *val;
   
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, "SIDepList");
      return SA_AIS_ERR_INVALID_PARAM;
   }
   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR,
           "No SI Dependencies Instances found");
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(!tmpNode)
      {
        return rc;
      }
     
      char *tmpString = XMLString::transcode(tmpNode->getNodeName());

      if(m_NCS_STRCMP(tmpString, "SIDep") == 0)
      {
         DOMNamedNodeMap *attributes = tmpNode->getAttributes();

         if(attributes->getLength() == 0)
         {
            XMLString::release(&tmpString);
            continue;
         }

         /* Get the sponsor SI name and tolerance timer value */
         for(unsigned int x=0; x < attributes->getLength(); x++)
         {
             tag = XMLString::transcode(attributes->item(x)->getNodeName());
             val = XMLString::transcode(attributes->item(x)->getNodeValue());
             if(m_NCS_STRCMP(tag, "name") == 0)
             {
                m_NCS_MEMSET(siDepName, 0, BAM_MAX_INDEX_LEN);
                m_NCS_STRCPY(siDepName, val);
             }
             else if(m_NCS_STRCMP(tag, "tolTime") == 0)
             {
                m_NCS_MEMSET(tolTime, 0, BAM_MAX_INDEX_LEN);
                m_NCS_STRCPY(tolTime, val);
             }

             XMLString::release(&tag);
             XMLString::release(&val);
         }

         /* Sent MIB sets to AvD only if sponsor SI is specified */  
         if(siDepName[0] == 0)
         {
            m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR,
                              "No sponsor SI available");
            XMLString::release(&tmpString);
            continue;
         }
         else {
            if (avd_bam_build_si_dep_list(siName, siDepName, tolTime) != NCSCC_RC_SUCCESS)
            {
               return SA_AIS_ERR_INVALID_PARAM;
            }
#if 0
            /* Update the SI dependency data */ 
            ncs_bam_build_mib_idx(&mib_idx, siName, NCSMIB_FMAT_OCT);
            ncs_bam_add_sec_mib_idx(&mib_idx, siDepName, NCSMIB_FMAT_OCT );
   
            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table,
                                           gl_amfConfig_table_size,
                                           "SIInstance",
                                           "SIdepToltime", &table_id, &param_id, &format);
   
            if (rc == SA_AIS_OK)
               ncs_bam_generate_counter64_mibset(table_id, param_id, 
                                                 &mib_idx, tolTime); 
     
            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table,
                                           gl_amfConfig_table_size,
                                           "SIInstance",
                                           "SIdepRowStatus", &table_id, &param_id, &format);
   
            if(rc == SA_AIS_OK)
            {
               sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
               ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx,
                                               rowStatus, format);
            }

            ncs_bam_free(mib_idx.i_inst_ids);
#endif
         }
      }

      XMLString::release(&tmpString);
      if(rc != SA_AIS_OK)
        return rc;
   }

   return SA_AIS_OK;
}


/*****************************************************************************
  PROCEDURE NAME: saAmfParseSIInstance

  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : set the row status, parent SG etc after the for loop of 
                  child nodes to allow the maxActive Assignments and other
                  mibs to be set so that the row status can be set active
                  Later parse the CSI list.
*****************************************************************************/

static SaAisErrorT
saAmfParseSIInstance(char *siName, char *parentSg, char *rank)
{
   SaAisErrorT       rc = SA_AIS_OK;
   DOMNode        *siNode, *tmpNode = NULL;
   NCSMIB_TBL_ID  table_id;   /* table id */
   uns32             param_id; 
   NCSMIB_FMAT_ID    format;
   NCSMIB_IDX        mib_idx;
   char              rowStatus[4];

   siNode = get_dom_node_by_name(siName, "SIInstance");

   /* Check to make sure that this is an ELEMENT NODE */
   if((!siNode) || (siNode->getNodeType() != DOMNode::ELEMENT_NODE))
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
          "Expected SI Instance Element", siName);
      return SA_AIS_ERR_INVALID_PARAM;
   }
   
   DOMNodeList * children = siNode->getChildNodes();

   if(children->getLength() == 0)
   {
      return rc;
   }

   ncs_bam_build_mib_idx(&mib_idx, siName, NCSMIB_FMAT_OCT);

   for(unsigned x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         
         if(m_NCS_STRCMP(tmpString, "CSIList") == 0)
         {
            ncs_bam_count_CSIs(tmpNode, siName);
            XMLString::release(&tmpString);
            continue; 
         }

         else if(m_NCS_STRCMP(tmpString, "SURankList") == 0)
         {
            saAmfParseSISURankList(tmpNode, siName);
            XMLString::release(&tmpString);
            continue; 
         }


         else if(m_NCS_STRCMP(tmpString, "preferredNumOfAssignments") == 0)
         {
            char *val = XMLString::transcode(tmpNode->getTextContent());
            
            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
               gl_amfConfig_table_size,"SIInstance", 
               tmpString, &table_id, &param_id, &format);
            if(rc != SA_AIS_OK)
            {
               m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING,
                                   tmpString);
               XMLString::release(&tmpString);
               XMLString::release(&val);
               /*return rc;*/
               continue; 
            }

            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, val, format); 
         }
         XMLString::release(&tmpString);
      }
   }


   /* Set the parent SG name for this SI */
   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
            gl_amfConfig_table_size,
            "SIInstance", 
            "parentSGName", &table_id, &param_id, &format);

   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      parentSg, format); 

   /* Set the RANK for this SI */
   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
            gl_amfConfig_table_size,
            "SIInstance", 
            "rank", &table_id, &param_id, &format);

   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      rank, format); 
#if 0
   /* set the SI_SU type to default */
   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
            gl_amfConfig_table_size,
            "SIInstance", 
            "siSuType", &table_id, &param_id, &format);
   /* reuse variable rowStatus */
   sprintf(rowStatus, "%d", NCS_SI_DEFAULT_SU_TYPE);
   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      rowStatus, format); 
#endif

   /* now set the row status to active */
   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
            gl_amfConfig_table_size,
            "SIInstance", 
            "rowStatus", &table_id, &param_id, &format);

   sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      rowStatus, format); 

   ncs_bam_free(mib_idx.i_inst_ids);

   m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
                      "SI row added succesfully", siName);
   for( uns32 i=0; i < children->getLength(); i++)
   {
      tmpNode = children->item(i);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         
         if(m_NCS_STRCMP(tmpString, "CSIList") == 0)
            rc = saAmfParseCSIList(tmpNode, siName);
         else if(m_NCS_STRCMP(tmpString, "SIDepList") == 0)
            rc = saAmfParseSIDepList(tmpNode, siName);

         XMLString::release(&tmpString);
      }
   }
   return SA_AIS_OK;
}


/*****************************************************************************
  PROCEDURE NAME: saAmfParseSIList
  DESCRIPTION   : This routine is to parse the list of SI Instances
                  and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseSIList(DOMNode *node, char *parentSg)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   char                 rank[4];
   
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, "SIList");
      return SA_AIS_ERR_INVALID_PARAM;
   }
   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR,
           "No SI Instances found");
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         char *siName = XMLString::transcode(tmpNode->getTextContent());

         if(m_NCS_STRCMP(tmpString, "SI") != 0)
         {
            XMLString::release(&tmpString);
            XMLString::release(&siName);
            return SA_AIS_ERR_NOT_SUPPORTED;
         }

         DOMNamedNodeMap *attributesNodes = tmpNode->getAttributes();
         if(attributesNodes->getLength() )
         {
            for(unsigned int x=0; x < attributesNodes->getLength(); x++)
            {
               char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
               char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());
               if(m_NCS_STRCMP(tag, "rank") == 0)
               {
                  m_NCS_STRCPY(rank, val);
               }
               XMLString::release(&tag);
               XMLString::release(&val);
            }
         } /* end of parsing attributes */

         rc = saAmfParseSIInstance(siName, parentSg, rank );
         XMLString::release(&tmpString);
         XMLString::release(&siName);
         if(rc != SA_AIS_OK)
            return rc;
      }
   }
   return SA_AIS_OK;
}        

        
/*****************************************************************************
  PROCEDURE NAME: saAmfParseSGInstance

  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

SaAisErrorT
saAmfParseSGInstance(DOMNode *node)
{
   SaAisErrorT       rc = SA_AIS_OK;
   DOMNode        *tmpNode = NULL;
   NCSMIB_TBL_ID  table_id;   /* table id */
   uns32             param_id; 
   NCSMIB_FMAT_ID    format;
   NCSMIB_IDX        mib_idx;
   char              index[BAM_MAX_INDEX_LEN];
   char              rowStatus[4];

   /* Check to make sure that this is an ELEMENT NODE */
   if(node->getNodeType() != DOMNode::ELEMENT_NODE)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR,
           "Expected SG Instance Element");
      return SA_AIS_ERR_INVALID_PARAM;
   }

   DOMNamedNodeMap *attributesNodes = node->getAttributes();
   if(attributesNodes->getLength() )
   {
      for(unsigned int x=0; x < attributesNodes->getLength(); x++)
      {
         char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
         char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());

         if(m_NCS_STRCMP(tag, "name") == 0)
         {
            /* this is name which is the index into the SG table */
            m_NCS_STRCPY(index, val);

            XMLString::release(&tag);
            XMLString::release(&val);
            break; 
         }

         XMLString::release(&tag);
         XMLString::release(&val);
      }
   }

   DOMNodeList * children = node->getChildNodes();

   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
      "No AMF objects found");
      return rc;
   }

   /* loop for elements of SG scope and loop yet again after setting the 
   ** row status of the SG to active coz the rest of the elements
   ** expect the SG to be created on which they reside.
   */
   for(unsigned x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         
         if(m_NCS_STRCMP(tmpString, "redundancyModel") == 0)
            rc = saAmfParseRedModel(tmpNode, index);

         else if(m_NCS_STRCMP(tmpString, "SGErrorEscalation") == 0)
            rc = saAmfParseSGErrorEsc(tmpNode, index);

         else if(m_NCS_STRCMP(tmpString, "vendorExtensions") == 0)
            rc = saAmfParseSGVendorExt(tmpNode, index);
         else if(m_NCS_STRCMP(tmpString, "adminstate") == 0) 
         {
     /* This is String a code change is require to convert the value into integer to be sent as Mib data */
            /* This is a boolean */
            char *val = XMLString::transcode(tmpNode->getTextContent());
   
            ncs_bam_build_mib_idx(&mib_idx, index, NCSMIB_FMAT_OCT);
            
            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, gl_amfConfig_table_size,
                                    "SGInstance", 
                                    tmpString, &table_id, &param_id, &format);
            if(rc != SA_AIS_OK)
            {
               m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, 
                                       tmpString);
               XMLString::release(&tmpString);
               XMLString::release(&val);
               ncs_bam_free(mib_idx.i_inst_ids);
               continue;
            }

            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                               val, format);

            XMLString::release(&val);
            ncs_bam_free(mib_idx.i_inst_ids);
         }
         if(rc != SA_AIS_OK)
            m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
            "error encountered parsing ", tmpString);

         XMLString::release(&tmpString);
      }
   }

#if 0
   /* set the su_type to default for PHASE- I OVERLOAD rowStatus var*/
   ncs_bam_build_mib_idx(&mib_idx, index, NCSMIB_FMAT_OCT);

   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
            gl_amfConfig_table_size,
            "SGInstance", 
            "suType", &table_id, &param_id, &format);

   sprintf(rowStatus, "%d", NCS_SG_DEFAULT_SU_TYPE);
   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      rowStatus, format); 
#endif

   ncs_bam_build_mib_idx(&mib_idx, index, NCSMIB_FMAT_OCT);

   /* now set the row status to active */
   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
            gl_amfConfig_table_size,
            "SGInstance", 
            "rowStatus", &table_id, &param_id, &format);


   sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      rowStatus, format); 

   ncs_bam_free(mib_idx.i_inst_ids);

   m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
                      "SG row created successfully",index); 

   /* Now that SG row status is active call out to parse the SIs */

   for( uns32 i=0; i < children->getLength(); i++)
   {
      tmpNode = children->item(i);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         
         if(m_NCS_STRCMP(tmpString, "SIList") == 0)
            rc = saAmfParseSIList(tmpNode, index);

         XMLString::release(&tmpString);
      }
   }
   return rc;
}

/*****************************************************************************
  PROCEDURE NAME: saAmfParseSISURankList
  DESCRIPTION   : This routine is to parse the list of SUs per SI 
                  and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseSISURankList(DOMNode *node, char *siName)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   char                 suName[BAM_MAX_INDEX_LEN];
   char                 rank[4];
   NCSMIB_TBL_ID        table_id;   /* table id */
   uns32                param_id; 
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
 
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, "SUsPerSIList");
      return SA_AIS_ERR_INVALID_PARAM;
   }
   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR,
           "No SU List found");
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(m_NCS_STRCMP(tmpString, "SU") != 0)
         {
            XMLString::release(&tmpString);
            return SA_AIS_ERR_NOT_SUPPORTED;
         }

         DOMNamedNodeMap *attributesNodes = tmpNode->getAttributes();
         if(attributesNodes->getLength() )
         {
            for(unsigned int x=0; x < attributesNodes->getLength(); x++)
            {
               char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
               char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());
               if(m_NCS_STRCMP(tag, "SUName") == 0)
               {
                  m_NCS_MEMSET(suName, 0, BAM_MAX_INDEX_LEN);
                  m_NCS_STRCPY(suName, val);
               }
               else if(m_NCS_STRCMP(tag, "rank") == 0)
               {
                  m_NCS_MEMSET(rank, 0, 4);
                  m_NCS_STRCPY(rank, val);
               }
               XMLString::release(&tag);
               XMLString::release(&val);
            }
         } /* end of parsing attributes */

         /* Now send the mib sets with the SIName and Rank as index */
         ncs_bam_build_mib_idx(&mib_idx, siName, NCSMIB_FMAT_OCT);
         ncs_bam_add_sec_mib_idx(&mib_idx, rank, NCSMIB_FMAT_INT );

         rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                              gl_amfConfig_table_size,"SIInstance", 
                              "SUName", &table_id, &param_id, &format);
         if(rc == SA_AIS_OK)
         {
            ncs_bam_build_and_generate_mibsets(table_id, param_id, 
                                    &mib_idx, suName, format); 
         }
            

         /* Set the row status to active */
         rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                              gl_amfConfig_table_size,"SIInstance", 
                              "SUsPerSIrowStatus", &table_id, &param_id, &format);
         if(rc == SA_AIS_OK)
         {
            sprintf(rank, "%d", NCSMIB_ROWSTATUS_ACTIVE);
            ncs_bam_build_and_generate_mibsets(table_id, param_id, 
                                    &mib_idx, rank, format); 
         }

         ncs_bam_free(mib_idx.i_inst_ids);

         /* End of sending mibsets... move on to next element in the list */
         XMLString::release(&tmpString);
         if(rc != SA_AIS_OK)
            return rc;
      }
   }
   return SA_AIS_OK;
}

/*****************************************************************************
  PROCEDURE NAME: saAmfParseCSIParamNameList
  DESCRIPTION   : This routine is to parse the list of CSI params
                  and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseCSIParamNameList(DOMNode *node, char *csiName)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   char                 rowStatus[4];
   NCSMIB_TBL_ID        table_id;   /* table id */
   uns32                param_id; 
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
   char                 param[BAM_MAX_INDEX_LEN];
    
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, "CSIParamList");
      return SA_AIS_ERR_INVALID_PARAM;
   }
   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR,
           "No CSI Params found");
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(m_NCS_STRCMP(tmpString, "ParamName") != 0)
         {
            XMLString::release(&tmpString);
            continue;
         }

         DOMNamedNodeMap *attributes= tmpNode->getAttributes();
         if(attributes->getLength() == 0)
         {
            XMLString::release(&tmpString);
            continue;
         }
         
         for(unsigned int x=0; x < attributes->getLength(); x++)
         {
            char *tag = XMLString::transcode(attributes->item(x)->getNodeName());
            char *val = XMLString::transcode(attributes->item(x)->getNodeValue());
            if(m_NCS_STRCMP(tag, "name") == 0)
            {
               m_NCS_MEMSET(param, 0, BAM_MAX_INDEX_LEN);
               m_NCS_STRCPY(param, val);
            }
            XMLString::release(&tag);
            XMLString::release(&val);
         }

         /* Now send the rowstatus with the CSIName and ParamName as index */
         ncs_bam_build_mib_idx(&mib_idx, csiName, NCSMIB_FMAT_OCT);
         ncs_bam_add_sec_mib_idx(&mib_idx, param, NCSMIB_FMAT_OCT );

         rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                              gl_amfConfig_table_size,"CSIInstance", 
                              "CSTypeParamRowStatus", &table_id, &param_id, &format);
         if(rc == SA_AIS_OK)
         {
            sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
            ncs_bam_build_and_generate_mibsets(table_id, param_id, 
                                    &mib_idx, rowStatus, format); 
         }

         ncs_bam_free(mib_idx.i_inst_ids);

         XMLString::release(&tmpString);
         if(rc != SA_AIS_OK)
            return rc;
      }
   }
   return SA_AIS_OK;
}        
