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

/* NCS specific include files */
#include <ncs_opt.h>
#include <gl_defs.h>
#include <t_suite.h>
#include <ncs_mib_pub.h>
#include "bamHWEntities.h"
#include "bam_log.h"
#include "bamParser.h"
#include "bamError.h"

DOMDocument *gl_hw_dom_doc;
EXTERN_C uns32 gl_ncs_bam_hdl;

#define LOCATION_RANGE_MIN_LENGTH 4

/*****************************************************************************
  PROCEDURE NAME: bam_fill_location_from_string
  DESCRIPTION   : 
  ARGUMENTS     : valid_location  -- pointer to NCS_HW_ENT_VALID_LOCATION
                  val -- This is location range string which need to be 
                         parsed. It can be in any of the form below.
                         1
                         1..3
                         1..3,
                         1..3, 4..6

  RETURNS       : NONE

  NOTES         :  use the re-entrant supported m_NCS_OS_STRTOK_R macro

*****************************************************************************/

static void
bam_fill_location_from_string(NCS_HW_ENT_VALID_LOCATION *valid_location, char *val)
{
   char str[8][MAX_POSSIBLE_LOC_RANGES] = {};
   char *tmp, *reentrant, junk, *token;
   int idx=0, x;

   token = m_NCS_OS_STRTOK_R(val, ",", &reentrant);
   
   /* This will take care of single location specified as opposed to 
   ** range
   */
   if( (!token) || (m_NCS_STRLEN(token) < LOCATION_RANGE_MIN_LENGTH))
   {
      valid_location->max[0] = atoi(val);
      valid_location->min[0] = atoi(val);
      return;
   }

   m_NCS_STRCPY(str[idx], token);

   while( (idx < 8)) /* && (token) &&(strlen(token) != 0) ) */
   {
      idx++;
      token = m_NCS_OS_STRTOK_R(NULL, ",", &reentrant);
      if((token) && (m_NCS_STRLEN(token)))
         m_NCS_STRCPY(str[idx], token);
      else
      {
         idx--;
         break;
      }
   }

   for(x=0; x<= idx; x++)
   {
      tmp = m_NCS_OS_STRSTR(str[x], "..");
      valid_location->max[x] = atoi(tmp+2);
      
      tmp = m_NCS_OS_STRTOK_R(str[x], "..", &junk);
      valid_location->min[x] = atoi(tmp);
   }
   return;
}

/*****************************************************************************
  PROCEDURE NAME: saHwParseFRUInformation
  DESCRIPTION   : 
  ARGUMENTS     : Dom node containing FRU Information.
                  
  RETURNS       : 0 for SUCCESS else non-zero for FAILURE

  NOTES         : 

*****************************************************************************/

static SaAisErrorT
saHwParseFRUInformation(DOMNode *node, NCS_HW_ENT_TYPE_DESC *entity_desc)
{
   SaAisErrorT     rc = SA_AIS_OK;
   
   if( (!node) || (!entity_desc) )
      return rc;
   
   DOMNamedNodeMap *attributesNodes = node->getAttributes();

   if(attributesNodes->getLength() )
   {
      for(unsigned int x=0; x < attributesNodes->getLength(); x++)
      {
         char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
         char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());

         if(m_NCS_STRCMP(tag, "ProductName") == 0 )
         {
            m_NCS_STRCPY( entity_desc->fru_product_name, val);
         }
         else if(m_NCS_STRCMP(tag, "ProductVersion") == 0 )
         {
            m_NCS_STRCPY( entity_desc->fru_product_version, val);
         }
         XMLString::release(&tag);
         XMLString::release(&val);
      }
   }
   return rc;
}

/*****************************************************************************
  PROCEDURE NAME: saHwParseFRUPolicy
  DESCRIPTION   : 
  ARGUMENTS     : Dom node containing FRU Information.
                  
  RETURNS       : 0 for SUCCESS else non-zero for FAILURE

  NOTES         : 

*****************************************************************************/

