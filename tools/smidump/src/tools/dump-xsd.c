/*
 * dump-xsd.c --
 *
 *      Operations to dump SMIng module information as XML schemas.
 *
 * Copyright (c) 2001 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-xsd.c,v 1.52 2002/11/14 17:08:26 tklie Exp $
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


extern int smiAsprintf(char **strp, const char *format, ...);


#define  INDENT                2    /* indent factor */
#define  INDENTVALUE        20   /* column to start values, except multiline */
#define  INDENTTEXTS        4    /* column to start multiline texts */
#define  INDENTMAX        64   /* max column to fill, break lines otherwise */

#ifndef MIN
#define MIN(a,b) ((a)) < ((b)) ? ((a)) : ((b))
#endif /* #ifndef MIN */

static char *schemaLocation = "http://www.ibr.cs.tu-bs.de/~tklie/";

typedef struct XmlEscape {
    char character;
    char *escape;
} XmlEscape;

static XmlEscape xmlEscapes [] = {
    { '<',        "&lt;" },
    { '>',        "&gt;" },
    { '&',        "&amp;" },
    { 0,        NULL }
};


typedef struct TypePrefix {
    char *type;
    char *prefix;
    struct TypePrefix *next;
} TypePrefix;

static TypePrefix *typePrefixes = NULL;

/* part of a display hint */
typedef struct DH {
    unsigned int number;
/*    int repeat;      repeat not yet supported */
    char type;
    char separator;
/*    char repTerm;   repeat not yet supported */
    struct DH *next;
} DH;



/* some forward declarations */
static void fprintElement( FILE *f, int indent, SmiNode *smiNode );
static char* getTypePrefix( char *typeName );

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

static char*
getStringStatus(SmiStatus status)
{
    char *statStr;
    
    switch( status ) {
    case SMI_STATUS_CURRENT:
        statStr = "current";
        break;
    case SMI_STATUS_DEPRECATED:
        statStr = "deprecated";
        break;
    case SMI_STATUS_OBSOLETE:
        statStr = "obsolete";
        break;
    case SMI_STATUS_MANDATORY:
        statStr = "mandatory";
        break;
    case SMI_STATUS_OPTIONAL:
        statStr = "optional";
        break;
    case SMI_STATUS_UNKNOWN:
    default:
        statStr = "unknown";
        break;
    }
    return statStr;
}

static char*
getStringAccess( SmiAccess smiAccess )
{
    switch( smiAccess ) {
    case SMI_ACCESS_NOT_IMPLEMENTED: return "not-implemented";
    case SMI_ACCESS_NOT_ACCESSIBLE : return "not-accessible";
    case SMI_ACCESS_NOTIFY         : return "notify";
    case SMI_ACCESS_READ_ONLY      : return "read-only";
    case SMI_ACCESS_READ_WRITE     : return "read-write";
    case SMI_ACCESS_UNKNOWN:
    default: return "unknown";
    }
}

static int pow1( int base, unsigned int exponent )
{
  unsigned int i;
  int ret = 1;
  
  if( exponent == 0 ) {
    return 1;
  }

  for( i = 0; i < exponent; i++ ) {
    ret *= base;
  }
  return ret;
}

static unsigned int
numDigits( unsigned int val )
{
    int ret  = 1;

    for(; val / 10; val = val / 10 ) {
        ret++;
    }
    
    return ret;
}



#if 0
static void fprint(FILE *f, char *fmt, ...)
{
    va_list ap;
    char    s[300];
    char    *p;
    int result;

    va_start(ap, fmt);

     
    result = vsnprintf(s, sizeof(s), fmt, ap);
    if( result == -1 || result > sizeof( s ) ) {
        fputs( "smidump: warning: output truncated\n", stderr );
        current_column += sizeof( s );
    }
    else {
        current_column += result;
    }
    va_end(ap);

    fputs(s, f);

    if ((p = strrchr(s, '\n'))) {
        current_column = strlen(p) - 1;
    }
}
#endif /* 0 */


static void fprintSegment(FILE *f, int column, char *string, int length)
{
    fprintf(f, "%*c%s", column, ' ', string);
    if (length) {
        fprintf(f, "%*c", length - strlen(string) - column, ' ');
    }
}



static void fprintMultilineString(FILE *f, int column, const char *s)
{
    int i, j, len;

#ifdef INDENTTEXTS
    fprintSegment(f, column + INDENTTEXTS, "", 0);
#endif
    if (s) {
        len = strlen(s);
        for (i=0; i < len; i++) {
            for (j = 0; xmlEscapes[j].character; j++) {
                if (xmlEscapes[j].character == s[i]) break;
            }
            if (xmlEscapes[j].character) {
                fputs(xmlEscapes[j].escape, f);
            } else {
                putc(s[i], f);
            }
            if (s[i] == '\n') {
#ifdef INDENTTEXTS
                fprintSegment(f, column + INDENTTEXTS, "", 0);
#endif
            }
        }
    }
}



static void fprintDocumentation(FILE *f, int indent, const char *description)
{
    if (description) {
        fprintSegment(f, indent, "<xsd:documentation>\n", 0);
        fprintMultilineString(f, indent, description);
        fprintf(f, "\n");
        fprintSegment(f, indent, "</xsd:documentation>\n", 0);
    }
}

static void fprintNamedNumber( FILE *f, int indent, SmiNamedNumber *nn )
{
    fprintSegment( f, indent, "<xsd:enumeration ", 0 );
    fprintf( f, "value=\"%s\">\n", nn->name );
    fprintSegment( f, indent + INDENT, "<xsd:annotation>\n", 0 );
    fprintSegment( f, indent + 2 * INDENT, "<xsd:appinfo>\n", 0 );
    fprintSegment( f, indent + 3 * INDENT, "<intVal>", 0 );
    fprintf( f, "%d</intVal>\n", (int)nn->value.value.integer32 );
    fprintSegment( f, indent + 2 * INDENT, "</xsd:appinfo>\n", 0 );
    fprintSegment( f, indent + INDENT, "</xsd:annotation>\n", 0 );
    fprintSegment( f, indent, "</xsd:enumeration>\n", 0 );
}


static void fprintStdRestHead( FILE *f, int indent, SmiType *smiType )
{
    char *baseTypeName = getStringBasetype(smiType->basetype);
    char *prefix = getTypePrefix( baseTypeName );
    
    fprintSegment(f, indent, "<xsd:restriction", 0);
    if( prefix ) {
        fprintf(f, " base=\"%s:%s\">\n", prefix, baseTypeName );
    }
    else {
        fprintf(f, " base=\"%s\">\n", baseTypeName );
    }
}


static void
fprintHexOrAsciiType( FILE *f, int indent, SmiType *parent,
                      SmiInteger32 minLength, SmiInteger32 maxLength,
                      char *name, int hex )
{
    char *prefix = parent ? getTypePrefix( parent->name ) : NULL;
    char *typeFlag = hex ? "Hex" : "Ascii";
    
    fprintSegment( f, indent, "<xsd:simpleType", 0 );
    if( name ) {
        fprintf( f, " name=\"%s%s\"", name, typeFlag );
    }
    fprintf( f,">\n" );
    fprintSegment( f, indent + INDENT,
                   "<xsd:restriction ", 0 );
    if( prefix ) {
        fprintf( f, "base=\"%s:%s%s\">\n", prefix, parent->name, typeFlag );
    }
    else {
        fprintf( f, "base=\"%s%s\">\n",parent->name, typeFlag );
    }
   
    if( minLength > 0 ) {
        fprintSegment( f, indent + 2 * INDENT, "<xsd:minLength", 0 );
        fprintf( f, " value=\"%d\"/>\n", (int)minLength );
    }
    if( maxLength > -1 ) {

        fprintSegment( f, indent + 2 * INDENT, "<xsd:maxLength", 0 );
        fprintf( f, " value=\"%d\"/>\n", (int)maxLength );
    }

    fprintSegment( f, indent + INDENT, "</xsd:restriction>\n", 0 );
    fprintSegment( f, indent, "</xsd:simpleType>\n", 0 );
}

