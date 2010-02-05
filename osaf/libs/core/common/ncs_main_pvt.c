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

  MODULE NAME:       ncs_main.c   

  DESCRIPTION:       Contains an API that starts up NCS.

******************************************************************************
*/

#include <ncsgl_defs.h>
#include "ncssysf_def.h"
#include "ncssysf_sem.h"
#include "ncs_main_pvt.h"
#include "ncs_main_papi.h"

#include "ncs_lib.h"
#include "mda_dl_api.h"

#if (NCS_VDS == 1)
#include "vds_dl_api.h"
#endif

#if (NCS_AVA == 1)
#include "ava_dl_api.h"
#endif
#if (NCS_AVD == 1)
#include "avd_dl_api.h"
#endif
#if (NCS_AVND == 1)
#include "avnd_dl_api.h"
#endif

#if (NCS_AVM == 1)
#include "avm_dl_api.h"
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

#if (NCS_HISV == 1)
#include "hisv_dl_api.h"
#endif

#if (NCS_SUND == 1)
#include "sund_dl_api.h"
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

#if (NCS_HISV_LOG == 1)
#include "hisv_logstr.h"
#endif

#if (SUND_LOG ==1)
#include "sund_log.h"
#endif

#if (NCSCLI_LOG == 1)
#include "clilog.h"
#endif
#endif   /* NCS_DTS */

#if (PCS_HALS_MONC == 1)
#include "hals.h"
#elif (PCS_HALS_PROXY == 1)
#include "hals.h"
#endif

/* NID specific hdr file */
#include "nid_api.h"


static uns32 ncs_d_nd_svr_startup(int argc, char *argv[]);
static uns32 ncs_d_nd_svr_shutdown(int argc, char *argv[]);
static uns32 mainget_svc_enable_info(char **pargv, uns32 *pargc, FILE *fp);

#if (NCS_AVND == 1)
NCSCONTEXT avnd_sem;
static void main_avnd_usr1_signal_hdlr(int signal);
#endif
/* NID specific NCS service global variable */
char gl_nid_svc_name[NID_MAXSNAME];
static void ncs_get_nid_svc_name(int argc, char *argv[], char *o_nid_svc_name);

/***************************************************************************\

  PROCEDURE    :    ncs_nid_notify

\***************************************************************************/
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

