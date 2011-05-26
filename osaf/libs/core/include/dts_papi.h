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

#ifndef DTS_PAPI_H
#define DTS_PAPI_H

#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncs_iplib.h"
#include "os_defs.h"
#include "ncs_main_papi.h"
#include "ncssysf_tsk.h"

#include "ncs_svd.h"
#include "ncs_ubaid.h"
#include "ncssysf_def.h"
#include "ncs_lib.h"
#include "ncssysf_def.h"

#include "ncs_log.h"

/************************************************************************

  F l e x l o g    R e g i s t r a t i o n   T i m e   S t r u c t s

  The following diagram provides a roadmap of how the various typedefs are
  intended to be used to build up such information.

                                           NCSFL_STR[]
                                       +->+------------+
                         NCSFL_SET[]    |  | str ID     |
                     +->+------------+ |  | str value  +-> "COLD SYNC Sent"
   NCSFL_ASCII_SPEC   |  | set ID     | |  + - - - - - -+
  +----------------+ |  | set count  | |  | str ID     |
  | Subsys ID      | |  | set values +-+  | str value  +-> "COLD SYNC Rcvd"
  | Subsys version | |  + - - - - - -+    + - - - - - -+
  | log file name  | |  | set ID     |    |            |
  | fopen Banner   | |  | set count  |    | ... n      |
  | string set     +-+  | set values +-+  +------------+
  | format set     +-+  + - - - - - -+ |
  | string count   | |  |            | +-> (etc.)
  | format count   | |  | ... n      |
  +----------------+ |  +------------+
                     |
                     |      NCSFL_FMAT[]
                     +--->+------------+
                          | format ID  |
                          | format type|
                          | format str +->"A formatted %s log str %d\n"
                          +------------+
                          | format ID  |
                          | format type|
                          | format str +->"Another %s log str %d\n"
                          +------------+
                          |            |
                          | ... n      |
                          +------------+

  Note That NCSFL_SET, NCSFL_STR and NCSFL_FMAT are to be initialized and
  shall be treated as NULL terminated arrays.

  Note that all three of these types include an ID field. The ID field
  value is set equal to the index value that this element represents in
  its respecfice array. This allows flexlog to perform a simple test at
  registration time to confirm proper data alignment.

*************************************************************************/

/************************************************************************
  NCSFL_STR

      This structure maps a relationship between an offset value (str_id)
      and a canned string (str_val).

      The str_id allows Flexlog to do a simple sanity check to make sure
      there is alignment between a numeric value expressed as a mnemonic
      and a particular string value that will be presented to a human.

      Such mappings with no sanity checking can be error prone and hard
      to detect.

 ************************************************************************/

typedef struct ncsfl_str {
	uns16 str_id;
	char *str_val;

} NCSFL_STR;

/************************************************************************
  NCSFL_SET

      This structure maps a relationship between aan offset value (set_id)
      and an array set of canned strings (set_val) housed in NCSFL_STR
.     structures.

      The set_id allows Flexlog to do a simple sanity check to make sure
      there is alignment between a numeric value expressed as a mnemonic
      and a particular set value that leads to the correct set of strings.

      The set_cnt field is a place where flexlog can put its count value
      of how many canned strings there are. This allows another layer of
      run-time checking when DEBUGGING is enabled.

************************************************************************/

typedef struct ncsfl_set {
	uns16 set_id;
	uns16 set_cnt;
	NCSFL_STR *set_vals;

} NCSFL_SET;

/************************************************************************
  NCSFL_FMAT

      This structure maps a relationship between aan offset value (fmat_id)
      and an array set of canned formatted strings (fmat_str).

      The fmat_id allows Flexlog to do a simple sanity check to make sure
      there is alignment between a numeric value expressed as a mnemonic
      and a particular set value that leads to the correct formatted string.

      The fmat_type field explains the layout of the argument types that
      are needed/expected in order to properly use this formatted string.

 ************************************************************************/

typedef struct ncsfl_fmat {
	uns8 fmat_id;
	char *fmat_type;
	char *fmat_str;

} NCSFL_FMAT;

/************************************************************************
 NCSFL_ASCII_SPEC

     This data structure houses the set of information that a Subsystem
     registers with FlexLog at startup time.

     It contains everything Flexlog needs to print any logging message
     this particular subsystem can generate.

 ************************************************************************/

typedef struct ncsfl_ascii_spec {

	/* PUBLIC part of ASCII SPEC data struct filled in by subsystem */

	SS_SVC_ID ss_id;	/* subsystem Identifier             */
	uns16 ss_ver;		/* subsystem version identifier */
	char *svc_name;		/* Service name of the service; Should 
				   be as concise as possible,
				   Ex. "DTSV" for distributed tracing service */

	NCSFL_SET *str_set;	/* canned strings used in messages  */

	NCSFL_FMAT *fmat_set;	/* format template information      */

	/* PRIVATE part of the ASCII SPEC data struct used by Flexlog   */

	uns16 str_set_cnt;	/* count of canned string sets      */
	uns16 fmat_set_cnt;	/* count of format string sets      */

} NCSFL_ASCII_SPEC;

/***************************************************************************
 * dtsv de-register canned strings (ASCII_SCPEC_TABLE)
 ***************************************************************************/

typedef struct ncs_deregister_ascii_spec {
	SS_SVC_ID svc_id;	/* Service ID of the service wants to de-register ascii spec */
	uns16 version;		/* Version of ASCII_SPEC being de-registered. */

} NCS_DEREGISTER_ASCII_SPEC;
/***************************************************************************
 * dtsv register canned strings (ASCII_SCPEC_TABLE)
 ***************************************************************************/

typedef struct ncs_register_ascii_spec {
	NCSFL_ASCII_SPEC *spec;	/* Ascii spec table to be register with DTSv */

} NCS_REGISTER_ASCII_SPEC;

/***************************************************************************
 * The operations set that a DTA instance supports
 ***************************************************************************/

typedef enum ncs_dtsv_ascii_spec_op {
	NCS_DTSV_OP_ASCII_SPEC_REGISTER,
	NCS_DTSV_OP_ASCII_SPEC_DEREGISTER
} NCS_DTSV_ASCII_SPEC_OP;

/***************************************************************************
 * The DTSV API single entry point for all services
 ***************************************************************************/
typedef struct ncs_dtsv_reg_canned_str {
	NCS_DTSV_ASCII_SPEC_OP i_op;	/* Operation; Register, deregister */

	union {
		NCS_REGISTER_ASCII_SPEC reg_ascii_spec;
		NCS_DEREGISTER_ASCII_SPEC dereg_ascii_spec;

	} info;

} NCS_DTSV_REG_CANNED_STR;

/***************************************************************************
 * DTSV public API's for registering ASCII strings
 ***************************************************************************/
uns32 ncs_dtsv_ascii_spec_api(NCS_DTSV_REG_CANNED_STR *arg);
uns32 dts_lib_req(NCS_LIB_REQ_INFO *req_info);

#endif   /* DTS_PAPI_H */
