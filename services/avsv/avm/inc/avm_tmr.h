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

  This module is the include file for Availability Directors timer module.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVM_TMR_H
#define AVM_TMR_H

/* leap timers are 10ms timers so divide with 10000000 */
#define AVM_NANOSEC_TO_LEAPTM 10000000

/* 1 second */
#define AVM_INIT_EDA_INTVL ((SaTimeT)100)

/* accumulate insertion pending events for 3 seconds */
#define AVM_SSU_TMR_INTVL  (700)

/* wait for blade boot-up for 16 Minutes (??) */
#define AVM_UPGD_SUCC_TMR_INTVL  (96000)
#define AVM_BOOT_SUCC_TMR_INTVL  (48000)
#define AVM_DHCP_FAIL_TMR_INTVL  (6000)    
#define AVM_BIOS_UPGRADE_TMR_INTVL  (15000) /*(48000)*/
#define AVM_BIOS_FAIL_TMR_INTVL  (18000)

/* IPMC upgrade takes around 4-5 mins                              */
/* First value below takes care of both ipmc/rtm ipmc upgrade time */
/* Second value is only for either ipmc or rtm ipmc upgrade        */
#define AVM_UPGD_IPMC_PLD_TMR_INTVL (60000) 
#define AVM_UPGD_IPMC_PLD_MOD_TMR_INTVL (30000) 


typedef enum avm_tmr_status_type
{
   AVM_TMR_RUNNING = 1,
   AVM_TMR_STOPPED,
   AVM_TMR_NOT_STARTED,
   AVM_TMR_EXPIRED,
   AVM_TMR_INVALID
}AVM_TMR_STATUS_T;
/* vivek_avm */
/* timer type enums */
typedef enum avm_tmr_type
{
   AVM_TMR_INIT_EDA = 1,          /* EDA LIB REQ timer  */ 
   AVM_TMR_SSU,               /* timer used by SSU */
   AVM_UPGD_SUCCESS,          /* Upgrade success timer */
   AVM_BOOT_SUCCESS,          /* Boot Success Timer */
   AVM_DHCP_FAIL,             /* Dhcp script Fail Timer */
   AVM_BIOS_UPGRADE,          /* Bios boot up timer */
   AVM_UPGD_IPMC,             /* IPMC Upgrade Timer */
   AVM_UPGD_IPMC_MOD,         /* IPMC Single Module Upgrade Timer */
   AVM_ROLE_CHG_WAIT,         /* When the avm changes the role and if the IPMC upgrade in progress, wait for the FUND to get freed */
   AVM_BIOS_FAIL,             /* After failover, avm starts the timer, to wait for openhpi initialization                          */
   AVM_TMR_MAX 
} AVM_TMR_TYPE_T;


/* AVM Timer definition */
struct avm_tmr
{   
   tmr_t                  tmr_id;   
   AVM_TMR_TYPE_T         type; 
   uns32                  cb_hdl;      /* cb hdl to retrieve the AvM cb ptr */
   uns32                  ent_hdl;     /* Entity hdl to retrieve the Entity Info. 
                                          Will be used only for Upgrade Timer */
   AVM_TMR_STATUS_T       status;
};

typedef struct avm_time
{
   uns32 seconds;
   uns32 milliseconds; 
}AVM_TIME_T;

 /* 
  * macro to start the EDA LIB REQ  timer. The cb structure
  * is the input.
 */

#define m_AVM_INIT_EDA_TMR_START(cb) \
{\
   cb->eda_init_tmr.cb_hdl = cb->cb_hdl; \
   cb->eda_init_tmr.type   = AVM_TMR_INIT_EDA; \
   avm_start_tmr(cb, &(cb->eda_init_tmr), AVM_INIT_EDA_INTVL); \
}

/* Macro to start SSU timer */
#define m_AVM_SSU_TMR_START(cb) \
{\
   cb->ssu_tmr.cb_hdl = cb->cb_hdl; \
   cb->ssu_tmr.type   = AVM_TMR_SSU; \
   avm_start_tmr(cb, &(cb->ssu_tmr), AVM_SSU_TMR_INTVL); \
}

/* Macro to Upgrade Success SSU timer */
#define m_AVM_SSU_UPGR_TMR_START(cb, ent_info) \
{\
   ent_info->upgd_succ_tmr.cb_hdl = cb->cb_hdl; \
   ent_info->upgd_succ_tmr.ent_hdl = ent_info->ent_hdl; \
   ent_info->upgd_succ_tmr.type   = AVM_UPGD_SUCCESS; \
   avm_start_tmr(cb, &ent_info->upgd_succ_tmr, AVM_UPGD_SUCC_TMR_INTVL); \
}

