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

  DESCRIPTION: Implementation of common EDU programs (of common LEAP
  data structures) for NCS_EDU service.

******************************************************************************
*/

#include <ncsgl_defs.h>
#include "ncs_osprm.h"
#include "patricia.h"
#include "ncs_stack.h"
#include "ncssysf_mem.h"
#include "ncsencdec_pub.h"
#include "ncs_edu.h"
#include "sysf_def.h"
#include "ncs_ubaid.h"

extern char gl_log_string[];

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_ncs_bool

  DESCRIPTION:      EDU program handler for "boolean" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "boolean" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_ncs_bool(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		       uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	NCS_BOOL u32 = 0;
	NCS_BOOL *uptr = NULL;

	/* Note that, NCS_BOOL is defined as "unsigned int" in ncsgl_defs.h. */
	switch (op) {
	case EDP_OP_TYPE_ENC:
		u32 = *(NCS_BOOL *)ptr;
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, 4);
			ncs_encode_32bit(&p8, u32);
			ncs_enc_claim_space(buf_env->info.uba, 4);
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			ncs_encode_tlv_32bit(&p8, u32);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 4);
		}
#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Encoded NCS_BOOL: 0x%x \n", u32);
		ncs_edu_log_msg(gl_log_string);
#endif
		break;
	case EDP_OP_TYPE_DEC:
		{
			if (*(NCS_BOOL **)ptr == NULL) {
				/* Since "ncs_edp_ncs_bool" is the responsibility of LEAP, LEAP
				   is supposed to malloc this memory. */
				(*(NCS_BOOL **)ptr) = uptr = m_MMGR_ALLOC_EDP_NCS_BOOL;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(NCS_BOOL));
			} else {
				uptr = *((NCS_BOOL **)ptr);
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u32, 4);
				u32 = ncs_decode_32bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 4);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u32 = ncs_decode_tlv_32bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 4);
			}
			*uptr = u32;
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Decoded NCS_BOOL: 0x%x \n", u32);
			ncs_edu_log_msg(gl_log_string);
#endif
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			if (ptr != NULL) {
				/* We need populate the pointer here, so that it can
				   be sent back to the parent invoker(most likely
				   to be able to perform the TEST condition). */
				uptr = m_MMGR_ALLOC_EDP_NCS_BOOL;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(NCS_BOOL));
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u32, 4);
				u32 = ncs_decode_32bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 4);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u32 = ncs_decode_tlv_32bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 4);
			}
			if (uptr != NULL) {
				*uptr = u32;
				(*(NCS_BOOL **)ptr) = uptr;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Populated NCS_BOOL in PPDB: 0x%x \n", u32);
				ncs_edu_log_msg(gl_log_string);
			}
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "PP NCS_BOOL: 0x%x \n", u32);
			ncs_edu_log_msg(gl_log_string);
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_uns8

  DESCRIPTION:      EDU program handler for "uns8" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "uns8" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_uns8(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		   uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	uns8 u8 = 0x00;
	uns16 len = 0;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	uns32 i = 0;
#endif

	switch (op) {
	case EDP_OP_TYPE_ENC:
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, 1);
			ncs_encode_8bit(&p8, *(uns8 *)ptr);
			ncs_enc_claim_space(buf_env->info.uba, 1);
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Encoded uns8: 0x%x \n", *(uns8 *)ptr);
			ncs_edu_log_msg(gl_log_string);
#endif
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			if (*ptr_data_len <= 1) {
				*ptr_data_len = 1;
			}
			/* Multiple instances of "uns8" to be encoded */
			ncs_encode_tlv_n_octets(&p8, (uns8 *)ptr, (uns8)*ptr_data_len);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + (*ptr_data_len));
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Encoded uns8: len = %d\n", *ptr_data_len);
			ncs_edu_log_msg(gl_log_string);
			for (i = 0; i < *ptr_data_len; i++) {
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				if (i == 0)
					sprintf(gl_log_string, "Encoded uns8: 0x%x \n", *(uns8 *)ptr);
				else
					sprintf(gl_log_string, "        uns8: 0x%x \n", *(uns8 *)((uns8 *)ptr + i));
				ncs_edu_log_msg(gl_log_string);
			}
#endif
		}
		break;
	case EDP_OP_TYPE_DEC:
		{
			uns8 *uptr = NULL;

			len = 1;	/* default */
			if (!buf_env->is_ubaid) {
				/* Look into length of the Octet-stream first. */
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				p8++;
				len = ((uns16)*(p8)++) << 8;
				len |= (uns16)*(p8)++;
			}

			if (*(uns8 **)ptr == NULL) {
				/* Since "ncs_edp_uns8" is the responsibility of LEAP, LEAP
				   is supposed to malloc this memory. */
				(*(uns8 **)ptr) = uptr = m_MMGR_ALLOC_EDP_UNS8(len);
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', len * sizeof(uns8));
			} else {
				uptr = *((uns8 **)ptr);
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, &u8, 1);
				u8 = ncs_decode_8bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 1);
				*uptr = u8;
#if(NCS_EDU_VERBOSE_PRINT == 1)
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Decoded uns8: 0x%x \n", u8);
				ncs_edu_log_msg(gl_log_string);
#endif
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				ncs_decode_tlv_n_octets(p8, (uns8 *)uptr, len);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + len);

#if(NCS_EDU_VERBOSE_PRINT == 1)
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Decoded uns8: len = %d\n", len);
				ncs_edu_log_msg(gl_log_string);
				for (i = 0; i < len; i++) {
					memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
					if (i == 0)
						sprintf(gl_log_string, "Decoded uns8: 0x%x \n", *(uns8 *)uptr);
					else
						sprintf(gl_log_string,
							"        uns8: 0x%x \n", *(uns8 *)((uns8 *)uptr + i));
					ncs_edu_log_msg(gl_log_string);
				}
