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
        
This include file contains the message definitions for PSSv checkpoint
updates between the ACTIVE and STANDBY PSS peers.
          
******************************************************************************
*/

#ifndef PSS_MBCSV_H
#define PSS_MBCSV_H

#include "mbcsv_papi.h"
#include "psr_def.h"

/* Checkpoint message types(Used as 'reotype' w.r.t mbcsv)  */

/* Some of the checkpoint update messages are processed similar to PSSv 
 * internal messages handling,  so the types are defined such that the 
 * values can be mapped straight with PSSv message types.
 */

typedef enum pss_ckpt_data_type
{
   PSS_CKPT_DATA_BASE = 1,                          /* Active to Standby */
   PSS_CKPT_DATA_TBL_BIND = PSS_CKPT_DATA_BASE,
   PSS_CKPT_DATA_TBL_UNBIND,
   PSS_CKPT_DATA_OAA_DOWN,
   PSS_CKPT_DATA_PLBCK_SSN_INFO,
   PSS_CKPT_DATA_PEND_WBREQ_TO_BAM,
   PSS_CKPT_DATA_BAM_CONF_DONE,
   PSS_CKPT_DATA_RELOAD_PSSVLIBCONF,
   PSS_CKPT_DATA_RELOAD_PSSVSPCNLIST,
   PSS_CKPT_DATA_MAX
}PSS_CKPT_DATA_TYPE;

/* PSSv-MBCSv PSS_CKPT_DATA_PLBCK_SSN_INFO message-related bitmasks. These are
   used in the encode/decode message by MBCSv. */
#define  PSS_RE_PLBCK_BITMASK_SSN_IN_PROGRESS      0x01
#define  PSS_RE_PLBCK_BITMASK_SSN_WARMBOOT         0x02
#define  PSS_RE_PLBCK_BITMASK_SSN_PCN_VALID        0x04
#define  PSS_RE_PLBCK_BITMASK_SSN_IS_DONE          0x08

typedef uns8         PSS_RE_PLBCK_BITMASK;

#define m_PSS_RE_PLBCK_IS_SSN_IN_PROGRESS(mask) \
   (((mask & PSS_RE_PLBCK_BITMASK_SSN_IN_PROGRESS) == \
        PSS_RE_PLBCK_BITMASK_SSN_IN_PROGRESS) ? TRUE : FALSE)

#define m_PSS_RE_PLBCK_IS_SSN_DONE(mask) \
   (((mask & PSS_RE_PLBCK_BITMASK_SSN_IS_DONE) == \
        PSS_RE_PLBCK_BITMASK_SSN_IS_DONE) ? TRUE : FALSE)

#define m_PSS_RE_PLBCK_IS_WARMBOOT_SSN(mask) \
   (((mask & PSS_RE_PLBCK_BITMASK_SSN_WARMBOOT) == \
        PSS_RE_PLBCK_BITMASK_SSN_WARMBOOT) ? TRUE : FALSE)

#define m_PSS_RE_PLBCK_IS_PCN_VALID(mask) \
   (((mask & PSS_RE_PLBCK_BITMASK_SSN_PCN_VALID) == \
        PSS_RE_PLBCK_BITMASK_SSN_PCN_VALID) ? TRUE : FALSE)


#define m_PSS_RE_PLBCK_SET_SSN_IN_PROGRESS(mask) \
   (mask |= PSS_RE_PLBCK_BITMASK_SSN_IN_PROGRESS)

#define m_PSS_RE_PLBCK_SET_SSN_IS_DONE(mask) \
   (mask |= PSS_RE_PLBCK_BITMASK_SSN_IS_DONE)

#define m_PSS_RE_PLBCK_SET_SSN_WARMBOOT(mask) \
   (mask |= PSS_RE_PLBCK_BITMASK_SSN_WARMBOOT)

#define m_PSS_RE_PLBCK_SET_SSN_PCN_VALID(mask) \
   (mask |= PSS_RE_PLBCK_BITMASK_SSN_PCN_VALID)


#define m_PSS_RE_PLBCK_CLEAR_SSN_IN_PROGRESS(mask) \
   (mask ^= PSS_RE_PLBCK_BITMASK_SSN_IN_PROGRESS)

#define m_PSS_RE_PLBCK_CLEAR_SSN_IS_DONE(mask) \
   (mask ^= PSS_RE_PLBCK_BITMASK_SSN_IS_DONE)

#define m_PSS_RE_PLBCK_CLEAR_SSN_WARMBOOT(mask) \
   (mask ^= PSS_RE_PLBCK_BITMASK_SSN_WARMBOOT)

#define m_PSS_RE_PLBCK_CLEAR_SSN_PCN_VALID(mask) \
   (mask ^= PSS_RE_PLBCK_BITMASK_SSN_PCN_VALID)


