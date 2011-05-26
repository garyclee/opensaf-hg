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

#include <configmake.h>
/*****************************************************************************
  DESCRIPTION:
          
 _Private_ Flex Log Server (DTS) abstractions and function prototypes.
            
*******************************************************************************/

/*
* Module Inclusion Control...
*/

#ifndef DTS_PVT_H
#define DTS_PVT_H

#include "mds_papi.h"
#include "saImmOi.h"
#include "ncssysf_tmr.h"
#include "ncs_queue.h"

/* File Types */
#define   GLOBAL_FILE        0x00
#define   PER_NODE_FILE      0x01
#define   PER_SVC_FILE       0x02

/* Loging devices */
#define LOG_FILE             0x80
#define CIRCULAR_BUFFER      0x40
#define OUTPUT_CONSOLE       0x20

/* Log Format */
#define COMPRESSED_FORMAT    0x01
#define EXPANDED_FORMAT      0x02


#define  DTS_AMF_HEALTH_CHECK_KEY  "A9FD64E12C12"	/* Some randomely generated key */
#define  DTS_NULL_HA_STATE         0
#define  DTS_HB_INVOCATION_TYPE    SA_AMF_HEALTHCHECK_AMF_INVOKED
#define  DTS_RECOVERY              SA_AMF_COMPONENT_FAILOVER

/* Default settings for Global logging parameters */
#define       GLOBAL_LOGGING          FALSE
#define       GLOBAL_LOG_DEV          LOG_FILE
#define       GLOBAL_LOGFILE_SIZE     1000
#define       GLOBAL_FILE_LOG_FMT     EXPANDED_FORMAT
#define       GLOBAL_CIR_BUFF_SIZE    111
#define       GLOBAL_BUFF_LOG_FMT     EXPANDED_FORMAT
#define       GLOBAL_NUM_LOG_FILES    10
#define       GLOBAL_ENABLE_SEQ       FALSE

/* Default settings for Global filtering parameters */
#define       GLOBAL_ENABLE           TRUE
#define       GLOBAL_CATEGORY_FILTER  0xFFFFFFFF
/* Global severity filter would be defaulted to 0xFC*/
#ifndef       GLOBAL_SEVERITY_FILTER_DEFAULT
#define       GLOBAL_SEVERITY_FILTER_DEFAULT  0xFC
#endif

#define       GLOBAL_SEVERITY_FILTER       GLOBAL_SEVERITY_FILTER_DEFAULT
#define       GLOBAL_SEVERITY_FILTER_ALL   0xFF

/* Default settings for NODE logging parameters */
#define       NODE_LOGGING          FALSE
#define       NODE_LOG_DEV          LOG_FILE
#define       NODE_LOGFILE_SIZE     1000
#define       NODE_FILE_LOG_FMT     EXPANDED_FORMAT
#define       NODE_CIR_BUFF_SIZE    120
#define       NODE_BUFF_LOG_FMT     EXPANDED_FORMAT

/* Default settings for NODE filtering parameters */
#define       NODE_ENABLE           TRUE
#define       NODE_CATEGORY_FILTER  0xFFFFFFFF
#define       NODE_SEVERITY_FILTER  0xFF

/* Default settings for SERVICE logging parameters */
#define       SVC_LOGGING          TRUE
#define       SVC_LOG_DEV          LOG_FILE
#define       SVC_LOGFILE_SIZE     1000
#define       SVC_FILE_LOG_FMT     EXPANDED_FORMAT
#define       SVC_CIR_BUFF_SIZE    12
#define       SVC_BUFF_LOG_FMT     EXPANDED_FORMAT

/* Default settings for NODE filtering parameters */
#define       SVC_ENABLE           TRUE
#define       SVC_CATEGORY_FILTER  0xFFFFFFFF

#ifndef       SVC_SEVERITY_FILTER_DEFAULT
/* The default svc-severity-filter has been left untouched by "build"er */
#define       SVC_SEVERITY_FILTER_DEFAULT  0xFC
#endif

#define       SVC_SEVERITY_FILTER      SVC_SEVERITY_FILTER_DEFAULT
#define       SVC_SEVERITY_FILTER_ALL  0xFF
extern uns32 gl_severity_filter;	/* To allow manipulat at init-time */

#define       CARRIAGE_RETURN      2

#define       GLOBAL_INST_ID_LEN   0x00
#define       NODE_INST_ID_LEN     0x01
#define       SVC_INST_ID_LEN      0x02

/* Circular Buffer defines */
#define       CLEAR                 0x01
#define       INUSE                 0x02
#define       FULL                  0x03

#define       NUM_BUFFS             0x06

#define       DUMP_BUFF_LOG         0x01
#define       MAX_NODE_ID_CHARS     5
#define       DTS_NUM_SVCS_TO_SUBSCRIBE  1
/* Limit for console device names */
#define       DTS_CONS_DEV_MAX      20
/* DTS ASCII SPEC table loading file define */
#define DTS_ASCII_SPEC_CONFIG_FILE PKGSYSCONFDIR "/dts_ascii_spec_config"

/* define for checking log device SET values */
#define       DTS_LOG_DEV_VAL_MAX   224

/* Versioning support - define for current DTS log version */
#define       DTS_LOG_VERSION    2
#define       DTS_MBCSV_VERSION  1
#define       DTS_MBCSV_VERSION_MIN  1

typedef struct ascii_spec_lib {
	NCS_PATRICIA_NODE libname_node;
	char lib_name[255];
	void *lib_hdl;
	uns32 use_count;
} ASCII_SPEC_LIB;

/*************************************************************************
* Datastructure(list) to hold ASCII_SPEC pointers used per DTA for a particular
* DTS_SVC_REG_TBL.
**************************************************************************/
typedef struct spec_entry {
	SYSF_ASCII_SPECS *spec_struct;	/*Ptr to ASCII_SPEC structure */
	ASCII_SPEC_LIB *lib_struct;	/*Ptr to structure for ASCII_SPEC library */
	MDS_DEST dta_addr;	/*MDS_DEST of DTA loading this ASCII_SPEC */
	char svc_name[DTSV_SVC_NAME_MAX];	/* Service name with which the                                                     DTA is getting regsitered */
	struct spec_entry *next_spec_entry;	/*Ptr to next spec_entry struct in list */
} SPEC_ENTRY;

/*************************************************************************
* Datastructure to hold values for checkpointing of last loaded ASCII_SPEC 
* table for a particular service.
**************************************************************************/
typedef struct spec_ckpt {
	char svc_name[DTSV_SVC_NAME_MAX];	/*Service name used to load ASCII_SPEC */
	uns16 version;		/* Version used to load ASCII_SPEC */
} SPEC_CKPT;