#if 0
static void
fprintStringUnion( FILE *f, int indent, SmiType *smiType,
                   SmiInteger32 minLength, SmiInteger32 maxLength,
                   int secondTime, char *typeName )
{
    SmiType *parent = smiGetParentType( smiType );

    if( !secondTime ) {
        /* let's create a union of an hexBinary and an ascii string */
        fprintSegment( f, indent, "<xsd:union", 0 );
        
        if( smiType->decl == SMI_DECL_IMPLICIT_TYPE ) {
            
            fprintf( f, ">\n" );

            /* print hexBinary subtype */
            fprintHexOrAsciiType( f, indent + INDENT, parent,
                                  minLength, maxLength, NULL, 1 );
            /* print Ascii subtype */
            fprintHexOrAsciiType( f, indent + INDENT, parent,
                                  minLength, maxLength, NULL, 0 );
            
            /* finish the union type */
            fprintSegment( f, indent, "</xsd:union>\n", 0 );
            
        }
        else {
            fprintf( f, " memberTypes=\"%sHex %sAscii\"/>\n",
                    smiType->name, smiType->name );
        }
    }
    else {
        /* we are her for the second time to print out the subtypes
           <typeName>Hex and <typeName>Ascii */

        fprintHexOrAsciiType( f, indent + INDENT, parent, minLength, maxLength,
                              typeName, 1 );
        fprintHexOrAsciiType( f, indent + INDENT, parent, minLength, maxLength,
                              typeName, 0 );
    }
}
#endif /* 0 */

static void
initDH( struct DH *dh )
{
            /* init with NULLs */
        dh->number = 0;
        /*dh->repeat = 0; repeat not yet supported */
        dh->type = 0;
        dh->separator = '\0';
        /* dh->repTerm = '\0'; repeat not yet supported */
        dh->next = NULL;
}

/* parse a display hint and create a list of DHs */
static struct DH
*parseDH( const char *hint )
{
    struct DH *iterDH = (struct DH *)malloc( sizeof( struct DH ) );
    struct DH *ret = iterDH;
    struct DH *oldIterDH = iterDH;
    unsigned int pos = 0;
    
    if( !ret ) {
        return NULL;
    }

    initDH( iterDH );
    while( pos < strlen( hint ) ) {

        if( ! iterDH ) {
            iterDH = (struct DH *)malloc( sizeof( struct DH ) );
            if( ! iterDH ) return NULL;
            initDH( iterDH );
            oldIterDH->next = iterDH;
            oldIterDH = iterDH;
        }

        switch( hint[ pos ] ) {
            
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            unsigned int endPos;
            char *strNum;
            
            /* find end of number */
            for( endPos = pos; isdigit( hint[ endPos ] ); endPos++ );
            
            /* parse number */
            strNum = (char *)malloc( endPos - pos );
            strncpy( strNum, &hint[ pos ], endPos - pos );
            strNum[ endPos - pos ] = '\0';
            iterDH->number = atoi( strNum );
            free( strNum );
            
            /* forward the position pointer */
            pos = endPos;
            break;
        }
        
        case 'a':
        case 'b':
        case 'd':
        case 'o':
        case 'x':
            iterDH->type = hint[ pos++ ];
            if( isdigit( hint[ pos ] ) || hint[ pos ] == '*' ) {
                iterDH = iterDH->next;
            }
            break;
            
        case '*':
            /* repeat not supported */
            return NULL;
            
        default:
            if( iterDH->separator ) {
                /* iterDH->repTerm = hint[ pos++ ];  repeat not supported */
            }
            else {
                iterDH->separator = hint[ pos++ ];
            }
            if( isdigit( hint[ pos ] ) || hint[ pos ] == '*' ) {
                iterDH = iterDH->next;
            }
            break;
        }        
    }
    return ret;
}


/* build a regexp from this display hint */
static char* getStrDHType( char *hint,
                           SmiInteger32 *lengths, unsigned int numSubranges  )
{
    unsigned int i = 0;
    char *ret = lengths[ i ] ? "(" : "((";
    DH *dh = parseDH( hint );

    if(! dh ) return NULL;

    do {
        unsigned int octetsUsed = 0;
        DH *iterDH;
        
        for( iterDH = dh; iterDH; iterDH = iterDH->next ) {
            char *baseRegexp = NULL;

            switch( iterDH->type ) {

            case 'a':
                /* ascii character */
                baseRegexp = "(#x00-#x7f)";
                break;

            case 'b':
                /* binary number */
                baseRegexp = "((0|1){8})";
                break;

            case 'd':
                /* decimal number */
                baseRegexp = "([0-9]{3})";
                break;

            case 'o':
                /* octal number */
                baseRegexp = "([0-7]{3})";
                break;

            case 'x':
                /* hexadecimal number */
                baseRegexp = "([0-9A-Fa-f]{2})";
                break;
            }
            

            if( iterDH->number < lengths[ i ] ) {
                /* there are more octets to come */
                if( iterDH->type == 'd' ) {
                    /* decimal number needs to be treated differently */
                    smiAsprintf( &ret, "%s(0|[1-9](([0-9]){0,%d}))",
                                 ret,
                                 numDigits( pow1( 255, iterDH->number ) ) - 1);

                    octetsUsed += iterDH->number;                    
                    if( octetsUsed >= lengths[ i + 1 ] ) {
                        /* maximum number of octets used,
                           we must exit the loop */
                        break;
                    }
                    
                    else if( iterDH->separator ) {
                        smiAsprintf( &ret, "%s%c", ret, iterDH->separator );
                    }
                }
                else {
                    if( iterDH->next ){
                        smiAsprintf( &ret, "%s(%s{%d})",
                                     ret,
                                     baseRegexp, iterDH->number );

                        octetsUsed += iterDH->number;
                        if( octetsUsed >= lengths[ i + 1 ] ) {
                        /* maximum number of octets used,
                           we must exit the loop */                            
                            break;
                        }

                        if( iterDH->separator ) {
                            smiAsprintf( &ret, "%s%c", ret, iterDH->separator );
                        }
                    }
                    else {                        
                        smiAsprintf( &ret, "%s(%s",
                                  ret, baseRegexp );

                        if( iterDH->separator ) {
                            smiAsprintf( &ret, "%s%c", ret, iterDH->separator );
                        }

                        smiAsprintf( &ret, "%s{%u,%u})%s",
                                     ret, lengths[ i ] - 1, lengths[ i+1 ] - 1,
                                     baseRegexp );
                        
                        octetsUsed += iterDH->number;
                        if( octetsUsed >= lengths[ i + 1 ] ) {
                            /* maximum number of octets used,
                               we must exit the loop */                            
                            break;
                        }
                    }
                }
            }
            else {
                /* might be the last one */

                if( iterDH->type == 'd' ) {
                    /* decimal number needs to be treated differently */
                    if( iterDH->number < lengths[ i+1 ] ) {
                        smiAsprintf( &ret, "%s(0|[1-9]([0-9]{0,%d}))",
                                     ret,
                                     numDigits( pow1( 255, iterDH->number ) ) );

                        octetsUsed += lengths[ i ];
                        if( octetsUsed >= lengths[ i + 1 ] ) {
                            /* the maximum number of octets have been reached,
                               we must exit the loop */
                            break;
                        }

                        if( iterDH->separator ) {
                            smiAsprintf( &ret, "%s%c", ret, iterDH->separator );
                        }                                                
                    }
                    else {
                        smiAsprintf( &ret, "%s(0|[1-9]([0-9]{0,%d})",
                                     ret,
                                     numDigits( pow1( 255, lengths[ i+1 ] ) ) );
                    }
                }
                else {
                    smiAsprintf( &ret, "%s(%s",  ret, baseRegexp );
                    if( iterDH->next ) {
                        if( iterDH->separator ) {
                            smiAsprintf( &ret, "%s%c", ret, iterDH->separator );
                        }
                        if( ! lengths[ i ] && lengths[ i+1 ] == 65535 ) {
                            smiAsprintf( &ret, "%s)*",ret );
                        }
                        else{
                            smiAsprintf( &ret, "%s){%u,%u}",ret, lengths[ i ],
                                         MIN( iterDH->number,
                                              lengths[ i + 1] ) - 1 );
                        }
                        octetsUsed += lengths[ i ];
                        if( octetsUsed >= lengths[ i + 1 ] ) {
                            /* the maximum number of octets have been reached,
                               we must exit the loop */
                            break;
                        }                                                
                    }
                    else {
                        octetsUsed += lengths[ i ];
                        if( iterDH->separator &&
                            octetsUsed < lengths[ i + 1 ] ) {

                            if( ! lengths[ i ] && lengths[ i+1 ] == 65535 ) {
                                smiAsprintf( &ret, "%s%c)*%s",
                                             ret, iterDH->separator, baseRegexp );
                            }
                            else {
                                smiAsprintf( &ret, "%s%c){%u,%u}%s",
                                             ret, iterDH->separator,
                                             lengths[ i ], lengths[ i + 1] - 1,
                                             baseRegexp );
                            }
                        }
                        else {
                            if( ! lengths[ i ] && lengths[ i+1 ] == 65535 ) {
                                smiAsprintf( &ret, "%s)*%c",
                                             ret, iterDH->separator );
                            }
                            else {
                                smiAsprintf( &ret, "%s){%u,%u}%c",
                                             ret, lengths[ i ],
                                             lengths[ i + 1],
                                             iterDH->separator );
                             }                            
                        }
                    }
                }
                
                if( octetsUsed >= lengths[ i + 1 ] ) {
                    /* the maximum number of octets have been reached,
                       we must exit the loop */
                    break;
                }
            }
        }
        i += 2;

        if( i < numSubranges  * 2 ) {
            smiAsprintf( &ret, "%s)|(", ret );
        }
        else {
            smiAsprintf( &ret, "%s)", ret );
            if( ! lengths[ i - 2 ] ) {
                smiAsprintf( &ret, "%s){0,1}", ret );
            }
        }
    } while( i < numSubranges * 2 );
    
    return ret;
}

