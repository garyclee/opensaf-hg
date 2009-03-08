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

#include <iostream>
#include <list>
#include <map>
#include <string>
#include <libxml/parser.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <syslog.h>
#include <configmake.h>

#include <saImmOm.h>
#include <immsv_api.h>
#include <saAis.h>
#include <logtrace.h>

#define MAX_DEPTH 10
#define MAX_CHAR_BUFFER_SIZE 200

static bool opensafClassCreated=false;
static bool opensafObjectCreated=false;

/* The possible states of the parser */
typedef enum
{
    STARTED,
    DONE,
    CLASS,
    OBJECT,
    DN,
    ATTRIBUTE,
    NAME,
    VALUE,
    CATEGORY,
    RDN,
    DEFAULT_VALUE,
    TYPE,
    FLAG,
    NTFID,
    IMM_CONTENTS
} StatesEnum;

/* The state struct for the parser */
typedef struct ParserStateStruct
{
    int                  immInit;
    int                  adminInit;
    int                  ccbInit;

    int                  doneParsingClasses;
    int                  depth;
    StatesEnum           state[MAX_DEPTH];

    char*                className;
    SaImmClassCategoryT  classCategory;
    int                  classCategorySet;

    /* AttrDefinition parameters */
    char*                attrName;
    SaImmValueTypeT      attrValueType;
    SaImmAttrFlagsT      attrFlags;
    SaUint32T            attrNtfId;
    char*                attrDefaultValueBuffer;

    int                  attrValueTypeSet;
    int                  attrNtfIdSet;
    int                  attrDefaultValueSet;
    int                  attrValueSet;

    std::list<SaImmAttrDefinitionT_2> attrDefinitions;
    std::map<std::string, std::map<std::string, SaImmValueTypeT> > classAttrTypeMap;

    /* Object parameters */
    char*                objectClass;
    char*                objectName;

    std::list<SaImmAttrValuesT_2> attrValues;
    std::list<char*>            attrValueBuffers;
    std::map<std::string, SaImmAttrValuesT_2>    classRDNMap;

    SaImmHandleT         immHandle;
    SaImmHandleT         ownerHandle;
    SaImmHandleT         ccbHandle;
} ParserState;

SaImmCallbacksT immCallbacks = 
{
};

/* Prototypes */

/* Helper functions */
static void addClassAttributeDefinition(ParserState* state);
static void addObjectAttributeDefinition(ParserState* state);
static void addToAttrTypeCache(ParserState*, SaImmValueTypeT);
static SaImmValueTypeT getClassAttrValueType(ParserState*, 
                                             const char*, 
                                             const char*);
static void saveRDNAttribute(ParserState* parserState);
static void getDNForClass(ParserState*, 
                          const SaImmClassNameT,
                          SaImmAttrValuesT_2*);
static void charsToValueHelper(SaImmAttrValueT*, 
                               SaImmValueTypeT, 
                               const char*);
static SaImmValueTypeT charsToTypeHelper(const xmlChar* str, size_t len);
static SaImmAttrFlagsT charsToFlagsHelper(const xmlChar* str, size_t len);

static void createImmClass(ParserState*);
static void createImmObject(ParserState*);


/* SAX callback handlers */
static char* getAttributeValue(const char* attr, const xmlChar** const attrArray);

static void errorHandler(void*, const char*, ...);

static void warningHandler(void*, const char*, ...);

static void startElementHandler(void*, const xmlChar*, const xmlChar**);

static void endElementHandler(void*, const xmlChar*);

static void startDocumentHandler(void* userData);

static void endDocumentHandler(void* userData);

static void attributeDeclHandler(void*,
                                 const xmlChar*,
                                 const xmlChar*,
                                 int,
                                 int,
                                 const xmlChar*,
                                 xmlEnumerationPtr);

static void charactersHandler(void*, const xmlChar*, int);

static xmlEntityPtr getEntityHandler(void*, const xmlChar*);

/* Data declarations */
xmlSAXHandler my_handler = {
    NULL,                  //   internalSubsetSAXFunc internalSubset;
    NULL,                  //   isStandaloneSAXFunc isStandalone,
    NULL,                  //   hasInternalSubsetSAXFunc hasInternalSubset,
    NULL,                  //   hasExternalSubsetSAXFunc hasExternalSubset,
    NULL,                  //   resolveEntitySAXFunc resolveEntity,
    getEntityHandler,
    NULL,                  //   entityDeclSAXFunc entityDecl,
    NULL,                  //   notationDeclSAXFunc notationDecl,
    attributeDeclHandler,
    NULL,                  //   elementDeclSAXFunc elementDecl,
    NULL,                  //   unparsedEntityDeclSAXFunc unparsedEntityDecl,
    NULL,                  //   setDocumentLocatorSAXFunc setDocumentLocator,
    startDocumentHandler,
    endDocumentHandler,
    startElementHandler,
    endElementHandler,
    NULL,                  //   referenceSAXFunc reference,
    charactersHandler,
    NULL,                  //   ignorableWhitespaceSAXFunc ignorableWhitespace,
    NULL,                  //   processingInstructionSAXFunc processingInstruction,
    NULL,                  //   commentSAXFunc comment,
    warningHandler,
    errorHandler,
    NULL                   //   fatalErrorSAXFunc fatalError;
};


/* Function bodies */

void opensafClassCreate(SaImmHandleT immHandle)
{
    SaAisErrorT err = SA_AIS_OK;
    int retries=0;
    SaImmAttrDefinitionT_2 d1, d2, d3;

    d1.attrName = OPENSAF_IMM_ATTR_RDN;
    d1.attrValueType = SA_IMM_ATTR_SANAMET;
    d1.attrFlags = SA_IMM_ATTR_CONFIG | SA_IMM_ATTR_RDN | SA_IMM_ATTR_INITIALIZED;
    d1.attrDefaultValue = NULL;

    d2.attrName = OPENSAF_IMM_ATTR_EPOCH;
    d2.attrValueType = SA_IMM_ATTR_SAUINT32T;
    d2.attrFlags = SA_IMM_ATTR_RUNTIME | SA_IMM_ATTR_CACHED | SA_IMM_ATTR_PERSISTENT;
    d2.attrDefaultValue = NULL;

    d3.attrName = OPENSAF_IMM_ATTR_CLASSES;
    d3.attrValueType = SA_IMM_ATTR_SASTRINGT;
    d3.attrFlags = SA_IMM_ATTR_RUNTIME | SA_IMM_ATTR_CACHED | SA_IMM_ATTR_PERSISTENT | 
        SA_IMM_ATTR_MULTI_VALUE;
    d3.attrDefaultValue = NULL;

    const SaImmAttrDefinitionT_2* attrDefs[4] = {&d1, &d2, &d3, 0};


    do {/* Create the class */
        
        if(err == SA_AIS_ERR_TRY_AGAIN) {
            TRACE_8("Got TRY_AGAIN (retries:%u) on class create",
                retries);
            usleep(500);
            err = SA_AIS_OK;
        }
        err = saImmOmClassCreate_2(
                                   immHandle, 
                                   OPENSAF_IMM_CLASS_NAME,
                                   SA_IMM_CLASS_CONFIG,
                                   attrDefs);

    } while(err == SA_AIS_ERR_TRY_AGAIN && ++retries < 32);

    if(err != SA_AIS_OK)
    {
        LOG_ER("Failed to create the class %s", OPENSAF_IMM_CLASS_NAME);
        exit(1);
    }
}


