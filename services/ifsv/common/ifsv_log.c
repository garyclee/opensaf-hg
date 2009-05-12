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
  FILE NAME: IFSV_LOG.C

  DESCRIPTION: IfSv Log file. This contains log format function and log 
               related functions.

  FUNCTIONS INCLUDED in this module:    

  ifsv_flx_log_reg ................ Register with Flex Log.
  ifsv_flx_log_dereg .............. Deregister with Flex Log.  
  ifsv_log_svc_id_from_comp_type  . get the service ID from comp type.
  ifsv_log_spt_string ............. converts shelf,slot,port,type, scope in to 
                                    string.  

******************************************************************************/

#include "ifsv.h"

/****************************************************************************
 * Name          : m_IFSV_LOG_SPT_INFO 
 *
 * Description   : This is the function displays spt info 
 *                 
 *
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void m_IFSV_LOG_SPT_INFO(uns32 index, NCS_IFSV_SPT_MAP *spt_map,uns8 *info2)
{
   uns8 string[128];

   memset(string,0,128);

   if(spt_map == NULL)
       return; 
   /* embedding subslot changes */
   sprintf(string,"Shelf:%d Slot:%d SubSlot:%d Port:%d Type:%d Scope:%d = IfIndex: %d",
                   spt_map->spt.shelf,
                   spt_map->spt.slot,
                   spt_map->spt.subslot,
                   spt_map->spt.port,
                   spt_map->spt.type,
                   spt_map->spt.subscr_scope,
                   spt_map->if_index);

   m_IFSV_LOG_STR_2_NORMAL(NCS_SERVICE_ID_IFD,index,string,info2);

   return;
}

