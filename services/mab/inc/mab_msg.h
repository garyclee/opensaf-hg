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

  DESCRIPTION: Common MAB sub-part messaging formats that travel between 
  sub-parts, being OAC, MAS and MAC.
 

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef MAB_MSG_H
#define MAB_MSG_H

#include "psr_def.h"

/* typedef for MDS Anchor values */ 
typedef V_DEST_QA MAB_ANCHOR;

/***************************************************************************
 * MAB sub-component op codes implies presence of particular fields
 ***************************************************************************/

typedef enum mab_svc_op
{

  /* service entities dispatched to at the MIB Access Server (MAS) */
  MAB_MAS_TRAP_FWDR,     /* data = undefined  */
  MAB_MAS_REQ_HDLR,      /* data = NCSMIB_ARG  */
  MAB_MAS_RSP_HDLR,      /* data = NCSMIB_ARG  */
  MAB_MAS_REG_HDLR,      /* data = MAS_REG    */
  MAB_MAS_UNREG_HDLR,    /* data = MAS_UNREG  */
  MAB_MAS_ASYNC_DONE,    /* data = unused     */ /* Active MAS sends Standby MAS */
  MAB_MAS_DESTROY,       /* data = unused     */
  MAB_MAS_AMF_INIT_RETRY, /* data = unused     */

  /* service entities dispatched at the MIB Access Client (MAC) */
  MAB_MAC_RSP_HDLR,      /* data = NCSMIB_ARG  */
  MAB_MAC_TRAP_HDLR,     /* data = undefined  */
  MAB_MAC_PLAYBACK_START,/* data = unused     */
  MAB_MAC_PLAYBACK_END,  /* data = unused     */
  MAB_MAC_DESTROY,       /* data = unused     */

  /* service entities dispatched to at the Object Access Client (OAC) */
  MAB_OAC_REQ_HDLR,      /* data = NCSMIB_ARG    */ 
  MAB_OAC_RSP_HDLR,      /* data = NCSMIB_ARG    */
  MAB_OAC_MAS_REFRESH,   /* data = undefined    */
  MAB_OAC_FIR_HDLR,      /* data = NCSMAB_IDX_FREE */
  MAB_OAC_DEF_REQ_HDLR,   /* data = NCSMIB_ARG    */
  MAB_OAC_PLAYBACK_START,/* data = unused       */
  MAB_OAC_PLAYBACK_END,  /* data = unused       */
  MAB_OAC_SVC_MDS_EVT,   /* SVCS MDS-DOWN/UP event from MDS */
  MAB_OAC_PSS_ACK,       /* Ack from PSS */
  MAB_OAC_PSS_EOP_EVT,   /* Warmboot playback status indication from PSS */
  MAB_OAC_DESTROY,       /* data = unused     */

  /* service entities dispatched to at the PSS */
  MAB_PSS_SET_REQUEST,   /* data = NCSMIB_ARG    */
  MAB_PSS_WARM_BOOT,     /* data = MAB_PSS_WARMBOOT_REQ       */
  MAB_PSS_TBL_BIND,     /* data = MAB_PSS_TBL_BIND_EVT       */
  MAB_PSS_TBL_UNBIND,     /* data = MAB_PSS_TBL_UNBIND_EVT       */
  MAB_PSS_MIB_REQUEST,    /* data = NCSMIB_ARG    */
  MAB_PSS_SVC_MDS_EVT,  /* SVCS MDS-DOWN/UP event for PSS from MDS */

  /* OAA down event from MDS (used by MAS) */
  MAB_OAA_DOWN_EVT,

  MAB_PSS_BAM_CONF_DONE, /* Config-done event from BAM to PSS */
  MAB_PSS_VDEST_ROLE_QUIESCED, /* VDEST role is now Quiesced */
  MAB_PSS_RE_RESUME_ACTIVE_ROLE_FUNCTIONALITY, /* Take over as Active */
  MAB_PSS_AMF_INIT_RETRY, /* data = unused */ 

  /* QUIESCED State change event (used by MAS)*/
  MAB_SVC_VDEST_ROLE_QUIESCED,  /* data = unused */ 
  MAB_OAA_CHG_ROLE,  /* data = unused */ 
  
  MAB_SVC_OP_MAX
} MAB_SVC_OP;

/***************************************************************************
 * Private: OAC -to- MAS exchange info with these containers
 ***************************************************************************/

typedef struct mas_reg    /* Data associated with MAS Filter Registration   */
  {
    
    uns32        tbl_id;
    NCSMAB_FLTR   fltr;
    uns32        fltr_id;

  } MAS_REG;

typedef struct mas_unreg  /* Data associated with MAS Filter Unregistration */
  {
    
    uns32        tbl_id;
    uns32        fltr_id;

  } MAS_UNREG;

/* Warmboot Request(of linked-list type) to PSS */
typedef struct mab_pss_warmboot_req_tag
{
    MAB_PSS_CLIENT_LIST pcn_list;

    NCS_BOOL            is_system_client;

    uns32               wbreq_hdl;  /* OAA provided unique handle */

    uns32               mib_key;   /* User handle */

    struct mab_pss_warmboot_req_tag *next;
}MAB_PSS_WARMBOOT_REQ;

/* OAC Table Bind event to PSS */
typedef struct mab_pss_oac_tbl_bind_evt_tag
{
    MAB_PSS_CLIENT_LIST pcn_list;

    struct mab_pss_oac_tbl_bind_evt_tag *next;
}MAB_PSS_TBL_BIND_EVT;

