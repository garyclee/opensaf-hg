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
FILE NAME: IFSV_EDU.C
DESCRIPTION: IFSV-EDU Routines

******************************************************************************/
#include "ifsv.h"

static int ipxs_evt_test_type_fnc (NCSCONTEXT arg);

/****************************************************************************
 * Name          : ipxs_evt_test_type_fnc
 *
 * Description   : This is the function offsets for encoding/decoding IPXS_EVT
 *                 Data structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
static int ipxs_evt_test_type_fnc (NCSCONTEXT arg)
{
    IPXS_EVT_TYPE    evt;
    enum
    {
       IPXS_EDU_IPXS_EVT_DTOND_IFIP_INFO = 1,
       IPXS_EDU_IPXS_EVT_ATOND_IFIP_GET,
       IPXS_EDU_IPXS_EVT_ATOND_IFIP_UPD,
       IPXS_EDU_IPXS_EVT_ATOND_IS_LOCAL,
       IPXS_EDU_IPXS_EVT_NDTOD_IFIP_INFO,
       IPXS_EDU_IPXS_EVT_NDTOA_IFIP_RSP,
       IPXS_EDU_IPXS_EVT_NDTOA_IFIP_ADD,
       IPXS_EDU_IPXS_EVT_NDTOA_IFIP_DEL,
       IPXS_EDU_IPXS_EVT_NDTOA_IFIP_UPD,
       IPXS_EDU_IPXS_EVT_NDTOA_NODE_REC,
       IPXS_EDU_IPXS_EVT_NDTOA_ISLOC_RSP
    };

    if(arg == NULL)
        return EDU_FAIL;

    evt = *(IPXS_EVT_TYPE *)arg;
   switch(evt)
   {
   case IPXS_EVT_DTOND_IFIP_INFO:
      return IPXS_EDU_IPXS_EVT_DTOND_IFIP_INFO;
      break;
   case IPXS_EVT_ATOND_IFIP_GET:
      return IPXS_EDU_IPXS_EVT_ATOND_IFIP_GET;
      break;
   case IPXS_EVT_ATOND_IFIP_UPD:
      return IPXS_EDU_IPXS_EVT_ATOND_IFIP_UPD;
      break;
   case IPXS_EVT_NDTOA_IFIP_UPD_RESP:
      return IPXS_EDU_IPXS_EVT_NDTOA_IFIP_UPD;
      break;
   case IPXS_EVT_ATOND_IS_LOCAL:
      return IPXS_EDU_IPXS_EVT_ATOND_IS_LOCAL;
      break;
   case IPXS_EVT_ATOND_NODE_REC_GET:
      return EDU_EXIT;  /* no data to be encoded. So, telling EDU to return */
      break;
   case IPXS_EVT_NDTOD_IFIP_INFO:
      return IPXS_EDU_IPXS_EVT_NDTOD_IFIP_INFO;
      break;
   case IPXS_EVT_NDTOA_IFIP_RSP:
      return IPXS_EDU_IPXS_EVT_NDTOA_IFIP_RSP;
      break;
   case IPXS_EVT_NDTOA_IFIP_ADD:
      return IPXS_EDU_IPXS_EVT_NDTOA_IFIP_ADD;
      break;
   case IPXS_EVT_NDTOA_IFIP_DEL:
      return IPXS_EDU_IPXS_EVT_NDTOA_IFIP_DEL;
      break;
   case IPXS_EVT_NDTOA_IFIP_UPD:
      return IPXS_EDU_IPXS_EVT_NDTOA_IFIP_UPD;
      break;
   case IPXS_EVT_NDTOA_NODE_REC:
      return IPXS_EDU_IPXS_EVT_NDTOA_NODE_REC;
      break;
   case IPXS_EVT_NDTOA_ISLOC_RSP:
      return IPXS_EDU_IPXS_EVT_NDTOA_ISLOC_RSP;
       break;
   default:
        return EDU_FAIL;
   }
}


