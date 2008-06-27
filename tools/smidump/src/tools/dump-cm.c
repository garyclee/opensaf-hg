/*
 * dump-cm.c --
 *
 *      Operations to dump conceptual models for MIB modules.
 *
 * Copyright (c) 2000 A. Mueller, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-cm.c,v 1.37 2002/10/30 09:17:37 schoenw Exp $
 */


/* 
 * -- TO DO --
 *
 * Berechnungen der UML Diagramme debuggen
 *
 */





#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "smidump.h"



#define ABS(a) ((a > 0.0) ? (a) : (-(a)))



typedef enum GraphCardinality {
    GRAPH_CARD_UNKNOWN            = 0,
    GRAPH_CARD_ONE_TO_ONE         = 1,
    GRAPH_CARD_ONE_TO_MANY        = 2,
    GRAPH_CARD_ZERO_TO_ONE        = 3,
    GRAPH_CARD_ZERO_TO_MANY       = 4,
    GRAPH_CARD_ONE_TO_ZERO_OR_ONE = 5
} GraphCardinality;

typedef enum GraphConnection {
    GRAPH_CON_UNKNOWN       = 0,
    GRAPH_CON_AGGREGATION   = 1,
    GRAPH_CON_DEPENDENCY    = 2,
    GRAPH_CON_ASSOCIATION   = 3
} GraphConnection;

typedef enum GraphEnhIndex {
    GRAPH_ENHINDEX_UNKNOWN      = 0,
    GRAPH_ENHINDEX_NOTIFICATION = 1,
    GRAPH_ENHINDEX_TYPES        = 2,
    GRAPH_ENHINDEX_NAMES        = 3,
    GRAPH_ENHINDEX_INDEX        = 4,
    GRAPH_ENHINDEX_REROUTE      = 5,
    GRAPH_ENHINDEX_POINTER      = 6
} GraphEnhIndex;

/*
 * Definition used by the dia output driver.
 */

#define DIA_PRINT_FLAG	0x01

typedef struct DiaNode {
    int   flags;		/* flags for the dia xml output driver */
    float x,y;			/* coordinates (left upper corner) */
    float w,h;			/* width and height of the dia node */
} DiaNode;

typedef struct DiaEdge {
    int   flags;		/* flags for the dia xml output driver */
} DiaEdge;

/*
 * Generic structures for the internal graph representation.
 */

typedef struct GraphNode {
    struct GraphNode *nextPtr;
    SmiNode          *smiNode;
    int              group;		/* group number of this graph node */
    DiaNode          dia;
} GraphNode;

typedef struct GraphEdge {
    struct GraphEdge *nextPtr;         
    GraphNode        *startNode;
    GraphNode        *endNode;
    SmiIndexkind     indexkind;
    GraphConnection  connection;
    GraphCardinality cardinality;
    GraphEnhIndex    enhancedindex;
    DiaEdge	     dia;
} GraphEdge;

typedef struct Graph {
    GraphNode *nodes;
    GraphEdge *edges;
} Graph;


/*
 * Constants used by the reverse engineering algorithm.
 */

static char *pointer[] = {
    "Index", NULL
};

static char *suffix[] = {
    "OrZero", NULL
};

static char *supportObjs[] = {
    "RowStatus", "StorageType", NULL
};

static char *baseTypes[] = {
    "Integer32", "OctetString", "Unsigned32", "Integer64",
    "Unsigned64", "Float32", "Float64", "Float128",
    "Enumeration", "Counter32", "Counter64","Bits",
    "Gauge", "Gauge32", "Integer", "TimeTicks",
    "IpAddress", "Opaque", "ObjectIdentifier",
    NULL
};

/*
 * Definitions used by the dia output driver (node layout).
 */

static const float HEADFONTSIZETABLE   = (float)0.51;
static const float HEADSPACESIZETABLE  = (float)0.6;
static const float ATTRFONTSIZE        = (float)0.48;
static const float ATTRSPACESIZE       = (float)2.4;
static const float RECTCORRECTION      = (float)0.85;
static const float EDGEYSPACING        = (float)2.0;
static const float TABLEHEIGHT         = (float)2.6;   /* headline of the table */
static const float TABLEELEMHEIGHT     = (float)0.675; /* height of one attribute */

/*
 * global dia graph layout
 */
static const float YSPACING            = (float)3.0;  /* y space between nodes */
static const float XSPACING            = (float)4.0;  /* x space between nodes */ 
static const float NEWLINEDISTANCE     = (float)40.0; /* length of one line */
static const float XOFFSET             = (float)2.0;  /* left upper start of graph */
static const float YOFFSET             = (float)5.0;  /* left upper start of graph */

/*
 * position of the dia info note 
 */
static const float XNOTE               = (float)1.0;  /* left upper corner of note */
static const float YNOTE               = (float)1.0;  /* left upper corner of note */

/*
 * Stereotype Name
 */
static const char* STEREOTYPE          = "smi mib class";

/*
 * Property String for index objects
 */
static const char* INDEXPROPERTY       = " {index}";

/*
 * driver output control
 */
static int       XPLAIN                = 0; /* false, generates ASCII output */
static int       DEBUG                 = 0; /* false, generates additional
					       output in xplain-mode */
static int       SUPPRESS_DEPRECATED   = 1; /* true, suppresses deprecated
					       objects */
static int       PRINT_DETAILED_ATTR   = 1; /* true, prints all column
					       objects */
static int       IGNORE_IMPORTED_NODES = 1; /* true, ignores nodes which are
					       imported from other MIBs*/

/*
 * global variables
 */
static Graph     *graph  = NULL;            /* the graph */

/*
 * help functions
 */
#ifndef max
#define max(a, b) ((a < b) ? b : a)
#endif
#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif





/* ------ Misc. -----------------                                            */





/*
 * cmpSmiNodes
 *
 * Compares two SmiNode and returns 1 if they are equal and 0 otherwise.
 */
static int cmpSmiNodes(SmiNode *node1, SmiNode *node2)
{
    SmiModule *module1, *module2;

    module1 = smiGetNodeModule(node1);
    module2 = smiGetNodeModule(node2);
    
    if (!node1 || !node2 || !module1 || !module2) return 0;
    
    return (strcmp(node1->name, node2->name) == 0 &&
	    strcmp(module1->name, module2->name) == 0);
}

/*
 * strpfxlen
 *
 * Returns the number of identical characters at the beginning of s1 and s2.
 */
static int strpfxlen(const char *s1, const char *s2)
{
    int i;

    for (i = 0; s1[i] && s2[i]; i++) {
	if (s1[i] != s2[i]) {
	    break;
	}
    }
    
    return i;
}





/* ------ Graph primitives ------                                            */





/*
 * graphInsertNode
 *
 *          Inserts a new node into an existing graph.
 *
 * Result : pointer to the new node
 */
static GraphNode *graphInsertNode(Graph *graph, SmiNode *smiNode)
{
    GraphNode *newNode;
    GraphNode *tNode;
    GraphNode *lastNode;

    newNode = xmalloc(sizeof(GraphNode));
    memset(newNode, 0, sizeof(GraphNode));
    newNode->smiNode = smiNode;

    if (graph->nodes == NULL) {
	graph->nodes = newNode;
	return newNode;
    }

    lastNode = NULL; 
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	lastNode = tNode;
    }

    lastNode->nextPtr = newNode;

    return newNode;
}

/*
 * graphInsertEdge
 *
 *          Inserts a new edge into an existing list of edges.
 *
 * Input  : graph     = pointer to an edge structure
 *          startNode = pointer to the starting node of the edge
 *          endNode   = pointer to the ending node of the edge
 *          indexkind = type of relationship between the two nodes
 *
 * Result : pointer to the new edge
 */
static GraphEdge *graphInsertEdge(Graph *graph, GraphNode *startNode,
			      GraphNode *endNode,
			      SmiIndexkind indexkind,
			      GraphEnhIndex enhancedindex)
{
    GraphEdge *newEdge;
    GraphEdge *tEdge;
    GraphEdge *lastEdge;

    newEdge = xmalloc(sizeof(GraphEdge));
    memset(newEdge, 0, sizeof(GraphEdge));
    newEdge->startNode = startNode;
    newEdge->endNode = endNode;
    newEdge->indexkind = indexkind;
    newEdge->connection = GRAPH_CON_ASSOCIATION;
    newEdge->enhancedindex = enhancedindex;

    switch (newEdge->indexkind) {
    case SMI_INDEX_AUGMENT : 
	newEdge->cardinality = GRAPH_CARD_ONE_TO_ONE; 
	break;
    case SMI_INDEX_SPARSE  : 
	newEdge->cardinality = GRAPH_CARD_ONE_TO_ZERO_OR_ONE; 
	break;
    case SMI_INDEX_EXPAND  : 
	newEdge->cardinality = GRAPH_CARD_UNKNOWN; 
	break;
    case SMI_INDEX_REORDER : 
	newEdge->cardinality = GRAPH_CARD_ONE_TO_ONE; 
	break;
    case SMI_INDEX_INDEX   : 
	newEdge->cardinality = GRAPH_CARD_UNKNOWN; 
	break;
    case SMI_INDEX_UNKNOWN : 
	newEdge->cardinality = GRAPH_CARD_UNKNOWN; 
	break;
    }

    if (graph->edges == NULL) {
	graph->edges = newEdge;
	return newEdge;
    }

    lastEdge = NULL; 
    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {
	lastEdge = tEdge;
    }

    lastEdge->nextPtr = newEdge;

    return newEdge;
}

/*
 * graphExit
 *
 * Frees all memory allocated by the graph.
 */
static void graphExit(Graph *graph)
{
    GraphEdge *tEdge, *dummyEdge;
    GraphNode *tNode, *dummyNode;

    if (graph) {

	dummyNode = NULL;
	tNode = graph->nodes;
	while (tNode != NULL) {
	    dummyNode = tNode;
	    tNode = tNode->nextPtr;
      
	    xfree(dummyNode);
	}
    
	dummyEdge = NULL;
	tEdge = graph->edges;
	while (tEdge != NULL) {
	    dummyEdge = tEdge;
	    tEdge = tEdge->nextPtr;
      
	    xfree(dummyEdge);
	}
    
	xfree(graph);
    }
}

/*
 * graphGetFirstEdgeByNode
 *
 * Returns the first edge adjacent to the given node (as startNode or EndNode).
 */
static GraphEdge *graphGetFirstEdgeByNode(Graph *graph, GraphNode *node)
{
    GraphEdge *tEdge;

    if (!graph || !node) {
	return NULL;
    }
    
    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {
	/*if (tEdge->startNode->smiNode->name == node->smiNode->name ||
	  tEdge->endNode->smiNode->name == node->smiNode->name) break;*/
	if (cmpSmiNodes(tEdge->startNode->smiNode, node->smiNode) ||
	    cmpSmiNodes(tEdge->endNode->smiNode, node->smiNode)) break;
    }

    return tEdge;
}

/*
 * graphGetNextEdgeByNode
 *
 * Returns the next edge adjacent to the given node (as startNode or EndNode)
 * after the given edge.
 */
