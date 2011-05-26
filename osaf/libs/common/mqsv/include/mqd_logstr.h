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

  This module contains the extern declaration for registering
  MQSv-MQD's canned strings with Dtsv server.
  
******************************************************************************
*/

#ifndef MQD_LOGSTR_H
#define MQD_LOGSTR_H

#if (NCS_DTS == 1)
uint32_t mqd_log_ascii_reg(void);
void mqd_log_ascii_dereg(void);
uint32_t mqsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
#endif   /* (NCS_DTS == 1) */

#endif   /* MQD_LOGSTR_H */
