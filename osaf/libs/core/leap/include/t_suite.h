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

  This module contains a list of include files to be pre-processed before
  *any* include files are processed. The contents of this file are
  extremely environment/target-system dependent.

  Update:  t_suite.h is scheduled for removal in Phase 2 of the LEAP 
           separation project, which is scheduled 

  LEAP - Basic : These files will remain in the basic form of LEAP after 
                 the separation is finalized.  These files should be directly
                 included in all source files.  Header files may include any
                 other header file, as long as they reside in pub_inc and NOT
                 in inc.  Source files may use any header file in inc and 
                 pub_inc.

  LEAP - Extd  : These files are only available in the extended form of LEAP.
                 Same rules apply as above.   

..............................................................................
*/

/*
 * Module Inclusion Control...
 */
#ifndef T_SUITE_H
#define T_SUITE_H

/* LEAP - Basic */
#include "ncs_opt.h"

#if(NCS_PNNI==1)
#include "pnni_ver.h"
#endif

/* LEAP - Basic */
#include "ncs_osprm.h"

/* LEAP - Prod */
#include "ncs_ipprm.h"

#if (NCS_SAF == 1)
#include "saAis.h"
#include "saAmf.h"
#include "saClm.h"
#include "saEvt.h"
#include "saLck.h"
#include "saCkpt.h"
#else

/* LEAP - Basic */
#include "ncs_saf.h"
#endif

/*
 * SYSF_* includes
 *
 * The following include files contain declarations that are used by the
 * entire Harris & Jeffries Product Suite for compilation.
 */

#ifndef ENABLE_ATM
#define ENABLE_ATM 0
#endif
/*******************************/
/*       LEAP - Prod           */
/* These were originally LEAP  */
/* files, however, these are to*/
/* be considered product files */
/* from now on. BBS/NCS builds */
/* will not have access to     */
/* them, so they should not be */
/* included any longer.        */
/*******************************/
#if (ENABLE_ATM == 1)
#include "sysf_svc.h"
#include "ncs_sar.h"
#include "sysfcpcs.h"
#include "sysfcsum.h"
#include "sysf_sfc.h"
#include "sysficra.h"
#include "ncs_hdlc.h"
#include "sysf_fr.h"
#include "mpls_mem.h"
#endif

/*******************************/
/*       LEAP - Basic          */
/* The below includes will     */
/* always be available in the  */
/* basic form of leap.  These  */
/* should be included directly */
/* into src files, and not     */
/* here.                       */
/*******************************/
#include "ncs_svd.h"
#include "ncs_hdl_pub.h"
#include "ncssysf_lck.h"
#include "ncsusrbuf.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"
#include "ncssysfpool.h"
#include "ncssysf_tmr.h"
#include "ncssysf_mem.h"
#include "ncssysf_ipc.h"
#include "ncssysf_tsk.h"
#include "ncssysf_sem.h"
#include "ncsmib_mem.h"
#include "ncspatricia.h"
#include "ncs_queue.h"

/*******************************/
/*       LEAP - Basic          */
/* The below includes violate  */
/* the inc/pub_inc rules and   */
/* WILL be removed by Phase 2  */
/* split.                      */
/*******************************/
#include "ncs_tasks.h"
#include "sysf_ipc.h"

/*******************************/
/*       LEAP - Prod           */
/* These two headers are IP    */
/* related, which will be moved*/
/* to leap_prod in phase 3,    */
/* they shouldn't be included  */
/* any longer in BBS/NCS       */
/* releases.                   */
/*******************************/
#include "sysf_ip.h"
#include "ncs_ip.h"

