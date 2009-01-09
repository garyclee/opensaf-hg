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
 */


#include "pdrbd.h"
#include "pdrbd_dl_api.h"



/* "Local" function declarations */
static uns32 pseudoCreate(NCS_LIB_REQ_INFO *reqInfo);
static uns32 pseudoDestroy(NCS_LIB_REQ_INFO *reqInfo);
static uns32 parseProxiedConfFile(void);
static uns32 parseProxiedInfo(uns8 *,uns32);
uns8 *getField(uns8 **,uns8);
static uns32 pseudoInitialise(void);
static void pseudoProcMain(void);
static void pseudoProcessPipeMsgs(void);

/* Pseudo control block global variable */
PSEUDO_CB pseudoCB;


/****************************************************************************
 * Name          : pseudoLibReq
 *
 * Description   : This is the NCS SE API which is used to init/destroy the
 *                 Pseudo module
 *
 * Arguments     : reqInfo - this is the pointer to the input information
 *                           which the system BOM gives
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
uns32 pseudoLibReq(NCS_LIB_REQ_INFO *reqInfo)
{
   uns32 ret=NCSCC_RC_SUCCESS;

   switch(reqInfo->i_op)
   {
   case NCS_LIB_REQ_CREATE:

      if((ret = pseudoCreate(reqInfo)) == NCSCC_RC_FAILURE)
      {
         m_NCS_CONS_PRINTF("ERROR: Pseudo DRBD component creation failed!!");
      }

      else
      {
         m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_CREATE_SUCCESS, NCSFL_SEV_NOTICE, 0);
      }

      break;

   case NCS_LIB_REQ_DESTROY:

      ret = pseudoDestroy(reqInfo);
      break;

   default:
      break;
   };

   return(ret);
}


/****************************************************************************
 * Name          : pseudoCreate
 *
 * Description   : This function creates the Pseudo thread
 *
 * Arguments     : reqInfo - this is the pointer to the input information
 *                           which the system BOM gives
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
static uns32 pseudoCreate(NCS_LIB_REQ_INFO *reqInfo)
{
   uns32 ret=NCSCC_RC_SUCCESS;
   int32 i,pipeFd;

   /* Register with logging service */
   if(pdrbd_log_bind() == NCSCC_RC_FAILURE)
   {
      m_NCS_CONS_PRINTF("ERROR: Pseudo DRBD FLS registration failed!!");
      ret = NCSCC_RC_FAILURE;
      goto pcRet;
   }

   m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_FLS_REGN_SUCCESS, NCSFL_SEV_NOTICE, 0);

   /* Parse the proxied component information file */
   if(parseProxiedConfFile() == NCSCC_RC_FAILURE)
   {
      ret = NCSCC_RC_FAILURE;
      m_LOG_PDRBD_MISC(PDRBD_MISC_PROXIED_CONF_FILE_PARSE_FAILED, NCSFL_SEV_ERROR, NULL);
      goto pcRet;
   }

   if((pseudoCB.cb_hdl = ncshm_create_hdl(NCS_HM_POOL_ID_NCS, NCS_SERVICE_ID_PDRBD, (NCSCONTEXT) &pseudoCB)) == 0)
   {
      ret = NCSCC_RC_FAILURE;
      goto pcRet;
   }

   /* Initialise certain fields in the global structure to a known state */
   pseudoCB.compsReady = FALSE;
   pseudoCB.hcTimeOutCnt = 0;

   for(i=0; i<PDRBD_MAX_PROXIED+1; i++)
   {
      pseudoCB.proxied_info[i].clean_done = FALSE;
      pseudoCB.proxied_info[i].cleaning_metadata = FALSE;
      pseudoCB.proxied_info[i].reconnect_done = FALSE;
      pseudoCB.proxied_info[i].stdalone_cnt = 0;
      pseudoCB.proxied_info[i].haState = PSEUDO_DRBD_DEFAULT_ROLE;
      pseudoCB.proxied_info[i].rsrc_num = i;
      pseudoCB.proxied_info[i].cb_ptr = &pseudoCB;
   }

   /* Get MDS Adest handle */
   if(pdrbd_get_ada_hdl() == NCSCC_RC_FAILURE)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_MDS_GET_HDL_FAIL, NCSFL_SEV_ERROR, NULL);
      ret = NCSCC_RC_FAILURE;
      goto pcRet;
   }

   /* Register with MDS service */
   if(pdrbd_mds_install_and_subscribe() == NCSCC_RC_FAILURE)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_MDS_REG_FAIL, NCSFL_SEV_ERROR, NULL);
      ret = NCSCC_RC_FAILURE;
      goto pcRet;
   }

   /* Remove the FIFO (pipe) if it's existing */
   m_NCS_POSIX_UNLINK(PSEUDO_DRBD_FIFO);

   /* Create the FIFO (pipe) */
   if(m_NCS_POSIX_MKFIFO(PSEUDO_DRBD_FIFO, 0600) < 0)
   {
      m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_PIPE_CREATE_FAILURE, NCSFL_SEV_ERROR, 0);
      ret = NCSCC_RC_FAILURE;
      goto pcRet;
   }

   m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_PIPE_CREATE_SUCCESS, NCSFL_SEV_INFO, 0);

   if((pipeFd = m_NCS_POSIX_OPEN(PSEUDO_DRBD_FIFO, O_RDWR | O_NONBLOCK)) < 0)
   {
      m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_PIPE_OPEN_FAILURE, NCSFL_SEV_ERROR, 0);
      ret = NCSCC_RC_FAILURE;
      goto pcRet;
   }

   pseudoCB.pipeFd = pipeFd;

   m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_PIPE_OPEN_SUCCESS, NCSFL_SEV_INFO, 0);

   /* create a mailbox */
   if(m_NCS_IPC_CREATE(&pseudoCB.mailBox) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_MBOX_CREATE_FAILURE, NCSFL_SEV_ERROR, 0);
      ret = NCSCC_RC_FAILURE;
      goto pcRet;
   }

   m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_MBOX_CREATE_SUCCESS, NCSFL_SEV_INFO, 0);

   if(m_NCS_IPC_ATTACH(&pseudoCB.mailBox) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_MBOX_ATTACH_FAILURE, NCSFL_SEV_ERROR, 0);
      ret = NCSCC_RC_FAILURE;
      goto pcRet;
   }

   m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_MBOX_ATTACH_SUCCESS, NCSFL_SEV_INFO, 0);

   /* Create the Pseudo task */
   if((ret = m_NCS_TASK_CREATE((NCS_OS_CB) pseudoProcMain, (void *) 0, "tProxyDRBD", PSEUDO_TASK_PRIORITY,
                                PSEUDO_TASK_STACK_SIZE, &pseudoCB.taskHandle)) == NCSCC_RC_FAILURE)
   {
      m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_THREAD_CREATE_FAILURE, NCSFL_SEV_ERROR, 0);
      goto pcRet;
   }

   m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_THREAD_CREATE_SUCCESS, NCSFL_SEV_INFO, 0);

   /* Start the Pseudo task */
   if((ret = m_NCS_TASK_START(pseudoCB.taskHandle)) == NCSCC_RC_FAILURE)
   {
      m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_THREAD_START_FAILURE, NCSFL_SEV_ERROR, 0);
      m_NCS_TASK_RELEASE(pseudoCB.taskHandle);
      goto pcRet;
   }

   m_LOG_PDRBD_HEADLINE(PDRBD_HDLN_PDRBD_THREAD_START_SUCCESS, NCSFL_SEV_INFO, 0);

