/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2009 The OpenSAF Foundation
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

  This file contains PID monitoring related definitions.
  
******************************************************************************
*/

#ifndef AVND_MON_H
#define AVND_MON_H


EXTERN_C uns32 gl_avnd_hdl;

EXTERN_C void avnd_pid_mon_list_init(struct avnd_cb_tag *);
EXTERN_C void avnd_pid_mon_list_destroy(struct avnd_cb_tag *);
EXTERN_C void avnd_mon_process(void *);
EXTERN_C uns32 avnd_mon_task_create(void);
EXTERN_C uns32 avnd_mon_req_del(struct avnd_cb_tag *, SaUint64T);
EXTERN_C uns32 avnd_mon_req_free(NCS_DB_LINK_LIST_NODE *);
EXTERN_C uns32 avnd_evt_pid_exit_evt(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C struct avnd_mon_req_tag *avnd_mon_req_add(struct avnd_cb_tag *, AVND_COMP_PM_REC *);

#endif   /* !AVND_MON_H */
