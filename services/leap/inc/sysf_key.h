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


  MODULE NAME: SYSF_KEY.H


..............................................................................

  DESCRIPTION: Contains macros related to NCS_KEYs


******************************************************************************
*/

#ifndef SYSF_KEY_H
#define SYSF_KEY_H

/* TBD : This should map to a target service call that returns 
 ** the ptr to the relevant control block. This defn has been put only for compilation!
 **/
#define m_NCS_VALIDATE_KEY(key) ((key).val.hdl)

/*USED BY AKE */
/***************************************************************************
                    Subsystem Mapper 
****************************************************************************/
#define MAX_VR  5

/*This array is stors the subsystem key that are register by the AKE */
extern NCS_KEY subsys_list[NCS_SERVICE_ID_MAX][MAX_VR];

/* Macro for Registering the subsystem with AKE */
#define m_REGISTER_SUBSYSTEM(key, vr_id)   subsys_list[key.svc][vr_id] = key
      
/* Macro for getting the Subsystem ID register with AKE */
#define m_GET_SUBSYSTEM_KEY(svc_id, vr_id)  subsys_list[svc_id][vr_id]
 
#endif
