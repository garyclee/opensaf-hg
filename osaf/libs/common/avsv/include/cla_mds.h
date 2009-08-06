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

  CLA - AvND communication related definitions.
  
******************************************************************************
*/

#ifndef CLA_MDS_H
#define CLA_MDS_H


/* In Service upgrade support */
#define CLA_MDS_SUB_PART_VERSION   1

#define CLA_AVND_SUBPART_VER_MIN   1
#define CLA_AVND_SUBPART_VER_MAX   1

/* 
 * Wrapper structure that encapsulates communication 
 * semantics for communication with AvND
 */
typedef struct cla_avnd_intf_tag
{
   MDS_HDL    mds_hdl;       /* mds handle */
   MDS_DEST   cla_mds_dest;  /* CLA absolute address */
   MDS_DEST   avnd_mds_dest; /* AvND absolute address */
   NCS_BOOL   avnd_up;
} CLA_AVND_INTF;


/* Macro to validate the CLM version */
#define m_CLA_VER_IS_VALID(ver) \
   ( (ver->releaseCode == 'B') && \
     (ver->majorVersion <= 0x1) )


/* Macro to validate the dispatch flags */
#define m_CLA_DISPATCH_FLAG_IS_VALID(flag) \
   ( (SA_DISPATCH_ONE == flag) || \
     (SA_DISPATCH_ALL == flag) || \
     (SA_DISPATCH_BLOCKING == flag) )

/*****************************************************************************
                 Macros to fill the MDS message structure
*****************************************************************************/
/* Macro to populate the 'CLM Initialize' message */
#define m_CLA_CLM_INIT_MSG_FILL(m, pid) \
{ \
   memset(&(m), 0, sizeof(AVSV_NDA_CLA_MSG)); \
   (m).type = AVSV_CLA_API_MSG; \
   (m).info.api_info.type = AVSV_CLM_INITIALIZE; \
   (m).info.api_info.prc_id = (pid); \
}

/* Macro to populate the 'CLM Finalize' message */
#define m_CLA_CLM_FINALIZE_MSG_FILL(m, pid, hd) \
{ \
   memset(&(m), 0, sizeof(AVSV_NDA_CLA_MSG)); \
   (m).type = AVSV_CLA_API_MSG; \
   (m).info.api_info.type = AVSV_CLM_FINALIZE; \
   (m).info.api_info.prc_id = (pid); \
   (m).info.api_info.param.finalize.hdl = (hd); \
}

/* Macro to populate the 'CLM track start' message */
#define m_CLA_TRACK_START_MSG_FILL(m, pid, hd, fl) \
{ \
   memset(&(m), 0, sizeof(AVSV_NDA_CLA_MSG)); \
   (m).type = AVSV_CLA_API_MSG; \
   (m).info.api_info.type = AVSV_CLM_TRACK_START; \
   (m).info.api_info.prc_id = (pid); \
   (m).info.api_info.param.track_start.hdl = (hd); \
   (m).info.api_info.param.track_start.flags = (fl); \
}

/* Macro to populate the 'CLM track stop' message */
#define m_CLA_TRACK_STOP_MSG_FILL(m, pid, hd) \
{ \
   memset(&(m), 0, sizeof(AVSV_NDA_CLA_MSG)); \
   (m).type = AVSV_CLA_API_MSG; \
   (m).info.api_info.type = AVSV_CLM_TRACK_STOP; \
   (m).info.api_info.prc_id = (pid); \
   (m).info.api_info.param.track_stop.hdl = (hd); \
}

/* Macro to populate the 'CLM node get' message */
#define m_CLA_NODE_GET_MSG_FILL(m, pid, hd, nid) \
{ \
   memset(&(m), 0, sizeof(AVSV_NDA_CLA_MSG)); \
   (m).type = AVSV_CLA_API_MSG; \
   (m).info.api_info.type = AVSV_CLM_NODE_GET; \
   (m).info.api_info.prc_id = (pid); \
   (m).info.api_info.param.node_get.hdl = (hd); \
   (m).info.api_info.param.node_get.node_id = (nid); \
}

/* Macro to populate the 'CLM node async get' message */
#define m_CLA_NODE_ASYNC_GET_MSG_FILL(m, pid, hd, in, nid) \
{ \
   memset(&(m), 0, sizeof(AVSV_NDA_CLA_MSG)); \
   (m).type = AVSV_CLA_API_MSG; \
   (m).info.api_info.type = AVSV_CLM_NODE_ASYNC_GET; \
   (m).info.api_info.prc_id = (pid); \
   (m).info.api_info.param.node_async_get.hdl = (hd); \
   (m).info.api_info.param.node_async_get.inv = (in); \
   (m).info.api_info.param.node_async_get.node_id = (nid); \
}


/*** Extern function declarations ***/
struct cla_cb_tag;
EXTERN_C uns32 cla_mds_reg (struct cla_cb_tag *);

EXTERN_C uns32 cla_mds_unreg (struct cla_cb_tag *);

EXTERN_C uns32 cla_mds_cbk (NCSMDS_CALLBACK_INFO *);

EXTERN_C uns32 cla_mds_send (struct cla_cb_tag *, AVSV_NDA_CLA_MSG *, 
                             AVSV_NDA_CLA_MSG **, uns32 timeout);

#endif /* !CLA_MDS_H */
