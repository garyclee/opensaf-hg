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
FILE NAME: IFD_DL_API.H
DESCRIPTION: API decleration for IFD library.
******************************************************************************/

#ifndef IFA_DL_API_H
#define IFA_DL_API_H

/*****************************************************************************
 * SE API used to initiate and create PWE's of IfSv component (IfA). 
 *****************************************************************************/
IFADLL_API uns32 ifa_lib_req(NCS_LIB_REQ_INFO * req_info);


#endif /* IFA_DL_API_H */


