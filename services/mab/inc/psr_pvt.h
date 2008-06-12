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

  _Private_ Persistent Store Restore service (PSSv) abstractions and 
  function prototypes.

*******************************************************************************/

/*
 * Module Inclusion Control...
 */


#ifndef PSR_PVT_H
#define PSR_PVT_H

#include "ncs_mib_pub.h"
#include "ncs_edu_pub.h"
#include "psr_def.h"
#if (NCS_PSS_RED == 1)
#include "pss_mbcsv.h"
#endif

#define NCS_PSS_MAX_CHUNK_SIZE   1024
#define NCS_PSS_MAX_PROFILE_DESC 256
#define NCS_PSS_UNBOUNDED_OCTET_SOFT_LIMITED_SIZE 0x200
#define NCS_PSS_MAX_IN_STORE_MEM_SIZE 2300000 /* approx. 2.2 MB */

/* Bounds on the rank of the MIB Tables */
#define NCSPSS_MIB_TBL_RANK_MIN  NCSMIB_TBL_RANK_MIN
#define NCSPSS_MIB_TBL_RANK_MAX  NCSMIB_TBL_RANK_MAX
#define NCSPSS_MIB_TBL_RANK_DEFAULT NCSMIB_TBL_RANK_DEFAULT
#define PSS_MIB_TBL_NAME_LEN_MAX 80
#define PSS_VAR_NAME_LEN_MAX     80

#define m_PSS_SPCN_SOURCE_BAM           "XML"
#define m_PSS_SPCN_SOURCE_PSSV          "PSS"

#define m_PSS_COMP_NAME_FILE "/var/opt/opensaf/ncs_pss_comp_name"
#define m_PSS_PID_FILE "/var/run/ncs_psr.pid"

typedef enum {
   PSS_RET_NORMAL = 1,
   PSS_RET_MIB_DESC_NULL,
   PSS_RET_TBL_ID_BEYOND_MAX_ID,
   PSS_RET_MAX
}PSS_RET_VAL;

typedef enum pss_ckpt_state_type
{
   PSS_COLD_SYNC_IDLE,
   PSS_COLD_SYNC_DATA_RESP_FAIL,
   PSS_COLD_SYNC_COMPLETE,
   PSS_COLD_SYNC_FAIL,
   PSS_WARM_SYNC_COMPLETE,
   PSS_WARM_SYNC_WAIT_FOR_DATA_RESP,
   PSS_WARM_SYNC_DATA_RESP_FAIL,
   PSS_WARM_SYNC_FAIL
}PSS_CKPT_STATE;

/* Fix for IR00085164 */
typedef enum pss_stdby_oaa_down_buffer_op
{
    PSS_STDBY_OAA_DOWN_BUFFER_ADD,
    PSS_STDBY_OAA_DOWN_BUFFER_DELETE
}PSS_STDBY_OAA_DOWN_BUFFER_OP;

struct pss_csi_node; 

/* data structure for going through the tables in rank order */
typedef struct pss_mib_tbl_rank
{
    uns32   tbl_id; /* Table id */
    struct  pss_mib_tbl_rank * next; /* The next table with the same rank */
} PSS_MIB_TBL_RANK;


typedef struct pss_var_info
{
    uns32   offset;      /* This value is calculated at registration time.
                        This value is the sum of the lengths of all preceding settable fields. */
    NCS_BOOL var_length;  /* This field is set to true if this parameter is of variable length. */
    NCSMIB_VAR_INFO var_info; /* The registered information about the MIB object*/
    char     var_name[PSS_VAR_NAME_LEN_MAX];

} PSS_VAR_INFO;


typedef struct pss_ncsmib_tbl_info
{
   NCS_BOOL       table_of_scalars;
   uns8           table_rank;
   uns16          num_objects;
   uns16          status_field;
   char           mib_tbl_name[PSS_MIB_TBL_NAME_LEN_MAX]; /* Assuming max 80 characters */
   uns16          *objects_local_to_tbl; /* Set only for Augmented tables. First location would be
                                            the number of local objects. */
   uns16          *idx_data; /* Relative-indices of MIB-index. First location would be 
                                            the number of index-elements. */
} PSS_NCSMIB_TBL_INFO;

typedef struct pss_mib_tbl_info
{
    NCSMIB_CAPABILITY capability; /* MIB's playback event capability */
    uns32 max_row_length; /* Maximum size of all the settable elements in the row */
    uns32 max_key_length; /* Total key length in bytes */
    uns32 num_inst_ids;   /* Max number of inst ids */
    uns32 bitmap_length;  /* Length of the preceding bitmap. depends on the number of settable fields */
    PSS_NCSMIB_TBL_INFO *ptbl_info; /* Contains registered table info */
    PSS_VAR_INFO * pfields; /* An array containing details of each field */
    uns32 i_tbl_version;      /* version of this table.  Default value is 1 */ 

} PSS_MIB_TBL_INFO;

typedef struct pss_mib_tbl_data
{
    NCS_PATRICIA_NODE pat_node; /* This must be the first entry in this structure */
    NCS_BOOL deleted : 1; /* This flag is set if this row is deleted */
    NCS_BOOL entry_on_disk : 1; /* Whether a corresponding entry exists on the persistent store */
    NCS_BOOL dirty : 1;   /* This flag is set if there is a change in data in this row */
    uns8 *  key;
    uns8 *  data;
} PSS_MIB_TBL_DATA;


