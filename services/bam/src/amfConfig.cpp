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


#include <stdlib.h>

/* NCS specific include files */
#include <ncs_opt.h>
#include <gl_defs.h>
#include <t_suite.h>
#include <ncs_mib_pub.h>
#include <ncsgl_defs.h>
#include <ncsmiblib.h>
#include <ncs_lib.h>
#include <ncsdlib.h>

#include <saAis.h>
#include "bam.h"
#include "bamParser.h"
#include "bam_log.h"

/* Global constants and #defines go here */
EXTERN_C uns32 gl_comp_type;

XERCES_CPP_NAMESPACE_USE

/* Default MAX value for heartbeat */
#define NCS_DEFAULT_HB_MAX "9999999999999999"

/* Some forward declarations for compiler */
static SaAisErrorT
saAmfParseCompClcCommands(DOMNode *, char *);

static SaAisErrorT
saAmfParseSUAttributes(DOMNode *, char *);

static SaAisErrorT
saAmfParseNodeAttributes(DOMNode *, char *);

static SaAisErrorT
saAmfParseAmfComponentT(DOMNode *, char *, uns32);

static SaAisErrorT saAmfParseExtSUInstance(DOMNode *node);

static SaAisErrorT
saAmfParseSaAwareComp(DOMNode *tmp, char *index)
{
   saAmfParseAmfComponentT(tmp, index, NCS_BAM_COMP_SA_AWARE);
   return SA_AIS_OK;
}

static uns32 get_capability_value(char *val)
{
   if(strcmp(val, "X_ACTIVE_AND_Y_STANDBY") == 0)
   {
      return NCS_BAM_COMP_CAP_X_ACTIVE_AND_Y_STANDBY;
   }
   else if(strcmp(val, "X_ACTIVE_OR_Y_STANDBY") == 0)
   {
      return NCS_BAM_COMP_CAP_X_ACTIVE_OR_Y_STANDBY;
   }
   else if(strcmp(val, "1_ACTIVE_OR_Y_STANDBY") == 0)
   {
      return NCS_BAM_COMP_CAP_1_ACTIVE_OR_Y_STANDBY;
   }
   else if(strcmp(val, "1_ACTIVE_OR_1_STANDBY") == 0)
   {
      return NCS_BAM_COMP_CAP_1_ACTIVE_OR_1_STANDBY;
   }
   else if(strcmp(val, "X_ACTIVE") == 0)
   {
      return NCS_BAM_COMP_CAP_X_ACTIVE;
   }
   else if(strcmp(val, "1_ACTIVE") == 0)
   {
      return NCS_BAM_COMP_CAP_1_ACTIVE;
   }
   else if(strcmp(val, "NO_STATE") == 0)
   {
      return NCS_BAM_COMP_CAP_NO_STATE;
   }

   /* send default */
   return NCS_BAM_COMP_CAP_1_ACTIVE_OR_1_STANDBY;
}

static uns32 get_recovery_value(char *val)
{
   if(strcmp(val, "SA_AMF_NO_RECOMMENDATION") == 0)
   {
      return SA_AMF_NO_RECOMMENDATION;
   }
   else if(strcmp(val, "SA_AMF_COMPONENT_RESTART") == 0)
   {
      return SA_AMF_COMPONENT_RESTART;
   }
   else if(strcmp(val, "SA_AMF_COMPONENT_FAILOVER") == 0)
   {
      return SA_AMF_COMPONENT_FAILOVER;
   }
   else if(strcmp(val, "SA_AMF_NODE_SWITCHOVER") == 0)
   {
      return SA_AMF_NODE_SWITCHOVER;
   }
   else if(strcmp(val, "SA_AMF_NODE_FAILOVER") == 0)
   {
      return SA_AMF_NODE_FAILOVER;
   }
   else if(strcmp(val, "SA_AMF_NODE_FAILFAST") == 0)
   {
      return SA_AMF_NODE_FAILFAST;
   }
   else if(strcmp(val, "SA_AMF_CLUSTER_RESET") == 0)
   {
      return SA_AMF_CLUSTER_RESET;
   }
   /* send def */
   return SA_AMF_NO_RECOMMENDATION;
}

static uns32 get_admstate_val(char *val)
{
/* Admin state at the intial time can have only two values LOCK and UNLOCK  */ 
   if(strcmp(val, "locked") == 0)
         return SA_AMF_LOCK;
   else
        return SA_AMF_UNLOCK;
}

static SaAisErrorT
saAmfParseProxiedComp(DOMNode *tmp, char *index, NCS_BOOL ext_comp_flag)
{
   SaAisErrorT          rc=SA_AIS_OK;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;

   /* Parse the attribute whether preInstantiable or not */
   DOMNamedNodeMap *attributesNodes = tmp->getAttributes();
   if(attributesNodes->getLength() )
   {
      for(unsigned int i=0; i < attributesNodes->getLength(); i++)
      {
         char *tag = XMLString::transcode(attributesNodes->item(i)->getNodeName());
         char *val = XMLString::transcode(attributesNodes->item(i)->getNodeValue());
         if(strcmp(tag, "preInstantiable") == 0)
         {
            if(atoi(val))
            {
               saAmfParseAmfComponentT(tmp, index,(ext_comp_flag == 0)?
                                       NCS_BAM_COMP_PROXIED_LOCAL_PRE_INST :
                                       NCS_BAM_COMP_EXTERNAL_PRE_INST);
            }
            else
            { 
               saAmfParseAmfComponentT(tmp, index,(ext_comp_flag == 0)?
                                       NCS_BAM_COMP_PROXIED_LOCAL_NONPRE_INST:
                                       NCS_BAM_COMP_EXTERNAL_NONPRE_INST);
            }
         }
         XMLString::release(&tag);
         XMLString::release(&val);
      }
   }

   /* Now add the Proxy comp related stuff*/
   DOMNodeList *children = tmp->getChildNodes();
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "NO proxy specific info found. ", index);
      return SA_AIS_OK;
   }

   ncs_bam_build_mib_idx(&mib_idx, index, NCSMIB_FMAT_OCT);	

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      DOMNode *tmpNode = children->item(x); 
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         /* All other tags were parsed earlier only proxy specific */
         if(!((strcmp(tmpString, "registrationRetries") == 0) ||
              (strcmp(tmpString, "registrationTimeout") == 0) ||
              (strcmp(tmpString, "proxiedCompInstCbTimeout") == 0) ||
              (strcmp(tmpString, "proxiedCompCleanCbTimeout") == 0) ))
         {
            XMLString::release(&tmpString);
            continue;
         }

         char *val = XMLString::transcode(tmpNode->getTextContent());
         
         rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                  gl_amfConfig_table_size,
                  "componentInstance", 
                  tmpString, &table_id, &param_id, &format);

         if(rc != SA_AIS_OK)
         {
            m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, tmpString);
            XMLString::release(&tmpString);
            XMLString::release(&val);
            continue;
         }

         if(strcmp(tmpString, "registrationRetries") == 0) 
         {

            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                    val, format); 
         }
         else
         {

            ncs_bam_generate_counter64_mibset(table_id, param_id, 
                                                &mib_idx, val); 
         }
         XMLString::release(&tmpString);
         XMLString::release(&val);
      }
   }

   ncs_bam_free(mib_idx.i_inst_ids);
   return SA_AIS_OK;
}

