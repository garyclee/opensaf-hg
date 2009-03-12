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
  
  DESCRIPTION: This file describes the Interface with EDU.  
  ***************************************************************************/ 
#include "subagt.h"

/******************************************************************************
 *  Name:          snmpsubagt_edu_edp_initialize
 *
 *  Description:   To fill the EDP_ENTRY details for the EDP ids   
 * 
 *  Arguments:     EDP_ENTRY * - pointer to the EDP Entry to be filled in
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   - failure
 *  NOTE: 
 *****************************************************************************/
uns32
snmpsubagt_edu_edp_initialize(EDU_HDL *edu_hdl)
{
    uns32   status = NCSCC_RC_SUCCESS; 

    /* Nothing to do */
    return status; 
}

/******************************************************************************
 *  Name:          subagt_edu_trap_decode
 *
 *  Description:   To decode the trap in the event from EDA
 * 
 *  Arguments:     uns8*    - evt_data -- Event Data from EDA
 *
 *  Returns:       NCS_TRAP* - Pointer to the Trap 
 *                 NULL                 - Failure
 *  NOTE: 
 *          Though the return value is a local variable, it is the pointer
 *          to a allocated memory during the decode process. 
 *****************************************************************************/
uns32 
subagt_edu_ncs_trap_decode(EDU_HDL *edu_hdl, 
                           uns8 *evt_data, 
                           uns32 evt_data_size, 
                           NCS_TRAP *o_ncs_trap)
{
    uns32               ret_code = NCSCC_RC_SUCCESS; 
    EDU_ERR             ed_error = 0; 

    /* validate the input */
    if ((evt_data == NULL) || 
        (evt_data_size == 0) || 
        (edu_hdl == NULL) || 
        (o_ncs_trap == NULL))
    {
        /* log the error */
        return NCSCC_RC_FAILURE; 
    }

    /* Now decode the trap from the event data */
    ret_code = m_NCS_EDU_TLV_EXEC(edu_hdl, ncs_edp_ncs_trap,
                    evt_data, evt_data_size, EDP_OP_TYPE_DEC, 
                    &o_ncs_trap, &ed_error); 
    if (ret_code != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        /* do I need to print this onto console?? */
        m_NCS_EDU_PRINT_ERROR_STRING(ed_error); 

        /* free the allocated memory, in case of evt_data is being
         * decoded partially
         */
        /* do not worry about the return value, this routine returns
         * success always... even otherwise nothing can be done...
         */
        ncs_trap_eda_trap_varbinds_free(o_ncs_trap->i_trap_vb); 
    }

    return ret_code; 
}/* end of the function */