#endif
			}
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			uns8 *uptr = NULL;

			len = 1;	/* default */
			if (!buf_env->is_ubaid) {
				/* Look into length of the Octet-stream first. */
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				p8++;
				len = ((uns16)*(p8)++) << 8;
				len |= (uns16)*(p8)++;
			}

			uptr = m_MMGR_ALLOC_EDP_UNS8(len);
			if (uptr == NULL) {
				/* Memory failure. */
				*o_err = EDU_ERR_MEM_FAIL;
				return NCSCC_RC_FAILURE;
			}
			memset(uptr, '\0', len);

			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, &u8, 1);
				u8 = ncs_decode_8bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 1);
				*uptr = u8;
#if(NCS_EDU_VERBOSE_PRINT == 1)
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "PP uns8: 0x%x \n", u8);
				ncs_edu_log_msg(gl_log_string);
#endif
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				ncs_decode_tlv_n_octets(p8, (uns8 *)uptr, len);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + len);

#if(NCS_EDU_VERBOSE_PRINT == 1)
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "PP uns8: len = %d\n", len);
				ncs_edu_log_msg(gl_log_string);
				for (i = 0; i < len; i++) {
					memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
					if (i == 0)
						sprintf(gl_log_string, "PP uns8: 0x%x \n", *(uns8 *)uptr);
					else
						sprintf(gl_log_string, "   uns8: 0x%x \n", *(uns8 *)((uns8 *)uptr + i));
					ncs_edu_log_msg(gl_log_string);
				}
#endif
			}

			if (ptr != NULL) {
				(*(uns8 **)ptr) = uptr;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Populated uns8 in PPDB: len = %d\n", len);
				ncs_edu_log_msg(gl_log_string);
			} else {
				/* Free uptr */
				m_NCS_MEM_FREE(uptr, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0);
			}
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_uns16

  DESCRIPTION:      EDU program handler for "uns16" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "uns16" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_uns16(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		    uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	uns16 u16, len = 1, byte_cnt = 0;

	switch (op) {
	case EDP_OP_TYPE_ENC:
		u16 = *(uns16 *)ptr;
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, 2);
			ncs_encode_16bit(&p8, u16);
			ncs_enc_claim_space(buf_env->info.uba, 2);
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Encoded uns16: 0x%x\n", u16);
			ncs_edu_log_msg(gl_log_string);
#endif
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			if (*ptr_data_len <= 1) {
				*ptr_data_len = 1;
			}
			/* "*ptr_data_len" instances of "uns16" to be encoded */
			byte_cnt = (uns16)ncs_encode_tlv_n_16bit(&p8, (uns16 *)ptr, (uns16)*ptr_data_len);
			ncs_edu_skip_space(&buf_env->info.tlv_env, byte_cnt);
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Encoded uns16: 0x%x\n", u16);
			ncs_edu_log_msg(gl_log_string);
#endif
		}
		break;
	case EDP_OP_TYPE_DEC:
		{
			uns16 *uptr = NULL;

			len = 1;	/* default */
			if (!buf_env->is_ubaid) {
				/* Look into length of the Octet-stream first. */
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				p8++;
				len = ((uns16)*(p8)++) << 8;
				len |= (uns16)*(p8)++;
			}

			if (*(uns16 **)ptr == NULL) {
				/* Since "ncs_edp_uns16" is the responsibility of LEAP, LEAP
				   is supposed to malloc this memory. */
				(*(uns16 **)ptr) = uptr = m_MMGR_ALLOC_EDP_UNS8(sizeof(uns16) * len);
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', (sizeof(uns16) * len));
			} else {
				uptr = *((uns16 **)ptr);
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u16, 2);
				u16 = ncs_decode_16bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 2);
				*uptr = u16;
#if(NCS_EDU_VERBOSE_PRINT == 1)
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Decoded uns16: 0x%x\n", u16);
				ncs_edu_log_msg(gl_log_string);
