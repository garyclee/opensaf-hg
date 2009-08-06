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
#ifndef NCS_STACK_H
#define NCS_STACK_H

#include "ncs_stack_pub.h"
#include "ncs_svd.h"

/***************************************************************************
 ***************************************************************************
 *
 *   N e t P l a n e   H J _ S t a c k
 *
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 * Private (internal) Stack abstractions
 ***************************************************************************/

#define SE_ALIGNMENT_MARKER 0xFFFF

/***************************************************************************
 * Compiler hint that there is such a structure in the world..
 ***************************************************************************/

struct ncsmib_arg;		/* this allows function prototypes below to fly */

/***************************************************************************
 * Compiler hint that there are such function prototypes in the world...
 ***************************************************************************/

typedef uns32 (*NCSMIB_FNC) (struct ncsmib_arg * req);

/***************************************************************************

  P u b l i c   S t a c k  E l e m e n t    S t r u c t u r e s 

 ***************************************************************************/

/***************************************************************************
 * Filter ID : Used by MAB to identify an Access Filter instance
 ***************************************************************************/

typedef struct ncs_se_filter_id {
	NCS_SE se;		/* All NCS_STACK Stack Elements start with this */

	uns32 fltr_id;		/* Filter ID */

} NCS_SE_FILTER_ID;

/***************************************************************************
 * Back To : Used by MAB to explain how to get back to the correct invoker
 ***************************************************************************/

typedef struct ncs_se_backto {
	NCS_SE se;		/* All NCS_STACK Stack Elements start with this */
	uns16 svcid;		/* Back to Subcomponent's Service ID */
	uns16 vrid;		/* Back to Virtual Router ID */
	MDS_DEST vcard;		/* Back to Virtual Card ID */

} NCS_SE_BACKTO;

/***************************************************************************
 * MIB Sync : Used by NetPlane MIB services to preserve MIB request sync info
 ***************************************************************************/

typedef struct ncs_se_mib_sync {
	NCS_SE se;		/* All NCS_STACK Stack Elements start with this */

	struct ncsmib_arg *stack_arg;	/* synchronous version uses NCSMIB_ARG* on stack */
	void *stack_sem;	/* We create/store a LEAP semaphor on the stack */
	NCSMIB_FNC usr_rsp_fnc;	/* store the response function for later use    */
	NCSMEM_AID *maid;	/* passed (stack) space holder for later allocs */

} NCS_SE_MIB_SYNC;

/***************************************************************************
 * MIB Timed : Used by NetPlane MIB services to preserve MIB request timed info
 ***************************************************************************/

typedef struct ncs_se_mib_timed {
	NCS_SE se;		/* All NCS_STACK Stack Elements start with this  */

	uns32 tm_xch_id;	/* transaction manager exchange id              */

} NCS_SE_MIB_TIMED;

/***************************************************************************
 * MIB Orig : Used by NetPlane MIB services to preserve info about the 
 *            original sender of the MIB request 
 ***************************************************************************/

typedef struct ncs_se_mib_orig {
	NCS_SE se;		/* All NCS_STACK Stack Elements start with this  */

	uns64 usr_key;		/* Original user key pointer                    */
	NCSMIB_FNC usr_rsp_fnc;	/* Original user response function              */

} NCS_SE_MIB_ORIG;

/***************************************************************************
 * Forward to PSR : Used by NetPlane OAC to determine which OAC should
 *                  forward SET Requests to the PSR
 ***************************************************************************/

typedef struct ncs_se_forward_to_psr {
	NCS_SE se;		/* All NCS_STACK Stack Elements start with this  */

	NCS_BOOL flag;		/* If TRUE, forward to PSR                      */
} NCS_SE_FORWARD_TO_PSR;

/***************************************************************************
 *
 *     P r i v a t e  H J _ S T A C K   F u n c t i o n s
 *
 ***************************************************************************/

EXTERN_C NCS_SE *get_top_se(NCS_STACK *st);

#endif   /* NCS_STACK_H */
