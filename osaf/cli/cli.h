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

 MODULE NAME:  CLI.H

..............................................................................

  DESCRIPTION:

  Main Header file for CLI

******************************************************************************
*/

#ifndef CLI_H
#define CLI_H


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Netplane Common Include Files.
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "ncsgl_defs.h"
#include "ncs_mib_pub.h"
#include "ncsmiblib.h"
#include "ncspatricia.h"
#include "ncs_cache.h"
#include "ncs_hdl_pub.h"
#include "ncs_mds_def.h"
#include "ncs_mtbl.h"
#include "ncs_qptrs.h"
#include "ncs_queue.h"
#include "ncs_saf.h"
#include "ncs_stack_pub.h"
#include "ncs_stree.h"
#include "ncs_svd.h"
#include "ncs_ubaid.h"
#include "ncs_ubsar_pub.h"
#include "ncsencdec_pub.h"

#include "ncs_edu_pub.h"
#include "ncs_ipprm.h"
#include "ncs_ip.h"
#include "ncs_iplib.h"
#include "ncs_ipv4.h"
#include "ncs_lib.h"
#include "mds_papi.h"
#include "ncs_mda_papi.h"
#include "ncs_tmr.h"
#include "ncssysf_sem.h"
#include "ncssysf_tsk.h"

#include "ncs_log.h"
#include "dta_papi.h"

#if (NCS_IPV6 == 1)
#include "ncs_ipv6.h"
#endif

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            MAA Include files

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "mac_papi.h"

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            H&J CLI Porting Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "cli_def.h"
#include "cli_opt.h"
#include "cli_mem.h"

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        CLI Base Include Files.
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "cliconst.h"
#include "ncs_cli.h"
#include "clistruc.h"
#include "cliio.h"
#include "clitrav.h"
#include "clipar.h"
#include "clilog.h"
#include "clictree.h"


#endif
