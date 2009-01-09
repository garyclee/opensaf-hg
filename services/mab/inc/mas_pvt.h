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

#include <config.h>

/*****************************************************************************
..............................................................................

  $Header: /ncs/software/release/UltraLinq/MAB/MAB1.1/base/products/mab/inc/mas_pvt.h 30    10/17/01 3:26p Questk $



..............................................................................

  DESCRIPTION:

  _Private_ MIB Access Server (MAS) abstractions and function prototypes.

*******************************************************************************/

/*
 * Module Inclusion Control...
 */
#ifndef MAS_PVT_H
#define MAS_PVT_H


/*****************************************************************************

  M A S _ R O W _ R E C : A record of ownership information about a particular
                  MIB Table within the context of the single MAS, 

                  This record is accessed by TBL ID. The assumption is that 
                  there is only one subsystem per local OAC that will claim
                  ownership of a particular MIB table.

*****************************************************************************/


struct mas_tbl;

/* MAS version */ 
#define MAS_MDS_SUB_PART_VERSION 2  /* mas version is increased as ncsmib_arg is changed(handles & ncsmib_rsp changed)*/
#define MAS_WRT_MAC_SUBPART_VER_AT_MIN_MSG_FMT    1 /* minimum version of MAC supported by MAS */
#define MAS_WRT_MAC_SUBPART_VER_AT_MAX_MSG_FMT    2 /* maximum version of MAC supported by MAS */

/* size of the array used to map MAC subpart version to mesage format version */
#define  MAS_WRT_MAC_SUBPART_VER_RANGE                \
         (MAS_WRT_MAC_SUBPART_VER_AT_MAX_MSG_FMT  -   \
         MAS_WRT_MAC_SUBPART_VER_AT_MIN_MSG_FMT + 1)

#define MAS_WRT_OAC_SUBPART_VER_AT_MIN_MSG_FMT    1 /* minimum version of OAC supported by MAS */
#define MAS_WRT_OAC_SUBPART_VER_AT_MAX_MSG_FMT    2 /* maximum version of OAC supported by MAS */

/* size of the array used to map OAC subpart version to mesage format version */
#define  MAS_WRT_OAC_SUBPART_VER_RANGE                \
         (MAS_WRT_OAC_SUBPART_VER_AT_MAX_MSG_FMT  -   \
         MAS_WRT_OAC_SUBPART_VER_AT_MIN_MSG_FMT + 1)


typedef void* (*MAS_FLTR_FNC)(struct mas_tbl* inst, NCSMAB_FLTR* fltr, uns32* inst_ids, uns32 inst_len,NCS_BOOL is_next_req);


/************************************************************************
  MAS RAnchor value, and the filter-id of the OAA 
*************************************************************************/
typedef struct mab_fltr_id_node_tag
{
    MAB_ANCHOR  anchor;     /* Anchor value of the OAA */ 
    uns32       fltr_id;    /* Filter-Id from the OAA */ 
    struct mab_fltr_id_node_tag *next; 
}MAB_FLTR_ANCHOR_NODE;

typedef struct mas_fltr_ids
{
  MAB_FLTR_ANCHOR_NODE  *active_fltr; /* filter ids of active anchor */
  MAB_FLTR_ANCHOR_NODE  *fltr_id_list; /* filter ids for different anchors */
}MAS_FLTR_IDS; 

/************************************************************************
  MAS FLTR  as stored in the MAS MIB TBL data base
*************************************************************************/

typedef struct mas_fltr
  {
  struct mas_fltr*    next;
  MAS_FLTR_FNC        test;        /* set up at registration time */
  NCSMAB_FLTR         fltr;
  MDS_DEST            vcard;
  MAS_FLTR_IDS        fltr_ids;
  uns16               ref_cnt;
  NCS_BOOL            wild_card;  
  } MAS_FLTR;

/************************************************************************
  MAS DEFAULT FLTR  as stored in the MAS MIB TBL data base
*************************************************************************/

typedef struct mas_dfltr
{
  MDS_DEST            vcard;
  MAS_FLTR_IDS        fltr_ids;
} MAS_DFLTR;

/************************************************************************
  MAS_ROW_REC filter and reachability info for matched MIB Row data.
*************************************************************************/

