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
 * Author(s): Ericsson AB
 *
 */

#include <ncsencdec_pub.h>
#include "ntfsv_enc_dec.h"
#include "ntfsv_mem.h"


static void print_object_attribute(SaNtfAttributeT *input)
{
	(void)TRACE_2("Attr ID: %d\n", (int)input->attributeId);
	(void)TRACE_2("Attr Type: %d\n", (int)input->attributeType);
	(void)TRACE_2("Attr Value: %d\n", (int)input->attributeValue.int32Val);
}

void ntfsv_print_object_attributes(SaNtfAttributeT *objectAttributes, SaUint16T numAttributes)
{
	int i;
	(void)TRACE_2("numAttr: %d\n", (int)numAttributes);

	for (i = 0; i < numAttributes; i++) {
		print_object_attribute(&objectAttributes[i]);
	}
}

static uns32 encodeSaNtfValueT(NCS_UBAID *uba, uint8_t *p8, uns32 total_bytes, SaNtfValueTypeT attributeType,
			       SaNtfValueT *ntfAttr)
{

	switch (attributeType) {
	case SA_NTF_VALUE_UINT8:
		p8 = ncs_enc_reserve_space(uba, 1);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_8bit(&p8, ntfAttr->uint8Val);
		ncs_enc_claim_space(uba, 1);
		total_bytes += 1;
		break;
	case SA_NTF_VALUE_INT8:
		p8 = ncs_enc_reserve_space(uba, 1);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_8bit(&p8, ntfAttr->int8Val);
		ncs_enc_claim_space(uba, 1);
		total_bytes += 1;
		break;
	case SA_NTF_VALUE_UINT16:
		p8 = ncs_enc_reserve_space(uba, 2);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_16bit(&p8, ntfAttr->uint16Val);
		ncs_enc_claim_space(uba, 2);
		total_bytes += 2;
		break;
	case SA_NTF_VALUE_INT16:
		p8 = ncs_enc_reserve_space(uba, 2);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_16bit(&p8, ntfAttr->int16Val);
		ncs_enc_claim_space(uba, 2);
		total_bytes += 2;
		break;
	case SA_NTF_VALUE_UINT32:
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_32bit(&p8, ntfAttr->uint32Val);
		ncs_enc_claim_space(uba, 4);
		total_bytes += 4;
		break;
	case SA_NTF_VALUE_INT32:
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_32bit(&p8, ntfAttr->int32Val);
		ncs_enc_claim_space(uba, 4);
		total_bytes += 4;
		break;
	case SA_NTF_VALUE_FLOAT:
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_32bit(&p8, (uns32)ntfAttr->floatVal);
		ncs_enc_claim_space(uba, 4);
		total_bytes += 4;
		break;
	case SA_NTF_VALUE_UINT64:
		p8 = ncs_enc_reserve_space(uba, 8);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_64bit(&p8, ntfAttr->uint64Val);
		ncs_enc_claim_space(uba, 8);
		total_bytes += 8;
		break;
	case SA_NTF_VALUE_INT64:
		p8 = ncs_enc_reserve_space(uba, 8);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_64bit(&p8, ntfAttr->int64Val);
		ncs_enc_claim_space(uba, 8);
		total_bytes += 8;
		break;
	case SA_NTF_VALUE_DOUBLE:
		p8 = ncs_enc_reserve_space(uba, 8);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_64bit(&p8, (SaDoubleT)ntfAttr->doubleVal);
		ncs_enc_claim_space(uba, 8);
		total_bytes += 8;
		break;
	case SA_NTF_VALUE_LDAP_NAME:
	case SA_NTF_VALUE_STRING:
	case SA_NTF_VALUE_IPADDRESS:
	case SA_NTF_VALUE_BINARY:
		TRACE_2("enc type: %d offset: %hu size: %hu",
			(int)attributeType, ntfAttr->ptrVal.dataOffset, ntfAttr->ptrVal.dataSize);
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_16bit(&p8, ntfAttr->ptrVal.dataOffset);
		ncs_encode_16bit(&p8, ntfAttr->ptrVal.dataSize);
		ncs_enc_claim_space(uba, 4);
		total_bytes += 4;
		break;
	case SA_NTF_VALUE_ARRAY:
		TRACE_2("enc type SA_NTF_VALUE_ARRAY: %d offset: %hu nelem: %hu esize: %hu",
			(int)attributeType,
			ntfAttr->arrayVal.arrayOffset, ntfAttr->arrayVal.numElements, ntfAttr->arrayVal.elementSize);
		p8 = ncs_enc_reserve_space(uba, 6);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_16bit(&p8, ntfAttr->arrayVal.arrayOffset);
		ncs_encode_16bit(&p8, ntfAttr->arrayVal.numElements);
		ncs_encode_16bit(&p8, ntfAttr->arrayVal.elementSize);
		ncs_enc_claim_space(uba, 6);
		total_bytes += 6;
		break;
	default:
		TRACE_2("attributeType %d not valid", (int)attributeType);
		return 0;
	}
	return total_bytes;
}

static uns32 encodeSaNtfAttributeChangeT(NCS_UBAID *uba, uint8_t *p8, uns32 total_bytes, SaNtfAttributeChangeT *ntfAttr)
{
	p8 = ncs_enc_reserve_space(uba, 8);
	if (!p8) {
		TRACE("p8 NULL!!!");
		return 0;
	}
	ncs_encode_16bit(&p8, ntfAttr->attributeId);
	ncs_encode_16bit(&p8, (uint16_t)ntfAttr->attributeType);
	ncs_encode_32bit(&p8, ntfAttr->oldAttributePresent);
	ncs_enc_claim_space(uba, 8);
	total_bytes += 8;
	if (ntfAttr->oldAttributePresent) {
		total_bytes += encodeSaNtfValueT(uba,
						 p8, total_bytes, ntfAttr->attributeType, &ntfAttr->oldAttributeValue);
	}
	total_bytes += encodeSaNtfValueT(uba, p8, total_bytes, ntfAttr->attributeType, &ntfAttr->newAttributeValue);
	return total_bytes;
}

static uns32 encodeSaNtfAttribute(NCS_UBAID *uba, uint8_t *p8, uns32 total_bytes, SaNtfAttributeT *ntfAttr)
{
	p8 = ncs_enc_reserve_space(uba, 4);
	if (!p8) {
		TRACE("p8 NULL!!!");
		return 0;
	}
	ncs_encode_16bit(&p8, ntfAttr->attributeId);
	ncs_encode_16bit(&p8, (uint16_t)ntfAttr->attributeType);
	ncs_enc_claim_space(uba, 4);
	total_bytes += 4;
	total_bytes += encodeSaNtfValueT(uba, p8, total_bytes, ntfAttr->attributeType, &ntfAttr->attributeValue);
	return total_bytes;
}

static uns32 decodeNtfValueT(NCS_UBAID *uba, uns32 total_bytes, SaNtfValueTypeT attributeType, SaNtfValueT *ntfAttr)
{
	uint8_t *p8;
	uint8_t local_data[8];

	switch (attributeType) {
	case SA_NTF_VALUE_UINT8:
		p8 = ncs_dec_flatten_space(uba, local_data, 1);
		ntfAttr->uint8Val = ncs_decode_8bit(&p8);
		ncs_dec_skip_space(uba, 1);
		total_bytes += 1;
		break;
	case SA_NTF_VALUE_INT8:
		p8 = ncs_dec_flatten_space(uba, local_data, 1);
		ntfAttr->int8Val = ncs_decode_8bit(&p8);
		ncs_dec_skip_space(uba, 1);
		total_bytes += 1;
		break;
	case SA_NTF_VALUE_UINT16:
		p8 = ncs_dec_flatten_space(uba, local_data, 2);
		ntfAttr->uint16Val = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 2);
		total_bytes += 2;
		break;
	case SA_NTF_VALUE_INT16:
		p8 = ncs_dec_flatten_space(uba, local_data, 2);
		ntfAttr->int16Val = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 2);
		total_bytes += 2;
		break;
	case SA_NTF_VALUE_UINT32:
		p8 = ncs_dec_flatten_space(uba, local_data, 4);
		ntfAttr->uint32Val = ncs_decode_32bit(&p8);
		ncs_dec_skip_space(uba, 4);
		total_bytes += 4;
		break;
	case SA_NTF_VALUE_INT32:
		p8 = ncs_dec_flatten_space(uba, local_data, 4);
		ntfAttr->int32Val = ncs_decode_32bit(&p8);
		ncs_dec_skip_space(uba, 4);
		total_bytes += 4;
		break;
	case SA_NTF_VALUE_FLOAT:
		p8 = ncs_dec_flatten_space(uba, local_data, 4);
		ntfAttr->floatVal = (SaFloatT)ncs_decode_32bit(&p8);
		ncs_dec_skip_space(uba, 4);
		total_bytes += 4;
		break;
	case SA_NTF_VALUE_UINT64:
		p8 = ncs_dec_flatten_space(uba, local_data, 8);
		ntfAttr->uint64Val = ncs_decode_64bit(&p8);
		ncs_dec_skip_space(uba, 8);
		total_bytes += 8;
		break;
	case SA_NTF_VALUE_INT64:
		p8 = ncs_dec_flatten_space(uba, local_data, 8);
		ntfAttr->int64Val = ncs_decode_64bit(&p8);
		ncs_dec_skip_space(uba, 8);
		total_bytes += 8;
		break;
	case SA_NTF_VALUE_DOUBLE:
		p8 = ncs_dec_flatten_space(uba, local_data, 8);
		ntfAttr->doubleVal = (SaDoubleT)ncs_decode_64bit(&p8);
		ncs_dec_skip_space(uba, 8);
		total_bytes += 8;
		break;
	case SA_NTF_VALUE_LDAP_NAME:
	case SA_NTF_VALUE_STRING:
	case SA_NTF_VALUE_IPADDRESS:
	case SA_NTF_VALUE_BINARY:
		p8 = ncs_dec_flatten_space(uba, local_data, 4);
		ntfAttr->ptrVal.dataOffset = ncs_decode_16bit(&p8);
		ntfAttr->ptrVal.dataSize = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 4);
		total_bytes += 4;
		TRACE_2("dec type: %d offset: %hu size: %hu",
			(int)attributeType, ntfAttr->ptrVal.dataOffset, ntfAttr->ptrVal.dataSize);
		break;
	case SA_NTF_VALUE_ARRAY:
		p8 = ncs_dec_flatten_space(uba, local_data, 6);
		ntfAttr->arrayVal.arrayOffset = ncs_decode_16bit(&p8);
		ntfAttr->arrayVal.numElements = ncs_decode_16bit(&p8);
		ntfAttr->arrayVal.elementSize = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 6);
		total_bytes += 6;
		TRACE_2("enc type: %d offset: %hu nelem: %hu esize: %hu",
			(int)attributeType,
			ntfAttr->arrayVal.arrayOffset, ntfAttr->arrayVal.numElements, ntfAttr->arrayVal.elementSize);
		break;
	default:
		TRACE_2("attributeType %d not valid", (int)attributeType);
	}
	return total_bytes;
}

