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

  DESCRIPTION:This module deals with DHCP server configuration utility functions.
  ..............................................................................

  Function Included in this Module:

******************************************************************************
*/

#include "avm.h"

static uns32 avm_ssu_clear_mac (AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info_in);


/****************************************************************************
 * Name          : avm_check_config
 *
 * Description   :This function is used to parse ssuHelper.conf file which  
 *                 contains entires in the form of node id
 *
 * Arguments     : 
 *                 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ****************************************************************************/

uns32 
avm_check_config(uns8 *ent_info,
                 uns32 *flag)
{
    NCS_OS_FILE conf_file;
    uns8 buff[256];
    struct slot_info slotInfo;
    uns32 rc = NCSCC_RC_FAILURE;
    conf_file.info.file_exists.i_file_name=CONF_FILE_PATH;
    if(ncs_os_file(&conf_file, NCS_OS_FILE_EXISTS) == NCSCC_RC_SUCCESS)
    {
       if(conf_file.info.file_exists.o_file_exists == TRUE)
       {
          conf_file.info.open.i_file_name =CONF_FILE_PATH;
          conf_file.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;

          if(m_NCS_OS_FILE(&conf_file, NCS_OS_FILE_OPEN) != NCSCC_RC_SUCCESS)
          {
             return NCSCC_RC_FAILURE;
          }
          else
          {
             while(m_NCS_OS_FGETS(buff,sizeof(buff),(FILE *)conf_file.info.open.o_file_handle))
             {
                buff[(m_NCS_OS_STRLEN(buff)-1)]='\0';

             if ((rc = extract_slot_shelf_subslot(buff,&slotInfo)) == NCSCC_RC_FAILURE)
             {
                return NCSCC_RC_FAILURE;
             }

             avm_prepare_entity_path(buff,slotInfo);

            if(m_NCS_OS_STRCMP(ent_info,buff)==0)
            {
               *flag=1;
               break;
            }

            }

             return NCSCC_RC_SUCCESS;
          }
       }
       else
       {
          return NCSCC_RC_SUCCESS;
       }

    }
    else
    {
       return NCSCC_RC_FAILURE;
    }
}

/****************************************************************************
 * Name          : avm_dhcp_file_validation
 *
 * Description   : This function is used for validating the image and
 *                 and then sets the software version.
 *
 * Arguments     : AVM_PER_LABEL_CONF - Pointer to Label configuration
 *                 NCSMIB_PARAM_VAL - Param value.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ****************************************************************************/

