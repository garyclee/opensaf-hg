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

  Flex Log Agent (DTA) abstractions and function prototypes

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef DTA_API_H
#define DTA_API_H

typedef struct ncsdta_create {
	NCSCONTEXT i_mds_hdl;	/* MDS hdl for 'installing'                    */
	uint8_t i_hmpool_id;	/* Handle Manager Pool Id                      */

} NCSDTA_CREATE;

/***************************************************************************
 * Destroy an instance of a DTA service instance (one per virtual router)
 ***************************************************************************/

typedef struct ncsdta_destroy {
	uns32 i_dta_key;
	void *i_meaningless;	/* place holder struct; do nothing */

} NCSDTA_DESTROY;

/***************************************************************************
 * The operations set that a DTA instance supports
 ***************************************************************************/

typedef enum dta_lm_op {
	DTA_LM_OP_CREATE,
	DTA_LM_OP_DESTROY,

} NCSDTA_OP;

/***************************************************************************
 * The DTA API single entry point for all services
 ***************************************************************************/

typedef struct ncsdta_arg {
	NCSDTA_OP i_op;		/* Operation; CREATE,DESTROY */

	union {
		NCSDTA_CREATE create;
		NCSDTA_DESTROY destroy;
	} info;

} DTA_LM_ARG;

/***************************************************************************
 * Global Instance of Layer Management
 ***************************************************************************/

uns32 dta_lm(DTA_LM_ARG *arg);

/***************************************************************************
 * Global Instance of DTA mailbox
 ***************************************************************************/
SYSF_MBX gl_dta_mbx;

uns32 dta_cleanup_seq(void);

#define DTA_CONGESTION_LOG_LIMIT 50
#define DTA_UNCONGESTED_LOG_LIMIT 100
#define DTA_MAX_THRESHOLD 2000

#endif   /* DTA_API_H */
