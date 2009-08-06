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

  DESCRIPTION:  This file declares the main entry point into NCS. 

******************************************************************************
*/
#ifndef NCS_MAIN_PVT_H
#define NCS_MAIN_PVT_H

#include "mds_papi.h"

#define NCS_MAIN_EOF              (3)
#define NCS_MAIN_ENTER_CHAR       (2)
#define NCS_MAIN_DEF_CLUSTER_ID   (1)
#define NCS_MAIN_DEF_PCON_ID      (0)
#define NCS_MAIN_MAX_INPUT       (128)
#define NCS_MAX_INPUT_ARG_DEF     (7)
#define NCS_MAX_STR_INPUT       (255)

#define MAX_NCS_CONFIG_ROOTDIR_LEN 200
#define MAX_NCS_CONFIG_FILENAME_LEN 30
#define MAX_NCS_CONFIG_FILEPATH_LEN \
   (MAX_NCS_CONFIG_ROOTDIR_LEN + MAX_NCS_CONFIG_FILENAME_LEN)

#define NCS_DEF_CONFIG_FILEPATH  OSAF_SYSCONFDIR

typedef struct ncs_sys_params {
	NCS_PHY_SLOT_ID slot_id;
	NCS_CHASSIS_ID shelf_id;
	NCS_NODE_ID node_id;
	uns32 cluster_id;
	uns32 pcon_id;
} NCS_SYS_PARAMS;

/***********************************************************************\
   
   ncspvt_load_config_n_startup: 

\***********************************************************************/
int ncspvt_load_config_n_startup(int argc, char *argv[]);

/***********************************************************************\
   
   ncspvt_svcs_startup: This function first starts up all agents available
                     in a process by invoking "ncs_agents_startup()" and
                     then invokes any Director, Node-Director or Server
                     enabled using compile time flags.

                     This function is not a public API. This function
                     is expected to invoked by "main()". But for inhouse
                     testing purposes, this function is available for use.
                     Thus this function could be called by one of

                     1)  A real "main()"
                     2)  From Powercode
                     3)  From Tetware main.

\***********************************************************************/
uns32 ncspvt_svcs_startup(int argc, char *argv[], FILE *fp);

/***********************************************************************\
   
   ncspvt_svcs_shutdown: Opposite of ncspvt_svcs_startup()
\***********************************************************************/
uns32 ncspvt_svcs_shutdown(int argc, char *argv[]);

/***********************************************************************\
****   
****     Utility APIs defined in  ncs_main_pub.c
****
\***********************************************************************/
char ncs_util_get_char_option(int argc, char *argv[], char *arg_prefix);

char *ncs_util_search_argv_list(int argc, char *argv[], char *arg_prefix);

uns32 file_get_word(FILE **fp, char *o_chword);
uns32 file_get_string(FILE **fp, char *o_chword);
EXTERN_C char *gl_pargv[NCS_MAIN_MAX_INPUT];
EXTERN_C uns32 gl_pargc;

/***********************************************************************\
   m_NCS_NID_NOTIFY: This function notifies NID about the occurance of 
                     an error while instantiating NCS pre-AMF spawned 
                     process. Ex: MDS, MASv, DTSv, PSSv, SCAP, PCAP.. 

                     Only the macro should be used, the function 
                     prototyped below should not be used. 

\***********************************************************************/
uns32 ncs_nid_notify(uns16 status);
#define m_NCS_NID_NOTIFY(status)  ncs_nid_notify(status)
#include "nid_api.h"

#endif
