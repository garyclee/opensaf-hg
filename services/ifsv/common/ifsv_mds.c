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
FILE NAME: IFSV_MDS.C
DESCRIPTION: IFSV-MDS Routines

  FUNCTIONS INCLUDED in this module:
  ifsv_edp_spt ................ NCS_IFSV_SPT struct Encode/Decode.
  ifsv_edp_intf_info .......... NCS_IFSV_INTF_INFO struct Encode/Decode.
  ifsv_edp_spt_map ............ NCS_IFSV_SPT_MAP struct Encode/Decode.
  ifsv_edp_stats_info ......... NCS_IFSV_INTF_STATS struct Encode/Decode.
  ifsv_evt_test_type_fnc ...... Function which distinguished the ifsv evts.
  ifsv_edp_intf_data .......... IFSV_INTF_DATA struct Encode/Decode.
  ifsv_edp_create_info ........ IFSV_INTF_CREATE_INFO struct Encode/Decode.
  ifsv_edp_destroy_info ....... IFSV_INTF_DESTROY_INFO struct Encode/Decode.
  ifsv_edp_init_done_info ..... IFSV_EVT_INIT_DONE_INFO struct Encode/Decode.
  ifsv_edp_spt_map_info ....... IFSV_EVT_SPT_MAP_INFO struct Encode/Decode.
  ifsv_edp_age_tmr_info ....... IFSV_EVT_TMR struct Encode/Decode.
  ifsv_edp_rec_sync_info ...... IFSV_EVT_INTF_REC_SYNC struct Encode/Decode.
  ifsv_edp_intf_get_info ...... IFSV_EVT_INTF_INFO_GET struct Encode/Decode.
  ifsv_edp_stats_get_info ..... IFSV_EVT_STATS_INFO struct Encode/Decode.

******************************************************************************/
#include "ifsv.h"
#include "ifsvifap.h"
/****************************************************************************
 * Name          : ifsv_edp_spt
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_SPT structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_spt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_SPT   *struct_ptr = NULL, **d_ptr = NULL;
    uns32          base_ver_401=IFD_MBCSV_VERSION;

    EDU_INST_SET    ifsv_spt_rules[ ] = {
        {EDU_START, ifsv_edp_spt, 0, 0, 0, sizeof(NCS_IFSV_SPT), 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SPT*)0)->shelf, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SPT*)0)->slot, 0, NULL},
        /* embedding subslot changes */
        {EDU_VER_GE, NULL, 0, 0, 2, 0, 0, (EDU_EXEC_RTINE)((uns16 *)(&(base_ver_401)))},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SPT*)0)->subslot, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SPT*)0)->port, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SPT*)0)->type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SPT*)0)->subscr_scope, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_SPT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_SPT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_SPT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_spt_rules, struct_ptr, ptr_data_len,
        buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ncs_svdest
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_SVC_DEST_UPD structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ncs_svdest(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_SVDEST   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_edp_ncs_svdest_rules[ ] = {
        {EDU_START, ifsv_edp_ncs_svdest, 0, 0, 0, sizeof(NCS_SVDEST), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_SVDEST*)0)->svcid, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_SVDEST*)0)->dest, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_SVDEST *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_SVDEST **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_SVDEST));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_edp_ncs_svdest_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_intf_info
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_INTF_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_intf_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                         NCSCONTEXT ptr, uns32 *ptr_data_len,
                         EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                         EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_INTF_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_spt_rules[ ] = {
        {EDU_START, ifsv_edp_intf_info, 0, 0, 0, sizeof(NCS_IFSV_INTF_INFO), 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->if_am, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->if_descr, IFSV_IF_DESC_SIZE, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->if_name, IFSV_IF_NAME_SIZE, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->mtu, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->if_speed, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->phy_addr, 6, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->admin_state, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->oper_state, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->last_change, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->addsvd_cnt, 0, NULL},

        {EDU_EXEC, ifsv_edp_ncs_svdest, EDQ_VAR_LEN_DATA, ncs_edp_uns8, 0,
                    (long)&((NCS_IFSV_INTF_INFO*)0)->addsvd_list,
                    (long)&((NCS_IFSV_INTF_INFO*)0)->addsvd_cnt, NULL},
        {EDU_EXEC_EXT, NULL, NCS_SERVICE_ID_IFSV /* Svc-ID */, NULL, 0, NCS_IFSV_SVC_SUB_ID_NCS_SVDEST /* Sub-ID */, 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->delsvd_cnt, 0, NULL},

        {EDU_EXEC, ifsv_edp_ncs_svdest, EDQ_VAR_LEN_DATA, ncs_edp_uns8, 0,
                    (long)&((NCS_IFSV_INTF_INFO*)0)->delsvd_list,
                    (long)&((NCS_IFSV_INTF_INFO*)0)->delsvd_cnt, NULL},
        {EDU_EXEC_EXT, NULL, NCS_SERVICE_ID_IFSV /* Svc-ID */, NULL, 0, NCS_IFSV_SVC_SUB_ID_NCS_SVDEST /* Sub-ID */, 0, NULL},
#if (NCS_IFSV_BOND_INTF == 1)
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->bind_master_ifindex, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->bind_slave_ifindex, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->bind_num_slaves, 0, NULL},
        {EDU_EXEC, ifsv_edp_nodeid_ifname, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->bind_master_info, 0, NULL},
        {EDU_EXEC, ifsv_edp_nodeid_ifname, 0, 0, 0, (long)&((NCS_IFSV_INTF_INFO*)0)->bind_slave_info, 0, NULL},
#endif
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_INTF_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_INTF_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_INTF_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_spt_rules, struct_ptr, ptr_data_len,
        buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_spt_map
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_SPT_MAP structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_spt_map(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                       NCSCONTEXT ptr, uns32 *ptr_data_len,
                       EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                       EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_SPT_MAP   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_spt_map_rules[ ] = {
        {EDU_START, ifsv_edp_spt_map, 0, 0, 0, sizeof(NCS_IFSV_SPT_MAP), 0, NULL},

        {EDU_EXEC, ifsv_edp_spt, 0, 0, 0, (long)&((NCS_IFSV_SPT_MAP*)0)->spt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SPT_MAP*)0)->if_index, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_SPT_MAP *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_SPT_MAP **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_SPT_MAP));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_spt_map_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_stats_info
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_INTF_STATS structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_stats_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                          NCSCONTEXT ptr, uns32 *ptr_data_len,
                          EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                          EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_INTF_STATS   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_stats_info_rules[ ] = {
        {EDU_START, ifsv_edp_stats_info, 0, 0, 0, sizeof(NCS_IFSV_INTF_STATS), 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->last_chg, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_octs, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_upkts, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_nupkts, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_dscrds, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_errs, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->in_unknown_prots, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_octs, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_upkts, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_nupkts, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_dscrds, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_errs, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_STATS*)0)->out_qlen, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_INTF_STATS *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_INTF_STATS **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_INTF_STATS));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_stats_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}
/****************************************************************************
 * Name          : ifsv_edp_idim_port_type
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_PORT_TYPE structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_idim_port_type(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                              NCSCONTEXT ptr, uns32 *ptr_data_len,
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                              EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_PORT_TYPE   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_port_type_rules[ ] = {
        {EDU_START, ifsv_edp_idim_port_type, 0, 0, 0, sizeof(NCS_IFSV_PORT_TYPE), 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_TYPE*)0)->port_id, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_TYPE*)0)->type, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_PORT_TYPE *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_PORT_TYPE **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_PORT_TYPE));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_port_type_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}



/****************************************************************************
 * Name          : ifsv_edp_idim_port_info
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_PORT_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_idim_port_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                              NCSCONTEXT ptr, uns32 *ptr_data_len,
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                              EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_PORT_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_port_info_rules[ ] = {
        {EDU_START, ifsv_edp_idim_port_info, 0, 0, 0, sizeof(NCS_IFSV_PORT_INFO), 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->if_am, 0, NULL},
        {EDU_EXEC, ifsv_edp_idim_port_type, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->port_type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->phy_addr, 6, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->oper_state, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->admin_state, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->mtu, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->if_name, IFSV_IF_NAME_SIZE, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->speed, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_IFSV_PORT_INFO*)0)->dest, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_PORT_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_PORT_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_PORT_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_port_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_idim_port_status
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_PORT_STATUS structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_idim_port_status(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_PORT_STATUS   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_port_status_rules[ ] = {
        {EDU_START, ifsv_edp_idim_port_status, 0, 0, 0, sizeof(NCS_IFSV_PORT_STATUS), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATUS*)0)->oper_state, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_PORT_STATUS *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_PORT_STATUS **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_PORT_STATUS));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_port_status_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_idim_ifnd_up_info
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFND_UP_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_idim_ifnd_up_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                 NCSCONTEXT ptr, uns32 *ptr_data_len,
                                 EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                 EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFND_UP_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_hw_ifnd_up_rules[ ] = {
        {EDU_START, ifsv_edp_idim_ifnd_up_info, 0, 0, 0, sizeof(NCS_IFND_UP_INFO), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFND_UP_INFO*)0)->vrid, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFND_UP_INFO*)0)->nodeid, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_IFND_UP_INFO*)0)->ifnd_addr, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFND_UP_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFND_UP_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFND_UP_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_hw_ifnd_up_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_idim_hw_rcv_info
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_HW_INFO structure, which the IDIM receives.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_idim_hw_rcv_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_HW_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_idim_hw_info_rules[ ] = {
        {EDU_START, ifsv_edp_idim_hw_rcv_info, 0, 0, 0, sizeof(NCS_IFSV_HW_INFO), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_INFO*)0)->msg_type, 0, NULL},
/*EXT_INT*/
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_INFO*)0)->subscr_scope, 0, NULL},         
   
        {EDU_EXEC, ifsv_edp_idim_port_type, 0, 0, 0, (long)&((NCS_IFSV_HW_INFO*)0)->port_type, 0, NULL},

        {EDU_TEST, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_INFO*)0)->msg_type, 0, ifsv_hw_resp_evt_test_type_fnc},

        /* For NCS_IFSV_HW_DRV_STATS */
        {EDU_EXEC, ifsv_edp_idim_port_stats, 0, 0, EDU_EXIT,
            (long)&((NCS_IFSV_HW_INFO*)0)->info.stats, 0, NULL},

      /* For NCS_IFSV_HW_DRV_PORT_REG */
        {EDU_EXEC, ifsv_edp_idim_port_info, 0, 0, EDU_EXIT,
            (long)&((NCS_IFSV_HW_INFO*)0)->info.reg_port, 0, NULL},

            /* For NCS_IFSV_HW_DRV_PORT_STATUS */
        {EDU_EXEC, ifsv_edp_idim_port_status, 0, 0, EDU_EXIT,
            (long)&((NCS_IFSV_HW_INFO*)0)->info.port_status, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_HW_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_HW_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_HW_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_idim_hw_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_idim_hw_req
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_HW_REQ_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_idim_hw_req(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_HW_REQ_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_hw_ifnd_up_rules[ ] = {
        {EDU_START, ifsv_edp_idim_ifnd_up_info, 0, 0, 0, sizeof(NCS_IFSV_HW_REQ_INFO), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_REQ_INFO*)0)->svc_id, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_IFSV_HW_REQ_INFO*)0)->dest_addr, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_REQ_INFO*)0)->app_hdl, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_HW_REQ_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_HW_REQ_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_HW_REQ_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_hw_ifnd_up_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_idim_hw_req_info
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_HW_DRV_REQ structure, which the IDIM sends the request
 *                 to the hardware drv.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_idim_hw_req_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_HW_DRV_REQ   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_idim_hw_req_rules[ ] = {
        {EDU_START, ifsv_edp_idim_hw_req_info, 0, 0, 0, sizeof(NCS_IFSV_HW_DRV_REQ), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_DRV_REQ*)0)->req_type, 0, NULL},
