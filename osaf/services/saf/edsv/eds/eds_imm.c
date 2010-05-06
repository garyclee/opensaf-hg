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

#include "eds.h"

static SaVersionT imm_version = { EDSV_IMM_RELEASE_CODE,
	EDSV_IMM_MAJOR_VERSION,
	EDSV_IMM_MINOR_VERSION
};

extern struct ImmutilWrapperProfile immutilWrapperProfile;

static const SaImmOiImplementerNameT implementer_name = EDSV_IMM_IMPLEMENTER_NAME;

SaAisErrorT saImmOiRtAttrUpdateCallback(SaImmOiHandleT immOiHandle,
					const SaNameT *objectName, const SaImmAttrNameT *attributeNames)
{
	SaAisErrorT rc = SA_AIS_OK;
	SaImmAttrNameT attributeName;
	unsigned int i = 0, attr_count = 0;
	SaUint32T num_users, num_subscribers, num_publishers, num_ret_evts, num_lost_evts;
	EDS_CB *cb = NULL;
	EDS_WORKLIST *wp = NULL;

	SaImmAttrModificationT_2 attr_output[5];
	const SaImmAttrModificationT_2 *attrMods[6];

	SaImmAttrValueT attrUpdateValues1[] = { &num_users };
	SaImmAttrValueT attrUpdateValues2[] = { &num_subscribers };
	SaImmAttrValueT attrUpdateValues3[] = { &num_publishers };
	SaImmAttrValueT attrUpdateValues4[] = { &num_ret_evts };
	SaImmAttrValueT attrUpdateValues5[] = { &num_lost_evts };

	/* Get EDS CB Handle. */
	if (NULL == (cb = (NCSCONTEXT)ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl)))
		return (rc = SA_AIS_ERR_FAILED_OPERATION);	/* Log it */

	/* Look up the channel worklist DB to find the channel indicated by objectName */
	wp = get_channel_from_worklist(cb, *objectName);
	if (wp == NULL) {
		/* No Such Channel. Log it */
		rc = SA_AIS_ERR_FAILED_OPERATION;
		ncshm_give_hdl(gl_eds_hdl);
		return rc;
	}

	/* Walk through the attribute Name list */
	while ((attributeName = attributeNames[i]) != NULL) {

		if (strcmp(attributeName, "saEvtChannelNumOpeners") == 0) {
			num_users = wp->chan_row.num_users;
			attr_output[attr_count].modType = SA_IMM_ATTR_VALUES_REPLACE;
			attr_output[attr_count].modAttr.attrName = attributeName;
			attr_output[attr_count].modAttr.attrValueType = SA_IMM_ATTR_SAUINT32T;
			attr_output[attr_count].modAttr.attrValuesNumber = 1;
			attr_output[attr_count].modAttr.attrValues = attrUpdateValues1;
			attrMods[attr_count] = &attr_output[attr_count];
			++attr_count;
		} else if (strcmp(attributeName, "saEvtChannelNumSubscribers") == 0) {
			num_subscribers = wp->chan_row.num_subscribers;
			attr_output[attr_count].modType = SA_IMM_ATTR_VALUES_REPLACE;
			attr_output[attr_count].modAttr.attrName = attributeName;
			attr_output[attr_count].modAttr.attrValueType = SA_IMM_ATTR_SAUINT32T;
			attr_output[attr_count].modAttr.attrValuesNumber = 1;
			attr_output[attr_count].modAttr.attrValues = attrUpdateValues2;
			attrMods[attr_count] = &attr_output[attr_count];
			++attr_count;
		} else if (strcmp(attributeName, "saEvtChannelNumPublishers") == 0) {
			num_publishers = wp->chan_row.num_publishers;
			attr_output[attr_count].modType = SA_IMM_ATTR_VALUES_REPLACE;
			attr_output[attr_count].modAttr.attrName = attributeName;
			attr_output[attr_count].modAttr.attrValueType = SA_IMM_ATTR_SAUINT32T;
			attr_output[attr_count].modAttr.attrValuesNumber = 1;
			attr_output[attr_count].modAttr.attrValues = attrUpdateValues3;
			attrMods[attr_count] = &attr_output[attr_count];
			++attr_count;
		} else if (strcmp(attributeName, "saEvtChannelNumRetainedEvents") == 0) {
			num_ret_evts = wp->chan_row.num_ret_evts;
			attr_output[attr_count].modType = SA_IMM_ATTR_VALUES_REPLACE;
			attr_output[attr_count].modAttr.attrName = attributeName;
			attr_output[attr_count].modAttr.attrValueType = SA_IMM_ATTR_SAUINT32T;
			attr_output[attr_count].modAttr.attrValuesNumber = 1;
			attr_output[attr_count].modAttr.attrValues = attrUpdateValues4;
			attrMods[attr_count] = &attr_output[attr_count];
			++attr_count;
		} else if (strcmp(attributeName, "saEvtChannelLostEventsEventCount") == 0) {
			num_lost_evts = wp->chan_row.num_lost_evts;
			attr_output[attr_count].modType = SA_IMM_ATTR_VALUES_REPLACE;
			attr_output[attr_count].modAttr.attrName = attributeName;
			attr_output[attr_count].modAttr.attrValueType = SA_IMM_ATTR_SAUINT32T;
			attr_output[attr_count].modAttr.attrValuesNumber = 1;
			attr_output[attr_count].modAttr.attrValues = attrUpdateValues5;
			attrMods[attr_count] = &attr_output[attr_count];
			++attr_count;
		} else {
			rc = SA_AIS_ERR_FAILED_OPERATION;
			ncshm_give_hdl(gl_eds_hdl);
			return rc;
		}
		++i;
	}			/*End while attributesNames() */
	attrMods[attr_count] = NULL;
	rc = saImmOiRtObjectUpdate_2(immOiHandle, objectName, attrMods);

	ncshm_give_hdl(gl_eds_hdl);
	return rc;
}

