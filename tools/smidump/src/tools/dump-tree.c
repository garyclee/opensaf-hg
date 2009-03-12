/*
 * dump-tree.c --
 *
 *      Operations to dump the OID tree in a human readable format.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-tree.c,v 1.34 2002/11/13 12:29:47 schoenw Exp $
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include "smi.h"
#include "smidump.h"


static int pmodc = 0;
static SmiModule **pmodv = NULL;

static int ignoreconformance = 0;
static int full = 0;


static char *getFlags(SmiNode *smiNode)
{

    switch (smiNode->access) {
    case SMI_ACCESS_UNKNOWN:
	return "---";
    case SMI_ACCESS_NOT_ACCESSIBLE:
	return "---";
    case SMI_ACCESS_NOTIFY:
	return "--n";
    case SMI_ACCESS_READ_ONLY:
	return "r-n";
    case SMI_ACCESS_READ_WRITE:
	return "rwn";
    case SMI_ACCESS_NOT_IMPLEMENTED:
	return "---";
    case SMI_ACCESS_INSTALL:
	return "-i-";
    case SMI_ACCESS_INSTALL_NOTIFY:
	return "-in";
    case SMI_ACCESS_REPORT_ONLY:
	return "--r";
    }

    return "";
}



static char getStatusChar(SmiStatus status)
{
    switch (status) {
    case SMI_STATUS_UNKNOWN:
	return '+';
    case SMI_STATUS_CURRENT:
	return '+';
    case SMI_STATUS_DEPRECATED:
	return 'x';
    case SMI_STATUS_MANDATORY:
        return '+';
    case SMI_STATUS_OPTIONAL:
	return '+';
    case SMI_STATUS_OBSOLETE:
	return 'o';
    }

    return ' ';
}



static char *getTypeName(SmiNode *smiNode)
{
    char *type;
    SmiType *smiType, *parentType;

    smiType = smiGetNodeType(smiNode);

    if (!smiType || smiNode->nodekind == SMI_NODEKIND_TABLE)
	return NULL;
    
    if (smiType->decl == SMI_DECL_IMPLICIT_TYPE) {
	parentType = smiGetParentType(smiType);
	if (!parentType)
	    return NULL;
	smiType = parentType;
    }

    type = xstrdup(smiType->name);
    return type;
}



static void fprintIndex(FILE *f, SmiNode *smiNode)
{
    char *indexname;
    int  i;
    SmiElement *smiElement;
    
    indexname = NULL;
    for (i = -1, smiElement = smiGetFirstElement(smiNode);
	 smiElement; smiElement = smiGetNextElement(smiElement), i++) {
	if (i > 0) fprintf(f, ",");
	if (indexname) {
	    fprintf(f, indexname);
	}
	indexname = smiGetElementNode(smiElement)->name;
    }
    if (indexname) {
	fprintf(f, "%s%s%s",
		(i > 0) ? "," : "",
		(smiNode->implied) ? "*" : "",
		indexname);
    }
}



static void fprintObjects(FILE *f, SmiNode *smiNode)
{
    char *objectname;
    int  i;
    SmiElement *smiElement;

    objectname = NULL;
    for (i = -1, smiElement = smiGetFirstElement(smiNode);
	 smiElement;
	 smiElement = smiGetNextElement(smiElement), i++) {
	if (i > 0) fprintf(f, ",");
	if (objectname) {
	    fprintf(f, objectname);
	}
	objectname = smiGetElementNode(smiElement)->name;
    }
    if (objectname) {
	fprintf(f, "%s%s", (i > 0) ? "," : "", objectname);
    }
}



static int pruneSubTree(SmiNode *smiNode)
{
    SmiModule *smiModule;
    SmiNode   *childNode;
    int       i;

    const int mask = (SMI_NODEKIND_GROUP | SMI_NODEKIND_COMPLIANCE);

    if (! smiNode) {
	return 1;
    }

    if (ignoreconformance && (smiNode->nodekind & mask)) {
	return 1;
    }

    smiModule = smiGetNodeModule(smiNode);
    for (i = 0; i < pmodc; i++) {
	if (strcmp(pmodv[i]->name, smiModule->name) == 0) {
	    if (!ignoreconformance || !smiGetFirstChildNode(smiNode)) {
		return 0;
	    }
	}
    }

    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {
	if (! pruneSubTree(childNode)) {
	    return 0;
	}
    }
    return 1;
}



static void fprintSubTree(FILE *f, SmiNode *smiNode,
			  char *prefix, size_t typefieldlen)
{
    SmiNode     *childNode, *indexNode;
    SmiNodekind lastNodeKind = SMI_NODEKIND_UNKNOWN;
    SmiType     *type;
    int         i = 0, cnt, prefixlen;
    size_t      newtypefieldlen = 9;
    char        c = 0;
    char	*type_name;

    if (smiNode) {
	prefixlen = strlen(prefix);
	switch (smiNode->nodekind) {
	case SMI_NODEKIND_SCALAR:
	case SMI_NODEKIND_COLUMN:
	    c = prefix[prefixlen-1];
	    prefix[prefixlen-1] = getStatusChar(smiNode->status);
	    type_name = getTypeName(smiNode);
	    if (type_name) {
		fprintf(f, "%s-- %s %-*s %s(%u)\n",
			prefix,
			getFlags(smiNode),
			typefieldlen,
			type_name,
			smiNode->name,
			smiNode->oid[smiNode->oidlen-1]);
		xfree(type_name);
	    }
	    if (c) {
		prefix[prefixlen-1] = c;
	    }
	    break;
	case SMI_NODEKIND_ROW:
	    if (prefixlen) {
		c = prefix[prefixlen-1];
		prefix[prefixlen-1] = getStatusChar(smiNode->status);
	    }
	    fprintf(f, "%s--%s(%u) [", prefix,
		    smiNode->name,
		    smiNode->oid[smiNode->oidlen-1]);
	    switch (smiNode->indexkind) {
	    case SMI_INDEX_INDEX:
	    case SMI_INDEX_REORDER:
		fprintIndex(f, smiNode);
		break;
	    case SMI_INDEX_EXPAND:  /* TODO: we have to do more work here! */
		break;
	    case SMI_INDEX_AUGMENT:
	    case SMI_INDEX_SPARSE:
		indexNode = smiGetRelatedNode(smiNode);
		if (indexNode) {
		    fprintIndex(f, indexNode);
		}
		break;
	    case SMI_INDEX_UNKNOWN:
		break;	    
	    }
	    fprintf(f, "]\n");
	    if (c) {
		prefix[prefixlen-1] = c;
	    }
	    break;
	case SMI_NODEKIND_NOTIFICATION:
	    if (prefixlen) {
		c = prefix[prefixlen-1];
		prefix[prefixlen-1] = getStatusChar(smiNode->status);
	    }
	    fprintf(f, "%s--%s(%u) [", prefix,
		    smiNode->name,
		    smiNode->oid[smiNode->oidlen-1]);
	    fprintObjects(f, smiNode);
	    fprintf(f, "]\n");
	    if (c) {
		prefix[prefixlen-1] = c;
	    }
	    break;
	default:
	    if (prefixlen) {
		c = prefix[prefixlen-1];
		prefix[prefixlen-1] = getStatusChar(smiNode->status);
	    }
	    if (smiNode->oid)
		if (prefixlen) {
		    fprintf(f, "%s--%s(%u)\n", prefix,
			    smiNode->name ? smiNode->name : " ",
			    smiNode->oid[smiNode->oidlen-1]);
		} else {
		    unsigned int j;
		    fprintf(f, "%s--%s(", prefix,
			    smiNode->name ? smiNode->name : " ");
		    for (j = 0; j < smiNode->oidlen; j++) {
			fprintf(f, "%s%u", j ? "." : "", smiNode->oid[j]);
		    }
		    fprintf(f, ")\n");
		}
	    else
		fprintf(f, "%s--%s(?)\n", prefix,
			smiNode->name ? smiNode->name : " ");
	    if (c) {
		prefix[prefixlen-1] = c;
	    }
	}
	for (childNode = smiGetFirstChildNode(smiNode), cnt = 0;
	     childNode;
	     childNode = smiGetNextChildNode(childNode)) {
	    if (! pruneSubTree(childNode)) {
		type = smiGetNodeType(childNode);
		if (type) {
		    type_name = getTypeName(childNode);
		    if (type_name) {
			if (strlen(type_name) > newtypefieldlen) {
			    newtypefieldlen = strlen(type_name);
			}
			xfree(type_name);
		    }
		}
		cnt++;
	    }
	}
	for (childNode = smiGetFirstChildNode(smiNode);
	     childNode;
	     childNode = smiGetNextChildNode(childNode)) {
	    char *newprefix;
	    if (pruneSubTree(childNode)) {
		continue;
	    }
	    i++;
	    if ((childNode->nodekind != SMI_NODEKIND_COLUMN
		&& childNode->nodekind != SMI_NODEKIND_SCALAR)
		|| (lastNodeKind != childNode->nodekind)) {
		fprintf(f, "%s  |\n", prefix);
	    }
	    newprefix = xmalloc(strlen(prefix)+10);
	    strcpy(newprefix, prefix);
	    if (cnt == 1 || cnt == i) {
		strcat(newprefix, "   ");
	    } else {
		strcat(newprefix, "  |");
	    }
	    fprintSubTree(f, childNode, newprefix, newtypefieldlen);
	    xfree(newprefix);
	    lastNodeKind = childNode->nodekind;
	}
    }
}