/*EXT_INT */
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_DRV_REQ*)0)->subscr_scope, 0, NULL},    
   
        {EDU_EXEC, ifsv_edp_idim_port_type, 0, 0, 0, (long)&((NCS_IFSV_HW_DRV_REQ*)0)->port_type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_DRV_REQ*)0)->drv_data, 0, NULL},

        {EDU_TEST, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_HW_DRV_REQ*)0)->req_type, 0, ifsv_hw_req_evt_test_type_fnc},

        /* For NCS_IFSV_HW_DRV_STATS */
        {EDU_EXEC, ifsv_edp_idim_hw_req, 0, 0, EDU_EXIT,
            (long)&((NCS_IFSV_HW_DRV_REQ*)0)->info.req_info, 0, NULL},

      /* For NCS_IFSV_HW_DRV_IFND_UP */
        {EDU_EXEC, ifsv_edp_idim_ifnd_up_info, 0, 0, EDU_EXIT,
            (long)&((NCS_IFSV_HW_DRV_REQ*)0)->info.ifnd_info, 0, NULL},

            /* For NCS_IFSV_HW_DRV_SET_PARAM */
        {EDU_EXEC, ifsv_edp_intf_info, 0, 0, EDU_EXIT,
            (long)&((NCS_IFSV_HW_DRV_REQ*)0)->info.set_param, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_HW_DRV_REQ *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_HW_DRV_REQ **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_HW_DRV_REQ));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_idim_hw_req_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_idim_port_stats
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_PORT_STATS structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_idim_port_stats(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                NCSCONTEXT ptr, uns32 *ptr_data_len,
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_PORT_STATS   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_port_stats_rules[ ] = {
        {EDU_START, ifsv_edp_idim_port_stats, 0, 0, 0, sizeof(NCS_IFSV_PORT_STATS), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATS*)0)->status, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATS*)0)->app_svc_id, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATS*)0)->app_dest, 0, NULL},
        {EDU_EXEC, ifsv_edp_stats_info, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATS*)0)->stats, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_PORT_STATS*)0)->usr_hdl, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_PORT_STATS *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_PORT_STATS **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_PORT_STATS));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_port_stats_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}



/****************************************************************************
 * Name          : ifsv_ifnd_evt_test_type_fnc
 *
 * Description   : This is the function which is used to test the event type
 *
 *
 * Notes         : None.
 *****************************************************************************/
int ifsv_ifnd_evt_test_type_fnc(NCSCONTEXT arg)
{
    /***** NOT UPDATED THIS SINCE THIS MIGHT BE DELETED *****/
    IFSV_EVT_TYPE    evt;

    if(arg == NULL)
        return EDU_FAIL;

    evt = *(IFSV_EVT_TYPE *)arg;
    switch(evt)
    {
    case IFD_EVT_INTF_CREATE:
        return 3;
    case IFD_EVT_INTF_DESTROY:
        return 4;
    case IFD_EVT_INIT_DONE:
        return EDU_EXIT;
    case IFD_EVT_IFINDEX_REQ:
       return 5;
    case IFD_EVT_IFINDEX_CLEANUP:
       return 6;
    case IFD_EVT_TMR_EXP:
       return EDU_EXIT;
    case IFD_EVT_INTF_REC_SYNC:
       return 7;

    case IFND_EVT_INTF_CREATE:
       return 9;
    case IFND_EVT_INTF_DESTROY:
       return 10;
    case IFND_EVT_INIT_DONE:
       return 8;
    case IFND_EVT_IFINDEX_RESP:
       return 11;
    case IFND_EVT_INTF_INFO_GET:
       return 13;
    case IFND_EVT_TMR_EXP:
       return 12;
    case IFND_EVT_IDIM_STATS_RESP:
       return 14;


    default:
        return EDU_EXIT;
    }
    return EDU_FAIL;
}

/****************************************************************************
 * Name          : ifsv_ifd_evt_test_type_fnc
 *
 * Description   : This is the function which is used to test the event type
 *
 *
 * Notes         : None.
 *****************************************************************************/
int ifsv_ifd_evt_test_type_fnc(NCSCONTEXT arg)
{
    /***** NOT UPDATED THIS SINCE THIS MIGHT BE DELETED *****/
    IFSV_EVT_TYPE    evt;

    if(arg == NULL)
        return EDU_FAIL;

    evt = *(IFSV_EVT_TYPE *)arg;
    switch(evt)
    {
    case IFD_EVT_INTF_CREATE:
        return 3;
    case IFD_EVT_INTF_DESTROY:
        return 4;
    case IFD_EVT_INIT_DONE:
        return 5;
    case IFD_EVT_IFINDEX_REQ:
       return 6;
    case IFD_EVT_IFINDEX_CLEANUP:
       return 7;
    case IFD_EVT_TMR_EXP:
       return 8;
    case IFD_EVT_INTF_REC_SYNC:
       return 9;

    case IFND_EVT_INTF_CREATE:
       return 10;
    case IFND_EVT_INTF_DESTROY:
       return 11;
    case IFND_EVT_INIT_DONE:
       return EDU_EXIT;
    case IFND_EVT_IFINDEX_RESP:
       return 12;
    case IFND_EVT_INTF_INFO_GET:
       return EDU_EXIT;
    case IFND_EVT_TMR_EXP:
       return EDU_EXIT;
    case IFND_EVT_IDIM_STATS_RESP:
       return EDU_EXIT;

    default:
        return EDU_EXIT;
    }
    return EDU_FAIL;
}

/****************************************************************************
 * Name          : ifsv_hw_resp_evt_test_type_fnc
 *
 * Description   : This is the function which is used to test the event type
 *                 received from the hardware driver.
 *
 *
 * Notes         : None.
 *****************************************************************************/
int ifsv_hw_resp_evt_test_type_fnc(NCSCONTEXT arg)
{
    typedef enum {
        LCL_TEST_JUMP_OFFSET_DRV_STATS = 1,
        LCL_TEST_JUMP_OFFSET_DRV_PORT_REG,
        LCL_TEST_JUMP_OFFSET_DRV_PORT_STATUS
    }LCL_TEST_JUMP_OFFSET;
   NCS_IFSV_HW_DRV_MSG_TYPE    evt;

   if(arg == NULL)
      return EDU_FAIL;

   evt = *(NCS_IFSV_HW_DRV_MSG_TYPE *)arg;
   switch(evt)
   {
   case NCS_IFSV_HW_DRV_STATS:
      return LCL_TEST_JUMP_OFFSET_DRV_STATS;
   case NCS_IFSV_HW_DRV_PORT_REG:
      return LCL_TEST_JUMP_OFFSET_DRV_PORT_REG;
   case NCS_IFSV_HW_DRV_PORT_STATUS:
      return LCL_TEST_JUMP_OFFSET_DRV_PORT_STATUS;
   default:
      return EDU_EXIT;
   }
   return EDU_FAIL;
}


/****************************************************************************
 * Name          : ifsv_hw_req_evt_test_type_fnc
 *
 * Description   : This is the function which is used to test the event type
 *                 which IDIM request the hardware.
 *
 *
 * Notes         : None.
 *****************************************************************************/
int ifsv_hw_req_evt_test_type_fnc(NCSCONTEXT arg)
{
    typedef enum {
        LCL_TEST_JUMP_OFFSET_DRV_STATS = 1,
        LCL_TEST_JUMP_OFFSET_DRV_IFND_UP,
        LCL_TEST_JUMP_OFFSET_DRV_SET_PARAM
    }LCL_TEST_JUMP_OFFSET;
   NCS_IFSV_HW_DRV_MSG_TYPE    evt;

   if(arg == NULL)
      return EDU_FAIL;

   evt = *(NCS_IFSV_HW_DRV_MSG_TYPE *)arg;
   switch(evt)
   {
   case NCS_IFSV_HW_DRV_STATS:
      return LCL_TEST_JUMP_OFFSET_DRV_STATS;
   case NCS_IFSV_HW_DRV_SET_PARAM:
      return LCL_TEST_JUMP_OFFSET_DRV_SET_PARAM;
   case NCS_IFSV_HW_DRV_IFND_UP:
      return LCL_TEST_JUMP_OFFSET_DRV_IFND_UP;
   default:
      return EDU_EXIT;
   }
   return EDU_FAIL;
}


/****************************************************************************
 * Name          : ifa_evt_test_type_fnc
 *
 * Description   : This is the function which is used to test the event type
 *
 *
 * Notes         : None.
 *****************************************************************************/
int ifa_evt_test_type_fnc (NCSCONTEXT arg)
{
    /***** NOT UPDATED THIS SINCE THIS IS NOT USED ANYWHERE *****/
    IFSV_EVT_TYPE    evt;

    if(arg == NULL)
        return EDU_FAIL;

    evt = *(IFSV_EVT_TYPE *)arg;
    switch(evt)
    {
    case IFD_EVT_INTF_CREATE:
        return 3;
    case IFD_EVT_INTF_DESTROY:
        return 4;
    case IFD_EVT_INIT_DONE:
        return 5;
    case IFD_EVT_IFINDEX_REQ:
       return 6;
    case IFD_EVT_IFINDEX_CLEANUP:
       return 7;
    case IFD_EVT_TMR_EXP:
       return 8;
    case IFD_EVT_INTF_REC_SYNC:
       return 9;

    case IFND_EVT_INTF_CREATE:
       return 10;
    case IFND_EVT_INTF_DESTROY:
       return 11;
    case IFND_EVT_INIT_DONE:
       return EDU_EXIT;
    case IFND_EVT_IFINDEX_RESP:
       return 12;
    case IFND_EVT_INTF_INFO_GET:
       return EDU_EXIT;
    case IFND_EVT_TMR_EXP:
       return EDU_EXIT;
    case IFND_EVT_IDIM_STATS_RESP:
       return EDU_EXIT;

    default:
        return EDU_EXIT;
    }
    return EDU_FAIL;
}

