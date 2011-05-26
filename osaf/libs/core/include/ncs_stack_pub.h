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
 

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef NCS_STACK_PUB_H
#define NCS_STACK_PUB_H

#include "ncsgl_defs.h"
#include "ncs_ubaid.h"

#ifdef  __cplusplus
extern "C" {
#endif

/***************************************************************************
 ***************************************************************************
 *
 *   N e t P l a n e   H J M E M _ A I D
 *
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 * ncsmem_aid : this little guy manages an arbitrary hunk of memory. It was
 *             conceived in the context of managing stack space that is to 
 *             be used for multiple ends. 
 *
 *  THE ISSUE SOLVED: The invoker of a service that takes an NCSMEM_AID 
 *                    pointer for an argument can control where that memory
 *                    comes from:
 * 
 *                Stack : if the passed pointer is not NULL, get memory from
 *                        NCSMEM_AID via ncsmem_aid_alloc().
 *                Heap  : if the passed pointer IS NULL, get memory from heap.
 *
 ***************************************************************************/
/***************************************************************************
 *
 * NOTE: All allocs from NCSMEM_AID are guarenteed to start on 4 byte 
 *       boundaries, aligned with the originally passed 'space' pointer.
 ***************************************************************************/ typedef struct ncsmem_aid {

		/* P R I V A T E fields should not be referenced by client            */

		uns32 max_len;	/* start len of passed buffer (HEAP or STACK)       */
		uint8_t *cur_ptr;	/* current place for getting memory                 */
		uint8_t *bgn_ptr;	/* original buffer ptr; if from HEAP, use to free   */

		/* P U B L I C   inspectable by client; set by NCSMEM_AID member funcs */

		uns32 status;	/* If any alloc fails, mark it                      */

	} NCSMEM_AID;

/***************************************************************************
 * NCSMEM_AID  public member function prototypes
 ***************************************************************************/

	void ncsmem_aid_init(NCSMEM_AID *ma, uint8_t *space, uns32 len);

	uint8_t *ncsmem_aid_alloc(NCSMEM_AID *ma, uns32 size);

	uint8_t *ncsmem_aid_cpy(NCSMEM_AID *ma, const uint8_t *ref, uns32 len);

/***************************************************************************
 ***************************************************************************
 *
 *   N e t P l a n e   H J _ S t a c k
 *
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 *
 * P u b l i c   H J _ S t a c k    O b j e c t s
 *
 ***************************************************************************/

/***************************************************************************
 *  NCS_STACK : keeps track of current stack state
 ***************************************************************************/

	typedef struct ncs_stack {
		uint16_t se_cnt;	/* Number of elements in stack     */
		uint16_t max_depth;	/* Maximum Depth                   */
		uint16_t cur_depth;	/* Current Depth                   */
		uint16_t pad;	/* To 32 bit boundary...           */

	} NCS_STACK;

/***************************************************************************
 *  NCS_SE    : All Stack Elements are preceeded by one of these..
 ***************************************************************************/

	typedef struct ncs_se {
		uint16_t type;	/* Stack element type              */
		uint16_t length;	/* lenght of stack element         */

	} NCS_SE;

/***************************************************************************
 *
 *     P u b l i c   H J _ S T A C K   F u n c t i o n s  &  M a c r o s
 *
 ***************************************************************************/

#define m_NCSSTACK_SPACE(se)        ((uint8_t*)((uint8_t*)se + sizeof(NCS_SE)))

	void ncsstack_init(NCS_STACK *st, uint16_t max_size);

	NCS_SE *ncsstack_peek(NCS_STACK *st);

	NCS_SE *ncsstack_push(NCS_STACK *st, uint16_t type, uint16_t size);

	NCS_SE *ncsstack_pop(NCS_STACK *st);
	uns32 ncsstack_get_utilization(NCS_STACK *st);
	uns32 ncsstack_get_element_count(NCS_STACK *st);

	uns32 ncsstack_encode(NCS_STACK *st, struct ncs_ubaid *uba);

	uns32 ncsstack_decode(NCS_STACK *st, struct ncs_ubaid *uba);

#ifdef  __cplusplus
}
#endif

#endif   /* NCS_STACK_PUB_H */
