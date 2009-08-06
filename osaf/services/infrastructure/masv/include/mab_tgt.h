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

*******************************************************************************/

/*
 * Module Inclusion Control...
 */
#ifndef MAB_TGT_H
#define MAB_TGT_H

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  MAB macros
  ***********

  These macros are used to send and receive data related to redundant operations. 
  The macros are also used to inform the target environment regarding important
  events that may have occured related to redundancy. 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/******************************************************************************
 *
 * MAS filter compare macro for MAB filters.
 *
 *****************************************************************************/

#define m_MAB_FLTR_CMP(res,src,dst,size) \
  { \
    uns32 i; \
    res = 0; \
    for(i = 0;i < size;i++) \
    { \
      if(*(src + i) == *(dst + i)) \
        continue; \
      else if(*(src + i) > *(dst + i)) \
      { \
        res = 1; \
        break; \
      } \
      else \
      { \
        res = -1; \
        break; \
      } \
    } \
  }

/*
 * m_MAB_DBG_SINK
 *
 * If MAB fails an if-conditional or other test that we would not expect
 * under normal conditions, it will call this macro. 
 * 
 * If MAB_DEBUG == 1, fall into the sink function. A customer can put
 * a breakpoint there or leave the CONSOLE print statement, etc.
 *
 * If MAB_DEBUG == 0, just echo the value passed. It is typically a 
 * return code or a NULL.
 *
 * MAB_DEBUG can be enabled in mab_opt,h
 */

#if (MAB_DEBUG == 1)

EXTERN_C MABCOM_API uns32 mab_dbg_sink(uns32, char *, uns32);

/* m_MAB_DBG_VOID() used to keep compiler happy @ void return functions */

#define m_MAB_DBG_SINK(r)  mab_dbg_sink(__LINE__,__FILE__,(uns32)r)
#define m_MAB_DBG_SINK_VOID(r)  mab_dbg_sink(__LINE__,__FILE__,(uns32)r)
#define m_MAB_DBG_VOID     mab_dbg_sink(__LINE__,__FILE__,1)
#else

#define m_MAB_DBG_SINK(r)  r
#define m_MAB_DBG_SINK_VOID(r)
#define m_MAB_DBG_VOID
#endif

/*
 * m_MAB_DBG_TRACE
 *
 * This macro is invoked at segnificant MAB function entry and exit
 * points. It may facilitate the understanding in terms of how MAB 
 * works as well as a means to trace where errors may be occuring in code.
 *
 * If MAB_TRACE == 1, m_MAB_DBG_TRACE maps to (trace) functionality.
 *
 * If MAB_TRACE == 0, m_MAB_DBG_TRACE is benign and removed from executable.
 *
 * MAB_TRACE can be enabled in mab_opt,h
 */

#if (MAB_TRACE == 1)

#define m_MAB_DBG_TRACE(t) printf(t)
#else

#define m_MAB_DBG_TRACE(t)
#endif

/*
 * m_MAB_DBG_TRACE2_XXXX
 *
 * These macro are invoked at segnificant MAS processing points like
 * filter/table record registration/unregistration and MAS instance entry. 
 * It may facilitate the understanding in terms of how MAB 
 * works as well as a means to trace where errors may be occuring in code.
 *
 * If MAB_TRACE2 == 1, m_MAB_DBG_TRACE2_XXXX maps to (trace) functionality.
 *
 * If MAB_TRACE2 == 0, m_MAB_DBG_TRACE2_XXXX is benign and removed 
 * from executable.
 *
 * MAB_TRACE2 can be enabled in mab_opt,h
 */

#if (MAB_TRACE2 == 1)

typedef enum mab_fmode {
	MFM_CREATE,
	MFM_MODIFY,
	MFM_DESTROY
} MAB_FMODE;

/* forward declarations to make the compiler happy */
struct mas_tbl;
struct mas_fltr;

EXTERN_C MABCOM_API void mab_dbg_dump_fltr_op(struct mas_tbl *inst,
					      struct mas_fltr *fltr, uns32 tbl_id, MAB_FMODE mode);

