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

/* A Bison parser, made from clipar.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */



/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "cli.h"

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                Variable declaration.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
static CLI_CB *parCli = 0;

/* Global variables */
int32 yylineno;

/* External defined functions */
EXTERN_C int8 *yytext;
EXTERN_C void yyerror(int8 *);
EXTERN_C int32 yylex();

static void
get_range_values(CLI_CMD_ELEMENT *, CLI_TOKEN_ATTRIB, int8 *, int8 *);
static void do_lbrace_opr(CLI_CB *);
static void do_rbarce_opr(CLI_CB *);
static void do_lcurlybrace_opr(CLI_CB *);
static void do_rcurlybrace_opr(CLI_CB *);
                 
#define m_SET_TOKEN_ATTRIBUTE(val)\
{\
    cli_set_token_attrib(parCli,\
        parCli->par_cb.tokList[parCli->par_cb.tokCnt - 1],\
        val, yytext);\
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                Token constants

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#ifndef YYSTYPE
# define YYSTYPE int
# define YYSTYPE_IS_TRIVIAL 1
#endif
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define    YYFINAL        58
#define    YYFLAG        -32768
#define    YYNTBASE    27

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 280 ? yytranslate[x] : 42)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     2,     4,     7,     8,    13,    15,    17,    21,
      23,    26,    27,    32,    36,    41,    42,    49,    51,    53,
      55,    57,    59,    61,    63,    65,    67,    69,    72,    74,
      77,    80,    83,    86,    88,    91,    94,    97,   100,   102,
     104,   106,   108,   110,   112,   114,   116,   118,   120
};
static const short yyrhs[] =
{
      28,     0,    30,     0,    30,    28,     0,     0,    30,    22,
      29,    28,     0,    37,     0,    31,     0,    35,    32,    36,
       0,    37,     0,    37,    32,     0,     0,    37,    22,    33,
      32,     0,    35,    32,    36,     0,    35,    32,    36,    32,
       0,     0,    35,    32,    36,    22,    34,    32,     0,    20,
       0,    18,     0,    16,     0,    21,     0,    19,     0,    17,
       0,    39,     0,    40,     0,    38,     0,    23,     0,    38,
      14,     0,     3,     0,    39,    12,     0,    39,    13,     0,
      39,    14,     0,    39,    15,     0,    41,     0,    40,    12,
       0,    40,    13,     0,    40,    14,     0,    40,    15,     0,
       4,     0,     5,     0,     7,     0,     8,     0,     9,     0,
      10,     0,    11,     0,     6,     0,    24,     0,    25,     0,
      26,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   130,   133,   134,   135,   135,   144,   145,   148,   151,
     152,   153,   153,   160,   161,   162,   162,   171,   172,   176,
     182,   183,   187,   193,   194,   195,   198,   208,   215,   220,
     226,   232,   238,   246,   247,   253,   259,   265,   273,   278,
     283,   288,   293,   298,   303,   308,   313,   318,   323
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "CLI_TOK_KEYWORD", "CLI_TOK_PARAM", 
  "CLI_TOK_NUMBER", "CLI_TOK_PASSWORD", "CLI_TOK_CIDRv4", 
  "CLI_TOK_IPADDRv4", "CLI_TOK_IPADDRv6", "CLI_TOK_MASKv4", 
  "CLI_TOK_CIDRv6", "CLI_TOK_RANGE", "CLI_TOK_DEFAULT_VAL", 
  "CLI_TOK_HELP_STR", "CLI_TOK_MODE_CHANGE", "CLI_TOK_LCURLYBRACE", 
  "CLI_TOK_RCURLYBRACE", "CLI_TOK_LBRACE", "CLI_TOK_RBRACE", 
  "CLI_TOK_LESS_THAN", "CLI_TOK_GRTR_THAN", "CLI_TOK_OR", 
  "CLI_TOK_CONTINOUS_EXP", "CLI_TOK_COMMUNITY", "CLI_TOK_WILDCARD", 
  "CLI_TOK_MACADDR", "Command_String", "token_param", "@1", "param_type", 
  "expression", "delimiter_option", "@2", "@3", "left_delimiter", 
  "right_delimiter", "tokens", "continous_exp", "keyword", "parameters", 
  "parameter_type", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    27,    28,    28,    29,    28,    30,    30,    31,    32,
      32,    33,    32,    32,    32,    34,    32,    35,    35,    35,
      36,    36,    36,    37,    37,    37,    38,    38,    39,    39,
      39,    39,    39,    40,    40,    40,    40,    40,    41,    41,
      41,    41,    41,    41,    41,    41,    41,    41,    41
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     1,     1,     2,     0,     4,     1,     1,     3,     1,
       2,     0,     4,     3,     4,     0,     6,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     2,
       2,     2,     2,     1,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       0,    28,    38,    39,    45,    40,    41,    42,    43,    44,
      19,    18,    17,    26,    46,    47,    48,     1,     2,     7,
       0,     6,    25,    23,    24,    33,     4,     3,     0,     0,
       9,    27,    29,    30,    31,    32,    34,    35,    36,    37,
       0,    22,    21,    20,     8,     0,    11,    10,     5,    13,
       0,    15,    14,    12,     0,    16,     0,     0,     0
};

