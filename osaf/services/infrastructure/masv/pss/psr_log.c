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

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging functions of PSSv.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:    log_pss_headline()
                                        log_pss_svc_prvdr()
                                        log_pss_lock()
                                        log_pss_memfail()
                                        log_pss_evt()
                                        log_pss_api()
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#if (NCS_PSR == 1)

#include "psr.h"

typedef struct pss_log_tbl_info {
	uns32 req_type;
	uns32 table;
} PSS_LOG_TBL_INFO;

typedef struct pss_log_param_info {
	uns32 param_id;
	uns32 fmt_id;
	uns32 i_len;
	uns32 status;
} PSS_LOG_PARAM_INFO;

typedef struct pss_log_octet_info {
	uns8 *octet_info;
} PSS_LOG_OCTETS;

typedef struct pss_log_int_info {
	uns32 int_info;
} PSS_LOG_INT_INFO;

static void pss_dump_param_val(NCSMIB_PARAM_VAL *l_param_val);

/*****************************************************************************

  pss_dtsv_bind

  DESCRIPTION: Function is used for binding with DTS.

*****************************************************************************/
uns32 pss_dtsv_bind()
{
	NCS_DTSV_RQ reg;
	memset(&reg, 0, sizeof(NCS_DTSV_RQ));

	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_PSS;
	/* fill version no. */
	reg.info.bind_svc.version = PSSV_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "PSSv");

	if (ncs_dtsv_su_req(&reg) == NCSCC_RC_FAILURE)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  pss_dtsv_unbind

  DESCRIPTION: Function is used for unbinding with DTS.

*****************************************************************************/
uns32 pss_dtsv_unbind()
{
	NCS_DTSV_RQ dereg;

	memset(&dereg, 0, sizeof(NCS_DTSV_RQ));

	dereg.i_op = NCS_DTSV_OP_UNBIND;
	dereg.info.unbind_svc.svc_id = NCS_SERVICE_ID_PSS;
	if (ncs_dtsv_su_req(&dereg) == NCSCC_RC_FAILURE)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	return NCSCC_RC_SUCCESS;
}

/******************************************************************************
 *  Name:          pss_log_ncsmib_arg
 *  Description:   To log the NCSMIB_ARG info
 *
 *  Arguments:     
 *
 *  Returns:     
 ******************************************************************************/
