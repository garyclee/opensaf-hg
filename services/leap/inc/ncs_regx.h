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
 * Author(s): Emerson Network Power
 *   
 */

/*****************************************************************************
..............................................................................

  MODULE NAME: NCS_REGX.H

..............................................................................

  DESCRIPTION:

  Abstractions used for Regular Expression management and matching.

  Fundemental Regular Expression Matching.......

  Pattern Matching Components
  ===========================

  [abc]    - Matches 'a' or 'b' or 'c'
  [a-z]    - Matches a character in the ascii range 'a' to 'z'
  a$       - Insists that the last character before \n or \0 is an 'a'
  ^a       - Insists that the first character in the string is an 'a'
  a?       - Matches 0 or one 'a' characters in a row
  a*       - Matches 0 or more 'a' characters in a row.
  a+       - Matches 1 or more 'a' characters in a row
  .        - Matches any character
  \d       - escape for special characters, which are {,},[,],.,$,^, etc.
              - control characters include \n, \r and \f
              - 'normal' characters can be preceeded by '\' with no 
               adverse effects.
  \125     - treated as an octal sequence. \125 happens to map to ascii 'U'

  Termination Characters
  =======================

  \0 and \n are both treated as termination characters for a regular
  expression ubmitted to ncs_regx_init() which results in internal 
  regular expression compile.

  Restrictions (NOT SUPPORTED)
  ============================

  (<regx>) - Grouping syntax '()' to group regular expression subparts into 
             a more complex expression. For example, ([a-z][A-Z])+, which might
             otherwise match bBcCdA. 

  Example Pattern Expressions
  ===========================

 "[a-z]+ABA8*$"     =  one or more characters in the range 'a' - 'z' (see 
                       ASCII chart) followed by ABA, followed by zero or 
                       more '8's. This pattern match must conclude at the 
                       null termination ($)                          
                       NOTE: \n termination on regular expression compile line 
                   
"\\{fred[\\{]*\\125A"=  match '{' followed by 'fred' followed by zero or more  '{' 
                       followed by 'U', which is \125 in octal, followed by 'A'.

"^z.fred[ab]c+"      = must start with 'z' followed by any-char'.' followed 
                       by 'fred' followed by either 'a' or 'b' followed by 
                       one or more 'c's.

  Collecting sub-parts of Patterns Matched
  ========================================
  This instance of regx allows a user to isolate matched subpatterns found 
  in the master pattern match. this is done by grouping that part of the 
  regular expression you wish isolated in '{}' delimiters.

  For example:

  This regular expression .. "^z.fred[ab]c+", 
  matches strings like    ..  "zmfredacccc","z<fredbc" and"z?fredbcc".

  Taking this same regular expression but adding substring syntax such as ..

  "^z{.}fred{[ab]c+}", 

  And using the same test strings, and then using functions 

  ncs_regx_get_count()  - The number of substrings collected 
  ncs_regx_get_result() - fetch a specific substring instance

  We would get the following results:

  Test String  |  'Register' Content for Match
  -------------+----------------------------------
  zmfredacccc  | 0) zmfredacccc  1) m  2) acccc
  z<fredbc     | 0) z<fredbc     1) <  2) bc
  z?fredbcc    | 0) z?fredbc     1) ?  2) bcc
  -------------+----------------------------------

  Notice that 'register 0' always contains the entire pattern matched.
******************************************************************************
*/

#ifndef NCS_REGX_H
#define NCS_REGX_H


#define  T_PATTERN_SIZE  300
#define  C_PATTERN_SIZE  500
#define  BITTAB_DIM      8

#define NBRA             10

/*************************************************************************** 
 * Required Memory Management Macros for Regular Expression
 ***************************************************************************/
      
#define m_MMGR_ALLOC_REGX      (NCS_REGX*) m_NCS_MEM_ALLOC(sizeof(NCS_REGX),\
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_COMMON,     \
                                               0)

/* The Free cases */

#define m_MMGR_FREE_REGX(p)                    m_NCS_MEM_FREE(p,            \
                                               NCS_MEM_REGION_PERSISTENT,   \
                                               NCS_SERVICE_ID_COMMON,       \
                                               0)
   
/*************************************************************************** 
 * Storage for the original regular expression (t_pattern) and the 
 * compiled version of the regular expression (c_pattern).
 ***************************************************************************/
   
typedef struct t_alist_s
  {
  char            t_pattern[T_PATTERN_SIZE];
  char            c_pattern[C_PATTERN_SIZE];
  
  } T_ALIST;

/*************************************************************************** 
 * Substring storage for matched substring parts.
 ***************************************************************************/

typedef struct subs 
  {
  char*           s_cp;
  unsigned int    s_len;
  
  } SUBS;

/*************************************************************************** 
 * The master NCS_REGX data structure...
 ***************************************************************************/

typedef struct ncs_regx
  {
  uns16   exists;
  SUBS    subspace[NBRA];
  SUBS*   subsp;

  T_ALIST t_alist;
      
  char*    braslist[NBRA];
  char*    braelist[NBRA];
  int      ebra;
  int      nbra;
  char*    loc1;
  char*    loc2;
  char*    locs;
  int      nodelim;
  
  int      low;
  int      size;
  
  } NCS_REGX;

/****************************************************************************
 *   H J _ R E G X      P U B L I C   F U N C T I O N S
 ****************************************************************************/
 
void*   ncs_regx_init      (char*  pattern);
uns32   ncs_regx_destroy   (void*  regx_hdl);  

uns32   ncs_regx_match     (void*  regx_hdl, 
                           char*  src);

uns32   ncs_regx_get_count (void*  regx_hdl); 
NCS_BOOL ncs_regx_get_result(void*  regx_hdl, 
                           uns32  idx, 
                           char*  space, 
                           uns32  len);

uns32   ncs_regx_get_length(void*  regx_hdl);

#endif
