/*
 * immombin.c -
 *   IMM Object Manager in Python. This file implements the low level
 *   adaptation to the SAF IMM API. There is a counterpart in Python,
 *   the "immom.py" module.
 */

#include <limits.h>
#include <Python.h>
#include <saImm.h>
#include <saImmOm.h>

static SaVersionT const immVersion = {'A', 2, 0};
static SaImmHandleT immOmHandle = 0;
static SaImmAdminOwnerHandleT adminOwnerHandle;
static SaImmCcbHandleT ccbHandle;
static int haveCcb = 0;
static int haveAdminOwner = 0;
static PyObject* aisException;


/* ----------------------------------------------------------------------
 * Simplified memory handling:
 * The idea is to use one (or a few) mallocs in a down-call. All memory
 * is released in a "memfreeall()" on return. This simplifies free of the
 * sometimes very complex structures used in SAF and prevents memory leaks.
 * immom_aisException as well as the help functions "immom_return_null" and
 * "immom_return_None" frees memory automatically and should be used when
 * applicable.
 */

struct MemChunk {
    struct MemChunk* next;
    unsigned int size;
    unsigned int used;
    char* blob;
};
static struct MemChunk* memChunk = NULL;
#define MEM_MINSIZE 4000
static void mem_addchunk(unsigned int minsize)
{
    struct MemChunk* mc;
    if (minsize < MEM_MINSIZE)
        minsize = MEM_MINSIZE;
    mc = (struct MemChunk*)malloc(sizeof(struct MemChunk) + minsize);
    if (mc == NULL) {
        Py_FatalError("Out of memory");
        abort();            /* (never reached?) */
    }
    mc->next = memChunk;
    mc->size = minsize;
    mc->used = 0;
    mc->blob = (char*)(mc+1);
    memChunk = mc;
}

static void* memget(unsigned int size)
{
    void* mp;
    if (memChunk == NULL || (memChunk->size - memChunk->used) > size)
        mem_addchunk(size);
    mp = memChunk->blob + memChunk->used;
    memChunk->used += size;
    return mp;
}

static void memfreeall(void)
{
    while (memChunk != NULL) {
        struct MemChunk* next = memChunk->next;
        free(memChunk);
        memChunk = next;
    }
}


/* ----------------------------------------------------------------------
 * Help functions;
 */

static char const* aiserr2str(SaAisErrorT rc)
{
    static char const* estr[] = {
        "UNKNOWN_ERROR",
        "SA_AIS_OK",
        "SA_AIS_ERR_LIBRARY",
        "SA_AIS_ERR_VERSION",
        "SA_AIS_ERR_INIT",
        "SA_AIS_ERR_TIMEOUT",
        "SA_AIS_ERR_TRY_AGAIN",
        "SA_AIS_ERR_INVALID_PARAM",
        "SA_AIS_ERR_NO_MEMORY",
        "SA_AIS_ERR_BAD_HANDLE",
        "SA_AIS_ERR_BUSY",
        "SA_AIS_ERR_ACCESS",
        "SA_AIS_ERR_NOT_EXIST",
        "SA_AIS_ERR_NAME_TOO_LONG",
        "SA_AIS_ERR_EXIST",
        "SA_AIS_ERR_NO_SPACE",
        "SA_AIS_ERR_INTERRUPT",
        "SA_AIS_ERR_NAME_NOT_FOUND",
        "SA_AIS_ERR_NO_RESOURCES",
        "SA_AIS_ERR_NOT_SUPPORTED",
        "SA_AIS_ERR_BAD_OPERATION",
        "SA_AIS_ERR_FAILED_OPERATION",
        "SA_AIS_ERR_MESSAGE_ERROR",
        "SA_AIS_ERR_QUEUE_FULL",
        "SA_AIS_ERR_QUEUE_NOT_AVAILABLE",
        "SA_AIS_ERR_BAD_FLAGS",
        "SA_AIS_ERR_TOO_BIG",
        "SA_AIS_ERR_NO_SECTIONS",
        "SA_AIS_ERR_NO_OP",
        "SA_AIS_ERR_REPAIR_PENDING",
        "SA_AIS_ERR_NO_BINDINGS",
        "SA_AIS_ERR_UNAVAILABLE"
    };
    unsigned int i = (unsigned int)rc;
    if (i > SA_AIS_ERR_UNAVAILABLE) i=0;
    return estr[i];
}

