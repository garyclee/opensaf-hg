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

#ifndef DTS_DEF_H
#define DTS_DEF_H

/*
 * m_<DTS>_RCV_MSG
 * m_<DTS>_SND_MDS
 *
 * RCV_MSG  : DTS thread will go to the mailbox value delivered at
 *            bindary time and attempt to get a message. This assumes the AKE
 *            has put the mailbox in start state.
 *
 * SND_MDS  : Typically it will be the MDS (barrowed) thread that will post a
 *            message to the proper DTA.
 * 
 */

extern DTSDLL_API SYSF_MBX        gl_dts_mbx;


/* The DTS Send and Recieve mailbox manipulations */

#define m_DTS_RCV_MSG(mbx,msg)      (struct dtsv_msg *)m_NCS_IPC_RECEIVE(mbx,msg)
#define m_DTS_SND_MSG(mbx,msg, pri)  m_NCS_IPC_SEND(mbx, msg, pri)

#endif