/*************************************************************************
* Datastructure to hold svc entries for a particular DTA..
**************************************************************************/
typedef struct svc_entry {
	struct dts_svc_reg_tbl *svc;	/*Pointer to service in DTA svc_list */
	struct svc_entry *next_in_dta_entry;	/*Pointer to next svc in DTA
						   svc_list */
} SVC_ENTRY;

/*************************************************************************
* Datastructure to hold dta entries for a particular DTS_SVC_REG_TBL..
**************************************************************************/
typedef struct dta_entry {
	struct dta_dest_list *dta;	/*pointer to dta in svc's v_cd_list */
	struct dta_entry *next_in_svc_entry;	/*pointer to next dta_entry in svc */
} DTA_ENTRY;

typedef struct svc_key {
	NODE_ID node;
	SS_SVC_ID ss_svc_id;
} SVC_KEY;

/*************************************************************************
* Message Sequencing structure and defines...
**************************************************************************/
#define SEQ_ARR_MAX_SIZE   1000
#define SEQ_TIME           200
#define FILE_CLOSE_TIME    6000

typedef struct seq_array {
	MS_TIME ms_time;
	DTSV_MSG *msg;
} SEQ_ARRAY;

typedef struct seq_buffer {
	uns32 num_msgs;
	SEQ_ARRAY *arr_ptr;
} SEQ_BUFFER;

/*************************************************************************
* Circular Buffer house keeping ****
**************************************************************************/
typedef struct circular_buffer_part {
	uns8 status;		/* Status : can be clear/in use/full */
	char *cir_buff_ptr;	/* Ponter to the buffer */
	uns32 num_of_elements;	/* Number of elements currently copied into buffer */
} CIRCULAR_BUFFER_PART;

typedef struct cir_buffer {
	uns8 buff_allocated;	/* TRUE : Buffer is allocated; FALSE : Buffer is freed */
	uns8 inuse;		/* TRUE : Currently in use else set to FALSE */
	char *cur_location;	/* Current pointer */
	uns8 cur_buff_num;	/* Holds the buffer number currently being used */
	uns32 cur_buff_offset;	/* Current Position offset from buffer partition base */
	uns32 part_size;	/* Partition size in KBytes */
	CIRCULAR_BUFFER_PART buff_part[NUM_BUFFS];	/* Circular Buffer partitions */
} CIR_BUFFER;

typedef struct cir_buffer_op_table {
	SVC_KEY my_key;		/* Key used for searching the element, Node ID + Service ID */
	uns8 operation;		/* Operation to be performed; currentlu only DUMP_LOG */
	uns8 op_device;		/* Device where to dump our log */
} CIR_BUFFER_OP_TABLE;
#define CIR_BUFFER_OP_TABLE_NULL    ((CIR_BUFFER_OP_TABLE *)0)

typedef struct policy {
	/* Logging policies */
	uns8 log_dev;		/* Log device: file, circular buffer or console */

	uns32 log_file_size;	/* File size to be defined in KB */
	NCS_BOOL file_log_fmt;	/* Log format: Compressed or Expanded */

	uns32 cir_buff_size;	/* Size of circular in memory buffer */
	NCS_BOOL buff_log_fmt;	/* Log format: Compressed or Expanded */

	/* Filtering policies, sent to DTA */
	NCS_BOOL enable;
	uns32 category_bit_map;	/* Category filter bit map */
	uns8 severity_bit_map;	/* Severity filter bit map */

} POLICY;
#define POLICY_NULL    ((POLICY *)0)

typedef struct svc_defaults {
	NCS_BOOL per_node_logging;	/* If TRUE- use one output device for a node, 
					   If FALSE- check for per service logging policy */
	POLICY policy;

} SVC_DEFAULTS;

typedef struct node_defaults {
	NCS_BOOL per_node_logging;	/* If TRUE- use one output device for a node, 
					   If FALSE- check for per service logging policy */
	POLICY policy;

} NODE_DEFAULTS;

typedef struct default_policy {
	SVC_DEFAULTS svc_dflt;
	NODE_DEFAULTS node_dflt;

} DEFAULT_POLICY;

/*************************************************************************
* Device Specific parametrs
*************************************************************************/
typedef struct dts_ll_file {
	char file_name[250];
	struct dts_ll_file *next;
} DTS_LL_FILE;

typedef struct dts_file_list {
	DTS_LL_FILE *head;
	DTS_LL_FILE *tail;
	uns32 num_of_files;
} DTS_FILE_LIST;

/*Smik - Added for the list of console devices */
typedef struct dts_cons_list {
	char cons_dev[DTS_CONS_DEV_MAX];	/* Console device name */
	int32 cons_fd;		/* fd for the opened console */
	uns8 cons_sev_filter;	/* severity filter for the console */
	struct dts_cons_list *next;
} DTS_CONS_LIST;

typedef struct op_device {
	/* Variables for managing the output device */
	DTS_FILE_LIST log_file_list;
	FILE *svc_fh;
	uns8 file_open;		/* IF TRUE indicates that file is open. */
	uns32 cur_file_size;	/* Current file size */
	uns8 new_file;		/* TRUE : Create new file. Set to FALSE after creating file */
	CIR_BUFFER cir_buffer;	/* Circular buffer */
	uns16 last_rec_id;

	uns8 file_log_fmt_change;
	uns8 buff_log_fmt_change;
	/* Added for console logging */
	DTS_CONS_LIST *cons_list_ptr;	/* List of consoles assoc. with this device */
	uns8 num_of_cons_conf;	/*current no. of configured console devices */
} OP_DEVICE;

/******************************************************************************
GLOBAL_POLICY : Global policy table.
******************************************************************************/
typedef struct global_policy {
   /*****************************************************************
    *Policies used if global logging (one log device for entire system) 
    *is enabled
   */
	NCS_BOOL global_logging;	/* If TRUE- use one output device for entire system, 
					   use one logging policy (global), 
					   If FALSE- check for per Node logging policy */
	POLICY g_policy;
	OP_DEVICE device;

   /*******************************************************************
   * Global logging policies - All these policies are applied on   
   *entire system when changed. These policies will not available for 
   *the configuration on per node or per service basis.
   */
	NCS_BOOL dflt_logging;	/* TRUE - Logging enabled by default, 
				   FALSE - Logging Disabled by default */
	/* Align datatype with IMM  object */
	uns32 g_num_log_files;	/*Number of old log files to be saved per service or per node */
	/*Smik - Added for console printing */
	uns8 g_num_cons_dev;	/*Max no. of console devices to be configured per service or per node */
	NCS_BOOL g_enable_seq;	/*Enable/Disable log message sequencing */
	NCS_BOOL g_close_files;	/* If set close all the opened files and the new files will 
				   be created for logging */

} GLOBAL_POLICY;
#define GLOBAL_POLICY_NULL    ((GLOBAL_POLICY *)0)