static SaAisErrorT
saHwParseFRUPolicy(DOMNode *node, NCS_HW_ENT_TYPE_DESC *entity_desc)
{
   SaAisErrorT     rc = SA_AIS_OK;
   DOMNode *tmpNode = NULL;

   /* Check to make sure that this is an ELEMENT NODE */
   if(node->getNodeType() != DOMNode::ELEMENT_NODE)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
          "Error in processing the DOM Tree expected FRU Info.");
      return SA_AIS_ERR_INVALID_PARAM;
   }

   DOMNamedNodeMap *attributesNodes = node->getAttributes();
   if(attributesNodes->getLength() == 1 )
   {
      char *tag = XMLString::transcode(attributesNodes->item(0)->getNodeName());
      char *val = XMLString::transcode(attributesNodes->item(0)->getNodeValue());

      if(m_NCS_STRCMP(tag, "Applicable") == 0)
      {
         entity_desc->is_fru = atoi(val);
      }
      
      if(entity_desc->is_fru != TRUE)
      {
         XMLString::release(&tag);
         XMLString::release(&val);
         return rc;
      }

      DOMNodeList * children = node->getChildNodes();

      if(children->getLength() == 0)
      {
         m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                           "No FRU Info found. ");
         m_MMGR_FREE_BAM_DEFAULT_VAL(entity_desc);
         return rc;
      }

      for(unsigned x=0; x < children->getLength(); x++)
      {
         tmpNode = children->item(x);
         if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
         {
            char *tmpString = XMLString::transcode(tmpNode->getNodeName());
            
            if(m_NCS_STRCMP(tmpString, "FruInformation") == 0)
            {
               rc = saHwParseFRUInformation(tmpNode, entity_desc);
            }

            XMLString::release(&tmpString);
         }
      }
      XMLString::release(&tag);
      XMLString::release(&val);
   }

   return rc;
}

/*****************************************************************************
  PROCEDURE NAME: saHwParseLocationRange
  DESCRIPTION   : 
  ARGUMENTS     : the parsed and built location range for the entity.
                  This includes the parent entity name and the valid
                  location. There can be multiple instances of this
                  and hence we try to add to the existing array.
                  
  RETURNS       : 0 for SUCCESS else non-zero for FAILURE

  NOTES         : This Hardware DOM document in walked one node at a time and
                  the validation information is extracted and added to the 
                  ENT_TYPE_DESC structure and added to the BAM-CB 

                  This might change in future release when AvM has MIB defined
                  and stores the validation Information.
*****************************************************************************/

static SaAisErrorT
saHwParseLocationRange(DOMNode *node, NCS_HW_ENT_TYPE_DESC *entity_desc)
{
   SaAisErrorT     rc = SA_AIS_OK;
   uns8            curr_num_parents = 0;
   
   if( (!node) || (!entity_desc) )
      return rc;
   
   curr_num_parents = entity_desc->num_possible_parents;

   if(curr_num_parents >= MAX_POSSIBLE_PARENTS -1)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
          "Max number of parent entity exceeded in EntityType Instance.");
      return rc;
   }

   DOMNamedNodeMap *attributesNodes = node->getAttributes();

   if(attributesNodes->getLength() )
   {
      for(unsigned int x=0; x < attributesNodes->getLength(); x++)
      {
         char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
         char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());

         if(m_NCS_STRCMP(tag, "ParentEntityTypeInstanceName") == 0)
         {
            m_NCS_STRCPY(
                       entity_desc->location_range[curr_num_parents].parent_ent,
                       val);
         }
         else if(m_NCS_STRCMP(tag, "LocationRegularExpression") == 0 )
         {
            bam_fill_location_from_string(
               &(entity_desc->location_range[curr_num_parents].valid_location), 
                 val);
         }
         XMLString::release(&tag);
         XMLString::release(&val);
      }
   }
   entity_desc->num_possible_parents++;
   return rc;
}


/*****************************************************************************
  PROCEDURE NAME: parse_ent_type_instance
  DESCRIPTION   : 
  ARGUMENTS     : the parsed and built DOMDocument.
                  
  RETURNS       : 0 for SUCCESS else non-zero for FAILURE

  NOTES         : This Hardware DOM document in walked one node at a time and
                  the validation information is extracted and added to the 
                  ENT_TYPE_DESC structure and added to the BAM-CB 

                  This might change in future release when AvM has MIB defined
                  and stores the validation Information.
*****************************************************************************/

