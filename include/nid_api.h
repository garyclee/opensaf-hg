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

  $Header:

  ..............................................................................

  DESCRIPTION:
  This file containts the structures and prototype declarations usesful for serv  ices to interface with nodeinitd. These definitions are used both by nodeini  td and blladeinit API.

******************************************************************************
*/

#ifndef NID_API_H
#define NID_API_H

/*#include "ncs_opt.h"*/
#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncs_scktprm.h"
#include "ncssysf_tsk.h"

#include "nid_err.h"

/* API Error Codes */
#define NID_INV_PARAM    11
#define NID_OFIFO_ERR    22
#define NID_WFIFO_ERR    33

#define NID_MAGIC       0xAAB49DAA

#define NID_MAX_STAT_CODS    100

#define NODE_HA_STATE  OSAF_LOCALSTATEDIR "node_ha_state"


/****************************************************************
 *       Service and Status Codes                                *
 ****************************************************************/
/* vivek_nid */
#define NID_IPMC_VER_SAME 2
#define NID_PHOENIXBIOS_VER_SAME  2
typedef enum nid_svc_code{
   NID_SERVERR = -1,
   NID_HPM = 0,
   NID_IPMCUP,
   NID_BIOSUP,
   NID_HLFM,
   NID_RDF, /* OpenSAF RDF */
   NID_OPENHPI,
   NID_XND,
   NID_LHCD,
   NID_LHCR,
   NID_NWNG,
   NID_DRBD,
   NID_TIPC,
   NID_IFSVDD,
   NID_SNMPD,
   NID_NCS,
   NID_DTSV,
   NID_MASV,
   NID_PSSV,
   NID_GLND,
   NID_EDSV,
   NID_SUBAGT, /* 61409 */
   NID_SCAP,
   NID_PCAP, /* Any new NCS services should be inserted btw NID_NCS and NID_PCAP*/
   NID_MAXSERV
} NID_SVC_CODE;


/****************************************************************
 *       Message format used by nodeinitd and spawned services  *
 *       for communicating initialization status.                *
 ****************************************************************/
typedef struct nid_fifo_msg {
   uns32 nid_magic_no;   /* Magic number */
   uns32 nid_serv_code;  /* Identifies the spawned service uniquely*/
   uns32 nid_stat_code;  /* Identifies the initialization status */
}NID_FIFO_MSG;

/*****************************************************************
 *       Structure to Map the service and status codes to         *
 *       corresponding descriptive message                        *
 *****************************************************************/
struct nid_stat_desc{
   uns8 * nid_stat_str; /* String representation of error code */
   uns8 * nid_stat_msg; /* descriptive message related to error */
};

typedef struct nid_svc_status {
   uns32 nid_max_err_code;
   uns8 * nid_serv_name; /* Service name related to service code */
   struct nid_stat_desc stat_info[NID_MAX_STAT_CODS];
} NID_SVC_STATUS;

EXTERN_C const NID_SVC_STATUS nid_serv_stat_info[NID_MAXSERV];

/**********************************************************************
 *    Exported finctions by NID_API                                    *
 **********************************************************************/
EXTERN_C LEAPDLL_API uns32 nid_notify(NID_SVC_CODE, NID_STATUS_CODE, uns32 *);  
EXTERN_C LEAPDLL_API uns32 nis_notify(uns8 *, uns32 *);  
EXTERN_C uns32 nid_create_ipc(uns8 * );
EXTERN_C uns32 nid_open_ipc(int32 *fd, uns8 * );
EXTERN_C void nid_close_ipc(void);
EXTERN_C uns32 nid_is_ipcopen(void);
#endif /*NID_API_H*/