/* Data structures for storing the in-memory configuration profile */
typedef struct pss_client_key
{
    char             pcn[NCSMIB_PCN_LENGTH_MAX];
}PSS_CLIENT_KEY;
typedef struct pss_tbl_rec_tag
{
    uns32                tbl_id; /* Table identifier              */
    NCS_BOOL             dirty:1;  /* TRUE if any changes have been
                                  * made to the patricia tree     */
    NCS_BOOL             is_scalar:1; /* TRUE for scalar table */
    PSS_CLIENT_KEY       *pss_client_key;   /* Pointer to the key in 
                                               "client_table" */
    struct pss_oaa_entry_tag  *p_oaa_entry;  /* Reverse-map to which OAA we belong to */

    union {
        struct {
            NCS_BOOL                tree_inited;  /* Is this tree initialized or not? */
            NCS_PATRICIA_TREE       data;   /* The actual data for this tree */
            NCS_PATRICIA_PARAMS     params; /* The root node for this tree   */
        }other;
        struct {
            uns8                    *data;  /* No Index required. */
            uns32                   row_len;
        }scalar;
    }info;

    struct pss_tbl_rec_tag * next;   /* The next table in the list    */
} PSS_TBL_REC;
typedef struct pss_client_entry_tag
{
    NCS_PATRICIA_NODE pat_node; /* This must be the first entry in this structure */

    PSS_CLIENT_KEY key;

    PSS_TBL_REC *hash[MAB_MIB_ID_HASH_TBL_SIZE]; /* hash table containing lists of tables */

    uns16       tbl_cnt; /* Number of MIB tables registered from this client */
} PSS_CLIENT_ENTRY;


typedef struct pss_qelem
{
    NCS_QELEM elem;
    uns32    tbl_id;
    uns8 *   data;
} PSS_QELEM;


typedef struct pss_oaakey_tag
{
    MDS_DEST    mds_dest;
} PSS_OAA_KEY;


typedef struct pss_oaa_clt_id_tag
{
    PSS_TBL_REC     *tbl_rec;
    struct pss_oaa_clt_id_tag *next;
} PSS_OAA_CLT_ID;


typedef struct pss_oaa_entry_tag
{
    NCS_PATRICIA_NODE  node;

    PSS_OAA_KEY        key;
    PSS_OAA_CLT_ID     *hash[MAB_MIB_ID_HASH_TBL_SIZE];

    uns16       tbl_cnt; /* Number of MIB tables registered from this address */
} PSS_OAA_ENTRY;


typedef struct pss_rfrsh_tbl_list_tag
{
    PSS_TBL_REC         *pss_tbl_rec;
    struct pss_rfrsh_tbl_list_tag *next;
}PSS_RFRSH_TBL_LIST;

typedef struct pss_spcn_wbreq_pend_list_tag
{
   char        *pcn;

   struct pss_spcn_wbreq_pend_list_tag *next;
}PSS_SPCN_WBREQ_PEND_LIST;


/* Require forward declaration here. */
struct pss_cb_tag;
struct pss_stdby_oaa_down_buffer_node;

typedef struct pss_pwe_cb_tag
{
   /* Environement ID */ 
   PW_ENV_ID         pwe_id;
   NCS_BOOL          is_mas_alive;
   MDS_HDL           mds_pwe_handle; 
   uns32             mac_key;
   uns32             hm_hdl;
 
   /* Checkpointing and redundancy related data */
   SaInvocationT       amf_invocation_id;

#if (NCS_PSS_RED == 1)
   NCS_MBCSV_CKPT_HDL          ckpt_hdl;

   PSS_CKPT_STATE              ckpt_state;

   NCS_BOOL                    is_cold_sync_done;   /* Default is FALSE. */

   NCS_BOOL                    warm_sync_in_progress;

   uns32                       async_cnt;

   struct pss_curr_plbck_ssn_info_tag curr_plbck_ssn_info;

   /* Standby PSS responds back to AMF with this invocation-id
    * once the cold-sync is complete.
    */
   NCS_BOOL            is_amfresponse_for_active_pending;   /* Default is FALSE. This would be set to TRUE,
                                  when CSI assignment to ACTIVE happens before cold-sync is done.
                                  On completion of cold-sync, if this boolean is TRUE, PSS need invoke the AMFresponse callback
                                  for role-assignment to ACTIVE. */

   NCS_BOOL                    processing_pending_active_events; /* like
                                  playback sessions, etc. */

   /* Fix for IR00085164 */
   struct pss_stdby_oaa_down_buffer_node       *pss_stdby_oaa_down_buffer;

#endif


   NCS_PATRICIA_PARAMS         params_oaa;

   /* Contains the BAM reachability information, and the list of
      clients whose warmboot playback from BAM is pending. */
   MDS_HDL             bam_pwe_hdl;  /* To store PWE-specific local handle */
   PSS_SPCN_WBREQ_PEND_LIST *spcn_wbreq_pend_list; /* Can be PWE-specific also */

   /*
    * Contains a list of OAA-MDS addresses from where MIB tables 
    * were registered with PSS.  
    */
   NCS_PATRICIA_TREE           oaa_tree;    
    
   NCS_PATRICIA_PARAMS         params_client;

   /* Contains the MIB SETs per client(PCN) received from each client. */
   NCS_PATRICIA_TREE           client_table;    

   /* List of MIB tables that got added/updated recently. */
   PSS_RFRSH_TBL_LIST          *refresh_tbl_list; 

   SYSF_MBX                    *mbx; 

   /* Pointer to PSS_CB main block */
   struct pss_cb_tag           *p_pss_cb;  
}PSS_PWE_CB;


typedef struct pss_pwe_info_tag
{
    PSS_PWE_CB          *pwe_cb;
}PSS_PWE_INFO;


typedef struct pss_spcn_list_tag
{
   char        *pcn;
   
   /* Whether to playback from BAM or not? */
   NCS_BOOL    plbck_frm_bam;      

   struct pss_spcn_list_tag *next;

}PSS_SPCN_LIST;