typedef struct dta_dest_list {
	NCS_QELEM qel;		/* Must be first in struct, Queue element */

	NCS_PATRICIA_NODE node;	/* Smik - added for having patricia tree */

	MDS_DEST dta_addr;	/* MDS destination address where DTA lives */

	uns32 dta_up;		/* Set to TRUE when DTA is up. Don't send any message
				   to DTA if it is down. */

	uns8 updt_req;		/* When DTA comes back again check this flag to send
				   Configuration info to DTA */

	uns32 dta_num_svcs;	/* Contains the no. of svcs associated with 
				   the DTA */

	SVC_ENTRY *svc_list;	/* List of svcs associated with this ADEST */

} DTA_DEST_LIST;

/******************************************************************************
DTS_SVC_REG_TBL : DTS service registration table.
******************************************************************************/

typedef struct dts_svc_reg_tbl {
	NCS_PATRICIA_NODE node;
	NCS_BOOL per_node_logging;	/* Set to TRUE if Node Logging is used
					   Set to FALSE if Service Logging is used */

	SVC_KEY my_key;		/* Key used for searching the element, Node ID + Service ID */

	/* Node/Svc-id in network order */
	SVC_KEY ntwk_key;

	uns32 num_svcs;		/* Counter of number of services node has */
	DTA_ENTRY *v_cd_list;	/* V-Card list, Used for configuring the 
				   services spanned across multiple V cards */

	uns32 dta_count;	/*No of dta_entries in v_cd_list */
	POLICY svc_policy;	/* Service policy table */
	OP_DEVICE device;
	/*NCSFL_ASCII_SPEC        *my_spec; */
	struct dts_svc_reg_tbl *my_node;
	SPEC_ENTRY *spec_list;	/*Ptr to linked-list of ASCII_SPECs corres. to
				   each DTA with this service */

} DTS_SVC_REG_TBL;
#define SVC_POLICY_NULL    ((DTS_SVC_REG_TBL *)0)

/* Macro to get svc_name corresponding to the svc_id */
/* versioning support : Note that now that spec ptr is removed frm svc_reg_tbl
 * we'll use the last_loaded_spec member of svc_reg_tbl to get the svc_name.
 * The above method works for versioning enabled services, for old code we'll
 * take the service name from the spec placed in 1st spec_struct in 
 * svc_reg_tbl's spec_list.
 * We expect service name to be same irrespective of change in version.
 */
#define m_DTS_GET_SVC_NAME(p, name) \
{ \
   DTS_SVC_REG_TBL *svc_reg; \
   SVC_KEY   nt_key; \
   uns32 rc = NCSCC_RC_SUCCESS; \
   \
   /* Network order key added */ \
   nt_key.node = m_NCS_OS_HTONL(p->node); \
   nt_key.ss_svc_id = m_NCS_OS_HTONL(p->ss_svc_id); \
   if((svc_reg = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&dts_cb.svc_tbl, \
             (const uns8*)&nt_key)) == NULL) \
       rc = m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_new_log_file_create: \
                      No service registration entry present"); \
   else \
   { \
      if(svc_reg->spec_list == NULL) \
         rc = m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_new_log_file_create: \
                      No Spec list present"); \
      else if(svc_reg->spec_list->spec_struct->ss_spec == NULL) \
      { \
         rc = m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_new_log_file_create: No Spec \
                    registered with service"); \
         sprintf(name,"SVC%d", svc_reg->my_key.ss_svc_id); \
      } \
      else if(svc_reg->spec_list->spec_struct->ss_spec->svc_name == NULL) \
         sprintf(name,"SVC%d", svc_reg->my_key.ss_svc_id); \
      else \
         strcpy(name, svc_reg->spec_list->spec_struct->ss_spec->svc_name); \
   } \
   rc; \
}

typedef struct dts_cb {
	/* Configuration Objects */
	NCS_BOOL dts_enbl;	/* RW ENABLE|DISABLE DTS services           */

	DEFAULT_POLICY dflt_plcy;
	GLOBAL_POLICY g_policy;

	SEQ_BUFFER s_buffer;

	NCS_LOCK lock;

	/* run-time learned values */
	tmr_t tmr;
	MDS_VDEST_ID vaddr;

	char log_path[200];

	/* Some storage used for strings */
	char log_str[SYSF_FL_LOG_BUFF_SIZE];
	char t_log_str[SYSF_FL_LOG_BUFF_SIZE];
	char cb_log_str[SYSF_FL_LOG_BUFF_SIZE];

	MDS_HDL mds_hdl;

	/* AMF variables */
	NCS_BOOL amf_init;
	/*uns32              amf_hdl; */
	SaAmfHandleT amf_hdl;
	SaNameT comp_name;
	SaAmfHAStateT ha_state;
	/* Smik - Add new member variables */
	SaAmfHAStateT ha_state_other;
	MDS_HDL vaddr_pwe_hdl;	/* The pwe handle returned when
				 * vdest address is created. */
	MDS_HDL vaddr_hdl;	/* The handle returned by mds
				 * for vaddr creation */

	SaSelectionObjectT dts_sel_obj;
	SaSelectionObjectT dts_amf_sel_obj;
	uns32 csi_set;
	uns16 dts_mbcsv_version;
	MDS_SVC_PVT_SUB_PART_VER dts_mds_version;
	SaAmfHealthcheckKeyT health_chk_key;
	NCS_BOOL healthCheckStarted;

	SaAmfHealthcheckInvocationT invocationType;

	/* Health Check Failure Recommended Recovery, 
	 * How do we get this information 
	 */
	SaAmfRecommendedRecoveryT recommendedRecovery;

	/* 
	 * MBCSv related variables.
	 */
	uns32 mbcsv_hdl;
	uns32 ckpt_hdl;
	SaSelectionObjectT mbcsv_sel_obj;
	SaVersionT dtsv_ver;
	NCS_BOOL in_sync;	/* Standby is in sync with Active */
	NCS_BOOL cold_sync_in_progress;	/* TRUE indicates that cold sync is on */
	DTSV_CKPT_MSG_REO_TYPE cold_sync_done;
	DTSV_ASYNC_UPDT_CNT async_updt_cnt;

	EDU_HDL edu_hdl;	/* EDU handle */

	/* Create time constants */
	NCS_PATRICIA_TREE svc_tbl;
	/* Smik - Adding patricia tree for DTA_DEST_LIST */
	NCS_PATRICIA_TREE dta_list;

	NCS_BOOL created;

#if (DTS_SIM_TEST_ENV == 1)
	NCS_BOOL is_test;
#endif

	uns8 hmpool_id;

	char *cons_dev;		/* Added 2 variables for console logging */
	int32 cons_fd;

	/*Patricia trees introduced to keep track of ASCII Spec tables registered */
	NCS_PATRICIA_TREE libname_asciispec_tree;
	NCS_PATRICIA_TREE svcid_asciispec_tree;

	/* New selection objects */
	NCS_SEL_OBJ_SET readfds;
	NCS_SEL_OBJ numfds;
	NCS_SEL_OBJ sighdlr_sel_obj;

	/* Added the amf_invocation hdl to be able send SaAmfResponse for 
	 * Quiesced complete ack frm dts_do_evt */
	SaInvocationT csi_cb_invocation;

	uns8 cli_bit_map;       /*Bit map to reflect change made in global
				   policy or node policy through CLI */
	MDS_DEST svc_rmv_mds_dest;	/*MDS_DEST corrsp to DTA from which 
					   service unreg is received */

	SPEC_CKPT last_spec_loaded;	/* Contains data to ckpt the details for last
					   spec loaded for this service.
					   Needed only for checkpointing. */
	SaImmOiHandleT immOiHandle;     
	SaSelectionObjectT imm_sel_obj;
#if (DTS_FLOW == 1)
	uns32 msg_count;
#endif
	NCS_BOOL imm_init_done;

} DTS_CB;

DTS_CB dts_cb;

/* Macro to get file name, can be used during logging */
#define m_DTS_LOG_FILE_NAME(dev)  dev->log_file_list.tail->file_name

/* Macro to get number of files */
#define m_DTS_NUM_LOG_FILES(dev)  dev->log_file_list.num_of_files

/* Macro to copy version fields */
#define m_DTS_VER_COPY(src, dest) \
{ \
   dest.releaseCode = src.releaseCode; \
   dest.majorVersion = src.majorVersion; \
   dest.minorVersion = src.minorVersion; \
}

/* Macro to add new file in the list */
#define m_DTS_ADD_NEW_FILE(dev) \
{  \
   DTS_FILE_LIST *list = &dev->log_file_list; \
   DTS_LL_FILE   *n_file = NULL; \
   n_file = m_MMGR_ALLOC_DTS_LOG_FILE; \
   \
   if (!n_file) \
      return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, \
          "Failed to allocate memory for log file list"); \
   memset(n_file, '\0', sizeof(DTS_LL_FILE)); \
   strcat(n_file->file_name, ""); \
   n_file->next = NULL; \
   \
   if (!list->head) \
      list->head = n_file; \
   else \
      list->tail->next = n_file; \
   \
   list->tail = n_file; \
   list->num_of_files++; \
}

