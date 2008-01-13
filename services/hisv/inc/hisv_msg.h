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

  $Header

..............................................................................

  DESCRIPTION:

  This include file contains the message definitions for HPL and HAM
  communication.

******************************************************************************
*/

/*
* Module Inclusion Control...
*/
#ifndef HISV_MSG_H
#define HISV_MSG_H

#include "hpl_api.h"

#define HISV_MSG_FMT_VER 1

/* HAM MDS status */
typedef enum ham_mds_status
{
   HAM_MDS_DOWN,              /* HAM went down */
   HAM_MDS_UP                 /* HAM is up with MDS registration done */
} HAM_MDS_STATUS;


/* structure for HPL API information to HAM */
/* 'data' Format is {"uns16-Type" - "uns16-Len" - uns16-Value"} */
typedef struct hisv_api_info
{
   HISV_API_CMD        cmd;    /* HPL command */
   uns32               arg;    /* argument to command */
   uns32               data_len;  /* length of input data */
   uns8               *data;   /* type, length and value of data fields */

} HISV_API_INFO;

/* return types for HPL call backs */
typedef enum hpl_ret_type
{
   HPL_HAM_VDEST_RET,
   HPL_GENERIC_DATA
} HPL_RET_TYPE;

/* structre information passed by HAM to HPL through chalback */
typedef struct hisv_vdest_info
{
   uns32               chassis_id;   /* chassis identifer */
   MDS_DEST            ham_dest;     /* VDEST of HAM instance */
   HAM_MDS_STATUS      ham_status;   /* HAM MDS UP/DOWN */
   uns8                status;       /* return status */
} HISV_HAM_VDEST;

/* structre information passed by HAM to HPL through chalback */
typedef struct hpl_ret_val
{
   uns32               ret_val;      /* return value, success/failure */
   uns32               data_len;     /* length of output data */
   uns8               *data;         /* generic return data */
} HPL_RET_VAL;

/* return values from HAM to HPL through chalback */
typedef struct hisv_hpl_ret
{
   uns32      ret_type;
   union
   {
      HISV_HAM_VDEST  h_vdest;
      HPL_RET_VAL  h_gen;
   }hpl_ret;
} HISV_HPL_RET;

/* message used for HPL-HAM interaction */
typedef struct hisv_msg
{
   union
   {
      /* elements encoded by HPL (& decoded by HAM) */
      HISV_API_INFO            api_info;       /* api info */

      /* elements encoded by HAM (& decoded at HPL) */
      HISV_HPL_RET          cbk_info;       /* callbk info */
   } info;
} HISV_MSG;

/* HISV event as received and constructed at HAM */
typedef struct hisv_evt_tag
{
   /* NCS_IPC_MSG       next; */
   struct hisv_evt_tag *next;
   uns32             cb_hdl;
   MDS_SYNC_SND_CTXT mds_ctxt; /* Relevant when this event has to be responded to
                                * in a synchronous fashion.
                                */
   MDS_SVC_ID        to_svc;  /* The service at the destination */
   MDS_DEST          fr_dest;
   NODE_ID           fr_node_id;
   MDS_SEND_PRIORITY_TYPE rcvd_prio; /* Priority of the recvd evt */
   uns32     evt_type;

   HISV_MSG     msg;

}HISV_EVT;

/* struct for encoding HPL data w.r.to type, lenght and value */
typedef struct hpl_tlv
{
   uns16 d_type;
   uns16 d_len;
} HPL_TLV;

/* different type of data encoded in hisv_msg input */
typedef enum hpl_d_type
{
   ENTITY_PATH,
   POWER_STATE,
   RESET_TYPE,
   HOTSWAP_CONFIG,
   HOTSWAP_MANAGE,
   ALARM_TYPE,
   ALARM_SEVERITY,
   EVLOG_TIME
} HPL_D_TYPE;


#endif /* !HISV_MSG_H*/