typedef struct pss_cb_tag
{

  NCS_LOCK            lock;

  /* Keep track of MAS and BAM instances */
  NCS_BOOL            is_bam_alive;
  MDS_DEST            bam_address; 

  /* Configuration files */
  char                lib_conf_file[NCS_PSS_CONF_FILE_NAME];
  char                spcn_list_file[NCS_PSS_CONF_FILE_NAME];

  /* Used for MIB operations */
  NCSMIB_IDX          mib_idx;
  uns8                profile_extract[NCS_PSS_MAX_PROFILE_NAME];

  PSS_SPCN_LIST       *spcn_list;

  SaAmfHAStateT       ha_state;
  uns16               version;
  EDU_HDL             edu_hdl;

  /*  contains MIB table descriptions */
  PSS_MIB_TBL_INFO *  mib_tbl_desc[MIB_UD_TBL_ID_END]; 
  PSS_MIB_TBL_RANK *  mib_tbl_rank[MIB_UD_TBL_ID_END]; 

  uns8                existing_profile[NCS_PSS_MAX_PROFILE_NAME];
  uns8                new_profile[NCS_PSS_MAX_PROFILE_NAME];
  uns8                current_profile[NCS_PSS_MAX_PROFILE_NAME];

  /* run-time learned values */
  NCSMIB_REQ_FNC      mib_req_func;
  MDS_DEST            my_vcard;
  PSS_SAVE_TYPE       save_type;
  uns32               profile_tbl_row_hdl;
  uns32               trigger_scl_row_hdl;
  uns32               cmd_tbl_row_hdl;
  uns32               oac_key;
  uns32               xch_id;

  /* Create time constants */
  MDS_HDL             mds_hdl;
  MAB_LM_CB           lm_cbfnc;
  NCS_PSSTS           pssts_api;
  uns8                hmpool_id;
  uns32               hm_hdl;
  uns32               pssts_hdl;

  SYSF_MBX            *mbx; 

  uns32               mem_in_store; /* denotes the memory allocated to store
                                     MIB SET(ROW/ALLROWS) in memory */
  uns32               bam_req_cnt;
} PSS_CB;

/* Used only during warmboot playback */
typedef struct pss_wbsort_key_tag
{
    uns8 rank;
    uns32 tbl_id;
} PSS_WBSORT_KEY;

typedef struct pss_wbsort_entry
{
    NCS_PATRICIA_NODE  pat_node;

    PSS_WBSORT_KEY       key;
    PSS_TBL_REC        *tbl_rec;
} PSS_WBSORT_ENTRY;

typedef struct pss_wbplayback_sort_table_tag
{
    NCS_PATRICIA_PARAMS sort_params;
    NCS_PATRICIA_TREE   sort_db;
    uns16               num_entries;
}PSS_WBPLAYBACK_SORT_TABLE;

/* Used only during on-demand playback */
typedef struct pss_odsort_key_tag
{
    uns8 rank;
    uns32 tbl_id;
} PSS_ODSORT_KEY;
typedef struct pss_odsort_tbl_rec_tag
{
    PSS_TBL_REC * tbl_rec;
    struct pss_odsort_tbl_rec_tag *next;
}PSS_ODSORT_TBL_REC;
typedef struct pss_odsort_entry
{
    NCS_PATRICIA_NODE  pat_node;

    PSS_ODSORT_KEY       key;
    PSS_ODSORT_TBL_REC   *tbl_rec_list;
} PSS_ODSORT_ENTRY;

typedef struct pss_odplayback_sort_table_tag
{
    NCS_PATRICIA_PARAMS sort_params;
    NCS_PATRICIA_TREE   sort_db;
    uns16               num_entries;
}PSS_ODPLAYBACK_SORT_TABLE;

/* Fix for IR00085164 */
typedef struct pss_stdby_oaa_down_buffer_node
{
    MDS_DEST           oaa_addr; /* oaa  address */
    
    /* pointer to PSS_OAA_ENTRY node in the oaa_tree for fr_card address */
    PSS_OAA_ENTRY      *oaa_node; 

    struct pss_stdby_oaa_down_buffer_node *next;
}PSS_STDBY_OAA_DOWN_BUFFER_NODE;

#define PSS_CB_NULL ((PSS_CB*)0)

/************************************************************************

  P S R   P r i v a t e   F u n c t i o n   P r o t o t y p e s

*************************************************************************/

/************************************************************************
  PSS Redundancy macros and functions
*************************************************************************/
#if (NCS_PSS_RED == 1)
#define m_PSS_RE_TBL_BIND_SYNC(pwe_cb, msg) \
{ \
   pwe_cb->async_cnt ++; \
   if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) \
      pss_re_sync(pwe_cb, msg, PSS_CKPT_DATA_TBL_BIND); \
}

#define m_PSS_RE_TBL_UNBIND_SYNC(pwe_cb, msg) \
{ \
   pwe_cb->async_cnt ++; \
   if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) \
      pss_re_sync(pwe_cb, msg, PSS_CKPT_DATA_TBL_UNBIND); \
}

#define m_PSS_RE_OAA_DOWN_SYNC(pwe_cb, msg) \
{ \
   pwe_cb->async_cnt ++; \
   if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) \
      pss_re_sync(pwe_cb, msg, PSS_CKPT_DATA_OAA_DOWN); \
}

#define m_PSS_RE_BAM_CONF_DONE(pwe_cb, msg) \
{ \
   pwe_cb->async_cnt ++; \
   if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) \
      pss_re_sync(pwe_cb, msg, PSS_CKPT_DATA_BAM_CONF_DONE); \
}

#define m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, msg) \
{ \
   pwe_cb->async_cnt ++; \
   if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) \
      pss_re_direct_sync(pwe_cb, msg, PSS_CKPT_DATA_PLBCK_SSN_INFO, \
                         NCS_MBCSV_ACT_ADD); \
}

#define m_PSS_RE_BAM_PEND_WBREQ_INFO(pwe_cb, ppcn, act_type) \
{ \
   pwe_cb->async_cnt ++; \
   if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) \
      pss_re_direct_sync(pwe_cb, ppcn, PSS_CKPT_DATA_PEND_WBREQ_TO_BAM, \
                         act_type);\
}

#define m_PSS_RE_RELOAD_PSSVLIBCONF(pwe_cb) \
{ \
   PSS_CKPT_MSG lcl_msg; \
   m_NCS_MEMSET(&lcl_msg, '\0', sizeof(lcl_msg)); \
   pwe_cb->async_cnt ++; \
   lcl_msg.ckpt_data.reload_pssvlibconf.lib_conf_file = (char*)&pwe_cb->p_pss_cb->lib_conf_file; \
   if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) \
      pss_re_direct_sync(pwe_cb, &lcl_msg, PSS_CKPT_DATA_RELOAD_PSSVLIBCONF, \
                         NCS_MBCSV_ACT_ADD);\
}

