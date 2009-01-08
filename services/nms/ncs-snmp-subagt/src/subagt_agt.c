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


  .....................................................................

  DESCRIPTION: This file describes the routines for the Agentx master
                agent interface

               This file has the OID Database mgmt routines as well.
  ***************************************************************************/
#include <limits.h>
#include "subagt.h"

#include "net-snmp/net-snmp-config.h"
#include "net-snmp/net-snmp-includes.h"
#include "net-snmp/agent/net-snmp-agent-includes.h"
#include "net-snmp/library/snmp_api.h"

#define MAX_ALLOWED_PING_TIME_DISCREPANCY 29

extern netsnmp_session *main_session;

static void
snmpsubagt_agt_usage(char *subagt);

static void
snmpsubagt_agt_log_setup(char *logfile);

static void
snmpsubagt_netsnmp_agentx_pingInterval(const char *token, char *cptr);

/*****************************************************************************\
 *  Name:          snmpsubagt_netsnmp_lib_initialize                          *
 *                                                                            *
 *  Description:   To initialize the session with the Agentx Agent            *
 *                  To do the above job NetSnmp Supplied APIs are used        *
 *                                                                            *
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block               *
 *                                                                            *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     *
\*****************************************************************************/
uns32
snmpsubagt_netsnmp_lib_initialize(struct ncsSa_cb *cb)
{
    int32   netsnmp_status = SNMP_ERR_NOERROR;
#if (SUBAGT_AGT_MONITOR == 1)
    uns32   status = NCSCC_RC_FAILURE;
#endif

    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_NETSNMP_LIB_INIT);

    /* As of now CB is not being initialized in this routine,
     * may be used in future.
     */
    /* validate the input */
    if (cb == NULL)
    {
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_AMF_CB_NULL);
        return NCSCC_RC_FAILURE;
    }

    /* initialize the logging for the NET-SNMP Lib code */
    snmp_log(LOG_INFO, "NCS SNMP SubAgt 2.0\n");
    snmp_log(LOG_INFO, "NET-SNMP version %s\n", netsnmp_get_version());

    /* make us a agentx client. */
    netsnmp_status =  netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID,
                                            NETSNMP_DS_AGENT_ROLE, 1);
    if (netsnmp_status != SNMP_ERR_NOERROR)
    {
        return NCSCC_RC_FAILURE;
    }

    /* set the type of the application */
    netsnmp_status = netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID,
                                         NETSNMP_DS_LIB_APPTYPE,
                                         "ncsSnmpSubagt");
    if (netsnmp_status != SNMP_ERR_NOERROR)
    {
        return NCSCC_RC_FAILURE;
    }

    m_SNMPSUBAGT_HEADLINE_LOG(NETSNMP_DS_SET_BOOLEAN);

#if (NET_SNMP_5_2_2_SUPPORT == 1)
 /*From Net-Snmp version 5.3.X  onwards , the equaling call
  "agentx_config_init" is invoked in "init_agent" API"*/
    /* initiliaze the function pointers to parse the agentx
     * configuration paramters
     */
    init_agentx_config();
#endif

    /* initialize the agentxPingInterval handler */
    snmpd_register_config_handler("agentxPingInterval",
                                  snmpsubagt_netsnmp_agentx_pingInterval, NULL,
                                   "Agentx Ping Interval (seconds)");

#if (SUBAGT_AGT_MONITOR == 1)
    /* initiliaze the default agent monitoring attributes */
    snmpsubagt_agt_monitor_attribs_init(&cb->agt_mntr_attrib);

    /*
     * register the tags to read the agent monitoring attributes
     * from the configuration file (ncsSnmpSubagt.conf)
     * Please note that, this function should be called before
     * init_snmp() always.  Otherwise these tags will not be
     * recognised by the NET-SNMP Library.
     */
    snmpsubagt_agt_register_pvt_tags();
