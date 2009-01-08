/*
 * error.c --
 *
 *      Error handling routines.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: error.c,v 1.101 2002/10/24 10:40:10 strauss Exp $
 */

#include <config.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(HAVE_WIN_H)
#include "win.h"
#endif

#include "smi.h"
#include "error.h"
#include "util.h"

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif



/*
 * Structure to hold error messages with their associated error level.
 * Note that it is possible to modify the error levels at run time.
 */

typedef struct Error {
    int level;          /* 0: fatal, no way to continue          */
                        /* 1: severe, changing semantics to continue */
                /*    must be corrected                      */
                /* 2: error, but able to continue,           */
                /*    should be corrected                    */
                /* 3: minor error, but should be corrected   */
                /* 4: change is recommended if possible      */
                /* 5: warning, but might totally correct     */
                /*    under some circumstances               */
                /* 6: just a notice                          */
    int id;         /* error id, used in error() invocation      */
    char *tag;          /* tag for error identification on cmd line  */
    char *fmt;          /* the complete error format string      */
} Error;



/*
 * Note: The Makefile produces a list of error macros for every `ERR...'
 * pattern in this file (error.c). This list is written to errormacros.h.
 */

static Error errors[] = {
    { 0, ERR_INTERNAL, "internal", 
      "internal error!!!" },
    { 0, ERR_MAX_LEX_DEPTH, "", 
      "maximum IMPORTS nesting, probably a loop?" },
    { 1, ERR_LEX_UNEXPECTED_CHAR, "lexical", 
      "lexically unexpected character, skipping to end of line" },
    { 0, ERR_OUT_OF_MEMORY, "memory", 
      "out of memory (internal error!)" },
    { 1, ERR_OTHER_ERROR, "other", 
      "%s" },
    { 1, ERR_ILLEGAL_KEYWORD, "keyword-illegal", 
      "illegal keyword `%s'" },
    { 2, ERR_ID_ENDS_IN_HYPHEN, "hyphen-end",
      "identifier `%s' illegally ends in a hyphen" },
    { 3, ERR_LEADING_ZEROS, "number-leading-zero",
      "leading zero(s) on a number" },
    { 2, ERR_NUMBER_TOO_LARGE, "number-range",
      "number `%s' too large" },
    { 2, ERR_HEX_ENDS_IN_B, "string-delimiter",
      "hexadecimal string terminated by binary string delimiter, assume hex value" },
    { 2, ERR_MODULENAME_64, "namelength-64-module",
      "module name `%s' must not be longer that 64 characters" },
    { 4, ERR_MODULENAME_32, "namelength-32-module",
      "module name `%s' longer than 32 characters" },
    { 2, ERR_TYPENAME_64, "namelength-64-type",
      "type name `%s' must not be longer that 64 characters" },
    { 4, ERR_TYPENAME_32, "namelength-32-type",
      "type name `%s' longer than 32 characters" },
    { 2, ERR_OIDNAME_64, "namelength-64-object",
      "object identifier name `%s' must not be longer that 64 characters" },
    { 4, ERR_OIDNAME_32, "namelength-32-object",
      "object identifier name `%s' longer than 32 characters" },
    { 2, ERR_ENUMNAME_64, "namelength-64-enumeration",
      "enumeration name `%s' must not be longer that 64 characters" },
    { 4, ERR_ENUMNAME_32, "namelength-32-enumeration",
      "enumeration name `%s' longer than 32 characters" },
    { 2, ERR_BITNAME_64, "namelength-64-bit",
      "bit name `%s' must not be longer than 64 characters" },
    { 4, ERR_BITNAME_32, "namelength-32-bit",
      "bit name `%s' longer than 32 characters" },
    { 2, ERR_UCIDENTIFIER_64, "namelength-64-uc-identifier",
      "uppercase identifier `%s' must not be longer than 64 characters" },
    { 2, ERR_LCIDENTIFIER_64, "namelength-64-lc-identifier",
      "lowercase identifier `%s' must not be longer than 64 characters" },
    { 2, ERR_TRAP_TYPE, "trap",
      "TRAP-TYPE macro is not allowed in SMIv2" },
    { 2, ERR_TOO_MANY_MODULE_IDENTITIES, "module-identity-multiple",
      "more than one MODULE-IDENTITY clause in SMIv2 MIB" },
    { 2, ERR_NO_MODULE_IDENTITY, "module-identity-missing",
      "missing MODULE-IDENTITY clause in SMIv2 MIB" },
    { 2, ERR_OID_DEFVAL_TOO_LONG_SMIV2, "default-too-long-smiv2",
      "object identifier default values must be expressed as a single identifier" },
    { 4, ERR_OID_DEFVAL_TOO_LONG_SMIV1, "default-too-long-smiv1",
      "object identifier default values in form of sub-identifier sequences are not implemented" },
    { 2, ERR_INVALID_SMIV1_ACCESS, "access-invalid-smiv1", 
      "invalid access `%s' in SMIv1 MIB" },
    { 2, ERR_INVALID_SMIV2_ACCESS, "access-invalid-smiv2",
      "invalid access `%s' in SMIv2 MIB" },
    { 2, ERR_SMIV2_WRITE_ONLY, "access-write-only-smiv2", 
      "access `write-only' is no longer allowed in SMIv2" },
    { 4, ERR_SMIV1_WRITE_ONLY, "access-write-only-smiv1", 
      "access `write-only' is not a good idea" },
    { 2, ERR_INVALID_NOTIFICATION_VARIATION_ACCESS, "variation-access-notification", 
      "invalid access `%s' in a notification variation" },
    { 2, ERR_INVALID_OBJECT_VARIATION_ACCESS, "variation-access-object", 
      "invalid access `%s' in an object variation" },
    { 2, ERR_INVALID_VARIATION_ACCESS, "variation-access", 
      "invalid access `%s' in a variation" },
    { 2, ERR_NOTIFICATION_VARIATION_SYNTAX, "variation-syntax", 
      "SYNTAX is not allowed in a notification variation" },
    { 2, ERR_NOTIFICATION_VARIATION_WRITESYNTAX, "variation-writesyntax", 
      "WRITE-SYNTAX is not allowed in a notification variation" },
    { 2, ERR_DEFVAL_SYNTAX, "defval-syntax", 
      "default value syntax does not match object syntax" },
    { 2, ERR_NOTIFICATION_VARIATION_DEFVAL, "variation-defval", 
      "default value is not allowed in a notification variation" },
    { 2, ERR_NOTIFICATION_VARIATION_CREATION, "variation-creation", 
      "CREATION-REQUIRES is not allowed in a notification variation" },
    { 3, ERR_MODULE_IDENTITY_NOT_FIRST, "module-identity-not-first", 
      "MODULE-IDENTITY clause must be the first declaration in a module" },
    { 2, ERR_INVALID_SMIV1_STATUS, "status-invalid-smiv1", 
      "invalid status `%s' in SMIv1 MIB" },
    { 2, ERR_INVALID_SMIV2_STATUS, "status-invalid-smiv2", 
      "invalid status `%s' in SMIv2 MIB" },
    { 2, ERR_INVALID_CAPABILITIES_STATUS, "status-invalid-capabilities", 
      "invalid status `%s' in AGENT-CAPABILITIES macro" },
    { 5, ERR_OIDNAME_INCLUDES_HYPHEN, "hyphen-in-object",
      "object identifier name `%s' should not include hyphens in SMIv2 MIB" },
    { 2, ERR_ILLEGAL_CHAR_IN_STRING, "char-illegal-string", 
      "illegal character `%c' (0x%2x) in quoted string" },
    { 2, ERR_BIN_STRING_MUL8, "", 
      "length of binary string `%s' is not a multiple of 8" },
    { 2, ERR_HEX_STRING_MUL2, "", 
      "length of hexadecimal string `%s' is not a multiple of 2" },
    { 5, ERR_FLUSH_DECLARATION, "", 
      "flushing recent incorrect declaration, see previous error(s)" },
    { 2, ERR_MAX_ACCESS_IN_SMIV1, "", 
      "MAX-ACCESS is SMIv2 style, use ACCESS in SMIv1 MIBs instead" },
    { 2, ERR_ACCESS_IN_SMIV2, "", 
      "ACCESS is SMIv1 style, use MAX-ACCESS in SMIv2 MIBs instead" },
    { 5, ERR_UNWANTED_MODULE, "", 
      "ignoring unwanted module `%s'" },
    { 1, ERR_MODULE_NOT_FOUND, "", 
      "failed to locate MIB module `%s'" },
    { 2, ERR_OBJECT_IDENTIFIER_REGISTERED, "", 
      "Object identifier label `%s.%s' already registered at `%s'" },
    { 1, ERR_OPENING_INPUTFILE, "", 
      "%s: %s" },
    { 1, ERR_ILLEGAL_INPUTFILE, "", 
      "%s: unable to determine SMI version" },
    { 1, ERR_UNKNOWN_OIDLABEL, "", 
      "unknown object identifier label `%s'" },
    { 2, ERR_SINGLE_SUBIDENTIFIER, "", 
      "single number `%s' is not a valid object identifier" },
    { 2, ERR_SUBIDENTIFIER_VS_OIDLABEL, "", 
      "subidentifier `%s' does not match object identifier label `%s'" },
    { 2, ERR_EXISTENT_OBJECT, "", 
      "an object named `%s' already exists" },
    { 2, ERR_IDENTIFIER_NOT_IN_MODULE, "", 
      "identifier `%s' cannot be imported from module `%s'" },
    { 1, ERR_MACRO, "", 
      "MACRO definitions are only allowed in SMI base modules" },
    { 1, ERR_CHOICE, "", 
      "CHOICE type definitions are only allowed in SMI base modules" },
    { 1, ERR_TYPE_SMI, "", 
      "type `%s' may only be defined in SMI/SPPI base modules" },
    { 1, ERR_TYPE_TAG, "", 
      "tagged or IMPLICIT types may only be defined in SMI base modules" },
    { 1, ERR_MACRO_ALREADY_EXISTS, "", 
      "module `%s' already declared a macro `%s'" },
    { 1, ERR_EXPORTS, "", 
      "EXPORTS are only allowed in SMIv1 base modules" },
    { 1, ERR_ILLEGALLY_QUALIFIED, "", 
      "illegally qualified object identifier label `%s'" },
    { 2, ERR_MISSING_DESCRIPTION, "", 
      "missing DESCRIPTION on SMIv2 OBJECT-TYPE" },
    { 2, ERR_OIDLABEL_NOT_FIRST, "", 
      "Object identifier element `%s' name only allowed as first element" },
    { 2, ERR_UNKNOWN_TYPE, "", 
      "unknown type `%s'" },
    { 1, ERR_LOCATION, "", 
      "opening MIB directory or file `%s': %s" },
    { 1, ERR_UNKNOWN_LOCATION_TYPE, "", 
      "unknown MIB location type `%s'" },
    { 2, ERR_ILLEGAL_RANGE_FOR_COUNTER, "counter-range-illegal", 
      "illegal range restriction for counter type `%s'" },
    { 2, ERR_ILLEGAL_RANGE_FOR_PARENT_TYPE, "range-illegal", 
      "illegal range restriction for non-numerical parent type `%s'" },
    { 2, ERR_ILLEGAL_SIZE_FOR_PARENT_TYPE, "size-illegal", 
      "illegal size restriction for non-octet-string parent type `%s'" },
    { 2, ERR_ILLEGAL_ENUM_FOR_PARENT_TYPE, "enum-illegal", 
      "illegal enumeration or bits restriction for non-enumeration-or-bits parent type `%s'" },
    { 1, ERR_SMIV2_SIGNED_NUMBER_RANGE, "", 
      "number `%s' is out of SMIv1/SMIv2 signed number range" },
    { 1, ERR_SMIV2_UNSIGNED_NUMBER_RANGE, "", 
      "number `%s' is out of SMIv1/SMIv2 unsigned number range" },
    { 1, ERR_INTEGER32_TOO_LARGE, "", 
      "Integer32 value `%u' is too large" },
    { 1, ERR_UNEXPECTED_VALUETYPE, "", 
      "type of value does not match declaration" },
    { 1, ERR_SMI_NOT_SUPPORTED, "", 
      "file `%s' seems to be SMIv1 or SMIv2 which is not supported" },
    { 1, ERR_SMING_NOT_SUPPORTED, "", 
      "file `%s' seems to be SMIng which is not supported" },
    { 6, ERR_UNUSED_IMPORT, "import-unused", 
      "identifier `%s' imported from module `%s' is never used" },
    { 2, ERR_MACRO_NOT_IMPORTED, "", 
      "macro `%s' has not been imported from module `%s'" },
    { 6, ERR_IMPLICIT_NODE, "", 
      "implicit node definition" },
    { 3, ERR_SCALAR_READCREATE, "", 
      "scalar object must not have a `read-create' access value" },
    { 4, ERR_NAMEDNUMBER_INCLUDES_HYPHEN, "hyphen-in-label", 
      "named number `%s' must not include a hyphen in SMIv2" },
    { 4, ERR_NAMEDBIT_INCLUDES_HYPHEN, "hyphen-in-label", 
      "named bit `%s' must not include a hyphen in SMIv2" },
    { 2, ERR_REDEFINITION, "", 
      "redefinition of identifier `%s'" },
    { 6, ERR_EXT_REDEFINITION, "", 
      "redefinition of identifier `%s::%s'" },
    { 5, ERR_CASE_REDEFINITION, "", 
      "identifier `%s' differs from `%s' only in case" },
    { 6, ERR_EXT_CASE_REDEFINITION, "", 
      "identifier `%s' differs from `%s::%s' only in case" },
    { 6, ERR_PREVIOUS_DEFINITION, "",
      "previous definition of `%s'" },
    { 2, ERR_INVALID_FORMAT, "invalid-format", 
      "invalid format specification `%s'" },
    { 3, ERR_REFINEMENT_ALREADY_EXISTS, "", 
      "refinement for `%s' already exists in this compliance statement" },
    { 3, ERR_OPTIONALGROUP_ALREADY_EXISTS, "", 
      "optional group definition for `%s' already exists in this compliance statement" },
    { 2, ERR_ILLEGAL_OID_DEFVAL, "", 
      "cannot handle other default values than 0.0 for `%s'" },
    { 2, ERR_UNEXPECTED_TYPE_RESTRICTION, "subtype-in-sequence", 
      "subtyping not allowed in SEQUENCE" },
    { 1, ERR_UNKNOWN_CONFIG_CMD, "", 
      "unknown configuration command `%s' in file `%s'" },
    { 6, ERR_CACHE_CONFIG_NOT_SUPPORTED, "", 
      "MIB caching is not supported though configured in file `%s'" },
    { 4, ERR_SMIV2_OPAQUE_OBSOLETE, "opaque-smiv2", 
      "SMIv2 provides Opaque solely for backward-compatibility" },
    { 4, ERR_SPPI_OPAQUE_OBSOLETE, "opaque-sppi", 
      "SPPI provides Opaque solely for backward-compatibility" },
    { 2, ERR_DATE_CHARACTER, "date-character", 
      "date specification `%s' contains an illegal character" },
    { 2, ERR_DATE_LENGTH, "date-length", 
      "date specification `%s' has an illegal length" },
    { 2, ERR_DATE_VALUE, "date-value", 
      "date specification `%s' contains an illegal value" },
    { 6, ERR_DATE_YEAR_2DIGITS, "date-year-2digits", 
      "date specification `%s' contains a two-digit year representing `%d'" },
    { 2, ERR_DATE_YEAR, "date-year", 
      "date specification `%s' contains an illegal year" },
    { 2, ERR_DATE_MONTH, "date-month", 
      "date specification `%s' contains an illegal month" },
    { 2, ERR_DATE_DAY, "date-day", 
      "date specification `%s' contains an illegal day" },
    { 2, ERR_DATE_HOUR, "date-hour", 
      "date specification `%s' contains an illegal hour" },
    { 2, ERR_DATE_MINUTES, "date-minutes", 
      "date specification `%s' contains illegal minutes" },
    { 4, ERR_DATE_IN_FUTURE, "date-in-future", 
      "date specification `%s' is in the future" },
    { 4, ERR_DATE_IN_PAST, "date-in-past", 
      "date specification `%s' predates the SMI standard" },
    { 6, ERR_INTEGER_IN_SMIV2, "integer-misuse", 
      "use Integer32 instead of INTEGER in SMIv2" },
    { 5, ERR_MODULE_ALREADY_LOADED, "", 
      "module `%s' is already loaded, aborting parser on this file" },
    { 2, ERR_SMIV2_BASETYPE_NOT_IMPORTED, "basetype-not-imported", 
      "SMIv2 base type `%s' must be imported from SNMPv2-SMI" },
    { 2, ERR_BASETYPE_UNKNOWN, "basetype-unknown", 
      "type `%s' of node `%s' does not resolve to a known base type" },
    { 2, ERR_ROW_SUBID_ONE, "row-node-subidentifier-one", 
      "subidentifier of row node `%s' must be 1" },
    { 6, ERR_ROWNAME_ENTRY, "row-name-entry", 
      "row identifier `%s' should match `*Entry'" },
    { 6, ERR_TABLENAME_TABLE, "table-name-table", 
      "table identifier `%s' should match `*Table'" },
    { 6, ERR_ROWNAME_TABLENAME, "row-name-table-name",
      "row identifier `%s' should have the same prefix as table identifier `%s'" },
    { 6, ERR_SMIV2_TYPE_ASSIGNEMENT, "type-assignement-smiv2", 
      "type assignement `%s' should be a textual convention in SMIv2" },
    { 6, ERR_ILLEGAL_IMPORT, "import-illegal", 
      "identifier `%s' may not be imported from module `%s'" },
    { 3, ERR_SMIV2_NESTED_TEXTUAL_CONVENTION, "textual-convention-nested", 
      "textual convention `%s' can not be derived from the textual convention `%s'" },
    { 3, ERR_TABLE_ACCESS, "access-table-illegal", 
      "table node `%s' must be `not-accessible'" },
    { 3, ERR_ROW_ACCESS, "access-row-illegal", 
      "row node `%s' must be `not-accessible'" },
    { 4, ERR_COUNTER_ACCESS, "access-counter-illegal", 
      "counter object `%s' must be `read-only' or `accessible-for-notify'" },
    { 2, ERR_ENUM_SUBTYPE, "subtype-enumeration-illegal", 
      "named number `%s(%d)' illegal in sub-type of `%s'" },
    { 2, ERR_BITS_SUBTYPE, "subtype-bits-illegal", 
      "named number `%s' illegal in sub-type of `%s'" },
    { 2, ERR_ENUM_NAME_REDEFINITION, "enum-name-redefinition", 
      "redefinition of name `%s' in number enumeration" },
    { 2, ERR_ENUM_NUMBER_REDEFINITION, "enum-number-redefinition", 
      "redefinition of number `%d' in number enumeration" },
    { 2, ERR_ENUM_ZERO, "enum-zero", 
      "number enumeration contains zero value in SMIv1 MIB" },
    { 2, ERR_BITS_NAME_REDEFINITION, "bits-name-redefinition", 
      "redefinition of name `%s' in named bits list" },
    { 2, ERR_BITS_NUMBER_REDEFINITION, "bits-number-redefinition", 
      "redefinition of number `%u' in named bits list" },
    { 2, ERR_BITS_NUMBER_TOO_LARGE, "bits-number-too-large", 
      "named bit `%s(%u)' exceeds maximum bit position" },
    { 4, ERR_BITS_NUMBER_LARGE, "bits-number-large", 
      "named bit `%s(%u)' may cause interoperability or implementation problems" },
    { 2, ERR_RANGE_OUT_OF_BASETYPE, "range-bounds",
      "range limit exceeds underlying basetype" },
    { 2, ERR_RANGE_OVERLAP, "range-overlap",
      "overlapping range limits" },
    { 6, ERR_RANGES_NOT_ASCENDING, "range-ascending",
      "ranges not in ascending order" },
    { 6, ERR_NAMED_NUMBERS_NOT_ASCENDING, "named-numbers-ascending",
      "named numbers not in ascending order" },
    { 2, ERR_EXCHANGED_RANGE_LIMITS, "range-exchanged",
      "range limits must be `lower-bound .. upper-bound'" },
    { 1, ERR_INDEX_BASETYPE, "index-illegal-basetype", 
      "illegal base type `%s' in index element `%s' of row %s" },
    { 5, ERR_INDEX_TOO_LARGE, "index-exceeds-too-large", 
      "index of row `%s' can exceed OID size limit by %d subidentifier(s)" },
    { 2, ERR_INDEX_NO_RANGE, "index-element-no-range",
      "index element `%s' of row `%s' must have a range restriction" },
    { 2, ERR_INDEX_NO_RANGE_MOD, "index-element-no-range",
      "index element `%s::%s' of row `%s' must have a range restriction" },
    { 1, ERR_INDEX_STRING_NO_SIZE, "index-element-no-size",
      "index element `%s' of row `%s' must have a size restriction" },
    { 1, ERR_INDEX_STRING_NO_SIZE_MOD, "index-element-no-size",
      "index element `%s::%s' of row `%s' must have a size restriction" },
    { 6, ERR_INDEX_OID_NO_SIZE, "index-element-no-size",
      "index element `%s' of row `%s' should but cannot have a size restriction" },
    { 6, ERR_INDEX_OID_NO_SIZE_MOD, "index-element-no-size",
      "index element `%s::%s' of row `%s' should but cannot have a size restriction" },
    { 1, ERR_INDEX_RANGE_NEGATIVE, "index-element-range-negative",
      "range restriction of index element `%s' of row `%s' must be non-negative" },
    { 1, ERR_INDEX_ENUM_NEGATIVE, "index-element-enum-negative",
      "enumerations of index element `%s' of row `%s' must be non-negative" },
    { 6, ERR_INDEX_NOT_COLUMN, "index-element-not-column",
      "index element `%s' of row `%s' must be a column" },
    { 2, ERR_AUGMENT_NESTED, "augment-nested",
      "row `%s' augments `%s' which is not a base table row" },
    { 2, ERR_AUGMENT_NO_ROW, "augment-no-row",
      "row `%s' augments or extends `%s' which is not a row" },
    { 4, ERR_NODE_NOT_IN_GROUP, "group-membership",
      "node `%s' must be contained in at least one conformance group" },
    { 4, ERR_NOTIFICATION_NOT_IN_GROUP, "group-membership",
      "notification `%s' must be contained in at least one conformance group" },
    { 3, ERR_INVALID_GROUP_MEMBER, "group-member-invalid",
      "node `%s' is an invalid member of group `%s'" },
    { 3, ERR_MIXED_GROUP_MEMBERS, "group-member-mixed",
      "group `%s' contains scalars/columns and notifications" },
    { 2, ERR_BAD_LOWER_IDENTIFIER_CASE, "bad-identifier-case",
      "`%s' should start with a lower case letter" },
    { 2, ERR_UNDERSCORE_IN_IDENTIFIER, "underscore-in-identifier",
      "identifier `%s' must not contain an underscore" },
    { 6, ERR_OID_REUSE, "oid-reuse",
      "identifier `%s' reuses object identifier assigned to identifier `%s'" },
    { 1, ERR_OID_REGISTERED, "oid-registered",
      "identifier `%s' registers object identifier already registered by `%s'" },
    { 1, ERR_OID_RECURSIVE, "oid-recursive",
      "object identifier `%s' defined recursively or too long" },
    { 6, ERR_COMMENT_TERMINATES, "comment-terminates",
      "note, -- terminates a comment" },
    { 6, ERR_OBSOLETE_IMPORT, "obsolete-import",
      "identifier `%s' should be imported from `%s' instead of `%s'" },
    { 4, ERR_GROUP_OBJECT_STATUS, "group-object-status",
      "%s group `%s' includes %s object `%s'" },
    { 4, ERR_COMPLIANCE_GROUP_STATUS, "compliance-group-status",
      "%s compliance statement `%s' includes %s group `%s'" },
    { 4, ERR_COMPLIANCE_OBJECT_STATUS, "compliance-object-status",
      "%s compliance statement `%s' includes %s object `%s'" },
    { 6, ERR_GROUP_UNREF, "group-unref",
      "%s group `%s' is not referenced in this module" },
    { 6, ERR_TYPE_UNREF, "type-unref",
      "%s type `%s' is not referenced in this module" },
    { 5, ERR_SEQUENCE_ORDER, "sequence-order", 
      "SEQUENCE element #%d `%s' does not match order of columnar objects under `%s'" },
    { 3, ERR_SEQUENCE_NO_COLUMN, "sequence-no-column", 
      "SEQUENCE element #%d `%s' is not a child node under `%s'" },
    { 3, ERR_SEQUENCE_MISSING_COLUMN, "sequence-missing-column", 
      "SEQUENCE of `%s' is missing columnar object `%s'" },
    { 4, ERR_ILLEGAL_ROWSTATUS_DEFAULT, "rowstatus-default", 
      "illegal `RowStatus' default value `%s'" },
    { 4, ERR_ILLEGAL_STORAGETYPE_DEFAULT, "storagetype-default", 
      "illegal `StorageType' default value `%s'" },
    { 2, ERR_DEFVAL_OUT_OF_BASETYPE, "defval-basetype", 
      "default value exceeds range of underlying basetype" },
    { 2, ERR_DEFVAL_OUT_OF_RANGE, "defval-range", 
      "default value does not match range restriction of underlying type" },
    { 2, ERR_DEFVAL_OUT_OF_ENUM, "defval-enum", 
      "default value does not match underlying enumeration type" },
    { 5, ERR_TADDRESS_WITHOUT_TDOMAIN, "taddress-tdomain", 
      "`TAddress' object should have an accompanied `TDomain' object" },
    { 5, ERR_INETADDRESS_WITHOUT_TYPE, "inetaddress-inetaddresstype", 
      "`InetAddress' object should have an accompanied preceding `InetAdressType' object" },
    { 5, ERR_INETADDRESSTYPE_SUBTYPED, "inetaddresstype-subtyped", 
      "`InetAddressType' should not be subtyped" },
    { 5, ERR_INETADDRESS_SPECIFIC, "inetaddress-specific", 
      "`InetAddress' should be used instead of `%s'" },
    { 3, ERR_NOTIFICATION_OBJECT_TYPE, "notification-object-type", 
      "object `%s' of notification `%s' must be a scalar or column" },
    { 3, ERR_NOTIFICATION_OBJECT_ACCESS, "notification-object-access", 
      "object `%s' of notification `%s' must not be `not-accessible'" },
    { 5, ERR_NOTIFICATION_OBJECT_MIX, "notification-object-mix", 
      "notification `%s' contains objects from multiple tables/groups" },
    { 6, ERR_EMPTY_DESCRIPTION, "empty-description",
      "zero-length description string" },
    { 6, ERR_EMPTY_REFERENCE, "empty-reference",
      "zero-length reference string" },
    { 6, ERR_EMPTY_ORGANIZATION, "empty-organization",
      "zero-length organization string" },
    { 6, ERR_EMPTY_CONTACT, "empty-contact",
      "zero-length contact string" },
    { 6, ERR_EMPTY_FORMAT, "empty-format",
      "zero-length format string" },
    { 6, ERR_EMPTY_UNITS, "empty-units",
      "zero-length units string" },
    { 6, ERR_INDEX_DEFVAL, "index-element-has-default",
      "index element `%s' of row `%s' has a default value" },
    { 2, ERR_SEQUENCE_TYPE_MISMATCH, "sequence-type-mismatch",
      "type of `%s' in sequence and object type definition do not match" },
    { 5, ERR_INDEX_ACCESSIBLE, "index-element-accessible",
      "index element `%s' of row `%s' should be not-accessible in SMIv2 MIB" },
    { 5, ERR_INDEX_NON_ACCESSIBLE, "index-element-not-accessible",
      "exactly one index element of row `%s' must be accessible in SMIv2 MIB" },
    { 3, ERR_REFINEMENT_NOT_LISTED, "refinement-not-listed",
      "refined object `%s' not listed in a mandatory or optional group" },
    { 5, ERR_NOTIFICATION_NOT_REVERSIBLE, "notification-not-reversible",
      "notification `%s' is not reverse mappable" },
    { 5, ERR_NOTIFICATION_ID_TOO_LARGE, "notification-id-too-large",
      "last sub-identifier of notification `%s' too large" },
    { 2, ERR_NODE_PARENT_TYPE, "parent-node",
      "node's parent node must be simple node" },
    { 2, ERR_SCALAR_PARENT_TYPE, "parent-scalar",
      "scalar's parent node must be simple node" },
    { 2, ERR_TABLE_PARENT_TYPE, "parent-table",
      "table's parent node must be simple node" },
    { 2, ERR_TABLE_ROW_TYPE_MISMATCH, "table-type",
      "table's SEQUENCE OF type does not match row type" },
    { 2, ERR_ROW_PARENT_TYPE, "parent-row",
      "row's parent node must be a table node" },
    { 2, ERR_COLUMN_PARENT_TYPE, "parent-column",
      "columnar object's parent node must be a row" },
    { 2, ERR_NOTIFICATION_PARENT_TYPE, "parent-notification",
      "notifications's parent node must be a simple node" },
    { 2, ERR_GROUP_PARENT_TYPE, "parent-group",
      "group's parent node must be a simple node" },
    { 2, ERR_COMPLIANCE_PARENT_TYPE, "parent-compliance",
      "compliance's parent node must be a simple node" },
    { 2, ERR_CAPABILITIES_PARENT_TYPE, "parent-capabilities",
      "capabilities' parent node must be a simple node" },
    { 1, ERR_SPPI_SIGNED64_NUMBER_RANGE, "", 
      "number `%s' is out of range for SPPI 64bit signed numbers" },
    { 1, ERR_SPPI_UNSIGNED64_NUMBER_RANGE, "", 
      "number `%s' is out of range for SPPI 64bit unsigned numbers" },
    { 0, ERR_SMI_CONSTRUCT_IN_PIB, "", 
      "the SMI construct/keyword `%s' may not be used in a PIB" },
    { 0, ERR_SPPI_CONSTRUCT_IN_MIB, "", 
      "the SPPI construct/keyword `%s' may not be used in a MIB" },
    { 2, ERR_POLICY_ACCESS_IN_PIB, "",
      "the PIB uses POLICY-ACCESS where PIB-ACCESS is required" },
    { 2, ERR_INVALID_SPPI_ACCESS, "access-invalid-sppi", 
      "invalid access `%s' in SPPI PIB" },
    { 2, ERR_INVALID_SPPI_STATUS, "status-invalid-sppi", 
      "invalid status `%s' in SPPI PIB" },
    { 2, ERR_INDEX_SHOULD_BE_PIB_INDEX, "",
      "the PIB uses ACCESS where PIB-INDEX is required" },
    { 0, ERR_INDEX_USED_TWICE, "",
      "INDEX was used twice inside an OBJECT-TYPE declaration" },
    { 1, ERR_SUBJECT_CATEGORIES_MISSING, "subject-categories-missing",
      "a MODULE-IDENTITY clause lacks SUBJECT-CATEGORIES" },
    { 1, ERR_UNKNOWN_SUBJECT_CATEGORY, "unknown-subject-category",
      "the subject category `%s' is invalid" },
    { 1, ERR_OBJECTS_MISSING_IN_OBJECT_GROUP, "objects-missing-in-group",
      "an OBJECT-GROUP is missing the OBJECTS part" },
    { 2, ERR_NOT_ACCESSIBLE_IN_PIB_ACCESS, "not-accessible-in-pib-access",
      "PIB-ACCESS must not be set to `not-accessible'" },
    { 2, ERR_REPORT_ONLY_IN_PIB_MIN_ACCESS, "report-only-in-pib-min-access",
      "PIB-MIN-ACCESS must not be set to `report-only'" },
    { 1, ERR_INDEX_AND_AUGMENTS_USED, "index-and-augments",
      "either INDEX or AUGMENTS may be used, but not both" },
    { 1, ERR_INDEX_WITHOUT_PIB_INDEX, "index-without-pib-index",
      "INDEX may not be used without PIB-INDEX" },
    { 3, ERR_ERROR_NUMBER_RANGE, "install-error-range",
      "the named-number for an INSTALL-ERROR is out of range: allowed 1..65535, current %d" },
    { 3, ERR_CATEGORY_ID_RANGE, "subject-categories-range",
      "the named-number for a SUBJECT-CATEGORIES entry must be greater than 0, current: %d" },
    { 2, ERR_SPPI_BASETYPE_NOT_IMPORTED, "sppi-basetype-not-imported",
      "SPPI basetype `%s' must be imported from COPS-PR-SPPI" },
    { 2, ERR_ROW_LACKS_PIB_INDEX, "row-lacks-pib-index",
      "row definition lacks a PIB-INDEX/AUGMENTS/EXTENDS clause" },
    { 2, ERR_PIB_INDEX_FOR_NON_ROW_TYPE, "pib-index-with-non-row-type",
      "non row type contains a PIB-INDEX/AUGMENTS/EXTENDS clause" },
    { 2, ERR_OPAQUE_IN_SYNTAX, "opaque-in-syntax",
      "`Opaque' must not be used in SYNTAX clauses in PIB modules" },
    { 2, ERR_IPADDRESS_IN_SYNTAX, "ipaddress-in-syntax",
      "`IpAddress' must not be used in SYNTAX clauses in PIB modules" },
    { 2, ERR_TABLE_LACKS_PIB_ACCESS, "table-lacks-pib-access",
      "the table definition must contain a PIB-ACCESS clause" },
    { 2, ERR_PIB_ACCESS_FOR_NON_TABLE, "pib-access-for-non-table",
      "type is not a table but contains a PIB-ACCESS clause" },
    { 2, ERR_INSTALL_ERRORS_FOR_NON_TABLE, "install-errors-for-non-table",
      "type is not a table but contains a INSTALL-ERRORS clause" },
    { 2, ERR_UNIQUENESS_FOR_NON_ROW, "uniqueness-for-non-row",
      "type is not a row but contains a UNIQUENESS clause" },
    { 2, ERR_NOT_A_COLUMN, "not-a-column",
      "`%s', which is used in a UNIQUENESS clause, is not a column in this table" },
    { 2, ERR_EXTENDS_WRONG_ROW_TYPE, "extend-row-type",
      "row `%s' extends `%s' which is neither a base table row nor a sparsely-augmented table row" },
    { 2, ERR_PIB_INDEX_NOT_INSTANCEID, "pib-index-not-instanceid",
      "the PIB-INDEX `%s' must be of type `InstanceId'" },
    { 2, ERR_LACKING_PIB_REFERENCES, "type-lacking-pib-references",
      "objects with SYNTAX `ReferenceId' must contain a PIB-REFERENCES clause" },
    { 2, ERR_PIB_REFERENCES_WRONG_TYPE, "pib-references-with-wrong-type",
      "object contains a PIB-REFERENCES clause but is not of type `ReferenceId'" },
    { 2, ERR_PIB_REFERENCES_NOT_ROW, "pib-references-not-row",
      "the PIB-REFERENCES does not point to a row" },
    { 2, ERR_LACKING_PIB_TAG, "type-lacking-pib-tag",
      "objects with SYNTAX `TagReferenceId' must contain a PIB-TAG clause" },
    { 2, ERR_PIB_TAG_WRONG_TYPE, "pib-tag-with-wrong-type",
      "object contains a PIB-TAG clause but is not of type `TagReferenceId'" },
    { 2, ERR_SUBJECT_CATEGORIES_MISSING_SUBID, "subject-categories-missing-subid",
      "the SUBJECT-CATEGORIES are missing a sub-identifier" },
    { 2, ERR_SUBJECT_CATEGORIES_ALL_WITH_SUBID, "subject-categories-all-with-subid",
      "the SUBJECT-CATEGORIES `all' must not have a sub-identifier" },
    { 3, ERR_SUBJECT_CATEGORIES_ALL, "subject-categories-all",
      "SUBJECT-CATEGORIES contains several categories although `all' is used aswell" },
    { 2, ERR_PIB_TAG_TYPE, "pib-tag-type",
      "a PIB-TAG must point to an object with a SYNTAX of `TagId'" },
    { 3, ERR_ATTRIBUTE_NOT_IN_GROUP, "attribute-not-in-group",
      "attribute `%s' must be contained in at least one conformance group" },
    { 0, ERR_OBJECTPTR_ELEMENT_IN_USE, "objectptr-element-in-use",
      "objectPtr->%s is already in use (%s)" },
    { 1, ERR_OID_ADMIN_ZERO, "",
      "last subidentifier assigned to `%s' may not be zero" },
    { 5, ERR_TYPE_STATUS_DEPRECATED, "type-status-deprecated",
      "type `%s' used by `%s' is deprecated" },
    { 5, ERR_TYPE_STATUS_OBSOLETE, "type-status-obsolete",
      "type `%s' used by `%s' is obsolete" },

    /* ESMI related */ 
    { 0, ERR_ESMI_TABLE_NOT_EXISTS, "",
      "Table is not existing in the MIB with the name : %s" },

    { 0, ERR_ESMI_TABLE_ALREADY_DEFINED, "",
      "Table already defined with the name : %s and with table ID: %s" },

    { 0, ERR_ESMI_TABLE_ID_ALREADY_DEFINED, "",
      "Table ID:  %s  already defined for the table : %s" },

    { 0, ERR_ESMI_OBJECT_NOT_EXISTS, "",
      "Object is not existing in the MIB with the name : %s" },

    { 0, ERR_ESMI_OBJECT_ALREADY_DEFINED, "",
      "Object already defined with the name : %s" },

    { 0, 0, NULL, NULL }
};



