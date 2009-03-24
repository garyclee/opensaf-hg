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


  MODULE NAME:       ncs_main_pub.c

  DESCRIPTION:       Contains an API that agent starts up NCS.

******************************************************************************
*/

#include <configmake.h>
#include "gl_defs.h"
#include "mds_papi.h"
#include "ncs_opt.h"
#include "t_suite.h"
#include "ncs_main_papi.h"
#include "ncs_mda_papi.h"
#include "ncs_sprr_papi.h"
#include "oac_papi.h"
#include "ncs_main_pvt.h"
#include "ncs_lib.h"
#include "mds_dl_api.h"
#include "sprr_dl_api.h"
#include "mda_dl_api.h"
#include "ncs_mib_pub.h"
#include "oac_dl_api.h"
#if ((NCS_MAS == 1) || (NCS_MAC == 1))
#include "mab.h"
#endif

#if (NCS_VDS == 1)
#include "vds_dl_api.h"
#endif

#if (NCS_CLI == 1)
#include "ncs_cli.h"
#endif

#if (NCS_PSR == 1)
#include "psr_papi.h"
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

#if (NCS_BAM == 1)
#include "bam.h"
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

#if (NCS_IFA == 1)
#include "ifa_dl_api.h"
#endif
#if (NCS_IFD == 1)
#include "ifd_dl_api.h"
#endif
#if (NCS_IFND == 1)
#include "ifnd_dl_api.h"
#endif

#if (NCS_SRMA == 1)
#include "srma_dl_api.h"
#endif
#if (NCS_SRMND == 1)
#include "srmnd_dl_api.h"
#endif

#if (NCS_MQA == 1)
#include "mqa_dl_api.h"
#endif
#if (NCS_MQD == 1)
#include "mqd_dl_api.h"
#endif
#if (NCS_MQND == 1)

#endif

#if (NCS_CPA == 1)
#include "cpa_dl_api.h"
#endif
#if (NCS_CPD == 1)
#include "cpd_dl_api.h"
#endif
#if (NCS_CPND == 1)
#include "cpnd_dl_api.h"
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

#if (NCS_DTS == 1)
#include "dts_dl_api.h"

#if (NCS_SNMPSUBAGT_LOG == 1)
#include "subagt_log.h"
#endif

#if (NCS_IFSV_LOG == 1)
#include "ifsv_logstr.h"
#endif

#if (NCS_AVSV_LOG == 1)
#include "avd_logstr.h"
#include "avnd_logstr.h"
#include "ava_logstr.h"
#endif

#if (NCS_SRMSV_LOG == 1)
#include "srmnd_logstr.h"
#include "srma_logstr.h"
#endif

#if (NCS_GLSV_LOG == 1)
#include "glsv_logstr.h"
#endif

#if (NCSMQD_LOG == 1)
#include "mqd_logstr.h"
#endif

#endif /* NCS_DTS */

#if (NCS_MAC == 1)
#include "mac_papi.h"
#endif

#include "ncs_sprr_papi.h"



#include "ncs_mib.h"
#include "ncsmiblib.h"


#ifdef __NCSINC_LINUX__
#define LOG_PATH OSAF_LOCALSTATEDIR "log"
#else
#define LOG_PATH   ""
#endif

#define NODE_ID_FILE OSAF_LOCALSTATEDIR "node_id"

/**************************************************************************\

       L O C A L      D A T A    S T R U C T U R E S

\**************************************************************************/

typedef uns32 (*LIB_REQ)(NCS_LIB_REQ_INFO *);

typedef struct ncs_agent_data
{
   uns32    use_count;
   LIB_REQ  lib_req;
} NCS_AGENT_DATA;

typedef struct ncs_main_pub_cb
{
#ifdef __NCSINC_WIN32__
   HINSTANCE           *lib_hdl;
#else
   NCS_OS_DLIB_HDL     *lib_hdl;
#endif

   NCS_LOCK            lock;
   uns32               lock_create;
   NCS_BOOL            core_started;
   uns32               my_nodeid;
   uns32               my_procid;

   uns32               core_use_count;
   uns32               leap_use_count;
   uns32               mds_use_count;
   uns32               dta_use_count;
   uns32               oac_use_count;

   NCS_AGENT_DATA      mbca;
   NCS_AGENT_DATA      ifa;
   NCS_AGENT_DATA      maa;
   NCS_AGENT_DATA      ncs_hpl;

} NCS_MAIN_PUB_CB;

static uns32 ncs_main_set_log_dir(void);
static uns32 ncs_main_create_log_dir(char *path);
static uns32 mainget_node_id(uns32 *node_id);
static uns32 ncs_set_config_root(void);
static uns32 ncs_util_get_sys_params(NCS_SYS_PARAMS *sys_params);
static uns32 ncs_non_core_agents_startup(int argc, char *argv[]);
static void ncs_get_sys_params_arg(int i_argc,
                                   char *i_argv[], 
                                   NCS_SYS_PARAMS *sys_params);
static uns32 ncs_update_sys_param_args(int argc, char *argv[]);

static char ncs_config_root[MAX_NCS_CONFIG_FILEPATH_LEN + 1];


#ifndef NCSMAINPUB_TRACE_LEVEL
#define NCSMAINPUB_TRACE_LEVEL 1
#endif

#if (NCSMAINPUB_TRACE_LEVEL == 1)
#define NCSMAINPUB_TRACE1_ARG1(x)    printf(x)
#define NCSMAINPUB_TRACE1_ARG2(x,y)  printf(x,y)

#define NCSMAINPUB_DBG_TRACE1_ARG1(x)    m_NCS_DBG_PRINTF(x)
#define NCSMAINPUB_DBG_TRACE1_ARG2(x,y)  m_NCS_DBG_PRINTF(x,y)
#else
#define NCSMAINPUB_TRACE1_ARG1(x)
#define NCSMAINPUB_TRACE1_ARG2(x,y)

#define NCSMAINPUB_DBG_TRACE1_ARG1(x)   
#define NCSMAINPUB_DBG_TRACE1_ARG2(x,y)
#endif


static NCS_MAIN_PUB_CB gl_ncs_main_pub_cb;

/* Global argument definitions */
char  *gl_pargv[NCS_MAIN_MAX_INPUT];
uns32 gl_pargc = 0;

EXTERN_C NCS_BOOL dts_sync_up_flag; 

/* Agent specific LOCKs */
#define m_NCS_AGENT_LOCK                                 \
   if (!gl_ncs_main_pub_cb.lock_create++)                \
   {                                                     \
      m_NCS_LOCK_INIT(&gl_ncs_main_pub_cb.lock);         \
   }                                                     \
   gl_ncs_main_pub_cb.lock_create = 1;                   \
   m_NCS_LOCK(&gl_ncs_main_pub_cb.lock, NCS_LOCK_WRITE); 
      