#if (NCS_RMS == 1)

#define m_MAB_DBG_TRACE2_MAS_IN(i,m) \
  printf("\nMAS:%p,role:%s | MSG:%p\n", \
                    inst, \
                      inst->re.role == NCSFT_ROLE_PRIMARY ? "PRIMARY" : \
                    inst->re.role == NCSFT_ROLE_BACKUP ? "BACKUP" : "OTHER", \
                      msg)
#else

#define m_MAB_DBG_TRACE2_MAS_IN(i,m) \
  printf("\nMAS:%p,role:%s | MSG:%p\n", \
                    inst, \
                    "NONE", \
                      msg)
#endif

#define m_MAB_DBG_TRACE2_MAS_TR_ALLOC(tr) \
  printf("\n ALLOCED TBL:(id:%d):%p\n",tr->tbl_id,tr)

#define m_MAB_DBG_TRACE2_MAS_TR_DALLOC(tr) \
  printf("\n DEALLOCED TBL:(id:%d):%p\n",tr->tbl_id,tr)

#if (NCS_RMS == 1)

#define m_MAB_DBG_TRACE2_MAS_DF_OP(i,m,tr,o) \
  printf("\n[DEF FLTR]:MAS_TBL:%p/role:%s/tbl_id:%d/vcard:%d/anc:%d\n", \
      i, \
      i->re.role == NCSFT_ROLE_PRIMARY ? "PRIMARY" : "OTHER", \
      tr->tbl_id, \
      tr->dfltr.vcard,msg->fr_anc)
#else

#define m_MAB_DBG_TRACE2_MAS_DF_OP(i,m,tr,o) \
  printf("\n[DEF FLTR]:MAS_TBL:%p/role:%s/tbl_id:%d/vcard:%d/anc:%d\n", \
      i, \
      "NONE", \
      tr->tbl_id, \
      tr->dfltr.vcard,msg->fr_anc)
#endif

#define m_MAB_DBG_TRACE2_MAS_MF_OP(i,mf,tid,o) mab_dbg_dump_fltr_op(i,mf,tid,o)
#else

#define m_MAB_DBG_TRACE2_MAS_IN(i,m)
#define m_MAB_DBG_TRACE2_MAS_TR_ALLOC(tr)
#define m_MAB_DBG_TRACE2_MAS_TR_DALLOC(tr)
#define m_MAB_DBG_TRACE2_MAS_DF_OP(i,m,tr,o)
#define m_MAB_DBG_TRACE2_MAS_MF_OP(i,mf,tid,o)
#endif

/*
 * m_<MAB>_VALIDATE_HDL
 *
 * VALIDATE_HDL   : in the LM create service for MAC, MAS and OAC, they each
 *                  return an NCSCONTEXT value called o_<MAB>_hdl (such as
 *                  o_mac_hdl. This value represents the internal control block
 *                  of that MAB sub-part, scoped by a particalar virtual router.
 *
 *                  When this macro is invoked, MAB wants the target service to
 *                  provide that handle value back. MAB does not know or care
 *                  how the value is stored or recovered. This is the business
 *                  of the target system.
 *   
 * NOTE: The default implementation here, is only an example. A real system can
 *       store this however it wants.
 */

EXTERN_C MABCOM_API void *sysf_mac_validate(uns32 k);
EXTERN_C MABCOM_API void *sysf_mas_validate(uns32 k);
EXTERN_C MABCOM_API void *sysf_oac_validate(uns32 k);
EXTERN_C MABCOM_API void *sysf_pss_validate(uns32 k);

/* The MAC validate macro */

#define m_MAC_VALIDATE_HDL(k)  sysf_mac_validate(k)

/* The MAS validate macro */

#define m_MAS_VALIDATE_HDL(k)  sysf_mas_validate(k)

/* The OAC validate macro */

#define m_OAC_VALIDATE_HDL(k)  sysf_oac_validate(k)

/* The PSS validate macro */

