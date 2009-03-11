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

  DESCRIPTION:This file defines structures and function definitions for
              DHCP server configuration supervisor.
  ..............................................................................


******************************************************************************
*/

#ifndef __AVM_DHCP_CONF_H__
#define __AVM_DHCP_CONF_H__

/* macros for base MAC addresses for payload blades */
#define AVM_MAX_MAC_ENTRIES          02
#define AVM_MAC_DATA_LEN             06

#define AVM_HOST_NAME_LENGTH         50
#define AVM_NAME_STR_LENGTH          256
#define AVM_TIME_STR_LEN             8
#define AVM_LOG_STR_MAX_LEN          500

#define AVM_HPI_ENT_HIR_LVL          3
#define AVM_SSU_DHCONF_SCRIPT   "/opt/opensaf/controller/scripts/ncs_ssu_dhconf.pl"
#define AVM_DHCPD_SCRIPT        "/etc/init.d/dhcp"
#define AVM_DHCPD_CONF          "/etc/dhcpd.conf"
#define AVM_DHCPD_SW_VER_FILE   "version"
#define AVM_DHCP_SCRIPT_ARG     " stop"
#define AVM_SCRIPT_BUF_LEN      2048
#define AVM_IP_ADDR_STR_LEN     18
#define AVM_DHCP_CONF_CHANGE    3
#define AVM_DHCPD_SW_PXE_FILE             "/pxelinux.0"
#define AVM_DHCPD_SW_IPMC_PLD_BLD_FILE    "/atca-7221-ipmc.fri"
#define AVM_DHCPD_SW_IPMC_PLD_RTM_FILE1   "/artm-7221-fc.fri"
#define AVM_DHCPD_SW_IPMC_PLD_RTM_FILE2   "/artm-7221-scsi.fri"
#define CONF_FILE_PATH "/repl/ssuHelper.conf"

#define AVM_DHCONF_MEMCMP_LEN(str1, str2, len1, len2)   (!(((len1) == (len2)) && \
                                                        (!memcmp((str1), (str2), (len2))))) 

/* enum for current default label and active label */
typedef enum {
   AVM_DEFAULT_LABEL_NULL,
   AVM_DEFAULT_LABEL_1,
   AVM_DEFAULT_LABEL_2
} AVM_DEFAULT_LABEL_NUM;

typedef enum {
   AVM_CUR_ACTIVE_LABEL_NULL,
   AVM_CUR_ACTIVE_LABEL_1,
   AVM_CUR_ACTIVE_LABEL_2
} AVM_CURRENT_ACTIVE_LABEL_NUM;


/* SSU Label status */
typedef enum {
    SSU_NO_CONFIG = 0,
    SSU_INSTALLED = 1,
    SSU_COMMIT_PENDING = 2,
    SSU_COMMITTED  = 3,
    SSU_UPGD_FAILED = 4,
    SSU_VALIDATED = 5
} AVM_SSU_LABEL_STATUS;

typedef enum {
    AVM_DHCP_COMMIT = 0,
    AVM_DHCP_UPGRADE
} AVM_DHCP_ADM_OPER;

typedef struct dt_tm
{
   uns16 year;
   uns8  month;
   uns8  day;
   uns8  hrs;
   uns8  min;
   uns8  sec;
}DT_TM;

typedef union date_time
{
  DT_TM                dttm;
  uns8                 install_time[8]; /* software installation time */
}DATE_TIME;

typedef struct avm_dhcp_conf_name_type
{
   uns8   name[AVM_NAME_STR_LENGTH];
   uns32  length;
}AVM_DHCP_CONF_NAME_TYPE;

/* Per label configuration information */
typedef struct avm_per_label_conf
{
   AVM_DHCP_CONF_NAME_TYPE   name;        /* Label name */
   AVM_DHCP_CONF_NAME_TYPE   file_name;   /* File name to be used for DHCP configuration */
   AVM_SSU_LABEL_STATUS      status;      /* Label software status */
   AVM_DHCP_CONF_NAME_TYPE   sw_version;  /* Software version */
   DATE_TIME                 install_time;   /* Installation time */

   /* Pointer to other label */
   struct avm_per_label_conf *other_label;

   NCS_BOOL                   conf_chg;    /* Lable configuration changed*/
}AVM_PER_LABEL_CONF;

