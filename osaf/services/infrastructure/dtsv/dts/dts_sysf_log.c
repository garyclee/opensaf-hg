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

  MODULE NAME: SYSF_LOG.C

..............................................................................

  DESCRIPTION:

  The LEAP MDS reference implementation. It is written as if all is local on
  a single card.

******************************************************************************
*/

#include <dlfcn.h>

#include "ncs_log.h"
#include "dts.h"

static uint32_t dts_get_and_return_val(char *t_str, char *ch, char *time, NCS_UBAID *uba,
				    NCSFL_ASCII_SPEC *spec, NCSFL_SET *set, long *arg_list,
				    uint32_t *log_msg_len, uint8_t msg_fmat_ver);

/*****************************************************************************

  PROCEDURE NAME:    dts_ascii_spec_register

  DESCRIPTION:       Introduce subsystem specific ASCII printing info for
                     logging and bump the ref-count. If the subsystem has
                     already registered (presumably a different instance),
                     just bump the refcount.

*****************************************************************************/

uint32_t dts_ascii_spec_register(NCSFL_ASCII_SPEC *spec)
{
	uint16_t i, j;
	NCSFL_SET *set = spec->str_set;
	NCSFL_FMAT *fmat = spec->fmat_set;
	NCSFL_STR *str;
	SYSF_ASCII_SPECS *spec_entry = NULL;
	SS_SVC_ID nt_svc_id;
	ASCII_SPEC_INDEX spec_key;

	if (spec == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_ascii_spec_register: NULL pointer passed as argument");

	if ((set == NULL) || (fmat == NULL))
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_ascii_spec_register: Either of spec->str_set or spec->fmat_set is passed as NULL");

	/*  Network order key added */
	nt_svc_id = m_NCS_OS_HTONL(spec->ss_id);

	/* memset the spec_key struct */
	memset(&spec_key, '\0', sizeof(ASCII_SPEC_INDEX));
	spec_key.svc_id = nt_svc_id;
	spec_key.ss_ver = spec->ss_ver;
	/* Versioning support : No support for old method of spec loading */
	/* using new key to index to ASCII_SPEC patricia tree */
	if ((spec_entry =
	     (SYSF_ASCII_SPECS *)ncs_patricia_tree_get(&dts_cb.svcid_asciispec_tree,
						       (const uint8_t *)&spec_key)) != NULL) {
		m_LOG_DTS_DBGSTR(DTS_SERVICE, "ASCII_SPEC table already registered for svc ", 0, spec->ss_id);
		return NCSCC_RC_SUCCESS;
	}

	/* Check for NULL service name */
	if (spec->svc_name == NULL) {
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_ERR_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE,
			   "TILLC", DTS_SPEC_SVC_NAME_NULL, 0, spec->ss_id, NULL);
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_REG, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILCL",
			   DTS_SPEC_REG_FAIL, spec->ss_id, spec->svc_name, spec->ss_ver);
		return m_DTS_DBG_SINK_SVC(NCSCC_RC_FAILURE,
					  "dts_ascii_spec_register: Service Name string is NULL. Check your service name",
					  spec->ss_id);
	}

	/* Check for the string length for svc_name, 
	   should not be more than 15 chars */
	if ((strlen(spec->svc_name) + 1) > DTSV_SVC_NAME_MAX) {
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_ERR_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE,
			   "TILLC", DTS_SPEC_SVC_NAME_ERR, 0, spec->ss_id, NULL);
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_REG, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILCL",
			   DTS_SPEC_REG_FAIL, spec->ss_id, NULL, spec->ss_ver);
		return m_DTS_DBG_SINK_SVC(NCSCC_RC_FAILURE,
					  "dts_ascii_spec_register: Service Name string size exceeds limits. Check your service name",
					  spec->ss_id);

	}

	/* validation stage : confirm offsets are all correct */
	for (i = 0; set[i].set_vals != NULL; i++) {
		if (set[i].set_id != i) {
			ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_ERR_EVT, DTS_FC_EVT, NCSFL_LC_EVENT,
				   NCSFL_SEV_NOTICE, "TILLC", DTS_SPEC_SET_ERR, i, spec->ss_id, spec->svc_name);
			ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_REG, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE,
				   "TILCL", DTS_SPEC_REG_FAIL, spec->ss_id, spec->svc_name, spec->ss_ver);
			return m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE,
						       "dts_ascii_spec_register: String set ID mismatch. Check your strung set ID's",
						       spec->svc_name);

		}
		str = set[i].set_vals;

		for (j = 0; str[j].str_val != NULL; j++) {
			if (str[j].str_id != j) {
				ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_ERR_EVT, DTS_FC_EVT, NCSFL_LC_EVENT,
					   NCSFL_SEV_NOTICE, "TILLC", DTS_SPEC_STR_ERR, j, spec->ss_id, spec->svc_name);
				ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_REG, DTS_FC_EVT, NCSFL_LC_EVENT,
					   NCSFL_SEV_NOTICE, "TILCL", DTS_SPEC_REG_FAIL, spec->ss_id, spec->svc_name,
					   spec->ss_ver);
				return m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE,
							       "dts_ascii_spec_register: String Index ID mismatched. Check your string index ID's",
							       spec->svc_name);

			}
		}
		set[i].set_cnt = j;
	}
	spec->str_set_cnt = i;

	for (i = 0; fmat[i].fmat_str != NULL; i++) {
		if (fmat[i].fmat_id != i) {
			ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_ERR_EVT, DTS_FC_EVT, NCSFL_LC_EVENT,
				   NCSFL_SEV_NOTICE, "TILLC", DTS_SPEC_FMAT_ERR, i, spec->ss_id, spec->svc_name);
			ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_REG, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE,
				   "TILCL", DTS_SPEC_REG_FAIL, spec->ss_id, spec->svc_name, spec->ss_ver);
			return m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE,
						       "dts_ascii_spec_register: String format ID mismatch. Check your string format ID's",
						       spec->svc_name);
		}
	}
	spec->fmat_set_cnt = i;

	/* Add new entry to the patricia tree */
	spec_entry = m_MMGR_ALLOC_DTS_SPEC_ENTRY;
	if (spec_entry == NULL)
		return m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE,
					       "dts_ascii_spec_register: Failed to allocate memory for spec registration",
					       spec->svc_name);
	memset(spec_entry, '\0', sizeof(SYSF_ASCII_SPECS));

	spec_entry->ss_spec = spec;
	/*  Network order key added */
	spec_entry->key.svc_id = nt_svc_id;
	spec_entry->key.ss_ver = spec->ss_ver;
	spec_entry->svcid_node.key_info = (uint8_t *)&spec_entry->key;

	if (ncs_patricia_tree_add(&dts_cb.svcid_asciispec_tree, (NCS_PATRICIA_NODE *)spec_entry) != NCSCC_RC_SUCCESS) {
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_REG, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILCL",
			   DTS_SPEC_REG_FAIL, spec->ss_id, spec->svc_name, spec->ss_ver);
		m_MMGR_FREE_DTS_SPEC_ENTRY(spec_entry);
		return m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE,
					       "dts_ascii_spec_register: Failed to add patricia tree entry",
					       spec->svc_name);
	}

	/* versioning support : The following piece of code is not necessary now. 
	 * Associating spec ptrs with svc_reg_tbl will now be done only during
	 * service registrations.
	 */

	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SPEC_REG, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILCL",
		   DTS_SPEC_REG_SUCC, spec->ss_id, spec->svc_name, spec->ss_ver);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    dts_ascii_spec_deregister

  DESCRIPTION:       Remove all references in corresponding svc reg table 
                     entries. If ASCII_SPEC table is present in patricia tree
                     free the entry and return success. Else return failure.
                    
                     versioning support changes - no need to go through the 
                     svc_reg_tbl entries to remove references to spec ptr.
                     just decrement the use count and delete only when use 
                     count goes to 0.