/* Removes the oldest file */
#define m_DTS_RMV_FILE(dev) \
{  \
   DTS_FILE_LIST *list = &dev->log_file_list; \
   DTS_LL_FILE   *rm_file = NULL; \
   \
   rm_file = list->head; \
   list->head = list->head->next; \
   if(remove(rm_file->file_name) != 0) \
      perror("Failed to remove old log files"); \
   m_MMGR_FREE_DTS_LOG_FILE(rm_file); \
   if (!list->head) \
      list->tail = NULL; \
   list->num_of_files--; \
}

/* Frees the DTS_FILE_LIST datastructure associated with the device*/
#define m_DTS_FREE_FILE_LIST(dev) \
{ \
   DTS_FILE_LIST *list = &dev->log_file_list; \
   DTS_LL_FILE   *rm_file = NULL; \
   \
   while(list->head != NULL) \
   { \
      rm_file = list->head; \
      list->head = list->head->next; \
      m_MMGR_FREE_DTS_LOG_FILE(rm_file); \
      if (!list->head) \
         list->tail = NULL; \
      list->num_of_files--; \
   } \
}

/* Macro to add console to global/node/service device*/
#define m_DTS_ADD_CONS(cb, dev, str, bit_map) \
{ \
  DTS_CONS_LIST *tmp=dev->cons_list_ptr, *prev=dev->cons_list_ptr; \
  int32 fd=-1; \
  while(tmp != NULL) \
  { \
     if(strcmp(str, tmp->cons_dev) == 0) \
        break; \
     \
     prev = tmp; \
     tmp = tmp->next; \
  } \
  if(tmp == NULL) \
  { \
     fd = dts_open_conf_cons(cb, O_RDWR|O_NOCTTY, str); \
     if(fd < 0) \
        return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "Unable to open specified console device"); \
     tmp = m_MMGR_ALLOC_DTS_CONS_DEV; \
     if(tmp == NULL) \
        return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "Failed to allocate memory"); \
     memset(tmp, '\0', sizeof(DTS_CONS_LIST)); \
     strcpy(tmp->cons_dev, str); \
     tmp->cons_fd = fd; \
     if(prev != NULL) \
        prev->next = tmp; \
     else \
        dev->cons_list_ptr = tmp; \
     dev->num_of_cons_conf++; \
  } \
  tmp->cons_sev_filter = bit_map; \
}

/* Macro to remove a console frm global/node/service device */
#define m_DTS_RMV_CONS(cb, dev, str) \
{ \
  DTS_CONS_LIST *next_ptr=NULL, *rmv_ptr=dev->cons_list_ptr, *tmp=NULL; \
  if(dev->cons_list_ptr == NULL) \
     return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "Console device list already empty"); \
  while(rmv_ptr != NULL) \
  { \
     if(strcmp(rmv_ptr->cons_dev, str) == 0) \
     { \
        next_ptr = rmv_ptr->next; \
        if(rmv_ptr->cons_fd > 0) \
           close(rmv_ptr->cons_fd); \
        m_MMGR_FREE_DTS_CONS_DEV(rmv_ptr); \
        rmv_ptr = NULL; \
        if(tmp == NULL) \
           dev->cons_list_ptr = next_ptr; \
        else \
           tmp->next = next_ptr; \
        dev->num_of_cons_conf--; \
        return NCSCC_RC_SUCCESS; \
    } \
    tmp = rmv_ptr; \
    rmv_ptr = rmv_ptr->next; \
  } \
  return NCSCC_RC_FAILURE; \
} \

