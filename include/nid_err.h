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

  $Header:

  ..............................................................................

  DESCRIPTION:
  This file containts the error codes specific to the services spawned by NID 

******************************************************************************
*/

#ifndef NID_ERR_H
#define NID_ERR_H

/* API Error Codes */
#define NID_INV_PARAM    11
#define NID_OFIFO_ERR    22
#define NID_WFIFO_ERR    33


/***************************************************************
 * Never Ever Dare To Change the Error Codes To Begin          *
 * With '0' or'1', for all the error codes below.              *
 * we reserve 1 for NCSCC_RC_SUCCESS                           * 
 ***************************************************************/

/*HPM initialization Error Codes */
enum nid_hpmstat_cods{
   NID_HPM_IPMC_OPEN_FAILED=2,
   NID_HPM_IPMC_FRU_CMD_FAILED,
   NID_HPM_IPMC_SEND_FAILED,
   NID_HPM_CLI_OPEN_FAILED,
   NID_HPMVER_CREATE_FAILED,
   NID_HPM_NCS_AGENTS_START_FAILED,
   NID_MAXHPMERR
};
/* F101 IPMC Upgrade Error Codes */
/* vivek_nid */
enum nid_ipmcupgradestat_cods{
   NID_IPMC_QUERY_NOT_SUPPORTED=4,
   NID_IPMC_QUERY_INVALID_VALUE,
   NID_IPMC_QUERY_DEVICE_NOT_FOUND,
   NID_IPMC_QUERY_OUTPUT_NOT_RECOGNIZED,
   NID_IPMC_NEW_VER_VERIFY_FAILURE,
   NID_IPMC_WRITE_NOT_SUPPORTED,
   NID_IPMC_WRITE_FAILURE,
   NID_IPMC_WRITE_INVALID_VALUE,
   NID_IPMC_WRITE_DEVICE_NOT_FOUND,
   NID_IPMC_WRITE_OUTPUT_NOT_RECOGNIZED,
   NID_BIOS_FCU_QUERY_NOT_SUPPORTED,
   NID_BIOS_FCU_QUERY_INVALID_VALUE,
   NID_BIOS_FCU_QUERY_DEVICE_NOT_FOUND,
   NID_BIOS_FCU_QUERY_OUTPUT_NOT_RECOGNIZED,
   NID_BIOS_FCU_CURRENT_BIOS_VER_EXT_FAILED,
   NID_BIOS_FCU_VERIFY_FAILURE,
   NID_BIOS_FCU_VERIFICATION_FAILURE,
   NID_BIOS_FCU_MARK_NOT_SUPPORTED,
   NID_BIOS_FCU_MARK_INVALID_VALUE,
   NID_BIOS_FCU_MARK_FAILURE,
   NID_BIOS_FCU_MARK_DEVICE_NOT_FOUND,
   NID_BIOS_FCU_MARK_OUTPUT_NOT_RECOGNIZED,
   NID_BIOS_FCU_WRITE_NOT_SUPPORTED,
   NID_BIOS_FCU_WRITE_FAILURE,
   NID_BIOS_FCU_WRITE_INVALID_VALUE,
   NID_BIOS_FCU_WRITE_DEVICE_NOT_FOUND,
   NID_BIOS_FCU_WRITE_SUCCESS,
   NID_BIOS_FCU_WRITE_OUTPUT_NOT_RECOGNIZED,
   NID_BIOS_UPGRADE_FAILURE,
   NID_BIOS_VERSION_INCONSISTENT_GOING_FOR_RECOVERY,
   NID_MAXIPMCBIOSERR
};
/* 7221 Phoenix Bios Upgrade Error Coded */
/* vivek_nid */
enum nid_phoenixbiosupgradestat_cods{
   NID_PHOENIXBIOS_QUERY_NOT_SUPPORTED=4,
   NID_PHOENIXBIOS_QUERY_INVALID_VALUE,
   NID_PHOENIXBIOS_QUERY_DEVICE_NOT_FOUND,
   NID_PHOENIXBIOS_QUERY_OUTPUT_NOT_RECOGNIZED,
   NID_PHOENIXBIOS_CURRENT_VER_EXT_FAILED,
   NID_PHOENIXBIOS_VERIFY_FAILURE,
   NID_PHOENIXBIOS_MARK_NOT_SUPPORTED,
   NID_PHOENIXBIOS_MARK_INVALID_VALUE,
   NID_PHOENIXBIOS_MARK_FAILURE,
   NID_PHOENIXBIOS_MARK_DEVICE_NOT_FOUND,
   NID_PHOENIXBIOS_MARK_OUTPUT_NOT_RECOGNIZED,
   NID_PHOENIXBIOS_WRITE_NOT_SUPPORTED,
   NID_PHOENIXBIOS_WRITE_INVALID_VALUE,
   NID_PHOENIXBIOS_WRITE_DEVICE_NOT_FOUND,
   NID_PHOENIXBIOS_WRITE_FAILURE,
   NID_PHOENIXBIOS_WRITE_OUTPUT_NOT_RECOGNIZED,
   NID_MAXPHOENIXBIOSERR
};