extern uns32
avm_dhcp_file_validation(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info, 
                         NCSMIB_PARAM_VAL param_val, AVM_PER_LABEL_CONF *label,
                         AVM_LABEL_NUM label_no)
{
   AVM_DHCP_CONF_NAME_TYPE file;
   char    *tmp_str, ver_str[AVM_NAME_STR_LENGTH];
   char file_name[AVM_NAME_STR_LENGTH + 11];
   FILE *fp;
   struct stat fileStat;
   char  time_str[30];
   uns32 year, month, day, hrs, min, sec;
   uns8  str[255];
   NCSMIB_PARAM_ID param_id1,param_id2;
   uns8 logbuf[500];

   logbuf[0] = '\0';
  
   str[0] = '\0';

   memset(&file, '\0', sizeof(AVM_DHCP_CONF_NAME_TYPE));
   m_AVM_SET_NAME(file, param_val);

   memset(file_name, '\0', sizeof(file_name));
   sprintf(file_name, "/%s", file.name);

   /* Check whether file exist */
   if (-1 == access(file_name, F_OK))
   {
      perror(file_name);
      return NCSCC_RC_FAILURE;
   }

   memset(&fileStat, '\0', sizeof(struct stat));
   stat(file_name, &fileStat);

   /* Check if the file is of DIRECTORY type */

   if(!(fileStat.st_mode & S_IFDIR))
   {
      m_AVM_LOG_GEN_EP_STR("AVM-SSU: Not A DIRECTORY File:", file_name,  NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   /* Check for pxelinux.0 */
   memset(&fileStat, '\0', sizeof(struct stat));
   strcat(file_name,AVM_DHCPD_SW_PXE_FILE);
   stat(file_name, &fileStat);

   if(!(fileStat.st_mode & S_IFREG))
   {
      m_AVM_LOG_GEN_EP_STR("AVM-SSU: Not A Regular File:", file_name,  NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }    
   strftime(time_str, 30, "%Y %m %d %k %M %S", localtime(&fileStat.st_mtime));  

   sscanf(time_str, "%d %d %d %d %d %d", &year, &month, &day, &hrs, &min, &sec);
   label->install_time.dttm.year = (uns16)year;
   label->install_time.dttm.month = (uns8)month;
   label->install_time.dttm.day = (uns8)day;
   label->install_time.dttm.hrs = (uns8)hrs;
   label->install_time.dttm.min = (uns8)min;
   label->install_time.dttm.sec = (uns8)sec;

   label->install_time.dttm.year = 
        m_NCS_OS_HTONS(label->install_time.dttm.year);

   tmp_str = strrchr(file_name, '/');
   *(++tmp_str) = '\0';

   /* read version file */
   strcat(file_name, AVM_DHCPD_SW_VER_FILE);
   if (NULL == (fp = sysf_fopen(file_name, "r")))
   {
      m_AVM_LOG_DEBUG("Version file might not be present, unable to open version file", NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   fscanf(fp, "%s", ver_str);
   fclose(fp);

  if (NULL == ver_str)
  {
      m_AVM_LOG_DEBUG("Version string is NULL", NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
  }

   /* Get the version information */
   m_AVM_STR_TO_VER(ver_str, label->sw_version);
   
   /* Check for the firmware files in the package */
   tmp_str = strrchr(file_name, '/');
   *(++tmp_str) = '\0';
  
   /* Check for AVM_DHCPD_SW_IPMC_PLD_BLD_FILE */ 
   strcat(file_name, AVM_DHCPD_SW_IPMC_PLD_BLD_FILE);
   if (-1 == access(file_name, F_OK))
   {
      sprintf(str,"AVM-SSU: Payload blade %s:AVM_DHCPD_SW_IPMC_PLD_BLD_FILE is missing in the file path",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str, NCSFL_SEV_WARNING);
   }

   tmp_str = strrchr(file_name, '/');
   *(++tmp_str) = '\0';
  
   /* Check for AVM_DHCPD_SW_IPMC_PLD_RTM_FILE1 */ 
   strcat(file_name, AVM_DHCPD_SW_IPMC_PLD_RTM_FILE1);
   if (-1 == access(file_name, F_OK))
   {
      sprintf(str,"AVM-SSU: Payload blade %s:AVM_DHCPD_SW_IPMC_PLD_RTM_FILE1 is missing in the file path",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str, NCSFL_SEV_WARNING);
   }

   tmp_str = strrchr(file_name, '/');
   *(++tmp_str) = '\0';
  
   /* Check for AVM_DHCPD_SW_IPMC_PLD_RTM_FILE2 */ 
   strcat(file_name, AVM_DHCPD_SW_IPMC_PLD_RTM_FILE2);
   if (-1 == access(file_name, F_OK))
   {
      sprintf(str,"AVM-SSU: Payload blade %s:AVM_DHCPD_SW_IPMC_PLD_RTM_FILE2 is missing in the file path",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str, NCSFL_SEV_WARNING);
   }

   /**************************************************************
      PUSH Install time and software version into PSSV
   **************************************************************/
    
   if (label_no == LABEL1)
   {
      param_id1 = ncsAvmEntDHCPConfInstallTimeLabel1_ID;
      param_id2 = ncsAvmEntDHCPConfLabel1SwVersion_ID; 
   }
   else if (label_no == LABEL2)    
   {
      param_id1 = ncsAvmEntDHCPConfInstallTimeLabel2_ID;
      param_id2 = ncsAvmEntDHCPConfLabel2SwVersion_ID; 
   }
   else
      return NCSCC_RC_FAILURE;
 
   /* Push install time*/
   m_AVM_SSU_PSSV_PUSH_STR(cb, label->install_time.install_time, param_id1, ent_info, 8);

   /* push software version */
   m_AVM_SSU_PSSV_PUSH_STR(cb, label->sw_version.name, param_id2, ent_info, label->sw_version.length);

   return NCSCC_RC_SUCCESS;
}

/**********************************************************************
 ******
 * Name          : avm_set_preferred_label
 *
 * Description   : This function handles the preferred label change and
 *                  the corresponding state changes.
 *
 * Arguments     : AVM_PER_LABEL_CONF - Pointer to Label configuration
 *                 NCSMIB_PARAM_VAL - Param value.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 *****/

extern uns32
avm_set_preferred_label(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info, AVM_ENT_DHCP_CONF *dhcp_conf, NCSMIB_PARAM_VAL param_val)
{
   uns8               logbuf[500];
 
   /* Check whether request is for setting it to the same value */
   if (!AVM_DHCONF_MEMCMP_LEN(param_val.info.i_oct, dhcp_conf->pref_label.name,
                              dhcp_conf->pref_label.length, param_val.i_length))
   {
      /* Same val...return success */
      return NCSCC_RC_SUCCESS;
   }

   logbuf[0] = '\0';

   /* First Check that the preferred label name is same as label 1
   * or label 2 name. Only then go ahead and set the preferred label
   */
   if (!AVM_DHCONF_MEMCMP_LEN(param_val.info.i_oct, dhcp_conf->label1.name.name,
                              dhcp_conf->label1.name.length, param_val.i_length))
   {
      /* Preferred label is getting set to label 1 */
      if ((SSU_NO_CONFIG == dhcp_conf->label1.status) ||
          (SSU_COMMIT_PENDING == dhcp_conf->label1.status))
         return NCSCC_RC_FAILURE;
      else
      {
         dhcp_conf->default_label = &dhcp_conf->label1;
         dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_1;
         dhcp_conf->default_chg = TRUE;

         if (SSU_UPGD_FAILED == dhcp_conf->label1.status)
            dhcp_conf->label1.status = SSU_INSTALLED;
      }

      memset(dhcp_conf->pref_label.name, '\0', AVM_NAME_STR_LENGTH);
      /* Set the preferred label */
      dhcp_conf->pref_label.length = param_val.i_length;
      memcpy(dhcp_conf->pref_label.name, param_val.info.i_oct,
                   param_val.i_length);
      /* Push the preferred label into pssv */
      m_AVM_SSU_PSSV_PUSH_STR(cb, dhcp_conf->pref_label.name, ncsAvmEntDHCPConfPrefLabel_ID, ent_info, dhcp_conf->pref_label.length);

      /* Do the state changes accordingly */
      m_AVM_PREF_LABEL_SET_STATE_CHAGES(dhcp_conf->label1);

     /* Push the label1 status into PSSV */
      m_AVM_SSU_PSSV_PUSH_INT(cb, ent_info->dhcp_serv_conf.label1.status, ncsAvmEntDHCPConfLabel1Status_ID, ent_info);

   }
   else if (!AVM_DHCONF_MEMCMP_LEN(param_val.info.i_oct, dhcp_conf->label2.name.name,
                                   dhcp_conf->label2.name.length, param_val.i_length))
   {
      /* Preferred label is getting set to label 2 */
      if ((SSU_NO_CONFIG == dhcp_conf->label2.status) ||
          (SSU_COMMIT_PENDING == dhcp_conf->label2.status))
         return NCSCC_RC_FAILURE;
      else
      {
         dhcp_conf->default_label = &dhcp_conf->label2;
         dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_2;
         dhcp_conf->default_chg = TRUE;

         if (SSU_UPGD_FAILED == dhcp_conf->label2.status)
            dhcp_conf->label2.status = SSU_INSTALLED;
      }

      memset(dhcp_conf->pref_label.name, '\0', AVM_NAME_STR_LENGTH);
      /* Set the preferred label */
      dhcp_conf->pref_label.length = param_val.i_length;
      memcpy(dhcp_conf->pref_label.name,
         param_val.info.i_oct, param_val.i_length);
      /* Push the preferred label into pssv */
      m_AVM_SSU_PSSV_PUSH_STR(cb, dhcp_conf->pref_label.name, ncsAvmEntDHCPConfPrefLabel_ID, ent_info, dhcp_conf->pref_label.length);

      /* Do the state changes accordingly */
      m_AVM_PREF_LABEL_SET_STATE_CHAGES(dhcp_conf->label2);
      /* Push the label2 status into PSSV */
      m_AVM_SSU_PSSV_PUSH_INT(cb, ent_info->dhcp_serv_conf.label2.status, ncsAvmEntDHCPConfLabel2Status_ID, ent_info);
   }
   else
      return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}


/**********************************************************************
 ******
 * Name          : avm_set_label_state_to_install
 *
 * Description   : This function chcks whether the state of the label can
 *                 be changed to INSTALLED from NO_CONFIG..
 *
 * Arguments     : AVM_PER_LABEL_CONF - Pointer to Label configuration
 *                 NCSMIB_PARAM_VAL - Param value.
 *
 * Return Values : None..
 *
 * Notes         : None.
 ***********************************************************************
 *****/
extern void
avm_set_label_state_to_install(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info, AVM_ENT_DHCP_CONF *dhcp_conf, AVM_PER_LABEL_CONF *label_cnf, AVM_LABEL_NUM label_no)
{
   NCSMIB_PARAM_ID param_id;

   /* Check whether TFTP server IP is set */
   if (0 == (long)dhcp_conf->tftp_serve_ip)
      return;

   /* Check whether Name is configured */
   if (0 == label_cnf->name.length)
      return;

   /* Check if file name is set */
   if (0 == label_cnf->file_name.length)
      return;

   /* All tests are passed..set the status to INSTALLED */


   /* set and Push the label status into pssv */
   if(label_no == LABEL1)
      param_id = ncsAvmEntDHCPConfLabel1Status_ID;
   else if(label_no == LABEL2)
      param_id = ncsAvmEntDHCPConfLabel2Status_ID;
   else
      return;

   label_cnf->status = SSU_INSTALLED;
   m_AVM_SSU_PSSV_PUSH_INT(cb, label_cnf->status, param_id, ent_info);

   return;
}

/*****************************************************************************
  * Name          : avm_ssu_dhconf_set
  *
  * Description   : This function is invoked to set the DHCP configuration
  *                 by invoking the ncs_ssu_dhconf.pl script.
  *
  * Arguments     : AVM_ENT_DHCP_CONF*    - Pointer to AvM DHCP configuration
  *                 AVM_PER_LABEL_CONF - Pointer to per label configuration
  *                 dhcp_restart - Boolean to check restart of DHCP server
  *
  * Return Values : AVM_DHCP_CONF_CHANGE/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
*****************************************************************************/

extern uns32
avm_ssu_dhconf_set(AVM_CB_T *avm_cb, AVM_ENT_DHCP_CONF  *dhcp_conf, AVM_PER_LABEL_CONF *dhcp_label, uns8 dhcp_restart)
{
   uns8  tftpserver[AVM_IP_ADDR_STR_LEN];
   uns8 script_buf[AVM_SCRIPT_BUF_LEN], log_str[AVM_SCRIPT_BUF_LEN + 30];
   uns8 *host1, *host2, *mac1, *mac2, *filename;
   uns32 rc;
   char pxe_file_name[AVM_NAME_STR_LENGTH + 11];
   char *pStrstr = NULL;

   /* check the validity of arguments */
   if (dhcp_conf == NULL)
   {
      m_AVM_LOG_DEBUG("AVM-SSU: DHCP conf is NULL\n",NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   if (dhcp_label == NULL)
      if ((dhcp_label = dhcp_conf->default_label) == NULL)
      {
         m_AVM_LOG_DEBUG("AVM-SSU: DHCP conf default label is NULL\n",NCSFL_SEV_ERROR);
         return NCSCC_RC_FAILURE;
      }

   /* retrieve DHCP configuraiton information */
   host1 = dhcp_conf->host_name_bc1;
   host2 = dhcp_conf->host_name_bc2;
   mac1 = dhcp_conf->mac_address[0];
   mac2 = dhcp_conf->mac_address[1];

   /* Get the configured Filename in local var */
   memset(pxe_file_name, '\0', sizeof(pxe_file_name));
   sprintf(pxe_file_name, "%s", dhcp_label->file_name.name);

   /* Check for pxelinux.0 */
   pStrstr = strstr(pxe_file_name, AVM_DHCPD_SW_PXE_FILE);

   /* If not found, assuming that pxe_file_name is either /tftpboot/7221 or /tftpboot/7221/pxelinux.0 */
   /* concatenate the pxelinux.0 */
   if (pStrstr == NULL)
   {
      strcat(pxe_file_name,AVM_DHCPD_SW_PXE_FILE); 
   }

   filename = pxe_file_name;

   sprintf(tftpserver, "%d.%d.%d.%d", dhcp_conf->tftp_serve_ip[0] & 0xff,
                                      dhcp_conf->tftp_serve_ip[1] & 0xff,
                                      dhcp_conf->tftp_serve_ip[2] & 0xff,
                                      dhcp_conf->tftp_serve_ip[3] & 0xff);

   sprintf(script_buf, "%s %s %s %d %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x %s %s %d ",
           AVM_SSU_DHCONF_SCRIPT, host1, host2, dhcp_conf->net_boot,
           mac1[0], mac1[1], mac1[2], mac1[3], mac1[4], mac1[5],
           mac2[0], mac2[1], mac2[2], mac2[3], mac2[4], mac2[5],
           filename, tftpserver, dhcp_restart);

   sprintf(log_str, "AVM-SSU: %s %s %s ", "DHCP script buffer = ",script_buf,"\n");

   m_AVM_LOG_DEBUG(log_str,NCSFL_SEV_NOTICE);
   /* Invoke script to reconfigure the DHCP server */
   rc = m_NCS_SYSTEM(script_buf);
   if (rc)
   {
      uns32 ret_val;
      /* exit value of the script, is in high byte, that is rc >> 8 */
      ret_val = rc >> 8;
      /* check if the /etc/init.d/dhcp stop failed.  If it is failed, start a timer for 1 minute.
         When it gets expired, try again to /etc/init.d/dhcp stop */
      if ( 1 == ret_val ) 
      {
          /* if the DHCP FAIL timer is not running, start it*/
          if( avm_cb->dhcp_fail_tmr.status != AVM_TMR_RUNNING ) 
             m_AVM_SSU_DHCP_FAIL_TMR_START(avm_cb); 
          m_AVM_LOG_DEBUG("AVM-SSU: execution of AVM_SSU_DHCONF_SCRIPT failed because of dhcp stop failed",NCSFL_SEV_NOTICE);
          m_AVM_LOG_DEBUG("AVM-SSU: DHCP_FAIL_TMR started",NCSFL_SEV_NOTICE);
      }
      else
      {
          m_AVM_LOG_DEBUG("AVM-SSU: execution of AVM_SSU_DHCONF_SCRIPT failed due to some unknown reason. DEBUG IT",NCSFL_SEV_ERROR);
      }   
      return NCSCC_RC_FAILURE; 
   }
   else
   {
      /* Check if the DHCP FAIL timer is running, if it is running, stop it */ 
      if( avm_cb->dhcp_fail_tmr.status == AVM_TMR_RUNNING )
      {
          avm_stop_tmr(avm_cb, &avm_cb->dhcp_fail_tmr);
          m_AVM_LOG_DEBUG("AVM-SSU: DHCP FAIL timer was running & stopped now",NCSFL_SEV_NOTICE);
      }
   }
   return AVM_DHCP_CONF_CHANGE;
}

/*****************************************************************************
  * Name          : avm_ssu_dhconf
  *
  * Description   : This function is invoked when a FRU is inserted in
  *                 the Chassis. It validates the FRU and goes ahead to
  *                 update the DHCP configuration file for the blade.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Insert
  *                 ion Pending Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
*****************************************************************************/

extern uns32
avm_ssu_dhconf(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt, uns8 dhcp_restart)
{
   AVM_ENT_DHCP_CONF  *dhcp_conf;
   AVM_PER_LABEL_CONF *dhcp_label;
   AVM_PER_LABEL_CONF dhcp_dummy_label;
   uns8               mac_addr[AVM_MAC_DATA_LEN],logbuf[500];
   uns32 conf_chg = TRUE, rc;

   HPI_EVT_T *hpi_evt = NULL;

   if (fsm_evt != NULL)
      hpi_evt = ((AVM_EVT_T*)fsm_evt)->evt.hpi_evt;

   m_AVM_LOG_FUNC_ENTRY("avm_ssu_dhconf");

   if(AVM_ENT_INFO_NULL == ent_info)
   {
      m_AVM_LOG_GEN("AVM-SSU: avm_ssu_dhconf: No Entity existing ", ent_info->depends_on.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_WARNING);
      return NCSCC_RC_SUCCESS;
   }
   dhcp_conf = &ent_info->dhcp_serv_conf;

   /*********************************************************************************************
    * Check If the uType is not INTEG and ipmc upgrade state is IPMC_UNLOCKED. Then skip the    *
    * below logic which modifies the dhcpd.conf file                                            *
    ********************************************************************************************/
    if((dhcp_conf->upgrade_type != INTEG) && (dhcp_conf->ipmc_upgd_state == IPMC_UPGD_DONE))
    {
       sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : dhconf set is skipped, as the IPMC upgrade is in progress",ent_info->ep_str.name);
       m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
       /* reset the ipmc_upgd_state */
       dhcp_conf->ipmc_upgd_state = 0;
       m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
       return NCSCC_RC_SUCCESS;   
    }    
    else if ((dhcp_conf->upgrade_type == INTEG) && (dhcp_conf->ipmc_upgd_state == IPMC_UPGD_DONE)) 
    {
       /* reset the ipmc_upgd_state */
       dhcp_conf->ipmc_upgd_state = 0;
       m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
    }

   /* Check if Net Booting is enabled And Default label is set.If yes then only proceed further *
    * Otherwise remove the mac entry from the  /etc/dhcpd.conf file        */
   if ((dhcp_conf->net_boot == NCS_SNMP_FALSE) || (dhcp_conf->default_label == NULL))
   {
      if(dhcp_conf->net_boot == NCS_SNMP_FALSE)  
         m_AVM_LOG_DEBUG("AVM-SSU: Netboot is not enabled\n",NCSFL_SEV_NOTICE);
      if(dhcp_conf->default_label == NULL)
         m_AVM_LOG_DEBUG("AVM-SSU: dhcp_conf default_label is NULL\n",NCSFL_SEV_NOTICE);
      dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_NULL;
      /* check for MAC address availability in the inventory data of entity */
      if (hpi_evt != NULL)
      {
         if (hpi_evt->inv_data.oem_inv_data.num_mac_entries < 2)
         {
            m_AVM_LOG_DEBUG("AVM-SSU: MAC address entries missing in inventory data\n",NCSFL_SEV_ERROR);
            return NCSCC_RC_FAILURE;
         }
         /* fill the MAC addresses in entity information */
         memcpy(dhcp_conf->mac_address[0],
                      hpi_evt->inv_data.oem_inv_data.interface_mac_addr[0], AVM_MAC_DATA_LEN);

         memcpy(dhcp_conf->mac_address[1],
                      hpi_evt->inv_data.oem_inv_data.interface_mac_addr[1], AVM_MAC_DATA_LEN);
      }
      
      memset(mac_addr, 0, AVM_MAC_DATA_LEN); /* Initialise the mac address with 0.0.0.0.0.0 to compare */ 
      /* check for the mac addresses, if these are 0.0.0.0.0.0, the insertion pending event is from F101, so just return */   
      if ((!m_NCS_MEMCMP(mac_addr, dhcp_conf->mac_address[0], AVM_MAC_DATA_LEN)) && 
          (!m_NCS_MEMCMP(mac_addr, dhcp_conf->mac_address[1], AVM_MAC_DATA_LEN)))
      {
         return NCSCC_RC_SUCCESS;
      }
  
      if (avm_ssu_clear_mac(avm_cb, ent_info))
      { 
         strcpy(dhcp_dummy_label.name.name, "dummy_label");
         strcpy(dhcp_dummy_label.file_name.name, "dummy_file"); 
         dhcp_label = &dhcp_dummy_label;
         dhcp_conf->net_boot = 0;
         m_AVM_LOG_DEBUG("AVM-SSU: Dummy label passed to avm_ssu_dhconf_set to remove the mac address from the dhcpd.conf file \n",NCSFL_SEV_NOTICE);
         rc = avm_ssu_dhconf_set(avm_cb, dhcp_conf, dhcp_label, dhcp_restart);
      } 
      return NCSCC_RC_SUCCESS;
   }
   /* Start upgd success timer if blade supports net-boot and
    * other label is in committed state */
   if((SSU_COMMITTED == dhcp_conf->default_label->other_label->status) &&
      (SSU_COMMITTED != dhcp_conf->default_label->status))
   {
      dhcp_conf->upgd_prgs = TRUE;
      m_AVM_SSU_UPGR_TMR_START(avm_cb, ent_info);
   }

   /*
    * Check if we really need to update DHCP configuration and
    * restart the DHCP server. If no then return success.
    */
   if ((FALSE == dhcp_conf->default_chg) &&
       (FALSE == dhcp_conf->default_label->conf_chg) &&
       (0 == access(AVM_DHCPD_CONF, F_OK)))
   {
      m_AVM_LOG_DEBUG("AVM-SSU: DHCP configuration change may not be required unless its a new blade\n",NCSFL_SEV_NOTICE);
      conf_chg = FALSE;
      dhcp_label = dhcp_conf->default_label;
   }
   else
   {
      /* Continue DHCP updation and set the config change to FALSE if done successfully. */
      dhcp_label = dhcp_conf->default_label;
      dhcp_conf->default_chg = FALSE;
      /* update the label status */
      switch (dhcp_conf->default_label->status)
      {
         case SSU_INSTALLED:
            /* if other label status is NO_CONFIG or INSTALLED then set default to COMMITTED */
            if ((dhcp_conf->default_label->other_label->status == SSU_NO_CONFIG) ||
               (dhcp_conf->default_label->other_label->status == SSU_INSTALLED))
            {
               dhcp_conf->default_label->status = SSU_COMMITTED;
               dhcp_conf->curr_act_label = dhcp_conf->default_label;
               dhcp_conf->cur_act_label_num = dhcp_conf->def_label_num;
               avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmCurrActiveLabelChange_ID);
               sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : CurrActiveLabel SET to:%s",ent_info->ep_str.name,dhcp_conf->curr_act_label->name.name);
               m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
               /* Push the current active label into pssv */
               m_AVM_SSU_PSSV_PUSH_STR(avm_cb, dhcp_conf->curr_act_label->name.name, ncsAvmEntDHCPConfCurrActiveLabel_ID, ent_info, dhcp_conf->curr_act_label->name.length);
               /* Push the Label1 state into PSSV */
               m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label1.status, ncsAvmEntDHCPConfLabel1Status_ID, ent_info);
               /* Push the Label2 state into PSSV */
               m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label2.status, ncsAvmEntDHCPConfLabel2Status_ID, ent_info);
            }
            else
            /* if other label status is SSU_COMMITTED then set default to COMMITTED_PEND */
            if (dhcp_conf->default_label->other_label->status == SSU_COMMITTED)
            {
               dhcp_conf->default_label->status = SSU_COMMIT_PENDING;
               dhcp_conf->curr_act_label = dhcp_conf->default_label;
               dhcp_conf->cur_act_label_num = dhcp_conf->def_label_num;
               if (dhcp_conf->def_label_num == AVM_DEFAULT_LABEL_1)
               {
                  dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_2;
               }
               else
               { 

                  dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_1;
               } 
               dhcp_conf->default_label = dhcp_conf->default_label->other_label;
      
               memset(dhcp_conf->pref_label.name, '\0', AVM_NAME_STR_LENGTH);
               /* Set the preferred label also to other label */
               dhcp_conf->pref_label.length = dhcp_conf->default_label->name.length;
               memcpy(dhcp_conf->pref_label.name, dhcp_conf->default_label->name.name,
                   dhcp_conf->pref_label.length);
               /* Push the preferred label into pssv */
               m_AVM_SSU_PSSV_PUSH_STR(avm_cb, dhcp_conf->pref_label.name, ncsAvmEntDHCPConfPrefLabel_ID, ent_info, dhcp_conf->pref_label.length);
               dhcp_conf->default_chg = TRUE;
               avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmCurrActiveLabelChange_ID);
               sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : CurrActiveLabel SET to : %s",ent_info->ep_str.name,dhcp_conf->curr_act_label->name.name);
               m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
               /* Push the current active label into pssv */
               m_AVM_SSU_PSSV_PUSH_STR(avm_cb, dhcp_conf->curr_act_label->name.name, ncsAvmEntDHCPConfCurrActiveLabel_ID, ent_info, dhcp_conf->curr_act_label->name.length);
               /* Push the Label1 state into PSSV */
               m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label1.status, ncsAvmEntDHCPConfLabel1Status_ID, ent_info);
               /* Push the Label2 state into PSSV */
               m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label2.status, ncsAvmEntDHCPConfLabel2Status_ID, ent_info);
            }
            break;

         case SSU_COMMITTED:
            /* if other label status is COMMIT_PEND then set it to UPGD_FAILED */
            if (dhcp_conf->default_label->other_label->status == SSU_COMMIT_PENDING)
            {
               dhcp_conf->default_label->other_label->status = SSU_UPGD_FAILED;
               /* does it need to be check pointed - TBD - JPL */

               dhcp_conf->curr_act_label = dhcp_conf->default_label;
               dhcp_conf->cur_act_label_num = dhcp_conf->def_label_num;
               avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmCurrActiveLabelChange_ID);
               sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s:CurrActiveLabel SET to: %s",ent_info->ep_str.name,dhcp_conf->curr_act_label->name.name);
               m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
               /* Push the current active label into pssv */
               m_AVM_SSU_PSSV_PUSH_STR(avm_cb, dhcp_conf->curr_act_label->name.name, ncsAvmEntDHCPConfCurrActiveLabel_ID, ent_info, dhcp_conf->curr_act_label->name.length);
               /* Push the Label1 state into PSSV */
               m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label1.status, ncsAvmEntDHCPConfLabel1Status_ID, ent_info);
               /* Push the Label2 state into PSSV */
               m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label2.status, ncsAvmEntDHCPConfLabel2Status_ID, ent_info);
               avm_ssu_dhcp_integ_rollback (avm_cb, ent_info);
               dhcp_conf->upgd_prgs = FALSE;
            }
            else if (dhcp_conf->default_label->other_label->status == SSU_COMMITTED)
            {
               if(m_NCS_MEMCMP(dhcp_conf->curr_act_label->name.name,dhcp_conf->default_label->name.name, dhcp_conf->default_label->name.length))
               {
                  dhcp_conf->curr_act_label = dhcp_conf->default_label;
                  dhcp_conf->cur_act_label_num = dhcp_conf->def_label_num;
                  avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmCurrActiveLabelChange_ID);
                  sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s: CurrActiveLabel SET to: %s",ent_info->ep_str.name, dhcp_conf->curr_act_label->name.name);
                  m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
                  /* Push the current active label into pssv */
                  m_AVM_SSU_PSSV_PUSH_STR(avm_cb, dhcp_conf->curr_act_label->name.name, ncsAvmEntDHCPConfCurrActiveLabel_ID, ent_info, dhcp_conf->curr_act_label->name.length);
               } 
            }
            /* if other label status is NO_CONFIG or INSTALLED then set the current active label */
            else if ((dhcp_conf->default_label->other_label->status == SSU_NO_CONFIG) ||
               (dhcp_conf->default_label->other_label->status == SSU_INSTALLED))
            {
               dhcp_conf->curr_act_label = dhcp_conf->default_label;
               dhcp_conf->cur_act_label_num = dhcp_conf->def_label_num;
               avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmCurrActiveLabelChange_ID);
               sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : CurrActiveLabel SET to: %s",ent_info->ep_str.name,dhcp_conf->curr_act_label->name.name);
               m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
               /* Push the current active label into pssv */
               m_AVM_SSU_PSSV_PUSH_STR(avm_cb, dhcp_conf->curr_act_label->name.name, ncsAvmEntDHCPConfCurrActiveLabel_ID, ent_info, dhcp_conf->curr_act_label->name.length);
            }
            break;

         default:
            break;
      }
   }

   /* check for MAC address availability in the inventory data of entity */
   if (hpi_evt != NULL)
   {
      if (hpi_evt->inv_data.oem_inv_data.num_mac_entries < 2)
      {
         m_AVM_LOG_DEBUG("AVM-SSU: MAC address entries missing in inventory data\n",NCSFL_SEV_ERROR);
         return NCSCC_RC_FAILURE;
      }
      /* compare the MAC address and configuration checks */
      if ((conf_chg == FALSE) && 
          (!m_NCS_MEMCMP(dhcp_conf->mac_address[0],hpi_evt->inv_data.oem_inv_data.interface_mac_addr[0], AVM_MAC_DATA_LEN)) && 
          (!m_NCS_MEMCMP(dhcp_conf->mac_address[1],hpi_evt->inv_data.oem_inv_data.interface_mac_addr[1], AVM_MAC_DATA_LEN)))
      {
         m_AVM_LOG_DEBUG("AVM-SSU: There was no configuration and Mac address change\n",NCSFL_SEV_NOTICE);
         return NCSCC_RC_SUCCESS;
      }

      /* fill the MAC addresses in entity information */
      memcpy(dhcp_conf->mac_address[0],
                   hpi_evt->inv_data.oem_inv_data.interface_mac_addr[0], AVM_MAC_DATA_LEN);

      memcpy(dhcp_conf->mac_address[1],
                   hpi_evt->inv_data.oem_inv_data.interface_mac_addr[1], AVM_MAC_DATA_LEN);
   }
   else if (conf_chg == FALSE)
   {
      m_AVM_LOG_DEBUG("AVM-SSU: There was no Configuration change\n",NCSFL_SEV_NOTICE);
      return NCSCC_RC_SUCCESS;
   }

   /* set the DHCP configuraiton information */
   rc = avm_ssu_dhconf_set(avm_cb, dhcp_conf, dhcp_label, dhcp_restart);
   
   /*  if there is a configuration change, remove the old data, if any */
   if (rc == AVM_DHCP_CONF_CHANGE)
   {
      avm_ssu_clear_mac(avm_cb, ent_info);
      /*  Update the change flag to FALSE only when dhcp.conf file has been updated successfully */
      m_AVM_LOG_DEBUG("AVM-SSU: DHCP CONF file updation SUCCESS, Set conf_chg flag to FALSE \n",NCSFL_SEV_NOTICE);
      dhcp_conf->default_label->conf_chg = FALSE;
   }
   else
   {
      sysf_sprintf(logbuf, "AVM-SSU: DHCP CONF file updation FAILED, rc =%d, dhcp_conf->default_chg =%d, dhcp_conf->default_label->conf_chg =%d \n", rc, dhcp_conf->default_chg, dhcp_conf->default_label->conf_chg);
      m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
   }
   /* async update for both configuration and state changes */
   m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_CONF_CHG);
   m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);
   return rc;
}

