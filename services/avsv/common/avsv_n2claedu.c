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

  This file contains utility routines for managing encode/decode operations
  in AvSv components AvND and CLA.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  avsv_edp_nd_cla_msg
  avsv_edp_cla_api_info
  avsv_edp_cla_cbq_info
  avsv_edp_cla_api_resp_info
  avsv_edp_saclmclusternodet
  avsv_edp_saclmclusternotificationt
  avsv_nd_cla_msg_test_type_fnc
  avsv_nd_cla_api_test_type_fnc
  avsv_nd_cla_cbq_test_type_fnc
  avsv_nd_cla_api_resp_test_type_fnc
  


******************************************************************************
*/
#include "avsv.h"
#include "avsv_n2clamem.h"
#include "avsv_clmparam.h"
#include "avsv_n2clamsg.h"
#include "avsv_n2claedu.h"

static uns32 avsv_edp_cla_api_info(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err);

static uns32 avsv_edp_cla_cbq_info(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err);

static uns32 avsv_edp_cla_api_resp_info(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err);

static uns32 avsv_edp_saclmclusternodet(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err);

static uns32 avsv_edp_saclmclusternotificationt(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err);

static int avsv_nd_cla_msg_test_type_fnc(NCSCONTEXT arg);

static int  avsv_nd_cla_api_test_type_fnc(NCSCONTEXT arg);

static int  avsv_nd_cla_cbq_test_type_fnc(NCSCONTEXT arg);

static int  avsv_nd_cla_api_resp_test_type_fnc(NCSCONTEXT arg);

