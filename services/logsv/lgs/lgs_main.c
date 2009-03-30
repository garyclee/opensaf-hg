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
 * Author(s): Ericsson AB
 *
 */

/* ========================================================================
 *   INCLUDE FILES
 * ========================================================================
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <configmake.h>

#include <ncs_main_pvt.h>

#include "lgs.h"
#include "lgs_util.h"

/* ========================================================================
 *   DEFINITIONS
 * ========================================================================
 */ 

#define FD_AMF 0
#define FD_MBCSV 1
#define FD_MBX 2
#define FD_IMM 3 /* Must be the last in the fds array */

/* ========================================================================
 *   TYPE DEFINITIONS
 * ========================================================================
 */ 

/* ========================================================================
 *   DATA DECLARATIONS
 * ========================================================================
 */ 

static lgs_cb_t _lgs_cb;
lgs_cb_t *lgs_cb = &_lgs_cb;

static int category_mask;

static struct pollfd fds[4];
static nfds_t nfds = 4;

/* ========================================================================
 *   FUNCTION PROTOTYPES
 * ========================================================================
 */ 

/**
 * USR1 signal handler to enable/disable trace (toggle)
 * @param sig
 */
static void usr1_sig_handler(int sig)
{
    if (category_mask == 0)
    {
        category_mask = CATEGORY_ALL;
        printf("Enabling traces");
    }
    else
    {
        category_mask = 0;
        printf("Disabling traces");
    }

    if (trace_category_set(category_mask) == -1)
        printf("trace_category_set failed");
}

/**
 * USR2 signal handler to dump information from all data structures
 * @param sig
 */
static void usr2_sig_handler(int sig)
{
    log_stream_t *stream;
    log_client_t *client;
    int old_category_mask = category_mask;

    if (trace_category_set(CATEGORY_ALL) == -1)
        printf("trace_category_set failed");

    TRACE("Control block information");
    TRACE("  comp_name:      %s", lgs_cb->comp_name.value);
    TRACE("  log_version:    %c.%02d.%02d", lgs_cb->log_version.releaseCode,
           lgs_cb->log_version.majorVersion, lgs_cb->log_version.minorVersion);
    TRACE("  mds_role:       %u", lgs_cb->mds_role);
    TRACE("  last_client_id: %u", lgs_cb->last_client_id);
    TRACE("  ha_state:       %u", lgs_cb->ha_state);
    TRACE("  ckpt_state:     %u", lgs_cb->ckpt_state);
    TRACE("  root_dir:       %s", lgs_cb->logsv_root_dir);

    TRACE("Client information");
    client = (log_client_t *) ncs_patricia_tree_getnext(&lgs_cb->client_tree, NULL);
    while (client != NULL)
    {
        lgs_stream_list_t *s = client->stream_list_root;
        TRACE("  client_id: %u", client->client_id);
        TRACE("    lga_client_dest: %llx", client->mds_dest);

        while (s != NULL)
        {
            TRACE("    stream id: %u", s->stream_id);
            s = s->next;
        }
        client = (log_client_t *) ncs_patricia_tree_getnext(&lgs_cb->client_tree,
                                                            (uns8 *)&client->client_id_net);
    }

    TRACE("Streams information");
    stream = (log_stream_t *) log_stream_getnext_by_name(NULL);
    while (stream != NULL)
    {
        log_stream_print(stream);
        stream = (log_stream_t *) log_stream_getnext_by_name(stream->name);
    }
    log_stream_id_print();

    if (trace_category_set(old_category_mask) == -1)
        printf("trace_category_set failed");
}

/**
 * Initialize lgs
 * 
 * @return uns32
 */
