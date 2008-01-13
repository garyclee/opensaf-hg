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

  This file contains logging offset indices that are used for logging CLA 
  information.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#ifndef CLA_LOG_H
#define CLA_LOG_H

#include "avsv_log.h"

/******************************************************************************
                    Logging offset indices for selection object
 ******************************************************************************/
typedef enum cla_log_sel_obj {
   CLA_LOG_SEL_OBJ_CREATE,
   CLA_LOG_SEL_OBJ_DESTROY,
   CLA_LOG_SEL_OBJ_IND_SEND,
   CLA_LOG_SEL_OBJ_IND_REM,
   CLA_LOG_SEL_OBJ_SUCCESS,
   CLA_LOG_SEL_OBJ_FAILURE,
   CLA_LOG_SEL_OBJ_MAX
} CLA_LOG_SEL_OBJ;


/******************************************************************************
                       Logging offset indices for CLM APIs
 ******************************************************************************/
/* ensure that the this ordering matches that of API type definition */
typedef enum cla_log_api {
   CLA_LOG_API_INITIALIZE,
   CLA_LOG_API_FINALIZE,
   CLA_LOG_API_TRACK_START,
   CLA_LOG_API_TRACK_STOP,
   CLA_LOG_API_NODE_GET,
   CLA_LOG_API_NODE_ASYNC_GET,
   CLA_LOG_API_SEL_OBJ_GET,
   CLA_LOG_API_DISPATCH,
   CLA_LOG_API_ERR_SA_OK,
   CLA_LOG_API_ERR_SA_LIBRARY,
   CLA_LOG_API_ERR_SA_VERSION,
   CLA_LOG_API_ERR_SA_INIT,
   CLA_LOG_API_ERR_SA_TIMEOUT,
   CLA_LOG_API_ERR_SA_TRY_AGAIN,
   CLA_LOG_API_ERR_SA_INVALID_PARAM,
   CLA_LOG_API_ERR_SA_NO_MEMORY,
   CLA_LOG_API_ERR_SA_BAD_HANDLE,
   CLA_LOG_API_ERR_SA_BUSY,
   CLA_LOG_API_ERR_SA_ACCESS,
   CLA_LOG_API_ERR_SA_NOT_EXIST,
   CLA_LOG_API_ERR_SA_NAME_TOO_LONG,
   CLA_LOG_API_ERR_SA_EXIST,
   CLA_LOG_API_ERR_SA_NO_SPACE,
   CLA_LOG_API_ERR_SA_INTERRUPT,
   CLA_LOG_API_ERR_SA_SYSTEM,
   CLA_LOG_API_ERR_SA_NAME_NOT_FOUND,
   CLA_LOG_API_ERR_SA_NO_RESOURCES,
   CLA_LOG_API_ERR_SA_NOT_SUPPORTED,
   CLA_LOG_API_ERR_SA_BAD_OPERATION,
   CLA_LOG_API_ERR_SA_FAILED_OPERATION,
   CLA_LOG_API_ERR_SA_MESSAGE_ERROR,
   CLA_LOG_API_ERR_SA_NO_MESSAGE,
   CLA_LOG_API_ERR_SA_QUEUE_FULL,
   CLA_LOG_API_ERR_SA_QUEUE_NOT_CLAILABLE,
   CLA_LOG_API_ERR_SA_BAD_CHECKPOINT,
   CLA_LOG_API_ERR_SA_BAD_FLAGS,
   CLA_LOG_API_MAX
} CLA_LOG_API;


/******************************************************************************
                Logging offset indices for CLA handle database
 ******************************************************************************/
typedef enum cla_log_hdl_db {
   CLA_LOG_HDL_DB_CREATE,
   CLA_LOG_HDL_DB_DESTROY,
   CLA_LOG_HDL_DB_REC_ADD,
   CLA_LOG_HDL_DB_REC_CBK_ADD,
   CLA_LOG_HDL_DB_REC_DEL,
   CLA_LOG_HDL_DB_SUCCESS,
   CLA_LOG_HDL_DB_FAILURE,
   CLA_LOG_HDL_DB_MAX
} CLA_LOG_HDL_DB;


/******************************************************************************
              Logging offset indices for miscellaneous CLA events
 ******************************************************************************/
typedef enum cla_log_misc {
   CLA_LOG_MISC_AVND_UP,
   CLA_LOG_MISC_AVND_DN,

   /* keep adding misc logs here */

   CLA_LOG_MISC_MAX
} CLA_LOG_MISC;