/****************************************************************************
 * Name          : ifsv_edp_intf_data
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_INTF_DATA structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_intf_data(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                         NCSCONTEXT ptr, uns32 *ptr_data_len,
                         EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                         EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    IFSV_INTF_DATA   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_intf_data_rules[ ] = {
        {EDU_START, ifsv_edp_intf_data, 0, 0, 0, sizeof(IFSV_INTF_DATA), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->if_index, 0, NULL},
        {EDU_EXEC, ifsv_edp_spt, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->spt_info, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->originator, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->originator_mds_destination, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->current_owner, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->current_owner_mds_destination, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->active, 0, NULL},
        {EDU_EXEC, ifsv_edp_intf_info, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->if_info, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->rec_type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->last_msg, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->no_data, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_DATA*)0)->evt_orig, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_INTF_DATA *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_INTF_DATA **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_INTF_DATA));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_intf_data_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_create_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_INTF_CREATE_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_create_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFSV_INTF_CREATE_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_create_rules[ ] = {
        {EDU_START, ifsv_edp_create_info, 0, 0, 0, sizeof(IFSV_INTF_CREATE_INFO), 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_CREATE_INFO*)0)->if_attr, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_CREATE_INFO*)0)->evt_orig, 0, NULL},
        {EDU_EXEC, ifsv_edp_intf_data, 0, 0, 0, (long)&((IFSV_INTF_CREATE_INFO*)0)->intf_data, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_INTF_CREATE_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_INTF_CREATE_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_INTF_CREATE_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_create_rules, struct_ptr,
       ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_destroy_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_INTF_DESTROY_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_destroy_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFSV_INTF_DESTROY_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_destroy_rules[ ] = {
        {EDU_START, ifsv_edp_destroy_info, 0, 0, 0, sizeof(IFSV_INTF_DESTROY_INFO), 0, NULL},

        {EDU_EXEC, ifsv_edp_spt, 0, 0, 0, (long)&((IFSV_INTF_DESTROY_INFO*)0)->spt_type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_INTF_DESTROY_INFO*)0)->orign, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((IFSV_INTF_DESTROY_INFO*)0)->own_dest, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_INTF_DESTROY_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_INTF_DESTROY_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_INTF_DESTROY_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_destroy_rules, struct_ptr,
       ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_init_done_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_INIT_DONE_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_init_done_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                              NCSCONTEXT ptr, uns32 *ptr_data_len,
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                              EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFSV_EVT_INIT_DONE_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_init_done_rules[ ] = {
        {EDU_START, ifsv_edp_init_done_info, 0, 0, 0, sizeof(IFSV_EVT_INIT_DONE_INFO), 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_INIT_DONE_INFO*)0)->init_done, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((IFSV_EVT_INIT_DONE_INFO*)0)->my_dest, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_EVT_INIT_DONE_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_EVT_INIT_DONE_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_EVT_INIT_DONE_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_init_done_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_spt_map_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_SPT_MAP_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_spt_map_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFSV_EVT_SPT_MAP_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_spt_map_rules[ ] = {
        {EDU_START, ifsv_edp_spt_map_info, 0, 0, 0, sizeof(IFSV_EVT_SPT_MAP_INFO), 0, NULL},

        {EDU_EXEC, ifsv_edp_spt_map, 0, 0, 0, (long)&((IFSV_EVT_SPT_MAP_INFO*)0)->spt_map, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_SPT_MAP_INFO*)0)->app_svc_id, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((IFSV_EVT_SPT_MAP_INFO*)0)->app_dest, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_EVT_SPT_MAP_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_EVT_SPT_MAP_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_EVT_SPT_MAP_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_spt_map_rules,
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
int ifsv_ifkey_type_test_fnc (NCSCONTEXT arg)
{
    enum
    {
       LCL_TEST_JUMP_OFFSET_KEY_IFINDEX = 1,
       LCL_TEST_JUMP_OFFSET_KEY_SPT,
       LCL_TEST_JUMP_OFFSET_DEFAULT
    };
    NCS_IFSV_KEY_TYPE    type;

    if(arg == NULL)
        return EDU_FAIL;

    type = *(NCS_IFSV_KEY_TYPE *)arg;
    switch(type)
    {
    case NCS_IFSV_KEY_IFINDEX:
        return LCL_TEST_JUMP_OFFSET_KEY_IFINDEX;
    case NCS_IFSV_KEY_SPT:
        return LCL_TEST_JUMP_OFFSET_KEY_SPT;
    default:
        return LCL_TEST_JUMP_OFFSET_DEFAULT;
    }
    return EDU_FAIL;
}


/****************************************************************************
 * Name          : ifsv_edp_ifkey_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_SPT_MAP_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifkey_info (EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_IFKEY          *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_spt_map_rules[ ] = {
        {EDU_START, ifsv_edp_ifkey_info, 0, 0, 0, sizeof(NCS_IFSV_IFKEY), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_IFKEY*)0)->type, 0, NULL},
        {EDU_TEST, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_IFKEY*)0)->type, 0, ifsv_ifkey_type_test_fnc},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_IFKEY*)0)->info.iface, 0, NULL},
        {EDU_EXEC, ifsv_edp_spt, 0, 0, 0, (long)&((NCS_IFSV_IFKEY*)0)->info.spt, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_IFKEY *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_IFKEY **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_IFKEY));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_spt_map_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_age_tmr_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_TMR structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_age_tmr_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFSV_EVT_TMR   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_age_tmr_rules[ ] = {
        {EDU_START, ifsv_edp_age_tmr_info, 0, 0, 0, sizeof(IFSV_EVT_TMR), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_TMR*)0)->tmr_type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_TMR*)0)->info.ifindex, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_EVT_TMR *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_EVT_TMR **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_EVT_TMR));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_age_tmr_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_edp_svcd_upd
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_SVC_DEST_UPD structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_svcd_upd(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_SVC_DEST_UPD   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_edp_svcd_upd_rules[ ] = {
        {EDU_START, ifsv_edp_svcd_upd, 0, 0, 0, sizeof(NCS_IFSV_SVC_DEST_UPD), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SVC_DEST_UPD*)0)->i_type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SVC_DEST_UPD*)0)->i_ifindex, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SVC_DEST_UPD*)0)->i_svdest.svcid, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_IFSV_SVC_DEST_UPD*)0)->i_svdest.dest, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_SVC_DEST_UPD *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_SVC_DEST_UPD **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_SVC_DEST_UPD));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_edp_svcd_upd_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_edp_svcd_get
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_SVC_DEST_UPD structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_svcd_get(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_SVC_DEST_GET   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_edp_svcd_get_rules[ ] = {
        {EDU_START, ifsv_edp_svcd_get, 0, 0, 0, sizeof(NCS_IFSV_SVC_DEST_GET), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SVC_DEST_GET*)0)->i_ifindex, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SVC_DEST_GET*)0)->i_svcid, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((NCS_IFSV_SVC_DEST_GET*)0)->o_dest, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_SVC_DEST_GET*)0)->o_answer, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_SVC_DEST_GET *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_SVC_DEST_GET **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_SVC_DEST_GET));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_edp_svcd_get_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_edp_rec_sync_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_INTF_REC_SYNC structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_rec_sync_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFSV_EVT_INTF_REC_SYNC   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_rec_sync_rules[ ] = {
        {EDU_START, ifsv_edp_rec_sync_info, 0, 0, 0, sizeof(IFSV_EVT_INTF_REC_SYNC), 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((IFSV_EVT_INTF_REC_SYNC*)0)->ifnd_vcard_id, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_EVT_INTF_REC_SYNC *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_EVT_INTF_REC_SYNC **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_EVT_INTF_REC_SYNC));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_rec_sync_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_intf_get_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_INTF_INFO_GET structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_intf_get_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFSV_EVT_INTF_INFO_GET   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_get_intf_info_rules[ ] = {
        {EDU_START, ifsv_edp_intf_get_info, 0, 0, 0, sizeof(IFSV_EVT_INTF_INFO_GET), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_INTF_INFO_GET*)0)->get_type, 0, NULL},
        {EDU_EXEC, ifsv_edp_ifkey_info, 0, 0, 0, (long)&((IFSV_EVT_INTF_INFO_GET*)0)->ifkey, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_INTF_INFO_GET*)0)->info_type ,0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_INTF_INFO_GET*)0)->my_svc_id ,0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((IFSV_EVT_INTF_INFO_GET*)0)->my_dest, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_INTF_INFO_GET*)0)->usr_hdl ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_EVT_INTF_INFO_GET *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_EVT_INTF_INFO_GET **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_EVT_INTF_INFO_GET));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_get_intf_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_stats_get_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_STATS_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_stats_get_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFSV_EVT_STATS_INFO   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_get_stats_info_rules[ ] = {
        {EDU_START, ifsv_edp_stats_get_info, 0, 0, 0, sizeof(IFSV_EVT_STATS_INFO), 0, NULL},
           {EDU_EXEC, ifsv_edp_spt, 0, 0, 0, (long)&((IFSV_EVT_STATS_INFO*)0)->spt_type, 0, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_STATS_INFO*)0)->status ,0, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_STATS_INFO*)0)->svc_id ,0, NULL},
           {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((IFSV_EVT_STATS_INFO*)0)->dest, 0, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT_STATS_INFO*)0)->usr_hdl ,0, NULL},
           {EDU_EXEC, ifsv_edp_stats_info, 0, 0, 0, (long)&((IFSV_EVT_STATS_INFO*)0)->stat_info ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_EVT_STATS_INFO *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_EVT_STATS_INFO **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_EVT_STATS_INFO));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_get_stats_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ifa_ifrec_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_STATS_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifa_ifrec_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_INTF_REC   *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_get_stats_info_rules[ ] = {
        {EDU_START, ifsv_edp_ifa_ifrec_info, 0, 0, 0, sizeof(NCS_IFSV_INTF_REC), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_REC*)0)->if_index, 0, NULL},
        {EDU_EXEC, ifsv_edp_spt, 0, 0, 0, (long)&((NCS_IFSV_INTF_REC*)0)->spt_info, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_INTF_REC*)0)->info_type, 0, NULL},
        {EDU_EXEC, ifsv_edp_intf_info, 0, 0, 0, (long)&((NCS_IFSV_INTF_REC*)0)->if_info, 0, NULL},
        {EDU_EXEC, ifsv_edp_stats_info, 0, 0, 0, (long)&((NCS_IFSV_INTF_REC*)0)->if_stats, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_INTF_REC *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_INTF_REC **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_INTF_REC));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_get_stats_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_edp_ifa_intf_create_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_STATS_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifa_intf_create_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFA_EVT                 *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_get_stats_info_rules[ ] = {
        {EDU_START, ifsv_edp_ifa_intf_create_info, 0, 0, 0, sizeof(IFA_EVT), 0, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFA_EVT*)0)->usrhdl, 0, NULL},
           {EDU_EXEC,  ncs_edp_uns32, 0, 0, 0, (long)&((IFA_EVT*)0)->subscr_scope, 0, NULL},
           {EDU_EXEC, ifsv_edp_ifa_ifrec_info, 0, 0, 0, (long)&((IFA_EVT*)0)->info.if_add_upd, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFA_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFA_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_INTF_REC));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_get_stats_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_edp_ifa_intf_destroy_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_STATS_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifa_intf_destroy_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFA_EVT                 *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_get_stats_info_rules[ ] = {
        {EDU_START, ifsv_edp_ifa_intf_destroy_info, 0, 0, 0, sizeof(IFA_EVT), 0, NULL},
        {EDU_EXEC,  ncs_edp_uns32, 0, 0, 0, (long)&((IFA_EVT*)0)->subscr_scope, 0, NULL},
        {EDU_EXEC,  ncs_edp_uns32, 0, 0, 0, (long)&((IFA_EVT*)0)->info.if_del, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFA_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFA_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFA_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_get_stats_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_edp_ifa_intf_create_rsp_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_STATS_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifa_intf_create_rsp_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFA_EVT                 *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_edp_ifa_intf_create_rsp_rules[ ] = {
        {EDU_START, ifsv_edp_ifa_intf_create_rsp_info, 0, 0, 0, sizeof(IFA_EVT), 0, NULL},
        {EDU_EXEC,  ncs_edp_uns32, 0, 0, 0, (long)&((IFA_EVT*)0)->subscr_scope, 0, NULL},
        {EDU_EXEC,  ncs_edp_uns32, 0, 0, 0, (long)&((IFA_EVT*)0)->info.if_add_rsp_idx, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFA_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFA_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFA_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_edp_ifa_intf_create_rsp_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_edp_ifa_ifget_info_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_STATS_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifa_ifget_info_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_IFGET_RSP                 *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_get_stats_info_rules[ ] = {
        {EDU_START, ifsv_edp_stats_get_info, 0, 0, 0, sizeof(NCS_IFSV_IFGET_RSP), 0, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_IFGET_RSP*)0)->error, 0, NULL},
           {EDU_EXEC, ifsv_edp_ifa_ifrec_info, 0, 0, 0, (long)&((NCS_IFSV_IFGET_RSP*)0)->if_rec, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_IFGET_RSP *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_IFGET_RSP **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_IFGET_RSP));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_get_stats_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ifa_ifget_rsp_info
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT_STATS_INFO structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifa_ifget_rsp_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFA_EVT                 *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_get_stats_info_rules[ ] = {
        {EDU_START, ifsv_edp_stats_get_info, 0, 0, 0, sizeof(IFA_EVT), 0, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFA_EVT*)0)->usrhdl, 0, NULL},
           {EDU_EXEC,  ncs_edp_uns32, 0, 0, 0, (long)&((IFA_EVT*)0)->subscr_scope, 0, NULL},
           {EDU_EXEC, ifsv_edp_ifa_ifget_info_info, 0, 0, 0, (long)&((IFA_EVT*)0)->info.ifget_rsp, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFA_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFA_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFA_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_get_stats_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_evt_test_type_fnc
 *
 * Description   : This is the function offsets for encoding/decoding IFSV_EVT
 *                 Data structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
int ifsv_evt_test_type_fnc (NCSCONTEXT arg)
{
    IFSV_EVT_TYPE    evt;
    enum
    {
       IFSV_EDU_IFD_EVT_INTF_CREATE_OFFSET = 1,
       IFSV_EDU_IFD_EVT_INTF_DESTROY_OFFSET,
       IFSV_EDU_IFD_EVT_INIT_DONE_OFFSET,
       IFSV_EDU_IFD_EVT_IFINDEX_REQ_OFFSET,
       IFSV_EDU_IFD_EVT_IFINDEX_CLEANUP_OFFSET,
       IFSV_EDU_IFD_EVT_INTF_AGING_TMR_EXP_OFFSET,
       IFSV_EDU_IFD_EVT_INTF_REC_SYNC_OFFSET,
       IFSV_EDU_IFD_EVT_SVCD_UPD,
       IFSV_EDU_IFND_EVT_INTF_CREATE_OFFSET,
       IFSV_EDU_IFND_EVT_INTF_DESTROY_OFFSET,
       IFSV_EDU_IFND_EVT_INIT_DONE_OFFSET,
       IFSV_EDU_IFND_EVT_INTF_INFO_GET_OFFSET,
       IFSV_EDU_IFND_EVT_IDIM_STATS_RESP_OFFSET,
       IFSV_EDU_IFND_EVT_INTF_AGING_TMR_EXP_OFFSET,
       IFSV_EDU_IFND_EVT_SVCD_UPD_FROM_IFA,
       IFSV_EDU_IFND_EVT_SVCD_UPD_FROM_IFD,
       IFSV_EDU_IFND_EVT_SVCD_GET,
       IFSV_EDU_IFND_EVT_IFINDEX_RESP_OFFSET,
       IFSV_EDU_IFA_EVT_INTF_CREATE_OFFSET,
       IFSV_EDU_IFA_EVT_INTF_DESTROY_OFFSET,
       IFSV_EDU_IFA_EVT_IFINFO_GET_RSP_OFFSET,
       IFSV_EDU_IFA_EVT_INTF_CREATE_RSP_OFFSET,
       IFSV_EDU_IFA_EVT_SVCD_GET_RSP_OFFSET,
#if(NCS_IFSV_IPXS == 1)
       IFSV_EDU_IFSV_IPXS_EVT,
#endif
#if(NCS_VIP == 1)
       IFSV_EDU_IFA_VIPD_INFO_ADD_REQ_OFFSET,
       IFSV_EDU_IFND_VIPD_INFO_ADD_REQ_OFFSET,
       IFSV_EDU_IFA_VIP_FREE_REQ_OFFSET,
       IFSV_EDU_IFND_VIP_FREE_REQ_OFFSET,
       IFSV_EDU_IFND_VIPD_INFO_ADD_REQ_RESP_OFFSET,
       IFSV_EDU_IFD_VIPD_INFO_ADD_REQ_RESP_OFFSET,
       IFSV_EDU_IFND_VIP_FREE_REQ_RESP_OFFSET,
       IFSV_EDU_IFD_VIP_FREE_REQ_RESP_OFFSET,
       IFSV_EDU_VIP_ERROR_OFFSET,
       IFSV_EDU_VIP_COMMON_EVENT_OFFSET,
       IFSV_EDU_VIP_IP_FROM_STALE_ENTRY_OFFSET,

#endif
    };

    if(arg == NULL)
        return EDU_FAIL;

    evt = *(IFSV_EVT_TYPE *)arg;
    switch(evt)
    {
    case IFD_EVT_INTF_CREATE:
        return IFSV_EDU_IFD_EVT_INTF_CREATE_OFFSET;

    case IFD_EVT_INTF_DESTROY:
        return IFSV_EDU_IFD_EVT_INTF_DESTROY_OFFSET;

    case IFD_EVT_INIT_DONE:
        return IFSV_EDU_IFD_EVT_INIT_DONE_OFFSET;

    case IFD_EVT_IFINDEX_REQ:
       return IFSV_EDU_IFD_EVT_IFINDEX_REQ_OFFSET;

    case IFD_EVT_IFINDEX_CLEANUP:
       return IFSV_EDU_IFD_EVT_IFINDEX_CLEANUP_OFFSET;

    case IFD_EVT_TMR_EXP:
       return IFSV_EDU_IFD_EVT_INTF_AGING_TMR_EXP_OFFSET;

    case IFD_EVT_INTF_REC_SYNC:
       return IFSV_EDU_IFD_EVT_INTF_REC_SYNC_OFFSET;

    case IFD_EVT_SVCD_UPD:
       return IFSV_EDU_IFD_EVT_SVCD_UPD;

    case IFND_EVT_INTF_CREATE:
       return IFSV_EDU_IFND_EVT_INTF_CREATE_OFFSET;

    case IFND_EVT_INTF_DESTROY:
       return IFSV_EDU_IFND_EVT_INTF_DESTROY_OFFSET;

    case IFND_EVT_INIT_DONE:
       return IFSV_EDU_IFND_EVT_INIT_DONE_OFFSET;

    case IFND_EVT_IFINDEX_RESP:
       return IFSV_EDU_IFND_EVT_IFINDEX_RESP_OFFSET;

    case IFND_EVT_INTF_INFO_GET:
       return IFSV_EDU_IFND_EVT_INTF_INFO_GET_OFFSET;

    case IFND_EVT_TMR_EXP:
       return IFSV_EDU_IFND_EVT_INTF_AGING_TMR_EXP_OFFSET;

    case IFND_EVT_SVCD_UPD_FROM_IFA:
       return IFSV_EDU_IFND_EVT_SVCD_UPD_FROM_IFA;

    case IFND_EVT_SVCD_UPD_FROM_IFD:
       return IFSV_EDU_IFND_EVT_SVCD_UPD_FROM_IFD;

    case IFND_EVT_SVCD_GET:
       return IFSV_EDU_IFND_EVT_SVCD_GET;

    case IFND_EVT_INTF_CREATE_RSP:
       return IFSV_EDU_IFND_EVT_IFINDEX_RESP_OFFSET;

    case IFND_EVT_INTF_DESTROY_RSP:
       return IFSV_EDU_IFND_EVT_IFINDEX_RESP_OFFSET;

    case IFND_EVT_IDIM_STATS_RESP:
       return IFSV_EDU_IFND_EVT_IDIM_STATS_RESP_OFFSET;

    case IFA_EVT_INTF_CREATE:
    case IFA_EVT_INTF_UPDATE:
       return IFSV_EDU_IFA_EVT_INTF_CREATE_OFFSET;

    case IFA_EVT_INTF_DESTROY:
       return IFSV_EDU_IFA_EVT_INTF_DESTROY_OFFSET;

    case IFA_EVT_IFINFO_GET_RSP:
       return IFSV_EDU_IFA_EVT_IFINFO_GET_RSP_OFFSET;

    case IFA_EVT_INTF_CREATE_RSP:
       return IFSV_EDU_IFA_EVT_INTF_CREATE_RSP_OFFSET;

    case IFA_EVT_INTF_DESTROY_RSP:
       return IFSV_EDU_IFND_EVT_IFINDEX_RESP_OFFSET;

    case IFA_EVT_SVCD_UPD_RSP:
       return IFSV_EDU_IFND_EVT_SVCD_UPD_FROM_IFD;

    case IFA_EVT_SVCD_GET_RSP:
       return IFSV_EDU_IFA_EVT_SVCD_GET_RSP_OFFSET;

    case IFND_EVT_SVCD_UPD_RSP_FROM_IFD:
       return IFSV_EDU_IFND_EVT_SVCD_UPD_FROM_IFD;

    case IFA_EVT_SVCD_GET_SYNC_RSP:
       return IFSV_EDU_IFND_EVT_SVCD_UPD_FROM_IFD;

#if(NCS_IFSV_IPXS == 1)
    case IFSV_IPXS_EVT:
      return IFSV_EDU_IFSV_IPXS_EVT;
#endif
#if (NCS_VIP == 1)
    case IFA_VIPD_INFO_ADD_REQ:
       return IFSV_EDU_IFA_VIPD_INFO_ADD_REQ_OFFSET;
    case IFND_VIPD_INFO_ADD_REQ:
       return IFSV_EDU_IFND_VIPD_INFO_ADD_REQ_OFFSET;
    case IFA_VIP_FREE_REQ:
       return IFSV_EDU_IFA_VIP_FREE_REQ_OFFSET;
    case IFND_VIP_FREE_REQ:
       return IFSV_EDU_IFND_VIP_FREE_REQ_OFFSET;
    case IFND_VIPD_INFO_ADD_REQ_RESP:
       return IFSV_EDU_IFND_VIPD_INFO_ADD_REQ_RESP_OFFSET;
    case IFD_VIPD_INFO_ADD_REQ_RESP:
       return IFSV_EDU_IFD_VIPD_INFO_ADD_REQ_RESP_OFFSET;
    case IFND_VIP_FREE_REQ_RESP:
       return IFSV_EDU_IFND_VIP_FREE_REQ_RESP_OFFSET;
    case IFD_VIP_FREE_REQ_RESP:
       return IFSV_EDU_IFD_VIP_FREE_REQ_RESP_OFFSET;
    case IFSV_VIP_ERROR:
       return IFSV_EDU_VIP_ERROR_OFFSET;
    case IFND_VIP_MARK_VIPD_STALE:
       return IFSV_EDU_VIP_COMMON_EVENT_OFFSET;
    case IFA_GET_IP_FROM_STALE_ENTRY_REQ:
       return IFSV_EDU_VIP_COMMON_EVENT_OFFSET;
    case IFND_GET_IP_FROM_STALE_ENTRY_REQ:
       return IFSV_EDU_VIP_COMMON_EVENT_OFFSET;
    case IFD_IP_FROM_STALE_ENTRY_RESP:
       return IFSV_EDU_VIP_IP_FROM_STALE_ENTRY_OFFSET;
    case  IFND_IP_FROM_STALE_ENTRY_RESP:
       return IFSV_EDU_VIP_IP_FROM_STALE_ENTRY_OFFSET;

#endif


    default:
        return EDU_FAIL;
    }
}

/****************************************************************************
 * Name          : ifsv_edp_ifsv_evt
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_EVT structures.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifsv_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    IFSV_EVT    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    test_ifsv_evt_rules[ ] = {
         {EDU_START, ifsv_edp_ifsv_evt, 0, 0, 0, sizeof(IFSV_EVT), 0, NULL},
         {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT*)0)->type, 0, NULL},
         {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT*)0)->error, 0, NULL},
         {EDU_TEST, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_EVT*)0)->type, 0, ifsv_evt_test_type_fnc},

            /* For IFD_EVT_INTF_CREATE */
         {EDU_EXEC, ifsv_edp_create_info, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.ifd_evt.info.intf_create, 0, NULL},

            /* For IFD_EVT_INTF_DESTROY */
         {EDU_EXEC, ifsv_edp_destroy_info, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.ifd_evt.info.intf_destroy, 0, NULL},

            /* For IFD_EVT_INIT_DONE */
         {EDU_EXEC, ifsv_edp_init_done_info, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.ifd_evt.info.init_done, 0, NULL},

            /* For IFD_EVT_IFINDEX_REQ */
         {EDU_EXEC, ifsv_edp_spt_map_info, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifd_evt.info.spt_map, 0, NULL},

           /* For IFD_EVT_IFINDEX_CLEANUP */
         {EDU_EXEC, ifsv_edp_spt_map_info, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifd_evt.info.spt_map, 0, NULL},

           /* For IFSV_EVT_AGING_TMR */
         {EDU_EXEC, ifsv_edp_age_tmr_info, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifd_evt.info.tmr_exp, 0, NULL},

           /* For IFD_EVT_INTF_REC_SYNC */
         {EDU_EXEC, ifsv_edp_rec_sync_info, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifd_evt.info.rec_sync, 0, NULL},

         /* For IFD_EVT_SVCD_UPD */
         {EDU_EXEC, ifsv_edp_svcd_upd, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifd_evt.info.svcd, 0, NULL},

            /* For IFND_EVT_INTF_CREATE */
         {EDU_EXEC, ifsv_edp_create_info, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.ifnd_evt.info.intf_create, 0, NULL},

            /* For IFND_EVT_INTF_DESTROY */
         {EDU_EXEC, ifsv_edp_destroy_info, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.ifnd_evt.info.intf_destroy, 0, NULL},

           /* For IFND_EVT_INIT_DONE */
        {EDU_EXEC, ifsv_edp_init_done_info, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.ifnd_evt.info.init_done, 0, NULL},

            /* For IFND_EVT_INTF_INFO_GET */
         {EDU_EXEC, ifsv_edp_intf_get_info, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.ifnd_evt.info.if_get, 0, NULL},

           /* For IFND_EVT_IDIM_STATS_RESP */
         {EDU_EXEC, ifsv_edp_stats_get_info, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.ifnd_evt.info.stats_info, 0, NULL},

            /* For IFSV_EVT_TMR */
         {EDU_EXEC, ifsv_edp_age_tmr_info, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifnd_evt.info.tmr_exp, 0, NULL},

         /* For IFND_EVT_SVCD_UPD_FROM_IFA */
         {EDU_EXEC, ifsv_edp_svcd_upd, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifnd_evt.info.svcd, 0, NULL},

         /* For IFND_EVT_SVCD_UPD_FROM_IFD */
         {EDU_EXEC, ifsv_edp_svcd_upd, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifnd_evt.info.svcd, 0, NULL},

         /* For IFND_EVT_SVCD_GET */
         {EDU_EXEC, ifsv_edp_svcd_get, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifnd_evt.info.svcd_get, 0, NULL},

            /* For IFND_EVT_IFINDEX_RESP */
         {EDU_EXEC, ifsv_edp_spt_map_info, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.ifnd_evt.info.spt_map, 0, NULL},

           /* For IFA_EVT_INTF_CREATE & IFA_EVT_INTF_UPDATE*/
         {EDU_EXEC, ifsv_edp_ifa_intf_create_info, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifa_evt, 0, NULL},

           /* For IFA_EVT_INTF_DESTROY */
         {EDU_EXEC, ifsv_edp_ifa_intf_destroy_info, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifa_evt, 0, NULL},

           /* For IFA_EVT_IFINFO_GET_RSP */
         {EDU_EXEC, ifsv_edp_ifa_ifget_rsp_info, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifa_evt, 0, NULL},

          /* For IFA_EVT_INTF_CREATE_RSP */
         {EDU_EXEC, ifsv_edp_ifa_intf_create_rsp_info, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifa_evt, 0, NULL},

         /* For IFA_EVT_SVCD_GET_RSP */
         {EDU_EXEC, ifsv_edp_svcd_get, 0, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ifa_evt.info.svcd_rsp, 0, NULL},