#endif


    m_SNMPSUBAGT_HEADLINE_LOG(INIT_SNMP_DONE);

    /* initialize the agent library */
#if (NET_SNMP_5_2_2_SUPPORT == 1)
    init_snmp("ncsSnmpSubagt");
    init_agent("ncsSnmpSubagt");
#else
    init_agent("ncsSnmpSubagt");
    init_snmp("ncsSnmpSubagt");
#endif

    /* Following are the bug details in NET-SNMP bug data base.
       http://sourceforge.net/tracker/index.php?func=detail&aid=1047767&group_id=12694&atid=112694
       This is temporary fix till we get the fix in NET-SNMP. This fixes the bugs IR00009056, IR00009136,
       IR00057949 and IR00058137. */

    signal (SIGPIPE, SIG_IGN);

#if (SUBAGT_AGT_MONITOR == 1)
    status = snmpsubagt_agt_monitor_kickoff(cb);
    if (status == NCSCC_RC_FAILURE)
    {
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_AGT_MONITOR_FAILED);
    }
#endif

    m_SNMPSUBAGT_HEADLINE_LOG(INIT_AGENT_DONE);
    return NCSCC_RC_SUCCESS;
}


/******************************************************************************
 *  Name:          snmpsubagt_netsnmp_lib_finalize
 *
 *  Description:   To Finalize the session with the Agentx Agent
 *                  To do the above job NetSnmp Supplied APIs are used
 *
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE:
 *****************************************************************************/
uns32
snmpsubagt_netsnmp_lib_finalize(NCSSA_CB *cb)
{
#if (SUBAGT_AGT_MONITOR == 1)
    uns32   status = NCSCC_RC_FAILURE;
#endif

    /* Here also CB is not used, useful in future */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_NETSNMP_LIB_FINALIZE);

    if (cb == NULL)
    {
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return NCSCC_RC_FAILURE;
    }

#if (SUBAGT_AGT_MONITOR == 1)
    /* stop monitoring the Agent */
    status = snmpsubagt_agt_monitor_stop(cb);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AGT_MONITOR_STOP_FAILED,
                                status,0,0);

        /* continue shutting down*/
    }
#endif

    /* shutdown the library */
    snmp_shutdown("ncsSnmpSubagt"); /* net-snmp library api */

    m_SNMPSUBAGT_HEADLINE_LOG(SNMP_SHUTDOWN_DONE);
    return NCSCC_RC_SUCCESS;
}
#if 0

/******************************************************************************
 *  Name:          snmpsubagt_netsnmp_lib_deinit
 *
 *  Description:   To de-initialize the session with the Agentx Agent
 *                  To do the above job NetSnmp Supplied APIs are used
 *                  De initialization inovolves the termination of the
 *                  session with the Agent.  This does not delete the
 *                  OID tree build at the SubAgent.
 *
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE:  This routine closes the session with the Agent, but it maintains
 *         the list of MIBs to be registered with the Agent.  This routine
 *         is not used any more for two reasons
 *              - STANDBY subagent also opens the session with the Agent
 *              - SubAgent registers the OIDs with the Agent, only when
 *                it is moved to INS State.
 ****************************************************************************/
uns32
snmpsubagt_netsnmp_lib_deinit(NCSSA_CB *cb)
{
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_NETSNMP_LIB_DEINIT);

    if (cb == NULL)
    {
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return NCSCC_RC_FAILURE;
    }
    /* unregister the alarms */
    snmp_alarm_unregister_all();
    m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_ALARM_UNREG_DONE);

    /* close the session */
    snmp_close_sessions();
    m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CLOSE_SESSIONS_DONE);
    return NCSCC_RC_SUCCESS;
}
#endif
/******************************************************************************
 *  Name:          snmpsubagt_send_buffered_traps
 *
 *  Description:   Sends buffered traps to master agent
 *
 *  Arguments:     cb - NCSSA_CB 
 *
 *  Returns:       Nothing
 *****************************************************************************/
