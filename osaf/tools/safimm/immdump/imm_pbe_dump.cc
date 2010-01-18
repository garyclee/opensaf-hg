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

#include <imm_dumper.hh>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>

#ifdef IMM_PBE

#include <sqlite3.h> 

static void typeToPBE(SaImmAttrDefinitionT_2* p, 
	void* dbHandle, std::string * table_def)
{
	TRACE_ENTER();
	switch (p->attrValueType)
	{
		case SA_IMM_ATTR_SAINT32T:
		case SA_IMM_ATTR_SAUINT32T:
		case SA_IMM_ATTR_SAINT64T:
		case SA_IMM_ATTR_SAUINT64T:
		case SA_IMM_ATTR_SATIMET:
			table_def->append(" integer");
			break;

		case SA_IMM_ATTR_SAFLOATT:
		case SA_IMM_ATTR_SADOUBLET:
			table_def->append(" real");
			break;

		case SA_IMM_ATTR_SANAMET:
		case SA_IMM_ATTR_SASTRINGT:
		case SA_IMM_ATTR_SAANYT:
			table_def->append(" text");
			break;

		default:
			TRACE_4("Unknown value type %u", p->attrValueType);
			exit(1);
	}
	TRACE_LEAVE();
}

static void valuesToPBE(SaImmAttrValuesT_2* p, SaImmAttrFlagsT attrFlags, 
	const char *objIdStr, void* db_handle)
{
	/* Handle the multivalued case. */	
	char *execErr=NULL;
	int rc=0;
	std::string sqlG("INSERT INTO ");
	sqlite3* dbHandle = (sqlite3 *) db_handle;
	TRACE_ENTER();
	assert(p->attrValues);
	assert(p->attrValuesNumber);
	assert(attrFlags & SA_IMM_ATTR_MULTI_VALUE);

	switch(p->attrValueType) {
		case SA_IMM_ATTR_SAINT32T: 
		case SA_IMM_ATTR_SAUINT32T: 
		case SA_IMM_ATTR_SAINT64T:  
		case SA_IMM_ATTR_SAUINT64T: 
		case SA_IMM_ATTR_SATIMET:   
			sqlG.append("objects_int_multi (obj_id, attr_name, int_val) values('");
			break;

		case SA_IMM_ATTR_SAFLOATT:  
		case SA_IMM_ATTR_SADOUBLET: 
			sqlG.append("objects_real_multi (obj_id, attr_name, real_val) values('");
			break;

		case SA_IMM_ATTR_SANAMET:
		case SA_IMM_ATTR_SASTRINGT:
		case SA_IMM_ATTR_SAANYT:
			sqlG.append("objects_text_multi (obj_id, attr_name, text_val) values('");
			break;

		default:
			LOG_ER("Unknown value type %u", p->attrValueType);
			assert(0);
	}
	sqlG.append(objIdStr);
	sqlG.append("', '");
	sqlG.append(p->attrName);
	sqlG.append("', '");
	for (unsigned int i = 0; i < p->attrValuesNumber; i++)
	{
		std::string sqlG1(sqlG);
		sqlG1.append(valueToString(p->attrValues[i], p->attrValueType));
		sqlG1.append("')");
		TRACE("GENERATED G:%s", sqlG1.c_str());
		rc = sqlite3_exec(dbHandle, sqlG1.c_str(), NULL, NULL, &execErr);
		if(rc != SQLITE_OK) {
			LOG_ER("SQL statement ('%s') failed because:\n %s", sqlG1.c_str(),
				execErr);
			sqlite3_free(execErr);
			exit(1);
		}
	}
	TRACE_LEAVE();
}