static SaAisErrorT
parse_ent_type_instance(DOMNode *node)
{
   SaAisErrorT     rc = SA_AIS_OK;
   DOMNode *tmpNode = NULL;
   NCS_HW_ENT_TYPE_DESC    *entity_desc = NULL;
   NCS_BAM_CB   *bam_cb = NULL;


   /* Check to make sure that this is an ELEMENT NODE */
   if(node->getNodeType() != DOMNode::ELEMENT_NODE)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
          "Error in processing the DOM Tree expected EntityType Instance.");
      return SA_AIS_ERR_INVALID_PARAM;
   }

   /* Allocate memory for the entity type instance */
   entity_desc = (NCS_HW_ENT_TYPE_DESC *) 
                  m_MMGR_ALLOC_BAM_DEFAULT_VAL(sizeof(NCS_HW_ENT_TYPE_DESC));

   if(!entity_desc)
   {
      m_LOG_BAM_MEMFAIL(BAM_ALLOC_FAILED);
      return SA_AIS_ERR_NO_MEMORY;
   }

   m_NCS_MEMSET(entity_desc, 0, sizeof(NCS_HW_ENT_TYPE_DESC));

   DOMNamedNodeMap *attributesNodes = node->getAttributes();
   if(attributesNodes->getLength() )
   {
      for(unsigned int x=0; x < attributesNodes->getLength(); x++)
      {
         char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
         char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());

         if(m_NCS_STRCMP(tag, "Name") == 0)
         {
            m_NCS_STRCPY(entity_desc->entity_name, val);
         }
         else if(m_NCS_STRCMP(tag, "HPIEntityType") == 0)
         {
            entity_desc->entity_type = get_entity_type_from_text(val);
         }
         else if(m_NCS_STRCMP(tag, "IsNode") == 0)
         {
            entity_desc->isNode = atoi(val);
         }
         XMLString::release(&tag);
         XMLString::release(&val);
      }
   } /* End of parsing Attributes */

   DOMNodeList * children = node->getChildNodes();

   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No Validation Info found. ");
      m_MMGR_FREE_BAM_DEFAULT_VAL(entity_desc);
      return rc;
   }

   for(unsigned x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         
         if(m_NCS_STRCMP(tmpString, "LocationRange") == 0)
         {
            rc = saHwParseLocationRange(tmpNode, entity_desc);
         }
         else if(m_NCS_STRCMP(tmpString, "FRUActivationPolicy") == 0)
         {
            rc = saHwParseFRUPolicy(tmpNode, entity_desc);
         }
         
         XMLString::release(&tmpString);
      }
   } /* End of parsing child elements */
   
   /* If the control reach here the entity_desc structure is
   ** populated. Add to the List
   */
   if((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      m_MMGR_FREE_BAM_DEFAULT_VAL(entity_desc);
      return SA_AIS_ERR_NOT_EXIST;
   }

   entity_desc->next = bam_cb->hw_entity_list;
   bam_cb->hw_entity_list = entity_desc;

   m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                      "Parsed ENTITY TYPE INSTANCE Successfully. ");

   ncshm_give_hdl(gl_ncs_bam_hdl);   
   return SA_AIS_OK;
}

/*****************************************************************************
  PROCEDURE NAME: parse_hardware_bom
  DESCRIPTION   : 
  ARGUMENTS     : the parsed and built DOMDocument.
                  
  RETURNS       : 0 for SUCCESS else non-zero for FAILURE

  NOTES         : This Hardware DOM document in walked one node at a time and
                  the validation information is extracted and added to the 
                  ENT_TYPE_DESC structure and added to the BAM-CB 

                  This might change in future release when AvM has MIB defined
                  and stores the validation Information.
*****************************************************************************/

static uns32
parse_hardware_bom(DOMDocument *doc)
{
   XMLCh *str = XMLString::transcode("ValidationConfig");
   DOMNodeList *elements = doc->getElementsByTagName( str);
   DOMNodeList *children;

   XMLString::release(&str);
   if(elements->getLength() != 1)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR,
                       "There should be exactly only one Validation element");
      return 1;
   }
   DOMNode *ValConfig = elements->item(0);
   children = ValConfig->getChildNodes();
   
   for(unsigned int i=0;i < children->getLength();i++)
   {
      DOMNode *tmpNode = children->item(i);
      if(tmpNode)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(m_NCS_STRCMP(tmpString, "EntityTypeInstance") == 0)
            parse_ent_type_instance(tmpNode);

         XMLString::release(&tmpString);
      }
   }
   return 0;
}

/*****************************************************************************
  PROCEDURE NAME: parse_and_build_hw_validation
  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : We use DOMBuilder as opposed to XercesDOMParser to get 
                  the handle of the built domDocument.
*****************************************************************************/

