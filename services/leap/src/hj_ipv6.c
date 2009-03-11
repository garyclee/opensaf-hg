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

  MODULE NAME: NCS_IPV6.C


..............................................................................

  DESCRIPTION: Contain library functions for handling IPv6 addresses. 

******************************************************************************
*/

#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncssysf_def.h"



#if (NCS_IPV6 == 1)

NCS_BOOL ncs_is_ipv6_addr_all_zero(NCS_IPV6_ADDR *addr)
{
    NCS_IPV6_ADDR lcl_addr;
    
    memset(&lcl_addr, '\0', sizeof(NCS_IPV6_ADDR));
    if(memcmp((char *)addr, (char *)&lcl_addr, sizeof(NCS_IPV6_ADDR)) 
        == 0)
    {
        return TRUE;
    }
    return FALSE;
}

void ncs_mask_prefix_addr(NCS_IPV6_ADDR *addr, uns32 pfxlen,
                          NCS_IPV6_ADDR *o_masked_addr)
{
    uns8 byte_to_be_masked = 0, bits_to_be_zeroed = 0;
    uns8 mask;

    if(o_masked_addr == NULL)
        return;

    if (pfxlen == 0)
    {
       memset(o_masked_addr, 0, sizeof(*o_masked_addr));
       return;
    }

    memcpy((char *)o_masked_addr, (char *)addr, sizeof(NCS_IPV6_ADDR));

    if (pfxlen >= 128)
    {
       return;
    }

    /* Mask the hostorder address, using pfxlen. */
       
    /* Byte_to_be_masked 
       Take examples to derive formula:-
       For mask-len 1 to 8, apply mask on 0th byte,
       For mask-len 9 to 16, apply mask on 1st byte,
       ...
       So the formula for the offset of the byte to be masked is 
       (pfxlen-1)/8 (or equivalent)...
    */
    byte_to_be_masked = (uns8)(   (pfxlen-1)>>3  );
    
    /* Bits to be zeroed. Take examples to derive formula
       For mask-len 1, mask in BINARY is (1000 0000), on 0th byte
       For mask-len 2, mask in BINARY is (1100 0000), on 0th byte
       ...
       For mask-len 8, mask in BINARY is (1111 1111), on 0th byte
       ....
       For mask-len 121, mask in BINARY is (1000 0000), on 15th byte
       ...
       So the formula for bits to be skipped (# of 0s in mask) is 
       (7 - (pfxlen-1)%8) (or equivalent) ...
    */
    bits_to_be_zeroed = (uns8)(7 - ((pfxlen-1)&0x7));
    mask = (uns8)(0xff << bits_to_be_zeroed);

    /* Apply the mask */
    o_masked_addr->ipv6.ipv6_addr8[byte_to_be_masked] &= mask;

    /* Reset the remaining bytes */
    if ((15 - byte_to_be_masked) > 0)
    {
       /* Bytes with the following offsets need to be reset 
          byte_to_be_masked + 1, 
          byte_to_be_masked + 2,
          ...
          15
          which makes it a total of (15 - byte_to_be_masked) bytes
       */
       memset(&o_masked_addr->ipv6.ipv6_addr8[byte_to_be_masked+1], 
          0, 15 - byte_to_be_masked);
    }

    return;
}

NCS_BOOL ncs_is_ipv6_linkloc_addr (NCS_IPV6_ADDR *addr)
{
   if((addr->ipv6.ipv6_addr8[0] == 0xfe) &&
      ((uns8)(addr->ipv6.ipv6_addr8[1] & 0xC0) == (uns8)0x80))
   {
      return TRUE;
   }
   return FALSE;
}

NCS_BOOL ncs_is_ipv6_6to4_addr (NCS_IPV6_ADDR *addr)
{
   if(((uns8)((uns8)addr->ipv6.ipv6_addr8[0] & (uns8)0xff) == (uns8)0x20) &&
      ((uns8)((uns8)addr->ipv6.ipv6_addr8[1] & (uns8)0xfe) == (uns8)0x02))
   {
      return TRUE;
   }
   return FALSE;
}

NCS_BOOL ncs_is_valid_ipv6_uc(NCS_IPV6_ADDR *addr, uns8 bits)
{
   if (bits > 128)
      return FALSE;

   if((uns8)addr->ipv6.ipv6_addr8[0] == (uns8)0xff)
   {
      /* This is the multicast address, return failure */
      return FALSE;
   }

   if((addr->ipv6.ipv6_addr32[0] == 0) && (addr->ipv6.ipv6_addr32[1] == 0) &&
      (addr->ipv6.ipv6_addr32[2] == 0) && (addr->ipv6.ipv6_addr16[6] == 0) &&
      (addr->ipv6.ipv6_addr8[14] == 0))
   {
      if(addr->ipv6.ipv6_addr8[15] == 0)
      {  
         /* This is unspecified address, return FALSE */
         return FALSE;
      }
      else if(addr->ipv6.ipv6_addr8[15] == 1)
      {
         /* This is loop back address return FALSE */
         return FALSE;
      }
   }

   /* Every thing else is the link-local/site-local/global unicast address */
   return TRUE;
}