/* Structures for Checkpoint data */

/* Registrationlist checkpoint record types, used in cold/warm/async checkpoint updates */

typedef struct pss_ckpt_tbl_bind_msg_tag
{
   MDS_DEST            fr_card; /* Sender's MDS address */
   MAB_PSS_CLIENT_LIST pcn_list; /* Client information to be bound. */
}PSS_CKPT_TBL_BIND_MSG;

typedef struct pss_ckpt_tbl_unbind_msg_tag
{
   MDS_DEST            fr_card; /* Sender's MDS address */
   uns32               tbl_id; /* Client table to unbound. */
}PSS_CKPT_TBL_UNBIND_MSG;

typedef struct pss_ckpt_oaa_down_msg_tag
{
   MDS_DEST            fr_card;  /* Address of the OAA which became non-reachable. */
}PSS_CKPT_OAA_DOWN_MSG;

typedef struct pss_ckpt_pend_wbreq_to_bam_msg_tag
{
   MAB_PSS_CLIENT_LIST pcn_list; /* List of PCNs which are to be configured by BAM */
}PSS_CKPT_PEND_WBREQ_TO_BAM_MSG;

typedef struct pss_ckpt_bam_conf_done_msg_tag
{
   MAB_PSS_CLIENT_LIST pcn_list; /* List of PCNs which are configured by BAM */
}PSS_CKPT_BAM_CONF_DONE_MSG;

typedef struct pss_ckpt_reload_pssvlibconf_msg_tag
{
   char *lib_conf_file;
}PSS_CKPT_RELOAD_PSSVLIBCONF_MSG;

struct pss_spcn_list_tag;
typedef struct pss_ckpt_reload_pssvspcnlist_msg_tag
{
   struct pss_spcn_list_tag *spcn_list;
}PSS_CKPT_RELOAD_PSSVSPCNLIST_MSG;

#define PSS_CKPT_PLBCK_SSN_INFO_MSG    PSS_CURR_PLBCK_SSN_INFO

typedef struct pss_curr_plbck_ssn_info_tag
{
   NCS_BOOL       plbck_ssn_in_progress;
   NCS_BOOL       is_warmboot_ssn;  /* Whether this is a warmboot or 
                                       on-demand session */

   union {
      struct {
         MAB_PSS_WARMBOOT_REQ    wb_req;
         uns16                   wbreq_cnt; /* Number of individual requests */
         NCS_BOOL                partial_delete_done;  /* The first request in the list is removed. */
         MDS_DEST                mds_dest;  /* The OAA's address */
         uns32                   seq_num;   /* OAA message's seq num */
      }info2;
      char                    alt_profile[NCS_PSS_MAX_PROFILE_NAME]; /* For on-demand session */
   }info;

   char           *pcn;       /* Current PCN played-back to MASv. 
                                 Default value is NULL. */

   uns16          tbl_id;     /* Current MIB Table played-back to MASv.
                                 Default value is 0. */

   NCSMIB_IDX     mib_idx;   /* Current MIB-index of the MIB row played-back to MASv.
                                 Default value is NULL. */

   uns32          mib_obj_id; /* Current MIB-object of the MIB-row played-back to MASv.
                                  Default value is 0. */
}PSS_CURR_PLBCK_SSN_INFO;

/* Checkpoint message containing PSS data of a particular type.
 * Used during cold-sync, warm-sync, and async updates.
 */
typedef struct pss_ckpt_msg_tag
{
   PSS_CKPT_DATA_TYPE  ckpt_data_type; /* Type of PSS data carried in this checkpoint */
   union
   {
      PSS_CKPT_TBL_BIND_MSG           tbl_bind;
      PSS_CKPT_TBL_UNBIND_MSG         tbl_unbind;
      PSS_CKPT_OAA_DOWN_MSG           oaa_down;
      PSS_CKPT_PLBCK_SSN_INFO_MSG     plbck_ssn_info;
      PSS_CKPT_PEND_WBREQ_TO_BAM_MSG  pend_wbreq_to_bam;
      PSS_CKPT_BAM_CONF_DONE_MSG      bam_conf_done;
      PSS_CKPT_RELOAD_PSSVLIBCONF_MSG reload_pssvlibconf;
      PSS_CKPT_RELOAD_PSSVSPCNLIST_MSG reload_pssvspcnlist;
   } ckpt_data;
} PSS_CKPT_MSG;

/* typedef uns32 (*PSS_CKPT_HDLR)(struct pss_pwe_cb_tag *pwe_cb, PSS_CKPT_MSG *msg); */
struct pss_cb_tag;
struct pss_pwe_cb_tag;

