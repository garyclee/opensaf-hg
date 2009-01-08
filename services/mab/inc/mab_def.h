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

  $Header $



..............................................................................

  DESCRIPTION:

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef MAB_DEF_H
#define MAB_DEF_H

#if 0
/* all this is not relevant when AKE is not present */

/*
 * m_<MAB>_RCV_MSG
 * m_<MAB>_SND_MDS
 *
 * RCV_MSG  : MAC, MAS, OAC or PSS thread will go to the mailbox value delivered at
 *            bindary time and attempt to get a message. This assumes the AKE
 *            has put the mailbox in start state.
 *
 * SND_MDS  : Typically it will be the MDS (barrowed) thread that will post a
 *            message to the proper MAB subcomponent (MAC, MAS, OAC and PSS).
 * 
 */

extern SYSF_MBX        gl_mac_mbx;
extern SYSF_MBX        gl_mas_mbx;
extern SYSF_MBX        gl_oac_mbx;
extern SYSF_MBX        gl_psr_mbx;
#endif


/* SMM this stuff in this area belongs to os_svcs, mab_ipc.h/c; Move it */

/* The MAC Send and Recieve mailbox manipulations */

#define m_MAC_RCV_MSG(mbx,msg) (struct mab_msg*)m_NCS_IPC_RECEIVE(mbx,msg)
#define m_MAC_SND_MSG(mbx,msg)      m_NCS_IPC_SEND(mbx, msg, NCS_IPC_PRIORITY_NORMAL)

/* The MAS Send and Recieve mailbox manipulations */

#if 0
#define m_MAS_RCV_MSG(mbx,msg) (struct mab_msg*)m_NCS_IPC_RECEIVE(mbx,msg)
#endif
#define m_MAS_RCV_MSG(mbx,msg) (struct mab_msg*)m_NCS_IPC_NON_BLK_RECEIVE(mbx,NULL)
#define m_MAS_SND_MSG(mbx,msg)      m_NCS_IPC_SEND(mbx, msg, NCS_IPC_PRIORITY_NORMAL)

/* The OAC Send and Recieve mailbox manipulations */

#define m_OAC_RCV_MSG(mbx,msg) (struct mab_msg*)m_NCS_IPC_RECEIVE(mbx,msg)
#define m_OAC_SND_MSG(mbx,msg)      m_NCS_IPC_SEND(mbx, msg, NCS_IPC_PRIORITY_NORMAL)

/* The PSS Send and Recieve mailbox manipulations */
#if 0
#define m_PSS_RCV_MSG(mbx,msg) (struct mab_msg*)m_NCS_IPC_RECEIVE(mbx,msg)
#endif
#define m_PSS_RCV_MSG(mbx,msg) (struct mab_msg*)m_NCS_IPC_NON_BLK_RECEIVE(mbx, NULL)
#define m_PSS_SND_MSG(mbx,msg)      m_NCS_IPC_SEND(mbx, msg, NCS_IPC_PRIORITY_NORMAL)


#endif
