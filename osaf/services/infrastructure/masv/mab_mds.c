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

  DESCRIPTION:

  This file contains MAB general MDS routines, being:
  
  mab_mds_enc - encode a MAB message
  mab_mds_dec - decode a MAB message
  mab_mds_cpy - copy a MAB message

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/
#include "mab.h"
#include "psr_bam.h"
#include "psr.h"

#if (NCS_MAB == 1)

/****************************************************************************
 *  MAB message helper functions
 ****************************************************************************/
uns32 mas_reg_msg_encode(MAS_REG *mr, NCS_UBAID *uba);
uns32 mas_reg_msg_decode(MAS_REG *mr, NCS_UBAID *uba);
uns32 mas_unreg_msg_encode(MAS_UNREG *mu, NCS_UBAID *uba);
uns32 mas_unreg_msg_decode(MAS_UNREG *mu, NCS_UBAID *uba);
uns32 mab_fltr_encode(NCSMAB_FLTR *mf, NCS_UBAID *uba);
uns32 mab_fltr_decode(NCSMAB_FLTR *mf, NCS_UBAID *uba);
uns32 oac_fir_msg_encode(NCSMAB_IDX_FREE *ifree, NCS_UBAID *uba);
uns32 oac_fir_msg_decode(NCSMAB_IDX_FREE *ifree, NCS_UBAID *uba);
uns32 mab_encode_pcn(NCS_UBAID *uba, char *pcn);
uns32 mab_decode_pcn(NCS_UBAID *uba, char **pcn);
uns32 oac_pss_encode_warmboot_req(NCS_UBAID *uba, MAB_PSS_WARMBOOT_REQ *warmboot_req);
uns32 oac_pss_decode_warmboot_req(NCS_UBAID *uba, MAB_PSS_WARMBOOT_REQ *warmboot_req);
uns32 oac_pss_tbl_encode_bind_req(NCS_UBAID *uba, MAB_PSS_TBL_BIND_EVT *bind_req);
uns32 oac_pss_tbl_decode_bind_req(NCS_UBAID *uba, MAB_PSS_TBL_BIND_EVT *bind_req);
uns32 oac_pss_tbl_encode_unbind_req(NCS_UBAID *uba, MAB_PSS_TBL_UNBIND_EVT *unbind_req);
uns32 oac_pss_tbl_decode_unbind_req(NCS_UBAID *uba, MAB_PSS_TBL_UNBIND_EVT *unbind_req);
uns32 pss_bam_encode_warmboot_req(NCS_UBAID *uba, PSS_BAM_WARMBOOT_REQ *warmboot_req);
void mab_pss_free_pss_tbl_list(MAB_PSS_TBL_LIST *tbl_list);

/* to encode the range filter */
static uns32 ncs_mab_range_fltr_encode(NCS_UBAID *uba, NCSMAB_RANGE *range_fltr);

/* to decode the range filter */
static uns32 ncs_mab_range_fltr_decode(NCS_UBAID *uba, NCSMAB_RANGE *range_fltr);

/* to encode the exact filter */
static uns32 ncs_mab_exact_fltr_encode(NCS_UBAID *uba, NCSMAB_EXACT *exact_fltr);

/* Decodes NCSMAB_FLTR_EXACT filter */
static uns32 ncs_mab_exact_fltr_decode(NCS_UBAID *uba, NCSMAB_EXACT *exact_fltr);

/****************************************************************************
 * Function Name: mab_mds_snd
 * Purpose:       Send a MAB message using MDS V2 APIs
 ****************************************************************************/

uns32 mab_mds_snd(MDS_HDL mds_hdl, NCSCONTEXT msg, MDS_SVC_ID fr_svc, MDS_SVC_ID to_svc, MDS_DEST to_dest)
{
	NCSMDS_INFO info;

	memset(&info, 0, sizeof(info));

	info.i_mds_hdl = mds_hdl;
	info.i_svc_id = fr_svc;
	info.i_op = MDS_SEND;

	info.info.svc_send.i_msg = msg;
	info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
	info.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
	info.info.svc_send.i_to_svc = to_svc;
	info.info.svc_send.info.snd.i_to_dest = to_dest;

	return ncsmds_api(&info);
}

/****************************************************************************
 * Function Name: mab_mds_enc
 * Purpose:        encode a MAB message headed out
 ****************************************************************************/

/*#define MAB_MSG_HDR_SIZE   ((2 * sizeof(uns16)) + sizeof(uns8) + sizeof(uns32) + sizeof(MDS_DEST) + sizeof(uns32))*/
#define MAB_MSG_HDR_SIZE   ((sizeof(uns16)) + sizeof(uns8) + sizeof(uns32) + (2*(sizeof(MDS_DEST))))
#define PSS_BAM_MSG_HDR_SIZE   (sizeof(uns8))

