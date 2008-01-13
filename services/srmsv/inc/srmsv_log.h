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

  This file contains logging offset indices that are used for logging SRMSv 
  common information.
..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

#ifndef SRMSV_LOG_H
#define SRMSV_LOG_H

/******************************************************************************
                      Logging offset indices for SE-API
******************************************************************************/
typedef enum srmsv_log_seapi {
   SRMSV_LOG_SEAPI_CREATE,
   SRMSV_LOG_SEAPI_DESTROY,
   SRMSV_LOG_SEAPI_SUCCESS,
   SRMSV_LOG_SEAPI_FAILURE,
   SRMSV_LOG_SEAPI_MAX
} SRMSV_LOG_SEAPI;

/******************************************************************************
                      Logging offset indices for MDS
******************************************************************************/
typedef enum srmsv_log_mds {
   SRMSV_LOG_MDS_REG,
   SRMSV_LOG_MDS_INSTALL,
   SRMSV_LOG_MDS_SUBSCRIBE,
   SRMSV_LOG_MDS_UNREG,
   SRMSV_LOG_MDS_ADEST_CRT,
   SRMSV_LOG_MDS_SEND,
   SRMSV_LOG_MDS_PRM_GET,
   SRMSV_LOG_MDS_RCV_CBK,
   SRMSV_LOG_MDS_CPY_CBK,
   SRMSV_LOG_MDS_SVEVT_CBK,
   SRMSV_LOG_MDS_ENC_CBK,
   SRMSV_LOG_MDS_FLAT_ENC_CBK,
   SRMSV_LOG_MDS_DEC_CBK,
   SRMSV_LOG_MDS_FLAT_DEC_CBK,
   SRMSV_LOG_MDS_SUCCESS,
   SRMSV_LOG_MDS_FAILURE,
   SRMSV_LOG_MDS_MAX
} SRMSV_LOG_MDS;

/******************************************************************************
                        Logging offset indices for EDU
******************************************************************************/
typedef enum srmsv_log_edu {
   SRMSV_LOG_EDU_INIT,
   SRMSV_LOG_EDU_FINALIZE,
   SRMSV_LOG_EDU_SUCCESS,
   SRMSV_LOG_EDU_FAILURE,
   SRMSV_LOG_EDU_MAX
} SRMSV_LOG_EDU;

/******************************************************************************
                        Logging offset indices for LOCK
******************************************************************************/
typedef enum srmsv_log_lock {
   SRMSV_LOG_LOCK_INIT,
   SRMSV_LOG_LOCK_DESTROY,
   SRMSV_LOG_LOCK_TAKE,
   SRMSV_LOG_LOCK_GIVE,
   SRMSV_LOG_LOCK_SUCCESS,
   SRMSV_LOG_LOCK_FAILURE,
   SRMSV_LOG_LOCK_MAX
} SRMSV_LOG_LOCK;

/******************************************************************************
                     Logging offset indices for control block
******************************************************************************/
typedef enum srmsv_log_cb {
   SRMSV_LOG_CB_CREATE,
   SRMSV_LOG_CB_DESTROY,
   SRMSV_LOG_CB_RETRIEVE,
   SRMSV_LOG_CB_RETURN,
   SRMSV_LOG_CB_SUCCESS,
   SRMSV_LOG_CB_FAILURE,
   SRMSV_LOG_CB_MAX
} SRMSV_LOG_CB;

/******************************************************************************
                     Logging offset indices for mailbox
******************************************************************************/
typedef enum srmsv_log_mbx {
   SRMSV_LOG_MBX_CREATE,
   SRMSV_LOG_MBX_ATTACH,
   SRMSV_LOG_MBX_DESTROY,
   SRMSV_LOG_MBX_DETACH,
   SRMSV_LOG_MBX_SEND,
   SRMSV_LOG_MBX_SUCCESS,
   SRMSV_LOG_MBX_FAILURE,
   SRMSV_LOG_MBX_MAX
} SRMSV_LOG_MBX;

