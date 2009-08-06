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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdarg.h>
#include <syslog.h>

#include <immutil.h>

static SaVersionT immVersion = { 'A', 2, 1 };

/* Memory handling functions */
#define CHUNK	4000
static struct Chunk *newChunk(struct Chunk *next, size_t size);
static void *clistMalloc(struct Chunk *clist, size_t size);
static void deleteClist(struct Chunk *clist);

/* SA-item duplicate functions */
static const SaNameT *dupSaNameT(struct Chunk *clist, const SaNameT *original);
static SaImmClassNameT dupSaImmClassNameT(struct Chunk *clist, const SaImmClassNameT original);
static const SaImmAttrValuesT_2 **dupSaImmAttrValuesT_array(struct Chunk *clist, const SaImmAttrValuesT_2 **original);
static const SaImmAttrModificationT_2 **dupSaImmAttrModificationT_array(struct Chunk *clist,
									const SaImmAttrModificationT_2 **original);
static char *dupStr(struct Chunk *clist, const char *original);

static void defaultImmutilError(char const *fmt, ...)
    __attribute__ ((format(printf, 1, 2)));

ImmutilErrorFnT immutilError = defaultImmutilError;
static struct CcbUtilCcbData *ccbList = NULL;

/**
 * Report to stderr and syslog and abort process
 * @param fmt
 */
static void defaultImmutilError(char const *fmt, ...)
{
	va_list ap;
	va_list ap2;

	va_start(ap, fmt);
	va_copy(ap2, ap);
	vfprintf(stderr, fmt, ap);
	vsyslog(LOG_ERR, fmt, ap2);
	va_end(ap);
	abort();
}

static struct CcbUtilCcbData *ccbutil_createCcbData(SaImmOiCcbIdT id)
{
	struct Chunk *clist = newChunk(NULL, CHUNK);
	struct CcbUtilCcbData *obj = (struct CcbUtilCcbData *)
	    clistMalloc(clist, sizeof(struct CcbUtilCcbData));
	obj->id = id;
	obj->memref = clist;
	obj->next = ccbList;
	ccbList = obj;
	return obj;
}

struct CcbUtilCcbData *ccbutil_findCcbData(SaImmOiCcbIdT id)
{
	struct CcbUtilCcbData *ccbitem = ccbList;
	while (ccbitem != NULL) {
		if (ccbitem->id == id)
			return ccbitem;
		ccbitem = ccbitem->next;
	}
	return NULL;
}

struct CcbUtilCcbData *ccbutil_getCcbData(SaImmOiCcbIdT id)
{
	struct CcbUtilCcbData *ccbitem = ccbutil_findCcbData(id);
	if (ccbitem == NULL)
		ccbitem = ccbutil_createCcbData(id);
	return ccbitem;
}

void ccbutil_deleteCcbData(struct CcbUtilCcbData *ccb)
{
	struct CcbUtilCcbData *item = ccbList;
	struct CcbUtilCcbData *prev = NULL;
	if (ccb == NULL)
		return;
	while (item != NULL) {
		if (ccb->id == item->id) {
			if (prev == NULL) {
				ccbList = item->next;
			} else {
				prev->next = item->next;
			}
		}
		prev = item;
		item = item->next;
	}
	struct Chunk *clist = (struct Chunk *)ccb->memref;
	deleteClist(clist);
}

static struct CcbUtilOperationData *newOperationData(struct CcbUtilCcbData *ccb, enum CcbUtilOperationType type)
{
	struct Chunk *clist = (struct Chunk *)ccb->memref;
	struct CcbUtilOperationData *operation = (struct CcbUtilOperationData *)
	    clistMalloc(clist, sizeof(struct CcbUtilOperationData));
	operation->operationType = type;
	if (ccb->operationListTail == NULL) {
		ccb->operationListTail = operation;
		ccb->operationListHead = operation;
	} else {
		ccb->operationListTail->next = operation;
		ccb->operationListTail = operation;
	}
	return operation;
}