pcRet:
   return(ret);
}


/****************************************************************************
 * Name          : pseudoDestroy
 *
 * Description   : This function destroys the Pseudo
 *
 * Arguments     : reqInfo - this is the pointer to the input information
 *                           which the system BOM gives
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
static uns32 pseudoDestroy(NCS_LIB_REQ_INFO *reqInfo)
{
    uns32 ret=NCSCC_RC_SUCCESS;

    return(ret);
}


/****************************************************************************
 * Name          : parseProxiedConfFile
 *
 * Description   : This function processes the proxied components' conf
 *                 file and sends the contents for parsing
 *
 * Arguments     : None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
static uns32 parseProxiedConfFile()
{
   uns8 *ch,*ch1,buff[256],proxiedFilePath[256],proxiedFilePathName[256];
   NCS_OS_FILE proxiedConfFile;
   uns32 compNo=0,ret=NCSCC_RC_SUCCESS;

   /* Get/set the conf file path before opening */
   m_NCS_STRCPY(proxiedFilePath, PDRBD_PROXIED_CONF_FILE_PATH);
   sysf_sprintf(proxiedFilePathName, "%s/"PDRBD_PROXIED_CONF_FILE_NAME, proxiedFilePath);
   proxiedConfFile.info.open.i_file_name = proxiedFilePathName;
   proxiedConfFile.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;

   /* Open the file */
   if(m_NCS_OS_FILE(&proxiedConfFile, NCS_OS_FILE_OPEN) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_PROXIED_COMP_FILE_ABSENT, NCSFL_SEV_ERROR, NULL);
      ret = NCSCC_RC_FAILURE;
      goto ppcfRet;
   }

   /* Start reading the conf file contents */
   while(m_NCS_OS_FGETS(buff, sizeof(buff), (FILE *) proxiedConfFile.info.open.o_file_handle))
   {
      /* Skip comments and tabs in the beginning */
      ch = buff;

      while(*ch == ' ' || *ch == '\t')
         ch++;

      if(*ch == '#' || *ch == '\n')
         continue;

      /* If there's any # in the string, then truncate the string from there */
      if((ch1 = strchr(ch, '#')) != NULL)
      {
         *ch1++ = '\n';
         *ch1 = '\0';
      }

      /* Now parse each line in the conf file */
      if(parseProxiedInfo(ch, compNo) == NCSCC_RC_SUCCESS)
      {
         compNo++;
         pseudoCB.noOfProxied = compNo;

         if(pseudoCB.noOfProxied > PDRBD_MAX_PROXIED)
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_MAX_PROXIED_COMP_EXCEED, NCSFL_SEV_WARNING, NULL);
            pseudoCB.noOfProxied = PDRBD_MAX_PROXIED;
            break;
         }
      }

      else
      {
         m_LOG_PDRBD_MISC(PDRBD_MISC_UNABLE_TO_PARSE_PROXIED_INFO, NCSFL_SEV_WARNING, NULL);
         return NCSCC_RC_FAILURE;
      }
   }


   if(pseudoCB.noOfProxied == 0)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_NO_VALID_ENTRIES, NCSFL_SEV_EMERGENCY, NULL);
      ret = NCSCC_RC_FAILURE;
   }