static const short yydefgoto[] =
{
      56,    17,    40,    18,    19,    28,    50,    54,    29,    44,
      30,    22,    23,    24,    25
};

static const short yypact[] =
{
      98,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    26,-32768,
      98,-32768,    -8,    -2,     2,-32768,-32768,-32768,   -12,    98,
      50,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
      98,-32768,-32768,-32768,-32768,   -12,-32768,-32768,-32768,    74,
      98,-32768,-32768,-32768,    98,-32768,     8,    20,-32768
};

static const short yypgoto[] =
{
  -32768,   -14,-32768,-32768,-32768,   -27,-32768,-32768,     0,   -24,
       1,-32768,-32768,-32768,-32768
};


#define    YYLAST        124


static const short yytable[] =
{
      20,    21,    45,    47,    27,    41,    31,    42,    57,    43,
      32,    33,    34,    35,    36,    37,    38,    39,    20,    21,
      58,    49,    52,    53,     0,     0,    48,    55,     0,     1,
       2,     3,     4,     5,     6,     7,     8,     9,     0,     0,
      20,    21,    10,     0,    11,     0,    12,     0,    26,    13,
      14,    15,    16,     1,     2,     3,     4,     5,     6,     7,
       8,     9,     0,     0,     0,     0,    10,     0,    11,     0,
      12,     0,    46,    13,    14,    15,    16,     1,     2,     3,
       4,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,    11,     0,    12,     0,    51,    13,    14,    15,
      16,     1,     2,     3,     4,     5,     6,     7,     8,     9,
       0,     0,     0,     0,    10,     0,    11,     0,    12,     0,
       0,    13,    14,    15,    16
};

static const short yycheck[] =
{
       0,     0,    29,    30,    18,    17,    14,    19,     0,    21,
      12,    13,    14,    15,    12,    13,    14,    15,    18,    18,
       0,    45,    49,    50,    -1,    -1,    40,    54,    -1,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    -1,    -1,
      40,    40,    16,    -1,    18,    -1,    20,    -1,    22,    23,
      24,    25,    26,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    -1,    -1,    -1,    -1,    16,    -1,    18,    -1,
      20,    -1,    22,    23,    24,    25,    26,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    -1,    -1,    -1,    -1,
      16,    -1,    18,    -1,    20,    -1,    22,    23,    24,    25,
      26,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      -1,    -1,    -1,    -1,    16,    -1,    18,    -1,    20,    -1,
      -1,    23,    24,    25,    26
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
     || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))    \
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))                \
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)        \
      do                    \
    {                    \
      register YYSIZE_T yyi;        \
      for (yyi = 0; yyi < (Count); yyi++)    \
        (To)[yyi] = (From)[yyi];        \
    }                    \
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)                    \
    do                                    \
      {                                    \
    YYSIZE_T yynewbytes;                        \
    YYCOPY (&yyptr->Stack, Stack, yysize);                \
    Stack = &yyptr->Stack;                        \
    yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;    \
    yyptr += yynewbytes / sizeof (*yyptr);                \
      }                                    \
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok        (yyerrstatus = 0)
#define yyclearin    (yychar = YYEMPTY)
#define YYEMPTY        -2
#define YYEOF        0
#define YYACCEPT    goto yyacceptlab
#define YYABORT     goto yyabortlab
#define YYERROR        goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL        goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)                    \
do                                \
  if (yychar == YYEMPTY && yylen == 1)                \
    {                                \
      yychar = (Token);                        \
      yylval = (Value);                        \
      yychar1 = YYTRANSLATE (yychar);                \
      YYPOPSTACK;                        \
      goto yybackup;                        \
    }                                \
  else                                \
    {                                 \
      yyerror ("syntax error: cannot back up");            \
      YYERROR;                            \
    }                                \
while (0)

