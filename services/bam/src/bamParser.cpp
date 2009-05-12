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

  This file captures all the generic routines common to BAM module to generate 
  the mibsets for the SAF specified configurable objects in the AMF.
  
******************************************************************************
*/

#include <ncs_opt.h>
#include <gl_defs.h>
#include <t_suite.h>
#include <ncsgl_defs.h>
#include <ncs_mib_pub.h>
#include <mac_papi.h>
#include <ncsmiblib.h>

#include <bam.h>
#include <bamParser.h>
#include <bamError.h>
#include <bam_log.h>

XERCES_CPP_NAMESPACE_USE

DOMDocument *gl_dom_doc;
EXTERN_C uns32 gl_ncs_bam_hdl;
EXTERN_C uns32 gl_bam_avd_cfg_msg_num;
EXTERN_C uns32 gl_bam_avm_cfg_msg_num;

TAG_OID_MAP_NODE gl_amfConfig_table[] =
{
   /* node instance table */
   {"nodeInstance",      "name",               
      NCSMIB_TBL_AVSV_AMF_NODE,   1, NCSMIB_FMAT_OCT},
   /* watch out the below entry is NCS TABLE */
   {"nodeInstance",      "ncsNodeID",
      NCSMIB_TBL_AVSV_NCS_NODE,   2, NCSMIB_FMAT_INT},
   {"nodeInstance",      "SUFailoverProbation",
      NCSMIB_TBL_AVSV_AMF_NODE,   2, NCSMIB_FMAT_OCT},
   {"nodeInstance",      "SUFailoverMax",         
      NCSMIB_TBL_AVSV_AMF_NODE,   3, NCSMIB_FMAT_INT},
   {"nodeInstance",      "adminstate",
      NCSMIB_TBL_AVSV_AMF_NODE,   4, NCSMIB_FMAT_INT},
   {"nodeInstance",      "nodeAdminStatus",
      NCSMIB_TBL_AVSV_AMF_NODE,   4, NCSMIB_FMAT_INT},
   {"nodeInstance",      "rowStatus",         
      NCSMIB_TBL_AVSV_AMF_NODE,   5, NCSMIB_FMAT_INT},


   /* SG Instance Table */
   /* SU Type(5), Cluster Name(6) and Admin Status(7) are not set */
   {"SGInstance",        "name",                  
      NCSMIB_TBL_AVSV_AMF_SG,     1, NCSMIB_FMAT_OCT},
   {"SGInstance",        "redundancyModel",       
      NCSMIB_TBL_AVSV_AMF_SG,     2, NCSMIB_FMAT_INT},
   {"SGInstance",        "failbackOption",       
      NCSMIB_TBL_AVSV_AMF_SG,     3, NCSMIB_FMAT_BOOL},
   {"SGInstance",        "numPrefActiveSUs",
      NCSMIB_TBL_AVSV_AMF_SG,     4, NCSMIB_FMAT_INT},
   {"SGInstance",        "numPrefStandbySUs",
      NCSMIB_TBL_AVSV_AMF_SG,     5, NCSMIB_FMAT_INT},
   {"SGInstance",        "numPrefInserviceSUs",
      NCSMIB_TBL_AVSV_AMF_SG,     6, NCSMIB_FMAT_INT},
   {"SGInstance",        "valueN",
      NCSMIB_TBL_AVSV_AMF_SG,     7, NCSMIB_FMAT_INT},
   {"SGInstance",        "numPrefAssignedSUs",
      NCSMIB_TBL_AVSV_AMF_SG,     7, NCSMIB_FMAT_INT},
   {"SGInstance",        "maxActiveSIsperSU",
      NCSMIB_TBL_AVSV_AMF_SG,     8, NCSMIB_FMAT_INT},
   {"SGInstance",        "maxStandbySIsperSU",
      NCSMIB_TBL_AVSV_AMF_SG,     9, NCSMIB_FMAT_INT},
   {"SGInstance",        "adminstate",
      NCSMIB_TBL_AVSV_AMF_SG,     10, NCSMIB_FMAT_INT},
   {"SGInstance",        "componentRestartProbation",
      NCSMIB_TBL_AVSV_AMF_SG,     11, NCSMIB_FMAT_OCT},
   {"SGInstance",        "componentRestartMax",    
      NCSMIB_TBL_AVSV_AMF_SG,     12, NCSMIB_FMAT_INT},
   {"SGInstance",        "SURestartProbation",
      NCSMIB_TBL_AVSV_AMF_SG,     13, NCSMIB_FMAT_OCT},
   {"SGInstance",        "SURestartMax", 
      NCSMIB_TBL_AVSV_AMF_SG,     14, NCSMIB_FMAT_INT},
   {"SGInstance",        "rowStatus", 
      NCSMIB_TBL_AVSV_AMF_SG,     18, NCSMIB_FMAT_INT},
   {"SGInstance",        "isNCS", 
      NCSMIB_TBL_AVSV_NCS_SG,     2,  NCSMIB_FMAT_INT},


   /* SU Instance Table */
   {"SUInstance",        "name",
      NCSMIB_TBL_AVSV_AMF_SU,     1, NCSMIB_FMAT_OCT},
   {"SUInstance",        "suRank",
      NCSMIB_TBL_AVSV_AMF_SU,     2, NCSMIB_FMAT_INT},
   {"SUInstance",        "numComponents",
      NCSMIB_TBL_AVSV_AMF_SU,     3, NCSMIB_FMAT_INT},
   {"SUInstance",        "adminstate",
      NCSMIB_TBL_AVSV_AMF_SU,     6, NCSMIB_FMAT_INT},
   {"SUInstance",        "SUFailover",
      NCSMIB_TBL_AVSV_AMF_SU,     7, NCSMIB_FMAT_BOOL},
   {"SUInstance",        "parentSGName",
      NCSMIB_TBL_AVSV_AMF_SU,     12, NCSMIB_FMAT_OCT},
   {"SUInstance",        "rowStatus",
      NCSMIB_TBL_AVSV_AMF_SU,     14, NCSMIB_FMAT_INT},

      /* SI Instance Table */
   {"SIInstance",        "name",
      NCSMIB_TBL_AVSV_AMF_SI,     1, NCSMIB_FMAT_OCT},
   {"SIInstance",        "rank",
      NCSMIB_TBL_AVSV_AMF_SI,     2, NCSMIB_FMAT_INT},
   {"SIInstance",        "numCSIs",
      NCSMIB_TBL_AVSV_AMF_SI,     3, NCSMIB_FMAT_INT},
   {"SIInstance",        "preferredNumOfAssignments",
      NCSMIB_TBL_AVSV_AMF_SI,     4, NCSMIB_FMAT_INT},
   {"SIInstance",        "parentSGName",  
      NCSMIB_TBL_AVSV_AMF_SI,     9, NCSMIB_FMAT_OCT},
   {"SIInstance",        "rowStatus",
      NCSMIB_TBL_AVSV_AMF_SI,     10, NCSMIB_FMAT_INT},
   /* Watch out that this is new table in MIBS 
    * but comes in SIInstance */
   {"SIInstance",        "SUName",
      NCSMIB_TBL_AVSV_AMF_SUS_PER_SI_RANK,     3, NCSMIB_FMAT_OCT},
   {"SIInstance",        "SUsPerSIrowStatus",
      NCSMIB_TBL_AVSV_AMF_SUS_PER_SI_RANK,     4, NCSMIB_FMAT_INT},

   /* SI-SI Dependency Table */
   {"SIInstance",        "SIdepName",  
      NCSMIB_TBL_AVSV_AMF_SI_SI_DEP, 1, NCSMIB_FMAT_OCT},
   {"SIInstance",        "SIdepDepName",
      NCSMIB_TBL_AVSV_AMF_SI_SI_DEP, 2, NCSMIB_FMAT_OCT},
   {"SIInstance",        "SIdepToltime",
      NCSMIB_TBL_AVSV_AMF_SI_SI_DEP, 3, NCSMIB_FMAT_OCT},
   {"SIInstance",        "SIdepRowStatus",
      NCSMIB_TBL_AVSV_AMF_SI_SI_DEP, 4, NCSMIB_FMAT_INT},

   /* component Instance table with tags */
   {"componentInstance",  "name",                
      NCSMIB_TBL_AVSV_AMF_COMP, 1, NCSMIB_FMAT_OCT},
   {"componentInstance",  "capability",          
      NCSMIB_TBL_AVSV_AMF_COMP, 2, NCSMIB_FMAT_INT},
   {"componentInstance",  "compProperty",    
      NCSMIB_TBL_AVSV_AMF_COMP, 3, NCSMIB_FMAT_INT},
   {"componentInstance",  "instantiateCommand",  
      NCSMIB_TBL_AVSV_AMF_COMP, 4, NCSMIB_FMAT_OCT},
   {"componentInstance",  "terminateCommand",    
      NCSMIB_TBL_AVSV_AMF_COMP, 5, NCSMIB_FMAT_OCT},
   {"componentInstance",  "cleanupCommand",      
      NCSMIB_TBL_AVSV_AMF_COMP, 6, NCSMIB_FMAT_OCT},
   {"componentInstance",  "amStartCommand",      
      NCSMIB_TBL_AVSV_AMF_COMP, 7, NCSMIB_FMAT_OCT},
   {"componentInstance",  "amStopCommand",      
      NCSMIB_TBL_AVSV_AMF_COMP, 8, NCSMIB_FMAT_OCT},
   {"componentInstance",  "instantiationLevel",      
      NCSMIB_TBL_AVSV_AMF_COMP, 9, NCSMIB_FMAT_INT},
   {"componentInstance",  "instantiatetimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 10, NCSMIB_FMAT_OCT},
   {"componentInstance",  "registrationTimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 10, NCSMIB_FMAT_OCT},
   {"componentInstance",  "delaybetweenInstantiateAttempts",      
      NCSMIB_TBL_AVSV_AMF_COMP, 11, NCSMIB_FMAT_INT},
   {"componentInstance",  "terminatetimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 12, NCSMIB_FMAT_OCT},
   {"componentInstance",  "cleanuptimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 13, NCSMIB_FMAT_OCT},
   {"componentInstance",  "amStarttimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 14, NCSMIB_FMAT_OCT},
   {"componentInstance",  "amStoptimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 15, NCSMIB_FMAT_OCT},
   {"componentInstance",  "terminationTimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 16, NCSMIB_FMAT_OCT},
   {"componentInstance",  "csiSetCallbackTimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 17, NCSMIB_FMAT_OCT},
   {"componentInstance",  "QuiescingDoneTimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 18, NCSMIB_FMAT_OCT},
   {"componentInstance",  "csiRmvCallbackTimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 19, NCSMIB_FMAT_OCT},
   {"componentInstance",  "proxiedCompInstCbTimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 20, NCSMIB_FMAT_OCT},
   {"componentInstance",  "proxiedCompCleanCbTimeout",      
      NCSMIB_TBL_AVSV_AMF_COMP, 21, NCSMIB_FMAT_OCT},
   {"componentInstance",  "nodeRebootCleanupFail",      
      NCSMIB_TBL_AVSV_AMF_COMP, 22, NCSMIB_FMAT_BOOL},
   {"componentInstance",  "recoveryOnTimeout",   
      NCSMIB_TBL_AVSV_AMF_COMP, 23, NCSMIB_FMAT_INT},
   {"componentInstance",  "numMaxInstantiate",   
      NCSMIB_TBL_AVSV_AMF_COMP, 24, NCSMIB_FMAT_INT},
   {"componentInstance",  "registrationRetries",   
      NCSMIB_TBL_AVSV_AMF_COMP, 24, NCSMIB_FMAT_INT},
   {"componentInstance",  "numMaxInstwithDelay",   
      NCSMIB_TBL_AVSV_AMF_COMP, 25, NCSMIB_FMAT_INT},
   {"componentInstance",  "maxAmStartRetries",   
      NCSMIB_TBL_AVSV_AMF_COMP, 26, NCSMIB_FMAT_INT},
   {"componentInstance",  "disableRestart",   
      NCSMIB_TBL_AVSV_AMF_COMP, 28, NCSMIB_FMAT_BOOL},
   {"componentInstance",  "numMaxActiveCSIs",   
      NCSMIB_TBL_AVSV_AMF_COMP, 29, NCSMIB_FMAT_INT},
   {"componentInstance",  "numMaxStandbyCSIs",   
      NCSMIB_TBL_AVSV_AMF_COMP, 30, NCSMIB_FMAT_INT},
   {"componentInstance",  "enableAM",      
      NCSMIB_TBL_AVSV_AMF_COMP, 38, NCSMIB_FMAT_BOOL},
   {"componentInstance",  "rowStatus",      
      NCSMIB_TBL_AVSV_AMF_COMP, 39, NCSMIB_FMAT_INT},
   /* Parse from the component instance and populate different table */
   {"componentInstance",  "compCsTypeSuppRowStatus",      
      NCSMIB_TBL_AVSV_AMF_COMP_CS_TYPE, 3, NCSMIB_FMAT_INT},

      /* CSI Instance Table */
   {"CSIInstance",        "name",
      NCSMIB_TBL_AVSV_AMF_CSI,     1, NCSMIB_FMAT_OCT},
   {"CSIInstance",        "csiType",
      NCSMIB_TBL_AVSV_AMF_CSI,     2, NCSMIB_FMAT_OCT},
   {"CSIInstance",        "rank",  
      NCSMIB_TBL_AVSV_AMF_CSI,     3, NCSMIB_FMAT_INT},
   {"CSIInstance",        "rowStatus",
      NCSMIB_TBL_AVSV_AMF_CSI,     4, NCSMIB_FMAT_INT},
   {"CSIInstance",        "NameValueParamVal",
      NCSMIB_TBL_AVSV_AMF_CSI_PARAM,     3, NCSMIB_FMAT_OCT},
   {"CSIInstance",        "NameValueRowStatus",
      NCSMIB_TBL_AVSV_AMF_CSI_PARAM,     4, NCSMIB_FMAT_INT},
   {"CSIInstance",        "CSTypeParamRowStatus",
      NCSMIB_TBL_AVSV_AMF_CS_TYPE_PARAM,     3, NCSMIB_FMAT_INT},

      /* NCS SCALAR TABLE */
   {"NCSScalar",        "sndHbInt",
      NCSMIB_TBL_AVSV_NCS_SCALAR,     1, NCSMIB_FMAT_OCT},
   {"NCSScalar",        "rcvHbInt",
      NCSMIB_TBL_AVSV_NCS_SCALAR,     2, NCSMIB_FMAT_OCT},
   {"NCSScalar",        "clusterAdminState",
      NCSMIB_TBL_AVSV_NCS_SCALAR,     3, NCSMIB_FMAT_OCT},
   {"NCSScalar",        "clusterInitTime",
      NCSMIB_TBL_AVSV_AMF_SCALAR,     6, NCSMIB_FMAT_OCT}
};	

TAG_OID_MAP_NODE gl_amfHealth_check[] =
{
   /* health check table */
   {"healthCheck",      	"key",
      NCSMIB_TBL_AVSV_AMF_HLT_CHK,   2, NCSMIB_FMAT_OCT},
   {"healthCheck",      	"period",
      NCSMIB_TBL_AVSV_AMF_HLT_CHK,   3, NCSMIB_FMAT_OCT},
   {"healthCheck",      	"maxDuration",
      NCSMIB_TBL_AVSV_AMF_HLT_CHK,   4, NCSMIB_FMAT_OCT},
   {"healthCheck",      	"rowStatus",
      NCSMIB_TBL_AVSV_AMF_HLT_CHK,   5, NCSMIB_FMAT_INT}
};


TAG_OID_MAP_NODE gl_amfCSINameValue[] =
{
   /*
   ** AMF CSI Name-Value Table Objects - the name/value pairs for
   ** CSIs are configured through this table.
   */

   {"CSINameValue",      	"csiName",
      NCSMIB_TBL_AVSV_AMF_CSI_PARAM,   1, NCSMIB_FMAT_OCT},
   {"CSINameValue",      	"name",
      NCSMIB_TBL_AVSV_AMF_CSI_PARAM,   2, NCSMIB_FMAT_OCT},
   {"CSINameValue",      	"value",
      NCSMIB_TBL_AVSV_AMF_CSI_PARAM,   3, NCSMIB_FMAT_OCT},
   {"CSINameValue",      	"rowStatus",
      NCSMIB_TBL_AVSV_AMF_CSI_PARAM,   4, NCSMIB_FMAT_INT}
};

TAG_OID_MAP_NODE gl_hwDeploymentEnt[] =
{
   /* HW Deployment Table */
   {"hwDeployment",      	"EntPath",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   1, NCSMIB_FMAT_OCT},
   {"hwDeployment",      	"IsNode",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   2, NCSMIB_FMAT_INT},
   {"hwDeployment",      	"NodeName",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   3, NCSMIB_FMAT_OCT},
   {"hwDeployment",      	"EntName",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   4, NCSMIB_FMAT_OCT},
   {"hwDeployment",      	"ParentEntName",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   5, NCSMIB_FMAT_OCT},
   {"hwDeployment",      	"DependsOn",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   9, NCSMIB_FMAT_OCT},
   {"hwDeployment",      	"isActivationSourceNCS",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   12, NCSMIB_FMAT_INT},
   {"hwDeployment",             "netBoot",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   14, NCSMIB_FMAT_BOOL},
   {"hwDeployment",             "tftpServIP",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   15, NCSMIB_FMAT_OCT},
   {"hwDeployment",             "label1Name",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   17, NCSMIB_FMAT_OCT},
   {"hwDeployment",             "label1FileName",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   18, NCSMIB_FMAT_OCT}, 
   {"hwDeployment",             "label2Name",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   22, NCSMIB_FMAT_OCT},
   {"hwDeployment",             "label2FileName",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   23, NCSMIB_FMAT_OCT},
   {"hwDeployment",             "preferredLabel",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   27, NCSMIB_FMAT_OCT},
   {"hwDeployment",      	"rowStatus",
      NCSMIB_TBL_AVM_ENT_DEPLOYMENT,   30, NCSMIB_FMAT_INT}
};


const uns32 gl_amfConfig_table_size = sizeof(gl_amfConfig_table) / sizeof(TAG_OID_MAP_NODE );
const uns32 gl_amfHealth_check_size = sizeof(gl_amfHealth_check) / sizeof(TAG_OID_MAP_NODE );
const uns32 gl_amfCSINameValue_size= sizeof(gl_amfCSINameValue) / sizeof(TAG_OID_MAP_NODE );
const uns32 gl_hwDeploymentEnt_size = sizeof(gl_hwDeploymentEnt) / sizeof(TAG_OID_MAP_NODE );

/*****************************************************************************
  PROCEDURE NAME: bam_parse_hardware_config

  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : 
*****************************************************************************/
uns32 bam_parse_hardware_config(void)
{
   NCS_BAM_CB   *bam_cb = NULL;

   if((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   parse_and_build_hw_validation(bam_cb->hw_filename); 

   bam_avm_send_validation_config();

   ncshm_give_hdl(gl_ncs_bam_hdl);   

   return NCSCC_RC_SUCCESS;
}

static uns32
parse_ais_config(DOMNode *aisNode, BAM_PARSE_SUB_TREE sub_tree)
{
   DOMNodeList *children;
   uns32 rc=0;

   children = aisNode->getChildNodes();
   
   for(unsigned int x=0; x < children->getLength();x++)
   {
      DOMNode *tmpNode = children->item(x);
      if(tmpNode)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(strcmp(tmpString, "AMFConfig") == 0)
         {
            rc = saAmfConfigParseAllObjs(tmpNode, sub_tree);
         }

         XMLString::release(&tmpString);
      }
   }
   return NCSCC_RC_SUCCESS;
}



static uns32
parse_ncs_config(DOMNode *ncsNode, BAM_PARSE_SUB_TREE sub_tree)
{
   DOMNodeList *children;
   uns32    rc=0;

   children = ncsNode->getChildNodes();
   
   for(unsigned int x=0; x < children->getLength();x++)
   {
      DOMNode *tmpNode = children->item(x);
      if(tmpNode)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(strcmp(tmpString, "AISConfig") == 0)
            rc = parse_ais_config(tmpNode, sub_tree);

          XMLString::release(&tmpString);
      }
   }

   if((NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }
   
   ncshm_give_hdl(gl_ncs_bam_hdl);   
   return NCSCC_RC_SUCCESS;
}


static uns32
parse_system_bom(DOMDocument *doc, BAM_PARSE_SUB_TREE sub_tree)
{
   XMLCh *str = XMLString::transcode("SystemConfig");
   DOMNodeList *elements = doc->getElementsByTagName( str);
   DOMNodeList *children;
   uns32 rc =0;

   XMLString::release(&str);
   if(elements->getLength() != 1)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR,
                       "There should be exactly only one element");
      return NCSCC_RC_FAILURE;
   }
   DOMNode *systemconfig = elements->item(0);
   children = systemconfig->getChildNodes();
   
   for(unsigned int i=0;i < children->getLength();i++)
   {
      DOMNode *tmpNode = children->item(i);
      if(tmpNode)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if( (strcmp(tmpString, "NCSConfig") == 0) && 
             ( (sub_tree == BAM_PARSE_NCS)  || (sub_tree == BAM_PARSE_APP) )
           )
         {
            rc = parse_ncs_config(tmpNode, sub_tree);
         }
         else if((strcmp(tmpString, "Hardware-Deployment-Config") == 0) &&
                  (sub_tree == BAM_PARSE_AVM) )
         {
            rc = parse_hw_deploy_config(tmpNode);
         }

         XMLString::release(&tmpString);
         if(rc)
            return rc;
      }
   }
   return NCSCC_RC_SUCCESS;
}

static uns32
parse_app_config(DOMDocument *doc, BAM_PARSE_SUB_TREE sub_tree)
{
   XMLCh *str = XMLString::transcode("APPConfig");
   DOMNodeList *elements = doc->getElementsByTagName( str);
   DOMNodeList *children;
   uns32 rc =0;

   XMLString::release(&str);
   if(elements->getLength() != 1)
   {
      m_LOG_BAM_MSG_TIC(BAM_INVALID_DOMNODE, NCSFL_SEV_ERROR,
                       "There should be exactly only one element");
      return NCSCC_RC_FAILURE;
   }
   DOMNode *appConfig = elements->item(0);
   children = appConfig->getChildNodes();

   for(unsigned int i=0;i < children->getLength();i++)
   {
      DOMNode *tmpNode = children->item(i);
      if(tmpNode)
      {
         char *tmpString = XMLString::transcode(tmpNode->getNodeName());

         if(strcmp(tmpString, "AMFConfig") == 0)
           
         {
            rc = saAmfConfigParseAllObjs(tmpNode, sub_tree);
         }
         XMLString::release(&tmpString);
         if(rc)
            return rc;
      }
   }
   return NCSCC_RC_SUCCESS;
}
/*****************************************************************************
  PROCEDURE NAME: parse_and_build_DOMDocument
  DESCRIPTION   : 
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES         : We use DOMBuilder as opposed to XercesDOMParser to get 
                  the handle of the built domDocument.
*****************************************************************************/

uns32
parse_and_build_DOMDocument(char *xmlFile, BAM_PARSE_SUB_TREE sub_tree)
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
       syslog(LOG_ERR, "NCS_AvSv: Error!: %s  during XML Parser initialization for! : %s file ", str, xmlFile);
       XMLString::release(&str);
       return NCSCC_RC_FAILURE;
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

    XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc = 0;

    try
    {
       /* reset document pool */
       parser->resetDocumentPool();

       doc = parser->parseURI(xmlFile);
    }

    catch (const XMLException& toCatch)
    {
       syslog(LOG_ERR, "NCS_AvSv: Error during parsing! : %s", xmlFile);
       char *str = XMLString::transcode(toCatch.getMessage()); 
       syslog(LOG_ERR, "NCS_AvSv: Exception message is! : %s", str);
       XMLString::release(&str);
       errorOccurred = true;
    }
    catch (const DOMException& toCatch)
    {
       const unsigned int maxChars = 2047;
       XMLCh errText[maxChars + 1];

       syslog(LOG_ERR, "NCS_AvSv: Error during parsing! : %s", xmlFile);
       syslog(LOG_ERR, "NCS_AvSv: Exception code is! : %s", toCatch.code);

       if (DOMImplementation::loadDOMExceptionMsg(toCatch.code, errText, maxChars))
       {
          char *str = XMLString::transcode(errText);
          syslog(LOG_ERR, "NCS_AvSv: Message is! : %s", str);
          XMLString::release(&str);
          errorOccurred = true;
       }
    }
    catch (...)
    {
       syslog(LOG_ERR, "NCS_AvSv: Unexpected exception during parsing! : %s", xmlFile);
       errorOccurred = true;
    }

    /*
    **  Extract the DOM tree, get the list of all the elements and report the
    **  length as the count of elements.
    */
    if (errorHandler.getSawErrors())
    {
       syslog(LOG_ERR, "NCS_AvSv: Errors occurred in DOM tree during parsing xml file: %s, no output available", xmlFile);
       errorOccurred = true;
    }
    else
    {
       if (doc)
       {
          gl_dom_doc = doc;
          if(sub_tree == BAM_PARSE_APP)
          {
             if(parse_app_config(doc, sub_tree) != NCSCC_RC_SUCCESS)
             {
                m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
                   "There was problem in parsing the sub_tree"); 
             }
          }
          else 
          {
             if(parse_system_bom(doc, sub_tree) != NCSCC_RC_SUCCESS)
             {
                m_LOG_BAM_MSG_TIC(BAM_PARSE_ERROR, NCSFL_SEV_ERROR, 
                   "There was problem in parsing the sub_tree"); 
             }
          }
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
        return NCSCC_RC_FAILURE;
    else
        return NCSCC_RC_SUCCESS;
}