void snmpsubagt_send_buffered_traps(NCSSA_CB    *cb)
{
   SNMPSUBAGT_TRAP_LIST  *trap_list = NULL;
   SNMPSUBAGT_TRAP_LIST  *trap_list_node = NULL;
   uns32                 status = NCSCC_RC_FAILURE;
   
   /* log the function entry */
   m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_FLUSH_BUFFERED_TRAPS);

   if (cb == NULL || cb->trap_list == NULL )
   {
       /* log the error */
       m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
       return ;
   }

   trap_list = cb->trap_list;

   while(trap_list)
   {
      trap_list_node = trap_list;

      trap_list = trap_list->next;

      /* validate the input */
      if( trap_list_node->trap_evt == NULL )
      {
         /* log that there is no data in the received event.
         * there is nothing to send to the Agent */
         m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_EVT_DATA_NULL);
      }
      else
      {
        /* Logging the NCS TRAP data - before Flushing out*/
        ncs_logmsg(NCS_SERVICE_ID_SNMPSUBAGT, SNMPSUBAGT_FMTID_STATE, SNMPSUBAGT_FS_ERRORS,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                       NCSFL_TYPE_TILL, SNMPSUBAGT_FLUSH_TRAP_LIST,
                       trap_list_node->trap_evt->i_trap_tbl_id,
                       trap_list_node->trap_evt->i_trap_id);

        status = subagt_send_v2trap(&cb->oidDatabase, trap_list_node->trap_evt);
        if (status != NCSCC_RC_SUCCESS)
        {
          /* log that unable to send the trap */
          m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TRAP_SEND_FAIL,status,
                                 trap_list_node->trap_evt->i_trap_tbl_id,
                                 trap_list_node->trap_evt->i_trap_id);
        }
        free(trap_list_node->trap_evt);
      }
      m_MMGR_SNMPSUBAGT_TRAP_LIST_NODE_FREE(trap_list_node); 
   }

   cb->trap_list = NULL;
   
   return;
}

/******************************************************************************
 *  Name:          snmpsubagt_request_process
 *
 *  Description:   To  Handle Multiple Interfaces of the SubAgent.
 *                   * 1. Interface with AMF
 *                   * 2. Interface with Agentx Agent
 *                   * 3. Interface with EDA
 *
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE:
 *****************************************************************************/
uns32
snmpsubagt_request_process(NCSSA_CB    *cb)
{
    int             numfds;
    fd_set          readfds;
    struct timeval  timeout = { LONG_MAX, 0 }, *tvp = &timeout;
    int             count;
    int             fakeblock = 0;
    SaAisErrorT     saf_status = SA_AIS_OK;
    NCS_SEL_OBJ     mbx_fd;
    uns32           status = NCSCC_RC_FAILURE;
    int             ping_interval = 0;

#if 0
    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_RQST_PROCESS);