/****************************************************************************
 * Name          : ncs_edp_ip_addr
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IPPFX structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ncs_edp_ip_addr(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    NCS_IP_ADDR         *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ncs_edp_ip_addr_rules[ ] = {
        {EDU_START, ncs_edp_ip_addr, 0, 0, 0, sizeof(NCS_IP_ADDR), 0, NULL},                        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IP_ADDR*)0)->type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IP_ADDR*)0)->info.v4 ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IP_ADDR *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IP_ADDR **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IP_ADDR));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ncs_edp_ip_addr_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ncs_edp_ippfx
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IPPFX structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ncs_edp_ippfx(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    NCS_IPPFX           *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ncs_edp_ippfx_rules[ ] = {
        {EDU_START, ncs_edp_ippfx, 0, 0, 0, sizeof(NCS_IPPFX), 0, NULL},                        
        {EDU_EXEC, ncs_edp_uns8, 0, 0, 0, (uns32)&((NCS_IPPFX*)0)->mask_len, 0, NULL},
        {EDU_EXEC, ncs_edp_ip_addr, 0, 0, 0, (uns32)&((NCS_IPPFX*)0)->ipaddr ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IPPFX *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IPPFX **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IPPFX));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ncs_edp_ippfx_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          :  ipxs_ifip_ip_info 
 *
 * Description   : This is the function which is used to encode decode 
 *                 IPXS_IFIP_IP_INFO structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ncs_edp_ipxs_ifip_ip_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    IPXS_IFIP_IP_INFO  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ncs_edp_ipxs_ifip_ip_info_rules[ ] = {
        {EDU_START, ncs_edp_ipxs_ifip_ip_info, 0, 0, 0, sizeof(IPXS_IFIP_IP_INFO), 0, NULL},                        
        {EDU_EXEC, ncs_edp_ippfx,0,0,0,(uns32)&((IPXS_IFIP_IP_INFO*)0)->ipaddr,0,NULL},
#if (NCS_VIP == 1)
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_IFIP_IP_INFO*)0)->poolHdl, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_IFIP_IP_INFO*)0)->ipPoolType, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_IFIP_IP_INFO*)0)->refCnt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_IFIP_IP_INFO*)0)->vipIp, 0, NULL},
#endif
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IPXS_IFIP_IP_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IPXS_IFIP_IP_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IPXS_IFIP_IP_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ncs_edp_ipxs_ifip_ip_info_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ipxs_edp_ifip_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IPXS_IPINFO structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ipxs_edp_ifip_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    NCS_IPXS_IPINFO     *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ipxs_edp_atond_ifip_rules[ ] = {
        {EDU_START, ipxs_edp_ifip_info, 0, 0, 0, sizeof(NCS_IPXS_IPINFO), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->ip_attr ,0, NULL},
        {EDU_EXEC, ncs_edp_uns8, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->is_v4_unnmbrd, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->addip_cnt, 0, NULL},
        {EDU_EXEC,  ncs_edp_ipxs_ifip_ip_info , EDQ_VAR_LEN_DATA, ncs_edp_uns8, 0,
                    (uns32)&((NCS_IPXS_IPINFO*)0)->addip_list, 
                    (uns32)&((NCS_IPXS_IPINFO*)0)->addip_cnt, NULL},
        {EDU_EXEC_EXT, NULL, NCS_SERVICE_ID_IFSV /* Svc-ID */, NULL, 0, NCS_IFSV_SVC_SUB_ID_IPXS_DEFAULT /* Sub-ID */, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->delip_cnt, 0, NULL},
        {EDU_EXEC,  ncs_edp_ippfx , EDQ_VAR_LEN_DATA, ncs_edp_uns8, 0,
                    (uns32)&((NCS_IPXS_IPINFO*)0)->delip_list, 
                    (uns32)&((NCS_IPXS_IPINFO*)0)->delip_cnt, NULL},
        {EDU_EXEC_EXT, NULL, NCS_SERVICE_ID_IFSV /* Svc-ID */, NULL, 0, NCS_IFSV_SVC_SUB_ID_IPXS_DEFAULT /* Sub-ID */, 0, NULL},
        {EDU_EXEC, ncs_edp_ip_addr, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->rrtr_ipaddr ,0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->rrtr_rtr_id, 0, NULL},
        {EDU_EXEC, ncs_edp_uns16, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->rrtr_as_id, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->rrtr_if_id, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->ud_1, 0, NULL},
