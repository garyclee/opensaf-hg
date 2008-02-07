/*
 * dump-smiv3.c --
 *
 *      Operations to dump SMIv3 module information.
 *
 * Copyright (c) 2002 J. Schoenwaelder, University of Osnabrueck
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-smiv3.c,v 1.5 2002/10/30 09:17:37 schoenw Exp $
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



#define  INDENT		4    /* indent factor */
#define  INDENTVALUE	20   /* column to start values, except multiline */
#define  INDENTTEXTS	4    /* column to start multiline texts */
#define  INDENTMAX	72   /* max column to fill, break lines otherwise */



#define SMIV3VERSION "<draft-ietf-sming-smiv3-00.txt>"
#define SMIV3COMMENT "--"



static char *excludeType[] = {
    "RFC1155-SMI", "ObjectSyntax",
    "RFC1155-SMI", "SimpleSyntax",
    "RFC1155-SMI", "ApplicationSyntax",
    "SNMPv2-SMI",  "ObjectSyntax",
    "SNMPv2-SMI",  "SimpleSyntax",
    "SNMPv2-SMI",  "ApplicationSyntax",
    NULL, NULL };
    
static char *convertType[] = {
    "INTEGER",             "Integer32",
    "OCTET STRING",        "OctetString",
    "OBJECT IDENTIFIER",   "Pointer",
    
    "Gauge",               "Gauge32",
    "Counter",             "Counter32",
    "NetworkAddress",      "IpAddress", /* ??? */
    
    NULL, NULL };

static char *convertImport[] = {
    "SNMPv2-SMI",   "MODULE-IDENTITY",    NULL, NULL,
    "SNMPv2-SMI",   "OBJECT-IDENTITY",    NULL, NULL,
    "SNMPv2-SMI",   "OBJECT-TYPE",        NULL, NULL,
    "SNMPv2-SMI",   "NOTIFICATION-TYPE",  NULL, NULL,
    "SNMPv2-SMI",   "ObjectName",         NULL, NULL,
    "SNMPv2-SMI",   "NotificationName",   NULL, NULL,
    "SNMPv2-SMI",   "ObjectSyntax",       NULL, NULL,
    "SNMPv2-SMI",   "SimpleSyntax",       NULL, NULL,
    "SNMPv2-SMI",   "Integer32",          NULL, NULL,
    "SNMPv2-SMI",   "Unsigned32",         NULL, NULL,
    "SNMPv2-SMI",   "ApplicationSyntax",  NULL, NULL,
    "SNMPv2-SMI",   "IpAddress",          "SMIV3-TYPES", "IpAddress",
    "SNMPv2-SMI",   "Counter32",          "SMIV3-TYPES", "Counter32",
    "SNMPv2-SMI",   "TimeTicks",          "SMIV3-TYPES", "TimeTicks",
    "SNMPv2-SMI",   "Opaque",             "SMIV3-TYPES", "Opaque",
    "SNMPv2-SMI",   "Counter64",          "SMIV3-TYPES", "Counter64",
    "SNMPv2-SMI",   "Gauge32",            "SMIV3-TYPES", "Gauge32",
    "SNMPv2-SMI",   "mib-2",              "SMIV3", "mib-2",

    "SNMPv2-TC",   "TimeStamp",           "SMIV3-TYPES", "TimeStamp",
    "SNMPv2-TC",   "TimeInterval",        "SMIV3-TYPES", "TimeInterval",
    "SNMPv2-TC",   "DateAndTime",         "SMIV3-TYPES", "DateAndTime",
    "SNMPv2-TC",   "TruthValue",          "SMIV3-TYPES", "TruthValue",
    "SNMPv2-TC",   "PhysAddress",         "SMIV3-TYPES", "PhysAddress",
    "SNMPv2-TC",   "MacAddress",          "SMIV3-TYPES", "MacAddress",
    "SNMPv2-TC",   "DisplayString",       "SMIV3-TYPES", "DisplayString255",
    "SNMPv2-TC",   "TestAndIncr",         "SMIV3-TYPES", "TestAndIncr",
    "SNMPv2-TC",   "AutonomousType",      "SMIV3-TYPES", "AutonomousType",
    "SNMPv2-TC",   "VariablePointer",     "SMIV3-TYPES", "VariablePointer",
    "SNMPv2-TC",   "RowPointer",          "SMIV3-TYPES", "RowPointer",
    "SNMPv2-TC",   "RowStatus",           "SMIV3-TYPES", "RowStatus",
    "SNMPv2-TC",   "StorageType",         "SMIV3-TYPES", "StorageType",
    "SNMPv2-TC",   "TDomain",             "SMIV3-TYPES", "TDomain",
    "SNMPv2-TC",   "TAddress",            "SMIV3-TYPES", "TAddress",
    "SNMPv2-SMI",   NULL,                 "SMIV3", NULL,
    "SNMPv2-TC",    "TEXTUAL-CONVENTION", NULL, NULL,
    "SNMPv2-TC",    NULL,                 "SMIV3-TYPES", NULL,
    "SNMPv2-CONF",  "OBJECT-GROUP",       NULL, NULL,
    "SNMPv2-CONF",  "NOTIFICATION-GROUP", NULL, NULL,
    "SNMPv2-CONF",  "MODULE-COMPLIANCE",  NULL, NULL,
    "SNMPv2-CONF",  "AGENT-CAPABILITIES", NULL, NULL,
 
    "RFC1155-SMI",  "OBJECT-TYPE",        NULL, NULL,
    "RFC1155-SMI",  "ObjectName",         NULL, NULL,
    "RFC1155-SMI",  "ObjectSyntax",       NULL, NULL,
    "RFC1155-SMI",  "SimpleSyntax",       NULL, NULL,
    "RFC1155-SMI",  "ApplicationSyntax",  NULL, NULL,
    "RFC1155-SMI",  "Gauge",              "SMIV3-TYPES", "Gauge32",
    "RFC1155-SMI",  "Counter",            "SMIV3-TYPES", "Counter32",
    "RFC1155-SMI",  "TimeTicks",          "SMIV3-TYPES", "TimeTicks",
    "RFC1155-SMI",  "IpAddress",          "SMIV3-TYPES", "IpAddress",
    "RFC1155-SMI",  "NetworkAddress",     NULL, NULL, /* ??? */
    "RFC1155-SMI",  "Opaque",             "SMIV3-TYPES", "Opaque",
    "RFC1155-SMI",  NULL,                 "SMIV3", NULL,
    "RFC1158-MIB",  "DisplayString",      "SMIV3-TYPES", "DisplayString255",
    "RFC-1212",     "OBJECT-TYPE",        NULL, NULL,
    "RFC1213-MIB",  "mib-2",              "SMIV3", "mib-2",
    "RFC1213-MIB",  "system",             "SNMPv2-MIB", "system",
    "RFC1213-MIB",  "interfaces",         "IF-MIB", "interfaces",
/*  "RFC1213-MIB",  "at",                 "RFC1213-MIB", "at", */
    "RFC1213-MIB",  "ip",                 "IP-MIB", "ip",
    "RFC1213-MIB",  "icmp",               "IP-MIB", "icmp",
    "RFC1213-MIB",  "tcp",                "TCP-MIB", "tcp",
    "RFC1213-MIB",  "udp",                "UDP-MIB", "udp",
/*  "RFC1213-MIB",  "egp",                "RFC1213-MIB", "egp", */
    "RFC1213-MIB",  "transmission",       "SNMPv2-SMI", "transmission",
    "RFC1213-MIB",  "snmp",               "SNMPv2-MIB", "snmp",
    "RFC1213-MIB",  "sysDescr",           "SNMPv2-MIB", "sysDescr",
    "RFC1213-MIB",  "sysObjectID",        "SNMPv2-MIB", "sysObjectID",
    "RFC1213-MIB",  "sysUpTime",          "SNMPv2-MIB", "sysUpTime",
    "RFC1213-MIB",  "ifIndex",            "IF-MIB", "ifIndex",
/* TODO ...many more objects from RFC1213-MIB.. */    
    "RFC1213-MIB",  "DisplayString",      "SMIV3-TYPES", "DisplayString255",
    "RFC1213-MIB",  "PhysAddress",	  "SMIV3-TYPES", "PhysAddress",
    "RFC-1215",     "TRAP-TYPE",          NULL, NULL,                          

    /* TODO: how to convert more SMIv1 information? */

    NULL, NULL, NULL, NULL };

