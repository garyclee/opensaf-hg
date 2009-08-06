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

  _Private_ Object Access Client (OAC) abstractions and function prototypes.

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef OAC_PVT_H
#define OAC_PVT_H

#define OAC_MDS_SUB_PART_VERSION  2  /* OAC version is increased as ncsmib_arg is changed(handles & ncsmib_rsp changed)*/
#define OAC_WRT_MAS_SUBPART_VER_AT_MIN_MSG_FMT    1 /* minimum version of MAS supported by OAC */
#define OAC_WRT_MAS_SUBPART_VER_AT_MAX_MSG_FMT    2 /* maximum version of MAS supported by OAC */

/* size of the array used to map MAS subpart version to mesage format version */
#define  OAC_WRT_MAS_SUBPART_VER_RANGE                \
         (OAC_WRT_MAS_SUBPART_VER_AT_MAX_MSG_FMT  -   \
         OAC_WRT_MAS_SUBPART_VER_AT_MIN_MSG_FMT + 1)

#define OAC_WRT_MAC_SUBPART_VER_AT_MIN_MSG_FMT    1 /* minimum version of MAC supported by OAC */
#define OAC_WRT_MAC_SUBPART_VER_AT_MAX_MSG_FMT    2 /* maximum version of MAC supported by OAC */

/* size of the array used to map MAS subpart version to mesage format version */
#define  OAC_WRT_MAC_SUBPART_VER_RANGE                \
         (OAC_WRT_MAC_SUBPART_VER_AT_MAX_MSG_FMT  -   \
         OAC_WRT_MAC_SUBPART_VER_AT_MIN_MSG_FMT + 1)

#define OAC_WRT_PSS_SUBPART_VER_AT_MIN_MSG_FMT    1
#define OAC_WRT_PSS_SUBPART_VER_AT_MAX_MSG_FMT    1

#define  OAC_WRT_PSS_SUBPART_VER_RANGE                \
         (OAC_WRT_PSS_SUBPART_VER_AT_MAX_MSG_FMT  -   \
         OAC_WRT_PSS_SUBPART_VER_AT_MIN_MSG_FMT + 1)

/************************************************************************
  OAC_TBL_REC filter and reachability info for matched MIB Row data.
*************************************************************************/

typedef struct oac_fltr
  {
  struct oac_fltr*    next;
  NCSMAB_FLTR         fltr;
  uns32               fltr_id; /*retured in the NCSOAC_ROW_OWNED.o_row_hdl field*/

  } OAC_FLTR;


/*****************************************************************************
  OAC_PCN_LIST  List of PCNs registered with OAC.
*****************************************************************************/
typedef struct oaa_pcn_list_tag
{
   uns32              tbl_id;
   char*              pcn;
   struct oaa_pcn_list_tag *next;
}OAA_PCN_LIST;

/*****************************************************************************
  OAA_WBREQ_PEND_LIST  List of pending warmboot requests from subsystems.
*****************************************************************************/
typedef struct oaa_wbreq_pend_list_tag
{
   char*               pcn;
   NCS_BOOL            is_system_client:1;
   NCSOAC_PSS_TBL_LIST *tbl_list;

   /* OAA generated unique handle per warmboot request */
   uns32               wbreq_hdl;

   /* The user-provided indication function pointer is stored here. */
   NCSOAC_EOP_USR_IND_FNC  eop_usr_ind_fnc;

   struct oaa_wbreq_pend_list_tag *next;
}OAA_WBREQ_PEND_LIST;

/*****************************************************************************
  OAA_WBREQ_HDL_LIST  List of warmboot-req-to-handle-mappings.
*****************************************************************************/
typedef struct oaa_wbreq_hdl_list_tag
{
   /* OAA generated unique handle per warmboot request */
   uns32               wbreq_hdl;

   /* The user-provided indication function pointer is stored here. */
   NCSOAC_EOP_USR_IND_FNC  eop_usr_ind_fnc;

   struct oaa_wbreq_hdl_list_tag *next;
}OAA_WBREQ_HDL_LIST;

/*****************************************************************************
  OAA_BUFR_HASH_LIST - Hash List of unacknowledged events by PSS.
*****************************************************************************/
typedef struct oaa_bufr_hash_list_tag
{
   MAB_MSG *msg;   /* Actual event is stored here */

   struct oaa_bufr_hash_list_tag *next;
}OAA_BUFR_HASH_LIST;