static GraphEdge *graphGetNextEdgeByNode(Graph *graph, 
					 GraphEdge *edge,
					 GraphNode *node) 
{
    GraphEdge *tEdge;

    if (!graph || !node) {
	return NULL;
    }
    
    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {
	/*if (tEdge->startNode->smiNode->name ==
	    edge->startNode->smiNode->name &&
	    tEdge->endNode->smiNode->name ==
	    edge->endNode->smiNode->name) break;*/
	if (cmpSmiNodes(tEdge->startNode->smiNode,edge->startNode->smiNode) &&
	    cmpSmiNodes(tEdge->endNode->smiNode,edge->endNode->smiNode))
	    break;
    }
  
    for (tEdge = tEdge->nextPtr; tEdge; tEdge = tEdge->nextPtr) {
	/*if (tEdge->startNode->smiNode->name == node->smiNode->name ||
	  tEdge->endNode->smiNode->name == node->smiNode->name) break;*/
	if (cmpSmiNodes(tEdge->startNode->smiNode, node->smiNode) ||
	    cmpSmiNodes(tEdge->endNode->smiNode, node->smiNode)) break;
    }

    return tEdge;
}

/*
 * graphCheckForRedundantEdge
 *
 * Finds redundant edges and returns 1 if one is found and 0 otherwise.
 */
static int graphCheckForRedundantEdge(Graph *graph,
			       GraphNode *startNode,
			       GraphNode *endNode)
{
    GraphEdge *tEdge;

    if (!graph || !startNode || !endNode) {
	return 0;
    }
    
    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {
	/*if (tEdge->startNode->smiNode->name == startNode->smiNode->name &&
	  tEdge->endNode->smiNode->name == endNode->smiNode->name) {*/
	if (cmpSmiNodes(tEdge->startNode->smiNode, startNode->smiNode) &&
	    cmpSmiNodes(tEdge->endNode->smiNode, endNode->smiNode)) {
	    
	    return 1;
	}
    }
    
    return 0;
}

/*
 * graphGetNode
 *
 * Returns the graph node containing smiNode.
 */
static GraphNode *graphGetNode(Graph *graph, SmiNode *smiNode)
{
    GraphNode *tNode;

    if (!smiNode || !graph) {
	return NULL;
    }
    
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->smiNode->name == smiNode->name) {
	    break;
	}
    }

    return tNode;
}

/*
 * graphShowNodes
 *
 * Prints the nodes of the graph.
 */
static void graphShowNodes(Graph *graph) 
{
    GraphNode *tNode;
  
    if (!graph->nodes) {
	printf("No nodes!\n");
	return;
    }

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->smiNode->nodekind == SMI_NODEKIND_TABLE)
	    printf(" [TABLE]");
	else printf("[SCALAR]");
	printf("%40s [%s]\n", tNode->smiNode->name,
	       smiGetNodeModule(tNode->smiNode)->name);
    }
}

/*
 * graphShowEdges
 *
 * Prints the edges with their attributes in the following order :
 *    - start node
 *    - reason for the link
 *    - connection type in UML
 *    - cardinality
 *    - index relationship derived from the row-objects of the tables
 *
 */
static void graphShowEdges(Graph *graph) 
{
    GraphEdge  *tEdge;
    
    if (!graph->edges) {
	printf("No edges!\n");
	return;
    }
  
    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {

	if (DEBUG) {
	    switch (tEdge->enhancedindex) {
	    case GRAPH_ENHINDEX_UNKNOWN :
		printf("[UNKNOWN] ");
		break;
	    case GRAPH_ENHINDEX_NOTIFICATION :
		break;
	    case GRAPH_ENHINDEX_NAMES :
		printf("  [NAMES] ");
		break;
	    case GRAPH_ENHINDEX_TYPES :
		printf("  [TYPES] ");
		break;
	    case GRAPH_ENHINDEX_INDEX :
		printf("  [INDEX] ");
		break;
	    case GRAPH_ENHINDEX_REROUTE :
		printf("[REROUTE] ");
		break;
	    case GRAPH_ENHINDEX_POINTER :
		printf("[POINTER] ");
		break;	    
	    }
	}
	
	switch (tEdge->connection) {
	case GRAPH_CON_AGGREGATION:
	    printf("AG.");
	    break;
	case GRAPH_CON_DEPENDENCY:
	    printf("D. ");
	    break;
	case GRAPH_CON_ASSOCIATION:
	    printf("A. ");
	    break;    
	case GRAPH_CON_UNKNOWN:
	    break;
	}

	switch (tEdge->cardinality) {
	case GRAPH_CARD_UNKNOWN      :
	    printf("  (-:-)  ");
	    break;
	case GRAPH_CARD_ONE_TO_ONE   :
	    printf("  (1:1)  ");
	    break;
	case GRAPH_CARD_ONE_TO_MANY  :
	    printf("  (1:n)  ");
	    break;
	case GRAPH_CARD_ZERO_TO_ONE  :
	    printf("  (0:1)  ");
	    break;
	case GRAPH_CARD_ZERO_TO_MANY :
	    printf("  (0:n)  ");
	    break;
	case GRAPH_CARD_ONE_TO_ZERO_OR_ONE :
	    printf("(1:0..1) ");
	    break;	    
	}

	switch (tEdge->indexkind) {
	case SMI_INDEX_UNKNOWN  :
	    printf("GENERIC ");
	    break;
	case SMI_INDEX_INDEX    :
	    printf("  INDEX ");
	    break;
	case SMI_INDEX_AUGMENT  :
	    printf("AUGMENT ");
	    break;
	case SMI_INDEX_SPARSE   :
	    printf(" SPARSE ");
	    break;
	case SMI_INDEX_EXPAND   :
	    printf(" EXPAND ");
	    break;
	case SMI_INDEX_REORDER  :
	    printf("REORDER ");
	    break;
	}
	printf("%29s - ",tEdge->startNode->smiNode->name);
	printf("%s\n",tEdge->endNode->smiNode->name);
    }
}





/* ------ algorithm primitives ------                                        */



/*
 * algCountIndexElements
 *
 * Returns the number of index elements in a given row entry.
 */

static int algCountIndexElements(SmiNode *smiNode) 
{
    int          count;
    SmiElement   *smiElement;

    if (smiNode->nodekind != SMI_NODEKIND_ROW) {
	return 0;
    }
    
    count = 0;
    for (smiElement = smiGetFirstElement(smiNode);
	 smiElement;
	 smiElement = smiGetNextElement(smiElement)) {
	count++;
    }

    return count;
}

/*
 * algInsertEdge
 *
 * Inserts an edge in a given graph. The edge is adjacent to snode 
 * and enode.
 * The type of edge is given in indexkind and the enhanced index as
 * an additional information in enhancedindex.
 * Nodes which are not loaded yet into the node list of the graph
 * are added (nodes from imported MIBs).
 */
static void algInsertEdge(SmiNode *snode, SmiNode *enode, 
			  SmiIndexkind indexkind,
			  GraphEnhIndex enhancedindex) 
{
    GraphNode *startNode;
    GraphNode *endNode;

    if (!graph) return;

    startNode = graphGetNode(graph, snode);
    endNode   = graphGetNode(graph, enode);

    /* insert imported nodes into graph if needed */
    if (startNode == NULL) {
	if (IGNORE_IMPORTED_NODES) return;
	
	startNode = graphInsertNode(graph, snode);
    }
    if (endNode == NULL) {
	if (IGNORE_IMPORTED_NODES) return;
	
	endNode = graphInsertNode(graph, enode);
    }  

    if (graphCheckForRedundantEdge(graph, startNode, endNode) == 0 &&
	graphCheckForRedundantEdge(graph, endNode, startNode) == 0) {
	graphInsertEdge(graph, startNode, endNode, indexkind, enhancedindex); 
    }
}

/*
 * algGetGroupByFatherNode
 *
 * Returns the group number associated with the father node of the
 * given node. If there is no group the result is 0 for no
 * grouping.
 */
static int algGetGroupByFatherNode(SmiNode *smiNode)
{
    GraphNode *tNode;
    
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->smiNode->nodekind == SMI_NODEKIND_SCALAR &&
	    !graphGetFirstEdgeByNode(graph, tNode)) {
	    if (cmpSmiNodes(smiGetParentNode(smiNode),
			    smiGetParentNode(tNode->smiNode))) {
		return tNode->group;
	    }
	}
    }

    return 0;
}

/*
 * algGetNumberOfGroups
 *
 * Returns the number of groups.
 */
static int algGetNumberOfGroups()
{
    GraphNode *tNode;
    int       maxGroup;

    maxGroup = 0;

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	maxGroup = max(maxGroup, tNode->group); 
    }

    return maxGroup;
}

/*
 * algPrintGroup
 *
 * Prints the group with the number group.
 */
static void algPrintGroup(int group)
{
    GraphNode *tNode;
    
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->group == group) {
	    printf("%2d - %35s\n", group, tNode->smiNode->name);
	}
    }
}

/*
 * algGetTypeName
 *
 * Returns the name of the data type used in smiNode.
 */
static char *algGetTypeName(SmiNode *smiNode)
{
    SmiType *smiType, *parentType;
  
    smiType = smiGetNodeType(smiNode);
  
    if (!smiType || smiNode->nodekind == SMI_NODEKIND_TABLE)
	return NULL;
  
    if (smiType->decl == SMI_DECL_IMPLICIT_TYPE) {
	parentType = smiGetParentType(smiType);
	smiType = parentType;
    }
  
    return smiType->name;
}

/*
 * algCountElementsFromOtherTables
 *
 * Returns the number of index objects derived from other tables than
 * the given one.
 */
static int algCountElementsFromOtherTables(SmiNode *smiNode)
{
    SmiElement *smiElement;
    SmiNode    *tNode;
    int        elems = 0;
    
    for (smiElement = smiGetFirstElement(smiNode);
	 smiElement;
	 smiElement = smiGetNextElement(smiElement)) {

	tNode = smiGetElementNode(smiElement);
	
	if (cmpSmiNodes(smiGetParentNode(smiGetParentNode(tNode)),
			smiGetParentNode(smiNode)) != 1) {
	    elems++;
	}
    }

    return elems;
}

/*
 * algGetNumberOfRows
 *
 * Returns the number of rows =
 *   number of elements in smiNode - number of rows 
 *   + RowStatus-Objects + StorageType-Objects
 *
 * If the number is > 0 it is only a supporting table (like entLPMappingTable) 
 * and if the number is < 0 it is a "full" table (like ifTable).
 *
 * Subroutine for algCheckForDependency.
 */