EXTERN_C uns32 pss_mbcsv_register(struct pss_cb_tag *cb);
EXTERN_C uns32 pss_mbcsv_deregister(struct pss_cb_tag *pss_cb);
EXTERN_C uns32 pss_mbcsv_obj_set(struct pss_pwe_cb_tag *pwe_cb,uns32 obj, uns32 val);
EXTERN_C uns32 pss_mbcsv_open_ckpt(struct pss_pwe_cb_tag *pwe_cb);
EXTERN_C uns32 pss_mbcsv_close_ckpt(NCS_MBCSV_CKPT_HDL ckpt_hdl);
EXTERN_C uns32 pss_mbcsv_callback(NCS_MBCSV_CB_ARG *arg);
EXTERN_C uns32 pss_ckpt_peer_info_cbk_handler(NCS_MBCSV_CB_ARG *arg);
EXTERN_C uns32 pss_ckpt_notify_cbk_handler (NCS_MBCSV_CB_ARG *arg);
EXTERN_C uns32 pss_ckpt_err_ind_cbk_handler (NCS_MBCSV_CB_ARG *arg);
EXTERN_C uns32 pss_ckpt_encode_cbk_handler(NCS_MBCSV_CB_ARG *cbk_arg);
EXTERN_C uns32 pss_ckpt_decode_cbk_handler(NCS_MBCSV_CB_ARG *cbk_arg, NCS_BOOL *is_in_sync);

