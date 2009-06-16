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
  
  DESCRIPTION: This file describes NCS SNMP SubAgent interface with the 
               MAC. 

               snmpsubagt_mac_initialize - To initialize the MAC Interface
               snmpsubagt_mac_finalize   - To Finalize the MAC Interface 
  ***************************************************************************/ 
#include "subagt.h"
  
/******************************************************************************
 *  Name:         snmpsubagt_mac_initialize
 *
 *  Description:   To initialize the MAC Interface 
 * 
 *  Arguments:     NONE
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: This routine gets the MAC handle, which shall be used in the 
 *        future communications with the MAC
 *****************************************************************************/
uns32
snmpsubagt_mac_initialize()
{
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_MAC_INITIALIZE);

    if(gl_mac_handle == (MABMAC_API uns32)(long) NULL)
    {
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MAC_INIT_FAILED, NCSCC_RC_FAILURE, 0, 0); 
        return NCSCC_RC_FAILURE;
    }
    /* set the MAC Handle */
    m_SUBAGT_MAC_HDL_SET(gl_mac_handle);

    /* Everything went great!! */
    return NCSCC_RC_SUCCESS; 
}
/******************************************************************************
 *  Name:         snmpsubagt_mac_finalize
 *
 *  Description:   To Finalize the MAC Interface   
 * 
 *  Arguments:     
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
uns32
snmpsubagt_mac_finalize()
{
    uns32               status = NCSCC_RC_FAILURE; 
    NCS_LIB_REQ_INFO    destroy_mac_intf; 

    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_MAC_FINALIZE);

    memset(&destroy_mac_intf, 0, sizeof(NCS_LIB_REQ_INFO));

    /* fill in the inputs reqd to desrtoy the MAC interface */ 
    destroy_mac_intf.i_op = NCS_LIB_REQ_DESTROY; 
    
    /* Call into MAC SE API */ 
    status  = maclib_request(&destroy_mac_intf);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log that MAC Finalization failed */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MAC_FINALIZE_FAILED, status, 0, 0); 
    }

    return status;
}