static int
dhInParent( SmiType *smiType ) {
    SmiType *parent = smiGetParentType( smiType );

    if( smiType->format && parent->format ) {
        return ! strcmp( smiType->format, parent->format );
    }
    return 0;
   
}

#define MD_DH_INT_NORMAL   1
#define MD_DH_INT_DECIMAL  2
#define MD_DH_INT_BIN      3
#define MD_DH_INT_OCT      4
#define MD_DH_INT_HEX      5

/* parse a (integer) display hint and specify the offset, if used */
static int getIntDHType( char *hint, int *offset )
{
    switch( hint[ 0 ] ) {

    case 'd':
        if( hint[1] ) {
            *offset = 0;
            *offset = atoi( &hint[2] );
            return MD_DH_INT_DECIMAL;
        }
        return MD_DH_INT_NORMAL;

    case 'b':
        /* binary value */
        return MD_DH_INT_BIN;
    case 'o':
        /* octet value */
        return MD_DH_INT_OCT;
    case 'x':
        /* hex value */
        return MD_DH_INT_HEX;
    default:
        /* should not occur */
        return 0;
    }
}


static void fprintRestriction(FILE *f, int indent, SmiType *smiType)
{
    SmiRange *smiRange;
    
    /* print ranges etc. */
    switch( smiType->basetype ) {

    case SMI_BASETYPE_INTEGER32:
    {
        SmiInteger32 min = -2147483647, max = 2147483646;
        int offset, useDecPoint = 0;

        if( smiType->format ) {
          /* we have a display hint here, so check if we have to use
             a decimal point */
          useDecPoint =  
            getIntDHType( smiType->format, &offset ) == MD_DH_INT_DECIMAL; 
          /* xxx: other display hint types (binary, oct, hex) */
        }

        if( useDecPoint ) {
          fprintSegment( f, indent,
                         "<xsd:restriction base=\"xsd:decimal\">\n", 0 );
          fprintSegment( f, indent + INDENT, "<xsd:fractionDigits", 0 );
          fprintf( f, "=\"%d\"/>\n", offset );
        }
        else {
          fprintStdRestHead( f, indent, smiType );
        }

        smiRange = smiGetFirstRange( smiType );
        while( smiRange ) {
            if( min == -2147483647 ||
                smiRange->minValue.value.integer32 < min ) {
                min = smiRange->minValue.value.integer32;
            }
            if( max == 2147483646 ||
                smiRange->maxValue.value.integer32 > max ) {
                max = smiRange->maxValue.value.integer32;
            }
            smiRange = smiGetNextRange( smiRange );
        }
        
        /* print minimu value */
        fprintSegment( f, indent + INDENT, "<xsd:minInclusive", 0 );        
        if( useDecPoint ) {
          fprintf( f, " value=\"%d.%d\"/>\n", 
                   (int)min / pow1( 10, offset ), 
                   abs( (int)min % pow1( 10, offset ) ) );
        }
        else {
          fprintf( f, " value=\"%d\"/>\n", (int)min );
        }

        /* print maximum value */
        fprintSegment( f, indent + INDENT, "<xsd:maxInclusive", 0 );
        if( useDecPoint ) {
          fprintf( f, " value=\"%d.%d\"/>\n", 
                   (int)max / pow1( 10, offset ), 
                   abs( (int)max % pow1( 10, offset ) ) );
        }
        else {
          fprintf( f, " value=\"%d\"/>\n", (int)max );
        }
        
        fprintSegment(f, indent, "</xsd:restriction>\n", 0);
        break;
    }
    
    case SMI_BASETYPE_OCTETSTRING:
    {
        SmiInteger32  minLength, maxLength;
        unsigned int numSubRanges = 0;
        
        minLength = 0;
        maxLength = -1;

        /* get range details */
        for( smiRange = smiGetFirstRange( smiType );
             smiRange;
             smiRange = smiGetNextRange( smiRange ) ) {
            if( minLength == 0 ||
                smiRange->minValue.value.integer32 < minLength ) {
                minLength = smiRange->minValue.value.integer32;             
            }
            if( smiRange->maxValue.value.integer32 > maxLength ) {
                maxLength = smiRange->maxValue.value.integer32;
            }
            numSubRanges++;
        }

        

        if( smiType->format &&
            ( smiType->decl == SMI_DECL_IMPLICIT_TYPE ||
              smiType->decl == SMI_DECL_TEXTUALCONVENTION ) &&
            ! dhInParent( smiType ) ) {
            /*
            fprintStringUnion( f, indent, smiType,
                               minLength, maxLength, 0, NULL );
            */
            SmiUnsigned32 *lengths;
            char *dh;
            if( numSubRanges ) {
                unsigned int lp = 0;
                SmiRange *smiRange;
                lengths = xmalloc( 2 * sizeof( SmiUnsigned32 ) *
                                   numSubRanges );
                                
                /* write subtype lengths to the array */
                for( smiRange = smiGetFirstRange( smiType );
                     smiRange;
                     smiRange = smiGetNextRange( smiRange ) ) {
                    lengths[ lp++ ] = smiRange->minValue.value.unsigned32;
                    lengths[ lp++ ] = smiRange->maxValue.value.unsigned32;
                }
            }
            else {
                lengths = xmalloc( 2 * sizeof( SmiUnsigned32 ) );
                lengths[0] = 0;
                lengths[1] = 65535;
            }
            
            fprintSegment( f, indent, "<xsd:restriction ", 0 );
            fprintf( f, "base=\"xsd:string\">\n" );

            /* create regexp */
            dh = getStrDHType( smiType->format, lengths, numSubRanges );

            if( dh ) {
                fprintSegment( f, indent + 2 * INDENT, "<xsd:pattern ", 0 );
                fprintf( f, "value=\"%s\"/>\n", dh );
                fprintSegment( f, indent, "</xsd:restriction>\n", 0 );
            }
            else {
                fprintf( f, "<!-- Warning: repeat in display hint. " );
                fprintf( f, "This feature is not supported. -->\n" );
            }
            
            xfree( lengths );
        }
        else {
            SmiType *parent = smiGetParentType( smiType );
            /*
            fprintStringUnion( f, indent, smiType,
                               minLength, maxLength, secondTime,
                               smiType->name );
            */
            if( parent ) {
                if( ! parent->format ) {
                    char *prefix = getTypePrefix( parent->name );

                    fprintSegment( f, indent, "<xsd:restriction", 0 );
                    if( prefix ) {
                        fprintf( f, " base=\"%s:%s\">\n",
                                 prefix, parent->name );
                    }
                    else {
                        fprintf( f, " base=\"%s\">\n", parent->name );
                    }

                    /* print length restriction */
                    fprintSegment( f, indent + INDENT, "<xsd:minLength ", 0 );
                    fprintf( f, "value=\"%d\"/>\n", (int)minLength );
                    fprintSegment( f, indent + INDENT, "<xsd:maxLength ", 0 );
                    fprintf( f, "value=\"%d\"/>\n", (int)maxLength );
                    
                    fprintSegment( f, indent, "</xsd:restriction>\n", 0 );
                }
                else {
                    SmiUnsigned32 *lengths =
                        xmalloc( 2 * sizeof( SmiUnsigned32 ) * numSubRanges );
                    unsigned int lp = 0;
                    SmiRange *smiRange;
                    
                    /* write subtype lengths to the array */
                    for( smiRange = smiGetFirstRange( smiType );
                         smiRange;
                         smiRange = smiGetNextRange( smiRange ) ) {
                        lengths[ lp++ ] = smiRange->minValue.value.unsigned32;
                        lengths[ lp++ ] = smiRange->maxValue.value.unsigned32;
                    }
                    
                    fprintSegment( f, indent + INDENT, "<xsd:restriction ",
                                   0 );
                    fprintf( f, "base=\"xsd:string\">\n" );
                    fprintSegment( f, indent + 2 * INDENT, "<xsd:pattern ",
                                   0 );
                    fprintf( f, "value=\"%s\"/>\n",
                             getStrDHType( parent->format,
                                           lengths, numSubRanges ) );
                    fprintSegment( f, indent + INDENT, "<xsd:restriction>\n",
                                   0 );
                    xfree( lengths );
                }
                
            }
        }
        break;
    }

    case SMI_BASETYPE_FLOAT128:
    {
/*        SmiFloat128 min, max; */
        fprintStdRestHead( f, indent, smiType );
        
        /* xxx, only SMIng */
        break;
    }

    case SMI_BASETYPE_FLOAT64:
    {
/*        SmiFloat64 min,max;*/
        fprintStdRestHead( f, indent, smiType );

        /* xxx, only SMIng */
        break;
    }
    
    case SMI_BASETYPE_FLOAT32:
    {
/*        SmiFloat32 min,max;*/
        fprintStdRestHead( f, indent,  smiType );
        
        /* xxx, only SMIng */
        break;
    }
    
    case SMI_BASETYPE_INTEGER64:
    {
/*        SmiInteger64 min,max;*/
        fprintStdRestHead( f, indent, smiType );
        
        /* xxx, only SMIng */
        break;
    }

    case SMI_BASETYPE_UNSIGNED64:
    {
        SmiUnsigned64 min, max;

        min = 0;
        max = 18446744073709551615Ull;

        fprintStdRestHead( f, indent, smiType );
        
        smiRange = smiGetFirstRange( smiType );
        while( smiRange ) {
            if( smiRange->minValue.value.unsigned64 < min ) {
                min = smiRange->minValue.value.unsigned64;
            }
            if( smiRange->maxValue.value.unsigned64 > max ) {
                max = smiRange->maxValue.value.unsigned64;
            }
            smiRange = smiGetNextRange( smiRange );
        }
        fprintSegment( f, indent + INDENT, "<xsd:minInclusive", 0 );
        fprintf( f, " value=\"%lu\"/>\n", (unsigned long)min );

        fprintSegment( f, indent + INDENT, "<xsd:maxInclusive", 0 );
        fprintf( f, " value=\"%lu\"/>\n",(unsigned long)max );
        
        fprintSegment(f, indent, "</xsd:restriction>\n", 0);
        
        break;
    }

    case SMI_BASETYPE_UNSIGNED32:
    {
        SmiUnsigned32 min, max;

        min = 0;
        max = 4294967295UL;

        fprintStdRestHead( f, indent, smiType );
        
        smiRange = smiGetFirstRange( smiType );
        while( smiRange ) {
            if( smiRange->minValue.value.unsigned32 < min ) {
                min = smiRange->minValue.value.unsigned32;
            }
            if( smiRange->maxValue.value.unsigned32 > max ) {
                max = smiRange->maxValue.value.unsigned32;
            }
            smiRange = smiGetNextRange( smiRange );
        }
        fprintSegment( f, indent + INDENT, "<xsd:minInclusive", 0 );
        fprintf( f, " value=\"%u\"/>\n", (unsigned int)min );

        fprintSegment( f, indent + INDENT, "<xsd:maxInclusive", 0 );
        fprintf( f, " value=\"%u\"/>\n",(unsigned int)max );
        
        fprintSegment(f, indent, "</xsd:restriction>\n", 0);
        break;
    }

    case SMI_BASETYPE_ENUM:
    case SMI_BASETYPE_BITS:
    {
        SmiNamedNumber *nn;
        
        fprintSegment(f, indent, "<xsd:restriction base=\"xsd:NMTOKEN\">\n",
                      0);

        /* iterate named numbers */
        for( nn = smiGetFirstNamedNumber( smiType );
             nn;
             nn = smiGetNextNamedNumber( nn ) ) {
            fprintNamedNumber( f, indent + INDENT, nn );
        }
        fprintSegment(f, indent, "</xsd:restriction>\n", 0);
        break;
    }

    case SMI_BASETYPE_OBJECTIDENTIFIER:
        fprintSegment( f, indent,
                       "<xsd:restriction base=\"smi:ObjectIdentifier\"/>\n", 0 );
        break;
    case SMI_BASETYPE_UNKNOWN:
        /* should not occur */
        break;
    }
}
    