static int current_column = 0;
static int silent = 0;


/*
 * Structure used to build a list of imported types.
 */


typedef struct Import {
    char          *module;
    char          *name;
    struct Import *nextPtr;
} Import;

static Import *importList = NULL;



static char*
getStringStatus(SmiStatus status)
{
    return
	(status == SMI_STATUS_CURRENT)     ? "current" :
	(status == SMI_STATUS_DEPRECATED)  ? "deprecated" :
	(status == SMI_STATUS_OBSOLETE)    ? "obsolete" :
	(status == SMI_STATUS_MANDATORY)   ? "current" :
	(status == SMI_STATUS_OPTIONAL)    ? "current" :
					     "<unknown>";
}



static char*
getAccessString(SmiAccess access)
{
    return
	(access == SMI_ACCESS_NOT_ACCESSIBLE) ? "not-accessible" :
	(access == SMI_ACCESS_NOTIFY)	      ? "accessible-for-notify" :
	(access == SMI_ACCESS_READ_ONLY)      ? "read-only" :
	(access == SMI_ACCESS_READ_WRITE)     ? "read-write" :
						"<unknown>";
}



static char*
getStringBasetype(SmiBasetype basetype)
{
    return
        (basetype == SMI_BASETYPE_UNKNOWN)           ? "<unknown>" :
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



static char*
getStringTime(time_t t)
{
    static char   s[27];
    struct tm	  *tm;

    tm = gmtime(&t);
    if (tm->tm_hour || tm->tm_min) {
	sprintf(s, "%04d-%02d-%02d %02d:%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min);
    } else {
	sprintf(s, "%04d-%02d-%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    }
    return s;
}



static char *getUppercaseString(char *s)
{
    static char *ss;

    ss = xstrdup(s);
    ss[0] = (char)toupper((int)ss[0]);
    return ss;
}



static char*
getTypeString(SmiBasetype basetype, SmiType *smiType)
{
    int         i;
    char        *typeModule, *typeName;

    typeName = smiType ? smiType->name : NULL;
    typeModule = smiType ? smiGetTypeModule(smiType)->name : NULL;
    
    if ((!typeModule) && (typeName) &&
	(basetype != SMI_BASETYPE_ENUM) &&
	(basetype != SMI_BASETYPE_BITS)) {
	for(i=0; convertType[i]; i += 2) {
	    if (!strcmp(typeName, convertType[i])) {
		return convertType[i+1];
	    }
	}
    }

    if ((!typeModule) || (!strlen(typeModule)) || (!typeName)) {
	if (basetype == SMI_BASETYPE_ENUM) {
	    return "Enumeration";
	}
	if (basetype == SMI_BASETYPE_BITS) {
	    return "Bits";
	}
    }
	
    if (!typeName) {
	return getStringBasetype(basetype);
    }
    
    /* TODO: fully qualified if unambigous */

    return typeName;
}



static char*
getOidString(SmiNode *smiNode, int importedParent)
{
    SmiNode	 *parentNode;
    SmiModule	 *smiModule;
    static char	 s[200];
    char	 append[200];
    unsigned int i;
    
    append[0] = 0;

    parentNode = smiNode;
    smiModule = smiGetNodeModule(smiNode);
    
    do {

	if (parentNode->oidlen <= 1) {
	    break;
	}
	
	/* prepend the cut-off subidentifier to `append'. */
	strcpy(s, append);
	sprintf(append, ".%u%s", parentNode->oid[parentNode->oidlen-1], s);

	/* retrieve the parent SmiNode */
	parentNode = smiGetParentNode(parentNode);

	if (!parentNode) {
	    sprintf(s, "%s", append);
	    return s;
	}

	/* found an imported or a local parent node? */
	if ((parentNode->name && strlen(parentNode->name)) &&
	    (smiIsImported(smiModule, NULL, parentNode->name) ||
	     (!importedParent &&
	      (smiGetNodeModule(parentNode) == smiModule)) ||
	     (parentNode->oidlen == 1))) {
	    sprintf(s, "%s%s", parentNode->name, append);
	    return s;
	}
	
    } while (parentNode);

    s[0] = 0;
    for (i=0; i < smiNode->oidlen; i++) {
	if (i) strcat(s, ".");
	sprintf(&s[strlen(s)], "%u", smiNode->oid[i]);
    }
    return s;
}



static Import*
addImport(char *module, char *name)
{
    Import **import, *newImport;
    int i;
    
    for (i = 0; convertImport[i]; i += 4) {
	if (convertImport[i] && convertImport[i+1]
	    && !strcmp(module, convertImport[i])
	    && !strcmp(name, convertImport[i+1])) {
	    module = convertImport[i+2];
	    name = convertImport[i+3];
	    break;
	} else if (convertImport[i] && !convertImport[i+1]
		   && !strcmp(module, convertImport[i])) {
	    module = convertImport[i+2];
	    break;
	}
    }

    if (!module || !name) {
	return NULL;
    }
	    
    for (import = &importList; *import; import = &(*import)->nextPtr) {
	int c = strcmp((*import)->module, module);
	if (c < 0) continue;
	if (c == 0) {
	    int d = strcmp((*import)->name, name);
	    if (d < 0) continue;
	    if (d == 0) return *import;
	    if (d > 0) break;
	}
	if (c > 0) break;
    }

    newImport = xmalloc(sizeof(Import));
    newImport->module = module;
    newImport->name = name;
    newImport->nextPtr = *import;
    *import = newImport;
	
    return *import;
}



static void
createImportList(SmiModule *smiModule)
{
    SmiNode     *smiNode;
    SmiType     *smiType;
    SmiModule   *smiTypeModule;
    SmiNodekind kind = SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN;
    SmiImport   *smiImport;
    
    for (smiNode = smiGetFirstNode(smiModule, kind);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, kind)) {

	smiType = smiGetNodeType(smiNode);
	if (smiType) {
	    smiTypeModule = smiGetTypeModule(smiType);
	    if (smiTypeModule &&
		strcmp(smiTypeModule->name, smiModule->name)) {
		if (strlen(smiTypeModule->name)) {
		    addImport(smiTypeModule->name, smiType->name);
		}
	    }
	    if (smiType->basetype == SMI_BASETYPE_INTEGER32) {
		addImport("SNMPv2-SMI", "Integer32");
	    }
	}
    }

    for (smiImport = smiGetFirstImport(smiModule); smiImport;
	 smiImport = smiGetNextImport(smiImport)) {
	if (islower((int) smiImport->name[0]) ||
	    (smiImport->module && !strcmp(smiImport->module, "SNMPv2-SMI")) ||
	    (smiImport->module && !strcmp(smiImport->module, "SNMPv2-TC"))) {
	    addImport(smiImport->module, smiImport->name);
	}
    }
}



static void
freeImportList(void)
{
    Import *import, *freeme;

    for (import = importList; import; ) {
	freeme = import;
	import = import->nextPtr;
	xfree(freeme);
    }
    importList = NULL;
}



static void
fprint(FILE *f, char *fmt, ...)
{
    va_list ap;
    char    s[200];
    char    *p;
    
    va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
    current_column += vsnprintf(s, sizeof(s), fmt, ap);
#else
    current_column += vsprintf(s, fmt, ap);	/* buffer overwrite */
#endif
    va_end(ap);

    fputs(s, f);

    if ((p = strrchr(s, '\n'))) {
	current_column = strlen(p) - 1;
    }
}



static void
fprintSegment(FILE *f, int column, char *string, int length)
{
    fprint(f, "%*c%s", column, ' ', string);
    if (length) {
	fprint(f, "%*c", length - strlen(string) - column, ' ');
    }
}



static void
fprintWrapped(FILE *f, int column, char *string)
{
    if ((current_column + strlen(string)) > INDENTMAX) {
	putc('\n', f);
	current_column = 0;
	fprintSegment(f, column, "", 0);
    }
    fprint(f, "%s", string);
}



static void
fprintMultilineString(FILE *f, int column, const char *s)
{
    int i, len;
    
    fprintSegment(f, column - 1 + INDENTTEXTS, "\"", 0);
    if (s) {
	len = strlen(s);
	for (i=0; i < len; i++) {
	    putc(s[i], f);
	    current_column++;
	    if (s[i] == '\n') {
		current_column = 0;
		fprintSegment(f, column + INDENTTEXTS, "", 0);
	    }
	}
    }
    putc('\"', f);
    current_column++;
}



static void
fprintComment(FILE *f, int indent, char *comment)
{
    if (indent) {
	fprintSegment(f, indent, SMIV3COMMENT, 0);
    } else {
	fprint(f, SMIV3COMMENT);
    }
    fprint(f, comment);
}



static char
*getValueString(SmiValue *valuePtr, SmiType *typePtr)
{
    static char    s[100];
    char           ss[9];
    int		   n;
    unsigned int   i;
    SmiNamedNumber *nn;
    SmiNode        *nodePtr;
    
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
	    sprintf(s, "\"%s\"", valuePtr->value.ptr);
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
	nodePtr = smiGetNodeByOID(valuePtr->len, valuePtr->value.oid);
	if (nodePtr) {
	    sprintf(s, "%s", nodePtr->name);
	} else {
	    strcpy(s, "");
	    for (i=0; i < valuePtr->len; i++) {
		if (i) strcat(s, ".");
		sprintf(&s[strlen(s)], "%u", valuePtr->value.oid[i]);
	    }
	}
	break;
    }

    return s;
}



static int
isAggregate(SmiNode *smiNode)
{
    SmiNode *childNode;
    
    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {
	if (childNode->nodekind == SMI_NODEKIND_SCALAR
	    || childNode->nodekind == SMI_NODEKIND_ROW) {
	    return 1;
	}
    }

    return 0;
}



static void
fprintSubtype(FILE *f, SmiType *smiType)
{
    SmiRange       *range;
    SmiNamedNumber *nn;
    char	   s[100];
    int		   i;

    if ((smiType->basetype == SMI_BASETYPE_ENUM) ||
	(smiType->basetype == SMI_BASETYPE_BITS)) {
	for(i = 0, nn = smiGetFirstNamedNumber(smiType);
	    nn ; i++, nn = smiGetNextNamedNumber(nn)) {
	    if (i) {
		fprint(f, ", ");
	    } else {
		fprint(f, " { ");
	    }
	    sprintf(s, "%s(%s)", nn->name,
		    getValueString(&nn->value, smiType));
	    fprintWrapped(f, INDENTVALUE + INDENT, s);
	}
	if (i) {
	    fprint(f, " }");
	}
    } else {
	for(i = 0, range = smiGetFirstRange(smiType);
	    range ; i++, range = smiGetNextRange(range)) {
	    if (i) {
		fprint(f, " | ");
	    } else {
		fprint(f, " (");
	    }	    
	    if (memcmp(&range->minValue, &range->maxValue,
		       sizeof(SmiValue))) {
		sprintf(s, "%s", getValueString(&range->minValue, smiType));
		sprintf(&s[strlen(s)], "..%s", 
			getValueString(&range->maxValue, smiType));
	    } else {
		sprintf(s, "%s", getValueString(&range->minValue, smiType));
	    }
	    fprintWrapped(f, INDENTVALUE + INDENT, s);
	}
	if (i) {
	    fprint(f, ")");
	}
    }
}



static void
fprintImports(FILE *f, SmiModule *smiModule)
{
    Import *import;
    char   *lastModuleName = NULL;
    int    pos = 0, len;

    createImportList(smiModule);

    for (import = importList; import; import = import->nextPtr) {
	int yaba = !lastModuleName || strcmp(import->module, lastModuleName);
	if (yaba) {
	    if (lastModuleName) {
		fprint(f, " FROM %s\n\n", lastModuleName);
	    }
	    fprintSegment(f, INDENT, "", 0);
	    fprint(f, "IMPORT ");
	    pos = INDENT + 12;
	} else {
	    fprint(f, ", ");
	}
	len = strlen(import->name);
	if (len + pos > INDENTMAX) {
	    fprint(f, "\n");
	    fprintSegment(f, INDENT, "", 11);
	    pos = INDENT + 12;
	}
	fprint(f, "%s", import->name);
	pos += len;
	lastModuleName = import->module;
    }

    if (lastModuleName) {
	fprint(f, " FROM %s\n\n", lastModuleName);
    }

    freeImportList();
}



static void
fprintFormat(FILE *f, int level, char *format)
{
    if (format) {
	fprintSegment(f, level * INDENT, "FORMAT", INDENTVALUE);
	fprint(f, "\"%s\"\n", format);
    }
}



static void
fprintUnits(FILE *f, int level, char *units)
{
    if (units) {
	fprintSegment(f, level * INDENT, "UNITS", INDENTVALUE);
	fprint(f, "\"%s\"\n", units);
    }
}



static void
fprintAccess(FILE *f, int level, SmiAccess access)
{
    fprintSegment(f, level * INDENT, "ACCESS", INDENTVALUE);
    fprint(f, "%s\n", getAccessString(access));
}


	
static void
fprintStatus(FILE *f, int level, SmiStatus status)
{
    if ((status != SMI_STATUS_UNKNOWN) &&
	(status != SMI_STATUS_MANDATORY) &&
	(status != SMI_STATUS_OPTIONAL)) {
	fprintSegment(f, level * INDENT, "STATUS", INDENTVALUE);
	fprint(f, "%s\n", getStringStatus(status));
    }
}


	
static void
fprintOrganization(FILE *f, int level, char *organization)
{
    fprintSegment(f, level * INDENT, "ORGANIZATION", 0);
    fprint(f, "\n");
    fprintMultilineString(f, level * INDENT, organization);
    fprint(f, "\n");
}



static void
fprintContact(FILE *f, int level, char *contact)
{
    fprintSegment(f, level * INDENT, "CONTACT", 0);
    fprint(f, "\n");
    fprintMultilineString(f, level * INDENT, contact);
    fprint(f, "\n");
}



static void
fprintDescription(FILE *f, int level, char *description)
{
    fprintSegment(f, level * INDENT, "DESCRIPTION", 0);
    fprint(f, "\n");
    fprintMultilineString(f, level * INDENT, description);
    fprint(f, "\n");
}



static void
fprintReference(FILE *f, int level, char *reference)
{
    if (reference) {
	fprintSegment(f, level * INDENT, "REFERENCE", 0);
	fprint(f, "\n");
	fprintMultilineString(f, level * INDENT, reference);
	fprint(f, "\n");
    }
}



static void
fprintRevisions(FILE *f, SmiModule *smiModule)
{
    int i;
    SmiRevision *smiRevision;
    
    for (i = 0, smiRevision = smiGetFirstRevision(smiModule);
	smiRevision; smiRevision = smiGetNextRevision(smiRevision)) {
	if (i) {
	    fprint(f, "\n");
	}
	fprintSegment(f, INDENT, "REVISION {\n", 0);
	fprintSegment(f, 2 * INDENT, "DATE", INDENTVALUE);
	fprint(f, "\"%s\"\n", getStringTime(smiRevision->date));
	fprintDescription(f, 2, smiRevision->description);
        fprintSegment(f, INDENT, "}\n", 0);
	i++;
    }
    if (i) {
	fprint(f, "\n");
    }
}



static void
fprintTypedef(FILE *f, SmiType *smiType, SmiNode *smiNode)
{
    fprintSegment(f, INDENT, "", 0);
    fprint(f, "TYPE %s {\n", smiType->name);
    
    fprintSegment(f, 2 * INDENT, "SYNTAX", INDENTVALUE);
    fprint(f, "%s", getTypeString(smiType->basetype,
				  smiGetParentType(smiType)));
    fprintSubtype(f, smiType);
    fprint(f, "\n");

    fprintFormat(f, 2, smiType->format);
    fprintUnits(f, 2, smiType->units);
    
    if (smiType->value.basetype != SMI_BASETYPE_UNKNOWN) {
	fprintSegment(f, 2 * INDENT, "DEFAULT", INDENTVALUE);
 	fprint(f, "%s", getValueString(&smiType->value, smiType));
	fprint(f, "\n");
    }
    
    fprintStatus(f, 2, smiType->status);
    fprintDescription(f, 2, smiType->description);
    fprintReference(f, 2, smiType->reference);
    fprintSegment(f, INDENT, "}\n\n", 0);
}



static void
fprintTypedefs(FILE *f, SmiModule *smiModule)
{
    int		 i, j;
    SmiType	 *smiType;
    
    for(i = 0, smiType = smiGetFirstType(smiModule);
	smiType; smiType = smiGetNextType(smiType)) {
	
	for (j = 0; excludeType[j]; j += 2) {
	    if ((!strcmp(smiModule->name, excludeType[j]))
		&& (!strcmp(smiType->name, excludeType[j+1]))) {
		break;
	    }
	}
	if (excludeType[j]) continue;

	if (! smiType->name) continue;
	    
	if (!i && !silent) {
	    fprintComment(f, INDENT, "\n");
	    fprintComment(f, INDENT, " DERIVED TYPE DEFINITIONS\n");
	    fprintComment(f, INDENT, "\n\n");
	}

	fprintTypedef(f, smiType, NULL);
	i++;
    }

    /* XXX add refinement type definitions */
}



static void
fprintAttribute(FILE *f, SmiNode *smiNode)
{
    SmiType *smiType;
    
    fprintSegment(f, 2 * INDENT, "", 0);
    fprint(f, "ATTRIBUTE %s {\n", smiNode->name);
    smiType = smiGetNodeType(smiNode);
    
    if (smiType) {
	fprintSegment(f, 3 * INDENT, "SYNTAX", INDENTVALUE);
	if (! smiType->name) {
	    /*
	     * an implicitly restricted type.
	     */
	    fprint(f, "%s", getTypeString(smiType->basetype,
					  smiGetParentType(smiType)));
	    fprintSubtype(f, smiType);
	    fprint(f, "\n");
	} else {
	    fprint(f, "%s\n",
		   getTypeString(smiType->basetype, smiType));
	}
    }

    fprintFormat(f, 3, smiNode->format);
    fprintUnits(f, 3, smiNode->units);
    
    if (smiNode->value.basetype != SMI_BASETYPE_UNKNOWN) {
	fprintSegment(f, 3 * INDENT, "DEFAULT", INDENTVALUE);
	fprint(f, "%s", getValueString(&smiNode->value, smiType));
	fprint(f, "\n");
    }

    fprintAccess(f, 3, smiNode->access);
    fprintStatus(f, 3, smiNode->status);
    fprintDescription(f, 3, smiNode->description);
    fprintReference(f, 3, smiNode->reference);
    fprintSegment(f, 2 * INDENT, "", 0);
    fprint(f, "} = %u\n", smiNode->oid[smiNode->oidlen-1]);
}



static void
fprintAttributes(FILE *f, SmiNode *smiNode)
{
    SmiNode *childNode;
    int i = 0;

    if (smiNode->nodekind == SMI_NODEKIND_TABLE) {
	smiNode = smiGetFirstChildNode(smiNode);
    }

    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode; childNode = smiGetNextChildNode(childNode)) {
	if (childNode->nodekind != SMI_NODEKIND_SCALAR
	    && childNode->nodekind != SMI_NODEKIND_COLUMN) {
	    continue;
	}
	if (i) {
	    fprint(f, "\n");
	}
	if (childNode->name) {
	    fprintAttribute(f, childNode);
	}
	i++;
    }
}



static void
fprintIndex(FILE *f, SmiNode *smiNode)
{
    SmiNode *relatedNode;
    SmiElement *smiElement;
    int j;

    relatedNode = smiGetRelatedNode(smiNode);
    switch (smiNode->indexkind) {
    case SMI_INDEX_INDEX:
	if (smiNode->implied) {
	    fprintSegment(f, 2 * INDENT, "INDEX IMPLIED",
			  INDENTVALUE);
	} else {
	    fprintSegment(f, 2 * INDENT, "INDEX", INDENTVALUE);
	}
	fprint(f, "{ ");
	for (j = 0, smiElement = smiGetFirstElement(smiNode); smiElement;
	     j++, smiElement = smiGetNextElement(smiElement)) {
	    if (j) {
		fprint(f, ", ");
	    }
	    fprintWrapped(f, INDENTVALUE + 1,
			  smiGetElementNode(smiElement)->name);
	    /* TODO: non-local name if non-local */
	} /* TODO: empty? -> print error */
	fprint(f, " }\n");
	break;
    case SMI_INDEX_AUGMENT:
	if (relatedNode) {
	    fprintSegment(f, 2 * INDENT, "AUGMENTS",
			  INDENTVALUE);
	    fprint(f, "{ %s }\n", relatedNode->name);
	    /* TODO: non-local name if non-local */
	} /* TODO: else print error */
	break;
    case SMI_INDEX_REORDER:
	if (relatedNode) {
	    fprintSegment(f, 2 * INDENT, "", 0);
	    fprint(f, "REORDERS %s", relatedNode->name);
	    /* TODO: non-local name if non-local */
	    if (smiNode->implied) {
		fprint(f, " IMPLIED");
	    }
	    fprint(f, " {");
	    for (j = 0, smiElement = smiGetFirstElement(smiNode);
		 smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
		if (j) {
		    fprint(f, ", ");
		}
		fprintWrapped(f, INDENTVALUE + 1,
			      smiGetElementNode(smiElement)->name);
		/* TODO: non-local name if non-local */
	    } /* TODO: empty? -> print error */
	    fprint(f, " }\n");
	} /* TODO: else print error */
	break;
    case SMI_INDEX_SPARSE:
	if (relatedNode) {
	    fprintSegment(f, 2 * INDENT, "SPARSE-AUGMENTS", INDENTVALUE);
	    fprint(f, "{ %s }\n", relatedNode->name);
	    /* TODO: non-local name if non-local */
	} /* TODO: else print error */
	break;
    case SMI_INDEX_EXPAND:
	if (relatedNode) {
	    fprintSegment(f, 2 * INDENT, "", 0);
	    fprint(f, "EXPANDS %s", relatedNode->name);
	    /* TODO: non-local name if non-local */
	    if (smiNode->implied) {
		fprint(f, " IMPLIED");
	    }
	    fprint(f, " { ");
	    for (j = 0, smiElement = smiGetFirstElement(smiNode);
		 smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
		if (j) {
		    fprint(f, ", ");
		}
		fprintWrapped(f, INDENTVALUE + 1,
			      smiGetElementNode(smiElement)->name);
		/* TODO: non-local name if non-local */
	    } /* TODO: empty? -> print error */
	    fprint(f, " }\n");
	} /* TODO: else print error */
	break;
    case SMI_INDEX_UNKNOWN:
	break;
    }
}



static void
fprintAggregate(FILE *f, SmiNode *smiNode)
{
    char *name;
    
    if (smiNode->nodekind == SMI_NODEKIND_TABLE) {
	smiNode = smiGetFirstChildNode(smiNode);
    }

    name = getUppercaseString(smiNode->name);
    
    fprintSegment(f, INDENT, "", 0);
    fprint(f, "STRUCT %s {\n", name);

    fprintStatus(f, 2, smiNode->status);
    fprintDescription(f, 2, smiNode->description);
    fprintReference(f, 2, smiNode->reference);
    fprint(f, "\n");
    fprintAttributes(f, smiNode);

    fprintSegment(f, INDENT, "", 0);
    fprint(f, "}\n\n");

    xfree(name);
}



static void
fprintAggregates(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    int i = 0;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (isAggregate(smiNode)) {
	    if (! i) {
		if (! silent) {
		    fprintComment(f, INDENT, "\n");
		    fprintComment(f, INDENT, " AGGREGATE TYPE DEFINITIONS\n");
		    fprintComment(f, INDENT, "\n\n");
		}
	    }
	    fprintAggregate(f, smiNode);
	    i++;
	}
    }
}