void* pbeRepositoryInit(const char* filePath)
{
	int fd=(-1);
	sqlite3* dbHandle=NULL;
	std::string newFilename;
	int rc=0;
	char *execErr=NULL;

	const char * sql_tr[] = 
		{"BEGIN EXCLUSIVE TRANSACTION",
		 "CREATE TABLE classes (class_id integer primary key, class_category integer, class_name text)",
		 "CREATE UNIQUE INDEX classes_idx on classes (class_name)",

		 "CREATE TABLE attr_def(class_id integer, attr_type integer, attr_flags integer, attr_name text)",
		 "CREATE UNIQUE INDEX attr_def_idx on attr_def (class_id, attr_name)",

		 "CREATE TABLE attr_dflt(attr_name text, class_id integer, int_dflt integer, text_dflt text, "
		 "real_dflt real, blob_dflt blob)",
		 "CREATE UNIQUE INDEX attr_dflt_idx on attr_dflt (class_id, attr_name)",

		 "CREATE TABLE objects (obj_id integer primary key, class_id integer, dn text)",
		 "CREATE UNIQUE INDEX objects_idx on objects (dn)",

		 "CREATE TABLE objects_int_multi (obj_id integer, attr_name text, int_val integer)",
		 "CREATE INDEX objects_int_multi_idx on objects_int_multi (obj_id, attr_name)",

		 "CREATE TABLE objects_text_multi (obj_id integer, attr_name text, text_val text)",
		 "CREATE INDEX objects_text_multi_idx on objects_text_multi (obj_id, attr_name)",

		 "CREATE TABLE objects_real_multi (obj_id integer, attr_name text, real_val real)",
		 "CREATE INDEX objects_real_multi_idx on objects_real_multi (obj_id, attr_name)",

		 "CREATE TABLE objects_blob_multi (obj_id integer, attr_name text, blob_val blob)",
		 "CREATE INDEX objects_blob_multi_idx on objects_blob_multi (obj_id, attr_name)",
		 "COMMIT TRANSACTION",
		 NULL
		};
	/* First, check if db-file already exists and if so mv it to a backup copy. */

	fd = open(filePath, O_RDWR);
	if(fd != (-1)) {
		close(fd);
		fd=(-1);
		newFilename.append(filePath);
		newFilename.append(".prev"); 
		/* Could mark with date, but that requires rotation. */

		if(rename(filePath, newFilename.c_str())!=0) {
			LOG_ER("Failed to rename %s to %s error:%s", 
				filePath, newFilename.c_str(), strerror(errno));
			exit(1);
		} else {
			LOG_NO("Renamed %s to %s.", filePath, newFilename.c_str());
		}
	}

	rc = sqlite3_open(filePath, &dbHandle);
	if(rc) {
		LOG_ER("Can't open sqlite pbe file '%s', cause:%s", 
			filePath, sqlite3_errmsg(dbHandle));
		exit(1);
	} 
	TRACE_2("Successfully opened sqlite pbe file %s", filePath);
		 

	/* Creating the schema. */
	for (int ix=0; sql_tr[ix]!=NULL; ++ix) {
		rc = sqlite3_exec(dbHandle, sql_tr[ix], NULL, NULL, &execErr);
		if (rc != SQLITE_OK) {
			LOG_ER("SQL statement %u/('%s') failed because:\n %s", ix, sql_tr[ix], execErr);
			sqlite3_free(execErr);
			goto bailout;
		}
		TRACE_2("Successfully executed %s", sql_tr[ix]);
	}

	return (void *) dbHandle;

 bailout:
	/* TODO: remove imm.db file */
	sqlite3_close(dbHandle);
	return NULL;
}

void pbeRepositoryClose(void* dbHandle)
{
	sqlite3_close((sqlite3 *) dbHandle);
}