/* Upgrade type */
typedef enum {
    SW_BIOS = 0,
    INTEG,
    IPMC
} AVM_UPGRADE;

/*IPMC upgrade state */
typedef enum {
    IPMC_UPGD_TRIGGERED = 1,
    IPMC_BLD_LOCKED,
    IPMC_UPGD_IN_PRG,
    IPMC_ROLLBACK_IN_PRG,
    IPMC_PLD_BLD_ROLLBACK_IN_PRG,
    IPMC_PLD_RTM_ROLLBACK_IN_PRG,
    IPMC_UPGD_DONE
}AVM_IPMC_UPGD_STATE;

/* BIOS upgrade state */
typedef enum {
    BIOS_TMR_STARTED = 1,
    BIOS_TMR_STOPPED,
    BIOS_STOP_BANK_0,
    BIOS_STOP_BANK_1,
    BIOS_TMR_EXPIRED,
    BIOS_EXP_BANK_0,
    BIOS_EXP_BANK_1 
}AVM_BIOS_UPGD_STATE;

struct avm_ent_path_str_type;

/* typedef struct avm_ent_path_str_type
{
   uns32  length;
   uns8   name[AVM_MAX_INDEX_LEN];
}AVM_ENT_PATH_STR_T; */

/*
 * This Structure holds the DHCP configuration elements.
 */
typedef struct avm_ent_dhcp_conf
{
   /* Run time fields */
   uns8                 mac_address[AVM_MAX_MAC_ENTRIES][AVM_MAC_DATA_LEN];
   AVM_PER_LABEL_CONF   *default_label;  /* Default label to be used for next time boot-up */
   char                 host_name_bc1[AVM_HOST_NAME_LENGTH]; /* Host entry name in dhcpd.conf for base channel 1 */
   char                 host_name_bc2[AVM_HOST_NAME_LENGTH]; /* Host entry name in dhcpd.conf for base channel 2 */
   NCS_BOOL             default_chg;   /* TRUE- Default Label change */

   NCS_BOOL             upgd_prgs; /* TRUE if software upgrade is in progress */

   /* MIB objects */
   NCS_BOOL                  net_boot; /* Object tells whether this blade supports net booting */
   AVM_DHCP_ADM_OPER         adm_oper; /* Administrative operations */
   uns8                      tftp_serve_ip[4];    /* TFTP server IP address for DHCP configuration */
   AVM_DHCP_CONF_NAME_TYPE   pref_label;  /* User configured preferred label */
   AVM_PER_LABEL_CONF        *curr_act_label; /* Current Active label */
   AVM_PER_LABEL_CONF        label1;       /* Label 1 related fields */
   AVM_PER_LABEL_CONF        label2;       /* Label 2 related fields */
   AVM_DEFAULT_LABEL_NUM           def_label_num;         /* default label num */
   AVM_CURRENT_ACTIVE_LABEL_NUM    cur_act_label_num;     /* current active label num */
   AVM_DHCP_CONF_NAME_TYPE   pss_curr_label; /* this is the current active label, which gets played back */
   AVM_DHCP_CONF_NAME_TYPE         ipmc_helper_node;      /* to store ipmc helper path in shelf|slot|sub-slot form */
   AVM_ENT_PATH_STR_T              ipmc_helper_ent_path;  /* to store the ipmc helper blade entity path */
   AVM_UPGRADE                     upgrade_type;          /* to store the type of upgrade. it can be SW-BIOS(0), INTEG(1), IPMC(2) */
   uns32                           ipmb_addr;             /* the ipmb address of the blade, which is used while upgrading IPMC */ 
   AVM_IPMC_UPGD_STATE             ipmc_upgd_state;       /* This state maintains the IPMC upgrade state */
   uns8                            pld_bld_ipmc_status;
   uns8                            pld_rtm_ipmc_status; 
   AVM_BIOS_UPGD_STATE             bios_upgd_state;
}AVM_ENT_DHCP_CONF;