static char const* vtstr[] = {
    "INVALID TYPE",
    "SAINT32T",
    "SAUINT32T",
    "SAINT64T",
    "SAUINT64T",
    "SATIMET",
    "SANAMET",
    "SAFLOATT",
    "SADOUBLET",
    "SASTRINGT",
    "SAANYT"
};
static char const* valuetype2str(SaImmValueTypeT t)
{
    unsigned int i = (unsigned int)t;
    if (i > SA_IMM_ATTR_SAANYT) i = 0;
    return vtstr[i];
}
static SaImmValueTypeT immom_parseTypeStr(
    char const* typestr)
{
    unsigned int i = (unsigned int)SA_IMM_ATTR_SAANYT;
    assert(i == 10);            /* Fail-fast on enum-change */
    while (i > 0 && strcmp(typestr, vtstr[i]) != 0) i--;
    return (SaImmValueTypeT)i;
}

static void*
immom_return_null(void)
{
    memfreeall();
    return NULL;
}

static PyObject*
immom_return_None(void)
{
    memfreeall();
    Py_RETURN_NONE;
}

static void*
immom_aisException(SaAisErrorT rc)
{
    PyErr_SetString(aisException, aiserr2str(rc));
    return immom_return_null();
}

static SaImmScopeT
immom_SaImmScope(char const* paramStr)
{
    if (strcmp(paramStr, "SA_IMM_ONE") == 0) {
        return SA_IMM_ONE;
    } else if (strcmp(paramStr, "SA_IMM_SUBLEVEL") == 0) {
        return SA_IMM_SUBLEVEL;
    } else if (strcmp(paramStr, "SA_IMM_SUBTREE") == 0) {
        return SA_IMM_SUBTREE;
    }
    return 0;
}

static SaNameT* immom_saName(char const* str, SaNameT* saname)
{
    int len = strlen(str);
    if (len > SA_MAX_NAME_LENGTH)
        return immom_aisException(SA_AIS_ERR_NAME_TOO_LONG);
    saname->length = len;
    memcpy(saname->value, str, len);
    if (len < SA_MAX_NAME_LENGTH)
        saname->value[len] = 0;
    return saname;
}

static SaNameT**
immom_parseNames(PyObject* nameList)
{
    SaNameT** all;
    SaNameT* names;
    unsigned int i;
    unsigned int len = PyList_Size(nameList);

    if (len == 0) return immom_aisException(SA_AIS_ERR_INVALID_PARAM);
    all = (SaNameT**)memget((len+1)*(sizeof(SaNameT*)+sizeof(SaNameT)));
    names = (SaNameT*)(all+(len+1));
    for (i = 0; i < len; i++) {
        PyObject* item = PyList_GetItem(nameList, i);
        char const* str = PyString_AsString(item);
        SaNameT* n = names+i;
        if (str == NULL) return NULL;
        all[i] = n;
        if (immom_saName(str, n) == NULL) return NULL;
     }
    all[len] = NULL;
    if (0) {
        SaNameT** np;
        for (np = all; *np != NULL; np++) {
            printf(" --> [%s]\n", (*np)->value);
        }
    }
    return all;
}

static SaImmAttrValuesT_2*
immom_parseAttrValue(
    SaImmAttrValuesT_2* attr,   /* OUT-parameter */
    char* name, char const* typestr, PyObject* valueList)
{
    unsigned int i;
    attr->attrName = name;
    attr->attrValueType = immom_parseTypeStr(typestr);
    if ((unsigned int)attr->attrValueType == 0)
        return NULL;
    attr->attrValuesNumber = PyList_Size(valueList);
    attr->attrValues = (SaImmAttrValueT*)memget(
        sizeof(SaImmAttrValueT) * (attr->attrValuesNumber+1));
    attr->attrValues[attr->attrValuesNumber] = NULL;
    for (i = 0; i < attr->attrValuesNumber; i++) {
        PyObject* item = PyList_GetItem(valueList, i);
        switch (attr->attrValueType) {
        case SA_IMM_ATTR_SAINT32T: {
            SaInt32T* vp = (SaInt32T*)memget(sizeof(SaInt32T));
            *vp = PyInt_AsLong(item);
            if (PyErr_Occurred()) return immom_return_null();
            attr->attrValues[i] = vp;
            break;
        }
        case SA_IMM_ATTR_SAUINT32T: {
            SaUint32T* vp = (SaUint32T*)memget(sizeof(SaUint32T));
            *vp = PyInt_AsUnsignedLongMask(item);
            if (PyErr_Occurred()) return immom_return_null();
            attr->attrValues[i] = vp;
            break;
        }
        case SA_IMM_ATTR_SATIMET:
        case SA_IMM_ATTR_SAINT64T: {
            SaInt64T* vp = (SaInt64T*)memget(sizeof(SaInt64T));
            *vp = PyLong_AsLongLong(item);
            if (PyErr_Occurred()) return immom_return_null();
            attr->attrValues[i] = vp;
            break;
        }
        case SA_IMM_ATTR_SAUINT64T: {
            SaUint64T* vp = (SaUint64T*)memget(sizeof(SaUint64T));
	    /* PyLong_AsUnsignedLongLong seems to be broken...
               *vp = PyLong_AsUnsignedLongLong(item);*/
            *vp = PyLong_AsLongLong(item);
            if (PyErr_Occurred()) return immom_return_null();
            attr->attrValues[i] = vp;
            break;
        }
        case SA_IMM_ATTR_SANAMET: {
            SaNameT* vp = (SaNameT*)memget(sizeof(SaNameT));
            char* str = PyString_AsString(item);
            if (str == NULL) return immom_return_null();
            if (immom_saName(str, vp) == NULL) return NULL;
            attr->attrValues[i] = vp;
            break;
        }
        case SA_IMM_ATTR_SASTRINGT: {
            SaStringT* vp = (SaStringT*)memget(sizeof(SaStringT));
            char* str = PyString_AsString(item);
            if (str == NULL) return immom_return_null();
            *vp = str;
            attr->attrValues[i] = vp;
            break;
        }
        case SA_IMM_ATTR_SAFLOATT:
        case SA_IMM_ATTR_SADOUBLET:
        case SA_IMM_ATTR_SAANYT:
        default:
            return immom_aisException(SA_AIS_ERR_NOT_SUPPORTED);
        }
    }
    return attr;
}