#define m_PSS_RE_RELOAD_PSSVSPCNLIST(pwe_cb) \
{ \
   PSS_CKPT_MSG lcl_msg; \
   m_NCS_MEMSET(&lcl_msg, '\0', sizeof(lcl_msg)); \
   pwe_cb->async_cnt ++; \
   lcl_msg.ckpt_data.reload_pssvspcnlist.spcn_list = pwe_cb->p_pss_cb->spcn_list; \
   if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) \
      pss_re_direct_sync(pwe_cb, &lcl_msg, PSS_CKPT_DATA_RELOAD_PSSVSPCNLIST, \
                         NCS_MBCSV_ACT_ADD);\
}

#else
#define m_PSS_RE_TBL_BIND_SYNC(pwe_cb, msg) 
#define m_PSS_RE_TBL_UNBIND_SYNC(pwe_cb, msg) 
#define m_PSS_RE_OAA_DOWN_SYNC(pwe_cb, msg)
#define m_PSS_RE_BAM_CONF_DONE(pwe_cb, msg)
#define m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, msg)
#define m_PSS_RE_BAM_PEND_WBREQ_INFO(pwe_cb, pcn, act_type)
#define m_PSS_RE_RELOAD_PSSVLIBCONF(pwe_cb)
#define m_PSS_RE_RELOAD_PSSVSPCNLIST(pwe_cb)
#endif


/************************************************************************
  PSS Target Service Instance function
*************************************************************************/
EXTERN_C MABPSR_API uns32 pss_ts_create(NCSPSS_CREATE *, uns32 ps_format_version);

/************************************************************************
  PSS tasking loop function
*************************************************************************/

EXTERN_C MABPSR_API uns32      pss_do_evts  ( SYSF_MBX*     mbx     );
EXTERN_C MABPSR_API uns32     pss_do_evt    ( MAB_MSG* msg, NCS_BOOL free_msg);

/************************************************************************
  PSS mailbox functions
*************************************************************************/

EXTERN_C void pss_flush_mbx_evts(SYSF_MBX *mbx);
EXTERN_C NCS_BOOL pss_flush_mbx_msg(void *arg, void *msg);

/************************************************************************
                 MAB Message Processing functions
************************************************************************/

EXTERN_C uns32     pss_process_tbl_bind(MAB_MSG *msg);
EXTERN_C uns32     pss_process_tbl_unbind(MAB_MSG *msg);
EXTERN_C uns32     pss_process_snmp_request(MAB_MSG * msg);
EXTERN_C uns32     pss_process_oac_warmboot(MAB_MSG * msg);
EXTERN_C uns32     pss_process_mib_request (MAB_MSG * msg);
EXTERN_C uns32     pss_handle_svc_mds_evt  (MAB_MSG * msg);
EXTERN_C uns32     pss_handle_oaa_down_event(PSS_PWE_CB *pwe_cb, 
                                             MDS_DEST *fr_card);
EXTERN_C uns32     pss_process_bam_conf_done(MAB_MSG *msg);

/************************************************************************
                 MIB Requests Processing functions
************************************************************************/

EXTERN_C uns32     pss_process_tbl_pss_profiles(PSS_CB * inst,
                                                NCSMIB_ARG * arg);
EXTERN_C uns32     pss_process_sclr_trigger    (PSS_CB * inst,
                                                NCSMIB_ARG * arg);
EXTERN_C uns32     pss_process_tbl_cmd         (PSS_CB * inst, 
                                                NCSMIB_ARG * arg);
EXTERN_C uns32     ncspssvprofiletableentry_get(NCSCONTEXT cb, 
                         NCSMIB_ARG *arg, NCSCONTEXT* data);
EXTERN_C uns32     ncspssvprofiletableentry_extract(NCSMIB_PARAM_VAL* param,
                         NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                         NCSCONTEXT buffer);
EXTERN_C uns32     ncspssvprofiletableentry_extract(NCSMIB_PARAM_VAL* param,
                         NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                         NCSCONTEXT buffer);
EXTERN_C uns32     ncspssvprofiletableentry_set(NCSCONTEXT cb, 
                         NCSMIB_ARG *arg, NCSMIB_VAR_INFO* var_info,
                         NCS_BOOL test_flag);
