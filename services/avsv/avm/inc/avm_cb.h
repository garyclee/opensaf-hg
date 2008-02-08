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
 
  DESCRIPTION:This is file contains the structure dfinition for AvM Ccontrol  
  Block. 
  ..............................................................................

 
******************************************************************************
*/



#ifndef __AVM_CB_H__
#define __AVM_CB_H__

typedef struct async_updt_cnt_type
{
   uns32 ent_cfg_updt;
   uns32 ent_updt;
   uns32 ent_sensor_updt;
   uns32 ent_dep_updt;
   uns32 ent_adm_op_updt;
   uns32 evt_id_updt;
   uns32 ent_dhconf_updt;
   uns32 ent_dhstate_updt;
   uns32 hlt_status_updt;
   uns32 ent_upgd_state_updt;
}AVM_ASYNC_CNT_T;

typedef enum 
{
   NO_MODULE = 1,
   IPMC_PLD,
   IPMC_PLD_BLD,
   IPMC_PLD_RTM,
   NCS_BIOS,
   ALL_MODULE
}AVM_UPGD_MODULE;

#if 0 /* remove later - JPL */
typedef enum
{
  IPMC_SAME_VERSION,
  UPGRADE_SUCCESS,
  ROLLBACK_TRIGGERED
}AVM_UPGD_PRG; 
#endif

struct avm_cb
{
   SYSF_MBX                mailbox;
   /* mail box required by SSU for accumulating inspending events */
   SYSF_MBX                ssu_mbx;
   /*The Control block handle */
   uns32                   cb_hdl;   
   NCSCONTEXT              task_hdl;

   /*The set of selection objects on which AvM does select*/
   NCS_SEL_OBJ_SET         sel_obj_set;

   /* The highest selction object */ 
   NCS_SEL_OBJ             mbx_sel_obj;   

   /*The selection object returned by EdSv*/
   SaSelectionObjectT      eda_sel_obj;

   /* The highest selction object */ 
   NCS_SEL_OBJ             sel_high;   
   
   /*MDS handle returned during initialization*/
   MDS_HDL                 mds_hdl;

   /*The pwe handle returned when adest is created*/
   MDS_HDL                 adest_pwe_hdl;

   /*VDEST handle*/
   MDS_HDL                 vaddr_hdl;

   /*The pwe handle returned when vdest is created*/
   MDS_HDL                 vaddr_pwe_hdl;

 
   MDS_DEST                vaddr;
   MDS_DEST                adest;   
   
        /*The EDSv handle returned after EDSv initialization*/
   SaEvtHandleT            eda_hdl;
   
   SaNameT                 event_chan_name;
   /*EDSv Channels for subscribing Fault and Non-Fault events*/
   SaEvtChannelHandleT     event_chan_hdl;

   SaEvtEventFilterArrayT   fault_filter;
   SaEvtEventFilterArrayT   non_fault_filter;

   /* Variables to handle trap channnel */
   SaNameT                 trap_chan_name;
   SaEvtChannelHandleT     trap_chan_hdl;
   SaNameT                 publisherName;
   NCS_BOOL                trap_channel;

   /*The MAB handle returned after the initialization*/
   uns32                    mab_hdl;
   uns32                    mab_row_hdls[(NCSMIB_TBL_AVM_END - NCSMIB_TBL_AVM_BASE)+1];
   NCS_LOCK                 lock;

   AVM_TMR_T                eda_init_tmr;
   AVM_TMR_T                ssu_tmr;
   uns32                    eda_init;  

   EDU_HDL                  edu_hdl;
   NCS_MBCSV_HDL            mbc_hdl;
   uns32                    ckpt_hdl;
   SaSelectionObjectT       mbc_sel_obj;
   SaVersionT               avm_ver; 
   
   AVM_DB_T                 db;      
   AVM_ASYNC_CNT_T          async_updt_cnt;

   AVM_CONFIG_STATE_T       cfg_state;
   AVM_EVT_Q_T              evt_q;
   SaAmfHAStateT            ha_state;
   AVM_HA_TRANSIT_STATE_T   ha_transit_state;
   NCS_BOOL                 cold_sync;
   uns32                    stby_in_sync;
   
   uns32                    adm_switch;
   /* Flag to mark that during admin switchover RDE sent an async standby role*/
   NCS_BOOL                 adm_async_role_rcv; 
   AVM_CONFIG_STATE_T       config_state;
   AVM_VALID_INFO_STATE_T   valid_info_state;  
   uns32                    config_cnt;
   uns32                    dummy_parent_cnt;
   HEALTH_STATUS            hlt_status[HEALTH_STATUS_MAX];
   NCS_BOOL                 is_platform;
   AVM_TMR_T                dhcp_fail_tmr;  /* Timer for handling dhcp script failure */
   AVM_UPGD_ERR             upgrade_error_type;
   AVM_UPGD_MODULE          upgrade_module;
   AVM_UPGD_PRG             upgrade_prg_evt;
};

#define AVM_CB_NULL ((AVM_CB_T *)0x0)

#endif /*__AVM_CB_H__*/
