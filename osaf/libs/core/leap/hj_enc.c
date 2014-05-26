/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008-2014 The OpenSAF Foundation
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
 * Author(s): Emerson Network Power, Ericsson
 *
 */

/*****************************************************************************
..............................................................................

  MODULE NAME: NCS_ENC.C  

  REVISION HISTORY:

  Date     Version  Name          Description
  -------- -------  ------------  --------------------------------------------
  10-15-97 1.00A    H&J (DS)      Original

  $Id: 

  $Log:

..............................................................................

  DESCRIPTION:

  This module contains useful encoding function used during construction of
  LUNI 1.0 control frames.

..............................................................................


*******************************************************************************
*/

#include <ncsgl_defs.h>
#include "ncs_osprm.h"
#include "ncsencdec_pub.h"
#include "ncssysf_def.h"
#include "ncssysf_mem.h"
#include "ncs_svd.h"

/** A NULL os implies "count" number of zeros...
 **/

USRBUF *ncs_encode_n_octets(USRBUF *u, uint8_t *os, unsigned int count)
{
	uint8_t *p;
	unsigned int remaining = count;
	unsigned int offset = 0;

	if (remaining > PAYLOAD_BUF_SIZE)
		count = PAYLOAD_BUF_SIZE;
	else
		count = remaining;

	do {
		if ((p = m_MMGR_RESERVE_AT_END(&u, count, uint8_t *)) != (uint8_t *)0) {
			/*
			 * Build the octet string...Remember a NULL pointer
			 * indicates an all-zero octet-string...
			 */
			if (os == (uint8_t *)0)
				memset((char *)p, '\0', (size_t)count);
			else
				memcpy((char *)p, (char *)(os + offset), (size_t)count);
		} else {
			break;
		}
		remaining = remaining - count;
		offset = offset + count;

		if (remaining > PAYLOAD_BUF_SIZE)
			count = PAYLOAD_BUF_SIZE;
		else
			count = remaining;

	} while (remaining > 0);
	return u;
}

USRBUF *ncs_encode_uns8(USRBUF *u, uint8_t val8)
{
	uint8_t *p8;

	if ((p8 = m_MMGR_RESERVE_AT_END(&u, 1, uint8_t *)) != (uint8_t *)0)
		*p8 = val8;

	return u;
}

USRBUF *ncs_encode_uns16(USRBUF *u, uint16_t val16)
{
	uint8_t *p16;

	if ((p16 = m_MMGR_RESERVE_AT_END(&u, 2, uint8_t *)) != (uint8_t *)0) {
		*p16++ = (uint8_t)(val16 >> 8);
		*p16 = (uint8_t)(val16);
	}

	return u;
}

USRBUF *ncs_encode_uns32(USRBUF *u, uint32_t val32)
{
	uint8_t *p32;

	if ((p32 = m_MMGR_RESERVE_AT_END(&u, 4, uint8_t *)) != (uint8_t *)0) {
		*p32++ = (uint8_t)(val32 >> 24);
		*p32++ = (uint8_t)(val32 >> 16);
		*p32++ = (uint8_t)(val32 >> 8);
		*p32 = (uint8_t)(val32);
	}

	return u;
}

USRBUF *ncs_encode_uns64(USRBUF *u, uint64_t val64)
{
	uint8_t *p64;

	if ((p64 = m_MMGR_RESERVE_AT_END(&u, 8, uint8_t *)) != (uint8_t *)0) {
		m_NCS_OS_HTONLL_P(p64, val64);
	}

	return u;
}

USRBUF *ncs_encode_float(USRBUF *u, float obj_val)
{
	float *pfloat;

	if ((pfloat = m_MMGR_RESERVE_AT_END(&u, (uint32_t)sizeof(obj_val), float *)) != (float *)0)
		 m_NCS_ENCODE_FLOAT(obj_val, pfloat);

	return u;
}

USRBUF *ncs_prepend_n_octets(USRBUF *pbuf, uint8_t *os, unsigned int length)
{
	uint8_t *pch;

	pch = m_MMGR_RESERVE_AT_START(&pbuf, length, uint8_t *);
	if (pch == NULL) {
		m_LEAP_DBG_SINK_VOID;
		return BNULL;
	}

	memcpy((char *)pch, (char *)os, (size_t)length);
	return (pbuf);
}