SaImmOiCallbacksT_2 oi_cbks = {
	.saImmOiAdminOperationCallback = NULL,
	.saImmOiCcbAbortCallback = NULL,
	.saImmOiCcbApplyCallback = NULL,
	.saImmOiCcbCompletedCallback = NULL,
	.saImmOiCcbObjectCreateCallback = NULL,
	.saImmOiCcbObjectDeleteCallback = NULL,
	.saImmOiCcbObjectModifyCallback = NULL,
	.saImmOiRtAttrUpdateCallback = saImmOiRtAttrUpdateCallback
};

SaAisErrorT eds_imm_init(EDS_CB *cb)
{
	SaAisErrorT rc;
	immutilWrapperProfile.errorsAreFatal = 0;
	rc = immutil_saImmOiInitialize_2(&cb->immOiHandle, &oi_cbks, &imm_version);
	if (rc == SA_AIS_OK)
		immutil_saImmOiSelectionObjectGet(cb->immOiHandle, &cb->imm_sel_obj);

	return rc;
}

SaAisErrorT eds_imm_declare_implementer(SaImmOiHandleT immOiHandle)
{
	return (immutil_saImmOiImplementerSet(immOiHandle, implementer_name));
}

EDS_WORKLIST *get_channel_from_worklist(EDS_CB *cb, SaNameT chan_name)
{
	EDS_WORKLIST *wp = NULL;

	wp = cb->eds_work_list;
	if (wp == NULL)
		return wp;	/*No channels yet */

	/* Loop through the list for a matching channel name */

	while (wp) {
		if (wp->cname_len == chan_name.length) {	/* Compare channel length */
			/*   len = chan_name.length; */

			/* Compare channel name */
			if (memcmp(wp->cname, chan_name.value, chan_name.length) == 0)
				return wp;	/* match found */
		}
		wp = wp->next;
	}			/*End while */

	/* if we reached here, no channel by that name */
	return ((EDS_WORKLIST *)NULL);
}	/*End get_channel_from_worklist */
