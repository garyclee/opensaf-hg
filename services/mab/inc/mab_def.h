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

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef MAB_DEF_H
#define MAB_DEF_H


/* SMM this stuff in this area belongs to os_svcs, mab_ipc.h/c; Move it */

/* The MAC Send and Recieve mailbox manipulations */

#define m_MAC_RCV_MSG(mbx,msg) (struct mab_msg*)m_NCS_IPC_RECEIVE(mbx,msg)
#define m_MAC_SND_MSG(mbx,msg)      m_NCS_IPC_SEND(mbx, msg, NCS_IPC_PRIORITY_NORMAL)

/* The MAS Send and Recieve mailbox manipulations */

#define m_MAS_RCV_MSG(mbx,msg) (struct mab_msg*)m_NCS_IPC_NON_BLK_RECEIVE(mbx,NULL)
#define m_MAS_SND_MSG(mbx,msg)      m_NCS_IPC_SEND(mbx, msg, NCS_IPC_PRIORITY_NORMAL)

/* The OAC Send and Recieve mailbox manipulations */

#define m_OAC_RCV_MSG(mbx,msg) (struct mab_msg*)m_NCS_IPC_RECEIVE(mbx,msg)
#define m_OAC_SND_MSG(mbx,msg)      m_NCS_IPC_SEND(mbx, msg, NCS_IPC_PRIORITY_NORMAL)

/* The PSS Send and Recieve mailbox manipulations */
#define m_PSS_RCV_MSG(mbx,msg) (struct mab_msg*)m_NCS_IPC_NON_BLK_RECEIVE(mbx, NULL)
#define m_PSS_SND_MSG(mbx,msg)      m_NCS_IPC_SEND(mbx, msg, NCS_IPC_PRIORITY_NORMAL)


#endif
