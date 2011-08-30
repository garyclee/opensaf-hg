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

  DESCRIPTION:  This file declares the main entry point into NCS. 

******************************************************************************
*/

#ifndef NCS_MAIN_PAPI_H
#define NCS_MAIN_PAPI_H

#include "ncsgl_defs.h"

#ifdef  __cplusplus
extern "C" {
#endif

/***********************************************************************\
   ncs_agents_startup: This function initializes all service agents for
                    a process. It attempts a symbol lookup on the
                    service-agent entry points. It starts all agents
                    that it can lookup this way. 

                    At a minimum it starts the following services
                    - leap
                    - mds

                    The following services are started only if a symbol
                    look up on their entry-points is successful.
                    - ava                    
                    - gla
                    - mqa
                    (etc.)

\***********************************************************************/
	unsigned int ncs_agents_startup(void);

/***********************************************************************\
   ncs_agents_shutdown: This function shutdown agents. 
\***********************************************************************/
	unsigned int ncs_agents_shutdown(void);

/***********************************************************************\
   Individual agents startup and shutdown functions  
\***********************************************************************/
	unsigned int ncs_core_agents_startup(void);
	unsigned int ncs_mbca_startup(void);

	unsigned int ncs_leap_startup(void);
	unsigned int ncs_mds_startup(void);

	unsigned int ncs_mbca_shutdown(void);
	unsigned int ncs_core_agents_shutdown(void);

	void ncs_mds_shutdown(void);
	void ncs_leap_shutdown(void);

#define m_NCS_GET_CHASSIS_TYPE(i_max_len, o_chassis_type )\
                         ncs_get_chassis_type( i_max_len, o_chassis_type )

/***********************************************************************\
Name:       m_NCS_GET_CHASSIS_TYPE 

Arguments:
 
i_max_len: The maximum number of bytes to be read into the "o_chassis_type" 
            buffer _excluding_ the null-terminating character. It should be 
            set to NCS_MAX_CHASSIS_TYPE_LEN to accomodate the longest 
            chassis-type string accepted

o_chassis_type: An "out" argument which will contain the chassis-type string  
                when this function returns success. It should be of size 
                "NCS_MAX_CHASSIS_TYPE_LEN+1"  to accomodate the longest
                chassis-type string accepted
 
Return Values:
                NCSCC_RC_SUCCESS:  
                NCSCC_RC_FAILURE: 
 
Description:
             This API fetches the chassis-type string into the user-provided 
             buffer. It reads upto "NCS_MAX_CHASSIS_TYPE_LEN" or "i_max_len"
             (whichever is lesser) bytes from the PKGSYSCONFDIR/chassis_type 
             file into the "o_chassis_type" buffer.  
             A null-terminating character is inserted after that.  
 
             It is assumed that the chassis-type string contains printable 
             characters only and does not contain white-space characters in  
             it (" ", "\n", etc.). Furthermore, the PKGSYSCONFDIR/chassis_type
             file will be assumed to be a single lined text file (containing the 
             characters comprising the chassis-type string followed by a  
             newline character.)
\***********************************************************************/
	uint32_t ncs_get_chassis_type(uint32_t i_max_len, char *o_chassis_type);

/* Excluding null character byte for string termination */
#define NCS_MAX_CHASSIS_TYPE_LEN  (40)

/***********************************************************************\
   m_NCS_GET_NODE_ID: This function returns a node-id (in the SAF sense).
                      Only the macro should be used, the function 
                      prototyped below should not be used. 

   Example usage :    NODE_ID node_id = m_NCS_GET_NODE_ID;
   Note : This Macro should be called only after calling the function  
          ncs_agents_startup or function ncs_core_agents_startup only
 
\***********************************************************************/
	NCS_NODE_ID ncs_get_node_id(void);
#define m_NCS_GET_NODE_ID ncs_get_node_id()

#define m_NCS_GET_NODE_ID_FROM_PHYINFO( i_chassis_id,  i_phy_slot_id ,\
                                        i_sub_slot_id,  o_node_id)\
                                        ncs_get_node_id_from_phyinfo( i_chassis_id ,\
                                        i_phy_slot_id, i_sub_slot_id, o_node_id)

/****************************************************************************
 Name          :  ncs_get_node_id_from_phyinfo

 Description   :  This function combines chassis id ,physical
                  slot id and  sub slot id into node_id

 Arguments     :  i_chassis_id  - chassis id
                  i_phy_slot_id - physical slot id
                  i_sub_slot_id - slot id
                  *o_node_id - node_id

 Return Values :  On Failure NCSCC_RC_FAILURE
                  On Success NCSCC_RC_SUCCESS

 Notes         :  None.
 ******************************************************************************/

	uint8_t ncs_get_node_id_from_phyinfo(NCS_CHASSIS_ID i_chassis_id,
						   NCS_PHY_SLOT_ID i_phy_slot_id,
						   NCS_SUB_SLOT_ID i_sub_slot_id, NCS_NODE_ID *o_node_id);

#define m_NCS_GET_PHYINFO_FROM_NODE_ID( i_node_id, o_chassis_id, o_phy_slot_id ,\
                                        o_sub_slot_id) ncs_get_phyinfo_from_node_id( i_node_id,\
                                        o_chassis_id , o_phy_slot_id, o_sub_slot_id)
/****************************************************************************
  Name          :  ncs_get_node_id_from_phyinfo

  Description   :  This function extracts node_id from chassis id ,physical
                   slot id and  sub slot id into node_id

  Arguments     :  i_chassis_id  - chassis id
                   i_phy_slot_id - physical slot id
                   i_sub_slot_id - slot id
                   *o_node_id - node_id

  Return Values :  On Failure NCSCC_RC_FAILURE
                   On Success NCSCC_RC_SUCCESS

  Notes         :  None.
******************************************************************************/
	uint8_t ncs_get_phyinfo_from_node_id(NCS_NODE_ID i_node_id,
						   NCS_CHASSIS_ID *o_chassis_id,
						   NCS_PHY_SLOT_ID *o_phy_slot_id, NCS_SUB_SLOT_ID *o_sub_slot_id);

#ifdef  __cplusplus
}
#endif

#endif