*****************************************************************************/

uint32_t dts_ascii_spec_deregister(SS_SVC_ID ss_id, uint16_t version)
{
	SYSF_ASCII_SPECS *spec_entry;
	ASCII_SPEC_INDEX spec_key;

	memset(&spec_key, '\0', sizeof(spec_key));
	spec_key.svc_id = m_NCS_OS_HTONL(ss_id);
	spec_key.ss_ver = version;

	/* If spec entry is not there then return error. if its there and use_count
	 * is zero, then remove it from tree else return success.
	 * Note: use_count for this struct is decremented while removing structures
	 * from svc_reg_tbl's spec_list.
	 */
	if ((spec_entry =
	     (SYSF_ASCII_SPECS *)ncs_patricia_tree_get(&dts_cb.svcid_asciispec_tree,
						       (const uint8_t *)&spec_key)) == NULL) {
		m_LOG_DTS_DBGSTR(DTS_SERVICE, "No ASCII_SPEC table corresponding to the service id", 0, ss_id);
		return NCSCC_RC_FAILURE;
	} else if (spec_entry->use_count == 0) {
		spec_entry->ss_spec = NULL;
		ncs_patricia_tree_del(&dts_cb.svcid_asciispec_tree, (NCS_PATRICIA_NODE *)spec_entry);
		m_MMGR_FREE_DTS_SPEC_ENTRY(spec_entry);
	}
	m_LOG_DTS_DBGSTR(DTS_SERVICE, "ASCII_SPEC table de-registered successfully for service", 0, ss_id);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    dts_ascii_spec_load
 
  INPUTS:            Pointer to the service registration table,

                     Pointer to the DTA_DEST_LIST structure for the DTA which

                     char pointer: having the service name. (Should be identical
                     to the service name in the ASCII_SPEC table being loaded.)i

                     uint16_t version no. of the ASCII_SPEC table. (Shared
                     library should have the identical version no.)

                     action to be performed.
                     0 for Unregister
                     1 for Register 
  
  RETURN             Handle to lib entry when action is 1. Else NULL. 

  DESCRIPTION:       Loads the ASCII_SPEC table from a shared library which is
                     formed from the service name and version no. passed as the
                     parameters as follows: lib_<svc_name>_<version>_logstr.so
                     It calls the entry function, again formed from the svc name
                     passed as follows: <svc_name>_log_str_lib_req(), to load
                     the symbols.
                     Users should always follow this convention while forming 
                     their ASCII_SPEC structures and loading/unloading 
                     functions.

*****************************************************************************/
NCSCONTEXT dts_ascii_spec_load(char *svc_name, uint16_t version, DTS_SPEC_ACTION action)
{
	char lib_name[DTS_MAX_LIBNAME] = { 0 };
	char func_name[DTS_MAX_FUNCNAME] = { 0 };
	uint32_t (*reg_unreg_routine) () = NULL;
	char *dl_error = NULL;
	NCS_LIB_REQ_INFO req_info;
	void *lib_hdl = NULL;
	ASCII_SPEC_LIB *lib_entry = NULL;
	char dbg_str[DTS_MAX_LIB_DBG];
	uint32_t status = NCSCC_RC_SUCCESS;

	/* first form the libname and fucntion names */
	/* Note: Version is given as per SAF versioning standards, so release code
	 * should always be a character and major & minor versions always intergers
	 */
	sprintf(lib_name, "lib%s_logstr.so.%d", svc_name, version);
	sprintf(func_name, "%s_log_str_lib_req", svc_name);

	/* convert them to all lowercase */
	dts_to_lowercase(lib_name);
	dts_to_lowercase(func_name);

	/* Check if the library name is already loaded. If loaded, then return
	 * after incrementing the use_count for registration action.
	 * If library is not present and de-registration event comes then return
	 * error.
	 */
	if (action == DTS_SPEC_LOAD) {	/* Reg event */
		/* If lib name is already present just bump the use_count */
		if ((lib_entry =
		     (ASCII_SPEC_LIB *)ncs_patricia_tree_get(&dts_cb.libname_asciispec_tree,
							     (const uint8_t *)lib_name)) != NULL) {
			lib_entry->use_count++;
			TRACE("\ndts_ascii_spec_load(): Library: %s already loaded\n", lib_name);
			return lib_entry;
		}
	} else if (action == DTS_SPEC_UNLOAD) {
		/* If lib name is not present return */
		if ((lib_entry =
		     (ASCII_SPEC_LIB *)ncs_patricia_tree_get(&dts_cb.libname_asciispec_tree,
							     (const uint8_t *)lib_name)) == NULL) {
			m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_ascii_spec_load: Failed to unload library");
			return NULL;
		}
	}

	/* Load the library */
	lib_hdl = dlopen(lib_name, RTLD_LAZY);
	if ((dl_error = dlerror()) != NULL) {
		/* log the error returned from dlopen() */
		m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_ascii_spec_load: Unable to load library.");
		return NULL;
	}

	/* Load the symbols by using the entry fucntion */
	reg_unreg_routine = dlsym(lib_hdl, func_name);
	if ((dl_error = dlerror()) != NULL) {
		/* log the error returned from dlopen() */
		m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_ascii_spec_load: Unable to load symbol");
		return NULL;
	}

	/* do the INIT/DEINIT now... */
	if (reg_unreg_routine != NULL) {
		memset(&req_info, 0, sizeof(NCS_LIB_REQ_INFO));

		/* Reg/De-reg based upon action */
		if (action == DTS_SPEC_LOAD)
			req_info.i_op = NCS_LIB_REQ_CREATE;
		else
			req_info.i_op = NCS_LIB_REQ_DESTROY;

		/* do the registration */
		status = (*reg_unreg_routine) (&req_info);
		if (status != NCSCC_RC_SUCCESS) {
			/* log the error */
			if (action == DTS_SPEC_LOAD) {
				sprintf(dbg_str, "ASCII spec registration failed for - %s", lib_name);
				m_LOG_DTS_DBGSTR_NAME(DTS_GLOBAL, dbg_str, 0, 0);
			} else {
				sprintf(dbg_str, "ASCII spec de-registration failed for - %s", lib_name);
				m_LOG_DTS_DBGSTR_NAME(DTS_GLOBAL, dbg_str, 0, 0);
			}
		} else {	/* Reg/De-reg successful */

			if (action == DTS_SPEC_LOAD) {	/* Reg event */
				/* Create patricia tree entry for ascii_spec lib name */
				lib_entry = m_MMGR_ALLOC_DTS_LIBNAME;
				if (lib_entry == NULL) {
					m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						       "dts_ascii_spec_load: Memory allocation for patricia node failed");
					return NULL;
				}
				memset(lib_entry, '\0', sizeof(ASCII_SPEC_LIB));
				strcpy((char *)lib_entry->lib_name, lib_name);
				lib_entry->libname_node.key_info = (uint8_t *)lib_entry->lib_name;
				lib_entry->lib_hdl = lib_hdl;
				/* Add node to patricia tree table */
				if (ncs_patricia_tree_add
				    (&dts_cb.libname_asciispec_tree,
				     (NCS_PATRICIA_NODE *)lib_entry) != NCSCC_RC_SUCCESS) {
					m_MMGR_FREE_DTS_LIBNAME(lib_entry);
					m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						       "dts_ascii_spec_load: Failed to add node to paticia tree");
					return NULL;
				}
				lib_entry->use_count++;
				return lib_entry;
			} else if (action == DTS_SPEC_UNLOAD) {	/* de-reg command */
				/* Remove entry if use_count falls to 0 */
				if (lib_entry->use_count == 0) {
					if (lib_entry->lib_hdl != NULL)
						dlclose(lib_entry->lib_hdl);
					ncs_patricia_tree_del(&dts_cb.libname_asciispec_tree,
							      (NCS_PATRICIA_NODE *)lib_entry);
					m_MMGR_FREE_DTS_LIBNAME(lib_entry);
					m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						       "dts_ascii_spec_load: unloaded a ASCII_SPEC shared lib");
				}
			}
			/*end of else if(action == DTS_SPEC_UNLOAD) */
		}		/*end of else */
	}
	/*end of if reg_unreg_routine != NULL */
	return NULL;
}

