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

  MODULE NAME: LMDSRUN.C
  
    ..............................................................................
    
      DESCRIPTION:
      
        Exercises LEAP MDS APIs.
        
          ******************************************************************************
          *
          * Get compile time options...
*/
#include "ncs_opt.h"

/** Global Declarations...
**/
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncssysf_def.h"
#include "ncssysfpool.h"
#include "ncs_regx.h"

/*****************************************************************************
Simple Object validation scheme loaded at regx->exists.
******************************************************************************/

#define NCS_REGX_EXISTS    0x44ab /* some pattern */

/*****************************************************************************
Internal flag constants used to mark Regx pattern rules captured at 
Regx compile time and referenced at match time.
******************************************************************************/

#define CBRA     2
#define CCHR     4
#define CDOT     8
#define CCL     12
#define CDOL    20
#define CCEOF   22
#define CKET    24
#define CBACK   36
#define CBOL    40      /* Beginning of line */
#define RSTAR    1
#define RPLUS   64      /* irritating numbering scheme */
#define RQUES   82      /* SMM added for '?' syntax    */
#define RNGE     3

/*****************************************************************************
Macros used within this Regx implementation only
******************************************************************************/

#define PLACE(c)         ep[c >> 3] |= bittab[c & 07]
#define ISTHERE(c)      (ep[c >> 3] & bittab[c & 07])
#define ecmp(s1, s2, n) (!m_NCS_STRNCMP(s1, s2, n))


#define GETC()          (*sp++)
#define INIT            register char *sp = instring;
#define UNGETC(c)       (--sp)
#define PEEKC()         (*sp)
#define GR_ERROR(n)     return(-n)
#define RETURN(c)       return(c - spp)

/*****************************************************************************
Static functions scoped just to this *.c file
******************************************************************************/

static void    getRange        ( NCS_REGX* regx, char* str);
static int     advance         ( NCS_REGX* regx, char* lp, char* ep);
static int     step            ( NCS_REGX* regx, char* p,  char* t, SUBS* subs);
static int     gMatch          ( NCS_REGX* regx, char* p,  char* t, SUBS* subs);
static int     compile         ( NCS_REGX* regx, char* instring, char* ep, char* endbuf, char seof);
static int     isOct           ( int      c);
static char*   patErrStr       ( int      status);
static void    resetSubsArray  ( NCS_REGX* regx);


char  bittab[BITTAB_DIM] = {(char)1,
(char)2,
(char)4,
(char)8,
(char)16,
(char)32,
(char)64,
(char)(-128)};  /* -128=0x80 = (unsigned char)(128). -128 avoid compiler warning */ 

/****************************************************************************
* 
*                          H J _ R E G X
* 
*          P R I V A T E   (S T A T I C)   F U N C T I O N S 
*
****************************************************************************/

/*****************************************************************************
Get Range private member function
******************************************************************************/
static void getRange( NCS_REGX* regx, char* str)
{
   regx->low = *str++ & 0377;
   regx->size = (((uns8)*str) == 255)? 20000: (*str &0377) - regx->low;
}