/* Macro to remove all DTS_CONS_LIST entries for a particular device */
#define m_DTS_RMV_ALL_CONS(dev) \
{ \
   DTS_CONS_LIST *ptr=NULL; \
   ptr = dev->cons_list_ptr; \
   while(ptr != NULL) \
   { \
      dev->cons_list_ptr = ptr->next; \
      if(ptr->cons_fd > 0) \
         close(ptr->cons_fd); \
      m_MMGR_FREE_DTS_CONS_DEV(ptr); \
      ptr = dev->cons_list_ptr; \
      dev->num_of_cons_conf--; \
   } \
} \


/* Removes the first entry in the datastructure, doesn't delete files */
#define m_DTS_RMV_LL_ENTRY(dev) \
{ \
   DTS_FILE_LIST *list = &dev->log_file_list; \
   DTS_LL_FILE   *rm_file=NULL; \
   \
   rm_file = list->head; \
   if(rm_file == NULL) \
   { \
      m_LOG_DTS_API(DTS_LOG_DEL_FAIL); \
   } \
   else \
   { \
      list->head = list->head->next; \
      m_MMGR_FREE_DTS_LOG_FILE(rm_file); \
      if (!list->head) \
         list->tail = NULL; \
      list->num_of_files--; \
   } \
}

/* macro for intializing buffer/log files when stby becomes active */
/* Irrespective of logging device, initialize both buffers & file for use*/
#define m_DTS_STBY_INIT(policy, device, file_type) \
{ \
   uns32 curr_file_size; \
   struct stat statinfo; \
   /* Initialize circular buffers */ \
   if((policy->log_dev & CIRCULAR_BUFFER) == CIRCULAR_BUFFER) \
   { \
      if (dts_circular_buffer_alloc(&device->cir_buffer, \
         policy->cir_buff_size) == NCSCC_RC_SUCCESS) \
         m_LOG_DTS_CBOP(DTS_CBOP_ALLOCATED, 0 , 0); \
      else \
         dts_cir_buff_set_default(&device->cir_buffer); \
   } \
   if(((policy->log_dev & LOG_FILE) == LOG_FILE) && (device->file_open == TRUE))\
   { \
      curr_file_size = dts_chk_file_size(m_DTS_LOG_FILE_NAME(device)); \
      if((device->new_file == FALSE) && \
         (curr_file_size <  (policy->log_file_size * 1024))) \
      { \
         if(stat(m_DTS_LOG_FILE_NAME(device), &statinfo) == -1) \
         { \
            perror("stat"); \
            m_DTS_DBG_SINK(NCSCC_RC_FAILURE, \
             "dts_stby_svc_initialize: Failed to open existing log file"); \
            m_DTS_LOGFILE_CREATE(device, file_type); \
         } \
         else \
         { \
            device->svc_fh = sysf_fopen(m_DTS_LOG_FILE_NAME(device), "a+"); \
         } \
         device->cur_file_size = curr_file_size; \
         device->file_open = TRUE; \
      } \
      else \
         m_DTS_LOGFILE_CREATE(device, file_type); \
   } \
}

#define m_DTS_LOGFILE_CREATE(device, file_type) \
{ \
   DTS_LOG_CKPT_DATA data; \
   memset(&data, '\0', sizeof(DTS_LOG_CKPT_DATA)); \
   m_DTS_ADD_NEW_FILE(device); \
   if (0 == (device->cur_file_size = \
      dts_new_log_file_create(m_DTS_LOG_FILE_NAME(device), \
               &key, file_type))) \
      return  m_DTS_DBG_SINK(NCSCC_RC_FAILURE, \
         "dts_stby_svc_initialize: Failed to create new service log file"); \
   device->new_file = FALSE; \
   strcpy(data.file_name, m_DTS_LOG_FILE_NAME(device)); \
   data.key = key; \
   data.new_file = device->new_file; \
   m_DTSV_SEND_CKPT_UPDT_ASYNC(&dts_cb, NCS_MBCSV_ACT_ADD, \
             (MBCSV_REO_HDL)(long)&data, DTSV_CKPT_DTS_LOG_FILE_CONFIG); \
   if (NULL == (device->svc_fh = \
          sysf_fopen(m_DTS_LOG_FILE_NAME(device), "a+"))) \
      return  m_DTS_DBG_SINK(NCSCC_RC_FAILURE, \
         "dts_stby_svc_initialize: Failed to open service log file"); \
   device->cur_file_size = 0; \
   device->file_open = TRUE; \
   if(m_DTS_NUM_LOG_FILES(device) > dts_cb.g_policy.g_num_log_files) \
   { \
      m_DTS_RMV_FILE(device); \
      m_DTSV_SEND_CKPT_UPDT_ASYNC(&dts_cb, NCS_MBCSV_ACT_RMV, \
                 (MBCSV_REO_HDL)(long)&data, DTSV_CKPT_DTS_LOG_FILE_CONFIG); \
   } \
}

/* Macros for common routine in CLI filters config */
#define m_DTS_CLI_CAT_FLTR(get_val, bit_set, intval, bit_map) \
{ \
   uns32 val, set_val=0x80000000; \
   if(bit_set) \
   { \
      val =  set_val>>intval; \
      bit_map = val|get_val; \
   } \
   else \
   { \
      val =  ~(set_val>>intval); \
      bit_map = val&get_val; \
   } \
} \

#define m_DTS_CLI_SEV_FLTR(pArgv, bit_map) \
{ \
   if(strcmp(pArgv->cmd.strval, "emergency") == 0) \
      bit_map = 0x80; \
   else if(strcmp(pArgv->cmd.strval, "alert") == 0) \
      bit_map = 0xC0; \
   else if(strcmp(pArgv->cmd.strval, "critical") == 0) \
      bit_map = 0xE0; \
   else if(strcmp(pArgv->cmd.strval, "error") == 0) \
      bit_map = 0xF0; \
   else if(strcmp(pArgv->cmd.strval, "warning") == 0) \
      bit_map = 0xF8; \
   else if(strcmp(pArgv->cmd.strval, "notice") == 0) \
      bit_map = 0xFC; \
   else if(strcmp(pArgv->cmd.strval, "info") == 0) \
      bit_map = 0xFE; \
   else if(strcmp(pArgv->cmd.strval, "debug") == 0) \
      bit_map = 0xFF; \
   else if(strcmp(pArgv->cmd.strval, "none") == 0) \
      bit_map = 0x00; \
} \