/*****************************************************************************
  PROCEDURE NAME : avm_ssu_dhcp_tmr_reconfig

  DESCRIPTION    : AvM SSU timer expiry DHCP reconfigure

  ARGUMENTS      : uarg - ptr to the AvM timer block

  RETURNS        : void

  NOTES         : None
*****************************************************************************/
extern uns32
avm_ssu_dhcp_tmr_reconfig (AVM_CB_T *avm_cb, AVM_EVT_T  *hpi_evt)
{
   AVM_ENT_INFO_T *ent_info;

   /* just return if it is not an insertion pending event */
   if (hpi_evt->evt.hpi_evt->hpi_event.EventDataUnion.HotSwapEvent.HotSwapState != SAHPI_HS_STATE_INSERTION_PENDING)
      return NCSCC_RC_SUCCESS;

   ent_info = avm_find_ent_info(avm_cb, &hpi_evt->evt.hpi_evt->entity_path);

   if(AVM_ENT_INFO_NULL == ent_info)
   {
      m_AVM_LOG_GEN("AVM-SSU: No Entity in the DB with Entity Path ", hpi_evt->evt.hpi_evt->entity_path.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_WARNING);
      return NCSCC_RC_FAILURE;
   }
   return (avm_ssu_dhconf(avm_cb, ent_info, hpi_evt, 0));
}