static unsigned int
getNamedNumberCount( SmiType *smiType )
{
    SmiNamedNumber *nn;
    unsigned int ret = 0;

    for( nn = smiGetFirstNamedNumber( smiType );
         nn;
         nn = smiGetNextNamedNumber( nn ) ) {
        ret++;
    }

    return ret;
}


static void fprintBitList( FILE *f, int indent, SmiType *smiType,
                           const char *name )
{
    fprintSegment( f, indent, "<xsd:simpleType", 0 );
    fprintf( f, " name=\"%sBitList\">\n", name );
    fprintSegment( f, indent + INDENT, "<xsd:list", 0 );
    fprintf( f, " itemType=\"%sBit\"/>\n", name );
    fprintSegment( f, indent, "</xsd:simpleType>\n\n", 0 );
    
    fprintSegment( f, indent, "<xsd:simpleType", 0 );
    fprintf( f, " name=\"%sBits\">\n", name );
    fprintSegment( f, indent + INDENT, "<xsd:restriction", 0 );
    fprintf( f, " base=\"%sBitList\">\n", name );
    fprintSegment( f, indent + 2 * INDENT, "<xsd:maxLength", 0 );
    fprintf( f, " value=\"%d\"/>\n", getNamedNumberCount( smiType ) );
    fprintSegment( f, indent + INDENT, "</xsd:restriction>\n", 0 );
    fprintSegment( f, indent, "</xsd:simpleType>\n\n", 0 );
}

static int getNumSubRanges( SmiType *smiType )
{
    SmiRange *smiRange;
    int num = 0;

    for( smiRange = smiGetFirstRange( smiType );
         smiRange;
         smiRange = smiGetNextRange( smiRange ) ) {
        num++;
    }
    
    return num;
}


