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


#include "mqnd.h"

/*****************************************************************************
  FILE NAME: MQND_LOG.C

  DESCRIPTION: MQND Logging utilities

  FUNCTIONS INCLUDED in this module:
      mqnd_flx_log_reg
      mqnd_flx_log_unreg


******************************************************************************/


#if 0

/*****************************************************************************

  PROCEDURE NAME:    mqnd_log_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void mqnd_log_headline(uns8 hdln_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_HDLN, MQND_FC_HDLN, 
               NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, 
               hdln_id);
}


/*****************************************************************************

  PROCEDURE NAME:    mqnd_log_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/
void mqnd_log_memfail(uns8 mf_id)
{
    ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_MEMFAIL, MQND_FC_MEMFAIL, 
               NCSFL_LC_MEMORY, NCSFL_SEV_ERROR, NCSFL_TYPE_TI, 
               mf_id);
}

/*****************************************************************************

  PROCEDURE NAME:    mqnd_log_evt

  DESCRIPTION:       Event logging info

*****************************************************************************/
void mqnd_log_evt(uns32 evt_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_EVT, MQND_FC_EVT, 
        NCSFL_LC_EVENT, sev, NCSFL_TYPE_TIL, evt_id);
}

/*****************************************************************************

  PROCEDURE NAME:    mqnd_log_tmr

  DESCRIPTION:       Timer logging info

*****************************************************************************/
void mqnd_log_tmr(uns8 hdln_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_HDLN, MQND_FC_HDLN, 
               NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, 
               hdln_id);
}

/*****************************************************************************

  PROCEDURE NAME:    mqnd_log_api

  DESCRIPTION:       API logging info

*****************************************************************************/
void mqnd_log_api(uns8 api_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_API, MQND_FC_API, 
               NCSFL_LC_API, sev, NCSFL_TYPE_TI, 
               api_id);
}

/*****************************************************************************

  PROCEDURE NAME:    mqnd_log_lockfail

  DESCRIPTION:       API logging info

*****************************************************************************/
void mqnd_log_lockfail(uns8 api_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_API, MQND_FC_API, 
               NCSFL_LC_LOCKS , sev, NCSFL_TYPE_TI, 
               api_id);
}


/*****************************************************************************

  PROCEDURE NAME:    mqnd_log_sys_call

  DESCRIPTION:       System call logging info

*****************************************************************************/
void mqnd_log_sys_call(uns8 id, uns32 node)
{
    ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_SYS_CALL, MQND_FC_SYS_CALL, 
        NCSFL_LC_SYS_CALL_FAIL, NCSFL_SEV_ERROR, NCSFL_TYPE_TIL, 
        id, node);
}

/*****************************************************************************

  PROCEDURE NAME:    mqnd_log_data_send

  DESCRIPTION:       MDS send logging info

*****************************************************************************/
void mqnd_log_data_send(uns8 id, uns32 node, uns32 evt_id)
{
    ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_DATA_SEND, MQND_FC_DATA_SEND, 
        NCSFL_LC_DATA, NCSFL_SEV_ERROR, NCSFL_TYPE_TILL, 
        id, node,evt_id);
}
#endif


/*****************************************************************************

  PROCEDURE NAME  :  mqnd_log

  DESCRIPTION     :  MQND logs

  ARGUMENT        :  id - Canned string id
                     category- Category of the log
                     sev - Severity of the log
                     rc - return code of the API or function
                     fname- filename
                     fno  - filenumber
*****************************************************************************/

void mqnd_log(uns8 id,uns32 category,uns8 sev,uns32 rc,char *fname,uns32 fno)
{

   /* Log New type logs */
   ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_HDLN, MQND_FC_HDLN,
              category, sev, NCSFL_TYPE_TCLIL,fname,fno,id,rc);

} /* End of mqnd_new_log()  */



/****************************************************************************
 * Name          : mqnd_flx_log_reg
 *
 * Description   : This is the function which registers the MQND logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void mqnd_flx_log_reg (void)
{
   NCS_DTSV_RQ            reg;

   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_MQND;
   /* fill version no. */
   reg.info.bind_svc.version = MQSV_LOG_VERSION;
   /* fill svc_name */
   m_NCS_STRCPY(reg.info.bind_svc.svc_name, "MQSv");

   ncs_dtsv_su_req(&reg);
   return;
}

/****************************************************************************
 * Name          : mqnd_flx_log_dereg
 *
 * Description   : This is the function which deregisters the MQND logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
mqnd_flx_log_dereg (void)
{
   NCS_DTSV_RQ        reg;
   
   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                   = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_MQND;
   ncs_dtsv_su_req(&reg);
   return;
}