static ClassInfo* classToPBE(std::string classNameString,
	SaImmHandleT immHandle,
	void* db_handle,
	unsigned int class_id)
{
	SaImmClassCategoryT classCategory;
	SaImmAttrDefinitionT_2 **attrDefinitions;
	SaAisErrorT errorCode;
	bool class_is_pure_runtime=true; /* true => no persistent rtattrs. */
	unsigned int strint_bsz=16;
	char classIdStr[strint_bsz];
	char attrPropStr[strint_bsz];
	int rc=0;
	char *execErr=NULL;
	sqlite3* dbHandle = (sqlite3 *) db_handle;
	ClassInfo* classInfo = new ClassInfo(class_id);
	TRACE_ENTER();

	std::string sqlA("INSERT INTO classes (class_id, class_name, class_category) values('");
	std::string sqlB;
	unsigned int sqlBsize = 2048;  /* There appears to be a bug in the version of 
					  std:string I am using that is provoked (segv)
					  when the string expands beyond aproximately
					  1K in size.
					  Preallocating large space to avoid this. 
					  This is also more efficient.
				       */
	std::string sqlC("INSERT INTO attr_def (class_id, attr_name, attr_type, attr_flags) values('");

	std::string sqlD("INSERT INTO attr_dflt (class_id, attr_name, ");

	/* Get the class description */
	errorCode = saImmOmClassDescriptionGet_2(immHandle,
		(char*)classNameString.c_str(),
		&classCategory,
		&attrDefinitions);

	if (SA_AIS_OK != errorCode)
	{
		TRACE_4("Failed to get the description for the %s class error:%u - exiting",
			classNameString.c_str(), errorCode);
		exit(1);
	}

	snprintf(classIdStr, strint_bsz, "%u", class_id);
	sqlA.append(classIdStr);
	sqlA.append("', '");
	sqlA.append(classNameString);
	sqlA.append("', '");
	if(classCategory == SA_IMM_CLASS_CONFIG) {
		sqlA.append("1');");
		class_is_pure_runtime=false; /* true => no persistent rtattrs. */
	} else if (classCategory == SA_IMM_CLASS_RUNTIME) {
		TRACE("ABT RUNTIME CLASS %s", (char*)classNameString.c_str());
		sqlA.append("2');");
	} else {
		LOG_ER("Class category %u for class %s is neither CONFIG nor RUNTIME",
			classCategory, classNameString.c_str());
		goto bailout;
	}

	TRACE("GENERATED A class tuple: (%s)", sqlA.c_str());

	rc = sqlite3_exec(dbHandle, sqlA.c_str(), NULL, NULL, &execErr);
	if(rc != SQLITE_OK) {
		LOG_ER("SQL statement ('%s') failed because:\n %s", sqlA.c_str(),
			execErr);
		sqlite3_free(execErr);
		goto bailout;
	}
	sqlA.resize(0); /* sqlA not used any more. */
	sqlB.reserve(sqlBsize);
	sqlB.append("CREATE TABLE ");
	sqlB.append(classNameString);
	sqlB.append("(obj_id integer primary key");
	sqlC.append(classIdStr);
	sqlC.append("', '");

	/* Attributes needed both in class def tuple and in instance relations. */
	for (SaImmAttrDefinitionT_2** p = attrDefinitions; *p != NULL; p++)
	{
		bool attr_is_pure_rt=false; /* This attribute is pure runtime?*/
		bool attr_is_multi=false;
		std::string sqlC2(sqlC);

		classInfo->mAttrMap[(*p)->attrName] = (*p)->attrFlags;

		if ((*p)->attrFlags & SA_IMM_ATTR_RUNTIME) {
			if((*p)->attrFlags & SA_IMM_ATTR_PERSISTENT) {
				TRACE_2("Persistent runtime attribute %s", (*p)->attrName);
				class_is_pure_runtime=false;
			} else {
				TRACE_2("PURE runtime attribute %s", (*p)->attrName);
				attr_is_pure_rt=true;
			}
		}

		if ((*p)->attrFlags & SA_IMM_ATTR_MULTI_VALUE) {
			TRACE_2("Multivalued attribute %s", (*p)->attrName);
			attr_is_multi=true;
		}

		if(!(attr_is_pure_rt || attr_is_multi)) {
			sqlB.append(", ");
			sqlB.append((*p)->attrName);
			typeToPBE(*p, dbHandle, &sqlB);
		}

		sqlC2.append((*p)->attrName);
		sqlC2.append("', '");

		snprintf(attrPropStr, strint_bsz, "%u", (*p)->attrValueType);
		sqlC2.append(attrPropStr);
		sqlC2.append("', '");
		snprintf(attrPropStr, strint_bsz, "%u", (unsigned int) (*p)->attrFlags);
		sqlC2.append(attrPropStr);		
		sqlC2.append("' )");
		TRACE("GENERATED C2: (%s)", sqlC2.c_str());
		rc = sqlite3_exec(dbHandle, sqlC2.c_str(), NULL, NULL, &execErr);
		if(rc != SQLITE_OK) {
			LOG_ER("SQL statement ('%s') failed because:\n %s",
				sqlC2.c_str(), execErr);
			sqlite3_free(execErr);
			goto bailout;
		}

		if ((*p)->attrDefaultValue != NULL)
		{
			std::string sqlD2(sqlD);
			switch ((*p)->attrValueType) {
				case SA_IMM_ATTR_SAINT32T:
				case SA_IMM_ATTR_SAUINT32T:
				case SA_IMM_ATTR_SAINT64T: 
				case SA_IMM_ATTR_SAUINT64T:
				case SA_IMM_ATTR_SATIMET:
					sqlD2.append(" int_dflt) values ('");
					break;

				case SA_IMM_ATTR_SAFLOATT:  
				case SA_IMM_ATTR_SADOUBLET: 
					sqlD2.append(" real_dflt) values ('");
					break;

				case SA_IMM_ATTR_SANAMET:
				case SA_IMM_ATTR_SASTRINGT:
				case SA_IMM_ATTR_SAANYT:
					sqlD2.append(" text_dflt) values ('");
					break;

				default:
					LOG_ER("Invalid attribte type %u", 
						(*p)->attrValueType);
					goto bailout;
			}

			sqlD2.append(classIdStr);
			sqlD2.append("', '");
			sqlD2.append((*p)->attrName);
			sqlD2.append("', '");
			sqlD2.append(valueToString((*p)->attrDefaultValue, 
					     (*p)->attrValueType));
			sqlD2.append("')");
			TRACE("GENERATED D2: (%s)", sqlD2.c_str());
			rc = sqlite3_exec(dbHandle, sqlD2.c_str(), NULL, NULL, &execErr);
			if(rc != SQLITE_OK) {
				LOG_ER("SQL statement ('%s') failed because:\n %s",
					sqlD2.c_str(), execErr);
				sqlite3_free(execErr);
				goto bailout;
			}
		}

		if (sqlB.size() > sqlBsize) {
			LOG_ER("SQL statement too long:%u max length:%u", 
				sqlB.size(), sqlBsize);
			goto bailout;
		}
	}

	sqlB.append(")");
	if(class_is_pure_runtime) {
		TRACE_2("ABT Class %s is pure runtime, no create of instance table",
			classNameString.c_str());
	} else {
		TRACE("GENERATED B: (%s)", sqlB.c_str());
		rc = sqlite3_exec(dbHandle, sqlB.c_str(), NULL, NULL, &execErr);
		if(rc != SQLITE_OK) {
			LOG_ER("SQL statement ('%s') failed because:\n %s", sqlB.c_str(),
				execErr);
			sqlite3_free(execErr);
			goto bailout;
		}
	}

	errorCode = saImmOmClassDescriptionMemoryFree_2(immHandle, attrDefinitions);
	if (SA_AIS_OK != errorCode)
	{
		TRACE_4("Failed to free the description of class %s error:%u",
			classNameString.c_str(), errorCode);
		goto bailout;
	}
	TRACE_LEAVE();
	return classInfo;

 bailout:
	sqlite3_close((sqlite3 *) dbHandle);
	delete classInfo;
	/* TODO remove imm.db file */
	exit(1);
}