/*HLFM initialization Error Codes*/
enum nid_hlfmstat_cods{

   NID_RDE_NCS_AGENTS_START_FAILED=2,
   NID_RDE_LHC_GET_ROLE_FAILED,
   NID_RDE_LHC_BONDING_CFG_FAILED,
   NID_RDE_LHC_OPEN_FAILED,
   NID_RDE_AMF_OPEN_FAILED,
   NID_RDE_RDA_OPEN_FAILED,
   NID_LFM_LHC_GET_ROLE_FAILURE,
   NID_LFM_LINK_STAT_RET_FAILURE,
   NID_LFM_GET_CHASSIS_TYPE_FAILURE,
   NID_LFM_GET_WRONG_CHASSIS_TYPE,
   NID_GFM_INIT_FAILURE, 
   NID_MAXHLFMERR
};

/*OPenHPI initialization Error Codes*/
enum nid_openhpistat_cods{

   NID_OPENHPI_START_FAILED=2,
   NID_OPENHPI_XXXX_FAILED,
   NID_OPENHPI_MAX_ERR
};

/* Switch Manager initialization Error Codes */
enum nid_swmgstat_cods{
   NID_XND_BCM_NOT_FND=2,
   NID_XND_PROC_NOT_FND,
   NID_XND_DAEMON_NOT_FND,
   NID_XND_DAEMON_START_FAILED,
   NID_XND_PORT_CONFIG_FAILED,
   NID_XND_HPM_SLOT_GET_ERROR,
   NID_XND_HPM_PART_NUM_GET_ERROR,
   NID_XND_PRE_NCS_INIT_FAILURE,
   NID_MAXXNDERR
};

/* LHCD initialization Error Codes */
enum lhcdstat_cods{
   LHCD_NID_CONF_FILE_ERR=2,
   LHCD_NID_CMD_LINE_ARG_ERR,
   LHCD_NID_DAEMON_INIT_FAILURE,
   LHCD_NID_NCS_CORE_AGENTS_STARTUP_FAILURE,
   LHCD_NID_TASK_START_FAILURE,
   LHCD_NID_DTSV_BIND_FAILURE,
   LHCD_NID_CMD_LM_FAILURE,
   LHCD_NID_MAB_REG_FAILURE,
   LHCD_NID_MAB_CLAIM_ROW_FAILURE,
   LHCD_NID_L2_CONN_FAILURE,
   NID_MAXLHCDERR
};

/* LHCR initialization Error Codes */
enum lhcrstat_cods{
   LHCR_NID_CONF_FILE_ERR=2,
   LHCR_NID_CMD_LINE_ARG_ERR,
   LHCR_NID_DAEMON_INIT_FAILURE,
   LHCR_NID_NCS_CORE_AGENTS_STARTUP_FAILURE,
   LHCR_NID_TASK_START_FAILURE,
   LHCR_NID_DTSV_BIND_FAILURE,
   LHCR_NID_CMD_LM_FAILURE,
   LHCR_NID_MAB_REG_FAILURE,
   LHCR_NID_MAB_CLAIM_ROW_FAILURE,
   LHCR_NID_L2_CONN_FAILURE,
   NID_MAXLHCRERR
};
/*LHC-NID -e*/

