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



#include "cpnd.h"

/*****************************************************************************
  FILE NAME: CPND_LOG.C

  DESCRIPTION: CPND Logging utilities

  FUNCTIONS INCLUDED in this module:
      cpnd_flx_log_reg
      cpnd_flx_log_unreg


******************************************************************************/


#if 0
/*****************************************************************************

  PROCEDURE NAME:    cpnd_log_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void cpnd_log_headline(uns8 hdln_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_HDLN, CPND_FC_HDLN, 
               NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, 
               hdln_id);
}

/*****************************************************************************

  PROCEDURE NAME:    cpnd_log_tmr

  DESCRIPTION:       Timer logging info

*****************************************************************************/
void cpnd_log_tmr(uns8 hdln_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_HDLN, CPND_FC_HDLN, 
               NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, 
               hdln_id);
}

/*****************************************************************************

  PROCEDURE NAME:    cpnd_log_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/
void cpnd_log_memfail(uns8 mf_id)
{
    ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_MEMFAIL, CPND_FC_MEMFAIL, 
               NCSFL_LC_MEMORY, NCSFL_SEV_ERROR, NCSFL_TYPE_TICL, 
               mf_id,__FILE__,__LINE__);
}

/*****************************************************************************

  PROCEDURE NAME:    cpnd_log_api

  DESCRIPTION:       API logging info

*****************************************************************************/
void cpnd_log_api(uns8 api_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_API, CPND_FC_API, 
               NCSFL_LC_API, sev, NCSFL_TYPE_TI, 
               api_id);
}


/*****************************************************************************

  PROCEDURE NAME:    cpnd_log_restart

  DESCRIPTION:       Restart logging info

*****************************************************************************/
void cpnd_log_restart(uns8 id,uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_RESTART, CPND_FC_RESTART,
               NCSFL_LC_API, sev, NCSFL_TYPE_TI,
               id);
}


/*****************************************************************************

  PROCEDURE NAME:    cpnd_mds_send_fail

  DESCRIPTION:       MDS send fail logging info

*****************************************************************************/

void cpnd_log_mds_send_fail(uns8 id,uns8 sev,uns64 from_dest,uns64 to_dest,uns64 ckpt_id,char *filename, uns32 lineno)
{
   ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_MDSFAIL, CPND_FC_MDSFAIL,
               NCSFL_LC_HEADLINE , sev, NCSFL_TYPE_TIFFFCL, id,(double)from_dest,
                (double)to_dest,(double)ckpt_id,filename,lineno );
}



void cpnd_log_ckptinfo(uns8 id, uns8 sev,uns64 ckptid,char *filename, uns32 lineno)
{
   ncs_logmsg(NCS_SERVICE_ID_CPND,CPND_LID_CKPTINFO,CPND_FC_CKPTINFO,
              NCSFL_LC_HEADLINE,sev, NCSFL_TYPE_TIFCL,id,(double)ckptid,filename,lineno);
}

/*****************************************************************************

  PROCEDURE NAME:    cpnd_log_lockfail

  DESCRIPTION:       API logging info

*****************************************************************************/
void cpnd_log_lockfail(uns8 api_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_API, CPND_FC_API, 
               NCSFL_LC_LOCKS , sev, NCSFL_TYPE_TI, 
               api_id);
}

/*****************************************************************************

  PROCEDURE NAME:    cpnd_log_evt

  DESCRIPTION:       Event logging info

*****************************************************************************/
void cpnd_log_evt(uns32 evt_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_EVT, CPND_FC_EVT, 
        NCSFL_LC_EVENT, sev, NCSFL_TYPE_TI, evt_id);
}

/*****************************************************************************

  PROCEDURE NAME:    cpnd_log_sys_call

  DESCRIPTION:       System call logging info

*****************************************************************************/
void cpnd_log_sys_call(uns8 id, uns32 node)
{
    ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_SYS_CALL, CPND_FC_SYS_CALL, 
        NCSFL_LC_SYS_CALL_FAIL, NCSFL_SEV_ERROR, NCSFL_TYPE_TIL, 
        id, node);
}

/*****************************************************************************

  PROCEDURE NAME:    cpnd_log_generic

  DESCRIPTION:       Generic logging info

*****************************************************************************/
void cpnd_log_generic(uns8 id, uns8 sev, uns32 rc, char *filename, uns32 lineno)
{
    ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_GENERIC, CPND_FC_GENERIC,
        NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TILCL, id, rc, filename, lineno);
}

#endif
#if 0

/*****************************************************************************

  PROCEDURE NAME:    cpnd_log_data_send

  DESCRIPTION:       MDS send logging info

*****************************************************************************/
void cpnd_log_data_send(uns8 id, uns32 node, uns32 evt_id)
{
    ncs_logmsg(NCS_SERVICE_ID_CPND, CPND_LID_DATA_SEND, CPND_FC_DATA_SEND, 
        NCSFL_LC_DATA, NCSFL_SEV_ERROR, NCSFL_TYPE_TILL, 
        id, node,evt_id);
}

#endif

/****************************************************************************
 * Name          : cpnd_flx_log_reg
 *
 * Description   : This is the function which registers the CPND logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void cpnd_flx_log_reg (void)
{
   NCS_DTSV_RQ            reg;

   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_CPND;
   /* fill version no. */
   reg.info.bind_svc.version = CPSV_LOG_VERSION;
   /* fill svc_name */
   m_NCS_STRCPY(reg.info.bind_svc.svc_name, "CPSv");
 
   ncs_dtsv_su_req(&reg);
   return;
}

/****************************************************************************
 * Name          : cpnd_flx_log_dereg
 *
 * Description   : This is the function which deregisters the CPND logging 
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
cpnd_flx_log_dereg (void)
{
   NCS_DTSV_RQ        reg;
   
   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                   = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_CPND;
   ncs_dtsv_su_req(&reg);
   return;
}

