/*
 * dump-perl.c --
 *
 *      Operations to dump SMIng module information in Perl format.
 *
 * Copyright (c) 2000 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 2000 J. Schoenwaelder, Technical University of Braunschweig.
 * Copyright (c) 2000 WideAwake Ltd.
 * Copyright (c) 2000 Martin Schulz martin.schulz@myrealbox.com
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-perl.c,v 1.4 2002/10/30 09:17:37 schoenw Exp $
 */

/*
 * TODO:
 *
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "smi.h"
#include "smidump.h"



#define  INDENT         4    /* indent factor */
#define  INDENTVALUE    20   /* column to start values, except multiline */
#define  INDENTTEXTS    4    /* column to start multiline texts */
#define  INDENTMAX      64   /* max column to fill, break lines otherwise */



typedef struct PerlEscape {
    char character;
    char *escape;
} PerlEscape;

static PerlEscape perlEscapes [] = {
    { '\'',     "\\'" },
    { 0,        NULL }
};


static int current_column = 0;

static char* currentModuleName = NULL;

static char *getStringLanguage(SmiLanguage lang)
{
    return
        (lang == SMI_LANGUAGE_SMIV1) ? "SMIv1" :
        (lang == SMI_LANGUAGE_SMIV2) ? "SMIv2" :
        (lang == SMI_LANGUAGE_SMING) ? "SMIng" :
                                        NULL;
}



static char *getStringStatus(SmiStatus status)
{
    return
        (status == SMI_STATUS_CURRENT)     ? "current" :
        (status == SMI_STATUS_DEPRECATED)  ? "deprecated" :
        (status == SMI_STATUS_OBSOLETE)    ? "obsolete" :
        (status == SMI_STATUS_MANDATORY)   ? "current" :
        (status == SMI_STATUS_OPTIONAL)    ? "current" :
                                             "<unknown>";
}



static char *getAccessString(SmiAccess access)
{
    return
        (access == SMI_ACCESS_NOT_ACCESSIBLE) ? "noaccess" :
        (access == SMI_ACCESS_NOTIFY)         ? "notifyonly" :
        (access == SMI_ACCESS_READ_ONLY)      ? "readonly" :
        (access == SMI_ACCESS_READ_WRITE)     ? "readwrite" :
                                                "<unknown>";
}



static char *getStringBasetype(SmiBasetype basetype)
{
    return
        (basetype == SMI_BASETYPE_UNKNOWN)           ? "<UNKNOWN>" :
        (basetype == SMI_BASETYPE_OCTETSTRING)       ? "OctetString" :
        (basetype == SMI_BASETYPE_OBJECTIDENTIFIER)  ? "ObjectIdentifier" :
        (basetype == SMI_BASETYPE_UNSIGNED32)        ? "Unsigned32" :
        (basetype == SMI_BASETYPE_INTEGER32)         ? "Integer32" :
        (basetype == SMI_BASETYPE_UNSIGNED64)        ? "Unsigned64" :
        (basetype == SMI_BASETYPE_INTEGER64)         ? "Integer64" :
        (basetype == SMI_BASETYPE_FLOAT32)           ? "Float32" :
        (basetype == SMI_BASETYPE_FLOAT64)           ? "Float64" :
        (basetype == SMI_BASETYPE_FLOAT128)          ? "Float128" :
        (basetype == SMI_BASETYPE_ENUM)              ? "Enumeration" :
        (basetype == SMI_BASETYPE_BITS)              ? "Bits" :
                                                   "<unknown>";
}



static char *getTimeString(time_t t)
{
    static char   s[27];
    struct tm     *tm;

    tm = gmtime(&t);
    sprintf(s, "%04d-%02d-%02d %02d:%02d",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min);
    return s;
}



static void print(char *fmt, ...)
{
    va_list ap;
    char    s[200];
    char    *p;

    va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
    current_column += vsnprintf(s, sizeof(s), fmt, ap);
#else
    current_column += vsprintf(s, fmt, ap);     /* buffer overwrite */
#endif
    va_end(ap);

    fputs(s, stdout);

    if ((p = strrchr(s, '\n'))) {
        current_column = strlen(p) - 1;
    }
}



