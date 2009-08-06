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

  _Private_ MIB Access Client (MAC) abstractions and function prototypes.

*******************************************************************************/

/*
 * Module Inclusion Control...
 */
#ifndef MAC_PVT_H
#define MAC_PVT_H

/*****************************************************************************

  M A C _ I N S T : A MAC instance maps to a virtual router instance. This 
                    implies a parallel MAS and OAC instances that also reside 
                    in this same 'Private World Environment'.

*****************************************************************************/

#define MAC_XCHG_HASH_SIZE  10

#define MAA_MDS_SUB_PART_VERSION 2	/* maa version is increased as ncsmib_arg is changed(handles & ncsmib_rsp changed) */
#define MAA_WRT_MAS_SUBPART_VER_AT_MIN_MSG_FMT    1	/* minimum version of MAS supported by MAA */
#define MAA_WRT_MAS_SUBPART_VER_AT_MAX_MSG_FMT    2	/* maximum version of MAS supported by MAA */

/* size of the array used to map MAS subpart version to mesage format version */
#define  MAA_WRT_MAS_SUBPART_VER_RANGE                \
         (MAA_WRT_MAS_SUBPART_VER_AT_MAX_MSG_FMT  -   \
         MAA_WRT_MAS_SUBPART_VER_AT_MIN_MSG_FMT + 1)

#define MAA_WRT_OAC_SUBPART_VER_AT_MIN_MSG_FMT    1	/* minimum version of OAC supported by MAA */
#define MAA_WRT_OAC_SUBPART_VER_AT_MAX_MSG_FMT    2	/* maximum version of OAC supported by MAA */

/* size of the array used to map OAC subpart version to mesage format version */
#define  MAA_WRT_OAC_SUBPART_VER_RANGE                \
         (MAA_WRT_OAC_SUBPART_VER_AT_MAX_MSG_FMT  -   \
         MAA_WRT_OAC_SUBPART_VER_AT_MIN_MSG_FMT + 1)

typedef struct mac_inst {
	/* Configuration Objects */

	NCS_BOOL mac_enbl;	/* RW ENABLE|DISABLE MAC services           */
	NCS_BOOL playback_session;
	uns16 *plbck_tbl_list;

	NCS_LOCK lock;

	/* run-time learned values */

	MDS_DEST my_vcard;
	MDS_DEST mas_vcard;
	NCS_BOOL mas_here;

	MDS_DEST psr_vcard;
	NCS_BOOL psr_here;

	/* Create time constants */

	char *logf_name;
	MDS_HDL mds_hdl;
	NCS_VRID vrid;
	uns8 hm_poolid;
	uns32 hm_hdl;
	NCS_SEL_OBJ mas_sync_sel;
	SYSF_MBX *mbx;
	MAB_LM_CB lm_cbfnc;

} MAC_INST;

/* default Env - Id */
#define m_MAC_DEFAULT_ENV_ID    1

/************************************************************************

  M A C   P r i v a t e   F u n c t i o n   P r o t o t y p e s

*************************************************************************/
/************************************************************************
  MAC tasking loop function
*************************************************************************/

EXTERN_C MABMAC_API void mac_do_evts(SYSF_MBX *mbx);
EXTERN_C uns32 mac_do_evt(MAB_MSG *msg);

/************************************************************************
  Basic MAC Layer Management Service entry points off std LM API
*************************************************************************/

EXTERN_C uns32 mac_svc_create(NCSMAC_CREATE *create);
EXTERN_C uns32 mac_svc_destroy(NCSMAC_DESTROY *destroy);
EXTERN_C uns32 mac_svc_get(NCSMAC_GET *get);
EXTERN_C uns32 mac_svc_set(NCSMAC_SET *set);

/***********************************************************************
 MAC callback routine that MAC puts in NCSMIB_ARG structure
************************************************************************/

EXTERN_C uns32 mac_mib_response(MAB_MSG *msg);
EXTERN_C uns32 mac_playback_start(MAB_MSG *msg);
EXTERN_C uns32 mac_playback_end(MAB_MSG *msg);

/************************************************************************
  MDS bindary stuff for MAC
*************************************************************************/
EXTERN_C uns32 mac_mds_cb(NCSMDS_CALLBACK_INFO *cbinfo);

/************************************************************************

  M I B    A c e s s   C l i e n t    L o c k s

 The MAC locks follow the lead of the master MIB Access Broker setting
 to determine what type of lock to implement.

*************************************************************************/

#if (NCSMAB_USE_LOCK_TYPE==MAB_NO_LOCKS)	/* NO Locks */

#define m_MAC_LK_CREATE(lk)
#define m_MAC_LK_INIT
#define m_MAC_LK(lk, perm)
#define m_MAC_UNLK(lk, perm)
#define m_MAC_LK_DLT(lk)
#elif (NCSMAB_USE_LOCK_TYPE==MAB_TASK_LOCKS)	/* Task Locks */

#define m_MAC_LK_CREATE(lk)
#define m_MAC_LK_INIT            m_INIT_CRITICAL
#define m_MAC_LK(lk, perm)             m_START_CRITICAL
#define m_MAC_UNLK(lk, perm)           m_END_CRITICAL
#define m_MAC_LK_DLT(lk)
#elif (NCSMAB_USE_LOCK_TYPE==MAB_OBJ_LOCKS)	/* Object Locks */

#define m_MAC_LK_CREATE(lk)   m_NCS_LOCK_INIT_V2(lk,NCS_SERVICE_ID_MAB, \
                                                 MAC_LOCK_ID)
#define m_MAC_LK_INIT
#define m_MAC_LK(lk, perm)    m_NCS_LOCK_V2(lk,   perm, \
                                            NCS_SERVICE_ID_MAB, MAC_LOCK_ID)
#define m_MAC_UNLK(lk, perm)  m_NCS_UNLOCK_V2(lk, perm, \
                                              NCS_SERVICE_ID_MAB, MAC_LOCK_ID)
#define m_MAC_LK_DLT(lk)      m_NCS_LOCK_DESTROY_V2(lk, NCS_SERVICE_ID_MAB, \
                                                    MAC_LOCK_ID)
#endif

#endif   /* MAC_PVT_H */
