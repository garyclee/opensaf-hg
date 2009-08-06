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

  This header file includes the temporary structures to store the parsed 
  information before generating the mibsets.
  
******************************************************************************
*/

#include "ncs_lib.h"
#include "ncsdlib.h"

#ifndef BAMPARSER_H
#define BAMPARSER_H

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>


#define NCS_BAM_MIB_REQ_TIMEOUT  60
#define NCS_BAM_BUFFER_LEN   200
#define BAM_MAX_INDEX_LEN 128
#define NCS_DEFAULT_COMP_TYPE 1


XERCES_CPP_NAMESPACE_USE

EXTERN_C DOMDocument *gl_dom_doc;

EXTERN_C DOMNode *
get_dom_node_by_name(char *, char* );

EXTERN_C SaAisErrorT
saAmfConfigParseAllObjs(DOMNode *, BAM_PARSE_SUB_TREE);

EXTERN_C TAG_OID_MAP_NODE gl_amfConfig_table[];
EXTERN_C const uns32 gl_amfConfig_table_size;

EXTERN_C TAG_OID_MAP_NODE gl_amfHealth_check[];
EXTERN_C const uns32 gl_amfHealth_check_size;

EXTERN_C TAG_OID_MAP_NODE gl_hwDeploymentEnt[];
EXTERN_C const uns32 gl_hwDeploymentEnt_size;

EXTERN_C SaAisErrorT
saAmfParseSGInstance(DOMNode *);

EXTERN_C SaAisErrorT
saAmfParseCSIPrototype(char *, char*);

EXTERN_C uns32
parse_process_config(DOMNode *);

EXTERN_C SaAisErrorT
ncs_bam_parse_libproto_list(DOMNode *, char *, char *);

EXTERN_C SaAisErrorT
ncs_bam_parse_lib_desc_list(DOMNode *, char *, char *);

EXTERN_C SaAisErrorT
ncs_bam_parse_lib_instance(DOMNode *);

EXTERN_C SaAisErrorT
ncs_bam_parse_ext_lib_proto(DOMNode *);

EXTERN_C SaAisErrorT
parse_hw_deploy_config(DOMNode *);

EXTERN_C uns32 
parse_and_build_hw_validation(char *);

EXTERN_C uns32 
bam_parse_hardware_config(void);

EXTERN_C uns32 
ncs_bam_map_csi_comp_add(char *, char *, uns32 );

EXTERN_C uns32 
ncs_bam_csi_comp_find(char *);

EXTERN_C uns32 
ncs_bam_csi_name_db_add(char *, char *);
/*
 * This structure is for the temporary storage for all the CLC command scripts.
 *
 * This structure is common for all the instantiation and termination commands.
 * The elements in this structure all from the XML element type scriptCommandT.
 */

typedef struct safScriptCommand
{
   char  *commandString; /* the CLC command string including path 
                            and command parameters */
   uns32 timeout;       /* timeout value for the command */
   uns32 retries;       /* Maximum number of retries */
}SAF_SCRIPT_COMMAND;

typedef struct csi_comp_type_map
{
   NCS_DB_LINK_LIST_NODE      node;   /* Node information */
   uns8                       compType;
   char                       csiName[BAM_MAX_INDEX_LEN];
   char                       compName[BAM_MAX_INDEX_LEN];
}CSI_COMP_TYPE_MAP;

#endif