static uns32 decodeSaNtfAttributeChangeT(NCS_UBAID *uba, uns32 total_bytes, SaNtfAttributeChangeT *ntfAttr)
{
	uint8_t *p8;
	uint8_t local_data[8];
	p8 = ncs_dec_flatten_space(uba, local_data, 8);
	ntfAttr->attributeId = ncs_decode_16bit(&p8);
	ntfAttr->attributeType = ncs_decode_16bit(&p8);
	ntfAttr->oldAttributePresent = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 8);
	total_bytes += 8;
	if (ntfAttr->oldAttributePresent) {
		total_bytes = decodeNtfValueT(uba, total_bytes, ntfAttr->attributeType, &ntfAttr->oldAttributeValue);
	}
	total_bytes = decodeNtfValueT(uba, total_bytes, ntfAttr->attributeType, &ntfAttr->newAttributeValue);
	return total_bytes;
}

static uns32 decodeSaNtfAttribute(NCS_UBAID *uba, uns32 total_bytes, SaNtfAttributeT *ntfAttr)
{
	uint8_t *p8;
	uint8_t local_data[4];

	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	ntfAttr->attributeId = ncs_decode_16bit(&p8);
	ntfAttr->attributeType = ncs_decode_16bit(&p8);
	ncs_dec_skip_space(uba, 4);
	total_bytes += 4;

	total_bytes = decodeNtfValueT(uba, total_bytes, ntfAttr->attributeType, &ntfAttr->attributeValue);
	return total_bytes;
}

static uns32 encodeSaNameT(NCS_UBAID *uba, uint8_t *p8, uns32 total_bytes, SaNameT *name)
{
	p8 = ncs_enc_reserve_space(uba, 2);
	if (!p8) {
		TRACE("p8 NULL!!!");
		return 0;
	}
	if (name->length > SA_MAX_NAME_LENGTH) {
		LOG_ER("SaNameT length too long %hd", name->length);
		assert(0);
	}
	ncs_encode_16bit(&p8, name->length);
	ncs_enc_claim_space(uba, 2);
	total_bytes += 2;
	ncs_encode_n_octets_in_uba(uba, name->value, (uns32)name->length);
	total_bytes += (uns32)name->length;
	return total_bytes;
}

static uns32 decodeSaNameT(NCS_UBAID *uba, uint8_t *p8, uns32 total_bytes, SaNameT *name)
{
	uint8_t local_data[2];
	p8 = ncs_dec_flatten_space(uba, local_data, 2);
	name->length = ncs_decode_16bit(&p8);
	if (name->length > SA_MAX_NAME_LENGTH) {
		LOG_ER("SaNameT length too long: %hd", name->length);
		/* this should not happen */
		assert(0);
	}
	ncs_dec_skip_space(uba, 2);
	total_bytes += 2;
	ncs_decode_n_octets_from_uba(uba, name->value, (uns32)name->length);
	total_bytes += name->length;
	return total_bytes;
}

static uns32 ntfsv_enc_not_header(NCS_UBAID *uba, SaNtfNotificationHeaderT *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;
	int i;

	/* additional text */
	p8 = ncs_enc_reserve_space(uba, 10);
	if (!p8) {
		goto error_done;
	}
	ncs_encode_16bit(&p8, param->numCorrelatedNotifications);
	ncs_encode_16bit(&p8, param->lengthAdditionalText);
	ncs_encode_16bit(&p8, param->numAdditionalInfo);
	ncs_encode_32bit(&p8, *param->eventType);
	ncs_enc_claim_space(uba, 10);
	total_bytes += 10;

	total_bytes = encodeSaNameT(uba, p8, total_bytes, param->notificationObject);
	if (total_bytes == 0) {
		goto error_done;
	}

	total_bytes = encodeSaNameT(uba, p8, total_bytes, param->notifyingObject);
	if (total_bytes == 0) {
		goto error_done;
	}

	p8 = ncs_enc_reserve_space(uba, 8);
	if (!p8) {
		goto error_done;
	}
	ncs_encode_32bit(&p8, param->notificationClassId->vendorId);
	ncs_encode_16bit(&p8, param->notificationClassId->majorId);
	ncs_encode_16bit(&p8, param->notificationClassId->minorId);
	ncs_enc_claim_space(uba, 8);
	total_bytes += 8;

	p8 = ncs_enc_reserve_space(uba, 8);
	if (!p8) {
		goto error_done;
	}
	ncs_encode_64bit(&p8, *param->eventTime);
	ncs_enc_claim_space(uba, 8);
	total_bytes += 8;

	p8 = ncs_enc_reserve_space(uba, 8);
	if (!p8) {
		goto error_done;
	}
	ncs_encode_64bit(&p8, *param->notificationId);
	ncs_enc_claim_space(uba, 8);
	total_bytes += 8;
	TRACE_2("ENC: Not ptr %p: %llu", param->notificationId, *param->notificationId);

	ncs_encode_n_octets_in_uba(uba, (uint8_t *)param->additionalText, (uns32)param->lengthAdditionalText);
	total_bytes += (uns32)param->lengthAdditionalText;

	for (i = 0; i < param->numCorrelatedNotifications; i++) {
		p8 = ncs_enc_reserve_space(uba, 8);
		if (!p8)
			goto error_done;
		ncs_encode_64bit(&p8, param->correlatedNotifications[i]);
		ncs_enc_claim_space(uba, 8);
		total_bytes += 8;
	}

	TRACE_2("enc : param->numAdditionalInfo %hu\n", param->numAdditionalInfo);
	for (i = 0; i < param->numAdditionalInfo; i++) {
		TRACE_2("enc additionalInfo Type: %d\n", param->additionalInfo[i].infoType);
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8)
			goto error_done;
		ncs_encode_16bit(&p8, param->additionalInfo[i].infoId);
		ncs_encode_16bit(&p8, param->additionalInfo[i].infoType);
		ncs_enc_claim_space(uba, 4);
		total_bytes += 4;
		total_bytes += encodeSaNtfValueT(uba,
						 p8,
						 total_bytes,
						 param->additionalInfo[i].infoType,
						 &param->additionalInfo[i].infoValue);
	}
	return total_bytes;
 error_done:
	TRACE("p8 NULL");
	return 0;
}

