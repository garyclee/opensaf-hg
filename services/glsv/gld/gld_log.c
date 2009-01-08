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

#include "gld.h"

#if (NCS_GLSV_LOG == 1)
/*****************************************************************************
  FILE NAME: GLD_LOG.C

  DESCRIPTION: GLD Logging utilities

  FUNCTIONS INCLUDED in this module:
      gld_flx_log_reg
      gld_flx_log_unreg
******************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:    gld_log_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void gld_log_headline(uns8 hdln_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_HDLN, GLD_FC_HDLN, 
               NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, 
               hdln_id);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/
void gld_log_memfail(uns8 mf_id)
{
    ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_MEMFAIL, GLD_FC_MEMFAIL, 
               NCSFL_LC_MEMORY, NCSFL_SEV_ERROR, NCSFL_TYPE_TI, 
               mf_id);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_api

  DESCRIPTION:       API logging info

*****************************************************************************/
void gld_log_api(uns8 api_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_API, GLD_FC_API, 
               NCSFL_LC_API, sev, NCSFL_TYPE_TI, 
               api_id);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_evt

  DESCRIPTION:       Event logging info

*****************************************************************************/
void gld_log_evt(uns8 evt_id, uns32 rsc_id, uns32 node)
{
    ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_EVT, GLD_FC_EVT, 
        NCSFL_LC_EVENT, NCSFL_SEV_INFO, NCSFL_TYPE_TILL, 
        evt_id,rsc_id, node);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_mbcsv_log

  DESCRIPTION:       mbcsv failure logging info

*****************************************************************************/


void gld_mbcsv_log(uns8 mbcsv_id,uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_MBCSV,GLD_FC_MBCSV,NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, mbcsv_id);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_svc_prvdr

  DESCRIPTION:       Service Provider logging info

*****************************************************************************/
void gld_log_svc_prvdr(uns8 sp_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_SVC_PRVDR, GLD_FC_SVC_PRVDR, 
               NCSFL_LC_SVC_PRVDR, sev, NCSFL_TYPE_TI, 
               sp_id);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_lck_oper

  DESCRIPTION:       lock oriented logging info

*****************************************************************************/
void gld_log_lck_oper(uns8 lck_id, uns8 sev ,char *rsc_name, uns32 rsc_id, uns32 node)
{
    ncs_logmsg(NCS_SERVICE_ID_DTSV, GLD_LID_LCK_OPER, GLD_FC_LCK_OPER,
               NCSFL_LC_MISC, sev, NCSFL_TYPE_TICLL,
               lck_id,rsc_name, rsc_id, node);
}


/****************************************************************************
 * Name          : gld_flx_log_reg
 *
 * Description   : This is the function which registers the GLD logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
gld_flx_log_reg (void)
{
   NCS_DTSV_RQ            reg;

   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_GLD;
   /* fill version no. */
   reg.info.bind_svc.version = GLSV_LOG_VERSION;
   /* fill svc_name */
   m_NCS_STRCPY(reg.info.bind_svc.svc_name, "GLSv");

   ncs_dtsv_su_req(&reg);
   return;
}

/****************************************************************************
 * Name          : gld_flx_log_dereg
 *
 * Description   : This is the function which deregisters the GLD logging 
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
gld_flx_log_dereg ()
{
   NCS_DTSV_RQ        reg;
   
   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                   = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_GLD;
   ncs_dtsv_su_req(&reg);
   return;
}
/*****************************************************************************

  PROCEDURE NAME:    gld_log_timer

  DESCRIPTION:       Timer stast/stop/exp logging info

*****************************************************************************/
void gld_log_timer(uns8 id, uns32 type)
{
    ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_TIMER, GLD_FC_TIMER,
        NCSFL_LC_TIMER, NCSFL_SEV_ERROR, NCSFL_TYPE_TIL,id,type);
}

#else
extern int dummy;
#endif