void ccbutil_ccbAddCreateOperation(struct CcbUtilCcbData *ccb,
				   const SaImmClassNameT className,
				   const SaNameT *parentName, const SaImmAttrValuesT_2 **attrValues)
{
	struct Chunk *clist = (struct Chunk *)ccb->memref;
	struct CcbUtilOperationData *operation = newOperationData(ccb, CCBUTIL_CREATE);
	operation->param.create.className = dupSaImmClassNameT(clist, className);
	operation->param.create.parentName = dupSaNameT(clist, parentName);
	operation->param.create.attrValues = dupSaImmAttrValuesT_array(clist, attrValues);
}

void ccbutil_ccbAddDeleteOperation(struct CcbUtilCcbData *ccb, const SaNameT *objectName)
{
	struct Chunk *clist = (struct Chunk *)ccb->memref;
	struct CcbUtilOperationData *operation = newOperationData(ccb, CCBUTIL_DELETE);
	operation->param.deleteOp.objectName = dupSaNameT(clist, objectName);
}

void ccbutil_ccbAddModifyOperation(struct CcbUtilCcbData *ccb,
				   const SaNameT *objectName, const SaImmAttrModificationT_2 **attrMods)
{
	struct Chunk *clist = (struct Chunk *)ccb->memref;
	struct CcbUtilOperationData *operation = newOperationData(ccb, CCBUTIL_MODIFY);
	operation->param.modify.objectName = dupSaNameT(clist, objectName);
	operation->param.modify.attrMods = dupSaImmAttrModificationT_array(clist, attrMods);
}

/* ----------------------------------------------------------------------
 * General IMM help utilities;
 */

char *immutil_strdup(struct CcbUtilCcbData *ccb, char const *source)
{
	struct Chunk *clist = (struct Chunk *)ccb->memref;
	return dupStr(clist, source);
}

char const *immutil_getClassName(struct CcbUtilCcbData *ccb, SaImmHandleT immHandle, const SaNameT *objectName)
{
	struct Chunk *clist = (struct Chunk *)ccb->memref;
	SaImmAccessorHandleT accessorHandle;
	SaImmAttrValuesT_2 **attributes;
	SaImmAttrValuesT_2 *cnameattr;
	SaImmAttrNameT classNameAttr[2] = { SA_IMM_ATTR_CLASS_NAME, NULL };
	char *cname = NULL;

	if (objectName == NULL)
		return NULL;
	if (immutil_saImmOmAccessorInitialize(immHandle, &accessorHandle) != SA_AIS_OK)
		return NULL;

	if (immutil_saImmOmAccessorGet_2(accessorHandle, objectName, classNameAttr, &attributes)
	    != SA_AIS_OK)
		goto finalize;
	if (attributes == NULL || *attributes == NULL)
		goto finalize;

	cnameattr = *attributes;
	if (strcmp(cnameattr->attrName, SA_IMM_ATTR_CLASS_NAME) != 0)
		goto finalize;
	assert(cnameattr->attrValueType == SA_IMM_ATTR_SASTRINGT);
	assert(cnameattr->attrValuesNumber == 1);

	cname = dupStr(clist, *((char **)*(cnameattr->attrValues)));

 finalize:
	(void)immutil_saImmOmAccessorFinalize(accessorHandle);
	return cname;
}

char const *immutil_getStringValue(char const *key, SaNameT const *name)
{
	static char buffer[SA_MAX_NAME_LENGTH + 1];
	unsigned int klen;
	char *cp;

	assert(name->length <= SA_MAX_NAME_LENGTH);
	memcpy(buffer, name->value, name->length);
	buffer[name->length] = 0;
	assert(key != NULL);
	klen = strlen(key);
	assert(klen > 1 || key[klen - 1] == '=');

	cp = strstr(buffer, key);
	while (cp != NULL) {
		if (cp == buffer || cp[-1] == ',') {
			char *value = cp + klen;
			if (*value == 0 || *value == ',')
				return NULL;
			cp = strchr(value, ',');
			if (cp != NULL)
				*cp = 0;
			return value;
		}
		cp += klen;
		cp = strstr(cp, key);
	}
	return NULL;
}

