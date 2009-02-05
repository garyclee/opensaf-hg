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

  DESCRIPTION:This file defines a common structure for AvM  events.
  ..............................................................................


******************************************************************************
*/


#ifndef __AVM_EVT_H__
#define __AVM_EVT_H__

typedef enum avm_rcv_svs
{
   AVM_EVT_HPI,
   AVM_EVT_AVD,
   AVM_EVT_BAM,
   AVM_EVT_MIB,
   AVM_EVT_AVM,
   AVM_EVT_RDE,
   AVM_EVT_MDS,
   AVM_EVT_FUND,

   /* Only put timer events after this. All other events should be before this */
   AVM_TMR_EVTS_START,
   AVM_EVT_TMR = AVM_TMR_EVTS_START,
   AVM_EVT_SSU,
   AVM_EVT_UPGD_SUCC,
   AVM_EVT_BOOT_SUCC,
   AVM_EVT_DHCP_FAIL,
   AVM_EVT_BIOS_UPGRADE,
   AVM_EVT_IPMC_UPGD,
   AVM_EVT_IPMC_MOD_UPGD,
   AVM_EVT_ROLE_CHG_WAIT,
   AVM_EVT_BIOS_FAILOVER,
}AVM_RECEIVE_SVCS;

typedef enum avm_role_msgs_type
{
   AVM_ROLE_CHG = 0,
   AVM_ROLE_RSP 
}AVM_ROLE_MSG_TYPE_T;

typedef struct avm_role_chg_type
{
   SaAmfHAStateT role;
}AVM_ROLE_CHG_T;

typedef struct avm_role_rsp_type
{
   SaAmfHAStateT role;
   uns32           role_rsp;
}AVM_ROLE_RSP_T;

typedef struct avm_role_msg_type
{
   AVM_ROLE_MSG_TYPE_T msg_type;
   union
   {
      AVM_ROLE_CHG_T role_chg;
      AVM_ROLE_RSP_T role_rsp;
   }role_info;
}AVM_ROLE_MSG_T;

typedef struct hpi_evt_type
{
   SaHpiEventT            hpi_event;
   SaHpiEntityPathT       entity_path;
   HISV_INV_DATA          inv_data;
   uns32                  version;      /* versioning; see the macros defined in hpl_msg.h */
}HPI_EVT_T;


typedef struct rde_msg_type
{
   SaAmfHAStateT      role;
   PCSRDA_RETURN_CODE ret_code;
}RDE_MSG_T;

typedef struct avm_evt_id_type
{
   uns32             evt_id;
   AVM_RECEIVE_SVCS  src;
}AVM_EVT_ID_T;

#define AVM_EVT_ID_NULL ((AVM_EVT_ID_T *)0x0)

struct avm_evt
{
    NCS_IPC_MSG         next;

   /*struct avm_evt_type *next;*/

   AVM_RECEIVE_SVCS   src;
   AVM_FSM_EVT_TYPE_T fsm_evt_type;
   uns32              data_len;

   union event
   {
      HPI_EVT_T       *hpi_evt;
      AVD_AVM_MSG_T   *avd_evt;
      BAM_AVM_MSG_T   *bam_evt;
      NCSMIB_ARG      *mib_req;
      AVM_ROLE_MSG_T   avm_role_msg;
      RDE_MSG_T        rde_evt;
      AVM_TMR_T       tmr;
   }evt;
   AVM_EVT_ID_T        ssu_evt_id;
};

#define AVM_EVT_NULL ((AVM_EVT_T *)0)

typedef struct evt_q_node_type
{
   uns32                     evt_id;
   NCS_BOOL                  is_proc; 
   AVM_EVT_T                 *evt;
   struct evt_q_node_type    *next;
}AVM_EVT_Q_NODE_T;

#define AVM_EVT_Q_NODE_NULL ((AVM_EVT_Q_NODE_T *)0x0)

typedef struct avm_evt_q_type
{
   AVM_EVT_Q_NODE_T *front;
   AVM_EVT_Q_NODE_T *rear;
   AVM_EVT_Q_NODE_T *evt_q;   
}AVM_EVT_Q_T;

#define AVM_EVT_Q_NULL ((AVM_EVT_Q_T *)0x0)

#endif /* __NCS_AVM_EVT_H__ */