static void
fprintNode(FILE *f, SmiNode *smiNode)
{
    fprintSegment(f, INDENT, "", 0);
    fprint(f, "NODE %s = %s\n", smiNode->name,
	   getOidString(smiNode, 0));
}



static void
fprintNodes(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    int i = 0;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NODE);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NODE)) {
	if (! isAggregate(smiNode) && smiNode->status == SMI_STATUS_UNKNOWN) {
	    if (!i && !silent) {
		fprintComment(f, INDENT, "\n");
		fprintComment(f, INDENT, " NODE DEFINITIONS\n");
		fprintComment(f, INDENT, "\n\n");
	    }
	    fprintNode(f, smiNode);
	    i++;
	}
    }

    if (i) {
	fprint(f, "\n");
    }
}



static void
fprintIdentity(FILE *f, SmiNode *smiNode)
{
    fprintSegment(f, INDENT, "", 0);
    fprint(f, "IDENTITY %s {\n", smiNode->name,
	   getOidString(smiNode, 0));
    fprintStatus(f, 2, smiNode->status);
    fprintDescription(f, 2, smiNode->description);
    fprintReference(f, 2, smiNode->reference);
    fprintSegment(f, INDENT, "", 0);
    fprint(f, "} = %s\n", getOidString(smiNode, 0));
}



