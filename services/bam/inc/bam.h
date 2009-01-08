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

  This module is the main include file for BOM Access Manager.
  
******************************************************************************
*/

#ifndef BAM_H
#define BAM_H


#include <ncs_opt.h>
#include <gl_defs.h>
#include <t_suite.h>
#include <ncs_mtbl.h>
#include <ncsgl_defs.h>
#include <saAis.h>
#include <stdio.h>
#include <ncs_mib_pub.h>
#include <ncs_lib.h>
#include <ncsdlib.h>
#include "mds_papi.h"
#include "ncs_mda_pvt.h"
#include "rda_papi.h"
#include "ncs_avsv_bam_msg.h"
#include <psr_bam.h>
#include "ncs_bam_avm.h"
#include "ncs_util.h"
#include <hpl_msg.h>

#define ANIL_DEBUGLOG printf
#define ncs_bam_free(x) m_MMGR_FREE_BAM_DEFAULT_VAL((void *)x)

#define NCS_BAM_VCARD_ID   (MDS_VDEST_ID)3 /* TBD */
#define m_BAM_MSG_SND(mbx,msg,prio)  m_NCS_IPC_SEND(mbx,(NCSCONTEXT)msg, prio)
#define m_NCS_BAM_MSG_RCV(mbx,msg)      m_NCS_IPC_RECEIVE(mbx,msg)

/* Versioning changes related definition */
#define BAM_MDS_SUB_PART_VERSION   1

#define BAM_AVD_SUBPART_VER_MIN    1
#define BAM_AVD_SUBPART_VER_MAX    1

#define BAM_AVM_SUBPART_VER_MIN    1
#define BAM_AVM_SUBPART_VER_MAX    1

#define BAM_PSS_SUBPART_VER_MIN    1
#define BAM_PSS_SUBPART_VER_MAX    1

/*
 * This structure aids in maintaining database of the conversion tables.
 *
 * This structure stores the XML identifying fields and their OID counterparts
 * that fill the MIBARG structure.
 */
typedef struct tag_oid_map_node
{
   char*          pKey;       /* the string uniquely identifying 
                                 the XML element (instance) */
   char*          sKey;       /* the string uniquely identifying 
                                 tag within the element */
   NCSMIB_TBL_ID   table_id;   /* table id */
   /*ANIL_NCSMIB_PARAM_ID param_id;*/   /* param id */  /* compilation problems */
   uns32           param_id;  
   NCSMIB_FMAT_ID  format;    /* is it a INT or OCT or BOOL */
}TAG_OID_MAP_NODE;


EXTERN_C TAG_OID_MAP_NODE gl_amfConfig_table[];
EXTERN_C const uns32 gl_amfConfig_table_size;

/* Service Sub IDs for BAM */
typedef enum
{
   NCS_SERVICE_BAM_SUB_ID_DEFAULT_VAL = 1,
   NCS_SERVICE_BAM_SUB_ID_CB,
   NCS_SERVICE_BAM_SUB_ID_BAM_MESSAGE,
   NCS_SERVICE_BAM_SUB_ID_CSI_COMP,
   NCS_SERVICE_BAM_SUB_ID_SI_DEP,
   NCS_SERVICE_BAM_SUB_ID_MAX
} NCS_SERVICE_BAM_SUB_ID;


typedef enum /* as defined in AvSV MIB */
{
   NCS_BAM_SG_REDMODEL_2N = 1,
   NCS_BAM_SG_REDMODEL_NPLUSM,
   NCS_BAM_SG_REDMODEL_NWAY,
   NCS_BAM_SG_REDMODEL_NWAYACTIVE,
   NCS_BAM_SG_REDMODEL_NONE
}NCS_BAM_SG_REDMODEL;


typedef enum /* as defined in AvSV SAF AMF MIB */
{
   NCS_BAM_COMP_SA_AWARE= 0,
   NCS_BAM_COMP_PROXIED_LOCAL_PRE_INST,
   NCS_BAM_COMP_PROXIED_LOCAL_NONPRE_INST,
   NCS_BAM_COMP_EXTERNAL_PRE_INST,
   NCS_BAM_COMP_EXTERNAL_NONPRE_INST,
   NCS_BAM_COMP_UNPROXIED_NON_SA
}NCS_BAM_COMP_TYPE;