static SaAisErrorT
saAmfParseUnproxiedComp(DOMNode *tmp, char *index)
{
   saAmfParseAmfComponentT(tmp, index, NCS_BAM_COMP_UNPROXIED_NON_SA);
   return SA_AIS_OK;
}
static SaAisErrorT
saAmfParseNcsScalars(DOMNode *node)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   NCSMIB_IDX           mib_idx;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID    format;
   char                 *tmpSndHb = NULL, *tmpRcvHb = NULL;
   SaInt64T             sndVal =0, rcvVal =0;

   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No NCS Scalar objects found. ");
      return rc;
   }
   
   mib_idx.i_inst_len = 0;
   mib_idx.i_inst_ids = NULL;

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tag = XMLString::transcode(tmpNode->getNodeName());

         if(strcmp(tag, "clusterInitTime") == 0)
         {
            char *val = XMLString::transcode(tmpNode->getTextContent());   

            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                                     gl_amfConfig_table_size,
                                       "NCSScalar", 
                                       tag, &table_id, &param_id, &format);
            if(rc == SA_AIS_OK)
            {
               ncs_bam_generate_counter64_mibset(table_id, param_id, 
                                                &mib_idx, val); 
            }
      
            XMLString::release(&val);
         }
         else if(strcmp(tag, "sndHbInt") == 0)
         {
            tmpSndHb = XMLString::transcode(tmpNode->getTextContent());
         }
         else if(strcmp(tag, "rcvHbInt") == 0)
         {
            tmpRcvHb = XMLString::transcode(tmpNode->getTextContent());
         }
         XMLString::release(&tag);
      }
   }
   /* this is temp fix.. change this later */
   for(uns8 x=0; x < (tmpSndHb ? strlen(tmpSndHb) : 0); x++)
   {
      char c = tmpSndHb[x];
      sndVal = (sndVal * 10) + atoi(&c);
   }

   for(uns8 x=0; x < (tmpRcvHb ? strlen(tmpRcvHb) : 0); x++)
   {
      char c = tmpRcvHb[x];
      rcvVal = (rcvVal * 10) + atoi(&c);
   }

   if( (sndVal < 10 ) || (rcvVal < 10) || ( (2 * sndVal) >  rcvVal ))
   {
      /* log err */
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
                        "Invalid Heartbeat Intervals Configured");
      return SA_AIS_ERR_INVALID_PARAM;
   }

   /* Now the values are valid values according to the MIB but we dont know
   ** what the existing values are ? We cant send the mibsets in one 
   ** order or the other coz if the value set is invalid with the existing
   ** value of the dependent variable the mibset will be rejected
   **
   ** To get around that we send 3 mib-sets. First set the rcvHb to very 
   ** large number / MAX value and then set the sndHb and actual rcvHb values
   */

   /* set the rcvHb to MAX */

   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                            gl_amfConfig_table_size,
                              "NCSScalar", 
                              "rcvHbInt", &table_id, &param_id, &format);
   if(rc == SA_AIS_OK)
   {
      ncs_bam_generate_counter64_mibset(table_id, param_id, 
                                       &mib_idx, NCS_DEFAULT_HB_MAX); 
   }
      
   /* Now set the sndHb */

   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                            gl_amfConfig_table_size,
                              "NCSScalar", 
                              "sndHbInt", &table_id, &param_id, &format);
   if(rc == SA_AIS_OK)
   {
      ncs_bam_generate_counter64_mibset(table_id, param_id, 
                                       &mib_idx, tmpSndHb); 
   }
      
   /* Now set the rcvHb */

   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                            gl_amfConfig_table_size,
                              "NCSScalar", 
                              "rcvHbInt", &table_id, &param_id, &format);
   if(rc == SA_AIS_OK)
   {
      ncs_bam_generate_counter64_mibset(table_id, param_id, 
                                       &mib_idx, tmpRcvHb); 
   }
   XMLString::release(&tmpSndHb);
   XMLString::release(&tmpRcvHb);
      
   return SA_AIS_OK;
}
/*****************************************************************************
  PROCEDURE NAME: saAmfParseVendorExt

  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/
static SaAisErrorT
saAmfParseVendorExt(DOMNode *node)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;

   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No Vendor Extensions Configured");
      return rc;
   }
   
   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tag = XMLString::transcode(tmpNode->getNodeName());

         if(strcmp(tag, "NCSScalars") == 0)
         {
            rc = saAmfParseNcsScalars(tmpNode);
         }
         XMLString::release(&tag);
      }
   }
   return rc;
}

/*****************************************************************************
  PROCEDURE NAME: saAmfParseHealthCheck

  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/
static SaAisErrorT
saAmfParseHealthCheck(DOMNode *tmp, char *index)
{
   SaAisErrorT    	rc=SA_AIS_OK;
   DOMNodeList 	        *children; 
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx = {0};
   char                 *tag, *val;
   char                 rowStatus[4];

   DOMNamedNodeMap *attributesNodes = tmp->getAttributes();
   if(attributesNodes->getLength() )
   {
      for(unsigned int i=0; i < attributesNodes->getLength(); i++)
      {
         tag = XMLString::transcode(attributesNodes->item(i)->getNodeName());
         val = XMLString::transcode(attributesNodes->item(i)->getNodeValue());
         if(strcmp(tag, "key") == 0)
         {
	    ncs_bam_build_mib_idx(&mib_idx, index, NCSMIB_FMAT_OCT);	
	    ncs_bam_add_sec_mib_idx(&mib_idx, val, NCSMIB_FMAT_OCT);	
	 }
	 XMLString::release(&tag);
	 XMLString::release(&val);
      }
   }

   children = tmp->getChildNodes();
   for(unsigned int x=0; x < children->getLength(); x++)
   {
      DOMNode *tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         tag = XMLString::transcode(tmpNode->getNodeName());
 	 val = XMLString::transcode(tmpNode->getTextContent());

	      rc = ncs_bam_search_table_for_oid(gl_amfHealth_check, 
			gl_amfHealth_check_size,
                        "healthCheck", 
                        tag, &table_id, &param_id, &format);
                        
             if((strcmp(tag, "period") == 0) ||
                (strcmp(tag, "maxDuration") == 0))
             {
                ncs_bam_generate_counter64_mibset(table_id, param_id, 
                                             &mib_idx, val); 
             }
             else
             {
      	        ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 	
								val, format);
             }
  
	     XMLString::release(&tag);
	     XMLString::release(&val);
      }
   }
	/* Now set the Row Status */
	rc = ncs_bam_search_table_for_oid(gl_amfHealth_check, 
								gl_amfHealth_check_size,
                        "healthCheck", 
                        "rowStatus", &table_id, &param_id, &format);
                        
	sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
	if(rc == SA_AIS_OK)
   	ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 	
								rowStatus, format);

	ncs_bam_free(mib_idx.i_inst_ids);
	return SA_AIS_OK;
}