/*****************************************************************************
  PROCEDURE NAME : avm_ssu_dhcp_rollback

  DESCRIPTION    : Function to cause rollback.

  ARGUMENTS      : uarg - ptr to the AvM timer block

  RETURNS        : void

  NOTES         : None
*****************************************************************************/
extern void
avm_ssu_dhcp_rollback (AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info)
{
   AVM_ENT_DHCP_CONF *dhcp_conf = &ent_info->dhcp_serv_conf;
   uns8 str[AVM_LOG_STR_MAX_LEN]; 

   str[0] = '\0';

  
   if ((dhcp_conf->curr_act_label->status == SSU_COMMIT_PENDING) &&
       (dhcp_conf->curr_act_label->other_label->status == SSU_COMMITTED))
   {
      /* stop the upgrade timer if running */
      if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
         avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

      dhcp_conf->curr_act_label->status = SSU_UPGD_FAILED;
      dhcp_conf->default_label = dhcp_conf->curr_act_label->other_label;
      
      dhcp_conf->curr_act_label = dhcp_conf->default_label;
      /* Push the current active label into pssv */
      m_AVM_SSU_PSSV_PUSH_STR(avm_cb, dhcp_conf->curr_act_label->name.name, ncsAvmEntDHCPConfCurrActiveLabel_ID, ent_info, dhcp_conf->curr_act_label->name.length);

      /* Set the default Label number as well */
      if (dhcp_conf->default_label == &dhcp_conf->label1)
      {
         /* Set the preferred label */
         memset(dhcp_conf->pref_label.name, '\0', AVM_NAME_STR_LENGTH);
         dhcp_conf->pref_label.length = dhcp_conf->label1.name.length;
         memcpy(dhcp_conf->pref_label.name, dhcp_conf->label1.name.name,
                      dhcp_conf->label1.name.length);
         /* Push the preferred label into pssv */
         m_AVM_SSU_PSSV_PUSH_STR(avm_cb, dhcp_conf->pref_label.name, ncsAvmEntDHCPConfPrefLabel_ID, ent_info, dhcp_conf->pref_label.length);

         /* Do the state changes accordingly */
         m_AVM_PREF_LABEL_SET_STATE_CHAGES(dhcp_conf->label1);
         dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_1;

         /* Push it into PSSV */
         m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label1.status, ncsAvmEntDHCPConfLabel1Status_ID, ent_info);
      }
      else
      {
         /* Set the preferred label */
         memset(dhcp_conf->pref_label.name, '\0', AVM_NAME_STR_LENGTH);
         dhcp_conf->pref_label.length = dhcp_conf->label2.name.length;
         memcpy(dhcp_conf->pref_label.name, dhcp_conf->label2.name.name,
                      dhcp_conf->label2.name.length);
         /* Push the preferred label into pssv */
         m_AVM_SSU_PSSV_PUSH_STR(avm_cb, dhcp_conf->pref_label.name, ncsAvmEntDHCPConfPrefLabel_ID, ent_info, dhcp_conf->pref_label.length);

         /* Do the state changes accordingly */
         m_AVM_PREF_LABEL_SET_STATE_CHAGES(dhcp_conf->label2);
         dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_2;

         /* Push it into PSSV */
         m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label2.status, ncsAvmEntDHCPConfLabel2Status_ID, ent_info);

      }

      /* Current Active Label No. requires update */
      dhcp_conf->cur_act_label_num = dhcp_conf->def_label_num;
      /* async update */
      m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);

      /* rollback the DHCP configuration also */
      avm_ssu_dhconf_set(avm_cb, &ent_info->dhcp_serv_conf, NULL, 1);

   }
   avm_ssu_dhcp_integ_rollback (avm_cb, ent_info);
   return;
}
/*****************************************************************************
  PROCEDURE NAME : avm_ssu_dhcp_integ_rollback

  DESCRIPTION    : .

  ARGUMENTS      : uarg - ptr to the AvM timer block

  RETURNS        : void

  NOTES         : None
*****************************************************************************/
extern void
avm_ssu_dhcp_integ_rollback (AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info)
{
   AVM_ENT_DHCP_CONF *dhcp_conf = &ent_info->dhcp_serv_conf;
   uns8 str[AVM_LOG_STR_MAX_LEN];
   AVM_EVT_T          fsm_evt;
   NCSMIB_ARG         ssu_dummy_mib_obj;
   AVM_FSM_STATES_T   ssu_temp_curr_state;

   uns32 rc = NCSCC_RC_FAILURE;
   str[0]   = '\0';


   if(dhcp_conf->upgrade_type == INTEG)
   {
      avm_cb->upgrade_prg_evt = AVM_ROLLBACK_TRIGGERED;
      avm_cb->upgrade_module = NCS_BIOS_IPMC;
      sprintf(str,"AVM-SSU: Payload blade %s: INTEG: Rollback triggered ",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_NOTICE);
      avm_send_boot_upgd_trap(avm_cb,ent_info,ncsAvmUpgradeProgress_ID);

      if((dhcp_conf->pld_bld_ipmc_status) || (dhcp_conf->pld_rtm_ipmc_status))
      {
         /* Rollback the IPMC */
         sprintf(str,"AVM-SSU: Payload blade %s: INTEG: IPMC Rollback triggered ",ent_info->ep_str.name);
         m_AVM_LOG_DEBUG(str,NCSFL_SEV_NOTICE);
         avm_upgrade_ipmc_trigger(avm_cb,ent_info);
      }
      else
      {
         /* Should I reset the blade ?? - TBD - JPL */

         /* Would go for hard-reset strategy             */
         /* If that fails, admin would need to interfere */
         sprintf(str,"AVM-SSU: Payload blade %s: INTEG: SW And BIOS Rollback triggered ",ent_info->ep_str.name);
         m_AVM_LOG_DEBUG(str,NCSFL_SEV_NOTICE);
         fsm_evt.fsm_evt_type = AVM_EVT_ADM_HARD_RESET_REQ;
         fsm_evt.evt.mib_req  = &ssu_dummy_mib_obj;
         ssu_temp_curr_state  = ent_info->current_state;
         rc = avm_fsm_handler(avm_cb, ent_info, &fsm_evt);
         if((rc != NCSCC_RC_SUCCESS) && (ssu_temp_curr_state != AVM_ENT_FAULTY))
         {
            m_MMGR_FREE_MIB_OCT(fsm_evt.evt.mib_req->rsp.add_info);
         }
         m_AVM_SEND_CKPT_UPDT_ASYNC_ADD(avm_cb, ent_info, AVM_CKPT_ENT_ADM_OP);
      }
   }
   else if(dhcp_conf->upgrade_type == IPMC)
   {
     /* This is the wrong place. Log a CRITICAL ERROR */
     sprintf(str,"AVM-SSU: Payload blade %s: IPMC Rollback triggered ",ent_info->ep_str.name);
     m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);

   }
   else if(dhcp_conf->upgrade_type == SW_BIOS)
   {
      avm_cb->upgrade_prg_evt = AVM_ROLLBACK_TRIGGERED;
      avm_cb->upgrade_module = NCS_BIOS;
      sprintf(str,"AVM-SSU: Payload blade %s: SW_BIOS Rollback triggered ",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_NOTICE);
      avm_send_boot_upgd_trap(avm_cb,ent_info,ncsAvmUpgradeProgress_ID);
   }
   return;
}

