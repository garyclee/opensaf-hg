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

  
  .....................................................................  
  
  DESCRIPTION: This file describes the definitions for SubAgent's mailbox    
               handling
  ***************************************************************************/ 
#ifndef SUBAGT_MBX_H
#define SUBAGT_MBX_H

/* kinds of messages that this subagent can understand */
typedef enum
{
    SNMPSUBAGT_MBX_MSG_DESTROY = 1, 
    SNMPSUBAGT_MBX_MSG_MIB_INIT_OR_DEINIT,
    SNMPSUBAGT_MBX_MSG_HEALTH_CHECK_START,
    SNMPSUBAGT_MBX_MSG_AGT_MONITOR,
    SNMPSUBAGT_MBX_MSG_RETRY_EDA_INIT,
    SNMPSUBAGT_MBX_MSG_RETRY_AMF_INIT,
    SNMPSUBAGT_MBX_MSG_RETRY_CHECK_SNMPD,
    SNMPSUBAGT_MBX_CONF_RELOAD, 
    SNMPSUBAGT_MBX_MSG_MAX
}SNMPSUBAGT_MBX_MSG_TYPE; 

/* Message format */ 
typedef struct snmpsubagt_mbx_msg
{
    NCS_IPC_MSG                 ipc_msg;

    /* type of the message */
    SNMPSUBAGT_MBX_MSG_TYPE     msg_type; 

    /* name of the init/deinit routine */
    uns8                        init_or_deinit_routine[255];
}SNMPSUBAGT_MBX_MSG;


/* to process the events/messages received */
EXTERN_C uns32
snmpsubagt_mbx_process(NCSSA_CB *cb);

#endif