/* OAC Table Unbind event to PSS */
typedef struct mab_pss_oac_tbl_unbind_evt_tag
{
    uns32       tbl_id;
}MAB_PSS_TBL_UNBIND_EVT;

typedef struct mab_pss_mds_svc_evt_tag
{
   V_DEST_RL          role;
   NCSMDS_CHG         change;  /* UP/DOWN status */
   MDS_HDL            svc_pwe_hdl;
}MAB_PSS_MDS_SVC_EVT;

typedef struct mab_oac_mds_svc_evt_tag
{
   NCSMDS_CHG         change;  /* UP/DOWN status */
   V_DEST_RL          role;
   V_DEST_QA          anc;
   NCSCONTEXT         svc_pwe_hdl;
}MAB_OAC_MDS_SVC_EVT;

typedef struct mab_oac_pss_eop_ind_tag
{
   uns32              mib_key;
   uns32              wbreq_hdl;  /* unique handle per warmboot request per OAA */
   uns32              status;   /* NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE */
}MAB_OAC_PSS_EOP_IND;

typedef struct mab_pss_bam_conf_done_evt_tag
{
    MAB_PSS_CLIENT_LIST pcn_list;
}MAB_PSS_BAM_CONF_DONE_EVT;

typedef struct mab_pss_playback_start_evt_tag
{
    USRBUF *i_ub;
    uns16  *o_tbl_list; /* First tuple stores number of remaining valid bytes in this memory */
}MAB_PSS_PLAYBACK_START_EVT;



/***************************************************************************
 * Private: MAB has different message content based on Service Operation
 ***************************************************************************/

typedef struct mab_msg_data    /* The union of all data types in a MAB_MSG */
  {
  uns32       seq_num;   /* Ack sequence number between OAA and PSS */
  union {

    NCSMIB_ARG*         snmp;
    MAS_REG             reg;
    MAS_UNREG           unreg;
    NCSMAB_IDX_FREE     idx_free;
    MAB_PSS_WARMBOOT_REQ oac_pss_warmboot_req; /* Warmboot playback req. from 
                                                  OAC-SU to PSS */
    MAB_PSS_TBL_BIND_EVT oac_pss_tbl_bind;     /* Table Bind request from 
                                                  OAC to PSS */
    MAB_PSS_TBL_UNBIND_EVT oac_pss_tbl_unbind; /* Table Unbind request from 
                                                  OAC to PSS */
    MAB_PSS_MDS_SVC_EVT    pss_mds_svc_evt;    /* MDS svc events to PSS */
    MAB_OAC_MDS_SVC_EVT    oac_mds_svc_evt;    /* MDS svc events to PSS */
    MAB_OAC_PSS_EOP_IND    oac_pss_eop_ind;    /* Warmboot playback status to OAA from PSS */
    MAB_PSS_BAM_CONF_DONE_EVT bam_conf_done;   /* Config done event from BAM to PSS */
    MAB_PSS_PLAYBACK_START_EVT plbck_start;    /* Broadcast event to all MAAs */
    } data;

  } MAB_MSG_DATA;

/***************************************************************************
 * Private: MAB message passing structure
 ***************************************************************************/

typedef struct mab_msg
  {
  struct mab_msg*    next;    /* for a linked list of them                    */
  NCS_VRID           vrid;    /* Virtual router ID for sanity sake            */
  NCSCONTEXT         yr_hdl;  /* MDS has this value as its MAC/MAS/OAC handle */
  MDS_DEST           fr_card; /* Sender's Virtual Card ID                     */
  MDS_HDL            pwe_hdl; /* PWE of the sender; used by PSS              */
  V_DEST_QA          fr_anc;  /* Sender's VCARD anchor                        */
  SS_SVC_ID          fr_svc;  /* Sender's Service ID                          */
  V_DEST_RL          fr_role; /* Sender's role                                */
  MAB_SVC_OP         op;      /* encoded by sender to proper subservice       */
  MAB_MSG_DATA       data;    /* Data corresponding to the type               */

  }MAB_MSG;


/* list to maintain the list of Env-id and the corresponding MAA handle */
/* this structure is an odd man out here in this file.  
 * since there is no other common file, that will get included in both 
 * MAA and OAA, this structure is made to live in this file. 
 */
typedef struct mab_inst_node
{
    PW_ENV_ID   i_env_id;  /* environment id */
    SaNameT     i_inst_name; /* Instance Name */ 
    uns32       i_hdl;  /* MAA/OAA handle */ 
    struct mab_inst_node *next;  /* pointer to the next MAA instance */ 
}MAB_INST_NODE; 

/* clones a range filter indices */ 
EXTERN_C uns32
mas_mab_range_fltr_clone(NCSMAB_RANGE *src_range, NCSMAB_RANGE *dst_range); 

/* clones a exact filter indices */ 
EXTERN_C uns32
mas_mab_exact_fltr_clone(NCSMAB_EXACT *src_exact, NCSMAB_EXACT *dst_exact); 

/* frees the indices information based on the type of the filter, and frees the 
 * message 
 */
EXTERN_C void
mas_mab_fltr_indices_cleanup(NCSMAB_FLTR *fltr); 

/* to free the messages in the mail-box of MAA, OAA, and MAS */ 
EXTERN_C NCS_BOOL 
mab_leave_on_queue_cb (void *arg1, void *arg2); 

EXTERN_C uns32  
mab_fltr_encode(NCSMAB_FLTR*      mf, NCS_UBAID* uba);

EXTERN_C uns32  
mab_fltr_decode(NCSMAB_FLTR*      mf, NCS_UBAID* uba);

#endif