/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_nd_cla_msg

  DESCRIPTION:      EDU program handler for "AVSV_NDA_CLA_MSG" data. This
                    function is invoked by EDU for performing encode/decode
                    operation on "AVSV_NDA_CLA_MSG" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_nd_cla_msg(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVSV_NDA_CLA_MSG    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_edp_nd_cla_msg_rules[ ] = {
        {EDU_START, avsv_edp_nd_cla_msg, 0, 0, 0,
                    sizeof(AVSV_NDA_CLA_MSG), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
                    (long)&((AVSV_NDA_CLA_MSG*)0)->type, 0, NULL},
        {EDU_TEST, ncs_edp_uns32, 0, 0, 0,
                    (long)&((AVSV_NDA_CLA_MSG*)0)->type, 0,
                                avsv_nd_cla_msg_test_type_fnc},

        {EDU_EXEC, avsv_edp_cla_api_info, 0, 0, EDU_EXIT,
            (long)&((AVSV_NDA_CLA_MSG*)0)->info.api_info, 0, NULL},

        {EDU_EXEC, avsv_edp_cla_cbq_info, 0, 0, EDU_EXIT,
            (long)&((AVSV_NDA_CLA_MSG*)0)->info.cbk_info, 0, NULL},

        {EDU_EXEC, avsv_edp_cla_api_resp_info, 0, 0, EDU_EXIT,
            (long)&((AVSV_NDA_CLA_MSG*)0)->info.api_resp_info, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVSV_NDA_CLA_MSG *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVSV_NDA_CLA_MSG **)ptr;
        if(*d_ptr == NULL)
        {
            *d_ptr = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG; 
            if(*d_ptr == NULL)
            {
               *o_err = EDU_ERR_MEM_FAIL;
               return NCSCC_RC_FAILURE;
            }
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVSV_NDA_CLA_MSG));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }
    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_edp_nd_cla_msg_rules, struct_ptr,
            ptr_data_len, buf_env, op, o_err);
    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_cla_api_info 

  DESCRIPTION:      EDU program handler for "AVSV_CLM_API_INFO" data. This
                    function is invoked by EDU for performing encode/decode
                    operation on "AVSV_CLM_API_INFO" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_cla_api_info(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVSV_CLM_API_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_edp_cla_api_info_rules[ ] = {
        {EDU_START, avsv_edp_cla_api_info, 0, 0, 0,
                    sizeof(AVSV_CLM_API_INFO), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
                    (long)&((AVSV_CLM_API_INFO*)0)->prc_id, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0,
                    (long)&((AVSV_CLM_API_INFO*)0)->dest, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
                    (long)&((AVSV_CLM_API_INFO*)0)->type, 0, NULL},
        {EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
                    (long)&((AVSV_CLM_API_INFO*)0)->is_sync_api, 0, NULL},
        {EDU_TEST, ncs_edp_uns32, 0, 0, 0,
                    (long)&((AVSV_CLM_API_INFO*)0)->type, 0,
                                avsv_nd_cla_api_test_type_fnc},

        {EDU_EXEC, m_NCS_EDP_SACLMHANDLET, 0, 0, EDU_EXIT,
            (long)&((AVSV_CLM_API_INFO*)0)->param.finalize.hdl, 0, NULL},

        {EDU_EXEC, m_NCS_EDP_SACLMHANDLET, 0, 0, 0,
            (long)&((AVSV_CLM_API_INFO*)0)->param.track_start.hdl, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAUINT8T, 0, 0, 0,
            (long)&((AVSV_CLM_API_INFO*)0)->param.track_start.flags, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAUINT64T, 0, 0, EDU_EXIT,
            (long)&((AVSV_CLM_API_INFO*)0)->param.track_start.viewNumber, 0, NULL},

        {EDU_EXEC, m_NCS_EDP_SACLMHANDLET, 0, 0, EDU_EXIT,
            (long)&((AVSV_CLM_API_INFO*)0)->param.track_stop.hdl, 0, NULL},

        {EDU_EXEC, m_NCS_EDP_SACLMHANDLET, 0, 0, 0,
            (long)&((AVSV_CLM_API_INFO*)0)->param.node_get.hdl, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SACLMNODEIDT, 0, 0, EDU_EXIT,
            (long)&((AVSV_CLM_API_INFO*)0)->param.node_get.node_id, 0, NULL},

        {EDU_EXEC, m_NCS_EDP_SACLMHANDLET, 0, 0, 0,
            (long)&((AVSV_CLM_API_INFO*)0)->param.node_async_get.hdl, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAINVOCATIONT, 0, 0, 0,
            (long)&((AVSV_CLM_API_INFO*)0)->param.node_async_get.inv, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SACLMNODEIDT, 0, 0, EDU_EXIT,
            (long)&((AVSV_CLM_API_INFO*)0)->param.node_async_get.node_id, 0, NULL},


        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVSV_CLM_API_INFO*)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVSV_CLM_API_INFO**)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVSV_CLM_API_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }
    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_edp_cla_api_info_rules, struct_ptr,
            ptr_data_len, buf_env, op, o_err);
    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_cla_cbq_info 

  DESCRIPTION:      EDU program handler for "AVSV_CLM_CBK_INFO" data. This
                    function is invoked by EDU for performing encode/decode
                    operation on "AVSV_CLM_CBK_INFO" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_cla_cbq_info(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVSV_CLM_CBK_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_edp_cla_cbq_info_rules[ ] = {
        {EDU_START, avsv_edp_cla_cbq_info, 0, 0, 0,
                    sizeof(AVSV_CLM_CBK_INFO), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
                    (long)&((AVSV_CLM_CBK_INFO*)0)->hdl, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
                    (long)&((AVSV_CLM_CBK_INFO*)0)->type, 0, NULL},
        {EDU_TEST, ncs_edp_uns32, 0, 0, 0,
                    (long)&((AVSV_CLM_CBK_INFO*)0)->type, 0,
                                avsv_nd_cla_cbq_test_type_fnc},

        {EDU_EXEC, m_NCS_EDP_SAUINT64T, 0, 0, 0,
            (long)&((AVSV_CLM_CBK_INFO*)0)->param.track.notify.viewNumber, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAUINT32T, 0, 0, 0,
            (long)&((AVSV_CLM_CBK_INFO*)0)->param.track.notify.numberOfItems, 0, NULL},
        {EDU_EXEC, avsv_edp_saclmclusternotificationt, EDQ_VAR_LEN_DATA, m_NCS_EDP_SAUINT32T, 0,
            (long)&((AVSV_CLM_CBK_INFO*)0)->param.track.notify.notification, 
            (long)&((AVSV_CLM_CBK_INFO*)0)->param.track.notify.numberOfItems, NULL},
        {EDU_EXEC_EXT, NULL, NCS_SERVICE_ID_AVSV, NULL, 0, NCS_SERVICE_AVSV_N2CLA_SUB_DEFAULT_VAL, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAUINT32T, 0, 0, 0,
            (long)&((AVSV_CLM_CBK_INFO*)0)->param.track.mem_num, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAERRORT, 0, 0, EDU_EXIT,
            (long)&((AVSV_CLM_CBK_INFO*)0)->param.track.err, 0, NULL},

        {EDU_EXEC, m_NCS_EDP_SAINVOCATIONT, 0, 0, 0,
            (long)&((AVSV_CLM_CBK_INFO*)0)->param.node_get.inv, 0, NULL},
        {EDU_EXEC, avsv_edp_saclmclusternodet, 0, 0, 0,
            (long)&((AVSV_CLM_CBK_INFO*)0)->param.node_get.node, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAERRORT, 0, 0, EDU_EXIT,
            (long)&((AVSV_CLM_CBK_INFO*)0)->param.node_get.err, 0, NULL},



        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVSV_CLM_CBK_INFO*)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVSV_CLM_CBK_INFO**)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVSV_CLM_CBK_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }
    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_edp_cla_cbq_info_rules, struct_ptr,
            ptr_data_len, buf_env, op, o_err);
    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_cla_api_resp_info 

  DESCRIPTION:      EDU program handler for "AVSV_CLM_API_RESP_INFO" data. This
                    function is invoked by EDU for performing encode/decode
                    operation on "AVSV_CLM_API_RESP_INFO" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_cla_api_resp_info(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVSV_CLM_API_RESP_INFO *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_edp_cla_api_resp_info_rules[ ] = {
        {EDU_START, avsv_edp_cla_api_resp_info, 0, 0, 0,
                    sizeof(AVSV_CLM_API_RESP_INFO), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
                    (long)&((AVSV_CLM_API_RESP_INFO*)0)->type, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAERRORT, 0, 0, 0,
                    (long)&((AVSV_CLM_API_RESP_INFO*)0)->rc, 0, NULL},
        {EDU_TEST, ncs_edp_uns32, 0, 0, 0,
                    (long)&((AVSV_CLM_API_RESP_INFO*)0)->type, 0,
                                avsv_nd_cla_api_resp_test_type_fnc},

        {EDU_EXEC, avsv_edp_saclmclusternodet, 0, 0, EDU_EXIT,
            (long)&((AVSV_CLM_API_RESP_INFO*)0)->param.node_get, 0, NULL},

        {EDU_EXEC, m_NCS_EDP_SAINVOCATIONT, 0, 0, EDU_EXIT,
            (long)&((AVSV_CLM_API_RESP_INFO*)0)->param.inv, 0, NULL},

        {EDU_EXEC, ncs_edp_uns16, 0, 0, 0,
            (long)&((AVSV_CLM_API_RESP_INFO*)0)->param.track.num, 0, NULL},

        {EDU_EXEC, avsv_edp_saclmclusternotificationt, EDQ_VAR_LEN_DATA, ncs_edp_uns16, 0,
            (long)&((AVSV_CLM_API_RESP_INFO*)0)->param.track.notify, 
            (long)&((AVSV_CLM_API_RESP_INFO*)0)->param.track.num, NULL},
        {EDU_EXEC_EXT, NULL, NCS_SERVICE_ID_AVSV, NULL, 0, NCS_SERVICE_AVSV_N2CLA_SUB_DEFAULT_VAL, 0, NULL},


        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVSV_CLM_API_RESP_INFO*)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVSV_CLM_API_RESP_INFO**)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVSV_CLM_API_RESP_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }
    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_edp_cla_api_resp_info_rules, struct_ptr,
            ptr_data_len, buf_env, op, o_err);
    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_saclmclusternodet 

  DESCRIPTION:      EDU program handler for "SaClmClusterNodeT" data. This
                    function is invoked by EDU for performing encode/decode
                    operation on "SaClmClusterNodeT" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_saclmclusternodet(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    SaClmClusterNodeT   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_edp_saclmclusternodet_rules[ ] = {
        {EDU_START, avsv_edp_saclmclusternodet, 0, 0, 0,
                    sizeof(SaClmClusterNodeT), 0, NULL},

        {EDU_EXEC, m_NCS_EDP_SACLMNODEIDT, 0, 0, 0,
            (long)&((SaClmClusterNodeT*)0)->nodeId, 0, NULL},
        {EDU_EXEC, ncs_edp_saclmnodeaddresst, 0, 0, 0,
            (long)&((SaClmClusterNodeT*)0)->nodeAddress, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
            (long)&((SaClmClusterNodeT*)0)->nodeName, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SABOOLT, 0, 0, 0,
            (long)&((SaClmClusterNodeT*)0)->member, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
            (long)&((SaClmClusterNodeT*)0)->bootTimestamp, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAUINT64T, 0, 0, 0,
            (long)&((SaClmClusterNodeT*)0)->initialViewNumber, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (SaClmClusterNodeT*)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (SaClmClusterNodeT**)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(SaClmClusterNodeT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_edp_saclmclusternodet_rules, struct_ptr,
            ptr_data_len, buf_env, op, o_err);

    return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_saclmclusternotificationt 

  DESCRIPTION:      EDU program handler for "SaClmClusterNotificationT" data. This
                    function is invoked by EDU for performing encode/decode
                    operation on "SaClmClusterNotificationT" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_saclmclusternotificationt(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    SaClmClusterNotificationT  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_edp_saclmclusternotificationt_rules[ ] = {
        {EDU_START, avsv_edp_saclmclusternotificationt, 0, 0, 0,
                    sizeof(SaClmClusterNotificationT), 0, NULL},

        {EDU_EXEC, avsv_edp_saclmclusternodet, 0, 0, 0,
            (long)&((SaClmClusterNotificationT*)0)->clusterNode, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SACLMCLUSTERCHANGEST, 0, 0, 0,
            (long)&((SaClmClusterNotificationT*)0)->clusterChange, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (SaClmClusterNotificationT*)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (SaClmClusterNotificationT**)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(SaClmClusterNotificationT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_edp_saclmclusternotificationt_rules, struct_ptr,
            ptr_data_len, buf_env, op, o_err);
    return rc;
}
        

/*****************************************************************************

  PROCEDURE NAME:   avsv_nd_cla_msg_test_type_fnc

  DESCRIPTION:      Test function, that gets invoked from the EDP
                    function "avsv_edp_nd_cla_msg".

  RETURNS:          uns32, denoting the offset of the next-instruction
                    to be executed(relative to the EDU_TEST instruction
                    defined in the EDP "avsv_edp_nd_cla_msg").

*****************************************************************************/
int avsv_nd_cla_msg_test_type_fnc(NCSCONTEXT arg)
{
    typedef enum {
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_INFO = 1,
        LCL_JMP_OFFSET_AVSV_N2A_CLA_CBQ_INFO = 2,
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_RESP_INFO = 3,

    }LCL_JMP_OFFSET_;

    AVSV_NDA_CLA_MSG_TYPE type;

    if(arg == NULL)
        return EDU_FAIL;

    type = *(AVSV_NDA_CLA_MSG_TYPE*)arg;

    switch(type)
    {
    case AVSV_CLA_API_MSG:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_INFO;
        break;

    case AVSV_AVND_CLM_CBK_MSG:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_CBQ_INFO;
        break;

    case AVSV_AVND_CLM_API_RESP_MSG:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_RESP_INFO;
        break;

    default:
        break;
    }

    return EDU_FAIL;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_nd_cla_api_test_type_fnc 

  DESCRIPTION:      Test function, that gets invoked from the EDP
                    function "avsv_edp_cla_api_info".

  RETURNS:          uns32, denoting the offset of the next-instruction
                    to be executed(relative to the EDU_TEST instruction
                    defined in the EDP "avsv_edp_cla_api_info").

*****************************************************************************/
int  avsv_nd_cla_api_test_type_fnc(NCSCONTEXT arg)
{
    typedef enum {
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_FINALIZE        = 1,
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_TRACK_START     = 2,
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_TRACK_STOP      = 5,
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_NODE_GET        = 6,
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_NODE_ASYNC_GET  = 8,
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_OTHERS          = 11,

    }LCL_JMP_OFFSET_;

    AVSV_CLM_API_TYPE type;

    if(arg == NULL)
        return EDU_FAIL;

    type = *(AVSV_CLM_API_TYPE*)arg;

    switch(type)
    {

    case AVSV_CLM_FINALIZE:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_FINALIZE;
        break;

    case AVSV_CLM_TRACK_START:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_TRACK_START;
        break;

    case AVSV_CLM_TRACK_STOP:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_TRACK_STOP;
        break;

    case AVSV_CLM_NODE_GET:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_NODE_GET;
        break;

    case AVSV_CLM_NODE_ASYNC_GET:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_NODE_ASYNC_GET;
        break;

    case AVSV_CLM_INITIALIZE:
    case AVSV_CLM_SEL_OBJ_GET:
    case AVSV_CLM_DISPATCH:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_AVSV_CLM_OTHERS;
        break;

    default:
        break;
    }

    return EDU_FAIL;
}



/*****************************************************************************

  PROCEDURE NAME:   avsv_nd_cla_cbq_test_type_fnc 

  DESCRIPTION:      Test function, that gets invoked from the EDP
                    function "avsv_edp_cla_cbq_info".

  RETURNS:          uns32, denoting the offset of the next-instruction
                    to be executed(relative to the EDU_TEST instruction
                    defined in the EDP "avsv_edp_cla_cbq_info").

*****************************************************************************/
int  avsv_nd_cla_cbq_test_type_fnc(NCSCONTEXT arg)
{
    typedef enum {
        LCL_JMP_OFFSET_AVSV_N2A_CLA_CBQ_AVSV_CLM_CBK_TRACK          = 1,
        LCL_JMP_OFFSET_AVSV_N2A_CLA_CBQ_AVSV_CLM_CBK_NODE_ASYNC_GET = 7,

    }LCL_JMP_OFFSET_;

     AVSV_CLM_CBK_TYPE type;

    if(arg == NULL)
        return EDU_FAIL;

    type = *(AVSV_CLM_CBK_TYPE*)arg;

    switch(type)
    {
    case AVSV_CLM_CBK_TRACK:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_CBQ_AVSV_CLM_CBK_TRACK;
        break;

    case AVSV_CLM_CBK_NODE_ASYNC_GET:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_CBQ_AVSV_CLM_CBK_NODE_ASYNC_GET;
        break;

    default:
        break;
    }

    return EDU_FAIL;
}




/*****************************************************************************

  PROCEDURE NAME:   avsv_nd_cla_api_resp_test_type_fnc 

  DESCRIPTION:      Test function, that gets invoked from the EDP
                    function "avsv_edp_cla_api_resp_info".

  RETURNS:          uns32, denoting the offset of the next-instruction
                    to be executed(relative to the EDU_TEST instruction
                    defined in the EDP "avsv_edp_cla_api_resp_info").

*****************************************************************************/
int  avsv_nd_cla_api_resp_test_type_fnc(NCSCONTEXT arg)
{
    typedef enum {
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_RESP_AVSV_CLM_NODE_GET        = 1,
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_RESP_AVSV_CLM_TRACK_START     = 3,
        LCL_JMP_OFFSET_AVSV_N2A_CLA_API_RESP_OTHERS                   = 6,

    }LCL_JMP_OFFSET_;

     AVSV_CLM_API_TYPE type;

    if(arg == NULL)
        return EDU_FAIL;

    type = *(AVSV_CLM_API_TYPE*)arg;

    switch(type)
    {
    case AVSV_CLM_NODE_GET:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_RESP_AVSV_CLM_NODE_GET;
        break;

    case AVSV_CLM_TRACK_START:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_RESP_AVSV_CLM_TRACK_START;
        break;

    case AVSV_CLM_TRACK_STOP:
    case AVSV_CLM_INITIALIZE:
    case AVSV_CLM_FINALIZE:
    case AVSV_CLM_NODE_ASYNC_GET:
    case AVSV_CLM_SEL_OBJ_GET:
    case AVSV_CLM_DISPATCH:
        return LCL_JMP_OFFSET_AVSV_N2A_CLA_API_RESP_OTHERS;
        break;

    default:
        break;
    }

    return EDU_FAIL;
}