static void opensafObjectCreate(SaImmCcbHandleT ccbHandle)
{
    SaAisErrorT err = SA_AIS_OK;
    int retries=0;
    SaNameT rdn;
    SaNameT parent;
    rdn.length = strlen(OPENSAF_IMM_OBJECT_RDN);
    strcpy((char *) rdn.value, OPENSAF_IMM_OBJECT_RDN);
    SaNameT*     nameValues[1];
    nameValues[0] = &rdn;

    parent.length = strlen(OPENSAF_IMM_OBJECT_PARENT);
    strcpy((char *) parent.value, OPENSAF_IMM_OBJECT_PARENT);

    SaUint32T epochValue=1;
    SaUint32T* intValues[1];
    intValues[0] = &epochValue;

    SaImmAttrValuesT_2 v1, v2;
    v1.attrName = OPENSAF_IMM_ATTR_RDN;
    v1.attrValueType = SA_IMM_ATTR_SANAMET;
    v1.attrValuesNumber = 1;
    v1.attrValues = (void**)nameValues;

    v2.attrName = OPENSAF_IMM_ATTR_EPOCH;
    v2.attrValueType = SA_IMM_ATTR_SAUINT32T;
    v2.attrValuesNumber = 1;
    v2.attrValues = (void**)intValues;

    const SaImmAttrValuesT_2* attrValues[3] = {&v1,&v2,0};

    do {/* Create the object */
        
        if(err == SA_AIS_ERR_TRY_AGAIN) {
            TRACE_8("Got TRY_AGAIN (retries:%u) on class create",
                retries);
            usleep(500);
            err = SA_AIS_OK;
        }

        err = saImmOmCcbObjectCreate_2(
                                   ccbHandle, 
                                   OPENSAF_IMM_CLASS_NAME, 
                                   &parent,
                                   attrValues);

    } while(err == SA_AIS_ERR_TRY_AGAIN && ++retries < 32);

    if(err != SA_AIS_OK)
    {
        LOG_ER("Failed to create the object %s", OPENSAF_IMM_OBJECT_DN);
        exit(1);
    }
}


/**
 * Creates an Imm Object through the ImmOm interface
 */
static void createImmObject(ParserState* state)
{
    SaImmClassNameT className;
    SaNameT parentName;
    SaImmAttrValuesT_2** attrValues;
    SaAisErrorT errorCode;
    int i;
    size_t DNlen;

    TRACE_8("CREATE IMM OBJECT %s, %s", 
            state->objectClass,
            state->objectName);

    /* Set the class name */
    className = state->objectClass;

    /* Set the parent name */
    parentName.length = 0;
    if (state->objectName != NULL)
    {
        char* parent;

        /* ',' is the delimeter */
        parent = strchr(state->objectName, ',');

        if (parent == NULL || strlen(parent) <= 1)
        {
            parent = NULL;
        }
        else
        {
            parent++;
            TRACE_8("PARENT %s\n", parent);
        }

        if (parent != NULL)
        {
            parentName.length = (SaUint16T)strlen(parent);
            strncpy((char*)parentName.value, parent, parentName.length);
        }
    }

    /* Get the length of the DN and truncate state->objectName */
    if (parentName.length > 0)
    {
        DNlen = strlen(state->objectName) - (parentName.length + 1);
    }
    else
    {
        DNlen = strlen(state->objectName);
    }

    state->objectName[DNlen] = '\0';

    TRACE_8("OBJECT NAME: %s", state->objectName);

    /* Set the attribute values array, add space for the rdn attribute
     * and a NULL terminator */

    /* Freed at the bottom of the function */
    attrValues = (SaImmAttrValuesT_2**)
                 malloc((state->attrValues.size() + 2)
                        * sizeof(SaImmAttrValuesT_2*));
    if (attrValues == NULL)
    {
        LOG_ER("Failed to malloc attrValues");
        return;
    }

    /* Add the NULL termination */
    attrValues[state->attrValues.size() + 1] = NULL; /* Adjust for RDN */

    std::list<SaImmAttrValuesT_2>::iterator it =
        state->attrValues.begin();

    i = 0;
    while (it != state->attrValues.end())
    {
        attrValues[i] = &(*it);

        i++;
        it++;
    }

    attrValues[i] = (SaImmAttrValuesT_2*)malloc(sizeof(SaImmAttrValuesT_2));
    getDNForClass(state, className, attrValues[i]);

    
    int retries=0;
    do {/* Do the object creation */
        
        if(errorCode == SA_AIS_ERR_TRY_AGAIN) {
            TRACE_8("Got TRY_AGAIN (retries:%u) on object create",
                retries);
            usleep(500);
            errorCode = SA_AIS_OK;
        }

        errorCode = saImmOmCcbObjectCreate_2(state->ccbHandle,
                                         className,
                                         &parentName,
                                         (const SaImmAttrValuesT_2**)
                                         attrValues);

    } while(errorCode == SA_AIS_ERR_TRY_AGAIN && ++retries < 32);


    if (SA_AIS_OK != errorCode)
    {
        LOG_ER("Failed to create the imm om object err: %d", errorCode);
        exit(1);
    }

    if(!opensafObjectCreated && 
        strcmp(className, OPENSAF_IMM_CLASS_NAME)==0)
    {/*Assuming here that the instance is the one and only correct instance.*/
        opensafObjectCreated = true;
    }

    TRACE_8("CREATE DONE");

    /* Free used parameters */
    free(state->objectClass);
    state->objectClass = NULL;
    free(state->objectName);
    state->objectName = NULL;

    /* Free the DN attrName later since it's re-used */
    //    free(attrValues[i]->attrValues);
    //    free(attrValues);


    for (it = state->attrValues.begin();
        it != state->attrValues.end();
        it++)
    {
        free(it->attrName);
        free(it->attrValues);
    }
    state->attrValues.clear();
}

/**
 * Creates an ImmClass through the ImmOm interface
 */
static void createImmClass(ParserState* state)
{
    SaImmClassNameT          className;
    SaImmClassCategoryT      classCategory;
    SaImmAttrDefinitionT_2** attrDefinition;
    SaAisErrorT              errorCode;
    int i;

    TRACE_8("CREATING IMM CLASS %s", state->className);

    /* Set the name */
    className = state->className;

    /* Set the category */
    if (state->classCategorySet)
    {
        classCategory = state->classCategory;
    }
    else
    {
        LOG_ER("NO CLASS CATEGORY");
    }

    /* Set the attrDefinition array */
    attrDefinition = (SaImmAttrDefinitionT_2**)
                     calloc((state->attrDefinitions.size() + 1),
                            sizeof(SaImmAttrDefinitionT_2*));
    if (attrDefinition == NULL)
    {
        LOG_ER("Failed to malloc attrDefinition");
        return;
    }

    attrDefinition[state->attrDefinitions.size()] = NULL;

    std::list<SaImmAttrDefinitionT_2>::iterator it =
        state->attrDefinitions.begin();

    i = 0;
    while (it != state->attrDefinitions.end())
    {
        attrDefinition[i] = &(*it);

        i++;
        it++;
    }

    int retries=0;
    do {/* Create the class */
        
        if(errorCode == SA_AIS_ERR_TRY_AGAIN) {
            TRACE_8("Got TRY_AGAIN (retries:%u) on class create",
                retries);
            usleep(500);
            errorCode = SA_AIS_OK;
        }
        errorCode = saImmOmClassCreate_2(state->immHandle,
                                     className,
                                     classCategory,
                                     (const SaImmAttrDefinitionT_2**)
                                     attrDefinition);

    } while(errorCode == SA_AIS_ERR_TRY_AGAIN && ++retries < 32);

    if (SA_AIS_OK != errorCode)
    {
        LOG_ER("FAILED to create IMM class, %d", errorCode);
        exit(1);
    }

    if(!opensafClassCreated && 
        strcmp(className, OPENSAF_IMM_CLASS_NAME)==0)
    {
        opensafClassCreated = true;
    }

    TRACE_8("CREATED IMM CLASS %s %u", className, opensafClassCreated);

    /* Free all each attrDefinition */
    it = state->attrDefinitions.begin();

    while (it != state->attrDefinitions.end())
    {
        free(it->attrName);
        it->attrName = NULL;
        it++;
    }

    /* Free the attrDefinition array and empty the list */
    free(attrDefinition);
    state->attrDefinitions.clear();

    TRACE_8("<CREATE IMM CLASS");
}