static PyObject*
immom_makeValueList(SaImmAttrValuesT_2* a, PyObject* vlist)
{
        switch (a->attrValueType) {
        case SA_IMM_ATTR_SAINT32T: {
            if (a->attrValuesNumber > 0) {
                int i;
                for (i = 0; i < a->attrValuesNumber; i++) {
                    long int v = *((SaInt32T*)a->attrValues[i]);
                    if (PyList_Append(vlist, Py_BuildValue("l", v)) < 0) 
                        return NULL;
                }
            }
            break;
        }
        case SA_IMM_ATTR_SAUINT32T: {
            if (a->attrValuesNumber > 0) {
                int i;
                for (i = 0; i < a->attrValuesNumber; i++) {
                    unsigned long v = *((SaUint32T*)a->attrValues[i]);
                    if (PyList_Append(vlist, Py_BuildValue("k", v)) < 0) 
                        return NULL;
                }
            }
            break;
        }
        case SA_IMM_ATTR_SANAMET: {
            if (a->attrValuesNumber > 0) {
                int i;
                for (i = 0; i < a->attrValuesNumber; i++) {
                    const SaNameT* v = (SaNameT*)a->attrValues[i];
                    if (PyList_Append(vlist, Py_BuildValue("s", v->value)) < 0)
                        return NULL;
                }
            }
            break;
        }
        case SA_IMM_ATTR_SASTRINGT: {
            if (a->attrValuesNumber > 0) {
                int i;
                for (i = 0; i < a->attrValuesNumber; i++) {
                    char const* v = *((char const**)a->attrValues[i]);
                    if (PyList_Append(vlist, Py_BuildValue("s", v)) < 0)
                        return NULL;
                }
            }
            break;
        }
        case SA_IMM_ATTR_SAINT64T: {
            if (a->attrValuesNumber > 0) {
                int i;
                for (i = 0; i < a->attrValuesNumber; i++) {
                    PY_LONG_LONG v = *((SaInt64T*)a->attrValues[i]);
                    if (PyList_Append(vlist, Py_BuildValue("L", v)) < 0) 
                        return NULL;
                }
            }
            break;
        }
        case SA_IMM_ATTR_SATIMET:
        case SA_IMM_ATTR_SAUINT64T: {
            if (a->attrValuesNumber > 0) {
                int i;
                for (i = 0; i < a->attrValuesNumber; i++) {
                    unsigned PY_LONG_LONG v = *((SaUint64T*)a->attrValues[i]);
                    if (PyList_Append(vlist, Py_BuildValue("K", v)) < 0) 
                        return NULL;
                }
            }
            break;
        }
        case SA_IMM_ATTR_SAFLOATT:
        case SA_IMM_ATTR_SADOUBLET:
        case SA_IMM_ATTR_SAANYT:
		/* NOT YET IMPLEMENTED */
		break;
        }
	return vlist;
}

/* ----------------------------------------------------------------------
 * Sub-commands;
 */

static PyObject *
immom_saImmOmInitialize(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    SaVersionT immVer;

    if (immOmHandle) {
        /* Already Initialized */
        return immom_aisException(SA_AIS_ERR_BAD_OPERATION);
    }
    immVer = immVersion;
    rc = saImmOmInitialize(&immOmHandle, NULL, /*in-out*/&immVer);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    Py_RETURN_NONE;
}