static uns32 initialize_lgs(const char *progname)
{
    uns32          rc;
    FILE           *fp = NULL;
    char path[NAME_MAX + 32];

    TRACE_ENTER();

    /* Initialize lgs control block */
    if (lgs_cb_init(lgs_cb) != NCSCC_RC_SUCCESS)
    {
        LOG_ER("lgs_cb_init FAILED");
        rc = NCSCC_RC_FAILURE;
        goto done;
    }

    /* Initialize stream class */
    if (log_stream_init() != NCSCC_RC_SUCCESS)
    {
        LOG_ER("log_stream_init FAILED");
        rc = NCSCC_RC_FAILURE;
        goto done;
    }

    /* Create pidfile */
    snprintf(path, sizeof(path), PIDPATH "%s.pid", basename(progname));
    if ((fp = fopen(path, "w")) == NULL)
    {
        LOG_ER("Could not open '%s': %s", path, strerror(errno));
        rc = NCSCC_RC_FAILURE; 
        goto done;
    }

    /* Write PID to it */
    if (fprintf(fp, "%d", getpid()) < 1)
    {
        fclose(fp);
        LOG_ER("Could not write to: %s", path);
        rc = NCSCC_RC_FAILURE;
        goto done;
    }
    fclose(fp);

    m_NCS_EDU_HDL_INIT(&lgs_cb->edu_hdl);

    /* Create the mailbox used for communication with LGS */
    if ((rc = m_NCS_IPC_CREATE(&lgs_cb->mbx)) != NCSCC_RC_SUCCESS)
    {
        LOG_ER("m_NCS_IPC_CREATE FAILED %d", rc);
        goto done;
    }

    /* Attach mailbox to this thread */
    if ((rc = m_NCS_IPC_ATTACH(&lgs_cb->mbx) != NCSCC_RC_SUCCESS))
    {
        LOG_ER("m_NCS_IPC_ATTACH FAILED %d", rc);
        goto done;
    }

    if ((rc = lgs_amf_init(lgs_cb)) != SA_AIS_OK)
    {
        LOG_ER("lgs_amf_init FAILED %d", rc);
        goto done;
    }

    if ((rc = lgs_mds_init(lgs_cb)) != NCSCC_RC_SUCCESS)
    {
        LOG_ER("lgs_mds_init FAILED %d", rc);
        return rc;
    }

    if ((rc = lgs_mbcsv_init(lgs_cb)) != NCSCC_RC_SUCCESS)
        LOG_ER("lgs_mbcsv_init FAILED");

    if ((rc = lgs_imm_init(lgs_cb)) != SA_AIS_OK)
        LOG_ER("lgs_imm_init FAILED");

done:
    TRACE_LEAVE();
    return(rc);
}

/**
 * Wait forever on IMM and do initialize, sel obj get &
 * implementer set.
 * @param arg
 * 
 * @return void*
 */
static void *imm_reinit_thread(void *arg)
{
    SaAisErrorT error;

    TRACE_ENTER();

    if ((error = lgs_imm_init(lgs_cb)) != SA_AIS_OK)
    {
        LOG_ER("lgs_imm_init FAILED: %u", error);
        exit(EXIT_FAILURE);
    }

    /* If this is the active server, become implementer again. */
    if (lgs_cb->ha_state == SA_AMF_HA_ACTIVE)
        lgs_imm_impl_set(lgs_cb);

    TRACE("New IMM fd: %llu", lgs_cb->immSelectionObject);
    fds[FD_IMM].fd = lgs_cb->immSelectionObject;
    nfds = FD_IMM + 1;

    TRACE_LEAVE();
    return NULL;
}

/**
 * Start a background thread to do IMM reinitialization.
 * 
 */