uns32 ntfsv_enc_not_msg(NCS_UBAID *uba, ntfsv_send_not_req_t *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;

	TRACE_ENTER();
	assert(uba != NULL);

	/** encode the contents **/
	p8 = ncs_enc_reserve_space(uba, 8);
	if (!p8) {
		TRACE("p8 NULL!!!");
		return 0;
	}
	ncs_encode_32bit(&p8, param->notificationType);
	ncs_encode_32bit(&p8, param->client_id);
	ncs_enc_claim_space(uba, 8);
	total_bytes += 8;

	switch (param->notificationType) {
		int i;
	case SA_NTF_TYPE_ALARM:
		TRACE("enc SA_NTF_TYPE_ALARM");
		ntfsv_enc_not_header(uba, &param->notification.alarm.notificationHeader);
		p8 = ncs_enc_reserve_space(uba, 6);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_16bit(&p8, param->notification.alarm.numSpecificProblems);
		ncs_encode_16bit(&p8, param->notification.alarm.numMonitoredAttributes);
		ncs_encode_16bit(&p8, param->notification.alarm.numProposedRepairActions);
		ncs_enc_claim_space(uba, 6);
		total_bytes += 6;
		p8 = ncs_enc_reserve_space(uba, 10);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_32bit(&p8, *param->notification.alarm.probableCause);
		ncs_encode_32bit(&p8, *param->notification.alarm.perceivedSeverity);
		ncs_encode_16bit(&p8, *param->notification.alarm.trend);
		ncs_enc_claim_space(uba, 10);
		total_bytes += 10;
		p8 = ncs_enc_reserve_space(uba, 12);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_16bit(&p8, param->notification.alarm.thresholdInformation->thresholdId);
		ncs_encode_16bit(&p8, param->notification.alarm.thresholdInformation->thresholdValueType);
		ncs_encode_64bit(&p8, param->notification.alarm.thresholdInformation->armTime);
		ncs_enc_claim_space(uba, 12);
		total_bytes += 12;
		TRACE("thresholdInformation->thresholdValueType %hu",
		      param->notification.alarm.thresholdInformation->thresholdValueType);
		total_bytes += encodeSaNtfValueT(uba,
						 p8,
						 total_bytes,
						 param->notification.alarm.thresholdInformation->thresholdValueType,
						 &param->notification.alarm.thresholdInformation->thresholdValue);
		total_bytes += encodeSaNtfValueT(uba,
						 p8,
						 total_bytes,
						 param->notification.alarm.thresholdInformation->thresholdValueType,
						 &param->notification.alarm.thresholdInformation->thresholdHysteresis);
		total_bytes += encodeSaNtfValueT(uba,
						 p8,
						 total_bytes,
						 param->notification.alarm.thresholdInformation->thresholdValueType,
						 &param->notification.alarm.thresholdInformation->observedValue);

		TRACE("thresholdInformation->numSpecificProblems %hu", param->notification.alarm.numSpecificProblems);
		for (i = 0; i < param->notification.alarm.numSpecificProblems; i++) {
			p8 = ncs_enc_reserve_space(uba, 12);
			if (!p8) {
				TRACE("p8 NULL!!!");
				return 0;
			}
			ncs_encode_16bit(&p8, param->notification.alarm.specificProblems[i].problemId);
			ncs_encode_16bit(&p8, param->notification.alarm.specificProblems[i].problemClassId.majorId);
			ncs_encode_16bit(&p8, param->notification.alarm.specificProblems[i].problemClassId.minorId);
			ncs_encode_32bit(&p8, param->notification.alarm.specificProblems[i].problemClassId.vendorId);
			ncs_encode_16bit(&p8, param->notification.alarm.specificProblems[i].problemType);
			ncs_enc_claim_space(uba, 12);
			total_bytes += 12;
			total_bytes += encodeSaNtfValueT(uba,
							 p8,
							 total_bytes,
							 param->notification.alarm.specificProblems[i].problemType,
							 &param->notification.alarm.specificProblems[i].problemValue);
		}
		TRACE("thresholdInformation->numMonitoredAttributes %hu",
		      param->notification.alarm.numMonitoredAttributes);
		for (i = 0; i < param->notification.alarm.numMonitoredAttributes; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8) {
				TRACE("p8 NULL!!!");
				return 0;
			}
			ncs_encode_16bit(&p8, param->notification.alarm.monitoredAttributes[i].attributeId);
			ncs_encode_16bit(&p8, param->notification.alarm.monitoredAttributes[i].attributeType);
			ncs_enc_claim_space(uba, 4);
			total_bytes += 4;
			total_bytes += encodeSaNtfValueT(uba,
							 p8,
							 total_bytes,
							 param->notification.alarm.monitoredAttributes[i].attributeType,
							 &param->notification.alarm.monitoredAttributes[i].
							 attributeValue);
		}
		TRACE("thresholdInformation->numProposedRepairActions %hu",
		      param->notification.alarm.numProposedRepairActions);
		for (i = 0; i < param->notification.alarm.numProposedRepairActions; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8) {
				TRACE("p8 NULL!!!");
				return 0;
			}
			ncs_encode_16bit(&p8, param->notification.alarm.proposedRepairActions[i].actionId);
			ncs_encode_16bit(&p8, param->notification.alarm.proposedRepairActions[i].actionValueType);
			ncs_enc_claim_space(uba, 4);
			total_bytes += 4;
			total_bytes += encodeSaNtfValueT(uba,
							 p8,
							 total_bytes,
							 param->notification.alarm.proposedRepairActions[i].
							 actionValueType,
							 &param->notification.alarm.proposedRepairActions[i].
							 actionValue);
		}
		break;
	case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
		TRACE("enc SA_NTF_TYPE_OBJECT_CREATE_DELETE");
		ntfsv_enc_not_header(uba, &param->notification.objectCreateDelete.notificationHeader);
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_16bit(&p8, param->notification.objectCreateDelete.numAttributes);
		ncs_encode_16bit(&p8, *param->notification.objectCreateDelete.sourceIndicator);
		TRACE_2("sourceIndicator %d", (int)*param->notification.objectCreateDelete.sourceIndicator);
		ncs_enc_claim_space(uba, 4);
		total_bytes += 4;
		ntfsv_print_object_attributes(param->notification.objectCreateDelete.objectAttributes,
					      param->notification.objectCreateDelete.numAttributes);

		for (i = 0; i < param->notification.objectCreateDelete.numAttributes; i++) {
			total_bytes = encodeSaNtfAttribute(uba,
							   p8,
							   total_bytes,
							   &param->notification.objectCreateDelete.objectAttributes[i]);
			if (total_bytes == 0) {
				goto error_done;
			}
		}
		break;
	case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
		TRACE("enc SA_NTF_TYPE_ATTRIBUTE_CHANGE");
		ntfsv_enc_not_header(uba, &param->notification.attributeChange.notificationHeader);
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_16bit(&p8, param->notification.attributeChange.numAttributes);
		ncs_encode_16bit(&p8, *param->notification.attributeChange.sourceIndicator);
		ncs_enc_claim_space(uba, 4);
		total_bytes += 4;
		for (i = 0; i < param->notification.attributeChange.numAttributes; i++) {
			total_bytes =
			    encodeSaNtfAttributeChangeT(uba,
							p8,
							total_bytes,
							&param->notification.attributeChange.changedAttributes[i]);
		}
		break;
	case SA_NTF_TYPE_STATE_CHANGE:
		TRACE("enc SA_NTF_TYPE_STATE_CHANGE");
		ntfsv_enc_not_header(uba, &param->notification.stateChange.notificationHeader);
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		TRACE("enc numStCh: %d", (int)param->notification.stateChange.numStateChanges);
		ncs_encode_16bit(&p8, param->notification.stateChange.numStateChanges);
		ncs_encode_16bit(&p8, *param->notification.stateChange.sourceIndicator);
		ncs_enc_claim_space(uba, 4);
		total_bytes += 4;
		for (i = 0; i < param->notification.stateChange.numStateChanges; i++) {
			p8 = ncs_enc_reserve_space(uba, 6);
			if (!p8) {
				TRACE("p8 NULL!!!");
				return 0;
			}
			ncs_encode_16bit(&p8, param->notification.stateChange.changedStates[i].stateId);
			ncs_encode_32bit(&p8, param->notification.stateChange.changedStates[i].oldStatePresent);
			ncs_enc_claim_space(uba, 6);
			total_bytes += 6;
			if (param->notification.stateChange.changedStates[i].oldStatePresent) {
				p8 = ncs_enc_reserve_space(uba, 2);
				if (!p8) {
					TRACE("p8 NULL!!!");
					return 0;
				}
				ncs_encode_16bit(&p8, param->notification.stateChange.changedStates[i].oldState);
				ncs_enc_claim_space(uba, 2);
				total_bytes += 2;
			}
			p8 = ncs_enc_reserve_space(uba, 2);
			if (!p8) {
				TRACE("p8 NULL!!!");
				return 0;
			}
			ncs_encode_16bit(&p8, param->notification.stateChange.changedStates[i].newState);
			ncs_enc_claim_space(uba, 2);
			total_bytes += 2;
		}
		break;
	case SA_NTF_TYPE_SECURITY_ALARM:
		TRACE("enc SA_NTF_TYPE_SECURITY_ALARM");
		ntfsv_enc_not_header(uba, &param->notification.securityAlarm.notificationHeader);
		p8 = ncs_enc_reserve_space(uba, 10);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_32bit(&p8, *param->notification.securityAlarm.probableCause);
		ncs_encode_32bit(&p8, *param->notification.securityAlarm.severity);
		ncs_encode_16bit(&p8, param->notification.securityAlarm.securityAlarmDetector->valueType);
		ncs_enc_claim_space(uba, 10);
		TRACE_2("enc Security Alarm Detector Type: %d\n", param->notification.securityAlarm.securityAlarmDetector->valueType);
		total_bytes = encodeSaNtfValueT(uba, p8, 0, param->notification.securityAlarm.securityAlarmDetector->valueType, &param->notification.securityAlarm.securityAlarmDetector->value);
		if (!total_bytes) {
			return 0;
		}
		p8 = ncs_enc_reserve_space(uba, 2);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}		
		ncs_encode_16bit(&p8, param->notification.securityAlarm.serviceUser->valueType);
		ncs_enc_claim_space(uba, 2);
		total_bytes += 2;
		total_bytes += encodeSaNtfValueT(uba,
						 p8,
						 total_bytes,
						 param->notification.securityAlarm.serviceUser->valueType,
						 &param->notification.securityAlarm.serviceUser->value);
		p8 = ncs_enc_reserve_space(uba, 2);
		if (!p8) {
			TRACE("p8 NULL!!!");
			return 0;
		}
		ncs_encode_16bit(&p8, param->notification.securityAlarm.serviceProvider->valueType);
		ncs_enc_claim_space(uba, 2);
		total_bytes += 2;
		total_bytes += encodeSaNtfValueT(uba,
						 p8,
						 total_bytes,
						 param->notification.securityAlarm.serviceProvider->valueType,
						 &param->notification.securityAlarm.serviceProvider->value);

		break;

	default:
		TRACE("notificationType: %d not valid", (int)param->notificationType);
		return 0;
		break;
	}
	p8 = ncs_enc_reserve_space(uba, 4);
	if (!p8) {
		TRACE("p8 NULL!!!");
		return 0;
	}
	ncs_encode_32bit(&p8, param->variable_data.size);
	ncs_enc_claim_space(uba, 4);
	total_bytes += 4;
	if (param->variable_data.size)
		ncs_encode_n_octets_in_uba(uba, param->variable_data.p_base, param->variable_data.size);

 done:
	TRACE_LEAVE();
	return total_bytes;

 error_done:
	TRACE_1("encoding error");
	goto done;
}

