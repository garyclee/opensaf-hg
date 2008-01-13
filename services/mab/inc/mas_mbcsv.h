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

  Contains the definitions for MAS redundancy interface with MBCSv. 

*******************************************************************************/

/*
 * Module Inclusion Control...
 */
#ifndef MAS_MBCSV_H
#define MAS_MBCSV_H

#if (NCS_MAS_RED == 1)

/* Include MBCSv Interface for redundancy */
#include "mbcsv_papi.h"

struct mas_tbl;

#define m_NCS_MAS_MBCSV_VERSION_MIN 1

/* MAS MBCSv version number */ 
#define m_NCS_MAS_MBCSV_VERSION 2

/* different types of Async Updates in MAS */ 
typedef enum mas_async_update_type_tag
{
    /* for OAA down */ 
    MAS_ASYNC_OAA_DOWN = 1, 

    /* for Filter registrations and de-registrations */ 
    MAS_ASYNC_REG_UNREG, 

    /* NCSMIB_ARG requests (get, set, etc) - Currently not used */
    MAS_ASYNC_MIB_ARG, 

    MAS_ASYNC_MAX
}MAS_ASYNC_UPDATE_TYPE; 

/* context information used in the warm-sync */ 
typedef struct warm_sync_cntxt_tag
{
    /* bkt_list: <bkt-id><bkt-id>.... */
    uns8    *bkt_list; 

    /* bucket-id to be sent to the STANDBY */
    uns8    bkt_count; 
}MAS_WARM_SYNC_CNTXT;

/* redundancy attributes */ 
typedef struct masv_red
{
    /* is this bucket synced or not? */ 
    NCS_BOOL    this_bucket_synced[MAB_MIB_ID_HASH_TBL_SIZE]; 

    /* number of async updates on this bucket */ 
    uns32       async_count[MAB_MIB_ID_HASH_TBL_SIZE];

    /* number of OAA down messages sent to Standby MAS */ 
    uns32       oaa_down_async_count; 

    /* HA state of this CSI */ 
    /* this is duplicate of HA State in MAS_CSI_NODE data structure */ 
    NCS_APP_AMF_HA_STATE    ha_state; 

    /* Is cold sync complete? */
    NCS_BOOL    cold_sync_done; 

    /* Is warm-sync in progress? */ 
    NCS_BOOL    warm_sync_in_progress; 

    /*checkpoint handle */
    uns32       ckpt_hdl;

    /* MBCSv Handle */ 
    uns32       mbcsv_hdl;

    /* this message will be processed by STANDBY MAS in two cases 
     * 1. During a failover
     * 2. After getting a Go-Ahead message from Active MAS for this message
     */
    MAB_MSG         *process_msg; 
}MAS_RED; 

/* MBCSV Interface attributes */ 
typedef struct mas_mbcsv_attribs_tag
{
    /* selction object for MBCSv Interface */
    SaSelectionObjectT   mbcsv_sel_obj; 

    /* dispatch flags for MBCSv Interface */
    SaDispatchFlagsT     mbcsv_disp_flags; 

    /* MAS Version */ 
    uns16               masv_version; 

    /*MBCSv Handle */ 
    NCS_MBCSV_HDL       mbcsv_hdl; 
}MAS_MBCSV_ATTRIBS;  

/* Initialize the MBCSv Interface and get the selection object */ 
EXTERN_C uns32
mas_mbcsv_interface_initialize(MAS_MBCSV_ATTRIBS *mbcsv_attribs); 

/* MAS - MBCSv callback function */ 
EXTERN_C uns32 
mas_mbcsv_cb(NCS_MBCSV_CB_ARG* arg);

/* Active MAS, Async Update API */ 
EXTERN_C uns32
mas_red_sync(struct mas_tbl* inst, MAB_MSG* msg, MAS_ASYNC_UPDATE_TYPE type); 

/* send the async done message to the standby MAS */
EXTERN_C uns32 
mas_red_sync_done(struct mas_tbl* inst, MAS_ASYNC_UPDATE_TYPE type); 

/* dump masv dictionary */ 
EXTERN_C void
mas_dictionary_dump(void); 
#endif /* #if (NCS_MAS_RED == 1) */ 
#endif /* MAS_MBCSV_H */ 

