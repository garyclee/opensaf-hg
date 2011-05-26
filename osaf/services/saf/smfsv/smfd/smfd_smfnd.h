/*      OpenSAF
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
 * Author(s): Ericsson AB
 *
 */

#ifndef SMFD_SMFND_H
#define SMFD_SMFND_H

/* ========================================================================
 *   INCLUDE FILES
 * ========================================================================
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <ncssysf_ipc.h>
#include <ncsgl_defs.h>
#include <saClm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 *   DEFINITIONS
 * ========================================================================
 */

/* ========================================================================
 *   TYPE DEFINITIONS
 * ========================================================================
 */
typedef struct smfd_smfnd_adest_invid_map{
        SaInvocationT                           inv_id;
        uint32_t                                   no_of_cbks;
	SYSF_MBX				*cbk_mbx;
        struct smfd_smfnd_adest_invid_map       *next_invid;
}SMFD_SMFND_ADEST_INVID_MAP;

/* ========================================================================
 *   DATA DECLARATIONS
 * ========================================================================
 */

	uint32_t smfnd_up(SaClmNodeIdT node_id, MDS_DEST smfnd_dest);
	uint32_t smfnd_down(SaClmNodeIdT node_id);
	MDS_DEST smfnd_dest_for_name(const char *nodeName);
	int smfnd_remote_cmd(const char *i_cmd, MDS_DEST i_smfnd_dest,
			     uint32_t i_timeout);

#ifdef __cplusplus
}
#endif
#endif				/* ifndef SMFD_SMFND_H */