#if(NCS_IFSV_IPXS == 1)
           /* For IFSV_IPXS_EVT */
         {EDU_EXEC, ifsv_edp_ifsv_ipxs_info, EDQ_POINTER, 0, EDU_EXIT,
           (long)&((IFSV_EVT*)0)->info.ipxs_evt, 0, NULL},
#endif

#if (NCS_VIP == 1)
#if 0
            /* For IFA_VIPD_INFO_ADD_REQ */
        {EDU_EXEC, ifsv_edp_ifsv_vip_hdl, 0, 0, EDU_EXIT,
            (uns32)&((IFSV_EVT*)0)->info.vip_evt.info.ifaVipAdd.handle, 0, NULL},
#endif

            /* For IFA_VIPD_INFO_ADD_REQ */
        {EDU_EXEC, ifsv_edp_ifa_vip_add, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.ifaVipAdd, 0, NULL},

            /* For IFND_VIPD_INFO_ADD_REQ_RESP */
        {EDU_EXEC, ifsv_edp_ifnd_vip_add, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.ifndVipAdd, 0, NULL},

            /* For IFA_VIP_FREE_REQ */
        {EDU_EXEC, ifsv_edp_ifa_vip_free, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.ifaVipFree, 0, NULL},

            /* For IFND_VIP_FREE_REQ */
        {EDU_EXEC, ifsv_edp_ifnd_vip_free, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.ifndVipFree, 0, NULL},

            /* For IFND_VIP_INFO_ADD_RESP */
        {EDU_EXEC, ifsv_edp_ifnd_info_add_resp, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.ifndVipAddResp, 0, NULL},

           /*  For IFD_VIP_INFO_ADD_RESP */
        {EDU_EXEC, ifsv_edp_ifd_info_add_resp, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.ifdVipAddResp, 0, NULL},

            /* For IFND_VIP_FREE_RESP */
        {EDU_EXEC, ifsv_edp_ifnd_free_resp, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.ifndVipFreeResp, 0, NULL},

            /* For IFD_VIP_FREE_RESP */
        {EDU_EXEC, ifsv_edp_ifd_free_resp, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.ifndVipFreeResp, 0, NULL},

           /* For IFSV_VIP_ERROR_EVT */
        {EDU_EXEC, ifsv_edp_vip_err_evt, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.errEvt, 0, NULL},

           /* For VIP_COMMON_EVT */
        {EDU_EXEC, ifsv_edp_vip_common_event, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.vipCommonEvt, 0, NULL},

           /* For VIP_IP_FROM_STALE_ENTRY_EVT */
        {EDU_EXEC, ifsv_edp_ip_from_stale_entry, 0, 0, EDU_EXIT,
            (long)&((IFSV_EVT*)0)->info.vip_evt.info.staleIp, 0, NULL},