static int algGetNumberOfRows(SmiNode *smiNode)
{
    SmiNode *tSmiNode;
    SmiNode *table;
    int     i, elemCount;

    elemCount = 0;
    table = smiNode;
    tSmiNode = smiGetFirstChildNode(table);

    elemCount = algCountIndexElements(tSmiNode) -
	algCountElementsFromOtherTables(tSmiNode);

    for (tSmiNode = smiGetNextNode(tSmiNode, SMI_NODEKIND_COLUMN);
	 tSmiNode;
	 tSmiNode = smiGetNextNode(tSmiNode, SMI_NODEKIND_COLUMN)) {

	if (cmpSmiNodes(table, smiGetParentNode(smiGetParentNode(tSmiNode)))
	    != 1) {
	    break;
	}
	
	for (i = 0; supportObjs[i]; i++) {
	    char *typeName = algGetTypeName(tSmiNode);
	    if (typeName && (strcasecmp(typeName, supportObjs[i]) == 0)) {
		break;
	    }
	}

	if (!supportObjs[i]) elemCount--;
    }

    return elemCount;
}

/*
 * isBaseType
 *
 * returns 1 if smiElement is a basetype used in SMIv2/SMIng. Otherwise
 * the result is 0.
 */
static int isBaseType(SmiNode *node)
{
    int i;

    for (i = 0; baseTypes[i]; i++) {
	if (strcasecmp(algGetTypeName(node), baseTypes[i]) == 0) {
	    return 1;
	}
    }

    return 0;
}

/*
 * algFindEqualType
 *
 * Looks in all tables indices for an equal type to the type used in typeNode.
 * It returns the first table found which is different to startTable.
 *
 * Subroutine for algCheckForDependency. 
 */
static SmiNode *algFindEqualType(SmiNode *startTable, SmiNode *typeNode)
{
    SmiElement *smiElement;
    SmiNode    *tSmiNode;
    char       *typeName;
    GraphNode  *tNode;
    
    typeName = algGetTypeName(typeNode);
    /* if (isBaseType(typeNode)) return NULL; */

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->smiNode->nodekind == SMI_NODEKIND_TABLE) {

	    tSmiNode = tNode->smiNode;
	    if (cmpSmiNodes(tSmiNode, startTable) == 1) break;
	    
	    for (smiElement = smiGetFirstElement(
		smiGetFirstChildNode(tSmiNode));
		 smiElement;
		 smiElement = smiGetNextElement(smiElement)) {
		/* found type matching ? */
		if (strcmp(typeName,
			       algGetTypeName(smiGetElementNode
					      (smiElement))) == 0) {
		    return tSmiNode;
		}
	    }
	    
	}
    }
    
    return NULL;
}

/*
 * 0 = totaly equal (order and objects)
 * 1 = equal objects but different order
 * 2 = inequal
 * 3 = error;
 */
 static int equalElements(SmiNode *firstRow, SmiNode *secondRow)
{
    int i,j,number1, number2;
    SmiElement *elemFirst, *elemSecond;

    if (!firstRow->nodekind == SMI_NODEKIND_ROW ||
	!secondRow->nodekind == SMI_NODEKIND_ROW) {
	/* printf("no row entries\n"); */
	return 3;
    }

    if (cmpSmiNodes(firstRow, secondRow) == 1) {
	/* printf("same objects\n"); */
	return 3;
    }
    
    number1 = algCountIndexElements(firstRow);
    number2 = algCountIndexElements(secondRow);

    if (number1 != number2) {
	/* printf("Different number of elements\n"); */
	return 2;
    }

    i = 0;
    for (elemFirst = smiGetFirstElement(firstRow);
	 elemFirst;
	 elemFirst = smiGetNextElement(elemFirst)) {
	i++;

	j = 0;
	for (elemSecond = smiGetFirstElement(secondRow);
	     elemSecond;
	     elemSecond = smiGetNextElement(elemSecond)) {
	    j++;
	    if (i == j && cmpSmiNodes(smiGetElementNode(elemFirst),
				      smiGetElementNode(elemSecond)) == 1) {
		break;
	    }
	}

	/* maybe same elements but in different order */
	if (!elemSecond) break;
    }
    if (!elemFirst) {
	/* printf("totaly equal\n"); */
	return 0;
    }

    for (elemFirst = smiGetFirstElement(firstRow);
	 elemFirst;
	 elemFirst = smiGetNextElement(elemFirst)) {

	for (elemSecond = smiGetFirstElement(secondRow);
	     elemSecond;
	     elemSecond = smiGetNextElement(elemSecond)) {

	    if (cmpSmiNodes(smiGetElementNode(elemFirst),
			    smiGetElementNode(elemSecond)) == 1) {
		break;
	    }
	}

	/* maybe same elements but in different order */
	if (!elemSecond) break;
    }

    if (!elemFirst) {
	/* printf("Equal\n"); */
	return 1;
    }
    else {
	/* printf("inequal"); */
	return 2;
    }
}

/*
 * algIsIndexElement
 *
 * Tests whether a given node is part of the index of a particular
 * table. Returns 1 if the node is an index node and 0 otherwise.
 */

static int algIsIndexElement(SmiNode *table, SmiNode *node)
{
    SmiElement *smiElement;
    SmiNode    *row;

    if (node->nodekind != SMI_NODEKIND_TABLE) {
	return 0;
    }
    
    row = smiGetFirstChildNode(table);
    if (!row || row->nodekind != SMI_NODEKIND_ROW) {
	return 0;
    }
    
    for (smiElement = smiGetFirstElement(row);
	 smiElement;
	 smiElement = smiGetNextElement(smiElement)) {
	SmiNode *indexNode = smiGetElementNode(smiElement);
	if (cmpSmiNodes(indexNode, node)) {
	    return 1;
	}
    }

    return 0;
}

/* -------------- main functions ------------------------------------------- */




/*
 * algCheckForExpandRel
 *
 * Checks the given table for an expand relationship to an other table.
 *
 * 1. gets the expanded table (father node of smiNode)
 * 2. gets the base table by walking through the elements of the smiNode
 *    starting at the end. The first table found which is different from the
 *    expanded table is the first candidate for the base table.
 * 3. comparing the number of elements in both tables
 *    if the number in the expanded table is greater -> 5.
 *    if the number in the base table is greater     -> 4.
 * 4. getting a second base table candidate :
 *    It is possible that a table expands an other table which is expanded
 *    by this.
 *    Therefore it is hard to track the base table.
 *    - scanning all tables which are different from the expanded table
 *    - the base table must have :
 *         - least elements
 *         - the same elementes must occur as in the expanded table
 * 5. the elements in both tables are checked for equality 
 */
static void algCheckForExpandRel(SmiNode *smiNode) 
{
    SmiNode      *tNode;
    SmiNode      *baseTable;
    SmiNode      *expTable;
    SmiElement   *smiElement;
    SmiElement   *findElement;
    unsigned int refcounter;
    unsigned int basecounter;

    if (!smiNode) return;

    expTable = smiGetParentNode(smiNode);  

    /* count the elements in the given table <- father of smiNode */
    refcounter = algCountIndexElements(smiNode);

    /* find the base table <- via the last element which does not refer to 
       the expTable */
    baseTable = NULL;
    for (smiElement = smiGetFirstElement(smiNode); 
	 smiElement; 
	 smiElement = smiGetNextElement(smiElement)) {  
	tNode = smiGetElementNode(smiElement);
	tNode = smiGetParentNode(tNode);
	tNode = smiGetParentNode(tNode);

	if (cmpSmiNodes(tNode, expTable) == 1) break;
	
	baseTable = tNode;
    }  

    if (!baseTable) return;

    /* count the elements in the basetable */
    basecounter = algCountIndexElements(smiGetFirstChildNode(baseTable));

    /* are baseTable and expTable identical ? */
    if (basecounter >= refcounter) {

	/* searching for new base table candidate in order to handle multiple
	   indices */
	for (baseTable = smiGetNextNode(baseTable, SMI_NODEKIND_TABLE);
	     baseTable;
	     baseTable = smiGetNextNode(baseTable, SMI_NODEKIND_TABLE)) {
      
	    basecounter = algCountIndexElements(smiGetFirstChildNode(baseTable));
      
	    if (basecounter < refcounter) {

		for (smiElement = smiGetFirstElement(
		    smiGetFirstChildNode(expTable)); 
		     smiElement; 
		     smiElement = smiGetNextElement(smiElement)) {
		    tNode = smiGetElementNode(smiElement);

		    /*if (smiGetParentNode(smiGetParentNode(tNode))->name == 
		      expTable->name) break; */
		    if (cmpSmiNodes(smiGetParentNode(smiGetParentNode(tNode)),
				    expTable) == 1) break;
	  
		    for (findElement = smiGetFirstElement(
			smiGetFirstChildNode(baseTable)); 
			 findElement; 
			 findElement = smiGetNextElement(findElement)) {
			if (cmpSmiNodes(tNode, smiGetElementNode(findElement))
			    == 1) break;
		    }
	  
		    if (!findElement) {
			return;
		    }
		}	
		break;
	    }
	}
    
	if (!baseTable) {
	    return;
	}
    }

    for (smiElement = smiGetFirstElement(smiGetFirstChildNode(baseTable)); 
	 smiElement; 
	 smiElement = smiGetNextElement(smiElement)) {
	tNode = smiGetElementNode(smiElement);

	for (findElement = smiGetFirstElement(smiGetFirstChildNode(expTable)); 
	     findElement; 
	     findElement = smiGetNextElement(findElement)) {
	    if (cmpSmiNodes(tNode, smiGetElementNode(findElement)) == 1)
		break;
	}
    
	if (!findElement) {
	    return;
	}
    }

    algInsertEdge(baseTable, expTable, SMI_INDEX_EXPAND,
		  GRAPH_ENHINDEX_INDEX);  
}

/*
 * algCheckForSparseRel
 *
 * Checks the given table for a sparse relationship to an other table. 
 *
 * Criterias for a sparse relationship :
 *    - same number of index elements in both tables
 *    - the same order of this elements
 * 1. Getting the basetable via the last element in the index of smiNode.
 * 2. comparing the number of elements
 */
static void algCheckForSparseRel(SmiNode *smiNode) 
{
    SmiNode      *tNode = NULL;
    SmiNode      *table;
    SmiElement   *smiElement;
    unsigned int basecounter;

    if (!smiNode) return;

    table = smiGetParentNode(smiNode);

    basecounter = algCountIndexElements(smiNode);

    /* getting the basetable via the father node of the last element
       to handle multiple instanceing */
    for (smiElement = smiGetFirstElement(smiNode);
	 smiElement; 
	 smiElement = smiGetNextElement(smiElement)) {
	tNode = smiGetElementNode(smiElement);    
    }

    if (! tNode) {
	return;
    }
    
    tNode = smiGetParentNode(tNode);
    if (equalElements(smiNode, tNode) == 0) {
	algInsertEdge(smiGetParentNode(tNode), smiGetParentNode(smiNode), 
		      SMI_INDEX_SPARSE, GRAPH_ENHINDEX_INDEX);
    }
}

/*
 * algCheckForReOrderRel
 *
 * Checks the given table for a reorder relationship to an other table.
 *
 * Criterias for reoder relationships :
 *    - same number of elements
 *    - same elements must occur in a different order
 */