typedef enum /* as defined in AvSV SAF AMF MIB */
{
   NCS_BAM_COMP_CAP_X_ACTIVE_AND_Y_STANDBY = 1,
   NCS_BAM_COMP_CAP_X_ACTIVE_OR_Y_STANDBY, 
   NCS_BAM_COMP_CAP_1_ACTIVE_OR_Y_STANDBY,
   NCS_BAM_COMP_CAP_1_ACTIVE_OR_1_STANDBY,
   NCS_BAM_COMP_CAP_X_ACTIVE,
   NCS_BAM_COMP_CAP_1_ACTIVE,
   NCS_BAM_COMP_CAP_NO_STATE
}NCS_BAM_COMP_CAPABILITY;

typedef enum
{
   NCS_BAM_AVD_INVALID_MSG = 0,
   NCS_BAM_PSR_PLAYBACK,
   NCS_BAM_AVM_UP,
}NCS_BAM_EVT_TYPE;

typedef struct bam_evt
{
   NCS_IPC_MSG next;
   uns32 cb_hdl;
   NCS_BAM_EVT_TYPE evt_type;
   union {
      PSS_BAM_MSG *pss_msg;
   } msg;

}BAM_EVT;

typedef enum
{
   BAM_PARSE_NCS,
   BAM_PARSE_AVM,
   BAM_PARSE_APP
}BAM_PARSE_SUB_TREE;
/* TBD Temp placed here */
#define NCS_BAM_NAME_STR "BAM"
#define NCS_BAM_PRIORITY 0
#define NCS_BAM_STCK_SIZE NCS_STACKSIZE_HUGE
#define BAM_MAX_FILE_LEN 200
#define BAM_MAX_CSIS 50


#define m_MMGR_ALLOC_BAM_DEFAULT_VAL(mem_size)  m_NCS_MEM_ALLOC( \
                                                mem_size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_BAM, \
                                                NCS_SERVICE_BAM_SUB_ID_DEFAULT_VAL)

#define m_MMGR_FREE_BAM_DEFAULT_VAL(p)    m_NCS_MEM_FREE(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_BAM, \
                                                NCS_SERVICE_BAM_SUB_ID_DEFAULT_VAL)


#define m_MMGR_ALLOC_NCS_BAM_CB       (NCS_BAM_CB*)m_NCS_MEM_ALLOC(sizeof(NCS_BAM_CB), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_BAM, \
                                                NCS_SERVICE_BAM_SUB_ID_CB)

#define m_MMGR_FREE_NCS_BAM_CB(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_BAM, \
                                                NCS_SERVICE_BAM_SUB_ID_CB)

#define m_MMGR_ALLOC_BAM_CSI_COMP(mem_size)  m_NCS_MEM_ALLOC( \
                                                mem_size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_BAM, \
                                                NCS_SERVICE_BAM_SUB_ID_CSI_COMP)

#define m_MMGR_FREE_NCS_BAM_CSI_COMP(p)      m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_BAM, \
                                                NCS_SERVICE_BAM_SUB_ID_CSI_COMP)

#define m_MMGR_ALLOC_BAM_SI_DEP(mem_size)  m_NCS_MEM_ALLOC( \
                                                mem_size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_BAM, \
                                                NCS_SERVICE_BAM_SUB_ID_SI_DEP)

#define m_MMGR_FREE_NCS_BAM_SI_DEP(p)      m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_BAM, \
                                                NCS_SERVICE_BAM_SUB_ID_SI_DEP)

#define m_BAM_CB_LOCK_INIT(bam_cb)  m_NCS_LOCK_INIT(&bam_cb->lock)


typedef enum {
   BAM_INIT,
   BAM_CFG_RDY,
   BAM_CFG_DONE
} BAM_INIT_STATE;

typedef struct bam_si_dep_node
{
   char  si_name[128];
   char  si_dep_name[128];
   char  tol_time[128];
   struct bam_si_dep_node *next;
} BAM_SI_DEP_NODE;

