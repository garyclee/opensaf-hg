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

#include "hcd.h"

#if (NCS_HISV_LOG == 1)
/*****************************************************************************
  FILE NAME: HCD_LOG.C

  DESCRIPTION: HCD Logging utilities

  FUNCTIONS INCLUDED in this module:
      hisv_flx_log_reg
      hisv_flx_log_unreg
******************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:    hisv_log_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void hisv_log_headline(uns8 hdln_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_HCD, HCD_LID_HDLN, HCD_FC_HDLN,
               NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI,
               hdln_id);
}

/*****************************************************************************

  PROCEDURE NAME:    hcd_log_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/
void hisv_log_memfail(uns8 mf_id)
{
    ncs_logmsg(NCS_SERVICE_ID_HCD, HCD_LID_MEMFAIL, HCD_FC_MEMFAIL,
               NCSFL_LC_MEMORY, NCSFL_SEV_CRITICAL, NCSFL_TYPE_TI,
               mf_id);
}

/*****************************************************************************

  PROCEDURE NAME:    hisv_log_api

  DESCRIPTION:       API logging info

*****************************************************************************/
void hisv_log_api(uns8 api_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_HCD, HCD_LID_API, HCD_FC_API,
               NCSFL_LC_API, sev, NCSFL_TYPE_TI,
               api_id);
}

/*****************************************************************************

  PROCEDURE NAME:    hisv_log_fwprog

  DESCRIPTION:       API firmware progress logging info

*****************************************************************************/
void hisv_log_fwprog(uns8 fwp_id, uns8 sev, uns32 num)
{
    ncs_logmsg(NCS_SERVICE_ID_HCD, HCD_LID_FWPROG, HCD_FC_FWPROG,
               NCSFL_LC_EVENT, sev, NCSFL_TYPE_TILL,
               fwp_id, num);
}

/*****************************************************************************

  PROCEDURE NAME:    hisv_log_fwerr

  DESCRIPTION:       API firmware error logging info

*****************************************************************************/
void hisv_log_fwerr(uns8 fwe_id, uns8 sev, uns32 num)
{
    ncs_logmsg(NCS_SERVICE_ID_HCD, HCD_LID_FWERR, HCD_FC_FWERR,
               NCSFL_LC_EVENT, sev, NCSFL_TYPE_TILL,
               fwe_id, num);
}

/*****************************************************************************

  PROCEDURE NAME:    hisv_log_evt

  DESCRIPTION:       Event logging info

*****************************************************************************/
void hisv_log_evt(uns8 evt_id, uns32 rsc_id, uns32 node)
{
    ncs_logmsg(NCS_SERVICE_ID_HCD, HCD_LID_EVT, HCD_FC_EVT,
        NCSFL_LC_EVENT, NCSFL_SEV_INFO, NCSFL_TYPE_TILL,
        evt_id,rsc_id, node);
}

/*****************************************************************************

  PROCEDURE NAME:    hisv_log_svc_prvdr

  DESCRIPTION:       Service Provider logging info

*****************************************************************************/
void hisv_log_svc_prvdr(uns8 sp_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_HCD, HCD_LID_SVC_PRVDR, HCD_FC_SVC_PRVDR,
               NCSFL_LC_SVC_PRVDR, sev, NCSFL_TYPE_TI,
               sp_id);
}

/*****************************************************************************

  PROCEDURE NAME:    hisv_log_lck_oper

  DESCRIPTION:       lock oriented logging info

*****************************************************************************/
void hisv_log_lck_oper(uns8 lck_id, uns8 sev ,char *rsc_name, uns32 rsc_id, uns32 node)
{
    ncs_logmsg(NCS_SERVICE_ID_DTSV, HCD_LID_LCK_OPER, HCD_FC_LCK_OPER,
               NCSFL_LC_MISC, sev, NCSFL_TYPE_TICLL,
               lck_id,rsc_name, rsc_id, node);
}

/*****************************************************************************

  PROCEDURE  : hisv_log_bind

  DESCRIPTION: Function is used for binding with HISV.

*****************************************************************************/

uns32 hisv_log_bind()
{
   NCS_DTSV_RQ        reg;

   reg.i_op = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_HCD;
   /* fill version no. */
    reg.info.bind_svc.version = HISV_LOG_VERSION;
    /* fill svc_name */
    m_NCS_STRCPY(reg.info.bind_svc.svc_name, "HISv");

   if (ncs_dtsv_su_req(&reg) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE  :hisv_log_unbind

  DESCRIPTION: Function is used for unbinding with HISV.

*****************************************************************************/

uns32 hisv_log_unbind()
{
   NCS_DTSV_RQ        dereg;

   dereg.i_op = NCS_DTSV_OP_UNBIND;
   dereg.info.unbind_svc.svc_id = NCS_SERVICE_ID_HCD;
   if (ncs_dtsv_su_req(&dereg) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;
   return NCSCC_RC_SUCCESS;
}

#else

/****************************************************************************
 * Name          : hisv_flx_log_reg
 *
 * Description   : This is the function which registers the HCD logging with
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
hisv_flx_log_reg ()
{
   NCS_DTSV_RQ            reg;

   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_HCD;
   ncs_dtsv_su_req(&reg);
   return;
}

/****************************************************************************
 * Name          : hisv_flx_log_dereg
 *
 * Description   : This is the function which deregisters the HCD logging
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
hisv_flx_log_dereg ()
{
   NCS_DTSV_RQ        reg;

   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                   = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_HCD;
   ncs_dtsv_su_req(&reg);
   return;
}

extern int dummy;
#endif