/******************************************************************************
 Logging offset indices for canned constant strings for the ASCII SPEC
 ******************************************************************************/
typedef enum cla_flex_sets {
   CLA_FC_SEAPI,
   CLA_FC_MDS,
   CLA_FC_EDU,
   CLA_FC_LOCK,
   CLA_FC_CB,
   CLA_FC_CBK,
   CLA_FC_SEL_OBJ,
   CLA_FC_API,
   CLA_FC_HDL_DB,
   CLA_FC_MISC,
} CLA_FLEX_SETS;

typedef enum cla_log_ids {
   CLA_LID_SEAPI,
   CLA_LID_MDS,
   CLA_LID_EDU,
   CLA_LID_LOCK,
   CLA_LID_CB,
   CLA_LID_CBK,
   CLA_LID_SEL_OBJ,
   CLA_LID_API,
   CLA_LID_HDL_DB,
   CLA_LID_MISC,
} CLA_LOG_IDS;


/*****************************************************************************
                          Macros for Logging
*****************************************************************************/
#if ( NCS_CLA_LOG == 1 )

#define m_CLA_LOG_SEAPI(op, st, sev)        cla_log_seapi(op, st, sev)
#define m_CLA_LOG_MDS(op, st, sev)          cla_log_mds(op, st, sev)
#define m_CLA_LOG_EDU(op, st, sev)          cla_log_edu(op, st, sev)
#define m_CLA_LOG_LOCK(op, st, sev)         cla_log_lock(op, st, sev)
#define m_CLA_LOG_CB(op, st, sev)           cla_log_cb(op, st, sev)
#define m_CLA_LOG_CBK(t, sev)               cla_log_cbk(t, sev)
#define m_CLA_LOG_SEL_OBJ(op, st, sev)      cla_log_sel_obj(op, st, sev)
#define m_CLA_LOG_API(t, s, sev)            cla_log_api(t, s, sev)
#define m_CLA_LOG_HDL_DB(op, st, hdl, sev)  cla_log_hdl_db(op, st, hdl, sev)
#define m_CLA_LOG_MISC(op, sev)             cla_log_misc(op, sev)

#else /* NCS_CLA_LOG == 1 */

#define m_CLA_LOG_SEAPI(op, st, sev)
#define m_CLA_LOG_MDS(op, st, sev)
#define m_CLA_LOG_EDU(op, st, sev)
#define m_CLA_LOG_LOCK(op, st, sev)
#define m_CLA_LOG_CB(op, st, sev)
#define m_CLA_LOG_CBK(t, sev)
#define m_CLA_LOG_SEL_OBJ(op, st, sev)
#define m_CLA_LOG_API(t, s, sev)
#define m_CLA_LOG_HDL_DB(op, st, hdl, sev)
#define m_CLA_LOG_MISC(op, sev)

#endif /* NCS_CLA_LOG == 1 */


/*****************************************************************************
                       Extern Function Declarations
*****************************************************************************/
#if ( NCS_CLA_LOG == 1 )

EXTERN_C uns32 cla_log_reg (void);
EXTERN_C uns32 cla_log_unreg (void);

EXTERN_C void cla_log_seapi (AVSV_LOG_SEAPI, AVSV_LOG_SEAPI, uns8);
EXTERN_C void cla_log_mds (AVSV_LOG_MDS, AVSV_LOG_MDS, uns8);
EXTERN_C void cla_log_edu (AVSV_LOG_EDU, AVSV_LOG_EDU, uns8);
EXTERN_C void cla_log_lock (AVSV_LOG_LOCK, AVSV_LOG_LOCK, uns8);
EXTERN_C void cla_log_cb (AVSV_LOG_CB, AVSV_LOG_CB, uns8);
EXTERN_C void cla_log_cbk (AVSV_CLM_CBK_TYPE, uns8);
EXTERN_C void cla_log_sel_obj (CLA_LOG_SEL_OBJ, CLA_LOG_SEL_OBJ, uns8);
EXTERN_C void cla_log_api (CLA_LOG_API, CLA_LOG_API, uns8);
EXTERN_C void cla_log_hdl_db (CLA_LOG_HDL_DB, CLA_LOG_HDL_DB, uns32, uns8);
EXTERN_C void cla_log_misc (CLA_LOG_MISC, uns8);

#endif /* NCS_CLA_LOG == 1 */

#endif /* !CLA_LOG_H */