uns32 mab_mds_enc(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg, SS_SVC_ID to_svc, NCS_UBAID *uba, uns16 msg_fmt_ver)
{
	uns8 *data;
	MAB_MSG *mm = NULL;
	PSS_BAM_MSG *pm = NULL;
	uns32 status = NCSCC_RC_SUCCESS;

	if (uba == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (to_svc == NCSMDS_SVC_ID_BAM) {
		/* Message from PSS to BAM */
		pm = (PSS_BAM_MSG *)msg;
		if (pm == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		data = ncs_enc_reserve_space(uba, PSS_BAM_MSG_HDR_SIZE);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		ncs_encode_8bit(&data, pm->i_evt);
		ncs_enc_claim_space(uba, PSS_BAM_MSG_HDR_SIZE);

		switch (pm->i_evt) {
		case PSS_BAM_EVT_WARMBOOT_REQ:
			return pss_bam_encode_warmboot_req(uba, &pm->info.warmboot_req);
			break;

		default:
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	} else {
		mm = (MAB_MSG *)msg;

		if (mm == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		data = ncs_enc_reserve_space(uba, MAB_MSG_HDR_SIZE);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		ncs_encode_16bit(&data, mm->vrid);
		mds_st_encode_mds_dest(&data, &mm->fr_card);
		mds_st_encode_mds_dest(&data, (MDS_DEST *)&mm->fr_anc);
		ncs_encode_32bit(&data, mm->fr_svc);
		ncs_encode_8bit(&data, mm->op);

		ncs_enc_claim_space(uba, MAB_MSG_HDR_SIZE);

		switch (mm->op) {
		case MAB_MAS_REQ_HDLR:
		case MAB_MAS_RSP_HDLR:
		case MAB_MAC_RSP_HDLR:
		case MAB_OAC_REQ_HDLR:
		case MAB_OAC_RSP_HDLR:
		case MAB_OAC_DEF_REQ_HDLR:
			status = ncsmib_encode(mm->data.data.snmp, uba, msg_fmt_ver);
			return status;
		case MAB_PSS_SET_REQUEST:
			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, mm->data.seq_num);
			ncs_enc_claim_space(uba, sizeof(uns32));

			return ncsmib_encode(mm->data.data.snmp, uba, msg_fmt_ver);

		case MAB_MAS_REG_HDLR:
			return mas_reg_msg_encode(&mm->data.data.reg, uba);

		case MAB_MAS_UNREG_HDLR:
			return mas_unreg_msg_encode(&mm->data.data.unreg, uba);

		case MAB_OAC_FIR_HDLR:
			return oac_fir_msg_encode(&mm->data.data.idx_free, uba);

			/* In all the following cases, there is no data to be encoded */
		case MAB_MAC_PLAYBACK_START:
			{
				uns16 cnt = 0;
				uns8 *p8_cnt = NULL;
				USRBUF *lcl_ub = NULL;

				p8_cnt = ncs_enc_reserve_space(uba, sizeof(uns16));
				if (p8_cnt == NULL)
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				ncs_enc_claim_space(uba, sizeof(uns16));

				if (mm->data.data.plbck_start.i_ub != NULL) {
					m_NCS_MEM_DBG_LOC(mm->data.data.plbck_start.i_ub);	/* Mem Leak assist */
#if 1
					/* To fix the leak introduced to fix the crash in MDS broadcast processing to multiple receivers. */
					lcl_ub = m_MMGR_DITTO_BUFR(mm->data.data.plbck_start.i_ub);
#else
					lcl_ub = mm->data.data.plbck_start.i_ub;
#endif
					cnt = (uns16)m_MMGR_LINK_DATA_LEN(lcl_ub);
					ncs_enc_append_usrbuf(uba, lcl_ub);
				}
				ncs_encode_16bit(&p8_cnt, cnt);
			}
			break;

		case MAB_MAC_PLAYBACK_END:
		case MAB_OAC_PLAYBACK_START:
		case MAB_OAC_PLAYBACK_END:
		case MAB_OAA_DOWN_EVT:
		case MAB_MAS_ASYNC_DONE:
		case MAB_SVC_VDEST_ROLE_QUIESCED:
			break;

		case MAB_OAC_PSS_ACK:
			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, mm->data.seq_num);
			ncs_enc_claim_space(uba, sizeof(uns32));

			break;

		case MAB_OAC_PSS_EOP_EVT:
			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, mm->data.data.oac_pss_eop_ind.mib_key);
			ncs_enc_claim_space(uba, sizeof(uns32));

			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, mm->data.data.oac_pss_eop_ind.wbreq_hdl);
			ncs_enc_claim_space(uba, sizeof(uns32));

			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, mm->data.data.oac_pss_eop_ind.status);
			ncs_enc_claim_space(uba, sizeof(uns32));
			break;

		case MAB_PSS_WARM_BOOT:
			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, mm->data.seq_num);
			ncs_enc_claim_space(uba, sizeof(uns32));

			return oac_pss_encode_warmboot_req(uba, &mm->data.data.oac_pss_warmboot_req);
			break;

		case MAB_PSS_TBL_BIND:
			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, mm->data.seq_num);
			ncs_enc_claim_space(uba, sizeof(uns32));

			return oac_pss_tbl_encode_bind_req(uba, &mm->data.data.oac_pss_tbl_bind);
			break;

		case MAB_PSS_TBL_UNBIND:
			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, mm->data.seq_num);
			ncs_enc_claim_space(uba, sizeof(uns32));

			return oac_pss_tbl_encode_unbind_req(uba, &mm->data.data.oac_pss_tbl_unbind);
			break;

		default:
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mab_mds_dec
 * Purpose:        decode a MAB message coming in
 ****************************************************************************/