static void objectToPBE(std::string objectNameString,
	SaImmAttrValuesT_2** attrs,
	SaImmHandleT immHandle,
	ClassMap *classIdMap,
	void* db_handle,
	unsigned int object_id)
{
	std::string valueString;
	std::string classNameString;
	unsigned int strint_bsz=16;
	char objIdStr[strint_bsz];
	char classIdStr[strint_bsz];
	unsigned int class_id=0;
	ClassInfo* classInfo=NULL;
	int rc=0;
	char *execErr=NULL;
	sqlite3* dbHandle = (sqlite3 *) db_handle;

	std::string sqlE("INSERT INTO objects (obj_id, class_id, dn) values('");
	std::string sqlF("INSERT INTO ");
	std::string sqlF1(" (obj_id ");
	std::string sqlF2(" values('");

	TRACE_ENTER();
	TRACE_1("Dumping object %s", objectNameString.c_str());
	classNameString = getClassName(attrs);
	classInfo = (*classIdMap)[classNameString];
	if(!classInfo) {
		LOG_ER("Class '%s' not found in classIdMap", 
			classNameString.c_str());
		goto bailout;
	}
	class_id = classInfo->mClassId;
	snprintf(classIdStr, strint_bsz, "%u", class_id);
	snprintf(objIdStr, strint_bsz, "%u", object_id);

	sqlE.append(objIdStr);
	sqlE.append("', '");
	sqlE.append(classIdStr);
	sqlE.append("', '");
	sqlE.append(objectNameString);
	sqlE.append("')");
	TRACE("GENERATED E:%s", sqlE.c_str());
	rc = sqlite3_exec(dbHandle, sqlE.c_str(), NULL, NULL, &execErr);
	if(rc != SQLITE_OK) {
		LOG_ER("SQL statement ('%s') failed because:\n %s", sqlE.c_str(),
			execErr);
		sqlite3_free(execErr);
		goto bailout;
	}

	sqlF.append(classNameString.c_str());
	sqlF2.append(objIdStr);

	for (SaImmAttrValuesT_2** p = attrs; *p != NULL; p++)
	{
		std::string attName((*p)->attrName);
		SaImmAttrFlagsT attrFlags = classInfo->mAttrMap[attName];
		if(attrFlags == 0) {
			LOG_ER("Missing attrInfo %s in classinfo %s", 
				(*p)->attrName, classNameString.c_str());
			goto bailout;
		}

		/* Skip attributes that have no values. */
		if ((*p)->attrValues == NULL || ((*p)->attrValuesNumber == 0))
		{
			continue;
		}

		if(attrFlags & SA_IMM_ATTR_RUNTIME) {
			if(attrFlags & SA_IMM_ATTR_PERSISTENT) {
				TRACE_2("Persistent runtime attribute %s", (*p)->attrName);
			} else {
				TRACE_2("PURE runtime attribute %s - ignoring",
					(*p)->attrName);
				continue;
			}
		}

		if (attrFlags & SA_IMM_ATTR_MULTI_VALUE) {
			TRACE_2("Attr %s is MULTIVALUED", (*p)->attrName);
			valuesToPBE(*p, attrFlags, objIdStr, db_handle);
		} else {
			if((*p)->attrValuesNumber != 1) {
				LOG_ER("attrValuesNumber not 1 :%u",
					(*p)->attrValuesNumber);
				assert((*p)->attrValuesNumber == 1);
			}
			sqlF1.append(", ");
			sqlF1.append(attName);

			sqlF2.append("', '");
			sqlF2.append(valueToString(*((*p)->attrValues),
					     (*p)->attrValueType));
		}
	}

	sqlF1.append(")");
	sqlF2.append("')");
	sqlF.append(sqlF1);
	sqlF.append(sqlF2);

	TRACE("GENERATED F:%s", sqlF.c_str());
	rc = sqlite3_exec(dbHandle, sqlF.c_str(), NULL, NULL, &execErr);
	if(rc != SQLITE_OK) {
		LOG_ER("SQL statement ('%s') failed because:\n %s", sqlF.c_str(),
			execErr);
		sqlite3_free(execErr);
		goto bailout;
	}
	TRACE_LEAVE();
	return;
 bailout:
	sqlite3_close(dbHandle);
	/* TODO remove imm.db file */
	exit(1);
}


