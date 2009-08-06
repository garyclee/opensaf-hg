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
  MODULE NAME: AVND_TRAP.H

..............................................................................

  DESCRIPTION:   This file contains avnd traps related definitions.

******************************************************************************/

#ifndef AVND_TRAP_H
#define AVND_TRAP_H

#define AVND_EDA_EVT_PUBLISHER_NAME "AVND"

EXTERN_C uns32 avnd_gen_comp_inst_failed_trap(AVND_CB *avnd_cb, AVND_COMP *comp);

EXTERN_C uns32 avnd_gen_comp_term_failed_trap(AVND_CB *avnd_cb, AVND_COMP *comp);

EXTERN_C uns32 avnd_gen_comp_clean_failed_trap(AVND_CB *avnd_cb, AVND_COMP *comp);

EXTERN_C uns32 avnd_gen_comp_proxied_orphaned_trap(AVND_CB *avnd_cb, AVND_COMP *comp);

EXTERN_C uns32 avnd_populate_comp_traps(AVND_CB *avnd_cb, AVND_COMP *comp, NCS_TRAP *trap, NCS_BOOL isProxTrap);

EXTERN_C uns32 avnd_gen_su_oper_state_chg_trap(AVND_CB *avnd_cb, AVND_SU *su);

EXTERN_C uns32 avnd_gen_su_pres_state_chg_trap(AVND_CB *avnd_cb, AVND_SU *su);

EXTERN_C uns32 avnd_populate_su_traps(AVND_CB *avnd_cb, AVND_SU *su, NCS_TRAP *trap, NCS_BOOL isOper);

EXTERN_C uns32 avnd_gen_comp_fail_on_node_trap(AVND_CB *avnd_cb, AVND_ERR_SRC errSrc, AVND_COMP *comp);

EXTERN_C uns32 avnd_create_and_send_trap(AVND_CB *avnd_cb, NCS_TRAP *trap, SaEvtEventPatternT *pattern);

EXTERN_C uns32 avnd_eda_initialize(AVND_CB *avnd_cb);

EXTERN_C uns32 avnd_eda_finalize(AVND_CB *avnd_cb);

#endif