uns32 mab_mds_dec(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT *msg, SS_SVC_ID to_svc, NCS_UBAID *uba, uns16 msg_fmt_ver)
{
	uns8 *data;
	MAB_MSG *mm;
	uns8 data_buff[MAB_MSG_HDR_SIZE];

	if (uba == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (msg == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	mm = m_MMGR_ALLOC_MAB_MSG;
	if (mm == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	memset(mm, '\0', sizeof(MAB_MSG));

	*msg = mm;

	data = ncs_dec_flatten_space(uba, data_buff, MAB_MSG_HDR_SIZE);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	mm->vrid = ncs_decode_16bit(&data);
	mds_st_decode_mds_dest(&data, &mm->fr_card);
	mds_st_decode_mds_dest(&data, &mm->fr_anc);
	mm->fr_svc = ncs_decode_32bit(&data);
	mm->op = ncs_decode_8bit(&data);

	ncs_dec_skip_space(uba, MAB_MSG_HDR_SIZE);

	switch (mm->op) {
	case MAB_MAS_REQ_HDLR:
	case MAB_MAS_RSP_HDLR:
	case MAB_MAC_RSP_HDLR:
	case MAB_OAC_REQ_HDLR:
	case MAB_OAC_RSP_HDLR:
	case MAB_OAC_DEF_REQ_HDLR:
		mm->data.data.snmp = ncsmib_decode(uba, msg_fmt_ver);
		if (mm->data.data.snmp == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		break;

	case MAB_PSS_SET_REQUEST:
		{
			NCSMIB_ARG *lcl_dec = NULL;

			data = ncs_dec_flatten_space(uba, (uns8 *)&mm->data.seq_num, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			mm->data.seq_num = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, sizeof(uns32));

			lcl_dec = ncsmib_decode(uba, msg_fmt_ver);
			if (lcl_dec == NULL) {
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}

			mm->data.data.snmp = lcl_dec;
		}
		break;

	case MAB_MAS_REG_HDLR:
		if (mas_reg_msg_decode(&(mm->data.data.reg), uba) != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		break;

	case MAB_MAS_UNREG_HDLR:
		if (mas_unreg_msg_decode(&(mm->data.data.unreg), uba) != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		break;

	case MAB_OAC_FIR_HDLR:
		if (oac_fir_msg_decode(&(mm->data.data.idx_free), uba) != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		break;

		/* In all the following cases, there is no data to be decoded */
	case MAB_MAC_PLAYBACK_START:
		{
			uns16 cnt = 0, k = 0, *tbl_list = NULL;
			data = ncs_dec_flatten_space(uba, (uns8 *)&cnt, sizeof(uns16));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			cnt = ncs_decode_16bit(&data);
			ncs_dec_skip_space(uba, sizeof(uns16));

			if (cnt != 0) {
				if ((tbl_list = m_MMGR_ALLOC_MAC_TBL_BUFR(cnt)) == NULL) {
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}

				memset((uns8 *)tbl_list, '\0', (cnt + 1) * sizeof(uns16));

				/* First tuple contains the number of table-ids received from PSSv. */
				tbl_list[0] = cnt;

				mm->data.data.plbck_start.o_tbl_list = tbl_list;
				for (k = 0; k < (cnt / 2); k++) {
					data = ncs_dec_flatten_space(uba, (uns8 *)&tbl_list[k + 1], sizeof(uns16));
					if (data == NULL) {
						m_MMGR_FREE_MAC_TBL_BUFR(tbl_list);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					tbl_list[k + 1] = ncs_decode_16bit(&data);
					ncs_dec_skip_space(uba, sizeof(uns16));
				}
			}
			break;
		}

	case MAB_MAC_PLAYBACK_END:
	case MAB_OAC_PLAYBACK_START:
	case MAB_OAC_PLAYBACK_END:
	case MAB_OAA_DOWN_EVT:
	case MAB_MAS_ASYNC_DONE:
	case MAB_SVC_VDEST_ROLE_QUIESCED:
		break;

	case MAB_OAC_PSS_ACK:
		data = ncs_dec_flatten_space(uba, (uns8 *)&mm->data.seq_num, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		mm->data.seq_num = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));
		break;

	case MAB_OAC_PSS_EOP_EVT:
		data = ncs_dec_flatten_space(uba, (uns8 *)&mm->data.data.oac_pss_eop_ind.mib_key, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		mm->data.data.oac_pss_eop_ind.mib_key = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));

		data = ncs_dec_flatten_space(uba, (uns8 *)&mm->data.data.oac_pss_eop_ind.wbreq_hdl, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		mm->data.data.oac_pss_eop_ind.wbreq_hdl = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));

		data = ncs_dec_flatten_space(uba, (uns8 *)&mm->data.data.oac_pss_eop_ind.status, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		mm->data.data.oac_pss_eop_ind.status = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));
		break;

	case MAB_PSS_WARM_BOOT:
		data = ncs_dec_flatten_space(uba, (uns8 *)&mm->data.seq_num, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		mm->data.seq_num = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));

		return oac_pss_decode_warmboot_req(uba, &mm->data.data.oac_pss_warmboot_req);
		break;

	case MAB_PSS_TBL_BIND:
		data = ncs_dec_flatten_space(uba, (uns8 *)&mm->data.seq_num, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		mm->data.seq_num = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));

		return oac_pss_tbl_decode_bind_req(uba, &mm->data.data.oac_pss_tbl_bind);
		break;

	case MAB_PSS_TBL_UNBIND:
		data = ncs_dec_flatten_space(uba, (uns8 *)&mm->data.seq_num, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		mm->data.seq_num = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));

		return oac_pss_tbl_decode_unbind_req(uba, &mm->data.data.oac_pss_tbl_unbind);
		break;

	default:
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mab_mds_cpy
 * Purpose:        copy a MAB message going to somebody in this memory space
 ****************************************************************************/
uns32 mab_mds_cpy(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg, SS_SVC_ID to_svc, NCSCONTEXT *cpy, NCS_BOOL last)
{
	MAB_MSG *mm;

	if (msg == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	mm = m_MMGR_ALLOC_MAB_MSG;

	if (mm == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	*cpy = mm;

	memcpy(mm, msg, sizeof(MAB_MSG));

	switch (mm->op) {
	case MAB_MAS_REQ_HDLR:
	case MAB_MAS_RSP_HDLR:
	case MAB_MAC_RSP_HDLR:
	case MAB_OAC_REQ_HDLR:
	case MAB_OAC_RSP_HDLR:
	case MAB_OAC_DEF_REQ_HDLR:
		if (((MAB_MSG *)msg)->data.data.snmp != NULL) {
			mm->data.data.snmp = ncsmib_memcopy(((MAB_MSG *)msg)->data.data.snmp);
			if (mm->data.data.snmp == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		break;

	case MAB_PSS_SET_REQUEST:
		mm->data.seq_num = ((MAB_MSG *)msg)->data.seq_num;
		if (((MAB_MSG *)msg)->data.data.snmp != NULL) {
			NCSMIB_ARG *lcl_cpy = NULL;

			lcl_cpy = ncsmib_memcopy(((MAB_MSG *)msg)->data.data.snmp);
			if (lcl_cpy != NULL) {
				mm->data.data.snmp = lcl_cpy;
			} else {
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		}
		break;

	case MAB_MAS_REG_HDLR:
		{
			NCSMAB_FLTR *src_mf = &((MAB_MSG *)msg)->data.data.reg.fltr;
			NCSMAB_FLTR *dst_mf = &mm->data.data.reg.fltr;

			if (src_mf->type == NCSMAB_FLTR_RANGE) {
				if (src_mf->fltr.range.i_min_idx_fltr != NULL) {
					dst_mf->fltr.range.i_min_idx_fltr =
					    m_MMGR_ALLOC_MIB_INST_IDS(dst_mf->fltr.range.i_idx_len);
					if (dst_mf->fltr.range.i_min_idx_fltr == NULL)
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}

				if (src_mf->fltr.range.i_max_idx_fltr != NULL) {
					dst_mf->fltr.range.i_max_idx_fltr =
					    m_MMGR_ALLOC_MIB_INST_IDS(dst_mf->fltr.range.i_idx_len);
					if (dst_mf->fltr.range.i_max_idx_fltr == NULL) {
						m_MMGR_FREE_MIB_INST_IDS(dst_mf->fltr.range.i_min_idx_fltr);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
				}

				memcpy((void *)dst_mf->fltr.range.i_min_idx_fltr,
				       (void *)src_mf->fltr.range.i_min_idx_fltr,
				       dst_mf->fltr.range.i_idx_len * sizeof(uns32));

				memcpy((void *)dst_mf->fltr.range.i_max_idx_fltr,
				       (void *)src_mf->fltr.range.i_max_idx_fltr,
				       dst_mf->fltr.range.i_idx_len * sizeof(uns32));
			}
			if (src_mf->type == NCSMAB_FLTR_EXACT) {
				return mas_mab_exact_fltr_clone(&src_mf->fltr.exact, &dst_mf->fltr.exact);
			}
			break;
		}

	case MAB_MAS_UNREG_HDLR:
	case MAB_MAC_PLAYBACK_START:
	case MAB_MAC_PLAYBACK_END:
	case MAB_OAC_PLAYBACK_START:
	case MAB_OAC_PLAYBACK_END:
		/* nothing to do here... */
		break;

	case MAB_OAC_PSS_ACK:
	case MAB_OAC_PSS_EOP_EVT:
	case MAB_PSS_WARM_BOOT:
		/* mm->data.seq_num = ((MAB_MSG*)msg)->data.seq_num; 
		   mm->data.data.oac_pss_warmboot_req = ((MAB_MSG*)msg)->data.data.oac_pss_warmboot_req; */
	case MAB_PSS_TBL_BIND:
		/* mm->data.seq_num = ((MAB_MSG*)msg)->data.seq_num;
		   mm->data.data.oac_pss_tbl_bind = ((MAB_MSG*)msg)->data.data.oac_pss_tbl_bind; */
	case MAB_PSS_TBL_UNBIND:
		/* mm->data.seq_num = ((MAB_MSG*)msg)->data.seq_num;
		   mm->data.data.oac_pss_tbl_unbind = ((MAB_MSG*)msg)->data.data.oac_pss_tbl_unbind; */
		break;

	case MAB_OAC_FIR_HDLR:
		{
			if (((MAB_MSG *)msg)->data.data.idx_free.fltr_type == NCSMAB_FLTR_RANGE) {
				NCSMAB_RANGE *src_mr =
				    &((MAB_MSG *)msg)->data.data.idx_free.idx_free_data.range_idx_free;
				NCSMAB_RANGE *dst_mr = &mm->data.data.idx_free.idx_free_data.range_idx_free;

				if (src_mr->i_min_idx_fltr != NULL) {
					dst_mr->i_min_idx_fltr = m_MMGR_ALLOC_MIB_INST_IDS(dst_mr->i_idx_len);
					if (dst_mr->i_min_idx_fltr == NULL)
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

					memcpy((void *)dst_mr->i_min_idx_fltr,
					       (void *)src_mr->i_min_idx_fltr, dst_mr->i_idx_len * sizeof(uns32));
				}

				if (src_mr->i_max_idx_fltr != NULL) {
					dst_mr->i_max_idx_fltr = m_MMGR_ALLOC_MIB_INST_IDS(dst_mr->i_idx_len);
					if (dst_mr->i_max_idx_fltr == NULL) {
						if (dst_mr->i_min_idx_fltr != NULL)
							m_MMGR_FREE_MIB_INST_IDS(dst_mr->i_min_idx_fltr);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					memcpy((void *)dst_mr->i_max_idx_fltr,
					       (void *)src_mr->i_max_idx_fltr, dst_mr->i_idx_len * sizeof(uns32));
				}
			} else if (((MAB_MSG *)msg)->data.data.idx_free.fltr_type == NCSMAB_FLTR_EXACT) {
				return mas_mab_exact_fltr_clone(&((MAB_MSG *)msg)->data.data.idx_free.idx_free_data.
								exact_idx_free,
								&mm->data.data.idx_free.idx_free_data.exact_idx_free);
			}

			break;
		}

	default:
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mas_reg_msg_encode
 * Purpose:        encodes a MAS_REG into a ubaid
 ****************************************************************************/

#define MAS_REG_CMN_SIZE  (sizeof(uns32) * 2)

uns32 mas_reg_msg_encode(MAS_REG *mr, NCS_UBAID *uba)
{
	uns8 *data;

	if (uba == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (mr == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_enc_reserve_space(uba, MAS_REG_CMN_SIZE);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	ncs_encode_32bit(&data, mr->tbl_id);
	ncs_encode_32bit(&data, mr->fltr_id);

	ncs_enc_claim_space(uba, MAS_REG_CMN_SIZE);

	if (mab_fltr_encode(&(mr->fltr), uba) != NCSCC_RC_SUCCESS)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mas_reg_msg_decode
 * Purpose:        decodes a MAS_REG from a ubaid
 ****************************************************************************/
uns32 mas_reg_msg_decode(MAS_REG *mr, NCS_UBAID *uba)
{
	uns8 *data;
	uns8 data_buff[MAS_REG_CMN_SIZE];

	if (uba == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (mr == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_dec_flatten_space(uba, data_buff, MAS_REG_CMN_SIZE);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	mr->tbl_id = ncs_decode_32bit(&data);
	mr->fltr_id = ncs_decode_32bit(&data);

	ncs_dec_skip_space(uba, MAS_REG_CMN_SIZE);

	if (mab_fltr_decode(&(mr->fltr), uba) != NCSCC_RC_SUCCESS)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mas_unreg_msg_encode
 * Purpose:        encodes a MAS_UNREG into a ubaid
 ****************************************************************************/

#define MAS_UNREG_CMN_SIZE  (sizeof(uns32) * 2)

uns32 mas_unreg_msg_encode(MAS_UNREG *mu, NCS_UBAID *uba)
{
	uns8 *data;

	if (uba == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (mu == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_enc_reserve_space(uba, MAS_UNREG_CMN_SIZE);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	ncs_encode_32bit(&data, mu->tbl_id);
	ncs_encode_32bit(&data, mu->fltr_id);

	ncs_enc_claim_space(uba, MAS_UNREG_CMN_SIZE);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mas_unreg_msg_decode
 * Purpose:        decodes a MAS_UNREG from a ubaid
 ****************************************************************************/
uns32 mas_unreg_msg_decode(MAS_UNREG *mu, NCS_UBAID *uba)
{
	uns8 *data;
	uns8 data_buff[MAS_UNREG_CMN_SIZE];

	if (uba == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (mu == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_dec_flatten_space(uba, data_buff, MAS_UNREG_CMN_SIZE);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	mu->tbl_id = ncs_decode_32bit(&data);
	mu->fltr_id = ncs_decode_32bit(&data);

	ncs_dec_skip_space(uba, MAS_UNREG_CMN_SIZE);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mab_fltr_encode
 * Purpose:        encodes a NCSMAB_FLTR into a ubaid
 ****************************************************************************/

#define MAS_RANGE_CMN_SIZE (sizeof(uns32) * 3)
#define MAS_EXACT_CMN_SIZE (sizeof(uns32) * 3)	/* Should it not be 2?  */

uns32 mab_fltr_encode(NCSMAB_FLTR *mf, NCS_UBAID *uba)
{
	uns8 *data;
	uns32 i;
	uns32 max;
	uns32 status;

	if (uba == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (mf == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_enc_reserve_space(uba, sizeof(uns8));
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_encode_8bit(&data, mf->type);
	ncs_enc_claim_space(uba, sizeof(uns8));

	switch (mf->type) {
	case NCSMAB_FLTR_RANGE:
		data = ncs_enc_reserve_space(uba, MAS_RANGE_CMN_SIZE);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		ncs_encode_32bit(&data, mf->fltr.range.i_bgn_idx);
		ncs_encode_32bit(&data, mf->fltr.range.i_idx_len);

		ncs_enc_claim_space(uba, MAS_RANGE_CMN_SIZE);

		for (i = 0, max = mf->fltr.range.i_idx_len; i < max; i++) {
			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, *(mf->fltr.range.i_min_idx_fltr + i));
			ncs_enc_claim_space(uba, sizeof(uns32));
		}

		for (i = 0, max = mf->fltr.range.i_idx_len; i < max; i++) {
			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, *(mf->fltr.range.i_max_idx_fltr + i));
			ncs_enc_claim_space(uba, sizeof(uns32));
		}

		break;

	case NCSMAB_FLTR_ANY:
	case NCSMAB_FLTR_DEFAULT:
		/* there's nothing good to encode here ;-] ... */
		break;

	case NCSMAB_FLTR_SAME_AS:
		data = ncs_enc_reserve_space(uba, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		ncs_encode_32bit(&data, mf->fltr.same_as.i_table_id);

		ncs_enc_claim_space(uba, sizeof(uns32));

		break;

	case NCSMAB_FLTR_EXACT:
		status = ncs_mab_exact_fltr_encode(uba, &mf->fltr.exact);
		if (status != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(status);
		break;

	default:
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	data = ncs_enc_reserve_space(uba, sizeof(uns32));
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_encode_32bit(&data, mf->is_move_row_fltr);
	ncs_enc_claim_space(uba, sizeof(uns32));

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mab_fltr_decode
 * Purpose:       decodes a NCSMAB_FLTR from a ubaid
 ****************************************************************************/

uns32 mab_fltr_decode(NCSMAB_FLTR *mf, NCS_UBAID *uba)
{
	uns8 *data;
	uns8 data_buff[MAS_RANGE_CMN_SIZE];
	uns32 i;
	uns32 max;
	uns32 status;

	if (uba == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (mf == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns8));
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	mf->type = ncs_decode_8bit(&data);
	ncs_dec_skip_space(uba, sizeof(uns8));

	switch (mf->type) {
	case NCSMAB_FLTR_RANGE:
		data = ncs_dec_flatten_space(uba, data_buff, MAS_RANGE_CMN_SIZE);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		mf->fltr.range.i_bgn_idx = ncs_decode_32bit(&data);
		mf->fltr.range.i_idx_len = ncs_decode_32bit(&data);

		ncs_dec_skip_space(uba, MAS_RANGE_CMN_SIZE);

		mf->fltr.range.i_min_idx_fltr = m_MMGR_ALLOC_MIB_INST_IDS(mf->fltr.range.i_idx_len);
		if (mf->fltr.range.i_min_idx_fltr == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		mf->fltr.range.i_max_idx_fltr = m_MMGR_ALLOC_MIB_INST_IDS(mf->fltr.range.i_idx_len);
		if (mf->fltr.range.i_max_idx_fltr == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		for (i = 0, max = mf->fltr.range.i_idx_len; i < max; i++) {
			data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			*((uns32 *)mf->fltr.range.i_min_idx_fltr + i) = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, sizeof(uns32));
		}

		for (i = 0, max = mf->fltr.range.i_idx_len; i < max; i++) {
			data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			*((uns32 *)mf->fltr.range.i_max_idx_fltr + i) = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, sizeof(uns32));
		}

		break;

	case NCSMAB_FLTR_ANY:
	case NCSMAB_FLTR_DEFAULT:
		/* there's nothing good to decode here ;-] ... */
		break;

	case NCSMAB_FLTR_SAME_AS:
		data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		mf->fltr.same_as.i_table_id = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));

		break;

	case NCSMAB_FLTR_EXACT:
		status = ncs_mab_exact_fltr_decode(uba, &mf->fltr.exact);
		if (status != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(status);
		break;

	default:
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	mf->is_move_row_fltr = ncs_decode_32bit(&data);
	ncs_dec_skip_space(uba, sizeof(uns32));

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: oac_fir_msg_encode
 * Purpose:        encodes a NCSMAB_IDX_FREE into a ubaid
 ****************************************************************************/

uns32 oac_fir_msg_encode(NCSMAB_IDX_FREE *ifree, NCS_UBAID *uba)
{
	uns8 *data;
	uns32 status;

	/* check whether valid filter type is given or not? */
	if ((ifree->fltr_type != NCSMAB_FLTR_RANGE) || (ifree->fltr_type != NCSMAB_FLTR_EXACT))
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	/* encode the filter type */
	data = ncs_enc_reserve_space(uba, sizeof(uns32));
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_encode_32bit(&data, ifree->fltr_type);
	ncs_enc_claim_space(uba, sizeof(uns32));

	/* encode the table-id */
	data = ncs_enc_reserve_space(uba, sizeof(uns32));
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_encode_32bit(&data, ifree->idx_tbl_id);
	ncs_enc_claim_space(uba, sizeof(uns32));

	/* encode the data(either a Range filter or EXACT filter) */
	switch (ifree->fltr_type) {
	case NCSMAB_FLTR_RANGE:
		status = ncs_mab_range_fltr_encode(uba, &ifree->idx_free_data.range_idx_free);
		if (status != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(status);
		break;

	case NCSMAB_FLTR_EXACT:
		status = ncs_mab_exact_fltr_encode(uba, &ifree->idx_free_data.exact_idx_free);
		if (status != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(status);
		break;

	default:
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	return NCSCC_RC_SUCCESS;
}

/* Encodes the Range (NCSMAB_RANGE) filter */
static uns32 ncs_mab_range_fltr_encode(NCS_UBAID *uba, NCSMAB_RANGE *range_fltr)
{
	uns8 *data;
	uns32 i;
	uns32 max;

	/* encode the bgn index and index length */
	data = ncs_enc_reserve_space(uba, MAS_RANGE_CMN_SIZE);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_encode_32bit(&data, range_fltr->i_bgn_idx);
	ncs_encode_32bit(&data, range_fltr->i_idx_len);
	ncs_enc_claim_space(uba, MAS_RANGE_CMN_SIZE);

	/* encode the minimum range */
	for (i = 0, max = range_fltr->i_idx_len; i < max; i++) {
		data = ncs_enc_reserve_space(uba, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		ncs_encode_32bit(&data, *(range_fltr->i_min_idx_fltr + i));
		ncs_enc_claim_space(uba, sizeof(uns32));
	}

	/* encode the maximum range */
	for (i = 0, max = range_fltr->i_idx_len; i < max; i++) {
		data = ncs_enc_reserve_space(uba, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		ncs_encode_32bit(&data, *(range_fltr->i_max_idx_fltr + i));
		ncs_enc_claim_space(uba, sizeof(uns32));
	}

	return NCSCC_RC_SUCCESS;
}

/* to decode the range filter */
static uns32 ncs_mab_range_fltr_decode(NCS_UBAID *uba, NCSMAB_RANGE *range_fltr)
{
	uns8 *data;
	uns8 data_buff[MAS_RANGE_CMN_SIZE];
	uns32 i;
	uns32 max;

	/* decode the begin index and the index length */
	data = ncs_dec_flatten_space(uba, data_buff, MAS_RANGE_CMN_SIZE);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	range_fltr->i_bgn_idx = ncs_decode_32bit(&data);
	range_fltr->i_idx_len = ncs_decode_32bit(&data);
	ncs_dec_skip_space(uba, MAS_RANGE_CMN_SIZE);

	/* allocate memory to hold the min range of the index */
	range_fltr->i_min_idx_fltr = m_MMGR_ALLOC_MIB_INST_IDS(range_fltr->i_idx_len);
	if (range_fltr->i_min_idx_fltr == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	/* allocate memory to hold the max range */
	range_fltr->i_max_idx_fltr = m_MMGR_ALLOC_MIB_INST_IDS(range_fltr->i_idx_len);
	if (range_fltr->i_max_idx_fltr == NULL) {
		m_MMGR_FREE_MIB_INST_IDS(range_fltr->i_min_idx_fltr);
		range_fltr->i_min_idx_fltr = NULL;
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* decode the min range */
	for (i = 0, max = range_fltr->i_idx_len; i < max; i++) {
		data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
		if (data == NULL) {
			m_MMGR_FREE_MIB_INST_IDS(range_fltr->i_min_idx_fltr);
			range_fltr->i_min_idx_fltr = NULL;
			m_MMGR_FREE_MIB_INST_IDS(range_fltr->i_max_idx_fltr);
			range_fltr->i_max_idx_fltr = NULL;
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		*((uns32 *)range_fltr->i_min_idx_fltr + i) = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));
	}

	/* decode the max range */
	for (i = 0, max = range_fltr->i_idx_len; i < max; i++) {
		data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
		if (data == NULL) {
			m_MMGR_FREE_MIB_INST_IDS(range_fltr->i_min_idx_fltr);
			range_fltr->i_min_idx_fltr = NULL;
			m_MMGR_FREE_MIB_INST_IDS(range_fltr->i_max_idx_fltr);
			range_fltr->i_max_idx_fltr = NULL;
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		*((uns32 *)range_fltr->i_max_idx_fltr + i) = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));
	}
	return NCSCC_RC_SUCCESS;
}

/* Encodes the Exact (NCSMAB_EXACT) filter */
static uns32 ncs_mab_exact_fltr_encode(NCS_UBAID *uba, NCSMAB_EXACT *exact_fltr)
{
	uns8 *data;
	uns32 i;
	uns32 max;

	/* encode the begin index and index length */
	data = ncs_enc_reserve_space(uba, MAS_EXACT_CMN_SIZE);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_encode_32bit(&data, exact_fltr->i_bgn_idx);
	ncs_encode_32bit(&data, exact_fltr->i_idx_len);
	ncs_enc_claim_space(uba, MAS_EXACT_CMN_SIZE);

	/* encode the indices */
	for (i = 0, max = exact_fltr->i_idx_len; i < max; i++) {
		data = ncs_enc_reserve_space(uba, sizeof(uns32));
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		ncs_encode_32bit(&data, *(exact_fltr->i_exact_idx + i));
		ncs_enc_claim_space(uba, sizeof(uns32));
	}
	return NCSCC_RC_SUCCESS;
}

/* Decodes NCSMAB_FLTR_EXACT filter */
static uns32 ncs_mab_exact_fltr_decode(NCS_UBAID *uba, NCSMAB_EXACT *exact_fltr)
{
	uns8 *data;
	uns8 data_buff[MAS_EXACT_CMN_SIZE];
	uns32 i;
	uns32 max;

	/* decode the begin index point and index length values */
	data = ncs_dec_flatten_space(uba, data_buff, MAS_EXACT_CMN_SIZE);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	exact_fltr->i_bgn_idx = ncs_decode_32bit(&data);
	exact_fltr->i_idx_len = ncs_decode_32bit(&data);
	ncs_dec_skip_space(uba, MAS_EXACT_CMN_SIZE);

	/* allocate the memory to hold the indices */
	exact_fltr->i_exact_idx = m_MMGR_ALLOC_MIB_INST_IDS(exact_fltr->i_idx_len);
	if (exact_fltr->i_exact_idx == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	for (i = 0, max = exact_fltr->i_idx_len; i < max; i++) {
		data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
		if (data == NULL) {
			m_MMGR_FREE_MIB_INST_IDS(exact_fltr->i_exact_idx);
			exact_fltr->i_exact_idx = NULL;
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		/* copy the index value */
		*((uns32 *)exact_fltr->i_exact_idx + i) = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uns32));
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: oac_fir_msg_decode
 * Purpose:        decodes a NCSMAB_IDX_FREE from a ubaid
 ****************************************************************************/
uns32 oac_fir_msg_decode(NCSMAB_IDX_FREE *ifree, NCS_UBAID *uba)
{
	uns8 *data;
	uns32 status;
	uns8 data_buff[MAS_RANGE_CMN_SIZE];

	/* decode the filter type and table id */
	data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ifree->fltr_type = ncs_decode_32bit(&data);
	ncs_dec_skip_space(uba, sizeof(uns32));

	if ((ifree->fltr_type != NCSMAB_FLTR_RANGE) || (ifree->fltr_type != NCSMAB_FLTR_EXACT))
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ifree->idx_tbl_id = ncs_decode_32bit(&data);
	ncs_dec_skip_space(uba, sizeof(uns32));

	/* decode the filter information based on the type */
	switch (ifree->fltr_type) {
	case NCSMAB_FLTR_RANGE:
		status = ncs_mab_range_fltr_decode(uba, &ifree->idx_free_data.range_idx_free);
		if (status != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(status);
		break;

	case NCSMAB_FLTR_EXACT:
		status = ncs_mab_exact_fltr_decode(uba, &ifree->idx_free_data.exact_idx_free);
		if (status != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(status);
		break;

	default:
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mab_encode_pcn
 * Purpose:       Encodes the <PCN> string into the ubaid.
 ****************************************************************************/
uns32 mab_encode_pcn(NCS_UBAID *uba, char *pcn)
{
	uns8 *data;
	uns16 len;

	if (pcn == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	len = strlen(pcn);
	data = ncs_enc_reserve_space(uba, 2);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_encode_16bit(&data, len);
	ncs_enc_claim_space(uba, 2);

	if (len != 0) {
		if (ncs_encode_n_octets_in_uba(uba, pcn, len) != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mab_decode_pcn
 * Purpose:       Decodes the <PCN> string into the ubaid.
 ****************************************************************************/
uns32 mab_decode_pcn(NCS_UBAID *uba, char **pcn)
{
	uns8 *data;
	uns16 len = 0;

	if (pcn == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_dec_flatten_space(uba, (uns8 *)&len, 2);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	len = ncs_decode_16bit(&data);
	ncs_dec_skip_space(uba, 2);

	if (len != 0) {
		if (*pcn == NULL) {
			if ((*pcn = m_MMGR_ALLOC_MAB_PCN_STRING(len + 1)) == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		memset(*pcn, '\0', len + 1);
		if (ncs_decode_n_octets_from_uba(uba, *pcn, len) != NCSCC_RC_SUCCESS) {
			m_MMGR_FREE_MAB_PCN_STRING(*pcn);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	} else {
		if (*pcn != NULL) {
			*pcn[0] = '\0';	/* Nullify the string */
		}
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: oac_pss_encode_warmboot_req
 * Purpose:       Encodes the Warmboot playback request structure from
 *                OAC-SU(to PSSv) into the NCS_UBAID.
 ****************************************************************************/
uns32 oac_pss_encode_warmboot_req(NCS_UBAID *uba, MAB_PSS_WARMBOOT_REQ *warmboot_req)
{
	MAB_PSS_WARMBOOT_REQ *levt = NULL;
	MAB_PSS_CLIENT_LIST *p_pcn = NULL;
	MAB_PSS_TBL_LIST *p_tbl = NULL;
	uns8 *data, *p_pcn_cnt, *p_tbl_cnt;
	uns16 pcn_cnt = 0, tbl_cnt = 0;

	p_pcn_cnt = ncs_enc_reserve_space(uba, 2);
	if (p_pcn_cnt == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_enc_claim_space(uba, 2);

	levt = warmboot_req;
	do {
		p_pcn = &levt->pcn_list;
		if (p_pcn->pcn == NULL) {
			levt = levt->next;
			continue;
		}

		if (mab_encode_pcn(uba, p_pcn->pcn) != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		data = ncs_enc_reserve_space(uba, 4);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		ncs_encode_32bit(&data, levt->is_system_client);
		ncs_enc_claim_space(uba, 4);

		data = ncs_enc_reserve_space(uba, 4);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		ncs_encode_32bit(&data, levt->wbreq_hdl);
		ncs_enc_claim_space(uba, 4);

		data = ncs_enc_reserve_space(uba, 4);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		ncs_encode_32bit(&data, (uns32)(levt->mib_key));
		ncs_enc_claim_space(uba, 4);

		tbl_cnt = 0;
		p_tbl_cnt = ncs_enc_reserve_space(uba, 2);
		if (p_tbl_cnt == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		ncs_enc_claim_space(uba, 2);

		p_tbl = p_pcn->tbl_list;
		while (p_tbl != NULL) {
			data = ncs_enc_reserve_space(uba, 4);
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, p_tbl->tbl_id);
			ncs_enc_claim_space(uba, 4);

			p_tbl = p_tbl->next;
			tbl_cnt++;
		};
		ncs_encode_16bit(&p_tbl_cnt, tbl_cnt);

		levt = levt->next;
		pcn_cnt++;
	} while (levt != NULL);

	ncs_encode_16bit(&p_pcn_cnt, pcn_cnt);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: oac_pss_decode_warmboot_req
 * Purpose:       Decodes the Warmboot playback request structure from
 *                OAC-SU(to PSSv) into the NCS_UBAID.
 ****************************************************************************/
uns32 oac_pss_decode_warmboot_req(NCS_UBAID *uba, MAB_PSS_WARMBOOT_REQ *warmboot_req)
{
	uns8 *data;
	uns16 pcn_cnt = 0, tbl_cnt = 0;
	uns32 pcn_loop = 0, tbl_loop = 0;
	MAB_PSS_WARMBOOT_REQ *levt = NULL;
	MAB_PSS_TBL_LIST *p_tbl = NULL;

	if (warmboot_req == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_dec_flatten_space(uba, (uns8 *)&pcn_cnt, 2);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	pcn_cnt = ncs_decode_16bit(&data);
	ncs_dec_skip_space(uba, 2);

	for (pcn_loop = 0; pcn_loop < pcn_cnt; pcn_loop++) {
		if (levt == NULL) {
			levt = warmboot_req;
			memset(levt, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
		} else {
			if ((levt->next = m_MMGR_ALLOC_MAB_PSS_WARMBOOT_REQ) == NULL) {
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			memset(levt->next, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
			levt = levt->next;
		}

		if (mab_decode_pcn(uba, &levt->pcn_list.pcn) != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		data = ncs_dec_flatten_space(uba, (uns8 *)&levt->is_system_client, 4);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		levt->is_system_client = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, 4);

		data = ncs_dec_flatten_space(uba, (uns8 *)(&levt->wbreq_hdl), 4);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		levt->wbreq_hdl = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, 4);

		data = ncs_dec_flatten_space(uba, (uns8 *)(&levt->mib_key), 4);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		levt->mib_key = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, 4);

		data = ncs_dec_flatten_space(uba, (uns8 *)&tbl_cnt, 2);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		tbl_cnt = ncs_decode_16bit(&data);
		ncs_dec_skip_space(uba, 2);

		p_tbl = levt->pcn_list.tbl_list;
		for (tbl_loop = 0; tbl_loop < tbl_cnt; tbl_loop++) {
			if (p_tbl == NULL) {
				levt->pcn_list.tbl_list = p_tbl = m_MMGR_ALLOC_MAB_PSS_TBL_LIST;
				if (p_tbl == NULL)
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			} else {
				p_tbl->next = m_MMGR_ALLOC_MAB_PSS_TBL_LIST;
				if (p_tbl->next == NULL)
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				p_tbl = p_tbl->next;
			}
			memset(p_tbl, '\0', sizeof(MAB_PSS_TBL_LIST));

			data = ncs_dec_flatten_space(uba, (uns8 *)&p_tbl->tbl_id, 2);
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			p_tbl->tbl_id = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, 4);
		}

	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: oac_pss_tbl_bind_req
 * Purpose:       Encodes the OAC Table Bind event from OAC to PSS.
 ****************************************************************************/
uns32 oac_pss_tbl_encode_bind_req(NCS_UBAID *uba, MAB_PSS_TBL_BIND_EVT *bind_req)
{
	MAB_PSS_TBL_BIND_EVT *levt = NULL;
	MAB_PSS_CLIENT_LIST *p_pcn = NULL;
	MAB_PSS_TBL_LIST *p_tbl = NULL;
	uns8 *data, *p_pcn_cnt, *p_tbl_cnt;
	uns16 pcn_cnt = 0, tbl_cnt = 0;

	p_pcn_cnt = ncs_enc_reserve_space(uba, 2);
	if (p_pcn_cnt == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_enc_claim_space(uba, 2);

	levt = bind_req;
	do {
		p_pcn = &levt->pcn_list;
		if (p_pcn->pcn == NULL) {
			levt = levt->next;
			continue;
		}

		if (mab_encode_pcn(uba, p_pcn->pcn) != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		tbl_cnt = 0;
		p_tbl_cnt = ncs_enc_reserve_space(uba, 2);
		if (p_tbl_cnt == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		ncs_enc_claim_space(uba, 2);

		p_tbl = p_pcn->tbl_list;
		while (p_tbl != NULL) {
			data = ncs_enc_reserve_space(uba, 4);
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			ncs_encode_32bit(&data, p_tbl->tbl_id);
			ncs_enc_claim_space(uba, 4);

			p_tbl = p_tbl->next;
			tbl_cnt++;
		};
		ncs_encode_16bit(&p_tbl_cnt, tbl_cnt);

		levt = levt->next;
		pcn_cnt++;
	} while (levt != NULL);

	ncs_encode_16bit(&p_pcn_cnt, pcn_cnt);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: oac_pss_tbl_decode_bind_req
 * Purpose:       Decodes the OAC Table Bind event from OAC to PSS.
 ****************************************************************************/
uns32 oac_pss_tbl_decode_bind_req(NCS_UBAID *uba, MAB_PSS_TBL_BIND_EVT *bind_req)
{
	uns8 *data;
	uns16 pcn_cnt = 0, tbl_cnt = 0;
	uns32 pcn_loop = 0, tbl_loop = 0;
	MAB_PSS_TBL_BIND_EVT *levt = NULL;
	MAB_PSS_TBL_LIST *p_tbl = NULL;

	if (bind_req == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_dec_flatten_space(uba, (uns8 *)&pcn_cnt, 2);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	pcn_cnt = ncs_decode_16bit(&data);
	ncs_dec_skip_space(uba, 2);

	for (pcn_loop = 0; pcn_loop < pcn_cnt; pcn_loop++) {
		if (levt == NULL) {
			levt = bind_req;
			memset(levt, '\0', sizeof(MAB_PSS_TBL_BIND_EVT));
		} else {
			if ((levt->next = m_MMGR_ALLOC_MAB_PSS_TBL_BIND_EVT) == NULL) {
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			memset(levt->next, '\0', sizeof(MAB_PSS_TBL_BIND_EVT));
			levt = levt->next;
		}

		if (mab_decode_pcn(uba, &levt->pcn_list.pcn) != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		data = ncs_dec_flatten_space(uba, (uns8 *)&tbl_cnt, 2);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		tbl_cnt = ncs_decode_16bit(&data);
		ncs_dec_skip_space(uba, 2);

		p_tbl = levt->pcn_list.tbl_list;
		for (tbl_loop = 0; tbl_loop < tbl_cnt; tbl_loop++) {
			if (p_tbl == NULL) {
				levt->pcn_list.tbl_list = p_tbl = m_MMGR_ALLOC_MAB_PSS_TBL_LIST;
				if (p_tbl == NULL)
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			} else {
				p_tbl->next = m_MMGR_ALLOC_MAB_PSS_TBL_LIST;
				if (p_tbl->next == NULL)
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				p_tbl = p_tbl->next;
			}
			memset(p_tbl, '\0', sizeof(MAB_PSS_TBL_LIST));

			data = ncs_dec_flatten_space(uba, (uns8 *)&p_tbl->tbl_id, 4);
			if (data == NULL)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			p_tbl->tbl_id = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, 4);
		}

	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: oac_pss_tbl_encode_unbind_req
 * Purpose:       Encodes the OAC Table Unbind event from OAC to PSS.
 ****************************************************************************/
uns32 oac_pss_tbl_encode_unbind_req(NCS_UBAID *uba, MAB_PSS_TBL_UNBIND_EVT *unbind_req)
{
	uns8 *data;

	data = ncs_enc_reserve_space(uba, 4);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_encode_32bit(&data, unbind_req->tbl_id);
	ncs_enc_claim_space(uba, 4);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: oac_pss_tbl_decode_unbind_req
 * Purpose:       Decodes the OAC Table Unbind event from OAC to PSS.
 ****************************************************************************/
uns32 oac_pss_tbl_decode_unbind_req(NCS_UBAID *uba, MAB_PSS_TBL_UNBIND_EVT *unbind_req)
{
	uns8 *data;

	if (unbind_req == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_dec_flatten_space(uba, (uns8 *)&unbind_req->tbl_id, 4);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	unbind_req->tbl_id = ncs_decode_32bit(&data);
	ncs_dec_skip_space(uba, 4);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_bam_decode_conf_done
 * Purpose:       Decodes the conf-done event from BAM
 ****************************************************************************/
uns32 pss_bam_decode_conf_done(NCS_UBAID *uba, MAB_PSS_BAM_CONF_DONE_EVT *conf_done)
{
	uns8 *data;
	MAB_PSS_TBL_LIST *list = NULL, *list_head = NULL, *tmp = NULL;
	uns32 tbl_cnt = 0, i = 0, tbl_id = 0;

	if (conf_done == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (mab_decode_pcn(uba, &conf_done->pcn_list.pcn) != NCSCC_RC_SUCCESS)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	data = ncs_dec_flatten_space(uba, (uns8 *)&tbl_cnt, 2);
	if (data == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	tbl_cnt = ncs_decode_16bit(&data);
	ncs_dec_skip_space(uba, 2);

	for (i = 0; i < tbl_cnt; i++) {
		data = ncs_dec_flatten_space(uba, (uns8 *)&tbl_id, 4);
		if (data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		tbl_id = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, 4);

		tmp = m_MMGR_ALLOC_MAB_PSS_TBL_LIST;
		if (tmp == NULL) {
			mab_pss_free_pss_tbl_list(list_head);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		if (list == NULL)
			list_head = list = tmp;
		else {
			list->next = tmp;
			list = list->next;
		}
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mab_pss_free_pss_tbl_list
 * Purpose:       Frees the MAB_PSS_TBL_LIST linked list.
 ****************************************************************************/
void mab_pss_free_pss_tbl_list(MAB_PSS_TBL_LIST *tbl_list)
{
	MAB_PSS_TBL_LIST *tmp = NULL;
	while (tbl_list != NULL) {
		tmp = tbl_list;
		tbl_list = tbl_list->next;
		m_MMGR_FREE_MAB_PSS_TBL_LIST(tmp);
	}
	return;
}

/****************************************************************************
 * Function Name: pss_bam_encode_warmboot_req
 * Purpose:       Encodes the Warmboot playback request structure from
 *                PSS(to BAM), into the NCS_UBAID.
 ****************************************************************************/
uns32 pss_bam_encode_warmboot_req(NCS_UBAID *uba, PSS_BAM_WARMBOOT_REQ *warmboot_req)
{
	PSS_BAM_WARMBOOT_REQ *levt = NULL;
	uns8 *p_pcn_cnt;
	uns16 pcn_cnt = 0;
	char **p_pcn = NULL;

	p_pcn_cnt = ncs_enc_reserve_space(uba, 2);
	if (p_pcn_cnt == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	ncs_enc_claim_space(uba, 2);

	levt = warmboot_req;
	do {
		p_pcn = &levt->pcn;
		if ((*p_pcn == NULL) || ((*p_pcn)[0] == '\0')) {
			levt = levt->next;
			continue;
		}

		if (mab_encode_pcn(uba, *p_pcn) != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		levt = levt->next;
		pcn_cnt++;
	} while (levt != NULL);

	ncs_encode_16bit(&p_pcn_cnt, pcn_cnt);

	return NCSCC_RC_SUCCESS;
}

/* clones a range filter indices */
uns32 mas_mab_range_fltr_clone(NCSMAB_RANGE *src_range, NCSMAB_RANGE *dst_range)
{
	if (src_range->i_idx_len == 0) {
		dst_range->i_min_idx_fltr = NULL;
		dst_range->i_max_idx_fltr = NULL;
	} else {
		/* copy the min index range */
		dst_range->i_min_idx_fltr = m_MMGR_ALLOC_MIB_INST_IDS(src_range->i_idx_len);
		if (dst_range->i_min_idx_fltr == NULL) {
			m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL,
					  MAB_MF_MAS_FLTR_MIN_IDX_CREATE_FAILED, "mas_mab_range_fltr_clone()");
			return NCSCC_RC_OUT_OF_MEM;
		}
		memcpy((void *)dst_range->i_min_idx_fltr,
		       (void *)src_range->i_min_idx_fltr, (src_range->i_idx_len * sizeof(uns32)));

		/* copy the max index range */
		dst_range->i_max_idx_fltr = m_MMGR_ALLOC_MIB_INST_IDS(src_range->i_idx_len);
		if (dst_range->i_max_idx_fltr == NULL) {
			m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL,
					  MAB_MF_MAS_FLTR_MAX_IDX_CREATE_FAILED, "mas_mab_range_fltr_clone()");
			m_MMGR_FREE_MIB_INST_IDS(dst_range->i_min_idx_fltr);
			return NCSCC_RC_OUT_OF_MEM;
		}
		memcpy((void *)dst_range->i_max_idx_fltr,
		       (void *)src_range->i_max_idx_fltr, (src_range->i_idx_len * sizeof(uns32)));

		/* copy the index length */
		dst_range->i_idx_len = src_range->i_idx_len;
	}
	return NCSCC_RC_SUCCESS;
}

/* clones a exact filter indices */
uns32 mas_mab_exact_fltr_clone(NCSMAB_EXACT *src_exact, NCSMAB_EXACT *dst_exact)
{
	/* copy the exact index */
	dst_exact->i_exact_idx = m_MMGR_ALLOC_MIB_INST_IDS(src_exact->i_idx_len);
	if (dst_exact->i_exact_idx == NULL) {
		m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL,
				  MAB_MF_MAS_FLTR_MAX_IDX_CREATE_FAILED, "mas_mab_exact_fltr_clone()");
		return NCSCC_RC_OUT_OF_MEM;
	}
	memcpy((void *)dst_exact->i_exact_idx, (void *)src_exact->i_exact_idx, (src_exact->i_idx_len * sizeof(uns32)));

	/* copy the index length */
	dst_exact->i_idx_len = src_exact->i_idx_len;
	return NCSCC_RC_SUCCESS;
}

void mas_mab_fltr_indices_cleanup(NCSMAB_FLTR *fltr)
{
	if (fltr == NULL)
		return;

	switch (fltr->type) {
	case NCSMAB_FLTR_RANGE:
		if (fltr->fltr.range.i_idx_len > 0) {
			if (fltr->fltr.range.i_min_idx_fltr) {
				m_MMGR_FREE_MIB_INST_IDS(fltr->fltr.range.i_min_idx_fltr);
				fltr->fltr.range.i_min_idx_fltr = NULL;
			}

			if (fltr->fltr.range.i_max_idx_fltr) {
				m_MMGR_FREE_MIB_INST_IDS(fltr->fltr.range.i_max_idx_fltr);
				fltr->fltr.range.i_max_idx_fltr = NULL;
			}
		}
		break;
	case NCSMAB_FLTR_EXACT:
		if (fltr->fltr.exact.i_idx_len > 0) {
			if (fltr->fltr.exact.i_exact_idx) {
				m_MMGR_FREE_MIB_INST_IDS(fltr->fltr.exact.i_exact_idx);
				fltr->fltr.exact.i_exact_idx = NULL;
			}
		}
		break;
	default:
		break;
	}
	return;
}

/****************************************************************************\
*  Name:          mab_leave_on_queue_cb                                      * 
*                                                                            *
*  Description:   This is callback function to free the pending message      *
*                 in the mailbox.                                            *
*                                                                            *
*  Arguments:     arg2 - Pointer to the message to be freed.                 * 
*                                                                            * 
*  Returns:       TRUE  - Everything went well                               *
*  NOTE:  There is no other common file to place this function               *  
\****************************************************************************/
NCS_BOOL mab_leave_on_queue_cb(void *arg1, void *arg2)
{
	MAB_MSG *msg;

	if ((msg = (MAB_MSG *)arg2) != NULL) {
		m_MMGR_FREE_MAB_MSG(msg);
	}
	return TRUE;
}

#endif   /* (NCS_MAB == 1) */
