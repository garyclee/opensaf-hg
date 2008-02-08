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

  MODULE NAME: VDS_EVT.H

$Header: 
..............................................................................

  DESCRIPTION: This file has the data structure definitions defined for VDS 
               specific event formats.

******************************************************************************/

#ifndef VDS_EVT_H
#define VDS_EVT_H

typedef enum vds_evt_type
{
   VDS_VDA_EVT_TYPE = 1,
   VDS_AMF_HEALTH_CHECK_EVT_TYPE,
   VDS_VDA_DESTROY_EVT_TYPE,
   VDS_MAX_EVT_TYPE
} VDS_EVT_TYPE;


typedef struct vds_vda_evt
{
   MDS_DEST dest;
   NCSVDA_INFO *vda_info;
   MDS_SYNC_SND_CTXT i_msg_ctxt; /* Valid only if "i_rsp_expected == TRUE */
}VDS_VDA_EVT;

typedef struct vds_evt
{
   struct vds_evt  *next;
   NCS_IPC_MSG   msg;
   VDS_EVT_TYPE  evt_type;
   union
   {
     VDS_VDA_EVT vda_evt_msg;
     SaInvocationT invocation;
   }evt_data;

} VDS_EVT;



/****************************************************************************
                     Function Prototypes
****************************************************************************/
EXTERN_C VDS_EVT *vds_evt_create(NCSCONTEXT arg,
                                 VDS_EVT_TYPE evt);
EXTERN_C void vds_evt_process(VDS_EVT *evt);
EXTERN_C void vds_evt_destroy(VDS_EVT *evt);

#endif /* VDS_EVT_H */

