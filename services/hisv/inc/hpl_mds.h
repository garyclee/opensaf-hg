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

  This file contains declarations of routines used by HPL library for MDS
  interaction in order to communicate with HCD.

******************************************************************************/

#ifndef HPL_MDS_H
#define HPL_MDS_H

#define HPL_MDS_SYNC_TIMEOUT  1500
#define HPL_MDS_SYNC_TIMEOUT_HP_PROLIANT  5000
/*
 * Wrapper structure that encapsulates communication
 * semantics for communication with HAM
 */

/*****************************************************************************
                 Macros to fill the MDS message structure
*****************************************************************************/

/* Macro to populate the 'HPL command, entity path' in MDS message */
#define m_HPL_HISV_ENTITY_MSG_FILL(m, act, param, len, data_ptr) \
do { \
   (m).info.api_info.cmd = (act); \
   (m).info.api_info.arg = (param); \
   (m).info.api_info.data_len = (len); \
   (m).info.api_info.data = (uns8 *)data_ptr;\
} while (0);

/* Macro to populate the 'HPL command to clear SEL' in MDS message */
#define m_HPL_HISV_LOG_CMD_MSG_FILL(m, act) \
do { \
   memset(&(m), 0, sizeof(HISV_MSG)); \
   (m).info.api_info.cmd = (act); \
   (m).info.api_info.data_len = 0; \
   (m).info.api_info.data = NULL; \
} while (0);

#define HPL_MDS_SUB_PART_VERSION 1

#define HPL_HCD_SUB_PART_VER_MIN  1
#define HPL_HCD_SUB_PART_VER_MAX  1

/*** Extern function declarations ***/

EXTERN_C uns32 hpl_mds_initialize(HPL_CB *hpl_cb);
EXTERN_C uns32 hpl_mds_finalize(HPL_CB *hpl_cb);
EXTERN_C uns32 hpl_mds_msg_async_send(HPL_CB *cb, HISV_MSG *i_msg,
                                      MDS_DEST *ham_dest, uns32 prio);
EXTERN_C HISV_MSG* hpl_mds_msg_sync_send(HPL_CB *cb, HISV_MSG *i_msg,
                                      MDS_DEST *ham_dest, uns32 prio, uns32 timeout);
EXTERN_C uns32 get_ham_dest (HPL_CB *hpl_cb, MDS_DEST *ham_dest,
                             uns32 chassis_d);

#endif /* !HPL_MDS_H */
