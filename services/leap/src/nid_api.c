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
*                                                                            *
*  MODULE NAME:  nid_api.c                                                   *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module defines the API used by bladeinitd spawned services. From ser *
*  vices point of view, this API provides interface to communicate the       *
*  initialization status to bladeinitd. Bladeinitd spawned services are      *
*  HPM,RDF,XND,LHC,DRBD,NETWORKING,TIPC,MASV,PSSV,DTSV,PCAP,SCAP.           *
*                                                                            *
*****************************************************************************/

#include "nid_api.h"


typedef union stat_code{
   NID_STATUS_CODE code;
   uns32 status_code;
} NID_STAT_CODE;

/*****************************************************************
 * Used to store the mappings betw service status and            *
 * descriptive message to be logged. Mainly used by              *
 * bladeinitd.                                                   *
 *                                                               *
 * NOTE:- Make sure that the sequence of service info listed     *
 * below matches with the sequence in SERVIC_CODE                *
 * enumeration specified in nid_api.h.                           *
*****************************************************************/ 
/* vivek_nid */
const NID_SVC_STATUS nid_serv_stat_info[]={     \
   {NID_MAXHPMERR,"HPM",{{"NID_HPM_IPMC_OPEN_FAILED","IPMC serial interface open failed"},\
                     {"NID_HPM_IPMC_FRU_CMD_FAILED","IPMC FRU info retrieve failed"},\
                     {"NID_HPM_IPMC_SEND_FAILED","Failed probing IPMC"},\
                     {"NID_HPM_CLI_OPEN_FAILED","Opening CLI socket failed for HPM"},\
                     {"NID_HPMVER_CREATE_FAILED","Failed creating HPMVers file"},\
                     {"NID_HPM_NCS_AGENTS_START_FAILED","NCS Agents startup failed for HPM"}}},\
   {NID_MAXIPMCBIOSERR,"IPMCUP",{{"NID_IPMC_QUERY_NOT_SUPPORTED","FCU IPMC Query Is Not Supported"},\
                     {"NID_IPMC_QUERY_INVALID_VALUE","FCU IPMC Query Invalid Value As Input"},\
                     {"NID_IPMC_QUERY_DEVICE_NOT_FOUND","FCU IPMC Query Cannot Locate Device"},\
                     {"NID_IPMC_QUERY_OUTPUT_NOT_RECOGNIZED","FCU IPMC Query Output Is Not Recognized"},\
                     {"NID_IPMC_NEW_VER_VERIFY_FAILURE","FCU IPMC File Version Extraction Failure"},\
                     {"NID_IPMC_WRITE_NOT_SUPPORTED","FCU IPMC Write Is Not Supported"},\
                     {"NID_IPMC_WRITE_FAILURE","FCU IPMC Write Failure"},\
                     {"NID_IPMC_WRITE_INVALID_VALUE","FCU IPMC Write Invalid Value As Input"},\
                     {"NID_IPMC_WRITE_DEVICE_NOT_FOUND","FCU IPMC Write Cannot Locate Device"},\
                     {"NID_IPMC_WRITE_OUTPUT_NOT_RECOGNIZED","FCU IPMC Write Output Is Not Recognized"},\
                     {"NID_BIOS_FCU_QUERY_NOT_SUPPORTED","FCU BIOS Query Is Not Supported"},\
                     {"NID_BIOS_FCU_QUERY_INVALID_VALUE","FCU BIOS Query Invalid Value As Input"},\
                     {"NID_BIOS_FCU_QUERY_DEVICE_NOT_FOUND","FCU BIOS Query Cannot Locate Device"},\
                     {"NID_BIOS_FCU_QUERY_OUTPUT_NOT_RECOGNIZED","FCU BIOS Query Output Is Not Recognized"},\
                     {"NID_BIOS_FCU_CURRENT_BIOS_VER_EXT_FAILED","FCU BIOS Current Version Extraction Failed"},\
                     {"NID_BIOS_FCU_VERIFY_FAILURE","FCU BIOS Version Extraction From File Failed"},\
                     {"NID_BIOS_FCU_VERIFICATION_FAILURE","FCU BIOS Version Is Not That Was Upgraded. Automatic Rollback Happened"},\
                     {"NID_BIOS_FCU_MARK_NOT_SUPPORTED","FCU BIOS Mark Is Not Supported"},\
                     {"NID_BIOS_FCU_MARK_INVALID_VALUE","FCU BIOS Mark Invalid Value As Input"},\
                     {"NID_BIOS_FCU_MARK_FAILURE","FCU BIOS Mark Failure"},\
                     {"NID_BIOS_FCU_MARK_DEVICE_NOT_FOUND","FCU BIOS Mark Cannot Locate Device"},\
                     {"NID_BIOS_FCU_MARK_OUTPUT_NOT_RECOGNIZED","FCU BIOS Mark Output Is Not Recognized"}}},\
   {NID_MAXPHOENIXBIOSERR,"BIOSUP",{{"NID_PHOENIXBIOS_QUERY_NOT_SUPPORTED","FCU PHOENIX BIOS Query Is Not Supported"},\
                     {"NID_PHOENIXBIOS_QUERY_INVALID_VALUE","FCU PHOENIX BIOS Query Invalid Value As Input"},\
                     {"NID_PHOENIXBIOS_QUERY_DEVICE_NOT_FOUND","FCU PHOENIX BIOS Query Cannot Locate Device"},\
                     {"NID_PHOENIXBIOS_QUERY_OUTPUT_NOT_RECOGNIZED","FCU PHOENIX BIOS Query Output Is Not Recognized"},\
                     {"NID_PHOENIXBIOS_CURRENT_VER_EXT_FAILED","FCU PHOENIX BIOS Failed To Extract Current Version"},\
                     {"NID_PHOENIXBIOS_VERIFY_FAILURE","FCU PHOENIX BIOS Failed to Extract New Version"},\
                     {"NID_PHOENIXBIOS_MARK_NOT_SUPPORTED","FCU PHOENIX BIOS Mark Is Not Supported"},\
                     {"NID_PHOENIXBIOS_MARK_INVALID_VALUE","FCU PHOENIX BIOS Mark Invalid Value As Input"},\
                     {"NID_PHOENIXBIOS_MARK_FAILURE","FCU PHOENIX BIOS Mark Failure"},\
                     {"NID_PHOENIXBIOS_MARK_DEVICE_NOT_FOUND","FCU PHOENIX BIOS Mark Cannot Locate Device"},\
                     {"NID_PHOENIXBIOS_MARK_OUTPUT_NOT_RECOGNIZED","FCU PHOENIX BIOS Mark Output Is Not Recognized"},\
                     {"NID_PHOENIXBIOS_WRITE_NOT_SUPPORTED","FCU PHOENIX BIOS Write Is Not Supported"},\
                     {"NID_PHOENIXBIOS_WRITE_INVALID_VALUE","FCU PHOENIX BIOS Write Invalid Value As Input"},\
                     {"NID_PHOENIXBIOS_WRITE_DEVICE_NOT_FOUND","FCU PHOENIX BIOS Write Cannot Locate Device"},\
                     {"NID_PHOENIXBIOS_WRITE_FAILURE","FCU PHOENIX BIOS Write Failure"},\
                     {"NID_PHOENIXBIOS_WRITE_OUTPUT_NOT_RECOGNIZED","FCU PHOENIX BIOS Write Output Is Not Recognized"}}},\
   {NID_MAXHLFMERR,"HLFM",{{"NID_RDE_NCS_AGENTS_START_FAILED","NCS agents startup failed"},\
                     {"NID_RDE_LHC_GET_ROLE_FAILED","Failed to get role from LHC"},\
                     {"NID_RDE_LHC_BONDING_CFG_FAILED","Failed to configure bonding driver"},\
                     {"NID_RDE_LHC_OPEN_FAILED","LHC interface open failed"},\
                     {"NID_RDE_AMF_OPEN_FAILED","AMF interface open failed"},\
                     {"NID_RDE_RDA_OPEN_FAILED","RDA interface open failed"},\
                     {"NID_LFM_LHC_GET_ROLE_FAILURE","LFM Get LHC role failed"},\
                     {"NID_LFM_LINK_STAT_RET_FAILURE", "LFM Failed to get Link Status"}}},\
   {NID_RDF_MAX_ERR,"RDF",{{"NID_RDF_AMF_OPEN_FAILED","AMF interface open failed"},\
                     {"NID_RDF_RDA_OPEN_FAILED","RDA interface open failed"}}},\
   {NID_OPENHPI_MAX_ERR,"OpenHPI",{{"NID_OPENHPI_START_FAILED","OpenHPI start failed, check syslog for details"},\
                     {"NID_OPENHPI_XXX_FAILED", "OpenHPI reserved error"}}},\

   {NID_MAXXNDERR,"XND",{{"NID_XND_BCM_NOT_FND","BCM kernel driver not found"},\
                      {"NID_XND_PROC_NOT_FND","Device not present in /proc/device directory"},\
                      {"NID_XND_DAEMON_NOT_FND","Unable to find the switch daemon"},\
                      {"NID_XND_DAEMON_START_FAILED","Failed to start switch daemon"},\
                      {"NID_XND_PORT_CONFIG_FAILED","BCM port/VLAN configuration failed"},\
                      {"NID_XND_HPM_SLOT_GET_ERROR","Failed to get slot-id from HPM"},\
                      {"NID_XND_HPM_PART_NUM_GET_ERROR","Failed to get pat-no from HPM"},\
                      {"NID_XND_PRE_NCS_INIT_FAILURE","Failed Pre-NCS initialization"}}},\
   {NID_MAXLHCDERR,"LHCD",{{" LHCD_NID_CONF_FILE_ERR "," Configuration file is not found"},\
                    {" LHCD_NID_CMD_LINE_ARG_ERR "," Wrong command line arguments passed to the lhc executable"},\
                    {" LHCD_NID_DAEMON_INIT_FAILURE "," failure while daemonising the lhc process"},\
                    {" LHCD_NID_NCS_CORE_AGENTS_STARTUP_FAILURE "," ncs_core_agents_startup failure "},\
                    {" LHCD_NID_TASK_START_FAILURE "," lhcsc_task_start failure "},\
                    {" LHCD_NID_DTSV_BIND_FAILURE "," ncs_dtsv_su_req: flexlog bind failure"},\
                    {" LHCD_NID_CMD_LM_FAILURE "," cmd_lm: failure to init the command line utility"},\
                    {" LHCD_NID_MAB_REG_FAILURE "," lhc_mab_register_table : Mib table registration failure"},\
                    {" LHCD_NID_MAB_CLAIM_ROW_FAILURE "," lhc_mab_claim_row_failure"},\
                    {" LHCD_NID_L2_CONN_FAILURE ","L2 connection failure"}}},\
   {NID_MAXLHCRERR,"LHCR",{{" LHCR_NID_CONF_FILE_ERR "," Configuration file is not found"},\
                    {" LHCR_NID_CMD_LINE_ARG_ERR "," Wrong command line arguments passed to the lhc executable"},\
                    {" LHCR_NID_DAEMON_INIT_FAILURE "," failure while daemonising the lhc process"},\
                    {" LHCR_NID_NCS_CORE_AGENTS_STARTUP_FAILURE "," ncs_core_agents_startup failure "},\
                    {" LHCR_NID_TASK_START_FAILURE "," lhcsc_task_start failure "},\
                    {" LHCR_NID_DTSV_BIND_FAILURE "," ncs_dtsv_su_req: flexlog bind failure"},\
                    {" LHCR_NID_CMD_LM_FAILURE "," cmd_lm: failure to init the command line utility"},\
                    {" LHCR_NID_MAB_REG_FAILURE "," lhc_mab_register_table : Mib table registration failure"},\
                    {" LHCR_NID_MAB_CLAIM_ROW_FAILURE "," lhc_mab_claim_row_failure"},\
                    {" LHCR_NID_L2_CONN_FAILURE ","L2 connection failure"}}},\
   {NID_MAXNWERR,"Networking",{{"NID_NWNG_FILE_NOT_FND","Network script file not found"},\
                     {"NID_NWNG_FAILED","Failed Configuring Network interfaces"}}},\
   {NID_MAXDRBDERR,"DRBD",{{"NID_DRBD_LOD_ERR","Error loading DRBD."},\
                       {"NID_DRBDADM_NOT_FND","File drbdadm not found."},\
                       {"NID_DRBD_MNT_ERR","Error mounting DRBD device."}}},\
   /* TIPC specific error code & the respective strings */
   {NID_TIPC_MAX_ERR,"TIPC",{{"NID_TIPC_LOAD_MODULE_ERR", "Unable to load TIPC module"},\
               {"NID_TIPC_HPM_SLOT_GET_ERR", "Unable to get SLOT-NUM for TIPC"},\
               {"NID_TIPC_UNINSTALL_FAILED", "Not able to uninstall TIPC module"}}},\
   /* IFSVDD specific error code & the respective strings */
   {NID_IFSVDD_MAX_ERR,"IFSVDD",{{"NID_IFSVDD_CREATE_FAILED", "IFSV Demo Driver Create Failed"},\
               {"NID_IFSVDD_ERR2", "Nothing"},\
               {"NID_IFSVDD_ERR3", "Nothing"}}},\
   /* SNMPD specific error code & the respective strings */
   {NID_SNMPD_MAX_ERR,"SNMPD",{{"NID_SNMPD_CREATE_FAILED", "SNMPD start Failed"},\
               {"NID_SNMPD_VIP_CFG_FILE_NOT_FOUND", "SNMPD VIP configuration file not found"},\
               {"NID_SNMPD_VIP_ADDR_NOT_FOUND", "SNMPD VIP Address not found in vip config file"},\
               {"NID_SNMPD_VIP_INTF_NOT_FOUND", "SNMPD VIP Interface not found in vip config file"}}},\
   /* NCS specific error codes & the respective strings */
   {NID_NCS_MAX_ERR,"NCS", {{"NID_NCS_WRONG_SVC_FILE_NAME",    "Wrong service file name"},\
               {"NID_NCS_GET_SLOT_ID_FAILED",      "Not able to get the SLOT ID"},\
               {"NID_NCS_GET_SHELF_ID_FAILED",     "Not able to get the SHELF ID"},\
               {"NID_NCS_GET_NODE_ID_FAILED",      "Not able to get the NODE ID"},\
               {"NID_NCS_CFG_DIR_ROOTNAME_LEN_EXCEED", "Cfg directory root name is too long"},\
               {"NID_NCS_CORE_AGENTS_ALREADY_STARTED", "NCS Core agents has already started"},\
               {"NID_NCS_NOT_ABLE_TO_CREATE_LOG_DIR",  "Not able to create NCS log dir"},\
               {"NID_NCS_LEAP_BASIC_SVCS_INIT_FAILED", "Not able to initialise LEAP basic svcs"},\
               {"NID_NCS_SPRR_LIB_REQ_FAILED",         "SPRR lib request failed"},\
               {"NID_NCS_MDS_START_UP_FAILED",         "MDS start-up has been failed"},\
               {"NID_NCS_MDA_LIB_REQ_FAILED",          "MDA lib request failed"},\
               {"NID_NCS_DTA_LIB_REQ_FAILED",          "DTA lib request failed"},\
               {"NID_NCS_OAC_LIB_REQ_FAILED",          "OAC lib request failed"},\
               {"NID_NCS_MBCA_STARTUP_FAILED",      "MBCA start-up has been failed"},\
               {"NID_NCS_VDS_LIB_REQ_FAILED",       "VDS lib request failed"},\
               {"NID_NCS_MAA_STARTUP_FAILED",       "MAA start-up has been failed"},\
               {"NID_NCS_SRMA_STARTUP_FAILED",      "SRMA start-up has been failed"},\
               {"NID_NCS_AVD_LIB_REQ_FAILED",       "AVD lib request failed"},\
               {"NID_NCS_AVND_LIB_REQ_FAILED",      "AVND lib request failed"},\
               {"NID_NCS_AVM_LIB_REQ_FAILED",       "AVM lib request failed"},\
               {"NID_NCS_AVA_CLA_STARTUP_FAILED",   "AVA CLA start-up has been failed"},\
               {"NID_NCS_SRMND_LIB_REQ_FAILED",     "SRMND lib request failed"},\
               {"NID_NCS_BAM_DL_FUNC_FAILED",       "BAM DL function failed"},\
               {"NID_NCS_DTS_LIB_REQ_FAILED",       "DTS lib request failed"},\
               {"NID_NCS_MAS_LIB_REQ_FAILED",       "MAS lib request failed"},\
               {"NID_NCS_CLI_LIB_REQ_FAILED",       "CLI lib request failed"},\
               {"NID_NCS_PSS_LIB_REQ_FAILED",       "PSS lib request failed"},\
               {"NID_NCS_EDS_LIB_REQ_FAILED",       "EDS lib request failed"},\
               {"NID_NCS_EDA_STARTUP_FAILED",       "EDA lib request failed"},\
               {"NID_NCS_SUBAGT_LD_LIB_FAILED",     "SUBAGT load lib failed"},\
               {"NID_NCS_SUBAGT_DL_SYM_FAILED",     "SUBAGT DL sym failed"},\
               {"NID_NCS_SUBAGT_LIB_CRT_FAILED",    "SUBAGT Lib create has been failed"},\
               {"NID_NCS_MQD_LIB_REQ_FAILED",       "MQD lib request failed"},\
               {"NID_NCS_MQND_LIB_REQ_FAILED",      "MQND lib request failed"},\
               {"NID_NCS_MQA_STARTUP_FAILED",       "MQA start-up has been failed"},\
               {"NID_NCS_GLD_LIB_REQ_FAILED",       "GLD lib request failed"},\
               {"NID_NCS_GLND_LIB_REQ_FAILED",      "GLND lib request failed"},\
               {"NID_NCS_GLA_STARTUP_FAILED",       "GLA start-up has been failed"},\
               {"NID_NCS_CPD_LIB_REQ_FAILED",       "CPD lib request failed"},\
               {"NID_NCS_CPND_LIB_REQ_FAILED",      "CPND lib request failed"},\
               {"NID_NCS_CPA_STARTUP_FAILED",       "CPA lib request failed"},\
               {"NID_NCS_IFD_LIB_REQ_FAILED",       "IFD lib request failed"},\
               {"NID_NCS_IFND_LIB_REQ_FAILED",      "IFND lib request failed"},\
               {"NID_NCS_IFA_STARTUP_FAILED",       "IFA start-up has been failed"},\
               {"NID_NCS_IFSV_DRV_SVC_REQ_FAILED",  "IFSV DRV svc request failed"},\
               {"NID_NCS_HISV_HCD_LIB_REQ_FAILED",  "HISV HCD lib request failed"}}},\
   /* DTSV specific error codes & the respective strings */
   {NID_DTSV_MAX_ERR,"DTSV",{{"ERR1","ERR1 in DTSV"}}},\
   /* MASV specific error codes & the respective strings */
   {NID_MASV_MAX_ERR,"MASV",{{"ERR1","ERR1 in MASV"}}},\
   /* PSSV specific error codes & the respective strings */
   {NID_PSSV_MAX_ERR,"PSSV",{{"ERR1","ERR1 in PSSV"}}},\
   /* GLND specific error codes & the respective strings */
   {NID_GLND_MAX_ERR,"GLND",{{"ERR1","ERR1 in GLND"}}},\
   /* PSSV specific error codes & the respective strings */
   {NID_EDSV_MAX_ERR,"EDSV",{{"ERR1","ERR1 in EDSV"}}},\
   /* 61409 */
   /* SUBAGT specific error codes & the respective strings */
   {NID_SUBAGT_MAX_ERR,"SUBAGT",{{"ERR1","ERR1 in SUBAGT"}}},\
   /* SCAP specific error codes & the respective strings */
   {NID_SCAP_MAX_ERR,"SCAP",{{"ERR1","ERR1 in SCAP"}}},\
   /* PCAP specific error codes & the respective strings */
   {NID_PCAP_MAX_ERR,"PCAP",{{"ERR1","ERR1 in PCAP"}}}
};