uns32 ntfsv_enc_discard_msg(NCS_UBAID *uba, ntfsv_discarded_info_t *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;
	int i;
	
	TRACE_ENTER();
	assert(uba != NULL);
	/** encode the contents **/
	p8 = ncs_enc_reserve_space(uba, 8);
	if (!p8) {
		TRACE("p8 NULL!!!");
		return 0;
	}
	TRACE_3("t:%#x, nd: %u",
		param->notificationType,
		param->numberDiscarded); 
	ncs_encode_32bit(&p8, param->notificationType);
	ncs_encode_32bit(&p8, param->numberDiscarded);
	ncs_enc_claim_space(uba, 8);
	total_bytes += 8;
	for (i = 0; i < param->numberDiscarded; i++) {
		p8 = ncs_enc_reserve_space(uba, 8);
		if (!p8) {
			TRACE_1("encoding error");
			total_bytes = 0;			
			break;
		}
		ncs_encode_64bit(&p8, param->discardedNotificationIdentifiers[i]);
		ncs_enc_claim_space(uba, 8);
		total_bytes += 8;
	}
	TRACE_LEAVE();  
	return total_bytes;
}

static uns32 ntfsv_dec_not_header(NCS_UBAID *uba, SaNtfNotificationHeaderT *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;
	uint8_t local_data[8];
	int i;
	/* SaNtfNotificationHeaderT size params */
	SaUint16T numCorrelatedNotifications;
	SaUint16T lengthAdditionalText;
	SaUint16T numAdditionalInfo;
	TRACE_ENTER();

/* SaNtfNotificationHeaderT size params */
	p8 = ncs_dec_flatten_space(uba, local_data, 6);
	numCorrelatedNotifications = ncs_decode_16bit(&p8);
	lengthAdditionalText = ncs_decode_16bit(&p8);
	numAdditionalInfo = ncs_decode_16bit(&p8);
	ncs_dec_skip_space(uba, 6);
	total_bytes += 6;

	ntfsv_alloc_ntf_header(param, numCorrelatedNotifications, lengthAdditionalText, numAdditionalInfo);

	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	*param->eventType = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4);
	total_bytes += 4;
	total_bytes = decodeSaNameT(uba, p8, total_bytes, param->notificationObject);
	total_bytes = decodeSaNameT(uba, p8, total_bytes, param->notifyingObject);

	p8 = ncs_dec_flatten_space(uba, local_data, 8);
	param->notificationClassId->vendorId = ncs_decode_32bit(&p8);
	param->notificationClassId->majorId = ncs_decode_16bit(&p8);
	param->notificationClassId->minorId = ncs_decode_16bit(&p8);
	ncs_dec_skip_space(uba, 8);
	total_bytes += 8;

	p8 = ncs_dec_flatten_space(uba, local_data, 8);
	*param->eventTime = ncs_decode_64bit(&p8);
	ncs_dec_skip_space(uba, 8);
	total_bytes += 8;

	p8 = ncs_dec_flatten_space(uba, local_data, 8);
	*param->notificationId = ncs_decode_64bit(&p8);
	ncs_dec_skip_space(uba, 8);
	total_bytes += 8;

	if (param->lengthAdditionalText > 0) {
		/* dealloc in ntfs_evt.c: proc_send_not_msg */
		ncs_decode_n_octets_from_uba(uba, (uint8_t *)param->additionalText, (uns32)param->lengthAdditionalText);
		total_bytes += (uns32)param->lengthAdditionalText;
		param->additionalText[param->lengthAdditionalText - 1] = '\0';
	}

	for (i = 0; i < param->numCorrelatedNotifications; i++) {
		p8 = ncs_dec_flatten_space(uba, local_data, 8);
		param->correlatedNotifications[i] = ncs_decode_64bit(&p8);
		ncs_dec_skip_space(uba, 8);
		total_bytes += 8;
	}
	for (i = 0; i < param->numAdditionalInfo; i++) {
		p8 = ncs_dec_flatten_space(uba, local_data, 4);
		param->additionalInfo[i].infoId = ncs_decode_16bit(&p8);
		param->additionalInfo[i].infoType = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 4);
		total_bytes += 4;
		total_bytes = decodeNtfValueT(uba, total_bytes,
					      param->additionalInfo[i].infoType, &param->additionalInfo[i].infoValue);
		TRACE_2("dec additionalInfo Type: %d\n", param->additionalInfo[i].infoType);
		TRACE_2("dec additionaInfo Value int32: %d\n", param->additionalInfo[i].infoValue.int32Val);
	}
	TRACE_LEAVE();
	return total_bytes;
}