/*****************************************************************************
  OAC_TBL_REC A record of ownership information about a particular MIB Table 
              within the context of the OAC, 

  This record is accessed by TBL ID. The assumption is that there is only one 
  subsystem per local OAC that will claim ownership of a particular MIB table.
*****************************************************************************/
typedef struct oac_tbl_rec
  {
  struct oac_tbl_rec* next;
  uns32               tbl_id;
  NCS_BOOL            is_persistent;
  uns32               ss_id;   
  uns64               handle; /* i_mib_key value, when making MIB Access Req */
  OAC_FLTR*           fltr_list;  
  NCSMIB_REQ_FNC      req_fnc;
  NCS_BOOL            dfltr_regd;
  NCSOAC_SS_CB        ss_cb_fnc;
  uns64               ss_cb_hdl;
  } OAC_TBL_REC;


/*****************************************************************************
  OAC_TBL  A table of all MIB_TBL_RECs that this local OAC knows
                  about, as learned during registration.

*****************************************************************************/

typedef struct oac_tbl
  {
  /* Configuration Objects */

  uns32              log_bits;    /* RW bitmap of log categories to install   */
  NCS_BOOL           log_enbl;    /* RW ENABLE|DISABLE logging                */
  NCS_BOOL           oac_enbl;    /* RW ENABLE|DISABLE OAC services           */
 
  NCS_LOCK            lock;
  uns32               nxt_fltr_id;
  OAC_TBL_REC*        hash[MAB_MIB_ID_HASH_TBL_SIZE];
  OAA_PCN_LIST        *pcn_list;   /* List of PCNs registered with the OAC */
  OAA_WBREQ_PEND_LIST *wbreq_list; /* List of pending warmboot requests */

  uns32               wbreq_hdl_index;  /* Counter to assign unique handle to each warmboot request */
  OAA_WBREQ_HDL_LIST  *wbreq_hdl_list;  /* All the active wbreq-to-handle mappings go here. */

  NCS_BOOL            playback_session; /*True if the PSR has started a playback session */
  /* NCS_BOOL         warmboot_start; Deprecated, since pcn_list is used */
  uns32               seq_num;  /* Ack sequence number between OAA and PSS */
  OAA_BUFR_HASH_LIST  *hash_bufr[MAB_MIB_ID_HASH_TBL_SIZE]; /* Stores unack'ed
                       events between OAA and PSS */

  /* run-time learned values */

  MDS_DEST           my_vcard;
  V_DEST_QA          my_anc;
  NCS_BOOL           is_active;
  MDS_DEST           mas_vcard;
  NCS_BOOL           mas_here;

  MDS_DEST           psr_vcard;
  NCS_BOOL           psr_here;

  /* Create time constants */

  uns8               hm_poolid;
  uns32              hm_hdl;
  char*              logf_name;
  MDS_HDL            mds_hdl;
  NCS_VRID           vrid; 
  SYSF_MBX*          mbx;           
  MAB_LM_CB          lm_cbfnc;

  } OAC_TBL;


/************************************************************************

  O A C   P r i v a t e   F u n c t i o n   P r o t o t y p e s

*************************************************************************/
/************************************************************************
  MAC tasking loop function
*************************************************************************/

EXTERN_C MABOAC_API void      oac_do_evts     ( SYSF_MBX*     mbx     );
EXTERN_C uns32     oac_do_evt      ( MAB_MSG*      msg     );

/************************************************************************
                 Entry points for the SubSytems
************************************************************************/

EXTERN_C uns32     oac_ss_tbl_reg  (NCSOAC_TBL_OWNED* tbl_owned,uns32 tbl_id, 
                                    uns32 oac_hdl);
EXTERN_C uns32     oac_ss_tbl_unreg(NCSOAC_TBL_GONE*  tbl_gone, uns32 tbl_id, 
                                    uns32 oac_hdl);
EXTERN_C uns32     oac_ss_row_reg  (NCSOAC_ROW_OWNED* row_owned,uns32 tbl_id, 
                                    uns32 oac_hdl);
EXTERN_C uns32     oac_ss_row_unreg(NCSOAC_ROW_GONE*  row_gone, uns32 tbl_id, 
                                    uns32 oac_hdl);
