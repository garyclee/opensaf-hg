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

#include "mqa.h"
#include "mqa_log.h"

/*****************************************************************************
  FILE NAME: MQA_LOG.C

  DESCRIPTION: MQA Logging utilities

  FUNCTIONS INCLUDED in this module:
      mqa_flx_log_reg
      mqa_flx_log_unreg
******************************************************************************/
/*****************************************************************************

  PROCEDURE NAME:    mqa_log

  DESCRIPTION:       MQA Logging info

*****************************************************************************/
#if((NCS_DTA == 1) && (NCS_MQSV_LOG == 1))
void mqa_log(uint8_t id, uns32 category, uint8_t sev, uns32 rc, char *fname, uns32 fno)
{
	ncs_logmsg(NCS_SERVICE_ID_MQA, MQA_LID_HDLN, MQA_FC_HDLN, category, sev, NCSFL_TYPE_TCLIL, fname, fno, id, rc);
}

/****************************************************************************
 * Name          : mqa_flx_log_reg
 *
 * Description   : This is the function which registers the MQA logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void mqa_flx_log_reg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_MQA;
	/* fill version no. */
	reg.info.bind_svc.version = MQSV_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "MQSv");

	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
 * Name          : mqa_flx_log_dereg
 *
 * Description   : This is the function which deregisters the MQA logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void mqa_flx_log_dereg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_MQA;
	ncs_dtsv_su_req(&reg);
	return;
}

#endif   /* #if((NCS_DTA == 1) && (NCS_MQSV_LOG == 1)) */
