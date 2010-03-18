#include "clmsv_enc_dec.h"
#include "clmsv_msg.h"

uns32 decodeSaNameT(NCS_UBAID *uba,SaNameT *name)
{
        uns8 local_data[2];
	uns8 *p8 = NULL;
	uns32 total_bytes =0;

        p8 = ncs_dec_flatten_space(uba, local_data, 2);
        name->length = ncs_decode_16bit(&p8);
        if (name->length > SA_MAX_NAME_LENGTH) {
                LOG_ER("SaNameT length too long: %hd", name->length);
                /* this should not happen */
        }
        ncs_dec_skip_space(uba, 2);
        total_bytes += 2;
        ncs_decode_n_octets_from_uba(uba, name->value, (uns32)name->length);
        total_bytes += name->length;
        return total_bytes;
}




uns32 decodeNodeAddressT(NCS_UBAID *uba,SaClmNodeAddressT * nodeAddress)
{
        uns8 local_data[5];
	uns8 *p8 = NULL;
	uns32 total_bytes =0;

        p8 = ncs_dec_flatten_space(uba, local_data, 4);
        nodeAddress->family = ncs_decode_32bit(&p8);
        if ( nodeAddress->family < SA_CLM_AF_INET || nodeAddress->family > SA_CLM_AF_INET6) {
                LOG_ER("nodeAddress->family is wrong: %hd", nodeAddress->family);
                /* this should not happen */
        }
        ncs_dec_skip_space(uba, 4);
        total_bytes += 4;

        p8 = ncs_dec_flatten_space(uba, local_data, 2);
        nodeAddress->length = ncs_decode_16bit(&p8);
        if (nodeAddress->length > SA_MAX_NAME_LENGTH) {
                LOG_ER("nodeAddress->length length too long: %hd", nodeAddress->length);
                /* this should not happen */
        }
        ncs_dec_skip_space(uba, 2);
        total_bytes += 2;
        ncs_decode_n_octets_from_uba(uba, nodeAddress->value, (uns32)nodeAddress->length);
        total_bytes += nodeAddress->length;
        return total_bytes;

}


uns32 encodeSaNameT(NCS_UBAID *uba, SaNameT *name)
{
	TRACE_ENTER();
	uns8 *p8 = NULL;
	uns32 total_bytes = 0;
        p8 = ncs_enc_reserve_space(uba, 2);
        if (!p8) {
                TRACE("p8 NULL!!!");
                return 0;
        }
        if (name->length > SA_MAX_NAME_LENGTH) {
                LOG_ER("SaNameT length too long %hd", name->length);
        }
        ncs_encode_16bit(&p8, name->length);
        ncs_enc_claim_space(uba, 2);
        total_bytes += 2;
        ncs_encode_n_octets_in_uba(uba, name->value, (uns32)name->length);
        total_bytes += (uns32)name->length;
	TRACE_LEAVE();
        return total_bytes;
}

