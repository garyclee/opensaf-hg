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

  DESCRIPTION:This module has all the functionality that deals with
  the DTSv modules agent for logging debug messages.


..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_flx_log_reg - registers the AVD logging with DTA.
  avd_flx_log_dereg - unregisters the AVD logging with DTA.


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"
#if (NCS_AVD_LOG == 1)


/****************************************************************************
  Name          : avd_log_admin_state_traps

  Description   : This routine logs the admin state related traps.

  Arguments     : State    - Admin State
                  name_net - ptr to the name (node/su/sg/si name)
                  sev      - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void  avd_log_admin_state_traps(AVD_ADMIN_STATE_FLEX state,
                                SaNameT *name_net,
                                uns8    sev)
{
   uns8 name[SA_MAX_NAME_LENGTH];

   m_NCS_OS_MEMSET(name, '\0', SA_MAX_NAME_LENGTH);

   /* convert name into string format */
   if (name_net)
      m_NCS_STRNCPY(name, name_net->value, m_NCS_OS_NTOHS(name_net->length));

   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_ADMIN, AVD_FC_ADMIN,
              NCSFL_LC_HEADLINE, sev, "TCI", name, state);

   return;
}

/****************************************************************************
  Name          : avd_log_si_unassigned_trap

  Description   : This routine logs the si unassigned related traps.

  Arguments     : State    - State
                  name_net - ptr to the si name
                  sev      - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void  avd_log_si_unassigned_trap(AVD_TRAP_FLEX state,
                                SaNameT *name_net,
                                uns8    sev)
{
   uns8 name[SA_MAX_NAME_LENGTH];

   m_NCS_OS_MEMSET(name, '\0', SA_MAX_NAME_LENGTH);

   /* convert name into string format */
   if (name_net)
      m_NCS_STRNCPY(name, name_net->value, m_NCS_OS_NTOHS(name_net->length));

   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_SI_UNASSIGN, AVD_FC_TRAP,
              NCSFL_LC_HEADLINE, sev, "TCI", name, state);

   return;
}

/****************************************************************************
  Name          : avd_log_oper_state_traps

  Description   : This routine logs the oper state related traps.

  Arguments     : State    - oper State
                  name_net - ptr to the name 
                  sev      - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void  avd_log_oper_state_traps(AVD_OPER_STATE_FLEX state,
                               SaNameT *name_net,
                               uns8    sev)
{
   uns8 name[SA_MAX_NAME_LENGTH];

   m_NCS_OS_MEMSET(name, '\0', SA_MAX_NAME_LENGTH);

   /* convert name into string format */
   if (name_net)
      m_NCS_STRNCPY(name, name_net->value, m_NCS_OS_NTOHS(name_net->length));

   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_OPER, AVD_FC_OPER,
              NCSFL_LC_HEADLINE, sev, "TCI", name, state);

   return;
}


/****************************************************************************
  Name          : avd_log_clm_node_traps

  Description   : This routine logs the clm node related traps.

  Arguments     : op    - Operation
                  cl    - "Cluster"
                  name_net - ptr to the node name
                  sev      - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void  avd_log_clm_node_traps(AVD_TRAP_FLEX cl,
                             AVD_TRAP_FLEX op,
                             SaNameT *name_net,
                             uns8    sev)
{
   uns8 name[SA_MAX_NAME_LENGTH];

   m_NCS_OS_MEMSET(name, '\0', SA_MAX_NAME_LENGTH);

   /* convert name into string format */
   if (name_net)
      m_NCS_STRNCPY(name, name_net->value, m_NCS_OS_NTOHS(name_net->length));

   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_CLM, AVD_FC_TRAP,
              NCSFL_LC_HEADLINE, sev, "TCII", name, op, cl);

   return;
}


/****************************************************************************
  Name          : avd_log_susi_ha_traps

  Description   : This routine logs the SUSI HA related traps.

  Arguments     : state         - HA State
                  su_name_net   - ptr to the su name
                  si_name_net   - ptr to the si name
                  sev           - severity
                  isStateChanged- Flag to specify that this routine is called in context of 
                                  HA State changed or HA Sate changing trap.

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avd_log_susi_ha_traps (AVD_HA_STATE_FLEX state,
                            SaNameT *su_name_net,
                            SaNameT *si_name_net,
                            uns8    sev,
                            NCS_BOOL isStateChanged)
{
   uns8 su_name[SA_MAX_NAME_LENGTH];
   uns8 si_name[SA_MAX_NAME_LENGTH];

   m_NCS_OS_MEMSET(su_name, '\0', SA_MAX_NAME_LENGTH);
   m_NCS_OS_MEMSET(si_name, '\0', SA_MAX_NAME_LENGTH);

   /* convert name into string format */
   if (su_name_net)
      m_NCS_STRNCPY(su_name, su_name_net->value, m_NCS_OS_NTOHS(su_name_net->length));
   
   /* convert name into string format */
   if (si_name_net)
      m_NCS_STRNCPY(si_name, si_name_net->value, m_NCS_OS_NTOHS(si_name_net->length));

   if(isStateChanged)
      ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_SUSI_HA, AVD_FC_SUSI_HA,
              NCSFL_LC_HEADLINE, sev, "TCCI", su_name, si_name, state);
   else
      ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_SUSI_HA_CHG_START, AVD_FC_SUSI_HA,
              NCSFL_LC_HEADLINE, sev, "TCCI", su_name, si_name, state);

   return;
}


/****************************************************************************
 * Name          : avd_flx_log_reg
 *
 * Description   : This is the function which registers the AVD logging with
 *                 the Flex Log agent.
 *                 
 *
 * Arguments     : None.
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
avd_flx_log_reg ()
{
   NCS_DTSV_RQ            reg;

   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_AVD;
   /* fill version no. */
   reg.info.bind_svc.version = AVSV_LOG_VERSION;
   /* fill svc_name */
   m_NCS_STRCPY(reg.info.bind_svc.svc_name, "AvSv");

   ncs_dtsv_su_req(&reg);
   return;
}

/****************************************************************************
 * Name          : avd_flx_log_dereg
 *
 * Description   : This is the function which deregisters the AVD logging 
 *                 with the Flex Log agent.
 *                 
 *
 * Arguments     : None.
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
avd_flx_log_dereg ()
{
   NCS_DTSV_RQ        reg;
   
   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                   = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_AVD;
   ncs_dtsv_su_req(&reg);
   return;
}

#endif /* (NCS_AVD_LOG == 1) */
