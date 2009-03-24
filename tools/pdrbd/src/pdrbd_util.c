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

/*
******************************************************************************
  DESCRIPTION:

  This file includes functions to spawn a script and a callback to receive
  response of the script.

..............................................................................

  FUNCTIONS INCLUDED in this module:



******************************************************************************
*/

#include "pdrbd.h"



static uns32 pdrbd_script_resp(NCS_OS_PROC_EXECUTE_TIMED_CB_INFO *);


/****************************************************************************
  Name          : pdrbd_script_resp

  Description   : This is a callback routine that is invoked when either the
                  script finishes execution (success or failure) or it times
                  out. It sends the corresponding event to the pseudo DRBD thread.

  Arguments     : info - ptr to the cbk info

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         : None.
******************************************************************************/
uns32 pdrbd_script_resp(NCS_OS_PROC_EXECUTE_TIMED_CB_INFO *info)
{
   PSEUDO_CB *cb=&pseudoCB;
   PDRBD_EVT *evt=0;
   uns32 rc=NCSCC_RC_SUCCESS;

   if(!info) goto done;

   /* create the event and send it to the PDRBD thread */
   if(NULL == (evt = m_MMGR_ALLOC_PDRBD_EVT))
   {
      m_LOG_PDRBD_SCRIPT(PDRBD_SCRIPT_EVT_CREATE_FAILURE, NCSFL_SEV_ERROR, 0);
      rc = NCSCC_RC_FAILURE;
      goto done;
   }

   evt->evt_data.info = *info;

   m_PDRBD_SND_MSG(&cb->mailBox, evt, NCS_IPC_PRIORITY_HIGH);

done:
   /* free the event */
   if(NCSCC_RC_SUCCESS != rc && evt)
      m_MMGR_FREE_PDRBD_EVT(evt);

   /* return pDRBD cb */
   if (cb) ncshm_give_hdl(info->i_usr_hdl);

   return rc;
}



/****************************************************************************
  Name          : pdrbd_script_execute

  Description   : This routine executes the specified script.

  Arguments     : cb       - ptr to the pDRBD control block
                  cmd_str  - Command string
                  time_out - Time-out in ms,
                  scr_cntxt - Context of the script
                  rsrc_num - index in to the proxied component array

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

******************************************************************************/
uns32 pdrbd_script_execute(PSEUDO_CB *cb, char *cmd_str, uns32 time_out, uns32 scr_cntxt, uns32 rsrc_num)
{
   NCS_OS_PROC_EXECUTE_TIMED_INFO cmd_info;
   char                     *argv[PDRBD_NUM_ARGS+2];
   char                     scr[PDRBD_SCRIPT_MAX_SIZE];
   char                     tmp_argv[PDRBD_NUM_ARGS+2][PDRBD_MAX_ARG_SIZE];
   uns32                    argc = 0, rc = NCSCC_RC_SUCCESS;

   memset(&cmd_info, 0, sizeof(NCS_OS_PROC_EXECUTE_TIMED_INFO));

   /* tokenize the cmd */
   m_SCRIPT_STR_PARSE(cmd_str, scr, argc, argv, tmp_argv);

   /* populate the cmd-info */
   cmd_info.i_script = argv[0];
   cmd_info.i_argv = argv;
   cmd_info.i_timeout_in_ms = time_out;

   if(rsrc_num == PDRBD_MAX_PROXIED)
      cmd_info.i_usr_hdl = PDRBD_MAX_PROXIED;
   else
      cmd_info.i_usr_hdl = cb->proxied_info[rsrc_num].rsrc_num;

   cmd_info.i_cb = pdrbd_script_resp;
   cmd_info.i_set_env_args = NULL;

   /* finally execute the Script  */
   rc = m_NCS_OS_PROCESS_EXECUTE_TIMED(&cmd_info);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_LOG_PDRBD_SCRIPT(PDRBD_SCRIPT_EXEC_FAILURE, NCSFL_SEV_ERROR, 0);
      m_PDRBD_CLR_SCR_CNTXT(cb, scr_cntxt, rsrc_num);
      return NCSCC_RC_FAILURE;
   }

   m_LOG_PDRBD_SCRIPT(PDRBD_SCRIPT_EXEC_SUCCESS, NCSFL_SEV_INFO, 0);

   cb->proxied_info[rsrc_num].script_status_array[scr_cntxt].status = TRUE;
   cb->proxied_info[rsrc_num].script_status_array[scr_cntxt].execute_hdl = cmd_info.o_exec_hdl;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : pdrbd_handle_script_resp

  Description   : This routine will be envoked from a main DRBD thread
                   and will handle the responses received form the script
                   execution utility.

  Arguments     : cb       - ptr to the pDRBD control block
                  evt      - Event received fromt the thread.

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