ppcfRet:
   return(ret);
}


/****************************************************************************
 * Name          : parseProxiedInfo
 *
 * Description   : This function parses a line in the proxied components'
 *                 configuration file and extracts the relevant information
 *
 * Arguments     : pointer to the string line to be parsed
 *                 proxied component array index
 *
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
static uns32 parseProxiedInfo(uns8 *str, uns32 compNo)
{
   uns8 *ptr,*qtr;
   PROXIED_CONF_FIELDS currField=PROXIED_CONF_COMP_ID;
   uns32 strLen,ret=NCSCC_RC_SUCCESS;
   uns8 proxiedInfo[750];

   ptr = str;

   while(currField != PROXIED_CONF_END)
   {
      switch(currField)
      {
      case PROXIED_CONF_COMP_ID:

         if(ptr[0] == ':')
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_MSNG_COMP_ID_IN_CONF_FILE, NCSFL_SEV_WARNING,
                                 pseudoCB.proxied_info[compNo].compName.value);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         qtr = getField(&ptr, ':');

         strLen = m_NCS_STRLEN(qtr);

         if(strLen >= PDRBD_PROXIED_MAX_COMP_ID_LEN)
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_PROXIED_COMP_LEN_EXCEED_MAX_LEN, NCSFL_SEV_WARNING,NULL);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         else
         {
            m_NCS_STRCPY((uns8 *) pseudoCB.proxied_info[compNo].compId, qtr);
            currField = PROXIED_CONF_SU_ID;
         }

         break;

      case PROXIED_CONF_SU_ID:

         if(ptr[0] == ':')
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_MSNG_SU_ID_IN_CONF_FILE, NCSFL_SEV_WARNING,
                                 pseudoCB.proxied_info[compNo].compName.value);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         qtr = getField(&ptr, ':');

         strLen = m_NCS_STRLEN(qtr);

         if(strLen >= PDRBD_PROXIED_MAX_SU_ID_LEN)
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_PROXIED_SUID_LEN_EXCEED_MAX_LEN, NCSFL_SEV_WARNING,NULL);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         else
         {
            m_NCS_STRCPY((uns8 *) pseudoCB.proxied_info[compNo].suId, qtr);
            currField = PROXIED_CONF_RES_NAME;
         }

         break;

      case PROXIED_CONF_RES_NAME:

         if(ptr[0] == ':' || ptr[0] == '\n')
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_MSNG_RSRC_NAME_IN_CONF_FILE, NCSFL_SEV_WARNING,
                                 pseudoCB.proxied_info[compNo].compName.value);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         qtr = getField(&ptr, ':');

         strLen = m_NCS_STRLEN(qtr);

         if(strLen >= PDRBD_PROXIED_MAX_RES_NAME_LEN)
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_PROXIED_RSC_LEN_EXCEED_MAX_LEN, NCSFL_SEV_WARNING,NULL);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         else
         {
            m_NCS_STRCPY(pseudoCB.proxied_info[compNo].resName, qtr);
            currField = PROXIED_CONF_DEV_NAME;
         }

         break;

      case PROXIED_CONF_DEV_NAME:

         if(ptr[0] == ':' || ptr[0] == '\n')
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_MSNG_DEVICE_NAME_IN_CONF_FILE, NCSFL_SEV_WARNING,
                                 pseudoCB.proxied_info[compNo].compName.value);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         qtr = getField(&ptr, ':');

         if(strLen >= PDRBD_PROXIED_MAX_DEV_NAME_LEN)
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_PROXIED_DEV_LEN_EXCEED_MAX_LEN, NCSFL_SEV_WARNING,NULL);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         else
         {
            m_NCS_STRCPY(pseudoCB.proxied_info[compNo].devName, qtr);
            currField = PROXIED_CONF_MOUNT_PNT;
         }

         break;

      case PROXIED_CONF_MOUNT_PNT:

         if(ptr[0] == ':' || ptr[0] == '\n')
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_MSNG_MNT_PNT_IN_CONF_FILE, NCSFL_SEV_WARNING,
                                 pseudoCB.proxied_info[compNo].compName.value);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         qtr = getField(&ptr, ':');

         if(strLen >= PDRBD_PROXIED_MAX_MNT_PNT_NAME_LEN)
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_PROXIED_MNT_PNT_LEN_EXCEED_MAX_LEN, NCSFL_SEV_WARNING,NULL);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         else
         {
            m_NCS_STRCPY(pseudoCB.proxied_info[compNo].mountPnt, qtr);
            currField = PROXIED_CONF_DATA_DISK;
         }

         break;

      case PROXIED_CONF_DATA_DISK:

         if(ptr[0] == ':' || ptr[0] == '\n')
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_MSNG_DATA_DSC_NAME_IN_CONF_FILE, NCSFL_SEV_WARNING,
                                 pseudoCB.proxied_info[compNo].compName.value);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         qtr = getField(&ptr, ':');

         if(strLen >= PDRBD_PROXIED_MAX_DATA_DISK_NAME_LEN)
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_PROXIED_DATA_DSC_LEN_EXCEED_MAX_LEN, NCSFL_SEV_WARNING,NULL);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         else
         {
            m_NCS_STRCPY(pseudoCB.proxied_info[compNo].dataDisk, qtr);
            currField = PROXIED_CONF_META_DISK;
         }

         break;

      case PROXIED_CONF_META_DISK:

         if(ptr[0] == ':' || ptr[0] == '\n')
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_MSNG_METADATA_DSC_NAME_IN_CONF_FILE, NCSFL_SEV_WARNING,
                                 pseudoCB.proxied_info[compNo].compName.value);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         qtr = getField(&ptr, ':');

         if(strLen >= PDRBD_PROXIED_MAX_META_DISK_NAME_LEN)
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_PROXIED_META_DATA_DSC_LEN_EXCEED_MAX_LEN, NCSFL_SEV_WARNING,NULL);
            ret = NCSCC_RC_FAILURE;
            goto parseRet;
         }

         else
         {
            m_NCS_STRCPY(pseudoCB.proxied_info[compNo].metaDisk, qtr);
            currField = PROXIED_CONF_END;
         }

         break;

      case PROXIED_CONF_END:
         break;
      };
   }

   sprintf(proxiedInfo,"\nComp %d - Comp ID: %s, SU ID: %s, Res name: %s, Dev name: %s, Mnt pnt: %s, Data disk: %s, \
               Meta disk: %s\n", compNo, pseudoCB.proxied_info[compNo].compId, pseudoCB.proxied_info[compNo].suId,
                                     pseudoCB.proxied_info[compNo].resName, pseudoCB.proxied_info[compNo].devName,
                                     pseudoCB.proxied_info[compNo].mountPnt, pseudoCB.proxied_info[compNo].dataDisk,
                                     pseudoCB.proxied_info[compNo].metaDisk);

   m_LOG_PDRBD_MISC(PDRBD_MISC_PROXIED_INFO_FROM_CONF_FILE, NCSFL_SEV_NOTICE,proxiedInfo);

parseRet:
   return(ret);
}


/****************************************************************************
 * Name          : getField                                                 *
 *                                                                          *
 * Description   : Parse and return string seperated by given token         *
 *                                                                          *
 * Arguments     : input string to be parsed                                *
 *                 tokenising character, in this case ':'                   *
 *                                                                          *
 * Return Values : String seperated by "token" or NULL if nothing           *
 *                                                                          *
 * Notes         : None.                                                    *
 ****************************************************************************/