void dumpClassesToPbe(SaImmHandleT immHandle, ClassMap *classIdMap,
	void* db_handle)
{
	std::list<std::string> classNameList;
	std::list<std::string>::iterator it;
	int rc=0;
	unsigned int class_id=0;
	char *execErr=NULL;	sqlite3* dbHandle = (sqlite3 *) db_handle;
	TRACE_ENTER();

	classNameList = getClassNames(immHandle);
	it = classNameList.begin();

	sqlite3_exec(dbHandle, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, &execErr);
	if(rc != SQLITE_OK) {
		LOG_ER("SQL statement ('BEGIN EXCLUSIVE TRANSACTION') failed because:\n %s",
			execErr);
		sqlite3_free(execErr);
		goto bailout;
	}

	while (it != classNameList.end())
	{
		(*classIdMap)[(*it)] = 
			classToPBE((*it), immHandle, dbHandle, ++class_id);
		it++;
	}

	sqlite3_exec(dbHandle, "COMMIT TRANSACTION", NULL, NULL, &execErr);
	if(rc != SQLITE_OK) {
		LOG_ER("SQL statement ('COMMIT TRANSACTION') failed because:\n %s",
			execErr);
		sqlite3_free(execErr);
		goto bailout;
	}

	TRACE_LEAVE();
	return;

 bailout:
	sqlite3_close(dbHandle);
	/* TODO remove imm.db file */
	exit(1);	
}

