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
  AvD-MDS interaction related definitions.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_MDS_H
#define AVD_MDS_H

/* In Service upgrade support */
#define AVD_MDS_SUB_PART_VERSION   2

#define AVD_AVND_SUBPART_VER_MIN   1
#define AVD_AVND_SUBPART_VER_MAX   2

#define AVD_BAM_SUBPART_VER_MIN    1
#define AVD_BAM_SUBPART_VER_MAX    1

#define AVD_AVM_SUBPART_VER_MIN    1
#define AVD_AVM_SUBPART_VER_MAX    1

#define AVD_AVD_SUBPART_VER_MIN    1
#define AVD_AVD_SUBPART_VER_MAX    1

/* Message format versions */
#define AVD_AVD_MSG_FMT_VER_1    1

EXTERN_C uns32 avd_mds_reg_def(struct cl_cb_tag *cb);
EXTERN_C uns32 avd_mds_set_vdest_role(struct cl_cb_tag *cb, SaAmfHAStateT role);
EXTERN_C uns32 avd_mds_reg(struct cl_cb_tag *cb);
EXTERN_C void avd_mds_unreg(struct cl_cb_tag *cb);

EXTERN_C uns32 avd_mds_cbk(struct ncsmds_callback_info *info);
EXTERN_C uns32 avd_avnd_mds_send(struct cl_cb_tag *cb, AVD_AVND *nd_node, AVD_DND_MSG *snd_msg);

#endif