static void imm_reinit_bg(void)
{
    pthread_t thread;

    TRACE_ENTER();
    if (pthread_create(&thread, NULL, imm_reinit_thread, NULL) != 0)
    {
        LOG_ER("lgs_imm_init_bg FAILED: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    TRACE_LEAVE();
}

/**
 * Forever wait on events on AMF, MBCSV & LGS Mailbox file descriptors
 * and process them.
 */
static void main_process(void)
{
    NCS_SEL_OBJ         mbx_fd;
    SaAisErrorT         error = SA_AIS_OK;
    uns32 rc;

    TRACE_ENTER();

    mbx_fd = ncs_ipc_get_sel_obj(&lgs_cb->mbx);

    /* Set up all file descriptors to listen to */
    fds[FD_AMF].fd       = lgs_cb->amfSelectionObject;
    fds[FD_AMF].events   = POLLIN;
    fds[FD_MBCSV].fd     = lgs_cb->mbcsv_sel_obj;
    fds[FD_MBCSV].events = POLLIN;
    fds[FD_MBX].fd       = mbx_fd.rmv_obj;
    fds[FD_MBX].events   = POLLIN;
    fds[FD_IMM].fd       = lgs_cb->immSelectionObject;
    fds[FD_IMM].events   = POLLIN;

    while (1)
    {
        int ret = poll(fds, nfds, -1);

        if (ret == -1)
        {
            if (errno == EINTR)
                continue;

            LOG_ER("poll failed - %s", strerror(errno));
            break;
        }

        if (fds[FD_AMF].revents & POLLIN)
        {
            /* dispatch all the AMF pending function */
            if ((error = saAmfDispatch(lgs_cb->amf_hdl, SA_DISPATCH_ALL)) != SA_AIS_OK)
            {
                LOG_ER("saAmfDispatch failed: %u", error);
                break;
            }
        }

        if (fds[FD_MBCSV].revents & POLLIN)
        {
            if ((rc = lgs_mbcsv_dispatch(lgs_cb->mbcsv_hdl)) != NCSCC_RC_SUCCESS)
            {
                LOG_ER("MBCSv Dispatch Failed");
                break;
            }
        }

        if (fds[FD_MBX].revents & POLLIN)
            lgs_process_mbx(&lgs_cb->mbx);

        if (lgs_cb->immOiHandle && fds[FD_IMM].revents & POLLIN)
        {
            error = saImmOiDispatch(lgs_cb->immOiHandle, SA_DISPATCH_ALL);

            /*
            ** BAD_HANDLE is interpreted as an IMM service restart. Try 
            ** reinitialize the IMM OI API in a background thread and let 
            ** this thread do business as usual especially handling write 
            ** requests.
            **
            ** All other errors are treated as non-recoverable (fatal) and will
            ** cause an exit of the process.
            */
            if (error == SA_AIS_ERR_BAD_HANDLE)
            {
                TRACE("saImmOiDispatch returned BAD_HANDLE");

                /* 
                ** Invalidate the IMM OI handle, this info is used in other
                ** locations. E.g. giving TRY_AGAIN responses to a create and
                ** close app stream requests. That is needed since the IMM OI
                ** is used in context of these functions.
                */
                lgs_cb->immOiHandle = 0;

                /* 
                ** Skip the IMM file descriptor in next poll(), IMM fd must
                ** be the last in the fd array.
                */
                nfds = FD_MBX + 1;

                /* Initiate IMM reinitializtion in the background */
                imm_reinit_bg();
            }
            else if (error != SA_AIS_OK)
            {
                LOG_ER("saImmOiDispatch FAILED: %u", error);
                break;
            }
        }
    }
}

/**
 * The main routine for the lgs daemon.
 * @param argc
 * @param argv
 * 
 * @return int
 */
int main(int argc, char *argv[])
{
    char *value;

    /* Initialize trace system first of all so we can see what is going on. */
    if ((value = getenv("LOGSV_TRACE_PATHNAME")) != NULL)
    {
        if (logtrace_init(basename(argv[0]), value) != 0)
        {
            syslog(LOG_ERR, "logtrace_init FAILED, exiting...");
            goto done;
        }

        if ((value = getenv("LOGSV_TRACE_CATEGORIES")) != NULL)
        {
            /* Do not care about categories now, get all */
            trace_category_set(CATEGORY_ALL);
        }
    }

    /*
    ** Get LOGSV root directory path. All created log files will be stored
    ** relative to this directory as described in spec.
    */
    if ((value = getenv("LOGSV_ROOT_DIRECTORY")) == NULL)
    {
        syslog(LOG_ERR, "LOGSV_ROOT_DIRECTORY not found, exiting...");
        goto done;
    }

    /* Save path in control block */
    lgs_cb->logsv_root_dir = value;

    if (ncspvt_svcs_startup(argc, argv, NULL) != NCSCC_RC_SUCCESS)
    {
        LOG_ER("ncspvt_svcs_startup failed");
        goto done;
    }

    if (initialize_lgs(argv[0]) != NCSCC_RC_SUCCESS)
    {
        LOG_ER("initialize_lgs failed");
        goto done;
    }

    if (signal(SIGUSR1, usr1_sig_handler) == SIG_ERR)
    {
        LOG_ER("signal SIGUSR1 failed");
        goto done;
    }

    if (signal(SIGUSR2, usr2_sig_handler) == SIG_ERR)
    {
        LOG_ER("signal SIGUSR2 failed");
        goto done;
    }

    main_process();

done:
    LOG_ER("failed, exiting...");
    exit(1);
}