/*******************************/
/*       LEAP - Basic          */
/* The below includes are to   */
/* headers in the inc folders  */
/* which create direct         */
/* violations.  These have been*/
/* replaced with their pub_inc */
/* equivalent above.           */
/*******************************/
/*
#include "ncs_hdl.h"
#include "sysf_lck.h"
#include "usrbuf.h"
#include "sysf_def.h"
#include "sysfpool.h"
#include "sysf_tmr.h"
#include "sysf_mem.h"
#include "sysf_tsk.h"
#include "sysf_sem.h"
#include "sysf_file.h"
#include "sysf_ni.h"
#include "mib_mem.h"
*/

 /*
  * Additional include files for CMS subsystem...
  */

#if (NCS_CMS == 1)
#include "cms_opt.h"
#include "cms_def.h"
#include "cms_mem.h"
#include "sysf_tsk.h"

#if (NCSCMS_PVCMGR_COMPONENT != 0)
#include "pvc_def.h"
#endif   /* (NCSCMS_PVCMGR_COMPONENT != 0) */
#endif

 /*
  * Additional include files for SSS subsystem...
  */
#if (NCS_SIGL == 1)
#include "ncssssopt.h"
#include "sig_def.h"
#include "sig_mem.h"

#if (PNNI == 1)
/*
 * Additional include files for PNNI Signalling subsystem...
 */
#include "psig_def.h"
#endif   /* (PNNI == 1) */
#endif   /* (NCS_SIGL == 1) */

 /*
  * Additional include files for SAAL subsystem...
  */

#if (NCS_SAAL == 1)
#include "saal_opt.h"
#include "saal_os.h"
#include "saal_def.h"
#endif   /* (NCS_SAAL == 1) */

 /*
  * Additional include files for A2SIG subsystem...
  */

#if (NCS_A2SIG == 1)
#include "a2sigopt.h"
#include "a2os_def.h"
#include "a2sigapp.h"
#endif   /* (NCS_A2SIG == 1) */

/*
 * Additional include files for LANES Client component...
 */
#if (NCS_LANES_LEC == 1)
#include "ncslecopt.h"
#include "t_sigl_h.h"
#include "lec_def.h"
#include "lec_mem.h"
#endif

/*
 * Additional include files for LANES Server component...
 */
#if (NCS_LANES_SERVER == 1)
#include "ncslesopt.h"
#include "t_lessig.h"
#include "ncslesdef.h"
#include "ncslesmem.h"
#endif

/*
 * Additional include files for the ILMI subsystem...
 */
#if (NCS_ILMI == 1)
#include "ncsil_opt.h"
#include "ncsil_def.h"
#include "ncsil_mem.h"
#include "ncsil_be.h"
#endif

/*
 * Additional include files for IPOA subsystem...
 */
#if (NCS_IPOA == 1)
#include "ipoa_opt.h"
#include "ipoa_sig.h"
#include "ipoa_def.h"
#include "ipoa_mem.h"
#endif

/*
 * Additional include files for MARS RFC 2022 subsystem...
 */
#if (NCS_MARS == 1)
#include "mars_opt.h"
#include "mars_sig.h"
#include "mars_def.h"
#include "mars_mem.h"
#endif

 /*
  * Additional include files for FRF8 subsystem...
  */
#if (NCS_FRF8 == 1)
#include "frf8_opt.h"
#include "atm_adr.h"
#include "atm_mis.h"
#include "patricia.h"
#include "frf8_def.h"
#include "frf8_mem.h"
#endif

/*
 * Additional include files for PNNI Routing...
 */
#if (NCS_PNNI == 1)

#include "pnni_opt.h"
#include "tsigl_if.h"

#include "pnni_def.h"
#include "pnni_mem.h"
#include "pnni_tsk.h"
#endif

/*
 * Additional include files for HPFR subsystem...
 */
#if (NCS_HPFR == 1)
#include "frsopt.h"
#include "frs_def.h"
#include "frsmem.h"
#endif

/*
 * Additional include files for Frame Relay SSS subsystem...
 */
#if (NCS_FRSIGL == 1)
#include "frsigopt.h"
#include "frsigdef.h"
#include "frsigmem.h"
#endif

/*
 * Additional include files for Frame Relay Q.922 subsystem...
 */
