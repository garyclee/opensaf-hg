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


#define m_NCS_GET_PHYINFO_FROM_NODE_ID( i_node_id, o_chassis_id, o_phy_slot_id ,\
                                        o_sub_slot_id) ncs_get_phyinfo_from_node_id( i_node_id,\
                                        o_chassis_id , o_phy_slot_id, o_sub_slot_id)
/****************************************************************************
  Name          :  ncs_get_phyinfo_from_node_id

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