/*****************************************************************************

  PROCEDURE NAME:    dts_log_msg_to_str

  DESCRIPTION:       Converts the Flex Log message into string.

*****************************************************************************/

uint32_t dts_log_msg_to_str(DTA_LOG_MSG *logmsg, char *str, NODE_ID node, uint32_t proc_id, uint32_t *len,
			 NCSFL_ASCII_SPEC *spec)
{
	char *temp_str = dts_cb.t_log_str;
	NCSFL_FMAT *fmat;
	NCSFL_SET *set;
	time_t tod;
	char asc_tod[32];
	char *sev;
	long args[30];
	uint32_t log_msg_len = 0;
	NCSFL_NORMAL *msg = &logmsg->log_msg;

	asc_tod[0] = 0;

	/*Initialize args to NULL to prevent DTS frm crashing */
	memset(args, '\0', sizeof(args));

	if (spec == NULL) {
		m_MMGR_FREE_OCT(msg->hdr.fmat_type);
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_LOG_ERR2, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILL",
			   DTS_SPEC_NOT_REG, 0, msg->hdr.ss_id);
		return NCSCC_RC_FAILURE;
	}

	/* A REAL Flexlog would expand TIME STAMP to ASCII form, at right time */

	tod = (time_t)msg->hdr.time.seconds;

	/* DTSv log messages should have both date and time */
	/*m_NCS_TIME_TO_STR(tod, asc_tod); */
	m_NCS_DATE_TIME_TO_STR(tod, asc_tod);

	sprintf((asc_tod + strlen(asc_tod)), ".%.3d", msg->hdr.time.millisecs);

	fmat = &spec->fmat_set[msg->hdr.fmat_id];
	set = spec->str_set;

	/* The following IF conditionals make sure passed info is valid */
	if (msg->hdr.fmat_id > spec->fmat_set_cnt) {
		m_MMGR_FREE_OCT(msg->hdr.fmat_type);
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_LOG_ERR1, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE,
			   "TILLLC", DTS_INVALID_FMAT, msg->hdr.fmat_id, spec->fmat_set_cnt, msg->hdr.ss_id,
			   spec->svc_name);
		return NCSCC_RC_FAILURE;
	}

	if (fmat->fmat_id != msg->hdr.fmat_id) {
		m_MMGR_FREE_OCT(msg->hdr.fmat_type);
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_LOG_ERR1, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE,
			   "TILLLC", DTS_FMAT_ID_MISMATCH, msg->hdr.fmat_id, fmat->fmat_id, msg->hdr.ss_id,
			   spec->svc_name);
		return NCSCC_RC_FAILURE;
	}

	if (0 != strcmp(fmat->fmat_type, msg->hdr.fmat_type)) {
		m_MMGR_FREE_OCT(msg->hdr.fmat_type);
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_LOG_ERR, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILLC",
			   DTS_FMAT_TYPE_MISMATCH, fmat->fmat_id, msg->hdr.ss_id, spec->svc_name);
		return NCSCC_RC_FAILURE;
	}

	m_MMGR_FREE_OCT(msg->hdr.fmat_type);

	if (fmat->fmat_str == NULL) {
		return m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE,
					       "dts_log_msg_to_str: Format string is NULL in ASCII_SPEC table for the received format ID.",
					       spec->svc_name);
	}

	/*
	 * First get the severity of the message.
	 */
	switch (msg->hdr.severity) {
	case NCSFL_SEV_EMERGENCY:
		sev = "EMRGCY : ";
		break;
	case NCSFL_SEV_ALERT:
		sev = "ALERT! : ";
		break;
	case NCSFL_SEV_CRITICAL:
		sev = "CRITCL : ";
		break;
	case NCSFL_SEV_ERROR:
		sev = "ERROR  : ";
		break;
	case NCSFL_SEV_WARNING:
		sev = "WRNING : ";
		break;
	case NCSFL_SEV_NOTICE:
		sev = "NOTICE : ";
		break;
	case NCSFL_SEV_INFO:
		sev = "INFORM : ";
		break;
	case NCSFL_SEV_DEBUG:
		sev = "DEBUG  : ";
		break;
	default:
		m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE, "dts_log_msg_to_str: Message Severity is wrong",
					spec->svc_name);
		sev = "SUBSYS ERR";
		break;
	}

	/* 
	 * Now first copy Message Severity and 
	 * Node ID from where we got this log message.
	 */
	/* Node-id display change */
	*len += (log_msg_len = sprintf(str, "%s 0x%08x %u %d %d ", sev, node, proc_id,
				       msg->hdr.ss_id, msg->hdr.inst_id));

	str += log_msg_len;