/*****************************************************************************
* Advance private member function
******************************************************************************/
static int advance(NCS_REGX* regx, char* lp, char* ep)
{
   register char* curlp;
   int            c;
   char*          bbeg;
   int            ct;
   
   for(;;) 
   {
      switch(*ep++) 
      {    
      case CCHR:
         if(*ep++ == *lp++)
            continue;
         return(0);
         
      case CDOT:
         if(*lp++)
            continue;
         return(0);
         
      case CDOL:
         if(*lp == 0)
            continue;
         return(0);
         
      case CCEOF:
         regx->loc2 = lp;
         return(1);
         
      case CCL:
         c = *lp++ & 0177;
         if(ISTHERE(c)) 
         {
            ep += 16;
            continue;
         }
         return(0);
         
      case CBRA:
         regx->braslist[(uns32)*ep++] = lp; /* uns32 type cast keeps linux compiler happy */
         continue;
         
      case CKET:
         regx->braelist[(uns32)*ep++] = lp;
         continue;
         
      case CCHR | RNGE:
         c = *ep++;
         getRange(regx, ep);
         while(regx->low--)
            if(*lp++ != c)
               return(0);
            curlp = lp;
            while(regx->size--) 
               if(*lp++ != c)
                  break;
               if(regx->size < 0)
                  lp++;
               ep += 2;
               goto star;
               
      case CDOT | RNGE:
         getRange(regx, ep);
         while(regx->low--)
            if(*lp++ == '\0')
               return(0);
            curlp = lp;
            while(regx->size--)
               if(*lp++ == '\0')
                  break;
               if(regx->size < 0)
                  lp++;
               ep += 2;
               goto star;
               
      case CCL | RNGE:
         getRange(regx, ep + 16);
         while(regx->low--) 
         {
            c = *lp++ & 0177;
            if(!ISTHERE(c))
               return(0);
         }
         curlp = lp;
         while(regx->size--) 
         {
            c = *lp++ & 0177;
            if(!ISTHERE(c))
               break;
         }
         if(regx->size < 0)
            lp++;
         ep += 18;       /* 16 + 2 */
         goto star;
         
      case CBACK:
         bbeg = regx->braslist[(uns32)*ep];
         ct = regx->braelist[(uns32)*ep++] - bbeg;
         if(ecmp(bbeg, lp, ct)) 
         {
            lp += ct;
            continue;
         }
         return(0);
         
         /* The RQUES cases  SMM Added these       */
         
      case CBACK | RQUES: /* SMM seems to work */
         bbeg = regx->braslist[(uns32)*ep];
         ct = regx->braelist[(uns32)*ep++] - bbeg;
         curlp = lp;
         if (ecmp(bbeg, lp, ct))
         {
            lp += ct;
            if (advance(regx,lp,ep))
               return 1;
            lp -= ct;
         }
         return 0;  
         
      case CDOT | RQUES: /* SMM seems to work !! NO LOOK-AHEAD !! */
         if ( *lp++)      
            continue;
         return 0;
         
      case CCHR | RQUES:  /* SMM seems to work */
         if(*ep++ == *lp++)
            continue;
         lp--;
         continue;
         
      case CCL | RQUES:  /* SMM seems to work */
         curlp = lp;
         c = *lp++ & 0177;
         if(ISTHERE(c))
         {
            ep += 16;
            continue;
         }
         ep += 16;
         lp--;
         continue;
         
         /* The RPLUS cases */
         
      case CBACK | RPLUS:
         bbeg = regx->braslist[(uns32)*ep];
         ct = regx->braelist[(uns32)*ep++] - bbeg;
         if (ecmp(bbeg, lp, ct))
            lp += ct;
         else
            return(0);
         curlp = lp;
         while(ecmp(bbeg, lp, ct))
            lp += ct;
         while(lp >= curlp) 
         {
            if(advance(regx, lp, ep)) return(1);
            lp -= ct;
         }
         return(0);
         
      case CDOT | RPLUS:
         if ( ! *lp++)
            return(0);
         curlp = lp;
         while(*lp++)
            ;
         goto star;
         
      case CCHR | RPLUS:
         if ( ! ( *lp++ == *ep))
            return(0);
         curlp = lp;
         while(*lp++ == *ep);
         ep++;
         goto star;
         
      case CCL | RPLUS:
         c = *lp++ & 0177;
         if( ! ISTHERE(c))
            return(0);
         curlp = lp;
         do {
            c = *lp++ & 0177;
         } while(ISTHERE(c));
         ep += 16;
         goto star;
         
         /* The RSTAR cases */
         
      case CBACK | RSTAR:
         bbeg = regx->braslist[(uns32)*ep];
         ct = regx->braelist[(uns32)*ep++] - bbeg;
         curlp = lp;
         while(ecmp(bbeg, lp, ct))
            lp += ct;
         
         while(lp >= curlp) 
         {
            if(advance(regx, lp, ep)) return(1);
            lp -= ct;
         }
         return(0);
         
      case CDOT | RSTAR:
         curlp = lp;
         while(*lp++);
         goto star;
         
      case CCHR | RSTAR:
         curlp = lp;
         while(*lp++ == *ep);
         ep++;
         goto star;
         
      case CCL | RSTAR:
         curlp = lp;
         do {
            c = *lp++ & 0177;
         } while(ISTHERE(c));
         ep += 16;
         goto star;
         
star:
         do {
            if(--lp == regx->locs)
               break;
            if(advance(regx, lp, ep))
               return(1);
         } while(lp > curlp);
         return(0);
      }
   }
}