static PyObject *
immom_saImmOmFinalize(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    rc = saImmOmFinalize(immOmHandle);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    immOmHandle = 0;
    Py_RETURN_NONE;
}

static PyObject *
immom_saImmOmSearchSublevel(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    SaNameT searchroot;
    SaImmSearchHandleT searchHandle;
    SaNameT objectName;
    const SaImmAttrValuesT_2** attr;
    PyObject* rlist;
    char const* root;

    if (!PyArg_ParseTuple(args, "s", &root))
        return NULL;
    if (immom_saName(root, &searchroot) == NULL)
        return NULL;

    rc = saImmOmSearchInitialize_2(
	immOmHandle, &searchroot, SA_IMM_SUBLEVEL,
	SA_IMM_SEARCH_GET_NO_ATTR, NULL, NULL, &searchHandle);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
 
    rlist = PyList_New(0);
    if (rlist == NULL) return NULL;

    rc = saImmOmSearchNext_2(
        searchHandle, &objectName, (SaImmAttrValuesT_2***)&attr);

    while (rc == SA_AIS_OK) {
        if (PyList_Append(rlist, Py_BuildValue("s", objectName.value)) < 0)
            return NULL; 
        rc = saImmOmSearchNext_2(
            searchHandle, &objectName, (SaImmAttrValuesT_2***)&attr);
    }

    (void)saImmOmSearchFinalize(searchHandle);
    return rlist;
}

static PyObject *
immom_saImmOmAccessorGet(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    SaImmAccessorHandleT accessorHandle;
    SaImmAttrValuesT_2** attributes;
    SaNameT searchroot;
    char const* root;
    PyObject* rlist;

    if (!PyArg_ParseTuple(args, "s", &root)) 
        return NULL;
    if (immom_saName(root, &searchroot) == NULL)
        return NULL;

    rc = saImmOmAccessorInitialize(immOmHandle, &accessorHandle);
    if (rc != SA_AIS_OK) return immom_aisException(rc);

    rc = saImmOmAccessorGet_2(
        accessorHandle, &searchroot, NULL, &attributes);
    if (rc != SA_AIS_OK) {
        (void)saImmOmAccessorFinalize(accessorHandle);
        return immom_aisException(rc);
    }

    rlist = PyList_New(0);
    if (rlist == NULL) return NULL;

    while (*attributes != NULL) {
        SaImmAttrValuesT_2* a = *attributes;
        char const* t = valuetype2str(a->attrValueType);
	PyObject* aval = Py_BuildValue("(ss[])", a->attrName, t);
	if (immom_makeValueList(a, PyTuple_GetItem(aval, 2)) == NULL) 
		return NULL;
	if (PyList_Append(rlist, aval) < 0) return NULL;
        attributes++;
    }

    (void)saImmOmAccessorFinalize(accessorHandle);
    return rlist;
}

static PyObject *
immom_saImmOmClassDescriptionGet(PyObject *self, PyObject *args)
{
    SaImmClassCategoryT classCategory;
    SaImmAttrDefinitionT_2 **attrDefinitions;
    char* className;
    SaAisErrorT rc;
    PyObject* rtuple;
    PyObject* alist;
    SaImmAttrDefinitionT_2 **ap;

    if (!PyArg_ParseTuple(args, "s", &className)) 
        return NULL;

    rc = saImmOmClassDescriptionGet_2(
        immOmHandle, className, &classCategory, &attrDefinitions);
    if (rc != SA_AIS_OK) return immom_aisException(rc);

    
    if (classCategory == SA_IMM_CLASS_CONFIG) {
        rtuple = Py_BuildValue("(s[])", "CONFIG");
    } else {
        rtuple = Py_BuildValue("(s[])", "RUNTIME");
    }
    alist = PyTuple_GetItem(rtuple, 1);
    for (ap = attrDefinitions; *ap != NULL; ap++) {
        SaImmAttrDefinitionT_2* a = *ap;
	PyObject* atuple = Py_BuildValue(
		"(ssK[])", a->attrName, valuetype2str(a->attrValueType),
		a->attrFlags);
	if (a->attrDefaultValue != NULL) {
		SaImmAttrValueT va[1];
		SaImmAttrValuesT_2 v;
		v.attrValueType = a->attrValueType;
		v.attrValuesNumber = 1;
		v.attrValues = va;
		va[0] = a->attrDefaultValue;
		(void)immom_makeValueList(&v, PyTuple_GetItem(atuple,3));
	}
        if (PyList_Append(alist, atuple) < 0) return NULL;
    }

    saImmOmClassDescriptionMemoryFree_2(immOmHandle, attrDefinitions);
    return rtuple;
}