#if (NCS_VIP == 1)
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->intfName, m_NCS_IFSV_VIP_INTF_NAME, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->shelfId, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->slotId, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO*)0)->nodeId, 0, NULL},
#endif
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IPXS_IPINFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IPXS_IPINFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IPXS_IPINFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_atond_ifip_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ipxs_edp_ipxs_intf_rec_info_noptr
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IPXS_INTF_REC structure.
 *****************************************************************************/
uns32 ipxs_edp_ipxs_intf_rec_info_noptr(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    NCS_IPXS_INTF_REC     *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ipxs_edp_ipxs_intf_rec_rules[ ] = {
        {EDU_START, ipxs_edp_ipxs_intf_rec_info_noptr, 0, 0, 0, sizeof(NCS_IPXS_INTF_REC), 0, NULL},
        {EDU_EXEC, ifsv_edp_spt, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->spt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->if_index, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->is_lcl_intf, 0, NULL},
        {EDU_EXEC, ifsv_edp_intf_info, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->if_info, 0, NULL},
        {EDU_EXEC, ipxs_edp_ifip_info, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->ip_info, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IPXS_INTF_REC *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IPXS_INTF_REC **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }

        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IPXS_INTF_REC));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_ipxs_intf_rec_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_ipxs_intf_rec_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IPXS_INTF_REC structure.
 *****************************************************************************/
uns32 ipxs_edp_ipxs_intf_rec_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    NCS_IPXS_INTF_REC     *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ipxs_edp_ipxs_intf_rec_rules[ ] = {
        {EDU_START, ipxs_edp_ipxs_intf_rec_info, EDQ_LNKLIST, 0, 0, sizeof(NCS_IPXS_INTF_REC), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->if_index, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->is_lcl_intf, 0, NULL},
        {EDU_EXEC, ifsv_edp_spt, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->spt, 0, NULL},
        {EDU_EXEC, ifsv_edp_intf_info, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->if_info, 0, NULL},
        {EDU_EXEC, ipxs_edp_ifip_info, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->ip_info, 0, NULL},
        {EDU_TEST_LL_PTR, ipxs_edp_ipxs_intf_rec_info, 0, 0, 0, (uns32)&((NCS_IPXS_INTF_REC*)0)->next, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IPXS_INTF_REC *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IPXS_INTF_REC **)ptr;
        if(*d_ptr == NULL)
        *d_ptr = m_MMGR_ALLOC_NCS_IPXS_INTF_REC;

        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IPXS_INTF_REC));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_ipxs_intf_rec_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_evt_if_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 structure IPXS_EVT_IF_INFO.
 *****************************************************************************/