char const *immutil_getDnItem(SaNameT const *name, unsigned int index)
{
	static char buffer[SA_MAX_NAME_LENGTH + 1];
	char *cp;
	char *value;

	assert(name->length <= SA_MAX_NAME_LENGTH);
	memcpy(buffer, name->value, name->length);
	buffer[name->length] = 0;

	value = buffer;
	cp = strchr(value, ',');
	while (index > 0) {
		if (cp == NULL)
			return NULL;
		value = cp + 1;
		cp = strchr(value, ',');
		index--;
	}
	if (cp != NULL)
		*cp = 0;
	return value;
}

long immutil_getNumericValue(char const *key, SaNameT const *name)
{
	char const *vp = immutil_getStringValue(key, name);
	char *endptr;
	long value;

	if (vp == NULL)
		return LONG_MIN;
	value = strtol(vp, &endptr, 0);
	if (endptr == NULL || endptr == vp || (*endptr != 0 && *endptr != ','))
		return LONG_MIN;
	return value;
}

char const *immutil_strnchr(char const *str, int c, size_t length)
{
	while (length > 0 && *str != 0) {
		if (*str == c)
			return str;
		str++;
		length--;
	}
	return NULL;
}

const SaNameT *immutil_getNameAttr(const SaImmAttrValuesT_2 **attr, char const *name, unsigned int index)
{
	unsigned int i;
	if (attr == NULL || attr[0] == NULL)
		return NULL;
	for (i = 0; attr[i] != NULL; i++) {
		if (strcmp(attr[i]->attrName, name) == 0) {
			if (index >= attr[i]->attrValuesNumber
			    || attr[i]->attrValues == NULL || attr[i]->attrValueType != SA_IMM_ATTR_SANAMET)
				return NULL;
			return (SaNameT *)attr[i]->attrValues[index];
		}
	}
	return NULL;
}

char const *immutil_getStringAttr(const SaImmAttrValuesT_2 **attr, char const *name, unsigned int index)
{
	unsigned int i;
	if (attr == NULL || attr[0] == NULL)
		return NULL;
	for (i = 0; attr[i] != NULL; i++) {
		if (strcmp(attr[i]->attrName, name) == 0) {
			if (index >= attr[i]->attrValuesNumber
			    || attr[i]->attrValues == NULL || attr[i]->attrValueType != SA_IMM_ATTR_SASTRINGT)
				return NULL;
			return *((const SaStringT *)attr[i]->attrValues[index]);
		}
	}
	return NULL;
}

const SaUint32T *immutil_getUint32Attr(const SaImmAttrValuesT_2 **attr, char const *name, unsigned int index)
{
	unsigned int i;
	if (attr == NULL || attr[0] == NULL)
		return NULL;
	for (i = 0; attr[i] != NULL; i++) {
		if (strcmp(attr[i]->attrName, name) == 0) {
			if (index >= attr[i]->attrValuesNumber
			    || attr[i]->attrValues == NULL || attr[i]->attrValueType != SA_IMM_ATTR_SAUINT32T)
				return NULL;
			return (SaUint32T *)attr[i]->attrValues[index];
		}
	}
	return NULL;
}

int immutil_matchName(SaNameT const *name, regex_t const *preg)
{
	char buffer[SA_MAX_NAME_LENGTH + 1];
	assert(name != NULL && preg != NULL);
	memcpy(buffer, name->value, name->length);
	buffer[name->length] = 0;
	return regexec(preg, buffer, 0, NULL, 0);
}