uns32 ntfsv_dec_not_msg(NCS_UBAID *uba, ntfsv_send_not_req_t *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;
	uint8_t local_data[16];

	TRACE_ENTER();

	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	param->notificationType = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4);
	total_bytes += 4;
	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	param->client_id = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4);
	total_bytes += 4;

	switch (param->notificationType) {
		int i;

	case SA_NTF_TYPE_ALARM:
		TRACE("dec SA_NTF_TYPE_ALARM");
		total_bytes += ntfsv_dec_not_header(uba, &param->notification.alarm.notificationHeader);
		p8 = ncs_dec_flatten_space(uba, local_data, 16);
		param->notification.alarm.numSpecificProblems = ncs_decode_16bit(&p8);
		param->notification.alarm.numMonitoredAttributes = ncs_decode_16bit(&p8);
		param->notification.alarm.numProposedRepairActions = ncs_decode_16bit(&p8);
		ntfsv_alloc_ntf_alarm(&param->notification.alarm,
				      param->notification.alarm.numSpecificProblems,
				      param->notification.alarm.numMonitoredAttributes,
				      param->notification.alarm.numProposedRepairActions);
		*param->notification.alarm.probableCause = ncs_decode_32bit(&p8);
		*param->notification.alarm.perceivedSeverity = ncs_decode_32bit(&p8);
		*param->notification.alarm.trend = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 16);
		total_bytes += 16;
		p8 = ncs_dec_flatten_space(uba, local_data, 12);
		param->notification.alarm.thresholdInformation->thresholdId = ncs_decode_16bit(&p8);
		param->notification.alarm.thresholdInformation->thresholdValueType = ncs_decode_16bit(&p8);
		param->notification.alarm.thresholdInformation->armTime = ncs_decode_64bit(&p8);
		ncs_dec_skip_space(uba, 12);
		total_bytes += 12;
		total_bytes = decodeNtfValueT(uba, total_bytes,
					      param->notification.alarm.thresholdInformation->thresholdValueType,
					      &param->notification.alarm.thresholdInformation->thresholdValue);
		total_bytes = decodeNtfValueT(uba, total_bytes,
					      param->notification.alarm.thresholdInformation->thresholdValueType,
					      &param->notification.alarm.thresholdInformation->thresholdHysteresis);
		total_bytes = decodeNtfValueT(uba, total_bytes,
					      param->notification.alarm.thresholdInformation->thresholdValueType,
					      &param->notification.alarm.thresholdInformation->observedValue);
		for (i = 0; i < param->notification.alarm.numSpecificProblems; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 12);
			param->notification.alarm.specificProblems[i].problemId = ncs_decode_16bit(&p8);
			param->notification.alarm.specificProblems[i].problemClassId.majorId = ncs_decode_16bit(&p8);
			param->notification.alarm.specificProblems[i].problemClassId.minorId = ncs_decode_16bit(&p8);
			param->notification.alarm.specificProblems[i].problemClassId.vendorId = ncs_decode_32bit(&p8);
			param->notification.alarm.specificProblems[i].problemType = ncs_decode_16bit(&p8);
			ncs_dec_skip_space(uba, 12);
			total_bytes += 12;
			total_bytes = decodeNtfValueT(uba, total_bytes,
						      param->notification.alarm.specificProblems[i].problemType,
						      &param->notification.alarm.specificProblems[i].problemValue);
		}
		for (i = 0; i < param->notification.alarm.numMonitoredAttributes; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->notification.alarm.monitoredAttributes[i].attributeId = ncs_decode_16bit(&p8);
			param->notification.alarm.monitoredAttributes[i].attributeType = ncs_decode_16bit(&p8);
			ncs_dec_skip_space(uba, 4);
			total_bytes += 4;
			total_bytes = decodeNtfValueT(uba, total_bytes,
						      param->notification.alarm.monitoredAttributes[i].attributeType,
						      &param->notification.alarm.monitoredAttributes[i].attributeValue);
		}
		for (i = 0; i < param->notification.alarm.numProposedRepairActions; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->notification.alarm.proposedRepairActions[i].actionId = ncs_decode_16bit(&p8);
			param->notification.alarm.proposedRepairActions[i].actionValueType = ncs_decode_16bit(&p8);
			ncs_dec_skip_space(uba, 4);
			total_bytes += 4;
			total_bytes = decodeNtfValueT(uba, total_bytes,
						      param->notification.alarm.proposedRepairActions[i].
						      actionValueType,
						      &param->notification.alarm.proposedRepairActions[i].actionValue);
		}
		break;

	case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
		TRACE("dec SA_NTF_TYPE_OBJECT_CREATE_DELETE");

		total_bytes += ntfsv_dec_not_header(uba, &param->notification.objectCreateDelete.notificationHeader);
		p8 = ncs_dec_flatten_space(uba, local_data, 4);
		param->notification.objectCreateDelete.numAttributes = ncs_decode_16bit(&p8);
		ntfsv_alloc_ntf_obj_create_del(&param->notification.objectCreateDelete,
					       param->notification.objectCreateDelete.numAttributes);
		*param->notification.objectCreateDelete.sourceIndicator = ncs_decode_16bit(&p8);
		TRACE_2("sourceIndicator %d", (int)*param->notification.objectCreateDelete.sourceIndicator);
		ncs_dec_skip_space(uba, 4);
		total_bytes += 4;

		if (param->notification.objectCreateDelete.numAttributes > 0) {
			for (i = 0; i < param->notification.objectCreateDelete.numAttributes; i++) {
				total_bytes =
				    decodeSaNtfAttribute(uba,
							 total_bytes,
							 &param->notification.objectCreateDelete.objectAttributes[i]);
			}
			ntfsv_print_object_attributes(param->notification.objectCreateDelete.objectAttributes,
						      param->notification.objectCreateDelete.numAttributes);
		}
		break;
	case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
		TRACE("dec A_NTF_TYPE_ATTRIBUTE_CHANGE");
		total_bytes += ntfsv_dec_not_header(uba, &param->notification.attributeChange.notificationHeader);
		p8 = ncs_dec_flatten_space(uba, local_data, 4);
		param->notification.attributeChange.numAttributes = ncs_decode_16bit(&p8);
		ntfsv_alloc_ntf_attr_change(&param->notification.attributeChange,
					    param->notification.objectCreateDelete.numAttributes);
		*param->notification.attributeChange.sourceIndicator = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 4);
		total_bytes += 4;
		for (i = 0; i < param->notification.attributeChange.numAttributes; i++) {
			total_bytes =
			    decodeSaNtfAttributeChangeT(uba,
							total_bytes,
							&param->notification.attributeChange.changedAttributes[i]);
		}
		break;
	case SA_NTF_TYPE_STATE_CHANGE:
		TRACE("dec SA_NTF_TYPE_STATE_CHANGE");
		total_bytes += ntfsv_dec_not_header(uba, &param->notification.stateChange.notificationHeader);
		p8 = ncs_dec_flatten_space(uba, local_data, 4);
		param->notification.stateChange.numStateChanges = ncs_decode_16bit(&p8);
		ntfsv_alloc_ntf_state_change(&param->notification.stateChange,
					     param->notification.stateChange.numStateChanges);
		*param->notification.stateChange.sourceIndicator = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 4);
		total_bytes += 4;
		TRACE("dec numStCh: %d", (int)param->notification.stateChange.numStateChanges);
		for (i = 0; i < param->notification.stateChange.numStateChanges; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 6);
			param->notification.stateChange.changedStates[i].stateId = ncs_decode_16bit(&p8);
			param->notification.stateChange.changedStates[i].oldStatePresent = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 6);
			total_bytes += 6;
			if (param->notification.stateChange.changedStates[i].oldStatePresent) {
				p8 = ncs_dec_flatten_space(uba, local_data, 2);
				param->notification.stateChange.changedStates[i].oldState = ncs_decode_16bit(&p8);
				ncs_dec_skip_space(uba, 2);
				total_bytes += 2;
			}
			p8 = ncs_dec_flatten_space(uba, local_data, 2);
			param->notification.stateChange.changedStates[i].newState = ncs_decode_16bit(&p8);
			ncs_dec_skip_space(uba, 2);
			total_bytes += 2;
		}
		break;
	case SA_NTF_TYPE_SECURITY_ALARM:
		TRACE("dec SA_NTF_TYPE_SECURITY_ALARM");
		total_bytes += ntfsv_dec_not_header(uba, &param->notification.securityAlarm.notificationHeader);
		p8 = ncs_dec_flatten_space(uba, local_data, 10);
		ntfsv_alloc_ntf_security_alarm(&param->notification.securityAlarm);
		*param->notification.securityAlarm.probableCause = ncs_decode_32bit(&p8);
		*param->notification.securityAlarm.severity = ncs_decode_32bit(&p8);
		param->notification.securityAlarm.securityAlarmDetector->valueType = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 10);
		total_bytes += 10;
		total_bytes = decodeNtfValueT(uba, total_bytes,
					      param->notification.securityAlarm.securityAlarmDetector->valueType,
					      &param->notification.securityAlarm.securityAlarmDetector->value);
		p8 = ncs_dec_flatten_space(uba, local_data, 2);
		param->notification.securityAlarm.serviceUser->valueType = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 2);
		total_bytes += 2;
		total_bytes = decodeNtfValueT(uba, total_bytes,
					      param->notification.securityAlarm.serviceUser->valueType,
					      &param->notification.securityAlarm.serviceUser->value);
		p8 = ncs_dec_flatten_space(uba, local_data, 2);
		param->notification.securityAlarm.serviceProvider->valueType = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 2);
		total_bytes += 2;
		total_bytes = decodeNtfValueT(uba, total_bytes,
					      param->notification.securityAlarm.serviceProvider->valueType,
					      &param->notification.securityAlarm.serviceProvider->value);

		break;
	default:
		TRACE("notificationType not valid");
		return 0;
		break;
	}
	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	param->variable_data.size = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4);
	total_bytes += 4;
	if (param->variable_data.size) {
		/* freed in ntfsv_dealloc_notification */
		param->variable_data.p_base = calloc(1, param->variable_data.size);
		TRACE("alloc v_data.p_base %p", param->variable_data.p_base);
		ncs_decode_n_octets_from_uba(uba, param->variable_data.p_base, param->variable_data.size);
	}
	TRACE_LEAVE();
	return total_bytes;
}

uns32 ntfsv_dec_discard_msg(NCS_UBAID *uba, ntfsv_discarded_info_t *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;
	uint8_t local_data[8];
	int i;
	
	TRACE_ENTER();
	p8 = ncs_dec_flatten_space(uba, local_data, 8);
	param->notificationType = ncs_decode_32bit(&p8);
	param->numberDiscarded = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 8);
	total_bytes += 8;
	TRACE_3("t:%#x, nd: %u", param->notificationType, param->numberDiscarded);
	if (param->numberDiscarded) {
		param->discardedNotificationIdentifiers = calloc(1, sizeof(SaNtfIdentifierT) * param->numberDiscarded);
		if (!param->discardedNotificationIdentifiers) {
			TRACE_LEAVE();
			return 0;
		}
		for (i = 0; i < param->numberDiscarded; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 8);
			param->discardedNotificationIdentifiers[i] = ncs_decode_64bit(&p8);
			ncs_dec_skip_space(uba, 8);
			total_bytes += 8;
		} 
	} else {
		param->discardedNotificationIdentifiers = NULL;
	}
	TRACE_LEAVE();
	return 1;
}

uns32 ntfsv_enc_filter_header(NCS_UBAID *uba, SaNtfNotificationFilterHeaderT *h)
{
	int i;
	uns32 rc;
	uint8_t *p8;

	p8 = ncs_enc_reserve_space(uba, 8);
	if (!p8)
		goto error_done;
	ncs_encode_16bit(&p8, h->numEventTypes);
	ncs_encode_16bit(&p8, h->numNotificationClassIds);
	ncs_encode_16bit(&p8, h->numNotificationObjects);
	ncs_encode_16bit(&p8, h->numNotifyingObjects);
	ncs_enc_claim_space(uba, 8);
	
	for (i = 0; i < h->numEventTypes; i++) {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8)
			goto error_done;
		ncs_encode_32bit(&p8, h->eventTypes[i]);
		ncs_enc_claim_space(uba, 4);
	}
	for (i = 0; i < h->numNotificationClassIds; i++) {
		p8 = ncs_enc_reserve_space(uba, 8);
		if (!p8)
			goto error_done;
		ncs_encode_32bit(&p8, h->notificationClassIds[i].vendorId);
		ncs_encode_16bit(&p8, h->notificationClassIds[i].majorId);
		ncs_encode_16bit(&p8, h->notificationClassIds[i].minorId);
		ncs_enc_claim_space(uba, 8);
	}
	for (i = 0; i < h->numNotifyingObjects; i++) {	
		rc = encodeSaNameT(uba, p8, 0, &h->notifyingObjects[i]);
		if (!rc) {
			goto error_done;
		}
	}
	for (i = 0; i < h->numNotificationObjects; i++) {	
		rc = encodeSaNameT(uba, p8, 0, &h->notificationObjects[i]);
		if (!rc) {
			goto error_done;
		}
	}
	return 1; 
	
 error_done:
	TRACE_2("reserv space failed");
	return 0;  
}  