void pss_log_ncsmib_arg(NCSMIB_ARG *arg)
{
#if (PSR_LOG == 1)
	NCSFL_MEM log_mem_dump;
	PSS_LOG_TBL_INFO tbl_info;
	PSS_LOG_PARAM_INFO param_info;
	NCSMIB_PARAM_VAL l_param_val, pv;
	USRBUF *ub = NULL;
	uns32 num_params = 0, i = 0, j = 0, num_rows = 0;
	NCSPARM_AID pa;
	NCSROW_AID ra;
	NCSMIB_IDX idx;
	NCSREMROW_AID rra;

	memset(&tbl_info, 0, sizeof(PSS_LOG_TBL_INFO));
	memset(&param_info, 0, sizeof(PSS_LOG_PARAM_INFO));
	memset(&log_mem_dump, 0, sizeof(NCSFL_MEM));
	memset(&l_param_val, 0, sizeof(NCSMIB_PARAM_VAL));

	/* LOG Table and Object Data */
	tbl_info.req_type = arg->i_op;
	tbl_info.table = arg->i_tbl_id;
	log_mem_dump.len = sizeof(PSS_LOG_TBL_INFO);
	log_mem_dump.dump = log_mem_dump.addr = (char *)&tbl_info;
	m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_TBL_DUMP, log_mem_dump);

	/* LOG Instance Ids */
	if (arg->i_idx.i_inst_len != 0) {
		memset(&log_mem_dump, 0, sizeof(log_mem_dump));
		log_mem_dump.len = (arg->i_idx.i_inst_len) * sizeof(uns32);
		log_mem_dump.dump = log_mem_dump.addr = (char *)arg->i_idx.i_inst_ids;
		m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_INSTID_DUMP, log_mem_dump);
	}

	/* LOG parameter id's */
	memset(&log_mem_dump, 0, sizeof(log_mem_dump));
	switch (arg->i_op) {
	case NCSMIB_OP_REQ_GET:
		param_info.param_id = (uns32)arg->req.info.get_req.i_param_id;

		log_mem_dump.len = sizeof(PSS_LOG_PARAM_INFO);
		log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;
		m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);
		break;

	case NCSMIB_OP_RSP_GET:
		param_info.param_id = (uns32)arg->rsp.info.get_rsp.i_param_val.i_param_id;
		param_info.fmt_id = (uns32)arg->rsp.info.get_rsp.i_param_val.i_fmat_id;
		param_info.i_len = (uns32)arg->rsp.info.get_rsp.i_param_val.i_length;
		memcpy(&l_param_val, &arg->rsp.info.get_rsp.i_param_val, sizeof(NCSMIB_PARAM_VAL));
		param_info.status = arg->rsp.i_status;

		log_mem_dump.len = sizeof(PSS_LOG_PARAM_INFO);
		log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;
		m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);
		break;

	case NCSMIB_OP_REQ_NEXT:
		param_info.param_id = (uns32)arg->req.info.next_req.i_param_id;

		log_mem_dump.len = sizeof(PSS_LOG_PARAM_INFO);
		log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;
		m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);
		break;

	case NCSMIB_OP_RSP_NEXT:
		param_info.param_id = (uns32)arg->rsp.info.next_rsp.i_param_val.i_param_id;
		param_info.fmt_id = (uns32)arg->rsp.info.next_rsp.i_param_val.i_fmat_id;
		param_info.i_len = (uns32)arg->rsp.info.next_rsp.i_param_val.i_length;
		memcpy(&l_param_val, &arg->rsp.info.next_rsp.i_param_val, sizeof(NCSMIB_PARAM_VAL));
		param_info.status = arg->rsp.i_status;

		log_mem_dump.len = sizeof(PSS_LOG_PARAM_INFO);
		log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;
		m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);
		break;

	case NCSMIB_OP_REQ_SET:
	case NCSMIB_OP_REQ_TEST:
		param_info.param_id = (uns32)arg->req.info.set_req.i_param_val.i_param_id;
		param_info.fmt_id = (uns32)arg->req.info.set_req.i_param_val.i_fmat_id;
		param_info.i_len = (uns32)arg->req.info.set_req.i_param_val.i_length;
		memcpy(&l_param_val, &arg->req.info.set_req.i_param_val, sizeof(NCSMIB_PARAM_VAL));

		log_mem_dump.len = sizeof(PSS_LOG_PARAM_INFO);
		log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;
		m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);

		pss_dump_param_val(&l_param_val);
		break;

	case NCSMIB_OP_RSP_SET:
	case NCSMIB_OP_RSP_TEST:
		param_info.param_id = (uns32)arg->rsp.info.set_rsp.i_param_val.i_param_id;
		param_info.fmt_id = (uns32)arg->rsp.info.set_rsp.i_param_val.i_fmat_id;
		param_info.i_len = (uns32)arg->rsp.info.set_rsp.i_param_val.i_length;
		memcpy(&l_param_val, &arg->rsp.info.set_rsp.i_param_val, sizeof(NCSMIB_PARAM_VAL));
		param_info.status = arg->rsp.i_status;

		log_mem_dump.len = sizeof(PSS_LOG_PARAM_INFO);
		log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;
		m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);

		pss_dump_param_val(&l_param_val);
		break;

	case NCSMIB_OP_REQ_SETROW:
		ub = m_MMGR_DITTO_BUFR(arg->req.info.setrow_req.i_usrbuf);

		if (ub == NULL)
			return;

		num_params = ncsparm_dec_init(&pa, ub);
		for (i = 0; i < num_params; i++) {
			memset(&pv, 0, sizeof(PSS_LOG_PARAM_INFO));
			ncsparm_dec_parm(&pa, &pv, NULL);
			param_info.param_id = (uns32)pv.i_param_id;
			param_info.fmt_id = (uns32)pv.i_fmat_id;
			param_info.i_len = (uns32)pv.i_length;
			memcpy(&l_param_val, &pv, sizeof(NCSMIB_PARAM_VAL));
			log_mem_dump.len = sizeof(PSS_LOG_PARAM_INFO);
			log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;
			m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);

			pss_dump_param_val(&l_param_val);

			if (pv.i_fmat_id == NCSMIB_FMAT_OCT) {
				if (pv.info.i_oct != NULL) {
					m_MMGR_FREE_MIB_OCT(pv.info.i_oct);
					pv.info.i_oct = NULL;
				}
			}
		}
		ncsparm_dec_done(&pa);
		break;

	case NCSMIB_OP_RSP_SETROW:
		ub = m_MMGR_DITTO_BUFR(arg->rsp.info.setrow_rsp.i_usrbuf);

		if (ub == NULL)
			return;

		num_params = ncsparm_dec_init(&pa, ub);
		for (i = 0; i < num_params; i++) {
			memset(&pv, 0, sizeof(PSS_LOG_PARAM_INFO));
			ncsparm_dec_parm(&pa, &pv, NULL);
			param_info.param_id = (uns32)pv.i_param_id;
			param_info.fmt_id = (uns32)pv.i_fmat_id;
			param_info.i_len = (uns32)pv.i_length;
			memcpy(&l_param_val, &pv, sizeof(NCSMIB_PARAM_VAL));
			log_mem_dump.len = sizeof(PSS_LOG_PARAM_INFO);
			log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;
			m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);

			pss_dump_param_val(&l_param_val);

			if (pv.i_fmat_id == NCSMIB_FMAT_OCT) {
				if (pv.info.i_oct != NULL) {
					m_MMGR_FREE_MIB_OCT(pv.info.i_oct);
					pv.info.i_oct = NULL;
				}
			}
		}
		ncsparm_dec_done(&pa);
		break;

	case NCSMIB_OP_REQ_SETALLROWS:
		ub = m_MMGR_DITTO_BUFR(arg->req.info.setallrows_req.i_usrbuf);

		if (ub == NULL)
			return;

		num_rows = ncssetallrows_dec_init(&ra, ub);
		if (num_rows == 0)
			return;

		for (i = 0; i < num_rows; i++) {
			num_params = ncsrow_dec_init(&ra);
			if (num_params == 0)
				return;

			idx.i_inst_ids = NULL;
			idx.i_inst_len = 0;

			if (ncsrow_dec_inst_ids(&ra, &idx, NULL) != NCSCC_RC_SUCCESS) {
				/* Log error. TBD */
				return;
			}
			memset(&log_mem_dump, 0, sizeof(log_mem_dump));
			log_mem_dump.len = idx.i_inst_len * sizeof(uns32);
			log_mem_dump.dump = log_mem_dump.addr = (char *)idx.i_inst_ids;
			m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_INSTID_DUMP, log_mem_dump);
			m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
			idx.i_inst_ids = NULL;

			for (j = 0; j < num_params; j++) {
				memset(&pv, 0, sizeof(PSS_LOG_PARAM_INFO));
				ncsrow_dec_param(&ra, &pv, NULL);
				param_info.param_id = (uns32)pv.i_param_id;
				param_info.fmt_id = (uns32)pv.i_fmat_id;
				param_info.i_len = (uns32)pv.i_length;
				memcpy(&l_param_val, &pv, sizeof(NCSMIB_PARAM_VAL));
				log_mem_dump.len = sizeof(PSS_LOG_PARAM_INFO);
				log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;
				m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);

				pss_dump_param_val(&l_param_val);

				if (pv.i_fmat_id == NCSMIB_FMAT_OCT) {
					if (pv.info.i_oct != NULL) {
						m_MMGR_FREE_MIB_OCT(pv.info.i_oct);
						pv.info.i_oct = NULL;
					}
				}
			}
			if (ncsrow_dec_done(&ra) != NCSCC_RC_SUCCESS)
				return;
		}
		if (ncssetallrows_dec_done(&ra) != NCSCC_RC_SUCCESS) {
			return;
		}
		break;

	case NCSMIB_OP_RSP_SETALLROWS:
		ub = m_MMGR_DITTO_BUFR(arg->rsp.info.setallrows_rsp.i_usrbuf);

		if (ub == NULL)
			return;

		num_rows = ncssetallrows_dec_init(&ra, ub);
		if (num_rows == 0)
			return;

		for (i = 0; i < num_rows; i++) {
			num_params = ncsrow_dec_init(&ra);
			if (num_params == 0)
				return;

			idx.i_inst_ids = NULL;
			idx.i_inst_len = 0;

			if (ncsrow_dec_inst_ids(&ra, &idx, NULL) != NCSCC_RC_SUCCESS) {
				/* Log error. TBD */
				return;
			}
			memset(&log_mem_dump, 0, sizeof(log_mem_dump));
			log_mem_dump.len = idx.i_inst_len * sizeof(uns32);
			log_mem_dump.dump = log_mem_dump.addr = (char *)idx.i_inst_ids;
			m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_INSTID_DUMP, log_mem_dump);
			m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
			idx.i_inst_ids = NULL;

			for (j = 0; j < num_params; j++) {
				memset(&pv, 0, sizeof(PSS_LOG_PARAM_INFO));
				ncsrow_dec_param(&ra, &pv, NULL);
				param_info.param_id = (uns32)pv.i_param_id;
				param_info.fmt_id = (uns32)pv.i_fmat_id;
				param_info.i_len = (uns32)pv.i_length;
				memcpy(&l_param_val, &pv, sizeof(NCSMIB_PARAM_VAL));
				log_mem_dump.len = sizeof(PSS_LOG_PARAM_INFO);
				log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;
				m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);

				pss_dump_param_val(&l_param_val);

				if (pv.i_fmat_id == NCSMIB_FMAT_OCT) {
					if (pv.info.i_oct != NULL) {
						m_MMGR_FREE_MIB_OCT(pv.info.i_oct);
						pv.info.i_oct = NULL;
					}
				}
			}
			if (ncsrow_dec_done(&ra) != NCSCC_RC_SUCCESS)
				return;
		}
		if (ncssetallrows_dec_done(&ra) != NCSCC_RC_SUCCESS) {
			return;
		}
		break;

	case NCSMIB_OP_REQ_REMOVEROWS:
		ub = m_MMGR_DITTO_BUFR(arg->req.info.removerows_req.i_usrbuf);

		if (ub == NULL)
			return;

		num_rows = ncsremrow_dec_init(&rra, ub);
		if (num_rows == 0)
			return;

		for (i = 0; i < num_rows; i++) {
			idx.i_inst_ids = NULL;
			idx.i_inst_len = 0;
			if (ncsremrow_dec_inst_ids(&rra, &idx, NULL) != NCSCC_RC_SUCCESS) {
				/* Log error. TBD */
				return;
			}
			memset(&log_mem_dump, 0, sizeof(log_mem_dump));
			log_mem_dump.len = idx.i_inst_len * sizeof(uns32);
			log_mem_dump.dump = log_mem_dump.addr = (char *)idx.i_inst_ids;
			m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_INSTID_DUMP, log_mem_dump);
			m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
			idx.i_inst_ids = NULL;
		}
		if (ncsremrow_dec_done(&rra) != NCSCC_RC_SUCCESS) {
			return;
		}
		break;

	case NCSMIB_OP_RSP_REMOVEROWS:
		ub = m_MMGR_DITTO_BUFR(arg->rsp.info.removerows_rsp.i_usrbuf);

		if (ub == NULL)
			return;

		num_rows = ncsremrow_dec_init(&rra, ub);
		if (num_rows == 0)
			return;

		for (i = 0; i < num_rows; i++) {
			idx.i_inst_ids = NULL;
			idx.i_inst_len = 0;
			if (ncsremrow_dec_inst_ids(&rra, &idx, NULL) != NCSCC_RC_SUCCESS) {
				/* Log error. TBD */
				return;
			}
			memset(&log_mem_dump, 0, sizeof(log_mem_dump));
			log_mem_dump.len = idx.i_inst_len * sizeof(uns32);
			log_mem_dump.dump = log_mem_dump.addr = (char *)idx.i_inst_ids;
			m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_INSTID_DUMP, log_mem_dump);
			m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
			idx.i_inst_ids = NULL;
		}
		if (ncsremrow_dec_done(&rra) != NCSCC_RC_SUCCESS) {
			return;
		}
		break;

	default:
		param_info.param_id = 0xFFFF;	/* strange value indicating error */
		break;
	}
