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

#include "ntfsv_enc_dec.h"
#include "ntfsv_mem.h"


static void print_object_attribute(SaNtfAttributeT *input) 
{    
    (void)TRACE_2("Attr ID: %d\n", (int)input->attributeId);
    (void)TRACE_2("Attr Type: %d\n", (int)input->attributeType);
    (void)TRACE_2("Attr Value: %d\n", (int)input->attributeValue.int32Val);
}


void ntfsv_print_object_attributes(SaNtfAttributeT *objectAttributes,
                                     SaUint16T numAttributes)
{
    int i;
    (void)TRACE_2("numAttr: %d\n", (int)numAttributes);

    for (i = 0; i < numAttributes; i++) {
        print_object_attribute(&objectAttributes[i]);
    }
}

static uns32 encodeSaNtfValueT(NCS_UBAID *uba,
                               uns8 *p8, 
                               uns32 total_bytes,
                               SaNtfValueTypeT attributeType,
                               SaNtfValueT* ntfAttr)
{

    switch (attributeType)
    {
        case SA_NTF_VALUE_UINT8:
            p8 = ncs_enc_reserve_space(uba, 1);
            if (!p8)
            {
                TRACE("p8 NULL!!!");
                return 0;
            }
            ncs_encode_8bit(&p8, ntfAttr->uint8Val);
            ncs_enc_claim_space(uba, 1);
            total_bytes += 1;
            break;
        case SA_NTF_VALUE_INT8:
            p8 = ncs_enc_reserve_space(uba, 1);
            if (!p8)
            {
                TRACE("p8 NULL!!!");
                return 0;
            }
            ncs_encode_8bit(&p8, ntfAttr->int8Val);
            ncs_enc_claim_space(uba, 1);
            total_bytes += 1;
            break;
        case SA_NTF_VALUE_UINT16:
            p8 = ncs_enc_reserve_space(uba, 2);
            if (!p8)
            {
                TRACE("p8 NULL!!!");
                return 0;
            }
            ncs_encode_16bit(&p8, ntfAttr->uint16Val);
            ncs_enc_claim_space(uba, 2);
            total_bytes += 2;
            break;
        case SA_NTF_VALUE_INT16:
            p8 = ncs_enc_reserve_space(uba, 2);
            if (!p8)
            {
                TRACE("p8 NULL!!!");
                return 0;
            }
            ncs_encode_16bit(&p8, ntfAttr->int16Val);
            ncs_enc_claim_space(uba, 2);
            total_bytes += 2;
            break;
        case SA_NTF_VALUE_UINT32:
            p8 = ncs_enc_reserve_space(uba, 4);
            if (!p8)
            {
                TRACE("p8 NULL!!!");
                return 0;
            }
            ncs_encode_32bit(&p8, ntfAttr->uint32Val);
            ncs_enc_claim_space(uba, 4);
            total_bytes += 4;
            break;
        case SA_NTF_VALUE_INT32:
            p8 = ncs_enc_reserve_space(uba, 4);
            if (!p8)
            {
                TRACE("p8 NULL!!!");
                return 0;
            }
            ncs_encode_32bit(&p8, ntfAttr->int32Val);
            ncs_enc_claim_space(uba, 4);
            total_bytes += 4;
            break;
        case SA_NTF_VALUE_FLOAT:
            p8 = ncs_enc_reserve_space(uba, 4);
            if (!p8)
            {
                TRACE("p8 NULL!!!");
                return 0;
            }
            ncs_encode_32bit(&p8, (uns32)ntfAttr->floatVal);
            ncs_enc_claim_space(uba, 4);
            total_bytes += 4;
            break;
        case SA_NTF_VALUE_UINT64:
            p8 = ncs_enc_reserve_space(uba, 8);
            if (!p8)
            {
                TRACE("p8 NULL!!!");
                return 0;
            }
            ncs_encode_64bit(&p8, ntfAttr->uint64Val);
            ncs_enc_claim_space(uba, 8);
            total_bytes += 8;
            break;
        case SA_NTF_VALUE_INT64:
            p8 = ncs_enc_reserve_space(uba, 8);
            if (!p8)
            {
                TRACE("p8 NULL!!!");
                return 0;
            }
            ncs_encode_64bit(&p8, ntfAttr->int64Val);
            ncs_enc_claim_space(uba, 8);
            total_bytes += 8;
            break;
        case SA_NTF_VALUE_DOUBLE:
            p8 = ncs_enc_reserve_space(uba, 8);
            if (!p8)
            {
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
        case SA_NTF_VALUE_ARRAY:
            TRACE_2("attributeType %d not implemented",
                    (int)attributeType);
            break;
        default:
            TRACE_2("attributeType %d not valid",
                    (int)attributeType);
    }
    return total_bytes;
}

static uns32 encodeSaNtfAttributeChangeT(NCS_UBAID *uba,
                                  uns8 *p8, 
                                  uns32 total_bytes,
                                  SaNtfAttributeChangeT* ntfAttr)
{
    p8 = ncs_enc_reserve_space(uba, 5);
    if (!p8)
    {
        TRACE("p8 NULL!!!");
        return 0;
    }
    ncs_encode_16bit(&p8, ntfAttr->attributeId);
    ncs_encode_16bit(&p8, (uns16)ntfAttr->attributeType);
    ncs_encode_8bit(&p8, (uns16)ntfAttr->oldAttributePresent);
    ncs_enc_claim_space(uba, 5);
    total_bytes += 5;
    if (ntfAttr->oldAttributePresent)
    {
        total_bytes += encodeSaNtfValueT(uba,
                                         p8,
                                         total_bytes,
                                         ntfAttr->attributeType,
                                         &ntfAttr->oldAttributeValue);
    }
    total_bytes += encodeSaNtfValueT(uba,
                                     p8,
                                     total_bytes,
                                     ntfAttr->attributeType,
                                     &ntfAttr->newAttributeValue);
    return total_bytes;
}

static uns32 encodeSaNtfAttribute(NCS_UBAID *uba,
                                  uns8 *p8, 
                                  uns32 total_bytes,
                                  SaNtfAttributeT* ntfAttr)
{
    p8 = ncs_enc_reserve_space(uba, 4);
    if (!p8)
    {
        TRACE("p8 NULL!!!");
        return 0;
    }
    ncs_encode_16bit(&p8, ntfAttr->attributeId);
    ncs_encode_16bit(&p8, (uns16)ntfAttr->attributeType);
    ncs_enc_claim_space(uba, 4);
    total_bytes += 4;
    total_bytes += encodeSaNtfValueT(uba,
                                     p8,
                                     total_bytes,
                                     ntfAttr->attributeType,
                                     &ntfAttr->attributeValue);
    return total_bytes;
}

static uns32 decodeNtfValueT(NCS_UBAID *uba,
                                   uns32 total_bytes,
                                   SaNtfValueTypeT attributeType,
                                   SaNtfValueT* ntfAttr)
{
    uns8        *p8;
    uns8       local_data[8];

    switch (attributeType)
    {
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
        case SA_NTF_VALUE_ARRAY:
            TRACE_2("attributeType %d not implemented",
                    (int)attributeType);
            break;
        default:
            TRACE_2("attributeType %d not valid",
                    (int)attributeType);
    }
    return total_bytes;
}

static uns32 decodeSaNtfAttributeChangeT(NCS_UBAID *uba,
                                         uns32 total_bytes,
                                         SaNtfAttributeChangeT* ntfAttr)
{
    uns8        *p8;
    uns8       local_data[5];
    p8 = ncs_dec_flatten_space(uba, local_data, 5);
    ntfAttr->attributeId = ncs_decode_16bit(&p8);
    ntfAttr->attributeType = ncs_decode_16bit(&p8);
    ntfAttr->oldAttributePresent = ncs_decode_8bit(&p8);
    ncs_dec_skip_space(uba, 5);
    total_bytes += 5;
    if (ntfAttr->oldAttributePresent)
    {
        total_bytes = decodeNtfValueT(uba, total_bytes,
                                      ntfAttr->attributeType,
                                      &ntfAttr->oldAttributeValue);
    }
    total_bytes = decodeNtfValueT(uba, total_bytes,
                                  ntfAttr->attributeType,
                                  &ntfAttr->newAttributeValue);
    return total_bytes;
}

static uns32 decodeSaNtfAttribute(NCS_UBAID *uba, uns32 total_bytes, SaNtfAttributeT* ntfAttr)
{
    uns8        *p8;
    uns8       local_data[4];

    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    ntfAttr->attributeId = ncs_decode_16bit(&p8);
    ntfAttr->attributeType = ncs_decode_16bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;

    total_bytes = decodeNtfValueT(uba, total_bytes, 
                                  ntfAttr->attributeType,
                                  &ntfAttr->attributeValue);
    return total_bytes;
}

static uns32 encodeSaNameT(NCS_UBAID *uba,
                           uns8 *p8, 
                           uns32 total_bytes,
                           SaNameT* name)
{
    p8 = ncs_enc_reserve_space(uba, 2);
    if (!p8)
    {
        TRACE("p8 NULL!!!");
        return 0;
    }
    ncs_encode_16bit(&p8, name->length);
    ncs_enc_claim_space(uba, 2);
    total_bytes += 2;

    ncs_encode_n_octets_in_uba(uba, name->value, 
                               (uns32)name->length); 
    total_bytes += (uns32)name->length;   
    return total_bytes;
}

static uns32 ntfsv_enc_not_header(NCS_UBAID *uba, SaNtfNotificationHeaderT *param)
{
    uns8 *p8;
    uns32 total_bytes = 0;

    /* additional text */
    p8 = ncs_enc_reserve_space(uba, 10);
    if (!p8)
    {
        goto error_done;
    }
    ncs_encode_16bit(&p8, param->numCorrelatedNotifications);
    ncs_encode_16bit(&p8, param->lengthAdditionalText);
    ncs_encode_16bit(&p8, param->numAdditionalInfo);
    ncs_encode_32bit(&p8, *param->eventType);
    ncs_enc_claim_space(uba, 10);
    total_bytes += 10;

    total_bytes = encodeSaNameT(uba, p8, total_bytes, param->notificationObject);
    if (total_bytes == 0)
    {
        goto error_done;
    }    

    total_bytes = encodeSaNameT(uba, p8, total_bytes, param->notifyingObject);
    if (total_bytes == 0)
    {
        goto error_done;
    }

    p8 = ncs_enc_reserve_space(uba, 8);
    if (!p8)
    {
        goto error_done;
    }
    ncs_encode_32bit(&p8, param->notificationClassId->vendorId);
    ncs_encode_16bit(&p8, param->notificationClassId->majorId);
    ncs_encode_16bit(&p8, param->notificationClassId->minorId);
    ncs_enc_claim_space(uba, 8);
    total_bytes += 8;

    p8 = ncs_enc_reserve_space(uba, 8);
    if (!p8)
    {
        goto error_done;
    }
    ncs_encode_64bit(&p8, *param->eventTime);
    ncs_enc_claim_space(uba, 8);
    total_bytes += 8;

    p8 = ncs_enc_reserve_space(uba, 8);
    if (!p8)
    {
        goto error_done;
    }
    ncs_encode_64bit(&p8, *param->notificationId);
    ncs_enc_claim_space(uba, 8);
    total_bytes += 8;
    TRACE_2("ENC: Not ptr %p: %llu", param->notificationId,
            *param->notificationId);

    ncs_encode_n_octets_in_uba(uba, 
                               (uns8*)param->additionalText, 
                               (uns32)param->lengthAdditionalText); 
    total_bytes += (uns32)param->lengthAdditionalText;

    return total_bytes;
error_done:
    TRACE("p8 NULL");
   return 0;
}

uns32 ntfsv_enc_not_msg(NCS_UBAID *uba, ntfsv_send_not_req_t *param)
{
    uns8 *p8;
    uns32 total_bytes = 0;

    TRACE_ENTER();
    assert(uba != NULL);
    
    /** encode the contents **/
    p8 = ncs_enc_reserve_space(uba, 8);
    if (!p8)
    {
        TRACE("p8 NULL!!!");
        return 0;
    }
    ncs_encode_32bit(&p8, param->notificationType);
    ncs_encode_32bit(&p8, param->client_id);
    ncs_enc_claim_space(uba, 8);
    total_bytes += 8;
    
    switch (param->notificationType)
    {
        int i;
     case SA_NTF_TYPE_ALARM:
         TRACE("enc SA_NTF_TYPE_ALARM");
         ntfsv_enc_not_header(uba, &param->notification.alarm.notificationHeader);
         p8 = ncs_enc_reserve_space(uba, 8);
         if (!p8)
         {
             TRACE("p8 NULL!!!");
             return 0;
         }
         ncs_encode_32bit(&p8, *param->notification.alarm.probableCause);
         ncs_encode_32bit(&p8, *param->notification.alarm.perceivedSeverity);
         ncs_enc_claim_space(uba, 8);
         total_bytes += 8;
         break;
    case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
        TRACE("enc SA_NTF_TYPE_OBJECT_CREATE_DELETE");
        ntfsv_enc_not_header(uba, &param->notification.objectCreateDelete.
                             notificationHeader);
        p8 = ncs_enc_reserve_space(uba, 4);
        if (!p8)
        {
            TRACE("p8 NULL!!!");
            return 0;
        }
        ncs_encode_16bit(&p8, param->notification.objectCreateDelete.numAttributes);
        ncs_encode_16bit(&p8, *param->notification.objectCreateDelete.sourceIndicator);
        TRACE_2("sourceIndicator %d",
                (int)*param->notification.objectCreateDelete.sourceIndicator);
        ncs_enc_claim_space(uba, 4);
        total_bytes += 4;
        ntfsv_print_object_attributes(param->notification. \
                                objectCreateDelete.objectAttributes,
                                param->notification.objectCreateDelete.numAttributes);

        for (i=0; i < param->notification.objectCreateDelete.numAttributes; i++)
        {
            total_bytes = encodeSaNtfAttribute(uba,
                                 p8,
                                 total_bytes,
                                 &param->notification.objectCreateDelete.objectAttributes[i]);
            if (total_bytes == 0)
            {
                goto error_done;
            }
        }
        break;
    case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
        TRACE("enc SA_NTF_TYPE_ATTRIBUTE_CHANGE");
        ntfsv_enc_not_header(uba, &param->notification.attributeChange.
                             notificationHeader);
        p8 = ncs_enc_reserve_space(uba, 4);
        if (!p8)
        {
            TRACE("p8 NULL!!!");
            return 0;
        }
        ncs_encode_16bit(&p8, param->notification.attributeChange.numAttributes);
        ncs_encode_16bit(&p8, *param->notification.attributeChange.sourceIndicator);
        ncs_enc_claim_space(uba, 4);
        total_bytes += 4;
        for (i=0; i < param->notification.attributeChange.numAttributes; i++)
        {
            total_bytes = 
                encodeSaNtfAttributeChangeT(uba,
                                            p8,
                                            total_bytes,
                                            &param->notification. \
                                            attributeChange.changedAttributes [i]);
        }
        break;
    case SA_NTF_TYPE_STATE_CHANGE:
        TRACE("enc SA_NTF_TYPE_STATE_CHANGE");
        ntfsv_enc_not_header(uba, &param->notification.stateChange.
                             notificationHeader);
        p8 = ncs_enc_reserve_space(uba, 4);
        if (!p8)
        {
            TRACE("p8 NULL!!!");
            return 0;
        }
        TRACE("enc numStCh: %d", (int)param->notification.stateChange.numStateChanges);
        ncs_encode_16bit(&p8, param->notification.stateChange.numStateChanges);
        ncs_encode_16bit(&p8, *param->notification.stateChange.sourceIndicator);
        ncs_enc_claim_space(uba, 4);
        total_bytes += 4;
        for (i=0; i< param->notification.stateChange.numStateChanges; i++)
        {
            p8 = ncs_enc_reserve_space(uba, 3);
            if (!p8){TRACE("p8 NULL!!!");return 0;}
            ncs_encode_16bit(&p8, param->notification.stateChange.changedStates[i].stateId);
            ncs_encode_8bit(&p8, param->notification.stateChange.changedStates[i].oldStatePresent);
            ncs_enc_claim_space(uba, 3);
            total_bytes += 3;
            if (param->notification.stateChange.changedStates[i].oldStatePresent)
            {
                p8 = ncs_enc_reserve_space(uba, 2);
                if (!p8){TRACE("p8 NULL!!!");return 0;}
                ncs_encode_16bit(&p8, param->notification.stateChange.
                                 changedStates[i].oldState);
                ncs_enc_claim_space(uba, 2);
                total_bytes += 2;
            }
            p8 = ncs_enc_reserve_space(uba, 2);
            if (!p8){TRACE("p8 NULL!!!");return 0;}
            ncs_encode_16bit(&p8, param->notification.stateChange.changedStates[i].newState);
            ncs_enc_claim_space(uba, 2);
            total_bytes += 2;
        }
        break;
    case SA_NTF_TYPE_SECURITY_ALARM:
        TRACE("enc SA_NTF_TYPE_SECURITY_ALARM");
        ntfsv_enc_not_header(uba, &param->notification.securityAlarm.
                             notificationHeader);
        p8 = ncs_enc_reserve_space(uba, 10);
        if (!p8)
        {
            TRACE("p8 NULL!!!");
            return 0;
        }
        ncs_encode_32bit(&p8, *param->notification.securityAlarm.probableCause);
        ncs_encode_32bit(&p8, *param->notification.securityAlarm.severity);
        ncs_encode_16bit(&p8, param->notification.securityAlarm.securityAlarmDetector->valueType);
        ncs_enc_claim_space(uba, 10);
        total_bytes += 10;
        total_bytes += encodeSaNtfValueT(uba,
                                         p8,
                                         total_bytes,
                                         param->notification.securityAlarm.securityAlarmDetector->valueType,
                                         &param->notification.securityAlarm.securityAlarmDetector->value);
        p8 = ncs_enc_reserve_space(uba, 2);
        TRACE_2("enc Security Alarm Detector Type: %d\n",
                 param->notification.securityAlarm.securityAlarmDetector->valueType);
        TRACE_2("enc Security Alarm Detector Value: %d\n",
                     param->notification.securityAlarm.securityAlarmDetector->value.int32Val);

        if (!p8)
        {
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
        if (!p8)
        {
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
        TRACE("notificationType: %d not valid",(int)param->notificationType);
        return 0;
        break;
    }
    
    done:
        TRACE_LEAVE();
    return total_bytes;

    error_done:
        TRACE_1("encoding error");
    goto done;
}

static uns32 ntfsv_dec_not_header(NCS_UBAID *uba, SaNtfNotificationHeaderT *param)
{
    uns8        *p8;
    uns32       total_bytes = 0;
    uns8       local_data[10];
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
    ntfsv_alloc_ntf_header(param, numCorrelatedNotifications,
                           lengthAdditionalText,
                           numAdditionalInfo);

    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    *param->eventType = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;

    /* saNameT  */
    p8 = ncs_dec_flatten_space(uba, local_data, 2);
    param->notificationObject->length = ncs_decode_16bit(&p8);
    ncs_dec_skip_space(uba, 2);
    total_bytes += 2;
    ncs_decode_n_octets_from_uba(uba, param->notificationObject->value,
                                 (uns32)param->notificationObject->length);
    total_bytes += (uns32)param->notificationObject->length;
    /* saNameT  */
    p8 = ncs_dec_flatten_space(uba, local_data, 2);
    param->notifyingObject->length = ncs_decode_16bit(&p8);
    ncs_dec_skip_space(uba, 2);
    total_bytes += 2;
    ncs_decode_n_octets_from_uba(uba, param->notifyingObject->value,
                                 (uns32)param->notifyingObject->length);
    total_bytes += (uns32)param->notifyingObject->length;

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
    TRACE_2("DEC: Not ptr %p: %llu", param->notificationId,
            *param->notificationId);

    if (param->lengthAdditionalText > 0)
    {
        /* dealloc in ntfs_evt.c: proc_send_not_msg */
        ncs_decode_n_octets_from_uba(uba,
                                     (uns8*) param->additionalText,
                                     (uns32)param->lengthAdditionalText);
        total_bytes += (uns32)param->lengthAdditionalText;
        param->additionalText[param->lengthAdditionalText-1]= '\0';
    }
    TRACE_LEAVE();
    return total_bytes;
}

uns32 ntfsv_dec_not_msg(NCS_UBAID *uba, ntfsv_send_not_req_t *param)
{
    uns8        *p8;
    uns32       total_bytes = 0;
    uns8       local_data[10];

    TRACE_ENTER();

    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    param->notificationType = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;
    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    param->client_id = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;

    switch (param->notificationType)
    {
        int i;
        
        case SA_NTF_TYPE_ALARM:
            TRACE("dec SA_NTF_TYPE_ALARM");
            total_bytes += ntfsv_dec_not_header(uba, &param->notification.alarm.
                                                notificationHeader);
            p8 = ncs_dec_flatten_space(uba, local_data, 8);
            ntfsv_alloc_ntf_alarm(&param->notification.alarm, 0, 0, 0);
            *param->notification.alarm.probableCause = ncs_decode_32bit(&p8);
            *param->notification.alarm.perceivedSeverity = ncs_decode_32bit(&p8);
            ncs_dec_skip_space(uba, 8);
            total_bytes += 8;
            break;

        case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
            TRACE("dec SA_NTF_TYPE_OBJECT_CREATE_DELETE");

            total_bytes += ntfsv_dec_not_header(uba, &param->notification.
                                                objectCreateDelete.notificationHeader);
            p8 = ncs_dec_flatten_space(uba, local_data, 4);
            param->notification.objectCreateDelete.numAttributes = 
                ncs_decode_16bit(&p8);
            ntfsv_alloc_ntf_obj_create_del(&param->notification.objectCreateDelete,
                                           param->notification.objectCreateDelete.numAttributes);
            *param->notification.objectCreateDelete.sourceIndicator = 
                ncs_decode_16bit(&p8);
            TRACE_2("sourceIndicator %d",
                    (int)*param->notification.objectCreateDelete.sourceIndicator);
            ncs_dec_skip_space(uba, 4);
            total_bytes += 4;

            if (param->notification.objectCreateDelete.numAttributes > 0)
            {
                /* dealloc in ntfs_evt.c: proc_send_not_msg */
                param->notification.objectCreateDelete.objectAttributes = 
                    (SaNtfAttributeT*) calloc(1,param->notification.
                           objectCreateDelete.numAttributes \
                           *sizeof(SaNtfAttributeT));
                if (NULL == param->notification.objectCreateDelete.objectAttributes) {
                    TRACE_1("calloc failed");
                    return 0;
                }

                for (i=0; i < param->notification.objectCreateDelete.numAttributes; i++)
                {
                    total_bytes = 
                        decodeSaNtfAttribute(uba,
                                             total_bytes,
                                             &param->notification.objectCreateDelete.objectAttributes[i]);
                }
                ntfsv_print_object_attributes(param->notification. \
                                        objectCreateDelete.objectAttributes,
                                        param->notification.objectCreateDelete. \
                                        numAttributes);
            }
            break;
        case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
            TRACE("dec A_NTF_TYPE_ATTRIBUTE_CHANGE");
            total_bytes += ntfsv_dec_not_header(uba, &param->notification.attributeChange.
                                                notificationHeader);
            p8 = ncs_dec_flatten_space(uba, local_data, 4);
            param->notification.attributeChange.numAttributes = 
                ncs_decode_16bit(&p8);
            ntfsv_alloc_ntf_attr_change(&param->notification.attributeChange,
                                        param->notification.objectCreateDelete.numAttributes);
            *param->notification.attributeChange.sourceIndicator = 
                ncs_decode_16bit(&p8);
            ncs_dec_skip_space(uba, 4);
            total_bytes += 4;
            if (param->notification.attributeChange.numAttributes != 0) 
            {
                param->notification.attributeChange.changedAttributes = 
                    (SaNtfAttributeChangeT *) calloc(1,
                        param->notification.attributeChange.numAttributes * \
                        sizeof (SaNtfAttributeChangeT));
                if (param->notification.attributeChange.changedAttributes == NULL) {
                    TRACE_1("calloc failed");
                    return 0;
                }
            } 
            else 
            {
                param->notification.attributeChange.changedAttributes = NULL;
            }
            for (i=0; i < param->notification.attributeChange.numAttributes; i++)
            {
                total_bytes = 
                    decodeSaNtfAttributeChangeT(uba,
                                                total_bytes,
                                                &param->notification. \
                                                attributeChange.changedAttributes [i]);
            }
            break;
        case SA_NTF_TYPE_STATE_CHANGE:
            TRACE("dec SA_NTF_TYPE_STATE_CHANGE");
            total_bytes += ntfsv_dec_not_header(uba, &param->notification.stateChange.
                                               notificationHeader);
            p8 = ncs_dec_flatten_space(uba, local_data, 4);
            param->notification.stateChange.numStateChanges = ncs_decode_16bit(&p8);
            ntfsv_alloc_ntf_state_change(&param->notification.stateChange,
                                         param->notification.stateChange.numStateChanges);
            *param->notification.stateChange.sourceIndicator = ncs_decode_16bit(&p8);
            ncs_dec_skip_space(uba, 4);
            total_bytes += 4;
            TRACE("dec numStCh: %d", (int)param->notification.stateChange.numStateChanges);
            param->notification.stateChange.changedStates = 
                (SaNtfStateChangeT*) calloc(1, sizeof (SaNtfStateChangeT)*
                                            param->notification.
                                            stateChange.numStateChanges);
            if (NULL == param->notification.stateChange.changedStates)
            {
                TRACE_1("calloc failed");
                return 0;
            }
            for (i=0; i< param->notification.stateChange.numStateChanges; i++)
            {
                p8 = ncs_dec_flatten_space(uba, local_data, 3);
                param->notification.stateChange.changedStates[i].stateId = ncs_decode_16bit(&p8);
                param->notification.stateChange.changedStates[i].oldStatePresent= ncs_decode_8bit(&p8);
                ncs_dec_skip_space(uba, 3);
                total_bytes += 3;
                if (param->notification.stateChange.changedStates[i].oldStatePresent)
                {
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
            total_bytes += ntfsv_dec_not_header(uba, &param->notification.securityAlarm.
                                               notificationHeader);
            p8 = ncs_dec_flatten_space(uba, local_data, 10);
            ntfsv_alloc_ntf_security_alarm(&param->notification.securityAlarm);
            *param->notification.securityAlarm.probableCause = ncs_decode_32bit(&p8);
            *param->notification.securityAlarm.severity = ncs_decode_32bit(&p8);
            param->notification.securityAlarm.securityAlarmDetector->valueType =
                 ncs_decode_16bit(&p8);
            ncs_dec_skip_space(uba, 10);
            total_bytes += 10;
            total_bytes = decodeNtfValueT(uba, total_bytes, 
                                          param->notification.securityAlarm.
                                          securityAlarmDetector->valueType,
                                          &param->notification.securityAlarm.
                                          securityAlarmDetector->value);
            p8 = ncs_dec_flatten_space(uba, local_data, 2);
            TRACE_2("dec Security Alarm Detector Type: %d\n",
                     param->notification.securityAlarm.securityAlarmDetector->valueType);
            TRACE_2("dec Security Alarm Detector Value: %d\n",
                    param->notification.securityAlarm.securityAlarmDetector->value.int32Val);

            param->notification.securityAlarm.serviceUser->valueType = ncs_decode_16bit(&p8);
            ncs_dec_skip_space(uba, 2);
            total_bytes += 2;
            total_bytes = decodeNtfValueT(uba, total_bytes, 
                                          param->notification.securityAlarm.
                                          serviceUser->valueType,
                                          &param->notification.securityAlarm.
                                          serviceUser->value);
            p8 = ncs_dec_flatten_space(uba, local_data, 2);
            param->notification.securityAlarm.serviceProvider->valueType = 
                ncs_decode_16bit(&p8);
            ncs_dec_skip_space(uba, 2);
            total_bytes += 2;
            total_bytes = decodeNtfValueT(uba, total_bytes, 
                                          param->notification.
                                          securityAlarm.serviceProvider->valueType,
                                          &param->notification.securityAlarm.
                                          serviceProvider->value);

            break;
        default:
            TRACE("notificationType not valid");
            return 0;
            break;
    }

    TRACE_LEAVE();
    return total_bytes;
}

uns32 ntfsv_enc_subscribe_msg(NCS_UBAID *uba, ntfsv_subscribe_req_t *param)
{
    uns8 *p8;
    uns32 total_bytes = 0;

    TRACE_ENTER();
    assert(uba != NULL);

    /** encode the contents **/
    p8 = ncs_enc_reserve_space(uba, 8);
    if (!p8)
    {
        TRACE("p8 NULL!!!");
        return 0;
    }
    ncs_encode_32bit(&p8, param->client_id);
    ncs_encode_32bit(&p8, param->subscriptionId);
    ncs_enc_claim_space(uba, 8);
    total_bytes += 8;

    TRACE_LEAVE();
    return total_bytes;
}

uns32 ntfsv_dec_subscribe_msg(NCS_UBAID *uba, ntfsv_subscribe_req_t *param)
{
    uns8        *p8;
    uns32       total_bytes = 0;
    uns8       local_data[4];

    /* client_id  */
    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    param->client_id          = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;

    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    param->subscriptionId = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;

    TRACE_8("NTFSV_SUBSCRIBE_REQ");
    return total_bytes;
}

uns32 ntfsv_enc_64bit_msg(NCS_UBAID *uba, uns64 param)
{
    uns8 *p8;
    uns32 total_bytes = 0;

    TRACE_ENTER();
    assert(uba != NULL);

    /** encode the contents **/
    p8 = ncs_enc_reserve_space(uba, 8);
    if (!p8)
    {
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
    uns8        *p8;
    uns32       total_bytes = 0;
    uns8        local_data[8];

    p8 = ncs_dec_flatten_space(uba, local_data, 8);
    *param = ncs_decode_64bit(&p8);
    ncs_dec_skip_space(uba, 8);
    total_bytes += 8;
    TRACE_8("NTFSV_ENC_64bit");
    return total_bytes;
}

uns32 ntfsv_enc_32bit_msg(NCS_UBAID *uba, uns32 param)
{
    uns8 *p8;
    uns32 total_bytes = 0;

    TRACE_ENTER();
    assert(uba != NULL);

    /** encode the contents **/
    p8 = ncs_enc_reserve_space(uba, 4);
    if (!p8)
    {
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
    uns8        *p8;
    uns32       total_bytes = 0;
    uns8        local_data[4];

    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    *param = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;
    TRACE_8("NTFSV_ENC_32bit");
    return total_bytes;
}

uns32 ntfsv_enc_unsubscribe_msg(NCS_UBAID *uba, ntfsv_unsubscribe_req_t *param)
{
    uns8 *p8;
    uns32 total_bytes = 0;
    assert(uba != NULL);

    /** encode the contents **/
    p8 = ncs_enc_reserve_space(uba, 8);
    if (!p8)
    {
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
    uns8        *p8;
    uns32       total_bytes = 0;
    uns8        local_data[8];

    /* client_id, lstr_id, lstr_open_id */
    p8 = ncs_dec_flatten_space(uba, local_data, 8);
    param->client_id         = ncs_decode_32bit(&p8);
    param->subscriptionId = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 8);
    total_bytes += 8;

    TRACE_8("NTFSV_UNSUBSCRIBE_REQ");
    return total_bytes;
}