EXTERN_C uns32     ncspssvprofiletableentry_next(NCSCONTEXT cb, 
                         NCSMIB_ARG *arg, NCSCONTEXT* data,
                         uns32* next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32     ncspssvprofiletableentry_setrow(NCSCONTEXT cb,
                         NCSMIB_ARG* args, 
                         NCSMIB_SETROW_PARAM_VAL* params, 
                         struct ncsmib_obj_info* obj_info,
                         NCS_BOOL testrow_flag);
EXTERN_C uns32     ncspssvscalars_set(NCSCONTEXT cb,
                         NCSMIB_ARG *arg,
                         NCSMIB_VAR_INFO *var_info,
                         NCS_BOOL test_flag);
EXTERN_C uns32     ncspssvscalars_extract(NCSMIB_PARAM_VAL *param,
                         NCSMIB_VAR_INFO *var_info,
                         NCSCONTEXT data,
                         NCSCONTEXT buffer);
EXTERN_C uns32     ncspssvscalars_get(NCSCONTEXT inst, 
                         NCSMIB_ARG *arg,
                         NCSCONTEXT *data);
EXTERN_C uns32     ncspssvscalars_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32     ncspssvscalars_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
EXTERN_C uns32     ncspssvscalars_tbl_reg(void);
EXTERN_C uns32     ncspssvprofiletableentry_tbl_reg(void);

EXTERN_C uns32     pss_process_set_plbck_option_for_spcn(PSS_CB * inst,
                                                    NCSMIB_ARG * arg);
EXTERN_C uns32     pss_update_entry_in_spcn_conf_file(PSS_CB *inst,
                                                      PSS_SPCN_LIST *entry);
EXTERN_C uns32     pss_process_display_mib_entries(PSS_CB * inst,
                                                    NCSMIB_ARG * arg);
EXTERN_C uns32     pss_process_display_profile_entries(PSS_CB * inst,
                                                    NCSMIB_ARG * arg);
EXTERN_C void      pss_process_reload_pssvlibconf(PSS_CB * inst,
                                                    NCSMIB_ARG * arg);
EXTERN_C void      pss_process_reload_pssvspcnlist(PSS_CB * inst, 
                                                    NCSMIB_ARG * arg);
EXTERN_C uns32     pss_process_trigger_op      (PSS_CB * inst,
                                                PSR_TRIGGER_VALUES val);

/************************************************************************
                 Entry points for the SubSytems
************************************************************************/
struct ncspss_tbl_arg_info_tag;
EXTERN_C uns32     pss_ss_tbl_reg  (PSS_CB *inst, struct ncspss_tbl_arg_info_tag *tbl_arg_info);
EXTERN_C uns32     pss_ss_tbl_gone (PSS_CB *inst, uns32 tbl_id);
EXTERN_C uns32     pss_flush_all_mib_tbl_info(PSS_CB *inst);

/************************************************************************
  Basic OAC Layer Management Service entry points off std LM API
*************************************************************************/

EXTERN_C uns32     pss_svc_create  (NCSPSS_CREATE* create);
EXTERN_C uns32     pss_svc_destroy ( uns32 usr_key       );
EXTERN_C void      pss_clean_inst_cb(PSS_CB *inst);
EXTERN_C uns32     pss_pwe_destroy_spcn_wbreq_pend_list(PSS_SPCN_WBREQ_PEND_LIST *spcn_wbreq_pend_list);
EXTERN_C uns32     pss_read_lib_conf_info(PSS_CB *inst, char *conf_file);

/*****************************************************************************
 PSS Table manipulation routines 
*****************************************************************************/

EXTERN_C void      pss_mib_tbl_rank_init(PSS_CB * inst);
EXTERN_C void      pss_mib_tbl_desc_init(PSS_CB * inst);

EXTERN_C void      pss_mib_tbl_rank_dest(PSS_CB * inst);
EXTERN_C void      pss_mib_tbl_desc_dest(PSS_CB * inst);

EXTERN_C uns32     pss_mib_tbl_rank_add(PSS_CB * inst, uns32 tbl_id, uns8 rank);

EXTERN_C void      pss_mac_keys_init(PSS_CB * inst);

/*****************************************************************************
 PSS SPCN list routines
*****************************************************************************/
EXTERN_C uns32 pss_read_create_spcn_config_file(PSS_CB *inst);
EXTERN_C PSS_SPCN_LIST *pss_findadd_entry_frm_spcnlist(PSS_CB *inst, char *p_pcn,
                                              NCS_BOOL add);
EXTERN_C void pss_reset_boolean_in_spcn_entry(PSS_PWE_CB *pwe_cb, 
                  PSS_SPCN_LIST *spcn_entry);
EXTERN_C void pss_destroy_spcn_list(PSS_SPCN_LIST *list);


/*****************************************************************************
 PSS Patricia Tree manipulation routines 
*****************************************************************************/

EXTERN_C void pss_client_entry_init(PSS_CLIENT_ENTRY *client_node);
EXTERN_C PSS_CLIENT_ENTRY *pss_find_client_entry(PSS_PWE_CB *pwe_cb, char *pcn, NCS_BOOL add);

EXTERN_C PSS_OAA_ENTRY *
pss_findadd_entry_in_oaa_tree(PSS_PWE_CB *pss_cb, MDS_DEST *mds_dest, NCS_BOOL add);
EXTERN_C uns32 pss_destroy_pwe_client_table(NCS_PATRICIA_TREE * pTree, NCS_BOOL destroy);
EXTERN_C uns32 pss_destroy_pwe_oaa_tree(NCS_PATRICIA_TREE * pTree, NCS_BOOL destroy);

EXTERN_C char *
pss_get_pcn_from_mdsdest(PSS_PWE_CB *pwe_cb, MDS_DEST *mdest, uns32 tbl_id);

EXTERN_C void      pss_index_tree_destroy(NCS_PATRICIA_TREE * pTree);
EXTERN_C uns32     pss_table_tree_destroy(NCS_PATRICIA_TREE * pTree,
                                    NCS_BOOL retain_tree);
EXTERN_C PSS_TBL_REC * pss_find_table_tree(PSS_PWE_CB *pwe_cb, 
                         PSS_CLIENT_ENTRY *client_node,
                         PSS_OAA_ENTRY *oaa_node, 
                         uns32 tbl_id, NCS_BOOL create,
                         PSS_RET_VAL *o_prc);
EXTERN_C NCS_PATRICIA_NODE * pss_find_inst_node(PSS_PWE_CB *pwe_cb,
                                      NCS_PATRICIA_TREE *pTree,
                                      char *p_pcn, uns32 tbl_id,
                                      uns32 num_inst_ids, uns32 * inst_ids,
                                      NCS_BOOL create);
EXTERN_C PSS_OAA_CLT_ID *pss_add_tbl_in_oaa_node(PSS_PWE_CB *pwe_cb, 
                             PSS_OAA_ENTRY *node, PSS_TBL_REC *trec);
EXTERN_C PSS_OAA_CLT_ID *pss_find_tbl_in_oaa_node(PSS_PWE_CB *pwe_cb, 
                             PSS_OAA_ENTRY *node, uns32 tbl_id);
EXTERN_C uns32 pss_find_scalar_node(PSS_PWE_CB *pwe_cb, PSS_TBL_REC * tbl_rec,
                           char *p_pcn, uns32 tbl_id,
                           NCS_BOOL create, NCS_BOOL *created_new);

/*****************************************************************************
 PSS PWE related routines             
*****************************************************************************/
EXTERN_C MABPSR_API uns32 
pss_pwe_cb_init(uns32 pss_cb_handle, PSS_PWE_CB *pwe_cb, PW_ENV_ID envid); 

EXTERN_C MABPSR_API uns32 
pss_pwe_cb_destroy(PSS_PWE_CB *pwe_cb); 

EXTERN_C MABPSR_API void pss_free_mab_msg(MAB_MSG *msg, NCS_BOOL free_msg);

EXTERN_C MABPSR_API uns32 pss_bam_decode_conf_done(NCS_UBAID *uba, MAB_PSS_BAM_CONF_DONE_EVT *conf_done);

EXTERN_C uns32 pss_send_ack_for_msg_to_oaa(PSS_PWE_CB *pwe_cb, MAB_MSG *msg);
EXTERN_C void pss_send_eop_status_to_oaa(MAB_MSG *mm, uns32 status, uns32 mib_key,
                                         uns32 wbreq_hdl, NCS_BOOL send_all);

/*****************************************************************************
 PSS Patricia Tree manipulation routines 
*****************************************************************************/

EXTERN_C uns32     pss_destroy_clients(PSS_CB *inst);
EXTERN_C void      pss_mib_data_init(PSS_MIB_TBL_DATA * mib_data, uns16 pwe, 
                                     char *svc_name);
EXTERN_C uns32     pss_delete_inst_node_from_tree(NCS_PATRICIA_TREE * pTree,
                                                  NCS_PATRICIA_NODE * pNode);
EXTERN_C uns32     pss_del_data_for_oac(PSS_CB * inst,
                                        MDS_DEST *mds_dest,
                                        PW_ENV_ID env);

/*****************************************************************************
 SNMP Request handling  routines 
*****************************************************************************/

EXTERN_C uns32 pss_process_set_request        (MAB_MSG* msg);
EXTERN_C uns32 pss_process_setrow_request     (MAB_MSG* msg, NCS_BOOL is_move_row);
EXTERN_C uns32 pss_process_setallrows_request (MAB_MSG* msg);
EXTERN_C uns32 pss_process_removerows_request (MAB_MSG* msg);
#if 0
EXTERN_C uns32 pss_process_ncsmib_response     (NCSMIB_ARG * rsp);
#endif
EXTERN_C uns32 pss_validate_set_mib_obj(PSS_MIB_TBL_INFO* tbl_obj,
                                        NCSMIB_PARAM_VAL* param);
EXTERN_C uns32 pss_validate_param_val(NCSMIB_VAR_INFO* var_info,
                                      NCSMIB_PARAM_VAL* param);
EXTERN_C uns32 pss_validate_index(PSS_MIB_TBL_INFO* tbl_obj,
                                  NCSMIB_IDX *idx);

/*****************************************************************************
 Data manipulation routines 
*****************************************************************************/

EXTERN_C NCS_BOOL pss_data_available_for_table(PSS_PWE_CB *pwe_cb, char *p_pcn,
                                      PSS_TBL_REC *trec);
EXTERN_C uns32     pss_apply_changes_to_node(PSS_MIB_TBL_INFO * tbl_info,
                                             PSS_MIB_TBL_DATA * pData,
                                             NCSMIB_PARAM_VAL * param);
EXTERN_C uns32 pss_apply_changes_to_sclr_node(PSS_MIB_TBL_INFO * tbl_info,
                                PSS_TBL_REC *tbl_rec, NCS_BOOL created_new,
                                NCSMIB_PARAM_VAL * param);
EXTERN_C uns32 pss_find_scalar_node(PSS_PWE_CB *pwe_cb, PSS_TBL_REC * tbl_rec,
                           char *p_pcn, uns32 tbl_id,
                           NCS_BOOL create, NCS_BOOL *created_new);
EXTERN_C uns8      pss_get_bit_for_param(uns8 * buffer, NCSMIB_PARAM_ID param);
EXTERN_C uns32     pss_get_count_of_valid_params(uns8* buffer, PSS_MIB_TBL_INFO* tbl_info);
EXTERN_C void      pss_set_bit_for_param(uns8 * buffer, NCSMIB_PARAM_ID param);
EXTERN_C uns32     pss_get_inst_ids_from_data(PSS_MIB_TBL_INFO * tbl_info,
                                              uns32 * pinst_ids,
                                              uns8 * buffer);
EXTERN_C uns32     pss_apply_inst_ids_to_node(PSS_MIB_TBL_INFO * tbl_info,
                                              PSS_MIB_TBL_DATA * tbl_data,
                                              uns32 num_inst_ids,
                                              uns32 * inst_ids);
EXTERN_C uns32     pss_set_key_from_inst_ids(PSS_MIB_TBL_INFO * tbl_info,
                                             uns8 * key, uns32 num_inst_ids,
                                             uns32 * inst_ids);
EXTERN_C uns32     pss_get_key_from_data(PSS_MIB_TBL_INFO * tbl_info,
                                         uns8 * pkey, uns8 * buffer);
EXTERN_C void      pss_get_profile_from_inst_ids(uns8 * profile_name,
                                                 uns32 num_inst_ids,
                                                 const uns32 * inst_ids);

/*****************************************************************************
 PSS Playback routines 
*****************************************************************************/

EXTERN_C uns32 pss_send_wbreq_to_bam(PSS_PWE_CB *pwe_cb, char *pcn);
EXTERN_C uns32 pss_add_entry_to_spcn_wbreq_pend_list(PSS_PWE_CB *pwe_cb, char *pcn);
EXTERN_C void pss_del_entry_frm_spcn_wbreq_pend_list(PSS_PWE_CB *pwe_cb, char *pcn);
EXTERN_C uns32 pss_send_pending_wbreqs_to_bam(PSS_PWE_CB *pwe_cb);
EXTERN_C void pss_free_wbreq_list(MAB_MSG * msg);
EXTERN_C void pss_free_tbl_list(MAB_PSS_TBL_LIST *tbl_list);
EXTERN_C uns32 pss_sort_wbreq_instore_tables_with_rank(PSS_PWE_CB *pwe_cb,
                                         PSS_WBPLAYBACK_SORT_TABLE *sortdb,
                                         PSS_CLIENT_ENTRY *client_node,
                                         PSS_SPCN_LIST *spcn_entry,
                                         NCS_BOOL *snd_evt_to_bam,
                                         NCS_UBAID *uba);
EXTERN_C uns32 pss_sort_wbreq_tables_with_rank(PSS_PWE_CB *pwe_cb,
                                PSS_WBPLAYBACK_SORT_TABLE *sortdb,
                                PSS_CLIENT_ENTRY *client_node,
                                PSS_SPCN_LIST *spcn_entry,
                                MAB_PSS_TBL_LIST *wbreq_head,
                                NCS_BOOL *snd_evt_to_bam,
                                NCS_UBAID *uba);
EXTERN_C uns32 pss_sort_odreq_instore_tables_with_rank(PSS_PWE_CB *pwe_cb,
                                PSS_ODPLAYBACK_SORT_TABLE *sortdb);
EXTERN_C uns32 pss_ondemand_playback_for_sorted_list(PSS_PWE_CB *pwe_cb,
                                         PSS_ODPLAYBACK_SORT_TABLE *sortdb,
                                         uns8* profile);
EXTERN_C uns32 pss_wb_playback_for_sorted_list(PSS_PWE_CB *pwe_cb, 
                                PSS_WBPLAYBACK_SORT_TABLE *sort_db,
                                PSS_CLIENT_ENTRY *node, char *p_pcn);
EXTERN_C uns32     pss_signal_start_of_playback(PSS_PWE_CB * inst, USRBUF *ub);
EXTERN_C uns32     pss_signal_end_of_playback  (PSS_PWE_CB * inst);
EXTERN_C uns32     
pss_broadcast_playback_signal(struct pss_csi_node *csi_list, NCS_BOOL is_start);
EXTERN_C uns32 pss_oac_warmboot_process_tbl(PSS_PWE_CB *pwe_cb, char *p_pcn,
                                   PSS_TBL_REC *tbl_rec,
                                   PSS_CLIENT_ENTRY *node,
#if (NCS_PSS_RED == 1)
                                   NCS_BOOL *i_continue_session,
#endif
                                   NCS_BOOL is_end_of_playback);
EXTERN_C uns32 pss_oac_warmboot_process_sclr_tbl(PSS_PWE_CB *pwe_cb, char *pcn,
                                        PSS_TBL_REC *tbl_rec,
                                        PSS_CLIENT_ENTRY *node,
#if (NCS_PSS_RED == 1)
                                        NCS_BOOL *i_continue_session,
#endif
                                        NCS_BOOL is_end_of_playback);
EXTERN_C uns32     
pss_on_demand_playback(PSS_CB * inst, struct pss_csi_node *csi_lsit, uns8 * profile);

EXTERN_C uns32 pss_playback_process_tbl(PSS_PWE_CB *pwe_cb, uns8 *profile,
                               PSS_TBL_REC *rec, NCS_QUEUE * add_q,
                               NCS_QUEUE * chg_q, 
#if (NCS_PSS_RED == 1)
                               NCS_BOOL *i_continue_session,
#endif
                               NCS_BOOL last_plbck_evt);
EXTERN_C uns32 pss_playback_process_sclr_tbl(PSS_PWE_CB *pwe_cb, uns8 *profile,
                                    PSS_TBL_REC * tbl_rec,
#if (NCS_PSS_RED == 1)
                                    NCS_BOOL *i_continue_session,
#endif
                                    NCS_BOOL last_plbck_evt);
EXTERN_C uns32 pss_playback_process_tbl_curprofile(PSS_PWE_CB *pwe_cb, 
                                    NCS_PATRICIA_TREE * pTree,
                                    NCS_PATRICIA_NODE * pNode, uns8 * cur_key,
                                    uns8 * cur_data, uns32 cur_rows_left,
                                    uns8 * cur_buf, uns8 * cur_ptr,
                                    long cur_file_hdl, uns32 buf_size,
                                    uns32 cur_file_offset,
                                    NCSMIB_IDX * first_idx, NCSMIB_IDX * idx,
#if (NCS_PSS_RED == 1)
                                    uns32 *pinst_re_ids,
                                    NCS_BOOL *i_continue_session,
#endif
                                    NCSREMROW_AID * rra, NCS_BOOL * rra_inited,
                                    uns32 tbl, uns16 pwe, char *p_pcn,
                                    NCS_BOOL cur_file_exists);
EXTERN_C uns32 pss_playback_process_queue(PSS_PWE_CB *pwe_cb, 
#if (NCS_PSS_RED == 1)
                                    NCS_BOOL *i_continue_session,
#endif
                                    NCS_QUEUE *add_queue, NCS_QUEUE *diff_queue);
EXTERN_C uns32     pss_playback_process_diff_q (PSS_CB * inst, NCS_QUEUE * diff_q,
                                                NCS_BOOL is_add, uns16 pwe);
EXTERN_C void      pss_destroy_queue           (NCS_QUEUE * diff_q);
EXTERN_C uns32 pss_add_to_diff_q (NCS_QUEUE * diff_q, uns32 tbl,
                         char *p_pcn, uns8 * data, uns32 len);
EXTERN_C void  pss_destroy_wbsort_db(PSS_WBPLAYBACK_SORT_TABLE *wbsort_db);
EXTERN_C void  pss_destroy_odsort_db(PSS_ODPLAYBACK_SORT_TABLE *odsort_db);
EXTERN_C uns32     pss_send_remrow_request     (PSS_PWE_CB *pwe_cb, USRBUF * ub,
                                                uns32 tbl, NCSMIB_IDX * first_idx);
EXTERN_C uns32     pss_send_remove_request     (PSS_PWE_CB *pwe_cb,
                                                uns32 tbl, NCSMIB_IDX * idx, 
                                                NCSMIB_PARAM_ID status_param_id);
EXTERN_C void pss_destroy_spcn_wbreq_pend_list(PSS_SPCN_WBREQ_PEND_LIST *pend_list);
EXTERN_C uns32 pss_sync_mib_req(NCSMIB_ARG *marg, NCSMIB_REQ_FNC mib_fnc, 
                                uns32 sleep_dur, NCSMEM_AID *mma);

/*****************************************************************************
 Persistent store routines 
*****************************************************************************/

EXTERN_C uns32 pss_flush_mib_entries_from_curr_profile(PSS_PWE_CB *pwe_cb,
                                        PSS_SPCN_LIST *spcn_entry,
                                        PSS_CLIENT_ENTRY *clt_node);
EXTERN_C uns32 pss_read_from_store(PSS_PWE_CB *pwe_cb, uns8 * profile_name,
                          uns8 * key, char *p_pcn,
                          uns32 tbl_id, NCS_PATRICIA_NODE * pNode,
                          NCS_BOOL * entry_found);
EXTERN_C uns32 pss_read_from_sclr_store(PSS_PWE_CB *pwe_cb, uns8 * profile_name,
                          uns8 *data, char *p_pcn,
                          uns32 tbl_id, NCS_BOOL * entry_found);
EXTERN_C uns32 pss_save_to_store(PSS_PWE_CB *pwe_cb, NCS_PATRICIA_TREE * pTree,
                        uns8 *profile_name, char *p_pcn, uns32 tbl_id);
EXTERN_C uns32     pss_save_to_sclr_store(PSS_PWE_CB *pwe_cb, PSS_TBL_REC * tbl_rec,
                                     uns8 * profile_name, char *p_pcn, uns32 tbl_id);
EXTERN_C uns32     pss_save_current_configuration(PSS_CB * inst);
EXTERN_C uns32 pss_add_to_refresh_tbl_list(PSS_PWE_CB *pwe_cb,
                   PSS_TBL_REC *tbl_rec);
EXTERN_C void  pss_free_refresh_tbl_list(PSS_PWE_CB *pwe_cb);
EXTERN_C void pss_del_rec_frm_refresh_tbl_list(PSS_PWE_CB *pwe_cb, PSS_TBL_REC *rec);
EXTERN_C uns32 pss_pop_mibrows_into_buffer(PSS_CB *inst, char *profile, PW_ENV_ID pwe_id, 
                    char *pcn, uns32 tbl_id, NCSMIB_ARG *arg, FILE *fh);
EXTERN_C uns32 pss_dump_sclr_tbl(PSS_CB *inst, char *profile, PW_ENV_ID pwe_id, char *pcn,
                        uns32 tbl_id, NCSMIB_ARG *arg, FILE *fh);
EXTERN_C uns32 pss_dump_tbl(PSS_CB *inst, char *profile, PW_ENV_ID pwe_id, char *pcn,
                   uns32 tbl_id, NCSMIB_ARG *arg, FILE *fh);
EXTERN_C void pss_dump_mib_var(FILE *fh, NCSMIB_PARAM_VAL *pv, PSS_MIB_TBL_INFO *tbl_info, uns32 param_index);

/*****************************************************************************
 MIB Registration and Request handling routines 
*****************************************************************************/

EXTERN_C uns32     pss_register_with_oac(PSS_CB * inst);
EXTERN_C uns32     pss_unregister_with_oac(PSS_CB * inst);
EXTERN_C uns32     pss_mib_request(struct ncsmib_arg *mib_args);

/************************************************************************
  MDS bindary stuff for PSS
*************************************************************************/

EXTERN_C uns32 pss_mds_grcv(MDS_HDL pwe_hdl,    MDS_CLIENT_HDL yr_svc_hdl,
                           NCSCONTEXT msg, 
                           MDS_DEST  fr_card,    SS_SVC_ID fr_svc,
                           MDS_DEST  to_card,    SS_SVC_ID to_svc);

EXTERN_C uns32 pss_mds_cb (NCSMDS_CALLBACK_INFO *cbinfo);

/************************************************************************
  Data Dump Utility Routines for Debugging
*************************************************************************/
EXTERN_C void pss_cb_data_dump(void);


/**************** Fix for IR00085164 ************************************
uns32 pss_stdby_oaa_down_list_update(MDS_DEST oaa_addr,NCSCONTEXT yr_hdl,PSS_STDBY_OAA_DOWN_BUFFER_OP buffer_op);
************************************************************************/

/************************************************************************

  O b j e c t   A c e s s   C l i e n t    L o c k s

 The PSS locks follow the lead of the master MIB Access Broker setting
 to determine what type of lock to implement.

*************************************************************************/


#if 0
#if (NCSMAB_USE_LOCK_TYPE == MAB_NO_LOCKS)                  /* NO Locks */

#define m_PSS_LK_CREATE(lk)
#define m_PSS_LK_INIT
#define m_PSS_LK(lk) 
#define m_PSS_UNLK(lk) 
#define m_PSS_LK_DLT(lk) 

#elif (NCSMAB_USE_LOCK_TYPE == MAB_TASK_LOCKS)            /* Task Locks */

#define m_PSS_LK_CREATE(lk)
#define m_PSS_LK_INIT            m_INIT_CRITICAL
#define m_PSS_LK(lk)             m_START_CRITICAL
#define m_PSS_UNLK(lk)           m_END_CRITICAL
#define m_PSS_LK_DLT(lk) 

#elif (NCSMAB_USE_LOCK_TYPE == MAB_OBJ_LOCKS)           /* Object Locks */

#endif
#endif

/* Always use Object Locks */
#define m_PSS_LK_CREATE(lk)      m_NCS_LOCK_INIT_V2(lk,NCS_SERVICE_ID_MAB, \
                                                    PSS_LOCK_ID)
#define m_PSS_LK_INIT
#define m_PSS_LK(lk)             m_NCS_LOCK_V2(lk,   NCS_LOCK_WRITE, \
                                                    NCS_SERVICE_ID_MAB, PSS_LOCK_ID)
#define m_PSS_UNLK(lk)           m_NCS_UNLOCK_V2(lk, NCS_LOCK_WRITE, \
                                                    NCS_SERVICE_ID_MAB, PSS_LOCK_ID)
#define m_PSS_LK_DLT(lk)         m_NCS_LOCK_DESTROY_V2(lk, NCS_SERVICE_ID_MAB, \
                                                    PSS_LOCK_ID)



#endif /* PSR_PVT_H */
