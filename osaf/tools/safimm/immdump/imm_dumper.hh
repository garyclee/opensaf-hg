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
 * Author(s): Ericsson AB
 *
 */


#include <libxml/xmlwriter.h>

#include "immpbe_dump.hh"


/* XML-Writer related functions */

void dumpClassesXMLw(SaImmHandleT, xmlTextWriterPtr,
                     std::list<std::string>&);
void classToXMLw(std::string, SaImmHandleT, xmlTextWriterPtr);
void objectToXMLw(std::string, 
                        SaImmAttrValuesT_2**, 
                        SaImmHandleT, 
                        std::map<std::string, std::string>,
                     	xmlTextWriterPtr);
void valuesToXMLw(SaImmAttrValuesT_2* p, xmlTextWriterPtr );
void dumpObjectsXMLw(SaImmHandleT, 
                        std::map<std::string, std::string>,
                        xmlTextWriterPtr);
void dumpObjectsXMLw(SaImmHandleT,
                        std::map<std::string, std::string>,
                        xmlTextWriterPtr,
                        std::list<std::string>&);
void flagsToXMLw(SaImmAttrDefinitionT_2*, xmlTextWriterPtr);
void typeToXMLw(SaImmAttrDefinitionT_2*, xmlTextWriterPtr);
