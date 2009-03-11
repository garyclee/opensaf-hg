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
 */

#ifndef PDRBD_CB_H
#define PDRBD_CB_H


#include "pdrbd.h"


#define   PSEUDO_TASK_PRIORITY        4
#define   PSEUDO_TASK_STACK_SIZE      NCS_STACKSIZE_HUGE
#define   PDRBD_NUM_ARGS              12
#define   PDRBD_MAX_ARG_SIZE          25
#define   PDRBD_SCRIPT_MAX_SIZE       100

#define PDRBD_MAX_PROXIED     5

#define PDRBD_PROXIED_MAX_RES_NAME_LEN          100
#define PDRBD_PROXIED_MAX_MNT_PNT_NAME_LEN      100
#define PDRBD_PROXIED_MAX_DEV_NAME_LEN          100
#define PDRBD_PROXIED_MAX_DATA_DISK_NAME_LEN    100
#define PDRBD_PROXIED_MAX_META_DISK_NAME_LEN    100
#define PDRBD_PROXIED_MAX_COMP_ID_LEN           100
#define PDRBD_PROXIED_MAX_SU_ID_LEN             100
#define PDRBD_PROXIED_MAX_NODE_ID_LEN           100

#define PDRBD_PROXIED_CONF_FILE_PATH       OSAF_SYSCONFDIR "pdrbd_proxied.conf" 

/* No of bytes to read from the pipe written into by script pdrbdctrl */
#define SCRIPT_MSG_SIZE    11

#define SCRIPT_MSG1_SIZE   1
#define SCRIPT_MSG2_SIZE   SCRIPT_MSG_SIZE-SCRIPT_MSG1_SIZE

#define HC_SCRIPT_TIMEOUT_MAX_COUNT     3

typedef enum
{
   PDRBD_PROXIED_COMP_ID_MASK=0xff000000,

   PSEUDO_SCRIPT_EXEC_LOC_MASK=0x00ff0000,

   PSEUDO_SET_PRI_LOC=0x00010000,
   PSEUDO_SET_SEC_LOC=0x00020000,
   PSEUDO_SET_QUI_LOC=0x00030000,
   PSEUDO_REMOVE_LOC=0x00040000,
   PSEUDO_HLTH_CHK_LOC=0x00050000,
   PSEUDO_TERMINATE_LOC=0x00060000,
   PSEUDO_CLEAN_META_LOC=0x00070000,

   PSEUDO_SCRIPT_RET_VAL_MASK=0x000000ff,

   PSEUDO_CSI_SET_OK=0x00000001,
   PSEUDO_CSI_SET_ERR=0x00000002,

   PSEUDO_DRBDADM_NOT_FND=0x00000003,
   PSEUDO_DRBD_MOD_LOD_ERR=0x00000004,
   PSEUDO_PRTN_MNT_ERR=0x00000005,
   PSEUDO_PRTN_UMNT_ERR=0x00000006,
   PSEUDO_DRBDADM_ERR=0x00000007,

   PSEUDO_CSI_REM_OK=0x0000000a,
   PSEUDO_CSI_REM_ERR=0x0000000b,

   PSEUDO_TERM_OK=0x0000000c,
   PSEUDO_TERM_ERR=0x0000000d,

   PSEUDO_CLMT_OK=0x0000000e,
   PSEUDO_CLMT_ERR=0x0000000f,

/*
   Don't change the names and values, otherwise code in pdrbd_proc.c, pdrbd_log.h/.c might fail
   Gathered from DRBD FAQ; www.drbd.org
*/
   PSEUDO_CS_UNCONFIGURED=0x00000010,
   PSEUDO_CS_STANDALONE=0x00000011,
   PSEUDO_CS_UNCONNECTED=0x00000012,
   PSEUDO_CS_WFCONNECTION=0x00000013,
   PSEUDO_CS_WFREPORTPARAMS=0x00000014,
   PSEUDO_CS_CONNECTED=0x00000015,
   PSEUDO_CS_TIMEOUT=0x00000016,
   PSEUDO_CS_BROKENPIPE=0x00000017,
   PSEUDO_CS_NETWORKFAILURE=0x00000018,
   PSEUDO_CS_WFBITMAPS=0x00000019,
   PSEUDO_CS_WFBITMAPT=0x0000001a,
   PSEUDO_CS_SYNCSOURCE=0x0000001b,
   PSEUDO_CS_SYNCTARGET=0x0000001c,
   PSEUDO_CS_PAUSEDSYNCS=0x0000001d,
   PSEUDO_CS_PAUSEDSYNCT=0x0000001e,
   PSEUDO_CS_OTHER=0x0000001f,

   PSEUDO_CS_INVALID=0x000000ff,    /* DRBD not running */

   PSEUDO_PIPE_MSG_VAL_MAX=0xffffffff

} PSEUDO_PIPE_MSG_VALS;

