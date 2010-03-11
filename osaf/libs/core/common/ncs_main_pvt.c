/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008-2010 The OpenSAF Foundation
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
 *            Wind River Systems
 *
 */

#include <ncsgl_defs.h>
#include "ncssysf_def.h"
#include "ncssysf_sem.h"
#include "ncs_main_pvt.h"
#include "ncs_main_papi.h"

#include "ncs_lib.h"
#include "mda_dl_api.h"
#include "nid_api.h"

#if (NCS_AVA == 1)
#include "ava_dl_api.h"
#endif

#if (NCS_MBCSV == 1)
#include "mbcsv_dl_api.h"
#endif

#if (NCS_GLA == 1)
#include "gla_dl_api.h"
#endif

#if (NCS_GLD == 1)
#include "gld_dl_api.h"
#endif

#if (NCS_GLND == 1)
#include "glnd_dl_api.h"
#endif

#if (NCS_CPD == 1)
#include "cpd_dl_api.h"
#endif

#if (NCS_CPND == 1)
#include "cpnd_dl_api.h"
#endif

#if (NCS_MQA == 1)
#include "mqa_dl_api.h"
#endif

#if (NCS_MQD == 1)
#include "mqd_dl_api.h"
#endif

#if (NCS_MQND == 1)
#include "mqnd_dl_api.h"
#endif

#if (NCS_DTA == 1)
#include "dta_dl_api.h"
#endif

#if (NCS_EDA == 1)
#include "eda_dl_api.h"
#endif

#if (NCS_EDS == 1)
#include "eds_dl_api.h"
#endif

#if (NCS_PDRBD == 1)
#include "pdrbd_dl_api.h"
#endif

#if (NCS_DTS == 1)
#include "dts.h"
#include "dts_dl_api.h"

#if (MBCSV_LOG == 1)
#include "mbcsv_log.h"
#endif

#if (NCS_AVSV_LOG == 1)
#include "avd_logstr.h"
#include "avnd_logstr.h"
#include "ava_logstr.h"
#include "avm_logstr.h"
#endif

#if (NCS_GLSV_LOG == 1)
#include "glsv_logstr.h"
#endif

#if (NCS_MQSV_LOG == 1)
#include "mqd_logstr.h"
#endif

#if (NCS_CPSV_LOG == 1)
#include "cpd_log.h"
#include "cpnd_log.h"
#include "cpa_log.h"
#endif

#if (NCS_EDSV_LOG == 1)
#include "edsv_logstr.h"
#endif
#endif   /* NCS_DTS */

/* NID specific NCS service global variable */
char gl_nid_svc_name[NID_MAXSNAME];

static uns32 ncs_d_nd_svr_startup(void);
static void ncs_get_nid_svc_name(int argc, char *argv[], char *o_nid_svc_name);