#endif
			} else {
				uns32 uns16_cnt = 0;

				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				uns16_cnt = ncs_decode_tlv_n_16bit(&p8, uptr);
				ncs_edu_skip_space(&buf_env->info.tlv_env,
						   EDU_TLV_HDR_SIZE + (sizeof(uns16) * uns16_cnt));
#if(NCS_EDU_VERBOSE_PRINT == 1)
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Decoded uns16: 0x%x\n", (*uptr));
				ncs_edu_log_msg(gl_log_string);
#endif
			}
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			uns16 *uptr = NULL;

			len = 1;	/* default */
			if (!buf_env->is_ubaid) {
				/* Look into length of the Octet-stream first. */
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				p8++;
				len = ((uns16)*(p8)++) << 8;
				len |= (uns16)*(p8)++;
			}

			uptr = m_MMGR_ALLOC_EDP_UNS8(sizeof(uns16) * len);
			if (uptr == NULL) {
				/* Memory failure. */
				*o_err = EDU_ERR_MEM_FAIL;
				return NCSCC_RC_FAILURE;
			}
			memset(uptr, '\0', sizeof(uns16) * len);

			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u16, 2);
				u16 = ncs_decode_16bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 2);
				*uptr = u16;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "PP uns16: 0x%x\n", u16);
				ncs_edu_log_msg(gl_log_string);
			} else {
				uns32 uns16_cnt = 0;

				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				uns16_cnt = ncs_decode_tlv_n_16bit(&p8, uptr);
				ncs_edu_skip_space(&buf_env->info.tlv_env,
						   EDU_TLV_HDR_SIZE + (sizeof(uns16) * uns16_cnt));
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "PP uns16: 0x%x\n", (*uptr));
				ncs_edu_log_msg(gl_log_string);
			}

			if (ptr != NULL) {
				(*(uns16 **)ptr) = uptr;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Populated uns16 in PPDB: 0x%x \n", (*uptr));
				ncs_edu_log_msg(gl_log_string);
			} else {
				m_NCS_MEM_FREE(uptr, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0);
			}
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_uns32

  DESCRIPTION:      EDU program handler for "uns32" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "uns32" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_uns32(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		    uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	uns32 u32 = 0x00000000;
	uns16 len = sizeof(uns8);

	switch (op) {
	case EDP_OP_TYPE_ENC:
		u32 = *(uns32 *)ptr;
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, 4);
			ncs_encode_32bit(&p8, u32);
			ncs_enc_claim_space(buf_env->info.uba, 4);
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Encoded uns32: 0x%x \n", u32);
			ncs_edu_log_msg(gl_log_string);
#endif
		} else {
			uns32 byte_cnt = 0;
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			if (*ptr_data_len <= 1) {
				*ptr_data_len = 1;
			}
			byte_cnt = ncs_encode_tlv_n_32bit(&p8, (uns32 *)ptr, (uns16)(*ptr_data_len));
			ncs_edu_skip_space(&buf_env->info.tlv_env, byte_cnt);
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Encoded uns32: 0x%x \n", u32);
			ncs_edu_log_msg(gl_log_string);
#endif
		}
		break;
	case EDP_OP_TYPE_DEC:
		{
			uns32 *uptr = NULL;

			if (!buf_env->is_ubaid) {
				/* Look into length of the "uns32" first. */
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				p8++;
				len = ((uns16)*(p8)++) << 8;
				len |= (uns16)*(p8)++;
			}

			if (*(uns32 **)ptr == NULL) {
				/* Since "ncs_edp_uns32" is the responsibility of LEAP, LEAP
				   is supposed to malloc this memory. */
				(*(uns32 **)ptr) = uptr = m_MMGR_ALLOC_EDP_UNS8(4 * len);
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', 4 * len);
			} else {
				uptr = *((uns32 **)ptr);
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u32, 4);
				u32 = ncs_decode_32bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 4);
				*uptr = u32;
#if(NCS_EDU_VERBOSE_PRINT == 1)
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Decoded uns32: 0x%x \n", u32);
				ncs_edu_log_msg(gl_log_string);
#endif
			} else {
				uns32 uns32_cnt = 0;

				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				uns32_cnt = ncs_decode_tlv_n_32bit(&p8, uptr);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + (4 * uns32_cnt));
#if(NCS_EDU_VERBOSE_PRINT == 1)
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Decoded uns32: 0x%x \n", (*uptr));
				ncs_edu_log_msg(gl_log_string);
#endif
			}
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			uns32 *uptr = NULL;

			if (!buf_env->is_ubaid) {
				/* Look into length of the "uns32" first. */
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				p8++;
				len = ((uns16)*(p8)++) << 8;
				len |= (uns16)*(p8)++;
			}
			uptr = m_MMGR_ALLOC_EDP_UNS8(4 * len);
			if (uptr == NULL) {
				/* Memory failure. */
				*o_err = EDU_ERR_MEM_FAIL;
				return NCSCC_RC_FAILURE;
			}
			memset(uptr, '\0', 4 * len);
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u32, 4);
				u32 = ncs_decode_32bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 4);
				*uptr = u32;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "PP uns32: 0x%x \n", u32);
				ncs_edu_log_msg(gl_log_string);
			} else {
				uns32 uns32_cnt = 0;

				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				uns32_cnt = ncs_decode_tlv_n_32bit(&p8, uptr);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + (4 * uns32_cnt));
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "PP uns32: 0x%x \n", (*uptr));
				ncs_edu_log_msg(gl_log_string);
			}
			if (ptr != NULL) {
				(*(uns32 **)ptr) = uptr;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Populated uns32 in PPDB: 0x%x \n", (*uptr));
				ncs_edu_log_msg(gl_log_string);
			} else {
				m_NCS_MEM_FREE(uptr, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0);
			}
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_char

  DESCRIPTION:      EDU program handler for "char" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "char" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_char(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		   uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;

	switch (op) {
	case EDP_OP_TYPE_ENC:
		{
			if (buf_env->is_ubaid) {
				p8 = ncs_enc_reserve_space(buf_env->info.uba, 1);
				ncs_encode_8bit(&p8, *(uns8 *)ptr);
				ncs_enc_claim_space(buf_env->info.uba, 1);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				ncs_encode_tlv_8bit(&p8, *(uns8 *)ptr);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 1);
			}
#if(NCS_EDU_VERBOSE_PRINT == 1)
			if (((char *)ptr)[0] != '\0') {
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Encoded char: %c \n", ((char *)ptr)[0]);
				ncs_edu_log_msg(gl_log_string);
			}
#endif
		}
		break;
	case EDP_OP_TYPE_DEC:
		{
			char *uptr = NULL;

			*ptr_data_len = 1;

			if (*(uns8 **)ptr == NULL) {
				/* Since "ncs_edp_char" is the responsibility of LEAP, LEAP
				   is supposed to malloc this memory. */
				(*(char **)ptr) = uptr = m_MMGR_ALLOC_EDP_CHAR(*ptr_data_len);
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', (sizeof(char) * (*ptr_data_len)));
			} else {
				uptr = *((char **)ptr);
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)uptr, *ptr_data_len);
				memcpy(uptr, p8, *ptr_data_len);
				ncs_dec_skip_space(buf_env->info.uba, *ptr_data_len);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				*uptr = ncs_decode_tlv_8bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + (*ptr_data_len));
			}
#if(NCS_EDU_VERBOSE_PRINT == 1)
			if (((char *)uptr)[0] != '\0') {
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Decoded char: %c \n", ((char *)uptr)[0]);
				ncs_edu_log_msg(gl_log_string);
			}