/*
 *----------------------------------------------------------------------
 *
 * smiSetErrorSeverity --
 *
 *      Sets the severity of errors with tags matching pattern.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Changes the severity in the module internal error table.
 *
 *----------------------------------------------------------------------
 */

void
smiSetErrorSeverity(char *pattern, int severity)
{
    int i;
    
    for (i = 0; errors[i].fmt; i++) {
    if (strstr(errors[i].tag, pattern) == errors[i].tag) {
        errors[i].level = severity;
    }
    }
}



/*
 *----------------------------------------------------------------------
 *
 * smiSetErrorHandler --
 *
 *      Set the function that is called to handle error messages.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Changes the error handler used for subsequent error messages.
 *
 *----------------------------------------------------------------------
 */

void
smiSetErrorHandler(SmiErrorHandler smiErrorHandler)
{
    smiHandle->errorHandler = smiErrorHandler;
}



/*
 *----------------------------------------------------------------------
 *
 * smiGetErrorSeverity --
 *
 *      Return the severity of the error identified by id.
 *
 * Results:
 *      The error severity.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int
smiGetErrorSeverity(int id)
{
    if (id < 0 || id >= (sizeof(errors) / sizeof(Error)) - 1) {
    return -1;
    }
    return errors[id].level;
}


/*
 *----------------------------------------------------------------------
 *
 * smiGetErrorTag --
 *
 *      Return the tag of the error identified by id.
 *
 * Results:
 *      The error tag or NULL.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

char*
smiGetErrorTag(int id)
{
    if (id < 0 || id >= sizeof(errors) / sizeof(Error)) {
    return NULL;
    }
    return errors[id].tag ? errors[id].tag : "";
}



/*
 *----------------------------------------------------------------------
 *
 * smiGetErrorMsg --
 *
 *      Return the message of the error identified by id.
 *
 * Results:
 *      The error message or NULL.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

char*
smiGetErrorMsg(int id)
{
    if (id < 0 || id >= sizeof(errors) / sizeof(Error)) {
    return NULL;
    }
    return errors[id].fmt ? errors[id].fmt : "";
}



/*
 *----------------------------------------------------------------------
 *
 * smiErrorHandler --
 *
 *      This is the default error printing function. This is the
 *  only place in the libsmi where any output is generated.
 *  This function may be replaced by an application specific
 *  error message handler.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Prints error messages to the stderr output channel.
 *
 *----------------------------------------------------------------------
 */