void dumpObjectsToPbe(SaImmHandleT immHandle, ClassMap* classIdMap,
	void* db_handle)
{
	int                    rc=0;
	SaNameT                root;
	SaImmSearchHandleT     searchHandle;
	SaAisErrorT            errorCode;
	SaNameT                objectName;
	SaImmAttrValuesT_2**   attrs;
	unsigned int           retryInterval = 1000000; /* 1 sec */
	unsigned int           maxTries = 15;          /* 15 times == max 15 secs */
	unsigned int           tryCount=0;
	char *execErr=NULL;
	sqlite3* dbHandle = (sqlite3 *) db_handle;
	TRACE_ENTER();
	unsigned int object_id=0;
	root.length = 0;
	strncpy((char*)root.value, "", 3);

	/* Initialize immOmSearch */

	TRACE_1("Before searchInitialize");
	do {
		if(tryCount) {
			usleep(retryInterval);
		}
		++tryCount;

		errorCode = saImmOmSearchInitialize_2(immHandle, 
			&root, 
			SA_IMM_SUBTREE,
			(SaImmSearchOptionsT)
			(SA_IMM_SEARCH_ONE_ATTR | 
				SA_IMM_SEARCH_GET_ALL_ATTR |
				SA_IMM_SEARCH_PERSISTENT_ATTRS),//Special & nonstandard
			NULL/*&params*/, 
			NULL, 
			&searchHandle);

	} while ((errorCode == SA_AIS_ERR_TRY_AGAIN) &&
		(tryCount < maxTries)); /* Can happen if imm is syncing. */

	TRACE_1("After searchInitialize rc:%u", errorCode);
	if (SA_AIS_OK != errorCode)
	{
		LOG_ER("Failed on saImmOmSearchInitialize:%u - exiting ", errorCode);
		goto bailout;
	}

	sqlite3_exec(dbHandle, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, &execErr);
	if(rc != SQLITE_OK) {
		LOG_ER("SQL statement ('BEGIN EXCLUSIVE TRANSACTION') failed because:\n %s",
			execErr);
		sqlite3_free(execErr);
		goto bailout;
	}

	/* Iterate through the object space */
	do
	{
		std::string className;
		errorCode = saImmOmSearchNext_2(searchHandle, &objectName, &attrs);

		if (SA_AIS_OK != errorCode)
		{
			break;
		}

		if (attrs[0] == NULL)
		{
			TRACE_2("ABT Skipping object %s because no attributes from searchNext",
				(char *) objectName.value);
			continue;
		}

		objectToPBE(std::string((char*)objectName.value, objectName.length),
			attrs, immHandle, classIdMap, dbHandle, 
			++object_id);
	} while (true);

	if (SA_AIS_ERR_NOT_EXIST != errorCode)
	{
		LOG_ER("Failed in saImmOmSearchNext_2:%u - exiting", errorCode);
		goto bailout;
	}

	sqlite3_exec(dbHandle, "COMMIT TRANSACTION", NULL, NULL, &execErr);
	if(rc != SQLITE_OK) {
		LOG_ER("SQL statement ('COMMIT TRANSACTION') failed because:\n %s",
			execErr);
		sqlite3_free(execErr);
		goto bailout;
	}

	/* End the search */
	saImmOmSearchFinalize(searchHandle);
	TRACE_LEAVE();
	return;
 bailout:
	sqlite3_close(dbHandle);
	/* TODO remove imm.db file */
	exit(1);	
}

#else
void* pbeRepositoryInit(const char* filePath)
{
	LOG_WA("immdump not built with Pbe option.");
	return NULL;
}

void pbeRepositoryClose(void* dbHandle) 
{
}

void dumpClassesToPbe(SaImmHandleT immHandle, ClassMap *classIdMap,
	void* db_handle)
{
	assert(0);
}

void dumpObjectsToPbe(SaImmHandleT immHandle, ClassMap* classIdMap,
	void* db_handle)
{
	assert(0);
}
#endif