/* Macro used to get the AMF version used */
#define m_PSEUDO_GET_AMF_VER(amfVer) amfVer.releaseCode='B'; amfVer.majorVersion=0x01; amfVer.minorVersion=0x01;

/* Name of pipe to be same as the one used in the script drbdstus */
#define PSEUDO_DRBD_FIFO     "/tmp/pseudo_drbd.fifo"

typedef enum script_spawn_place
{
   PDRBD_PRI_ROLE_ASSIGN_CB,
   PDRBD_SEC_ROLE_ASSIGN_CB,
   PDRBD_QUI_ROLE_ASSIGN_CB,
   PDRBD_REMOVE_CB,
   PDRBD_HEALTH_CHECK_CB,
   PDRBD_TERMINATE_CB,
   PDRBD_CLEAN_META,
   SCRIPT_SPAWN_PLACE_MAX

} SCRIPT_SPAWN_PLACE;


typedef enum pdrbd_evt_type
{
   PDRBD_SCRIPT_CB_EVT,
   PDRBD_PEER_CLEAN_MSG_EVT,
   PDRBD_PEER_ACK_MSG_EVT

} PDRBD_EVT_TYPE;


typedef struct pdrbd_script_status
{
   SaInvocationT    invocation;
   uns32            execute_hdl;
   NCS_BOOL         status;               /* TRUE - Sript is invoked, FALSE - Script exited */

} PDRBD_SCRIPT_STATUS;


/* Fields to be extracted while parsing proxied conf file */
typedef enum proxiedConfFields
{
   PROXIED_CONF_COMP_ID,
   PROXIED_CONF_SU_ID,
   PROXIED_CONF_RES_NAME,
   PROXIED_CONF_DEV_NAME,
   PROXIED_CONF_MOUNT_PNT,
   PROXIED_CONF_DATA_DISK,
   PROXIED_CONF_META_DISK,
   PROXIED_CONF_END

} PROXIED_CONF_FIELDS;


typedef struct pdrbd_proxied_info
{
   struct pseudo_cb *cb_ptr;
   uns32          rsrc_num;
   SaAmfHAStateT  haState;
   uns8           compId[PDRBD_PROXIED_MAX_COMP_ID_LEN];   /* Component ID */
   uns8           suId[PDRBD_PROXIED_MAX_SU_ID_LEN];       /* SU ID */
   SaNameT        compName;        /* Component name in the form: CompID,SUID,NodeID */
   uns8           resName[PDRBD_PROXIED_MAX_RES_NAME_LEN];
   uns8           devName[PDRBD_PROXIED_MAX_DEV_NAME_LEN];
   uns8           mountPnt[PDRBD_PROXIED_MAX_MNT_PNT_NAME_LEN];
   uns8           dataDisk[PDRBD_PROXIED_MAX_DATA_DISK_NAME_LEN];
   uns8           metaDisk[PDRBD_PROXIED_MAX_META_DISK_NAME_LEN];
   PDRBD_SCRIPT_STATUS  script_status_array[SCRIPT_SPAWN_PLACE_MAX];

   NCS_BOOL        regDone;         /* Flag to indicate success of regn with AMF */

   /* Runtime variables at Standby */
   NCS_BOOL        clean_done;      /* Flag is used to track whether cleaning is done once */
   NCS_BOOL        cleaning_metadata;

   /* Runtime variables at Active */
   NCS_BOOL        reconnect_done;      /* Flag is used to track whether reconnect is done once */
   uns32           stdalone_cnt;

} PDRBD_PROXIED_INFO;


