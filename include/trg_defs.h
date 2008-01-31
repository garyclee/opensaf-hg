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
   MODULE NAME:  TRG_DEFS.H

   DESCRIPTION:

   THIS FILE WILL BE INCLUDED BY OS_DEFS.H 
   IF THE END USER DEFINES "USE_TARGET_SYSTEM_TYPEDEFS" non zero

   IN THIS FILE THE END USER SHOULD SUPPLY THE DEFINES WHICH WILL RESOLVE TO 
   THE END USER SYSTEM'S PRIMITIVE TYPES
******************************************************************************/
#ifndef __TRG_DEFS_H__
#define __TRG_DEFS_H__

#ifdef  __cplusplus
extern "C" {
#endif

/* The following are the required typedefs and defines the end user has to supply. */

typedef unsigned char     uns8;       /*  8-bit */
typedef unsigned short    uns16;      /* 16-bit */
typedef unsigned int      uns32;      /* 32-bit */

typedef signed   char     int8;
typedef signed   short    int16;
typedef signed   long     int32;             
 
/** support for double is dependent upon the target system and/or C-compiler.
 ** If your system does NOT support double, change this to uns32.
 **/
typedef double DOUBLE;


typedef float             ncsfloat32;

typedef uns16             NCS_VRID;    /* Virtual Router ID */

typedef unsigned short    cfgflag;    /* Usually a YES/NO or T/F boolean */
typedef unsigned short    cfgenum;    /* An enumerated value */

typedef unsigned int      BOOLEAN;
typedef unsigned int      NCS_BOOL;    /* move to this solves BOOLEAN problem */
typedef unsigned char     bcd;        /* Binary-Coded-Decimal */

typedef void *            NCSCONTEXT;  /* opaque context between svc-usr/svc-provider */

typedef uns32             IE_DESC[2]; /* IE Descriptor for ATM Signallng */
typedef uns32             FIE_DESC;   /* IE Descriptor for FR Signalling.*/

typedef uns32             ncs_oid;  /* Basic data type for SNMP/ILMI Object Sub-ids...*/

typedef uns32 NCS_IPV4_ADDR;

typedef enum ncs_ip_addr_type
{
  NCS_IP_ADDR_TYPE_NONE,
  NCS_IP_ADDR_TYPE_IPV4,
  NCS_IP_ADDR_TYPE_IPV6,  /* Do not use (yet!) */

  NCS_IP_ADDR_TYPE_MAX    /* Must be last. */
} NCS_IP_ADDR_TYPE;

typedef struct ncs_ip_addr
{
  NCS_IP_ADDR_TYPE type;
  union
    {
      NCS_IPV4_ADDR v4;
      /* Some day a v6 structure will go here. */
    } info;
} NCS_IP_ADDR;


/* prototype for registered function to 'Probe' protocol values */

typedef void (*PROBER)(uns32 dir_flag, void *,  const void *);

#ifdef  __cplusplus
}
#endif

#endif /* __TRG_DEFS_H__ */