#define m_PSS_VALIDATE_HDL(k)  sysf_pss_validate(k)

/* Define PSS Target Service API here */
#define NCS_PSSTS_PROFILE_DESC_FILE     "ProDesc.txt"	/* Profile Description file name */
#define NCS_PSSTS_MAX_PATH_LEN           256	/* Maximum length of the path of a file */
#define NCS_PSSTS_ROOT_DIRECTORY         "c:"	/* Default root directory */
#define NCS_PSSTS_DEFAULT_PROFILE        "current"	/* Current profile name   */
#define NCS_PSSTS_TEMP_FILE_NAME         "tmp123.dat"	/* Temporary file name    */

typedef enum {
	NCS_PSSTS_OP_OPEN_FILE = 1,	/* Open an existing file for reading or a new/existing file for writing */
	NCS_PSSTS_OP_READ_FILE,	/* Read in some data from an open file from a given offset */
	NCS_PSSTS_OP_WRITE_FILE,	/* Write the given data to the file */
	NCS_PSSTS_OP_CLOSE_FILE,	/* Close the opened file */
	NCS_PSSTS_OP_FILE_EXISTS,	/* Returns TRUE if the file exists */
	NCS_PSSTS_OP_FILE_SIZE,	/* Returns the size of the file */
	NCS_PSSTS_OP_CREATE_PROFILE,	/* Creates a new profile. */
	NCS_PSSTS_OP_MOVE_PROFILE,	/* Renames a profile. So, moves the corresponding directory */
	NCS_PSSTS_OP_COPY_PROFILE,	/* Makes a copy of an existing profile */
	NCS_PSSTS_OP_DELETE_PROFILE,	/* Delete the files associated with an existing profile */
	NCS_PSSTS_OP_GET_PROFILE_DESC,	/* Get the Description Text for a profile */
	NCS_PSSTS_OP_SET_PROFILE_DESC,	/* Set the Description Text for a profile */
	NCS_PSSTS_OP_GET_PSS_CONFIG,	/* Retrieve the basic configuration for PSS */
	NCS_PSSTS_OP_SET_PSS_CONFIG,	/* Set the basic configuration for PSS */
	NCS_PSSTS_OP_FILE_DELETE,	/* Delete a MIB file in a given profile */
	NCS_PSSTS_OP_PCN_DELETE,	/* Delete a PCN in a given profile */
	NCS_PSSTS_OP_OPEN_TEMP_FILE,	/* Open a temporary file for writing some data */
	NCS_PSSTS_OP_COPY_TEMP_FILE,	/* Copy the contents of the temporary file to the actual file */
	NCS_PSSTS_OP_GET_NEXT_PROFILE,	/* Get the next profile on the persistent store */
	NCS_PSSTS_OP_GET_MIB_LIST_PER_PCN,	/* Get the MIB files list for a <pwe><PCN> from the persistent store */
	NCS_PSSTS_OP_PROFILE_EXISTS,	/* Check if the given profile exists */
	NCS_PSSTS_OP_PWE_EXISTS,	/* Check if a pwe exists for a given profile */
	NCS_PSSTS_OP_PCN_EXISTS,	/* Check if a vcard exists for a given profile, pwe */
	NCS_PSSTS_OP_GET_CLIENTS,	/* Get the list of clients, given a profile */
	NCS_PSSTS_OP_DELETE_TEMP_FILE,
	NCS_PSSTS_OP_MOVE_TBL_DETAILS_FILE,
	NCS_PSSTS_OP_DELETE_TBL_DETAILS_FILE,
	NCS_PSSTS_OP_MAX
} NCS_PSSTS_REQUEST;

/* Modes in which the files can be opened */
#define NCS_PSSTS_FILE_PERM_READ   0x01
#define NCS_PSSTS_FILE_PERM_WRITE  0x02

typedef struct ncs_pssts_arg_open_file {
	uns8 *i_profile_name;
	char *i_pcn;
	uns16 i_pwe_id;
	uns32 i_tbl_id;
	uns8 i_mode;		/* Mode in which the file is to be opened */
	long o_handle;
} NCS_PSSTS_ARG_OPEN_FILE;