/****************************************************************************
 * Name          : ifsv_flx_log_TCIC_fmt
 *
 * Description   : This is the function which logs TICL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - string Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TCIC_fmt (uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
             uns32 fmt_id, uns32 indx, uns8 *info1, uns8 *info2)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, NCSFL_TYPE_TCIC, info1,indx,info2);
   return;
}


/****************************************************************************
 * Name          : ifsv_flx_log_TICL_fmt
 *
 * Description   : This is the function which logs TICL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - string Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TICL_fmt (uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                       uns32 fmt_id, uns32 indx, uns8 *info1, uns32 info2)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, NCSFL_TYPE_TICL,
      indx,info1,info2);
   return;
}

/****************************************************************************
 * Name          : ifsv_flx_log_TICLL_fmt
 *
 * Description   : This is the function which logs TICLL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - string Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *               : info3           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TICLL_fmt (uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                        uns32 fmt_id, uns32 indx, uns8 *info1, uns32 info2,
                        uns32 info3)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, NCSFL_TYPE_TICLL,
      indx, info1, info2, info3);   
   return;
}

/****************************************************************************
 * Name          : ifsv_flx_log_TICLLL_fmt
 *
 * Description   : This is the function which logs TICLLL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - string Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *               : info3           - Long Information to be logged. 
 *               : info4           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TICLLL_fmt (uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                         uns32 fmt_id, uns32 indx, uns8 *info1, uns32 info2,
                         uns32 info3, uns32 info4)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, 
      NCSFL_TYPE_TICLLL, indx, info1, info2, info3, info4);   
   return;     
}

/****************************************************************************
 * Name          : ifsv_flx_log_TICLLLLL_fmt
 *
 * Description   : This is the function which logs TICLLL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - string Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *               : info3           - Long Information to be logged. 
 *               : info4           - Long Information to be logged. 
 *               : info5           - Long Information to be logged. 
 *               : info6           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TICLLLLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                          uns32 fmt_id, uns32 indx, uns8 *info1, uns32 info2,
                          uns32 info3, uns32 info4, uns32 info5, uns32 info6)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, 
      NCSFL_TYPE_TICLLLLL, indx, info1, info2, info3, info4, info5, info6);   
   return;     
}


/****************************************************************************
 * Name          : ifsv_flx_log_TICLLLLLL_fmt
 *
 * Description   : This is the function which logs TICLLLLLL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - string Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *               : info3           - Long Information to be logged. 
 *               : info4           - Long Information to be logged. 
 *               : info5           - Long Information to be logged. 
 *               : info6           - Long Information to be logged.
 *               : info7           - Long Information to be logged.
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TICLLLLLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                          uns32 fmt_id, uns32 indx, uns8 *info1, uns32 info2,
                          uns32 info3, uns32 info4, uns32 info5, uns32 info6,
                          uns32 info7)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, 
      NCSFL_TYPE_TICLLLLLL, indx, info1, info2, info3, info4, info5, info6,
      info7);   
   return;     
}


/****************************************************************************
 * Name          : ifsv_flx_log_TICLLLLLLL_fmt
 *
 * Description   : This is the function which logs TICLLLLLLL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - string Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *               : info3           - Long Information to be logged. 
 *               : info4           - Long Information to be logged. 
 *               : info5           - Long Information to be logged. 
 *               : info6           - Long Information to be logged. 
 *               : info7           - Long Information to be logged. 
 *               : info8           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TICLLLLLLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                          uns32 fmt_id, uns32 indx, uns8 *info1, uns32 info2,
                          uns32 info3, uns32 info4, uns32 info5, uns32 info6,
                          uns32 info7, uns32 info8)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, 
      NCSFL_TYPE_TICLLLLLLL, indx, info1, info2, info3, info4, info5, info6,
      info7, info8);   
   return;     
}
/****************************************************************************
 * Name          : ifsv_flx_log_TIL_fmt
 *
 * Description   : This is the function which logs TIL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info            - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TIL_fmt (uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                      uns32 fmt_id, uns32 indx, uns32 info)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, 
      NCSFL_TYPE_TIL, indx, info);   
   return;        
}

/****************************************************************************
 * Name          : ifsv_flx_log_TILL_fmt
 *
 * Description   : This is the function which logs TILL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - Long Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TILL_fmt (uns32 scv_id, uns32 cat, uns32 str_set, 
                       uns32 sev, uns32 fmt_id, uns32 indx, uns32 info1,
                       uns32 info2)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, 
      NCSFL_TYPE_TILL, indx, info1, info2);   
   return;     
}

/****************************************************************************
 * Name          : ifsv_flx_log_TILLL_fmt
 *
 * Description   : This is the function which logs TILLL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - Long Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *               : info3           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TILLL_fmt (uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                        uns32 fmt_id, uns32 indx, uns32 info1, uns32 info2, 
                        uns32 info3)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, 
      NCSFL_TYPE_TILLL, indx, info1, info2, info3);   
   return;   
}

/****************************************************************************
 * Name          : ifsv_flx_log_TILLLL_fmt
 *
 * Description   : This is the function which logs TILLLL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - Long Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *               : info3           - Long Information to be logged. 
 *               : info4           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TILLLL_fmt (uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                         uns32 fmt_id, uns32 indx, uns32 info1, uns32 info2, 
                         uns32 info3, uns32 info4)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, 
      NCSFL_TYPE_TILLLL, indx, info1, info2, info3, info4);   
   return;    
}

/****************************************************************************
 * Name          : ifsv_flx_log_TILLLLL_fmt
 *
 * Description   : This is the function which logs TILLLLL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - Long Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *               : info3           - Long Information to be logged. 
 *               : info4           - Long Information to be logged. 
 *               : info5           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TILLLLL_fmt (uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                          uns32 fmt_id, uns32 indx, uns32 info1, uns32 info2, 
                          uns32 info3, uns32 info4, uns32 info5)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, 
      NCSFL_TYPE_TILLLLL, indx, info1, info2, info3, info4, info5);   
   return;       
}


/****************************************************************************
 * Name          : ifsv_flx_log_TILLLLLL_fmt
 *
 * Description   : This is the function which logs TILLLLLL format
 *                 
 *
 * Arguments     : scv_id          - service ID for which log is done.
 *                 cat             - category
 *                 str_set         - string set
 *                 sev             - Severity
 *                 fmt_id          - String format ID.
 *                 indx            - Canned string index.
 *               : info1           - Long Information to be logged. 
 *               : info2           - Long Information to be logged. 
 *               : info3           - Long Information to be logged. 
 *               : info4           - Long Information to be logged. 
 *               : info5           - Long Information to be logged. 
 *               : info6           - Long Information to be logged. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_TILLLLLL_fmt (uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                           uns32 fmt_id, uns32 indx, uns32 info1, uns32 info2,
                           uns32 info3, uns32 info4, uns32 info5, uns32 info6)
{
   ncs_logmsg(scv_id, (uns8)fmt_id, (uns8)str_set, cat, (uns8)sev, 
      NCSFL_TYPE_TILLLLLL, indx, info1, info2, info3, info4, info5, info6);   
   return;   
}

/****************************************************************************
 * Name          : ifsv_flx_log_reg
 *
 * Description   : This is the function which registers the IfSv logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : comp_type    - Component Type to be registered with. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifsv_flx_log_reg (uns32 comp_type)
{
   NCS_DTSV_RQ            reg;
   NCS_SERVICE_ID         scv_id=0;

   if (comp_type == IFSV_COMP_IFD)
   {
      scv_id = NCS_SERVICE_ID_IFD;
   } else if (comp_type == IFSV_COMP_IFND)
   {
      scv_id = NCS_SERVICE_ID_IFND;
   }else if (comp_type == IFSV_COMP_IFA)
   {
      scv_id = NCS_SERVICE_ID_IFA;
   }
   

   memset(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = scv_id;
   /* fill version no. */
   reg.info.bind_svc.version = IFSV_LOG_VERSION;
   /* fill svc_name */
   strcpy(reg.info.bind_svc.svc_name, "IFSv");


   return(ncs_dtsv_su_req(&reg));   
}

