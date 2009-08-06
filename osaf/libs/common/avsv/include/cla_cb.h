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

  CLA CB related definitions.
  
******************************************************************************
*/

#ifndef CLA_CB_H
#define CLA_CB_H

/* CLA control block */
typedef struct cla_cb_tag {
	uns32 cb_hdl;		/* CB hdl returned by hdl mngr */
	EDU_HDL edu_hdl;	/* EDU handle */
	uns8 pool_id;		/* pool-id used by hdl mngr */
	uns32 prc_id;		/* process identifier */
	uns32 pend_fin;		/* count of pending agent shutdown */
	uns32 pend_dis;		/* count of pending dispatch */
	CLA_HDL_DB hdl_db;	/* CLA handle database */
	CLA_AVND_INTF avnd_intf;	/* AvND interface */
	NCS_LOCK lock;		/* CB lock */
	NCS_SEL_OBJ sel_obj;	/* sel obj for mds sync indication */
	uns32 flag;		/* flags */
} CLA_CB;

/* CLA Flags */
#define CLA_FLAG_FD_VALID   0x00000001
#define m_CLA_FLAG_IS_FD_VALID(cb)    (cb->flag & CLA_FLAG_FD_VALID)
#define m_CLA_FLAG_SET(cb, bitmap)    (cb->flag |= bitmap)
#define m_CLA_FLAG_RESET(cb, bitmap)  (cb->flag &= ~bitmap)

/*** Extern function declarations ***/

EXTERN_C uns32 cla_create(NCS_LIB_CREATE *);

EXTERN_C void cla_destroy(NCS_LIB_DESTROY *);

EXTERN_C uns32 cla_avnd_msg_prc(CLA_CB *, AVSV_NDA_CLA_MSG *);

#endif   /* !CLA_CB_H */
