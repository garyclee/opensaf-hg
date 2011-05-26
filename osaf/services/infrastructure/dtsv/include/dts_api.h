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

 _Public_ Flex Log Server (DTS) abstractions and function prototypes

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef DTS_API_H
#define DTS_API_H

/******************************************************************************

 Global API 

    Layer Management API as single entry point

    Create         - Create an instance of DTS (one global instance)
    Destroy        - Destroy the global instance of DTS

*******************************************************************************/

typedef struct dts_create {
	uint8_t i_hmpool_id;	/* Handle Manager Pool Id                      */
	NCSCONTEXT task_handle;
	NCS_BOOL reg_with_amf;
} DTS_CREATE;

/***************************************************************************
 * Destroy an instance of a DTS service instance (one per virtual router)
 ***************************************************************************/

typedef struct ncsdts_destroy {
	void *i_meaningless;	/* place holder struct; do nothing */

} DTS_DESTROY;

/***************************************************************************
 * The operations set that a DTS instance supports
 ***************************************************************************/

typedef enum dts_lm_op {
	DTS_LM_OP_CREATE,
	DTS_LM_OP_DESTROY,

} DTS_OP;

/***************************************************************************
 * The DTS API single entry point for all services
 ***************************************************************************/

typedef struct ncsdts_arg {
	DTS_OP i_op;		/* Operation; CREATE,DESTROY,GET,SET */

	union {
		DTS_CREATE create;
		DTS_DESTROY destroy;
	} info;

} DTS_LM_ARG;

/* New structure to act as index for loaded ASCII_SPEC library patricia tree */
typedef struct ascii_spec_index {
	SS_SVC_ID svc_id;	/* The service id corres. to the spec */
	uint16_t ss_ver;		/* The version id of the spec */
} ASCII_SPEC_INDEX;

/***************************************************************************
 * Global Instance of Layer Management
 ***************************************************************************/

uns32 dts_lm(DTS_LM_ARG *arg);

#endif   /* DTS_API_H */