/*****************************************************************************
  PROCEDURE NAME: saAmfParseAmfComponentT

  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/
static SaAisErrorT
saAmfParseAmfComponentT(DOMNode *tmp, char *index, uns32 compType)
{
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
   SaAisErrorT          rc=SA_AIS_OK;
   DOMNodeList          *children = tmp->getChildNodes();
   char                 ctype[4];

   ncs_bam_build_mib_idx(&mib_idx, index, NCSMIB_FMAT_OCT);

   /* set the component property here */
   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                     gl_amfConfig_table_size,
                     "componentInstance", 
                     "compProperty", &table_id, &param_id, &format);
   
   sprintf(ctype, "%d", compType);

   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                    ctype, format); 

   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "Empty Component found. ", index);
      ncs_bam_free(mib_idx.i_inst_ids);
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      DOMNode *tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tag = XMLString::transcode(tmpNode->getNodeName());

         if(strcmp(tag, "healthCheck") == 0)
         {
            /* 
            ** The component name passed  as compoenent name is ignored for 
            ** this release and the entries will be placed in the global health 
            ** check table
            */
            rc = saAmfParseHealthCheck(tmpNode, index);
            XMLString::release(&tag);
            continue;
         }	

         char *val = XMLString::transcode(tmpNode->getTextContent());

         if(strcmp(tag, "capability") == 0)
         {
            sprintf(val, "%d", get_capability_value(val)); 
         }
         else if(strcmp(tag, "recoveryOnTimeout") == 0)
         {
            sprintf(val, "%d", get_recovery_value(val)); 
         }

        rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                     gl_amfConfig_table_size,
                     "componentInstance", 
                     tag, &table_id, &param_id, &format);
   
         if(rc != SA_AIS_OK)
         {
            m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, tag);
            XMLString::release(&tag);
            XMLString::release(&val);
            continue;
         }

         if((strcmp(tag, "QuiescingDoneTimeout") == 0) ||
            (strcmp(tag, "terminationTimeout") == 0) ||
            (strcmp(tag, "csiSetCallbackTimeout") == 0) ||
            (strcmp(tag, "csiRmvCallbackTimeout") == 0) )
         {
            ncs_bam_generate_counter64_mibset(table_id, param_id,
                                              &mib_idx, val);
         }
         else
         {
            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                    val, format); 
         }
	      XMLString::release(&tag);
         XMLString::release(&val);
      }
   }

   ncs_bam_free(mib_idx.i_inst_ids);
   return rc;
}

/*****************************************************************************
  PROCEDURE NAME: ncs_bam_count_set_components

  DESCRIPTION   : This function counts the number of components listed under
                  the particular SU prototype or the instance. This function
                  also generates the mibset for the corresponding variable.
  ARGUMENTS     :
                  
  RETURNS       : 
  NOTES         : 
*****************************************************************************/
static void
ncs_bam_count_set_components(DOMNode *suNode, bool fromPrototype, char *suName)
{
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
   DOMElement           *tmpElem = NULL;
   char                 count[4];
   DOMNodeList *elements = NULL;

   /* the passed argument is an ELEMENT NODE */

   if(fromPrototype)
   {
      XMLCh *str = XMLString::transcode("componentPrototype");
      tmpElem =  static_cast<DOMElement*>(suNode);
      elements = tmpElem->getElementsByTagName(str);
   }
   else
   {   
      XMLCh *str = XMLString::transcode("componentInstance");
      tmpElem =  static_cast<DOMElement*>(suNode);
      elements = tmpElem->getElementsByTagName(str);
   }

   /* 
   ** Now set the number of components in the su row with the count
   */
   ncs_bam_build_mib_idx(&mib_idx, suName, NCSMIB_FMAT_OCT);
   
   ncs_bam_search_table_for_oid(gl_amfConfig_table, 
      gl_amfConfig_table_size,
      "SUInstance", 
      "numComponents", &table_id, &param_id, &format);
   
   sprintf(count, "%d", (int)elements->getLength());
   
   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      count, format); 

   ncs_bam_free(mib_idx.i_inst_ids);

   return;
}


/* 
 * This routine is to parse the prototype and generate 
 * the mibsets for the instance which uses this prototype
 */

static SaAisErrorT
saAmfParseComponentAttributes(DOMNode *compNode, char *protoName, 
                              char *index, NCS_BOOL ext_su_flag)
{
   SaAisErrorT    rc=SA_AIS_OK;
   DOMNodeList *children = compNode->getChildNodes();
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
   char                 rowStatus[4];

   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "NO Component Attributes found. ", index);
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      DOMNode *tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tag = XMLString::transcode(tmpNode->getNodeName());

         if(strcmp(tag, "clcCommands") == 0)
         {
            saAmfParseCompClcCommands(tmpNode, index);
         }
         else if(strcmp(tag, "saAwareComponent") == 0)
         {
            saAmfParseSaAwareComp(tmpNode, index);
         }
         else if(strcmp(tag, "proxiedComponent") == 0)
         {
            saAmfParseProxiedComp(tmpNode, index, ext_su_flag);
         }
         else if(strcmp(tag, "unproxiedComponent") == 0)
         {
            saAmfParseUnproxiedComp(tmpNode, index);
         }
         else if(strcmp(tag, "csiPrototypeName") == 0)
         {
            char *val = XMLString::transcode(tmpNode->getTextContent());

            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, gl_amfConfig_table_size,
                                    "componentInstance", 
                                    "compCsTypeSuppRowStatus", &table_id, &param_id, &format);

            if(rc != SA_AIS_OK)
            {
               
               m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, tag);
               XMLString::release(&tag);
               XMLString::release(&val);
               //return rc;
               continue;
            }
 
            ncs_bam_build_mib_idx(&mib_idx, index, NCSMIB_FMAT_OCT);
            ncs_bam_add_sec_mib_idx(&mib_idx, val, NCSMIB_FMAT_OCT );

            sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);

            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                    rowStatus, format); 

            ncs_bam_free(mib_idx.i_inst_ids);
            XMLString::release(&val);
         }

         else if( (strcmp(tag, "instantiationLevel") == 0) ||
                  (strcmp(tag, "enableAM") == 0) )
         {
            char *val = XMLString::transcode(tmpNode->getTextContent());

            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, gl_amfConfig_table_size,
                                    "componentInstance", 
                                    tag, &table_id, &param_id, &format);

            if(rc != SA_AIS_OK)
            {
               
               m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, tag);
               XMLString::release(&tag);
               XMLString::release(&val);
               //return rc;
               continue;
            }
            ncs_bam_build_mib_idx(&mib_idx, index, NCSMIB_FMAT_OCT);
            
            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, val, format); 
            ncs_bam_free(mib_idx.i_inst_ids);
            XMLString::release(&val);
         }


         XMLString::release(&tag);
      }
   }

   return rc;
}

 
static SaAisErrorT
saAmfParseCompPrototype(char *protoName, char *index, bool fromPrototype, NCS_BOOL ext_su_flag)
{
   char        genName[BAM_MAX_INDEX_LEN] = {0};
   DOMNode     *compPrototype;
   NCSMIB_IDX           mib_idx;
   SaAisErrorT          rc = SA_AIS_OK;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   char                 rowStatus[4];
   
   compPrototype = get_dom_node_by_name(protoName, "componentPrototype");
   if(!compPrototype)
   {
      m_LOG_BAM_MSG_TIC(BAM_NO_PROTO_MATCH, NCSFL_SEV_WARNING, protoName);
      return SA_AIS_ERR_NOT_EXIST;
   }
   
   strncpy(genName, index, BAM_MAX_INDEX_LEN-1); /* for instances */

   /* Check to see if the finalPrototype attribute is set to true */
   DOMNamedNodeMap *attributesNodes = compPrototype->getAttributes();
   if(attributesNodes->getLength() )
   {
      for(unsigned int i=0; i < attributesNodes->getLength(); i++)
      {
         char *tag = XMLString::transcode(attributesNodes->item(i)->getNodeName());
         if(strcmp(tag, "finalPrototype") == 0)
         {
            char *val = XMLString::transcode(attributesNodes->item(i)->getNodeValue());
            if((fromPrototype) && (atoi(val)))
            {
               /* 
               * Invoke Name Generation algorithm if any for the final prototype
               * Or just for this release concatenate the nodeName with the SU
               * prototype name.
               *
               * If the prototype is from the instance leave the genName to be 
               * just the suName.
               */
               memset(genName, 0, BAM_MAX_INDEX_LEN);
               strncpy(genName, protoName, BAM_MAX_INDEX_LEN-1);
               strncat(genName, ",", BAM_MAX_INDEX_LEN-strlen(genName)-1);
               strncat(genName, index, BAM_MAX_INDEX_LEN-strlen(genName)-1);
            }
            /* we dont care about the other attributes... break */
            XMLString::release(&tag);
            XMLString::release(&val);
            break;
         }
         XMLString::release(&tag);
      }
   }
   
   saAmfParseComponentAttributes(compPrototype, protoName, genName, ext_su_flag);

   /*
   ** The SUName and row status is set only if from prototype 
   ** is true. For the instance the index is not the suName
   ** and hence setting it from the instance where its
   ** available makes sense.
   */
   if(fromPrototype)
   {
      ncs_bam_build_mib_idx(&mib_idx, genName, NCSMIB_FMAT_OCT);

      /* now set the row status to active */
      rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
         gl_amfConfig_table_size,
         "componentInstance", 
         "rowStatus", &table_id, &param_id, &format);
      
      sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
      if(rc == SA_AIS_OK)
      {
         ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
            rowStatus, format); 
      }
      
      ncs_bam_free(mib_idx.i_inst_ids);
   }

   m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
                      "component row successfully created", genName);
   return SA_AIS_OK;
}




