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
 */

/*****************************************************************************
  DESCRIPTION:

  This module contains function protoype for the fake ss and also the data 
  structures used by fake ss..

*******************************************************************************/

#include <stdio.h>
#include <opensaf/ncs_lib.h>
#include <opensaf/ncsgl_defs.h>
#include <opensaf/os_defs.h>
#include <opensaf/ncs_main_papi.h>
#include <opensaf/ncssysf_tsk.h>
#include <opensaf/ncs_svd.h>
#include <opensaf/mbcsv_papi.h>
#include <opensaf/ncssysf_lck.h>
#include <opensaf/ncssysf_def.h>

#define  TIME_MULT         100
#define  MBCSV_DUMMY_SVC1  UD_DTSV_DEMO_SERVICE1
#define  MBCSV_DUMMY_SVC2  UD_DTSV_DEMO_SERVICE2

#define FAKE_SS_DATA_SIZE  (5 * sizeof(uns32))

#define DEMO_MBCSV_VERSION_ONE 1
#define DEMO_MBCSV_VERSION_TWO 2

typedef struct mbcsv_fake_ss_create_parms
{
    SS_SVC_ID           svc_id;
    SaAmfHAStateT       role;
}MBCSV_FAKE_SS_CREATE_PARMS;


typedef struct mbcsv_fake_ss_config
{
    SS_SVC_ID     svc_id;
    char          *vdest_name;
    uns32         role;
    uns16 version;
}MBCSV_FAKE_SS_CONFIG;


typedef struct mbcsv_time
{
    uns32      seconds;
    uns32      millisecs;
}MBCSV_TIME;

typedef struct fake_ss_data_struct
{
   NCS_BOOL         exist;
   uns32            rand_value;
   MBCSV_TIME       time;
}FAKE_SS_DATA_STRUCT;


typedef struct fake_ss_ckpt
{
   FAKE_SS_DATA_STRUCT    my_data[10002];
   SaAmfHAStateT          ha_state;
   struct fake_ss_cb      *cb_ptr;
   uns32                   data_to_send;
   /*
    * Run-time perameters.
    */
   uns32                  pwe_hdl;
   uns32                  ckpt_hdl;
   uns32                  cold_sync_elem_sent;
   uns32                  warm_sync_elem_sent;
   uns32                  cold_sync_cmplt_sent;
   uns32                  in_cold_sync;
   uns32                  in_warm_sync;
}FAKE_SS_CKPT;

typedef struct fake_ss_cb
{
   NCS_LOCK               fake_ss_lock;
   uns32                  svc_id;
   SaSelectionObjectT     sel_obj;
   uns32                  mbcsv_hdl;
   uns16                  my_version;
   void*                  task_hdl;

   FAKE_SS_CKPT           my_ckpt[3];

}FAKE_SS_CB;

EXTERN_C void create_fake_ss(void * arg);
EXTERN_C void mbcsv_print_fake_ss_data(void);
EXTERN_C uns32 mbcsv_set_ckpt_role(uns32    mbcsv_hdl,
                                  uns32    ckpt_hdl,
                                  uns32    role);