static void fprintTree(FILE *f)
{
    SmiNode *smiNode;
    SmiNode *childNode;
    SmiNode *nextNode;
    int cnt;
    
    smiNode = smiGetNode(NULL, "iso");

    if (! full) {
	do {
	    for (childNode = smiGetFirstChildNode(smiNode), cnt = 0, nextNode = NULL;
		 childNode;
		 childNode = smiGetNextChildNode(childNode)) {
		if (! pruneSubTree(childNode)) {
		    cnt++;
		    if (! nextNode) {
			nextNode = childNode;
		    }
		}
	    }
	    if (cnt == 1) {
		smiNode = nextNode;
	    }
	} while (cnt == 1);
    }
    
    if (smiNode) {
	fprintSubTree(f, smiNode, "", 0);
    }
}



static void dumpTree(int modc, SmiModule **modv, int flags, char *output)
{
    int     i;
    FILE    *f = stdout;
    
    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    if (flags & SMIDUMP_FLAG_UNITE) {
	
	pmodc = modc;
	pmodv = modv;
	
	if (! (flags & SMIDUMP_FLAG_SILENT)) {
	    fprintf(f, "# united registration tree (generated by smidump "
		    SMI_VERSION_STRING ")\n\n");
	}
	fprintTree(f);
	
    } else {

	for (i = 0; i < modc; i++) {

	    pmodc = 1;
	    pmodv = &(modv[i]);
	
	    if (! (flags & SMIDUMP_FLAG_SILENT)) {
		fprintf(f, "# %s registration tree (generated by smidump "
			SMI_VERSION_STRING ")\n\n", modv[i]->name);
	    }
	    fprintTree(f);
	}
    }

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}



void initTree()
{
    static SmidumpDriverOption opt[] = {
	{ "no-conformance", OPT_FLAG, &ignoreconformance, 0,
	  "do not show conformance definitions"},
	{ "full-root", OPT_FLAG, &full, 0,
	  "show the full path to the root"},
        { 0, OPT_END, 0, 0 }
    };
    
    static SmidumpDriver driver = {
	"tree",
	dumpTree,
	SMI_FLAG_NODESCR,
	0,
	"structure of the OID tree",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
