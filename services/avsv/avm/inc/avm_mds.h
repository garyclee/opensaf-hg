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
  AvM-MDS interaction related definitions.
  ..............................................................................

 
******************************************************************************
*/


#ifndef __AVM_MDS_H__
#define __AVM_MDS_H__

/* In Service upgrade support */
#define AVM_MDS_SUB_PART_VERSION  1

#define AVM_AVD_SUBPART_VER_MIN   1
#define AVM_AVD_SUBPART_VER_MAX   1

#define AVM_BAM_SUBPART_VER_MIN   1
#define AVM_BAM_SUBPART_VER_MAX   1

extern uns32 avm_mds_initialize(AVM_CB_T *cb);
extern uns32 avm_mds_finalize (AVM_CB_T *cb);
extern uns32
avm_mds_set_vdest_role(AVM_CB_T *cb, SaAmfHAStateT role);



extern uns32
avm_mds_msg_send(
                  AVM_CB_T                *cb,
                  AVM_AVD_MSG_T           *msg,
                  MDS_DEST                *dest,
                  MDS_SEND_PRIORITY_TYPE  prio
                );


#endif /* __AVM_MDS_H__ */
