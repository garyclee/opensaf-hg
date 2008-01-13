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

..............................................................................

  DESCRIPTION:

  This module is the main include file for Availability Agent (AvA).
  
******************************************************************************
*/

#ifndef EDA_H
#define EDA_H

/*  Get compile time options */
#include "ncs_opt.h"

/* Get general definitions */
#include "ncsgl_defs.h"

/* Get target's suite of header files...*/
#include "t_suite.h"

/* From /base/common/inc */
#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncsft_rms.h"
#include "ncs_ubaid.h"
#include "ncsencdec.h"
#include "ncs_stack.h"
#include "ncs_mib.h"
#include "ncs_mtbl.h"
#include "ncs_log.h"
#include "ncs_lib.h"
#include "ncs_dummy.h"

/* From targsvcs/common/inc */
#include "mds_papi.h"
#if 0
#include "sysf_mds.h" 

#if (NCS_MDS == 1)
#include "mds_inc.h"
#endif
#endif

/* From /base/products/rms/inc */
#if (NCS_RMS == 1)
#include "rms_env.h"
#endif

#include "mds_papi.h"

/* DTS header file */
#if (NCS_DTS == 1)
#include "dts_papi.h"
#endif

/* DTA header file */
#if (NCS_DTA == 1)
#include "dta_papi.h"
#endif

/* EDA specific inc Files */
#include "edsv_msg.h"

#include "eda_cb.h"
#include "eda_hdl.h"
#include "eda_mds.h"
#include "eda_mem.h"
#include "eda_log.h"
#include "edsv_logstr.h"
#include "edsv_mem.h"

/* EDSV specific include files */
#include "edsv_defs.h"
#include "edsv_util.h"

/* EDA CB global handle declaration */
EXTERN_C uns32 gl_eda_hdl;

/* EDA Default MDS timeout value */
#define EDA_MDS_DEF_TIMEOUT 100

/* EDSv maximum length of data of an event(in bytes) */
#define SA_EVENT_DATA_MAX_SIZE 4096

/* EDSv maximum length of a pattern size */
#define EDSV_MAX_PATTERN_SIZE 255

/*Length of a default publisher name */
#define EDSV_DEF_NAME_LEN 4

/*Max limit for reserved events (from 0 - 1000)*/  
#define MAX_RESERVED_EVENTID 1000  

#endif /* !EDA_H */