#define YYTERROR    1
#define YYERRCODE    256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)           \
   Current.last_line   = Rhs[N].last_line;    \
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX        yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX        yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX        yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX        yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX            yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)            \
do {                        \
  if (yydebug)                    \
    YYFPRINTF Args;                \
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef    YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES            \
/* The lookahead symbol.  */                \
int yychar;                        \
                            \
/* The semantic value of the lookahead symbol. */    \
YYSTYPE yylval;                        \
                            \
/* Number of parse errors so far.  */            \
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES            \
YY_DECL_NON_LSP_VARIABLES            \
                        \
/* Location data for the lookahead symbol.  */    \
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES            \
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short    yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;        /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
    /* Give user a chance to reallocate the stack. Use copies of
       these so that the &'s don't force the real ones into
       memory.  */
    YYSTYPE *yyvs1 = yyvs;
    short *yyss1 = yyss;

    /* Each stack pointer address is followed by the size of the
       data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
    YYLTYPE *yyls1 = yyls;
    /* This used to be a conditional around just the two extra args,
       but that might be undefined if yyoverflow is a macro.  */
    yyoverflow ("parser stack overflow",
            &yyss1, yysize * sizeof (*yyssp),
            &yyvs1, yysize * sizeof (*yyvsp),
            &yyls1, yysize * sizeof (*yylsp),
            &yystacksize);
    yyls = yyls1;
# else
    yyoverflow ("parser stack overflow",
            &yyss1, yysize * sizeof (*yyssp),
            &yyvs1, yysize * sizeof (*yyvsp),
            &yystacksize);
# endif
    yyss = yyss1;
    yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
    goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
    yystacksize = YYMAXDEPTH;

      {
    short *yyss1 = yyss;
    union yyalloc *yyptr =
      (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
    if (! yyptr)
      goto yyoverflowlab;
    YYSTACK_RELOCATE (yyss);
    YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
    YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
    if (yyss1 != yyssa)
      YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
          (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
    YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)        /* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;        /* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
    which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
    {
      YYFPRINTF (stderr, "Next token is %d (%s",
             yychar, yytname[yychar1]);
      /* Give the individual parser a way to print the precise
         meaning of a token, for further debugging info.  */
# ifdef YYPRINT
      YYPRINT (stderr, yychar, yylval);
# endif
      YYFPRINTF (stderr, ")\n");
    }
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
    goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
          yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
         yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 4:
{
                        /* set the OR flag */
                        parCli->par_cb.orFlag = TRUE;                                               
                    ;
    break;}
case 11:
{
                            /* set the OR flag */
                            parCli->par_cb.orFlag = TRUE;
                        ;
    break;}
case 15:
{
                            /* set the OR flag */
                            parCli->par_cb.orFlag = TRUE;                       
                        ;
    break;}
case 18:
{       
                        do_lbrace_opr(parCli);
                    ;
    break;}
case 19:
{   
                        do_lcurlybrace_opr(parCli);                        
                    ;
    break;}
case 21:
{   
                        do_rbarce_opr(parCli);
                    ;
    break;}
case 22:
{                     
                        do_rcurlybrace_opr(parCli);
                    ;
    break;}
case 26:
{
                        /* Evaluate the level and relation type of token */                      
                        cli_evaluate_token(NCSCLI_CONTINOUS_EXP);
                        
                        /* Set the attribute of the token */  
                        m_SET_TOKEN_ATTRIBUTE(CLI_CONTINOUS_RANGE);                      
                        
                        parCli->par_cb.is_tok_cont = TRUE;                        
                    ;
    break;}
case 27:
{
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_HELP_STR);                        
                    ;
    break;}
case 28:
{   
                        /* Evaluate the level and relation type of token */                      
                        cli_evaluate_token(NCSCLI_KEYWORD);                                              
                    ;
    break;}
case 29:
{
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_RANGE_VALUE);                                                
                    ;
    break;}
case 30:
{
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_DEFALUT_VALUE);                        
                    ;
    break;}
case 31:
{
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_HELP_STR);                        
                    ;
    break;}
case 32:
{
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_MODE_CHANGE);                        
                    ;
    break;}
case 34:
{
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_RANGE_VALUE);                        
                    ;
    break;}
case 35:
{
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_DEFALUT_VALUE);                        
                    ;
    break;}
case 36:
{
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_HELP_STR);                        
                    ;
    break;}
case 37:
{
                        /* Set the attribute of the token */
                        m_SET_TOKEN_ATTRIBUTE(CLI_MODE_CHANGE);                        
                    ;
    break;}
case 38:
{   
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_STRING);
                    ;
    break;}
case 39:
{   
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_NUMBER);
                    ;
    break;}
case 40:
{
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_CIDRv4);
                    ;
    break;}
case 41:
{   
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_IPv4);
                    ;
    break;}
case 42:
{
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_IPv6);                 
                    ;
    break;}
