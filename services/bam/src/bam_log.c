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

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */


#include "bam_log.h"

/*****************************************************************************

  PROCEDURE NAME:    bam_log_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void bam_log_headline(uns8 hdln_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_BAM, BAM_LID_HDLN, BAM_FC_HDLN, 
               NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, 
               hdln_id);
}

/*****************************************************************************

  PROCEDURE NAME:    bam_log_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/
void bam_log_memfail(uns8 mf_id)
{
    ncs_logmsg(NCS_SERVICE_ID_BAM, BAM_LID_MEMFAIL, BAM_FC_MEMFAIL, 
               NCSFL_LC_MEMORY, NCSFL_SEV_ERROR, NCSFL_TYPE_TI, 
               mf_id);
}

/*****************************************************************************

  PROCEDURE NAME:    bam_log_api

  DESCRIPTION:       API logging info

*****************************************************************************/
void bam_log_api(uns8 api_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_BAM, BAM_LID_API, BAM_FC_API, 
               NCSFL_LC_API, sev, NCSFL_TYPE_TI, 
               api_id);
}

/*****************************************************************************

  PROCEDURE NAME:    bam_log_msg

  DESCRIPTION:       Event logging info

*****************************************************************************/
void bam_log_msg(uns8 msg_id, uns8 sev, uns32 node)
{
    ncs_logmsg(NCS_SERVICE_ID_BAM, BAM_LID_MSG, BAM_FC_MSG, 
        NCSFL_LC_EVENT, sev, NCSFL_TYPE_TIL, 
        msg_id, node);
}

/*****************************************************************************

  PROCEDURE NAME:    bam_log_msg_TIL

  DESCRIPTION:       Event logging info

*****************************************************************************/
void bam_log_msg_TIL(uns8 msg_id, uns8 sev, uns32 info)
{
    ncs_logmsg(NCS_SERVICE_ID_BAM, BAM_LID_MSG_TIL, BAM_FC_INFO, 
        NCSFL_LC_EVENT, sev, NCSFL_TYPE_TIL, 
        msg_id, info);
}

/****************************************************************************
 * Name          : bam_log_msg_TIC
 *
 * Description   : This is the function which logs TIC format
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/

void bam_log_msg_TIC(uns8 msg_id, uns8 sev, char *string1)
{

   ncs_logmsg(NCS_SERVICE_ID_BAM, BAM_LID_MSG_TIC, BAM_FC_INFO,
        NCSFL_LC_EVENT, sev, NCSFL_TYPE_TIC,
        msg_id, string1);
}

/*****************************************************************************

  PROCEDURE NAME:    bam_log_svc_prvdr

  DESCRIPTION:       Service Provider logging info

*****************************************************************************/
void bam_log_svc_prvdr(uns8 sp_id, uns8 sev)
{
    ncs_logmsg(NCS_SERVICE_ID_BAM, BAM_LID_SVC_PRVDR, BAM_FC_SVC_PRVDR, 
               NCSFL_LC_SVC_PRVDR, sev, NCSFL_TYPE_TI, 
               sp_id);
}

/****************************************************************************
 * Name          : bam_concatenate_twin_string
 *
 * Description   : This is the function which converts two strings to one 
 *                 for printing.
 *                 
 *
 * Arguments     : char *string1 - First String
 *                 char *string2 - second String
 *                 o_str - output string
 *
 * Return Values : char *
 *
 * Notes         : None.
 *****************************************************************************/
char *bam_concatenate_twin_string (char *string1,char *string2, char *o_str)
{
    if(string1)
    {
        strncpy(o_str,string1, 255);
        strncat(o_str,",", 255-strlen(o_str));
    }

    if(string2) 
    {
        strncat(o_str,string2, 255-strlen(o_str));
    }
   
    return (o_str);
}


/****************************************************************************
 * Name          : bam_log_TICL_fmt_2string
 *
 * Description   : This is the function which logs TICL format
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/

void bam_log_TICL_fmt_2string(uns8 msg_id, uns8 sev, char *string1, char *string2)
{
   char log_info[256]; /* TBD Fix me */

   ncs_logmsg(NCS_SERVICE_ID_BAM, BAM_LID_MSG_TIC, BAM_FC_MSG, 
        NCSFL_LC_EVENT, sev, NCSFL_TYPE_TIC, 
        msg_id, bam_concatenate_twin_string(string1,string2,log_info));
}

/****************************************************************************
 * Name          : bam_flx_log_reg
 *
 * Description   : This is the function which registers the BAM logging with
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
bam_flx_log_reg(void)
{
   NCS_DTSV_RQ            reg;

   memset(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_BAM;
   /* fill version no. */
   reg.info.bind_svc.version = BAM_LOG_VERSION;
   /* fill svc_name */
   strncpy(reg.info.bind_svc.svc_name, "BAM", sizeof(reg.info.bind_svc.svc_name)-1);

   ncs_dtsv_su_req(&reg);
   return;
}

/****************************************************************************
 * Name          : bam_flx_log_dereg
 *
 * Description   : This is the function which deregisters the BAM logging 
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
bam_flx_log_dereg(void)
{
   NCS_DTSV_RQ        reg;
   
   memset(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                   = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_BAM;
   ncs_dtsv_su_req(&reg);
   return;
}