uns32 ntfsv_enc_subscribe_msg(NCS_UBAID *uba, ntfsv_subscribe_req_t *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0, rc;
	int i;
	
	TRACE_ENTER();
	assert(uba != NULL);

	/** encode the contents **/
	p8 = ncs_enc_reserve_space(uba, 8);
	if (!p8) {
		TRACE("p8 NULL!!!");
		return 0;
	}
	ncs_encode_32bit(&p8, param->client_id);
	ncs_encode_32bit(&p8, param->subscriptionId);
	ncs_enc_claim_space(uba, 8);
	total_bytes += 8;
	
	TRACE_3("alarm filter p: %p", param->f_rec.alarm_filter);
	if (param->f_rec.alarm_filter != NULL) {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8)
			return 0;
		ncs_encode_32bit(&p8, SA_NTF_TYPE_ALARM);
		ncs_enc_claim_space(uba, 4);
		rc = ntfsv_enc_filter_header(uba, &param->f_rec.alarm_filter->notificationFilterHeader);
		if (!rc)
			return 0;
		
		p8 = ncs_enc_reserve_space(uba, 6);
		if (!p8) 
			return 0;
		ncs_encode_16bit(&p8, param->f_rec.alarm_filter->numProbableCauses);
		ncs_encode_16bit(&p8, param->f_rec.alarm_filter->numPerceivedSeverities);
		ncs_encode_16bit(&p8, param->f_rec.alarm_filter->numTrends);
		ncs_enc_claim_space(uba, 6);
		
		for (i = 0; i < param->f_rec.alarm_filter->numProbableCauses; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8) {
				return 0;
			}
			ncs_encode_32bit(&p8, param->f_rec.alarm_filter->probableCauses[i]);
			ncs_enc_claim_space(uba, 4);
		}
		for (i = 0; i < param->f_rec.alarm_filter->numPerceivedSeverities; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8)
				return 0;
			ncs_encode_32bit(&p8, param->f_rec.alarm_filter->perceivedSeverities[i]);
			ncs_enc_claim_space(uba, 4);
		}
		for (i = 0; i < param->f_rec.alarm_filter->numTrends; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8)
				return 0;
			ncs_encode_32bit(&p8, param->f_rec.alarm_filter->trends[i]);
			ncs_enc_claim_space(uba, 4);
		}  
	} else {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8)
			return 0;
		ncs_encode_32bit(&p8, 0);
		ncs_enc_claim_space(uba, 4);
	}

	TRACE_3("sec alarm filter p: %p", param->f_rec.sec_al_filter);
	if (param->f_rec.sec_al_filter != NULL) {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8) {
			return 0;
		}
		ncs_encode_32bit(&p8, SA_NTF_TYPE_SECURITY_ALARM);
		ncs_enc_claim_space(uba, 4);
		rc = ntfsv_enc_filter_header(uba, &param->f_rec.sec_al_filter->notificationFilterHeader);
		if (!rc)
			return 0;
		p8 = ncs_enc_reserve_space(uba, 10);
		if (!p8) {
			return 0;
		}		
		ncs_encode_16bit(&p8, param->f_rec.sec_al_filter->numProbableCauses);
		ncs_encode_16bit(&p8, param->f_rec.sec_al_filter->numSeverities);
		ncs_encode_16bit(&p8, param->f_rec.sec_al_filter->numSecurityAlarmDetectors);
		ncs_encode_16bit(&p8, param->f_rec.sec_al_filter->numServiceProviders);
		ncs_encode_16bit(&p8, param->f_rec.sec_al_filter->numServiceUsers);
		ncs_enc_claim_space(uba, 10);

		for (i = 0; i < param->f_rec.sec_al_filter->numProbableCauses; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8) {
				return 0;
			}
			ncs_encode_32bit(&p8, param->f_rec.sec_al_filter->probableCauses[i]);
			ncs_enc_claim_space(uba, 4);
		}
		for (i = 0; i < param->f_rec.sec_al_filter->numSeverities; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8)
				return 0;
			ncs_encode_32bit(&p8, param->f_rec.sec_al_filter->severities[i]);
			ncs_enc_claim_space(uba, 4);
		}
		
		for (i = 0; i < param->f_rec.sec_al_filter->numSecurityAlarmDetectors; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8)
				return 0;
			ncs_encode_32bit(&p8, param->f_rec.sec_al_filter->securityAlarmDetectors[i].valueType);
			ncs_enc_claim_space(uba, 4);
			rc = encodeSaNtfValueT(uba, p8, 0, param->f_rec.sec_al_filter->securityAlarmDetectors[i].valueType, &param->f_rec.sec_al_filter->securityAlarmDetectors[i].value); 
			if (!rc) return 0;  
		}
		  
		for (i = 0; i < param->f_rec.sec_al_filter->numServiceProviders; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8)
				return 0;
			ncs_encode_32bit(&p8, param->f_rec.sec_al_filter->serviceProviders[i].valueType);
			ncs_enc_claim_space(uba, 4);
			rc = encodeSaNtfValueT(uba, p8, 0, param->f_rec.sec_al_filter->serviceProviders[i].valueType, &param->f_rec.sec_al_filter->serviceProviders[i].value);			
			if (!rc) return 0;   
		}
		  
		for (i = 0; i < param->f_rec.sec_al_filter->numServiceUsers; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8)
				return 0;
			ncs_encode_32bit(&p8, param->f_rec.sec_al_filter->serviceUsers[i].valueType);
			ncs_enc_claim_space(uba, 4);
			rc = encodeSaNtfValueT(uba, p8, 0, param->f_rec.sec_al_filter->serviceUsers[i].valueType, &param->f_rec.sec_al_filter->serviceUsers[i].value);
			if (!rc) return 0;  
		}
	} else {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8) {
			return 0;
		}
		ncs_encode_32bit(&p8, 0);
		ncs_enc_claim_space(uba, 4);
	}
	
	TRACE_3("att_ch_filter p: %p", param->f_rec.att_ch_filter);
	if (param->f_rec.att_ch_filter != NULL) {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8)
			return 0;
		ncs_encode_32bit(&p8, SA_NTF_TYPE_ATTRIBUTE_CHANGE);
		ncs_enc_claim_space(uba, 4);
		rc = ntfsv_enc_filter_header(uba, &param->f_rec.att_ch_filter->notificationFilterHeader);
		if (!rc)
			return 0;

		p8 = ncs_enc_reserve_space(uba, 2);
		if (!p8) 
			return 0;
		ncs_encode_16bit(&p8, param->f_rec.att_ch_filter->numSourceIndicators);
		ncs_enc_claim_space(uba, 2);
		  
		for (i = 0; i < param->f_rec.att_ch_filter->numSourceIndicators; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8) {
				return 0;
			}
			ncs_encode_32bit(&p8, param->f_rec.att_ch_filter->sourceIndicators[i]);
			ncs_enc_claim_space(uba, 4);
		}
	} else {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8)
			return 0;
		ncs_encode_32bit(&p8, 0);
		ncs_enc_claim_space(uba, 4);
	}

	TRACE_3("obj_cr_del_filter p: %p", param->f_rec.obj_cr_del_filter);
	if (param->f_rec.obj_cr_del_filter != NULL) {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8)
			return 0;
		ncs_encode_32bit(&p8, SA_NTF_TYPE_OBJECT_CREATE_DELETE);
		ncs_enc_claim_space(uba, 4);
		rc = ntfsv_enc_filter_header(uba, &param->f_rec.obj_cr_del_filter->notificationFilterHeader);
		if (!rc)
			return 0;

		p8 = ncs_enc_reserve_space(uba, 2);
		if (!p8) 
			return 0;
		ncs_encode_16bit(&p8, param->f_rec.obj_cr_del_filter->numSourceIndicators);
		ncs_enc_claim_space(uba, 2);

		for (i = 0; i < param->f_rec.obj_cr_del_filter->numSourceIndicators; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8) {
				return 0;
			}
			ncs_encode_32bit(&p8, param->f_rec.obj_cr_del_filter->sourceIndicators[i]);
			ncs_enc_claim_space(uba, 4);
		}
	} else {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8)
			return 0;
		ncs_encode_32bit(&p8, 0);
		ncs_enc_claim_space(uba, 4);
	}
	
	TRACE_3("sta_ch_filter p: %p", param->f_rec.sta_ch_filter);
	if (param->f_rec.sta_ch_filter != NULL) {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8)
			return 0;
		ncs_encode_32bit(&p8, SA_NTF_TYPE_STATE_CHANGE);
		ncs_enc_claim_space(uba, 4);
		rc = ntfsv_enc_filter_header(uba, &param->f_rec.sta_ch_filter->notificationFilterHeader);
		if (!rc)
			return 0;

		p8 = ncs_enc_reserve_space(uba, 8);
		if (!p8) 
			return 0;
		ncs_encode_32bit(&p8, param->f_rec.sta_ch_filter->numSourceIndicators);
		ncs_encode_32bit(&p8, param->f_rec.sta_ch_filter->numStateChanges);
		ncs_enc_claim_space(uba, 8);
		TRACE_3("numSourceIndicators: %d", (int)param->f_rec.sta_ch_filter->numSourceIndicators);
		for (i = 0; i < param->f_rec.sta_ch_filter->numSourceIndicators; i++) {
			p8 = ncs_enc_reserve_space(uba, 4);
			if (!p8) {
				return 0;
			}
			ncs_encode_32bit(&p8, param->f_rec.sta_ch_filter->sourceIndicators[i]);
			ncs_enc_claim_space(uba, 4);
		}
		TRACE_3("numStateChanges: %d", (int)param->f_rec.sta_ch_filter->numStateChanges);
		for (i = 0; i < param->f_rec.sta_ch_filter->numStateChanges; i++) {
			p8 = ncs_enc_reserve_space(uba, 8);
			if (!p8)	return 0;
			ncs_encode_16bit(&p8, param->f_rec.sta_ch_filter->changedStates[i].stateId);
			ncs_encode_16bit(&p8, param->f_rec.sta_ch_filter->changedStates[i].newState);
			ncs_encode_32bit(&p8, param->f_rec.sta_ch_filter->changedStates[i].oldStatePresent);
			ncs_enc_claim_space(uba, 8);
			TRACE_3("oldStatePresent: %d", param->f_rec.sta_ch_filter->changedStates[i].oldStatePresent);
			if (param->f_rec.sta_ch_filter->changedStates[i].oldStatePresent) {
				p8 = ncs_enc_reserve_space(uba, 2);
				if (!p8) return 0;
				ncs_encode_16bit(&p8, param->f_rec.sta_ch_filter->changedStates[i].oldState);
				ncs_enc_claim_space(uba, 2);
			}
		}
	} else {
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8)
			return 0;
		ncs_encode_32bit(&p8, 0);
		ncs_enc_claim_space(uba, 4);
	}	
	
	rc = ntfsv_enc_discard_msg(uba, &param->d_info);
	if (!rc)
		return 0;

	TRACE_LEAVE();
	return total_bytes;
}