#define m_DTS_CLI_CONF_CONS(pArgv, bit_map) \
{ \
   if(strcmp(pArgv->cmd.strval, "emergency") == 0) \
      bit_map = 0x80; \
   else if(strcmp(pArgv->cmd.strval, "alert") == 0) \
      bit_map = 0x40; \
   else if(strcmp(pArgv->cmd.strval, "critical") == 0) \
      bit_map = 0x20; \
   else if(strcmp(pArgv->cmd.strval, "error") == 0) \
      bit_map = 0x10; \
   else if(strcmp(pArgv->cmd.strval, "warning") == 0) \
      bit_map = 0x08; \
   else if(strcmp(pArgv->cmd.strval, "notice") == 0) \
      bit_map = 0x04; \
   else if(strcmp(pArgv->cmd.strval, "all") == 0) \
      bit_map = 0xFC; \
} \

/* Macro for console printing */
#define m_DTS_CONS_PRINT(cons_dev, msg, str, len) \
{ \
   if(cons_dev != NULL) \
   { \
      while(cons_dev != NULL) \
      { \
         if(((msg->data.data.msg.log_msg.hdr.severity)&(cons_dev->cons_sev_filter)) && (cons_dev->cons_fd > 0)) \
         { \
            if(-1 == write(cons_dev->cons_fd, str, len)) \
               perror("m_DTS_CONS_PRINT"); \
         } \
         cons_dev = cons_dev->next; \
      } \
   } \
   else if(dts_cb.cons_fd >= 0) \
   { \
      if(-1 == write(dts_cb.cons_fd, str, len)) \
               perror("m_DTS_CONS_PRINT"); \
   } \
} \

/* Define the Ranks for the persistent IMM Objects */
typedef enum {
	DTSV_TBL_RANK_MIN = 1,
	DTSV_TBL_RANK_GLOBAL = DTSV_TBL_RANK_MIN,
	DTSV_TBL_RANK_NODE,
	DTSV_TBL_RANK_SVC,
	DTSV_TBL_RANK_MAX
} DTSV_TBL_RANK;

/************************************************************************

  F L S   P r i v a t e   F u n c t i o n   P r o t o t y p e s
  
*************************************************************************/
/************************************************************************
DTS tasking loop function
*************************************************************************/

void dts_do_evts(SYSF_MBX *mbx);
uns32 dts_do_evt(DTSV_MSG *msg);

/************************************************************************
DTSv Message Processing functions
************************************************************************/
uns32 dts_handle_signal(void);
uns32 dts_handle_dta_event(DTSV_MSG *msg);
uns32 dts_handle_immnd_event(DTSV_MSG *msg);
uns32 dts_register_service(DTSV_MSG *msg);
uns32 dts_unregister_service(DTSV_MSG *msg);
uns32 dts_log_data(DTSV_MSG *msg);

uns32 dts_log_my_data(DTSV_MSG *msg);
uns32 dts_close_opened_files(void);
uns32 dts_close_files_quiesced(void);

/************************************************************************
DTSv Message logging policy functions
************************************************************************/
void dts_default_policy_set(DEFAULT_POLICY *dflt_plcy);
void dts_global_policy_set(GLOBAL_POLICY *gpolicy);
void dts_default_svc_policy_set(DTS_SVC_REG_TBL *service);
void dts_default_node_policy_set(POLICY *policy, OP_DEVICE *device, uns32 node_id);

uns32 dts_new_log_file_create(char *file, SVC_KEY *svc, uns8 file_type);
uns32 dtsv_log_msg(DTSV_MSG *msg, POLICY *policy, OP_DEVICE *device, uns8 file_type, NCSFL_ASCII_SPEC *spec);
uns32
dts_create_new_pat_entry(DTS_CB *inst, DTS_SVC_REG_TBL **node, uns32 node_id, SS_SVC_ID svc_id, uns8 log_level);

/************************************************************************
DTSv Circular buffer functions.
************************************************************************/
uns32 dts_circular_buffer_alloc(CIR_BUFFER *cir_buff, uns32 buffer_size);
uns32 dts_circular_buffer_free(CIR_BUFFER *cir_buff);
uns32 dts_circular_buffer_clear(CIR_BUFFER *cir_buff);
uns32 dts_cir_buff_set_default(CIR_BUFFER *cir_buff);
uns32 dts_dump_to_cir_buffer(CIR_BUFFER *buffer, char *str);
uns32 dts_dump_log_to_op_device(CIR_BUFFER *cir_buff, uns8 device, char *file);
uns32 dts_buff_size_increased(CIR_BUFFER *cir_buff, uns32 new_size);
uns32 dts_buff_size_decreased(CIR_BUFFER *cir_buff, uns32 new_size);
uns32 dts_dump_buffer_to_buffer(CIR_BUFFER *src_cir_buff, CIR_BUFFER *dst_cir_buff, uns32 number);

/************************************************************************
DTSv Message Sequencing functions.
************************************************************************/
void dts_sort_msgs(SEQ_ARRAY *msg_seq, uns32 array_size);
void dts_sift_down(SEQ_ARRAY *msg_seq, uns32 root, uns32 bottom);
void dts_cpy_seq_struct(SEQ_ARRAY *dst, SEQ_ARRAY *src);
uns32 dts_queue_seq_msg(DTS_CB *inst, DTSV_MSG *msg);
uns32 dts_dump_seq_msg(DTS_CB *inst, NCS_BOOL all);
uns32 dts_enable_sequencing(DTS_CB *inst);
uns32 dts_disable_sequencing(DTS_CB *inst);
uns32 dts_free_msg_content(NCSFL_NORMAL *msg);


/************************************************************************
Basic DTS Layer Management Service entry points off std LM API
*************************************************************************/

uns32 dts_svc_create(DTS_CREATE *create);
uns32 dts_svc_destroy(DTS_DESTROY *destroy);

/*****************************************************************************
* Patricia deletion functions
******************************************************************************/
uns32 dtsv_clear_asciispec_tree(DTS_CB *cb);
uns32 dtsv_clear_libname_tree(DTS_CB *cb);

/*****************************************************************************
* Flex log log
******************************************************************************/
void log_my_msg(NCSFL_NORMAL *lmsg, DTS_CB *inst);

#define m_NCS_DTS_LOG(m, i)       log_my_msg(m, i)

