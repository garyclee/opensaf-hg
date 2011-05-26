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

  DESCRIPTION: Common DTSv sub-part messaging formats that travels between 
  DTS and DTA.
 

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef DTSV_MSG_H
#define DTSV_MSG_H

/**************************************************************************
* DTSV encode decode message format sizes.
***************************************************************************/

#define DTSV_DTS_DTA_MSG_HDR_SIZE    ((sizeof(uint16_t)) + (sizeof(uint8_t)))
#define DTSV_REG_CONF_MSG_SIZE       ((2 * sizeof(uns32)) + sizeof(uint8_t) + sizeof(NCS_BOOL))
#define DTSV_FLTR_MSG_SIZE           ((2 * sizeof(uns32)) + sizeof(uint8_t) + sizeof(NCS_BOOL))
#define DTSV_DTA_DTS_HDR_SIZE        (sizeof(uint16_t) + sizeof(uint8_t))
#define DTS_LOG_MSG_HDR_SIZE         ((5 * sizeof(uns32)) + (sizeof(uint16_t)) + (2 * sizeof(uint8_t)))
#define DTS_MAX_SIZE_DATA             512
#define DTS_MAX_DBL_DIGITS            30
/**************************************************************************
* DTSV OP_REQ_CLI encode/decode message format sizes.
***************************************************************************/
#define DTSV_ADD_GBL_CONS_SIZE  2*sizeof(uint8_t)
#define DTSV_RMV_GBL_CONS_SIZE  sizeof(uint8_t)
#define DTSV_ADD_NODE_CONS_SIZE ((2*sizeof(uint8_t)) + sizeof(uns32))
#define DTSV_RMV_NODE_CONS_SIZE (sizeof(uint8_t)+ sizeof(uns32))
#define DTSV_ADD_SVC_CONS_SIZE  ((2*sizeof(uint8_t)) + (2*sizeof(uns32)))
#define DTSV_RMV_SVC_CONS_SIZE  (sizeof(uint8_t)+ (2*sizeof(uns32)))
#define DTSV_CLI_MAX_SIZE  256

/**************************************************************************
* DTSV OP_RSP_CLI encode/decode message format sizes.
***************************************************************************/
#define DTSV_RSP_CONS_SIZE    2*sizeof(uint8_t)

/* DTSV service name(for registration/ASCII_SPEC) limit */
#define DTSV_SVC_NAME_MAX     15

/***************************************************************************
 * DTS sub-component op codes implies presence of particular fields
 ***************************************************************************/

typedef enum dts_svc_msg_type {

	DTS_DTA_EVT_RCV,
	/* service entities dispatched to Flex Log Agent (DTA) */
	DTS_SVC_REG_CONF,	/* Registration confirmation message send 
				   filter information with reg confirmation. */
	DTS_SVC_MSG_FLTR,

	/* service entities dispatched to Flex Log Server (DTS) */

	DTA_REGISTER_SVC,	/* data = Service ID of the registering service  */
	DTA_UNREGISTER_SVC,	/* data = Service ID of the registering service  */
	DTA_LOG_DATA,		/* NCSFL_NORMAL and Policy handle */
	DTSV_DUMP_SEQ_MSGS,

	/* New message to indicate AMF is up for DTS to configure itself with AMF */
	DTS_AMF_COMPONENTIZE,
	/* New message to indicate act DTS of fail-over */
	DTS_FAIL_OVER,
	/* New message to signal completion of Quiesced state */
	DTS_QUIESCED_CMPLT,
	/* New message to signal ascii_spec reload frm CLI */
	DTS_SPEC_RELOAD,
	/* New message for current DTS config prints */
	DTS_PRINT_CONFIG,
	/* Message used by DTA for DTS UP indication */
	DTS_UP_EVT,
	/* Message indicating DTA lib destroy call */
	DTA_DESTROY_EVT,
	/* Message for Congestion estimation */
	DTA_FLOW_CONTROL,
	/* Message type indicating the congestion state of DTS as seen by DTA */
	DTS_CONGESTION_HIT,
	DTS_CONGESTION_CLEAR,
	DTS_IMMND_EVT_RCV
} DTS_SVC_MSG_TYPE;

/***************************************************************************
 * Private: DTA to DTS exchange info with these containers
 ***************************************************************************/

typedef struct ms_time {
	uns32 seconds;
	uns32 millisecs;
} MS_TIME;