SaAisErrorT immutil_update_one_rattr(SaImmOiHandleT immOiHandle,
				     const char *dn,
				     SaImmAttrNameT attributeName, SaImmValueTypeT attrValueType, void *value)
{
	SaImmAttrModificationT_2 attrMod;
	const SaImmAttrModificationT_2 *attrMods[] = { &attrMod, NULL };
	SaImmAttrValueT attrValues[] = { value };
	SaNameT objectName;

	strncpy((char *)objectName.value, dn, SA_MAX_NAME_LENGTH);
	objectName.length = strnlen((char *)objectName.value, SA_MAX_NAME_LENGTH);

	attrMod.modType = SA_IMM_ATTR_VALUES_REPLACE;
	attrMod.modAttr.attrName = attributeName;
	attrMod.modAttr.attrValuesNumber = 1;
	attrMod.modAttr.attrValueType = attrValueType;
	attrMod.modAttr.attrValues = attrValues;
	return immutil_saImmOiRtObjectUpdate_2(immOiHandle, &objectName, attrMods);
}

SaImmClassNameT immutil_get_className(SaNameT *objectName)
{
	SaImmHandleT omHandle;
	SaImmClassNameT className;
	SaImmAccessorHandleT accessorHandle;
	SaImmAttrValuesT_2 **attributes;
	SaImmAttrNameT attributeNames[] = { "SaImmAttrClassName", NULL };

	(void)immutil_saImmOmInitialize(&omHandle, NULL, &immVersion);
	(void)immutil_saImmOmAccessorInitialize(omHandle, &accessorHandle);
	(void)immutil_saImmOmAccessorGet_2(accessorHandle, objectName, attributeNames, &attributes);
	className = strdup(*((char **)attributes[0]->attrValues[0]));
	(void)immutil_saImmOmAccessorFinalize(accessorHandle);
	(void)immutil_saImmOmFinalize(omHandle);

	return className;
}

SaAisErrorT immutil_get_attrValueType(SaNameT *objectName, SaImmAttrNameT attrName, SaImmValueTypeT *attrValueType)
{
	SaAisErrorT rc = SA_AIS_ERR_INVALID_PARAM;
	SaImmHandleT omHandle;
	SaImmClassCategoryT classCategory;
	SaImmAttrDefinitionT_2 *attrDef;
	SaImmAttrDefinitionT_2 **attrDefinitions;
	SaImmClassNameT className = immutil_get_className(objectName);
	int i = 0;

	(void)immutil_saImmOmInitialize(&omHandle, NULL, &immVersion);
	(void)saImmOmClassDescriptionGet_2(omHandle, className, &classCategory, &attrDefinitions);

	while ((attrDef = attrDefinitions[i++]) != NULL) {
		if (!strcmp(attrName, attrDef->attrName)) {
			*attrValueType = attrDef->attrValueType;
			rc = SA_AIS_OK;
			break;
		}
	}

	(void)saImmOmClassDescriptionMemoryFree_2(omHandle, attrDefinitions);
	free(className);
	return rc;
}

void *immutil_new_attrValue(SaImmValueTypeT attrValueType, const char *str)
{
	void *attrValue = NULL;

	switch (attrValueType) {
	case SA_IMM_ATTR_SAINT32T:
		attrValue = malloc(sizeof(SaInt32T));
		*((SaInt32T *)attrValue) = strtol(str, NULL, 10);
		break;
	case SA_IMM_ATTR_SAUINT32T:
		attrValue = malloc(sizeof(SaUint32T));
		*((SaUint32T *)attrValue) = strtol(str, NULL, 10);
		break;
	case SA_IMM_ATTR_SAINT64T:
		attrValue = malloc(sizeof(SaInt64T));
		*((SaInt64T *)attrValue) = strtoll(str, NULL, 10);
		break;
	case SA_IMM_ATTR_SAUINT64T:
		attrValue = malloc(sizeof(SaUint64T));
		*((SaUint64T *)attrValue) = strtoll(str, NULL, 10);
		break;
	case SA_IMM_ATTR_SATIMET:
		attrValue = malloc(sizeof(SaTimeT));
		*((SaTimeT *)attrValue) = strtoll(str, NULL, 10);
		break;
	case SA_IMM_ATTR_SAFLOATT:
		{
			SaFloatT myfloat;
			attrValue = malloc(sizeof(SaFloatT));
			sscanf(str, "%f", &myfloat);
			*((SaFloatT *)attrValue) = myfloat;
			break;
		}
	case SA_IMM_ATTR_SADOUBLET:
		{
			double mydouble;
			attrValue = malloc(sizeof(double));
			sscanf(str, "%lf", &mydouble);
			*((double *)attrValue) = mydouble;
			break;
		}
	case SA_IMM_ATTR_SANAMET:
		{
			SaNameT *mynamet;
			attrValue = mynamet = malloc(sizeof(SaNameT));
			mynamet->length = strnlen(str, SA_MAX_NAME_LENGTH);
			strncpy((char *)mynamet->value, str, SA_MAX_NAME_LENGTH);
			break;
		}
	case SA_IMM_ATTR_SASTRINGT:
		{
			attrValue = malloc(sizeof(SaStringT));
			*((SaStringT *)attrValue) = strdup(str);
			break;
		}
	case SA_IMM_ATTR_SAANYT:
		/* Cannot handle this one... */
	default:
		break;
	}

	return attrValue;
}