case 43:
{   
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_MASKv4);
                    ;
    break;}
case 44:
{   
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_CIDRv6);
                    ;
    break;}
case 45:
{
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_PASSWORD);
                    ;
    break;}
case 46:
{
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_COMMUNITY);
                    ;
    break;}
case 47:
{
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_WILDCARD);
                    ;
    break;}
case 48:
{
                        /* Evaluate the level and relation type of token */
                        cli_evaluate_token(NCSCLI_MACADDR);
                    ;
    break;}
}



  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
    YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
    {
      YYSIZE_T yysize = 0;
      char *yymsg;
      int yyx, yycount;

      yycount = 0;
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  */
      for (yyx = yyn < 0 ? -yyn : 0;
           yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
        if (yycheck[yyx + yyn] == yyx)
          yysize += yystrlen (yytname[yyx]) + 15, yycount++;
      yysize += yystrlen ("parse error, unexpected ") + 1;
      yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
      yymsg = (char *) YYSTACK_ALLOC (yysize);
      if (yymsg != 0)
        {
          char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
          yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

          if (yycount < 5)
        {
          yycount = 0;
          for (yyx = yyn < 0 ? -yyn : 0;
               yyx < (int) (sizeof (yytname) / sizeof (char *));
               yyx++)
            if (yycheck[yyx + yyn] == yyx)
              {
            const char *yyq = ! yycount ? ", expecting " : " or ";
            yyp = yystpcpy (yyp, yyq);
            yyp = yystpcpy (yyp, yytname[yyx]);
            yycount++;
              }
        }
          yyerror (yymsg);
          YYSTACK_FREE (yymsg);
        }
      else
        yyerror ("parse error; also virtual memory exhausted");
    }
      else
#endif /* defined (YYERROR_VERBOSE) */
    yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
     error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
    YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
          yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;        /* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
    YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
    goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


/*****************************************************************************
 Procedure NAME   : do_lbrace_opr
 DESCRIPTION       : Routine thats does the sematic matching when left barce 
                        is encountered
 ARGUMENTS           : CLI_CB *
 RETURNS              : none 
*****************************************************************************/
static void do_lbrace_opr(CLI_CB *pCli)
{
    /*  increment the Optional brace type counter */
    pCli->par_cb.optCntr++;

    /* Set the level */                 
    if(pCli->par_cb.orFlag == TRUE) {
        pCli->par_cb.optLvlCntr = pCli->par_cb.tokLvlCntr;

        if(pCli->par_cb.grpCntr == 0) {   
            if(pCli->par_cb.optCntr > 1)
                pCli->par_cb.relnType = CLI_OPT_SIBLING_TOKEN;
            else
                pCli->par_cb.relnType = CLI_SIBLING_TOKEN;                            
        }
        else pCli->par_cb.relnType = CLI_GRP_SIBLING_TOKEN;
    }
    else {
        pCli->par_cb.optLvlCntr = (uns16)(pCli->par_cb.tokLvlCntr + 1);
    
        if(pCli->par_cb.grpCntr == 0) {   
            if(pCli->par_cb.optCntr > 1)
                pCli->par_cb.relnType = CLI_OPT_CHILD_TOKEN;
            else
                pCli->par_cb.relnType = CLI_CHILD_TOKEN;                                                  
        }
        else pCli->par_cb.relnType = CLI_GRP_CHILD_TOKEN;
    }
                
    /* push the current level into the optional stack */                                                
    pCli->par_cb.optStack[++pCli->par_cb.optStackPtr] = pCli->par_cb.optLvlCntr;

    pCli->par_cb.brcType = CLI_OPT_BRACE;                       
    pCli->par_cb.brcStack[++pCli->par_cb.brcStackPtr] = CLI_OPT_BRACE;

    /*Process token*/
    cli_process_token(pCli->par_cb.optLvlCntr, pCli->par_cb.relnType, NCSCLI_OPTIONAL);                     
    pCli->par_cb.firstOptChild = TRUE;    
}

/*****************************************************************************
 Procedure NAME    : do_lcurlybrace_opr
 DESCRIPTION       : Routine thats does the sematic matching when left curly barce 
                        is encountered
 ARGUMENTS           : CLI_CB *
 RETURNS              : none 
*****************************************************************************/
static void do_lcurlybrace_opr(CLI_CB *pCli)
{
    pCli->par_cb.grpCntr++;
                        
    /* Set the level */                 
    if(pCli->par_cb.orFlag == TRUE) {
        pCli->par_cb.grpLvlCntr = pCli->par_cb.tokLvlCntr;
    
        if(pCli->par_cb.optCntr == 0) {
            if(pCli->par_cb.grpCntr > 1)
                pCli->par_cb.relnType = CLI_GRP_SIBLING_TOKEN;
            else
                pCli->par_cb.relnType = CLI_SIBLING_TOKEN;                            
        }
        else pCli->par_cb.relnType = CLI_OPT_SIBLING_TOKEN;                            
    }
    else {
        pCli->par_cb.grpLvlCntr = (uns16)(pCli->par_cb.tokLvlCntr + 1);
    
        if(pCli->par_cb.optCntr == 0) {
            if(pCli->par_cb.grpCntr > 1)
                pCli->par_cb.relnType = CLI_GRP_CHILD_TOKEN;
            else 
                pCli->par_cb.relnType = CLI_CHILD_TOKEN;                                                  
        }
        else pCli->par_cb.relnType = CLI_OPT_CHILD_TOKEN;                          
    }
                
    /* push the current level into the optional stack */                                                
    pCli->par_cb.grpStack[++pCli->par_cb.grpStackPtr] = pCli->par_cb.grpLvlCntr;
                                                    
    pCli->par_cb.brcType = CLI_GRP_BRACE;                       
    pCli->par_cb.brcStack[++pCli->par_cb.brcStackPtr] = CLI_GRP_BRACE;                      

    /*Process token */
    cli_process_token(pCli->par_cb.grpLvlCntr, pCli->par_cb.relnType, NCSCLI_GROUP);
    pCli->par_cb.firstGrpChild = TRUE;
}

/*****************************************************************************
 Procedure NAME    : do_rbrace_opr
 DESCRIPTION       : Routine thats does the sematic matching when right barce 
                        is encountered
 ARGUMENTS           : CLI_CB *
 RETURNS              : none 
*****************************************************************************/
static void do_rbarce_opr(CLI_CB *pCli)
{
    /* decrement the optional counter */
    pCli->par_cb.optCntr--;

    /* pop the stord level from the stack */
    pCli->par_cb.tokLvlCntr = (uns16)pCli->par_cb.optStack[pCli->par_cb.optStackPtr];    
    pCli->par_cb.optStackPtr--;                   

    pCli->par_cb.optTokCnt = 0;

    --pCli->par_cb.brcStackPtr;
    if(pCli->par_cb.brcStackPtr > -1)
        pCli->par_cb.brcType = pCli->par_cb.brcStack[pCli->par_cb.brcStackPtr];
    else
        pCli->par_cb.brcType = CLI_NONE_BRACE;
    
    /*Process token */
    cli_process_token(pCli->par_cb.grpLvlCntr, CLI_NO_RELATION, NCSCLI_BRACE_END);
}

/*****************************************************************************
 Procedure NAME    : do_rcurlybrace_opr
 DESCRIPTION       : Routine thats does the sematic matching when right curly barce 
                        is encountered
 ARGUMENTS           : CLI_CB *
 RETURNS              : none 
*****************************************************************************/
static void do_rcurlybrace_opr(CLI_CB *pCli)
{
    /* decrement the optional counter */
    pCli->par_cb.grpCntr--;

    /* pop the stord level from the stack */
    pCli->par_cb.tokLvlCntr = (uns16)pCli->par_cb.grpStack[pCli->par_cb.grpStackPtr];    
    pCli->par_cb.grpStackPtr--;

    pCli->par_cb.grpTokCnt = 0;    
    --pCli->par_cb.brcStackPtr;
    if(pCli->par_cb.brcStackPtr > -1)
        pCli->par_cb.brcType = pCli->par_cb.brcStack[pCli->par_cb.brcStackPtr];
    else
        pCli->par_cb.brcType = CLI_NONE_BRACE;
            
    /*Process token */
    pCli->par_cb.is_tok_cont = FALSE;
    cli_process_token(pCli->par_cb.grpLvlCntr, CLI_NO_RELATION, NCSCLI_BRACE_END);
}

/*****************************************************************************
  Procedure NAME    :    yygettoken
  DESCRIPTION        :    The yygettoken function is called by the parser whenever 
                           it needs a new token. The function returns a zero or 
                           negative value if there are no more tokens, otherwise a 
                           positive value indicating the next token. The default 
                           implementation of yygettoken simply calls yylex.

  ARGUMENTS            :   none
  RETURNS            :    Zero or negative if there are no more tokens for the 
                           parser, otherwise a positive value indicating the next 
                           token.
  NOTES                :
*****************************************************************************/
uns32 yygettoken(void)
{
    return yylex(); 
}

/*****************************************************************************
  Procedure NAME    :    cli_evaluate_token
  DESCRIPTION        :    A function is called by the parser when it encounters any
                           keyword, parameter which may be either NCSCLI_NUMBER, 
                           NCSCI_IP_ADDRESS etc. to evaluate the level of the token and
                           the other necessary attributes that is required by the
                           command tree.
  ARGUMENTS            :    Token type
  RETURNS            :    none
  NOTES                :
*****************************************************************************/
void cli_evaluate_token(NCSCLI_TOKEN_TYPE i_token_type)
{
   if(TRUE == parCli->par_cb.orFlag) {
      parCli->par_cb.orFlag = FALSE;

      switch(parCli->par_cb.brcType) {
      case CLI_OPT_BRACE:                                     
         if(TRUE == parCli->par_cb.firstOptChild) {
            parCli->par_cb.firstOptChild = FALSE;
            parCli->par_cb.relnType  = CLI_OPT_CHILD_TOKEN;                 
            parCli->par_cb.optTokCnt++;
         }
         else parCli->par_cb.relnType  = CLI_OPT_SIBLING_TOKEN;                                    
         break;

      case CLI_GRP_BRACE:
         if(TRUE == parCli->par_cb.firstGrpChild) {
            parCli->par_cb.firstGrpChild = FALSE;
            parCli->par_cb.relnType  = CLI_GRP_CHILD_TOKEN;                 
            parCli->par_cb.grpTokCnt++;
         }
         else parCli->par_cb.relnType = CLI_GRP_SIBLING_TOKEN;                                                
         break;

      case CLI_NONE_BRACE:
         parCli->par_cb.relnType = CLI_SIBLING_TOKEN;
         break;              

      default:
         break;            
      }       
   }
   else {
      parCli->par_cb.tokLvlCntr++;
      parCli->par_cb.optLvlCntr = parCli->par_cb.tokLvlCntr;
      parCli->par_cb.grpLvlCntr = parCli->par_cb.tokLvlCntr;                          
  
      switch(parCli->par_cb.brcType) {
      case CLI_OPT_BRACE:                                     
         parCli->par_cb.optTokCnt++;
                                  
         if(TRUE == parCli->par_cb.firstOptChild)
            parCli->par_cb.firstOptChild = FALSE;

         if(parCli->par_cb.optCntr > 0)
            parCli->par_cb.relnType = CLI_OPT_CHILD_TOKEN;            
         break;

      case CLI_GRP_BRACE:
         parCli->par_cb.grpTokCnt++;
                  
         if(TRUE == parCli->par_cb.firstGrpChild)
            parCli->par_cb.firstGrpChild = FALSE;

         if(parCli->par_cb.grpCntr > 0)
            parCli->par_cb.relnType = CLI_GRP_CHILD_TOKEN;
         break;              

      case CLI_NONE_BRACE:
         parCli->par_cb.relnType = CLI_CHILD_TOKEN;              
         break;                  

      default:
         break;
      }
   }
            
   cli_process_token(parCli->par_cb.tokLvlCntr, parCli->par_cb.relnType, i_token_type);     
}

/*****************************************************************************
  Procedure NAME    :    cli_process_token
  DESCRIPTION        :    A function is called by the parser when it encounters any
                        valid token. The function populates the nessary attribute 
                        of the token so that it can be added into the command tree
                        depending upon the attributes.                    
  ARGUMENTS            :    i_tokenLevel     - The level of the token.
                        i_token_relation - Realtionship of the token
                        i_token_type     - Token type
  RETURNS            :   none
  NOTES                :
*****************************************************************************/
void cli_process_token(int32 i_token_level, CLI_TOKEN_RELATION i_token_relation, 
                       NCSCLI_TOKEN_TYPE i_token_type)
{   
    /* Create token node */ 
    CLI_CMD_ELEMENT *cmd_node = m_MMGR_ALLOC_CLI_CMD_ELEMENT;
    
    if(0 == cmd_node) return;        
    memset(cmd_node, 0, sizeof(CLI_CMD_ELEMENT));
    
    /* Set the token type */
    cmd_node->tokType  = i_token_type;
    cmd_node->tokLvl = (uns8)i_token_level;          
    
    if(TRUE == parCli->par_cb.is_tok_cont)
        cmd_node->isCont = TRUE;        
    else
        cmd_node->isCont = FALSE;         
        
    /* Set the token type ie Mandatory or Optional */
   switch(i_token_relation) {           
   case CLI_CHILD_TOKEN:
      if(NCSCLI_OPTIONAL == i_token_type)
         cmd_node->isMand = FALSE;
      else                        
         cmd_node->isMand = TRUE;                                           
      break;

   case CLI_OPT_CHILD_TOKEN:
      cmd_node->isMand = FALSE;                              
      break;          

   case CLI_GRP_CHILD_TOKEN:
   cmd_node->isMand = TRUE;                   
      if(1 == parCli->par_cb.grpTokCnt && parCli->par_cb.optCntr > 0)
         cmd_node->isMand = FALSE;
      else if(i_token_type == NCSCLI_OPTIONAL)
         cmd_node->isMand = FALSE;
      break;

   case CLI_SIBLING_TOKEN:
      if(NCSCLI_OPTIONAL == i_token_type)
         cmd_node->isMand = FALSE;
      else            
         cmd_node->isMand = TRUE;                                       
      break;

   case CLI_OPT_SIBLING_TOKEN:     
      cmd_node->isMand = FALSE;                      
      break;

   case CLI_GRP_SIBLING_TOKEN:                                 
      cmd_node->isMand = TRUE;               
      break;              

   case CLI_NO_RELATION:
      break;      

   default:
      break;
   }       
        
   if(CLI_NO_RELATION != i_token_relation)
   {   
      if(TRUE == parCli->par_cb.firstOptChild)
         parCli->par_cb.firstOptChild = FALSE;       
        
      if(TRUE == parCli->par_cb.firstGrpChild)
         parCli->par_cb.firstGrpChild = FALSE;       
        
      /* Assign the token name */         
      switch(i_token_type) {           
      case NCSCLI_KEYWORD:
           cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(yytext)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
            strcpy(cmd_node->tokName, yytext);                
         break;

      case NCSCLI_CONTINOUS_EXP:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_CONTINOUS_EXP)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_CONTINOUS_EXP);                
         break;    

      case NCSCLI_GROUP:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_GRP_NODE)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_GRP_NODE);
         break;

      case NCSCLI_OPTIONAL:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_OPT_NODE)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_OPT_NODE);
         break;      

      case NCSCLI_STRING:
           cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_STRING)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_STRING);
         break;

      case NCSCLI_NUMBER:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_NUMBER)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_NUMBER);
         break;

      case NCSCLI_CIDRv4:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_CIDRv4)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_CIDRv4);
         break;

      case NCSCLI_IPv4:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_IPADDRESSv4)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_IPADDRESSv4);
         break;

      case NCSCLI_IPv6:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_IPADDRESSv6)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_IPADDRESSv6);
         break;

      case NCSCLI_MASKv4:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_IPMASKv4)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_IPMASKv4);
         break;

      case NCSCLI_CIDRv6:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_CIDRv6)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_CIDRv6);
         break;                

      case NCSCLI_PASSWORD:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_PASSWORD)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_PASSWORD);
         break;

      case NCSCLI_COMMUNITY:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_COMMUNITY)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_COMMUNITY);
         break;  

      case NCSCLI_WILDCARD:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_WILDCARD)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_WILDCARD);
         break;

      case NCSCLI_MACADDR:
            cmd_node->tokName = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(CLI_MACADDR)+1);
            if(!cmd_node->tokName) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
         strcpy(cmd_node->tokName, CLI_MACADDR);
         break;

      default:
         break;
      }
        
        sprintf(parCli->par_cb.str, "OF LEVEL (%d)", i_token_level);     
      m_LOG_NCSCLI_COMMENTS(cmd_node->tokName, (TRUE == cmd_node->isMand)?
                     NCSCLI_PAR_MANDATORY_TOKEN:NCSCLI_PAR_OPTIONAL_TOKEN, 
                     parCli->par_cb.str);             
   }
        
   /* Add token into the command tree */
   cmd_node->tokRel = i_token_relation;
   parCli->par_cb.tokList[parCli->par_cb.tokCnt] = cmd_node;
   parCli->par_cb.tokCnt++;   
}