static void printSegment(int column, char *string, int length)
{
    print("%*c%s", column, ' ', string);
    if (length) {
        print("%*c", length - strlen(string) - column, ' ');
    }
}



static void printMultilineString(int column, const char *s)
{
    int i, j, len;

#ifdef INDENTTEXTS
    printSegment(column + INDENTTEXTS, "", 0);
#endif
    if (s) {
        fputs ("'", stdout);
        current_column += 3;
        len = strlen(s);
        for (i=0; i < len; i++) {
            for (j = 0; perlEscapes[j].character; j++) {
                if (perlEscapes[j].character == s[i]) break;
            }
            if (perlEscapes[j].character) {
                fputs(perlEscapes[j].escape, stdout);
                current_column += strlen(perlEscapes[j].escape);
            } else {
                putc(s[i], stdout);
                current_column++;
            }
            if (s[i] == '\n') {
                current_column = 0;
            }
        }
        fputs ("'", stdout);
        current_column += 3;
    }
    current_column++;
}



static char *getValueString(SmiValue *valuePtr, SmiType *typePtr)
{
    static char    s[100];
    char           ss[9];
    int            n;
    unsigned int   i;
    SmiNamedNumber *nn;

    s[0] = 0;

    switch (valuePtr->basetype) {
    case SMI_BASETYPE_UNSIGNED32:
        sprintf(s, "%lu", valuePtr->value.unsigned32);
        break;
    case SMI_BASETYPE_INTEGER32:
        sprintf(s, "%ld", valuePtr->value.integer32);
        break;
    case SMI_BASETYPE_UNSIGNED64:
        sprintf(s, UINT64_FORMAT, valuePtr->value.unsigned64);
        break;
    case SMI_BASETYPE_INTEGER64:
        sprintf(s, INT64_FORMAT, valuePtr->value.integer64);
        break;
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
        break;
    case SMI_BASETYPE_ENUM:
        for (nn = smiGetFirstNamedNumber(typePtr); nn;
             nn = smiGetNextNamedNumber(nn)) {
            if (nn->value.value.unsigned32 == valuePtr->value.unsigned32)
                break;
        }
        if (nn) {
            sprintf(s, "%s", nn->name);
        } else {
            sprintf(s, "%ld", valuePtr->value.integer32);
        }
        break;
    case SMI_BASETYPE_OCTETSTRING:
        for (i = 0; i < valuePtr->len; i++) {
            if (!isprint((int)valuePtr->value.ptr[i])) break;
        }
        if (i == valuePtr->len) {
            sprintf(s, "%s", valuePtr->value.ptr);
        } else {
            sprintf(s, "0x%*s", 2 * valuePtr->len, "");
            for (i=0; i < valuePtr->len; i++) {
                sprintf(ss, "%02x", valuePtr->value.ptr[i]);
                strncpy(&s[2+2*i], ss, 2);
            }
        }
        break;
    case SMI_BASETYPE_BITS:
        sprintf(s, "(");
        for (i = 0, n = 0; i < valuePtr->len * 8; i++) {
            if (valuePtr->value.ptr[i/8] & (1 << (7-(i%8)))) {
                if (n)
                    sprintf(&s[strlen(s)], ", ");
                n++;
                for (nn = smiGetFirstNamedNumber(typePtr); nn;
                     nn = smiGetNextNamedNumber(nn)) {
                    if (nn->value.value.unsigned32 == i)
                        break;
                }
                if (nn) {
                    sprintf(&s[strlen(s)], "%s", nn->name);
                } else {
                    sprintf(s, "%d", i);
                }
            }
        }
        sprintf(&s[strlen(s)], ")");
        break;
    case SMI_BASETYPE_UNKNOWN:
        break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
        for (i = 0; i < valuePtr->len; i++) {
            sprintf(&s[strlen(s)], i ? ".%u" : "%u", valuePtr->value.oid[i]);
        }
        break;
    }

    return s;
}