#endif


         {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, test_ifsv_evt_rules, struct_ptr, ptr_data_len,
        buf_env, op, o_err);
    return rc;
}

/*****************************************************************************
 *                       MBCSV_BLOCK starts here...
 *     It has all the EDU rutines for IfD 2N redundancy implementation.
 *
 *****************************************************************************/

/****************************************************************************
 * Name          : ifd_a2s_evt_type_test_fnc
 *
 * Description   : This is the function offsets for encoding/decoding
 *                 IFD_A2S_MSG Data structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
int ifd_a2s_evt_type_test_fnc (NCSCONTEXT arg)
{
  IFD_A2S_EVT_TYPE    evt;

  enum
  {
       IFSV_EDU_IFD_A2S_IFINDEX_SPT_MAP_OFFSET = 1,
       IFSV_EDU_IFD_A2S_INTF_DATA_OFFSET,
       IFSV_EDU_IFD_A2S_SVC_DEST_UPD_OFFSET,
       IFSV_EDU_IFD_A2S_IFND_UP_DOWN_OFFSET,
       IFSV_EDU_IFD_A2S_IFINDEX_UPD_OFFSET,
#if(NCS_IFSV_IPXS == 1)
       IFSV_EDU_IFD_A2S_IPXS_INTF_INFO_OFFSET,
#endif
#if(NCS_VIP == 1)
       IFSV_EDU_IFD_A2S_VIP_REC_INFO_OFFSET,
#endif
  };


    if(arg == NULL)
        return EDU_FAIL;

    evt = *(IFD_A2S_EVT_TYPE *)arg;

    switch(evt)
    {
      case IFD_A2S_EVT_IFINDEX_SPT_MAP:
          return IFSV_EDU_IFD_A2S_IFINDEX_SPT_MAP_OFFSET;

      case IFD_A2S_EVT_INTF_DATA:
          return IFSV_EDU_IFD_A2S_INTF_DATA_OFFSET;

      case IFD_A2S_EVT_SVC_DEST_UPD:
          return IFSV_EDU_IFD_A2S_SVC_DEST_UPD_OFFSET;

      case IFD_A2S_EVT_IFND_UP_DOWN:
          return IFSV_EDU_IFD_A2S_IFND_UP_DOWN_OFFSET;

      case IFD_A2S_EVT_IFINDEX_UPD:
          return IFSV_EDU_IFD_A2S_IFINDEX_UPD_OFFSET;

#if(NCS_IFSV_IPXS == 1)
      case IFD_A2S_EVT_IPXS_INTF_INFO:
          return IFSV_EDU_IFD_A2S_IPXS_INTF_INFO_OFFSET;
#endif

#if(NCS_VIP == 1)
      case IFD_A2S_EVT_VIP_REC_INFO:
          return IFSV_EDU_IFD_A2S_VIP_REC_INFO_OFFSET;
#endif

      default:
        return EDU_FAIL;
    } /* switch(evt) */
} /* function ifd_a2s_evt_type_test_fnc() ends here. */

/****************************************************************************
 * Name          : ifsv_edp_ifd_a2s_ifindex_spt_map_evt
 *
 * Description   : This is the function which is used to encode decode
 *                 IFD_A2S_IFINDEX_SPT_MAP_EVT structures.
 *
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifd_a2s_ifindex_spt_map_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    IFD_A2S_IFINDEX_SPT_MAP_EVT  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_a2s_ifindex_spt_map_rules[ ] = {
        {EDU_START, ifsv_edp_ifd_a2s_ifindex_spt_map_evt, 0, 0, 0,
         sizeof(IFD_A2S_IFINDEX_SPT_MAP_EVT), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((IFD_A2S_IFINDEX_SPT_MAP_EVT*)0)->type, 0, NULL},
        {EDU_EXEC, ifsv_edp_spt_map, 0, 0, 0,
         (long)&((IFD_A2S_IFINDEX_SPT_MAP_EVT*)0)->info, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFD_A2S_IFINDEX_SPT_MAP_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFD_A2S_IFINDEX_SPT_MAP_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFD_A2S_IFINDEX_SPT_MAP_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_a2s_ifindex_spt_map_rules,
                             struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;

}/* function ifsv_edp_ifd_a2s_ifindex_spt_map_evt() ends here. */

/****************************************************************************
 * Name          :ifsv_edp_ifd_a2s_intf_data_evt
 *
 * Description   : This is the function which is used to encode decode
 *                 IFD_A2S_INTF_DATA_EVT structures.
 *
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifd_a2s_intf_data_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    IFD_A2S_INTF_DATA_EVT  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_a2s_intf_data_rules[ ] = {
        {EDU_START, ifsv_edp_ifd_a2s_intf_data_evt, 0, 0, 0,
         sizeof(IFD_A2S_INTF_DATA_EVT), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((IFD_A2S_INTF_DATA_EVT*)0)->type, 0, NULL},
        {EDU_EXEC, ifsv_edp_intf_data, 0, 0, 0,
         (long)&((IFD_A2S_INTF_DATA_EVT*)0)->info, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFD_A2S_INTF_DATA_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFD_A2S_INTF_DATA_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFD_A2S_INTF_DATA_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_a2s_intf_data_rules,
                             struct_ptr, ptr_data_len, buf_env, op, o_err);

    return rc;
}/* function ifsv_edp_ifd_a2s_intf_data_evt() ends here.  */

/****************************************************************************
 * Name          : ifsv_edp_ifd_a2s_svc_dest_upd_evt
 *
 * Description   : This is the function which is used to encode decode
 *                  structures.
 *
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifd_a2s_svc_dest_upd_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    IFD_A2S_SVC_DEST_UPD_EVT  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_a2s_svc_dest_upd_rules[ ] = {
        {EDU_START, ifsv_edp_ifd_a2s_svc_dest_upd_evt, 0, 0, 0,
         sizeof(IFD_A2S_SVC_DEST_UPD_EVT), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((IFD_A2S_SVC_DEST_UPD_EVT*)0)->type, 0, NULL},
        {EDU_EXEC, ifsv_edp_svcd_upd, 0, 0, 0,
         (long)&((IFD_A2S_SVC_DEST_UPD_EVT*)0)->info, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFD_A2S_SVC_DEST_UPD_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFD_A2S_SVC_DEST_UPD_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFD_A2S_SVC_DEST_UPD_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_a2s_svc_dest_upd_rules,
                             struct_ptr, ptr_data_len, buf_env, op, o_err);

    return rc;

}/* function ifsv_edp_ifd_a2s_svc_dest_upd_evt() ends here.  */

/****************************************************************************
 * Name          : ifsv_edp_ifd_a2s_ifnd_up_down_evt
 *
 * Description   : This is the function which is used to encode decode
 *                 IFD_A2S_IFND_UP_DOWN_EVT structures.
 *
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifd_a2s_ifnd_up_down_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    IFD_A2S_IFND_UP_DOWN_EVT  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_a2s_ifnd_down_rules[ ] = {
        {EDU_START, ifsv_edp_ifd_a2s_ifnd_up_down_evt, 0, 0, 0,
         sizeof(IFD_A2S_IFND_UP_DOWN_EVT), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((IFD_A2S_IFND_UP_DOWN_EVT*)0)->type, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0,
         (long)&((IFD_A2S_IFND_UP_DOWN_EVT*)0)->mds_dest, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFD_A2S_IFND_UP_DOWN_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFD_A2S_IFND_UP_DOWN_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFD_A2S_IFND_UP_DOWN_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_a2s_ifnd_down_rules,
                             struct_ptr, ptr_data_len, buf_env, op, o_err);

    return rc;

}/* function ifsv_edp_ifd_a2s_ifnd_up_down_evt() ends here.  */

/****************************************************************************
 * Name          : ifsv_edp_ifd_a2s_ifindex_upd_evt
 *
 * Description   : This is the function which is used to encode decode
 *                 IFD_A2S_IFINDEX_UPD_EVT structures.
 *
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifd_a2s_ifindex_upd_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    IFD_A2S_IFINDEX_UPD_EVT  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_a2s_ifindex_upd_rules[ ] = {
        {EDU_START, ifsv_edp_ifd_a2s_ifindex_upd_evt, 0, 0, 0,
         sizeof(IFD_A2S_IFINDEX_UPD_EVT), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((IFD_A2S_IFINDEX_UPD_EVT*)0)->type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((IFD_A2S_IFINDEX_UPD_EVT*)0)->ifindex, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFD_A2S_IFINDEX_UPD_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFD_A2S_IFINDEX_UPD_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFD_A2S_IFINDEX_UPD_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_a2s_ifindex_upd_rules,
                             struct_ptr, ptr_data_len, buf_env, op, o_err);

    return rc;

}/* function ifsv_edp_ifd_a2s_ifindex_upd_evt() ends here.  */