uns32 ipxs_edp_evt_if_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    IPXS_EVT_IF_INFO    *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ipxs_edp_evt_if_info_rules[ ] = {
        {EDU_START, ipxs_edp_evt_if_info, 0, 0, 0, sizeof(IPXS_EVT_IF_INFO), 0, NULL},                        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_EVT_IF_INFO*)0)->if_index, 0, NULL},
        {EDU_EXEC, ipxs_edp_ifip_info, 0, 0, 0, (uns32)&((IPXS_EVT_IF_INFO*)0)->ip_info, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IPXS_EVT_IF_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IPXS_EVT_IF_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IPXS_EVT_IF_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_evt_if_info_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_ifkey_type_test_fnc
 *
 * Description   : This is the function offsets for encoding/decoding IFSV_EVT
 *                 Data structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
int ipxs_ifkey_type_test_fnc (NCSCONTEXT arg)
{
    enum
    {
       IPXS_EDU_NCS_IPXS_KEY_IFINDEX = 1,
       IPXS_EDU_NCS_IPXS_KEY_SPT,
       IPXS_EDU_NCS_IPXS_KEY_IP_ADDR,
    };
    
    NCS_IFSV_KEY_TYPE    type;

    if(arg == NULL)
        return EDU_FAIL;

    type = *(NCS_IFSV_KEY_TYPE *)arg;
    switch(type)
    {
    case 0:
    case NCS_IPXS_KEY_IFINDEX:
        return IPXS_EDU_NCS_IPXS_KEY_IFINDEX;

    case NCS_IPXS_KEY_SPT:
        return IPXS_EDU_NCS_IPXS_KEY_SPT;

    case NCS_IPXS_KEY_IP_ADDR:
       return IPXS_EDU_NCS_IPXS_KEY_IP_ADDR;

    default:
        return EDU_FAIL;
    }

    return EDU_FAIL;
}


/****************************************************************************
 * Name          : ipxs_edp_ipxs_ifkey_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IPXS_IFKEY structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ipxs_edp_ipxs_ifkey_info (EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IPXS_IFKEY          *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ipxs_edp_ipxs_ifkey_rules[ ] = {
        {EDU_START, ipxs_edp_ipxs_ifkey_info, 0, 0, 0, sizeof(NCS_IPXS_IFKEY), 0, NULL},        
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_IFKEY*)0)->type, 0, NULL},
        {EDU_TEST, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_IFKEY*)0)->type, 0, ipxs_ifkey_type_test_fnc},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, EDU_EXIT, (uns32)&((NCS_IPXS_IFKEY*)0)->info.iface, 0, NULL},
        {EDU_EXEC, ifsv_edp_spt, 0, 0, EDU_EXIT, (uns32)&((NCS_IPXS_IFKEY*)0)->info.spt, 0, NULL},
        {EDU_EXEC, ncs_edp_ip_addr, 0, 0, EDU_EXIT, (uns32)&((NCS_IPXS_IFKEY*)0)->info.ip_addr, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IPXS_IFKEY *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IPXS_IFKEY **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IPXS_IFKEY));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_ipxs_ifkey_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_evt_if_rec_get_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 IPXS_EVT_IF_REC_GET structure.
 *
 *****************************************************************************/
uns32 ipxs_edp_evt_if_rec_get_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IPXS_EVT_IF_REC_GET     *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ipxs_edp_evt_if_rec_get_rules[ ] = {
        {EDU_START, ipxs_edp_evt_if_rec_get_info, 0, 0, 0, sizeof(IPXS_EVT_IF_REC_GET), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_EVT_IF_REC_GET*)0)->get_type, 0, NULL},
        {EDU_EXEC, ipxs_edp_ipxs_ifkey_info, 0, 0, 0, (uns32)&((IPXS_EVT_IF_REC_GET*)0)->key, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_EVT_IF_REC_GET*)0)->my_svc_id, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (uns32)&((IPXS_EVT_IF_REC_GET*)0)->my_dest, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_EVT_IF_REC_GET*)0)->usr_hdl, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IPXS_EVT_IF_REC_GET *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IPXS_EVT_IF_REC_GET **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IPXS_EVT_IF_REC_GET));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_evt_if_rec_get_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_ipxs_evt_islocal_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 IPXS_EVT_ISLOCAL structure.
 *
 *****************************************************************************/
uns32 ipxs_edp_ipxs_evt_islocal_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IPXS_EVT_ISLOCAL        *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ipxs_edp_ipxs_evt_islocal_rules[ ] = {
        {EDU_START, ipxs_edp_ipxs_evt_islocal_info, 0, 0, 0, sizeof(IPXS_EVT_ISLOCAL), 0, NULL},\
        {EDU_EXEC, ncs_edp_ip_addr, 0, 0, 0, (uns32)&((IPXS_EVT_ISLOCAL*)0)->ip_addr, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, 0, 0, 0, (uns32)&((IPXS_EVT_ISLOCAL*)0)->maskbits, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_EVT_ISLOCAL*)0)->obs, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IPXS_EVT_ISLOCAL *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IPXS_EVT_ISLOCAL **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IPXS_EVT_ISLOCAL));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_ipxs_evt_islocal_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_node_rec_get_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NULL structure.
 *
 *****************************************************************************/