EXTERN_C uns32 pss_ckpt_enc_coldsync_resp(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *io_uba, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_enc_warmsync_resp(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *io_uba);
EXTERN_C uns32 pss_ckpt_dec_coldsync_resp(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *io_uba, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_dec_warmsync_resp(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *io_uba,
                   NCS_BOOL *is_in_sync);
EXTERN_C uns32 pss_ckpt_enc_data_req(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *io_uba);
EXTERN_C uns32 pss_ckpt_enc_data_resp(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *io_uba, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_dec_data_req(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *io_uba);
EXTERN_C uns32 pss_ckpt_dec_data_resp(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *io_uba, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_encode_async_update(NCS_MBCSV_CB_ARG *cbk_arg);
EXTERN_C uns32 pss_ckpt_decode_async_update(NCS_MBCSV_CB_ARG *cbk_arg);
EXTERN_C uns32 pss_set_ckpt_role(struct pss_pwe_cb_tag *pwe_cb, SaAmfHAStateT role);

EXTERN_C uns32 pss_send_data_req(NCS_MBCSV_CKPT_HDL ckpt_hdl);
EXTERN_C uns32 pss_send_ckpt_data(struct pss_pwe_cb_tag *pwe_cb,
                         uns32      action,
                         uns32      reo_hdl,
                         uns32      reo_type,
                         uns32      send_type);

EXTERN_C uns32 pss_ckpt_enc_tbl_bind(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *uba, PSS_CKPT_TBL_BIND_MSG *bind, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_enc_tbl_unbind(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *uba, PSS_CKPT_TBL_UNBIND_MSG *unbind, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_enc_oaa_down(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *uba, PSS_CKPT_OAA_DOWN_MSG *oaa_down, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_enc_plbck_ssn_info(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *uba, PSS_CKPT_PLBCK_SSN_INFO_MSG *plbck_ssninfo, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_enc_bam_conf_done(NCS_UBAID *uba, PSS_CKPT_BAM_CONF_DONE_MSG *bam_conf_done);
EXTERN_C uns32 pss_ckpt_dec_tbl_bind(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *uba, PSS_CKPT_TBL_BIND_MSG *bind, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_dec_tbl_unbind(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *uba, PSS_CKPT_TBL_UNBIND_MSG *unbind, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_dec_oaa_down(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *uba, PSS_CKPT_OAA_DOWN_MSG *oaa_down, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_dec_plbck_ssn_info(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *uba, PSS_CKPT_PLBCK_SSN_INFO_MSG *plbck_ssninfo, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_ckpt_dec_bam_conf_done(NCS_UBAID *uba, PSS_CKPT_BAM_CONF_DONE_MSG *bam_conf_done);
EXTERN_C uns32 pss_ckpt_enc_reload_pssvlibconf(NCS_UBAID *uba, char *file_name);
EXTERN_C uns32 pss_ckpt_dec_n_updt_reload_pssvlibconf(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *uba); 
EXTERN_C uns32 pss_ckpt_enc_reload_pssvspcnlist(NCS_UBAID *uba, struct pss_spcn_list_tag *spcn_list);
EXTERN_C uns32 pss_ckpt_dec_n_updt_reload_pssvspcnlist(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *uba);
EXTERN_C uns32 pss_ckpt_convert_n_process_event(struct pss_pwe_cb_tag *pwe_cb, PSS_CKPT_MSG *data,
                   NCS_MBCSV_CB_ARG *cbk_arg);
EXTERN_C uns32 pss_process_re_plbck_ssn_info(struct pss_pwe_cb_tag *pwe_cb, PSS_CKPT_MSG *data);
EXTERN_C uns32 pss_mdfy_plbck_info(struct pss_pwe_cb_tag *pwe_cb, PSS_CKPT_MSG *data);
EXTERN_C void pss_flush_plbck_ssn_info(PSS_CURR_PLBCK_SSN_INFO *ssn_info);
EXTERN_C void pss_partial_flush_warmboot_plbck_ssn_info(PSS_CURR_PLBCK_SSN_INFO *ssn_info);
EXTERN_C uns32 pss_re_sync(struct pss_pwe_cb_tag *pwe_cb, MAB_MSG *mab_msg, PSS_CKPT_DATA_TYPE ckpt_type);
EXTERN_C uns32 pss_duplicate_pcn_list(MAB_PSS_CLIENT_LIST *src, MAB_PSS_CLIENT_LIST *p_dest);
EXTERN_C uns32 pss_dup_re_wbreq_info(MAB_PSS_WARMBOOT_REQ *src, MAB_PSS_WARMBOOT_REQ *dst);
EXTERN_C uns32 pss_duplicate_plbck_ssn_info(PSS_CURR_PLBCK_SSN_INFO *src, PSS_CURR_PLBCK_SSN_INFO *dst);
EXTERN_C void pss_free_client_list(MAB_PSS_CLIENT_LIST *list);
EXTERN_C void pss_free_re_msg(PSS_CKPT_MSG *re_msg);
EXTERN_C void pss_free_plbck_ssn_info(PSS_CURR_PLBCK_SSN_INFO *ssn_info);
EXTERN_C void pss_free_re_wbreq_info(MAB_PSS_WARMBOOT_REQ *wbreq);
EXTERN_C uns32 pss_encode_str(NCS_UBAID *uba, char *str_ptr);
EXTERN_C uns32 pss_decode_str(NCS_UBAID *uba, char *p_str);
EXTERN_C uns32 pss_encode_pcn_list(NCS_UBAID *uba, MAB_PSS_CLIENT_LIST *pcn_list);
EXTERN_C uns32 pss_decode_pcn_list(NCS_UBAID *uba, MAB_PSS_CLIENT_LIST *pcn_list);

EXTERN_C uns32 pss_resume_active_role_activity(MAB_MSG *msg);
EXTERN_C uns32 pss_re_update_pend_wbreq_info(struct pss_pwe_cb_tag *pwe_cb,
            PSS_CKPT_PEND_WBREQ_TO_BAM_MSG *pend_wbreq_to_bam,
            NCS_MBCSV_ACT_TYPE action);
EXTERN_C uns32 pss_re_direct_sync(struct pss_pwe_cb_tag *pwe_cb, NCSCONTEXT msg, PSS_CKPT_DATA_TYPE ckpt_type, NCS_MBCSV_ACT_TYPE act_type);
EXTERN_C uns32 pss_re_sync(struct pss_pwe_cb_tag *pwe_cb, MAB_MSG *mab_msg, PSS_CKPT_DATA_TYPE ckpt_type);
EXTERN_C uns32 pss_indicate_end_of_playback_to_standby(struct pss_pwe_cb_tag *pwe_cb, NCS_BOOL is_error_condition);
EXTERN_C uns32 pss_updt_in_wbreq_into_cb(struct pss_pwe_cb_tag *pwe_cb, MAB_PSS_WARMBOOT_REQ *req);
EXTERN_C uns32 pss_gen_wbreq_from_cb(PSS_CURR_PLBCK_SSN_INFO *src, MAB_PSS_WARMBOOT_REQ *wbreq);
EXTERN_C uns32 pss_enc_pwe_data_for_sync_with_standby(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *io_uba, NCS_BOOL enc_lib_conf, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_dec_pwe_data_for_sync_with_standby(struct pss_pwe_cb_tag *pwe_cb, NCS_UBAID *io_uba, NCS_BOOL dec_lib_conf, uns16 peer_mbcsv_version);
EXTERN_C uns32 pss_process_vdest_role_quiesced(MAB_MSG *msg);
EXTERN_C uns32 pss_encode_pcn(NCS_UBAID *uba, char *pcn);
EXTERN_C uns32 pss_decode_pcn(NCS_UBAID *uba, char **pcn);
EXTERN_C uns32 pss_ckpt_enc_pend_wbreq_to_bam_info(NCS_UBAID *uba, PSS_CKPT_PEND_WBREQ_TO_BAM_MSG *msg);
EXTERN_C uns32 pss_ckpt_dec_pend_wbreq_to_bam_info(NCS_UBAID *uba, PSS_CKPT_PEND_WBREQ_TO_BAM_MSG *msg);


#endif /* !PSS_MBCSV_H*/