uns32 ntfsv_dec_filter_header(NCS_UBAID *uba, SaNtfNotificationFilterHeaderT *h)
{
	int i;
	uint8_t *p8;
	uns32 rc ;
	uint8_t local_data[8];
	
	p8 = ncs_dec_flatten_space(uba, local_data, 8);
	h->numEventTypes = ncs_decode_16bit(&p8);
	h->numNotificationClassIds = ncs_decode_16bit(&p8);
	h->numNotificationObjects = ncs_decode_16bit(&p8);
	h->numNotifyingObjects = ncs_decode_16bit(&p8);
	ncs_dec_skip_space(uba, 8);
	ntfsv_filter_header_alloc(h, h->numEventTypes, h->numNotificationObjects, h->numNotifyingObjects,
		h->numNotificationClassIds);
	for (i = 0; i < h->numEventTypes; i++) {
		p8 = ncs_dec_flatten_space(uba, local_data, 4);
		if (!p8)
			goto error_done;
		h->eventTypes[i] = ncs_decode_32bit(&p8);
		ncs_dec_skip_space(uba, 4);
	}
	for (i = 0; i < h->numNotificationClassIds; i++) {
		p8 = ncs_dec_flatten_space(uba, local_data, 8);
		if (!p8)
			goto error_done;
		h->notificationClassIds[i].vendorId = ncs_decode_32bit(&p8);
		h->notificationClassIds[i].majorId = ncs_decode_16bit(&p8);
		h->notificationClassIds[i].minorId = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 8);
	}
	for (i = 0; i < h->numNotifyingObjects; i++) {	
		rc = decodeSaNameT(uba, p8, 0, &h->notifyingObjects[i]);
		if (!rc) {
			goto error_done;
		}
	}
	for (i = 0; i < h->numNotificationObjects; i++) {	
		rc = decodeSaNameT(uba, p8, 0, &h->notificationObjects[i]);
		if (!rc) {
			goto error_done;
		}
	}
	return 1;
	
 error_done:
	 ntfsv_filter_header_free(h);
	TRACE_2("reserv space failed");
	return 0;    
}

uns32 ntfsv_dec_subscribe_msg(NCS_UBAID *uba, ntfsv_subscribe_req_t *param)
{
	int i;
	uint8_t *p8;
	uns32 rc = 0;
	uint8_t local_data[10];
	uns32 filter_type;
	param->f_rec.alarm_filter = NULL;
	param->f_rec.att_ch_filter = NULL;
	param->f_rec.obj_cr_del_filter = NULL;
	param->f_rec.sec_al_filter = NULL;
	param->f_rec.sta_ch_filter = NULL;
TRACE_ENTER();
	/* client_id  */
	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	param->client_id = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4);

	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	param->subscriptionId = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4);

	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	filter_type = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4);
	 