#endif
	return;
}

/******************************************************************************
 *  Name:          pss_dump_param_val
 *  Description:   To log the NCSMIB_PARAM_VAL info
 *
 *  Arguments:     
 *
 *  Returns:     
 ******************************************************************************/
static void pss_dump_param_val(NCSMIB_PARAM_VAL *l_param_val)
{
#if (PSR_LOG == 1)
	NCSFL_MEM log_mem_dump;
	PSS_LOG_INT_INFO int_info;
	PSS_LOG_OCTETS oct_info;

	memset(&log_mem_dump, 0, sizeof(NCSFL_MEM));
	memset(&int_info, 0, sizeof(PSS_LOG_INT_INFO));
	memset(&oct_info, 0, sizeof(PSS_LOG_OCTETS));

	/* Dump INT param value */
	if (l_param_val->i_fmat_id == NCSMIB_FMAT_INT) {
		int_info.int_info = (uns32)l_param_val->info.i_int;
		log_mem_dump.len = sizeof(PSS_LOG_INT_INFO);
		log_mem_dump.dump = log_mem_dump.addr = (char *)&int_info.int_info;
		m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_INT_INFO_DUMP, log_mem_dump);
	}

	if (l_param_val->i_fmat_id == NCSMIB_FMAT_OCT) {
		oct_info.octet_info = (uns8 *)l_param_val->info.i_oct;
		log_mem_dump.len = (uns32)l_param_val->i_length;
		log_mem_dump.dump = log_mem_dump.addr = (char *)oct_info.octet_info;
		m_PSS_MEMDUMP_LOG(PSS_NCSMIB_ARG_OCT_INFO_DUMP, log_mem_dump);
	}
#endif
	return;
}

/******************************************************************************
 *  Name:          pss_log_memdump
 *  Description:   To log the error 
 *
 *  Arguments:     
 *
 *  Returns:     
 ******************************************************************************/
void pss_log_memdump(uns8 index, NCSFL_MEM mem)
{
#if (PSR_LOG == 1)
	NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_PSS;

	ncs_logmsg(svc_id, (uns8)PSS_LID_MEMDUMP, (uns8)PSS_FC_MEMDUMP, NCSFL_LC_DATA, NCSFL_SEV_INFO, NCSFL_TYPE_TID,
		   index, mem);
#endif
	return;
}

#endif