/*****************************************************************************
* Step private member function
*****************************************************************************/

static int step(NCS_REGX* regx, char* p1, char* p2, SUBS* subs)
{  
   register int c;
   int      n;
   
   regx->locs = p1 - 1; /* backed one before beginning of line for 'break' comparison. */
   
   for ( n = 0; n < NBRA; n++) {
      regx->braslist[n] = regx->braelist[n] = (char *) 0;
      if ( subs) {
         subs[n].s_cp = (char *) 0;
         subs[n].s_len = 0;
      }
   }
   if( *p2 == CBOL) {
      regx->loc1 = p1;
      if ( advance(regx, p1, ++p2))
         goto success;
      else
         return(0);
   }
   /* fast check for first character */
   if(*p2 == CCHR) {
      c = p2[1];
      do {
         if(*p1 != c)
            continue;
         if(advance(regx, p1, p2)) {
            regx->loc1 = p1;
            goto success;
         }
      } while(*p1++);
      return(0);
   }
   /* regular algorithm */
   do {
      if(advance(regx, p1, p2)) {
         regx->loc1 = p1;
         goto success;
      }
   } while(*p1++);
   return(0);
   
success:
   if ( subs) 
   {
      subs[0].s_cp  = regx->loc1;
      subs[0].s_len = regx->loc2 - regx->loc1;
      for ( n = 0; regx->braslist[n]; n++) 
      {
         subs[ n + 1].s_cp  = regx->braslist[n];
         subs[ n + 1].s_len = regx->braelist[n] - regx->braslist[n];
      }
   }
   else
      for ( n = 0; regx->braslist[n]; n++)
         ;
      return( n + 1);
}

/*****************************************************************************
* GMatch private member function
*****************************************************************************/

static int gMatch ( NCS_REGX* regx, char* p, char* t, SUBS* subs)
{
   return( step( regx, t, p, subs));
}

/*****************************************************************************
* IsOct private member function
*****************************************************************************/

static int isOct ( int c)
{
   return( '0' <= c && c <= '7');
}

/*****************************************************************************
* compiler private member function
*****************************************************************************/