/*****************************************************************************
  PROCEDURE NAME : avm_ssu_clear_mac

  DESCRIPTION    : Function to clear old mac address.

  ARGUMENTS      : uarg - ptr to the AvM control block and entity info

  RETURNS        : uns32 

  NOTES         : None
*****************************************************************************/
static uns32 
avm_ssu_clear_mac (AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info_in)
{
   AVM_ENT_DHCP_CONF *dhcp_conf1 = NULL, *dhcp_conf = &ent_info_in->dhcp_serv_conf;
   AVM_ENT_INFO_T    *ent_info;
   AVM_ENT_PATH_STR_T ep;
   uns32 mac_change = 0, dhcp_entry_change = 0;

   m_AVM_LOG_FUNC_ENTRY("AVM-SSU: avm_ssu_clear_mac");
   memset(ep.name, '\0', AVM_MAX_INDEX_LEN); 
   ep.length = 0;

   for(ent_info = avm_find_next_ent_str_info(cb, &ep); 
       ent_info != AVM_ENT_INFO_NULL; 
       ent_info = (AVM_ENT_INFO_T*) avm_find_next_ent_str_info(cb, &ep))
   {
      mac_change = 0;
      if (ent_info_in == ent_info)
      {
         memcpy(ep.name, ent_info->ep_str.name, AVM_MAX_INDEX_LEN);
         ep.length = m_NCS_OS_NTOHS(ent_info->ep_str.length);
         continue;
      }
      dhcp_conf1 = &ent_info->dhcp_serv_conf;

      /* check for MAC address and erase if it is same */
      if ((!m_NCS_MEMCMP(dhcp_conf->mac_address[0], dhcp_conf1->mac_address[0], AVM_MAC_DATA_LEN)) ||
          (!m_NCS_MEMCMP(dhcp_conf->mac_address[1], dhcp_conf1->mac_address[0], AVM_MAC_DATA_LEN)))
      {
         mac_change = 1;
         memset(dhcp_conf1->mac_address[0], 0, AVM_MAC_DATA_LEN);
      }

      if ((!m_NCS_MEMCMP(dhcp_conf->mac_address[0], dhcp_conf1->mac_address[1], AVM_MAC_DATA_LEN)) ||
          (!m_NCS_MEMCMP(dhcp_conf->mac_address[1], dhcp_conf1->mac_address[1], AVM_MAC_DATA_LEN)))
      {
         mac_change = 1;
         memset(dhcp_conf1->mac_address[1], 0, AVM_MAC_DATA_LEN);
      }

       /* async update for MAC configuration change */
      if (mac_change)
      {
         mac_change = 0;
         dhcp_entry_change = 1;
         m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(cb, ent_info, AVM_CKPT_ENT_DHCP_CONF_CHG);
      }

      memcpy(ep.name, ent_info->ep_str.name, AVM_MAX_INDEX_LEN);
      ep.length = m_NCS_OS_NTOHS(ent_info->ep_str.length);
   }
   return dhcp_entry_change;
}