EXTERN_C uns32     oac_ss_row_move (NCSOAC_ROW_MOVE*  row_move, uns32 oac_hdl);
EXTERN_C uns32     oac_ss_warmboot_req_to_pssv(NCSOAC_SS_ARG *arg);
EXTERN_C uns32     oac_ss_push_mibarg_data_to_pssv(NCSOAC_SS_ARG *arg);
EXTERN_C uns32     oac_handle_svc_mds_evt(MAB_MSG * msg);
EXTERN_C uns32     oac_handle_pss_eop_ind(MAB_MSG * msg);

/************************************************************************
  Basic OAC Layer Management Service entry points off std LM API
*************************************************************************/

EXTERN_C uns32     oac_svc_create  ( NCSOAC_CREATE* create  );
EXTERN_C uns32     oac_svc_destroy ( NCSOAC_DESTROY* destroy);
EXTERN_C uns32     oac_svc_get     ( NCSOAC_GET*    get     );
EXTERN_C uns32     oac_svc_set     ( NCSOAC_SET*    set     );

/*****************************************************************************
 OAC Table Entry manipulation routines 
*****************************************************************************/

EXTERN_C void         oac_tbl_rec_init (OAC_TBL* inst                      );
EXTERN_C void         oac_tbl_rec_clr  (OAC_TBL* inst                      );
EXTERN_C void         oac_tbl_rec_put  (OAC_TBL* inst, OAC_TBL_REC* tbl_rec);
EXTERN_C OAC_TBL_REC* oac_tbl_rec_rmv  (OAC_TBL* inst, uns32        tbl_id );
EXTERN_C OAC_TBL_REC* oac_tbl_rec_find (OAC_TBL* inst, uns32        tbl_id );  
EXTERN_C uns32        oac_move_row_fltr_delete
                                       (OAC_TBL* inst, NCSMIB_ARG*   arg    );
/*****************************************************************************
 OAC Filter manipulation routines 
*****************************************************************************/

EXTERN_C void         oac_fltr_put           (OAC_FLTR** head, OAC_FLTR*       new_fltr);
EXTERN_C OAC_FLTR*    oac_fltr_rmv           (OAC_FLTR** head, uns32            fltr_id);
EXTERN_C OAC_FLTR*    oac_fltr_find          (OAC_FLTR** head, uns32            fltr_id);
EXTERN_C OAC_FLTR*    oac_fltr_create        (OAC_TBL* inst, NCSMAB_FLTR*       mab_fltr);
EXTERN_C uns32        oac_fltr_reg_xmit      (OAC_TBL* inst,OAC_FLTR* fltr,uns32 tbl_id);
EXTERN_C uns32        oac_fltr_unreg_xmit    (OAC_TBL* inst,uns32 fltr_id,uns32  tbl_id);
EXTERN_C OAC_FLTR* oac_fltrs_exact_fltr_locate(OAC_FLTR *fltr_list, NCSMAB_EXACT *exact); 

EXTERN_C void         oac_sync_fltrs_with_mas(OAC_TBL* inst                            );

EXTERN_C uns32 oac_psr_send_bind_req(OAC_TBL *inst, MAB_PSS_TBL_BIND_EVT *bind_req, NCS_BOOL free_head);
EXTERN_C uns32 oac_psr_send_unbind_req(OAC_TBL *inst, uns32 tbl_id);
EXTERN_C void oac_del_pcn_from_list(OAC_TBL *inst, uns32 tbl_id);
EXTERN_C OAA_PCN_LIST 
*oac_findadd_pcn_in_list(OAC_TBL *inst, char *pcn, NCS_BOOL is_system_client,
                         uns32 tbl_id, NCS_BOOL add);
EXTERN_C uns32 oac_add_warmboot_req_in_wbreq_list(OAC_TBL *inst, NCSOAC_PSS_WARMBOOT_REQ *req,
                                         OAA_WBREQ_PEND_LIST **o_wbreq_node);
EXTERN_C uns32 oac_validate_warmboot_req(OAC_TBL* inst, NCSOAC_PSS_WARMBOOT_REQ *req);
EXTERN_C uns32 oac_send_pending_warmboot_reqs_to_pssv(OAC_TBL* inst);
EXTERN_C void oac_free_pss_tbl_list(MAB_PSS_TBL_LIST *list);
EXTERN_C NCS_BOOL oac_validate_pcn(OAC_TBL *inst, char *pcn);
EXTERN_C uns32 oac_convert_input_wbreq_to_mab_request(OAC_TBL* inst,
            NCSOAC_PSS_WARMBOOT_REQ *in_wbreq, MAB_PSS_WARMBOOT_REQ *req);