#define m_NCS_AGENT_UNLOCK m_NCS_UNLOCK(&gl_ncs_main_pub_cb.lock, NCS_LOCK_WRITE)


#define NCS_LOG_DIR_NAME_MAXLEN 256
char gl_ncs_log_dir[NCS_LOG_DIR_NAME_MAXLEN]; /* To store the path for logging directory */

/***************************************************************************\

  PROCEDURE    :    ncs_agents_startup

\***************************************************************************/
unsigned int ncs_agents_startup(int argc, char *argv[])
{
   uns32 rc = NCSCC_RC_SUCCESS;

   rc = ncs_core_agents_startup(argc, argv);
   if (rc != NCSCC_RC_SUCCESS)
      return rc;
   
   /* From now on, use gl_pargc & gl_pargv */
   rc = ncs_non_core_agents_startup(gl_pargc, gl_pargv);
   if (rc != NCSCC_RC_SUCCESS)
      return rc;

   return rc;
}


/***************************************************************************\

  PROCEDURE    :    ncs_agents_shutdown

\***************************************************************************/
unsigned int ncs_agents_shutdown(int argc, char *argv[])
{
   ncs_maa_shutdown();
   ncs_ifa_shutdown();
   ncs_mbca_shutdown();

   ncs_core_agents_shutdown();

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\

  PROCEDURE    :    ncs_leap_startup

\***************************************************************************/
unsigned int ncs_leap_startup(int argc, char *argv[])
{
   NCS_LIB_REQ_INFO lib_create;
   char             *p_field;

   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.leap_use_count > 0)
   {
      gl_ncs_main_pub_cb.leap_use_count++;
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_SUCCESS;
   }   

   /* Print the process-id for information sakes */
   gl_ncs_main_pub_cb.my_procid = (uns32)getpid();
   NCSMAINPUB_DBG_TRACE1_ARG2("\nNCS:PROCESS_ID=%d\n", gl_ncs_main_pub_cb.my_procid);

   memset(&lib_create, 0, sizeof(lib_create));
   lib_create.i_op = NCS_LIB_REQ_CREATE;
   lib_create.info.create.argc = argc;
   lib_create.info.create.argv = argv;

   if (ncs_main_set_log_dir() != NCSCC_RC_SUCCESS)
   {
      NCSMAINPUB_TRACE1_ARG1("\nERROR: Couldn't create log directory\n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   /*** Initalize basic services ***/
   if (leap_env_init() != NCSCC_RC_SUCCESS)
   {
      NCSMAINPUB_TRACE1_ARG1("\nERROR: Couldn't initialised LEAP basic services \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   if (sprr_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {      
      NCSMAINPUB_TRACE1_ARG1("\nERROR: SPRR lib_req failed \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   /****** DTSV SYNC flag *******/
   p_field = ncs_util_search_argv_list(argc, argv, "DTSV_SYNC_UP=");
   if (p_field != NULL)
   {
      if (sscanf(p_field + strlen("DTSV_SYNC_UP="), "%d", &dts_sync_up_flag) != 1)
      {
         NCSMAINPUB_TRACE1_ARG1("ERROR:Problem in dts_sync_up argument\n");
         m_NCS_AGENT_UNLOCK;
         return NCSCC_RC_FAILURE;
      }
   }   

   gl_ncs_main_pub_cb.leap_use_count = 1;

   m_NCS_AGENT_UNLOCK;

     /*** start initializing all the required agents ***/
#ifdef __NCSINC_WIN32__
   gl_ncs_main_pub_cb.lib_hdl = m_NCS_OS_DLIB_LOAD("NCS_DLL", m_NCS_OS_DLIB_ATTR);
#else
   gl_ncs_main_pub_cb.lib_hdl = m_NCS_OS_DLIB_LOAD(NULL, m_NCS_OS_DLIB_ATTR);
#endif

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\

  PROCEDURE    :    ncs_mds_startup

\***************************************************************************/
unsigned int ncs_mds_startup(int argc, char *argv[])
{
   NCS_LIB_REQ_INFO lib_create;


   m_NCS_AGENT_LOCK;
   
   if (!gl_ncs_main_pub_cb.leap_use_count)
   {
      NCSMAINPUB_TRACE1_ARG1("\nLEAP core not yet started.... \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   if (gl_ncs_main_pub_cb.mds_use_count > 0)
   {
      gl_ncs_main_pub_cb.mds_use_count++;
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_SUCCESS;
   }
   
   /* Get & Update system specific arguments */
   if (ncs_update_sys_param_args(argc, argv) != NCSCC_RC_SUCCESS)
   {
      NCSMAINPUB_TRACE1_ARG1("ERROR: Update System Param args \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   memset(&lib_create, 0, sizeof(lib_create));
   lib_create.i_op = NCS_LIB_REQ_CREATE;
   lib_create.info.create.argc = gl_pargc;
   lib_create.info.create.argv = gl_pargv;

   /* STEP : Initialize the MDS layer */
   if (mds_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {      
      NCSMAINPUB_TRACE1_ARG1("ERROR: MDS lib_req failed \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   /* STEP : Initialize the ADA/VDA layer */
   if (mda_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {      
      NCSMAINPUB_TRACE1_ARG1("ERROR: MDA lib_req failed \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   gl_ncs_main_pub_cb.mds_use_count = 1;

   m_NCS_AGENT_UNLOCK;

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\

  PROCEDURE    :    ncs_dta_startup

\***************************************************************************/
unsigned int ncs_dta_startup(int argc, char *argv[])
{
   NCS_LIB_REQ_INFO lib_create;

   
   m_NCS_AGENT_LOCK;

   if (!gl_ncs_main_pub_cb.leap_use_count)
   {
      NCSMAINPUB_TRACE1_ARG1("\nLEAP not yet started.... \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   if (!gl_ncs_main_pub_cb.mds_use_count)
   {
      NCSMAINPUB_TRACE1_ARG1("\nMDS not yet started.... \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   if (gl_ncs_main_pub_cb.dta_use_count > 0)
   {
      gl_ncs_main_pub_cb.dta_use_count++;
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_SUCCESS;
   }

   memset(&lib_create, 0, sizeof(lib_create));
   lib_create.i_op = NCS_LIB_REQ_CREATE;
   lib_create.info.create.argc = argc;
   lib_create.info.create.argv = argv;

   /* STEP : Initialize the DTA layer */
   if (dta_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {      
      NCSMAINPUB_TRACE1_ARG1("ERROR: DTA lib_req failed \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   gl_ncs_main_pub_cb.dta_use_count = 1;

   m_NCS_AGENT_UNLOCK;

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\

  PROCEDURE    :    ncs_oac_startup

\***************************************************************************/
unsigned int ncs_oac_startup(int argc, char *argv[])
{
   NCS_LIB_REQ_INFO lib_create;


   m_NCS_AGENT_LOCK;

   if (!gl_ncs_main_pub_cb.leap_use_count)
   {
      NCSMAINPUB_TRACE1_ARG1("\nLEAP not yet started.... \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   if (!gl_ncs_main_pub_cb.mds_use_count)
   {
      NCSMAINPUB_TRACE1_ARG1("\nMDS not yet started.... \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }
   
   if (!gl_ncs_main_pub_cb.dta_use_count)
   {
      NCSMAINPUB_TRACE1_ARG1("\nDTA not yet started.... \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   if (gl_ncs_main_pub_cb.oac_use_count > 0)
   {
      gl_ncs_main_pub_cb.oac_use_count++;
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_SUCCESS;
   }

   memset(&lib_create, 0, sizeof(lib_create));
   lib_create.i_op = NCS_LIB_REQ_CREATE;
   lib_create.info.create.argc = argc;
   lib_create.info.create.argv = argv;

   /* STEP : Initialize the OAA layer */
   if (oaclib_request(&lib_create) != NCSCC_RC_SUCCESS)
   {      
      NCSMAINPUB_TRACE1_ARG1("ERROR: OAC lib_req failed \n");
      m_NCS_AGENT_UNLOCK;
      return NCSCC_RC_FAILURE;
   }

   gl_ncs_main_pub_cb.oac_use_count = 1;

   /* All CORE services are started */
   gl_ncs_main_pub_cb.core_started = TRUE;

   m_NCS_AGENT_UNLOCK;

   return NCSCC_RC_SUCCESS;
}
 

/***************************************************************************\

  PROCEDURE    :    ncs_non_core_agents_startup

\***************************************************************************/
uns32 ncs_non_core_agents_startup(int argc, char *argv[])
{
   uns32 rc = NCSCC_RC_SUCCESS;

   rc = ncs_mbca_startup(argc, argv);
   if (rc != NCSCC_RC_SUCCESS)
      return rc;
   
   rc = ncs_ifa_startup(argc, argv);
   if (rc != NCSCC_RC_SUCCESS)
      return rc;

   rc = ncs_maa_startup(argc, argv);
   if (rc != NCSCC_RC_SUCCESS)
      return rc;

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\

  PROCEDURE    :    ncs_core_agents_startup

\***************************************************************************/
unsigned int ncs_core_agents_startup(int argc, char *argv[])
{
   if (gl_ncs_main_pub_cb.core_use_count)
   {      
      gl_ncs_main_pub_cb.core_use_count++;
      return NCSCC_RC_SUCCESS;
   }   

   if (ncs_leap_startup(argc, argv) != NCSCC_RC_SUCCESS)
   {      
      NCSMAINPUB_TRACE1_ARG1("ERROR: LEAP svcs startup failed \n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   if (ncs_mds_startup(argc, argv) != NCSCC_RC_SUCCESS)
   {      
      NCSMAINPUB_TRACE1_ARG1("ERROR: MDS startup failed \n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   if (ncs_dta_startup(argc, argv) != NCSCC_RC_SUCCESS)
   {      
      NCSMAINPUB_TRACE1_ARG1("ERROR: DTA startup failed \n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   if (ncs_oac_startup(argc, argv) != NCSCC_RC_SUCCESS)
   {      
      NCSMAINPUB_TRACE1_ARG1("ERROR: OAC startup failed \n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   gl_ncs_main_pub_cb.core_started = TRUE;
   gl_ncs_main_pub_cb.core_use_count = 1;

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\

  PROCEDURE    :    ncs_mbca_startup

\***************************************************************************/
unsigned int ncs_mbca_startup(int argc, char *argv[])
{
   NCS_LIB_REQ_INFO lib_create;

   if (!gl_ncs_main_pub_cb.core_started)
   {
      NCSMAINPUB_TRACE1_ARG1("\nNCS core not yet started.... \n");
      return NCSCC_RC_FAILURE;
   }

   memset(&lib_create, 0, sizeof(lib_create));
   lib_create.i_op = NCS_LIB_REQ_CREATE;
   lib_create.info.create.argc = argc;
   lib_create.info.create.argv = argv;

   if (gl_ncs_main_pub_cb.lib_hdl == NULL)
      return NCSCC_RC_SUCCESS; /* No agents to load */

   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.mbca.use_count > 0)
   {
       /* Already created, so just increment the use_count */
       gl_ncs_main_pub_cb.mbca.use_count++;
   }
   else /*** Init MBCA ***/   
   if ('n' != ncs_util_get_char_option(argc, argv, "MBCSV="))
   {
      gl_ncs_main_pub_cb.mbca.lib_req = (LIB_REQ)m_NCS_OS_DLIB_SYMBOL(gl_ncs_main_pub_cb.lib_hdl, "mbcsv_lib_req");
      if (gl_ncs_main_pub_cb.mbca.lib_req == NULL)
      {
         NCSMAINPUB_DBG_TRACE1_ARG1("\nMBCSV:MBCA:OFF");
      }
      else
      {
         if ((*gl_ncs_main_pub_cb.mbca.lib_req)(&lib_create) != NCSCC_RC_SUCCESS)
         {
            m_NCS_AGENT_UNLOCK;
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         }
         else
         {
            NCSMAINPUB_DBG_TRACE1_ARG1("\nMBCSV:MBCA:ON");
            gl_ncs_main_pub_cb.mbca.use_count = 1;
         }
      }      
   }

   m_NCS_AGENT_UNLOCK;

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\

  PROCEDURE    :    ncs_hisv_hpl_startup

\***************************************************************************/
unsigned int ncs_hisv_hpl_startup(int argc, char *argv[])
{
   NCS_LIB_REQ_INFO lib_create;

   if (!gl_ncs_main_pub_cb.core_started)
   {
      NCSMAINPUB_TRACE1_ARG1("\nNCS core not yet started.... \n");
      return NCSCC_RC_FAILURE;
   }

   memset(&lib_create, 0, sizeof(lib_create));
   lib_create.i_op = NCS_LIB_REQ_CREATE;
   lib_create.info.create.argc = argc;
   lib_create.info.create.argv = argv;

   if (gl_ncs_main_pub_cb.lib_hdl == NULL)
      return NCSCC_RC_SUCCESS; /* No agents to load */

   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.ncs_hpl.use_count > 0)
   {
      /* Already created, so just increment the use_count */
      gl_ncs_main_pub_cb.ncs_hpl.use_count++;
   }
   else  /*** Init HPL ***/
   if ('n' != ncs_util_get_char_option(argc, argv, "HISV="))
   {
      gl_ncs_main_pub_cb.ncs_hpl.lib_req = (LIB_REQ)m_NCS_OS_DLIB_SYMBOL(gl_ncs_main_pub_cb.lib_hdl, "ncs_hpl_lib_req");
      if (gl_ncs_main_pub_cb.ncs_hpl.lib_req == NULL)
      {
         NCSMAINPUB_DBG_TRACE1_ARG1("\nHISV:HPL:OFF");
      }
      else
      {
         if ((*gl_ncs_main_pub_cb.ncs_hpl.lib_req)(&lib_create) != NCSCC_RC_SUCCESS)
         {
            m_NCS_AGENT_UNLOCK;
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         }
         else
         {
            NCSMAINPUB_DBG_TRACE1_ARG1("\nHISV:HPL:ON");
            gl_ncs_main_pub_cb.ncs_hpl.use_count = 1;
         }
      }
   }

   m_NCS_AGENT_UNLOCK;

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\

  PROCEDURE    :    ncs_ifa_startup

\***************************************************************************/
unsigned int ncs_ifa_startup(int argc, char *argv[])
{
   NCS_LIB_REQ_INFO lib_create;

   if (!gl_ncs_main_pub_cb.core_started)
   {
      NCSMAINPUB_TRACE1_ARG1("\nNCS core not yet started.... \n");
      return NCSCC_RC_FAILURE;
   }

   memset(&lib_create, 0, sizeof(lib_create));
   lib_create.i_op = NCS_LIB_REQ_CREATE;
   lib_create.info.create.argc = argc;
   lib_create.info.create.argv = argv;

   if (gl_ncs_main_pub_cb.lib_hdl == NULL)
      return NCSCC_RC_SUCCESS; /* No agents to load */

   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.ifa.use_count > 0)
   {
      /* Already created, so just increment the use_count */
      gl_ncs_main_pub_cb.ifa.use_count++;
   }
   else  /*** Init IFA ***/
   if ('n' != ncs_util_get_char_option(argc, argv, "IFSV="))
   {      
      gl_ncs_main_pub_cb.ifa.lib_req = (LIB_REQ)m_NCS_OS_DLIB_SYMBOL(gl_ncs_main_pub_cb.lib_hdl, "ifa_lib_req");
      if (gl_ncs_main_pub_cb.ifa.lib_req == NULL)
      {
         NCSMAINPUB_DBG_TRACE1_ARG1("\nIFSV:IFA:OFF");
      }
      else
      {
         if ((*gl_ncs_main_pub_cb.ifa.lib_req)(&lib_create) != NCSCC_RC_SUCCESS)
         {
            m_NCS_AGENT_UNLOCK;
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         }
         else
         {
            NCSMAINPUB_DBG_TRACE1_ARG1("\nIFSV:IFA:ON");
            gl_ncs_main_pub_cb.ifa.use_count = 1;
         }
      }
   }

   m_NCS_AGENT_UNLOCK;

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\

  PROCEDURE    :    ncs_maa_startup

\***************************************************************************/
unsigned int ncs_maa_startup(int argc, char *argv[])
{
   NCS_LIB_REQ_INFO lib_create;

   if (!gl_ncs_main_pub_cb.core_started)
   {
      NCSMAINPUB_TRACE1_ARG1("\nNCS core not yet started.... \n");
      return NCSCC_RC_FAILURE;
   }

   memset(&lib_create, 0, sizeof(lib_create));
   lib_create.i_op = NCS_LIB_REQ_CREATE;
   lib_create.info.create.argc = argc;
   lib_create.info.create.argv = argv;

   if (gl_ncs_main_pub_cb.lib_hdl == NULL)
      return NCSCC_RC_SUCCESS; /* No agents to load */

   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.maa.use_count > 0)
   {
      /* Already created, so just increment the use_count */
      gl_ncs_main_pub_cb.maa.use_count++;
   }
   else       /*** Init MAA ***/
   if ('n' != ncs_util_get_char_option(argc, argv, "MASV="))
   {      
      gl_ncs_main_pub_cb.maa.lib_req = (LIB_REQ)m_NCS_OS_DLIB_SYMBOL(gl_ncs_main_pub_cb.lib_hdl, "maclib_request");
      if (gl_ncs_main_pub_cb.maa.lib_req == NULL)
      {
         NCSMAINPUB_DBG_TRACE1_ARG1("\nMASV:MAA:OFF");
      }
      else
      {
         if ((*gl_ncs_main_pub_cb.maa.lib_req)(&lib_create) != NCSCC_RC_SUCCESS)
         {
            m_NCS_AGENT_UNLOCK;
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         }
         else
         {
            NCSMAINPUB_DBG_TRACE1_ARG1("\nMASV:MAA:ON");
            gl_ncs_main_pub_cb.maa.use_count = 1;
         }
      }
   }

   m_NCS_AGENT_UNLOCK;

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\

  PROCEDURE    :    ncs_ifa_shutdown

\***************************************************************************/
unsigned int ncs_ifa_shutdown(void)
{
   NCS_LIB_REQ_INFO  lib_destroy;
   uns32             rc = NCSCC_RC_SUCCESS;

   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.ifa.use_count > 1)
   {
      /* Still users extis, so just decrement the use_count */
      gl_ncs_main_pub_cb.ifa.use_count--;
      m_NCS_AGENT_UNLOCK;
      return rc;
   }

   memset(&lib_destroy, 0, sizeof(lib_destroy));
   lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
   lib_destroy.info.destroy.dummy = 0;;

   if (gl_ncs_main_pub_cb.ifa.lib_req != NULL)
      rc = (*gl_ncs_main_pub_cb.ifa.lib_req)(&lib_destroy);

   gl_ncs_main_pub_cb.ifa.use_count = 0;
   gl_ncs_main_pub_cb.ifa.lib_req = NULL;

   m_NCS_AGENT_UNLOCK;

   return rc;
}


/***************************************************************************\

  PROCEDURE    :    ncs_hisv_hpl_shutdown

\***************************************************************************/
unsigned int ncs_hisv_hpl_shutdown(void)
{
   NCS_LIB_REQ_INFO  lib_destroy;
   uns32             rc = NCSCC_RC_SUCCESS;

   m_NCS_AGENT_LOCK;
   if (gl_ncs_main_pub_cb.ncs_hpl.use_count > 1)
   {
      /* Still users extis, so just decrement the use_count */
      gl_ncs_main_pub_cb.ncs_hpl.use_count--;
      m_NCS_AGENT_UNLOCK;
      return rc;
   }

   memset(&lib_destroy, 0, sizeof(lib_destroy));
   lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
   lib_destroy.info.destroy.dummy = 0;;

   if (gl_ncs_main_pub_cb.ncs_hpl.lib_req != NULL)
      rc = (*gl_ncs_main_pub_cb.ncs_hpl.lib_req)(&lib_destroy);

   gl_ncs_main_pub_cb.ncs_hpl.use_count = 0;
   gl_ncs_main_pub_cb.ncs_hpl.lib_req = NULL;

   m_NCS_AGENT_UNLOCK;

   return rc;
}


/***************************************************************************\

  PROCEDURE    :    ncs_mbca_shutdown

\***************************************************************************/
unsigned int ncs_mbca_shutdown(void)
{
   NCS_LIB_REQ_INFO  lib_destroy;
   uns32             rc = NCSCC_RC_SUCCESS;

   m_NCS_AGENT_LOCK;
   if (gl_ncs_main_pub_cb.mbca.use_count > 1)
   {
      /* Still users extis, so just decrement the use_count */
      gl_ncs_main_pub_cb.mbca.use_count--;
      m_NCS_AGENT_UNLOCK;
      return rc;
   }

   memset(&lib_destroy, 0, sizeof(lib_destroy));
   lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
   lib_destroy.info.destroy.dummy = 0;;

   if (gl_ncs_main_pub_cb.mbca.lib_req != NULL)
      rc = (*gl_ncs_main_pub_cb.mbca.lib_req)(&lib_destroy);

   gl_ncs_main_pub_cb.mbca.use_count = 0;
   gl_ncs_main_pub_cb.mbca.lib_req = NULL;

   m_NCS_AGENT_UNLOCK;

   return rc;
}


/***************************************************************************\

  PROCEDURE    :    ncs_maa_shutdown

\***************************************************************************/
unsigned int ncs_maa_shutdown(void)
{
   NCS_LIB_REQ_INFO  lib_destroy;
   uns32             rc = NCSCC_RC_SUCCESS;

   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.maa.use_count > 1)
   {
      /* Still users extis, so just decrement the use_count */
      gl_ncs_main_pub_cb.maa.use_count--;
      m_NCS_AGENT_UNLOCK;
      return rc;
   }

   memset(&lib_destroy, 0, sizeof(lib_destroy));
   lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
   lib_destroy.info.destroy.dummy = 0;;

   if (gl_ncs_main_pub_cb.maa.lib_req != NULL)
      rc = (*gl_ncs_main_pub_cb.maa.lib_req)(&lib_destroy);

   gl_ncs_main_pub_cb.maa.use_count = 0;
   gl_ncs_main_pub_cb.maa.lib_req = NULL;
   
   m_NCS_AGENT_UNLOCK;

   return rc;
}


/***************************************************************************\

  PROCEDURE    :    ncs_leap_shutdown

\***************************************************************************/
void ncs_leap_shutdown()
{
   NCS_LIB_REQ_INFO lib_destroy;
 
   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.leap_use_count > 1)
   {
      /* Still users extis, so just decrement the use_count */
      gl_ncs_main_pub_cb.leap_use_count--;
      m_NCS_AGENT_UNLOCK;
      return;
   }


   memset(&lib_destroy, 0, sizeof(lib_destroy));
   lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
   lib_destroy.info.destroy.dummy = 0;

#ifdef __NCSINC_WIN32__
   m_NCS_OS_DLIB_CLOSE(gl_ncs_main_pub_cb.lib_hdl);
#else
   m_NCS_OS_DLIB_CLOSE(gl_ncs_main_pub_cb.lib_hdl);
#endif
   gl_ncs_main_pub_cb.lib_hdl = NULL;


   sprr_lib_req(&lib_destroy);
   
   leap_env_destroy();
   
   gl_ncs_main_pub_cb.leap_use_count = 0;
   gl_ncs_main_pub_cb.core_started = FALSE;

   m_NCS_AGENT_UNLOCK;

   return;
}


/***************************************************************************\

  PROCEDURE    :    ncs_mds_shutdown

\***************************************************************************/
void ncs_mds_shutdown()
{
   NCS_LIB_REQ_INFO lib_destroy;
   uns32 tmp_ctr;

   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.mds_use_count > 1)
   {
      /* Still users extis, so just decrement the use_count */
      gl_ncs_main_pub_cb.mds_use_count--;
      m_NCS_AGENT_UNLOCK;
      return;
   }
   

   memset(&lib_destroy, 0, sizeof(lib_destroy));
   lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
   lib_destroy.info.destroy.dummy = 0;

   mda_lib_req(&lib_destroy);
   mds_lib_req(&lib_destroy);
   
   gl_ncs_main_pub_cb.mds_use_count = 0;
   gl_ncs_main_pub_cb.core_started = FALSE;

   for (tmp_ctr=0;tmp_ctr<gl_pargc;tmp_ctr++)
       free(gl_pargv[tmp_ctr]);
   gl_pargc = 0;

   m_NCS_AGENT_UNLOCK;

   return;
}


/***************************************************************************\

  PROCEDURE    :    ncs_dta_shutdown

\***************************************************************************/
void ncs_dta_shutdown()
{
   NCS_LIB_REQ_INFO lib_destroy;

   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.dta_use_count > 1)
   {
      /* Still users extis, so just decrement the use_count */
      gl_ncs_main_pub_cb.dta_use_count--;
      m_NCS_AGENT_UNLOCK;
      return;
   }


   memset(&lib_destroy, 0, sizeof(lib_destroy));
   lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
   lib_destroy.info.destroy.dummy = 0;

   dta_lib_req(&lib_destroy);
   
   gl_ncs_main_pub_cb.dta_use_count = 0;
   gl_ncs_main_pub_cb.core_started = FALSE;

   m_NCS_AGENT_UNLOCK;

   return;
}


/***************************************************************************\

  PROCEDURE    :    ncs_oac_shutdown

\***************************************************************************/
void ncs_oac_shutdown()
{
   NCS_SPIR_REQ_INFO spir_req;
   NCS_LIB_REQ_INFO  lib_destroy;
   int               rc;

   m_NCS_AGENT_LOCK;

   if (gl_ncs_main_pub_cb.oac_use_count > 1)
   {
      /* Still users extis, so just decrement the use_count */
      gl_ncs_main_pub_cb.oac_use_count--;
      m_NCS_AGENT_UNLOCK;
      return;
   }

   /* STEP: Check if any "OAC" has been created on ADEST-PWE1. This is
            required because the NCSADA_GET_HDLS does not have a 
            corresponding NCSADA_REL_HDLS API. Hence, any OAC on the
            pre-created ADEST-PWE1 has to be explicitly destroyed. 

            A similar thing has to be done when a PWE on VDEST or ADEST
            is destroyed; It needs to be checked whether any OAC has
            been created, and if yes, then it has to be destroyed. Please 
            see NCSVDA_PWE_DESTROY/NCSADA_PWE_DESTROY code for this.
   */
   memset(&spir_req, 0, sizeof(spir_req));
   spir_req.type = NCS_SPIR_REQ_LOOKUP_INST;
   spir_req.i_environment_id = 1;
   spir_req.i_sp_abstract_name = m_OAA_SP_ABST_NAME;
   spir_req.i_instance_name = m_MDS_SPIR_ADEST_NAME;
   if (ncs_spir_api(&spir_req) == NCSCC_RC_SUCCESS)
   {
        /* PWE1-OAC has been created. So destroy it */
        memset(&spir_req, 0, sizeof(spir_req));
        spir_req.type = NCS_SPIR_REQ_REL_INST;
        spir_req.i_environment_id = 1;
        spir_req.i_sp_abstract_name = m_OAA_SP_ABST_NAME;
        spir_req.i_instance_name = m_MDS_SPIR_ADEST_NAME;
            
        if (ncs_spir_api(&spir_req) != NCSCC_RC_SUCCESS)
        {
           rc = m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
           return;
        }
    }

   /* STEP: Invoke OAC library shutdown */         
   memset(&lib_destroy, 0, sizeof(lib_destroy));
   lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
   lib_destroy.info.destroy.dummy = 0;

   oaclib_request(&lib_destroy);
   
   gl_ncs_main_pub_cb.oac_use_count = 0;
   gl_ncs_main_pub_cb.core_started = FALSE;

   m_NCS_AGENT_UNLOCK;

   return;
}


/***************************************************************************\

  PROCEDURE    :    ncs_core_agents_shutdown

\***************************************************************************/
unsigned int ncs_core_agents_shutdown()
{
   if (!gl_ncs_main_pub_cb.core_use_count)
   {
      NCSMAINPUB_TRACE1_ARG1("\nNCS core not yet started.... \n");
      return NCSCC_RC_FAILURE;
   }

   if (gl_ncs_main_pub_cb.core_use_count > 1)
   {
      /* Decrement the use count */
      gl_ncs_main_pub_cb.core_use_count--;
      return NCSCC_RC_SUCCESS;
   }

   /*** Shutdown basic services ***/
   ncs_oac_shutdown();
   /*usleep(1000); */ 
   ncs_dta_shutdown();
   ncs_mds_shutdown();
   ncs_leap_shutdown();
   gl_ncs_main_pub_cb.core_use_count = 0;
   
   return (NCSCC_RC_SUCCESS);
}


/***************************************************************************\

  PROCEDURE    :    ncs_get_node_id

\***************************************************************************/
NCS_NODE_ID ncs_get_node_id(void)
{
   if (!gl_ncs_main_pub_cb.core_started)
   {
      return m_LEAP_DBG_SINK(0);
   }

   return gl_ncs_main_pub_cb.my_nodeid;
}




uns32 file_get_word(FILE **fp, char *o_chword)
{
   int temp_char;
   unsigned int temp_ctr=0;
try_again:
   temp_ctr = 0;
   temp_char = getc(*fp);
   while ((temp_char != EOF) && (temp_char != '\n') && (temp_char != ' ') && (temp_char != '\0'))
   {
      o_chword[temp_ctr] = (char)temp_char;
      temp_char = getc(*fp);
      temp_ctr++;
   }
   o_chword[temp_ctr] = '\0';
   if (temp_char == EOF)
   {
      return(NCS_MAIN_EOF);
   }
   if (temp_char == '\n')
   {
      return(NCS_MAIN_ENTER_CHAR);
   }
   if(o_chword[0] == 0x0)
      goto try_again;
   return(0);
}

uns32 file_get_string(FILE **fp, char *o_chword)
{
   int temp_char;
   unsigned int temp_ctr=0;
try_again:
   temp_ctr = 0;
   temp_char = getc(*fp);
   while ((temp_char != EOF) && (temp_char != '\n') && (temp_char != '\0'))
   {
      o_chword[temp_ctr] = (char)temp_char;
      temp_char = getc(*fp);
      temp_ctr++;
   }
   o_chword[temp_ctr] = '\0';
   if (temp_char == EOF)
   {
      return(NCS_MAIN_EOF);
   }
   if (temp_char == '\n')
   {
      return(NCS_MAIN_ENTER_CHAR);
   }
   if(o_chword[0] == 0x0)
      goto try_again;
   return(0);
}



uns32 mainget_node_id(uns32 *node_id)
{
   FILE *fp;
   char get_word[256];
   uns32 res = NCSCC_RC_SUCCESS;
   uns32 d_len, f_len;
   char *tmp;

#ifdef __NCSINC_LINUX__
#if (MDS_MULTI_HUB_PER_OS_INSTANCE == 1)
   tmp = getenv("NCS_SIM_NODE_ID");
   if (tmp != NULL)
   {
       m_NCS_DBG_PRINTF("\nNCS: Reading node_id(%s) from environment var.\n",tmp);
       *node_id = atoi(tmp);
       return NCSCC_RC_SUCCESS;
   }
#else
   USE(tmp);
#endif
   d_len = strlen(ncs_config_root);
   f_len = strlen("/node_id");
   if ( (d_len + f_len) >= MAX_NCS_CONFIG_FILEPATH_LEN)
   {
       printf("\n Filename too long \n");
      return NCSCC_RC_FAILURE;
   }
   /* Hack ncs_config_root to construct path */
   sprintf(ncs_config_root + d_len, "%s", "/node_id");


   /* LSB changes. Pick nodeid from OSAF_LOCALSTATEDIR */

   fp = fopen(NODE_ID_FILE,"r");

   /* Reverse hack ncs_config_root to original value*/
   ncs_config_root[d_len] = 0;
#else
   fp = fopen("c:\\ncs\\node_id","r");
#endif
   if (fp == NULL)
   {
      res = NCSCC_RC_FAILURE;
   } else
   {
      do
      {   
         file_get_word(&fp,get_word);
          
         if (sscanf((const char *)&get_word,"%x",node_id)!= 1)
         { 
             res = NCSCC_RC_FAILURE;
             break;
         }     
         fclose(fp);

      } while(0);
   }

   return (res);
}


/* Fetchs the chassis type string */
uns32
ncs_get_chassis_type(uns32 i_max_len , char *o_chassis_type)
{
   FILE *fp;
   uns32 res = NCSCC_RC_SUCCESS;
   uns32 d_len, f_len;
   uns32 file_size = 0;
   char temp_ncs_config_root[MAX_NCS_CONFIG_FILEPATH_LEN + 1];

   if((res = ncs_set_config_root()) != NCSCC_RC_SUCCESS) 
      return NCSCC_RC_FAILURE;

   memset(temp_ncs_config_root,0,sizeof(temp_ncs_config_root));

   strncpy(temp_ncs_config_root,ncs_config_root,sizeof(temp_ncs_config_root)-1);
  
   if(i_max_len > NCS_MAX_CHASSIS_TYPE_LEN)
       return NCSCC_RC_FAILURE;

   d_len = strlen(temp_ncs_config_root);

   f_len = strlen("/chassis_type");
   if ( (d_len + f_len) >= MAX_NCS_CONFIG_FILEPATH_LEN)
   {
      printf("\n Filename too long \n");
      return NCSCC_RC_FAILURE;
   }

   /* Hack ncs_config_root to construct path */
   sprintf(temp_ncs_config_root + d_len, "%s", "/chassis_type");
   fp = fopen(temp_ncs_config_root,"r");
   if (fp == NULL)
   {
      printf("\nNCS: Couldn't open %s/chassis_type \n", temp_ncs_config_root);
      return NCSCC_RC_FAILURE;
   }

   /* positions the file pointer to the end of the file */
   if(0 != fseek(fp,0L,SEEK_END))
   {
      printf("fseek call failed with errno  %d \n",errno);
      fclose(fp);
      return NCSCC_RC_FAILURE;
   }
   
   /* gets the file pointer offset from the start of the file */
   file_size  = ftell(fp);
   if(file_size == -1)
   { 
        printf("ftell call failed with errno %d \n",errno);
        fclose(fp);
        return NCSCC_RC_FAILURE;
   }

   /* validating the file size */
   if((file_size > NCS_MAX_CHASSIS_TYPE_LEN +1)||(file_size > i_max_len + 1) ||(file_size == 0)) 
   {
     printf("Some thing wrong with chassis_type file \n");
     fclose(fp);
     return NCSCC_RC_FAILURE;
   }

   /* positions the file pointer to the end of the file */
   if(0 != fseek(fp,0L,SEEK_SET))
   {
      printf("fseek call failed with errno  %d \n",errno);
      fclose(fp);
      return NCSCC_RC_FAILURE;
   }

   do
   {
      /* reads the chassis type string from the file and copies into the user provided buffer*/ 
      file_get_string(&fp,o_chassis_type);

      fclose(fp);

   } while(0);

   return(res);
}

static uns32 ncs_set_config_root(void)
{
   char *tmp;  
   static NCS_BOOL config_root_init =FALSE;
  
   if(config_root_init == TRUE)
     return NCSCC_RC_SUCCESS;
   else
      config_root_init = TRUE;

   tmp = getenv("NCS_SIMULATION_CONFIG_ROOTDIR");
   if (tmp != NULL)
   {
      if (strlen (tmp) >= MAX_NCS_CONFIG_ROOTDIR_LEN)
      {
          /* m_NCS_NID_NOTIFY(NID_NCS_CFG_DIR_ROOTNAME_LEN_EXCEED); */
             printf("Config directory root name too long\n");
             return NCSCC_RC_FAILURE;
      }
     sprintf(ncs_config_root, "%s", tmp);
   
     m_NCS_DBG_PRINTF("\nNCS: Using %s as config directory root\n", ncs_config_root);
   }
   else
   {
      sprintf(ncs_config_root, "%s", NCS_DEF_CONFIG_FILEPATH);
   }

   return NCSCC_RC_SUCCESS;
}

uns32 ncs_util_get_sys_params(NCS_SYS_PARAMS *sys_params)
{
   char  *tmp_ptr;
   uns32 res = NCSCC_RC_SUCCESS;

   memset(sys_params, 0, sizeof(NCS_SYS_PARAMS));
   
   if((res = ncs_set_config_root())!= NCSCC_RC_SUCCESS)
   {
      printf("Unable to set config root \n");
      return NCSCC_RC_FAILURE;
   }


   if(mainget_node_id(&sys_params->node_id)!= NCSCC_RC_SUCCESS)
   {
      /* m_NCS_NID_NOTIFY(NID_NCS_GET_NODE_ID_FAILED); */
       printf("Not able to get the NODE ID\n");
      return(NCSCC_RC_FAILURE);
   }

      
   if ((tmp_ptr = getenv("NCS_PCON_ID")) != NULL)
   {
      sys_params->pcon_id = atoi(tmp_ptr);
   }
   else
   {
      sys_params->pcon_id = NCS_MAIN_DEF_PCON_ID ;
   }

   return NCSCC_RC_SUCCESS;
}


void ncs_get_sys_params_arg(int i_argc,
                            char *i_argv[], 
                            NCS_SYS_PARAMS *sys_params)
{
   char   *p_field;
   uns32  tmp_ctr;
   uns32  orig_argc;
   NCS_SUB_SLOT_ID   sub_slot_id = 0;
   NCS_SYS_PARAMS params;

 
   orig_argc = gl_pargc;
   for (tmp_ctr=0; tmp_ctr<NCS_MAX_INPUT_ARG_DEF; tmp_ctr++)
   {
     gl_pargv[(gl_pargc) + tmp_ctr] = (char*)malloc(NCS_MAX_STR_INPUT);
     memset(gl_pargv[(gl_pargc) + tmp_ctr],0,NCS_MAX_STR_INPUT);
   }
   gl_pargc += tmp_ctr;

   /* Check   argv[argc-1] through argv[1] */
   for ( ; i_argc > 1; i_argc -- )
   {
      p_field = strstr(i_argv[i_argc-1], "NODE_ID=");
      if (p_field != NULL)
      {
         if (sscanf(p_field + strlen("NODE_ID="), "%d", &params.node_id) == 1)
            sys_params->node_id = params.node_id;

         continue;
      }
      else
      p_field = strstr(i_argv[i_argc-1], "PCON_ID=");
      if (p_field != NULL)
      {
         if (sscanf(p_field + strlen("PCON_ID="), "%d", &params.pcon_id) == 1)
            sys_params->pcon_id = params.pcon_id;

         continue;
      }
      else      
      {
         /* else store whatever comes */
         gl_pargv[gl_pargc] = (char*)malloc(NCS_MAX_STR_INPUT);
         memset(gl_pargv[gl_pargc],0,NCS_MAX_STR_INPUT);
         strcpy(gl_pargv[gl_pargc], i_argv[i_argc-1]);
         gl_pargc = (gl_pargc) + 1;
      }
   }
   
   if ( getenv("NCS_ENV_NODE_ID") )
       sys_params->node_id = atoi(getenv("NCS_ENV_NODE_ID"));
   
   m_NCS_DBG_PRINTF("NCS:NODE_ID=0x%08X\n", sys_params->node_id);

   gl_ncs_main_pub_cb.my_nodeid = sys_params->node_id;

   if(m_NCS_GET_PHYINFO_FROM_NODE_ID(sys_params->node_id,&sys_params->shelf_id,
                              &sys_params->slot_id,&sub_slot_id)!= NCSCC_RC_SUCCESS)
   {
      
        m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
        return ; 
   }

   sprintf(gl_pargv[orig_argc + 0],"NONE");
   sprintf(gl_pargv[orig_argc + 1],"CLUSTER_ID=%d",sys_params->cluster_id);
   sprintf(gl_pargv[orig_argc + 2],"SHELF_ID=%d",sys_params->shelf_id);
   sprintf(gl_pargv[orig_argc + 3],"SLOT_ID=%d",sys_params->slot_id);
   sprintf(gl_pargv[orig_argc + 4],"NODE_ID=%d",sys_params->node_id);
   sprintf(gl_pargv[orig_argc + 5],"PCON_ID=%d",sys_params->pcon_id);
      
   return;
}

uns32 ncs_update_sys_param_args(int argc, char *argv[]) 
{
   NCS_SYS_PARAMS sys_params;

   /* Get the system specific parameters */
   ncs_util_get_sys_params(&sys_params);

   /* Frame input arguments */
   ncs_get_sys_params_arg(argc, argv, &sys_params);
   
   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\

  PROCEDURE    :    ncs_util_get_char_option

\***************************************************************************/
char ncs_util_get_char_option(int argc, char *argv[], char *arg_prefix)
{
   char char_option;
   char                 *p_field;

   p_field = ncs_util_search_argv_list(argc, argv, arg_prefix);
   if (p_field == NULL)
   {
      return 0;
   }
   if (sscanf(p_field + strlen(arg_prefix), "%c", &char_option) != 1)
   {
      return 0;
   }
   if (isupper(char_option))
      char_option = (char)tolower(char_option);

   return char_option;
}
/***************************************************************************\

  PROCEDURE    :    ncs_util_search_argv_list

\***************************************************************************/
char *ncs_util_search_argv_list(int argc, char *argv[], char *arg_prefix)
{
   char *tmp;

   /* Check   argv[argc-1] through argv[1] */
   for ( ; argc > 1; argc -- )
   {
      tmp = strstr(argv[argc-1], arg_prefix);
      if (tmp != NULL)
         return tmp;
   }
   return NULL;
}

/*****************************************************************************

  PROCEDURE     :    ncs_main_set_log_dir

  DESCRIPTION:       Function used for setting the global variable contain
                     log path.

                     Global variable name = "gl_ncs_log_dir"


  RETURNS:           SUCCESS - All went well.
                     FAILURE - Something went wrong.

*****************************************************************************/
static uns32 ncs_main_set_log_dir()
{
#ifdef __NCSINC_LINUX__
   {
       char * env_var;
       env_var = getenv("NCS_LOG_PATH");

       if(env_var)
       {
           strcpy(gl_ncs_log_dir, env_var);
       }
       else
       {
           strcpy(gl_ncs_log_dir, LOG_PATH);
       }

       if (ncs_main_create_log_dir(gl_ncs_log_dir) != NCSCC_RC_SUCCESS)
       {
           return NCSCC_RC_FAILURE;
       }
   }
#else
   strcpy(gl_ncs_log_dir, LOG_PATH);
#endif
   return NCSCC_RC_SUCCESS;
}

#ifdef __NCSINC_LINUX__
/*****************************************************************************

  PROCEDURE NAME:    ncs_main_create_log_dir

  DESCRIPTION:       Function used for creating the log directory.

  RETURNS:           SUCCESS - All went well.
                     FAILURE - Something went wrong.

*****************************************************************************/
static uns32 ncs_main_create_log_dir(char *path)
{
    uns32 retval = NCSCC_RC_SUCCESS;
    uns32 i= 0, len = 0;
    char *tmp, tchar[200];

    if (NCS_LOG_DIR_NAME_MAXLEN < strlen(path))
    {
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }
    if (*path != '/')
    {
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }

    len = strlen(path);

    if (path[len-1] != '/')
    {
        path[len] = '/';
        path[len+1] = '\0';
    }

    tmp = path;

    while(*tmp != '\0')
    {
        tchar[i] = *tmp;
        tmp++;
        i++;

        if ((*tmp == '/') || (*tmp == '\0'))
        {
            tchar[i] = '\0';
            if (MKDIR(tchar, 0x3ED) == -1)
            {
               if (errno != EEXIST)
               {
                   perror("Failed to create log directory:");
                   retval = NCSCC_RC_FAILURE;
                   break;
               }
            }
            tchar[i] = '/';
        }
    }
    NCSMAINPUB_DBG_TRACE1_ARG2("\n My log directory path = %s \n", path);
    return retval;
}
#endif

/****************************************************************************
  Name          :  ncs_get_node_id_from_phyinfo

  Description   :  This function combines  chassis id ,physical 
                   slot id and  sub slot id into node_id

  Arguments     :  i_chassis_id  - chassis id 
                   i_phy_slot_id - physical slot id
                   i_sub_slot_id - slot id 
                   *o_node_id - node_id 

  Return Values :  On Failure NCSCC_RC_FAILURE
                   On Success NCSCC_RC_SUCCESS

  Notes         :  None.
******************************************************************************/
uns8 ncs_get_node_id_from_phyinfo( NCS_CHASSIS_ID i_chassis_id, NCS_PHY_SLOT_ID i_phy_slot_id, 
                                                 NCS_SUB_SLOT_ID i_sub_slot_id , NCS_NODE_ID *o_node_id)
{
    if ( o_node_id == NULL)
       return NCSCC_RC_FAILURE;

    *o_node_id = ((NCS_CHASSIS_ID)i_chassis_id << 16) |
                 ((NCS_NODE_ID)i_phy_slot_id << 8) |
                 (NCS_NODE_ID)i_sub_slot_id ;
                                  
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncs_get_phyinfo_from_node_id

  Description   :  This function extracts chassis id,physical slot
                    id and sub slot id from node_id

  Arguments     :  i_node_id  -    node id
                   *o_chassis_id  - chassis id
                   *o_phy_slot_id - physical slot id
                   *o_sub_slot_id - slot id

  Return Values :  On Failure NCSCC_RC_FAILURE
                   On Success NCSCC_RC_SUCCESS

  Notes         :  None.
******************************************************************************/
uns8 ncs_get_phyinfo_from_node_id( NCS_NODE_ID i_node_id , NCS_CHASSIS_ID *o_chassis_id, 
                                               NCS_PHY_SLOT_ID  *o_phy_slot_id, NCS_SUB_SLOT_ID *o_sub_slot_id)
{
   if(o_sub_slot_id!=NULL)
      *o_sub_slot_id =  ((NCS_SUB_SLOT_ID) i_node_id);

   if(o_chassis_id!=NULL)
      *o_chassis_id  =  (NCS_CHASSIS_ID)(i_node_id >>16);
   
   if(o_phy_slot_id!=NULL)
      *o_phy_slot_id =  (NCS_PHY_SLOT_ID)(i_node_id >>8);

   return NCSCC_RC_SUCCESS;
}
        