/*****************************************************************************
* DTS configured console opening 
******************************************************************************/
int32 dts_open_conf_cons(DTS_CB *cb, uns32 mode, char *str);


/************************************************************************
DTS fail over related 
*************************************************************************/
uns32 dts_handle_fail_over(void);
uns32 dts_chk_file_size(char *file);
uns32 dts_fail_over_enc_msg(DTSV_MSG *mm);
uns32 dts_stby_update_dta_config(void);

/************************************************************************
Function to clear svc registration and associated datastructures 
*************************************************************************/
void dtsv_clear_registration_table(DTS_CB *inst);

/************************************************************************
MDS bindary stuff for DTS
*************************************************************************/
#define DTS_MDS_SUB_PART_VERSION   2

/* Definitions for min/max expected message-format-version from DTA to DTS */
#define DTS_MDS_MIN_MSG_FMAT_VER_SUPPORT 1
#define DTS_MDS_MAX_MSG_FMAT_VER_SUPPORT 2

uns32 dts_mds_reg(DTS_CB *cb);

/*uns32 dts_mds_change_role(V_DEST_RL role);*/
uns32 dts_mds_change_role(DTS_CB *cb, SaAmfHAStateT role);

void dts_mds_unreg(DTS_CB *cb, NCS_BOOL un_install);

uns32 dts_mds_send_msg(DTSV_MSG *msg, MDS_DEST dta_dest, MDS_CLIENT_HDL mds_hdl);

uns32 dts_mds_callback(NCSMDS_CALLBACK_INFO *cbinfo);

uns32 dts_mds_rcv(NCSMDS_CALLBACK_INFO *cbinfo);

void dts_mds_evt(MDS_CALLBACK_SVC_EVENT_INFO svc_info, MDS_CLIENT_HDL yr_svc_hdl);

void dts_set_dta_up_down(NODE_ID node_id, MDS_DEST adest, NCS_BOOL up_down);

uns32 dts_mds_enc(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg,
			   SS_SVC_ID to_svc, NCS_UBAID *uba,
			   MDS_SVC_PVT_SUB_PART_VER remote_ver, MDS_CLIENT_MSG_FORMAT_VER *msg_fmat_ver);

uns32 dts_mds_dec(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT *msg,
			   SS_SVC_ID to_svc, NCS_UBAID *uba, MDS_CLIENT_MSG_FORMAT_VER msg_fmat_ver);

uns32 dts_mds_cpy(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg,
			   SS_SVC_ID to_svc, NCSCONTEXT *cpy,
			   NCS_BOOL last, MDS_SVC_PVT_SUB_PART_VER remote_ver, MDS_CLIENT_MSG_FORMAT_VER *msg_fmat_ver);

uns32 decode_ip_address(NCS_UBAID *uba, NCS_IP_ADDR *ipa);

uns32 dts_log_str_decode(NCS_UBAID *uba, char **str);

uns32 dts_log_msg_decode(NCSFL_NORMAL *logmsg, NCS_UBAID *uba);

NCS_BOOL dts_find_reg(void *key, void *qelem);

uns32 dts_send_filter_config_msg(DTS_CB *inst, DTS_SVC_REG_TBL *svc, DTA_DEST_LIST *dta);

NCSCONTEXT dts_get_next_node_entry(NCS_PATRICIA_TREE *node, SVC_KEY *key);
NCSCONTEXT dts_get_next_svc_entry(NCS_PATRICIA_TREE *node, SVC_KEY *key);

void dts_log_policy_change(DTS_SVC_REG_TBL *node, POLICY *old_plcy, POLICY *new_plcy);

void dts_filter_policy_change(DTS_SVC_REG_TBL *node, POLICY *old_plcy, POLICY *new_plcy);

uns32 dtsv_svc_filtering_policy_change(DTS_CB *inst,
						DTS_SVC_REG_TBL *service,
						unsigned int param_id, uns32 node_id, SS_SVC_ID svc_id);

uns32 dts_log_device_set(POLICY *policy, OP_DEVICE *device, uns8 old_value);

uns32 dts_buff_size_set(POLICY *policy, OP_DEVICE *device, uns32 old_value);

/************************************************************************
 Funtions to add/delete elements from the linked-list of services for DTA
************************************************************************/
void dts_add_svc_to_dta(DTA_DEST_LIST *dta, DTS_SVC_REG_TBL *svc);
NCSCONTEXT dts_del_svc_frm_dta(DTA_DEST_LIST *dta, DTS_SVC_REG_TBL *svc);

/************************************************************************
 Funtions to add/delete/find elements from the linked-list of dta for svc_reg
************************************************************************/
void dts_enqueue_dta(DTS_SVC_REG_TBL *svc, DTA_DEST_LIST *dta);
NCSCONTEXT dts_dequeue_dta(DTS_SVC_REG_TBL *svc, DTA_DEST_LIST *dta);
NCSCONTEXT dts_find_dta(DTS_SVC_REG_TBL *svc, MDS_DEST *dta_key);

/************************************************************************
 Funtions to delete/find elements from the spec list of svc_reg_tbl
************************************************************************/
NCSCONTEXT dts_find_spec(DTS_SVC_REG_TBL *svc, MDS_DEST *dta_key);
NCSCONTEXT dts_find_spec_entry(DTS_SVC_REG_TBL *svc, MDS_DEST *dta_key);
uns32 dts_del_spec_frm_svc(DTS_SVC_REG_TBL *svc, MDS_DEST dta_addr, SPEC_CKPT *ver);

/************************************************************************
 Funtions to print the Patricia Tree contents 
************************************************************************/
void dts_print_svc_reg_pat(DTS_CB *cb, FILE *fp);
void dts_print_cb(DTS_CB *cb, FILE *fp);
void dts_print_cb_dbg(void);
void dts_print_dta_dest_pat(void);
void dts_print_reg_tbl_dbg(void);
void dts_print_next_svc_dta_list(NCS_PATRICIA_TREE *node, SVC_KEY *key);
void dts_printall_svc_per_node(uns32 node_id);
uns32 dts_print_current_config(DTS_CB *cb);

/************************************************************************
* DTS ascii_spec reload 
************************************************************************/
uns32 dts_ascii_spec_reload(DTS_CB *cb);

/************************************************************************
* Defines used for converting the log data into strings 
************************************************************************/
#define MAX_OCT_LEN            255
#define m_S_SET(i)             (uns16)((i&0xffff0000)>>16)
#define m_S_STR(i)             (uns16)((i)&0x0000ffff)