/* Network initialization Error Codes */
enum nid_nw_stat_cods {
   NID_NWNG_FILE_NOT_FND=2,
   NID_NWNG_FAILED,
   NID_MAXNWERR
};

/* DRBD Device initialization Error Codes */
enum nid_drbd_stat_cods {
   NID_DRBD_LOD_ERR=2,
   NID_DRBDADM_NOT_FND,
   NID_DRBD_MNT_ERR,
   NID_MAXDRBDERR
};

/* IFSVDD specific Error Codes */
enum nid_ifsvdd_stat_cods{
   NID_IFSVDD_CREATE_FAILED=2,
   NID_IFSVDD_ERR2,
   NID_IFSVDD_ERR3,
   NID_IFSVDD_MAX_ERR
};

/* SNMPD specific Error Codes */
enum nid_snmpd_stat_cods{
   NID_SNMPD_CREATE_FAILED=2,
   NID_SNMPD_VIP_CFG_FILE_NOT_FOUND,
   NID_SNMPD_VIP_ADDR_NOT_FOUND,
   NID_SNMPD_VIP_INTF_NOT_FOUND,
   NID_SNMPD_MAX_ERR
};

/* TIPC specific Error Codes */ 
enum nid_tipc_stat_cods{
   NID_TIPC_LOAD_MODULE_ERR=2,
   NID_TIPC_HPM_SLOT_GET_ERR,
   NID_TIPC_UNINSTALL_FAILED,  
   NID_TIPC_MAX_ERR
};

enum nid_ncs_stat_cods {
   NID_NCS_DAEMON_NOT_FOUND=2,
   NID_NCS_PROCESS_START_FAILED,
   NID_NCS_WRONG_SVC_FILE_NAME, 
   NID_NCS_GET_SLOT_ID_FAILED,    
   NID_NCS_GET_SHELF_ID_FAILED,  
   NID_NCS_GET_NODE_ID_FAILED,  
   NID_NCS_CFG_DIR_ROOTNAME_LEN_EXCEED, 
   NID_NCS_CORE_AGENTS_ALREADY_STARTED,
   NID_NCS_NOT_ABLE_TO_CREATE_LOG_DIR, 
   NID_NCS_LEAP_BASIC_SVCS_INIT_FAILED,
   NID_NCS_SPRR_LIB_REQ_FAILED,       
   NID_NCS_MDS_START_UP_FAILED,      
   NID_NCS_MDA_LIB_REQ_FAILED,      
   NID_NCS_DTA_LIB_REQ_FAILED,     
   NID_NCS_OAC_LIB_REQ_FAILED,    
   NID_NCS_MBCA_STARTUP_FAILED,  
   NID_NCS_VDS_LIB_REQ_FAILED,  
   NID_NCS_MAA_STARTUP_FAILED, 
   NID_NCS_SRMA_STARTUP_FAILED,
   NID_NCS_AVD_LIB_REQ_FAILED,
   NID_NCS_AVND_LIB_REQ_FAILED, 
   NID_NCS_AVM_LIB_REQ_FAILED, 
   NID_NCS_AVA_CLA_STARTUP_FAILED,   
   NID_NCS_SRMND_LIB_REQ_FAILED,    
   NID_NCS_BAM_DL_FUNC_FAILED,     
   NID_NCS_DTS_LIB_REQ_FAILED, 
   NID_NCS_MAS_LIB_REQ_FAILED,
   NID_NCS_CLI_LIB_REQ_FAILED,
   NID_NCS_PSS_LIB_REQ_FAILED,
   NID_NCS_EDS_LIB_REQ_FAILED, 
   NID_NCS_EDA_STARTUP_FAILED,
   NID_NCS_SUBAGT_LD_LIB_FAILED,   
   NID_NCS_SUBAGT_DL_SYM_FAILED,  
   NID_NCS_SUBAGT_LIB_CRT_FAILED,
   NID_NCS_MQD_LIB_REQ_FAILED,  
   NID_NCS_MQND_LIB_REQ_FAILED,
   NID_NCS_MQA_STARTUP_FAILED,
   NID_NCS_GLD_LIB_REQ_FAILED,
   NID_NCS_GLND_LIB_REQ_FAILED, 
   NID_NCS_GLA_STARTUP_FAILED, 
   NID_NCS_CPD_LIB_REQ_FAILED,
   NID_NCS_CPND_LIB_REQ_FAILED, 
   NID_NCS_CPA_STARTUP_FAILED, 
   NID_NCS_IFD_LIB_REQ_FAILED,
   NID_NCS_IFND_LIB_REQ_FAILED, 
   NID_NCS_IFA_STARTUP_FAILED, 
   NID_NCS_IFSV_DRV_SVC_REQ_FAILED, 
   NID_NCS_HISV_HCD_LIB_REQ_FAILED,
   NID_NCS_MAX_ERR
};