#endif
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			char *uptr = NULL;

			*ptr_data_len = 1;
			uptr = m_MMGR_ALLOC_EDP_CHAR(*ptr_data_len);
			if (uptr == NULL) {
				/* Memory failure. */
				*o_err = EDU_ERR_MEM_FAIL;
				return NCSCC_RC_FAILURE;
			}
			memset(uptr, '\0', (sizeof(char) * (*ptr_data_len)));

			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)uptr, *ptr_data_len);
				memcpy(uptr, p8, *ptr_data_len);
				ncs_dec_skip_space(buf_env->info.uba, *ptr_data_len);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				*uptr = ncs_decode_tlv_8bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + (*ptr_data_len));
			}

			if (((char *)uptr)[0] != '\0') {
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "PP char: %c \n", ((char *)uptr)[0]);
				ncs_edu_log_msg(gl_log_string);
			}

			if (ptr != NULL) {
				(*(char **)ptr) = uptr;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Populated char in PPDB: %c \n", ((char *)uptr)[0]);
				ncs_edu_log_msg(gl_log_string);
			} else {
				m_NCS_MEM_FREE(uptr, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0);
			}
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_string

  DESCRIPTION:      EDU program handler for "string" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "string" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_string(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		     uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8, *src_p8;
	uns16 len = 0, byte_cnt = 0;

	switch (op) {
	case EDP_OP_TYPE_ENC:
		if (ptr_data_len == NULL)
			return NCSCC_RC_FAILURE;

		/* Length of data to be encoded is "*ptr_data_len" */
		len = (uns16)(*ptr_data_len);
		src_p8 = (uns8 *)ptr;
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, 2);
			ncs_encode_16bit(&p8, len);
			ncs_enc_claim_space(buf_env->info.uba, 2);
			if (len != 0) {
				ncs_encode_n_octets_in_uba(buf_env->info.uba, src_p8, len);
			}
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			byte_cnt = (uns16)ncs_encode_tlv_n_octets(&p8, src_p8, (uns8)len);
			ncs_edu_skip_space(&buf_env->info.tlv_env, byte_cnt);
		}
#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Encoded string: len = 0x%x\n", len);
		ncs_edu_log_msg(gl_log_string);
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Encoded string: %s\n", src_p8);
		ncs_edu_log_msg(gl_log_string);
#endif
		break;
	case EDP_OP_TYPE_DEC:
		if (ptr_data_len == NULL)
			return NCSCC_RC_FAILURE;

		if (buf_env->is_ubaid) {
			p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&len, 2);
			len = ncs_decode_16bit(&p8);
			ncs_dec_skip_space(buf_env->info.uba, 2);
			*ptr_data_len = len;
		} else {
			/* Look into length of the string first. */
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			p8++;
			len = ((uns16)*(p8)++) << 8;
			len |= (uns16)*(p8)++;
			if (len == 0) {
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE);
#if(NCS_EDU_VERBOSE_PRINT == 1)
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Decoded string: len = 0x%x\n", len);
				ncs_edu_log_msg(gl_log_string);
#endif
			}
		}

		if (len != 0) {
			char *uptr = NULL;

			if (*(char **)ptr == NULL) {
				/* Since "ncs_edp_string" is the responsibility of LEAP, LEAP
				   is supposed to malloc this memory. */
				(*(char **)ptr) = uptr = m_MMGR_ALLOC_EDP_CHAR(len + 1);
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', (len * sizeof(char) + 1));
			} else {
				uptr = *((char **)ptr);
			}

			if (buf_env->is_ubaid) {
				if (ncs_decode_n_octets_from_uba(buf_env->info.uba,
								 (uns8 *)uptr, len) != NCSCC_RC_SUCCESS) {
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				ncs_decode_tlv_n_octets(p8, (uns8 *)uptr, (uns32)len);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + len);
			}

#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Decoded string: len = 0x%x\n", len);
			ncs_edu_log_msg(gl_log_string);
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Decoded string: %s\n", uptr);
			ncs_edu_log_msg(gl_log_string);
#endif
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			if (ptr_data_len == NULL)
				return NCSCC_RC_FAILURE;

			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&len, 2);
				len = ncs_decode_16bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 2);
			} else {
				/* Look into length of the string first. */
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				p8++;
				len = (uns16)*(p8)++ << 8;
				len |= (uns16)*(p8)++;
				if (len == 0)
					ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE);
			}
			*ptr_data_len = len;

			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "PP string: len = 0x%x\n", len);
			ncs_edu_log_msg(gl_log_string);

			if (len != 0) {
				char *uptr = NULL;

				uptr = m_MMGR_ALLOC_EDP_CHAR(len + 1);
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', (len * sizeof(char) + 1));
				/* This string pointer(uptr) is now null-terminated. */

				if (buf_env->is_ubaid) {
					if (ncs_decode_n_octets_from_uba(buf_env->info.uba,
									 (uns8 *)uptr, len) != NCSCC_RC_SUCCESS) {
						*o_err = EDU_ERR_MEM_FAIL;
						return NCSCC_RC_FAILURE;
					}
				} else {
					p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
					ncs_decode_tlv_n_octets(p8, (uns8 *)uptr, (uns32)len);
					ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + len);
				}

				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "PP string: %s\n", uptr);
				ncs_edu_log_msg(gl_log_string);

				if (ptr != NULL) {
					(*(char **)ptr) = uptr;
					memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
					sprintf(gl_log_string, "Populated string in PPDB: %s \n", uptr);
					ncs_edu_log_msg(gl_log_string);
				} else {
					m_NCS_MEM_FREE(uptr, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0);
				}
			}
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_short

  DESCRIPTION:      EDU program handler for "short" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "short" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_short(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		    uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	short u16;

	switch (op) {
	case EDP_OP_TYPE_ENC:
		u16 = *(uns16 *)ptr;
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, 2);
			ncs_encode_16bit(&p8, u16);
			ncs_enc_claim_space(buf_env->info.uba, 2);
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			ncs_encode_tlv_16bit(&p8, u16);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 2);
		}