/*****************************************************************************
  Procedure NAME    :    yyerror
  DESCRIPTION        :    A function is called by the parser when it encounters any
                        parse error to notify the user specying the line no and 
                        the error token.                    
  ARGUMENTS            :    The error string that is to flushed to the user.
  RETURNS            :   none
  NOTES                :
*****************************************************************************/
void yyerror(int8 *text)
{
    if(strlen(yytext) != 0)
       m_NCS_CONS_PRINTF("Error on line %d at %s (%s)\n",yylineno, yytext, text);
}

/*****************************************************************************
  Procedure NAME    :    cli_pcb_set
  DESCRIPTION        :   A function is called by the parser when it encounters any
                        parse error to notify the user specying the line no and 
                        the error token.                    
  ARGUMENTS            :    Pointer to CLI control block
  RETURNS            :   none
  NOTES                :
*****************************************************************************/
void cli_pcb_set(CLI_CB *pCli)
{
    parCli = pCli;  
}

/*****************************************************************************
  PROCEDURE NAME    :    cli_set_token_attrib
  DESCRIPTION        :    The function assigns the diffrent attributes parsed by
                     the to the command node that is recently added into the
                     tree.        
                    
  ARGUMENTS            :    pCli - Pointer to CLI control block
                           i_node - Poinetr to the token node
                           i_token_attrib - Type of the attribute e.g. Help string,
                     range value,mode change etc.
                           i_value - Value of the attribute.          
  RETURNS            :  none
  NOTES                :    1. Scan the type of attribute.
                     2. covert the value of the attribute to there nessasery 
                        data type if required. e.g uns32, ipaddress etc.
                     3. Store the attribute in the corresponding value of 
                        the attribute in the node that is currently added
*****************************************************************************/
void cli_set_token_attrib(CLI_CB             *pCli, 
                            CLI_CMD_ELEMENT    *i_node, 
                          CLI_TOKEN_ATTRIB   i_token_attrib, 
                          int8               *i_value)
{
   uns32   token_len;
   int8    *token = 0;    

   if(!i_node) return;

   switch(i_token_attrib) {
   case CLI_HELP_STR:
      token = strtok(i_value, CLI_HELP_IDENTIFIER);

        i_node->helpStr = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(token)+1);
        if(!i_node->helpStr) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
      strcpy(i_node->helpStr, token);            
      break;    

   case CLI_DEFALUT_VALUE:
      token = strtok(i_value, CLI_DEFAULT_IDENTIFIER);

      if(NCSCLI_NUMBER == i_node->tokType) {
         i_node->defVal = (uns32 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(uns32));
         if(!i_node->defVal) return;
         *((uns32 *)i_node->defVal) = atoi(token);
      }
      else {
         token_len = strlen(token) + 1;
         i_node->defVal = (int8 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(int8) * token_len);
         if(!i_node->defVal) return;
         strcpy(((int8 *)i_node->defVal), token);
      }
      break;        

   case CLI_RANGE_VALUE:
       {
           int8    range_dilimeter[] = "<..>";           
            get_range_values(i_node, i_token_attrib, i_value, range_dilimeter);
        }
      break;        
    
    case CLI_CONTINOUS_RANGE:
        {
            int8    range_dilimeter[] = "(...)";   
            get_range_values(i_node, i_token_attrib, i_value, range_dilimeter);        
        }
        break;

   case CLI_MODE_CHANGE:
      token = strtok(i_value, CLI_MODE_IDENTIFIER);
   
      i_node->modChg = TRUE;
      i_node->nodePath = m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(token)+1);
        if(!i_node->nodePath) m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
      strcpy(i_node->nodePath, token);            
      break;
    
   case CLI_DO_FUNC:
      cli_set_cef(pCli);
      break;        
    
   default:
      break;
   }
}