/*****************************************************************************
  PROCEDURE NAME: saAmfParseCompProtoList

  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/
static SaAisErrorT
saAmfParseCompProtoList(DOMNode *node, char *index, NCS_BOOL ext_su_flag)
{
   if(!node)
      return SA_AIS_ERR_INVALID_PARAM;

   DOMNodeList *children = node->getChildNodes();
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "Empty Component proto list found. ", index);
      return SA_AIS_OK;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      DOMNode *tmpNode = children->item(x); /* The compProto Name */
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         char *val = XMLString::transcode(tmpNode->getTextContent());
         if(strcmp(tmpString, "componentPrototype") == 0)
         {
            saAmfParseCompPrototype(val, index, TRUE, ext_su_flag);           
         }
         XMLString::release(&tmpString);
         XMLString::release(&val);
      }
   }
   return SA_AIS_OK;
}


/*****************************************************************************
  PROCEDURE NAME: saAmfParseSUPrototype
  DESCRIPTION   : This routine searches for the SU prototype that matches
                  the name passed, then generates the name for the 
                  instance and then parse the suproto and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/
static SaAisErrorT
saAmfParseSUPrototype(char *suName, char *index, bool fromPrototype, 
                      NCS_BOOL ext_su_flag)
{
   char                 genName[BAM_MAX_INDEX_LEN];
   DOMNode              *suPrototype;
   NCSMIB_IDX           mib_idx;
   SaAisErrorT          rc = SA_AIS_OK;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   char                 def_val[4];
   
   suPrototype = get_dom_node_by_name(suName, "SUPrototype");
   if(!suPrototype)
   {
      m_LOG_BAM_MSG_TIC(BAM_NO_PROTO_MATCH, NCSFL_SEV_WARNING, suName);
      return SA_AIS_ERR_NOT_EXIST;
   }
   
   strncpy(genName, index, BAM_MAX_INDEX_LEN-1); /* for instances */

   /* Check to see if the finalPrototype attribute is set to true */
   DOMNamedNodeMap *attributesNodes = suPrototype->getAttributes();
   if(attributesNodes->getLength() )
   {
      for(unsigned int i=0; i < attributesNodes->getLength(); i++)
      {
         char *tag = XMLString::transcode(attributesNodes->item(i)->getNodeName());
         char *val = XMLString::transcode(attributesNodes->item(i)->getNodeValue());
         if(strcmp(tag, "finalPrototype") == 0)
         {
            if((fromPrototype) && (atoi(val)))
            {
            /****************************************************************
            ** Ignore the SUFinalPrototype for now coz the name generation 
            ** algorithm will generate an SU from SG which doesnt match 
            ** with the SU from node and without both parentSG and parentNode 
            ** the SU will not be active so get rid of final prototype.
            **
            *****************************************************************/

            /* 
            * Invoke Name Generation algorithm if any for the final prototype
            * Or just for this release concatenate the nodeName with the SU
            * prototype name.
            *
            * If the prototype is from the instance leave the genName to be 
            * just the suName.
            */
               memset(genName, 0, BAM_MAX_INDEX_LEN);
               strncpy(genName, suName, BAM_MAX_INDEX_LEN-1);
               strncat(genName, ",", BAM_MAX_INDEX_LEN-strlen(genName)-1);
               strncat(genName, index, BAM_MAX_INDEX_LEN-strlen(genName)-1);
            }
            /* we dont care about the other attributes... break */
            XMLString::release(&tag);
            XMLString::release(&val);
            break;
         }
         XMLString::release(&tag);
         XMLString::release(&val);
      }
   } /* end of parsing attributes */
   
   DOMNodeList *children = suPrototype->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "Empty SU found. ", suName);
      return SA_AIS_OK;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      DOMNode *tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         

         if(strcmp(tmpString, "SUAttributes") == 0)
         {
            saAmfParseSUAttributes(tmpNode, genName); /* catch the error and return ? */

            /*
            ** set the row status right here so that walking the
            ** component list and setting the parent SU Name the check 
            ** for SU row status active will not fail.
            **
            */
            /*
            ** Note that if componentList is missing in XML the 
            ** request to set the rowStatus active
            ** will be rejected anyway by AVD code.
            ** SO, set the rowStatus Active in the comp parse context. 
            */
         }
         else if(strcmp(tmpString, "componentPrototypeList") == 0)
         {
            
            /* Set the num of components value and si_max_active variables
            ** in the SU for the row status to be active
            */
            ncs_bam_count_set_components(suPrototype, TRUE, genName);

            /*
            ** set the row status right here so that walking the
            ** component list and setting the parent SU Name the check 
            ** for SU row status active will not fail.
            **
            ** Note that if componentList is missing in XML the 
            ** request to set the rowStatus active
            ** will be rejected anyway by AVD code.
            */
            ncs_bam_build_mib_idx(&mib_idx, genName, NCSMIB_FMAT_OCT);

            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                  gl_amfConfig_table_size, "SUInstance", 
                  "rowStatus", &table_id, &param_id, &format);
   
            sprintf(def_val, "%d", NCSMIB_ROWSTATUS_ACTIVE);
            if(rc == SA_AIS_OK)
               ncs_bam_build_and_generate_mibsets(table_id, param_id, 
                                &mib_idx, def_val, format); 
   
            /* Now go ahead and parse comp */
            saAmfParseCompProtoList(tmpNode, genName, ext_su_flag);
         }

         XMLString::release(&tmpString);
      }
   }
   return SA_AIS_OK;
}