extern uns32
extract_slot_shelf_subslot(uns8 *node_name, struct slot_info *slot_info)
{
   uns8 *param;
   uns8 *str = node_name;
  
   param = str;

   str = strchr(str,'/');
   if(!str)
   {
      return NCSCC_RC_FAILURE;
   }
   *str++ = '\0';
   slot_info->shelf = atoi(param);

   param = str;
   str = strchr(str,'/');
   if(!str)
   {
      return NCSCC_RC_FAILURE;
   }  
   *str++ = '\0';
   slot_info->slot = atoi(param);

   slot_info->subSlot = atoi(str);

   return NCSCC_RC_SUCCESS;
}
extern uns32
avm_prepare_entity_path (uns8* str, const struct slot_info sInfo)
{
   uns32 no_entries = 0;
   uns32 i = 0;
   uns32 len = 0;
   SaHpiEntityPathT  entity_path;

   memset(entity_path.Entry, 0, sizeof(SaHpiEntityPathT));

#ifdef HPI_A
   entity_path.Entry[2].EntityType  = SAHPI_ENT_ROOT; /* Same for all entities */
   entity_path.Entry[2].EntityInstance = 0;
   no_entries++;
   entity_path.Entry[1].EntityType = SAHPI_ENT_SYSTEM_CHASSIS;
   entity_path.Entry[1].EntityInstance = sInfo.shelf; /* This is our chassi id. We get it in openhpi.conf file or so.*/
   no_entries++;
   entity_path.Entry[0].EntityType = SAHPI_ENT_SYSTEM_BOARD; /* enum defined in SaHpi.h */
   entity_path.Entry[0].EntityInstance = sInfo.slot;                          /*SCXB slot id */
   no_entries++;
#else
  if (sInfo.subSlot)
   {
   entity_path.Entry[3].EntityType  = SAHPI_ENT_ROOT; /* Same for all entities */
   entity_path.Entry[3].EntityLocation = 0;
   no_entries++;
   entity_path.Entry[2].EntityType = SAHPI_ENT_ADVANCEDTCA_CHASSIS;
   entity_path.Entry[2].EntityLocation = sInfo.shelf; /* This is our chassi id. We get it in openhpi.conf file or so.*/
   no_entries++;
   entity_path.Entry[1].EntityType = SAHPI_ENT_PHYSICAL_SLOT; /* enum defined in SaHpi.h */
   entity_path.Entry[1].EntityLocation = sInfo.slot;                          /*SCXB slot id */
   no_entries++;
   entity_path.Entry[0].EntityType = AMC_SUB_SLOT_TYPE; /* enum defined in SaHpi.h */
   entity_path.Entry[0].EntityLocation = sInfo.subSlot;                          /*SCXB slot id */
   no_entries++;
   }
  else /* invalid */
  {
   entity_path.Entry[2].EntityType  = SAHPI_ENT_ROOT; /* Same for all entities */
   entity_path.Entry[2].EntityLocation = 0;
   no_entries++;
   entity_path.Entry[1].EntityType = SAHPI_ENT_ADVANCEDTCA_CHASSIS;
   entity_path.Entry[1].EntityLocation = sInfo.shelf; /* This is our chassi id. We get it in openhpi.conf file or so.*/
   no_entries++;
   entity_path.Entry[0].EntityType = SAHPI_ENT_PHYSICAL_SLOT; /* enum defined in SaHpi.h */
   entity_path.Entry[0].EntityLocation = sInfo.slot;                          /*SCXB slot id */
   no_entries++;
   }   
#endif
   
   str [0] = '\0';

   sprintf (str, "%s", "{");
   len = m_NCS_STRLEN (str);


   for (i = 0; i < no_entries; i++)
   {
       switch (entity_path.Entry[i].EntityType)
       {
#ifdef HPI_A
       case SAHPI_ENT_SYSTEM_CHASSIS:

           sprintf (& (str[len]), "%s%d%s", "{23,", sInfo.shelf, "},");
           len = m_NCS_STRLEN (str); 
           break;

       case SAHPI_ENT_SYSTEM_BOARD:
 
           sprintf (&str [len] , "%s%d%s", "{7,", sInfo.slot,"},");
           len = m_NCS_STRLEN (str);
           break;
#else
       case SAHPI_ENT_ADVANCEDTCA_CHASSIS:

           sprintf (& (str[len]), "%s%d%s", "{65539,", sInfo.shelf, "},");
           len = m_NCS_STRLEN (str); 
           break;
       case AMC_SUB_SLOT_TYPE:

           sprintf (& (str[len]), "%s%d%s", "{151,",sInfo.subSlot, "},");
           len = m_NCS_STRLEN (str); 
           break;
       
       case SAHPI_ENT_PHYSICAL_SLOT :
           sprintf (&str [len] , "%s%d%s", "{65557,", sInfo.slot,"},");
           len = m_NCS_STRLEN (str); 
           break;
#endif

       case SAHPI_ENT_ROOT:
           sprintf (&str [len],"%s%d%s","{65535,",0,"}");
           len = m_NCS_STRLEN (str); 
           break;

       default:
           return NCSCC_RC_FAILURE;
           break;
       }
       if (entity_path.Entry[i].EntityType == SAHPI_ENT_ROOT)
       {
           /* m_NCS_STRCAT (str, "}"); */
           sprintf (&str [len] ,"%s", "}");
           break;
       }
   }
   return NCSCC_RC_SUCCESS;

}


extern uns32
avm_convert_nodeid_to_entpath(NCSMIB_PARAM_VAL param_val, AVM_ENT_PATH_STR_T* ep)
{
   uns32 rc = NCSCC_RC_FAILURE;
   AVM_DHCP_CONF_NAME_TYPE node_name;
   struct slot_info sInfo;
   memset(node_name.name, '\0', AVM_NAME_STR_LENGTH);
   m_AVM_SET_NAME(node_name, param_val);
   
   if ((rc = extract_slot_shelf_subslot(node_name.name,&sInfo)) == NCSCC_RC_FAILURE)   
   {
       return NCSCC_RC_FAILURE;
   }  
   
   avm_prepare_entity_path (ep->name, sInfo);
        
   ep->length = m_NCS_STRLEN (ep->name); 
   return NCSCC_RC_SUCCESS;
}

extern uns32
avm_compute_ipmb_address (AVM_ENT_INFO_T *ent_info)
{
   char chassis_type[NCS_MAX_CHASSIS_TYPE_LEN+1];
   uns8 str[255];
   uns32 len=0;
   uns32 physical_slot_id = 0;

   str [0] = '\0';

   if ( ent_info->ep_str.name[4] == '1')
   {
       if(ent_info->ep_str.name[5] == '}')
           physical_slot_id = ent_info->ep_str.name[4] -'0';
       else if(ent_info->ep_str.name[5] >= '0' && ent_info->ep_str.name[5] <= '4')
       {
           physical_slot_id = (ent_info->ep_str.name[4] - '0') * 10 + (ent_info->ep_str.name[5] - '0');
       }
       else
       {
           sprintf(str,"AVM-SSU: Payload blade %s : Unknown physical slot number ",ent_info->ep_str.name);
           m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR); 
           return NCSCC_RC_FAILURE;
       }
   }
   else if((ent_info->ep_str.name[4] >= '2' && ent_info->ep_str.name[4] <= '9') && (ent_info->ep_str.name[5] == '}'))
   {
       physical_slot_id =  ent_info->ep_str.name[4] -'0';     
   }  
   else
   {
       sprintf(str,"AVM-SSU: Payload blade %s : Unknown physical slot number ",ent_info->ep_str.name);
       m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR); 
       return NCSCC_RC_FAILURE;
   }
 
   memset(chassis_type, '\0', NCS_MAX_CHASSIS_TYPE_LEN+1);
   m_NCS_GET_CHASSIS_TYPE(NCS_MAX_CHASSIS_TYPE_LEN,chassis_type);

   len =  m_NCS_STRLEN(chassis_type);   

   if (!m_NCS_MEMCMP(chassis_type, AVM_CHASSIS_TYPE_1405, len))
   {
       if (physical_slot_id < 7)
          ent_info->dhcp_serv_conf.ipmb_addr = 130 + 4 * (7 - physical_slot_id);
       else if (physical_slot_id > 8)
          ent_info->dhcp_serv_conf.ipmb_addr = 132 + 4 * (physical_slot_id - 8);
       else
       {
          sprintf(str,"AVM-SSU: Blade %s : is not a payload blade",ent_info->ep_str.name);
          m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR); 
          return NCSCC_RC_FAILURE;
       }
   }
   else if(!m_NCS_MEMCMP(chassis_type, AVM_CHASSIS_TYPE_1406, len))
   {
       if (physical_slot_id > 1 && physical_slot_id <= 7)
          ent_info->dhcp_serv_conf.ipmb_addr = 130 + 4 * (physical_slot_id - 1);
       else if (physical_slot_id < 14)
          ent_info->dhcp_serv_conf.ipmb_addr = 136 + 4 * (physical_slot_id - 8); 
       else if (physical_slot_id == 14)
          ent_info->dhcp_serv_conf.ipmb_addr = 132;
       else
       {
          sprintf(str,"AVM-SSU: Blade %s : is not a payload blade",ent_info->ep_str.name);
          m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR); 
          return NCSCC_RC_FAILURE;
       }
   }
   else
   {
       sprintf(str,"AVM-SSU: Payload blade %s : Unknown chassis type ",ent_info->ep_str.name);
       m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR); 
       return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}

