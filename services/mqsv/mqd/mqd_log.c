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

  MODULE: MQDLOG.C

  $Header: /ncs/software/leap/base/products/mqd/src/mqdlog.c 8     3/28/01 12:01p Claytom $


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging/tracing functions for MQD

   mqd_lfx_log_reg....................Routine to register with DTA for looging
   mqd_flx_log_dereg..................Routine to deregister from DTA
   mqd_log............................Routine to MQD Logs
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mqd.h"

/****************************************************************************\
 Name          : mqd_flx_log_reg

 Description   : This is the function which registers the MQD logging with
                 the Flex Log agent.
                 

 Arguments     : None.

 Return Values : None
\*****************************************************************************/
void mqd_flx_log_reg(void)
{
#if((NCS_DTA == 1) && (NCS_MQSV_LOG == 1))
   NCS_DTSV_RQ            reg;

   m_NCS_MEMSET(&reg, 0, sizeof(NCS_DTSV_RQ));
   reg.i_op = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_MQD;
   /* fill version no. */
   reg.info.bind_svc.version = MQSV_LOG_VERSION;
   /* fill svc_name */
   m_NCS_STRCPY(reg.info.bind_svc.svc_name, "MQSv");

   ncs_dtsv_su_req(&reg);
#endif
   return;
} /* End of mqd_flx_log_reg() */

/****************************************************************************
 Name          : mqd_flx_log_dereg
 
 Description   : This is the function which deregisters the MQD logging 
                 with the Flex Log agent.
                 
 Arguments     : None.
 
 Return Values : None 
\*****************************************************************************/
void mqd_flx_log_dereg(void)
{
#if((NCS_DTA == 1) && (NCS_MQSV_LOG == 1))
   NCS_DTSV_RQ        reg;
   
   m_NCS_MEMSET(&reg, 0, sizeof(NCS_DTSV_RQ));
   reg.i_op = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_MQD;
   ncs_dtsv_su_req(&reg);
#endif
   return;
} /* End of mqd_flx_log_dereg() */

#if((NCS_DTA == 1) && (NCS_MQSV_LOG == 1))

/*****************************************************************************

  PROCEDURE NAME  :  mqd_log

  DESCRIPTION     :  MQD RNew logs

  ARGUMENT        :  id - Canned string id
                     category- Category of the log
                     sev - Severity of the log
                     rc - return code of the API or function
                     fname- filename
                     fno  - filenumber 
*****************************************************************************/

void mqd_log(uns8 id,uns32 category,uns8 sev,uns32 rc,char *fname,uns32 fno)
{
   
   /* Log New type logs */
   ncs_logmsg(NCS_SERVICE_ID_MQD, MQD_LID_HDLN, MQD_FC_HDLN,
              category, sev, NCSFL_TYPE_TCLIL,fname,fno,id,rc);

} /* End of mqd_new_log()  */

#endif /* (NCS_DTA == 1) && (NCS_MQSV_LOG == 1) */