static void fprintSubRangeType( FILE *f, int indent,
                                SmiRange *smiRange, SmiType *smiType )
{
    
    switch( smiType->basetype ) {

    case SMI_BASETYPE_UNSIGNED32: {
        SmiUnsigned32 min, max;

        min = 0;
        max = 4294967295UL;

        if( smiRange->minValue.value.unsigned32 < min ) {
            min = smiRange->minValue.value.unsigned32;
        }
        if( smiRange->maxValue.value.unsigned32 > max ) {
            max = smiRange->maxValue.value.unsigned32;
        }
        
        fprintSegment( f, indent, "<xsd:simpleType>\n", 0 );
        fprintStdRestHead( f, indent + INDENT, smiType );

        fprintSegment( f, indent + 2 * INDENT, "<xsd:minInclusive", 0 );
        fprintf( f, " value=\"%u\"/>\n", (unsigned int)min );
        
        fprintSegment( f, indent + 2 * INDENT, "<xsd:maxInclusive", 0 );
        fprintf( f, " value=\"%u\"/>\n",(unsigned int)max );

        fprintSegment(f, indent + INDENT, "</xsd:restriction>\n", 0);
        break;
    }

    case SMI_BASETYPE_INTEGER32: {
        SmiInteger32 min, max;

        min = -2147483647;
        max = 2147483646;

        if( min == -2147483647 ||
            smiRange->minValue.value.integer32 < min ) {
            min = smiRange->minValue.value.integer32;
        }
        if( max == 2147483646 ||
            smiRange->maxValue.value.integer32 > max ) {
            max = smiRange->maxValue.value.integer32;
        }

        fprintSegment( f, indent, "<xsd:simpleType>\n", 0 );
            fprintStdRestHead( f, indent + INDENT, smiType );
        
        fprintSegment( f, indent + 2 * INDENT, "<xsd:minInclusive", 0 );
        fprintf( f, " value=\"%d\"/>\n", (int)min );

        fprintSegment( f, indent + 2 * INDENT, "<xsd:maxInclusive", 0 );
        fprintf( f, " value=\"%d\"/>\n",  (int)max );

        fprintSegment(f, indent + INDENT, "</xsd:restriction>\n", 0);        
        break;
        
    }

    case SMI_BASETYPE_OCTETSTRING: {
        SmiInteger32  minLength, maxLength;
        
        minLength = 0;
        maxLength = -1;
        
        if( smiRange->minValue.value.integer32 < minLength ) {
            minLength = smiRange->minValue.value.integer32;             
        }
        if( smiRange->maxValue.value.integer32 > maxLength ) {
            maxLength = smiRange->maxValue.value.integer32;
        }
        fprintHexOrAsciiType( f, indent + INDENT, smiType,
                              minLength, maxLength, NULL, 1 );
        break;
    }

    case SMI_BASETYPE_FLOAT128:
    {
/*        SmiFloat128 min, max; 
        xxx, only SMIng */
        break;
    }

    case SMI_BASETYPE_FLOAT64:
    {
/*        SmiFloat64 min,max;
        xxx, only SMIng */
        break;
    }
    
    case SMI_BASETYPE_FLOAT32:
    {
/*        SmiFloat32 min,max;
        xxx, only SMIng */
        break;
    }

    case SMI_BASETYPE_INTEGER64:
    {
/*        SmiInteger64 min,max;
         xxx, only SMIng */
        break;
    }

    case SMI_BASETYPE_UNSIGNED64:
    {
        SmiUnsigned64 min, max;

        min = 0;
        max = 18446744073709551615Ull;

        if( smiRange->minValue.value.unsigned64 < min ) {
            min = smiRange->minValue.value.unsigned64;
        }
        if( smiRange->maxValue.value.unsigned32 > max ) {
            max = smiRange->maxValue.value.unsigned64;
        }
        
        fprintSegment( f, indent, "<xsd:simpleType>\n", 0 );
        fprintStdRestHead( f, indent + INDENT, smiType );

        fprintSegment( f, indent + 2 * INDENT, "<xsd:minInclusive", 0 );
        fprintf( f, " value=\"%lu\"/>\n", (unsigned long)min );

        fprintSegment( f, indent + 2 * INDENT, "<xsd:maxInclusive", 0 );
        fprintf( f, " value=\"%lu\"/>\n",(unsigned long)max );

        fprintSegment(f, indent + INDENT, "</xsd:restriction>\n", 0);
        break;
    }

    case SMI_BASETYPE_ENUM:
    case SMI_BASETYPE_BITS:
    case SMI_BASETYPE_OBJECTIDENTIFIER:
    case SMI_BASETYPE_UNKNOWN:
        /* should not occur */
        break;
        
    }
    
    fprintSegment(f, indent, "</xsd:simpleType>\n", 0 );
}

static void fprintDisplayHint( FILE *f, int indent, char *format )
{
    fprintSegment( f, indent, "<displayHint>", 0 );
    fprintf( f, "%s</displayHint>\n", format );
}




static void fprintTypedef(FILE *f, int indent, SmiType *smiType,
                          const char *name)
{
    SmiRange *smiRange;
    unsigned int numSubRanges = getNumSubRanges( smiType );
    
    if ( name ) {
        if( smiType->basetype == SMI_BASETYPE_BITS ) {
            fprintSegment(f, indent, "<xsd:simpleType", 0);
            fprintf(f, " name=\"%sBit\"", name);
            fprintf(f, ">\n");
        }
/*        else if( smiType->basetype == SMI_BASETYPE_OCTETSTRING ) {
            fprintSegment(f, indent, "<xsd:simpleType", 0);
            fprintf( f, " name=\"%sType\">\n", name );
            }*/
        else {
            fprintSegment(f, indent, "<xsd:simpleType", 0);
            fprintf(f, " name=\"%s\"", name);
            fprintf(f, ">\n");
        }
    }

    else {
        /* unnamed simple type */
        fprintSegment(f, indent, "<xsd:simpleType>\n", 0 );
    }

    if( smiType->description ) {
        fprintSegment( f, indent + INDENT, "<xsd:annotation>\n", 0 );
        fprintDocumentation(f, indent + 2 * INDENT, smiType->description);
        if( smiType->format ) {
            fprintSegment( f, indent + 2 * INDENT, "<xsd:appinfo>\n", 0 );
            fprintDisplayHint( f, indent + 3 * INDENT, smiType->format );
            fprintSegment( f, indent + 2 * INDENT, "</xsd:appinfo>\n", 0 );
        }
        fprintSegment( f, indent + INDENT, "</xsd:annotation>\n", 0 );
    }
    
    if( smiType->format && smiType->basetype == SMI_BASETYPE_OCTETSTRING ) {
        fprintRestriction( f, indent + INDENT, smiType );
        fprintSegment(f, indent, "</xsd:simpleType>\n", 0);
    }
    
    else if( ( numSubRanges > 1 ) &&
             ( smiType->basetype != SMI_BASETYPE_OCTETSTRING ) ) {
        
        fprintSegment( f, indent + INDENT, "<xsd:union>\n", 0 );
        
        for( smiRange = smiGetFirstRange( smiType );
             smiRange;
             smiRange = smiGetNextRange( smiRange ) ) {
            fprintSubRangeType( f, indent + 2 * INDENT, smiRange, smiType );
        }
        
        fprintSegment( f, indent + INDENT, "</xsd:union>\n", 0 );
        fprintSegment(f, indent, "</xsd:simpleType>\n", 0);
    }
    else {
        fprintRestriction(f, indent + INDENT, smiType );
        fprintSegment(f, indent, "</xsd:simpleType>\n", 0);
    }

    /* print an empty line after global types */
    if( smiType->decl != SMI_DECL_IMPLICIT_TYPE && name ) {
        fprintf( f, "\n" );
    }
  

    if( smiType->basetype == SMI_BASETYPE_BITS ) {
        /* if basetype is BITS we have to do something more */
        fprintBitList( f, indent, smiType, name );
    }
}


static char* getTypePrefix( char *typeName )
{
    TypePrefix *iterTPr;

    if( !typeName ) {
        return NULL;
    }

    for( iterTPr = typePrefixes; iterTPr; iterTPr = iterTPr->next ) {
        if( ! strcmp( iterTPr->type, typeName ) ) {
            return iterTPr->prefix;
        }
    }

    return NULL;
}


static void fprintAnnotationElem( FILE *f, int indent, SmiNode *smiNode ) {
    int i;
    
    fprintSegment( f, indent, "<xsd:annotation>\n", 0 );
    fprintSegment( f, indent + INDENT, "<xsd:appinfo>\n", 0 );

    fprintSegment( f, indent + 2 * INDENT, "<maxAccess>", 0 );
    fprintf( f, "%s</maxAccess>\n", getStringAccess( smiNode->access ) );

    fprintSegment( f, indent + 2 * INDENT, "<status>", 0 );
    fprintf( f, "%s</status>\n",  getStringStatus( smiNode->status ) );

    fprintSegment( f, indent + 2 * INDENT, "<oid>", 0 );
    for (i = 0; i < smiNode->oidlen; i++) {
        fprintf(f, i ? ".%u" : "%u", smiNode->oid[i]);
    }
    fprintf( f, "</oid>\n" );

    if( smiNode->format ) {
        fprintDisplayHint( f, indent + 2 * INDENT, smiNode->format );
    }

    if( smiNode->units ) {
        fprintSegment( f, indent + 2 *INDENT, "<units>", 0 );
        fprintf( f, "%s</units>\n", smiNode->units );
    }
  
    fprintSegment( f, indent +  INDENT, "</xsd:appinfo>\n", 0 );
    fprintDocumentation( f, indent + INDENT, smiNode->description );
    fprintSegment( f, indent, "</xsd:annotation>\n", 0 );
    
}

static int hasChildren( SmiNode *smiNode, SmiNodekind nodekind )
{
    SmiNode *iterNode;
    int childNodeCount = 0; 

    for( iterNode = smiGetFirstChildNode( smiNode );
         iterNode;
         iterNode = smiGetNextChildNode( iterNode ) ){
        if( nodekind & iterNode->nodekind ) {
            childNodeCount++;
        }
    }
    return childNodeCount;
}