typedef union avm_pssv_push
{
   AVM_SSU_LABEL_STATUS int_val;
   AVM_DHCP_CONF_NAME_TYPE   oct_str;
}AVM_PSSV_PUSH;

typedef enum label_num
{
   LABEL1 = 1,
   LABEL2 = 2
}AVM_LABEL_NUM;  

struct slot_info
{
   uns32 shelf;
   uns32 slot;
   uns32 subSlot;
};

#define AVM_ENT_DHCP_CONF_NULL ((AVM_ENT_DHCP_CONF*)0x0)
#define AVM_PER_LABEL_CONF_NULL ((AVM_PER_LABEL_CONF*)0x0)

/*   Some Useful Macros */

/* Macro to handle state changes when preferred labe is changed.
* If label status is UPGD FAILED then change the state to
* INSTALLED and if the label status is COMMIT PENDING then
* change the state to UPGD FAILED */
#define m_AVM_PREF_LABEL_SET_STATE_CHAGES(label) \
{ \
   if (SSU_UPGD_FAILED == label.status) \
      label.status = SSU_INSTALLED; \
   if (SSU_COMMIT_PENDING == label.other_label->status) \
      label.other_label->status = SSU_UPGD_FAILED; \
}


/* Macro to create host entry name  for this entity path */
#ifdef HPI_A
#define m_AVM_CREATE_HOST_ENTRY(entity_path,dhcp_serv_conf) \
{ \
   int i; \
   char  str[50], *tmp_str; \
   tmp_str = str; \
   tmp_str += sysf_sprintf(tmp_str,"dhcp_host_"); \
   for (i = 0; i < AVM_HPI_ENT_HIR_LVL; i++)  \
      tmp_str += sysf_sprintf(tmp_str,"%d_",entity_path->Entry[i].EntityInstance); \
   sysf_sprintf(dhcp_serv_conf.host_name_bc1,"%sbc1",str); \
   sysf_sprintf(dhcp_serv_conf.host_name_bc2,"%sbc2",str); \
}
#else
#define m_AVM_CREATE_HOST_ENTRY(entity_path,dhcp_serv_conf) \
{ \
   int i; \
   char  str[50], *tmp_str; \
   tmp_str = str; \
   tmp_str += sysf_sprintf(tmp_str,"dhcp_host_"); \
   for (i = 0; i < AVM_HPI_ENT_HIR_LVL; i++)  \
      tmp_str += sysf_sprintf(tmp_str,"%d_",entity_path->Entry[i].EntityLocation); \
   sysf_sprintf(dhcp_serv_conf.host_name_bc1,"%sbc1",str); \
   sysf_sprintf(dhcp_serv_conf.host_name_bc2,"%sbc2",str); \
}
#endif

/* Macro to set Name */
#define m_AVM_SET_NAME(dest_name,param_val) \
{ \
   dest_name.length = param_val.i_length; \
   memcpy(dest_name.name, param_val.info.i_oct, param_val.i_length); \
   dest_name.name[param_val.i_length] = '\0'; \
}

#define m_AVM_STR_TO_VER(st, ver) \
{ \
   ver.length = m_NCS_STRLEN(st); \
   memcpy(ver.name, st, ver.length); \
}

/* Macro to push an Integer value to PSSV */
#define m_AVM_SSU_PSSV_PUSH_INT(avm_cb, push_val, push_obj, ent_info) \
      { \
         AVM_PSSV_PUSH  avm_pssv; \
         uns8 push_logbuf[500]; \
         avm_pssv.int_val = push_val; \
         if (avm_send_dynamic_data(avm_cb, ent_info, push_obj, \
                                 NCSMIB_FMAT_INT, &avm_pssv) != NCSCC_RC_SUCCESS) \
         { \
            sysf_sprintf(push_logbuf,"AVM-SSU: Payload blade %s: Push Failed at line: %d in file: %s",ent_info->ep_str.name, \
                                                                                     __LINE__, __FILE__); \
            m_AVM_LOG_DEBUG(push_logbuf,NCSFL_SEV_ERROR); \
         } \
         else \
         { \
            sysf_sprintf(push_logbuf,"AVM-SSU: Payload blade %s: Push Success at line: %d in file: %s",ent_info->ep_str.name, \
                                                                                     __LINE__, __FILE__); \
            m_AVM_LOG_DEBUG(push_logbuf,NCSFL_SEV_NOTICE); \
         } \
      }                                                                         
 
