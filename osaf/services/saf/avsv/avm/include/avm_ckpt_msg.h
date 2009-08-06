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

  This include file contains the message definitions for chepointing data from 
  Active to Standby AvM.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVM_CKPT_MSG_H
#define AVM_CKPT_MSG_H

/* REO types fro Async updates */
typedef enum avm_ckpt_msg_reo_type
{
   AVM_CKPT_ENT_CFG,
   AVM_CKPT_ENT_DHCP_CONF_CHG,
   AVM_CKPT_ENT_DHCP_STATE_CHG,
   AVM_CKPT_ENT_STATE,
   AVM_CKPT_ENT_STATE_SENSOR,
   AVM_CKPT_ENT_ADD_DEPENDENT,
   AVM_CKPT_ENT_ADM_OP,
   AVM_CKPT_EVT_ID,
   AVM_CKPT_EVT_DUMMY,
   AVM_CKPT_ENT_UPGD_STATE_CHG,
   AVM_CKPT_MSG_MAX
}AVM_CKPT_MSG_REO_TYPE;

/* REO types for cold synd response */
typedef enum avm_ckpt_cold_sync_msg_reo_type
{
   AVM_CKPT_COLD_SYNC_VALIDATION_INFO,
   AVM_CKPT_COLD_SYNC_ENT_INFO,
   AVM_CKPT_COLD_SYNC_ASYNC_UPDT_CNT,
   AVM_CKPT_COLD_SYNC_HLT_STATUS_DUMMY,
   AVM_CKPT_COLD_SYNC_MSG_MAX
} AVM_COLD_SYNC_MSG_REO_TYPE;

/* Macros for Async updates with diff MBCSv send options */

#define m_AVM_SEND_CKPT_UPDT_ASYNC_ADD(cb, r_hdl, r_type) \
              avm_send_ckpt_data(cb, NCS_MBCSV_ACT_ADD, NCS_PTR_TO_UNS64_CAST(r_hdl), r_type, NCS_MBCSV_SND_USR_ASYNC)

#define m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(cb, r_hdl, r_type) \
              avm_send_ckpt_data(cb, NCS_MBCSV_ACT_UPDATE, NCS_PTR_TO_UNS64_CAST(r_hdl), r_type, NCS_MBCSV_SND_USR_ASYNC)

#define m_AVM_SEND_CKPT_UPDT_ASYNC_RMV(cb, r_hdl, r_type) \
              avm_send_ckpt_data(cb, NCS_MBCSV_ACT_RMV, NCS_PTR_TO_UNS64_CAST(r_hdl), r_type, NCS_MBCSV_SND_USR_ASYNC)

#define m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, r_hdl, r_type) \
              avm_send_ckpt_data(cb, NCS_MBCSV_ACT_UPDATE, NCS_PTR_TO_UNS64_CAST(r_hdl), r_type, NCS_MBCSV_SND_SYNC)

#endif