/****************************************************************************
 * Name          : ifsv_flx_log_dereg
 *
 * Description   : This is the function which deregisters the IfSv logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : comp_type    - Component Type to be deregistered with. 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
ifsv_flx_log_dereg (uns32 comp_type)
{
   NCS_DTSV_RQ        reg;
   NCS_SERVICE_ID scv_id=0;

   if (comp_type == IFSV_COMP_IFD)
   {
      scv_id = NCS_SERVICE_ID_IFD;
   } else if (comp_type == IFSV_COMP_IFND)
   {
      scv_id = NCS_SERVICE_ID_IFND;
   }else if (comp_type == IFSV_COMP_IFA)
   {
      scv_id = NCS_SERVICE_ID_IFA;
   }
   memset(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = scv_id;
   ncs_dtsv_su_req(&reg);
   return;
}

/****************************************************************************
 * Name          : ifsv_log_svc_id_from_comp_type
 *
 * Description   : This is the function which gets the serivce Id from the 
 *                 component type.
 *                 
 *
 * Arguments     : comp_type    - Component Type. 
 *
 * Return Values : Service ID
 *
 * Notes         : None.
 *****************************************************************************/
NCS_SERVICE_ID 
ifsv_log_svc_id_from_comp_type (uns32 comp_type)
{
   NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_BASE;

   if(comp_type == IFSV_COMP_IFD)
   {
      svc_id = NCS_SERVICE_ID_IFD;
   } else if (comp_type == IFSV_COMP_IFND)
   {
      svc_id = NCS_SERVICE_ID_IFND;
   } else
   {
      svc_id = NCS_SERVICE_ID_IFA;
   }
   return (svc_id);
}

/****************************************************************************
 * Name          : ifsv_log_spt_string
 *
 * Description   : This is the function which converts 
 *                 shelf/slot/subslot/port/type/scope in to string.
 *                 
 *
 * Arguments     : spt       - shelf/slot/subslot/port/type/scope
 *                 o_spt_str - string converts the shelf/slot/subslot/port/type/scope 
 *                 in to a string.
 *
 * Return Values : char *
 *
 * Notes         : None.
 *****************************************************************************/
char *
ifsv_log_spt_string (NCS_IFSV_SPT spt, char *o_spt_str)
{
   char tem_char[35];
   sprintf(tem_char,"%d",spt.shelf);
   strcpy(o_spt_str,tem_char);
   strcat(o_spt_str,"/");

   sprintf(tem_char,"%d",spt.slot);
   strcat(o_spt_str,tem_char);
   strcat(o_spt_str,"/");
   
   /* embedding subslot changes */
   sprintf(tem_char,"%d",spt.subslot);
   strcat(o_spt_str,tem_char);
   strcat(o_spt_str,"/");

   sprintf(tem_char,"%d",spt.port);
   strcat(o_spt_str,tem_char);
   strcat(o_spt_str,"/");

   sprintf(tem_char,"%d",spt.type);
   strcat(o_spt_str,tem_char);
   strcat(o_spt_str,"/");
   
   sprintf(tem_char,"%d",spt.subscr_scope);
   strcat(o_spt_str,tem_char);
   
   return (o_spt_str);
}