/**
 * Returns an SaImmAttrValueT struct representing the DN for an object
 */
static void getDNForClass(ParserState* state,
                          const SaImmClassNameT className,
                          SaImmAttrValuesT_2* values)
{
    std::string classNameString;

    classNameString = std::string(className);

    if (state->classRDNMap.find(classNameString) == 
        state->classRDNMap.end())
    {
        LOG_ER("CLASS %s NOT FOUND", className);
        exit(1);
    }

    *values = state->classRDNMap[classNameString];

    values->attrValues = (SaImmAttrValueT*)malloc(sizeof(SaImmAttrValueT));

    values->attrValuesNumber = 1;

    charsToValueHelper(values->attrValues,
                       values->attrValueType,
                       state->objectName);
}


static void errorHandler(void* userData,
                         const char* msg,
                         ...)
{
    LOG_ER("Error occured during parsing: %s", msg);
}

static void warningHandler(void* userData,
                           const char* msg,
                           ...)
{
    LOG_WA("Warning occured during parsing: %s", msg);
}

/**
 * This is the handler for start tags
 */
static void startElementHandler(void* userData,
                                const xmlChar*  name,
                                const xmlChar** attrs)
{
    ParserState* state;    

    TRACE_8("TAG %s", name);

    state = (ParserState*) userData;
    state->depth++;

    if (state->depth >= MAX_DEPTH)
    {
        LOG_ER( "The document is too deply nested");
    }

    /* <class ...> */
    if (strcmp((const char*)name, "class") == 0)
    {
        char* nameAttr;

        if (state->doneParsingClasses)
        {
            LOG_ER("CLASS TAG AT INVALID PLACE IN XML");
            exit(1);
        }

        state->state[state->depth] = CLASS;
        nameAttr = getAttributeValue("name", attrs);

        if (nameAttr != NULL)
        {
            size_t len;

            len = strlen(nameAttr);

            state->className = (char*)malloc(len + 1);
            strcpy(state->className, nameAttr);
            state->className[len] = '\0';

            TRACE_8("CLASS NAME: %s", state->className);
        }
        else
        {
            LOG_ER( "NAME ATTRIBUTE IS NULL");
        }
        /* <object ...> */
    }
    else if (strcmp((const char*)name, "object") == 0)
    {
        char* classAttr;

        state->attrFlags = 0;

        state->state[state->depth] = OBJECT;

        /* Get the class attribute */
        classAttr = getAttributeValue("class", attrs);

        if (classAttr != NULL)
        {
            size_t len;

            len = strlen(classAttr);

            state->objectClass = (char*)malloc(len + 1);
            strncpy(state->objectClass, classAttr, len);
            state->objectClass[len] = '\0';

            TRACE_8("OBJECT CLASS NAME: %s", 
                    state->objectClass);
        }
        else
        {
            LOG_ER("OBJECT %s HAS NO CLASS ATTRIBUTE", state->objectName);
        }

        /* <dn> */
    }
    else if (strcmp((const char*)name, "dn") == 0)
    {
        state->state[state->depth] = DN;
        /* <attr> */
    }
    else if (strcmp((const char*)name, "attr") == 0)
    {
        state->state[state->depth] = ATTRIBUTE;

        state->attrFlags = 0x0;
        state->attrName  = NULL;
        state->attrDefaultValueBuffer = NULL;

        state->attrValueTypeSet    = 0;
        state->attrNtfIdSet        = 0;
        state->attrDefaultValueSet = 0;
        /* <name> */
    }
    else if (strcmp((const char*)name, "name") == 0)
    {
        state->state[state->depth] = NAME;
        /* <value> */
    }
    else if (strcmp((const char*)name, "value") == 0)
    {
        state->state[state->depth] = VALUE;
        /* <category> */
    }
    else if (strcmp((const char*)name, "category") == 0)
    {
        state->state[state->depth] = CATEGORY;
        /* <rdn> */
    }
    else if (strcmp((const char*)name, "rdn") == 0)
    {
        state->attrFlags        = SA_IMM_ATTR_RDN;

        state->attrName = NULL;

        state->attrDefaultValueBuffer = NULL;

        state->attrValueTypeSet    = 0;
        state->attrDefaultValueSet = 0;
        state->state[state->depth] = RDN;
        /* <default-value> */
    }
    else if (strcmp((const char*)name, "default-value") == 0)
    {
        state->state[state->depth] = DEFAULT_VALUE;
        /* <type> */
    }
    else if (strcmp((const char*)name, "type") == 0)
    {
        state->state[state->depth] = TYPE;
        /* <flag> */
    }
    else if (strcmp((const char*)name, "flag") == 0)
    {
        state->state[state->depth] = FLAG;
        /* <ntfid> */
    }
    else if (strcmp((const char*)name, "ntfid") == 0)
    {
        state->state[state->depth] = NTFID;
        /* <imm:IMM-contents> */
    }
    else if (strcmp((const char*)name, "imm:IMM-contents") == 0)
    {
        state->state[state->depth] = IMM_CONTENTS;
    }
    else
    {
        LOG_ER("UNKNOWN TAG! (%s)", name);
        exit(1);
    }
}

/**
 * Called when an end-tag is reached
 */