#endif

    if (cb == NULL)
    {
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return NCSCC_RC_FAILURE;
    }

    numfds = 0;
    FD_ZERO(&readfds);
    snmp_select_info(&numfds, &readfds, tvp, &fakeblock);
    if (cb->block != 0 && fakeblock != 0)
    {
        /*
         * There are no alarms registered, and the caller asked for blocking,
         * let select() block forever.
         */
        tvp = NULL;
    }
    else if (cb->block != 0 && fakeblock == 0)
    {
        /*
         * The caller asked for blocking, but there is an alarm due sooner than
         * LONG_MAX seconds from now, so use the modified timeout returned by
         * snmp_select_info as the timeout for select().
         */
     }
     else if (cb->block == 0)
     {
        /*
         * The caller does not want us to block at all.
         */
         tvp->tv_sec = 0;
         tvp->tv_usec = 0;
     }
     /* Fix provided for the bug IR00079744 */
     ping_interval = netsnmp_ds_get_int(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL);
     if((ping_interval > 0) && (tvp != NULL) && (tvp->tv_sec  > ping_interval))
     {
         ncs_logmsg(NCS_SERVICE_ID_SNMPSUBAGT, SNMPSUBAGT_FMTID_ERRORS, SNMPSUBAGT_FS_ERRORS,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                       NCSFL_TYPE_TILLL, SNMPSUBAGT_TIME_TO_WAIT_DISCREPANCY, tvp->tv_sec, ping_interval, 0);
         if (tvp->tv_sec  >= (ping_interval + MAX_ALLOWED_PING_TIME_DISCREPANCY))
         {
              if(cb->amfHandle)
              {
                  saAmfComponentErrorReport(cb->amfHandle,
                                            &cb->compName,
                                            0, /* errorDetectionTime; AvNd will provide this value */
                                            SA_AMF_COMPONENT_RESTART/* recovery */, 0/*NTF Id*/);
                  return NCSCC_RC_INVALID_PING_TIMEOUT;
              }
              else
              {
                  m_NCS_CONS_PRINTF("NCS SSA: exiting...\n");
                  exit(1);
              }
         }
     }

#if (BBS_SNMPSUBAGT == 0)
    /* Add the external socket interfaces FDs to the list */
    /*Add the AMF SelectionObject into the list of FDs*/
    if (cb->amfHandle != 0)
    {
        FD_SET((int)cb->amfSelectionObject, &readfds);
#if 0
        /* log the fd */
        m_SNMPSUBAGT_DATA_LOG(SNMPSUBAGT_AMF_SEL_OBJ,
                              (int)cb->amfSelectionObject);
#endif
        if ((cb->amfSelectionObject + 1) > numfds)
        {
            numfds = cb->amfSelectionObject+1;
        }
    }
    else /* if(cb->amfHandle == 0) */ /* IR00061409 */
    {
        /* add the sigusr1 signal handler fd */
        FD_SET(m_GET_FD_FROM_SEL_OBJ(cb->sigusr1hdlr_sel_obj), &readfds);
        if ( (m_GET_FD_FROM_SEL_OBJ(cb->sigusr1hdlr_sel_obj))+1 > numfds)
        {
            numfds = (m_GET_FD_FROM_SEL_OBJ(cb->sigusr1hdlr_sel_obj))+1;
        }
    }
#endif

    /*Add the EDA SelectionObject into the list of FDs*/
    if ((cb->evtHandle != 0) && (cb->evtChannelHandle != 0))
    {
        FD_SET((int)cb->evtSelectionObject, &readfds);
        if ((cb->evtSelectionObject + 1) > numfds)
        {
            numfds = cb->evtSelectionObject+1;
        }
    }

    /* Get selection object for the mailbox */
    mbx_fd = m_NCS_IPC_GET_SEL_OBJ(&cb->subagt_mbx);

    /* Add the subagent mail box fd to the select list */
    FD_SET((m_GET_FD_FROM_SEL_OBJ(mbx_fd)),&readfds);
#if 0
    /* log the ipc fd */
    m_SNMPSUBAGT_DATA_LOG(SNMPSUBAGT_IPC_MBX_SEL_OBJ,
                          m_GET_FD_FROM_SEL_OBJ(mbx_fd));
#endif

    if ( (m_GET_FD_FROM_SEL_OBJ(mbx_fd))+1 > numfds)
    {
        numfds = (m_GET_FD_FROM_SEL_OBJ(mbx_fd))+1;
    }
#if 0
    /* Log the num of FDs */
    m_SNMPSUBAGT_DATA_LOG(SNMPSUBAGT_NUM_FDS, numfds);