******************************************************************************/
uns32 pdrbd_handle_script_resp(PSEUDO_CB *cb, PDRBD_EVT *evt)
{
   uns32  i = 0, rsrc_num;
   NCS_BOOL      scr_cntxt_found = FALSE;

   /* This block is for health-check script invocation */
   if(evt->evt_data.info.i_usr_hdl == PDRBD_MAX_PROXIED)
   {
      switch(evt->evt_data.info.exec_stat.value)
      {
         case NCS_OS_PROC_EXIT_WAIT_TIMEOUT:

            pseudoCB.hcTimeOutCnt++;
            m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_HLT_CHK_SCRIPT_TMD_OUT, NCSFL_SEV_ERROR, NULL);
            system("pkill -9 pdrbdsts");     /* Kill the script here */
            return NCSCC_RC_SUCCESS;         /* Need not return failure, just log it */

         case NCS_OS_PROC_EXIT_NORMAL:

            /* Here we dont need to handle this case. Our pipe processing will handle. Just log info.*/
            m_LOG_PDRBD_SCRIPT(PDRBD_SCRIPT_EXIT_NORMAL, NCSFL_SEV_INFO, evt->evt_data.info.exec_stat.value);
            pseudoCB.hcTimeOutCnt = 0;
            return NCSCC_RC_SUCCESS;

         case NCS_OS_PROC_EXEC_FAIL:
         case NCS_OS_PROC_EXIT_ON_SIGNAL:
         case NCS_OS_PROC_EXIT_WITH_CODE:
         default:
            /* Log Error */
            return NCSCC_RC_FAILURE;
      };
   }

   rsrc_num = evt->evt_data.info.i_usr_hdl;

   /* Find script context. If it is already cleared then return success */
   for (i=0; i<=SCRIPT_SPAWN_PLACE_MAX; i++)
   {
      if(cb->proxied_info[rsrc_num].script_status_array[i].execute_hdl == evt->evt_data.info.i_exec_hdl)
      {
         scr_cntxt_found = TRUE;
         break;
      }
   }

   /* check if we have found the script context */
   if(scr_cntxt_found == TRUE)
   {
      /* This is for script invocation other than health-check */
      switch(evt->evt_data.info.exec_stat.value)
      {
      case NCS_OS_PROC_EXIT_WAIT_TIMEOUT:
         /* Kill the script here? */
      case NCS_OS_PROC_EXEC_FAIL:
      case NCS_OS_PROC_EXIT_ON_SIGNAL:
      case NCS_OS_PROC_EXIT_WITH_CODE:
         /* all these are error conditions. Send error resp to AMF and clear script context*/

         if(saAmfResponse(pseudoCB.proxiedAmfHandle, pseudoCB.proxied_info[rsrc_num].script_status_array[i].invocation,
                           SA_AIS_ERR_FAILED_OPERATION) != SA_AIS_OK)
         {
            m_LOG_PDRBD_AMF(PDRBD_SP_AMF_RESPONSE_FAILURE, NCSFL_SEV_ERROR, 0);
         }

         else
         {
            m_LOG_PDRBD_AMF(PDRBD_SP_AMF_RESPONSE_SUCCESS, NCSFL_SEV_INFO, 0);
         }

         m_PDRBD_CLR_SCR_CNTXT(cb, i, rsrc_num);
         m_LOG_PDRBD_SCRIPT(PDRBD_SCRIPT_EXIT_ERROR, NCSFL_SEV_ERROR, evt->evt_data.info.exec_stat.value);
         break;

      case NCS_OS_PROC_EXIT_NORMAL:
         /* Here we dont need to handle this case. Our pipe processing will handle. Just log info.*/
         m_LOG_PDRBD_SCRIPT(PDRBD_SCRIPT_EXIT_NORMAL, NCSFL_SEV_INFO, evt->evt_data.info.exec_stat.value);
         break;

      default:
         /* Log error. We should never reach here. */
         m_LOG_PDRBD_SCRIPT(PDRBD_SCRIPT_EXIT_ERROR, NCSFL_SEV_ERROR, evt->evt_data.info.exec_stat.value);
         break;
      }
   }
   else
      /* We can log an info here */
      m_LOG_PDRBD_SCRIPT(PDRBD_SCRIPT_CTX_NOT_FOUND, NCSFL_SEV_WARNING, i);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : pdrbd_process_peer_msg

  Description   : This routine will be envoked from a main DRBD thread
                   and will handle the messages received from peer.

  Arguments     : cb       - ptr to the pDRBD control block
                  evt      - Event received fromt the thread.

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