static void fprintIndexAttr( FILE *f, int indent, SmiNode *smiNode,
                             SmiNode *augments )
{
    char *typeName, *prefix;
    SmiType *smiType;

    smiType = smiGetNodeType( smiNode );
    if( !smiType ) {
/*        fprint( f, "<!-- error: no type in %s -->\n", smiNode->name );*/
        return;
    }
    
    typeName = smiType->name ?
        smiType->name :
        getStringBasetype( smiType->basetype );
    prefix = getTypePrefix( typeName );
    
    fprintSegment( f, indent, "<xsd:attribute", 0 );
    fprintf( f, " name=\"%s\"", smiNode->name );
    if( prefix ) {
        fprintf( f, " type=\"%s:%s\">\n", prefix, typeName );
    }
    else {
        fprintf( f, " type=\"%s\">\n", typeName );
    }
        
    if( augments ) {
        fprintSegment( f, indent + INDENT, "<xsd:annotation>\n", 0 );
        fprintSegment( f, indent + 2 * INDENT, "<xsd:appinfo>\n", 0 );
        fprintSegment( f, indent + 3 * INDENT, "<augments>", 0 );
        fprintf( f, "%s</augments>\n", augments->name );
        fprintSegment( f, indent + 2 * INDENT, "</xsd:appinfo>\n", 0 );
        fprintSegment( f, indent + INDENT, "</xsd:annotation>\n", 0 );
    }
    else {
        fprintAnnotationElem( f, indent + INDENT, smiNode );
    }
    fprintSegment( f, indent, "</xsd:attribute>\n", 0 ); 
}



static void fprintIndex( FILE *f, int indent, SmiNode *smiNode,
                         SmiNode *augments )
{
    SmiNode *iterNode;
    SmiElement *iterElem;
  
    /* iterate INDEX columns */
    for( iterElem = smiGetFirstElement( smiNode );
         iterElem;
         iterElem = smiGetNextElement( iterElem ) ) {
        iterNode = smiGetElementNode( iterElem );
        fprintIndexAttr( f, indent, iterNode, augments );
    }

    /* print AUGMENTS-clause */
    iterNode = smiGetRelatedNode( smiNode );
    if( iterNode ) {
        fprintIndex( f, indent, iterNode, iterNode );
    }
}


static void fprintComplexType( FILE *f, int indent,
                               SmiNode *smiNode, const char *name )
{
    SmiNode *iterNode;
    int numChildren;
    
    fprintSegment( f, indent, "<xsd:complexType", 0 );
    if( name ) {
        fprintf( f, " name=\"%sType\">\n", smiNode->name );
    }
    else {
        fprintf( f, ">\n" );
    }

    numChildren = hasChildren( smiNode, SMI_NODEKIND_ANY );
    if( numChildren ) {
        fprintSegment( f, indent + INDENT, "<xsd:sequence>\n", 0 );
    }
    /* print child elements */
    for( iterNode = smiGetFirstChildNode( smiNode );
         iterNode;
         iterNode = smiGetNextChildNode( iterNode ) ) {
        
        fprintElement( f, indent + 2 * INDENT, iterNode );
        
    }
    if( numChildren ) {
        fprintSegment( f, indent + INDENT, "</xsd:sequence>\n", 0 );
    }
    fprintIndex( f, indent + INDENT, smiNode, NULL );
    
    fprintSegment( f, indent, "</xsd:complexType>\n", 0 );
    if( name ) {
        /* we are printing out a global type,
           so let's leave a blank line after it. */
        fprintf( f, "\n" );
    }

    for( iterNode = smiGetFirstChildNode( smiNode );
         iterNode;
         iterNode = smiGetNextChildNode( iterNode ) ) {
        if( iterNode->nodekind == SMI_NODEKIND_NODE ) {
            fprintComplexType( f, indent, iterNode, iterNode->name );
        }
    }
}

static char
*getParentDisplayHint( SmiType *smiType )
{
    SmiType *iterType;

    for( iterType = smiGetParentType( smiType );
         iterType;
         iterType = smiGetParentType( iterType ) ) {
        if( iterType->format ) {
            return iterType->format;
        }        
    }
    return NULL;
}
    



static void fprintElement( FILE *f, int indent, SmiNode *smiNode ) /*  */
{
    switch( smiNode->nodekind ) {
        SmiType *smiType;

    case SMI_NODEKIND_NODE :
        {
            SmiNode *iterNode;
            int moreIndent;
            int numScalars = hasChildren( smiNode, SMI_NODEKIND_SCALAR );

            moreIndent = numScalars > 1 ? INDENT : 0;
            fprintSegment( f, indent, "<xsd:element", 0 );
            fprintf( f, " name=\"%s\">\n", smiNode->name );
            fprintSegment( f, indent + INDENT, "<xsd:complexType>\n", 0 );
            fprintSegment( f, indent + 2 * INDENT, "<xsd:sequence>\n", 0 );
            for( iterNode = smiGetFirstChildNode( smiNode );
                 iterNode;
                 iterNode = smiGetNextChildNode( iterNode ) ) {
                if( iterNode->nodekind == SMI_NODEKIND_SCALAR ) {
                    fprintElement( f, indent + moreIndent + 2 * INDENT,
                                   iterNode );
                }
            }
            fprintSegment( f, indent + 2 * INDENT, "</xsd:sequence>\n", 0 );
            fprintSegment( f, indent + INDENT, "</xsd:complexType>\n", 0 );
            fprintSegment( f, indent, "</xsd:element>\n", 0 );
        }
        break;
        
    case SMI_NODEKIND_TABLE :
    {
        SmiNode *iterNode;
        
        /* ignore tables and just include their entries */
        for( iterNode = smiGetFirstChildNode( smiNode );
             iterNode;
             iterNode = smiGetNextChildNode( iterNode ) ) {
            fprintElement( f, indent, iterNode );
        }
        break;
    }

    case SMI_NODEKIND_ROW :
        fprintSegment( f, indent, "<xsd:element", 0 );
        fprintf( f, " name=\"%s\"",  smiNode->name );
        fprintf( f, " minOccurs=\"0\" maxOccurs=\"unbounded\">\n" );

        fprintAnnotationElem( f, indent + INDENT, smiNode );

        fprintComplexType( f, indent + INDENT, smiNode, NULL );
        fprintSegment( f, indent, "</xsd:element>\n", 0 );
        break;
        
    case SMI_NODEKIND_SCALAR :
    case SMI_NODEKIND_COLUMN:
    {
        SmiElement *iterElem;
        char *prefix;
        char *typeName;
        
        /* check if we are index column */
        for( iterElem = smiGetFirstElement( smiGetParentNode( smiNode ) ) ;
             iterElem;
             iterElem = smiGetNextElement( iterElem ) ) {
            if( smiNode == smiGetElementNode( iterElem ) ) {
                /* we are index coulumn ==> do not print element */
                return;
            }
        }
        
        if( smiNode->access < SMI_ACCESS_READ_ONLY ) {
            /* only print accessible nodes */
            return;
        }
        
        smiType = smiGetNodeType( smiNode );

        fprintSegment( f, indent, "<xsd:element", 0 );
        fprintf( f, " name=\"%s\"", smiNode->name );

        if( smiType->name ) {
            typeName = smiType->name;
        }
        else {
            typeName = getStringBasetype( smiType->basetype );
        }
        prefix = getTypePrefix( typeName );

        if( smiType->basetype == SMI_BASETYPE_BITS ) {
            fprintf( f, " type=\"%s%s\"",
                    smiNode->name,
                    getStringBasetype( smiType->basetype ) );
            fprintf( f, " minOccurs=\"0\">\n" );
            fprintAnnotationElem( f, indent + INDENT, smiNode );
        }
        
        else if( smiType->basetype == SMI_BASETYPE_ENUM &&
            ! smiType->name ) {
            fprintf( f, " minOccurs=\"0\">\n" );
            fprintAnnotationElem( f, indent + INDENT, smiNode );
            fprintTypedef( f, indent + INDENT, smiType, NULL );
        }

        else if( smiType->basetype == SMI_BASETYPE_OCTETSTRING ) {            
            
            if( smiType->decl == SMI_DECL_IMPLICIT_TYPE ) {

                char *hint = getParentDisplayHint( smiType );

                fprintf( f, " minOccurs=\"0\">\n" );
                if( ! hint ) {
                    fprintAnnotationElem( f, indent + INDENT, smiNode );
                    fprintTypedef( f, indent + INDENT, smiType, NULL );
                }
                else {
                    unsigned int numSubRanges = getNumSubRanges( smiType ),
                        lp = 0;
                    SmiRange *smiRange;
                    SmiUnsigned32 *lengths = xmalloc(  2 * numSubRanges * 4 );

                    fprintAnnotationElem( f, indent + INDENT, smiNode );
                    
                    /* write subtype lengths to the array */
                    for( smiRange = smiGetFirstRange( smiType );
                         smiRange;
                         smiRange = smiGetNextRange( smiRange ) ) {
                        lengths[ lp++ ] = smiRange->minValue.value.unsigned32;
                        lengths[ lp++ ] = smiRange->maxValue.value.unsigned32;
                    }                    
                    fprintSegment( f, indent + INDENT,
                                   "<xsd:simpleType>\n", 0 );
                    fprintSegment( f, indent + 2 * INDENT,
                                   "<xsd:restriction base=\"xsd:string\">\n",
                                   0 );
                    fprintSegment( f, indent + 3* INDENT, "<xsd:pattern " , 0);
                    fprintf( f, "value=\"%s\"/>\n",
                             getStrDHType( hint, lengths, numSubRanges ) );
                    fprintSegment( f, indent + 2 * INDENT,
                                   "</xsd:restriction>\n", 0 );
                    fprintSegment( f, indent + INDENT,
                                   "</xsd:simpleType>\n", 0 );
                    xfree( lengths );
                }
            }

            else {
                if( prefix ) {
                    fprintf( f, " type=\"%s:%s\" minOccurs=\"0\">\n",
                             prefix, typeName );
                }
                else {                    
                    fprintf( f, " type=\"%s\" minOccurs=\"0\">\n",
                             typeName );
                }
                fprintAnnotationElem( f, indent + INDENT, smiNode );
            }                      
#if 0
            fprintSegment( f, indent + INDENT, "<xsd:complexType>\n", 0 );
            fprintSegment( f, indent + 2 * INDENT,
                           "<xsd:simpleContent>\n", 0 );
            fprintSegment( f, indent + 3 * INDENT, "<xsd:extension", 0 );
            if( smiType->decl == SMI_DECL_IMPLICIT_TYPE  ) {
                fprintf( f, " base=\"%sType\">\n",
                         smiNode->name ); 
            }
            else {
                
                if( prefix ) {
                    fprintf( f, " base=\"%s:%s\">\n", prefix, typeName );
                }
                else {
                    fprintf( f, " base=\"%s\">\n", typeName );
                }
            }
            
            fprintSegment( f, indent + 4 * INDENT,
                           "<xsd:attribute name=\"enc\" " , 0 );
            fprintf( f, "type=\"smi:EncAttrType\"/>\n" );
            fprintSegment( f, indent + 3 * INDENT, "</xsd:extension>\n", 0 );
            
            fprintSegment(f, indent + 2 * INDENT, "</xsd:simpleContent>\n", 0);
            fprintSegment( f, indent + INDENT, "</xsd:complexType>\n", 0 );
#endif /* 0 */
        }

        else if( smiType->decl == SMI_DECL_IMPLICIT_TYPE ) {
            fprintf( f, " minOccurs=\"0\">\n" );
            fprintAnnotationElem( f, indent + INDENT, smiNode );
            fprintTypedef( f, indent + INDENT, smiType, NULL );
        }
        
        else {
            if( prefix ) {
                fprintf( f, " type=\"%s:%s\"", prefix, typeName );
            }
            else {
                fprintf( f, " type=\"%s\"", typeName );
            }
            fprintf( f, " minOccurs=\"0\">\n" );
            fprintAnnotationElem( f, indent + INDENT, smiNode );
        }
        fprintSegment( f, indent, "</xsd:element>\n", 0 ); 
        break;
    }

    case SMI_NODEKIND_NOTIFICATION:
        fprintSegment( f, indent, "<xsd:element ", 0 );
        fprintf( f, "name=\"%s\"/>\n", smiNode->name );
        break;

    default:
        fprintf( f, "<!-- Warning! Unhandled Element! No details available!\n" );
        fprintf( f, "      Nodekind: %#4x -->\n\n", smiNode->nodekind );
        
    }
}


