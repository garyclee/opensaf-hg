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
  
  This file contains AvND logging utility routines.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

#include "avnd.h"

#if ( NCS_AVND_LOG == 1 )

void _avnd_log(uns8 severity, const char *function, const char *format, ...)
{
        char preamble[128];
        char str[128];
        va_list ap;

        va_start(ap, format);
        snprintf(preamble, sizeof(preamble), "%s - %s", function, format);
        vsnprintf(str, sizeof(str), preamble, ap);
        va_end(ap);

        ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_GENLOG, AVND_FC_GENLOG,
		   NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TC, str);
}

void _avnd_trace(const char *file, unsigned int line, const char *format, ...)
{
        char preamble[64];
        char str[128];
        va_list ap;

        va_start(ap, format);
        snprintf(preamble, sizeof(preamble), "%s:%04u %s", file, line, format);
        vsnprintf(str, sizeof(str), preamble, ap);
        va_end(ap);

        ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_GENLOG, AVND_FC_GENLOG,
		   NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, NCSFL_TYPE_TC, str);
}

/****************************************************************************
  Name          : avnd_log_seapi
 
  Description   : This routine logs the SE-API info.
                  
  Arguments     : op     - se-api operation
                  status - status of se-api operation execution
                  sev    - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_seapi(AVSV_LOG_SEAPI op, AVSV_LOG_SEAPI status, uns8 sev)
{
	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_SEAPI, AVND_FC_SEAPI,
		   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

	return;
}

/****************************************************************************
  Name          : avnd_log_mds
 
  Description   : This routine logs the MDS info.
                  
  Arguments     : op     - mds operation
                  status - status of mds operation execution
                  sev    - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_mds(AVSV_LOG_MDS op, AVSV_LOG_MDS status, uns8 sev)
{
	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_MDS, AVND_FC_MDS, NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

	return;
}

/****************************************************************************
  Name          : avnd_log_edu
 
  Description   : This routine logs the EDU info.
                  
  Arguments     : op     - edu operation
                  status - status of edu operation execution
                  sev    - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_edu(AVSV_LOG_EDU op, AVSV_LOG_EDU status, uns8 sev)
{
	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_EDU, AVND_FC_EDU, NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

	return;
}

/****************************************************************************
  Name          : avnd_log_lock
 
  Description   : This routine logs the LOCK info.
                  
  Arguments     : op     - lock operation
                  status - status of lock operation execution
                  sev    - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_lock(AVSV_LOG_LOCK op, AVSV_LOG_LOCK status, uns8 sev)
{
	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_LOCK, AVND_FC_LOCK, NCSFL_LC_LOCKS, sev, NCSFL_TYPE_TII, op, status);

	return;
}

/****************************************************************************
  Name          : avnd_log_cb
 
  Description   : This routine logs the control block info.
                  
  Arguments     : op     - cb operation
                  status - status of cb operation execution
                  sev    - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_cb(AVSV_LOG_CB op, AVSV_LOG_CB status, uns8 sev)
{
	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_CB, AVND_FC_CB, NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

	return;
}

/****************************************************************************
  Name          : avnd_log_mbx
 
  Description   : This routine logs the mailbox info.
                  
  Arguments     : op     - mbx operation
                  status - status of operation execution
                  sev    - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_mbx(AVSV_LOG_MBX op, AVSV_LOG_MBX status, uns8 sev)
{
	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_MBX, AVND_FC_MBX, NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

	return;
}

/****************************************************************************
  Name          : avnd_log_task
 
  Description   : This routine logs the task info.
                  
  Arguments     : op     - task operation
                  status - status of operation execution
                  sev    - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_task(AVSV_LOG_TASK op, AVSV_LOG_TASK status, uns8 sev)
{
	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_TASK, AVND_FC_TASK,
		   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

	return;
}

/****************************************************************************
  Name          : avnd_log_tmr
 
  Description   : This routine logs the timer info.
                  
  Arguments     : type   - timer type
                  op     - timer operation
                  status - status of the operation
                  sev    - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_tmr(AVND_LOG_TMR type, AVND_LOG_TMR op, AVND_LOG_TMR status, uns8 sev)
{
	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_TMR, AVND_FC_TMR,
		   NCSFL_LC_TIMER, sev, NCSFL_TYPE_TIII, type - 1, op, status);

	return;
}

/****************************************************************************
  Name          : avnd_log_evt
 
  Description   : This routine logs the event info.
                  
  Arguments     : type   - event type
                  op     - event operation
                  status - status of the operation
                  sev    - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_evt(AVND_LOG_EVT type, AVND_LOG_EVT op, AVND_LOG_EVT status, uns8 sev)
{
	if (!op && !status) {
		ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_EVT1, AVND_FC_EVT,
			   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, type - 1);
	}

	if (!op && status) {
		ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_EVT2, AVND_FC_EVT,
			   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, type - 1, status);
	}

	if (op && status) {
		ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_EVT3, AVND_FC_EVT,
			   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIII, type - 1, op, status);
	}

	return;
}

/****************************************************************************
  Name          : avnd_log_clm_db
 
  Description   : This routine logs the CLM database info.
                  
  Arguments     : op      - db operation
                  status  - status of the operation
                  node_id - node-id that's added / deleted
                  sev     - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_clm_db(AVND_LOG_CLM_DB op, AVND_LOG_CLM_DB status, SaClmNodeIdT node_id, uns8 sev)
{
	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_CLM_DB, AVND_FC_CLM_DB,
		   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIIL, op, status, node_id);

	return;
}

/****************************************************************************
  Name          : avnd_log_pg_db
 
  Description   : This routine logs the PG database info.
                  
  Arguments     : op       - db operation
                  status   - status of the operation
                  csi_name - ptr to the csi-name that is added / deleted
                  sev      - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_pg_db(AVND_LOG_PG_DB op, AVND_LOG_PG_DB status, SaNameT *csi_name, uns8 sev)
{
	uns8 csi[SA_MAX_NAME_LENGTH];

	memset(csi, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (csi_name)
		strncpy(csi, csi_name->value, m_NCS_OS_NTOHS(csi_name->length));

	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_PG_DB, AVND_FC_PG_DB,
		   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIIC, op, status, csi);

	return;
}

/****************************************************************************
  Name          : avnd_log_su_db
 
  Description   : This routine logs the SU database info.
                  
  Arguments     : op      - db operation
                  status  - status of the operation
                  su_name - ptr to the su name that's added / deleted
                  si_name - ptr to the si-name that's assigned / removed
                  sev     - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_su_db(AVND_LOG_SU_DB op, AVND_LOG_SU_DB status, SaNameT *su_name, SaNameT *si_name, uns8 sev)
{
	uns8 su[SA_MAX_NAME_LENGTH];
	uns8 si[SA_MAX_NAME_LENGTH];

	memset(su, '\0', SA_MAX_NAME_LENGTH);
	memset(si, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (su_name)
		strncpy(su, su_name->value, m_NCS_OS_NTOHS(su_name->length));

	if (si_name)
		strncpy(si, si_name->value, m_NCS_OS_NTOHS(si_name->length));

	if ((AVND_LOG_SU_DB_SI_ADD == op) ||
	    (AVND_LOG_SU_DB_SI_DEL == op) || (AVND_LOG_SU_DB_SI_ASSIGN == op) || (AVND_LOG_SU_DB_SI_REMOVE == op)) {
		ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_SU_DB2, AVND_FC_SU_DB,
			   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIICC, op, status, su, si);
	} else {
		ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_SU_DB1, AVND_FC_SU_DB,
			   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIIC, op, status, su);
	}

	return;
}

/****************************************************************************
  Name          : avnd_log_comp_db
 
  Description   : This routine logs the Component database info.
                  
  Arguments     : op        - db operation
                  status    - status of the operation
                  comp_name - ptr to the comp name that's added / deleted
                  csi_name  - ptr to the csi-name that's assigned / removed
                  sev       - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_comp_db(AVND_LOG_COMP_DB op, AVND_LOG_COMP_DB status, SaNameT *comp_name, SaNameT *csi_name, uns8 sev)
{
	uns8 comp[SA_MAX_NAME_LENGTH];
	uns8 csi[SA_MAX_NAME_LENGTH];

	memset(comp, '\0', SA_MAX_NAME_LENGTH);
	memset(csi, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (comp_name)
		strncpy(comp, comp_name->value, m_NCS_OS_NTOHS(comp_name->length));

	if (csi_name)
		strncpy(csi, csi_name->value, m_NCS_OS_NTOHS(csi_name->length));

	if ((AVND_LOG_COMP_DB_CSI_ADD == op) ||
	    (AVND_LOG_COMP_DB_CSI_DEL == op) ||
	    (AVND_LOG_COMP_DB_CSI_ASSIGN == op) || (AVND_LOG_COMP_DB_CSI_REMOVE == op)) {
		ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_COMP_DB2, AVND_FC_COMP_DB,
			   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIICC, op, status, comp, csi);
	} else {
		ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_COMP_DB1, AVND_FC_COMP_DB,
			   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIIC, op, status, comp);
	}

	return;
}

/****************************************************************************
  Name          : avnd_pxy_pxd_log
 
  Description   : This routine logs the proxy-proxied communication.
                  
  Arguments     : sev       - Severity
                  index     - Index 
                  info      - Ptr to general info abt logs (Strings)
                  comp_name - Ptr to the comp name, if any.
                  Info1     - Value corresponding to Info description 
                  Info2     - Value corresponding to Info description 
                  Info3     - Value corresponding to Info description 
                  Info4     - Value corresponding to Info description 
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_pxy_pxd_log(uns32 sev,
		      uns32 index, uns8 *info, SaNameT *comp_name, uns32 info1, uns32 info2, uns32 info3, uns32 info4)
{
	uns8 comp[SA_MAX_NAME_LENGTH];

	memset(comp, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (comp_name)
		strncpy(comp, comp_name->value, m_NCS_OS_NTOHS(comp_name->length));

	ncs_logmsg(NCS_SERVICE_ID_AVND, (uns8)AVND_AVND_MSG, (uns8)AVND_FC_AVND_MSG,
		   NCSFL_LC_HEADLINE, (uns8)sev, NCSFL_TYPE_TICCLLLL, index, info, comp, info1, info2, info3, info4);
	return;
}

/****************************************************************************
  Name          : avnd_log_hc_db
 
  Description   : This routine logs the healthcheck database info.
                  
  Arguments     : op     - db operation
                  status - status of the operation
                  hc_key - ptr to the hc-key that's added / deleted
                  sev    - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_hc_db(AVND_LOG_HC_DB op, AVND_LOG_HC_DB status, SaAmfHealthcheckKeyT *hc_key, uns8 sev)
{
	uns8 key[SA_AMF_HEALTHCHECK_KEY_MAX];

	memset(key, '\0', SA_AMF_HEALTHCHECK_KEY_MAX);

	/* convert key into string format */
	if (hc_key)
		strncpy(key, hc_key->key, hc_key->keyLen);

	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HC_DB, AVND_FC_HC_DB,
		   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIIC, op, status, key);

	return;
}