/*****************************************************************************
  PROCEDURE NAME: saAmfParseSUFinalProtoList
  DESCRIPTION   : This routine searches for the SU prototype that matches
                  the name passed, then generates the name for the instance 
                  and then parse the suproto and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/
static SaAisErrorT
saAmfParseSUFinalProtoList(DOMNode *node, char *index)
{

   /* have to check the flag if that is really a final prototype
   else its a config error */
   if(!node)
      return SA_AIS_ERR_INVALID_PARAM;

   DOMNodeList *children = node->getChildNodes();
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No SUs found in the list. ", index);
      return SA_AIS_OK;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      DOMNode *tmpNode = children->item(x); /* The SU Name */
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         char *val = XMLString::transcode(tmpNode->getTextContent());
         if(strcmp(tmpString, "SUFinalPrototype") == 0)
         {
            saAmfParseSUPrototype(val, index, TRUE,FALSE);
         }
         XMLString::release(&tmpString);
         XMLString::release(&val);
      }
   }
   return SA_AIS_OK;
}


/*****************************************************************************
  PROCEDURE NAME: saAmfParseNodePrototype
  DESCRIPTION   : This routine searches for the node prototype that matches
                  the name passed, then parse the node and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/
static SaAisErrorT
saAmfParseNodePrototype(char *prototypeName, char *index, BAM_PARSE_SUB_TREE sub_tree)
{
   SaAisErrorT rc= SA_AIS_OK;
   /* First find the DOM node with prototype name match */
   DOMNode *nodePrototype;
   nodePrototype = get_dom_node_by_name(prototypeName, "nodePrototype");
   if(!nodePrototype)
   {
      m_LOG_BAM_MSG_TIC(BAM_NO_PROTO_MATCH, NCSFL_SEV_WARNING, prototypeName);
      return SA_AIS_ERR_NOT_EXIST;
   }
   
   DOMNodeList *children = nodePrototype->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "Empty NODE proto found. ", prototypeName);
      return SA_AIS_OK;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      DOMNode *tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if((strcmp(tmpString, "nodeAttributes") == 0) &&
            (sub_tree == BAM_PARSE_NCS) )
            rc = saAmfParseNodeAttributes(tmpNode, index);
         else if(strcmp(tmpString, "SUFinalPrototypeList") == 0)
            rc = saAmfParseSUFinalProtoList(tmpNode, index);

         XMLString::release(&tmpString);
      }
      /* should be check for error and quit or ignore and continue ? */
   }
   return rc; 
}


/*****************************************************************************
  PROCEDURE NAME: saAmfParseCompClcCommands
  DESCRIPTION   : This routine is to parse the component life cycle scripts
                  and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : The first phase of implementation, the mibs are defined only
                  for the script commands and all the commands in the 
                  static mapping table are under component table and hence
                  the tag names for the search and look up in component table 
                  should match to get the oid.
*****************************************************************************/

static SaAisErrorT
saAmfParseCompClcCommands(DOMNode *node, char *index)
{
   SaAisErrorT             rc = SA_AIS_OK;
//   SAF_SCRIPT_COMMAND   cmd;
   DOMNode              *tmpNode = NULL;
   DOMNode              *command = NULL;
   DOMNodeList          *children = node->getChildNodes();
   char                 commandString[BAM_MAX_INDEX_LEN];
   char                 *lookup;
   DOMNodeList          *cmdNodes = NULL;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
   char                 ctype[4];
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No CLC commands found. ", index);
      return rc;
   }

   ncs_bam_build_mib_idx(&mib_idx, index, NCSMIB_FMAT_OCT);

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         memset(commandString, 0, BAM_MAX_INDEX_LEN-1);
         strncpy(commandString, tmpString, BAM_MAX_INDEX_LEN-1); 
                                      /* This string could be one of the 
                                       * a. instantiateCommand
                                       * b. terminateCommand
                                       * c. cleanupCommand
                                       * d. amStartCommand
                                       * e. amStopCommand
                                       */


         /*saAmfParseClcParams(tmpNode, index, tmpString); */

         cmdNodes = tmpNode->getChildNodes();
         if(cmdNodes->getLength())
         {
            for(unsigned int y = 0; y < cmdNodes->getLength(); y++)
            {
               command = cmdNodes->item(y);
               if(command && command->getNodeType() == DOMNode::ELEMENT_NODE)
               {
                  char *tag = XMLString::transcode(command->getNodeName());
                  char *val = XMLString::transcode(command->getTextContent());
         
                  if(strcmp(tag, "command") == 0)
                  {
                     lookup = commandString;
                     rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                            gl_amfConfig_table_size,
                            "componentInstance", 
                            lookup, &table_id, &param_id, &format);

                  }
                  else if(strcmp(tag, "timeout") == 0)
                  {
                     char *c_ptr = strstr(commandString, "Command");
                     strncpy(c_ptr, "\0", 1);
                     strncat(commandString, tag, BAM_MAX_INDEX_LEN-strlen(commandString)-1);
                     lookup = commandString;
                     rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                            gl_amfConfig_table_size,
                            "componentInstance", 
                            lookup, &table_id, &param_id, &format);

                     ncs_bam_generate_counter64_mibset(table_id, param_id,
                                                 &mib_idx, val);

                     XMLString::release(&tag);
                     XMLString::release(&val);
                     continue;
                  }
                  else
                  {

                     rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                                       gl_amfConfig_table_size,
                                       "componentInstance", 
                                       tag, &table_id, 
                                       &param_id, &format);
                  }

                  if(rc != SA_AIS_OK)
                  {
                     m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, 
                                       tag);
                     XMLString::release(&tag);
                     XMLString::release(&val);
                     continue;
                  }
                  ncs_bam_build_and_generate_mibsets(table_id, param_id, 
                                                  &mib_idx, val, format); 
                  XMLString::release(&tag);
                  XMLString::release(&val);
               }
            } 
         }  /* End of parsing CLC params */

         /* set the default value for the retries in stop command */
         if( (strcmp(tmpString, "amStartCommand") == 0) )
         {
            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                     gl_amfConfig_table_size,
                     "componentInstance", 
                     "MaxAmstartRetries", &table_id, &param_id, &format);
   
            sprintf(ctype, "%d", 1);

            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                    ctype, format); 
         }

         XMLString::release(&tmpString);
      } /* endif */
   } /* outer loop for the ALL the commands */
   ncs_bam_free(mib_idx.i_inst_ids);
   return SA_AIS_OK;
}