static void printNodeStartTag(int indent, const char *tag, SmiNode *smiNode)
{
    unsigned int i;

    printSegment(indent, "", 0);
    print("{\n", smiNode->name);
    printSegment(indent + INDENT, "", 0);
    print("\"name\" => \"%s\",\n", smiNode->name);
    printSegment(indent + INDENT, "", 0);
    print("\"type\" => \"%s\",\n", tag);
    printSegment(indent + INDENT, "", 0);
    print("\"module\" => \"%s\",\n", currentModuleName);
    printSegment(indent + INDENT, "", 0);
    print("\"oid\" => \"");
    for (i = 0; i < smiNode->oidlen; i++) {
        print(i ? ".%u" : "%u", smiNode->oid[i]);
    }
    print("\"");

    if (smiNode->create) {
        print (",\n");
        printSegment(indent + INDENT, "\"create\" => \"true\"", 0);
    }
    if (smiNode->status != SMI_STATUS_UNKNOWN) {
        print (",\n");
        printSegment(indent + INDENT, "", 0);
        print("\"status\" => \"%s\"", getStringStatus(smiNode->status));
    }
    print (",\n");
}



static void printNodeEndTag(int indent, const char *tag, const char *name)
{
    printSegment(indent, "", 0);
    print("}, # %s\n", name);
}



static void printRanges(int indent, SmiType *smiType)
{
    SmiRange       *range;

    for(range = smiGetFirstRange(smiType);
        range;
        range = smiGetNextRange(range)) {
        printSegment(indent, "\"range\" => {\n", 0);
        printSegment(indent + INDENT, "", 0);
        print("\"min\" => \"%s\",\n",
              getValueString(&range->minValue, smiType));
        printSegment(indent + INDENT, "", 0);
        print("\"max\" => \"%s\"\n", getValueString(&range->maxValue, smiType));
        printSegment(indent, "},\n", 0);
    }
}



static void printNamedNumbers(int indent, SmiType *smiType)
{
    SmiNamedNumber *nn;

    if ((smiType->basetype != SMI_BASETYPE_ENUM) &&
        (smiType->basetype != SMI_BASETYPE_BITS)) {
        return;
    }

    for (nn = smiGetFirstNamedNumber(smiType);
         nn;
         nn = smiGetNextNamedNumber(nn)) {
        printSegment(indent, "\"", 0);
        print ("%s\" => ", nn->name);
        print("\"%s\",\n", getValueString(&nn->value, smiType));
    }
}



static void printValue(int indent, SmiValue *smiValue, SmiType *smiType)
{
    if (smiType && smiValue && smiValue->basetype != SMI_BASETYPE_UNKNOWN) {
        printSegment(indent, "\"DEFVAL\" => ", 0);
        print("\"%s\"", getValueString(smiValue, smiType));
        print(",\n");
    }
}



static void printDescription(int indent, const char *description)
{
    if (description) {
        printSegment(indent, "\"description\" =>\n", 0);
        printMultilineString(indent, description);
        print(",\n");
    }
}



static void printReference(int indent, const char *reference)
{
    if (reference) {
        printSegment(indent, "\"reference>\" =>\n", 0);
        printMultilineString(indent, reference);
        print(",\n");
    }
}



static void printFormat(int indent, const char *format)
{
    if (format) {
        printSegment(indent, "", 0);
        print ("\"DISPLAY-HINT\" => \"%s\",\n", format);
    }
}



static void printUnits(int indent, const char *units)
{
    if (units) {
        printSegment(indent, "", 0);
        print ("\"UNITS\" => \"%s\",\n", units);
    }
}



static void printAccess(int indent, SmiAccess smiAccess)
{
    if (smiAccess != SMI_ACCESS_UNKNOWN) {
        printSegment(indent, "", 0);
        print("\"access\" => \"%s\",\n", getAccessString(smiAccess));
    }
}