#endif

    /* add the SIGHUP signal handler fd */
    FD_SET(m_GET_FD_FROM_SEL_OBJ(cb->sighdlr_sel_obj), &readfds);
    if ( (m_GET_FD_FROM_SEL_OBJ(cb->sighdlr_sel_obj))+1 > numfds)
    {
        /*numfds = (m_GET_FD_FROM_SEL_OBJ(mbx_fd))+1;*/
        numfds = (m_GET_FD_FROM_SEL_OBJ(cb->sighdlr_sel_obj))+1;
    }

    /* add the SIGUSR2 signal handler fd */
    FD_SET(m_GET_FD_FROM_SEL_OBJ(cb->sigusr2hdlr_sel_obj), &readfds);
    if ( (m_GET_FD_FROM_SEL_OBJ(cb->sigusr2hdlr_sel_obj))+1 > numfds)
    {
        numfds = (m_GET_FD_FROM_SEL_OBJ(cb->sigusr2hdlr_sel_obj))+1;
    }


    /*
     * Wait for the request from
     * 1. From AVA
     * 2. From EDA
     * 3. From SPA(Destroy)
     * 4. From SNMP Agent
     * 5. From Caller thread about register/deregister the OID tree
     * 6. SIGHUP Handler
     */
    count = select(numfds, &readfds, 0, 0, tvp);
    if (count > 0)
    {
#if (BBS_SNMPSUBAGT == 0)

           /* IR00061409 */
           if (cb->amfHandle == 0)
           {
               if (FD_ISSET((m_GET_FD_FROM_SEL_OBJ(cb->sigusr1hdlr_sel_obj)), &readfds))
               {
                   subagt_process_sig_usr1_signal(cb);
                   m_NCS_SEL_OBJ_CLR(cb->sigusr1hdlr_sel_obj, &readfds);
                   m_NCS_SEL_OBJ_RMV_IND(cb->sigusr1hdlr_sel_obj, TRUE, TRUE);
                   m_NCS_SEL_OBJ_DESTROY(cb->sigusr1hdlr_sel_obj);
               }
           } /* IR00061409 */
           else /* if (cb->amfHandle != 0) */
           {

               /* Event from AMF */
               if (FD_ISSET((int)cb->amfSelectionObject, &readfds))
               {
                   /* process the event from AMF */
                   saf_status = saAmfDispatch(cb->amfHandle,
                                        cb->subagt_dispatch_flags);
                   if (saf_status != SA_AIS_OK)
                   {
                       /* log the saf error  */
                       m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_DSPATCH_ERR,
                                            saf_status,0,0);
                   }

                   /*clear the bit off for this fd*/
                   FD_CLR((int)cb->amfSelectionObject, &readfds);
               }
           }