static void algCheckForReorderRel(SmiNode *smiNode)
{
    SmiNode   *tNode;
    GraphNode *graphNode;
    
    if (!smiNode) {
	return;
    }

    for (graphNode = graph->nodes;
	 graphNode;
	 graphNode = graphNode->nextPtr) {	
	tNode = graphNode->smiNode;
	if (tNode->nodekind == SMI_NODEKIND_TABLE) {
	    if (equalElements(smiNode, smiGetFirstChildNode(tNode)) == 1) {
		algInsertEdge(smiGetParentNode(smiNode), tNode,
			      SMI_INDEX_REORDER, GRAPH_ENHINDEX_INDEX);
		break;
	    }
	}
    }
}

/*
 * algGetIndexkind
 *
 * Gets the indexkind of the given table. The row object of the table is
 * passed to this function.
 * Therefore three subfunctions are called to get the indexkind. 
 *   - algChechForExpandRel
 *   - algCheckForSparseRel
 *   - algCheckForReOrderRel
 * Look there for further information.
 */
static void algGetIndexkind(SmiNode *row) 
{
    algCheckForExpandRel(row);
    algCheckForSparseRel(row);
    algCheckForReorderRel(row);
}

/*
 * algLinkTables
 *
 * Links the tables of the given module.
 *
 *  1. Getting the tables and the scalars from the given module.
 *  2. Linking the tables :
 *     - AUGMENT no work to do just adding the corresponding edge
 *     - for other relationships the subfunction algGetIndexkind is called
 *       look there for further information  
 */
static void algLinkTables() 
{
    GraphNode  *tGraphNode;
    SmiNode    *node; 
    SmiNode    *tSmiNode;

    /* linking the tables */
    for (tGraphNode = graph->nodes;
	 tGraphNode;
	 tGraphNode = tGraphNode->nextPtr) {
	node = tGraphNode->smiNode;

	if (node->nodekind == SMI_NODEKIND_TABLE) {
	    node = smiGetFirstChildNode(node);

	    if (node->nodekind == SMI_NODEKIND_ROW) {

		/* get the relationship between the tables and insert
		   the edges */
		if (node->indexkind == SMI_INDEX_INDEX) {
		    algGetIndexkind(node);
		} else {
		    tSmiNode = node;
		    node = smiGetRelatedNode(node);
		    node = smiGetParentNode(node);
		    algInsertEdge(node,
				  smiGetParentNode(tSmiNode), 
				  tSmiNode->indexkind,
				  GRAPH_ENHINDEX_INDEX);	    
		}
	    }
	}
    }

   if (XPLAIN) {
       printf("--- Second Phase - linking the tables\n\n");
       graphShowEdges(graph);
   } 
}

/*
 * algCheckLinksByName
 *
 * Reordering the connections by looking at the names.
 * Related objects are linked (if their names are related).
 * Every expand relationship is examined. Therefore number of
 * identical characters at the beginning of the both table names is
 * counted.
 * Then every sparse relationship (only the ending node) is checked if 
 * there is better connection with more identical characters at the beginning.
 * The starting node must be same as in the expand relationship to make sure
 * that there are no unrelated tables are linked.
 * Only legal overlappings lead to a new edge. A illegal overlap is :
 * - when the next character in both strings is a lower case character
 *   because if something like this is detected the two strings are
 *   corresponding in the middle of their names and not at defined points
 *   like suffix or prefix (-> the next character must be upper case or NULL)
 */
static void algCheckLinksByName() 
{
    GraphEdge *tEdge, *tEdge2, *newEdge;
    char      *start, *end, *end2;
    int       overlap, longestOverlap, i;

    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {

	if (tEdge->indexkind == SMI_INDEX_EXPAND) {

	    start = tEdge->startNode->smiNode->name;
	    end = tEdge->endNode->smiNode->name;

	    overlap = strpfxlen(start,end);
      
	    /*
	     * looking for better connection with longer overlap
	     * maybe better to traverse the edges with graphGetNextEdgeByNode
	     * using tEdge->startNode
	     */  
	    newEdge = NULL;
	    longestOverlap = overlap;
	    for (tEdge2 = graphGetFirstEdgeByNode(graph,tEdge->startNode);
		 tEdge2;
		 tEdge2 = graphGetNextEdgeByNode(graph, tEdge2,
						 tEdge->startNode)) {
	
		/*
		 * must be a sparse relationship to get a correct new edge
		 */
		if (tEdge2->indexkind == SMI_INDEX_SPARSE) {
		    end2 = tEdge2->endNode->smiNode->name;
	  
		    /*
		     * new overlap longer and different tables ? 
		     */
		    i = strpfxlen(end,end2);
		    if (overlap < i &&
			!cmpSmiNodes(tEdge->endNode->smiNode,
				     tEdge2->endNode->smiNode)) {
			/*
			 * legal overlap ?
			 */
			if (islower((int)end[i]) && islower((int)end2[i])) {
			    break;
			}

			longestOverlap=i;
			newEdge = tEdge2;
		    }
		}
	    }
      
	    /* better connection found -> changing the edge */
	    if (newEdge) {
		tEdge->startNode = newEdge->endNode;
	    }
	}
    }

    if (XPLAIN) {
	printf("\n--- Third Phase - reordering the connections\n\n");
	graphShowEdges(graph);
    }
}

/*
 * algLinkObjectsByNames
 *
 * Links Scalars to Tables using the prefix
 * Links Tables to Tables using the prefix
 */
static void algLinkObjectsByNames()
{
    GraphNode *tNode, *tNode2;
    int       overlap,minoverlap,new;

    /* getting the minimum overlap for all nodes */
    minoverlap = 10000;
    tNode2 = graph->nodes;
    for (tNode = tNode2->nextPtr; tNode; tNode = tNode->nextPtr) {	
	minoverlap = min(minoverlap, strpfxlen(tNode->smiNode->name,
					       tNode2->smiNode->name));
    }

    /*
     * prefix overlap of one is too short to create any usefull edges
     */
    if (minoverlap == 1) return;
    
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {    
	if (!graphGetFirstEdgeByNode(graph, tNode)) {
	    overlap = minoverlap;

	    for (tNode2 = graph->nodes; tNode2; tNode2 = tNode2->nextPtr) {
		if (cmpSmiNodes(tNode->smiNode, tNode2->smiNode)) continue;
		
		new = strpfxlen(tNode->smiNode->name, tNode2->smiNode->name);
		
		if (new >= overlap) {

		    /*
		     * no scalar - scalar edges
		     */
		    if (tNode->smiNode->nodekind == SMI_NODEKIND_SCALAR &&
			tNode2->smiNode->nodekind == SMI_NODEKIND_SCALAR) {
			continue;
		    }

		    /*
		     * is it a legal prefix
		     * (if the next char is NULL || uppercase)
		     */
		    if (tNode->smiNode->name[new] &&
			tNode2->smiNode->name[new]) {
			if (!isupper((int)tNode->smiNode->name[new]) ||
			    !isupper((int)tNode2->smiNode->name[new])) continue;
		    }
		    
		    overlap = new;
		}
	    }

	    if (overlap == minoverlap) continue;
	    
	    new = 0;
	    for (tNode2 = graph->nodes; tNode2; tNode2 = tNode2->nextPtr) {
		if (cmpSmiNodes(tNode->smiNode, tNode2->smiNode)) continue;

		new = strpfxlen(tNode->smiNode->name, tNode2->smiNode->name);
		
		if (new == overlap && new > minoverlap) {

		    /*
		     * a scalar should only be adjacent to one node
		     */
		    if (tNode2->smiNode->nodekind == SMI_NODEKIND_SCALAR &&
			graphGetFirstEdgeByNode(graph,tNode2)) continue;
		    if (tNode->smiNode->nodekind == SMI_NODEKIND_SCALAR &&
			graphGetFirstEdgeByNode(graph,tNode)) continue;    

		    /*
		     * adding only table -> scalar edges
		     */
		    if (tNode->smiNode->nodekind == SMI_NODEKIND_SCALAR &&
			tNode2->smiNode->nodekind == SMI_NODEKIND_SCALAR) {
			continue;
		    }
		    
		    if (tNode->smiNode->nodekind == SMI_NODEKIND_SCALAR) {
			algInsertEdge(tNode2->smiNode, tNode->smiNode,
				      SMI_INDEX_UNKNOWN,
				      GRAPH_ENHINDEX_NAMES);
		    } else {
			algInsertEdge(tNode->smiNode, tNode2->smiNode,
				      SMI_INDEX_UNKNOWN,
				      GRAPH_ENHINDEX_NAMES);
		    }
		}
	    }
	}
    }
}


/*
 * algGroupScalars
 *
 * Grouping the scalars using a common fathernode.
 */
static void algGroupScalars()
{
    GraphNode *tNode;
    int       actGroup, fGroup;
    
    actGroup = 0;
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {	
	if (!graphGetFirstEdgeByNode(graph, tNode) &&
	    tNode->smiNode->nodekind == SMI_NODEKIND_SCALAR) {
	    fGroup = algGetGroupByFatherNode(tNode->smiNode);
	    if (fGroup == 0) {
		tNode->group = ++actGroup;
	    }
	    else {
		tNode->group = fGroup; 
	    }
	}
    }

    if (XPLAIN) {
	printf("Scalar Groups : \n");

	if (algGetNumberOfGroups() != 0) {
	    for (actGroup = 1;
		 actGroup <= algGetNumberOfGroups();
		 actGroup++) {
		algPrintGroup(actGroup);
		printf("\n");
	    }
	} else printf("No groups!\n");
	printf("\n");
    }
}

/*
 * algLinkLonelyTables
 *
 * Links isolated tables with other tables via the types of the
 * index objects.
 * If no type is found the types are checked via the suffix if they differ
 * only in the range (-OrZero). Therefore the suffix-vector is checked.
 * Basetypes used in SMIv1/v2/ng are sorted out.
 */
static void algLinkLonelyTables()
{
    SmiElement *smiElement, *smiElement2 = NULL;
    GraphNode  *tNode, *tNode2;
    int        i;

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (!graphGetFirstEdgeByNode(graph, tNode) &&
	    tNode->smiNode->nodekind == SMI_NODEKIND_TABLE) {

	    for (smiElement = smiGetFirstElement(
		smiGetFirstChildNode(tNode->smiNode));
		 smiElement;
		 smiElement = smiGetNextElement(smiElement)) {

		for (tNode2 = graph->nodes;
		     tNode2;
		     tNode2 = tNode2->nextPtr) {
		    if (tNode->smiNode->nodekind == SMI_NODEKIND_TABLE &&
			tNode != tNode2) {

			for (smiElement2 = smiGetFirstElement(
			    smiGetFirstChildNode(tNode2->smiNode));
			     smiElement2;
			     smiElement2 = smiGetNextElement(smiElement2)) {

			    if (strcasecmp(algGetTypeName(
				           smiGetElementNode(smiElement2)),
				       algGetTypeName(
					   smiGetElementNode(smiElement)))
				== 0) {

				if (isBaseType(smiGetElementNode(smiElement))
				    == 1){
				    continue;
				}
				
				graphInsertEdge(graph,tNode2, tNode,
						SMI_INDEX_UNKNOWN,
						GRAPH_ENHINDEX_TYPES);
				break;
			    }
			    else {
				for (i = 0; suffix[i]; i++) {
				    if (strstr(
					algGetTypeName(
					    smiGetElementNode(smiElement)),
					suffix[i])) {
					graphInsertEdge(graph,tNode2,
							tNode,
							SMI_INDEX_UNKNOWN,
							GRAPH_ENHINDEX_TYPES);
					break;
				    }
				}
				
				if (suffix[i]) break;
			    }
			}
		    }
		    if (smiElement2) break;
		}
		if (tNode2) break;
	    }
		
	}
    }
}

