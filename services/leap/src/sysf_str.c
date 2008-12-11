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

..............................................................................

  DESCRIPTION:

  This module contains routines related H&J Counting Semaphores.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  ncs_sem_create....create/initialize a semaphore
  ncs_sem_release...release a semaphore
  ncs_sem_give......increment the semaphore by 1
  ncs_sem_take......wait for semaphore to be greater than 0

 ******************************************************************************
 */


#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncssysf_def.h"


/* Parse S into tokens separated by characters in DELIM.
   If S is NULL, the last string strtok() was called with is
   used.
*/
int8 * sysf_strtok( int8 *s, const int8 *delim)
{
    static int8 *olds = 0;
    int8 *token;

    if (s == 0)
    {
        if (olds == 0)
        {
            return 0;
        } else
            s = olds;
    }

    /* Scan leading delimiters.  */
    s += strspn((char *)s, (const char *)delim);
    if (*s == '\0')
    {
        olds = 0;
        return 0;
    }

    /* Find the end of the token.  */
    token = s;
    s = strpbrk((char *)token, (const char *)delim);
    if (s == 0)
    {
        /* This token finishes the string.  */
        olds = 0;
    }
    else
    {
        /* Terminate the token and make OLDS point past it.  */
        *s = '\0';
        olds = s + 1;
    }
    return token;
}


int8 *storeme = 0; /* for 'backwards compatability' */

int8 * sysf_strtok_r(int8 * s, const int8 * delim, int8 ** save_ptr)
{
    int8 *token;

    token = 0;      /* Initialize to no token. */
#if DEBUG_INFO
    printf("sysf_strtok_r: s == %s\n", s);
    /*printf("sysf_strtok_r: delim == %s\n", delim);*/
    printf("sysf_strtok_r: save_ptr == %s\n", *save_ptr);
#endif
    if (s == 0)    /* If not first time called... */
    {
        s = *save_ptr; /* restart from where we left off. */
    }

    if (s != 0)    /* If not finished... */
    {
        *save_ptr = 0;

        s += strspn((char *)s, (const char *)delim);  /* Skip past any leading delimiters. */
        if (*s != '\0')         /* We have a token. */
        {
            token = s;
            *save_ptr = strpbrk((char *)token, (const char *)delim); /* Find token's end. */
            if (*save_ptr != 0)
            {
                /* Terminate the token and make SAVE_PTR point past it.  */
                *(*save_ptr)++ = '\0';
            }
        }
    }
#if DEBUG_INFO
    printf("sysf_strtok_r: s == %s\n", s);
    /*printf("sysf_strtok_r: delim == %s\n", delim);*/
    printf("sysf_strtok_r: save_ptr == %s\n", *save_ptr);
#endif
    return token;
}



/****************************************************************************
  PROCEDURE NAME:   sysf_strrcspn

  DESCRIPTION:
     finds the delimiter, left of the start positions in the string.
     See also strcspn, strspn.

  RETURNS:
     index into string where delimiter is left of start position.
*****************************************************************************/
int32
sysf_strrcspn(const uns8 *s, const int32 start_pos, const uns8 *reject)
{
    int32 i;
    int32 j;
    uns32 rej_len = m_NCS_STRLEN((char*)reject);

    for(i=(int32)start_pos; i>=0; i--)
    {
        for(j=0; j<(int32)rej_len; j++)
        {
            if(s[i] == reject[j])
            {
                return i;
            }
        }
    }
    return EOF;
}



/****************************************************************************
  PROCEDURE NAME:   sysf_strincmp

  DESCRIPTION:
  case insensitive string compare, but no more than to n characters.
  See also stricmp and strncmp

  RETURNS:
     index into string where delimiter is left of start position.
*****************************************************************************/
int32
sysf_strincmp(const uns8 *s1, const uns8 *s2, uns32 n)
{
    uns8 c1 = '\0';
    uns8 c2 = '\0';

    while (n > 0) 
    {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;

        if('a' <= c1 && 'z' >= c1)
        {
            c1 -= 'a' - 'A';
        }

        if('a' <= c2 && 'z' >= c2)
        {
            c2 -= 'a' - 'A';
        }

        if (c1 == '\0' || c1 != c2)
        {
            return c1 - c2;
        }
        n--;
    }

    return c1 - c2;
}