typedef  struct  ncs_bam_cb{

   SYSF_MBX             *bam_mbx;               /* mailbox on which BAM waits */
   uns32                cb_handle;              /* The control block handle of BAM
                                                 */
   NCSCONTEXT           mds_handle;             /* The handle returned by MDS when
                                                 * initialized
                                                 */
   NCS_LOCK             lock;                   /* the control block lock */
                                                 
   NCSCONTEXT           addr_pwe_hdl;          /* The pwe handle returned when
                                                 * dest address is created.
                                                 */
   MDS_HDL              mds_hdl;                /* The handle returned by mds
                                                 * for addr creation
                                                 */
   MDS_DEST             my_addr;                  /* vcard address of the BAM
                                                 */
   V_DEST_QA            card_anchor;           /* the anchor value that differentiates the
                                                 * primary and standby vcards of BAM
                                                 */

   uns32                mab_hdl;                /* MAB handle returned during initilisation. */
   MDS_VDEST_ID         vcard_id;               /* the mds_vdest id of BAM */
   char                 ncs_filename[BAM_MAX_FILE_LEN]; /* NCS XML filename */
   char                 app_filename[BAM_MAX_FILE_LEN]; /* APP XML filename */
   char                 hw_filename[BAM_MAX_FILE_LEN]; /* HW XML filename */
   NCS_BOOL             avd_cfg_rdy;
   MDS_DEST             avd_dest;                /* vcard address of AVD */
   MDS_DEST             avm_dest;                /* vcard address of AVD */
   MDS_DEST             pss_dest;                /* vcard address of PSS */

   NCS_HW_ENT_TYPE_DESC *hw_entity_list;          /* List of HW entity instances */
   NCS_PATRICIA_TREE    deploy_ent_anchor;  /* Tree of deploy instances */
   void                 *deploy_list;   /* List of deploy instances */
   SaAmfHAStateT         ha_state; 
   BAM_SI_DEP_NODE      *si_dep_list;
} NCS_BAM_CB;


/* 
** This structure is obsolete and not is use for this release 
** as there is  no message posted to mail-box.
*/
typedef struct ncs_bam_msg
{
  struct ncs_bam_msg    *next;
  MDS_SVC_ID            i_fr_svc_id;
  uns32                 msg_type ;/* ANIL_ replace this with ENUM */
  uns32                 vrid ;
  uns32                 cb_hdl; 
  /* Add any structure if needed. for now, msg_type will trigger BAM parsing */
} BAM_MSG;



typedef struct bam_name_list_node{
   char  *name;
   struct bam_name_list_node *next;
}BAM_NAME_LIST_NODE;

EXTERN_C uns32 
ncs_bam_destroy();

EXTERN_C uns32 
ncs_bam_initialize(NCS_LIB_REQ_INFO *);

EXTERN_C uns32
parse_and_build_DOMDocument(char *xmlFilename, BAM_PARSE_SUB_TREE);

EXTERN_C uns32 
ncs_bam_dl_func(NCS_LIB_REQ_INFO *req_info);

EXTERN_C uns32
ncs_bam_mds_reg(NCS_BAM_CB *);

EXTERN_C uns32
ncs_bam_rda_reg(NCS_BAM_CB *);

EXTERN_C void ncs_bam_send_cfg_done_msg(void);
EXTERN_C void pss_bam_send_cfg_done_msg(BAM_PARSE_SUB_TREE);

EXTERN_C uns32
bam_avd_rcv_msg(NCS_BAM_CB *bam_cb, AVD_BAM_MSG  *msg);

EXTERN_C uns32
bam_psr_rcv_msg(NCS_BAM_CB *bam_cb, PSS_BAM_MSG  *msg);

EXTERN_C uns32 ncs_bam_restart(void);
EXTERN_C uns32 avd_bam_snd_message(AVD_BAM_MSG *snd_msg);
EXTERN_C uns32 pss_bam_snd_message(PSS_BAM_MSG *snd_msg);
EXTERN_C void bam_mds_unreg(NCS_BAM_CB *bam_cb);
EXTERN_C void ncs_bam_avm_send_cfg_done_msg(void);
EXTERN_C void bam_delete_hw_ent_list(void);
EXTERN_C uns32 bam_avm_snd_message(BAM_AVM_MSG_T *);
EXTERN_C uns32 bam_avm_send_validation_config(void);

EXTERN_C uns32 avd_bam_build_si_dep_list(char *si_name, char *si_dep_name, char *tol_time);

EXTERN_C uns32
ncs_bam_build_and_generate_mibsets(NCSMIB_TBL_ID, uns32, 
                                NCSMIB_IDX *, char *, NCSMIB_FMAT_ID);

EXTERN_C uns32
ncs_bam_generate_counter64_mibset(NCSMIB_TBL_ID, uns32, 
                                  NCSMIB_IDX *, char * );

EXTERN_C SaAisErrorT 
ncs_bam_search_table_for_oid(TAG_OID_MAP_NODE *, uns32, char *, 
                             char *, NCSMIB_TBL_ID *, 
                             uns32 *, NCSMIB_FMAT_ID  *);

EXTERN_C uns32
ncs_bam_build_mib_idx(NCSMIB_IDX *, char *, NCSMIB_FMAT_ID); 

EXTERN_C uns32
ncs_bam_add_sec_mib_idx(NCSMIB_IDX *mib_idx, char *idx_val, NCSMIB_FMAT_ID format);

#endif