/****************************************************************************
  Name          : avnd_log_su_fsm
 
  Description   : This routine logs the SU FSM info.
                  
  Arguments     : op          - fsm operation
                  st          - fsm state
                  ev          - fsm event
                  su_name_net - ptr to the su name
                  sev         - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_su_fsm(AVND_LOG_SU_FSM op, NCS_PRES_STATE st, AVND_SU_PRES_FSM_EV ev, SaNameT *su_name_net, uns8 sev)
{
	uns8 name[SA_MAX_NAME_LENGTH];

	memset(name, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (su_name_net)
		strncpy(name, su_name_net->value, m_NCS_OS_NTOHS(su_name_net->length));

	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_SU_FSM, AVND_FC_SU_FSM,
		   NCSFL_LC_STATE_MCH, sev, NCSFL_TYPE_TIIIC, op, (st - 1),
		   (ev) ? (ev + AVND_LOG_SU_FSM_EV_NONE) : (AVND_LOG_SU_FSM_EV_NONE), name);

	return;
}

/****************************************************************************
  Name          : avnd_log_comp_fsm
 
  Description   : This routine logs the component FSM info.
                  
  Arguments     : op            - fsm operation
                  st            - fsm state
                  ev            - fsm event
                  comp_name_net - ptr to the comp name
                  sev           - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_comp_fsm(AVND_LOG_COMP_FSM op,
		       NCS_PRES_STATE st, AVND_COMP_CLC_PRES_FSM_EV ev, SaNameT *comp_name_net, uns8 sev)
{
	uns8 name[SA_MAX_NAME_LENGTH];

	memset(name, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (comp_name_net)
		strncpy(name, comp_name_net->value, m_NCS_OS_NTOHS(comp_name_net->length));

	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_COMP_FSM, AVND_FC_COMP_FSM,
		   NCSFL_LC_STATE_MCH, sev, NCSFL_TYPE_TIIIC, op, (st - 1),
		   (ev) ? (ev + AVND_LOG_COMP_FSM_EV_NONE) : (AVND_LOG_COMP_FSM_EV_NONE), name);

	return;
}

/****************************************************************************
  Name          : avnd_log_err
 
  Description   : This routine logs the errors.
                  
  Arguments     : src          - source 
                  rec          - recovery
                  comp_name_net - ptr to the comp name
                  sev           - severity
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_err(AVND_LOG_ERR src, AVND_LOG_ERR rec, SaNameT *comp_name_net, uns8 sev)
{
	uns8 name[SA_MAX_NAME_LENGTH];

	memset(name, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (comp_name_net)
		strncpy(name, comp_name_net->value, m_NCS_OS_NTOHS(comp_name_net->length));

	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_ERR, AVND_FC_ERR,
		   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIIC, src, rec, name);

	return;
}

/****************************************************************************
  Name          : avnd_log_misc
 
  Description   : This routine logs miscellaneous info.
                  
  Arguments     : op  - misc operation
                  sev - severity
                  sa_name - ptr to the comp/su name
 
  Return Values : None
 
  Notes         : None.
 *****************************************************************************/
