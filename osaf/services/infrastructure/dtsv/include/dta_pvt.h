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

  Flex Log Agent (DTA) abstractions and function prototypes.

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef DTA_PVT_H
#define DTA_PVT_H

#include "ncspatricia.h"

#define DTA_MDS_SEND_TIMEOUT  1500

typedef struct reg_tbl_entry {
	/*NCS_QELEM  qel; */	/* Must be first in struct, Queue element */
	/*Change reg table frm a queue to a patricia tree */
	NCS_PATRICIA_NODE node;

	/* Key for the Reg table entry */
	SS_SVC_ID svc_id;	/* Service ID registerd with the DTA & DTS */
	bool log_msg;	/* Set to true ifreceived reg-confirmation */
	/* No need of policy handles */
	/*uint32_t       policy_hdl; */
	/*Flag to indicate svc_reg entry on Active DTS upon DTS fail-over */
	bool svc_flag;	/* Set to true if new policy handles received, else setto false */

	/* Message Filter for the above defined service */
	bool enable_log;	/* Enable or disable logging */
	uint32_t category_bit_map;	/* Category Filter bit map */
	uint8_t severity_bit_map;	/* Severity Filter bit map */
	/* Add all the other new filter elements above this */

	/* version field */
	uint16_t version;
	/* Service name */
	char svc_name[DTSV_SVC_NAME_MAX];

} REG_TBL_ENTRY;

/* Link-list for buffering log messages till DTS comes up */
typedef struct dta_buffered_log {
	DTSV_MSG *buf_msg;
	struct dta_buffered_log *next;
} DTA_BUFFERED_LOG;

typedef struct dta_log_buffer {
	DTA_BUFFERED_LOG *head;
	DTA_BUFFERED_LOG *tail;
	uint32_t num_of_logs;
} DTA_LOG_BUFFER;

typedef struct dta_cb {
	/* Configuration Objects */
	NCS_LOCK lock;

	/*NCS_QUEUE           reg_tbl; */	/* Service registration table */
	/*Change reg tbl from a queue to a Patricia tree */
	NCS_PATRICIA_TREE reg_tbl;

	NCSCONTEXT task_handle;

	/* run-time learned values */
	MDS_DEST dta_dest;

	MDS_DEST dts_dest;
	NODE_ID dts_node_id;	/* Node-id on which "i_dest" lives */
	PW_ENV_ID dts_pwe_id;

	bool dts_exist;
	bool created;	/* true : CB created, false : CB destroyed */

	/* Create time constants */
	MDS_HDL mds_hdl;

	/* Link-list for buffering log messages till DTS comes up */
	DTA_LOG_BUFFER log_buffer;

#if (DTA_FLOW == 1)
	/* Additions for flow control */
	uint32_t logs_received;
	bool dts_congested;
	uint32_t msg_count;
#endif

	/* DTA MDS sub-part version */
	MDS_SVC_PVT_SUB_PART_VER dta_mds_version;
	/* Version of Active DTS as seen by the DTA */
	MDS_SVC_PVT_SUB_PART_VER act_dts_ver;

} DTA_CB;

DTA_CB dta_cb;

/* Limit of 1000 logs can be buffered */
#define DTA_BUF_LIMIT 1000

/* Versioning changes - Defining min/max message format version to be handled */
#define DTA_MDS_MIN_MSG_FMAT_VER_SUPPORT 1
#define DTA_MDS_MAX_MSG_FMAT_VER_SUPPORT 1

#define DTA_MIN_ACT_DTS_MDS_SUB_PART_VER 1
#define DTA_MAX_ACT_DTS_MDS_SUB_PART_VER 2

#define DTA_MDS_SUB_PART_VERSION 2

/************************************************************************
  Basic Layer Management Service entry points off std LM API
*************************************************************************/

uint32_t dta_svc_create(NCSDTA_CREATE *create);
uint32_t dta_svc_destroy(NCSDTA_DESTROY *destroy);

/************************************************************************
  DTA tasking loop functions.
*************************************************************************/
void dta_do_evts(SYSF_MBX *mbx);
uint32_t dta_do_evt(DTSV_MSG *msg);

/************************************************************************
  SE_API entry functions.
*************************************************************************/
uint32_t dta_log_msg(NCSFL_NORMAL *msg);
uint32_t dta_reg_svc(NCS_BIND_SVC *bind_svc);
uint32_t dta_dereg_svc(SS_SVC_ID svc_id);

/************************************************************************
  MDS bindary stuff for DTA
*************************************************************************/
uint32_t ncs_logmsg_int(SS_SVC_ID svc_id,
			      uint32_t inst_id,
			      uint8_t fmat_id,
			      uint8_t str_table_id, uint32_t category, uint8_t severity, char *fmat_type, va_list argp);

uint32_t dta_get_ada_hdl(void);

uint32_t dta_mds_install_and_subscribe(void);

uint32_t dta_mds_uninstall(void);

uint32_t dta_mds_callback(NCSMDS_CALLBACK_INFO *cbinfo);

#if (DTA_FLOW == 1)
uint32_t dta_mds_sync_send(DTSV_MSG *msg, DTA_CB *inst, uint32_t timeout, bool svc_reg);
#else
uint32_t dta_mds_sync_send(DTSV_MSG *msg, DTA_CB *inst, uint32_t timeout);
#endif

uint32_t dta_mds_async_send(DTSV_MSG *msg, DTA_CB *inst);

uint32_t dta_mds_rcv(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg);