/* memory allocated is freed in filter destructors */
	if (filter_type == SA_NTF_TYPE_ALARM) {
		TRACE_2("dec: SA_NTF_TYPE_ALARM");
		param->f_rec.alarm_filter = malloc(sizeof(*param->f_rec.alarm_filter));
		if (param->f_rec.alarm_filter == NULL) {
			rc = 0;
			goto error_free;
		}
		rc = ntfsv_dec_filter_header(uba, &param->f_rec.alarm_filter->notificationFilterHeader);
		if (!rc)
			goto error_free;
		
		p8 = ncs_dec_flatten_space(uba, local_data, 6);
		param->f_rec.alarm_filter->numProbableCauses = ncs_decode_16bit(&p8);
		param->f_rec.alarm_filter->numPerceivedSeverities = ncs_decode_16bit(&p8);
		param->f_rec.alarm_filter->numTrends = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 6);
		
		rc = ntfsv_filter_alarm_alloc(param->f_rec.alarm_filter,
			param->f_rec.alarm_filter->numProbableCauses,
			param->f_rec.alarm_filter->numPerceivedSeverities,
			param->f_rec.alarm_filter->numTrends);
		if (!rc)
			goto error_free;
		
		for (i = 0; i < param->f_rec.alarm_filter->numProbableCauses; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.alarm_filter->probableCauses[i] = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);

		}
		for (i = 0; i < param->f_rec.alarm_filter->numPerceivedSeverities; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.alarm_filter->perceivedSeverities[i] = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);
		}
		for (i = 0; i < param->f_rec.alarm_filter->numTrends; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.alarm_filter->trends[i] = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);
		}  				  
	} else {
		assert(!filter_type);
		param->f_rec.alarm_filter = NULL;
	}
	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	filter_type = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4);

	if (filter_type == SA_NTF_TYPE_SECURITY_ALARM) {
		TRACE_2("dec: SA_NTF_TYPE_SECURITY_ALARM");
		param->f_rec.sec_al_filter = malloc(sizeof(*param->f_rec.sec_al_filter));
		if (param->f_rec.sec_al_filter == NULL) {
			rc = 0;
			goto error_free;
		}
		rc = ntfsv_dec_filter_header(uba, &param->f_rec.sec_al_filter->notificationFilterHeader);
		if (!rc)
			goto error_free;
		p8 = ncs_dec_flatten_space(uba, local_data, 10);
		param->f_rec.sec_al_filter->numProbableCauses = ncs_decode_16bit(&p8);
		param->f_rec.sec_al_filter->numSeverities = ncs_decode_16bit(&p8);
		param->f_rec.sec_al_filter->numSecurityAlarmDetectors = ncs_decode_16bit(&p8);
		param->f_rec.sec_al_filter->numServiceProviders = ncs_decode_16bit(&p8);
		param->f_rec.sec_al_filter->numServiceUsers = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 10);

		rc = ntfsv_filter_sec_alarm_alloc(param->f_rec.sec_al_filter,
			param->f_rec.sec_al_filter->numProbableCauses,
			param->f_rec.sec_al_filter->numSeverities,
			param->f_rec.sec_al_filter->numSecurityAlarmDetectors,
			param->f_rec.sec_al_filter->numServiceUsers,
			param->f_rec.sec_al_filter->numServiceProviders);
		if (!rc)
			goto error_free;

		for (i = 0; i < param->f_rec.sec_al_filter->numProbableCauses; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.sec_al_filter->probableCauses[i] = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);

		}
		for (i = 0; i < param->f_rec.sec_al_filter->numSeverities; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.sec_al_filter->severities[i] = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);
		}
		for (i = 0; i < param->f_rec.sec_al_filter->numSecurityAlarmDetectors; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.sec_al_filter->securityAlarmDetectors[i].valueType = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);
			rc = decodeNtfValueT(uba, 0, param->f_rec.sec_al_filter->securityAlarmDetectors[i].valueType, &param->f_rec.sec_al_filter->securityAlarmDetectors[i].value);
			if (!rc)
				goto error_free;
		}  				  		
		for (i = 0; i < param->f_rec.sec_al_filter->numServiceProviders; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.sec_al_filter->serviceProviders[i].valueType = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);
			rc = decodeNtfValueT(uba, 0, param->f_rec.sec_al_filter->serviceProviders[i].valueType, &param->f_rec.sec_al_filter->serviceProviders[i].value);
			if (!rc)
				goto error_free;
		}  				  
		for (i = 0; i < param->f_rec.sec_al_filter->numServiceUsers; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.sec_al_filter->serviceUsers[i].valueType = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);
			rc = decodeNtfValueT(uba, 0, param->f_rec.sec_al_filter->serviceUsers[i].valueType, &param->f_rec.sec_al_filter->serviceUsers[i].value);
			if (!rc)
				goto error_free;
		}  				  
	} else {
		assert(!filter_type);
		param->f_rec.sec_al_filter = NULL;
	}
	
	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	filter_type = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4) ;
	if (filter_type == SA_NTF_TYPE_ATTRIBUTE_CHANGE) {
		TRACE_2("dec: SA_NTF_TYPE_ATTRIBUTE_CHANGE");
		param->f_rec.att_ch_filter = malloc(sizeof(*param->f_rec.att_ch_filter));
		if (param->f_rec.att_ch_filter == NULL) {
			rc = 0;
			goto error_free;
		}
		rc = ntfsv_dec_filter_header(uba, &param->f_rec.att_ch_filter->notificationFilterHeader);
		if (!rc)
			goto error_free;

		p8 = ncs_dec_flatten_space(uba, local_data, 2);
		param->f_rec.att_ch_filter->numSourceIndicators = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 2);

		rc = ntfsv_filter_attr_change_alloc(param->f_rec.att_ch_filter,
			param->f_rec.att_ch_filter->numSourceIndicators);
		if (!rc)
			goto error_free;

		for (i = 0; i < param->f_rec.att_ch_filter->numSourceIndicators; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.att_ch_filter->sourceIndicators[i] = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);
		}
	} else {
		assert(!filter_type);
		param->f_rec.att_ch_filter = NULL;
	}

	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	filter_type = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4) ;
	if (filter_type == SA_NTF_TYPE_OBJECT_CREATE_DELETE) {
		TRACE_2("dec: SA_NTF_TYPE_OBJECT_CREATE_DELETE");
		param->f_rec.obj_cr_del_filter = malloc(sizeof(*param->f_rec.obj_cr_del_filter));
		if (param->f_rec.obj_cr_del_filter == NULL) {
			rc = 0;
			goto error_free;
		}
		rc = ntfsv_dec_filter_header(uba, &param->f_rec.obj_cr_del_filter->notificationFilterHeader);
		if (!rc)
			goto error_free;

		p8 = ncs_dec_flatten_space(uba, local_data, 2);
		param->f_rec.obj_cr_del_filter->numSourceIndicators = ncs_decode_16bit(&p8);
		ncs_dec_skip_space(uba, 2);

		rc = ntfsv_filter_obj_cr_del_alloc(param->f_rec.obj_cr_del_filter,
			param->f_rec.obj_cr_del_filter->numSourceIndicators);
		if (!rc)
			goto error_free;

		for (i = 0; i < param->f_rec.obj_cr_del_filter->numSourceIndicators; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.obj_cr_del_filter->sourceIndicators[i] = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);
		}
	} else {
		assert(!filter_type);
		param->f_rec.obj_cr_del_filter = NULL;
	}
 	
	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	filter_type = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4) ;
	if (filter_type == SA_NTF_TYPE_STATE_CHANGE) {
		TRACE_2("dec: SA_NTF_TYPE_STATE_CHANGE");
		param->f_rec.sta_ch_filter = malloc(sizeof(*param->f_rec.sta_ch_filter));
		if (param->f_rec.sta_ch_filter == NULL) {
			rc = 0;
			goto error_free;
		}
		rc = ntfsv_dec_filter_header(uba, &param->f_rec.sta_ch_filter->notificationFilterHeader);
		if (!rc)
			goto error_free;

		p8 = ncs_dec_flatten_space(uba, local_data, 8);
		param->f_rec.sta_ch_filter->numSourceIndicators = ncs_decode_32bit(&p8);
		param->f_rec.sta_ch_filter->numStateChanges = ncs_decode_32bit(&p8);
		ncs_dec_skip_space(uba, 8);
		
		rc = ntfsv_filter_state_ch_alloc(param->f_rec.sta_ch_filter,
			param->f_rec.sta_ch_filter->numSourceIndicators, 
			param->f_rec.sta_ch_filter->numStateChanges);
		if (!rc)
			goto error_free;
		TRACE_2("numSourceIndicators: %d", param->f_rec.sta_ch_filter->numSourceIndicators);

		for (i = 0; i < param->f_rec.sta_ch_filter->numSourceIndicators; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 4);
			param->f_rec.sta_ch_filter->sourceIndicators[i] = ncs_decode_32bit(&p8);
			ncs_dec_skip_space(uba, 4);
		}
		TRACE_2("numStateChanges: %d", param->f_rec.sta_ch_filter->numStateChanges);
		for (i = 0; i < param->f_rec.sta_ch_filter->numStateChanges; i++) {
			p8 = ncs_dec_flatten_space(uba, local_data, 8);
			param->f_rec.sta_ch_filter->changedStates[i].stateId = ncs_decode_16bit(&p8);
			param->f_rec.sta_ch_filter->changedStates[i].newState = ncs_decode_16bit(&p8);
			param->f_rec.sta_ch_filter->changedStates[i].oldStatePresent = ncs_decode_32bit(&p8);
			TRACE_2("oldStatePresent: %d", param->f_rec.sta_ch_filter->changedStates[i].oldStatePresent);
			ncs_dec_skip_space(uba, 8);
			if (param->f_rec.sta_ch_filter->changedStates[i].oldStatePresent){
				p8 = ncs_dec_flatten_space(uba, local_data, 2);
				param->f_rec.sta_ch_filter->changedStates[i].oldState = ncs_decode_16bit(&p8);				
				ncs_dec_skip_space(uba, 2);
			}
		}
	} else {
		assert(!filter_type);
		param->f_rec.sta_ch_filter = NULL;
	}
	
	rc = ntfsv_dec_discard_msg(uba, &param->d_info);
	if (!rc)
		goto error_free;
done:	
	TRACE_8("NTFSV_SUBSCRIBE_REQ");
	TRACE_LEAVE();
	return rc;

error_free:
   TRACE("No memory");
	free(param->f_rec.alarm_filter);
	free(param->f_rec.att_ch_filter);
	free(param->f_rec.obj_cr_del_filter);
	free(param->f_rec.sec_al_filter);
	free(param->f_rec.sta_ch_filter);
	goto done;
}

uns32 ntfsv_enc_64bit_msg(NCS_UBAID *uba, uns64 param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;

	TRACE_ENTER();
	assert(uba != NULL);

	/** encode the contents **/
	p8 = ncs_enc_reserve_space(uba, 8);
	if (!p8) {
		TRACE("p8 NULL!!!");
		return 0;
	}
	ncs_encode_64bit(&p8, param);
	ncs_enc_claim_space(uba, 8);
	total_bytes += 8;

	TRACE_LEAVE();
	return total_bytes;
    }

uns32 ntfsv_dec_64bit_msg(NCS_UBAID *uba, uns64 *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;
	uint8_t local_data[8];

	p8 = ncs_dec_flatten_space(uba, local_data, 8);
	*param = ncs_decode_64bit(&p8);
	ncs_dec_skip_space(uba, 8);
	total_bytes += 8;
	TRACE_8("NTFSV_ENC_64bit");
	return total_bytes;
}

uns32 ntfsv_enc_32bit_msg(NCS_UBAID *uba, uns32 param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;

	TRACE_ENTER();
	assert(uba != NULL);

	/** encode the contents **/
	p8 = ncs_enc_reserve_space(uba, 4);
	if (!p8) {
		TRACE("p8 NULL!!!");
		return 0;
	}
	ncs_encode_32bit(&p8, param);
	ncs_enc_claim_space(uba, 4);
	total_bytes += 4;

	TRACE_LEAVE();
	return total_bytes;
}

uns32 ntfsv_dec_32bit_msg(NCS_UBAID *uba, uns32 *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;
	uint8_t local_data[4];

	p8 = ncs_dec_flatten_space(uba, local_data, 4);
	*param = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 4);
	total_bytes += 4;
	TRACE_8("NTFSV_ENC_32bit");
	return total_bytes;
}

uns32 ntfsv_enc_unsubscribe_msg(NCS_UBAID *uba, ntfsv_unsubscribe_req_t *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;
	assert(uba != NULL);

	/** encode the contents **/
	p8 = ncs_enc_reserve_space(uba, 8);
	if (!p8) {
		TRACE("p8 NULL!!!");
		return 0;
	}
	ncs_encode_32bit(&p8, param->client_id);
	ncs_encode_32bit(&p8, param->subscriptionId);
	ncs_enc_claim_space(uba, 8);
	total_bytes += 8;

	return total_bytes;
}

uns32 ntfsv_dec_unsubscribe_msg(NCS_UBAID *uba, ntfsv_unsubscribe_req_t *param)
{
	uint8_t *p8;
	uns32 total_bytes = 0;
	uint8_t local_data[8];

	/* client_id, lstr_id, lstr_open_id */
	p8 = ncs_dec_flatten_space(uba, local_data, 8);
	param->client_id = ncs_decode_32bit(&p8);
	param->subscriptionId = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(uba, 8);
	total_bytes += 8;

	TRACE_8("NTFSV_UNSUBSCRIBE_REQ");
	return total_bytes;
}