/*****************************************************************************
  PROCEDURE NAME: saAmfParseNodeAttributes
  DESCRIPTION   : This routine is to parse the AMF Node Attributes
                  like node escalation etc.. and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseNodeAttributes(DOMNode *node, char *index)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   DOMNodeList          *grandChildren = NULL;
   DOMNode              *errEscNode = NULL;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
   char                 rowStatus[4];
      
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, 
                        index);
      return SA_AIS_ERR_INVALID_PARAM;
   }
   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No Node Attributes found.",index);  
      return rc;
   }

   ncs_bam_build_mib_idx(&mib_idx, index, NCSMIB_FMAT_OCT);
   
   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(strcmp(tmpString, "nodeErrorEscalation") == 0)
         {
            grandChildren = tmpNode->getChildNodes();
            if(grandChildren->getLength())
            {
               for(unsigned int y=0; y < grandChildren->getLength(); y++)
               {
                  errEscNode = grandChildren->item(y);
                  if(errEscNode && errEscNode->getNodeType() == DOMNode::ELEMENT_NODE)
                  {
                     char *tag = XMLString::transcode(errEscNode->getNodeName());
                     char *val = XMLString::transcode(errEscNode->getTextContent());
                     
                     rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                                    gl_amfConfig_table_size,
                                    "nodeInstance", 
                                    tag, &table_id, &param_id, &format);

                     if(rc != SA_AIS_OK)
                     {
                        m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, 
                                       tag);
                        //return rc;
                        XMLString::release(&tag);
                        XMLString::release(&val);
                        continue;
                     }
                     if(strcmp(tag, "SUFailoverProbation") == 0)
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
            }
         }
         else if(strcmp(tmpString, "adminstate")==0)
         {
            /*Denote Initial admin state ,value received in String*/
            char *val = XMLString::transcode(tmpNode->getTextContent());
            
            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, gl_amfConfig_table_size,
                                    "nodeInstance", 
                                    tmpString, &table_id, &param_id, &format);
            if(rc != SA_AIS_OK)
            {
               m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING, 
                                       tmpString);
               XMLString::release(&tmpString);
               XMLString::release(&val);
               continue;
            }
       
            sprintf(val, "%d", get_admstate_val(val));
            ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                               val, format);

            XMLString::release(&val);

         }
         XMLString::release(&tmpString);
      }
   }

   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
            gl_amfConfig_table_size,
            "nodeInstance", 
            "rowStatus", &table_id, &param_id, &format);

   if(rc != SA_AIS_OK)
      return rc;

   sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      rowStatus, format); 

   ncs_bam_free(mib_idx.i_inst_ids);
   m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
                      "Node created successfully", index);
   return SA_AIS_OK;
}


/*****************************************************************************
  PROCEDURE NAME: saAmfParseSUAttributes
  DESCRIPTION   : This routine is to parse the list of SU Attributes
                  and generate the mibsets.
  ARGUMENTS     : DOMNode
                  suName, this is the index for the mibsets.
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseSUAttributes(DOMNode *node, char *suName)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_IDX           mib_idx;
   NCSMIB_FMAT_ID    format;


   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No SU Attributes found. ", suName);
      return rc;
   }
   
   ncs_bam_build_mib_idx(&mib_idx, suName, NCSMIB_FMAT_OCT);

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tag = XMLString::transcode(tmpNode->getNodeName());
         char *val = XMLString::transcode(tmpNode->getTextContent());   

         if(strcmp(tag, "vendorExtensions") == 0)
         {
            XMLString::release(&tag);
            XMLString::release(&val);
            continue;
         }

         rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
		                            gl_amfConfig_table_size,
                                    "SUInstance", 
                                    tag, &table_id, &param_id, &format);
         
         if(rc != SA_AIS_OK)
           {
             m_LOG_BAM_MSG_TIC(BAM_NO_OID_MATCH, NCSFL_SEV_WARNING,
                                       tag);
             XMLString::release(&tag);
             XMLString::release(&val);
               continue;
            }
  
         if(strcmp(tag, "adminstate")==0)
          {
               sprintf(val, "%d", get_admstate_val(val));
               ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                               val, format); 
          }
          else
          {
               ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
                                               val, format); 
          }
   
         XMLString::release(&tag);
         XMLString::release(&val);
      }
   }
   ncs_bam_free(mib_idx.i_inst_ids);
   return SA_AIS_OK;
}


/*****************************************************************************
  PROCEDURE NAME: saAmfParseComponentInstance
  DESCRIPTION   : This routine is to parse the list of SU Instances
                  and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseComponentInstance(DOMNode *node, char *suName, NCS_BOOL ext_su_flag)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   char                 *tag, *compName=NULL;
   NCSMIB_TBL_ID        table_id;   /* table id */
   uns32                param_id; 
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
   char                 genName[BAM_MAX_INDEX_LEN];
   char                 rowStatus[4];
   
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, suName);
      return SA_AIS_ERR_INVALID_PARAM;
   }

   DOMNamedNodeMap *attributesNodes = node->getAttributes();
   if(attributesNodes->getLength() )
   {
      /* Only one attribute so lets avoid loop and get the name */
      tag = XMLString::transcode(attributesNodes->item(0)->getNodeName());
      if(strcmp(tag, "name") != 0)
      {
         XMLString::release(&tag);
         return SA_AIS_ERR_INVALID_PARAM;
      }
      compName = XMLString::transcode(attributesNodes->item(0)->getNodeValue());

      memset(genName, 0, BAM_MAX_INDEX_LEN);
      strncpy(genName, compName, BAM_MAX_INDEX_LEN-1);
      strncat(genName, ",", BAM_MAX_INDEX_LEN-strlen(genName)-1);
      strncat(genName, suName, BAM_MAX_INDEX_LEN-strlen(genName)-1);

      XMLString::release(&compName);
      XMLString::release(&tag);
   }

   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "Empty Comp Instance. ", genName);
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(strcmp(tmpString, "prototypeName") == 0)
         {
            char *val = XMLString::transcode(tmpNode->getTextContent());
            rc = saAmfParseCompPrototype(val, genName, FALSE, ext_su_flag);
            if(rc != SA_AIS_OK)
            {
               XMLString::release(&tmpString);
               XMLString::release(&val);
               return rc;
            }
            XMLString::release(&val);
         }
         else if(strcmp(tmpString, "componentAttributes") == 0)
            rc = saAmfParseComponentAttributes(tmpNode, NULL, genName, ext_su_flag);

         XMLString::release(&tmpString);
      }
   }

   /* now set the row status to active */
   ncs_bam_build_mib_idx(&mib_idx, genName, NCSMIB_FMAT_OCT);

   rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
            gl_amfConfig_table_size,
            "componentInstance", 
            "rowStatus", &table_id, &param_id, &format);

   sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_ACTIVE);
   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      rowStatus, format); 

   ncs_bam_free(mib_idx.i_inst_ids);

   m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO,
                     "component created successfully", genName);
   return rc;
}
   