typedef struct mas_row_rec
  {
  struct mas_row_rec* next;
  uns32               tbl_id;
  uns32               ss_id;
  MAS_FLTR*           fltr_list;
  NCS_BOOL            dfltr_regd;
  MAS_DFLTR           dfltr;
  } MAS_ROW_REC;

/************************************************************************
  Free macro for MAS_FLTR
*************************************************************************/
EXTERN_C void mas_fltr_free (MAS_FLTR   *fltr); 
#define m_MAS_FLTR_FREE(mas_fltr) mas_fltr_free(mas_fltr)
/************************************************************************
  Free macro for MAS_FLTR
*************************************************************************/
EXTERN_C void mas_fltr_free (MAS_FLTR   *fltr); 
#define m_MAS_FLTR_FREE(mas_fltr) mas_fltr_free(mas_fltr)

/*****************************************************************************

  M A S _ T B L _ S E T : A table of all MAS_TBL_RECs that the MAS knows 
                  about, as learned during registration.

*****************************************************************************/
typedef struct mas_tbl
{
    /* to protect this structure */ 
    NCS_LOCK           lock;

    /* Hash table to store the table/filter data of OAAs */ 
    MAS_ROW_REC*       hash[MAB_MIB_ID_HASH_TBL_SIZE];

    /* my Name as understood by others, MAS Address */ 
    MDS_DEST           my_vcard;
    
    /* Create time constants */
    /* Handle Manager output */ 
    uns32              hm_hdl;

    /* handle manager input */ 
    uns8               hm_poolid;

    /* MDS Handle for this Private World Environment */ 
    MDS_HDL            mds_hdl;

    /* Private World Envioronment Identifier */ 
    NCS_VRID           vrid;  

    /* MAS Mailbox details */ 
    SYSF_MBX*          mbx;           

    /* All Knowing Entity Callback function */ 
    MAB_LM_CB          lm_cbfnc;
    
    /* Standby MAS responds to AMF with this invocation-id 
     * once the cold sync is complete, and is used for QUIESCED state processing. 
     */
    SaInvocationT   amf_invocation_id;  


#if (NCS_MAS_RED == 1)
    /* to store the redundancy related attribus (MBCSv hdl, ckpt hdl, etc...) */ 
    MAS_RED  red;
#endif
} MAS_TBL;

typedef struct mas_mbx
{
    SYSF_MBX   mas_mbx;
    void *     mas_mbx_hdl;
}MAS_MBX;

/************************************************************************

  M A S    P r i v a t e   F u n c t i o n   P r o t o t y p e s

*************************************************************************/

#define m_MAS_COMP_NAME_FILE LOCALSTATEDIR "ncs_mas_comp_name"
#define m_MAS_PID_FILE PIDPATH "ncs_mas.pid"

/************************************************************************
  MAC tasking loop function
*************************************************************************/

EXTERN_C MABMAS_API uns32      mas_do_evts     ( SYSF_MBX*     mbx     );
EXTERN_C uns32     mas_do_evt      ( MAB_MSG*      msg     );


/************************************************************************
  Basic MAS Layer Management Service entry points off std LM API
*************************************************************************/
EXTERN_C uns32     mas_svc_create  ( NCSMAS_CREATE* create);
EXTERN_C uns32     mas_svc_destroy ( NCSMAS_DESTROY* destroy );

/*****************************************************************************
 MAS Table Entry manipulation routines 
*****************************************************************************/
EXTERN_C void         mas_row_rec_init( MAS_TBL* inst                      );
EXTERN_C void         mas_row_rec_clr ( MAS_TBL* inst                      );
EXTERN_C void         mas_row_rec_put ( MAS_TBL* inst, MAS_ROW_REC* tbl_rec);
EXTERN_C MAS_ROW_REC* mas_row_rec_rmv ( MAS_TBL* inst, uns32 tbl_id        );
EXTERN_C MAS_ROW_REC* mas_row_rec_find( MAS_TBL* inst, uns32 tbl_id        );

/*****************************************************************************
 MAS Filter manipulation routines 
*****************************************************************************/
EXTERN_C void      mas_fltr_put       (MAS_FLTR**   insert_after, 
                                       MAS_FLTR*    new_fltr    );

EXTERN_C MAS_FLTR* mas_fltr_rmv       (MAS_FLTR**   head, 
                                       MAS_FLTR*    del_me);