void ncs_str_to_ipv6addr(NCS_IPV6_ADDR *o_v6addr, int8* i_str)
{
   int8    *delimiter = ":";
   int8    buffer[IPV6_DISPLAY_STR_MAX_SIZE];
   int8    *token = 0, idx = 0;
   uns32   val = 0;
   uns8    dlmt_cnt=0, dbl_dlmt_pos = 8,i=0, str_len;
   uns32   dlmt_flag = FALSE;

   /* Traverse the string and find the delimiter count 
   & double delimiter position */
   str_len = (uns8)strlen(i_str);

   if((i_str[0] == ':') && (i_str[1] == ':'))
   {
      /* Double delimiter at the begining */
      dbl_dlmt_pos = 0;
      dlmt_cnt++;
      i=2;
   }

   for(; i<str_len; i++)
   {
      if(i_str[i] == ':')
      {
         if(dlmt_flag == FALSE)
         {
            dlmt_cnt++;
            dlmt_flag = TRUE;
         }
         else
            dbl_dlmt_pos = dlmt_cnt;
      }
      else
         dlmt_flag = FALSE;
   }

   /* Adjust the dlmt_cnt if the double delimiter is at the end */
   if(i_str[i-1] == ':')
      dlmt_cnt--;
   
   /* Adjust the dlmt_cnt if the double delimiter is at the begin */
   if((dbl_dlmt_pos == 0) && (dlmt_cnt > 0))
       dlmt_cnt--;

   /* Fill the arg_val */
   memset(buffer, 0, sizeof(buffer));
   strcpy(buffer, i_str);

   token = strtok(buffer, delimiter);

   /* Read the value from token */
   if(token)
      sscanf(token, "%x", &val);
   else
      val = 0;

   val = (uns16)m_NCS_OS_HTONS((uns16)val);

   if(dbl_dlmt_pos == idx)
   {
      /* Fill the array with the zeros */
      for(i=0; i < (NCS_IPV6_ADDR_UNS16_CNT-(dlmt_cnt+1)); i++)
         o_v6addr->ipv6.ipv6_addr16[idx++] = 0;
   }
   o_v6addr->ipv6.ipv6_addr16[idx++] = (uns16)val;
   while(0 != token)
   {
      token = strtok(0, delimiter);
      if(token)
      {
         sscanf(token, "%x", &val);
         val = (uns16)m_NCS_OS_HTONS((uns16)val);
         if(dbl_dlmt_pos == idx)
         {
            /* Fill the array with the zeros */
            for(i=0; i < (NCS_IPV6_ADDR_UNS16_CNT-(dlmt_cnt+1)); i++)
               o_v6addr->ipv6.ipv6_addr16[idx++] = 0;
         }
         o_v6addr->ipv6.ipv6_addr16[idx++] = (uns16)val;
      }
      else
      {
         /* Fill the rest of the array with zeros */   
         if(dbl_dlmt_pos == idx)
         {
            /* Fill the array with the zeros */
            for(i=0; i < (NCS_IPV6_ADDR_UNS16_CNT-(dlmt_cnt+1)); i++)
               o_v6addr->ipv6.ipv6_addr16[idx++] = 0;
         }
      }
   }
   return;
}

void ncs_str_to_ipv6pfx(NCS_IPV6_ADDR *o_v6addr, 
                               uns32 *o_pfx_len, int8* str)
{
   int8    *delimiter = "/";
   int8    *token = 0;
   uns32   val = 0;
   uns8    pfx_len = 0, idx=0, bit_mask = 0xff;

   token = strtok(str, delimiter);
   token = strtok(0, delimiter);

   /* Read the pfx_len value from token */
   if(token)
      sscanf(token, "%d", &val);
   pfx_len = (uns8) val;

   ncs_str_to_ipv6addr(o_v6addr, str);

   /* Fill the prefix length value in "*o_pfx_len" */
   *o_pfx_len = pfx_len;

   /* Find number of unwanted full octets */
   idx = (uns8)(pfx_len/8);

   /* Fill the unwanted bits with zeros */
   if(pfx_len%8)
   {
      bit_mask = (uns8)(bit_mask << (8-pfx_len%8));
      o_v6addr->ipv6.ipv6_addr8[idx]
         = (uns8) (o_v6addr->ipv6.ipv6_addr8[idx] & bit_mask);
      idx++;
   }

   /* Fill the Unwanted bytes with zeros */
   while(idx < NCS_IPV6_ADDR_UNS8_CNT)
   {
      o_v6addr->ipv6.ipv6_addr8[idx] = 0;
      idx++;
   }
   return;
}

void ncs_ipv6_str_from_ipv6addr(NCS_IPV6_ADDR *i_v6addr, int8 *o_str)
{
   uns8  i;
   NCS_IPV6_ADDR temp_addr;

   /* Convert the address in to host order format */
   temp_addr = *i_v6addr;

   for(i=0; i<NCS_IPV6_ADDR_UNS16_CNT; i++)
   {
      temp_addr.ipv6.ipv6_addr16[i] = 
         (uns16)m_NCS_OS_NTOHS((uns16)temp_addr.ipv6.ipv6_addr16[i]);
   }

   sysf_sprintf(o_str,"%x:%x:%x:%x:%x:%x:%x:%x",temp_addr.ipv6.ipv6_addr16[0],
      temp_addr.ipv6.ipv6_addr16[1], temp_addr.ipv6.ipv6_addr16[2], 
      temp_addr.ipv6.ipv6_addr16[3], temp_addr.ipv6.ipv6_addr16[4], 
      temp_addr.ipv6.ipv6_addr16[5], temp_addr.ipv6.ipv6_addr16[6], 
      temp_addr.ipv6.ipv6_addr16[7]);

   return;
}

#else   /* #if (NCS_IPV6 == 1) */
extern int dummy; /* To avoid compiler warning for an empty file */
#endif  /* #if (NCS_IPV6 == 1) */
