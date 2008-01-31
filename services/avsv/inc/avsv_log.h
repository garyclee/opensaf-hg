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

  This file contains logging offset indices that are used for logging AvSv 
  common information.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#ifndef AVSV_LOG_H
#define AVSV_LOG_H

/******************************************************************************
                      Logging offset indices for SE-API
 ******************************************************************************/
typedef enum avsv_log_seapi {
   AVSV_LOG_SEAPI_CREATE,
   AVSV_LOG_SEAPI_DESTROY,
   AVSV_LOG_SEAPI_SUCCESS,
   AVSV_LOG_SEAPI_FAILURE,
   AVSV_LOG_SEAPI_MAX
} AVSV_LOG_SEAPI;


/******************************************************************************
                      Logging offset indices for MDS
 ******************************************************************************/
typedef enum avsv_log_mds {
   AVSV_LOG_MDS_REG,
   AVSV_LOG_MDS_INSTALL,
   AVSV_LOG_MDS_SUBSCRIBE,
   AVSV_LOG_MDS_UNREG,
   AVSV_LOG_MDS_VDEST_CRT,
   AVSV_LOG_MDS_VDEST_ROL,
   AVSV_LOG_MDS_SEND,
   AVSV_LOG_MDS_PRM_GET,
   AVSV_LOG_MDS_RCV_CBK,
   AVSV_LOG_MDS_CPY_CBK,
   AVSV_LOG_MDS_SVEVT_CBK,
   AVSV_LOG_MDS_ENC_CBK,
   AVSV_LOG_MDS_FLAT_ENC_CBK,
   AVSV_LOG_MDS_DEC_CBK,
   AVSV_LOG_MDS_FLAT_DEC_CBK,
   AVSV_LOG_MDS_SUCCESS,
   AVSV_LOG_MDS_FAILURE,
   AVSV_LOG_MDS_MAX
} AVSV_LOG_MDS;


/******************************************************************************
                      Logging offset indices for SRM
 ******************************************************************************/
typedef enum avsv_log_srm {
   AVSV_LOG_SRM_REG,
   AVSV_LOG_SRM_FINALIZE,
   AVSV_LOG_SRM_UNREG,
   AVSV_LOG_SRM_MON_START,
   AVSV_LOG_SRM_MON_STOP,
   AVSV_LOG_SRM_CALLBACK,
   AVSV_LOG_SRM_DISPATCH,
   AVSV_LOG_SRM_SUCCESS,
   AVSV_LOG_SRM_FAILURE,
   AVSV_LOG_SRM_MAX
} AVSV_LOG_SRM;

/******************************************************************************
                        Logging offset indices for EDU
 ******************************************************************************/
typedef enum avsv_log_edu {
   AVSV_LOG_EDU_INIT,
   AVSV_LOG_EDU_FINALIZE,
   AVSV_LOG_EDU_SUCCESS,
   AVSV_LOG_EDU_FAILURE,
   AVSV_LOG_EDU_MAX
} AVSV_LOG_EDU;


/******************************************************************************
                        Logging offset indices for LOCK
 ******************************************************************************/
typedef enum avsv_log_lock {
   AVSV_LOG_LOCK_INIT,
   AVSV_LOG_LOCK_FINALIZE,
   AVSV_LOG_LOCK_TAKE,
   AVSV_LOG_LOCK_GIVE,
   AVSV_LOG_LOCK_SUCCESS,
   AVSV_LOG_LOCK_FAILURE,
   AVSV_LOG_LOCK_MAX
} AVSV_LOG_LOCK;


/******************************************************************************
                     Logging offset indices for control block
 ******************************************************************************/
typedef enum avsv_log_cb {
   AVSV_LOG_CB_CREATE,
   AVSV_LOG_CB_DESTROY,
   AVSV_LOG_CB_HDL_ASS_CREATE,
   AVSV_LOG_CB_HDL_ASS_REMOVE,
   AVSV_LOG_CB_RETRIEVE,
   AVSV_LOG_CB_RETURN,
   AVSV_LOG_CB_SUCCESS,
   AVSV_LOG_CB_FAILURE,
   AVSV_LOG_CB_MAX
} AVSV_LOG_CB;


/******************************************************************************
                     Logging offset indices for mailbox
 ******************************************************************************/
typedef enum avsv_log_mbx {
   AVSV_LOG_MBX_CREATE,
   AVSV_LOG_MBX_ATTACH,
   AVSV_LOG_MBX_DESTROY,
   AVSV_LOG_MBX_DETACH,
   AVSV_LOG_MBX_SEND,
   AVSV_LOG_MBX_SUCCESS,
   AVSV_LOG_MBX_FAILURE,
   AVSV_LOG_MBX_MAX
} AVSV_LOG_MBX;


/******************************************************************************
                     Logging offset indices for task
 ******************************************************************************/
typedef enum avsv_log_task {
   AVSV_LOG_TASK_CREATE,
   AVSV_LOG_TASK_START,
   AVSV_LOG_TASK_RELEASE,
   AVSV_LOG_TASK_SUCCESS,
   AVSV_LOG_TASK_FAILURE,
   AVSV_LOG_TASK_MAX
} AVSV_LOG_TASK;


/******************************************************************************
                     Logging offset indices for patricia tree
 ******************************************************************************/
typedef enum avsv_log_pat {
   AVSV_LOG_PAT_INIT,
   AVSV_LOG_PAT_ADD,
   AVSV_LOG_PAT_DEL,
   AVSV_LOG_PAT_SUCCESS,
   AVSV_LOG_PAT_FAILURE,
   AVSV_LOG_PAT_MAX
} AVSV_LOG_PAT;


/******************************************************************************
                     Logging offset indices for **************************
                       Canned string for 
 ******************************************************************************/
typedef enum avsv_log_amf_cbk {
   AVSV_LOG_AMF_HC,
   AVSV_LOG_AMF_COMP_TERM,
   AVSV_LOG_AMF_CSI_SET,
   AVSV_LOG_AMF_CSI_REM,
   AVSV_LOG_AMF_PG_TRACK,
   AVSV_LOG_AMF_PXIED_COMP_INST,
   AVSV_LOG_AMF_PXIED_COMP_CLEAN,
} AVSV_LOG_AMF_CBK;

/* DTSv versioning support */
#define AVSV_LOG_VERSION 1

#endif /* !AVSV_LOG_H */
