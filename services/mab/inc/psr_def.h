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

  MODULE NAME: PSR_DEF.H

..............................................................................

  DESCRIPTION: Contains definitions for PSR definitions and constants.


******************************************************************************
*/

#ifndef PSR_DEF_H
#define PSR_DEF_H

#define NCS_PSS_MAX_PROFILE_NAME 256
#define NCS_PSS_CONF_FILE_NAME   256

#define m_PSS_LIB_CONF_FILE_NAME  "/etc/opt/opensaf/pssv_lib_conf"
#define m_PSS_SPCN_LIST_FILE_NAME "/etc/opt/opensaf/pssv_spcn_list"
#define NCS_PSS_DEF_PSSV_ROOT_PATH    "/var/opt/opensaf/pssv_store/"

typedef struct mab_pss_tbl_list_tag
{
    uns32       tbl_id;
    struct mab_pss_tbl_list_tag *next;
}MAB_PSS_TBL_LIST;

typedef struct mab_pss_client_list_tag
{
    char             *pcn;
    MAB_PSS_TBL_LIST *tbl_list;
}MAB_PSS_CLIENT_LIST;


#ifdef NCS_PSS_DEBUG
#if (NCS_PSS_DEBUG == 1)
#define m_PSS_DBG_PRINTF m_NCS_CONS_PRINTF
#else
#define m_PSS_DBG_PRINTF 
#endif
#else
#define NCS_PSS_DEBUG    0
#define m_PSS_DBG_PRINTF 
#endif

#endif