EXTERN_C MAS_FLTR* mas_fltr_find      (MAS_TBL*     inst,
                                       MAS_FLTR**   head, 
                                       uns32        fltr_id,
                                       MAB_ANCHOR   anc,
                                       MDS_DEST     vcard);

EXTERN_C MAS_FLTR* mas_fltr_create    (MAS_TBL*     inst,
                                       NCSMAB_FLTR* mab_fltr,
                                       MDS_DEST     vcard,
                                       V_DEST_RL    fr_oaa_role,
                                       MAB_ANCHOR   anc,
                                       uns32        fltr_id,
                                       MAS_FLTR_FNC fltr_fnc    );

EXTERN_C void*     mas_classify_range  (MAS_TBL*    inst, 
                                        NCSMAB_FLTR* fltr, 
                                        uns32*      inst_ids, 
                                        uns32       inst_len,
                                        NCS_BOOL    is_next_req);

EXTERN_C void*     mas_classify_any    (MAS_TBL*    inst, 
                                        NCSMAB_FLTR* fltr, 
                                        uns32*      inst_ids, 
                                        uns32       inst_len,
                                        NCS_BOOL    is_next_req);

EXTERN_C void*     mas_classify_same_as(MAS_TBL*    inst, 
                                        NCSMAB_FLTR* fltr, 
                                        uns32*      inst_ids, 
                                        uns32       inst_len,
                                        NCS_BOOL    is_next_req);

EXTERN_C void*     mas_classify_exact(MAS_TBL* inst, NCSMAB_FLTR* fltr,
                    uns32* inst_ids, uns32 inst_len,
                    NCS_BOOL is_next_req); 

EXTERN_C MAS_FLTR* mas_locate_dst_oac  (MAS_TBL*    inst, 
                                        MAS_FLTR*   head, 
                                        uns32*      inst_ids, 
                                        uns32       inst_len,
                                        NCS_BOOL    is_next_req);

/*****************************************************************************
 MAS MAB msg forwarding routines 
*****************************************************************************/

EXTERN_C uns32 mas_forward_msg       (MAB_MSG*   msg, 
                                      MAB_SVC_OP msg_op, 
                                      MDS_HDL    mds_hdl, 
                                      MDS_DEST   to_vcard, 
                                      SS_SVC_ID  to_svc_id,  
                                      NCS_BOOL   is_req   );

EXTERN_C uns32 mas_relay_msg_to_mac  (MAB_MSG*   msg, 
                                      MAS_TBL*   inst, 
                                      NCS_BOOL   is_req   );

EXTERN_C uns32 mas_next_op_stack_push(MAS_TBL*   inst, 
                                      NCS_STACK* stack, 
                                      uns32      fltr_id  );
EXTERN_C uns32  mas_cli_msg_to_mac(MAB_MSG *msg, MAS_TBL *inst);

/*****************************************************************************
 MAB message handling  routines 
*****************************************************************************/

EXTERN_C uns32 mas_info_request   (MAB_MSG* msg);
EXTERN_C uns32 mas_info_response  (MAB_MSG* msg);
EXTERN_C uns32 mas_info_register  (MAB_MSG* msg);
EXTERN_C uns32 mas_info_unregister(MAB_MSG* msg);

/************************************************************************
  MDS bindary stuff for MAS
*************************************************************************/
EXTERN_C uns32 mas_mds_cb (NCSMDS_CALLBACK_INFO *cbinfo);

EXTERN_C uns32 maslib_mas_destroy (void);


/************************************************************************
  Macros for the data structures in this file 
*************************************************************************/
EXTERN_C void mas_fltr_free(MAS_FLTR *mas_fltr); 
#define m_FREE_MAS_FLTR(mas_fltr)   mas_fltr_free(mas_fltr)

/************************************************************************

  M A S    P r i v a t e   M a c r o s   

*************************************************************************/
#if (NCS_MAS_RED == 1)
#define m_MAS_RE_OAA_DOWN_SYNC(inst,msg) \
{ \
    if(inst->red.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE) \
    {\
        inst->red.oaa_down_async_count++;\
        mas_red_sync(inst, msg, MAS_ASYNC_OAA_DOWN); \
    }\
}