void dta_mds_evt(MDS_CALLBACK_SVC_EVENT_INFO svc_info, MDS_CLIENT_HDL yr_svc_hdl);

uint32_t dta_mds_enc(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg,
			   SS_SVC_ID to_svc, NCS_UBAID *uba,
			   MDS_SVC_PVT_SUB_PART_VER remote_ver, MDS_CLIENT_MSG_FORMAT_VER *msg_fmat_ver);

uint32_t dta_mds_dec(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT *msg,
			   SS_SVC_ID to_svc, NCS_UBAID *uba, MDS_CLIENT_MSG_FORMAT_VER msg_fmat_ver);

uint32_t dta_mds_cpy(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg,
			   SS_SVC_ID to_svc, NCSCONTEXT *cpy,
			   bool last, MDS_SVC_PVT_SUB_PART_VER remote_ver, MDS_CLIENT_MSG_FORMAT_VER *msg_fmat_ver);

uint32_t encode_ip_address(NCS_UBAID *uba, NCS_IP_ADDR ipa);

uint32_t dta_log_msg_encode(NCSFL_NORMAL *logmsg, NCS_UBAID *uba);

uint32_t dta_copy_octets(char **dest, char *src, uint16_t length);

bool dta_match_service(void *key, void *qelem);

void copy_ip_addr(NCS_IP_ADDR *ipa, va_list argp);

uint32_t dta_svc_reg_config(DTA_CB *inst, DTSV_MSG *msg);

uint32_t dta_svc_reg_check(DTA_CB *inst);

uint32_t dta_svc_reg_updt(DTA_CB *inst, uint32_t svc_id, uint32_t enable_log,
				uint32_t category_bit_map, uint8_t severity_bit_map);

/***********************************************************************
*     Flex Log policy rrelated functions 
************************************************************************/
uint32_t dta_svc_reg_log_en(REG_TBL_ENTRY *svc, NCSFL_NORMAL *lmsg);
uint32_t dta_fill_reg_msg(DTSV_MSG *msg, SS_SVC_ID svc_id, const uint16_t version, const char *svc_name,
				uint8_t operation);

#define MAX_OCT_LEN             255

#define m_NCS_DTA_VALIDATE_STR_LENGTH(len) \
{ \
   if (len > MAX_OCT_LEN) \
      return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "String length exceeds 255 characters."); \
}

/* The DTA Send and Recieve mailbox manipulations */

#define m_DTA_RCV_MSG(mbx,msg)      (DTSV_MSG *)m_NCS_IPC_RECEIVE(mbx,msg)
#define m_DTA_SND_MSG(mbx,msg, pri)  m_NCS_IPC_SEND(mbx, msg, pri)

/* Versioning support - version copying routine */
#define m_DTA_VER_COPY(src, dest) \
{ \
   dest.releaseCode = src.releaseCode; \
   dest.majorVersion = src.majorVersion; \
   dest.minorVersion = src.minorVersion; \
}

/************************************************************************
 ************************************************************************

    I n d e x   M a n i p u l a t i o n   M a c r o s

  One of the key types in Flexlog is the Index value, which leads to a
  canned printable string. This value is as follows:

  uint32_t  index = byte1byte2byte3byte4
  ===================================
  Most segnificant bytes-byte1byte2 : Table ID otherwise known as an NCSFL_SET.
  Least segnificant bytes-byte3byte4 : String ID offset in said table.

 ************************************************************************
 ************************************************************************/
#define m_NCSFL_MAKE_IDX(i,j)   (uint32_t)((i<<16)|j)

/************************************************************************

  D i s t r i b u t e d    T r a c i n g    A g e n t    L o c k s

*************************************************************************/

#if (NCSDTA_USE_LOCK_TYPE == DTA_NO_LOCKS)	/* NO Locks */

#define m_DTA_LK_CREATE(lk)
#define m_DTA_LK_INIT
#define m_DTA_LK(lk)
#define m_DTA_UNLK(lk)
#define m_DTA_LK_DLT(lk)
#elif (NCSDTA_USE_LOCK_TYPE == DTA_TASK_LOCKS)	/* Task Locks */

#define m_DTA_LK_CREATE(lk)
#define m_DTA_LK_INIT            m_INIT_CRITICAL
#define m_DTA_LK(lk)             m_START_CRITICAL
#define m_DTA_UNLK(lk)           m_END_CRITICAL
#define m_DTA_LK_DLT(lk)
#elif (NCSDTA_USE_LOCK_TYPE == DTA_OBJ_LOCKS)	/* Object Locks */

#define m_DTA_LK_CREATE(lk)      m_NCS_LOCK_INIT_V2(lk,NCS_SERVICE_ID_DTSV, \
                                                    DTA_LOCK_ID)
#define m_DTA_LK_INIT
#define m_DTA_LK(lk)             m_NCS_LOCK_V2(lk,   NCS_LOCK_WRITE, \
                                                    NCS_SERVICE_ID_DTSV, DTA_LOCK_ID)
#define m_DTA_UNLK(lk)           m_NCS_UNLOCK_V2(lk, NCS_LOCK_WRITE, \
                                                    NCS_SERVICE_ID_DTSV, DTA_LOCK_ID)
#define m_DTA_LK_DLT(lk)         m_NCS_LOCK_DESTROY_V2(lk, NCS_SERVICE_ID_DTSV, \
                                                    DTA_LOCK_ID)
#endif

#endif   /* DTA_PVT_H */
