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

  This module is the include file for handling Availability Node Directors 
  CLM structures
  
******************************************************************************
*/

#ifndef AVND_CLM_H
#define AVND_CLM_H

struct avnd_cb_tag;

/* clm db records */
typedef struct avnd_clm_rec {
   NCS_DB_LINK_LIST_NODE  clm_dll_node; /* clm dll node */
   AVSV_CLM_INFO          info;         /* node info */
   /* Update received flag, which will normally be FALSE and will be
    * TRUE if updates are received from the AVD on fail-over.*/
   NCS_BOOL            avd_updt_flag;
} AVND_CLM_REC;

/* clm track Info */
typedef struct avnd_clm_trk_info {
   SaClmHandleT req_hdl;    /* clm handle (index) */
   MDS_DEST  mds_dest;   /* mds dest of the prc that started clm tracking (index) */
   uns8      track_flag; /* track flag */
   struct avnd_clm_trk_info *next;
} AVND_CLM_TRK_INFO;

/* clm db decalaration */
typedef struct avnd_clm_db {
   AVSV_AVND_CARD     type;          /* node type (scxb or payload) */
   SaClmClusterNodeT  node_info;     /* this node's info */
   SaUint64T          curr_view_num; /* current view no */
   NCS_DB_LINK_LIST   clm_list;      /* list of nodes in the cluster */
   AVND_CLM_TRK_INFO  *clm_trk_info; /* list of clm track requests */
} AVND_CLM_DB;


/* macros for managing clm track flags */
#define m_AVND_CLM_IS_TRACK_CURRENT(x) \
             (((x)->track_flag) & SA_TRACK_CURRENT)
#define m_AVND_CLM_IS_TRACK_CHANGES(x) \
             (((x)->track_flag) & SA_TRACK_CHANGES)
#define m_AVND_CLM_IS_TRACK_CHANGES_ONLY(x) \
             (((x)->track_flag) & SA_TRACK_CHANGES_ONLY)

#define m_AVND_CLM_TRACK_CURRENT_SET(x) \
             (((x)->track_flag) |= SA_TRACK_CURRENT)
#define m_AVND_CLM_TRACK_CHANGES_SET(x) \
             (((x)->track_flag) |= SA_TRACK_CHANGES)
#define m_AVND_CLM_TRACK_CHANGES_ONLY_SET(x) \
             (((x)->track_flag) |= SA_TRACK_CHANGES_ONLY)

#define m_AVND_CLM_TRACK_CURRENT_RESET(x) \
             (((x)->track_flag) &= ~SA_TRACK_CURRENT)
#define m_AVND_CLM_TRACK_CHANGES_RESET(x) \
             (((x)->track_flag) &= ~SA_TRACK_CHANGES)
#define m_AVND_CLM_TRACK_CHANGES_ONLY_RESET(x) \
             (((x)->track_flag) &= ~SA_TRACK_CHANGES_ONLY)


/* Extern Declarations */
EXTERN_C uns32 avnd_clmdb_init(struct avnd_cb_tag *);
EXTERN_C uns32 avnd_clmdb_destroy(struct avnd_cb_tag *);
EXTERN_C AVND_CLM_REC *avnd_clmdb_rec_add(struct avnd_cb_tag *, AVSV_CLM_INFO *);
EXTERN_C uns32 avnd_clmdb_rec_del(struct avnd_cb_tag *, SaClmNodeIdT);
EXTERN_C AVND_CLM_REC *avnd_clmdb_rec_get(struct avnd_cb_tag *, SaClmNodeIdT);

EXTERN_C uns32 avnd_clm_trkinfo_list_add(struct avnd_cb_tag *, AVND_CLM_TRK_INFO *);
EXTERN_C AVND_CLM_TRK_INFO *avnd_clm_trkinfo_list_del(struct avnd_cb_tag *, SaClmHandleT, MDS_DEST * );
EXTERN_C AVND_CLM_TRK_INFO *avnd_clm_trkinfo_list_find(struct avnd_cb_tag *, SaClmHandleT , MDS_DEST *);

EXTERN_C uns32 avnd_clm_track_current_resp(struct avnd_cb_tag *, 
                                           AVND_CLM_TRK_INFO *, 
                                           MDS_SYNC_SND_CTXT *, NCS_BOOL);

EXTERN_C void avnd_clm_snd_track_changes(struct avnd_cb_tag *, 
                                         AVND_CLM_REC *, 
                                         SaClmClusterChangesT );

#endif