static int compile(NCS_REGX* regx, 
                   char*    instring, 
                   char*    ep,   
                   char*    endbuf, 
                   char     seof)
{
   /*    register char *cp; */
   register int c;
   register int eof = seof;
   char*    lastep          /* = instring */;  /* changed from original (dsr) */
   int      cclcnt;
   char     bracket[NBRA];
   char*    bracketp;
   char     ebracket[NBRA];    /* dsr */
   int      bc;                /* bracket close */
   char*    spp;
   int      closed;
   int      neg;
   int      lc;
   int      i, cflg;
   
   INIT
      lastep = 0;
   spp = ep;
   if((c = GETC()) == eof || c == '\n') 
   {
      if(c == '\n') 
      {
         UNGETC(c);
         regx->nodelim = 1;
      }
      /*      if(*ep == 0 && !sed) GR_ERROR(41); RETURN(ep); */
      if ( *ep == 0)
         GR_ERROR(41);
   }
   for ( i = 0; i < NBRA; i++)
      ebracket[ i] = '\0';
   bracketp = bracket;
   closed = regx->nbra = regx->ebra = 0;
   if(c == '^') 
   {
      if ( ep >= endbuf)
         GR_ERROR(50);
      else
         *ep++ = CBOL;
   }
   else
      UNGETC(c);
   for(;;) 
   {
      if(ep >= endbuf)
         GR_ERROR(50);
      c = GETC();
      if( c != '*' && c != '|' && c != '+' && c != '?') /* SMM added '?' */
         lastep = ep;
      if(c == eof) 
      {
         *ep++ = CCEOF;
         RETURN(ep);
      }
      switch(c) 
      {
      case '.':
         *ep++ = CDOT;
         continue;
         
      case '\n':      /* removed: if ( sed ) GR_ERROR(36); */
         /* UNGETC(c); */    /* changed from original (dsr) */
         *ep++ = CCEOF;
         regx->nodelim = 1;
         RETURN(ep);
         
      case '*':
         if(lastep == 0 || *lastep == CBRA || *lastep == CKET)
            goto defchar;
         *lastep |= RSTAR;
         continue;
         
      case '+':
         if(lastep == 0 || *lastep == CBRA || *lastep == CKET)
            goto defchar;
         *lastep |= RPLUS;
         continue;
         
      case '?':
         if(lastep == 0 || *lastep == CBRA || *lastep == CKET)
            goto defchar;
         *lastep |= RQUES; /* SMM */
         continue;
         
      case '$':
         if(PEEKC() != eof && PEEKC() != '\n')
            goto defchar;
         *ep++ = CDOL;
         continue;
         
      case '{':
         if(regx->nbra >= NBRA)
            GR_ERROR(43);
         *bracketp++ = (char)regx->nbra; /* SMM cast */
         *ep++ = CBRA;
         *ep++ = (char)regx->nbra++;     /* SMM cast */
         continue;
         
      case '}':
         if(bracketp <= bracket ||  regx->nbra <= 0)
            GR_ERROR(42);
         for ( bc = regx->nbra - 1; bc && ebracket[ bc]; --bc)
            ;
         if ( ! ebracket[ bc])
            ebracket[ bc]++;
         else
            GR_ERROR( 42);
         *ep++ = CKET;
         *ep++ = (char)bc;  /* previously: *ep++ = *--bracketp;   */
         closed++;
         continue;
         
      case '|':             /* pattern width */
         if(lastep == (char *) 0)
            goto defchar;
         *lastep |= RNGE;
         cflg = 0;
nlim:
         c = GETC();
         i = 0;
         do {
            if('0' <= c && c <= '9')
               i = 10 * i + c - '0';
            else
               GR_ERROR(16);
         } while(((c = GETC()) != '|') && (c != ','));
         if(i > 255)
            GR_ERROR(11);
         *ep++ = (char)i; /* SMM added cast */
         if(c == ',') 
         {
            if(cflg++)
               GR_ERROR(44);
            if((c = GETC()) == '|')
               *ep++ = (char)(-1); /* -1 = 255. "-1" avoids VStudio compiler warning */
            else 
            {
               UNGETC(c);
               goto nlim;
               /* get 2'nd number */
            }
         }
         if( c != '|')
            GR_ERROR(45);
         if(!cflg)   /* one number */
            *ep++ = (char)i;
         else if((ep[-1] & 0377) < (ep[-2] & 0377))
            GR_ERROR(46);
         continue;
         
      case '[':
         if(&ep[17] >= endbuf)
            GR_ERROR(50);
         
         *ep++ = CCL;
         lc = 0;
         for(i = 0; i < 16; i++)
            ep[i] = 0;
         
         neg = 0;
         if((c = GETC()) == '^') 
         {
            neg = 1;
            c = GETC();
         }
         
         do 
         {
            if ( c == '\\') 
            {
               if ( isOct( sp[0]) && isOct( sp[1]) && isOct( sp[2]))
               {
                  for ( c = i = 0; i <= 2; i++)
                     c = ( c << 3) + GETC() - '0';
               }
               else
                  c = GETC(); /* SMM If not octal, treat as escape for next char */
            }
            if (c == '\0' /* || c == '\n' (dsr, jan. 12, 1990) */)
               GR_ERROR(49);
            if(c == '-' && lc != 0)
            {
               if((c = GETC()) == ']')
               {
                  PLACE('-');
                  break;
               }
               if ( c == '\\') 
               {
                  if ( isOct( sp[0]) && isOct( sp[1]) && isOct( sp[2]))
                  {
                     for ( c = i = 0; i <= 2; i++)
                        c = ( c << 3) + GETC() - '0';
                  }
                  else
                     c = GETC(); /* SMM If not octal, treat as escape for next char */
               }
               if ( c == '\0')
                  GR_ERROR(49);
               while(lc < c)
               {
                  PLACE(lc);
                  lc++;
               }
            }
            lc = c;
            PLACE(c);
         } while((c = GETC()) != ']');
         if(neg) 
         {
            for(cclcnt = 0; cclcnt < 16; cclcnt++)
               ep[cclcnt] ^= -1;
            ep[0] &= 0376;
         }
         
         ep += 16;
         
         continue;
         
      case '\\':
         switch(c = GETC()) 
         {
         case '\n':
            GR_ERROR(36);
            
         case 'n':
            c = '\n';
            goto defchar;
            
         case 'f':
            c = '\f';
            goto defchar;
            
         case 'r':
            c = '\r';
            goto defchar;
            
         default:
            if ( isOct(c) && isOct( sp[0]) && isOct( sp[1])) /* SMM added */
            {
               UNGETC(c); /* SMM just put it back so for loop looks like others */
               for ( c = i = 0; i <= 2; i++)
                  c = ( c << 3) + GETC() - '0';
               goto defchar;
            }
            
            if(c >= '1' && c <= '9') 
            {
               if((c -= '1') >= closed)
                  GR_ERROR(25);
               *ep++ = CBACK;
               *ep++ = (char)c;
               continue;
            }
         }
         /* Drop through to default to use \ to turn off special chars */
         
defchar:
         default:
            lastep = ep;
            *ep++ = CCHR;
            *ep++ = (char)c; /* SMM added cast */
      }
   }
}



