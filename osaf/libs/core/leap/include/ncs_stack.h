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
	uint16_t svcid;		/* Back to Subcomponent's Service ID */
	MDS_DEST vcard;		/* Back to Virtual Card ID */

} NCS_SE_BACKTO;


/***************************************************************************
 *
 *     P r i v a t e  H J _ S T A C K   F u n c t i o n s
 *
 ***************************************************************************/

NCS_SE *get_top_se(NCS_STACK *st);

#endif   /* NCS_STACK_H */