uns8 *getField(uns8 **str, uns8 token)
{
   uns8 *p, *q;

   if((str == NULL) || (*str == 0) || (**str == '\n') || (**str == '\0'))
      return(NULL);

   p = q = *str;

   if(**str == ':')
   {
      *p++ = 0;
      *str = p;
      return(NULL);
   }

   while((*p != token) && (*p != '\n') && *p)
      p++;

   if((*p == token) || (*p == '\n'))
   {
      *p++ = 0;
      *str = p;
   }

   return q;
}


/****************************************************************************
 * Name          : pseudoInitialise
 *
 * Description   : This function initialises and registers AMF, starts
 *                 health check
 *
 * Arguments     : None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
uns32 pseudoInitialise()
{
   uns32 i,sts,ret=NCSCC_RC_SUCCESS;
   SaAmfHealthcheckKeyT amfHthChkKey;
   int8 *hthChkKey;
   SaAisErrorT amfError;
   uns8 buff[250];

   /* Register the proxy (main) component's callbacks with AMF */
   if((ret = pseudoAmfInitialise()) == NCSCC_RC_FAILURE)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_INITIALIZE_FAILURE, NCSFL_SEV_ERROR, NULL);
      goto piRet;
   }

   /* Register the Pseudo proxy (main) component with AMF */
   if((ret = pseudoAmfRegister()) == NCSCC_RC_FAILURE)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_REG_FAILED, NCSFL_SEV_ERROR, NULL);

      /* Unregister the callbacks from AMF */
      pseudoAmfUninitialise();
      goto piRet;
   }

   /* Form the full component name of each of the proxied (sub) components */
   for(i=0; i<pseudoCB.noOfProxied; i++)
   {
      m_NCS_MEMSET(buff, 0, sizeof(buff));
      sysf_sprintf(buff, "safComp=%s,safSu=%s,%s", pseudoCB.proxied_info[i].compId, pseudoCB.proxied_info[i].suId,
                     pseudoCB.nodeId);
      m_NCS_STRCPY((uns8 *) pseudoCB.proxied_info[i].compName.value, (uns8 *) buff);
      pseudoCB.proxied_info[i].compName.length = m_NCS_STRLEN(buff);
   }

   /* Register the proxied (sub) component's callbacks with AMF */
   if((ret = pdrbdProxiedAmfInitialise()) == NCSCC_RC_FAILURE)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_INITIALIZE_FAILURE, NCSFL_SEV_ERROR, NULL);
      goto piRet;
   }

   /* Register the proxied components with AMF */
   for(i=0; i<pseudoCB.noOfProxied; i++)
   {
      if((sts = pdrbdProxiedAmfRegister(i)) == NCSCC_RC_FAILURE)
      {
         m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_REG_FAILED, NCSFL_SEV_WARNING, pseudoCB.proxied_info[i].compName.value);

         /* The first (zeroth) component is of NCS; if it fails then no point in proceeding */
         if(i == 0)
         {
            ret = sts;
            goto piRet;
         }
      }
   }

   /* Get and set the health check key */
   m_NCS_MEMSET(&amfHthChkKey, 0, sizeof(amfHthChkKey));
   hthChkKey = m_NCS_OS_PROCESS_GET_ENV_VAR("PSEUDO_DRBD_ENV_HEALTH_CHECK_KEY");

   if(hthChkKey == NULL)
   {
      m_NCS_STRCPY(amfHthChkKey.key, "A5A5");   /* Key also defined in Pseudo init script */
   }

   else
   {
      m_NCS_STRCPY(amfHthChkKey.key, hthChkKey);
   }

   amfHthChkKey.keyLen = strlen(amfHthChkKey.key);

   /* Start the health check */
   if((amfError = saAmfHealthcheckStart(pseudoCB.amfHandle, &pseudoCB.compName, &amfHthChkKey,
                                          SA_AMF_HEALTHCHECK_AMF_INVOKED, SA_AMF_NODE_FAILOVER)) != SA_AIS_OK)
   {
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_HCHK_START_FAILURE, NCSFL_SEV_WARNING, 0);
      ret = NCSCC_RC_FAILURE;
      goto piRet;
   }

   m_LOG_PDRBD_AMF(PDRBD_SP_AMF_HCHK_START_SUCCESS, NCSFL_SEV_INFO, 0);