#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Encoded short: 0x%x\n", u16);
		ncs_edu_log_msg(gl_log_string);
#endif
		break;
	case EDP_OP_TYPE_DEC:
		{
			short *uptr = NULL;

			if (*(short **)ptr == NULL) {
				/* Since "ncs_edp_short" is the responsibility of LEAP, LEAP
				   is supposed to malloc this memory. */
				(*(short **)ptr) = uptr = m_MMGR_ALLOC_EDP_SHORT;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(short));
			} else {
				uptr = *((short **)ptr);
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u16, 2);
				u16 = ncs_decode_16bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 2);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u16 = ncs_decode_tlv_16bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 2);
			}
			*uptr = u16;
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Decoded short: 0x%x\n", u16);
			ncs_edu_log_msg(gl_log_string);
#endif
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			short *uptr = NULL;

			if (ptr != NULL) {
				/* We need populate the pointer here, so that it can
				   be sent back to the parent invoker(most likely
				   to be able to perform the TEST condition). */
				uptr = m_MMGR_ALLOC_EDP_SHORT;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(short));
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u16, 2);
				u16 = ncs_decode_16bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 2);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u16 = ncs_decode_tlv_16bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 2);
			}

			if (uptr != NULL) {
				*uptr = u16;
				(*(short **)ptr) = uptr;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Populated short in PPDB: 0x%x \n", u16);
				ncs_edu_log_msg(gl_log_string);
			}
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "PP short: 0x%x\n", u16);
			ncs_edu_log_msg(gl_log_string);
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_int

  DESCRIPTION:      EDU program handler for "int" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "int" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_int(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		  uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	uns32 u32 = 0x00000000;

	switch (op) {
	case EDP_OP_TYPE_ENC:
		u32 = (uns32)*(int *)ptr;	/* Typecast int to uns32 */
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, 4);
			ncs_encode_32bit(&p8, u32);
			ncs_enc_claim_space(buf_env->info.uba, 4);
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			ncs_encode_tlv_32bit(&p8, u32);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 4);
		}

#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Encoded int: %d \n", u32);
		ncs_edu_log_msg(gl_log_string);
#endif
		break;
	case EDP_OP_TYPE_DEC:
		{
			int *uptr = NULL;

			if (*(int **)ptr == NULL) {
				/* Since "ncs_edp_int" is the responsibility of LEAP, LEAP
				   is supposed to malloc this memory. */
				(*(int **)ptr) = uptr = m_MMGR_ALLOC_EDP_INT;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(int));
			} else {
				uptr = *((int **)ptr);
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u32, 4);
				u32 = ncs_decode_32bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 4);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u32 = ncs_decode_tlv_32bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 4);
			}
			*uptr = (int)u32;	/* Typecast uns32 to int */
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Decoded int: %d \n", u32);
			ncs_edu_log_msg(gl_log_string);
#endif
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			int *uptr = NULL;

			if (ptr != NULL) {
				/* We need populate the pointer here, so that it can
				   be sent back to the parent invoker(most likely
				   to be able to perform the TEST condition). */
				uptr = m_MMGR_ALLOC_EDP_INT;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(int));
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u32, 4);
				u32 = ncs_decode_32bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 4);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u32 = ncs_decode_tlv_32bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 4);
			}

			if (uptr != NULL) {
				*uptr = (int)u32;	/* Typecast uns32 to int */
				(*(int **)ptr) = uptr;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Populated int in PPDB: %d \n", u32);
				ncs_edu_log_msg(gl_log_string);
			}
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "PP int: %d \n", u32);
			ncs_edu_log_msg(gl_log_string);
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_int8

  DESCRIPTION:      EDU program handler for "int8" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "int8" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_int8(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		   uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	int8 u8 = 0x00;

	switch (op) {
	case EDP_OP_TYPE_ENC:
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, 1);
			ncs_encode_8bit(&p8, *(int8 *)ptr);
			ncs_enc_claim_space(buf_env->info.uba, 1);
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			ncs_encode_tlv_8bit(&p8, *(int8 *)ptr);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 1);
		}
#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Encoded int8: 0x%x \n", *(int8 *)ptr);
		ncs_edu_log_msg(gl_log_string);
#endif
		break;
	case EDP_OP_TYPE_DEC:
		{
			int8 *uptr = NULL;

			if (*(int8 **)ptr == NULL) {
				(*(int8 **)ptr) = uptr = m_MMGR_ALLOC_EDP_INT8;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(int8));
			} else {
				uptr = *((int8 **)ptr);
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u8, 1);
				u8 = ncs_decode_8bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 1);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u8 = ncs_decode_tlv_8bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 1);
			}
			*uptr = u8;
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Decoded int8: 0x%x \n", u8);
			ncs_edu_log_msg(gl_log_string);
#endif
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			int8 *uptr = NULL;

			if (ptr != NULL) {
				/* We need populate the pointer here, so that it can
				   be sent back to the parent invoker(most likely
				   to be able to perform the TEST condition). */
				uptr = m_MMGR_ALLOC_EDP_INT8;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(uns8));
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u8, 1);
				u8 = ncs_decode_8bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 1);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u8 = ncs_decode_tlv_8bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 1);
			}

			if (uptr != NULL) {
				*uptr = u8;
				(*(int8 **)ptr) = uptr;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Populated int8 in PPDB: 0x%x \n", u8);
				ncs_edu_log_msg(gl_log_string);
			}
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "PP int8: 0x%x\n", u8);
			ncs_edu_log_msg(gl_log_string);
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_int16

  DESCRIPTION:      EDU program handler for "int16" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "int16" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_int16(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		    uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	int16 u16;

	switch (op) {
	case EDP_OP_TYPE_ENC:
		u16 = *(int16 *)ptr;
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, 2);
			ncs_encode_16bit(&p8, u16);
			ncs_enc_claim_space(buf_env->info.uba, 2);
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			ncs_encode_tlv_16bit(&p8, u16);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 2);
		}