/*
 * algConnectLonelyNodes
 *
 * Connect the isolated nodes (scalars and tables)
 *
 * 1. Trying to link tables via the type of the index objects
 * 2. Trying to link scalars to tables using the names
 * 3. Trying to group scalars which have not been adjacent to any edge.
 */
static void algConnectLonelyNodes() 
{
    if (XPLAIN) {
	printf("\n--- Fourth Phase -  connecting isolated nodes\n\n");
    }

    algLinkLonelyTables();

    algLinkObjectsByNames();

    algGroupScalars();

    if (XPLAIN) {
	graphShowEdges(graph);
    }
}

/*
 * Subfunction of algCheckForDependencies
 *
 * change UML-edges to dependecy if possible
 */
static void createDependencies()
{
    GraphEdge *tEdge;
    int       elemCount;
    
    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {
	
	if (tEdge->indexkind == SMI_INDEX_EXPAND ||
	    tEdge->indexkind == SMI_INDEX_UNKNOWN) {

	    if (tEdge->startNode->smiNode->nodekind == SMI_NODEKIND_TABLE &&
		tEdge->endNode->smiNode->nodekind   == SMI_NODEKIND_TABLE) {
		
		elemCount = algGetNumberOfRows(tEdge->startNode->smiNode);
		if (elemCount >= 0) tEdge->connection = GRAPH_CON_DEPENDENCY;
		
		elemCount = algGetNumberOfRows(tEdge->endNode->smiNode);
		if (elemCount >= 0) tEdge->connection = GRAPH_CON_DEPENDENCY;
	    }
	}
    }
}

/*
 * returns 1 if a new rerouting edge creates a loop and 0 otherwise
 */
static int isloop(GraphEdge *tEdge, SmiNode *depTable)
{
    GraphEdge *loopEdge;

    for (loopEdge = graphGetFirstEdgeByNode(graph, tEdge->endNode);
	 loopEdge;
	 loopEdge = graphGetNextEdgeByNode(graph,loopEdge,tEdge->endNode)) {

	if ((cmpSmiNodes(loopEdge->startNode->smiNode, depTable)) ||
	    (cmpSmiNodes(loopEdge->endNode->smiNode, depTable) &&
	     (loopEdge != tEdge))) {
	    return 1;
	}
    }

    return 0;
}

/*
 * subfunction of algCheckForDependency
 */
static void rerouteDependencyEdge(GraphEdge *tEdge)
{
    SmiNode    *startTable, *endTable, *depTable;
    SmiElement *smiElement;
    
    startTable = tEdge->startNode->smiNode;
    endTable = tEdge->endNode->smiNode;
    
    for (smiElement = smiGetFirstElement(
	smiGetFirstChildNode(endTable));
	 smiElement;
	 smiElement = smiGetNextElement(smiElement)) {
	
	/* look only at expanded indices (only present in endTable) */
	if (cmpSmiNodes(endTable, smiGetParentNode(
	    smiGetParentNode(smiGetElementNode(smiElement)))) == 1) {
	    depTable = algFindEqualType(startTable,
					smiGetElementNode(smiElement));
	    
	    /* depTable different to endTable? */
	    if (depTable && !cmpSmiNodes(depTable, endTable)) {

		/* prevent loops */
		if (isloop(tEdge, depTable)) continue;
		
		algInsertEdge(depTable,
			      startTable, SMI_INDEX_UNKNOWN,
			      GRAPH_ENHINDEX_REROUTE);
		break;
	    }
	}
    }
}

/*
 * algCheckForDependency
 *
 * Tries to change connection types from association to dependency by
 * looking at the types.
 *
 * 1. Supporting tables are revealed. Supporting tables consist only
 *    index objects and RowStatus-Objects and/or Storage-Type/Objects.
 * 2. If a supporting table is found the connection type is changed to
 *    dependency.
 * 3. Now the types of the index objects are checked (only in the end-table).
 *    If the same type is found in an other table the start-table is going to
 *    be connected with it (inserting a new edge). 
 *    The end-table is still connected to the start-table.
 */
static void algCheckForDependency() 
{
    GraphEdge  *tEdge;

    createDependencies();
    
    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {

	if (tEdge->connection == GRAPH_CON_DEPENDENCY) {

	    rerouteDependencyEdge(tEdge);
	}
    }

    if (XPLAIN) {
	printf("\n--- Fifth Phase - checking for dependency relationships\n\n");
	graphShowEdges(graph);
    }
}

/*
 *
 */
static SmiNode *algFindTable(SmiNode *node)
{
    GraphNode  *tNode;
    SmiNode    *smiNode;
    SmiElement *smiElement;
    char       *toFind;
    
    if (!node) return NULL;

    toFind = xstrdup(node->name
		     + strpfxlen(node->name, smiGetParentNode(node)->name));
		     
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->smiNode->nodekind == SMI_NODEKIND_TABLE) {
	    if (cmpSmiNodes(node, tNode->smiNode)) continue;
	    smiNode = smiGetFirstChildNode(tNode->smiNode);
	    
	    for (smiElement = smiGetFirstElement(smiNode);
		 smiElement;
		 smiElement = smiGetNextElement(smiElement)) {
		smiNode = smiGetElementNode(smiElement);
		if (strstr(smiNode->name, toFind)) {
		    xfree(toFind);
		    return smiGetParentNode(smiGetParentNode(smiNode));
		}
	    }
	}
    }

    xfree(toFind);
    
    return NULL;
}

/*
 * Creates edges based on column-objects pointing to other tables.
 * Column-objects with a substring "Index" are examined.
 */
static void algCheckForPointerRels()
{
    GraphNode *tNode;
    SmiModule *module;
    SmiNode   *smiNode = NULL;
    SmiNode   *ppNode, *table;
    int       i;
    
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->smiNode->nodekind == SMI_NODEKIND_TABLE) {

	    module  = smiGetNodeModule(tNode->smiNode);

	    for (smiNode = smiGetFirstNode(module, SMI_NODEKIND_COLUMN);
		 smiNode;
		 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COLUMN)) {
		ppNode = smiGetParentNode(smiNode);
		ppNode = smiGetParentNode(ppNode);
	
		if (!algIsIndexElement(tNode->smiNode, smiNode) &&
		    cmpSmiNodes(tNode->smiNode, ppNode)) {

		    for (i = 0; pointer[i]; i++) {
			if (strstr(smiNode->name, pointer[i])) {
			    /* printf("%s \n",smiNode->name); */
			    table = algFindTable(smiNode);
			    if (table) {
				algInsertEdge(table, tNode->smiNode,
					      SMI_INDEX_UNKNOWN,
					      GRAPH_ENHINDEX_POINTER);
			    }
			}
		    }
		}
	    }
	}
    }

    if (XPLAIN) {
	printf("\n--- Sixth Phase - checking for pointer relationships\n\n");
	graphShowEdges(graph);	
    }
}

/*
 * Creates the graph nodes of the given module
 */
static void algCreateNodes(SmiModule *module)
{
    SmiNode *node;
    
    /* get tables and scalars from the MIB module */
    for (node = smiGetFirstNode(module, SMI_NODEKIND_TABLE);
	 node;
	 node = smiGetNextNode(node, SMI_NODEKIND_TABLE)) {
	if (node->status != SMI_STATUS_OBSOLETE) {
	    if (SUPPRESS_DEPRECATED && node->status != SMI_STATUS_DEPRECATED)
		graphInsertNode(graph, node);
	}
    }
    for (node = smiGetFirstNode(module, SMI_NODEKIND_SCALAR);
	 node;
	 node = smiGetNextNode(node, SMI_NODEKIND_SCALAR)) {
	if (node->status != SMI_STATUS_OBSOLETE) {
	    if (SUPPRESS_DEPRECATED && node->status != SMI_STATUS_DEPRECATED)
		graphInsertNode(graph, node);
	}
    }
}



/* ------ XML primitives ------                                              */





static void diaPrintXMLHeader()
{
    printf("<?xml version=\"1.0\"?>\n");
    printf("<diagram xmlns:dia=\"http://www.lysator.liu.se/~alla/dia/\">\n");
    printf("  <diagramdata>\n");
    printf("    <attribute name=\"background\">\n");
    printf("      <color val=\"#ffffff\"/>\n");
    printf("    </attribute>\n");
    printf("    <attribute name=\"paper\">\n");
    printf("      <composite type=\"paper\">\n");
    printf("        <attribute name=\"name\">\n");
    printf("          <string>#A4#</string>\n");
    printf("        </attribute>\n");
    printf("        <attribute name=\"tmargin\">\n");
    printf("         <real val=\"2.82\"/>\n");
    printf("       </attribute>\n");
    printf("       <attribute name=\"bmargin\">\n");
    printf("         <real val=\"2.82\"/>\n");
    printf("        </attribute>\n");
    printf("       <attribute name=\"lmargin\">\n");
    printf("         <real val=\"2.82\"/>\n");
    printf("       </attribute>\n");
    printf("       <attribute name=\"rmargin\">\n");
    printf("         <real val=\"2.82\"/>\n");
    printf("       </attribute>\n");
    printf("       <attribute name=\"is_portrait\">\n");
    printf("         <boolean val=\"true\"/>\n");
    printf("       </attribute>\n");
    printf("      <attribute name=\"scaling\">\n");
    printf("         <real val=\"1\"/>\n");
    printf("      </attribute>\n");
    printf("      <attribute name=\"fitto\">\n");
    printf("        <boolean val=\"false\"/>\n");
    printf("      </attribute>\n");
    printf("    </composite>\n");
    printf("   </attribute>\n");
    printf("  </diagramdata>\n");
    printf("  <layer name=\"Background\" visible=\"true\">\n");   
}

static void diaPrintXMLClose()
{
    printf("  </layer>\n");
    printf("</diagram>\n");
}

/*
 * prints the type of a given node
 */  
static void diaPrintXMLType(SmiNode *smiNode, int index)
{
    printf("          <attribute name=\"type\">\n");
    if (index) {
	printf("            <string>#%s%s#</string>\n",
	       algGetTypeName(smiNode), INDEXPROPERTY);
    } else {
	printf("            <string>#%s#</string>\n", algGetTypeName(smiNode));
    }
    printf("          </attribute>\n");    
}

/*
 * index = 0 -> no index element
 * index = 1 -> index element -> printed with "+"
 */