******************************************************************************/
uns32 pdrbd_process_peer_msg(PSEUDO_CB *cb, PDRBD_EVT *evt)
{
   uns32 ret;
   char script[250]={0},buff[250];

   if(PDRBD_PEER_CLEAN_MSG_EVT == evt->evt_type)
   {
      /* Call script to clean */
      if(FALSE == cb->proxied_info[evt->evt_data.number].clean_done)
      {
         cb->proxied_info[evt->evt_data.number].clean_done = TRUE;

         /* Invoke the pdrbdctrl script to clean metadata */
         cb->proxied_info[evt->evt_data.number].cleaning_metadata = TRUE;

         sprintf(script, "%s %s %d %s %s %s %s %s", PSEUDO_CTRL_SCRIPT_NAME, "clean_meta", evt->evt_data.number,
                        cb->proxied_info[evt->evt_data.number].resName, cb->proxied_info[evt->evt_data.number].devName,
                        cb->proxied_info[evt->evt_data.number].mountPnt, cb->proxied_info[evt->evt_data.number].dataDisk,
                        cb->proxied_info[evt->evt_data.number].metaDisk);

         m_LOG_PDRBD_MISC(PDRBD_MISC_INVOKING_SCRIPT_TO_CLEAN_RSRC_METADATA, NCSFL_SEV_NOTICE,
                              cb->proxied_info[evt->evt_data.number].resName );

         ret = pdrbd_script_execute(cb, script, PDRBD_SCRIPT_TIMEOUT_CLEAN_META, PDRBD_CLEAN_META, evt->evt_data.number);

         if(ret == NCSCC_RC_FAILURE)
         {
            m_LOG_PDRBD_MISC(PDRBD_MISC_CLEAN_METADATA_SCRIPT_EXEC_FAILED, NCSFL_SEV_ERROR, NULL);
         }
      }
   }

   else if (PDRBD_PEER_ACK_MSG_EVT == evt->evt_type)
   {
      /* Call script to re-connect */
      if(FALSE == cb->proxied_info[evt->evt_data.number].reconnect_done)
      {
         cb->proxied_info[evt->evt_data.number].reconnect_done = TRUE;

         m_LOG_PDRBD_MISC(PDRBD_MISC_RECONNECTING_RESOURCE, NCSFL_SEV_NOTICE,
                              cb->proxied_info[evt->evt_data.number].resName );

         sprintf(buff, "%s %s", "/sbin/drbdadm connect", cb->proxied_info[evt->evt_data.number].resName);
         system(buff);
      }
   }

   else
   {
/*      m_NCS_CONS_PRINTF("ERROR: received invalid event type");     */
   }

   return NCSCC_RC_SUCCESS;
}