#define m_NCSFL_MAKE_STR(s,p) (s[m_S_SET(p)].set_vals[m_S_STR(p)].str_val)

#define m_GET_VALUE_FROM_UBA_TO_PRINT(tstr,i,ch,t,uba,sp,set) ((ch=='\0')?NULL:(ch=='T')?dts_get_time_value(i,t):(ch=='L')?dts_get_long_value(i,uba):(ch=='M')?dts_get_u16_value(i,uba):(ch=='S')?dts_get_uns8_value(i,uba):(ch=='I')?dts_idx_to_str(i,uba,sp,set):((ch=='D')||(ch=='P')||(ch=='A')||(ch=='C'))?dts_get_str_value(tstr,i,ch,uba):NULL)

#define m_DTS_VALIDATE_INDX( spec, indx) \
{ \
    if(m_S_SET(indx) >= spec->str_set_cnt) \
{ \
        return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "String set ID exceeds the max string set."); \
} \
    \
    if(m_S_STR(indx) >= spec->str_set[m_S_SET(indx)].set_cnt) \
{ \
        return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "String index ID exceeds the maximum number of string ID count"); \
} \
}

#define m_NCSFL_MAKE_STR_FROM_PDU(str, len, mem) \
{ \
   uns16 index; \
   uns16 mlen; \
   char  buf[10]; \
   \
   *str = '\0'; \
   /* check the value of the length field - should be less than 256. \
    * If it's more than that, truncate the size of the memory dump \
    * Truncation part is removed - Parag. \
    */  \
   mlen = len; \
   \
   for (index = 1; index <= mlen; index++)\
   {\
      sprintf(buf, " %02X", (mem)[index - 1]); \
      strcat((str), buf);\
      /* Print max of 16 bytes in a single line */ \
      if( index >= 16 && !(index % 16)) \
         strcat((str), "\n"); \
   }\
   strcat((str), "\n"); \
}

#if (NCS_IPV6 == 1)
#define m_NCSFL_MAKE_STR_FRM_IPADDR(ipa, t_str) \
{ \
  uns8  *ptr; \
  if (ipa.type == NCS_IP_ADDR_TYPE_IPV4) \
  { \
       ptr = (uns8 *)&ipa.info.v4;  \
       sprintf(t_str,"%d.%d.%d.%d", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3)); \
  } \
  else if (ipa.type == NCS_IP_ADDR_TYPE_IPV6)\
  {     int i;\
       ptr = (uns8 *)&ipa.info.v6;  \
       for (i=0; i<7; i++) \
           sprintf(&t_str[i*5],"%04x:", ntohs(ipa.info.v6.ipv6.ipv6_addr16[i]));\
       sprintf(&t_str[i*5],"%04x", ntohs(ipa.info.v6.ipv6.ipv6_addr16[i]));     \
  }\
  else \
  {\
     sprintf(t_str,"NONE!");\
  }\
}
#else
#define m_NCSFL_MAKE_STR_FRM_IPADDR(ipa, t_str) \
{ \
  uns8  *ptr; \
  if (ipa.type == NCS_IP_ADDR_TYPE_IPV4) \
  { \
       ptr = (uns8 *)&ipa.info.v4;  \
       sprintf(t_str,"%d.%d.%d.%d", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3)); \
  } \
  else \
  {\
     sprintf(t_str,"NONE!");\
  }\
}
#endif

#define m_NCSFL_MAKE_STR_FROM_MEM(str, addr, len, mem) \
{ \
   char *str1; \
   sprintf((str), "0x%08X\n", (uns32)(addr));\
   /* Now get to the end of the string */ \
   str1 = (str) + strlen((str)); \
   /* Fill in the memory dump now */ \
   m_NCSFL_MAKE_STR_FROM_PDU ((str1), (len), (mem)); \
}

/* Macro for printing 64bit values */
#define m_NCSFL_MAKE_STR_FROM_MEM_64(str, addr, len, mem) \
{ \
   char *str1; \
   sprintf((str), "0x%08" PRIX64 "\n", (uns64)(addr));\
   /* Now get to the end of the string */ \
   str1 = (str) + strlen((str)); \
   /* Fill in the memory dump now */ \
   m_NCSFL_MAKE_STR_FROM_PDU ((str1), (len), (mem)); \
}

/************************************************************************

  F L E X   L O G   S E R V I C E    L o c k s
  
*************************************************************************/

#if (NCSDTS_USE_LOCK_TYPE == DTS_NO_LOCKS)	/* NO Locks */

#define m_DTS_LK_CREATE(lk)
#define m_DTS_LK_INIT
#define m_DTS_LK(lk)
#define m_DTS_UNLK(lk)
#define m_DTS_LK_DLT(lk)
#elif (NCSDTS_USE_LOCK_TYPE == DTS_TASK_LOCKS)	/* Task Locks */

#define m_DTS_LK_CREATE(lk)
#define m_DTS_LK_INIT            m_INIT_CRITICAL
#define m_DTS_LK(lk)             m_START_CRITICAL
#define m_DTS_UNLK(lk)           m_END_CRITICAL
#define m_DTS_LK_DLT(lk)
#elif (NCSDTS_USE_LOCK_TYPE == DTS_OBJ_LOCKS)	/* Object Locks */

#define m_DTS_LK_CREATE(lk)      m_NCS_LOCK_INIT_V2(lk,NCS_SERVICE_ID_DTSV, \
DTS_LOCK_ID)
#define m_DTS_LK_INIT
#define m_DTS_LK(lk)             m_NCS_LOCK_V2(lk,   NCS_LOCK_WRITE, \
NCS_SERVICE_ID_DTSV, DTS_LOCK_ID)
#define m_DTS_UNLK(lk)           m_NCS_UNLOCK_V2(lk, NCS_LOCK_WRITE, \
NCS_SERVICE_ID_DTSV, DTS_LOCK_ID)
#define m_DTS_LK_DLT(lk)         m_NCS_LOCK_DESTROY_V2(lk, NCS_SERVICE_ID_DTSV, \
DTS_LOCK_ID)
#endif

/* Offset to calculate start of DTA_DEST_LIST after get frm patricia tree */
#define DTA_DEST_LIST_NODE_ADDR &((DTA_DEST_LIST*)0)->node
#define DTA_DEST_LIST_QEL_ADDR  &((DTA_DEST_LIST*)0)->qel
#define DTA_DEST_LIST_OFFSET  ((long)DTA_DEST_LIST_NODE_ADDR - (long)DTA_DEST_LIST_QEL_ADDR)

#endif   /* DTS_PVT_H */