piRet:
   return(ret);
}


/****************************************************************************
 * Name          : pseudoProcessPipeMsgs
 *
 * Description   : This function reads from pipe and processes the
 *                 messages coming from script
 *
 * Arguments     : None
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/
static void pseudoProcessPipeMsgs()
{
   uns8 buff[SCRIPT_MSG_SIZE+1],buff1[SCRIPT_MSG1_SIZE+1],buff2[SCRIPT_MSG_SIZE+1];
   int32 i,nBytes,invocIndex,compIndex;
   uns32 value,count=0;
   SaAisErrorT error=SA_AIS_OK;

   while(count < (SCRIPT_MSG_SIZE + 1))
   {
      nBytes = m_NCS_POSIX_READ(pseudoCB.pipeFd, buff + count, (sizeof(buff) - count));

      if (nBytes <= 0)
         break;

      count += nBytes;
   }

   if (count < SCRIPT_MSG_SIZE)
   {
      m_LOG_PDRBD_PIPE(PDRBD_PIPE_MSG_INCOMPLETE, NCSFL_SEV_WARNING, 0);
      return;
   }

   /*******************************************************************************
   * The scripts pdrbdctrl and pdrbdsts send their return values through the named
   * pipe in the form of:
   *
   *                         ------------------------------
   *                         | <compNo><4-byte hex value> |
   *                         ------------------------------
   *
   * for example, "20x00010001"
   *
   * in the above eg, '2'is the component index number immediately followed by a
   * 4-byte value
   *
   * The 4 bytes are encoded/decoded as follows:
   *
   *                              ---------------------
   *                              | B3 | B2 | B1 | B0 |
   *                              ---------------------
   *
   *  B3: reserved, always 00
   *  B2: particular location from where the script was invoked;
   *      for valid values see pdrbd_cb.h (eg. PSEUDO_SET_PRI_LOC)
   *  B1: reserved, always 00
   *  B0: result of the command/operation;
   *      for valid values see pdrbd_cb.h (eg. PSEUDO_PRTN_MNT_ERR)
   *******************************************************************************/

   /* The starting char(s) in the buffer received are the proxied commponent's index */
   for(i=0; i<SCRIPT_MSG1_SIZE; i++)
   {
      buff1[i] = buff[i];
   }

   buff1[i] = '\0';

   compIndex = strtoul(buff1, (char **) NULL, 10);

   /* Strip the rest of the buffer */
   for(i=0; i<SCRIPT_MSG_SIZE; i++)
   {
      buff2[i] = buff[i+1];
   }

   buff2[i] = '\0';

   value = strtoul(buff2, (char **) NULL, 16);

   /*
      Find out which script exec. location in the callbacks this return value
      from script refers to
   */
   switch(value & PSEUDO_SCRIPT_EXEC_LOC_MASK)
   {
   /* this is the return value of script invoked in CSI-set (Primary) */
   case PSEUDO_SET_PRI_LOC:

      /* point to the appropriate invocation saved in the array in CB */
      invocIndex = PDRBD_PRI_ROLE_ASSIGN_CB;

      /* Check the return values */
      switch(value & PSEUDO_SCRIPT_RET_VAL_MASK)
      {
      case PSEUDO_CSI_SET_OK:
         error = SA_AIS_OK;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_SET_PRI_SUCCESS, NCSFL_SEV_NOTICE, compIndex);
         break;

      case PSEUDO_DRBDADM_NOT_FND:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBDADM_NOT_FND, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_DRBD_MOD_LOD_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBD_MOD_LOD_FAILURE, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_PRTN_MNT_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_PRTN_MNT_FAILURE, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_DRBDADM_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBDADM_ERR, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_CSI_SET_ERR:
      default:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_SET_PRI_FAILURE, NCSFL_SEV_CRITICAL, compIndex);
         break;
      };

      break;

   /* this is the return value of script invoked in CSI-set (Secondary) */
   case PSEUDO_SET_SEC_LOC:

      /* point to the appropriate invocation saved in the array */
      invocIndex = PDRBD_SEC_ROLE_ASSIGN_CB;

      /* Check the return values */
      switch(value & PSEUDO_SCRIPT_RET_VAL_MASK)
      {
      case PSEUDO_CSI_SET_OK:

         error = SA_AIS_OK;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_SET_SEC_SUCCESS, NCSFL_SEV_NOTICE, compIndex);

         if(TRUE == pseudoCB.proxied_info[compIndex].cleaning_metadata)
            pseudoCB.proxied_info[compIndex].cleaning_metadata = FALSE;

         break;

      case PSEUDO_DRBDADM_NOT_FND:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBDADM_NOT_FND, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_DRBD_MOD_LOD_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBD_MOD_LOD_FAILURE, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_PRTN_UMNT_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_PRTN_UMNT_FAILURE, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_DRBDADM_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBDADM_ERR, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_CSI_SET_ERR:
      default:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_SET_SEC_FAILURE, NCSFL_SEV_CRITICAL, compIndex);
         break;
      };

      break;

   /* this is the return value of script invoked in CSI-set (Quiesced) */
   case PSEUDO_SET_QUI_LOC:

      /* point to the appropriate invocation saved in the array */
      invocIndex = PDRBD_QUI_ROLE_ASSIGN_CB;

      /* Check the return values */
      switch(value & PSEUDO_SCRIPT_RET_VAL_MASK)
      {
      case PSEUDO_CSI_SET_OK:
         error = SA_AIS_OK;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_SET_QUI_SUCCESS, NCSFL_SEV_NOTICE, compIndex);
         break;

      case PSEUDO_DRBDADM_NOT_FND:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBDADM_NOT_FND, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_DRBD_MOD_LOD_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBD_MOD_LOD_FAILURE, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_PRTN_UMNT_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_PRTN_UMNT_FAILURE, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_DRBDADM_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBDADM_ERR, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_CSI_SET_ERR:
      default:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_SET_QUI_FAILURE, NCSFL_SEV_CRITICAL, compIndex);
         break;
      };

      break;

   /* this is the return value of script invoked in remove callback */
   case PSEUDO_REMOVE_LOC:

      /* point to the appropriate invocation saved in the array */
      invocIndex = PDRBD_REMOVE_CB;

      /* Check the return values */
      switch(value & PSEUDO_SCRIPT_RET_VAL_MASK)
      {
      case PSEUDO_CSI_REM_OK:
         error = SA_AIS_OK;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_REM_SUCCESS, NCSFL_SEV_NOTICE, compIndex);
         break;

      case PSEUDO_DRBDADM_NOT_FND:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBDADM_NOT_FND, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_PRTN_UMNT_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_PRTN_UMNT_FAILURE, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_CSI_REM_ERR:
      default:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_REM_FAILURE, NCSFL_SEV_ERROR, compIndex);
         break;
      };

      break;

   /* this is the return value of script invoked in terminate callback */
   case PSEUDO_TERMINATE_LOC:

      /* point to the appropriate invocation saved in the array */
      invocIndex = PDRBD_TERMINATE_CB;

      /* Check the return values */
      switch(value & PSEUDO_SCRIPT_RET_VAL_MASK)
      {
      case PSEUDO_TERM_OK:
         error = SA_AIS_OK;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_TERM_SUCCESS, NCSFL_SEV_NOTICE, compIndex);
         break;

      case PSEUDO_DRBDADM_NOT_FND:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBDADM_NOT_FND, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_PRTN_UMNT_ERR:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_PRTN_UMNT_FAILURE, NCSFL_SEV_ERROR, compIndex);
         break;

      case PSEUDO_TERM_ERR:
      default:
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_TERM_FAILURE, NCSFL_SEV_ERROR, compIndex);
         break;
      };

      break;

   /* this is the return value of script invoked by Standby when an MDS msg is received */
   case PSEUDO_CLEAN_META_LOC:

      /* Check the return values */
      switch(value & PSEUDO_SCRIPT_RET_VAL_MASK)
      {
      case PSEUDO_CLMT_OK:
         pseudoCB.proxied_info[compIndex].cleaning_metadata = FALSE;
         pdrbd_mds_async_send(&pseudoCB, PDRBD_PEER_ACK_MSG_EVT, compIndex);
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_CLMT_SUCCESS, NCSFL_SEV_NOTICE, compIndex);
         goto plainRet;
         break;

      case PSEUDO_DRBDADM_NOT_FND:
         pseudoCB.proxied_info[compIndex].cleaning_metadata = FALSE;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBDADM_NOT_FND, NCSFL_SEV_ERROR, compIndex);
         goto plainRet;
         break;

      case PSEUDO_DRBD_MOD_LOD_ERR:
         pseudoCB.proxied_info[compIndex].cleaning_metadata = FALSE;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_DRBD_MOD_LOD_FAILURE, NCSFL_SEV_ERROR, compIndex);
         goto plainRet;
         break;

      case PSEUDO_CLMT_ERR:
         pseudoCB.proxied_info[compIndex].cleaning_metadata = FALSE;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_CLMT_FAILURE, NCSFL_SEV_ERROR, compIndex);
         goto plainRet;
         break;

      default:
         pseudoCB.proxied_info[compIndex].cleaning_metadata = FALSE;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_CLMT_FAILURE, NCSFL_SEV_ERROR, compIndex);
         goto plainRet;
         break;
      };

      break;

   /*
      This is the return value of script invoked in health check callback. The actual response to the health-check
      callback is given by the proxy (main) in pdrbd_proxy.c; here in case of any error condition use the
      asynchronous error report function to report error to the AMF (below), otherwise in case of "OK" conditions
      simply return without doing anything, i.e don't report anything to the AMF
    */
   case PSEUDO_HLTH_CHK_LOC:

      /* point to the appropriate invocation saved in the array */
      invocIndex = PDRBD_HEALTH_CHECK_CB;

      /* Check the return values */
      switch(value & PSEUDO_SCRIPT_RET_VAL_MASK)
      {
      case PSEUDO_CS_INVALID:    /* DRBD not running at all */
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_CS_INVALID, NCSFL_SEV_WARNING, compIndex);
         goto prxHcRet;
         break;

      case PSEUDO_CS_OTHER:
         error = SA_AIS_ERR_FAILED_OPERATION;
         goto prxHcRet;
         break;

      case PSEUDO_CS_UNCONFIGURED:
         error = SA_AIS_OK;
         m_LOG_PDRBD_PIPE(PDRBD_PIPE_CS_UNCONFIGURED, NCSFL_SEV_INFO, compIndex);
         goto plainRet;
         break;

      case PSEUDO_CS_STANDALONE:    /* trigger a failover if it's StandAlone */

         if(pseudoCB.proxied_info[compIndex].stdalone_cnt < 2)
         {
            pdrbd_mds_async_send(&pseudoCB, PDRBD_PEER_CLEAN_MSG_EVT, pseudoCB.proxied_info[compIndex].stdalone_cnt);
            pseudoCB.proxied_info[compIndex].stdalone_cnt++;
            error = SA_AIS_OK;
            m_LOG_PDRBD_PIPE(PDRBD_PIPE_CS_STANDALONE, NCSFL_SEV_INFO, compIndex);
            goto plainRet;
         }

         else
         {
            error = SA_AIS_ERR_FAILED_OPERATION;
            m_LOG_PDRBD_PIPE(PDRBD_PIPE_CS_STANDALONE, NCSFL_SEV_WARNING, compIndex);
            goto prxHcRet;
         }

         break;

      case PSEUDO_CS_UNCONNECTED:
      case PSEUDO_CS_WFCONNECTION:
      case PSEUDO_CS_WFREPORTPARAMS:
      case PSEUDO_CS_CONNECTED:
      case PSEUDO_CS_TIMEOUT:
      case PSEUDO_CS_BROKENPIPE:
      case PSEUDO_CS_NETWORKFAILURE:
      case PSEUDO_CS_WFBITMAPS:
      case PSEUDO_CS_WFBITMAPT:
      case PSEUDO_CS_SYNCSOURCE:
      case PSEUDO_CS_SYNCTARGET:
      case PSEUDO_CS_PAUSEDSYNCS:
      case PSEUDO_CS_PAUSEDSYNCT:

         error = SA_AIS_OK;
         m_LOG_PDRBD_PIPE(((value & PSEUDO_SCRIPT_RET_VAL_MASK) - PDRBD_PIPE_CS_UNCONNECTED) + PDRBD_PIPE_CS_UNCONNECTED,
                                NCSFL_SEV_INFO, compIndex);
         goto plainRet;
         break;
      };

      break;

   default:
      error = SA_AIS_ERR_FAILED_OPERATION;
      goto prxHcRet;
      break;
   };

   if(saAmfResponse(pseudoCB.proxiedAmfHandle, pseudoCB.proxied_info[compIndex].script_status_array[invocIndex].invocation,
                     error) != SA_AIS_OK)
   {
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_RESPONSE_FAILURE, NCSFL_SEV_ERROR, value);
      goto plainRet;
   }

   else
   {
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_RESPONSE_SUCCESS, NCSFL_SEV_INFO, value);
      goto plainRet;
   }

