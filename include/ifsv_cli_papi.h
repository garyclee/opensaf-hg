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

  MODULE NAME: IFSV_CLI_PAPI.H

..............................................................................

  DESCRIPTION: Contains definitions for IFSv CLI interface.


******************************************************************************
*/

#ifndef IFSV_CLI_PAPI_H
#define IFSV_CLI_PAPI_H

#include "ncsgl_defs.h"
#include "ifsv_papi.h"
#include "ncs_mib_pub.h"
#include "ncs_iplib.h"

#ifdef  __cplusplus
extern "C" {
#endif

/******************************************************************************
 * IFSV - CLI Mode Data 
 *****************************************************************************/
typedef struct ifsv_cli_mode_data
{
   NCS_IFSV_IFINDEX  ifindex;
   NCS_IFSV_SPT      spt;
} NCS_IFSV_CLI_MODE_DATA;


/* API to get IF index for a given Shelf/slot/port/type This API is used by the
   CEFs of other subsystems 
*/
uns32 ncs_ifsv_cli_get_ippfx_from_spt(uns32            i_mib_key,
                                     uns32            i_usr_key,
                                     NCSMIB_REQ_FNC   reqfnc,
                                     NCS_IFSV_SPT     *i_spt, 
                                     NCS_IPPFX        **o_ippfx, 
                                     uns32            *pfx_cnt);

uns32 ncs_ifsv_cli_get_ifindex_from_spt(uns32            i_mib_key, 
                                       uns32             i_usr_key,  
                                       NCSMIB_REQ_FNC   reqfnc, 
                                       NCS_IFSV_SPT     *i_spt, 
                                       NCS_IFSV_IFINDEX *o_ifindex);
#ifdef  __cplusplus
}
#endif

#endif

   