typedef struct ncs_pssts_arg_read_file {
	long i_handle;
	uns32 i_bytes_to_read;
	uns32 i_offset;
	uns8 *io_buffer;
	uns32 o_bytes_read;
} NCS_PSSTS_ARG_READ_FILE;

typedef struct ncs_pssts_arg_write_file {
	long i_handle;
	uns8 *i_buffer;
	uns32 i_buf_size;
} NCS_PSSTS_ARG_WRITE_FILE;

typedef struct ncs_pssts_arg_close_file {
	long i_handle;
} NCS_PSSTS_ARG_CLOSE_FILE;

typedef struct ncs_pssts_arg_file_exists {
	uns8 *i_profile_name;
	char *i_pcn;
	uns16 i_pwe_id;
	uns32 i_tbl_id;
	NCS_BOOL o_exists;
} NCS_PSSTS_ARG_FILE_EXISTS;

typedef struct ncs_pssts_arg_file_size {
	uns8 *i_profile_name;
	char *i_pcn;
	uns16 i_pwe_id;
	uns32 i_tbl_id;
	uns32 o_file_size;
} NCS_PSSTS_ARG_FILE_SIZE;

typedef struct ncs_pssts_arg_create_profile {
	uns8 *i_profile_name;
} NCS_PSSTS_ARG_CREATE_PROFILE;

typedef struct ncs_pssts_arg_move_profile {
	uns8 *i_profile_name;
	uns8 *i_new_profile_name;
} NCS_PSSTS_ARG_MOVE_PROFILE;

typedef struct ncs_pssts_arg_copy_profile {
	uns8 *i_profile_name;
	uns8 *i_new_profile_name;
} NCS_PSSTS_ARG_COPY_PROFILE;

typedef struct ncs_pssts_arg_delete_profile {
	uns8 *i_profile_name;
} NCS_PSSTS_ARG_DELETE_PROFILE;

typedef struct ncs_pssts_arg_delete_file {
	uns8 *i_profile_name;
	uns16 i_pwe;
	char *i_pcn;
	uns32 i_tbl_id;
} NCS_PSSTS_ARG_DELETE_FILE;

typedef struct ncs_pssts_arg_delete_pcn {
	uns8 *i_profile_name;
	uns16 i_pwe;
	char *i_pcn;
} NCS_PSSTS_ARG_DELETE_PCN;

typedef struct ncs_pssts_arg_get_desc {
	uns8 *i_profile_name;
	uns8 *io_buffer;
	uns32 i_buf_length;
	NCS_BOOL o_exists;
} NCS_PSSTS_ARG_GET_DESC;

typedef struct ncs_pssts_arg_set_desc {
	uns8 *i_profile_name;
	uns8 *i_buffer;
} NCS_PSSTS_ARG_SET_DESC;

typedef struct ncs_pssts_arg_pss_config {
	uns8 *current_profile_name;	/* null-terminated string */
} NCS_PSSTS_ARG_PSS_CONFIG;

typedef struct ncs_pssts_arg_set_config {
	uns8 *i_current_profile_name;	/* null-terminated string */
} NCS_PSSTS_ARG_SET_CONFIG;

typedef struct ncs_pssts_arg_open_temp_file {
	long o_handle;		/* Handle to the opened file */
} NCS_PSSTS_ARG_OPEN_TEMP_FILE;

typedef struct ncs_pssts_arg_copy_temp_file {
	uns8 *i_profile_name;
	char *i_pcn;
	uns16 i_pwe_id;
	uns32 i_tbl_id;
} NCS_PSSTS_ARG_COPY_TEMP_FILE;

typedef struct ncs_pssts_arg_get_next_profile {
	uns8 *i_profile_name;
	uns8 *io_buffer;
	uns32 i_buf_length;
} NCS_PSSTS_ARG_GET_NEXT_PROFILE;

