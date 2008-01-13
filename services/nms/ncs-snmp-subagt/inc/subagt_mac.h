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

  
  .....................................................................  
  
  DESCRIPTION: This file includes the files that the NCS SNMP SubAgent is 
               dependent on. 
*****************************************************************************/ 
#ifndef SUBAGT_MAC_H
#define SUBAGT_MAC_H

/* Temporary macros... may come from SE API of the SUbagent itself...  TBD */
#define SUBAGT_VCARD_ID  1
#define SUBAGT_POOL_ID   1

/* Following Macro definition here is looking dirty.. needs an alternative */
/* You can find the GET macro in the subagt_mab.h file */
/* subagt_mab.h file need not have this Macro */
EXTERN_C uns32 g_subagt_mac_hdl;

/* to set the MAC Handle */
#define     m_SUBAGT_MAC_HDL_SET(hdl)\
            g_subagt_mac_hdl = hdl
            
EXTERN_C uns32
snmpsubagt_mac_initialize(void); 

EXTERN_C uns32
snmpsubagt_mac_finalize(void);

EXTERN_C NCSCONTEXT gl_mds_hdl;

#endif