void avnd_log_misc(AVND_LOG_MISC op, SaNameT *sa_name, uns8 sev)
{
	uns8 name[SA_MAX_NAME_LENGTH];

	memset(name, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (sa_name)
		strncpy(name, sa_name->value, m_NCS_OS_NTOHS(sa_name->length));

	if (!sa_name) {
		ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_MISC1, AVND_FC_MISC,
			   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, op);
	} else {
		ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_MISC2, AVND_FC_MISC,
			   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIC, op + AVND_LOG_MISC_AVA_DN, name);
	}

	return;
}

/****************************************************************************
  Name          : avnd_log_clc_ntfs

  Description   : This routine logs the clc related ntfs.

  Arguments     : op        - operation (Instantiation/Termination/Cleanup)
                  status    - status of the operation (Failed)
                  comp_name - ptr to the comp name 
                  sev       - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avnd_log_clc_ntfs(AVND_LOG_NTF op, AVND_LOG_NTF status, SaNameT *comp_name, uns8 sev)
{
	uns8 comp[SA_MAX_NAME_LENGTH];

	memset(comp, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (comp_name)
		strncpy(comp, comp_name->value, m_NCS_OS_NTOHS(comp_name->length));

	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_NTFS_CLC, AVND_FC_NTF,
		   NCSFL_LC_HEADLINE, sev, "TCII", comp, op, status);

	return;
}

/****************************************************************************
  Name          : avnd_log_su_oper_state_ntfs

  Description   : This routine logs the su oper related ntfs.

  Arguments     : State   - SU Oper State
                  su_name - ptr to the su name
                  sev     - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avnd_log_su_oper_state_ntfs(AVND_LOG_OPER_STATE state, SaNameT *su_name, uns8 sev)
{
	uns8 su[SA_MAX_NAME_LENGTH];

	memset(su, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (su_name)
		strncpy(su, su_name->value, m_NCS_OS_NTOHS(su_name->length));

	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_NTFS_OPER_STAT, AVND_FC_OPER,
		   NCSFL_LC_HEADLINE, sev, "TCI", su, state);

	return;
}

/****************************************************************************
  Name          : avnd_log_su_pres_state_ntfs

  Description   : This routine logs the su pres related ntfs.

  Arguments     : State   - SU Pres State
                  su_name - ptr to the su name
                  sev     - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avnd_log_su_pres_state_ntfs(AVND_LOG_PRES_STATE state, SaNameT *su_name, uns8 sev)
{
	uns8 su[SA_MAX_NAME_LENGTH];

	memset(su, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (su_name)
		strncpy(su, su_name->value, m_NCS_OS_NTOHS(su_name->length));

	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_NTFS_PRES_STAT, AVND_FC_PRES,
		   NCSFL_LC_HEADLINE, sev, "TCI", su, state);

	return;
}

/****************************************************************************
  Name          : avnd_log_proxied_orphaned_ntfs

  Description   : This routine logs the proxied orphaned ntfs.

                  status        - status of proxied component(Orphaned)
                  pxy_status    - status of proxy component  (Failed)
                  comp_name     - ptr to the proxied comp name
                  pxy_comp_name - ptr to the proxy comp name
                  sev           - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avnd_log_proxied_orphaned_ntfs(AVND_LOG_NTF status,
				    AVND_LOG_NTF pxy_status, SaNameT *comp_name, SaNameT *pxy_comp_name, uns8 sev)
{
	uns8 comp[SA_MAX_NAME_LENGTH];
	uns8 pxy_comp[SA_MAX_NAME_LENGTH];

	memset(comp, '\0', SA_MAX_NAME_LENGTH);
	memset(pxy_comp, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (comp_name)
		strncpy(comp, comp_name->value, m_NCS_OS_NTOHS(comp_name->length));

	if (pxy_comp_name)
		strncpy(pxy_comp, pxy_comp_name->value, m_NCS_OS_NTOHS(pxy_comp_name->length));

	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_NTFS_PROXIED, AVND_FC_NTF,
		   NCSFL_LC_HEADLINE, sev, "TCICI", comp, status, pxy_comp, pxy_status);

	return;
}

/****************************************************************************
  Name          : avnd_log_comp_failed_ntfs

  Description   : This routine logs the component failed ntfs.

  Arguments     : node_id  - Node Id
                  name_net - Comp name
                  errSrc   - Error Source
                  sev      - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avnd_log_comp_failed_ntfs(uns32 node_id, SaNameT *name_net, AVND_LOG_ERR errSrc, uns8 sev)
{
	uns8 name[SA_MAX_NAME_LENGTH];

	memset(name, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (name_net)
		strncpy(name, name_net->value, m_NCS_OS_NTOHS(name_net->length));

	ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_NTFS_COMP_FAIL, AVND_FC_ERR,
		   NCSFL_LC_HEADLINE, sev, "TCLI", name, node_id, errSrc);

	return;
}

/****************************************************************************
  Name          : avnd_log_reg
 
  Description   : This routine registers AvND with flex log service.
                  
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
 *****************************************************************************/
uns32 avnd_log_reg()
{
	NCS_DTSV_RQ reg;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));

	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_AVND;
	/* fill version no. */
	reg.info.bind_svc.version = AVSV_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "AvSv");

	rc = ncs_dtsv_su_req(&reg);

	return rc;
}

/****************************************************************************
  Name          : avnd_log_unreg
 
  Description   : This routine unregisters AvND with flex log service.
                  
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
 *****************************************************************************/
uns32 avnd_log_unreg()
{
	NCS_DTSV_RQ reg;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));

	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_AVND;

	rc = ncs_dtsv_su_req(&reg);

	return rc;
}

#endif   /* NCS_AVND_LOG == 1 */