/* ----------------------------------------------------------------------
 * SA-item duplicate functions
 */

static char *dupStr(struct Chunk *clist, const char *original)
{
	unsigned int len;
	if (original == NULL)
		return NULL;
	len = strlen(original) + 1;
	SaImmClassNameT copy = (SaImmClassNameT)clistMalloc(clist, len);
	memcpy(copy, original, len);
	return copy;
}

static const SaNameT *dupSaNameT(struct Chunk *clist, const SaNameT *original)
{
	SaNameT *copy;
	if (original == NULL)
		return NULL;
	copy = (SaNameT *)clistMalloc(clist, sizeof(SaNameT));
	memcpy(copy, original, sizeof(SaNameT));
	return copy;
}

static SaImmClassNameT dupSaImmClassNameT(struct Chunk *clist, const SaImmClassNameT original)
{
	return dupStr(clist, (const char *)original);
}

/*
 * The "SaImmAttrValueT* attrValues" field;
 *
 *                  +-------+
 * attrValues ----> ! void* + --------> value0
 *                  +-------+
 *                  ! void* + --------> value1
 *                  +-------+
 *                  ! void* + --------> value2
 *                  +-------+
 *                  !       !
 *
 * Especially silly case for one SaString;
 *
 *                  +-------+         +-------+
 * attrValues ----> ! void* + ------->! char* !------> char-data
 *                  +-------+         +-------+
 *
 */

static void copySaImmAttrValuesT(struct Chunk *clist, SaImmAttrValuesT_2 *copy, const SaImmAttrValuesT_2 *original)
{
	size_t valueSize;
	unsigned int i, valueCount = original->attrValuesNumber;
	char *databuffer;
	copy->attrName = dupStr(clist, (const char *)original->attrName);
	copy->attrValuesNumber = valueCount;
	copy->attrValueType = original->attrValueType;
	if (valueCount == 0)
		return;		/* (just in case...) */
	copy->attrValues = clistMalloc(clist, valueCount * sizeof(SaImmAttrValueT));

	switch (original->attrValueType) {
	case SA_IMM_ATTR_SAINT32T:
		valueSize = sizeof(SaInt32T);
		break;
	case SA_IMM_ATTR_SAUINT32T:
		valueSize = sizeof(SaUint32T);
		break;
	case SA_IMM_ATTR_SAINT64T:
		valueSize = sizeof(SaInt64T);
		break;
	case SA_IMM_ATTR_SAUINT64T:
		valueSize = sizeof(SaUint64T);
		break;
	case SA_IMM_ATTR_SATIMET:
		valueSize = sizeof(SaTimeT);
		break;
	case SA_IMM_ATTR_SANAMET:
		valueSize = sizeof(SaNameT);
		break;
	case SA_IMM_ATTR_SAFLOATT:
		valueSize = sizeof(SaFloatT);
		break;
	case SA_IMM_ATTR_SADOUBLET:
		valueSize = sizeof(SaDoubleT);
		break;
	case SA_IMM_ATTR_SASTRINGT:
		valueSize = sizeof(SaStringT);
		break;
	case SA_IMM_ATTR_SAANYT:
		assert(0);
		break;
	}

	databuffer = (char *)clistMalloc(clist, valueCount * valueSize);
	for (i = 0; i < valueCount; i++) {
		copy->attrValues[i] = databuffer;
		if (original->attrValueType == SA_IMM_ATTR_SASTRINGT) {
			char *cporig = *((char **)original->attrValues[i]);
			char **cpp = (char **)databuffer;
			*cpp = dupStr(clist, cporig);
		} else {
			memcpy(databuffer, original->attrValues[i], valueSize);
		}
		databuffer += valueSize;
	}
}

