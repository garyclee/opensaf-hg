/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2016 The OpenSAF Foundation
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
 * Author(s): Ericsson AB
 *
 */

#ifndef LGA_STATE_H
#define	LGA_STATE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "lga.h"

/* Recovery functions */
void lga_no_server_state_set(void);
void lga_serv_recov1state_set(void);
int recover_one_client(lga_client_hdl_rec_t *p_client);
uint32_t delete_one_client(
	lga_client_hdl_rec_t **list_head,
	lga_client_hdl_rec_t *rm_node
	);

#ifdef	__cplusplus
}
#endif

#endif	/* LGA_STATE_H */