USRBUF *ncs_prepend_uns16(USRBUF *pbuf, uint16_t val16)
{
	uint8_t *p16;

	p16 = m_MMGR_RESERVE_AT_START(&pbuf, (uint32_t)sizeof(uint16_t), uint8_t *);
	if (p16 == NULL) {
		m_LEAP_DBG_SINK_VOID;
		return BNULL;
	}
	*p16++ = (uint8_t)(val16 >> 8);
	*p16 = (uint8_t)(val16);

	return (pbuf);
}

USRBUF *ncs_prepend_uns32(USRBUF *pbuf, uint32_t val32)
{
	uint8_t *p32;

	p32 = m_MMGR_RESERVE_AT_START(&pbuf, (uint32_t)sizeof(uint32_t), uint8_t *);
	if (p32 == NULL) {
		m_LEAP_DBG_SINK_VOID;
		return BNULL;
	}
	*p32++ = (uint8_t)(val32 >> 24);
	*p32++ = (uint8_t)(val32 >> 16);
	*p32++ = (uint8_t)(val32 >> 8);
	*p32 = (uint8_t)(val32);

	return (pbuf);
}

USRBUF *ncs_prepend_uns64(USRBUF *pbuf, uint64_t val64)
{
	uint8_t *p64;

	p64 = m_MMGR_RESERVE_AT_START(&pbuf, 8, uint8_t *);
	if (p64 == NULL) {
		m_LEAP_DBG_SINK_VOID;
		return BNULL;
	}

	m_NCS_OS_HTONLL_P(p64, val64);

	return (pbuf);
}

/*****************************************************************************
 *
 * Routines to encode 16, 24, 32, or 64 ints into network order in pdu ...
 *
 * "stream" points to the start of a n-octet  in the host order.
 * Network-order is the same as "big-endian" order (MSB first).
 * These functions has a built-in network-to-host order effect.
 *
 *****************************************************************************/

uint64_t ncs_encode_64bit(uint8_t **stream, uint64_t val)
{

	m_NCS_OS_HTONLL_P((*stream), val);
	(*stream) += 8;
	return 8;
}

uint32_t ncs_encode_32bit(uint8_t **stream, uint32_t val)
{
	*(*stream)++ = (uint8_t)(val >> 24);
	*(*stream)++ = (uint8_t)(val >> 16);
	*(*stream)++ = (uint8_t)(val >> 8);
	*(*stream)++ = (uint8_t)(val);
	return 4;
}

uint32_t ncs_encode_24bit(uint8_t **stream, uint32_t val)
{
	*(*stream)++ = (uint8_t)(val >> 16);
	*(*stream)++ = (uint8_t)(val >> 8);
	*(*stream)++ = (uint8_t)(val);
	return 3;
}

uint32_t ncs_encode_16bit(uint8_t **stream, uint32_t val)
{
	*(*stream)++ = (uint8_t)(val >> 8);
	*(*stream)++ = (uint8_t)(val);
	return 2;
}

uint32_t ncs_encode_8bit(uint8_t **stream, uint32_t val)
{
	*(*stream)++ = (uint8_t)(val);
	return 1;
}

uint32_t ncs_encode_key(uint8_t **stream, NCS_KEY *key)
{
	uint8_t len;

	*(*stream)++ = (uint8_t)(key->svc);
	*(*stream)++ = (uint8_t)(key->fmat);
	*(*stream)++ = (uint8_t)(key->type);

	switch (key->fmat) {
	case NCS_FMT_NUM:
		break;

	case NCS_FMT_STR:
		len = (uint8_t)strlen((char *)key->val.str);
		ncs_encode_8bit(stream, len);
		ncs_encode_octets(stream, key->val.str, len);
		return (3 + len + 1);
		break;

	case NCS_FMT_OCT:
		len = key->val.oct.len;
		ncs_encode_8bit(stream, len);
		ncs_encode_octets(stream, key->val.oct.data, len);
		return (3 + len + 1);
		break;
	}
	return m_LEAP_DBG_SINK(0);
}