EXTERN_C void oac_free_wbreq(MAB_PSS_WARMBOOT_REQ *head);
EXTERN_C uns32 oac_gen_tbl_bind(OAA_PCN_LIST *pcn_list, 
                                MAB_PSS_TBL_BIND_EVT **bind);
EXTERN_C void oac_free_bind_req_list(MAB_PSS_TBL_BIND_EVT *bind_list);
EXTERN_C uns32 oac_psr_add_to_buffer_zone(OAC_TBL *inst, MAB_MSG* msg);
EXTERN_C uns32 oac_handle_pss_ack(MAB_MSG * msg);
EXTERN_C void oac_free_buffer_zone_entry(OAA_BUFR_HASH_LIST *ptr);
EXTERN_C void oac_free_buffer_zone_entry_msg(MAB_MSG *msg);
EXTERN_C uns32 oac_send_buffer_zone_evts(OAC_TBL *inst);
EXTERN_C uns32 oac_dup_warmboot_req(MAB_PSS_WARMBOOT_REQ *in, MAB_PSS_WARMBOOT_REQ *out);
EXTERN_C void oac_refresh_table_bind_to_pssv(OAC_TBL* inst);


/*****************************************************************************
 MAB message handling  routines 
*****************************************************************************/

EXTERN_C uns32 oac_mas_fltrs_refresh (MAB_MSG* msg);
EXTERN_C uns32 oac_mib_request       (MAB_MSG* msg);
EXTERN_C uns32 oac_mab_response      (MAB_MSG* msg);
EXTERN_C uns32 oac_free_idx_request  (MAB_MSG* msg);
EXTERN_C uns32 oac_sa_mib_request    (MAB_MSG* msg);
EXTERN_C uns32 oac_playback_start    (MAB_MSG* msg);
EXTERN_C uns32 oac_playback_end      (MAB_MSG* msg);


/************************************************************************
  MDS bindary stuff for MAC
*************************************************************************/
EXTERN_C uns32 oac_mds_cb (NCSMDS_CALLBACK_INFO *cbinfo);

EXTERN_C  uns32
oaclib_oac_destroy(NCS_LIB_REQ_INFO * req_info); 
/************************************************************************

  O b j e c t   A c e s s   C l i e n t    L o c k s

 The OAC locks follow the lead of the master MIB Access Broker setting
 to determine what type of lock to implement.

*************************************************************************/


#if (NCSMAB_USE_LOCK_TYPE == MAB_NO_LOCKS)                  /* NO Locks */

#define m_OAC_LK_CREATE(lk)
#define m_OAC_LK_INIT
#define m_OAC_LK(lk) 
#define m_OAC_UNLK(lk) 
#define m_OAC_LK_DLT(lk) 

#elif (NCSMAB_USE_LOCK_TYPE == MAB_TASK_LOCKS)            /* Task Locks */

#define m_OAC_LK_CREATE(lk)
#define m_OAC_LK_INIT            m_INIT_CRITICAL
#define m_OAC_LK(lk)             m_START_CRITICAL
#define m_OAC_UNLK(lk)           m_END_CRITICAL
#define m_OAC_LK_DLT(lk) 

#elif (NCSMAB_USE_LOCK_TYPE == MAB_OBJ_LOCKS) /* Object Locks */


#define m_OAC_LK_CREATE(lk)      m_NCS_LOCK_INIT_V2(lk,NCS_SERVICE_ID_MAB, \
                                                    OAC_LOCK_ID)
#define m_OAC_LK_INIT
#define m_OAC_LK(lk)             m_NCS_LOCK_V2(lk,   NCS_LOCK_WRITE, \
                                                    NCS_SERVICE_ID_MAB, OAC_LOCK_ID)
#define m_OAC_UNLK(lk)           m_NCS_UNLOCK_V2(lk, NCS_LOCK_WRITE, \
                                                    NCS_SERVICE_ID_MAB, OAC_LOCK_ID)
#define m_OAC_LK_DLT(lk)         m_NCS_LOCK_DESTROY_V2(lk, NCS_SERVICE_ID_MAB, \
                                                    OAC_LOCK_ID)

#endif 



#endif /* OAC_PVT_H */