static void
fprintIdentities(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    int i = 0;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NODE);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NODE)) {
	if (! isAggregate(smiNode) && smiNode->status != SMI_STATUS_UNKNOWN) {
	    if (!i && !silent) {
		fprintComment(f, INDENT, "\n");
		fprintComment(f, INDENT, " IDENTITY DEFINITIONS\n");
		fprintComment(f, INDENT, "\n\n");
	    }
	    if (i) {
		fprint(f, "\n");
	    }
	    fprintIdentity(f, smiNode);
	    i++;
	}
    }

    if (i) {
	fprint(f, "\n");
    }
}



static void
fprintScalar(FILE *f, SmiNode *smiNode)
{
    fprintSegment(f, INDENT, "", 0);
    fprint(f, "VAR STRUCT %s {\n", smiNode->name);
    fprintSegment(f, 2 * INDENT, "SYNTAX", INDENTVALUE);
    fprint(f, "%s\n", smiNode->name);
    fprintStatus(f, 2, smiNode->status);
    fprintDescription(f, 2, smiNode->description);
    fprintReference(f, 2, smiNode->reference);
    fprintSegment(f, INDENT, "", 0);
    fprint(f, "} = %s\n\n", getOidString(smiNode, 0));
}



static void
fprintTable(FILE *f, SmiNode *smiNode)
{
    SmiNode *childNode;
    char *name;
    
    fprintSegment(f, INDENT, "", 0);
    fprint(f, "VAR STRUCT %s {\n", smiNode->name);
    childNode = smiGetFirstChildNode(smiNode);
    if (childNode && childNode->name) {
	name = getUppercaseString(childNode->name);
	fprintSegment(f, 2 * INDENT, "SYNTAX", INDENTVALUE);
	fprint(f, "%s\n", name);
	xfree(name);
    }
    fprintStatus(f, 2, smiNode->status);
    fprintDescription(f, 2, smiNode->description);
    fprintReference(f, 2, smiNode->reference);
    fprintIndex(f, childNode);
    fprintSegment(f, INDENT, "", 0);
    fprint(f, "} = %s\n\n", getOidString(smiNode, 0));
}