static void printElementList(int indent, const char *tag,
                             SmiElement *smiElement, int asArray)
{
    SmiModule *smiModule;
    SmiNode   *smiNode;

    for (; smiElement; smiElement = smiGetNextElement(smiElement)) {
        smiNode = smiGetElementNode(smiElement);
        smiModule = smiGetNodeModule(smiNode);
        printSegment(indent, "", 0);
        if ( tag ) {
            if ( asArray ) {
                print("{\n");
                printSegment(indent + INDENT, "", 0);
                print("\"name\" => \"%s\",\n", smiNode->name);
            } else {
                print("\"%s\" => {\n", smiNode->name);
            }
            printSegment(indent + INDENT, "", 0);
            print("\"type\" => \"%s\",\n", tag);
            printSegment(indent + INDENT, "", 0);
            print("\"module\" => \"%s\"\n", smiModule->name);
            printSegment(indent, "},\n", 0);
        } else {
            if ( asArray ) {
                print("{ \"module\" => \"%s\", ", smiModule->name);
                print("\"name\" => \"%s\" },\n", smiNode->name);
            } else {
                print("\"%s\" => \"%s\",\n", smiNode->name, smiModule->name);
            }
        }
    }
}


static void printIndexModule (int indent, const char *modname,
                              const char *nodename, const char *indexkind)
{
    printSegment(indent, "", 0);
    print("\"%s\" => {\n", indexkind);
    printSegment(indent + INDENT, "", 0);
    print("\"module\" => \"%s\",\n", modname);
    printSegment(indent + INDENT, "", 0);
    print("\"name\" => \"%s\",\n", nodename);
    printSegment(indent, "},\n", 0);
}

static void printIndex(int indent, SmiNode *smiNode)
{
    SmiNode   *relatedNode;
    SmiModule *relatedModule = NULL;

    if (smiNode->implied) {
        printSegment(indent, "\"indeximplied\" => \"true\",\n", 0);
    }

    relatedNode = smiGetRelatedNode(smiNode);
    if (relatedNode) {
        relatedModule = smiGetNodeModule(relatedNode);
    }
    switch (smiNode->indexkind) {
    case SMI_INDEX_INDEX:
        printSegment(indent, "\"index\" => [\n", 0);
        printElementList(indent + INDENT, NULL,
                         smiGetFirstElement(smiNode), 1);
        printSegment(indent, "], #index\n", 0);
        break;
    case SMI_INDEX_AUGMENT:
        if (relatedNode && relatedModule) {
            printIndexModule (indent, relatedModule->name,
                              relatedNode->name, "augments");
        } /* TODO: else print error */
        break;
    case SMI_INDEX_REORDER:
        if (relatedNode && relatedModule) {
            printIndexModule (indent, relatedModule->name,
                              relatedNode->name, "reorders");
            printSegment(indent, "\"index\" => [\n", 0);
            printElementList(indent + INDENT, NULL,
                             smiGetFirstElement(smiNode), 1);
            printSegment(indent, "], #index\n", 0);
        } /* TODO: else print error */
        break;
    case SMI_INDEX_SPARSE:
        if (relatedNode && relatedModule) {
            printIndexModule (indent, relatedModule->name,
                              relatedNode->name, "sparse");
        } /* TODO: else print error */
        break;
    case SMI_INDEX_EXPAND:
        if (relatedNode && relatedModule) {
            printIndexModule (indent, relatedModule->name,
                              relatedNode->name, "expands");
        } /* TODO: else print error */
        break;
    case SMI_INDEX_UNKNOWN:
        break;
    }
}



static void printModule(SmiModule *smiModule)
{
    SmiRevision *smiRevision;
    SmiNode     *smiNode;
    int         i;
    int         there_were_revisions = 0;

    currentModuleName = smiModule->name;

    if (smiModule->organization) {
        printSegment(INDENT, "\"organization\" =>", INDENTVALUE);
        print("\n");
        printMultilineString(INDENT, smiModule->organization);
        print(",\n");
    }

    if (smiModule->contactinfo) {
        printSegment(INDENT, "\"contact\" =>", INDENTVALUE);
        print("\n");
        printMultilineString(INDENT, smiModule->contactinfo);
        print(",\n");
    }
    printDescription(INDENT, smiModule->description);
    printReference(INDENT, smiModule->reference);

    for(i = 0, smiRevision = smiGetFirstRevision(smiModule);
        smiRevision; smiRevision = smiGetNextRevision(smiRevision)) {
        if (i == 0) {
            printSegment(INDENT, "\"revisions\" => [\n", 0);
            there_were_revisions = 1;
        }
        printSegment(2 * INDENT, "{\n", 0);
        printSegment(3 * INDENT, "", 0);
        print("\"date\" => \"%s\",\n",
              getTimeString(smiRevision->date));
        printDescription(3 * INDENT, smiRevision->description);
        printSegment(2 * INDENT, "},\n", 0);
        i++;
    }
    if (there_were_revisions) {
        printSegment(INDENT, "],\n", 0);
    }

    smiNode = smiGetModuleIdentityNode(smiModule);
    if (smiNode) {
        printSegment(INDENT, "", 0);
        print("\"identity\" => \"%s\",\n", smiNode->name);
    }
    print("\n");
}



