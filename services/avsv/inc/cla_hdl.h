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

  CLA handle database related definitions.
  
******************************************************************************
*/

#ifndef CLA_HDL_H
#define CLA_HDL_H


/*
 * Pending callback list definitions 
 */

/* Pending callback rec */
typedef struct cla_pend_cbk_rec
{
   AVSV_CLM_CBK_INFO *cbk_info;   /* callbk info */

   struct cla_pend_cbk_rec   *next;
} CLA_PEND_CBK_REC;

/* Pending callback list */
typedef struct cla_pend_cbk
{
   uns16    num;    /* no of pending callbacks */
   CLA_PEND_CBK_REC  *head;
   CLA_PEND_CBK_REC  *tail;
} CLA_PEND_CBK;


/* CLA handle database records */
typedef struct cla_hdl_rec_tag
{
   NCS_PATRICIA_NODE hdl_node; /* hdl-db tree node */

   uns32       hdl;     /* CLM handle (derived from hdl-mngr) */
   NCS_SEL_OBJ sel_obj; /* selection object */
   
   SaClmCallbacksT reg_cbk; /* callbacks registered by the application */

   CLA_PEND_CBK pend_cbk; /* list of pending AvSv callbacks */
} CLA_HDL_REC;


/* CLA handle database top level structure */
typedef struct cla_hdl_db_tag
{
   NCS_PATRICIA_TREE hdl_db_anchor; /* root of the handle db */
   uns32 num;                       /* no of hdl db records */
} CLA_HDL_DB;


/*** Macro Definitions */

/* Macro to find the hdl rec */
#define m_CLA_HDL_REC_FIND(db, hdl, o_rec) \
{ \
    uns32 hm_hdl = (uns32)(hdl); \
    (o_rec) = (CLA_HDL_REC *)ncs_patricia_tree_get(&(db)->hdl_db_anchor, \
                                                   (uns8 *)&hm_hdl); \
}

/* 
 * Macro to push the callback record to the 
 * tail of the pending callbk list
 */
#define m_CLA_HDL_PEND_CBK_PUSH(list, rec) \
{ \
   if (!((list)->head)) { \
       m_AVSV_ASSERT(!((list)->num)); \
       (list)->head = (rec); \
   } \
   else \
      (list)->tail->next = (rec); \
   (list)->tail = (rec); \
   ((list)->num)++; \
}

/* 
 * Macro to pop the callback record from the 
 * head of the pending callbk list
 */
#define m_CLA_HDL_PEND_CBK_POP(list, o_rec) \
{ \
   if ((list)->head) { \
      (o_rec) = (list)->head; \
      (list)->head = (o_rec)->next; \
      (o_rec)->next = 0; \
      if ((list)->tail == (o_rec)) (list)->tail = 0; \
      (list)->num--; \
   } else (o_rec) = 0; \
}

/* Macro to push the pg record to the head of the pg list */
#define m_CLA_HDL_PG_PUSH(list, rec) \
{ \
   (rec)->next = (list)->head; \
   (list)->head = (rec); \
   (list)->num++; \
}

/* Macro to pop the pg record from the head of the pg list */
#define m_CLA_HDL_PG_POP(list, o_rec) \
{ \
   if ((list)->head) { \
      (o_rec) = (list)->head; \
      (list)->head = (o_rec)->next; \
      (o_rec)->next = 0; \
      (list)->num--; \
   } else (o_rec) = 0; \
}

/* Macro to determine if application has requested a CLM track */
#define m_CLA_HDL_IS_CLM_TRACK(hdl_rec)  ((hdl_rec)->track.num)


/*** Extern function declarations ***/

struct cla_cb_tag;

EXTERN_C uns32 cla_hdl_cbk_param_add (struct cla_cb_tag *, CLA_HDL_REC *, 
                                      AVSV_CLM_CBK_INFO *);

EXTERN_C uns32 cla_hdl_init (CLA_HDL_DB *);

EXTERN_C void cla_hdl_cbk_list_del(struct cla_cb_tag *, CLA_HDL_REC *);

EXTERN_C void cla_hdl_del (struct cla_cb_tag *);

EXTERN_C void cla_hdl_rec_del (struct cla_cb_tag *, CLA_HDL_DB *, 
                               CLA_HDL_REC *);

EXTERN_C CLA_HDL_REC *cla_hdl_rec_add (struct cla_cb_tag *, CLA_HDL_DB *, 
                              const SaClmCallbacksT *);

EXTERN_C uns32 cla_hdl_cbk_dispatch (struct cla_cb_tag **, CLA_HDL_REC **, 
                                     SaDispatchFlagsT);
#endif /* !CLA_HDL_H */
