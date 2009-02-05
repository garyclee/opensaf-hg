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

  MODULE NAME: SRMA_CBK.H

..............................................................................

  DESCRIPTION: This file has the data structure definitions defined for SRMA 
               database. It also contains the related function prototype 
               definitions.

******************************************************************************/

#ifndef SRMA_CBK_H
#define SRMA_CBK_H


/* 
 * Macro to add CBK record to appl callback data list
 */
#define m_SRMA_ADD_APPL_CBK_INFO(appl, cbk_rec)  \
{  \
   if ((appl->pend_cbk.head == NULL) &&     \
       (appl->pend_cbk.tail == NULL))       \
   {                                        \
      appl->pend_cbk.head = cbk_rec;        \
      appl->pend_cbk.tail = cbk_rec;        \
   }                                        \
   else                                     \
   {                                        \
      appl->pend_cbk.tail->next = cbk_rec;  \
      appl->pend_cbk.tail = cbk_rec;        \
   }                                        \
   appl->pend_cbk.rec_num++;                \
}

/* 
 * Macro to del CBK record from appl callback data list
 */
#define m_SRMA_DEL_APPL_CBK_INFO(appl, cbk_rec) \
{ \
   appl->pend_cbk.head = cbk_rec->next;    \
   if (appl->pend_cbk.head == NULL)        \
   {                                       \
      appl->pend_cbk.tail = NULL;          \
      appl->pend_cbk.rec_num = 0;          \
   }                                       \
   else                                    \
      appl->pend_cbk.rec_num--;            \
   m_MMGR_FREE_SRMA_PEND_CBK_REC(cbk_rec); \
}       

/* 
 * Record to store pending callback data for meant for application 
 */
typedef struct srma_pend_cbk_rec
{   
   NCS_BOOL for_srma;
   union
   {
      MDS_DEST  srmnd_dest;
      NCS_SRMSV_RSRC_CBK_INFO cbk_info;
   } info;

   struct srma_pend_cbk_rec *next;
} SRMA_PEND_CBK_REC;

/* Pending callback list */
typedef struct srma_pend_callback
{
   uns16  rec_num;
   SRMA_PEND_CBK_REC *head;
   SRMA_PEND_CBK_REC *tail;
} SRMA_PEND_CALLBACK;

#endif /* SRMA_CBK_H */



