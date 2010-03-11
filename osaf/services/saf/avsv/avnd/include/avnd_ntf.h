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
  MODULE NAME: AVND_NTF.H

..............................................................................

  DESCRIPTION:   This file contains avnd ntfs related definitions.

******************************************************************************/

#ifndef AVND_NTF_H
#define AVND_NTF_H

#define ADDITION_TEXT_LENGTH 256
#define AVND_NTF_SENDER "AVND"

EXTERN_C uns32 avnd_gen_comp_inst_failed_ntf(AVND_CB *avnd_cb, AVND_COMP *comp);

EXTERN_C uns32 avnd_gen_comp_term_failed_ntf(AVND_CB *avnd_cb, AVND_COMP *comp);

EXTERN_C uns32 avnd_gen_comp_clean_failed_ntf(AVND_CB *avnd_cb, AVND_COMP *comp);

EXTERN_C uns32 avnd_gen_comp_proxied_orphaned_ntf(AVND_CB *avnd_cb, AVND_COMP *comp);

EXTERN_C uns32 avnd_gen_su_oper_state_chg_ntf(AVND_CB *avnd_cb, AVND_SU *su);

EXTERN_C uns32 avnd_gen_su_pres_state_chg_ntf(AVND_CB *avnd_cb, AVND_SU *su);

EXTERN_C uns32 avnd_gen_comp_fail_on_node_ntf(AVND_CB *avnd_cb, AVND_ERR_SRC errSrc, AVND_COMP *comp);

EXTERN_C void fill_ntf_header_part_avnd(SaNtfNotificationHeaderT *notificationHeader,
					SaNtfEventTypeT eventType,
					SaNameT comp_name,
					SaUint8T *add_text,
					SaUint16T minorid,
					SaInt8T *avnd_name);

EXTERN_C uns32 sendAlarmNotificationAvnd(AVND_CB *avnd_cb,
					 SaNameT comp_name,
					 SaUint8T *add_text,
					 SaUint16T minorid,
					 uns32 probableCause,
					 uns32 perceivedSeverity);

EXTERN_C uns32 sendStateChangeNotificationAvnd(AVND_CB *avnd_cb,
					       SaNameT comp_name,
					       SaUint8T *add_text,
					       SaUint16T minorId,
					       uns32 sourceIndicator,
					       SaUint16T stateId,
					       SaUint16T newState);

#endif
