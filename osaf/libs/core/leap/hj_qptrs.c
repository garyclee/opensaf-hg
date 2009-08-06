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

  DESCRIPTION: What ncs_qptrs.h says.

******************************************************************************
*/

/** Get compile time options...
 **/
#include "ncs_opt.h"

/** Get general definitions...
 **/
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "sysf_def.h"
#include "ncssysfpool.h"
/** Get NCS Pointer Queue Facility header file.
 **/
#include "ncs_qptrs.h"

#define NCS_QLINK_NULL          (NCS_QLINK*)0

/*****************************************************************************

  PROCEDURE NAME:    ncs_qspace_construct

  DESCRIPTION:

        get the NCS_QSPACE struct in start state w/no memory commitments.

*****************************************************************************/

void ncs_qspace_construct(NCS_QSPACE *qs)
{
	qs->front = 0;
	qs->back = 0;
	qs->slots = 0;
	qs->f_idx = 0;
	qs->b_idx = 0;
	qs->count = 0;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_circ_qspace_construct

  DESCRIPTION:

        get the NCS_QSPACE struct in start state w/no memory commitments.

*****************************************************************************/

void ncs_circ_qspace_construct(NCS_QSPACE *qs, int32 max_size)
{
	qs->max_size = max_size;
	ncs_qspace_construct(qs);
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_qspace_init

  DESCRIPTION:

        Prepare to actually enqueue things. Commit the memory and put local
        NCS_QSPACE fields in ready state.

*****************************************************************************/

void ncs_qspace_init(NCS_QSPACE *qs)
{
	ncs_qspace_delete(qs);	/* free stale information first */

	qs->front = (NCS_QLINK *)m_MMGR_ALLOC_NCS_QLINK;
	m_QSPACE_STUFF_OWNER(qs, qs->front);

	qs->front->next = NCS_QLINK_NULL;
	qs->back = qs->front;
	qs->slots = (sizeof(NCS_QLINK) - sizeof(NCS_QLINK *)) / sizeof(void *);
	qs->f_idx = 0;
	qs->b_idx = 0;
	qs->count = 0;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_qspace_enq

  DESCRIPTION:

        put the thing-to-enqueue at the end of the queue represented by the
        passed NCS_QSPACE.

*****************************************************************************/

void ncs_qspace_enq(NCS_QSPACE *qs, void *q_it)
{
	if (qs->b_idx == qs->slots) {
		qs->back->next = (NCS_QLINK *)m_MMGR_ALLOC_NCS_QLINK;
		m_QSPACE_STUFF_OWNER(qs, qs->back->next);
		qs->back = qs->back->next;
		qs->back->next = NCS_QLINK_NULL;
		qs->b_idx = 0;
	}
	qs->back->slot[qs->b_idx++] = (uns32 *)q_it;
	qs->count++;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_qspace_deq

  DESCRIPTION:

        Fetch the thing on the front of the queue represented by the passed
        NCS_QSPACE.

        Return (VOID*) 0 if the queue is empty.

*****************************************************************************/

void *ncs_qspace_deq(NCS_QSPACE *qs)
{
	void *retval;
	NCS_QLINK *temp;

	if ((qs->front == qs->back) && (qs->f_idx == qs->b_idx))
		return (void *)0;	/* the queue is empty */

	if (qs->f_idx == qs->slots) {
		temp = qs->front->next;
		m_MMGR_FREE_NCS_QLINK(qs->front);
		qs->front = temp;
		qs->f_idx = 0;
	}

	qs->count--;
	if ((retval = (void *)qs->front->slot[qs->f_idx++]) == (void *)NCS_QSPACE_DEAD)
		return ncs_qspace_deq(qs);
	else
		return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_qspace_hunt

  DESCRIPTION:

        this function looks to see if an exiting element is already in the 
        queueu. It returns TRUE if it finds it.

*****************************************************************************/

NCS_BOOL ncs_qspace_hunt(NCS_QSPACE *qs, void *tgt)
{
	NCS_QLINK *qlink = qs->front;
	int32 st_loop = qs->f_idx;
	int32 bk_loop = qs->slots;
	int32 i;

	while (qlink != (NCS_QLINK *)0) {
		if (qlink == qs->back)	/* figure out how big the loop can be */
			bk_loop = qs->b_idx;

		for (i = st_loop; i < bk_loop; i++) {	/* now go hunting */
			if (qlink->slot[i] == tgt)
				return TRUE;	/* found a match; return TRUE */
		}
		qlink = qlink->next;
		st_loop = 0;	/* OK, past first one, now start with zero-eth element */
	}
	return FALSE;		/* no match */
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_qspace_remove

  DESCRIPTION:

        this function looks to see if an exiting element is already in the 
        queueu. It removes it if found, and returns TRUE.

  ARGUMENTS:
        qs       NCS_QSPACE that holds queue information
        tgt      the value to hunt down and remove
        find_all if TRUE, look through entire queueu and remove all.
                 if FALSE, just find first instance and remove just it

  NOTE: 
        this is accomplished by putting an NCS_QSPACE_DEAD marker in the
        slot that has a match. This is a known constraint for using this
        as per disclaimer at beginning of this function set.

*****************************************************************************/

NCS_BOOL ncs_qspace_remove(NCS_QSPACE *qs, void *tgt, NCS_BOOL find_all)
{
	NCS_QLINK *qlink = qs->front;
	NCS_BOOL code = FALSE;	/* default to not found */
	int32 st_loop = qs->f_idx;
	int32 bk_loop = qs->slots;
	int32 i;

	while (qlink != (NCS_QLINK *)0) {
		if (qlink == qs->back)	/* figure out how big the loop can be */
			bk_loop = qs->b_idx;

		for (i = st_loop; i < bk_loop; i++) {	/* now go hunting */
			if (qlink->slot[i] == (uns32 *)tgt) {
				qlink->slot[i] = (uns32 *)NCS_QSPACE_DEAD;
				if (find_all == FALSE)
					return TRUE;	/* found a match; return TRUE */
				else
					code = TRUE;	/* found a match, but keep looking */
			}
		}
		qlink = qlink->next;
		st_loop = 0;	/* OK, past first one, now start with zero-eth element */
	}
	return code;		/* report what we found */
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_qspace_delete

  DESCRIPTION:

        Clean up any dangling resources associated with the queue represented
        by the passed NCS_QSPACE..

*****************************************************************************/

void ncs_qspace_delete(NCS_QSPACE *qs)
{
	NCS_QLINK *temp;
	NCS_QLINK *qlink = qs->front;

	while (qlink != 0) {
		temp = qlink->next;
		m_MMGR_FREE_NCS_QLINK(qlink);
		qlink = temp;
	}

	ncs_qspace_construct(qs);	/* put it back to constructor start-state */
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_circ_qspace_set_maxsize

  DESCRIPTION:

        Change the maximum size of the circular queue.  It fails if trying
        reduce the size if there are items on the queue.

*****************************************************************************/

uns32 ncs_circ_qspace_set_maxsize(NCS_QSPACE *qs, int32 new_max_size)
{
	uns32 retval = NCSCC_RC_FAILURE;

	if (qs->count < new_max_size) {
		qs->max_size = new_max_size;
		retval = NCSCC_RC_SUCCESS;
	}

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_qspace_peek

  DESCRIPTION:

*****************************************************************************/

void *ncs_qspace_peek(NCS_QSPACE *qs, int32 index)
{
	NCS_QLINK *qlink = qs->front;
	int32 st_loop = qs->f_idx;
	int32 bk_loop = qs->slots;
	int32 i;
	int32 local_index = 0;
	void *retval = NULL;

	while (qlink != (NCS_QLINK *)0) {
		if (qlink == qs->back) {	/* figure out how big the loop can be */
			bk_loop = qs->b_idx;
		}

		for (i = st_loop; i < bk_loop; i++) {	/* now go hunting */

			if (local_index == index) {
				retval = qlink->slot[i];	/* found a match */
			}
			local_index++;
		}
		qlink = qlink->next;
		st_loop = 0;	/* OK, past first one, now start with zero-eth element */
	}

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_qspace_pop

  DESCRIPTION:
  remove the most recently enqued item

*****************************************************************************/

void *ncs_qspace_pop(NCS_QSPACE *qs)
{
	void *retval;

	qs->b_idx--;
	qs->count--;
	retval = qs->back->slot[qs->b_idx];

	if (0 == qs->b_idx) {
		/* find prev ncs_qlink */
		NCS_QLINK *thisql = qs->front;
		NCS_QLINK *prevql = NULL;
		NCS_QLINK *delql = NULL;
		do {
			if (thisql == qs->back) {
				delql = thisql;
				if (NULL == prevql) {
					qs->front = delql->next;
				} else {
					prevql->next = delql->next;
				}
				thisql = thisql->next;
				m_MMGR_FREE_NCS_QLINK(delql);
			} else {
				prevql = thisql;
				thisql = thisql->next;
			}
		} while (NULL != thisql);
	}

	return retval;
}