/* Macro to push a String to PSSV */
#define m_AVM_SSU_PSSV_PUSH_STR(avm_cb, push_str, push_obj, ent_info, \
                         push_length) \
      { \
         AVM_PSSV_PUSH  avm_pssv; \
         uns8 push_logbuf[500]; \
         memset(&avm_pssv.oct_str.name, '\0', AVM_NAME_STR_LENGTH); \
         avm_pssv.oct_str.length = push_length; \
         memcpy(&avm_pssv.oct_str.name, push_str, avm_pssv.oct_str.length); \
         if (avm_send_dynamic_data(avm_cb, ent_info, push_obj, \
                                 NCSMIB_FMAT_OCT, &avm_pssv) != NCSCC_RC_SUCCESS) \
         { \
            sysf_sprintf(push_logbuf,"AVM-SSU: Payload blade %s: Push Failed at line: %d in file: %s",ent_info->ep_str.name, \
                                                                                     __LINE__, __FILE__); \
            m_AVM_LOG_DEBUG(push_logbuf,NCSFL_SEV_ERROR); \
         } \
         else \
         { \
            sysf_sprintf(push_logbuf,"AVM-SSU: Payload blade %s: Push Success at line: %d in file: %s",ent_info->ep_str.name, \
                                                                                     __LINE__, __FILE__); \
            m_AVM_LOG_DEBUG(push_logbuf,NCSFL_SEV_NOTICE); \
         } \
      }

extern uns32
avm_set_preferred_label(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info, AVM_ENT_DHCP_CONF *dhcp_conf, NCSMIB_PARAM_VAL param_val);

extern void
avm_set_label_state_to_install(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info, AVM_ENT_DHCP_CONF *dhcp_conf, AVM_PER_LABEL_CONF *label_cnf, AVM_LABEL_NUM label_no);

extern void
avm_ssu_dhcp_rollback (AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info);

extern void
avm_ssu_dhcp_integ_rollback (AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info);

extern uns32
extract_slot_shelf_subslot(uns8 *node_name, struct slot_info *slot);

extern uns32
avm_convert_nodeid_to_entpath(NCSMIB_PARAM_VAL param_val, AVM_ENT_PATH_STR_T* ep);

extern uns32
avm_prepare_entity_path (uns8* str, const struct slot_info sInfo);

extern uns32
avm_compute_ipmb_address (AVM_ENT_INFO_T *ent_info);

extern uns32
avm_upgrade_ipmc_trigger(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info);
extern uns32
avm_upgrade_ipmc(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info);
extern NCS_BOOL
avm_is_the_helper_payload_present(AVM_CB_T *avm_cb, AVM_ENT_PATH_STR_T helper_ent_path);
extern NCS_BOOL
avm_validate_state_for_dhcp_conf_change(AVM_ENT_INFO_T *ent_info, AVM_PER_LABEL_CONF *self_label, AVM_PER_LABEL_CONF *other_label);
extern void
avm_role_change_check_pld_upgd_prg(AVM_CB_T *avm_cb);

extern uns32
avm_dhcp_file_validation(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info, NCSMIB_PARAM_VAL param_val, AVM_PER_LABEL_CONF *label, AVM_LABEL_NUM label_no);

extern uns32
avm_send_dynamic_data(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info, NCSMIB_PARAM_ID param_id, NCSMIB_FMAT_ID fmt_id, AVM_PSSV_PUSH *avm_pss);

extern uns32 avm_check_config(uns8 *ent_info, uns32 *flag);

extern void
avm_role_change_check_pld_upgd_prg(AVM_CB_T *avm_cb);

#endif /*__AVM_DHCP_CONF_H__*/
