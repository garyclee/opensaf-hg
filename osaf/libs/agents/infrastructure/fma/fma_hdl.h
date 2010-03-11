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

#ifndef FMA_HDL_H
#define FMA_HDL_H

typedef enum fma_cbk_type {
	NODE_RESET_IND,
	SWITCHOVER_REQ
} FMA_CBK_TYPE;

typedef struct fma_cbk_info {
	FMA_CBK_TYPE cbk_type;
	SaHpiEntityPathT node_reset_info;
	uns32 inv;
} FMA_CBK_INFO;

/* Pending callback rec */
typedef struct fma_pend_cbk_rec {
	FMA_CBK_INFO *cbk_info;	/* callbk info */
	uns32 flag;		/* unused, dont remove it */

	struct fma_pend_cbk_rec *next;
} FMA_PEND_CBK_REC;

/* Pending callback list */
typedef struct fma_pend_cbk {
	uns16 num;		/* no of pending callbacks */
	FMA_PEND_CBK_REC *head;
	FMA_PEND_CBK_REC *tail;
} FMA_PEND_CBK;

/* Pending response rec */
typedef struct fma_pend_resp_rec {
	FMA_CBK_INFO *cbk_info;	/* callbk info */
	uns32 flag;		/* used to see, if we have responded
				   in callback itself */

	struct fma_pend_resp_rec *next;
} FMA_PEND_RESP_REC;

/* Pending response list */
typedef struct fma_resp_cbk {
	uns16 num;		/* num of pending callbacks */
	FMA_PEND_RESP_REC *head;
	FMA_PEND_RESP_REC *tail;
} FMA_PEND_RESP;

/* FMA handle database records */
typedef struct fma_hdl_rec {
	NCS_PATRICIA_NODE hdl_node;	/* hdl-db tree node */
	uns32 hdl;		/* from hdl-mngr */
	NCS_SEL_OBJ sel_obj;	/* selection object */
	fmCallbacksT reg_cbk;	/* callbacks registered by the application */
	FMA_PEND_CBK pend_cbk;	/* list of pending callbacks */
	FMA_PEND_RESP pend_resp;	/* list of pending Response */
} FMA_HDL_REC;

/* FM handle database top level structure */
typedef struct fm_hdl_db_tag {
	NCS_PATRICIA_TREE hdl_db_anchor;	/* root of the handle db */
	uns32 num;		/* no of hdl db records */
} FMA_HDL_DB;

#define m_FMA_HDL_PEND_CBK_ADD(list, rec) \
{ \
   if (!((list)->head)) { \
       m_FMA_ASSERT(!((list)->num)); \
       (list)->head = (rec); \
   } \
   else \
   {\
      (list)->tail->next = (rec); \
   }\
   (list)->tail = (rec); \
   ((list)->num)++; \
}

/*
 * Macro to pop the callback record from the
 * head of the pending callbk list
 */
#define m_FMA_HDL_PEND_CBK_GET(list, o_rec) \
{ \
   if ((list)->head) { \
      (o_rec) = (list)->head; \
      (list)->head = (o_rec)->next; \
      (o_rec)->next = 0; \
   if ((list)->tail == (o_rec)) (list)->tail = 0; \
      (list)->num--; \
   } else (o_rec) = 0; \
}

#define m_FMA_HDL_PEND_RESP_ADD  m_FMA_HDL_PEND_CBK_ADD
#define m_FMA_HDL_PEND_RESP_GET  m_FMA_HDL_PEND_CBK_GET

struct fma_cb;

EXTERN_C FMA_HDL_REC *fma_hdl_rec_add(struct fma_cb *cb, FMA_HDL_DB *hdl_cb, const fmCallbacksT *fmaCallback);
EXTERN_C void fma_hdl_rec_del(struct fma_cb *cb, FMA_HDL_DB *hdl_db, FMA_HDL_REC *hdl_rec);
EXTERN_C uns32 fma_hdl_db_init(FMA_HDL_DB *hdl_db);
EXTERN_C void fma_hdl_db_del(struct fma_cb *cb);
EXTERN_C uns32 fma_hdl_callbk_dispatch(struct fma_cb **cb, FMA_HDL_REC **hdl_rec, SaDispatchFlagsT flags);
EXTERN_C FMA_PEND_RESP_REC *fma_hdl_pend_resp_get(FMA_PEND_RESP *list, SaInvocationT key);
EXTERN_C FMA_PEND_RESP_REC *fma_hdl_pend_resp_pop(FMA_PEND_RESP *list, SaInvocationT key);

#endif
