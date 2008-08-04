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

  This file contains SRMSv related definitions.
  
******************************************************************************
*/

#ifndef AVND_SRM_H
#define AVND_SRM_H

/*****************************************************************************
 * structure for mapping SRMSv resource handle and component passive         *
 * monitoring record                                                         *
 *****************************************************************************/
                                                
typedef struct avnd_srm_req_tag {
   NCS_DB_LINK_LIST_NODE cb_dll_node; /* key is rsrc hdl */
   uns32                 rsrc_hdl;    /* srmsv rsrc handle */
   AVND_COMP_PM_REC      *pm_rec;     /* ptr to comp pm rec */
} AVND_SRM_REQ;



EXTERN_C uns32 gl_avnd_hdl;

EXTERN_C uns32 avnd_srm_reg (AVND_CB *);
EXTERN_C uns32 avnd_srm_unreg (AVND_CB *);
EXTERN_C void avnd_srm_callback(NCS_SRMSV_RSRC_CBK_INFO *);
EXTERN_C uns32 avnd_srm_start(AVND_CB *, AVND_COMP_PM_REC *, SaAisErrorT *);
EXTERN_C uns32 avnd_srm_stop(AVND_CB *, AVND_COMP_PM_REC *, SaAisErrorT *);
EXTERN_C void avnd_srm_process(uns32);

#endif /* !AVND_SRM_H */