/****************************************************************************
 * Name          : ifsv_edp_ifd_a2s_iaps_evt
 *
 * Description   : This is the function which is used to encode decode
 *                 IFAP_INFO_LIST_A2S structures.
 *
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifd_a2s_iaps_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    IFAP_INFO_LIST_A2S  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_a2s_iaps_rules[ ] = {
        {EDU_START, ifsv_edp_ifd_a2s_iaps_evt, 0, 0, 0,
         sizeof(IFAP_INFO_LIST_A2S), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((IFAP_INFO_LIST_A2S*)0)->max_ifindex, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((IFAP_INFO_LIST_A2S*)0)->num_free_ifindex, 0, NULL},

        {EDU_EXEC, ncs_edp_uns32, EDQ_VAR_LEN_DATA,
         ncs_edp_uns32, 0, (long)&((IFAP_INFO_LIST_A2S *)0)->free_list,
        (long)&((IFAP_INFO_LIST_A2S *)0)->num_free_ifindex, NULL},

        {EDU_EXEC_EXT, NULL, NCS_SERVICE_ID_IFSV /* Svc-ID */, NULL, 0,
         NCS_IFSV_SVC_SUB_ID_NCS_IAPS /* Sub-ID */, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFAP_INFO_LIST_A2S *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFAP_INFO_LIST_A2S **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFAP_INFO_LIST_A2S));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_a2s_iaps_rules,
                             struct_ptr, ptr_data_len, buf_env, op, o_err);

    return rc;

}/* function ifsv_edp_ifd_a2s_iaps_evt() ends here.  */

/****************************************************************************
 * Name          : ifsv_edp_ifd_a2s_msg_evt
 *
 * Description   : This is the function which is used to encode decode
 *                 MBCSV sync update structures.
 *
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifd_a2s_msg_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32       rc = NCSCC_RC_SUCCESS;
    IFD_A2S_MSG    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifd_a2s_msg_evt_rules[ ] = {
         {EDU_START, ifsv_edp_ifd_a2s_msg_evt, 0, 0, 0, sizeof(IFD_A2S_MSG), 0,
                    NULL},
         {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFD_A2S_MSG*)0)->type, 0,
                    NULL},
         {EDU_TEST, ncs_edp_uns32, 0, 0, 0, (long)&((IFD_A2S_MSG*)0)->type, 0,
                    ifd_a2s_evt_type_test_fnc},

         /* For IFD_A2S_EVT_IFINDEX_SPT_MAP */
         {EDU_EXEC, ifsv_edp_ifd_a2s_ifindex_spt_map_evt, 0, 0, EDU_EXIT,
            (long)&((IFD_A2S_MSG*)0)->info.ifd_a2s_ifindex_spt_map_evt, 0,
            NULL},

            /* For IFD_A2S_EVT_INTF_DATA */
         {EDU_EXEC, ifsv_edp_ifd_a2s_intf_data_evt, 0, 0, EDU_EXIT,
            (long)&((IFD_A2S_MSG*)0)->info.ifd_a2s_intf_data_evt, 0, NULL},

            /* For IFD_A2S_EVT_SVC_DEST_UPD */
         {EDU_EXEC, ifsv_edp_ifd_a2s_svc_dest_upd_evt, 0, 0, EDU_EXIT,
            (long)&((IFD_A2S_MSG*)0)->info.ifd_a2s_svc_dest_upd_evt, 0, NULL},

           /* For IFD_A2S_EVT_IFND_UP_DOWN */
         {EDU_EXEC, ifsv_edp_ifd_a2s_ifnd_up_down_evt, 0, 0, EDU_EXIT,
           (long)&((IFD_A2S_MSG*)0)->info.ifd_a2s_ifnd_up_down_evt, 0, NULL},

           /* For IFD_A2S_EVT_IFINDEX_UPD */
         {EDU_EXEC, ifsv_edp_ifd_a2s_ifindex_upd_evt, 0, 0, EDU_EXIT,
           (long)&((IFD_A2S_MSG*)0)->info.ifd_a2s_ifindex_upd_evt, 0, NULL},

#if(NCS_IFSV_IPXS == 1)
            /* For IFD_A2S_EVT_IPXS_INTF_INFO */
         {EDU_EXEC, ifsv_edp_ifd_a2s_ipxs_intf_info_evt, 0, 0, EDU_EXIT,
           (long)&((IFD_A2S_MSG*)0)->info.ifd_a2s_ipxs_intf_info_evt, 0, NULL},
#endif
#if(NCS_VIP == 1)
         {EDU_EXEC, ifsv_edp_ifd_a2s_vip_rec_info_evt, 0, 0, EDU_EXIT,
           (long)&((IFD_A2S_MSG*)0)->info.ifd_a2s_vip_rec_info_evt, 0, NULL},
#endif
         {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFD_A2S_MSG *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFD_A2S_MSG **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFD_A2S_MSG));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifd_a2s_msg_evt_rules,
                             struct_ptr, ptr_data_len,
        buf_env, op, o_err);
    return rc;
}/* function ifsv_edp_ifd_a2s_msg_evt() ends here. */

/*****************************************************************************
 *          MBCSV_BLOCK ends here...
 *****************************************************************************/

/****************************************************************************
 * Name          : ifsv_mds_normal_send
 *
 * Description   : This is the function which is used to send the message
 *                 using MDS.
 *
 * Arguments     : mds_hdl  - MDS handle
 *                 from_svc - From Serivce ID.
 *                 evt      - Event to be sent.
 *                 dest     - MDS destination to be sent to.
 *                 to_svc   - To Service ID.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifsv_mds_normal_send (MDS_HDL mds_hdl,
                     NCSMDS_SVC_ID from_svc,
                     NCSCONTEXT evt, MDS_DEST dest,
                     NCSMDS_SVC_ID to_svc)
{

   NCSMDS_INFO   info;
   uns32         res;

   m_NCS_MEMSET(&info, 0, sizeof(info));

   info.i_mds_hdl  = mds_hdl;
   info.i_op       = MDS_SEND;
   info.i_svc_id   = from_svc;

   info.info.svc_send.i_msg = evt;
   info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
   info.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
   info.info.svc_send.i_to_svc   = to_svc;

   m_NCS_MEMSET(&info.info.svc_send.info.snd.i_to_dest, 0, sizeof(MDS_DEST));
   info.info.svc_send.info.snd.i_to_dest = dest;

   res = ncsmds_api(&info);
   return(res);
}

/****************************************************************************
 * Name          : ifsv_mds_scoped_send
 *
 * Description   : This is the function which is used to send the message
 *                 using MDS only with the scope which we want to.
 *
 * Arguments     : mds_hdl  - MDS handle
 *                 scope    - Scope to be sent
 *                            NCSMDS_SCOPE_INTRANODE/NCSMDS_SCOPE_INTRAPCON
 *                 from_svc - From Serivce ID.
 *                 evt      - Event to be sent.
 *                 to_svc   - To Service ID.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifsv_mds_scoped_send (MDS_HDL mds_hdl,
                      NCSMDS_SCOPE_TYPE scope,
                      NCSMDS_SVC_ID from_svc,
                      NCSCONTEXT evt,
                      NCSMDS_SVC_ID to_svc)
{

   NCSMDS_INFO   info;
   uns32         res;

   m_NCS_MEMSET(&info, 0, sizeof(info));

   info.i_mds_hdl  = mds_hdl;
   info.i_op       = MDS_SEND;
   info.i_svc_id   = from_svc;

   info.info.svc_send.i_msg = evt;
   info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
   info.info.svc_send.i_sendtype = MDS_SENDTYPE_BCAST;
   info.info.svc_send.i_to_svc   = to_svc;

   /* There is some message context in this broadcast structure - TBD */
   info.info.svc_send.info.bcast.i_bcast_scope = scope;

   res = ncsmds_api(&info);
   return(res);
}

/****************************************************************************
 * Name          : ifsv_mds_bcast_send
 *
 * Description   : This is the function which is used to send the message
 *                 using MDS broadcast.
 *
 * Arguments     : mds_hdl  - MDS handle
 *                 from_svc - From Serivce ID.
 *                 evt      - Event to be sent.
 *                 to_svc   - To Service ID.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifsv_mds_bcast_send (MDS_HDL mds_hdl,
                      NCSMDS_SVC_ID from_svc,
                      NCSCONTEXT evt,
                      NCSMDS_SVC_ID to_svc)
{

   NCSMDS_INFO   info;
   uns32         res;

   m_NCS_MEMSET(&info, 0, sizeof(info));

   info.i_mds_hdl  = mds_hdl;
   info.i_op       = MDS_SEND;
   info.i_svc_id   = from_svc;

   info.info.svc_send.i_msg = evt;
   info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
   info.info.svc_send.i_sendtype = MDS_SENDTYPE_BCAST;
   info.info.svc_send.i_to_svc   = to_svc;

   /* There is some message context in this broadcast structure - TBD */
   info.info.svc_send.info.bcast.i_bcast_scope = NCSMDS_SCOPE_NONE;

   res = ncsmds_api(&info);
   return(res);
}


/****************************************************************************
  Name          : ifsv_mds_msg_sync_send

  Description   : This routine sends the Sinc requests from IFA/IFND/IFD

  Arguments     : cb  - ptr to the IFSV CB
                  i_evt - ptr to the MQSV message
                  o_evt - ptr to the MQSV message returned
                  timeout - timeout value in 10 ms

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         : None.
******************************************************************************/
uns32 ifsv_mds_msg_sync_send (MDS_HDL mds_hdl,
                              uns32       from_svc,
                              uns32       to_svc,
                              MDS_DEST    to_dest,
                              IFSV_EVT    *i_evt,
                              IFSV_EVT    **o_evt,
                              uns32       timeout)
{


   NCSMDS_INFO                mds_info;
   uns32                      rc;

   if(!i_evt)
      return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(&mds_info, 0, sizeof(NCSMDS_INFO));
   mds_info.i_mds_hdl = mds_hdl;
   mds_info.i_svc_id = from_svc;
   mds_info.i_op = MDS_SEND;

   /* fill the send structure */
   mds_info.info.svc_send.i_msg = (NCSCONTEXT)i_evt;
   mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
   mds_info.info.svc_send.i_to_svc = to_svc;
   mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SNDRSP;

   /* fill the send rsp strcuture */
   mds_info.info.svc_send.info.sndrsp.i_time_to_wait = timeout; /* timeto wait in 10ms */
   mds_info.info.svc_send.info.sndrsp.i_to_dest = to_dest;

   /* send the message */
   rc = ncsmds_api(&mds_info);
   if ( rc == NCSCC_RC_SUCCESS)
      *o_evt = mds_info.info.svc_send.info.sndrsp.o_rsp;

   return rc;
}

/****************************************************************************
 * Name          : ifsv_mds_send_rsp
 *
 * Description   : Send the Response to Sync Requests
 *
 * Arguments     :
 *
 * Return Values :
 *
 * Notes         :
 *****************************************************************************/

uns32 ifsv_mds_send_rsp(MDS_HDL mds_hdl,
                        NCSMDS_SVC_ID  from_svc,
                        IFSV_SEND_INFO *s_info,
                        IFSV_EVT       *evt)
{
   NCSMDS_INFO                mds_info;
   uns32                      rc = NCSCC_RC_SUCCESS;

   if(s_info->stype != MDS_SENDTYPE_RSP)
      return rc;

   m_NCS_OS_MEMSET(&mds_info, 0, sizeof(NCSMDS_INFO));
   mds_info.i_mds_hdl = mds_hdl;
   mds_info.i_svc_id = from_svc;
   mds_info.i_op = MDS_SEND;

   /* fill the send structure */
   mds_info.info.svc_send.i_msg = (NCSCONTEXT)evt;
   mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;

   mds_info.info.svc_send.i_to_svc = s_info->to_svc;
   mds_info.info.svc_send.i_sendtype = s_info->stype;
   mds_info.info.svc_send.info.rsp.i_msg_ctxt = s_info->ctxt;
   mds_info.info.svc_send.info.rsp.i_sender_dest = s_info->dest;

   /* send the message */
   rc = ncsmds_api(&mds_info);

   return rc;
}