void
smiErrorHandler(char *path, int line, int severity, char *msg, char *tag)
{
    if (path) {
    fprintf(stderr, "%s:%d: ", path, line);
    }
    if (severity > 2) {
    fprintf(stderr, "warning: ");
    }
    fprintf(stderr, "%s\n", msg);

    /*
     * A severe error, no way to continue :-(
     */
    if (severity <= 0) {
    exit(1);
    }
}



/*
 *----------------------------------------------------------------------
 *
 * printError --
 *
 *      Internal error printer which is called by the varargs
 *  entry points (see below). If formats the error message
 *  and calls the error handling function that is currently
 *  registered.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Terminates the program if the error is severe and there
 *  is no way to continue.
 *
 *----------------------------------------------------------------------
 */

static void
printError(Parser *parser, int id, int line, va_list ap)
{
    char *buffer;
    int i;
    
    if (! smiHandle->errorHandler) {
    return;
    }

    /*
     * Search for the tag instead of just using the id as an index so
     * that we do not run into trouble if the id is bogus.
     */

    for (i = 0; errors[i].fmt; i++) {
    if (errors[i].id == id) break;
    }
    if (! errors[i].fmt) {
    i = 0;      /* assumes that 0 is the internal error */
    }

    if (parser) {

    if (parser->modulePtr) {
        if ((parser->modulePtr->export.conformance > errors[i].level) ||
        (parser->modulePtr->export.conformance == 0)) {
        parser->modulePtr->export.conformance = errors[i].level;
        }
    }
    
    if ((errors[i].level <= smiHandle->errorLevel) &&
        (parser->flags & SMI_FLAG_ERRORS) &&
        ((smiDepth == 1) || (parser->flags & SMI_FLAG_RECURSIVE))) {
        smiVasprintf(&buffer, errors[i].fmt, ap);
        (smiHandle->errorHandler) (parser->path, line,
                       errors[i].level, buffer, errors[i].tag);
    }
    } else {
    if (errors[i].level <= smiHandle->errorLevel) {
        smiVasprintf(&buffer, errors[i].fmt, ap);
        (smiHandle->errorHandler) (NULL, 0, errors[i].level,
                       buffer, errors[i].tag);
    }
    }
}



/*
 *----------------------------------------------------------------------
 *
 * smiPrintErrorAtLine --
 *
 *      Like smiPrintError() but shows a specfic line no.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiPrintErrorAtLine(Parser *parser, int id, int line, ...)
{
    va_list ap;

    va_start(ap, line);
    printError(parser, id, line, ap);
    va_end(ap);
}



/*
 *----------------------------------------------------------------------
 *
 * smiPrintError --
 *
 *      Prints an error message. The line number is taken from
 *  the current parser position.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

void
smiPrintError(Parser *parser, int id, ...)
{
    va_list ap;

    va_start(ap, id);
    printError(parser, id, parser ? parser->line : 0, ap);
    va_end(ap);
}