static const SaImmAttrValuesT_2 *dupSaImmAttrValuesT(struct Chunk *clist, const SaImmAttrValuesT_2 *original)
{
	SaImmAttrValuesT_2 *copy = (SaImmAttrValuesT_2 *)
	    clistMalloc(clist, sizeof(SaImmAttrValuesT_2));
	copySaImmAttrValuesT(clist, copy, original);
	return copy;
}

static const SaImmAttrModificationT_2 *dupSaImmAttrModificationT(struct Chunk *clist,
								 const SaImmAttrModificationT_2 *original)
{
	SaImmAttrModificationT_2 *copy = (SaImmAttrModificationT_2 *)
	    clistMalloc(clist, sizeof(SaImmAttrModificationT_2));
	copy->modType = original->modType;
	copySaImmAttrValuesT(clist, &(copy->modAttr), &(original->modAttr));
	return copy;
}

static const SaImmAttrValuesT_2 **dupSaImmAttrValuesT_array(struct Chunk *clist, const SaImmAttrValuesT_2 **original)
{
	const SaImmAttrValuesT_2 **copy;
	unsigned int i, alen = 0;
	if (original == NULL)
		return NULL;
	while (original[alen] != NULL)
		alen++;
	copy = (const SaImmAttrValuesT_2 **)
	    clistMalloc(clist, (alen + 1) * sizeof(SaImmAttrValuesT_2 *));
	for (i = 0; i < alen; i++) {
		copy[i] = dupSaImmAttrValuesT(clist, original[i]);
	}
	return copy;
}

static const SaImmAttrModificationT_2 **dupSaImmAttrModificationT_array(struct Chunk *clist,
									const SaImmAttrModificationT_2 **original)
{
	const SaImmAttrModificationT_2 **copy;
	unsigned int i, alen = 0;
	if (original == NULL)
		return NULL;
	while (original[alen] != NULL)
		alen++;
	copy = (const SaImmAttrModificationT_2 **)
	    clistMalloc(clist, (alen + 1) * sizeof(SaImmAttrModificationT_2 *));
	for (i = 0; i < alen; i++) {
		copy[i] = dupSaImmAttrModificationT(clist, original[i]);
	}
	return copy;
}

/* ----------------------------------------------------------------------
 * Memory handling
 */

struct Chunk {
	struct Chunk *next;
	unsigned int capacity;
	unsigned int free;
	unsigned char data[1];
};

static struct Chunk *newChunk(struct Chunk *next, size_t size)
{
	struct Chunk *chunk = (struct Chunk *)malloc(sizeof(struct Chunk) + size);
	if (chunk == NULL)
		immutilError("Out of memory");
	chunk->next = next;
	chunk->capacity = size;
	chunk->free = size;
	return chunk;
}

static void deleteClist(struct Chunk *clist)
{
	while (clist != NULL) {
		struct Chunk *chunk = clist;
		clist = clist->next;
		free(chunk);
	}
}

