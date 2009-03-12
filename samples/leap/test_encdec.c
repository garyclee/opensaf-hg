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

#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysf_mem.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncsencdec_pub.h>
#include <opensaf/ncs_ubaid.h>
#include "leaptest.h"

static uns32 lt_test_uba_encdec_ops(void);
static void lt_free_uba_contents(NCS_UBAID *p_uba);

/* APIs to be demonstrated :
    - ncs_encode_n_octets_in_uba(operates on NCS_UBAID envelope)
    - ncs_decode_n_octets_from_uba(operates on NCS_UBAID envelope)
    - ncs_encode_64bit (uses m_NCS_OS_HTONLL_P)
    - ncs_decode_64bit (uses m_NCS_OS_NTOHLL_P)
    - ncs_encode_32bit
    - ncs_decode_32bit
    - ncs_encode_16bit
    - ncs_decode_16bit
    - ncs_encode_8bit
    - ncs_decode_8bit
 */

int
lt_encdec_ops(int argc, char **argv)
{
    lt_test_uba_encdec_ops( );
   return NCSCC_RC_SUCCESS;
}

static uns32 lt_test_uba_encdec_ops(void)
{
    NCS_UBAID   src_uba;
    uns8        *p8;
    uns16       len = 0x0000;
    /* Source data */
    uns64       src_64bit_data;
    uns32       src_32bit_data;
    uns16       src_16bit_data;
    uns8        src_8bit_data;
    char        src_str_data[300];
    /* Destination data */
    uns64       dst_64bit_data;
    uns32       dst_32bit_data;
    uns16       dst_16bit_data;
    uns8        dst_8bit_data;
    char        dst_str_data[300];

    memset(&src_uba, '\0', sizeof(src_uba));
    memset(&src_str_data, '\0', sizeof(src_str_data));
    memset(&dst_str_data, '\0', sizeof(dst_str_data));

    /* Populate input data */
    ((uns8*)&src_64bit_data)[0] = 0x01;
    ((uns8*)&src_64bit_data)[1] = 0x02;
    ((uns8*)&src_64bit_data)[2] = 0x03;
    ((uns8*)&src_64bit_data)[3] = 0x04;
    ((uns8*)&src_64bit_data)[4] = 0x05;
    ((uns8*)&src_64bit_data)[5] = 0x06;
    ((uns8*)&src_64bit_data)[6] = 0x07;
    ((uns8*)&src_64bit_data)[7] = 0x08;
    src_32bit_data = 0x0a0b0c0d;
    src_16bit_data = 0x0e0f;
    src_8bit_data = 0x0a;
    strcpy(&src_str_data, "this is test data for input strings into NCS_UBAID.(uns64, uns32, uns16, string-data are inserted into this USRBUF.). This further tries to stretch the data over two USRBUFs so that we can test whether the LEAP routines are still resilient. To add more tests later.");
    /* Note: The strlen( ) of the above string is 265 bytes. */
    len = (uns16)strlen((char*)&src_str_data);

    /* Always do "enc-init" before starting to encode anything onto the UBA */
    ncs_enc_init_space(&src_uba);

    /* Encoding 64bit data */
    p8 = ncs_enc_reserve_space(&src_uba, sizeof(uns64));
    ncs_encode_64bit(&p8, src_64bit_data);
    ncs_enc_claim_space(&src_uba, sizeof(uns64));
    m_NCS_CONS_PRINTF("Encoded 64-bit value...\n");

    /* Encoding 32bit data */
    p8 = ncs_enc_reserve_space(&src_uba, sizeof(uns32));
    ncs_encode_32bit(&p8, src_32bit_data);
    ncs_enc_claim_space(&src_uba, sizeof(uns32));
    m_NCS_CONS_PRINTF("Encoded 32-bit value...\n");

    /* Encoding 16bit data */
    p8 = ncs_enc_reserve_space(&src_uba, sizeof(uns16));
    ncs_encode_16bit(&p8, src_16bit_data);
    ncs_enc_claim_space(&src_uba, sizeof(uns16));
    m_NCS_CONS_PRINTF("Encoded 16-bit value...\n");

    /* Encoding 8bit data */
    p8 = ncs_enc_reserve_space(&src_uba, sizeof(uns8));
    ncs_encode_8bit(&p8, src_8bit_data);
    ncs_enc_claim_space(&src_uba, sizeof(uns8));
    m_NCS_CONS_PRINTF("Encoded 8-bit value...\n");

    /* Encoding string data */
    /* Length of the string(typically 16bit) is also to be 
       encoded separately into the UBA */
    p8 = ncs_enc_reserve_space(&src_uba, 2);
    ncs_encode_16bit(&p8, len);
    ncs_enc_claim_space(&src_uba, 2);
    if(ncs_encode_n_octets_in_uba(&src_uba, (uns8*)&src_str_data, len)
        != NCSCC_RC_SUCCESS)
    {
        lt_free_uba_contents(&src_uba);
        return NCSCC_RC_FAILURE;
    }
    m_NCS_CONS_PRINTF("Encoded string of length %d...\n", len);

    /*** Now start decoding the NCS_UBAID ***/
    /* Always do "dec-init" before decoding starts */
    ncs_dec_init_space(&src_uba, src_uba.start);

    /* Decoding 64bit data */
    p8 = ncs_dec_flatten_space(&src_uba, (uns8*)&dst_64bit_data, sizeof(uns64));
    dst_64bit_data = ncs_decode_64bit(&p8);
    ncs_dec_skip_space(&src_uba, sizeof(uns64));
    m_NCS_CONS_PRINTF("Decoded 64-bit value...");
    if(memcmp((uns8*)&src_64bit_data, (uns8*)&dst_64bit_data, sizeof(uns64))
        == 0)
    {
        m_NCS_CONS_PRINTF("Decoded 64-bit value matches the encoded value...\n");
    }
    else
    {
        m_NCS_CONS_PRINTF("FAILURE : Decoded 64-bit value didn't match the encoded value!!!!\n");
        lt_free_uba_contents(&src_uba);
        return NCSCC_RC_FAILURE;
    }

    /* Decoding 32bit data */
    p8 = ncs_dec_flatten_space(&src_uba, (uns8*)&dst_32bit_data, sizeof(uns32));
    dst_32bit_data = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(&src_uba, sizeof(uns32));
    m_NCS_CONS_PRINTF("Decoded 32-bit value...");
    if(memcmp((uns8*)&src_32bit_data, (uns8*)&dst_32bit_data, sizeof(uns32))
        == 0)
    {
        m_NCS_CONS_PRINTF("Decoded 32-bit value matches the encoded value...\n");
    }
    else
    {
        m_NCS_CONS_PRINTF("FAILURE : Decoded 32-bit value didn't match the encoded value!!!!\n");
        lt_free_uba_contents(&src_uba);
        return NCSCC_RC_FAILURE;
    }

    /* Decoding 16bit data */
    p8 = ncs_dec_flatten_space(&src_uba, (uns8*)&dst_16bit_data, sizeof(uns16));
    dst_16bit_data = ncs_decode_16bit(&p8);
    ncs_dec_skip_space(&src_uba, sizeof(uns16));
    m_NCS_CONS_PRINTF("Decoded 16-bit value...");
    if(memcmp((uns8*)&src_16bit_data, (uns8*)&dst_16bit_data, sizeof(uns16))
        == 0)
    {
        m_NCS_CONS_PRINTF("Decoded 16-bit value matches the encoded value...\n");
    }
    else
    {
        m_NCS_CONS_PRINTF("FAILURE : Decoded 16-bit value didn't match the encoded value!!!!\n");
        lt_free_uba_contents(&src_uba);
        return NCSCC_RC_FAILURE;
    }

    /* Decoding 8bit data */
    p8 = ncs_dec_flatten_space(&src_uba, (uns8*)&dst_8bit_data, sizeof(uns8));
    dst_8bit_data = ncs_decode_8bit(&p8);
    ncs_dec_skip_space(&src_uba, sizeof(uns8));
    m_NCS_CONS_PRINTF("Decoded 8-bit value...");
    if(src_8bit_data == dst_8bit_data)
    {
        m_NCS_CONS_PRINTF("Decoded 8-bit value matches the encoded value...\n");
    }
    else
    {
        m_NCS_CONS_PRINTF("FAILURE : Decoded 8-bit value didn't match the encoded value!!!!\n");
        lt_free_uba_contents(&src_uba);
        return NCSCC_RC_FAILURE;
    }

    /* Decoding string data */
    p8 = ncs_dec_flatten_space(&src_uba, (uns8*)&len, 2);
    len = ncs_decode_16bit(&p8);
    ncs_dec_skip_space(&src_uba, 2);
    if(ncs_decode_n_octets_from_uba(&src_uba, (uns8*)&dst_str_data, 
        len) != NCSCC_RC_SUCCESS)
    {
        lt_free_uba_contents(&src_uba);
        return NCSCC_RC_FAILURE;
    }
    m_NCS_CONS_PRINTF("Decoded string of length %d...", len);
    if(strcmp((char*)&src_str_data, (char*)&dst_str_data) == 0)
    {
        /* Decode successful. */
        m_NCS_CONS_PRINTF("Decoded string matches the encoded string...\n");
    }
    else
    {
        m_NCS_CONS_PRINTF("FAILURE : Decoded string didn't match the encoded string!!!!\n");
        lt_free_uba_contents(&src_uba);
        return NCSCC_RC_FAILURE;
    }
    lt_free_uba_contents(&src_uba); /* Free any unconsumed memory in "src_uba" */
    return NCSCC_RC_SUCCESS;
}

static void lt_free_uba_contents(NCS_UBAID *p_uba)
{
    if(p_uba->start) {
        m_MMGR_FREE_BUFR_LIST(p_uba->start);
    }
    else {
        m_MMGR_FREE_BUFR_LIST(p_uba->ub);
    }
    /* Since "p_uba" is a stack pointer, we shouldn't be freeing it. But, if it was a malloc'ed 
       pointer, it should be freed here. */

    return;
}