uint32_t ncs_encode_octets(uint8_t **stream, uint8_t *val, uint32_t count)
{
	uint32_t i;
	for (i = 0; i < count; i++)
		*(*stream)++ = *val++;
	return count;
}

/***** new style (2014) encoding/decoding functions follows ******/

static uint8_t *encode_reserve_space(NCS_UBAID *ub, int32_t count)
{
    uint8_t *p8 = ncs_enc_reserve_space(ub, count);
    osafassert(p8);
    return p8;
}

static uint8_t *decode_flatten_space(NCS_UBAID *uba, uint8_t *os, int32_t count)
{
	uint8_t *p8 = ncs_dec_flatten_space(uba, os, count);
	osafassert(p8);
	return p8;
}

void osaf_encode_uint8(NCS_UBAID *ub, uint8_t value)
{
	uint8_t *p8 = encode_reserve_space(ub, 1);
	ncs_encode_8bit(&p8, value);
	ncs_enc_claim_space(ub, 1);
}

void osaf_decode_uint8(NCS_UBAID *ub, uint8_t *to)
{
	uint8_t buf[1];

	uint8_t *p8 = decode_flatten_space(ub, buf, 1);
	*to = ncs_decode_8bit(&p8);
	ncs_dec_skip_space(ub, 1);
}

void osaf_encode_uint16(NCS_UBAID *ub, uint16_t value)
{
	uint8_t *p8 = encode_reserve_space(ub, 2);
	ncs_encode_16bit(&p8, value);
	ncs_enc_claim_space(ub, 2);
}

void osaf_decode_uint16(NCS_UBAID *ub, uint16_t *to)
{
	uint8_t buf[2];

	uint8_t *p8 = decode_flatten_space(ub, buf, 2);
	*to = ncs_decode_16bit(&p8);
	ncs_dec_skip_space(ub, 2);
}

void osaf_encode_uint32(NCS_UBAID *ub, uint32_t value)
{
	uint8_t *p8 = encode_reserve_space(ub, 4);
	ncs_encode_32bit(&p8, value);
	ncs_enc_claim_space(ub, 4);
}

void osaf_decode_uint32(NCS_UBAID *ub, uint32_t *to)
{
	uint8_t buf[4];

	uint8_t *p8 = decode_flatten_space(ub, buf, 4);
	*to = ncs_decode_32bit(&p8);
	ncs_dec_skip_space(ub, 4);
}

void osaf_encode_uint64(NCS_UBAID *ub, uint64_t value)
{
	uint8_t *p8 = encode_reserve_space(ub, 8);
	ncs_encode_64bit(&p8, value);
	ncs_enc_claim_space(ub, 8);
}

void osaf_decode_uint64(NCS_UBAID *ub, uint64_t *to)
{
	uint8_t buf[8];

	uint8_t *p8 = decode_flatten_space(ub, buf, 8);
	*to = ncs_decode_64bit(&p8);
	ncs_dec_skip_space(ub, 8);
}

void osaf_encode_sanamet(NCS_UBAID *ub, const SaNameT *name)
{
	int i;
	osaf_encode_uint16(ub, name->length);
	for (i = 0; i < SA_MAX_NAME_LENGTH; i++)
		osaf_encode_uint8(ub, name->value[i]);
}

void osaf_decode_sanamet(NCS_UBAID *ub, SaNameT *name)
{
	int i;
	osaf_decode_uint16(ub, &name->length);
	for (i = 0; i < SA_MAX_NAME_LENGTH; i++)
		osaf_decode_uint8(ub, &name->value[i]);
}

void osaf_encode_satimet(NCS_UBAID *ub, SaTimeT time)
{
	osaf_encode_uint64(ub, time);
}

void osaf_decode_satimet(NCS_UBAID *ub, SaTimeT *time)
{
	osaf_decode_uint64(ub, (uint64_t*)time);
}

void osaf_encode_bool(NCS_UBAID *ub, bool value)
{
	// for backwards compatibility reasons a bool is encoded as 4 bytes
	uint32_t tmp = value;
	osaf_encode_uint32(ub, tmp);
}

void osaf_decode_bool(NCS_UBAID *ub, bool *to)
{
	uint32_t value;
	osaf_decode_uint32(ub, &value);
	*to = (bool)value;
}