extern uns32
avm_upgrade_ipmc_trigger(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info)
{
   NCSMIB_ARG         ssu_dummy_mib_obj;
   AVM_EVT_T          fsm_evt;
   uns32              rc = NCSCC_RC_FAILURE;
   uns8 str[AVM_LOG_STR_MAX_LEN];
   uns32              flag_upgrade_sanity =  NCSCC_RC_SUCCESS;

   str[0] = '\0';

   /* Perform sanity tests before triggering upgrade */
   if (((ent_info->current_state != AVM_ENT_INACTIVE) && (ent_info->current_state != AVM_ENT_ACTIVE)) && 
        (ent_info->current_state != AVM_ENT_RESET_REQ))
   {
      /* Entity's state is not good for triggering upgrade */
      flag_upgrade_sanity = NCSCC_RC_FAILURE;
      sprintf(str,"AVM-SSU: Payload blade %s :Target Payload State Not Good. IPMC upgrade failed",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR);
   }
   else if (FALSE == avm_is_the_helper_payload_present(avm_cb,ent_info->dhcp_serv_conf.ipmc_helper_ent_path))
   {
      /* Helper Payload not present */
      flag_upgrade_sanity = NCSCC_RC_FAILURE;
      sprintf(str,"AVM-SSU: Payload blade %s : Helper blade not present. IPMC upgrade failed",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR);
   } 
   /* Sanity test failed */
   if (flag_upgrade_sanity == NCSCC_RC_FAILURE)
   {
      /* send the upgrade failure trap */
      avm_cb->upgrade_module = IPMC_PLD_BLD;
      avm_cb->upgrade_error_type = GEN_ERROR;
      avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);
      sprintf(str,"AVM-SSU: Payload blade %s : IPMC upgrade sanity check failed",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   if ((ent_info->adm_lock == AVM_ADM_LOCK) && (ent_info->current_state == AVM_ENT_INACTIVE))
   {
      /* Entity is already in required state */
      sprintf(str,"AVM-SSU: Payload blade %s : Blade Already In Lock State",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);
      avm_upgrade_ipmc(avm_cb,ent_info);
   }
   else
   {
      /* Set the flag. Wait till the target blade goes to INACTIVE state. */
      sprintf(str,"AVM-SSU: Payload blade %s : Lock the blade",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);
      ent_info->dhcp_serv_conf.ipmc_upgd_state = IPMC_BLD_LOCKED;
      m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
      fsm_evt.evt.mib_req  = &ssu_dummy_mib_obj;
      /* Lock the target payload blade, before upgrading the IPMC */
      fsm_evt.fsm_evt_type = AVM_ADM_LOCK + AVM_EVT_ADM_OP_BASE -1;
      rc = avm_fsm_handler(avm_cb, ent_info, &fsm_evt);
      if(rc != NCSCC_RC_SUCCESS)
      {
         m_MMGR_FREE_MIB_OCT(fsm_evt.evt.mib_req->rsp.add_info); 
      }      
      m_AVM_SEND_CKPT_UPDT_ASYNC_ADD(avm_cb, ent_info, AVM_CKPT_ENT_ADM_OP);
   }
   return rc;
}
 
extern uns32
avm_upgrade_ipmc(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info)
{
   uns32              rc = NCSCC_RC_FAILURE;
   uns8               str[AVM_LOG_STR_MAX_LEN];

   str[0] = '\0';


   /*************************************************************************************************
      Check the uType. If it is INTEG, check whether it is getting upgraded or rolling back.
      If the uType is IPMC, it is upgrade case.
      This function shouldn't be called for SW-BIOS case.
   *************************************************************************************************/
   /* send the request to FUND */

   if(INTEG == ent_info->dhcp_serv_conf.upgrade_type)
   {
      if (ent_info->dhcp_serv_conf.pld_bld_ipmc_status && ent_info->dhcp_serv_conf.pld_rtm_ipmc_status)  
      { 
         /* IPMC of both payload blade and payload RTM should be rolled back */
         /* reset the pld_bld_ipmc_status and pld_rtm_ipmc_status*/
         sprintf(str,"AVM-SSU: Payload blade %s : INTEG : IPMC for PAYLOAD AND RTM requires ROLLBACK",ent_info->ep_str.name);
         m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);

         ent_info->dhcp_serv_conf.pld_bld_ipmc_status = 0;
         ent_info->dhcp_serv_conf.pld_rtm_ipmc_status = 0;
         ent_info->dhcp_serv_conf.ipmc_upgd_state = IPMC_ROLLBACK_IN_PRG;
         m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
         m_AVM_UPGD_IPMC_PLD_TMR_START(avm_cb,ent_info);
         return avm_gen_fund_mib_set(avm_cb, ent_info,AVM_FUND_PLD_BLD_AND_RTM_IPMC_DNGD_CMD,
                                             ent_info->dhcp_serv_conf.default_label->file_name.name);
      }
      else if(ent_info->dhcp_serv_conf.pld_bld_ipmc_status)
      {
         /* it is in rollback process. IPMC of payload blade should be rolled back */
         /* reset the pld_bld_ipmc_status */
         sprintf(str,"AVM-SSU: Payload blade %s : INTEG : IPMC for PAYLOAD requires ROLLBACK",ent_info->ep_str.name);
         m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);

         ent_info->dhcp_serv_conf.pld_bld_ipmc_status = 0;
         ent_info->dhcp_serv_conf.ipmc_upgd_state = IPMC_PLD_BLD_ROLLBACK_IN_PRG;
         m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
         m_AVM_UPGD_IPMC_PLD_MOD_TMR_START(avm_cb,ent_info);
         return avm_gen_fund_mib_set(avm_cb, ent_info,AVM_FUND_PLD_BLD_IPMC_DNGD_CMD,
                                             ent_info->dhcp_serv_conf.default_label->file_name.name);
      }
      else if(ent_info->dhcp_serv_conf.pld_rtm_ipmc_status)
      {      
         /* it is in rollback process. IPMC of payload RTM should be rolled back */
         /* reset the pld_rtm_ipmc_status */
         sprintf(str,"AVM-SSU: Payload blade %s : INTEG : IPMC for PAYLOAD RTM requires ROLLBACK",ent_info->ep_str.name);
         m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);

         ent_info->dhcp_serv_conf.pld_rtm_ipmc_status = 0;
         ent_info->dhcp_serv_conf.ipmc_upgd_state = IPMC_PLD_RTM_ROLLBACK_IN_PRG;
         m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
         m_AVM_UPGD_IPMC_PLD_MOD_TMR_START(avm_cb,ent_info);
         return avm_gen_fund_mib_set(avm_cb, ent_info,AVM_FUND_PLD_RTM_IPMC_DNGD_CMD,
                                             ent_info->dhcp_serv_conf.default_label->file_name.name);
      }
      else
      {
         sprintf(str,"AVM-SSU: Payload blade %s : INTEG : IPMC Upgrade In Prg",ent_info->ep_str.name);
         m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);
 
         ent_info->dhcp_serv_conf.ipmc_upgd_state = IPMC_UPGD_IN_PRG;
         m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
         /* uType is INTEG and upgrade triggered */ 
         m_AVM_UPGD_IPMC_PLD_TMR_START(avm_cb,ent_info);
         rc =  avm_gen_fund_mib_set(avm_cb, ent_info,AVM_FUND_PLD_BLD_AND_RTM_IPMC_UPGD_CMD,
                                    ent_info->dhcp_serv_conf.curr_act_label->other_label->file_name.name);
         if (rc != NCSCC_RC_SUCCESS)
         {

            /* Upgrade Failure Case            */
            /* We would rollback the labels     */

            sprintf(str,"AVM-SSU: Payload blade %s : INTEG : IPMC Upgrade FUND Mib Set Failed ",ent_info->ep_str.name);
            m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);

            AVM_ENT_DHCP_CONF *dhcp_conf = &ent_info->dhcp_serv_conf;
            /* change the state to upgrade failure */
            ent_info->dhcp_serv_conf.curr_act_label->other_label->status = SSU_UPGD_FAILED;

            /* vivek_push */
            /* Push the Label1 state into PSSV */
            m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label1.status, 
                                    ncsAvmEntDHCPConfLabel1Status_ID, ent_info);
            /* Push the Label2 state into PSSV */
            m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label2.status, 
                                    ncsAvmEntDHCPConfLabel2Status_ID, ent_info);


            /* rollback the preferred label */
            if (dhcp_conf->def_label_num == AVM_DEFAULT_LABEL_1)
                dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_2;
            else
                dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_1;
            dhcp_conf->default_label = dhcp_conf->default_label->other_label;
            /* Set the preferred label also to other label */
            dhcp_conf->pref_label.length = dhcp_conf->default_label->name.length;
            memcpy(dhcp_conf->pref_label.name, dhcp_conf->default_label->name.name,
                         dhcp_conf->pref_label.length);

            /* Push preferred label into PSSv */
            /* vivek_push */
            m_AVM_SSU_PSSV_PUSH_STR(avm_cb, ent_info->dhcp_serv_conf.pref_label.name, 
                                    ncsAvmEntDHCPConfPrefLabel_ID, 
                                    ent_info, ent_info->dhcp_serv_conf.pref_label.length );

            dhcp_conf->default_chg = FALSE;
            /* vivek_ckpt */
            m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);

            ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
            m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);   
         }
         return rc;
      }
   }
   else
   {
      sprintf(str,"AVM-SSU: Payload blade %s : PAYLOAD IPMC Upgrade In PRG",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str, NCSFL_SEV_DEBUG);

      ent_info->dhcp_serv_conf.ipmc_upgd_state = IPMC_UPGD_IN_PRG;
      m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
      m_AVM_UPGD_IPMC_PLD_TMR_START(avm_cb,ent_info);
      rc = avm_gen_fund_mib_set(avm_cb, ent_info,AVM_FUND_PLD_BLD_AND_RTM_IPMC_UPGD_CMD,
                                          ent_info->dhcp_serv_conf.curr_act_label->other_label->file_name.name);
      if (rc != NCSCC_RC_SUCCESS)
      {
        sprintf(str,"AVM-SSU: Payload blade %s : IPMC Upgrade FUND Mib Set Failed ",ent_info->ep_str.name);
        m_AVM_LOG_DEBUG(str, NCSFL_SEV_ERROR);

         ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
         m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
      }
      return rc;
   } 
}

extern NCS_BOOL 
avm_is_the_helper_payload_present(AVM_CB_T *avm_cb, AVM_ENT_PATH_STR_T helper_ent_path)
{
   AVM_ENT_INFO_T  *helper_ent_info;

   helper_ent_info = avm_find_ent_str_info(avm_cb, &helper_ent_path,TRUE);

   if(AVM_ENT_INFO_NULL == helper_ent_info)
   {
       return FALSE;
   }

   if(AVM_ENT_ACTIVE != helper_ent_info->current_state)
   {
       return FALSE;
   }

   return TRUE;
}

extern NCS_BOOL
avm_validate_state_for_dhcp_conf_change(AVM_ENT_INFO_T *ent_info, AVM_PER_LABEL_CONF *self_label, AVM_PER_LABEL_CONF *other_label)
{
   uns8 str[AVM_LOG_STR_MAX_LEN];

   str[0] = '\0';

   if (self_label == ent_info->dhcp_serv_conf.curr_act_label)
   {
      sprintf(str,"AVM-SSU: Payload blade %s : DHCP Configuration can't be changed for the current active label",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR);
      return FALSE;
   }  
   else if (other_label->status == SSU_COMMIT_PENDING)
   {
      sprintf(str,"AVM-SSU: Payload blade %s : DHCP Configuration can't be changed, when the other label is in COMMIT PENDING state",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR);
      return FALSE;
   }
   else if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
   {
      sprintf(str,"AVM-SSU: Payload blade %s : Upgrade is in progress, DHCP Configuration can't be changed",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR);
      return FALSE;
   } 
   else if (ent_info->dhcp_serv_conf.ipmc_upgd_state) 
   {
      sprintf(str,"AVM-SSU: Payload blade %s : IPMC Upgrade is in progress, DHCP Configuration can't be changed",ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_ERROR);
      return FALSE;
   }
   else 
      return TRUE;
}