static void diaPrintXMLAttribute(SmiNode *node, int index)
{
    printf("        <composite type=\"umlattribute\">\n");
    printf("          <attribute name=\"name\">\n");
    printf("            <string>#%s#</string>\n", node->name);
    printf("          </attribute>\n");
    
    diaPrintXMLType(node,index);
    
    printf("          <attribute name=\"value\">\n");
    printf("            <string/>\n");
    printf("          </attribute>\n");
    printf("          <attribute name=\"visibility\">\n");

    if (node->access == SMI_ACCESS_NOT_ACCESSIBLE) {
	printf("            <enum val=\"1\"/>\n");
    } else {
	printf("            <enum val=\"0\"/>\n");
    }
    
    printf("          </attribute>\n");
    printf("          <attribute name=\"abstract\">\n");
    printf("            <boolean val=\"false\"/>\n");
    printf("          </attribute>\n");
    printf("          <attribute name=\"class_scope\">\n");
    
    if (node->nodekind == SMI_NODEKIND_SCALAR) {
	printf("            <boolean val=\"true\"/>\n");
    } else {
	printf("            <boolean val=\"false\"/>\n");	
    }

    printf("          </attribute>\n");
    printf("        </composite>\n");	   
}

/*
 * prints the related scalars for a given table
 */
static void diaPrintXMLRelatedScalars(GraphNode *node)
{
    GraphEdge *tEdge;
    
    for (tEdge = graphGetFirstEdgeByNode(graph, node);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, node)) {
	if (tEdge->startNode == node  &&
	    tEdge->endNode->smiNode->nodekind == SMI_NODEKIND_SCALAR) {
	    tEdge->dia.flags |= DIA_PRINT_FLAG;
	    tEdge->endNode->dia.flags |= DIA_PRINT_FLAG;

	    diaPrintXMLAttribute(tEdge->endNode->smiNode,0);
	}
    }
}

/*
 * prints all columns objects of the given node
 */
static void diaPrintXMLAllColumns(GraphNode *node)
{
    SmiModule *module  = NULL;
    SmiNode   *smiNode = NULL;
    SmiNode   *ppNode;

    module  = smiGetNodeModule(node->smiNode);

    for (smiNode = smiGetFirstNode(module, SMI_NODEKIND_COLUMN);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_COLUMN)) {
	ppNode = smiGetParentNode(smiNode);
	ppNode = smiGetParentNode(ppNode);
	
	if (!algIsIndexElement(node->smiNode, smiNode) &&
	    cmpSmiNodes(node->smiNode, ppNode))
	    diaPrintXMLAttribute(smiNode, 0);
    }
}

/*
 * adds the index to an augmenting table (row-element)
 */
static void diaPrintAugmentIndex(GraphNode *tNode)
{
    GraphEdge  *tEdge;
    SmiElement *smiElement;

    for (tEdge = graphGetFirstEdgeByNode(graph, tNode);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, tNode)) {
	if (tEdge->indexkind == SMI_INDEX_AUGMENT) {
	    for (smiElement = smiGetFirstElement(
		smiGetFirstChildNode(tEdge->startNode->smiNode));
		 smiElement;
		 smiElement = smiGetNextElement(smiElement)) {
		if (!cmpSmiNodes(tNode->smiNode, tEdge->startNode->smiNode)) {
		    diaPrintXMLAttribute(smiGetElementNode(smiElement),1);
		}
	    }
	}
    }
}

static void diaPrintXMLObject(GraphNode *node, float x, float y)
{
    SmiElement *smiElement;
    
    if (!node) return;
    if (node->dia.flags & DIA_PRINT_FLAG) return;

    node->dia.x = x;
    node->dia.y = y;
    node->dia.flags |= DIA_PRINT_FLAG; /* object is now printed */
    
    printf("    <object type=\"UML - Class\" version=\"0\" id=\"%s\">\n",
	   node->smiNode->name);
    printf("      <attribute name=\"obj_pos\">\n");
    printf("       <point val=\"%.2f,%.2f\"/>\n",x,y);
    printf("      </attribute>\n");
    printf("     <attribute name=\"obj_bb\">\n");
    printf("       <rectangle val=\"0.0,0.0;0.0,0.0\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_corner\">\n");
    printf("       <point val=\"%.2f,%.2f\"/>\n",x,y);
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_width\">\n");
    printf("       <real val=\"%.2f\"/>\n",node->dia.w);
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_height\">\n");
    printf("       <real val=\"%.2f\"/>\n",node->dia.h);
    printf("     </attribute>\n");
    printf("     <attribute name=\"name\">\n");
    printf("       <string>#%s#</string>\n",
	   smiGetFirstChildNode(node->smiNode)->name);
    printf("     </attribute>\n");
    printf("     <attribute name=\"stereotype\">\n");
    printf("        <string>#%s#</string>\n", STEREOTYPE);
    printf("     </attribute>\n");
    printf("     <attribute name=\"abstract\">\n");
    printf("       <boolean val=\"false\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"suppress_attributes\">\n");
    printf("        <boolean val=\"false\"/>\n");    
    printf("      </attribute>\n");   
    printf("      <attribute name=\"suppress_operations\">\n");
    printf("        <boolean val=\"true\"/>\n");
    printf("      </attribute>\n");
    printf("     <attribute name=\"visible_attributes\">\n");
    printf("       <boolean val=\"true\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"visible_operations\">\n");
    printf("        <boolean val=\"false\"/>\n");
    printf("      </attribute>\n");

    printf("     <attribute name=\"attributes\">\n");

    if (node->smiNode->nodekind == SMI_NODEKIND_TABLE) {

	diaPrintXMLRelatedScalars(node);

	diaPrintAugmentIndex(node);
	
	for (smiElement = smiGetFirstElement(
	    smiGetFirstChildNode(node->smiNode));
	     smiElement;
	     smiElement = smiGetNextElement(smiElement)) {
	    diaPrintXMLAttribute(smiGetElementNode(smiElement),1);
	}

	if (PRINT_DETAILED_ATTR) {
	    diaPrintXMLAllColumns(node);
	}
    }
    
    printf("      </attribute>\n");
    
    printf("     <attribute name=\"operations\"/>\n");
    printf("    <attribute name=\"template\">\n");
    printf("      <boolean val=\"false\"/>\n");
    printf("    </attribute>\n");
    printf("     <attribute name=\"templates\"/>\n");
    printf("   </object>\n");
}

/*
 * prints a group of scalars denoted by group
 */
static void diaPrintXMLGroup(int group, float x, float y)
{
    GraphNode *tNode;

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->group == group) break;
    }

    if (!tNode) return;
    
    printf("    <object type=\"UML - Class\" version=\"0\" id=\"%s\">\n",
	   smiGetParentNode(tNode->smiNode)->name);
    printf("      <attribute name=\"obj_pos\">\n");
    printf("       <point val=\"%.2f,%.2f\"/>\n",x,y);
    printf("      </attribute>\n");
    printf("     <attribute name=\"obj_bb\">\n");
    printf("       <rectangle val=\"0.0,0.0;0.0,0.0\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_corner\">\n");
    printf("       <point val=\"%.2f,%.2f\"/>\n",x,y);
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_width\">\n");
    printf("       <real val=\"%.2f\"/>\n",0.0);
    printf("     </attribute>\n");
    printf("     <attribute name=\"elem_height\">\n");
    printf("       <real val=\"%.2f\"/>\n",0.0);
    printf("     </attribute>\n");
    printf("     <attribute name=\"name\">\n");
    printf("       <string>#%s#</string>\n",
	   smiGetParentNode(tNode->smiNode)->name);
    printf("     </attribute>\n");
    printf("     <attribute name=\"stereotype\">\n");
    printf("         <string>#%s#</string>\n", STEREOTYPE);
    printf("     </attribute>\n");
    printf("     <attribute name=\"abstract\">\n");
    printf("       <boolean val=\"false\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"suppress_attributes\">\n");
    printf("        <boolean val=\"false\"/>\n");    
    printf("      </attribute>\n");   
    printf("      <attribute name=\"suppress_operations\">\n");
    printf("        <boolean val=\"true\"/>\n");
    printf("      </attribute>\n");
    printf("     <attribute name=\"visible_attributes\">\n");
    printf("       <boolean val=\"true\"/>\n");
    printf("     </attribute>\n");
    printf("     <attribute name=\"visible_operations\">\n");
    printf("        <boolean val=\"false\"/>\n");
    printf("      </attribute>\n");

    printf("     <attribute name=\"attributes\">\n");

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (tNode->group == group) {
	    diaPrintXMLAttribute(tNode->smiNode,0);
	}
    }
    
    printf("      </attribute>\n");
    
    printf("     <attribute name=\"operations\"/>\n");
    printf("    <attribute name=\"template\">\n");
    printf("      <boolean val=\"false\"/>\n");
    printf("    </attribute>\n");
    printf("     <attribute name=\"templates\"/>\n");
    printf("   </object>\n");
}

static float getRectSX(GraphNode *tNode)
{
    return (float) tNode->dia.w / 2 + tNode->dia.x - RECTCORRECTION;
}

static float getRectEX(GraphNode *tNode)
{
    return (float) tNode->dia.w / 2 + tNode->dia.x + RECTCORRECTION;
}

static float getRectSY(GraphNode *tNode)
{
    return (float) tNode->dia.y - 2 - RECTCORRECTION;
}

static float getRectEY(GraphNode *tNode)
{
    return (float) tNode->dia.y - 2 + RECTCORRECTION;
}


static int getConPoint(GraphNode *snode, GraphNode *enode)
{
    float x1,y1,x2,y2;
    int   con = 1;
    
    x1 = snode->dia.x;
    y1 = snode->dia.y;
    x2 = enode->dia.x;
    y2 = enode->dia.y;    

    if (x1 == x2 && y1 < y2) con = 6;
    if (x1 == x2 && y1 > y1) con = 1;
    if (x1 > x2 && y1 == y2) con = 3;
    if (x1 < x2 && y1 == y2) con = 4;
    if (x1 > x2 && y1 > y2)  con = 0;
    if (x1 > x2 && y1 < y2)  con = 5;
    if (x1 < x2 && y1 > y2)  con = 2;
    if (x1 < x2 && y1 < y2)  con = 7;

    return con;
}

static float getObjX(GraphNode *node, int con)
{
    switch (con) {
    case 0 :
	return node->dia.x;
	break;
    case 1 :
	return node->dia.w / 2 + node->dia.x;
	break;
    case 2 :
	return node->dia.x + node->dia.w;
	break;
    case 3 :
	return node->dia.x;
	break;	
    case 4 :
	return node->dia.x + node->dia.w;
	break;
    case 5 :
	return node->dia.x;
	break;	
    case 6 :
	return node->dia.w / 2 + node->dia.x;
	break;
    case 7 :
	return node->dia.x + node->dia.w;
	break;
    }
    return 0;
}

static float getObjY(GraphNode *node, int con)
{
    switch (con) {
    case 0 :
	return node->dia.y;
	break;
    case 1 :
	return node->dia.y;
	break;
    case 2 :
	return node->dia.y;
	break;
    case 3 :
	return node->dia.y + TABLEHEIGHT / 2;
	break;	
    case 4 :
	return node->dia.y + TABLEHEIGHT / 2;
	break;
    case 5 :
	return node->dia.y + node->dia.h;
	break;	
    case 6 :
	return node->dia.y + node->dia.h;
	break;
    case 7 :
	return node->dia.y + node->dia.h;
	break;
    }
    return 0;
}