/* Macro to Boot Success SSU timer */
#define m_AVM_SSU_BOOT_TMR_START(cb, ent_info) \
{\
   ent_info->boot_succ_tmr.cb_hdl = cb->cb_hdl; \
   ent_info->boot_succ_tmr.ent_hdl = ent_info->ent_hdl; \
   ent_info->boot_succ_tmr.type   = AVM_BOOT_SUCCESS; \
   avm_start_tmr(cb, &ent_info->boot_succ_tmr, AVM_BOOT_SUCC_TMR_INTVL); \
}

/* Macro to Dhcp Script Fail SSU timer */
#define m_AVM_SSU_DHCP_FAIL_TMR_START(cb) \
{\
   cb->dhcp_fail_tmr.cb_hdl = cb->cb_hdl; \
   cb->dhcp_fail_tmr.type   = AVM_DHCP_FAIL; \
   avm_start_tmr(cb, &cb->dhcp_fail_tmr, AVM_DHCP_FAIL_TMR_INTVL); \
}

/* Macro to start Bios Upgrade Timer */
#define m_AVM_SSU_BIOS_UPGRADE_TMR_START(cb, ent_info) \
{\
   ent_info->bios_upgrade_tmr.cb_hdl = cb->cb_hdl; \
   ent_info->bios_upgrade_tmr.ent_hdl = ent_info->ent_hdl; \
   ent_info->bios_upgrade_tmr.type   = AVM_BIOS_UPGRADE; \
   avm_start_tmr(cb, &ent_info->bios_upgrade_tmr, AVM_BIOS_UPGRADE_TMR_INTVL); \
}

/* Macro to start Upgrade Payload IPMC timer */
#define m_AVM_UPGD_IPMC_PLD_TMR_START(cb, ent_info) \
{\
   ent_info->ipmc_tmr.cb_hdl = cb->cb_hdl; \
   ent_info->ipmc_tmr.ent_hdl = ent_info->ent_hdl; \
   ent_info->ipmc_tmr.type   = AVM_UPGD_IPMC; \
   avm_start_tmr(cb, &ent_info->ipmc_tmr, AVM_UPGD_IPMC_PLD_TMR_INTVL); \
}

/* Macro to start Upgrade Payload IPMC Single Module timer */
#define m_AVM_UPGD_IPMC_PLD_MOD_TMR_START(cb, ent_info) \
{\
   ent_info->ipmc_mod_tmr.cb_hdl = cb->cb_hdl; \
   ent_info->ipmc_mod_tmr.ent_hdl = ent_info->ent_hdl; \
   ent_info->ipmc_mod_tmr.type   = AVM_UPGD_IPMC_MOD; \
   avm_start_tmr(cb, &ent_info->ipmc_mod_tmr, AVM_UPGD_IPMC_PLD_MOD_TMR_INTVL); \
}

/* Macro to start ROLE_CHG_WAIT timer */
#define m_AVM_ROLE_CHG_WAIT_TMR_START(cb, ent_info, intvl) \
{\
   ent_info->role_chg_wait_tmr.cb_hdl = cb->cb_hdl; \
   ent_info->role_chg_wait_tmr.ent_hdl = ent_info->ent_hdl; \
   ent_info->role_chg_wait_tmr.type   = AVM_ROLE_CHG_WAIT; \
   avm_start_tmr(cb, &ent_info->role_chg_wait_tmr, intvl); \
}

#define m_AVM_SSU_BIOS_FAILOVER_TMR_START(avm_cb, ent_info) \
{\
   ent_info->bios_failover_tmr.cb_hdl = avm_cb->cb_hdl; \
   ent_info->bios_failover_tmr.ent_hdl = ent_info->ent_hdl; \
   ent_info->bios_failover_tmr.type   = AVM_BIOS_FAIL; \
   avm_start_tmr(avm_cb, &ent_info->bios_failover_tmr, AVM_BIOS_FAIL_TMR_INTVL); \
}
/*** Extern function declarations ***/

EXTERN_C void avm_tmr_exp(void *);

EXTERN_C uns32 avm_start_tmr(
                              AVM_CB_T *, 
                              AVM_TMR_T *,  
                              SaTimeT
                            );

EXTERN_C void avm_stop_tmr(
                              AVM_CB_T*, 
                              AVM_TMR_T*
                          );

#endif