static void printImports(SmiModule *smiModule)
{
    SmiImport *smiImport;
    SmiIdentifier lastModule = NULL;
    int        i;

    for (i = 0, smiImport = smiGetFirstImport(smiModule);
         smiImport;
         i++,
         lastModule = smiImport->module,
         smiImport = smiGetNextImport(smiImport)) {
        if (i == 0) {
            printSegment(INDENT, "\"imports\" => [\n", 0);
        }
        
        if ( lastModule == NULL || strcmp(lastModule, smiImport->module)) {
            if ( lastModule ) {
                printSegment(3 * INDENT, "]\n", 0);
                printSegment(2 * INDENT, "},\n", 0);
            }
            printSegment(2 * INDENT, "{\n", 0);
            printSegment(3 * INDENT, "", 0);
            print("\"module\" => \"%s\",\n",
                smiImport->module);
            printSegment(3 * INDENT, "\"names\" => [\n", 0);
        }
        printSegment(4 * INDENT, "", 0);
        print("\"%s\",\n", smiImport->name);
    }
    if (lastModule) {
        printSegment(3 * INDENT, "],\n", 0);
        printSegment(2 * INDENT, "}\n", 0);
    }

    if (i) {
        printSegment(INDENT, "], # imports\n\n", 0);
    }
}



static void printTypedef(int indent, SmiType *smiType)
{
    SmiModule *parentModule;
    SmiType *parentType;

    printSegment(indent, "", 0);
    if (smiType->name) {
        print("\"%s\" => ", smiType->name);
    }
    print ("{\n");
    printSegment(indent + INDENT, "", 0);
    print("\"basetype\" => \"%s\",\n", getStringBasetype(smiType->basetype));
    if (smiType->name && smiType->status != SMI_STATUS_UNKNOWN) {
        printSegment(indent + INDENT, "", 0);
        print("\"status\" => \"%s\",\n", getStringStatus(smiType->status));
    }

    parentType = smiGetParentType(smiType);
    parentModule = smiGetTypeModule(parentType);
    if (parentType && parentModule && strlen(parentModule->name)) {
        printSegment(indent + INDENT, "\"parent\" => {\n", 0);
        printSegment(indent + (2 * INDENT), "", 0);
        print("\"module\" => \"%s\",\n", parentModule->name);
        printSegment(indent + (2 * INDENT), "", 0);
        print("\"name\" => \"%s\",\n", parentType->name);
        printSegment(indent + INDENT, "},\n", 0);
    }
    printRanges(indent + INDENT, smiType);
    printNamedNumbers(indent + INDENT, smiType);
    printValue(indent + INDENT, &smiType->value, smiType);
    printFormat(indent + INDENT, smiType->format);
    printUnits(indent + INDENT, smiType->units);
    printDescription(indent + INDENT, smiType->description);
    printReference(indent + INDENT, smiType->reference);

    printSegment(indent, "},", 0);
    if (smiType->name) {
            print(" # %s\n", smiType->name);
    } else {
            print("\n", smiType->name);
    }
}



static void printTypedefs(SmiModule *smiModule)
{
    int          i;
    SmiType      *smiType;

    for(i = 0, smiType = smiGetFirstType(smiModule);
        smiType;
        i++, smiType = smiGetNextType(smiType)) {

        if (i == 0) {
            printSegment(INDENT, "\"typedefs\" => {\n", 0);
        }
        printTypedef(2 * INDENT, smiType);
    }

    if (i) {
        printSegment(INDENT, "}, # typedefs\n\n", 0);
    }
}