static float getObjYRel(GraphEdge *edge, int con)
{
    GraphNode *node, *node2;
    float     dist;
    
    node = edge->startNode;
    node2 = edge->endNode;
    if (node->dia.y < node2->dia.y) {
	dist = ABS(((node->dia.y + node->dia.h) - node2->dia.y ) / 2);
    } else {
	dist = ABS((node->dia.y - (node2->dia.y + node2->dia.h)) / 2);
    }
    
    switch (con) {
    case 0 :
	return node->dia.y - dist;
	break;
    case 1 :
	return node->dia.y - dist;
	break;
    case 2 :
	return node->dia.y - dist;
	break;
    case 3 :
	return node->dia.y + TABLEHEIGHT / 2;
	break;	
    case 4 :
	return node->dia.y + TABLEHEIGHT / 2;
	break;
    case 5 :
	return node->dia.y + node->dia.h + dist;
	break;	
    case 6 :
	return node->dia.y + node->dia.h + dist;
	break;
    case 7 :
	return node->dia.y + node->dia.h + dist;
	break;
    }
    return 0;
}

/*
 * diaPrintXMLCoordinates
 *
 * prints and calculates the coordinates of a given edge
 */
static void diaPrintXMLCoordinates(GraphEdge *tEdge)
{
    int scon, econ;

    scon = getConPoint(tEdge->startNode, tEdge->endNode);
    econ = getConPoint(tEdge->endNode, tEdge->startNode);
    
    printf("      <attribute name=\"obj_pos\">\n");
    printf("        <point val=\"%.2f,%.2f\"/>\n"
	   ,getObjX(tEdge->startNode,scon)
	   ,getObjY(tEdge->startNode,scon));	   
    printf("     </attribute>\n");
    printf("      <attribute name=\"obj_bb\">\n");
    printf("       <rectangle val=\"%.2f,%.2f;%.2f,%.2f\"/>\n"
	   ,getRectSX(tEdge->startNode)
	   ,getRectSY(tEdge->startNode)
	   ,getRectEX(tEdge->startNode)
	   ,getRectEY(tEdge->startNode));
    printf("     </attribute>\n");
    printf("     <attribute name=\"orth_points\">\n");
    printf("       <point val=\"%.2f,%.2f\"/>\n"
	   ,getObjX(tEdge->startNode,scon)
	   ,getObjY(tEdge->startNode,scon));	
    printf("       <point val=\"%.2f,%.2f\"/>\n"
	   ,getObjX(tEdge->startNode,scon)
	   ,getObjYRel(tEdge,scon));
    printf("       <point val=\"%.2f,%.2f\"/>\n"
	   ,getObjX(tEdge->endNode,econ)
	   ,getObjYRel(tEdge,scon));
    printf("       <point val=\"%.2f,%.2f\"/>\n"
	   ,getObjX(tEdge->endNode,econ)
	   ,getObjY(tEdge->endNode,econ));	
    printf("     </attribute>\n");
}

/*
 * diaPrintXMLConPoints
 *
 * prints the connection points of an edge
 */
static void diaPrintXMLConPoints(GraphEdge *tEdge)
{
    int scon, econ;

    scon = getConPoint(tEdge->startNode, tEdge->endNode);
    econ = getConPoint(tEdge->endNode, tEdge->startNode);
    
    printf("    <connections>\n");
    printf("      <connection handle=\"0\" to=\"%s\" connection=\"%d\"/>\n",
	   tEdge->startNode->smiNode->name,scon);
    printf("      <connection handle=\"1\" to=\"%s\" connection=\"%d\"/>\n",
	   tEdge->endNode->smiNode->name, econ);
    printf("    </connections>\n");    
}

static void diaPrintXMLDependency(GraphEdge *tEdge)
{
    if (tEdge->dia.flags & DIA_PRINT_FLAG) return;
    tEdge->dia.flags |= DIA_PRINT_FLAG;

    printf("    <object type=\"UML - Dependency\" "
	   "version=\"0\" id=\"Depend:%s:%s\">\n",
	   tEdge->startNode->smiNode->name,
	   tEdge->endNode->smiNode->name);    

    diaPrintXMLCoordinates(tEdge);
    
    printf("     <attribute name=\"orth_orient\">\n");
    printf("       <enum val=\"1\"/>\n");
    printf("       <enum val=\"0\"/>\n");
    printf("       <enum val=\"1\"/>\n");
    printf("     </attribute>\n");
    printf("    <attribute name=\"draw_arrow\">\n");
    printf("       <boolean val=\"true\"/>\n");
    printf("      </attribute>\n");
    printf("     <attribute name=\"name\">\n");
    printf("        <string/>\n");
    printf("   </attribute>\n");
    printf("     <attribute name=\"stereotype\">\n");
    printf("      <string/>\n");
    printf("    </attribute>\n");

    diaPrintXMLConPoints(tEdge);
    
    printf("    </object>\n");
}

/*
 * Aggregation is a special case of the association.
 * If aggregate = 1 it is an aggregation if 0 it is an association.
 */