static PyObject *
immom_saImmOmClassCreate(PyObject *self, PyObject *args)
{
    char* className;
    char* categoryStr;
    SaImmClassCategoryT classCategory;
    PyObject* alist;
    unsigned int len, i;
    SaImmAttrDefinitionT_2** attrDefinitions;
    SaAisErrorT rc;

    if (!PyArg_ParseTuple(args, "ssO", &className, &categoryStr, &alist)) 
        return NULL;
    if (!PyList_Check(alist))
        return immom_aisException(SA_AIS_ERR_INVALID_PARAM);
    if (strcmp(categoryStr, "CONFIG") == 0) {
        classCategory = SA_IMM_CLASS_CONFIG;
    } else if (strcmp(categoryStr, "RUNTIME") == 0) {
        classCategory = SA_IMM_CLASS_RUNTIME;
    } else {
        return immom_aisException(SA_AIS_ERR_INVALID_PARAM);
    }

    len = PyList_Size(alist);
    attrDefinitions = (SaImmAttrDefinitionT_2**)
        memget(sizeof(SaImmAttrDefinitionT_2*) * (len+1));
    attrDefinitions[len] = NULL;

    for (i = 0; i < len; i++) {
        PyObject* item = PyList_GetItem(alist, i);
        char* attrName;
        char* attrType;
        unsigned PY_LONG_LONG flags;
	PyObject* def;
        SaImmAttrDefinitionT_2* ad;

        if (!PyTuple_Check(item))
            return immom_aisException(SA_AIS_ERR_INVALID_PARAM);
        if (!PyArg_ParseTuple(item, "ssKO", &attrName, &attrType, &flags,&def))
            return immom_return_null();
	if (!PyList_Check(def))
	    return immom_aisException(SA_AIS_ERR_INVALID_PARAM);
        ad = (SaImmAttrDefinitionT_2*)memget(sizeof(SaImmAttrDefinitionT_2));
        ad->attrName = attrName;
        ad->attrValueType = immom_parseTypeStr(attrType);
        if ((int)ad->attrValueType == 0)
            return immom_aisException(SA_AIS_ERR_INVALID_PARAM);
        ad->attrFlags = flags;
	ad->attrDefaultValue = NULL;
	if (PyList_Size(def) > 0) {
		SaImmAttrValuesT_2* attr = memget(sizeof(SaImmAttrValuesT_2));
		if (immom_parseAttrValue(attr, attrName, attrType, def)== NULL)
			immom_return_null();
		ad->attrDefaultValue = attr->attrValues[0];
	}
        attrDefinitions[i] = ad;
    }

    rc = saImmOmClassCreate_2(
        immOmHandle, className, classCategory, 
        (const SaImmAttrDefinitionT_2**)attrDefinitions);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    return immom_return_None();
}

static PyObject *
immom_saImmOmClassDelete(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    char* className;
    if (!PyArg_ParseTuple(args, "s", &className)) 
        return NULL;
    rc = saImmOmClassDelete(immOmHandle, className);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    Py_RETURN_NONE;
}

static PyObject*
immom_saImmOmAdminOwnerInitialize(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    char* adminName;

    if (haveAdminOwner) 
        return immom_aisException(SA_AIS_ERR_BAD_OPERATION);
    if (!PyArg_ParseTuple(args, "s", &adminName))
        return NULL;
    if (strlen(adminName) > SA_MAX_NAME_LENGTH) 
        return immom_aisException(SA_AIS_ERR_NAME_TOO_LONG);

    rc = saImmOmAdminOwnerInitialize(
        immOmHandle, adminName, SA_TRUE, &adminOwnerHandle);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    haveAdminOwner = 1;
    Py_RETURN_NONE;
}

static PyObject*
immom_saImmOmAdminOwnerFinalize(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    if (!haveAdminOwner) 
        return immom_aisException(SA_AIS_ERR_BAD_OPERATION);
    rc = saImmOmAdminOwnerFinalize(adminOwnerHandle);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    haveAdminOwner = 0;
    haveCcb = 0;
    Py_RETURN_NONE;
}