static void endElementHandler(void* userData,
                              const xmlChar* name)
{
    ParserState* state;

    state = (ParserState*)userData;

    TRACE_8("END %s", name);

    /* </value> */
    if (strcmp((const char*)name, "value") == 0)
    {
        if (state->attrValueBuffers.size() == 0)
        {
            char* str = (char*)malloc(1);

            str[0] = '\0';
            state->attrValueBuffers.push_front(str);
        }
        /* </default-value> */
    }
    else if (strcmp((const char*)name, "default-value") == 0)
    {
        if (state->attrDefaultValueBuffer == NULL ||
            !state->attrDefaultValueSet)
        {
            state->attrDefaultValueBuffer = (char*)malloc(1);

            state->attrDefaultValueBuffer[0] = '\0';
            state->attrDefaultValueSet = 1;
            TRACE_8("EMPTY DEFAULT-VALUE TAG");
            TRACE_8("Attribute: %s", state->attrName);
        }
        /* </class> */
    }
    else if (strcmp((const char*)name, "class") == 0)
    {
        if (state->doneParsingClasses)
        {
            LOG_ER("INVALID CLASS PLACEMENT");
            exit(1);
        }
        else
        {
            createImmClass(state);
            state->attrFlags = 0;

            state->attrValueTypeSet    = 0;
            state->attrNtfIdSet        = 0;
            state->attrDefaultValueSet = 0;
        }
        /* </attr> or </rdn> */
    }
    else if (strcmp((const char*)name, "attr") == 0 ||
             strcmp((const char*)name, "rdn") == 0)
    {
        if (state->state[state->depth - 1] == CLASS)
        {
            addClassAttributeDefinition(state);
        }
        else
        {
            addObjectAttributeDefinition(state);
        }
        /* </object> */
    }
    else if (strcmp((const char*)name, "object") == 0)
    {
        TRACE_8("END OBJECT");
        if (!state->doneParsingClasses)
        {
            TRACE_8("CCB INIT");
            SaAisErrorT errorCode;

            state->doneParsingClasses = 1;

            if(!opensafClassCreated) {
                LOG_NO("Creating the class %s since it is missing from the "
                       "imm.xml load file", OPENSAF_IMM_CLASS_NAME);
                opensafClassCreate(state->immHandle);
            }


            /* First time, initialize the imm object api */
            errorCode = saImmOmAdminOwnerInitialize(state->immHandle,
                                                    "IMMLOADER",
                                                    SA_FALSE,
                                                    &state->ownerHandle);
            if (errorCode != SA_AIS_OK)
            {
                LOG_ER("Failed on saImmOmAdminOwnerInitialize %d",
                       errorCode);
                exit(1);
            }
            state->adminInit = 1;

            /* ... and initialize the imm ccb api  */
            errorCode = saImmOmCcbInitialize(state->ownerHandle,
                                             0
                                             /*(SaImmCcbFlagsT)
                                               SA_IMM_CCB_REGISTERED_OI*/,
                                             &state->ccbHandle);
            if (errorCode != SA_AIS_OK)
            {
                LOG_ER("Failed to initialize ImmOmCcb %d", errorCode);
                exit(1);
            }
            state->ccbInit = 1;

        }

        /* Create the object */
        createImmObject(state);
        /* </imm:IMM-contents> */
    }
    else if (strcmp((const char*)name, "imm:IMM-contents") == 0)
    {
        SaAisErrorT errorCode;

        if(!opensafObjectCreated) {
            LOG_NO("Creating the %s object of class %s since it is missing "
                "from the imm.xml load file", OPENSAF_IMM_OBJECT_DN,
                OPENSAF_IMM_CLASS_NAME);
            opensafObjectCreate(state->ccbHandle);
        }

        /* Apply the object creations */
        if (state->ccbInit)
        {
            errorCode = saImmOmCcbApply(state->ccbHandle);
            if (SA_AIS_OK != errorCode)
            {
                LOG_ER("Failed to apply object creations %d", errorCode);
                exit(1);
            }
        }

        /* Finalize the ccb connection*/
        if (state->ccbInit)
        {
            errorCode = saImmOmCcbFinalize(state->ccbHandle);
            if (SA_AIS_OK != errorCode)
            {
                LOG_ER("Failed to finalize the ccb object connection %d",
                       errorCode);
            }
            else
            {
                state->ccbInit = 0;
            }
        }

        /* Finalize the owner connection */
        if (state->adminInit)
        {
            errorCode = saImmOmAdminOwnerFinalize(state->ownerHandle);
            if (SA_AIS_OK != errorCode)
            {
                LOG_ER("Failed on saImmOmAdminOwnerFinalize (%d)", 
                       errorCode);
            }
            else
            {
                state->adminInit = 0;
            }
        }

        /* Finalize the imm connection */
        if (state->immInit)
        {
            errorCode = saImmOmFinalize(state->immHandle);  
            if (SA_AIS_OK != errorCode)
            {
                LOG_ER("Failed on saImmOmFinalize (%d)", errorCode);
            }
            else
            {
                state->immInit = 0;
            }
        }
    }

    ((ParserState*)userData)->depth--;
}

static void startDocumentHandler(void* userData)
{
    ParserState* state;
    state = (ParserState*)userData;

    state->depth = 0;
    state->state[0] = STARTED;
}

static void endDocumentHandler(void* userData)
{
    if (((ParserState*)userData)->depth != 0)
    {
        LOG_ER( "Document ends too early\n");
    }
    TRACE_8("endDocument occured\n");
}

static void attributeDeclHandler(void *ctx,
                                 const xmlChar *elem,
                                 const xmlChar *fullname,
                                 int type,
                                 int def,
                                 const xmlChar *defaultValue,
                                 xmlEnumerationPtr tree)
{
    TRACE_8("attributeDecl occured\n");
}

static void charactersHandler(void* userData,
                              const xmlChar* chars,
                              int len)
{
    ParserState* state;
    std::list<char*>::iterator it;

    state = (ParserState*)userData;

    if (len > MAX_CHAR_BUFFER_SIZE)
    {
        LOG_ER("The character field is too big len(%d) > max(%d)",
            len, MAX_CHAR_BUFFER_SIZE);
        exit(1);
    }
    else if (len < 0)
    {
        LOG_ER("The character array length is negative %d", len);
        exit(1);
    }

    /* Treat each tag type separately */
    switch (state->state[state->depth])
    {
        case IMM_CONTENTS:
            break;
        case STARTED:
            break;
        case DONE:
            break;
        case CLASS:
            break;
        case OBJECT:
            break;
        case ATTRIBUTE:
            break;
        case DN:
            /* Copy the distinguished name */
            state->objectName = (char*)malloc((size_t)len + 1);

            strncpy(state->objectName, (const char*)chars, (size_t)len);

            state->objectName[len] = '\0';

            break;
        case NAME:
            /* The attrName must be NULL */
            assert(!state->attrName);

            if (state->state[state->depth - 1] == ATTRIBUTE ||
                state->state[state->depth - 1] == RDN)
            {
                state->attrName = (char*)malloc((size_t)len + 1);
                if (state->attrName == NULL)
                {
                    LOG_ER("Failed to malloc state->attrName");
                }

                strncpy(state->attrName, (const char*)chars, (size_t)len);
                state->attrName[len] = '\0';
            }
            else
            {
                LOG_ER("Name not immediately inside an attribute tag");
                exit(1);
            }
            break;
        case VALUE:
            if (state->state[state->depth - 1] == ATTRIBUTE)
            {
                char* str;

                str = (char*)malloc((size_t)len + 1);
                if (str == NULL)
                {
                    LOG_ER("Failed to malloc state->attrName");
                    exit(1);
                }

                strncpy(str, (const char*)chars, (size_t)len);
                str[len] = '\0';

                state->attrValueBuffers.push_front(str);
            }
            else
            {
                LOG_ER("UNKNOWN PLACEMENT OF VALUE");
                exit(1);
            }
            break;
        case CATEGORY:
            if (state->state[state->depth - 1] == CLASS)
            {
                SaImmClassCategoryT category;
                if (strncmp((const char*)chars, "SA_CONFIG", (size_t)len) == 0)
                {
                    category = SA_IMM_CLASS_CONFIG;
                }
                else
                {
                    category = SA_IMM_CLASS_RUNTIME;
                }

                state->classCategorySet = 1;
                state->classCategory = category;
            }
            else if (state->state[state->depth - 1] == ATTRIBUTE ||
                     state->state[state->depth - 1] == RDN)
            {
                SaImmAttrFlagsT category;
                if (strncmp((const char*)chars, "SA_CONFIG", (size_t)len) == 0)
                {
                    category = SA_IMM_ATTR_CONFIG;
                }
                else
                {
                    category = SA_IMM_ATTR_RUNTIME;
                }
                state->attrFlags = state->attrFlags | category;
            }

            break;
        case DEFAULT_VALUE:
            /* The defaultValueBuffer must be NULL */
            assert(!state->attrDefaultValueBuffer);

            if (state->state[state->depth - 1] == ATTRIBUTE)
            {
                state->attrDefaultValueBuffer = (char*)malloc((size_t)len + 1);
                strncpy(state->attrDefaultValueBuffer, 
                        (const char*)chars, 
                        (size_t)len);
                state->attrDefaultValueBuffer[len] = '\0';

                state->attrDefaultValueSet = 1;
            }
            else
            {
                LOG_ER("UNKNOWN PLACEMENT OF DEFAULT VALUE");
                exit(1);
            }
            break;
        case TYPE:
            state->attrValueType = charsToTypeHelper(chars, (size_t)len);
            state->attrValueTypeSet = 1;

            addToAttrTypeCache(state, state->attrValueType);

            break;
        case RDN:

            break;
        case FLAG:
            state->attrFlags = 
                state->attrFlags | charsToFlagsHelper(chars, (size_t)len);
            break;
        case NTFID:
            if (atoi((const char*)chars) < 0)
            {
                LOG_ER("NtfId can not be negative");
                exit(1);
            }

            state->attrNtfId = (SaUint32T)atoi((const char*)chars);
            state->attrNtfIdSet = 1;
            break;
        default:
            LOG_ER("Unknown state");
            exit(1);
    }

}