/***************************************************************************\

  PROCEDURE    :    ncspvt_svcs_startup

\***************************************************************************/
uns32 ncspvt_svcs_startup(int argc, char *argv[], FILE *fp)
{
   /*** Init NCS core agents (Leap, NCS-Common, MDS, ADA/VDA, DTA, OAA ***/
	if (ncs_core_agents_startup(argc, argv) != NCSCC_RC_SUCCESS)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	syslog(LOG_INFO, "NODE_ID=0x%08X PID %u \n", ncs_get_node_id(), getpid());

	if (mainget_svc_enable_info(gl_pargv, &gl_pargc, fp) != NCSCC_RC_SUCCESS)
		return (NCSCC_RC_FAILURE);

   /*** start all the required directors/node director/servers ***/
	if (ncs_d_nd_svr_startup(gl_pargc, gl_pargv) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return (NCSCC_RC_SUCCESS);
}

/***************************************************************************\

  PROCEDURE    :    ncspvt_svcs_shutdown

\***************************************************************************/
uns32 ncspvt_svcs_shutdown(int argc, char *argv[])
{
   /*** start all the required directors/node director/servers ***/
	return ncs_d_nd_svr_shutdown(argc, argv);
}

/***************************************************************************\

  PROCEDURE    :    ncs_d_nd_svr_startup

\***************************************************************************/
static uns32 ncs_d_nd_svr_startup(int argc, char *argv[])
{
	NCS_LIB_REQ_INFO lib_create;
#if (NCS_DTS == 1)
	char *p_field;
#endif

   /*** Init LIB_CREATE request for Directors, NodeDirectors, 
   and Server ***/
	memset(&lib_create, 0, sizeof(lib_create));
	lib_create.i_op = NCS_LIB_REQ_CREATE;
	lib_create.info.create.argc = argc;
	lib_create.info.create.argv = argv;

   /*** Init MBCA ***/
	if (ncs_mbca_startup(argc, argv) != NCSCC_RC_SUCCESS) {
		m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
		printf("MBCA start-up has been failed\n");
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

   /*** Init DTS ***/
#if (NCS_DTS == 1)
   /****** DTSV default service severity level *******/
	p_field = getenv("DTS_LOG_ALL");
	if (p_field != NULL) {
		if (atoi(p_field)) {
			m_NCS_DBG_PRINTF("\nINFO:DTS default service severity = ALL\n");
			gl_severity_filter = GLOBAL_SEVERITY_FILTER_ALL;
		}
	}

	if ('n' != ncs_util_get_char_option(argc, argv, "DTSV=")) {
		m_NCS_DBG_PRINTF("\nDTSV:DTS:ON");
		if (dts_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
			m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
			printf("DTS lib request failed\n");
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
/*      
#if (NCS_AVSV_LOG == 1)
      ava_str_reg();
      avnd_str_reg();
      avd_reg_strings();
      avm_reg_strings();
      cla_str_reg();
#endif

#if (NCS_GLSV_LOG == 1)
      gld_reg_strings();
      gla_reg_strings();
      glnd_reg_strings();
#endif

#if (NCS_MQSV_LOG == 1)
      mqa_log_ascii_reg();
      mqd_log_ascii_reg();
      mqnd_log_ascii_reg();
#endif
#if (NCS_CPSV_LOG == 1)
      cpnd_log_ascii_reg();
      cpd_log_ascii_reg();
      cpa_log_ascii_reg();
#endif

#if (NCS_BAM_LOG == 1)
      bam_reg_strings();
#endif

#if (NCS_EDSV_LOG == 1)
      eda_flx_log_ascii_set_reg();
      eds_flx_log_ascii_set_reg();
#endif

#if (NCS_HISV_LOG == 1)
      hisv_reg_strings();
#endif

#if (NCSCLI_LOG == 1)
   cli_log_ascii_reg();
#endif
*/
	}
#endif   /* If NCS_DTS == 1 */

	if ('n' != ncs_util_get_char_option(argc, argv, "EDSV=")) {
#if (NCS_EDS == 1)
      /*** Init EDS ***/
		m_NCS_DBG_PRINTF("\nEDSV:EDS:ON");
		if (ncs_edsv_eds_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
			m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
			printf("EDS lib request failed\n");
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		} else
			m_NCS_DBG_PRINTF("\nEDSV:EDS libcreate success");
#endif
	}

	if ('n' != ncs_util_get_char_option(argc, argv, "CPSV=")) {
#if (NCS_CPD == 1)
      /*** Init CPD ***/
		m_NCS_DBG_PRINTF("\nCPSV:CPD:ON");
		if (cpd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
			m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
			printf("CPD lib request failed\n");
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
#endif
#if (NCS_CPND == 1)
      /*** Init CPND ***/
		m_NCS_DBG_PRINTF("\nCPSV:CPND:ON");
		if (cpnd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
			m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
			printf("CPND lib request failed\n");
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
#endif
	}

	if ('n' != ncs_util_get_char_option(argc, argv, "MQSV=")) {
#if (NCS_MQD == 1)
      /*** Init MQD ***/
		m_NCS_DBG_PRINTF("\nMQSV:MQD:ON");
		if (mqd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
			m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
			printf("MQD lib request failed\n");
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
#endif

#if (NCS_MQND == 1)
      /*** Init MQND ***/
		m_NCS_DBG_PRINTF("\nMQSV:MQND:ON");
		if (mqnd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
			m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
			printf("MQND lib request failed\n");
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
#endif
	}

	if ('n' != ncs_util_get_char_option(argc, argv, "GLSV=")) {
#if (NCS_GLD == 1)
      /*** Init GLD ***/
		m_NCS_DBG_PRINTF("\nGLSV:GLD:ON");
		if (gld_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
			m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
			printf("GLD lib request failed\n");
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
#endif
#if (NCS_GLND == 1)
      /*** Init GLND ***/
		m_NCS_DBG_PRINTF("\nGLSV:GLND:ON");
		if (glnd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
			m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
			printf("GLND lib request failed\n");
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
#endif
	}
   /*** Init VDS ***/
#if (NCS_VDS == 1)
	if ('n' != ncs_util_get_char_option(argc, argv, "VDSV=")) {
		m_NCS_DBG_PRINTF("\nVDS:ON");
		if (vds_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
			m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
			printf("VDS lib request failed\n");
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		}
	}
#endif

	if ('n' != ncs_util_get_char_option(argc, argv, "HISV=")) {
#if (NCS_HISV == 1)
      /*** Init HISV ***/
		m_NCS_DBG_PRINTF("\nHISV:ON");
		if (ncs_hisv_hcd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
			m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
			printf("\nHISV:HCD lib create failed");
			return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
		} else
			m_NCS_DBG_PRINTF("\nHISV:HCD libcreate success");
#endif
      /*** Init HISV-HPL ***/

	}
#if (PCS_HALS_MONC == 1)
	/* Initialize Monitoring Component */
	printf("\nMonitoring:ON\n");
	if (pcs_svc_lib_req(&lib_create, PCS_SERVICE_ID_MONC) != NCSCC_RC_SUCCESS)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	/* End of LSMC */
#endif

#if (PCS_HALS_PROXY == 1)
	/* Initialize Proxy Component */
	m_NCS_DBG_PRINTF("\nProxy:ON\n");
	if (pcs_svc_lib_req(&lib_create, PCS_SERVICE_ID_PROXY) != NCSCC_RC_SUCCESS)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	/* End of LSMC */
#endif

#if (NCS_SUND == 1)
	/* Initialize SUND service */
	m_NCS_DBG_PRINTF("\nSSU_SUND:ON\n");
	if (sund_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		printf("\nSSU: SUND Lib Creation Failed");
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
	/* End of SUND Initialization */
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

/***************************************************************************\

  PROCEDURE    :    ncs_d_nd_svr_shutdown

\***************************************************************************/
static uns32 ncs_d_nd_svr_shutdown(int argc, char *argv[])
{
	NCS_LIB_REQ_INFO lib_destroy;

#if (NCS_VDS == 1)
	uns32 dummy_status;
#endif

	memset(&lib_destroy, 0, sizeof(lib_destroy));
	lib_destroy.i_op = NCS_LIB_REQ_DESTROY;

   /*** Destroy VDS ***/
#if (NCS_VDS == 1)
	if (vds_lib_req(&lib_destroy) != NCSCC_RC_SUCCESS)
		/* Fall through even in case of failure */
		dummy_status = m_LEAP_DBG_SINK(0);
#endif

#if (NCS_SUND == 1)
	/* Shutdown SUND */
	sund_lib_req(&lib_destroy);
	/*End of SUND shutdown */
#endif

#if (NCS_PDRBD == 1)
	/* Shutdown PDRBD */
	pseudoLibReq(&lib_destroy);
	/*End of PDRBD shutdown */
#endif

	return NCSCC_RC_SUCCESS;
}

/* If NCS_MAIN_EXTERNAL is enabled, then an external main() controls the
startup. This is useful when starting up from Powercode, Tetware
or Demo which need to have their own main().
*/

#ifndef NCS_MAIN_EXTERNAL
#if (NCS_POWERCODE == 1)
/* So that people currently using POWERCODE are not surprised by 
   a duplicate main. This check on NCS_POWERCODE can be removed later
*/
#define NCS_MAIN_EXTERNAL 1
#else
#define NCS_MAIN_EXTERNAL 0
#endif
#endif

uns32 mds_hub_flag = 0;
uns32 gl_pcon_id = NCS_MAIN_DEF_PCON_ID;

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
	FILE *fp = NULL;
	char *p_field = NULL;
#if (NCS_AVND == 1)
	NCS_LIB_REQ_INFO lib_destroy;
#endif

	/* If the service is spawned by NID, then get the respective
	   NID specific service ID */
	ncs_get_nid_svc_name(argc, argv, gl_nid_svc_name);

	if (argc == 2) {
		p_field = ncs_util_search_argv_list(argc, argv, "CONFFILE=");
		if (p_field != NULL) {
	  /*** validate the file ***/
			fp = fopen(p_field + strlen("CONFFILE="), "r");
			if (fp == NULL) {
				m_NCS_NID_NOTIFY(NCSCC_RC_FAILURE);
				printf("Wrong service file name svc_nmae:%s \n", gl_nid_svc_name);
			}
		}
	}

   /** start up the node **/
	if (ncspvt_svcs_startup(argc, argv, fp) != NCSCC_RC_SUCCESS) {
		if (fp)
			fclose(fp);
		return (NCSCC_RC_FAILURE);
	}

	if (fp)
		fclose(fp);

	/* Notify NID about the success of service creation 
	 ** For SCAP/PCAP process the notification will be done
	 ** after all the NCS processes are up 
	 */
	if ((strcmp(gl_nid_svc_name, "SCAP") != 0) && (strcmp(gl_nid_svc_name, "PCAP") != 0)) {
		m_NCS_NID_NOTIFY(NCSCC_RC_SUCCESS);
	}
#if (NCS_AVND == 1)
	/* initialize the signal handler */
	m_NCS_SIGNAL(SIGUSR1, main_avnd_usr1_signal_hdlr);

	if (m_NCS_SEM_CREATE(&avnd_sem) == NCSCC_RC_FAILURE) {
		return NCSCC_RC_FAILURE;
	}
#endif

   /** sleep here forever until you get signal.**/
	while (1) {
#if (NCS_AVND == 1)
		m_NCS_SEM_TAKE(avnd_sem);
		memset(&lib_destroy, 0, sizeof(lib_destroy));
		lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
		avnd_lib_req(&lib_destroy);
#else
		m_NCS_TASK_SLEEP(0xfffffff0);
#endif
	}
	return (NCSCC_RC_SUCCESS);
}

static uns32 mainget_svc_enable_info(char **pargv, uns32 *pargc, FILE *fp)
{
	char get_word[512];
	uns32 res;
#if (MDS_HUB == 1)
	pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
	strcpy(pargv[*pargc], "HUB=y");
	*pargc = (*pargc) + 1;
#endif
#if (TDS_MASTER == 1)
	pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
	strcpy(pargv[*pargc], "MASTER=y");
	*pargc = (*pargc) + 1;
#endif
   /*** There cane be a case where we don't want to enable any services ***/
	if (fp == NULL) {
		return (NCSCC_RC_SUCCESS);
	} else {
		while ((res = file_get_word(&fp, get_word)) != NCS_MAIN_EOF) {
			if (*pargc != NCS_MAIN_MAX_INPUT) {
				if (get_word[0] == '#') {
					continue;
				}
				if (strncmp(get_word, "RCP_LOG_MASK", sizeof("RCP_LOG_MASK") - 1) == 0) {
					pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
					strcpy(pargv[*pargc], get_word);
					*pargc = (*pargc) + 1;
					continue;
				}
				if (strncmp(get_word, "TDS_TRACE_LEVEL", sizeof("TDS_TRACE_LEVEL") - 1) == 0) {
					pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
					strcpy(pargv[*pargc], get_word);
					*pargc = (*pargc) + 1;
					continue;
				}
				if (strcmp(get_word, "HUB=y") == 0 || strcmp(get_word, "HUB=Y") == 0) {
					pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
					strcpy(pargv[*pargc], get_word);
					*pargc = (*pargc) + 1;
					continue;
				}

				if (strcmp(get_word, "MASTER=y") == 0 || strcmp(get_word, "MASTER=Y") == 0) {
					pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
					strcpy(pargv[*pargc], get_word);
					*pargc = (*pargc) + 1;
					continue;
				}

				if (strncmp(get_word, "MASTER_POLL_TMR", sizeof("MASTER_POLL_TMR") - 1) == 0) {
					pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
					strcpy(pargv[*pargc], get_word);
					*pargc = (*pargc) + 1;
					continue;
				}

				if (strncmp
				    (get_word, "MASTER_KEEP_ALIVE_TO_POLL_RATIO",
				     sizeof("MASTER_KEEP_ALIVE_TO_POLL_RATIO") - 1) == 0) {
					pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
					strcpy(pargv[*pargc], get_word);
					*pargc = (*pargc) + 1;
					continue;
				}

				if (strncmp(get_word, "SLAVE_HB_TMR", sizeof("SLAVE_HB_TMR") - 1) == 0) {
					pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
					strcpy(pargv[*pargc], get_word);
					*pargc = (*pargc) + 1;
					continue;
				}

				if (strncmp(get_word, "SLAVE_HB_TO_WATCH_RATIO", sizeof("SLAVE_HB_TO_WATCH_RATIO") - 1)
				    == 0) {
					pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
					strcpy(pargv[*pargc], get_word);
					*pargc = (*pargc) + 1;
					continue;
				}

				/* else store whatever comes */
				pargv[*pargc] = (char *)malloc(NCS_MAX_STR_INPUT);
				strcpy(pargv[*pargc], get_word);
				*pargc = (*pargc) + 1;
			} else {
				break;
			}
		}
	}

	/*  fclose() removed. It has to be taken care of by calling function */

	return (NCSCC_RC_SUCCESS);
}

#if (NCS_AVND == 1)
static void main_avnd_usr1_signal_hdlr(int signal)
{
	if (signal == SIGUSR1) {
		m_NCS_SEM_GIVE(avnd_sem);
	}
	return;
}

#endif