static PyObject*
immom_saImmOmAdminOwnerSet(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    char const* scopeStr;
    PyObject* nameList;
    SaImmScopeT scope;
    SaNameT** objectNames;

    if (!haveAdminOwner) 
        return immom_aisException(SA_AIS_ERR_BAD_OPERATION);
    if (!PyArg_ParseTuple(args, "sO", &scopeStr, &nameList)) 
        return NULL;
    scope = immom_SaImmScope(scopeStr);
    if ((int)scope == 0) return NULL;
    if (!PyList_Check(nameList))
        return immom_aisException(SA_AIS_ERR_INVALID_PARAM);

    objectNames = immom_parseNames(nameList);
    rc = saImmOmAdminOwnerSet(
        adminOwnerHandle, (const SaNameT**)objectNames, scope);
    if (rc != SA_AIS_OK) return immom_aisException(rc);

    return immom_return_None();
}

static PyObject*
immom_saImmOmAdminOwnerClear(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    char const* scopeStr;
    PyObject* nameList;
    SaImmScopeT scope;
    SaNameT** objectNames;

    if (!PyArg_ParseTuple(args, "sO", &scopeStr, &nameList)) 
        return NULL;
    scope = immom_SaImmScope(scopeStr);
    if ((int)scope == 0) return NULL;

    objectNames = immom_parseNames(nameList);
    rc = saImmOmAdminOwnerClear(
        immOmHandle, (const SaNameT**)objectNames, scope);
    if (rc != SA_AIS_OK) return immom_aisException(rc);

    return immom_return_None();
}

static PyObject*
immom_saImmOmCcbInitialize(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    unsigned int flag;
    if (!haveAdminOwner || haveCcb)
        return immom_aisException(SA_AIS_ERR_BAD_OPERATION);
    if (!PyArg_ParseTuple(args, "I", &flag))
        return NULL;
    rc = saImmOmCcbInitialize(adminOwnerHandle, flag, &ccbHandle);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    haveCcb = 1;
    Py_RETURN_NONE;
}

static PyObject*
immom_saImmOmCcbApply(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    if (!haveCcb) 
        return immom_aisException(SA_AIS_ERR_NOT_EXIST);
    rc = saImmOmCcbApply(ccbHandle);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    Py_RETURN_NONE;
}

static PyObject*
immom_saImmOmCcbFinalize(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    if (!haveCcb) 
        return immom_aisException(SA_AIS_ERR_NOT_EXIST);
    rc = saImmOmCcbFinalize(ccbHandle);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    haveCcb = 0;
    Py_RETURN_NONE;
}

static PyObject*
immom_saImmOmCcbObjectDelete(PyObject *self, PyObject *args)
{
    SaNameT objectName;
    char const* dn;
    SaAisErrorT rc;

    if (!haveCcb) 
        return immom_aisException(SA_AIS_ERR_BAD_OPERATION);
    if (!PyArg_ParseTuple(args, "s", &dn)) 
        return NULL;
    if (immom_saName(dn, &objectName) == NULL)
        return NULL;
    rc = saImmOmCcbObjectDelete(ccbHandle, &objectName);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    return immom_return_None();
}

static PyObject*
immom_saImmOmCcbObjectCreate(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    char* className;
    char* parentStr;
    PyObject* attrList;
    SaNameT parentName;
    unsigned int len, i;
    SaImmAttrValuesT_2** attrValues;

    if (!haveCcb) 
        return immom_aisException(SA_AIS_ERR_BAD_OPERATION);
    if (!PyArg_ParseTuple(args, "ssO", &parentStr, &className, &attrList)) 
        return NULL;
    if (immom_saName(parentStr, &parentName) == NULL)
        return NULL;
    if (!PyList_Check(attrList))
        return immom_aisException(SA_AIS_ERR_INVALID_PARAM);

    len = PyList_Size(attrList);
    attrValues = (SaImmAttrValuesT_2**)memget(
        sizeof(SaImmAttrValuesT_2*) * (len+1));
    attrValues[len] = NULL;

    for (i = 0; i < len; i++) {
        PyObject* item = PyList_GetItem(attrList, i);
        char* attrName;
        char* attrType;
        PyObject* valueList;
        SaImmAttrValuesT_2* av;

        if (!PyTuple_Check(item))
            return immom_aisException(SA_AIS_ERR_INVALID_PARAM);
        if (!PyArg_ParseTuple(item, "ssO", &attrName, &attrType, &valueList)) 
            return immom_return_null();
        if (!PyList_Check(valueList))
            return immom_aisException(SA_AIS_ERR_INVALID_PARAM);

        av = (SaImmAttrValuesT_2*)memget(sizeof(SaImmAttrValuesT_2));

        if (immom_parseAttrValue(av, attrName, attrType, valueList) == NULL)
            return immom_return_null();

        attrValues[i] = av;
    }

    rc = saImmOmCcbObjectCreate_2(
        ccbHandle, className, &parentName, 
        (const SaImmAttrValuesT_2**)attrValues);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
    return immom_return_None();
}