static void fprintRows( FILE *f, SmiModule *smiModule )
{
    SmiNode *iterNode;

    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_ROW );
         iterNode;
         iterNode = smiGetNextNode( iterNode,  SMI_NODEKIND_ROW ) ) {
        if( hasChildren( iterNode, SMI_NODEKIND_COLUMN | SMI_NODEKIND_TABLE ) ){
            fprintElement( f, 5 * INDENT, iterNode );
        }
    }
}


static void fprintBitsAndStrings( FILE *f, SmiModule *smiModule )
{
    SmiNode *iterNode;
    SmiType *smiType;

    for(iterNode = smiGetFirstNode(smiModule,
                                   SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN);
        iterNode;
        iterNode = smiGetNextNode(iterNode,
                                  SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN)) {
        smiType = smiGetNodeType( iterNode );
        if( smiType ) {
                switch( smiType->basetype ) {
                    
                case SMI_BASETYPE_BITS :
                    if( ! getTypePrefix( smiType->name ) ) {
                        fprintTypedef( f, INDENT, smiType, iterNode->name );
                        break;
                    }

                case SMI_BASETYPE_OCTETSTRING:
#if 0
                    if( smiType->decl == SMI_DECL_IMPLICIT_TYPE ) {
                        fprintTypedef( f, INDENT, smiType, iterNode->name );
                    }
#endif /* 0 */
                    break;
                default:
                    break;
                }
        }
    }
}


static void fprintKeys( FILE *f, int indent, SmiModule *smiModule )
{
    SmiNode *iterNode, *relNode;
    SmiElement *iterElem;
        
    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_ROW );
         iterNode;
         iterNode = smiGetNextNode( iterNode, SMI_NODEKIND_ROW ) ) {

        switch( iterNode->indexkind ) {

        case SMI_INDEX_INDEX:

            /* print key */
            fprintSegment( f, indent, "<xsd:key " , 0 );
            fprintf( f, "name=\"%sKey\">\n", iterNode->name );
            fprintSegment( f, 2 * indent, "<xsd:selector ", 0 );
            fprintf( f, "xpath=\"%s\"/>\n", iterNode->name );
            for( iterElem = smiGetFirstElement( iterNode );
                 iterElem;
                 iterElem = smiGetNextElement( iterElem ) ) {
                SmiNode *indexNode = smiGetElementNode( iterElem );
                fprintSegment( f, 2 * indent, "<xsd:field ", 0 );
                fprintf( f, "xpath=\"@%s\"/>\n", indexNode->name );
            }
            fprintSegment( f, indent, "</xsd:key>\n\n", 0 );
            break;

        case SMI_INDEX_AUGMENT:

            /* print keyref */
            fprintSegment( f, indent, "<xsd:keyref " , 0 );
            relNode = smiGetRelatedNode( iterNode );
            fprintf( f, "name=\"%sKeyRef\" ", iterNode->name );
            fprintf( f, "refer=\"%sKey\">\n", relNode->name );
            fprintSegment( f, 2 * indent, "<xsd:selector ", 0 );
            fprintf( f, "xpath=\"%s\"/>\n", iterNode->name );
            for( iterElem = smiGetFirstElement( relNode );
                 iterElem;
                 iterElem = smiGetNextElement( iterElem ) ) {
                SmiNode *indexNode = smiGetElementNode( iterElem );
                fprintSegment( f, 2 * INDENT, "<xsd:field ", 0 );
                fprintf( f, "xpath=\"@%s\"/>\n", indexNode->name );
            }
            fprintSegment( f, indent, "</xsd:keyref>\n", 0 );

            /* print unique clause */
            fprintSegment( f, indent, "<xsd:unique " , 0 );
            fprintf( f, "name=\"%sKeyRefUnique\">\n", iterNode->name );
            fprintSegment( f, 2 * indent, "<xsd:selector ", 0 );
            fprintf( f, "xpath=\"%s\"/>\n", iterNode->name );
            for( iterElem = smiGetFirstElement( relNode );
                 iterElem;
                 iterElem = smiGetNextElement( iterElem ) ) {
                SmiNode *indexNode = smiGetElementNode( iterElem );
                fprintSegment( f, 2 * indent, "<xsd:field ", 0 );
                fprintf( f, "xpath=\"@%s\"/>\n", indexNode->name );
            }
            fprintSegment( f, indent, "</xsd:unique>\n\n", 0 );
            break;

        case SMI_INDEX_REORDER:
        case SMI_INDEX_SPARSE:
        case SMI_INDEX_EXPAND:
            /* SMIng, not implemented yet */
            break;

        default:
            fprintf( f, "<!-- Error: Unknown index type -->\n" );
            break;
        }
    }
}


