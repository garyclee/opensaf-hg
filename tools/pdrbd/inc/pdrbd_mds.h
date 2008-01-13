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

#ifndef PDRBD_MDS_H
#define PDRBD_MDS_H



#include "pdrbd.h"


extern uns32 pdrbd_get_ada_hdl(void);
extern uns32 pdrbd_mds_install_and_subscribe(void);
extern uns32 pdrbd_mds_uninstall (void);
extern uns32 pdrbd_mds_callback(NCSMDS_CALLBACK_INFO * cbinfo);
extern uns32 pdrbd_mds_rcv(NCSCONTEXT yr_svc_hdl, NCSCONTEXT msg);
extern void pdrbd_mds_evt(MDS_CALLBACK_SVC_EVENT_INFO svc_info,
                   NCSCONTEXT  yr_svc_hdl);
extern uns32 pdrbd_mds_enc(NCSCONTEXT yr_svc_hdl, NCSCONTEXT msg,
                  SS_SVC_ID to_svc,     NCS_UBAID* uba);
extern uns32 pdrbd_mds_dec(NCSCONTEXT yr_svc_hdl, NCSCONTEXT* msg,
                  SS_SVC_ID to_svc,     NCS_UBAID*  uba);
extern uns32 pdrbd_mds_async_send(struct pseudo_cb *inst, PDRBD_EVT_TYPE evt_type, uns32 msg_cnt);

#endif /* PDRBD_MDS_H */