#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Encoded int16: 0x%x\n", u16);
		ncs_edu_log_msg(gl_log_string);
#endif
		break;
	case EDP_OP_TYPE_DEC:
		{
			int16 *uptr = NULL;

			if (*(int16 **)ptr == NULL) {
				(*(int16 **)ptr) = uptr = m_MMGR_ALLOC_EDP_INT16;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(int16));
			} else {
				uptr = *((int16 **)ptr);
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u16, 2);
				u16 = ncs_decode_16bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 2);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u16 = ncs_decode_tlv_16bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 2);
			}
			*uptr = u16;
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Decoded int16: 0x%x\n", u16);
			ncs_edu_log_msg(gl_log_string);
#endif
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			int16 *uptr = NULL;

			if (ptr != NULL) {
				/* We need populate the pointer here, so that it can
				   be sent back to the parent invoker(most likely
				   to be able to perform the TEST condition). */
				uptr = m_MMGR_ALLOC_EDP_INT16;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(int16));
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u16, 2);
				u16 = ncs_decode_16bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 2);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u16 = ncs_decode_tlv_16bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 2);
			}

			if (uptr != NULL) {
				*uptr = u16;
				(*(int16 **)ptr) = uptr;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Populated int16 in PPDB: 0x%x \n", u16);
				ncs_edu_log_msg(gl_log_string);
			}
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "PP int16: 0x%x\n", u16);
			ncs_edu_log_msg(gl_log_string);
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_int32

  DESCRIPTION:      EDU program handler for "int32" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "int32" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_int32(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		    uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	int32 u32 = 0x00000000;

	switch (op) {
	case EDP_OP_TYPE_ENC:
		u32 = *(int32 *)ptr;
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, 4);
			ncs_encode_32bit(&p8, u32);
			ncs_enc_claim_space(buf_env->info.uba, 4);
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			ncs_encode_tlv_32bit(&p8, u32);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 4);
		}
#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Encoded int32: 0x%x \n", (int)u32);
		ncs_edu_log_msg(gl_log_string);
#endif
		break;
	case EDP_OP_TYPE_DEC:
		{
			int32 *uptr = NULL;

			if (*(int32 **)ptr == NULL) {
				(*(int32 **)ptr) = uptr = m_MMGR_ALLOC_EDP_INT32;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(int32));
			} else {
				uptr = *((int32 **)ptr);
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u32, 4);
				u32 = ncs_decode_32bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 4);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u32 = ncs_decode_tlv_32bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 4);
			}
			*uptr = u32;
#if(NCS_EDU_VERBOSE_PRINT == 1)
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Decoded int32: 0x%x \n", (int)u32);
			ncs_edu_log_msg(gl_log_string);
#endif
		}
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		{
			int32 *uptr = NULL;

			if (ptr != NULL) {
				/* We need populate the pointer here, so that it can
				   be sent back to the parent invoker(most likely
				   to be able to perform the TEST condition). */
				uptr = m_MMGR_ALLOC_EDP_INT32;
				if (uptr == NULL) {
					/* Memory failure. */
					*o_err = EDU_ERR_MEM_FAIL;
					return NCSCC_RC_FAILURE;
				}
				memset(uptr, '\0', sizeof(int32));
			}
			if (buf_env->is_ubaid) {
				p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)&u32, 4);
				u32 = ncs_decode_32bit(&p8);
				ncs_dec_skip_space(buf_env->info.uba, 4);
			} else {
				p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
				u32 = ncs_decode_tlv_32bit(&p8);
				ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + 4);
			}
			if (uptr != NULL) {
				*uptr = u32;
				(*(int32 **)ptr) = uptr;
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				sprintf(gl_log_string, "Populated int32 in PPDB: 0x%x \n", (int)u32);
				ncs_edu_log_msg(gl_log_string);
			}
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "PP int32: 0x%x \n", (int)u32);
			ncs_edu_log_msg(gl_log_string);
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_ncs_key

  DESCRIPTION:      EDU program handler for "NCS_KEY" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "NCS_KEY" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_ncs_key(EDU_HDL *hdl, EDU_TKN *edu_tkn,
		      NCSCONTEXT ptr, uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	NCS_KEY *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET ncs_key_rules[] = {
		{EDU_START, ncs_edp_ncs_key, 0, 0, 0, sizeof(NCS_KEY), 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_KEY *)0)->svc, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0, (long)&((NCS_KEY *)0)->type, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0, (long)&((NCS_KEY *)0)->fmat, 0, NULL},

		{EDU_TEST, ncs_edp_uns8, 0, 0, 0,
		 (long)&((NCS_KEY *)0)->fmat, 0, ncs_edu_ncs_key_test_fmat_fnc},

		{EDU_EXEC, ncs_edp_uns32, 0, 0, EDU_EXIT,
		 (long)&((NCS_KEY *)0)->val.num, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, EDU_EXIT,
		 (long)&((NCS_KEY *)0)->val.str, SYSF_MAX_KEY_LEN, NULL},

		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((NCS_KEY *)0)->val.oct.len, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, EDU_EXIT,
		 (long)&((NCS_KEY *)0)->val.oct.data, SYSF_MAX_KEY_LEN, NULL},

		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (NCS_KEY *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (NCS_KEY **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(NCS_KEY));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = ptr;
	}
	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, ncs_key_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);
	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edu_ncs_key_test_fmat_fnc

  DESCRIPTION:      Test function, that gets invoked from the EDP
                    function "ncs_edp_ncs_key".

  RETURNS:          char *, denoting the label to execute next.