#define m_MAS_RE_OAA_DOWN_SYNC_DONE(inst,msg) \
{ \
    if(inst->red.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE) \
    {\
        mas_red_sync_done(inst,MAS_ASYNC_OAA_DOWN); \
        m_MMGR_FREE_MAB_MSG(msg); \
    }\
    else\
    {\
        inst->red.oaa_down_async_count++;\
        m_MMGR_FREE_MAB_MSG(msg); \
    }\
}

#define m_MAS_RE_REG_UNREG_SYNC(inst, msg, tbl_id) \
{ \
    if (inst->red.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE)\
    {\
        inst->red.async_count[tbl_id%MAB_MIB_ID_HASH_TBL_SIZE]++; \
        mas_red_sync(inst,msg,MAS_ASYNC_REG_UNREG); \
    }\
}

#define m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, tbl_id) \
{ \
    if (inst->red.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE)\
    {\
        mas_red_sync_done(inst, MAS_ASYNC_REG_UNREG); \
        mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);\
        m_MMGR_FREE_MAB_MSG(msg); \
    }\
    else \
    { \
        inst->red.async_count[tbl_id%MAB_MIB_ID_HASH_TBL_SIZE]++; \
        mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);\
        m_MMGR_FREE_MAB_MSG(msg); \
    } \
}

#define m_MAS_RE_MIB_ARG_SYNC(inst, msg, tbl_id) \
{ \
    if (inst->red.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE)\
        mas_red_sync(inst,msg,MAS_ASYNC_MIB_ARG); \
}

#define m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, tbl_id) \
{ \
    if (inst->red.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE) \
    {\
        mas_red_sync_done(inst, MAS_ASYNC_MIB_ARG); \
    }\
}
#else

#define m_MAS_RE_OAA_DOWN_SYNC(inst,msg) 
#define m_MAS_RE_OAA_DOWN_SYNC_DONE(inst,msg) \
{ \
    m_MMGR_FREE_MAB_MSG(msg); \
} 

#define m_MAS_RE_REG_UNREG_SYNC(inst, msg, tbl_id)
#define m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, tbl_id) \
{ \
    mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);\
    m_MMGR_FREE_MAB_MSG(msg); \
}

#define m_MAS_RE_MIB_ARG_SYNC(inst, msg, tbl_id) 
#define m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, tbl_id) 

#endif /* #if (NCS_MAS_RED == 1) */ 

/************************************************************************

  M I B    A c e s s   S e r v e r    L o c k s

 The MAS locks follow the lead of the master MIB Access Broker setting
 to determine what type of lock to implement.

*************************************************************************/
#if (NCSMAB_USE_LOCK_TYPE == MAB_NO_LOCKS)                  /* NO Locks */

#define m_MAS_LK_CREATE(lk)
#define m_MAS_LK_INIT
#define m_MAS_LK(lk) 
#define m_MAS_UNLK(lk) 
#define m_MAS_LK_DLT(lk) 

#elif (NCSMAB_USE_LOCK_TYPE == MAB_TASK_LOCKS)            /* Task Locks */

#define m_MAS_LK_CREATE(lk)
#define m_MAS_LK_INIT            m_INIT_CRITICAL
#define m_MAS_LK(lk)             m_START_CRITICAL
#define m_MAS_UNLK(lk)           m_END_CRITICAL
#define m_MAS_LK_DLT(lk) 

#elif (NCSMAB_USE_LOCK_TYPE == MAB_OBJ_LOCKS)           /* Object Locks */


#define m_MAS_LK_CREATE(lk)      m_NCS_LOCK_INIT_V2(lk,NCS_SERVICE_ID_MAB, \
                                                    MAS_LOCK_ID)
#define m_MAS_LK_INIT
#define m_MAS_LK(lk)             m_NCS_LOCK_V2(lk,   NCS_LOCK_WRITE, \
                                                    NCS_SERVICE_ID_MAB, MAS_LOCK_ID)
#define m_MAS_UNLK(lk)           m_NCS_UNLOCK_V2(lk, NCS_LOCK_WRITE, \
                                                    NCS_SERVICE_ID_MAB, MAS_LOCK_ID)
#define m_MAS_LK_DLT(lk)         m_NCS_LOCK_DESTROY_V2(lk, NCS_SERVICE_ID_MAB, \
                                                    MAS_LOCK_ID)

#endif 

#endif /* MAS_PVT_H */