uns32 ipxs_edp_node_rec_get_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    EDU_INST_SET    ipxs_edp_node_rec_get_rules[ ] = {
        {EDU_START, ipxs_edp_node_rec_get_info, 0, 0, 0, 0, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_node_rec_get_rules, 
       NULL, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_ndtoa_ifip_rsp_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IPXS_IPINFO_GET_RSP structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ipxs_edp_ndtoa_ifip_rsp_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IPXS_IPINFO_GET_RSP *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ipxs_edp_ndtoa_ifip_rsp_rules[ ] = {
        {EDU_START, ipxs_edp_ndtoa_ifip_rsp_info, 0, 0, 0, sizeof(NCS_IPXS_IPINFO_GET_RSP), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_IPINFO_GET_RSP*)0)->err, 0, NULL},
        {EDU_EXEC, ipxs_edp_ipxs_intf_rec_info, EDQ_POINTER, 0, 0, (uns32)&((NCS_IPXS_IPINFO_GET_RSP*)0)->ipinfo,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IPXS_IPINFO_GET_RSP *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IPXS_IPINFO_GET_RSP **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IPXS_IPINFO_GET_RSP));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_ndtoa_ifip_rsp_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_ndtoa_ifip_add_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IPXS_IPINFO_ADD structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ipxs_edp_ndtoa_ifip_add_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IPXS_IPINFO_ADD *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ipxs_edp_ndtoa_ifip_add_rules[ ] = {
        {EDU_START, ipxs_edp_ndtoa_ifip_add_info, 0, 0, 0, sizeof(NCS_IPXS_IPINFO_ADD), 0, NULL},
        {EDU_EXEC, ipxs_edp_ipxs_intf_rec_info, EDQ_POINTER, 0, 0, (uns32)&((NCS_IPXS_IPINFO_ADD*)0)->i_ipinfo, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IPXS_IPINFO_ADD *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IPXS_IPINFO_ADD **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IPXS_IPINFO_ADD));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_ndtoa_ifip_add_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_ndtoa_ifip_del_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 IPXS_EVT_NDTOA_IFIP_DEL structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ipxs_edp_ndtoa_ifip_del_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IPXS_IPINFO_DEL *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ipxs_edp_ndtoa_ifip_del_rules[ ] = {
        {EDU_START, ipxs_edp_ndtoa_ifip_del_info, 0, 0, 0, sizeof(NCS_IPXS_IPINFO_DEL), 0, NULL},
        {EDU_EXEC, ipxs_edp_ipxs_intf_rec_info, EDQ_POINTER, 0, 0, (uns32)&((NCS_IPXS_IPINFO_DEL*)0)->i_ipinfo, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IPXS_IPINFO_DEL *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IPXS_IPINFO_DEL **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IPXS_IPINFO_DEL));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_ndtoa_ifip_del_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_ndtoa_ifip_upd_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 IPXS_EVT_NDTOA_IFIP_RSP structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ipxs_edp_ndtoa_ifip_upd_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IPXS_IPINFO_UPD *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ipxs_edp_ndtoa_ifip_upd_rules[ ] = {
        {EDU_START, ipxs_edp_ndtoa_ifip_upd_info, 0, 0, 0, sizeof(NCS_IPXS_IPINFO_UPD), 0, NULL},
        {EDU_EXEC, ipxs_edp_ipxs_intf_rec_info, EDQ_POINTER, 0, 0, (uns32)&((NCS_IPXS_IPINFO_UPD*)0)->i_ipinfo, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL}
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IPXS_IPINFO_UPD *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IPXS_IPINFO_UPD **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IPXS_IPINFO_UPD));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_ndtoa_ifip_upd_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_node_rec_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IPXS_ISLOCAL structure.
 *
 *****************************************************************************/