/*****************************************************************************
  PROCEDURE NAME: saAmfParseComponentLists
  DESCRIPTION   : This routine is to parse the list of component instances
                  and generate the mibsets.
  ARGUMENTS     :
                  suName for the parent Su name to be marked in the component.
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseComponentList(DOMNode *node, char *suName, NCS_BOOL ext_su_flag)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, suName);
      return SA_AIS_ERR_INVALID_PARAM;
   }
   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No Component Instances found. ", suName);
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         
         if(strcmp(tmpString, "componentInstance") != 0)
         {
            XMLString::release(&tmpString);
            return SA_AIS_ERR_NOT_SUPPORTED;
         }
         XMLString::release(&tmpString);
         rc = saAmfParseComponentInstance(tmpNode, suName, ext_su_flag);
         if(rc != SA_AIS_OK)
            return rc;
      }
   }
   return SA_AIS_OK;
}        

/*****************************************************************************
  PROCEDURE NAME: saAmfParseSUInstance
  DESCRIPTION   : This routine is to parse the list of SU Instances
                  and generate the mibsets.
  ARGUMENTS     : node           : node pointer,
                  parentNodeName : Pointer to its parent node name,
                  ext_su_flag    : TRUE if it has to parse external SU Instance,
                                   else FALSE.
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

static SaAisErrorT
saAmfParseSUInstance(DOMNode *node, char *parentNodeName, NCS_BOOL ext_su_flag)
{
   SaAisErrorT          rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   char                 *tag, *suName=NULL;
   NCSMIB_TBL_ID        table_id;   /* table id */
   uns32                param_id; 
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
   char                 genName[BAM_MAX_INDEX_LEN];
   char                 def_val[4];
   /* The following variables are used for external SU validation. In any case,
      only one prototype and one SU Attr or one comp list and one SU Attr should
      be there.*/
   uns32                prototype_counter=0;
   uns32                complist_counter=0;
   uns32                su_attr_counter=0;
   
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, "SUInstance");
      return SA_AIS_ERR_INVALID_PARAM;
   }

   DOMNamedNodeMap *attributesNodes = node->getAttributes();
   if(attributesNodes->getLength() )
   {
         /* there is only one attribute but if added in future */
      for(unsigned int x=0; x < attributesNodes->getLength(); x++)
      {
         tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
         if(strcmp(tag, "name") == 0)
         {
            suName = XMLString::transcode(attributesNodes->item(x)->getNodeValue());
            memset(genName, 0, BAM_MAX_INDEX_LEN);
            strncpy(genName, suName, BAM_MAX_INDEX_LEN-1);
            if(ext_su_flag == FALSE)
            {
              /* For external component, there is no node name. */
              strncat(genName, ",", BAM_MAX_INDEX_LEN-strlen(genName)-1);
              strncat(genName, parentNodeName, BAM_MAX_INDEX_LEN-strlen(genName)-1);
            }
            XMLString::release(&suName);
         }
         else
         {
            XMLString::release(&tag);
            continue;
         }

         XMLString::release(&tag);
      }
   }

   /* ***************************************************************
   ** Now parse the instance only if this is in the context of 
   ** parsing a nodeInstance or parentNode is present
   ** ***************************************************************
   */

   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No SU Attributes and comp list found. ", genName);
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(strcmp(tmpString, "prototypeName") == 0)
         {
            if((0 == prototype_counter) || (FALSE == ext_su_flag))
            {
              char *val = XMLString::transcode(tmpNode->getTextContent());
              rc = saAmfParseSUPrototype(val, genName, FALSE, ext_su_flag);
              if(rc != SA_AIS_OK)
              {
                 XMLString::release(&tmpString);
                 XMLString::release(&val);
                 return rc;
              }
              XMLString::release(&val);
            }
            else
            {
              m_LOG_BAM_MSG_TICL(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
                                 "More than one prototype configured: ", 
                                 tmpString);
            }
            prototype_counter ++;
         }
         else if(strcmp(tmpString, "SUAttributes") == 0)
         {
            if((su_attr_counter == 0) || (FALSE == ext_su_flag))
            {
               rc = saAmfParseSUAttributes(tmpNode, genName);
            }
            else
            {
              m_LOG_BAM_MSG_TICL(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
                                 "More than one SUAttributes configured: ", 
                                 tmpString);
            }
            su_attr_counter ++;
            /*
            ** set the row status right here so that walking the
            ** component list and setting the parent SU Name the check 
            ** for SU row status active will not fail.
            **
            */
            /*
            ** Note that if componentList is missing in XML the 
            ** request to set the rowStatus active
            ** will be rejected anyway by AVD code.
            ** SO, set the rowStatus Active in the comp parse context. 
            */
         }

         else if(strcmp(tmpString, "componentList") == 0)
         {

          if((complist_counter == 0) || (FALSE == ext_su_flag))
          {
            /* Set the num of components value and si_max_active variables
            ** in the SU for the row status to be active
            */
            ncs_bam_count_set_components(node, FALSE, genName);

            /*
            ** set the row status right here so that walking the
            ** component list and setting the parent SU Name the check 
            ** for SU row status active will not fail.
            **
            ** Note that if componentList is missing in XML the 
            ** request to set the rowStatus active
            ** will be rejected anyway by AVD code.
            */
            ncs_bam_build_mib_idx(&mib_idx, genName, NCSMIB_FMAT_OCT);

            rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
                  gl_amfConfig_table_size, "SUInstance", 
                  "rowStatus", &table_id, &param_id, &format);
   
            sprintf(def_val, "%d", NCSMIB_ROWSTATUS_ACTIVE);
            if(rc == SA_AIS_OK)
               ncs_bam_build_and_generate_mibsets(table_id, param_id, 
                                &mib_idx, def_val, format); 
   
            /* Now that rowStatus is set, go ahead and parse comps */
            rc = saAmfParseComponentList(tmpNode, genName, ext_su_flag);
          }
          else
          {
              m_LOG_BAM_MSG_TICL(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
                                 "More than one componentList configured: ", 
                                 tmpString);
          }
          complist_counter++;
         }

         XMLString::release(&tmpString);
      }
   }

   return SA_AIS_OK;
}


/*****************************************************************************
  PROCEDURE NAME: saAmfParseSUList
  DESCRIPTION   : This routine is to parse the list of SU Instances
                  and generate the mibsets.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/

SaAisErrorT
saAmfParseSUList(DOMNode *node, char *parentNodeName)
{
   SaAisErrorT             rc = SA_AIS_OK;
   DOMNode              *tmpNode = NULL;
   DOMNodeList          *children = NULL;
   
   if(!node)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR, "SUList");
      return SA_AIS_ERR_INVALID_PARAM;
   }
   children = node->getChildNodes();
   
   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No SU Instances found. ");
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         
         if(strcmp(tmpString, "SUInstance") != 0)
         {
            XMLString::release(&tmpString);
            return SA_AIS_ERR_NOT_SUPPORTED;
         }
         XMLString::release(&tmpString);
         rc = saAmfParseSUInstance(tmpNode, parentNodeName,FALSE);
         if(rc != SA_AIS_OK)
            return rc;
      }
   }
   return SA_AIS_OK;
}        


static SaAisErrorT
set_ncs_node_table(char *nodeName, char *nodeId)
{
   NCSMIB_TBL_ID     table_id;   /* table id */
   uns32             param_id; 
   NCSMIB_FMAT_ID    format;
   NCSMIB_IDX        mib_idx;
   uns32 nd_id;
   char node_id[32];

   /* set the NodeName as the index.  */
   ncs_bam_build_mib_idx(&mib_idx, nodeName, NCSMIB_FMAT_OCT); 

   /* set the value of nodeId in the NCS mib table */
   
   ncs_bam_search_table_for_oid(gl_amfConfig_table, 
      gl_amfConfig_table_size, "nodeInstance", 
      "ncsNodeID", &table_id, &param_id, &format);

   sscanf(nodeId,"%x",&nd_id);
   memset(node_id,0,sizeof(node_id));
   sprintf(node_id,"%u", nd_id);
   
   ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
      node_id, format); 

   /* The MIB object ncsNDIsSysCtrl is set/left to default ( false )
   **
   ** The row status of the AMFnode table control the entries/rows 
   ** in the table also. The rows in this table are not standalone.
   */

   ncs_bam_free(mib_idx.i_inst_ids);
   
   return SA_AIS_OK;
}