/****************************************************************************
 * Name          : ifsv_ifa_app_if_info_indicate
 *
 * Description   : This is the function to inform applications (IFA) about
 *                 the Interface Changes
 *
 *
 * Arguments     : actual_data - The data to be indicated to the application.
 *               : intf_evt    - Record event to be sent (Add/Modify/Delete).
 *                 attr        - Attributes which has been changed.
 *                 cb          - IfSv Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifsv_ifa_app_if_info_indicate(IFSV_INTF_DATA *actual_data,
                            IFSV_INTF_REC_EVT intf_evt, uns32 attr,
                            IFSV_CB *cb)
{
   IFSV_EVT *evt = NULL;
   uns32    rc = NCSCC_RC_SUCCESS;

   if (cb->comp_type == IFSV_COMP_IFND)
   {
      /* Allocate the event data structure */
      evt = m_MMGR_ALLOC_IFSV_EVT;
      if (evt == IFSV_NULL)
         return NCSCC_RC_FAILURE;

      m_NCS_MEMSET(evt, 0, sizeof(IFSV_EVT));

      evt->type = intf_evt;

      switch(intf_evt)
      {
      case IFSV_INTF_REC_ADD:
         evt->type = IFA_EVT_INTF_CREATE;
         evt->info.ifa_evt.subscr_scope = actual_data->spt_info.subscr_scope;
         evt->info.ifa_evt.info.if_add_upd.if_index = actual_data->if_index;
         evt->info.ifa_evt.info.if_add_upd.info_type = NCS_IFSV_IF_INFO;
         rc = ifsv_intf_info_cpy(&actual_data->if_info,
                &evt->info.ifa_evt.info.if_add_upd.if_info, IFSV_MALLOC_FOR_MDS_SEND);
         if(rc != NCSCC_RC_SUCCESS)
          {
            m_IFSV_LOG_STR_2_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_FUNC_RET_FAIL,"ifsv_intf_info_cpy() returned failure"," ");
            goto free_mem;
          }
         evt->info.ifa_evt.info.if_add_upd.if_info = actual_data->if_info;
         evt->info.ifa_evt.info.if_add_upd.spt_info = actual_data->spt_info;
         break;
      case IFSV_INTF_REC_MODIFY:
         evt->type = IFA_EVT_INTF_UPDATE;
         evt->info.ifa_evt.subscr_scope = actual_data->spt_info.subscr_scope;
         evt->info.ifa_evt.info.if_add_upd.if_index = actual_data->if_index;
         evt->info.ifa_evt.info.if_add_upd.info_type = NCS_IFSV_IF_INFO;
         rc = ifsv_intf_info_cpy(&actual_data->if_info,
                &evt->info.ifa_evt.info.if_add_upd.if_info, IFSV_MALLOC_FOR_MDS_SEND);
         if(rc != NCSCC_RC_SUCCESS)
          {
            m_IFSV_LOG_STR_2_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_FUNC_RET_FAIL,"ifsv_intf_info_cpy() returned failure"," ");
            goto free_mem;
          }
         evt->info.ifa_evt.info.if_add_upd.if_info.if_am = attr;
         evt->info.ifa_evt.info.if_add_upd.spt_info = actual_data->spt_info;
         break;
      case IFSV_INTF_REC_DEL:
      case IFSV_INTF_REC_MARKED_DEL:
         evt->type = IFA_EVT_INTF_DESTROY;
         evt->info.ifa_evt.info.if_del = actual_data->if_index;
         evt->info.ifa_evt.subscr_scope = actual_data->spt_info.subscr_scope;
         break;
      default:
         rc = NCSCC_RC_FAILURE;
         break;
      }

      /* Broadcast the event to all the IFAs in on the same node */
      rc = ifsv_mds_scoped_send(cb->my_mds_hdl, NCSMDS_SCOPE_INTRANODE,
         NCSMDS_SVC_ID_IFND, evt, NCSMDS_SVC_ID_IFA);

      if(rc != NCSCC_RC_SUCCESS)
      {
         m_IFSV_LOG_STR_2_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),IFSV_LOG_FUNC_RET_FAIL,"ifsv_mds_scoped_send() returned failure"," ");
         ifsv_intf_info_free(&evt->info.ifa_evt.info.if_add_upd.if_info,
                                                    IFSV_MALLOC_FOR_MDS_SEND);
      }

free_mem:
      m_MMGR_FREE_IFSV_EVT(evt);

#if(NCS_IFSV_IPXS == 1)
      /* Inform the IPXS applications */
      rc = ipxs_ifa_app_if_info_indicate(actual_data, intf_evt, attr, cb);

#endif
   }
   return (rc);
}


/* Function to free the internal pointers of the event structure */
void ifsv_evt_ptrs_free(IFSV_EVT *evt, IFSV_EVT_TYPE type)
{
   switch(type)
   {
   case IFA_EVT_INTF_CREATE:
   case IFA_EVT_INTF_UPDATE:
      ifsv_intf_info_free(&evt->info.ifa_evt.info.if_add_upd.if_info, IFSV_MALLOC_FOR_MDS_SEND);
      break;
   case IFA_EVT_IFINFO_GET_RSP:
      ifsv_intf_info_free(&evt->info.ifa_evt.info.ifget_rsp.if_rec.if_info, IFSV_MALLOC_FOR_MDS_SEND);
      break;
   default:
      break;
   }

   return;
}

#if (NCS_VIP == 1)
/****************************************************************************
 * Name          : ifsv_edp_ifsv_vip_hdl
 *
 * Description   : This is the function which is used to encode decode
 *                 NCS_IFSV_VIP_INT_HDL structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/


uns32 ifsv_edp_ifsv_vip_hdl(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_VIP_INT_HDL   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_vip_hdl_info_rules[ ] = {
        {EDU_START, ifsv_edp_ifsv_vip_hdl, 0, 0, 0, sizeof(NCS_IFSV_VIP_INT_HDL), 0, NULL},
           {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_VIP_INT_HDL*)0)->vipApplName, m_NCS_IFSV_VIP_APPL_NAME, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_VIP_INT_HDL*)0)->poolHdl ,0, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_VIP_INT_HDL*)0)->ipPoolType ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_VIP_INT_HDL *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_VIP_INT_HDL **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_VIP_INT_HDL));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_vip_hdl_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ifa_vip_add
 *
 * Description   : This is the function which is used to encode decode
 *                 IFA_VIPD_INFO_ADD structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifa_vip_add(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFA_VIPD_INFO_ADD   *struct_ptr = NULL, **d_ptr = NULL;
    /*write a function for the "ifsv_edp_ifsv_vip_hdl"   TBComplemented */
    EDU_INST_SET    ifsv_ifa_vip_add_info_rules[ ] = {
        {EDU_START, ifsv_edp_ifa_vip_add, 0, 0, 0, sizeof(IFA_VIPD_INFO_ADD), 0, NULL},
           {EDU_EXEC, ifsv_edp_ifsv_vip_hdl, 0, 0, 0, (long)&((IFA_VIPD_INFO_ADD*)0)->handle ,0, NULL},
           {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((IFA_VIPD_INFO_ADD*)0)->intfName, m_NCS_IFSV_VIP_INTF_NAME, NULL},
           {EDU_EXEC, ncs_edp_ippfx, 0, 0, 0, (long)&((IFA_VIPD_INFO_ADD*)0)->ipAddr ,0, NULL},
#if (VIP_HALS_SUPPORT == 1)
           {EDU_EXEC,ncs_edp_uns32, 0, 0, 0,(long)&((IFA_VIPD_INFO_ADD*)0)->infraFlag, 0, NULL},
#endif
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFA_VIPD_INFO_ADD *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFA_VIPD_INFO_ADD **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFA_VIPD_INFO_ADD));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_ifa_vip_add_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ifnd_vip_add
 *
 * Description   : This is the function which is used to encode decode
 *                 IFND_VIPD_INFO_ADD structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifsv_edp_ifnd_vip_add(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFND_VIPD_INFO_ADD   *struct_ptr = NULL, **d_ptr = NULL;
    /*write a function for the "ifsv_edp_ifsv_vip_hdl"   TBComplemented */
    EDU_INST_SET    ifsv_ifnd_vip_add_info_rules[ ] = {
        {EDU_START, ifsv_edp_ifnd_vip_add, 0, 0, 0, sizeof(IFND_VIPD_INFO_ADD), 0, NULL},
           {EDU_EXEC, ifsv_edp_ifsv_vip_hdl, 0, 0, 0, (long)&((IFND_VIPD_INFO_ADD*)0)->handle ,0, NULL},
           {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((IFND_VIPD_INFO_ADD*)0)->intfName, m_NCS_IFSV_VIP_INTF_NAME, NULL},
           {EDU_EXEC, ncs_edp_ippfx, 0, 0, 0, (long)&((IFND_VIPD_INFO_ADD*)0)->ipAddr ,0, NULL},
#if (VIP_HALS_SUPPORT == 1)
           {EDU_EXEC,ncs_edp_uns32, 0, 0, 0,(long)&((IFND_VIPD_INFO_ADD*)0)->infraFlag, 0, NULL},
#endif
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFND_VIPD_INFO_ADD *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFND_VIPD_INFO_ADD **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFND_VIPD_INFO_ADD));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_ifnd_vip_add_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ifa_vip_free
 *
 * Description   : This is the function which is used to encode decode
 *                 IFA_VIP_FREE structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifsv_edp_ifa_vip_free(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFA_VIP_FREE   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_ifa_vip_free_rules[ ] = {
        {EDU_START, ifsv_edp_ifa_vip_free, 0, 0, 0, sizeof(IFA_VIP_FREE), 0, NULL},
           {EDU_EXEC, ifsv_edp_ifsv_vip_hdl, 0, 0, 0, (long)&((IFA_VIP_FREE*)0)->handle ,0, NULL},
#if (VIP_HALS_SUPPORT == 1)
           {EDU_EXEC,ncs_edp_uns32, 0, 0, 0,(long)&((IFA_VIP_FREE*)0)->infraFlag, 0, NULL},
#endif
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFA_VIP_FREE *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFA_VIP_FREE **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFA_VIP_FREE));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_ifa_vip_free_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ifnd_vip_free
 *
 * Description   : This is the function which is used to encode decode
 *                 IFA_VIP_FREE structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifsv_edp_ifnd_vip_free(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFND_VIP_FREE   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_ifnd_vip_free_rules[ ] = {
        {EDU_START, ifsv_edp_ifnd_vip_free, 0, 0, 0, sizeof(IFND_VIP_FREE), 0, NULL},
        {EDU_EXEC, ifsv_edp_ifsv_vip_hdl, 0, 0, 0, (long)&((IFND_VIP_FREE*)0)->handle ,0, NULL},
#if (VIP_HALS_SUPPORT == 1)
           {EDU_EXEC,ncs_edp_uns32, 0, 0, 0,(long)&((IFND_VIP_FREE*)0)->infraFlag, 0, NULL},
#endif
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFND_VIP_FREE *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFND_VIP_FREE **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFND_VIP_FREE));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_ifnd_vip_free_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ifnd_info_add_resp
 *
 * Description   : This is the function which is used to encode decode
 *                 IFND_VIPD_INFO_ADD_RESP structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifnd_info_add_resp(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFND_VIPD_INFO_ADD_RESP   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_ifnd_info_add_resp_rules[ ] = {
        {EDU_START, ifsv_edp_ifnd_info_add_resp, 0, 0, 0, sizeof(IFND_VIPD_INFO_ADD_RESP), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFND_VIPD_INFO_ADD_RESP*)0)->err ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFND_VIPD_INFO_ADD_RESP *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFND_VIPD_INFO_ADD_RESP **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFND_VIPD_INFO_ADD_RESP));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_ifnd_info_add_resp_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ifd_info_add_resp
 *
 * Description   : This is the function which is used to encode decode
 *                 IFD_VIPD_INFO_ADD_RESP structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/