prxHcRet:
   if(saAmfComponentErrorReport(pseudoCB.proxiedAmfHandle, &pseudoCB.proxied_info[compIndex].compName, 0,
                                 SA_AMF_COMPONENT_FAILOVER, 0) != SA_AIS_OK)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_ERROR_RPT_FAILED, NCSFL_SEV_ERROR, NULL);
   }

plainRet:
   return;
}


/****************************************************************************
 * Name          : pseudoProcMain
 *
 * Description   : This function is the Pseudo task's entry point
 *
 * Arguments     : None
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/
static void pseudoProcMain()
{
   uns32 ret=NCSCC_RC_SUCCESS;
   PDRBD_EVT *evt;
   NCS_SEL_OBJ mbx_sel_obj,selFd,selfd_high,ncsSelObj,proxiedSelObj,pdrbd_pipe_fd;
   NCS_SEL_OBJ_SET selObjSet,selObjSet_pass;
   SaSelectionObjectT amfSelObj,proxiedAmfSelObj;
   SaAmfHandleT amfHandle,proxiedAmfHandle;
   SaAisErrorT amfError;
   int32 selStat=0;

   /* Initialise and register AMF; start health check */
   if((ret = pseudoInitialise()) == NCSCC_RC_FAILURE)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_INITIALIZE_FAILURE, NCSFL_SEV_ERROR, NULL);
      exit(-3);
   }

   amfHandle = pseudoCB.amfHandle;
   proxiedAmfHandle = pseudoCB.proxiedAmfHandle;

   if((amfError = saAmfSelectionObjectGet(amfHandle, &amfSelObj)) != SA_AIS_OK)
   {
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_GET_SEL_OBJ_FAILURE, NCSFL_SEV_ERROR, 0);
      exit(-3);
   }

   if((amfError = saAmfSelectionObjectGet(proxiedAmfHandle, &proxiedAmfSelObj)) != SA_AIS_OK)
   {
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_GET_SEL_OBJ_FAILURE, NCSFL_SEV_ERROR, 0);
      exit(-3);
   }

   m_LOG_PDRBD_AMF(PDRBD_SP_AMF_GET_SEL_OBJ_SUCCESS, NCSFL_SEV_INFO, 0);

   m_NCS_SEL_OBJ_ZERO(&selObjSet);

   mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(&pseudoCB.mailBox);
   m_NCS_SEL_OBJ_SET(mbx_sel_obj, &selObjSet);
   selfd_high = mbx_sel_obj;


   m_SET_FD_IN_SEL_OBJ(pseudoCB.pipeFd, pdrbd_pipe_fd);
   m_NCS_SEL_OBJ_SET(pdrbd_pipe_fd, &selObjSet);
   selfd_high = m_GET_HIGHER_SEL_OBJ(selfd_high, pdrbd_pipe_fd);

   m_SET_FD_IN_SEL_OBJ((uns32) amfSelObj, ncsSelObj);
   m_NCS_SEL_OBJ_SET(ncsSelObj, &selObjSet);
   selfd_high = m_GET_HIGHER_SEL_OBJ(selfd_high, ncsSelObj);


   m_SET_FD_IN_SEL_OBJ((uns32) proxiedAmfSelObj, proxiedSelObj);
   m_NCS_SEL_OBJ_SET(proxiedSelObj, &selObjSet);
   selfd_high = m_GET_HIGHER_SEL_OBJ(selfd_high, proxiedSelObj);

   selFd = selfd_high;

   selObjSet_pass = selObjSet;

   /* Select on the AMF FD, pipe FD */
   while((selStat = m_NCS_SEL_OBJ_SELECT(selFd, &selObjSet_pass, NULL, NULL, NULL)) != -1)
   {
      /* Check if it's an event from AMF */
      if(m_NCS_SEL_OBJ_ISSET(ncsSelObj, &selObjSet_pass))
      {
         /* Despatch all the AMF pending functions */
         if((amfError = saAmfDispatch(amfHandle, SA_DISPATCH_ALL)) != SA_AIS_OK)
         {
            m_LOG_PDRBD_AMF(PDRBD_SP_AMF_DESPATCH_FAILURE, NCSFL_SEV_ERROR, 0);
         }

         m_LOG_PDRBD_AMF(PDRBD_SP_AMF_DESPATCH_SUCCESS, NCSFL_SEV_INFO, 0);
      }

      /* Check if it's an event from AMF */
      if(m_NCS_SEL_OBJ_ISSET(proxiedSelObj, &selObjSet_pass))
      {
         /* Despatch all the AMF pending functions */
         if((amfError = saAmfDispatch(proxiedAmfHandle, SA_DISPATCH_ALL)) != SA_AIS_OK)
         {
            m_LOG_PDRBD_AMF(PDRBD_SP_AMF_DESPATCH_FAILURE, NCSFL_SEV_ERROR, 0);
         }

         m_LOG_PDRBD_AMF(PDRBD_SP_AMF_DESPATCH_SUCCESS, NCSFL_SEV_INFO, 0);
      }


      /* Check if it's an event from pipe */
      if(m_NCS_SEL_OBJ_ISSET(pdrbd_pipe_fd, &selObjSet_pass))
      {
         pseudoProcessPipeMsgs();
      }

       /* Check if it's an event from script call-back */
      if(m_NCS_SEL_OBJ_ISSET(mbx_sel_obj, &selObjSet_pass))
      {
         while (NULL != (evt = m_PDRBD_RCV_MSG(&pseudoCB.mailBox)))
         {
            if (PDRBD_SCRIPT_CB_EVT == evt->evt_type)
               pdrbd_handle_script_resp(&pseudoCB, evt);
            else if ((PDRBD_PEER_CLEAN_MSG_EVT == evt->evt_type) ||
                     (PDRBD_PEER_ACK_MSG_EVT == evt->evt_type))
               pdrbd_process_peer_msg(&pseudoCB, evt);

            if (NULL != evt)
               m_MMGR_FREE_PDRBD_EVT(evt);
         }
      }

      selObjSet_pass = selObjSet;
      selFd = selfd_high;
   }
}