*****************************************************************************/
int ncs_edu_ncs_key_test_fmat_fnc(NCSCONTEXT arg)
{
	typedef enum {
		LCL_TEST_JUMP_OFFSET_FMT_NUM = NCS_FMT_NUM,
		LCL_TEST_JUMP_OFFSET_FMT_STR = NCS_FMT_STR,
		LCL_TEST_JUMP_OFFSET_FMT_OCT = NCS_FMT_OCT
	} LCL_TEST_JUMP_OFFSET;
	uns8 fmat;

	fmat = *(uns8 *)arg;

	switch (fmat) {
	case NCS_FMT_NUM:
		return LCL_TEST_JUMP_OFFSET_FMT_NUM;
	case NCS_FMT_STR:
		return LCL_TEST_JUMP_OFFSET_FMT_STR;
	case NCS_FMT_OCT:
		return LCL_TEST_JUMP_OFFSET_FMT_OCT;
	}

	return EDU_FAIL;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_uns64

  DESCRIPTION:      EDU program handler for "uns64" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "uns64" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_uns64(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		    uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	uns64 *uptr = NULL;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	uns8 *src_p8;
	uns32 cnt = 0;
#endif

	/* Note :- TLV-encoding/decoding support not put up for this data type, 
	   as no requirement yet on the TLV side arose.
	 */
	switch (op) {
	case EDP_OP_TYPE_ENC:
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, sizeof(uns64));
			ncs_encode_64bit(&p8, *(uns64 *)ptr);
			ncs_enc_claim_space(buf_env->info.uba, sizeof(uns64));
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			ncs_encode_tlv_64bit(&p8, *(uns64 *)ptr);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + sizeof(uns64));
		}

#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Encoded uns64: len = 0x%x : ", sizeof(uns64));
		ncs_edu_log_msg(gl_log_string);
		for (cnt = 0; cnt < sizeof(uns64); cnt++) {
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			if (cnt == (sizeof(uns64) - 1))
				sprintf(gl_log_string, "0x%x \n", src_p8[cnt]);
			else
				sprintf(gl_log_string, "0x%x ", src_p8[cnt]);
			ncs_edu_log_msg(gl_log_string);
		}
#endif
		break;
	case EDP_OP_TYPE_DEC:
		if (*(uns64 **)ptr == NULL) {
			/* Since "uns64" is the responsibility of LEAP, LEAP
			   is supposed to malloc this memory. */
			(*(uns64 **)ptr) = uptr = m_MMGR_ALLOC_EDP_UNS8(sizeof(uns64) / sizeof(uns8));
			if (uptr == NULL) {
				/* Memory failure. */
				*o_err = EDU_ERR_MEM_FAIL;
				return NCSCC_RC_FAILURE;
			}
			memset(uptr, '\0', sizeof(uns64));
		} else {
			uptr = *((uns64 **)ptr);
		}
		if (buf_env->is_ubaid) {
			p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)uptr, sizeof(uns64));
			*uptr = ncs_decode_64bit(&p8);
			ncs_dec_skip_space(buf_env->info.uba, sizeof(uns64));
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			*uptr = ncs_decode_tlv_64bit(&p8);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + sizeof(uns64));
		}

#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Decoded uns64: len = 0x%x : ", sizeof(uns64));
		ncs_edu_log_msg(gl_log_string);
		src_p8 = (uns8 *)uptr;
		for (cnt = 0; cnt < sizeof(uns64); cnt++) {
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			if (cnt == (sizeof(uns64) - 1))
				sprintf(gl_log_string, "0x%x \n", src_p8[cnt]);
			else
				sprintf(gl_log_string, "0x%x ", src_p8[cnt]);
			ncs_edu_log_msg(gl_log_string);
		}
#endif
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		/* We need populate the pointer here, so that it can
		   be sent back to the parent invoker(most likely
		   to be able to perform the TEST condition). */
		uptr = m_MMGR_ALLOC_EDP_UNS8(sizeof(uns64) / sizeof(uns8));
		if (uptr == NULL) {
			/* Memory failure. */
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(uptr, '\0', sizeof(uns64));
		if (buf_env->is_ubaid) {
			p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)uptr, sizeof(uns64));
			*uptr = ncs_decode_64bit(&p8);
			ncs_dec_skip_space(buf_env->info.uba, sizeof(uns64));
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			*uptr = ncs_decode_tlv_64bit(&p8);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + sizeof(uns64));
		}

		src_p8 = (uns8 *)uptr;
#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "PP uns64: len = 0x%x : ", sizeof(uns64));
		ncs_edu_log_msg(gl_log_string);
		for (cnt = 0; cnt < sizeof(uns64); cnt++) {
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			if (cnt == (sizeof(uns64) - 1))
				sprintf(gl_log_string, "0x%x \n", src_p8[cnt]);
			else
				sprintf(gl_log_string, "0x%x ", src_p8[cnt]);
			ncs_edu_log_msg(gl_log_string);
		}