/* 
 * This routine is to parse the node instance and generate 
 * the mibsets for this instance. Note that this instance may
 * use prototypes also which need to ne instantiated.
 */
 
static SaAisErrorT
saAmfParseNodeInstance(DOMNode *node, BAM_PARSE_SUB_TREE sub_tree)
{
   SaAisErrorT     rc = SA_AIS_OK;
   DOMNode *tmpNode = NULL;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   NCSMIB_FMAT_ID       format;
   NCSMIB_IDX           mib_idx;
   char                 nodeId[BAM_MAX_INDEX_LEN];
   char                 nodeName[BAM_MAX_INDEX_LEN];
   char                 rowStatus[4];

   /* Check to make sure that this is an ELEMENT NODE */
   if(node->getNodeType() != DOMNode::ELEMENT_NODE)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
          "Error in processing the DOM Tree expected Node Instance.");
      return SA_AIS_ERR_INVALID_PARAM;
   }

   DOMNamedNodeMap *attributesNodes = node->getAttributes();
   if(attributesNodes->getLength() )
   {
      for(unsigned int x=0; x < attributesNodes->getLength(); x++)
      {
         char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
         char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());

         if(strcmp(tag, "nodeID") == 0)
         {
            strncpy(nodeId, val, BAM_MAX_INDEX_LEN-1);
            XMLString::release(&tag);
            XMLString::release(&val);
            continue;
         }
         else if(strcmp(tag, "name") == 0)
         {
            strncpy(nodeName, val, BAM_MAX_INDEX_LEN-1);
         }
      }
   }

   if(sub_tree == BAM_PARSE_NCS)
   {
      ncs_bam_build_mib_idx(&mib_idx, nodeName, NCSMIB_FMAT_OCT);

      rc = ncs_bam_search_table_for_oid(gl_amfConfig_table, 
               gl_amfConfig_table_size,
               "nodeInstance", 
               "rowStatus", &table_id, &param_id, &format);

      if(rc != SA_AIS_OK)
         return rc;

      sprintf(rowStatus, "%d", NCSMIB_ROWSTATUS_CREATE_WAIT);
      ncs_bam_build_and_generate_mibsets(table_id, param_id, &mib_idx, 
         rowStatus, format); 

      ncs_bam_free(mib_idx.i_inst_ids);
   
      /*
      ** From Release 2.0 the attribute "name" is the index and nodeId will
      ** also be configured in the NCS mib table along with the AMF mib table. 
      */

      set_ncs_node_table(nodeName, nodeId);
   }
 
   DOMNodeList * children = node->getChildNodes();

   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No SU Instances found in Node. ");
      return rc;
   }

   for(unsigned x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());
         

         /* If the prototype tag is present and the name is mentioned
         ** parse the prototype and generate mib requests. Argument val is
         ** sufficient argument as the prototypes will be searched with 
         ** the name and then parsed. The node is not an argument 
         ** coz we dont have handle to the prototype as it hangs 
         ** from the main tree not a containment here
         */
         if(strcmp(tmpString, "prototypeName") == 0)
         {
            char *tagValue = XMLString::transcode(tmpNode->getTextContent());
            saAmfParseNodePrototype(tagValue, nodeName, sub_tree);
            XMLString::release(&tagValue);
         }
         
         /*  Now do we do anything with the special flags of final prototype and
         * stuff???
         * 
         * probably nothing in the parsing ??
         */

         else if ((strcmp(tmpString, "nodeAttributes") == 0) &&
                  (sub_tree == BAM_PARSE_NCS) )
            rc = saAmfParseNodeAttributes(tmpNode, nodeName);
         else if(strcmp(tmpString, "SUList") == 0)
            rc = saAmfParseSUList(tmpNode, nodeName);

         XMLString::release(&tmpString);
      }    
   }
   
   return SA_AIS_OK;
}

/* 
 * This routine is the entry point to parse all the AMF objects.
 *
 */
SaAisErrorT
saAmfConfigParseAllObjs(DOMNode *node, BAM_PARSE_SUB_TREE sub_tree)
{
   SaAisErrorT     rc = SA_AIS_OK;
   DOMNode *tmpNode = NULL;
   DOMNodeList *children = node->getChildNodes();

   if(children->getLength() == 0)
   {
      m_LOG_BAM_MSG_TIC(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "No AMF Object found . ");
      return rc;
   }

   for(unsigned int x=0; x < children->getLength(); x++)
   {
      tmpNode = children->item(x);
      if(tmpNode && tmpNode->getNodeType() == DOMNode::ELEMENT_NODE)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         m_LOG_BAM_MSG_TICL(BAM_PARSE_SUCCESS, NCSFL_SEV_INFO, 
                        "The AMF Config Node to be parsed is: ", tmpString);
         
         if(strcmp(tmpString, "SGInstance") == 0)
            rc = saAmfParseSGInstance(tmpNode);
         
         else if(strcmp(tmpString, "nodeInstance") == 0)
            rc = saAmfParseNodeInstance(tmpNode, sub_tree);

         else if(strcmp(tmpString, "vendorExtensions") == 0)
            rc = saAmfParseVendorExt(tmpNode);
         else if(strcmp(tmpString, "extSUInstance") == 0)
            rc = saAmfParseExtSUInstance(tmpNode);

         if(rc != SA_AIS_OK)
            m_LOG_BAM_MSG_TICL(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
                        "Error encountered parsing: ", tmpString);

         XMLString::release(&tmpString);
      }
   }
   return rc;
}

/*****************************************************************************
  PROCEDURE NAME: saAmfParseExtSUInstance

  DESCRIPTION   : This routine is to parse the SU instance and generate
                  the mibsets for this instance. Note that this instance may
                  use prototypes also which need to be instantiated.
  ARGUMENTS     : Pointer to the DOM Node.

  RETURNS       : SUCCESS or FAILURE
  NOTES         :
*****************************************************************************/
static SaAisErrorT saAmfParseExtSUInstance(DOMNode *node)
{
   SaAisErrorT          rc = SA_AIS_OK;
   char                 nodeName[BAM_MAX_INDEX_LEN];

   memset(&nodeName, '\0', BAM_MAX_INDEX_LEN);
   
   rc = saAmfParseSUInstance(node, (char *)&nodeName, TRUE);

   if(rc != SA_AIS_OK)
   {
     m_LOG_BAM_MSG_TICL(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
       "Error encountered parsing: ", "saAmfParseSUInstance returned failure");
     return rc;
   }
     return rc;
}
