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

  MODULE NAME:  NCS_IPV6.H

..............................................................................

  DESCRIPTION: Contains common IPV4 definitions, and utilities

******************************************************************************
*/

#ifndef _NCS_IPV4_H
#define _NCS_IPV4_H

#ifdef  __cplusplus
extern "C" {
#endif

/* Macros */

/* NOTE:  Macro should be only used for values in the range 0-32. */
#define m_NCS_IPV4ADDR_MASK_FROM_LEN(x)   ((x)?0xffffffff << (32 - (x)):0)

/* m_LEN_IPV4_MASK_LEN_P : This macro counts the number of bits in an
   IPV4 address MASK. This macro will have to be used when "m" is a pointer to 
   memory which is, possibly, not word aligned.

   m = pointer to the mask. The mask should be in NETWORK ORDER.
   l = this will be set to the length 
*/
#define m_NCS_IPV4_MASK_LEN_P(l,m) \
  { l =0; while (l < 32 && (uns8)(((uns8*)(m))[l>>3] <<(l%8))) l++; }

/* m_LEN_IPV4_MASK_LEN : This macro counts the number of bits in an
   IPV4 address MASK.    

   m = An uns32 mask. The mask must be an ordinary variable (host ordered)
   l = this will be set to the length.

   The check on m&0x01 is to prevent l becoming 32.
*/
#define m_NCS_IPV4_MASK_LEN(l,m)    \
        { l = 0;                    \
          if ((m)&0x1)              \
               l = 32;              \
          else                      \
               while((m)<<l) l++;   \
        }

#define m_NCS_COMP_IPV4_ADDR_N(a,b)  memcmp((void *)a, (void *)b, 4);

/* This macro can be used to determine a multicast ip address */
#define m_NCS_IPV4_IS_MULTICAST(i)  (i>=0xE0000000)?((i<=0xEFFFFFFF)?1:0):0

#ifdef  __cplusplus
}
#endif

#endif   /* _NCS_IPV4_H */