typedef struct ncs_pssts_arg_profile_exists {
	uns8 *i_profile_name;
	NCS_BOOL o_exists;
} NCS_PSSTS_ARG_PROFILE_EXISTS;

typedef struct ncs_pssts_arg_pwe_exists {
	uns8 *i_profile_name;
	uns16 i_pwe;
	NCS_BOOL o_exists;
} NCS_PSSTS_ARG_PWE_EXISTS;

typedef struct ncs_pssts_arg_pcn_exists {
	uns8 *i_profile_name;
	uns16 i_pwe;
	char *i_pcn;
	NCS_BOOL o_exists;
} NCS_PSSTS_ARG_PCN_EXISTS;

typedef struct ncs_pssts_arg_get_clients {
	uns8 *i_profile_name;
	USRBUF *o_usrbuf;	/* Clients information is encoded and available here */
} NCS_PSSTS_ARG_GET_CLIENTS;

typedef struct ncs_pssts_arg_get_miblist_per_pcn {
	uns8 *i_profile_name;
	char *i_pcn;
	USRBUF *o_usrbuf;	/* MIB LIST information is encoded and available here */
} NCS_PSSTS_ARG_GET_MIBLIST_PER_PCN;

typedef struct ncs_pssts_arg_move_tbl_details_file {
	uns8 *i_profile_name;
	char *i_pcn;
	uns16 i_pwe_id;
	uns32 i_tbl_id;
} NCS_PSSTS_ARG_MOVE_TBL_DETAILS_FILE;

typedef struct ncs_pssts_arg_tag {
	NCS_PSSTS_REQUEST i_op;	/* Specifies the request type */
	uns32 ncs_hdl;		/* Handle from Handle Manager */
	union {
		NCS_PSSTS_ARG_OPEN_FILE open_file;
		NCS_PSSTS_ARG_READ_FILE read_file;
		NCS_PSSTS_ARG_WRITE_FILE write_file;
		NCS_PSSTS_ARG_CLOSE_FILE close_file;
		NCS_PSSTS_ARG_FILE_EXISTS file_exists;
		NCS_PSSTS_ARG_FILE_SIZE file_size;
		NCS_PSSTS_ARG_CREATE_PROFILE create_profile;
		NCS_PSSTS_ARG_MOVE_PROFILE move_profile;
		NCS_PSSTS_ARG_COPY_PROFILE copy_profile;
		NCS_PSSTS_ARG_DELETE_PROFILE delete_profile;
		NCS_PSSTS_ARG_DELETE_FILE delete_file;
		NCS_PSSTS_ARG_DELETE_PCN delete_pcn;
		NCS_PSSTS_ARG_GET_DESC get_desc;
		NCS_PSSTS_ARG_SET_DESC set_desc;
		NCS_PSSTS_ARG_PSS_CONFIG pss_config;
		NCS_PSSTS_ARG_SET_CONFIG set_config;
		NCS_PSSTS_ARG_OPEN_TEMP_FILE open_tfile;
		NCS_PSSTS_ARG_COPY_TEMP_FILE copy_tfile;
		NCS_PSSTS_ARG_GET_NEXT_PROFILE get_next;
		NCS_PSSTS_ARG_PROFILE_EXISTS profile_exists;
		NCS_PSSTS_ARG_PWE_EXISTS pwe_exists;
		NCS_PSSTS_ARG_PCN_EXISTS pcn_exists;
		NCS_PSSTS_ARG_GET_CLIENTS get_clients;
		NCS_PSSTS_ARG_GET_MIBLIST_PER_PCN get_miblist_per_pcn;
		/* Deleting temporary file : Arguments are not required for this as it is always in the same path with the same name */
		NCS_PSSTS_ARG_MOVE_TBL_DETAILS_FILE move_detailsfile;
		/* Deleting table details file: Structure NCS_PSSTS_ARG_DELETE_FILE can be used */
	} info;

} NCS_PSSTS_ARG;

typedef uns32 (*NCS_PSSTS) (NCS_PSSTS_ARG *parg);

