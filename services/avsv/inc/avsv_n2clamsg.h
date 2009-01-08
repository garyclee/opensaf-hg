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

  This include file contains the message definitions for CLA and AvND
  communication.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVSV_N2CLAMSG_H
#define AVSV_N2CLAMSG_H


/* Message format versions */
#define AVSV_AVND_CLA_MSG_FMT_VER_1    1

/* clm track info */
typedef struct cla_track_tag
{
   SaClmClusterNotificationT *notify; /* buffer provided by the appl */
   uns16                     num;     /* no of items */
} CLA_TRACK;

/* CLM API enums */
typedef enum avsv_nda_cla_msg_type
{
   AVSV_CLA_API_MSG = 1,
   AVSV_AVND_CLM_CBK_MSG,
   AVSV_AVND_CLM_API_RESP_MSG,
   AVSV_NDA_CLA_MSG_MAX
} AVSV_NDA_CLA_MSG_TYPE;

/* API param definition */
typedef struct avsv_clm_api_info
{
   uns32              prc_id;
   MDS_DEST           dest; /* mds dest */
   AVSV_CLM_API_TYPE  type; /* api type */
   NCS_BOOL           is_sync_api;
   union {
      AVSV_CLM_FINALIZE_PARAM       finalize;
      AVSV_CLM_TRACK_START_PARAM    track_start;
      AVSV_CLM_TRACK_STOP_PARAM     track_stop;
      AVSV_CLM_NODE_GET_PARAM       node_get;
      AVSV_CLM_NODE_ASYNC_GET_PARAM node_async_get;
   } param;
} AVSV_CLM_API_INFO;

/* API response definition */
typedef struct avsv_clm_api_resp_info
{
   AVSV_CLM_API_TYPE type; /* api type */
   SaAisErrorT          rc;   /* return code */
   union {
      SaClmClusterNodeT        node_get;
      SaInvocationT            inv;
      CLA_TRACK                track;
   } param;
} AVSV_CLM_API_RESP_INFO;

/* message used for AvND-CLA interaction */
typedef struct avsv_nda_cla_msg
{
   AVSV_NDA_CLA_MSG_TYPE    type;   /* message type */

   union {
      /* elements encoded by CLA (& decoded by AvND) */
      AVSV_CLM_API_INFO   api_info;       /* api info */

      /* elements encoded by AvND (& decoded by CLA) */
      AVSV_CLM_CBK_INFO       cbk_info;      /* callbk info */
      AVSV_CLM_API_RESP_INFO  api_resp_info;  /* api response info */
   } info;
} AVSV_NDA_CLA_MSG;


/* Extern Fuction Prototypes */

EXTERN_C void avsv_nda_cla_msg_free (AVSV_NDA_CLA_MSG *);

EXTERN_C uns32 avsv_nda_cla_msg_copy (AVSV_NDA_CLA_MSG *, AVSV_NDA_CLA_MSG *);

#endif /* !AVSV_N2CLAMSG_H */