static xmlEntityPtr
    getEntityHandler(void *user_data, const xmlChar *name)
{
    return xmlGetPredefinedEntity(name);
}

/**
 * Takes a string and returns the corresponding flag
 */
static SaImmAttrFlagsT charsToFlagsHelper(const xmlChar* str, size_t len)
{
    if (strncmp((const char*)str, "SA_MULTI_VALUE", len) == 0)
    {
        return SA_IMM_ATTR_MULTI_VALUE;
    }
    else if (strncmp((const char*)str, "SA_RDN", len) == 0)
    {
        return SA_IMM_ATTR_RDN;
    }
    else if (strncmp((const char*)str, "SA_CONFIG", len) == 0)
    {
        return SA_IMM_ATTR_CONFIG;
    }
    else if (strncmp((const char*)str, "SA_WRITABLE", len ) == 0)
    {
        return SA_IMM_ATTR_WRITABLE;
    }
    else if (strncmp((const char*)str, "SA_INITIALIZED", len) == 0)
    {
        return SA_IMM_ATTR_INITIALIZED;
    }
    else if (strncmp((const char*)str, "SA_RUNTIME", len ) == 0)
    {
        return SA_IMM_ATTR_RUNTIME;
    }
    else if (strncmp((const char*)str, "SA_PERSISTENT", len ) == 0)
    {
        return SA_IMM_ATTR_PERSISTENT;
    }
    else if (strncmp((const char*)str, "SA_CACHED", len) == 0)
    {
        return SA_IMM_ATTR_CACHED;
    }

    LOG_ER("UNKNOWN FLAGS, %s", str);

    exit(1);
}

/**
 * Takes a string and returns the corresponding valueType
 */
static SaImmValueTypeT charsToTypeHelper(const xmlChar* str, size_t len)
{
    if (strncmp((const char*)str, "SA_NAME_T", len) == 0)
    {
        return SA_IMM_ATTR_SANAMET;
    }
    else if (strncmp((const char*)str, "SA_INT32_T", len ) == 0)
    {
        return SA_IMM_ATTR_SAINT32T;
    }
    else if (strncmp((const char*)str, "SA_UINT32_T", len) == 0)
    {
        return SA_IMM_ATTR_SAUINT32T;
    }
    else if (strncmp((const char*)str, "SA_INT64_T", len ) == 0)
    {
        return SA_IMM_ATTR_SAINT64T;
    }
    else if (strncmp((const char*)str, "SA_UINT64_T", len) == 0)
    {
        return SA_IMM_ATTR_SAUINT64T;
    }
    else if (strncmp((const char*)str, "SA_TIME_T", len  ) == 0)
    {
        return SA_IMM_ATTR_SATIMET;
    }
    else if (strncmp((const char*)str, "SA_NAME_T", len  ) == 0)
    {
        return SA_IMM_ATTR_SANAMET;
    }
    else if (strncmp((const char*)str, "SA_FLOAT_T", len ) == 0)
    {
        return SA_IMM_ATTR_SAFLOATT;
    }
    else if (strncmp((const char*)str, "SA_DOUBLE_T", len) == 0)
    {
        return SA_IMM_ATTR_SADOUBLET;
    }
    else if (strncmp((const char*)str, "SA_STRING_T", len) == 0)
    {
        return SA_IMM_ATTR_SASTRINGT;
    }
    else if (strncmp((const char*)str, "SA_ANY_T", len   ) == 0)
    {
        return SA_IMM_ATTR_SAANYT;
    }

    LOG_ER("UNKNOWN VALUE TYPE, %s", str);

    exit(1);
}

/**
 * Returns the value for a given attribute name in the xml attribute array
 */
static char* getAttributeValue(const char* attr, 
                               const xmlChar** const attrArray)
{
    int i;

    if (attrArray == NULL)
    {
        LOG_ER( "The document is TOO DEEPLY NESTED");
        return NULL;
    }

    for (i = 0; attrArray != NULL && attrArray[i*2] != NULL; i++)
    {
        if (strcmp(attr, (const char*)attrArray[i*2]) == 0)
        {
            return(char*)attrArray[i*2 + 1];
        }
    }

    LOG_ER( "RETURNING NULL");
    return NULL;
}

/**
 * Adds an object attr definition to the state->attrValues list
 */
static void addObjectAttributeDefinition(ParserState* state)
{
    std::list<char*>::iterator it;
    SaImmAttrValuesT_2 attrValues;
    int i;
    size_t len;

    /* The attrName must be set */
    assert(state->attrName);

    /* The value array can not be empty */
    assert(state->attrValueBuffers.size() != 0);

    /* The object class must be set */
    assert(state->objectClass);

    /* Set the valueType */
    attrValues.attrValueType = getClassAttrValueType(state, 
                                                     state->objectClass,
                                                     state->attrName);

    TRACE_8("addObjectAttributeDefinition %s, %s, %d",
            state->className, 
            state->attrName,
            attrValues.attrValueType);

    /* For each value, convert from char* to SaImmAttrValuesT_2 and
       store an array pointing to all in attrValues */
    attrValues.attrValuesNumber = state->attrValueBuffers.size();
    attrValues.attrValues = (SaImmAttrValueT*)
                            malloc(sizeof(SaImmAttrValuesT_2) * 
                                   attrValues.attrValuesNumber + 1);

    attrValues.attrValues[attrValues.attrValuesNumber] = NULL;

    it = state->attrValueBuffers.begin();
    i = 0;
    while (it != state->attrValueBuffers.end())
    {
        TRACE_8("NAME: %s", state->attrName);

        charsToValueHelper(&attrValues.attrValues[i],
                           attrValues.attrValueType, 
                           *it);

        i++;
        it++;
    }

    /* Assign the name */
    len = strlen(state->attrName);
    attrValues.attrName = (char*) malloc(len + 1);
    if (attrValues.attrName == NULL)
    {
        LOG_ER("Failed to malloc attrValues.attrName");
        exit(1);
    }
    strncpy(attrValues.attrName, 
            state->attrName,
            len);
    attrValues.attrName[len] = '\0';

    /* Add attrValues to the list */
    state->attrValues.push_front(attrValues);

    /* Free unneeded data */
    for (it = state->attrValueBuffers.begin();
        it != state->attrValueBuffers.end();
        it++)
    {
        free(*it);
    }

    state->attrValueBuffers.clear();
    assert(state->attrValueBuffers.size() == 0);
}

/**
 * Saves the rdn attribute for a class in a map
 */
