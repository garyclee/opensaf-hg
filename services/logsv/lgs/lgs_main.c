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

#include <ncs_main_pvt.h>

#include "lgs.h"
#include "lgs_util.h"

/* ========================================================================
 *   DEFINITIONS
 * ========================================================================
 */ 

#define ALARM_LOG_FILENAME_BASE "saLogAlarm"
#define NOTIF_LOG_FILENAME_BASE "saLogNotification"
#define SYSTEM_LOG_FILENAME_BASE "saLogSystem"

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

/* ========================================================================
 *   FUNCTION PROTOTYPES
 * ========================================================================
 */ 

/**
 * Toggle trace categories
 * @param sig
 */
static void usr1_sig_handler(int sig)
{
    static int category_mask;

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
 * Dump information from all data structures
 * @param sig
 */
static void usr2_sig_handler(int sig)
{
    log_stream_t *stream;
    lga_reg_rec_t *reg_rec;

    TRACE("Control block information");
    TRACE("  comp_name:   %s", lgs_cb->comp_name.value);
    TRACE("  log_version: %c.%02d.%02d", lgs_cb->log_version.releaseCode,
           lgs_cb->log_version.majorVersion, lgs_cb->log_version.minorVersion);
    TRACE("  mds_role:    %u", lgs_cb->mds_role);
    TRACE("  last_reg_id: %u", lgs_cb->last_reg_id);
    TRACE("  ha_state:    %u", lgs_cb->ha_state);
    TRACE("  ckpt_state:  %u", lgs_cb->ckpt_state);

    TRACE("Client information");
    reg_rec = (lga_reg_rec_t *) ncs_patricia_tree_getnext(&lgs_cb->lga_reg_list, NULL);
    while (reg_rec != NULL)
    {
        lgs_stream_list_t *s = reg_rec->first_stream;
        TRACE("  reg_id: %u", reg_rec->reg_id);
        TRACE("    lga_client_dest: %llx", reg_rec->lga_client_dest);

        while (s != NULL)
        {
            TRACE("    stream id: %u", s->stream_id);
            s = s->next;
        }
        reg_rec = (lga_reg_rec_t *) ncs_patricia_tree_getnext(&lgs_cb->lga_reg_list,
                                                              (uns8 *)&reg_rec->reg_id_Net);
    }

    TRACE("Streams information");
    stream = (log_stream_t *) log_stream_getnext_by_name(NULL);
    while (stream != NULL)
    {
        log_stream_print(stream);
        stream = (log_stream_t *) log_stream_getnext_by_name(stream->name);
    }
    log_stream_id_print();
}

/**
 * Initialize lgs
 * 
 * @return uns32
 */
static uns32 lgs_init(const char *progname)
{
    uns32          rc;
    FILE           *fp = NULL;
    char path[NAME_MAX + 32];

    TRACE_ENTER();

    /* Initialize lgs control block */
    if (lgs_cb_init(lgs_cb) != NCSCC_RC_SUCCESS)
    {
        TRACE("lgs_cb_init FAILED");
        rc = NCSCC_RC_FAILURE;
        goto done;
    }

    /* Initialize stream class */
    if (log_stream_init() != NCSCC_RC_SUCCESS)
    {
        TRACE("log_stream_init FAILED");
        rc = NCSCC_RC_FAILURE;
        goto done;
    }

    /* Create pidfile */
    sprintf(path, "/var/run/%s.pid", basename(progname));
    if ((fp = fopen(path, "w")) == NULL)
    {
        LOG_ER("Could not open %s", path);
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
        TRACE("m_NCS_IPC_CREATE FAILED %d", rc);
        goto done;
    }

    /* Attach mailbox to this thread */
    if ((rc = m_NCS_IPC_ATTACH(&lgs_cb->mbx) != NCSCC_RC_SUCCESS))
    {
        TRACE("m_NCS_IPC_ATTACH FAILED %d", rc);
        goto done;
    }

    if ((rc = lgs_amf_init(lgs_cb)) != NCSCC_RC_SUCCESS)
    {
        TRACE("lgs_amf_init FAILED %d", rc);
        goto done;
    }

    if ((rc = lgs_mds_init(lgs_cb)) != NCSCC_RC_SUCCESS)
    {
        TRACE("lgs_mds_init FAILED %d", rc);
        return rc;
    }

    if ((rc = lgs_mbcsv_init(lgs_cb)) != NCSCC_RC_SUCCESS)
        TRACE("lgs_mbcsv_init FAILED");

done:
    TRACE_LEAVE();
    return(rc);
}

/**
 * Forever wait on events on AMF, MBCSV & LGS Mailbox and then process them.
 */
static void lgs_main_process(void)
{
    NCS_SEL_OBJ_SET     all_sel_obj;
    NCS_SEL_OBJ         mbx_fd, amf_ncs_sel_obj, mbcsv_sel_obj , numfds;
    SaAisErrorT         error = SA_AIS_OK;
    int                 count;
    uns32 rc;

    TRACE_ENTER();

    mbx_fd = m_NCS_IPC_GET_SEL_OBJ(&lgs_cb->mbx);

    /* Set the fd for mbcsv events */
    m_SET_FD_IN_SEL_OBJ(lgs_cb->mbcsv_sel_obj, mbcsv_sel_obj);

    while (1)
    {
        /* re-intialize the FDs and count */
        numfds.raise_obj = 0;
        numfds.rmv_obj = 0;
        m_NCS_SEL_OBJ_ZERO(&all_sel_obj);

        /* Get the selection object from the MBX */
        /* Set the fd for internal events */
        m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);
        numfds = m_GET_HIGHER_SEL_OBJ(mbx_fd, numfds);

        /* Set the fd for amf events */
        m_SET_FD_IN_SEL_OBJ(lgs_cb->amfSelectionObject, amf_ncs_sel_obj);
        m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);
        numfds = m_GET_HIGHER_SEL_OBJ(amf_ncs_sel_obj, numfds);

        /* set the Mbcsv selection object */
        m_NCS_SEL_OBJ_SET(mbcsv_sel_obj, &all_sel_obj);
        numfds = m_GET_HIGHER_SEL_OBJ(mbcsv_sel_obj, numfds);

        /** LGS thread main processing loop.
         **/
        count = m_NCS_SEL_OBJ_SELECT(numfds, &all_sel_obj,0,0,0); 
        if (count > 0)
        {
            /* process all the AMF messages */
            if (m_NCS_SEL_OBJ_ISSET(amf_ncs_sel_obj,&all_sel_obj))
            {
                /* dispatch all the AMF pending function */
                error = saAmfDispatch(lgs_cb->amf_hdl, SA_DISPATCH_ALL);
                if (error != SA_AIS_OK)
                    TRACE("saAmfDispatch FAILED: %u", error);
            }

            /* process all mbcsv messages */
            if (m_NCS_SEL_OBJ_ISSET(mbcsv_sel_obj,&all_sel_obj))
            {
                rc = lgs_mbcsv_dispatch(lgs_cb->mbcsv_hdl);
                if (rc != NCSCC_RC_SUCCESS)
                    TRACE("MBCSV DISPATCH FAILED: %u", rc);
                else
                    m_NCS_SEL_OBJ_CLR(mbcsv_sel_obj,&all_sel_obj);
            }

            /* Process the LGS Mail box, if lgs is ACTIVE. */
            if (m_NCS_SEL_OBJ_ISSET(mbx_fd, &all_sel_obj))
                lgs_process_mbx(&lgs_cb->mbx);
        }
        else
        {
            TRACE("select FAILED");
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

    /* Initialize trace system first of all so we can see what is going. */
    if ((value = getenv("LOGSV_TRACE_PATHNAME")) != NULL)
    {
        if (logtrace_init("lgs", value) != 0)
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

    if (ncspvt_svcs_startup(argc, argv, NULL) != NCSCC_RC_SUCCESS)
        goto done;

    if (lgs_init(argv[0]) != NCSCC_RC_SUCCESS)
        goto done;

    if (signal(SIGUSR1, usr1_sig_handler) == SIG_ERR)
        goto done;

    if (signal(SIGUSR2, usr2_sig_handler) == SIG_ERR)
        goto done;

    lgs_main_process();

done:
    LOG_ER("lgs initialization failed, exiting...");
    exit(1);
}