uns32 ipxs_edp_node_rec_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCSIPXS_NODE_REC        *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ipxs_edp_node_rec_rules[ ] = {
        {EDU_START, ipxs_edp_node_rec_info, 0, 0, 0, sizeof(NCSIPXS_NODE_REC), 0, NULL},\
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCSIPXS_NODE_REC*)0)->ndattr, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCSIPXS_NODE_REC*)0)->rtr_id, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCSIPXS_NODE_REC*)0)->lb_ia, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCSIPXS_NODE_REC*)0)->ud_1, 0, NULL},
        {EDU_EXEC, ncs_edp_uns16, 0, 0, 0, (uns32)&((NCSIPXS_NODE_REC*)0)->as_id, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCSIPXS_NODE_REC*)0)->ud_2, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCSIPXS_NODE_REC*)0)->ud_3, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL}
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCSIPXS_NODE_REC *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCSIPXS_NODE_REC **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCSIPXS_NODE_REC));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_node_rec_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ipxs_edp_ipxs_evt_isloc_rsp_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 NCS_IPXS_ISLOCAL structure.
 *
 *****************************************************************************/
uns32 ipxs_edp_ipxs_evt_isloc_rsp_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IPXS_ISLOCAL        *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ipxs_edp_ipxs_evt_isloc_rsp_rules[ ] = {
        {EDU_START, ipxs_edp_ipxs_evt_isloc_rsp_info, 0, 0, 0, sizeof(NCS_IPXS_ISLOCAL), 0, NULL},\
        {EDU_EXEC, ncs_edp_ip_addr, 0, 0, 0, (uns32)&((NCS_IPXS_ISLOCAL*)0)->i_ip_addr, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, 0, 0, 0, (uns32)&((NCS_IPXS_ISLOCAL*)0)->i_maskbits, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_ISLOCAL*)0)->i_obs, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_ISLOCAL*)0)->o_answer, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((NCS_IPXS_ISLOCAL*)0)->o_iface, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IPXS_ISLOCAL *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IPXS_ISLOCAL **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IPXS_ISLOCAL));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ipxs_edp_ipxs_evt_isloc_rsp_rules, 
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_edp_ifsv_ipxs_info
 *
 * Description   : This is the function which is used to encode decode 
 *                 IPXS_EVT structure.
 * 
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifsv_ipxs_info (EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    IPXS_EVT    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_ipxs_evt_rules[ ] = {
         {EDU_START, ifsv_edp_ifsv_ipxs_info, 0, 0, 0, sizeof(IPXS_EVT), 0, NULL},
         {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_EVT*)0)->type, 0, NULL},
         {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_EVT*)0)->error, 0, NULL},
         {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_EVT*)0)->usrhdl, 0, NULL},
         {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_EVT*)0)->netlink_msg, 0, NULL},
         {EDU_TEST, ncs_edp_uns32, 0, 0, 0, (uns32)&((IPXS_EVT*)0)->type, 0, ipxs_evt_test_type_fnc},

          /* For IPXS_EVT_DTOND_IFIP_INFO */
         {EDU_EXEC, ipxs_edp_evt_if_info, 0, 0, EDU_EXIT, 
            (uns32)&((IPXS_EVT*)0)->info.nd.dtond_upd, 0, NULL},

         /* For IPXS_EVT_ATOND_IFIP_GET */
         {EDU_EXEC, ipxs_edp_evt_if_rec_get_info, 0, 0, EDU_EXIT, 
            (uns32)&((IPXS_EVT*)0)->info.nd.get, 0, NULL},

         /* For IPXS_EVT_ATOND_IFIP_UPD */
         {EDU_EXEC, ipxs_edp_ipxs_intf_rec_info_noptr, 0, 0, EDU_EXIT, 
            (uns32)&((IPXS_EVT*)0)->info.nd.atond_upd, 0, NULL},

         /* For IPXS_EVT_ATOND_IS_LOCAL */
         {EDU_EXEC, ipxs_edp_ipxs_evt_islocal_info, 0, 0, EDU_EXIT, 
            (uns32)&((IPXS_EVT*)0)->info.nd.is_loc, 0, NULL},

         /* For IPXS_EVT_ATOND_NODE_REC_GET, no data to be encoded. 
            So EDU returns normally from this EDP. */

         /* For IPXS_EVT_NDTOD_IFIP_INFO */
         {EDU_EXEC, ipxs_edp_evt_if_info, 0, 0, EDU_EXIT, 
            (uns32)&((IPXS_EVT*)0)->info.d.ndtod_upd, 0, NULL},

         /* For IPXS_EVT_NDTOA_IFIP_RSP */
         {EDU_EXEC, ipxs_edp_ndtoa_ifip_rsp_info, 0, 0, EDU_EXIT, 
            (uns32)&((IPXS_EVT*)0)->info.agent.get_rsp, 0, NULL},

         /* For IPXS_EVT_NDTOA_IFIP_ADD */
         {EDU_EXEC, ipxs_edp_ndtoa_ifip_add_info, 0, 0, EDU_EXIT, 
           (uns32)&((IPXS_EVT*)0)->info.agent.ip_add, 0, NULL},

         /* For IPXS_EVT_NDTOA_IFIP_DEL */
         {EDU_EXEC, ipxs_edp_ndtoa_ifip_del_info, 0, 0, EDU_EXIT, 
           (uns32)&((IPXS_EVT*)0)->info.agent.ip_del, 0, NULL},

         /* For IPXS_EVT_NDTOA_IFIP_UPD */
         {EDU_EXEC, ipxs_edp_ndtoa_ifip_upd_info, 0, 0, EDU_EXIT, 
           (uns32)&((IPXS_EVT*)0)->info.agent.ip_upd, 0, NULL},

         /* For IPXS_EVT_NDTOA_NODE_REC */
         {EDU_EXEC, ipxs_edp_node_rec_info, 0, 0, EDU_EXIT, 
            (uns32)&((IPXS_EVT*)0)->info.agent.node_rec, 0, NULL},

         /* For IPXS_EVT_NDTOA_ISLOC_RSP */
         {EDU_EXEC, ipxs_edp_ipxs_evt_isloc_rsp_info, 0, 0, EDU_EXIT, 
            (uns32)&((IPXS_EVT*)0)->info.agent.isloc_rsp, 0, NULL},

         {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };


    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IPXS_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IPXS_EVT **)ptr;
        if(*d_ptr == NULL)
           *d_ptr = m_MMGR_ALLOC_IPXS_EVT;

        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IPXS_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_ipxs_evt_rules, struct_ptr, ptr_data_len, 
        buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ifd_a2s_ipxs_intf_info_evt
 *
 * Description   : This is the function which is used to encode decode
 *                 IFD_A2S_IPXS_INTF_INFO_EVT structures.
 *
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifd_a2s_ipxs_intf_info_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    IFD_A2S_IPXS_INTF_INFO_EVT  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_a2s_ipxs_intf_info_rules[ ] = {
        {EDU_START, ifsv_edp_ifd_a2s_ipxs_intf_info_evt, 0, 0, 0,
         sizeof(IFD_A2S_IPXS_INTF_INFO_EVT), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (uns32)&((IFD_A2S_IPXS_INTF_INFO_EVT*)0)->type, 0, NULL},
        {EDU_EXEC, ipxs_edp_evt_if_info, 0, 0, 0,
         (uns32)&((IFD_A2S_IPXS_INTF_INFO_EVT*)0)->info, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFD_A2S_IPXS_INTF_INFO_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFD_A2S_IPXS_INTF_INFO_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFD_A2S_IPXS_INTF_INFO_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_a2s_ipxs_intf_info_rules,
                             struct_ptr, ptr_data_len, buf_env, op, o_err);

    return rc;

}/* function ifsv_edp_ifd_a2s_ipxs_intf_info_evt() ends here.  */


