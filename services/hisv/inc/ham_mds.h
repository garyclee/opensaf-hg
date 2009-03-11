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

  This file contains declarations of routines used by ham library for MDS
  interaction in order to receive requests from HPI clients.
 *****************************************************************************
*/

#ifndef HAM_MDS_H
#define HAM_MDS_H

/*****************************************************************************
                 Macros to fill the MDS message structure
*****************************************************************************/

/* Macro to populate the HAM MDS message to send chassis_id to HPL */

#define m_HAM_HISV_CHASSIS_ID_FILL(m, id, vdest, stat) \
do { \
   m_NCS_MEMSET(&(m), 0, sizeof(HISV_MSG)); \
   (m).info.cbk_info.hpl_ret.h_vdest.chassis_id = (id); \
   (m).info.cbk_info.hpl_ret.h_vdest.ham_dest = (vdest); \
   (m).info.cbk_info.hpl_ret.h_vdest.ham_status = (stat); \
} while (0);

#define m_HAM_HISV_ENTITY_MSG_FILL(m, act, param, len, data_ptr) \
do { \
   (m).info.api_info.cmd = (act); \
   (m).info.api_info.arg = (param); \
   (m).info.api_info.data_len = (len); \
   memcpy((m).info.api_info.data, (data_ptr), (len)); \
} while (0);

#define m_HAM_HISV_LOG_CMD_MSG_FILL(m, act) \
do { \
   m_NCS_MEMSET(&(m), 0, sizeof(HISV_MSG)); \
   (m).info.api_info.cmd = (act); \
   (m).info.api_info.data_len = 0; \
   (m).info.api_info.data = NULL; \
} while (0);

#define HCD_MDS_SUB_PART_VERSION 1

#define HCD_HPL_SUB_PART_VER_MIN  1
#define HCD_HPL_SUB_PART_VER_MAX  1

/*** Extern function declarations ***/
EXTERN_C uns32 ham_mds_initialize(HAM_CB *ham_cb);
EXTERN_C uns32 ham_mds_finalize(HAM_CB *ham_cb);
EXTERN_C uns32 ham_mds_msg_send (HAM_CB  *cb, HISV_MSG  *msg, MDS_DEST *dest,
                      MDS_SEND_PRIORITY_TYPE prio, uns32 send_type, HISV_EVT *evt);

#endif /* !HAM_MDS_H */