void
avm_role_change_check_pld_upgd_prg(AVM_CB_T *avm_cb)
{
   AVM_ENT_INFO_T      *ent_info;
   SaHpiEntityPathT   entity_path;
   uns8 logbuf[AVM_LOG_STR_MAX_LEN];
   uns8  *entity_path_str   = NULL; 
   uns32 rc = NCSCC_RC_SUCCESS; 
   uns32 chassis_id;
   NCSMIB_ARG         ssu_dummy_mib_obj;

   logbuf[0] = '\0';

   /* for each node, if upgrade is in progress, start the SSU timer */
   memset(entity_path.Entry, 0, sizeof(SaHpiEntityPathT));
   for(ent_info = avm_find_next_ent_info(avm_cb, &entity_path);
       ent_info != AVM_ENT_INFO_NULL; ent_info = avm_find_next_ent_info(avm_cb, &entity_path))
   {
      memcpy(entity_path.Entry, ent_info->entity_path.Entry, sizeof(SaHpiEntityPathT));
      if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      {
         sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : Upgrade Timer Start ", ent_info->ep_str.name);
         m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

         m_AVM_SSU_UPGR_TMR_START(avm_cb, ent_info);
      }
      else if(ent_info->dhcp_serv_conf.ipmc_upgd_state)
      { 
         /* If ipmc upgrade was in progress, before role change */
         switch(ent_info->dhcp_serv_conf.ipmc_upgd_state)
         {
            case IPMC_UPGD_TRIGGERED:
            {
               sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : IPMC State=IPMC_UPGD_TRIGGERED", ent_info->ep_str.name);
               m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

               /* Trigger the IPMC Upgrade */
               rc = avm_upgrade_ipmc_trigger(avm_cb,ent_info);
               if (rc != NCSCC_RC_SUCCESS)
               {
                  if(IPMC == ent_info->dhcp_serv_conf.upgrade_type)
                  {
                     sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : IPMC IPMC_UPGD_TRIGGERED Failed", ent_info->ep_str.name);
                     m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

                     ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
                     m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
                  }
                  else if (INTEG == ent_info->dhcp_serv_conf.upgrade_type)
                  {
                     sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : INTEG IPMC_UPGD_TRIGGERED Failed", ent_info->ep_str.name);
                     m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

                     AVM_ENT_DHCP_CONF *dhcp_conf = &ent_info->dhcp_serv_conf;
                     ent_info->dhcp_serv_conf.curr_act_label->other_label->status = SSU_UPGD_FAILED;
                     m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label1.status,
                                             ncsAvmEntDHCPConfLabel1Status_ID, ent_info);
                     m_AVM_SSU_PSSV_PUSH_INT(avm_cb, ent_info->dhcp_serv_conf.label2.status,
                                             ncsAvmEntDHCPConfLabel2Status_ID, ent_info);

                     if (dhcp_conf->def_label_num == AVM_DEFAULT_LABEL_1)
                         dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_2;
                     else
                         dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_1;

                     dhcp_conf->default_label = dhcp_conf->default_label->other_label;
                     dhcp_conf->pref_label.length = dhcp_conf->default_label->name.length;
                     memcpy(dhcp_conf->pref_label.name, dhcp_conf->default_label->name.name,
                                  dhcp_conf->pref_label.length);

                     m_AVM_SSU_PSSV_PUSH_STR(avm_cb, ent_info->dhcp_serv_conf.pref_label.name,
                                             ncsAvmEntDHCPConfPrefLabel_ID, ent_info,
                                             ent_info->dhcp_serv_conf.pref_label.length );

                     dhcp_conf->default_chg = FALSE;
                     m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);

                     ent_info->dhcp_serv_conf.ipmc_upgd_state = 0;
                     m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
                  } 
               }
            }
            break;

            case IPMC_BLD_LOCKED:
            {
               AVM_EVT_T          fsm_evt;
               uns32              rc;

               sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : IPMC State=IPMC_BLD_LOCKED", ent_info->ep_str.name);
               m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

               if ((ent_info->adm_lock == AVM_ADM_LOCK) && (ent_info->current_state == AVM_ENT_INACTIVE))
               {
                  /* Entity is already in required state */
                  sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : Blade Already Locked", ent_info->ep_str.name);
                  m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

                  rc = avm_upgrade_ipmc(avm_cb,ent_info);
               }
               else
               {
                  /* Lock the blade again - just safe side. Wait till the target blade goes to INACTIVE state. */
                  sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : Blade Lock Invoked", ent_info->ep_str.name);
                  m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);
                  fsm_evt.evt.mib_req  = &ssu_dummy_mib_obj;  
                  fsm_evt.fsm_evt_type = AVM_ADM_LOCK + AVM_EVT_ADM_OP_BASE -1;
                  rc = avm_fsm_handler(avm_cb, ent_info, &fsm_evt);
                  if(rc != NCSCC_RC_SUCCESS)
                  {
                     m_MMGR_FREE_MIB_OCT(fsm_evt.evt.mib_req->rsp.add_info);
                  }
                  m_AVM_SEND_CKPT_UPDT_ASYNC_ADD(avm_cb, ent_info, AVM_CKPT_ENT_ADM_OP);
               }
            }
            break;

            case IPMC_UPGD_IN_PRG:
            {
               /*************************************************************************
                 In this state, there are 3 cases possible:
                   case 1: Fund request might have not gone
                   case 2: Fund request might have gone & response might have not come 
                   case 3: response might have come but not processed
                 It it difficult to distinguish these cases. So, send the request to 
                 FUND again. But the FUND can process one response at a time. If it is
                 still processing the request, to avoid the confusion, wait for the 
                 FUND to get freed. 
               *************************************************************************/

               sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : IPMC State=IPMC_UPGD_IN_PRG", ent_info->ep_str.name);
               m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

               m_AVM_ROLE_CHG_WAIT_TMR_START(avm_cb, ent_info, AVM_UPGD_IPMC_PLD_TMR_INTVL);
            }
            break;
             
            case IPMC_ROLLBACK_IN_PRG:
            {
               /* Same as IPMC_UPGD_IN_PRG case, instead of upgrade, roleback is in progress */ 
               sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : IPMC State=IPMC_ROLLBACK_IN_PRG", ent_info->ep_str.name);
               m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

               m_AVM_ROLE_CHG_WAIT_TMR_START(avm_cb, ent_info, AVM_UPGD_IPMC_PLD_TMR_INTVL);
            }
            break;
  
            case IPMC_PLD_BLD_ROLLBACK_IN_PRG:
            {
               /* Same as IPMC_UPGD_IN_PRG case, instead of upgrade, roleback of payload blade is in progress */
               sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : IPMC State=IPMC_PLD_BLD_ROLLBACK_IN_PRG", ent_info->ep_str.name);
               m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

               m_AVM_ROLE_CHG_WAIT_TMR_START(avm_cb, ent_info, AVM_UPGD_IPMC_PLD_MOD_TMR_INTVL); 
            }
            break;

            case IPMC_PLD_RTM_ROLLBACK_IN_PRG:
            {
               /* Same as IPMC_UPGD_IN_PRG case, instead of upgrade, roleback of payload RTM is in progress */
               sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : IPMC State=IPMC_PLD_RTM_ROLLBACK_IN_PRG", ent_info->ep_str.name);
               m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

               m_AVM_ROLE_CHG_WAIT_TMR_START(avm_cb, ent_info, AVM_UPGD_IPMC_PLD_MOD_TMR_INTVL); 
            }
            break;

            case IPMC_UPGD_DONE:
            {
               AVM_EVT_T          fsm_evt;
               uns32              rc;

               sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : IPMC State=IPMC_UPGD_DONE", ent_info->ep_str.name);
               m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

               /* unlock the target payload blade, after upgrading the IPMC */
               /* is it already unlocked? should we check for it? TBD-JPL */
               fsm_evt.fsm_evt_type = AVM_ADM_UNLOCK + AVM_EVT_ADM_OP_BASE -1;
               fsm_evt.evt.mib_req  = &ssu_dummy_mib_obj;
               rc = avm_fsm_handler(avm_cb, ent_info, &fsm_evt);
               if(rc != NCSCC_RC_SUCCESS)
               {
                  m_MMGR_FREE_MIB_OCT(fsm_evt.evt.mib_req->rsp.add_info);
               }
               m_AVM_SEND_CKPT_UPDT_ASYNC_ADD(avm_cb, ent_info, AVM_CKPT_ENT_ADM_OP);
            }
            break;
         }
         
      }

      if(ent_info->dhcp_serv_conf.bios_upgd_state)
      {
         rc = avm_convert_entity_path_to_string(ent_info->entity_path, &entity_path_str);

         if(NCSCC_RC_SUCCESS != rc)
         {
            m_AVM_LOG_GEN("Cant convert the entity path to string:", ent_info->entity_path.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_ERROR);
            return;
         }

         rc = avm_find_chassis_id(&ent_info->entity_path, &chassis_id);

         switch(ent_info->dhcp_serv_conf.bios_upgd_state)
         {
            case BIOS_TMR_STARTED: 
            {
               sysf_sprintf(logbuf, "StandbyToActive: Payload blade %s : BIOS State=BIOS_TMR_STARTED", ent_info->ep_str.name);
               m_AVM_LOG_DEBUG(logbuf, NCSFL_SEV_DEBUG);

               m_AVM_SSU_BIOS_UPGRADE_TMR_START(avm_cb, ent_info);   
            }
            break;

            /* A timer would be started to let openhpi be initilaized at new active */
            case BIOS_TMR_EXPIRED:
            case BIOS_EXP_BANK_0:
            case BIOS_EXP_BANK_1:
            {
               sysf_sprintf(logbuf, "StandbyToActive:: Payload blade %s : BIOS State = :%d",
                                        ent_info->ep_str.name,ent_info->dhcp_serv_conf.bios_upgd_state);
               m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_DEBUG);

                m_AVM_SSU_BIOS_FAILOVER_TMR_START(avm_cb, ent_info);
            }
            break;

            default:
            {
               sysf_sprintf(logbuf, "StandbyToActive:: Payload blade %s : Invalid bios_upgd_state:%d",
                                        ent_info->ep_str.name,ent_info->dhcp_serv_conf.bios_upgd_state);
               m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_CRITICAL);
            }
            break;
         }

       
      } 
   }
}
