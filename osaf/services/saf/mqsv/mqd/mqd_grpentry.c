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

#include "mqd.h"

/********************************************************************
 * Name        : samsgqueuegroupentry_set
 *
 * Description : 
 *
 *******************************************************************/

uns32 samsgqueuegroupentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
	return NCSCC_RC_FAILURE;
}

/***********************************************************************
 * Name        : samsgqueuegroupentry_setrow
 *
 * Description :
 *
 **********************************************************************/

uns32 samsgqueuegroupentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				  NCSMIB_SETROW_PARAM_VAL *params,
				  struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag)
{
	return NCSCC_RC_FAILURE;
}

/**************************************************************************
 * Name         :  samsgqueuegroupentry_extract
 *
 * Description  :
 *
 **************************************************************************/

uns32 samsgqueuegroupentry_extract(NCSMIB_PARAM_VAL *param,
				   NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{

	uns32 rc = NCSCC_RC_FAILURE;

	rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
	return rc;
}

/***************************************************************************************
 * Name        :  samsgqueuegroupentry_get
 *
 * Description :  
 *
 **************************************************************************************/

uns32 samsgqueuegroupentry_get(NCSCONTEXT pcb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
	MQD_CB *cb = 0;
	MQD_OBJ_NODE *pNode = 0;
	SaNameT queuename;
	uns32 i;
	cb = (MQD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQD, arg->i_mib_key);

	if (cb == NULL)
		return NCSCC_RC_NO_INSTANCE;

	memset(&queuename, '\0', sizeof(SaNameT));
	if (arg->i_idx.i_inst_len == 0) {
		return NCSCC_RC_FAILURE;
	}
	queuename.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
	for (i = 0; i < queuename.length; i++) {
		queuename.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
	}

	pNode = (MQD_OBJ_NODE *)ncs_patricia_tree_get(&cb->qdb, (uns8 *)&queuename);
	if (pNode == NULL) {
		ncshm_give_hdl(cb->hdl);
		return NCSCC_RC_FAILURE;
	}
	if (pNode->oinfo.type == MQSV_OBJ_QUEUE) {
		ncshm_give_hdl(cb->hdl);
		return NCSCC_RC_NO_INSTANCE;
	}

	*data = (NCSCONTEXT)&pNode->oinfo;
	ncshm_give_hdl(cb->hdl);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************************
 * Name        : samsgqueuegroupentry_next
 *
 * Description :
 *
 ***************************************************************************************/

uns32 samsgqueuegroupentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
				NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len)
{
	MQD_CB *cb = 0;
	MQD_OBJ_NODE *pNode = 0;
	SaNameT queuename;
	uns32 i, counter = 0;
	cb = (MQD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQD, arg->i_mib_key);

	if (cb == NULL)
		return NCSCC_RC_NO_INSTANCE;

	memset(&queuename, '\0', sizeof(SaNameT));
	if (arg->i_idx.i_inst_len != 0) {
		queuename.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
		for (i = 0; i < queuename.length; i++) {
			queuename.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
		}
		pNode = (MQD_OBJ_NODE *)ncs_patricia_tree_getnext(&cb->qdb, (uns8 *)&queuename);
		while (pNode) {
			if (pNode->oinfo.type == MQSV_OBJ_QUEUE) {
				queuename = pNode->oinfo.name;
				pNode = (MQD_OBJ_NODE *)ncs_patricia_tree_getnext(&cb->qdb, (uns8 *)&queuename);
			} else
				break;
		}		/* end of while to find out next group name node */
	} else {
		pNode = (MQD_OBJ_NODE *)ncs_patricia_tree_getnext(&cb->qdb, (uns8 *)NULL);
		while (pNode) {
			if (pNode->oinfo.type == MQSV_OBJ_QUEUE) {
				queuename = pNode->oinfo.name;
				pNode = (MQD_OBJ_NODE *)ncs_patricia_tree_getnext(&cb->qdb, (uns8 *)&queuename);
			} else
				break;
		}
	}
	if (pNode == NULL) {
		ncshm_give_hdl(cb->hdl);
		return NCSCC_RC_NO_INSTANCE;
	}

	*data = (NCSCONTEXT)&pNode->oinfo;
	*next_inst_id_len = pNode->oinfo.name.length;
	next_inst_id[0] = pNode->oinfo.name.length;
	for (counter = 0; counter < pNode->oinfo.name.length; counter++) {
		*(next_inst_id + counter + 1) = (uns32)pNode->oinfo.name.value[counter];
	}
	*next_inst_id_len = pNode->oinfo.name.length + 1;

	ncshm_give_hdl(cb->hdl);
	return NCSCC_RC_SUCCESS;
}
