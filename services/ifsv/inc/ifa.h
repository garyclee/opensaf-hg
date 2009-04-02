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


/* IFA.H: Master include file for all IFA *.C files */

#ifndef IFA_H
#define IFA_H

#include "ifsv.h"
#include "ifa_dl_api.h"
#include "ifa_db.h"
#include "ifa_mds.h"

#define m_IFA_GET_CB_HDL() (gl_ifa_cb_hdl)
#define m_IFA_SET_CB_HDL(hdl) (gl_ifa_cb_hdl=hdl)



/* IfA must have wait time more than IfNd, which has wait time as 
   IFSV_MDS_SYNC_TIME. The reason is that IfA gets timeout event before IfND
   so, IfNd reply doesn't reach IfA.*/
#define IFA_MDS_SYNC_TIME  (IFSV_MDS_SYNC_TIME+200)
#define IFA_VIP_MDS_SYNC_TIME  (IFSV_MDS_SYNC_TIME+200)

#endif