static void
get_range_values(CLI_CMD_ELEMENT    *i_node, 
                 CLI_TOKEN_ATTRIB   i_token_attrib, 
                 int8               *i_value,
                 int8                *delimiter)
{
   int8    *token = 0;
   uns32   token_len = 0;    
   int       val = 0;        

   i_node->range = m_MMGR_ALLOC_CLI_RANGE;
   if(!i_node->range) return;

   memset(i_node->range, 0, sizeof(CLI_RANGE));
   token = strtok(i_value, delimiter);

   if(NCSCLI_NUMBER == i_node->tokType ||
   NCSCLI_CONTINOUS_EXP == i_node->tokType) {
      i_node->range->lLimit = (uns32 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(uns32));
   
      if(!i_node->range->lLimit) return;
      sscanf(token, "%u", &val);
      *((uns32 *)i_node->range->lLimit) = val;
   }
   else {
      token_len = strlen(token) + 1;
   
      i_node->range->lLimit = (int8 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(int8) * token_len);
      if(!i_node->range->lLimit) return;
      strcpy(((int8 *)i_node->range->lLimit), token);
   }

   while(0 != token) {
      token = strtok(0, delimiter);
   
      if(token) {               
         if(NCSCLI_NUMBER == i_node->tokType ||
            NCSCLI_CONTINOUS_EXP == i_node->tokType) {
            i_node->range->uLimit = (uns32 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(uns32));
            if(!i_node->range->uLimit) return;
            sscanf(token, "%u", &val);
            *((uns32 *)i_node->range->uLimit) = val;
         }
         else
         {
            token_len = strlen(token) + 1;
            i_node->range->uLimit = (int8 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(int8) * token_len);
            if(!i_node->range->uLimit) return;
            strcpy(((int8 *)i_node->range->uLimit), token);
         }
      }
   }
}