/****************************************************************************
 * Name          : nid_notify                                               *
 *                                                                          *
 * Description   : Opens the FIFO to bladeinitd and write the service and   *
 *                 status code to FIFO.                                     *
 *                                                                          *
 * Arguments     : service  - input parameter providing service code        *
 *                 status   - input parameter providing status code         *
 *                 error    - output parameter to return error code if any  *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..                      *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32 nid_notify(NID_SVC_CODE service,NID_STATUS_CODE status,uns32 *error)
{
   NID_STAT_CODE scode;
   uns8 msg[250];
   int32 fd = -1;
   uns32 retry=3,service_id;
   char strbuff[256];
                                                                                                         
   scode.code = status;
                                                                                                         
   if((service < NID_HPM) || (service >= NID_MAXSERV)){
      if(error != NULL) *error = NID_INV_PARAM;
      return NCSCC_RC_FAILURE;
   }
                                               
   if((service > NID_NCS) && (service <= NID_PCAP))
      service_id = NID_NCS;
   else
      service_id = service;
                                                       
   if((scode.status_code < 0) || \
      (scode.status_code >= nid_serv_stat_info[service_id].nid_max_err_code)){
      if(error != NULL) *error = NID_INV_PARAM;
      return NCSCC_RC_FAILURE;
   }
                                                                                                         
   while(retry){
        if(nid_open_ipc(&fd,strbuff) != NCSCC_RC_SUCCESS){
          retry--;
        }else break;
   }
                                                                                                         
   if((fd < 0) && (retry == 0)){
     if(error != NULL) *error = NID_OFIFO_ERR;
     return NCSCC_RC_FAILURE;
   }
                                                                                                         
   /************************************************************
   *    Prepare the message to be sent                         *
   ************************************************************/
   sprintf(msg,"%x:%s:%d",NID_MAGIC,nid_serv_stat_info[service].nid_serv_name,scode.status_code);                                                                                                       
   /************************************************************
   *    Send the message                                       *
   ************************************************************/
   retry=3;
   while(retry){
        if(m_NCS_POSIX_WRITE(fd,msg,m_NCS_STRLEN(msg)) == m_NCS_STRLEN(msg)) break;
        else retry--;
   }
                                                                                                         
   if(retry == 0){
     if(error != NULL) *error = NID_WFIFO_ERR;
     return NCSCC_RC_FAILURE;
   }
                 
   nid_close_ipc();
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : nis_notify                                               *
 *                                                                          *
 * Description   : Opens the FIFO to bladeinitd and write the service and   *
 *                 status code to FIFO.                                     *
 *                                                                          *
 * Arguments     : status   - input parameter providing status              *
 *                 error    - output parameter to return error code if any  *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..                      *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32 nis_notify(uns8 *status,uns32 *error)
{

   int32 fd = -1;
   uns32 retry=3;
   char strbuff[256];
                                                                                                         
   if( status == NULL ){ 
      if(error != NULL) *error = NID_INV_PARAM;
      return NCSCC_RC_FAILURE;
   }
                                                                                                         
   while(retry){
        if(nid_open_ipc(&fd,strbuff) != NCSCC_RC_SUCCESS){
          retry--;
        }else break;
   }
                                                                                                         
   if((fd < 0) && (retry == 0)){
     if(error != NULL) *error = NID_OFIFO_ERR;
     return NCSCC_RC_FAILURE;
   }
                                                                                                         
   /************************************************************
   *    Send the message                                       *
   ************************************************************/
   retry=3;
   while(retry){
        if(m_NCS_POSIX_WRITE(fd,status,strlen(status)) == strlen(status)) break;
        else retry--;
   }
                                                                                                         
   if(retry == 0){
     if(error != NULL) *error = NID_WFIFO_ERR;
     return NCSCC_RC_FAILURE;
   }
                 
   nid_close_ipc();
   return NCSCC_RC_SUCCESS;
}

