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
  
  DESCRIPTION: This file includes the files that the NCS SNMP SubAgent is 
               dependent on. 
*****************************************************************************/
#ifndef SUBAGT_H
#define SUBAGT_H

/* global definitions */
#include "ncsgl_defs.h"
#include "ncs_mib_pub.h"
#include "ncs_ubaid.h"
#include "ncs_svd.h"
#include "ncspatricia.h"
#include "ncs_hdl.h"
#include "ncs_lib.h"
#include "ncsdlib.h"
#include "sysf_def.h"
#include "ncssysf_tmr.h"
#include "ncs_trap.h"

/* mab specific */
#include "mab.h"
#include "mac_papi.h"

/* EDU specific */
#include "ncs_edu_pub.h"

/* NCS Logging */
#include "ncs_log.h"

/* SAF specific includes */
#include "saAis.h"
#include "saAmf.h"
#include "saEvt.h"

#if (NET_SNMP_5_2_2_SUPPORT == 0)

/* Net-snmp 5.4 specifics */
#include "net-snmp/net-snmp-config.h"
#endif

/* SubAgent specifics */
#include "subagt_amf.h"
#include "subagt_dlapi.h"
#include "subagt_edu.h"
#include "subagt_agt.h"
#include "subagt_log.h"
#include "subagt_mem.h"
#include "subagt_eda.h"
#include "subagt_cb.h"
#include "subagt_mac.h"
#include "subagt_mbx.h"

#endif