static void
fprintVariables(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    int i = 0;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NODE);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NODE)) {
	if (isAggregate(smiNode)) {
	    if (!i && !silent) {
		fprintComment(f, INDENT, "\n");
		fprintComment(f, INDENT, " VARIABLE DEFINITIONS\n");
		fprintComment(f, INDENT, "\n\n");
	    }
	    fprintScalar(f, smiNode);
	    i++;
	}
    }
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_TABLE);
	 smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_TABLE)) {
	if (!i && !silent) {
	    fprintComment(f, INDENT, "\n");
	    fprintComment(f, INDENT, " VARIABLE DEFINITIONS\n");
	    fprintComment(f, INDENT, "\n\n");
	}
	fprintTable(f, smiNode);
	i++;
    }
}



static void
fprintNotifications(FILE *f, SmiModule *smiModule)
{
    int		 i, j;
    SmiNode	 *smiNode;
    SmiElement   *smiElement;
    
    for (i = 0, smiNode = smiGetFirstNode(smiModule,
					 SMI_NODEKIND_NOTIFICATION);
	smiNode;
	smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {

	if (!i && !silent) {
	    fprintComment(f, INDENT, "\n");
	    fprintComment(f, INDENT, " NOTIFICATION DEFINITIONS\n");
	    fprintComment(f, INDENT, "\n\n");
	}

	fprintSegment(f, INDENT, "", 0);
	fprint(f, "NOTIFICATION %s {\n", smiNode->name);

	if ((smiElement = smiGetFirstElement(smiNode))) {
	    fprintSegment(f, 2 * INDENT, "OBJECTS", INDENTVALUE);
	    fprint(f, "{ ");
	    for (j = 0; smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
		if (j) {
		    fprint(f, ", ");
		}
		fprintWrapped(f, INDENTVALUE + 1,
			      smiGetElementNode(smiElement)->name);
		/* TODO: non-local name if non-local */
	    } /* TODO: empty? -> print error */
	    fprint(f, " }\n");
	}

	fprintStatus(f, 2, smiNode->status);
	fprintDescription(f, 2, smiNode->description);
	fprintReference(f, 2, smiNode->reference);

	fprintSegment(f, INDENT, "", 0);
	fprint(f, "} = %s\n\n", getOidString(smiNode, 0));
	i++;
    }
}



static void
fprintGroups(FILE *f, SmiModule *smiModule)
{
    int		 i, j;
    SmiNode	 *smiNode;
    SmiElement   *smiElement;
    
    for(i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_GROUP);
	smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_GROUP)) {
	
	if (!i && !silent) {
	    fprintComment(f, INDENT, "\n");
	    fprintComment(f, INDENT, " GROUP DEFINITIONS\n");
	    fprintComment(f, INDENT, "\n\n");
	}
	
	fprintSegment(f, INDENT, "", 0);
	fprint(f, "GROUP %s {\n", smiNode->name);
	
	fprintSegment(f, 2 * INDENT, "MEMBERS", INDENTVALUE);
	fprint(f, "{ ");
	for (j = 0, smiElement = smiGetFirstElement(smiNode); smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
	    if (j) {
		fprint(f, ", ");
	    }
	    fprintWrapped(f, INDENTVALUE + 2,
			  smiGetElementNode(smiElement)->name);
	    /* TODO: non-local name if non-local */
	} /* TODO: empty? -> print error */
	fprint(f, " }\n");

	fprintStatus(f, 2, smiNode->status);
	fprintDescription(f, 2, smiNode->description);
	fprintReference(f, 2, smiNode->reference);
	
	fprintSegment(f, INDENT, "", 0);
	fprint(f, "} = %s\n\n", getOidString(smiNode, 0));
	i++;
    }
}