static void printNode(int indent, SmiNode *smiNode, SmiNode *lastSmiNode)
{
    SmiModule   *smiModule;
    SmiType     *smiType;
    char        *tag = NULL;

    if (smiNode->nodekind == SMI_NODEKIND_NODE) {
        tag = "node";
    } else if (smiNode->nodekind == SMI_NODEKIND_CAPABILITIES) {
        tag = "capabilities";
    } else if (smiNode->nodekind == SMI_NODEKIND_TABLE) {
        tag = "table";
    } else if (smiNode->nodekind == SMI_NODEKIND_ROW) {
        /* indent += INDENT; */
        tag = "row";
    } else if (smiNode->nodekind == SMI_NODEKIND_COLUMN) {
        /* indent += 2 * INDENT; */
        tag = "column";
    } else if (smiNode->nodekind == SMI_NODEKIND_SCALAR) {
        tag = "scalar";
    }

/*      if (lastSmiNode */
/*      && lastSmiNode->nodekind == SMI_NODEKIND_COLUMN */
/*      && smiNode->nodekind != SMI_NODEKIND_COLUMN) { */
/*      printNodeEndTag(indent + INDENT, "row", smiNode->name); */
/*          printNodeEndTag(indent, "table", smiNode->name); */
/*      } */

    smiType = smiGetNodeType(smiNode);

    printNodeStartTag(indent, tag, smiNode);
    printDescription(indent + INDENT, smiNode->description);
    printReference(indent + INDENT, smiNode->reference);

    if (smiType && (smiType->basetype != SMI_BASETYPE_UNKNOWN)) {
        printSegment(indent + INDENT, "\"syntax\" => {\n", 0);
        smiModule = smiGetTypeModule(smiType);
        printSegment(indent + 2 *INDENT, "", 0);
        print("\"type\" => ");
        if (smiType->name && smiModule) {
            print("{ \"module\" => \"%s\", \"name\" => \"%s\"},\n",
                   smiModule->name, smiType->name);
        } else {
            print("\n");
            printTypedef(indent + 2 * INDENT, smiType);
        }
        printSegment(indent + INDENT, "},\n", 0);
    }
    if ((smiNode->nodekind != SMI_NODEKIND_TABLE) &&
        (smiNode->nodekind != SMI_NODEKIND_ROW) &&
        (smiNode->nodekind != SMI_NODEKIND_CAPABILITIES) &&
        (smiNode->nodekind != SMI_NODEKIND_NODE)) {
        printAccess(indent + INDENT, smiNode->access);
    }
    if (smiType) {
        printValue(indent + INDENT, &smiNode->value, smiType);
    }
    printFormat(indent + INDENT, smiNode->format);
    printUnits(indent + INDENT, smiNode->units);
    if (smiNode->nodekind == SMI_NODEKIND_ROW) {
        printIndex(indent + INDENT, smiNode);
    }

    printNodeEndTag (indent, tag, smiNode->name);
}



static void printNodes(SmiModule *smiModule)
{
    int          i;
    SmiNode      *smiNode, *lastSmiNode;
    SmiNodekind  nodekinds;

    nodekinds =  SMI_NODEKIND_NODE | SMI_NODEKIND_TABLE |
        SMI_NODEKIND_ROW | SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR |
        SMI_NODEKIND_CAPABILITIES;

    for (i = 0,
             lastSmiNode = NULL,
             smiNode = smiGetFirstNode(smiModule, nodekinds);
         smiNode;
         i++,
             lastSmiNode = smiNode,
             smiNode = smiGetNextNode(smiNode, nodekinds)) {

        if (i == 0) {
            printSegment(INDENT, "\"nodes\" => [\n", 0);
        }

        printNode(2 * INDENT, smiNode, lastSmiNode);
    }

/*      if (lastSmiNode */
/*      && lastSmiNode->nodekind == SMI_NODEKIND_COLUMN) { */
/*      printNodeEndTag(3 * INDENT, "row", smiNode->name); */
/*          printNodeEndTag(2 * INDENT, "table", smiNode->name); */
/*      } */

    if (i) {
        printSegment(INDENT, "], # nodes\n\n", 0);
    }
}