uns32
parse_and_build_hw_validation(char *xmlFile)
{
    /*const char*                xmlFile = 0;*/
    AbstractDOMParser::ValSchemes valScheme = AbstractDOMParser::Val_Auto;
    bool                       doNamespaces       = true;
    bool                       doSchema           = true;
    bool                       schemaFullChecking = true;
    bool                       errorOccurred = false;

    /*   Turning this on recognizeNEL will lead to non-standard compliance 
    **   behaviour it will recognize the unicode character 0x85 as new line 
    **   character instead of regular character as specified in XML 1.0
    **   do not turn this on unless really necessary
    */
    bool                       recognizeNEL = false;
    
    /* The locale is set iff the Initialize() is invoked for the very first time,
     * to ensure that each and every message loaders, in the process space, share
     * the same locale. We dont use it in the phase-I
     */

    /* Initialize the XML4C system */
    try
    {
       XMLPlatformUtils::Initialize();

       if (recognizeNEL)
       {
          XMLPlatformUtils::recognizeNEL(recognizeNEL);
       }
    }

    catch (const XMLException& toCatch)
    {
       char *str = XMLString::transcode(toCatch.getMessage());
       m_NCS_SYSLOG(NCS_LOG_ERR, "NCS_AvSv: Error!: %s  during XML Parser initialization for! : %s file ", str, xmlFile);
       XMLString::release(&str);
       return 1;
    }

    // Instantiate the DOM parser.
    static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
    DOMBuilder        *parser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

    parser->setFeature(XMLUni::fgDOMNamespaces, doNamespaces);
    parser->setFeature(XMLUni::fgXercesSchema, doSchema);
    parser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);
    /*parser->setFeature(AbstractDOMParser::ignorableWhitespace, true); */

    if (valScheme == AbstractDOMParser::Val_Auto)
    {
        parser->setFeature(XMLUni::fgDOMValidateIfSchema, true);
    }
    else if (valScheme == AbstractDOMParser::Val_Never)
    {
        parser->setFeature(XMLUni::fgDOMValidation, false);
    }
    else if (valScheme == AbstractDOMParser::Val_Always)
    {
        parser->setFeature(XMLUni::fgDOMValidation, true);
    }

    // enable datatype normalization - default is off
    parser->setFeature(XMLUni::fgDOMDatatypeNormalization, true);

    // And create our error handler and install it
    DOMBamErrorHandler errorHandler;
    parser->setErrorHandler(&errorHandler);

    /* reset error count first */
    errorHandler.resetErrors();

    XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *hw_doc = 0;

    try
    {
       /* reset document pool */
       parser->resetDocumentPool();

       hw_doc = parser->parseURI(xmlFile);
    }

    catch (const XMLException& toCatch)
    {
       m_NCS_SYSLOG(NCS_LOG_ERR, "NCS_AvSv: Error during parsing! : %s", xmlFile);
       char *str = XMLString::transcode(toCatch.getMessage()); 
       m_NCS_SYSLOG(NCS_LOG_ERR, "NCS_AvSv: Exception message is! : %s", str);
       XMLString::release(&str);
       errorOccurred = true;
    }
    catch (const DOMException& toCatch)
    {
       const unsigned int maxChars = 2047;
       XMLCh errText[maxChars + 1];

       m_NCS_SYSLOG(NCS_LOG_ERR, "NCS_AvSv: Error during parsing! : %s", xmlFile);
       m_NCS_SYSLOG(NCS_LOG_ERR, "NCS_AvSv: Exception code is!  : %s", toCatch.code);

       if (DOMImplementation::loadDOMExceptionMsg(toCatch.code, errText, maxChars))
       {
          char *str = XMLString::transcode(errText);
          m_NCS_SYSLOG(NCS_LOG_ERR, "NCS_AvSv: Message is! : %s", str);
          XMLString::release(&str);
          errorOccurred = true;
       }
    }
    catch (...)
    {
       m_NCS_SYSLOG(NCS_LOG_ERR, "NCS_AvSv: Unexpected exception during parsing! : %s", xmlFile);
       errorOccurred = true;
    }

    /*
    **  Extract the DOM tree, get the list of all the elements and report the
    **  length as the count of elements.
    */
    if (errorHandler.getSawErrors())
    {
       m_NCS_SYSLOG(NCS_LOG_ERR, "NCS_AvSv: Errors occurred in DOM tree during parsing xml file: %s, no output available", xmlFile);
       errorOccurred = true;
    }
    else
    {
       if (hw_doc)
       {
          gl_hw_dom_doc = hw_doc;
          if(parse_hardware_bom(hw_doc))
             return 1;
          // test getElementsByTagName and getLength
       }
    }
    

    /*
    **  Delete the parser itself.  Must be done prior to calling Terminate, below.
    */
    parser->release();

    /* And call the termination method */
    XMLPlatformUtils::Terminate();

    if (errorOccurred)
        return 4;
    else
        return 0;
}
