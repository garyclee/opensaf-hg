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
  
  MQSv event definitions.
    
******************************************************************************
*/

#ifndef MQSV_H
#define MQSV_H

/* Get General Global Definations */
#include "ncsgl_defs.h"

/* Get target's suite of header files...*/

/* From /base/common/inc */
#include "ncs_lib.h"
#include "ncs_main_papi.h"
#include "mds_papi.h"
#include "ncs_mda_papi.h"

#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncs_ubaid.h"
#include "ncsdlib.h"
#include "ncs_queue.h"
#include "ncs_saf.h"
#include "ncs_tmr.h"
#include "ncssysf_tsk.h"

/* EDU Includes... */
#include "ncs_edu_pub.h"
#include "ncsencdec_pub.h"
#include "ncs_saf_edu.h"

/* SAF defines. */
#include "saAis.h"
#include "saMsg.h"
#include "saf_def.h"

/* Common MQSv defines */
#include "saf_mem.h"
#include "mqsv_mem.h"
#include "mqsv_def.h"
#include "mqsv_evt.h"
#include "mqsv_edu.h"
#include "mqsv_mbedu.h"
/* ASAPi defines */
#include "mqsv_asapi.h"
#include "stdlib.h"

/* From /leap/os_svcs/leap_basic/inc */
#include "sysf_def.h"
#include "mqsv_common.h"
#include "ncs_util.h"

#endif   /* MQSV_H */
