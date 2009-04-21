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
 */

#ifndef PXY_PXD_H
#define PXY_PXD_H

#define PXY_PXD_NUM_OF_PROXIED_COMP   4

/*############################################################################
                            Global Variables
############################################################################*/

typedef struct pxy_pxd_comp_info
{
   SaNameT         compName;        /* Component name */
   SaNameT         csi_compName;    /* CSI name */
   SaAmfHandleT    amfHandle;       /* AMF handle obtained during AMF initialisation */
   SaAmfHAStateT   haState;         /* Current state of Proxy */
   SaAmfHandleT    amf_hdl;
   SaAmfHealthcheckKeyT healthcheck_key;
   NCS_BOOL        health_start;
   NCS_BOOL        health_start_comp_inv;
   NCS_BOOL        csi_assigned;
   NCS_BOOL        reg_pxied_comp;
} PXY_PXD_COMP_INFO;

typedef struct pxy_pxd_cb
{
   PXY_PXD_COMP_INFO  pxy_info; 
   PXY_PXD_COMP_INFO  pxd_info[PXY_PXD_NUM_OF_PROXIED_COMP]; 
   NCS_BOOL           unreg_pxied_comp;
}PXY_PXD_CB;

PXY_PXD_CB  pxy_pxd_cb;

/* AvSv AMF interface task handle */
NCSCONTEXT gl_amf_task_hdl = 0;

/* Canned strings for HA State */
const char *ha_state_str[] =
{
   "None",
   "Active",    /* SA_AMF_HA_ACTIVE       */
   "Standby",   /* SA_AMF_HA_STANDBY      */
   "Quiesced",  /* SA_AMF_HA_QUIESCED     */
   "Quiescing"  /* SA_AMF_HA_QUIESCING    */
};

/* Canned strings for CSI Flags */
const char *csi_flag_str[] =
{
   "None",
   "Add One",    /* SA_AMF_CSI_ADD_ONE    */
   "Target One", /* SA_AMF_CSI_TARGET_ONE */
   "None",
   "Target All", /* SA_AMF_CSI_TARGET_ALL */
};

/* Canned strings for Transition Descriptor */
const char *trans_desc_str[] =
{
   "None",
   "New Assign",   /* SA_AMF_CSI_NEW_ASSIGN   */
   "Quiesced",     /* SA_AMF_CSI_QUIESCED     */
   "Not Quiesced", /* SA_AMF_CSI_NOT_QUIESCED */
   "Still Active"  /* SA_AMF_CSI_STILL_ACTIVE */
};

/* Canned strings for Protection Group Change */
const char *pg_change_str[] =
{
   "None",
   "No Change",    /* SA_AMF_PROTECTION_GROUP_NO_CHANGE    */
   "Added",        /* SA_AMF_PROTECTION_GROUP_ADDED        */
   "Removed",      /* SA_AMF_PROTECTION_GROUP_REMOVED      */
   "State Change"  /* SA_AMF_PROTECTION_GROUP_STATE_CHANGE */
};

/*############################################################################
                            Macro Definitions
############################################################################*/

/* AMF interface task related parameters  */
#define PXY_PXD_TASK_PRIORITY   (5)
#define PXY_PXD_STACKSIZE       NCS_STACKSIZE_HUGE

/* Macro to retrieve the AMF version */
#define m_PXY_PXD_VER_GET(ver) \
{ \
   ver.releaseCode = 'B'; \
   ver.majorVersion = 0x01; \
   ver.minorVersion = 0x01; \
};


/*############################################################################
                       Extern Function Decalarations
############################################################################*/

/* Top level routine to start AMF Interface task */
extern uns32 pxy_pxd_init(void);

#endif