#endif
           /* event from EDA */
           if (FD_ISSET((int)cb->evtSelectionObject, &readfds))
           {
               saf_status = saEvtDispatch(cb->evtHandle,
                                          cb->subagt_dispatch_flags);
               if (saf_status != SA_AIS_OK)
               {
                   /* log the saf error  */
                   m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_DSPATCH_ERR,
                                           saf_status, 0, 0);
               }

               /*clear the bit off for this fd*/
               FD_CLR((int)cb->evtSelectionObject, &readfds);
           }
           /* Do snmp_read() before processing mbx fd. Fix for IR 84087 */
           {
               /* Check if data is on snmp fds */
               snmp_read(&readfds);/* by default for all fds */
           }
           /* event from SPA to destroy the SubAgent */
           if (FD_ISSET((m_GET_FD_FROM_SEL_OBJ(mbx_fd)), &readfds))
           {
               status = snmpsubagt_mbx_process(cb);
               if (status != NCSCC_RC_SUCCESS)
               {
                   /* log on to the console */
               }
           }
           if (FD_ISSET((m_GET_FD_FROM_SEL_OBJ(cb->sighdlr_sel_obj)), &readfds))
           {
               /* Process the SIGHUP only if HAState is ACTIVE */
               if (cb->haCstate == SNMPSUBAGT_HA_STATE_ACTIVE)
               {
                  m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_PROCESSING_SIGHUP);
                  /* reload the Configuration file */
                  snmpsubagt_mibs_reload(cb);
               }

               m_NCS_SEL_OBJ_CLR(cb->sighdlr_sel_obj, &readfds);
               m_NCS_SEL_OBJ_RMV_IND(cb->sighdlr_sel_obj, TRUE, TRUE);
           }
           if (FD_ISSET((m_GET_FD_FROM_SEL_OBJ(cb->sigusr2hdlr_sel_obj)), &readfds))
           {
              m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_PROCESSING_SIGUSR2);

              /* reset the debugging */
              snmp_set_do_debugging(!snmp_get_do_debugging());

              m_NCS_SEL_OBJ_CLR(cb->sigusr2hdlr_sel_obj, &readfds);
              m_NCS_SEL_OBJ_RMV_IND(cb->sigusr2hdlr_sel_obj, TRUE, TRUE);
           }
    }
    else /* No packets found */
    {
        switch (count)
        {
            case 0:
                m_SNMPSUBAGT_DATA_LOG(SNMPSUBAGT_SPL_FD_PROCESS, count);
                snmp_timeout();
                break;

            case -1:
                if (errno != EINTR)
                {
                    m_SNMPSUBAGT_DATA_LOG(SNMPSUBAGT_SPL_FD_PROCESS, count);
                    snmp_log_perror("select");
                }
                return NCSCC_RC_FAILURE;

            default:
                   m_SNMPSUBAGT_DATA_LOG(SNMPSUBAGT_SPL_FD_PROCESS, count);
                   snmp_log(LOG_ERR, "select returned %d\n", count);
                return NCSCC_RC_FAILURE;
          }
    }
    /* endif -- count>0 */

    /*
     * Run requested alarms.
     */
    run_alarms();

    /* If the  traps are buffered and snmpd is up, send the traps from the  buffered list */
    if( cb->trap_list && main_session  )
      snmpsubagt_send_buffered_traps(cb);

    return NCSCC_RC_SUCCESS;
} /* end */



/******************************************************************************
 *  Name:          snmpsubagt_agt_startup_params_process
 *
 *  Description:   To  process the startup parameters
 *
 *  Arguments:     argc         - Number of startup arguments
 *                 argv         - Pointer to the arguments
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE:
 *****************************************************************************/
