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

  This file contains logging offset indices that are used for logging AvA 
  information.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#ifndef AVA_LOG_H
#define AVA_LOG_H

#include "avsv_log.h"

/******************************************************************************
                    Logging offset indices for selection object
 ******************************************************************************/
typedef enum ava_log_sel_obj {
	AVA_LOG_SEL_OBJ_CREATE,
	AVA_LOG_SEL_OBJ_DESTROY,
	AVA_LOG_SEL_OBJ_IND_SEND,
	AVA_LOG_SEL_OBJ_IND_REM,
	AVA_LOG_SEL_OBJ_SUCCESS,
	AVA_LOG_SEL_OBJ_FAILURE,
	AVA_LOG_SEL_OBJ_MAX
} AVA_LOG_SEL_OBJ;

/******************************************************************************
                       Logging offset indices for AMF APIs
 ******************************************************************************/
/* ensure that the this ordering matches that of API type definition */
typedef enum ava_log_api {
	AVA_LOG_API_FINALIZE,
	AVA_LOG_API_COMP_REG,
	AVA_LOG_API_COMP_UNREG,
	AVA_LOG_API_PM_START,
	AVA_LOG_API_PM_STOP,
	AVA_LOG_API_HC_START,
	AVA_LOG_API_HC_STOP,
	AVA_LOG_API_HC_CONFIRM,
	AVA_LOG_API_CSI_QUIESCING_COMPLETE,
	AVA_LOG_API_HA_STATE_GET,
	AVA_LOG_API_PG_START,
	AVA_LOG_API_PG_STOP,
	AVA_LOG_API_ERR_REP,
	AVA_LOG_API_ERR_CLEAR,
	AVA_LOG_API_RESP,
	AVA_LOG_API_INITIALIZE,
	AVA_LOG_API_SEL_OBJ_GET,
	AVA_LOG_API_DISPATCH,
	AVA_LOG_API_COMP_NAME_GET,
	AVA_LOG_API_ERR_OFFSET = AVA_LOG_API_COMP_NAME_GET,
	AVA_LOG_API_ERR_SA_OK,
	AVA_LOG_API_ERR_SA_LIBRARY,
	AVA_LOG_API_ERR_SA_VERSION,
	AVA_LOG_API_ERR_SA_INIT,
	AVA_LOG_API_ERR_SA_TIMEOUT,
	AVA_LOG_API_ERR_SA_TRY_AGAIN,
	AVA_LOG_API_ERR_SA_INVALID_PARAM,
	AVA_LOG_API_ERR_SA_NO_MEMORY,
	AVA_LOG_API_ERR_SA_BAD_HANDLE,
	AVA_LOG_API_ERR_SA_BUSY,
	AVA_LOG_API_ERR_SA_ACCESS,
	AVA_LOG_API_ERR_SA_NOT_EXIST,
	AVA_LOG_API_ERR_SA_NAME_TOO_LONG,
	AVA_LOG_API_ERR_SA_EXIST,
	AVA_LOG_API_ERR_SA_NO_SPACE,
	AVA_LOG_API_ERR_SA_INTERRUPT,
	AVA_LOG_API_ERR_SA_NAME_NOT_FOUND,
	AVA_LOG_API_ERR_SA_NO_RESOURCES,
	AVA_LOG_API_ERR_SA_NOT_SUPPORTED,
	AVA_LOG_API_ERR_SA_BAD_OPERATION,
	AVA_LOG_API_ERR_SA_FAILED_OPERATION,
	AVA_LOG_API_ERR_SA_MESSAGE_ERROR,
	AVA_LOG_API_ERR_SA_QUEUE_FULL,
	AVA_LOG_API_ERR_SA_QUEUE_NOT_AVAILABLE,
	AVA_LOG_API_ERR_SA_BAD_CHECKPOINT,
	AVA_LOG_API_ERR_SA_BAD_FLAGS,
	AVA_LOG_API_ERR_SA_TOO_BIG,
	AVA_LOG_API_ERR_SA_NO_SECTIONS,
	AVA_LOG_API_MAX
} AVA_LOG_API;

/******************************************************************************
                Logging offset indices for AvA handle database
 ******************************************************************************/
typedef enum ava_log_hdl_db {
	AVA_LOG_HDL_DB_CREATE,
	AVA_LOG_HDL_DB_DESTROY,
	AVA_LOG_HDL_DB_REC_ADD,
	AVA_LOG_HDL_DB_REC_CBK_ADD,
	AVA_LOG_HDL_DB_REC_DEL,
	AVA_LOG_HDL_DB_SUCCESS,
	AVA_LOG_HDL_DB_FAILURE,
	AVA_LOG_HDL_DB_MAX
} AVA_LOG_HDL_DB;