/************************************************************************
  NCSFL_HDR

  This struct is that part of a Flexlog normalized form data structure
  that is common to any particular logging entity.

 ************************************************************************/

typedef struct ncsfl_hdr {
	SS_SVC_ID ss_id;	/* logging entity's subsystem ID      */
	uns32 inst_id;		/* Instance ID of the service         */
	uint8_t severity;		/* as per IEFT-draft syslog           */
	uns32 category;		/* generic category event belongs to  */
	MS_TIME time;		/* time stamp; filled by Flexlog Agent */
	uint8_t fmat_id;		/* offset into format strings         */
	char *fmat_type;	/* format argument sequence type      */
	uint8_t str_table_id;

} NCSFL_HDR;

/************************************************************************
  NCSFL_NORMAL

  This struct is the normalized form that any Flexlog client must use
  to present logging info the the Flexlog service.

 ************************************************************************/

typedef struct ncsfl_msg {
	NCSFL_HDR hdr;		/* Common header stuff for any event  */
	NCS_UBAID uba;

} NCSFL_NORMAL;

typedef struct svc_reg {	/* Data associated with Service Registration with DTS  */

	SS_SVC_ID svc_id;
	uint16_t version;
	char svc_name[DTSV_SVC_NAME_MAX];

} SVC_REG;

typedef struct svc_unreg {	/* Data associated with Service Unregistration */

	SS_SVC_ID svc_id;
	uint16_t version;
	char svc_name[DTSV_SVC_NAME_MAX];

} SVC_UNREG;

typedef struct dta_log_msg {	/* Data associated with registration confirmation */

	NCSFL_NORMAL log_msg;
	/* Versioning changes: Field to indicate use of fmat type 'D' & DTS version
	 * for which the msg was encoded. Default value is 0.
	 */
	uint8_t msg_fmat_ver;

} DTA_LOG_MSG;

/***************************************************************************
 * Private: DTS to DTA exchange info with these containers
 ***************************************************************************/

typedef struct log_msg_fltr {	/* Data associated with the Service specific log filter   */

	SS_SVC_ID svc_id;	/* Service ID of the service */
	NCS_BOOL enable_log;	/* TRUE = Enable; FALSE = Disable */
	uns32 category_bit_map;	/* Category filter Bit map  */
	uint8_t severity_bit_map;	/* Severity filter Bit Map */

	/* No need of policy handles */
	/*uns32            policy_hdl; */

} LOG_MSG_FLTR;

typedef struct svc_reg_conf {	/* Data associated with registration confirmation */

	LOG_MSG_FLTR msg_fltr;
} SVC_REG_CONF;

typedef struct dts_dta_evt {	/*Event change received from DTA */

	NCSMDS_CHG change;
} DTS_DTA_EVT;

/***************************************************************************
 * Private: DTS has different message content based on Service Operation
 ***************************************************************************/

typedef struct dtsv_msg_data {	/* The union of all data types in a DTSV_MSG */
	union {

		DTS_DTA_EVT evt;
		LOG_MSG_FLTR msg_fltr;	/* Message for setting the log filter */
		SVC_REG_CONF reg_conf;	/* Registration confirmation Message */
		SVC_REG reg;	/* Service registration message */
		SVC_UNREG unreg;	/* Service deregistration Message */
		DTA_LOG_MSG msg;	/* Log Message */
		uns64 dta_ptr;	/* Pointer to DTA node in patricia tree */
	} data;

} DTSV_MSG_DATA;

/***************************************************************************
 * Private: DTS message passing structure
 ***************************************************************************/

typedef struct dtsv_msg {
	struct dtsv_msg *next;	/* for a linked list of them                    */
	NCS_BOOL seq_msg;	/* Set to TRUE if message is received from the 
				 * Sequencing buffer */

	NCS_BOOL rsp_reqd;	/* TRUE if send is awaiting a response            */
	MDS_SYNC_SND_CTXT msg_ctxt;	/* Valid only if "i_rsp_expected == TRUE"         */
	NODE_ID node;		/* Senders physical card number */
	MDS_DEST dest_addr;	/* Senders destination address */
	DTS_SVC_MSG_TYPE msg_type;	/* encoded by sender to proper subservice       */
	DTSV_MSG_DATA data;	/* Data corresponding to the type               */

} DTSV_MSG;

#endif
