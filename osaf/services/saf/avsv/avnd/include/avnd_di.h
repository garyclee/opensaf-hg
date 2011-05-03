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

  This file contains declarations fro AvD interface.
  
******************************************************************************
*/

#ifndef AVND_DI_H
#define AVND_DI_H

/* macro to find the matching record (based on the msg-id) */
/* 
 * Caution!!! It is assumed that the msg-id is the 1st element in the message
 * structure. Ensure it. Else move the msg id to the common portion of the 
 * message structure (outside the msg type specific contents).
 */
#define m_AVND_DIQ_REC_FIND(cb, mid, o_rec) \
{ \
   AVND_DND_LIST *list = &((cb)->dnd_list); \
   for ((o_rec) = list->head; \
        (o_rec) && !(*((uns32 *)(&((o_rec)->msg.info.avd->msg_info)))== (mid)); \
        (o_rec) = (o_rec)->next); \
}

/* macro to find & pop a given record */
#define m_AVND_DIQ_REC_FIND_POP(cb, rec) \
{ \
   AVND_DND_LIST *list = &((cb)->dnd_list); \
   AVND_DND_MSG_LIST *prv = list->head, *curr; \
   for (curr = list->head; \
        curr && !(curr == (rec)); \
        prv = curr, curr = curr->next); \
   if (curr) \
   { \
      if ( curr == list->head ) \
      { \
         list->head = curr->next; \
         if ( list->tail == curr ) list->tail = list->head; \
      } else { \
         prv->next = curr->next; \
         if ( list->tail == curr ) list->tail = prv; \
      } \
      curr->next = 0; \
   } \
}

struct avnd_cb_tag;

EXTERN_C uns32 avnd_di_oper_send(struct avnd_cb_tag *, AVND_SU *, uns32);
EXTERN_C uns32 avnd_di_susi_resp_send(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SI_REC *);
EXTERN_C uns32 avnd_di_object_upd_send(struct avnd_cb_tag *, AVSV_PARAM_INFO *);
EXTERN_C uns32 avnd_di_pg_act_send(struct avnd_cb_tag *, SaNameT *, AVSV_PG_TRACK_ACT, NCS_BOOL);
EXTERN_C uns32 avnd_di_msg_send(struct avnd_cb_tag *, AVND_MSG *);
EXTERN_C void avnd_di_msg_ack_process(struct avnd_cb_tag *, uns32);
EXTERN_C void avnd_diq_del(struct avnd_cb_tag *);
EXTERN_C void avnd_snd_shutdown_app_su_msg(struct avnd_cb_tag *);
EXTERN_C AVND_DND_MSG_LIST *avnd_diq_rec_add(struct avnd_cb_tag *cb, AVND_MSG *msg);
EXTERN_C void avnd_diq_rec_del(struct avnd_cb_tag *cb, AVND_DND_MSG_LIST *rec);
EXTERN_C uns32 avnd_diq_rec_send(struct avnd_cb_tag *cb, AVND_DND_MSG_LIST *rec);
EXTERN_C uns32 avnd_di_reg_su_rsp_snd(struct avnd_cb_tag *cb, SaNameT *su_name, uns32 ret_code);
EXTERN_C uns32 avnd_di_reg_comp_rsp_snd(struct avnd_cb_tag *cb, SaNameT *comp_name, uns32 ret_code);
EXTERN_C uns32 avnd_di_ack_nack_msg_send(struct avnd_cb_tag *cb, uns32 rcv_id, uns32 view_num);
extern void avnd_di_uns32_upd_send(int class_id, int attr_id, const SaNameT *dn, uns32 value);

#endif   /* !AVND_OPER_H */
