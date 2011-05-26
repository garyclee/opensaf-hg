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
  FILE NAME: CPA_LOG.C

  DESCRIPTION: CPA Logging utilities

  FUNCTIONS INCLUDED in this module:
      cpa_flx_log_reg
      cpa_flx_log_unreg
      cpa_log_headline
      cpa_log_tmr
      cpa_log_memfail
      cpa_log_api
      cpa_log_lockfail
      cpa_log_evt
      cpa_log_sys_call
      cpa_log_data_send
      
******************************************************************************/
#include "cpa.h"

/****************************************************************************
 * Name          : cpa_flx_log_reg
 *
 * Description   : This is the function which registers the CPA logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void cpa_flx_log_reg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_CPA;
	/* fill version no. */
	reg.info.bind_svc.version = CPSV_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "CPSv");

	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
 * Name          : cpa_flx_log_dereg
 *
 * Description   : This is the function which deregisters the CPA logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void cpa_flx_log_dereg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_CPA;
	ncs_dtsv_su_req(&reg);
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    cpa_log_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void cpa_log_headline(uint8_t hdln_id, uint8_t sev)
{
	ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_HDLN, CPA_FC_HDLN, NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, hdln_id);
}

/*****************************************************************************

  PROCEDURE NAME:    cpa_log_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/
void cpa_log_memfail(uint8_t mf_id)
{
	ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_MEMFAIL, CPA_FC_MEMFAIL,
		   NCSFL_LC_MEMORY, NCSFL_SEV_ERROR, NCSFL_TYPE_TI, mf_id);
}

/*****************************************************************************

  PROCEDURE NAME:    cpa_log_api

  DESCRIPTION:       API logging info

*****************************************************************************/
void cpa_log_api(uint8_t api_id, uint8_t sev)
{
	ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_API, CPA_FC_API, NCSFL_LC_API, sev, NCSFL_TYPE_TI, api_id);
}

/*****************************************************************************

  PROCEDURE NAME:    cpa_log_data_send

  DESCRIPTION:       MDS send logging info

*****************************************************************************/
void cpa_log_data_send(uint8_t id, uint32_t node, uint32_t evt_id)
{
	ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_DATA_SEND, CPA_FC_DATA_SEND,
		   NCSFL_LC_DATA, NCSFL_SEV_ERROR, NCSFL_TYPE_TILL, id, node, evt_id);
}

/*****************************************************************************
                                                                                
  PROCEDURE NAME:    cpa_log_db
                                                                                
  DESCRIPTION:       MDS db operations info
                                                                                
*****************************************************************************/
void cpa_log_db(uint8_t hdln_id, uint8_t sev)
{
	ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_DB, CPA_FC_DB, NCSFL_LC_API, sev, NCSFL_TYPE_TI, hdln_id);
}