uns32 ifsv_edp_ifd_info_add_resp(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFD_VIPD_INFO_ADD_RESP   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_ifd_info_add_resp_rules[ ] = {
        {EDU_START, ifsv_edp_ifd_info_add_resp, 0, 0, 0, sizeof(IFD_VIPD_INFO_ADD_RESP), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFD_VIPD_INFO_ADD_RESP*)0)->err ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFD_VIPD_INFO_ADD_RESP *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFD_VIPD_INFO_ADD_RESP **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFD_VIPD_INFO_ADD_RESP));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_ifd_info_add_resp_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_edp_ifnd_free_resp
 *
 * Description   : This is the function which is used to encode decode
 *                 IFND_VIP_FREE_RESP structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifsv_edp_ifnd_free_resp(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFND_VIP_FREE_RESP   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_ifnd_free_resp_rules[ ] = {
        {EDU_START, ifsv_edp_ifnd_free_resp, 0, 0, 0, sizeof(IFND_VIP_FREE_RESP), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFND_VIP_FREE_RESP*)0)->err ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFND_VIP_FREE_RESP *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFND_VIP_FREE_RESP **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFND_VIP_FREE_RESP));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_ifnd_free_resp_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ifd_free_resp
 *
 * Description   : This is the function which is used to encode decode
 *                 IFD_VIP_FREE_RESP structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifd_free_resp(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFD_VIP_FREE_RESP   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_ifd_free_resp_rules[ ] = {
        {EDU_START, ifsv_edp_ifd_free_resp, 0, 0, 0, sizeof(IFD_VIP_FREE_RESP), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFD_VIP_FREE_RESP*)0)->err ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFD_VIP_FREE_RESP *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFD_VIP_FREE_RESP **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFD_VIP_FREE_RESP));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_ifd_free_resp_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/***************************************************************************
***************************************************************************/
/****************************************************************************
 * Name          : ifsv_edp_vip_common_event
 *
 * Description   : This is the function which is used to encode decode
 *                 VIP_COMMON_EVENT structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_vip_common_event(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    VIP_COMMON_EVENT       *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET       ifsv_vip_common_event_rules[ ] = {
        {EDU_START, ifsv_edp_vip_common_event, 0, 0, 0, sizeof(VIP_COMMON_EVENT), 0, NULL},
        {EDU_EXEC, ifsv_edp_ifsv_vip_hdl, 0, 0, 0, (long)&((VIP_COMMON_EVENT*)0)->handle ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (VIP_COMMON_EVENT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (VIP_COMMON_EVENT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(VIP_COMMON_EVENT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_vip_common_event_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_ip_from_stale_entry
 *
 * Description   : This is the function which is used to encode decode
 *                 VIP_IP_FROM_STALE_ENTRY structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ip_from_stale_entry(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    VIP_IP_FROM_STALE_ENTRY       *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET       ifsv_vip_ip_from_stale_entry_rules[ ] = {
        {EDU_START, ifsv_edp_ip_from_stale_entry, 0, 0, 0, sizeof(VIP_IP_FROM_STALE_ENTRY), 0, NULL},
        {EDU_EXEC, ncs_edp_ippfx, 0, 0, 0, (long)&((VIP_IP_FROM_STALE_ENTRY*)0)->ipAddr ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (VIP_IP_FROM_STALE_ENTRY *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (VIP_IP_FROM_STALE_ENTRY **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(VIP_IP_FROM_STALE_ENTRY));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_vip_ip_from_stale_entry_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/**************************************************************************
***************************************************************************/


/****************************************************************************
 * Name          : ifsv_edp_vip_err_evt
 *
 * Description   : This is the function which is used to encode decode
 *                 IFSV_VIP_ERROR_EVT structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_vip_err_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    IFSV_VIP_ERROR_EVT   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_vip_err_evt_rules[ ] = {
        {EDU_START, ifsv_edp_vip_err_evt, 0, 0, 0, sizeof(IFSV_VIP_ERROR_EVT), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((IFSV_VIP_ERROR_EVT*)0)->err ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFSV_VIP_ERROR_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFSV_VIP_ERROR_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFSV_VIP_ERROR_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_vip_err_evt_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/*
MDS Routines to Incorporate CheckPointing Facility.
*/



/****************************************************************************
 * Name          : ifsv_edp_ifd_
 *
 * Description   : This is the function which is used to encode decode
 *                 IFD_A2S_IFINDEX_UPD_EVT structures.
 *
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifsv_edp_ifd_a2s_vip_rec_info_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    IFD_A2S_VIP_REC_INFO_EVT  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifd_a2s_vip_rec_info_evt_rules[ ] = {
        {EDU_START, ifsv_edp_ifd_a2s_vip_rec_info_evt, 0, 0, 0,
         sizeof(IFD_A2S_VIP_REC_INFO_EVT), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((IFD_A2S_VIP_REC_INFO_EVT*)0)->type, 0, NULL},
        {EDU_EXEC,ifsv_edp_vip_chk_pt_full_rec , 0, 0, 0,
         (long)&((IFD_A2S_VIP_REC_INFO_EVT*)0)->info, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (IFD_A2S_VIP_REC_INFO_EVT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (IFD_A2S_VIP_REC_INFO_EVT **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(IFD_A2S_VIP_REC_INFO_EVT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifd_a2s_vip_rec_info_evt_rules,
                             struct_ptr, ptr_data_len, buf_env, op, o_err);

    return rc;

}/* function ifsv_edp_ifd_a2s_ifindex_upd_evt() ends here.  */

/****************************************************************************
 * Name          : ifsv_edp_vip_ch_kpt_full_rec
 *
 * Description   : This is the function which is used to encode decode
 *                 VIP_REDUNDANCY_RECORD structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifsv_edp_vip_chk_pt_full_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    VIP_REDUNDANCY_RECORD   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_vip_chk_pt_full_rec_info_rules[ ] = {
        {EDU_START, ifsv_edp_vip_chk_pt_full_rec, 0, 0, 0, sizeof(VIP_REDUNDANCY_RECORD), 0, NULL},
           {EDU_EXEC, ifsv_edp_ifsv_vip_hdl, 0, 0, 0, (long)&((VIP_REDUNDANCY_RECORD*)0)->handle ,0, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((VIP_REDUNDANCY_RECORD*)0)->vip_entry_attr ,0, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((VIP_REDUNDANCY_RECORD*)0)->ref_cnt ,0, NULL},

           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((VIP_REDUNDANCY_RECORD*)0)->ip_list_cnt ,0, NULL},
           {EDU_EXEC, ifsv_edp_vip_chk_pt_ip_data, 
                      EDQ_VAR_LEN_DATA, ncs_edp_uns32, 0, 
                      (long)&((VIP_REDUNDANCY_RECORD*)0)->ipInfo,
                      (long)&((VIP_REDUNDANCY_RECORD*)0)->ip_list_cnt, NULL},
           {EDU_EXEC_EXT, NULL, NCS_SERVICE_ID_IFSV /* Svc-ID */, NULL, 0, NCS_IFSV_SVC_SUB_ID_NCS_SVDEST /* Sub-ID */, 0, NULL},
           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((VIP_REDUNDANCY_RECORD*)0)->intf_list_cnt ,0, NULL},
           {EDU_EXEC, ifsv_edp_vip_chk_pt_intf_data, 
                      EDQ_VAR_LEN_DATA, ncs_edp_uns32, 0, 
                      (long)&((VIP_REDUNDANCY_RECORD*)0)->intfInfo ,
                      (long)&((VIP_REDUNDANCY_RECORD*)0)->intf_list_cnt, NULL},
           {EDU_EXEC_EXT, NULL, NCS_SERVICE_ID_IFSV /* Svc-ID */, NULL, 0, NCS_IFSV_SVC_SUB_ID_NCS_SVDEST /* Sub-ID */, 0, NULL},

           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((VIP_REDUNDANCY_RECORD*)0)->alloc_ip_list_cnt ,0, NULL},

           {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((VIP_REDUNDANCY_RECORD*)0)->owner_list_cnt ,0, NULL},
           {EDU_EXEC, ifsv_edp_vip_chk_pt_owner_data, 
                      EDQ_VAR_LEN_DATA, ncs_edp_uns32, 0, 
                      (long)&((VIP_REDUNDANCY_RECORD*)0)->ownerInfo ,
                      (long)&((VIP_REDUNDANCY_RECORD*)0)->owner_list_cnt, NULL},
           {EDU_EXEC_EXT, NULL, NCS_SERVICE_ID_IFSV /* Svc-ID */, NULL, 0, NCS_IFSV_SVC_SUB_ID_NCS_SVDEST /* Sub-ID */, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (VIP_REDUNDANCY_RECORD *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (VIP_REDUNDANCY_RECORD **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(VIP_REDUNDANCY_RECORD));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_vip_chk_pt_full_rec_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_vip_chk_pt_ip_data
 *
 * Description   : This is the function which is used to encode decode
 *                 VIP_RED_IP_NODE structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifsv_edp_vip_chk_pt_ip_data(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32        rc = NCSCC_RC_SUCCESS;
    VIP_RED_IP_NODE   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_edp_vip_chk_pt_ip_data_info_rules[ ] = {
        {EDU_START, ifsv_edp_vip_chk_pt_ip_data, 0, 0, 0, sizeof(VIP_RED_IP_NODE), 0, NULL},
        {EDU_EXEC, ncs_edp_ippfx, 0, 0, 0, (long)&((VIP_RED_IP_NODE*)0)->ip_addr ,0, NULL},
        {EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0, (long)&((VIP_RED_IP_NODE*)0)->ipAllocated ,0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((VIP_RED_IP_NODE*)0)->intfName, m_NCS_IFSV_VIP_INTF_NAME, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (VIP_RED_IP_NODE *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (VIP_RED_IP_NODE **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(VIP_RED_IP_NODE));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_edp_vip_chk_pt_ip_data_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}


/****************************************************************************
 * Name          : ifsv_edp_vip_chk_pt_intf_data
 *
 * Description   : This is the function which is used to encode decode
 *                 VIP_RED_INTF_NODE structure.
 *
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifsv_edp_vip_chk_pt_intf_data(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    VIP_RED_INTF_NODE   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_edp_vip_chk_pt_intf_data_info_rules[ ] = {
        {EDU_START, ifsv_edp_vip_chk_pt_intf_data, 0, 0, 0, sizeof(VIP_RED_INTF_NODE), 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((VIP_RED_INTF_NODE*)0)->intfName, m_NCS_IFSV_VIP_INTF_NAME, NULL},
        {EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0, (long)&((VIP_RED_INTF_NODE*)0)->active_standby ,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (VIP_RED_INTF_NODE *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (VIP_RED_INTF_NODE **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(VIP_RED_INTF_NODE));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_edp_vip_chk_pt_intf_data_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

/****************************************************************************
 * Name          : ifsv_edp_vip_chk_pt_owner_data
 *
 * Description   : This is the function which is used to encode decode
 *                 VIP_RED_OWNER_NODE structure.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifsv_edp_vip_chk_pt_owner_data(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    VIP_RED_OWNER_NODE   *struct_ptr = NULL, **d_ptr = NULL;
    EDU_INST_SET    ifsv_edp_vip_chk_pt_owner_data_info_rules[ ] = {
        {EDU_START, ifsv_edp_vip_chk_pt_owner_data, 0, 0, 0, sizeof(VIP_RED_OWNER_NODE), 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (long)&((VIP_RED_OWNER_NODE*)0)->owner, 0, NULL},
#if (VIP_HALS_SUPPORT == 1)
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((VIP_RED_OWNER_NODE*)0)->infraFlag, 0, NULL},
#endif

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (VIP_RED_OWNER_NODE *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (VIP_RED_OWNER_NODE **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(VIP_RED_OWNER_NODE));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_edp_vip_chk_pt_owner_data_info_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}

#endif

#if(NCS_IFSV_BOND_INTF == 1)
uns32 ifsv_edp_nodeid_ifname(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                       NCSCONTEXT ptr, uns32 *ptr_data_len,
                       EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                       EDU_ERR *o_err)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    NCS_IFSV_NODEID_IFNAME  *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET    ifsv_nodeid_ifname_rules[ ] = {
        {EDU_START, ifsv_edp_nodeid_ifname, 0, 0, 0, sizeof(NCS_IFSV_NODEID_IFNAME), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (long)&((NCS_IFSV_NODEID_IFNAME*)0)->node_id, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, (long)&((NCS_IFSV_NODEID_IFNAME*)0)->if_name, 20, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (NCS_IFSV_NODEID_IFNAME *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (NCS_IFSV_NODEID_IFNAME **)ptr;
        if(*d_ptr == NULL)
        {
           *o_err = EDU_ERR_MEM_FAIL;
           return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(NCS_IFSV_NODEID_IFNAME));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }
    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, ifsv_nodeid_ifname_rules,
       struct_ptr, ptr_data_len, buf_env, op, o_err);
    return rc;
}
#endif