static void saveRDNAttribute(ParserState* state)
{
    if (state->attrFlags & SA_IMM_ATTR_RDN &&
        state->classRDNMap.find(std::string(state->className)) ==
        state->classRDNMap.end())
    {
        SaImmAttrValuesT_2 values;
        size_t len;

        /* Set the RDN name */
        len = strlen(state->attrName);
        values.attrName = (char*)malloc(len + 1);
        if (values.attrName == NULL)
        {
            LOG_ER( "Failed to malloc values.attrName");
            return;
        }

        strncpy(values.attrName, state->attrName, len);
        values.attrName[len] = '\0';

        /* Set the valueType */
        values.attrValueType = state->attrValueType;

        /* Set the number of attr values */
        values.attrValuesNumber = 1;

        TRACE_8("ADDED CLASS TO RDN MAP");
        state->classRDNMap[std::string(state->className)] =
            values;
    }
}

/**
 * Adds the given valueType to the a mapping in the state
 *
 * Mapping: class -> attribute name -> type
 */
static void addToAttrTypeCache(ParserState* state, 
                               SaImmValueTypeT valueType)
{
    /* className and attrName can not be NULL */
    assert(state->className && state->attrName);

    std::string classString;
    std::string attrNameString;

    classString = std::string(state->className);
    attrNameString = std::string(state->attrName);
    state->classAttrTypeMap[classString][attrNameString] = valueType;
}

/**
 * Returns the valueType for a given state, classname and attribute name
 */
static SaImmValueTypeT getClassAttrValueType(ParserState* state,
                                             const char* className,
                                             const char* attrName)
{
    std::string classNameString;
    std::string attrNameString;

    std::map<std::string, std::map<std::string, std::string> >::iterator classIt;
    std::map<std::string, std::string>::iterator attrIt;

    classNameString = std::string(className);
    attrNameString = std::string(attrName);

    if (state->classAttrTypeMap.find(classNameString) ==
        state->classAttrTypeMap.end())
    {
        LOG_ER("NO CORRESPONDING CLASS %s", className);
        exit(1);
    }

    if (state->classAttrTypeMap[classNameString].find(attrNameString) ==
        state->classAttrTypeMap[classNameString].end())
    {
        LOG_ER("NO CORRESPONDING ATTRIBUTE %s in class %s", attrName,
               className);
        exit(1);
    }

    return state->classAttrTypeMap[classNameString][attrNameString];
}

/**
 * Adds an class attribute definition to the list
 */
static void addClassAttributeDefinition(ParserState* state)
{
    SaImmAttrDefinitionT_2 attrDefinition;

    /* Set the name */
    if (state->attrName != NULL)
    {
        attrDefinition.attrName = state->attrName;
    }
    else
    {
        LOG_ER( "NO ATTR NAME");
    }

    /* Save the attribute definition in classRDNMap if the RDN flag is
     * set */
    saveRDNAttribute(state);

    /* Set attrValueType */
    assert(state->attrValueTypeSet);
    attrDefinition.attrValueType = state->attrValueType;
    if (state->state[state->depth] == RDN)
    {
        TRACE_8("ADDING RDN!");
        /*
        // Work-around since the IMM-server only accepts SaNameT RDN
        if (attrDefinition.attrValueType != SA_IMM_ATTR_SANAMET)
        {
            LOG_IN("RDN of type SA_STRING_T is not supported. "
                   "Changed to SA_NAME_T for class %s",
                   state->className);
        }

        attrDefinition.attrValueType = SA_IMM_ATTR_SANAMET;
        */
    }
    else if (state->attrValueTypeSet)
    {
        attrDefinition.attrValueType = state->attrValueType;
        TRACE_8("ATTR %s, %d",
                attrDefinition.attrName,
                attrDefinition.attrValueType);
    }
    else
    {
        LOG_ER("NO ATTR VALUE TYPE");
        exit(1);
    }

    /* Set the flags */
    attrDefinition.attrFlags = state->attrFlags;

    /* Set the NtfId */
    if (state->attrNtfIdSet)
    {
        LOG_WA("IGNORING NTF-ID FOR CLASS CREATE");
        //attrDefinition.attrNtfId = state->attrNtfId;
    }
    else
    {
        //TRACE_8("NO ATTR NTF ID");
        //attrDefinition.attrNtfId = 0;
    }

    /* Set the default value */
    if (state->attrDefaultValueSet)
    {
        charsToValueHelper(&attrDefinition.attrDefaultValue,
                           state->attrValueType,
                           state->attrDefaultValueBuffer);
    }
    else
    {
        attrDefinition.attrDefaultValue = NULL;
    }

    /* Add to the list of attrDefinitions */
    state->attrDefinitions.push_front(attrDefinition);

    /* Free the default value */
    free(state->attrDefaultValueBuffer);
    state->attrDefaultValueBuffer = NULL;
}

/**
 * Converts an array of chars to an SaImmAttrValueT
 */
static void charsToValueHelper(SaImmAttrValueT* value, 
                               SaImmValueTypeT type,
                               const char* str)
{
    size_t len;
    unsigned int i;
    char byte[5];
    char* endMark;

    TRACE_8("CHARS TO VALUE HELPER");

    switch (type)
    {
        case SA_IMM_ATTR_SAINT32T:
            *value = malloc(sizeof(SaInt32T));
            *((SaInt32T*)*value) = (SaInt32T)atoi(str);
            break;
        case SA_IMM_ATTR_SAUINT32T:
            *value = malloc(sizeof(SaUint32T));
            *((SaUint32T*)*value) = (SaUint32T)atoi(str);
            break;
        case SA_IMM_ATTR_SAINT64T:
            *value = malloc(sizeof(SaInt64T));
            *((SaInt64T*)*value) = (SaInt64T)atoll(str);
            break;
        case SA_IMM_ATTR_SAUINT64T:
            *value = malloc(sizeof(SaUint64T));
            endMark = (char*)(str + len);
            *((SaUint64T*)*value) = (SaUint64T)
                                    strtoull(str, &endMark, 10);
            break;
        case SA_IMM_ATTR_SATIMET: // Int64T
            *value = malloc(sizeof(SaInt64T));
            *((SaTimeT*)*value) = (SaTimeT)atoll(str);
            break;
        case SA_IMM_ATTR_SANAMET:
            len = strlen(str);
            *value = malloc(sizeof(SaNameT));
            ((SaNameT*)*value)->length = (SaUint16T)len;
            strncpy((char*)((SaNameT*)*value)->value, str, len);
            ((SaNameT*)*value)->value[len] = '\0';
            break;
        case SA_IMM_ATTR_SAFLOATT:
            *value = malloc(sizeof(SaFloatT));
            *((SaFloatT*)*value) = (SaFloatT)atof(str);
            break;
        case SA_IMM_ATTR_SADOUBLET:
            *value = malloc(sizeof(SaDoubleT));
            *((SaDoubleT*)*value) = (SaDoubleT)atof(str);
            break;
        case SA_IMM_ATTR_SASTRINGT:
            len = strlen(str);
            *value = malloc(sizeof(SaStringT));
            *((SaStringT*)*value) = (SaStringT)malloc(len + 1);
            strncpy(*((SaStringT*)*value), str, len);
            (*((SaStringT*)*value))[len] = '\0';
            break;
        case SA_IMM_ATTR_SAANYT:
            len = strlen(str) / 2;
            *value = malloc(sizeof(SaAnyT));
            ((SaAnyT*)*value)->bufferAddr = 
                (SaUint8T*)malloc(sizeof(SaUint8T) * len);
            ((SaAnyT*)*value)->bufferSize = len;

            byte[0] = '0';
            byte[1] = 'x';
            byte[4] = '\0';

            endMark = byte + 4;

            for (i = 0; i < len; i++)
            {
                byte[2] = str[2*i];
                byte[3] = str[2*i + 1];

                ((SaAnyT*)*value)->bufferAddr[i] = 
                    (SaUint8T)strtod(byte, &endMark);
            }

            TRACE_8("SIZE: %d", (int) ((SaAnyT*)*value)->bufferSize);
            break;
        default:
            LOG_ER("BAD VALUE TYPE");
            exit(1);
    }
}