/* Data structure to hold the Pseudo's control information */
typedef struct pseudo_cb
{
   SaNameT         compName;        /* Component name */
   SaAmfHandleT    amfHandle;       /* AMF handle obtained during AMF initialisation */
   SaAmfHAStateT   haState;         /* Current state of Pseudo */
   int32           pipeFd;          /* FD of pipe used for comm. between code and script */
   SYSF_MBX        mailBox;
   uns8            nodeId[PDRBD_PROXIED_MAX_NODE_ID_LEN];
   NCSCONTEXT      taskHandle;      /* Pseudo thread/task handle */
   uns32           cb_hdl;          /* Handle for retrieving CB */
   MDS_HDL      mds_hdl;         /* MDS Handle */
   MDS_DEST        my_pdrbd_dest;   /* My PDRBD Adest */
   MDS_DEST        peer_pdrbd_dest; /* Peer PDRBD Adest */
   uns32           my_evt_number;
   uns32           hcTimeOutCnt;    /* Health-check script (pdrbdsts) execution timeout count */

   /* PDRBD Proxied information variables */
   PDRBD_PROXIED_INFO  proxied_info[PDRBD_MAX_PROXIED+1];
   SaAmfHandleT        proxiedAmfHandle;
   int32               noOfProxied;    /* No of proxied components to be maintained */

   NCS_BOOL            compsReady;  /* Flag that indicates whether the proxied (sub) components are ready or not */

} PSEUDO_CB;

extern PSEUDO_CB pseudoCB;


/* Event to be sent from script callback */
typedef struct pdrbd_evt
{
    struct pdrbd_evt    *next;
    PDRBD_EVT_TYPE      evt_type;

    /* Info received from the Script in call-back */
    union {
       NCS_OS_PROC_EXECUTE_TIMED_CB_INFO info;
       uns32  number;
    }evt_data;

} PDRBD_EVT;


/***********************************************************************************

  pDRBD Mailbox manipulation macros

***********************************************************************************/
#define m_PDRBD_RCV_MSG(mbx)            (struct pdrbd_evt *)m_NCS_IPC_NON_BLK_RECEIVE(mbx,NULL)
#define m_PDRBD_SND_MSG(mbx, msg, pri)  m_NCS_IPC_SEND(mbx, msg, pri)


/* Macro to clear context */
#define m_PDRBD_CLR_SCR_CNTXT(cb,cnt,num) \
{ \
   cb->proxied_info[num].script_status_array[cnt].status = FALSE; \
   cb->proxied_info[num].script_status_array[cnt].execute_hdl = 0; \
   cb->proxied_info[num].script_status_array[cnt].invocation = 0; \
}


/* macro to parse the script string */
#define m_SCRIPT_STR_PARSE(st, sc, ac, av, tav) \
{ \
   char str[100], *tok = NULL; \
   /* copy the str as strtok modifies the original str */ \
   strcpy(str, st); \
   ac = 0; \
   if ( NULL != (tok = m_NCS_STRTOK(str, " ")) ) { \
      strcpy(sc, tok); \
      av[ac] = sc; \
   } \
   ac++; \
   while ( (NULL != (tok = m_NCS_STRTOK(NULL, " "))) && \
           (ac < (PDRBD_NUM_ARGS+1)) ) { \
      if ( m_NCS_STRLEN(tok) > PDRBD_MAX_ARG_SIZE ) break; \
      strcpy(tav[ac], tok); \
      av[ac] = tav[ac]; \
      ac++;\
   } \
   if ( NULL != tok ) { \
      sc[0] = (char)(long)NULL; \
      av[0] = NULL; \
      ac = 0; \
   } else \
      av[ac] = NULL; \
}

#endif /* PDRBD_H */