/*****************************************************************************
* PatErrStatus private member function
*****************************************************************************/

static char* patErrStr (int status)
{
   switch( status)
   {
   case -11:
      return( "Range endpoint too large.");
      
   case -16:
      return( "Bad number.");
      
   case -25:
      return( "digit out of range.");                  /* SMM was =>"``\digit'' out of range." */
      
   case -36:
      return( "Illegal or missing delimiter.");
      
   case -41:
      return( "No remembered search string.");
      
   case -42:
      return( "( ) imbalance.");                       /* SMM was =>"\(~\) imbalance." */
      
   case -43:
      return( "Too many (.");                          /* SMM was =>"Too many \(." */
      
   case -44:
      return( "More than 2 numbers given in { }.");    /* SMM was =>"More than 2 numbers given in \{~\}." */
      
   case -45:
      return( "} expected after .");                   /* SMM was =>"} expected after \." */ 
      
   case -46:
      return( "First number exceeds second in { }.");  /* SMM was =>"First number exceeds second in \{~\}." */
      
   case -49:
      return( "[ ] imbalance.");
      
   case -50:
      return( "Regular expression overflow.");
      
   default:
      return( "PatErrStatus(): unknown error: ");
   }
}


/*****************************************************************************
* This method resets all contained sub-strings to NULL. 
*****************************************************************************/

void resetSubsArray (NCS_REGX* regx )
{
   uns32 i;
   for ( i = 0; i < NBRA; i++)
   {
      regx->subsp[i].s_cp = NULL;
      regx->subsp[i].s_len = 0;
   }
   return;
}


/****************************************************************************
* 
*                               H J _ R E G X
* 
*                        P U B L I C     F U N C T I O N S 
*
****************************************************************************/


/****************************************************************************
* Function Name: ncs_regx_init
* Purpose:   Create an NCS_REGX instance and put it in start state. 
*            Then validate that the passed pattern (a regular expression)
*            is valid by 'compiling' it into quasi-machine code.
*           
*            Reasons for failure (NULL return) include:
*              - HEAP out of memory
*              - a pattern longer than 300 characters
*              - Pattern does not compile (criptic error msg given)
****************************************************************************/

