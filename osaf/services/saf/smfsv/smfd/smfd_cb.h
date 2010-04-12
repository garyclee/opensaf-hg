/*       OpenSAF
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

#ifndef SMFD_CB_H
#define SMFD_CB_H

#include <saImmOm.h>
#include <saImmOi.h>

/* Default HA state assigned locally during smfd initialization */
#define SMFD_HA_INIT_STATE SA_AMF_HA_STANDBY

#define SMFD_MDS_PVT_SUBPART_VERSION 1

typedef struct smfd_cb {
	SYSF_MBX mbx;		                    /* SMFD mailbox                           */
	V_DEST_RL mds_role;      	            /* Current MDS role - ACTIVE/STANDBY      */
	uns32 mds_handle;
	MDS_DEST mds_dest;	                    /* My destination in MDS                  */
	SaVersionT smf_version;	                    /* The version currently supported        */
	SaNameT comp_name;	                    /* Components's name SMFD                 */
	SaAmfHandleT amf_hdl;	                    /* AMF handle, obtained thru AMF init     */
	SaInvocationT amf_invocation_id;            /* AMF InvocationID - needed to handle Quiesed state */
	NCS_BOOL is_quisced_set;
	SaSelectionObjectT amfSelectionObject;      /* Selection Object to wait for amf events          */
	SaImmOiHandleT campaignOiHandle;            /* IMM Campaign OI handle                           */
	SaSelectionObjectT campaignSelectionObject; /* Selection Object to wait for campaign IMM events */
	SaAmfHAStateT ha_state;	                    /* present AMF HA state of the component            */
	NCS_SEL_OBJ usr1_sel_obj;                   /* Selection object for USR1 signal events          */
	MDS_DEST smfnd_dests[NCS_MAX_SLOTS];        /* destinations for all smfnd   */
	uns32 nid_started;	                    /* Started by NID or AMF        */
	char *backupCreateCmd;	                    /* Backup create cmd string     */
	char *bundleCheckCmd;	                    /* Bundle check cmd string      */
	char *nodeCheckCmd;	                    /* Node check cmd string        */
	char *repositoryCheckCmd;                   /* Repository check cmd string  */
	char *clusterRebootCmd;	                    /* Cluster reboot cmd string    */
	SaTimeT adminOpTimeout;                     /* Timeout for admin operations */
	SaTimeT cliTimeout;	                    /* Timeout for cli commands     */
	SaTimeT rebootTimeout;	                    /* Timeout for reboot to finish */
	char *nodeBundleActCmd;	                    /* Command for activation of bundles on a node */
} smfd_cb_t;

#ifdef __cplusplus
extern "C" {
#endif

	extern uns32 smfd_cb_init(smfd_cb_t *);

#ifdef __cplusplus
}
#endif
#endif