static PyObject*
immom_saImmOmCcbObjectModify(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    PyObject* attrList;
    unsigned int len, i;
    char* dn;
    SaNameT objectName;
    SaImmAttrModificationT_2** attrValues;

    if (!haveCcb) 
        return immom_aisException(SA_AIS_ERR_BAD_OPERATION);
    if (!PyArg_ParseTuple(args, "sO", &dn, &attrList)) 
        return NULL;
    if (immom_saName(dn, &objectName) == NULL)
        return NULL;
    if (!PyList_Check(attrList))
        return immom_aisException(SA_AIS_ERR_INVALID_PARAM);

    len = PyList_Size(attrList);
    attrValues = (SaImmAttrModificationT_2**)memget(
        sizeof(SaImmAttrModificationT_2*) * (len+1));
    attrValues[len] = NULL;

    for (i = 0; i < len; i++) {
        PyObject* item = PyList_GetItem(attrList, i);
        char* attrName;
        char* attrType;
        PyObject* valueList;
        SaImmAttrModificationT_2* mv;

        if (!PyTuple_Check(item))
            return immom_aisException(SA_AIS_ERR_INVALID_PARAM);
        if (!PyArg_ParseTuple(item, "ssO", &attrName, &attrType, &valueList)) 
            return immom_return_null();
        if (!PyList_Check(valueList))
            return immom_aisException(SA_AIS_ERR_INVALID_PARAM);

        mv = (SaImmAttrModificationT_2*)memget(
            sizeof(SaImmAttrModificationT_2));
        if (immom_parseAttrValue(
                &(mv->modAttr), attrName, attrType, valueList) == NULL)
            return immom_return_null();
        mv->modType = SA_IMM_ATTR_VALUES_REPLACE;

        attrValues[i] = mv;
    }

    rc = saImmOmCcbObjectModify_2(
        ccbHandle, &objectName, (const SaImmAttrModificationT_2**)attrValues);
    if (rc != SA_AIS_OK) {
        return immom_aisException(rc);
    }
    return immom_return_None();
}

static PyObject*
immom_saImmOmAdminOperationInvoke(PyObject *self, PyObject *args)
{
    SaAisErrorT rc, oprc;
    PyObject* attrList;
    unsigned int len, i;
    char* dn;
    unsigned long long op;
    SaNameT objectName;
    SaImmAdminOperationParamsT_2** params;

    if (!haveAdminOwner) 
        return immom_aisException(SA_AIS_ERR_BAD_OPERATION);
    if (!PyArg_ParseTuple(args, "sLO", &dn, &op, &attrList))
        return NULL;
    if (immom_saName(dn, &objectName) == NULL)
        return NULL;
    if (!PyList_Check(attrList))
        return immom_aisException(SA_AIS_ERR_INVALID_PARAM);

    len = PyList_Size(attrList);
    params = (SaImmAdminOperationParamsT_2**)memget(
        sizeof(SaImmAdminOperationParamsT_2*) * (len+1));
    params[len] = NULL;

    for (i = 0; i < len; i++) {
        PyObject* item = PyList_GetItem(attrList, i);
        char* attrName;
        char* attrType;
        PyObject* valueList;
        SaImmAttrValuesT_2 av;
        SaImmAdminOperationParamsT_2* p;

        if (!PyTuple_Check(item))
            return immom_aisException(SA_AIS_ERR_INVALID_PARAM);
        if (!PyArg_ParseTuple(item, "ssO", &attrName, &attrType, &valueList)) 
            return immom_return_null();
        if (!PyList_Check(valueList))
            return immom_aisException(SA_AIS_ERR_INVALID_PARAM);

        if (immom_parseAttrValue(
                &av, attrName, attrType, valueList) == NULL)
            return immom_return_null();
        if (av.attrValuesNumber != 1)
            return immom_aisException(SA_AIS_ERR_INVALID_PARAM);

        p = (SaImmAdminOperationParamsT_2*)
            memget(sizeof(SaImmAdminOperationParamsT_2));
        p->paramName = av.attrName;
        p->paramType = av.attrValueType;
        p->paramBuffer = av.attrValues[0];
        params[i] = p;
    }

    rc = saImmOmAdminOperationInvoke_2(
        adminOwnerHandle, &objectName, 0ULL, op, 
        (SaImmAdminOperationParamsT_2 const**)params, &oprc, 
        SA_TIME_ONE_MINUTE);
    if (rc != SA_AIS_OK)
        return immom_aisException(rc);
    if (oprc != SA_AIS_OK)
        return immom_aisException(oprc);

    return immom_return_None();
}