int loadImmXML(std::string xmldir, std::string file)
{
    ParserState state;
    SaVersionT version;
    SaAisErrorT errorCode;
    std::string filename;
    int result;

    result = -1;

    version.releaseCode   = 'A';
    version.majorVersion  = 2;
    version.minorVersion  = 1;

    TRACE("Loading from %s/%s", xmldir.c_str(), file.c_str());

    errorCode = saImmOmInitialize(&(state.immHandle), 
                                  &immCallbacks,
                                  &version);
    if (SA_AIS_OK != errorCode)
    {
        LOG_ER("Failed to initialize the IMM OM interface (%d)", errorCode);
        exit(1);
    }
    state.immInit = 1;

    /* Initialize the state */
    state.attrDefaultValueBuffer = NULL;
    state.objectClass            = NULL;
    state.objectName             = NULL;
    state.className              = NULL;
    state.attrName               = NULL;
    state.classCategorySet    = 0;
    state.attrValueTypeSet    = 0;
    state.attrNtfIdSet        = 0;
    state.attrDefaultValueSet = 0;
    state.attrValueSet        = 0;
    state.doneParsingClasses  = 0;
    state.depth               = 0;
    state.attrFlags           = 0;

    state.immInit   = 0;
    state.adminInit = 0;
    state.ccbInit   = 0;

    /* Build the filename */
    filename = xmldir;
    filename.append("/");
    filename.append(file);

    std::cout << "Loading " << filename << std::endl;

    result = xmlSAXUserParseFile(&my_handler, &state, filename.c_str());

    /* Make sure to finalize the imm connections */
    /* Finalize the ccb connection*/
    if (state.ccbInit)
    {
        errorCode = saImmOmCcbFinalize(state.ccbHandle);
        if (SA_AIS_OK != errorCode)
        {
            LOG_ER("Failed to finalize the ccb object connection %d",
                   errorCode);
        }
    }

    /* Finalize the owner connection */
    if (state.adminInit)
    {
        errorCode = saImmOmAdminOwnerFinalize(state.ownerHandle);
        if (SA_AIS_OK != errorCode)
        {
            LOG_ER("Failed on saImmOmAdminOwnerFinalize (%d)", errorCode);
        }
    }

    /* Finalize the imm connection */
    if (state.immInit)
    {
        errorCode = saImmOmFinalize(state.immHandle);  
        if (SA_AIS_OK != errorCode)
        {
            LOG_ER("Failed on saImmOmFinalize (%d)", errorCode);
        }
    }

    return !result; 
}

int getClassNames(SaImmHandleT& immHandle, std::list<std::string>& classNamesList)
{
    //std::list<std::string>::iterator it;
    //SaImmSearchHandleT searchHandle;
    SaImmAccessorHandleT accessorHandle;
    SaImmAttrValuesT_2** attributes;
    SaImmAttrValuesT_2** p;
    SaNameT tspSaObjectName;
    SaAisErrorT err=SA_AIS_OK;


    if (saImmOmAccessorInitialize(immHandle, &accessorHandle) != SA_AIS_OK)
    {
        LOG_ER("saImmOmAccessorInitialize failed");
        exit(1);
    }

    strcpy((char*)tspSaObjectName.value, OPENSAF_IMM_OBJECT_DN);
    tspSaObjectName.length = strlen(OPENSAF_IMM_OBJECT_DN);

    SaImmAttrNameT attNames[2] = {OPENSAF_IMM_ATTR_CLASSES,0};

    err = saImmOmAccessorGet_2(accessorHandle, 
                               &tspSaObjectName, 
                               attNames, 
                               &attributes);
    if (err != SA_AIS_OK)
    {
        if (err == SA_AIS_ERR_NOT_EXIST)
        {
            LOG_ER("The IMM meta-object (%s) does not exist!", 
                   OPENSAF_IMM_OBJECT_DN);
        }
        else
        {
            LOG_ER("Failed to fetch the IMM meta-object (%s) error code:%u",
                   OPENSAF_IMM_OBJECT_DN, err);

        }
        exit(1);      
    }


    /* Find the classes attribute */
    for (p = attributes; ((p != NULL) && (*p != NULL)); p++)
    {
        if (strcmp((*p)->attrName, OPENSAF_IMM_ATTR_CLASSES) == 0)
        {
            attributes = p;
            break;
        }
    }

    if (p == NULL)
    {
        LOG_ER("Failed to find the %s attribute", OPENSAF_IMM_ATTR_CLASSES);
        exit(1);
    }

    /* Iterate through the class names */
    for (SaUint32T i = 0; i < (*attributes)->attrValuesNumber; i++)
    {
        if ((*attributes)->attrValueType == SA_IMM_ATTR_SASTRINGT)
        {
            std::string classNameString =
                std::string(*(SaStringT*)(*attributes)->attrValues[i]);

            classNamesList.push_front(classNameString);
        }
        else if ((*attributes)->attrValueType == SA_IMM_ATTR_SANAMET)
        {
            std::string classNameString((char*)((SaNameT*)(*attributes)->attrValues
                                                + i)->value,
                                        ((SaNameT*)(*attributes)->attrValues + i)->length);

            classNamesList.push_front(classNameString);
        }
        else
        {
            LOG_ER("Invalid type for attribute %s in class %s",
                   OPENSAF_IMM_ATTR_CLASSES, OPENSAF_IMM_CLASS_NAME);
            exit(1);
        }
    }

    saImmOmAccessorFinalize(accessorHandle);

    return classNamesList.size();
}

void syncClassDescription(std::string className, SaImmHandleT& immHandle)
{
    SaImmClassCategoryT classCategory;
    SaImmAttrDefinitionT_2 **attrDefinitions;
    TRACE("Syncing class description for %s", className.c_str());

    const SaImmClassNameT cln = (char *) className.c_str();

    SaAisErrorT err = saImmOmClassDescriptionGet_2(immHandle, cln,
                                                   &classCategory,
                                                   &attrDefinitions);
    TRACE_8("Back from classDescriptionGet_2 for %s ", cln);

    if (err != SA_AIS_OK)
    {
        LOG_ER("Failed to fetch class description for %s, error code:%u", cln, 
               err);
        exit(1);
    }

    err = saImmOmClassCreate_2(immHandle, cln, classCategory, 
                               (const SaImmAttrDefinitionT_2**) attrDefinitions);
    if (err != SA_AIS_OK)
    {
        LOG_ER("Failed to sync-create class %s, error code:%u", cln, err);
        exit(1);
    }

    saImmOmClassDescriptionMemoryFree_2(immHandle, attrDefinitions);
}

