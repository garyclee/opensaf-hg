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

#include <string>
#include <list>
#include <map>
#include <cstring>
#include <saImmOm.h>
#include <immsv_api.h>
#include <logtrace.h>

/* Prototypes */
typedef std::map<std::string, SaImmAttrFlagsT> AttrMap;
struct ClassInfo
{
	ClassInfo(unsigned int class_id) : mClassId(class_id) { }
	~ClassInfo() { }
    
	unsigned int mClassId;
	AttrMap mAttrMap;
};
typedef std::map<std::string, ClassInfo*> ClassMap;

std::list<std::string> getClassNames(SaImmHandleT handle);
std::string getClassName(SaImmAttrValuesT_2** attrs);
std::string valueToString(SaImmAttrValueT, SaImmValueTypeT);
void* pbeRepositoryInit(const char* filePath);
void pbeRepositoryClose(void* dbHandle);
void dumpClassesToPbe(SaImmHandleT immHandle, ClassMap *classIdMap,
	void* db_handle);
void dumpObjectsToPbe(SaImmHandleT immHandle, ClassMap* classIdMap,
	void* db_handle);