#endif

		if (ptr != NULL) {
			(*(uns64 **)ptr) = uptr;

			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Populated uns64 in PPDB: len = 0x%x : ", sizeof(uns64));
			ncs_edu_log_msg(gl_log_string);
			for (cnt = 0; cnt < sizeof(uns64); cnt++) {
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				if (cnt == (sizeof(uns64) - 1))
					sprintf(gl_log_string, "0x%x \n", src_p8[cnt]);
				else
					sprintf(gl_log_string, "0x%x ", src_p8[cnt]);
				ncs_edu_log_msg(gl_log_string);
			}
		} else {
			m_NCS_MEM_FREE(uptr, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0);
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncs_edp_int64

  DESCRIPTION:      EDU program handler for "int64" data. This function
                    is invoked by EDU for performing encode/decode operation
                    on "int64" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 ncs_edp_int64(EDU_HDL *hdl, EDU_TKN *edu_tkn, NCSCONTEXT ptr,
		    uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uns8 *p8;
	int64 *uptr = NULL;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	uns8 *src_p8;
	uns32 cnt = 0;
#endif

	/* Note :- TLV-encoding/decoding support not put up for this data type, 
	   as no requirement yet on the TLV side arose.
	 */
	switch (op) {
	case EDP_OP_TYPE_ENC:
		if (buf_env->is_ubaid) {
			p8 = ncs_enc_reserve_space(buf_env->info.uba, sizeof(int64));
			ncs_encode_64bit(&p8, *(int64 *)ptr);
			ncs_enc_claim_space(buf_env->info.uba, sizeof(int64));
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			ncs_encode_tlv_64bit(&p8, *(int64 *)ptr);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + sizeof(int64));
		}

#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Encoded int64: len = 0x%x : ", sizeof(int64));
		ncs_edu_log_msg(gl_log_string);
		for (cnt = 0; cnt < sizeof(int64); cnt++) {
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			if (cnt == (sizeof(int64) - 1))
				sprintf(gl_log_string, "0x%x \n", src_p8[cnt]);
			else
				sprintf(gl_log_string, "0x%x ", src_p8[cnt]);
			ncs_edu_log_msg(gl_log_string);
		}
#endif
		break;
	case EDP_OP_TYPE_DEC:
		if (*(int64 **)ptr == NULL) {
			/* Since "int64" is the responsibility of LEAP, LEAP
			   is supposed to malloc this memory. */
			(*(int64 **)ptr) = uptr = m_MMGR_ALLOC_EDP_UNS8(sizeof(int64) / sizeof(uns8));
			if (uptr == NULL) {
				/* Memory failure. */
				*o_err = EDU_ERR_MEM_FAIL;
				return NCSCC_RC_FAILURE;
			}
			memset(uptr, '\0', sizeof(int64));
		} else {
			uptr = *((int64 **)ptr);
		}
		if (buf_env->is_ubaid) {
			p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)uptr, sizeof(int64));
			*uptr = ncs_decode_64bit(&p8);
			ncs_dec_skip_space(buf_env->info.uba, sizeof(int64));
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			*uptr = ncs_decode_tlv_64bit(&p8);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + sizeof(int64));
		}

#if(NCS_EDU_VERBOSE_PRINT == 1)
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "Decoded int64: len = 0x%x : ", sizeof(int64));
		ncs_edu_log_msg(gl_log_string);
		src_p8 = (uns8 *)uptr;
		for (cnt = 0; cnt < sizeof(int64); cnt++) {
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			if (cnt == (sizeof(int64) - 1))
				sprintf(gl_log_string, "0x%x \n", src_p8[cnt]);
			else
				sprintf(gl_log_string, "0x%x ", src_p8[cnt]);
			ncs_edu_log_msg(gl_log_string);
		}
#endif
		break;
#if(NCS_EDU_VERBOSE_PRINT == 1)
	case EDP_OP_TYPE_PP:
		/* We need populate the pointer here, so that it can
		   be sent back to the parent invoker(most likely
		   to be able to perform the TEST condition). */
		uptr = m_MMGR_ALLOC_EDP_UNS8(sizeof(int64) / sizeof(uns8));
		if (uptr == NULL) {
			/* Memory failure. */
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(uptr, '\0', sizeof(int64));
		if (buf_env->is_ubaid) {
			p8 = ncs_dec_flatten_space(buf_env->info.uba, (uns8 *)uptr, sizeof(int64));
			*uptr = ncs_decode_64bit(&p8);
			ncs_dec_skip_space(buf_env->info.uba, sizeof(int64));
		} else {
			p8 = (uns8 *)buf_env->info.tlv_env.cur_bufp;
			*uptr = ncs_decode_tlv_64bit(&p8);
			ncs_edu_skip_space(&buf_env->info.tlv_env, EDU_TLV_HDR_SIZE + sizeof(int64));
		}

		src_p8 = (uns8 *)uptr;
		memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
		sprintf(gl_log_string, "PP int64: len = 0x%x : ", sizeof(int64));
		ncs_edu_log_msg(gl_log_string);
		for (cnt = 0; cnt < sizeof(int64); cnt++) {
			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			if (cnt == (sizeof(int64) - 1))
				sprintf(gl_log_string, "0x%x \n", src_p8[cnt]);
			else
				sprintf(gl_log_string, "0x%x ", src_p8[cnt]);
			ncs_edu_log_msg(gl_log_string);
		}

		if (ptr != NULL) {
			(*(int64 **)ptr) = uptr;

			memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
			sprintf(gl_log_string, "Populated int64 in PPDB: len = 0x%x : ", sizeof(int64));
			ncs_edu_log_msg(gl_log_string);
			for (cnt = 0; cnt < sizeof(int64); cnt++) {
				memset(&gl_log_string, '\0', GL_LOG_STRING_LEN);
				if (cnt == (sizeof(int64) - 1))
					sprintf(gl_log_string, "0x%x \n", src_p8[cnt]);
				else
					sprintf(gl_log_string, "0x%x ", src_p8[cnt]);
				ncs_edu_log_msg(gl_log_string);
			}
		} else {
			m_NCS_MEM_FREE(uptr, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0);
		}
		break;
#endif
	default:
		/* Invalid Operation */
		break;
	}
	return NCSCC_RC_SUCCESS;
}