uns32
snmpsubagt_agt_startup_params_process(int32    argc,
                                      uns8     **argv)
{
    uns32       status = NCSCC_RC_SUCCESS;

    /* delimiters for the subagent */
    char        options[128] = "aAc:CdD:fhHI:l:LP:qrsS:UvV-:";
    int32       arg;
    char        logfile[PATH_MAX + 1] = { 0 };
    int         netsnmp_status = SNMP_ERR_NOERROR;

    /* set the default values for all the startup parameters */
    strcpy(logfile, SNMPSUBAGT_LOG_FILE);


    /* '-h' -- print the help message */


    /* '-v' -- version information */

    /* Now process the options */
    while ((arg = getopt(argc, (char **)argv, options)) != EOF)
    {
        switch (arg)
        {
            case '-':
            /* help given as --help */
            if (strcasecmp(optarg, "help") == 0)
            {
                 snmpsubagt_agt_usage(argv[0]);
            }
            /* version info  --version */
            if (strcasecmp(optarg, "version") == 0)
            {
                m_NCS_CONS_PRINTF("NCS SNMP SubAgt 1.0\n");
                m_NCS_CONS_PRINTF("NET-SNMP Version %s\n", netsnmp_get_version());
                exit(1);
            }
            /* I did not get what this long options is all about?? */
#if 0
            handle_long_opt(optarg);
#endif
            break;

            /* '-c' -- config file */
            case 'c':
            if (optarg != NULL)
            {
                netsnmp_status = netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID,
                                                NETSNMP_DS_LIB_OPTIONALCONFIG, optarg);
                if (netsnmp_status != SNMP_ERR_NOERROR)
                {
                    exit(1);
                }
            }
            else
            {
                snmpsubagt_agt_usage(argv[0]);
            }
            break;

            /* '-C' -- do not read the default config specs */
            case 'C':
            netsnmp_status = netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID,
                                   NETSNMP_DS_LIB_DONT_READ_CONFIGS, 1);
            if (netsnmp_status != SNMP_ERR_NOERROR)
            {
                exit(1);
            }
            break;

            /* '-d' -- to dump the packets */
            case 'd':
            snmp_set_dump_packet(1);
            netsnmp_status = netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID,
                               NETSNMP_DS_AGENT_VERBOSE, 1);
            if (netsnmp_status != SNMP_ERR_NOERROR)
            {
                exit(1);
            }
            break;

            /* '-D' -- to dump the debug messages from NET-SNMP code */
            case 'D':
            debug_register_tokens(optarg);
            snmp_set_do_debugging(1);
            break;

            case 'h':
            snmpsubagt_agt_usage(argv[0]);
            break;

            /* '-l' -- log file */
            case 'l':
            if (optarg != NULL)
            {
               if (strlen(optarg) > PATH_MAX)
               {
                   fprintf(stderr,
                           "%s: logfile path too long (limit %d chars)\n",
                           argv[0],
                           PATH_MAX);
                   exit(1);
               }
               strncpy(logfile, optarg, PATH_MAX);
            }
            else
            {
                 snmpsubagt_agt_usage(argv[0]);
            }
            break;

            case 'v':
            m_NCS_CONS_PRINTF("NCS SNMP SubAgt 1.0\n");
            m_NCS_CONS_PRINTF("NET-SNMP Version %s\n", netsnmp_get_version());
            exit(1);
            break;

            default:
            m_NCS_CONS_PRINTF("Illegal input...\n");
            status = NCSCC_RC_FAILURE;
            break;
        }
    }
    /* enable logging */
    snmpsubagt_agt_log_setup(logfile);
    return status;
}

static void
snmpsubagt_agt_usage(char *subagt)
{
    m_NCS_CONS_PRINTF("%s Usage...\n", subagt);
    m_NCS_CONS_PRINTF("NCS SNMP SubAgt 1.0\n");
    m_NCS_CONS_PRINTF("\nNET-SNMP Version %s\n", netsnmp_get_version());
    m_NCS_CONS_PRINTF("  -c FILE\t\tread FILE as a configuration file\n");
    m_NCS_CONS_PRINTF("  -C\t\t\tdo not read the default configuration files\n");
    m_NCS_CONS_PRINTF("  -d\t\t\tdump sent and received SNMP packets\n");
    m_NCS_CONS_PRINTF("  -D\t\t\tturn on debugging output\n");
    m_NCS_CONS_PRINTF("  -h, --help\t\tdisplay this usage message\n");
    m_NCS_CONS_PRINTF("  -l FILE\t\tprint warnings/messages to FILE\n");
    m_NCS_CONS_PRINTF("  -v --version\t\tVersion Information of the subagent\n");
    m_NCS_CONS_PRINTF("\n");
    exit(1);
}

static void
snmpsubagt_agt_log_setup(char *logfile)
{

    /* SNMP Agent's log function setup_log() takes more number of
     * parameters as input.  I do not understand them completely.
     * For the SubAgent it is enough, if we do the following.
     */
    /* snmp_enable_stderrlog(); */ /* Fix for IR 82446 */

    if (logfile != NULL)
    {
        snmp_enable_filelog(logfile, 0/*dont_zero_log*/);
    }

    return;
}

static void
snmpsubagt_netsnmp_agentx_pingInterval(const char *token, char *cptr)
{
    int x = atoi(cptr);

    /* log the function entry */
     m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_PING_INT_CONF);

    DEBUGMSGTL(("agentx/config/PingInterval", "%s\n", cptr));
    netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL, x);
    return;
}