static void printNotification(int indent, SmiNode *smiNode)
{
    printNodeStartTag(indent, "notification", smiNode);
    printDescription(indent + INDENT, smiNode->description);
    printReference(indent + INDENT, smiNode->reference);

    printSegment(indent + INDENT, "\"objects\" => [\n", 0);
    printElementList(indent + 2 * INDENT, NULL,
                     smiGetFirstElement(smiNode), 1);
    printSegment(indent + INDENT, "],\n", 0);

    printNodeEndTag(indent, "notification", smiNode->name);
}



static void printNotifications(SmiModule *smiModule)
{
    SmiNode *smiNode;
    int      i;

    for(i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
        smiNode;
        i++, smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {

        if (i == 0) {
            printSegment(INDENT, "\"notifications\" => [\n", 0);
        }
        printNotification(2 * INDENT, smiNode);
    }

    if (i) {
        printSegment(INDENT, "], # notifications\n\n", 0);
    }
}



static void printGroup(int indent, SmiNode *smiNode)
{
    printNodeStartTag(indent, "group", smiNode);
    printDescription(indent + INDENT, smiNode->description);
    printReference(indent + INDENT, smiNode->reference);

    printSegment(indent + INDENT, "\"members\" => [\n", 0);
    printElementList(indent + 2 * INDENT, NULL,
                     smiGetFirstElement(smiNode), 1);
    printSegment(indent + INDENT, "], # members\n", 0);

    printNodeEndTag(indent, "group", smiNode->name);
}



static void printGroups(SmiModule *smiModule)
{
    SmiNode *smiNode;
    int      i;

    for(i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_GROUP);
        smiNode;
        i++, smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {

        if (i == 0) {
            printSegment(INDENT, "\"groups\" => [\n", 0);
        }
        printGroup(2 * INDENT, smiNode);
    }

    if (i) {
        printSegment(INDENT, "], # groups\n\n", 0);
    }
}



static void printComplGroups(int indent, SmiNode *smiNode)
{
    SmiNode   *optSmiNode;
    SmiModule *optSmiModule;
    SmiOption *smiOption;

    if (! smiGetFirstElement(smiNode) && !smiGetFirstOption(smiNode)) {
        return;
    }

    printSegment(indent, "\"requires\" => [\n", 0);
    printElementList(indent + INDENT, "mandatory",
                     smiGetFirstElement(smiNode), 1);

    for(smiOption = smiGetFirstOption(smiNode);
        smiOption;
        smiOption = smiGetNextOption(smiOption)) {
        optSmiNode = smiGetOptionNode(smiOption);
        optSmiModule = smiGetNodeModule(optSmiNode);
        printSegment(indent + INDENT, "", 0);
        print("\"%s\" => {\n", optSmiNode->name);
        printSegment(indent + 2*INDENT, "\"type\" => \"optional\",\n", 0);
        printSegment(indent + 2*INDENT, "", 0);
        print("\"module\" => \"%s\",\n", optSmiModule->name);
        printDescription(indent + 2 * INDENT, smiOption->description);
        printSegment(indent + INDENT, "},\n", 0);
    }

    printSegment(indent, "], # requires\n", 0);
}



static void printRefinement(int indent, SmiRefinement *smiRefinement)
{
    SmiModule *smiModule;
    SmiNode   *smiNode;
    SmiType   *smiType;

    smiNode = smiGetRefinementNode(smiRefinement);
    smiModule = smiGetNodeModule(smiNode);

    printSegment(indent, "", 0);
    print("\"%s\" => {\n", smiNode->name);
    printSegment(indent + INDENT, "", 0);
    print("\"module\" => \"%s\",\n", smiModule->name);

    smiType = smiGetRefinementType(smiRefinement);
    if (smiType) {
        printSegment(indent + INDENT, "\"syntax\" => {\n", 0);
        printSegment(indent + 2*INDENT, "\"type\" => ", 0);
        printTypedef(indent + 2 * INDENT, smiType);
        printSegment(indent + INDENT, "},\n", 0);
    }

    smiType = smiGetRefinementWriteType(smiRefinement);
    if (smiType) {
        printSegment(indent + INDENT, "\"writesyntax\" => {\n", 0);
        printSegment(indent + 2*INDENT, "\"type\" => ", 0);
        printTypedef(indent + 2 * INDENT, smiType);
        printSegment(indent + INDENT, "},\n", 0);
    }

    if (smiRefinement->access != SMI_ACCESS_UNKNOWN) {
        printAccess(indent + INDENT, smiRefinement->access);
    }
    printDescription(indent + INDENT, smiRefinement->description);
    printSegment(indent, "},\n", 0);
}



static void printRefinements(int indent, SmiNode *smiNode)
{
    SmiRefinement *smiRefinement;
    int            i;

    for(i = 0, smiRefinement = smiGetFirstRefinement(smiNode);
        smiRefinement;
        i++, smiRefinement = smiGetNextRefinement(smiRefinement)) {

        if (!i) {
            printSegment(indent, "\"refinements\" => {\n", 0);
        }

        printRefinement(indent + INDENT, smiRefinement);
    }

    if (i) {
        printSegment(indent, "}, # refinements\n\n", 0);
    }
}



static void printCompliance(int indent, SmiNode *smiNode)
{
    printNodeStartTag(indent, "compliance", smiNode);
    printDescription(indent + INDENT, smiNode->description);
    printReference(indent + INDENT, smiNode->reference);

    printComplGroups(indent + INDENT, smiNode);
    printRefinements(indent + INDENT, smiNode);

    printNodeEndTag(indent, "compliance", smiNode->name);
}



static void printCompliances(SmiModule *smiModule)
{
    SmiNode *smiNode;
    int      i;

    for(i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_COMPLIANCE);
        smiNode;
        i++, smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {

        if (!i) {
            printSegment(INDENT, "\"compliances\" => [\n", 0);
        }

        printCompliance(2 * INDENT, smiNode);
    }

    if (i) {
        printSegment(INDENT, "], # compliances\n\n", 0);
    }
}



static void dumpPerl(int modc, SmiModule **modv, int flags, char *output)
{
    int  i;
    FILE *f = stdout;
    char *lang;
    time_t now;

    if (output) {
        f = fopen(output, "w");
        if (!f) {
            fprintf(stderr, "smidump: cannot open %s for writing: ", output);
            perror(NULL);
            exit(1);
        }
    }

    for (i = 0; i < modc; i++) {
        now = time(NULL);
        print("# Perl data structure for %s.\n", modv[i]->name);
        print("#\n");
        print("# Generated on %s", ctime(&now));
        print("# by smidump " SMI_VERSION_STRING ".\n");
        print("#\n");
        print("# Do not edit.\n\n");

        print("{\n");

        print("\"%s\" => {\n", modv[i]->name);
        printSegment(INDENT, "", 0);
        print("\"name\" => \"%s\",\n\n", modv[i]->name);

        printSegment(INDENT, "#    Compiler Info\n", 0);
        print("");
        printSegment(INDENT, "", 0);
        print("\"path\" => \"%s\",\n", modv[i]->path);

        lang = getStringLanguage(modv[i]->language);
        if (lang) {
            printSegment(INDENT, "", 0);
            print("\"language\" => \"%s\",\n", lang);
        }

        print("\n");
        printSegment(INDENT, "#    Module Identity\n", 0);
        printModule(modv[i]);

        printSegment(INDENT, "#    Imports\n", 0);
        printImports(modv[i]);
        printSegment(INDENT, "#    Module Body\n", 0);
        printTypedefs(modv[i]);
        printNodes(modv[i]);
        printNotifications(modv[i]);
        printGroups(modv[i]);
        printCompliances(modv[i]);

        print("  } # %s\n", modv[i]->name);
        print("};\n");
    }

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
        fclose(f);
    }
}



void initPerl()
{
    static SmidumpDriver driver = {
        "perl",
        dumpPerl,
        0,
        SMIDUMP_DRIVER_CANT_UNITE | SMIDUMP_DRIVER_CANT_OUTPUT,
        "Perl MIB dictionaries",
        NULL,
        NULL
    };

    smidumpRegisterDriver(&driver);
}
