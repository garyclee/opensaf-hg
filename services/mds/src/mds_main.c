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

#include <configmake.h>

/*****************************************************************************
..............................................................................


..............................................................................

  DESCRIPTION:  MDS layer initialization and destruction entry points

******************************************************************************
*/

#include "gl_defs.h"
#include "t_suite.h"
#include "ncs_lib.h"
#include "ncssysf_tmr.h"
#include "ncs_main_pvt.h"
#include "mds_dl_api.h"
#include "mds_core.h"
#include "patricia.h"
#include "mds_log.h"



/* MDS Control Block */
MDS_MCM_CB *gl_mds_mcm_cb = NULL;

NCS_LOCK   gl_lock;

NCS_LOCK*  mds_lock(void) 
{
    static int lock_inited = FALSE;
    /* Initialize the lock first time mds_lock() is called */
    if (!lock_inited)
    {
        m_NCS_LOCK_INIT(&gl_lock);   
        lock_inited = TRUE;
    }
    return &gl_lock;
}

/* global Log level variable */
uns32 gl_mds_log_level = 2;
uns32 gl_mds_checksum = 0;

uns32 MDS_QUIESCED_TMR_VAL = 200;
uns32 MDS_AWAIT_ACTIVE_TMR_VAL = 18000;
uns32 MDS_SUBSCRIPTION_TMR_VAL = 500;
uns32 MDTM_REASSEMBLE_TMR_VAL  = 500;
uns32 MDTM_CACHED_EVENTS_TMR_VAL = 24000;

/* ******************************************** */
/* ******************************************** */
/* ******************************************** */

/*          mds_lib_req                         */

/* ******************************************** */
/* ******************************************** */
/* ******************************************** */