int syncObjectsOfClass(std::string className, SaImmHandleT& immHandle)
{
    SaImmClassCategoryT classCategory;
    SaImmAttrDefinitionT_2 **attrDefinitions;
    TRACE("Syncing instances of class %s", className.c_str());

    const SaImmClassNameT cln = (char *) className.c_str();

    SaAisErrorT err = saImmOmClassDescriptionGet_2(immHandle, cln,
                                                   &classCategory,
                                                   &attrDefinitions);
    if (err != SA_AIS_OK)
    {
        LOG_ER("Failed to fetch class description for %s, error code:%u", 
               cln, err);
        exit(1);
    }

    //Determine how many attributes the class defines.
    int nrofAtts;
    for (nrofAtts=0;attrDefinitions[nrofAtts]; ++nrofAtts)
    {
        if (nrofAtts > 999)
        {
            LOG_ER("Class apparently has too many attributes");
            exit(1);
        }
    }

    if (!nrofAtts)
    { //Classes with no attributes are not possible
        LOG_ER("Class apparently has NO attributes ?");
        exit(1);
    }

    SaImmAttrNameT* attrNames = (SaImmAttrNameT *) 
                                calloc(nrofAtts+1, sizeof(SaImmAttrNameT)); //one added for null-term

    //Then pick all attributes except runtimes that are neither cached
    //nor persistent. These "pure" runtime atts will not be synced.
    SaImmAttrDefinitionT_2* attd;
    int ix=0;
    for (nrofAtts=0;attrDefinitions[nrofAtts]; ++nrofAtts)
    {
        attd=attrDefinitions[nrofAtts];
        if ((attd->attrFlags & SA_IMM_ATTR_RUNTIME) &&
            !(attd->attrFlags & SA_IMM_ATTR_PERSISTENT) &&
            !(attd->attrFlags & SA_IMM_ATTR_CACHED)
           )
        {
            continue;
        }
        attrNames[ix++] = attd->attrName;
    }


    SaImmSearchParametersT_2 param; //Filter objects on class-name attribute.

    param.searchOneAttr.attrName = SA_IMM_ATTR_CLASS_NAME;
    param.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
    param.searchOneAttr.attrValue = (void *) &cln;  //Pointer TO char*

    SaImmSearchHandleT searchHandle;
    err = saImmOmSearchInitialize_2(immHandle, 
                                    NULL, //rootname
                                    SA_IMM_SUBTREE,
                                    (SaImmSearchOptionsT)
                                    (SA_IMM_SEARCH_ONE_ATTR |
                                     SA_IMM_SEARCH_GET_SOME_ATTR |
                                     SA_IMM_SEARCH_SYNC_CACHED_ATTRS),
                                    &param,
                                    attrNames,
                                    &searchHandle);

    //Should perhaps do retries if err == SA_AIS_ERR_TRY_AGAIN
    free(attrNames);
    attrNames = NULL;
    if (err != SA_AIS_OK)
    {
        LOG_ER("Failed to initialize search for instances "
               "of class %s, error code:%u", cln, err);
        exit(1);
    }

    saImmOmClassDescriptionMemoryFree_2(immHandle, attrDefinitions);

    //Iterate over objects
    SaNameT objectName;
    objectName.value[0] = 0;
    objectName.length = 0;
    SaImmAttrValuesT_2 **attributes=NULL;
    err = saImmOmSearchNext_2(searchHandle, &objectName, &attributes);

    int nrofObjs=0;
    while (err != SA_AIS_ERR_NOT_EXIST)
    {
        if (err != SA_AIS_OK)
        {
            //Retries not likely on search-next since it simply fetches from a copy
            LOG_ER("Sync failed on call saImmOmSearchNext_2, error:%u", err);
            exit(1);      
        }
        ++nrofObjs;

        int retries = 0;
        do
        {
            err = immsv_sync(immHandle, cln, &objectName, 
                             (const SaImmAttrValuesT_2 **) attributes);
        } while (err == SA_AIS_ERR_TRY_AGAIN && (++retries < 10));

        if (err != SA_AIS_OK)
        {
            LOG_ER("Sync failed on call immsv_sync, retries: %u error:%u", 
                   retries, err);
            exit(1);
        }

        TRACE("Synced object: %s", objectName.value);

        err = saImmOmSearchNext_2(searchHandle, &objectName, &attributes);
    }

    saImmOmSearchFinalize(searchHandle);
    return nrofObjs;
}

/* 1=>OK, 0=>FAIL */
int immsync(void)
{
    std::list<std::string> classNamesList;
    std::list<std::string>::iterator it;
    SaVersionT version;
    SaAisErrorT errorCode;
    SaImmHandleT immHandle;

    version.releaseCode   = 'A';
    version.majorVersion  = 2;
    version.minorVersion  = 1;

    int retries = 0;
    do
    {
        errorCode = saImmOmInitialize(&immHandle,
                                      NULL,
                                      &version);
        if (retries)
        {
            TRACE_8("IMM-SYNC initialize retry %u", retries);
        }
    } while ((errorCode == SA_AIS_ERR_TRY_AGAIN) && (++retries < 10));
    if (SA_AIS_OK != errorCode)
    {
        LOG_ER("Failed to initialize the IMM OM interface (%d)", errorCode);
        return 0;
    }

    int nrofClasses = getClassNames(immHandle, classNamesList);
    if (nrofClasses <= 0)
    {
        LOG_ER("No classes ??");
        return 0;
    }

    it = classNamesList.begin();

    while (it != classNamesList.end())
    {
        syncClassDescription(*it, immHandle);
        it++;
    }
    TRACE("Sync'ed %u class-descriptions", nrofClasses);

    it = classNamesList.begin();

    int nrofObjects = 0;
    while (it != classNamesList.end())
    {
        int objects = syncObjectsOfClass(*it, immHandle);
        TRACE("Synced %u objects of class %s", objects, (*it).c_str());
        nrofObjects += objects;
        it++;
    }
    LOG_IN("Synced %u objects in total", nrofObjects);

    if (immsv_finalize_sync(immHandle) != SA_AIS_OK)
    {
        LOG_ER("immsv_finalize_sync failed!");
        return 0;
    }

    saImmOmFinalize(immHandle);
    return 1;
}


/** 
 * immloader main entry point.
 * 
 * @param argc 
 * @param argv 
 * 
 * @return 
 */
int main(int argc, char* argv[])
{
    std::string xmldir;
    std::string file;
    const char* defaultLog = OSAF_LOCALSTATEDIR "stdouts/immnd_trace";
    const char* logPath;

    if (argc < 3)
    {
        LOG_ER("Too few arguments. Please provide a path and a file");
        exit(1);
    }

    if ((logPath = getenv("IMMND_TRACE_PATHNAME")) == NULL)
    {
        logPath = defaultLog;
    }

    xmldir = std::string(argv[1]);
    file   = std::string(argv[2]);

    if (argc > 3)
    {
        std::string syncMarker("sync");
        if (syncMarker == std::string(argv[3]))
        {

            if (logtrace_init("imm-sync", logPath) == -1)
            {
                syslog(LOG_ERR, "logtrace_init FAILED");
                /* We allow the sync to execute anyway. */
            }

            if (getenv("IMMSV_TRACE_CATEGORIES") != NULL)
            {
                /* Do not care about categories now, get all */
                if (trace_category_set(CATEGORY_ALL) == -1)
                {
                    printf("Failed to initialize tracing!\n");
                    LOG_ER("Failed to initialize tracing!");
                }
            }

	    syslog(LOG_NOTICE, "Sync starting");
            if (immsync())
            {
                syslog(LOG_NOTICE, "Sync ending normally");
                exit(0);
            }
            else
            {
                syslog(LOG_ERR, "Sync ending ABNORMALLY");
                exit(1);
            }
        }
    }

    if (logtrace_init("imm-load", logPath) == -1)
    {
        printf("logtrace_init FAILED\n");
        syslog(LOG_ERR, "logtrace_init FAILED");
    }

    if (getenv("IMMSV_TRACE_CATEGORIES") != NULL)
    {
        /* Do not care about categories now, get all */
        if (trace_category_set(CATEGORY_ALL) == -1)
        {
            LOG_ER("Failed to initialize tracing!");
        }
    }

    syslog(LOG_NOTICE, "Load starting");
    if (!loadImmXML(xmldir, file))
        goto err;

    syslog(LOG_NOTICE, "Load ending normally");
    return 0;

err:
    syslog(LOG_ERR, "Load ending ABNORMALLY dir:%s file:%s", argv[1], argv[2]);
    exit(1);

    return 0;
}

