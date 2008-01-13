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
FILE NAME: IFSVIPXS.H
DESCRIPTION: Enums, Structures and Function prototypes for IPXS
******************************************************************************/

#ifndef IFSVIPXS_H
#define IFSVIPXS_H

/* Data Structure & Enums used for creating/destroying/accessing IPXS library */

/* Requiest types into IPXS library */
typedef enum
{
   IPXS_LIB_REQ_CREATE = 1,
   IPXS_LIB_REQ_DESTROY,
   IPXS_LIB_REQ_TYPE_MAX
} IPXS_LIB_REQ_TYPE;


/* Create info */
typedef struct ipxs_lib_create
{
   NCS_SERVICE_ID       svc_id;    /* Service ID of the IfD/IfND */
   uns8                 pool_id;   /* Pool ID of the Handle Manager */
   uns32                oac_hdl;   /* OAC Handle */
   MDS_HDL              mds_hdl;
   uns32                ifsv_hdl;
}IPXS_LIB_CREATE;

/* IPXS library request info */
typedef struct ipxs_lib_req_info
{
   IPXS_LIB_REQ_TYPE       op;
   union
   {
      IPXS_LIB_CREATE      create;
    } info;
} IPXS_LIB_REQ_INFO;

/* Macros to Set the flags */
#define m_NCS_IPXS_IPAM_ADDR_SET(attr)  (attr = attr | NCS_IPXS_IPAM_ADDR)
#define m_NCS_IPXS_IPAM_UNNMBD_SET(attr)  (attr = attr | NCS_IPXS_IPAM_UNNMBD)
#define m_NCS_IPXS_IPAM_RRTRID_SET(attr)  (attr = attr | NCS_IPXS_IPAM_RRTRID)
#define m_NCS_IPXS_IPAM_RMTRID_SET(attr)  (attr = attr | NCS_IPXS_IPAM_RMTRID)
#define m_NCS_IPXS_IPAM_RMT_AS_SET(attr)  (attr = attr | NCS_IPXS_IPAM_RMT_AS)
#define m_NCS_IPXS_IPAM_RMTIFID_SET(attr)  (attr = attr | NCS_IPXS_IPAM_RMTIFID)
#define m_NCS_IPXS_IPAM_UD_1_SET(attr)  (attr = attr | NCS_IPXS_IPAM_UD_1)
#define m_NCS_IPXS_IPAM
#if (NCS_VIP == 1)
#define NCS_IPXS_IPAM_VIP_SET(attr)     (attr = attr |  NCS_IPXS_IPAM_VIP) 
#define NCS_IPXS_IPAM_REFCNT_SET(attr)  (attr = attr |  NCS_IPXS_IPAM_REFCNT)    
#define NCS_IPXS_IPAM_VIP_FREE(attr)    (attr = attr & ~NCS_IPXS_IPAM_VIP) 
#define NCS_IPXS_IPAM_REFCNT_FREE(attr) (attr = attr & ~NCS_IPXS_IPAM_REFCNT)    
#endif


/*****************************************************************************
 * Macros used to check the presence of attributes in the record data.
 *****************************************************************************/
#define m_NCS_IPXS_IS_IPAM_ADDR_SET(attr)   \
            (((attr & NCS_IPXS_IPAM_ADDR) != 0)?TRUE:FALSE)
#define m_NCS_IPXS_IS_IPAM_UNNMBD_SET(attr)   \
            (((attr & NCS_IPXS_IPAM_UNNMBD) != 0)?TRUE:FALSE)
#define m_NCS_IPXS_IS_IPAM_RRTRID_SET(attr)   \
            (((attr & NCS_IPXS_IPAM_RRTRID) != 0)?TRUE:FALSE)
#define m_NCS_IPXS_IS_IPAM_RMTRID_SET(attr)   \
            (((attr & NCS_IPXS_IPAM_RMTRID) != 0)?TRUE:FALSE)
#define m_NCS_IPXS_IS_IPAM_RMT_AS_SET(attr)   \
            (((attr & NCS_IPXS_IPAM_RMT_AS) != 0)?TRUE:FALSE)
#define m_NCS_IPXS_IS_IPAM_RMTIFID_SET(attr)   \
            (((attr & NCS_IPXS_IPAM_RMTIFID) != 0)?TRUE:FALSE)
#define m_NCS_IPXS_IS_IPAM_UD_1_SET(attr)   \
            (((attr & NCS_IPXS_IPAM_UD_1) != 0)?TRUE:FALSE)
#if (NCS_VIP == 1)
#define m_NCS_IPXS_IS_IPAM_VIP_SET(attr)   \
            (((attr & NCS_IPXS_IPAM_VIP) != 0)?TRUE:FALSE)
#define m_NCS_IPXS_IS_IPAM_VIP_REFCNT_SET(attr)\
            (((attr & NCS_IPXS_IPAM_REFCNT) != 0)?TRUE:FALSE)
#endif

/* Declerations for IPXS EDU Routines */
uns32 ipxs_edp_ipxs_intf_rec_info_noptr(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_ipxs_intf_rec_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_evt_if_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

int ipxs_ifkey_type_test_fnc (NCSCONTEXT arg);

uns32 ipxs_edp_ipxs_ifkey_info (EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err);

uns32 ipxs_edp_evt_if_rec_get_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_ipxs_evt_islocal_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_node_rec_get_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_ndtoa_ifip_add_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_ndtoa_ifip_del_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_node_rec_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_ipxs_evt_isloc_rsp_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ifsv_edp_ifsv_ipxs_info (EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);
uns32 ifsv_edp_ifd_a2s_ipxs_intf_info_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err);

uns32 ncs_edp_ip_addr(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ncs_edp_ippfx(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_ifip_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ncs_edp_ipxs_ifip_ip_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_atond_ifip_req_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_ndtoa_ifip_rsp_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ipxs_edp_ndtoa_ifip_upd_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err);

uns32 ipxs_update_ifadd(NCS_SERVICE_ID svc_id, IFSV_INTF_DATA *intf_data);
uns32 ipxs_update_ifdel(NCS_SERVICE_ID svc_id, uns32 if_index);

/* IPXS Record Free */
void ipxs_intf_rec_attr_free(NCS_IPXS_INTF_REC *i_intf_rec, NCS_BOOL i_is_rec_free);
uns32 ipxs_intf_rec_list_cpy(NCS_IPXS_INTF_REC *src, NCS_IPXS_INTF_REC **dest,
                             NCS_IPXS_SUBCR *subr, NCS_BOOL *is_app_send, 
                             IPXS_EVT_TYPE evt_type, NCS_BOOL node_scope_flag);
uns32 ipxs_intf_rec_cpy(NCS_IPXS_INTF_REC *src, 
                        NCS_IPXS_INTF_REC *dest,
                        IFSV_MALLOC_USE_TYPE purpose);

uns32 ipxs_ipinfo_cpy(NCS_IPXS_IPINFO *src, 
                      NCS_IPXS_IPINFO *dest, 
                      IFSV_MALLOC_USE_TYPE purpose);

void ipxs_ipinfo_free(NCS_IPXS_IPINFO *ip_info, IFSV_MALLOC_USE_TYPE purpose);

/* IPXS IFND function declerations */
uns32 ipxs_ifnd_lib_req(IPXS_LIB_REQ_INFO *ipxs_info);
uns32 ipxs_ifa_app_if_info_indicate(IFSV_INTF_DATA *actual_data, 
                            IFSV_INTF_REC_EVT intf_evt, uns32 attr,
                            IFSV_CB *cb);

#endif /* IFSVIPXS_H */