static void *clistMalloc(struct Chunk *clist, size_t size)
{
	struct Chunk *chunk;

	size = (size + 3) & ~3;

	if (size > CHUNK) {
		chunk = newChunk(clist->next, size);
		clist->next = chunk;
		chunk->free = 0;
		memset(chunk->data, 0, size);
		return chunk->data;
	}

	for (chunk = clist; chunk != NULL; chunk = chunk->next) {
		if (chunk->free >= size) {
			unsigned char *mem = chunk->data + (chunk->capacity - chunk->free);
			chunk->free -= size;
			memset(mem, 0, size);
			return mem;
		}
	}

	chunk = newChunk(clist->next, CHUNK);
	clist->next = chunk;
	chunk->free -= size;
	memset(chunk->data, 0, size);
	return chunk->data;
}

/* ----------------------------------------------------------------------
 * IMM call wrappers; This wrapper interface offloads the burden to
 * handle return values and retries for each and every IMM-call. It
 * makes the code cleaner.
 */

struct ImmutilWrapperProfile immutilWrapperProfile = { 1, 25, 400 };

SaAisErrorT immutil_saImmOiInitialize_2(SaImmOiHandleT *immOiHandle,
					const SaImmOiCallbacksT_2 *immOiCallbacks, SaVersionT *version)
{
	SaAisErrorT rc = saImmOiInitialize_2(immOiHandle, immOiCallbacks, version);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOiInitialize_2(immOiHandle, immOiCallbacks, version);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOiInitialize FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOiSelectionObjectGet(SaImmOiHandleT immOiHandle, SaSelectionObjectT *selectionObject)
{
	SaAisErrorT rc = saImmOiSelectionObjectGet(immOiHandle, selectionObject);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOiSelectionObjectGet(immOiHandle, selectionObject);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOiSelectionObjectGet FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOiClassImplementerSet(SaImmOiHandleT immOiHandle, const SaImmClassNameT className)
{
	SaAisErrorT rc = saImmOiClassImplementerSet(immOiHandle, className);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOiClassImplementerSet(immOiHandle, className);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOiClassImplementerSet FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOiClassImplementerRelease(SaImmOiHandleT immOiHandle, const SaImmClassNameT className)
{
	SaAisErrorT rc = saImmOiClassImplementerRelease(immOiHandle, className);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOiClassImplementerRelease(immOiHandle, className);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOiClassImplementerRelease FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOiImplementerSet(SaImmOiHandleT immOiHandle, const SaImmOiImplementerNameT implementerName)
{
	SaAisErrorT rc = saImmOiImplementerSet(immOiHandle, implementerName);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOiImplementerSet(immOiHandle, implementerName);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOiImplementerSet FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOiImplementerClear(SaImmOiHandleT immOiHandle)
{
	SaAisErrorT rc = saImmOiImplementerClear(immOiHandle);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOiImplementerClear(immOiHandle);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOiImplementerClear FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOiRtObjectCreate_2(SaImmOiHandleT immOiHandle,
					    const SaImmClassNameT className,
					    const SaNameT *parentName, const SaImmAttrValuesT_2 **attrValues)
{
	SaAisErrorT rc = saImmOiRtObjectCreate_2(immOiHandle, className, parentName, attrValues);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOiRtObjectCreate_2(immOiHandle, className, parentName, attrValues);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOiRtObjectCreate_2 FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOiRtObjectDelete(SaImmOiHandleT immOiHandle, const SaNameT *objectName)
{
	SaAisErrorT rc = saImmOiRtObjectDelete(immOiHandle, objectName);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOiRtObjectDelete(immOiHandle, objectName);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOiRtObjectDelete FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOiRtObjectUpdate_2(SaImmOiHandleT immOiHandle,
					    const SaNameT *objectName, const SaImmAttrModificationT_2 **attrMods)
{
	SaAisErrorT rc = saImmOiRtObjectUpdate_2(immOiHandle, objectName, attrMods);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOiRtObjectUpdate_2(immOiHandle, objectName, attrMods);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOiRtObjectUpdate_2 FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOiAdminOperationResult(SaImmOiHandleT immOiHandle,
						SaInvocationT invocation, SaAisErrorT result)
{
	SaAisErrorT rc = saImmOiAdminOperationResult(immOiHandle, invocation, result);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOiAdminOperationResult(immOiHandle, invocation, result);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOiAdminOperationResult FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOmInitialize(SaImmHandleT *immHandle, const SaImmCallbacksT *immCallbacks, SaVersionT *version)
{
	SaAisErrorT rc = saImmOmInitialize(immHandle, immCallbacks, version);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOmInitialize(immHandle, immCallbacks, version);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOmInitialize FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOmFinalize(SaImmHandleT immHandle)
{
	SaAisErrorT rc = saImmOmFinalize(immHandle);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOmFinalize(immHandle);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOmFinalize FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOmAccessorInitialize(SaImmHandleT immHandle, SaImmAccessorHandleT *accessorHandle)
{
	SaAisErrorT rc = saImmOmAccessorInitialize(immHandle, accessorHandle);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOmAccessorInitialize(immHandle, accessorHandle);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOmAccessorInitialize FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOmAccessorGet_2(SaImmAccessorHandleT accessorHandle,
					 const SaNameT *objectName,
					 const SaImmAttrNameT *attributeNames, SaImmAttrValuesT_2 ***attributes)
{
	SaAisErrorT rc = saImmOmAccessorGet_2(accessorHandle, objectName, attributeNames, attributes);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOmAccessorGet_2(accessorHandle, objectName, attributeNames, attributes);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOmAccessorGet FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOmAccessorFinalize(SaImmAccessorHandleT accessorHandle)
{
	SaAisErrorT rc = saImmOmAccessorFinalize(accessorHandle);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOmAccessorFinalize(accessorHandle);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOmAccessorFinalize FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOmSearchInitialize_2(SaImmHandleT immHandle,
					      const SaNameT *rootName,
					      SaImmScopeT scope,
					      SaImmSearchOptionsT searchOptions,
					      const SaImmSearchParametersT_2 *searchParam,
					      const SaImmAttrNameT *attributeNames, SaImmSearchHandleT *searchHandle)
{
	SaAisErrorT rc = saImmOmSearchInitialize_2(immHandle, rootName, scope, searchOptions, searchParam,
						   attributeNames, searchHandle);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOmSearchInitialize_2(immHandle, rootName, scope, searchOptions, searchParam,
					       attributeNames, searchHandle);
		nTries++;
	}
	if (rc == SA_AIS_ERR_NOT_EXIST)
		return rc;
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOmSearchInitialize FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOmSearchFinalize(SaImmSearchHandleT searchHandle)
{
	SaAisErrorT rc = saImmOmSearchFinalize(searchHandle);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOmSearchFinalize(searchHandle);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOmSearchFinalize FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOmSearchNext_2(SaImmSearchHandleT searchHandle,
					SaNameT *objectName, SaImmAttrValuesT_2 ***attributes)
{
	SaAisErrorT rc = saImmOmSearchNext_2(searchHandle, objectName, attributes);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOmSearchNext_2(searchHandle, objectName, attributes);
		nTries++;
	}
	if (rc == SA_AIS_ERR_NOT_EXIST)
		return rc;
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOmSearchNext FAILED, rc = %d", (int)rc);
	return rc;
}

SaAisErrorT immutil_saImmOmAdminOwnerClear(SaImmHandleT immHandle, const SaNameT **objectNames, SaImmScopeT scope)
{
	SaAisErrorT rc = saImmOmAdminOwnerClear(immHandle, objectNames, scope);
	unsigned int nTries = 1;
	while (rc == SA_AIS_ERR_TRY_AGAIN && nTries < immutilWrapperProfile.nTries) {
		usleep(immutilWrapperProfile.retryInterval * 1000);
		rc = saImmOmAdminOwnerClear(immHandle, objectNames, scope);
		nTries++;
	}
	if (rc != SA_AIS_OK && immutilWrapperProfile.errorsAreFatal)
		immutilError("saImmOmAdminOwnerClear FAILED, rc = %d", (int)rc);
	return rc;
}