/******************************************************************************
                     Logging offset indices for task
******************************************************************************/
typedef enum srmsv_log_task {
   SRMSV_LOG_TASK_CREATE,
   SRMSV_LOG_TASK_START,
   SRMSV_LOG_TASK_RELEASE,
   SRMSV_LOG_IPC_CREATE,  
   SRMSV_LOG_IPC_ATTACH,
   SRMSV_LOG_IPC_SEND,
   SRMSV_LOG_IPC_DETACH,
   SRMSV_LOG_IPC_RELEASE,  
   SRMSV_LOG_TIM_SUCCESS,
   SRMSV_LOG_TIM_FAILURE,
   SRMSV_LOG_TIM_MAX
} SRMSV_LOG_TASK;

/******************************************************************************
                     Logging offset indices for patricia tree
******************************************************************************/
typedef enum srmsv_log_pat {
   SRMSV_LOG_PAT_INIT,
   SRMSV_LOG_PAT_DESTROY,
   SRMSV_LOG_PAT_ADD,
   SRMSV_LOG_PAT_DEL,
   SRMSV_LOG_PAT_SUCCESS,
   SRMSV_LOG_PAT_FAILURE,
   SRMSV_LOG_PAT_MAX
} SRMSV_LOG_PAT;

/******************************************************************************
                Logging offset indices for Handle Manager associations
******************************************************************************/
typedef enum srmsv_log_hdl {
   SRMSV_LOG_HDL_CREATE_CB,
   SRMSV_LOG_HDL_DESTROY_CB,
   SRMSV_LOG_HDL_RETRIEVE_CB,
   SRMSV_LOG_HDL_RETURN_CB,
   SRMSV_LOG_HDL_CREATE_RSRC_MON,
   SRMSV_LOG_HDL_DESTROY_RSRC_MON,
   SRMSV_LOG_HDL_RETRIEVE_RSRC_MON,
   SRMSV_LOG_HDL_RETURN_RSRC_MON,
   SRMSV_LOG_HDL_CREATE_USER,
   SRMSV_LOG_HDL_DESTROY_USER,
   SRMSV_LOG_HDL_RETRIEVE_USER,
   SRMSV_LOG_HDL_RETURN_USER,
   SRMSV_LOG_HDL_SUCCESS,
   SRMSV_LOG_HDL_FAILURE,
   SRMSV_LOG_HDL_MAX
} SRMSV_LOG_HDL;

/******************************************************************************
                    Logging offset indices for Memory
******************************************************************************/
typedef enum srmsv_log_mem
{
   SRMSV_MEM_SRMA_MSG,
   SRMSV_MEM_SRMA_CRT_RSRC,
   SRMSV_MEM_SRMND_MSG,
   SRMSV_MEM_SRMND_CRTD_RSRC,
   SRMSV_MEM_ALLOC_SUCCESS,
   SRMSV_MEM_ALLOC_FAILED,
   SRMSV_MEM_ALLOC_MAX
} SRMSV_LOG_MEM;


/******************************************************************************
                    Logging offset indices for Timer
******************************************************************************/
typedef enum srmsv_log_tmr
{
   SRMSV_LOG_TMR_INIT,
   SRMSV_LOG_TMR_DESTROY,
   SRMSV_LOG_TMR_EXP,
   SRMSV_LOG_TMR_STOP,
   SRMSV_LOG_TMR_START,
   SRMSV_LOG_TMR_CREATE,
   SRMSV_LOG_TMR_DELETE,
   SRMSV_LOG_TMR_SUCCESS,
   SRMSV_LOG_TMR_FAILURE,
   SRMSV_LOG_TMR_MAX
} SRMSV_LOG_TMR;

/******************************************************************************
                Logging offset indices for Resource Monitor cfgs
******************************************************************************/
typedef enum srmsv_log_rsrc_mon {
   SRMSV_LOG_RSRC_MON_CREATE,
   SRMSV_LOG_RSRC_MON_DELETE,
   SRMSV_LOG_RSRC_MON_SUBSCR,
   SRMSV_LOG_RSRC_MON_UNSUBSCR,
   SRMSV_LOG_RSRC_MON_MODIFY,
   SRMSV_LOG_RSRC_MON_BULK_CREATE,
   SRMSV_LOG_RSRC_MON_SUCCESS,
   SRMSV_LOG_RSRC_MON_FAILED,
   SRMSV_LOG_RSRC_MON_MAX
} SRMSV_LOG_RSRC_MON;


/* DTSv versioning changes */
#define SRMSV_LOG_VERSION 2

#endif /* !SRMSV_LOG_H */


