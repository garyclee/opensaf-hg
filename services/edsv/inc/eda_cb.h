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
.............................................................................

  DESCRIPTION:

  EDA CB related definitions.
  
*******************************************************************************/
#ifndef EDA_CB_H
#define EDA_CB_H

#include "eda.h"

/*  EDA control block is the master anchor structure for all
 *  EDA instantiations within a process
 */
#if 0 /* CLEAN UNNECESSARY DATA STRUCTURE */
struct eda_event_instance_rec_tag;
#endif 

struct event_handle_rec_tag;
struct eda_subsc_rec_tag;
struct eda_channel_hdl_rec_tag;
struct eda_client_hdl_rec_tag;

#define EDA_EVT_PUBLISHED 0x1
#define EDA_EVT_RECEIVED  0x2

#if 0  /* CLEAN UNNECESSARY DATA STRUCTURE */
typedef struct eda_evt_inst_rec_tag
{
   uns32                evt_inst_id; /* allocated by the hdl manager */
   uns32                ret_evt_ch_oid; 
   uns32                pub_evt_id; /*The event ID sent to EDS ias part of Publish */
   uns32                del_evt_id; /* The event ID got from EDS as part of Delivery to this EDA*/
   uns32                parent_chan_hdl;
   struct eda_evt_inst_rec_tag *next;
} EDA_EVT_INST_REC;
#endif

typedef struct event_handle_rec_tag
{
   uns32                event_hdl; /* The hdl for this event allocated by the hdl-mgr */
   uns8                 priority;
   SaTimeT              retention_time;
   SaTimeT              publish_time; 
   SaNameT              publisher_name;
   SaEvtEventPatternArrayT *pattern_array;
   SaSizeT              event_data_size; /* size of the evt data */
   uns8                 *evt_data;       /* Event-specific data  */
   uns8                 evt_type;        /* published or rcvd     */
   struct eda_channel_hdl_rec_tag  *parent_chan;
   struct event_handle_rec_tag     *next;
   uns32                pub_evt_id; /*The event ID sent to EDS ias part of Publish */
   uns32                del_evt_id; /* The event ID got from EDS as part of Delivery to this EDA*/
#if 0  /* CLEAN UNNECESSARY DATA STRUCTURE */
   struct eda_evt_inst_rec_tag     *evt_inst_list;
#endif

} EDA_EVENT_HDL_REC;

typedef struct eda_subsc_rec_tag
{
   uns32    subsc_id;
   struct eda_subsc_rec_tag *next;
} EDA_SUBSC_REC;

/* Channel Handle Definition */
typedef struct eda_channel_hdl_rec_tag
{
   uns32             channel_hdl;     /* Channel HDL from handle mgr */
   SaNameT           channel_name;    /* channel name mentioned during open channel */
   uns32             open_flags;      /* channel open flags as defined in AIS.01.01 */
   uns32             eds_chan_id;     /* server reference for this channel */
   uns32             eds_chan_open_id;/* server reference for this instance of channel open */ 
   uns32             last_pub_evt_id; /* Last Published event ID */
   uns8              ulink; 
   struct event_handle_rec_tag    *chan_event_anchor;
   struct eda_subsc_rec_tag       *subsc_list;     /* List of subscriptions off this channel */
   struct eda_channel_hdl_rec_tag *next;
   struct eda_client_hdl_rec_tag  *parent_hdl; /* Back Pointer to the of the EDA client instantiation */
} EDA_CHANNEL_HDL_REC;


/* EDA handle database records */
typedef struct eda_client_hdl_rec_tag
{
   uns32             eds_reg_id;      /* handle value returned by EDS for this instantiation */
   uns32             local_hdl;       /* EVT handle (derived from hdl-mngr) */
   SaVersionT        version;
   SaEvtCallbacksT   reg_cbk;         /* callbacks registered by the application */
   EDA_CHANNEL_HDL_REC  *chan_list;
   SYSF_MBX          mbx;             /* priority q mbx b/w MDS & Library */  
   struct eda_client_hdl_rec_tag *next;
} EDA_CLIENT_HDL_REC;


typedef struct eda_eds_intf_tag
{
   MDS_HDL    mds_hdl;       /* mds handle */
   MDS_DEST   eda_mds_dest;  /* EDA absolute address */
   MDS_DEST   eds_mds_dest;  /* EDS absolute/virtual address */
   NCS_BOOL   eds_up;        /* Boolean to indicate that MDS subscription
                              * is complete
                              */
  NCS_BOOL    eds_up_publish;     /* Boolean to indicate that EDS is down */
} EDA_EDS_INTF;

typedef struct eda_cb_tag
{
   uns32         cb_hdl;                 /* CB hdl returned by hdl mngr */
   uns8          pool_id;                /* pool-id used by hdl mngr */
   uns32         prc_id;                 /* process identifier */
   EDA_EDS_INTF  eds_intf;               /* EDS interface (mds address & hdl)*/
   NCS_LOCK      cb_lock;                /* CB lock */
   EDA_CLIENT_HDL_REC *eda_init_rec_list;/* EDA client handle database */
   /* EDS EDA sync params */
   NCS_BOOL      eds_sync_awaited ;
   NCS_LOCK      eds_sync_lock;
   NCS_SEL_OBJ   eds_sync_sel; 
#if 0 
   /* Event Service Limits - Obtained from EDS */
   SaUint64T     max_chan;
   SaUint64T     max_evt_size;
   SaUint64T     max_ptrn_size;
   SaUint64T     max_num_ptrns;
   SaTimeT       max_ret_time;
#endif
   SaClmClusterChangesT node_status;
} EDA_CB;

/*** Extern function declarations ***/

EXTERN_C LEAPDLL_API uns32 ncs_eda_lib_req (NCS_LIB_REQ_INFO *);
EXTERN_C unsigned int ncs_eda_startup(void);
EXTERN_C unsigned int ncs_eda_shutdown(void);
EXTERN_C uns32 eda_create  (NCS_LIB_CREATE *);
EXTERN_C void  eda_destroy (NCS_LIB_DESTROY *);
NCS_BOOL eda_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg);

#endif /* !EDA_CB_H */