#if 0
static void
fprintCompliances(FILE *f, SmiModule *smiModule)
{
    int		  i, j;
    SmiNode	  *smiNode, *smiNode2;
    SmiType	  *smiType;
    SmiOption	  *smiOption;
    SmiRefinement *smiRefinement;
    SmiElement    *smiElement;
    
    for(i = 0, smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_COMPLIANCE);
	smiNode; smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COMPLIANCE)) {
	
	if (!i && !silent) {
	    fprint(f, "//\n// COMPLIANCE DEFINITIONS\n//\n\n");
	}

	fprintSegment(f, INDENT, "", 0);
	fprint(f, "compliance %s {\n", smiNode->name);
	    
	if (smiNode->oid) {
	    fprintSegment(f, 2 * INDENT, "oid", INDENTVALUE);
	    fprint(f, "%s;\n", getOidString(smiNode, 0));
	}
	    
	if ((smiNode->status != SMI_STATUS_CURRENT) &&
	    (smiNode->status != SMI_STATUS_UNKNOWN) &&
	    (smiNode->status != SMI_STATUS_MANDATORY) &&
	    (smiNode->status != SMI_STATUS_OPTIONAL)) {
	    fprintSegment(f, 2 * INDENT, "status", INDENTVALUE);
	    fprint(f, "%s;\n", getStringStatus(smiNode->status));
	}
	    
	if (smiNode->description) {
	    fprintSegment(f, 2 * INDENT, "description", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiNode->description);
	    fprint(f, ";\n");
	}
	    
	if (smiNode->reference) {
	    fprintSegment(f, 2 * INDENT, "reference", INDENTVALUE);
	    fprint(f, "\n");
	    fprintMultilineString(f, 2 * INDENT, smiNode->reference);
	    fprint(f, ";\n");
	}

	if ((smiElement = smiGetFirstElement(smiNode))) {
	    fprint(f, "\n");
	    fprintSegment(f, 2 * INDENT, "mandatory", INDENTVALUE);
	    fprint(f, "(");
	    for (j = 0; smiElement;
		 j++, smiElement = smiGetNextElement(smiElement)) {
		if (j) {
		    fprint(f, ", ");
		}
		fprintWrapped(f, INDENTVALUE + 1,
			      smiGetElementNode(smiElement)->name);
		/* TODO: non-local name if non-local */
	    } /* TODO: empty? -> print error */
	    fprint(f, ");\n");
	}
	
	if ((smiOption = smiGetFirstOption(smiNode))) {
	    fprint(f, "\n");
	    for(; smiOption; smiOption = smiGetNextOption(smiOption)) {
		smiNode2 = smiGetOptionNode(smiOption);
		fprintSegment(f, 2 * INDENT, "", 0);
		fprint(f, "optional %s {\n", smiNode2->name);
		fprintSegment(f, 3 * INDENT, "description", INDENTVALUE);
		fprint(f, "\n");
		fprintMultilineString(f, 3 * INDENT, smiOption->description);
		fprint(f, ";\n");
		fprintSegment(f, 2 * INDENT, "};\n", 0);
	    }
	}
	
	if ((smiRefinement = smiGetFirstRefinement(smiNode))) {
	    fprint(f, "\n");
	    for(; smiRefinement;
		smiRefinement = smiGetNextRefinement(smiRefinement)) {
		fprintSegment(f, 2 * INDENT, "", 0);
		fprint(f, "refine %s {\n",
		       smiGetRefinementNode(smiRefinement)->name);

		smiType = smiGetRefinementType(smiRefinement);
		if (smiType) {
		    fprintSegment(f, 3 * INDENT, "type", INDENTVALUE);
		    fprint(f, "%s",
			   getTypeString(smiType->basetype,
					 smiGetParentType(smiType)));
		    fprintSubtype(f, smiType);
		    fprint(f, ";\n");
		}

		smiType = smiGetRefinementWriteType(smiRefinement);
		if (smiType) {
		    fprintSegment(f, 3 * INDENT, "writetype", INDENTVALUE);
		    fprint(f, "%s",
			   getTypeString(smiType->basetype,
					 smiGetParentType(smiType)));
		    fprintSubtype(f, smiType);
		    fprint(f, ";\n");
		}

		if (smiRefinement->access != SMI_ACCESS_UNKNOWN) {
		    fprintSegment(f, 3 * INDENT, "access", INDENTVALUE);
		    fprint(f, "%s;\n", getAccessString(smiRefinement->access));
		}
		fprintSegment(f, 3 * INDENT, "description", INDENTVALUE);
		fprint(f, "\n");
		fprintMultilineString(f, 3 * INDENT,
				      smiRefinement->description);
		fprint(f, ";\n");
		fprintSegment(f, 2 * INDENT, "};\n", 0);
	    }
	}
	
	fprintSegment(f, INDENT, "", 0);
	fprint(f, "};\n\n");
	i++;
    }
}
#endif