static void diaPrintXMLAssociation(GraphEdge *tEdge, int aggregate)
{
    if (tEdge->dia.flags & DIA_PRINT_FLAG) return;
    tEdge->dia.flags |= DIA_PRINT_FLAG;
    if (aggregate > 1) aggregate = 1;
    if (aggregate < 0) aggregate = 0;
    
    printf("    <object type=\"UML - Association\" "
	   "version=\"0\" id=\"Assoc:%s:%s\">\n",
	   tEdge->startNode->smiNode->name,
	   tEdge->endNode->smiNode->name);
    
    diaPrintXMLCoordinates(tEdge);
    
    printf("      <attribute name=\"orth_orient\">\n");
    printf("        <enum val=\"1\"/>\n");
    printf("        <enum val=\"0\"/>\n");
    printf("        <enum val=\"1\"/>\n");   
    printf("      </attribute>\n");
    printf("      <attribute name=\"name\">\n");

    switch(tEdge->indexkind) {
    case SMI_INDEX_UNKNOWN :
	switch (tEdge->enhancedindex) {
	case GRAPH_ENHINDEX_UNKNOWN :
	    break;
	case GRAPH_ENHINDEX_TYPES :
	    printf("       <string>#%s#</string>\n","");
	    break;
	case GRAPH_ENHINDEX_NAMES :
	    printf("       <string>#%s#</string>\n","");
	    break;
	case GRAPH_ENHINDEX_NOTIFICATION :
	    printf("       <string>#%s#</string>\n","");
	    break;
	case GRAPH_ENHINDEX_INDEX :
	    /* should not occur - is handled below */
	    printf("       <string>#%s#</string>\n","");
	    break;
	case GRAPH_ENHINDEX_REROUTE :
	    printf("       <string>#%s#</string>\n","");
	    break;
	case GRAPH_ENHINDEX_POINTER :
	    printf("       <string>#%s#</string>\n","");
	    break;	    
	}
	break;
    case SMI_INDEX_INDEX :
	printf("       <string>#%s#</string>\n","");
	break;
    case SMI_INDEX_AUGMENT :
	printf("       <string>#%s#</string>\n","augments");
	break;
    case SMI_INDEX_SPARSE :
	printf("       <string>#%s#</string>\n","sparses");
	break;
    case SMI_INDEX_REORDER :
	printf("       <string>#%s#</string>\n","reorders");
	break;
    case SMI_INDEX_EXPAND :
	printf("       <string>#%s#</string>\n","expands");
	break;
    }
    
    printf("      </attribute>\n");
    printf("      <attribute name=\"direction\">\n");
    printf("        <enum val=\"0\"/>\n");
    printf("      </attribute>\n");
    printf("      <attribute name=\"ends\">\n");
    printf("        <composite>\n");
    printf("          <attribute name=\"role\">\n");
    printf("            <string/>\n");
    printf("          </attribute>\n");
    printf("          <attribute name=\"multiplicity\">\n");
    
    switch (tEdge->cardinality) {
    case GRAPH_CARD_UNKNOWN :
	printf("       <string># #</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_ONE :
	printf("       <string>#1#</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_MANY :
	printf("       <string>#1#</string>\n");
	break;
    case GRAPH_CARD_ZERO_TO_ONE :
	printf("       <string>#0#</string>\n");
	break;
    case GRAPH_CARD_ZERO_TO_MANY :
	printf("       <string>#0#</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_ZERO_OR_ONE :
	printf("       <string>#1#</string>\n");
	break;		
    }
   
    printf("          </attribute>\n");
    printf("          <attribute name=\"arrow\">\n");
    printf("            <boolean val=\"false\"/>\n");
    printf("          </attribute>\n");
    printf("          <attribute name=\"aggregate\">\n");
    printf("            <enum val=\"0\"/>\n");
    printf("          </attribute>\n");
    printf("        </composite>\n");
    printf("        <composite>\n");
    printf("          <attribute name=\"role\">\n");
    printf("            <string/>\n");
    printf("          </attribute>\n");

    printf("          <attribute name=\"multiplicity\">\n");

    switch (tEdge->cardinality) {
    case GRAPH_CARD_UNKNOWN :
	printf("       <string># #</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_ONE :
	printf("       <string>#1#</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_MANY :
	printf("       <string>#*#</string>\n");
	break;
    case GRAPH_CARD_ZERO_TO_ONE :
	printf("       <string>#1#</string>\n");
	break;
    case GRAPH_CARD_ZERO_TO_MANY :
	printf("       <string>#*#</string>\n");
	break;
    case GRAPH_CARD_ONE_TO_ZERO_OR_ONE :
	printf("       <string>#0..1#</string>\n");
	break;	
    }
    
    printf("          </attribute>\n");
    printf("          <attribute name=\"arrow\">\n");
    printf("            <boolean val=\"false\"/>\n");
    printf("          </attribute>\n");
    printf("          <attribute name=\"aggregate\">\n");
    printf("            <enum val=\"%d\"/>\n",aggregate);
    printf("          </attribute>\n");
    printf("        </composite>\n");
    printf("      </attribute>\n");

    diaPrintXMLConPoints(tEdge);
    
    printf("    </object>\n");
}

static void diaPrintXMLConnection(GraphEdge *tEdge)
{
    switch (tEdge->connection) {
    case GRAPH_CON_UNKNOWN:
	break;
    case GRAPH_CON_AGGREGATION :
	diaPrintXMLAssociation(tEdge,1);
	break;
    case GRAPH_CON_DEPENDENCY :
	diaPrintXMLDependency(tEdge);
	break;
    case GRAPH_CON_ASSOCIATION :
	diaPrintXMLAssociation(tEdge,0);
	break;	    
    }
}

/*
 * diaPrintXMLInfoNote
 *
 * Prints an UML note with a short information on it (Modulename and
 * smidump version).
 */
static void diaPrintXMLInfoNote(int modc, SmiModule **modv)
{
    size_t  length;
    float   width;
    char   *note;
    int	    i;

    const char *s1 = "Conceptual model of ";
    const char *s2 = "- generated by smidump " SMI_VERSION_STRING;

    /*
     * Calculate the length of the string...
     */
    
    length = strlen(s1) + strlen(s2) + 1;

    for (i = 0; i < modc; i++) {
	length += strlen(modv[i]->name) + 1;
    }

    /*
     * ... before allocating a buffer and putting the string together.
     */

    note = xmalloc(length);
    strcpy(note, s1);
    for (i = 0; i < modc; i++) {
	strcat(note, modv[i]->name);
	strcat(note, " ");
    }
    strcat(note, s2);

    width = strlen(note) * (float)0.76;	/* don't ask */

    printf("<object type=\"UML - Note\" version=\"0\" id=\"O0\">\n");
    printf("  <attribute name=\"obj_pos\">\n");
    printf("    <point val=\"%.2f,%.2f\"/>\n",XNOTE, YNOTE);
    printf("  </attribute>\n");
    printf("  <attribute name=\"obj_bb\">\n");
    printf("    <rectangle val=\"%.2f,%.2f;%.2f,%.2f\"/>\n",
	   XNOTE-0.5, YNOTE-0.5, XNOTE-0.5 + width, YNOTE - 0.5 + 1.7);
    printf("  </attribute>\n");
    printf("  <attribute name=\"elem_corner\">\n");
    printf("    <point val=\"%.2f,%.2f\"/>\n",XNOTE, YNOTE);
    printf("  </attribute>\n");
    printf("  <attribute name=\"elem_width\">\n");
    printf("    <real val=\"%.2f\"/>\n", width);
    printf("  </attribute>\n");
    printf("  <attribute name=\"elem_height\">\n");
    printf("    <real val=\"1.7\"/>\n");
    printf("  </attribute>\n");
    printf("  <attribute name=\"text\">\n");
    printf("    <composite type=\"text\">\n");
    printf("      <attribute name=\"string\">\n");
    printf("        <string>#%s#</string>\n", note);
    printf("      </attribute>\n");
    printf("      <attribute name=\"font\">\n");
    printf("        <font name=\"Courier\"/>\n");
    printf("      </attribute>\n");
    printf("      <attribute name=\"height\">\n");
    printf("        <real val=\"0.8\"/>\n");
    printf("      </attribute>\n");
    printf("      <attribute name=\"pos\">\n");
    printf("        <point val=\"%.2f,%.2f\"/>\n", XNOTE + 0.35, YNOTE + 1.28);
    printf("      </attribute>\n");
    printf("      <attribute name=\"color\">\n");
    printf("        <color val=\"#000000\"/>\n");
    printf("      </attribute>\n");
    printf("      <attribute name=\"alignment\">\n");
    printf("        <enum val=\"0\"/>\n");
    printf("      </attribute>\n");
    printf("    </composite>\n");
    printf("  </attribute>\n");
    printf("</object>\n");

    xfree(note);
}

/*
 * diaCalcSize
 *
 * Calculates the size of a given node for the UML representation.
 */
static GraphNode *diaCalcSize(GraphNode *node)
{
    GraphEdge  *tEdge;
    SmiNode    *tNode,*ppNode;
    SmiElement *smiElement;
    SmiModule  *module;

    if (node->smiNode->nodekind == SMI_NODEKIND_SCALAR) return node;
    
    node->dia.w = (strlen(node->smiNode->name)+4) * HEADFONTSIZETABLE
	+ HEADSPACESIZETABLE;
    
    node->dia.h = TABLEHEIGHT;
    for (smiElement = smiGetFirstElement(
	smiGetFirstChildNode(node->smiNode));
	 smiElement;
	 smiElement = smiGetNextElement(smiElement)) {
	
	tNode = smiGetElementNode(smiElement);
	
	node->dia.w = max(node->dia.w, (strlen(tNode->name) +
					strlen(algGetTypeName(tNode)) +
					strlen(INDEXPROPERTY))
		      * ATTRFONTSIZE
		      + ATTRSPACESIZE);
	node->dia.h += TABLEELEMHEIGHT;
    }
    
    for (tEdge = graphGetFirstEdgeByNode(graph,node);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, node)) {
	if (tEdge->startNode == node &&
	    tEdge->endNode->smiNode->nodekind == SMI_NODEKIND_SCALAR) {
	    node->dia.h += TABLEELEMHEIGHT;
	    tNode = tEdge->endNode->smiNode;
	    
	    node->dia.w = max(node->dia.w, (strlen(tNode->name) +
				    strlen(algGetTypeName(tNode)))
			  * ATTRFONTSIZE
			  + ATTRSPACESIZE);		
	}
    }

    if (PRINT_DETAILED_ATTR && node->smiNode->nodekind == SMI_NODEKIND_TABLE) {
	module  = smiGetNodeModule(node->smiNode);

	for (tNode = smiGetFirstNode(module, SMI_NODEKIND_COLUMN);
	     tNode;
	     tNode = smiGetNextNode(tNode, SMI_NODEKIND_COLUMN)) {
	    ppNode = smiGetParentNode(tNode);
	    ppNode = smiGetParentNode(ppNode);

	    if (cmpSmiNodes(node->smiNode, ppNode)) {
		int len;
		char *typeName;

		typeName = algGetTypeName(tNode);
		len = strlen(tNode->name) + (typeName ? strlen(typeName) : 0);
		node->dia.h += TABLEELEMHEIGHT;
		node->dia.w = max(node->dia.w, len)
		    * ATTRFONTSIZE
		    + ATTRSPACESIZE;
	    }
	}
    }
    
    return node;
}

static float diaPrintNode(GraphNode *node, float x, float y)
{
    GraphEdge *tEdge;

    for (tEdge = graphGetFirstEdgeByNode(graph, node);
	 tEdge;
	 tEdge = graphGetNextEdgeByNode(graph, tEdge, node)) {
	if (! (tEdge->dia.flags & DIA_PRINT_FLAG)) {
	    if (node == tEdge->startNode) {
		y += tEdge->endNode->dia.h + YSPACING;    
		diaPrintXMLObject(tEdge->endNode, x, y);
		diaPrintXMLConnection(tEdge);
		y = diaPrintNode(tEdge->startNode, x, y);
			      /* (x+tEdge->startNode->dia.w+XSPACING),y); */
		
		y = diaPrintNode(tEdge->endNode,
		  (x+tEdge->startNode->dia.w+XSPACING), y);
	    }
	}
    }

    return y;
}

static void diaPrintXML(int modc, SmiModule **modv)
{
    GraphNode *tNode;
    GraphEdge *tEdge;
    float     x,y,ydiff;
    int       group;
    
    diaPrintXMLHeader();

    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {	
	tNode = diaCalcSize(tNode);
    }

    diaPrintXMLInfoNote(modc, modv);
    
    x = XOFFSET;
    y = YOFFSET;
    ydiff = 0;

    for (tEdge = graph->edges; tEdge; tEdge = tEdge->nextPtr) {
	if (! (tEdge->dia.flags & DIA_PRINT_FLAG)) {
	    diaPrintXMLObject(tEdge->startNode, x, y);
	    x = x + tEdge->startNode->dia.w + XSPACING;

	    diaPrintXMLObject(tEdge->endNode, x, y);
	    diaPrintXMLConnection(tEdge);
	    
	    ydiff = tEdge->startNode->dia.h;

      	    y = diaPrintNode(tEdge->startNode,x,y);
	    y = diaPrintNode(tEdge->endNode,x,y);    

	    y = y + ydiff + YSPACING;
	    x = XOFFSET;
	}
    }
    
    x = XOFFSET;
    y += ydiff;
    ydiff = 0;
    
    /* printing singular tables */
    for (tNode = graph->nodes; tNode; tNode = tNode->nextPtr) {
	if (!graphGetFirstEdgeByNode(graph,tNode) &&
	    tNode->smiNode->nodekind != SMI_NODEKIND_SCALAR) {
	    diaPrintXMLObject(tNode,x,y);
	    
	    x += tNode->dia.w + XSPACING;
	    ydiff = max(ydiff, tNode->dia.h);
	    if (x >= NEWLINEDISTANCE) {
		x = XOFFSET;
		y += ydiff + YSPACING;
	    }
	}
    }

    /* printing scalar groups */
    x = XOFFSET;
    y += ydiff + YSPACING;
    for (group = 1;
	 group <= algGetNumberOfGroups();
	 group++) {
	diaPrintXMLGroup(group,x,y);
	x += 2;
	y += 2;
    }
    
    diaPrintXMLClose();
}



/* ------------------------------------------------------------------------- */

static void printModuleNames(int modc, SmiModule **modv)
{
    int i;
    
    printf("Conceptual model of: ");

    for (i = 0; i < modc; i++) {
	printf("%s ", modv[i]->name);
    }

    printf("(generated by smidump " SMI_VERSION_STRING ")\n\n");
}



static void dumpCm(int modc, SmiModule **modv, int flags, char *output)
{
    int       i;

    if (flags & SMIDUMP_FLAG_UNITE) {
	if (! graph) {
	    graph = xmalloc(sizeof(Graph));
	    graph->nodes = NULL;
	    graph->edges = NULL;
	}
	
	for (i = 0; i < modc; i++) {
	    algCreateNodes(modv[i]);
	}
	
	if (XPLAIN) {
	    printModuleNames(modc, modv);
	    printf("\n--- First Phase - loading tables and scalars\n\n");
	    graphShowNodes(graph);
	    printf("\n");
	}
	
	algLinkTables();
	algCheckLinksByName();
	algConnectLonelyNodes();
	algCheckForDependency();
	algCheckForPointerRels();
	
	if (!XPLAIN) {
	    diaPrintXML(modc, modv);
	}
	graphExit(graph);
	graph = NULL;
    } else {
	for (i = 0; i < modc; i++) {
	    if (! graph) {
		graph = xmalloc(sizeof(Graph));
		graph->nodes = NULL;
		graph->edges = NULL;
	    }
	    
	    algCreateNodes(modv[i]);
	    
	    if (XPLAIN) {
		printModuleNames(1, &(modv[i]));
		printf("\n--- First Phase - loading tables and scalars\n\n");
		graphShowNodes(graph);
		printf("\n");
	    }
	
	    algLinkTables();
	    algCheckLinksByName();
	    algConnectLonelyNodes();
	    algCheckForDependency();
	    algCheckForPointerRels();
	    
	    if (!XPLAIN) {
		diaPrintXML(1, &(modv[i]));
	    }
	
	    graphExit(graph);
	    graph = NULL;
	}
    }

    if (fflush(stdout) || ferror(stdout)) {
	perror("smidump: write error");
	exit(1);
    }
}



void initCm()
{
    static SmidumpDriverOption opt[] = {
	{ "explain", OPT_FLAG, &XPLAIN, 0,
	  "explain what the algorithm does"},
        { 0, OPT_END, 0, 0 }
    };

    static SmidumpDriver driver = {
	"cm",
	dumpCm,
	SMI_FLAG_NODESCR,
	SMIDUMP_DRIVER_CANT_OUTPUT,
	"reverse engineered conceptual model",
	opt,
	NULL
    };

    smidumpRegisterDriver(&driver);
}