/****************************************************************************

  Below is set of generic formats understood by FlexLog so far. Other
  subsystems will add to this set as new data layouts come about..

****************************************************************************/
	if (NCSCC_RC_SUCCESS != dts_get_and_return_val(temp_str,
						       fmat->fmat_type, asc_tod, &msg->uba, spec, set, args,
						       &log_msg_len, logmsg->msg_fmat_ver)) {
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_LOG_ERR, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILLC",
			   DTS_LOG_CONV_FAIL, 0, msg->hdr.ss_id, spec->svc_name);
		return NCSCC_RC_FAILURE;
	}

	/* Return length of string to the calling function for console printing */
	*len += sprintf(str, fmat->fmat_str,
			args[0], args[1], args[2], args[3], args[4], args[5], args[6],
			args[7], args[8], args[9], args[10], args[11], args[12], args[13],
			args[14], args[15], args[16], args[17], args[18], args[19], args[20]);

	return NCSCC_RC_SUCCESS;
}

static uint32_t dts_get_and_return_val(char *t_str, char *ch, char *time,
				    NCS_UBAID *uba, NCSFL_ASCII_SPEC *spec,
				    NCSFL_SET *set, long *arg_list, uint32_t *log_msg_len, uint8_t msg_fmat_ver)
{
	uint8_t *data = NULL;
	uint8_t data_buff[DTS_MAX_SIZE_DATA];
	uint32_t cnt = 0;

	while (*ch != '\0') {

		switch (*ch) {
		case 'T':
			{
				arg_list[cnt] = NCS_PTR_TO_UNS64_CAST(time);
				*log_msg_len += 15;
				break;
			}
		case 'I':
			{
				uint32_t idx;
				data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
				if (data == NULL)
					return NCSCC_RC_FAILURE;

				idx = ncs_decode_32bit(&data);
				ncs_dec_skip_space(uba, sizeof(uint32_t));
				if (m_S_SET(idx) >= spec->str_set_cnt) {
					ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_LOG_ERR1, DTS_FC_EVT, NCSFL_LC_EVENT,
						   NCSFL_SEV_NOTICE, "TILLLC", DTS_LOG_SETIDX_ERR, m_S_SET(idx),
						   spec->str_set_cnt, spec->ss_id, spec->svc_name);
					return NCSCC_RC_FAILURE;
				}
				if (m_S_STR(idx) >= spec->str_set[m_S_SET(idx)].set_cnt) {
					ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_LOG_ERR1, DTS_FC_EVT, NCSFL_LC_EVENT,
						   NCSFL_SEV_NOTICE, "TILLLC", DTS_LOG_STRIDX_ERR, m_S_STR(idx),
						   spec->str_set[m_S_SET(idx)].set_cnt, spec->ss_id, spec->svc_name);
					return NCSCC_RC_FAILURE;
				}

				arg_list[cnt] = NCS_PTR_TO_UNS64_CAST(m_NCSFL_MAKE_STR(set, idx));
				*log_msg_len += strlen((char *)arg_list[cnt]);

				break;
			}
		case 'L':
			{
				uint32_t lval;

				data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
				if (data == NULL)
					return NCSCC_RC_FAILURE;

				lval = ncs_decode_32bit(&data);
				ncs_dec_skip_space(uba, sizeof(uint32_t));

				arg_list[cnt] = (long)lval;
				*log_msg_len += 10;
				break;
			}
		case 'M':
			{
				uint16_t u16_val;

				data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint16_t));
				if (data == NULL)
					return NCSCC_RC_FAILURE;

				u16_val = ncs_decode_16bit(&data);
				ncs_dec_skip_space(uba, sizeof(uint16_t));

				arg_list[cnt] = (long)u16_val;
				*log_msg_len += 10;
				break;
			}
		case 'S':
			{
				uint8_t u8_val;

				data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint8_t));
				if (data == NULL)
					return NCSCC_RC_FAILURE;

				u8_val = ncs_decode_8bit(&data);
				ncs_dec_skip_space(uba, sizeof(uint8_t));

				arg_list[cnt] = (long)u8_val;
				*log_msg_len += 3;
				break;
			}
		case 'D':
			{
				NCSFL_MEM mem_d;
				/* Added for printing 64-bit addresses */
				uint64_t mem_addr = 0;

				/* Support for DTA version 1 */
				if (msg_fmat_ver == 1) {
					data = ncs_dec_flatten_space(uba, data_buff, (sizeof(uint16_t) + sizeof(uint32_t)));
					if (data == NULL)
						return NCSCC_RC_FAILURE;

					mem_d.len = ncs_decode_16bit(&data);
					mem_addr = ncs_decode_32bit(&data);
					ncs_dec_skip_space(uba, (sizeof(uint16_t) + sizeof(uint32_t)));
				}
				/* Versioning changes : New code for 64-bit support */
				else {
					assert(msg_fmat_ver == 2);
					data = ncs_dec_flatten_space(uba, data_buff, (sizeof(uint16_t) + sizeof(uint64_t)));
					if (data == NULL)
						return NCSCC_RC_FAILURE;

					mem_d.len = ncs_decode_16bit(&data);
					mem_addr = ncs_decode_64bit(&data);
					ncs_dec_skip_space(uba, (sizeof(uint16_t) + sizeof(uint64_t)));
				}

				mem_d.dump = m_MMGR_ALLOC_OCT(mem_d.len);
				if (mem_d.dump == NULL) {
					return m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE,
								       "dts_log_msg_decode: Memory allocation failed",
								       spec->svc_name);
				}
				memset(mem_d.dump, '\0', mem_d.len);
				ncs_decode_n_octets_from_uba(uba, (uint8_t *)mem_d.dump, mem_d.len);

				{
					m_NCSFL_MAKE_STR_FROM_MEM_64(t_str, (mem_addr),
								     (mem_d.len), (uint8_t *)(mem_d.dump));
				}

				m_MMGR_FREE_OCT(mem_d.dump);

				arg_list[cnt] = (long)t_str;
				*log_msg_len += (strlen(t_str) + 1);
				t_str += (*log_msg_len);
				break;
			}
		case 'P':
			{
				NCSFL_PDU pdu;

				data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint16_t));
				if (data == NULL)
					return NCSCC_RC_FAILURE;

				pdu.len = ncs_decode_16bit(&data);
				ncs_dec_skip_space(uba, sizeof(uint16_t));

				pdu.dump = m_MMGR_ALLOC_OCT(pdu.len);
				if (pdu.dump == NULL) {
					return m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE,
								       "dts_log_msg_decode: Memory allocation failed",
								       spec->svc_name);
				}
				memset(pdu.dump, '\0', pdu.len);
				ncs_decode_n_octets_from_uba(uba, (uint8_t *)pdu.dump, pdu.len);

				m_NCSFL_MAKE_STR_FROM_PDU(t_str, pdu.len, (uint8_t *)pdu.dump);

				m_MMGR_FREE_OCT(pdu.dump);

				arg_list[cnt] = NCS_PTR_TO_UNS64_CAST(t_str);

				*log_msg_len += (strlen(t_str) + 1);
				t_str += (*log_msg_len);
				break;
			}
		case 'A':
			{
				NCS_IP_ADDR ipa;
				decode_ip_address(uba, &ipa);
				m_NCSFL_MAKE_STR_FRM_IPADDR(ipa, t_str);
				arg_list[cnt] = NCS_PTR_TO_UNS64_CAST(t_str);
				*log_msg_len += (strlen(t_str) + 1);
				t_str += (*log_msg_len);
				break;
			}
		case 'C':
			{
				uint32_t length;
				uint8_t *data;
				uint8_t data_buff[DTS_MAX_SIZE_DATA];

				data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
				if (data == NULL)
					return NCSCC_RC_FAILURE;

				length = ncs_decode_32bit(&data);
				ncs_dec_skip_space(uba, sizeof(uint32_t));

				ncs_decode_n_octets_from_uba(uba, (uint8_t *)t_str, length);

				arg_list[cnt] = NCS_PTR_TO_UNS64_CAST(t_str);
				t_str += (strlen(t_str) + 1);
				*log_msg_len += length;
				break;
			}
			/* Added code for handling float values */
		case 'F':
			{
				uint32_t length;
				uint8_t *data;
				uint8_t data_buff[DTS_MAX_SIZE_DATA];

				data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
				if (data == NULL)
					return NCSCC_RC_FAILURE;

				length = ncs_decode_32bit(&data);
				ncs_dec_skip_space(uba, sizeof(uint32_t));

				ncs_decode_n_octets_from_uba(uba, (uint8_t *)t_str, length);
				arg_list[cnt] = NCS_PTR_TO_UNS64_CAST(t_str);
				t_str += (strlen(t_str) + 1);
				*log_msg_len += length;
				break;
			}
		case 'N':
			{
				uint32_t length;
				uint8_t *data;
				uint8_t data_buff[DTS_MAX_DBL_DIGITS];

				data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
				if (data == NULL)
					return NCSCC_RC_FAILURE;

				length = ncs_decode_32bit(&data);
				ncs_dec_skip_space(uba, sizeof(uint32_t));

				ncs_decode_n_octets_from_uba(uba, (uint8_t *)t_str, length);
				arg_list[cnt] = NCS_PTR_TO_UNS64_CAST(t_str);
				t_str += (strlen(t_str) + 1);
				*log_msg_len += length;
				break;
			}
		case 'U':
			{
				uint32_t length;
				uint8_t *data;
				uint8_t data_buff[DTS_MAX_DBL_DIGITS];

				data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
				if (data == NULL)
					return NCSCC_RC_FAILURE;

				length = ncs_decode_32bit(&data);
				ncs_dec_skip_space(uba, sizeof(uint32_t));

				ncs_decode_n_octets_from_uba(uba, (uint8_t *)t_str, length);
				arg_list[cnt] = NCS_PTR_TO_UNS64_CAST(t_str);
				t_str += (strlen(t_str) + 1);
				*log_msg_len += length;
				break;
			}
		case 'X':
			{
				uint32_t length;
				uint8_t *data;
				uint8_t data_buff[DTS_MAX_DBL_DIGITS];

				data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
				if (data == NULL)
					return NCSCC_RC_FAILURE;

				length = ncs_decode_32bit(&data);
				ncs_dec_skip_space(uba, sizeof(uint32_t));

				ncs_decode_n_octets_from_uba(uba, (uint8_t *)t_str, length);
				arg_list[cnt] = NCS_PTR_TO_UNS64_CAST(t_str);
				t_str += (strlen(t_str) + 1);
				*log_msg_len += length;
				break;
			}
		default:
			{
				return m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE,
							       "Unable to detect format type", spec->svc_name);
			}
		}
		ch++;
		cnt++;

		if (*log_msg_len > SYSF_FL_LOG_SIZE)
			return m_DTS_DBG_SINK_SVC_NAME(NCSCC_RC_FAILURE,
						       "Log message exceeds the max length.", spec->svc_name);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    dts_to_lowercase 

  DESCRIPTION:       Converts string to lowercase.

*****************************************************************************/
void dts_to_lowercase(char *str)
{
	int i = 0;
	while (str[i]) {
		str[i] = tolower(str[i]);
		i++;
	}
	return;
}