typedef enum {
	NCS_PSSTS_LM_OP_CREATE = 1,
	NCS_PSSTS_LM_OP_DESTROY,
	NCS_PSSTS_LM_OP_MAX
} NCS_PSSTS_LM_OP;

typedef struct ncs_pssts_lm_create {
	uns32 i_usr_key;
	uns8 i_hmpool_id;
	uns8 *i_root_dir;
	uns32 o_handle;
} NCS_PSSTS_LM_CREATE;

typedef struct ncs_pssts_lm_destroy {
	uns32 i_handle;
} NCS_PSSTS_LM_DESTROY;

typedef struct ncs_pssts_cb {
	uns8 hmpool_id;
	uns32 hm_hdl;
	uns8 root_dir[NCS_PSSTS_MAX_PATH_LEN];
	uns8 current_profile[NCS_PSSTS_MAX_PATH_LEN];
	uns32 my_key;
} NCS_PSSTS_CB;

typedef struct ncs_pssts_lm_arg {
	NCS_PSSTS_LM_OP i_op;
	union {
		NCS_PSSTS_LM_CREATE create;
		NCS_PSSTS_LM_DESTROY destroy;
	} info;

} NCS_PSSTS_LM_ARG;

/* Sort table for list of profiles in the persistent store. */
typedef struct ncs_pssts_sort_key_tag {
	uns16 len;
	char name[NCS_PSSTS_MAX_PATH_LEN];
} NCS_PSSTS_SORT_KEY;

typedef struct ncs_pssts_sort_node_tag {
	NCS_PATRICIA_NODE pat_node;

	NCS_PSSTS_SORT_KEY key;
} NCS_PSSTS_SORT_NODE;

typedef struct ncs_pssts_sort_db_tag {
	NCS_PATRICIA_TREE tree;

	NCS_PATRICIA_PARAMS params;
} NCS_PSSTS_SORT_DB;

/***************************************************************************
 * Global Instance of Layer Management
 ***************************************************************************/

EXTERN_C MABCOM_API uns32 ncspssts_lm(NCS_PSSTS_LM_ARG *arg);

/***************************************************************************
 * Some useful macros for PSSTS
 ***************************************************************************/

#define m_NCS_PSSTS_PROFILE_EXISTS(func, hdl, ret, name, exists) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_PROFILE_EXISTS; \
        pssts_arg.info.profile_exists.i_profile_name = name; \
        pssts_arg.info.profile_exists.o_exists = FALSE; \
        ret = (func)(&pssts_arg); \
        exists = pssts_arg.info.profile_exists.o_exists; \
    }

#define m_NCS_PSSTS_GET_DESC(func, hdl, ret, name, len, buf, exists) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_GET_PROFILE_DESC; \
        pssts_arg.info.get_desc.i_profile_name = name; \
        pssts_arg.info.get_desc.i_buf_length = len; \
        pssts_arg.info.get_desc.io_buffer = buf; \
        ret = (func)(&pssts_arg); \
        exists = pssts_arg.info.get_desc.o_exists; \
    }

#define m_NCS_PSSTS_SET_DESC(func, hdl, ret, name, buf) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_SET_PROFILE_DESC; \
        pssts_arg.info.set_desc.i_profile_name = name; \
        pssts_arg.info.set_desc.i_buffer = buf; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_PROFILE_CREATE(func, hdl, ret, name) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_CREATE_PROFILE; \
        pssts_arg.info.create_profile.i_profile_name = name; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_PROFILE_DELETE(func, hdl, ret, name) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_DELETE_PROFILE; \
        pssts_arg.info.delete_profile.i_profile_name = name; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_GET_NEXT_PROFILE(func, hdl, ret, name, len, buf) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_GET_NEXT_PROFILE; \
        pssts_arg.info.get_next.i_profile_name = name; \
        pssts_arg.info.get_next.i_buf_length = len; \
        pssts_arg.info.get_next.io_buffer = buf; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_PROFILE_COPY(func, hdl, ret, name, copyfrom) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_COPY_PROFILE; \
        pssts_arg.info.copy_profile.i_profile_name = copyfrom; \
        pssts_arg.info.copy_profile.i_new_profile_name = name; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_PROFILE_MOVE(func, hdl, ret, name, movefrom) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_MOVE_PROFILE; \
        pssts_arg.info.move_profile.i_profile_name = movefrom; \
        pssts_arg.info.move_profile.i_new_profile_name = name; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_PWE_EXISTS(func, hdl, ret, name, pwe, exists) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_PWE_EXISTS; \
        pssts_arg.info.pwe_exists.i_profile_name = name; \
        pssts_arg.info.pwe_exists.i_pwe = pwe; \
        pssts_arg.info.pwe_exists.o_exists = FALSE; \
        ret = (func)(&pssts_arg); \
        exists = pssts_arg.info.pwe_exists.o_exists; \
    }