static PyObject *
immom_saImmOmInstanceOf(PyObject *self, PyObject *args)
{
    SaAisErrorT rc;
    SaNameT searchroot;
    SaImmSearchHandleT searchHandle;
    SaNameT objectName;
    const SaImmAttrValuesT_2** attr;
    PyObject* rlist;
    char const* root;
    char const* classname;
    SaImmSearchParametersT_2 searchParam;

    if (!PyArg_ParseTuple(args, "ss", &root, &classname))
        return NULL;
    if (immom_saName(root, &searchroot) == NULL)
        return NULL;

    searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
    searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
    searchParam.searchOneAttr.attrValue = &classname;

    rc = saImmOmSearchInitialize_2(
	immOmHandle, &searchroot, SA_IMM_SUBTREE,
	SA_IMM_SEARCH_GET_NO_ATTR | SA_IMM_SEARCH_ONE_ATTR, 
	&searchParam, NULL, &searchHandle);
    if (rc != SA_AIS_OK) return immom_aisException(rc);
 
    rlist = PyList_New(0);
    if (rlist == NULL) return NULL;

    rc = saImmOmSearchNext_2(
        searchHandle, &objectName, (SaImmAttrValuesT_2***)&attr);

    while (rc == SA_AIS_OK) {
        if (PyList_Append(rlist, Py_BuildValue("s", objectName.value)) < 0)
            return NULL; 
        rc = saImmOmSearchNext_2(
            searchHandle, &objectName, (SaImmAttrValuesT_2***)&attr);
    }

    (void)saImmOmSearchFinalize(searchHandle);
    return rlist;
}



/* ----------------------------------------------------------------------
 * Python init
 */

static PyMethodDef ImmomMethods[] = {
    {"saImmOmInitialize", immom_saImmOmInitialize, METH_VARARGS,
     "Initialize the IMM Object Manager interface."},
    {"saImmOmFinalize", immom_saImmOmFinalize, METH_VARARGS,
     "Finalize the IMM Object Manager interface."},
    {"saImmOmSearchSublevel", immom_saImmOmSearchSublevel, METH_VARARGS,
     "Search one level the IMM object tree."},
    {"saImmOmAccessorGet", immom_saImmOmAccessorGet, METH_VARARGS,
     "Read attributes of an object."},
    {"saImmOmClassDescriptionGet", immom_saImmOmClassDescriptionGet, 
     METH_VARARGS, "Get a Class description."},
    {"saImmOmClassCreate", immom_saImmOmClassCreate, 
     METH_VARARGS, "Create a Class description."},
    {"saImmOmClassDelete", immom_saImmOmClassDelete, 
     METH_VARARGS, "Delete a Class description."},
    {"saImmOmAdminOwnerInitialize", immom_saImmOmAdminOwnerInitialize,
     METH_VARARGS, "Initialize an Admin Owner"},
    {"saImmOmAdminOwnerFinalize", immom_saImmOmAdminOwnerFinalize,
     METH_VARARGS, "Finalize an Admin Owner"},
    {"saImmOmAdminOwnerSet", immom_saImmOmAdminOwnerSet,
     METH_VARARGS, "Set Admin Owner for objects"},
    {"saImmOmAdminOwnerClear", immom_saImmOmAdminOwnerClear,
     METH_VARARGS, "Clear Admin Owner for objects"},
    {"saImmOmCcbInitialize", immom_saImmOmCcbInitialize, METH_VARARGS,
     "Initialize a CCB"},
    {"saImmOmCcbApply", immom_saImmOmCcbApply, METH_VARARGS,
     "Apply a CCB"},
    {"saImmOmCcbFinalize", immom_saImmOmCcbFinalize, METH_VARARGS,
     "Finalize a CCB"},
    {"saImmOmCcbObjectDelete", immom_saImmOmCcbObjectDelete, METH_VARARGS,
     "Delete an object"},
    {"saImmOmCcbObjectCreate", immom_saImmOmCcbObjectCreate, METH_VARARGS,
     "Create an object"},
    {"saImmOmCcbObjectModify", immom_saImmOmCcbObjectModify, METH_VARARGS,
     "Modify an object"},
    {"saImmOmInstanceOf", immom_saImmOmInstanceOf, METH_VARARGS,
     "Modify an object"},
    {"saImmOmAdminOperationInvoke", immom_saImmOmAdminOperationInvoke,
     METH_VARARGS, "Invoke an Administrative Operation"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initimmombin(void)
{ 
    PyObject* m;
    m = Py_InitModule("immombin", ImmomMethods);
    aisException = PyErr_NewException("immombin.AisException", NULL, NULL);
    Py_INCREF(aisException);
    PyModule_AddObject(m, "AisException", aisException);
}


/*
 * Local Variables:
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * End:
 */