#if (NCS_Q922 == 1)
#include "q922opt.h"
#include "q922defs.h"
#include "q922mem.h"
#endif

/*
 * Additional include files for the FRF5 subsystem...
 */
#if (NCS_FRF5 == 1)
#include "frf5_opt.h"
#include "atm_mis.h"
#include "frf5_def.h"
#include "frf5_mem.h"
#endif

/*
 * Additional include files for MPOA Client component...
 */
#if (NCS_ECM == 1)
#include "ecm_opt.h"
#include "ecm_def.h"
#include "ecm_mem.h"
#endif

/*
 * Additional include files for MPOA Client component...
 */
#if (NCS_MPC == 1)
#include "ncsmpcopt.h"
#include "mpc_def.h"
#include "mpc_mem.h"
#endif

/*
 * Additional include files for MPOA Server component...
 */
#if (NCS_MPS == 1)
#include "ncsmpsopt.h"
#include "mps_def.h"
#include "mps_mem.h"
#include "mps_rdef.h"
#endif

/*
 * Additional include files for RMS subsystem...
 */
#if (NCS_RMS == 1)
#include "rms_opt.h"
#include "rms_def.h"
#include "rms_mem.h"
#endif

/*
 * Additional include files for LCTS system...
 */
#if (APS_LMS == 1)
#include "lms_opt.h"
#include "lms_def.h"
#include "lms_mem.h"
#endif

/*
 * This one is common for LTCS & OPTIRoute, and must
 * come AFTER lms_def.h.
 */

#if (NEEDS_TO_BE_REMOVED_FROM_LEAP == 1)
#include "rdb_def.h"
#endif

#if (NCSMDS_USE_MDS_XLIM == 1)

#include "patricia.h"
#include "sysfxlim.h"
#endif

/* 
 * Additional include files for IPRP system...
 */
#if (APS_IPRP == 1)

#include "iprp_opt.h"
#include "iprp_def.h"
#include "iprp_mem.h"

#if (APS_OSPF == 1)
#include "ospf_opt.h"
#include "ospf_def.h"
#include "ospf_mem.h"
#include "ospf_ipc.h"
#endif   /* (APS_OSPF == 1) */

#if (APS_ISIS == 1)
#include "isis_opt.h"
#include "isis_def.h"
#include "isis_mem.h"
#include "isis_ipc.h"
#endif   /* (APS_ISIS == 1) */

#if (APS_BGP == 1)
#include "bgp_opt.h"
#include "bgp_def.h"
#include "bgp_mem.h"
#include "bgp_ipc.h"
#endif   /* (APS_BGP == 1) */

#if (APS_RIP == 1)
#include "rip_opt.h"
#include "rip_def.h"
#include "rip_mem.h"
#include "rip_ipc.h"
#endif   /* (APS_RIP == 1) */

#if ((APS_FIBS == 1) || (APS_FIBC == 1))
#include "fib_opt.h"
#include "fib_defs.h"
#include "fib_ipc.h"
#include "fib_mem.h"
#endif   /* ((APS_FIBS == 1) || (APS_FIBC == 1)) */
#endif   /* (APS_IPRP == 1) */

/* Additional include files for CLI system... */

/*
 * Additional include files for MAB subsystem...
 */

#if (NCS_MAB == 1)
#include "mab_opt.h"
#include "mab_def.h"
#include "mab_mem.h"
#include "mab_tgt.h"
#endif

#if (NCS_USE_SYSMON == 1)
#include "ncs_sysm.h"
#include "sysfsysm.h"
#endif

/*
 * Additional include files for DPE...
 */
#if (NCS_SOFT_DPE == 1)
#include "sysfdpde.h"
#endif

/*
 * Additional include files for LMP subsystem...
 */

#if (APS_LMP == 1)
#include "lmp_opt.h"
#include "lmp_def.h"
#include "lmp_mem.h"
#include "lmp_ipc.h"
#endif

#endif   /** T_SUITE_H **/
