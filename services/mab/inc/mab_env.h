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




  DESCRIPTION: Common abstractions shared by two or more of the MAB sub-parts,
               being OAC, MAS and MAC.

*******************************************************************************/

/* Module Inclusion Control....*/

#ifndef MAB_ENV_H
#define MAB_ENV_H



/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

      STRUCTURES, UNIONS and Derived Types

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#define MAC_LOCK_ID 1
#define MAS_LOCK_ID 2
#define OAC_LOCK_ID 3
#define PSR_LOCK_ID 4

/* Values for the MAB NCS_KEY field which identifies the MAB's subcomponent */

#define NCSMAB_MAC   1
#define NCSMAB_MAS   2
#define NCSMAB_OAC   3
#define NCSMAB_MAB   4
#define NCSMAB_PSR   5

/***************************************************************************
 * MAB General Function Prototypes
 ***************************************************************************/

/* mab_mds.c : MAB generic MDS particular encode/decode/copy routines */

EXTERN_C MABCOM_API uns32     mab_mds_enc(MDS_CLIENT_HDL  yr_svc_hdl, NCSCONTEXT msg, 
                               SS_SVC_ID to_svc,     NCS_UBAID* uba,
                               uns16 msg_fmt_ver);

EXTERN_C MABCOM_API uns32     mab_mds_dec(MDS_CLIENT_HDL  yr_svc_hdl, NCSCONTEXT* msg, 
                               SS_SVC_ID to_svc,     NCS_UBAID*  uba,
                               uns16 msg_fmt_ver);

EXTERN_C MABCOM_API uns32     mab_mds_cpy(MDS_CLIENT_HDL  yr_svc_hdl, NCSCONTEXT  msg,
                               SS_SVC_ID to_svc,     NCSCONTEXT*  cpy,
                               NCS_BOOL   last                       );

EXTERN_C MABCOM_API uns32     mab_mds_snd(MDS_HDL    mds_hdl,    NCSCONTEXT  msg,
                                          MDS_SVC_ID fr_svc,     MDS_SVC_ID  to_svc,
                                          MDS_DEST   to_dest);

#endif

