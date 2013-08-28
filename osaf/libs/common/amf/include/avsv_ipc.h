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

  This file contains macros for IPC operations.
  
******************************************************************************
*/

#ifndef AVSV_IPC_H
#define AVSV_IPC_H

#include <ncssysf_ipc.h>

/*****************************************************************************

  MACRO: m_AVSV_MBX_CREATE    

   Macro to create a mailbox. 
   
   In the sample below, this macro invokes a common macro that actually 
   allocates and/or initializes an IPC mailbox. This macro returns
   success or failure. 

******************************************************************************/

#define m_AVSV_MBX_CREATE(cb, srv_id, rc) \
      {\
      rc = NCSCC_RC_FAILURE; \
      if ((cb->mbx = m_NCS_MEM_ALLOC (sizeof(SYSF_MBX), \
                        NCS_MEM_REGION_PERSISTENT,\
                        srv_id, 1)) != NULL)\
         rc = m_NCS_IPC_CREATE(cb->mbx); \
      }\


/*****************************************************************************

  MACRO: m_AVSV_MBX_ATTACH    

   Macro to attach to a mailbox. 
   
   In the sample below, this macro invokes a common macro that is 
   invoked in order to attach to an IPC mailbox. This macro returns
   success or failure.  

******************************************************************************/

#define m_AVSV_MBX_ATTACH(cb, rc) \
         rc = m_NCS_IPC_ATTACH(cb->mbx)

/*****************************************************************************

  MACRO: m_AVSV_MBX_SEND    

   Macro to send an "event" to a <LAYER>-instance. 
   
   In the sample below, this macro invokes a common macro that actually 
   sends the event    to the IPC "In" mailbox attached to the <LAYER>-instance 
   with priority specified in msg_priority.This macro returns
   success or failure. 

******************************************************************************/

#define m_AVSV_MBX_SEND(cb, evt, priority, rc) \
      rc = m_NCS_IPC_SEND(&cb->mbx, evt, priority)

/*****************************************************************************
   MACRO: m_AVSV_MBX_RECV
   Macro to receive an "event" for a <LAYER>-instance. 

   In the sample below, this macro invokes a lower-tier SYSF macro that actually
   receives the event from the IPC "In" mailbox attached to the <LAYER>-instance. 
   You may implement this macro to "block" the caller's processing thread until 
   new events occur. However, special attention needs to be directed towards the
   m_<LAYER>_RELEASE_MBX macro. By default, this macro will invoke an H&J
   supplied function that attempts to dequeue any pending messages in the
   mailbox and discard all resources associated with each event. If the _RECV_EVT
   macro pends, then the H&J function should be replaced by a function that
   can peek-and-dequeue events from IPC mailboxes to avoid being "unexpectedly
   blocked" whilst releasing a mailbox.
******************************************************************************/
#define m_AVSV_MBX_RECV(cd, evt, evt_buf)  \
                evt = m_NCS_IPC_RECEIVE(cd->mbx, evt)

/*****************************************************************************
   MACRO: m_AVSV_MBX_DETACH
   Macro to invoke freeing of "events" for an AVD or AVND mailbox.
   This is invoked from inside the AVD or AVND code when AVD or AVND
   is destroyed.This macro returns
   success or failure. 

******************************************************************************/
#define m_AVSV_MBX_DETACH(cb, func_callbk, rc)  \
              rc = m_NCS_IPC_DETACH(cb->mbx, (NCS_IPC_CB)func_callbk, cb)

/*****************************************************************************
   MACRO: m_AVSV_MBX_RELEASE
   Macro to invoke release of the mailbox. This macro returns
   success or failure. 

******************************************************************************/
#define m_AVSV_MBX_RELEASE(cb, func_callbk, rc, srv_id)  \
         {\
            rc = NCSCC_RC_FAILURE; \
            if ((rc = m_NCS_IPC_RELEASE (cb->mbx, NULL)) == NCSCC_RC_SUCCESS) \
              m_NCS_MEM_FREE (cb->mbx, NCS_MEM_REGION_PERSISTENT, srv_id, 1); \
          }\


typedef NCS_IPC_MSG AVSV_MBX_MSG;

#endif   /* !AVSV_IPC_H */