#define m_NCS_PSSTS_FILE_EXISTS(func, hdl, ret, profile, pwe, pcn_name, tbl, exists) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_FILE_EXISTS; \
        pssts_arg.info.file_exists.i_profile_name = profile; \
        pssts_arg.info.file_exists.i_pwe_id = pwe; \
        pssts_arg.info.file_exists.i_pcn = pcn_name; \
        pssts_arg.info.file_exists.i_tbl_id = tbl; \
        pssts_arg.info.file_exists.o_exists = FALSE; \
        ret = (func)(&pssts_arg); \
        exists = pssts_arg.info.file_exists.o_exists; \
    }

#define m_NCS_PSSTS_FILE_OPEN(func, hdl, ret, profile, pwe, pcn_name, tbl, mode, filehdl) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_OPEN_FILE; \
        pssts_arg.info.open_file.i_profile_name = profile; \
        pssts_arg.info.open_file.i_pwe_id = pwe; \
        pssts_arg.info.open_file.i_pcn = pcn_name; \
        pssts_arg.info.open_file.i_tbl_id = tbl; \
        pssts_arg.info.open_file.i_mode = mode; \
        pssts_arg.info.open_file.o_handle = 0; \
        ret = (func)(&pssts_arg); \
        filehdl = pssts_arg.info.open_file.o_handle; \
    }

#define m_NCS_PSSTS_FILE_OPEN_TEMP(func, hdl, ret, tfilehdl) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_OPEN_TEMP_FILE; \
        ret = (func)(&pssts_arg); \
        tfilehdl = pssts_arg.info.open_tfile.o_handle; \
    }

#define m_NCS_PSSTS_FILE_READ(func, hdl, ret, filehdl, bytes_to_read, offset, buffer, bytes_read) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_READ_FILE; \
        pssts_arg.info.read_file.i_handle = filehdl; \
        pssts_arg.info.read_file.i_bytes_to_read = bytes_to_read; \
        pssts_arg.info.read_file.i_offset = offset; \
        pssts_arg.info.read_file.io_buffer = buffer; \
        pssts_arg.info.read_file.o_bytes_read = 0; \
        ret = (func)(&pssts_arg); \
        bytes_read = pssts_arg.info.read_file.o_bytes_read; \
    }

#define m_NCS_PSSTS_FILE_WRITE(func, hdl, ret, filehdl, bytes_to_write, buffer) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_WRITE_FILE; \
        pssts_arg.info.write_file.i_handle = filehdl; \
        pssts_arg.info.write_file.i_buf_size = bytes_to_write; \
        pssts_arg.info.write_file.i_buffer = buffer; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_FILE_CLOSE(func, hdl, ret, filehdl) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_CLOSE_FILE; \
        pssts_arg.info.close_file.i_handle = filehdl; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_FILE_SIZE(func, hdl, ret, profile, pwe, pcn_name, tbl, file_size) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_FILE_SIZE; \
        pssts_arg.info.file_size.i_profile_name = profile; \
        pssts_arg.info.file_size.i_pwe_id       = pwe; \
        pssts_arg.info.file_size.i_pcn     = pcn_name; \
        pssts_arg.info.file_size.i_tbl_id       = tbl; \
        ret = (func)(&pssts_arg); \
        file_size = pssts_arg.info.file_size.o_file_size; \
    }