void* ncs_regx_init(char* pattern)
{
   NCS_REGX* regx = m_MMGR_ALLOC_REGX;
   int32    ret  = 0;
   
   m_NCS_ASSERT( strlen(pattern) < (T_PATTERN_SIZE - 1));
   
   if (regx == NULL)
   {
      m_LEAP_DBG_SINK(NULL);
      return NULL;
   }
   
   /* set up the Results stuff   resLen = NBRA; */
   
   regx->subsp  = (SUBS*)&regx->subspace;
   regx->exists = NCS_REGX_EXISTS;
   resetSubsArray(regx);
   
   /* set up the pattern holding stuff */
   regx->t_alist.t_pattern[0] = '\0';
   regx->t_alist.c_pattern[0] = '\0';
   
   if(( ret = compile( regx,
      pattern, 
      regx->t_alist.c_pattern,
      regx->t_alist.c_pattern + sizeof(regx->t_alist.c_pattern),
      '\0')) < 0)
   {
      m_NCS_CONS_PRINTF(" NCS_REGX:%s  COMPILING: %s\n",patErrStr(ret), pattern);
      m_MMGR_FREE_REGX(regx);
      return NULL;
   }
   
   m_NCS_STRCPY( regx->t_alist.t_pattern, pattern);
   return (void*) regx;
}

/****************************************************************************
* Function Name: ncs_regx_destroy
* Purpose:   Recover all resources associated with this REGX.
****************************************************************************/

uns32 ncs_regx_destroy(void* regx_hdl)
{
   NCS_REGX* regx = (NCS_REGX*) regx_hdl;
   m_NCS_ASSERT(regx->exists == NCS_REGX_EXISTS);
   
   m_MMGR_FREE_REGX(regx);
   
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
* Function Name: ncs_regx_match
* Purpose:   This function attempts to match the src against all registered
*            patterns. If a successful match occurs then the internal
*            result buffer is loaded.                                   
*            The Regx attempts to match the patterns in the order they
*            are registered, meaning (probably) more specific patterns
*            first.                                                        
****************************************************************************/

uns32 ncs_regx_match(void* regx_hdl, char*  src)
{
   NCS_REGX* regx = (NCS_REGX*) regx_hdl;
   m_NCS_ASSERT(regx->exists == NCS_REGX_EXISTS);
   
   resetSubsArray(regx);   /* prepare for loading new stuff */
   if (gMatch(regx, regx->t_alist.c_pattern, src, regx->subsp))
      return NCSCC_RC_SUCCESS;
   return NCSCC_RC_FAILURE;
}

/****************************************************************************
* Function Name: ncs_regx_get_count
* Purpose:   This function returns the number of collected substrings 
*            currently stored in the Regx Result buffer.
****************************************************************************/

uns32 ncs_regx_get_count(void* regx_hdl)
{
   NCS_REGX* regx = (NCS_REGX*) regx_hdl;
   uns32    i    = 0;
   
   m_NCS_ASSERT(regx->exists == NCS_REGX_EXISTS);
   
   for (i = 0; regx->subsp[i].s_cp; i++)
      ;
   return i;
}

/****************************************************************************
* Function Name: ncs_regx_get_result
* Purpose:   This function finds the stored substring by index and copies
*            the matched substring into the passed buffer and then NULL 
*            terminates the string.
*            
* Note       As long as there has been a successful match, the 0 slot 
*            contains the entire string sequence matched. 
*            If the regular expression used embedded '{}' syntax, then
*            there will be sub-string match results as well in other
*            registers.
****************************************************************************/

NCS_BOOL ncs_regx_get_result(void* regx_hdl, uns32 idx, char* space, uns32 len)
{
   NCS_REGX* regx = (NCS_REGX*) regx_hdl;
   
   m_NCS_ASSERT(regx->exists == NCS_REGX_EXISTS);
   m_NCS_ASSERT(idx < NBRA);
   m_NCS_ASSERT(strlen(regx->subsp[idx].s_cp) < len);
   
   m_NCS_MEMCPY(space, regx->subsp[idx].s_cp, regx->subsp[idx].s_len);
   space[regx->subsp[idx].s_len] = '\0';
   
   return TRUE;
}

/****************************************************************************
* Function Name: ncs_regx_get_length
* Purpose:   This function returns the length of the string-portion matched.
****************************************************************************/

uns32 ncs_regx_get_length(void* regx_hdl)
{
   NCS_REGX* regx = (NCS_REGX*) regx_hdl;
   m_NCS_ASSERT(regx->exists == NCS_REGX_EXISTS);
   
   return regx->subsp[0].s_len;
}