uns32 ncs_nid_notify(uns16 status)
{
	uns32 error;
	uns32 nid_stat_code;

	/* NID is OFF */
	if (strcmp(gl_nid_svc_name, "") == 0)
		return NCSCC_RC_SUCCESS;

	if (status != NCSCC_RC_SUCCESS)
		status = NCSCC_RC_FAILURE;

	nid_stat_code = status;

	/* CALL the NID specific API to notify */
	if (nid_notify(gl_nid_svc_name, nid_stat_code, &error) != NCSCC_RC_SUCCESS) {
		printf("\n NID failed, ERR: %d for gl_nid_svc_name :%s \n", error, gl_nid_svc_name);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}


uns32 ncspvt_svcs_startup(void)
{
	/* Init NCS core agents (Leap, NCS-Common, MDS, ADA/VDA, DTA, OAA */
	if (ncs_core_agents_startup() != NCSCC_RC_SUCCESS)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	/* start all the required directors/node director/servers */
	if (ncs_d_nd_svr_startup() != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return (NCSCC_RC_SUCCESS);
}


static uns32 ncs_d_nd_svr_startup(void)
{
	NCS_LIB_REQ_INFO lib_create;
#if (NCS_DTS == 1)
	char *p_field;
#endif

	/* Init LIB_CREATE request for Directors, NodeDirectors, and Server */
	memset(&lib_create, 0, sizeof(lib_create));
	lib_create.i_op = NCS_LIB_REQ_CREATE;
	lib_create.info.create.argc = 0;
	lib_create.info.create.argv = NULL;

	/* Init MBCA */
	if (ncs_mbca_startup() != NCSCC_RC_SUCCESS) {
		m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
		printf("MBCA start-up has been failed\n");
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

#if (NCS_DTS == 1)
	/* Init DTS */
	/* DTSV default service severity level */
	p_field = getenv("DTS_LOG_ALL");
	if (p_field != NULL) {
		if (atoi(p_field)) {
			m_NCS_DBG_PRINTF("\nINFO:DTS default service severity = ALL\n");
			gl_severity_filter = GLOBAL_SEVERITY_FILTER_ALL;
		}
	}

	m_NCS_DBG_PRINTF("\nDTSV:DTS:ON");
	if (dts_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
		printf("DTS lib request failed\n");
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

#endif   /* If NCS_DTS == 1 */

#if (NCS_EDS == 1)
	/* Init EDS */
	m_NCS_DBG_PRINTF("\nEDSV:EDS:ON");
	if (ncs_edsv_eds_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
		printf("EDS lib request failed\n");
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	} else
		m_NCS_DBG_PRINTF("\nEDSV:EDS libcreate success");
#endif

#if (NCS_MQND == 1)
	/* Init MQND */
	m_NCS_DBG_PRINTF("\nMQSV:MQND:ON");
	if (mqnd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
		printf("MQND lib request failed\n");
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
#endif

#if (NCS_PDRBD == 1)
	/* Initialize Pseudo DRBD service */
	m_NCS_DBG_PRINTF("\nPSEUDO_DRBD:ON\n");
	if (pseudoLibReq(&lib_create) != NCSCC_RC_SUCCESS) {
		printf("\n PSEUDO DRBD Lib Creation Failed");
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
	/* End of Pseudo DRBD Initialization */
#endif

	return (NCSCC_RC_SUCCESS);
}

#if (NCS_MAIN_EXTERNAL != 1)
int main(int argc, char *argv[])
{
	return ncspvt_load_config_n_startup(argc, argv);
}
#endif


/***************************************************************************

  PROCEDURE   :  ncs_get_nid_svc_id

  DESCRIPTION :  Function used to get the NID specific NCS service ID.
 
  RETURNS     :  Nothing
***************************************************************************/
static void ncs_get_nid_svc_name(int argc, char *argv[], char *o_nid_svc_name)
{
	char *p_field = NULL;
	char nid_svc_name[NID_MAXSNAME];

	p_field = ncs_util_search_argv_list(argc, argv, "NID_SVC_NAME=");
	if (p_field == NULL) {
		return;
	}

	if (sscanf(p_field + strlen("NID_SVC_NAME="), "%s", nid_svc_name) != 1) {
		printf("\nERROR:Problem in NID_SVC_NAME argument\n");
		return;
	}

	strcpy(o_nid_svc_name, nid_svc_name);

	return;
}


int ncspvt_load_config_n_startup(int argc, char *argv[])
{
	/*
	 * If the service is spawned by NID, then get the respective
	 * NID specific service ID
	 */
	ncs_get_nid_svc_name(argc, argv, gl_nid_svc_name);

	/* start up the node */
	if (ncspvt_svcs_startup() != NCSCC_RC_SUCCESS)
		return (NCSCC_RC_FAILURE);

	m_NCS_NID_NOTIFY(NCSCC_RC_SUCCESS);

	/* sleep here forever until you get signal */
	while (1) {
		m_NCS_TASK_SLEEP(0xfffffff0);
	}
	return (NCSCC_RC_SUCCESS);
}
