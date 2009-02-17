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

#ifndef GLD_CB_H
#define GLD_CB_H

# include "gld_tmr.h"

#define m_GLSV_UNS64_TO_PARAM(param,buffer,val64) \
{\
   param->i_fmat_id = NCSMIB_FMAT_OCT; \
   param->i_length = 8; \
   param->info.i_oct = (uns8 *)buffer; \
   m_NCS_OS_HTONLL_P(param->info.i_oct,val64); \
}


/* typedef enums */
typedef enum
{
   GLD_RESOURCE_NOT_INITIALISED = 0,
   GLD_RESOURCE_ACTIVE_MASTER,
   GLD_RESOURCE_ACTIVE_NON_MASTER,
   GLD_RESOURCE_ELECTION_IN_PROGESS,
}GLD_RESOURCE_STATUS;


typedef struct glsv_node_list_tag{
   MDS_DEST                   dest_id;
   uns32                      node_id;
   struct glsv_node_list_tag  *next;
}GLSV_NODE_LIST;

typedef struct glsv_gld_rsc_info_tag{
   NCS_PATRICIA_NODE    pat_node;
   SaLckResourceIdT     rsc_id;           /* unique resource id - Index for            */
   SaNameT              lck_name;         /* Cluster-wide unique lock name             */
   GLSV_NODE_LIST       *node_list;       /* Nodes on which this resource is reffered  */
   NCS_BOOL             can_orphan;       /* is this resource allocated in orphan mode */
   SaLckLockModeT       orphan_lck_mode;  /* related to orphan mode                    */
   GLD_TMR              reelection_timer;
   uns32                status;
   struct glsv_gld_rsc_info_tag  *next;   /* List of resources                         */
   struct glsv_gld_rsc_info_tag  *prev;
 
   /* MIB variables */  
   SaTimeT             saf_rsc_creation_time;   
   uns32               saf_rsc_no_of_users;
   uns32               saf_rsc_stripped_cnt;
}GLSV_GLD_RSC_INFO;


typedef struct glsv_glnd_rsc_ref_tag{
   NCS_PATRICIA_NODE    pat_node;
   SaLckResourceIdT     rsc_id;           /* unique resource id - Index for  */
   GLSV_GLD_RSC_INFO    *rsc_info;
}GLSV_GLD_GLND_RSC_REF;

typedef struct glsv_gld_glnd_details_tag{
   NCS_PATRICIA_NODE    pat_node;
   MDS_DEST             dest_id;          /* Vcard id as given by MDS                  */
   uns32                node_id;          /* Node id of the Node Director              */
   uns32                status;
   GLD_TMR              restart_timer;
   NCS_PATRICIA_TREE    rsc_info_tree;    /* List of resources ref by this node        */
}GLSV_GLD_GLND_DETAILS;

typedef struct glsv_gld_rsc_map_info{
   NCS_PATRICIA_NODE    pat_node;
   SaLckResourceIdT     rsc_id;           /* unique resource id - Index for            */
   SaNameT              rsc_name;         
}GLSV_GLD_RSC_MAP_INFO;

typedef struct glsv_gld_cb_tag{
   SYSF_MBX             mbx;              /* GLDs mailbox                              */
   SaNameT              comp_name;        /* Component name - "GLD"                    */
   MDS_HDL              mds_handle;       /* PWE handle used for interacting with NDs  */
   uns8                 hm_poolid;        /* For use with handle manager               */
   NCSCONTEXT           task_hdl;
   uns32                my_hdl;           /* Handle manager handle                     */
   uns32                clm_hdl;           /* Handle manager handle                     */
   NCS_MBCSV_HDL        mbcsv_handle;
   NCS_MBCSV_CKPT_HDL   o_ckpt_hdl;
   uns32                oac_hdl;
   SaSelectionObjectT   mbcsv_sel_obj;
   uns32                gld_async_cnt;
   V_DEST_QA            my_anc;
   MDS_VDEST_ID         my_dest_id;       /* My identification in MDS                  */

   NCS_BOOL             glnd_details_tree_up;
   NCS_PATRICIA_TREE    glnd_details;     /* Details of nodes on which NDs are up      */
   NCS_BOOL             rsc_info_id_tree_up;
   NCS_PATRICIA_TREE    rsc_info_id;      /* Resource info indexed by rsc_id           */
   NCS_BOOL             rsc_map_info_tree_up;
   NCS_PATRICIA_TREE    rsc_map_info;   /* Resource info indexed by rsc_name           */
   GLSV_GLD_RSC_INFO    *rsc_info;        /* List of resources referred to             */
   SaLckResourceIdT     nxt_rsc_id;       /* Next rsc id to be generated               */
   SaLckResourceIdT     prev_rsc_id;       /* Prev res_id to be used for next cold sync rsp*/

   SaAmfHandleT         amf_hdl;          /* AMF handle, obtained thru AMF init        */   
   SaAmfHAStateT        ha_state;         /* present AMF HA state of the component     */   

   EDU_HDL              edu_hdl;          /* EDU Handle for encode decodes             */

   SaInvocationT        invocation;
   uns32                is_quiasced; 

   /* MIB scalar elements */
   SaNameT              saf_spec_ver;
   SaNameT              saf_agent_vend;
   uns32                saf_agent_vend_prod;
   NCS_BOOL             saf_serv_state_enabled;
   uns32                saf_serv_state;
}GLSV_GLD_CB;