/* DTSV initialization Error Codes */
enum nid_dtsv_stat_cods{
   NID_DTSV_ERR1=2,
   NID_DTSV_MAX_ERR
};

/* MSAV initialization Error Codes */
enum nid_masv_stat_cods{
   NID_MASV_ERR1=2,
   NID_MASV_MAX_ERR
};

/* PSSV initialization Error Codes */
enum nid_pssv_stat_cods{
   NID_PSSV_ERR1=2,
   NID_PSSV_MAX_ERR
};

/* GLND initialization Error Codes */
enum nid_glnd_stat_cods{
   NID_GLND_ERR1=2,
   NID_GLND_MAX_ERR
};

/* EDSV initialization Error Codes */
enum nid_edsv_stat_cods{
   NID_EDSV_ERR1=2,
   NID_EDSV_MAX_ERR
};

/* 61409 */
/* SUBAGT initialization Error Codes */
enum nid_subagt_stat_cods{
   NID_SUBAGT_ERR1=2,
   NID_SUBAGT_MAX_ERR
};

/* SCAP initialization Error Codes */
enum nid_scap_stat_cods{
   NID_SCAP_ERR1=2,
   NID_SCAP_MAX_ERR
};

/* PCAP initialization Error Codes */
enum nid_pcap_stat_cods{
   NID_PCAP_ERR1=2,
   NID_PCAP_MAX_ERR
};

/* OPENSAF RDF Initialization Error Codes */
enum nid_rdf_cods{
   NID_RDF_AMF_OPEN_FAILED=2,
   NID_RDF_RDA_OPEN_FAILED,
   NID_RDF_MAX_ERR
};

/*****************************************************************
 *  Structure to represent all error codes                       *
 *****************************************************************/
typedef union nid_status_code{
   enum nid_hpmstat_cods hpm_status;
   enum nid_hlfmstat_cods hlfm_status;
   enum nid_rdf_cods rdf_status;
   enum nid_openhpistat_cods openhpi_status;
   enum nid_swmgstat_cods swmg_status;
   enum lhcdstat_cods lhcd_status;
   enum lhcrstat_cods lhcr_status;
   enum nid_nw_stat_cods nw_status;
   enum nid_drbd_stat_cods derbddev_status;

   enum nid_ifsvdd_stat_cods ifsvdd_status;
   enum nid_tipc_stat_cods tipc_status;
   enum nid_snmpd_stat_cods snmpd_status;

   /* NCS specific error-status codes */
   enum nid_ncs_stat_cods  ncs_status;
   enum nid_dtsv_stat_cods dtsv_status;
   enum nid_masv_stat_cods masv_status;
   enum nid_pssv_stat_cods pssv_status;
   enum nid_glnd_stat_cods glnd_status;
   enum nid_edsv_stat_cods edsv_status;
   enum nid_subagt_stat_cods subagt_status; /* 61409 */
   enum nid_scap_stat_cods scap_status;
   enum nid_pcap_stat_cods pcap_status;
} NID_STATUS_CODE;

#endif /*NID_ERR_H*/