#define m_NCS_PSSTS_FILE_COPY_TEMP(func, hdl, ret, profile, pwe, pcn_name, tbl) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_COPY_TEMP_FILE; \
        pssts_arg.info.copy_tfile.i_profile_name = profile; \
        pssts_arg.info.copy_tfile.i_pwe_id       = pwe; \
        pssts_arg.info.copy_tfile.i_pcn     = pcn_name; \
        pssts_arg.info.copy_tfile.i_tbl_id       = tbl; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_SET_PSS_CONFIG(func, hdl, ret, current_profile) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_SET_PSS_CONFIG; \
        pssts_arg.info.set_config.i_current_profile_name = current_profile; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_FILE_DELETE(func, hdl, ret, profile, pwe, pcn_name, tbl_id) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_FILE_DELETE; \
        pssts_arg.info.delete_file.i_profile_name = profile; \
        pssts_arg.info.delete_file.i_pwe = pwe; \
        pssts_arg.info.delete_file.i_pcn = pcn_name; \
        pssts_arg.info.delete_file.i_tbl_id = tbl_id; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_PCN_DELETE(func, hdl, ret, profile, pwe, pcn_name) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_PCN_DELETE; \
        pssts_arg.info.delete_pcn.i_profile_name = profile; \
        pssts_arg.info.delete_pcn.i_pwe = pwe; \
        pssts_arg.info.delete_pcn.i_pcn = pcn_name; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_GET_CLIENTS(func, hdl, ret, profile, o_p_usrbuf) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_GET_CLIENTS; \
        pssts_arg.info.get_clients.i_profile_name = profile; \
        ret = (func)(&pssts_arg); \
        *o_p_usrbuf = pssts_arg.info.get_clients.o_usrbuf; \
    }

#define m_NCS_PSSTS_GET_MIB_LIST_PER_PCN(func, hdl, ret, profile, pcn, o_p_usrbuf) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_GET_MIB_LIST_PER_PCN; \
        pssts_arg.info.get_miblist_per_pcn.i_profile_name = profile; \
        pssts_arg.info.get_miblist_per_pcn.i_pcn = pcn; \
        ret = (func)(&pssts_arg); \
        *o_p_usrbuf = pssts_arg.info.get_miblist_per_pcn.o_usrbuf; \
    }

#define m_NCS_PSSTS_DELETE_TEMP_FILE(func, hdl, ret) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_DELETE_TEMP_FILE; \
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_MOVE_TBL_DETAILS_FILE(func, hdl, ret, profile, pwe, pcn_name, tbl) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_MOVE_TBL_DETAILS_FILE; \
        pssts_arg.info.move_detailsfile.i_profile_name = profile; \
        pssts_arg.info.move_detailsfile.i_pwe_id       = pwe; \
        pssts_arg.info.move_detailsfile.i_pcn          = pcn_name; \
        pssts_arg.info.move_detailsfile.i_tbl_id       = tbl; \
        printf("\nEntered Move Table Details macro");\
        ret = (func)(&pssts_arg); \
    }

#define m_NCS_PSSTS_DELETE_TBL_DETAILS_FILE(func, hdl, ret, profile, pwe, pcn_name, tbl_id) \
    { \
        assert(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE); \
        NCS_PSSTS_ARG pssts_arg; \
        memset(&pssts_arg, 0, sizeof(pssts_arg)); \
        pssts_arg.ncs_hdl = hdl; \
        pssts_arg.i_op   = NCS_PSSTS_OP_DELETE_TBL_DETAILS_FILE; \
        pssts_arg.info.delete_file.i_profile_name = profile; \
        pssts_arg.info.delete_file.i_pwe = pwe; \
        pssts_arg.info.delete_file.i_pcn = pcn_name; \
        pssts_arg.info.delete_file.i_tbl_id = tbl_id; \
        ret = (func)(&pssts_arg); \
    }

#endif   /* MAB_TGT_H */