/******************************************************************************
              Logging offset indices for miscellaneous AvA events
 ******************************************************************************/
typedef enum ava_log_misc {
	AVA_LOG_MISC_AVND_UP,
	AVA_LOG_MISC_AVND_DN,

	/* keep adding misc logs here */

	AVA_LOG_MISC_MAX
} AVA_LOG_MISC;

/******************************************************************************
 Logging offset indices for canned constant strings for the ASCII SPEC
 ******************************************************************************/
typedef enum ava_flex_sets {
	AVA_FC_SEAPI,
	AVA_FC_MDS,
	AVA_FC_EDU,
	AVA_FC_LOCK,
	AVA_FC_CB,
	AVA_FC_CBK,
	AVA_FC_SEL_OBJ,
	AVA_FC_API,
	AVA_FC_HDL_DB,
	AVA_FC_MISC,
} AVA_FLEX_SETS;

typedef enum ava_log_ids {
	AVA_LID_SEAPI,
	AVA_LID_MDS,
	AVA_LID_EDU,
	AVA_LID_LOCK,
	AVA_LID_CB,
	AVA_LID_CBK,
	AVA_LID_SEL_OBJ,
	AVA_LID_API,
	AVA_LID_HDL_DB,
	AVA_LID_MISC,
} AVA_LOG_IDS;

/*****************************************************************************
                          Macros for Logging
*****************************************************************************/
#if ( NCS_AVA_LOG == 1 )

#define m_AVA_LOG_SEAPI(op, st, sev)        ava_log_seapi(op, st, sev)
#define m_AVA_LOG_MDS(op, st, sev)          ava_log_mds(op, st, sev)
#define m_AVA_LOG_EDU(op, st, sev)          ava_log_edu(op, st, sev)
#define m_AVA_LOG_LOCK(op, st, sev)         ava_log_lock(op, st, sev)
#define m_AVA_LOG_CB(op, st, sev)           ava_log_cb(op, st, sev)
#define m_AVA_LOG_CBK(t, n, sev)            ava_log_cbk(t, n, sev)
#define m_AVA_LOG_SEL_OBJ(op, st, sev)      ava_log_sel_obj(op, st, sev)
#define m_AVA_LOG_API(t, s, n, sev)         ava_log_api(t, s, n, sev)
#define m_AVA_LOG_HDL_DB(op, st, hdl, sev)  ava_log_hdl_db(op, st, hdl, sev)
#define m_AVA_LOG_MISC(op, sev)             ava_log_misc(op, sev)
#else				/* NCS_AVA_LOG == 1 */

#define m_AVA_LOG_SEAPI(op, st, sev)
#define m_AVA_LOG_MDS(op, st, sev)
#define m_AVA_LOG_EDU(op, st, sev)
#define m_AVA_LOG_LOCK(op, st, sev)
#define m_AVA_LOG_CB(op, st, sev)
#define m_AVA_LOG_CBK(t, n, sev)
#define m_AVA_LOG_SEL_OBJ(op, st, sev)
#define m_AVA_LOG_API(t, s, n, sev)
#define m_AVA_LOG_HDL_DB(op, st, hdl, sev)
#define m_AVA_LOG_MISC(op, sev)
#endif   /* NCS_AVA_LOG == 1 */

/*****************************************************************************
                       Extern Function Declarations
*****************************************************************************/
#if ( NCS_AVA_LOG == 1 )

EXTERN_C uns32 ava_log_reg(void);
EXTERN_C uns32 ava_log_unreg(void);

EXTERN_C void ava_log_seapi(AVSV_LOG_SEAPI, AVSV_LOG_SEAPI, uns8);
EXTERN_C void ava_log_mds(AVSV_LOG_MDS, AVSV_LOG_MDS, uns8);
EXTERN_C void ava_log_edu(AVSV_LOG_EDU, AVSV_LOG_EDU, uns8);
EXTERN_C void ava_log_lock(AVSV_LOG_LOCK, AVSV_LOG_LOCK, uns8);
EXTERN_C void ava_log_cb(AVSV_LOG_CB, AVSV_LOG_CB, uns8);
EXTERN_C void ava_log_cbk(AVSV_LOG_AMF_CBK, SaNameT *, uns8);
EXTERN_C void ava_log_sel_obj(AVA_LOG_SEL_OBJ, AVA_LOG_SEL_OBJ, uns8);
EXTERN_C void ava_log_api(AVA_LOG_API, AVA_LOG_API, const SaNameT *, uns8);
EXTERN_C void ava_log_hdl_db(AVA_LOG_HDL_DB, AVA_LOG_HDL_DB, uns32, uns8);
EXTERN_C void ava_log_misc(AVA_LOG_MISC, uns8);
#endif   /* NCS_AVA_LOG == 1 */

#endif   /* !AVA_LOG_H */