#if 0
/****************************************************************************
 * Name          : nid_get_process_id                                       *
 *                                                                          *
 * Description   : Given the process/service name, this function returns    *
 *                 the respective NID service code.                         *
 *                                                                          *
 * Arguments     : i_proc_name - input service/process name                 *
 *                 nid_proc_id - output respective NID svc-id               *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..                      *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32 nid_get_process_id(char *i_proc_name, int *nid_proc_id)
{
   char  *out_data;
   char  buffer[256];
   char  proc_name[256];
   char  *split_patt = NID_PROCESS_NAME_SPLIT_PATTERN;

   m_NCS_OS_MEMSET(&buffer, 0, 256);   
   m_NCS_OS_STRCPY(buffer, i_proc_name);

   if ((out_data = strtok(buffer, split_patt)) == NULL)
      return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(proc_name, 0, 256);
   m_NCS_OS_STRCPY(proc_name, out_data);

   while (out_data)
   {
      m_NCS_OS_MEMSET(proc_name, 0, 256);
      m_NCS_OS_STRCPY(proc_name, out_data);
      out_data = strtok(NULL, split_patt);
   }

   if (!m_NCS_OS_STRCMP(proc_name, NID_MDS_PROC_NAME)) 
      *nid_proc_id = NID_MDS;
   else        
   if (!m_NCS_OS_STRCMP(proc_name, NID_MASV_PROC_NAME)) 
      *nid_proc_id = NID_MASV;
   else        
   if (!m_NCS_OS_STRCMP(proc_name, NID_PSSV_PROC_NAME)) 
      *nid_proc_id = NID_PSSV;
   else        
   if (!m_NCS_OS_STRCMP(proc_name, NID_SCAP_PROC_NAME)) 
      *nid_proc_id = NID_SCAP;

   return NCSCC_RC_SUCCESS;
}
#endif