static void fprintImports( FILE *f, int indent, SmiModule *smiModule )
{
    SmiImport *iterImp;
    char *lastModName = "lastModName";
    
    for( iterImp = smiGetFirstImport( smiModule );
         iterImp;
         iterImp = smiGetNextImport( iterImp ) ) {
        /* assume imports to be ordered by module names */
        if( strcmp( iterImp->module, lastModName ) ) {
            fprintSegment( f, indent, "<xsd:import ", 0 );
            fprintf( f, "namespace=\"%s%s\" schemaLocation=\"%s%s.xsd\"/>\n",
                    schemaLocation, iterImp->module,
                    schemaLocation, iterImp->module );
        }
        lastModName = iterImp->module;
    }
    fprintSegment( f, indent, "<xsd:import namespace=\"http://www.ibr.cs.tu-bs.de/~tklie/smi\" schemaLocation=\"http://www.ibr.cs.tu-bs.de/~tklie/smi.xsd\"/>\n\n", 0 );
}


static void fprintNodes( FILE *f, SmiModule *smiModule )
{
    SmiNode *iterNode;
    
    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_NODE );
         iterNode;
         iterNode = smiGetNextNode( iterNode, SMI_NODEKIND_NODE ) ) {
        if( hasChildren( iterNode, SMI_NODEKIND_SCALAR ) ) {
            fprintElement( f, 5 * INDENT, iterNode );
        }
    }   
}

static void fprintNotifications( FILE *f, SmiModule *smiModule )
{
    SmiNode *iterNode;
    
    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_NOTIFICATION );
         iterNode;
         iterNode = smiGetNextNode( iterNode, SMI_NODEKIND_NOTIFICATION ) ) {
        fprintElement( f, 5 * INDENT, iterNode );
    }
}


static void fprintModule(FILE *f, SmiModule *smiModule)
{
    if( smiModule->description ) {
        fprintSegment(f, INDENT, "<xsd:annotation>\n", 0);
        fprintDocumentation(f, 2 * INDENT, smiModule->description);
        fprintSegment(f, INDENT, "</xsd:annotation>\n\n", 0);
    }
    
    fprintImports(f, INDENT, smiModule);

    /* print root node */
    fprintSegment( f, INDENT, "<xsd:element ", 0 );
    fprintf( f, "name=\"%s\">\n", smiModule->name );
    fprintSegment( f, 2 * INDENT, "<xsd:complexType>\n", 0 );
    fprintSegment( f, 3 *INDENT, "<xsd:choice>\n", 0 );
    fprintSegment( f, 4 * INDENT, "<xsd:sequence>\n", 0 );
    fprintNodes( f, smiModule );
    fprintRows( f, smiModule );
    fprintSegment( f, 4 * INDENT, "</xsd:sequence>\n", 0 );
    fprintSegment( f, 4 * INDENT, "<xsd:sequence>\n", 0 );
    fprintNotifications( f, smiModule );
    fprintSegment( f, 4 * INDENT, "</xsd:sequence>\n", 0 );
    fprintSegment( f, 3 * INDENT, "</xsd:choice>\n", 0 );
    fprintSegment( f, 3 * INDENT,
                   "<xsd:attribute name=\"agent\" type=\"xsd:NMTOKEN\" "
                   "use=\"required\"/>\n",0 );
    fprintSegment( f, 3 * INDENT,
                   "<xsd:attribute name=\"community\" type=\"xsd:NMTOKEN\" "
                   "use=\"required\"/>\n",0 );
    fprintSegment( f, 3 * INDENT,
                   "<xsd:attribute name=\"port\" "
                   "type=\"xsd:unsignedInt\" use=\"required\"/>\n", 0 );
    fprintSegment( f, 3 * INDENT,
                   "<xsd:attribute name=\"time\" "
                   "type=\"xsd:dateTime\" use=\"required\"/>\n", 0 );
    fprintSegment( f, 2 * INDENT, "</xsd:complexType>\n", 0 );

    fprintKeys( f, 2 * INDENT, smiModule );
    
    fprintSegment( f, INDENT, "</xsd:element>\n\n", 0 );

    fprintBitsAndStrings( f, smiModule );
}


static void fprintTypedefs(FILE *f, SmiModule *smiModule)
{
    int                 i;
    SmiType         *smiType;
    
    for(i = 0, smiType = smiGetFirstType(smiModule);
        smiType;
        i++, smiType = smiGetNextType(smiType)) {
        fprintf(f, "\n");
        fprintTypedef(f, INDENT, smiType, smiType->name);
    }
}

static void registerType( char *type, char *module )
{
    TypePrefix *oldTPr = NULL, *iterTPr = NULL;

    for( iterTPr = typePrefixes; iterTPr; iterTPr = iterTPr->next ) {
        oldTPr = iterTPr;
    }
    if( ! oldTPr ) {
        /* type prefixes do not exist yet */
        typePrefixes = xmalloc( sizeof( TypePrefix ) );
        typePrefixes->type = type;
        typePrefixes->prefix = module;
        typePrefixes->next = NULL;
    }
    else {
        /* create new TypePrefix */
        oldTPr->next = xmalloc( sizeof( TypePrefix ) );
        oldTPr->next->type = type;
        oldTPr->next->prefix = module;
        oldTPr->next->next = NULL;
    }
}


static void fprintSchemaDef( FILE *f, SmiModule *smiModule )
{
    SmiImport *iterImp;
    char *lastModName = "lastModName";
    /* There is no mib called "lastModName", is there?
       So let's use this to initialize variable. */
    
    fprintf(f,
           "<xsd:schema targetNamespace=\"%s%s\"\n",
           schemaLocation, smiModule->name);
    
    fprintf(f, "            xmlns=\"%s%s\"\n",
           schemaLocation, smiModule->name);
/*    fprintf(f, "            xmlns:xmn=\"http://www.w3.org/XML/1998/namespace\"\n"); */
    fprintf(f, "            xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n");
    fprintf(f, "            xmlns:smi=\"http://www.ibr.cs.tu-bs.de/~tklie/smi\"\n");
    
    for( iterImp = smiGetFirstImport( smiModule );
         iterImp;
         iterImp = smiGetNextImport( iterImp ) ) {
        registerType( iterImp->name, iterImp->module );
        /* assume imports to be ordered by module names */
        if( strcmp( iterImp->module, lastModName ) ) {
            fprintf( f, "            xmlns:%s=\"%s%s\"\n",
                    iterImp->module, schemaLocation, iterImp->module );
        }
        lastModName = iterImp->module;
    }
  
    fprintf(f, "            xml:lang=\"en\"\n");
    fprintf(f, "            elementFormDefault=\"qualified\"\n");
    fprintf(f, "            attributeFormDefault=\"unqualified\">\n\n");
}

static void dumpXsd(int modc, SmiModule **modv, int flags, char *output)
{
    int  i;
    FILE *f = stdout;

    if (output) {
        f = fopen(output, "w");
        if (!f) {
            fprintf(stderr, "smidump: cannot open %s for writing: ", output);
            perror(NULL);
            exit(1);
        }
    }

    for (i = 0; i < modc; i++) {

        /* register smi basetypes */
        registerType( "Integer32","smi" );
        registerType( "ObjectIdentifier", "smi" );
        registerType( "OctetString", "smi" );
        registerType( "Unsigned32", "smi" );
        registerType( "Unsigned64", "smi" );

        /* make sure url ends with '/' */
        if( schemaLocation[ strlen( schemaLocation ) - 1 ] != '/' ) {
            strcat( schemaLocation, "/" );
        }
        
        fprintf(f, "<?xml version=\"1.0\"?>\n"); 
        fprintf(f, "<!-- This module has been generated by smidump "
               SMI_VERSION_STRING ". Do not edit. -->\n");
        fprintf(f, "\n");

        fprintSchemaDef(f, modv[i]);        

        fprintModule(f, modv[i]);
        fprintTypedefs(f, modv[i]);
        
        fprintf(f, "\n</xsd:schema>\n");

        /* delete type-prefix-mapping */
        free( typePrefixes );
    }

    if (fflush(f) || ferror(f)) {
        perror("smidump: write error");
        exit(1);
    }

    if (output) {
        fclose(f);
    }
}



void initXsd()
{

    static SmidumpDriverOption opt[] = {
        { "schema-url", OPT_STRING, &schemaLocation, 0,
          "The URL where the imported MIB modules reside" },
        { 0, OPT_END, 0, 0 }
    };
    
    static SmidumpDriver driver = {
        "xsd",
        dumpXsd,
        0,
        SMIDUMP_DRIVER_CANT_UNITE,
        "XML schema definitions",
        opt,
        NULL
    };
    
    smidumpRegisterDriver(&driver);
}
