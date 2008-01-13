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

  MODULE NAME: IFA_MDS.H

$Header: 
..............................................................................

  DESCRIPTION: Prototypes definations for IFA-MDS functions


******************************************************************************/

#ifndef IFA_MDS_H
#define IFA_MDS_H

#define IFA_SVC_PVT_SUBPART_VERSION  1
#define IFA_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT 1
#define IFA_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT 1
#define IFA_WRT_IFND_SUBPART_VER_RANGE             \
        (IFA_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT - \
         IFA_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT +1)


EXTERN_C uns32 ifa_mds_init (IFA_CB *cb);
EXTERN_C void ifa_mds_shut (IFA_CB *cb);
EXTERN_C uns32 ifa_mds_adest_get (IFA_CB *cb);
EXTERN_C uns32 ifa_mds_rcv(IFA_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info);
EXTERN_C uns32 ifa_mds_enc(IFA_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info);
EXTERN_C uns32 ifa_mds_dec (IFA_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info);
EXTERN_C uns32 ifa_mds_cpy (IFA_CB *cb, MDS_CALLBACK_COPY_INFO *cpy_info);
EXTERN_C uns32 ifa_mds_svc_evt(IFA_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt);
EXTERN_C uns32 ifa_mds_msg_send (IFA_CB *ifa_cb, IFSV_EVT *evt);
EXTERN_C uns32 ifa_mds_callback (NCSMDS_CALLBACK_INFO *info);
#endif /* IFA_MDS_H */