#define GLD_CB_NULL  ((GLSV_GLD_CB *)0)


/* Function Declarations */

uns32 gld_mds_quiesced_process(GLSV_GLD_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *rcv_info);
void  gld_snd_master_status(GLSV_GLD_CB *gld_cb, GLSV_GLD_RSC_INFO *rsc_info, uns32 status);
GLSV_GLD_RSC_INFO *gld_find_rsc_by_id(GLSV_GLD_CB              *gld_cb,
                                         SaLckResourceIdT         rsc_id);
EXTERN_C uns32 glsv_gld_mbcsv_register(GLSV_GLD_CB *gld_cb);
EXTERN_C uns32 glsv_gld_mbcsv_unregister(GLSV_GLD_CB *gld_cb);
EXTERN_C void gld_dump_cb(void);

GLSV_GLD_RSC_INFO *gld_find_add_rsc_name(GLSV_GLD_CB              *gld_cb,
                                         SaNameT                  rsc_name,
                                         SaLckResourceIdT         rsc_id,               
                                         SaLckResourceOpenFlagsT  flag,
                                         SaAisErrorT                 *error);

void gld_free_rsc_info(GLSV_GLD_CB *gld_cb,
                       GLSV_GLD_RSC_INFO *rsc_info);

void gld_rsc_rmv_node_ref(GLSV_GLD_CB *gld_cb, 
                          GLSV_GLD_RSC_INFO *rsc_info,
                          GLSV_GLD_GLND_RSC_REF   *glnd_rsc,
                          GLSV_GLD_GLND_DETAILS *node_details,
                          NCS_BOOL orphan_flag);

void gld_rsc_add_node_ref(GLSV_GLD_CB *gld_cb,
                          GLSV_GLD_GLND_DETAILS *node_details,
                          GLSV_GLD_RSC_INFO *rsc_info);

#define m_GLSV_GLD_RETRIEVE_GLD_CB  ncshm_take_hdl(NCS_SERVICE_ID_GLD, gl_gld_hdl)
#define m_GLSV_GLD_GIVEUP_GLD_CB    ncshm_give_hdl(gl_gld_hdl)

#define GLD_RESOURCE_INFO_NODE_NULL ((GLSV_GLD_RSC_INFO *)0)
EXTERN_C uns32 gld_rsc_ref_set_orphan(GLSV_GLD_GLND_DETAILS   *node_details, SaLckResourceIdT            rsc_id, NCS_BOOL          orphan, SaLckLockModeT    lck_mode);
EXTERN_C GLSV_GLD_RSC_INFO *gld_add_rsc_info(GLSV_GLD_CB              *gld_cb,
                                         SaNameT             rsc_name,
                                         SaLckResourceIdT    rsc_id,
                                         SaAisErrorT         *error);
EXTERN_C uns32 gld_reg_with_mab(GLSV_GLD_CB *cb);
EXTERN_C uns32 gld_unreg_with_mab(GLSV_GLD_CB *cb);
EXTERN_C uns32 gld_reg_with_miblib();
EXTERN_C uns32 gld_process_node_down_evts( GLSV_GLD_CB *gld_cb );
#endif