static void
dumpSmiv3(int modc, SmiModule **modv, int flags, char *output)
{
    SmiModule   *smiModule;
    int		i;
    FILE	*f = stdout;

    silent = (flags & SMIDUMP_FLAG_SILENT);

    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    for (i = 0; i < modc; i++) {

	smiModule = modv[i];
    
	fprintComment(f, 0, "\n");
	fprintComment(f, 0, " This SMIv3 module has been generated by smidump "
		            SMI_VERSION_STRING ". Do not edit.\n");
	fprintComment(f, 0, "\n");
	fprintComment(f, 0, " It conforms to SMIv3 as defined in " SMIV3VERSION ".\n");
	fprintComment(f, 0, "\n\n");

	fprint(f, "MODULE %s {\n\n", smiModule->name);

	fprintImports(f, smiModule);
	
	if (! silent) {
	    fprintComment(f, INDENT, "\n");
	    fprintComment(f, INDENT, " MODULE META INFORMATION\n");
	    fprintComment(f, INDENT, "\n\n");
	}

	if (smiModule->organization) {
	    fprintOrganization(f, 1, smiModule->organization);
	    fprint(f, "\n");
	}
	if (smiModule->contactinfo) {
	    fprintContact(f, 1, smiModule->contactinfo);
	    fprint(f, "\n");
	}
	if (smiModule->description) {
	    fprintDescription(f, 1, smiModule->description);
	    fprint(f, "\n");
	}
	if (smiModule->reference) {
	    fprintReference(f, 1, smiModule->reference);
	    fprint(f, "\n");
	}
	fprintRevisions(f, smiModule);

#if 0
	{
	    SmiNode	*smiNode;
	    smiNode = smiGetModuleIdentityNode(smiModule);
	    if (smiNode) {
		fprintSegment(f, INDENT, "IDENTITY", INDENTVALUE);
		fprint(f, "%s\n\n", smiNode->name);
	    }
	}
#endif
	
	fprintTypedefs(f, smiModule);
	fprintAggregates(f, smiModule);
	fprintNodes(f, smiModule);
	fprintIdentities(f, smiModule);
	fprintVariables(f, smiModule);
	fprintNotifications(f, smiModule);
	fprintGroups(f, smiModule);
#if 0
	fprintCompliances(f, smiModule);
#endif
	
	fprint(f, "}\n", smiModule->name);
    }

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void
initSmiv3()
{
    static SmidumpDriver driver = {
	"smiv3",
	dumpSmiv3,
	0,
	SMIDUMP_DRIVER_CANT_UNITE,
	"SMIv3 (experimental)",
	NULL,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