uns32 mds_lib_req(NCS_LIB_REQ_INFO *req)
{
    char *p_field = NULL;
    uns32 node_id, cluster_id, mds_tipc_ref=0; /* this mds tipc ref is random num part of the TIPC id */
    uns32 status = NCSCC_RC_SUCCESS;
    uns32        destroy_ack_tmout;
    NCS_SEL_OBJ  destroy_ack_obj;

    switch(req->i_op)
    {
        case NCS_LIB_REQ_CREATE:

            /* Take lock : Initialization of lock done in mds_lock(), if in case it is not initialized*/
            m_NCS_LOCK(mds_lock(),NCS_LOCK_WRITE);         

            if (gl_mds_mcm_cb != NULL)
            {
                m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_CREATE : MDS is already initialized");
                m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                return NCSCC_RC_FAILURE;
            }

            /* Initialize mcm database */
            mds_mcm_init();

            /* Extract parameters from req and fill adest and pcon_id */

            /* Get Node_id */
            p_field = (char *)ncs_util_search_argv_list(req->info.create.argc, req->info.create.argv, 
                                                    "NODE_ID=");
            if (p_field != NULL)
            {
                if (sscanf(p_field + strlen("NODE_ID="), "%d", &node_id) != 1)
                {
                    m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_CREATE : Problem in NODE_ID argument\n");
                    mds_mcm_destroy();
                    m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
                }
            }

#if 0            
            /* Get Pcon_id */
            p_field = NULL;
            p_field = (char *)ncs_util_search_argv_list(req->info.create.argc, req->info.create.argv,"PCON_ID=");
            if (p_field != NULL)
            {
                if (sscanf(p_field + strlen("PCON_ID="), "%d", &pcon_id) != 1)
                {
                    m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_CREATE : Problem in PCON_ID argument\n");
                    mds_mcm_destroy();
                    m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
                }
            }
#endif

            /* Get Cluster_id */
            p_field = NULL;
            p_field = (char *)ncs_util_search_argv_list(req->info.create.argc, req->info.create.argv, 
                                                    "CLUSTER_ID=");
            if (p_field != NULL)
            {
                if (sscanf(p_field + strlen("CLUSTER_ID="), "%d", &cluster_id) != 1)
                {
                    m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_CREATE : Problem in CLUSTER_ID argument\n");
                    mds_mcm_destroy();
                    m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
                }
            }

            /* vishal : to use cluster id in mds prefix? */


            /* Get gl_mds_log_level */

            /* vishal : setting MDS_LOG_LEVEL from environment variable if given */
            if ( m_NCS_OS_PROCESS_GET_ENV_VAR("MDS_LOG_LEVEL") )
            {
                gl_mds_log_level = atoi(m_NCS_OS_PROCESS_GET_ENV_VAR("MDS_LOG_LEVEL"));
            }
            else
            {
            
                p_field = NULL;
                p_field = (char *)ncs_util_search_argv_list(req->info.create.argc, req->info.create.argv, 
                                                    "MDS_LOG_LEVEL=");
                if (p_field != NULL)
                {
                    if (sscanf(p_field + strlen("MDS_LOG_LEVEL="), "%d", &gl_mds_log_level) != 1)
                    {
                        m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_CREATE : Problem in MDS_LOG_LEVEL argument\n");
                        mds_mcm_destroy();
                        m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                        return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
                    }
                }
             }
             /* gl_mds_log_level consistency check */
             if (gl_mds_log_level > 5 || gl_mds_log_level < 1)
             {
                 /* gl_mds_log_level specified is outside range so reset to Default = 3 */
                 gl_mds_log_level = 2; 
             }

            /* Get gl_mds_checksum */

            /* vishal : setting MDS_CHECKSUM from environment variable if given */
            if ( m_NCS_OS_PROCESS_GET_ENV_VAR("MDS_CHECKSUM") )
            {
                gl_mds_checksum = atoi(m_NCS_OS_PROCESS_GET_ENV_VAR("MDS_CHECKSUM"));
            }
            else
            {
            
                p_field = NULL;
                p_field = (char *)ncs_util_search_argv_list(req->info.create.argc, req->info.create.argv, 
                                                    "MDS_CHECKSUM=");
                if (p_field != NULL)
                {
                    if (sscanf(p_field + strlen("MDS_CHECKSUM="), "%d", &gl_mds_checksum) != 1)
                    {
                        m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_CREATE : Problem in MDS_CHECKSUM argument\n");
                        mds_mcm_destroy();
                        m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                        return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
                    }
                }
             }
             /* gl_mds_checksum consistency check */
             if (gl_mds_checksum != 1)
             {
                 /* gl_mds_checksum specified is not 1 so reset to 0 */
                 gl_mds_checksum = 0;
             }            

            /*****************************/
            /* Timer value Configuration */
            /*****************************/

            /* Get Subscription timer value */
            p_field = NULL;
            p_field = (char *)ncs_util_search_argv_list(req->info.create.argc, req->info.create.argv, 
                                                    "SUBSCRIPTION_TMR_VAL=");
            if (p_field != NULL)
            {
                if (sscanf(p_field + strlen("SUBSCRIPTION_TMR_VAL="), "%d", &MDS_SUBSCRIPTION_TMR_VAL) != 1)
                {
                    m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_CREATE : Problem in SUBSCRIPTION_TMR_VAL argument\n");
                    mds_mcm_destroy();
                    m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
                }
            }

            /* Get Await Active timer value */
            p_field = NULL;
            p_field = (char *)ncs_util_search_argv_list(req->info.create.argc, req->info.create.argv, 
                                                    "AWAIT_ACTIVE_TMR_VAL=");
            if (p_field != NULL)
            {
                if (sscanf(p_field + strlen("AWAIT_ACTIVE_TMR_VAL="), "%d", &MDS_AWAIT_ACTIVE_TMR_VAL) != 1)
                {
                    m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_CREATE : Problem in AWAIT_ACTIVE_TMR_VAL argument\n");
                    mds_mcm_destroy();
                    m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
                }
            }

            /* Get Quiesced timer value */
            p_field = NULL;
            p_field = (char *)ncs_util_search_argv_list(req->info.create.argc, req->info.create.argv, 
                                                    "QUIESCED_TMR_VAL=");
            if (p_field != NULL)
            {
                if (sscanf(p_field + strlen("QUIESCED_TMR_VAL="), "%d", &MDS_QUIESCED_TMR_VAL) != 1)
                {
                    m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_CREATE : Problem in QUIESCED_TMR_VAL argument\n");
                    mds_mcm_destroy();
                    m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
                }
            }

            /* Get Reassembly timer value */
            p_field = NULL;
            p_field = (char *)ncs_util_search_argv_list(req->info.create.argc, req->info.create.argv, 
                                                    "REASSEMBLE_TMR_VAL=");
            if (p_field != NULL)
            {
                if (sscanf(p_field + strlen("REASSEMBLE_TMR_VAL="), "%d", &MDTM_REASSEMBLE_TMR_VAL) != 1)
                {
                    m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_CREATE : Problem in REASSEMBLE_TMR_VAL argument\n");
                    mds_mcm_destroy();
                    m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
                }
            }


            /* Invoke MDTM-INIT.  */
            status = mds_mdtm_init(node_id, &mds_tipc_ref); 
            if (status != NCSCC_RC_SUCCESS)
            {
                /* vishal : todo cleanup */
                return NCSCC_RC_FAILURE;
            }
            gl_mds_mcm_cb->adest = m_MDS_GET_ADEST_FROM_NODE_ID_AND_PROCESS_ID(node_id, mds_tipc_ref);
            
            /* Initialize logging */
            {
               char buff[50],pref[50];
               sprintf(buff,OSAF_LOCALSTATEDIR "mdslog/ncsmds_n%08x",node_id);
               sprintf(pref," <0x%08x,%u> ",node_id,mds_tipc_ref);
               m_NCS_OS_STRCAT(buff,".log");
               mds_log_init(buff, pref);
            }

            m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
            
            break;

        case NCS_LIB_REQ_DESTROY:
            /* STEP 1: Invoke MDTM-Destroy. */
            /* mds_mdtm_destroy (); */
            /* STEP 2: Destroy MCM-CB;      */
            /* ncs_patricia_tree_destroy(&gl_mds_mcm_cb->vdest_list); */
            /* m_MMGR_FREE_MDS_CB(gl_mds_mcm_cb); */
            /* Destroy lock */
            /* m_NCS_LOCK_INIT(mds_lock); */
            
            m_NCS_SEL_OBJ_CREATE(&destroy_ack_obj);

            /* Post a dummy message to MDS thread to guarantee that it
               wakes up (and thereby sees the destroy_ind) */
            if (mds_destroy_event(destroy_ack_obj) == NCSCC_RC_FAILURE) 
            { 
               m_NCS_SEL_OBJ_DESTROY(destroy_ack_obj);
               return NCSCC_RC_FAILURE;
            }

            /* Wait for indication from MDS thread that it is ok to kill it */
            destroy_ack_tmout = 7000; /* 70seconds */
            m_NCS_SEL_OBJ_POLL_SINGLE_OBJ(destroy_ack_obj, &destroy_ack_tmout);
            m_MDS_LOG_DBG("LIB_DESTROY:Destroy ack from MDS thread in %d ms", destroy_ack_tmout*10);

            /* Take the lock before killing the thread */
            m_NCS_LOCK(mds_lock(),NCS_LOCK_WRITE);

            /* Now two things have happened 
               (1) MDS thread has acked the destroy-event. So it will do no further things beyound
                   MDS unlock
               (2) We have obtained MDS-Lock. So, even the un-lock by MDS thead is completed
               Now we can proceed with the systematic destruction of MDS internal Data */

            /* Free the objects related to destroy-indication. The destroy mailbox event
               will be automatically freed by MDS processing or during MDS mailbox
               destruction. Since we will be destroying the MDS-thread, the following 
               selection-object can no longer be accessed. Hence, it is safe and correct 
               to destroy it now */
            m_NCS_SEL_OBJ_DESTROY(destroy_ack_obj);
            m_NCS_MEMSET(&destroy_ack_obj, 0, sizeof(destroy_ack_obj)); /* Destroy info */

            /* Sanity check */
            if (gl_mds_mcm_cb == NULL)
            {
                m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LIB_DESTROY : MDS is already Destroyed");
                m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
                return NCSCC_RC_FAILURE;
            }

            status = mds_mdtm_destroy();
            if (status != NCSCC_RC_SUCCESS)
            {
                /* vishal : todo anything? */
            }
            status = mds_mcm_destroy();
            if (status != NCSCC_RC_SUCCESS)
            {
                /* vishal : todo anything? */
            }


            /* Just Unlock the lock, Lock is never destroyed */ 
            m_NCS_UNLOCK(mds_lock(),NCS_LOCK_WRITE);
            break;

        default:
            break;
    }

    return NCSCC_RC_SUCCESS;
}
